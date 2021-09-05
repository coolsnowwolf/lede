
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_stream.h"


#define NGX_TS_PACKET_SIZE  188


typedef struct {
    ngx_chain_t   *cl;
    u_char        *p;
} ngx_ts_byte_read_t;


typedef struct {
    unsigned       pusi:1;
    unsigned       cont:4;
    unsigned       rand:1;
    unsigned       pcrf:1;
    unsigned       stuff:1;
    uint16_t       pid;
    uint64_t       pcr;
} ngx_ts_header_t;


static ngx_int_t ngx_ts_run_handlers(ngx_ts_event_e event, ngx_ts_stream_t *ts,
    ngx_ts_program_t *prog, ngx_ts_es_t *es, ngx_chain_t *bufs);
static void ngx_ts_byte_read_init(ngx_ts_byte_read_t *br, ngx_chain_t *cl);
static ngx_int_t ngx_ts_byte_read(ngx_ts_byte_read_t *br, u_char *dst,
    size_t len);
static ngx_int_t ngx_ts_byte_read_skip(ngx_ts_byte_read_t *br, size_t len);
static ngx_int_t ngx_ts_byte_read8(ngx_ts_byte_read_t *br, u_char *v);
static ngx_int_t ngx_ts_byte_read16(ngx_ts_byte_read_t *br, uint16_t *v);

static ssize_t ngx_ts_read_header(ngx_ts_stream_t *ts, u_char *p,
    ngx_ts_header_t *h);
static ngx_int_t ngx_ts_read_packet(ngx_ts_stream_t *ts, ngx_buf_t *b);
static ngx_int_t ngx_ts_read_pat(ngx_ts_stream_t *ts, ngx_ts_header_t *h,
    ngx_buf_t *b);
static ngx_int_t ngx_ts_read_pmt(ngx_ts_stream_t *ts, ngx_ts_program_t *prog,
    ngx_ts_header_t *h, ngx_buf_t *b);
static ngx_int_t ngx_ts_read_pes(ngx_ts_stream_t *ts, ngx_ts_program_t *prog,
    ngx_ts_es_t *es, ngx_ts_header_t *h, ngx_buf_t *b);
static ngx_chain_t *ngx_ts_packetize(ngx_ts_stream_t *ts, ngx_ts_header_t *h,
    ngx_chain_t *in);

static ngx_int_t ngx_ts_free_buf(ngx_ts_stream_t *ts, ngx_buf_t *b);
static ngx_int_t ngx_ts_append_buf(ngx_ts_stream_t *ts, ngx_ts_header_t *h,
    ngx_chain_t **ll, ngx_buf_t *b);
static uint32_t ngx_ts_crc32(u_char *p, size_t len);


static uint32_t  ngx_ts_crc32_table_ieee[] = {
    0x00000000, 0xb71dc104, 0x6e3b8209, 0xd926430d,
    0xdc760413, 0x6b6bc517, 0xb24d861a, 0x0550471e,
    0xb8ed0826, 0x0ff0c922, 0xd6d68a2f, 0x61cb4b2b,
    0x649b0c35, 0xd386cd31, 0x0aa08e3c, 0xbdbd4f38,
    0x70db114c, 0xc7c6d048, 0x1ee09345, 0xa9fd5241,
    0xacad155f, 0x1bb0d45b, 0xc2969756, 0x758b5652,
    0xc836196a, 0x7f2bd86e, 0xa60d9b63, 0x11105a67,
    0x14401d79, 0xa35ddc7d, 0x7a7b9f70, 0xcd665e74,
    0xe0b62398, 0x57abe29c, 0x8e8da191, 0x39906095,
    0x3cc0278b, 0x8bdde68f, 0x52fba582, 0xe5e66486,
    0x585b2bbe, 0xef46eaba, 0x3660a9b7, 0x817d68b3,
    0x842d2fad, 0x3330eea9, 0xea16ada4, 0x5d0b6ca0,
    0x906d32d4, 0x2770f3d0, 0xfe56b0dd, 0x494b71d9,
    0x4c1b36c7, 0xfb06f7c3, 0x2220b4ce, 0x953d75ca,
    0x28803af2, 0x9f9dfbf6, 0x46bbb8fb, 0xf1a679ff,
    0xf4f63ee1, 0x43ebffe5, 0x9acdbce8, 0x2dd07dec,
    0x77708634, 0xc06d4730, 0x194b043d, 0xae56c539,
    0xab068227, 0x1c1b4323, 0xc53d002e, 0x7220c12a,
    0xcf9d8e12, 0x78804f16, 0xa1a60c1b, 0x16bbcd1f,
    0x13eb8a01, 0xa4f64b05, 0x7dd00808, 0xcacdc90c,
    0x07ab9778, 0xb0b6567c, 0x69901571, 0xde8dd475,
    0xdbdd936b, 0x6cc0526f, 0xb5e61162, 0x02fbd066,
    0xbf469f5e, 0x085b5e5a, 0xd17d1d57, 0x6660dc53,
    0x63309b4d, 0xd42d5a49, 0x0d0b1944, 0xba16d840,
    0x97c6a5ac, 0x20db64a8, 0xf9fd27a5, 0x4ee0e6a1,
    0x4bb0a1bf, 0xfcad60bb, 0x258b23b6, 0x9296e2b2,
    0x2f2bad8a, 0x98366c8e, 0x41102f83, 0xf60dee87,
    0xf35da999, 0x4440689d, 0x9d662b90, 0x2a7bea94,
    0xe71db4e0, 0x500075e4, 0x892636e9, 0x3e3bf7ed,
    0x3b6bb0f3, 0x8c7671f7, 0x555032fa, 0xe24df3fe,
    0x5ff0bcc6, 0xe8ed7dc2, 0x31cb3ecf, 0x86d6ffcb,
    0x8386b8d5, 0x349b79d1, 0xedbd3adc, 0x5aa0fbd8,
    0xeee00c69, 0x59fdcd6d, 0x80db8e60, 0x37c64f64,
    0x3296087a, 0x858bc97e, 0x5cad8a73, 0xebb04b77,
    0x560d044f, 0xe110c54b, 0x38368646, 0x8f2b4742,
    0x8a7b005c, 0x3d66c158, 0xe4408255, 0x535d4351,
    0x9e3b1d25, 0x2926dc21, 0xf0009f2c, 0x471d5e28,
    0x424d1936, 0xf550d832, 0x2c769b3f, 0x9b6b5a3b,
    0x26d61503, 0x91cbd407, 0x48ed970a, 0xfff0560e,
    0xfaa01110, 0x4dbdd014, 0x949b9319, 0x2386521d,
    0x0e562ff1, 0xb94beef5, 0x606dadf8, 0xd7706cfc,
    0xd2202be2, 0x653deae6, 0xbc1ba9eb, 0x0b0668ef,
    0xb6bb27d7, 0x01a6e6d3, 0xd880a5de, 0x6f9d64da,
    0x6acd23c4, 0xddd0e2c0, 0x04f6a1cd, 0xb3eb60c9,
    0x7e8d3ebd, 0xc990ffb9, 0x10b6bcb4, 0xa7ab7db0,
    0xa2fb3aae, 0x15e6fbaa, 0xccc0b8a7, 0x7bdd79a3,
    0xc660369b, 0x717df79f, 0xa85bb492, 0x1f467596,
    0x1a163288, 0xad0bf38c, 0x742db081, 0xc3307185,
    0x99908a5d, 0x2e8d4b59, 0xf7ab0854, 0x40b6c950,
    0x45e68e4e, 0xf2fb4f4a, 0x2bdd0c47, 0x9cc0cd43,
    0x217d827b, 0x9660437f, 0x4f460072, 0xf85bc176,
    0xfd0b8668, 0x4a16476c, 0x93300461, 0x242dc565,
    0xe94b9b11, 0x5e565a15, 0x87701918, 0x306dd81c,
    0x353d9f02, 0x82205e06, 0x5b061d0b, 0xec1bdc0f,
    0x51a69337, 0xe6bb5233, 0x3f9d113e, 0x8880d03a,
    0x8dd09724, 0x3acd5620, 0xe3eb152d, 0x54f6d429,
    0x7926a9c5, 0xce3b68c1, 0x171d2bcc, 0xa000eac8,
    0xa550add6, 0x124d6cd2, 0xcb6b2fdf, 0x7c76eedb,
    0xc1cba1e3, 0x76d660e7, 0xaff023ea, 0x18ede2ee,
    0x1dbda5f0, 0xaaa064f4, 0x738627f9, 0xc49be6fd,
    0x09fdb889, 0xbee0798d, 0x67c63a80, 0xd0dbfb84,
    0xd58bbc9a, 0x62967d9e, 0xbbb03e93, 0x0cadff97,
    0xb110b0af, 0x060d71ab, 0xdf2b32a6, 0x6836f3a2,
    0x6d66b4bc, 0xda7b75b8, 0x035d36b5, 0xb440f7b1
};


ngx_int_t
ngx_ts_add_handler(ngx_ts_stream_t *ts, ngx_ts_handler_pt handler, void *data)
{
    ngx_ts_handler_t  *h;

    h = ngx_palloc(ts->pool, sizeof(ngx_ts_handler_t));
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->handler = handler;
    h->data = data;

    h->next = ts->handlers;
    ts->handlers = h;

    return NGX_OK;
}


static ngx_int_t
ngx_ts_run_handlers(ngx_ts_event_e event, ngx_ts_stream_t *ts,
    ngx_ts_program_t *prog, ngx_ts_es_t *es, ngx_chain_t *bufs)
{
    ngx_ts_handler_t       *h;
    ngx_ts_handler_data_t   hd;

    hd.event = event;
    hd.ts = ts;
    hd.prog = prog;
    hd.es = es;
    hd.bufs = bufs;

    for (h = ts->handlers; h; h = h->next) {
        hd.data = h->data;

        if (h->handler(&hd) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static void
ngx_ts_byte_read_init(ngx_ts_byte_read_t *br, ngx_chain_t *cl)
{
    br->cl = cl;
    br->p = cl ? cl->buf->pos : NULL;
}


static ngx_int_t
ngx_ts_byte_read(ngx_ts_byte_read_t *br, u_char *dst, size_t len)
{
    size_t  n;

    while (br->cl && len) {
        n = ngx_min((size_t) (br->cl->buf->last - br->p), len);

        if (dst) {
            dst = ngx_cpymem(dst, br->p, n);
        }

        br->p += n;
        len -= n;

        if (br->p == br->cl->buf->last) {
            br->cl = br->cl->next;
            br->p = br->cl ? br->cl->buf->pos : NULL;
        }
    }

    return len ? NGX_AGAIN : NGX_OK;
}


static ngx_int_t
ngx_ts_byte_read_skip(ngx_ts_byte_read_t *br, size_t len)
{
    return ngx_ts_byte_read(br, NULL, len);
}


static ngx_int_t
ngx_ts_byte_read8(ngx_ts_byte_read_t *br, u_char *v)
{
    return ngx_ts_byte_read(br, v, 1);
}


static ngx_int_t
ngx_ts_byte_read16(ngx_ts_byte_read_t *br, uint16_t *v)
{
    if (ngx_ts_byte_read(br, (u_char *) v, 2) == NGX_AGAIN) {
        return NGX_AGAIN;
    }

#if (NGX_HAVE_LITTLE_ENDIAN)
    *v = htons(*v);
#endif

    return NGX_OK;
}


ngx_int_t
ngx_ts_read(ngx_ts_stream_t *ts, ngx_chain_t *in)
{
    size_t        n, size;
    ngx_buf_t    *b;
    ngx_chain_t  *cl;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts read");

    for (/* void */; in; in = in->next) {

        while (in->buf->pos != in->buf->last) {
            b = ts->buf;

            if (b == NULL) {
                if (ts->free) {
                    cl = ts->free;
                    ts->free = cl->next;

                    b = cl->buf;
                    ngx_free_chain(ts->pool, cl);

                    b->pos = b->start;
                    b->last = b->start;

                } else {
                    b = ngx_create_temp_buf(ts->pool, NGX_TS_PACKET_SIZE);
                    if (b == NULL) {
                        return NGX_ERROR;
                    }
                }

                ts->buf = b;
            }

            n = b->end - b->last;
            size = in->buf->last - in->buf->pos;

            if (n > size) {
                n = size;
            }

            b->last = ngx_cpymem(b->last, in->buf->pos, n);
            in->buf->pos += n;

            if (b->last == b->end) {
                if (ngx_ts_read_packet(ts, b) != NGX_OK) {
                    return NGX_ERROR;
                }

                ts->buf = NULL;
            }
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ts_read_packet(ngx_ts_stream_t *ts, ngx_buf_t *b)
{
    ssize_t            n;
    ngx_uint_t         i, j;
    ngx_chain_t       *cl;
    ngx_ts_es_t       *es;
    ngx_ts_header_t    h;
    ngx_ts_program_t  *prog;

    n = ngx_ts_read_header(ts, b->pos, &h);

    if (n == NGX_ERROR) {
        return NGX_ERROR;
    }

    if (n == NGX_DONE) {
        b->pos = b->last;

    } else {
        b->pos += n;
    }

    if (h.pid == 0) {
        return ngx_ts_read_pat(ts, &h, b);
    }

    for (i = 0; i < ts->nprogs; i++) {
        prog = &ts->progs[i];

        if (h.pid == prog->pid) {
            return ngx_ts_read_pmt(ts, prog, &h, b);
        }

        for (j = 0; j < prog->nes; j++) {
            es = &prog->es[j];

            if (h.pid == es->pid) {
                return ngx_ts_read_pes(ts, prog, es, &h, b);
            }
        }
    }

    ngx_log_error(NGX_LOG_INFO, ts->log, 0,
                  "dropping unexpected TS packet pid:0x%04uxd",
                  (unsigned) h.pid);

    cl = ngx_alloc_chain_link(ts->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = ts->free;
    ts->free = cl;

    return NGX_OK;
}


static ssize_t
ngx_ts_read_header(ngx_ts_stream_t *ts, u_char *p, ngx_ts_header_t *h)
{
    /*
     * TS Packet Header
     * ISO/IEC 13818-1 : 2000 (E)
     * 2.4.3.2 Transport Stream packet layer, p. 18
     */

    u_char    alen, afic;
    ssize_t   n;
    uint64_t  pcrb, pcre;

    /* sync_byte */
    if (*p++ != 0x47) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "invalid TS sync byte");
        return NGX_ERROR;
    }

    ngx_memzero(h, sizeof(ngx_ts_header_t));

    /* payload_unit_start_indicator */
    h->pusi = (*p & 0x40) ? 1 : 0;

    /* PID */
    h->pid = *p++ & 0x1f;
    h->pid = (h->pid << 8) + *p++;

    /* adaptation_field_control */
    afic = (*p & 0x30) >> 4;

    /* continuity_counter */
    h->cont = *p++ & 0x0f;

    if (afic == 0) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "invalid TS packet");
        return NGX_ERROR;
    }

    n = 4;

    if (afic & 0x02) {
        /* adaptation_field_length */
        alen = *p++;

        if (alen > 183) {
            ngx_log_error(NGX_LOG_ERR, ts->log, 0,
                          "too long TS adaptation field");
            return NGX_ERROR;
        }

        if (afic & 0x01) {
            n += alen + 1;

        } else {
            n = NGX_DONE;
        }

        if (alen) {
            /* random_access_indicator */
            h->rand = (*p & 0x40) ? 1 : 0;

            /* PCR_flag */
            h->pcrf = (*p & 0x10) ? 1 : 0;

            p++;

            if (h->pcrf) {
                /* program_clock_reference_base */
                pcrb = *p++;
                pcrb = (pcrb << 8) + *p++;
                pcrb = (pcrb << 8) + *p++;
                pcrb = (pcrb << 8) + *p++;
                pcrb = (pcrb << 1) + (*p >> 7);

                /* program_clock_reference_extension */
                pcre = *p++ & 0x01;
                pcre = (pcre << 8) + *p++;

                h->pcr = pcrb * 300 + pcre;
            }
        }
    }

    ngx_log_debug6(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts packet "
                   "pid:0x%04uxd, pusi:%d, c:%02d, r:%d, pcr:%uL, n:%uz",
                   (unsigned) h->pid, h->pusi, h->cont, h->rand, h->pcr,
                   n == NGX_DONE ? 0 : NGX_TS_PACKET_SIZE - n);

    return n;
}


static ngx_int_t
ngx_ts_read_pat(ngx_ts_stream_t *ts, ngx_ts_header_t *h, ngx_buf_t *b)
{
    /*
     * PAT
     * ISO/IEC 13818-1 : 2000 (E)
     * 2.4.4.3 Program association Table, p. 43
     */

    u_char               ptr;
    uint16_t             len, number, pid;
    ngx_uint_t           nprogs, n;
    ngx_ts_program_t    *prog;
    ngx_ts_byte_read_t   br, pr;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts pat");

    if (ts->nprogs) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0,
                       "ts dropping successive pat");
        return ngx_ts_free_buf(ts, b);
    }

    if (ngx_ts_append_buf(ts, h, &ts->bufs, b) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_byte_read_init(&br, ts->bufs);

    /* pointer_field */
    if (ngx_ts_byte_read8(&br, &ptr) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* skipped bytes + table_id */
    if (ngx_ts_byte_read_skip(&br, ptr + 1) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* section_length */
    if (ngx_ts_byte_read16(&br, &len) == NGX_AGAIN) {
        return NGX_OK;
    }

    len &= 0x0fff;

    if (len < 9) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "malformed PAT");
        return NGX_ERROR;
    }

    if (len > 0x03fd) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "too big PAT: %ud",
                      (unsigned) len);
        return NGX_ERROR;
    }

    pr = br;

    if (ngx_ts_byte_read_skip(&pr, len) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* PAT is fully available */

    /* transport_stream_id .. last_section_number */
    ngx_ts_byte_read_skip(&br, 5);

    nprogs = (len - 9) / 4;

    ts->progs = ngx_pcalloc(ts->pool,
                            nprogs * sizeof(ngx_ts_program_t));
    if (ts->progs == NULL) {
        return NGX_ERROR;
    }

    prog = ts->progs;

    for (n = 0; n < nprogs; n++) {
        /* program_number */
        (void) ngx_ts_byte_read16(&br, &number);

        /* network_PID / program_map_PID */
        (void) ngx_ts_byte_read16(&br, &pid);

        if (number) {
            pid = pid & 0x1fff;

            prog->number = number;
            prog->pid = pid;
            prog++;

            ngx_log_debug2(NGX_LOG_DEBUG_CORE, ts->log, 0,
                           "ts program %ud, pid:0x%04uxd",
                           (unsigned) number, (unsigned) pid);
        }
    }

    ts->nprogs = prog - ts->progs;

    if (ngx_ts_run_handlers(NGX_TS_PAT, ts, NULL, NULL, NULL) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_free_chain(ts, &ts->bufs);

    return NGX_OK;
}


static ngx_int_t
ngx_ts_read_pmt(ngx_ts_stream_t *ts, ngx_ts_program_t *prog, ngx_ts_header_t *h,
    ngx_buf_t *b)
{
    /*
     * PMT
     * ISO/IEC 13818-1 : 2000 (E)
     * 2.4.4.8 Program Map Table, p. 46
     */

    u_char               ptr, type;
    uint16_t             len, pilen, elen, pid;
    ngx_uint_t           nes, n;
    ngx_ts_es_t         *es;
    ngx_ts_byte_read_t   br, pr;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts pmt");

    if (prog->nes) {
        ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0,
                       "ts dropping successive pmt");
        return ngx_ts_free_buf(ts, b);
    }

    if (ngx_ts_append_buf(ts, h, &prog->bufs, b) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_byte_read_init(&br, prog->bufs);

    /* pointer_field */
    if (ngx_ts_byte_read8(&br, &ptr) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* skipped bytes + table_id */
    if (ngx_ts_byte_read_skip(&br, ptr + 1) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* section_length */
    if (ngx_ts_byte_read16(&br, &len) == NGX_AGAIN) {
        return NGX_OK;
    }

    len &= 0x0fff;

    if (len < 13) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "malformed PMT");
        return NGX_ERROR;
    }

    if (len > 0x03fd) {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "too big PMT: %ud",
                      (unsigned) len);
        return NGX_ERROR;
    }

    pr = br;

    if (ngx_ts_byte_read_skip(&pr, len) == NGX_AGAIN) {
        return NGX_OK;
    }

    /* PMT is fully available */

    /* program_number .. last_sesion_number */
    (void) ngx_ts_byte_read_skip(&br, 5);

    /* PCR_PID */
    (void) ngx_ts_byte_read16(&br, &pid);

    prog->pcr_pid = pid & 0x1fff;

    /* program_info_length */
    (void) ngx_ts_byte_read16(&br, &pilen);

    pilen &= 0x0fff;

    if (ngx_ts_byte_read_skip(&br, pilen) == NGX_AGAIN
        || len < 13 + pilen)
    {
        ngx_log_error(NGX_LOG_ERR, ts->log, 0, "malformed PMT");
        return NGX_ERROR;
    }

    len -= 13 + pilen;

    pr = br;

    for (nes = 0; len > 0; nes++) {
        if (ngx_ts_byte_read(&pr, NULL, 3) == NGX_AGAIN
            || ngx_ts_byte_read16(&pr, &elen) == NGX_AGAIN
            || ngx_ts_byte_read(&pr, NULL, elen & 0x0fff) == NGX_AGAIN
            || len < 5 + (elen & 0x0fff))
        {
            ngx_log_error(NGX_LOG_ERR, ts->log, 0, "malformed PMT");
            return NGX_ERROR;
        }

        len -= 5 + (elen & 0x0fff);
    }

    es = ngx_pcalloc(ts->pool, nes * sizeof(ngx_ts_es_t));
    if (es == NULL) {
        return NGX_ERROR;
    }

    prog->es = es;
    prog->nes = nes;

    for (n = 0; n < nes; n++, es++) {
        /* stream_type */
        (void) ngx_ts_byte_read8(&br, &type);

        /* elementary_PID */
        (void) ngx_ts_byte_read16(&br, &pid);

        /* ES_info_length */
        (void) ngx_ts_byte_read16(&br, &elen);

        /* ES_info */
        (void) ngx_ts_byte_read_skip(&br, elen & 0x0fff);

        pid = pid & 0x1fff;

        es->type = type;
        es->pid = pid;

        if (type == NGX_TS_VIDEO_MPEG1
            || type == NGX_TS_VIDEO_MPEG2
            || type == NGX_TS_VIDEO_MPEG4
            || type == NGX_TS_VIDEO_AVC)
        {
            es->video = 1;
            prog->video = 1;
        }

        ngx_log_debug3(NGX_LOG_DEBUG_CORE, ts->log, 0,
                       "ts es type:%ui, video:%d, pid:0x%04uxd",
                       (ngx_uint_t) type, es->video, (unsigned) pid);
    }

    if (ngx_ts_run_handlers(NGX_TS_PMT, ts, prog, NULL, NULL) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_free_chain(ts, &prog->bufs);

    return NGX_OK;
}


static ngx_int_t
ngx_ts_read_pes(ngx_ts_stream_t *ts, ngx_ts_program_t *prog, ngx_ts_es_t *es,
    ngx_ts_header_t *h, ngx_buf_t *b)
{
    /*
     * PES Packet
     * ISO/IEC 13818-1 : 2000 (E)
     * 2.4.3.6 PES packet, p. 31
     */

    u_char              sid, pfx[3], v8, hlen;
    uint16_t            len, flags, v16;
    uint64_t            pts, dts;
    ngx_uint_t          ptsf;
    ngx_ts_byte_read_t  br, pr;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts pes");

    if (es->bufs && h->pusi && b) {
        if (ngx_ts_read_pes(ts, prog, es, h, NULL) != NGX_OK) {
            return NGX_ERROR;
        }
    }

    if (es->bufs == NULL) {
        es->rand = h->rand;
    }

    if (h->pcrf && prog->pcr_pid == es->pid) {
        prog->pcr = h->pcr;
    }

    if (ngx_ts_append_buf(ts, h, &es->bufs, b) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_byte_read_init(&br, es->bufs);

    /* packet_start_code_prefix */
    if (ngx_ts_byte_read(&br, pfx, 3) == NGX_AGAIN) {
        return NGX_OK;
    }

    if (pfx[0] != 0 || pfx[1] != 0 || pfx[2] != 1) {
        ngx_log_error(NGX_LOG_INFO, ts->log, 0, "missing PES start prefix");
        return NGX_ERROR;
    }

    /* stream_id */
    if (ngx_ts_byte_read8(&br, &sid) == NGX_AGAIN) {
        return NGX_OK;
    }

    es->sid = sid;

    /* PES_packet_length */
    if (ngx_ts_byte_read16(&br, &len) == NGX_AGAIN) {
        return NGX_OK;
    }

    if (len) {
        pr = br;

        if (ngx_ts_byte_read_skip(&pr, len) == NGX_AGAIN) {
            return NGX_OK;
        }

    } else if (b) {
        /* wait for PUSI */
        return NGX_OK;
    }

    /* PES is fully available */

    if (sid == 0xbe) {  /* padding_stream */
        ngx_ts_free_chain(ts, &es->bufs);
        return NGX_OK;
    }

    ptsf = 0;

    if (sid != 0xbc     /* program_stream_map */
        && sid != 0xbf  /* private_stream_2 */
        && sid != 0xf0  /* ECM_stream */
        && sid != 0xf1  /* EMM_stream */
        && sid != 0xff  /* program_stream_directory */
        && sid != 0xf2  /* DSMCC_stream */
        && sid != 0xf8) /* ITU-T Rec. H.222.1 type E stream */
    {
        /* PES_scrambling_control .. PES_extension_flag */
        if (ngx_ts_byte_read16(&br, &flags) == NGX_AGAIN) {
            return NGX_OK;
        }

        /* PES_header_data_length */
        if (ngx_ts_byte_read8(&br, &hlen) == NGX_AGAIN) {
            return NGX_OK;
        }

        if (len) {
            if (len < 3 + hlen) {
                ngx_log_error(NGX_LOG_INFO, ts->log, 0, "malformed PES");
                return NGX_ERROR;
            }

            len -= 3 + hlen;
        }

        pr = br;

        if (ngx_ts_byte_read_skip(&br, hlen) == NGX_AGAIN) {
            return NGX_OK;
        }

        if ((flags & 0x00c0) == 0x0080) { /* PTS_DTS_flags == '10' */
            ptsf = 1;

            /* PTS[32..30] */
            if (ngx_ts_byte_read8(&pr, &v8) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts = (uint64_t) (v8 & 0x0e) << 29;

            /* PTS[29..15] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts |= (uint64_t) (v16 & 0xfffe) << 14;

            /* PTS[14..0] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts |= v16 >> 1;

            es->pts = pts;
            es->dts = pts;

            ngx_log_debug1(NGX_LOG_DEBUG_CORE, ts->log, 0,
                           "ts pes pts:%uL", pts);
        }

        if ((flags & 0x00c0) == 0x00c0) { /* PTS_DTS_flags == '11' */
            ptsf = 1;

            /* PTS[32..30] */
            if (ngx_ts_byte_read8(&pr, &v8) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts = (uint64_t) (v8 & 0x0e) << 29;

            /* PTS[29..15] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts |= (uint64_t) (v16 & 0xfffe) << 14;

            /* PTS[14..0] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            pts |= v16 >> 1;

            /* DTS[32..30] */
            if (ngx_ts_byte_read8(&pr, &v8) == NGX_AGAIN) {
                return NGX_OK;
            }

            dts = (uint64_t) (v8 & 0x0e) << 29;

            /* DTS[29..15] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            dts |= (uint64_t) (v16 & 0xfffe) << 14;

            /* DTS[14..0] */
            if (ngx_ts_byte_read16(&pr, &v16) == NGX_AGAIN) {
                return NGX_OK;
            }

            dts |= v16 >> 1;

            es->pts = pts;
            es->dts = dts;

            ngx_log_debug2(NGX_LOG_DEBUG_CORE, ts->log, 0,
                           "ts pes pts:%uL, dts:%uL", pts, dts);
        }
    }

    if (br.cl) {
        br.cl->buf->pos = br.p;
    }

    if (len) {
        pr = br;

        if (ngx_ts_byte_read_skip(&pr, len) == NGX_AGAIN) {
            return NGX_OK;
        }

        if (pr.cl) {
            pr.cl->buf->last = pr.p;
        }
    }

    es->ptsf = ptsf;

    if (ngx_ts_run_handlers(NGX_TS_PES, ts, prog, es, br.cl) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_ts_free_chain(ts, &es->bufs);

    return NGX_OK;
}


ngx_chain_t *
ngx_ts_write_pat(ngx_ts_stream_t *ts, ngx_ts_program_t *prog)
{
    size_t            len;
    u_char           *p, *data;
    uint32_t          crc;
    ngx_buf_t         b;
    ngx_uint_t        n, nprogs;
    ngx_chain_t       in;
    ngx_ts_header_t   h;

    if (prog) {
        nprogs = 1;

    } else {
        nprogs = ts->nprogs;
        prog = ts->progs;
    }

    len = 9 + nprogs * 4;

    data = ngx_pnalloc(ts->pool, 4 + len);
    if (data == NULL) {
        return NULL;
    }

    p = data;

    /* pointer_field */
    *p++ = 0;

    /* table_id */
    *p++ = 0;

    /* section_syntax_indicator, section_length */
    *p++ = 0x80 | (u_char) (len >> 8);
    *p++ = (u_char) len;

    /* transport_stream_id */
    *p++ = 0;
    *p++ = 0;

    /* version_number, current_next_indicator */
    *p++ = 0x01;

    /* section_number */
    *p++ = 0;

    /* last_section_number */
    *p++ = 0;

    for (n = 0; n < nprogs; n++, prog++) {
        /* program_number */
        *p++ = (u_char) (prog->number >> 8);
        *p++ = (u_char) prog->number;

        /* program_map_PID */
        *p++ = (u_char) (prog->pid >> 8);
        *p++ = (u_char) prog->pid;
    }

    crc = ngx_ts_crc32(data + 1, p - data - 1);

    *p++ = (u_char) crc;
    *p++ = (u_char) (crc >> 8);
    *p++ = (u_char) (crc >> 16);
    *p++ = (u_char) (crc >> 24);

    ngx_memzero(&b, sizeof(ngx_buf_t));

    b.start = data;
    b.pos = data;
    b.last = p;
    b.end = p;

    in.buf = &b;
    in.next = NULL;

    ngx_memzero(&h, sizeof(ngx_ts_header_t));

    return ngx_ts_packetize(ts, &h, &in);
}


ngx_chain_t *
ngx_ts_write_pmt(ngx_ts_stream_t *ts, ngx_ts_program_t *prog)
{
    size_t            len;
    u_char           *p, *data;
    uint32_t          crc;
    ngx_buf_t         b;
    ngx_uint_t        n;
    ngx_chain_t       in;
    ngx_ts_es_t      *es;
    ngx_ts_header_t   h;

    len = 13 + 5 * prog->nes;

    data = ngx_pnalloc(ts->pool, 4 + len);
    if (data == NULL) {
        return NULL;
    }

    p = data;

    /* pointer_field */
    *p++ = 0;

    /* table_id */
    *p++ = 0x02;

    /* section_syntax_indicator */
    *p++ = 0x80 | (u_char) (len >> 8);
    *p++ = (u_char) len;
    
    /* program_number */
    *p++ = (u_char) (prog->number >> 8);
    *p++ = (u_char) prog->number;

    /* version_number, current_next_indicator */
    *p++ = 0x01;

    /* section_number */
    *p++ = 0;

    /* last_section_number */
    *p++ = 0;

    /* PCR_PID */
    *p++ = (u_char) (prog->pcr_pid >> 8);
    *p++ = (u_char) prog->pcr_pid;

    /* program_info_length */
    *p++ = 0;
    *p++ = 0;

    for (n = 0; n < prog->nes; n++) {
        es = &prog->es[n];

        /* stream_type */
        *p++ = es->type;

        /* elementary_PID */
        *p++ = (u_char) (es->pid >> 8);
        *p++ = (u_char) es->pid;

        /* ES_info_length */
        *p++ = 0;
        *p++ = 0;
    }

    crc = ngx_ts_crc32(data + 1, p - data - 1);

    *p++ = (u_char) crc;
    *p++ = (u_char) (crc >> 8);
    *p++ = (u_char) (crc >> 16);
    *p++ = (u_char) (crc >> 24);

    ngx_memzero(&b, sizeof(ngx_buf_t));

    b.start = data;
    b.pos = data;
    b.last = p;
    b.end = p;

    in.buf = &b;
    in.next = NULL;

    ngx_memzero(&h, sizeof(ngx_ts_header_t));

    h.pid = prog->pid;

    return ngx_ts_packetize(ts, &h, &in);
}


ngx_chain_t *
ngx_ts_write_pes(ngx_ts_stream_t *ts, ngx_ts_program_t *prog, ngx_ts_es_t *es,
    ngx_chain_t *bufs)
{
    size_t            len;
    u_char           *p, *plen, *phflags, *phlen;
    ngx_buf_t         b;
    ngx_chain_t       in, *cl, *out;
    ngx_ts_header_t   h;
    u_char            buf[19];

    p = buf;

    /* packet_start_code_prefix */
    *p++ = 0;
    *p++ = 0;
    *p++ = 1;

    /* stream_id */
    *p++ = es->sid;

    /* PES_packet_length */
    plen = p;
    *p++ = 0;
    *p++ = 0;

    if (es->sid != 0xbc     /* program_stream_map */
        && es->sid != 0xbe  /* padding_stream */
        && es->sid != 0xbf  /* private_stream_2 */
        && es->sid != 0xf0  /* ECM_stream */
        && es->sid != 0xf1  /* EMM_stream */
        && es->sid != 0xff  /* program_stream_directory */
        && es->sid != 0xf2  /* DSMCC_stream */
        && es->sid != 0xf8) /* ITU-T Rec. H.222.1 type E stream */
    {
        /* PES_scrambling_control .. original_or_copy */
        *p++ = 0x80;

        /* XXX ptsf */

        /* PTS_DTS_flags .. PES_extension_flag */
        phflags = p;
        *p++ = 0;

        /* PES_header_data_lenth */
        phlen = p;
        *p++ = 0;

        if (es->pts == es->dts) {
            *phlen += 5;
            *phflags |= 0x80;

            /* PTS */
            *p++ = 0x21 | (u_char) ((es->pts >> 29) & 0x0e);
            *p++ = (u_char) (es->pts >> 22);
            *p++ = 0x01 | (u_char) (es->pts >> 14);
            *p++ = (u_char) (es->pts >> 7);
            *p++ = 0x01 | (u_char) (es->pts << 1);

        } else {
            *phlen += 10;
            *phflags |= 0xc0;

            /* PTS */
            *p++ = 0x31 | (u_char) ((es->pts >> 29) & 0x0e);
            *p++ = (u_char) (es->pts >> 22);
            *p++ = 0x01 | (u_char) (es->pts >> 14);
            *p++ = (u_char) (es->pts >> 7);
            *p++ = 0x01 | (u_char) (es->pts << 1);

            /* DTS */
            *p++ = 0x11 | (u_char) ((es->dts >> 29) & 0x0e);
            *p++ = (u_char) (es->dts >> 22);
            *p++ = 0x01 | (u_char) (es->dts >> 14);
            *p++ = (u_char) (es->dts >> 7);
            *p++ = 0x01 | (u_char) (es->dts << 1);
        }
    }

    len = p - plen - 2;
    for (cl = bufs; cl; cl = cl->next) {
        len += cl->buf->last - cl->buf->pos;
    }

    if (len <= 0xffff) {
        *plen++ = (u_char) (len >> 8);
        *plen++ = (u_char) len;
    }

    ngx_memzero(&b, sizeof(ngx_buf_t));

    b.start = buf;
    b.pos = buf;
    b.last = p;
    b.end = p;

    in.buf = &b;
    in.next = bufs;

    ngx_memzero(&h, sizeof(ngx_ts_header_t));

    h.pid = es->pid;
    h.rand = es->rand;
    h.cont = es->cont;
    h.stuff = 1;

    if (prog->pcr_pid == es->pid) {
        h.pcrf = 1;
        h.pcr = prog->pcr;
    }

    out = ngx_ts_packetize(ts, &h, &in);
    if (out == NULL) {
        return NULL;
    }

    es->cont = h.cont;

    return out;
}


static ngx_chain_t *
ngx_ts_packetize(ngx_ts_stream_t *ts, ngx_ts_header_t *h, ngx_chain_t *in)
{
    u_char       *p, *ip, *af, *aflen;
    size_t        n, left;
    uint64_t      pcrb, pcre;
    ngx_buf_t    *b;
    ngx_chain_t  *out, *cl, **ll;

    left = 0;

    for (cl = in; cl; cl = cl->next) {
        left += cl->buf->last - cl->buf->pos;
    }

    ip = in->buf->pos;

    ll = &out;

    do {
        if (ts->free) {
            cl = ts->free;
            ts->free = cl->next;
            b = cl->buf;

        } else {
            ngx_log_debug1(NGX_LOG_DEBUG_CORE, ts->log, 0,
                           "ts alloc buffer:%d", NGX_TS_PACKET_SIZE);

            b = ngx_create_temp_buf(ts->pool, NGX_TS_PACKET_SIZE);
            if (b == NULL) {
                return NULL;
            }

            cl = ngx_alloc_chain_link(ts->pool);
            if (cl == NULL) {
                return NULL;
            }

            cl->buf = b;
        }

        b->pos = b->start;
        b->last = b->end;

        p = b->pos;

        /* sync_byte */
        *p++ = 0x47;

        /* payload_unit_start_indicator, PID */
        *p++ = (ll == &out ? 0x40 : 0x00) | (u_char) (h->pid >> 8);
        *p++ = (u_char) h->pid;

        /* payload_present, continuity_counter */
        af = p;
        *p++ = 0x10 | h->cont;
        h->cont = (h->cont + 1) & 0x0f;

        if (h->rand || h->pcrf || (h->stuff && left < 184)) {
            /* adaptation_field_control */
            *af |= 0x20;

            /* adaptation_field_length */
            aflen = p;
            *p++ = 1;

            /* random_access_indicator, PCR_flag */
            *p++ = (h->rand ? 0x40 : 0x00) | (h->pcrf ? 0x10 : 0x00);

            if (h->pcrf) {
                pcrb = h->pcr / 300;
                pcre = h->pcr % 300;

                /*
                 * program_clock_reference_base,
                 * program_clock_reference_extension
                 */
                *p++ = (u_char) (pcrb >> 25);
                *p++ = (u_char) (pcrb >> 17);
                *p++ = (u_char) (pcrb >> 9);
                *p++ = (u_char) (pcrb >> 1);
                *p++ = (u_char) (pcrb << 7) | (u_char) ((pcre >> 8) & 0x01);
                *p++ = (u_char) pcre;

                *aflen += 6;
            }

            if (h->stuff && left < (size_t) (b->end - p)) {
                n = b->end - p - left;

                /* stuffing_byte */
                ngx_memset(p, '\xff', n);
                p += n;
                *aflen += n;
            }
        }

        /* data_byte */

        while (p != b->end) {
            n = ngx_min(in->buf->last - ip, b->end - p);

            p = ngx_cpymem(p, ip, n);

            left -= n;
            ip += n;

            if (ip == in->buf->last) {
                in = in->next;
                if (in == NULL) {
                    ngx_memset(p, '\xff', b->end - p);
                    break;
                }

                ip = in->buf->pos;
            }
        }

        h->rand = 0;
        h->pcrf = 0;

        *ll = cl;
        ll = &cl->next;

    } while (in);

    *ll = NULL;

    return out;
}


static ngx_int_t
ngx_ts_free_buf(ngx_ts_stream_t *ts, ngx_buf_t *b)
{
    ngx_chain_t  *cl;

    cl = ngx_alloc_chain_link(ts->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = ts->free;
    ts->free = cl;

    return NGX_OK;
}


void
ngx_ts_free_chain(ngx_ts_stream_t *ts, ngx_chain_t **ll)
{
    ngx_chain_t  **fl;

    if (*ll == NULL) {
        return;
    }

    fl = ll;

    while (*ll) {
        ll = &(*ll)->next;
    }

    *ll = ts->free;
    ts->free = *fl;

    *fl = NULL;
}


static ngx_int_t
ngx_ts_append_buf(ngx_ts_stream_t *ts, ngx_ts_header_t *h, ngx_chain_t **ll,
    ngx_buf_t *b)
{
    ngx_chain_t  *cl;

    if (b == NULL) {
        return NGX_OK;
    }

    if (!h->pusi && *ll == NULL) {
        ngx_log_error(NGX_LOG_INFO, ts->log, 0, "dropping orhaned TS packet");
        return ngx_ts_free_buf(ts, b);
    }

    if (h->pusi && *ll) {
        ngx_log_error(NGX_LOG_INFO, ts->log, 0,
                      "dropping unfinished TS packets");
        ngx_ts_free_chain(ts, ll);
    }

    while (*ll) {
        ll = &(*ll)->next;
    }

    cl = ngx_alloc_chain_link(ts->pool);
    if (cl == NULL) {
        return NGX_ERROR;
    }

    cl->buf = b;
    cl->next = NULL;

    *ll = cl;

    return NGX_OK;
}


static uint32_t
ngx_ts_crc32(u_char *p, size_t len)
{
    uint32_t  crc;

    crc = 0xffffffff;

    while (len--) {
        crc = ngx_ts_crc32_table_ieee[(crc ^ *p++) & 0xff] ^ (crc >> 8);
    }

    return crc;
}


ngx_uint_t
ngx_ts_dash_get_oti(u_char type)
{
    /*
     * ISO/IEC 14496-1:2001(E)
     * Table 8 - objectTypeIndication Values, p. 30
     */

    switch (type) {
    case NGX_TS_VIDEO_MPEG1:
        return 0x6a;

    case NGX_TS_VIDEO_MPEG2:
        /* treat as Main Profile */
        return 0x61;

    case NGX_TS_VIDEO_MPEG4:
        return 0x20;

    case NGX_TS_VIDEO_AVC:
        return 0x21;

    case NGX_TS_AUDIO_MPEG1:
        return 0x6b;

    case NGX_TS_AUDIO_MPEG2:
        return 0x69;

    case NGX_TS_AUDIO_AAC:
        /* consider as ISO/IEC 14496-3 Audio */
        return 0x40;

    default:
        return 0;
    }
}
