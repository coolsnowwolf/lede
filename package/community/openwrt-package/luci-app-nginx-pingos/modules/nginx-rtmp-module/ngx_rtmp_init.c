
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_live_relay.h"
#include "ngx_rtmp_proxy_protocol.h"
#include "ngx_http_client.h"
#include "ngx_rbuf.h"
#include "ngx_poold.h"


static void ngx_rtmp_close_connection(ngx_connection_t *c);
static u_char * ngx_rtmp_log_error(ngx_log_t *log, u_char *buf, size_t len);


typedef struct {
    ngx_str_t              *client;
    ngx_rtmp_session_t     *session;
    void                   *data;       // combined log
    ngx_log_handler_pt      handler;    // combined log handler
} ngx_rtmp_error_log_ctx_t;


char *ngx_live_stage[] = {
    "init",
    "handshake_done",
    "connect",
    "create_stream",
    "publish",
    "play",
    "audio_video",
    "close_stream",
};


char *ngx_live_err[] = {
    "internal_err",
    "normal_close",
    "rtmp_send_err",
    "rtmp_send_timeout",
    "flv_send_err",
    "flv_send_timeout",
    "rtmp_recv_err",
    "flv_recv_err",
    "relay_transit",
    "relay_timeout",
    "control_drop",
    "drop_idle",
    "notify_notify_err",
    "notify_relay_err",
    "notify_para_err",
    "relay_close",
    "process_exit",
};


void
ngx_rtmp_init_connection(ngx_connection_t *c)
{
    ngx_uint_t             i;
    ngx_rtmp_port_t       *port;
    struct sockaddr       *sa;
    struct sockaddr_in    *sin;
    ngx_rtmp_in_addr_t    *addr;
    ngx_rtmp_session_t    *s;
    ngx_rtmp_addr_conf_t  *addr_conf;
    ngx_int_t              unix_socket;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6   *sin6;
    ngx_rtmp_in6_addr_t   *addr6;
#endif

    ++ngx_rtmp_naccepted;

    /* find the server configuration for the address:port */

    /* AF_INET only */

    port = c->listening->servers;
    unix_socket = 0;

    if (port->naddrs > 1) {

        /*
         * There are several addresses on this port and one of them
         * is the "*:port" wildcard so getsockname() is needed to determine
         * the server address.
         *
         * AcceptEx() already gave this address.
         */

        if (ngx_connection_local_sockaddr(c, NULL, 0) != NGX_OK) {
            ngx_rtmp_close_connection(c);
            return;
        }

        sa = c->local_sockaddr;

        switch (sa->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) sa;

            addr6 = port->addrs;

            /* the last address is "*" */

            for (i = 0; i < port->naddrs - 1; i++) {
                if (ngx_memcmp(&addr6[i].addr6, &sin6->sin6_addr, 16) == 0) {
                    break;
                }
            }

            addr_conf = &addr6[i].conf;

            break;
#endif

        case AF_UNIX:
            unix_socket = 1;

            sin = (struct sockaddr_in *) sa;

            addr = port->addrs;

            /* the last address is "*" */

            for (i = 0; i < port->naddrs - 1; i++) {
                if (addr[i].addr == sin->sin_addr.s_addr) {
                    break;
                }
            }

            addr_conf = &addr[i].conf;

            break;

        default: /* AF_INET */
            sin = (struct sockaddr_in *) sa;

            addr = port->addrs;

            /* the last address is "*" */

            for (i = 0; i < port->naddrs - 1; i++) {
                if (addr[i].addr == sin->sin_addr.s_addr) {
                    break;
                }
            }

            addr_conf = &addr[i].conf;

            break;
        }

    } else {
        switch (c->local_sockaddr->sa_family) {

#if (NGX_HAVE_INET6)
        case AF_INET6:
            addr6 = port->addrs;
            addr_conf = &addr6[0].conf;
            break;
#endif

        case AF_UNIX:
            unix_socket = 1;
            addr = port->addrs;
            addr_conf = &addr[0].conf;
            break;

        default: /* AF_INET */
            addr = port->addrs;
            addr_conf = &addr[0].conf;
            break;
        }
    }

    ngx_log_error(NGX_LOG_INFO, c->log, 0, "*%ui client connected '%V'",
                  c->number, &c->addr_text);

    s = ngx_rtmp_create_session(addr_conf);
    if (s == NULL) {
        ngx_log_error(NGX_LOG_INFO, c->log, 0, "create rtmp session failed");
        return;
    }
    s->log->connection = c->number;
    s->number = c->number;
    s->remote_addr_text.data = ngx_pcalloc(s->pool, c->addr_text.len);
    s->remote_addr_text.len = c->addr_text.len;
    ngx_memcpy(s->remote_addr_text.data, c->addr_text.data, c->addr_text.len);
    s->sockaddr = ngx_pcalloc(s->pool, sizeof(struct sockaddr));
    ngx_memcpy(s->sockaddr, c->sockaddr, sizeof(struct sockaddr));

    ngx_rtmp_init_session(s, c);

    /* only auto-pushed connections are
     * done through unix socket */

    s->interprocess = unix_socket;

    if (addr_conf->proxy_protocol) {
        ngx_rtmp_proxy_protocol(s);

    } else {
        ngx_rtmp_handshake(s);
    }
}


static u_char *
ngx_rtmp_log_error(ngx_log_t *log, u_char *buf, size_t len)
{
    u_char                     *p;
    ngx_rtmp_session_t         *s;
    ngx_rtmp_error_log_ctx_t   *ctx;

    p = buf;

    ctx = log->data;

    if (ctx->client) {
        p = ngx_snprintf(buf, len, ", client: %V", ctx->client);
        len -= p - buf;
        buf = p;
    }

    s = ctx->session;

    if (s == NULL) {
        return p;
    }

    p = ngx_snprintf(buf, len, ", server: %V, session: %p", s->addr_text, s);
    len -= p - buf;
    buf = p;

    if (s->stream.len) {
        p = ngx_snprintf(buf, len, ", stream: %V", &s->stream);
        len -= p - buf;
        buf = p;
    }

    if (ctx->data) { // combined log
        log->data = ctx->data;
        p = ctx->handler(log, p, len);
        log->data = ctx;
    }

    return p;
}


static void
ngx_rtmp_close_connection(ngx_connection_t *c)
{
    ngx_pool_t                         *pool;

    if (c == NULL) {
        return;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, c->log, 0, "close connection");

#if (NGX_STAT_STUB)
    (void) ngx_atomic_fetch_add(ngx_stat_active, -1);
#endif

    pool = c->pool;
    ngx_close_connection(c);
    ngx_destroy_pool(pool);
}


static void
ngx_rtmp_close_session(ngx_rtmp_session_t *s)
{
    if (s->ping_evt.timer_set) {
        ngx_del_timer(&s->ping_evt);
    }

    if (s->in_old_pool) {
        NGX_DESTROY_POOL(s->in_old_pool);
        s->in_old_pool = NULL;
    }

    if (s->in_pool) {
        NGX_DESTROY_POOL(s->in_pool);
        s->in_pool = NULL;
    }

    if (s->live_type == NGX_HTTP_FLV_LIVE) {
        ngx_put_chainbufs(s->in_streams[0].in);
    }

    ngx_rtmp_free_handshake_buffers(s);

    ngx_rtmp_free_merge_frame(s);

    if (s->live_type == NGX_MPEGTS_LIVE) {
        while (s->out_pos != s->out_last) {
            ngx_rtmp_shared_free_mpegts_frame(s->mpegts_out[s->out_pos++]);
            s->out_pos %= s->out_queue;
        }
    } else {
        while (s->out_pos != s->out_last) {
            ngx_rtmp_shared_free_frame(s->out[s->out_pos++]);
            s->out_pos %= s->out_queue;
        }
    }

    if (s->pool) {
        NGX_DESTROY_POOL(s->pool);
    }
}


static void
ngx_rtmp_close_session_handler(ngx_event_t *e)
{
    ngx_rtmp_session_t                 *s;
    ngx_connection_t                   *c;

    s = e->data;
    c = s->connection;
    if (c) {
        c->destroyed = 1;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "async close session");

    ngx_rtmp_fire_event(s, NGX_RTMP_DISCONNECT, NULL, NULL);

    // close connection before session, connection log use session log
    // may cause memory error
    ngx_rtmp_close_connection(c);

    ngx_rtmp_close_session(s);
}


static void
ngx_rtmp_async_finalize_http_client(ngx_event_t *ev)
{
    ngx_rtmp_session_t         *s;
    ngx_http_request_t         *hcr;

    s = ev->data;
    hcr = s->request;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "asyn finalize http client");

    if (hcr) {
        ngx_http_client_finalize_request(hcr, 1);
    }
}


static void
ngx_rtmp_async_finalize_http_request(ngx_event_t *ev)
{
    ngx_rtmp_session_t         *s;
    ngx_http_request_t         *r;

    s = ev->data;
    r = s->request;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "asyn finalize http request");

    if (!r) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "Http request is null");
        return;
    }

    if (r->header_sent) {
        ngx_http_finalize_request(r, NGX_HTTP_CLIENT_CLOSED_REQUEST);
        ngx_http_run_posted_requests(r->connection);
    } else {
        r->error_page = 1;

        if (s->status) {
            ngx_http_finalize_request(r, s->status);
        } else {
            ngx_http_finalize_request(r, NGX_HTTP_SERVICE_UNAVAILABLE);
        }
    }
}


void
ngx_rtmp_finalize_session(ngx_rtmp_session_t *s)
{
    ngx_event_t        *e;
    ngx_connection_t   *c;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "finalize session");

    if (s->live_type == NGX_HLS_LIVE) {
        ngx_rtmp_finalize_fake_session(s);
        return;
    }

    c = s->connection;
    if (c && c->destroyed) {
        return;
    }

    if (s->finalized) {
        ngx_log_error(NGX_LOG_WARN, s->log, 0, "The session has been finalized.");
        return;
    }

    s->finalized = 1;

    if (s->live_type != NGX_RTMP_LIVE) {
        e = &s->close;
        e->data = s;
        if (s->relay) {
            e->handler = ngx_rtmp_async_finalize_http_client;
        } else {
            e->handler = ngx_rtmp_async_finalize_http_request;
        }
        e->log = s->log;

        ngx_post_event(e, &ngx_posted_events);

        return;
    }

    e = &s->close;
    e->data = s;
    e->handler = ngx_rtmp_close_session_handler;
    e->log = s->log;

    ngx_post_event(e, &ngx_posted_events);
}


// session combined with http request or http client request
void
ngx_rtmp_finalize_fake_session(ngx_rtmp_session_t *s)
{
    ngx_log_error(NGX_LOG_INFO, s->log, 0, "finalize fake session");


    if (s->finalized) {
        ngx_log_error(NGX_LOG_WARN, s->log, 0, "The fake session has been finalized.");
        return;
    }

    s->finalized = 1;

    ngx_rtmp_fire_event(s, NGX_RTMP_DISCONNECT, NULL, NULL);

    ngx_rtmp_close_session(s);
}


void
ngx_rtmp_set_combined_log(ngx_rtmp_session_t *s, void *d, ngx_log_handler_pt h)
{
    ngx_rtmp_error_log_ctx_t       *ctx;

    ctx = s->log->data;
    ctx->data = d;
    ctx->handler = h;
}


// Only rtmp need to init session
void
ngx_rtmp_init_session(ngx_rtmp_session_t *s, ngx_connection_t *c)
{
    ngx_rtmp_error_log_ctx_t       *ctx;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

    c->data = s;
    s->connection = c;

    c->log = s->log;
    c->write->log = s->log;
    c->read->log = s->log;

    ctx = s->log->data;
    ctx->client = &c->addr_text;

    s->log->connection = c->number;

    c->log_error = NGX_ERROR_INFO;

    ngx_rtmp_set_chunk_size(s, NGX_RTMP_DEFAULT_CHUNK_SIZE);

    if (s->variables == NULL) {
        s->variables = ngx_pcalloc(s->pool, cmcf->variables.nelts
                * sizeof(ngx_http_variable_value_t));
    }

    if (s->variables == NULL) {
        ngx_rtmp_finalize_session(s);
        return;
    }

    return;
}


ngx_rtmp_session_t *
ngx_rtmp_create_relay_session(ngx_rtmp_session_t *s, void *tag)
{
    ngx_rtmp_session_t         *rs;
    ngx_live_relay_ctx_t       *rctx;
    ngx_rtmp_core_main_conf_t  *cmcf;

    rs = ngx_rtmp_create_session(s->addr_conf);
    if (rs == NULL) {
        return NULL;
    }

    rs->relay = 1;
    rs->main_conf = s->main_conf;
    rs->srv_conf = s->srv_conf;
    rs->app_conf = s->app_conf;

    // clone para from origin session
#define NGX_RTMP_RELAY_SESSION_COPY_PARA(para)                      \
    if (ngx_copy_str(rs->pool, &rs->para, &s->para) != NGX_OK) {    \
        goto destroy;                                               \
    }

    NGX_RTMP_RELAY_SESSION_COPY_PARA(stream);

    NGX_RTMP_RELAY_SESSION_COPY_PARA(name);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(pargs);

    NGX_RTMP_RELAY_SESSION_COPY_PARA(app);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(args);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(flashver);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(swf_url);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(tc_url);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(page_url);

    NGX_RTMP_RELAY_SESSION_COPY_PARA(serverid);
#undef NGX_RTMP_RELAY_SESSION_COPY_PARA

    rs->acodecs = s->acodecs;
    rs->vcodecs = s->vcodecs;

    ngx_rtmp_cmd_middleware_init(rs);

    // create relay ctx
    rctx = ngx_pcalloc(rs->pool, sizeof(ngx_live_relay_ctx_t));
    if (rctx == NULL) {
        goto destroy;
    }
    rctx->domain = rs->domain;
    rctx->app = rs->app;
    rctx->args = rs->args;
    rctx->name = rs->name;
    rctx->pargs = rs->pargs;

    rctx->referer = rs->page_url;
    rctx->user_agent = rs->flashver;
    rctx->swf_url = rs->swf_url;
    rctx->acodecs = rs->acodecs;
    rctx->vcodecs = rs->vcodecs;

    rctx->tag = tag;

    ngx_rtmp_set_ctx(rs, rctx, ngx_live_relay_module);

    cmcf = ngx_rtmp_get_module_main_conf(rs, ngx_rtmp_core_module);
    if (rs->variables == NULL) {
        rs->variables = ngx_pcalloc(rs->pool, cmcf->variables.nelts
                * sizeof(ngx_http_variable_value_t));
    }

    return rs;

destroy:
    ngx_rtmp_close_session(rs);

    return NULL;
}


ngx_rtmp_session_t *
ngx_rtmp_create_static_session(ngx_live_relay_t *relay,
        ngx_rtmp_addr_conf_t *addr_conf, void *tag)
{
    ngx_rtmp_session_t         *rs;
    ngx_live_relay_ctx_t       *rctx;
    ngx_uint_t                  n;
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_rtmp_core_app_conf_t  **cacfp;

    rs = ngx_rtmp_create_session(addr_conf);
    if (rs == NULL) {
        return NULL;
    }

    rs->static_pull = 1;
    rs->relay = 1;

    // clone para from origin session
#define NGX_RTMP_RELAY_SESSION_COPY_PARA(to, from)                  \
    if (ngx_copy_str(rs->pool, &rs->to, &relay->from) != NGX_OK) {  \
        goto destroy;                                               \
    }

    NGX_RTMP_RELAY_SESSION_COPY_PARA(app, app);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(name, name);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(pargs, pargs);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(page_url, referer);
    NGX_RTMP_RELAY_SESSION_COPY_PARA(flashver, user_agent);

    NGX_RTMP_RELAY_SESSION_COPY_PARA(stream, stream);
#undef NGX_RTMP_RELAY_SESSION_COPY_PARA

    rs->tc_url.len = sizeof("rtmp://") - 1 + relay->domain.len
                   + sizeof("/") - 1 + relay->app.len;
    rs->tc_url.data = ngx_pcalloc(rs->pool, rs->tc_url.len);
    if (rs->tc_url.data == NULL) {
        goto destroy;
    }
    ngx_snprintf(rs->tc_url.data, rs->tc_url.len, "rtmp://%V/%V",
            &relay->domain, &relay->app);

    ngx_rtmp_cmd_middleware_init(rs);

    if (ngx_rtmp_set_virtual_server(rs, &rs->domain)) {
        goto destroy;
    }
    cscf = ngx_rtmp_get_module_srv_conf(rs, ngx_rtmp_core_module);

    rs->live_server = ngx_live_create_server(&rs->serverid);

    cacfp = cscf->applications.elts;
    for (n = 0; n < cscf->applications.nelts; ++n, ++cacfp) {
        if ((*cacfp)->name.len == rs->app.len &&
            ngx_strncmp((*cacfp)->name.data, rs->app.data, rs->app.len) == 0)
        {
            /* found app! */
            rs->app_conf = (*cacfp)->app_conf;
            break;
        }
    }

    if (rs->app_conf == NULL) {

        if (cscf->default_app == NULL || cscf->default_app->app_conf == NULL) {
            ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                    "static session: application not found: '%V'", &rs->app);
            goto destroy;
        }

        rs->app_conf = cscf->default_app->app_conf;
    }

    // create relay ctx
    rctx = ngx_pcalloc(rs->pool, sizeof(ngx_live_relay_ctx_t));
    if (rctx == NULL) {
        goto destroy;
    }
    rctx->domain = rs->domain;
    rctx->app = rs->app;
    rctx->args = rs->args;
    rctx->name = rs->name;
    rctx->pargs = rs->pargs;

    rctx->referer = rs->page_url;
    rctx->user_agent = rs->flashver;
    rctx->swf_url = rs->swf_url;
    rctx->acodecs = rs->acodecs;
    rctx->vcodecs = rs->vcodecs;

    rctx->tag = tag;

    ngx_rtmp_set_ctx(rs, rctx, ngx_live_relay_module);

    return rs;

destroy:
    ngx_rtmp_close_session(rs);

    return NULL;
}


// if return NULL, memory must be error
ngx_rtmp_session_t *
ngx_rtmp_create_session(ngx_rtmp_addr_conf_t *addr_conf)
{
    ngx_rtmp_session_t             *s;
    ngx_rtmp_core_srv_conf_t       *cscf;
    ngx_pool_t                     *pool;
    ngx_rtmp_error_log_ctx_t       *ctx;

    pool = NGX_CREATE_POOL(4096, ngx_cycle->log);
    if (pool == NULL) {
        return NULL;
    }

    s = ngx_pcalloc(pool, sizeof(ngx_rtmp_session_t) +
            sizeof(ngx_rtmp_frame_t *) * ((ngx_rtmp_core_srv_conf_t *)
                addr_conf->default_server->ctx-> srv_conf[ngx_rtmp_core_module
                    .ctx_index])->out_queue);
    if (s == NULL) {
        goto destroy;
    }
    s->pool = pool;

    s->addr_conf = addr_conf;

    s->main_conf = addr_conf->default_server->ctx->main_conf;
    s->srv_conf = addr_conf->default_server->ctx->srv_conf;

    s->addr_text = ngx_pcalloc(s->pool, sizeof(ngx_str_t));
    s->addr_text->data = ngx_pcalloc(s->pool, addr_conf->addr_text.len);
    s->addr_text->len = addr_conf->addr_text.len;
    ngx_memcpy(s->addr_text->data,
        addr_conf->addr_text.data, addr_conf->addr_text.len);

    s->ctx = ngx_pcalloc(pool, sizeof(void *) * ngx_rtmp_max_module);
    if (s->ctx == NULL) {
        goto destroy;
    }

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    s->out_queue = cscf->out_queue;
    s->out_cork = cscf->out_cork;
    s->in_streams = ngx_pcalloc(pool, sizeof(ngx_rtmp_stream_t)
            * cscf->max_streams);
    if (s->in_streams == NULL) {
        goto destroy;
    }

#if (nginx_version >= 1007005)
    ngx_queue_init(&s->posted_dry_events);
#endif

    s->epoch = ngx_current_msec;
    s->timeout = cscf->timeout;
    s->buflen = cscf->buflen;

    // init log
    ctx = ngx_pcalloc(pool, sizeof(ngx_rtmp_error_log_ctx_t));
    if (ctx == NULL) {
        goto destroy;
    }

    ctx->session = s;

    s->log = ngx_pcalloc(pool, sizeof(ngx_log_t));
    if (s->pool == NULL) {
        goto destroy;
    }
    *s->log = ngx_cycle->new_log;

    s->log->handler = ngx_rtmp_log_error;
    s->log->data = ctx;

    s->stage = NGX_LIVE_INIT;
    s->init_time = ngx_current_msec;

    s->mpegts_out = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_frame_t *) *
                ((ngx_rtmp_core_srv_conf_t *) addr_conf->default_server->ctx->
                srv_conf[ngx_rtmp_core_module.ctx_index])->out_queue);

    return s;

destroy:
    if (pool) {
        NGX_DESTROY_POOL(pool);
    }

    return NULL;
}
