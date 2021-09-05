/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_live.h"


static void *ngx_live_create_conf(ngx_cycle_t *cf);
static char *ngx_live_init_conf(ngx_cycle_t *cycle, void *conf);


static ngx_command_t  ngx_live_commands[] = {

    { ngx_string("stream_buckets"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      0,
      offsetof(ngx_live_conf_t, stream_buckets),
      NULL },

    { ngx_string("server_buckets"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      0,
      offsetof(ngx_live_conf_t, server_buckets),
      NULL },

      ngx_null_command
};


static ngx_core_module_t  ngx_live_module_ctx = {
    ngx_string("live"),
    ngx_live_create_conf,           /* create conf */
    ngx_live_init_conf              /* init conf */
};


ngx_module_t  ngx_live_module = {
    NGX_MODULE_V1,
    &ngx_live_module_ctx,           /* module context */
    ngx_live_commands,              /* module directives */
    NGX_CORE_MODULE,                /* module type */
    NULL,                           /* init master */
    NULL,                           /* init module */
    NULL,                           /* init process */
    NULL,                           /* init thread */
    NULL,                           /* exit thread */
    NULL,                           /* exit process */
    NULL,                           /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_live_create_conf(ngx_cycle_t *cycle)
{
    ngx_live_conf_t            *lcf;

    lcf = ngx_pcalloc(cycle->pool, sizeof(ngx_live_conf_t));
    if (lcf == NULL) {
        return NULL;
    }

    lcf->stream_buckets = NGX_CONF_UNSET_SIZE;
    lcf->server_buckets = NGX_CONF_UNSET_SIZE;

    return lcf;
}

static char *
ngx_live_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_live_conf_t            *lcf = conf;

    lcf->pool = ngx_create_pool(4096, cycle->log);
    if (lcf->pool == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_conf_init_size_value(lcf->stream_buckets, 10007);
    ngx_conf_init_size_value(lcf->server_buckets, 1031);

    lcf->servers = ngx_pcalloc(lcf->pool,
            sizeof(ngx_live_server_t *) * lcf->server_buckets);
    if (lcf->servers == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ngx_live_server_t **
ngx_live_find_server(ngx_str_t *serverid)
{
    ngx_live_conf_t            *lcf;
    ngx_live_server_t         **psrv;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    psrv = &lcf->servers[ngx_hash_key(serverid->data, serverid->len)
                         % lcf->server_buckets];
    for (; *psrv; psrv = &(*psrv)->next) {
        if (ngx_strlen((*psrv)->serverid) == serverid->len &&
            ngx_memcmp((*psrv)->serverid, serverid->data, serverid->len) == 0)
        {
            break;
        }
    }

    return psrv;
}

static ngx_live_server_t *
ngx_live_get_server(ngx_str_t *serverid)
{
    ngx_live_conf_t            *lcf;
    ngx_live_server_t          *srv;

    if (serverid->len > NGX_LIVE_SERVERID_LEN - 1) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "serverid too long: %ui", serverid->len);
        return NULL;
    }

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    srv = lcf->free_server;
    if (srv == NULL) {
        srv = ngx_pcalloc(lcf->pool, sizeof(ngx_live_server_t));
        if (srv == NULL) {
            return NULL;
        }

        srv->streams = ngx_pcalloc(lcf->pool,
                sizeof(ngx_live_stream_t *) * lcf->stream_buckets);
        if (srv->streams == NULL) {
            return NULL;
        }

        ++lcf->alloc_server_count;
    } else {
        lcf->free_server = srv->next;
        --lcf->free_server_count;
    }

    *ngx_cpymem(srv->serverid, serverid->data, serverid->len) = 0;
    srv->deleted = 0;
    srv->n_stream = 0;

    return srv;
}

static void
ngx_live_put_server(ngx_live_server_t *server)
{
    ngx_live_conf_t            *lcf;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    server->next = lcf->free_server;
    lcf->free_server = server;
    ++lcf->free_server_count;
}

static ngx_live_stream_t **
ngx_live_find_stream(ngx_live_server_t *server, ngx_str_t *stream)
{
    ngx_live_conf_t            *lcf;
    ngx_live_stream_t         **pst;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    pst = &server->streams[ngx_hash_key(stream->data, stream->len)
                           % lcf->stream_buckets];
    for (; *pst; pst = &(*pst)->next) {
        if (ngx_strlen((*pst)->name) == stream->len &&
            ngx_memcmp((*pst)->name, stream->data, stream->len) == 0)
        {
            break;
        }
    }

    return pst;
}

static ngx_live_stream_t *
ngx_live_get_stream(ngx_str_t *stream)
{
    ngx_live_conf_t            *lcf;
    ngx_live_stream_t          *st;

    if (stream->len > NGX_LIVE_STREAM_LEN - 1) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "stream too long: %ui", stream->len);
        return NULL;
    }

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    st = lcf->free_stream;
    if (st == NULL) {
        st = ngx_pcalloc(lcf->pool, sizeof(ngx_live_stream_t));
        ++lcf->alloc_stream_count;
    } else {
        lcf->free_stream = st->next;
        --lcf->free_stream_count;
        ngx_memzero(st, sizeof(ngx_live_stream_t));
    }

    *ngx_cpymem(st->name, stream->data, stream->len) = 0;
    st->pslot = -1;
    st->epoch = ngx_current_msec;
    ngx_map_init(&st->pubctx, ngx_map_hash_int, ngx_cmp_int);

    return st;
}

static void
ngx_live_put_stream(ngx_live_stream_t *st)
{
    ngx_live_conf_t            *lcf;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);

    st->next = lcf->free_stream;
    lcf->free_stream = st;
    ++lcf->free_stream_count;
}

ngx_live_server_t *
ngx_live_create_server(ngx_str_t *serverid)
{
    ngx_live_server_t         **psrv;

    psrv = ngx_live_find_server(serverid);
    if (*psrv) {
        (*psrv)->deleted = 0;
        return *psrv;
    }

    *psrv = ngx_live_get_server(serverid);

    return *psrv;
}

ngx_live_server_t *
ngx_live_fetch_server(ngx_str_t *serverid)
{
    ngx_live_server_t         **psrv;

    psrv = ngx_live_find_server(serverid);

    return *psrv;
}

void
ngx_live_delete_server(ngx_str_t *serverid)
{
    ngx_live_server_t         **psrv, *srv;

    psrv = ngx_live_find_server(serverid);
    if (*psrv == NULL) {
        return;
    }

    if ((*psrv)->n_stream != 0) {
        (*psrv)->deleted = 1;
    }

    if ((*psrv)->n_stream == 0) {
        srv = *psrv;
        *psrv = srv->next;
        ngx_live_put_server(srv);
    }
}

ngx_live_stream_t *
ngx_live_create_stream(ngx_str_t *serverid, ngx_str_t *stream)
{
    ngx_live_server_t         **psrv;
    ngx_live_stream_t         **pst;

    psrv = ngx_live_find_server(serverid);
    if (*psrv == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "server %V does not exist when create stream", serverid);
        return NULL;
    }

    pst = ngx_live_find_stream(*psrv, stream);

    if (*pst) {
        return *pst;
    }

    *pst = ngx_live_get_stream(stream);
    ++(*psrv)->n_stream;

    return *pst;
}

ngx_live_stream_t *
ngx_live_fetch_stream(ngx_str_t *serverid, ngx_str_t *stream)
{
    ngx_live_server_t         **psrv;
    ngx_live_stream_t         **pst;

    psrv = ngx_live_find_server(serverid);
    if (*psrv == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "server %V does not exist when fetch stream", serverid);
        return NULL;
    }

    pst = ngx_live_find_stream(*psrv, stream);

    return *pst;
}

void
ngx_live_delete_stream(ngx_str_t *serverid, ngx_str_t *stream)
{
    ngx_live_server_t         **psrv;
    ngx_live_stream_t         **pst, *st;

    psrv = ngx_live_find_server(serverid);
    if (*psrv == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "server %V does not exist when delete stream", serverid);
        return;
    }

    pst = ngx_live_find_stream(*psrv, stream);

    if (*pst == NULL) {
        return;
    }

    st = *pst;

    *pst = st->next;
    ngx_live_put_stream(st);
    --(*psrv)->n_stream;

    if ((*psrv)->deleted && (*psrv)->n_stream == 0) {
        ngx_live_delete_server(serverid);
    }
}

void
ngx_live_create_ctx(ngx_rtmp_session_t *s, unsigned publishing)
{
    ngx_rtmp_core_ctx_t        *ctx, **pctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_core_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_core_ctx_t));
        if (ctx == NULL) {
            return;
        }

        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_core_module);
    }

    ctx->publishing = publishing;
    ctx->session = s;
    if (publishing) {
        pctx = &s->live_stream->publish_ctx;
    } else {
        pctx = &s->live_stream->play_ctx;
    }

    ctx->next = (*pctx);
    *pctx = ctx;
}

void
ngx_live_delete_ctx(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_ctx_t        *ctx, **pctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_core_module);
    if (ctx == NULL) {
        return;
    }

    if (ctx->publishing) {
        pctx = &s->live_stream->publish_ctx;
    } else {
        pctx = &s->live_stream->play_ctx;
    }

    for (/* void */; *pctx; pctx = &(*pctx)->next) {
        if (*pctx == ctx) {
            *pctx = ctx->next;
            return;
        }
    }
}


ngx_chain_t *
ngx_live_state(ngx_http_request_t *r)
{
    ngx_live_conf_t            *lcf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len;

    lcf = (ngx_live_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_live_module);


    len = sizeof("##########ngx live state##########\n") - 1
        + sizeof("ngx_live nalloc server: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_live nfree server: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_live nalloc stream: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_live nfree stream: \n") - 1 + NGX_OFF_T_LEN;

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
            "##########ngx live state##########\n"
            "ngx_live nalloc server: %ui\nngx_live nfree server: %ui\n"
            "ngx_live nalloc stream: %ui\nngx_live nfree stream: %ui\n",
            lcf->alloc_server_count, lcf->free_server_count,
            lcf->alloc_stream_count, lcf->free_stream_count);

    return cl;
}
