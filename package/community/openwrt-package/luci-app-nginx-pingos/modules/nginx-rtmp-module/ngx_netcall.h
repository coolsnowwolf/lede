/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_NETCALL_H_INCLUDE_
#define _NGX_NETCALL_H_INCLUDE_


#include <ngx_core.h>
#include "ngx_http_client.h"


#define NGX_NETCALL_MAX_URL_LEN     2048

typedef struct ngx_netcall_ctx_s  ngx_netcall_ctx_t;

typedef void (* ngx_netcall_handler_pt)(ngx_netcall_ctx_t *ctx, ngx_int_t code);

struct ngx_netcall_ctx_s {
    ngx_pool_t                 *pool;

    ngx_str_t                   url;

    ngx_uint_t                  idx;
    ngx_uint_t                  type;
    ngx_str_t                   groupid;
    ngx_uint_t                  stage;
    ngx_msec_t                  timeout;
    ngx_msec_t                  update;

    ngx_event_t                 ev;
    ngx_http_request_t         *hcr;
    ngx_netcall_handler_pt      handler;
    void                       *data;
};


ngx_netcall_ctx_t *ngx_netcall_create_ctx(ngx_uint_t type, ngx_str_t *groupid,
    ngx_uint_t stage, ngx_msec_t timeout, ngx_msec_t update, ngx_uint_t idx);

void ngx_netcall_create(ngx_netcall_ctx_t *nctx, ngx_log_t *log);
void ngx_netcall_destroy(ngx_netcall_ctx_t *nctx);
ngx_str_t *ngx_netcall_header(ngx_netcall_ctx_t *nctx, ngx_str_t *key);


#endif
