
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_BANDWIDTH_H_INCLUDED_
#define _NGX_RTMP_BANDWIDTH_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/* Bandwidth update interval in seconds */
#define NGX_RTMP_BANDWIDTH_INTERVAL     10


typedef struct {
    uint64_t            bytes;
    uint64_t            bandwidth;      /* bytes/sec */

    time_t              intl_end;
    uint64_t            intl_bytes;
} ngx_rtmp_bandwidth_t;


void ngx_rtmp_update_bandwidth(ngx_rtmp_bandwidth_t *bw, uint32_t bytes);


#endif /* _NGX_RTMP_BANDWIDTH_H_INCLUDED_ */
