
/*
 * Copyright (C) Pingo (cczjp89@gmail.com)
 */

#ifndef _NGX_MPEGTS_GOP_MODULE_H_INCLUDE_
#define _NGX_MPEGTS_GOP_MODULE_H_INCLUDE_

#include "ngx_rtmp.h"

ngx_int_t
ngx_mpegts_gop_cache(ngx_rtmp_session_t *s, ngx_mpegts_frame_t *frame);
ngx_int_t
ngx_mpegts_gop_link(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss,
    ngx_msec_t time_offset, ngx_msec_t duration);
ngx_int_t
ngx_mpegts_gop_send(ngx_rtmp_session_t *s, ngx_rtmp_session_t *ss);

#endif
