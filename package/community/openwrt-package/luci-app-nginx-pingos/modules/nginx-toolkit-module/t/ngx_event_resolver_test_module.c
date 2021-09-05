#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_event_resolver.h"


static char *ngx_event_resolver_test(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


static ngx_command_t  ngx_event_resolver_test_commands[] = {

    { ngx_string("event_resolver_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_event_resolver_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_event_resolver_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_event_resolver_test_module = {
    NGX_MODULE_V1,
    &ngx_event_resolver_test_module_ctx,    /* module context */
    ngx_event_resolver_test_commands,       /* module directives */
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
ngx_event_resolver_test_result(void *data, ngx_resolver_addr_t *addrs,
    ngx_uint_t naddrs)
{
    ngx_chain_t                    *cl, **ll;
    ngx_buf_t                      *b;
    size_t                          len;
    ngx_int_t                       rc;
    ngx_uint_t                      i;
    ngx_http_request_t             *r;
    u_char                          text[NGX_SOCKADDRLEN];

    r = data;

    r->headers_out.status = NGX_HTTP_OK;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send header");

    if (naddrs == 0) {
        r->headers_out.content_length_n = 0;
        r->header_only = 1;
    }

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        ngx_http_finalize_request(r, rc);
        return;
    }

    ll = &cl;
    for (i = 0; i < naddrs; ++i) {
        len = ngx_sock_ntop(addrs[i].sockaddr, addrs[i].socklen, text,
                            NGX_SOCKADDRLEN, 0) + 1;

        b = ngx_create_temp_buf(r->pool, len);

        if (b == NULL) {
            ngx_http_finalize_request(r, NGX_HTTP_INTERNAL_SERVER_ERROR);
            return;
        }

        b->last = ngx_snprintf(b->last, len, "%s\n", text);
        if (i == naddrs - 1) {
            b->last_buf = 1;
            b->last_in_chain = 1;
        }

        *ll = ngx_alloc_chain_link(r->pool);
        (*ll)->buf = b;
        (*ll)->next = NULL;
        ll = &(*ll)->next;
    }

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send body");

    ngx_http_output_filter(r, cl);

    ngx_http_finalize_request(r, NGX_OK);
}

static ngx_int_t
ngx_event_resolver_test_handler(ngx_http_request_t *r)
{
    ngx_str_t                       domain;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "event resolver test handler");

    if (ngx_http_arg(r, (u_char *) "domain", sizeof("domain") - 1, &domain)
            != NGX_OK)
    {
        return NGX_HTTP_BAD_REQUEST;
    }

    ++r->count;

    ngx_event_resolver_start_resolver(&domain,
                                      ngx_event_resolver_test_result, r);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "after start resolver");

    return NGX_DONE;
}


static char *
ngx_event_resolver_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_event_resolver_test_handler;

    return NGX_CONF_OK;
}
