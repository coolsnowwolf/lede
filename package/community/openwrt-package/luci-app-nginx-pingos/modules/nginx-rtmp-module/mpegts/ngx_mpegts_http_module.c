
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_http.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rbuf.h"
#include "ngx_http_set_header.h"
#include "ngx_rtmp_monitor_module.h"
#include "ngx_mpegts_gop_module.h"
#include "ngx_mpegts_live_module.h"


static char *ngx_mpegts_http(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void *ngx_mpegts_http_create_loc_conf(ngx_conf_t *cf);
static char *ngx_mpegts_http_merge_loc_conf(ngx_conf_t *cf, void *parent,
       void *child);

static ngx_keyval_t ngx_mpegts_http_headers[] = {
    { ngx_string("Cache-Control"),  ngx_string("no-cache") },
//    { ngx_string("Content-Type"),   ngx_string("video/x-ts") },
    { ngx_null_string, ngx_null_string }
};

typedef struct {
    ngx_rtmp_session_t         *session;
} ngx_mpegts_http_ctx_t;

typedef struct {
    ngx_str_t                   app;
    ngx_str_t                   flashver;
    ngx_str_t                   swf_url;
    ngx_str_t                   tc_url;
    ngx_str_t                   page_url;

    ngx_rtmp_addr_conf_t       *addr_conf;
} ngx_mpegts_http_loc_conf_t;


static ngx_command_t  ngx_mpegts_http_commands[] = {

    { ngx_string("ts_live"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
      ngx_mpegts_http,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_mpegts_http_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                               /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_mpegts_http_create_loc_conf,  /* create location configuration */
    ngx_mpegts_http_merge_loc_conf    /* merge location configuration */
};


ngx_module_t  ngx_mpegts_http_module = {
    NGX_MODULE_V1,
    &ngx_mpegts_http_module_ctx,      /* module context */
    ngx_mpegts_http_commands,         /* module directives */
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


static ngx_int_t
ngx_mpegts_http_send_header(ngx_http_request_t *r)
{
    ngx_int_t                rc;
    ngx_keyval_t            *h;
    ngx_chain_t              out;
    ngx_mpegts_http_ctx_t   *ctx;
    ngx_rtmp_session_t      *s, *ps;

    if (r->header_sent) {
        return NGX_OK;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_mpegts_http_module);
    s = ctx->session;
    ps = s->live_stream->publish_ctx->session;

    r->headers_out.status = NGX_HTTP_OK;
    r->keepalive = 0; /* set Connection to closed */

    h = ngx_mpegts_http_headers;
    while (h->key.len) {
        rc = ngx_http_set_header_out(r, &h->key, &h->value);
        if (rc != NGX_OK) {
            return rc;
        }
        ++h;
    }

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK) {
        return rc;
    }

    ngx_memzero(&out, sizeof(out));
    out.buf = ngx_create_temp_buf(s->pool, 376);
    out.buf->last = ngx_cpymem(out.buf->pos,
        ngx_rtmp_mpegts_pat, 188);

    ngx_rtmp_mpegts_gen_pmt(ps->vcodec, ps->acodec, s->log, out.buf->last);
    out.buf->last += 188;

    out.buf->flush = 1;
    out.buf->memory = 1;

    return ngx_http_output_filter(r, &out);
}

static void
ngx_mpegts_http_write_handler(ngx_http_request_t *r)
{
    ngx_mpegts_http_ctx_t   *ctx;
    ngx_rtmp_session_t      *s;
    ngx_event_t             *wev;
    size_t                   present, sent;
    ngx_int_t                rc;
    ngx_chain_t             *cl;

    wev = r->connection->write;

    if (r->connection->destroyed) {
        return;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_mpegts_http_module);
    s = ctx->session;

    if (wev->timedout) {
        ngx_log_error(NGX_LOG_INFO, s->log, NGX_ETIMEDOUT,
                "mpegts-http: write_handler| client timed out");
        r->connection->timedout = 1;
        s->finalize_reason = NGX_LIVE_FLV_SEND_TIMEOUT;
        if (r->header_sent) {
            ngx_http_finalize_request(r, NGX_HTTP_REQUEST_TIME_OUT);
        } else {
            r->error_page = 1;
            ngx_http_finalize_request(r, NGX_HTTP_SERVICE_UNAVAILABLE);
        }

        return;
    }

    if (wev->timer_set) {
        ngx_del_timer(wev);
    }

    if (ngx_rtmp_core_main_conf->fast_reload && (ngx_exiting || ngx_terminate)) {
        r->error_page = 1;
        ngx_http_finalize_request(r, NGX_HTTP_SERVICE_UNAVAILABLE);
        return;
    }

    if (ngx_rtmp_prepare_merge_frame(s) == NGX_ERROR) {
        ngx_http_finalize_request(r, NGX_ERROR);
        return;
    }

    if (s->out_chain) {
        rc = ngx_mpegts_http_send_header(r);
        if (rc == NGX_ERROR || rc > NGX_OK) {
            s->finalize_reason = NGX_LIVE_FLV_SEND_ERR;
            ngx_http_finalize_request(r, rc);
            return;
        }
    }

    while (s->out_chain) {
        present = r->connection->sent;

        if (r->connection->buffered) {
            rc = ngx_http_output_filter(r, NULL);
        } else {
            rc = ngx_http_output_filter(r, s->out_chain);
        }

        sent = r->connection->sent - present;

        ngx_rtmp_update_bandwidth(&ngx_rtmp_bw_out, sent);

        if (rc == NGX_AGAIN) {
            ngx_add_timer(wev, s->timeout);
            if (ngx_handle_write_event(wev, 0) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                        "mpegts-http: write_handler| write event failed");
                ngx_http_finalize_request(r, NGX_ERROR);
            }
            return;
        }

        if (rc == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, ngx_errno,
                    "mpegts-http: write_handler| send error");
            s->finalize_reason = NGX_LIVE_FLV_SEND_ERR;
            ngx_http_finalize_request(r, NGX_ERROR);
            return;
        }

        /* NGX_OK */
        for (cl = s->out_chain; cl;) {
            s->out_chain = cl->next;
            ngx_free_chain(s->pool, cl);
            cl = s->out_chain;
        }

        if (ngx_rtmp_prepare_merge_frame(s) == NGX_ERROR) {
            ngx_http_finalize_request(r, NGX_ERROR);
            return;
        }
    }

    if (wev->active) {
        ngx_del_event(wev, NGX_WRITE_EVENT, 0);
    }
}


static void
ngx_mpegts_http_parse_url(ngx_http_request_t *r, ngx_str_t *app,
        ngx_str_t *name)
{
    u_char                             *p, *end, *pos;

    p = r->uri.data + 1; /* skip '/' */
    end = r->uri.data + r->uri.len;
    app->data = p;

    pos = (u_char *) ngx_strnstr(p, ".ts", end - p);
    if (pos) {
        end = pos;
    }

    p = (u_char *) ngx_strnstr(p, "/", end - p);
    while (p) {
        name->data = p;
        p = (u_char *) ngx_strnstr(p + 1, "/", end - p);
    }

    if (name->data == NULL) {
        return;
    }

    app->len = name->data - app->data;

    ++name->data;
    name->len = end - name->data;
}

static ngx_int_t
ngx_mpegts_http_parse(ngx_http_request_t *r, ngx_rtmp_session_t *s,
        ngx_rtmp_play_t *v)
{
    ngx_mpegts_http_loc_conf_t       *hflcf;
    ngx_str_t                           app, stream;
    size_t                              tcurl_len;
    u_char                             *p;

    hflcf = ngx_http_get_module_loc_conf(r, ngx_mpegts_http_module);

    ngx_memzero(&app, sizeof(ngx_str_t));
    ngx_memzero(&stream, sizeof(ngx_str_t));

    ngx_mpegts_http_parse_url(r, &app, &stream);

    if (app.len == 0 || stream.len == 0 || stream.len > NGX_RTMP_MAX_NAME) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "mpegts-http: http_parse| url error: %V", &r->uri);
        return NGX_HTTP_BAD_REQUEST;
    }

    if (hflcf->app.len) {
        app = hflcf->app;
    }

    ngx_http_arg(r, (u_char *) "app", 3, &app);

    s->app = app;

    if (ngx_http_arg(r, (u_char *) "flashver", 8, &s->flashver) != NGX_OK) {
        s->flashver = hflcf->flashver;
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
    tcurl_len += r->headers_in.server.len + 1 + app.len;

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
    p = ngx_cpymem(p, app.data, app.len);

    /* page_url */
    if (r->headers_in.referer) {
        s->page_url = r->headers_in.referer->value;
    } else {
        s->page_url = hflcf->page_url;
    }

    s->acodecs = 0x0DF7;
    s->vcodecs = 0xFC;

    ngx_memcpy(v->name, stream.data, stream.len);

    if (r->args.len) {
        ngx_memcpy(v->args, r->args.data,
                ngx_min(r->args.len, NGX_RTMP_MAX_ARGS));
    }

    ngx_rtmp_cmd_middleware_init(s);

    return NGX_OK;
}

static void
ngx_mpegts_http_cleanup(void *data)
{
    ngx_http_request_t                 *r;
    ngx_mpegts_http_ctx_t            *ctx;

    r = data;

    ctx = ngx_http_get_module_ctx(r, ngx_mpegts_http_module);

    if (ctx == NULL) {
        return;
    }

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "mpegts-http: cleanup| cleanup");

    if (ctx->session) {
        ctx->session->request = NULL;

        if (ctx->session->finalize_reason == 0) {
            ctx->session->finalize_reason = r->connection->read->error?
                NGX_LIVE_FLV_RECV_ERR : NGX_LIVE_NORMAL_CLOSE;
        }

        ngx_rtmp_finalize_fake_session(ctx->session);
    }
}

static ngx_chain_t *
ngx_mpegts_http_prepare_out_chain(ngx_rtmp_session_t *s)
{
    ngx_mpegts_frame_t                 *frame;
    ngx_chain_t                        *head, **ll, *cl;

    frame = NULL;
    head = NULL;

    if (s->out_pos != s->out_last) {
        frame = s->mpegts_out[s->out_pos];
    }

    /* no frame to send */
    if (frame == NULL) {
        return NULL;
    }

    ll = &head;

    for (cl = frame->chain; cl; cl = cl->next) {
        (*ll) = ngx_get_chainbuf(0, 0);
        if (*ll == NULL) {
            goto falied;
        }
        (*ll)->buf->pos = cl->buf->pos;
        (*ll)->buf->last = cl->buf->last;
        if (!(*ll)->next) {
            (*ll)->buf->flush = 1;
        }

        ll = &(*ll)->next;
    }

    return head;

falied:
    ngx_put_chainbufs(head);

    ngx_rtmp_finalize_session(s);
    return NULL;
}

static ngx_int_t
ngx_mpegts_http_handler(ngx_http_request_t *r)
{
    ngx_mpegts_http_loc_conf_t   *hflcf;
    ngx_mpegts_http_ctx_t        *ctx;
    ngx_rtmp_session_t           *s;
    ngx_rtmp_play_t               v;
    ngx_int_t                     rc;
    ngx_uint_t                    n;
    ngx_rtmp_core_srv_conf_t     *cscf;
    ngx_rtmp_core_app_conf_t    **cacfp;
    ngx_http_cleanup_t           *cln;
    ngx_rtmp_core_main_conf_t    *cmcf;

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_mpegts_http_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_http_set_ctx(r, ctx, ngx_mpegts_http_module);

    /* cleanup handler */
    cln = ngx_http_cleanup_add(r, 0);
    if (cln == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    cln->handler = ngx_mpegts_http_cleanup;
    cln->data = r;

    hflcf = ngx_http_get_module_loc_conf(r, ngx_mpegts_http_module);

    /* create fake session */
    s = ngx_rtmp_create_session(hflcf->addr_conf);
    if (s == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ctx->session = s;
    s->connection = r->connection;
    ngx_rtmp_set_combined_log(s, r->connection->log->data,
            r->connection->log->handler);
    s->log->connection = r->connection->number;
    s->number = r->connection->number;
    s->remote_addr_text.data = ngx_pcalloc(s->pool, r->connection->addr_text.len);
    s->remote_addr_text.len = r->connection->addr_text.len;
    ngx_memcpy(s->remote_addr_text.data,
        r->connection->addr_text.data, r->connection->addr_text.len);

    /* get host, app, stream name */
    ngx_memzero(&v, sizeof(ngx_rtmp_play_t));
    rc = ngx_mpegts_http_parse(r, s, &v);
    if (rc != NGX_OK) {
        return rc;
    }

    if (ngx_rtmp_set_virtual_server(s, &s->domain)) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    s->live_type = NGX_MPEGTS_LIVE;
    s->live_server = ngx_live_create_server(&s->serverid);
    s->request = r;

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
                    "mpegts-http: http_handler| "
                    "application not found '%V'", &s->app);
            return NGX_HTTP_NOT_FOUND;
        }

        s->app_conf = cscf->default_app->app_conf;
    }

    s->prepare_handler = ngx_mpegts_http_prepare_out_chain;

    s->stage = NGX_LIVE_PLAY;
    s->ptime = ngx_current_msec;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);
    s->variables = ngx_pcalloc(s->pool, cmcf->variables.nelts
            * sizeof(ngx_http_variable_value_t));
    if (s->variables == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (ngx_rtmp_play_filter(s, &v) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    ngx_add_timer(r->connection->write, s->timeout);

    r->read_event_handler = ngx_http_test_reading;
    r->write_event_handler = ngx_mpegts_http_write_handler;

    ++r->count;

    return NGX_DONE;
}


static void *
ngx_mpegts_http_create_loc_conf(ngx_conf_t *cf)
{
    ngx_mpegts_http_loc_conf_t   *hflcf;

    hflcf = ngx_pcalloc(cf->pool, sizeof(ngx_mpegts_http_loc_conf_t));
    if (hflcf == NULL) {
        return NULL;
    }

    return hflcf;
}

static char *
ngx_mpegts_http_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_mpegts_http_loc_conf_t   *prev = parent;
    ngx_mpegts_http_loc_conf_t   *conf = child;

    ngx_conf_merge_str_value(conf->app, prev->app, "");
    ngx_conf_merge_str_value(conf->flashver, prev->flashver, "");
    ngx_conf_merge_str_value(conf->swf_url, prev->swf_url, "");
    ngx_conf_merge_str_value(conf->tc_url, prev->tc_url, "");
    ngx_conf_merge_str_value(conf->page_url, prev->page_url, "");

    return NGX_CONF_OK;
}

static char *
ngx_mpegts_http(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t     *clcf;
    ngx_mpegts_http_loc_conf_t   *hflcf;
    ngx_str_t                    *value, v;
    ngx_uint_t                    i;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_mpegts_http_handler;

    hflcf = conf;

    value = cf->args->elts;

    hflcf->addr_conf = ngx_rtmp_find_related_addr_conf(cf->cycle, &value[1]);
    if (hflcf->addr_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (i = 2; i < cf->args->nelts; ++i) {
        if (ngx_strncmp(value[i].data, "app=", 4) == 0) {
            v.data = value[i].data + 4;
            v.len = value[i].len - 4;
            hflcf->app = v;
        } else {
            return NGX_CONF_ERROR;
        }
    }

    return NGX_CONF_OK;
}
