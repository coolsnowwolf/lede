/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_event_timer_module.h"


static ngx_int_t ngx_event_timer_process_init(ngx_cycle_t *cycle);

static void *ngx_event_timer_create_conf(ngx_cycle_t *cycle);
static char *ngx_event_timer_init_conf(ngx_cycle_t *cycle, void *conf);


typedef struct {
    ngx_uint_t              timerid;

    ngx_event_t             event;

    ngx_timer_handler_pt    handler;
    void                   *data;
} ngx_event_timer_ctx_t;

typedef struct {
    ngx_uint_t              timer_n;

    ngx_event_timer_ctx_t  *timer_ctx; /* array to store timers */

    ngx_event_timer_ctx_t  *free_timers; /* timer unused */
    ngx_uint_t              free_timer_n;
} ngx_event_timer_conf_t;


static ngx_str_t  event_timer_name = ngx_string("event_timer");


static ngx_command_t  ngx_event_timer_commands[] = {

    { ngx_string("worker_timers"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_event_timer_conf_t, timer_n),
      NULL },

      ngx_null_command
};


ngx_event_module_t  ngx_event_timer_module_ctx = {
    &event_timer_name,
    ngx_event_timer_create_conf,            /* create configuration */
    ngx_event_timer_init_conf,              /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


/* this module use ngx_cycle->log */
ngx_module_t  ngx_event_timer_module = {
    NGX_MODULE_V1,
    &ngx_event_timer_module_ctx,            /* module context */
    ngx_event_timer_commands,               /* module directives */
    NGX_EVENT_MODULE,                       /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_event_timer_process_init,           /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_event_timer_create_conf(ngx_cycle_t *cycle)
{
    ngx_event_timer_conf_t           *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_event_timer_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->timer_n = NGX_CONF_UNSET_UINT;

    return conf;
}

static char *
ngx_event_timer_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_event_timer_conf_t     *etcf = conf;

    ngx_conf_init_uint_value(etcf->timer_n, 1024);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_event_timer_process_init(ngx_cycle_t *cycle)
{
    ngx_event_timer_conf_t     *etcf;
    ngx_event_timer_ctx_t      *t, *next;
    ngx_uint_t                  i;

    etcf = ngx_event_get_conf(cycle->conf_ctx, ngx_event_timer_module);

    if (etcf->timer_n == 0) {
        return NGX_OK;
    }

    etcf->timer_ctx =
        ngx_alloc(sizeof(ngx_event_timer_ctx_t) * etcf->timer_n, cycle->log);
    if (etcf->timer_ctx == NULL) {
        return NGX_ERROR;
    }

    t = etcf->timer_ctx;

    i = etcf->timer_n;
    next = NULL;

    do {
        --i;

        t[i].timerid = i;
        t[i].event.timer_set = 0;
        t[i].event.log = NULL;
        t[i].handler = NULL;
        t[i].data = next;

        next = &t[i];
    } while (i);

    etcf->free_timers = next;
    etcf->free_timer_n = etcf->timer_n;

    return NGX_OK;
}

static ngx_event_timer_ctx_t *
ngx_event_timer_get_timer()
{
    ngx_event_timer_conf_t     *etcf;
    ngx_event_timer_ctx_t      *free;

    etcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_timer_module);

    free = etcf->free_timers;
    if (etcf->free_timer_n == 0) {
        ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0,
                      "nginx event timer module, no free timer");
        return NULL;
    }

    etcf->free_timers = free->data;
    --etcf->free_timer_n;

    return free;
}

static void
ngx_event_timer_free_timer(ngx_event_timer_ctx_t *ctx)
{
    ngx_event_timer_conf_t     *etcf;

    etcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_timer_module);

    ctx->data = etcf->free_timers;
    etcf->free_timers = ctx;
    ++etcf->free_timer_n;
}

static void
ngx_event_timer_event_handler(ngx_event_t *e)
{
    ngx_event_timer_ctx_t      *ctx;

    ctx = e->data;

    if (ctx->handler) {
        ctx->handler(ctx->data);
    }

    ngx_event_timer_free_timer(ctx);
}

ngx_int_t
ngx_event_timer_add_timer(ngx_msec_t tv, ngx_timer_handler_pt h, void *data)
{
    ngx_event_timer_ctx_t      *ctx;

    if (h == NULL) {
        return NGX_ERROR;
    }

    ctx = ngx_event_timer_get_timer();
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    ctx->event.handler = ngx_event_timer_event_handler;
    ctx->event.data = ctx;

    ctx->handler = h;
    ctx->data = data;

    if (ctx->event.log == NULL) {
        ctx->event.log = ngx_cycle->log;
    }

    ngx_event_add_timer(&ctx->event, tv);

    return ctx->timerid;
}

void
ngx_event_timer_del_timer(ngx_uint_t timerid)
{
    ngx_event_timer_ctx_t      *ctx;
    ngx_event_timer_conf_t     *etcf;

    etcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_timer_module);

    ctx = &etcf->timer_ctx[timerid];

    if (!ctx->event.timer_set) {
        ngx_log_debug0(NGX_LOG_DEBUG_EVENT, ngx_cycle->log, 0,
                       "timer has already deleted");
        return;
    }

    ngx_event_del_timer(&ctx->event);
    ngx_event_timer_free_timer(ctx);
}

ngx_chain_t *
ngx_event_timer_state(ngx_http_request_t *r)
{
    ngx_event_timer_conf_t     *etcf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len;

    etcf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_timer_module);

    len = sizeof("##########event timer state##########\n") - 1
        + sizeof("ngx_event_timer alloc: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_event_timer free: \n") - 1 + NGX_OFF_T_LEN;

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NULL;
    }
    cl->next = NULL;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NULL;
    }
    cl->buf = b;

    b->last = ngx_snprintf(b->last, len,
            "##########event timer state##########\n"
            "ngx_event_timer alloc: %ui\nngx_event_timer free: %ui\n",
            etcf->timer_n, etcf->free_timer_n);

    return cl;
}
