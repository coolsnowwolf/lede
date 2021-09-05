
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"


typedef struct {
    ngx_int_t       max_conn;
    ngx_shm_zone_t *shm_zone;
} ngx_rtmp_limit_main_conf_t;


static ngx_str_t    shm_name = ngx_string("rtmp_limit");


static ngx_int_t ngx_rtmp_limit_postconfiguration(ngx_conf_t *cf);
static void *ngx_rtmp_limit_create_main_conf(ngx_conf_t *cf);


static ngx_command_t  ngx_rtmp_limit_commands[] = {

    { ngx_string("max_connections"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_MAIN_CONF_OFFSET,
      offsetof(ngx_rtmp_limit_main_conf_t, max_conn),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_limit_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_limit_postconfiguration,       /* postconfiguration */
    ngx_rtmp_limit_create_main_conf,        /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    NULL,                                   /* create app configuration */
    NULL                                    /* merge app configuration */
};


ngx_module_t  ngx_rtmp_limit_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_limit_module_ctx,             /* module context */
    ngx_rtmp_limit_commands,                /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
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
ngx_rtmp_limit_create_main_conf(ngx_conf_t *cf)
{
    ngx_rtmp_limit_main_conf_t      *lmcf;

    lmcf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_limit_main_conf_t));
    if (lmcf == NULL) {
        return NULL;
    }

    lmcf->max_conn = NGX_CONF_UNSET;

    return lmcf;
}


static ngx_int_t
ngx_rtmp_limit_connect(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    ngx_rtmp_limit_main_conf_t *lmcf;
    ngx_slab_pool_t            *shpool;
    ngx_shm_zone_t             *shm_zone;
    uint32_t                   *nconn, n;
    ngx_int_t                   rc;

    lmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_limit_module);
    if (lmcf->max_conn == NGX_CONF_UNSET) {
        return NGX_OK;
    }

    shm_zone = lmcf->shm_zone;
    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;
    nconn = shm_zone->data;

    ngx_shmtx_lock(&shpool->mutex);
    n = ++*nconn;
    ngx_shmtx_unlock(&shpool->mutex);

    rc = n > (ngx_uint_t) lmcf->max_conn ? NGX_ERROR : NGX_OK;

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "limit: inc conection counter: %uD", n);

    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "limit: too many connections: %uD > %i",
                      n, lmcf->max_conn);
    }

    return rc;
}


static ngx_int_t
ngx_rtmp_limit_disconnect(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    ngx_rtmp_limit_main_conf_t *lmcf;
    ngx_slab_pool_t            *shpool;
    ngx_shm_zone_t             *shm_zone;
    uint32_t                   *nconn, n;

    lmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_limit_module);
    if (lmcf->max_conn == NGX_CONF_UNSET) {
        return NGX_OK;
    }

    shm_zone = lmcf->shm_zone;
    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;
    nconn = shm_zone->data;

    ngx_shmtx_lock(&shpool->mutex);
    n = --*nconn;
    ngx_shmtx_unlock(&shpool->mutex);

    (void) n;
    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "limit: dec conection counter: %uD", n);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_limit_shm_init(ngx_shm_zone_t *shm_zone, void *data)
{
    ngx_slab_pool_t    *shpool;
    uint32_t           *nconn;

    if (data) {
        shm_zone->data = data;
        return NGX_OK;
    }

    shpool = (ngx_slab_pool_t *) shm_zone->shm.addr;

    nconn = ngx_slab_alloc(shpool, 4);
    if (nconn == NULL) {
        return NGX_ERROR;
    }

    *nconn = 0;

    shm_zone->data = nconn;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_limit_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_rtmp_limit_main_conf_t *lmcf;
    ngx_rtmp_handler_pt        *h;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_CONNECT]);
    *h = ngx_rtmp_limit_connect;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_DISCONNECT]);
    *h = ngx_rtmp_limit_disconnect;

    lmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_limit_module);
    if (lmcf->max_conn == NGX_CONF_UNSET) {
        return NGX_OK;
    }

    lmcf->shm_zone = ngx_shared_memory_add(cf, &shm_name, ngx_pagesize * 2,
                                           &ngx_rtmp_limit_module);
    if (lmcf->shm_zone == NULL) {
        return NGX_ERROR;
    }

    lmcf->shm_zone->init = ngx_rtmp_limit_shm_init;

    return NGX_OK;
}
