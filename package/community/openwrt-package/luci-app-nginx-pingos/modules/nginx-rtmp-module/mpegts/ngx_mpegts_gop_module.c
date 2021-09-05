
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_codec_module.h"
#include "ngx_rtmp_live_module.h"
#include "ngx_mpegts_live_module.h"


static ngx_rtmp_close_stream_pt         next_close_stream;

static ngx_mpegts_video_pt next_mpegts_video;
static ngx_mpegts_audio_pt next_mpegts_audio;

static void *ngx_mpegts_gop_create_app_conf(ngx_conf_t *cf);
static char *ngx_mpegts_gop_merge_app_conf(ngx_conf_t *cf, void *parent,
       void *child);

static ngx_int_t ngx_mpegts_gop_postconfiguration(ngx_conf_t *cf);

#define ngx_mpegts_gop_next(s, pos) ((pos + 1) % s->out_queue)
#define ngx_mpegts_gop_prev(s, pos) (pos == 0 ? s->out_queue - 1 : pos - 1)

typedef struct {
    /* publisher: head of cache
     * player: cache send position of publisher's out
     */
    size_t                      gop_pos;
    /* tail of cache */
    size_t                      gop_last;
    /* 0 for not send, 1 for sending, 2 for sent */
    ngx_flag_t                  send_gop;

    ngx_mpegts_frame_t         *keyframe;

    ngx_uint_t                  meta_version;

    uint64_t                    first_timestamp;
    uint64_t                    current_timestamp;

    ngx_uint_t                  base_type;

    /* only for publisher, must at last of ngx_mpegts_gop_ctx_t */
    ngx_mpegts_frame_t         *cache[];
} ngx_mpegts_gop_ctx_t;

typedef struct {
    ngx_msec_t                  cache_time;
    ngx_msec_t                  roll_back;
    ngx_msec_t                  one_off_send;
    ngx_flag_t                  low_latency;
    ngx_flag_t                  send_all;
    ngx_msec_t                  fix_timestamp;
    ngx_flag_t                  zero_start;
} ngx_mpegts_gop_app_conf_t;


static ngx_command_t  ngx_mpegts_gop_commands[] = {

    { ngx_string("mpegts_cache_time"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, cache_time),
      NULL },

    { ngx_string("mpegts_roll_back"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, roll_back),
      NULL },

    { ngx_string("mpegts_one_off_send"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, one_off_send),
      NULL },

    { ngx_string("mpegts_low_latency"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, low_latency),
      NULL },

    { ngx_string("mpegts_send_all"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, send_all),
      NULL },

    { ngx_string("mpegts_fix_timestamp"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, fix_timestamp),
      NULL },

    { ngx_string("mpegts_zero_start"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_gop_app_conf_t, zero_start),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_mpegts_gop_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_mpegts_gop_postconfiguration,       /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_mpegts_gop_create_app_conf,         /* create app configuration */
    ngx_mpegts_gop_merge_app_conf           /* merge app configuration */
};


ngx_module_t  ngx_mpegts_gop_module = {
    NGX_MODULE_V1,
    &ngx_mpegts_gop_module_ctx,             /* module context */
    ngx_mpegts_gop_commands,                /* module directives */
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
ngx_mpegts_gop_create_app_conf(ngx_conf_t *cf)
{
    ngx_mpegts_gop_app_conf_t    *gacf;

    gacf = ngx_pcalloc(cf->pool, sizeof(ngx_mpegts_gop_app_conf_t));
    if (gacf == NULL) {
        return NULL;
    }

    gacf->cache_time = NGX_CONF_UNSET_MSEC;
    gacf->roll_back = NGX_CONF_UNSET_MSEC;
    gacf->one_off_send = NGX_CONF_UNSET_MSEC;
    gacf->low_latency = NGX_CONF_UNSET;
    gacf->send_all = NGX_CONF_UNSET;
    gacf->fix_timestamp = NGX_CONF_UNSET_MSEC;
    gacf->zero_start = NGX_CONF_UNSET;

    return gacf;
}

static char *
ngx_mpegts_gop_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_mpegts_gop_app_conf_t    *prev = parent;
    ngx_mpegts_gop_app_conf_t    *conf = child;

    ngx_conf_merge_msec_value(conf->cache_time, prev->cache_time, 0);
    ngx_conf_merge_msec_value(conf->roll_back, prev->roll_back, conf->cache_time);
    ngx_conf_merge_msec_value(conf->one_off_send, prev->one_off_send, 3000);
    ngx_conf_merge_value(conf->low_latency, prev->low_latency, 0);
    ngx_conf_merge_value(conf->send_all, prev->send_all, 0);
    ngx_conf_merge_msec_value(conf->fix_timestamp, prev->fix_timestamp, 10000);
    ngx_conf_merge_value(conf->zero_start, prev->zero_start, 0);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_mpegts_gop_link_frame(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    ngx_uint_t              nmsg;

    if (frame == NULL) {
        return NGX_OK;
    }

    nmsg = (s->out_last - s->out_pos) % s->out_queue + 1;

    if (nmsg >= s->out_queue) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "link frame nmsg(%ui) >= out_queue(%O)", nmsg, s->out_queue);
        return NGX_AGAIN;
    }

#if 0
    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
        "link frame pos[%O] last[%O], pts[%ud]",
        s->out_pos, s->out_last, frame->pts);
#endif

    s->mpegts_out[s->out_last] = frame;
    s->out_last = ngx_mpegts_gop_next(s, s->out_last);

    ngx_rtmp_shared_acquire_mpegts_frame(frame);

    return NGX_OK;
}


static void
ngx_mpegts_gop_reset_gop(ngx_rtmp_session_t *s, ngx_mpegts_gop_ctx_t *ctx,
        ngx_mpegts_frame_t *frame)
{
    ngx_mpegts_gop_app_conf_t   *gacf;
    ngx_mpegts_frame_t          *f, *next_keyframe;
    size_t                       pos;
    ngx_uint_t                   nmsg;
    ngx_msec_t                   cache_time;

    pos = ctx->gop_pos;

    f = ctx->cache[pos];
    if (f == NULL) {
        return;
    }

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_gop_module);

    cache_time = gacf->cache_time > gacf->roll_back ?
        gacf->cache_time : gacf->roll_back;

    /* only audio in cache */
    if (ctx->keyframe == NULL) {
        if (frame->pts - ctx->cache[ctx->gop_pos]->pts > cache_time * 90) {
            ngx_rtmp_shared_free_mpegts_frame(f);
            ctx->cache[ctx->gop_pos] = NULL;
            ctx->gop_pos = ngx_mpegts_gop_next(s, ctx->gop_pos);
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

    if (frame->type == NGX_MPEGTS_MSG_AUDIO) {
        return;
    }

    if (frame->type == NGX_MPEGTS_MSG_VIDEO && frame->pts
            - next_keyframe->pts < cache_time * 90)
    {
        return;
    }

reset:
    for (pos = ctx->gop_pos; ctx->cache[pos] != next_keyframe;
            pos = ngx_mpegts_gop_next(s, pos))
    {
        f = ctx->cache[pos];

        ngx_rtmp_shared_free_mpegts_frame(f);

        ctx->cache[pos] = NULL;
    }

    ctx->keyframe = next_keyframe;
    ctx->gop_pos = pos;
}

static void
ngx_mpegts_gop_print_cache(ngx_rtmp_session_t *s, ngx_mpegts_gop_ctx_t *ctx)
{
#if (NGX_DEBUG)
    ngx_mpegts_frame_t         *frame;
    u_char                      content[10240], *p;
    size_t                      pos;

    ngx_memzero(content, sizeof(content));

    p = content;
    for (pos = ctx->gop_pos; pos != ctx->gop_last;
            pos = ngx_mpegts_gop_next(s, pos))
    {
        frame = ctx->cache[pos];
        switch (frame->type) {
        case NGX_MPEGTS_MSG_AUDIO:
            *p++ = 'A';
            break;
        case NGX_MPEGTS_MSG_VIDEO:
            *p++ = 'V';
            break;
        default:
            *p++ = 'O';
            break;
        }

        if (frame->key) {
            *p++ = 'I';
        }

        *p++ = ' ';
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "[%z %z] %s", ctx->gop_pos, ctx->gop_last, content);
#endif
}

ngx_int_t
ngx_mpegts_gop_cache(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    ngx_mpegts_gop_app_conf_t    *gacf;
    ngx_mpegts_gop_ctx_t         *ctx;
    ngx_mpegts_frame_t          **keyframe;
    ngx_uint_t                    nmsg;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_gop_module);
    if (gacf->cache_time == 0) {
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_gop_ctx_t)
                          + s->out_queue * sizeof(ngx_mpegts_frame_t *));
        if (ctx == NULL) {
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(s, ctx, ngx_mpegts_gop_module);
    }

    nmsg = (ctx->gop_last - ctx->gop_pos) % s->out_queue + 1;
    if (nmsg >= s->out_queue) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "cache frame nmsg(%ui) >= out_queue(%z)", nmsg, s->out_queue);
        return NGX_AGAIN;
    }
#if 0
    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "cache frame: %ud[%d], %ud, %ud",
            frame->type, frame->key, frame->pts, frame->length);
#endif
    /* first video frame is not intra_frame */
    if (ctx->keyframe == NULL && frame->type == NGX_MPEGTS_MSG_VIDEO
            && !frame->key)
    {
        return NGX_OK;
    }

    /* video intra_frame */
    if (frame->key) {
        for (keyframe = &ctx->keyframe; *keyframe;
                keyframe = &((*keyframe)->next));
        *keyframe = frame;
    }

    frame->pos = ctx->gop_last;
    ctx->current_timestamp = frame->pts;
    ctx->cache[ctx->gop_last] = frame;
    ctx->gop_last = ngx_mpegts_gop_next(s, ctx->gop_last);

    ngx_rtmp_shared_acquire_mpegts_frame(frame);

    ngx_mpegts_gop_reset_gop(s, ctx, frame);

    ngx_mpegts_gop_print_cache(s, ctx);

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_gop_send_gop(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss)
{
    ngx_mpegts_gop_app_conf_t    *gacf;
    ngx_mpegts_gop_ctx_t         *sctx, *ssctx;
    ngx_mpegts_frame_t           *frame, *keyframe;
    size_t                        pos;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_gop_module);

    sctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_mpegts_gop_module);

    /* already send gop */
    if (ssctx->send_gop == 2) {
        return NGX_OK;
    }

    //*****************************************************************
    //
    // TODO: send pat pmt
    //
    //*****************************************************************

    /* link frame in s to ss */
    if (ssctx->send_gop == 0) {
        ssctx->gop_pos = sctx->gop_pos;
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            return NGX_AGAIN;
        }

        ssctx->send_gop = 1;
        ssctx->first_timestamp = sctx->cache[ssctx->gop_pos]->pts;
        ssctx->base_type = sctx->cache[ssctx->gop_pos]->type;
    } else {
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            ssctx->gop_pos = sctx->gop_pos;
        }
    }

    frame = NULL;

    keyframe = sctx->keyframe;
    while (ss->roll_back && keyframe &&
        ((sctx->current_timestamp - keyframe->pts) > ss->roll_back * 90))
    {
        ngx_log_error(NGX_LOG_INFO, s->log, 0,
            "rtmp-gop: send_gop| curr %D - k %D, %D",
            sctx->current_timestamp, keyframe->pts, ss->roll_back * 90);
        frame = keyframe;
        keyframe = keyframe->next;
    }

    if (frame == NULL) {
        pos = ssctx->gop_pos;
        frame = sctx->cache[pos];
    } else {
        pos = frame->pos;
    }

    while (frame) {

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "send gop link %D, type %d, curr %D",
            frame->pts/90, frame->type, sctx->current_timestamp/90);
        if (ngx_mpegts_gop_link_frame(ss, frame) == NGX_AGAIN) {
            break;
        }

        if (!gacf->send_all && frame->type == ssctx->base_type &&
            frame->pts - ssctx->first_timestamp >= gacf->one_off_send * 90)
        {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                "gone %D, type %d, first %D, curr %D, send %D",
                frame->pts/90, frame->type, ssctx->first_timestamp/90,
                sctx->current_timestamp/90, gacf->one_off_send);

            ssctx->send_gop = 2;
            pos = ngx_mpegts_gop_next(s, pos);
            break;
        }

        pos = ngx_mpegts_gop_next(s, pos);
        frame = sctx->cache[pos];
    }

    if (frame == NULL) { /* send all frame in cache */
        ssctx->send_gop = 2;
    }

    ssctx->gop_pos = pos;
    ngx_rtmp_send_message(ss, NULL, 0);

    return NGX_AGAIN;
}

ngx_int_t
ngx_mpegts_gop_send(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss)
{
    ngx_mpegts_gop_app_conf_t   *gacf;
    ngx_mpegts_gop_ctx_t        *sctx, *ssctx;
    ngx_mpegts_frame_t          *frame;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_gop_module);
    if (gacf->cache_time == 0) {
        return NGX_DECLINED;
    }

    sctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    if (sctx == NULL) { /* publisher doesn't publish av frame */
        return NGX_DECLINED;
    }

    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_mpegts_gop_module);
    if (ssctx == NULL) {
        ssctx = ngx_pcalloc(ss->pool, sizeof(ngx_mpegts_gop_ctx_t));
        if (ssctx == NULL) {
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(ss, ssctx, ngx_mpegts_gop_module);
    }

    if (ngx_mpegts_gop_send_gop(s, ss) == NGX_AGAIN) {
        return NGX_OK;
    }

    if (sctx->cache[ssctx->gop_pos] == NULL) {
        ngx_log_error(NGX_LOG_ERR, ss->log, 0,
                "mpegts-gop: gop_send| current gop pos is NULL, "
                "skip to new postion [pos %d last %d] %d",
                sctx->gop_pos, sctx->gop_last, ssctx->gop_pos);

        ssctx->gop_pos = sctx->gop_pos;
    }

    frame = sctx->cache[ssctx->gop_pos];
    if (ngx_mpegts_gop_link_frame(ss, frame) == NGX_AGAIN) {
        return NGX_AGAIN;
    }

    ssctx->gop_pos = ngx_mpegts_gop_next(s, ssctx->gop_pos);
    ngx_rtmp_send_message(ss, NULL, 0);

    return NGX_OK;
}

static ngx_int_t
ngx_mpegts_gop_offset_frames(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss,
    ngx_msec_t time_offset, ngx_msec_t duration)
{
    ngx_mpegts_gop_ctx_t         *sctx, *ssctx;
    ngx_mpegts_frame_t           *frame, *keyframe;
    size_t                        pos;

    sctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_mpegts_gop_module);

    /* already send gop */
    if (ssctx->send_gop == 2) {
        return NGX_OK;
    }

    //*****************************************************************
    //
    // TODO: send pat pmt
    //
    //*****************************************************************

    /* link frame in s to ss */
    if (ssctx->send_gop == 0) {
        ssctx->gop_pos = sctx->gop_pos;
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            return NGX_AGAIN;
        }

        ssctx->send_gop = 1;
        ssctx->first_timestamp = sctx->cache[ssctx->gop_pos]->pts;
        ssctx->base_type = sctx->cache[ssctx->gop_pos]->type;
    } else {
        if (sctx->cache[ssctx->gop_pos] == NULL) {
            ssctx->gop_pos = sctx->gop_pos;
        }
    }

    frame = NULL;

    keyframe = sctx->keyframe;
    while (time_offset && keyframe &&
        ((sctx->current_timestamp - keyframe->pts) > time_offset * 90))
    {
        ngx_log_error(NGX_LOG_DEBUG, ss->log, 0,
            "mpegts-gop: offset_frames| curr %D - k %D = %D, %D",
            sctx->current_timestamp, keyframe->pts,
            (sctx->current_timestamp - keyframe->pts) / 90, time_offset);
        frame = keyframe;
        keyframe = keyframe->next;
        ssctx->first_timestamp = frame->pts;
        ssctx->base_type = frame->type;
    }

    if (frame == NULL) {
        pos = ssctx->gop_pos;
        frame = sctx->cache[pos];
    } else {
        pos = frame->pos;
    }

    while (frame) {

        if (ngx_mpegts_gop_link_frame(ss, frame) == NGX_AGAIN) {
            break;
        }

        if (frame->type == ssctx->base_type &&
            frame->pts - ssctx->first_timestamp >= duration * 90)
        {
            ngx_log_error(NGX_LOG_INFO, ss->log, 0, "gone %D, first %D, curr %D",
                frame->pts, ssctx->first_timestamp, sctx->current_timestamp);
            ssctx->send_gop = 2;
            pos = ngx_mpegts_gop_next(s, pos);
            break;
        }

        pos = ngx_mpegts_gop_next(s, pos);
        frame = sctx->cache[pos];
    }

    if (frame == NULL) { /* send all frame in cache */
        ssctx->send_gop = 2;
    }

    ssctx->gop_pos = pos;

    return NGX_AGAIN;
}

ngx_int_t
ngx_mpegts_gop_link(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss,
    ngx_msec_t time_offset, ngx_msec_t duration)
{
    ngx_mpegts_gop_app_conf_t    *gacf;
    ngx_mpegts_gop_ctx_t         *sctx, *ssctx;
    ngx_mpegts_frame_t           *frame;

    gacf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_gop_module);
    if (gacf->cache_time == 0) {
        return NGX_DECLINED;
    }

    sctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    if (sctx == NULL) { /* publisher doesn't publish av frame */
        return NGX_DECLINED;
    }

    ssctx = ngx_rtmp_get_module_ctx(ss, ngx_mpegts_gop_module);
    if (ssctx == NULL) {
        ssctx = ngx_pcalloc(ss->pool, sizeof(ngx_mpegts_gop_ctx_t));
        if (ssctx == NULL) {
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(ss, ssctx, ngx_mpegts_gop_module);
    }

    if (ngx_mpegts_gop_offset_frames(s, ss, time_offset, duration) == NGX_AGAIN)
    {
        return NGX_OK;
    }

    /* new frame is video key frame */
    if (sctx->cache[ssctx->gop_pos] == NULL) {
        ngx_log_error(NGX_LOG_ERR, ss->log, 0,
                "mpegts-gop: link| current gop pos is NULL, "
                "skip to new postion [pos %d last %d] %d",
                sctx->gop_pos, sctx->gop_last, ssctx->gop_pos);

        ssctx->gop_pos = sctx->gop_pos;
    }

    frame = sctx->cache[ssctx->gop_pos];
    if (ngx_mpegts_gop_link_frame(ss, frame) == NGX_AGAIN) {
        return NGX_AGAIN;
    }

    ssctx->gop_pos = ngx_mpegts_gop_next(s, ssctx->gop_pos);

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_gop_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_mpegts_gop_ctx_t   *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_gop_module);
    if (ctx == NULL) {
        goto next;
    }

    if (!s->published) {
        goto next;
    }

    /* free cache in publisher */
    while (ctx->gop_pos != ctx->gop_last) {
        ngx_rtmp_shared_free_mpegts_frame(ctx->cache[ctx->gop_pos]);
        ctx->gop_pos = ngx_mpegts_gop_next(s, ctx->gop_pos);
    }

next:
    return next_close_stream(s, v);
}

static ngx_int_t
ngx_mpegts_gop_av(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    if (frame->type == NGX_MPEGTS_MSG_VIDEO) {
        ngx_mpegts_gop_cache(s, frame);

        return next_mpegts_video(s, frame);
    } else if (frame->type == NGX_MPEGTS_MSG_AUDIO) {
        ngx_mpegts_gop_cache(s, frame);

        return next_mpegts_audio(s, frame);
    }

    return NGX_ERROR;
}

static ngx_int_t
ngx_mpegts_gop_postconfiguration(ngx_conf_t *cf)
{
    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_mpegts_gop_close_stream;

    next_mpegts_video = ngx_mpegts_video;
    ngx_mpegts_video = ngx_mpegts_gop_av;

    next_mpegts_audio = ngx_mpegts_audio;
    ngx_mpegts_audio = ngx_mpegts_gop_av;

    return NGX_OK;
}
