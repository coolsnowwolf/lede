/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_live_relay.h"
#include "ngx_poold.h"
#include "ngx_toolkit_misc.h"
#include "ngx_dynamic_resolver.h"


static ngx_int_t ngx_live_relay_rtmp_postconfiguration(ngx_conf_t *cf);
static void *ngx_live_relay_rtmp_create_app_conf(ngx_conf_t *cf);
static char *ngx_live_relay_rtmp_merge_app_conf(ngx_conf_t *cf, void *parent,
       void *child);


typedef struct {
    ngx_msec_t                  buflen;
} ngx_live_relay_rtmp_app_conf_t;


typedef struct {
    char                       *code;
    ngx_uint_t                  status;
    ngx_flag_t                  finalize;
} ngx_rtmp_status_code_t;


static ngx_rtmp_status_code_t ngx_rtmp_relay_status_error_code[] = {
    { "NetStream.Publish.BadName",      400, 1 },
    { "NetStream.Stream.Forbidden",     403, 1 },
    { "NetStream.Play.StreamNotFound",  404, 1 },
    { "NetStream.Relay.ServerError",    500, 1 },
    { NULL, 0, 0 }
};


#define NGX_RTMP_RELAY_CONNECT_TRANS            1
#define NGX_RTMP_RELAY_CREATE_STREAM_TRANS      2


#define NGX_RTMP_RELAY_CSID_AMF_INI             3
#define NGX_RTMP_RELAY_CSID_AMF                 5
#define NGX_RTMP_RELAY_MSID                     1


static ngx_command_t  ngx_live_relay_rtmp_commands[] = {

    { ngx_string("relay_buffer"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_relay_rtmp_app_conf_t, buflen),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_relay_rtmp_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_relay_rtmp_postconfiguration,  /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_live_relay_rtmp_create_app_conf,    /* create app configuration */
    ngx_live_relay_rtmp_merge_app_conf      /* merge app configuration */
};


ngx_module_t  ngx_live_relay_rtmp_module = {
    NGX_MODULE_V1,
    &ngx_live_relay_rtmp_module_ctx,        /* module context */
    ngx_live_relay_rtmp_commands,           /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
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
ngx_live_relay_rtmp_create_app_conf(ngx_conf_t *cf)
{
    ngx_live_relay_rtmp_app_conf_t *racf;

    racf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_rtmp_app_conf_t));
    if (racf == NULL) {
        return NULL;
    }

    racf->buflen = NGX_CONF_UNSET_MSEC;

    return racf;
}


static char *
ngx_live_relay_rtmp_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_live_relay_rtmp_app_conf_t *prev = parent;
    ngx_live_relay_rtmp_app_conf_t *conf = child;

    ngx_conf_merge_msec_value(conf->buflen, prev->buflen, 5000);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_send_connect(ngx_rtmp_session_t *s)
{
    ngx_str_t                       app, tcurl;
    double                          acodecs = 3575, vcodecs = 252;
    static double                   trans = NGX_RTMP_RELAY_CONNECT_TRANS;

    static ngx_rtmp_amf_elt_t       out_cmd[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_string("app"),
          NULL, 0 }, /* <-- fill */

        { NGX_RTMP_AMF_STRING,
          ngx_string("tcUrl"),
          NULL, 0 }, /* <-- fill */

        { NGX_RTMP_AMF_STRING,
          ngx_string("pageUrl"),
          NULL, 0 }, /* <-- fill */

        { NGX_RTMP_AMF_STRING,
          ngx_string("swfUrl"),
          NULL, 0 }, /* <-- fill */

        { NGX_RTMP_AMF_STRING,
          ngx_string("flashVer"),
          NULL, 0 }, /* <-- fill */

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("audioCodecs"),
          NULL, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("videoCodecs"),
          NULL, 0 }
    };

    static ngx_rtmp_amf_elt_t   out_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "connect", 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &trans, 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          out_cmd, sizeof(out_cmd) }
    };

    ngx_rtmp_core_app_conf_t       *cacf;
    ngx_rtmp_core_srv_conf_t       *cscf;
    ngx_live_relay_ctx_t           *ctx;
    ngx_rtmp_header_t               h;


    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (cacf == NULL || ctx == NULL) {
        return NGX_ERROR;
    }

    /* app */
    if (ctx->args.len) {
        app.len = ctx->app.len + 1 + ctx->args.len;
        app.data = ngx_pcalloc(s->pool, app.len);
        if (app.data == NULL) {
            return NGX_ERROR;
        }

        ngx_snprintf(app.data, app.len, "%V?%V", &ctx->app, &ctx->args);
    } else {
        app = ctx->app;
    }

    out_cmd[0].data = app.data;
    out_cmd[0].len  = app.len;

    /* tcUrl */
    tcurl.len = sizeof("rtmp://") - 1 + ctx->domain.len + 1 + ctx->app.len;
    tcurl.data = ngx_palloc(s->pool, tcurl.len);
    if (tcurl.data == NULL) {
        return NGX_ERROR;
    }
    ngx_snprintf(tcurl.data, tcurl.len, "rtmp://%V/%V",
            &ctx->domain, &ctx->app);

    out_cmd[1].data = tcurl.data;
    out_cmd[1].len = tcurl.len;

    /* pageUrl */
    out_cmd[2].data = ctx->referer.data;
    out_cmd[2].len  = ctx->referer.len;

    /* swfUrl */
    out_cmd[3].data = ctx->swf_url.data;
    out_cmd[3].len  = ctx->swf_url.len;

    /* flashVer */
    out_cmd[4].data = ctx->user_agent.data;
    out_cmd[4].len  = ctx->user_agent.len;

    if (ctx->acodecs != 0) {
        acodecs = (double) ctx->acodecs;
    }
    out_cmd[5].data = &acodecs;

    if (ctx->vcodecs != 0) {
        vcodecs = (double) ctx->vcodecs;
    }
    out_cmd[6].data = &vcodecs;

    ngx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    s->status = NGX_LIVE_CONNECT;
    s->connect_time = ngx_current_msec;

    return ngx_rtmp_send_chunk_size(s, cscf->chunk_size) != NGX_OK
        || ngx_rtmp_send_ack_size(s, cscf->ack_window) != NGX_OK
        || ngx_rtmp_send_amf(s, &h, out_elts,
            sizeof(out_elts) / sizeof(out_elts[0])) != NGX_OK
        ? NGX_ERROR
        : NGX_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_send_create_stream(ngx_rtmp_session_t *s)
{
    static double                   trans = NGX_RTMP_RELAY_CREATE_STREAM_TRANS;

    static ngx_rtmp_amf_elt_t       out_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "createStream", 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 }
    };

    ngx_rtmp_header_t               h;


    ngx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF_INI;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    s->stage = NGX_LIVE_CREATE_STREAM;
    s->create_stream_time = ngx_current_msec;

    return ngx_rtmp_send_amf(s, &h, out_elts,
            sizeof(out_elts) / sizeof(out_elts[0]));
}


static ngx_int_t
ngx_live_relay_rtmp_send_publish(ngx_rtmp_session_t *s)
{
    ngx_str_t                       name;
    static double                   trans;

    static ngx_rtmp_amf_elt_t       out_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "publish", 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          NULL, 0 }, /* <- to fill */

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "live", 0 }
    };

    ngx_rtmp_header_t               h;
    ngx_live_relay_ctx_t           *ctx;


    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        return NGX_ERROR;
    }

    if (ctx->pargs.len) {
        name.len = ctx->name.len + 1 + ctx->pargs.len;
        name.data = ngx_pcalloc(s->pool, name.len);
        if (name.data == NULL) {
            return NGX_ERROR;
        }

        ngx_snprintf(name.data, name.len, "%V?%V", &ctx->name, &ctx->pargs);
    } else {
        name = ctx->name;
    }

    out_elts[3].data = name.data;
    out_elts[3].len  = name.len;

    ngx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF;
    h.msid = NGX_RTMP_RELAY_MSID;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    s->stage = NGX_LIVE_PUBLISH;
    s->ptime = ngx_current_msec;

    return ngx_rtmp_send_amf(s, &h, out_elts,
            sizeof(out_elts) / sizeof(out_elts[0]));
}


static ngx_int_t
ngx_live_relay_rtmp_send_play(ngx_rtmp_session_t *s)
{
    ngx_str_t                       name;
    static double                   trans;
    static double                   start, duration;

    static ngx_rtmp_amf_elt_t       out_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "play", 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          NULL, 0 }, /* <- fill */

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &start, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &duration, 0 },
    };

    ngx_rtmp_header_t               h;
    ngx_live_relay_ctx_t           *ctx;
    ngx_live_relay_rtmp_app_conf_t *racf;


    racf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_rtmp_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (racf == NULL) {
        return NGX_ERROR;
    }

    if (ctx->pargs.len) {
        name.len = ctx->name.len + 1 + ctx->pargs.len;
        name.data = ngx_pcalloc(s->pool, name.len);
        if (name.data == NULL) {
            return NGX_ERROR;
        }

        ngx_snprintf(name.data, name.len, "%V?%V", &ctx->name, &ctx->pargs);
    } else {
        name = ctx->name;
    }

    out_elts[3].data = name.data;
    out_elts[3].len  = name.len;

    start = -1000;
    duration = -1000;

    ngx_memzero(&h, sizeof(h));
    h.csid = NGX_RTMP_RELAY_CSID_AMF;
    h.msid = NGX_RTMP_RELAY_MSID;
    h.type = NGX_RTMP_MSG_AMF_CMD;

    s->stage = NGX_LIVE_PLAY;
    s->ptime = ngx_current_msec;

    return ngx_rtmp_send_amf(s, &h, out_elts,
            sizeof(out_elts) / sizeof(out_elts[0])) != NGX_OK
           || ngx_rtmp_send_set_buflen(s, NGX_RTMP_RELAY_MSID,
                   racf->buflen) != NGX_OK
           ? NGX_ERROR
           : NGX_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_status_error(ngx_rtmp_session_t *s, char *type, char *code,
        char *level, char *desc)
{
    ngx_rtmp_core_ctx_t            *cctx;
    size_t                          i;
    ngx_flag_t                      status = 0;

    if (ngx_strcmp(type, "onStatus") == 0) {
        status = 1;
    }

    for (i = 0; ngx_rtmp_relay_status_error_code[i].code; ++i) {

        if (ngx_strcmp(ngx_rtmp_relay_status_error_code[i].code, code)
                != 0)
        {
            continue;
        }

        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "relay transit, %s: level='%s' code='%s' description='%s'",
                type, level, code, desc);

        if (s->publishing) {
            cctx = s->live_stream->play_ctx;
        } else {
            cctx = s->live_stream->publish_ctx;
        }

        for (; cctx; cctx = cctx->next) {
            cctx->session->status = ngx_rtmp_relay_status_error_code[i].status;
            status ? ngx_rtmp_send_status(cctx->session, code, level, desc)
                   : ngx_rtmp_send_error(cctx->session, code, level, desc);

            if (ngx_rtmp_relay_status_error_code[i].finalize
                    && !cctx->session->static_pull)
            {
                cctx->session->finalize_reason = NGX_LIVE_RELAY_TRANSIT;
                ngx_rtmp_finalize_session(cctx->session);
            }
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_on_result(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_relay_ctx_t           *ctx;
    static struct {
        double                      trans;
        u_char                      level[32];
        u_char                      code[128];
        u_char                      desc[1024];
    } v;

    static ngx_rtmp_amf_elt_t       in_inf[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_string("level"),
          &v.level, sizeof(v.level) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("code"),
          &v.code, sizeof(v.code) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("description"),
          &v.desc, sizeof(v.desc) },
    };

    static ngx_rtmp_amf_elt_t       in_elts[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &v.trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          in_inf, sizeof(in_inf) },
    };


    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    ngx_memzero(&v, sizeof(v));
    if (ngx_rtmp_receive_amf(s, in, in_elts,
                sizeof(in_elts) / sizeof(in_elts[0])))
    {
        return NGX_ERROR;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "relay: _result: level='%s' code='%s' description='%s'",
            v.level, v.code, v.desc);

    switch ((ngx_int_t)v.trans) {
        case NGX_RTMP_RELAY_CONNECT_TRANS:
            return ngx_live_relay_rtmp_send_create_stream(s);

        case NGX_RTMP_RELAY_CREATE_STREAM_TRANS:
            if (s->publishing == 0) {
                if (ngx_live_relay_rtmp_send_publish(s) != NGX_OK) {
                    return NGX_ERROR;
                }
                return ngx_live_relay_play_local(s);

            } else {
                if (ngx_live_relay_rtmp_send_play(s) != NGX_OK) {
                    return NGX_ERROR;
                }
                return ngx_live_relay_publish_local(s);
            }

        default:
            return NGX_OK;
    }
}


static ngx_int_t
ngx_live_relay_rtmp_on_error(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_relay_ctx_t           *ctx;
    static struct {
        double                      trans;
        u_char                      level[32];
        u_char                      code[128];
        u_char                      desc[1024];
    } v;

    static ngx_rtmp_amf_elt_t       in_inf[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_string("level"),
          &v.level, sizeof(v.level) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("code"),
          &v.code, sizeof(v.code) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("description"),
          &v.desc, sizeof(v.desc) },
    };

    static ngx_rtmp_amf_elt_t       in_elts[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &v.trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          in_inf, sizeof(in_inf) },
    };


    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    ngx_memzero(&v, sizeof(v));
    if (ngx_rtmp_receive_amf(s, in, in_elts,
                sizeof(in_elts) / sizeof(in_elts[0])))
    {
        return NGX_ERROR;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "relay: _error: level='%s' code='%s' description='%s'",
            v.level, v.code, v.desc);

    ngx_live_relay_rtmp_status_error(s, "_error", (char *) v.code,
            (char *) v.level, (char *) v.desc);

    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_on_status(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_relay_ctx_t           *ctx;
    static struct {
        double                      trans;
        u_char                      level[32];
        u_char                      code[128];
        u_char                      desc[1024];
    } v;

    static ngx_rtmp_amf_elt_t       in_inf[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_string("level"),
          &v.level, sizeof(v.level) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("code"),
          &v.code, sizeof(v.code) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("description"),
          &v.desc, sizeof(v.desc) },
    };

    static ngx_rtmp_amf_elt_t       in_elts[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &v.trans, 0 },

        { NGX_RTMP_AMF_NULL,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          in_inf, sizeof(in_inf) },
    };

    static ngx_rtmp_amf_elt_t       in_elts_meta[] = {

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          in_inf, sizeof(in_inf) },
    };


    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    ngx_memzero(&v, sizeof(v));
    if (h->type == NGX_RTMP_MSG_AMF_META) {
        ngx_rtmp_receive_amf(s, in, in_elts_meta,
                sizeof(in_elts_meta) / sizeof(in_elts_meta[0]));
    } else {
        ngx_rtmp_receive_amf(s, in, in_elts,
                sizeof(in_elts) / sizeof(in_elts[0]));
    }

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "relay: onStatus: level='%s' code='%s' description='%s'",
            v.level, v.code, v.desc);

    ngx_live_relay_rtmp_status_error(s, "onStatus", (char *) v.code,
            (char *) v.level, (char *) v.desc);

    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_rtmp_handshake_done(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_relay_ctx_t           *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    return ngx_live_relay_rtmp_send_connect(s);
}


static ngx_int_t
ngx_live_relay_rtmp_get_peer(ngx_peer_connection_t *pc, void *data)
{
    return NGX_OK;
}


static void
ngx_live_relay_rtmp_free_peer(ngx_peer_connection_t *pc, void *data,
        ngx_uint_t state)
{
}


ngx_int_t
ngx_live_relay_create_rtmp(ngx_rtmp_session_t *s, ngx_live_relay_t *relay,
        ngx_live_relay_url_t *url)
{
    ngx_live_relay_ctx_t           *rctx;
    ngx_pool_t                     *pool;
    ngx_peer_connection_t          *pc;
    ngx_connection_t               *c;
    ngx_int_t                       rc;
    ngx_str_t                       name;
    u_char                          text[NGX_SOCKADDRLEN];
    // must use ngx_sockaddr_t, because sizeof(struct sockaddr)
    //   is not long enouph, content will be covered by other var
    ngx_sockaddr_t                  nsa;
    struct sockaddr                *sa;
    socklen_t                       len;

    rctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (rctx == NULL) {
        return NGX_ERROR;
    }

    pool = ngx_create_pool(4096, ngx_cycle->log);
    if (pool == NULL) {
        goto destroy;
    }

#define NGX_LIVE_RELAY_CTX(para)                                        \
    if (ngx_copy_str(s->pool, &rctx->para, &relay->para) != NGX_OK) {   \
        goto destroy;                                                   \
    }

    NGX_LIVE_RELAY_CTX(domain);
    NGX_LIVE_RELAY_CTX(app);
    NGX_LIVE_RELAY_CTX(name);
    NGX_LIVE_RELAY_CTX(pargs);
    NGX_LIVE_RELAY_CTX(referer);
    NGX_LIVE_RELAY_CTX(user_agent);
#undef NGX_LIVE_RELAY_CTX

    rctx->tag = relay->tag;

    // connect server
    pc = ngx_pcalloc(s->pool, sizeof(ngx_peer_connection_t));
    if (pc == NULL) {
        goto destroy;
    }
    pc->log = s->log;

    // get address
    sa = (struct sockaddr *) &nsa;
    len = ngx_dynamic_resolver_gethostbyname(&url->url.host, sa);
    if (len == 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "relay rtmp: gethostbyname failed %V", &url->url.host);
        goto destroy;
    }

    if (url->port != 0) {
        ngx_inet_set_port(sa, url->port);
    }

    ngx_memzero(text, sizeof(text));
    name.len = ngx_sock_ntop(sa, len, text, NGX_SOCKADDRLEN, 1);
    name.data = text;

    /* copy log to keep shared log unchanged */
    pc->get = ngx_live_relay_rtmp_get_peer;
    pc->free = ngx_live_relay_rtmp_free_peer;
    pc->name = &name;
    pc->socklen = len;
    pc->sockaddr = sa;

    rc = ngx_event_connect_peer(pc);
    if (rc != NGX_OK && rc != NGX_AGAIN ) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "relay: connection failed");
        goto destroy;
    }
    c = pc->connection;
    if (ngx_copy_str(pool, &c->addr_text, &url->url.host_with_port) != NGX_OK) {
        goto destroy;
    }
    ngx_rtmp_init_session(s, c);
    c->pool = pool;

#if (NGX_STAT_STUB)
    (void) ngx_atomic_fetch_add(ngx_stat_active, 1);
#endif

    ngx_rtmp_client_handshake(s, 1);

    return NGX_OK;

destroy:
    if (pool) {
        ngx_destroy_pool(pool);
    }

    ngx_rtmp_finalize_session(s);

    return NGX_ERROR;
}


static ngx_int_t
ngx_live_relay_rtmp_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t      *cmcf;
    ngx_rtmp_handler_pt            *h;
    ngx_rtmp_amf_handler_t         *ch;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_HANDSHAKE_DONE]);
    *h = ngx_live_relay_rtmp_handshake_done;

    ch = ngx_array_push(&cmcf->amf);
    ngx_str_set(&ch->name, "_result");
    ch->handler = ngx_live_relay_rtmp_on_result;

    ch = ngx_array_push(&cmcf->amf);
    ngx_str_set(&ch->name, "_error");
    ch->handler = ngx_live_relay_rtmp_on_error;

    ch = ngx_array_push(&cmcf->amf);
    ngx_str_set(&ch->name, "onStatus");
    ch->handler = ngx_live_relay_rtmp_on_status;

    return NGX_OK;
}
