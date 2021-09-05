#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_dynamic_conf.h"


/******************************************************************************/
static void *ngx_dynamic_core_test_create_conf(ngx_conf_t *cf);
static char *ngx_dynamic_core_test_init_conf(ngx_conf_t *cf, void *conf);


typedef struct {
    ngx_uint_t                  i;
    ngx_str_t                   s;
} ngx_dynamic_core_test_conf_t;


ngx_core_module_t  ngx_dynamic_core_test_module_ctx = {
    ngx_string("dynamic_core_test"),
    NULL,
    NULL
};

static ngx_command_t  ngx_dynamic_core_test_dcommands[] = {

    { ngx_string("dynamic_test_i"),
      NGX_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_dynamic_core_test_conf_t, i),
      NULL },

    { ngx_string("dynamic_test_s"),
      NGX_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_str_slot,
      0,
      offsetof(ngx_dynamic_core_test_conf_t, s),
      NULL },

      ngx_null_command
};


static ngx_dynamic_core_module_t ngx_dynamic_core_test_module_dctx = {
    ngx_string("dynamic_core_test"),
    ngx_dynamic_core_test_create_conf,
    ngx_dynamic_core_test_init_conf
};


ngx_module_t  ngx_dynamic_core_test_module = {
    NGX_MODULE_V1,
    &ngx_dynamic_core_test_module_ctx,      /* module context */
    NULL,                                   /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_dynamic_core_test_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_dynamic_core_test_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void *
ngx_dynamic_core_test_create_conf(ngx_conf_t *cf)
{
    ngx_dynamic_core_test_conf_t       *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_dynamic_core_test_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->i = NGX_CONF_UNSET_UINT;

    return conf;
}


static char *
ngx_dynamic_core_test_init_conf(ngx_conf_t *cf, void *conf)
{
    ngx_dynamic_core_test_conf_t       *dctcf;

    dctcf = conf;

    ngx_conf_init_uint_value(dctcf->i, 1000);

    return NGX_CONF_OK;
}
/******************************************************************************/


static char *ngx_dynamic_conf_test(ngx_conf_t *cf, ngx_command_t *cmd,
    void *conf);


#define MAXBUFSIZE  4096


static ngx_command_t  ngx_dynamic_conf_test_commands[] = {

    { ngx_string("dynamic_conf_test"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_dynamic_conf_test,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_dynamic_conf_test_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_dynamic_conf_test_module = {
    NGX_MODULE_V1,
    &ngx_dynamic_conf_test_module_ctx,      /* module context */
    ngx_dynamic_conf_test_commands,         /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_dynamic_conf_test_handler(ngx_http_request_t *r)
{
    ngx_chain_t                     cl;
    ngx_buf_t                      *b;
    ngx_dynamic_core_test_conf_t   *dctcf;

    ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0, "dynamic conf test handler");

    b = ngx_create_temp_buf(r->pool, MAXBUFSIZE);

    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    dctcf = (ngx_dynamic_core_test_conf_t *)
                ngx_get_dconf(&ngx_dynamic_core_test_module);
    if (dctcf == NULL) {
        ngx_log_error(NGX_LOG_ERR, ngx_cycle->log, 0,
                "dynamic conf test handler, dynamic conf not configured");
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    r->headers_out.status = NGX_HTTP_OK;

    ngx_http_send_header(r);

    b->last = ngx_snprintf(b->last, MAXBUFSIZE, "dynamic core test %ui [%V]\n",
                           dctcf->i, &dctcf->s);
    b->last_buf = 1;
    b->last_in_chain = 1;

    cl.buf = b;
    cl.next = NULL;

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "send body");

    return ngx_http_output_filter(r, &cl);
}


static char *
ngx_dynamic_conf_test(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_dynamic_conf_test_handler;

    return NGX_CONF_OK;
}
