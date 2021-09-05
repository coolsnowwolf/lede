/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_POOL_DEBUG_H_INCLUDED_
#define _NGX_POOL_DEBUG_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


#define NGX_CREATE_POOL(size, log)                          \
    ngx_create_pool_debug(size, log, __FILE__, __LINE__)

#define NGX_DESTROY_POOL(pool)                              \
    ngx_destroy_pool_debug(pool, __FILE__, __LINE__)


/*
 * create pool and record where to create it for debugging
 *  such as pool not destroy
 *
 * paras:
 *      size: pool size for pool creating interface in raw ngx_create_pool
 *      log:  log for pool creating interface in raw ngx_create_pool
 *      file: use __FILE__ for recording file
 *      line: use __LINE__ for recording line
 * return:
 *      pool for successd, NULL for failed
 */
ngx_pool_t *ngx_create_pool_debug(size_t size, ngx_log_t *log,
        char *file, int line);


/*
 * destroy pool for debugging, use as pair for ngx_create_pool_debug
 *
 * paras:
 *      pool: pool for pool destroying interface in ran ngx_destroy_pool
 *      file: use __FILE__ for recording file
 *      line: use __LINE__ for recording line
 */
void ngx_destroy_pool_debug(ngx_pool_t *pool, char *file, int line);


/*
 * paras:
 *      r: http request to query status of rbuf
 */
ngx_chain_t *ngx_poold_state(ngx_http_request_t *r, unsigned detail);


#endif
