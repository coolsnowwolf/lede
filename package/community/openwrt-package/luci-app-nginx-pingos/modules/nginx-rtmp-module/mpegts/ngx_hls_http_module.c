
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_rtmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ngx_rtmp_cmd_module.h>
#include "ngx_http_set_header.h"
#include "ngx_mpegts_live_module.h"
#include "ngx_hls_live_module.h"
#include "ngx_rbuf.h"
#include "ngx_rtmp_dynamic.h"

#ifndef NGX_HTTP_GONE
#define NGX_HTTP_GONE 410
#endif

static ngx_keyval_t ngx_302_headers[] = {
    { ngx_string("Location"),  ngx_null_string },
    { ngx_null_string, ngx_null_string }
};

static ngx_keyval_t ngx_m3u8_headers[] = {
    { ngx_string("Cache-Control"),  ngx_string("no-cache") },
    { ngx_string("Content-Type"),   ngx_string("application/vnd.apple.mpegurl") },
    { ngx_null_string, ngx_null_string }
};

static ngx_keyval_t ngx_ts_headers[] = {
    { ngx_string("Cache-Control"),  ngx_string("no-cache") },
    { ngx_string("Content-Type"),   ngx_string("video/mp2t") },
    { ngx_null_string, ngx_null_string }
};


typedef struct {
    ngx_str_t                   app;
    ngx_str_t                   name;
    ngx_str_t                   stream;
    ngx_str_t                   serverid;
    ngx_str_t                   sid;
    ngx_rtmp_session_t         *session;
    ngx_msec_t                  timeout;
    ngx_uint_t                  content_pos;
    ngx_chain_t                *m3u8;
    ngx_uint_t                  out_pos;
    ngx_uint_t                  out_last;
    ngx_chain_t                *out_chain;
    ngx_hls_live_frag_t        *frag;
} ngx_hls_http_ctx_t;


typedef struct {
    ngx_str_t                   app;
    ngx_str_t                   flashver;
    ngx_str_t                   swf_url;
    ngx_str_t                   tc_url;
    ngx_str_t                   page_url;
	ngx_msec_t                  timeout;
    ngx_rtmp_addr_conf_t       *addr_conf;
} ngx_hls_http_loc_conf_t;

static u_char  NGX_HLS_LIVE_ARG_SESSION[] = "session";

static ngx_int_t NGX_HLS_LIVE_ARG_SESSION_LENGTH = 7;

static void * ngx_hls_http_create_loc_conf(ngx_conf_t *cf);
static char * ngx_hls_http_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child);
static char * ngx_http_hls(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
//static char * ngx_hls_http_variant(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_hls_http_postconfiguration(ngx_conf_t *cf);

static ngx_command_t  ngx_hls_http_commands[] = {

    { ngx_string("hls2_live"),
      NGX_HTTP_LOC_CONF|NGX_CONF_1MORE,
      ngx_http_hls,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};

static ngx_http_module_t  ngx_hls_http_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_hls_http_postconfiguration,     /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_hls_http_create_loc_conf,  /* create location configuration */
    ngx_hls_http_merge_loc_conf    /* merge location configuration */
};

ngx_module_t  ngx_hls_http_module = {
    NGX_MODULE_V1,
    &ngx_hls_http_module_ctx,      /* module context */
    ngx_hls_http_commands,         /* module directives */
    NGX_HTTP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};

static void *
ngx_hls_http_create_loc_conf(ngx_conf_t *cf)
{
    ngx_hls_http_loc_conf_t       *hlcf;

    hlcf = ngx_pcalloc(cf->pool, sizeof(ngx_hls_http_loc_conf_t));
    if (hlcf == NULL) {
        return NULL;
    }

    return hlcf;
}

static char *
ngx_hls_http_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_hls_http_loc_conf_t       *prev = parent;
    ngx_hls_http_loc_conf_t       *conf = child;

	ngx_conf_merge_str_value(conf->app, prev->app, "");
    ngx_conf_merge_str_value(conf->flashver, prev->flashver, "");
    ngx_conf_merge_str_value(conf->swf_url, prev->swf_url, "");
    ngx_conf_merge_str_value(conf->tc_url, prev->tc_url, "");
    ngx_conf_merge_str_value(conf->page_url, prev->page_url, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_hls_http_ctx_init(ngx_http_request_t *r, ngx_rtmp_addr_conf_t *addr_conf)
{
    u_char                             *p, *e;
    ngx_buf_t                          *buf;
    ngx_rtmp_core_srv_conf_t           *cscf;
    ngx_hls_http_loc_conf_t            *hlcf;
    ngx_hls_http_ctx_t                 *ctx;
    ngx_str_t                          *app, *name, *stream, *domain, *serverid;
    ngx_int_t                           rc;

    hlcf = ngx_http_get_module_loc_conf(r, ngx_hls_http_module);
    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);

    app = &ctx->app;
    name = &ctx->name;
    stream = &ctx->stream;
    serverid = &ctx->serverid;
    domain = &r->headers_in.server;

    p = r->uri.data;
    e = r->uri.data + r->uri.len;

    p++;
    app->data = p;
    p = ngx_strlchr(p, e, '/');
    if (p == NULL) {
        app->data = NULL;
        app->len = 0;

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ctx_init| invalid uri, lack of app");

        return NGX_ERROR;
    }
    app->len = p - app->data;

    if (hlcf->app.len > 0 && hlcf->app.data) {
        *app = hlcf->app;
    }

    p++;
    name->data = p;
    if (ngx_strncmp(&e[-5], ".m3u8", 5) == 0) {
        p = ngx_strlchr(p, e, '.');
    } else if (ngx_strncmp(&e[-3], ".ts", 3) == 0) {
        p = ngx_strlchr(p, e, '-');
    } else {
        p = NULL;
    }

    if (p == NULL) {
        name->data = NULL;
        name->len = 0;

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ctx_init| invalid uri, lack of name");

        return NGX_ERROR;
    }
    name->len = p - name->data;

    cscf = addr_conf->default_server->
            ctx->srv_conf[ngx_rtmp_core_module.ctx_index];

    rc = ngx_rtmp_find_virtual_server(addr_conf->virtual_names, domain, &cscf);
    if (rc != NGX_OK && rc != NGX_DECLINED) {

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ctx_init| server(%V) not found.", domain);

        return NGX_ERROR;
    }

    if (cscf && cscf->serverid.len) {
        serverid->data = ngx_pcalloc(r->connection->pool, cscf->serverid.len);
        if (serverid->data == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "hls-http: ctx_init| pcalloc failed.");
            return NGX_ERROR;
        }
        serverid->len = cscf->serverid.len;
        ngx_memcpy(serverid->data, cscf->serverid.data, cscf->serverid.len);
    } else {
        *serverid = *domain;
    }

    buf = ngx_create_temp_buf(r->connection->pool,
                              serverid->len + 1 + app->len + name->len + 1);
    buf->last = ngx_slprintf(buf->start, buf->end,
        "%V/%V/%V", serverid, app, name);

    stream->data = buf->pos;
    stream->len = buf->last - buf->pos;

    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "hls-http: ctx_init| hls stream (%V).", stream);

    return NGX_OK;
}


static ngx_int_t
ngx_hls_http_send_header(ngx_http_request_t *r, ngx_uint_t status, ngx_keyval_t *h)
{
    ngx_int_t                           rc;

    r->headers_out.status = status;
//    r->keepalive = 0; /* set Connection to closed */

    //set eTag
    if (ngx_http_set_etag(r) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    while (h && h->key.len) {
        rc = ngx_http_set_header_out(r, &h->key, &h->value);
        if (rc != NGX_OK) {
            return rc;
        }
        ++h;
    }

    return ngx_http_send_header(r);
}

static ngx_int_t
ngx_hls_http_master_m3u8_handler(ngx_http_request_t *r,
    ngx_rtmp_addr_conf_t *addr_conf)
{

    ngx_int_t                            rc;
    ngx_str_t                            m3u8_url;
    ngx_str_t                            host;
    u_char                               sstr[NGX_RTMP_MAX_SESSION] = {0};
    static ngx_uint_t                    sindex = 0;
    ngx_str_t                            location = ngx_string("");
    ngx_str_t                            uri;
    ngx_str_t                            uri_tail;
    ngx_buf_t                           *m3u8;
    ngx_chain_t                          out;
    ngx_str_t                            scheme = ngx_string("");
    ngx_str_t                            http = ngx_string("http");
    ngx_str_t                            https = ngx_string("https");
    u_char                              *p;

    host = r->headers_in.host->value;

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    *ngx_snprintf(sstr, sizeof(sstr) - 1, "%uDt-%uDi-%dp-%uDc",
             time(NULL), sindex++, ngx_process_slot, r->connection->number) = 0;

    ngx_http_arg(r, (u_char*)"location", 8, &location);
    if (location.len == 0) {
        uri = r->uri;
    } else {
        uri_tail.data =
        ngx_strlchr(r->uri.data + 1, r->uri.data + r->uri.len - 1, '/');
        if (uri_tail.data == NULL) {
            uri_tail = r->uri;
        } else {
            uri_tail.len = r->uri.data+r->uri.len - uri_tail.data;
        }

        uri.len = location.len + uri_tail.len;
        uri.data = ngx_pcalloc(r->pool, uri.len);
        if (uri.data == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "hls-http: master_m3u8_handler| pcalloc uri buffer failed");
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        ngx_snprintf(uri.data, uri.len, "%V%V", &location, &uri_tail);
    }

    ngx_http_arg(r, (u_char*)"scheme", 6, &scheme);
    if (scheme.len == 0) {
#if (NGX_HTTP_SSL)
        if (r->connection->ssl) {
            scheme = https;
        } else
#endif
        {
            scheme = http;
        }
    }

    m3u8_url.len = scheme.len + ngx_strlen("://") +
            host.len +
            uri.len +
            NGX_HLS_LIVE_ARG_SESSION_LENGTH + 2 +
            ngx_strlen(sstr);

    if (r->args.len > 0) {
        m3u8_url.len += 1 + r->args.len;
    }

    m3u8_url.data = ngx_pcalloc(r->connection->pool, m3u8_url.len);

    p = ngx_snprintf(m3u8_url.data, m3u8_url.len, "%V://%V%V?%s=%s",
                &scheme, &host, &uri, NGX_HLS_LIVE_ARG_SESSION, sstr);

    if (r->args.len > 0) {
        ngx_slprintf(p, m3u8_url.data + m3u8_url.len, "&%V", &r->args);
    }

    m3u8 = ngx_create_temp_buf(r->connection->pool, 64 * 1024);
    m3u8->memory = 1;
    m3u8->flush = 1;
    out.buf = m3u8;
    out.next = NULL;

    m3u8->last = ngx_snprintf(m3u8->pos, m3u8->end - m3u8->start,
        "#EXTM3U\n"
        "#EXT-X-STREAM-INF:BANDWIDTH=1,AVERAGE-BANDWIDTH=1\n"
        //"#EXT-X-STREAM-INF:PROGRAM-ID=1,BANDWIDTH=1\n"
        "%V\n", &m3u8_url);

    r->headers_out.content_length_n = m3u8->last - m3u8->pos;

    rc = ngx_hls_http_send_header(r, NGX_HTTP_OK, ngx_m3u8_headers);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: master_m3u8_handler| "
            "send header failed, redirect url: %V, rc=%d", &m3u8_url, rc);
    } else {
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                "hls-http: master_m3u8_handler| redirect url %V", &m3u8_url);
    }

    return ngx_http_output_filter(r, &out);
}

static ngx_int_t
ngx_hls_http_redirect_handler(ngx_http_request_t *r,
    ngx_rtmp_addr_conf_t *addr_conf)
{
    ngx_int_t                            rc;
    ngx_str_t                            m3u8_url;
    ngx_str_t                            host;
    u_char                               sstr[NGX_RTMP_MAX_SESSION] = {0};
    static ngx_uint_t                    sindex = 0;
    ngx_str_t                            location = ngx_string("");
    ngx_str_t                            uri;
    ngx_str_t                            uri_tail;
    ngx_str_t                            scheme = ngx_string("");
    ngx_str_t                            http = ngx_string("http");
    ngx_str_t                            https = ngx_string("https");
    u_char                              *p;

    host = r->headers_in.host->value;

    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK) {
        return rc;
    }

    *ngx_snprintf(sstr, sizeof(sstr) - 1, "%uDt-%uDi-%dp-%uDc",
             time(NULL), sindex++, ngx_process_slot, r->connection->number) = 0;

    ngx_http_arg(r, (u_char*)"location", 8, &location);
    if (location.len == 0) {
        uri = r->uri;
    } else {
        uri_tail.data =
        ngx_strlchr(r->uri.data + 1, r->uri.data + r->uri.len - 1, '/');
        if (uri_tail.data == NULL) {
            uri_tail = r->uri;
        } else {
            uri_tail.len = r->uri.data+r->uri.len - uri_tail.data;
        }

        uri.len = location.len + uri_tail.len;
        uri.data = ngx_pcalloc(r->pool, uri.len);
        if (uri.data == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                       "hls-http: redirect_handler| pcalloc uri buffer failed");
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        ngx_snprintf(uri.data, uri.len, "%V%V", &location, &uri_tail);
    }

    ngx_http_arg(r, (u_char*)"scheme", 6, &scheme);
    if (scheme.len == 0) {
#if (NGX_HTTP_SSL)
        if (r->connection->ssl) {
            scheme = https;
        } else
#endif
        {
            scheme = http;
        }
    }

    m3u8_url.len = scheme.len + ngx_strlen("://") +
            host.len +
            uri.len +
            NGX_HLS_LIVE_ARG_SESSION_LENGTH + 2 +
            ngx_strlen(sstr);

    if (r->args.len > 0) {
        m3u8_url.len += 1 + r->args.len;
    }

    m3u8_url.data = ngx_pcalloc(r->connection->pool, m3u8_url.len);

    p = ngx_snprintf(m3u8_url.data, m3u8_url.len, "%V://%V%V?%s=%s",
                &scheme, &host, &uri, NGX_HLS_LIVE_ARG_SESSION, sstr);

    if (r->args.len > 0) {
        ngx_slprintf(p, m3u8_url.data + m3u8_url.len, "&%V", &r->args);
    }

    ngx_http_set_header_out(r, &ngx_302_headers[0].key, &m3u8_url);

    r->headers_out.content_length_n = 0;
    r->header_only = 1;

    rc = ngx_hls_http_send_header(r, NGX_HTTP_MOVED_TEMPORARILY, NULL);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: redirect_handler| "
            "send header failed, redirect url: %V, rc=%d", &m3u8_url, rc);
    } else {
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                "hls-http: redirect_handler| redirect url %V", &m3u8_url);
    }

    return rc;
}


static ngx_hls_http_ctx_t *
ngx_hls_http_create_ctx(ngx_http_request_t *r, ngx_rtmp_addr_conf_t *addr_conf)
{
    ngx_hls_http_ctx_t         *ctx;
    ngx_rtmp_core_srv_conf_t   *cscf;
    u_char                     *p;

    cscf = addr_conf->default_server->ctx->
            srv_conf[ngx_rtmp_core_module.ctx_index];

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);
    if (ctx == NULL) {
        p = ngx_pcalloc(r->connection->pool, sizeof(ngx_hls_http_ctx_t));

        if (p == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "hls-http: create_ctx| alloc hls live ctx failed");
            return NULL;
        }

        ctx = (ngx_hls_http_ctx_t *)p;

        ngx_http_set_ctx(r, ctx, ngx_hls_http_module);
    }

    ctx->timeout = cscf->timeout;

    ngx_hls_http_ctx_init(r, addr_conf);

    if (ctx->app.len == 0 || ctx->name.len == 0) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: create_ctx| parse app or name failed, uri %V", &r->uri);
        return NULL;
    }

    ngx_http_arg(r, NGX_HLS_LIVE_ARG_SESSION, NGX_HLS_LIVE_ARG_SESSION_LENGTH,
        &ctx->sid);

    return ctx;
}


static void
ngx_hls_http_cleanup(void *data)
{
    ngx_http_request_t   *r;
    ngx_hls_http_ctx_t   *ctx;
    ngx_chain_t          *cl;

    r = data;
    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);

    if (!ctx) {
        return;
    }

    if (ctx->out_chain == NULL) {
        cl = ctx->out_chain;
        while (cl) {
            ctx->out_chain = cl->next;
            ngx_put_chainbuf(cl);
            cl = ctx->out_chain;
        }
    }

    ctx->content_pos = 0;

    if (ctx->session) {
        ctx->session->request = NULL;
        ctx->session->connection = NULL;
    }

    ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "hls-http: cleanup_handler| http cleanup");

    if (ctx->frag) {
        ngx_log_error(NGX_LOG_DEBUG, r->connection->log, 0,
            "hls-http: cleanup_handler| free frag[%p]", ctx->frag);
        ngx_hls_live_free_frag(ctx->frag);
        ctx->frag = NULL;
    }
}


static ngx_int_t
ngx_hls_http_parse(ngx_http_request_t *r, ngx_rtmp_session_t *s,
        ngx_rtmp_play_t *v)
{
    ngx_hls_http_loc_conf_t   *hlcf;
    size_t                     tcurl_len;
    u_char                    *p;
    ngx_hls_http_ctx_t        *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);

    hlcf = ngx_http_get_module_loc_conf(r, ngx_hls_http_module);

#define NGX_HLS_HTTP_SET_VAL(_val)                                             \
    s->_val.data = ngx_pcalloc(s->pool, ctx->_val.len);                        \
    s->_val.len = ctx->_val.len;                                               \
    ngx_memcpy(s->_val.data, ctx->_val.data, ctx->_val.len)

    NGX_HLS_HTTP_SET_VAL(app);
    NGX_HLS_HTTP_SET_VAL(stream);

#undef NGX_HLS_HTTP_SET_VAL

    if (ngx_http_arg(r, (u_char *) "flashver", 8, &s->flashver) != NGX_OK) {
        s->flashver = hlcf->flashver;
    }

    /* tc_url */
#if (NGX_HTTP_SSL)
    if (r->connection->ssl) {
        tcurl_len = sizeof("https://") - 1;
    } else
#endif
    {
        tcurl_len = sizeof("http://") - 1;
    }
    tcurl_len += r->headers_in.server.len + 1 + s->app.len;

    s->tc_url.len = tcurl_len;
    s->tc_url.data = ngx_pcalloc(r->pool, tcurl_len);
    if (s->tc_url.data == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    p = s->tc_url.data;

#if (NGX_HTTP_SSL)
    if (r->connection->ssl) {
        p = ngx_cpymem(p, "https://", sizeof("https://") - 1);
    } else
#endif
    {
        p = ngx_cpymem(p, "http://", sizeof("http://") - 1);
    }

    p = ngx_cpymem(p, r->headers_in.server.data, r->headers_in.server.len);
    *p++ = '/';
    p = ngx_cpymem(p, s->app.data, s->app.len);

    /* page_url */
    if (r->headers_in.referer) {
        s->page_url = r->headers_in.referer->value;
    } else {
        s->page_url = hlcf->page_url;
    }

    s->acodecs = 0x0DF7;
    s->vcodecs = 0xFC;

    ngx_memcpy(v->name, ctx->name.data, ctx->name.len);

    if (r->args.len) {
        ngx_memcpy(v->args, r->args.data,
            ngx_min(r->args.len, NGX_RTMP_MAX_ARGS));
    }

    ngx_memcpy(v->session, ctx->sid.data, ctx->sid.len);

    ngx_rtmp_cmd_middleware_init(s);

    return NGX_OK;
}


static ngx_rtmp_session_t*
ngx_hls_http_create_session(ngx_http_request_t *r)
{
    ngx_hls_http_loc_conf_t    *hlcf;
    ngx_rtmp_session_t         *s;
    ngx_rtmp_play_t             v;
    ngx_int_t                   rc;
    ngx_uint_t                  n;
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_rtmp_core_app_conf_t  **cacfp;
    ngx_rtmp_core_main_conf_t  *cmcf;

    hlcf = ngx_http_get_module_loc_conf(r, ngx_hls_http_module);

    /* create fake session */
    s = ngx_rtmp_create_session(hlcf->addr_conf);
    if (s == NULL) {
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "hls-http: create_session| create session failed");
        return NULL;
    }

    /* get host, app, stream name */
    ngx_memzero(&v, sizeof(ngx_rtmp_play_t));

    rc = ngx_hls_http_parse(r, s, &v);
    if (rc != NGX_OK) {
        return NULL;
    }

    if (ngx_rtmp_set_virtual_server(s, &s->domain)) {
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "hls-http: create_session| set virtual server failed, %V",
            &s->domain);
        return NULL;
    }
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    s->log->connection = r->connection->number;
    s->number = r->connection->number;
    s->live_type = NGX_HLS_LIVE;
    s->live_server = ngx_live_create_server(&s->serverid);
    s->remote_addr_text.data = ngx_pcalloc(s->pool, r->connection->addr_text.len);
    s->remote_addr_text.len = r->connection->addr_text.len;
    ngx_memcpy(s->remote_addr_text.data,
        r->connection->addr_text.data, r->connection->addr_text.len);

    v.silent = 1;

    cacfp = cscf->applications.elts;
    for (n = 0; n < cscf->applications.nelts; ++n, ++cacfp) {
        if ((*cacfp)->name.len == s->app.len &&
            ngx_strncmp((*cacfp)->name.data, s->app.data, s->app.len) == 0)
        {
            /* found app! */
            s->app_conf = (*cacfp)->app_conf;
            break;
        }
    }

    if (s->app_conf == NULL) {

        if (cscf->default_app == NULL || cscf->default_app->app_conf == NULL) {
            ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
                "hls-http: create_session| application not found '%V'", &s->app);

            ngx_rtmp_finalize_fake_session(s);

            return NULL;
        }

        s->app_conf = cscf->default_app->app_conf;
    }

    s->stage = NGX_LIVE_PLAY;
    s->ptime = ngx_current_msec;
//    s->connection = r->connection;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);
    s->variables = ngx_pcalloc(s->pool, cmcf->variables.nelts
            * sizeof(ngx_http_variable_value_t));
    if (s->variables == NULL) {
        return NULL;
    }

    if (ngx_rtmp_play_filter(s, &v) != NGX_OK) {
        return NULL;
    }

    ngx_add_timer(r->connection->write, s->timeout);

    return s;
}


static ngx_int_t
ngx_hls_http_m3u8_handler(ngx_http_request_t *r, ngx_rtmp_addr_conf_t *addr_conf)
{
    ngx_hls_http_ctx_t   *ctx;
    ngx_int_t             rc;
    ngx_rtmp_session_t   *s;
    ngx_chain_t          *out;
    ngx_buf_t            *buf;

    ctx = ngx_hls_http_create_ctx(r, addr_conf);
    if (ctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: m3u8_handler| create ctx failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    s = ngx_hls_live_fetch_session(&ctx->serverid, &ctx->stream, &ctx->sid);
    if (s == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: m3u8_handler| hls session %V not found", &ctx->sid);

        s = ngx_hls_http_create_session(r);
        if (s == NULL) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "hls-http: m3u8_handler| create hls session %V error",
                &ctx->sid);

            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        s->sockaddr = ngx_pcalloc(s->pool, sizeof(struct sockaddr));
        ngx_memcpy(s->sockaddr, r->connection->sockaddr, sizeof(struct sockaddr));
    }

    s->request = r;

    ctx->session = s;

    if (!ctx->m3u8) {
        ctx->m3u8 = ngx_pcalloc(r->connection->pool, sizeof(ngx_chain_t));
        ctx->m3u8->buf = ngx_create_temp_buf(r->connection->pool, 1024*512);
    }

    out = ctx->m3u8;
    buf = out->buf;
    buf->last = buf->pos = buf->start;
    buf->memory = 1;
    buf->flush = 1;
//    buf->last_in_chain = 1;
//    buf->last_buf = 1;

    rc = ngx_hls_live_write_playlist(s, buf, &r->headers_out.last_modified_time);
    if (rc != NGX_OK) {
        goto again;
    }

    r->headers_out.content_length_n = buf->last - buf->pos;

    s->out_bytes += r->headers_out.content_length_n;

    if (!r->header_sent) {
        rc = ngx_hls_http_send_header(r, NGX_HTTP_OK, ngx_m3u8_headers);
        if (rc != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "hls-http: m3u8_handler| send http header failed");
            return rc;
        }
    }

    rc = ngx_http_output_filter(r, out);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: m3u8_handler| send http content failed");
        return rc;
    }

    return rc;

again:
    r->count++;

    return NGX_DONE;
}


static u_char*
ngx_hls_http_strrchr(ngx_str_t *str, u_char c)
{
    u_char                             *s, *e;

    s = str->data;
    e = str->data + str->len;

    e--;
    while(e != s) {
        if (*e == c) {
            break;
        }
        e--;
    }

    if (e == s) {
        return NULL;
    }

    return e;
}


static ngx_int_t
ngx_hls_http_parse_frag(ngx_http_request_t *r, ngx_str_t *name)
{
    u_char                             *s, *e;

    e = ngx_hls_http_strrchr(&r->uri, '?');
    if (e == NULL) {
        e = r->uri.data + r->uri.len;
    }

    s = ngx_hls_http_strrchr(&r->uri, '/');
    if (s == NULL) {
        s = r->uri.data;
    } else {
        s++;
    }

    name->data = s;
    name->len = e - s;

    return NGX_OK;
}

static ngx_chain_t *
ngx_hls_http_prepare_out_chain(ngx_http_request_t *r, ngx_int_t nframes)
{
    ngx_hls_http_ctx_t   *ctx;
     ngx_hls_live_frag_t *frag;
    ngx_rtmp_session_t   *s;
    ngx_chain_t          *out, *cl, **ll;
    ngx_mpegts_frame_t   *frame;
    ngx_int_t             i = 0;

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);
    s = ctx->session;
    frag = ctx->frag;

    out = NULL;

    ll = &out;
    while (i < nframes && ctx->content_pos != frag->content_last) {
        frame = frag->content[ctx->content_pos];

        for (cl = frame->chain; cl; cl = cl->next) {
            *ll = ngx_get_chainbuf(0, 0);
            (*ll)->buf->pos = cl->buf->pos;
            (*ll)->buf->last = cl->buf->last;
            (*ll)->buf->flush = 1;

            ll = &(*ll)->next;
        }

        *ll = NULL;
        ctx->content_pos = ngx_hls_live_next(s, ctx->content_pos);
        i++;
    }

    return out;
}

static void
ngx_hls_http_write_handler(ngx_http_request_t *r)
{
    ngx_hls_http_ctx_t   *ctx;
    ngx_rtmp_session_t   *s;
    ngx_event_t          *wev;
    size_t                present, sent;
    ngx_int_t             rc;
    ngx_chain_t          *cl;

    wev = r->connection->write;           //wev->handler = ngx_http_request_handler;

    if (r->connection->destroyed) {
        return;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);
    s = ctx->session;

    if (wev->timedout) {
        ngx_log_error(NGX_LOG_INFO, s->log, NGX_ETIMEDOUT,
                "hls_http: write_handler| client timed out");
        r->connection->timedout = 1;
        if (r->header_sent) {
            ngx_http_finalize_request(r, NGX_HTTP_CLIENT_CLOSED_REQUEST);
        } else {
            r->error_page = 1;
            ngx_http_finalize_request(r, NGX_HTTP_SERVICE_UNAVAILABLE);
        }

        return;
    }

    if (wev->timer_set) {
        ngx_del_timer(wev);
    }

    if (ctx->out_chain == NULL) {
        ctx->out_chain = ngx_hls_http_prepare_out_chain(r, 4);
    }

    rc = NGX_OK;

    while (ctx->out_chain) {
        present = r->connection->sent;

        if (r->connection->buffered) {
            rc = ngx_http_output_filter(r, NULL);
        } else {
            rc = ngx_http_output_filter(r, ctx->out_chain);
        }

        sent = r->connection->sent - present;

        ngx_rtmp_update_bandwidth(&ngx_rtmp_bw_out, sent);

        if (rc == NGX_AGAIN) {
            ngx_add_timer(wev, s->timeout);
            if (ngx_handle_write_event(wev, 0) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                        "hls_http: write_handler| handle write event failed");
                ngx_http_finalize_request(r, NGX_ERROR);
            }
            return;
        }

        if (rc == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                    "hls_http: write_handler| send error");
            ngx_http_finalize_request(r, NGX_ERROR);

            return;
        }

        /* NGX_OK */
        cl = ctx->out_chain;
        while (cl) {
            ctx->out_chain = cl->next;
            ngx_put_chainbuf(cl);
            cl = ctx->out_chain;
        }

        if (ctx->frag->content_pos == ctx->frag->content_last) {
            ctx->out_chain = NULL;
            break;
        }

        ctx->out_chain = ngx_hls_http_prepare_out_chain(r, 4);
    }

    if (wev->active) {
        ngx_del_event(wev, NGX_WRITE_EVENT, 0);
    }

    ngx_http_finalize_request(r, NGX_HTTP_OK);
}

static ngx_int_t
ngx_hls_http_ts_handler(ngx_http_request_t *r, ngx_rtmp_addr_conf_t *addr_conf)
{
    ngx_hls_http_ctx_t                 *ctx;
    ngx_rtmp_session_t                 *s;
    ngx_hls_live_frag_t                *frag;
    ngx_int_t                           rc;
    ngx_str_t                           name;

    ctx = ngx_hls_http_create_ctx(r, addr_conf);
    if (ctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ts_handler| create ctx failed");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    s = ngx_hls_live_fetch_session(&ctx->serverid, &ctx->stream, &ctx->sid);
    if (s == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ts_handler| hls session %V not found", &ctx->sid);
        return NGX_DECLINED;
    }
    ctx->session = s;
    s->request = r;

//    ngx_rtmp_set_combined_log(s, r->connection->log->data,
//            r->connection->log->handler);

    rc = ngx_hls_http_parse_frag(r, &name);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ts_handler| parse frag args failed %V", &r->uri);
        return NGX_HTTP_NOT_ALLOWED;
    }

    frag = ngx_hls_live_find_frag(s, &name);
    if (frag == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ts_handler| ts not found, %V", &r->uri);
        return NGX_HTTP_NOT_FOUND;
    }

    ctx->frag = frag;

    r->headers_out.content_length_n = frag->length;
    r->headers_out.last_modified_time = frag->last_modified_time;
    s->out_bytes += r->headers_out.content_length_n;

    rc = ngx_hls_http_send_header(r, NGX_HTTP_OK, ngx_ts_headers);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: ts_handler| send http header failed, %V", &r->uri);
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_rtmp_shared_acquire_frag(frag);

    if (1) {
        r->write_event_handler = ngx_hls_http_write_handler;

        r->count++;

        ngx_hls_http_write_handler(r);

        return NGX_DONE;
    } else {
        ctx->out_chain = ngx_hls_live_prepare_frag(s, frag);

        ngx_rtmp_update_bandwidth(&ngx_rtmp_bw_out, frag->length);

        return ngx_http_output_filter(r, ctx->out_chain);
    }
}


static ngx_int_t
ngx_hls_http_handler(ngx_http_request_t *r)
{
    ngx_hls_http_loc_conf_t    *hlcf;
    ngx_int_t                   rc;
    ngx_http_cleanup_t         *cln;
    ngx_str_t                   sstr;
    ngx_hls_http_ctx_t         *ctx;

    hlcf = ngx_http_get_module_loc_conf(r, ngx_hls_http_module);

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: live_handler| donnot support the method");
        return NGX_HTTP_NOT_ALLOWED;
    }

    if (ngx_rtmp_core_main_conf->fast_reload && (ngx_exiting || ngx_terminate)) {
        return NGX_DECLINED;
    }

    if (r->uri.len < 4) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: live_handler| donnot support the file type");
        return NGX_DECLINED;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);
    if (!ctx) {
        cln = ngx_http_cleanup_add(r, 0);
        if (cln == NULL) {
            return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        cln->handler = ngx_hls_http_cleanup;
        cln->data = r;
        r->read_event_handler = ngx_http_test_reading;
    }

    if(!ngx_strncmp(r->uri.data + r->uri.len - 5, ".m3u8", 5)) {

        rc = ngx_http_arg(r, NGX_HLS_LIVE_ARG_SESSION,
                         NGX_HLS_LIVE_ARG_SESSION_LENGTH, &sstr);

        if (rc != NGX_OK || sstr.len == 0) {
            if (1) {
                return ngx_hls_http_master_m3u8_handler(r, hlcf->addr_conf);
            } else {
                return ngx_hls_http_redirect_handler(r, hlcf->addr_conf);
            }
        } else {
            return ngx_hls_http_m3u8_handler(r, hlcf->addr_conf);
        }

    } else if (!ngx_strncmp(r->uri.data + r->uri.len - 3, ".ts", 3)) {
        return ngx_hls_http_ts_handler(r, hlcf->addr_conf);
    }

    return NGX_DECLINED;
}

static ngx_int_t
ngx_hls_http_m3u8(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_http_request_t                 *r;
    ngx_int_t                           rc;
    ngx_hls_http_ctx_t                 *ctx;
    ngx_buf_t                          *buf;
    ngx_chain_t                        *out;

    r = s->request;
    if (!r) {
        return NGX_ERROR;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);

    if (!ctx->m3u8) {
        ctx->m3u8 = ngx_pcalloc(r->connection->pool, sizeof(ngx_chain_t));
        ctx->m3u8->buf = ngx_create_temp_buf(r->connection->pool, 1024*512);
    }

    out = ctx->m3u8;
    buf = out->buf;
    buf->last = buf->pos = buf->start;
    buf->memory = 1;
    buf->flush = 1;
//    buf->last_in_chain = 1;
//    buf->last_buf = 1;

    ngx_hls_live_write_playlist(s, buf, &r->headers_out.last_modified_time);

    r->headers_out.content_length_n = buf->last - buf->pos;

    s->out_bytes += r->headers_out.content_length_n;

    rc = ngx_hls_http_send_header(r, NGX_HTTP_OK, ngx_m3u8_headers);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: m3u8| send http header failed");
        return rc;
    }

    rc = ngx_http_output_filter(r, out);
    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "hls-http: m3u8| send http content failed");
        return rc;
    }

    ngx_http_finalize_request(r, NGX_HTTP_OK);

    return NGX_OK;
}

static ngx_int_t
ngx_hls_http_close(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_http_request_t   *r;
    ngx_hls_http_ctx_t   *ctx;

    r = s->request;
    if (!r) {
        return NGX_OK;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_hls_http_module);
    if (!ctx) {
        return NGX_OK;
    }

    s->request = NULL;
    s->connection = NULL;
    ctx->session = NULL;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
        "hls-http: close| finalize http request");

    ngx_http_finalize_request(r, NGX_HTTP_GONE);

    return NGX_OK;
}


static char *
ngx_http_hls(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t           *clcf;
    ngx_hls_http_loc_conf_t            *hlcf;
    ngx_str_t                          *value;
    ngx_uint_t                          n;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_hls_http_handler;

    hlcf = conf;

    value = cf->args->elts;

    hlcf->addr_conf = ngx_rtmp_find_related_addr_conf(cf->cycle, &value[1]);
    if (hlcf->addr_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (n = 2; n < cf->args->nelts; ++n) {
#define PARSE_CONF_ARGS(arg)                                                   \
        {                                                                      \
            size_t len = sizeof(#arg"=") - 1;                                  \
            if (ngx_memcmp(value[n].data, #arg"=", len) == 0) {                \
                hlcf->arg.data = value[n].data + len;                          \
                hlcf->arg.len = value[n].len - len;                            \
                continue;                                                      \
            }                                                                  \
        }

        PARSE_CONF_ARGS(app);
        PARSE_CONF_ARGS(flashver);
        PARSE_CONF_ARGS(swf_url);
        PARSE_CONF_ARGS(tc_url);
        PARSE_CONF_ARGS(page_url);

#undef PARSE_CONF_ARGS

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                "\"%V\" para not support", &value[n]);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_hls_http_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t   *cmcf;
    ngx_rtmp_handler_pt         *h;

    cmcf = ngx_rtmp_core_main_conf;

    if (!cmcf) {
        return NGX_OK;
    }

    h = ngx_array_push(&cmcf->events[NGX_MPEGTS_MSG_M3U8]);
    *h = ngx_hls_http_m3u8;

    h = ngx_array_push(&cmcf->events[NGX_MPEGTS_MSG_CLOSE]);
    *h = ngx_hls_http_close;

    return NGX_OK;
}

