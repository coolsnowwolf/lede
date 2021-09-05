/*
 * Copyright (c) 2014, Dustin Lundquist <dustin@null-ptr.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ev.h>
#include <udns.h>

#ifdef __MINGW32__
#include "win32.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#endif

#include "resolv.h"
#include "utils.h"
#include "netutils.h"

/*
 * Implement DNS resolution interface using libudns
 */

struct ResolvQuery {
    void (*client_cb)(struct sockaddr *, void *);
    void (*client_free_cb)(void *);
    void *client_cb_data;
    struct dns_query *queries[2];
    size_t response_count;
    struct sockaddr **responses;
    uint16_t port;
};

extern int verbose;

static struct ev_io resolv_io_watcher;
static struct ev_timer resolv_timeout_watcher;
static const int MODE_IPV4_ONLY  = 0;
static const int MODE_IPV6_ONLY  = 1;
static const int MODE_IPV4_FIRST = 2;
static const int MODE_IPV6_FIRST = 3;
static int resolv_mode           = 0;

static void resolv_sock_cb(struct ev_loop *, struct ev_io *, int);
static void resolv_timeout_cb(struct ev_loop *, struct ev_timer *, int);
static void dns_query_v4_cb(struct dns_ctx *, struct dns_rr_a4 *, void *);
static void dns_query_v6_cb(struct dns_ctx *, struct dns_rr_a6 *, void *);
static void dns_timer_setup_cb(struct dns_ctx *, int, void *);
static void process_client_callback(struct ResolvQuery *);
static inline int all_queries_are_null(struct ResolvQuery *);
static struct sockaddr *choose_ipv4_first(struct ResolvQuery *);
static struct sockaddr *choose_ipv6_first(struct ResolvQuery *);
static struct sockaddr *choose_any(struct ResolvQuery *);

int
resolv_init(struct ev_loop *loop, char **nameservers, int nameserver_num, int ipv6first)
{
    if (ipv6first)
        resolv_mode = MODE_IPV6_FIRST;
    else
        resolv_mode = MODE_IPV4_FIRST;

    struct dns_ctx *ctx = &dns_defctx;
    if (nameservers == NULL) {
        /* Nameservers not specified, use system resolver config */
        dns_init(ctx, 0);
    } else {
        dns_reset(ctx);

        for (int i = 0; i < nameserver_num; i++) {
            char *server = nameservers[i];
            dns_add_serv(ctx, server);
        }
    }

    int sockfd = dns_open(ctx);
    if (sockfd < 0) {
        FATAL("Failed to open DNS resolver socket");
    }

    if (nameserver_num == 1 && nameservers != NULL) {
        if (strncmp("127.0.0.1", nameservers[0], 9) == 0
            || strncmp("::1", nameservers[0], 3) == 0) {
            if (verbose) {
                LOGI("bind UDP resolver to %s", nameservers[0]);
            }
            if (bind_to_address(sockfd, nameservers[0]) == -1)
                ERROR("bind_to_address");
        }
    }

#ifdef __MINGW32__
    setnonblocking(sockfd);
#else
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
#endif

    ev_io_init(&resolv_io_watcher, resolv_sock_cb, sockfd, EV_READ);
    resolv_io_watcher.data = ctx;

    ev_io_start(loop, &resolv_io_watcher);

    ev_timer_init(&resolv_timeout_watcher, resolv_timeout_cb, 0.0, 0.0);
    resolv_timeout_watcher.data = ctx;

    dns_set_tmcbck(ctx, dns_timer_setup_cb, loop);

    return sockfd;
}

void
resolv_shutdown(struct ev_loop *loop)
{
    struct dns_ctx *ctx = (struct dns_ctx *)resolv_io_watcher.data;

    ev_io_stop(loop, &resolv_io_watcher);

    if (ev_is_active(&resolv_timeout_watcher)) {
        ev_timer_stop(loop, &resolv_timeout_watcher);
    }

    dns_close(ctx);
}

struct ResolvQuery *
resolv_query(const char *hostname, void (*client_cb)(struct sockaddr *, void *),
             void (*client_free_cb)(void *), void *client_cb_data,
             uint16_t port)
{
    struct dns_ctx *ctx = (struct dns_ctx *)resolv_io_watcher.data;

    /*
     * Wrap udns's call back in our own
     */
    struct ResolvQuery *cb_data = ss_malloc(sizeof(struct ResolvQuery));
    if (cb_data == NULL) {
        LOGE("Failed to allocate memory for DNS query callback data.");
        return NULL;
    }
    memset(cb_data, 0, sizeof(struct ResolvQuery));

    cb_data->client_cb      = client_cb;
    cb_data->client_free_cb = client_free_cb;
    cb_data->client_cb_data = client_cb_data;
    memset(cb_data->queries, 0, sizeof(cb_data->queries));
    cb_data->response_count = 0;
    cb_data->responses      = NULL;
    cb_data->port           = port;

    /* Submit A and AAAA queries */
    if (resolv_mode != MODE_IPV6_ONLY) {
        cb_data->queries[0] = dns_submit_a4(ctx,
                                            hostname, 0,
                                            dns_query_v4_cb, cb_data);
        if (cb_data->queries[0] == NULL) {
            LOGE("Failed to submit DNS query: %s",
                 dns_strerror(dns_status(ctx)));
        }
    }

    if (resolv_mode != MODE_IPV4_ONLY) {
        cb_data->queries[1] = dns_submit_a6(ctx,
                                            hostname, 0,
                                            dns_query_v6_cb, cb_data);
        if (cb_data->queries[1] == NULL) {
            LOGE("Failed to submit DNS query: %s",
                 dns_strerror(dns_status(ctx)));
        }
    }

    if (all_queries_are_null(cb_data)) {
        if (cb_data->client_free_cb != NULL) {
            cb_data->client_free_cb(cb_data->client_cb_data);
        }
        ss_free(cb_data);
    }

    return cb_data;
}

void
resolv_cancel(struct ResolvQuery *query_handle)
{
    struct ResolvQuery *cb_data = (struct ResolvQuery *)query_handle;
    struct dns_ctx *ctx         = (struct dns_ctx *)resolv_io_watcher.data;

    for (int i = 0; i < sizeof(cb_data->queries) / sizeof(cb_data->queries[0]);
         i++)
        if (cb_data->queries[i] != NULL) {
            dns_cancel(ctx, cb_data->queries[i]);
            ss_free(cb_data->queries[i]);
        }

    if (cb_data->client_free_cb != NULL) {
        cb_data->client_free_cb(cb_data->client_cb_data);
    }

    ss_free(cb_data);
}

/*
 * DNS UDP socket activity callback
 */
static void
resolv_sock_cb(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct dns_ctx *ctx = (struct dns_ctx *)w->data;

    if (revents & EV_READ) {
        dns_ioevent(ctx, ev_now(loop));
    }
}

/*
 * Wrapper for client callback we provide to udns
 */
static void
dns_query_v4_cb(struct dns_ctx *ctx, struct dns_rr_a4 *result, void *data)
{
    struct ResolvQuery *cb_data = (struct ResolvQuery *)data;

    if (result == NULL) {
        if (verbose) {
            LOGI("IPv4 resolv: %s", dns_strerror(dns_status(ctx)));
        }
    } else if (result->dnsa4_nrr > 0) {
        struct sockaddr **new_responses = ss_realloc(cb_data->responses,
                                                     (cb_data->response_count +
                                                      result->dnsa4_nrr) *
                                                     sizeof(struct sockaddr *));
        if (new_responses == NULL) {
            LOGE("Failed to allocate memory for additional DNS responses");
        } else {
            cb_data->responses = new_responses;

            for (int i = 0; i < result->dnsa4_nrr; i++) {
                struct sockaddr_in *sa =
                    (struct sockaddr_in *)ss_malloc(sizeof(struct sockaddr_in));
                sa->sin_family = AF_INET;
                sa->sin_port   = cb_data->port;
                sa->sin_addr   = result->dnsa4_addr[i];

                cb_data->responses[cb_data->response_count] =
                    (struct sockaddr *)sa;
                if (cb_data->responses[cb_data->response_count] == NULL) {
                    LOGE(
                        "Failed to allocate memory for DNS query result address");
                } else {
                    cb_data->response_count++;
                }
            }
        }
    }

    ss_free(result);
    cb_data->queries[0] = NULL; /* mark A query as being completed */

    /* Once all queries have completed, call client callback */
    if (all_queries_are_null(cb_data)) {
        return process_client_callback(cb_data);
    }
}

static void
dns_query_v6_cb(struct dns_ctx *ctx, struct dns_rr_a6 *result, void *data)
{
    struct ResolvQuery *cb_data = (struct ResolvQuery *)data;

    if (result == NULL) {
        if (verbose) {
            LOGI("IPv6 resolv: %s", dns_strerror(dns_status(ctx)));
        }
    } else if (result->dnsa6_nrr > 0) {
        struct sockaddr **new_responses = ss_realloc(cb_data->responses,
                                                     (cb_data->response_count +
                                                      result->dnsa6_nrr) *
                                                     sizeof(struct sockaddr *));
        if (new_responses == NULL) {
            LOGE("Failed to allocate memory for additional DNS responses");
        } else {
            cb_data->responses = new_responses;

            for (int i = 0; i < result->dnsa6_nrr; i++) {
                struct sockaddr_in6 *sa =
                    (struct sockaddr_in6 *)ss_malloc(sizeof(struct sockaddr_in6));
                sa->sin6_family = AF_INET6;
                sa->sin6_port   = cb_data->port;
                sa->sin6_addr   = result->dnsa6_addr[i];

                cb_data->responses[cb_data->response_count] =
                    (struct sockaddr *)sa;
                if (cb_data->responses[cb_data->response_count] == NULL) {
                    LOGE(
                        "Failed to allocate memory for DNS query result address");
                } else {
                    cb_data->response_count++;
                }
            }
        }
    }

    ss_free(result);
    cb_data->queries[1] = NULL; /* mark AAAA query as being completed */

    /* Once all queries have completed, call client callback */
    if (all_queries_are_null(cb_data)) {
        return process_client_callback(cb_data);
    }
}

/*
 * Called once all queries have been completed
 */
static void
process_client_callback(struct ResolvQuery *cb_data)
{
    struct sockaddr *best_address = NULL;

    if (resolv_mode == MODE_IPV4_FIRST) {
        best_address = choose_ipv4_first(cb_data);
    } else if (resolv_mode == MODE_IPV6_FIRST) {
        best_address = choose_ipv6_first(cb_data);
    } else {
        best_address = choose_any(cb_data);
    }

    cb_data->client_cb(best_address, cb_data->client_cb_data);

    for (int i = 0; i < cb_data->response_count; i++)
        ss_free(cb_data->responses[i]);

    ss_free(cb_data->responses);
    if (cb_data->client_free_cb != NULL) {
        cb_data->client_free_cb(cb_data->client_cb_data);
    }
    ss_free(cb_data);
}

static struct sockaddr *
choose_ipv4_first(struct ResolvQuery *cb_data)
{
    for (int i = 0; i < cb_data->response_count; i++)
        if (cb_data->responses[i]->sa_family == AF_INET) {
            return cb_data->responses[i];
        }

    return choose_any(cb_data);
}

static struct sockaddr *
choose_ipv6_first(struct ResolvQuery *cb_data)
{
    for (int i = 0; i < cb_data->response_count; i++)
        if (cb_data->responses[i]->sa_family == AF_INET6) {
            return cb_data->responses[i];
        }

    return choose_any(cb_data);
}

static struct sockaddr *
choose_any(struct ResolvQuery *cb_data)
{
    if (cb_data->response_count >= 1) {
        return cb_data->responses[0];
    }

    return NULL;
}

/*
 * DNS timeout callback
 */
static void
resolv_timeout_cb(struct ev_loop *loop, struct ev_timer *w, int revents)
{
    struct dns_ctx *ctx = (struct dns_ctx *)w->data;

    if (revents & EV_TIMER) {
        dns_timeouts(ctx, 30, ev_now(loop));
    }
}

/*
 * Callback to setup DNS timeout callback
 */
static void
dns_timer_setup_cb(struct dns_ctx *ctx, int timeout, void *data)
{
    struct ev_loop *loop = (struct ev_loop *)data;

    if (ev_is_active(&resolv_timeout_watcher)) {
        ev_timer_stop(loop, &resolv_timeout_watcher);
    }

    if (ctx != NULL && timeout >= 0) {
        ev_timer_set(&resolv_timeout_watcher, timeout, 0.0);
        ev_timer_start(loop, &resolv_timeout_watcher);
    }
}

static inline int
all_queries_are_null(struct ResolvQuery *cb_data)
{
    int result = 1;

    for (int i = 0; i < sizeof(cb_data->queries) / sizeof(cb_data->queries[0]);
         i++)
        result = result && cb_data->queries[i] == NULL;

    return result;
}
