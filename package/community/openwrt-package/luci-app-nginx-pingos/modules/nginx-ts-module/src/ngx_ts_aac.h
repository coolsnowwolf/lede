
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_stream.h"


#ifndef _NGX_TS_AAC_H_INCLUDED_
#define _NGX_TS_AAC_H_INCLUDED_


typedef struct {
    ngx_uint_t   version;
    ngx_uint_t   profile;
    ngx_uint_t   freq_index;
    ngx_uint_t   freq;
    ngx_uint_t   chan;
} ngx_ts_aac_params_t;


ngx_ts_aac_params_t *ngx_ts_aac_decode_params(ngx_ts_stream_t *ts, u_char *adts,
    size_t adts_len);


#endif /* _NGX_TS_AAC_H_INCLUDED_ */
