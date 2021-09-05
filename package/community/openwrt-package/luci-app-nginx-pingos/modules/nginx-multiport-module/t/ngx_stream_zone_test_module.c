#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_stream_zone_module.h"


static char *ngx_stream_zone_test(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);

static ngx_command_t  ngx_stream_zone_test_commands[] = {

    { ngx_string("stream_zone_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_stream_zone_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_stream_zone_test_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                               /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,                               /* create location configuration */
    NULL                                /* merge location configuration */
};


ngx_module_t  ngx_stream_zone_test_module = {
    NGX_MODULE_V1,
    &ngx_stream_zone_test_module_ctx,   /* module context */
    ngx_stream_zone_test_commands,      /* module directives */
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
ngx_stream_zone_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                     cl, *out;
    ngx_buf_t                      *b;
    size_t                          len;
    ngx_str_t                       stream;
    ngx_int_t                       rc;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "stream zone test handler");

    rc = -1;
    if (r->method == NGX_HTTP_GET) {
        out = ngx_stream_zone_state(r, 1);
        out->buf->last_buf = 1;
        out->buf->last_in_chain = 1;

        r->headers_out.status = NGX_HTTP_OK;
        ngx_http_send_header(r);

        return ngx_http_output_filter(r, out);
    } else if (r->method == NGX_HTTP_DELETE) {

        if (ngx_http_arg(r, (u_char *) "stream", 6, &stream) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "stream zone test, no stream in http args");
            return NGX_HTTP_BAD_REQUEST;
        }

        len = sizeof("delete stream=\n") - 1 + stream.len;

        ngx_stream_zone_delete_stream(&stream);
    } else if (r->method == NGX_HTTP_POST) {

        if (ngx_http_arg(r, (u_char *) "stream", 6, &stream) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "stream zone test, no stream in http args");
            return NGX_HTTP_BAD_REQUEST;
        }

        len = sizeof("stream= in process:4294967296\n") - 1 + stream.len;

        rc = ngx_stream_zone_insert_stream(&stream);
    } else {
        return NGX_HTTP_BAD_REQUEST;
    }

    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_send_header(r);

    b = ngx_create_temp_buf(r->pool, len);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (r->method == NGX_HTTP_DELETE) {
        b->last = ngx_snprintf(b->last, len, "delete stream=%V\n", &stream);
    } else {
        b->last = ngx_snprintf(b->last, len,
                "stream=%V in process:%i\n", &stream, rc);
    }
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    return ngx_http_output_filter(r, &cl);
}

static char *
ngx_stream_zone_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_stream_zone_test_handler;

    return NGX_CONF_OK;
}
