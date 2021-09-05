/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_LIVE_RECORD_H_INCLUDED_
#define _NGX_LIVE_RECORD_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "hls/ngx_rtmp_mpegts.h"


typedef struct {
    unsigned                    open; /* 0 close, 1 open, 2 wait for key */

    time_t                      last_time;

    ngx_file_t                  index;

    ngx_rtmp_mpegts_file_t      ts;
    ngx_file_t                  file;

    ngx_rtmp_publish_t          pubv;

    ngx_uint_t                  audio_cc;
    ngx_uint_t                  video_cc;

    ngx_msec_t                  begintime;
    ngx_msec_t                  starttime;
    ngx_msec_t                  endtime;
    off_t                       startsize;
    off_t                       endsize;

    ngx_msec_t                  publish_epoch;
    ngx_msec_t                  basetime;
} ngx_live_record_ctx_t;


typedef ngx_int_t (*ngx_live_record_start_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_live_record_update_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_live_record_done_pt)(ngx_rtmp_session_t *s);


extern ngx_live_record_start_pt     ngx_live_record_start;
extern ngx_live_record_update_pt    ngx_live_record_update;
extern ngx_live_record_done_pt      ngx_live_record_done;


extern ngx_module_t                 ngx_live_record_module;


const char *ngx_live_record_open(ngx_rtmp_session_t *s);
const char *ngx_live_record_close(ngx_rtmp_session_t *s);


#endif
