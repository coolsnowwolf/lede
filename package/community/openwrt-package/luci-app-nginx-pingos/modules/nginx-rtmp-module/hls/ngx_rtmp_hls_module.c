
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include <ngx_rtmp_cmd_module.h>
#include <ngx_rtmp_codec_module.h>
#include "ngx_rtmp_mpegts.h"


static ngx_rtmp_publish_pt              next_publish;
static ngx_rtmp_close_stream_pt         next_close_stream;
static ngx_rtmp_stream_begin_pt         next_stream_begin;
static ngx_rtmp_stream_eof_pt           next_stream_eof;


static char * ngx_rtmp_hls_variant(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static ngx_int_t ngx_rtmp_hls_postconfiguration(ngx_conf_t *cf);
static void * ngx_rtmp_hls_create_app_conf(ngx_conf_t *cf);
static char * ngx_rtmp_hls_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_rtmp_hls_flush_audio(ngx_rtmp_session_t *s);
static ngx_int_t ngx_rtmp_hls_ensure_directory(ngx_rtmp_session_t *s,
       ngx_str_t *path);


#define NGX_RTMP_HLS_BUFSIZE            (1024*1024)
#define NGX_RTMP_HLS_DIR_ACCESS         0744


typedef struct {
    uint64_t                            id;
    uint64_t                            key_id;
    double                              duration;
    unsigned                            active:1;
    unsigned                            discont:1; /* before */
} ngx_rtmp_hls_frag_t;


typedef struct {
    ngx_str_t                           suffix;
    ngx_array_t                         args;
} ngx_rtmp_hls_variant_t;


typedef struct {
    unsigned                            opened:1;

    ngx_rtmp_mpegts_file_t              file;

    ngx_str_t                           playlist;
    ngx_str_t                           playlist_bak;
    ngx_str_t                           var_playlist;
    ngx_str_t                           var_playlist_bak;
    ngx_str_t                           stream;
    ngx_str_t                           keyfile;
    ngx_str_t                           name;
    u_char                              key[16];

    uint64_t                            frag;
    uint64_t                            frag_ts;
    uint64_t                            key_id;
    ngx_uint_t                          nfrags;
    ngx_rtmp_hls_frag_t                *frags; /* circular 2 * winfrags + 1 */

    ngx_uint_t                          audio_cc;
    ngx_uint_t                          video_cc;
    ngx_uint_t                          key_frags;

    uint64_t                            aframe_base;
    uint64_t                            aframe_num;

    ngx_buf_t                          *aframe;
    uint64_t                            aframe_pts;

    ngx_rtmp_hls_variant_t             *var;
} ngx_rtmp_hls_ctx_t;


typedef struct {
    ngx_str_t                           path;
    ngx_msec_t                          playlen;
    ngx_uint_t                          frags_per_key;
} ngx_rtmp_hls_cleanup_t;


typedef struct {
    ngx_flag_t                          hls;
    ngx_msec_t                          fraglen;
    ngx_msec_t                          max_fraglen;
    ngx_msec_t                          muxdelay;
    ngx_msec_t                          sync;
    ngx_msec_t                          playlen;
    ngx_uint_t                          winfrags;
    ngx_flag_t                          continuous;
    ngx_flag_t                          nested;
    ngx_str_t                           path;
    ngx_uint_t                          naming;
    ngx_uint_t                          slicing;
    ngx_uint_t                          type;
    ngx_path_t                         *slot;
    ngx_msec_t                          max_audio_delay;
    size_t                              audio_buffer_size;
    ngx_flag_t                          cleanup;
    ngx_array_t                        *variant;
    ngx_str_t                           base_url;
    ngx_int_t                           granularity;
    ngx_flag_t                          keys;
    ngx_str_t                           key_path;
    ngx_str_t                           key_url;
    ngx_uint_t                          frags_per_key;
} ngx_rtmp_hls_app_conf_t;


#define NGX_RTMP_HLS_NAMING_SEQUENTIAL  1
#define NGX_RTMP_HLS_NAMING_TIMESTAMP   2
#define NGX_RTMP_HLS_NAMING_SYSTEM      3


#define NGX_RTMP_HLS_SLICING_PLAIN      1
#define NGX_RTMP_HLS_SLICING_ALIGNED    2


#define NGX_RTMP_HLS_TYPE_LIVE          1
#define NGX_RTMP_HLS_TYPE_EVENT         2


static ngx_conf_enum_t                  ngx_rtmp_hls_naming_slots[] = {
    { ngx_string("sequential"),         NGX_RTMP_HLS_NAMING_SEQUENTIAL },
    { ngx_string("timestamp"),          NGX_RTMP_HLS_NAMING_TIMESTAMP  },
    { ngx_string("system"),             NGX_RTMP_HLS_NAMING_SYSTEM     },
    { ngx_null_string,                  0 }
};


static ngx_conf_enum_t                  ngx_rtmp_hls_slicing_slots[] = {
    { ngx_string("plain"),              NGX_RTMP_HLS_SLICING_PLAIN },
    { ngx_string("aligned"),            NGX_RTMP_HLS_SLICING_ALIGNED  },
    { ngx_null_string,                  0 }
};


static ngx_conf_enum_t                  ngx_rtmp_hls_type_slots[] = {
    { ngx_string("live"),               NGX_RTMP_HLS_TYPE_LIVE  },
    { ngx_string("event"),              NGX_RTMP_HLS_TYPE_EVENT },
    { ngx_null_string,                  0 }
};


static ngx_command_t ngx_rtmp_hls_commands[] = {

    { ngx_string("hls"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, hls),
      NULL },

    { ngx_string("hls_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, fraglen),
      NULL },

    { ngx_string("hls_max_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, max_fraglen),
      NULL },

    { ngx_string("hls_path"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, path),
      NULL },

    { ngx_string("hls_playlist_length"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, playlen),
      NULL },

    { ngx_string("hls_muxdelay"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, muxdelay),
      NULL },

    { ngx_string("hls_sync"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, sync),
      NULL },

    { ngx_string("hls_continuous"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, continuous),
      NULL },

    { ngx_string("hls_nested"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, nested),
      NULL },

    { ngx_string("hls_fragment_naming"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, naming),
      &ngx_rtmp_hls_naming_slots },

    { ngx_string("hls_fragment_slicing"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, slicing),
      &ngx_rtmp_hls_slicing_slots },

    { ngx_string("hls_type"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, type),
      &ngx_rtmp_hls_type_slots },

    { ngx_string("hls_max_audio_delay"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, max_audio_delay),
      NULL },

    { ngx_string("hls_audio_buffer_size"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, audio_buffer_size),
      NULL },

    { ngx_string("hls_cleanup"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, cleanup),
      NULL },

    { ngx_string("hls_variant"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_hls_variant,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("hls_base_url"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, base_url),
      NULL },

    { ngx_string("hls_fragment_naming_granularity"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, granularity),
      NULL },

    { ngx_string("hls_keys"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, keys),
      NULL },

    { ngx_string("hls_key_path"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, key_path),
      NULL },

    { ngx_string("hls_key_url"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, key_url),
      NULL },

    { ngx_string("hls_fragments_per_key"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_hls_app_conf_t, frags_per_key),
      NULL },

    ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_hls_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_rtmp_hls_postconfiguration,     /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_rtmp_hls_create_app_conf,       /* create location configuration */
    ngx_rtmp_hls_merge_app_conf,        /* merge location configuration */
};


ngx_module_t  ngx_rtmp_hls_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_hls_module_ctx,           /* module context */
    ngx_rtmp_hls_commands,              /* module directives */
    NGX_RTMP_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_rtmp_hls_frag_t *
ngx_rtmp_hls_get_frag(ngx_rtmp_session_t *s, ngx_int_t n)
{
    ngx_rtmp_hls_ctx_t         *ctx;
    ngx_rtmp_hls_app_conf_t    *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    return &ctx->frags[(ctx->frag + n) % (hacf->winfrags * 2 + 1)];
}


static void
ngx_rtmp_hls_next_frag(ngx_rtmp_session_t *s)
{
    ngx_rtmp_hls_ctx_t         *ctx;
    ngx_rtmp_hls_app_conf_t    *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    if (ctx->nfrags == hacf->winfrags) {
        ctx->frag++;
    } else {
        ctx->nfrags++;
    }
}


static ngx_int_t
ngx_rtmp_hls_rename_file(u_char *src, u_char *dst)
{
    /* rename file with overwrite */

#if (NGX_WIN32)
    return MoveFileEx((LPCTSTR) src, (LPCTSTR) dst, MOVEFILE_REPLACE_EXISTING);
#else
    return ngx_rename_file(src, dst);
#endif
}


static ngx_int_t
ngx_rtmp_hls_write_variant_playlist(ngx_rtmp_session_t *s)
{
    static u_char             buffer[1024];

    u_char                   *p, *last;
    ssize_t                   rc;
    ngx_fd_t                  fd;
    ngx_str_t                *arg;
    ngx_uint_t                n, k;
    ngx_rtmp_hls_ctx_t       *ctx;
    ngx_rtmp_hls_variant_t   *var;
    ngx_rtmp_hls_app_conf_t  *hacf;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    fd = ngx_open_file(ctx->var_playlist_bak.data, NGX_FILE_WRONLY,
                       NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_open_file_n " failed: '%V'",
                      &ctx->var_playlist_bak);

        return NGX_ERROR;
    }

#define NGX_RTMP_HLS_VAR_HEADER "#EXTM3U\n#EXT-X-VERSION:3\n"

    rc = ngx_write_fd(fd, NGX_RTMP_HLS_VAR_HEADER,
                      sizeof(NGX_RTMP_HLS_VAR_HEADER) - 1);
    if (rc < 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_write_fd_n " failed: '%V'",
                      &ctx->var_playlist_bak);
        ngx_close_file(fd);
        return NGX_ERROR;
    }

    var = hacf->variant->elts;
    for (n = 0; n < hacf->variant->nelts; n++, var++)
    {
        p = buffer;
        last = buffer + sizeof(buffer);

        p = ngx_slprintf(p, last, "#EXT-X-STREAM-INF:PROGRAM-ID=1");

        arg = var->args.elts;
        for (k = 0; k < var->args.nelts; k++, arg++) {
            p = ngx_slprintf(p, last, ",%V", arg);
        }

        if (p < last) {
            *p++ = '\n';
        }

        p = ngx_slprintf(p, last, "%V%*s%V",
                         &hacf->base_url,
                         ctx->name.len - ctx->var->suffix.len, ctx->name.data,
                         &var->suffix);
        if (hacf->nested) {
            p = ngx_slprintf(p, last, "%s", "/index");
        }

        p = ngx_slprintf(p, last, "%s", ".m3u8\n");

        rc = ngx_write_fd(fd, buffer, p - buffer);
        if (rc < 0) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "hls: " ngx_write_fd_n " failed '%V'",
                          &ctx->var_playlist_bak);
            ngx_close_file(fd);
            return NGX_ERROR;
        }
    }

    ngx_close_file(fd);

    if (ngx_rtmp_hls_rename_file(ctx->var_playlist_bak.data,
                                 ctx->var_playlist.data)
        == NGX_FILE_ERROR)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: rename failed: '%V'->'%V'",
                      &ctx->var_playlist_bak, &ctx->var_playlist);
        return NGX_ERROR;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_write_playlist(ngx_rtmp_session_t *s)
{
    static u_char                   buffer[1024];
    ngx_fd_t                        fd;
    u_char                         *p, *end;
    ngx_rtmp_hls_ctx_t             *ctx;
    ssize_t                         n;
    ngx_rtmp_hls_app_conf_t        *hacf;
    ngx_rtmp_hls_frag_t            *f;
    ngx_uint_t                      i, max_frag;
    ngx_str_t                       name_part, key_name_part;
    uint64_t                        prev_key_id;
    const char                     *sep, *key_sep;


    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    fd = ngx_open_file(ctx->playlist_bak.data, NGX_FILE_WRONLY,
                       NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_open_file_n " failed: '%V'",
                      &ctx->playlist_bak);
        return NGX_ERROR;
    }

    max_frag = hacf->fraglen / 1000;

    for (i = 0; i < ctx->nfrags; i++) {
        f = ngx_rtmp_hls_get_frag(s, i);
        if (f->duration > max_frag) {
            max_frag = (ngx_uint_t) (f->duration + .5);
        }
    }

    p = buffer;
    end = p + sizeof(buffer);

    p = ngx_slprintf(p, end,
                     "#EXTM3U\n"
                     "#EXT-X-VERSION:3\n"
                     "#EXT-X-MEDIA-SEQUENCE:%uL\n"
                     "#EXT-X-TARGETDURATION:%ui\n",
                     ctx->frag, max_frag);

    if (hacf->type == NGX_RTMP_HLS_TYPE_EVENT) {
        p = ngx_slprintf(p, end, "#EXT-X-PLAYLIST-TYPE: EVENT\n");
    }

    n = ngx_write_fd(fd, buffer, p - buffer);
    if (n < 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_write_fd_n " failed: '%V'",
                      &ctx->playlist_bak);
        ngx_close_file(fd);
        return NGX_ERROR;
    }

    sep = hacf->nested ? (hacf->base_url.len ? "/" : "") : "-";
    key_sep = hacf->nested ? (hacf->key_url.len ? "/" : "") : "-";

    name_part.len = 0;
    if (!hacf->nested || hacf->base_url.len) {
        name_part = ctx->name;
    }

    key_name_part.len = 0;
    if (!hacf->nested || hacf->key_url.len) {
        key_name_part = ctx->name;
    }

    prev_key_id = 0;

    for (i = 0; i < ctx->nfrags; i++) {
        f = ngx_rtmp_hls_get_frag(s, i);

        p = buffer;
        end = p + sizeof(buffer);

        if (f->discont) {
            p = ngx_slprintf(p, end, "#EXT-X-DISCONTINUITY\n");
        }

        if (hacf->keys && (i == 0 || f->key_id != prev_key_id)) {
            p = ngx_slprintf(p, end, "#EXT-X-KEY:METHOD=AES-128,"
                             "URI=\"%V%V%s%uL.key\",IV=0x%032XL\n",
                             &hacf->key_url, &key_name_part,
                             key_sep, f->key_id, f->key_id);
        }

        prev_key_id = f->key_id;

        p = ngx_slprintf(p, end,
                         "#EXTINF:%.3f,\n"
                         "%V%V%s%uL.ts\n",
                         f->duration, &hacf->base_url, &name_part, sep, f->id);

        ngx_log_debug5(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: fragment frag=%uL, n=%ui/%ui, duration=%.3f, "
                       "discont=%i",
                       ctx->frag, i + 1, ctx->nfrags, f->duration, f->discont);

        n = ngx_write_fd(fd, buffer, p - buffer);
        if (n < 0) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "hls: " ngx_write_fd_n " failed '%V'",
                          &ctx->playlist_bak);
            ngx_close_file(fd);
            return NGX_ERROR;
        }
    }

    ngx_close_file(fd);

    if (ngx_rtmp_hls_rename_file(ctx->playlist_bak.data, ctx->playlist.data)
        == NGX_FILE_ERROR)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: rename failed: '%V'->'%V'",
                      &ctx->playlist_bak, &ctx->playlist);
        return NGX_ERROR;
    }

    if (ctx->var) {
        return ngx_rtmp_hls_write_variant_playlist(s);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_copy(ngx_rtmp_session_t *s, void *dst, u_char **src, size_t n,
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
ngx_rtmp_hls_append_hevc_aud(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    static u_char   aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x46, 0x01, 0x50 };

    if (out->last + sizeof(aud_nal) > out->end) {
        return NGX_ERROR;
    }

    out->last = ngx_cpymem(out->last, aud_nal, sizeof(aud_nal));

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_append_hevc_vps_sps_pps(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    u_char                         *p;
    ngx_chain_t                    *in;
    ngx_rtmp_hls_ctx_t             *ctx;
    ngx_uint_t                      i, j, rnal_unit_len, nal_unit_len,
                                    num_arrays, nal_unit_type,
                                    rnum_nalus, num_nalus;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (ctx == NULL || codec_ctx == NULL) {
        return NGX_ERROR;
    }

    in = codec_ctx->avc_header->chain;
    if (in == NULL) {
        return NGX_ERROR;
    }

    p = in->buf->pos;
    /*  6 bytes
     * FrameType                                    4 bits
     * CodecID                                      4 bits
     * AVCPacketType                                1 byte
     * CompositionTime                              3 bytes
     * HEVCDecoderConfigurationRecord
     *      configurationVersion                    1 byte
     */

    /*  20 bytes
     * HEVCDecoderConfigurationRecord
     *      general_profile_space                   2 bits
     *      general_tier_flag                       1 bit
     *      general_profile_idc                     5 bits
     *      general_profile_compatibility_flags     4 bytes
     *      general_constraint_indicator_flags      6 bytes
     *      general_level_idc                       1 byte
     *      min_spatial_segmentation_idc            4 bits reserved + 12 bits
     *      parallelismType                         6 bits reserved + 2 bits
     *      chroma_format_idc                       6 bits reserved + 2 bits
     *      bit_depth_luma_minus8                   5 bits reserved + 3 bits
     *      bit_depth_chroma_minus8                 5 bits reserved + 3 bits
     *      avgFrameRate                            2 bytes
     */

    /* 1 bytes
     * HEVCDecoderConfigurationRecord
     *      constantFrameRate                       2 bits
     *      numTemporalLayers                       3 bits
     *      temporalIdNested                        1 bit
     *      lengthSizeMinusOne                      2 bits
     */

    if (ngx_rtmp_hls_copy(s, NULL, &p, 27, &in) != NGX_OK) {
        return NGX_ERROR;
    }

     /* 1 byte
     * HEVCDecoderConfigurationRecord
     *      numOfArrays                             1 byte
     */
    num_arrays = 0;
    if (ngx_rtmp_hls_copy(s, &num_arrays, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    for (i = 0; i < num_arrays; ++i) {
        /*
         * array_completeness                       1 bit
         * reserved                                 1 bit
         * NAL_unit_type                            6 bits
         * numNalus                                 2 bytes
         */
        if (ngx_rtmp_hls_copy(s, &nal_unit_type, &p, 1, &in) != NGX_OK) {    // 32 33 34 39
            return NGX_ERROR;
        }
        nal_unit_type &= 0x3f;
        ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
            "rtmp-hls: append_vps_sps_pps| type %d", nal_unit_type);
        if (ngx_rtmp_hls_copy(s, &rnum_nalus, &p, 2, &in) != NGX_OK) {
            return NGX_ERROR;
        }
        num_nalus = 0;
        ngx_rtmp_rmemcpy(&num_nalus, &rnum_nalus, 2);
        for (j = 0; j < num_nalus; ++j) {
            /*
             * nalUnitLength                        2 bytes
             */
            if (ngx_rtmp_hls_copy(s, &rnal_unit_len, &p, 2, &in) != NGX_OK) {
                return NGX_ERROR;
            }
            nal_unit_len = 0;
            ngx_rtmp_rmemcpy(&nal_unit_len, &rnal_unit_len, 2);
            if (out->end - out->last < 4) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for hevc header NAL size");
                return NGX_ERROR;
            }

            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 1;

            if (out->end - out->last < (ngx_int_t)nal_unit_len) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for hevc header NAL");
                return NGX_ERROR;
            }

            if (ngx_rtmp_hls_copy(s, out->last, &p, nal_unit_len, &in)
                != NGX_OK)
            {
                return NGX_ERROR;
            }

            out->last += nal_unit_len;
        }
    }
    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_append_aud(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    static u_char   aud_nal[] = { 0x00, 0x00, 0x00, 0x01, 0x09, 0xf0 };

    if (out->last + sizeof(aud_nal) > out->end) {
        return NGX_ERROR;
    }

    out->last = ngx_cpymem(out->last, aud_nal, sizeof(aud_nal));

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_append_sps_pps(ngx_rtmp_session_t *s, ngx_buf_t *out)
{
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    u_char                         *p;
    ngx_chain_t                    *in;
    ngx_rtmp_hls_ctx_t             *ctx;
    int8_t                          nnals;
    uint16_t                        len, rlen;
    ngx_int_t                       n;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (ctx == NULL || codec_ctx == NULL) {
        return NGX_ERROR;
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

    if (ngx_rtmp_hls_copy(s, NULL, &p, 10, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* number of SPS NALs */
    if (ngx_rtmp_hls_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    nnals &= 0x1f; /* 5lsb */

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: SPS number: %uz", nnals);

    /* SPS */
    for (n = 0; ; ++n) {
        for (; nnals; --nnals) {

            /* NAL length */
            if (ngx_rtmp_hls_copy(s, &rlen, &p, 2, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            ngx_rtmp_rmemcpy(&len, &rlen, 2);

            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                           "hls: header NAL length: %uz", (size_t) len);

            /* AnnexB prefix */
            if (out->end - out->last < 4) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for header NAL size");
                return NGX_ERROR;
            }

            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 0;
            *out->last++ = 1;

            /* NAL body */
            if (out->end - out->last < len) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: too small buffer for header NAL");
                return NGX_ERROR;
            }

            if (ngx_rtmp_hls_copy(s, out->last, &p, len, &in) != NGX_OK) {
                return NGX_ERROR;
            }

            out->last += len;
        }

        if (n == 1) {
            break;
        }

        /* number of PPS NALs */
        if (ngx_rtmp_hls_copy(s, &nnals, &p, 1, &in) != NGX_OK) {
            return NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: PPS number: %uz", nnals);
    }

    return NGX_OK;
}


static uint64_t
ngx_rtmp_hls_get_fragment_id(ngx_rtmp_session_t *s, uint64_t ts)
{
    ngx_rtmp_hls_ctx_t         *ctx;
    ngx_rtmp_hls_app_conf_t    *hacf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    switch (hacf->naming) {

    case NGX_RTMP_HLS_NAMING_TIMESTAMP:
        return ts;

    case NGX_RTMP_HLS_NAMING_SYSTEM:
        return (uint64_t) ngx_cached_time->sec * 1000 + ngx_cached_time->msec;

    default: /* NGX_RTMP_HLS_NAMING_SEQUENTIAL */
        return ctx->frag + ctx->nfrags;
    }
}


static ngx_int_t
ngx_rtmp_hls_close_fragment(ngx_rtmp_session_t *s)
{
    ngx_rtmp_hls_ctx_t         *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);
    if (ctx == NULL || !ctx->opened) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: close fragment n=%uL", ctx->frag);

    ngx_rtmp_mpegts_close_file(&ctx->file);

    ctx->opened = 0;

    ngx_rtmp_hls_next_frag(s);

    ngx_rtmp_hls_write_playlist(s);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_open_fragment(ngx_rtmp_session_t *s, uint64_t ts,
    ngx_int_t discont)
{
    uint64_t                  id;
    ngx_fd_t                  fd;
    ngx_uint_t                g;
    ngx_rtmp_hls_ctx_t       *ctx;
    ngx_rtmp_hls_frag_t      *f;
    ngx_rtmp_hls_app_conf_t  *hacf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    if (ctx->opened) {
        return NGX_OK;
    }

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    if (ngx_rtmp_hls_ensure_directory(s, &hacf->path) != NGX_OK) {
        return NGX_ERROR;
    }

    if (hacf->keys &&
        ngx_rtmp_hls_ensure_directory(s, &hacf->key_path) != NGX_OK)
    {
        return NGX_ERROR;
    }

    id = ngx_rtmp_hls_get_fragment_id(s, ts);

    if (hacf->granularity) {
        g = (ngx_uint_t) hacf->granularity;
        id = (uint64_t) (id / g) * g;
    }

    ngx_sprintf(ctx->stream.data + ctx->stream.len, "%uL.ts%Z", id);

    if (hacf->keys) {
        if (ctx->key_frags == 0) {

            ctx->key_frags = hacf->frags_per_key - 1;
            ctx->key_id = id;

            if (RAND_bytes(ctx->key, 16) < 0) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                              "hls: failed to create key");
                return NGX_ERROR;
            }

            ngx_sprintf(ctx->keyfile.data + ctx->keyfile.len, "%uL.key%Z", id);

            fd = ngx_open_file(ctx->keyfile.data, NGX_FILE_WRONLY,
                               NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

            if (fd == NGX_INVALID_FILE) {
                ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                              "hls: failed to open key file '%s'",
                              ctx->keyfile.data);
                return NGX_ERROR;
            }

            if (ngx_write_fd(fd, ctx->key, 16) != 16) {
                ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                              "hls: failed to write key file '%s'",
                              ctx->keyfile.data);
                ngx_close_file(fd);
                return NGX_ERROR;
            }

            ngx_close_file(fd);

        } else {
            if (hacf->frags_per_key) {
                ctx->key_frags--;
            }

            if (ngx_set_file_time(ctx->keyfile.data, 0, ngx_cached_time->sec)
                != NGX_OK)
            {
                ngx_log_error(NGX_LOG_ALERT, s->log, ngx_errno,
                              ngx_set_file_time_n " '%s' failed",
                              ctx->keyfile.data);
            }
        }
    }

    ngx_log_debug6(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: open fragment file='%s', keyfile='%s', "
                   "frag=%uL, n=%ui, time=%uL, discont=%i",
                   ctx->stream.data,
                   ctx->keyfile.data ? ctx->keyfile.data : (u_char *) "",
                   ctx->frag, ctx->nfrags, ts, discont);

    if (hacf->keys &&
        ngx_rtmp_mpegts_init_encryption(&ctx->file, ctx->key, 16, ctx->key_id)
        != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: failed to initialize hls encryption");
        return NGX_ERROR;
    }

    ctx->file.acodec = s->acodec;
    ctx->file.vcodec = s->vcodec;
    if (ngx_rtmp_mpegts_open_file(&ctx->file, ctx->stream.data,
                                  s->log)
        != NGX_OK)
    {
        return NGX_ERROR;
    }

    ctx->opened = 1;

    f = ngx_rtmp_hls_get_frag(s, ctx->nfrags);

    ngx_memzero(f, sizeof(*f));

    f->active = 1;
    f->discont = discont;
    f->id = id;
    f->key_id = ctx->key_id;

    ctx->frag_ts = ts;

    /* start fragment with audio to make iPhone happy */

    ngx_rtmp_hls_flush_audio(s);

    return NGX_OK;
}


static void
ngx_rtmp_hls_restore_stream(ngx_rtmp_session_t *s)
{
    ngx_rtmp_hls_ctx_t             *ctx;
    ngx_file_t                      file;
    ssize_t                         ret;
    off_t                           offset;
    u_char                         *p, *last, *end, *next, *pa, *pp, c;
    ngx_rtmp_hls_frag_t            *f;
    double                          duration;
    ngx_int_t                       discont;
    uint64_t                        mag, key_id, base;
    static u_char                   buffer[4096];

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    ngx_memzero(&file, sizeof(file));

    file.log = s->log;

    ngx_str_set(&file.name, "m3u8");

    file.fd = ngx_open_file(ctx->playlist.data, NGX_FILE_RDONLY, NGX_FILE_OPEN,
                            0);
    if (file.fd == NGX_INVALID_FILE) {
        return;
    }

    offset = 0;
    ctx->nfrags = 0;
    f = NULL;
    duration = 0;
    discont = 0;
    key_id = 0;

    for ( ;; ) {

        ret = ngx_read_file(&file, buffer, sizeof(buffer), offset);
        if (ret <= 0) {
            goto done;
        }

        p = buffer;
        end = buffer + ret;

        for ( ;; ) {
            last = ngx_strlchr(p, end, '\n');

            if (last == NULL) {
                if (p == buffer) {
                    goto done;
                }
                break;
            }

            next = last + 1;
            offset += (next - p);

            if (p != last && last[-1] == '\r') {
                last--;
            }


#define NGX_RTMP_MSEQ           "#EXT-X-MEDIA-SEQUENCE:"
#define NGX_RTMP_MSEQ_LEN       (sizeof(NGX_RTMP_MSEQ) - 1)


            if (ngx_memcmp(p, NGX_RTMP_MSEQ, NGX_RTMP_MSEQ_LEN) == 0) {

                ctx->frag = (uint64_t) strtod((const char *)
                                              &p[NGX_RTMP_MSEQ_LEN], NULL);

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "hls: restore sequence frag=%uL", ctx->frag);
            }


#define NGX_RTMP_XKEY           "#EXT-X-KEY:"
#define NGX_RTMP_XKEY_LEN       (sizeof(NGX_RTMP_XKEY) - 1)

            if (ngx_memcmp(p, NGX_RTMP_XKEY, NGX_RTMP_XKEY_LEN) == 0) {

                /* recover key id from initialization vector */

                key_id = 0;
                base = 1;
                pp = last - 1;

                for ( ;; ) {
                    if (pp < p) {
                        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                "hls: failed to read key id");
                        break;
                    }

                    c = *pp;
                    if (c == 'x') {
                        break;
                    }

                    if (c >= '0' && c <= '9') {
                        c -= '0';
                        goto next;
                    }

                    c |= 0x20;

                    if (c >= 'a' && c <= 'f') {
                        c -= 'a' - 10;
                        goto next;
                    }

                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                  "hls: bad character in key id");
                    break;

                next:

                    key_id += base * c;
                    base *= 0x10;
                    pp--;
                }
            }


#define NGX_RTMP_EXTINF         "#EXTINF:"
#define NGX_RTMP_EXTINF_LEN     (sizeof(NGX_RTMP_EXTINF) - 1)


            if (ngx_memcmp(p, NGX_RTMP_EXTINF, NGX_RTMP_EXTINF_LEN) == 0) {

                duration = strtod((const char *) &p[NGX_RTMP_EXTINF_LEN], NULL);

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "hls: restore durarion=%.3f", duration);
            }


#define NGX_RTMP_DISCONT        "#EXT-X-DISCONTINUITY"
#define NGX_RTMP_DISCONT_LEN    (sizeof(NGX_RTMP_DISCONT) - 1)


            if (ngx_memcmp(p, NGX_RTMP_DISCONT, NGX_RTMP_DISCONT_LEN) == 0) {

                discont = 1;

                ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "hls: discontinuity");
            }

            /* find '.ts\r' */

            if (p + 4 <= last &&
                last[-3] == '.' && last[-2] == 't' && last[-1] == 's')
            {
                f = ngx_rtmp_hls_get_frag(s, ctx->nfrags);

                ngx_memzero(f, sizeof(*f));

                f->duration = duration;
                f->discont = discont;
                f->active = 1;
                f->id = 0;

                discont = 0;

                mag = 1;
                for (pa = last - 4; pa >= p; pa--) {
                    if (*pa < '0' || *pa > '9') {
                        break;
                    }
                    f->id += (*pa - '0') * mag;
                    mag *= 10;
                }

                f->key_id = key_id;

                ngx_rtmp_hls_next_frag(s);

                ngx_log_debug6(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "hls: restore fragment '%*s' id=%uL, "
                               "duration=%.3f, frag=%uL, nfrags=%ui",
                               (size_t) (last - p), p, f->id, f->duration,
                               ctx->frag, ctx->nfrags);
            }

            p = next;
        }
    }

done:
    ngx_close_file(file.fd);
}


static ngx_int_t
ngx_rtmp_hls_ensure_directory(ngx_rtmp_session_t *s, ngx_str_t *path)
{
    size_t                    len;
    ngx_file_info_t           fi;
    ngx_rtmp_hls_ctx_t       *ctx;
    ngx_rtmp_hls_app_conf_t  *hacf;

    static u_char  zpath[NGX_MAX_PATH + 1];

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    if (path->len + 1 > sizeof(zpath)) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "hls: too long path");
        return NGX_ERROR;
    }

    ngx_snprintf(zpath, sizeof(zpath), "%V%Z", path);

    if (ngx_file_info(zpath, &fi) == NGX_FILE_ERROR) {

        if (ngx_errno != NGX_ENOENT) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "hls: " ngx_file_info_n " failed on '%V'", path);
            return NGX_ERROR;
        }

        /* ENOENT */

        if (ngx_create_dir(zpath, NGX_RTMP_HLS_DIR_ACCESS) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "hls: " ngx_create_dir_n " failed on '%V'", path);
            return NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: directory '%V' created", path);

    } else {

        if (!ngx_is_dir(&fi)) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "hls: '%V' exists and is not a directory", path);
            return  NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: directory '%V' exists", path);
    }

    if (!hacf->nested) {
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    len = path->len;
    if (path->data[len - 1] == '/') {
        len--;
    }

    if (len + 1 + ctx->name.len + 1 > sizeof(zpath)) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "hls: too long path");
        return NGX_ERROR;
    }

    ngx_snprintf(zpath, sizeof(zpath) - 1, "%*s/%V%Z", len, path->data,
                 &ctx->name);

    if (ngx_file_info(zpath, &fi) != NGX_FILE_ERROR) {

        if (ngx_is_dir(&fi)) {
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                           "hls: directory '%s' exists", zpath);
            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: '%s' exists and is not a directory", zpath);

        return  NGX_ERROR;
    }

    if (ngx_errno != NGX_ENOENT) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_file_info_n " failed on '%s'", zpath);
        return NGX_ERROR;
    }

    /* NGX_ENOENT */

    if (ngx_create_dir(zpath, NGX_RTMP_HLS_DIR_ACCESS) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "hls: " ngx_create_dir_n " failed on '%s'", zpath);
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: directory '%s' created", zpath);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_rtmp_hls_app_conf_t        *hacf;
    ngx_rtmp_hls_ctx_t             *ctx;
    u_char                         *p, *pp;
    ngx_rtmp_hls_frag_t            *f;
    ngx_buf_t                      *b;
    size_t                          len;
    ngx_rtmp_hls_variant_t         *var;
    ngx_uint_t                      n;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    if (hacf == NULL || !hacf->hls || hacf->path.len == 0) {
        goto next;
    }

    if (s->interprocess) {
        goto next;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: publish: name='%s' type='%s'",
                   v->name, v->type);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    if (ctx == NULL) {

        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_hls_ctx_t));
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_hls_module);

    } else {

        f = ctx->frags;
        b = ctx->aframe;

        ngx_memzero(ctx, sizeof(ngx_rtmp_hls_ctx_t));

        ctx->frags = f;
        ctx->aframe = b;

        if (b) {
            b->pos = b->last = b->start;
        }
    }

    if (ctx->frags == NULL) {
        ctx->frags = ngx_pcalloc(s->pool,
                                 sizeof(ngx_rtmp_hls_frag_t) *
                                 (hacf->winfrags * 2 + 1));
        if (ctx->frags == NULL) {
            return NGX_ERROR;
        }
    }

    if (ngx_strstr(v->name, "..")) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: bad stream name: '%s'", v->name);
        return NGX_ERROR;
    }

    ctx->name.len = ngx_strlen(v->name);
    ctx->name.data = ngx_palloc(s->pool, ctx->name.len + 1);

    if (ctx->name.data == NULL) {
        return NGX_ERROR;
    }

    *ngx_cpymem(ctx->name.data, v->name, ctx->name.len) = 0;

    len = hacf->path.len + 1 + ctx->name.len + sizeof(".m3u8");
    if (hacf->nested) {
        len += sizeof("/index") - 1;
    }

    ctx->playlist.data = ngx_palloc(s->pool, len);
    p = ngx_cpymem(ctx->playlist.data, hacf->path.data, hacf->path.len);

    if (p[-1] != '/') {
        *p++ = '/';
    }

    p = ngx_cpymem(p, ctx->name.data, ctx->name.len);

    /*
     * ctx->stream holds initial part of stream file path
     * however the space for the whole stream path
     * is allocated
     */

    ctx->stream.len = p - ctx->playlist.data + 1;
    ctx->stream.data = ngx_palloc(s->pool,
                                  ctx->stream.len + NGX_INT64_LEN +
                                  sizeof(".ts"));

    ngx_memcpy(ctx->stream.data, ctx->playlist.data, ctx->stream.len - 1);
    ctx->stream.data[ctx->stream.len - 1] = (hacf->nested ? '/' : '-');

    /* varint playlist path */

    if (hacf->variant) {
        var = hacf->variant->elts;
        for (n = 0; n < hacf->variant->nelts; n++, var++) {
            if (ctx->name.len > var->suffix.len &&
                ngx_memcmp(var->suffix.data,
                           ctx->name.data + ctx->name.len - var->suffix.len,
                           var->suffix.len)
                == 0)
            {
                ctx->var = var;

                len = (size_t) (p - ctx->playlist.data);

                ctx->var_playlist.len = len - var->suffix.len + sizeof(".m3u8")
                                        - 1;
                ctx->var_playlist.data = ngx_palloc(s->pool,
                                                    ctx->var_playlist.len + 1);

                pp = ngx_cpymem(ctx->var_playlist.data, ctx->playlist.data,
                               len - var->suffix.len);
                pp = ngx_cpymem(pp, ".m3u8", sizeof(".m3u8") - 1);
                *pp = 0;

                ctx->var_playlist_bak.len = ctx->var_playlist.len +
                                            sizeof(".bak") - 1;
                ctx->var_playlist_bak.data = ngx_palloc(s->pool,
                                                 ctx->var_playlist_bak.len + 1);

                pp = ngx_cpymem(ctx->var_playlist_bak.data,
                                ctx->var_playlist.data,
                                ctx->var_playlist.len);
                pp = ngx_cpymem(pp, ".bak", sizeof(".bak") - 1);
                *pp = 0;

                break;
            }
        }
    }


    /* playlist path */

    if (hacf->nested) {
        p = ngx_cpymem(p, "/index.m3u8", sizeof("/index.m3u8") - 1);
    } else {
        p = ngx_cpymem(p, ".m3u8", sizeof(".m3u8") - 1);
    }

    ctx->playlist.len = p - ctx->playlist.data;

    *p = 0;

    /* playlist bak (new playlist) path */

    ctx->playlist_bak.data = ngx_palloc(s->pool,
                                        ctx->playlist.len + sizeof(".bak"));
    p = ngx_cpymem(ctx->playlist_bak.data, ctx->playlist.data,
                   ctx->playlist.len);
    p = ngx_cpymem(p, ".bak", sizeof(".bak") - 1);

    ctx->playlist_bak.len = p - ctx->playlist_bak.data;

    *p = 0;

    /* key path */

    if (hacf->keys) {
        len = hacf->key_path.len + 1 + ctx->name.len + 1 + NGX_INT64_LEN
              + sizeof(".key");

        ctx->keyfile.data = ngx_palloc(s->pool, len);
        if (ctx->keyfile.data == NULL) {
            return NGX_ERROR;
        }

        p = ngx_cpymem(ctx->keyfile.data, hacf->key_path.data,
                       hacf->key_path.len);

        if (p[-1] != '/') {
            *p++ = '/';
        }

        p = ngx_cpymem(p, ctx->name.data, ctx->name.len);
        *p++ = (hacf->nested ? '/' : '-');

        ctx->keyfile.len = p - ctx->keyfile.data;
    }

    ngx_log_debug4(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: playlist='%V' playlist_bak='%V' "
                   "stream_pattern='%V' keyfile_pattern='%V'",
                   &ctx->playlist, &ctx->playlist_bak,
                   &ctx->stream, &ctx->keyfile);

    if (hacf->continuous) {
        ngx_rtmp_hls_restore_stream(s);
    }

next:
    return next_publish(s, v);
}


static ngx_int_t
ngx_rtmp_hls_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_rtmp_hls_app_conf_t        *hacf;
    ngx_rtmp_hls_ctx_t             *ctx;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    if (hacf == NULL || !hacf->hls || ctx == NULL) {
        goto next;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: close stream");

    ngx_rtmp_hls_close_fragment(s);

next:
    return next_close_stream(s, v);
}


static ngx_int_t
ngx_rtmp_hls_parse_aac_header(ngx_rtmp_session_t *s, ngx_uint_t *objtype,
    ngx_uint_t *srindex, ngx_uint_t *chconf)
{
    ngx_rtmp_codec_ctx_t   *codec_ctx;
    ngx_chain_t            *cl;
    u_char                 *p, b0, b1;

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    cl = codec_ctx->aac_header->chain;

    p = cl->buf->pos;

    if (ngx_rtmp_hls_copy(s, NULL, &p, 2, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_rtmp_hls_copy(s, &b0, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_rtmp_hls_copy(s, &b1, &p, 1, &cl) != NGX_OK) {
        return NGX_ERROR;
    }

    *objtype = b0 >> 3;
    if (*objtype == 0 || *objtype == 0x1f) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: unsupported adts object type:%ui", *objtype);
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
                       "hls: unsupported adts sample rate:%ui", *srindex);
        return NGX_ERROR;
    }

    *chconf = (b1 >> 3) & 0x0f;

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: aac object_type:%ui, sample_rate_index:%ui, "
                   "channel_config:%ui", *objtype, *srindex, *chconf);

    return NGX_OK;
}


static void
ngx_rtmp_hls_update_fragment(ngx_rtmp_session_t *s, uint64_t ts,
    ngx_int_t boundary, ngx_uint_t flush_rate)
{
    ngx_rtmp_hls_ctx_t         *ctx;
    ngx_rtmp_hls_app_conf_t    *hacf;
    ngx_rtmp_hls_frag_t        *f;
    ngx_msec_t                  ts_frag_len;
    ngx_int_t                   same_frag, force,discont;
    ngx_buf_t                  *b;
    int64_t                     d;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);
    f = NULL;
    force = 0;
    discont = 1;

    if (ctx->opened) {
        f = ngx_rtmp_hls_get_frag(s, ctx->nfrags);
        d = (int64_t) (ts - ctx->frag_ts);

        if (d > (int64_t) hacf->max_fraglen * 90 || d < -90000) {
            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                          "hls: force fragment split: %.3f sec, ", d / 90000.);
            force = 1;

        } else {
            f->duration = (ts - ctx->frag_ts) / 90000.;
            discont = 0;
        }
    }

    switch (hacf->slicing) {
        case NGX_RTMP_HLS_SLICING_PLAIN:
            if (f && f->duration < hacf->fraglen / 1000.) {
                boundary = 0;
            }
            break;

        case NGX_RTMP_HLS_SLICING_ALIGNED:

            ts_frag_len = hacf->fraglen * 90;
            same_frag = ctx->frag_ts / ts_frag_len == ts / ts_frag_len;

            if (f && same_frag) {
                boundary = 0;
            }

            if (f == NULL && (ctx->frag_ts == 0 || same_frag)) {
                ctx->frag_ts = ts;
                boundary = 0;
            }

            break;
    }

    if (boundary || force) {
        ngx_rtmp_hls_close_fragment(s);
        ngx_rtmp_hls_open_fragment(s, ts, discont);
    }

    b = ctx->aframe;
    if (ctx->opened && b && b->last > b->pos &&
        ctx->aframe_pts + (uint64_t) hacf->max_audio_delay * 90 / flush_rate
        < ts)
    {
        ngx_rtmp_hls_flush_audio(s);
    }
}


static ngx_int_t
ngx_rtmp_hls_flush_audio(ngx_rtmp_session_t *s)
{
    ngx_rtmp_hls_ctx_t             *ctx;
    ngx_rtmp_mpegts_frame_t         frame;
    ngx_int_t                       rc;
    ngx_buf_t                      *b;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    if (ctx == NULL || !ctx->opened) {
        return NGX_OK;
    }

    b = ctx->aframe;

    if (b == NULL || b->pos == b->last) {
        return NGX_OK;
    }

    ngx_memzero(&frame, sizeof(frame));

    frame.dts = ctx->aframe_pts;
    frame.pts = frame.dts;
    frame.cc = ctx->audio_cc;
    frame.pid = 0x101;
    frame.sid = 0xc0;

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: flush audio pts=%uL", frame.pts);

    rc = ngx_rtmp_mpegts_write_frame(&ctx->file, &frame, b);

    if (rc != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: audio flush failed");
    }

    ctx->audio_cc = frame.cc;
    b->pos = b->last = b->start;

    return rc;
}


static ngx_int_t
ngx_rtmp_hls_audio(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    ngx_rtmp_hls_app_conf_t        *hacf;
    ngx_rtmp_hls_ctx_t             *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    uint64_t                        pts, est_pts;
    int64_t                         dpts;
    size_t                          bsize;
    ngx_buf_t                      *b;
    u_char                         *p = NULL;
    ngx_uint_t                      objtype, srindex, chconf, size = 0;
    ngx_flag_t                      mp3_tag = 0;

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (hacf == NULL || !hacf->hls || ctx == NULL ||
        codec_ctx == NULL  || h->mlen < 2)
    {
        return NGX_OK;
    }

    if ((codec_ctx->audio_codec_id != NGX_RTMP_AUDIO_AAC ||
        codec_ctx->aac_header == NULL || ngx_rtmp_is_codec_header(in)) &&
        codec_ctx->audio_codec_id != NGX_RTMP_AUDIO_MP3)
    {
        return NGX_OK;
    }

    if (s->pause) {
        return NGX_OK;
    }

    b = ctx->aframe;

    if (b == NULL) {

        b = ngx_pcalloc(s->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return NGX_ERROR;
        }

        ctx->aframe = b;

        b->start = ngx_palloc(s->pool, hacf->audio_buffer_size);
        if (b->start == NULL) {
            return NGX_ERROR;
        }

        b->end = b->start + hacf->audio_buffer_size;
        b->pos = b->last = b->start;
    }

    if (codec_ctx->audio_codec_id == NGX_RTMP_AUDIO_AAC) {
        size = h->mlen - 2 + 7;
    } else if (codec_ctx->audio_codec_id == NGX_RTMP_AUDIO_MP3) {
        size = h->mlen - 1;
    }

    pts = (uint64_t) h->timestamp * 90;

    if (b->start + size > b->end) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: too big audio frame");
        return NGX_OK;
    }

    /*
     * start new fragment here if
     * there's no video at all, otherwise
     * do it in video handler
     */

    ngx_rtmp_hls_update_fragment(s, pts, codec_ctx->avc_header == NULL, 2);

    if (b->last + size > b->end) {
        ngx_rtmp_hls_flush_audio(s);
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: audio pts=%uL", pts);

    if (codec_ctx->audio_codec_id == NGX_RTMP_AUDIO_AAC) {
        if (b->last + 7 > b->end) {
            ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                        "hls: not enough buffer for audio header");
            return NGX_OK;
        }

        p = b->last;
        b->last += 5;

        /* copy payload */

        for (; in && b->last < b->end; in = in->next) {

            bsize = in->buf->last - in->buf->pos;
            if (b->last + bsize > b->end) {
                bsize = b->end - b->last;
            }

            b->last = ngx_cpymem(b->last, in->buf->pos, bsize);
        }

        /* make up ADTS header */

        if (ngx_rtmp_hls_parse_aac_header(s, &objtype, &srindex, &chconf)
            != NGX_OK)
        {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "hls: aac header error");
            return NGX_OK;
        }

        /* we have 5 free bytes + 2 bytes of RTMP frame header */

        p[0] = 0xff;
        p[1] = 0xf1;
        p[2] = (u_char) (((objtype - 1) << 6) | (srindex << 2) |
                        ((chconf & 0x04) >> 2));
        p[3] = (u_char) (((chconf & 0x03) << 6) | ((size >> 11) & 0x03));
        p[4] = (u_char) (size >> 3);
        p[5] = (u_char) ((size << 5) | 0x1f);
        p[6] = 0xfc;
    } else if (codec_ctx->audio_codec_id == NGX_RTMP_AUDIO_MP3) {
		if (b->last > b->end) {
            ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                            "hls: not enough buffer for audio header");
            return NGX_OK;
        }

        p = b->last;
            
        /* copy payload */

        for (; in && b->last < b->end; in = in->next) {

            bsize = in->buf->last - in->buf->pos;
            if (b->last + bsize > b->end) {
                bsize = b->end - b->last;
            }

            if (0 == mp3_tag) {
                b->last = ngx_cpymem(b->last, in->buf->pos+1, bsize-1);
                mp3_tag = 1;
            } else {
                b->last = ngx_cpymem(b->last, in->buf->pos, bsize);
            }
        }
    }

    if (p != b->start) {
        ctx->aframe_num++;
        return NGX_OK;
    }

    ctx->aframe_pts = pts;

    if (!hacf->sync || codec_ctx->sample_rate == 0) {
        return NGX_OK;
    }

    /* align audio frames */

    /* TODO: We assume here AAC frame size is 1024
     *       Need to handle AAC frames with frame size of 960 */

    est_pts = ctx->aframe_base + ctx->aframe_num * 90000 * 1024 /
                                 codec_ctx->sample_rate;
    dpts = (int64_t) (est_pts - pts);

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: audio sync dpts=%L (%.5fs)",
                   dpts, dpts / 90000.);

    if (dpts <= (int64_t) hacf->sync * 90 &&
        dpts >= (int64_t) hacf->sync * -90)
    {
        ctx->aframe_num++;
        ctx->aframe_pts = est_pts;
        return NGX_OK;
    }

    ctx->aframe_base = pts;
    ctx->aframe_num  = 1;

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: audio sync gap dpts=%L (%.5fs)",
                   dpts, dpts / 90000.);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_video(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    ngx_rtmp_hls_app_conf_t        *hacf;
    ngx_rtmp_hls_ctx_t             *ctx;
    ngx_rtmp_codec_ctx_t           *codec_ctx;
    u_char                         *p;
    uint8_t                         fmt, ftype, htype, nal_type, src_nal_type;
    uint32_t                        len, rlen;
    ngx_buf_t                       out, *b;
    uint32_t                        cts;
    ngx_rtmp_mpegts_frame_t         frame;
    ngx_uint_t                      nal_bytes;
    ngx_int_t                       aud_sent, sps_pps_sent, boundary;
    static u_char                   buffer[NGX_RTMP_HLS_BUFSIZE];
    ngx_rtmp_core_app_conf_t       *cacf;

    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);

    hacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_hls_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_hls_module);

    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (hacf == NULL || !hacf->hls || ctx == NULL || codec_ctx == NULL ||
        codec_ctx->avc_header == NULL || h->mlen < 1)
    {
        return NGX_OK;
    }

    /* Only H264 is supported */
    if (codec_ctx->video_codec_id != NGX_RTMP_VIDEO_H264 &&
        codec_ctx->video_codec_id != cacf->hevc_codec)
    {
        return NGX_OK;
    }

    if (s->pause) {
        return NGX_OK;
    }

    p = in->buf->pos;
    if (ngx_rtmp_hls_copy(s, &fmt, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* 1: keyframe (IDR)
     * 2: inter frame
     * 3: disposable inter frame */

    ftype = (fmt & 0xf0) >> 4;

    /* H264 HDR/PICT */

    if (ngx_rtmp_hls_copy(s, &htype, &p, 1, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    /* proceed only with PICT */

    if (htype != 1) {
        return NGX_OK;
    }

    /* 3 bytes: decoder delay */

    if (ngx_rtmp_hls_copy(s, &cts, &p, 3, &in) != NGX_OK) {
        return NGX_ERROR;
    }

    cts = ((cts & 0x00FF0000) >> 16) | ((cts & 0x000000FF) << 16) |
          (cts & 0x0000FF00);

    ngx_memzero(&out, sizeof(out));

    out.start = buffer;
    out.end = buffer + sizeof(buffer);
    out.pos = out.start;
    out.last = out.pos;

    nal_bytes = codec_ctx->avc_nal_bytes;
    aud_sent = 0;
    sps_pps_sent = 0;

    while (in) {
        if (ngx_rtmp_hls_copy(s, &rlen, &p, nal_bytes, &in) != NGX_OK) {
            return NGX_OK;
        }

        len = 0;
        ngx_rtmp_rmemcpy(&len, &rlen, nal_bytes);

        if (len == 0) {
            continue;
        }

        if (ngx_rtmp_hls_copy(s, &src_nal_type, &p, 1, &in) != NGX_OK) {
            return NGX_OK;
        }

        if (codec_ctx->video_codec_id == NGX_RTMP_VIDEO_H264) {
            nal_type = src_nal_type & 0x1f;

            ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "hls: h264 NAL type=%ui, len=%uD",
                       (ngx_uint_t) nal_type, len);

            if (nal_type >= 7 && nal_type <= 9) {
                if (ngx_rtmp_hls_copy(s, NULL, &p, len - 1, &in) != NGX_OK) {
                    return NGX_ERROR;
                }
                continue;
            }

            if (!aud_sent) {
                switch (nal_type) {
                    case 1:
                    case 5:
                    case 6:
                        if (ngx_rtmp_hls_append_aud(s, &out) != NGX_OK) {
                            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                        "hls: error appending AUD NAL");
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
                    if (ngx_rtmp_hls_append_sps_pps(s, &out) != NGX_OK) {
                        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                    "hls: error appenging SPS/PPS NALs");
                    }
                    sps_pps_sent = 1;
                    break;
            }
        } else {
            if (src_nal_type == 0) {
                if (ngx_rtmp_hls_copy(s, out.last, &p, 3, &in) != NGX_OK) {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                    "rtmp-hls: video| buffer error");
                    return NGX_ERROR;
                }
                if (out.last[0] == 0x00 && out.last[1] == 0x00 &&
                    out.last[2] == 0x01)
                {
                    ngx_rtmp_hls_copy(s, &src_nal_type, &p, 1, &in);
                    len -= 4;
                } else {
                    p -= 3;
                }
            }

            nal_type = (src_nal_type >> 1) & 0x3f;

            ngx_log_error(NGX_LOG_DEBUG, s->log, 0,
                        "hls: h265 NAL type=%ui, len=%uD",
                        (ngx_uint_t) nal_type, len);

            if (nal_type == 32      // VPS
             || nal_type == 33      // SPS
             || nal_type == 34      // PPS
             || nal_type == 35      // AUD
             )
            {
                if (ngx_rtmp_hls_copy(s, NULL, &p, len - 1, &in) != NGX_OK) {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "rtmp-hls: video| jump nal, failed copy ");
                    return NGX_ERROR;
                }
                continue;
            }

            if (!aud_sent) {
                if (ngx_rtmp_hls_append_hevc_aud(s, &out) != NGX_OK) {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                "hls: error appending AUD NAL");
                }
                aud_sent = 1;
            }

            if (!IS_IRAP(nal_type)) {
                sps_pps_sent = 0;
            } else if (!sps_pps_sent) {
                if (ngx_rtmp_hls_append_hevc_vps_sps_pps(s, &out) != NGX_OK)
                {
                    ngx_log_error(NGX_LOG_ERR, s->log, 0,
                                "hls: error appenging VPS/SPS/PPS NALs");
                }
                sps_pps_sent = 1;
            }
        }

        /* AnnexB prefix */

        if (out.end - out.last < 5) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "hls: not enough buffer for AnnexB prefix");
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
                          "hls: not enough buffer for NAL");
            return NGX_OK;
        }

        if (ngx_rtmp_hls_copy(s, out.last, &p, len - 1, &in) != NGX_OK) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "rtmp-hls: video| copy nal, failed copy, len %d, in %d",
                        len - 1, in->buf->last - in->buf->pos);
            return NGX_ERROR;
        }

        out.last += (len - 1);
    }

    ngx_memzero(&frame, sizeof(frame));

    frame.cc = ctx->video_cc;
    frame.dts = (uint64_t) h->timestamp * 90;
    frame.pts = frame.dts + cts * 90;
    frame.pid = 0x100;
    frame.sid = 0xe0;
    frame.key = (ftype == 1);

    /*
     * start new fragment if
     * - we have video key frame AND
     * - we have audio buffered or have no audio at all or stream is closed
     */

    b = ctx->aframe;
    boundary = frame.key && (codec_ctx->aac_header == NULL || !ctx->opened ||
                             (b && b->last > b->pos));

    ngx_rtmp_hls_update_fragment(s, frame.dts, boundary, 1);

    if (!ctx->opened) {
        return NGX_OK;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "hls: video pts=%uL, dts=%uL", frame.pts, frame.dts);

    if (ngx_rtmp_mpegts_write_frame(&ctx->file, &frame, &out) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "hls: video frame failed");
    }

    ctx->video_cc = frame.cc;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_hls_stream_begin(ngx_rtmp_session_t *s, ngx_rtmp_stream_begin_t *v)
{
    return next_stream_begin(s, v);
}


static ngx_int_t
ngx_rtmp_hls_stream_eof(ngx_rtmp_session_t *s, ngx_rtmp_stream_eof_t *v)
{
    ngx_rtmp_hls_flush_audio(s);

    ngx_rtmp_hls_close_fragment(s);

    return next_stream_eof(s, v);
}


static ngx_int_t
ngx_rtmp_hls_cleanup_dir(ngx_str_t *ppath, ngx_msec_t playlen)
{
    ngx_dir_t               dir;
    time_t                  mtime, max_age;
    ngx_err_t               err;
    ngx_str_t               name, spath;
    u_char                 *p;
    ngx_int_t               nentries, nerased;
    u_char                  path[NGX_MAX_PATH + 1];

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                   "hls: cleanup path='%V' playlen=%M",
                   ppath, playlen);

    if (ngx_open_dir(ppath, &dir) != NGX_OK) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, ngx_errno,
                      "hls: cleanup open dir failed '%V'", ppath);
        return NGX_ERROR;
    }

    nentries = 0;
    nerased = 0;

    for ( ;; ) {
        ngx_set_errno(0);

        if (ngx_read_dir(&dir) == NGX_ERROR) {
            err = ngx_errno;

            if (ngx_close_dir(&dir) == NGX_ERROR) {
                ngx_log_error(NGX_LOG_CRIT, ngx_cycle->log, ngx_errno,
                              "hls: cleanup " ngx_close_dir_n " \"%V\" failed",
                              ppath);
            }

            if (err == NGX_ENOMOREFILES) {
                return nentries - nerased;
            }

            ngx_log_error(NGX_LOG_CRIT, ngx_cycle->log, err,
                          "hls: cleanup " ngx_read_dir_n
                          " '%V' failed", ppath);
            return NGX_ERROR;
        }

        name.data = ngx_de_name(&dir);
        if (name.data[0] == '.') {
            continue;
        }

        name.len = ngx_de_namelen(&dir);

        p = ngx_snprintf(path, sizeof(path) - 1, "%V/%V", ppath, &name);
        *p = 0;

        spath.data = path;
        spath.len = p - path;

        nentries++;

        if (!dir.valid_info && ngx_de_info(path, &dir) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_CRIT, ngx_cycle->log, ngx_errno,
                          "hls: cleanup " ngx_de_info_n " \"%V\" failed",
                          &spath);

            continue;
        }

        if (ngx_de_is_dir(&dir)) {

            if (ngx_rtmp_hls_cleanup_dir(&spath, playlen) == 0) {
                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                               "hls: cleanup dir '%V'", &name);

                /*
                 * null-termination gets spoiled in win32
                 * version of ngx_open_dir
                 */

                *p = 0;

                if (ngx_delete_dir(path) == NGX_FILE_ERROR) {
                    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, ngx_errno,
                                  "hls: cleanup " ngx_delete_dir_n
                                  " failed on '%V'", &spath);
                } else {
                    nerased++;
                }
            }

            continue;
        }

        if (!ngx_de_is_file(&dir)) {
            continue;
        }

        if (name.len >= 3 && name.data[name.len - 3] == '.' &&
                             name.data[name.len - 2] == 't' &&
                             name.data[name.len - 1] == 's')
        {
            max_age = playlen / 500;

        } else if (name.len >= 5 && name.data[name.len - 5] == '.' &&
                                    name.data[name.len - 4] == 'm' &&
                                    name.data[name.len - 3] == '3' &&
                                    name.data[name.len - 2] == 'u' &&
                                    name.data[name.len - 1] == '8')
        {
            max_age = playlen / 1000;

        } else if (name.len >= 4 && name.data[name.len - 4] == '.' &&
                                    name.data[name.len - 3] == 'k' &&
                                    name.data[name.len - 2] == 'e' &&
                                    name.data[name.len - 1] == 'y')
        {
            max_age = playlen / 500;

        } else {
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                           "hls: cleanup skip unknown file type '%V'", &name);
            continue;
        }

        mtime = ngx_de_mtime(&dir);
        if (mtime + max_age > ngx_cached_time->sec) {
            continue;
        }

        ngx_log_debug3(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                       "hls: cleanup '%V' mtime=%T age=%T",
                       &name, mtime, ngx_cached_time->sec - mtime);

        if (ngx_delete_file(path) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, ngx_errno,
                          "hls: cleanup " ngx_delete_file_n " failed on '%V'",
                          &spath);
            continue;
        }

        nerased++;
    }
}


#if (nginx_version >= 1011005)
static ngx_msec_t
#else
static time_t
#endif
ngx_rtmp_hls_cleanup(void *data)
{
    ngx_rtmp_hls_cleanup_t *cleanup = data;

    ngx_rtmp_hls_cleanup_dir(&cleanup->path, cleanup->playlen);

#if (nginx_version >= 1011005)
    return cleanup->playlen * 2;
#else
    return cleanup->playlen / 500;
#endif
}


static char *
ngx_rtmp_hls_variant(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_hls_app_conf_t  *hacf = conf;

    ngx_str_t                *value, *arg;
    ngx_uint_t                n;
    ngx_rtmp_hls_variant_t   *var;

    value = cf->args->elts;

    if (hacf->variant == NULL) {
        hacf->variant = ngx_array_create(cf->pool, 1,
                                         sizeof(ngx_rtmp_hls_variant_t));
        if (hacf->variant == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    var = ngx_array_push(hacf->variant);
    if (var == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(var, sizeof(ngx_rtmp_hls_variant_t));

    var->suffix = value[1];

    if (cf->args->nelts == 2) {
        return NGX_CONF_OK;
    }

    if (ngx_array_init(&var->args, cf->pool, cf->args->nelts - 2,
                       sizeof(ngx_str_t))
        != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    arg = ngx_array_push_n(&var->args, cf->args->nelts - 2);
    if (arg == NULL) {
        return NGX_CONF_ERROR;
    }

    for (n = 2; n < cf->args->nelts; n++) {
        *arg++ = value[n];
    }

    return NGX_CONF_OK;
}


static void *
ngx_rtmp_hls_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_hls_app_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_hls_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->hls = NGX_CONF_UNSET;
    conf->fraglen = NGX_CONF_UNSET_MSEC;
    conf->max_fraglen = NGX_CONF_UNSET_MSEC;
    conf->muxdelay = NGX_CONF_UNSET_MSEC;
    conf->sync = NGX_CONF_UNSET_MSEC;
    conf->playlen = NGX_CONF_UNSET_MSEC;
    conf->continuous = NGX_CONF_UNSET;
    conf->nested = NGX_CONF_UNSET;
    conf->naming = NGX_CONF_UNSET_UINT;
    conf->slicing = NGX_CONF_UNSET_UINT;
    conf->type = NGX_CONF_UNSET_UINT;
    conf->max_audio_delay = NGX_CONF_UNSET_MSEC;
    conf->audio_buffer_size = NGX_CONF_UNSET_SIZE;
    conf->cleanup = NGX_CONF_UNSET;
    conf->granularity = NGX_CONF_UNSET;
    conf->keys = NGX_CONF_UNSET;
    conf->frags_per_key = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_rtmp_hls_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_hls_app_conf_t    *prev = parent;
    ngx_rtmp_hls_app_conf_t    *conf = child;
    ngx_rtmp_hls_cleanup_t     *cleanup;

    ngx_conf_merge_value(conf->hls, prev->hls, 0);
    ngx_conf_merge_msec_value(conf->fraglen, prev->fraglen, 5000);
    ngx_conf_merge_msec_value(conf->max_fraglen, prev->max_fraglen,
                              conf->fraglen * 10);
    ngx_conf_merge_msec_value(conf->muxdelay, prev->muxdelay, 700);
    ngx_conf_merge_msec_value(conf->sync, prev->sync, 2);
    ngx_conf_merge_msec_value(conf->playlen, prev->playlen, 30000);
    ngx_conf_merge_value(conf->continuous, prev->continuous, 1);
    ngx_conf_merge_value(conf->nested, prev->nested, 0);
    ngx_conf_merge_uint_value(conf->naming, prev->naming,
                              NGX_RTMP_HLS_NAMING_SEQUENTIAL);
    ngx_conf_merge_uint_value(conf->slicing, prev->slicing,
                              NGX_RTMP_HLS_SLICING_PLAIN);
    ngx_conf_merge_uint_value(conf->type, prev->type,
                              NGX_RTMP_HLS_TYPE_LIVE);
    ngx_conf_merge_msec_value(conf->max_audio_delay, prev->max_audio_delay,
                              300);
    ngx_conf_merge_size_value(conf->audio_buffer_size, prev->audio_buffer_size,
                              NGX_RTMP_HLS_BUFSIZE);
    ngx_conf_merge_value(conf->cleanup, prev->cleanup, 1);
    ngx_conf_merge_str_value(conf->base_url, prev->base_url, "");
    ngx_conf_merge_value(conf->granularity, prev->granularity, 0);
    ngx_conf_merge_value(conf->keys, prev->keys, 0);
    ngx_conf_merge_str_value(conf->key_path, prev->key_path, "");
    ngx_conf_merge_str_value(conf->key_url, prev->key_url, "");
    ngx_conf_merge_uint_value(conf->frags_per_key, prev->frags_per_key, 0);

    if (conf->fraglen) {
        conf->winfrags = conf->playlen / conf->fraglen;
    }

    /* schedule cleanup */

    if (conf->hls && conf->path.len && conf->cleanup &&
        conf->type != NGX_RTMP_HLS_TYPE_EVENT)
    {
        if (conf->path.data[conf->path.len - 1] == '/') {
            conf->path.len--;
        }

        cleanup = ngx_pcalloc(cf->pool, sizeof(*cleanup));
        if (cleanup == NULL) {
            return NGX_CONF_ERROR;
        }

        cleanup->path = conf->path;
        cleanup->playlen = conf->playlen;

        conf->slot = ngx_pcalloc(cf->pool, sizeof(*conf->slot));
        if (conf->slot == NULL) {
            return NGX_CONF_ERROR;
        }

        conf->slot->manager = ngx_rtmp_hls_cleanup;
        conf->slot->name = conf->path;
        conf->slot->data = cleanup;
        conf->slot->conf_file = cf->conf_file->file.name.data;
        conf->slot->line = cf->conf_file->line;

        if (ngx_add_path(cf, &conf->slot) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    ngx_conf_merge_str_value(conf->path, prev->path, "");

    if (conf->keys && conf->cleanup && conf->key_path.len &&
        ngx_strcmp(conf->key_path.data, conf->path.data) != 0 &&
        conf->type != NGX_RTMP_HLS_TYPE_EVENT)
    {
        if (conf->key_path.data[conf->key_path.len - 1] == '/') {
            conf->key_path.len--;
        }

        cleanup = ngx_pcalloc(cf->pool, sizeof(*cleanup));
        if (cleanup == NULL) {
            return NGX_CONF_ERROR;
        }

        cleanup->path = conf->key_path;
        cleanup->playlen = conf->playlen;

        conf->slot = ngx_pcalloc(cf->pool, sizeof(*conf->slot));
        if (conf->slot == NULL) {
            return NGX_CONF_ERROR;
        }

        conf->slot->manager = ngx_rtmp_hls_cleanup;
        conf->slot->name = conf->key_path;
        conf->slot->data = cleanup;
        conf->slot->conf_file = cf->conf_file->file.name.data;
        conf->slot->line = cf->conf_file->line;

        if (ngx_add_path(cf, &conf->slot) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    ngx_conf_merge_str_value(conf->key_path, prev->key_path, "");

    if (conf->key_path.len == 0) {
        conf->key_path = conf->path;
    }

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_rtmp_hls_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t   *cmcf;
    ngx_rtmp_handler_pt         *h;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_rtmp_hls_video;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_rtmp_hls_audio;

    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_rtmp_hls_publish;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_rtmp_hls_close_stream;

    next_stream_begin = ngx_rtmp_stream_begin;
    ngx_rtmp_stream_begin = ngx_rtmp_hls_stream_begin;

    next_stream_eof = ngx_rtmp_stream_eof;
    ngx_rtmp_stream_eof = ngx_rtmp_hls_stream_eof;

    return NGX_OK;
}
