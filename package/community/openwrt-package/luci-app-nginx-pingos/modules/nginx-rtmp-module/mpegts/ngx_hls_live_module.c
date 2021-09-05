
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include <ngx_rtmp_cmd_module.h>
#include <ngx_rtmp_codec_module.h>
#include "ngx_rbuf.h"
#include "ngx_hls_live_module.h"
#include "ngx_mpegts_live_module.h"
#include "ngx_rtmp_live_module.h"
#include "ngx_mpegts_gop_module.h"

static ngx_rtmp_play_pt                 next_play;
static ngx_rtmp_close_stream_pt         next_close_stream;

static ngx_int_t ngx_hls_live_postconfiguration(ngx_conf_t *cf);
static void * ngx_hls_live_create_main_conf(ngx_conf_t *cf);
static void * ngx_hls_live_create_app_conf(ngx_conf_t *cf);
static char * ngx_hls_live_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t
ngx_hls_live_write_frame(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame);
static void
ngx_hls_live_update_playlist(ngx_rtmp_session_t *s);
static ngx_int_t
ngx_hls_live_update(ngx_rtmp_session_t *s, ngx_rtmp_codec_ctx_t *codec_ctx);

static ngx_mpegts_video_pt next_mpegts_video;
static ngx_mpegts_audio_pt next_mpegts_audio;

typedef struct {
    ngx_flag_t                          hls;
    ngx_msec_t                          fraglen;
    ngx_msec_t                          max_fraglen;
    ngx_msec_t                          playlen;
    ngx_uint_t                          winfrags;
    ngx_uint_t                          minfrags;
    ngx_uint_t                          slicing;
    ngx_uint_t                          type;
    ngx_path_t                         *slot;
    size_t                              audio_buffer_size;
    ngx_flag_t                          cleanup;
    ngx_array_t                        *variant;
    ngx_str_t                           base_url;
    ngx_pool_t                         *pool;
    ngx_msec_t                          timeout;
} ngx_hls_live_app_conf_t;

typedef struct {
    ngx_hls_live_frag_t                *free_frag;
    ngx_pool_t                         *pool;
} ngx_hls_live_main_conf_t;

ngx_hls_live_main_conf_t *ngx_hls_live_main_conf = NULL;

#define NGX_RTMP_HLS_NAMING_SEQUENTIAL  1
#define NGX_RTMP_HLS_NAMING_TIMESTAMP   2
#define NGX_RTMP_HLS_NAMING_SYSTEM      3


#define NGX_RTMP_HLS_SLICING_PLAIN      1
#define NGX_RTMP_HLS_SLICING_ALIGNED    2


#define NGX_RTMP_HLS_TYPE_LIVE          1
#define NGX_RTMP_HLS_TYPE_EVENT         2


static ngx_conf_enum_t                  ngx_hls_live_slicing_slots[] = {
    { ngx_string("plain"),              NGX_RTMP_HLS_SLICING_PLAIN },
    { ngx_string("aligned"),            NGX_RTMP_HLS_SLICING_ALIGNED  },
    { ngx_null_string,                  0 }
};

/*
static ngx_conf_enum_t                  ngx_hls_live_type_slots[] = {
    { ngx_string("live"),               NGX_RTMP_HLS_TYPE_LIVE  },
    { ngx_string("event"),              NGX_RTMP_HLS_TYPE_EVENT },
    { ngx_null_string,                  0 }
};
*/

static ngx_command_t ngx_hls_live_commands[] = {

    { ngx_string("hls2memory"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, hls),
      NULL },

    { ngx_string("hls2"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, hls),
      NULL },

    { ngx_string("hls2_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, fraglen),
      NULL },

    { ngx_string("hls2_max_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, max_fraglen),
      NULL },

    { ngx_string("hls2_playlist_length"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, playlen),
      NULL },

    { ngx_string("hls2_minfrags"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, minfrags),
      NULL },

    { ngx_string("hls2_fragment_slicing"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, slicing),
      &ngx_hls_live_slicing_slots },

    { ngx_string("hls2_audio_buffer_size"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, audio_buffer_size),
      NULL },

    { ngx_string("hls2_cleanup"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, cleanup),
      NULL },

    { ngx_string("hls2_base_url"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, base_url),
      NULL },

    { ngx_string("hls2_timeout"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_hls_live_app_conf_t, timeout),
      NULL },

    ngx_null_command
};


static ngx_rtmp_module_t  ngx_hls_live_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_hls_live_postconfiguration,     /* postconfiguration */

    ngx_hls_live_create_main_conf,      /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_hls_live_create_app_conf,       /* create location configuration */
    ngx_hls_live_merge_app_conf,        /* merge location configuration */
};


ngx_module_t  ngx_hls_live_module = {
    NGX_MODULE_V1,
    &ngx_hls_live_module_ctx,           /* module context */
    ngx_hls_live_commands,              /* module directives */
    NGX_RTMP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_hls_live_frag_t *
ngx_hls_live_get_frag(ngx_rtmp_session_t *s, ngx_int_t n)
{
    ngx_hls_live_ctx_t         *ctx;
    ngx_hls_live_app_conf_t    *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    return ctx->frags[(ctx->nfrag + n) % (hacf->winfrags * 2 + 1)];
}


static void
ngx_hls_live_next_frag(ngx_rtmp_session_t *s)
{
    ngx_hls_live_ctx_t         *ctx;
    ngx_hls_live_app_conf_t    *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    if (ctx->nfrags == hacf->winfrags) {
        ctx->nfrag++;
    } else {
        ctx->nfrags++;
    }
}


ngx_int_t
ngx_hls_live_write_playlist(ngx_rtmp_session_t *s, ngx_buf_t *out,
    time_t *last_modified_time)
{
    ngx_hls_live_ctx_t        *ctx;
    ngx_str_t                  m3u8;
    ngx_hls_live_app_conf_t   *hacf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);
    if (ctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "hls-live: playlist| ctx is null");

        return NGX_ERROR;
    }

    ctx->last_time = time(NULL);

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    if (ctx->nfrags < hacf->minfrags || ctx->playing == 0) {
        return NGX_AGAIN;
    }

    ngx_hls_live_update_playlist(s);

    *last_modified_time = ctx->playlist_modified_time;

    out->last = ngx_cpymem(out->pos, ctx->playlist->pos,
        ctx->playlist->last - ctx->playlist->pos);

    m3u8.data = out->pos;
    m3u8.len = out->last - out->pos;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0, "hls-live: playlist| %V, %D",
        &m3u8, ctx->last_time);

    return NGX_OK;
}


ngx_hls_live_frag_t*
ngx_hls_live_find_frag(ngx_rtmp_session_t *s, ngx_str_t *name)
{
    ngx_hls_live_ctx_t        *ctx;
    u_char                    *p0, *p1, *e;
    ngx_uint_t                 frag_id;
    ngx_hls_live_frag_t       *frag;
    ngx_hls_live_app_conf_t   *hacf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    p0 = name->data;
    e = p0 + name->len;

    for (; *e != '.' && e != p0; e--);

    if (e == p0) {
        return NULL;
    }

    p1 = e;

    for (; *e != '-' && e != p0; e--);

    if (e == p0) {
        return NULL;
    }

    p0 = e + 1;

    frag_id = ngx_atoi(p0, p1 - p0);

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    if (frag_id > ctx->nfrag + ctx->nfrags ||
        ctx->nfrag + ctx->nfrags - frag_id > hacf->winfrags * 2 + 1)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
            "hls-live: find_frag| invalid frag id[%d], curr id [%d]",
            frag_id, ctx->nfrag + ctx->nfrags);
        return NULL;
    }

    frag = ctx->frags[frag_id % (hacf->winfrags * 2 + 1)];

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
        "hls-live: find_frag| find frag %p [%d] [frag %d] length %ui",
        frag, frag_id, frag->id, frag->length);

    return frag;
}

void
ngx_rtmp_shared_acquire_frag(ngx_hls_live_frag_t *frag)
{
    frag->ref++;
}

ngx_chain_t*
ngx_hls_live_prepare_frag(ngx_rtmp_session_t *s, ngx_hls_live_frag_t *frag)
{
    ngx_chain_t          *out, *cl, **ll, *last_chain;
    ngx_mpegts_frame_t   *frame;

    ll = &out;
    if (frag->content_pos == frag->content_last) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "hls-live: prepare_frag| "
        "pos %D == last %D",
        frag->content_pos, frag->content_last);
        return NULL;
    }

    last_chain = NULL;

    while (frag->content_pos != frag->content_last) {
        frame = frag->content[frag->content_pos];

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0, "hls-live: prepare_frag| "
        "pos %D, last %D, frame chain %p",
        frag->content_pos, frag->content_last,
        frame);
        for (cl = frame->chain; cl; cl = cl->next) {
            *ll = ngx_get_chainbuf(0, 0);
            (*ll)->buf->pos = cl->buf->pos;
            (*ll)->buf->last = cl->buf->last;
            (*ll)->buf->flush = 1;

            last_chain = *ll;

            ll = &(*ll)->next;
        }

        *ll = NULL;
        frag->content_pos = ngx_hls_live_next(s, frag->content_pos);
    }

    if (last_chain) {
        last_chain->buf->last_buf = 1;
        last_chain->buf->last_in_chain = 1;
    }
    frag->content_pos = 0;

    return out;
}


ngx_rtmp_session_t*
ngx_hls_live_fetch_session(ngx_str_t *server,
    ngx_str_t *stream, ngx_str_t *session)
{
    ngx_live_stream_t    *live_stream;
    ngx_hls_live_ctx_t   *ctx;

    live_stream = ngx_live_fetch_stream(server, stream);
    if (live_stream) {
        for (ctx = live_stream->hls_ctx; ctx; ctx = ctx->next) {
            if (session->len == ctx->sid.len &&
                !ngx_strncmp(ctx->sid.data, session->data, session->len))
            {
                ngx_log_error(NGX_LOG_DEBUG, ctx->session->log, 0,
                    "hls-live: fetch_session| session[%p]"
                    "server %V, stream %V, session %V",
                    ctx->session, server, stream, session);
                return ctx->session;
            }
        }
    }

    return NULL;
}

static uint64_t
ngx_hls_live_get_fragment_id(ngx_rtmp_session_t *s, uint64_t ts)
{
    ngx_hls_live_ctx_t         *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    return ctx->nfrag + ctx->nfrags;
}

static void
ngx_hls_live_update_playlist(ngx_rtmp_session_t *s)
{
    u_char                         *p, *end;
    ngx_hls_live_ctx_t             *ctx;
    ngx_hls_live_app_conf_t        *hacf;
    ngx_hls_live_frag_t            *frag;
    ngx_uint_t                      i, max_frag;
    ngx_str_t                       name_part;
    ngx_str_t                       m3u8;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);
    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    time(&ctx->playlist_modified_time);

    max_frag = hacf->fraglen / 1000;

    frag = NULL;

    for (i = 0; i < ctx->nfrags; i++) {
        frag = ngx_hls_live_get_frag(s, i);
        if (frag && frag->duration > max_frag) {
            max_frag = (ngx_uint_t) (frag->duration + .5);
        }
    }

    if (!ctx->playlist) {
        ctx->playlist = ngx_create_temp_buf(s->pool, 1024*512);
    }

    p = ctx->playlist->pos;
    end = ctx->playlist->end;

    p = ngx_slprintf(p, end,
                     "#EXTM3U\n"
                     "#EXT-X-VERSION:3\n"
                     "#EXT-X-MEDIA-SEQUENCE:%uL\n"
                     "#EXT-X-TARGETDURATION:%ui\n",
                     ctx->nfrag, max_frag);

//    if (hacf->type == NGX_RTMP_HLS_TYPE_EVENT) {
//        p = ngx_slprintf(p, end, "#EXT-X-PLAYLIST-TYPE: EVENT\n");
//    }

    name_part = s->name;

    for (i = 0; i < ctx->nfrags; i++) {
        frag = ngx_hls_live_get_frag(s, i);

        if (frag->discont) {
            p = ngx_slprintf(p, end, "#EXT-X-DISCONTINUITY\n");
        }

        p = ngx_slprintf(p, end,
            "#EXTINF:%.3f,\n"
            "%V%V-%uL.ts?session=%V&slot=%d\n",
            frag->duration, &hacf->base_url,
            &name_part, frag->id, &ctx->sid, ngx_process_slot);

        ngx_log_debug5(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "hls: fragment nfrag=%uL, n=%ui/%ui, duration=%.3f, "
            "discont=%i",
            ctx->nfrag, i + 1, ctx->nfrags, frag->duration, frag->discont);
    }
/*
    for (; frag && i < hacf->winfrags; i++) {
        p = ngx_slprintf(p, end,
                    "#EXTINF:%.3f,\n"
                    "%V%V-%uL.ts?session=%V&slot=%d\n",
                    hacf->fraglen/1000.0, &hacf->base_url,
                    &name_part, i, &ctx->sid, ngx_process_slot);
    }
*/
    ctx->playlist->last = p;
    m3u8.data = ctx->playlist->pos;
    m3u8.len = ctx->playlist->last - ctx->playlist->pos;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0, "hls-live: playlist| %V", &m3u8);

}

static ngx_int_t
ngx_hls_live_close_fragment(ngx_rtmp_session_t *s)
{
    ngx_hls_live_ctx_t        *ctx;
    ngx_hls_live_app_conf_t   *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    if (ctx == NULL || !ctx->opened) {
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "hls: close fragment id=%uL", ctx->nfrag);

    ctx->opened = 0;

    ngx_hls_live_next_frag(s);

    if (ctx->nfrags >= hacf->minfrags && !ctx->playing)
    {
        ctx->playing = 1;
        ngx_rtmp_fire_event(s, NGX_MPEGTS_MSG_M3U8, NULL, NULL);
    }

    return NGX_OK;
}


void
ngx_hls_live_free_frag(ngx_hls_live_frag_t *frag)
{
    ngx_mpegts_frame_t        *frame;
    ngx_uint_t                 i;

    frag->ref--;

    if (frag->ref > 0) {
        return;
    }

    ngx_log_error(NGX_LOG_DEBUG, ngx_cycle->log, 0,
        "hls-live: free_frag| frag[%p] ref %ui", frag, frag->ref);

    for (i = 0; i < frag->content_last; ++i) {
        frame = frag->content[i];
        if (frame) {
            ngx_rtmp_shared_free_mpegts_frame(frame);
        }
    }

    frag->next = ngx_hls_live_main_conf->free_frag;
    ngx_hls_live_main_conf->free_frag = frag;
}


static ngx_hls_live_frag_t*
ngx_hls_live_create_frag(ngx_rtmp_session_t *s) {
    ngx_hls_live_frag_t       *frag;

    if (ngx_hls_live_main_conf->free_frag) {
        frag = ngx_hls_live_main_conf->free_frag;
        ngx_hls_live_main_conf->free_frag =
            ngx_hls_live_main_conf->free_frag->next;

        ngx_memzero(frag, sizeof(ngx_hls_live_frag_t) +
            sizeof(ngx_mpegts_frame_t*) * s->out_queue);
    } else {
        frag = ngx_pcalloc(ngx_hls_live_main_conf->pool,
            sizeof(ngx_hls_live_frag_t) +
            sizeof(ngx_mpegts_frame_t*) * s->out_queue);
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
        "hls-live: create_frag| create frag[%p]", frag);

    return frag;
}


static ngx_int_t
ngx_hls_live_open_fragment(ngx_rtmp_session_t *s, uint64_t ts,
    ngx_int_t discont)
{
    uint64_t                  id;
    ngx_hls_live_ctx_t       *ctx;
    ngx_hls_live_frag_t     **ffrag, *frag;
    ngx_hls_live_app_conf_t  *hacf;
    ngx_mpegts_frame_t       *frame;
    ngx_chain_t               patpmt;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    if (ctx->opened) {
        return NGX_OK;
    }

    id = ngx_hls_live_get_fragment_id(s, ts);

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "hls: open_fragment| create frag[%uL] timestamp %uL", id, ts);

    ffrag = &(ctx->frags[id % (hacf->winfrags * 2 + 1)]);
    if (*ffrag) {
        ngx_hls_live_free_frag(*ffrag);
    }
    *ffrag = ngx_hls_live_create_frag(s);

    frag = *ffrag;

    ngx_memzero(frag, sizeof(*frag));

    frag->ref = 1;
    frag->active = 1;
    frag->discont = discont;
    frag->id = id;

    ctx->opened = 1;
    ctx->frag_ts = ts;

    ngx_memzero(&patpmt, sizeof(patpmt));
    patpmt.buf = ngx_create_temp_buf(s->pool, 376);
    patpmt.buf->last = ngx_cpymem(patpmt.buf->pos,
        ngx_rtmp_mpegts_pat, 188);

    ngx_rtmp_mpegts_gen_pmt(s->vcodec, s->acodec, s->log, patpmt.buf->last);
    patpmt.buf->last += 188;

    frame = ngx_rtmp_shared_alloc_mpegts_frame(&patpmt, 1);

    ngx_hls_live_write_frame(s, frame);

    ngx_rtmp_shared_free_mpegts_frame(frame);

    frag->length = 376;

    return NGX_OK;
}


static void
ngx_hls_live_timeout(ngx_event_t *ev)
{
    ngx_rtmp_session_t   *s;
    ngx_hls_live_ctx_t   *ctx;

    s = ev->data;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    if (time(NULL) - ctx->last_time > ctx->timeout/1000) {
        ngx_log_error(NGX_LOG_ALERT, s->log, 0, "hls-live: timeout| "
            "hls live timeout, %D - %D > %D.", time(NULL), ctx->last_time,
            ctx->timeout/1000);

        ngx_rtmp_finalize_fake_session(s);
        return;
    }

    ngx_add_timer(ev, (ctx->timeout + 3000)/2);
}


static ngx_int_t
ngx_hls_live_join(ngx_rtmp_session_t *s, u_char *name, unsigned publisher)
{
    ngx_hls_live_ctx_t           *ctx;
    ngx_live_stream_t            *st;
    ngx_rtmp_live_app_conf_t     *lacf;
    ngx_hls_live_app_conf_t      *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_live_module);
    if (lacf == NULL) {
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);
    if (ctx && ctx->stream) {
        ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "mpegts-live: join| already joined");
        return NGX_ERROR;
    }

    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_hls_live_ctx_t));
        ngx_rtmp_set_ctx(s, ctx, ngx_hls_live_module);

        ctx->frags = ngx_pcalloc(s->pool,
            sizeof(ngx_hls_live_frag_t *) * (hacf->winfrags * 2 + 1));
        if (ctx->frags == NULL) {
            return NGX_ERROR;
        }
    }

    ctx->session = s;

    ngx_log_error(NGX_LOG_INFO, s->log, 0,
                   "mpegts-live: join| join '%s'", name);

    st = s->live_stream;

    if (!(publisher || st->publish_ctx || lacf->idle_streams)) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "mpegts-live: join| stream not found");

        s->status = 404;

        ngx_rtmp_finalize_session(s);

        return NGX_ERROR;
    }

    ctx->stream = st;
    ctx->next = st->hls_ctx;

    st->hls_ctx = ctx;

    ctx->ev.data = s;
    ctx->ev.handler = ngx_hls_live_timeout;
    ctx->ev.log = s->log;
    ctx->timeout = hacf->timeout;

    ngx_add_timer(&ctx->ev, (ctx->timeout + 3000)/2);

    return NGX_OK;
}

static ngx_int_t
ngx_hls_live_play(ngx_rtmp_session_t *s, ngx_rtmp_play_t *v)
{
    ngx_hls_live_app_conf_t        *hacf;
    ngx_hls_live_ctx_t             *ctx;
//    ngx_rtmp_codec_ctx_t           *codec_ctx;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);
    if (hacf == NULL || !hacf->hls || s->live_type != NGX_HLS_LIVE) {
        goto next;
    }

    if (s->interprocess || s->live_type != NGX_HLS_LIVE) {
        goto next;
    }

    if (ngx_hls_live_join(s, v->name, 0) == NGX_ERROR) {
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    ctx->sid.len = ngx_strlen(v->session);
    ctx->sid.data = ngx_pcalloc(s->pool, ctx->sid.len);
    ngx_memcpy(ctx->sid.data, v->session, ctx->sid.len);
/*
    if (s->live_stream->publish_ctx) {

        codec_ctx = ngx_rtmp_get_module_ctx(s->live_stream->publish_ctx->session,
            ngx_rtmp_codec_module);

        ngx_mpegts_gop_link(s->live_stream->publish_ctx->session,
            s, hacf->playlen, hacf->playlen);
        ngx_hls_live_update(s, codec_ctx);
    }
*/
next:
    return next_play(s, v);
}


static ngx_int_t
ngx_hls_live_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_hls_live_app_conf_t   *hacf;
    ngx_hls_live_ctx_t        *ctx, **cctx;
    ngx_uint_t                 i;
    ngx_hls_live_frag_t       *frag;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    if (hacf == NULL || !hacf->hls || ctx == NULL) {
        goto next;
    }

    if (ctx->ev.timer_set) {
        ngx_del_timer(&ctx->ev);
    }

    if (ctx->stream == NULL) {
        goto next;
    }

    ngx_rtmp_fire_event(s, NGX_MPEGTS_MSG_CLOSE, NULL, NULL);

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: close stream");

    ngx_hls_live_close_fragment(s);

    for (i = 0; i < 2 * hacf->winfrags + 1; i++) {
        frag = ctx->frags[i % (hacf->winfrags * 2 + 1)];
        if (frag) {
            ngx_hls_live_free_frag(frag);
        }
    }

    for (cctx = &ctx->stream->hls_ctx; *cctx; cctx = &(*cctx)->next) {
        if (*cctx == ctx) {
            *cctx = ctx->next;
            break;
        }
    }

    if (ctx->stream->hls_ctx) {
        ctx->stream = NULL;
        goto next;
    }

next:
    return next_close_stream(s, v);
}


static void
ngx_hls_live_update_fragment(ngx_rtmp_session_t *s, uint64_t ts,
    ngx_int_t boundary)
{
    ngx_hls_live_ctx_t         *ctx;
    ngx_hls_live_app_conf_t    *hacf;
    ngx_hls_live_frag_t        *frag;
    ngx_msec_t                  ts_frag_len;
    ngx_int_t                   same_frag, force,discont;
    int64_t                     d;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);
    frag = NULL;
    force = 0;
    discont = 1;

    if (ctx->opened) {
        frag = ngx_hls_live_get_frag(s, ctx->nfrags);
        d = (int64_t) (ts - ctx->frag_ts);

        if (d > (int64_t) hacf->max_fraglen * 90 || d < -90000) {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                        "hls: force fragment split: %.3f sec, ", d / 90000.);
            force = 1;
//            if (!boundary) {
                discont = 0;
//            }
        } else {
            frag->duration = (ts - ctx->frag_ts) / 90000.;
            discont = 0;
        }
    }

    switch (hacf->slicing) {
        case NGX_RTMP_HLS_SLICING_PLAIN:
            if (frag && frag->duration < hacf->fraglen / 1000.) {
                boundary = 0;
            }
            break;

        case NGX_RTMP_HLS_SLICING_ALIGNED:

            ts_frag_len = hacf->fraglen * 90;
            same_frag = ctx->frag_ts / ts_frag_len == ts / ts_frag_len;

            if (frag && same_frag) {
                boundary = 0;
            }

            if (frag == NULL && (ctx->frag_ts == 0 || same_frag)) {
                ctx->frag_ts = ts;
                boundary = 0;
            }

            break;
    }

    if (boundary || force) {
        ngx_hls_live_close_fragment(s);
        ngx_hls_live_open_fragment(s, ts, discont);
    }
}


static ngx_int_t
ngx_hls_live_write_frame(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    ngx_hls_live_frag_t   *frag;
    ngx_hls_live_ctx_t    *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    frag = ngx_hls_live_get_frag(s, ctx->nfrags);

    frag->length += frame->length;

    frag->content[frag->content_last] = frame;
    frag->content_last = ngx_hls_live_next(s, frag->content_last);
    time(&frag->last_modified_time);

    ngx_rtmp_shared_acquire_frame(frame);

    return NGX_OK;
}


static ngx_int_t
ngx_hls_live_update(ngx_rtmp_session_t *s, ngx_rtmp_codec_ctx_t *codec_ctx)
{
    ngx_hls_live_ctx_t   *ctx;
    ngx_mpegts_frame_t   *frame;
    ngx_int_t             boundary;
    ngx_buf_t            *b;

    b = NULL;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_hls_live_module);

    while (s->out_pos != s->out_last) {

        frame = s->mpegts_out[s->out_pos];
#if 1
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "hls-live: update| "
            "frame[%p] pos[%O] last[%O] pts[%uL] type [%d], key %d, opened %d",
            frame, s->out_pos, s->out_last,frame->pts,
            frame->type, frame->key, ctx->opened);
#endif
        boundary = 0;

        if (frame->type == NGX_MPEGTS_MSG_AUDIO) {
            boundary = codec_ctx->avc_header == NULL;
        } else if (frame->type == NGX_MPEGTS_MSG_VIDEO) {
            b = ctx->aframe;
            boundary = frame->key &&
                (codec_ctx->aac_header == NULL || !ctx->opened ||
                (b && b->last > b->pos));
        } else {
            return NGX_ERROR;
        }

        s->acodec = codec_ctx->audio_codec_id;
        s->vcodec = codec_ctx->video_codec_id;

        ngx_hls_live_update_fragment(s, frame->pts, boundary);

        if (!ctx->opened) {
            break;
        }

        ngx_hls_live_write_frame(s, frame);

        ngx_rtmp_shared_free_mpegts_frame(frame);

        ++s->out_pos;
        s->out_pos %= s->out_queue;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_hls_live_av(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    ngx_live_stream_t         *live_stream;
    ngx_hls_live_ctx_t        *ctx;
    ngx_rtmp_session_t        *ss;
    ngx_rtmp_codec_ctx_t      *codec_ctx;
    ngx_hls_live_app_conf_t   *hacf;

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    hacf = ngx_rtmp_get_module_app_conf(s, ngx_hls_live_module);

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "hls-live: av| pts[%uL] type [%d] key[%d]",
            frame->dts/90, frame->type, frame->key);

    live_stream = s->live_stream;
    for (ctx = live_stream->hls_ctx; ctx; ctx = ctx->next) {
        ss = ctx->session;

        switch (ngx_mpegts_gop_link(s, ss, hacf->playlen, hacf->playlen)) {
        case NGX_DECLINED:
            continue;
        case NGX_ERROR:
            ngx_rtmp_finalize_fake_session(ss);
            continue;
        default:
            break;
        }

        ngx_hls_live_update(ss, codec_ctx);
    }

    if (frame->type == NGX_MPEGTS_MSG_VIDEO) {
        return next_mpegts_video(s, frame);
    } else if (frame->type == NGX_MPEGTS_MSG_AUDIO) {
        return next_mpegts_audio(s, frame);
    }

    return NGX_ERROR;
}

static void *
ngx_hls_live_create_main_conf(ngx_conf_t *cf)
{
    ngx_hls_live_main_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_hls_live_main_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->pool = ngx_create_pool(1024, ngx_cycle->log);
    ngx_hls_live_main_conf = conf;

    return conf;
}

static void *
ngx_hls_live_create_app_conf(ngx_conf_t *cf)
{
    ngx_hls_live_app_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_hls_live_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->pool = ngx_create_pool(1024, ngx_cycle->log);

    conf->hls = NGX_CONF_UNSET;
    conf->fraglen = NGX_CONF_UNSET_MSEC;
    conf->max_fraglen = NGX_CONF_UNSET_MSEC;
    conf->playlen = NGX_CONF_UNSET_MSEC;
    conf->slicing = NGX_CONF_UNSET_UINT;
    conf->type = NGX_CONF_UNSET_UINT;
    conf->audio_buffer_size = NGX_CONF_UNSET_SIZE;
    conf->cleanup = NGX_CONF_UNSET;
    conf->timeout = NGX_CONF_UNSET_MSEC;
    conf->minfrags = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_hls_live_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_hls_live_app_conf_t    *prev = parent;
    ngx_hls_live_app_conf_t    *conf = child;

    ngx_conf_merge_value(conf->hls, prev->hls, 0);
    ngx_conf_merge_msec_value(conf->fraglen, prev->fraglen, 5000);
    ngx_conf_merge_msec_value(conf->max_fraglen, prev->max_fraglen,
                              conf->fraglen * 10);
    ngx_conf_merge_msec_value(conf->playlen, prev->playlen, conf->fraglen * 3);
    ngx_conf_merge_uint_value(conf->slicing, prev->slicing,
                              NGX_RTMP_HLS_SLICING_PLAIN);
    ngx_conf_merge_value(conf->cleanup, prev->cleanup, 1);
    ngx_conf_merge_str_value(conf->base_url, prev->base_url, "");
    ngx_conf_merge_uint_value(conf->minfrags, prev->minfrags, 2);

    conf->timeout = conf->playlen * 3;

    if (conf->fraglen) {
        conf->winfrags = conf->playlen / conf->fraglen;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_hls_live_postconfiguration(ngx_conf_t *cf)
{
    next_play = ngx_rtmp_play;
    ngx_rtmp_play = ngx_hls_live_play;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_hls_live_close_stream;

    next_mpegts_video = ngx_mpegts_video;
    ngx_mpegts_video = ngx_hls_live_av;

    next_mpegts_audio = ngx_mpegts_audio;
    ngx_mpegts_audio = ngx_hls_live_av;

    return NGX_OK;
}
