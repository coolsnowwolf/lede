
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_stream.h>

#include "ngx_ts_stream.h"
#include "ngx_ts_hls.h"
#include "ngx_ts_dash.h"


typedef struct {
    ngx_ts_hls_conf_t   *hls;
    ngx_ts_dash_conf_t  *dash;
} ngx_stream_ts_srv_conf_t;


typedef struct {
    ngx_ts_stream_t     *ts;
} ngx_stream_ts_ctx_t;


static void ngx_stream_ts_handler(ngx_stream_session_t *s);
static char *ngx_stream_ts(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_stream_ts_create_conf(ngx_conf_t *cf);
static char *ngx_stream_ts_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);


static ngx_command_t  ngx_stream_ts_commands[] = {

    { ngx_string("ts"),
      NGX_STREAM_SRV_CONF|NGX_CONF_NOARGS,
      ngx_stream_ts,
      0,
      0,
      NULL },

    { ngx_string("ts_hls"),
      NGX_STREAM_SRV_CONF|NGX_CONF_1MORE,
      ngx_ts_hls_set_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_stream_ts_srv_conf_t, hls),
      NULL },

    { ngx_string("ts_dash"),
      NGX_STREAM_SRV_CONF|NGX_CONF_1MORE,
      ngx_ts_dash_set_slot,
      NGX_STREAM_SRV_CONF_OFFSET,
      offsetof(ngx_stream_ts_srv_conf_t, dash),
      NULL },

      ngx_null_command
};


static ngx_stream_module_t  ngx_stream_ts_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    ngx_stream_ts_create_conf,     /* create server configuration */
    ngx_stream_ts_merge_conf       /* merge server configuration */
};


ngx_module_t  ngx_stream_ts_module = {
    NGX_MODULE_V1,
    &ngx_stream_ts_module_ctx,     /* module context */
    ngx_stream_ts_commands,        /* module directives */
    NGX_STREAM_MODULE,             /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static void
ngx_stream_ts_handler(ngx_stream_session_t *s);
{
    ngx_str_t                  name;
    ngx_connection_t          *c;
    ngx_stream_ts_ctx_t       *ctx;
    ngx_stream_ts_srv_conf_t  *tscf;

    c = s->connection;

    ctx = ngx_pcalloc(c->pool, sizeof(ngx_stream_ts_ctx_t));
    if (ctx == NULL) {
        ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
        return;
    }

    ctx->ts = ngx_pcalloc(c->pool, sizeof(ngx_ts_stream_t));
    if (ctx->ts == NULL) {
        ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
        return;
    }

    ctx->ts->pool = c->pool;
    ctx->ts->log = c->log;

    /* XXX */
    ngx_ste_set(&name, "foo");

    /* XXX detect streams with the same name, add shared zone */

    tscf = ngx_stream_get_module_srv_conf(s, ngx_stream_ts_module);

    if (tscf->hls) {
        if (ngx_ts_hls_create(tscf->hls, ctx->ts, &name) == NULL) {
            ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
            return;
        }
    }

    if (tscf->dash) {
        if (ngx_ts_dash_create(tscf->dash, ctx->ts, &name) == NULL) {
            ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
            return;
        }
    }

    ngx_stream_set_ctx(r, ctx, ngx_stream_ts_module);

    c->write->handler = ngx_stream_ts_read_handler;

    ngx_stream_ts_read_handler(c->write);
}


static voi
ngx_stream_ts_read_handler(ngx_event_t *rev)
{
    ssize_t                n;
    ngx_buf_t              b;
    ngx_chain_t            in;
    ngx_connection_t      *c;
    ngx_stream_ts_ctx_t   *ctx;
    ngx_stream_session_t  *s;
    u_char                 buf[4096];

    c = rev->data;
    s = c->data;

    if (ngx_exiting || ngx_terminate) {
        ngx_stream_finalize_request(s, NGX_STREAM_OK);
        return;
    }

    if (rev->timedout) {
        ngx_stream_finalize_session(s, NGX_STREAM_OK);
        return;
    }

    ctx = ngx_stream_get_module_ctx(s, ngx_stream_ts_module);

    in.buf = &b;
    in.next = NULL;

    ngx_memzero(&b, sizeof(ngx_buf_t));

    while (rev->ready) {
        n = c->recv(c, buf, sizeof(buf));

        if (n == NGX_ERROR || n == 0) {
            ngx_stream_finalize_session(s, NGX_STREAM_OK);
            return;
        }

        if (n == NGX_AGAIN) {
            break;
        }

        b.pos = buf;
        b.last = buf + n;

        if (ngx_ts_read(ctx->ts, &in) != NGX_OK) {
            ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
            return;
        }
    }

    ngx_add_timer(rev, 5000); /* XXX */

    if (ngx_handle_read_event(rev, 0) != NGX_OK) {
        ngx_stream_finalize_session(s, NGX_STREAM_INTERNAL_SERVER_ERROR);
        return;
    }
}


static char *
ngx_stream_ts(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_stream_core_srv_conf_t  *cscf;

    cscf = ngx_stream_conf_get_module_srv_conf(cf, ngx_stream_core_module);
    cscf->handler = ngx_stream_ts_handler;

    return NGX_CONF_OK;
}


static void *
ngx_stream_ts_create_conf(ngx_conf_t *cf)
{
    ngx_stream_ts_srv_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_stream_ts_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->hls = NGX_CONF_UNSET_PTR;
    conf->dash = NGX_CONF_UNSET_PTR;

    return conf;
}


static char *
ngx_stream_ts_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_stream_ts_srv_conf_t *prev = parent;
    ngx_stream_ts_srv_conf_t *conf = child;

    ngx_conf_merge_ptr_value(conf->hls, prev->hls, NULL);
    ngx_conf_merge_ptr_value(conf->dash, prev->dash, NULL);

    return NGX_CONF_OK;
}
