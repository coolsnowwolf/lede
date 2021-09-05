/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_codec_module.h"
#include "ngx_rtmp_live_module.h"


static ngx_rtmp_close_stream_pt         next_close_stream;


static void *ngx_rtmp_gop_create_app_conf(ngx_conf_t *cf);
static char *ngx_rtmp_gop_merge_app_conf(ngx_conf_t *cf, void *parent,
       void *child);

static ngx_int_t ngx_rtmp_gop_postconfiguration(ngx_conf_t *cf);

#define ngx_rtmp_gop_next(s, pos) ((pos + 1) % s->out_queue)
#define ngx_rtmp_gop_prev(s, pos) (pos == 0 ? s->out_queue - 1 : pos - 1)

typedef struct {
    /* publisher: head of cache
     * player: cache send position of publisher's out
     */
    size_t                      gop_pos;
    /* tail of cache */
    size_t                      gop_last;
    /* 0 for not send, 1 for sending, 2 for sent */
    ngx_flag_t                  send_gop;

    ngx_rtmp_frame_t           *keyframe;

    ngx_rtmp_frame_t           *aac_header;
    ngx_rtmp_frame_t           *avc_header;

    ngx_rtmp_frame_t           *latest_aac_header;
    ngx_rtmp_frame_t           *latest_avc_header;

    ngx_uint_t                  meta_version;

    uint32_t                    first_timestamp;

    /* only for publisher, must at last of ngx_rtmp_gop_ctx_t */
    ngx_rtmp_frame_t           *cache[];
} ngx_rtmp_gop_ctx_t;

typedef struct {
    ngx_msec_t                  cache_time;
    ngx_flag_t                  low_latency;
    ngx_flag_t                  send_all;
    ngx_msec_t                  fix_timestamp;
    ngx_flag_t                  zero_start;
} ngx_rtmp_gop_app_conf_t;


static ngx_command_t  ngx_rtmp_gop_commands[] = {

    { ngx_string("cache_time"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_gop_app_conf_t, cache_time),
      NULL },

    { ngx_string("low_latency"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_gop_app_conf_t, low_latency),
      NULL },

    { ngx_string("send_all"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_gop_app_conf_t, send_all),
      NULL },

    { ngx_string("fix_timestamp"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_gop_app_conf_t, fix_timestamp),
      NULL },

    { ngx_string("zero_start"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_gop_app_conf_t, zero_start),
      NULL },
      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_gop_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_gop_postconfiguration,         /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_rtmp_gop_create_app_conf,           /* create app configuration */
    ngx_rtmp_gop_merge_app_conf             /* merge app configuration */
};


ngx_module_t  ngx_rtmp_gop_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_gop_module_ctx,               /* module context */
    ngx_rtmp_gop_commands,                  /* module directives */
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
ngx_rtmp_gop_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_gop_app_conf_t    *gacf;

    gacf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_gop_app_conf_t));
    if (gacf == NULL) {
        return NULL;
    }

    gacf->cache_time = NGX_CONF_UNSET_MSEC;
    gacf->low_latency = NGX_CONF_UNSET;
    gacf->send_all = NGX_CONF_UNSET;
    gacf->fix_timestamp = NGX_CONF_UNSET_MSEC;
    gacf->zero_start = NGX_CONF_UNSET;

    return gacf;
}

static char *
ngx_rtmp_gop_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_gop_app_conf_t    *prev = parent;
    ngx_rtmp_gop_app_conf_t    *conf = child;

    ngx_conf_merge_msec_value(conf->cache_time, prev->cache_time, 0);
    ngx_conf_merge_value(conf->low_latency, prev->low_latency, 0);
    ngx_conf_merge_value(conf->send_all, prev->send_all, 0);
    ngx_conf_merge_msec_value(conf->fix_timestamp, prev->fix_timestamp, 10000);
    ngx_conf_merge_value(conf->zero_start, prev->zero_start, 0);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_rtmp_gop_link_frame(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *frame)
{
    ngx_uint_t                      nmsg;
    ngx_rtmp_live_chunk_stream_t   *cs;
    ngx_uint_t                      csidx;
    ngx_rtmp_live_ctx_t            *lctx;
    ngx_rtmp_gop_ctx_t             *ctx;
    uint32_t                        delta;
    ngx_rtmp_gop_app_conf_t        *gacf;

    if (frame == NULL) {
        return NGX_OK;
    }

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_gop_module);
    if (gacf->fix_timestamp) {
        lctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_live_module);
        csidx = !(frame->hdr.type == NGX_RTMP_MSG_VIDEO);

        cs  = &lctx->cs[csidx];

        delta = frame->hdr.timestamp > cs->last_timestamp ?
            frame->hdr.timestamp - cs->last_timestamp :
            cs->last_timestamp - frame->hdr.timestamp;

        if (delta > gacf->fix_timestamp) {
            delta = 0;
        }

        if (!gacf->zero_start && cs->timestamp == 0) {
            cs->timestamp = frame->hdr.timestamp;
        } else if (frame->hdr.timestamp > cs->last_timestamp) {
            cs->timestamp += delta;
        } else if (cs->timestamp >= delta) {
            cs->timestamp -= delta;
        }

        cs->last_timestamp = frame->hdr.timestamp;

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "gop: link_frame| type %d, delta %d,"
            " timestamp %uD, fixed timestamp %uD",
            frame->hdr.type, delta, frame->hdr.timestamp, cs->timestamp);

        frame->hdr.timestamp = cs->timestamp;
        if (frame->hdr.type == NGX_RTMP_MSG_AMF_META) {
            frame->hdr.timestamp = 0;
        }
    }

    nmsg = (s->out_last - s->out_pos) % s->out_queue + 1;

    if (nmsg >= s->out_queue) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "link frame nmsg(%ui) >= out_queue(%O)", nmsg, s->out_queue);
        return NGX_AGAIN;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_gop_module);

    if (frame->hdr.type == NGX_RTMP_MSG_AUDIO && frame->av_header) {
        ctx->latest_aac_header = frame;
    }

    if (frame->hdr.type == NGX_RTMP_MSG_VIDEO && frame->av_header) {
        ctx->latest_avc_header = frame;
    }

    s->out[s->out_last] = frame;
    s->out_last = ngx_rtmp_gop_next(s, s->out_last);

    ngx_rtmp_shared_acquire_frame(frame);

    return NGX_OK;
}

static void
ngx_rtmp_gop_set_avframe_tag(ngx_rtmp_frame_t *frame)
{
    ngx_chain_t                *cl;

    if (frame->hdr.type != NGX_RTMP_MSG_AUDIO &&
            frame->hdr.type != NGX_RTMP_MSG_VIDEO)
    {
        return;
    }

    cl = frame->chain;

    frame->av_header = ngx_rtmp_is_codec_header(cl);
    frame->keyframe = (frame->hdr.type == NGX_RTMP_MSG_VIDEO)
            ? (ngx_rtmp_get_video_frame_type(cl) == NGX_RTMP_VIDEO_KEY_FRAME)
            : 0;

    if (frame->av_header) {
        frame->mandatory = 1;
    }
}

static void
ngx_rtmp_gop_reset_avheader(ngx_rtmp_gop_ctx_t *ctx,
        ngx_rtmp_frame_t *frame)
{
    if (frame->hdr.type == NGX_RTMP_MSG_AUDIO) {
        if (ctx->aac_header) {
            ngx_rtmp_shared_free_frame(ctx->aac_header);
        }
        ctx->aac_header = frame;
    } else {
        if (ctx->avc_header) {
            ngx_rtmp_shared_free_frame(ctx->avc_header);
        }
        ctx->avc_header = frame;
    }
}

static void
ngx_rtmp_gop_reset_gop(ngx_rtmp_session_t *s, ngx_rtmp_gop_ctx_t *ctx,
        ngx_rtmp_frame_t *frame)
{
    ngx_rtmp_gop_app_conf_t    *gacf;
    ngx_rtmp_frame_t           *f, *next_keyframe;
    size_t                      pos;
    ngx_uint_t                  nmsg;

    /* reset av_header at the front of cache */
    for (pos = ctx->gop_pos; pos != ctx->gop_last;
            pos = ngx_rtmp_gop_next(s, pos))
    {
        if (ctx->cache[pos]->av_header) {
            ngx_rtmp_gop_reset_avheader(ctx, ctx->cache[pos]);
            ctx->gop_pos = ngx_rtmp_gop_next(s, ctx->gop_pos);
            continue;
        }

        break;
    }

    f = ctx->cache[pos];
    if (f == NULL) {
        return;
    }

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_gop_module);

    /* only audio in cache */
    if (ctx->keyframe == NULL) {
        if (frame->hdr.timestamp - ctx->cache[ctx->gop_pos]->hdr.timestamp
                > gacf->cache_time)
        {
            ngx_rtmp_shared_free_frame(f);
            ctx->cache[ctx->gop_pos] = NULL;
            ctx->gop_pos = ngx_rtmp_gop_next(s, ctx->gop_pos);
        }

        return;
    }

    /* only video of video + audio */
    next_keyframe = ctx->keyframe->next;

    /* only one gop in cache */
    if (next_keyframe == NULL) {
        return;
    }

    nmsg = (ctx->gop_last - ctx->gop_pos) % s->out_queue + 2;
    if (nmsg >= s->out_queue) {
        goto reset;
    }

    if (frame->hdr.type == NGX_RTMP_MSG_AUDIO) {
        return;
    }

    if (frame->hdr.type == NGX_RTMP_MSG_VIDEO && frame->hdr.timestamp
            - next_keyframe->hdr.timestamp < gacf->cache_time)
    {
        return;
    }

reset:
    for (pos = ctx->gop_pos; ctx->cache[pos] != next_keyframe;
            pos = ngx_rtmp_gop_next(s, pos))
    {
        f = ctx->cache[pos];

        if (f->av_header) {
            ngx_rtmp_gop_reset_avheader(ctx, f);
        } else {
            ngx_rtmp_shared_free_frame(f);
        }

        ctx->cache[pos] = NULL;
    }

    ctx->keyframe = next_keyframe;
    ctx->gop_pos = pos;
}

static void
ngx_rtmp_gop_print_cache(ngx_rtmp_session_t *s, ngx_rtmp_gop_ctx_t *ctx)
{
#if (NGX_DEBUG)
    ngx_rtmp_frame_t           *frame;
    u_char                      content[10240], *p;
    size_t                      pos;

    ngx_memzero(content, sizeof(content));

    p = content;
    for (pos = ctx->gop_pos; pos != ctx->gop_last;
            pos = ngx_rtmp_gop_next(s, pos))
    {
        frame = ctx->cache[pos];
        switch (frame->hdr.type) {
        case NGX_RTMP_MSG_AUDIO:
            *p++ = 'A';
            break;
        case NGX_RTMP_MSG_VIDEO:
            *p++ = 'V';
            break;
        default:
            *p++ = 'O';
            break;
        }

        if (frame->keyframe) {
            *p++ = 'I';
        }

        if (frame->av_header) {
            *p++ = 'H';
        }

        *p++ = ' ';
    }

    ngx_log_debug5(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "[%z %z] [%p %p] %s", ctx->gop_pos, ctx->gop_last, ctx->aac_header,
            ctx->avc_header, content);
#endif
}

ngx_int_t
ngx_rtmp_gop_cache(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *frame)
{
    ngx_rtmp_gop_app_conf_t    *gacf;
    ngx_rtmp_gop_ctx_t         *ctx;
    ngx_rtmp_frame_t          **keyframe;
    ngx_uint_t                  nmsg;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_gop_module);
    if (gacf->cache_time == 0) {
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_gop_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_gop_ctx_t)
                          + s->out_queue * sizeof(ngx_rtmp_frame_t *));
        if (ctx == NULL) {
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_gop_module);
    }

    nmsg = (ctx->gop_last - ctx->gop_pos) % s->out_queue + 1;
    if (nmsg >= s->out_queue) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "cache frame nmsg(%ui) >= out_queue(%z)", nmsg, s->out_queue);
        return NGX_AGAIN;
    }

    ngx_rtmp_gop_set_avframe_tag(frame);

    ngx_log_debug5(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "cache frame: %ud[%d %d], %ud, %ud",
            frame->hdr.type, frame->keyframe, frame->av_header,
            frame->hdr.timestamp, frame->hdr.mlen);

    if (frame->hdr.type == NGX_RTMP_MSG_AUDIO && frame->av_header) {
        ctx->latest_aac_header = frame;
    }

    if (frame->hdr.type == NGX_RTMP_MSG_VIDEO && frame->av_header) {
        ctx->latest_avc_header = frame;
    }

    /* first video frame is not intra_frame or video header */
    if (ctx->keyframe == NULL && frame->hdr.type == NGX_RTMP_MSG_VIDEO
            && !frame->keyframe && !frame->av_header)
    {
        return NGX_OK;
    }

    /* video intra_frame */
    if (frame->keyframe && !frame->av_header) {
        for (keyframe = &ctx->keyframe; *keyframe;
                keyframe = &((*keyframe)->next));
        *keyframe = frame;
    }

    ctx->cache[ctx->gop_last] = frame;
    ctx->gop_last = ngx_rtmp_gop_next(s, ctx->gop_last);

    ngx_rtmp_shared_acquire_frame(frame);

    ngx_rtmp_gop_reset_gop(s, ctx, frame);

    ngx_rtmp_gop_print_cache(s, ctx);

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_gop_send_meta(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss)
{
    ngx_rtmp_gop_ctx_t         *ssctx;
    ngx_rtmp_codec_ctx_t       *cctx;

    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_rtmp_gop_module);
    cctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    /* meta */
    if (ssctx->meta_version != cctx->meta_version
            && ngx_rtmp_gop_link_frame(ss, cctx->meta) == NGX_AGAIN)
    {
        return NGX_AGAIN;
    }
    ssctx->meta_version = cctx->meta_version;

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_gop_send_gop(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss)
{
    ngx_rtmp_gop_app_conf_t    *gacf;
    ngx_rtmp_gop_ctx_t         *sctx, *ssctx;
    ngx_rtmp_frame_t           *frame;
    size_t                      pos;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_gop_module);

    sctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_gop_module);
    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_rtmp_gop_module);

    /* already send gop */
    if (ssctx->send_gop == 3) {
        return NGX_OK;
    }

    if (ssctx->send_gop == 0) {
        if (ngx_rtmp_gop_send_meta(s, ss) == NGX_AGAIN) {
            return NGX_AGAIN;
        }

        ngx_rtmp_send_message(ss, NULL, 0);

        ssctx->send_gop = 1;

        return NGX_AGAIN;
    }

    /* link frame in s to ss */
    if (ssctx->send_gop == 1) {
        ssctx->gop_pos = sctx->gop_pos;
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            return NGX_AGAIN;
        }

        if (sctx->aac_header) {
            if (ngx_rtmp_gop_link_frame(ss, sctx->aac_header) == NGX_AGAIN) {
                return NGX_AGAIN;
            }
        }

        if (sctx->avc_header) {
            if (ngx_rtmp_gop_link_frame(ss, sctx->avc_header) == NGX_AGAIN) {
                return NGX_AGAIN;
            }
        }

        ssctx->send_gop = 2;
        ssctx->first_timestamp = sctx->cache[ssctx->gop_pos]->hdr.timestamp;
    } else {
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            ssctx->gop_pos = sctx->gop_pos;
        }
    }

    pos = ssctx->gop_pos;
    frame = sctx->cache[pos];
    while (frame) {
        if (!gacf->send_all &&
            frame->hdr.timestamp - ssctx->first_timestamp >= gacf->cache_time)
        {
            ssctx->send_gop = 3;
            break;
        }

        if (ngx_rtmp_gop_link_frame(ss, frame) == NGX_AGAIN) {
            break;
        }

        pos = ngx_rtmp_gop_next(s, pos);
        frame = sctx->cache[pos];
    }

    if (frame == NULL) { /* send all frame in cache */
        ssctx->send_gop = 3;
    }

    ssctx->gop_pos = pos;
    ngx_rtmp_send_message(ss, NULL, 0);

    return NGX_AGAIN;
}

ngx_int_t
ngx_rtmp_gop_send(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss)
{
    ngx_rtmp_gop_app_conf_t    *gacf;
    ngx_rtmp_gop_ctx_t         *sctx, *ssctx;
    ngx_rtmp_frame_t           *frame;
    size_t                      pos;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_gop_module);
    if (gacf->cache_time == 0) {
        return NGX_DECLINED;
    }

    sctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_gop_module);
    if (sctx == NULL) { /* publisher doesn't publish av frame */
        return NGX_DECLINED;
    }

    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_rtmp_gop_module);
    if (ssctx == NULL) {
        ssctx = ngx_pcalloc(ss->pool, sizeof(ngx_rtmp_gop_ctx_t));
        if (ssctx == NULL) {
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(ss, ssctx, ngx_rtmp_gop_module);
    }

    if (ngx_rtmp_gop_send_gop(s, ss) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* send frame by frame */
    if (ngx_rtmp_gop_send_meta(s, ss) == NGX_AGAIN) {
        return NGX_AGAIN;
    }

    pos = ngx_rtmp_gop_prev(s, sctx->gop_last);
    /* new frame is video key frame */
    if (sctx->cache[pos]->keyframe && !sctx->cache[pos]->av_header) {
        if (gacf->low_latency && pos != ssctx->gop_pos) {
            ssctx->gop_pos = pos;

            ss->out_pos = ss->out_last;

            ngx_log_error(NGX_LOG_INFO, ss->log, 0,
                    "gop, low latency, chase to new keyframe");

            if (sctx->latest_aac_header
                    && sctx->latest_aac_header != ssctx->latest_aac_header)
            {
                if (ngx_rtmp_gop_link_frame(ss, sctx->latest_aac_header)
                        == NGX_AGAIN)
                {
                    return NGX_AGAIN;
                }
            }

            if (sctx->latest_avc_header
                    && sctx->latest_avc_header != ssctx->latest_avc_header)
            {
                if (ngx_rtmp_gop_link_frame(ss, sctx->latest_avc_header)
                        == NGX_AGAIN)
                {
                    return NGX_AGAIN;
                }
            }
        }
    } else {
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            ngx_log_error(NGX_LOG_ERR, ss->log, 0,
                    "gop, current gop pos is NULL, "
                    "skip to new postion [pos %d last %d] %d",
                    sctx->gop_pos, sctx->gop_last, ssctx->gop_pos);

            if (sctx->aac_header
                    && sctx->aac_header != ssctx->latest_aac_header)
            {
                if (ngx_rtmp_gop_link_frame(ss, sctx->aac_header)
                        == NGX_AGAIN)
                {
                    return NGX_AGAIN;
                }
            }

            if (sctx->avc_header
                    && sctx->avc_header != ssctx->latest_avc_header)
            {
                if (ngx_rtmp_gop_link_frame(ss, sctx->avc_header)
                        == NGX_AGAIN)
                {
                    return NGX_AGAIN;
                }
            }

            ssctx->gop_pos = sctx->gop_pos;
        }
    }

    frame = sctx->cache[ssctx->gop_pos];
    if (ngx_rtmp_gop_link_frame(ss, frame) == NGX_AGAIN) {
        return NGX_AGAIN;
    }

    ssctx->gop_pos = ngx_rtmp_gop_next(s, ssctx->gop_pos);
    ngx_rtmp_send_message(ss, NULL, 0);

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_gop_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_rtmp_gop_ctx_t         *ctx;
    ngx_rtmp_live_ctx_t        *lctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_gop_module);
    if (ctx == NULL) {
        goto next;
    }

    lctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_live_module);

    if (!lctx->publishing) {
        goto next;
    }

    if (ctx->avc_header) {
        ngx_rtmp_shared_free_frame(ctx->avc_header);
    }

    if (ctx->aac_header) {
        ngx_rtmp_shared_free_frame(ctx->aac_header);
    }

    /* free cache in publisher */
    while (ctx->gop_pos != ctx->gop_last) {
        ngx_rtmp_shared_free_frame(ctx->cache[ctx->gop_pos]);
        ctx->gop_pos = ngx_rtmp_gop_next(s, ctx->gop_pos);
    }

next:
    return next_close_stream(s, v);
}

static ngx_int_t
ngx_rtmp_gop_postconfiguration(ngx_conf_t *cf)
{
    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_rtmp_gop_close_stream;

    return NGX_OK;
}
