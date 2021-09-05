/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_poold.h"
#include "ngx_map.h"


typedef struct ngx_poold_node_s ngx_poold_node_t;

static ngx_pool_t              *ngx_poold_pool;

static ngx_map_t                ngx_poold_map;
static ngx_poold_node_t        *ngx_poold_free_node;

static ngx_uint_t               ngx_poold_nalloc;
static ngx_uint_t               ngx_poold_nfree;


struct ngx_poold_node_s {
    ngx_map_node_t              m;      /* map node */
    ngx_poold_node_t           *next;   /* free node */

    ngx_pool_t                 *pool;
    char                       *file;   /* file create pool */
    int                         line;   /* line create pool */
};


static ngx_int_t
ngx_poold_init()
{
    ngx_poold_pool = ngx_create_pool(4096, ngx_cycle->log);
    if (ngx_poold_pool == NULL) {
        return NGX_ERROR;
    }

    ngx_map_init(&ngx_poold_map, ngx_map_hash_uint, ngx_cmp_uint);
    ngx_poold_free_node = NULL;

    ngx_poold_nalloc = 0;
    ngx_poold_nfree = 0;

    return NGX_OK;
}


static ngx_poold_node_t *
ngx_poold_get_node()
{
    ngx_poold_node_t           *n;

    n = ngx_poold_free_node;
    if (n == NULL) {
        n = ngx_pcalloc(ngx_poold_pool, sizeof(ngx_poold_node_t));
        if (n == NULL) {
            return NULL;
        }

        ++ngx_poold_nalloc;
    } else {
        ngx_poold_free_node = n->next;
        ngx_memzero(n, sizeof(ngx_poold_node_t));

        --ngx_poold_nfree;
    }

    return n;
}


static void
ngx_poold_put_node(ngx_poold_node_t *node)
{
    if (ngx_poold_pool == NULL) {
        return;
    }

    if (node == NULL) {
        return;
    }

    node->next = ngx_poold_free_node;
    ngx_poold_free_node = node;

    ++ngx_poold_nfree;
}


ngx_pool_t *
ngx_create_pool_debug(size_t size, ngx_log_t *log, char *file, int line)
{
    ngx_poold_node_t           *node;

    if (ngx_poold_pool == NULL) {
        ngx_poold_init();
    }

    /* construct a poold node */
    node = ngx_poold_get_node();
    node->pool = ngx_create_pool(size, log);
    node->file = file;
    node->line = line;

    /* record node in poold map */
    node->m.raw_key = (intptr_t) node->pool;
    ngx_map_insert(&ngx_poold_map, &node->m, 0);

    return node->pool;
}


void
ngx_destroy_pool_debug(ngx_pool_t *pool, char *file, int line)
{
    ngx_poold_node_t           *node;
    ngx_map_node_t             *m;

    /* get node by pool */
    m = ngx_map_find(&ngx_poold_map, (intptr_t) pool);
    if (m == NULL) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                "destroy pool twice: %s:%d", file, line);
        return;
    }
    ngx_map_delete(&ngx_poold_map, (intptr_t) pool);
    node = (ngx_poold_node_t *) ((char *) m - offsetof(ngx_poold_node_t, m));

    ngx_destroy_pool(pool);

    /* put node in poold map */
    ngx_poold_put_node(node);
}


ngx_chain_t *
ngx_poold_state(ngx_http_request_t *r, unsigned detail)
{
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    ngx_map_node_t             *node;
    ngx_poold_node_t           *pn;
    size_t                      len, len1;
    ngx_uint_t                  n;

    len = sizeof("##########ngx debug pool##########\n") - 1
        + sizeof("ngx_poold nalloc node: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_poold nfree node: \n") - 1 + NGX_OFF_T_LEN;

    len1 = 0;

    /* node for create pool */
    if (detail) {
        n = ngx_poold_nalloc - ngx_poold_nfree;
        /* "    file:line\n" */
        len1 = 4 + 256 + 1 + NGX_OFF_T_LEN + 1;
        len += len1 * n;
    }

    cl = ngx_alloc_chain_link(r->pool);
    if (cl == NULL) {
        return NULL;
    }
    cl->next = NULL;

    b = ngx_create_temp_buf(r->pool, len);
    if (b == NULL) {
        return NULL;
    }
    cl->buf = b;

    b->last = ngx_snprintf(b->last, len,
            "##########ngx debug pool##########\n"
            "ngx_poold nalloc node: %ui\nngx_poold nfree node: %ui\n",
            ngx_poold_nalloc, ngx_poold_nfree);

    if (detail) {
        for (node = ngx_map_begin(&ngx_poold_map); node;
                node = ngx_map_next(node))
        {
            /* m is first element of ngx_poold_node_t */
            pn = (ngx_poold_node_t *) node;
            b->last = ngx_snprintf(b->last, len1, "    %s:%d\n",
                    pn->file, pn->line);
        }
    }

    return cl;
}
