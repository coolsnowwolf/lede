
/*
 * Copyright (C) Roman Arutyunyan
 */


#ifndef _NGX_RTMP_BITOP_H_INCLUDED_
#define _NGX_RTMP_BITOP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


typedef struct {
    u_char      *pos;
    u_char      *last;
    ngx_uint_t   offs;
    ngx_uint_t   err;
} ngx_rtmp_bit_reader_t;


void ngx_rtmp_bit_init_reader(ngx_rtmp_bit_reader_t *br, u_char *pos,
    u_char *last);
uint64_t ngx_rtmp_bit_read(ngx_rtmp_bit_reader_t *br, ngx_uint_t n);
uint64_t ngx_rtmp_bit_read_golomb(ngx_rtmp_bit_reader_t *br);


#define ngx_rtmp_bit_read_err(br) ((br)->err)

#define ngx_rtmp_bit_read_eof(br) ((br)->pos == (br)->last)

#define ngx_rtmp_bit_read_8(br)                                               \
    ((uint8_t) ngx_rtmp_bit_read(br, 8))

#define ngx_rtmp_bit_read_16(br)                                              \
    ((uint16_t) ngx_rtmp_bit_read(br, 16))

#define ngx_rtmp_bit_read_32(br)                                              \
    ((uint32_t) ngx_rtmp_bit_read(br, 32))

#define ngx_rtmp_bit_read_64(br)                                              \
    ((uint64_t) ngx_rtmp_read(br, 64))


#endif /* _NGX_RTMP_BITOP_H_INCLUDED_ */
