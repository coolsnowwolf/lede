#ifndef _NGX_RTMP_HTTP_HEADER_OUT_H_INCLUDED_
#define _NGX_RTMP_HTTP_HEADER_OUT_H_INCLUDED_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

ngx_int_t ngx_http_set_header_out(ngx_http_request_t *r,
    ngx_str_t *key, ngx_str_t *value);

#endif
