
/*
 * Copyright (C) Roman Arutyunyan
 */


#include <ngx_config.h>
#include <ngx_core.h>
#include "ngx_rtmp.h"

#include <openssl/hmac.h>
#include <openssl/sha.h>


static void ngx_rtmp_handshake_send(ngx_event_t *wev);
static void ngx_rtmp_handshake_recv(ngx_event_t *rev);
static void ngx_rtmp_handshake_done(ngx_rtmp_session_t *s);


/* RTMP handshake :
 *
 *          =peer1=                      =peer2=
 * challenge ----> (.....[digest1]......) ----> 1537 bytes
 * response  <---- (...........[digest2]) <---- 1536 bytes
 *
 *
 * - both packets contain random bytes except for digests
 * - digest1 position is calculated on random packet bytes
 * - digest2 is always at the end of the packet
 *
 * digest1: HMAC_SHA256(packet, peer1_partial_key)
 * digest2: HMAC_SHA256(packet, HMAC_SHA256(digest1, peer2_full_key))
 */


/* Handshake keys */
static u_char
ngx_rtmp_server_key[] = {
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'M', 'e', 'd', 'i', 'a', ' ',
    'S', 'e', 'r', 'v', 'e', 'r', ' ',
    '0', '0', '1',

    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
    0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};


static u_char
ngx_rtmp_client_key[] = {
    'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ',
    'F', 'l', 'a', 's', 'h', ' ', 'P', 'l', 'a', 'y', 'e', 'r', ' ',
    '0', '0', '1',

    0xF0, 0xEE, 0xC2, 0x4A, 0x80, 0x68, 0xBE, 0xE8, 0x2E, 0x00, 0xD0, 0xD1,
    0x02, 0x9E, 0x7E, 0x57, 0x6E, 0xEC, 0x5D, 0x2D, 0x29, 0x80, 0x6F, 0xAB,
    0x93, 0xB8, 0xE6, 0x36, 0xCF, 0xEB, 0x31, 0xAE
};


static const u_char
ngx_rtmp_server_version[4] = {
    0x0D, 0x0E, 0x0A, 0x0D
};


static const u_char
ngx_rtmp_client_version[4] = {
    0x0C, 0x00, 0x0D, 0x0E
};


#define NGX_RTMP_HANDSHAKE_KEYLEN                   SHA256_DIGEST_LENGTH
#define NGX_RTMP_HANDSHAKE_BUFSIZE                  3073
#define NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE           1537


#define NGX_RTMP_HANDSHAKE_SERVER_RECV_CHALLENGE    1
#define NGX_RTMP_HANDSHAKE_SERVER_SEND_CHALLRESP    2
#define NGX_RTMP_HANDSHAKE_SERVER_RECV_RESPONSE     3
#define NGX_RTMP_HANDSHAKE_SERVER_DONE              4


#define NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE    6
#define NGX_RTMP_HANDSHAKE_CLIENT_RECV_CHALLENGE    7
#define NGX_RTMP_HANDSHAKE_CLIENT_RECV_RESPONSE     8
#define NGX_RTMP_HANDSHAKE_CLIENT_SEND_RESPONSE     9
#define NGX_RTMP_HANDSHAKE_CLIENT_DONE              10


static ngx_str_t            ngx_rtmp_server_full_key
    = { sizeof(ngx_rtmp_server_key), ngx_rtmp_server_key };
static ngx_str_t            ngx_rtmp_server_partial_key
    = { 36, ngx_rtmp_server_key };

static ngx_str_t            ngx_rtmp_client_full_key
    = { sizeof(ngx_rtmp_client_key), ngx_rtmp_client_key };
static ngx_str_t            ngx_rtmp_client_partial_key
    = { 30, ngx_rtmp_client_key };


static ngx_int_t
ngx_rtmp_make_digest(ngx_str_t *key, ngx_buf_t *src,
        u_char *skip, u_char *dst, ngx_log_t *log)
{
    static HMAC_CTX        *hmac;
    unsigned int            len;

    if (hmac == NULL) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        static HMAC_CTX  shmac;
        hmac = &shmac;
        HMAC_CTX_init(hmac);
#else
        hmac = HMAC_CTX_new();
        if (hmac == NULL) {
            return NGX_ERROR;
        }
#endif
    }

    HMAC_Init_ex(hmac, key->data, key->len, EVP_sha256(), NULL);

    if (skip && src->pos <= skip && skip <= src->last) {
        if (skip != src->pos) {
            HMAC_Update(hmac, src->pos, skip - src->pos);
        }
        if (src->last != skip + NGX_RTMP_HANDSHAKE_KEYLEN) {
            HMAC_Update(hmac, skip + NGX_RTMP_HANDSHAKE_KEYLEN,
                    src->last - skip - NGX_RTMP_HANDSHAKE_KEYLEN);
        }
    } else {
        HMAC_Update(hmac, src->pos, src->last - src->pos);
    }

    HMAC_Final(hmac, dst, &len);

    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_find_digest(ngx_buf_t *b, ngx_str_t *key, size_t base, ngx_log_t *log)
{
    size_t                  n, offs;
    u_char                  digest[NGX_RTMP_HANDSHAKE_KEYLEN];
    u_char                 *p;

    offs = 0;
    for (n = 0; n < 4; ++n) {
        offs += b->pos[base + n];
    }
    offs = (offs % 728) + base + 4;
    p = b->pos + offs;

    if (ngx_rtmp_make_digest(key, b, p, digest, log) != NGX_OK) {
        return NGX_ERROR;
    }

    if (ngx_memcmp(digest, p, NGX_RTMP_HANDSHAKE_KEYLEN) == 0) {
        return offs;
    }

    return NGX_ERROR;
}


static ngx_int_t
ngx_rtmp_write_digest(ngx_buf_t *b, ngx_str_t *key, size_t base,
        ngx_log_t *log)
{
    size_t                  n, offs;
    u_char                 *p;

    offs = 0;
    for (n = 8; n < 12; ++n) {
        offs += b->pos[base + n];
    }
    offs = (offs % 728) + base + 12;
    p = b->pos + offs;

    if (ngx_rtmp_make_digest(key, b, p, p, log) != NGX_OK) {
        return NGX_ERROR;
    }

    return NGX_OK;
}


static void
ngx_rtmp_fill_random_buffer(ngx_buf_t *b, u_char *end)
{
    for (; b->last != end; ++b->last) {
        *b->last = (u_char) rand();
    }
}


static ngx_buf_t *
ngx_rtmp_alloc_handshake_buffer(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_chain_t                *cl;
    ngx_buf_t                  *b;

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: allocating buffer");

    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);

    if (cscf->free_hs) {
        cl = cscf->free_hs;
        b = cl->buf;
        cscf->free_hs = cl->next;
        ngx_free_chain(cscf->pool, cl);

    } else {
        b = ngx_pcalloc(cscf->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
            return NULL;
        }
        b->memory = 1;
        b->start = ngx_pcalloc(cscf->pool, NGX_RTMP_HANDSHAKE_BUFSIZE);
        if (b->start == NULL) {
            return NULL;
        }
        b->end = b->start + NGX_RTMP_HANDSHAKE_BUFSIZE;
    }

    b->pos = b->last = b->start;

    return b;
}


void
ngx_rtmp_free_handshake_buffers(ngx_rtmp_session_t *s)
{
    ngx_rtmp_core_srv_conf_t   *cscf;
    ngx_chain_t                *cl;

    if (s->hs_buf == NULL) {
        return;
    }
    cscf = ngx_rtmp_get_module_srv_conf(s, ngx_rtmp_core_module);
    cl = ngx_alloc_chain_link(cscf->pool);
    if (cl == NULL) {
        return;
    }
    cl->buf = s->hs_buf;
    cl->next = cscf->free_hs;
    cscf->free_hs = cl;
    s->hs_buf = NULL;
}


static ngx_int_t
ngx_rtmp_handshake_create_challenge(ngx_rtmp_session_t *s,
        const u_char version[4], ngx_str_t *key)
{
    ngx_buf_t          *b;
    u_char             *end;

    b = s->hs_buf;
    b->last = b->pos = b->start;
    end = b->start + NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
    *b->last++ = '\x03';
    b->last = ngx_rtmp_rcpymem(b->last, &s->epoch, 4);
    b->last = ngx_cpymem(b->last, version, 4);
    ngx_rtmp_fill_random_buffer(b, end);
    ++b->pos;
    if (ngx_rtmp_write_digest(b, key, 0, s->log) != NGX_OK) {
        return NGX_ERROR;
    }
    --b->pos;
    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_handshake_parse_challenge(ngx_rtmp_session_t *s,
        ngx_str_t *peer_key, ngx_str_t *key)
{
    ngx_buf_t              *b;
    u_char                 *p;
    ngx_int_t               offs;

    b = s->hs_buf;
    if (*b->pos != '\x03') {
        ngx_log_error(NGX_LOG_INFO, s->log, 0,
                "handshake: unexpected RTMP version: %i",
                (ngx_int_t)*b->pos);
        return NGX_ERROR;
    }
    ++b->pos;
    s->peer_epoch = 0;
    ngx_rtmp_rmemcpy(&s->peer_epoch, b->pos, 4);

    p = b->pos + 4;
    ngx_log_debug5(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: peer version=%i.%i.%i.%i epoch=%uD",
            (ngx_int_t)p[3], (ngx_int_t)p[2],
            (ngx_int_t)p[1], (ngx_int_t)p[0],
            (uint32_t)s->peer_epoch);
    if (*(uint32_t *)p == 0) {
        s->hs_old = 1;
        return NGX_OK;
    }

    offs = ngx_rtmp_find_digest(b, peer_key, 772, s->log);
    if (offs == NGX_ERROR) {
        offs = ngx_rtmp_find_digest(b, peer_key, 8, s->log);
    }
    if (offs == NGX_ERROR) {
        ngx_log_error(NGX_LOG_INFO, s->log, 0, "handshake: digest not found");
        s->hs_old = 1;
        return NGX_OK;
    }
    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: digest found at pos=%i", offs);
    b->pos += offs;
    b->last = b->pos + NGX_RTMP_HANDSHAKE_KEYLEN;
    s->hs_digest = ngx_palloc(s->pool, NGX_RTMP_HANDSHAKE_KEYLEN);
    if (ngx_rtmp_make_digest(key, b, NULL, s->hs_digest, s->log)
            != NGX_OK)
    {
        return NGX_ERROR;
    }
    return NGX_OK;
}


static ngx_int_t
ngx_rtmp_handshake_create_response(ngx_rtmp_session_t *s)
{
    ngx_buf_t          *b;
    u_char             *p, *pos;
    ngx_str_t           key;

    b = s->hs_buf;
    pos = b->pos;
    b->pos = b->start + NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
    ngx_rtmp_fill_random_buffer(b, b->end);
    if (s->hs_digest) {
        p = b->last - NGX_RTMP_HANDSHAKE_KEYLEN;
        key.data = s->hs_digest;
        key.len = NGX_RTMP_HANDSHAKE_KEYLEN;
        if (ngx_rtmp_make_digest(&key, b, p, p, s->log) != NGX_OK) {
            return NGX_ERROR;
        }
    }
    b->pos = pos;

    return NGX_OK;
}


static void
ngx_rtmp_handshake_done(ngx_rtmp_session_t *s)
{
    ngx_rtmp_free_handshake_buffers(s);

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: done");

    if (ngx_rtmp_fire_event(s, NGX_RTMP_HANDSHAKE_DONE,
                NULL, NULL) != NGX_OK)
    {
        ngx_rtmp_finalize_session(s);
        return;
    }

    s->stage = NGX_LIVE_HANDSHAKE_DONE;
    s->handshake_done_time = ngx_current_msec;

    ngx_rtmp_cycle(s);
}


static void
ngx_rtmp_handshake_recv(ngx_event_t *rev)
{
    ssize_t                     n;
    ngx_connection_t           *c;
    ngx_rtmp_session_t         *s;
    ngx_buf_t                  *b;

    c = rev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    if (rev->timedout) {
        ngx_log_error(NGX_LOG_INFO, s->log, NGX_ETIMEDOUT,
                "handshake: recv: client timed out");
        c->timedout = 1;
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (rev->timer_set) {
        ngx_del_timer(rev);
    }

    b = s->hs_buf;

    while (b->last != b->end) {
        n = c->recv(c, b->last, b->end - b->last);

        if (n == NGX_ERROR || n == 0) {
            ngx_rtmp_finalize_session(s);
            return;
        }

        if (n == NGX_AGAIN) {
            ngx_add_timer(rev, s->timeout);
            if (ngx_handle_read_event(c->read, 0) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "handshake: recv: handle read event error");
                ngx_rtmp_finalize_session(s);
            }
            return;
        }

        b->last += n;
    }

    if (rev->active) {
        ngx_del_event(rev, NGX_READ_EVENT, 0);
    }

    ++s->hs_stage;
    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: stage %ui", s->hs_stage);

    switch (s->hs_stage) {
        case NGX_RTMP_HANDSHAKE_SERVER_SEND_CHALLRESP:
            if (ngx_rtmp_handshake_parse_challenge(s,
                    &ngx_rtmp_client_partial_key,
                    &ngx_rtmp_server_full_key) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "handshake: error parsing challenge");
                ngx_rtmp_finalize_session(s);
                return;
            }

            // create version and challege
            if (ngx_rtmp_handshake_create_challenge(s, ngx_rtmp_server_version,
                    &ngx_rtmp_server_partial_key) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "handshake: error creating challenge");
                ngx_rtmp_finalize_session(s);
                return;
            }

            // create response
            if (s->hs_old) {
                ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
                        "handshake: old-style response");
                s->hs_buf->last = s->hs_buf->end;
            } else if (ngx_rtmp_handshake_create_response(s) != NGX_OK) {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "handshake: response error");
                ngx_rtmp_finalize_session(s);
                return;
            }

            ngx_rtmp_handshake_send(c->write);
            break;

        case NGX_RTMP_HANDSHAKE_SERVER_DONE:
            ngx_rtmp_handshake_done(s);
            break;

        case NGX_RTMP_HANDSHAKE_CLIENT_RECV_RESPONSE:
            if (ngx_rtmp_handshake_parse_challenge(s,
                    &ngx_rtmp_server_partial_key,
                    &ngx_rtmp_client_full_key) != NGX_OK)
            {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "handshake: error parsing challenge");
                ngx_rtmp_finalize_session(s);
                return;
            }
            s->hs_buf->pos = s->hs_buf->last = s->hs_buf->start
                           + NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
            ngx_rtmp_handshake_recv(c->read);
            break;

        case NGX_RTMP_HANDSHAKE_CLIENT_SEND_RESPONSE:
            if (ngx_rtmp_handshake_create_response(s) != NGX_OK) {
                ngx_log_error(NGX_LOG_INFO, s->log, 0,
                        "handshake: response error");
                ngx_rtmp_finalize_session(s);
                return;
            }
            ngx_rtmp_handshake_send(c->write);
            break;
    }
}


static void
ngx_rtmp_handshake_send(ngx_event_t *wev)
{
    ngx_int_t                   n;
    ngx_connection_t           *c;
    ngx_rtmp_session_t         *s;
    ngx_buf_t                  *b;

    c = wev->data;
    s = c->data;

    if (c->destroyed) {
        return;
    }

    if (wev->timedout) {
        ngx_log_error(NGX_LOG_INFO, s->log, NGX_ETIMEDOUT,
                "handshake: send: client timed out");
        c->timedout = 1;
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (wev->timer_set) {
        ngx_del_timer(wev);
    }

    b = s->hs_buf;

    while(b->pos != b->last) {
        n = c->send(c, b->pos, b->last - b->pos);

        if (n == NGX_ERROR) {
            ngx_rtmp_finalize_session(s);
            return;
        }

        if (n == NGX_AGAIN || n == 0) {
            ngx_add_timer(c->write, s->timeout);
            if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
                ngx_log_error(NGX_LOG_ERR, s->log, 0,
                        "handshake: recv: handle write event error");
                ngx_rtmp_finalize_session(s);
            }
            return;
        }

        b->pos += n;
    }

    if (wev->active) {
        ngx_del_event(wev, NGX_WRITE_EVENT, 0);
    }

    ++s->hs_stage;
    ngx_log_debug1(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: stage %ui", s->hs_stage);

    switch (s->hs_stage) {

        case NGX_RTMP_HANDSHAKE_SERVER_RECV_RESPONSE:
            s->hs_buf->last = s->hs_buf->start
                            + NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
            ngx_rtmp_handshake_recv(c->read);
            break;

        case NGX_RTMP_HANDSHAKE_CLIENT_RECV_CHALLENGE:
            s->hs_buf->pos = s->hs_buf->last = s->hs_buf->end
                           - NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
            ngx_rtmp_handshake_recv(c->read);
            break;

        case NGX_RTMP_HANDSHAKE_CLIENT_DONE:
            ngx_rtmp_handshake_done(s);
            break;
    }
}


void
ngx_rtmp_handshake(ngx_rtmp_session_t *s)
{
    ngx_connection_t           *c;

    c = s->connection;
    c->read->handler =  ngx_rtmp_handshake_recv;
    c->write->handler = ngx_rtmp_handshake_send;

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: start server handshake");

    s->hs_buf = ngx_rtmp_alloc_handshake_buffer(s);
    s->hs_buf->pos = s->hs_buf->last = s->hs_buf->end
                   - NGX_RTMP_HANDSHAKE_CHALLENGE_SIZE;
    s->hs_stage = NGX_RTMP_HANDSHAKE_SERVER_RECV_CHALLENGE;

    ngx_rtmp_handshake_recv(c->read);
}


void
ngx_rtmp_client_handshake(ngx_rtmp_session_t *s, unsigned async)
{
    ngx_connection_t           *c;

    c = s->connection;
    c->read->handler =  ngx_rtmp_handshake_recv;
    c->write->handler = ngx_rtmp_handshake_send;

    ngx_log_debug0(NGX_LOG_DEBUG_RTMP, s->log, 0,
            "handshake: start client handshake");

    s->hs_buf = ngx_rtmp_alloc_handshake_buffer(s);
    s->hs_stage = NGX_RTMP_HANDSHAKE_CLIENT_SEND_CHALLENGE;

    if (ngx_rtmp_handshake_create_challenge(s,
                ngx_rtmp_client_version,
                &ngx_rtmp_client_partial_key) != NGX_OK)
    {
        ngx_rtmp_finalize_session(s);
        return;
    }

    if (async) {
        ngx_add_timer(c->write, s->timeout);
        if (ngx_handle_write_event(c->write, 0) != NGX_OK) {
            ngx_rtmp_finalize_session(s);
        }
        return;
    }

    ngx_rtmp_handshake_send(c->write);
}

