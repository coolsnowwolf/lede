/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_TIMER_DEBUG_H_INCLUDED_
#define _NGX_TIMER_DEBUG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#define NGX_ADD_TIMER(ev, timer, fpoff)                                 \
    ngx_add_timer_debug(ev, timer, fpoff, __FILE__, __LINE__)

#define NGX_DEL_TIMER(ev, footprint)                                    \
    ngx_del_timer_debug(ev, footprint, __FILE__, __LINE__)


/*
 * generate a new footprint
 *
 * return:
 *      footprint
 */
ngx_uint_t ngx_timerd_footprint();


/*
 * add timer and record where to add it for debugging
 *  such as timer not destroy
 *
 * paras:
 *      ev:    nginx event for timer
 *      timer: timer interval for triggering timer
 *      fpoff: footprint offset in event data
 *      file:  use __FILE__ for recording file
 *      line:  use __LINE__ for recording line
 * return:
 *      pool for successd, NULL for failed
 */
void ngx_add_timer_debug(ngx_event_t *ev, ngx_msec_t timer, off_t fpoff,
        char *file, int line);


/*
 * destroy pool for debugging, use as pair for ngx_create_pool_debug
 *
 * paras:
 *      ev:        nginx event for timer
 *      footprint: footprint value in user event data
 *      file:      use __FILE__ for recording file
 *      line:      use __LINE__ for recording line
 */
void ngx_del_timer_debug(ngx_event_t *ev, ngx_uint_t footprint,
        char *file, int line);


/*
 * paras:
 *      r: http request to query status of rbuf
 */
ngx_chain_t *ngx_timerd_state(ngx_http_request_t *r, unsigned detail);


#endif
