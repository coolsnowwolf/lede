/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_DYNAMIC_RESOLVER_H_INCLUDED_
#define _NGX_DYNAMIC_RESOLVER_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


/*
 * paras:
 *      data   : user private data set in ngx_dynamic_resolver_start_resolver
 *      sa     : sock address get
 *      socklen: sock address len, 0 for get none address
 */
typedef void (* ngx_dynamic_resolver_handler_pt)(void *data,
        struct sockaddr *sa, socklen_t socklen);

/*
 * add domain into dynamic resolver
 *
 * paras:
 *      domain: domain for DNS query
 */

void ngx_dynamic_resolver_add_domain(ngx_str_t *domain, ngx_cycle_t *cycle);

/*
 * del domain from dynamic resolver
 *
 * paras:
 *      domain: domain for DNS query
 */
void ngx_dynamic_resolver_del_domain(ngx_str_t *domain);

/*
 * get addr from dynamic resolver by domain if exist,
 *  otherwise, get from resolver configured into event resolver
 *  and add domain into dynamic resolver
 *
 * paras:
 *      domain: domain for DNS query
 *      h     : callback handler
 *      data  : data for callback
 */
void ngx_dynamic_resolver_start_resolver(ngx_str_t *domain,
        ngx_dynamic_resolver_handler_pt h, void *data);

/*
 * get addr from dynamic resolver by domain,
 *
 * return:
 *      0 for failed
 *      socklen for successd
 *
 * paras:
 *      domain: domain for query
 *      sa    : sockaddr for domain
 */
socklen_t ngx_dynamic_resolver_gethostbyname(ngx_str_t *domain,
        struct sockaddr *sa);

/*
 * paras:
 *      r: http request to query status of dynamic resolver
 */
ngx_chain_t *ngx_dynamic_resolver_state(ngx_http_request_t *r);


#endif
