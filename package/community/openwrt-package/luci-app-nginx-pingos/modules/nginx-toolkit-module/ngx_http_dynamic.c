/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_dynamic_conf.h"
#include "ngx_http_dynamic.h"


static void *ngx_http_dynamic_create_conf(ngx_conf_t *cf);
static void *ngx_http_dynamic_core_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_core_init_main_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_dynamic_core_create_srv_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_core_init_srv_conf(ngx_conf_t *cf, void *conf);
static void *ngx_http_dynamic_core_create_loc_conf(ngx_conf_t *cf);
static char *ngx_http_dynamic_core_init_loc_conf(ngx_conf_t *cf, void *conf);

static char *ngx_http_dynamic_block(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_http_dynamic_core_server(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);
static char *ngx_http_dynamic_core_server_name(ngx_conf_t *cf,
       ngx_command_t *cmd, void *conf);
static char *ngx_http_dynamic_core_location(ngx_conf_t *cf, ngx_command_t *cmd,
       void *conf);


typedef struct ngx_http_dynamic_location_tree_node_s
               ngx_http_dynamic_location_tree_node_t;
typedef struct ngx_http_dynamic_core_loc_conf_s
               ngx_http_dynamic_core_loc_conf_t;

struct ngx_http_dynamic_core_loc_conf_s {
    ngx_str_t                           name;       /* location name */

#if (NGX_PCRE)
    ngx_dynamic_regex_t                *regex;
#endif

    unsigned                            exact_match:1;
    unsigned                            noregex:1;

    ngx_queue_t                        *locations;

    ngx_http_dynamic_location_tree_node_t  *static_locations;
#if (NGX_PCRE)
    ngx_http_dynamic_core_loc_conf_t  **regex_locations;
#endif

    void                              **loc_conf;
};

typedef struct {
    ngx_queue_t                         queue;
    ngx_http_dynamic_core_loc_conf_t   *exact;
    ngx_http_dynamic_core_loc_conf_t   *inclusive;
    ngx_str_t                          *name;
    u_char                             *file_name;
    ngx_uint_t                          line;
    ngx_queue_t                         list;
} ngx_http_dynamic_location_queue_t;

struct ngx_http_dynamic_location_tree_node_s {
    ngx_http_dynamic_location_tree_node_t  *left;
    ngx_http_dynamic_location_tree_node_t  *right;
    ngx_http_dynamic_location_tree_node_t  *tree;

    ngx_http_dynamic_core_loc_conf_t   *exact;
    ngx_http_dynamic_core_loc_conf_t   *inclusive;

    u_char                              len;
    u_char                              name[1];
};


typedef struct {
    /* array of the ngx_http_dynamic_server_name_t, "server_name" directive */
    ngx_array_t                         server_names;

    ngx_http_dynamic_core_loc_conf_t   *hdclcf;

    void                              **srv_conf;
} ngx_http_dynamic_core_srv_conf_t;

typedef struct {
#if (NGX_PCRE)
    ngx_dynamic_regex_t                *regex;
#endif
    ngx_http_dynamic_core_srv_conf_t   *server;   /* virtual name server conf */
    ngx_str_t                           name;
} ngx_http_dynamic_server_name_t;


typedef struct {
    /* ngx_http_dynamic_srv_conf_t */
    ngx_array_t                         servers;

    ngx_uint_t                          server_names_hash_max_size;
    ngx_uint_t                          server_names_hash_bucket_size;

    /* the default server configuration for this address:port */
    ngx_http_dynamic_core_srv_conf_t   *default_server;

    ngx_hash_combined_t                 names;

#if (NGX_PCRE)
    ngx_uint_t                          nregex;
    ngx_http_dynamic_server_name_t     *regex;
#endif
} ngx_http_dynamic_core_main_conf_t;


typedef struct {
    void                              **main_conf;
} ngx_http_dynamic_conf_t;


static ngx_core_module_t  ngx_http_dynamic_module_ctx = {
    ngx_string("http_dynamic"),
    NULL,
    NULL
};

static ngx_command_t  ngx_http_dynamic_dcommands[] = {

    { ngx_string("http"),
      NGX_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_http_dynamic_block,
      0,
      0,
      NULL },

      ngx_null_command
};

static ngx_dynamic_core_module_t  ngx_http_dynamic_module_dctx = {
    ngx_string("http_dynamic"),
    ngx_http_dynamic_create_conf,
    NULL
};

ngx_module_t  ngx_http_dynamic_module = {
    NGX_MODULE_V1,
    &ngx_http_dynamic_module_ctx,           /* module context */
    NULL,                                   /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_http_dynamic_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_http_dynamic_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static ngx_http_module_t  ngx_http_dynamic_core_module_ctx = {
    NULL,                                   /* preconfiguration */
    NULL,                                   /* postconfiguration */

    NULL,                                   /* create main configuration */
    NULL,                                   /* init main configuration */

    NULL,                                   /* create server configuration */
    NULL,                                   /* merge server configuration */

    NULL,                                   /* create location configuration */
    NULL                                    /* merge location configuration */
};


static ngx_command_t  ngx_http_dynamic_core_dcommands[] = {

    { ngx_string("server_names_hash_max_size"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_http_dynamic_core_main_conf_t, server_names_hash_max_size),
      NULL },

    { ngx_string("server_names_hash_bucket_size"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_TAKE1,
      ngx_conf_set_num_slot,
      0,
      offsetof(ngx_http_dynamic_core_main_conf_t,
               server_names_hash_bucket_size),
      NULL },

    { ngx_string("server"),
      NGX_HTTP_MAIN_CONF|NGX_CONF_BLOCK|NGX_CONF_NOARGS,
      ngx_http_dynamic_core_server,
      0,
      0,
      NULL },

    { ngx_string("server_name"),
      NGX_HTTP_SRV_CONF|NGX_CONF_1MORE,
      ngx_http_dynamic_core_server_name,
      0,
      0,
      NULL },

    { ngx_string("location"),
      NGX_HTTP_SRV_CONF|NGX_CONF_BLOCK|NGX_CONF_TAKE12,
      ngx_http_dynamic_core_location,
      0,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_dynamic_module_t  ngx_http_dynamic_core_module_dctx = {
    ngx_http_dynamic_core_create_main_conf, /* create main configuration */
    ngx_http_dynamic_core_init_main_conf,   /* init main configuration */

    ngx_http_dynamic_core_create_srv_conf,  /* create server configuration */
    ngx_http_dynamic_core_init_srv_conf,    /* init server configuration */

    ngx_http_dynamic_core_create_loc_conf,  /* create location configuration */
    ngx_http_dynamic_core_init_loc_conf     /* init location configuration */
};


ngx_module_t  ngx_http_dynamic_core_module = {
    NGX_MODULE_V1,
    &ngx_http_dynamic_core_module_ctx,      /* module context */
    NULL,                                   /* module directives */
    NGX_HTTP_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    (uintptr_t) &ngx_http_dynamic_core_module_dctx, /* module dynamic context */
    (uintptr_t) ngx_http_dynamic_core_dcommands, /* module dynamic directives */
    NGX_MODULE_V1_DYNAMIC_PADDING
};


static void *
ngx_http_dynamic_create_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_conf_t                *hdccf;

    hdccf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_conf_t));
    if (hdccf == NULL) {
        return NULL;
    }

    return hdccf;
}

static void *
ngx_http_dynamic_core_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_core_main_conf_t      *hdcmcf;

    hdcmcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_core_main_conf_t));
    if (hdcmcf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&hdcmcf->servers, cf->pool, 4,
        sizeof(ngx_http_dynamic_core_srv_conf_t *)) != NGX_OK)
    {
        return NULL;
    }

    hdcmcf->server_names_hash_max_size = NGX_CONF_UNSET_UINT;
    hdcmcf->server_names_hash_bucket_size = NGX_CONF_UNSET_UINT;

    return hdcmcf;
}

static char *
ngx_http_dynamic_core_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_http_dynamic_core_main_conf_t      *hdcmcf;

    hdcmcf = conf;

    ngx_conf_init_uint_value(hdcmcf->server_names_hash_max_size, 512);
    ngx_conf_init_uint_value(hdcmcf->server_names_hash_bucket_size,
                             ngx_cacheline_size);

    return NGX_CONF_OK;
}

static void *
ngx_http_dynamic_core_create_srv_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;

    hdcscf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_core_srv_conf_t));
    if (hdcscf == NULL) {
        return NULL;
    }

    if (ngx_array_init(&hdcscf->server_names, cf->pool, 4,
        sizeof(ngx_http_dynamic_server_name_t)) != NGX_OK)
    {
        return NULL;
    }

    return hdcscf;
}

static char *
ngx_http_dynamic_core_init_srv_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static void *
ngx_http_dynamic_core_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_dynamic_core_loc_conf_t       *hdclcf;

    hdclcf = ngx_pcalloc(cf->pool, sizeof(ngx_http_dynamic_core_loc_conf_t));
    if (hdclcf == NULL) {
        return NULL;
    }

    return hdclcf;
}

static char *
ngx_http_dynamic_core_init_loc_conf(ngx_conf_t *cf, void *conf)
{
    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_dynamic_core_regex_location(ngx_conf_t *cf,
        ngx_http_dynamic_core_loc_conf_t *hdclcf,
        ngx_str_t *regex, ngx_uint_t caseless)
{
#if (NGX_PCRE)
    ngx_regex_compile_t                     rc;
    u_char                                  errstr[NGX_MAX_CONF_ERRSTR];

    ngx_memzero(&rc, sizeof(ngx_regex_compile_t));

    rc.pattern = *regex;
    rc.err.len = NGX_MAX_CONF_ERRSTR;
    rc.err.data = errstr;

#if (NGX_HAVE_CASELESS_FILESYSTEM)
    rc.options = NGX_REGEX_CASELESS;
#else
    rc.options = caseless ? NGX_REGEX_CASELESS : 0;
#endif

    hdclcf->regex = ngx_dynamic_regex_compile(cf, &rc);
    if (hdclcf->regex == NULL) {
        return NGX_ERROR;
    }

    hdclcf->name = *regex;

    return NGX_OK;

#else

    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
            "using regex \"%V\" requires PCRE library",
            regex);
    return NGX_ERROR;

#endif
}

static ngx_int_t
ngx_http_dynamic_core_predeal_location(ngx_conf_t *cf,
        ngx_http_dynamic_core_loc_conf_t *hdclcf)
{
    ngx_str_t                              *value, *name;
    size_t                                  len;
    u_char                                 *mod;

    value = cf->args->elts;

    if (cf->args->nelts == 3) {

        len = value[1].len;
        mod = value[1].data;
        name = &value[2];

        if (len == 1 && mod[0] == '=') {

            hdclcf->name = *name;
            hdclcf->exact_match = 1;

        } else if (len == 2 && mod[0] == '^' && mod[1] == '~') {

            hdclcf->name = *name;
            hdclcf->noregex = 1;

        } else if (len == 1 && mod[0] == '~') {

            if (ngx_http_dynamic_core_regex_location(cf, hdclcf, name, 0)
                != NGX_OK)
            {
                return NGX_ERROR;
            }

        } else if (len == 2 && mod[0] == '~' && mod[1] == '*') {

            if (ngx_http_dynamic_core_regex_location(cf, hdclcf, name, 1)
                != NGX_OK)
            {
                return NGX_ERROR;
            }

        } else {
            ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                               "invalid location modifier \"%V\"", &value[1]);
            return NGX_ERROR;
        }
    } else { /* cf->args->nelts == 2 */

        name = &value[1];

        if (name->data[0] == '=') {

            hdclcf->name.len = name->len - 1;
            hdclcf->name.data = name->data + 1;
            hdclcf->exact_match = 1;

        } else if (name->data[0] == '^' && name->data[1] == '~') {

            hdclcf->name.len = name->len - 2;
            hdclcf->name.data = name->data + 2;
            hdclcf->noregex = 1;

        } else if (name->data[0] == '~') {

            name->len--;
            name->data++;

            if (name->data[0] == '*') {

                name->len--;
                name->data++;

                if (ngx_http_dynamic_core_regex_location(cf, hdclcf, name, 1)
                    != NGX_OK)
                {
                    return NGX_ERROR;
                }

            } else {
                if (ngx_http_dynamic_core_regex_location(cf, hdclcf, name, 0)
                    != NGX_OK)
                {
                    return NGX_ERROR;
                }
            }

        } else {

            hdclcf->name = *name;
        }
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_dynamic_core_cmp_locations(const ngx_queue_t *one,
        const ngx_queue_t *two)
{
    ngx_int_t                               rc;
    ngx_http_dynamic_core_loc_conf_t       *first, *second;
    ngx_http_dynamic_location_queue_t      *lq1, *lq2;

    lq1 = (ngx_http_dynamic_location_queue_t *) one;
    lq2 = (ngx_http_dynamic_location_queue_t *) two;

    first = lq1->exact ? lq1->exact : lq1->inclusive;
    second = lq2->exact ? lq2->exact : lq2->inclusive;

#if (NGX_PCRE)
    if (first->regex && !second->regex) {
        /* shift the regex matches to the end */
        return 1;
    }

    if (!first->regex && second->regex) {
        /* shift the regex matches to the end */
        return -1;
    }

    if (first->regex || second->regex) {
        /* do not sort the regex matches */
        return 0;
    }
#endif

    rc = ngx_filename_cmp(first->name.data, second->name.data,
            ngx_min(first->name.len, second->name.len) + 1);

    if (rc == 0 && !first->exact_match && second->exact_match) {
        /* an exact match must be before the same inclusive one */
        return 1;
    }

    return rc;
}

static ngx_int_t
ngx_http_dynamic_core_join_exact_locations(ngx_conf_t *cf,
        ngx_queue_t *locations)
{
    ngx_queue_t                            *q, *x;
    ngx_http_dynamic_location_queue_t      *lq, *lx;

    q = ngx_queue_head(locations);

    while (q != ngx_queue_last(locations)) {

        x = ngx_queue_next(q);

        lq = (ngx_http_dynamic_location_queue_t *) q;
        lx = (ngx_http_dynamic_location_queue_t *) x;

        if (lq->name->len == lx->name->len
            && ngx_filename_cmp(lq->name->data, lx->name->data, lx->name->len)
               == 0)
        {
            if ((lq->exact && lx->exact) || (lq->inclusive && lx->inclusive)) {
                ngx_log_error(NGX_LOG_EMERG, cf->log, 0,
                              "duplicate location \"%V\" in %s:%ui",
                              lx->name, lx->file_name, lx->line);

                return NGX_ERROR;
            }

            lq->inclusive = lx->inclusive;

            ngx_queue_remove(x);

            continue;
        }

        q = ngx_queue_next(q);
    }

    return NGX_OK;
}

static void
ngx_http_dynamic_core_create_locations_list(ngx_queue_t *locations,
        ngx_queue_t *q)
{
    u_char                                 *name;
    size_t                                  len;
    ngx_queue_t                            *x, tail;
    ngx_http_dynamic_location_queue_t      *lq, *lx;

    if (q == ngx_queue_last(locations)) {
        return;
    }

    lq = (ngx_http_dynamic_location_queue_t *) q;

    if (lq->inclusive == NULL) {
        ngx_http_dynamic_core_create_locations_list(locations,
                                                    ngx_queue_next(q));
        return;
    }

    len = lq->name->len;
    name = lq->name->data;

    for (x = ngx_queue_next(q);
         x != ngx_queue_sentinel(locations);
         x = ngx_queue_next(x))
    {
        lx = (ngx_http_dynamic_location_queue_t *) x;

        if (len > lx->name->len
            || ngx_filename_cmp(name, lx->name->data, len) != 0)
        {
            break;
        }
    }

    q = ngx_queue_next(q);

    if (q == x) {
        ngx_http_dynamic_core_create_locations_list(locations, x);
        return;
    }

    ngx_queue_split(locations, q, &tail);
    ngx_queue_add(&lq->list, &tail);

    if (x == ngx_queue_sentinel(locations)) {
        ngx_http_dynamic_core_create_locations_list(&lq->list,
                                                    ngx_queue_head(&lq->list));
        return;
    }

    ngx_queue_split(&lq->list, x, &tail);
    ngx_queue_add(locations, &tail);

    ngx_http_dynamic_core_create_locations_list(&lq->list,
                                                ngx_queue_head(&lq->list));

    ngx_http_dynamic_core_create_locations_list(locations, x);
}

/*
 * to keep cache locality for left leaf nodes, allocate nodes in following
 * order: node, left subtree, right subtree, inclusive subtree
 */

static ngx_http_dynamic_location_tree_node_t *
ngx_http_dynamic_core_create_locations_tree(ngx_conf_t *cf,
        ngx_queue_t *locations, size_t prefix)
{
    size_t                                  len;
    ngx_queue_t                            *q, tail;
    ngx_http_dynamic_location_queue_t      *lq;
    ngx_http_dynamic_location_tree_node_t  *node;

    q = ngx_queue_middle(locations);

    lq = (ngx_http_dynamic_location_queue_t *) q;
    len = lq->name->len - prefix;

    node = ngx_palloc(cf->pool,
                offsetof(ngx_http_dynamic_location_tree_node_t, name) + len);
    if (node == NULL) {
        return NULL;
    }

    node->left = NULL;
    node->right = NULL;
    node->tree = NULL;
    node->exact = lq->exact;
    node->inclusive = lq->inclusive;

    node->len = (u_char) len;
    ngx_memcpy(node->name, &lq->name->data[prefix], len);

    ngx_queue_split(locations, q, &tail);

    if (ngx_queue_empty(locations)) {
        /*
         * ngx_queue_split() insures that if left part is empty,
         * then right one is empty too
         */
        goto inclusive;
    }

    node->left = ngx_http_dynamic_core_create_locations_tree(cf, locations,
                                                             prefix);
    if (node->left == NULL) {
        return NULL;
    }

    ngx_queue_remove(q);

    if (ngx_queue_empty(&tail)) {
        goto inclusive;
    }

    node->right = ngx_http_dynamic_core_create_locations_tree(cf, &tail,
                                                              prefix);
    if (node->right == NULL) {
        return NULL;
    }

inclusive:

    if (ngx_queue_empty(&lq->list)) {
        return node;
    }

    node->tree = ngx_http_dynamic_core_create_locations_tree(cf, &lq->list,
                                                             prefix + len);
    if (node->tree == NULL) {
        return NULL;
    }

    return node;
}

static ngx_int_t
ngx_http_dynamic_core_init_locations(ngx_conf_t *cf,
        ngx_http_dynamic_core_loc_conf_t *phdclcf)
{
    ngx_queue_t                            *q, *locations, tail;
    ngx_http_dynamic_core_loc_conf_t       *hdclcf;
    ngx_http_dynamic_location_queue_t      *lq;
    ngx_http_dynamic_core_loc_conf_t      **hdclcfp;
#if (NGX_PCRE)
    ngx_uint_t                              r;
    ngx_queue_t                            *regex;
#endif

    locations = phdclcf->locations;

    if (locations == NULL) {
        return NGX_OK;
    }

    ngx_queue_sort(locations, ngx_http_dynamic_core_cmp_locations);

#if (NGX_PCRE)
    regex = NULL;
    r = 0;
#endif

    for (q = ngx_queue_head(locations);
         q != ngx_queue_sentinel(locations);
         q = ngx_queue_next(q))
    {
        lq = (ngx_http_dynamic_location_queue_t *) q;

        hdclcf = lq->exact ? lq->exact : lq->inclusive;

#if (NGX_PCRE)

        if (hdclcf->regex) {
            r++;

            if (regex == NULL) {
                regex = q;
            }

            continue;
        }

#endif
    }

    if (q != ngx_queue_sentinel(locations)) {
        ngx_queue_split(locations, q, &tail);
    }

#if (NGX_PCRE)

    if (regex) {

        hdclcfp = ngx_palloc(cf->pool,
                    (r + 1) * sizeof(ngx_http_dynamic_core_loc_conf_t *));
        if (hdclcfp == NULL) {
            return NGX_ERROR;
        }

        phdclcf->regex_locations = hdclcfp;

        for (q = regex;
             q != ngx_queue_sentinel(locations);
             q = ngx_queue_next(q))
        {
            lq = (ngx_http_dynamic_location_queue_t *) q;

            *(hdclcfp++) = lq->exact;
        }

        *hdclcfp = NULL;

        ngx_queue_split(locations, regex, &tail);
    }

#endif

    return NGX_OK;
}

static ngx_int_t
ngx_http_dynamic_core_init_static_location_trees(ngx_conf_t *cf,
    	ngx_http_dynamic_core_loc_conf_t *phdclcf)
{
    ngx_queue_t                            *locations;

    locations = phdclcf->locations;

    if (locations == NULL) {
        return NGX_OK;
    }

    if (ngx_queue_empty(locations)) {
        return NGX_OK;
    }

    if (ngx_http_dynamic_core_join_exact_locations(cf, locations) != NGX_OK) {
        return NGX_ERROR;
    }

    ngx_http_dynamic_core_create_locations_list(locations,
                                                ngx_queue_head(locations));

    phdclcf->static_locations =
        ngx_http_dynamic_core_create_locations_tree(cf, locations, 0);
    if (phdclcf->static_locations == NULL) {
        return NGX_ERROR;
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_dynamic_core_add_location(ngx_conf_t *cf, ngx_queue_t **locations,
        ngx_http_dynamic_core_loc_conf_t *hdclcf)
{
    ngx_http_dynamic_location_queue_t              *lq;

    if (ngx_http_dynamic_core_predeal_location(cf, hdclcf) != NGX_OK) {
        return NGX_ERROR;
    }

    if (*locations == NULL) {
        *locations = ngx_pcalloc(cf->temp_pool,
                                 sizeof(ngx_http_location_queue_t));
        if (*locations == NULL) {
            return NGX_ERROR;
        }

        ngx_queue_init(*locations);
    }

    lq = ngx_pcalloc(cf->temp_pool, sizeof(ngx_http_location_queue_t));
    if (lq == NULL) {
        return NGX_ERROR;
    }

    if (hdclcf->exact_match
#if (NGX_PCRE)
        || hdclcf->regex
#endif
        )
    {
        lq->exact = hdclcf;
        lq->inclusive = NULL;
    } else {
        lq->exact = NULL;
        lq->inclusive = hdclcf;
    }

    lq->name = &hdclcf->name;
    lq->file_name = cf->conf_file->file.name.data;
    lq->line = cf->conf_file->line;

    ngx_queue_init(&lq->list);

    ngx_queue_insert_tail(*locations, &lq->queue);

    return NGX_OK;
}

/*
 * NGX_OK       - exact or regex match
 * NGX_AGAIN    - inclusive match
 * NGX_DECLINED - no match
 */

static ngx_int_t
ngx_http_dynamic_core_find_static_location(ngx_http_request_t *r,
        ngx_http_dynamic_core_loc_conf_t *phdclcf,
        ngx_http_dynamic_core_loc_conf_t **hdclcf)
{
    ngx_http_dynamic_location_tree_node_t  *node;
    u_char                                 *uri;
    size_t                                  len, n;
    ngx_int_t                               rc, rv;

    len = r->uri.len;
    uri = r->uri.data;

    rv = NGX_DECLINED;

    node = phdclcf->static_locations;

    for ( ;; ) {

        if (node == NULL) {
            return rv;
        }

        ngx_log_debug2(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                       "test location: \"%*s\"",
                       (size_t) node->len, node->name);

        n = (len <= (size_t) node->len) ? len : node->len;

        rc = ngx_filename_cmp(uri, node->name, n);

        if (rc != 0) {
            node = (rc < 0) ? node->left : node->right;

            continue;
        }

        if (len > (size_t) node->len) {

            if (node->inclusive) {

                *hdclcf = node->inclusive->
                            loc_conf[ngx_http_dynamic_core_module.ctx_index];
                rv = NGX_AGAIN;

                node = node->tree;
                uri += n;
                len -= n;

                continue;
            }

            /* exact only */

            node = node->right;

            continue;
        }

        if (len == (size_t) node->len) {

            if (node->exact) {
                *hdclcf = node->exact->
                            loc_conf[ngx_http_dynamic_core_module.ctx_index];
                return NGX_OK;

            } else {
                *hdclcf = node->inclusive->
                            loc_conf[ngx_http_dynamic_core_module.ctx_index];
                return NGX_AGAIN;
            }
        }

        /* len < node->len */

        node = node->left;
    }
}

/*
 * NGX_OK       - exact or regex match
 * NGX_AGAIN    - inclusive match
 * NGX_ERROR    - regex error
 * NGX_DECLINED - no match
 */

static ngx_int_t
ngx_http_dynamic_core_find_location(ngx_http_request_t *r,
        ngx_http_dynamic_core_srv_conf_t *hdcscf,
        ngx_http_dynamic_core_loc_conf_t **hdclcf)
{
    ngx_int_t                               rc;
    ngx_http_dynamic_core_loc_conf_t       *phdclcf;
#if (NGX_PCRE)
    ngx_int_t                               n;
    ngx_uint_t                              noregex;
    ngx_http_dynamic_core_loc_conf_t      **hdclcfp;

    noregex = 0;
#endif

    phdclcf = hdcscf->hdclcf;

    rc = ngx_http_dynamic_core_find_static_location(r, phdclcf, hdclcf);

    if (rc == NGX_AGAIN) {

#if (NGX_PCRE)
        noregex = (*hdclcf)->noregex;
#endif

    }

    if (rc == NGX_OK || rc == NGX_DONE) {
        return rc;
    }

    /* rc == NGX_DECLINED or rc == NGX_AGAIN in nested location */

#if (NGX_PCRE)

    if (noregex == 0 && phdclcf->regex_locations) {

        for (hdclcfp = phdclcf->regex_locations; *hdclcfp; hdclcfp++) {

            ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                           "test location: ~ \"%V\"", &(*hdclcfp)->name);

            n = ngx_regex_exec((*hdclcfp)->regex->regex, &r->uri, NULL, 0);

            if (n == NGX_OK) {
                *hdclcf = (*hdclcfp)->
                            loc_conf[ngx_http_dynamic_core_module.ctx_index];

                return NGX_OK;
            }

            if (n == NGX_REGEX_NO_MATCHED) {
                continue;
            }

            return NGX_ERROR;
        }
    }
#endif

    return rc;
}

static ngx_int_t
ngx_http_dynamic_core_init_virtual_servers(ngx_conf_t *cf,
        ngx_http_dynamic_conf_t *hdcf)
{
    ngx_http_dynamic_core_main_conf_t      *hdcmcf;
    ngx_http_dynamic_core_srv_conf_t      **hdcscfp;
    ngx_hash_init_t                         hash;
    ngx_hash_keys_arrays_t                  ha;
    ngx_http_dynamic_server_name_t         *name;
    ngx_uint_t                              s, n;
    ngx_int_t                               rc;
#if (NGX_PCRE)
    ngx_uint_t                              regex, i;

    regex = 0;
#endif

    hdcmcf = hdcf->main_conf[ngx_http_dynamic_core_module.ctx_index];

    ngx_memzero(&ha, sizeof(ngx_hash_keys_arrays_t));

    ha.temp_pool = ngx_create_pool(NGX_DEFAULT_POOL_SIZE, cf->log);
    if (ha.temp_pool == NULL) {
        return NGX_ERROR;
    }

    ha.pool = cf->pool;

    if (ngx_hash_keys_array_init(&ha, NGX_HASH_LARGE) != NGX_OK) {
        goto failed;
    }

    hdcscfp = hdcmcf->servers.elts;
    hdcmcf->default_server = hdcscfp[0];

    for (s = 0; s < hdcmcf->servers.nelts; ++s) {

        name = hdcscfp[s]->server_names.elts;

        for (n = 0; n < hdcscfp[s]->server_names.nelts; ++n) {
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
    hash.max_size = hdcmcf->server_names_hash_max_size;
    hash.bucket_size = hdcmcf->server_names_hash_bucket_size;
    hash.name = "http_dynamic_server_names_hash";
    hash.pool = cf->pool;

    if (ha.keys.nelts) {
        hash.hash = &hdcmcf->names.hash;
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

        hdcmcf->names.wc_head = (ngx_hash_wildcard_t *) hash.hash;
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

        hdcmcf->names.wc_tail = (ngx_hash_wildcard_t *) hash.hash;

    }

    ngx_destroy_pool(ha.temp_pool);

#if (NGX_PCRE)
    if (regex == 0) {
        return NGX_OK;
    }

    hdcmcf->nregex = regex;
    hdcmcf->regex = ngx_pcalloc(cf->pool,
                                regex * sizeof(ngx_http_dynamic_server_name_t));
    if (hdcmcf->regex == NULL) {
        return NGX_ERROR;
    }

    i = 0;

    for (s = 0; s < hdcmcf->servers.nelts; ++s) {

        name = hdcscfp[s]->server_names.elts;

        for (n = 0; n < hdcscfp[s]->server_names.nelts; ++n) {
            if (name[n].regex) {
                hdcmcf->regex[i++] = name[n];
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
ngx_http_dynamic_core_find_virtual_server(ngx_http_request_t *r,
        ngx_http_dynamic_core_main_conf_t *hdcmcf,
        ngx_http_dynamic_core_srv_conf_t **hdcscfp)
{
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;
    ngx_str_t                              *server;

    server = &r->headers_in.server;

    hdcscf = ngx_hash_find_combined(&hdcmcf->names,
                                    ngx_hash_key(server->data, server->len),
                                    server->data, server->len);

    if (hdcscf) {
        *hdcscfp = hdcscf;
        return NGX_OK;
    }

#if (NGX_PCRE)

    if (server->len && hdcmcf->nregex) {
        ngx_int_t                           n;
        ngx_uint_t                          i;
        ngx_http_dynamic_server_name_t     *sn;

        sn = hdcmcf->regex;

        for (i = 0; i < hdcmcf->nregex; ++i) {

            n = ngx_regex_exec(sn[i].regex->regex, server, NULL, 0);

            if (n == NGX_REGEX_NO_MATCHED) {
                continue;
            }

            if (n >= 0) {
                *hdcscfp = sn[i].server;
                return NGX_OK;
            }

            return NGX_ERROR;
        }
    }
#endif

    return NGX_DECLINED;
}

static char *
ngx_http_dynamic_core_location(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_dynamic_module_t              *module;
    ngx_http_dynamic_core_loc_conf_t       *hdclcf, *phdclcf;
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;
    ngx_conf_t                              pcf;
    void                                   *mconf, **loc_conf;
    ngx_uint_t                              i, ci;
    char                                   *rv = NULL;

    hdcscf = conf;
    phdclcf = hdcscf->hdclcf;

    /* loc_conf is temp cause hdclcf does not created now */
    loc_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_http_max_module);
    if (loc_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;

        if (module == NULL) {
            continue;
        }

        if (module->create_loc_conf) {
            mconf = module->create_loc_conf(cf);
            if (mconf == NULL) {
                return NGX_CONF_ERROR;
            }

            loc_conf[cf->cycle->modules[i]->ctx_index] = mconf;
        }
    }

    hdclcf = loc_conf[ngx_http_dynamic_core_module.ctx_index];
    hdclcf->loc_conf = loc_conf;

    if (ngx_http_dynamic_core_add_location(cf, &phdclcf->locations, hdclcf)
        != NGX_OK)
    {
        return NGX_CONF_ERROR;
    }

    pcf = *cf;
    cf->ctx = hdclcf->loc_conf;
    cf->cmd_type = NGX_HTTP_LOC_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;
        if (module == NULL) {
            continue;
        }

        if (module->init_loc_conf) {
            rv = module->init_loc_conf(cf, hdclcf->loc_conf[ci]);
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
ngx_http_dynamic_core_server_name(ngx_conf_t *cf, ngx_command_t *cmd,
        void *conf)
{
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;
    u_char                                  ch;
    ngx_str_t                              *value;
    ngx_uint_t                              i;
    ngx_http_dynamic_server_name_t         *sn;

    hdcscf = conf;

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

        sn = ngx_array_push(&hdcscf->server_names);
        if (sn == NULL) {
            return NGX_CONF_ERROR;
        }

#if (NGX_PCRE)
        sn->regex = NULL;
#endif
        sn->server = hdcscf;

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
ngx_http_dynamic_core_server(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_dynamic_module_t              *module;
    ngx_http_dynamic_core_main_conf_t      *hdcmcf;
    ngx_http_dynamic_core_srv_conf_t       *hdcscf, **hdcscfp;
    ngx_http_dynamic_core_loc_conf_t       *phdclcf;
    ngx_conf_t                              pcf;
    void                                   *mconf, **srv_conf;
    ngx_uint_t                              i, ci;
    char                                   *rv = NULL;

    hdcmcf = conf;

    /* srv_conf is temp cause hdcscf does not created now */
    srv_conf = ngx_pcalloc(cf->pool, sizeof(void *) * ngx_http_max_module);
    if (srv_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
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

    hdcscf = srv_conf[ngx_http_dynamic_core_module.ctx_index];
    hdcscf->srv_conf = srv_conf;

    hdcscf->hdclcf = ngx_pcalloc(cf->pool,
                                 sizeof(ngx_http_dynamic_core_loc_conf_t));

    /* save hdcscf into hdcmcf */
    hdcscfp = ngx_array_push(&hdcmcf->servers);
    if (hdcscfp == NULL) {
        return NGX_CONF_ERROR;
    }
    *hdcscfp = hdcscf;

    pcf = *cf;
    cf->ctx = hdcscf->srv_conf;
    cf->cmd_type = NGX_HTTP_SRV_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;
        if (module == NULL) {
            continue;
        }

        if (module->init_srv_conf) {
            rv = module->init_srv_conf(cf, hdcscf->srv_conf[ci]);
            if (rv != NGX_CONF_OK) {
                goto failed;
            }
        }
    }

    *cf = pcf;

    phdclcf = hdcscf->hdclcf;

    if (ngx_http_dynamic_core_init_locations(cf, phdclcf) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    if (ngx_http_dynamic_core_init_static_location_trees(cf, phdclcf) != NGX_OK) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;

failed:
    *cf = pcf;

    if (rv) {
        return rv;
    }

    return NGX_CONF_ERROR;

}

static char *
ngx_http_dynamic_block(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_dynamic_conf_t                *hdccf;
    ngx_http_dynamic_module_t              *module;
    ngx_conf_t                              pcf;
    ngx_uint_t                              i, ci;
    char                                   *rv = NULL;

    hdccf = conf;

    /* create main_conf ctx */
    hdccf->main_conf = ngx_pcalloc(cf->pool, sizeof(void *)
                                   * ngx_http_max_module);
    if (hdccf->main_conf == NULL) {
        return NGX_CONF_ERROR;
    }

    /* create http dynamic conf for all http module */
    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;

        if (module == NULL) {
            continue;
        }

        if (module->create_main_conf) {
            hdccf->main_conf[ci] = module->create_main_conf(cf);
            if (hdccf->main_conf[ci] == NULL) {
                return NGX_CONF_ERROR;
            }
        }
    }

    /* save conf for recovery */
    pcf = *cf;

    cf->ctx = hdccf->main_conf;
    cf->module_type = NGX_HTTP_MODULE;
    cf->cmd_type = NGX_HTTP_MAIN_CONF;

    if (ngx_dynamic_conf_parse(cf, 0) != NGX_OK) {
        goto failed;
    }

    for (i = 0; cf->cycle->modules[i]; ++i) {
        if (cf->cycle->modules[i]->type != NGX_HTTP_MODULE) {
            continue;
        }

        module = (ngx_http_dynamic_module_t *)
                    cf->cycle->modules[i]->spare_hook0;
        ci = cf->cycle->modules[i]->ctx_index;

        if (module == NULL) {
            continue;
        }

        if (module->init_main_conf) {
            rv = module->init_main_conf(cf, hdccf->main_conf[ci]);
            if (rv != NGX_CONF_OK) {
                goto failed;
            }
        }
    }

    if (ngx_http_dynamic_core_init_virtual_servers(cf, hdccf) != NGX_OK) {
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
ngx_http_get_module_main_dconf(ngx_http_request_t *r, ngx_module_t *m)
{
    ngx_http_dynamic_conf_t                *hdcf;

    hdcf = ngx_get_dconf(&ngx_http_dynamic_module);
    if (hdcf == NULL) {
        return NULL;
    }

    return hdcf->main_conf[m->ctx_index];
}

void *
ngx_http_get_module_srv_dconf(ngx_http_request_t *r, ngx_module_t *m)
{
    ngx_http_dynamic_core_main_conf_t      *hdcmcf;
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;
    ngx_int_t                               rc;

    hdcmcf = ngx_http_get_module_main_dconf(r, &ngx_http_dynamic_core_module);
    if (hdcmcf == NULL) {
        return NULL;
    }

    rc = ngx_http_dynamic_core_find_virtual_server(r, hdcmcf, &hdcscf);
    switch (rc) {
    case NGX_ERROR:
        return NULL;
    case NGX_DECLINED:  /* virtual server not found */
        return hdcmcf->default_server->srv_conf[m->ctx_index];
    default:
        return hdcscf->srv_conf[m->ctx_index];
    }
}

void *
ngx_http_get_module_loc_dconf(ngx_http_request_t *r, ngx_module_t *m)
{
    ngx_http_dynamic_core_srv_conf_t       *hdcscf;
    ngx_http_dynamic_core_loc_conf_t       *hdclcf;
    ngx_int_t                               rc;

    hdcscf = ngx_http_get_module_srv_dconf(r, &ngx_http_dynamic_core_module);
    if (hdcscf == NULL) {
        return NULL;
    }

    hdclcf = NULL;
    rc = ngx_http_dynamic_core_find_location(r, hdcscf, &hdclcf);
    if (rc == NGX_ERROR) {
        return NULL;
    }

    if (hdclcf) {
        return hdclcf->loc_conf[m->ctx_index];
    }

    return NULL;
}
