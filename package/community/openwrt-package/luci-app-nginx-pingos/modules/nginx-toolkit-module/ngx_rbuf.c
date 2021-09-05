/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_map.h"


static ngx_pool_t              *ngx_rbuf_pool;

static ngx_map_t                ngx_rbuf_map;
static ngx_chain_t             *ngx_rbuf_free_chain;

static ngx_uint_t               ngx_rbuf_nalloc_node;

static ngx_uint_t               ngx_rbuf_nalloc_buf;
static ngx_uint_t               ngx_rbuf_nfree_buf;

static ngx_uint_t               ngx_rbuf_nalloc_chain;
static ngx_uint_t               ngx_rbuf_nfree_chain;

static ngx_map_t                ngx_rbuf_using;

#define ngx_rbuf_buf(b)                                             \
    (ngx_rbuf_t *) ((u_char *) (b) - offsetof(ngx_rbuf_t, buf))

typedef struct ngx_rbuf_s   ngx_rbuf_t;

struct ngx_rbuf_s {
    size_t                      size;
    ngx_rbuf_t                 *next;
    u_char                      buf[];
};

typedef struct {
    ngx_map_node_t              node;
    ngx_rbuf_t                 *rbuf;
} ngx_rbuf_node_t;

typedef struct {
    ngx_chain_t                 cl;
    ngx_buf_t                   buf;
    unsigned                    alloc;

    ngx_map_node_t              node;
    char                       *file;
    int                         line;
} ngx_chainbuf_t;


static ngx_int_t
ngx_rbuf_init()
{
    ngx_rbuf_pool = ngx_create_pool(4096, ngx_cycle->log);
    if (ngx_rbuf_pool == NULL) {
        return NGX_ERROR;
    }

    ngx_map_init(&ngx_rbuf_map, ngx_map_hash_uint, ngx_cmp_uint);
    ngx_map_init(&ngx_rbuf_using, ngx_map_hash_uint, ngx_cmp_uint);

    ngx_rbuf_nalloc_node = 0;
    ngx_rbuf_nalloc_buf = 0;
    ngx_rbuf_nfree_buf = 0;
    ngx_rbuf_nalloc_chain = 0;
    ngx_rbuf_nfree_chain = 0;

    return NGX_OK;
}


static ngx_rbuf_t *
ngx_rbuf_get_buf(size_t key)
{
    ngx_rbuf_node_t            *rn;
    ngx_map_node_t             *node;
    ngx_rbuf_t                 *rb;

    node = ngx_map_find(&ngx_rbuf_map, key);
    if (node == NULL) { /* new key */
        rn = ngx_pcalloc(ngx_rbuf_pool, sizeof(ngx_rbuf_node_t));
        if (rn == NULL) {
            return NULL;
        }

        node = &rn->node;
        node->raw_key = key;
        ngx_map_insert(&ngx_rbuf_map, node, 0);

        ++ngx_rbuf_nalloc_node;
    } else {
        rn = (ngx_rbuf_node_t *) node;
    }

    rb = rn->rbuf;
    if (rb == NULL) {
        rb = ngx_pcalloc(ngx_rbuf_pool, sizeof(ngx_rbuf_t) + key);
        if (rb == NULL) {
            return NULL;
        }
        rb->size = key;

        ++ngx_rbuf_nalloc_buf;
    } else {
        rn->rbuf = rb->next;
        rb->next = NULL;

        --ngx_rbuf_nfree_buf;
    }

    return rb;
}

static void
ngx_rbuf_put_buf(ngx_rbuf_t *rb)
{
    ngx_rbuf_node_t            *rn;
    ngx_map_node_t             *node;

    node = ngx_map_find(&ngx_rbuf_map, rb->size);
    if (node == NULL) {
        return;
    }

    rn = (ngx_rbuf_node_t *) node;
    rb->next = rn->rbuf;
    rn->rbuf = rb;

    ++ngx_rbuf_nfree_buf;
}


static u_char *
ngx_rbuf_alloc(size_t size)
{
    ngx_rbuf_t                 *rb;

    rb = ngx_rbuf_get_buf(size);

    return rb->buf;
}

static void
ngx_rbuf_free(u_char *rb)
{
    ngx_rbuf_t                 *rbuf;

    rbuf = ngx_rbuf_buf(rb);
    ngx_rbuf_put_buf(rbuf);
}


ngx_chain_t *
ngx_get_chainbuf_debug(size_t size, ngx_flag_t alloc_rbuf, char *file, int line)
{
    ngx_chainbuf_t             *cb;
    ngx_chain_t                *cl;

    if (ngx_rbuf_pool == NULL) {
        ngx_rbuf_init();
    }

    cl = ngx_rbuf_free_chain;
    if (cl) {
        ngx_rbuf_free_chain = cl->next;
        cl->next = NULL;
        cb = (ngx_chainbuf_t *) cl;

        --ngx_rbuf_nfree_chain;
    } else {
        cb = ngx_pcalloc(ngx_rbuf_pool, sizeof(ngx_chainbuf_t));
        if (cb == NULL) {
            return NULL;
        }

        cl = &cb->cl;
        cl->buf = &cb->buf;

        ++ngx_rbuf_nalloc_chain;
    }

    if (alloc_rbuf) {
        cl->buf->last = cl->buf->pos = cl->buf->start = ngx_rbuf_alloc(size);
        cl->buf->end = cl->buf->start + size;
        cb->alloc = 1;
    } else {
        cl->buf->pos = cl->buf->last = cl->buf->start = cl->buf->end = NULL;
        cb->alloc = 0;
    }
    cl->buf->memory = 1;

    // record chainbuf in using map
    cb->file = file;
    cb->line = line;
    cb->node.raw_key = (intptr_t) cl;
    ngx_map_insert(&ngx_rbuf_using, &cb->node, 0);

    return cl;
}

void
ngx_put_chainbuf_debug(ngx_chain_t *cl, char *file, int line)
{
    ngx_chainbuf_t             *cb;

    if (ngx_rbuf_pool == NULL) {
        return;
    }

    if (cl == NULL) {
        return;
    }

    cb = (ngx_chainbuf_t *) cl;

    if (cb->alloc) {
        ngx_rbuf_free(cl->buf->start);
    }
    cl->next = ngx_rbuf_free_chain;
    ngx_rbuf_free_chain = cl;
    ++ngx_rbuf_nfree_chain;

    // delete chainbuf from using map
    if (ngx_map_find(&ngx_rbuf_using, (intptr_t) cl) == NULL) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                "destroy chainbuf twice: %s:%d", file, line);
        return;
    }
    ngx_map_delete(&ngx_rbuf_using, (intptr_t) cl);
}

ngx_chain_t *
ngx_rbuf_state(ngx_http_request_t *r, unsigned detail)
{
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len, len1;
    ngx_uint_t                  n;
    ngx_chainbuf_t             *cb;
    ngx_map_node_t             *node;

    len = sizeof("##########ngx rbuf state##########\n") - 1
        + sizeof("ngx_rbuf nalloc node: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_rbuf nalloc buf: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_rbuf nfree buf: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_rbuf nalloc chain: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_rbuf nfree chain: \n") - 1 + NGX_OFF_T_LEN;

    len1 = 0;

    if (detail) {
        n = ngx_rbuf_nalloc_chain - ngx_rbuf_nfree_chain;
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
            "##########ngx rbuf state##########\nngx_rbuf nalloc node: %ui\n"
            "ngx_rbuf nalloc buf: %ui\nngx_rbuf nfree buf: %ui\n"
            "ngx_rbuf nalloc chain: %ui\nngx_rbuf nfree chain: %ui\n",
            ngx_rbuf_nalloc_node, ngx_rbuf_nalloc_buf, ngx_rbuf_nfree_buf,
            ngx_rbuf_nalloc_chain, ngx_rbuf_nfree_chain);

    if (detail) {
        for (node = ngx_map_begin(&ngx_rbuf_using); node;
                node = ngx_map_next(node))
        {
            cb = (ngx_chainbuf_t *) ((char *) node
                    - offsetof(ngx_chainbuf_t, node));
            b->last = ngx_snprintf(b->last, len1, "    %s:%d\n",
                    cb->file, cb->line);
        }
    }

    return cl;
}
