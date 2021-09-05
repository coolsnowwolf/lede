/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>


static ngx_int_t ngx_process_slot_module_init(ngx_cycle_t *cycle);
static ngx_int_t ngx_process_slot_process_init(ngx_cycle_t *cycle);
static void ngx_process_slot_process_exit(ngx_cycle_t *cycle);

static void *ngx_process_slot_module_create_conf(ngx_cycle_t *cycle);
static char *ngx_process_slot_module_init_conf(ngx_cycle_t *cycle, void *conf);


#define MAX_PROCESSES  128

typedef struct {
    ngx_atomic_int_t            process_slot[MAX_PROCESSES];
} ngx_process_slot_ctx_t;

typedef struct {
    ngx_process_slot_ctx_t     *ctx;
} ngx_process_slot_conf_t;


static ngx_command_t  ngx_process_slot_commands[] = {

      ngx_null_command
};


static ngx_core_module_t  ngx_process_slot_module_ctx = {
    ngx_string("process_slot"),
    ngx_process_slot_module_create_conf,
    ngx_process_slot_module_init_conf
};


ngx_module_t  ngx_process_slot_module = {
    NGX_MODULE_V1,
    &ngx_process_slot_module_ctx,          /* module context */
    ngx_process_slot_commands,             /* module directives */
    NGX_CORE_MODULE,                       /* module type */
    NULL,                                  /* init master */
    ngx_process_slot_module_init,          /* init module */
    ngx_process_slot_process_init,         /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    ngx_process_slot_process_exit,         /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_process_slot_module_create_conf(ngx_cycle_t *cycle)
{
    ngx_process_slot_conf_t    *pscf;

    pscf = ngx_palloc(cycle->pool, sizeof(ngx_process_slot_conf_t));
    if (pscf == NULL) {
        return NULL;
    }

    return pscf;
}

static char *
ngx_process_slot_module_init_conf(ngx_cycle_t *cycle, void *conf)
{
    return NGX_CONF_OK;
}

static ngx_int_t
ngx_process_slot_module_init(ngx_cycle_t *cycle)
{
    ngx_process_slot_conf_t    *pscf;
    ngx_shm_t                   shm;
    ngx_uint_t                  i;

    pscf = (ngx_process_slot_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                    ngx_process_slot_module);

    shm.size = sizeof(ngx_process_slot_ctx_t);
    shm.name.len = sizeof("process_slot_zone") - 1;
    shm.name.data = (u_char *) "process_slot_zone";
    shm.log = cycle->log;

    if (ngx_shm_alloc(&shm) != NGX_OK) {
        return NGX_ERROR;
    }

    pscf->ctx = (ngx_process_slot_ctx_t *) shm.addr;

    for (i = 0; i < MAX_PROCESSES; ++i) {
        pscf->ctx->process_slot[i] = -1;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_process_slot_process_init(ngx_cycle_t *cycle)
{
    ngx_process_slot_conf_t    *pscf;
    ngx_process_slot_ctx_t     *ctx;

    if (ngx_process != NGX_PROCESS_WORKER) {
        return NGX_OK;
    }

    pscf = (ngx_process_slot_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                    ngx_process_slot_module);
    ctx = pscf->ctx;

    for (;;) {
        if (ngx_atomic_cmp_set((ngx_atomic_t *) &ctx->process_slot[ngx_worker],
            (ngx_atomic_uint_t)ctx->process_slot[ngx_worker], ngx_process_slot))
        {
            break;
        }
    }

    return NGX_OK;
}

static void
ngx_process_slot_process_exit(ngx_cycle_t *cycle)
{
    ngx_process_slot_conf_t    *pscf;
    ngx_process_slot_ctx_t     *ctx;

    if (ngx_process != NGX_PROCESS_WORKER) {
        return;
    }

    pscf = (ngx_process_slot_conf_t *) ngx_get_conf(cycle->conf_ctx,
                                                    ngx_process_slot_module);
    ctx = pscf->ctx;

    ngx_atomic_cmp_set((ngx_atomic_t *) &ctx->process_slot[ngx_worker],
            (ngx_atomic_uint_t)ngx_process_slot, -1);
}

ngx_int_t
ngx_multiport_get_slot(ngx_uint_t wpid)
{
    ngx_process_slot_conf_t    *pscf;
    ngx_process_slot_ctx_t     *ctx;
    ngx_core_conf_t            *ccf;

    ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_core_module);

    if (wpid >= (ngx_uint_t)ccf->worker_processes) {
        return -1;
    }

    pscf = (ngx_process_slot_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                    ngx_process_slot_module);
    ctx = pscf->ctx;

    return ctx->process_slot[wpid];
}
