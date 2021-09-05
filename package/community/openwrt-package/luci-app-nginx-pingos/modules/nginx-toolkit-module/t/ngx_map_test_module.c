#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_map.h"
#include "ngx_test_macro.h"


static char *ngx_map_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_map_test_commands[] = {

    { ngx_string("map_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_map_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_map_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_map_test_module = {
    NGX_MODULE_V1,
    &ngx_map_test_module_ctx,               /* module context */
    ngx_map_test_commands,                  /* module directives */
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


#define MAP_NODE_INIT_STR(n, k)                                 \
    ngx_map_node_t      n;                                      \
    static ngx_str_t    k = ngx_string(#k);                     \
    ngx_memzero(&n, sizeof(ngx_map_node_t));                    \
    n.raw_key = (intptr_t) &k;

#define MAP_NODE_PRINT_KEY_STR(n)                               \
    {                                                           \
        ngx_str_t      *s = (ngx_str_t *) (n)->raw_key;         \
        ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,      \
                "!!!!!!!!!!!!!!!!%V", s);                       \
    }

static ngx_int_t
ngx_map_test_handler(ngx_http_request_t *r)
{
    ngx_buf_t                  *b;
    ngx_chain_t                 cl;
    size_t                      len;
    ngx_map_t                   map;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
            "map test handler");

    ngx_memzero(&map, sizeof(ngx_map_t));
    ngx_map_init(&map, ngx_map_hash_str, ngx_cmp_str);

    MAP_NODE_INIT_STR(n1, k1)
    MAP_NODE_INIT_STR(n2, k2)
    MAP_NODE_INIT_STR(n3, k3)
    MAP_NODE_INIT_STR(n4, k4)
    MAP_NODE_INIT_STR(n5, k5)
    MAP_NODE_INIT_STR(n6, k6)
    MAP_NODE_INIT_STR(n7, k7)
    MAP_NODE_INIT_STR(n8, k8)
    MAP_NODE_INIT_STR(n9, k9)
    MAP_NODE_INIT_STR(n10, k10)
    ngx_map_node_t      n11;
    static ngx_str_t    k11 = ngx_string("k5");
    ngx_memzero(&n11, sizeof(ngx_map_node_t));
    n11.raw_key = (intptr_t) &k11;

    ngx_map_insert(&map, &n1, 0);
    ngx_map_insert(&map, &n2, 0);
    ngx_map_insert(&map, &n3, 0);
    ngx_map_insert(&map, &n4, 0);
    ngx_map_insert(&map, &n5, 0);
    ngx_map_insert(&map, &n6, 0);
    ngx_map_insert(&map, &n7, 0);
    ngx_map_insert(&map, &n8, 0);
    ngx_map_insert(&map, &n9, 0);
    ngx_map_insert(&map, &n10, 0);
    ngx_map_insert(&map, &n11, 0);

    NGX_TEST_INIT

    ngx_map_node_t     *n;
    n = ngx_map_begin(&map);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n1 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n2 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n3 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n4 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n5 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n6 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n7 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n8 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n9 == n)

    n = ngx_map_next(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n10 == n)

    n = ngx_map_next(n);
    NGX_TEST_ISOK(NULL == n)

    n = ngx_map_rbegin(&map);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n10 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n9 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n8 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n7 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n6 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n5 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n4 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n3 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n2 == n)

    n = ngx_map_prev(n);
    MAP_NODE_PRINT_KEY_STR(n)
    NGX_TEST_ISOK(&n1 == n)

    n = ngx_map_prev(n);
    NGX_TEST_ISOK(NULL == n)

    static ngx_str_t kk1 = ngx_string("k100");
    NGX_TEST_ISOK(NULL == ngx_map_find(&map, (intptr_t) &kk1))

    static ngx_str_t kk2 = ngx_string("k5");
    NGX_TEST_ISOK(&n5 == ngx_map_find(&map, (intptr_t) &kk2))

    ngx_map_insert(&map, &n11, 1);
    NGX_TEST_ISOK(&n11 == ngx_map_find(&map, (intptr_t) &kk2))

    static ngx_str_t kk3 = ngx_string("k8");
    ngx_map_delete(&map, (intptr_t) &kk3);
    NGX_TEST_ISOK(NULL == ngx_map_find(&map, (intptr_t) &kk3))

    static ngx_str_t kk4 = ngx_string("k8");
    ngx_map_insert(&map, &n8, 1);
    NGX_TEST_ISOK(&n8 == ngx_map_find(&map, (intptr_t) &kk4))

    static ngx_str_t kk5 = ngx_string("k7");
    NGX_TEST_ISOK(&n7 == ngx_map_find(&map, (intptr_t) &kk5))

    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);

    len = sizeof("TEST cases 4294967296, 4294967296 pass\n") - 1;
    b = ngx_create_temp_buf(r->pool, len);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    b->last = ngx_snprintf(b->last, len, "TEST cases %d, %d pass\n",
            count, pass);
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    return ngx_http_output_filter(r, &cl);
}


static char *
ngx_map_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t   *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_map_test_handler;

    return NGX_CONF_OK;
}

