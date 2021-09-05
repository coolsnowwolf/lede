#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_timerd.h"


static char *ngx_timerd_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


typedef struct {
    ngx_pool_t                 *pool;
    ngx_event_t                 ev;
    ngx_uint_t                  footprint;
} ngx_timer_test_data_t;


static ngx_command_t  ngx_timerd_test_commands[] = {

    { ngx_string("timerd_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_timerd_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_timerd_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_timerd_test_module = {
    NGX_MODULE_V1,
    &ngx_timerd_test_module_ctx,            /* module context */
    ngx_timerd_test_commands,               /* module directives */
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
ngx_timerd_test_timer(ngx_event_t *ev)
{
    ngx_timer_test_data_t      *data;

    data = ev->data;

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "timerd test timer: %p %ui",
            data, data->footprint);
}


static ngx_int_t
ngx_timerd_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                *out;

    // normal timer trigger
    ngx_timer_test_data_t      *test;
    ngx_pool_t                 *pool;

    pool = ngx_create_pool(4096, r->connection->log);
    test = ngx_pcalloc(pool, sizeof(ngx_timer_test_data_t));

    test->pool = pool;
    test->footprint = ngx_timerd_footprint();
    test->ev.data = test;
    test->ev.handler = ngx_timerd_test_timer;
    NGX_ADD_TIMER(&test->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "!!!!!!!!!!normal timer trigger: %p %ui", test, test->footprint);

    // delete timer twice
    ngx_timer_test_data_t      *test1;
    ngx_pool_t                 *pool1;

    pool1 = ngx_create_pool(4096, r->connection->log);
    test1 = ngx_pcalloc(pool1, sizeof(ngx_timer_test_data_t));

    test1->pool = pool1;
    test1->footprint = ngx_timerd_footprint();
    test1->ev.data = test1;
    test1->ev.handler = ngx_timerd_test_timer;
    NGX_ADD_TIMER(&test1->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "!!!!!!!!!!delete timer twice: %p %ui", test1, test1->footprint);
    NGX_DEL_TIMER(&test1->ev, test1->footprint);
    NGX_DEL_TIMER(&test1->ev, test1->footprint);

    // timer trigger after event destroy
    ngx_timer_test_data_t      *test2;
    ngx_pool_t                 *pool2;

    pool2 = ngx_create_pool(4096, r->connection->log);
    test2 = ngx_pcalloc(pool2, sizeof(ngx_timer_test_data_t));

    test2->pool = pool2;
    test2->footprint = ngx_timerd_footprint();
    test2->ev.data = test2;
    test2->ev.handler = ngx_timerd_test_timer;
    NGX_ADD_TIMER(&test2->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "!!!!!!!!!!timer trigger after event destroy: %p %ui",
            test2, test2->footprint);
    test2->footprint = ngx_timerd_footprint();

    ngx_destroy_pool(pool2);

    // delete timer after event destroy
    ngx_timer_test_data_t      *test3;
    ngx_pool_t                 *pool3;

    pool3 = ngx_create_pool(4096, r->connection->log);
    test3 = ngx_pcalloc(pool3, sizeof(ngx_timer_test_data_t));

    test3->pool = pool3;
    test3->footprint = ngx_timerd_footprint();
    test3->ev.data = test3;
    test3->ev.handler = ngx_timerd_test_timer;
    NGX_ADD_TIMER(&test3->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "!!!!!!!!!!delete timer after event destroy: %p %ui",
            test3, test3->footprint);
    test3->footprint = ngx_timerd_footprint();

    NGX_DEL_TIMER(&test3->ev, test3->footprint);

    ngx_destroy_pool(pool3);


    // add timer after event destroy
    ngx_timer_test_data_t      *test4;
    ngx_pool_t                 *pool4;

    pool4 = ngx_create_pool(4096, r->connection->log);
    test4 = ngx_pcalloc(pool4, sizeof(ngx_timer_test_data_t));

    test4->pool = pool4;
    test4->footprint = ngx_timerd_footprint();
    test4->ev.data = test4;
    test4->ev.handler = ngx_timerd_test_timer;
    NGX_ADD_TIMER(&test4->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_log_error(NGX_LOG_INFO, r->connection->log, 0,
            "!!!!!!!!!!add timer after event destroy: %p %ui",
            test4, test4->footprint);
    test4->footprint = ngx_timerd_footprint();

    NGX_ADD_TIMER(&test4->ev, 5000, offsetof(ngx_timer_test_data_t, footprint));

    ngx_destroy_pool(pool4);


    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_send_header(r);

    out = ngx_timerd_state(r, 1);
    out->buf->last_buf = 1;

    return ngx_http_output_filter(r, out);
}


static char *
ngx_timerd_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t   *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_timerd_test_handler;

    return NGX_CONF_OK;
}

