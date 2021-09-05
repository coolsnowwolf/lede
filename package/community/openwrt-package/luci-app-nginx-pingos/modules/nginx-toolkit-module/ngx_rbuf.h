/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_RBUF_H_INCLUDED_
#define _NGX_RBUF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


ngx_chain_t *ngx_get_chainbuf_debug(size_t size, ngx_flag_t alloc_rbuf,
        char *file, int line);
void ngx_put_chainbuf_debug(ngx_chain_t *cl, char *file, int len);

/*
 * paras:
 *      size: buffer size for allocate
 *      alloc_rbuf: whether alloc rbuf
 * return:
 *      nginx chain
 */
#define ngx_get_chainbuf(size, alloc_rbuf)                          \
    ngx_get_chainbuf_debug(size, alloc_rbuf, __FILE__, __LINE__);

/*
 * paras:
 *      cl: nginx chain return by ngx_rtmp_shared_get_chainbuf
 */
#define ngx_put_chainbuf(cl)                                        \
    ngx_put_chainbuf_debug(cl, __FILE__, __LINE__);


/*
 * paras:
 *      cl: nginx chain return by ngx_rtmp_shared_get_chainbuf
 */
#define ngx_put_chainbufs(cl)                                       \
{                                                                   \
    ngx_chain_t                *l;                                  \
                                                                    \
    l = cl;                                                         \
    while (l) {                                                     \
        cl = l->next;                                               \
        ngx_put_chainbuf_debug(l, __FILE__, __LINE__);              \
        l = cl;                                                     \
    }                                                               \
}

/*
 * paras:
 *      r: http request to query status of rbuf
 */
ngx_chain_t *ngx_rbuf_state(ngx_http_request_t *r, unsigned detail);

#endif
