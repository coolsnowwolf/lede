
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_dash.h"


#define NGX_TS_DASH_BUFFER_SIZE     1024

#define NGX_TS_DASH_DEFAULT_WIDTH   400
#define NGX_TS_DASH_DEFAULT_HEIGHT  400


/*
 * ISO base media file format
 * ISO/IEC 14496-12:2008(E)
 */


static void ngx_ts_dash_box_styp(ngx_buf_t *b);
static void ngx_ts_dash_box_sidx(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_moof(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_mfhd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_traf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_tfhd(ngx_buf_t *b);
static void ngx_ts_dash_box_tfdt(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_trun(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_mdat(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_ftyp(ngx_buf_t *b);
static void ngx_ts_dash_box_moov(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_mvhd(ngx_buf_t *b);
static void ngx_ts_dash_box_mvex(ngx_buf_t *b);
static void ngx_ts_dash_box_trex(ngx_buf_t *b);
static void ngx_ts_dash_box_trak(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_tkhd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_mdia(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_mdhd(ngx_buf_t *b);
static void ngx_ts_dash_box_hdlr(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_minf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_vmhd(ngx_buf_t *b);
static void ngx_ts_dash_box_smhd(ngx_buf_t *b);
static void ngx_ts_dash_box_dinf(ngx_buf_t *b);
static void ngx_ts_dash_box_dref(ngx_buf_t *b);
static void ngx_ts_dash_box_url(ngx_buf_t *b);
static void ngx_ts_dash_box_stbl(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_stts(ngx_buf_t *b);
static void ngx_ts_dash_box_stsc(ngx_buf_t *b);
static void ngx_ts_dash_box_stsz(ngx_buf_t *b);
static void ngx_ts_dash_box_stco(ngx_buf_t *b);
static void ngx_ts_dash_box_stsd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_video(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_audio(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_avcc(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_box_esds(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);

static u_char *ngx_ts_dash_box(ngx_buf_t *b, const char type[4]);
static u_char *ngx_ts_dash_box_full(ngx_buf_t *b, const char type[4],
    u_char version, uint32_t flags);
static void ngx_ts_dash_box_update(ngx_buf_t *b, u_char *p);

static void ngx_ts_dash_desc_es(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_desc_dec_conf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_desc_dec_spec(ngx_buf_t *b, ngx_ts_dash_rep_t *rep);
static void ngx_ts_dash_desc_sl_conf(ngx_buf_t *b);

static u_char *ngx_ts_dash_desc(ngx_buf_t *b, u_char tag);
static void ngx_ts_dash_desc_update(ngx_buf_t *b, u_char *p);

static void ngx_ts_dash_cpymem(ngx_buf_t *b, const void *data, size_t size);
static void ngx_ts_dash_write64(ngx_buf_t *b, uint64_t v);
static void ngx_ts_dash_write32(ngx_buf_t *b, uint32_t v);
static void ngx_ts_dash_write24(ngx_buf_t *b, uint32_t v);
static void ngx_ts_dash_write16(ngx_buf_t *b, uint16_t v);
static void ngx_ts_dash_write8(ngx_buf_t *b, u_char v);

static void ngx_ts_dash_set_sub(ngx_buf_t *b, u_char **p);
static uint32_t ngx_ts_dash_get32(u_char *p);
static void ngx_ts_dash_put32(u_char *p, uint32_t v);
static void ngx_ts_dash_put64(u_char *p, uint64_t v);

static ngx_chain_t *ngx_ts_dash_get_buffer(ngx_ts_dash_t *dash);


void
ngx_ts_dash_write_init_segment(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    ngx_ts_dash_box_ftyp(b);
    ngx_ts_dash_box_moov(b, rep);
}


static void
ngx_ts_dash_box_styp(ngx_buf_t *b)
{
    /* TODO
     * ETSI TS 126 244 V12.3.0 (2014-10)
     * 13.2 Segment types, p. 52
     */

    u_char  *p;

    p = ngx_ts_dash_box(b, "styp");

    /* major_brand */
    ngx_ts_dash_cpymem(b, "iso6", 4); /* XXX 3gh9 */

    /* TODO version */
    /* minor_version */
    ngx_ts_dash_write32(b, 1);

    /* TODO brands */
    /* compatible_brands */
    ngx_ts_dash_cpymem(b, "isom", 4);
    ngx_ts_dash_cpymem(b, "iso6", 4);
    ngx_ts_dash_cpymem(b, "dash", 4);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_sidx(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    /* TODO
     * ETSI TS 126 244 V12.3.0 (2014-10)
     * 13.4 Segment Index Box, p. 53
     */

    u_char  *p;

    p = ngx_ts_dash_box_full(b, "sidx", 1, 0);

    /* reference_ID */
    ngx_ts_dash_write32(b, 1);

    /* timescale */
    ngx_ts_dash_write32(b, 90000);

    /* earliest_presentation_time */
    ngx_ts_dash_set_sub(b, &rep->subs.pts);
    ngx_ts_dash_write64(b, 0);

    /* first_offset */
    ngx_ts_dash_write64(b, 0);

    /* reference_count */
    ngx_ts_dash_write32(b, 1);

    /* referenced_size */
    ngx_ts_dash_set_sub(b, &rep->subs.moof_mdat);
    ngx_ts_dash_write32(b, 0);

    /* subsegment_duration */
    ngx_ts_dash_set_sub(b, &rep->subs.duration);
    ngx_ts_dash_write32(b, 0);

    /* starts_with_SAP, SAP_type, SAP_delta_time */
    ngx_ts_dash_write32(b, 0x80000000);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_moof(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    ngx_ts_dash_set_sub(b, &rep->subs.moof);

    p = ngx_ts_dash_box(b, "moof");

    ngx_ts_dash_box_mfhd(b, rep);
    ngx_ts_dash_box_traf(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mfhd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "mfhd", 0, 0);

    /* sequence_number */
    ngx_ts_dash_set_sub(b, &rep->subs.seq);
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_traf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    ngx_ts_dash_set_sub(b, &rep->subs.traf);

    p = ngx_ts_dash_box(b, "traf");

    ngx_ts_dash_box_tfhd(b);
    ngx_ts_dash_box_tfdt(b, rep);
    ngx_ts_dash_box_trun(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_tfhd(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "tfhd", 0, 0);

    /* track_ID */
    ngx_ts_dash_write32(b, 1);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_tfdt(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    /* 
     * ETSI TS 126 244 V12.3.0 (2014-10)
     * 13.5 Track Fragment Decode Time Box, p. 55
     */

    u_char  *p;

    p = ngx_ts_dash_box_full(b, "tfdt", 1, 0);

    /* baseMediaDecodeTime */
    ngx_ts_dash_set_sub(b, &rep->subs.dts);
    ngx_ts_dash_write64(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_trun(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char    *p;
    uint32_t   flags;

    flags = 0x000001          /* data-offset-present */
            | 0x000100        /* sample-duration-present */
            | 0x000200;       /* sample-size-present */

    if (rep->es->video) {
        flags |= 0x000400     /* sample-flags-present */
                 | 0x000800;  /* sample-composition-time-offset-present */
    }

    ngx_ts_dash_set_sub(b, &rep->subs.trun);

    p = ngx_ts_dash_box_full(b, "trun", 0, flags);

    /* sample_count */
    ngx_ts_dash_set_sub(b, &rep->subs.nsamples);
    ngx_ts_dash_write32(b, 0);

    /* data_offset */
    ngx_ts_dash_set_sub(b, &rep->subs.moof_data);
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mdat(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    ngx_ts_dash_set_sub(b, &rep->subs.mdat);

    p = ngx_ts_dash_box(b, "mdat");

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_ftyp(ngx_buf_t *b)
{
    /*
     * ISO/IEC 14496-12:2008(E)
     * 4.3 File Type Box, p. 4
     */

    u_char  *p;

    p = ngx_ts_dash_box(b, "ftyp");

    /* major_brand */
    ngx_ts_dash_cpymem(b, "iso6", 4);

    /* minor_version */
    ngx_ts_dash_write32(b, 1);

    /* TODO brands */
    /* compatible_brands */
    ngx_ts_dash_cpymem(b, "isom", 4);
    ngx_ts_dash_cpymem(b, "iso6", 4);
    ngx_ts_dash_cpymem(b, "dash", 4);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_moov(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "moov");

    ngx_ts_dash_box_mvhd(b);
    ngx_ts_dash_box_mvex(b);
    ngx_ts_dash_box_trak(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mvhd(ngx_buf_t *b)
{
    u_char  *p;
    time_t   now;

    p = ngx_ts_dash_box_full(b, "mvhd", 0, 0);

    now = ngx_time();

    /* creation_time */
    ngx_ts_dash_write32(b, now);

    /* modification_time */
    ngx_ts_dash_write32(b, now);

    /* timescale */
    ngx_ts_dash_write32(b, 90000);

    /* duration */
    ngx_ts_dash_write32(b, 0);

    /* rate */
    ngx_ts_dash_write32(b, 0x00010000);

    /* volume */
    ngx_ts_dash_write32(b, 0x01000000);

    /* reserved */
    ngx_ts_dash_write64(b, 0);

    /* matrix */
    ngx_ts_dash_write32(b, 0x00010000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00010000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x40000000);

    /* pre_defined */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    /* next_track_ID */
    ngx_ts_dash_write32(b, 1);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mvex(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "mvex");

    ngx_ts_dash_box_trex(b);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_trex(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "trex", 0, 0);

    /* track_ID */
    ngx_ts_dash_write32(b, 1);

    /* default_sample_description_index */
    ngx_ts_dash_write32(b, 1);

    /* default_sample_duration */
    ngx_ts_dash_write32(b, 0);

    /* default_sample_size */
    ngx_ts_dash_write32(b, 0);

    /* default_sample_flags */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_trak(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "trak");

    ngx_ts_dash_box_tkhd(b, rep);
    ngx_ts_dash_box_mdia(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_tkhd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;
    time_t   now;

    /* flags: Track_enabled (0x01), Track_in_movie (0x02) */
    p = ngx_ts_dash_box_full(b, "tkhd", 0, 0x03);

    now = ngx_time();

    /* creation_time */
    ngx_ts_dash_write32(b, now);

    /* modification_time */
    ngx_ts_dash_write32(b, now);

    /* track_ID */
    ngx_ts_dash_write32(b, 1);

    /* reserved */
    ngx_ts_dash_write32(b, 0);

    /* duration */
    ngx_ts_dash_write32(b, 0xffffffff);

    /* reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    /* layer, alternate_group */
    ngx_ts_dash_write32(b, 0);

    /* volume */
    ngx_ts_dash_write32(b, rep->es->video ? 0x00000000 : 0x01000000);

    /* matrix */
    ngx_ts_dash_write32(b, 0x00010000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00010000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x00000000);
    ngx_ts_dash_write32(b, 0x40000000);

    ngx_ts_dash_write32(b, (rep->avc ? rep->avc->width
                                     : NGX_TS_DASH_DEFAULT_WIDTH) << 16);

    ngx_ts_dash_write32(b, (rep->avc ? rep->avc->height
                                     : NGX_TS_DASH_DEFAULT_HEIGHT) << 16);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mdia(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "mdia");

    ngx_ts_dash_box_mdhd(b);
    ngx_ts_dash_box_hdlr(b, rep);
    ngx_ts_dash_box_minf(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_mdhd(ngx_buf_t *b)
{
    u_char  *p;

    time_t   now;

    p = ngx_ts_dash_box_full(b, "mdhd", 0, 0);

    now = ngx_time();

    /* creation_time */
    ngx_ts_dash_write32(b, now);

    /* modification_time */
    ngx_ts_dash_write32(b, now);

    /* timescale */
    ngx_ts_dash_write32(b, 90000);

    /* duration */
    ngx_ts_dash_write32(b, 0);

    /* language: und */
    ngx_ts_dash_write32(b, 0x55c40000);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_hdlr(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "hdlr", 0, 0);

    /* pre_defined */
    ngx_ts_dash_write32(b, 0);

    /* handler_type */
    ngx_ts_dash_cpymem(b, rep->es->video ? "vide" : "soun", 4);

    /* reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    /* name */
    ngx_ts_dash_cpymem(b, rep->es->video ? "video" : "audio", 6);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_minf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "minf");

    if (rep->es->video) {
        ngx_ts_dash_box_vmhd(b);

    } else {
        ngx_ts_dash_box_smhd(b);
    }

    ngx_ts_dash_box_dinf(b);
    ngx_ts_dash_box_stbl(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_vmhd(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "vmhd", 0, 1);

    /* graphicsmode, opcolor */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_smhd(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "smhd", 0, 0);

    /* balance, reserved */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_dinf(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "dinf");

    ngx_ts_dash_box_dref(b);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_dref(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "dref", 0, 0);

    /* entry_count */
    ngx_ts_dash_write32(b, 1);

    ngx_ts_dash_box_url(b);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_url(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "url ", 0, 0x01);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stbl(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "stbl");

    ngx_ts_dash_box_stsd(b, rep);
    ngx_ts_dash_box_stts(b);
    ngx_ts_dash_box_stsc(b);
    ngx_ts_dash_box_stsz(b);
    ngx_ts_dash_box_stco(b);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stts(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "stts", 0, 0);

    /* entry_count */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stsc(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "stsc", 0, 0);

    /* entry_count */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stsz(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "stsz", 0, 0);

    /* sample_size */
    ngx_ts_dash_write32(b, 0);

    /* sample_count */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stco(ngx_buf_t *b)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "stco", 0, 0);

    /* entry_count */
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_stsd(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "stsd", 0, 0);

    /* entry_count */
    ngx_ts_dash_write32(b, 1);
    
    if (rep->es->video) {
        ngx_ts_dash_box_video(b, rep);

    } else {
        ngx_ts_dash_box_audio(b, rep);
    }

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_video(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char      *p;
    ngx_uint_t   avc1;

    avc1 = (rep->es->type == NGX_TS_VIDEO_AVC);

    p = ngx_ts_dash_box(b, avc1 ? "avc1" : "mp4v");

    /* reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write16(b, 0);

    /* data_reference_index */
    ngx_ts_dash_write16(b, 1);

    /* pre_defined, reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    ngx_ts_dash_write16(b, rep->avc ? rep->avc->width
                                    : NGX_TS_DASH_DEFAULT_WIDTH);

    ngx_ts_dash_write16(b, rep->avc ? rep->avc->height
                                    : NGX_TS_DASH_DEFAULT_HEIGHT);

    /* horizresolution */
    ngx_ts_dash_write32(b, 0x00480000);

    /* vertresolution */
    ngx_ts_dash_write32(b, 0x00480000);

    /* reserved */
    ngx_ts_dash_write32(b, 0);

    /* frame_count */
    ngx_ts_dash_write16(b, 1);

    /* compressorname */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    /* depth */
    ngx_ts_dash_write16(b, 0x0018);

    /* pre_defined */
    ngx_ts_dash_write16(b, 0xffff);

    if (avc1) {
        ngx_ts_dash_box_avcc(b, rep);

    } else {
        ngx_ts_dash_box_esds(b, rep);
    }

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_audio(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box(b, "mp4a");

    /* reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write16(b, 0);

    /* data_reference_index */
    ngx_ts_dash_write16(b, 1);

    /* reserved */
    ngx_ts_dash_write32(b, 0);
    ngx_ts_dash_write32(b, 0);

    /* channel_count */
    ngx_ts_dash_write16(b, 2);

    /* samplesize */
    ngx_ts_dash_write16(b, 16);

    /* pre_defined, reserved */
    ngx_ts_dash_write32(b, 0);

    /* XXX samplerate */
    ngx_ts_dash_write32(b, 90000);

    ngx_ts_dash_box_esds(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_avcc(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p, *sps, *pps;
    size_t   sps_len, pps_len;

    sps_len = rep->sps_len;
    pps_len = rep->pps_len;

    if (sps_len < 4 || pps_len == 0) {
        return;
    }

    sps = rep->sps;
    pps = rep->pps;

    p = ngx_ts_dash_box(b, "avcC");

    /*
     * ISO/IEC 14496-15:2004(E)
     * 5.2.4.1 AVC decoder configuration record, p. 12
     */

    /* configurationVersion */
    ngx_ts_dash_write8(b, 1);

    /* AVCProfileIndication, profile_compatibility, AVCLevelIndication */
    ngx_ts_dash_cpymem(b, &sps[1], 3);

    /* lengthSizeMinusOne (lengthSize = 4) */
    ngx_ts_dash_write8(b, 0xff);

    /* numOfSequenceParameterSets = 1 */
    ngx_ts_dash_write8(b, 0xe1);

    /* sequenceParameterSetLength */
    ngx_ts_dash_write16(b, sps_len);

    /* sequenceParameterSetNALUnit */
    ngx_ts_dash_cpymem(b, sps, sps_len);

    /* numOfPictureParameterSets */
    ngx_ts_dash_write8(b, 1);

    /* pictureParameterSetLength */
    ngx_ts_dash_write16(b, pps_len);

    /* pictureParameterSetNALUnit */
    ngx_ts_dash_cpymem(b, pps, pps_len);

    ngx_ts_dash_box_update(b, p);
}


static void
ngx_ts_dash_box_esds(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    p = ngx_ts_dash_box_full(b, "esds", 0, 0);

    ngx_ts_dash_desc_es(b, rep);

    ngx_ts_dash_box_update(b, p);
}


static u_char *
ngx_ts_dash_box(ngx_buf_t *b, const char type[4])
{
    /*
     * class Box
     * ISO/IEC 14496-12:2008(E)
     * 4.2 Object Structure, p. 4
     */

    u_char  *p;

    p = b->last;

    /* size */
    ngx_ts_dash_write32(b, 0);

    /* type */
    ngx_ts_dash_cpymem(b, type, 4);

    return p;
}


static u_char *
ngx_ts_dash_box_full(ngx_buf_t *b, const char type[4], u_char version,
    uint32_t flags)
{
    /*
     * class Box
     * ISO/IEC 14496-12:2008(E)
     * 4.2 Object Structure, p. 4
     */

    u_char  *p;

    p = ngx_ts_dash_box(b, type);

    /* version */
    ngx_ts_dash_write8(b, version);

    /* flags */
    ngx_ts_dash_write24(b, flags);

    return p;
}


static void
ngx_ts_dash_box_update(ngx_buf_t *b, u_char *p)
{
    uint32_t  size;

    if (p + 4 > b->end) {
        return;
    }

    size = b->last - p;

    *p++ = (u_char) (size >> 24);
    *p++ = (u_char) (size >> 16);
    *p++ = (u_char) (size >> 8);
    *p++ = (u_char) size;
}


static void
ngx_ts_dash_desc_es(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    /*
     * ISO/IEC 14496-1:2001(E)
     * 8.6.5 ES_Descriptor, p. 28
     */

    u_char  *p;

    p = ngx_ts_dash_desc(b, 0x03);

    /* ES_ID */
    ngx_ts_dash_write16(b, 1);

    /* flags */
    ngx_ts_dash_write8(b, 0);

    ngx_ts_dash_desc_dec_conf(b, rep);
    ngx_ts_dash_desc_sl_conf(b);

    ngx_ts_dash_desc_update(b, p);
}


static void
ngx_ts_dash_desc_dec_conf(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    /*
     * ISO/IEC 14496-1:2001(E)
     * 8.6.6 DecoderConfigDescriptor, p. 30
     */

    u_char  *p;

    p = ngx_ts_dash_desc(b, 0x04);

    /* objectTypeIndication */
    ngx_ts_dash_write8(b, ngx_ts_dash_get_oti(rep->es->type));

    /* streamType, upStream, reserved */
    ngx_ts_dash_write8(b, (rep->es->video ? 0x04 : 0x05) << 2);

    /* bufferSizeDB */
    ngx_ts_dash_write24(b, 0);

    /* maxBitrate */
    ngx_ts_dash_write32(b, rep->bandwidth * 2);

    /* avgBitrate */
    ngx_ts_dash_write32(b, rep->bandwidth);

    ngx_ts_dash_desc_dec_spec(b, rep);

    ngx_ts_dash_desc_update(b, p);
}


static void
ngx_ts_dash_desc_dec_spec(ngx_buf_t *b, ngx_ts_dash_rep_t *rep)
{
    u_char  *p;

    if (rep->aac == NULL) {
        return;
    }

    /* TODO
     * AudioSpecificConfig
     * https://wiki.multimedia.cx/index.php/MPEG-4_Audio#Audio_Specific_Config
     */

    p = ngx_ts_dash_desc(b, 0x05);

    ngx_ts_dash_write8(b,
                       (rep->aac->profile << 3) + (rep->aac->freq_index >> 1));
    ngx_ts_dash_write8(b, (rep->aac->freq_index << 7) + (rep->aac->chan << 3));

    ngx_ts_dash_desc_update(b, p);
}


static void
ngx_ts_dash_desc_sl_conf(ngx_buf_t *b)
{
    /*
     * ISO/IEC 14496-1:2001(E)
     * 10.2.3 SL Packet Header Configuration, p. 227
     */

    u_char  *p;

    p = ngx_ts_dash_desc(b, 0x06);

    /* predefined */
    ngx_ts_dash_write8(b, 0x02);

    ngx_ts_dash_desc_update(b, p);
}


static u_char *
ngx_ts_dash_desc(ngx_buf_t *b, u_char tag)
{
    /*
     * ISO/IEC 14496-1:2001(E)
     * 8.2.2 BaseDescriptor, p. 16
     */

    u_char  *p;

    ngx_ts_dash_write8(b, tag);

    p = b->last;

    /* size */
    ngx_ts_dash_write32(b, 0);

    return p;
}


static void
ngx_ts_dash_desc_update(ngx_buf_t *b, u_char *p)
{
    uint32_t  size;

    if (p + 4 > b->end) {
        return;
    }

    size = b->last - p - 4;

    *p++ = (size >> 21) | 0x80;
    *p++ = (size >> 14) | 0x80;
    *p++ = (size >> 7) | 0x80;
    *p++ = size & 0x7f;
}


static void
ngx_ts_dash_cpymem(ngx_buf_t *b, const void *data, size_t size)
{
    size_t  n;

    n = ngx_min((size_t) (b->end - b->last), size);

    b->last = ngx_cpymem(b->last, data, n);
}


static void
ngx_ts_dash_write64(ngx_buf_t *b, uint64_t v)
{
    ngx_int_t  n;

    for (n = 56; n >= 0 && b->last < b->end; n -= 8) {
        *b->last++ = (u_char) (v >> n);
    }
}


static void
ngx_ts_dash_write32(ngx_buf_t *b, uint32_t v)
{
    ngx_int_t  n;

    for (n = 24; n >= 0 && b->last < b->end; n -= 8) {
        *b->last++ = (u_char) (v >> n);
    }
}


static void
ngx_ts_dash_write24(ngx_buf_t *b, uint32_t v)
{
    ngx_int_t  n;

    for (n = 16; n >= 0 && b->last < b->end; n -= 8) {
        *b->last++ = (u_char) (v >> n);
    }
}


static void
ngx_ts_dash_write16(ngx_buf_t *b, uint16_t v)
{
    ngx_int_t  n;

    for (n = 8; n >= 0 && b->last < b->end; n -= 8) {
        *b->last++ = (u_char) (v >> n);
    }
}


static void
ngx_ts_dash_write8(ngx_buf_t *b, u_char v)
{
    if (b->last < b->end) {
        *b->last++ = v;
    }
}


ngx_int_t
ngx_ts_dash_start_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep)
{
    ngx_buf_t  *b;

    ngx_memzero(&rep->subs, sizeof(ngx_ts_dash_subs_t));

    rep->nsamples = 0;
    rep->nmeta = 0;
    rep->ndata = 0;

    rep->meta = ngx_ts_dash_get_buffer(dash);
    if (rep->meta == NULL) {
        return NGX_ERROR;
    }

    rep->last_meta = rep->meta;

    b = rep->meta->buf;

    ngx_ts_dash_box_styp(b);
    ngx_ts_dash_box_sidx(b, rep);
    ngx_ts_dash_box_moof(b, rep);

    if (b->last == b->end) {
        return NGX_ERROR;
    }

    rep->data = ngx_ts_dash_get_buffer(dash);
    if (rep->data == NULL) {
        return NGX_ERROR;
    }

    rep->last_data = rep->data;

    b = rep->data->buf;

    ngx_ts_dash_box_mdat(b, rep);

    if (b->last == b->end) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


ngx_chain_t *
ngx_ts_dash_end_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep)
{
    int32_t              d;
    uint32_t             traf, trun, moof, mdat;
    ngx_chain_t         *out;
    ngx_ts_dash_subs_t  *subs;

    subs = &rep->subs;

    ngx_ts_dash_put64(subs->pts, rep->seg_pts);
    ngx_ts_dash_put64(subs->dts, rep->seg_dts);

    ngx_ts_dash_put32(subs->seq, rep->seg);
    ngx_ts_dash_put32(subs->nsamples, rep->nsamples);
    ngx_ts_dash_put32(subs->duration, rep->es->dts - rep->seg_dts);

    d = rep->es->dts - rep->dts;

    if (subs->sample_duration && d > 0) {
        ngx_ts_dash_put32(subs->sample_duration, d);
    }

    traf = ngx_ts_dash_get32(subs->traf) + rep->nmeta;
    ngx_ts_dash_put32(subs->traf, traf);

    trun = ngx_ts_dash_get32(subs->trun) + rep->nmeta;
    ngx_ts_dash_put32(subs->trun, trun);

    moof = ngx_ts_dash_get32(subs->moof) + rep->nmeta;
    ngx_ts_dash_put32(subs->moof, moof);

    mdat = ngx_ts_dash_get32(subs->mdat) + rep->ndata;
    ngx_ts_dash_put32(subs->mdat, mdat);

    ngx_ts_dash_put32(subs->moof_mdat, moof + mdat);
    ngx_ts_dash_put32(subs->moof_data, moof + 8);

    out = rep->meta;
    rep->last_meta->next = rep->data;

    rep->meta = NULL;
    rep->data = NULL;
    rep->last_meta = NULL;
    rep->last_data = NULL;

    return out;
}


static void
ngx_ts_dash_set_sub(ngx_buf_t *b, u_char **p)
{
    *p = b->last;
}


static uint32_t
ngx_ts_dash_get32(u_char *p)
{
    uint32_t  v;

    v = *p++;
    v = (v << 8) + *p++;
    v = (v << 8) + *p++;
    v = (v << 8) + *p;

    return v;
}


static void
ngx_ts_dash_put32(u_char *p, uint32_t v)
{
    *p++ = (u_char) (v >> 24);
    *p++ = (u_char) (v >> 16);
    *p++ = (u_char) (v >> 8);
    *p++ = (u_char) v;
}


static void
ngx_ts_dash_put64(u_char *p, uint64_t v)
{
    *p++ = (u_char) (v >> 56);
    *p++ = (u_char) (v >> 48);
    *p++ = (u_char) (v >> 40);
    *p++ = (u_char) (v >> 32);
    *p++ = (u_char) (v >> 24);
    *p++ = (u_char) (v >> 16);
    *p++ = (u_char) (v >> 8);
    *p++ = (u_char) v;
}


void
ngx_ts_dash_free_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *out)
{
    ngx_chain_t  *cl;

    for (cl = out; cl->next; cl = cl->next);

    cl->next = dash->free;
    dash->free = out;
}


ngx_int_t
ngx_ts_dash_append_meta(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    size_t size, uint64_t dts)
{
    size_t        n;
    ngx_buf_t    *b;
    ngx_chain_t  *cl;
    ngx_ts_es_t  *es;

    es = rep->es;

    rep->ndata += size;
    rep->nsamples++;

    n = es->video ? 16 : 8;

    cl = rep->last_meta;
    b = cl->buf;

    if ((size_t) (b->end - b->last) < n) {
        cl->next = ngx_ts_dash_get_buffer(dash);
        if (cl->next == NULL) {
            return NGX_ERROR;
        }

        cl = cl->next;
        rep->last_meta = cl;
        b = cl->buf;
    }

    rep->nmeta += n;

    /* sample_duration */
    if (rep->subs.sample_duration) {
        ngx_ts_dash_put32(rep->subs.sample_duration, dts - rep->dts);
    }

    ngx_ts_dash_set_sub(b, &rep->subs.sample_duration);
    ngx_ts_dash_write32(b, 0);

    /* sample_size */
    ngx_ts_dash_write32(b, size);

    if (es->video) {

        /*
         * ISO/IEC 14496-12:2008(E)
         * 8.8.3 Track Extends Box, Sample flags, p. 44
         * sample_is_difference_sample for non-key sample
         */

        ngx_ts_dash_write32(b, es->rand ? 0x00000000 : 0x00010000);

        /* sample_composition_time_offset */
        ngx_ts_dash_write32(b, es->pts - es->dts);
    }

    rep->dts = dts;

    return NGX_OK;
}


ngx_int_t
ngx_ts_dash_append_data(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    u_char *data, size_t len)
{
    size_t        n;
    ngx_chain_t  *cl;

    cl = rep->last_data;

    while (len) {
        if (cl->buf->last == cl->buf->end) {
            cl->next = ngx_ts_dash_get_buffer(dash);
            if (cl->next == NULL) {
                return NGX_ERROR;
            }

            cl = cl->next;
            rep->last_data = cl;
        }

        n = ngx_min((size_t) (cl->buf->end - cl->buf->last), len);

        cl->buf->last = ngx_cpymem(cl->buf->last, data, n);

        data += n;
        len -= n;
    }

    return NGX_OK;
}


static ngx_chain_t *
ngx_ts_dash_get_buffer(ngx_ts_dash_t *dash)
{
    ngx_buf_t    *b;
    ngx_chain_t  *out;

    if (dash->free) {
        out = dash->free;
        dash->free = out->next;
        out->next = NULL;
        b = out->buf;

    } else {
        out = ngx_alloc_chain_link(dash->ts->pool);
        if (out == NULL) {
            return NULL;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                       "ts dash alloc buffer:%d", NGX_TS_DASH_BUFFER_SIZE);

        b = ngx_create_temp_buf(dash->ts->pool, NGX_TS_DASH_BUFFER_SIZE);
        if (b == NULL) {
            return NULL;
        }

        out->buf = b;
        out->next = NULL;
    }

    b->pos = b->start;
    b->last = b->start;

    return out;
}
