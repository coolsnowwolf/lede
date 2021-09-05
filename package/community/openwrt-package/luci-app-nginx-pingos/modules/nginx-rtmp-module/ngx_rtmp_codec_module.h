
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_CODEC_H_INCLUDED_
#define _NGX_RTMP_CODEC_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"


/* hevc nal type */
enum {
    NAL_TRAIL_N    = 0,
    NAL_TRAIL_R    = 1,
    NAL_TSA_N      = 2,
    NAL_TSA_R      = 3,
    NAL_STSA_N     = 4,
    NAL_STSA_R     = 5,
    NAL_RADL_N     = 6,
    NAL_RADL_R     = 7,
    NAL_RASL_N     = 8,
    NAL_RASL_R     = 9,
    NAL_BLA_W_LP   = 16,
    NAL_BLA_W_RADL = 17,
    NAL_BLA_N_LP   = 18,
    NAL_IDR_W_RADL = 19,
    NAL_IDR_N_LP   = 20,
    NAL_CRA_NUT    = 21,
    NAL_VPS        = 32,
    NAL_SPS        = 33,
    NAL_PPS        = 34,
    NAL_AUD        = 35,
    NAL_EOS_NUT    = 36,
    NAL_EOB_NUT    = 37,
    NAL_FD_NUT     = 38,
    NAL_SEI_PREFIX = 39,
    NAL_SEI_SUFFIX = 40,
};


u_char * ngx_rtmp_get_audio_codec_name(ngx_uint_t id);
u_char * ngx_rtmp_get_video_codec_name(ngx_uint_t id);


typedef struct {
    ngx_uint_t                  width;
    ngx_uint_t                  height;
    ngx_uint_t                  duration;
    double                      frame_rate;
    ngx_uint_t                  video_data_rate;
    ngx_uint_t                  video_codec_id;
    ngx_uint_t                  audio_data_rate;
    ngx_uint_t                  audio_codec_id;
    ngx_uint_t                  aac_profile;
    ngx_uint_t                  aac_chan_conf;
    ngx_uint_t                  aac_sbr;
    ngx_uint_t                  aac_ps;
    ngx_uint_t                  avc_profile;    /* avc or hevc */
    ngx_uint_t                  avc_compat;     /* avc or hevc */
    ngx_uint_t                  avc_level;      /* avc or hevc */
    ngx_uint_t                  avc_nal_bytes;  /* avc or hevc */
    ngx_uint_t                  avc_ref_frames; /* avc or hevc */
    ngx_uint_t                  sample_rate;    /* 5512, 11025, 22050, 44100 */
    ngx_uint_t                  sample_size;    /* 1=8bit, 2=16bit */
    ngx_uint_t                  audio_channels; /* 1, 2 */
    u_char                      profile[32];
    u_char                      level[32];

    ngx_rtmp_frame_t           *avc_header;     /* avc or hevc */
    ngx_rtmp_frame_t           *aac_header;

    ngx_rtmp_frame_t           *meta;
    ngx_uint_t                  meta_version;
} ngx_rtmp_codec_ctx_t;


extern ngx_module_t  ngx_rtmp_codec_module;


#endif /* _NGX_RTMP_LIVE_H_INCLUDED_ */
