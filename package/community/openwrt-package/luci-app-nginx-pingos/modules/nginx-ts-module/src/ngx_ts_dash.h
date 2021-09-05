
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_stream.h"
#include "ngx_ts_avc.h"
#include "ngx_ts_aac.h"


#ifndef _NGX_TS_DASH_H_INCLUDED_
#define _NGX_TS_DASH_H_INCLUDED_


typedef struct {
    ngx_path_t             *path;
    ngx_msec_t              min_seg;
    ngx_msec_t              max_seg;
    ngx_msec_t              analyze;
    size_t                  max_size;
    ngx_uint_t              nsegs;
} ngx_ts_dash_conf_t;


typedef struct {
    uint64_t                start;
    uint64_t                duration;
} ngx_ts_dash_segment_t;


typedef struct {
    u_char                 *dts;  /* 64-bit */
    u_char                 *pts;  /* 64-bit */
    u_char                 *seq;
    u_char                 *duration;
    u_char                 *sample_duration;
    u_char                 *nsamples;
    u_char                 *traf;
    u_char                 *trun;
    u_char                 *moof;
    u_char                 *moof_mdat;
    u_char                 *moof_data;
    u_char                 *mdat;
} ngx_ts_dash_subs_t;


typedef struct {
    ngx_ts_es_t            *es;

    ngx_ts_dash_segment_t  *segs;
    ngx_uint_t              nsegs;
    ngx_uint_t              seg;
    uint64_t                seg_pts;
    uint64_t                seg_dts;
    uint64_t                dts;

    ngx_uint_t              bandwidth;
    ngx_uint_t              bandwidth_bytes;
    uint64_t                bandwidth_dts;

    u_char                 *sps;
    u_char                 *pps;
    size_t                  sps_len;
    size_t                  pps_len;

    ngx_ts_avc_params_t    *avc;
    ngx_ts_aac_params_t    *aac;

    ngx_str_t               path;
    u_char                 *init_path;
    u_char                 *init_tmp_path;

    ngx_chain_t            *meta;
    ngx_chain_t            *last_meta;
    ngx_chain_t            *data;
    ngx_chain_t            *last_data;

    ngx_uint_t              nsamples;
    ngx_uint_t              nmeta;
    ngx_uint_t              ndata;

    ngx_ts_dash_subs_t      subs;
} ngx_ts_dash_rep_t;


typedef struct {
    ngx_ts_dash_rep_t      *reps;
    ngx_uint_t              nreps;
    ngx_uint_t              video;  /* unsigned  video:1; */
} ngx_ts_dash_set_t;


typedef struct {
    ngx_ts_stream_t        *ts;
    ngx_ts_dash_conf_t     *conf;

    u_char                 *mpd_path;
    u_char                 *mpd_tmp_path;
    ngx_str_t               path;
    size_t                  playlist_len;
    size_t                  init_seg_len;
    time_t                  availability_start;

    ngx_chain_t            *free;
    ngx_ts_dash_set_t      *sets;
    ngx_uint_t              nsets;

    ngx_uint_t              flush;  /* unsigned  flush:1; */
} ngx_ts_dash_t;


ngx_ts_dash_t *ngx_ts_dash_create(ngx_ts_dash_conf_t *conf, ngx_ts_stream_t *ts,
    ngx_str_t *name);
char *ngx_ts_dash_set_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

void ngx_ts_dash_write_init_segment(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);

ngx_int_t ngx_ts_dash_start_segment(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep);
ngx_chain_t *ngx_ts_dash_end_segment(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep);
void ngx_ts_dash_free_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *out);
ngx_int_t ngx_ts_dash_append_meta(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    size_t size, uint64_t dts);
ngx_int_t ngx_ts_dash_append_data(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    u_char *data, size_t len);


#endif /* _NGX_TS_DASH_H_INCLUDED_ */
