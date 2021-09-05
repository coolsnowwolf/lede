
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_CMD_H_INCLUDED_
#define _NGX_RTMP_CMD_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include "ngx_rtmp.h"


#define NGX_RTMP_MAX_NAME           256
#define NGX_RTMP_MAX_FLASHVER       256
#define NGX_RTMP_MAX_URL            256
#define NGX_RTMP_MAX_SESSION        256
#define NGX_RTMP_MAX_ARGS           NGX_RTMP_MAX_NAME


/* Basic RTMP call support */

typedef struct {
    double                          trans;
    u_char                          app[NGX_RTMP_MAX_NAME];
    u_char                          args[NGX_RTMP_MAX_ARGS];
    u_char                          flashver[NGX_RTMP_MAX_FLASHVER];
    u_char                          swf_url[NGX_RTMP_MAX_URL];
    u_char                          tc_url[NGX_RTMP_MAX_URL];
    double                          acodecs;
    double                          vcodecs;
    u_char                          page_url[NGX_RTMP_MAX_URL];
    double                          object_encoding;
} ngx_rtmp_connect_t;


typedef struct {
    double                          trans;
    double                          stream;
} ngx_rtmp_create_stream_t;


typedef struct {
    double                          stream;
} ngx_rtmp_delete_stream_t;


typedef struct {
    double                          stream;
} ngx_rtmp_close_stream_t;


typedef struct {
    u_char                          name[NGX_RTMP_MAX_NAME];
    u_char                          args[NGX_RTMP_MAX_ARGS];
    u_char                          type[16];
    int                             silent;
} ngx_rtmp_publish_t;


typedef struct {
    u_char                          name[NGX_RTMP_MAX_NAME];
    u_char                          args[NGX_RTMP_MAX_ARGS];
    u_char                          session[NGX_RTMP_MAX_SESSION];
    double                          start;
    double                          duration;
    int                             reset;
    int                             silent;
} ngx_rtmp_play_t;


typedef struct {
    double                          offset;
} ngx_rtmp_seek_t;


typedef struct {
    uint8_t                         pause;
    double                          position;
} ngx_rtmp_pause_t;


typedef struct {
    uint32_t                        msid;
} ngx_rtmp_msid_t;


typedef ngx_rtmp_msid_t             ngx_rtmp_stream_begin_t;
typedef ngx_rtmp_msid_t             ngx_rtmp_stream_eof_t;
typedef ngx_rtmp_msid_t             ngx_rtmp_stream_dry_t;
typedef ngx_rtmp_msid_t             ngx_rtmp_recorded_t;


typedef struct {
    uint32_t                        msid;
    uint32_t                        buflen;
} ngx_rtmp_set_buflen_t;


void ngx_rtmp_cmd_fill_args(u_char name[NGX_RTMP_MAX_NAME],
        u_char args[NGX_RTMP_MAX_ARGS]);
void ngx_rtmp_cmd_middleware_init(ngx_rtmp_session_t *s);
void ngx_rtmp_cmd_stream_init(ngx_rtmp_session_t *s, u_char *name, u_char *args,
        unsigned publishing);

ngx_int_t ngx_rtmp_publish_filter(ngx_rtmp_session_t *s,
        ngx_rtmp_publish_t *v);
ngx_int_t ngx_rtmp_play_filter(ngx_rtmp_session_t *s,
        ngx_rtmp_play_t *v);
ngx_int_t ngx_rtmp_close_stream_filter(ngx_rtmp_session_t *s,
        ngx_rtmp_close_stream_t *v);

ngx_int_t ngx_rtmp_push_filter(ngx_rtmp_session_t *s);
ngx_int_t ngx_rtmp_pull_filter(ngx_rtmp_session_t *s);

typedef ngx_int_t (*ngx_rtmp_connect_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_connect_t *v);
typedef ngx_int_t (*ngx_rtmp_disconnect_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_rtmp_create_stream_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_create_stream_t *v);
typedef ngx_int_t (*ngx_rtmp_close_stream_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_close_stream_t *v);
typedef ngx_int_t (*ngx_rtmp_delete_stream_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_delete_stream_t *v);
typedef ngx_int_t (*ngx_rtmp_publish_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_publish_t *v);
typedef ngx_int_t (*ngx_rtmp_play_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_play_t *v);
typedef ngx_int_t (*ngx_rtmp_seek_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_seek_t *v);
typedef ngx_int_t (*ngx_rtmp_pause_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_pause_t *v);

typedef ngx_int_t (*ngx_rtmp_stream_begin_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_stream_begin_t *v);
typedef ngx_int_t (*ngx_rtmp_stream_eof_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_stream_eof_t *v);
typedef ngx_int_t (*ngx_rtmp_stream_dry_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_stream_dry_t *v);
typedef ngx_int_t (*ngx_rtmp_recorded_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_recorded_t *v);
typedef ngx_int_t (*ngx_rtmp_set_buflen_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_set_buflen_t *v);


extern ngx_rtmp_connect_pt          ngx_rtmp_connect;
extern ngx_rtmp_disconnect_pt       ngx_rtmp_disconnect;
extern ngx_rtmp_create_stream_pt    ngx_rtmp_create_stream;
extern ngx_rtmp_close_stream_pt     ngx_rtmp_close_stream;
extern ngx_rtmp_delete_stream_pt    ngx_rtmp_delete_stream;
extern ngx_rtmp_publish_pt          ngx_rtmp_publish;
extern ngx_rtmp_play_pt             ngx_rtmp_play;
extern ngx_rtmp_seek_pt             ngx_rtmp_seek;
extern ngx_rtmp_pause_pt            ngx_rtmp_pause;

extern ngx_rtmp_stream_begin_pt     ngx_rtmp_stream_begin;
extern ngx_rtmp_stream_eof_pt       ngx_rtmp_stream_eof;
extern ngx_rtmp_stream_dry_pt       ngx_rtmp_stream_dry;
extern ngx_rtmp_set_buflen_pt       ngx_rtmp_set_buflen;
extern ngx_rtmp_recorded_pt         ngx_rtmp_recorded;


#endif /*_NGX_RTMP_CMD_H_INCLUDED_ */
