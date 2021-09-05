
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp_bandwidth.h"


void
ngx_rtmp_update_bandwidth(ngx_rtmp_bandwidth_t *bw, uint32_t bytes)
{
    if (ngx_cached_time->sec > bw->intl_end) {
        bw->bandwidth = ngx_cached_time->sec >
            bw->intl_end + NGX_RTMP_BANDWIDTH_INTERVAL
            ? 0
            : bw->intl_bytes / NGX_RTMP_BANDWIDTH_INTERVAL;
        bw->intl_bytes = 0;
        bw->intl_end = ngx_cached_time->sec + NGX_RTMP_BANDWIDTH_INTERVAL;
    }

    bw->bytes += bytes;
    bw->intl_bytes += bytes;
}
