
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */

#ifndef _NGX_RTMP_MPEGTS_MODULE_H
#define _NGX_RTMP_MPEGTS_MODULE_H

#include "ngx_rtmp.h"

typedef ngx_int_t (*ngx_mpegts_video_pt)(ngx_rtmp_session_t *s,
    ngx_mpegts_frame_t *frame);
typedef ngx_int_t (*ngx_mpegts_audio_pt)(ngx_rtmp_session_t *s,
    ngx_mpegts_frame_t *frame);

extern ngx_mpegts_video_pt ngx_mpegts_video;
extern ngx_mpegts_audio_pt ngx_mpegts_audio;

ngx_int_t
ngx_rtmp_mpegts_gen_pmt(ngx_int_t vcodec, ngx_int_t acodec,
    ngx_log_t *log, u_char *pmt);
ngx_int_t
ngx_mpegts_live_video_filter(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame);
ngx_int_t
ngx_mpegts_live_audio_filter(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame);

extern u_char ngx_rtmp_mpegts_pat[];

#endif
