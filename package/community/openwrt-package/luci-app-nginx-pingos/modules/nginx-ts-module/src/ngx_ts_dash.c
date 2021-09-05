
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>

#include "ngx_ts_dash.h"


#define NGX_TS_DASH_DATETIME_LEN  sizeof("2000-12-31T23:59:59Z")
#define NGX_TS_DASH_CODEC_LEN     sizeof("avc1.PPPCCCLLL")


static void ngx_ts_dash_cleanup(void *data);
static ngx_int_t ngx_ts_dash_handler(ngx_ts_handler_data_t *hd);
static ngx_int_t ngx_ts_dash_pmt_handler(ngx_ts_dash_t *dash);
static ngx_int_t ngx_ts_dash_pes_handler(ngx_ts_dash_t *dash,
    ngx_ts_program_t *prog, ngx_ts_es_t *es, ngx_chain_t *in);
static void ngx_ts_dash_update_bandwidth(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep, ngx_chain_t *in, uint64_t dts);
static ngx_int_t ngx_ts_dash_copy_avc(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep, ngx_chain_t *in);
static ngx_int_t ngx_ts_dash_copy_aac(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep, ngx_chain_t *in);
static ngx_int_t ngx_ts_dash_copy_default(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep, ngx_chain_t *in);
static ngx_int_t ngx_ts_dash_close_segment(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep);
static ngx_int_t ngx_ts_dash_update_playlist(ngx_ts_dash_t *dash);
static ngx_int_t ngx_ts_dash_write_file(u_char *path1, u_char *path2,
    u_char *data, size_t len, ngx_log_t *log);
static void ngx_ts_dash_format_datetime(u_char *p, time_t t);
static void ngx_ts_dash_format_codec(u_char *p, ngx_ts_dash_rep_t *rep);
static ngx_int_t ngx_ts_dash_update_init_segments(ngx_ts_dash_t *dash);
static ngx_int_t ngx_ts_dash_open_segment(ngx_ts_dash_t *dash,
    ngx_ts_dash_rep_t *rep);

static ngx_msec_t ngx_ts_dash_file_manager(void *data);
static ngx_int_t ngx_ts_dash_manage_file(ngx_tree_ctx_t *ctx, ngx_str_t *path);
static ngx_int_t ngx_ts_dash_manage_directory(ngx_tree_ctx_t *ctx,
    ngx_str_t *path);
static ngx_int_t ngx_ts_dash_delete_directory(ngx_tree_ctx_t *ctx,
    ngx_str_t *path);
static ngx_int_t ngx_ts_dash_delete_file(ngx_tree_ctx_t *ctx, ngx_str_t *path);


ngx_ts_dash_t *
ngx_ts_dash_create(ngx_ts_dash_conf_t *conf, ngx_ts_stream_t *ts,
    ngx_str_t *name)
{
    size_t               len;
    ngx_ts_dash_t       *dash;
    ngx_pool_cleanup_t  *cln;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts dash create");

    dash = ngx_pcalloc(ts->pool, sizeof(ngx_ts_dash_t));
    if (dash == NULL) {
        return NULL;
    }

    dash->conf = conf;
    dash->ts = ts;
    dash->playlist_len = 128;
    dash->init_seg_len = 256;

    dash->path.len = conf->path->name.len + 1 + name->len;
    dash->path.data = ngx_pnalloc(ts->pool, dash->path.len + 1);
    if (dash->path.data == NULL) {
        return NULL;
    }

    ngx_sprintf(dash->path.data, "%V/%V%Z", &conf->path->name, name);

    /* index.mpd */

    len = dash->path.len + sizeof("/index.mpd");

    dash->mpd_path = ngx_pnalloc(ts->pool, len);
    if (dash->mpd_path == NULL) {
        return NULL;
    }

    ngx_sprintf(dash->mpd_path, "%V/index.mpd%Z", &dash->path);

    /* index.mpd.tmp */

    len += sizeof(".tmp") - 1;

    dash->mpd_tmp_path = ngx_pnalloc(ts->pool, len);
    if (dash->mpd_tmp_path == NULL) {
        return NULL;
    }

    ngx_sprintf(dash->mpd_tmp_path, "%s.tmp%Z", dash->mpd_path);

    cln = ngx_pool_cleanup_add(ts->pool, 0);
    if (cln == NULL) {
        return NULL;
    }

    cln->handler = ngx_ts_dash_cleanup;
    cln->data = dash;

    if (ngx_ts_add_handler(ts, ngx_ts_dash_handler, dash) != NGX_OK) {
        return NULL;
    }

    return dash;
}


static void
ngx_ts_dash_cleanup(void *data)
{
    ngx_ts_dash_t *dash = data;

    ngx_uint_t          i, j;
    ngx_ts_dash_set_t  *set;
    ngx_ts_dash_rep_t  *rep;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, dash->ts->log, 0, "ts dash cleanup");

    dash->flush = 1;

    for (i = 0; i < dash->nsets; i++) {
        set = &dash->sets[i];

        for (j = 0; j < set->nreps; j++) {
            rep =  &set->reps[j];

            if (ngx_ts_dash_close_segment(dash, rep) != NGX_OK) {
                return;
            }
        }
    }
}


static ngx_int_t
ngx_ts_dash_handler(ngx_ts_handler_data_t *hd)
{
    ngx_ts_dash_t *dash = hd->data;

    switch (hd->event) {

    case NGX_TS_PMT:
        return ngx_ts_dash_pmt_handler(dash);

    case NGX_TS_PES:
        return ngx_ts_dash_pes_handler(dash, hd->prog, hd->es, hd->bufs);

    default:
        return NGX_OK;
    }
}


static ngx_int_t
ngx_ts_dash_pmt_handler(ngx_ts_dash_t *dash)
{
    size_t              len;
    ngx_uint_t          i, j, n;
    ngx_ts_es_t        *es;
    ngx_ts_stream_t    *ts;
    ngx_ts_program_t   *prog;
    ngx_ts_dash_rep_t  *rep;
    ngx_ts_dash_set_t  *set, *aset, *vset;

    if (dash->sets) {
        return NGX_OK;
    }

    ts = dash->ts;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0, "ts dash pmt");

    n = 0;

    for (i = 0; i < ts->nprogs; i++) {
        prog = &ts->progs[i];

        if (prog->es == NULL) {
            return NGX_OK;
        }

        n += prog->nes;
    }

    dash->nsets = 0;
    dash->sets = ngx_pcalloc(ts->pool, 2 * sizeof(ngx_ts_dash_set_t));
    if (dash->sets == NULL) {
        return NGX_ERROR;
    }

    aset = NULL;
    vset = NULL;

    for (i = 0; i < ts->nprogs; i++) {
        prog = &ts->progs[i];

        for (j = 0; j < prog->nes; j++) {
            es = &prog->es[j];

            switch (es->type) {
            case NGX_TS_VIDEO_MPEG1:
            case NGX_TS_VIDEO_MPEG2:
            case NGX_TS_VIDEO_MPEG4:
            case NGX_TS_VIDEO_AVC:
                if (vset == NULL) {
                    vset = &dash->sets[dash->nsets++];
                    vset->video = 1;
                }

                set = vset;
                break;

            case NGX_TS_AUDIO_MPEG1:
            case NGX_TS_AUDIO_MPEG2:
            case NGX_TS_AUDIO_AAC:
                if (aset == NULL) {
                    aset = &dash->sets[dash->nsets++];
                }

                set = aset;
                break;

            default:
                continue;
            }

            if (set->reps == NULL) {
                set->nreps = 0;
                set->reps = ngx_pcalloc(ts->pool,
                                        n * sizeof(ngx_ts_dash_rep_t));
                if (set->reps == NULL) {
                    return NGX_ERROR;
                }
            }

            rep = &set->reps[set->nreps++];

            rep->es = es;

            rep->nsegs = dash->conf->nsegs;
            rep->segs = ngx_pcalloc(ts->pool,
                                    rep->nsegs * sizeof(ngx_ts_dash_segment_t));
            if (rep->segs == NULL) {
                return NGX_ERROR;
            }

            len = dash->path.len + 1 + NGX_INT_T_LEN + 1 + NGX_INT64_LEN
                  + sizeof(".mp4");

            rep->path.data = ngx_pnalloc(ts->pool, len);
            if (rep->path.data == NULL) {
                return NGX_ERROR;
            }

            rep->path.len = ngx_sprintf(rep->path.data, "%V/%ui.",
                                        &dash->path, (ngx_uint_t) es->pid)
                            - rep->path.data;

            /* init.mp4 */

            len = dash->path.len + 1 + NGX_INT_T_LEN + sizeof(".init.mp4");

            rep->init_path = ngx_pnalloc(ts->pool, len);
            if (rep->init_path == NULL) {
                return NGX_ERROR;
            }

            ngx_sprintf(rep->init_path, "%V/%ui.init.mp4%Z",
                        &dash->path, (ngx_uint_t) es->pid);

            /* init.mp4.tmp */

            len += sizeof(".tmp") - 1;

            rep->init_tmp_path = ngx_pnalloc(ts->pool, len);
            if (rep->init_tmp_path == NULL) {
                return NGX_ERROR;
            }

            ngx_sprintf(rep->init_tmp_path, "%s.tmp%Z", rep->init_path);
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_pes_handler(ngx_ts_dash_t *dash, ngx_ts_program_t *prog,
    ngx_ts_es_t *es, ngx_chain_t *in)
{
    ngx_uint_t          i, j;
    ngx_ts_dash_set_t  *set;
    ngx_ts_dash_rep_t  *rep;

    if (!es->ptsf) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, dash->ts->log, 0, "ts dash pes pid:%ud",
                   (unsigned) es->pid);

    for (i = 0; i < dash->nsets; i++) {
        set = &dash->sets[i];

        for (j = 0; j < set->nreps; j++) {
            rep =  &set->reps[j];

            if (rep->es == es) {
                goto found;
            }
        }
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                   "ts dash skip es pid:%ud", (unsigned) es->pid);

    return NGX_OK;

found:

    ngx_ts_dash_update_bandwidth(dash, rep, in, es->dts);

    if (ngx_ts_dash_close_segment(dash, rep) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_ts_dash_open_segment(dash, rep) != NGX_OK) {
        return NGX_ERROR;
    }

    switch (es->type) {
    case NGX_TS_VIDEO_AVC:
        return ngx_ts_dash_copy_avc(dash, rep, in);

    case NGX_TS_AUDIO_AAC:
        return ngx_ts_dash_copy_aac(dash, rep, in);
/*
    case NGX_TS_AUDIO_MPEG1:
    case NGX_TS_AUDIO_MPEG2:
        return ngx_ts_dash_copy_mp3(dash, rep, in);
*/

    default:
        return ngx_ts_dash_copy_default(dash, rep, in);
    }
}


static void
ngx_ts_dash_update_bandwidth(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *in, uint64_t dts)
{
    int64_t  d, analyze;

    if (rep->bandwidth) {
        return;
    }

    if (rep->bandwidth_bytes == 0) {
        rep->bandwidth_dts = dts;
    }

    while (in) {
        rep->bandwidth_bytes += in->buf->last - in->buf->pos;
        in = in->next;
    }

    d = dts - rep->bandwidth_dts;
    analyze = (int64_t) dash->conf->analyze * 90;

    if (d >= analyze) {
        rep->bandwidth = rep->bandwidth_bytes * 8 * 90000 / d;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                   "ts dash bandwidth:%ui, pid:%ud",
                   rep->bandwidth, (unsigned) rep->es->pid);
}


static ngx_int_t
ngx_ts_dash_copy_avc(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *in)
{
    /*
     * Source format:
     * ISO/IEC 14496-10:2004(E)
     * Annex B. Byte Stream Format, p. 216
     *
     * Destination format:
     * ISO/IEC 14496-15:2004(E)
     * 5.3.4.2 Sample format, p. 15
     */

    size_t        n, size, len;
    u_char       *p, *s, *spec, c, type, buf[4];
    ngx_uint_t    z;
    ngx_chain_t  *cl;

    if (in == NULL) {
        return NGX_OK;
    }

    p = in->buf->pos;

    size = 0;

    while (in) {
        z = 0;
        len = 0;
        type = 0;
        cl = in;
        s = p;

        for ( ;; ) {
            if (p == in->buf->last) {
                in = in->next;
                if (in == NULL) {
                    break;
                }

                p = in->buf->pos;
                continue;
            }

            c = *p++;

            if (c == 0) {
                z++;
                continue;
            }

            if (c == 1 && z >= 2) {
                break;
            }

            if (z >= 3) {
                goto failed;
            }

            if (len == 0) {
                type = z ? 0 : (c & 0x1f);
            }

            len += z + 1;
            z = 0;
        }

        if (len == 0) {
            continue;
        }

        if (type == 7 || type == 8) {
            if ((type == 7 && rep->sps) || (type == 8 && rep->pps)) {
                continue;
            }

            if (len > 0xffff) {
                goto failed;
            }

            spec = ngx_pnalloc(dash->ts->pool, len);
            if (spec == NULL) {
                return NGX_ERROR;
            }

            if (type == 7) {
                rep->sps = spec;
                rep->sps_len = len;

            } else if (type == 8) {
                rep->pps = spec;
                rep->pps_len = len;
            }

            while (len) {
                if (s == cl->buf->last) {
                    cl = cl->next;
                    s = cl->buf->pos;
                }

                n = ngx_min((size_t) (cl->buf->last - s), len);

                spec = ngx_cpymem(spec, s, n);

                s += n;
                len -= n;
            }

            if (rep->sps && rep->pps) {
                rep->avc = ngx_ts_avc_decode_params(dash->ts,
                                                    rep->sps, rep->sps_len,
                                                    rep->pps, rep->pps_len);
                if (rep->avc == NULL) {
                    return NGX_ERROR;
                }
            }

            continue;
        }

        buf[0] = len >> 24;
        buf[1] = len >> 16;
        buf[2] = len >> 8;
        buf[3] = len;

        if (ngx_ts_dash_append_data(dash, rep, buf, 4) != NGX_OK) {
            return NGX_ERROR;
        }

        size += 4;

        while (len) {
            if (s == cl->buf->last) {
                cl = cl->next;
                s = cl->buf->pos;
            }

            n = ngx_min((size_t) (cl->buf->last - s), len);

            if (ngx_ts_dash_append_data(dash, rep, s, n) != NGX_OK) {
                return  NGX_ERROR;
            }

            size += n;
            s += n;
            len -= n;
        }
    }

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                   "ts dash AVC frame:%uz, dts:%uL", size, rep->es->dts);

    return ngx_ts_dash_append_meta(dash, rep, size, rep->es->dts);

failed:

    ngx_log_error(NGX_LOG_ERR, dash->ts->log, 0, "invalid AVC frame");

    return NGX_ERROR;

}


static ngx_int_t
ngx_ts_dash_copy_aac(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *in)
{
    /*
     * XXX
     * ISO/IEC CD 14496-3 Subpart 4: 1998
     * 1.1.2 Audio_Data_Transport_Stream frame, ADTS, p. 10
     */

    size_t       len, n;
    u_char      *p, adts[9];
    uint64_t     dts;
    ngx_uint_t   i;

    if (in == NULL) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, dash->ts->log, 0, "ts dash AAC ts:%uL",
                   rep->es->dts);

    i = 0;

    p = in->buf->pos;

    for ( ;; ) {
        n = 0;

        /* protection_absent */
        adts[1] = 0;

        while (n < ((adts[1] & 0x01) ? 7 : 9 )) {
            if (p == in->buf->last) {
                in = in->next;

                if (in == NULL) {
                    if (n == 0) {
                        return NGX_OK;
                    }

                    goto failed;
                }

                p = in->buf->pos;
                continue;
            }

            adts[n++] = *p++;
        }

        if (rep->aac == NULL) {
            rep->aac = ngx_ts_aac_decode_params(dash->ts, adts, 7);
            if (rep->aac == NULL) {
                return NGX_ERROR;
            }
        }

        len = adts[3] & 0x03;
        len = (len << 8) + adts[4];
        len = (len << 3) + (adts[5] >> 5);

        if (len < n) {
            goto failed;
        }

        len -= n;

        dts = rep->es->dts + (uint64_t) 90000 * 1024 * i++ / rep->aac->freq;

        ngx_log_debug3(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                       "ts dash AAC adts:%uz, frame:%uz, ts:%uL", n, len, dts);

        if (ngx_ts_dash_append_meta(dash, rep, len, dts) != NGX_OK) {
            return NGX_ERROR;
        }

        while (len) {
            if (p == in->buf->last) {
                in = in->next;
                if (in == NULL) {
                    goto failed;
                }

                p = in->buf->pos;
            }

            n = ngx_min((size_t) (in->buf->last - p), len);

            if (ngx_ts_dash_append_data(dash, rep, p, n) != NGX_OK) {
                return  NGX_ERROR;
            }

            p += n;
            len -= n;
        }
    }

failed:

    ngx_log_error(NGX_LOG_ERR, dash->ts->log, 0, "invalid AAC ADTS frame");

    return NGX_ERROR;
}


static ngx_int_t
ngx_ts_dash_copy_default(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep,
    ngx_chain_t *in)
{
    size_t  n, size;

    for (size = 0; in; in = in->next) {
        n = in->buf->last - in->buf->pos;

        if (ngx_ts_dash_append_data(dash, rep, in->buf->pos, n) != NGX_OK) {
            return  NGX_ERROR;
        }

        size += n;
    }

    return ngx_ts_dash_append_meta(dash, rep, size, rep->es->dts);
}


static ngx_int_t
ngx_ts_dash_close_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep)
{
    size_t                  max_size;
    ssize_t                 n;
    int64_t                 d, min_seg, max_seg;
    ngx_err_t               err;
    ngx_str_t              *path;
    ngx_file_t              file;
    ngx_uint_t              try;
    ngx_chain_t            *out;
    ngx_ts_es_t            *es;
    ngx_ts_stream_t        *ts;
    ngx_ts_dash_segment_t  *seg;

    if (rep->meta == NULL) {
        return NGX_OK;
    }

    ts = dash->ts;

    es = rep->es;

    d = es->dts - rep->seg_dts;

    min_seg = (int64_t) dash->conf->min_seg * 90;
    max_seg = (int64_t) dash->conf->max_seg * 90;
    max_size = dash->conf->max_size;

    if (!dash->flush) {
        if (d < min_seg
            || (d < max_seg && es->video && !es->rand))
        {
            if (max_size == 0 || rep->nmeta + rep->ndata < max_size) {
                return NGX_OK;
            }

            ngx_log_error(NGX_LOG_WARN, ts->log, 0,
                          "closing DASH segment \"%V%uL.mp4\" on size limit",
                          &rep->path, rep->seg_dts);
        }
    }

    path = &rep->path;

    ngx_memzero(&file, sizeof(ngx_file_t));

    file.log = ts->log;
    file.name.data = path->data;
    file.name.len = ngx_sprintf(path->data + path->len, "%uL.mp4%Z",
                                rep->seg_dts)
                    - path->data - 1;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ts->log, 0,
                   "ts dash close segment \"%s\"", file.name.data);

    for (try = 0; /* void */; try++) {
        file.fd = ngx_open_file(path->data,
                                NGX_FILE_WRONLY,
                                NGX_FILE_TRUNCATE,
                                NGX_FILE_DEFAULT_ACCESS);

        if (file.fd != NGX_INVALID_FILE) {
            break;
        }

        err = ngx_errno;

        if (try || (err != NGX_ENOENT && err != NGX_ENOTDIR)) {
            ngx_log_error(NGX_LOG_EMERG, ts->log, err,
                          ngx_open_file_n " \"%s\" failed", path->data);
            return NGX_ERROR;
        }

        /* XXX dir access mode */
        if (ngx_create_dir(dash->path.data, 0700) == NGX_FILE_ERROR) {
            err = ngx_errno;

            if (err != NGX_EEXIST) {
                ngx_log_error(NGX_LOG_CRIT, ts->log, err,
                              ngx_create_dir_n " \"%s\" failed",
                              dash->path.data);
                return NGX_ERROR;
            }
        }
    }

    out = ngx_ts_dash_end_segment(dash, rep);

    n = ngx_write_chain_to_file(&file, out, 0, ts->pool);

    if (ngx_close_file(file.fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, ts->log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", path->data);
    }

    ngx_ts_dash_free_segment(dash, rep, out);

    if (n == NGX_ERROR) {
        return NGX_ERROR;
    }

    seg = &rep->segs[rep->seg++ % rep->nsegs];
    seg->start = rep->seg_dts;
    seg->duration = d;

    if (ngx_ts_dash_update_playlist(dash) != NGX_OK) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_update_playlist(ngx_ts_dash_t *dash)
{
    u_char                 *p, *last, *data;
    time_t                  now;
    size_t                  len;
    ngx_uint_t              i, j, k, pid, bandwidth, min_update, min_buftime,
                            buf_depth;
    ngx_ts_stream_t        *ts;
    ngx_ts_dash_set_t      *set;
    ngx_ts_dash_rep_t      *rep;
    ngx_ts_dash_segment_t  *seg;
    u_char                  codec[NGX_TS_DASH_CODEC_LEN];
    u_char                  avail_start_time[NGX_TS_DASH_DATETIME_LEN];
    u_char                  pub_time[NGX_TS_DASH_DATETIME_LEN];

    if (ngx_ts_dash_update_init_segments(dash) != NGX_OK) {
        return NGX_ERROR;
    }

    ts = dash->ts;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ts->log, 0,
                   "ts dash update playlist \"%s\"", dash->mpd_path);

    now = ngx_time();

    if (dash->availability_start == 0) {
        dash->availability_start = now;
    }

    ngx_ts_dash_format_datetime(avail_start_time, dash->availability_start);
    ngx_ts_dash_format_datetime(pub_time, now);

    /*
     *                 timeShiftBufferDepth
     *       ----------------------------------------
     *      |                                        |
     * -----///////----------------*-----------------> now
     *            |                |                 |
     *             ---------------- -----------------
     *                 liveDelay     lastSegDuration
     *           = 2 * minBufferTime
     *
     */

    min_update = dash->conf->min_seg / 1000;
    min_buftime = dash->conf->min_seg / 1000;
    buf_depth = 2 * min_buftime + dash->conf->max_seg / 1000 + 1;

    for ( ;; ) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, ts->log, 0,
                       "ts dash playlist len:%uz", dash->playlist_len);

        data = ngx_alloc(dash->playlist_len, ts->log);
        if (data == NULL) {
            return NGX_ERROR;
        }

        p = data;
        last = data + dash->playlist_len;

        p = ngx_slprintf(p, last,
                "<?xml version=\"1.0\"?>\n"
                "<MPD\n"
                "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
                "    xmlns=\"urn:mpeg:DASH:schema:MPD:2011\"\n"
                "    xsi:schemaLocation=\"urn:mpeg:DASH:schema:MPD:2011 "
                                                             "DASH-MPD.xsd\"\n"
                "    type=\"dynamic\"\n"
                "    availabilityStartTime=\"%s\"\n"
                "    publishTime=\"%s\"\n"
                "    minimumUpdatePeriod=\"PT%uiS\"\n"
                "    minBufferTime=\"PT%uiS\"\n"
                "    timeShiftBufferDepth=\"PT%uiS\"\n"
                "    profiles=\"urn:hbbtv:dash:profile:isoff-live:2012,"
                               "urn:mpeg:dash:profile:isoff-live:2011\">\n"
                "  <Period\n"
                "      id=\"0\"\n"
                "      start=\"PT0S\">\n",
                avail_start_time, pub_time, min_update, min_buftime, buf_depth);

        for (i = 0; i < dash->nsets; i++) {
            set = &dash->sets[i];

            p = ngx_slprintf(p, last,
                    "    <AdaptationSet\n"
                    "        segmentAlignment=\"true\"\n"
                    "        mimeType=\"%s/mp4\">\n",
                    set->video ? "video" : "audio");

            for (j = 0; j < set->nreps; j++) {
                rep = &set->reps[j];

                if (rep->bandwidth == 0) {
                    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ts->log, 0,
                                   "ts dash bandwidth not available");
                    ngx_free(data);
                    return NGX_OK;
                }

                pid = rep->es->pid;
                bandwidth = rep->bandwidth;

                ngx_ts_dash_format_codec(codec, rep);

                p = ngx_slprintf(p, last,
                        "      <Representation\n"
                        "          id=\"%ui\"\n"
                        "          codecs=\"%s\"\n"
                        "          bandwidth=\"%ui\">\n"
                        "        <SegmentTemplate\n"
                        "            timescale=\"90000\"\n"
                        "            media=\"$RepresentationID$.$Time$.mp4\"\n"
                        "            initialization="
                                           "\"$RepresentationID$.init.mp4\">\n"
                        "          <SegmentTimeline>\n",
                        pid, codec, bandwidth);

                for (k = 0; k < rep->nsegs; k++) {
                    seg = &rep->segs[(rep->seg + k) % rep->nsegs];

                    if (seg->duration) {
                        p = ngx_slprintf(p, last,
                                "            <S t=\"%uL\" d=\"%uL\"/>\n",
                                seg->start, seg->duration);
                    }
                }

                p = ngx_slprintf(p, last,
                        "          </SegmentTimeline>\n"
                        "        </SegmentTemplate>\n"
                        "      </Representation>\n");
            }

            p = ngx_slprintf(p, last,
                    "    </AdaptationSet>\n");
        }

        p = ngx_slprintf(p, last,
                "  </Period>\n"
                "</MPD>\n");

        if (p != last) {
            break;
        }

        ngx_free(data);

        dash->playlist_len *= 2;
    }

    len = p - data;

    if (ngx_ts_dash_write_file(dash->mpd_tmp_path, dash->mpd_path, data, len,
                               ts->log)
        != NGX_OK)
    {
        ngx_free(data);
        return NGX_ERROR;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_write_file(u_char *path1, u_char *path2, u_char *data, size_t len,
    ngx_log_t *log)
{
    ssize_t    n;
    ngx_fd_t   fd;
    ngx_err_t  err;

    fd = ngx_open_file(path1,
                       NGX_FILE_WRONLY,
                       NGX_FILE_TRUNCATE,
                       NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_EMERG, log, ngx_errno,
                      ngx_open_file_n " \"%s\" failed", path1);
        return NGX_ERROR;
    }

    n = ngx_write_fd(fd, data, len);

    err = errno;

    if (ngx_close_file(fd) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                      ngx_close_file_n " \"%s\" failed", path1);
        return NGX_ERROR;
    }

    if (n < 0) {
        ngx_log_error(NGX_LOG_ALERT, log, err,
                      ngx_write_fd_n " to \"%s\" failed", path1);
        return NGX_ERROR;
    }

    if ((size_t) n != len) {
        ngx_log_error(NGX_LOG_ALERT, log, 0,
                      "incomplete write to \"%s\"", path1);
        return NGX_ERROR;
    }

    if (path2) {
        if (ngx_rename_file(path1, path2) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ALERT, log, ngx_errno,
                          ngx_rename_file_n " \"%s\" to \"%s\" failed",
                          path1, path2);
            return NGX_ERROR;
        }
    }

    return NGX_OK;
}


static void
ngx_ts_dash_format_datetime(u_char *p, time_t t)
{
    struct tm  tm;

    ngx_libc_gmtime(t, &tm);

    if (strftime((char *) p, NGX_TS_DASH_DATETIME_LEN, "%Y-%m-%dT%H:%M:%SZ",
                 &tm)
        == 0)
    {
        *p = 0;
    }
}


static void
ngx_ts_dash_format_codec(u_char *p, ngx_ts_dash_rep_t *rep)
{
    ngx_uint_t  oti;

    if (rep->avc) {
        ngx_sprintf(p, "avc1.%02uXi%02uXi%02uXi%Z",
                    rep->avc->profile_idc,
                    rep->avc->constraints,
                    rep->avc->level_idc);
        return;
    }

    if (rep->aac) {
        ngx_sprintf(p, "mp4a.40.%ui%Z", rep->aac->profile);
        return;
    }

    oti = ngx_ts_dash_get_oti(rep->es->type);

    ngx_sprintf(p, "mp4%c.%02uXi%Z", rep->es->video ? 'v' : 'a', oti);
}


static ngx_int_t
ngx_ts_dash_update_init_segments(ngx_ts_dash_t *dash)
{
    u_char             *data;
    ngx_buf_t           out;
    ngx_uint_t          i, j;
    ngx_ts_stream_t    *ts;
    ngx_ts_dash_set_t  *set;
    ngx_ts_dash_rep_t  *rep;

    ts = dash->ts;

    for (i = 0; i < dash->nsets; i++) {
        set = &dash->sets[i];

        for (j = 0; j < set->nreps; j++) {
            rep = &set->reps[j];

            if (rep->bandwidth == 0) {
                continue;
            }

            for ( ;; ) {
                ngx_log_debug2(NGX_LOG_DEBUG_CORE, ts->log, 0,
                               "ts dash write init segment \"%s\", buf:%uz",
                               rep->init_path, dash->init_seg_len);

                ngx_memzero(&out, sizeof(ngx_buf_t));

                data = ngx_alloc(dash->init_seg_len, ts->log);
                if (data == NULL) {
                    return NGX_ERROR;
                }

                out.start = data;
                out.pos = data;
                out.last = data;
                out.end = data + dash->init_seg_len;

                ngx_ts_dash_write_init_segment(&out, rep);

                if (out.last < out.end) {
                    break;
                }

                ngx_free(data);

                dash->init_seg_len *= 2;
            }

            if (ngx_ts_dash_write_file(rep->init_tmp_path, rep->init_path,
                                       out.pos, out.last - out.pos, ts->log)
                != NGX_OK)
            {
                ngx_free(data);
                return NGX_ERROR;
            }

            ngx_free(data);
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_open_segment(ngx_ts_dash_t *dash, ngx_ts_dash_rep_t *rep)
{
    ngx_ts_es_t  *es;

    if (rep->meta) {
        return NGX_OK;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_CORE, dash->ts->log, 0,
                   "ts dash open segment \"%V%uL.mp4\"",
                   &rep->path, rep->seg_dts);

    es = rep->es;

    rep->seg_pts = es->pts;
    rep->seg_dts = es->dts;

    return ngx_ts_dash_start_segment(dash, rep);
}


static ngx_msec_t
ngx_ts_dash_file_manager(void *data)
{
    ngx_ts_dash_conf_t *dash = data;

    ngx_tree_ctx_t  tree;

    ngx_log_debug0(NGX_LOG_DEBUG_CORE, ngx_cycle->log, 0,
                  "ts dash file manager");

    tree.init_handler = NULL;
    tree.file_handler = ngx_ts_dash_manage_file;
    tree.pre_tree_handler = ngx_ts_dash_manage_directory;
    tree.post_tree_handler = ngx_ts_dash_delete_directory;
    tree.spec_handler = ngx_ts_dash_delete_file;
    tree.data = dash;
    tree.alloc = 0;
    tree.log = ngx_cycle->log;

    (void) ngx_walk_tree(&tree, &dash->path->name);

    return dash->max_seg * dash->nsegs;
}


static ngx_int_t
ngx_ts_dash_manage_file(ngx_tree_ctx_t *ctx, ngx_str_t *path)
{
    ngx_ts_dash_conf_t *dash = ctx->data;

    time_t  age, max_age;

    age = ngx_time() - ctx->mtime;

    max_age = 0;

    if (path->len >= 4
        && ngx_memcmp(path->data + path->len - 4, ".mpd", 4) == 0)
    {
        max_age = dash->max_seg * dash->nsegs / 1000;
    }

    if (path->len >= 4
        && ngx_memcmp(path->data + path->len - 4, ".mp4", 4) == 0)
    {
        max_age = dash->max_seg * dash->nsegs / 500;
    }

    if (path->len >= 4
        && ngx_memcmp(path->data + path->len - 4, ".tmp", 4) == 0)
    {
        max_age = 10;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_CORE, ctx->log, 0,
                   "ts dash file \"%s\", age:%T, max_age:%T",
                   path->data, age, max_age);

    if (age < max_age) {
        return NGX_OK;
    }

    return ngx_ts_dash_delete_file(ctx, path);
}


static ngx_int_t
ngx_ts_dash_manage_directory(ngx_tree_ctx_t *ctx, ngx_str_t *path)
{
    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_delete_directory(ngx_tree_ctx_t *ctx, ngx_str_t *path)
{
    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ctx->log, 0,
                   "ts dash delete dir: \"%s\"", path->data);

    /* non-empty directory will not be removed anyway */

    /* TODO count files instead */

    (void) ngx_delete_dir(path->data);

    return NGX_OK;
}


static ngx_int_t
ngx_ts_dash_delete_file(ngx_tree_ctx_t *ctx, ngx_str_t *path)
{
    ngx_log_debug1(NGX_LOG_DEBUG_CORE, ctx->log, 0,
                   "ts dash file delete: \"%s\"", path->data);

    if (ngx_delete_file(path->data) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_CRIT, ctx->log, ngx_errno,
                      ngx_delete_file_n " \"%s\" failed", path->data);
    }

    return NGX_OK;
}


char *
ngx_ts_dash_set_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char  *p = conf;

    ssize_t              max_size;
    ngx_str_t           *value, s, ss, path;
    ngx_int_t            v;
    ngx_uint_t           i, nsegs, clean;
    ngx_msec_t           min_seg, max_seg, analyze;
    ngx_ts_dash_conf_t  *dash, **field;

    field = (ngx_ts_dash_conf_t **) (p + cmd->offset);

    if (*field != NGX_CONF_UNSET_PTR) {
        return "is duplicate";
    }

    value = cf->args->elts;

    ngx_str_null(&path);

    min_seg = 5000;
    max_seg = 10000;
    analyze = 0;
    max_size = 16 * 1024 * 1024;
    nsegs = 6;
    clean = 1;

    for (i = 1; i < cf->args->nelts; i++) {

        if (ngx_strncmp(value[i].data, "path=", 5) == 0) {

            path.len = value[i].len - 5;
            path.data = value[i].data + 5;

            if (path.data[path.len - 1] == '/') {
                path.len--;
            }

            if (ngx_conf_full_name(cf->cycle, &path, 0) != NGX_OK) {
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "segment=", 8) == 0) {

            s.len = value[i].len - 8;
            s.data = value[i].data + 8;

            ss.data = (u_char *) ngx_strchr(s.data, ':');

            if (ss.data) {
                ss.len = s.data + s.len - ss.data - 1;
                s.len = ss.data - s.data;
                ss.data++;
            }

            min_seg = ngx_parse_time(&s, 0);
            if (min_seg == (ngx_msec_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid segment duration value \"%V\"",
                                   &value[i]);
                return NGX_CONF_ERROR;
            }

            if (ss.data) {
                max_seg = ngx_parse_time(&ss, 0);
                if (max_seg == (ngx_msec_t) NGX_ERROR) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "invalid segment duration value \"%V\"",
                                       &value[i]);
                    return NGX_CONF_ERROR;
                }

            } else {
                max_seg = min_seg * 2;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "analyze=", 8) == 0) {

            s.len = value[i].len - 8;
            s.data = value[i].data + 8;

            analyze = ngx_parse_time(&s, 0);
            if (analyze == (ngx_msec_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid analyze duration value \"%V\"",
                                   &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "max_size=", 9) == 0) {

            s.len = value[i].len - 9;
            s.data = value[i].data + 9;

            max_size = ngx_parse_size(&s);
            if (max_size == NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid max segment size value \"%V\"",
                                   &value[i]);
                return NGX_CONF_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(value[i].data, "segments=", 9) == 0) {

            v = ngx_atoi(value[i].data + 9, value[i].len - 9);
            if (v == NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "invalid segments number value \"%V\"",
                                   &value[i]);
                return NGX_CONF_ERROR;
            }

            nsegs = v;

            continue;
        }

        if (ngx_strcmp(value[i].data, "noclean") == 0) {
            clean = 0;
            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid parameter \"%V\"", &value[i]);
        return NGX_CONF_ERROR;
    }

    if (path.len == 0) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "\"%V\" must have \"path\" parameter", &cmd->name);
        return NGX_CONF_ERROR;
    }

    dash = ngx_pcalloc(cf->pool, sizeof(ngx_ts_dash_conf_t));
    if (dash == NULL) {
        return NGX_CONF_ERROR;
    }

    dash->path = ngx_pcalloc(cf->pool, sizeof(ngx_path_t));
    if (dash->path == NULL) {
        return NGX_CONF_ERROR;
    }

    dash->path->name = path;

    dash->min_seg = min_seg;
    dash->max_seg = max_seg;
    dash->analyze = analyze ? analyze : min_seg;
    dash->max_size = max_size;
    dash->nsegs = nsegs;

    if (clean) {
        dash->path->manager = ngx_ts_dash_file_manager;
    }

    dash->path->data = dash;
    dash->path->conf_file = cf->conf_file->file.name.data;
    dash->path->line = cf->conf_file->line;

    if (ngx_add_path(cf, &dash->path) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    *field = dash;

    return NGX_CONF_OK;
}
