/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include "ngx_multiport.h"


static ngx_int_t ngx_event_multiport_process_init(ngx_cycle_t *cycle);
static void ngx_event_multiport_process_exit(ngx_cycle_t *cycle);

static void *ngx_event_multiport_create_conf(ngx_cycle_t *cycle);
static char *ngx_event_multiport_init_conf(ngx_cycle_t *cycle, void *conf);

static char *ngx_event_multiport_listen(ngx_conf_t *cf, ngx_command_t *cmd,
            void *conf);


//multiport_listen listenparas relationport
typedef struct {
    ngx_str_t               multiport;
    ngx_str_t               relatedport;
    ngx_listening_t         listening;
} ngx_event_multiport_listen_t;

typedef struct {
    ngx_array_t            *ports; /* array of ngx_event_multiport_listen_t */
} ngx_event_multiport_conf_t;


static ngx_str_t  event_multiport_name = ngx_string("event_multiport");


static ngx_command_t  ngx_event_multiport_commands[] = {

    { ngx_string("multi_listen"),
      NGX_EVENT_CONF|NGX_CONF_TAKE2,
      ngx_event_multiport_listen,
      0,
      0,
      NULL },

      ngx_null_command
};


ngx_event_module_t  ngx_event_multiport_module_ctx = {
    &event_multiport_name,
    ngx_event_multiport_create_conf,        /* create configuration */
    ngx_event_multiport_init_conf,          /* init configuration */

    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};


/* this module use ngx_cycle->log */
ngx_module_t  ngx_event_multiport_module = {
    NGX_MODULE_V1,
    &ngx_event_multiport_module_ctx,        /* module context */
    ngx_event_multiport_commands,           /* module directives */
    NGX_EVENT_MODULE,                       /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_event_multiport_process_init,       /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    ngx_event_multiport_process_exit,       /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_event_multiport_create_conf(ngx_cycle_t *cycle)
{
    ngx_event_multiport_conf_t     *conf;

    conf = ngx_pcalloc(cycle->pool, sizeof(ngx_event_multiport_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

static char *
ngx_event_multiport_init_conf(ngx_cycle_t *cycle, void *conf)
{
    return NGX_CONF_OK;
}

static char *
ngx_event_multiport_listen(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_event_multiport_conf_t     *emcf = conf;

    ngx_str_t                      *value;
    ngx_event_multiport_listen_t   *mls;
    ngx_url_t                       u;

    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    /* check listen base port */
    u.url = value[1];
    u.listen = 1;
    u.default_port = 0;

    if (ngx_parse_url(cf->temp_pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                    "%s in \"%V\" of the \"multi_listen"
                    " baseport\" directive",
                    u.err, &u.url);
        }

        return NGX_CONF_ERROR;
    }

    ngx_memzero(&u, sizeof(ngx_url_t));

    /* check relation port */
    u.url = value[2];
    u.listen = 1;
    u.default_port = 0;

    if (ngx_parse_url(cf->temp_pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                    "%s in \"%V\" of the \"multi_listen"
                    " relationport\" directive",
                    u.err, &u.url);
        }

        return NGX_CONF_ERROR;
    }

    if (emcf->ports == NULL) {
        emcf->ports = ngx_array_create(cf->pool, 4,
                                       sizeof(ngx_event_multiport_listen_t));
        if (emcf->ports == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    mls = ngx_array_push(emcf->ports);
    if (mls == NULL) {
        return NGX_CONF_ERROR;
    }
    mls->multiport = value[1];
    mls->relatedport = value[2];

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_event_multiport_init_listening(ngx_cycle_t *cycle, ngx_listening_t *ls,
        void *sockaddr, socklen_t socklen, ngx_listening_t *cls)
{
    size_t            len;
    struct sockaddr  *sa;
    u_char            text[NGX_SOCKADDR_STRLEN];

    ngx_memzero(ls, sizeof(ngx_listening_t));

    sa = ngx_palloc(cycle->pool, socklen);
    if (sa == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(sa, sockaddr, socklen);

    ls->sockaddr = sa;
    ls->socklen = socklen;

    len = ngx_sock_ntop(sa, socklen, text, NGX_SOCKADDR_STRLEN, 1);
    ls->addr_text.len = len;

    switch (ls->sockaddr->sa_family) {
#if (NGX_HAVE_INET6)
    case AF_INET6:
        ls->addr_text_max_len = NGX_INET6_ADDRSTRLEN;
        break;
#endif
#if (NGX_HAVE_UNIX_DOMAIN)
    case AF_UNIX:
        ls->addr_text_max_len = NGX_UNIX_ADDRSTRLEN;
        len++;
        break;
#endif
    case AF_INET:
        ls->addr_text_max_len = NGX_INET_ADDRSTRLEN;
        break;
    default:
        ls->addr_text_max_len = NGX_SOCKADDR_STRLEN;
        break;
    }

    ls->addr_text.data = ngx_pnalloc(cycle->pool, len);
    if (ls->addr_text.data == NULL) {
        return NGX_ERROR;
    }

    ngx_memcpy(ls->addr_text.data, text, len);

#if (NGX_HAVE_UNIX_DOMAIN)
    if (ls->sockaddr->sa_family == AF_UNIX) {
        u_char *name = ls->addr_text.data + sizeof("unix:") - 1;
        ngx_delete_file(name);
    }
#endif

    ls->fd = (ngx_socket_t) -1;
    ls->type = SOCK_STREAM;

    ls->backlog = cls->backlog;
    ls->rcvbuf = cls->rcvbuf;
    ls->sndbuf = cls->sndbuf;

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
    ls->keepidle = cls->keepidle;
    ls->keepintvl = cls->keepintvl;
    ls->keepcnt = cls->keepcnt;
#endif

    ls->addr_ntop = cls->addr_ntop;
    ls->handler = cls->handler;
    ls->servers = cls->servers;
    ls->worker = ngx_worker;

    ls->log= cls->log;
    ls->logp = cls->logp;

    ls->pool_size = cls->pool_size;
    ls->post_accept_buffer_size = cls->post_accept_buffer_size;
    ls->post_accept_timeout = cls->post_accept_timeout;

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
    ls->ipv6only = cls->ipv6only;
#endif

    ls->keepalive = cls->keepalive;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    ls->deferred_accept = cls->deferred_accept;
    ls->delete_deferred = cls->delete_deferred;
    ls->add_deferred = cls->add_deferred;
#endif

#ifdef SO_ACCEPTFILTER
    ls->accept_filter = cls->accept_filter;
#endif

#if (NGX_HAVE_SETFIB)
    ls->setfib = cls->setfib;
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    ls->fastopen = cls->fastopen;
#endif

    return NGX_OK;
}

static ngx_int_t
ngx_event_multiport_open_listening_sock(ngx_cycle_t *cycle, ngx_listening_t *ls)
{
    int               reuseaddr;
    ngx_uint_t        tries, failed;
    ngx_err_t         err;
    ngx_log_t        *log;
    ngx_socket_t      s;

    reuseaddr = 1;
#if (NGX_SUPPRESS_WARN)
    failed = 0;
#endif

    log = cycle->log;

    /* TODO: configurable try number */

    for (tries = 5; tries; tries--) {
        failed = 0;

        if (ls->fd != (ngx_socket_t) -1) {
            return NGX_OK;
        }

        s = ngx_socket(ls->sockaddr->sa_family, ls->type, 0);

        if (s == (ngx_socket_t) -1) {
            ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                          "multiport, " ngx_socket_n " %V failed",
                          &ls->addr_text);
            return NGX_ERROR;
        }

        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
                       (const void *) &reuseaddr, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                          "multiport, setsockopt(SO_REUSEADDR) %V failed",
                          &ls->addr_text);

            if (ngx_close_socket(s) == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "multiport, " ngx_close_socket_n " %V failed",
                              &ls->addr_text);
            }

            return NGX_ERROR;
        }

#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)

        if (ls->sockaddr->sa_family == AF_INET6) {
            int  ipv6only;

            ipv6only = ls->ipv6only;

            if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY,
                           (const void *) &ipv6only, sizeof(int))
                == -1)
            {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "multiport, setsockopt(IPV6_V6ONLY) %V failed,"
                              " ignored", &ls->addr_text);
            }
        }
#endif
        /* TODO: close on exit */

        if (!(ngx_event_flags & NGX_USE_IOCP_EVENT)) {
            if (ngx_nonblocking(s) == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "multiport, " ngx_nonblocking_n " %V failed",
                              &ls->addr_text);

                if (ngx_close_socket(s) == -1) {
                    ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                                  "multiport, " ngx_close_socket_n " %V failed",
                                  &ls->addr_text);
                }

                return NGX_ERROR;
            }
        }

        ngx_log_debug2(NGX_LOG_DEBUG_CORE, log, 0,
                       "multiport, bind() %V #%d ", &ls->addr_text, s);

        if (bind(s, ls->sockaddr, ls->socklen) == -1) {
            ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                          "multiport, bind() to %V failed", &ls->addr_text);

            if (ngx_close_socket(s) == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "multiport, " ngx_close_socket_n " %V failed",
                              &ls->addr_text);
            }

            return NGX_ERROR;
        }

#if (NGX_HAVE_UNIX_DOMAIN)

        if (ls->sockaddr->sa_family == AF_UNIX) {
            mode_t   mode;
            u_char  *name;

            name = ls->addr_text.data + sizeof("unix:") - 1;
            mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);

            if (chmod((char *) name, mode) == -1) {
                ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_errno,
                              "multiport, chmod() \"%s\" failed", name);
            }
        }
#endif

        if (ls->type != SOCK_STREAM) {
            ls->fd = s;
            continue;
        }

        if (listen(s, ls->backlog) == -1) {
            err = ngx_socket_errno;

            /*
             * on OpenVZ after suspend/resume EADDRINUSE
             * may be returned by listen() instead of bind(), see
             * https://bugzilla.openvz.org/show_bug.cgi?id=2470
             */

            if (ngx_close_socket(s) == -1) {
                ngx_log_error(NGX_LOG_EMERG, log, ngx_socket_errno,
                              "multiport, " ngx_close_socket_n " %V failed",
                              &ls->addr_text);
            }

            if (err != NGX_EADDRINUSE) {
                ngx_log_error(NGX_LOG_EMERG, log, err,
                              "multiport, listen() to %V, backlog %d failed",
                              &ls->addr_text, ls->backlog);
                return NGX_ERROR;
            }

            failed = 1;
        }

        ls->listen = 1;
        ls->fd = s;

        if (!failed) {
            break;
        }

        /* TODO: delay configurable */

        ngx_log_error(NGX_LOG_NOTICE, log, 0,
                      "multiport, try again to bind() after 500ms");

        ngx_msleep(500);
    }

    if (failed) {
        ngx_log_error(NGX_LOG_EMERG, log, 0,
                      "multiport, still could not bind()");
        return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_event_multiport_configure_listening_socket(ngx_cycle_t *cycle,
        ngx_listening_t *ls)
{
    int                        value;

#if (NGX_HAVE_DEFERRED_ACCEPT && defined SO_ACCEPTFILTER)
    struct accept_filter_arg   af;
#endif

    ls->log = *ls->logp;

    if (ls->rcvbuf != -1) {
        if (setsockopt(ls->fd, SOL_SOCKET, SO_RCVBUF,
                       (const void *) &ls->rcvbuf, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_RCVBUF, %d) %V failed,"
                          " ignored", ls->rcvbuf, &ls->addr_text);
        }
    }

    if (ls->sndbuf != -1) {
        if (setsockopt(ls->fd, SOL_SOCKET, SO_SNDBUF,
                       (const void *) &ls->sndbuf, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_SNDBUF, %d) %V failed,"
                          " ignored", ls->sndbuf, &ls->addr_text);
        }
    }

    if (ls->keepalive) {
        value = (ls->keepalive == 1) ? 1 : 0;

        if (setsockopt(ls->fd, SOL_SOCKET, SO_KEEPALIVE,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_KEEPALIVE, %d) %V failed,"
                          " ignored", value, &ls->addr_text);
        }
    }

#if (NGX_HAVE_KEEPALIVE_TUNABLE)

    if (ls->keepidle) {
        value = ls->keepidle;

#if (NGX_KEEPALIVE_FACTOR)
        value *= NGX_KEEPALIVE_FACTOR;
#endif

        if (setsockopt(ls->fd, IPPROTO_TCP, TCP_KEEPIDLE,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(TCP_KEEPIDLE, %d) %V failed,"
                          " ignored", value, &ls->addr_text);
        }
    }

    if (ls->keepintvl) {
        value = ls->keepintvl;

#if (NGX_KEEPALIVE_FACTOR)
        value *= NGX_KEEPALIVE_FACTOR;
#endif

        if (setsockopt(ls->fd, IPPROTO_TCP, TCP_KEEPINTVL,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                         "multiport, setsockopt(TCP_KEEPINTVL, %d) %V failed,"
                         " ignored", value, &ls->addr_text);
        }
    }

    if (ls->keepcnt) {
        if (setsockopt(ls->fd, IPPROTO_TCP, TCP_KEEPCNT,
                       (const void *) &ls->keepcnt, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(TCP_KEEPCNT, %d) %V failed,"
                          " ignored", ls->keepcnt, &ls->addr_text);
        }
    }

#endif

#if (NGX_HAVE_SETFIB)
    if (ls->setfib != -1) {
        if (setsockopt(ls->fd, SOL_SOCKET, SO_SETFIB,
                       (const void *) &ls->setfib, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_SETFIB, %d) %V failed,"
                          " ignored", ls->setfib, &ls->addr_text);
        }
    }
#endif

#if (NGX_HAVE_TCP_FASTOPEN)
    if (ls->fastopen != -1) {
        if (setsockopt(ls->fd, IPPROTO_TCP, TCP_FASTOPEN,
                       (const void *) &ls->fastopen, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(TCP_FASTOPEN, %d) %V failed,"
                          " ignored", ls->fastopen, &ls->addr_text);
        }
    }
#endif

    if (ls->listen) {

        /* change backlog via listen() */

        if (listen(ls->fd, ls->backlog) == -1) {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, listen() to %V, backlog %d failed",
                          &ls->addr_text, ls->backlog);
            return NGX_ERROR;
        }
    }

    /*
     * setting deferred mode should be last operation on socket,
     * because code may prematurely continue cycle on failure
     */

#if (NGX_HAVE_DEFERRED_ACCEPT)

#ifdef SO_ACCEPTFILTER

    if (ls->delete_deferred) {
        if (setsockopt(ls->fd, SOL_SOCKET, SO_ACCEPTFILTER, NULL, 0)
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_ACCEPTFILTER, NULL) "
                          "for %V failed, ignored", &ls->addr_text);

            if (ls->accept_filter) {
                ngx_log_error(NGX_LOG_ALERT, cycle->log, 0,
                              "multiport, could not change the accept filter "
                              "to \"%s\" for %V, ignored",
                              ls->accept_filter, &ls->addr_text);
            }

            continue;
        }

        ls->deferred_accept = 0;
    }

    if (ls->add_deferred) {
        ngx_memzero(&af, sizeof(struct accept_filter_arg));
        (void) ngx_cpystrn((u_char *) af.af_name,
                           (u_char *) ls->accept_filter, 16);

        if (setsockopt(ls->fd, SOL_SOCKET, SO_ACCEPTFILTER,
                       &af, sizeof(struct accept_filter_arg))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(SO_ACCEPTFILTER, \"%s\") "
                          "for %V failed, ignored",
                          ls->accept_filter, &ls->addr_text);
            continue;
        }

        ls->deferred_accept = 1;
    }

#endif

#ifdef TCP_DEFER_ACCEPT

    if (ls->add_deferred || ls->delete_deferred) {

        if (ls->add_deferred) {
            /*
             * There is no way to find out how long a connection was
             * in queue (and a connection may bypass deferred queue at all
             * if syncookies were used), hence we use 1 second timeout
             * here.
             */
            value = 1;

        } else {
            value = 0;
        }

        if (setsockopt(ls->fd, IPPROTO_TCP, TCP_DEFER_ACCEPT,
                       &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(TCP_DEFER_ACCEPT, %d)"
                          " for %V failed, ignored", value, &ls->addr_text);

            return NGX_ERROR;
        }
    }

    if (ls->add_deferred) {
        ls->deferred_accept = 1;
    }

#endif

#endif /* NGX_HAVE_DEFERRED_ACCEPT */

#if (NGX_HAVE_IP_RECVDSTADDR)

    if (ls->wildcard
        && ls->type == SOCK_DGRAM
        && ls->sockaddr->sa_family == AF_INET)
    {
        value = 1;

        if (setsockopt(ls->fd, IPPROTO_IP, IP_RECVDSTADDR,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(IP_RECVDSTADDR) "
                          "for %V failed, ignored", &ls->addr_text);
        }
    }

#elif (NGX_HAVE_IP_PKTINFO)

    if (ls->wildcard
        && ls->type == SOCK_DGRAM
        && ls->sockaddr->sa_family == AF_INET)
    {
        value = 1;

        if (setsockopt(ls->fd, IPPROTO_IP, IP_PKTINFO,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(IP_PKTINFO) "
                          "for %V failed, ignored", &ls->addr_text);
        }
    }

#endif

#if (NGX_HAVE_INET6 && NGX_HAVE_IPV6_RECVPKTINFO)

    if (ls->wildcard
        && ls->type == SOCK_DGRAM
        && ls->sockaddr->sa_family == AF_INET6)
    {
        value = 1;

        if (setsockopt(ls->fd, IPPROTO_IPV6, IPV6_RECVPKTINFO,
                       (const void *) &value, sizeof(int))
            == -1)
        {
            ngx_log_error(NGX_LOG_ALERT, cycle->log, ngx_socket_errno,
                          "multiport, setsockopt(IPV6_RECVPKTINFO) "
                          "for %V failed, ignored", &ls->addr_text);
        }
    }

#endif

    return NGX_OK;
}

static void
ngx_event_multiport_close_listening_sock(ngx_cycle_t *cycle,
        ngx_listening_t *ls)
{
    ngx_connection_t               *c;

    c = ls->connection;

    if (c) {
        if (c->read->active) {
            if (ngx_event_flags & NGX_USE_EPOLL_EVENT) {

                /*
                 * it seems that Linux-2.6.x OpenVZ sends events
                 * for closed shared listening sockets unless
                 * the events was explicitly deleted
                 */

                ngx_del_event(c->read, NGX_READ_EVENT, 0);

            } else {
                ngx_del_event(c->read, NGX_READ_EVENT, NGX_CLOSE_EVENT);
            }
        }

        ngx_free_connection(c);

        c->fd = (ngx_socket_t) -1;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, cycle->log, 0,
                   "multiport, close listening %V #%d", &ls->addr_text, ls->fd);

    if (ngx_close_socket(ls->fd) == -1) {
        ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                      "multiport, ", ngx_close_socket_n " %V failed",
                      &ls->addr_text);
    }

#if (NGX_HAVE_UNIX_DOMAIN)

    if (ls->sockaddr->sa_family == AF_UNIX
        && ngx_process == NGX_PROCESS_WORKER)
    {
        u_char *name = ls->addr_text.data + sizeof("unix:") - 1;

        if (ngx_delete_file(name) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                          "multiport, " ngx_delete_file_n " %s failed", name);
        }
    }

#endif

    ls->fd = (ngx_socket_t) -1;
}

static ngx_listening_t *
ngx_event_multiport_find_relation_port(ngx_cycle_t *cycle, ngx_str_t *str)
{
    ngx_url_t                       u;
    ngx_listening_t                *ls;
    ngx_uint_t                      i;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = *str;
    u.listen = 1;
    u.default_port = 0;

    if (ngx_parse_url(cycle->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, 0,
                          "multiport, relation port err: %V", str);
        }

        return NULL;
    }

    ls = cycle->listening.elts;

    for (i = 0; i < cycle->listening.nelts; ++i) {

        if (ls[i].socklen == u.socklen
                && ngx_memcmp(ls[i].sockaddr, &u.sockaddr, u.socklen) == 0)
        {
            return &ls[i];
        }
    }

    ngx_log_error(NGX_LOG_EMERG, cycle->log, 0,
                  "multiport, can not find relation port: %V", str);
    return NULL;
}

static ngx_int_t
ngx_event_multiport_set_port(ngx_cycle_t *cycle,
        ngx_event_multiport_listen_t *mls, ngx_listening_t *ls)
{
    ngx_str_t                   port;
    ngx_int_t                   rc;
    ngx_url_t                   u;

    ngx_memzero(&port, sizeof(ngx_str_t));

    rc = ngx_multiport_get_port(cycle->pool, &port, &mls->multiport,
                                ngx_process_slot);
    if (rc == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                "multiport, get multiport error");
        return NGX_ERROR;
    }

    ngx_memzero(&u, sizeof(ngx_url_t));
    u.url = port;
    u.listen = 1;
    u.default_port = 0;

    if (ngx_parse_url(cycle->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                    "multiport, %s in \"%V\" when set port", u.err, &u.url);
        }

        return NGX_ERROR;
    }

    /* init multiport listening */
    if (ngx_event_multiport_init_listening(cycle, &mls->listening,
            (struct sockaddr *) &u.sockaddr, u.socklen, ls) != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                      "multiport, init listening error: %V", &mls->multiport);
        return NGX_ERROR;
    }

    /* open multiport listening */
    if (ngx_event_multiport_open_listening_sock(cycle, &mls->listening)
            != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, cycle->log, 0,
                      "multiport, open listening error: %V", &mls->multiport);
        return NGX_ERROR;
    }

    /* config multiport listening */
    if (ngx_event_multiport_configure_listening_socket(cycle,
            &mls->listening) != NGX_OK)
    {
        return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_event_multiport_enable_accept_event(ngx_cycle_t *cycle, ngx_listening_t *ls)
{
    ngx_connection_t   *c;
    ngx_event_t        *rev;

    c = ngx_get_connection(ls->fd, ngx_cycle->log);

    if (c == NULL) {
        return NGX_ERROR;
    }

    c->type = ls->type;
    c->log = &ls->log;

    c->listening = ls;
    ls->connection = c;

    rev = c->read;

    rev->log = c->log;
    rev->accept = 1;

#if (NGX_HAVE_DEFERRED_ACCEPT)
    rev->deferred_accept = ls->deferred_accept;
#endif

#if (NGX_WIN32)

    if (ngx_event_flags & NGX_USE_IOCP_EVENT) {
        ngx_iocp_conf_t  *iocpcf;

        rev->handler = ngx_event_acceptex;

        if (ngx_add_event(rev, 0, NGX_IOCP_ACCEPT) == NGX_ERROR) {
            return NGX_ERROR;
        }

        ls->log.handler = ngx_acceptex_log_error;

        iocpcf = ngx_event_get_conf(cycle->conf_ctx, ngx_iocp_module);
        if (ngx_event_post_acceptex(ls, iocpcf->post_acceptex)
                == NGX_ERROR)
        {
            return NGX_ERROR;
        }

    } else {
        rev->handler = ngx_event_accept;

        if (ngx_use_accept_mutex) {
            return NGX_OK;
        }

        if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
            return NGX_ERROR;
        }
    }

#else

    rev->handler = (c->type == SOCK_STREAM)? ngx_event_accept
        : ngx_event_recvmsg;

    if (ngx_add_event(rev, NGX_READ_EVENT, 0) == NGX_ERROR) {
        return NGX_ERROR;
    }

#endif

    return NGX_OK;
}

static ngx_int_t
ngx_event_multiport_process_init(ngx_cycle_t *cycle)
{
    ngx_event_multiport_conf_t     *emcf;
    ngx_event_multiport_listen_t   *mls;
    ngx_listening_t                *rls;
    ngx_uint_t                      i;

    if (ngx_process != NGX_PROCESS_WORKER) {
        return NGX_OK;
    }

    emcf = ngx_event_get_conf(cycle->conf_ctx, ngx_event_multiport_module);
    if (emcf->ports == NULL) {
        return NGX_OK;
    }

    mls = emcf->ports->elts;
    for (i = 0; i < emcf->ports->nelts; ++i) {
        /* find relation listening in cycle->listening */
        rls = ngx_event_multiport_find_relation_port(cycle,
                                                     &mls[i].relatedport);
        if (rls == NULL) {
            return NGX_ERROR;
        }

        /* set process port with relation port */
        if (ngx_event_multiport_set_port(cycle, &mls[i], rls) != NGX_OK) {
            return NGX_ERROR;
        }

        /* enable accept */
        if (ngx_event_multiport_enable_accept_event(cycle, &mls[i].listening)
                != NGX_OK)
        {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, ngx_socket_errno,
                          "multiport, enable accept failed");
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}

static void
ngx_event_multiport_process_exit(ngx_cycle_t *cycle)
{
    ngx_event_multiport_conf_t     *emcf;
    ngx_event_multiport_listen_t   *mls;
    ngx_uint_t                      i;

    if (ngx_process != NGX_PROCESS_WORKER) {
        return;
    }

    emcf = ngx_event_get_conf(cycle->conf_ctx, ngx_event_multiport_module);
    if (emcf->ports == NULL) {
        return;
    }

    mls = emcf->ports->elts;

    for (i = 0; i < emcf->ports->nelts; ++i) {
        ngx_event_multiport_close_listening_sock(cycle, &mls[i].listening);
    }
}
