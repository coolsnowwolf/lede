/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_rtmp_monitor_module.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_codec_module.h"


static ngx_rtmp_close_stream_pt         next_close_stream;


static void *ngx_rtmp_monitor_create_app_conf(ngx_conf_t *cf);
static char *ngx_rtmp_monitor_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_rtmp_monitor_postconfiguration(ngx_conf_t *cf);

static char *ngx_rtmp_monitor_dump(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_rtmp_monitor_buffered_log(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);


#define NGX_RTMP_MONITOR_BUFFER_SIZE    61


typedef struct {
    ngx_str_t                   dump_path;
    ngx_flag_t                  monitor;
    ngx_log_t                  *buffered_log;
} ngx_rtmp_monitor_app_conf_t;


typedef struct {
    ngx_event_t                 consume;
    ngx_msec_t                  buffered;
    ngx_uint_t                  nbuffered;

    double                      frame_rate;
    double                      nframes;

    double                      buffers[NGX_RTMP_MONITOR_BUFFER_SIZE];
    ngx_uint_t                  fps[NGX_RTMP_MONITOR_BUFFER_SIZE];
    ngx_uint_t                  curr;

    ngx_flag_t                  dump;

    unsigned                    publishing:1;
} ngx_rtmp_monitor_ctx_t;


static ngx_command_t  ngx_rtmp_monitor_commands[] = {

    { ngx_string("dump"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_rtmp_monitor_dump,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("buffered_log"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_monitor_buffered_log,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_monitor_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_monitor_postconfiguration,     /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_rtmp_monitor_create_app_conf,       /* create app configuration */
    ngx_rtmp_monitor_merge_app_conf         /* merge app configuration */
};


ngx_module_t  ngx_rtmp_monitor_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_monitor_module_ctx,           /* module context */
    ngx_rtmp_monitor_commands,              /* module directives */
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


static void
ngx_rtmp_monitor_dump_frame(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
}

static void
ngx_rtmp_monitor_consume(ngx_event_t *ev)
{
    ngx_rtmp_session_t             *s, *ps;
    ngx_rtmp_monitor_ctx_t         *ctx;
    ngx_rtmp_codec_ctx_t           *cctx;
    ngx_rtmp_monitor_app_conf_t    *macf;
    u_char                          peer[NGX_SOCKADDR_STRLEN];
    u_char                          local[NGX_SOCKADDR_STRLEN];
    struct sockaddr                 paddr, laddr;
    socklen_t                       plen, llen;

    s = ev->data;

    macf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_monitor_module);

    if (macf->monitor == 0) {
        return;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_monitor_module);
    if (ctx == NULL) {
        return;
    }

    if (ctx->publishing) {
        ps = s;
    } else {
        if (s->live_stream->publish_ctx) {
            ps = s->live_stream->publish_ctx->session;
        } else {
            goto next;
        }
    }

    cctx = ngx_rtmp_get_module_ctx(ps, ngx_rtmp_codec_module);
    if (cctx == NULL) {
        goto next;
    }

    if (cctx->frame_rate != 0) {
        ctx->frame_rate = cctx->frame_rate;
    }

next:
    if (ctx->frame_rate == 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "monitor, frame rate error, stream: %V, frame rate: %.2f",
                &s->stream, ctx->frame_rate);
        return;
    }

    ctx->nframes -= ctx->frame_rate;

    if (ctx->nframes <= 0) {
        ctx->nframes = 0;
        if (ctx->buffered == 0) {
            ++ctx->nbuffered;
        }
        ++ctx->buffered;

        ngx_memzero(local, sizeof(local));
        ngx_memzero(peer, sizeof(peer));
        plen = sizeof(paddr);
        llen = sizeof(laddr);

        if (s->connection) {
            getpeername(s->connection->fd, &paddr, &plen);
            getsockname(s->connection->fd, &laddr, &llen);

            ngx_sock_ntop(&paddr, plen, peer, NGX_SOCKADDR_STRLEN, 1);
            ngx_sock_ntop(&laddr, llen, local, NGX_SOCKADDR_STRLEN, 1);

            ngx_log_error(NGX_LOG_ERR, macf->buffered_log, 0,
                    "%p %s, peer: %s, local: %s, "
                    "stream: %V, buffered: %ui, time: %uis",
                    s, ctx->publishing ? "publisher" : "player", peer, local,
                    &s->stream, ctx->nbuffered, ctx->buffered);

        }
    } else {
        ctx->buffered = 0;
    }

    ctx->buffers[ctx->curr] = ctx->nframes;

    ctx->curr = (ctx->curr + 1) % NGX_RTMP_MONITOR_BUFFER_SIZE;

    ctx->fps[ctx->curr] = 0;

    ngx_add_timer(&ctx->consume, 1000);
}

static ngx_int_t
ngx_rtmp_monitor_av(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_rtmp_monitor_frame(s, h, in, ngx_rtmp_is_codec_header(in), 1);

//    if (h->type == NGX_RTMP_MSG_VIDEO) {
//        ngx_log_error(NGX_LOG_INFO, s->log, 0,
//                "!!!!!!video(%d): %02xD %02xD %02xD %02xD %02xD",
//                h->mlen, in->buf->pos[0], in->buf->pos[1], in->buf->pos[2],
//                in->buf->pos[3], in->buf->pos[4], in->buf->pos[5]);
//    }
//
//    if (h->type == NGX_RTMP_MSG_AUDIO) {
//        ngx_log_error(NGX_LOG_INFO, s->log, 0,
//                "!!!!!!audio(%d): %02xD %02xD %02xD %02xD %02xD",
//                h->mlen, in->buf->pos[0], in->buf->pos[1], in->buf->pos[2],
//                in->buf->pos[3], in->buf->pos[4], in->buf->pos[5]);
//    }

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_monitor_meta_data(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    if (s->first_metadata == 0) {
        s->stage = NGX_LIVE_AV;
        s->first_metadata = ngx_current_msec;
        s->first_data = s->first_data == 0? ngx_current_msec: s->first_data;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "receive metadata");

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_monitor_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_rtmp_monitor_ctx_t     *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_monitor_module);

    if (ctx == NULL) {
        goto next;
    }

    if (ctx->consume.timer_set) {
        ngx_del_timer(&ctx->consume);
    }

    if (ctx->consume.posted) {
        ngx_delete_posted_event(&ctx->consume);
    }

next:
    return next_close_stream(s, v);
}

void
ngx_rtmp_monitor_frame(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in, ngx_flag_t is_header, ngx_flag_t publishing)
{
    ngx_rtmp_monitor_app_conf_t    *macf;
    ngx_rtmp_monitor_ctx_t         *ctx;

    if (s->first_metadata == 0 && (h->type == NGX_RTMP_MSG_AMF_META
                               || h->type == NGX_RTMP_MSG_AMF3_META))
    {
        s->stage = NGX_LIVE_AV;
        s->first_metadata = ngx_current_msec;
        s->first_data = s->first_data == 0? ngx_current_msec: s->first_data;
    }

    if (s->first_audio == 0 && h->type == NGX_RTMP_MSG_AUDIO) {
        s->stage = NGX_LIVE_AV;
        s->first_audio = ngx_current_msec;
        s->first_data = s->first_data == 0? ngx_current_msec: s->first_data;
    }

    if (s->first_video == 0 && h->type == NGX_RTMP_MSG_VIDEO) {
        s->stage = NGX_LIVE_AV;
        s->first_video = ngx_current_msec;
        s->first_data = s->first_data == 0? ngx_current_msec: s->first_data;
    }

    if (h->type == NGX_RTMP_MSG_AUDIO && is_header) {
        if (s->publishing) {
            ngx_log_error(NGX_LOG_INFO, s->log, 0, "receive audio header");
        } else {
            ngx_log_error(NGX_LOG_INFO, s->log, 0, "send audio header");
        }
    }

    if (h->type == NGX_RTMP_MSG_VIDEO && is_header) {
        if (s->publishing) {
            ngx_log_error(NGX_LOG_INFO, s->log, 0, "receive video header");
        } else {
            ngx_log_error(NGX_LOG_INFO, s->log, 0, "send video header");
        }
    }

    if (h->type != NGX_RTMP_MSG_VIDEO) {
        return;
    }

    macf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_monitor_module);

    if (macf->monitor == 0) {
        return;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_monitor_module);

    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_monitor_ctx_t));
        if (ctx == NULL) {
            return;
        }
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_monitor_module);

        ctx->dump = macf->dump_path.len > 0;
        ctx->publishing = publishing;

        ctx->consume.data = s;
        ctx->consume.log = s->log;
        ctx->consume.handler = ngx_rtmp_monitor_consume;
        ngx_add_timer(&ctx->consume, 1000);
    }

    if (publishing && ctx->dump) {
        ngx_rtmp_monitor_dump_frame(s, h, in);
    }

    if (is_header) {
        return;
    }

    ++ctx->nframes;
    ++ctx->fps[ctx->curr];
}


static void *
ngx_rtmp_monitor_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_monitor_app_conf_t	   *macf;

    macf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_monitor_app_conf_t));
    if (macf == NULL) {
        return NULL;
    }

    macf->monitor = NGX_CONF_UNSET;

    return macf;
}

static char *
ngx_rtmp_monitor_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_monitor_app_conf_t    *prev = parent;
    ngx_rtmp_monitor_app_conf_t    *conf = child;

    if (conf->dump_path.data == NULL) {
        conf->dump_path = prev->dump_path;
    }

    ngx_conf_merge_value(conf->monitor, prev->monitor, 0);

    if (conf->buffered_log == NULL) {
        if (prev->buffered_log) {
            conf->buffered_log = prev->buffered_log;
        } else {
            conf->buffered_log = &cf->cycle->new_log;
        }
    }

    return NGX_CONF_OK;
}

static char *
ngx_rtmp_monitor_dump(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_monitor_app_conf_t    *macf;
    ngx_str_t                      *value;

    macf = conf;

    if (macf->dump_path.len) {
        return "is duplicate";
    }

    value = cf->args->elts;

    macf->dump_path = value[1];

    if (macf->dump_path.len > 0 &&
            macf->dump_path.data[macf->dump_path.len - 1] == '/')
    {
        --macf->dump_path.len;
    }

    if (ngx_conf_full_name(cf->cycle, &macf->dump_path, 0) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static char *
ngx_rtmp_monitor_buffered_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_monitor_app_conf_t    *macf;

    macf = conf;

    macf->monitor = 1;

    return ngx_log_set_log(cf, &macf->buffered_log);
}

static ngx_int_t
ngx_rtmp_monitor_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t          *cmcf;
    ngx_rtmp_handler_pt                *h;
    ngx_rtmp_amf_handler_t             *ch;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_rtmp_monitor_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_rtmp_monitor_av;

    /* register metadata handler */
    ch = ngx_array_push(&cmcf->amf);
    if (ch == NULL) {
        return NGX_ERROR;
    }
    ngx_str_set(&ch->name, "@setDataFrame");
    ch->handler = ngx_rtmp_monitor_meta_data;

    ch = ngx_array_push(&cmcf->amf);
    if (ch == NULL) {
        return NGX_ERROR;
    }
    ngx_str_set(&ch->name, "onMetaData");
    ch->handler = ngx_rtmp_monitor_meta_data;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_rtmp_monitor_close_stream;

    return NGX_OK;
}
