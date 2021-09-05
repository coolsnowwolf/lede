
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_RECORD_H_INCLUDED_
#define _NGX_RTMP_RECORD_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"


#define NGX_RTMP_RECORD_OFF             0x01
#define NGX_RTMP_RECORD_AUDIO           0x02
#define NGX_RTMP_RECORD_VIDEO           0x04
#define NGX_RTMP_RECORD_KEYFRAMES       0x08
#define NGX_RTMP_RECORD_MANUAL          0x10


typedef struct {
    ngx_str_t                           id;
    ngx_uint_t                          flags;
    ngx_str_t                           path;
    size_t                              max_size;
    size_t                              max_frames;
    ngx_msec_t                          interval;
    ngx_str_t                           suffix;
    ngx_flag_t                          unique;
    ngx_flag_t                          append;
    ngx_flag_t                          lock_file;
    ngx_flag_t                          notify;
    ngx_url_t                          *url;

    void                              **rec_conf;
    ngx_array_t                         rec; /* ngx_rtmp_record_app_conf_t * */
} ngx_rtmp_record_app_conf_t;


typedef struct {
    ngx_rtmp_record_app_conf_t         *conf;
    ngx_file_t                          file;
    ngx_uint_t                          nframes;
    uint32_t                            epoch, time_shift;
    ngx_time_t                          last;
    time_t                              timestamp;
    unsigned                            failed:1;
    unsigned                            initialized:1;
    unsigned                            aac_header_sent:1;
    unsigned                            avc_header_sent:1;
    unsigned                            video_key_sent:1;
    unsigned                            audio:1;
    unsigned                            video:1;
} ngx_rtmp_record_rec_ctx_t;


typedef struct {
    ngx_array_t                         rec; /* ngx_rtmp_record_rec_ctx_t */
    u_char                              name[NGX_RTMP_MAX_NAME];
    u_char                              args[NGX_RTMP_MAX_ARGS];
} ngx_rtmp_record_ctx_t;


ngx_uint_t ngx_rtmp_record_find(ngx_rtmp_record_app_conf_t *racf,
           ngx_str_t *id);


/* Manual recording control,
 * 'n' is record node index in config array.
 * Note: these functions allocate path in static buffer */

ngx_int_t ngx_rtmp_record_open(ngx_rtmp_session_t *s, ngx_uint_t n,
          ngx_str_t *path);
ngx_int_t ngx_rtmp_record_close(ngx_rtmp_session_t *s, ngx_uint_t n,
          ngx_str_t *path);


typedef struct {
    ngx_str_t                           recorder;
    ngx_str_t                           path;
} ngx_rtmp_record_done_t;


typedef ngx_int_t (*ngx_rtmp_record_done_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_record_done_t *v);


extern ngx_rtmp_record_done_pt          ngx_rtmp_record_done;


extern ngx_module_t                     ngx_rtmp_record_module;


#endif /* _NGX_RTMP_RECORD_H_INCLUDED_ */
