#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_poold.h"


static char *ngx_poold_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_poold_test_commands[] = {

    { ngx_string("poold_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_poold_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_poold_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_poold_test_module = {
    NGX_MODULE_V1,
    &ngx_poold_test_module_ctx,             /* module context */
    ngx_poold_test_commands,                /* module directives */
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
ngx_poold_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                    *out;
    ngx_pool_t                     *pool;

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0, "poold test handler");

    pool = NGX_CREATE_POOL(4096, r->connection->log);

    pool = NGX_CREATE_POOL(1024, r->connection->log);

    NGX_DESTROY_POOL(pool);

    NGX_DESTROY_POOL(pool);

    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_send_header(r);

    out = ngx_poold_state(r, 1);
    out->buf->last_buf = 1;

    return ngx_http_output_filter(r, out);
}


static char *
ngx_poold_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t   *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_poold_test_handler;

    return NGX_CONF_OK;
}

