/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_STREAM_ZONE_MODULE_H_INCLUDED_
#define _NGX_STREAM_ZONE_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


/*
 * return value:
 *      process_slot for owner of stream, NGX_ERROR for error
 * name:
 *      stream name
 */
ngx_int_t ngx_stream_zone_insert_stream(ngx_str_t *name);

/*
 * name:
 *      stream name
 */
void ngx_stream_zone_delete_stream(ngx_str_t *name);

/*
 * return value:
 *      chain of stream zone state for returning to http client
 * paras:
 *      r: http request to query status of rbuf
 *      detail: print stream detail in log
 */
ngx_chain_t *ngx_stream_zone_state(ngx_http_request_t *r, ngx_flag_t detail);


#endif
