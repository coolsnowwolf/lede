/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_LIVE_H_INCLUDED_
#define _NGX_LIVE_H_INCLUDED_


#include "ngx_rtmp.h"


typedef struct {
    size_t                      stream_buckets;
    size_t                      server_buckets;

    ngx_live_server_t         **servers;

    ngx_live_server_t          *free_server;
    ngx_live_stream_t          *free_stream;

    ngx_uint_t                  alloc_server_count;
    ngx_uint_t                  free_server_count;

    ngx_uint_t                  alloc_stream_count;
    ngx_uint_t                  free_stream_count;

    ngx_pool_t                 *pool;
} ngx_live_conf_t;


extern ngx_module_t     ngx_live_module;


/*
 * paras:
 *      r: http request to query status of rbuf
 */
ngx_chain_t *ngx_live_state(ngx_http_request_t *r);


#endif
