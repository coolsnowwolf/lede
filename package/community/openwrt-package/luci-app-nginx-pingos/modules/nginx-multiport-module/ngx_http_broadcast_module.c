/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_multiport.h"


typedef struct {
	ngx_str_t                           multiport;
    ngx_str_t                           uri;
} ngx_http_broadcast_conf_t;

typedef struct {
    ngx_int_t                           workerid;
    ngx_http_request_t                 *sr;
} ngx_http_broadcast_ctx_t;


static ngx_int_t ngx_http_broadcast_filter_init(ngx_conf_t *cf);

static void *ngx_http_broadcast_create_conf(ngx_conf_t *cf);
static char *ngx_http_broadcast_merge_conf(ngx_conf_t *cf,
       void *parent, void *child);
static char *ngx_http_broadcast(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);


static ngx_command_t  ngx_http_broadcast_commands[] = {

    { ngx_string("broadcast"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE2,
      ngx_http_broadcast,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_broadcast_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_http_broadcast_filter_init,         /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    ngx_http_broadcast_create_conf,         /* create location configuration */
    ngx_http_broadcast_merge_conf           /* merge location configuration */
};


ngx_module_t  ngx_http_broadcast_module = {
    NGX_MODULE_V1,
    &ngx_http_broadcast_module_ctx,         /* module context */
    ngx_http_broadcast_commands,            /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_http_output_header_filter_pt  ngx_http_next_header_filter;
static ngx_http_output_body_filter_pt    ngx_http_next_body_filter;


static void *
ngx_http_broadcast_create_conf(ngx_conf_t *cf)
{
    ngx_http_broadcast_conf_t          *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_broadcast_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

static char *
ngx_http_broadcast_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_broadcast_conf_t          *prev = parent;
    ngx_http_broadcast_conf_t          *conf = child;

    ngx_conf_merge_str_value(conf->multiport, prev->multiport, "");
    ngx_conf_merge_str_value(conf->uri, prev->uri, "");

    return NGX_CONF_OK;
}

static char *
ngx_http_broadcast(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_broadcast_conf_t  *hbcf;
    ngx_str_t                  *value;

    hbcf = conf;

    if (hbcf->multiport.data != NULL) {
        return "is duplicate";
    }

    value = cf->args->elts;

    hbcf->multiport = value[1];
    hbcf->uri = value[2];

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_broadcast_header_filter(ngx_http_request_t *r)
{
    ngx_http_broadcast_conf_t  *hbcf;

    hbcf = ngx_http_get_module_loc_conf(r, ngx_http_broadcast_module);

    if (hbcf == NULL || hbcf->multiport.len == 0) { /* not configured */
        goto next;
    }

    ngx_log_debug4(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "broadcast header filter, r:%p r->main:%p, %ui, %O",
            r, r->main, r->headers_out.status, r->headers_out.content_length_n);

    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_clear_content_length(r);
    ngx_http_clear_accept_ranges(r);

next:
    return ngx_http_next_header_filter(r);
}


static ngx_int_t
ngx_http_broadcast_send_subrequest(ngx_http_request_t *r, ngx_int_t pslot)
{
    ngx_http_broadcast_conf_t  *hbcf;
    ngx_str_t                   uri;
    ngx_str_t                   port;
    ngx_http_request_t         *sr;
    ngx_int_t                   rc;

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "broadcast send subrequest to %i", pslot);

    hbcf = ngx_http_get_module_loc_conf(r, ngx_http_broadcast_module);

    if (ngx_multiport_get_port(r->pool, &port, &hbcf->multiport, pslot)
            == NGX_ERROR)
    {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "broadcast get port error, %V %i", &hbcf->multiport, pslot);
        return NGX_ERROR;
    }

    uri.len = hbcf->uri.len + 1 + port.len;
    uri.data = ngx_pcalloc(r->pool, uri.len);
    ngx_snprintf(uri.data, uri.len, "%V/%V", &hbcf->uri, &port);

    rc = ngx_http_subrequest(r, &uri, &r->args, &sr, NULL, 0);
    sr->method = r->method;
    sr->method_name = r->method_name;

    return rc;
}

static ngx_int_t
ngx_http_broadcast_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
    ngx_http_broadcast_conf_t  *hbcf;
    ngx_http_broadcast_ctx_t   *ctx;
    ngx_core_conf_t            *ccf;
    ngx_int_t                   rc;
    ngx_buf_t                  *b;
    ngx_chain_t                 cl;

    hbcf = ngx_http_get_module_loc_conf(r->main, ngx_http_broadcast_module);

    if (hbcf == NULL || hbcf->multiport.len == 0) { /* not configured */
        return ngx_http_next_body_filter(r, in);
    }

    ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
            "broadcast body filter, r:%p r->main:%p", r, r->main);

    if (r != r->main) { /* send subrequest */
        if (r->headers_out.status != NGX_HTTP_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "broadcast subrequest send non 200 response: %i",
                    r->headers_out.status);
            return NGX_OK;
        }

        return ngx_http_next_body_filter(r, in);
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_broadcast_module);

    if (ctx == NULL) {
        ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_broadcast_ctx_t));
        if (ctx == NULL) {
            return NGX_ERROR;
        }

        ngx_http_set_ctx(r, ctx, ngx_http_broadcast_module);
    }

    /* send to all process */

    ccf = (ngx_core_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                           ngx_core_module);

    while (ctx->workerid < ccf->worker_processes) {

        rc = ngx_http_broadcast_send_subrequest(r,
                ngx_multiport_get_slot(ctx->workerid));
        ++ctx->workerid;

        return rc;
    }

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->last_buf = 1;

    cl.buf = b;
    cl.next = NULL;

    return ngx_http_next_body_filter(r, &cl);
}


static ngx_int_t
ngx_http_broadcast_filter_init(ngx_conf_t *cf)
{
    ngx_http_next_header_filter = ngx_http_top_header_filter;
    ngx_http_top_header_filter = ngx_http_broadcast_header_filter;

    ngx_http_next_body_filter = ngx_http_top_body_filter;
    ngx_http_top_body_filter = ngx_http_broadcast_body_filter;

    return NGX_OK;
}
