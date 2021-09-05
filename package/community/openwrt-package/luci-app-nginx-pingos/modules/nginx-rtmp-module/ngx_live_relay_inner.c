/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_live_relay.h"
#include "ngx_stream_zone_module.h"
#include "ngx_multiport.h"


static ngx_live_push_pt                 next_push;
static ngx_live_pull_pt                 next_pull;
static ngx_live_push_close_pt           next_push_close;
static ngx_live_pull_close_pt           next_pull_close;


static void *ngx_live_relay_inner_create_app_conf(ngx_conf_t *cf);
static char *ngx_live_relay_inner_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_live_relay_inner_postconfiguration(ngx_conf_t *cf);


typedef struct {
    ngx_flag_t                          inner_pull;
    ngx_str_t                           inner_pull_port;
} ngx_live_relay_inner_app_conf_t;


static ngx_command_t  ngx_live_relay_inner_commands[] = {

    { ngx_string("rtmp_auto_pull"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_relay_inner_app_conf_t, inner_pull),
      NULL },

    { ngx_string("rtmp_auto_pull_port"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_relay_inner_app_conf_t, inner_pull_port),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_relay_inner_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_relay_inner_postconfiguration, /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_live_relay_inner_create_app_conf,   /* create app configuration */
    ngx_live_relay_inner_merge_app_conf     /* merge app configuration */
};


ngx_module_t  ngx_live_relay_inner_module = {
    NGX_MODULE_V1,
    &ngx_live_relay_inner_module_ctx,       /* module context */
    ngx_live_relay_inner_commands,          /* module directives */
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
ngx_live_relay_inner_create_app_conf(ngx_conf_t *cf)
{
    ngx_live_relay_inner_app_conf_t    *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_inner_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->inner_pull = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_live_relay_inner_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_live_relay_inner_app_conf_t    *prev = parent;
    ngx_live_relay_inner_app_conf_t    *conf = child;

    ngx_conf_merge_value(conf->inner_pull, prev->inner_pull, 1);
    ngx_conf_merge_str_value(conf->inner_pull_port, prev->inner_pull_port,
                             "unix:/tmp/inner.sock");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_live_relay_inner_create_relay(ngx_rtmp_session_t *rs,
        ngx_live_relay_t *relay, ngx_int_t pslot)
{
    ngx_live_relay_inner_app_conf_t    *riacf;
    ngx_live_relay_url_t               *url;
    ngx_str_t                           port;

    riacf = ngx_rtmp_get_module_app_conf(rs, ngx_live_relay_inner_module);

    ngx_memzero(relay, sizeof(ngx_live_relay_t));
    if (ngx_array_init(&relay->urls, rs->pool, 1, sizeof(ngx_live_relay_url_t))
            != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "inner relay, relay init error: %V", &riacf->inner_pull_port);
        return NGX_ERROR;
    }

    url = ngx_array_push(&relay->urls);
    if (url == NULL) {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "inner relay, get url failed: %V", &riacf->inner_pull_port);
        return NGX_ERROR;
    }
    relay->tag = &ngx_live_relay_inner_module;

    ngx_memzero(url, sizeof(ngx_live_relay_url_t));
    ngx_memzero(&port, sizeof(ngx_str_t));

    if (ngx_multiport_get_port(rs->pool, &port,
            &riacf->inner_pull_port, pslot) == NGX_ERROR)
    {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "inner relay, get mulitport error: %V",
                &riacf->inner_pull_port);
        return NGX_ERROR;
    }

    url->url.host = port;
    url->url.host_with_port = port;
    url->relay_type = NGX_LIVE_RELAY_RTMP;

    return NGX_OK;
}


static void
ngx_live_relay_inner_handler(ngx_event_t *ev)
{
    ngx_rtmp_session_t                 *s;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_live_relay_ctx_t               *ctx;
    ngx_live_relay_t                    relay;

    s = ev->data;
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    if (!ctx->failed_delay && ev->timedout) { // connect timeout
        ngx_log_error(NGX_LOG_ERR, s->log, NGX_ETIMEDOUT,
                "inner relay, relay timeout");
        s->finalize_reason = NGX_LIVE_RELAY_TIMEOUT;
        ngx_rtmp_finalize_session(s);

        return;
    }

    // relay pull, no player or relay push no publisher
    if ((s->publishing && s->live_stream->play_ctx == NULL)
            || (!s->publishing && s->live_stream->publish_ctx == NULL))
    {
        return;
    }

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_module);

    ngx_add_timer(&ctx->reconnect, lracf->relay_reconnect);

    if (ngx_live_relay_inner_create_relay(s, &relay, s->live_stream->pslot)
            != NGX_OK)
    {
        return;
    }

    ngx_live_relay_create(s, &relay);
}


// if stream's need to continue pull or push chain, otherwise return NGX_OK
static ngx_int_t
ngx_live_relay_inner_relay(ngx_rtmp_session_t *s, unsigned publishing)
{
    ngx_rtmp_session_t                 *rs;
    ngx_live_relay_ctx_t               *ctx, *pctx;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_int_t                           pslot;

    pslot = ngx_stream_zone_insert_stream(&s->stream);
    if (pslot == NGX_ERROR) { // stream zone not configured or configured error
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "inner relay, insert stream %V failed", &s->stream);
        return NGX_DECLINED;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0,
            "inner relay, stream %V not in current process, "
            "pslot:%i ngx_process_slot:%i",
            &s->stream, pslot, ngx_process_slot);

    s->live_stream->pslot = pslot;
    if (pslot == ngx_process_slot) { // current process become stream owner
        return NGX_DECLINED;
    }

    rs = ngx_rtmp_create_relay_session(s, &ngx_live_relay_inner_module);
    if (rs == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "inner relay, create relay session failed");
        return NGX_DECLINED;
    }
    rs->publishing = publishing;
    rs->live_stream = s->live_stream;
    ngx_live_create_ctx(rs, publishing);

    ctx = ngx_rtmp_get_module_ctx(rs, ngx_live_relay_module);
    ctx->reconnect.log = rs->log;
    ctx->reconnect.data = rs;
    ctx->reconnect.handler = ngx_live_relay_inner_handler;

    lracf = ngx_rtmp_get_module_app_conf(rs, ngx_live_relay_module);

    // play trigger pull or publish trigger push
    if (s->publishing != rs->publishing) {
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    // normal publisher close, need to trigger pull
    if (s->publishing && !s->relay) {
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    // reconnect
    pctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (pctx->successd) { // prev relay susccessd
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    if (!pctx->reconnect.timer_set) { // prev relay timeout
        ctx->failed_reconnect = ngx_min(pctx->failed_reconnect * 2,
                lracf->relay_reconnect);
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    if (pctx->failed_reconnect) {
        ctx->failed_reconnect = ngx_min(pctx->failed_reconnect * 2,
                lracf->relay_reconnect);
    } else {
        ctx->failed_reconnect = lracf->failed_reconnect;
    }

    ctx->failed_delay = 1;
    ngx_add_timer(&ctx->reconnect, ctx->failed_reconnect);

    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_inner_push(ngx_rtmp_session_t *s)
{
    ngx_live_relay_inner_app_conf_t      *riacf;

    riacf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_inner_module);

    if (!riacf->inner_pull) {
        goto next;
    }

    if (s->relay) {
        goto next;
    }

    if (ngx_live_relay_inner_relay(s, 0) == NGX_OK) {
        return NGX_OK;
    }

next:
    return next_push(s);
}


static ngx_int_t
ngx_live_relay_inner_pull(ngx_rtmp_session_t *s)
{
    ngx_live_relay_inner_app_conf_t      *riacf;

    riacf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_inner_module);

    if (!riacf->inner_pull) {
        goto next;
    }

    if (!s->publishing && s->relay) { // relay push
        goto next;
    }

    if (s->live_stream->pslot != -1) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "inner relay, stream %V already in current process",
                &s->stream);
        if (s->live_stream->pslot == ngx_process_slot) {
            // stream leader is in current process, continue pull or push chain
            goto next;
        }

        return NGX_OK;
    }

    if (ngx_live_relay_inner_relay(s, 1) == NGX_OK) {
        return NGX_OK;
    }

next:
    return next_pull(s);
}


static ngx_int_t
ngx_live_relay_inner_push_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx->tag != &ngx_live_relay_inner_module) {
        goto next;
    }

    // relay push and has publisher in stream
    if (!ctx->giveup && s->live_stream->publish_ctx) {
        ngx_live_relay_inner_relay(s, 0);
    }

next:
    return next_push_close(s);
}


static ngx_int_t
ngx_live_relay_inner_pull_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->tag != &ngx_live_relay_inner_module) {
        goto next;
    }

    // inner relay close
    s->live_stream->pslot = -1;

next:
    return next_pull_close(s);
}


static ngx_int_t
ngx_live_relay_inner_postconfiguration(ngx_conf_t *cf)
{
    /* chain handlers */

    next_push = ngx_live_push;
    ngx_live_push = ngx_live_relay_inner_push;

    next_pull = ngx_live_pull;
    ngx_live_pull = ngx_live_relay_inner_pull;

    next_push_close = ngx_live_push_close;
    ngx_live_push_close = ngx_live_relay_inner_push_close;

    next_pull_close = ngx_live_pull_close;
    ngx_live_pull_close = ngx_live_relay_inner_pull_close;

    return NGX_OK;
}
