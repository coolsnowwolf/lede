/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_LIVE_RELAY_H_INCLUDE_
#define _NGX_LIVE_RELAY_H_INCLUDE_


#include <ngx_core.h>
#include "ngx_rtmp.h"


#define NGX_LIVE_RELAY_HTTPFLV      0
#define NGX_LIVE_RELAY_RTMP         1
#define NGX_LIVE_RELAY_MAXTYPE      2


typedef struct {
    // reconnect
    ngx_event_t                 reconnect;
    ngx_flag_t                  successd;           // connect successd
    /*
     * connect failed for first time, reconnect immediately
     * then reconnect after failed_reconect
     * then reconnect after 2 * failed_reconect
     * then reconnect after 2 * 2 * failed_reconect
     * ...
     * when failed_reconnect is bigger than relay_reconnect
     * then use relay_reconnect as failed_reconect
     *
     * connect failed not include connect timeout
     */
    ngx_msec_t                  failed_reconnect;   // reconnect timeout
    ngx_flag_t                  failed_delay;
    ngx_flag_t                  giveup;             // no need to reconnect

    // base para
    ngx_str_t                   domain;
    ngx_str_t                   app;
    ngx_str_t                   args;               // rtmp app args
    ngx_str_t                   name;
    ngx_str_t                   pargs;

    // other para
    ngx_str_t                   referer;            // rtmp page_url
    ngx_str_t                   user_agent;         // rtmp flashver
    ngx_str_t                   swf_url;            // rtmp swf_url
    uint32_t                    acodecs;
    uint32_t                    vcodecs;

    void                       *tag;
    ngx_uint_t                  idx;
} ngx_live_relay_ctx_t;


typedef struct {
    ngx_request_url_t           url;
    in_port_t                   port;
    ngx_uint_t                  relay_type;
} ngx_live_relay_url_t;


typedef struct {
    ngx_msec_t                  failed_reconnect;
    ngx_msec_t                  relay_reconnect;
} ngx_live_relay_app_conf_t;


extern ngx_module_t  ngx_live_relay_module;


ngx_int_t ngx_live_relay_create_httpflv(ngx_rtmp_session_t *rs,
        ngx_live_relay_t *relay, ngx_live_relay_url_t *url);

ngx_int_t ngx_live_relay_create_rtmp(ngx_rtmp_session_t *rs,
        ngx_live_relay_t *relay, ngx_live_relay_url_t *url);

ngx_int_t ngx_live_relay_create(ngx_rtmp_session_t *rs,
        ngx_live_relay_t *relay);

ngx_int_t ngx_live_relay_play_local(ngx_rtmp_session_t *rs);

ngx_int_t ngx_live_relay_publish_local(ngx_rtmp_session_t *rs);


typedef ngx_int_t (*ngx_live_pull_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_live_push_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_live_pull_close_pt)(ngx_rtmp_session_t *s);
typedef ngx_int_t (*ngx_live_push_close_pt)(ngx_rtmp_session_t *s);


extern ngx_live_push_pt         ngx_live_push;
extern ngx_live_pull_pt         ngx_live_pull;
extern ngx_live_push_close_pt   ngx_live_push_close;
extern ngx_live_pull_close_pt   ngx_live_pull_close;


#endif
