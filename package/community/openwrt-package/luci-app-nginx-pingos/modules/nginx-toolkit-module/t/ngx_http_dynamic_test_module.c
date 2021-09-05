#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_dynamic_conf.h"
#include "ngx_http_dynamic.h"


static char *ngx_http_dynamic_test(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);

static void *ngx_http_dynamic_test_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_test_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_dynamic_test_create_srv_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_test_init_srv_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_dynamic_test_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_test_init_loc_conf(ngx_conf_t *cf, void *conf);

#define MAXBUFSIZE  4096

typedef struct {
    ngx_uint_t                  mi;
    ngx_str_t                   ms;
} ngx_http_dynamic_test_main_conf_t;

typedef struct {
    ngx_uint_t                  si;
    ngx_str_t                   ss;
} ngx_http_dynamic_test_srv_conf_t;

typedef struct {
    ngx_uint_t                  li;
    ngx_str_t                   ls;
} ngx_http_dynamic_test_loc_conf_t;


static ngx_command_t  ngx_http_dynamic_test_commands[] = {

    { ngx_string("http_dynamic_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_dynamic_test,
      0,
      0,
      NULL },

      ngx_null_command
};

static ngx_http_module_t  ngx_http_dynamic_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};

static ngx_command_t  ngx_http_dynamic_test_dcommands[] = {

    { ngx_string("main_int"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_http_dynamic_test_main_conf_t, mi),
      NULL },

    { ngx_string("main_str"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      0,
      offsetof(ngx_http_dynamic_test_main_conf_t, ms),
      NULL },

    { ngx_string("srv_int"),
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_http_dynamic_test_srv_conf_t, si),
      NULL },

    { ngx_string("srv_str"),
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      0,
      offsetof(ngx_http_dynamic_test_srv_conf_t, ss),
      NULL },

    { ngx_string("loc_int"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_http_dynamic_test_loc_conf_t, li),
      NULL },

    { ngx_string("loc_str"),
      NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      0,
      offsetof(ngx_http_dynamic_test_loc_conf_t, ls),
      NULL },

      ngx_null_command
};

static ngx_http_dynamic_module_t  ngx_http_dynamic_test_module_dctx = {
    ngx_http_dynamic_test_create_main_conf, /* create main configuration */
    ngx_http_dynamic_test_init_main_conf,   /* init main configuration */

    ngx_http_dynamic_test_create_srv_conf,  /* create server configuration */
    ngx_http_dynamic_test_init_srv_conf,    /* init server configuration */

    ngx_http_dynamic_test_create_loc_conf,  /* create location configuration */
    ngx_http_dynamic_test_init_loc_conf     /* init location configuration */
};

ngx_module_t  ngx_http_dynamic_test_module = {
    NGX_MODULE_V1,
    &ngx_http_dynamic_test_module_ctx,      /* module context */
    ngx_http_dynamic_test_commands,         /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_http_dynamic_test_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_http_dynamic_test_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void *
ngx_http_dynamic_test_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_test_main_conf_t      *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_test_main_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->mi = NGX_CONF_UNSET_UINT;

    return conf;
}

static char *
ngx_http_dynamic_test_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_dynamic_test_main_conf_t      *hdtmcf;

    hdtmcf = conf;

    ngx_conf_init_uint_value(hdtmcf->mi, 100);

    if (hdtmcf->ms.len == 0) {
        ngx_str_set(&hdtmcf->ms, "test for ms");
    }

    return NGX_CONF_OK;
}

static void *
ngx_http_dynamic_test_create_srv_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_test_srv_conf_t       *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_test_srv_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->si = NGX_CONF_UNSET_UINT;

    return conf;
}

static char *
ngx_http_dynamic_test_init_srv_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_dynamic_test_srv_conf_t       *hdtscf;

    hdtscf = conf;

    ngx_conf_init_uint_value(hdtscf->si, 100);

    if (hdtscf->ss.len == 0) {
        ngx_str_set(&hdtscf->ss, "test for ss");
    }

    return NGX_CONF_OK;
}

static void *
ngx_http_dynamic_test_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_test_loc_conf_t       *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_test_loc_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->li = NGX_CONF_UNSET_UINT;

    return conf;
}

static char *
ngx_http_dynamic_test_init_loc_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_dynamic_test_loc_conf_t       *hdtlcf;

    hdtlcf = conf;

    ngx_conf_init_uint_value(hdtlcf->li, 100);

    if (hdtlcf->ls.len == 0) {
        ngx_str_set(&hdtlcf->ls, "test for ls");
    }

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_dynamic_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                             cl;
    ngx_buf_t                              *b;
    ngx_http_dynamic_test_main_conf_t      *hdtmcf;
    ngx_http_dynamic_test_srv_conf_t       *hdtscf;
    ngx_http_dynamic_test_loc_conf_t       *hdtlcf;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "http dynamic test handler");

    b = ngx_create_temp_buf(r->pool, MAXBUFSIZE);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    hdtmcf = ngx_http_get_module_main_dconf(r, &ngx_http_dynamic_test_module);
    if (hdtmcf == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic conf test handler, dynamic main conf not configured");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    hdtscf = ngx_http_get_module_srv_dconf(r, &ngx_http_dynamic_test_module);
    if (hdtscf == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic conf test handler, dynamic srv conf not configured");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    hdtlcf = ngx_http_get_module_loc_dconf(r, &ngx_http_dynamic_test_module);
    if (hdtlcf == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic conf test handler, dynamic loc conf not configured");
    }

    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);

    b->last = ngx_snprintf(b->last, MAXBUFSIZE,
            "dynamic core test mi:%ui ms:%V\n", hdtmcf->mi, &hdtmcf->ms);
    b->last = ngx_snprintf(b->last, MAXBUFSIZE,
            "dynamic core test si:%ui ss:%V\n", hdtscf->si, &hdtscf->ss);
    if (hdtlcf) {
        b->last = ngx_snprintf(b->last, MAXBUFSIZE,
                "dynamic core test li:%ui ls:%V\n", hdtlcf->li, &hdtlcf->ls);
    }
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send body");

    return ngx_http_output_filter(r, &cl);
}

static char *
ngx_http_dynamic_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_dynamic_test_handler;

    return NGX_CONF_OK;
}
