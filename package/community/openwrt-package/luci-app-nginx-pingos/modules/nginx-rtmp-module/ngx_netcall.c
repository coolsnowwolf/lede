/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_netcall.h"
#include "ngx_poold.h"


// cleanup only be called when connect failed(exclusive timeout)
static void
ngx_netcall_cleanup(void *data)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = data;

    if (nctx->ev.timer_set) {
        ngx_del_timer(&nctx->ev);
    }

    if (nctx->ev.posted) {
        ngx_delete_posted_event(&nctx->ev);
    }

    if (nctx->hcr) {
        ngx_http_client_detach(nctx->hcr);
        nctx->handler(nctx, NGX_ERROR);
        nctx->hcr = NULL;
    }
}

// netcall timeout
static void
ngx_netcall_timeout(ngx_event_t *ev)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = ev->data;

    if (nctx->ev.timer_set) {
        ngx_del_timer(&nctx->ev);
    }

    if (nctx->ev.posted) {
        ngx_delete_posted_event(&nctx->ev);
    }

    if (nctx->hcr) {
        ngx_http_client_detach(nctx->hcr);
        nctx->handler(nctx, NGX_ERROR);
        nctx->hcr = NULL;
    }
}

static void
ngx_netcall_handler(void *data, ngx_http_request_t *hcr)
{
    ngx_netcall_ctx_t          *nctx;
    ngx_int_t                   code;

    nctx = data;

    if (nctx->ev.timer_set) {
        ngx_del_timer(&nctx->ev);
    }

    if (nctx->ev.posted) {
        ngx_delete_posted_event(&nctx->ev);
    }

    code = ngx_http_client_status_code(hcr);

    if (nctx->hcr) {
        ngx_http_client_detach(nctx->hcr);
        nctx->handler(nctx, code);
        nctx->hcr = NULL;
    }
}

static void
ngx_netcall_destroy_handler(ngx_event_t *ev)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = ev->data;

    NGX_DESTROY_POOL(nctx->pool);
}

ngx_netcall_ctx_t *
ngx_netcall_create_ctx(ngx_uint_t type, ngx_str_t *groupid, ngx_uint_t stage,
        ngx_msec_t timeout, ngx_msec_t update, ngx_uint_t idx)
{
    ngx_netcall_ctx_t          *ctx;
    ngx_pool_t                 *pool;

    pool = NGX_CREATE_POOL(4096, ngx_cycle->log);
    if (pool == NULL) {
        return NULL;
    }

    ctx = ngx_pcalloc(pool, sizeof(ngx_netcall_ctx_t));
    if (ctx == NULL) {
        NGX_DESTROY_POOL(pool);
        return NULL;
    }

    ctx->url.data = ngx_pcalloc(pool, NGX_NETCALL_MAX_URL_LEN);
    if (ctx->url.data == NULL) {
        NGX_DESTROY_POOL(pool);
        return NULL;
    }

    ctx->pool = pool;

    ctx->idx = idx;
    ctx->type = type;

    ctx->groupid.len = groupid->len;
    ctx->groupid.data = ngx_pcalloc(pool, ctx->groupid.len);
    if (ctx->groupid.data == NULL) {
        NGX_DESTROY_POOL(pool);
        return NULL;
    }
    ngx_memcpy(ctx->groupid.data, groupid->data, groupid->len);

    ctx->ev.log = ngx_cycle->log;
    ctx->ev.data = ctx;

    ctx->stage = stage;
    ctx->timeout = timeout;
    ctx->update = update;

    return ctx;
}

void
ngx_netcall_create(ngx_netcall_ctx_t *nctx, ngx_log_t *log)
{
    ngx_http_request_t         *hcr;
    ngx_http_cleanup_t         *cln;

    hcr = ngx_http_client_get(log, &nctx->url, NULL, nctx);
    if (hcr == NULL) {
        return;
    }

    ngx_http_client_set_read_handler(hcr, ngx_netcall_handler);

    cln = ngx_http_client_cleanup_add(hcr, 0);
    if (cln == NULL) {
        ngx_log_error(NGX_LOG_ERR, log, 0,
                "netcall create add cleanup failed");
        return;
    }
    cln->handler = ngx_netcall_cleanup;
    cln->data = nctx;

    // detach old http client request
    if (nctx->hcr) {
        ngx_http_client_detach(nctx->hcr);
    }

    nctx->hcr = hcr;

    nctx->ev.log = log;
    nctx->ev.handler = ngx_netcall_timeout;
    ngx_add_timer(&nctx->ev, nctx->timeout);
}

void
ngx_netcall_destroy(ngx_netcall_ctx_t *nctx)
{
    if (nctx->ev.timer_set) {
        ngx_del_timer(&nctx->ev);
    }

    if (nctx->ev.posted) {
        ngx_delete_posted_event(&nctx->ev);
    }

    if (nctx->hcr) { // use detach will keep client connection alive
        ngx_http_client_detach(nctx->hcr);
        nctx->hcr = NULL;
    }

    // destroy may called in nctx->handler
    // destroy pool may cause memory error
    // so we destroy nctx pool asynchronous
    nctx->ev.handler = ngx_netcall_destroy_handler;
    // reset ev log, use rtmp session log may be destroy
    nctx->ev.log = ngx_cycle->log;
    ngx_post_event(&nctx->ev, &ngx_posted_events);
}

ngx_str_t *
ngx_netcall_header(ngx_netcall_ctx_t *nctx, ngx_str_t *key)
{
    ngx_http_request_t         *hcr;

    hcr = nctx->hcr;

    return ngx_http_client_header_in(hcr, key);
}
