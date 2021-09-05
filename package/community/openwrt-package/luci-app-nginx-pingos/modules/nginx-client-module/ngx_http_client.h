/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_HTTP_CLIENT_H_INCLUDE_
#define _NGX_HTTP_CLIENT_H_INCLUDE_

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_client.h"
#include "ngx_toolkit_misc.h"


// http client method
#define NGX_HTTP_CLIENT_GET         0
#define NGX_HTTP_CLIENT_HEAD        1
#define NGX_HTTP_CLIENT_POST        2
#define NGX_HTTP_CLIENT_PUT         3
#define NGX_HTTP_CLIENT_DELETE      4
#define NGX_HTTP_CLIENT_MKCOL       5
#define NGX_HTTP_CLIENT_COPY        6
#define NGX_HTTP_CLIENT_MOVE        7
#define NGX_HTTP_CLIENT_OPTIONS     8
#define NGX_HTTP_CLIENT_PROPFIND    9
#define NGX_HTTP_CLIENT_PROPPATCH   10
#define NGX_HTTP_CLIENT_LOCK        11
#define NGX_HTTP_CLIENT_UNLOCK      12
#define NGX_HTTP_CLIENT_PATCH       13
#define NGX_HTTP_CLIENT_TRACE       14

// http client version
#define NGX_HTTP_CLIENT_VERSION_9   0
#define NGX_HTTP_CLIENT_VERSION_10  1
#define NGX_HTTP_CLIENT_VERSION_11  2
#define NGX_HTTP_CLIENT_VERSION_20  3

// http client opt
#define NGX_HTTP_CLIENT_OPT_CONNECT_TIMEOUT     0
#define NGX_HTTP_CLIENT_OPT_SEND_TIMEOUT        1
#define NGX_HTTP_CLIENT_OPT_POSTPONE_OUTPUT     2
#define NGX_HTTP_CLIENT_OPT_DYNAMIC_RESOLVER    3
#define NGX_HTTP_CLIENT_OPT_TCP_NODELAY         4
#define NGX_HTTP_CLIENT_OPT_TCP_NOPUSH          5
#define NGX_HTTP_CLIENT_OPT_HEADER_TIMEOUT      6


typedef void (* ngx_http_client_handler_pt)(void *r, ngx_http_request_t *hcr);


/* create and set http request */

/*
 * create a http request for sending to server
 *
 * return value:
 *      return http request for successd, return NULL for failed
 *
 * paras:
 *      log: error in create will use this log
 *      method: http client method
 *      url: full request url like "http://test.com/index.html?hello=world"
 *      headers: http request header for sending
 *      send_body: callback for sending body
 *      request: who send http request
 */
ngx_http_request_t *ngx_http_client_create(ngx_log_t *log,
    ngx_uint_t method, ngx_str_t *url, ngx_keyval_t *headers,
    ngx_http_client_handler_pt send_body, void *request);

/*
 * add cleanup as ngx_http_cleanup_add
 */
ngx_http_cleanup_t *ngx_http_client_cleanup_add(ngx_http_request_t *r,
    size_t size);

/*
 * set read handler for http client, should set before send request,
 *  otherwise body from server will discard
 *
 * return value:
 *      void
 *
 * paras:
 *      r: http client request
 *      read_handler: handler for setting
 */
void ngx_http_client_set_read_handler(ngx_http_request_t *r,
    ngx_http_client_handler_pt read_handler);

/*
 * set http headers
 *
 * return value:
 *      NGX_OK for successd, NGX_ERROR for failed
 *
 * paras:
 *      r: http client request
 *      headers: headers set into r
 *          if value is not null, will set or modify the header
 *          if value is null string, will delete the header
 */
ngx_int_t ngx_http_client_set_headers(ngx_http_request_t *r,
        ngx_keyval_t *headers);

/*
 * set write handler for http client, if set,
 *  will use this handler for sending body,
 *
 * return value:
 *      void
 *
 * paras:
 *      r: http client request
 *      write_handler: handler for setting
 */
void ngx_http_client_set_write_handler(ngx_http_request_t *r,
    ngx_http_client_handler_pt write_handler);

/*
 * set write handler for http client, if set,
 *  will use this handler for sending body,
 *
 * return value:
 *      void
 *
 * paras:
 *      r: http client request
 *      write_handler: handler for setting
 */
void ngx_http_client_set_version(ngx_http_request_t *r, ngx_uint_t version);

/*
 * set http client option
 *
 * return value:
 *      void
 *
 * paras:
 *      r: http client request
 *      opt: http client opt
 *          NGX_HTTP_CLIENT_OPT_CONNECT_TIMEOUT:
 *              connect server timer
 *          NGX_HTTP_CLIENT_OPT_SEND_TIMEOUT:
 *              send data timer when buffer full
 *          NGX_HTTP_CLIENT_OPT_POSTPONE_OUTPUT:
 *              size threshold to send
 *          NGX_HTTP_CLIENT_OPT_DYNAMIC_RESOLVER:
 *              whether use dynamic resolver to resolv domain
 *          NGX_HTTP_CLIENT_OPT_TCP_NODELAY:
 *              whether set TCP_NODELAY
 *          NGX_HTTP_CLIENT_OPT_TCP_NOPUSH:
 *              whether set TCP_CORK
 *          NGX_HTTP_CLIENT_OPT_HEADER_TIMEOUT:
 *              timer for waiting response header from server
 *      value: http client opt value want to set
 */
void ngx_http_client_setopt(ngx_http_request_t *r, unsigned opt,
    ngx_uint_t value);

/* send http request */

/*
 * send http request
 *
 * return value:
 *      NGX_OK for successd, NGX_ERROR for failed
 *
 * paras:
 *      r: http request for seding, create by ngx_http_client_create
 */
ngx_int_t ngx_http_client_send(ngx_http_request_t *r);

/*
 * create and send http GET request to server
 *
 * return value:
 *      return http request for successd, return NULL for failed
 *
 * paras:
 *      log: error in create will use this log
 *      url: full request url like "http://test.com/index.html?hello=world"
 *      headers: http request header for sending
 *      request: who send http request
 */
ngx_http_request_t *ngx_http_client_get(ngx_log_t *log, ngx_str_t *url,
    ngx_keyval_t *headers, void *request);

/*
 * create and send http HEAD request to server
 *
 * return value:
 *      return http request for successd, return NULL for failed
 *
 * paras:
 *      log: error in create will use this log
 *      url: full request url like "http://test.com/index.html?hello=world"
 *      headers: http request header for sending
 *      request: who send http request
 */
ngx_http_request_t *ngx_http_client_head(ngx_log_t *log, ngx_str_t *url,
    ngx_keyval_t *headers, void *request);

/*
 * create and send http POST request to server
 *
 * return value:
 *      return http request for successd, return NULL for failed
 *
 * paras:
 *      log: error in create will use this log
 *      url: full request url like "http://test.com/index.html?hello=world"
 *      headers: http request header for sending
 *      send_body: callback for sending body
 *      request: who send http request
 */
ngx_http_request_t *ngx_http_client_post(ngx_log_t *log, ngx_str_t *url,
    ngx_keyval_t *headers, ngx_http_client_handler_pt send_body, void *request);


/* get response */

/*
 * get http response version
 *
 * return value:
 *      http response version
 *
 * paras:
 *      r: http client request
 */
ngx_uint_t ngx_http_client_http_version(ngx_http_request_t *r);

/*
 * get http response status code
 *
 * return value:
 *      http response status code like 200, 500
 *
 * paras:
 *      r: http client request
 */
ngx_uint_t ngx_http_client_status_code(ngx_http_request_t *r);

/*
 * get http response header's value
 *
 * return value:
 *      http response header's value
 *
 * paras:
 *      r: http client request
 *      key: http header like "Host", "Content-Type"
 */
ngx_str_t *ngx_http_client_header_in(ngx_http_request_t *r, ngx_str_t *key);

/*
 * read http response body
 *
 * return value:
 *      NGX_AGAIN: read part of data
 *      0: tcp connection disconnect, need finalize request with 1
 *      NGX_ERROR: tcp connection error disconnect, need finalize request with 1
 *      NGX_DONE: response body has been read, could finalize request with 0
 *
 * paras:
 *      r: http client request
 *      in: where read data put
 */
ngx_int_t ngx_http_client_read_body(ngx_http_request_t *r, ngx_chain_t **in);

/*
 * get receive bytes
 *
 * return value:
 *      bytes receive from server
 *
 * paras:
 *      r: http client request
 */
off_t ngx_http_client_rbytes(ngx_http_request_t *r);

/*
 * get send bytes
 *
 * return value:
 *      bytes send to server
 *
 * paras:
 *      r: http client request
 */
off_t ngx_http_client_wbytes(ngx_http_request_t *r);


/* end request */

/*
 * detach http client request with it's creator,
 *  all read and write handler will not be triggered
 *
 * return value:
 *      bytes send to server
 *
 * paras:
 *      r: http client request
 */
void ngx_http_client_detach(ngx_http_request_t *r);

/*
 * finalize http client request
 *
 * return value:
 *      void
 *
 * paras:
 *      r: http client request
 *      closed: set to 1, will close connection to server
 *          set to 0, will keep connection to server alive
 */
void ngx_http_client_finalize_request(ngx_http_request_t *r, ngx_flag_t closed);

#endif
