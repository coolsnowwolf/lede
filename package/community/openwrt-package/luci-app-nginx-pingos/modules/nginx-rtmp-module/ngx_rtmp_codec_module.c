
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp_codec_module.h"
#include "ngx_rtmp_live_module.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_bitop.h"
#include "ngx_rbuf.h"


#define NGX_RTMP_CODEC_META_OFF     0
#define NGX_RTMP_CODEC_META_ON      1
#define NGX_RTMP_CODEC_META_COPY    2


static void * ngx_rtmp_codec_create_app_conf(ngx_conf_t *cf);
static char * ngx_rtmp_codec_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_rtmp_codec_postconfiguration(ngx_conf_t *cf);
static ngx_int_t ngx_rtmp_codec_reconstruct_meta(ngx_rtmp_session_t *s);
static ngx_int_t ngx_rtmp_codec_copy_meta(ngx_rtmp_session_t *s,
       ngx_rtmp_header_t *h, ngx_chain_t *in);
static ngx_int_t ngx_rtmp_codec_prepare_meta(ngx_rtmp_session_t *s,
       uint32_t timestamp);
static void ngx_rtmp_codec_parse_aac_header(ngx_rtmp_session_t *s,
       ngx_chain_t *in);
static void ngx_rtmp_codec_parse_avc_header(ngx_rtmp_session_t *s,
       ngx_chain_t *in);
static void ngx_rtmp_codec_parse_hevc_header(ngx_rtmp_session_t *s,
       ngx_chain_t *in);
#if (NGX_DEBUG)
static void ngx_rtmp_codec_dump_header(ngx_rtmp_session_t *s, const char *type,
       ngx_chain_t *in);
#endif


typedef struct {
    ngx_uint_t                      meta;
} ngx_rtmp_codec_app_conf_t;


static ngx_conf_enum_t ngx_rtmp_codec_meta_slots[] = {
    { ngx_string("off"),            NGX_RTMP_CODEC_META_OFF  },
    { ngx_string("on"),             NGX_RTMP_CODEC_META_ON   },
    { ngx_string("copy"),           NGX_RTMP_CODEC_META_COPY },
    { ngx_null_string,              0 }
};


static ngx_command_t  ngx_rtmp_codec_commands[] = {

    { ngx_string("meta"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_codec_app_conf_t, meta),
      &ngx_rtmp_codec_meta_slots },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_codec_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_codec_postconfiguration,       /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_rtmp_codec_create_app_conf,         /* create app configuration */
    ngx_rtmp_codec_merge_app_conf           /* merge app configuration */
};


ngx_module_t  ngx_rtmp_codec_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_codec_module_ctx,             /* module context */
    ngx_rtmp_codec_commands,                /* module directives */
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


static const char *
audio_codecs[] = {
    "",
    "ADPCM",
    "MP3",
    "LinearLE",
    "Nellymoser16",
    "Nellymoser8",
    "Nellymoser",
    "G711A",
    "G711U",
    "",
    "AAC",
    "Speex",
    "",
    "",
    "MP3-8K",
    "DeviceSpecific",
    "Uncompressed"
};


static const char *
video_codecs[] = {
    "",
    "Jpeg",
    "Sorenson-H263",
    "ScreenVideo",
    "On2-VP6",
    "On2-VP6-Alpha",
    "ScreenVideo2",
    "H264",
};


u_char *
ngx_rtmp_get_audio_codec_name(ngx_uint_t id)
{
    return (u_char *)(id < sizeof(audio_codecs) / sizeof(audio_codecs[0])
        ? audio_codecs[id]
        : "");
}


u_char *
ngx_rtmp_get_video_codec_name(ngx_uint_t id)
{
    return (u_char *)(id < sizeof(video_codecs) / sizeof(video_codecs[0])
        ? video_codecs[id]
        : "");
}


static ngx_uint_t
ngx_rtmp_codec_get_next_version()
{
    ngx_uint_t          v;
    static ngx_uint_t   version;

    do {
        v = ++version;
    } while (v == 0);

    return v;
}


static ngx_int_t
ngx_rtmp_codec_disconnect(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_rtmp_codec_ctx_t               *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    if (ctx->avc_header) {
        ngx_rtmp_shared_free_frame(ctx->avc_header);
        ctx->avc_header = NULL;
    }

    if (ctx->aac_header) {
        ngx_rtmp_shared_free_frame(ctx->aac_header);
        ctx->aac_header = NULL;
    }

    if (ctx->meta) {
        ngx_rtmp_shared_free_frame(ctx->meta);
        ctx->meta = NULL;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_codec_av(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_rtmp_core_srv_conf_t           *cscf;
    ngx_rtmp_core_app_conf_t           *cacf;
    ngx_rtmp_codec_ctx_t               *ctx;
    ngx_rtmp_frame_t                  **header;
    uint8_t                             fmt;
    u_char                              frametype;
    static ngx_uint_t                   sample_rates[] =
                                        { 5512, 11025, 22050, 44100 };

    if (h->type != NGX_RTMP_MSG_AUDIO && h->type != NGX_RTMP_MSG_VIDEO) {
        return NGX_OK;
    }

    if (h->type == NGX_RTMP_MSG_VIDEO) {
        frametype = in->buf->pos[0] & 0xf0;
        if (frametype != 0x10 && frametype != 0x20) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "codec: receive unkwnon frametype %02xD", frametype);
            return NGX_OK;
        }
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_codec_ctx_t));
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_codec_module);
    }

    /* save codec */
    if (in->buf->last - in->buf->pos < 1) {
        return NGX_OK;
    }

    fmt =  in->buf->pos[0];
    if (h->type == NGX_RTMP_MSG_AUDIO) {
        ctx->audio_codec_id = (fmt & 0xf0) >> 4;
        ctx->audio_channels = (fmt & 0x01) + 1;
        ctx->sample_size = (fmt & 0x02) ? 2 : 1;

        if (ctx->sample_rate == 0) {
            ctx->sample_rate = sample_rates[(fmt & 0x0c) >> 2];
        }
        s->acodec = ctx->audio_codec_id;
    } else {
        ctx->video_codec_id = (fmt & 0x0f);
        s->vcodec = ctx->video_codec_id;
    }

    /* save AVC/AAC header */
    if (in->buf->last - in->buf->pos < 3) {
        return NGX_OK;
    }

    /* no conf */
    if (!ngx_rtmp_is_codec_header(in)) {
        return NGX_OK;
    }

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);
    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);
    header = NULL;

    if (h->type == NGX_RTMP_MSG_AUDIO) {
        if (ctx->audio_codec_id == NGX_RTMP_AUDIO_AAC) {
            header = &ctx->aac_header;
            ngx_rtmp_codec_parse_aac_header(s, in);
        }
    } else {
        if (ctx->video_codec_id == NGX_RTMP_VIDEO_H264) {
            header = &ctx->avc_header;
            ngx_rtmp_codec_parse_avc_header(s, in);
        } else if (ctx->video_codec_id == cacf->hevc_codec) {
            header = &ctx->avc_header;
            ngx_rtmp_codec_parse_hevc_header(s, in);
        }
    }

    if (header == NULL) {
        return NGX_OK;
    }

    if (*header) {
        ngx_rtmp_shared_free_frame(*header);
    }

    *header = ngx_rtmp_shared_alloc_frame(cscf->chunk_size, in, 0);

    return NGX_OK;
}


static void
ngx_rtmp_codec_parse_aac_header(ngx_rtmp_session_t *s, ngx_chain_t *in)
{
    ngx_uint_t              idx;
    ngx_rtmp_codec_ctx_t   *ctx;
    ngx_rtmp_bit_reader_t   br;

    static ngx_uint_t      aac_sample_rates[] =
        { 96000, 88200, 64000, 48000,
          44100, 32000, 24000, 22050,
          16000, 12000, 11025,  8000,
           7350,     0,     0,     0 };

#if (NGX_DEBUG)
    ngx_rtmp_codec_dump_header(s, "aac", in);
#endif

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    ngx_rtmp_bit_init_reader(&br, in->buf->pos, in->buf->last);

    ngx_rtmp_bit_read(&br, 16);

    ctx->aac_profile = (ngx_uint_t) ngx_rtmp_bit_read(&br, 5);
    if (ctx->aac_profile == 31) {
        ctx->aac_profile = (ngx_uint_t) ngx_rtmp_bit_read(&br, 6) + 32;
    }

    idx = (ngx_uint_t) ngx_rtmp_bit_read(&br, 4);
    if (idx == 15) {
        ctx->sample_rate = (ngx_uint_t) ngx_rtmp_bit_read(&br, 24);
    } else {
        ctx->sample_rate = aac_sample_rates[idx];
    }

    ctx->aac_chan_conf = (ngx_uint_t) ngx_rtmp_bit_read(&br, 4);

    if (ctx->aac_profile == 5 || ctx->aac_profile == 29) {
        
        if (ctx->aac_profile == 29) {
            ctx->aac_ps = 1;
        }

        ctx->aac_sbr = 1;

        idx = (ngx_uint_t) ngx_rtmp_bit_read(&br, 4);
        if (idx == 15) {
            ctx->sample_rate = (ngx_uint_t) ngx_rtmp_bit_read(&br, 24);
        } else {
            ctx->sample_rate = aac_sample_rates[idx];
        }

        ctx->aac_profile = (ngx_uint_t) ngx_rtmp_bit_read(&br, 5);
        if (ctx->aac_profile == 31) {
            ctx->aac_profile = (ngx_uint_t) ngx_rtmp_bit_read(&br, 6) + 32;
        }
    }

    /* MPEG-4 Audio Specific Config

       5 bits: object type
       if (object type == 31)
         6 bits + 32: object type
       4 bits: frequency index
       if (frequency index == 15)
         24 bits: frequency
       4 bits: channel configuration

       if (object_type == 5)
           4 bits: frequency index
           if (frequency index == 15)
             24 bits: frequency
           5 bits: object type
           if (object type == 31)
             6 bits + 32: object type

       var bits: AOT Specific Config
     */

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "codec: aac header profile=%ui, "
                   "sample_rate=%ui, chan_conf=%ui",
                   ctx->aac_profile, ctx->sample_rate, ctx->aac_chan_conf);
}


/*
 * ITU-T H.265 7.3.1 General NAL unit syntax
 */
static ngx_int_t
ngx_rtmp_codec_parse_hevc_nal_to_rbsp(ngx_rtmp_session_t *s, u_char *p,
        ngx_rtmp_bit_reader_t *br, ngx_uint_t nal_unit_type,
        ngx_uint_t nal_unit_len)
{
    ngx_uint_t                  i, count, rbsp_bytes;

    /*
     * nal_unit
     *      nal_unit_header()
     *      NumBytesInRbsp = 0
     *      for (i = 2; i < NumBytesInNalUnit; i++)
     *          if (i + 2 < NumBytesInNalUnit && next_bits(24) == 0x000003) {
     *              rbsp_byte[NumBytesInRbsp++]
     *              rbsp_byte[NumBytesInRbsp++]
     *              i += 2
     *              emulation_prevention_three_byte // equal to 0x03
     *          } else
     *              rbsp_byte[NumBytesInRbsp++]
     *
     * nal_unit_header
     *      forbidden_zero_bit                      1 bit
     *      nal_unit_type                           6 bits
     *      nuh_layer_id                            6 bits
     *      nuh_temporal_id_plus1                   3 bits
     *
     * ITU-T H.265 7.4.2.1
     * emulation_prevention_three_byte is a byte equal to 0x03.
     * When an emulation_prevention_three_byte is present in the NAL unit,
     * it shall be discarded by the decoding process
     *      Within the NAL unit, the following three-byte sequences shall not
     *      occur at any byte-aligned position:
     *          0x000000
     *          0x000001
     *          0x000002
     *      Within the NAL unit, any four-byte sequence that starts with
     *      0x000003 other than the following sequences shall not occur at
     *      any byte-aligned position:
     *          0x00000300
     *          0x00000301
     *          0x00000302
     *          0x00000303
     */

    ngx_rtmp_bit_read(br, 1);
    if (ngx_rtmp_bit_read(br, 6) != nal_unit_type) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "nal_unit_type not expect %ui", nal_unit_type);
        return NGX_ERROR;
    }
    ngx_rtmp_bit_read(br, 6);
    ngx_rtmp_bit_read(br, 3);

    count = 0;
    rbsp_bytes = 0;
    for (i = 0; i < nal_unit_len; ++i) {
        if (count == 2) { /* already 0x0000 */
            if (br->pos[i] < 0x03) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "three bytes sequence error");
                return NGX_ERROR;
            }

            if (br->pos[i] == 0x03 && br->pos[i + 1] > 0x03) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "four bytes sequence error");
                return NGX_ERROR;
            }

            if (br->pos[i] == 0x03) {
                count = 0;
                continue;
            }
        }

        *p++ = br->pos[i];
        ++rbsp_bytes;
        if (br->pos[i] == 0x00) {
            ++count;
        } else {
            count = 0;
        }
    }

    return rbsp_bytes;
}

/*
 * ITU-T H.265 7.3.3 Profile, tier and level syntax
 */
static void
ngx_rtmp_codec_parse_hevc_ptl(ngx_rtmp_session_t *s, ngx_rtmp_bit_reader_t *br,
        ngx_flag_t profilePresentFlag, ngx_uint_t maxNumSubLayersMinus1)
{
    ngx_uint_t                  i, slppf[8], sllpf[8];

    if (profilePresentFlag) {
        /*
         * profile_tier_level
         *      general_profile_space                       2 bits
         *      general_tier_flag                           1 bit
         *      general_profile_idc                         5 bits
         *      for (j = 0; j < 32; j++)
         *          general_profile_compatibility_flag[j]   1 bit
         *      general_progressive_source_flag             1 bit
         *      general_interlaced_source_flag              1 bit
         *      general_non_packed_constraint_flag          1 bit
         *      general_frame_only_constraint_flag          1 bit
         *
         *      general_max_12bit_constraint_flag           1 bit
         *      general_max_10bit_constraint_flag           1 bit
         *      general_max_8bit_constraint_flag            1 bit
         *      general_max_422chroma_constraint_flag       1 bit
         *      general_max_420chroma_constraint_flag       1 bit
         *      general_max_monochrome_constraint_flag      1 bit
         *      general_intra_constraint_flag               1 bit
         *      general_one_picture_only_constraint_flag    1 bit
         *      general_lower_bit_rate_constraint_flag      1 bit
         *      general_reserved_zero_34bits                34 bits
         *
         *      general_inbld_flag                          1 bit
         */
        ngx_rtmp_bit_read(br, 88);
    }

    /*
     * profile_tier_level
     *      general_level_idc                               8 bits
     */
    ngx_rtmp_bit_read(br, 8);

    /*
     * profile_tier_level
     *      for(i = 0; i < maxNumSubLayersMinus1; i++) {
     *           sub_layer_profile_present_flag[i]          1 bit
     *           sub_layer_level_present_flag[i]            1 bit
     *      }
     *
     *      if (maxNumSubLayersMinus1 > 0)
     *           for(i = maxNumSubLayersMinus1; i < 8; i++)
     *               reserved_zero_2bits[i]                 2 bits
     */
    for (i = 0; i < maxNumSubLayersMinus1; ++i) {
        slppf[i] = ngx_rtmp_bit_read(br, 1);
        sllpf[i] = ngx_rtmp_bit_read(br, 1);
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "%d sub_layer_profile_present_flag:%d, "
                "sub_layer_level_present_flag:%d", i, slppf[i], sllpf[i]);
    }

    if (maxNumSubLayersMinus1 > 0) {
        for (i = maxNumSubLayersMinus1; i < 8; ++i) {
            ngx_uint_t t = ngx_rtmp_bit_read(br, 2);
            ngx_log_error(NGX_LOG_ERR, s->log, 0, "zero bit %d", t);
        }
    }

    /*
     * profile_tier_level
     *      for (i = 0; i < maxNumSubLayersMinus1; i++) {
     *          if (sub_layer_profile_present_flag[i] {
     *                                                      44 bits
     *          }
     *          if (sub_layer_level_present_flag[i]) {
     *              sub_layer_level_idc[i]                  8 bits
     *          }
     *      }
     */
    for (i = 0; i < maxNumSubLayersMinus1; ++i) {
        if (slppf[i]) {
            ngx_rtmp_bit_read(br, 88);
        }

        if (sllpf[i]) {
            ngx_rtmp_bit_read(br, 8);
        }
    }
}

/*
 * ITU-T H.265 7.3.2.2 Sequence parameter set RBSP syntax
 */
static void
ngx_rtmp_codec_parse_hevc_sps(ngx_rtmp_session_t *s, ngx_rtmp_codec_ctx_t *ctx,
        ngx_rtmp_bit_reader_t *pbr, ngx_uint_t nal_unit_len)
{
    ngx_uint_t              mslm, psi, cfi, width, height,
                            subwidthC, subheightC,
                            cwlo, cwro, cwto, cwbo;
    ngx_rtmp_bit_reader_t   br;
    u_char                  buf[4096];
    ngx_int_t               rbsp_bytes;

    ngx_rtmp_bit_init_reader(&br, pbr->pos, pbr->pos + nal_unit_len);
    rbsp_bytes = ngx_rtmp_codec_parse_hevc_nal_to_rbsp(s, buf, &br, NAL_SPS,
                                                       nal_unit_len);
    if (rbsp_bytes == NGX_ERROR) {
        return;
    }

    ngx_rtmp_bit_init_reader(&br, buf, buf + rbsp_bytes);

    /*
     * seq_parameter_set_rbsp
     *      sps_video_parameter_set_id              4 bits
     *      sps_max_sub_layers_minus1               3 bits
     *      sps_temporal_id_nesting_flag            1 bit
     */
    ngx_rtmp_bit_read(&br, 4);
    mslm = ngx_rtmp_bit_read(&br, 3);
    ngx_rtmp_bit_read(&br, 1);

    /*
     * seq_parameter_set_rbsp
     *      profile_tier_level(1, sps_max_sub_layers_minus1)
     */
    ngx_rtmp_codec_parse_hevc_ptl(s, &br, 1, mslm);

    /* calc resolution */
    /*
     * seq_parameter_set_rbsp
     *      sps_seq_parameter_set_id                v
     *      chroma_format_idc                       v
     *      if (chroma_format_idc == 3)
     *          separate_colour_plane_flag          1 bit
     *      pic_width_in_luma_samples               v
     *      pic_height_in_luma_samples              v
     *      conformance_window_flag                 1 bit
     *      if (conformance_window_flag) {
     *          conf_win_left_offset                v
     *          conf_win_right_offset               v
     *          conf_win_top_offset                 v
     *          conf_win_bottom_offset              v
     *      }
     */
    psi = ngx_rtmp_bit_read_golomb(&br);
    if (psi > 16 || br.err) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "read sps_seq_parameter_set_id error: %ui", psi);
        return;
    }

    cfi = ngx_rtmp_bit_read_golomb(&br);
    if (cfi > 3 || br.err) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "read chroma_format_idc error: %ui", cfi);
        return;
    }

    if (cfi == 3) {
        ngx_rtmp_bit_read(&br, 1);
    }

    width = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
    if (br.err) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "read width error");
        return;
    }

    height = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
    if (br.err) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "read height error");
        return;
    }

    if (ngx_rtmp_bit_read(&br, 1)) {
        cwlo = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        cwro = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        cwto = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        cwbo = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

        /*
         * ITU-T H.265 Table 6-1
         */
        if (cfi == 1) { /* 4:2:0 */
            subwidthC = 2;
            subheightC = 2;
        } else if (cfi == 2) { /* 4:2:2 */
            subwidthC = 2;
            subheightC = 1;
        } else { /* Monochrome or 4:4:4 */
            subwidthC = 1;
            subheightC = 1;
        }

        /*
         * ITU-T H.265 7.4.3.2.1
         *
         * horizontal picture coordinates from
         *  SubWidthC * conf_win_left_offset to
         *  pic_width_in_luma_samples - (SubWidthC * conf_win_right_offset + 1)
         * vertical picture coordinates from
         *  SubHeightC * conf_win_top_offset to
         *  pic_height_in_luma_samples -
         *  (SubHeightC * conf_win_bottom_offset + 1)
         */
        ctx->width = width - (subwidthC * cwro + 1) - (subwidthC * cwlo);
        ctx->height = height - (subheightC * cwbo + 1) - (subheightC * cwto);
    } else {
        ctx->width = width;
        ctx->height = height;
    }

    return;
}

static void
ngx_rtmp_codec_parse_hevc_header(ngx_rtmp_session_t *s, ngx_chain_t *in)
{
    ngx_uint_t              i, j, num_arrays, nal_unit_type, num_nalus,
                            nal_unit_len;
    ngx_rtmp_codec_ctx_t   *ctx;
    ngx_rtmp_bit_reader_t   br;

#if (NGX_DEBUG)
    ngx_rtmp_codec_dump_header(s, "hevc", in);
#endif

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    ngx_rtmp_bit_init_reader(&br, in->buf->pos, in->buf->last);

    /*
     * FrameType                                    4 bits
     * CodecID                                      4 bits
     * AVCPacketType                                1 byte
     * CompositionTime                              3 bytes
     * HEVCDecoderConfigurationRecord
     *      configurationVersion                    1 byte
     */
    ngx_rtmp_bit_read(&br, 48);

    /*
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
    ngx_rtmp_bit_read(&br, 160);

    /*
     * HEVCDecoderConfigurationRecord
     *      constantFrameRate                       2 bits
     *      numTemporalLayers                       3 bits
     *      temporalIdNested                        1 bit
     *      lengthSizeMinusOne                      2 bits
     */
    ctx->avc_nal_bytes = (ngx_uint_t) ((ngx_rtmp_bit_read_8(&br) & 0x03) + 1);

    /*
     * HEVCDecoderConfigurationRecord
     *      numOfArrays                             1 byte
     */
    num_arrays = (ngx_uint_t) ngx_rtmp_bit_read_8(&br);

    for (i = 0; i < num_arrays; ++i) {
        /*
         * array_completeness                       1 bit
         * reserved                                 1 bit
         * NAL_unit_type                            6 bits
         * numNalus                                 2 bytes
         */
        nal_unit_type = (ngx_uint_t) (ngx_rtmp_bit_read_8(&br) & 0x3f);
        num_nalus = (ngx_uint_t) ngx_rtmp_bit_read_16(&br);

        for (j = 0; j < num_nalus; ++j) {
            /*
             * nalUnitLength                        2 bytes
             */
            nal_unit_len = (ngx_uint_t) ngx_rtmp_bit_read_16(&br);

            switch (nal_unit_type) {
            case NAL_SPS:
                ngx_rtmp_codec_parse_hevc_sps(s, ctx, &br, nal_unit_len);
                ngx_rtmp_bit_read(&br, nal_unit_len * 8);
                break;
            default:
                ngx_rtmp_bit_read(&br, nal_unit_len * 8);
                break;
            }
        }
    }

    ngx_log_debug7(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "codec: hevc header "
                   "profile=%ui, compat=%ui, level=%ui, "
                   "nal_bytes=%ui, ref_frames=%ui, width=%ui, height=%ui",
                   ctx->avc_profile, ctx->avc_compat, ctx->avc_level,
                   ctx->avc_nal_bytes, ctx->avc_ref_frames,
                   ctx->width, ctx->height);
}


static void
ngx_rtmp_codec_parse_avc_header(ngx_rtmp_session_t *s, ngx_chain_t *in)
{
    ngx_uint_t              profile_idc, width, height, crop_left, crop_right,
                            crop_top, crop_bottom, frame_mbs_only, n, cf_idc,
                            num_ref_frames;
    ngx_rtmp_codec_ctx_t   *ctx;
    ngx_rtmp_bit_reader_t   br;

#if (NGX_DEBUG)
    ngx_rtmp_codec_dump_header(s, "avc", in);
#endif

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    ngx_rtmp_bit_init_reader(&br, in->buf->pos, in->buf->last);

    ngx_rtmp_bit_read(&br, 48);

    ctx->avc_profile = (ngx_uint_t) ngx_rtmp_bit_read_8(&br);
    ctx->avc_compat = (ngx_uint_t) ngx_rtmp_bit_read_8(&br);
    ctx->avc_level = (ngx_uint_t) ngx_rtmp_bit_read_8(&br);

    /* nal bytes */
    ctx->avc_nal_bytes = (ngx_uint_t) ((ngx_rtmp_bit_read_8(&br) & 0x03) + 1);

    /* nnals */
    if ((ngx_rtmp_bit_read_8(&br) & 0x1f) == 0) {
        return;
    }

    /* nal size */
    ngx_rtmp_bit_read(&br, 16);

    /* nal type */
    if (ngx_rtmp_bit_read_8(&br) != 0x67) {
        return;
    }

    /* SPS */

    /* profile idc */
    profile_idc = (ngx_uint_t) ngx_rtmp_bit_read(&br, 8);

    /* flags */
    ngx_rtmp_bit_read(&br, 8);

    /* level idc */
    ngx_rtmp_bit_read(&br, 8);

    /* SPS id */
    ngx_rtmp_bit_read_golomb(&br);

    if (profile_idc == 100 || profile_idc == 110 ||
        profile_idc == 122 || profile_idc == 244 || profile_idc == 44 ||
        profile_idc == 83 || profile_idc == 86 || profile_idc == 118)
    {
        /* chroma format idc */
        cf_idc = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

        if (cf_idc == 3) {

            /* separate color plane */
            ngx_rtmp_bit_read(&br, 1);
        }

        /* bit depth luma - 8 */
        ngx_rtmp_bit_read_golomb(&br);

        /* bit depth chroma - 8 */
        ngx_rtmp_bit_read_golomb(&br);

        /* qpprime y zero transform bypass */
        ngx_rtmp_bit_read(&br, 1);

        /* seq scaling matrix present */
        if (ngx_rtmp_bit_read(&br, 1)) {

            for (n = 0; n < (cf_idc != 3 ? 8u : 12u); n++) {

                /* seq scaling list present */
                if (ngx_rtmp_bit_read(&br, 1)) {

                    /* TODO: scaling_list()
                    if (n < 6) {
                    } else {
                    }
                    */
                }
            }
        }
    }

    /* log2 max frame num */
    ngx_rtmp_bit_read_golomb(&br);

    /* pic order cnt type */
    switch (ngx_rtmp_bit_read_golomb(&br)) {
    case 0:

        /* max pic order cnt */
        ngx_rtmp_bit_read_golomb(&br);
        break;

    case 1:

        /* delta pic order alwys zero */
        ngx_rtmp_bit_read(&br, 1);

        /* offset for non-ref pic */
        ngx_rtmp_bit_read_golomb(&br);

        /* offset for top to bottom field */
        ngx_rtmp_bit_read_golomb(&br);

        /* num ref frames in pic order */
        num_ref_frames = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

        for (n = 0; n < num_ref_frames; n++) {

            /* offset for ref frame */
            ngx_rtmp_bit_read_golomb(&br);
        }
    }

    /* num ref frames */
    ctx->avc_ref_frames = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

    /* gaps in frame num allowed */
    ngx_rtmp_bit_read(&br, 1);

    /* pic width in mbs - 1 */
    width = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

    /* pic height in map units - 1 */
    height = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

    /* frame mbs only flag */
    frame_mbs_only = (ngx_uint_t) ngx_rtmp_bit_read(&br, 1);

    if (!frame_mbs_only) {

        /* mbs adaprive frame field */
        ngx_rtmp_bit_read(&br, 1);
    }

    /* direct 8x8 inference flag */
    ngx_rtmp_bit_read(&br, 1);

    /* frame cropping */
    if (ngx_rtmp_bit_read(&br, 1)) {

        crop_left = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        crop_right = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        crop_top = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);
        crop_bottom = (ngx_uint_t) ngx_rtmp_bit_read_golomb(&br);

    } else {

        crop_left = 0;
        crop_right = 0;
        crop_top = 0;
        crop_bottom = 0;
    }

    ctx->width = (width + 1) * 16 - (crop_left + crop_right) * 2;
    ctx->height = (2 - frame_mbs_only) * (height + 1) * 16 -
                  (crop_top + crop_bottom) * 2;

    ngx_log_debug7(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "codec: avc header "
                   "profile=%ui, compat=%ui, level=%ui, "
                   "nal_bytes=%ui, ref_frames=%ui, width=%ui, height=%ui",
                   ctx->avc_profile, ctx->avc_compat, ctx->avc_level,
                   ctx->avc_nal_bytes, ctx->avc_ref_frames,
                   ctx->width, ctx->height);
}


#if (NGX_DEBUG)
static void
ngx_rtmp_codec_dump_header(ngx_rtmp_session_t *s, const char *type,
    ngx_chain_t *in)
{
    u_char buf[256], *p, *pp;
    u_char hex[] = "0123456789abcdef";

    for (pp = buf, p = in->buf->pos;
         p < in->buf->last && pp < buf + sizeof(buf) - 1;
         ++p)
    {
        *pp++ = hex[*p >> 4];
        *pp++ = hex[*p & 0x0f];
    }

    *pp = 0;

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "codec: %s header %s", type, buf);
}
#endif


static ngx_int_t
ngx_rtmp_codec_reconstruct_meta(ngx_rtmp_session_t *s)
{
    ngx_rtmp_codec_ctx_t           *ctx;
    ngx_rtmp_core_srv_conf_t       *cscf;
    ngx_int_t                       rc;

    static struct {
        double                      width;
        double                      height;
        double                      duration;
        double                      frame_rate;
        double                      video_data_rate;
        double                      video_codec_id;
        double                      audio_data_rate;
        double                      audio_codec_id;
        u_char                      profile[32];
        u_char                      level[32];
    }                               v;

    static ngx_rtmp_amf_elt_t       out_inf[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_string("Server"),
          "PingOS (https://pingos.io)", 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("width"),
          &v.width, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("height"),
          &v.height, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("displayWidth"),
          &v.width, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("displayHeight"),
          &v.height, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("duration"),
          &v.duration, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("framerate"),
          &v.frame_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("fps"),
          &v.frame_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("videodatarate"),
          &v.video_data_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("videocodecid"),
          &v.video_codec_id, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("audiodatarate"),
          &v.audio_data_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("audiocodecid"),
          &v.audio_codec_id, 0 },

        { NGX_RTMP_AMF_STRING,
          ngx_string("profile"),
          &v.profile, sizeof(v.profile) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("level"),
          &v.level, sizeof(v.level) },
    };

    static ngx_rtmp_amf_elt_t       out_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          "onMetaData", 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          out_inf, sizeof(out_inf) },
    };

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx == NULL) {
        return NGX_OK;
    }

    if (ctx->meta) {
        ngx_rtmp_shared_free_frame(ctx->meta);
    }

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    ctx->meta = ngx_rtmp_shared_alloc_frame(cscf->chunk_size, NULL, 1);

    v.width = ctx->width;
    v.height = ctx->height;
    v.duration = ctx->duration;
    v.frame_rate = ctx->frame_rate;
    v.video_data_rate = ctx->video_data_rate;
    v.video_codec_id = ctx->video_codec_id;
    v.audio_data_rate = ctx->audio_data_rate;
    v.audio_codec_id = ctx->audio_codec_id;
    ngx_memcpy(v.profile, ctx->profile, sizeof(ctx->profile));
    ngx_memcpy(v.level, ctx->level, sizeof(ctx->level));

    rc = ngx_rtmp_append_amf(s, &ctx->meta->chain, &ctx->meta->chain, out_elts,
                             sizeof(out_elts) / sizeof(out_elts[0]));
    if (rc != NGX_OK || ctx->meta == NULL) {
        return NGX_ERROR;
    }

    return ngx_rtmp_codec_prepare_meta(s, 0);
}


static ngx_int_t
ngx_rtmp_codec_copy_meta(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_rtmp_codec_ctx_t      *ctx;
    ngx_rtmp_core_srv_conf_t  *cscf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    if (ctx->meta) {
        ngx_rtmp_shared_free_frame(ctx->meta);
    }

    ctx->meta = ngx_rtmp_shared_alloc_frame(cscf->chunk_size, in, 0);

    if (ctx->meta == NULL) {
        return NGX_ERROR;
    }

    return ngx_rtmp_codec_prepare_meta(s, h->timestamp);
}


static ngx_int_t
ngx_rtmp_codec_prepare_meta(ngx_rtmp_session_t *s, uint32_t timestamp)
{
    ngx_rtmp_codec_ctx_t  *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    ctx->meta->hdr.csid = NGX_RTMP_CSID_AMF;
    ctx->meta->hdr.msid = NGX_RTMP_MSID;
    ctx->meta->hdr.type = NGX_RTMP_MSG_AMF_META;
    ctx->meta->hdr.timestamp = timestamp;

    ctx->meta_version = ngx_rtmp_codec_get_next_version();

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_codec_meta_data(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_rtmp_codec_app_conf_t      *cacf;
    ngx_rtmp_codec_ctx_t           *ctx;
    ngx_uint_t                      skip;

    static struct {
        double                      width;
        double                      height;
        double                      duration;
        double                      frame_rate;
        double                      video_data_rate;
        double                      video_codec_id_n;
        u_char                      video_codec_id_s[32];
        double                      audio_data_rate;
        double                      audio_codec_id_n;
        u_char                      audio_codec_id_s[32];
        u_char                      profile[32];
        u_char                      level[32];
    }                               v;

    static ngx_rtmp_amf_elt_t       in_video_codec_id[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &v.video_codec_id_n, 0 },

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          &v.video_codec_id_s, sizeof(v.video_codec_id_s) },
    };

    static ngx_rtmp_amf_elt_t       in_audio_codec_id[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_null_string,
          &v.audio_codec_id_n, 0 },

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          &v.audio_codec_id_s, sizeof(v.audio_codec_id_s) },
    };

    static ngx_rtmp_amf_elt_t       in_inf[] = {

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("width"),
          &v.width, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("height"),
          &v.height, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("duration"),
          &v.duration, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("framerate"),
          &v.frame_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("fps"),
          &v.frame_rate, 0 },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("videodatarate"),
          &v.video_data_rate, 0 },

        { NGX_RTMP_AMF_VARIANT,
          ngx_string("videocodecid"),
          in_video_codec_id, sizeof(in_video_codec_id) },

        { NGX_RTMP_AMF_NUMBER,
          ngx_string("audiodatarate"),
          &v.audio_data_rate, 0 },

        { NGX_RTMP_AMF_VARIANT,
          ngx_string("audiocodecid"),
          in_audio_codec_id, sizeof(in_audio_codec_id) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("profile"),
          &v.profile, sizeof(v.profile) },

        { NGX_RTMP_AMF_STRING,
          ngx_string("level"),
          &v.level, sizeof(v.level) },
    };

    static ngx_rtmp_amf_elt_t       in_elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          NULL, 0 },

        { NGX_RTMP_AMF_OBJECT,
          ngx_null_string,
          in_inf, sizeof(in_inf) },
    };

    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_codec_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_codec_ctx_t));
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_codec_module);
    }

    ngx_memzero(&v, sizeof(v));

    /* use -1 as a sign of unchanged data;
     * 0 is a valid value for uncompressed audio */
    v.audio_codec_id_n = -1;

    /* FFmpeg sends a string in front of actal metadata; ignore it */
    skip = !(in->buf->last > in->buf->pos
            && *in->buf->pos == NGX_RTMP_AMF_STRING);
    if (ngx_rtmp_receive_amf(s, in, in_elts + skip,
                sizeof(in_elts) / sizeof(in_elts[0]) - skip))
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "codec: error parsing data frame");
        return NGX_OK;
    }

    ctx->width = (ngx_uint_t) v.width;
    ctx->height = (ngx_uint_t) v.height;
    ctx->duration = (ngx_uint_t) v.duration;
    ctx->frame_rate = v.frame_rate;
    ctx->video_data_rate = (ngx_uint_t) v.video_data_rate;
    ctx->video_codec_id = (ngx_uint_t) v.video_codec_id_n;
    ctx->audio_data_rate = (ngx_uint_t) v.audio_data_rate;
    ctx->audio_codec_id = (v.audio_codec_id_n == -1
            ? 0 : v.audio_codec_id_n == 0
            ? NGX_RTMP_AUDIO_UNCOMPRESSED : (ngx_uint_t) v.audio_codec_id_n);
    ngx_memcpy(ctx->profile, v.profile, sizeof(v.profile));
    ngx_memcpy(ctx->level, v.level, sizeof(v.level));

    ngx_log_debug8(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "codec: data frame: "
            "width=%ui height=%ui duration=%ui frame_rate=%f "
            "video=%s (%ui) audio=%s (%ui)",
            ctx->width, ctx->height, ctx->duration, ctx->frame_rate,
            ngx_rtmp_get_video_codec_name(ctx->video_codec_id),
            ctx->video_codec_id,
            ngx_rtmp_get_audio_codec_name(ctx->audio_codec_id),
            ctx->audio_codec_id);

    switch (cacf->meta) {
        case NGX_RTMP_CODEC_META_ON:
            return ngx_rtmp_codec_reconstruct_meta(s);
        case NGX_RTMP_CODEC_META_COPY:
            return ngx_rtmp_codec_copy_meta(s, h, in);
    }

    /* NGX_RTMP_CODEC_META_OFF */

    return NGX_OK;
}


static void *
ngx_rtmp_codec_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_codec_app_conf_t  *cacf;

    cacf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_codec_app_conf_t));
    if (cacf == NULL) {
        return NULL;
    }

    cacf->meta = NGX_CONF_UNSET_UINT;

    return cacf;
}


static char *
ngx_rtmp_codec_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_codec_app_conf_t *prev = parent;
    ngx_rtmp_codec_app_conf_t *conf = child;

    ngx_conf_merge_uint_value(conf->meta, prev->meta, NGX_RTMP_CODEC_META_ON);

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_rtmp_codec_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t          *cmcf;
    ngx_rtmp_handler_pt                *h;
    ngx_rtmp_amf_handler_t             *ch;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_rtmp_codec_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_rtmp_codec_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_DISCONNECT]);
    *h = ngx_rtmp_codec_disconnect;

    /* register metadata handler */
    ch = ngx_array_push(&cmcf->amf);
    if (ch == NULL) {
        return NGX_ERROR;
    }
    ngx_str_set(&ch->name, "@setDataFrame");
    ch->handler = ngx_rtmp_codec_meta_data;

    ch = ngx_array_push(&cmcf->amf);
    if (ch == NULL) {
        return NGX_ERROR;
    }
    ngx_str_set(&ch->name, "onMetaData");
    ch->handler = ngx_rtmp_codec_meta_data;


    return NGX_OK;
}
