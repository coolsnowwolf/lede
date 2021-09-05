/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */



#ifndef _NGX_EVENT_RESOLVER_H_INCLUDED_
#define _NGX_EVENT_RESOLVER_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_http.h>


/*
 * paras:
 *      data  : user private data set in ngx_event_resolver_start_resolver
 *      addrs : addrs resolv by DNS
 *      naddrs: number of addrs resolv by DNS
 */
typedef void (* ngx_event_resolver_handler_pt)(void *data,
        ngx_resolver_addr_t *addrs, ngx_uint_t naddrs);


/*
 * paras:
 *      domain: domain for resolving
 *      h     : callback handler
 *      data  : data for callback
 */
void ngx_event_resolver_start_resolver(ngx_str_t *domain,
        ngx_event_resolver_handler_pt h, void *data);

/*
 * paras:
 *      r: http request to query status of event resolver
 */
ngx_chain_t *ngx_event_resolver_state(ngx_http_request_t *r);


#endif
