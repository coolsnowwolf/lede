/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_live_relay.h"
#include "ngx_rtmp_dynamic.h"
#include "ngx_dynamic_resolver.h"


static ngx_live_push_pt                 next_push;
static ngx_live_pull_pt                 next_pull;
static ngx_live_push_close_pt           next_push_close;


static void *ngx_live_relay_simple_create_app_conf(ngx_conf_t *cf);
static char *ngx_live_relay_simple_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_live_relay_simple_postconfiguration(ngx_conf_t *cf);

static char *ngx_live_relay_push(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_live_relay_pull(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);


typedef struct {
    ngx_live_relay_t                    pull;
    ngx_array_t                         pushes; /* ngx_live_relay_t */
} ngx_live_relay_simple_app_conf_t;


typedef struct {
    ngx_live_relay_t                   *relay;
} ngx_live_relay_simple_ctx_t;


static ngx_command_t  ngx_live_relay_simple_commands[] = {

    { ngx_string("push"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_live_relay_push,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("pull"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_live_relay_pull,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_relay_simple_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_relay_simple_postconfiguration,/* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_live_relay_simple_create_app_conf,  /* create app configuration */
    ngx_live_relay_simple_merge_app_conf    /* merge app configuration */
};


ngx_module_t  ngx_live_relay_simple_module = {
    NGX_MODULE_V1,
    &ngx_live_relay_simple_module_ctx,      /* module context */
    ngx_live_relay_simple_commands,         /* module directives */
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
ngx_live_relay_simple_create_app_conf(ngx_conf_t *cf)
{
    ngx_live_relay_simple_app_conf_t   *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_simple_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&conf->pushes, cf->pool, 8, sizeof(ngx_live_relay_t))
            != NGX_OK)
    {
        return NULL;
    }

    return conf;
}


static char *
ngx_live_relay_simple_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    return NGX_CONF_OK;
}


static void
ngx_live_relay_simple_handler(ngx_event_t *ev)
{
    ngx_rtmp_session_t                 *s;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_live_relay_ctx_t               *ctx;
    ngx_live_relay_simple_ctx_t        *sctx;
    ngx_live_relay_t                   *relay;

    s = ev->data;
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    if (!ctx->failed_delay && ev->timedout) { // connect timeout
        ngx_log_error(NGX_LOG_ERR, s->log, NGX_ETIMEDOUT,
                "simple relay, relay timeout");
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

    sctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_simple_module);
    relay = sctx->relay;

    ngx_live_relay_create(s, relay);
}


static ngx_int_t
ngx_live_relay_simple_relay(ngx_rtmp_session_t *s, ngx_live_relay_t *relay,
        unsigned publishing)
{
    ngx_rtmp_session_t                 *rs;
    ngx_live_relay_ctx_t               *ctx, *pctx;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_live_relay_simple_ctx_t        *sctx;

    rs = ngx_rtmp_create_relay_session(s, &ngx_live_relay_simple_module);
    if (rs == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "relay simple, create relay session failed");
        return NGX_DECLINED;
    }
    rs->publishing = publishing;
    rs->live_stream = s->live_stream;
    ngx_live_create_ctx(rs, publishing);

    sctx = ngx_pcalloc(rs->pool, sizeof(ngx_live_relay_simple_ctx_t));
    if (sctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "relay simple, create simple relay ctx failed");
        ngx_rtmp_finalize_session(rs);

        return NGX_OK;
    }
    ngx_rtmp_set_ctx(rs, sctx, ngx_live_relay_simple_module);
    sctx->relay = relay;

    ctx = ngx_rtmp_get_module_ctx(rs, ngx_live_relay_module);
    ctx->reconnect.log = rs->log;
    ctx->reconnect.data = rs;
    ctx->reconnect.handler = ngx_live_relay_simple_handler;

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
    if (pctx->successd) { // prev relay successd
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    ctx->idx = pctx->idx;
    ctx->failed_reconnect = pctx->failed_reconnect;

    if (ctx->idx < relay->urls.nelts) { // retry backup url immediately
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    lracf = ngx_rtmp_get_module_app_conf(rs, ngx_live_relay_module);

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
ngx_live_relay_simple_push(ngx_rtmp_session_t *s)
{
    ngx_live_relay_simple_app_conf_t   *rsacf;
    ngx_live_relay_t                   *relay;
    ngx_uint_t                          i;

    rsacf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_simple_module);

    if (rsacf->pushes.nelts == 0) { // not configured
        goto next;
    }

    if (s->relay && s->static_pull == 0) {
        goto next;
    }

    relay = rsacf->pushes.elts;
    for (i = 0; i < rsacf->pushes.nelts; ++i, ++relay) {
        ngx_live_relay_simple_relay(s, relay, 0);
    }

next:
    return next_push(s);
}


static ngx_int_t
ngx_live_relay_simple_pull(ngx_rtmp_session_t *s)
{
    ngx_live_relay_simple_app_conf_t   *rsacf;
    ngx_live_relay_t                   *relay;

    rsacf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_simple_module);

    if (rsacf->pull.urls.nelts == 0) { // not configured
        goto next;
    }

    if (!s->publishing && s->relay) { // relay push
        goto next;
    }

    if (s->live_stream->publish_ctx) { // already has publisher
        goto next;
    }

    relay = &rsacf->pull;

    if (ngx_live_relay_simple_relay(s, relay, 1) == NGX_OK) {
        return NGX_OK;
    }

next:
    return next_pull(s);
}


static ngx_int_t
ngx_live_relay_simple_push_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t               *ctx;
    ngx_live_relay_simple_ctx_t        *sctx;
    ngx_live_relay_t                   *relay;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx->tag != &ngx_live_relay_simple_module) {
        goto next;
    }

    // relay push and has publisher in stream
    if (!ctx->giveup && s->live_stream->publish_ctx) {
        sctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_simple_module);
        relay = sctx->relay;

        ngx_live_relay_simple_relay(s, relay, 0);
    }

next:
    return next_push_close(s);
}


static char *
ngx_live_relay_push_pull(ngx_conf_t *cf, ngx_live_relay_t *relay)
{
    ngx_live_relay_url_t               *url;
    ngx_str_t                          *value, n, v;
    ngx_uint_t                          i;
    u_char                             *p;

    relay->tag = &ngx_live_relay_simple_module;

    if (ngx_array_init(&relay->urls, cf->pool, 8, sizeof(ngx_live_relay_url_t))
            != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    value = cf->args->elts;
    ++value;

    for (i = 1; i < cf->args->nelts; ++i, ++value) {
        if (ngx_strncasecmp(value->data, (u_char *) "rtmp://", 7) == 0
            || ngx_strncasecmp(value->data, (u_char *) "http://", 7) == 0)
        {
            url = ngx_array_push(&relay->urls);
            if (url == NULL) {
                return NGX_CONF_ERROR;
            }
            ngx_memzero(url, sizeof(ngx_live_relay_url_t));

            if (value->data[0] == 'h') {
                url->relay_type = NGX_LIVE_RELAY_HTTPFLV;
            } else {
                url->relay_type = NGX_LIVE_RELAY_RTMP;
            }

            if (ngx_parse_request_url(&url->url, value) != NGX_OK) {
                return NGX_CONF_ERROR;
            }

            url->port = ngx_request_port(&url->url.scheme, &url->url.port);
            if (url->port == 0) {
                return "invalid port";
            }

            ngx_dynamic_resolver_add_domain(&url->url.host, cf->cycle);

            continue;
        }

        p = ngx_strlchr(value->data, value->data + value->len, '=');

        if (p == NULL) {
            return "unsupported parameter format";
        } else {
            n.data = value->data;
            n.len = p - value->data;

            v.data = p + 1;
            v.len = value->data + value->len - v.data;
        }

#define NGX_LIVE_RELAY_STR_PAR(name, var)                               \
        if (n.len == sizeof(name) - 1                                   \
            && ngx_strncasecmp(n.data, (u_char *) name, n.len) == 0)    \
        {                                                               \
            relay->var = v;                                             \
            continue;                                                   \
        }

        NGX_LIVE_RELAY_STR_PAR("domain",        domain);
        NGX_LIVE_RELAY_STR_PAR("app",           app);
        NGX_LIVE_RELAY_STR_PAR("name",          name);
        NGX_LIVE_RELAY_STR_PAR("pargs",         pargs);
        NGX_LIVE_RELAY_STR_PAR("referer",       referer);
        NGX_LIVE_RELAY_STR_PAR("user_agent",    user_agent);

#undef NGX_LIVE_RELAY_STR_PAR

        return "unsupported parameter";
    }

    return NGX_CONF_OK;
}


static char *
ngx_live_relay_push(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_live_relay_simple_app_conf_t   *rsacf;
    ngx_live_relay_t                   *relay;
    char                               *rc;

    rsacf = conf;

    relay = ngx_array_push(&rsacf->pushes);
    if (relay == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_memzero(relay, sizeof(ngx_live_relay_t));

    rc = ngx_live_relay_push_pull(cf, relay);
    if (rc != NGX_CONF_OK) {
        return rc;
    }

    return NGX_CONF_OK;
}


static char *
ngx_live_relay_pull(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_live_relay_simple_app_conf_t   *rsacf;

    rsacf = conf;

    if (rsacf->pull.urls.nelts) {
        return "is duplicate";
    }

    return ngx_live_relay_push_pull(cf, &rsacf->pull);
}


static ngx_int_t
ngx_live_relay_simple_postconfiguration(ngx_conf_t *cf)
{
    /* chain handlers */

    next_push = ngx_live_push;
    ngx_live_push = ngx_live_relay_simple_push;

    next_pull = ngx_live_pull;
    ngx_live_pull = ngx_live_relay_simple_pull;

    next_push_close = ngx_live_push_close;
    ngx_live_push_close = ngx_live_relay_simple_push_close;

    return NGX_OK;
}
