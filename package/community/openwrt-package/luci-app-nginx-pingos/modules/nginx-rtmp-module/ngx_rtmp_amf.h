
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_AMF_H_INCLUDED_
#define _NGX_RTMP_AMF_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


/* basic types */
#define NGX_RTMP_AMF_NUMBER             0x00
#define NGX_RTMP_AMF_BOOLEAN            0x01
#define NGX_RTMP_AMF_STRING             0x02
#define NGX_RTMP_AMF_OBJECT             0x03
#define NGX_RTMP_AMF_NULL               0x05
#define NGX_RTMP_AMF_ARRAY_NULL         0x06
#define NGX_RTMP_AMF_MIXED_ARRAY        0x08
#define NGX_RTMP_AMF_END                0x09
#define NGX_RTMP_AMF_ARRAY              0x0a

/* extended types */
#define NGX_RTMP_AMF_INT8               0x0100
#define NGX_RTMP_AMF_INT16              0x0101
#define NGX_RTMP_AMF_INT32              0x0102
#define NGX_RTMP_AMF_VARIANT_           0x0103

/* r/w flags */
#define NGX_RTMP_AMF_OPTIONAL           0x1000
#define NGX_RTMP_AMF_TYPELESS           0x2000
#define NGX_RTMP_AMF_CONTEXT            0x4000

#define NGX_RTMP_AMF_VARIANT            (NGX_RTMP_AMF_VARIANT_\
                                        |NGX_RTMP_AMF_TYPELESS)


typedef struct {
    ngx_int_t                           type;
    ngx_str_t                           name;
    void                               *data;
    size_t                              len;
} ngx_rtmp_amf_elt_t;


typedef ngx_chain_t * (*ngx_rtmp_amf_alloc_pt)(void *arg);


typedef struct {
    ngx_chain_t                        *link, *first;
    size_t                              offset;
    ngx_rtmp_amf_alloc_pt               alloc;
    void                               *arg;
    ngx_log_t                          *log;
} ngx_rtmp_amf_ctx_t;


/* reading AMF */
ngx_int_t ngx_rtmp_amf_read(ngx_rtmp_amf_ctx_t *ctx,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);

/* writing AMF */
ngx_int_t ngx_rtmp_amf_write(ngx_rtmp_amf_ctx_t *ctx,
        ngx_rtmp_amf_elt_t *elts, size_t nelts);


#endif /* _NGX_RTMP_AMF_H_INCLUDED_ */

