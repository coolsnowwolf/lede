/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_EVENT_TIMER_MODULE_H_INCLUDED_
#define _NGX_EVENT_TIMER_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_http.h>


typedef void (* ngx_timer_handler_pt)(void *data);

/*
 * return value:
 *      return timerid for successd, NGX_ERROR for failed
 * paras:
 *      tv   : timer interval to trigger handler
 *      h    : timer handler
 *      data : data of h para
 */
ngx_int_t ngx_event_timer_add_timer(ngx_msec_t tv,
        ngx_timer_handler_pt h, void *data);

/*
 * paras:
 *      timerid: value return in ngx_timer_add_timer
 */
void ngx_event_timer_del_timer(ngx_uint_t timerid);

/*
 * paras:
 *      r: http request to query status of event timer
 */
ngx_chain_t *ngx_event_timer_state(ngx_http_request_t *r);


#endif
