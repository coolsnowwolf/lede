/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_codec_module.h"
#include "ngx_live_record.h"


ngx_live_record_start_pt            ngx_live_record_start;
ngx_live_record_update_pt           ngx_live_record_update;
ngx_live_record_done_pt             ngx_live_record_done;

static ngx_live_record_start_pt     next_record_start;
static ngx_live_record_update_pt    next_record_update;
static ngx_live_record_done_pt      next_record_done;

static ngx_rtmp_publish_pt          next_publish;
static ngx_rtmp_close_stream_pt     next_close_stream;


static ngx_int_t ngx_live_record_postconfiguration(ngx_conf_t *cf);
static void * ngx_live_record_create_app_conf(ngx_conf_t *cf);
static char * ngx_live_record_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);


#define NGX_LIVE_RECORD_BUFSIZE     (10*1024*1024)


typedef struct {
    ngx_flag_t                      record;
    ngx_str_t                       path;
    ngx_msec_t                      interval;

    ngx_msec_t                      min_fraglen;
    ngx_msec_t                      max_fraglen;

    size_t                          buffer;
} ngx_live_record_app_conf_t;


static ngx_command_t  ngx_live_record_commands[] = {

    { ngx_string("live_record"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, record),
      NULL },

    { ngx_string("live_record_path"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, path),
      NULL },

    { ngx_string("live_record_interval"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, interval),
      NULL },

    { ngx_string("live_record_min_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, min_fraglen),
      NULL },

    { ngx_string("live_record_max_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, max_fraglen),
      NULL },

    { ngx_string("live_record_buffer"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_live_record_app_conf_t, buffer),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_live_record_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_live_record_postconfiguration,      /* postconfiguration */
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_live_record_create_app_conf,        /* create app configuration */
    ngx_live_record_merge_app_conf          /* merge app configuration */
};


ngx_module_t  ngx_live_record_module = {
    NGX_MODULE_V1,
    &ngx_live_record_module_ctx,            /* module context */
    ngx_live_record_commands,               /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_live_record_create_app_conf(ngx_conf_t *cf)
{
    ngx_live_record_app_conf_t     *racf;

    racf = ngx_pcalloc(cf->pool, sizeof(ngx_live_record_app_conf_t));

    if (racf == NULL) {
        return NULL;
    }

    racf->record = NGX_CONF_UNSET;
    racf->interval = NGX_CONF_UNSET_MSEC;
    racf->min_fraglen = NGX_CONF_UNSET_MSEC;
    racf->max_fraglen = NGX_CONF_UNSET_MSEC;
    racf->buffer = NGX_CONF_UNSET_SIZE;

    return racf;
}


static char *
ngx_live_record_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_err_t                       err;
    ngx_live_record_app_conf_t     *prev;
    ngx_live_record_app_conf_t     *conf;
    u_char                          path[NGX_MAX_PATH + 1];

    prev = parent;
    conf = child;

    ngx_conf_merge_value(conf->record, prev->record, 0);
    ngx_conf_merge_str_value(conf->path, prev->path, "record");
    ngx_conf_merge_msec_value(conf->interval, prev->interval, 10 * 60 * 1000);
    ngx_conf_merge_msec_value(conf->min_fraglen, prev->min_fraglen, 8 * 1000);
    ngx_conf_merge_msec_value(conf->max_fraglen, prev->max_fraglen, 12 * 1000);
    ngx_conf_merge_msec_value(conf->buffer, prev->buffer, 1024 * 1024);

    if (conf->path.data[conf->path.len - 1] == '/') {
        --conf->path.len;
    }

    if (ngx_get_full_name(cf->pool, &cf->cycle->prefix, &conf->path)
            != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    *ngx_snprintf(path, sizeof(path) - 1, "%V/", &conf->path) = 0;
    err = ngx_create_full_path(path, 0755);
    if (err) {
        ngx_conf_log_error(NGX_LOG_CRIT, cf, err,
                ngx_create_dir_n " \"%s\" failed", path);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static ssize_t
ngx_live_record_flush(ngx_rtmp_mpegts_file_t *file)
{
    ssize_t                         rc;

    rc = ngx_write_fd(file->fd, file->wbuf.pos,
            file->wbuf.last - file->wbuf.pos);
    if (rc < 0) {
        ngx_log_error(NGX_LOG_ERR, file->log, ngx_errno,
                "flush record buf error");
        return rc;
    }

    file->file_size += rc;
    file->wbuf.last = file->wbuf.pos;

    return rc;
}


static ssize_t
ngx_live_record_write_buf(ngx_rtmp_mpegts_file_t *file, u_char *in,
        size_t in_size)
{
    u_char                         *p, *end;
    size_t                          len;
    ssize_t                         rc, n;

    end = in + in_size;
    n = 0;

    for (p = in; p != end; /* void */ ) {
        len = ngx_min(file->wbuf.end - file->wbuf.last, end - p);
        file->wbuf.last = ngx_cpymem(file->wbuf.last, p, len);

        p += len;
        n += len;

        if (file->wbuf.last == file->wbuf.end) {
            rc = ngx_live_record_flush(file);
            if (rc < 0) {
                return rc;
            }
        }
    }

    return n;
}


static ngx_int_t
ngx_live_record_open_file(ngx_rtmp_session_t *s)
{
    ngx_live_record_app_conf_t     *lracf;
    ngx_live_record_ctx_t          *ctx;
    ngx_err_t                       err;
    off_t                           file_size;
    size_t                          len;
    struct tm                       tm;
    u_char                         *p;
    ngx_rtmp_codec_ctx_t           *codec_ctx;

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_record_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    len = lracf->path.len + sizeof("/") - 1 + s->serverid.len + sizeof("/") - 1
        + s->app.len + sizeof("/") - 1 + s->name.len + sizeof("/") - 1
        + sizeof("YYYYMMDD/") - 1 + s->name.len
        + NGX_OFF_T_LEN + sizeof("_.ts") - 1;

    if (ctx->file.name.len == 0) { // first create in current session
        ctx->file.name.data = ngx_pcalloc(s->pool, len + 1);
        if (ctx->file.name.data == NULL) {
            ngx_log_error(NGX_LOG_CRIT, s->log, 0,
                    "record: alloc for ts name failed");
            return NGX_ERROR;
        }
    }

    // fill file name
    ngx_libc_localtime(ctx->last_time, &tm);

    p = ngx_snprintf(ctx->file.name.data, len,
            "%V/%V/%V/%V/%04d%02d%02d/%V_%d.ts",
            &lracf->path, &s->serverid, &s->app, &s->name,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            &s->name, ctx->last_time);
    *p = 0;
    ctx->file.name.len = p - ctx->file.name.data;

    // create dir
    err = ngx_create_full_path(ctx->file.name.data, 0755);
    if (err) {
        ngx_log_error(NGX_LOG_ERR, s->log, err,
                ngx_create_dir_n " \"%V\" failed", &ctx->index.name);
        return NGX_ERROR;
    }

    // open file
    ctx->file.fd = ngx_open_file(ctx->file.name.data, NGX_FILE_RDWR,
            NGX_FILE_CREATE_OR_OPEN, NGX_FILE_DEFAULT_ACCESS);
    if (ctx->file.fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_CRIT, s->log, ngx_errno,
                "record: failed to open file '%V'", &ctx->file.name);

        return NGX_ERROR;
    }

    file_size = lseek(ctx->file.fd, 0, SEEK_END);
    if (file_size == (off_t) -1) {
        ngx_log_error(NGX_LOG_CRIT, s->log, ngx_errno,
                "record: %V seek failed", &ctx->file.name);

        return NGX_ERROR;
    }

    if (ctx->ts.wbuf.start == NULL) {
        ctx->ts.wbuf.start = ngx_pcalloc(s->pool, lracf->buffer);
        if (ctx->ts.wbuf.start == NULL) {
            ngx_log_error(NGX_LOG_CRIT, s->log, 0,
                    "record: alloc write buffer error");
            return NGX_ERROR;
        }

        ctx->ts.wbuf.pos = ctx->ts.wbuf.last = ctx->ts.wbuf.start;
        ctx->ts.wbuf.end = ctx->ts.wbuf.start + lracf->buffer;

        ctx->ts.whandle = ngx_live_record_write_buf;
    }
    ctx->ts.fd = ctx->file.fd;
    ctx->ts.log = s->log;
    ctx->ts.file_size = file_size;
    ctx->ts.vcodec = codec_ctx->video_codec_id;
    ctx->ts.acodec = codec_ctx->audio_codec_id;

    if (file_size == 0) { // empty file
        if (ngx_rtmp_mpegts_write_header(&ctx->ts) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                    "record: error writing fragment header");

            return NGX_ERROR;
        }
        ngx_live_record_flush(&ctx->ts);
    }

    ctx->startsize = ctx->ts.file_size;
    ctx->endsize = ctx->ts.file_size;

    return NGX_OK;
}


static void
ngx_live_record_write_index(ngx_rtmp_session_t *s, ngx_live_record_ctx_t *ctx,
        ngx_msec_t curr_time)
{
    u_char                         *p, buf[1024];

    ngx_live_record_flush(&ctx->ts);

    ctx->endsize = ctx->ts.file_size - 1;

    p = ngx_snprintf(buf, sizeof(buf) - 1,
            "%V-%D.ts?startsize=%O&endsize=%O&starttime=%M&endtime=%M\n",
            &s->name, ctx->last_time, ctx->startsize, ctx->endsize,
            ctx->starttime, ctx->endtime);
    *p = 0;

    if (ngx_write_fd(ctx->index.fd, buf, p - buf) < 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                "record, write %V failed: %s", &ctx->index.name, buf);
    }

    ctx->startsize = ctx->ts.file_size;
    ctx->starttime = curr_time;
}


static ngx_int_t
ngx_live_record_open_index(ngx_rtmp_session_t *s)
{
    ngx_live_record_app_conf_t     *lracf;
    ngx_live_record_ctx_t          *ctx;
    ngx_err_t                       err;
    size_t                          len;
    struct tm                       tm;
    u_char                         *p;

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_record_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    len = lracf->path.len + sizeof("/") - 1 + s->serverid.len + sizeof("/") - 1
        + s->app.len + sizeof("/") - 1 + s->name.len + sizeof("/") - 1
        + sizeof("index/YYYYMMDD/") - 1 + s->name.len
        + NGX_OFF_T_LEN + sizeof("-.m3u8") - 1;

    if (ctx->index.name.len == 0) { // first create in current session
        ctx->index.name.data = ngx_pcalloc(s->pool, len + 1);
        if (ctx->index.name.data == NULL) {
            ngx_log_error(NGX_LOG_CRIT, s->log, 0,
                    "record: alloc for index name failed");
            return NGX_ERROR;
        }
    }

    // fill index and file name
    ngx_libc_localtime(ctx->last_time, &tm);

    p = ngx_snprintf(ctx->index.name.data, len,
            "%V/%V/%V/%V/index/%04d%02d%02d/%V-%D.m3u8",
            &lracf->path, &s->serverid, &s->app, &s->name,
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            &s->name, ctx->last_time);
    *p = 0;
    ctx->index.name.len = p - ctx->index.name.data;

    // create dir
    err = ngx_create_full_path(ctx->index.name.data, 0755);
    if (err) {
        ngx_log_error(NGX_LOG_ERR, s->log, err,
                ngx_create_dir_n " \"%V\" failed", &ctx->index.name);
        return NGX_ERROR;
    }

    // open index
    ctx->index.fd = ngx_open_file(ctx->index.name.data, NGX_FILE_RDWR,
            NGX_FILE_CREATE_OR_OPEN, NGX_FILE_DEFAULT_ACCESS);
    if (ctx->index.fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_CRIT, s->log, ngx_errno,
                "record: failed to open index '%V'", &ctx->index.name);

        return NGX_OK;
    }

    if (lseek(ctx->index.fd, 0, SEEK_END) == (off_t) -1) {
        ngx_log_error(NGX_LOG_CRIT, s->log, ngx_errno,
                "record, %V seek failed", &ctx->index.name);

        return NGX_OK;
    }

    return ngx_live_record_open_file(s);
}


static void
ngx_live_record_close_index(ngx_rtmp_session_t *s, ngx_live_record_ctx_t *ctx)
{
    if (ctx->index.fd == -1 || ctx->file.fd == -1) {
        return;
    }

    ngx_live_record_write_index(s, ctx, 0);

    ngx_close_file(ctx->file.fd);
    ctx->file.fd = -1;

    ngx_close_file(ctx->index.fd);
    ctx->index.fd = -1;
}


static void
ngx_live_record_reopen_index(ngx_rtmp_session_t *s, ngx_live_record_ctx_t *ctx,
        ngx_msec_t curr_time, time_t last_time)
{
    // close old index and file
    ngx_live_record_close_index(s, ctx);

    ngx_live_record_update(s);

    ctx->last_time = last_time;

    ctx->begintime = curr_time;
    ctx->starttime = curr_time;
    ctx->endtime = curr_time;

    // open new index and file
    if (ngx_live_record_open_index(s) == NGX_ERROR) {
        ctx->last_time = 0;

        if (ctx->index.fd != -1) {
            ngx_close_file(ctx->index.fd);
        }

        if (ctx->file.fd != -1) {
            ngx_close_file(ctx->file.fd);
        }

        return;
    }
}


static ngx_int_t
ngx_live_record_copy(ngx_rtmp_session_t *s, void *dst, u_char **src, size_t n,
    ngx_chain_t **in)
{
    u_char  *last;
    size_t   pn;

    if (*in == NULL) {
        return NGX_ERROR;
    }

    for ( ;; ) {
        last = (*in)->buf->last;

        if ((size_t)(last - *src) >= n) {
            if (dst) {
                ngx_memcpy(dst, *src, n);
            }

            *src += n;

            while (*in && *src == (*in)->buf->last) {
                *in = (*in)->next;
                if (*in) {
                    *src = (*in)->buf->pos;
                }
            }

            return NGX_OK;
        }

        pn = last - *src;

        if (dst) {
            ngx_memcpy(dst, *src, pn);
            dst = (u_char *)dst + pn;
        }

        n -= pn;
        *in = (*in)->next;

        if (*in == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "hls: failed to read %uz byte(s)", n);
            return NGX_ERROR;
        }

        *src = (*in)->buf->pos;
    }
}


static ngx_int_t
ngx_live_record_parse_aac_header(ngx_rtmp_session_t *s, ngx_uint_t *objtype,
    ngx_uint_t *srindex, ngx_uint_t *chconf)
{
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    ngx_chain_t                    *cl;
    u_char                         *p, b0, b1;

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    cl = codec_ctx->aac_header->chain;

    p = cl->buf->pos;

    if (ngx_live_record_copy(s, NULL, &p, 2, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_live_record_copy(s, &b0, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_live_record_copy(s, &b1, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    *objtype = b0 >> 3;
    if (*objtype == 0 || *objtype == 0x1f) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "record: unsupported adts object type:%ui", *objtype);
        return NGX_ERROR;
    }

    if (*objtype > 4) {

        /*
         * Mark all extended profiles as LC
         * to make Android as happy as possible.
         */

        *objtype = 2;
    }

    *srindex = ((b0 << 1) & 0x0f) | ((b1 & 0x80) >> 7);
    if (*srindex == 0x0f) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "record: unsupported adts sample rate:%ui", *srindex);
        return NGX_ERROR;
    }

    *chconf = (b1 >> 3) & 0x0f;

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "record: aac object_type:%ui, sample_rate_index:%ui, "
                   "channel_config:%ui", *objtype, *srindex, *chconf);

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_append_aud(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    static u_char   aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xf0 };

    if (out->last + sizeof(aud_nal) > out->end) {
        return NGX_ERROR;
    }

    out->last = ngx_cpymem(out->last, aud_nal, sizeof(aud_nal));

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_append_sps_pps(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    u_char                         *p;
    ngx_chain_t                    *in;
    int8_t                          nnals;
    uint16_t                        len, rlen;
    ngx_int_t                       n;

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (codec_ctx == NULL || codec_ctx->avc_header == NULL) {
        return NGX_OK;
    }

    in = codec_ctx->avc_header->chain;
    if (in == NULL) {
        return NGX_ERROR;
    }

    p = in->buf->pos;

    /*
     * Skip bytes:
     * - flv fmt
     * - H264 CONF/PICT (0x00)
     * - 0
     * - 0
     * - 0
     * - version
     * - profile
     * - compatibility
     * - level
     * - nal bytes
     */

    if (ngx_live_record_copy(s, NULL, &p, 10, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* number of SPS NALs */
    if (ngx_live_record_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    nnals &= 0x1f; /* 5lsb */

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "record: SPS number: %uz", nnals);

    /* SPS */
    for (n = 0; ; ++n) {
        for (; nnals; --nnals) {

            /* NAL length */
            if (ngx_live_record_copy(s, &rlen, &p, 2, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            ngx_rtmp_rmemcpy(&len, &rlen, 2);

            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                           "record: header NAL length: %uz", (size_t) len);

            /* AnnexB prefix */
            if (out->end - out->last < 4) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "record: too small buffer for header NAL size");
                return NGX_ERROR;
            }

            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 1;

            /* NAL body */
            if (out->end - out->last < len) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "record: too small buffer for header NAL");
                return NGX_ERROR;
            }

            if (ngx_live_record_copy(s, out->last, &p, len, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            out->last += len;
        }

        if (n == 1) {
            break;
        }

        /* number of PPS NALs */
        if (ngx_live_record_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "record: PPS number: %uz", nnals);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_aac(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_record_ctx_t          *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    uint64_t                        pts;
    ngx_rtmp_mpegts_frame_t         frame;
    ngx_buf_t                       out;
    u_char                         *p;
    ngx_uint_t                      objtype, srindex, chconf, size;
    static u_char                   buffer[NGX_LIVE_RECORD_BUFSIZE];
    ngx_live_record_app_conf_t     *lracf;
    ngx_msec_t                      curr_time;
    time_t                          last_time;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (codec_ctx == NULL || codec_ctx->aac_header == NULL) {
        return NGX_OK;
    }

    if (h->mlen < 2) {
        return NGX_OK;
    }

    if (ctx->open == 2) {
        if (codec_ctx->avc_header == NULL) { // pure audio
            ctx->open = 1;
        } else {
            return NGX_OK;
        }
    }

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_record_module);

    if (ctx->last_time == 0) {
        ctx->publish_epoch = ngx_current_msec;
        ctx->last_time = ngx_time() - ngx_time() % (lracf->interval / 1000);
        ctx->basetime = ctx->publish_epoch - h->timestamp;

        ctx->begintime = ngx_current_msec;
        ctx->starttime = ngx_current_msec;
        ctx->endtime = ngx_current_msec;

        // open new index and file
        if (ngx_live_record_open_index(s) == NGX_ERROR) {
            ctx->last_time = 0;

            if (ctx->index.fd != -1) {
                ngx_close_file(ctx->index.fd);
            }

            if (ctx->file.fd != -1) {
                ngx_close_file(ctx->file.fd);
            }

            return NGX_OK;
        }
    }

    /*
     * FLV Audio data config
     *  SoundFormat 4bits, SoundRate 2bits, SoundSize 1bit, SoundType 1bit
     *  AACPacketType 1byte
     *
     * mpegts ADTS 7 bytes
     */
    size = h->mlen - 2 + 7;
    pts = (uint64_t) h->timestamp * 90;

    p = in->buf->pos;

    /* skip FLV Audio data config */
    if (ngx_live_record_copy(s, NULL, &p, 2, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_memzero(&out, sizeof(out));

    out.start = buffer;
    out.end = buffer + sizeof(buffer);
    out.pos = out.start;
    out.last = out.pos;

    /* make up ADTS */
    if (ngx_live_record_parse_aac_header(s, &objtype, &srindex, &chconf)
            != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "record: aac header error");
        return NGX_OK;
    }

    *out.last++ = 0xff;
    *out.last++ = 0xf1;
    *out.last++ = (u_char) (((objtype - 1) << 6) | (srindex << 2) |
                  ((chconf & 0x04) >> 2));
    *out.last++ = (u_char) (((chconf & 0x03) << 6) | ((size >> 11) & 0x03));
    *out.last++ = (u_char) (size >> 3);
    *out.last++ = (u_char) ((size << 5) | 0x1f);
    *out.last++ = 0xfc;

    /* copy payload */
    while (in) {
        if (in->buf->last - p) {
            out.last = ngx_cpymem(out.last, p, in->buf->last - p);
        }

        in = in->next;
        if (in) {
            p = in->buf->pos;
        }
    }

    // reopen index and ts file
    curr_time = ctx->basetime + h->timestamp;
    if (codec_ctx->avc_header == NULL) { // no video
        last_time = curr_time / 1000 - (curr_time / 1000)
                                     % (lracf->interval / 1000);
        if (curr_time > ctx->starttime + lracf->min_fraglen) {
            if (last_time > ctx->last_time) {
                ngx_live_record_reopen_index(s, ctx, curr_time, last_time);
            } else {
                ngx_live_record_write_index(s, ctx, curr_time);
            }
        }
    }

    /* write frame */
    ngx_memzero(&frame, sizeof(frame));

    frame.cc = ctx->audio_cc;
    frame.dts = pts;
    frame.pts = frame.dts;
    frame.pid = 0x101;
    frame.sid = 0xc0;

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "record: audio pts=%uL, dts=%uL", frame.pts, frame.dts);

    if (ngx_rtmp_mpegts_write_frame(&ctx->ts, &frame, &out) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "record: audio frame failed");
    }

    ctx->endtime = curr_time;
    ctx->audio_cc = frame.cc;

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_avc(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_record_ctx_t          *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    u_char                         *p;
    uint8_t                         fmt, ftype, nal_type, src_nal_type;
    uint32_t                        len, rlen;
    ngx_buf_t                       out;
    uint32_t                        cts;
    ngx_rtmp_mpegts_frame_t         frame;
    ngx_uint_t                      nal_bytes;
    ngx_int_t                       aud_sent, sps_pps_sent;
    static u_char                   buffer[NGX_LIVE_RECORD_BUFSIZE];
    ngx_live_record_app_conf_t     *lracf;
    ngx_msec_t                      curr_time;
    time_t                          last_time;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    p = in->buf->pos;
    if (ngx_live_record_copy(s, &fmt, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /*
     * frame type:
     *  1: keyframe (for AVC, a seekable frame)
     *  2: inter frame (for AVC, a non- seekable frame)
     *  3: disposable inter frame (H.263 only)
     *  4: generated keyframe (reserved for server use only)
     *  5: video info/command frame
     */
    ftype = (fmt & 0xf0) >> 4;

    if (ctx->open == 2) { // wait for key frame
        if (ftype == 1) {
            ctx->open = 1;
        } else {
            return NGX_OK;
        }
    }

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_record_module);

    if (ctx->last_time == 0) {
        ctx->publish_epoch = ngx_current_msec;
        ctx->last_time = ngx_time() - ngx_time() % (lracf->interval / 1000);
        ctx->basetime = ctx->publish_epoch - h->timestamp;

        ctx->begintime = ngx_current_msec;
        ctx->starttime = ngx_current_msec;
        ctx->endtime = ngx_current_msec;

        // open new index and file
        if (ngx_live_record_open_index(s) == NGX_ERROR) {
            ctx->last_time = 0;

            if (ctx->index.fd != -1) {
                ngx_close_file(ctx->index.fd);
            }

            if (ctx->file.fd != -1) {
                ngx_close_file(ctx->file.fd);
            }

            return NGX_OK;
        }
    }

    if (ngx_live_record_copy(s, NULL, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_live_record_copy(s, &cts, &p, 3, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* CompositionTime */
    cts = ((cts & 0x00FF0000) >> 16) | ((cts & 0x000000FF) << 16) |
          (cts & 0x0000FF00);

    /* Data */
    ngx_memzero(&out, sizeof(out));

    out.start = buffer;
    out.end = buffer + sizeof(buffer);
    out.pos = out.start;
    out.last = out.pos;

    nal_bytes = codec_ctx->avc_nal_bytes;
    aud_sent = 0;
    sps_pps_sent = 0;

    while (in) {
        if (ngx_live_record_copy(s, &rlen, &p, nal_bytes, &in) != NGX_OK) {
            return NGX_OK;
        }

        len = 0;
        ngx_rtmp_rmemcpy(&len, &rlen, nal_bytes);

        if (len == 0) {
            continue;
        }

        if (ngx_live_record_copy(s, &src_nal_type, &p, 1, &in) != NGX_OK) {
            return NGX_OK;
        }

        nal_type = src_nal_type & 0x1f;

        ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "record: h264 NAL type=%ui, len=%uD",
                       (ngx_uint_t) nal_type, len);

        if (nal_type >= 7 && nal_type <= 9) {
            if (ngx_live_record_copy(s, NULL, &p, len - 1, &in) != NGX_OK) {
                return NGX_ERROR;
            }
            continue;
        }

        if (!aud_sent) {
            switch (nal_type) {
                case 1:
                case 5:
                case 6:
                    if (ngx_live_record_append_aud(s, &out) != NGX_OK) {
                        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                      "record: error appending AUD NAL");
                    }
                    aud_sent = 1;
                    break;

                case 9:
                    aud_sent = 1;
                    break;
            }
        }

        switch (nal_type) {
            case 1:
                sps_pps_sent = 0;
                break;
            case 5:
                if (sps_pps_sent) {
                    break;
                }
                if (ngx_live_record_append_sps_pps(s, &out) != NGX_OK) {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                  "record: error appenging SPS/PPS NALs");
                }
                sps_pps_sent = 1;
                break;
        }

        /* AnnexB prefix */

        if (out.end - out.last < 5) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "record: not enough buffer for AnnexB prefix");
            return NGX_OK;
        }

        /* first AnnexB prefix is long (4 bytes) */

        if (out.last == out.pos) {
            *out.last++ = 0;
        }

        *out.last++ = 0;
        *out.last++ = 0;
        *out.last++ = 1;
        *out.last++ = src_nal_type;

        /* NAL body */

        if (out.end - out.last < (ngx_int_t) len) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "record: not enough buffer for NAL");
            return NGX_OK;
        }

        if (ngx_live_record_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        out.last += (len - 1);
    }

    // reopen index and ts file
    curr_time = ctx->basetime + h->timestamp;
    last_time = curr_time / 1000 - (curr_time / 1000)
                                 % (lracf->interval / 1000);
    if (ftype == 1) { // key frame
        if (curr_time > ctx->starttime + lracf->min_fraglen) {
            if (last_time > ctx->last_time) {
                ngx_live_record_reopen_index(s, ctx, curr_time, last_time);
            } else {
                ngx_live_record_write_index(s, ctx, curr_time);
            }
        }
    } else if (curr_time > ctx->starttime + lracf->max_fraglen) { // force slice
        if (last_time > ctx->last_time) {
            ngx_log_error(NGX_LOG_INFO, s->log, 0, "record: force slice, "
                    "curr_time:%M, starttime:%M, max_fraglen:%M",
                    curr_time, ctx->starttime, lracf->max_fraglen);

            ngx_live_record_reopen_index(s, ctx, curr_time, last_time);
        } else {
            ngx_live_record_write_index(s, ctx, curr_time);
        }
    }

    /* write frame */
    ngx_memzero(&frame, sizeof(frame));

    frame.cc = ctx->video_cc;
    frame.dts = (uint64_t) h->timestamp * 90;
    frame.pts = (h->timestamp + cts) * 90;
    frame.pid = 0x100;
    frame.sid = 0xe0;
    frame.key = (ftype == 1);

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "record: video pts=%uL, dts=%uL", frame.pts, frame.dts);

    if (ngx_rtmp_mpegts_write_frame(&ctx->ts, &frame, &out) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "record: video frame failed");
    }

    ctx->endtime = curr_time;
    ctx->video_cc = frame.cc;

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_av(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
        ngx_chain_t *in)
{
    ngx_live_record_ctx_t          *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);
    if (ctx == NULL || !ctx->open) {
        return NGX_OK;
    }

    if (ngx_rtmp_is_codec_header(in)) {
        return NGX_OK;
    }

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (h->type == NGX_RTMP_MSG_AUDIO) {
        switch (codec_ctx->audio_codec_id) {
            case NGX_RTMP_AUDIO_AAC:
                return ngx_live_record_aac(s, h, in);
        }
    } else {
        switch (codec_ctx->video_codec_id) {
            case NGX_RTMP_VIDEO_H264:
                return ngx_live_record_avc(s, h, in);
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_live_record_start_handle(ngx_rtmp_session_t *s)
{
    ngx_live_record_ctx_t          *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    return next_publish(s, &ctx->pubv);
}


static ngx_int_t
ngx_live_record_update_handle(ngx_rtmp_session_t *s)
{
    return NGX_OK;
}


static ngx_int_t
ngx_live_record_done_handle(ngx_rtmp_session_t *s)
{
    return NGX_OK;
}


const char *
ngx_live_record_open(ngx_rtmp_session_t *s)
{
    ngx_live_record_ctx_t          *ctx;

    if (s->interprocess) {
        return "interprocess";
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    if (ctx->open) {
        return NGX_CONF_OK;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "record: open %V:", &s->stream);

    ctx->open = 2;

    return NGX_CONF_OK;
}


const char *
ngx_live_record_close(ngx_rtmp_session_t *s)
{
    ngx_live_record_ctx_t          *ctx;

    if (s->interprocess) {
        return "interprocess";
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    if (ctx->open == 0) {
        return NGX_CONF_OK;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "record: close %V:", &s->stream);

    ngx_live_record_done(s);

    ngx_live_record_close_index(s, ctx);

    ctx->open = 0;
    ctx->last_time = 0;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_live_record_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_live_record_app_conf_t     *lracf;
    ngx_live_record_ctx_t          *ctx;

    if (s->interprocess) {
        return next_publish(s, v);
    }

    ctx = ngx_pcalloc(s->pool, sizeof(ngx_live_record_ctx_t));
    if (ctx == NULL) {
        return NGX_ERROR;
    }
    ngx_rtmp_set_ctx(s, ctx, ngx_live_record_module);

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_record_module);

    if (lracf->record) {
        ctx->open = 1;
    }
    ctx->pubv = *v;
    ctx->index.fd = -1;
    ctx->file.fd = -1;

    return ngx_live_record_start(s);
}


static ngx_int_t
ngx_live_record_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_live_record_ctx_t          *ctx;

    if (s->interprocess) {
        goto next;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->open == 0) {
        goto next;
    }

    ngx_live_record_done(s);

    ngx_live_record_close_index(s, ctx);
    ctx->open = 0;

next:
    return next_close_stream(s, v);
}


static ngx_int_t
ngx_live_record_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t          *cmcf;
    ngx_rtmp_handler_pt                *h;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_live_record_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_live_record_av;

    next_record_start = ngx_live_record_start;
    ngx_live_record_start = ngx_live_record_start_handle;

    next_record_update = ngx_live_record_update;
    ngx_live_record_update = ngx_live_record_update_handle;

    next_record_done = ngx_live_record_done;
    ngx_live_record_done = ngx_live_record_done_handle;

    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_live_record_publish;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_live_record_close_stream;

    return NGX_OK;
}
