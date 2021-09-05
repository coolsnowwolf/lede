/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_http.h>
#include "ngx_rtmp.h"
#include "ngx_rbuf.h"
#include "ngx_poold.h"
#include "ngx_stream_zone_module.h"
#include "ngx_event_timer_module.h"
#include "ngx_event_resolver.h"
#include "ngx_dynamic_resolver.h"
#include "ngx_live.h"


static char *ngx_rtmp_sys_stat(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_chain_t *ngx_live_relay_static_state(ngx_http_request_t *r);


static ngx_command_t  ngx_rtmp_sys_stat_commands[] = {

    { ngx_string("sys_stat"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_rtmp_sys_stat,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_rtmp_sys_stat_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                               /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,                               /* create location configuration */
    NULL                                /* merge location configuration */
};


ngx_module_t  ngx_rtmp_sys_stat_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_sys_stat_module_ctx,      /* module context */
    ngx_rtmp_sys_stat_commands,         /* module directives */
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
ngx_rtmp_sys_stat_handler(ngx_http_request_t *r)
{
    ngx_chain_t                       **ll, *out;
    ngx_buf_t                          *b;
    ngx_str_t                           detail_arg;
    ngx_uint_t                          detail = 0;
    size_t                              len;

    if (ngx_http_arg(r, (u_char *) "detail", sizeof("detail") - 1, &detail_arg)
            == NGX_OK)
    {
        detail = 1;
    }

    r->headers_out.status = NGX_HTTP_OK;
    ngx_http_send_header(r);

    ll = &out;

    len = sizeof("--------------------------------------------------\n") - 1
        + sizeof("ngx_worker:   ngx_process_slot:   pid: \n") - 1
        + 3 * NGX_OFF_T_LEN;

    *ll = ngx_alloc_chain_link(r->pool);
    if (*ll == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    (*ll)->next = NULL;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    (*ll)->buf = b;

    b->last = ngx_snprintf(b->last, len,
            "--------------------------------------------------\n"
            "ngx_worker: %i  ngx_process_slot: %i  pid: %i\n",
            ngx_worker, ngx_process_slot, ngx_pid);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_live_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_live_relay_static_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_rtmp_shared_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_rbuf_state(r, detail);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_stream_zone_state(r, 0);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_event_timer_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_event_resolver_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_dynamic_resolver_state(r);

    if (*ll) {
        ll = &(*ll)->next;
    }
    *ll = ngx_poold_state(r, detail);

    (*ll)->buf->last_buf = 1;

    return ngx_http_output_filter(r, out);
}

static char *
ngx_rtmp_sys_stat(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t           *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_rtmp_sys_stat_handler;

    return NGX_CONF_OK;
}
