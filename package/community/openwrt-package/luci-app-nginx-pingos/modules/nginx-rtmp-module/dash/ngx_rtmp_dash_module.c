

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_rtmp.h>
#include <ngx_rtmp_codec_module.h>
#include "ngx_rtmp_live_module.h"
#include "ngx_rtmp_mp4.h"


static ngx_rtmp_publish_pt              next_publish;
static ngx_rtmp_close_stream_pt         next_close_stream;
static ngx_rtmp_stream_begin_pt         next_stream_begin;
static ngx_rtmp_stream_eof_pt           next_stream_eof;


static ngx_int_t ngx_rtmp_dash_postconfiguration(ngx_conf_t *cf);
static void * ngx_rtmp_dash_create_app_conf(ngx_conf_t *cf);
static char * ngx_rtmp_dash_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static ngx_int_t ngx_rtmp_dash_write_init_segments(ngx_rtmp_session_t *s);


#define NGX_RTMP_DASH_BUFSIZE           (1024*1024)
#define NGX_RTMP_DASH_MAX_MDAT          (10*1024*1024)
#define NGX_RTMP_DASH_MAX_SAMPLES       1024
#define NGX_RTMP_DASH_DIR_ACCESS        0744


typedef struct {
    uint32_t                            timestamp;
    uint32_t                            duration;
} ngx_rtmp_dash_frag_t;


typedef struct {
    ngx_uint_t                          id;
    ngx_uint_t                          opened;
    ngx_uint_t                          mdat_size;
    ngx_uint_t                          sample_count;
    ngx_uint_t                          sample_mask;
    ngx_fd_t                            fd;
    char                                type;
    uint32_t                            earliest_pres_time;
    uint32_t                            latest_pres_time;
    ngx_rtmp_mp4_sample_t               samples[NGX_RTMP_DASH_MAX_SAMPLES];
} ngx_rtmp_dash_track_t;


typedef struct {
    ngx_str_t                           playlist;
    ngx_str_t                           playlist_bak;
    ngx_str_t                           name;
    ngx_str_t                           stream;
    time_t                              start_time;

    ngx_uint_t                          nfrags;
    ngx_uint_t                          frag;
    ngx_rtmp_dash_frag_t               *frags; /* circular 2 * winfrags + 1 */

    unsigned                            opened:1;
    unsigned                            has_video:1;
    unsigned                            has_audio:1;

    ngx_file_t                          video_file;
    ngx_file_t                          audio_file;

    ngx_uint_t                          id;

    ngx_rtmp_dash_track_t               audio;
    ngx_rtmp_dash_track_t               video;
} ngx_rtmp_dash_ctx_t;


typedef struct {
    ngx_str_t                           path;
    ngx_msec_t                          playlen;
} ngx_rtmp_dash_cleanup_t;


typedef struct {
    ngx_flag_t                          dash;
    ngx_msec_t                          fraglen;
    ngx_msec_t                          playlen;
    ngx_flag_t                          nested;
    ngx_str_t                           path;
    ngx_uint_t                          winfrags;
    ngx_flag_t                          cleanup;
    ngx_path_t                         *slot;
} ngx_rtmp_dash_app_conf_t;


static ngx_command_t ngx_rtmp_dash_commands[] = {

    { ngx_string("dash"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, dash),
      NULL },

    { ngx_string("dash_fragment"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, fraglen),
      NULL },

    { ngx_string("dash_path"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, path),
      NULL },

    { ngx_string("dash_playlist_length"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, playlen),
      NULL },

    { ngx_string("dash_cleanup"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, cleanup),
      NULL },

    { ngx_string("dash_nested"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_dash_app_conf_t, nested),
      NULL },

    ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_dash_module_ctx = {
    NULL,                               /* preconfiguration */
    ngx_rtmp_dash_postconfiguration,    /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    ngx_rtmp_dash_create_app_conf,      /* create location configuration */
    ngx_rtmp_dash_merge_app_conf,       /* merge location configuration */
};


ngx_module_t  ngx_rtmp_dash_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_dash_module_ctx,          /* module context */
    ngx_rtmp_dash_commands,             /* module directives */
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


static ngx_rtmp_dash_frag_t *
ngx_rtmp_dash_get_frag(ngx_rtmp_session_t *s, ngx_int_t n)
{
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    return &ctx->frags[(ctx->frag + n) % (dacf->winfrags * 2 + 1)];
}


static void
ngx_rtmp_dash_next_frag(ngx_rtmp_session_t *s)
{
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    if (ctx->nfrags == dacf->winfrags) {
        ctx->frag++;
    } else {
        ctx->nfrags++;
    }
}


static ngx_int_t
ngx_rtmp_dash_rename_file(u_char *src, u_char *dst)
{
    /* rename file with overwrite */

#if (NGX_WIN32)
    return MoveFileEx((LPCTSTR) src, (LPCTSTR) dst, MOVEFILE_REPLACE_EXISTING);
#else
    return ngx_rename_file(src, dst);
#endif
}


static ngx_int_t
ngx_rtmp_dash_write_playlist(ngx_rtmp_session_t *s)
{
    char                      *sep;
    u_char                    *p, *last;
    ssize_t                    n;
    ngx_fd_t                   fd;
    struct tm                  tm;
    ngx_str_t                  noname, *name;
    ngx_uint_t                 i;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_codec_ctx_t      *codec_ctx;
    ngx_rtmp_dash_frag_t      *f;
    ngx_rtmp_dash_app_conf_t  *dacf;

    static u_char              buffer[NGX_RTMP_DASH_BUFSIZE];
    static u_char              start_time[sizeof("1970-09-28T12:00:00Z")];
    static u_char              pub_time[sizeof("1970-09-28T12:00:00Z")];

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (dacf == NULL || ctx == NULL || codec_ctx == NULL) {
        return NGX_ERROR;
    }

    if (ctx->id == 0) {
        ngx_rtmp_dash_write_init_segments(s);
    }

    fd = ngx_open_file(ctx->playlist_bak.data, NGX_FILE_WRONLY,
                       NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: open failed: '%V'", &ctx->playlist_bak);
        return NGX_ERROR;
    }

#define NGX_RTMP_DASH_MANIFEST_HEADER                                          \
    "<?xml version=\"1.0\"?>\n"                                                \
    "<MPD\n"                                                                   \
    "    type=\"dynamic\"\n"                                                   \
    "    xmlns=\"urn:mpeg:dash:schema:mpd:2011\"\n"                            \
    "    availabilityStartTime=\"%s\"\n"                                       \
    "    publishTime=\"%s\"\n"                                                 \
    "    minimumUpdatePeriod=\"PT%uiS\"\n"                                     \
    "    minBufferTime=\"PT%uiS\"\n"                                           \
    "    timeShiftBufferDepth=\"PT%uiS\"\n"                                    \
    "    profiles=\"urn:hbbtv:dash:profile:isoff-live:2012,"                   \
                   "urn:mpeg:dash:profile:isoff-live:2011\"\n"                 \
    "    xmlns:xsi=\"http://www.w3.org/2011/XMLSchema-instance\"\n"            \
    "    xsi:schemaLocation=\"urn:mpeg:DASH:schema:MPD:2011 DASH-MPD.xsd\">\n" \
    "  <Period start=\"PT0S\" id=\"dash\">\n"


#define NGX_RTMP_DASH_MANIFEST_VIDEO                                           \
    "    <AdaptationSet\n"                                                     \
    "        id=\"1\"\n"                                                       \
    "        segmentAlignment=\"true\"\n"                                      \
    "        maxWidth=\"%ui\"\n"                                               \
    "        maxHeight=\"%ui\"\n"                                              \
    "        maxFrameRate=\"%ui\">\n"                                          \
    "      <Representation\n"                                                  \
    "          id=\"%V_H264\"\n"                                               \
    "          mimeType=\"video/mp4\"\n"                                       \
    "          codecs=\"avc1.%02uxi%02uxi%02uxi\"\n"                           \
    "          width=\"%ui\"\n"                                                \
    "          height=\"%ui\"\n"                                               \
    "          frameRate=\"%ui\"\n"                                            \
    "          startWithSAP=\"1\"\n"                                           \
    "          bandwidth=\"%ui\">\n"                                           \
    "        <SegmentTemplate\n"                                               \
    "            timescale=\"1000\"\n"                                         \
    "            media=\"%V%s$Time$.m4v\"\n"                                   \
    "            initialization=\"%V%sinit.m4v\">\n"                           \
    "          <SegmentTimeline>\n"


#define NGX_RTMP_DASH_MANIFEST_VIDEO_FOOTER                                    \
    "          </SegmentTimeline>\n"                                           \
    "        </SegmentTemplate>\n"                                             \
    "      </Representation>\n"                                                \
    "    </AdaptationSet>\n"


#define NGX_RTMP_DASH_MANIFEST_TIME                                            \
    "             <S t=\"%uD\" d=\"%uD\"/>\n"


#define NGX_RTMP_DASH_MANIFEST_AUDIO                                           \
    "    <AdaptationSet\n"                                                     \
    "        id=\"2\"\n"                                                       \
    "        segmentAlignment=\"true\">\n"                                     \
    "      <AudioChannelConfiguration\n"                                       \
    "          schemeIdUri=\"urn:mpeg:dash:"                                   \
                                "23003:3:audio_channel_configuration:2011\"\n" \
    "          value=\"1\"/>\n"                                                \
    "      <Representation\n"                                                  \
    "          id=\"%V_AAC\"\n"                                                \
    "          mimeType=\"audio/mp4\"\n"                                       \
    "          codecs=\"mp4a.%s\"\n"                                           \
    "          audioSamplingRate=\"%ui\"\n"                                    \
    "          startWithSAP=\"1\"\n"                                           \
    "          bandwidth=\"%ui\">\n"                                           \
    "        <SegmentTemplate\n"                                               \
    "            timescale=\"1000\"\n"                                         \
    "            media=\"%V%s$Time$.m4a\"\n"                                   \
    "            initialization=\"%V%sinit.m4a\">\n"                           \
    "          <SegmentTimeline>\n"


#define NGX_RTMP_DASH_MANIFEST_AUDIO_FOOTER                                    \
    "          </SegmentTimeline>\n"                                           \
    "        </SegmentTemplate>\n"                                             \
    "      </Representation>\n"                                                \
    "    </AdaptationSet>\n"


#define NGX_RTMP_DASH_MANIFEST_FOOTER                                          \
    "  </Period>\n"                                                            \
    "</MPD>\n"

    ngx_libc_gmtime(ctx->start_time, &tm);

    ngx_sprintf(start_time, "%4d-%02d-%02dT%02d:%02d:%02dZ%Z",
                tm.tm_year + 1900, tm.tm_mon + 1,
                tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec);

    ngx_libc_gmtime(ngx_time(), &tm);

    ngx_sprintf(pub_time, "%4d-%02d-%02dT%02d:%02d:%02dZ%Z",
                tm.tm_year + 1900, tm.tm_mon + 1,
                tm.tm_mday, tm.tm_hour,
                tm.tm_min, tm.tm_sec);

    ngx_memset(buffer, 0, sizeof(buffer));
    last = buffer + sizeof(buffer);

    p = ngx_slprintf(buffer, last, NGX_RTMP_DASH_MANIFEST_HEADER,
                     start_time,
                     pub_time,
                     (ngx_uint_t) (dacf->fraglen / 1000),
                     (ngx_uint_t) (dacf->fraglen / 1000),
                     (ngx_uint_t) (dacf->fraglen / 250 + 1));

    /*
     * timeShiftBufferDepth formula:
     *     2 * minBufferTime + max_fragment_length + 1
     */

    n = ngx_write_fd(fd, buffer, p - buffer);

    ngx_str_null(&noname);

    name = (dacf->nested ? &noname : &ctx->name);
    sep = (dacf->nested ? "" : "-");

    if (ctx->has_video) {
        p = ngx_slprintf(buffer, last, NGX_RTMP_DASH_MANIFEST_VIDEO,
                         codec_ctx->width,
                         codec_ctx->height,
                         (ngx_uint_t) codec_ctx->frame_rate,
                         &ctx->name,
                         codec_ctx->avc_profile,
                         codec_ctx->avc_compat,
                         codec_ctx->avc_level,
                         codec_ctx->width,
                         codec_ctx->height,
                         (ngx_uint_t) codec_ctx->frame_rate,
                         (ngx_uint_t) (codec_ctx->video_data_rate * 1000),
                         name, sep,
                         name, sep);

        for (i = 0; i < ctx->nfrags; i++) {
            f = ngx_rtmp_dash_get_frag(s, i);
            p = ngx_slprintf(p, last, NGX_RTMP_DASH_MANIFEST_TIME,
                             f->timestamp, f->duration);
        }

        p = ngx_slprintf(p, last, NGX_RTMP_DASH_MANIFEST_VIDEO_FOOTER);

        n = ngx_write_fd(fd, buffer, p - buffer);
    }

    if (ctx->has_audio) {
        p = ngx_slprintf(buffer, last, NGX_RTMP_DASH_MANIFEST_AUDIO,
                         &ctx->name,
                         codec_ctx->audio_codec_id == NGX_RTMP_AUDIO_AAC ?
                         (codec_ctx->aac_sbr ? "40.5" : "40.2") : "6b",
                         codec_ctx->sample_rate,
                         (ngx_uint_t) (codec_ctx->audio_data_rate * 1000),
                         name, sep,
                         name, sep);

        for (i = 0; i < ctx->nfrags; i++) {
            f = ngx_rtmp_dash_get_frag(s, i);
            p = ngx_slprintf(p, last, NGX_RTMP_DASH_MANIFEST_TIME,
                             f->timestamp, f->duration);
        }

        p = ngx_slprintf(p, last, NGX_RTMP_DASH_MANIFEST_AUDIO_FOOTER);

        n = ngx_write_fd(fd, buffer, p - buffer);
    }

    p = ngx_slprintf(buffer, last, NGX_RTMP_DASH_MANIFEST_FOOTER);
    n = ngx_write_fd(fd, buffer, p - buffer);

    if (n < 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: write failed: '%V'", &ctx->playlist_bak);
        ngx_close_file(fd);
        return NGX_ERROR;
    }

    ngx_close_file(fd);

    if (ngx_rtmp_dash_rename_file(ctx->playlist_bak.data, ctx->playlist.data)
        == NGX_FILE_ERROR)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: rename failed: '%V'->'%V'",
                      &ctx->playlist_bak, &ctx->playlist);
        return NGX_ERROR;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_write_init_segments(ngx_rtmp_session_t *s)
{
    ngx_fd_t               fd;
    ngx_int_t              rc;
    ngx_buf_t              b;
    ngx_rtmp_dash_ctx_t   *ctx;
    ngx_rtmp_codec_ctx_t  *codec_ctx;

    static u_char          buffer[NGX_RTMP_DASH_BUFSIZE];

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (ctx == NULL || codec_ctx == NULL) {
        return NGX_ERROR;
    }

    /* init video */

    *ngx_sprintf(ctx->stream.data + ctx->stream.len, "init.m4v") = 0;

    fd = ngx_open_file(ctx->stream.data, NGX_FILE_RDWR, NGX_FILE_TRUNCATE,
                       NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: error creating video init file");
        return NGX_ERROR;
    }

    b.start = buffer;
    b.end = b.start + sizeof(buffer);
    b.pos = b.last = b.start;

    ngx_rtmp_mp4_write_ftyp(&b);
    ngx_rtmp_mp4_write_moov(s, &b, NGX_RTMP_MP4_VIDEO_TRACK);

    rc = ngx_write_fd(fd, b.start, (size_t) (b.last - b.start));
    if (rc == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: writing video init failed");
    }

    ngx_close_file(fd);

    /* init audio */

    *ngx_sprintf(ctx->stream.data + ctx->stream.len, "init.m4a") = 0;

    fd = ngx_open_file(ctx->stream.data, NGX_FILE_RDWR, NGX_FILE_TRUNCATE,
                       NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: error creating dash audio init file");
        return NGX_ERROR;
    }

    b.pos = b.last = b.start;

    ngx_rtmp_mp4_write_ftyp(&b);
    ngx_rtmp_mp4_write_moov(s, &b, NGX_RTMP_MP4_AUDIO_TRACK);

    rc = ngx_write_fd(fd, b.start, (size_t) (b.last - b.start));
    if (rc == NGX_ERROR) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: writing audio init failed");
    }

    ngx_close_file(fd);

    return NGX_OK;
}


static void
ngx_rtmp_dash_close_fragment(ngx_rtmp_session_t *s, ngx_rtmp_dash_track_t *t)
{
    u_char                    *pos, *pos1;
    size_t                     left;
    ssize_t                    n;
    ngx_fd_t                   fd;
    ngx_buf_t                  b;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_frag_t      *f;

    static u_char              buffer[NGX_RTMP_DASH_BUFSIZE];

    if (!t->opened) {
        return;
    }

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: close fragment id=%ui, type=%c, pts=%uD",
                   t->id, t->type, t->earliest_pres_time);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    b.start = buffer;
    b.end = buffer + sizeof(buffer);
    b.pos = b.last = b.start;

    ngx_rtmp_mp4_write_styp(&b);

    pos = b.last;
    b.last += 44; /* leave room for sidx */

    ngx_rtmp_mp4_write_moof(&b, t->earliest_pres_time, t->sample_count,
                            t->samples, t->sample_mask, t->id);
    pos1 = b.last;
    b.last = pos;

    ngx_rtmp_mp4_write_sidx(&b, t->mdat_size + 8 + (pos1 - (pos + 44)),
                            t->earliest_pres_time, t->latest_pres_time);
    b.last = pos1;
    ngx_rtmp_mp4_write_mdat(&b, t->mdat_size + 8);

    /* move the data down to make room for the headers */

    f = ngx_rtmp_dash_get_frag(s, ctx->nfrags);

    *ngx_sprintf(ctx->stream.data + ctx->stream.len, "%uD.m4%c",
                 f->timestamp, t->type) = 0;

    fd = ngx_open_file(ctx->stream.data, NGX_FILE_RDWR,
                       NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

    if (fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: error creating dash temp video file");
        goto done;
    }

    if (ngx_write_fd(fd, b.pos, (size_t) (b.last - b.pos)) == NGX_ERROR) {
        goto done;
    }

    left = (size_t) t->mdat_size;

#if (NGX_WIN32)
    if (SetFilePointer(t->fd, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "dash: SetFilePointer error");
        goto done;
    }
#else
    if (lseek(t->fd, 0, SEEK_SET) == -1) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: lseek error");
        goto done;
    }
#endif

    while (left > 0) {

        n = ngx_read_fd(t->fd, buffer, ngx_min(sizeof(buffer), left));
        if (n == NGX_ERROR) {
            break;
        }

        n = ngx_write_fd(fd, buffer, (size_t) n);
        if (n == NGX_ERROR) {
            break;
        }

        left -= n;
    }

done:

    if (fd != NGX_INVALID_FILE) {
        ngx_close_file(fd);
    }

    ngx_close_file(t->fd);

    t->fd = NGX_INVALID_FILE;
    t->opened = 0;
}


static ngx_int_t
ngx_rtmp_dash_close_fragments(ngx_rtmp_session_t *s)
{
    ngx_rtmp_dash_ctx_t  *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    if (ctx == NULL || !ctx->opened) {
        return NGX_OK;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: close fragments");

    ngx_rtmp_dash_close_fragment(s, &ctx->video);
    ngx_rtmp_dash_close_fragment(s, &ctx->audio);

    ngx_rtmp_dash_next_frag(s);

    ngx_rtmp_dash_write_playlist(s);

    ctx->id++;
    ctx->opened = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_open_fragment(ngx_rtmp_session_t *s, ngx_rtmp_dash_track_t *t,
    ngx_uint_t id, char type)
{
    ngx_rtmp_dash_ctx_t   *ctx;

    if (t->opened) {
        return NGX_OK;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: open fragment id=%ui, type='%c'", id, type);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    *ngx_sprintf(ctx->stream.data + ctx->stream.len, "raw.m4%c", type) = 0;

    t->fd = ngx_open_file(ctx->stream.data, NGX_FILE_RDWR,
                          NGX_FILE_TRUNCATE, NGX_FILE_DEFAULT_ACCESS);

    if (t->fd == NGX_INVALID_FILE) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: error creating fragment file");
        return NGX_ERROR;
    }

    t->id = id;
    t->type = type;
    t->sample_count = 0;
    t->earliest_pres_time = 0;
    t->latest_pres_time = 0;
    t->mdat_size = 0;
    t->opened = 1;

    if (type == 'v') {
        t->sample_mask = NGX_RTMP_MP4_SAMPLE_SIZE|
                         NGX_RTMP_MP4_SAMPLE_DURATION|
                         NGX_RTMP_MP4_SAMPLE_DELAY|
                         NGX_RTMP_MP4_SAMPLE_KEY;
    } else {
        t->sample_mask = NGX_RTMP_MP4_SAMPLE_SIZE|
                         NGX_RTMP_MP4_SAMPLE_DURATION;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_open_fragments(ngx_rtmp_session_t *s)
{
    ngx_rtmp_dash_ctx_t  *ctx;

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: open fragments");

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    if (ctx->opened) {
        return NGX_OK;
    }

    ngx_rtmp_dash_open_fragment(s, &ctx->video, ctx->id, 'v');

    ngx_rtmp_dash_open_fragment(s, &ctx->audio, ctx->id, 'a');

    ctx->opened = 1;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_ensure_directory(ngx_rtmp_session_t *s)
{
    size_t                     len;
    ngx_file_info_t            fi;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    static u_char              path[NGX_MAX_PATH + 1];

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);

    *ngx_snprintf(path, sizeof(path) - 1, "%V", &dacf->path) = 0;

    if (ngx_file_info(path, &fi) == NGX_FILE_ERROR) {

        if (ngx_errno != NGX_ENOENT) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "dash: " ngx_file_info_n " failed on '%V'",
                          &dacf->path);
            return NGX_ERROR;
        }

        /* ENOENT */

        if (ngx_create_dir(path, NGX_RTMP_DASH_DIR_ACCESS) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "dash: " ngx_create_dir_n " failed on '%V'",
                          &dacf->path);
            return NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "dash: directory '%V' created", &dacf->path);

    } else {

        if (!ngx_is_dir(&fi)) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                          "dash: '%V' exists and is not a directory",
                          &dacf->path);
            return  NGX_ERROR;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "dash: directory '%V' exists", &dacf->path);
    }

    if (!dacf->nested) {
        return NGX_OK;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    len = dacf->path.len;
    if (dacf->path.data[len - 1] == '/') {
        len--;
    }

    *ngx_snprintf(path, sizeof(path) - 1, "%*s/%V", len, dacf->path.data,
                  &ctx->name) = 0;

    if (ngx_file_info(path, &fi) != NGX_FILE_ERROR) {

        if (ngx_is_dir(&fi)) {
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                           "dash: directory '%s' exists", path);
            return NGX_OK;
        }

        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "dash: '%s' exists and is not a directory", path);

        return  NGX_ERROR;
    }

    if (ngx_errno != NGX_ENOENT) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: " ngx_file_info_n " failed on '%s'", path);
        return NGX_ERROR;
    }

    /* NGX_ENOENT */

    if (ngx_create_dir(path, NGX_RTMP_DASH_DIR_ACCESS) == NGX_FILE_ERROR) {
        ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                      "dash: " ngx_create_dir_n " failed on '%s'", path);
        return NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: directory '%s' created", path);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    u_char                    *p;
    size_t                     len;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_frag_t      *f;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    if (dacf == NULL || !dacf->dash || dacf->path.len == 0) {
        goto next;
    }

    if (s->interprocess) {
        goto next;
    }

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: publish: name='%s' type='%s'", v->name, v->type);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_dash_ctx_t));
        if (ctx == NULL) {
            goto next;
        }
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_dash_module);

    } else {
        if (ctx->opened) {
            goto next;
        }

        f = ctx->frags;
        ngx_memzero(ctx, sizeof(ngx_rtmp_dash_ctx_t));
        ctx->frags = f;
    }

    if (ctx->frags == NULL) {
        ctx->frags = ngx_pcalloc(s->pool,
                                 sizeof(ngx_rtmp_dash_frag_t) *
                                 (dacf->winfrags * 2 + 1));
        if (ctx->frags == NULL) {
            return NGX_ERROR;
        }
    }

    ctx->id = 0;

    if (ngx_strstr(v->name, "..")) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                      "dash: bad stream name: '%s'", v->name);
        return NGX_ERROR;
    }

    ctx->name.len = ngx_strlen(v->name);
    ctx->name.data = ngx_palloc(s->pool, ctx->name.len + 1);

    if (ctx->name.data == NULL) {
        return NGX_ERROR;
    }

    *ngx_cpymem(ctx->name.data, v->name, ctx->name.len) = 0;

    len = dacf->path.len + 1 + ctx->name.len + sizeof(".mpd");
    if (dacf->nested) {
        len += sizeof("/index") - 1;
    }

    ctx->playlist.data = ngx_palloc(s->pool, len);
    p = ngx_cpymem(ctx->playlist.data, dacf->path.data, dacf->path.len);

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
                                  ctx->stream.len + NGX_INT32_LEN +
                                  sizeof(".m4x"));

    ngx_memcpy(ctx->stream.data, ctx->playlist.data, ctx->stream.len - 1);
    ctx->stream.data[ctx->stream.len - 1] = (dacf->nested ? '/' : '-');

    if (dacf->nested) {
        p = ngx_cpymem(p, "/index.mpd", sizeof("/index.mpd") - 1);
    } else {
        p = ngx_cpymem(p, ".mpd", sizeof(".mpd") - 1);
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

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: playlist='%V' playlist_bak='%V' stream_pattern='%V'",
                   &ctx->playlist, &ctx->playlist_bak, &ctx->stream);

    ctx->start_time = ngx_time();

    if (ngx_rtmp_dash_ensure_directory(s) != NGX_OK) {
        return NGX_ERROR;
    }

next:
    return next_publish(s, v);
}


static ngx_int_t
ngx_rtmp_dash_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);

    if (dacf == NULL || !dacf->dash || ctx == NULL) {
        goto next;
    }

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "dash: delete stream");

    ngx_rtmp_dash_close_fragments(s);

next:
    return next_close_stream(s, v);
}


static void
ngx_rtmp_dash_update_fragments(ngx_rtmp_session_t *s, ngx_int_t boundary,
    uint32_t timestamp)
{
    int32_t                    d;
    ngx_int_t                  hit;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_dash_frag_t      *f;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    f = ngx_rtmp_dash_get_frag(s, ctx->nfrags);

    d = (int32_t) (timestamp - f->timestamp);

    if (d >= 0) {

        f->duration = timestamp - f->timestamp;
        hit = (f->duration >= dacf->fraglen);

        /* keep fragment lengths within 2x factor for dash.js  */
        if (f->duration >= dacf->fraglen * 2) {
            boundary = 1;
        }

    } else {

        /* sometimes clients generate slightly unordered frames */

        hit = (-d > 1000);
    }

    if (ctx->has_video && !hit) {
        boundary = 0;
    }

    if (!ctx->has_video && ctx->has_audio) {
        boundary = hit;
    }

    if (ctx->audio.mdat_size >= NGX_RTMP_DASH_MAX_MDAT) {
        boundary = 1;
    }

    if (ctx->video.mdat_size >= NGX_RTMP_DASH_MAX_MDAT) {
        boundary = 1;
    }

    if (!ctx->opened) {
        boundary = 1;
    }

    if (boundary) {
        ngx_rtmp_dash_close_fragments(s);
        ngx_rtmp_dash_open_fragments(s);

        f = ngx_rtmp_dash_get_frag(s, ctx->nfrags);
        f->timestamp = timestamp;
    }
}


static ngx_int_t
ngx_rtmp_dash_append(ngx_rtmp_session_t *s, ngx_chain_t *in,
    ngx_rtmp_dash_track_t *t, ngx_int_t key, uint32_t timestamp, uint32_t delay)
{
    u_char                 *p;
    size_t                  size, bsize;
    ngx_rtmp_mp4_sample_t  *smpl;

    static u_char           buffer[NGX_RTMP_DASH_BUFSIZE];

    p = buffer;
    size = 0;

    for (; in && size < sizeof(buffer); in = in->next) {

        bsize = (size_t) (in->buf->last - in->buf->pos);
        if (size + bsize > sizeof(buffer)) {
            bsize = (size_t) (sizeof(buffer) - size);
        }

        p = ngx_cpymem(p, in->buf->pos, bsize);
        size += bsize;
    }

    ngx_rtmp_dash_update_fragments(s, key, timestamp);

    if (t->sample_count == 0) {
        t->earliest_pres_time = timestamp;
    }

    t->latest_pres_time = timestamp;

    if (t->sample_count < NGX_RTMP_DASH_MAX_SAMPLES) {

        if (ngx_write_fd(t->fd, buffer, size) == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, s->log, ngx_errno,
                          "dash: " ngx_write_fd_n " failed");
            return NGX_ERROR;
        }

        smpl = &t->samples[t->sample_count];

        smpl->delay = delay;
        smpl->size = (uint32_t) size;
        smpl->duration = 0;
        smpl->timestamp = timestamp;
        smpl->key = (key ? 1 : 0);

        if (t->sample_count > 0) {
            smpl = &t->samples[t->sample_count - 1];
            smpl->duration = timestamp - smpl->timestamp;
        }

        t->sample_count++;
        t->mdat_size += (ngx_uint_t) size;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_dash_audio(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    u_char                     htype;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_codec_ctx_t      *codec_ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (dacf == NULL || !dacf->dash || ctx == NULL ||
        codec_ctx == NULL || h->mlen < 2)
    {
        return NGX_OK;
    }

    /* Only AAC is supported */

    if (codec_ctx->audio_codec_id != NGX_RTMP_AUDIO_AAC ||
        codec_ctx->aac_header == NULL)
    {
        return NGX_OK;
    }

    if (in->buf->last - in->buf->pos < 2) {
        return NGX_ERROR;
    }

    /* skip AAC config */

    htype = in->buf->pos[1];
    if (htype != 1) {
        return NGX_OK;
    }

    ctx->has_audio = 1;

    /* skip RTMP & AAC headers */

    in->buf->pos += 2;

    return ngx_rtmp_dash_append(s, in, &ctx->audio, 0, h->timestamp, 0);
}


static ngx_int_t
ngx_rtmp_dash_video(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
    ngx_chain_t *in)
{
    u_char                    *p;
    uint8_t                    ftype, htype;
    uint32_t                   delay;
    ngx_rtmp_dash_ctx_t       *ctx;
    ngx_rtmp_codec_ctx_t      *codec_ctx;
    ngx_rtmp_dash_app_conf_t  *dacf;

    dacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_dash_module);
    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_dash_module);
    codec_ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);

    if (dacf == NULL || !dacf->dash || ctx == NULL || codec_ctx == NULL ||
        codec_ctx->avc_header == NULL || h->mlen < 5)
    {
        return NGX_OK;
    }

    /* Only H264 is supported */

    if (codec_ctx->video_codec_id != NGX_RTMP_VIDEO_H264) {
        return NGX_OK;
    }

    if (in->buf->last - in->buf->pos < 5) {
        return NGX_ERROR;
    }

    ftype = (in->buf->pos[0] & 0xf0) >> 4;

    /* skip AVC config */

    htype = in->buf->pos[1];
    if (htype != 1) {
        return NGX_OK;
    }

    p = (u_char *) &delay;

    p[0] = in->buf->pos[4];
    p[1] = in->buf->pos[3];
    p[2] = in->buf->pos[2];
    p[3] = 0;

    ctx->has_video = 1;

    /* skip RTMP & H264 headers */

    in->buf->pos += 5;

    return ngx_rtmp_dash_append(s, in, &ctx->video, ftype == 1, h->timestamp,
                                delay);
}


static ngx_int_t
ngx_rtmp_dash_stream_begin(ngx_rtmp_session_t *s, ngx_rtmp_stream_begin_t *v)
{
    return next_stream_begin(s, v);
}


static ngx_int_t
ngx_rtmp_dash_stream_eof(ngx_rtmp_session_t *s, ngx_rtmp_stream_eof_t *v)
{
    ngx_rtmp_dash_close_fragments(s);

    return next_stream_eof(s, v);
}


static ngx_int_t
ngx_rtmp_dash_cleanup_dir(ngx_str_t *ppath, ngx_msec_t playlen)
{
    time_t           mtime, max_age;
    u_char          *p;
    u_char           path[NGX_MAX_PATH + 1], mpd_path[NGX_MAX_PATH + 1];
    ngx_dir_t        dir;
    ngx_err_t        err;
    ngx_str_t        name, spath, mpd;
    ngx_int_t        nentries, nerased;
    ngx_file_info_t  fi;

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                   "dash: cleanup path='%V' playlen=%M", ppath, playlen);

    if (ngx_open_dir(ppath, &dir) != NGX_OK) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, ngx_errno,
                       "dash: cleanup open dir failed '%V'", ppath);
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
                              "dash: cleanup " ngx_close_dir_n " \"%V\" failed",
                              ppath);
            }

            if (err == NGX_ENOMOREFILES) {
                return nentries - nerased;
            }

            ngx_log_error(NGX_LOG_CRIT, ngx_cycle->log, err,
                          "dash: cleanup " ngx_read_dir_n
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
                          "dash: cleanup " ngx_de_info_n " \"%V\" failed",
                          &spath);

            continue;
        }

        if (ngx_de_is_dir(&dir)) {

            if (ngx_rtmp_dash_cleanup_dir(&spath, playlen) == 0) {
                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                               "dash: cleanup dir '%V'", &name);

                /*
                 * null-termination gets spoiled in win32
                 * version of ngx_open_dir
                 */

                *p = 0;

                if (ngx_delete_dir(path) == NGX_FILE_ERROR) {
                    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, ngx_errno,
                                  "dash: cleanup " ngx_delete_dir_n
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

        if (name.len >= 8 && name.data[name.len - 8] == 'i' &&
                             name.data[name.len - 7] == 'n' &&
                             name.data[name.len - 6] == 'i' &&
                             name.data[name.len - 5] == 't' &&
                             name.data[name.len - 4] == '.' &&
                             name.data[name.len - 3] == 'm' &&
                             name.data[name.len - 2] == '4')
        {
            if (name.len == 8) {
                ngx_str_set(&mpd, "index");
            } else {
                mpd.data = name.data;
                mpd.len = name.len - 9;
            }

            p = ngx_snprintf(mpd_path, sizeof(mpd_path) - 1, "%V/%V.mpd",
                             ppath, &mpd);
            *p = 0;

            if (ngx_file_info(mpd_path, &fi) != NGX_FILE_ERROR) {
                ngx_log_debug2(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                               "dash: cleanup '%V' delayed, mpd exists '%s'",
                               &name, mpd_path);
                continue;
            }

            ngx_log_debug2(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                           "dash: cleanup '%V' allowed, mpd missing '%s'",
                           &name, mpd_path);

            max_age = 0;

        } else if (name.len >= 4 && name.data[name.len - 4] == '.' &&
                                    name.data[name.len - 3] == 'm' &&
                                    name.data[name.len - 2] == '4' &&
                                    name.data[name.len - 1] == 'v')
        {
            max_age = playlen / 500;

        } else if (name.len >= 4 && name.data[name.len - 4] == '.' &&
                                    name.data[name.len - 3] == 'm' &&
                                    name.data[name.len - 2] == '4' &&
                                    name.data[name.len - 1] == 'a')
        {
            max_age = playlen / 500;

        } else if (name.len >= 4 && name.data[name.len - 4] == '.' &&
                                    name.data[name.len - 3] == 'm' &&
                                    name.data[name.len - 2] == 'p' &&
                                    name.data[name.len - 1] == 'd')
        {
            max_age = playlen / 500;

        } else if (name.len >= 4 && name.data[name.len - 4] == '.' &&
                                    name.data[name.len - 3] == 'r' &&
                                    name.data[name.len - 2] == 'a' &&
                                    name.data[name.len - 1] == 'w')
        {
            max_age = playlen / 1000;

        } else {
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                           "dash: cleanup skip unknown file type '%V'", &name);
            continue;
        }

        mtime = ngx_de_mtime(&dir);
        if (mtime + max_age > ngx_cached_time->sec) {
            continue;
        }

        ngx_log_debug3(NGX_LOG_DEBUG_RTMP, ngx_cycle->log, 0,
                       "dash: cleanup '%V' mtime=%T age=%T",
                       &name, mtime, ngx_cached_time->sec - mtime);

        if (ngx_delete_file(path) == NGX_FILE_ERROR) {
            ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, ngx_errno,
                          "dash: cleanup " ngx_delete_file_n " failed on '%V'",
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
ngx_rtmp_dash_cleanup(void *data)
{
    ngx_rtmp_dash_cleanup_t *cleanup = data;

    ngx_rtmp_dash_cleanup_dir(&cleanup->path, cleanup->playlen);

#if (nginx_version >= 1011005)
    return cleanup->playlen * 2;
#else
    return cleanup->playlen / 500;
#endif
}


static void *
ngx_rtmp_dash_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_dash_app_conf_t *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_dash_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->dash = NGX_CONF_UNSET;
    conf->fraglen = NGX_CONF_UNSET_MSEC;
    conf->playlen = NGX_CONF_UNSET_MSEC;
    conf->cleanup = NGX_CONF_UNSET;
    conf->nested = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_rtmp_dash_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_dash_app_conf_t    *prev = parent;
    ngx_rtmp_dash_app_conf_t    *conf = child;
    ngx_rtmp_dash_cleanup_t     *cleanup;

    ngx_conf_merge_value(conf->dash, prev->dash, 0);
    ngx_conf_merge_msec_value(conf->fraglen, prev->fraglen, 5000);
    ngx_conf_merge_msec_value(conf->playlen, prev->playlen, 30000);
    ngx_conf_merge_value(conf->cleanup, prev->cleanup, 1);
    ngx_conf_merge_value(conf->nested, prev->nested, 0);

    if (conf->fraglen) {
        conf->winfrags = conf->playlen / conf->fraglen;
    }

    /* schedule cleanup */

    if (conf->dash && conf->path.len && conf->cleanup) {
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

        conf->slot->manager = ngx_rtmp_dash_cleanup;
        conf->slot->name = conf->path;
        conf->slot->data = cleanup;
        conf->slot->conf_file = cf->conf_file->file.name.data;
        conf->slot->line = cf->conf_file->line;

        if (ngx_add_path(cf, &conf->slot) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    ngx_conf_merge_str_value(conf->path, prev->path, "");

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_rtmp_dash_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_handler_pt        *h;
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_rtmp_dash_video;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_rtmp_dash_audio;

    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_rtmp_dash_publish;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_rtmp_dash_close_stream;

    next_stream_begin = ngx_rtmp_stream_begin;
    ngx_rtmp_stream_begin = ngx_rtmp_dash_stream_begin;

    next_stream_eof = ngx_rtmp_stream_eof;
    ngx_rtmp_stream_eof = ngx_rtmp_dash_stream_eof;

    return NGX_OK;
}
