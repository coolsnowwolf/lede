#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_dynamic_resolver.h"


static char *ngx_dynamic_resolver_test(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


typedef struct {
    ngx_flag_t                  sync;
} ngx_dynamic_resolver_test_ctx_t;


static ngx_command_t  ngx_dynamic_resolver_test_commands[] = {

    { ngx_string("dynamic_resolver_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_ANY,
      ngx_dynamic_resolver_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_dynamic_resolver_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_dynamic_resolver_test_module = {
    NGX_MODULE_V1,
    &ngx_dynamic_resolver_test_module_ctx,  /* module context */
    ngx_dynamic_resolver_test_commands,     /* module directives */
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


static void
ngx_dynamic_resolver_test_result(void *data, struct sockaddr *sa, socklen_t len)
{
    ngx_dynamic_resolver_test_ctx_t *ctx;
    ngx_chain_t                     cl;
    ngx_buf_t                      *b;
    ngx_int_t                       rc;
    ngx_http_request_t             *r;
    u_char                          text[NGX_SOCKADDRLEN];

    r = data;

    r->headers_out.status = NGX_HTTP_OK;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send header");

    if (len == 0) {
        r->headers_out.content_length_n = 0;
        r->header_only = 1;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_dynamic_resolver_test_module);
    if (ctx == NULL) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        ngx_http_finalize_request(r, rc);
        return;
    }

    ngx_memzero(text, sizeof(text));
    len = ngx_sock_ntop(sa, len, text, NGX_SOCKADDRLEN, 0) + 1;

    b = ngx_create_temp_buf(r->pool, len);

    if (b == NULL) {
        ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    b->last = ngx_snprintf(b->last, len, "%s\n", text);
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send body");

    ngx_http_output_filter(r, &cl);

    if (ctx->sync) {
        return;
    }

    ngx_http_finalize_request(r, NGX_OK);
}

static ngx_int_t
ngx_dynamic_resolver_test_handler(ngx_http_request_t *r)
{
    ngx_dynamic_resolver_test_ctx_t *ctx;
    ngx_str_t                       domain, sync;
    struct sockaddr                 sa;
    socklen_t                       len;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                  "dynamic resolver test handler");

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_dynamic_resolver_test_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_http_set_ctx(r, ctx, ngx_dynamic_resolver_test_module);

    if (ngx_http_arg(r, (u_char *) "domain", 6, &domain) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "dynamic resolver test, no domain in http args");
        return NGX_HTTP_BAD_REQUEST;
    }

    if (ngx_http_arg(r, (u_char *) "sync", 4, &sync) == NGX_OK) {
        ctx->sync = 1;
    }

    if (r->method == NGX_HTTP_GET && ctx->sync == 0) {
        r->count++;
        ngx_dynamic_resolver_start_resolver(&domain,
                ngx_dynamic_resolver_test_result, r);

        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                      "dynamic resolver test, after start resolver");
        return NGX_DONE;
    } else if (r->method == NGX_HTTP_GET && ctx->sync) {
        ngx_memzero(&sa, sizeof(sa));
        len = ngx_dynamic_resolver_gethostbyname(&domain, &sa);
        ngx_dynamic_resolver_test_result(r, &sa, len);

        return NGX_OK;
    } else if (r->method != NGX_HTTP_DELETE) {
        return NGX_HTTP_BAD_REQUEST;
    }

    ngx_dynamic_resolver_del_domain(&domain);

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = 0;
    r->header_only = 1;

    return ngx_http_send_header(r);
}


static char *
ngx_dynamic_resolver_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t       *clcf;
    ngx_str_t                      *value;
    ngx_uint_t                      i;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_dynamic_resolver_test_handler;

    value = cf->args->elts;

    for (i = 1; i < cf->args->nelts; ++i) {
        ngx_dynamic_resolver_add_domain(&value[i], cf->cycle);
    }

    return NGX_CONF_OK;
}
