
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
#include "ngx_rtmp_proxy_protocol.h"


static void ngx_rtmp_proxy_protocol_recv(ngx_event_t *rev);


void
ngx_rtmp_proxy_protocol(ngx_rtmp_session_t *s)
{
    ngx_event_t       *rev;
    ngx_connection_t  *c;

    c = s->connection;
    rev = c->read;
    rev->handler =  ngx_rtmp_proxy_protocol_recv;

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "proxy_protocol: start");

    if (rev->ready) {
        /* the deferred accept(), rtsig, aio, iocp */

        if (ngx_use_accept_mutex) {
            ngx_post_event(rev, &ngx_posted_events);
            return;
        }

        rev->handler(rev);
        return;
    }

    ngx_add_timer(rev, s->timeout);

    if (ngx_handle_read_event(rev, 0) != NGX_OK) {
        ngx_rtmp_finalize_session(s);
        return;
    }
}


static void
ngx_rtmp_proxy_protocol_recv(ngx_event_t *rev)
{
    u_char               buf[107], *p, *pp, *text;
    size_t               len;
    ssize_t              n;
    ngx_err_t            err;
    ngx_int_t            i;
    ngx_addr_t           addr;
    ngx_connection_t    *c;
    ngx_rtmp_session_t  *s;

    c = rev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    if (rev->timedout) {
        ngx_log_error(NGX_LOG_INFO, c->log, NGX_ETIMEDOUT,
                "proxy_protocol: recv: client timed out");
        c->timedout = 1;
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (rev->timer_set) {
        ngx_del_timer(rev);
    }

    n = recv(c->fd, (char *) buf, sizeof(buf), MSG_PEEK);

    err = ngx_socket_errno;

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, c->log, 0, "recv(): %d", n);

    if (n == -1) {

        if (err == NGX_EAGAIN) {
            ngx_add_timer(rev, s->timeout);

            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
                ngx_rtmp_finalize_session(s);
            }

            return;
        }

        ngx_rtmp_finalize_session(s);

        return;
    }

    p = buf;

    if (n <= 8 && ngx_strncmp(p, "PROXY ", 6) != 0) {
        goto bad_header;
    }

    n -= 6;
    p += 6;

    ngx_memzero(&addr, sizeof(ngx_addr_t));

    if (n >= 7 && ngx_strncmp(p, "UNKNOWN", 7) == 0) {
        n -= 7;
        p += 7;
        goto skip;
    }

    if (n < 5 || ngx_strncmp(p, "TCP", 3) != 0
        || (p[3] != '4' && p[3] != '6') || p[4] != ' ')
    {
        goto bad_header;
    }

    n -= 5;
    p += 5;

    pp = ngx_strlchr(p, p + n, ' ');

    if (pp == NULL) {
        goto bad_header;
    }

    if (ngx_parse_addr(s->pool, &addr, p, pp - p) != NGX_OK) {
        goto bad_header;
    }

    n -= pp - p;
    p = pp;

skip:

    for (i = 0; i + 1 < n; i++) {
        if (p[i] == CR && p[i + 1] == LF) {
            break;
        }
    }

    if (i + 1 >= n) {
        goto bad_header;
    }

    n = p - buf + i + 2;

    if (c->recv(c, buf, n) != n) {
        goto failed;
    }

    if (addr.socklen) {
        text = ngx_palloc(s->pool, NGX_SOCKADDR_STRLEN);

        if (text == NULL) {
            goto failed;
        }

        len = ngx_sock_ntop(addr.sockaddr,
#if (nginx_version >= 1005003)
                            addr.socklen,
#endif
                            text, NGX_SOCKADDR_STRLEN, 0);
        if (len == 0) {
            goto failed;
        }

        c->sockaddr = addr.sockaddr;
        c->socklen = addr.socklen;
        c->addr_text.data = text;
        c->addr_text.len = len;

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, c->log, 0,
                       "proxy_protocol: remote_addr:'%V'", &c->addr_text);
    }

    ngx_rtmp_handshake(s);

    return;

bad_header:

    ngx_log_error(NGX_LOG_INFO, c->log, 0, "proxy_protocol: bad header");

failed:

    ngx_rtmp_finalize_session(s);
}
