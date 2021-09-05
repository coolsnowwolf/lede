
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_stream.h"


#ifndef _NGX_TS_AVC_H_INCLUDED_
#define _NGX_TS_AVC_H_INCLUDED_


typedef struct {
    ngx_uint_t   profile_idc;
    ngx_uint_t   constraints;
    ngx_uint_t   level_idc;
    ngx_uint_t   sps_id;
    ngx_uint_t   chroma_format_idc;
    ngx_uint_t   bit_depth_luma;
    ngx_uint_t   bit_depth_chroma;
    ngx_uint_t   max_frame_num;
    ngx_uint_t   pic_order_cnt_type;
    ngx_uint_t   max_pic_order_cnt_lsb;
    ngx_int_t    offset_for_non_ref_pic;
    ngx_int_t    offset_for_top_to_bottom_field;
    ngx_uint_t   num_ref_frames_in_pic_order_cnt_cycle;
    ngx_uint_t   num_ref_frames;
    ngx_uint_t   pic_width_in_mbs;
    ngx_uint_t   pic_height_in_map_units;
    ngx_uint_t   frame_crop_left_offset;
    ngx_uint_t   frame_crop_right_offset;
    ngx_uint_t   frame_crop_top_offset;
    ngx_uint_t   frame_crop_bottom_offset;
    ngx_uint_t   width;
    ngx_uint_t   height;

    unsigned     delta_pic_order_always_zero_flag:1;
    unsigned     gaps_in_frame_num_value_allowed_flag:1;
    unsigned     frame_mbs_only_flag:1;
    unsigned     mb_adaptive_frame_field_flag:1;
    unsigned     direct_8x8_inference_flag:1;
    unsigned     frame_cropping_flag:1;
    unsigned     residual_colour_transform_flagseparate_colour_plane_flag:1;
    unsigned     qpprime_y_zero_transform_bypass_flag:1;
    unsigned     seq_scaling_matrix_present_flag:1;
} ngx_ts_avc_params_t;


ngx_ts_avc_params_t *ngx_ts_avc_decode_params(ngx_ts_stream_t *ts, u_char *sps,
    size_t sps_len, u_char *pps, size_t pps_len);


#endif /* _NGX_TS_AVC_H_INCLUDED_ */
