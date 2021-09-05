/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 * fixed by Ping cczjp89@gmail.com
 */


#include <ngx_event_connect.h>
#include "ngx_client.h"
#include "ngx_event_resolver.h"
#include "ngx_dynamic_resolver.h"
#include "ngx_poold.h"
#include "ngx_map.h"
#include "ngx_timerd.h"


#define NGX_CLIENT_DISCARD_BUFFER_SIZE  4096


static void *ngx_client_module_create_conf(ngx_cycle_t *cycle);
static char *ngx_client_module_init_conf(ngx_cycle_t *cycle, void *conf);


typedef struct ngx_client_pool_s  ngx_client_pool_t;


struct ngx_client_pool_s {
    ngx_map_node_t              node;
    ngx_queue_t                 cs_queue;  /* client session queue */
    ngx_uint_t                  qsize;     /* client pool size */

    u_char                      addr[NGX_SOCKADDR_STRLEN];
    ngx_str_t                   paddr;

    ngx_client_pool_t          *next;      /* free pool node */
};


typedef struct {
    ngx_map_t                   client_pools; /* key is ip:port */
    /* max keepalive client session */
    ngx_uint_t                  max_idle_client;
    ngx_msec_t                  keepalive;
    ngx_msec_t                  connect_timeout;
    ngx_msec_t                  send_timeout;
    size_t                      postpone_output;
    ngx_flag_t                  dynamic_resolver;
    ngx_flag_t                  tcp_nodelay;
    ngx_flag_t                  tcp_nopush;

    ngx_uint_t                  idle_connction; /* connection num in pools */
    ngx_uint_t                  nalloc;
    ngx_uint_t                  nfree;
    ngx_client_pool_t          *free;         /* recycle free pool node */
} ngx_client_conf_t;


static ngx_command_t  ngx_client_commands[] = {

    { ngx_string("max_idle_client"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_client_conf_t, max_idle_client),
      NULL },

    { ngx_string("keepalive"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_client_conf_t, keepalive),
      NULL },

    { ngx_string("connect_timeout"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_client_conf_t, connect_timeout),
      NULL },

    { ngx_string("send_timeout"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      0,
      offsetof(ngx_client_conf_t, send_timeout),
      NULL },

    { ngx_string("postpone_output"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      0,
      offsetof(ngx_client_conf_t, postpone_output),
      NULL },

    { ngx_string("dynamic_resolver"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_client_conf_t, dynamic_resolver),
      NULL },

    { ngx_string("tcp_nodelay"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_client_conf_t, tcp_nodelay),
      NULL },

    { ngx_string("tcp_nopush"),
      NGX_MAIN_CONF|NGX_DIRECT_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      0,
      offsetof(ngx_client_conf_t, tcp_nopush),
      NULL },

      ngx_null_command
};


static ngx_core_module_t    ngx_client_module_ctx = {
    ngx_string("client"),
    ngx_client_module_create_conf,
    ngx_client_module_init_conf
};


ngx_module_t  ngx_client_module = {
    NGX_MODULE_V1,
    &ngx_client_module_ctx,                 /* module context */
    ngx_client_commands,                    /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_client_module_create_conf(ngx_cycle_t *cycle)
{
    ngx_client_conf_t          *ccf;

    ccf = ngx_pcalloc(cycle->pool, sizeof(ngx_client_conf_t));
    if (ccf == NULL) {
        return NULL;
    }

    ngx_map_init(&ccf->client_pools, ngx_map_hash_str, ngx_cmp_str);

    ccf->max_idle_client = NGX_CONF_UNSET_UINT;
    ccf->keepalive = NGX_CONF_UNSET_MSEC;
    ccf->connect_timeout = NGX_CONF_UNSET_MSEC;
    ccf->send_timeout = NGX_CONF_UNSET_MSEC;
    ccf->postpone_output = NGX_CONF_UNSET_SIZE;
    ccf->dynamic_resolver = NGX_CONF_UNSET;
    ccf->tcp_nodelay = NGX_CONF_UNSET;
    ccf->tcp_nodelay = NGX_CONF_UNSET;

    return ccf;
}


static char *
ngx_client_module_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_client_conf_t          *ccf = conf;

    ngx_conf_init_uint_value(ccf->max_idle_client, 1024);
    ngx_conf_init_msec_value(ccf->keepalive, 60000);

    ngx_conf_init_msec_value(ccf->connect_timeout, 3000);
    ngx_conf_init_msec_value(ccf->send_timeout, 10000);
    ngx_conf_init_size_value(ccf->postpone_output, 1460);
    ngx_conf_init_value(ccf->dynamic_resolver, 1);
    ngx_conf_init_value(ccf->tcp_nodelay, 1);
    ngx_conf_init_value(ccf->tcp_nopush, 0);

    return NGX_CONF_OK;
}


/* client pool */
static ngx_client_pool_t *
ngx_client_get_client_pool()
{
    ngx_client_conf_t          *ccf;
    ngx_client_pool_t          *pool;

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    pool = ccf->free;
    if (pool == NULL) {
        pool = ngx_pcalloc(ngx_cycle->pool, sizeof(ngx_client_pool_t));
        if (pool == NULL) {
            return NULL;
        }

        ++ccf->nalloc;
    } else {
        ccf->free = pool->next;
        ngx_memzero(pool, sizeof(ngx_client_pool_t));

        --ccf->nfree;
    }

    ngx_queue_init(&pool->cs_queue);

    return pool;
}


static void
ngx_client_put_client_pool(ngx_client_pool_t *p)
{
    ngx_client_conf_t          *ccf;

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    p->next = ccf->free;
    ccf->free = p;

    ++ccf->nfree;
}


static ngx_connection_t *
ngx_client_get_connection(struct sockaddr *sockaddr, socklen_t socklen)
{
    ngx_client_conf_t          *ccf;
    ngx_client_pool_t          *pool;
    ngx_map_node_t             *node;
    ngx_queue_t                *cq;
    ngx_str_t                   paddr;
    u_char                      addr[NGX_SOCKADDR_STRLEN];
    ngx_connection_t           *c;

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

#if (NGX_HAVE_UNIX_DOMAIN)
    if (sockaddr->sa_family == AF_UNIX) { // Unix will not reuse
        return NULL;
    }
#endif

    // get client connection pool for sockaddr
    paddr.data = addr;
    paddr.len = NGX_SOCKADDR_STRLEN;
    paddr.len = ngx_sock_ntop(sockaddr, socklen, paddr.data, paddr.len, 1);

    node = ngx_map_find(&ccf->client_pools, (intptr_t) &paddr);

    if (node == NULL) { // connection pool for addr is empty
        return NULL;
    }

    // get a idle connection from client connection pool
    pool = (ngx_client_pool_t *) node;
    cq = ngx_queue_head(&pool->cs_queue);
    ngx_queue_remove(cq);
    c = (ngx_connection_t *) ((char *) cq - offsetof(ngx_connection_t, queue));
    --ccf->idle_connction;
    --pool->qsize;

    // recycle empty pool
    if (ngx_queue_empty(&pool->cs_queue)) {
        ngx_map_delete(&ccf->client_pools, (intptr_t) &paddr);
        ngx_client_put_client_pool(pool);
    }

    // init connection
    if (c->read->timer_set) {
        NGX_DEL_TIMER(c->read, c->number);
    }

    if (c->read->posted) {
        ngx_delete_posted_event(c->read);
    }

    return c;
}


static void
ngx_client_keepalive_handler(ngx_event_t *rev)
{
    ngx_client_conf_t          *ccf;
    ngx_client_pool_t          *pool;
    ngx_connection_t           *c;
    ngx_int_t                   n;
    ngx_buf_t                   b;
    u_char                      buffer[NGX_CLIENT_DISCARD_BUFFER_SIZE];

    c = rev->data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, c->log, 0, "client keepalive handler");

    if (rev->timedout || c->close) {
        goto close;
    }

    // read and discard data
    b.start = buffer;
    b.end = buffer + NGX_CLIENT_DISCARD_BUFFER_SIZE;

    for (;;) {
        b.pos = b.last = b.start;

        n = c->recv(c, b.last, b.end - b.last);

        if (n == 0 || n == NGX_ERROR) {
            ngx_log_error(NGX_LOG_INFO, c->log, ngx_errno,
                    "server close while client keepalive");
            goto close;
        }

        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "server recv data while client keepalive %d", n);

        goto close;
    }

close:
    // remove connection from pool
    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    pool = c->data;

    ngx_queue_remove(&c->queue);
    --ccf->idle_connction;
    --pool->qsize;

    // recycle empty pool
    if (ngx_queue_empty(&pool->cs_queue)) {
        ngx_map_delete(&ccf->client_pools, (intptr_t) &pool->paddr);
        ngx_client_put_client_pool(pool);
    }

    if (c->read->timer_set) {
        NGX_DEL_TIMER(c->read, c->number);
    }

    if (c->write->timer_set) {
        NGX_DEL_TIMER(c->write, c->number);
    }

    ngx_close_connection(c);
}


static void
ngx_client_reusable_connection(ngx_client_session_t *s)
{
    ngx_client_conf_t          *ccf;
    ngx_client_pool_t          *pool;
    ngx_map_node_t             *node;
    ngx_connection_t           *c;
    ngx_str_t                   paddr;
    u_char                      addr[NGX_SOCKADDR_STRLEN];

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    c = s->connection;

#if (NGX_HAVE_UNIX_DOMAIN)
    if (s->peer.sockaddr->sa_family == AF_UNIX) { // Unix will not reuse
        return;
    }
#endif

    if (ccf->idle_connction > ccf->keepalive) {
        ngx_log_error(NGX_LOG_INFO, c->log, 0,
                "too many connections in pool");
        return;
    }

    ngx_log_error(NGX_LOG_INFO, &s->log, 0, "client, put connection in pool");

    c->pool = NULL;

    // get client connection pool for c->sockaddr
    paddr.data = addr;
    paddr.len = NGX_SOCKADDR_STRLEN;
    paddr.len = ngx_sock_ntop(s->peer.sockaddr, s->peer.socklen,
                              paddr.data, paddr.len, 1);

    node = ngx_map_find(&ccf->client_pools, (intptr_t) &paddr);
    pool = (ngx_client_pool_t *) node;

    if (pool == NULL) { // connection pool for addr is empty
        pool = ngx_client_get_client_pool();
        if (pool == NULL) {
            return;
        }

        ngx_memcpy(pool->addr, addr, NGX_SOCKADDR_STRLEN);
        pool->paddr.data = pool->addr;
        pool->paddr.len = paddr.len;

        pool->node.raw_key = (intptr_t) &pool->paddr;
        ngx_map_insert(&ccf->client_pools, &pool->node, 0);
    }

    // put connection in connection pool
    ngx_queue_insert_head(&pool->cs_queue, &c->queue);
    c->data = pool;
    ++ccf->idle_connction;
    ++pool->qsize;

    c->log = ngx_cycle->log;
    c->read->log = ngx_cycle->log;

    // set timer for keepalive time
    c->read->handler = ngx_client_keepalive_handler;
    NGX_ADD_TIMER(c->read, ccf->keepalive, offsetof(ngx_connection_t, number));

    if (c->write->timer_set) {
        NGX_DEL_TIMER(c->write, c->number);
    }

    if (c->write->posted) {
        ngx_delete_posted_event(c->write);
    }

    if (c->write->active && (ngx_event_flags & NGX_USE_LEVEL_EVENT)) {
        if (ngx_del_event(c->write, NGX_WRITE_EVENT, 0) != NGX_OK) {
            if (c->read->timer_set) {
                NGX_DEL_TIMER(c->read, c->number);
            }

            ngx_close_connection(c);
            return;
        }
    }
}


/* client */

static u_char *
ngx_client_log_error(ngx_log_t *log, u_char *buf, size_t len)
{
    u_char                     *p;
    ngx_client_session_t       *s;

    p = buf;

    if (log->action) {
        p = ngx_snprintf(buf, len, " while %s", log->action);
        len -= p - buf;
        buf = p;
    }

    s = log->data;
    if (s == NULL) {
        return p;
    }

    if (s->connection) {
        p = ngx_snprintf(buf, len, ", server ip: %V",
                &s->connection->addr_text);
        len -= p - buf;
        buf = p;
    }

    p = ngx_snprintf(buf, len, ", server: %V, csession: %p", &s->server, s);
    len -= p - buf;
    buf = p;

    return p;
}


static ngx_int_t
ngx_client_get_peer(ngx_peer_connection_t *pc, void *data)
{
    ngx_connection_t           *c;

    c = ngx_client_get_connection(pc->sockaddr, pc->socklen);
    if (c == NULL) { // cannot find reusable keepalive connection
        return NGX_OK;
    }

    c->idle = 0;
    c->sent = 0;
    c->log = pc->log;
    c->read->log = pc->log;
    c->write->log = pc->log;

    pc->connection = c;
    pc->cached = 1;

    return NGX_DONE;
}


static void
ngx_client_free_peer(ngx_peer_connection_t *pc, void *data,
        ngx_uint_t state)
{
}


static ngx_int_t
ngx_client_test_connect(ngx_connection_t *c)
{
    int        err;
    socklen_t  len;

#if (NGX_HAVE_KQUEUE)

    if (ngx_event_flags & NGX_USE_KQUEUE_EVENT)  {
        if (c->write->pending_eof || c->read->pending_eof) {
            if (c->write->pending_eof) {
                err = c->write->kq_errno;

            } else {
                err = c->read->kq_errno;
            }

            (void) ngx_connection_error(c, err,
                    "kevent() reported that connect() failed");
            return NGX_ERROR;
        }

    } else
#endif
    {
        err = 0;
        len = sizeof(int);

        /*
         * BSDs and Linux return 0 and set a pending error in err
         * Solaris returns -1 and sets errno
         */

        if (getsockopt(c->fd, SOL_SOCKET, SO_ERROR, (void *) &err, &len)
                == -1)
        {
            err = ngx_socket_errno;
        }

        if (err) {
            (void) ngx_connection_error(c, err, "connect() failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static void
ngx_client_connected(ngx_client_session_t *s)
{
    ngx_event_t                *wev;

    if (ngx_client_test_connect(s->connection) != NGX_OK) {
        ngx_client_close(s);
        return;
    }

    s->log.action = NULL;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client connected");

    wev = s->connection->write;

    if (wev->timedout) { /* rev or wev timedout */
        ngx_log_error(NGX_LOG_ERR, &s->log, NGX_ETIMEDOUT,
                "server timed out");
        s->connection->timedout = 1;

        ngx_client_close(s);

        return;
    }

    if (wev->timer_set) {
        NGX_DEL_TIMER(wev, s->connection->number);
    }

    s->connected = 1;
    s->peer.tries = 0;

    if (s->client_connected) {
        s->client_connected(s);
    }
}


static void
ngx_client_write_handler(ngx_event_t *ev)
{
    ngx_connection_t           *c;
    ngx_client_session_t       *s;
    ngx_int_t                   n;

    c = ev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client write handler");

    if (!s->connected) {
        ngx_client_connected(s);

        return;
    }

    if (s->client_send) {
        s->client_send(s);

        return;
    }

    /* write data buffered in s->out */
    if (s->out == NULL) {
        return;
    }

    n = ngx_client_write(s, NULL);
    if (n == NGX_ERROR) {
        ngx_client_close(s);
    }
}


static void
ngx_client_read_discarded(ngx_client_session_t *s)
{
    ngx_int_t                   n;
    ngx_buf_t                   b;
    u_char                      buffer[NGX_CLIENT_DISCARD_BUFFER_SIZE];

    b.start = buffer;
    b.end = buffer + NGX_CLIENT_DISCARD_BUFFER_SIZE;

    for (;;) {
        b.pos = b.last = b.start;

        n = ngx_client_read(s, &b);

        if (n == NGX_ERROR || n == 0) {
            ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno,
                    "nginx client read discard error");
            ngx_client_close(s);

            return;
        }

        if (n == NGX_AGAIN) {
            return;
        }
    }
}


static void
ngx_client_read_handler(ngx_event_t *ev)
{
    ngx_connection_t           *c;
    ngx_client_session_t       *s;

    c = ev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client read handler");

    if (!s->connected) {
        ngx_client_connected(s);

        return;
    }

    if (s->client_recv) {
        s->client_recv(s);
    } else {
        /* read and drop */
        ngx_client_read_discarded(s);
    }
}


static void
ngx_client_connect_server(void *data, struct sockaddr *sa, socklen_t socklen)
{
    ngx_client_session_t       *s;
    ngx_connection_t           *c;
    ngx_int_t                   rc;
    int                         tcp_nodelay;

    s = data;

    if (sa == NULL) {
        ngx_log_error(NGX_LOG_ERR, &s->log, 0,
                "nginx client resolver peer %v failed", &s->server);
        goto failed;
    }

    ngx_inet_set_port(sa, s->port);

    s->peer.sockaddr = ngx_pcalloc(s->pool, sizeof(socklen));
    ngx_memcpy(s->peer.sockaddr, sa, socklen);
    s->peer.socklen = socklen;
    s->peer.name = &s->server;

    s->log.action = "connecting to server";

    rc = ngx_event_connect_peer(&s->peer);
    if (rc == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno,
                "nginx client connect peer failed");
        goto failed;
    } else if (rc == NGX_DECLINED) {
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno,
                "nginx client connect peer failed");
        goto failed;
    }

    if (rc == NGX_DONE) {
        ngx_log_error(NGX_LOG_INFO, &s->log, 0, "client, reuse connection");
    }

    // NGX_AGAIN: send syn, wait for syn,ack
    // NGX_OK:    connect to server
    // NGX_DONE:  reuse keepalive connection

    s->connection = s->peer.connection;
    c = s->connection;
    c->pool = s->pool;
    c->pool->log = s->peer.log;

    // set tcp_nodelay
    if (c->type == SOCK_STREAM && s->tcp_nodelay
            && c->tcp_nodelay == NGX_TCP_NODELAY_UNSET)
    {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, c->log, 0, "tcp_nodelay");

        tcp_nodelay = 1;

        if (setsockopt(c->fd, IPPROTO_TCP, TCP_NODELAY,
                       (const void *) &tcp_nodelay, sizeof(int)) == -1)
        {
            ngx_connection_error(c, ngx_socket_errno,
                                 "setsockopt(TCP_NODELAY) failed");
            goto failed;
        }

        c->tcp_nodelay = NGX_TCP_NODELAY_SET;
    }

    // set tcp_nopush
    if (!s->tcp_nopush) {
        c->tcp_nopush = NGX_TCP_NOPUSH_DISABLED;
    }

    c->addr_text.data = ngx_pcalloc(s->pool, NGX_SOCKADDR_STRLEN);
    if (c->addr_text.data == NULL) {
        goto failed;
    }
    c->addr_text.len = ngx_sock_ntop(sa, socklen, c->addr_text.data,
                                     NGX_SOCKADDR_STRLEN, 1);

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client connect server, rc: %i", rc);

    c->log->connection = c->number;

    c->data = s;

    c->write->handler = ngx_client_write_handler;
    c->read->handler = ngx_client_read_handler;

    if (rc == NGX_AGAIN) {
        NGX_ADD_TIMER(c->write, s->connect_timeout,
                      offsetof(ngx_connection_t, number));
        return;
    }

    /* NGX_OK */

    ngx_client_connected(s);

    return;

failed:
    ngx_client_close(s);
}


static void
ngx_client_resolver_server(void *data, ngx_resolver_addr_t *addrs,
        ngx_uint_t naddrs)
{
    ngx_client_session_t       *s;
    ngx_uint_t                  n;

    s = data;

    if (naddrs == 0) {
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno,
                "nginx client resolver failed");
        ngx_client_close(s);
        return;
    }

    n = ngx_random() % naddrs;

    ngx_client_connect_server(data, addrs[n].sockaddr, addrs[n].socklen);
}


static void
ngx_client_close_connection(ngx_client_session_t *s)
{
    ngx_connection_t           *c;

    c = s->connection;

    if (c == NULL || c->destroyed) {
        return;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client close connection");

    s->connected = 0;
    s->connection = NULL;
    c->destroyed = 1;

    if (c->write->timer_set) {
        NGX_DEL_TIMER(c->write, c->number);
    }

    if (c->read->timer_set) {
        NGX_DEL_TIMER(c->read, c->number);
    }

    ngx_close_connection(c);
}


ngx_client_session_t *
ngx_client_create(ngx_str_t *peer, ngx_str_t *local, ngx_flag_t udp,
        ngx_log_t *log)
{
    ngx_client_session_t       *s;
    ngx_client_conf_t          *ccf;
    ngx_pool_t                 *pool;
    ngx_int_t                   rc, n;
    u_char                     *p, *last;
    size_t                      plen;

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    if (peer == NULL || peer->len == 0) {
        ngx_log_error(NGX_LOG_ERR, log, 0, "client init, peer is NULL");
        return NULL;
    }

    pool = NGX_CREATE_POOL(4096, ngx_cycle->log);
    if (pool == NULL) {
        return NULL;
    }

    s = ngx_pcalloc(pool, sizeof(ngx_client_session_t));
    if (s == NULL) {
        goto clear;
    }
    s->pool = pool;

    /* set log */
    // ci->log.connection not set, should set when connection established
    s->log = ngx_cycle->new_log;
    s->log.handler = ngx_client_log_error;
    s->log.data = s;
    s->log.action = "create client";

    s->log.log_level = NGX_LOG_INFO;

    /* parse peer */
    last = peer->data + peer->len;

#if (NGX_HAVE_INET6)
    if (peer->len && peer->data[0] == '[') {

        p = ngx_strlchr(peer->data, last, ']');

        if (p == NULL || p == last - 1) {
            ngx_log_error(NGX_LOG_ERR, log, 0,
                    "client init, parse peer %V error", peer);
            goto clear;
        }

        ++p;
    } else
#endif
    {
        p = ngx_strlchr(peer->data, last, ':');
        if (p == NULL) {
            p = last;
        }
    }

    s->server.len = p - peer->data;
    s->server.data = ngx_pcalloc(s->pool, s->server.len);
    if (s->server.data == NULL) {
        goto clear;
    }
    ngx_memcpy(s->server.data, peer->data, peer->len);

    if (p != last) { /* has port */
        if (*p != ':') {
            ngx_log_error(NGX_LOG_ERR, log, 0,
                    "client init, parse peer %V error", peer);
            goto clear;
        }

        ++p;
        plen = last - p;

        n = ngx_atoi(p, plen);
        if (n < 1 || n > 65535) {
            ngx_log_error(NGX_LOG_ERR, log, 0,
                    "client init, parse peer %V error", peer);
            goto clear;
        }
        s->port = n;
    }

    /* parse local */
    if (local && local->len) {
        s->peer.local = ngx_pcalloc(s->pool, sizeof(ngx_addr_t));
        if (s->peer.local == NULL) {
            goto clear;
        }

        rc = ngx_parse_addr_port(s->pool, s->peer.local, peer->data, peer->len);
        if (rc != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, log, 0, "invalid local address \"%V\"",
                    local);
            goto clear;
        }

        s->peer.local->name.data = ngx_pcalloc(s->pool, sizeof(local->len));
        if (s->peer.local->name.data == NULL) {
            goto clear;
        }
        ngx_memcpy(s->peer.local->name.data, local->data, local->len);
        s->peer.local->name.len = local->len;
    }

    /* set default */
    s->connect_timeout = ccf->connect_timeout;
    s->send_timeout = ccf->send_timeout;
    s->postpone_output = ccf->postpone_output;
    s->dynamic_resolver = ccf->dynamic_resolver;
    s->tcp_nodelay = ccf->tcp_nodelay;
    s->tcp_nopush = ccf->tcp_nopush;

    /* set peer */
    s->peer.log = &s->log;
    s->peer.get = ngx_client_get_peer;
    s->peer.free = ngx_client_free_peer;
    s->peer.type = udp ? SOCK_DGRAM : SOCK_STREAM;
    s->peer.log_error = NGX_ERROR_INFO;

    return s;

clear:
    NGX_DESTROY_POOL(pool);

    return NULL;
}


void
ngx_client_connect(ngx_client_session_t *s)
{
    s->log.action = "resolving";

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, &s->log, 0, "nginx client connect %V",
            &s->server);

    /* start connect */
    if (s->dynamic_resolver) {
        ngx_dynamic_resolver_start_resolver(&s->server,
                ngx_client_connect_server, s);
    } else {
        ngx_event_resolver_start_resolver(&s->server,
                ngx_client_resolver_server, s);
    }
}


ngx_int_t
ngx_client_write(ngx_client_session_t *s, ngx_chain_t *out)
{
    off_t                       size;
    ngx_uint_t                  last, flush, sync;
    ngx_chain_t                *cl, *ln, **ll, *chain;
    ngx_connection_t           *c;
    ngx_event_t                *wev;

    c = s->peer.connection;
    wev = c->write;

    if (c->error) {
        return NGX_ERROR;
    }

    size = 0;
    flush = 0;
    sync = 0;
    last = 0;
    ll = &s->out;

    /* find the size, the flush point and the last link of the saved chain */

    for (cl = s->out; cl; cl = cl->next) {
        ll = &cl->next;

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, &s->log, 0,
                       "nginx client write, write old buf t:%d f:%d %p, "
                       "pos %p, size: %z file: %O, size: %O",
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);

#if 1
        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, &s->log, 0,
                          "nginx client write, zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }
#endif

        size += ngx_buf_size(cl->buf);

        if (cl->buf->flush || cl->buf->recycled) {
            flush = 1;
        }

        if (cl->buf->sync) {
            sync = 1;
        }

        if (cl->buf->last_buf) {
            last = 1;
        }
    }

    /* add the new chain to the existent one */

    for (ln = out; ln; ln = ln->next) {
        cl = ngx_alloc_chain_link(s->pool);
        if (cl == NULL) {
            return NGX_ERROR;
        }

        cl->buf = ln->buf;
        *ll = cl;
        ll = &cl->next;

        ngx_log_debug7(NGX_LOG_DEBUG_EVENT, &s->log, 0,
                       "nginx client write, write new buf t:%d f:%d %p, "
                       "pos %p, size: %z file: %O, size: %O",
                       cl->buf->temporary, cl->buf->in_file,
                       cl->buf->start, cl->buf->pos,
                       cl->buf->last - cl->buf->pos,
                       cl->buf->file_pos,
                       cl->buf->file_last - cl->buf->file_pos);

#if 1
        if (ngx_buf_size(cl->buf) == 0 && !ngx_buf_special(cl->buf)) {
            ngx_log_error(NGX_LOG_ALERT, &s->log, 0,
                          "nginx client write, zero size buf in writer "
                          "t:%d r:%d f:%d %p %p-%p %p %O-%O",
                          cl->buf->temporary,
                          cl->buf->recycled,
                          cl->buf->in_file,
                          cl->buf->start,
                          cl->buf->pos,
                          cl->buf->last,
                          cl->buf->file,
                          cl->buf->file_pos,
                          cl->buf->file_last);

            ngx_debug_point();
            return NGX_ERROR;
        }
#endif

        size += ngx_buf_size(cl->buf);

        if (cl->buf->flush || cl->buf->recycled) {
            flush = 1;
        }

        if (cl->buf->sync) {
            sync = 1;
        }

        if (cl->buf->last_buf) {
            last = 1;
        }
    }

    *ll = NULL;

    ngx_log_debug3(NGX_LOG_DEBUG_HTTP, &s->log, 0, "nginx client write, "
                   "http write filter: l:%ui f:%ui s:%O", last, flush, size);

    /*
     * avoid the output if there are no last buf, no flush point,
     * there are the incoming bufs and the size of all bufs
     * is smaller than "postpone_output" directive
     */

    if (!last && !flush && out && size < (off_t) s->postpone_output) {
        return NGX_OK;
    }

    if (size == 0 && !(last && c->need_last_buf)) {
        if (last || flush || sync) {
            for (cl = s->out; cl; /* void */) {
                ln = cl;
                cl = cl->next;
                ngx_free_chain(s->pool, ln);
            }

            s->out = NULL;

            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ALERT, &s->log, 0,
                      "nginx client write, the output chain is empty");

        ngx_debug_point();

        return NGX_ERROR;
    }

    chain = c->send_chain(c, s->out, 0);

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, &s->log, 0,
                   "nginx client write %p", chain);

    if (chain == NGX_CHAIN_ERROR) {
        c->error = 1;
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno, "write error");
        return NGX_ERROR;
    }

    for (cl = s->out; cl && cl != chain; /* void */) {
        ln = cl;
        cl = cl->next;
        ngx_free_chain(s->pool, ln);
    }

    s->out = chain;

    if (chain) {
        NGX_ADD_TIMER(c->write, s->send_timeout,
                      offsetof(ngx_connection_t, number));
        if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        return NGX_AGAIN;
    }

    if (wev->active) { /* if NGX_OK, del write notification */
        if (ngx_del_event(wev, NGX_WRITE_EVENT, 0) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_client_read(ngx_client_session_t *s, ngx_buf_t *b)
{
    ngx_connection_t           *c;
    ngx_int_t                   n;

    if (s == NULL || b == NULL) {
        return NGX_ERROR;
    }

    if (b->last == b->end) {
        return NGX_DECLINED;
    }

    c = s->peer.connection;

    n = c->recv(c, b->last, b->end - b->last);

    if (n == 0) {
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno, "server closed");
        return 0;
    }

    if (n == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, &s->log, ngx_errno, "read error");
        return NGX_ERROR;
    }

    if (n == NGX_AGAIN) {
        if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
            return NGX_ERROR;
        }

        return NGX_AGAIN;
    }

    b->last += n;
    s->recv += n;

    return n;
}


void
ngx_client_set_keepalive(ngx_client_session_t *s)
{
    ngx_pool_t                 *pool;

    if (s->closed) {
        return;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0,
            "nginx client set keepalive");

    ngx_client_reusable_connection(s);

    pool = s->pool;
    NGX_DESTROY_POOL(pool);
}


static void
ngx_client_close_handler(ngx_event_t *ev)
{
    ngx_client_session_t       *s;
    ngx_pool_t                 *pool;
    ngx_client_closed_pt        closed;

    s = ev->data;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, &s->log, 0, "nginx client close");

    if (s->client_closed) {
        closed = s->client_closed;
        s->client_closed = NULL;

        closed(s);
    }

    ngx_client_close_connection(s);

    pool = s->pool;
    NGX_DESTROY_POOL(pool); /* s alloc from pool */
}


void
ngx_client_close(ngx_client_session_t *s)
{
    ngx_event_t                *e;

    if (s->closed) {
        return;
    }

    s->log.action = "close";

    s->closed = 1;

    e = &s->close;
    e->data = s;
    e->handler = ngx_client_close_handler;
    e->log = &s->log;

    ngx_post_event(e, &ngx_posted_events);
}


ngx_chain_t *
ngx_client_state(ngx_http_request_t *r, unsigned detail)
{
    ngx_client_conf_t          *ccf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    ngx_map_node_t             *node;
    ngx_client_pool_t          *pool;
    size_t                      len, len1;
    ngx_uint_t                  n;

    ccf = (ngx_client_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                             ngx_client_module);

    len = sizeof("##########ngx client connection pool##########\n") - 1
        + sizeof("ngx_client_pool nalloc node: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_client_pool nfree node: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_client_pool idle connection: \n") - 1 + NGX_OFF_T_LEN;

    len1 = 0;

    /* node for create pool */
    if (detail) {
        n = ccf->nalloc - ccf->nfree;
        /* "    addr:port: qsize\n" */
        len1 = 4 + NGX_SOCKADDR_STRLEN + 2 + NGX_OFF_T_LEN + 1;
        len += len1 * n;
    }

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NULL;
    }
    cl->next = NULL;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NULL;
    }
    cl->buf = b;

    b->last = ngx_snprintf(b->last, len,
            "##########ngx client connection pool##########\n"
            "ngx_client_pool nalloc node: %ui\n"
            "ngx_client_pool nfree node: %ui\n"
            "ngx_client_pool idle connection: %ui\n",
            ccf->nalloc, ccf->nfree, ccf->idle_connction);

    if (detail) {
        for (node = ngx_map_begin(&ccf->client_pools); node;
                node = ngx_map_next(node))
        {
            /* m is first element of ngx_poold_node_t */
            pool = (ngx_client_pool_t *) node;
            b->last = ngx_snprintf(b->last, len1, "    %V: %ui\n",
                    &pool->paddr, pool->qsize);
        }
    }

    return cl;
}
