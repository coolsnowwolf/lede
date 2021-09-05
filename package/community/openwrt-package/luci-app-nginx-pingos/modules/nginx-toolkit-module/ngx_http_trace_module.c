#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


typedef struct {
    ngx_flag_t                  trace;
} ngx_http_trace_main_conf_t;


typedef struct {
    u_char                      traceid[32]; /* X-NTM-Traceid */
    u_char                      cid[32];     /* X-NTM-Currentid */
    u_char                      pid[32];     /* X-NTM-Parentid */
    ngx_flag_t                  debug;       /* X-NTM-Debug */
} ngx_http_trace_ctx_t;


static void *ngx_http_trace_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_trace_init_main_conf(ngx_conf_t *cf, void *conf);

static char *ngx_http_trace(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_int_t ngx_http_trace_add_variables(ngx_conf_t *cf);
static ngx_int_t ngx_http_trace_init(ngx_conf_t *cf);

static ngx_int_t ngx_http_trace_traceid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_trace_currentid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_trace_parentid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_trace_debug_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);
static ngx_int_t ngx_http_trace_newid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data);


static ngx_command_t  ngx_http_trace_commands[] = {

    { ngx_string("http_trace"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_NOARGS,
      ngx_http_trace,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_trace_module_ctx = {
    ngx_http_trace_add_variables,           /* preconfiguration */
    ngx_http_trace_init,                    /* postconfiguration */

    ngx_http_trace_create_main_conf,        /* create main configuration */
    ngx_http_trace_init_main_conf,          /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


ngx_module_t  ngx_http_trace_module = {
    NGX_MODULE_V1,
    &ngx_http_trace_module_ctx,             /* module context */
    ngx_http_trace_commands,                /* module directives */
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


static ngx_http_variable_t  ngx_http_trace_vars[] = {

    { ngx_string("ntm_traceid"), NULL,
      ngx_http_trace_traceid_variable, 0, 0, 0 },

    { ngx_string("ntm_currentid"), NULL,
      ngx_http_trace_currentid_variable, 0, 0, 0 },

    { ngx_string("ntm_parentid"), NULL,
      ngx_http_trace_parentid_variable, 0, 0, 0 },

    { ngx_string("ntm_debug"), NULL,
      ngx_http_trace_debug_variable, 0, 0, 0 },

    { ngx_string("ntm_newid"), NULL,
      ngx_http_trace_newid_variable, 0, 0, 0 },

    { ngx_null_string, NULL, NULL, 0, 0, 0 }
};


static void
ngx_http_trace_genid(u_char *id) {
    ngx_sprintf(id, "%08xD%08xD%08xD%08xD",
                (uint32_t) ngx_random(), (uint32_t) ngx_random(),
                (uint32_t) ngx_random(), (uint32_t) ngx_random());
}


static u_char *
ngx_http_trace_log_error(ngx_log_t *log, u_char *buf, size_t len)
{
    u_char                         *p;
    ngx_http_request_t             *r;
    ngx_http_log_ctx_t             *ctx;
    ngx_http_trace_ctx_t           *tctx;
    ngx_str_t                       traceid, currentid, parentid;

    if (log->action) {
        p = ngx_snprintf(buf, len, " while %s", log->action);
        len -= p - buf;
        buf = p;
    }

    ctx = log->data;

    p = ngx_snprintf(buf, len, ", client: %V", &ctx->connection->addr_text);
    len -= p - buf;
    buf = p;

    r = ctx->request;

    if (r) {
        p = r->log_handler(r, ctx->current_request, p, len);
        len -= p - buf;
        buf = p;

    } else {
        p = ngx_snprintf(p, len, ", server: %V",
                         &ctx->connection->listening->addr_text);
        len -= p - buf;
        buf = p;
    }

    tctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (tctx) {
        traceid.data = tctx->traceid;
        traceid.len = sizeof(tctx->traceid);

        currentid.data = tctx->cid;
        currentid.len = sizeof(tctx->cid);

        parentid.data = tctx->pid;
        parentid.len = sizeof(tctx->pid);

        p = ngx_snprintf(p, len, ", [NGINX-TRACE] traceid: %V, currentid: %V, "
            "parentid: %V [NGINX-TRACE-END]",
            &traceid, &currentid, &parentid);
    }

    return p;
}


// First request without X-NTM-Traceid X-NTM-Currentid or X-NTM-Parentid
// Need to gen these ids and set X-NTM headers in main request
static ngx_int_t
ngx_http_trace_first_request(ngx_http_request_t *r, ngx_http_trace_ctx_t *ctx)
{
    ngx_table_elt_t                *h;

    ngx_http_trace_genid(ctx->traceid);
    ngx_http_trace_genid(ctx->cid);
    ngx_sprintf(ctx->pid, "00000000000000000000000000000000");

    // Set X-NTM-Traceid
    h = ngx_list_push(&r->headers_in.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->key.data = (u_char *) "X-NTM-Traceid";
    h->key.len = sizeof("X-NTM-Traceid") - 1;

    h->value.data = ctx->traceid;
    h->value.len = sizeof(ctx->traceid);

    h->lowcase_key = ngx_pnalloc(r->pool, h->key.len);
    if (h->lowcase_key == NULL) {
        return NGX_ERROR;
    }
    h->hash = ngx_hash_strlow(h->lowcase_key, h->key.data, h->key.len);

    // Set X-NTM-Currentid
    h = ngx_list_push(&r->headers_in.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->key.data = (u_char *) "X-NTM-Currentid";
    h->key.len = sizeof("X-NTM-Currentid") - 1;

    h->value.data = ctx->cid;
    h->value.len = sizeof(ctx->cid);

    h->lowcase_key = ngx_pnalloc(r->pool, h->key.len);
    if (h->lowcase_key == NULL) {
        return NGX_ERROR;
    }
    h->hash = ngx_hash_strlow(h->lowcase_key, h->key.data, h->key.len);

    // Set X-NTM-Parentid
    h = ngx_list_push(&r->headers_in.headers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    h->key.data = (u_char *) "X-NTM-Parentid";
    h->key.len = sizeof("X-NTM-Parentid") - 1;

    h->value.data = ctx->pid;
    h->value.len = sizeof(ctx->pid);

    h->lowcase_key = ngx_pnalloc(r->pool, h->key.len);
    if (h->lowcase_key == NULL) {
        return NGX_ERROR;
    }
    h->hash = ngx_hash_strlow(h->lowcase_key, h->key.data, h->key.len);

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_handler(ngx_http_request_t *r)
{
    ngx_http_trace_main_conf_t     *tmcf;
    ngx_http_trace_ctx_t           *ctx;
    ngx_http_variable_value_t       v;
    ngx_str_t                       header;

    if (r != r->main) { // subrequest
        return NGX_DECLINED;
    }

    tmcf = ngx_http_get_module_main_conf(r, ngx_http_trace_module);
    if (!tmcf->trace) {
        return NGX_DECLINED;
    }

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx) { // ctx already create, enter twice
        return NGX_DECLINED;
    }

    ctx = ngx_pcalloc(r->pool, sizeof(ngx_http_trace_ctx_t));
    if (ctx == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    ngx_http_set_ctx(r, ctx, ngx_http_trace_module);

    // traceid currentid parentid log into nginx error log
    r->connection->log->handler = ngx_http_trace_log_error;

    // Get X-NTM-Debug
    header.data = (u_char *) "http_x_ntm_debug";
    header.len = sizeof("http_x_ntm_debug") - 1;
    ngx_http_variable_unknown_header(&v, &header, &r->headers_in.headers.part,
                                     sizeof("http_") - 1);
    if (!v.not_found && v.len == 1 && v.data[0] == '1') {
        // Has header X-NTM-Debug: 1
        ctx->debug = 1;
    }

    // Get X-NTM-Traceid
    header.data = (u_char *) "http_x_ntm_traceid";
    header.len = sizeof("http_x_ntm_traceid") - 1;
    ngx_http_variable_unknown_header(&v, &header, &r->headers_in.headers.part,
                                     sizeof("http_") - 1);
    if (v.not_found) {
        goto notfound;
    }

    ngx_memcpy(ctx->traceid, v.data, ngx_min(v.len, sizeof(ctx->traceid)));

    // Get X-NTM-Currentid
    header.data = (u_char *) "http_x_ntm_currentid";
    header.len = sizeof("http_x_ntm_currentid") - 1;
    ngx_http_variable_unknown_header(&v, &header, &r->headers_in.headers.part,
                                     sizeof("http_") - 1);
    if (v.not_found) {
        goto notfound;
    }

    ngx_memcpy(ctx->cid, v.data, ngx_min(v.len, sizeof(ctx->cid)));

    // Get X-NTM-Parentid
    header.data = (u_char *) "http_x_ntm_parentid";
    header.len = sizeof("http_x_ntm_parentid") - 1;
    ngx_http_variable_unknown_header(&v, &header, &r->headers_in.headers.part,
                                     sizeof("http_") - 1);
    if (v.not_found) {
        goto notfound;
    }

    ngx_memcpy(ctx->pid, v.data, ngx_min(v.len, sizeof(ctx->pid)));

    return NGX_DECLINED;

notfound:

    if (ngx_http_trace_first_request(r, ctx) != NGX_OK) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    return NGX_DECLINED;
}


static ngx_int_t
ngx_http_trace_traceid_variable(ngx_http_request_t *r,
        ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_trace_ctx_t           *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->data = ctx->traceid;
    v->len = sizeof(ctx->traceid);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_currentid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_trace_ctx_t           *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->data = ctx->cid;
    v->len = sizeof(ctx->cid);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_parentid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_trace_ctx_t           *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    v->data = ctx->pid;
    v->len = sizeof(ctx->pid);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_debug_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_trace_ctx_t           *ctx;

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    if (ctx->debug) {
        v->data = (u_char *) "1";
    } else {
        v->data = (u_char *) "0";
    }

    v->len = sizeof("1") - 1;
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_newid_variable(ngx_http_request_t *r,
    ngx_http_variable_value_t *v, uintptr_t data)
{
    ngx_http_trace_ctx_t           *ctx;
    u_char                         *newid;

    ctx = ngx_http_get_module_ctx(r, ngx_http_trace_module);
    if (ctx == NULL) {
        v->not_found = 1;
        return NGX_OK;
    }

    newid = ngx_pcalloc(r->pool, sizeof(ctx->cid));
    if (newid == NULL) {
        return NGX_ERROR;
    }

    // fill newid
    ngx_http_trace_genid(newid);

    v->data = newid;
    v->len = sizeof(ctx->cid);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static void *
ngx_http_trace_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_trace_main_conf_t     *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_trace_main_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->trace = NGX_CONF_UNSET;

    return conf;
}


static char *
ngx_http_trace_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_trace_main_conf_t     *tmcf;

    tmcf = conf;

    ngx_conf_init_value(tmcf->trace, 0);

    return NGX_CONF_OK;
}


static char *
ngx_http_trace(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_trace_main_conf_t     *tmcf;

    tmcf = conf;

    if (tmcf->trace != NGX_CONF_UNSET) {
        return "is duplicate";
    }

    tmcf->trace = 1;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_trace_add_variables(ngx_conf_t *cf)
{
    ngx_http_variable_t  *var, *v;

    for (v = ngx_http_trace_vars; v->name.len; v++) {
        var = ngx_http_add_variable(cf, &v->name, v->flags);
        if (var == NULL) {
            return NGX_ERROR;
        }

        var->get_handler = v->get_handler;
        var->data = v->data;
    }

    return NGX_OK;
}


static ngx_int_t
ngx_http_trace_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_POST_READ_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }

    *h = ngx_http_trace_handler;

    return NGX_OK;
}
