
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"
#include "ngx_rtmp_amf.h"
#include "ngx_rtmp_cmd_module.h"
#include <string.h>


ngx_int_t
ngx_rtmp_protocol_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_buf_t              *b;
    u_char                 *p;
    uint32_t                val;
    uint8_t                 limit;

    b = in->buf;

    if (b->last - b->pos < 4) {
        ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "too small buffer for %d message: %d",
                (int)h->type, b->last - b->pos);
        return NGX_OK;
    }

    p = (u_char*)&val;
    p[0] = b->pos[3];
    p[1] = b->pos[2];
    p[2] = b->pos[1];
    p[3] = b->pos[0];

    switch(h->type) {
        case NGX_RTMP_MSG_CHUNK_SIZE:
            /* set chunk size =val */
            ngx_rtmp_set_chunk_size(s, val);
            break;

        case NGX_RTMP_MSG_ABORT:
            /* abort chunk stream =val */
            break;

        case NGX_RTMP_MSG_ACK:
            /* receive ack with sequence number =val */
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "receive ack seq=%uD", val);
            break;

        case NGX_RTMP_MSG_ACK_SIZE:
            /* receive window size =val */
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "receive ack_size=%uD", val);
            s->ack_size = val;
            break;

        case NGX_RTMP_MSG_BANDWIDTH:
            if (b->last - b->pos >= 5) {
                limit = *(uint8_t*)&b->pos[4];

                (void)val;
                (void)limit;

                ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                    "receive bandwidth=%uD limit=%d",
                    val, (int)limit);

                /* receive window size =val
                 * && limit */
            }
            break;

        default:
            return NGX_ERROR;
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_user_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
                              ngx_chain_t *in)
{
    ngx_buf_t              *b;
    u_char                 *p;
    uint16_t                evt;
    uint32_t                val;

    b = in->buf;

    if (b->last - b->pos < 6) {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "too small buffer for user message: %d",
                       b->last - b->pos);
        return NGX_OK;
    }

    p = (u_char*)&evt;

    p[0] = b->pos[1];
    p[1] = b->pos[0];

    ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                   "RTMP recv user evt %s (%i)",
                   ngx_rtmp_user_message_type(evt), (ngx_int_t) evt);

    p = (u_char *) &val;

    p[0] = b->pos[5];
    p[1] = b->pos[4];
    p[2] = b->pos[3];
    p[3] = b->pos[2];

    switch(evt) {
        case NGX_RTMP_USER_STREAM_BEGIN:
            {
                ngx_rtmp_stream_begin_t     v;

                v.msid = val;

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "receive: stream_begin msid=%uD", v.msid);

                return ngx_rtmp_stream_begin(s, &v);
            }

        case NGX_RTMP_USER_STREAM_EOF:
            {
                ngx_rtmp_stream_eof_t       v;

                v.msid = val;

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "receive: stream_eof msid=%uD", v.msid);

                return ngx_rtmp_stream_eof(s, &v);
            }

        case NGX_RTMP_USER_STREAM_DRY:
            {
                ngx_rtmp_stream_dry_t       v;

                v.msid = val;

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "receive: stream_dry msid=%uD", v.msid);

                return ngx_rtmp_stream_dry(s, &v);
            }

        case NGX_RTMP_USER_SET_BUFLEN:
            {
                ngx_rtmp_set_buflen_t       v;

                v.msid = val;

                if (b->last - b->pos < 10) {
                    return NGX_OK;
                }

                p = (u_char *) &v.buflen;

                p[0] = b->pos[9];
                p[1] = b->pos[8];
                p[2] = b->pos[7];
                p[3] = b->pos[6];

                ngx_log_debug2(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "receive: set_buflen msid=%uD buflen=%uD",
                               v.msid, v.buflen);

                /*TODO: move this to play module */
                s->buflen = v.buflen;

                return ngx_rtmp_set_buflen(s, &v);
            }

        case NGX_RTMP_USER_RECORDED:
            {
                ngx_rtmp_recorded_t       v;

                v.msid = val;

                ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                               "receive: recorded msid=%uD", v.msid);

                return ngx_rtmp_recorded(s, &v);
            }

        case NGX_RTMP_USER_PING_REQUEST:
            return ngx_rtmp_send_ping_response(s, val);

        case NGX_RTMP_USER_PING_RESPONSE:

            /* val = incoming timestamp */

            ngx_rtmp_reset_ping(s);

            return NGX_OK;

        default:
            ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                           "unexpected user event: %i", (ngx_int_t) evt);

            return NGX_OK;
    }
}


static ngx_int_t
ngx_rtmp_fetch(ngx_chain_t **in, u_char *ret)
{
    while (*in && (*in)->buf->pos >= (*in)->buf->last) {
        *in = (*in)->next;
    }

    if (*in == NULL) {
        return NGX_DONE;
    }

    *ret = *(*in)->buf->pos++;

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_fetch_uint8(ngx_chain_t **in, uint8_t *ret)
{
    return ngx_rtmp_fetch(in, (u_char *) ret);
}


static ngx_int_t
ngx_rtmp_fetch_uint32(ngx_chain_t **in, uint32_t *ret, ngx_int_t n)
{
    u_char     *r = (u_char *) ret;
    ngx_int_t   rc;

    *ret = 0;

    while (--n >= 0) {
        rc = ngx_rtmp_fetch(in, &r[n]);
        if (rc != NGX_OK) {
            return rc;
        }
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_aggregate_message_handler(ngx_rtmp_session_t *s, ngx_rtmp_header_t *h,
                                   ngx_chain_t *in)
{
    uint32_t            base_time, timestamp, prev_size;
    size_t              len;
    ngx_int_t           first;
    u_char             *last;
    ngx_int_t           rc;
    ngx_buf_t          *b;
    ngx_chain_t        *cl, *next;
    ngx_rtmp_header_t   ch;

    ch = *h;

    first = 1;
    base_time = 0;

    while (in) {
        if (ngx_rtmp_fetch_uint8(&in, &ch.type) != NGX_OK) {
            return NGX_OK;
        }

        if (ngx_rtmp_fetch_uint32(&in, &ch.mlen, 3) != NGX_OK) {
            return NGX_ERROR;
        }

        if (ngx_rtmp_fetch_uint32(&in, &timestamp, 3) != NGX_OK) {
            return NGX_ERROR;
        }

        if (ngx_rtmp_fetch_uint8(&in, (uint8_t *) &timestamp + 3) != NGX_OK)
        {
            return NGX_ERROR;
        }

        if (ngx_rtmp_fetch_uint32(&in, &ch.msid, 3) != NGX_OK)
        {
            return NGX_ERROR;
        }

        if (first) {
            base_time = timestamp;
            first = 0;
        }

        ngx_log_debug6(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "RTMP aggregate %s (%d) len=%uD time=%uD (+%D) msid=%uD",
                       ngx_rtmp_message_type(ch.type),
                       (ngx_int_t) ch.type, ch.mlen, ch.timestamp,
                       timestamp - base_time, ch.msid);

        /* limit chain */

        len = 0;
        cl = in;
        while (cl) {
            b = cl->buf;
            len += (b->last - b->pos);
            if (len > ch.mlen) {
                break;
            }
            cl = cl->next;
        }

        if (cl == NULL) {
            ngx_log_error(NGX_LOG_INFO, s->log, 0,
                          "RTMP error parsing aggregate");
            return NGX_ERROR;
        }

        next = cl->next;
        cl->next = NULL;
        b = cl->buf;
        last = b->last;
        b->last -= (len - ch.mlen);

        /* handle aggregated message */

        ch.timestamp = h->timestamp + timestamp - base_time;

        rc = ngx_rtmp_receive_message(s, &ch, in);

        /* restore chain before checking the result */

        in = cl;
        in->next = next;
        b->pos = b->last;
        b->last = last;

        if (rc != NGX_OK) {
            return rc;
        }

        /* read 32-bit previous tag size */

        if (ngx_rtmp_fetch_uint32(&in, &prev_size, 4) != NGX_OK) {
            return NGX_OK;
        }

        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                       "RTMP aggregate prev_size=%uD", prev_size);
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_amf_message_handler(ngx_rtmp_session_t *s,
        ngx_rtmp_header_t *h, ngx_chain_t *in)
{
    ngx_rtmp_amf_ctx_t          act;
    ngx_rtmp_core_main_conf_t  *cmcf;
    ngx_array_t                *ch;
    ngx_rtmp_handler_pt        *ph;
    size_t                      len, n;

    static u_char               func[128];

    static ngx_rtmp_amf_elt_t   elts[] = {

        { NGX_RTMP_AMF_STRING,
          ngx_null_string,
          func,   sizeof(func) },
    };

    /* AMF command names come with string type, but shared object names
     * come without type */
    if (h->type == NGX_RTMP_MSG_AMF_SHARED ||
        h->type == NGX_RTMP_MSG_AMF3_SHARED)
    {
        elts[0].type |= NGX_RTMP_AMF_TYPELESS;
    } else {
        elts[0].type &= ~NGX_RTMP_AMF_TYPELESS;
    }

    if ((h->type == NGX_RTMP_MSG_AMF3_SHARED ||
         h->type == NGX_RTMP_MSG_AMF3_META ||
         h->type == NGX_RTMP_MSG_AMF3_CMD)
         && in->buf->last > in->buf->pos)
    {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "AMF3 prefix: %ui", (ngx_int_t)*in->buf->pos);
        ++in->buf->pos;
    }

    cmcf = ngx_rtmp_get_module_main_conf(s, ngx_rtmp_core_module);

    /* read AMF func name & transaction id */
    ngx_memzero(&act, sizeof(act));
    act.link = in;
    act.log = s->log;
    memset(func, 0, sizeof(func));

    if (ngx_rtmp_amf_read(&act, elts,
                sizeof(elts) / sizeof(elts[0])) != NGX_OK)
    {
        ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "AMF cmd failed");
        return NGX_ERROR;
    }

    /* skip name */
    in = act.link;
    in->buf->pos += act.offset;

    len = ngx_strlen(func);

    ch = ngx_hash_find(&cmcf->amf_hash,
            ngx_hash_strlow(func, func, len), func, len);

    if (ch && ch->nelts) {
        ph = ch->elts;
        for (n = 0; n < ch->nelts; ++n, ++ph) {
            ngx_log_debug3(NGX_LOG_DEBUG_RTMP, s->log, 0,
                "AMF func '%s' passed to handler %d/%d",
                func, n, ch->nelts);
            switch ((*ph)(s, h, in)) {
                case NGX_ERROR:
                    return NGX_ERROR;
                case NGX_DONE:
                    return NGX_OK;
            }
        }
    } else {
        ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "AMF cmd '%s' no handler", func);
    }

    return NGX_OK;
}


ngx_int_t
ngx_rtmp_receive_amf(ngx_rtmp_session_t *s, ngx_chain_t *in,
        ngx_rtmp_amf_elt_t *elts, size_t nelts)
{
    ngx_rtmp_amf_ctx_t     act;

    ngx_memzero(&act, sizeof(act));
    act.link = in;
    act.log = s->log;

    return ngx_rtmp_amf_read(&act, elts, nelts);
}
