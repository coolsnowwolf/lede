/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_TOOLKIT_MISC_H_INCLUDED_
#define _NGX_TOOLKIT_MISC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/*
 * scheme://[user@]host[:port]/path[?args][#fragment]
 */
typedef struct {
    ngx_str_t                   scheme;
    ngx_str_t                   user;
    ngx_str_t                   host;
    ngx_str_t                   port;
    ngx_str_t                   path;
    ngx_str_t                   args;
    ngx_str_t                   fragment;

    ngx_str_t                   host_with_port; /* host[:port] */
    ngx_str_t                   uri_with_args;  /* /path[?args][#fragment] */
} ngx_request_url_t;


#define NGX_MD5KEY_LEN  32


/*
 * parse request url format as:
 *      scheme://[user@]host[:port]/path[?args][#fragment]
 *
 * return value:
 *      NGX_OK   : parse success
 *      NGX_ERROR: request url format error
 * paras:
 *      request_url: url parse result return to user,
 *          all paras in request url is segment point to url
 *      url        : request url for parse
 */
ngx_int_t ngx_parse_request_url(ngx_request_url_t *request_url, ngx_str_t *url);

/*
 * convert port to in_port_t according to scheme and port
 *
 * return value:
 *      If port is set to correct number range in [1, 65535], return port
 *      If port is set to non correct value, return 0
 *      If port is not set, return default value for scheme:
 *          80 for http
 *          443 for https
 *          1935 for rtmp
 *          0 for others now
 * values:
 *      scheme : sheme string like http https or rtmp
 *      port   : port for convert to in_port_t
 */
in_port_t ngx_request_port(ngx_str_t *scheme, ngx_str_t *port);

/*
 * calculating file md5key as md5sum in shell
 *
 * return value:
 *      NGX_OK   : calculating success
 *      NGX_ERROR: calculating error such as file is not exist
 * paras:
 *      fd    : file desc for calculating md5key
 *      md5key: md5key result
 */
ngx_int_t ngx_md5_file(ngx_fd_t fd, u_char md5key[NGX_MD5KEY_LEN]);


/*
 * copy str
 *
 * return value:
 *      NGX_OK: copy successd
 *      NGX_ERROR: copy failed
 * paras:
 *      pool: for alloc space for storing str
 *      dst: str copy to
 *      src: str copy from
 */
ngx_int_t ngx_copy_str(ngx_pool_t *pool, ngx_str_t *dst, ngx_str_t *src);


/*
 * parse text to struct sockaddr
 *
 * return value:
 *      0: if text is not ipv4/ipv6/unix address
 *      >0: socklen for sa
 *
 * paras:
 *      sa: struct sockadd parse to
 *      text: ipv4/ipv6/unix address for parsing, text could have port
 *      len: text's length
 */
socklen_t ngx_sock_pton(struct sockaddr *sa, u_char *text, size_t len);


#ifdef NGX_DEBUG

#define NGX_START_TIMING                                            \
    struct timeval      __start, __end;                             \
    ngx_gettimeofday(&__start);

#define NGX_STOP_TIMING(log, msg)                                   \
    ngx_gettimeofday(&__end);                                       \
    ngx_log_error(NGX_LOG_INFO, log, 0, msg " spend %ui us",        \
        (__end.tv_sec - __start.tv_sec) * 1000000                   \
        + (__end.tv_usec - __start.tv_usec));

#else

#define NGX_START_TIMING
#define NGX_STOP_TIMING(log, msg)

#endif

#endif
