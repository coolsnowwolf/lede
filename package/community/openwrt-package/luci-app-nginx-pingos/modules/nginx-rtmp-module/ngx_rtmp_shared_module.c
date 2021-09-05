/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rbuf.h"


static void *ngx_rtmp_shared_create_conf(ngx_cycle_t *cycle);
static char *ngx_rtmp_shared_init_conf(ngx_cycle_t *cycle, void *conf);


/* 1316 == 188 * 7 RTP pack 7 MPEG-TS packets as a RTP package */
#define NGX_MPEGTS_BUF_SIZE   1316

typedef struct {
    ngx_rtmp_frame_t           *free_frame;
    ngx_mpegts_frame_t         *free_mpegts_frame;
    ngx_pool_t                 *pool;

    ngx_uint_t                  nalloc_frame;
    ngx_uint_t                  nfree_frame;
} ngx_rtmp_shared_conf_t;


static ngx_command_t  ngx_rtmp_shared_commands[] = {

      ngx_null_command
};


static ngx_core_module_t  ngx_rtmp_shared_module_ctx = {
    ngx_string("rtmp_shared"),
    ngx_rtmp_shared_create_conf,            /* create conf */
    ngx_rtmp_shared_init_conf               /* init conf */
};


ngx_module_t  ngx_rtmp_shared_module = {
    NGX_MODULE_V1,
    &ngx_rtmp_shared_module_ctx,            /* module context */
    ngx_rtmp_shared_commands,               /* module directives */
    NGX_CORE_MODULE,                        /* module type */
    NULL,                                   /* init master */
    NULL,                                   /* init module */
    NULL,                                   /* init process */
    NULL,                                   /* init thread */
    NULL,                                   /* exit thread */
    NULL,                                   /* exit process */
    NULL,                                   /* exit master */
    NGX_MODULE_V1_PADDING
};


static void *
ngx_rtmp_shared_create_conf(ngx_cycle_t *cycle)
{
    ngx_rtmp_shared_conf_t     *rscf;

    rscf = ngx_pcalloc(cycle->pool, sizeof(ngx_rtmp_shared_conf_t));
    if (rscf == NULL) {
        return NULL;
    }

    return rscf;
}


static char *
ngx_rtmp_shared_init_conf(ngx_cycle_t *cycle, void *conf)
{
    ngx_rtmp_shared_conf_t     *rscf = conf;

    rscf->pool = ngx_create_pool(4096, cycle->log);
    if (rscf->pool == NULL) {
        return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}


ngx_int_t
ngx_rtmp_prepare_merge_frame(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_app_conf_t   *cacf;
    ngx_chain_t                *ll, **ln;
    ngx_uint_t                  n;

    // merge frame not send completely
    if (s->out_chain) {
        return NGX_OK;
    }

    cacf = ngx_rtmp_get_module_app_conf(s, ngx_rtmp_core_module);

    ngx_rtmp_free_merge_frame(s);

    ln = &s->out_chain;

    for (n = 0; n < cacf->merge_frame && s->out_pos != s->out_last; ++n) {
        // save frame chain with rtmp chunk or flv tag
        s->merge[n] = s->prepare_handler(s);

        if (s->out_pos == s->out_last) {
            break;
        }

        // normal link to chain all merge frames
        for (ll = s->merge[n]; ll; ll = ll->next, ln = &(*ln)->next) {
            *ln = ngx_alloc_chain_link(s->pool);
            if (*ln == NULL) {
                s->nframe = n;
                return NGX_ERROR;
            }
            (*ln)->next = NULL;

            (*ln)->buf = ll->buf;
        }

        // save frame prepare to send
        if (s->live_type == NGX_MPEGTS_LIVE) {
            s->prepare_mpegts_frame[n] = s->mpegts_out[s->out_pos];
        } else {
            s->prepare_frame[n] = s->out[s->out_pos];
        }

        ++s->out_pos;
        s->out_pos %= s->out_queue;
    }

    s->nframe = n;

    return NGX_OK;
}

void
ngx_rtmp_free_merge_frame(ngx_rtmp_session_t *s)
{
    ngx_uint_t                  n;

    if (s->live_type == NGX_MPEGTS_LIVE) {
        for (n = 0; n < s->nframe; ++n) {
            ngx_put_chainbufs(s->merge[n]);
            s->merge[n] = NULL;

            ngx_rtmp_shared_free_mpegts_frame(s->prepare_mpegts_frame[n]);
            s->prepare_mpegts_frame[n] = NULL;
        }
    } else {
        for (n = 0; n < s->nframe; ++n) {
            ngx_put_chainbufs(s->merge[n]);
            s->merge[n] = NULL;

            ngx_rtmp_shared_free_frame(s->prepare_frame[n]);
            s->prepare_frame[n] = NULL;
        }
    }

    s->nframe = 0;
}

void
ngx_rtmp_shared_append_chain(ngx_rtmp_frame_t *frame, size_t size,
        ngx_chain_t *cl, ngx_flag_t mandatory)
{
    ngx_chain_t               **ll;
    u_char                     *p;
    size_t                      len;

    for (ll = &frame->chain; (*ll) && (*ll)->next; ll = &(*ll)->next);

    if (cl == NULL) {
        if (mandatory) {
            *ll = ngx_get_chainbuf(size, 1);
        }
        return;
    }

    p = cl->buf->pos;

    for (;;) {
        if ((*ll) && (*ll)->buf->last == (*ll)->buf->end) {
            ll = &(*ll)->next;
        }

        if (*ll == NULL) {
            *ll = ngx_get_chainbuf(size, 1);
        }

        while ((*ll)->buf->end - (*ll)->buf->last >= cl->buf->last - p) {
            len = cl->buf->last - p;
            (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
            cl = cl->next;
            if (cl == NULL) {
                return;
            }
            p = cl->buf->pos;
        }

        len = (*ll)->buf->end - (*ll)->buf->last;
        (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
        p += len;
    }
}

ngx_rtmp_frame_t *
ngx_rtmp_shared_alloc_frame(size_t size, ngx_chain_t *cl, ngx_flag_t mandatory)
{
    ngx_rtmp_shared_conf_t     *rscf;
    ngx_rtmp_frame_t           *frame;

    rscf = (ngx_rtmp_shared_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                   ngx_rtmp_shared_module);

    frame = rscf->free_frame;
    if (frame) {
        rscf->free_frame = frame->next;
        frame->chain = NULL;
        --rscf->nfree_frame;
    } else {
        frame = ngx_pcalloc(rscf->pool, sizeof(ngx_rtmp_frame_t));
        if (frame == NULL) {
            return NULL;
        }
        ++rscf->nalloc_frame;
    }

    frame->ref = 1;
    frame->next = NULL;

    ngx_rtmp_shared_append_chain(frame, size, cl, mandatory);

    return frame;
}

void
ngx_rtmp_shared_free_frame(ngx_rtmp_frame_t *frame)
{
    ngx_rtmp_shared_conf_t     *rscf;
    ngx_chain_t                *cl;

    rscf = (ngx_rtmp_shared_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                   ngx_rtmp_shared_module);

    if (frame == NULL || --frame->ref) {
        return;
    }

    /* recycle chainbuf */
    cl = frame->chain;
    while (cl) {
        frame->chain = cl->next;
        ngx_put_chainbuf(cl);
        cl = frame->chain;
    }

    /* recycle frame */
    frame->next = rscf->free_frame;
    rscf->free_frame = frame;
    ++rscf->nfree_frame;
}

void
ngx_mpegts_shared_append_chain(ngx_mpegts_frame_t *frame, ngx_chain_t *cl,
        ngx_flag_t mandatory)
{
    ngx_chain_t               **ll;
    u_char                     *p;
    size_t                      len;

    for (ll = &frame->chain; (*ll) && (*ll)->next; ll = &(*ll)->next);

    if (cl == NULL) {
        if (mandatory) {
            *ll = ngx_get_chainbuf(NGX_MPEGTS_BUF_SIZE, 1);
        }
        return;
    }

    p = cl->buf->pos;

    for (;;) {
        if ((*ll) && (*ll)->buf->last == (*ll)->buf->end) {
            ll = &(*ll)->next;
        }

        if (*ll == NULL) {
            *ll = ngx_get_chainbuf(NGX_MPEGTS_BUF_SIZE, 1);
        }

        while ((*ll)->buf->end - (*ll)->buf->last >= cl->buf->last - p) {
            len = cl->buf->last - p;
            (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
            cl = cl->next;
            if (cl == NULL) {
                return;
            }
            p = cl->buf->pos;
        }

        len = (*ll)->buf->end - (*ll)->buf->last;
        (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
        p += len;
    }
}

ngx_mpegts_frame_t *
ngx_rtmp_shared_alloc_mpegts_frame(ngx_chain_t *cl, ngx_flag_t mandatory)
{
    ngx_rtmp_shared_conf_t     *rscf;
    ngx_mpegts_frame_t         *frame;

    rscf = (ngx_rtmp_shared_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                   ngx_rtmp_shared_module);

    frame = rscf->free_mpegts_frame;
    if (frame) {
        rscf->free_mpegts_frame = frame->next;
        --rscf->nfree_frame;
    } else {
        frame = ngx_pcalloc(rscf->pool, sizeof(ngx_mpegts_frame_t));
        if (frame == NULL) {
            return NULL;
        }
        ++rscf->nalloc_frame;
    }

    ngx_memset(frame, 0, sizeof(ngx_mpegts_frame_t));
    frame->ref = 1;
    frame->next = NULL;

    ngx_mpegts_shared_append_chain(frame, cl, mandatory);

    return frame;
}

void
ngx_rtmp_shared_free_mpegts_frame(ngx_mpegts_frame_t *frame)
{
    ngx_rtmp_shared_conf_t     *rscf;
    ngx_chain_t                *cl;

    rscf = (ngx_rtmp_shared_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                   ngx_rtmp_shared_module);

    if (frame == NULL || --frame->ref) {
        return;
    }

    /* recycle chainbuf */
    cl = frame->chain;
    while (cl) {
        frame->chain = cl->next;
        ngx_put_chainbuf(cl);
        cl = frame->chain;
    }

    /* recycle frame */
    frame->next = rscf->free_mpegts_frame;
    rscf->free_mpegts_frame = frame;
    ++rscf->nfree_frame;
}

ngx_chain_t *
ngx_rtmp_shared_state(ngx_http_request_t *r)
{
	ngx_rtmp_shared_conf_t     *rscf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;
    size_t                      len;

    rscf = (ngx_rtmp_shared_conf_t *) ngx_get_conf(ngx_cycle->conf_ctx,
                                                   ngx_rtmp_shared_module);

    len = sizeof("##########rtmp shared state##########\n") - 1
        + sizeof("ngx_rtmp_shared alloc frame: \n") - 1 + NGX_OFF_T_LEN
        + sizeof("ngx_rtmp_shared free frame: \n") - 1 + NGX_OFF_T_LEN;

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
            "##########rtmp shared state##########\n"
            "ngx_rtmp_shared alloc frame: %ui\n"
            "ngx_rtmp_shared free frame: %ui\n",
            rscf->nalloc_frame, rscf->nfree_frame);

    return cl;
}
