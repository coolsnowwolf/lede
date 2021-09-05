/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#ifndef _NGX_RTMP_MONITOR_H_INCLUDED_
#define _NGX_RTMP_MONITOR_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include "ngx_rtmp.h"


void ngx_rtmp_monitor_frame(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in, ngx_flag_t is_header, ngx_flag_t publishing);


#endif
