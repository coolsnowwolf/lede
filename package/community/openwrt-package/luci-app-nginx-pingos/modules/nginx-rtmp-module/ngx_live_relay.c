/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_live_relay.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_toolkit_misc.h"


static ngx_rtmp_publish_pt              next_publish;
static ngx_rtmp_play_pt                 next_play;
static ngx_rtmp_close_stream_pt         next_close_stream;


static ngx_int_t ngx_live_relay_postconfiguration(ngx_conf_t *cf);
static void *ngx_live_relay_create_app_conf(ngx_conf_t *cf);
static char *ngx_live_relay_merge_app_conf(ngx_conf_t *cf, void *parent,
       void *child);


ngx_live_push_pt                        ngx_live_push;
ngx_live_pull_pt                        ngx_live_pull;
ngx_live_push_close_pt                  ngx_live_push_close;
ngx_live_pull_close_pt                  ngx_live_pull_close;


typedef ngx_int_t (* relay_create_pt)(ngx_rtmp_session_t *rs,
                                      ngx_live_relay_t *relay,
                                      ngx_live_relay_url_t *url);


static relay_create_pt create_relay[] = {
    ngx_live_relay_create_httpflv,
    ngx_live_relay_create_rtmp
};


static const char *relay_protocol[] = {
    "httpflv",
    "rtmp"
};


static const char *relay_type[] = {
    "push",
    "pull"
};


static ngx_command_t  ngx_live_relay_commands[] = {

    { ngx_string("failed_reconnect"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_relay_app_conf_t, failed_reconnect),
      NULL },

    { ngx_string("relay_reconnect"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_relay_app_conf_t, relay_reconnect),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_relay_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_relay_postconfiguration,       /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_live_relay_create_app_conf,         /* create app configuration */
    ngx_live_relay_merge_app_conf           /* merge app configuration */
};


ngx_module_t  ngx_live_relay_module = {
    NGX_MODULE_V1,
    &ngx_live_relay_module_ctx,             /* module context */
    ngx_live_relay_commands,                /* module directives */
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
ngx_live_relay_create_app_conf(ngx_conf_t *cf)
{
    ngx_live_relay_app_conf_t  *racf;

    racf = ngx_pcalloc(cf->pool, sizeof(ngx_live_relay_app_conf_t));
    if (racf == NULL) {
        return NULL;
    }

    racf->failed_reconnect = NGX_CONF_UNSET_MSEC;
    racf->relay_reconnect = NGX_CONF_UNSET_MSEC;

    return racf;
}


static char *
ngx_live_relay_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_live_relay_app_conf_t  *prev = parent;
    ngx_live_relay_app_conf_t  *conf = child;

    ngx_conf_merge_msec_value(conf->failed_reconnect, prev->failed_reconnect,
            50);
    ngx_conf_merge_msec_value(conf->relay_reconnect, prev->relay_reconnect,
            3000);

    return NGX_CONF_OK;
}


ngx_int_t
ngx_live_relay_create(ngx_rtmp_session_t *rs, ngx_live_relay_t *relay)
{
    ngx_live_relay_ctx_t       *ctx;
    ngx_live_relay_url_t       *url;
    relay_create_pt             create;

    ctx = ngx_rtmp_get_module_ctx(rs, ngx_live_relay_module);
    if (ctx->idx >= relay->urls.nelts) {
        ctx->idx = 0;
    }
    ctx->failed_delay = 0;

    url = relay->urls.elts;
    url += ctx->idx;

    if (url->relay_type >= NGX_LIVE_RELAY_MAXTYPE) {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0, "unsupported relay type %ui",
                url->relay_type);
        return NGX_ERROR;
    }

    create = create_relay[url->relay_type];

    ngx_log_error(NGX_LOG_INFO, rs->log, 0,
            "create %s relay %s to %V:%d, domain='%V' app='%V' name='%V' "
            "pargs='%V' referer='%V' user_agent='%V'",
            relay_protocol[url->relay_type], relay_type[rs->publishing],
            &url->url.host, url->port, &relay->domain, &relay->app,
            &relay->name, &relay->pargs, &relay->referer, &relay->user_agent);

    ++ctx->idx;

    return create(rs, relay, url);
}


ngx_int_t
ngx_live_relay_play_local(ngx_rtmp_session_t *rs)
{
    ngx_rtmp_play_t             v;

    ngx_memzero(&v, sizeof(ngx_rtmp_play_t));
    v.silent = 1;
    *(ngx_cpymem(v.name, rs->name.data, ngx_min(sizeof(v.name) - 1,
                 rs->name.len))) = 0;
    if (rs->pargs.len) {
        *(ngx_cpymem(v.args, rs->pargs.data, ngx_min(sizeof(v.args) - 1,
                     rs->pargs.len))) = 0;
    }

    return ngx_rtmp_play_filter(rs, &v);
}


ngx_int_t
ngx_live_relay_publish_local(ngx_rtmp_session_t *rs)
{
    ngx_rtmp_publish_t          v;

    ngx_memzero(&v, sizeof(ngx_rtmp_publish_t));
    v.silent = 1;
    *(ngx_cpymem(v.name, rs->name.data, ngx_min(sizeof(v.name) - 1,
                 rs->name.len))) = 0;
    if (rs->pargs.len) {
        *(ngx_cpymem(v.args, rs->pargs.data, ngx_min(sizeof(v.args) - 1,
                     rs->pargs.len))) = 0;
    }

    return ngx_rtmp_publish_filter(rs, &v);
}


static ngx_int_t
ngx_live_relay_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_live_relay_ctx_t       *ctx;

    // second publish will not trigger push
    if (s->live_stream->publish_ctx->next != NULL) {
        goto next;
    }

    if (ngx_live_push(s) != NGX_OK) {
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->reconnect.timer_set) {
        ngx_del_timer(&ctx->reconnect);
    }

    if (ctx->reconnect.posted) {
        ngx_delete_posted_event(&ctx->reconnect);
    }

    ctx->successd = 1;
    ctx->failed_reconnect = 0;

    ctx->idx = 0;

next:
    return next_publish(s, v);
}


static ngx_int_t
ngx_live_relay_play(ngx_rtmp_session_t *s, ngx_rtmp_play_t *v)
{
    ngx_live_relay_ctx_t       *ctx;

    if (ngx_live_pull(s) != NGX_OK) {
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->reconnect.timer_set) {
        ngx_del_timer(&ctx->reconnect);
    }

    if (ctx->reconnect.posted) {
        ngx_delete_posted_event(&ctx->reconnect);
    }

    ctx->successd = 1;
    ctx->failed_reconnect = 0;

    ctx->idx = 0;

next:

    return next_play(s, v);
}


static ngx_int_t
ngx_live_relay_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_rtmp_core_ctx_t        *ctx;
    ngx_live_relay_ctx_t       *rctx;

    if (s->live_stream == NULL) {
        goto next;
    }

    if (s->publishing) {
        /*
         * normal publisher close or relay puller close
         * need to trigger ngx_live_pull_close
         */
        ngx_live_pull_close(s);

        // all publishers close, close push (play + relay)
        if (s->live_stream->publish_ctx == NULL) {
            for (ctx = s->live_stream->play_ctx; ctx; ctx = ctx->next) {
                if (ctx->session->relay) {
                    ctx->session->finalize_reason = NGX_LIVE_RELAY_CLOSE;
                    ngx_rtmp_finalize_session(ctx->session);
                }
            }
        }
    } else {
        rctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
        if (rctx) {
            // push close will trigger relay push reconnect in relay modules
            ngx_live_push_close(s);
        }

        // all players close, close pull (publish + relay)
        if (s->live_stream->play_ctx == NULL) {
            for (ctx = s->live_stream->publish_ctx; ctx; ctx = ctx->next) {
                if (ctx->session->relay) {
                    ctx->session->finalize_reason = NGX_LIVE_RELAY_CLOSE;
                    ngx_rtmp_finalize_session(ctx->session);
                }
            }
        }
    }

next:
    return next_close_stream(s, v);
}


static ngx_int_t
ngx_live_relay_push(ngx_rtmp_session_t *s)
{
    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_pull(ngx_rtmp_session_t *s)
{
    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_push_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    if (ctx && ctx->reconnect.timer_set) {
        ngx_del_timer(&ctx->reconnect);
    }

    if (ctx && ctx->reconnect.posted) {
        ngx_delete_posted_event(&ctx->reconnect);
    }

    return NGX_OK;
}


/*
 * pull reconnect need to trigger ngx_live_pull chain
 */
static ngx_int_t
ngx_live_relay_pull_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;
    ngx_flag_t                  has_player;
    ngx_rtmp_core_ctx_t        *cctx;;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    has_player = 0;

    if (s->static_pull == 0) {
        for (cctx = s->live_stream->play_ctx; cctx; cctx = cctx->next) {
            if (cctx->session->relay == 0) { // has pure player, not relay push
                has_player = 1;
                break;
            }
        }
    }

    if (has_player || s->static_pull) { // has player in stream
        // ctx is NULL, s is a normal publisher
        // ctx is not NULL, s is a puller, if giveup flag set
        //      no need to create pull reconnect
        if (ctx == NULL || !ctx->giveup) {
            ngx_live_pull(s);
        }
    }

    if (ctx && ctx->reconnect.timer_set) {
        ngx_del_timer(&ctx->reconnect);
    }

    if (ctx && ctx->reconnect.posted) {
        ngx_delete_posted_event(&ctx->reconnect);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_live_relay_postconfiguration(ngx_conf_t *cf)
{
    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_live_relay_publish;

    next_play = ngx_rtmp_play;
    ngx_rtmp_play = ngx_live_relay_play;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_live_relay_close_stream;

    ngx_live_pull = ngx_live_relay_pull;
    ngx_live_push = ngx_live_relay_push;
    ngx_live_pull_close = ngx_live_relay_pull_close;
    ngx_live_push_close = ngx_live_relay_push_close;

    return NGX_OK;
}
