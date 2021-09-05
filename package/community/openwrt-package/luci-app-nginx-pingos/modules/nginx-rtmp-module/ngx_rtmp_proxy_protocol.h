
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_PROXY_PROTOCOL_H_INCLUDED_
#define _NGX_RTMP_PROXY_PROTOCOL_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"


void ngx_rtmp_proxy_protocol(ngx_rtmp_session_t *c);


#endif /* _NGX_RTMP_PROXY_PROTOCOL_H_INCLUDED_ */
