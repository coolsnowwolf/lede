/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_dynamic.h"


static void *ngx_rtmp_dynamic_create_conf(ngx_conf_t *cf);
static void *ngx_rtmp_dynamic_core_create_main_conf(ngx_conf_t *cf);
static char *ngx_rtmp_dynamic_core_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_rtmp_dynamic_core_create_srv_conf(ngx_conf_t *cf);
static char *ngx_rtmp_dynamic_core_init_srv_conf(ngx_conf_t *cf, void *conf);
static void *ngx_rtmp_dynamic_core_create_app_conf(ngx_conf_t *cf);
static char *ngx_rtmp_dynamic_core_init_app_conf(ngx_conf_t *cf, void *conf);

static char *ngx_rtmp_dynamic_block(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_rtmp_dynamic_core_server(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_rtmp_dynamic_core_server_name(ngx_conf_t *cf,
       ngx_command_t *cmd, void *conf);
static char *ngx_rtmp_dynamic_core_application(ngx_conf_t *cf,
       ngx_command_t *cmd, void *conf);


typedef struct {
    ngx_str_t                           name;
    ngx_map_node_t                      node;

    void                              **app_conf;
} ngx_rtmp_dynamic_core_app_conf_t;

typedef struct {
    /* array of the ngx_rtmp_server_name_t, "server_name" directive */
    ngx_array_t                         server_names;

    ngx_rtmp_dynamic_core_app_conf_t   *default_app;
    ngx_map_t                           app_conf;

    void                              **srv_conf;
} ngx_rtmp_dynamic_core_srv_conf_t;

typedef struct {
#if (NGX_PCRE)
    ngx_dynamic_regex_t                *regex;
#endif
    ngx_rtmp_dynamic_core_srv_conf_t   *server;   /* virtual name server conf */
    ngx_str_t                           name;
} ngx_rtmp_dynamic_server_name_t;


typedef struct {
    /* ngx_rtmp_dynamic_srv_conf_t */
    ngx_array_t                         servers;

    ngx_uint_t                          server_names_hash_max_size;
    ngx_uint_t                          server_names_hash_bucket_size;

    /* the default server configuration for this address:port */
    ngx_rtmp_dynamic_core_srv_conf_t   *default_server;

    ngx_hash_combined_t                 names;

#if (NGX_PCRE)
    ngx_uint_t                          nregex;
    ngx_rtmp_dynamic_server_name_t     *regex;
#endif
} ngx_rtmp_dynamic_core_main_conf_t;


typedef struct {
    void                              **main_conf;
} ngx_rtmp_dynamic_conf_t;


static ngx_core_module_t  ngx_rtmp_dynamic_module_ctx = {
    ngx_string("rtmp_dynamic"),
    NULL,
    NULL
};

static ngx_command_t  ngx_rtmp_dynamic_dcommands[] = {

    { ngx_string("rtmp"),
      NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_rtmp_dynamic_block,
      0,
      0,
      NULL },

      ngx_null_command
};

static ngx_dynamic_core_module_t  ngx_rtmp_dynamic_module_dctx = {
    ngx_string("rtmp_dynamic"),
    ngx_rtmp_dynamic_create_conf,
    NULL
};

ngx_module_t  ngx_rtmp_dynamic_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_dynamic_module_ctx,       /* module context */
    NULL,                               /* module directives */
    NGX_CORE_MODULE,                    /* module type */
    NULL,                               /* init master */
    NULL,                               /* init module */
    NULL,                               /* init process */
    NULL,                               /* init thread */
    NULL,                               /* exit thread */
    NULL,                               /* exit process */
    NULL,                               /* exit master */
    (uintptr_t) &ngx_rtmp_dynamic_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_rtmp_dynamic_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static ngx_rtmp_module_t  ngx_rtmp_dynamic_core_module_ctx = {
    NULL,                               /* preconfiguration */
    NULL,                               /* postconfiguration */

    NULL,                               /* create main configuration */
    NULL,                               /* init main configuration */

    NULL,                               /* create server configuration */
    NULL,                               /* merge server configuration */

    NULL,                               /* create application configuration */
    NULL                                /* merge application configuration */
};


static ngx_command_t  ngx_rtmp_dynamic_core_dcommands[] = {

    { ngx_string("server_names_hash_max_size"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_rtmp_dynamic_core_main_conf_t, server_names_hash_max_size),
      NULL },

    { ngx_string("server_names_hash_bucket_size"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_rtmp_dynamic_core_main_conf_t,
               server_names_hash_bucket_size),
      NULL },

    { ngx_string("server"),
      NGX_RTMP_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_rtmp_dynamic_core_server,
      0,
      0,
      NULL },

    { ngx_string("server_name"),
      NGX_RTMP_SRV_CONF|NGX_CONF_1MORE,
      ngx_rtmp_dynamic_core_server_name,
      0,
      0,
      NULL },

    { ngx_string("application"),
      NGX_RTMP_SRV_CONF|NGX_CONF_BLOCK|NGX_CONF_TAKE12,
      ngx_rtmp_dynamic_core_application,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_rtmp_dynamic_module_t  ngx_rtmp_dynamic_core_module_dctx = {
    ngx_rtmp_dynamic_core_create_main_conf, /* create main configuration */
    ngx_rtmp_dynamic_core_init_main_conf,   /* init main configuration */

    ngx_rtmp_dynamic_core_create_srv_conf,  /* create srv configuration */
    ngx_rtmp_dynamic_core_init_srv_conf,    /* init srv configuration */

    ngx_rtmp_dynamic_core_create_app_conf,  /* create app configuration */
    ngx_rtmp_dynamic_core_init_app_conf     /* init app configuration */
};


ngx_module_t  ngx_rtmp_dynamic_core_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_dynamic_core_module_ctx,      /* module context */
    NULL,                                   /* module directives */
    NGX_RTMP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_rtmp_dynamic_core_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_rtmp_dynamic_core_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void *
ngx_rtmp_dynamic_create_conf(ngx_conf_t *cf)
{
    ngx_rtmp_dynamic_conf_t                *rdccf;

    rdccf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_dynamic_conf_t));
    if (rdccf == NULL) {
        return NULL;
    }

    return rdccf;
}

static void *
ngx_rtmp_dynamic_core_create_main_conf(ngx_conf_t *cf)
{
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;

    rdcmcf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_dynamic_core_main_conf_t));
    if (rdcmcf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&rdcmcf->servers, cf->pool, 4,
        sizeof(ngx_rtmp_dynamic_core_srv_conf_t *)) != NGX_OK)
    {
        return NULL;
    }

    rdcmcf->server_names_hash_max_size = NGX_CONF_UNSET_UINT;
    rdcmcf->server_names_hash_bucket_size = NGX_CONF_UNSET_UINT;

    return rdcmcf;
}

static char *
ngx_rtmp_dynamic_core_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;

    rdcmcf = conf;

    ngx_conf_init_uint_value(rdcmcf->server_names_hash_max_size, 512);
    ngx_conf_init_uint_value(rdcmcf->server_names_hash_bucket_size,
                             ngx_cacheline_size);

    return NGX_CONF_OK;
}

static void *
ngx_rtmp_dynamic_core_create_srv_conf(ngx_conf_t *cf)
{
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;

    rdcscf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_dynamic_core_srv_conf_t));
    if (rdcscf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&rdcscf->server_names, cf->pool, 4,
        sizeof(ngx_rtmp_dynamic_server_name_t)) != NGX_OK)
    {
        return NULL;
    }

    return rdcscf;
}

static char *
ngx_rtmp_dynamic_core_init_srv_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static void *
ngx_rtmp_dynamic_core_create_app_conf(ngx_conf_t *cf)
{
    ngx_rtmp_dynamic_core_app_conf_t       *rdcacf;

    rdcacf = ngx_pcalloc(cf->pool, sizeof(ngx_rtmp_dynamic_core_app_conf_t));
    if (rdcacf == NULL) {
        return NULL;
    }

    return rdcacf;
}

static char *
ngx_rtmp_dynamic_core_init_app_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static ngx_int_t
ngx_rtmp_dynamic_core_find_application(ngx_rtmp_session_t *s,
        ngx_rtmp_dynamic_core_srv_conf_t *rdcscf,
        ngx_rtmp_dynamic_core_app_conf_t **rdcacf)
{
    ngx_map_node_t                         *node;

    node = ngx_map_find(&rdcscf->app_conf, (intptr_t) &s->app);
    if (node == NULL) {
        *rdcacf = rdcscf->default_app;
    } else {
        *rdcacf = (ngx_rtmp_dynamic_core_app_conf_t *)
            ((char *) node - offsetof(ngx_rtmp_dynamic_core_app_conf_t, node));
    }

    return NGX_OK;
}

static ngx_int_t
ngx_rtmp_dynamic_core_init_virtual_servers(ngx_conf_t *cf,
        ngx_rtmp_dynamic_conf_t *hdcf)
{
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;
    ngx_rtmp_dynamic_core_srv_conf_t      **rdcscfp;
    ngx_hash_init_t                         hash;
    ngx_hash_keys_arrays_t                  ha;
    ngx_rtmp_dynamic_server_name_t         *name;
    ngx_uint_t                              s, n;
    ngx_int_t                               rc;
#if (NGX_PCRE)
    ngx_uint_t                              regex, i;

    regex = 0;
#endif

    rdcmcf = hdcf->main_conf[ngx_rtmp_dynamic_core_module.ctx_index];

    ngx_memzero(&ha, sizeof(ngx_hash_keys_arrays_t));

    ha.temp_pool = ngx_create_pool(NGX_DEFAULT_POOL_SIZE, cf->log);
    if (ha.temp_pool == NULL) {
        return NGX_ERROR;
    }

    ha.pool = cf->pool;

    if (ngx_hash_keys_array_init(&ha, NGX_HASH_LARGE) != NGX_OK) {
        goto failed;
    }

    rdcscfp = rdcmcf->servers.elts;
    rdcmcf->default_server = rdcscfp[0];

    for (s = 0; s < rdcmcf->servers.nelts; ++s) {

        name = rdcscfp[s]->server_names.elts;

        for (n = 0; n < rdcscfp[s]->server_names.nelts; ++n) {
#if (NGX_PCRE)
            if (name[n].regex) {
                ++regex;
                continue;
            }
#endif

            rc = ngx_hash_add_key(&ha, &name[n].name, name[n].server,
                                  NGX_HASH_WILDCARD_KEY);
            if (rc == NGX_ERROR) {
                return NGX_ERROR;
            }

            if (rc == NGX_DECLINED) {
                ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                        "invalid server name or wildcard \"%V\"",
                        &name[n].name);
                return NGX_ERROR;
            }

            if (rc == NGX_BUSY) {
                ngx_log_error(NGX_LOG_WARN, cf->log, 0,
                        "conflicting server name \"%V\", ignored",
                        &name[n].name);
            }
        }
    }

    hash.key = ngx_hash_key_lc;
    hash.max_size = rdcmcf->server_names_hash_max_size;
    hash.bucket_size = rdcmcf->server_names_hash_bucket_size;
    hash.name = "rtmp_dynamic_server_names_hash";
    hash.pool = cf->pool;

    if (ha.keys.nelts) {
        hash.hash = &rdcmcf->names.hash;
        hash.temp_pool = NULL;

        if (ngx_hash_init(&hash, ha.keys.elts, ha.keys.nelts) != NGX_OK) {
            goto failed;
        }
    }

    if (ha.dns_wc_head.nelts) {

        ngx_qsort(ha.dns_wc_head.elts, (size_t) ha.dns_wc_head.nelts,
                  sizeof(ngx_hash_key_t), ngx_dynamic_cmp_dns_wildcards);

        hash.hash = NULL;
        hash.temp_pool = ha.temp_pool;

        if (ngx_hash_wildcard_init(&hash, ha.dns_wc_head.elts,
            ha.dns_wc_head.nelts) != NGX_OK)
        {
            goto failed;
        }

        rdcmcf->names.wc_head = (ngx_hash_wildcard_t *) hash.hash;
    }

    if (ha.dns_wc_tail.nelts) {

        ngx_qsort(ha.dns_wc_tail.elts, (size_t) ha.dns_wc_tail.nelts,
                  sizeof(ngx_hash_key_t), ngx_dynamic_cmp_dns_wildcards);

        hash.hash = NULL;
        hash.temp_pool = ha.temp_pool;

        if (ngx_hash_wildcard_init(&hash, ha.dns_wc_tail.elts,
            ha.dns_wc_tail.nelts) != NGX_OK)
        {
            goto failed;
        }

        rdcmcf->names.wc_tail = (ngx_hash_wildcard_t *) hash.hash;

    }

    ngx_destroy_pool(ha.temp_pool);

#if (NGX_PCRE)
    if (regex == 0) {
        return NGX_OK;
    }

    rdcmcf->nregex = regex;
    rdcmcf->regex = ngx_pcalloc(cf->pool,
                                regex * sizeof(ngx_rtmp_dynamic_server_name_t));
    if (rdcmcf->regex == NULL) {
        return NGX_ERROR;
    }

    i = 0;

    for (s = 0; s < rdcmcf->servers.nelts; ++s) {

        name = rdcscfp[s]->server_names.elts;

        for (n = 0; n < rdcscfp[s]->server_names.nelts; ++n) {
            if (name[n].regex) {
                rdcmcf->regex[i++] = name[n];
            }
        }
    }
#endif

    return NGX_OK;

failed:
    ngx_destroy_pool(ha.temp_pool);

    return NGX_ERROR;
}

static ngx_int_t
ngx_rtmp_dynamic_core_find_virtual_server(ngx_str_t *server,
        ngx_rtmp_dynamic_core_main_conf_t *rdcmcf,
        ngx_rtmp_dynamic_core_srv_conf_t **rdcscfp)
{
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;

    rdcscf = ngx_hash_find_combined(&rdcmcf->names,
                                    ngx_hash_key(server->data, server->len),
                                    server->data, server->len);

    if (rdcscf) {
        *rdcscfp = rdcscf;
        return NGX_OK;
    }

#if (NGX_PCRE)

    if (server->len && rdcmcf->nregex) {
        ngx_int_t                           n;
        ngx_uint_t                          i;
        ngx_rtmp_dynamic_server_name_t     *sn;

        sn = rdcmcf->regex;

        for (i = 0; i < rdcmcf->nregex; ++i) {

            n = ngx_regex_exec(sn[i].regex->regex, server, NULL, 0);

            if (n == NGX_REGEX_NO_MATCHED) {
                continue;
            }

            if (n >= 0) {
                *rdcscfp = sn[i].server;
                return NGX_OK;
            }

            return NGX_ERROR;
        }
    }
#endif

    return NGX_DECLINED;
}

static char *
ngx_rtmp_dynamic_core_application(ngx_conf_t *cf, ngx_command_t *cmd,
        void *conf)
{
    ngx_rtmp_dynamic_module_t              *module;
    ngx_rtmp_dynamic_core_app_conf_t       *rdcacf;
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;
    ngx_conf_t                              pcf;
    void                                   *mconf, **app_conf;
    ngx_uint_t                              i, ci;
    ngx_str_t                              *value;
    char                                   *rv = NULL;

    rdcscf = conf;

    /* app_conf is temp cause hdclcf does not created now */
    app_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_rtmp_max_module);
    if (app_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;

        if (module == NULL) {
            continue;
        }

        if (module->create_app_conf) {
            mconf = module->create_app_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            app_conf[cf->cycle->modules[i]->ctx_index] = mconf;
        }
    }

    rdcacf = app_conf[ngx_rtmp_dynamic_core_module.ctx_index];
    rdcacf->app_conf = app_conf;

    value = cf->args->elts;

    rdcacf->name = value[1];
    if (rdcacf->name.len == 1 && rdcacf->name.data[0] == '*') { /* default */
        if (rdcscf->default_app) {
            return "is duplicate";
        }

        rdcscf->default_app = rdcacf;
    } else {
        rdcacf->node.raw_key = (intptr_t) &rdcacf->name;
        ngx_map_insert(&rdcscf->app_conf, &rdcacf->node, 0);
    }

    pcf = *cf;
    cf->ctx = rdcacf->app_conf;
    cf->cmd_type = NGX_RTMP_APP_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;
        if (module == NULL) {
            continue;
        }

        if (module->init_app_conf) {
            rv = module->init_app_conf(cf, rdcacf->app_conf[ci]);
            if (rv != NGX_CONF_OK) {
                goto failed;
            }
        }
    }

    *cf = pcf;

    return NGX_CONF_OK;

failed:
    *cf = pcf;

    if (rv) {
        return rv;
    }

    return NGX_CONF_ERROR;
}

static char *
ngx_rtmp_dynamic_core_server_name(ngx_conf_t *cf, ngx_command_t *cmd,
        void *conf)
{
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;
    u_char                                  ch;
    ngx_str_t                              *value;
    ngx_uint_t                              i;
    ngx_rtmp_dynamic_server_name_t         *sn;

    rdcscf = conf;

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

        sn = ngx_array_push(&rdcscf->server_names);
        if (sn == NULL) {
            return NGX_CONF_ERROR;
        }

#if (NGX_PCRE)
        sn->regex = NULL;
#endif
        sn->server = rdcscf;

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
        u_char                             *p;
        ngx_regex_compile_t                 rc;
        u_char                              errstr[NGX_MAX_CONF_ERRSTR];

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

        sn->regex = ngx_dynamic_regex_compile(cf, &rc);
        if (sn->regex == NULL) {
            return NGX_CONF_ERROR;
        }

        sn->name = value[i];
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

static char *
ngx_rtmp_dynamic_core_server(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_dynamic_module_t              *module;
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf, **rdcscfp;
    ngx_conf_t                              pcf;
    void                                   *mconf, **srv_conf;
    ngx_uint_t                              i, ci;
    char                                   *rv = NULL;

    rdcmcf = conf;

    /* srv_conf is temp cause hdcscf does not created now */
    srv_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_rtmp_max_module);
    if (srv_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        if (module == NULL) {
            continue;
        }

        if (module->create_srv_conf) {
            mconf = module->create_srv_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            srv_conf[cf->cycle->modules[i]->ctx_index] = mconf;
        }
    }

    rdcscf = srv_conf[ngx_rtmp_dynamic_core_module.ctx_index];
    rdcscf->srv_conf = srv_conf;
    ngx_map_init(&rdcscf->app_conf, ngx_map_hash_str, ngx_cmp_str);

    /* save hdcscf into hdcmcf */
    rdcscfp = ngx_array_push(&rdcmcf->servers);
    if (rdcscfp == NULL) {
        return NGX_CONF_ERROR;
    }
    *rdcscfp = rdcscf;

    pcf = *cf;
    cf->ctx = rdcscf->srv_conf;
    cf->cmd_type = NGX_RTMP_SRV_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;
        if (module == NULL) {
            continue;
        }

        if (module->init_srv_conf) {
            rv = module->init_srv_conf(cf, rdcscf->srv_conf[ci]);
            if (rv != NGX_CONF_OK) {
                goto failed;
            }
        }
    }

    *cf = pcf;

    return NGX_CONF_OK;

failed:
    *cf = pcf;

    if (rv) {
        return rv;
    }

    return NGX_CONF_ERROR;
}

static char *
ngx_rtmp_dynamic_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_rtmp_dynamic_conf_t                *rdccf;
    ngx_rtmp_dynamic_module_t              *module;
    ngx_conf_t                              pcf;
    ngx_uint_t                              i, ci;
    char                                   *rv = NULL;

    rdccf = conf;

    /* create main_conf ctx */
    rdccf->main_conf = ngx_pcalloc(cf->pool, sizeof(void *)
                                   * ngx_rtmp_max_module);
    if (rdccf->main_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    /* create http dynamic conf for all http module */
    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;

        if (module == NULL) {
            continue;
        }

        if (module->create_main_conf) {
            rdccf->main_conf[ci] = module->create_main_conf(cf);
            if (rdccf->main_conf[ci] == NULL) {
                return NGX_CONF_ERROR;
            }
        }
    }

    /* save conf for recovery */
    pcf = *cf;

    cf->ctx = rdccf->main_conf;
    cf->module_type = NGX_RTMP_MODULE;
    cf->cmd_type = NGX_RTMP_MAIN_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_RTMP_MODULE) {
            continue;
        }

        module = (ngx_rtmp_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;

        if (module == NULL) {
            continue;
        }

        if (module->init_main_conf) {
            rv = module->init_main_conf(cf, rdccf->main_conf[ci]);
            if (rv != NGX_CONF_OK) {
                goto failed;
            }
        }
    }

    if (ngx_rtmp_dynamic_core_init_virtual_servers(cf, rdccf) != NGX_OK) {
        goto failed;
    }

    *cf = pcf;

    return NGX_CONF_OK;

failed:
    *cf = pcf;

    if (rv) {
        return rv;
    }

    return NGX_CONF_ERROR;
}


/* interface from here */

void *
ngx_rtmp_get_module_main_dconf(ngx_rtmp_session_t *s, ngx_module_t *m)
{
    ngx_rtmp_dynamic_conf_t                *rdcf;

    rdcf = ngx_get_dconf(&ngx_rtmp_dynamic_module);
    if (rdcf == NULL || rdcf->main_conf == NULL) {
        return NULL;
    }

    return rdcf->main_conf[m->ctx_index];
}

void *
ngx_rtmp_get_module_srv_dconf(ngx_rtmp_session_t *s, ngx_module_t *m)
{
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;
    ngx_int_t                               rc;

    rdcmcf = ngx_rtmp_get_module_main_dconf(s, &ngx_rtmp_dynamic_core_module);
    if (rdcmcf == NULL) {
        return NULL;
    }

    rc = ngx_rtmp_dynamic_core_find_virtual_server(&s->domain, rdcmcf, &rdcscf);
    switch (rc) {
    case NGX_ERROR:
        return NULL;
    case NGX_DECLINED:  /* virtual server not found */
        return rdcmcf->default_server->srv_conf[m->ctx_index];
    default:
        return rdcscf->srv_conf[m->ctx_index];
    }
}

void *
ngx_rtmp_get_module_app_dconf(ngx_rtmp_session_t *s, ngx_module_t *m)
{
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;
    ngx_rtmp_dynamic_core_app_conf_t       *rdcacf;
    ngx_int_t                               rc;

    rdcscf = ngx_rtmp_get_module_srv_dconf(s, &ngx_rtmp_dynamic_core_module);
    if (rdcscf == NULL) {
        return NULL;
    }

    rdcacf = NULL;
    rc = ngx_rtmp_dynamic_core_find_application(s, rdcscf, &rdcacf);
    if (rc == NGX_ERROR) {
        return NULL;
    }

    if (rdcacf) {
        return rdcacf->app_conf[m->ctx_index];
    }

    return NULL;
}

void
ngx_rmtp_get_serverid_by_domain(ngx_str_t *serverid, ngx_str_t *domain)
{
    ngx_rtmp_dynamic_conf_t                *rdcf;
    ngx_rtmp_dynamic_core_main_conf_t      *rdcmcf;
    ngx_rtmp_dynamic_core_srv_conf_t       *rdcscf;
    ngx_rtmp_core_srv_dconf_t              *rcsdcf;

    rdcf = ngx_get_dconf(&ngx_rtmp_dynamic_module);
    if (rdcf == NULL || rdcf->main_conf) {
        goto notfound;
    }

    rdcmcf = rdcf->main_conf[ngx_rtmp_dynamic_core_module.ctx_index];
    if (rdcmcf == NULL) {
        goto notfound;
    }

    ngx_rtmp_dynamic_core_find_virtual_server(domain, rdcmcf, &rdcscf);
    if (rdcscf && rdcscf->srv_conf) {
        rcsdcf = rdcscf->srv_conf[ngx_rtmp_core_module.ctx_index];
        if (rcsdcf && rcsdcf->serverid.len) {
            *serverid = rcsdcf->serverid;
            return;
        }
    }

notfound:
    *serverid = *domain;
}
