#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_event_timer_module.h"


static char *ngx_event_timer_test(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);

static ngx_command_t  ngx_event_timer_test_commands[] = {

    { ngx_string("event_timer_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_event_timer_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_event_timer_test_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                               /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,                               /* create location configuration */
    NULL                                /* merge location configuration */
};


ngx_module_t  ngx_event_timer_test_module = {
    NGX_MODULE_V1,
    &ngx_event_timer_test_module_ctx,   /* module context */
    ngx_event_timer_test_commands,      /* module directives */
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


static void
ngx_event_timer_test_timer_handler(void *data)
{
    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
            "event timer test timer handler");
}

static ngx_int_t
ngx_event_timer_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                     cl;
    ngx_buf_t                      *b;
    size_t                          len;
    ngx_int_t                       rc, timerid;
    ngx_msec_t                      time;
    ngx_str_t                       arg;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                  "event timer test handler");

    if (r->method == NGX_HTTP_DELETE) {
        len = sizeof("delete timer timerid=NGX_OFF_T_LEN\n") - 1;

        if (ngx_http_arg(r, (u_char *) "timerid", 7, &arg) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "event timer test, no timerid in http args");
            return NGX_HTTP_BAD_REQUEST;
        }

        timerid = ngx_atoi(arg.data, arg.len);
        if (timerid == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "event timer test, timerid arg not int type");
            return NGX_HTTP_BAD_REQUEST;
        }

        ngx_event_timer_del_timer(timerid);
    } else if (r->method == NGX_HTTP_POST) {
        len = sizeof("add timer timerid=NGX_OFF_T_LEN\n") - 1;

        if (ngx_http_arg(r, (u_char *) "time", 4, &arg) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "event timer test, no time in http args");
            return NGX_HTTP_BAD_REQUEST;
        }

        time = ngx_parse_time(&arg, 0);
        if (time == (ngx_msec_t) NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, 0,
                    "event timer test, time arg not time type");
            return NGX_HTTP_BAD_REQUEST;
        }

        timerid = ngx_event_timer_add_timer(time,
                ngx_event_timer_test_timer_handler, NULL);
    } else {
        return NGX_HTTP_BAD_REQUEST;
    }

    r->headers_out.status = NGX_HTTP_OK;

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    b = ngx_create_temp_buf(r->pool, len);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    if (r->method == NGX_HTTP_DELETE) {
        b->last = ngx_snprintf(b->last, len,
                "delete timer timerid=%i\n", timerid);
    } else {
        b->last = ngx_snprintf(b->last, len,
                "add timer timerid=%i\n", timerid);
    }
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    return ngx_http_output_filter(r, &cl);
}

static char *
ngx_event_timer_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_event_timer_test_handler;

    return NGX_CONF_OK;
}
