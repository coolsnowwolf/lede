
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp_cmd_module.h"
#include "ngx_rtmp_codec_module.h"


static ngx_rtmp_publish_pt  next_publish;
static ngx_rtmp_play_pt     next_play;


static ngx_int_t ngx_rtmp_log_postconfiguration(ngx_conf_t *cf);
static void *ngx_rtmp_log_create_main_conf(ngx_conf_t *cf);
static void * ngx_rtmp_log_create_app_conf(ngx_conf_t *cf);
static char * ngx_rtmp_log_merge_app_conf(ngx_conf_t *cf,
       void *parent, void *child);
static char * ngx_rtmp_log_set_log(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char * ngx_rtmp_log_set_format(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char * ngx_rtmp_log_compile_format(ngx_conf_t *cf, ngx_array_t *ops,
       ngx_array_t *args, ngx_uint_t s);


typedef struct ngx_rtmp_log_op_s ngx_rtmp_log_op_t;

#define MAX_ACCESS_LOG_LINE_LEN     4096

typedef size_t (*ngx_rtmp_log_op_getlen_pt)(ngx_rtmp_session_t *s,
        ngx_rtmp_log_op_t *op);
typedef u_char * (*ngx_rtmp_log_op_getdata_pt)(ngx_rtmp_session_t *s,
        u_char *buf, ngx_rtmp_log_op_t *log);


struct ngx_rtmp_log_op_s {
    ngx_rtmp_log_op_getlen_pt   getlen;
    ngx_rtmp_log_op_getdata_pt  getdata;
    ngx_str_t                   value;
    ngx_uint_t                  offset;
};


typedef struct {
    ngx_str_t                   name;
    ngx_rtmp_log_op_getlen_pt   getlen;
    ngx_rtmp_log_op_getdata_pt  getdata;
    ngx_uint_t                  offset;
} ngx_rtmp_log_var_t;


typedef struct {
    ngx_str_t                   name;
    ngx_array_t                *ops; /* ngx_rtmp_log_op_t */
} ngx_rtmp_log_fmt_t;


typedef struct {
    ngx_open_file_t            *file;
    time_t                      disk_full_time;
    time_t                      error_log_time;
    ngx_msec_t                  trunc_timer;
    ngx_rtmp_log_fmt_t         *format;
} ngx_rtmp_log_t;


typedef struct {
    ngx_array_t                *logs; /* ngx_rtmp_log_t */
    ngx_uint_t                  off;
} ngx_rtmp_log_app_conf_t;


typedef struct {
    ngx_array_t                 formats; /* ngx_rtmp_log_fmt_t */
    ngx_uint_t                  combined_used;
} ngx_rtmp_log_main_conf_t;


typedef struct {
    ngx_rtmp_session_t         *session;
    ngx_event_t                 event;
    ngx_rtmp_log_t             *log;
} ngx_rtmp_log_timer_ctx_t;

typedef struct {
    unsigned                    play:1;
    unsigned                    publish:1;
    ngx_uint_t                  last_bytes;
    u_char                      name[NGX_RTMP_MAX_NAME];
    u_char                      stream[NGX_RTMP_MAX_NAME];
    u_char                      args[NGX_RTMP_MAX_ARGS];
    ngx_array_t                 timers; /* ngx_rtmp_log_timer_ctx_t */
} ngx_rtmp_log_ctx_t;


static ngx_str_t ngx_rtmp_access_log = ngx_string(NGX_HTTP_LOG_PATH);


static ngx_command_t  ngx_rtmp_log_commands[] = {

    { ngx_string("access_log"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE123,
      ngx_rtmp_log_set_log,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("log_format"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_2MORE,
      ngx_rtmp_log_set_format,
      NGX_RTMP_MAIN_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_log_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_log_postconfiguration,         /* postconfiguration */
    ngx_rtmp_log_create_main_conf,          /* create main configuration */
    NULL,                                   /* init main configuration */
    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */
    ngx_rtmp_log_create_app_conf,           /* create app configuration */
    ngx_rtmp_log_merge_app_conf             /* merge app configuration */
};


ngx_module_t  ngx_rtmp_log_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_log_module_ctx,               /* module context */
    ngx_rtmp_log_commands,                  /* module directives */
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


static ngx_str_t ngx_rtmp_combined_fmt =
    ngx_string("$remote_addr [$time_local] $command "
               "\"$app\" \"$name\" \"$args\" - "
               "$bytes_received $bytes_sent "
               "\"$pageurl\" \"$flashver\" ($session_readable_time)");


static size_t
ngx_rtmp_log_var_default_getlen(ngx_rtmp_session_t *s, ngx_rtmp_log_op_t *op)
{
    return op->value.len;
}


static u_char *
ngx_rtmp_log_var_default_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    return ngx_cpymem(buf, op->value.data, op->value.len);
}

static size_t
ngx_rtmp_log_var_parg_getlen(ngx_rtmp_session_t *s, ngx_rtmp_log_op_t *op)
{
    ngx_str_t                  *name = &(op->value);
    u_char                     *arg;
    size_t                      len;
    ngx_str_t                   value;

    len = name->len - (sizeof("parg_") - 1);
    arg = name->data + sizeof("parg_") - 1;

    if (ngx_rtmp_arg(s, arg, len, &value) != NGX_OK) {
        return op->value.len;
    }

    return value.len;
}


static u_char *
ngx_rtmp_log_var_parg_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_str_t                  *name = &(op->value);
    u_char                     *arg;
    size_t                      len;
    ngx_str_t                   value;

    len = name->len - (sizeof("parg_") - 1);
    arg = name->data + sizeof("parg_") - 1;

    if (ngx_rtmp_arg(s, arg, len, &value) != NGX_OK) {
        return ngx_cpymem(buf, op->value.data, op->value.len);
    }

    return ngx_cpymem(buf, value.data, value.len);
}

static size_t
ngx_rtmp_log_var_connection_getlen(ngx_rtmp_session_t *s, ngx_rtmp_log_op_t *op)
{
    return NGX_INT_T_LEN;
}

static u_char *
ngx_rtmp_log_var_connection_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    return ngx_sprintf(buf, "%ui", (ngx_uint_t) s->number);
}


static size_t
ngx_rtmp_log_var_remote_addr_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return s->remote_addr_text.len;
}


static u_char *
ngx_rtmp_log_var_remote_addr_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    return ngx_cpymem(buf, s->remote_addr_text.data,
                           s->remote_addr_text.len);
}


static size_t
ngx_rtmp_log_var_msec_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return NGX_TIME_T_LEN + 4;
}


static u_char *
ngx_rtmp_log_var_msec_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_time_t  *tp;

    tp = ngx_timeofday();

    return ngx_sprintf(buf, "%T.%03M", tp->sec, tp->msec);
}

static size_t
ngx_rtmp_log_var_sec_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return NGX_TIME_T_LEN + 4;
}


static u_char *
ngx_rtmp_log_var_sec_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_time_t  *tp;

    tp = ngx_timeofday();

    return ngx_sprintf(buf, "%T", tp->sec);
}


static size_t
ngx_rtmp_log_var_session_string_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return ((ngx_str_t *) ((u_char *) s + op->offset))->len;
}


static u_char *
ngx_rtmp_log_var_session_string_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_str_t  *str;

    str = (ngx_str_t *) ((u_char *) s + op->offset);

    return ngx_cpymem(buf, str->data, str->len);
}


static size_t
ngx_rtmp_log_var_command_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return sizeof("PLAY+PUBLISH") - 1;
}


static u_char *
ngx_rtmp_log_var_command_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_rtmp_log_ctx_t *ctx;
    ngx_str_t          *cmd;
    ngx_uint_t          n;

    static ngx_str_t    commands[] = {
        ngx_string("NONE"),
        ngx_string("PLAY"),
        ngx_string("PUBLISH"),
        ngx_string("PLAY+PUBLISH")
    };

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_log_module);

    n = ctx ? (ctx->play + ctx->publish * 2) : 0;

    cmd = &commands[n];

    return ngx_cpymem(buf, cmd->data, cmd->len);
}


static size_t
ngx_rtmp_log_var_context_cstring_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return ngx_max(NGX_RTMP_MAX_NAME, NGX_RTMP_MAX_ARGS);
}


static u_char *
ngx_rtmp_log_var_context_cstring_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_rtmp_log_ctx_t *ctx;
    u_char             *p;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_log_module);
    if (ctx == NULL) {
        return buf;
    }

    p = (u_char *) ctx + op->offset;
    while (*p) {
        *buf++ = *p++;
    }

    return buf;
}


static size_t
ngx_rtmp_log_var_session_uint32_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return NGX_INT32_LEN;
}


static u_char *
ngx_rtmp_log_var_session_uint32_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    uint32_t   *v;

    v = (uint32_t *) ((uint8_t *) s + op->offset);

    return ngx_sprintf(buf, "%uD", *v);
}

static u_char *
ngx_rtmp_log_var_session_video_flag(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_rtmp_codec_ctx_t   *ctx;
    ngx_int_t               flag;

    flag = 0;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx) {
        flag = (ctx->video_codec_id > 0);
    }

    return ngx_sprintf(buf, "%d", flag);
}

static u_char *
ngx_rtmp_log_var_session_audio_flag(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_rtmp_codec_ctx_t   *ctx;
    ngx_int_t               flag;

    flag = 0;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_codec_module);
    if (ctx) {
        flag = (ctx->audio_codec_id > 0);
    }

    return ngx_sprintf(buf, "%d", flag);
}

static u_char *
ngx_rtmp_log_var_session_bitrate_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    ngx_rtmp_log_ctx_t   *ctx;
    uint32_t              bitrate;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_log_module);
    if (!ctx) {
        return buf;
    }

    bitrate = 0;
    if (s->publishing) {
        bitrate = 8 * (s->in_bytes - ctx->last_bytes);
        ctx->last_bytes = s->in_bytes;
    } else {
        bitrate = 8 * (s->out_bytes - ctx->last_bytes);
        ctx->last_bytes = s->out_bytes;
    }

    return ngx_sprintf(buf, "%D", bitrate);
}

static size_t
ngx_rtmp_log_var_time_local_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return ngx_cached_http_log_time.len;
}


static u_char *
ngx_rtmp_log_var_time_local_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    return ngx_cpymem(buf, ngx_cached_http_log_time.data,
                      ngx_cached_http_log_time.len);
}


static size_t
ngx_rtmp_log_var_session_time_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return NGX_INT64_LEN;
}


static u_char *
ngx_rtmp_log_var_session_time_getdata(ngx_rtmp_session_t *s, u_char *buf,
    ngx_rtmp_log_op_t *op)
{
    return ngx_sprintf(buf, "%L",
                       (int64_t) (ngx_current_msec - s->epoch) / 1000);
}


static size_t
ngx_rtmp_log_var_session_readable_time_getlen(ngx_rtmp_session_t *s,
    ngx_rtmp_log_op_t *op)
{
    return NGX_INT_T_LEN + sizeof("d 23h 59m 59s") - 1;
}


static u_char *
ngx_rtmp_log_var_session_readable_time_getdata(ngx_rtmp_session_t *s,
    u_char *buf, ngx_rtmp_log_op_t *op)
{
    int64_t     v;
    ngx_uint_t  days, hours, minutes, seconds;

    v = (ngx_current_msec - s->epoch) / 1000;

    days = (ngx_uint_t) (v / (60 * 60 * 24));
    hours = (ngx_uint_t) (v / (60 * 60) % 24);
    minutes = (ngx_uint_t) (v / 60 % 60);
    seconds = (ngx_uint_t) (v % 60);

    if (days) {
        buf = ngx_sprintf(buf, "%uid ", days);
    }

    if (days || hours) {
        buf = ngx_sprintf(buf, "%uih ", hours);
    }

    if (days || hours || minutes) {
        buf = ngx_sprintf(buf, "%uim ", minutes);
    }

    buf = ngx_sprintf(buf, "%uis", seconds);

    return buf;
}


static ngx_rtmp_log_var_t ngx_rtmp_log_vars[] = {
    { ngx_string("connection"),
      ngx_rtmp_log_var_connection_getlen,
      ngx_rtmp_log_var_connection_getdata,
      0 },

    { ngx_string("remote_addr"),
      ngx_rtmp_log_var_remote_addr_getlen,
      ngx_rtmp_log_var_remote_addr_getdata,
      0 },

    { ngx_string("app"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, app) },

    { ngx_string("flashver"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, flashver) },

    { ngx_string("swfurl"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, swf_url) },

    { ngx_string("tcurl"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, tc_url) },

    { ngx_string("domain"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, domain) },

    { ngx_string("parg_"),
      ngx_rtmp_log_var_parg_getlen,
      ngx_rtmp_log_var_parg_getdata,
      0 },

    { ngx_string("pageurl"),
      ngx_rtmp_log_var_session_string_getlen,
      ngx_rtmp_log_var_session_string_getdata,
      offsetof(ngx_rtmp_session_t, page_url) },

    { ngx_string("command"),
      ngx_rtmp_log_var_command_getlen,
      ngx_rtmp_log_var_command_getdata,
      0 },

    { ngx_string("name"),
      ngx_rtmp_log_var_context_cstring_getlen,
      ngx_rtmp_log_var_context_cstring_getdata,
      offsetof(ngx_rtmp_log_ctx_t, name) },

    { ngx_string("stream"),
      ngx_rtmp_log_var_context_cstring_getlen,
      ngx_rtmp_log_var_context_cstring_getdata,
      offsetof(ngx_rtmp_log_ctx_t, stream) },

    { ngx_string("args"),
      ngx_rtmp_log_var_context_cstring_getlen,
      ngx_rtmp_log_var_context_cstring_getdata,
      offsetof(ngx_rtmp_log_ctx_t, args) },

    { ngx_string("bytes_sent"),
      ngx_rtmp_log_var_session_uint32_getlen,
      ngx_rtmp_log_var_session_uint32_getdata,
      offsetof(ngx_rtmp_session_t, out_bytes) },

    { ngx_string("bytes_received"),
      ngx_rtmp_log_var_session_uint32_getlen,
      ngx_rtmp_log_var_session_uint32_getdata,
      offsetof(ngx_rtmp_session_t, in_bytes) },

    { ngx_string("time_local"),
      ngx_rtmp_log_var_time_local_getlen,
      ngx_rtmp_log_var_time_local_getdata,
      0 },

    { ngx_string("msec"),
      ngx_rtmp_log_var_msec_getlen,
      ngx_rtmp_log_var_msec_getdata,
      0 },

    { ngx_string("ntp"),
      ngx_rtmp_log_var_sec_getlen,
      ngx_rtmp_log_var_sec_getdata,
      0 },

    { ngx_string("session_time"),
      ngx_rtmp_log_var_session_time_getlen,
      ngx_rtmp_log_var_session_time_getdata,
      0 },

    { ngx_string("session_readable_time"),
      ngx_rtmp_log_var_session_readable_time_getlen,
      ngx_rtmp_log_var_session_readable_time_getdata,
      0 },

    { ngx_string("bitrate"),
      ngx_rtmp_log_var_session_uint32_getlen,
      ngx_rtmp_log_var_session_bitrate_getdata,
      0 },

    { ngx_string("be_video"),
      ngx_rtmp_log_var_session_uint32_getlen,
      ngx_rtmp_log_var_session_video_flag,
      0 },

    { ngx_string("be_audio"),
      ngx_rtmp_log_var_session_uint32_getlen,
      ngx_rtmp_log_var_session_audio_flag,
      0 },

    { ngx_null_string, NULL, NULL, 0 }
};


static void *
ngx_rtmp_log_create_main_conf(ngx_conf_t *cf)
{
    ngx_rtmp_log_main_conf_t   *lmcf;
    ngx_rtmp_log_fmt_t         *fmt;

    lmcf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_log_main_conf_t));
    if (lmcf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&lmcf->formats, cf->pool, 4, sizeof(ngx_rtmp_log_fmt_t))
        != NGX_OK)
    {
        return NULL;
    }

    fmt = ngx_array_push(&lmcf->formats);
    if (fmt == NULL) {
        return NULL;
    }

    ngx_str_set(&fmt->name, "combined");

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_rtmp_log_op_t));
    if (fmt->ops == NULL) {
        return NULL;
    }

    return lmcf;

}


static void *
ngx_rtmp_log_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_log_app_conf_t *lacf;

    lacf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_log_app_conf_t));
    if (lacf == NULL) {
        return NULL;
    }

    return lacf;
}


static char *
ngx_rtmp_log_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_log_app_conf_t    *prev = parent;
    ngx_rtmp_log_app_conf_t    *conf = child;
    ngx_rtmp_log_main_conf_t   *lmcf;
    ngx_rtmp_log_fmt_t         *fmt;
    ngx_rtmp_log_t             *log;

    if (conf->logs || conf->off) {
        return NGX_OK;
    }

    conf->logs = prev->logs;
    conf->off = prev->off;

    if (conf->logs || conf->off) {
        return NGX_OK;
    }

    conf->logs = ngx_array_create(cf->pool, 2, sizeof(ngx_rtmp_log_t));
    if (conf->logs == NULL) {
        return NGX_CONF_ERROR;
    }

    log = ngx_array_push(conf->logs);
    if (log == NULL) {
        return NGX_CONF_ERROR;
    }

    log->file = ngx_conf_open_file(cf->cycle, &ngx_rtmp_access_log);
    if (log->file == NULL) {
        return NGX_CONF_ERROR;
    }

    log->disk_full_time = 0;
    log->error_log_time = 0;
    log->trunc_timer = 0;

    lmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_log_module);
    fmt = lmcf->formats.elts;

    log->format = &fmt[0];
    lmcf->combined_used = 1;

    return NGX_CONF_OK;
}


/*
 * access_log off;
 * access_log file;
 * access_log file format_name;
 * access_log file trunc=1m;
 * access_log file format_name trunc=1m;
 */
static char *
ngx_rtmp_log_set_log(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_log_app_conf_t    *lacf = conf;

    ngx_rtmp_log_main_conf_t   *lmcf;
    ngx_rtmp_log_fmt_t         *fmt;
    ngx_rtmp_log_t             *log;
    ngx_str_t                  *value, name, timer;
    ngx_uint_t                  n;
    ngx_flag_t                  format_configured;

    name.len = 0;
    format_configured = 0;

    value = cf->args->elts;

    if (ngx_strcmp(value[1].data, "off") == 0) {
        lacf->off = 1;
        return NGX_CONF_OK;
    }

    if (lacf->logs == NULL) {
        lacf->logs = ngx_array_create(cf->pool, 2, sizeof(ngx_rtmp_log_t));
        if (lacf->logs == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    log = ngx_array_push(lacf->logs);
    if (log == NULL) {
        return NGX_CONF_ERROR;
    }

    ngx_memzero(log, sizeof(*log));

    lmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_log_module);

    log->file = ngx_conf_open_file(cf->cycle, &value[1]);
    if (log->file == NULL) {
        return NGX_CONF_ERROR;
    }

    for (n = 2; n < cf->args->nelts; ++n) {
        /* sizeof("trunc=") - 1 = 6 */
        if (ngx_memcmp("trunc=", value[n].data, 6) == 0) {
            timer.data = value[n].data + 6;
            timer.len = value[n].len - 6;

            log->trunc_timer = ngx_parse_time(&timer, 0);
            if (log->trunc_timer == (ngx_msec_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                        "unknown trunc timer format \"%V\"", &timer);
                return NGX_CONF_ERROR;
            }
        } else {
            if (format_configured) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                        "format name has been configured");
                return NGX_CONF_ERROR;
            }

            format_configured = 1;
            name = value[n];
        }
    }

    if (name.len == 0) {
        ngx_str_set(&name, "combined");
        lmcf->combined_used = 1;

    } else {
        if (ngx_strcmp(name.data, "combined") == 0) {
            lmcf->combined_used = 1;
        }
    }

    fmt = lmcf->formats.elts;
    for (n = 0; n < lmcf->formats.nelts; ++n, ++fmt) {
        if (fmt->name.len == name.len &&
            ngx_strncasecmp(fmt->name.data, name.data, name.len) == 0)
        {
            log->format = fmt;
            break;
        }
    }

    if (log->format == NULL) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "unknown log format \"%V\"",
                           &name);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static char *
ngx_rtmp_log_set_format(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_log_main_conf_t   *lmcf = conf;
    ngx_rtmp_log_fmt_t         *fmt;
    ngx_str_t                  *value;
    ngx_uint_t                  i;

    value = cf->args->elts;

    if (cf->cmd_type != NGX_RTMP_MAIN_CONF) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "\"log_format\" directive can only be used on "
                           "\"rtmp\" level");
    }

    fmt = lmcf->formats.elts;
    for (i = 0; i < lmcf->formats.nelts; i++) {
        if (fmt[i].name.len == value[1].len &&
            ngx_strcmp(fmt[i].name.data, value[1].data) == 0)
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "duplicate \"log_format\" name \"%V\"",
                               &value[1]);
            return NGX_CONF_ERROR;
        }
    }

    fmt = ngx_array_push(&lmcf->formats);
    if (fmt == NULL) {
        return NGX_CONF_ERROR;
    }

    fmt->name = value[1];

    fmt->ops = ngx_array_create(cf->pool, 16, sizeof(ngx_rtmp_log_op_t));
    if (fmt->ops == NULL) {
        return NGX_CONF_ERROR;
    }

    return ngx_rtmp_log_compile_format(cf, fmt->ops, cf->args, 2);
}


static char *
ngx_rtmp_log_compile_format(ngx_conf_t *cf, ngx_array_t *ops, ngx_array_t *args,
                            ngx_uint_t s)
{
    size_t              i, len;
    u_char             *data, *d, c;
    ngx_uint_t          bracket;
    ngx_str_t          *value, var, pre_var;
    ngx_rtmp_log_op_t  *op;
    ngx_rtmp_log_var_t *v;
    u_char             *p;

    value = args->elts;

    for (; s < args->nelts; ++s) {
        i = 0;

        len = value[s].len;
        d = value[s].data;

        while (i < len) {

            op = ngx_array_push(ops);
            if (op == NULL) {
                return NGX_CONF_ERROR;
            }

            ngx_memzero(op, sizeof(*op));

            data = &d[i];

            if (d[i] == '$') {
                if (++i == len) {
                    goto invalid;
                }

                if (d[i] == '{') {
                    bracket = 1;
                    if (++i == len) {
                        goto invalid;
                    }
                } else {
                    bracket = 0;
                }

                var.data = &d[i];

                for (var.len = 0; i < len; ++i, ++var.len) {
                    c = d[i];

                    if (c == '}' && bracket) {
                        ++i;
                        bracket = 0;
                        break;
                    }

                    if ((c >= 'A' && c <= 'Z') ||
                        (c >= 'a' && c <= 'z') ||
                        (c >= '0' && c <= '9') ||
                        (c == '_'))
                    {
                        continue;
                    }

                    break;
                }

                if (bracket) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "missing closing bracket in \"%V\"",
                                       &var);
                    return NGX_CONF_ERROR;
                }

                if (var.len == 0) {
                    goto invalid;
                }

                for (v = ngx_rtmp_log_vars; v->name.len; ++v) {
                    if (v->name.len == var.len &&
                        ngx_strncmp(v->name.data, var.data, var.len) == 0)
                    {
                        op->getlen = v->getlen;
                        op->getdata = v->getdata;
                        op->offset = v->offset;
                        break;
                    }
                }

                if (v->name.len == 0) {
                    p = ngx_strnstr(var.data, "_", var.len);
                    if (p == NULL) {
                        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "unknown variable \"%V\"", &var);
                        return NGX_CONF_ERROR;
                    }

                    pre_var.data = var.data;
                    pre_var.len = p - var.data + 1;
                    for (v = ngx_rtmp_log_vars; v->name.len; ++v) {
                        if (v->name.len == pre_var.len &&
                            !ngx_strncmp(v->name.data, pre_var.data, pre_var.len))
                        {
                            op->getlen = v->getlen;
                            op->getdata = v->getdata;
                            op->offset = v->offset;
                            op->value = var;
                            break;
                        }
                    }
                }

                if (v->name.len == 0) {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "unknown variable \"%V\"", &var);
                    return NGX_CONF_ERROR;
                }

                continue;
            }

            ++i;

            while (i < len && d[i] != '$') {
                ++i;
            }

            op->getlen = ngx_rtmp_log_var_default_getlen;
            op->getdata = ngx_rtmp_log_var_default_getdata;

            op->value.len = &d[i] - data;

            op->value.data = ngx_pnalloc(cf->pool, op->value.len);
            if (op->value.data == NULL) {
                return NGX_CONF_ERROR;
            }

            ngx_memcpy(op->value.data, data, op->value.len);
        }
    }

    return NGX_CONF_OK;

invalid:

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "invalid parameter \"%s\"", data);

    return NGX_CONF_ERROR;
}


static ngx_rtmp_log_ctx_t *
ngx_rtmp_log_set_names(ngx_rtmp_session_t *s, u_char *name, u_char *args)
{
    ngx_rtmp_log_ctx_t         *ctx;
    ngx_rtmp_log_app_conf_t    *lacf;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_log_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_log_ctx_t));
        if (ctx == NULL) {
            return NULL;
        }

        lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_log_module);

        if (lacf->logs) {
            if (ngx_array_init(&ctx->timers, s->pool,
                lacf->logs->nelts, sizeof(ngx_rtmp_log_timer_ctx_t)) != NGX_OK)
            {
                return NULL;
            }
        }

        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_log_module);
    }

    ngx_memcpy(ctx->stream, s->stream.data, s->stream.len);
    ngx_memcpy(ctx->name, name, NGX_RTMP_MAX_NAME);
    ngx_memcpy(ctx->args, args, NGX_RTMP_MAX_ARGS);

    return ctx;
}


static void
ngx_rtmp_log_write(ngx_rtmp_session_t *s, ngx_rtmp_log_t *log, u_char *buf,
                   size_t len)
{
    u_char *name;
    time_t  now;
    ssize_t n;
    int     err;

    err = 0;
    name = log->file->name.data;
    n = ngx_write_fd(log->file->fd, buf, len);

    if (n == (ssize_t) len) {
        return;
    }

    now = ngx_time();

    if (n == -1) {
        err = ngx_errno;

        if (err == NGX_ENOSPC) {
            log->disk_full_time = now;
        }

        if (now - log->error_log_time > 59) {
            ngx_log_error(NGX_LOG_ALERT, s->log, err,
                          ngx_write_fd_n " to \"%s\" failed", name);
            log->error_log_time = now;
        }
    }

    if (now - log->error_log_time > 59) {
        ngx_log_error(NGX_LOG_ALERT, s->log, err,
                      ngx_write_fd_n " to \"%s\" was incomplete: %z of %uz",
                      name, n, len);
        log->error_log_time = now;
    }
}

static void
ngx_rtmp_log_pre_write(ngx_rtmp_session_t *s, ngx_rtmp_log_t *log)
{
    ngx_rtmp_log_op_t          *op;
    u_char                      line[MAX_ACCESS_LOG_LINE_LEN], *p;
    size_t                      len;
    ngx_uint_t                  n;

    if (ngx_time() == log->disk_full_time) {
        /* FreeBSD full disk protection;
         * nginx http logger does the same */
        return;
    }

    len = 0;
    op = log->format->ops->elts;
    for (n = 0; n < log->format->ops->nelts; ++n, ++op) {
        len += op->getlen(s, op);
    }
    len += NGX_LINEFEED_SIZE;

    if (len > MAX_ACCESS_LOG_LINE_LEN) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "Access line len %z greater than %d",
                len, MAX_ACCESS_LOG_LINE_LEN);
        ngx_rtmp_finalize_session(s);

        return;
    }

    p = line;
    op = log->format->ops->elts;
    for (n = 0; n < log->format->ops->nelts; ++n, ++op) {
        p = op->getdata(s, p, op);
    }

    ngx_linefeed(p);

    ngx_rtmp_log_write(s, log, line, p - line);
}

static void
ngx_rtmp_log_trunc_timer(ngx_event_t *ev)
{
    ngx_rtmp_log_timer_ctx_t   *ltctx;
    ngx_rtmp_session_t         *s;
    ngx_rtmp_log_t             *log;
    ngx_msec_t                  t;

    ltctx = ev->data;

    s = ltctx->session;
    log = ltctx->log;

    ngx_rtmp_log_pre_write(s, log);

    t = log->trunc_timer - ngx_current_msec % log->trunc_timer;
    ngx_add_timer(ev, t);
}

static void
ngx_rtmp_log_add_trunc_timer(ngx_rtmp_session_t *s, ngx_rtmp_log_ctx_t *ctx,
        ngx_rtmp_log_t *log)
{
    ngx_rtmp_log_timer_ctx_t   *ltctx;
    ngx_event_t                *e;
    ngx_msec_t                  t;

    ltctx = ngx_array_push(&ctx->timers);
    ngx_memzero(ltctx, sizeof(ngx_rtmp_log_timer_ctx_t));
    ltctx->session = s;
    ltctx->log = log;
    e = &ltctx->event;

    e->data = ltctx;
    e->log = s->log;
    e->handler = ngx_rtmp_log_trunc_timer;

    t = log->trunc_timer - ngx_current_msec % log->trunc_timer;
    ngx_add_timer(e, t);
}

static ngx_int_t
ngx_rtmp_log_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_rtmp_log_app_conf_t    *lacf;
    ngx_rtmp_log_t             *log;
    ngx_rtmp_log_ctx_t         *ctx;
    ngx_uint_t                  i;

    if (s->interprocess || s->relay) {
        goto next;
    }

    ctx = ngx_rtmp_log_set_names(s, v->name, v->args);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->publish) { /* avoid multi push */
        goto next;
    }

    ctx->publish = 1;

    lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_log_module);

    if (lacf->logs == NULL) {
        goto next;
    }

    log = lacf->logs->elts;
    for (i = 0; i < lacf->logs->nelts; ++i, ++log) {
        if (log->trunc_timer != 0) {
            ngx_rtmp_log_add_trunc_timer(s, ctx, log);
        }
    }

next:
    return next_publish(s, v);
}


static ngx_int_t
ngx_rtmp_log_play(ngx_rtmp_session_t *s, ngx_rtmp_play_t *v)
{
    ngx_rtmp_log_app_conf_t    *lacf;
    ngx_rtmp_log_t             *log;
    ngx_rtmp_log_ctx_t         *ctx;
    ngx_uint_t                  i;

    if (s->interprocess || s->relay) {
        goto next;
    }

    ctx = ngx_rtmp_log_set_names(s, v->name, v->args);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->play) { /* avoid mulit pull */
        goto next;
    }

    ctx->play = 1;

    lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_log_module);

    if (lacf->logs == NULL) {
        goto next;
    }

    log = lacf->logs->elts;
    for (i = 0; i < lacf->logs->nelts; ++i, ++log) {
        if (log->trunc_timer != 0) {
            ngx_rtmp_log_add_trunc_timer(s, ctx, log);
        }
    }

next:
    return next_play(s, v);
}


static ngx_int_t
ngx_rtmp_log_disconnect(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
                        ngx_chain_t *in)
{
    ngx_rtmp_log_app_conf_t    *lacf;
    ngx_rtmp_log_ctx_t         *ctx;
    ngx_rtmp_log_timer_ctx_t   *ltctx;
    ngx_rtmp_log_t             *log;
    ngx_uint_t                  i;

    if (s->interprocess || s->relay) {
        return NGX_OK;
    }

    lacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_log_module);
    if (lacf == NULL || lacf->off || lacf->logs == NULL) {
        return NGX_OK;
    }

    log = lacf->logs->elts;
    for (i = 0; i < lacf->logs->nelts; ++i, ++log) {
        ngx_rtmp_log_pre_write(s, log);
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_log_module);

    if (ctx == NULL) {
        return NGX_OK;
    }

    ltctx = ctx->timers.elts;
    for (i = 0; i < ctx->timers.nelts; ++i, ++ltctx) {
        ngx_del_timer(&ltctx->event);
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_log_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_rtmp_handler_pt        *h;
    ngx_rtmp_log_main_conf_t   *lmcf;
    ngx_array_t                 a;
    ngx_rtmp_log_fmt_t         *fmt;
    ngx_str_t                  *value;

    lmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_log_module);
    if (lmcf->combined_used) {
        if (ngx_array_init(&a, cf->pool, 1, sizeof(ngx_str_t)) != NGX_OK) {
            return NGX_ERROR;
        }

        value = ngx_array_push(&a);
        if (value == NULL) {
            return NGX_ERROR;
        }

        *value = ngx_rtmp_combined_fmt;
        fmt = lmcf->formats.elts;

        if (ngx_rtmp_log_compile_format(cf, fmt->ops, &a, 0)
            != NGX_CONF_OK)
        {
            return NGX_ERROR;
        }
    }

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_DISCONNECT]);
    *h = ngx_rtmp_log_disconnect;

    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_rtmp_log_publish;

    next_play = ngx_rtmp_play;
    ngx_rtmp_play = ngx_rtmp_log_play;

    return NGX_OK;
}
