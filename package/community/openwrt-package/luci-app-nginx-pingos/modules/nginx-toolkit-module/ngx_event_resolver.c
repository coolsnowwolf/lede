/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_event_resolver.h"


static void *ngx_event_resolver_create_conf(ngx_cycle_t *cycle);
static char *ngx_event_resolver_init_conf(ngx_cycle_t *cycle, void *conf);

static char *ngx_event_resolver(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


typedef struct ngx_event_resolver_ctx_s ngx_event_resolver_ctx_t;

struct ngx_event_resolver_ctx_s {
    ngx_event_resolver_handler_pt   handler;
    void                           *data;
    ngx_event_resolver_ctx_t       *next;
};

typedef struct {
    ngx_msec_t                      resolver_timeout;
    ngx_resolver_t                 *resolver;
    ngx_event_resolver_ctx_t       *free_ctx;

    ngx_uint_t                      nalloc;
    ngx_uint_t                      nfree;
} ngx_event_resolver_conf_t;


static ngx_str_t    event_resolver_name = ngx_string("event_resolver");


static ngx_command_t  ngx_event_resolver_commands[] = {

	{ ngx_string("resolver"),
      NGX_EVENT_CONF|NGX_CONF_1MORE,
      ngx_event_resolver,
      0,
      0,
      NULL },

    { ngx_string("resolver_timeout"),
      NGX_EVENT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_event_resolver_conf_t, resolver_timeout),
      NULL },

	  ngx_null_command
};


ngx_event_module_t  ngx_event_resolver_module_ctx = {
    &event_resolver_name,
    ngx_event_resolver_create_conf,         /* create configuration */
    ngx_event_resolver_init_conf,           /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


/* this module use ngx_cycle->log */
ngx_module_t  ngx_event_resolver_module = {
    NGX_MODULE_V1,
    &ngx_event_resolver_module_ctx,         /* module context */
    ngx_event_resolver_commands,            /* module directives */
    NGX_EVENT_MODULE,                       /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_event_resolver_create_conf(ngx_cycle_t *cycle)
{
    ngx_event_resolver_conf_t      *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_event_resolver_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->resolver_timeout = NGX_CONF_UNSET_MSEC;

    return conf;
}

static char *
ngx_event_resolver_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_event_resolver_conf_t      *ercf = conf;

    ngx_conf_init_msec_value(ercf->resolver_timeout, 60000);

    return NGX_CONF_OK;
}

static char *
ngx_event_resolver(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_event_resolver_conf_t      *ercf = conf;

    ngx_str_t                      *value;

    if (ercf->resolver) {
        return "is duplicate";
    }

    value = cf->args->elts;

    ercf->resolver = ngx_resolver_create(cf, &value[1], cf->args->nelts - 1);
    if (ercf->resolver == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_event_resolver_ctx_t *
ngx_event_resolver_get_ctx()
{
    ngx_event_resolver_ctx_t       *ctx;
    ngx_event_resolver_conf_t      *ercf;

    ercf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_resolver_module);

    ctx = ercf->free_ctx;
    if (ctx == NULL) {
        ctx = ngx_pcalloc(ngx_cycle->pool, sizeof(ngx_event_resolver_ctx_t));
        ++ercf->nalloc;
    } else {
        ercf->free_ctx = ctx->next;
        ctx->next = NULL;
        --ercf->nfree;
    }

    return ctx;
}

static void
ngx_event_resolver_put_ctx(ngx_event_resolver_ctx_t *ctx)
{
    ngx_event_resolver_conf_t      *ercf;

    ercf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_resolver_module);

    ctx->next = ercf->free_ctx;
    ercf->free_ctx = ctx;
    ++ercf->nfree;
}

static void
ngx_event_resolver_handler(ngx_resolver_ctx_t *ctx)
{
    ngx_event_resolver_ctx_t       *erctx;

    erctx = ctx->data;

    if (ctx->state) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "event resolver, domain '%V' could not be resolved (%i: %s)",
                &ctx->name, ctx->state, ngx_resolver_strerror(ctx->state));
        erctx->handler(erctx->data, NULL, 0);

        goto failed;
    }

    erctx->handler(erctx->data, ctx->addrs, ctx->naddrs);

failed:
    ngx_resolve_name_done(ctx);
    ngx_event_resolver_put_ctx(erctx);
}

void
ngx_event_resolver_start_resolver(ngx_str_t *domain,
        ngx_event_resolver_handler_pt h, void *data)
{
    ngx_event_resolver_conf_t      *ercf;
    ngx_event_resolver_ctx_t       *erctx;
    ngx_resolver_ctx_t             *ctx, temp;

    ercf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_resolver_module);

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0, "event resolver, "
            "start resolv domain '%V'", domain);

    if (ercf->resolver == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "event resolver, "
                "resolver does not configured");
        h(data, NULL, 0);

        return;
    }

    temp.name = *domain;

    erctx = ngx_event_resolver_get_ctx();
    if (erctx == NULL) {
        return;
    }
    erctx->handler = h;
    erctx->data = data;

    ctx = ngx_resolve_start(ercf->resolver, &temp);
    if (ctx == NULL) {
        goto failed;
    }

    if (ctx == NGX_NO_RESOLVER) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "event resolver, "
                "no resolver defined to resolv %V", domain);
        goto failed;
    }

    ctx->name = *domain;
    ctx->handler = ngx_event_resolver_handler;
    ctx->data = erctx;
    ctx->timeout = ercf->resolver_timeout;

    if (ngx_resolve_name(ctx) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "event resolver, "
                "resolv %V failed", domain);

        goto failed;
    }

    return;

failed:
    h(data, NULL, 0);

    if (ctx == NULL || ctx == NGX_NO_RESOLVER) {
        ngx_resolve_name_done(ctx);
        ngx_event_resolver_put_ctx(erctx);
    }
}

ngx_chain_t *
ngx_event_resolver_state(ngx_http_request_t *r)
{
	ngx_event_resolver_conf_t  *ercf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len;

    ercf = ngx_event_get_conf(ngx_cycle->conf_ctx, ngx_event_resolver_module);

    len = sizeof("##########event resolver state##########\n") - 1
        + sizeof("ngx_event_resolver alloc: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_event_resolver free: \n") - 1 + NGX_OFF_T_LEN;

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
            "##########event resolver state##########\n"
            "ngx_event_resolver alloc: %ui\nngx_event_resolver free: %ui\n",
            ercf->nalloc, ercf->nfree);

    return cl;
}
