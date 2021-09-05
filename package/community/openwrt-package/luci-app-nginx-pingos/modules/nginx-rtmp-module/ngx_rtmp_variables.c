/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <nginx.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_variables.h"
#include "ngx_live_relay.h"


static ngx_int_t ngx_rtmp_variable_session_get_str(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_session_get_uint(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_session_get_int(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_session_get_msec(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_rtmp_variable_relay_get_str(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_relay_get_uint(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);

static ngx_int_t ngx_rtmp_variable_finalize_reason(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_stage(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_remote_addr(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_remote_port(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_local_addr(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_local_port(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_nginx_version(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_pid(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_msec(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_time_iso8601(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_time_local(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_ngx_worker(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_rtmp_variable_argument(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data);


static ngx_rtmp_variable_t  ngx_rtmp_core_variables[] = {

    { ngx_string("domain"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, domain), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("app"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, app), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("name"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, name), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("stream"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, stream), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("pargs"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, pargs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("args"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, args), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("flashver"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, flashver), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("swf_url"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, swf_url), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("tc_url"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, tc_url), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("page_url"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, page_url), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("acodecs"), NULL, ngx_rtmp_variable_session_get_uint,
      offsetof(ngx_rtmp_session_t, acodecs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("vcodecs"), NULL, ngx_rtmp_variable_session_get_uint,
      offsetof(ngx_rtmp_session_t, vcodecs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("scheme"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, scheme), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("serverid"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, serverid), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("groupid"), NULL, ngx_rtmp_variable_session_get_str,
      offsetof(ngx_rtmp_session_t, groupid), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("notify_status"), NULL, ngx_rtmp_variable_session_get_int,
      offsetof(ngx_rtmp_session_t, notify_status), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("finalize_reason"), NULL, ngx_rtmp_variable_finalize_reason,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("stage"), NULL, ngx_rtmp_variable_stage,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("init"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, init_time), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("handshake_done"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, handshake_done_time),
      NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("connect"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, connect_time), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("create_stream"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, create_stream_time),
      NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("ptime"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, ptime), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("first_data"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, first_data), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("first_metadata"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, first_metadata),
      NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("first_audio"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, first_audio), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("first_video"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, first_video), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("close_stream"), NULL, ngx_rtmp_variable_session_get_msec,
      offsetof(ngx_rtmp_session_t, close_stream_time),
      NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_domain"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, domain), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_app"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, app), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_name"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, name), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_args"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, args), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_pargs"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, pargs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_referer"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, referer), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_user_agent"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, user_agent), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_swf_url"), NULL, ngx_rtmp_variable_relay_get_str,
      offsetof(ngx_live_relay_ctx_t, swf_url), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_acodecs"), NULL, ngx_rtmp_variable_relay_get_uint,
      offsetof(ngx_live_relay_ctx_t, acodecs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("relay_vcodecs"), NULL, ngx_rtmp_variable_relay_get_uint,
      offsetof(ngx_live_relay_ctx_t, vcodecs), NGX_RTMP_VAR_NOCACHEABLE, 0},

    { ngx_string("remote_addr"), NULL, ngx_rtmp_variable_remote_addr,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("remote_port"), NULL, ngx_rtmp_variable_remote_port,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("server_addr"), NULL, ngx_rtmp_variable_local_addr,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("server_port"), NULL, ngx_rtmp_variable_local_port,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("nginx_version"), NULL,
      ngx_rtmp_variable_nginx_version, 0, 0, 0},

    { ngx_string("pid"), NULL, ngx_rtmp_variable_pid, 0, 0, 0},

    { ngx_string("msec"), NULL, ngx_rtmp_variable_msec,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("time_iso8601"), NULL, ngx_rtmp_variable_time_iso8601,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("time_local"), NULL, ngx_rtmp_variable_time_local,
      0, NGX_RTMP_VAR_NOCACHEABLE, 0 },

    { ngx_string("ngx_worker"), NULL, ngx_rtmp_variable_ngx_worker,
      0, 0, 0},

    { ngx_string("parg_"), NULL, ngx_rtmp_variable_argument,
      0, NGX_RTMP_VAR_NOCACHEABLE|NGX_RTMP_VAR_PREFIX, 0 },

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


ngx_rtmp_variable_value_t  ngx_rtmp_variable_null_value =
    ngx_rtmp_variable("");
ngx_rtmp_variable_value_t  ngx_rtmp_variable_true_value =
    ngx_rtmp_variable("1");


static ngx_uint_t  ngx_rtmp_variable_depth = 100;


extern char *ngx_live_stage[];
extern char *ngx_live_err[];


static ngx_int_t
ngx_rtmp_variable_session_get_str(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_str_t                  *str;

    str = (ngx_str_t *) ((u_char *) s + data);

    v->data = str->data;
    v->len = str->len;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_session_get_uint(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_uint_t                 *ui;

    ui = (ngx_uint_t *) ((u_char *) s + data);

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT_T_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%ui", *ui) - v->data;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_session_get_int(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_int_t                  *i;

    i = (ngx_int_t *) ((u_char *) s + data);

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT_T_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%i", *i) - v->data;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_session_get_msec(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_msec_t                 *m;

    m = (ngx_msec_t *) ((u_char *) s + data);

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT_T_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%i", *m) - v->data;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_relay_get_str(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_live_relay_ctx_t       *ctx;
    ngx_str_t                  *str;

    if (!s->relay) {
        v->not_found = 1;
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    str = (ngx_str_t *) ((u_char *) ctx + data);

    v->data = str->data;
    v->len = str->len;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_relay_get_uint(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_live_relay_ctx_t       *ctx;
    ngx_uint_t                 *ui;

    if (!s->relay) {
        v->not_found = 1;
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    ui = (ngx_uint_t *) ((u_char *) ctx + data);

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT_T_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%ui", *ui) - v->data;
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_finalize_reason(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->data = (u_char *) ngx_live_err[s->finalize_reason];
    v->len = ngx_strlen(v->data);
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_stage(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->data = (u_char *) ngx_live_stage[s->stage];
    v->len = ngx_strlen(v->data);
    v->valid = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_remote_addr(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_sockaddr_t              nsa;
    struct sockaddr            *sa;
    socklen_t                   len;

    if (s->connection == NULL || s->connection->fd == (ngx_socket_t) -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    sa = (struct sockaddr *) &nsa;
    len = sizeof(nsa);

    if (getpeername(s->connection->fd, sa, &len) == -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_SOCKADDRLEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = ngx_sock_ntop(sa, sizeof(nsa), v->data, NGX_SOCKADDRLEN, 0);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_remote_port(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_sockaddr_t              nsa;
    struct sockaddr            *sa;
    socklen_t                   len;
    in_port_t                   port;

    if (s->connection == NULL || s->connection->fd == (ngx_socket_t) -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    sa = (struct sockaddr *) &nsa;
    len = sizeof(nsa);

    if (getpeername(s->connection->fd, sa, &len) == -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, sizeof("65535") - 1);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    port = ngx_inet_get_port(sa);
    if (port == 0) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = ngx_sprintf(v->data, "%ui", port) - v->data;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_local_addr(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_sockaddr_t              nsa;
    struct sockaddr            *sa;
    socklen_t                   len;

    if (s->connection == NULL || s->connection->fd == (ngx_socket_t) -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    sa = (struct sockaddr *) &nsa;
    len = sizeof(nsa);

    if (getsockname(s->connection->fd, sa, &len) == -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_SOCKADDRLEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = ngx_sock_ntop(sa, sizeof(nsa), v->data, NGX_SOCKADDRLEN, 0);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_local_port(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_sockaddr_t              nsa;
    struct sockaddr            *sa;
    socklen_t                   len;
    in_port_t                   port;

    if (s->connection == NULL || s->connection->fd == (ngx_socket_t) -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    sa = (struct sockaddr *) &nsa;
    len = sizeof(nsa);

    if (getsockname(s->connection->fd, sa, &len) == -1) {
        v->not_found = 1;
        return NGX_OK;
    }

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, sizeof("65535") - 1);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    port = ngx_inet_get_port(sa);
    if (port == 0) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->len = ngx_sprintf(v->data, "%ui", port) - v->data;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_nginx_version(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    v->len = sizeof(NGINX_VERSION) - 1;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;
    v->data = (u_char *) NGINX_VERSION;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_pid(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT64_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%P", ngx_pid) - v->data;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_msec(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_time_t                 *tp;

    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_TIME_T_LEN + 4);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    tp = ngx_timeofday();

    v->len = ngx_sprintf(v->data, "%T.%03M", tp->sec, tp->msec) - v->data;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_time_iso8601(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, ngx_cached_http_log_iso8601.len);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    ngx_memcpy(v->data, ngx_cached_http_log_iso8601.data,
               ngx_cached_http_log_iso8601.len);

    v->len = ngx_cached_http_log_iso8601.len;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_time_local(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, ngx_cached_http_log_time.len);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    ngx_memcpy(v->data, ngx_cached_http_log_time.data,
               ngx_cached_http_log_time.len);

    v->len = ngx_cached_http_log_time.len;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_ngx_worker(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    if (v->data == NULL) {
        v->data = ngx_pnalloc(s->pool, NGX_INT64_LEN);
        if (v->data == NULL) {
            return NGX_ERROR;
        }
    }

    v->len = ngx_sprintf(v->data, "%ui", ngx_worker) - v->data;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_variable_argument(ngx_rtmp_session_t *s,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_str_t                  *name = (ngx_str_t *) data;

    u_char                     *arg;
    size_t                      len;
    ngx_str_t                   value;

    len = name->len - (sizeof("parg_") - 1);
    arg = name->data + sizeof("parg_") - 1;

    if (ngx_rtmp_arg(s, arg, len, &value) != NGX_OK) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->data = value.data;
    v->len = value.len;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_rtmp_variable_t *
ngx_rtmp_add_prefix_variable(ngx_conf_t *cf, ngx_str_t *name, ngx_uint_t flags)
{
    ngx_uint_t                  i;
    ngx_rtmp_variable_t        *v;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    v = cmcf->prefix_variables.elts;
    for (i = 0; i < cmcf->prefix_variables.nelts; i++) {
        if (name->len != v[i].name.len
            || ngx_strncasecmp(name->data, v[i].name.data, name->len) != 0)
        {
            continue;
        }

        v = &v[i];

        if (!(v->flags & NGX_HTTP_VAR_CHANGEABLE)) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "the duplicate \"%V\" variable", name);
            return NULL;
        }

        v->flags &= flags | ~NGX_HTTP_VAR_WEAK;

        return v;
    }

    v = ngx_array_push(&cmcf->prefix_variables);
    if (v == NULL) {
        return NULL;
    }

    v->name.len = name->len;
    v->name.data = ngx_pnalloc(cf->pool, name->len);
    if (v->name.data == NULL) {
        return NULL;
    }

    ngx_strlow(v->name.data, name->data, name->len);

    v->set_handler = NULL;
    v->get_handler = NULL;
    v->data = 0;
    v->flags = flags;
    v->index = 0;

    return v;
}


static u_char *
ngx_rtmp_strlechr(u_char *p, u_char *last)
{
    while (p != last) {
        if ((*p >= '0' && *p <= '9') ||
            (*p >= 'a' && *p <= 'z') ||
            (*p >= 'A' && *p <= 'Z') ||
            *p == '_')
        {
            p++;
            continue;
        }

        return p;
    }

    return NULL;
}


ngx_int_t
ngx_rtmp_variable_transform_index(ngx_conf_t *cf, ngx_str_t *origin, ngx_str_t *target)
{
    u_char                   *p, *e, *t;
    u_char                   *wp, *we;
    ngx_str_t                 str, var;
    ngx_buf_t                *buf;
    ngx_int_t                 index;

    p = origin->data;
    e = origin->data + origin->len;

    buf = ngx_create_temp_buf(cf->pool, 2 * origin->len);
    if (buf == NULL) {
        return NGX_ERROR;
    }

    wp = buf->start;
    we = buf->end;

    while (p < e) {
        t = ngx_strlchr(p, e, '$');
        if (t == NULL) {
            t = e;
        }
        str.data = p;
        str.len = t - p;
        wp = ngx_slprintf(wp, we, "%V", &str);

        if (t == e) {
            break;
        }

        var.data = ++t;
        t = ngx_rtmp_strlechr(t, e);
        if (t == NULL) {
            t = e;
        }
        var.len = t - var.data;

        index = ngx_rtmp_get_variable_index(cf, &var);
        if (index == NGX_ERROR) {
            return NGX_ERROR;
        }

        wp = ngx_slprintf(wp, we, "$%d", index);
        p = t;
    }

    target->data = buf->start;
    target->len = wp - buf->start;

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_fetch_variable(ngx_rtmp_session_t *s, ngx_pool_t *pool,
                          ngx_str_t *origin, ngx_str_t *target)
{
    ngx_rtmp_variable_value_t         *vv;
    u_char                            *p, *e, *t;
    u_char                            *wp, *we;
    ngx_chain_t                       *ch, *cl, *ct;
    u_char                            *pt;
    ngx_uint_t                         length;
    ngx_int_t                          index;
    ngx_str_t                          var;

    length = 0;
    p = origin->data;
    e = p + origin->len;

    #define NGX_RTMP_NOTIFY_BUF(__start__, __end__)                            \
    ct = cl;                                                                   \
    pt = ngx_pcalloc(pool, sizeof(ngx_chain_t) +                               \
                     sizeof(ngx_buf_t) + __end__ - __start__);                 \
    cl = (ngx_chain_t*)pt;                                                     \
    cl->buf = (ngx_buf_t*)(pt + sizeof(ngx_chain_t));                          \
    cl->buf->start =                                                           \
    cl->buf->pos =                                                             \
    cl->buf->last = pt + sizeof(ngx_chain_t) + sizeof(ngx_buf_t);              \
    if (ch == NULL) {                                                          \
        ch = cl;                                                               \
    } else {                                                                   \
        ct->next = cl;                                                         \
    }                                                                          \
    cl->buf->last = ngx_cpymem(cl->buf->pos, __start__, __end__ - __start__);  \
    length += __end__ - __start__

    ch = cl = ct = NULL;

    while(p < e) {
        t = ngx_strlchr(p, e, '$');
        if (t == NULL) {
            t = e;
        }
        NGX_RTMP_NOTIFY_BUF(p, t);
        if (t == e) {
            break;
        }

        var.data = ++t;
        t = ngx_rtmp_strlechr(t, e);
        if (t == NULL) {
            t = e;
        }
        var.len = t - var.data;
        index = ngx_atoi(var.data, var.len);
        vv = ngx_rtmp_get_indexed_variable(s, index);
        if (vv == NULL) {
            p = t;
            continue;
        }
        wp = vv->data;
        we = vv->data + vv->len;

        NGX_RTMP_NOTIFY_BUF(wp, we);
        p = t;
    }

    #undef NGX_RTMP_NOTIFY_BUF

    wp = ngx_pcalloc(pool, length);
    we = wp;

    for (ct = ch; ct;) {
        we = ngx_cpymem(we, ct->buf->pos, ct->buf->last - ct->buf->pos);
        cl = ct->next;
        ngx_pfree(pool, ct);
        ct = cl;
    }
    target->data = wp;
    target->len = we - wp;
    if (target->len != length) {
        ngx_log_error(NGX_LOG_ERR, s->connection->log, 0,
        "variable: fetch_variable| target len = %d, content length = %d",
         target->len, length);
        return NGX_ERROR;
    }

    return NGX_OK;
}

ngx_rtmp_variable_t *
ngx_rtmp_add_variable(ngx_conf_t *cf, ngx_str_t *name, ngx_uint_t flags)
{
    ngx_int_t                   rc;
    ngx_uint_t                  i;
    ngx_hash_key_t             *key;
    ngx_rtmp_variable_t        *v;
    ngx_rtmp_core_main_conf_t  *cmcf;

    if (name->len == 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"$\"");
        return NULL;
    }

    if (flags & NGX_HTTP_VAR_PREFIX) {
        return ngx_rtmp_add_prefix_variable(cf, name, flags);
    }

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    key = cmcf->variables_keys->keys.elts;
    for (i = 0; i < cmcf->variables_keys->keys.nelts; i++) {
        if (name->len != key[i].key.len
            || ngx_strncasecmp(name->data, key[i].key.data, name->len) != 0)
        {
            continue;
        }

        v = key[i].value;

        if (!(v->flags & NGX_HTTP_VAR_CHANGEABLE)) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "the duplicate \"%V\" variable", name);
            return NULL;
        }

        v->flags &= flags | ~NGX_HTTP_VAR_WEAK;

        return v;
    }

    v = ngx_palloc(cf->pool, sizeof(ngx_rtmp_variable_t));
    if (v == NULL) {
        return NULL;
    }

    v->name.len = name->len;
    v->name.data = ngx_pnalloc(cf->pool, name->len);
    if (v->name.data == NULL) {
        return NULL;
    }

    ngx_strlow(v->name.data, name->data, name->len);

    v->set_handler = NULL;
    v->get_handler = NULL;
    v->data = 0;
    v->flags = flags;
    v->index = 0;

    rc = ngx_hash_add_key(cmcf->variables_keys, &v->name, v, 0);

    if (rc == NGX_ERROR) {
        return NULL;
    }

    if (rc == NGX_BUSY) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "conflicting variable name \"%V\"", name);
        return NULL;
    }

    return v;
}


ngx_int_t
ngx_rtmp_get_variable_index(ngx_conf_t *cf, ngx_str_t *name)
{
    ngx_uint_t                  i = 0, n;
    ngx_rtmp_variable_t        *v, *av;
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_hash_key_t             *key;

    if (name->len == 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"$\"");
        return NGX_ERROR;
    }

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    v = cmcf->variables.elts;
    key = cmcf->variables_keys->keys.elts;

    if (v == NULL) {
        if (ngx_array_init(&cmcf->variables, cf->pool, 4,
                           sizeof(ngx_http_variable_t))
            != NGX_OK)
        {
            return NGX_ERROR;
        }

    } else {
        for (i = 0; i < cmcf->variables.nelts; i++) {
            if (name->len != v[i].name.len
                || ngx_strncasecmp(name->data, v[i].name.data, name->len) != 0)
            {
                continue;
            }

            return i;
        }
    }

    v = ngx_array_push(&cmcf->variables);
    if (v == NULL) {
        return NGX_ERROR;
    }

    v->name.len = name->len;
    v->name.data = ngx_pnalloc(cf->pool, name->len);
    if (v->name.data == NULL) {
        return NGX_ERROR;
    }

    ngx_strlow(v->name.data, name->data, name->len);

    for (n = 0; n < cmcf->variables_keys->keys.nelts; n++) {
        av = key[n].value;
        if (av->get_handler
            && v->name.len == key[n].key.len
            && ngx_strncmp(v->name.data, key[n].key.data, v->name.len) == 0)
        {
            v->get_handler = av->get_handler;
            v->data = av->data;
            av->flags= NGX_RTMP_VAR_INDEXED;
            v->flags = av->flags;
            av->index = i;

            goto next;
        }
    }

    ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                "variables: get_variable_index| unknown \"%V\" variable",
                &v->name);

    return NGX_ERROR;

next:
    v->set_handler = NULL;
    v->index = cmcf->variables.nelts - 1;

    return v->index;
}


ngx_rtmp_variable_value_t *
ngx_rtmp_get_indexed_variable(ngx_rtmp_session_t *s, ngx_uint_t index)
{
    ngx_rtmp_variable_t        *v;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

    if (cmcf->variables.nelts <= index) {
        ngx_log_error(NGX_LOG_ALERT, s->log, 0,
                      "unknown variable index: %ui", index);
        return NULL;
    }

    if (s->variables[index].not_found || s->variables[index].valid) {
        return &s->variables[index];
    }

    v = cmcf->variables.elts;

    if (ngx_rtmp_variable_depth == 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "cycle while evaluating variable \"%V\"",
                      &v[index].name);
        return NULL;
    }

    ngx_rtmp_variable_depth--;

    if (v[index].get_handler(s, &s->variables[index], v[index].data)
        == NGX_OK)
    {
        ngx_rtmp_variable_depth++;

        if (v[index].flags & NGX_HTTP_VAR_NOCACHEABLE) {
            s->variables[index].no_cacheable = 1;
        }

        return &s->variables[index];
    }

    ngx_rtmp_variable_depth++;

    s->variables[index].valid = 0;
    s->variables[index].not_found = 1;

    return NULL;
}


ngx_rtmp_variable_value_t *
ngx_rtmp_get_flushed_variable(ngx_rtmp_session_t *s, ngx_uint_t index)
{
    ngx_rtmp_variable_value_t  *v;

    v = &s->variables[index];

    if (v->valid || v->not_found) {
        if (!v->no_cacheable) {
            return v;
        }

        v->valid = 0;
        v->not_found = 0;
    }

    return ngx_rtmp_get_indexed_variable(s, index);
}


ngx_rtmp_variable_value_t *
ngx_rtmp_get_variable(ngx_rtmp_session_t *s, ngx_str_t *name, ngx_uint_t key)
{
    size_t                      len;
    ngx_uint_t                  i, n;
    ngx_rtmp_variable_t        *v;
    ngx_rtmp_variable_value_t  *vv;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

    v = ngx_hash_find(&cmcf->variables_hash, key, name->data, name->len);

    if (v) {
        if (v->flags & NGX_HTTP_VAR_INDEXED) {
            return ngx_rtmp_get_flushed_variable(s, v->index);
        }

        if (ngx_rtmp_variable_depth == 0) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "cycle while evaluating variable \"%V\"", name);
            return NULL;
        }

        ngx_rtmp_variable_depth--;

        vv = ngx_palloc(s->pool, sizeof(ngx_rtmp_variable_value_t));

        if (vv && v->get_handler(s, vv, v->data) == NGX_OK) {
            ngx_rtmp_variable_depth++;
            return vv;
        }

        ngx_rtmp_variable_depth++;
        return NULL;
    }

    vv = ngx_palloc(s->pool, sizeof(ngx_rtmp_variable_value_t));
    if (vv == NULL) {
        return NULL;
    }

    len = 0;

    v = cmcf->prefix_variables.elts;
    n = cmcf->prefix_variables.nelts;

    for (i = 0; i < cmcf->prefix_variables.nelts; i++) {
        if (name->len >= v[i].name.len && name->len > len
            && ngx_strncmp(name->data, v[i].name.data, v[i].name.len) == 0)
        {
            len = v[i].name.len;
            n = i;
        }
    }

    if (n != cmcf->prefix_variables.nelts) {
        if (v[n].get_handler(s, vv, (uintptr_t) name) == NGX_OK) {
            return vv;
        }

        return NULL;
    }

    vv->not_found = 1;

    return vv;
}


void *
ngx_rtmp_map_find(ngx_rtmp_session_t *s, ngx_rtmp_map_t *map, ngx_str_t *match)
{
    void        *value;
    u_char      *low;
    size_t       len;
    ngx_uint_t   key;

    len = match->len;

    if (len) {
        low = ngx_pnalloc(s->pool, len);
        if (low == NULL) {
            return NULL;
        }

    } else {
        low = NULL;
    }

    key = ngx_hash_strlow(low, match->data, len);

    value = ngx_hash_find_combined(&map->hash, key, low, len);
    if (value) {
        return value;
    }

#if (NGX_PCRE)

    if (len && map->nregex) {
        ngx_int_t              n;
        ngx_uint_t             i;
        ngx_rtmp_map_regex_t  *reg;

        reg = map->regex;

        for (i = 0; i < map->nregex; i++) {

            n = ngx_rtmp_regex_exec(s, reg[i].regex, match);

            if (n == NGX_OK) {
                return reg[i].value;
            }

            if (n == NGX_DECLINED) {
                continue;
            }

            /* NGX_ERROR */

            return NULL;
        }
    }

#endif

    return NULL;
}


#if (NGX_PCRE)

static ngx_int_t
ngx_rtmp_variable_not_found(ngx_rtmp_session_t *s, ngx_rtmp_variable_value_t *v,
    uintptr_t data)
{
    v->not_found = 1;
    return NGX_OK;
}


ngx_rtmp_regex_t *
ngx_rtmp_regex_compile(ngx_conf_t *cf, ngx_regex_compile_t *rc)
{
    u_char                     *p;
    size_t                      size;
    ngx_str_t                   name;
    ngx_uint_t                  i, n;
    ngx_rtmp_variable_t        *v;
    ngx_rtmp_regex_t           *re;
    ngx_rtmp_regex_variable_t  *rv;
    ngx_rtmp_core_main_conf_t  *cmcf;

    rc->pool = cf->pool;

    if (ngx_regex_compile(rc) != NGX_OK) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "%V", &rc->err);
        return NULL;
    }

    re = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_regex_t));
    if (re == NULL) {
        return NULL;
    }

    re->regex = rc->regex;
    re->ncaptures = rc->captures;
    re->name = rc->pattern;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);
    cmcf->ncaptures = ngx_max(cmcf->ncaptures, re->ncaptures);

    n = (ngx_uint_t) rc->named_captures;

    if (n == 0) {
        return re;
    }

    rv = ngx_palloc(rc->pool, n * sizeof(ngx_rtmp_regex_variable_t));
    if (rv == NULL) {
        return NULL;
    }

    re->variables = rv;
    re->nvariables = n;

    size = rc->name_size;
    p = rc->names;

    for (i = 0; i < n; i++) {
        rv[i].capture = 2 * ((p[0] << 8) + p[1]);

        name.data = &p[2];
        name.len = ngx_strlen(name.data);

        v = ngx_rtmp_add_variable(cf, &name, NGX_HTTP_VAR_CHANGEABLE);
        if (v == NULL) {
            return NULL;
        }

        rv[i].index = ngx_rtmp_get_variable_index(cf, &name);
        if (rv[i].index == NGX_ERROR) {
            return NULL;
        }

        v->get_handler = ngx_rtmp_variable_not_found;

        p += size;
    }

    return re;
}


ngx_int_t
ngx_rtmp_regex_exec(ngx_rtmp_session_t *s, ngx_rtmp_regex_t *re, ngx_str_t *str)
{
    ngx_int_t                   rc, index;
    ngx_uint_t                  i, n, len;
    ngx_rtmp_variable_value_t  *vv;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

    if (re->ncaptures) {
        len = cmcf->ncaptures;

        if (s->captures == NULL) {
            s->captures = ngx_palloc(s->pool, len * sizeof(int));
            if (s->captures == NULL) {
                return NGX_ERROR;
            }
        }

    } else {
        len = 0;
    }

    rc = ngx_regex_exec(re->regex, str, s->captures, len);

    if (rc == NGX_REGEX_NO_MATCHED) {
        return NGX_DECLINED;
    }

    if (rc < 0) {
        ngx_log_error(NGX_LOG_ALERT, s->log, 0,
                      ngx_regex_exec_n " failed: %i on \"%V\" using \"%V\"",
                      rc, str, &re->name);
        return NGX_ERROR;
    }

    for (i = 0; i < re->nvariables; i++) {

        n = re->variables[i].capture;
        index = re->variables[i].index;
        vv = &s->variables[index];

        vv->len = s->captures[n + 1] - s->captures[n];
        vv->valid = 1;
        vv->no_cacheable = 0;
        vv->not_found = 0;
        vv->data = &str->data[s->captures[n]];

#if (NGX_DEBUG)
        {
        ngx_rtmp_variable_t  *v;

        v = cmcf->variables.elts;

        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, s->log, 0,
                       "http regex set $%V to \"%v\"", &v[index].name, vv);
        }
#endif
    }

    s->ncaptures = rc * 2;
    s->captures_data = str->data;

    return NGX_OK;
}

#endif


ngx_int_t
ngx_rtmp_variables_add_core_vars(ngx_conf_t *cf)
{
    ngx_rtmp_variable_t        *cv, *v;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    cmcf->variables_keys = ngx_pcalloc(cf->temp_pool,
                                       sizeof(ngx_hash_keys_arrays_t));
    if (cmcf->variables_keys == NULL) {
        return NGX_ERROR;
    }

    cmcf->variables_keys->pool = cf->pool;
    cmcf->variables_keys->temp_pool = cf->pool;

    if (ngx_hash_keys_array_init(cmcf->variables_keys, NGX_HASH_SMALL)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    if (ngx_array_init(&cmcf->prefix_variables, cf->pool, 8,
                       sizeof(ngx_rtmp_variable_t))
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    for (cv = ngx_rtmp_core_variables; cv->name.len; cv++) {
        v = ngx_rtmp_add_variable(cf, &cv->name, cv->flags);
        if (v == NULL) {
            return NGX_ERROR;
        }

        *v = *cv;
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_variables_init_vars(ngx_conf_t *cf)
{
    size_t                      len;
    ngx_uint_t                  i, n;
    ngx_hash_key_t             *key;
    ngx_hash_init_t             hash;
    ngx_rtmp_variable_t        *v, *av, *pv;
    ngx_rtmp_core_main_conf_t  *cmcf;

    /* set the handlers for the indexed http variables */

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    v = cmcf->variables.elts;
    pv = cmcf->prefix_variables.elts;
    key = cmcf->variables_keys->keys.elts;

    for (i = 0; i < cmcf->variables.nelts; i++) {

        for (n = 0; n < cmcf->variables_keys->keys.nelts; n++) {

            av = key[n].value;

            if (v[i].name.len == key[n].key.len
                && ngx_strncmp(v[i].name.data, key[n].key.data, v[i].name.len)
                   == 0)
            {
                v[i].get_handler = av->get_handler;
                v[i].data = av->data;

                av->flags |= NGX_HTTP_VAR_INDEXED;
                v[i].flags = av->flags;

                av->index = i;

                if (av->get_handler == NULL
                    || (av->flags & NGX_HTTP_VAR_WEAK))
                {
                    break;
                }

                goto next;
            }
        }

        len = 0;
        av = NULL;

        for (n = 0; n < cmcf->prefix_variables.nelts; n++) {
            if (v[i].name.len >= pv[n].name.len && v[i].name.len > len
                && ngx_strncmp(v[i].name.data, pv[n].name.data, pv[n].name.len)
                   == 0)
            {
                av = &pv[n];
                len = pv[n].name.len;
            }
        }

        if (av) {
            v[i].get_handler = av->get_handler;
            v[i].data = (uintptr_t) &v[i].name;
            v[i].flags = av->flags;

            goto next;
        }

        if (v[i].get_handler == NULL) {
            ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                          "unknown \"%V\" variable", &v[i].name);

            return NGX_ERROR;
        }

    next:
        continue;
    }


    for (n = 0; n < cmcf->variables_keys->keys.nelts; n++) {
        av = key[n].value;

        if (av->flags & NGX_HTTP_VAR_NOHASH) {
            key[n].key.data = NULL;
        }
    }


    hash.hash = &cmcf->variables_hash;
    hash.key = ngx_hash_key;
    hash.max_size = cmcf->variables_hash_max_size;
    hash.bucket_size = cmcf->variables_hash_bucket_size;
    hash.name = "variables_hash";
    hash.pool = cf->pool;
    hash.temp_pool = NULL;

    if (ngx_hash_init(&hash, cmcf->variables_keys->keys.elts,
                      cmcf->variables_keys->keys.nelts)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    cmcf->variables_keys = NULL;

    return NGX_OK;
}
