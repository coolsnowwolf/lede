#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_multiport.h"
#include "ngx_test_macro.h"


static char *ngx_multiport_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_multiport_test_commands[] = {

    { ngx_string("multiport_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_multiport_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_multiport_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_multiport_test_module = {
    NGX_MODULE_V1,
    &ngx_multiport_test_module_ctx,         /* module context */
    ngx_multiport_test_commands,            /* module directives */
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


ngx_int_t
ngx_multiport_test_get_port(ngx_http_request_t *r, char *multiport,
        ngx_int_t pslot, char *expect)
{
    ngx_str_t                   port;
    ngx_int_t                   rc;
    ngx_str_t                   mp;

    mp.data = (u_char *) multiport;
    mp.len = ngx_strlen(multiport);

    ngx_memzero(&port, sizeof(ngx_str_t));
    rc = ngx_multiport_get_port(r->pool, &port, &mp, pslot);
    if (port.len) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "port: %V, %s, %d",
                &port, port.data, port.len);
    }
    if (expect == NULL && rc == NGX_ERROR) {
        return 1;
    } else {
        return ngx_test_str(&port, expect);
    }

    return 0;
}

static ngx_int_t
ngx_multiport_test_handler(ngx_http_request_t *r)
{
    ngx_buf_t                  *b;
    ngx_chain_t                 cl;
    size_t                      len;
    ngx_int_t                   rc;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "multiport test handler");

    rc = ngx_http_inner_proxy_request(r, ngx_multiport_get_slot(0));
    if (rc != NGX_DECLINED) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                "inner proxy return %i", rc);
        return rc;
    }

    NGX_TEST_INIT

    /* test ngx_multiport_get_port */
    /* normal format */
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "10", 127, "137"));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "127.0.0.1:55635", 4,
                                              "127.0.0.1:55639"));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1]:1024", 0,
                                              "[::127.0.0.1]:1024"));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "unix:/tmp.test", 7,
                                              "unix:/tmp.test.7"));

    /* inet6 format error */
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1:1024", 0, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1]:abcd",
                                              0, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1]:65409",
                                              0, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1]:", 0, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "[::127.0.0.1]", 0, NULL));

    /* inet format error */
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "127.0.0.1:", 4, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "abcd", 4, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "65410", 4, NULL));

    /* pslot error */
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "65410", -1, NULL));
    NGX_TEST_ISOK(ngx_multiport_test_get_port(r, "65410", 128, NULL));

    /* test ngx_multiport_get_slot */
    NGX_TEST_ISOK(ngx_multiport_get_slot(4) == -1);
    NGX_TEST_ISOK(ngx_multiport_get_slot(0) == 0);
    NGX_TEST_ISOK(ngx_multiport_get_slot(1) == 1);
    NGX_TEST_ISOK(ngx_multiport_get_slot(2) == 2);
    NGX_TEST_ISOK(ngx_multiport_get_slot(3) == 3);

    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);

    len = sizeof("pslot: %i TEST cases 4294967296, 4294967296 pass\n") - 1;
    b = ngx_create_temp_buf(r->pool, len);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->last = ngx_snprintf(b->last, len, "pslot: %i TEST cases %d, %d pass\n",
            ngx_process_slot, count, pass);
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    return ngx_http_output_filter(r, &cl);
}


static char *
ngx_multiport_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t   *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_multiport_test_handler;

    return NGX_CONF_OK;
}
