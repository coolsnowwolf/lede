/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_timerd.h"
#include "ngx_map.h"


typedef struct ngx_timerd_node_s ngx_timerd_node_t;

static ngx_pool_t              *ngx_timerd_pool;

static ngx_map_t                ngx_timerd_map;
static ngx_timerd_node_t        *ngx_timerd_free_node;

static ngx_uint_t               timerd_footprint;
static ngx_uint_t               ngx_timerd_nalloc;
static ngx_uint_t               ngx_timerd_nfree;


struct ngx_timerd_node_s {
    ngx_map_node_t              m;      /* map node */
    ngx_timerd_node_t          *next;   /* free node */

    ngx_uint_t                  footprint;
    off_t                       fpoff;
    ngx_event_t                 ev;

    ngx_event_t                *uev;
    char                       *file;   /* file create pool */
    int                         line;   /* line create pool */
};


static ngx_int_t
ngx_timerd_init()
{
    ngx_timerd_pool = ngx_create_pool(4096, ngx_cycle->log);
    if (ngx_timerd_pool == NULL) {
        return NGX_ERROR;
    }

    ngx_map_init(&ngx_timerd_map, ngx_map_hash_uint, ngx_cmp_uint);
    ngx_timerd_free_node = NULL;

    ngx_timerd_nalloc = 0;
    ngx_timerd_nfree = 0;

    return NGX_OK;
}


static ngx_timerd_node_t *
ngx_timerd_get_node()
{
    ngx_timerd_node_t          *n;

    n = ngx_timerd_free_node;
    if (n == NULL) {
        n = ngx_pcalloc(ngx_timerd_pool, sizeof(ngx_timerd_node_t));
        if (n == NULL) {
            return NULL;
        }

        ++ngx_timerd_nalloc;
    } else {
        ngx_timerd_free_node = n->next;
        ngx_memzero(n, sizeof(ngx_timerd_node_t));

        --ngx_timerd_nfree;
    }

    return n;
}


static void
ngx_timerd_put_node(ngx_timerd_node_t *node)
{
    if (ngx_timerd_pool == NULL) {
        return;
    }

    if (node == NULL) {
        return;
    }

    node->next = ngx_timerd_free_node;
    ngx_timerd_free_node = node;

    ++ngx_timerd_nfree;
}


static void
ngx_timerd_wrap(ngx_event_t *ev)
{
    ngx_timerd_node_t          *node;
    ngx_event_t                *uev;
    ngx_uint_t                 *fp;

    node = ev->data;
    uev = node->uev;

    fp = (ngx_uint_t *) ((char *) (uev->data) + node->fpoff);
    if (*fp != node->footprint) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                "timer wrap, timer trigger but not timer owner: %s:%d",
                node->file, node->line);
        return;
    }

    ngx_map_delete(&ngx_timerd_map, (intptr_t) uev);

    uev->timer_set = 0;
    uev->timedout = 1;

    uev->handler(uev);

    ngx_timerd_put_node(node);
}


ngx_uint_t
ngx_timerd_footprint()
{
    return timerd_footprint++;
}


void
ngx_add_timer_debug(ngx_event_t *ev, ngx_msec_t timer, off_t fpoff,
        char *file, int line)
{
    ngx_timerd_node_t          *node;
    ngx_map_node_t             *m;
    ngx_uint_t                 *fp;

    if (ngx_timerd_pool == NULL) {
        ngx_timerd_init();
    }

    fp = (ngx_uint_t *) ((char *) ev->data + fpoff);

    m = ngx_map_find(&ngx_timerd_map, (intptr_t) ev);
    if (m == NULL) { // first add
        node = ngx_timerd_get_node();

        node->m.raw_key = (intptr_t) ev;
        ngx_map_insert(&ngx_timerd_map, &node->m, 0);

        node->footprint = *fp;
        node->fpoff = fpoff;

        node->ev.log = ngx_cycle->log;
        node->ev.data = node;
        node->ev.handler = ngx_timerd_wrap;

        node->uev = ev;
        node->file = file;
        node->line = line;
    } else {
        node = (ngx_timerd_node_t *) ((char *) m -
                                      offsetof(ngx_timerd_node_t, m));
        if (node->footprint != *fp) {
            ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                    "add timer but not timer owner(%s:%d): %s:%d",
                    node->file, node->line, file, line);
            return;
        }
    }

    ev->timer_set = 1;
    ngx_add_timer(&node->ev, timer);
}


void
ngx_del_timer_debug(ngx_event_t *ev, ngx_uint_t footprint, char *file, int line)
{
    ngx_timerd_node_t          *node;
    ngx_map_node_t             *m;

    /* get node by pool */
    m = ngx_map_find(&ngx_timerd_map, (intptr_t) ev);
    if (m == NULL) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                "delete timer twice: %s:%d", file, line);
        return;
    }

    node = (ngx_timerd_node_t *) ((char *) m - offsetof(ngx_timerd_node_t, m));
    if (node->footprint != footprint) {
        ngx_log_error(NGX_LOG_EMERG, ngx_cycle->log, 0,
                "delete timer but not timer owner: %s:%d", file, line);
        return;
    }

    ngx_map_delete(&ngx_timerd_map, (intptr_t) ev);

    if (node->ev.timer_set) {
        ev->timer_set = 0;
        ngx_del_timer(&node->ev);
    }

    if (node->ev.posted) {
        ngx_delete_posted_event(&node->ev);
    }

    /* put node in timerd map */
    ngx_timerd_put_node(node);
}


ngx_chain_t *
ngx_timerd_state(ngx_http_request_t *r, unsigned detail)
{
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    ngx_map_node_t             *node;
    ngx_timerd_node_t          *pn;
    size_t                      len, len1;
    ngx_uint_t                  n;

    len = sizeof("##########ngx debug pool##########\n") - 1
        + sizeof("ngx_timerd nalloc node: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_timerd nfree node: \n") - 1 + NGX_OFF_T_LEN;

    len1 = 0;

    /* node for create pool */
    if (detail) {
        n = ngx_timerd_nalloc - ngx_timerd_nfree;
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
            "ngx_timerd nalloc node: %ui\nngx_timerd nfree node: %ui\n",
            ngx_timerd_nalloc, ngx_timerd_nfree);

    if (detail) {
        for (node = ngx_map_begin(&ngx_timerd_map); node;
                node = ngx_map_next(node))
        {
            /* m is first element of ngx_timerd_node_t */
            pn = (ngx_timerd_node_t *) node;
            b->last = ngx_snprintf(b->last, len1, "    %s:%d\n",
                    pn->file, pn->line);
        }
    }

    return cl;
}
