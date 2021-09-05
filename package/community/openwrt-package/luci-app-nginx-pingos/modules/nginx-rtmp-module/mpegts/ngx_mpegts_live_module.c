
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_live_module.h"
#include "ngx_rbuf.h"
#include "ngx_mpegts_live_module.h"
#include "ngx_mpegts_gop_module.h"
#include "ngx_rtmp_codec_module.h"

ngx_mpegts_video_pt ngx_mpegts_video;
ngx_mpegts_audio_pt ngx_mpegts_audio;

static ngx_rtmp_publish_pt         next_publish;
static ngx_rtmp_play_pt            next_play;
static ngx_rtmp_close_stream_pt    next_close_stream;

#define NGX_MPEGTS_BUF_SIZE   1316
#define NGX_RTMP_MPEG_BUFSIZE 1024*1024

#define TS_AUDIO_TYPE_AAC    0
#define TS_AUDIO_TYPE_MP3    1

#define TS_VIDEO_TYPE_H264   0
#define TS_VIDEO_TYPE_H265   1

typedef struct ngx_mpegts_live_app_conf_s {
    ngx_pool_t             *pool;
    size_t                  audio_buffer_size;
    ngx_msec_t              sync;
    ngx_msec_t              audio_delay;
    size_t                  out_queue;
    ngx_mpegts_live_ctx_t  *players;
    u_char                  packet_buffer[NGX_RTMP_MPEG_BUFSIZE];
} ngx_mpegts_live_app_conf_t;

typedef struct ngx_mpegts_live_avc_codec_s {
    ngx_rtmp_frame_t       *avc_header;
    ngx_uint_t              video_codec_id;
    ngx_uint_t              avc_nal_bytes;
} ngx_mpegts_live_avc_codec_t;

typedef struct ngx_mpegts_live_aac_codec_s {
    ngx_rtmp_frame_t       *aac_header;
    uint64_t                sample_rate;
} ngx_mpegts_live_aac_codec_t;

struct ngx_mpegts_live_ctx_s {

    ngx_mpegts_live_ctx_t        *next;
    ngx_rtmp_session_t           *session;
    ngx_live_stream_t            *stream;

    /* mpegts-module config */
    size_t                        audio_buffer_size;
    ngx_msec_t                    sync;
    ngx_msec_t                    audio_delay;
    size_t                        out_queue;

    /* pat pmt frame*/
    ngx_mpegts_frame_t           *patpmt;

    /* video packet */
    ngx_mpegts_live_avc_codec_t  *avc_codec;
    ngx_uint_t                    video_cc;

    /* audio packet */
    ngx_mpegts_live_aac_codec_t  *aac_codec;
    ngx_uint_t                    audio_cc;
    uint64_t                      aframe_pts;
    ngx_uint_t                    aframe_num;
    ngx_msec_t                    aframe_base;
    ngx_buf_t                    *aframe;
};

/* 700 ms PCR delay */
#define NGX_RTMP_MEGPTS_DELAY  63000

static void *
ngx_mpegts_live_create_app_conf(ngx_conf_t *cf);
static char *
ngx_mpegts_live_merge_app_conf(ngx_conf_t *cf, void *parent, void *child);
static ngx_int_t
ngx_mpegts_live_postconfiguration(ngx_conf_t *cf);

static ngx_command_t ngx_mpegts_live_commands[] = {

    { ngx_string("mpegts_audio_buffer_size"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_live_app_conf_t, audio_buffer_size),
      NULL },

    { ngx_string("mpegts_sync"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_live_app_conf_t, sync),
      NULL },

    { ngx_string("mpegts_audio_delay"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_live_app_conf_t, audio_delay),
      NULL },

    { ngx_string("mpegts_out_queue"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_mpegts_live_app_conf_t, out_queue),
      NULL },

    ngx_null_command
};


static ngx_rtmp_module_t ngx_mpegts_live_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_mpegts_live_postconfiguration,      /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_mpegts_live_create_app_conf,        /* create app configuration */
    ngx_mpegts_live_merge_app_conf          /* merge app configuration */
};


ngx_module_t ngx_mpegts_live_module = {
    NGX_MODULE_V1,
    &ngx_mpegts_live_ctx,                   /* module context */
    ngx_mpegts_live_commands,               /* module directives */
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
ngx_mpegts_live_create_app_conf(ngx_conf_t *cf)
{
    ngx_mpegts_live_app_conf_t   *macf;

    macf = ngx_pcalloc(cf->pool, sizeof(ngx_mpegts_live_app_conf_t));
    if (!macf) {
        return NULL;
    }

    macf->audio_buffer_size = NGX_CONF_UNSET;
    macf->sync = NGX_CONF_UNSET_MSEC;
    macf->audio_delay = NGX_CONF_UNSET_MSEC;
    macf->out_queue = NGX_CONF_UNSET;

    return macf;
}


static char *
ngx_mpegts_live_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_mpegts_live_app_conf_t *prev = parent;
    ngx_mpegts_live_app_conf_t *conf = child;

    ngx_conf_merge_size_value(conf->audio_buffer_size, prev->audio_buffer_size,
                              NGX_RTMP_MPEG_BUFSIZE);
    ngx_conf_merge_msec_value(conf->sync, prev->sync, 2);
    ngx_conf_merge_msec_value(conf->audio_delay, prev->audio_delay, 300);
    ngx_conf_merge_size_value(conf->out_queue, prev->out_queue, 4096);
    conf->pool = ngx_create_pool(4096, &cf->cycle->new_log);
    if (!conf->pool) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static u_char *
ngx_mpegts_live_write_pcr(u_char *p, uint64_t pcr)
{
    *p++ = (u_char) (pcr >> 25);
    *p++ = (u_char) (pcr >> 17);
    *p++ = (u_char) (pcr >> 9);
    *p++ = (u_char) (pcr >> 1);
    *p++ = (u_char) (pcr << 7 | 0x7e);
    *p++ = 0;

    return p;
}

static u_char *
ngx_mpegts_live_write_pts(u_char *p, ngx_uint_t fb, uint64_t pts)
{
    ngx_uint_t val;

    val = fb << 4 | (((pts >> 30) & 0x07) << 1) | 1;
    *p++ = (u_char) val;

    val = (((pts >> 15) & 0x7fff) << 1) | 1;
    *p++ = (u_char) (val >> 8);
    *p++ = (u_char) val;

    val = (((pts) & 0x7fff) << 1) | 1;
    *p++ = (u_char) (val >> 8);
    *p++ = (u_char) val;

    return p;
}

ngx_int_t
ngx_mpegts_live_shared_append_chain(ngx_mpegts_frame_t *f, ngx_buf_t *b,
                                    ngx_flag_t mandatory)
{
    ngx_uint_t   pes_size, header_size, body_size, in_size, stuff_size, flags;
    u_char      *packet, *p, *base;
    ngx_int_t    first;
    ngx_chain_t *cl, **ll;
    uint64_t     pcr;

    for (ll = &f->chain; (*ll) && (*ll)->next; ll = &(*ll)->next);
    cl = *ll;

    if ((b == NULL || b->pos == b->last) && mandatory) {
        *ll = ngx_get_chainbuf(NGX_MPEGTS_BUF_SIZE, 1);
        (*ll)->buf->flush = 1;
        return NGX_OK;
    }

    first = 1;

    while (b->pos < b->last) {
        if ((*ll) && (*ll)->buf->end - (*ll)->buf->last < 188) {
            ll = &(*ll)->next;
            cl = *ll;
        }

        if (*ll == NULL) {
            *ll = ngx_get_chainbuf(NGX_MPEGTS_BUF_SIZE, 1);
            cl = *ll;
            cl->buf->flush = 1;
        }

        packet = cl->buf->last;
        p = packet;

        f->cc++;

        *p++ = 0x47;
        *p++ = (u_char) (f->pid >> 8);

        if (first) {
            p[-1] |= 0x40;
        }

        *p++ = (u_char) f->pid;
        *p++ = 0x10 | (f->cc & 0x0f); /* payload */

        if (first) {

            if (f->key) {
                packet[3] |= 0x20; /* adaptation */

                *p++ = 7;    /* size */
                *p++ = 0x50; /* random access + PCR */
                if (f->dts < NGX_RTMP_MEGPTS_DELAY) {
                    pcr = 0;
                } else {
                    pcr = f->dts;
                }
                p = ngx_mpegts_live_write_pcr(p, pcr);
            }

            /* PES header */

            *p++ = 0x00;
            *p++ = 0x00;
            *p++ = 0x01;
            *p++ = (u_char) f->sid;

            header_size = 5;
            flags = 0x80; /* PTS */

            if (f->dts != f->pts) {
                header_size += 5;
                flags |= 0x40; /* DTS */
            }

            pes_size = (b->last - b->pos) + header_size + 3;
            if (pes_size > 0xffff) {
                pes_size = 0;
            }

            *p++ = (u_char) (pes_size >> 8);
            *p++ = (u_char) pes_size;
            *p++ = 0x80; /* H222 */
            *p++ = (u_char) flags;
            *p++ = (u_char) header_size;

            p = ngx_mpegts_live_write_pts(p, flags >> 6, f->pts +
                                                         NGX_RTMP_MEGPTS_DELAY);

            if (f->dts != f->pts) {
                p = ngx_mpegts_live_write_pts(p, 1, f->dts +
                                                    NGX_RTMP_MEGPTS_DELAY);
            }

            first = 0;
        }

        body_size = (ngx_uint_t) (packet + 188 - p);
        in_size = (ngx_uint_t) (b->last - b->pos);

        if (body_size <= in_size) {
            ngx_memcpy(p, b->pos, body_size);
            b->pos += body_size;

        } else {
            stuff_size = (body_size - in_size);

            if (packet[3] & 0x20) {

                /* has adaptation */

                base = &packet[5] + packet[4];
                p = ngx_movemem(base + stuff_size, base, p - base);
                ngx_memset(base, 0xff, stuff_size);
                packet[4] += (u_char) stuff_size;

            } else {

                /* no adaptation */

                packet[3] |= 0x20;
                p = ngx_movemem(&packet[4] + stuff_size, &packet[4],
                                p - &packet[4]);

                packet[4] = (u_char) (stuff_size - 1);
                if (stuff_size >= 2) {
                    packet[5] = 0;
                    ngx_memset(&packet[6], 0xff, stuff_size - 2);
                }
            }

            ngx_memcpy(p, b->pos, in_size);
            b->pos = b->last;
        }

        cl->buf->last += 188;
        f->length += 188;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_copy(ngx_rtmp_session_t *s, void *dst, u_char **src, size_t n,
    ngx_chain_t **in)
{
    u_char  *last;
    size_t   pn;

    if (*in == NULL) {
        return NGX_ERROR;
    }

    for ( ;; ) {
        last = (*in)->buf->last;

        if ((size_t)(last - *src) >= n) {
            if (dst) {
                ngx_memcpy(dst, *src, n);
            }

            *src += n;

            while (*in && *src == (*in)->buf->last) {
                *in = (*in)->next;
                if (*in) {
                    *src = (*in)->buf->pos;
                }
            }

            return NGX_OK;
        }

        pn = last - *src;

        if (dst) {
            ngx_memcpy(dst, *src, pn);
            dst = (u_char *)dst + pn;
        }

        n -= pn;
        *in = (*in)->next;

        if (*in == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "rtmp-mpegts: mpegts_copy| failed to read %uz byte(s)", n);
            return NGX_ERROR;
        }

        *src = (*in)->buf->pos;
    }
}


static ngx_int_t
ngx_mpegts_live_append_aud(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    static u_char   aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xf0 };

    if (out->last + sizeof(aud_nal) > out->end) {
        return NGX_ERROR;
    }

    out->last = ngx_cpymem(out->last, aud_nal, sizeof(aud_nal));

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_append_sps_pps(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    u_char                         *p;
    ngx_chain_t                    *in;
    ngx_mpegts_live_ctx_t          *ctx;
    int8_t                          nnals;
    uint16_t                        len, rlen;
    ngx_int_t                       n;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);

    if (ctx == NULL || ctx->avc_codec == NULL) {
        return NGX_ERROR;
    }

    in = ctx->avc_codec->avc_header->chain;
    if (in == NULL) {
        return NGX_ERROR;
    }

    p = in->buf->pos;

    /*
     * Skip bytes:
     * - flv fmt
     * - H264 CONF/PICT (0x00)
     * - 0
     * - 0
     * - 0
     * - version
     * - profile
     * - compatibility
     * - level
     * - nal bytes
     */

    if (ngx_mpegts_live_copy(s, NULL, &p, 10, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* number of SPS NALs */
    if (ngx_mpegts_live_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    nnals &= 0x1f; /* 5lsb */

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: append_sps_pps| SPS number: %uz", nnals);

    /* SPS */
    for (n = 0; ; ++n) {
        for (; nnals; --nnals) {

            /* NAL length */
            if (ngx_mpegts_live_copy(s, &rlen, &p, 2, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            ngx_rtmp_rmemcpy(&len, &rlen, 2);

            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                           "rtmp-mpegts: append_sps_pps| header NAL length: %uz", (size_t) len);

            /* AnnexB prefix */
            if (out->end - out->last < 4) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "rtmp-mpegts: append_sps_pps| too small buffer for header NAL size");
                return NGX_ERROR;
            }

            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 1;

            /* NAL body */
            if (out->end - out->last < len) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "rtmp-mpegts: append_sps_pps| too small buffer for header NAL");
                return NGX_ERROR;
            }

            if (ngx_mpegts_live_copy(s, out->last, &p, len, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            out->last += len;
        }

        if (n == 1) {
            break;
        }

        /* number of PPS NALs */
        if (ngx_mpegts_live_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: append_sps_pps| PPS number: %uz", nnals);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_init_aac_codec(ngx_rtmp_session_t *s)
{
    ngx_mpegts_live_ctx_t          *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s->live_stream->publish_ctx->session, ngx_rtmp_codec_module);

    if (ctx->aac_codec) {
        return NGX_OK;
    }

    if (codec_ctx == NULL || codec_ctx->aac_header == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                   "rtmp-mpegts: aac_codec| codec ctx %p, aac_header is null", codec_ctx);
        return NGX_AGAIN;
    }

    ctx->aac_codec = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_live_aac_codec_t));
    if (ctx->aac_codec == NULL) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: aac_codec| alloc mpegts aac_codec failed");
        return NGX_ERROR;
    }

    ctx->aac_codec->aac_header = ngx_rtmp_shared_alloc_frame(1024,
                                    codec_ctx->aac_header->chain, 0);

    ctx->aac_codec->sample_rate = codec_ctx->sample_rate;

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_init_avc_codec(ngx_rtmp_session_t *s)
{
    ngx_mpegts_live_ctx_t          *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s->live_stream->publish_ctx->session, ngx_rtmp_codec_module);

    if (ctx->avc_codec) {
        return NGX_OK;
    }

    if (codec_ctx == NULL || codec_ctx->avc_header == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                   "rtmp-mpegts: avc_codec| codec ctx %p, avc_header is null", codec_ctx);
        return NGX_AGAIN;
    }

    ctx->avc_codec = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_live_avc_codec_t));
    if (ctx->avc_codec == NULL) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: avc_codec| alloc mpegts avc_codec failed");
        return NGX_ERROR;
    }

    ctx->avc_codec->video_codec_id = codec_ctx->video_codec_id;
    ctx->avc_codec->avc_nal_bytes = codec_ctx->avc_nal_bytes;
    ctx->avc_codec->avc_header = ngx_rtmp_shared_alloc_frame(1024,
                                 codec_ctx->avc_header->chain, 0);

    return NGX_OK;
}

/*
static void
ngx_mpegts_live_append_out_chain(ngx_chain_t **header, ngx_mpegts_frame_t *frame)
{
    ngx_chain_t   **ll;
    ngx_chain_t    *tail;

    ngx_rtmp_shared_acquire_frame(frame);
    tail = frame->chain;

    for (ll = header; *ll; ll = &((*ll)->next)) {
        (*ll)->buf->flush = 1;
    }

    *ll = tail;

    for (; *ll; ll = &((*ll)->next)) {
        (*ll)->buf->flush = 1;
        if (!(*ll)->next) {
            (*ll)->buf->last_in_chain = 1;
        }
    }
}
*/

static ngx_int_t
ngx_mpegts_live_flush_audio(ngx_rtmp_session_t *s)
{
    ngx_mpegts_live_ctx_t          *ctx;
    ngx_mpegts_frame_t             *frame;
    ngx_int_t                       rc;
    ngx_buf_t                      *b;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);

    if (ctx == NULL) {
        return NGX_OK;
    }

    b = ctx->aframe;

    if (b == NULL || b->pos == b->last) {
        return NGX_OK;
    }

    frame = ngx_rtmp_shared_alloc_mpegts_frame(NULL, 0);

    frame->dts = ctx->aframe_pts;
    frame->pts = ctx->aframe_pts;
    frame->cc = ctx->audio_cc;
    frame->pid = 0x101;
    frame->sid = 0xc0;
    frame->type = NGX_MPEGTS_MSG_AUDIO;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: flush_audio| pts=%uL", frame->pts);

    rc = ngx_mpegts_live_shared_append_chain(frame, b, 1);

    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "rtmp-mpegts: flush_audio| flush failed");
    } else {
        ctx->audio_cc = frame->cc;
        ngx_mpegts_live_audio_filter(s, frame);
    }

    ngx_rtmp_shared_free_mpegts_frame(frame);

    b->pos = b->last = b->start;

    return rc;
}


static ngx_int_t
ngx_mpegts_live_append_hevc_vps_sps_pps(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    ngx_mpegts_live_ctx_t          *ctx;
    u_char                         *p;
    ngx_chain_t                    *in;
    ngx_uint_t                      rnal_unit_len, nal_unit_len, i, j,
                                    num_arrays, nal_unit_type, rnum_nalus,
                                    num_nalus;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);

    if (ctx == NULL || ctx->avc_codec == NULL) {
        return NGX_ERROR;
    }

    in = ctx->avc_codec->avc_header->chain;
    if (in == NULL) {
        return NGX_ERROR;
    }

    p = in->buf->pos;
    /*  6 bytes
     * FrameType                                    4 bits
     * CodecID                                      4 bits
     * AVCPacketType                                1 byte
     * CompositionTime                              3 bytes
     * HEVCDecoderConfigurationRecord
     *      configurationVersion                    1 byte
     */

    /*  20 bytes
     * HEVCDecoderConfigurationRecord
     *      general_profile_space                   2 bits
     *      general_tier_flag                       1 bit
     *      general_profile_idc                     5 bits
     *      general_profile_compatibility_flags     4 bytes
     *      general_constraint_indicator_flags      6 bytes
     *      general_level_idc                       1 byte
     *      min_spatial_segmentation_idc            4 bits reserved + 12 bits
     *      parallelismType                         6 bits reserved + 2 bits
     *      chroma_format_idc                       6 bits reserved + 2 bits
     *      bit_depth_luma_minus8                   5 bits reserved + 3 bits
     *      bit_depth_chroma_minus8                 5 bits reserved + 3 bits
     *      avgFrameRate                            2 bytes
     */

    /* 1 bytes
     * HEVCDecoderConfigurationRecord
     *      constantFrameRate                       2 bits
     *      numTemporalLayers                       3 bits
     *      temporalIdNested                        1 bit
     *      lengthSizeMinusOne                      2 bits
     */

    if (ngx_mpegts_live_copy(s, NULL, &p, 27, &in) != NGX_OK) {
        return NGX_ERROR;
    }

     /* 1 byte
     * HEVCDecoderConfigurationRecord
     *      numOfArrays                             1 byte
     */
    num_arrays = 0;
    if (ngx_mpegts_live_copy(s, &num_arrays, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    for (i = 0; i < num_arrays; ++i) {
        /*
         * array_completeness                       1 bit
         * reserved                                 1 bit
         * NAL_unit_type                            6 bits
         * numNalus                                 2 bytes
         */
        if (ngx_mpegts_live_copy(s, &nal_unit_type, &p, 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }
        nal_unit_type &= 0x3f;

        if (ngx_mpegts_live_copy(s, &rnum_nalus, &p, 2, &in) != NGX_OK) {
            return NGX_ERROR;
        }
        num_nalus = 0;
        ngx_rtmp_rmemcpy(&num_nalus, &rnum_nalus, 2);

        for (j = 0; j < num_nalus; ++j) {
            /*
             * nalUnitLength                        2 bytes
             */
            if (ngx_mpegts_live_copy(s, &rnal_unit_len, &p, 2, &in) != NGX_OK) {
                return NGX_ERROR;
            }
            nal_unit_len = 0;
            ngx_rtmp_rmemcpy(&nal_unit_len, &rnal_unit_len, 2);
            if (out->end - out->last < 4) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for header NAL size");
                return NGX_ERROR;
            }

            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 1;

            if (out->end - out->last < (ngx_int_t)nal_unit_len) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for header NAL");
                return NGX_ERROR;
            }

            if (ngx_mpegts_live_copy(s, out->last, &p, nal_unit_len, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            out->last += nal_unit_len;
        }
    }
    return NGX_OK;
}


/* set h265 aud first, now is null*/
static ngx_int_t
ngx_mpegts_live_append_hevc_aud(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    static u_char   aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x50 };

    if (out->last + sizeof(aud_nal) > out->end) {
        return NGX_ERROR;
    }

    out->last = ngx_cpymem(out->last, aud_nal, sizeof(aud_nal));

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_h265_handler(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *f)
{
    ngx_mpegts_live_ctx_t       *ctx;
    ngx_mpegts_live_app_conf_t  *macf;
    u_char                      *p;
    uint8_t                     fmt, ftype, htype, nal_type, src_nal_type;
    uint32_t                    len, rlen;
    ngx_buf_t                   out;
    uint32_t                    cts;
    ngx_mpegts_frame_t         *frame;
    ngx_uint_t                  nal_bytes;
    ngx_int_t                   aud_sent, sps_pps_sent;
    u_char                     *buffer;
    ngx_rtmp_header_t          *h;
    ngx_chain_t                *in;
    ngx_int_t                   rc;
    ngx_rtmp_core_app_conf_t   *cacf;

    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);

    h = &f->hdr;
    in = f->chain;

    macf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_live_module);
    buffer = macf->packet_buffer;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx == NULL || h->mlen < 1) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: h265_handler| "
                       "resource error, mpegts_ctx=%p, h->mlen=%d",
                       ctx, h->mlen);
        return NGX_OK;
    }

    if (ctx->avc_codec == NULL) {
        rc = ngx_mpegts_live_init_avc_codec(s);
        if (rc == NGX_ERROR) {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: h265_handler| init avc_codec failed");
            return NGX_OK;
        } else if (rc == NGX_AGAIN) {
            return NGX_OK;
        }
    }

    /* H265 is supported */
    if (ctx->avc_codec->video_codec_id != cacf->hevc_codec)
    {
        return NGX_OK;
    }

    p = in->buf->pos;
    if (ngx_mpegts_live_copy(s, &fmt, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* 1: keyframe (IDR)
     * 2: inter frame
     * 3: disposable inter frame */

    ftype = (fmt & 0xf0) >> 4;    // 0x17/0x27/...

    /* H264 HDR/PICT */

    if (ngx_mpegts_live_copy(s, &htype, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* proceed only with PICT */

    if (htype != 1) { /*0:AVC sequence header,1:AVC NALU 2:AVC end of sequence*/
        return NGX_OK;
    }

    /* 3 bytes: decoder delay */

    if (ngx_mpegts_live_copy(s, &cts, &p, 3, &in) != NGX_OK) {
        return NGX_ERROR;
    }
    /* convert big end to little end */
    cts = ((cts & 0x00FF0000) >> 16) | ((cts & 0x000000FF) << 16) |
          (cts & 0x0000FF00);

    ngx_memzero(&out, sizeof(out));

    out.start = buffer;
    out.end = buffer + NGX_RTMP_MPEG_BUFSIZE;
    out.pos = out.start;
    out.last = out.pos;

    nal_bytes = ctx->avc_codec->avc_nal_bytes;
    aud_sent = 0;
    sps_pps_sent = 0;
    ngx_int_t vps_copy = 0;
    ngx_int_t sps_copy = 0;
    ngx_int_t pps_copy = 0;

    while (in) {
        if (ngx_mpegts_live_copy(s, &rlen, &p, nal_bytes, &in) != NGX_OK) {
            return NGX_OK;
        }

        len = 0;
        ngx_rtmp_rmemcpy(&len, &rlen, nal_bytes);

        if (len == 0) {
            continue;
        }

        if (ngx_mpegts_live_copy(s, &src_nal_type, &p, 1, &in) != NGX_OK) {
            return NGX_OK;
        }

        nal_type = (src_nal_type & 0x7e) >> 1;

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: h265_handler| h265 NAL type=%ui, len=%uD",
                       (ngx_uint_t) nal_type, len);

        /* h264 format of rtmp_flv contains NAL header Prefix "00 00 00 01" */
        if (0 == nal_type) {
            u_char nal_header[4] = {0};
            if (ngx_mpegts_live_copy(s, nal_header, &p, 3, &in) != NGX_OK) {
                return NGX_OK;
            }

            if (0 != ngx_strcmp(nal_header, "\0\0\1")) {
                ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                    "mpegts module: h265 hander| "
                    "is not h265 NAL header [00 00 00 01]");
                p -= 3;
                goto NAL_TRAIL_N;
                return NGX_OK;
            }

            if (ngx_mpegts_live_copy(s, &src_nal_type, &p, 1, &in) != NGX_OK) {
                return NGX_OK;
            }

            nal_type = (src_nal_type & 0x7e) >> 1;
            if (0 == nal_type) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "mpegts module: h265 hander| h265 NAL type reparse error");
                return NGX_OK;
            }

#define HEVC_NAL_AUD_LENGTH 0
            if (out.end - out.last < (ngx_int_t) (len + HEVC_NAL_AUD_LENGTH)) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "mpegts module: h265 hander| not enough buffer for NAL");
                return NGX_OK;
            }
#if 1
            if (ngx_mpegts_live_append_hevc_aud(s, &out) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "mpegts module: h265 hander| error appending AUD NAL");
            }
#endif
            /* back to 00 00 01 nal_type*/
            p = p - 4;
            if (ngx_mpegts_live_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            out.last += (len - 1);
            break;
        }

NAL_TRAIL_N:

        ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "mpegts module: h265 hander| h265 NAL type=%ui, len=%uD",
            (ngx_uint_t) nal_type, len);

        /*
         *  NAL_VPS 32
         *  NAL_SPS 33
         *  NAL_PPS 34
         *  NAL_AUD 35
         *  NAL_SEI_PREFIX 39
         *  NAL_SEI_SUFFIX 40
         */
        if ((nal_type >= 32 && nal_type <= 35)
            || nal_type == 39 || nal_type == 40)
        {
            if (out.end - out.last < (5 + len -1)) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "hls: not enough buffer for AnnexB prefix");
                return NGX_OK;
            }
            if (32 == nal_type) {
                ++vps_copy;
                if(!aud_sent){
                    if (ngx_mpegts_live_append_hevc_aud(s, &out) != NGX_OK) {
                        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                      "hls: error appending AUD NAL");
                    }
                    aud_sent = 1;
                }
            } else if (33 == nal_type) {
                ++sps_copy;
            } else if (34 == nal_type) {
                ++pps_copy;
            } else if (35 == nal_type) {
                aud_sent = 1;
            }

            *out.last++ = 0;
            *out.last++ = 0;
            *out.last++ = 0;
            *out.last++ = 1;
            *out.last++ = src_nal_type;
            if (ngx_mpegts_live_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
                return NGX_ERROR;
            }
            out.last += (len - 1);
            continue;
        }

        if (vps_copy > 0 && sps_copy > 0 && pps_copy > 0) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "mpegts vps_copy %i, sps_copy %i, pps_copy %i\n",
                        vps_copy, sps_copy, pps_copy);
            sps_pps_sent = 1;
        }

        if (!aud_sent) {
            if (35 == nal_type) {
                aud_sent = 1;
            } else if (!sps_pps_sent) {
                if (ngx_mpegts_live_append_hevc_vps_sps_pps(s, &out) != NGX_OK)
                {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                      "hls: error appending AUD NAL");
                }
                aud_sent = 1;
            }
        }

        if (IS_IRAP(nal_type)) {
            if (!sps_pps_sent) {
                if (ngx_mpegts_live_append_hevc_vps_sps_pps(s, &out) != NGX_OK)
                {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                  "hls: error appenging VPS/SPS/PPS NALs");
                }
                sps_pps_sent = 1;
            }
        }

        /* AnnexB prefix */
        if (out.end - out.last < 5) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "hls: not enough buffer for AnnexB prefix");
            return NGX_OK;
        }

        /* first AnnexB prefix is long (4 bytes) */

        if (out.last == out.pos) {
            *out.last++ = 0;
        }

        *out.last++ = 0;
        *out.last++ = 0;
        *out.last++ = 1;
        *out.last++ = src_nal_type;

        /* NAL body */
        if (out.end - out.last < (ngx_int_t) len) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "hls: not enough buffer for NAL");
            return NGX_OK;
        }

        if (ngx_mpegts_live_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        out.last += (len - 1);
    }

    frame = ngx_rtmp_shared_alloc_mpegts_frame(NULL, 0);
    if (frame == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "rtmp-mpegts: h265_handler| "
                      "memory error, alloc mpegts frame failed");
        return NGX_ERROR;
    }

    /* continuity counter */
    frame->cc = ctx->video_cc;
    frame->dts = (uint64_t) h->timestamp * 90;
    /* pts = dts + composition time */
    frame->pts = frame->dts + cts * 90;
    /* program id */
    frame->pid = 0x100;
    /* stream id, video range from 0xe0 to 0xef */
    frame->sid = 0xe0;
    frame->key = (ftype == 1);
    frame->type = NGX_MPEGTS_MSG_VIDEO;

    /*
     * start new fragment if
     * - we have video key frame AND
     * - we have audio buffered or have no audio at all or stream is closed
     */
    if (ctx->aframe && ctx->aframe->last > ctx->aframe->pos &&
        ctx->aframe_pts + (uint64_t) ctx->audio_delay * 90  < frame->dts)
    {
        ngx_mpegts_live_flush_audio(s);
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: h265_handler| video pts=%uL, dts=%uL",
                   frame->pts, frame->dts);

    if (ngx_mpegts_live_shared_append_chain(frame, &out, 1) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "rtmp-mpegts: h264_handler| video frame failed");
    } else {
        ctx->video_cc = frame->cc;
        ngx_mpegts_live_video_filter(s, frame);
    }

    ngx_rtmp_shared_free_mpegts_frame(frame);

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_h264_handler(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *f)
{
    ngx_rtmp_header_t            *h;
    ngx_chain_t                  *in;
    ngx_mpegts_live_ctx_t         *ctx;
    u_char                       *p;
    uint8_t                       fmt, ftype, htype, nal_type, src_nal_type;
    uint32_t                      len, rlen;
    ngx_buf_t                     out;
    uint32_t                      cts;
    ngx_mpegts_frame_t           *frame;
    ngx_uint_t                    nal_bytes;
    ngx_int_t                     aud_sent, sps_pps_sent;
    u_char                       *buffer;
    ngx_mpegts_live_app_conf_t    *macf;
    ngx_int_t                     rc;

    h = &f->hdr;
    in = f->chain;

    macf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_live_module);
    buffer = macf->packet_buffer;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx == NULL || h->mlen < 1) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "mpegts-mux: h264_handler| "
                       "resource error, mpegts_ctx=%p, h->mlen=%d",
                       ctx, h->mlen);
        return NGX_OK;
    }

    if (ctx->avc_codec == NULL) {
        rc = ngx_mpegts_live_init_avc_codec(s);
        if (rc == NGX_ERROR) {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "mpegts-mux: h264_handler| init avc_codec failed");
            return NGX_OK;
        } else if (rc == NGX_AGAIN) {
            return NGX_OK;
        }
    }

    /* H264 is supported */
    if (ctx->avc_codec->video_codec_id != NGX_RTMP_VIDEO_H264) {
        return NGX_OK;
    }

    p = in->buf->pos;
    if (ngx_mpegts_live_copy(s, &fmt, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* 1: keyframe (IDR)
     * 2: inter frame
     * 3: disposable inter frame */

    ftype = (fmt & 0xf0) >> 4;

    /* H264 HDR/PICT */

    if (ngx_mpegts_live_copy(s, &htype, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* proceed only with PICT */

    if (htype != 1) {
        return NGX_OK;
    }

    /* 3 bytes: decoder delay */

    if (ngx_mpegts_live_copy(s, &cts, &p, 3, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    cts = ((cts & 0x00FF0000) >> 16) | ((cts & 0x000000FF) << 16) |
          (cts & 0x0000FF00);

    ngx_memzero(&out, sizeof(out));

    out.start = buffer;
    out.end = buffer + NGX_RTMP_MPEG_BUFSIZE;
    out.pos = out.start;
    out.last = out.pos;

    nal_bytes = ctx->avc_codec->avc_nal_bytes;
    aud_sent = 0;
    sps_pps_sent = 0;

    while (in) {
        if (ngx_mpegts_live_copy(s, &rlen, &p, nal_bytes, &in) != NGX_OK) {
            return NGX_OK;
        }

        len = 0;
        ngx_rtmp_rmemcpy(&len, &rlen, nal_bytes);

        if (len == 0) {
            continue;
        }

        if (ngx_mpegts_live_copy(s, &src_nal_type, &p, 1, &in) != NGX_OK) {
            return NGX_OK;
        }

        nal_type = src_nal_type & 0x1f;

        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "mpegts-mux: h264_handler| h264 NAL type=%ui, len=%uD",
                       (ngx_uint_t) nal_type, len);

        if (nal_type >= 7 && nal_type <= 9) {
            if (ngx_mpegts_live_copy(s, NULL, &p, len - 1, &in) != NGX_OK) {
                return NGX_ERROR;
            }
            continue;
        }

        if (!aud_sent) {
            switch (nal_type) {
                case 1:
                case 5:
                case 6:
                    if (ngx_mpegts_live_append_aud(s, &out) != NGX_OK) {
                        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                            "mpegts-mux: h264_handler| error appending AUD NAL");
                    }
                    aud_sent = 1;
                    break;

                case 9:
                    aud_sent = 1;
                    break;
            }
        }

        switch (nal_type) {
            case 1:
                sps_pps_sent = 0;
                break;
            case 5:
                if (sps_pps_sent) {
                    break;
                }
                if (ngx_mpegts_live_append_sps_pps(s, &out) != NGX_OK) {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "mpegts-mux: h264_handler| error appenging SPS/PPS NALs");
                }
                sps_pps_sent = 1;
                break;
        }

        /* AnnexB prefix */

        if (out.end - out.last < 5) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "mpegts-mux: h264_handler| not enough buffer for AnnexB prefix");
            return NGX_OK;
        }

        /* first AnnexB prefix is long (4 bytes) */

        if (out.last == out.pos) {
            *out.last++ = 0;
        }

        *out.last++ = 0;
        *out.last++ = 0;
        *out.last++ = 1;
        *out.last++ = src_nal_type;

        /* NAL body */

        if (out.end - out.last < (ngx_int_t) len) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "mpegts-mux: h264_handler| not enough buffer for NAL");
            return NGX_OK;
        }

        if (ngx_mpegts_live_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        out.last += (len - 1);
    }

    frame = ngx_rtmp_shared_alloc_mpegts_frame(NULL, 0);
    if (frame == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
            "mpegts-mux: h264_handler| "
            "memory error, alloc mpegts frame failed");
        return NGX_ERROR;
    }

    frame->cc = ctx->video_cc;
    frame->dts = (uint64_t) h->timestamp * 90;
    frame->pts = frame->dts + (cts * 90);
    frame->pid = 0x100;
    frame->sid = 0xe0;
    frame->key = (ftype == 1);
    frame->type = NGX_MPEGTS_MSG_VIDEO;

    if (ctx->aframe && ctx->aframe->last > ctx->aframe->pos &&
        ctx->aframe_pts + (uint64_t) ctx->audio_delay * 90  < frame->dts)
    {
        ngx_mpegts_live_flush_audio(s);
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                    "mpegts-mux: h264_handler| video pts=%uL, dts=%uL",
                    frame->pts/90, frame->dts/90);

    if (ngx_mpegts_live_shared_append_chain(frame, &out, 1) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "mpegts-mux: h264_handler| video frame failed");
    } else {
        ctx->video_cc = frame->cc;
        ngx_mpegts_live_video_filter(s, frame);
    }

    ngx_rtmp_shared_free_mpegts_frame(frame);

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_parse_aac_header(ngx_rtmp_session_t *s, ngx_uint_t *objtype,
    ngx_uint_t *srindex, ngx_uint_t *chconf)
{
    ngx_mpegts_live_ctx_t  *ctx;
    ngx_chain_t            *cl;
    u_char                 *p, b0, b1;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);

    cl = ctx->aac_codec->aac_header->chain;

    p = cl->buf->pos;

    if (ngx_mpegts_live_copy(s, NULL, &p, 2, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_mpegts_live_copy(s, &b0, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_mpegts_live_copy(s, &b1, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    *objtype = b0 >> 3;
    if (*objtype == 0 || *objtype == 0x1f) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: parse_aac_header| "
                       "unsupported adts object type:%ui", *objtype);
        return NGX_ERROR;
    }

    if (*objtype > 4) {

        /*
         * Mark all extended profiles as LC
         * to make Android as happy as possible.
         */

        *objtype = 2;
    }

    *srindex = ((b0 << 1) & 0x0f) | ((b1 & 0x80) >> 7);
    if (*srindex == 0x0f) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                       "rtmp-mpegts: parse_aac_header| "
                       "unsupported adts sample rate:%ui", *srindex);
        return NGX_ERROR;
    }

    *chconf = (b1 >> 3) & 0x0f;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: parse_aac_header| "
                   "aac object_type:%ui, sample_rate_index:%ui, "
                   "channel_config:%ui", *objtype, *srindex, *chconf);

    return NGX_OK;
}


static ngx_int_t
ngx_mpegts_live_aac_handler(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *f)
{
    ngx_rtmp_header_t              *h;
    ngx_chain_t                    *in;
    ngx_mpegts_live_ctx_t          *ctx;
    uint64_t                        pts, est_pts;
    int64_t                         dpts;
    size_t                          bsize;
    ngx_buf_t                      *b;
    u_char                         *p;
    ngx_uint_t                      objtype, srindex, chconf, size;
    ngx_int_t                       rc;

    h = &f->hdr;
    in = f->chain;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);

    if (ctx == NULL || h->mlen < 2) {
        return NGX_OK;
    }

    if (ctx->aac_codec == NULL) {
        rc = ngx_mpegts_live_init_aac_codec(s);
        if (rc == NGX_ERROR) {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                    "rtmp-mpegts: aac_handler| init aac_codec failed");
            return NGX_OK;
        } else if(rc == NGX_AGAIN) {
            return NGX_OK;
        }
    }

    if (ngx_rtmp_is_codec_header(in)) {
        return NGX_OK;
    }

    b = ctx->aframe;

    if (b == NULL) {

        b = ngx_pcalloc(s->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return NGX_ERROR;
        }

        ctx->aframe = b;

        b->start = ngx_palloc(s->pool, ctx->audio_buffer_size);
        if (b->start == NULL) {
            return NGX_ERROR;
        }

        b->end = b->start + ctx->audio_buffer_size;
        b->pos = b->last = b->start;
    }

    size = h->mlen - 2 + 7;
    pts = (uint64_t) h->timestamp * 90;

    if (b->start + size > b->end) {
        return NGX_OK;
    }

    if (b->last > b->pos &&
        ctx->aframe_pts + (uint64_t) ctx->audio_delay * 90 / 2 < pts)
    {
        ngx_mpegts_live_flush_audio(s);
    }

    if (b->last + size > b->end) {
        ngx_mpegts_live_flush_audio(s);
    }

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
        "rtmp-mpegts: aac_handler| audio pts=%uL", pts);

    if (b->last + 7 > b->end) {
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "rtmp-mpegts: aac_handler| not enough buffer for audio header");
        return NGX_OK;
    }

    p = b->last;
    b->last += 5;

    /* copy payload */

    for (; in && b->last < b->end; in = in->next) {

        bsize = in->buf->last - in->buf->pos;
        if (b->last + bsize > b->end) {
            bsize = b->end - b->last;
        }

        b->last = ngx_cpymem(b->last, in->buf->pos, bsize);
    }

    /* make up ADTS header */

    if (ngx_mpegts_live_parse_aac_header(s, &objtype, &srindex, &chconf)
        != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "rtmp-mpegts: aac_handler| aac header error");
        return NGX_OK;
    }

    /* we have 5 free bytes + 2 bytes of RTMP frame header */

    p[0] = 0xff;
    p[1] = 0xf1;
    p[2] = (u_char) (((objtype - 1) << 6) | (srindex << 2) |
                     ((chconf & 0x04) >> 2));
    p[3] = (u_char) (((chconf & 0x03) << 6) | ((size >> 11) & 0x03));
    p[4] = (u_char) (size >> 3);
    p[5] = (u_char) ((size << 5) | 0x1f);
    p[6] = 0xfc;

    if (p != b->start) {
        ctx->aframe_num++;
        return NGX_OK;
    }

    ctx->aframe_pts = pts;

    if (!ctx->sync || ctx->aac_codec->sample_rate == 0) {
        return NGX_OK;
    }

    /* align audio frames */

    /* TODO: We assume here AAC frame size is 1024
     *       Need to handle AAC frames with frame size of 960 */

    est_pts = ctx->aframe_base + ctx->aframe_num * 90000 * 1024 /
                                 ctx->aac_codec->sample_rate;
    dpts = (int64_t) (est_pts - pts);

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: aac_handler| audio sync dpts=%L (%.5fs)",
                   dpts, dpts / 90000.);

    if (dpts <= (int64_t) ctx->sync * 90 &&
        dpts >= (int64_t) ctx->sync * -90)
    {
        ctx->aframe_num++;
        ctx->aframe_pts = est_pts;
        return NGX_OK;
    }

    ctx->aframe_base = pts;
    ctx->aframe_num  = 1;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                   "rtmp-mpegts: aac_handler| audio sync gap dpts=%L (%.5fs)",
                   dpts, dpts / 90000.);

    return NGX_OK;
}

static ngx_int_t
ngx_mpegts_live_av(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
                 ngx_chain_t *in)
{
    ngx_mpegts_live_ctx_t    *ctx;
    ngx_rtmp_frame_t          frame;
    ngx_rtmp_codec_ctx_t     *codec_ctx;
    ngx_rtmp_core_app_conf_t *cacf;

    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx == NULL || codec_ctx == NULL || codec_ctx->avc_header == NULL) {
        return NGX_OK;
    }

    /* Only H264 and H265 is supported */
    if (codec_ctx->video_codec_id != NGX_RTMP_VIDEO_H264 &&
        codec_ctx->video_codec_id != cacf->hevc_codec)
    {
        return NGX_OK;
    }

    if (s->pause) {
        return NGX_OK;
    }

    ngx_memzero(&frame, sizeof(frame));

    frame.hdr = *h;
    frame.chain = in;

    ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "mpegts-mux: av| pts[%uL] type [%d] key[%d]",
            frame.hdr.timestamp, frame.hdr.type, frame.keyframe);

    switch (frame.hdr.type) {
    case NGX_RTMP_MSG_AUDIO:
        // only aac, for now
        ngx_mpegts_live_aac_handler(s, &frame);
        break;

    case NGX_RTMP_MSG_VIDEO:
        /* h264 h265 */
        if (codec_ctx->video_codec_id == NGX_RTMP_VIDEO_H264) {
            ngx_mpegts_live_h264_handler(s, &frame);
        } else if (codec_ctx->video_codec_id == cacf->hevc_codec) {
            ngx_mpegts_live_h265_handler(s, &frame);
        }
        break;

    default:
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "rtmp-mpegts: av| unknown frame-type=%d", frame.hdr.type);
        break;
    }

    return NGX_OK;
}

ngx_int_t
ngx_mpegts_live_video_filter(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    return ngx_mpegts_video(s, frame);
}

ngx_int_t
ngx_mpegts_live_audio_filter(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    return ngx_mpegts_audio(s, frame);
}

static ngx_int_t
ngx_mpegts_live_avframe(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame)
{
    ngx_mpegts_live_ctx_t   *cctx;
    ngx_rtmp_session_t      *ss;

    for (cctx = s->live_stream->mpegts_ctx; cctx; cctx = cctx->next) {
        ss = cctx->session;
        ngx_mpegts_gop_send(s, ss);

        if (!s->connection->write->active) {
            ngx_post_event(s->connection->write, &ngx_posted_events);
        }
    }

    return NGX_OK;
}

static ngx_int_t
ngx_mpegts_live_ctx_init(ngx_rtmp_session_t *s)
{
    ngx_mpegts_live_app_conf_t         *macf;
    ngx_mpegts_live_ctx_t              *ctx;

    macf = ngx_rtmp_get_module_app_conf(s, ngx_mpegts_live_module);
    if (macf == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "rtmp-mpegts: ctx_init| get app conf failed");
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_live_ctx_t));
        if (ctx == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "rtmp-mpegts: ctx_init| pcalloc ctx failed");
            return NGX_ERROR;
        }

        ngx_rtmp_set_ctx(s, ctx, ngx_mpegts_live_module);
        ctx->session = s;
    }

    ctx->sync = macf->sync;
    ctx->audio_buffer_size = macf->audio_buffer_size;
    ctx->audio_delay = macf->audio_delay;
    ctx->out_queue = macf->out_queue;

    return NGX_OK;
}

static ngx_int_t
ngx_mpegts_live_join(ngx_rtmp_session_t *s, u_char *name, unsigned publisher)
{
    ngx_mpegts_live_ctx_t        *ctx;
    ngx_live_stream_t            *st;
    ngx_rtmp_live_app_conf_t     *lacf;

    lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_live_module);
    if (lacf == NULL) {
        return NGX_ERROR;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx && ctx->stream) {
        ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "mpegts-live: join| already joined");
        return NGX_ERROR;
    }

    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_mpegts_live_ctx_t));
        if (ctx == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "mpegts-live: join| pcalloc ctx failed");
            return NGX_ERROR;
        }

        ngx_rtmp_set_ctx(s, ctx, ngx_mpegts_live_module);
    }

    ngx_memzero(ctx, sizeof(*ctx));

    ctx->session = s;

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "mpegts-live: join| join '%s'", name);

    st = s->live_stream;

    if (!(publisher || st->publish_ctx || lacf->idle_streams)) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "mpegts-live: join| stream not found");

        s->status = 404;

        return NGX_ERROR;
    }

    ctx->stream = st;
    ctx->next = st->mpegts_ctx;

    st->mpegts_ctx = ctx;

    return NGX_OK;
}

static ngx_int_t
ngx_mpegts_live_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_mpegts_live_ctx_init(s);

    return next_publish(s, v);
}

static ngx_int_t
ngx_mpegts_live_play(ngx_rtmp_session_t *s, ngx_rtmp_play_t *v)
{
    if (s->live_type != NGX_MPEGTS_LIVE) {
        goto next;
    }

    if (ngx_mpegts_live_join(s, v->name, 0) == NGX_ERROR) {
        return NGX_ERROR;
    }

next:
    return next_play(s, v);
}

static ngx_int_t
ngx_mpegts_live_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_mpegts_live_ctx_t      *ctx, **cctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_mpegts_live_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->stream == NULL) {
        ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "mpegts-live: close_stream| not joined");
        goto next;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "mpegts-live: close_stream| leave '%s'", ctx->stream->name);

    for (cctx = &ctx->stream->mpegts_ctx; *cctx; cctx = &(*cctx)->next) {
        if (*cctx == ctx) {
            *cctx = ctx->next;
            break;
        }
    }

    if (ctx->stream->mpegts_ctx) {
        ctx->stream = NULL;
        goto next;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "mpegts-live: close_stream| delete empty stream '%s'",
                   ctx->stream->name);

    ctx->stream = NULL;

next:
    return next_close_stream(s, v);
}

static ngx_int_t
ngx_mpegts_live_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t          *cmcf;
    ngx_rtmp_handler_pt                *h;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    /* register raw event handlers */

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_mpegts_live_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_mpegts_live_av;

    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_mpegts_live_publish;

    next_play = ngx_rtmp_play;
    ngx_rtmp_play = ngx_mpegts_live_play;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_mpegts_live_close_stream;

    ngx_mpegts_video = ngx_mpegts_live_avframe;
    ngx_mpegts_audio = ngx_mpegts_live_avframe;

    return NGX_OK;
}

