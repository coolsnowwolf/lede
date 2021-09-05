
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_LIVE_H_INCLUDED_
#define _NGX_RTMP_LIVE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_bandwidth.h"
#include "ngx_rtmp_streams.h"
#include "ngx_map.h"


typedef struct {
    unsigned                            active:1;
    uint32_t                            timestamp;
    uint32_t                            last_timestamp;
    uint32_t                            csid;
    uint32_t                            dropped;
} ngx_rtmp_live_chunk_stream_t;


struct ngx_rtmp_live_ctx_s {
    ngx_map_node_t                      node;
    ngx_int_t                           pubpri;
    ngx_rtmp_session_t                 *session;
    ngx_live_stream_t                  *stream;
    ngx_rtmp_live_ctx_t                *next;
    ngx_uint_t                          ndropped;
    ngx_rtmp_live_chunk_stream_t        cs[2];
    ngx_uint_t                          meta_version;
    ngx_event_t                         idle_evt;
    unsigned                            active:1;
    unsigned                            publishing:1;
    unsigned                            silent:1;
    unsigned                            paused:1;
};


typedef struct {
    ngx_flag_t                          live;
    ngx_flag_t                          meta;
    ngx_msec_t                          sync;
    ngx_msec_t                          idle_timeout;
    ngx_flag_t                          atc;
    ngx_flag_t                          interleave;
    ngx_flag_t                          wait_key;
    ngx_flag_t                          wait_video;
    ngx_flag_t                          publish_notify;
    ngx_flag_t                          play_restart;
    ngx_flag_t                          idle_streams;
    ngx_flag_t                          fix_timestamp;
    ngx_msec_t                          buflen;
} ngx_rtmp_live_app_conf_t;


extern ngx_module_t  ngx_rtmp_live_module;


#endif /* _NGX_RTMP_LIVE_H_INCLUDED_ */
