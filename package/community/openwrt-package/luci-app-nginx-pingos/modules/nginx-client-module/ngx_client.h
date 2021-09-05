/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_CLIENT_H_INCLUDED_
#define _NGX_CLIENT_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <ngx_http.h>


typedef struct ngx_client_session_s     ngx_client_session_t;

typedef void (* ngx_client_connect_pt)(ngx_client_session_t *s);
typedef void (* ngx_client_recv_pt)(ngx_client_session_t *s);
typedef void (* ngx_client_send_pt)(ngx_client_session_t *s);
typedef void (* ngx_client_closed_pt)(ngx_client_session_t *s);


struct ngx_client_session_s {
    ngx_peer_connection_t       peer;
    ngx_str_t                   server; /* server original address */
    in_port_t                   port;   /* server port */

    ngx_connection_t           *connection;

    ngx_pool_t                 *pool;
    ngx_log_t                   log;

    void                       *data;   /* save ctx for callback */

    ngx_chain_t                *out;    /* save data unsend */

    /* configured part */

    /* timer for connecting to server */
    ngx_msec_t                  connect_timeout;

    /* timer for sending buffer full */
    ngx_msec_t                  send_timeout;

    /*
     * data will be postponed until nginx has at least
     * postpone_output bytes of data to send
     */
    size_t                      postpone_output;

    /* use dynamic resolver mechanism for resolving domain */
    unsigned                    dynamic_resolver:1;

    unsigned                    tcp_nodelay:1; /* TCP_NODELAY */
    unsigned                    tcp_nopush:1;  /* TCP_CORK */

    /* runtime part */

    size_t                      recv;        /* client recv bytes */

    unsigned                    connected:1; /* client connected to server */
    unsigned                    closed:1;    /* client has been closed */

    ngx_event_t                 close;       /* for async close */

    /* callback */

    ngx_client_connect_pt       client_connected; /* connect successd */
    ngx_client_recv_pt          client_recv;      /* recv msg from peer */
    ngx_client_send_pt          client_send;      /* send msg to peer */
    ngx_client_closed_pt        client_closed;    /* finalize connection */
};


/*
 * create a client session
 *
 * return value:
 *      return client session for successd, return NULL for failed
 * paras:
 *      peer: server address and port, address could be domain or ip
 *      local: set if need to bind local address, or set NULL
 *      udp: set 1, use udp, set 0, use tcp
 *      log: for logging error when create client session failed
 */
ngx_client_session_t *ngx_client_create(ngx_str_t *peer, ngx_str_t *local,
        ngx_flag_t udp, ngx_log_t *log);


/*
 * connect to client server, should use client session created by
 *  ngx_client_create. before connect to server, user can set paras in
 *  configured part.
 *
 * return value:
 *      void
 * paras:
 *      s: client session created by ngx_client_create
 */
void ngx_client_connect(ngx_client_session_t *s);


/*
 * send data to server
 *
 * return value:
 *      NGX_ERROR: write error, client session will be closed
 *      NGX_AGAIN: data not sent completely, it will save in client session out
 *      NGX_OK:    data sent completely
 * paras:
 *      s: client session
 *      out: data for sending
 */
ngx_int_t ngx_client_write(ngx_client_session_t *s, ngx_chain_t *out);


/*
 * read data from server
 *
 * return value:
 *      NGX_ERROR: read error, client session will be closed
 *      NGX_DECLINED: buf for receiving data is full
 *      NGX_AGAIN: no data for reading
 *      0: server closed
 *      >0: bytes read into buffer
 * paras:
 *      s: client session
 *      b: buffer for receiving data
 */
ngx_int_t ngx_client_read(ngx_client_session_t *s, ngx_buf_t *b);


/*
 * keepalive client connection, and destroy session
 *  if use client connect the same ip:port,
 *  new client session will reuse the connection
 *
 * return value:
 *      void
 * paras:
 *      s: client session
 */
void ngx_client_set_keepalive(ngx_client_session_t *s);


/*
 * close client session
 *
 * return value:
 *      void
 * paras:
 *      s: client session
 */
void ngx_client_close(ngx_client_session_t *s);


/*
 * paras:
 *      r: http request to query status of client
 */
ngx_chain_t *ngx_client_state(ngx_http_request_t *r, unsigned detail);


#endif
