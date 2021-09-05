
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_event.h>
#include <nginx.h>
#include "ngx_rtmp.h"
#include "ngx_dynamic_conf.h"
#include "ngx_rtmp_dynamic.h"
#include "ngx_rtmp_variables.h"


static ngx_int_t ngx_rtmp_core_preconfiguration(ngx_conf_t *cf);

static void *ngx_rtmp_core_create_main_conf(ngx_conf_t *cf);
static char *ngx_rtmp_core_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_rtmp_core_create_srv_conf(ngx_conf_t *cf);
static char *ngx_rtmp_core_merge_srv_conf(ngx_conf_t *cf, void *parent,
    void *child);
static void *ngx_rtmp_core_create_app_conf(ngx_conf_t *cf);
static char *ngx_rtmp_core_merge_app_conf(ngx_conf_t *cf, void *parent,
    void *child);
static char *ngx_rtmp_core_server(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_rtmp_core_listen(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_rtmp_core_server_name(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);
static char *ngx_rtmp_core_application(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);

static void *ngx_rtmp_core_create_srv_dconf(ngx_conf_t *cf);
static char *ngx_rtmp_core_init_srv_dconf(ngx_conf_t *cf, void *conf);

static char *ngx_rtmp_merge_frame(ngx_conf_t *cf, void *post, void *data);

static ngx_conf_post_handler_pt  ngx_rtmp_merge_frame_p =
    ngx_rtmp_merge_frame;

ngx_rtmp_core_main_conf_t      *ngx_rtmp_core_main_conf;


static ngx_conf_deprecated_t  ngx_conf_deprecated_so_keepalive = {
    ngx_conf_deprecated, "so_keepalive",
    "so_keepalive\" parameter of the \"listen"
};


static ngx_command_t  ngx_rtmp_core_commands[] = {

    { ngx_string("server_names_hash_max_size"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_MAIN_CONF_OFFSET,
      offsetof(ngx_rtmp_core_main_conf_t, server_names_hash_max_size),
      NULL },

    { ngx_string("server_names_hash_bucket_size"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_MAIN_CONF_OFFSET,
      offsetof(ngx_rtmp_core_main_conf_t, server_names_hash_bucket_size),
      NULL },

    { ngx_string("fast_reload"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_MAIN_CONF_OFFSET,
      offsetof(ngx_rtmp_core_main_conf_t, fast_reload),
      NULL },

    { ngx_string("server"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_rtmp_core_server,
      0,
      0,
      NULL },

    { ngx_string("listen"),
      NGX_RTMP_SRV_CONF|NGX_CONF_TAKE12,
      ngx_rtmp_core_listen,
      NGX_RTMP_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("server_name"),
      NGX_RTMP_SRV_CONF|NGX_CONF_1MORE,
      ngx_rtmp_core_server_name,
      NGX_RTMP_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("application"),
      NGX_RTMP_SRV_CONF|NGX_CONF_BLOCK|NGX_CONF_TAKE1,
      ngx_rtmp_core_application,
      NGX_RTMP_SRV_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("so_keepalive"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, so_keepalive),
      &ngx_conf_deprecated_so_keepalive },

    { ngx_string("timeout"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, timeout),
      NULL },

    { ngx_string("ping"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, ping),
      NULL },

    { ngx_string("ping_timeout"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, ping_timeout),
      NULL },

    { ngx_string("max_streams"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, max_streams),
      NULL },

    { ngx_string("ack_window"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, ack_window),
      NULL },

    { ngx_string("chunk_size"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, chunk_size),
      NULL },

    { ngx_string("max_message"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, max_message),
      NULL },

    { ngx_string("out_queue"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, out_queue),
      NULL },

    { ngx_string("merge_frame"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_core_app_conf_t, merge_frame),
      &ngx_rtmp_merge_frame_p },

    { ngx_string("tcp_nodelay"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_core_app_conf_t, tcp_nodelay),
      NULL },

    { ngx_string("out_cork"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_size_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, out_cork),
      NULL },

    { ngx_string("busy"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, busy),
      NULL },

    /* time fixes are needed for flash clients */
    { ngx_string("play_time_fix"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, play_time_fix),
      NULL },

    { ngx_string("publish_time_fix"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_flag_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, publish_time_fix),
      NULL },

    { ngx_string("buflen"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_msec_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, buflen),
      NULL },

    { ngx_string("serverid"),
      NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_conf_t, serverid),
      NULL },

    { ngx_string("hevc_codecid"),
      NGX_RTMP_MAIN_CONF|NGX_RTMP_SRV_CONF|NGX_RTMP_APP_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      NGX_RTMP_APP_CONF_OFFSET,
      offsetof(ngx_rtmp_core_app_conf_t, hevc_codec),
      NULL },

      ngx_null_command
};


static ngx_rtmp_module_t  ngx_rtmp_core_module_ctx = {
    ngx_rtmp_core_preconfiguration,         /* preconfiguration */
    NULL,                                   /* postconfiguration */
    ngx_rtmp_core_create_main_conf,         /* create main configuration */
    ngx_rtmp_core_init_main_conf,           /* init main configuration */
    ngx_rtmp_core_create_srv_conf,          /* create server configuration */
    ngx_rtmp_core_merge_srv_conf,           /* merge server configuration */
    ngx_rtmp_core_create_app_conf,          /* create app configuration */
    ngx_rtmp_core_merge_app_conf            /* merge app configuration */
};

/* nginx rtmp dynamic */
static ngx_command_t  ngx_rtmp_core_dcommands[] = {

    { ngx_string("serverid"),
      NGX_RTMP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      NGX_RTMP_SRV_CONF_OFFSET,
      offsetof(ngx_rtmp_core_srv_dconf_t, serverid),
      NULL },

      ngx_null_command
};

static ngx_rtmp_dynamic_module_t  ngx_rtmp_core_module_dctx = {
    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    ngx_rtmp_core_create_srv_dconf,         /* create server configuration */
    ngx_rtmp_core_init_srv_dconf,           /* merge server configuration */

    NULL,                                   /* create app configuration */
    NULL                                    /* merge app configuration */
};


ngx_module_t  ngx_rtmp_core_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_core_module_ctx,              /* module context */
    ngx_rtmp_core_commands,                 /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_rtmp_core_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_rtmp_core_dcommands,    /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void *
ngx_rtmp_core_create_main_conf(ngx_conf_t *cf)
{
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_core_main_conf_t));
    if (cmcf == NULL) {
        return NULL;
    }

    ngx_rtmp_core_main_conf = cmcf;

    if (ngx_array_init(&cmcf->servers, cf->pool, 4,
                       sizeof(ngx_rtmp_core_srv_conf_t *))
        != NGX_OK)
    {
        return NULL;
    }

    cmcf->server_names_hash_max_size = NGX_CONF_UNSET_UINT;
    cmcf->server_names_hash_bucket_size = NGX_CONF_UNSET_UINT;
    cmcf->fast_reload = NGX_CONF_UNSET;
    cmcf->variables_hash_max_size = 1024;
    cmcf->variables_hash_bucket_size = 64;

    return cmcf;
}


static char *
ngx_rtmp_core_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_rtmp_core_main_conf_t  *cmcf;

    cmcf = conf;

    ngx_conf_init_uint_value(cmcf->server_names_hash_max_size, 512);
    ngx_conf_init_uint_value(cmcf->server_names_hash_bucket_size,
                             ngx_cacheline_size);
    ngx_conf_init_value(cmcf->fast_reload, 0);

    return NGX_CONF_OK;
}


static void *
ngx_rtmp_core_create_srv_conf(ngx_conf_t *cf)
{
    ngx_rtmp_core_srv_conf_t   *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_core_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&conf->applications, cf->pool, 4,
                       sizeof(ngx_rtmp_core_app_conf_t *))
        != NGX_OK)
    {
        return NULL;
    }

    if (ngx_array_init(&conf->server_names, cf->temp_pool, 4,
                       sizeof(ngx_rtmp_server_name_t))
        != NGX_OK)
    {
        return NULL;
    }

    conf->timeout = NGX_CONF_UNSET_MSEC;
    conf->ping = NGX_CONF_UNSET_MSEC;
    conf->ping_timeout = NGX_CONF_UNSET_MSEC;
    conf->so_keepalive = NGX_CONF_UNSET;
    conf->max_streams = NGX_CONF_UNSET;
    conf->chunk_size = NGX_CONF_UNSET;
    conf->ack_window = NGX_CONF_UNSET_UINT;
    conf->max_message = NGX_CONF_UNSET_SIZE;
    conf->out_queue = NGX_CONF_UNSET_SIZE;
    conf->out_cork = NGX_CONF_UNSET_SIZE;
    conf->play_time_fix = NGX_CONF_UNSET;
    conf->publish_time_fix = NGX_CONF_UNSET;
    conf->buflen = NGX_CONF_UNSET_MSEC;
    conf->busy = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_rtmp_core_merge_srv_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_core_srv_conf_t *prev = parent;
    ngx_rtmp_core_srv_conf_t *conf = child;

    ngx_str_t                   name;
    ngx_rtmp_server_name_t     *sn;

    ngx_conf_merge_msec_value(conf->timeout, prev->timeout, 60000);
    ngx_conf_merge_msec_value(conf->ping, prev->ping, 60000);
    ngx_conf_merge_msec_value(conf->ping_timeout, prev->ping_timeout, 30000);

    ngx_conf_merge_value(conf->so_keepalive, prev->so_keepalive, 0);
    ngx_conf_merge_value(conf->max_streams, prev->max_streams, 32);
    ngx_conf_merge_value(conf->chunk_size, prev->chunk_size, 4096);
    ngx_conf_merge_uint_value(conf->ack_window, prev->ack_window, 5000000);
    ngx_conf_merge_size_value(conf->max_message, prev->max_message,
            1 * 1024 * 1024);
    ngx_conf_merge_size_value(conf->out_queue, prev->out_queue, 2048);
    ngx_conf_merge_size_value(conf->out_cork, prev->out_cork,
            conf->out_queue / 8);
    ngx_conf_merge_value(conf->play_time_fix, prev->play_time_fix, 1);
    ngx_conf_merge_value(conf->publish_time_fix, prev->publish_time_fix, 1);
    ngx_conf_merge_msec_value(conf->buflen, prev->buflen, 1000);
    ngx_conf_merge_value(conf->busy, prev->busy, 0);
    ngx_conf_merge_str_value(conf->serverid, prev->serverid, "default");

    if (prev->pool == NULL) {
        prev->pool = ngx_create_pool(4096, &cf->cycle->new_log);
        if (prev->pool == NULL) {
            return NGX_CONF_ERROR;
        }
    }

    conf->pool = prev->pool;

    if (conf->server_names.nelts == 0) {
        sn = ngx_array_push(&conf->server_names);
#if (NGX_PCRE)
        sn->regex = NULL;
#endif
        sn->server = conf;
        ngx_str_set(&sn->name, "");
    }

    sn = conf->server_names.elts;
    name = sn[0].name;

#if (NGX_PCRE)
    if (sn->regex) {
        name.len++;
        name.data--;
    } else
#endif
    if (name.data[0] == '.') {
        name.len--;
        name.data++;
    }

    conf->server_name.len = name.len;
    conf->server_name.data = ngx_pstrdup(cf->pool, &name);
    if (conf->server_name.data == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static void *
ngx_rtmp_core_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_core_app_conf_t   *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_core_app_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&conf->applications, cf->pool, 1,
                       sizeof(ngx_rtmp_core_app_conf_t *))
        != NGX_OK)
    {
        return NULL;
    }

    conf->hevc_codec = NGX_CONF_UNSET_UINT;
    conf->merge_frame = NGX_CONF_UNSET_UINT;
    conf->tcp_nodelay = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_rtmp_core_merge_app_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_rtmp_core_app_conf_t *prev = parent;
    ngx_rtmp_core_app_conf_t *conf = child;

    ngx_conf_merge_uint_value(conf->hevc_codec, prev->hevc_codec, 12);
    ngx_conf_merge_uint_value(conf->merge_frame, prev->merge_frame, 32);
    ngx_conf_merge_value(conf->tcp_nodelay, prev->tcp_nodelay, 1);

    NGX_RTMP_HEVC_CODEC_ID = conf->hevc_codec;

    return NGX_CONF_OK;
}

/* nginx rtmp dynamic */
static void *
ngx_rtmp_core_create_srv_dconf(ngx_conf_t *cf)
{
    ngx_rtmp_core_srv_dconf_t      *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_core_srv_dconf_t));
    if (conf == NULL) {
        return NULL;
    }

    return conf;
}

static char *
ngx_rtmp_core_init_srv_dconf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}


static char *
ngx_rtmp_merge_frame(ngx_conf_t *cf, void *post, void *data)
{
    ngx_uint_t                 *mfp;

    mfp = data;

    if (*mfp > NGX_RTMP_MAX_MERGE_FRAME) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "the merge_frame must be no larger than %ui",
                           NGX_RTMP_MAX_MERGE_FRAME);
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


static char *
ngx_rtmp_core_server(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char                       *rv;
    void                       *mconf;
    ngx_uint_t                  m;
    ngx_conf_t                  pcf;
    ngx_module_t              **modules;
    ngx_rtmp_module_t          *module;
    struct sockaddr_in         *sin;
    ngx_rtmp_conf_ctx_t        *ctx, *rtmp_ctx;
    ngx_rtmp_listen_opt_t       lsopt;
    ngx_rtmp_core_srv_conf_t   *cscf, **cscfp;
    ngx_rtmp_core_main_conf_t  *cmcf;

    ctx = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_conf_ctx_t));
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }

    rtmp_ctx = cf->ctx;
    ctx->main_conf = rtmp_ctx->main_conf;

    /* the server{}'s srv_conf */

    ctx->srv_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_rtmp_max_module);
    if (ctx->srv_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    ctx->app_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_rtmp_max_module);
    if (ctx->app_conf == NULL) {
        return NGX_CONF_ERROR;
    }

#if (nginx_version >= 1009011)
    modules = cf->cycle->modules;
#else
    modules = ngx_modules;
#endif

    for (m = 0; modules[m]; m++) {
        if (modules[m]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = modules[m]->ctx;

        if (module->create_srv_conf) {
            mconf = module->create_srv_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            ctx->srv_conf[modules[m]->ctx_index] = mconf;
        }

        if (module->create_app_conf) {
            mconf = module->create_app_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            ctx->app_conf[modules[m]->ctx_index] = mconf;
        }
    }

    /* the server configuration context */

    cscf = ctx->srv_conf[ngx_rtmp_core_module.ctx_index];
    cscf->ctx = ctx;

    cmcf = ctx->main_conf[ngx_rtmp_core_module.ctx_index];

    cscfp = ngx_array_push(&cmcf->servers);
    if (cscfp == NULL) {
        return NGX_CONF_ERROR;
    }

    *cscfp = cscf;

    /* parse inside server{} */

    pcf = *cf;
    cf->ctx = ctx;
    cf->cmd_type = NGX_RTMP_SRV_CONF;

    rv = ngx_conf_parse(cf, NULL);

    *cf = pcf;

    if (rv == NGX_CONF_OK && !cscf->listen) {
        /* if listen not configured, set 1935 as default */
        ngx_memzero(&lsopt, sizeof(ngx_rtmp_listen_opt_t));

        lsopt.sockaddr = ngx_pcalloc(cf->pool, NGX_SOCKADDR_STRLEN);
        if (lsopt.sockaddr == NULL) {
            return NGX_CONF_ERROR;
        }

        sin = (struct sockaddr_in *) lsopt.sockaddr;

        sin->sin_family = AF_INET;
        sin->sin_port = htons(1935);
        sin->sin_addr.s_addr = INADDR_ANY;

        lsopt.socklen = sizeof(struct sockaddr_in);
        lsopt.wildcard = 1;

        (void) ngx_sock_ntop(lsopt.sockaddr, lsopt.socklen, lsopt.addr,
                             NGX_SOCKADDR_STRLEN, 1);

        if (ngx_rtmp_add_listen(cf, cscf, &lsopt) != NGX_OK) {
            return NGX_CONF_ERROR;
        }
    }

    return rv;
}


static char *
ngx_rtmp_core_application(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    char                       *rv;
    ngx_int_t                   i;
    ngx_str_t                  *value;
    ngx_conf_t                  save;
    ngx_module_t              **modules;
    ngx_rtmp_module_t          *module;
    ngx_rtmp_conf_ctx_t        *ctx, *pctx;
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_rtmp_core_app_conf_t   *cacf, **cacfp;

    ctx = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_conf_ctx_t));
    if (ctx == NULL) {
        return NGX_CONF_ERROR;
    }

    pctx = cf->ctx;
    ctx->main_conf = pctx->main_conf;
    ctx->srv_conf = pctx->srv_conf;

    ctx->app_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_rtmp_max_module);
    if (ctx->app_conf == NULL) {
        return NGX_CONF_ERROR;
    }

#if (nginx_version >= 1009011)
    modules = cf->cycle->modules;
#else
    modules = ngx_modules;
#endif

    for (i = 0; modules[i]; i++) {
        if (modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = modules[i]->ctx;

        if (module->create_app_conf) {
            ctx->app_conf[modules[i]->ctx_index] = module->create_app_conf(cf);
            if (ctx->app_conf[modules[i]->ctx_index] == NULL) {
                return NGX_CONF_ERROR;
            }
        }
    }

    cacf = ctx->app_conf[ngx_rtmp_core_module.ctx_index];
    cacf->app_conf = ctx->app_conf;

    value = cf->args->elts;

    cacf->name = value[1];
    cscf = pctx->srv_conf[ngx_rtmp_core_module.ctx_index];

    if (cacf->name.len == 1 && cacf->name.data[0] == '*') {  /* default_app */
        cscf->default_app = cacf;
    }

    cacfp = ngx_array_push(&cscf->applications);
    if (cacfp == NULL) {
        return NGX_CONF_ERROR;
    }

    *cacfp = cacf;

    save = *cf;
    cf->ctx = ctx;
    cf->cmd_type = NGX_RTMP_APP_CONF;

    rv = ngx_conf_parse(cf, NULL);

    *cf= save;

    return rv;
}


static char *
ngx_rtmp_core_listen(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_core_srv_conf_t    *cscf = conf;

    ngx_str_t                  *value;
    ngx_url_t                   u;
    ngx_uint_t                  i;
    ngx_rtmp_listen_opt_t       lsopt;

    cscf->listen = 1;

    value = cf->args->elts;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = value[1];
    u.listen = 1;
    u.default_port = 1935;

    if (ngx_parse_url(cf->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "%s in \"%V\" of the \"listen\" directive",
                               u.err, &u.url);
        }

        return NGX_CONF_ERROR;
    }

    ngx_memzero(&lsopt, sizeof(ngx_rtmp_listen_opt_t));

    lsopt.sockaddr = ngx_pcalloc(cf->pool, NGX_SOCKADDR_STRLEN);
    if (lsopt.sockaddr == NULL) {
        return NGX_CONF_ERROR;
    }
    ngx_memcpy(lsopt.sockaddr, (u_char *) &u.sockaddr, u.socklen);

    lsopt.socklen = u.socklen;
    lsopt.wildcard = u.wildcard;

    (void) ngx_sock_ntop(lsopt.sockaddr, lsopt.socklen, lsopt.addr,
                         NGX_SOCKADDR_STRLEN, 1);

    for (i = 2; i < cf->args->nelts; i++) {

        if (ngx_strcmp(value[i].data, "default_server") == 0
            || ngx_strcmp(value[i].data, "default") == 0)
        {
            lsopt.default_server = 1;
            continue;
        }

        if (ngx_strcmp(value[i].data, "bind") == 0) {
            lsopt.bind = 1;
            continue;
        }

        if (ngx_strncmp(value[i].data, "ipv6only=o", 10) == 0) {
#if (NGX_HAVE_INET6 && defined IPV6_V6ONLY)
            struct sockaddr  *sa;

            sa = lsopt.sockaddr;

            if (sa->sa_family == AF_INET6) {

                if (ngx_strcmp(&value[i].data[10], "n") == 0) {
                    lsopt.ipv6only = 1;

                } else if (ngx_strcmp(&value[i].data[10], "ff") == 0) {
                    lsopt.ipv6only = 0;

                } else {
                    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                       "invalid ipv6only flags \"%s\"",
                                       &value[i].data[9]);
                    return NGX_CONF_ERROR;
                }

                lsopt.bind = 1;

            } else {
                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "ipv6only is not supported "
                                   "on addr \"%s\", ignored", lsopt.addr);
            }

            continue;
#else
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "bind ipv6only is not supported "
                               "on this platform");
            return NGX_CONF_ERROR;
#endif
        }

        if (ngx_strcmp(value[i].data, "reuseport") == 0) {
#if (NGX_HAVE_REUSEPORT)
            lsopt.reuseport = 1;
            lsopt.set = 1;
            lsopt.bind = 1;
#else
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "reuseport is not supported "
                               "on this platform, ignored");
#endif
            continue;
        }

        if (ngx_strncmp(value[i].data, "so_keepalive=", 13) == 0) {

            if (ngx_strcmp(&value[i].data[13], "on") == 0) {
                lsopt.so_keepalive = 1;

            } else if (ngx_strcmp(&value[i].data[13], "off") == 0) {
                lsopt.so_keepalive = 2;

            } else {

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
                u_char     *p, *end;
                ngx_str_t   s;

                end = value[i].data + value[i].len;
                s.data = value[i].data + 13;

                p = ngx_strlchr(s.data, end, ':');
                if (p == NULL) {
                    p = end;
                }

                if (p > s.data) {
                    s.len = p - s.data;

                    lsopt.tcp_keepidle = ngx_parse_time(&s, 1);
                    if (lsopt.tcp_keepidle == (time_t) NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                s.data = (p < end) ? (p + 1) : end;

                p = ngx_strlchr(s.data, end, ':');
                if (p == NULL) {
                    p = end;
                }

                if (p > s.data) {
                    s.len = p - s.data;

                    lsopt.tcp_keepintvl = ngx_parse_time(&s, 1);
                    if (lsopt.tcp_keepintvl == (time_t) NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                s.data = (p < end) ? (p + 1) : end;

                if (s.data < end) {
                    s.len = end - s.data;

                    lsopt.tcp_keepcnt = ngx_atoi(s.data, s.len);
                    if (lsopt.tcp_keepcnt == NGX_ERROR) {
                        goto invalid_so_keepalive;
                    }
                }

                if (lsopt.tcp_keepidle == 0 && lsopt.tcp_keepintvl == 0
                    && lsopt.tcp_keepcnt == 0)
                {
                    goto invalid_so_keepalive;
                }

                lsopt.so_keepalive = 1;

#else

                ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                                   "the \"so_keepalive\" parameter accepts "
                                   "only \"on\" or \"off\" on this platform");
                return NGX_CONF_ERROR;

#endif
            }

            lsopt.bind = 1;

            continue;

#if (NGX_HAVE_KEEPALIVE_TUNABLE)
        invalid_so_keepalive:

            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid so_keepalive value: \"%s\"",
                               &value[i].data[13]);
            return NGX_CONF_ERROR;
#endif
        }

        if (ngx_strcmp(value[i].data, "proxy_protocol") == 0) {
            lsopt.proxy_protocol = 1;
            continue;
        }

        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "the invalid \"%V\" parameter", &value[i]);
        return NGX_CONF_ERROR;
    }

    if (ngx_rtmp_add_listen(cf, cscf, &lsopt) == NGX_OK) {
        return NGX_CONF_OK;
    }

    return NGX_CONF_ERROR;
}

static char *
ngx_rtmp_core_server_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_core_srv_conf_t *cscf = conf;

    u_char                   ch;
    ngx_str_t               *value;
    ngx_uint_t               i;
    ngx_rtmp_server_name_t  *sn;

    value = cf->args->elts;

    for (i = 1; i < cf->args->nelts; i++) {

        ch = value[i].data[0];

        if ((ch == '*' && (value[i].len < 3 || value[i].data[1] != '.'))
            || (ch == '.' && value[i].len < 2))
        {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "server name \"%V\" is invalid", &value[i]);
            return NGX_CONF_ERROR;
        }

        if (ngx_strchr(value[i].data, '/')) {
            ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                               "server name \"%V\" has suspicious symbols",
                               &value[i]);
        }

        sn = ngx_array_push(&cscf->server_names);
        if (sn == NULL) {
            return NGX_CONF_ERROR;
        }

#if (NGX_PCRE)
        sn->regex = NULL;
#endif
        sn->server = cscf;

        if (ngx_strcasecmp(value[i].data, (u_char *) "$hostname") == 0) {
            sn->name = cf->cycle->hostname;

        } else {
            sn->name = value[i];
        }

        if (value[i].data[0] != '~') {
            ngx_strlow(sn->name.data, sn->name.data, sn->name.len);
            continue;
        }

#if (NGX_PCRE)
        {
        u_char               *p;
        ngx_regex_compile_t   rc;
        u_char                errstr[NGX_MAX_CONF_ERRSTR];

        if (value[i].len == 1) {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "empty regex in server name \"%V\"", &value[i]);
            return NGX_CONF_ERROR;
        }

        value[i].len--;
        value[i].data++;

        ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

        rc.pattern = value[i];
        rc.err.len = NGX_MAX_CONF_ERRSTR;
        rc.err.data = errstr;

        for (p = value[i].data; p < value[i].data + value[i].len; p++) {
            if (*p >= 'A' && *p <= 'Z') {
                rc.options = NGX_REGEX_CASELESS;
                break;
            }
        }

        sn->regex = ngx_rtmp_regex_compile(cf, &rc);
        if (sn->regex == NULL) {
            return NGX_CONF_ERROR;
        }

        sn->name = value[i];
        cscf->captures = (rc.captures > 0);
        }
#else
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "using regex \"%V\" "
                           "requires PCRE library", &value[i]);

        return NGX_CONF_ERROR;
#endif
    }

    return NGX_CONF_OK;
}

static ngx_rtmp_addr_conf_t *
ngx_rtmp_get_addr_conf(ngx_listening_t *ls, struct sockaddr *sa, socklen_t len)
{
    ngx_uint_t                  i;
    ngx_rtmp_addr_conf_t       *addr_conf;
    ngx_rtmp_port_t            *port;
    struct sockaddr_in         *sin;
    ngx_rtmp_in_addr_t         *addr;
#if (NGX_HAVE_INET6)
    struct sockaddr_in6        *sin6;
    ngx_rtmp_in6_addr_t        *addr6;
#endif

    port = ls->servers;
    addr_conf = NULL;

    if (port->naddrs > 1) {
        switch (sa->sa_family) {
#if (NGX_HAVE_INET6)
        case AF_INET6:
            sin6 = (struct sockaddr_in6 *) sa;

            addr6 = port->addrs;

            /* the last address is "*" */

            for (i = 0; i < port->naddrs - 1; ++i) {
                if (ngx_memcmp(&addr6[i].addr6, &sin6->sin6_addr, 16) == 0) {
                    break;
                }
            }

            addr_conf = &addr6[i].conf;

            break;
#endif
        default:
            sin = (struct sockaddr_in *) sa;

            addr = port->addrs;

            /* the last address is "*" */

            for (i = 0; i < port->naddrs - 1; ++i) {
                if (addr[i].addr == sin->sin_addr.s_addr) {
                    break;
                }
            }

            addr_conf = &addr[i].conf;

            break;
        }
    } else {

        switch (sa->sa_family) {
#if (NGX_HAVE_INET6)
        case AF_INET6:
            addr6 = port->addrs;
            addr_conf = &addr6[0].conf;
            break;
#endif

        default:
            addr = port->addrs;
            addr_conf = &addr[0].conf;
            break;
        }
    }

    return addr_conf;
}

ngx_rtmp_addr_conf_t *
ngx_rtmp_find_related_addr_conf(ngx_cycle_t *cycle, ngx_str_t *addr)
{
    ngx_url_t                   u;
    ngx_listening_t            *ls;
    ngx_uint_t                  i;

    ngx_memzero(&u, sizeof(ngx_url_t));

    u.url = *addr;
    u.listen = 1;
    u.default_port = 0;

    if (ngx_parse_url(cycle->pool, &u) != NGX_OK) {
        if (u.err) {
            ngx_log_error(NGX_LOG_EMERG, cycle->log, 0,
                    "Relation port err: %V", addr);
        }

        return NULL;
    }

    ls = cycle->listening.elts;

    for (i = 0; i < cycle->listening.nelts; ++i, ++ls) {

        if (ls->handler == ngx_rtmp_init_connection
            && ls->socklen == u.socklen
                && ngx_memcmp(ls->sockaddr, (u_char *) &u.sockaddr,
                    u.socklen) == 0)
        {
            return ngx_rtmp_get_addr_conf(ls, &u.sockaddr.sockaddr, u.socklen);
        }
    }

    ngx_log_error(NGX_LOG_EMERG, cycle->log, 0,
            "Can not find relation port: %V", addr);

    return NULL;
}


ngx_int_t
ngx_rtmp_arg(ngx_rtmp_session_t *s, u_char *name, size_t len, ngx_str_t *value)
{
    u_char                     *p, *last;

    if (s->pargs.len == 0) {
        return NGX_DECLINED;
    }

    p = s->pargs.data;
    last = p + s->pargs.len;

    for ( /* void */ ; p < last; p++) {

        /* we need '=' after name, so drop one char from last */

        p = ngx_strlcasestrn(p, last - 1, name, len - 1);

        if (p == NULL) {
            return NGX_DECLINED;
        }

        if ((p == s->pargs.data || *(p - 1) == '&') && *(p + len) == '=') {

            value->data = p + len + 1;

            p = ngx_strlchr(p, last, '&');

            if (p == NULL) {
                p = s->pargs.data + s->pargs.len;
            }

            value->len = p - value->data;

            return NGX_OK;
        }
    }

    return NGX_DECLINED;
}


static ngx_int_t
ngx_rtmp_core_preconfiguration(ngx_conf_t *cf)
{
    return ngx_rtmp_variables_add_core_vars(cf);
}
