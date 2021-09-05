/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_live_relay.h"
#include "ngx_map.h"
#include "ngx_dynamic_conf.h"
#include "ngx_rtmp_dynamic.h"
#include "ngx_dynamic_resolver.h"

static ngx_live_pull_pt                 next_pull;


static void *ngx_live_relay_static_create_main_conf(ngx_conf_t *cf);
static char *ngx_live_relay_static_init_main_conf(ngx_conf_t *cf, void *conf);

static void *ngx_live_relay_static_create_main_dconf(ngx_conf_t *cf);
static char *ngx_live_relay_static_init_main_dconf(ngx_conf_t *cf, void *conf);

static char *ngx_live_relay_pull(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);

static ngx_int_t ngx_live_relay_static_postconfiguration(ngx_conf_t *cf);


typedef struct ngx_live_relay_static_relay_s  ngx_live_relay_static_relay_t;

struct ngx_live_relay_static_relay_s {
    ngx_map_node_t                      node;
    ngx_live_relay_t                   *relay;
    ngx_rtmp_session_t                 *session;
    ngx_live_relay_static_relay_t      *next;
};


typedef struct {
    /* ngx_live_relay_static_relay_t */
    ngx_map_t                           pulls[2];
    /* 0 and 1 for index of conf */
    unsigned                            used;
    ngx_str_t                           pull_port;

    ngx_live_relay_static_relay_t      *free;
    unsigned                            nalloc;
    unsigned                            nfree;
} ngx_live_relay_static_main_conf_t;


typedef struct {
    ngx_array_t                         pulls; /* ngx_live_relay_t */
} ngx_live_relay_static_main_dconf_t;


typedef struct {
    ngx_live_relay_static_relay_t      *relay;
} ngx_live_relay_static_ctx_t;


static ngx_command_t  ngx_live_relay_static_dcommands[] = {

    { ngx_string("static_pull"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_1MORE,
      ngx_live_relay_pull,
      NGX_RTMP_MAIN_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_rtmp_dynamic_module_t  ngx_live_relay_static_module_dctx = {
    ngx_live_relay_static_create_main_dconf,/* create main configuration */
    ngx_live_relay_static_init_main_dconf,  /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create app configuration */
    NULL                                    /* merge app configuration */
};


static ngx_command_t  ngx_live_relay_static_commands[] = {

    { ngx_string("static_pull_port"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_MAIN_CONF_OFFSET,
      offsetof(ngx_live_relay_static_main_conf_t, pull_port),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_relay_static_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_relay_static_postconfiguration,/* postconfiguration */
    ngx_live_relay_static_create_main_conf, /* create main configuration */
    ngx_live_relay_static_init_main_conf,   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    NULL,                                   /* create app configuration */
    NULL                                    /* merge app configuration */
};


ngx_module_t  ngx_live_relay_static_module = {
    NGX_MODULE_V1,
    &ngx_live_relay_static_module_ctx,      /* module context */
    ngx_live_relay_static_commands,         /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_live_relay_static_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_live_relay_static_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void
ngx_live_relay_static_handler(ngx_event_t *ev)
{
    ngx_rtmp_session_t                 *s;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_live_relay_ctx_t               *ctx;
    ngx_live_relay_static_ctx_t        *sctx;
    ngx_live_relay_t                   *relay;

    s = ev->data;
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    if (!ctx->failed_delay && ev->timedout) { // connect timeout
        ngx_log_error(NGX_LOG_ERR, s->log, NGX_ETIMEDOUT,
                "static relay, relay timeout");
        s->finalize_reason = NGX_LIVE_RELAY_TIMEOUT;
        ngx_rtmp_finalize_session(s);

        return;
    }

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_module);

    ngx_add_timer(&ctx->reconnect, lracf->relay_reconnect);

    sctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_static_module);
    relay = sctx->relay->relay;

    ngx_live_relay_create(s, relay);
}


static ngx_int_t
ngx_live_relay_static_relay(ngx_rtmp_session_t *s,
        ngx_live_relay_static_relay_t *r)
{
    ngx_rtmp_session_t                 *rs;
    ngx_live_relay_ctx_t               *ctx, *pctx;
    ngx_live_relay_app_conf_t          *lracf;
    ngx_live_relay_static_main_conf_t  *rsmcf;
    ngx_live_relay_static_ctx_t        *sctx;
    ngx_live_relay_t                   *relay;
    ngx_rtmp_addr_conf_t               *addr_conf;

    relay = r->relay;
    rsmcf = ngx_rtmp_cycle_get_module_main_conf(ngx_cycle,
                                                ngx_live_relay_static_module);
    addr_conf = ngx_rtmp_find_related_addr_conf((ngx_cycle_t *) ngx_cycle,
                                                &rsmcf->pull_port);
    if (addr_conf == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "relay static, find related add_conf for %V failed",
                &rsmcf->pull_port);
        return NGX_DECLINED;
    }

    rs = ngx_rtmp_create_static_session(relay, addr_conf,
                                        &ngx_live_relay_static_module);
    if (rs == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "relay static, create relay session %V failed", &relay->stream);
        return NGX_DECLINED;
    }
    r->session = rs;

    rs->publishing = 1;
    rs->live_stream = ngx_live_create_stream(&rs->domain, &rs->stream);
    ngx_live_create_ctx(rs, 1);

    sctx = ngx_pcalloc(rs->pool, sizeof(ngx_live_relay_static_ctx_t));
    if (sctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "relay static, create static relay ctx failed");
        ngx_rtmp_finalize_session(rs);

        return NGX_OK;
    }
    ngx_rtmp_set_ctx(rs, sctx, ngx_live_relay_static_module);
    sctx->relay = r;

    ctx = ngx_rtmp_get_module_ctx(rs, ngx_live_relay_module);
    ctx->reconnect.log = rs->log;
    ctx->reconnect.data = rs;
    ctx->reconnect.handler = ngx_live_relay_static_handler;

    if (s == NULL) {
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


// only be called when reconnect
static ngx_int_t
ngx_live_relay_static_pull(ngx_rtmp_session_t *s)
{
    ngx_live_relay_static_ctx_t        *ctx;
    ngx_live_relay_ctx_t               *rctx;

    rctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (rctx == NULL || rctx->tag != &ngx_live_relay_static_module) {
        goto next;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_static_module);

    if (ngx_live_relay_static_relay(s, ctx->relay) == NGX_OK) {
        return NGX_OK;
    }

next:
    return next_pull(s);
}


static ngx_live_relay_static_relay_t *
ngx_live_relay_get_static_relay(ngx_live_relay_static_main_conf_t *rsmcf)
{
    ngx_live_relay_static_relay_t      *r;

    r = rsmcf->free;
    if (r) {
        rsmcf->free = r->next;
        r->session = NULL;
        r->next = NULL;

        --rsmcf->nfree;
    } else {
        r = ngx_pcalloc(ngx_cycle->pool, sizeof(ngx_live_relay_static_relay_t));
        if (r == NULL) {
            return NULL;
        }

        ++rsmcf->nalloc;
    }

    return r;
}


static void
ngx_live_relay_put_static_relay(ngx_live_relay_static_main_conf_t *rsmcf,
        ngx_live_relay_static_relay_t *r)
{
    r->next = rsmcf->free;
    rsmcf->free = r;
    ++rsmcf->nfree;
}


static void *
ngx_live_relay_static_create_main_conf(ngx_conf_t *cf)
{
    ngx_live_relay_static_main_conf_t  *rsmcf;

    rsmcf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_static_main_conf_t));
    if (rsmcf == NULL) {
        return NULL;
    }

    rsmcf->used = 1;

    ngx_map_init(&rsmcf->pulls[0], ngx_map_hash_str, ngx_cmp_str);
    ngx_map_init(&rsmcf->pulls[1], ngx_map_hash_str, ngx_cmp_str);

    return rsmcf;
}


static char *
ngx_live_relay_static_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_live_relay_static_main_conf_t  *rsmcf;

    rsmcf = conf;

    if (rsmcf->pull_port.len == 0) {
        rsmcf->pull_port.data = (u_char *) "1935";
        rsmcf->pull_port.len = sizeof("1935") - 1;
    }

    return NGX_CONF_OK;
}


static void *
ngx_live_relay_static_create_main_dconf(ngx_conf_t *cf)
{
    ngx_live_relay_static_main_dconf_t *rsmdcf;

    rsmdcf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_static_main_dconf_t));
    if (rsmdcf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&rsmdcf->pulls, cf->pool, 64, sizeof(ngx_live_relay_t))
            != NGX_OK)
    {
        return NULL;
    }

    return rsmdcf;
}


// merge all static pull into ngx_live_relay_static_main_conf_t;
static char *
ngx_live_relay_static_init_main_dconf(ngx_conf_t *cf, void *conf)
{
    ngx_live_relay_static_main_conf_t  *rsmcf;
    ngx_live_relay_static_main_dconf_t *rsmdcf;
    ngx_core_conf_t                    *ccf;
    ngx_live_relay_t                   *relay;
    ngx_live_relay_static_relay_t      *srelay, *old, *sl, *sln, **sll;
    ngx_live_relay_static_ctx_t        *ctx;
    ngx_live_relay_ctx_t               *rctx;
    ngx_map_node_t                     *node;
    unsigned                            used;
    char                               *rc;
    ngx_uint_t                          i, hash;

    rsmdcf = conf;
    rsmcf = ngx_rtmp_cycle_get_module_main_conf(ngx_cycle,
                                                ngx_live_relay_static_module);
    ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_core_module);

    used = rsmcf->used? 0: 1;
    sl = NULL;
    sll = &sl;

    relay = rsmdcf->pulls.elts;
    for (i = 0; i < rsmdcf->pulls.nelts; ++i, ++relay) {
        // should static pull in current process?
        if (ngx_process == NGX_PROCESS_WORKER) {
            hash = ngx_hash_key_lc(relay->stream.data, relay->stream.len);
            if (hash % ccf->worker_processes != ngx_worker) {
                continue;
            }
        }

        // check static pull duplicate
        node = ngx_map_find(&rsmcf->pulls[used], (intptr_t) &relay->stream);
        if (node) {
            rc = "duplicate static pull";
            goto error;
        }

        srelay = ngx_live_relay_get_static_relay(rsmcf);
        if (srelay == NULL) {
            rc = "get static relay failed";
            goto error;
        }
        srelay->relay = relay;
        srelay->node.raw_key = (intptr_t) &relay->stream;
        ngx_map_insert(&rsmcf->pulls[used], &srelay->node, 0);

        // check static pull is exist
        node = ngx_map_find(&rsmcf->pulls[rsmcf->used],
                            (intptr_t) &relay->stream);
        if (node) {
            old = (ngx_live_relay_static_relay_t *) node;
            srelay->session = old->session;

            // link swap static pull
            *sll = old;
            sll = &(*sll)->next;
        }
    }

    // delete swap static pull from old
    while (sl) {
        sln = sl;
        sl = sl->next;
        ngx_map_delete(&rsmcf->pulls[rsmcf->used],
                       (intptr_t) &sln->relay->stream);
        ngx_live_relay_put_static_relay(rsmcf, sln);
    }

    // stop deleted static pull
    node = ngx_map_begin(&rsmcf->pulls[rsmcf->used]);
    while (node) {
        srelay = (ngx_live_relay_static_relay_t *) node;
        node = ngx_map_next(node);
        ngx_live_relay_put_static_relay(rsmcf, srelay);

        rctx = ngx_rtmp_get_module_ctx(srelay->session, ngx_live_relay_module);
        rctx->giveup = 1;
        srelay->session->finalize_reason = NGX_LIVE_NORMAL_CLOSE;
        ngx_rtmp_finalize_session(srelay->session);

        ngx_map_delete(&rsmcf->pulls[rsmcf->used],
                       (intptr_t) &srelay->relay->stream);
    }

    // new static relay
    node = ngx_map_begin(&rsmcf->pulls[used]);
    for (; node; node = ngx_map_next(node)) {
        srelay = (ngx_live_relay_static_relay_t *) node;
        if (srelay->session == NULL) {
            ngx_live_relay_static_relay(NULL, srelay);
        } else {
            ctx = ngx_rtmp_get_module_ctx(srelay->session,
                                          ngx_live_relay_static_module);
            ctx->relay = srelay;
        }
    }

    rsmcf->used = used;

    return NGX_CONF_OK;

error:
    // recycle static relay resource
    node = ngx_map_begin(&rsmcf->pulls[used]);
    while (node) {
        srelay = (ngx_live_relay_static_relay_t *) node;
        node = ngx_map_next(node);
        ngx_live_relay_put_static_relay(rsmcf, srelay);

        ngx_map_delete(&rsmcf->pulls[used],
                       (intptr_t) &srelay->relay->stream);
    }

    return rc;
}


static char *
ngx_live_relay_pull(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_live_relay_static_main_dconf_t *rsmdcf;
    ngx_live_relay_t                   *relay;
    ngx_live_relay_url_t               *url;
    ngx_str_t                          *value, n, v;
    ngx_uint_t                          i;
    u_char                             *p;

    rsmdcf = conf;

    relay = ngx_array_push(&rsmdcf->pulls);
    if (relay == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_memzero(relay, sizeof(ngx_live_relay_t));

    relay->tag = &ngx_live_relay_static_module;

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

    if (relay->domain.len == 0) {
        return "domain not configured";
    }

    if (relay->app.len == 0) {
        return "app not configured";
    }

    if (relay->name.len == 0) {
        return "name not configured";
    }

    // domain/app/name
    relay->stream.len = relay->domain.len + 1 + relay->app.len + 1
                      + relay->name.len;
    relay->stream.data = ngx_pcalloc(cf->pool, relay->stream.len);
    if (relay->stream.data == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_snprintf(relay->stream.data, relay->stream.len, "%V/%V/%V",
            &relay->domain, &relay->app, &relay->name);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_live_relay_static_postconfiguration(ngx_conf_t *cf)
{
    next_pull = ngx_live_pull;
    ngx_live_pull = ngx_live_relay_static_pull;

    return NGX_OK;
}


ngx_chain_t *
ngx_live_relay_static_state(ngx_http_request_t *r)
{
    ngx_live_relay_static_main_conf_t  *rsmcf;
    ngx_chain_t                        *cl;
    ngx_buf_t                          *b;
    size_t                              len;

    rsmcf = ngx_rtmp_cycle_get_module_main_conf(ngx_cycle,
                                                ngx_live_relay_static_module);

    len = sizeof("##########rtmp live relay static##########\n") - 1
        + sizeof("relay_static alloc frame: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("relay_static free frame: \n") - 1 + NGX_OFF_T_LEN;

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
            "##########rtmp live relay static##########\n"
            "relay_static alloc frame: %ui\n"
            "relay_static free frame: %ui\n",
            rsmcf->nalloc, rsmcf->nfree);

    return cl;
}
