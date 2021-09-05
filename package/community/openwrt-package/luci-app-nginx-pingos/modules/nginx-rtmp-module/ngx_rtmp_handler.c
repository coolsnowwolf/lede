
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_amf.h"
#include "ngx_rbuf.h"
#include "ngx_poold.h"
#include "ngx_rtmp_monitor_module.h"


static void ngx_rtmp_recv(ngx_event_t *rev);
static void ngx_rtmp_send(ngx_event_t *rev);
static void ngx_rtmp_ping(ngx_event_t *rev);
static ngx_int_t ngx_rtmp_finalize_set_chunk_size(ngx_rtmp_session_t *s);


ngx_uint_t                  ngx_rtmp_naccepted;


ngx_rtmp_bandwidth_t        ngx_rtmp_bw_out;
ngx_rtmp_bandwidth_t        ngx_rtmp_bw_in;


#ifdef NGX_DEBUG
char*
ngx_rtmp_message_type(uint8_t type)
{
    static char*    types[] = {
        "?",
        "chunk_size",
        "abort",
        "ack",
        "user",
        "ack_size",
        "bandwidth",
        "edge",
        "audio",
        "video",
        "?",
        "?",
        "?",
        "?",
        "?",
        "amf3_meta",
        "amf3_shared",
        "amf3_cmd",
        "amf_meta",
        "amf_shared",
        "amf_cmd",
        "?",
        "aggregate"
    };

    return type < sizeof(types) / sizeof(types[0])
        ? types[type]
        : "?";
}


char*
ngx_rtmp_user_message_type(uint16_t evt)
{
    static char*    evts[] = {
        "stream_begin",
        "stream_eof",
        "stream dry",
        "set_buflen",
        "recorded",
        "",
        "ping_request",
        "ping_response",
    };

    return evt < sizeof(evts) / sizeof(evts[0])
        ? evts[evt]
        : "?";
}
#endif


void
ngx_rtmp_cycle(ngx_rtmp_session_t *s)
{
    ngx_connection_t           *c;

    c = s->connection;
    c->read->handler =  ngx_rtmp_recv;
    c->write->handler = ngx_rtmp_send;

    s->ping_evt.data = c;
    s->ping_evt.log = s->log;
    s->ping_evt.handler = ngx_rtmp_ping;
    ngx_rtmp_reset_ping(s);

    ngx_rtmp_recv(c->read);
}


static ngx_chain_t *
ngx_rtmp_alloc_in_buf(ngx_rtmp_session_t *s)
{
    ngx_chain_t        *cl;
    ngx_buf_t          *b;
    size_t              size;

    if ((cl = ngx_alloc_chain_link(s->in_pool)) == NULL
       || (cl->buf = ngx_calloc_buf(s->in_pool)) == NULL)
    {
        return NULL;
    }

    cl->next = NULL;
    b = cl->buf;
    size = s->in_chunk_size + NGX_RTMP_MAX_CHUNK_HEADER;

    b->start = b->last = b->pos = ngx_palloc(s->in_pool, size);
    if (b->start == NULL) {
        return NULL;
    }
    b->end = b->start + size;

    return cl;
}


void
ngx_rtmp_reset_ping(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_srv_conf_t   *cscf;

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);
    if (cscf->ping == 0) {
        return;
    }

    s->ping_active = 0;
    s->ping_reset = 0;
    ngx_add_timer(&s->ping_evt, cscf->ping);

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "ping: wait %Mms", cscf->ping);
}


static void
ngx_rtmp_ping(ngx_event_t *pev)
{
    ngx_connection_t           *c;
    ngx_rtmp_session_t         *s;
    ngx_rtmp_core_srv_conf_t   *cscf;

    c = pev->data;
    s = c->data;

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    /* i/o event has happened; no need to ping */
    if (s->ping_reset) {
        ngx_rtmp_reset_ping(s);
        return;
    }

    if (s->ping_active) {
        ngx_log_error(NGX_LOG_INFO, s->log, 0, "ping: unresponded");
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (cscf->busy) {
        ngx_log_error(NGX_LOG_INFO, s->log, 0, "ping: not busy between pings");
        ngx_rtmp_finalize_session(s);
        return;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "ping: schedule %Mms", cscf->ping_timeout);

    if (ngx_rtmp_send_ping_request(s, (uint32_t)ngx_current_msec) != NGX_OK) {
        ngx_rtmp_finalize_session(s);
        return;
    }

    s->ping_active = 1;
    ngx_add_timer(pev, cscf->ping_timeout);
}


static void
ngx_rtmp_recv(ngx_event_t *rev)
{
    ngx_int_t                   n;
    ngx_connection_t           *c;
    ngx_rtmp_session_t         *s;
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_rtmp_header_t          *h;
    ngx_rtmp_stream_t          *st, *st0;
    ngx_chain_t                *in, *head;
    ngx_buf_t                  *b;
    u_char                     *p, *pp, *old_pos;
    size_t                      size, fsize, old_size;
    uint8_t                     fmt, ext;
    uint32_t                    csid, timestamp;

    c = rev->data;
    s = c->data;
    b = NULL;
    old_pos = NULL;
    old_size = 0;
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    if (c->destroyed) {
        return;
    }

    if (ngx_rtmp_core_main_conf->fast_reload && (ngx_exiting || ngx_terminate)) {
        s->finalize_reason = NGX_LIVE_PROCESS_EXIT;
        ngx_rtmp_finalize_session(s);
        return;
    }

    for( ;; ) {

        st = &s->in_streams[s->in_csid];

        /* allocate new buffer */
        if (st->in == NULL) {
            st->in = ngx_rtmp_alloc_in_buf(s);
            if (st->in == NULL) {
                ngx_log_error(NGX_LOG_INFO, s->log, 0, "in buf alloc failed");
                ngx_rtmp_finalize_session(s);
                return;
            }
        }

        h  = &st->hdr;
        in = st->in;
        b  = in->buf;

        if (old_size) {

            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                    "reusing formerly read data: %d", old_size);

            b->pos = b->start;
            b->last = ngx_movemem(b->pos, old_pos, old_size);

            if (s->in_chunk_size_changing) {
                ngx_rtmp_finalize_set_chunk_size(s);
            }

        } else {

            if (old_pos) {
                b->pos = b->last = b->start;
            }

            n = c->recv(c, b->last, b->end - b->last);

            if (n == NGX_ERROR || n == 0) {
                s->finalize_reason = n == 0? NGX_LIVE_NORMAL_CLOSE:
                                             NGX_LIVE_RTMP_RECV_ERR;
                ngx_rtmp_finalize_session(s);
                return;
            }

            if (n == NGX_AGAIN) {
                if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
                    ngx_rtmp_finalize_session(s);
                }
                return;
            }

            s->ping_reset = 1;
            ngx_rtmp_update_bandwidth(&ngx_rtmp_bw_in, n);
            b->last += n;
            s->in_bytes += n;

            if (s->in_bytes >= 0xf0000000) {
                ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "resetting byte counter");
                s->in_bytes = 0;
                s->in_last_ack = 0;
            }

            if (s->ack_size && s->in_bytes - s->in_last_ack >= s->ack_size) {

                s->in_last_ack = s->in_bytes;

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                        "sending RTMP ACK(%uD)", s->in_bytes);

                if (ngx_rtmp_send_ack(s, s->in_bytes)) {
                    ngx_rtmp_finalize_session(s);
                    return;
                }
            }
        }

        old_pos = NULL;
        old_size = 0;

        /* parse headers */
        if (b->pos == b->start) {
            p = b->pos;

            /* chunk basic header */
            fmt  = (*p >> 6) & 0x03;
            csid = *p++ & 0x3f;

            if (csid == 0) {
                if (b->last - p < 1)
                    continue;
                csid = 64;
                csid += *(uint8_t*)p++;

            } else if (csid == 1) {
                if (b->last - p < 2)
                    continue;
                csid = 64;
                csid += *(uint8_t*)p++;
                csid += (uint32_t)256 * (*(uint8_t*)p++);
            }

            ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                    "RTMP bheader fmt=%d csid=%D",
                    (int)fmt, csid);

            if (csid >= (uint32_t)cscf->max_streams) {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                    "RTMP in chunk stream too big: %D >= %D",
                    csid, cscf->max_streams);
                ngx_rtmp_finalize_session(s);
                return;
            }

            /* link orphan */
            if (s->in_csid == 0) {

                /* unlink from stream #0 */
                st->in = st->in->next;

                /* link to new stream */
                s->in_csid = csid;
                st = &s->in_streams[csid];
                if (st->in == NULL) {
                    in->next = in;
                } else {
                    in->next = st->in->next;
                    st->in->next = in;
                }
                st->in = in;
                h = &st->hdr;
                h->csid = csid;
            }

            ext = st->ext;
            timestamp = st->dtime;
            if (fmt <= 2 ) {
                if (b->last - p < 3)
                    continue;
                /* timestamp:
                 *  big-endian 3b -> little-endian 4b */
                pp = (u_char*)&timestamp;
                pp[2] = *p++;
                pp[1] = *p++;
                pp[0] = *p++;
                pp[3] = 0;

                ext = (timestamp == 0x00ffffff);

                if (fmt <= 1) {
                    if (b->last - p < 4)
                        continue;
                    /* size:
                     *  big-endian 3b -> little-endian 4b
                     * type:
                     *  1b -> 1b*/
                    pp = (u_char*)&h->mlen;
                    pp[2] = *p++;
                    pp[1] = *p++;
                    pp[0] = *p++;
                    pp[3] = 0;
                    h->type = *(uint8_t*)p++;

                    if (fmt == 0) {
                        if (b->last - p < 4)
                            continue;
                        /* stream:
                         *  little-endian 4b -> little-endian 4b */
                        pp = (u_char*)&h->msid;
                        pp[0] = *p++;
                        pp[1] = *p++;
                        pp[2] = *p++;
                        pp[3] = *p++;
                    }
                }
            }

            /* extended header */
            if (ext) {
                if (b->last - p < 4)
                    continue;
                pp = (u_char*)&timestamp;
                pp[3] = *p++;
                pp[2] = *p++;
                pp[1] = *p++;
                pp[0] = *p++;
            }

            if (st->len == 0) {
                /* Messages with type=3 should
                 * never have ext timestamp field
                 * according to standard.
                 * However that's not always the case
                 * in real life */
                st->ext = (ext && cscf->publish_time_fix);
                if (fmt) {
                    st->dtime = timestamp;
                } else {
                    h->timestamp = timestamp;
                    st->dtime = 0;
                }
            }

            ngx_log_debug8(NGX_LOG_DEBUG_RTMP, s->log, 0,
                    "RTMP mheader fmt=%d %s (%d) "
                    "time=%uD+%uD mlen=%D len=%D msid=%D",
                    (int)fmt, ngx_rtmp_message_type(h->type), (int)h->type,
                    h->timestamp, st->dtime, h->mlen, st->len, h->msid);

            /* header done */
            b->pos = p;

            if (h->mlen > cscf->max_message) {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "too big message: %uz", cscf->max_message);
                ngx_rtmp_finalize_session(s);
                return;
            }
        }

        size = b->last - b->pos;
        fsize = h->mlen - st->len;

        if (size < ngx_min(fsize, s->in_chunk_size))
            continue;

        /* buffer is ready */

        if (fsize > s->in_chunk_size) {
            /* collect fragmented chunks */
            st->len += s->in_chunk_size;
            b->last = b->pos + s->in_chunk_size;
            old_pos = b->last;
            old_size = size - s->in_chunk_size;

        } else {
            /* handle! */
            head = st->in->next;
            st->in->next = NULL;
            b->last = b->pos + fsize;
            old_pos = b->last;
            old_size = size - fsize;
            st->len = 0;
            h->timestamp += st->dtime;

            if (ngx_rtmp_receive_message(s, h, head) != NGX_OK) {
                ngx_rtmp_finalize_session(s);
                return;
            }

            if (s->in_chunk_size_changing) {
                /* copy old data to a new buffer */
                if (!old_size) {
                    ngx_rtmp_finalize_set_chunk_size(s);
                }

            } else {
                /* add used bufs to stream #0 */
                st0 = &s->in_streams[0];
                st->in->next = st0->in;
                st0->in = head;
                st->in = NULL;
            }
        }

        s->in_csid = 0;
    }
}


static ngx_flag_t
ngx_rtmp_relative_timestamp(ngx_rtmp_session_t *s, ngx_rtmp_header_t *lh)
{
    return 0;
}

static ngx_chain_t *
ngx_rtmp_prepare_out_chain(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_rtmp_frame_t           *frame;
    ngx_chain_t                *head, *l, **ll;
    uint32_t                    mlen, timestamp, ext_timestamp;
    uint8_t                     fmt, hsize, thsize;
    static uint8_t              hdrsize[] = { 12, 8, 4, 1 };
    ngx_flag_t                  relative;
    ngx_rtmp_header_t           lh;
    u_char                      th[7], *p, *pp;

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    frame = s->out[s->out_pos];
    head = NULL;

    if (frame->hdr.csid >= (uint32_t)cscf->max_streams) {
        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "RTMP out chunk stream too big: %D >= %D",
                frame->hdr.csid, cscf->max_streams);
        goto failed;
    }

    mlen = 0;
    fmt = 0;
    relative = ngx_rtmp_relative_timestamp(s, &lh);

    for (l = frame->chain; l; l = l->next) {
        mlen += ngx_buf_size(l->buf);
    }

    if (relative && lh.csid && frame->hdr.msid == lh.msid) {
        ++fmt;
        if (frame->hdr.type == lh.type && mlen && mlen == lh.mlen) {
            ++fmt;
            if (frame->hdr.timestamp == lh.timestamp) {
                ++fmt;
            }
        }
        timestamp = frame->hdr.timestamp - lh.timestamp;
    } else {
        timestamp = frame->hdr.timestamp;
    }

    hsize = hdrsize[fmt];

    ngx_log_debug7(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "RTMP prep %s (%d) fmt=%d csid=%uD timestamp=%uD mlen=%uD msid=%uD",
            ngx_rtmp_message_type(frame->hdr.type), (int)frame->hdr.type,
            (int)fmt, frame->hdr.csid, timestamp, mlen, frame->hdr.msid);

    ext_timestamp = 0;
    if (timestamp >= 0x00ffffff) {
        ext_timestamp = timestamp;
        timestamp = 0x00ffffff;
        hsize += 4;
    }

    if (frame->hdr.csid >= 64) {
        ++hsize;
        if (frame->hdr.csid >= 320) {
            ++hsize;
        }
    }

    /* fill initial header */
    head = ngx_get_chainbuf(NGX_RTMP_MAX_CHUNK_HEADER, 1);
    if (head == NULL) {
        goto failed;
    }
    p = head->buf->pos;

    /* basic header */
    *p = (fmt << 6);
    if (frame->hdr.csid >= 2 && frame->hdr.csid <= 63) {
        *p++ |= (((uint8_t)frame->hdr.csid) & 0x3f);
    } else if (frame->hdr.csid >= 64 && frame->hdr.csid < 320) {
        ++p;
        *p++ = (uint8_t)(frame->hdr.csid - 64);
    } else {
        *p++ |= 1;
        *p++ = (uint8_t)(frame->hdr.csid - 64);
        *p++ = (uint8_t)((frame->hdr.csid - 64) >> 8);
    }

    /* create fmt3 header for successive fragments */
    thsize = p - head->buf->pos;
    ngx_memcpy(th, head->buf->pos, thsize);
    th[0] |= 0xc0;

    /* message header */
    if (fmt <= 2) {
        pp = (u_char *)&timestamp;
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];
        if (fmt <= 1) {
            pp = (u_char *)&mlen;
            *p++ = pp[2];
            *p++ = pp[1];
            *p++ = pp[0];
            *p++ = frame->hdr.type;
            if (fmt == 0) {
                pp = (u_char *)&frame->hdr.msid;
                *p++ = pp[0];
                *p++ = pp[1];
                *p++ = pp[2];
                *p++ = pp[3];
            }
        }
    }

    /* extended timestamp */
    if (ext_timestamp) {
        pp = (u_char *)&ext_timestamp;
        *p++ = pp[3];
        *p++ = pp[2];
        *p++ = pp[1];
        *p++ = pp[0];

        /* This CONTRADICTS the standard
         * but that's the way flash client
         * wants data to be encoded;
         * ffmpeg complains */
        if (cscf->play_time_fix) {
            ngx_memcpy(&th[thsize], p - 4, 4);
            thsize += 4;
        }
    }
    head->buf->last = p;

    /* append headers to successive fragments */
    ll = &head->next;
    l = frame->chain;
    while (l && l->buf->pos == l->buf->last) {
        l = l->next;
    }
    if (l == NULL) {
        return head;
    }

    *ll = ngx_get_chainbuf(0, 0);
    (*ll)->buf->pos = l->buf->pos;
    (*ll)->buf->last = l->buf->last;

    for (l = l->next; l; l = l->next) {
        /* chunk header */
        ll = &(*ll)->next;
        *ll = ngx_get_chainbuf(NGX_RTMP_MAX_CHUNK_HEADER, 1);
        (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, th, thsize);

        /* payload */
        ll = &(*ll)->next;
        *ll = ngx_get_chainbuf(0, 0);
        (*ll)->buf->pos = l->buf->pos;
        (*ll)->buf->last = l->buf->last;
    }

    ngx_rtmp_monitor_frame(s, &frame->hdr, NULL, frame->av_header, 0);

    return head;

failed:
    ngx_put_chainbufs(head);

    ngx_rtmp_finalize_session(s);
    return NULL;
}

static void
ngx_rtmp_send(ngx_event_t *wev)
{
    ngx_connection_t           *c;
    ngx_rtmp_session_t         *s;
    ngx_int_t                   n;
    ngx_chain_t                *chain, *cl;
    off_t                       sent;

    c = wev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    if (wev->timedout) {
        ngx_log_error(NGX_LOG_INFO, s->log, NGX_ETIMEDOUT, "client timed out");
        c->timedout = 1;
        s->finalize_reason = NGX_LIVE_RTMP_SEND_TIMEOUT;
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (wev->timer_set) {
        ngx_del_timer(wev);
    }

    if (s->prepare_handler == NULL) {
        s->prepare_handler = ngx_rtmp_prepare_out_chain;
    }

    if (ngx_rtmp_core_main_conf->fast_reload && (ngx_exiting || ngx_terminate)) {
        s->finalize_reason = NGX_LIVE_PROCESS_EXIT;
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (ngx_rtmp_prepare_merge_frame(s) == NGX_ERROR) {
        ngx_rtmp_finalize_session(s);
        return;
    }

    while (s->out_chain) {
        sent = c->sent;

        chain = c->send_chain(c, s->out_chain, 0);

        n = c->sent - sent;

        if (chain == NGX_CHAIN_ERROR) { /* NGX_ERROR */
            c->error = 1;
            s->finalize_reason = NGX_LIVE_RTMP_SEND_ERR;
            ngx_rtmp_finalize_session(s);
            return;
        }

        for (cl = s->out_chain; cl != chain;) {
            s->out_chain = cl->next;
            ngx_free_chain(s->pool, cl);
            cl = s->out_chain;
        }

        if (chain) { /* NGX_AGAIN */
            ngx_add_timer(c->write, s->timeout);
            if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
                ngx_rtmp_finalize_session(s);
            }
            return;
        }

        s->out_bytes += n;
        s->ping_reset = 1;
        ngx_rtmp_update_bandwidth(&ngx_rtmp_bw_out, n);

        if (ngx_rtmp_prepare_merge_frame(s) == NGX_ERROR) {
            ngx_rtmp_finalize_session(s);
            return;
        }
    }

    if (wev->active) {
        ngx_del_event(wev, NGX_WRITE_EVENT, 0);
    }

    ngx_event_process_posted((ngx_cycle_t *) ngx_cycle, &s->posted_dry_events);
}


ngx_int_t
ngx_rtmp_send_message(ngx_rtmp_session_t *s, ngx_rtmp_frame_t *out,
        ngx_uint_t priority)
{
    ngx_uint_t                      nmsg;

    if (out == NULL) {
        goto send;
    }

    nmsg = (s->out_last - s->out_pos) % s->out_queue + 1;

    if (priority > 3) {
        priority = 3;
    }

    /* drop packet?
     * Note we always leave 1 slot free */
    if (nmsg + priority * s->out_queue / 4 >= s->out_queue) {
        ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "RTMP drop message bufs=%ui, priority=%ui",
                nmsg, priority);
        return NGX_AGAIN;
    }

    s->out[s->out_last++] = out;
    s->out_last %= s->out_queue;

    ngx_rtmp_shared_acquire_frame(out);

    ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "RTMP send nmsg=%ui, priority=%ui #%ui",
            nmsg, priority, s->out_last);

    if (priority && s->out_buffer && nmsg < s->out_cork) {
        return NGX_OK;
    }

send:
    if (!s->connection->write->active) {
        ngx_post_event(s->connection->write, &ngx_posted_events);
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_receive_message(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_array_t                *evhs;
    size_t                      n;
    ngx_rtmp_handler_pt        *evh;

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

#ifdef NGX_DEBUG
    {
        int             nbufs;
        ngx_chain_t    *ch;

        for(nbufs = 1, ch = in;
                ch->next;
                ch = ch->next, ++nbufs);

        ngx_log_debug7(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "RTMP recv %s (%d) csid=%D timestamp=%D "
                "mlen=%D msid=%D nbufs=%d",
                ngx_rtmp_message_type(h->type), (int)h->type,
                h->csid, h->timestamp, h->mlen, h->msid, nbufs);
    }
#endif

    if (h->type > NGX_RTMP_MSG_MAX) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "unexpected RTMP message type: %d", (int)h->type);
        return NGX_OK;
    }

    evhs = &cmcf->events[h->type];
    evh = evhs->elts;

    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0, "nhandlers: %d", evhs->nelts);

    for(n = 0; n < evhs->nelts; ++n, ++evh) {
        if (!evh) {
            continue;
        }
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0, "calling handler %d", n);

        switch ((*evh)(s, h, in)) {
            case NGX_ERROR:
                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                        "handler %d failed", n);
                return NGX_ERROR;
            case NGX_DONE:
                return NGX_OK;
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_set_chunk_size(ngx_rtmp_session_t *s, ngx_uint_t size)
{
    ngx_rtmp_core_srv_conf_t           *cscf;
    ngx_chain_t                        *li, *fli, *lo, *flo;
    ngx_buf_t                          *bi, *bo;
    ngx_int_t                           n;

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    s->in_old_pool = s->in_pool;
    s->in_chunk_size = size;
    s->in_pool = NGX_CREATE_POOL(4096, ngx_cycle->log);

    /* copy existing chunk data */
    if (s->in_old_pool) {
        s->in_chunk_size_changing = 1;
        s->in_streams[0].in = NULL;

        for(n = 1; n < cscf->max_streams; ++n) {
            /* stream buffer is circular
             * for all streams except for the current one
             * (which caused this chunk size change);
             * we can simply ignore it */
            li = s->in_streams[n].in;
            if (li == NULL || li->next == NULL) {
                s->in_streams[n].in = NULL;
                continue;
            }
            /* move from last to the first */
            li = li->next;
            fli = li;
            lo = ngx_rtmp_alloc_in_buf(s);
            if (lo == NULL) {
                return NGX_ERROR;
            }
            flo = lo;
            for ( ;; ) {
                bi = li->buf;
                bo = lo->buf;

                if (bo->end - bo->last >= bi->last - bi->pos) {
                    bo->last = ngx_cpymem(bo->last, bi->pos,
                            bi->last - bi->pos);
                    li = li->next;
                    if (li == fli)  {
                        lo->next = flo;
                        s->in_streams[n].in = lo;
                        break;
                    }
                    continue;
                }

                bi->pos += (ngx_cpymem(bo->last, bi->pos,
                            bo->end - bo->last) - bo->last);
                lo->next = ngx_rtmp_alloc_in_buf(s);
                lo = lo->next;
                if (lo == NULL) {
                    return NGX_ERROR;
                }
            }
        }
    }

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_finalize_set_chunk_size(ngx_rtmp_session_t *s)
{
    if (s->in_chunk_size_changing && s->in_old_pool) {
        NGX_DESTROY_POOL(s->in_old_pool);
        s->in_old_pool = NULL;
        s->in_chunk_size_changing = 0;
    }
    return NGX_OK;
}


