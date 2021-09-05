/*
 * Copyright (C) AlexWoo(Wu Jie) wj19840501@gmail.com
 */


#include "ngx_live_relay.h"
#include "ngx_http_client.h"
#include "ngx_toolkit_misc.h"
#include "ngx_dynamic_resolver.h"
#include "ngx_rbuf.h"


typedef struct {
    ngx_uint_t                  status;
    char                       *code;
    char                       *level;
    char                       *desc;
} ngx_http_status_code_t;


static ngx_http_status_code_t ngx_http_relay_status_code[] = {
    { 400, "NetStream.Play.BadName", "error", "Bad Request" },
    { 403, "NetStream.Play.Forbidden", "error", "Forbidden" },
    { 404, "NetStream.Play.StreamNotFound", "error", "No such stream" },
    { 503, "NetStream.Play.ServiceUnavailable", "error", "Service Unavailable"},
    { 0, "NetStream.Play.StreamError", "error", "Stream Error" }
};


static ngx_int_t
ngx_live_relay_httpflv_parse(ngx_rtmp_session_t *s, ngx_buf_t *b)
{
    u_char                      ch, *p, *pc;
    ngx_rtmp_stream_t          *st;
    ngx_rtmp_header_t          *h;
    ngx_chain_t               **ll;
    size_t                      len;
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_int_t                   rc = NGX_AGAIN;
    enum {
        flv_header_F = 0,
        flv_header_FL,
        flv_header_FLV,
        flv_header_Version,
        flv_header_Flags,
        flv_header_DataOffset0,
        flv_header_DataOffset1,
        flv_header_DataOffset2,
        flv_header_DataOffset3,
        flv_tagsize0,
        flv_tagsize1,
        flv_tagsize2,
        flv_tagsize3,
        flv_tagtype,
        flv_datasize0,
        flv_datasize1,
        flv_datasize2,
        flv_timestamp0,
        flv_timestamp1,
        flv_timestamp2,
        flv_timestamp_extended,
        flv_streamid0,
        flv_streamid1,
        flv_streamid2,
        flv_data
    } state;

    state = s->flv_state;
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    for (p = b->pos; p < b->last; ++p) {
        ch = *p;

        switch (state) {

        case flv_header_F:
            switch (ch) {
            case 'F':
                state = flv_header_FL;
                break;
            default:
                rc = NGX_ERROR;
                goto done;
            }
            break;

        case flv_header_FL:
            switch (ch) {
            case 'L':
                state = flv_header_FLV;
                break;
            default:
                rc = NGX_ERROR;
                goto done;
            }
            break;

        case flv_header_FLV:
            switch (ch) {
            case 'V':
                state = flv_header_Version;
                break;
            default:
                rc = NGX_ERROR;
                goto done;
            }
            break;

        case flv_header_Version:
            s->flv_version = ch;
            if (s->flv_version != 1) {
                rc = NGX_ERROR;
                goto done;
            }
            state = flv_header_Flags;
            break;

        case flv_header_Flags:
            s->flv_flags = ch;
            state = flv_header_DataOffset0;
            break;

        case flv_header_DataOffset0:
            pc = (u_char *) &s->flv_data_offset;
            pc[3] = ch;
            state = flv_header_DataOffset1;
            break;

        case flv_header_DataOffset1:
            pc = (u_char *) &s->flv_data_offset;
            pc[2] = ch;
            state = flv_header_DataOffset2;
            break;

        case flv_header_DataOffset2:
            pc = (u_char *) &s->flv_data_offset;
            pc[1] = ch;
            state = flv_header_DataOffset3;
            break;

        case flv_header_DataOffset3:
            pc = (u_char *) &s->flv_data_offset;
            pc[0] = ch;
            state = flv_tagsize0;
            break;

        case flv_tagsize0:
            s->flv_tagsize = 0;
            pc = (u_char *) &s->flv_tagsize;
            pc[3] = ch;
            state = flv_tagsize1;
            break;

        case flv_tagsize1:
            pc = (u_char *) &s->flv_tagsize;
            pc[2] = ch;
            state = flv_tagsize2;
            break;

        case flv_tagsize2:
            pc = (u_char *) &s->flv_tagsize;
            pc[1] = ch;
            state = flv_tagsize3;
            break;

        case flv_tagsize3:
            pc = (u_char *) &s->flv_tagsize;
            pc[0] = ch;

            st = &s->in_streams[0];
            h = &st->hdr;

            if (h->mlen == 0 && s->flv_first_pts == 0) {
                s->flv_first_pts = 1;
                if (s->flv_tagsize != 0) {
                    rc = NGX_ERROR;
                    goto done;
                }
            } else {
                if (h->mlen + 11 != s->flv_tagsize) {
                    rc = NGX_ERROR;
                    goto done;
                }
            }
            state = flv_tagtype;

            break;

        case flv_tagtype:
            if (ch != NGX_RTMP_MSG_AMF_META && ch != NGX_RTMP_MSG_AUDIO
                    && ch != NGX_RTMP_MSG_VIDEO)
            {
                rc = NGX_ERROR;
                goto done;
            }

            st = &s->in_streams[0];
            h = &st->hdr;
            h->type = ch;
            state = flv_datasize0;

            break;

        case flv_datasize0:
            st = &s->in_streams[0];
            h = &st->hdr;
            h->mlen = 0;
            pc = (u_char *) &h->mlen;

            pc[2] = ch;
            state = flv_datasize1;

            break;

        case flv_datasize1:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->mlen;

            pc[1] = ch;
            state = flv_datasize2;

            break;

        case flv_datasize2:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->mlen;

            pc[0] = ch;
            state = flv_timestamp0;
            st->len = h->mlen;

            break;

        case flv_timestamp0:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->timestamp;

            pc[2] = ch;
            state = flv_timestamp1;

            break;

        case flv_timestamp1:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->timestamp;

            pc[1] = ch;
            state = flv_timestamp2;

            break;

        case flv_timestamp2:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->timestamp;

            pc[0] = ch;
            state = flv_timestamp_extended;

            break;

        case flv_timestamp_extended:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->timestamp;

            pc[3] = ch;
            state = flv_streamid0;

            break;

        case flv_streamid0:
            st = &s->in_streams[0];
            h = &st->hdr;
            h->msid = 0;
            pc = (u_char *) &h->msid;

            pc[2] = ch;
            state = flv_streamid1;

            break;

        case flv_streamid1:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->msid;

            pc[1] = ch;
            state = flv_streamid2;

            break;

        case flv_streamid2:
            st = &s->in_streams[0];
            h = &st->hdr;
            pc = (u_char *) &h->msid;

            pc[0] = ch;
            state = flv_data;

            break;

        case flv_data:
            st = &s->in_streams[0];

            for (ll = &st->in; (*ll) && (*ll)->buf->last == (*ll)->buf->end;
                    ll = &(*ll)->next);

            for (;;) {
                if (*ll == NULL) {
                    *ll = ngx_get_chainbuf(cscf->chunk_size, 1);
                }

                len = ngx_min(st->len, b->last - p);
                if ((*ll)->buf->end - (*ll)->buf->last >= (long) len) {
                    (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
                    p += len;
                    st->len -= len;

                    break;
                }

                len = (*ll)->buf->end - (*ll)->buf->last;
                (*ll)->buf->last = ngx_cpymem((*ll)->buf->last, p, len);
                p += len;
                st->len -= len;

                ll = &(*ll)->next;
            }

            if (st->len != 0) {
                rc = NGX_AGAIN;
                goto done;
            }

            state = flv_tagsize0;
            rc = NGX_OK;
            goto done;
        }
    }

done:
    b->pos = p;
    s->flv_state = state;

    return rc;
}


static void
ngx_live_relay_httpflv_recv_body(void *request, ngx_http_request_t *hcr)
{
    ngx_int_t                   n;
    ngx_rtmp_session_t         *s;
    ngx_chain_t                *cl, *l, *in;
    ngx_rtmp_header_t          *h;
    ngx_rtmp_stream_t          *st = NULL;

    s = request;

    n = ngx_http_client_read_body(hcr, &cl);

    if (n == 0 || n == NGX_ERROR) {
        s->finalize_reason = n == 0? NGX_LIVE_NORMAL_CLOSE:
                                     NGX_LIVE_FLV_RECV_ERR;
        ngx_log_error(NGX_LOG_INFO, s->log, ngx_errno,
                "http relay, recv body error");
        ngx_rtmp_finalize_session(s);
        return;
    }

    l = cl;
    for (;;) {
        if (l && l->buf->pos == l->buf->last) {
            l = l->next;
        }

        if (l == NULL) {
            return;
        }

        n = ngx_live_relay_httpflv_parse(s, l->buf);

        if (n == NGX_ERROR) {
            ngx_log_error(NGX_LOG_ERR, s->log, 0,
                    "http relay, parse flv frame failed in state %d",
                    s->flv_state);
            ngx_http_client_finalize_request(hcr, 1);

            return;
        }

        if (n == NGX_AGAIN) {
            continue;
        }

        /* NGX_OK */
        st = &s->in_streams[0];
        h = &st->hdr;
        in = st->in;

        if (ngx_rtmp_receive_message(s, h, in) != NGX_OK) {
            ngx_rtmp_finalize_session(s);
            return;
        }

        ngx_put_chainbufs(st->in);
        st->in = NULL;
    }
}


static void
ngx_live_relay_httpflv_cleanup(void *data)
{
    ngx_rtmp_session_t         *s;

    s = data;

    ngx_log_error(NGX_LOG_INFO, s->log, 0, "http flv client, cleanup");

    if (s) {
        if (s->close.posted) {
            ngx_delete_posted_event(&s->close);
        }

        if (s->finalize_reason == 0) {
            s->finalize_reason = NGX_LIVE_FLV_RECV_ERR;
        }

        ngx_rtmp_finalize_fake_session(s);
    }
}


static void
ngx_live_relay_httpflv_error(ngx_rtmp_session_t *s, ngx_uint_t status)
{
    ngx_live_stream_t          *st;
    ngx_rtmp_core_ctx_t        *cctx;
    char                       *code, *level, *desc;
    size_t                      i;

    for (i = 0; ngx_http_relay_status_code[i].status; ++i) {

        if (status != ngx_http_relay_status_code[i].status) {
            continue;
        }

        break;
    }

    code = ngx_http_relay_status_code[i].code;
    level = ngx_http_relay_status_code[i].level;
    desc = ngx_http_relay_status_code[i].desc;

    ngx_log_error(NGX_LOG_ERR, s->log, 0,
            "http relay transit, %d: level='%s' code='%s' description='%s'",
            status, level, code, desc);

    st = ngx_live_create_stream(&s->serverid, &s->stream);
    cctx = st->play_ctx;

    for (; cctx; cctx = cctx->next) {
        cctx->session->status = status;
        ngx_rtmp_send_status(cctx->session, code, level, desc);

        if (ngx_strcmp(level, "error") == 0 && !cctx->session->static_pull) {
            cctx->session->finalize_reason = NGX_LIVE_RELAY_TRANSIT;
            ngx_rtmp_finalize_session(cctx->session);
        }
    }
}


static void
ngx_live_relay_httpflv_recv(void *request, ngx_http_request_t *hcr)
{
    ngx_rtmp_session_t         *s;
    ngx_uint_t                  status_code;

    s = request;
    status_code = ngx_http_client_status_code(hcr);

    s->stage = NGX_LIVE_PLAY;
    s->ptime = ngx_current_msec;

    s->connection = hcr->connection;
    ngx_rtmp_set_combined_log(s, hcr->connection->log->data,
            hcr->connection->log->handler);
    s->log->connection = s->connection->number;

    if (ngx_rtmp_core_main_conf->fast_reload && (ngx_exiting || ngx_terminate)) {
        ngx_live_relay_httpflv_error(s, NGX_LIVE_PROCESS_EXIT);
        s->finalize_reason = NGX_LIVE_PROCESS_EXIT;
        ngx_http_client_finalize_request(hcr, 1);
        return;
    }

    if (status_code != NGX_HTTP_OK) {
        ngx_live_relay_httpflv_error(s, status_code);
        s->finalize_reason = NGX_LIVE_FLV_RECV_ERR;
        ngx_http_client_finalize_request(hcr, 1);
        return;
    }

    ngx_live_relay_publish_local(s);

    ngx_http_client_set_read_handler(hcr, ngx_live_relay_httpflv_recv_body);
    ngx_live_relay_httpflv_recv_body(request, hcr);
}


static ngx_int_t
ngx_live_relay_httpflv_send_request(ngx_rtmp_session_t *s,
        ngx_live_relay_url_t *url)
{
    ngx_http_request_t         *hcr;
    ngx_str_t                   request_url;
    size_t                      len;
    ngx_live_relay_ctx_t       *ctx;
    ngx_http_cleanup_t         *cln;
    u_char                     *p;

    ctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);

    // request url
    // http:// + url + ':' + port + '/' + app + '/' + name + ['?' + pargs]
    len = 7 + url->url.host.len + 1 + sizeof("65535") - 1
        + 1 + ctx->app.len + 1 + ctx->name.len;
    if (ctx->pargs.len) {
        len = len + 1 + ctx->pargs.len;
    }

    request_url.data = ngx_pcalloc(s->pool, len);
    if (request_url.data == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "alloc mem for request url failed");
        return NGX_ERROR;
    }

    if (ctx->pargs.len) {
        p = ngx_snprintf(request_url.data, len, "http://%V:%d/%V/%V?%V",
                &url->url.host, url->port, &ctx->app, &ctx->name, &ctx->pargs);
    } else {
        p = ngx_snprintf(request_url.data, len, "http://%V:%d/%V/%V",
                &url->url.host, url->port, &ctx->app, &ctx->name);
    }
    request_url.len = p - request_url.data;

    // request headers
    ngx_keyval_t                headers[] = {
        { ngx_string("Host"),       ctx->domain     },
        { ngx_string("Referer"),    ctx->referer    },
        { ngx_string("User-Agent"), ctx->user_agent },
        { ngx_null_string,          ngx_null_string }
    };

    s->stage = NGX_LIVE_CONNECT;
    s->connect_time = ngx_current_msec;

    hcr = ngx_http_client_get(s->log, &request_url, headers, s);
    ngx_http_client_set_read_handler(hcr, ngx_live_relay_httpflv_recv);

    cln = ngx_http_client_cleanup_add(hcr, 0);
    if (cln == NULL) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0, "http client add cleanup failed");
        ngx_live_relay_httpflv_error(s, NGX_HTTP_INTERNAL_SERVER_ERROR);
        ngx_http_client_finalize_request(hcr, 1);
        return NGX_ERROR;
    }
    cln->handler = ngx_live_relay_httpflv_cleanup;
    cln->data = s;

    s->request = hcr;
    s->live_type = NGX_HTTP_FLV_LIVE;

    return NGX_OK;
}


ngx_int_t
ngx_live_relay_create_httpflv(ngx_rtmp_session_t *s, ngx_live_relay_t *relay,
        ngx_live_relay_url_t *url)
{
    ngx_live_relay_ctx_t       *rctx;
    // must use ngx_sockaddr_t, because sizeof(struct sockaddr)
    //   is not long enouph, content will be covered by other var
    ngx_sockaddr_t              nsa;
    struct sockaddr            *sa;
    socklen_t                   len;

    rctx = ngx_rtmp_get_module_ctx(s, ngx_live_relay_module);
    if (rctx == NULL) {
        return NGX_ERROR;
    }

#define NGX_LIVE_RELAY_CTX(para)                                        \
    if (ngx_copy_str(s->pool, &rctx->para, &relay->para) != NGX_OK) {   \
        goto destroy;                                                   \
    }

    NGX_LIVE_RELAY_CTX(domain);
    NGX_LIVE_RELAY_CTX(app);
    NGX_LIVE_RELAY_CTX(name);
    NGX_LIVE_RELAY_CTX(pargs);
    NGX_LIVE_RELAY_CTX(referer);
    NGX_LIVE_RELAY_CTX(user_agent);
#undef NGX_LIVE_RELAY_CTX

    rctx->tag = relay->tag;

    // get address, host in url must be resolv sync
    sa = (struct sockaddr *) &nsa;
    len = ngx_dynamic_resolver_gethostbyname(&url->url.host, sa);
    if (len == 0) {
        ngx_log_error(NGX_LOG_ERR, s->log, 0,
                "relay httpflv: gethostbyname failed %V", &url->url.host);
        goto destroy;
    }

    // send http request
    if (ngx_live_relay_httpflv_send_request(s, url) != NGX_OK) {
        goto destroy;
    }

    return NGX_OK;

destroy:
    ngx_rtmp_finalize_session(s);

    return NGX_ERROR;
}
