#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "../ngx_rbuf.h"


static char *ngx_rbuf_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_rbuf_test_commands[] = {

    { ngx_string("rbuf_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_rbuf_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_rbuf_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_rbuf_test_module = {
    NGX_MODULE_V1,
    &ngx_rbuf_test_module_ctx,              /* module context */
    ngx_rbuf_test_commands,                 /* module directives */
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


static ngx_int_t
ngx_rbuf_test_handler(ngx_http_request_t *r)
{
    ngx_str_t                       size;
    ngx_int_t                       key;
    ngx_chain_t                    *cl, *cl1, *cl2;
    ngx_chain_t                    *out;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "rbuf test handler");

    if (ngx_http_arg(r, (u_char *) "size", sizeof("size") - 1, &size)
            != NGX_OK)
    {
        return NGX_HTTP_BAD_REQUEST;
    }

    key = ngx_atoi(size.data, size.len);
    cl = ngx_get_chainbuf(key, 1);
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "cl: %p", cl);

    cl1 = ngx_get_chainbuf(key, 0);
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "cl1: %p", cl1);
    cl1->buf->start = cl->buf->start;
    cl1->buf->end = cl->buf->end;
    cl1->buf->pos = cl->buf->pos;
    cl1->buf->last = cl->buf->last;

    cl2 = ngx_get_chainbuf(key, 0);
    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "cl2: %p", cl2);
    cl2->buf->start = cl->buf->start;
    cl2->buf->end = cl->buf->end;
    cl2->buf->pos = cl->buf->pos;
    cl2->buf->last = cl->buf->last;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "rbuf free");

    ngx_put_chainbuf(cl2);

    ngx_put_chainbuf(cl);

    ngx_put_chainbuf(cl1);

    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_send_header(r);

    out = ngx_rbuf_state(r, 1);
    out->buf->last_buf = 1;

    return ngx_http_output_filter(r, out);
}


static char *
ngx_rbuf_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_rbuf_test_handler;

    return NGX_CONF_OK;
}

