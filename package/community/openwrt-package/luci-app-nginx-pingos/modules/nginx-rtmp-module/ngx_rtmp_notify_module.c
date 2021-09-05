/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 *
 * Open Capability Live Platform
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_cmd_module.h"
#include "ngx_live_record.h"
#include "ngx_live_relay.h"
#include "ngx_dynamic_resolver.h"
#include "ngx_toolkit_misc.h"
#include "ngx_netcall.h"
#include "ngx_rtmp_variables.h"


static ngx_live_record_start_pt     next_record_start;
static ngx_live_record_update_pt    next_record_update;
static ngx_live_record_done_pt      next_record_done;


static ngx_rtmp_publish_pt          next_publish;
static ngx_rtmp_play_pt             next_play;
static ngx_rtmp_close_stream_pt     next_close_stream;


static ngx_live_push_pt             next_push;
static ngx_live_pull_pt             next_pull;
static ngx_live_push_close_pt       next_push_close;
static ngx_live_pull_close_pt       next_pull_close;


static ngx_int_t ngx_rtmp_notify_init_process(ngx_cycle_t *cycle);

static ngx_int_t ngx_rtmp_notify_postconfiguration(ngx_conf_t *cf);
static void *ngx_rtmp_notify_create_main_conf(ngx_conf_t *cf);
static char *ngx_rtmp_notify_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_rtmp_notify_create_srv_conf(ngx_conf_t *cf);
static char *ngx_rtmp_notify_merge_srv_conf(ngx_conf_t *cf, void *parent,
       void *child);
static void *ngx_rtmp_notify_create_app_conf(ngx_conf_t *cf);
static char *ngx_rtmp_notify_merge_app_conf(ngx_conf_t *cf, void *parent,
       void *child);

static char *ngx_rtmp_notify_on_main_event(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_rtmp_notify_on_srv_event(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_rtmp_notify_on_app_event(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);

#define NGX_RTMP_OCLP_START     0x01
#define NGX_RTMP_OCLP_UPDATE    0x02
#define NGX_RTMP_OCLP_DONE      0x04


enum {
    NGX_RTMP_OCLP_PROC,         /* only notify */
    NGX_RTMP_OCLP_MAIN_MAX
};

enum {
    NGX_RTMP_OCLP_CONNECT,      /* only notify */
    NGX_RTMP_OCLP_SRV_MAX
};

enum {
    NGX_RTMP_OCLP_PLAY,         /* only notify */
    NGX_RTMP_OCLP_PUBLISH,      /* only notify */
    NGX_RTMP_OCLP_PULL,         /* relay pull */
    NGX_RTMP_OCLP_PUSH,         /* relay push */
    NGX_RTMP_OCLP_STREAM,       /* only notify */
    NGX_RTMP_OCLP_META,         /* for transcode */
    NGX_RTMP_OCLP_RECORD,       /* for record */
    NGX_RTMP_OCLP_APP_MAX
};

static char *ngx_rtmp_notify_stage[] = {
    "",
    "start",
    "update",
    "",
    "done",
};

static char *ngx_rtmp_notify_app_type[] = {
    "play",
    "publish",
    "pull",
    "push",
    "stream",
    "meta",
    "record",
};

typedef struct {
    ngx_uint_t                  status;
    char                       *code;
    char                       *level;
    char                       *desc;
} ngx_rtmp_notify_relay_error_t;

static ngx_rtmp_notify_relay_error_t ngx_rtmp_notify_relay_errors[] = {
    { 404, "NetStream.Play.StreamNotFound", "error", "No such stream" },
    { 400, "NetStream.Publish.BadName",     "error", "Already publishing" },
    { 0, NULL, NULL, NULL },
};


#define NGX_RTMP_OCLP_META_VIDEO    0
#define NGX_RTMP_OCLP_META_AUDIO    1
#define NGX_RTMP_OCLP_META_BOTH     2

static ngx_conf_enum_t ngx_rtmp_notify_meta_type[] = {
    { ngx_string("video"),  NGX_RTMP_OCLP_META_VIDEO },
    { ngx_string("audio"),  NGX_RTMP_OCLP_META_AUDIO },
    { ngx_string("both"),   NGX_RTMP_OCLP_META_BOTH  },
    { ngx_null_string,      0 }
};

typedef struct {
    ngx_str_t                   url;
    ngx_str_t                   args;
    ngx_str_t                   groupid;
    ngx_uint_t                  stage;
    ngx_msec_t                  timeout;
    ngx_msec_t                  update;
} ngx_rtmp_notify_event_t;

typedef struct {
    ngx_rtmp_notify_event_t       events[NGX_RTMP_OCLP_MAIN_MAX];
} ngx_rtmp_notify_main_conf_t;

typedef struct {
    ngx_rtmp_notify_event_t       events[NGX_RTMP_OCLP_SRV_MAX];
} ngx_rtmp_notify_srv_conf_t;

typedef struct {
    ngx_flag_t                  ignore_invalid_notify;
    ngx_flag_t                  meta_once;
    ngx_uint_t                  meta_type;
    ngx_array_t                 events[NGX_RTMP_OCLP_APP_MAX];
} ngx_rtmp_notify_app_conf_t;


typedef struct {
    ngx_netcall_ctx_t          *nctx;
    ngx_netcall_ctx_t          *rctx;

    ngx_rtmp_notify_event_t      *event;
    ngx_uint_t                  type;
    ngx_live_relay_t           *relay;

    ngx_rtmp_publish_t          publish_v;
    ngx_rtmp_play_t             play_v;
} ngx_rtmp_notify_ctx_t;


static ngx_command_t ngx_rtmp_notify_commands[] = {

    { ngx_string("ignore_invalid_notify"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_notify_app_conf_t, ignore_invalid_notify),
      NULL },

    { ngx_string("on_proc"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_main_event,
      NGX_RTMP_MAIN_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_connect"),
      NGX_RTMP_SRV_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_srv_event,
      NGX_RTMP_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_play"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_publish"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_pull"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_push"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_stream"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_meta"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_record"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("on_meta_once"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_notify_app_conf_t, meta_once),
      NULL },

    { ngx_string("on_meta_type"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_notify_app_conf_t, meta_type),
      &ngx_rtmp_notify_meta_type },
////////////////////////////////////////////////////////////////////

    { ngx_string("oclp_proc"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_main_event,
      NGX_RTMP_MAIN_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_connect"),
      NGX_RTMP_SRV_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_srv_event,
      NGX_RTMP_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_play"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_publish"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_pull"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_push"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_stream"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_meta"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_record"),
      NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_rtmp_notify_on_app_event,
      NGX_RTMP_APP_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("oclp_meta_once"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_notify_app_conf_t, meta_once),
      NULL },

    { ngx_string("oclp_meta_type"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_1MORE,
      ngx_conf_set_enum_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_notify_app_conf_t, meta_type),
      &ngx_rtmp_notify_meta_type },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_notify_module_ctx = {
    NULL,                                   /* preconfiguration */
    ngx_rtmp_notify_postconfiguration,        /* postconfiguration */
    ngx_rtmp_notify_create_main_conf,         /* create main configuration */
    ngx_rtmp_notify_init_main_conf,           /* init main configuration */
    ngx_rtmp_notify_create_srv_conf,          /* create server configuration */
    ngx_rtmp_notify_merge_srv_conf,           /* merge server configuration */
    ngx_rtmp_notify_create_app_conf,          /* create app configuration */
    ngx_rtmp_notify_merge_app_conf            /* merge app configuration */
};


ngx_module_t  ngx_rtmp_notify_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_notify_module_ctx,              /* module context */
    ngx_rtmp_notify_commands,                 /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    ngx_rtmp_notify_init_process,             /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_rtmp_notify_create_main_conf(ngx_conf_t *cf)
{
    ngx_rtmp_notify_main_conf_t  *omcf;

    omcf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_notify_main_conf_t));
    if (omcf == NULL) {
        return NULL;
    }

    return omcf;
}

static char *
ngx_rtmp_notify_init_main_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static void *
ngx_rtmp_notify_create_srv_conf(ngx_conf_t *cf)
{
    ngx_rtmp_notify_srv_conf_t   *oscf;

    oscf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_notify_srv_conf_t));
    if (oscf == NULL) {
        return NULL;
    }

    return oscf;
}

static char *
ngx_rtmp_notify_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    return NGX_CONF_OK;
}

static void *
ngx_rtmp_notify_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_uint_t                  n;

    oacf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_notify_app_conf_t));
    if (oacf == NULL) {
        return NULL;
    }

    for (n = 0; n < NGX_RTMP_OCLP_APP_MAX; ++n) {
        if (ngx_array_init(&oacf->events[n], cf->pool, NGX_RTMP_MAX_OCLP,
            sizeof(ngx_rtmp_notify_event_t)) == NGX_ERROR)
        {
            return NULL;
        }
    }

    oacf->meta_once = NGX_CONF_UNSET;
    oacf->meta_type = NGX_CONF_UNSET_UINT;
    oacf->ignore_invalid_notify = NGX_CONF_UNSET;

    return oacf;
}

static char *
ngx_rtmp_notify_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_notify_app_conf_t   *prev = parent;
    ngx_rtmp_notify_app_conf_t   *conf = child;

    ngx_conf_merge_value(conf->meta_once, prev->meta_once, 1);
    ngx_conf_merge_uint_value(conf->meta_type, prev->meta_type,
                              NGX_RTMP_OCLP_META_VIDEO);
    ngx_conf_merge_value(conf->ignore_invalid_notify,
        prev->ignore_invalid_notify, 1);

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_rtmp_notify_create_event(ngx_conf_t *cf, ngx_rtmp_notify_event_t *event,
    ngx_str_t *values, ngx_uint_t n)
{
    ngx_request_url_t           ru;
    ngx_str_t                   tmp;
    ngx_uint_t                  i;
    u_char                     *p, *last;

    for (i = 0; i < n; ++i) {
        if (ngx_strncmp(values[i].data, "args=", 5) == 0) {
            tmp = values[i];
            tmp.len -= 5;
            tmp.data += 5;
            ngx_rtmp_variable_transform_index(cf, &tmp, &event->args);
//            event->args.len = values[i].len - 5;
//            event->args.data = values[i].data + 5;
            continue;
        }

        if (ngx_strncmp(values[i].data, "groupid=", 8) == 0) {
            event->groupid.len = values[i].len - 8;
            event->groupid.data = values[i].data + 8;
            continue;
        }

        if (ngx_strncmp(values[i].data, "stage=", 6) == 0) {
            p = values[i].data + 6;
            last = values[i].data + values[i].len;

            while (1) {
                tmp.data = p;
                p = ngx_strlchr(p, last, ',');
                if (p == NULL) {
                    tmp.len = last - tmp.data;
                } else {
                    tmp.len = p - tmp.data;
                }

                switch (tmp.len) {
                case 4:
                    if (ngx_strncmp(tmp.data, "done", 4) == 0) {
                        event->stage |= NGX_RTMP_OCLP_DONE;
                    } else {
                        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                                "unknown stage \"%V\"", &tmp);
                        return NGX_ERROR;
                    }
                    break;
                case 5:
                    if (ngx_strncmp(tmp.data, "start", 5) == 0) {
                        event->stage |= NGX_RTMP_OCLP_START;
                    } else {
                        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                                "unknown stage \"%V\"", &tmp);
                        return NGX_ERROR;
                    }
                    break;
                case 6:
                    if (ngx_strncmp(tmp.data, "update", 6) == 0) {
                        event->stage |= NGX_RTMP_OCLP_UPDATE;
                    } else {
                        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                                "unknown stage \"%V\"", &tmp);
                        return NGX_ERROR;
                    }
                    break;
                default:
                    ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                            "unknown stage \"%V\"", &tmp);
                    return NGX_ERROR;
                }

                if (p == last || p == NULL) {
                    break;
                }

                ++p;
            }

            continue;
        }

        if (ngx_strncmp(values[i].data, "timeout=", 8) == 0) {
            tmp.len = values[i].len - 8;
            tmp.data = values[i].data + 8;

            event->timeout = ngx_parse_time(&tmp, 0);
            if (event->timeout == (ngx_msec_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                        "unknown timeout timer format \"%V\"", &tmp);
                return NGX_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(values[i].data, "update=", 7) == 0) {
            tmp.len = values[i].len - 7;
            tmp.data = values[i].data + 7;

            event->update = ngx_parse_time(&tmp, 0);
            if (event->update == (ngx_msec_t) NGX_ERROR) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                        "unknown cont timer format \"%V\"", &tmp);
                return NGX_ERROR;
            }

            continue;
        }

        if (ngx_strncmp(values[i].data, "http://", 7) != 0) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "must be http url");
            return NGX_ERROR;
        }

        if (event->url.len != 0) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate url");
            return NGX_ERROR;
        }

        event->url.len = values[i].len;
        event->url.data = values[i].data;
        if (values[i].data[values[i].len - 1] != '/') {
            event->url.data = ngx_pcalloc(cf->pool, values[i].len + 1);
            event->url.len = values[i].len + 1;
            ngx_snprintf(event->url.data, event->url.len, "%V/", &values[i]);
        }

        if (ngx_parse_request_url(&ru, &event->url) != NGX_OK) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "request url format error");
            return NGX_ERROR;
        }

        ngx_dynamic_resolver_add_domain(&ru.host, cf->cycle);
    }

    if (event->url.len == 0) {
        return NGX_ERROR;
    } else {
        if (event->timeout == 0) {
            event->timeout = 3000;
        }

        if (event->update == 0) {
            event->update = 60000;
        }

        event->stage |= NGX_RTMP_OCLP_START;

        return NGX_OK;
    }
}

static char *
ngx_rtmp_notify_on_main_event(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_notify_main_conf_t  *omcf;
    ngx_str_t                  *name, *value;
    ngx_uint_t                  n;

    omcf = conf;

    value = cf->args->elts;

    name = &value[0];

    n = 0;

    switch (name->len) {
    case sizeof("on_proc") - 1:
        n = NGX_RTMP_OCLP_PROC;
        break;
    case sizeof("oclp_proc") - 1:
        n = NGX_RTMP_OCLP_PROC;
        break;
    }

    if (ngx_rtmp_notify_create_event(cf, &omcf->events[n], &value[1],
            cf->args->nelts - 1) == NGX_ERROR)
    {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static char *
ngx_rtmp_notify_on_srv_event(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_notify_srv_conf_t   *oscf;
    ngx_str_t                  *name, *value;
    ngx_uint_t                  n;

    oscf = conf;

    value = cf->args->elts;

    name = &value[0];

    n = 0;

    switch (name->len) {
    case sizeof("on_connect") - 1:
        n = NGX_RTMP_OCLP_CONNECT;
        break;
    case sizeof("oclp_connect") - 1:
        n = NGX_RTMP_OCLP_CONNECT;
        break;
    }

    if (ngx_rtmp_notify_create_event(cf, &oscf->events[n], &value[1],
            cf->args->nelts - 1) == NGX_ERROR)
    {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}

static char *
ngx_rtmp_notify_on_app_event(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_str_t                  *name, *value;
    ngx_uint_t                  n;

    oacf = conf;

    value = cf->args->elts;

    name = &value[0];

    n = 0;

    if (name->data[1] == 'c') {
        switch (name->len) {
        case sizeof("oclp_play") - 1:
            if (name->data[8] == 'y') { /* oclp_play */
                n = NGX_RTMP_OCLP_PLAY;
                if (oacf->events[n].nelts != 0) {
                    ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                    return NGX_CONF_ERROR;
                }
            } else if (name->data[8] == 'l') { /* oclp_pull */
                n = NGX_RTMP_OCLP_PULL;
                if (oacf->events[n].nelts != 0) {
                    ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                    return NGX_CONF_ERROR;
                }
            } else if (name->data[8] == 'h') { /* oclp_push */
                n = NGX_RTMP_OCLP_PUSH;
            } else if (name->data[8] == 'a') { /* oclp_meta */
                n = NGX_RTMP_OCLP_META;
            }
            break;
        case sizeof("oclp_publish") - 1:
            n = NGX_RTMP_OCLP_PUBLISH;
            if (oacf->events[n].nelts != 0) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                return NGX_CONF_ERROR;
            }
            break;
        case sizeof("oclp_stream") - 1:
            if (name->data[5] == 's') { /* oclp_stream */
                n = NGX_RTMP_OCLP_STREAM;
            } else if (name->data[5] == 'r') { /* oclp_record */
                n = NGX_RTMP_OCLP_RECORD;
            }

            if (oacf->events[n].nelts != 0) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                return NGX_CONF_ERROR;
            }
            break;
        }
    } else {
        switch (name->len) {
        case sizeof("on_play") - 1:
            if (name->data[6] == 'y') { /* on_play */
                n = NGX_RTMP_OCLP_PLAY;
                if (oacf->events[n].nelts != 0) {
                    ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                    return NGX_CONF_ERROR;
                }
            } else if (name->data[6] == 'l') { /* on_pull */
                n = NGX_RTMP_OCLP_PULL;
                if (oacf->events[n].nelts != 0) {
                    ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                    return NGX_CONF_ERROR;
                }
            } else if (name->data[6] == 'h') { /* on_push */
                n = NGX_RTMP_OCLP_PUSH;
            } else if (name->data[6] == 'a') { /* on_meta */
                n = NGX_RTMP_OCLP_META;
            }
            break;
        case sizeof("on_publish") - 1:
            n = NGX_RTMP_OCLP_PUBLISH;
            if (oacf->events[n].nelts != 0) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                return NGX_CONF_ERROR;
            }
            break;
        case sizeof("on_stream") - 1:
            if (name->data[3] == 's') { /* on_stream */
                n = NGX_RTMP_OCLP_STREAM;
            } else if (name->data[3] == 'r') { /* on_record */
                n = NGX_RTMP_OCLP_RECORD;
            }

            if (oacf->events[n].nelts != 0) {
                ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "duplicate %V", name);
                return NGX_CONF_ERROR;
            }
            break;
        }
    }


    if (oacf->events[n].nelts >= NGX_RTMP_MAX_OCLP) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0, "too much %V events", name);
        return NGX_CONF_ERROR;
    }

    event = ngx_array_push(&oacf->events[n]);
    ngx_memzero(event, sizeof(ngx_rtmp_notify_event_t));
    if (ngx_rtmp_notify_create_event(cf, event, &value[1], cf->args->nelts - 1)
            == NGX_ERROR)
    {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static void
ngx_rtmp_notify_init_process_url(ngx_str_t *url, ngx_pool_t *pool,
    ngx_rtmp_notify_event_t *event)
{
    size_t                      len;
    u_char                     *p;

    len = event->url.len + sizeof("?call=init_process&worker_id=") - 1
        + sizeof("256") - 1;

    url->data = ngx_pcalloc(pool, len);
    if (url->data == NULL) {
        return;
    }

    p = url->data;
    p = ngx_snprintf(p, len, "%V?call=init_process&worker_id=%ui",
            &event->url, ngx_worker);
    url->len = p - url->data;
}

static void
ngx_rtmp_notify_init_process_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    if (code != NGX_HTTP_OK) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "notify init process notify error: %i", code);
    }

    return;
}

static void
ngx_rtmp_notify_init_process_create(ngx_event_t *ev)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = ev->data;

    ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0, "notify init process create");

    ngx_netcall_create(nctx, ngx_cycle->log);
}

static ngx_int_t
ngx_rtmp_notify_init_process(ngx_cycle_t *cycle)
{
    ngx_rtmp_notify_main_conf_t  *omcf;
    ngx_rtmp_notify_event_t      *event;
    ngx_rtmp_conf_ctx_t        *ctx;
    ngx_netcall_ctx_t          *nctx;
    ngx_event_t                *ev;

    if (ngx_process != NGX_PROCESS_WORKER &&
        ngx_process != NGX_PROCESS_SINGLE)
    {
        return NGX_OK;
    }

    if (ngx_rtmp_core_main_conf == NULL) {
        return NGX_OK;
    }

    ctx = (ngx_rtmp_conf_ctx_t *) ngx_get_conf(cycle->conf_ctx,
                                               ngx_rtmp_module);
    omcf = (ngx_rtmp_notify_main_conf_t *)
            ctx->main_conf[ngx_rtmp_notify_module.ctx_index];

    if (omcf->events[NGX_RTMP_OCLP_PROC].url.len == 0) {
        return NGX_OK;
    }

    event = &omcf->events[NGX_RTMP_OCLP_PROC];

    nctx = ngx_netcall_create_ctx(NGX_RTMP_OCLP_PROC, &event->groupid,
            event->stage, event->timeout, event->update, 0);
    if (nctx == NULL) {
        return NGX_ERROR;
    }

    ngx_rtmp_notify_init_process_url(&nctx->url, nctx->pool, event);
    nctx->handler = ngx_rtmp_notify_init_process_handle;
    nctx->data = nctx;

    ev = &nctx->ev;
    ev->handler = ngx_rtmp_notify_init_process_create;

    ngx_post_event(ev, &ngx_rtmp_init_queue);

    return NGX_OK;
}

static void
ngx_rtmp_notify_common_url(ngx_str_t *url, ngx_rtmp_session_t *s,
    ngx_rtmp_notify_event_t *event, ngx_netcall_ctx_t *nctx, ngx_uint_t stage)
{
    ngx_request_url_t           ru;
    size_t                      len;
    u_char                     *p, *buf;
    ngx_live_record_ctx_t      *lrctx;
    unsigned                    fill = 0;
    ngx_str_t                   args;

    ngx_memzero(&ru, sizeof(ngx_request_url_t));
    ngx_parse_request_url(&ru, &event->url);

    len = NGX_NETCALL_MAX_URL_LEN;
    buf = url->data;
    p = ngx_snprintf(buf, len , "%V", &event->url);
    if (fill) {
        *p++ = '/';
    }
    len -= p - buf;
    buf = p;

    if (ru.args.len) { // url already has args
        p = ngx_snprintf(buf, len,
                "&call=%s&act=%s&domain=%V&app=%V&name=%V&clientid=%D",
                ngx_rtmp_notify_app_type[nctx->type],
                ngx_rtmp_notify_stage[stage],
                &s->domain, &s->app, &s->name, s->number);
    } else {
        p = ngx_snprintf(buf, len,
                "?call=%s&act=%s&domain=%V&app=%V&name=%V&clientid=%D",
                ngx_rtmp_notify_app_type[nctx->type],
                ngx_rtmp_notify_stage[stage],
                &s->domain, &s->app, &s->name, s->number);
    }
    len -= p - buf;
    buf = p;

    if (nctx->type == NGX_RTMP_OCLP_RECORD && stage != NGX_RTMP_OCLP_START) {
        lrctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);
        p = ngx_snprintf(buf, len, "&begintime=%M&endtime=%M&index=%V&file=%V",
                lrctx->begintime, lrctx->endtime, &lrctx->index.name,
                &lrctx->file.name);

        len -= p - buf;
        buf = p;
    }

    if (event->groupid.len) {
        p = ngx_snprintf(buf, len, "&groupid=%V", &event->groupid);
        len -= p - buf;
        buf = p;
    }

    if (event->args.len) {
        ngx_rtmp_fetch_variable(s, s->pool, &event->args, &args);
        if (args.len > 0) {
            p = ngx_snprintf(buf, len, "&%V", &args);
            len -= p - buf;
            buf = p;
        }
    }

    url->len = p - url->data;
}

static void
ngx_rtmp_notify_common_timer(ngx_event_t *ev)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = ev->data;

    ngx_log_error(NGX_LOG_INFO, ev->log, 0, "notify %s update create %V",
            ngx_rtmp_notify_app_type[nctx->type], &nctx->url);

    ngx_netcall_create(nctx, ev->log);
}

static void
ngx_rtmp_notify_common_update_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_event_t                *ev;

    ev = &nctx->ev;
    ev->handler = ngx_rtmp_notify_common_timer;

    if (code != NGX_HTTP_OK) {
        ngx_log_error(NGX_LOG_ERR, ev->log, 0,
                "notify %s update notify error: %i",
                ngx_rtmp_notify_app_type[nctx->type], code);
    }

    ngx_add_timer(ev, nctx->update);
}

static void
ngx_rtmp_notify_common_update_create(ngx_rtmp_session_t *s,
    ngx_netcall_ctx_t *nctx)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_event_t                *ev;

    if ((nctx->stage & NGX_RTMP_OCLP_UPDATE) == NGX_RTMP_OCLP_UPDATE) {
        oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

        event = oacf->events[nctx->type].elts;
        event += nctx->idx;

        ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                 NGX_RTMP_OCLP_UPDATE);
        nctx->handler = ngx_rtmp_notify_common_update_handle;

        ev = &nctx->ev;
        ev->data = nctx;
        ev->handler = ngx_rtmp_notify_common_timer;

        ngx_add_timer(ev, nctx->update);
    }
}

static void
ngx_rtmp_notify_common_done(ngx_rtmp_session_t *s, ngx_netcall_ctx_t *nctx)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;

    if (nctx == NULL) {
        return;
    }

    if ((nctx->stage & NGX_RTMP_OCLP_DONE) == NGX_RTMP_OCLP_DONE) {
        oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);
        event = oacf->events[nctx->type].elts;

        ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                 NGX_RTMP_OCLP_DONE);

        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "notify %s done create %V %p",
                ngx_rtmp_notify_app_type[nctx->type], &nctx->url, nctx);

        ngx_netcall_create(nctx, s->log);
    }

    ngx_netcall_destroy(nctx);
}

static void
ngx_rtmp_notify_pnotify_start_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_rtmp_session_t           *s;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_rtmp_notify_app_conf_t   *oacf;

    s = nctx->data;

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

    s->notify_status = code;

    if (code < NGX_HTTP_OK || code > NGX_HTTP_SPECIAL_RESPONSE) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify %s start notify error: %i",
               ngx_rtmp_notify_app_type[nctx->type], code);

        if (code != -1 || !oacf->ignore_invalid_notify) {
            goto error;
        }

        goto next;
    }

next:
    if (octx->type == NGX_RTMP_OCLP_PUBLISH) {
        if (next_publish(s, &octx->publish_v) != NGX_OK) {
            goto error;
        }
    } else {
        if (next_play(s, &octx->play_v) != NGX_OK) {
            goto error;
        }
    }

    ngx_rtmp_notify_common_update_create(s, nctx);

    return;

error:
    if (octx->type == NGX_RTMP_OCLP_PUBLISH) {
        ngx_rtmp_send_status(s, "NetStream.Publish.Forbidden", "status",
                "Publish stream Forbidden");
    } else if (s->live_type == NGX_RTMP_LIVE) {
        s->status = 403;
        ngx_rtmp_send_status(s, "NetStream.Play.Forbidden", "status",
                "Play stream Forbidden");
    }
    s->finalize_reason = NGX_LIVE_OCLP_NOTIFY_ERR;
    ngx_rtmp_finalize_session(s);
}

static ngx_int_t
ngx_rtmp_notify_pnotify_start(ngx_rtmp_session_t *s, ngx_uint_t type)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_rtmp_notify_ctx_t        *ctx;
    ngx_netcall_ctx_t          *nctx;

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

    if (oacf->events[type].nelts == 0) {
        return NGX_DECLINED;
    }

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    if (ctx == NULL) {
        ctx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_notify_ctx_t));
        if (ctx == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0, "palloc notify ctx failed");
            return NGX_ERROR;
        }
        ngx_rtmp_set_ctx(s, ctx, ngx_rtmp_notify_module);
    }

    event = oacf->events[type].elts;

    nctx = ngx_netcall_create_ctx(type, &event->groupid,
            event->stage, event->timeout, event->update, 0);
    nctx->ev.log = s->log;

    ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                NGX_RTMP_OCLP_START);
    nctx->handler = ngx_rtmp_notify_pnotify_start_handle;
    nctx->data = s;

    ctx->nctx = nctx;
    ctx->type = type;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "notify %s start create %V",
            ngx_rtmp_notify_app_type[nctx->type], &nctx->url);

    ngx_netcall_create(nctx, s->log);

    return NGX_OK;
}

static void
ngx_rtmp_notify_pnotify_done(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_ctx_t        *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    if (ctx == NULL) {
        return;
    }

    ngx_rtmp_notify_common_done(s, ctx->nctx);
}

static void
ngx_rtmp_notify_relay_error(ngx_rtmp_session_t *s, ngx_uint_t status)
{
    ngx_rtmp_core_ctx_t        *cctx;
    size_t                      i;

    for (i = 0; ngx_rtmp_notify_relay_errors[i].status; ++i) {
        if (status != ngx_rtmp_notify_relay_errors[i].status) {
            continue;
        }

        if (s->publishing) { // relay pull
            cctx = s->live_stream->play_ctx;
        } else { // relay push
            cctx = s->live_stream->publish_ctx;
        }

        for (; cctx; cctx = cctx->next) {
            if (!cctx->session->static_pull) {
                cctx->session->status = status;
                cctx->session->finalize_reason = NGX_LIVE_RELAY_TRANSIT;
                ngx_rtmp_finalize_session(cctx->session);
            }
        }
    }
}

static void
ngx_rtmp_notify_relay_start_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_rtmp_session_t         *s;
    ngx_live_relay_t            relay;
    ngx_live_relay_url_t       *url;
    ngx_str_t                  *local_name;
    ngx_str_t                  *local_domain;
    u_char                     *p, *last;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_live_relay_ctx_t       *ctx;

    static ngx_str_t            location = ngx_string("location");
    static ngx_str_t            domain = ngx_string("domain");

    s = nctx->data;
    // relay pull, no player or relay push no publisher
    if ((s->publishing && s->live_stream->play_ctx == NULL)
            || (!s->publishing && s->live_stream->publish_ctx == NULL))
    {
        return;
    }

    s->notify_status = code;

    if (code == -1) { // wait for notify relay reconnect
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "notify relay start failed");

        s->finalize_reason = NGX_LIVE_OCLP_RELAY_ERR;
        ngx_rtmp_finalize_session(s); // only reconnect immediately

        return;
    }

    if (code >= 400) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "notify: relay_start_handle| "
                "notify error: %i", code);

        if (s->publishing) { // relay pull
            ngx_rtmp_notify_relay_error(s, 404);
        } else { // relay push
            ngx_rtmp_notify_relay_error(s, 400);
        }
        s->finalize_reason = NGX_LIVE_OCLP_RELAY_ERR;
        ngx_rtmp_finalize_session(s);

        return;
    }

    if (code == NGX_HTTP_OK) { // successd but no need to relay
        ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
        ctx->giveup = 1;
        s->finalize_reason = NGX_LIVE_NORMAL_CLOSE;
        ngx_rtmp_finalize_session(s);
        return;
    }

    /* redirect */
    local_name = ngx_netcall_header(nctx, &location);
    if (local_name == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start has no Location when redirect");
        s->finalize_reason = NGX_LIVE_OCLP_PARA_ERR;
        ngx_rtmp_finalize_session(s);

        return;
    }

    ngx_memzero(&relay, sizeof(ngx_live_relay_t));
    if (ngx_array_init(&relay.urls, s->pool, 1, sizeof(ngx_live_relay_url_t))
            != NGX_OK)
    {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start, init relay urls failed");
        ngx_rtmp_finalize_session(s);

        return;
    }

    url = ngx_array_push(&relay.urls);
    if (url == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start, alloc url from relay urls failed");
        ngx_rtmp_finalize_session(s);

        return;
    }
    ngx_memzero(url, sizeof(ngx_live_relay_url_t));
    relay.tag = &ngx_rtmp_notify_module;

    if (ngx_parse_request_url(&url->url, local_name) != NGX_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start, request url format error: %V", &location);
        s->finalize_reason = NGX_LIVE_OCLP_PARA_ERR;
        ngx_rtmp_finalize_session(s);

        return;
    }

    if (ngx_strncasecmp(url->url.scheme.data, (u_char *) "rtmp://", 7) == 0) {
        url->relay_type = NGX_LIVE_RELAY_RTMP;
    }

    url->port = ngx_request_port(&url->url.scheme, &url->url.port);
    if (url->port == 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start, request url port error");
        s->finalize_reason = NGX_LIVE_OCLP_PARA_ERR;
        ngx_rtmp_finalize_session(s);

        return;
    }

    // set domain
    local_domain = ngx_netcall_header(nctx, &domain);
    if (local_domain && local_domain->len) {
        relay.domain = *local_domain;
    }

    if (url->url.path.len == 0) { // no app and stream
        goto relay_create;
    }

    p = url->url.path.data;
    last = url->url.path.data + url->url.path.len;

    // set app
    relay.app.data = p;
    p = ngx_strlchr(p, last, '/');
    if (p == NULL) { // only has app
        relay.app.len = last - relay.app.data;
        goto relay_create;
    } else {
        relay.app.len = p - relay.app.data;
    }

    ++p;
    if (p == last) {
        goto relay_create;
    }

    // set stream name
    relay.name.data = p;
    relay.name.len = last - relay.name.data;

    // set args
    if (url->url.args.len) {
        relay.pargs = url->url.args;
    }

relay_create:
    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    ngx_rtmp_notify_common_update_create(s, octx->nctx);

    ngx_live_relay_create(s, &relay);

    return;
}

static void
ngx_rtmp_notify_relay_handler(ngx_event_t *ev)
{
    ngx_rtmp_session_t         *s;
    ngx_live_relay_app_conf_t  *lracf;
    ngx_live_relay_ctx_t       *ctx;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_rtmp_notify_event_t      *event;
    ngx_netcall_ctx_t          *nctx;

    s = ev->data;
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    event = octx->event;

    if (!ctx->failed_delay && ev->timedout) { // connect timeout
        ngx_log_error(NGX_LOG_ERR, s->log, NGX_ETIMEDOUT,
                "notify relay, relay timeout");
        s->finalize_reason = NGX_LIVE_RELAY_TIMEOUT;
        ngx_rtmp_finalize_session(s);

        if (octx->nctx->hcr) {
            ngx_http_client_detach(octx->nctx->hcr);
            octx->nctx->hcr = NULL;

            if (octx->nctx->ev.timer_set) {
                ngx_del_timer(&octx->nctx->ev);
            }

            if (octx->nctx->ev.posted) {
                ngx_delete_posted_event(&octx->nctx->ev);
            }
        }

        return;
    }

    // relay pull, no player or relay push no publisher
    if ((s->publishing && s->live_stream->play_ctx == NULL)
            || (!s->publishing && s->live_stream->publish_ctx == NULL))
    {
        return;
    }

    lracf = ngx_rtmp_get_module_app_conf(s, ngx_live_relay_module);

    // this timer control start netcall until relay create success
    ngx_add_timer(&ctx->reconnect, lracf->relay_reconnect);

    nctx = ngx_netcall_create_ctx(octx->type, &event->groupid, event->stage,
            event->timeout, event->update, 0);
    if (nctx == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "notify relay, create nctx failed");
        ngx_rtmp_finalize_session(s);

        return;
    }
    nctx->ev.log = s->log;

    ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
            NGX_RTMP_OCLP_START);
    nctx->handler = ngx_rtmp_notify_relay_start_handle;
    nctx->data = s;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "notify %s start create %V",
            ngx_rtmp_notify_app_type[nctx->type], &nctx->url);

    octx->nctx = nctx;

    ngx_netcall_create(nctx, s->log);
}

static ngx_int_t
ngx_rtmp_notify_relay_start(ngx_rtmp_session_t *s, ngx_rtmp_notify_event_t *event,
    ngx_uint_t type, unsigned publishing)
{
    ngx_rtmp_session_t         *rs;
    ngx_live_relay_ctx_t       *ctx, *pctx;
    ngx_live_relay_app_conf_t  *lracf;
    ngx_rtmp_notify_ctx_t        *octx;

    rs = ngx_rtmp_create_relay_session(s, &ngx_rtmp_notify_module);
    if (rs == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify relay start, create relay session failed");
        return NGX_DECLINED;
    }
    rs->publishing = publishing;
    rs->groupid = event->groupid;
    rs->live_stream = s->live_stream;
    ngx_live_create_ctx(rs, publishing);

    octx = ngx_pcalloc(rs->pool, sizeof(ngx_rtmp_notify_ctx_t));
    if (octx == NULL) {
        ngx_log_error(NGX_LOG_ERR, rs->log, 0,
                "notify relay start, create notify ctx failed");
        ngx_rtmp_finalize_session(rs);

        return NGX_OK;
    }
    ngx_rtmp_set_ctx(rs, octx, ngx_rtmp_notify_module);
    octx->event = event;
    octx->type = type;

    ctx = ngx_rtmp_get_module_ctx(rs, ngx_live_relay_module);
    ctx->reconnect.log = rs->log;
    ctx->reconnect.data = rs;
    ctx->reconnect.handler = ngx_rtmp_notify_relay_handler;

    if (s->publishing != rs->publishing) {
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    // normal publisher close, need to trigger pull
    if (s->publishing && !s->relay) {
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    // reconnect
    pctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (pctx->successd) { // prev relay successd
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    ctx->idx = pctx->idx;
    ctx->failed_reconnect = pctx->failed_reconnect;

    lracf = ngx_rtmp_get_module_app_conf(rs, ngx_live_relay_module);

    if (!pctx->reconnect.timer_set) { // prev relay timeout
        ctx->failed_reconnect = ngx_min(pctx->failed_reconnect * 2,
                lracf->relay_reconnect);
        ngx_post_event(&ctx->reconnect, &ngx_posted_events);
        return NGX_OK;
    }

    if (pctx->failed_reconnect) {
        ctx->failed_reconnect = ngx_min(pctx->failed_reconnect * 2,
                lracf->relay_reconnect);
    } else {
        ctx->failed_reconnect = lracf->failed_reconnect;
    }

    ctx->failed_delay = 1;
    ngx_add_timer(&ctx->reconnect, ctx->failed_reconnect);

    return NGX_OK;
}

static void
ngx_rtmp_notify_relay_done(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_netcall_ctx_t          *nctx;

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    nctx = octx->nctx;
    ngx_rtmp_notify_common_done(s, nctx);
}

static void
ngx_rtmp_notify_stream_start_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_live_stream_t          *st;
    ngx_rtmp_session_t         *s;

    st = nctx->data;

    if (code != NGX_HTTP_OK) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "notify stream start notify error: %i", code);

        return;
    }

    if (st->play_ctx) {
        s = st->play_ctx->session;
    } else if (st->publish_ctx) {
        s = st->publish_ctx->session;
    } else {
        ngx_log_error(NGX_LOG_INFO, ngx_cycle->log, 0,
                "notify stream start handle but all stream session close");
        return;
    }

    ngx_rtmp_notify_common_update_create(s, nctx);
}

void
ngx_rtmp_notify_stream_start(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_netcall_ctx_t          *nctx;

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

    if (oacf->events[NGX_RTMP_OCLP_STREAM].nelts == 0) {
        return;
    }

    nctx = s->live_stream->stream_nctx;
    if (nctx) { /* stream start has been sent */
        return;
    }

    event = oacf->events[NGX_RTMP_OCLP_STREAM].elts;

    if ((event->stage & NGX_RTMP_OCLP_START) == NGX_RTMP_OCLP_START) {
        nctx = ngx_netcall_create_ctx(NGX_RTMP_OCLP_STREAM, &event->groupid,
                event->stage, event->timeout, event->update, 0);

        ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                 NGX_RTMP_OCLP_START);
        nctx->handler = ngx_rtmp_notify_stream_start_handle;
        nctx->data = s->live_stream;

        s->live_stream->stream_nctx = nctx;

        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "notify stream start create %V", &nctx->url);

        ngx_netcall_create(nctx, ngx_cycle->log);
    }
}

void
ngx_rtmp_notify_stream_done(ngx_rtmp_session_t *s)
{
    ngx_netcall_ctx_t          *nctx;

    nctx = s->live_stream->stream_nctx;
    ngx_rtmp_notify_common_done(s, nctx);
    s->live_stream->stream_nctx = NULL;
}

static void
ngx_rtmp_notify_record_start_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_rtmp_session_t         *s;
    ngx_live_record_ctx_t      *ctx;

    s = nctx->data;
    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_record_module);

    if (code == NGX_HTTP_OK) {
        ctx->open = 1;
    } else if (code != -1) {
        ctx->open = 0;
    }

    ngx_log_error(NGX_LOG_INFO, s->log, 0,
            "notify record receive code %i, open: %d", code, ctx->open);

    if (next_record_start(s) != NGX_OK) {
        ngx_rtmp_finalize_session(s);
    }
}

static ngx_int_t
ngx_rtmp_notify_record_start(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_netcall_ctx_t          *nctx;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_rtmp_notify_event_t      *event;

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

    if (oacf->events[NGX_RTMP_OCLP_RECORD].nelts == 0) {
        goto next;
    }

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    if (octx == NULL) {
        octx = ngx_pcalloc(s->pool, sizeof(ngx_rtmp_notify_ctx_t));
        if (octx == NULL) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0, "pcalloc notify ctx failed");
            goto next;
        }
        ngx_rtmp_set_ctx(s, octx, ngx_rtmp_notify_module);
    }

    event = oacf->events[NGX_RTMP_OCLP_RECORD].elts;

    nctx = ngx_netcall_create_ctx(NGX_RTMP_OCLP_RECORD, &event->groupid,
            event->stage, event->timeout, event->update, 0);

    ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                             NGX_RTMP_OCLP_START);
    nctx->handler = ngx_rtmp_notify_record_start_handle;
    nctx->data = s;

    octx->rctx = nctx;
    octx->type = NGX_RTMP_OCLP_RECORD;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "notify record start create %V",
            &nctx->url);

    ngx_netcall_create(nctx, s->log);

    return NGX_OK;

next:
    return next_record_start(s);
}

static void
ngx_rtmp_notify_record_update_handle(ngx_netcall_ctx_t *nctx, ngx_int_t code)
{
    ngx_rtmp_session_t         *s;

    s = nctx->data;

    if (code != NGX_HTTP_OK) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "notify record update notify error: %i", code);
    }
}

static ngx_int_t
ngx_rtmp_notify_record_update(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_netcall_ctx_t          *nctx;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_rtmp_notify_event_t      *event;

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    if (octx == NULL || octx->rctx == NULL) {
        goto next;
    }

    nctx = octx->rctx;

    if ((nctx->stage & NGX_RTMP_OCLP_UPDATE) == NGX_RTMP_OCLP_UPDATE) {
        oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);
        event = oacf->events[nctx->type].elts;

        ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                 NGX_RTMP_OCLP_UPDATE);
        nctx->handler = ngx_rtmp_notify_record_update_handle;

        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "notify record update create %V %p", &nctx->url, nctx);

        ngx_netcall_create(nctx, s->log);
    }

next:
    return next_record_update(s);
}

static ngx_int_t
ngx_rtmp_notify_record_done(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_netcall_ctx_t          *nctx;
    ngx_rtmp_notify_ctx_t        *octx;
    ngx_rtmp_notify_event_t      *event;

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    if (octx == NULL || octx->rctx == NULL) {
        goto next;
    }

    nctx = octx->rctx;

    if ((nctx->stage & NGX_RTMP_OCLP_DONE) == NGX_RTMP_OCLP_DONE) {
        oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);
        event = oacf->events[nctx->type].elts;

        ngx_rtmp_notify_common_url(&nctx->url, s, event, nctx,
                                 NGX_RTMP_OCLP_DONE);

        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "notify record done create %V %p", &nctx->url, nctx);

        ngx_netcall_create(nctx, s->log);
    }

    ngx_netcall_destroy(nctx);

next:
    return next_record_done(s);
}

static ngx_int_t
ngx_rtmp_notify_push(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_uint_t                  i;

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);
    if (oacf->events[NGX_RTMP_OCLP_PUSH].nelts == 0) { // not configured
        goto next;
    }

    if (s->relay && s->static_pull == 0) {
        goto next;
    }

    event = oacf->events[NGX_RTMP_OCLP_PUSH].elts;
    for (i = 0; i < oacf->events[NGX_RTMP_OCLP_PUSH].nelts; ++i, ++event) {
        ngx_rtmp_notify_relay_start(s, event, NGX_RTMP_OCLP_PUSH, 0);
    }

next:
    return next_push(s);
}

static ngx_int_t
ngx_rtmp_notify_pull(ngx_rtmp_session_t *s)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;

    oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);
    if (oacf->events[NGX_RTMP_OCLP_PULL].nelts == 0) { // not configured
        goto next;
    }

    if (!s->publishing && s->relay) { // relay push
        goto next;
    }

    if (s->live_stream->publish_ctx) { // already has publisher
        goto next;
    }

    event = oacf->events[NGX_RTMP_OCLP_PULL].elts;

    if (ngx_rtmp_notify_relay_start(s, event, NGX_RTMP_OCLP_PULL, 1) == NGX_OK) {
        return NGX_OK;
    }

next:
    return next_pull(s);
}

static ngx_int_t
ngx_rtmp_notify_push_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;
    ngx_rtmp_notify_ctx_t        *octx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx->tag != &ngx_rtmp_notify_module) {
        goto next;
    }

    ngx_rtmp_notify_relay_done(s); // nctx will close in relay done

    // relay push and has publisher in stream
    if (!ctx->giveup && s->live_stream->publish_ctx) {
        octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);

        ngx_rtmp_notify_relay_start(s, octx->event, octx->type, 0);
    }

next:
    return next_push_close(s);
}

static ngx_int_t
ngx_rtmp_notify_pull_close(ngx_rtmp_session_t *s)
{
    ngx_live_relay_ctx_t       *ctx;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (ctx == NULL) {
        goto next;
    }

    if (ctx->tag != &ngx_rtmp_notify_module) {
        goto next;
    }

    ngx_rtmp_notify_relay_done(s); // nctx will close in relay done

next:
    return next_pull_close(s);
}

static ngx_int_t
ngx_rtmp_notify_av(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_rtmp_notify_app_conf_t   *oacf;
    ngx_rtmp_notify_event_t      *event;
    ngx_uint_t                  i;

    if (ngx_rtmp_is_codec_header(in)) {
        oacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_notify_module);

        if (oacf->meta_once && s->live_stream->notify_meta) {
            return NGX_OK;
        }

        switch (oacf->meta_type) {
        case NGX_RTMP_OCLP_META_VIDEO:
            if (h->type == NGX_RTMP_MSG_AUDIO) {
                return NGX_OK;
            }

            break;
        case NGX_RTMP_OCLP_META_AUDIO:
            if (h->type == NGX_RTMP_MSG_VIDEO) {
                return NGX_OK;
            }

            break;
        default:
            break;
        }

        s->live_stream->notify_meta = 1;

        event = oacf->events[NGX_RTMP_OCLP_META].elts;
        for (i = 0; i < oacf->events[NGX_RTMP_OCLP_META].nelts; ++i, ++event) {
            ngx_rtmp_notify_relay_start(s, event, NGX_RTMP_OCLP_META, 0);
        }
    }

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_notify_publish(ngx_rtmp_session_t *s, ngx_rtmp_publish_t *v)
{
    ngx_rtmp_notify_ctx_t        *octx;

    if (s->relay || s->interprocess) {
        goto next;
    }

    switch (ngx_rtmp_notify_pnotify_start(s, NGX_RTMP_OCLP_PUBLISH)) {
    case NGX_OK:
        break;
    case NGX_DECLINED:
        goto next;
    default:
        return NGX_ERROR;
    }

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    ngx_memcpy(&octx->publish_v, v, sizeof(ngx_rtmp_publish_t));

    return NGX_OK;

next:
    return next_publish(s, v);
}

static ngx_int_t
ngx_rtmp_notify_play(ngx_rtmp_session_t *s, ngx_rtmp_play_t *v)
{
    ngx_rtmp_notify_ctx_t        *octx;

    if (s->relay || s->interprocess) {
        goto next;
    }

    switch (ngx_rtmp_notify_pnotify_start(s, NGX_RTMP_OCLP_PLAY)) {
    case NGX_OK:
        break;
    case NGX_DECLINED:
        goto next;
    default:
        return NGX_ERROR;
    }

    octx = ngx_rtmp_get_module_ctx(s, ngx_rtmp_notify_module);
    ngx_memcpy(&octx->play_v, v, sizeof(ngx_rtmp_play_t));

    return NGX_OK;

next:
    return next_play(s, v);
}

static ngx_int_t
ngx_rtmp_notify_close_stream(ngx_rtmp_session_t *s, ngx_rtmp_close_stream_t *v)
{
    if (s->relay || s->interprocess) {
        goto next;
    }

    ngx_rtmp_notify_pnotify_done(s);

next:
    return next_close_stream(s, v);
}

static ngx_int_t
ngx_rtmp_notify_postconfiguration(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_rtmp_handler_pt        *h;

    cmcf = ngx_rtmp_conf_get_module_main_conf(cf, ngx_rtmp_core_module);

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_AUDIO]);
    *h = ngx_rtmp_notify_av;

    h = ngx_array_push(&cmcf->events[NGX_RTMP_MSG_VIDEO]);
    *h = ngx_rtmp_notify_av;

    /* record */
    next_record_start = ngx_live_record_start;
    ngx_live_record_start = ngx_rtmp_notify_record_start;

    next_record_update = ngx_live_record_update;
    ngx_live_record_update = ngx_rtmp_notify_record_update;

    next_record_done = ngx_live_record_done;
    ngx_live_record_done = ngx_rtmp_notify_record_done;

    /* pull & push */
    next_push = ngx_live_push;
    ngx_live_push = ngx_rtmp_notify_push;

    next_pull = ngx_live_pull;
    ngx_live_pull = ngx_rtmp_notify_pull;

    next_push_close = ngx_live_push_close;
    ngx_live_push_close = ngx_rtmp_notify_push_close;

    next_pull_close = ngx_live_pull_close;
    ngx_live_pull_close = ngx_rtmp_notify_pull_close;

    /* publish & play */
    next_publish = ngx_rtmp_publish;
    ngx_rtmp_publish = ngx_rtmp_notify_publish;

    next_play = ngx_rtmp_play;
    ngx_rtmp_play = ngx_rtmp_notify_play;

    next_close_stream = ngx_rtmp_close_stream;
    ngx_rtmp_close_stream = ngx_rtmp_notify_close_stream;

    return NGX_OK;
}
