/*
 * udprelay.c - Setup UDP relay for both client and server
 *
 * Copyright (C) 2013 - 2016, Max Lv <max.c.lv@gmail.com>
 *
 * This file is part of the shadowsocks-libev.
 *
 * shadowsocks-libev is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * shadowsocks-libev is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with shadowsocks-libev; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

#ifndef __MINGW32__
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(HAVE_SYS_IOCTL_H) && defined(HAVE_NET_IF_H) && defined(__linux__)
#include <net/if.h>
#include <sys/ioctl.h>
#define SET_INTERFACE
#endif

#ifdef __MINGW32__
#include "win32.h"
#endif

#include <libcork/core.h>
#include <udns.h>

#include "utils.h"
#include "netutils.h"
#include "cache.h"
#include "udprelay.h"

#ifdef MODULE_REMOTE
#define MAX_UDP_CONN_NUM 512
#else
#define MAX_UDP_CONN_NUM 256
#endif

#ifdef MODULE_REMOTE
#ifdef MODULE_
#error "MODULE_REMOTE and MODULE_LOCAL should not be both defined"
#endif
#endif

#ifndef EAGAIN
#define EAGAIN EWOULDBLOCK
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif

static void server_recv_cb(EV_P_ ev_io *w, int revents);
static void remote_recv_cb(EV_P_ ev_io *w, int revents);
static void remote_timeout_cb(EV_P_ ev_timer *watcher, int revents);

static char *hash_key(const int af, const struct sockaddr_storage *addr);
#ifdef MODULE_REMOTE
static void query_resolve_cb(struct sockaddr *addr, void *data);
#endif
static void close_and_free_remote(EV_P_ remote_ctx_t *ctx);
static remote_ctx_t *new_remote(int fd, server_ctx_t *server_ctx);

#ifdef ANDROID
extern uint64_t tx;
extern uint64_t rx;
extern int vpn;
#endif

extern int verbose;
#ifdef MODULE_REMOTE
extern uint64_t tx;
extern uint64_t rx;
#endif

static int packet_size                               = DEFAULT_PACKET_SIZE;
static int buf_size                                  = DEFAULT_PACKET_SIZE * 2;
static int server_num                                = 0;
static server_ctx_t *server_ctx_list[MAX_REMOTE_NUM] = { NULL };

#ifndef __MINGW32__
static int
setnonblocking(int fd)
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

#endif

#if defined(MODULE_REMOTE) && defined(SO_BROADCAST)
static int
set_broadcast(int socket_fd)
{
    int opt = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
}

#endif

#ifdef SO_NOSIGPIPE
static int
set_nosigpipe(int socket_fd)
{
    int opt = 1;
    return setsockopt(socket_fd, SOL_SOCKET, SO_NOSIGPIPE, &opt, sizeof(opt));
}

#endif

#ifdef MODULE_REDIR

#ifndef IP_TRANSPARENT
#define IP_TRANSPARENT       19
#endif

#ifndef IP_RECVORIGDSTADDR
#define IP_RECVORIGDSTADDR   20
#endif

static int
get_dstaddr(struct msghdr *msg, struct sockaddr_storage *dstaddr)
{
    struct cmsghdr *cmsg;

    for (cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVORIGDSTADDR) {
            memcpy(dstaddr, CMSG_DATA(cmsg), sizeof(struct sockaddr_in));
            dstaddr->ss_family = AF_INET;
            return 0;
        } else if (cmsg->cmsg_level == SOL_IPV6 && cmsg->cmsg_type == IP_RECVORIGDSTADDR) {
            memcpy(dstaddr, CMSG_DATA(cmsg), sizeof(struct sockaddr_in6));
            dstaddr->ss_family = AF_INET6;
            return 0;
        }
    }

    return 1;
}

#endif

#define HASH_KEY_LEN sizeof(struct sockaddr_storage) + sizeof(int)
static char *
hash_key(const int af, const struct sockaddr_storage *addr)
{
    size_t addr_len = sizeof(struct sockaddr_storage);
    static char key[HASH_KEY_LEN];

    memset(key, 0, HASH_KEY_LEN);
    memcpy(key, &af, sizeof(int));
    memcpy(key + sizeof(int), (const uint8_t *)addr, addr_len);

    return key;
}

#if defined(MODULE_REDIR) || defined(MODULE_REMOTE)
static int
construct_udprealy_header(const struct sockaddr_storage *in_addr,
                          char *addr_header)
{
    int addr_header_len = 0;
    if (in_addr->ss_family == AF_INET) {
        struct sockaddr_in *addr = (struct sockaddr_in *)in_addr;
        size_t addr_len          = sizeof(struct in_addr);
        addr_header[addr_header_len++] = 1;
        memcpy(addr_header + addr_header_len, &addr->sin_addr, addr_len);
        addr_header_len += addr_len;
        memcpy(addr_header + addr_header_len, &addr->sin_port, 2);
        addr_header_len += 2;
    } else if (in_addr->ss_family == AF_INET6) {
        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)in_addr;
        size_t addr_len           = sizeof(struct in6_addr);
        addr_header[addr_header_len++] = 4;
        memcpy(addr_header + addr_header_len, &addr->sin6_addr, addr_len);
        addr_header_len += addr_len;
        memcpy(addr_header + addr_header_len, &addr->sin6_port, 2);
        addr_header_len += 2;
    } else {
        return 0;
    }
    return addr_header_len;
}

#endif

static int
parse_udprealy_header(const char *buf, const size_t buf_len,
                      char *host, char *port, struct sockaddr_storage *storage)
{
    const uint8_t atyp = *(uint8_t *)buf;
    int offset         = 1;

    // get remote addr and port
    if ((atyp & ADDRTYPE_MASK) == 1) {
        // IP V4
        size_t in_addr_len = sizeof(struct in_addr);
        if (buf_len >= in_addr_len + 3) {
            if (storage != NULL) {
                struct sockaddr_in *addr = (struct sockaddr_in *)storage;
                addr->sin_family = AF_INET;
                addr->sin_addr   = *(struct in_addr *)(buf + offset);
                addr->sin_port   = *(uint16_t *)(buf + offset + in_addr_len);
            }
            if (host != NULL) {
                dns_ntop(AF_INET, (const void *)(buf + offset),
                         host, INET_ADDRSTRLEN);
            }
            offset += in_addr_len;
        }
    } else if ((atyp & ADDRTYPE_MASK) == 3) {
        // Domain name
        uint8_t name_len = *(uint8_t *)(buf + offset);
        if (name_len + 4 <= buf_len) {
            if (storage != NULL) {
                char tmp[257] = { 0 };
                struct cork_ip ip;
                memcpy(tmp, buf + offset + 1, name_len);
                if (cork_ip_init(&ip, tmp) != -1) {
                    if (ip.version == 4) {
                        struct sockaddr_in *addr = (struct sockaddr_in *)storage;
                        dns_pton(AF_INET, tmp, &(addr->sin_addr));
                        addr->sin_port   = *(uint16_t *)(buf + offset + 1 + name_len);
                        addr->sin_family = AF_INET;
                    } else if (ip.version == 6) {
                        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)storage;
                        dns_pton(AF_INET, tmp, &(addr->sin6_addr));
                        addr->sin6_port   = *(uint16_t *)(buf + offset + 1 + name_len);
                        addr->sin6_family = AF_INET6;
                    }
                }
            }
            if (host != NULL) {
                memcpy(host, buf + offset + 1, name_len);
            }
            offset += 1 + name_len;
        }
    } else if ((atyp & ADDRTYPE_MASK) == 4) {
        // IP V6
        size_t in6_addr_len = sizeof(struct in6_addr);
        if (buf_len >= in6_addr_len + 3) {
            if (storage != NULL) {
                struct sockaddr_in6 *addr = (struct sockaddr_in6 *)storage;
                addr->sin6_family = AF_INET6;
                addr->sin6_addr   = *(struct in6_addr *)(buf + offset);
                addr->sin6_port   = *(uint16_t *)(buf + offset + in6_addr_len);
            }
            if (host != NULL) {
                dns_ntop(AF_INET6, (const void *)(buf + offset),
                         host, INET6_ADDRSTRLEN);
            }
            offset += in6_addr_len;
        }
    }

    if (offset == 1) {
        LOGE("[udp] invalid header with addr type %d", atyp);
        return 0;
    }

    if (port != NULL) {
        sprintf(port, "%d", ntohs(*(uint16_t *)(buf + offset)));
    }
    offset += 2;

    return offset;
}

static char *
get_addr_str(const struct sockaddr *sa)
{
    static char s[SS_ADDRSTRLEN];
    memset(s, 0, SS_ADDRSTRLEN);
    char addr[INET6_ADDRSTRLEN] = { 0 };
    char port[PORTSTRLEN]       = { 0 };
    uint16_t p;

    switch (sa->sa_family) {
    case AF_INET:
        dns_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                 addr, INET_ADDRSTRLEN);
        p = ntohs(((struct sockaddr_in *)sa)->sin_port);
        sprintf(port, "%d", p);
        break;

    case AF_INET6:
        dns_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                 addr, INET6_ADDRSTRLEN);
        p = ntohs(((struct sockaddr_in *)sa)->sin_port);
        sprintf(port, "%d", p);
        break;

    default:
        strncpy(s, "Unknown AF", SS_ADDRSTRLEN);
    }

    int addr_len = strlen(addr);
    int port_len = strlen(port);
    memcpy(s, addr, addr_len);
    memcpy(s + addr_len + 1, port, port_len);
    s[addr_len] = ':';

    return s;
}

int
create_remote_socket(int ipv6)
{
    int remote_sock;

    if (ipv6) {
        // Try to bind IPv6 first
        struct sockaddr_in6 addr;
        memset(&addr, 0, sizeof(struct sockaddr_in6));
        addr.sin6_family = AF_INET6;
        addr.sin6_addr   = in6addr_any;
        addr.sin6_port   = 0;
        remote_sock      = socket(AF_INET6, SOCK_DGRAM, 0);
        if (remote_sock == -1) {
            ERROR("[udp] cannot create socket");
            return -1;
        }
        if (bind(remote_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            FATAL("[udp] cannot bind remote");
            return -1;
        }
    } else {
        // Or else bind to IPv4
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(struct sockaddr_in));
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = 0;
        remote_sock          = socket(AF_INET, SOCK_DGRAM, 0);
        if (remote_sock == -1) {
            ERROR("[udp] cannot create socket");
            return -1;
        }

        if (bind(remote_sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
            FATAL("[udp] cannot bind remote");
            return -1;
        }
    }
    return remote_sock;
}

int
create_server_socket(const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp, *ipv4v6bindall;
    int s, server_sock;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;               /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_DGRAM;              /* We want a UDP socket */
    hints.ai_flags    = AI_PASSIVE | AI_ADDRCONFIG; /* For wildcard IP address */
    hints.ai_protocol = IPPROTO_UDP;

    s = getaddrinfo(host, port, &hints, &result);
    if (s != 0) {
        LOGE("[udp] getaddrinfo: %s", gai_strerror(s));
        return -1;
    }

    rp = result;

    /*
     * On Linux, with net.ipv6.bindv6only = 0 (the default), getaddrinfo(NULL) with
     * AI_PASSIVE returns 0.0.0.0 and :: (in this order). AI_PASSIVE was meant to
     * return a list of addresses to listen on, but it is impossible to listen on
     * 0.0.0.0 and :: at the same time, if :: implies dualstack mode.
     */
    if (!host) {
        ipv4v6bindall = result;

        /* Loop over all address infos found until a IPV6 address is found. */
        while (ipv4v6bindall) {
            if (ipv4v6bindall->ai_family == AF_INET6) {
                rp = ipv4v6bindall; /* Take first IPV6 address available */
                break;
            }
            ipv4v6bindall = ipv4v6bindall->ai_next; /* Get next address info, if any */
        }
    }

    for (/*rp = result*/; rp != NULL; rp = rp->ai_next) {
        server_sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (server_sock == -1) {
            continue;
        }

        if (rp->ai_family == AF_INET6) {
            int ipv6only = host ? 1 : 0;
            setsockopt(server_sock, IPPROTO_IPV6, IPV6_V6ONLY, &ipv6only, sizeof(ipv6only));
        }

        int opt = 1;
        setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_NOSIGPIPE
        set_nosigpipe(server_sock);
#endif
        int err = set_reuseport(server_sock);
        if (err == 0) {
            LOGI("udp port reuse enabled");
        }
#ifdef IP_TOS
        // Set QoS flag
        int tos = 46;
        setsockopt(server_sock, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
#endif

#ifdef MODULE_REDIR
        if (setsockopt(server_sock, SOL_IP, IP_TRANSPARENT, &opt, sizeof(opt))) {
            ERROR("[udp] setsockopt IP_TRANSPARENT");
            exit(EXIT_FAILURE);
        }
        if (setsockopt(server_sock, IPPROTO_IP, IP_RECVORIGDSTADDR, &opt, sizeof(opt))) {
            FATAL("[udp] setsockopt IP_RECVORIGDSTADDR");
        }
#endif

        s = bind(server_sock, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        } else {
            ERROR("[udp] bind");
        }

        close(server_sock);
    }

    if (rp == NULL) {
        LOGE("[udp] cannot bind");
        return -1;
    }

    freeaddrinfo(result);

    return server_sock;
}

remote_ctx_t *
new_remote(int fd, server_ctx_t *server_ctx)
{
    remote_ctx_t *ctx = ss_malloc(sizeof(remote_ctx_t));
    memset(ctx, 0, sizeof(remote_ctx_t));

    ctx->fd         = fd;
    ctx->server_ctx = server_ctx;

    ev_io_init(&ctx->io, remote_recv_cb, fd, EV_READ);
    ev_timer_init(&ctx->watcher, remote_timeout_cb, server_ctx->timeout,
                  server_ctx->timeout);

    return ctx;
}

server_ctx_t *
new_server_ctx(int fd)
{
    server_ctx_t *ctx = ss_malloc(sizeof(server_ctx_t));
    memset(ctx, 0, sizeof(server_ctx_t));

    ctx->fd = fd;

    ev_io_init(&ctx->io, server_recv_cb, fd, EV_READ);

    return ctx;
}

#ifdef MODULE_REMOTE
struct query_ctx *
new_query_ctx(char *buf, size_t len)
{
    struct query_ctx *ctx = ss_malloc(sizeof(struct query_ctx));
    memset(ctx, 0, sizeof(struct query_ctx));
    ctx->buf = ss_malloc(sizeof(buffer_t));
    balloc(ctx->buf, len);
    memcpy(ctx->buf->array, buf, len);
    ctx->buf->len = len;
    return ctx;
}

void
close_and_free_query(EV_P_ struct query_ctx *ctx)
{
    if (ctx != NULL) {
        if (ctx->query != NULL) {
            resolv_cancel(ctx->query);
            ctx->query = NULL;
        }
        if (ctx->buf != NULL) {
            bfree(ctx->buf);
            ss_free(ctx->buf);
        }
        ss_free(ctx);
    }
}

#endif

void
close_and_free_remote(EV_P_ remote_ctx_t *ctx)
{
    if (ctx != NULL) {
        ev_timer_stop(EV_A_ & ctx->watcher);
        ev_io_stop(EV_A_ & ctx->io);
        close(ctx->fd);
        ss_free(ctx);
    }
}

static void
remote_timeout_cb(EV_P_ ev_timer *watcher, int revents)
{
    remote_ctx_t *remote_ctx
        = cork_container_of(watcher, remote_ctx_t, watcher);

    if (verbose) {
        LOGI("[udp] connection timeout");
    }

    char *key = hash_key(remote_ctx->af, &remote_ctx->src_addr);
    cache_remove(remote_ctx->server_ctx->conn_cache, key, HASH_KEY_LEN);
}

#ifdef MODULE_REMOTE
static void
query_resolve_cb(struct sockaddr *addr, void *data)
{
    struct query_ctx *query_ctx = (struct query_ctx *)data;
    struct ev_loop *loop        = query_ctx->server_ctx->loop;

    if (verbose) {
        LOGI("[udp] udns resolved");
    }

    query_ctx->query = NULL;

    if (addr == NULL) {
        LOGE("[udp] udns returned an error");
    } else {
        remote_ctx_t *remote_ctx = query_ctx->remote_ctx;
        int cache_hit            = 0;

        // Lookup in the conn cache
        if (remote_ctx == NULL) {
            char *key = hash_key(AF_UNSPEC, &query_ctx->src_addr);
            cache_lookup(query_ctx->server_ctx->conn_cache, key, HASH_KEY_LEN, (void *)&remote_ctx);
        }

        if (remote_ctx == NULL) {
            int remotefd = create_remote_socket(addr->sa_family == AF_INET6);
            if (remotefd != -1) {
                setnonblocking(remotefd);
#ifdef SO_BROADCAST
                set_broadcast(remotefd);
#endif
#ifdef SO_NOSIGPIPE
                set_nosigpipe(remotefd);
#endif
#ifdef IP_TOS
                // Set QoS flag
                int tos = 46;
                setsockopt(remotefd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
#endif
#ifdef SET_INTERFACE
                if (query_ctx->server_ctx->iface) {
                    if (setinterface(remotefd, query_ctx->server_ctx->iface) == -1)
                        ERROR("setinterface");
                }
#endif
                remote_ctx                  = new_remote(remotefd, query_ctx->server_ctx);
                remote_ctx->src_addr        = query_ctx->src_addr;
                remote_ctx->server_ctx      = query_ctx->server_ctx;
                remote_ctx->addr_header_len = query_ctx->addr_header_len;
                memcpy(remote_ctx->addr_header, query_ctx->addr_header,
                       query_ctx->addr_header_len);
            } else {
                ERROR("[udp] bind() error");
            }
        } else {
            cache_hit = 1;
        }

        if (remote_ctx != NULL) {
            memcpy(&remote_ctx->dst_addr, addr, sizeof(struct sockaddr_storage));

            size_t addr_len = get_sockaddr_len(addr);
            int s           = sendto(remote_ctx->fd, query_ctx->buf->array, query_ctx->buf->len,
                                     0, addr, addr_len);

            if (s == -1) {
                ERROR("[udp] sendto_remote");
                if (!cache_hit) {
                    close_and_free_remote(EV_A_ remote_ctx);
                }
            } else {
                if (!cache_hit) {
                    // Add to conn cache
                    char *key = hash_key(AF_UNSPEC, &remote_ctx->src_addr);
                    cache_insert(query_ctx->server_ctx->conn_cache, key, HASH_KEY_LEN, (void *)remote_ctx);
                    ev_io_start(EV_A_ & remote_ctx->io);
                    ev_timer_start(EV_A_ & remote_ctx->watcher);
                }
            }
        }
    }

    // clean up
    close_and_free_query(EV_A_ query_ctx);
}

#endif

static void
remote_recv_cb(EV_P_ ev_io *w, int revents)
{
    ssize_t r;
    remote_ctx_t *remote_ctx = (remote_ctx_t *)w;
    server_ctx_t *server_ctx = remote_ctx->server_ctx;

    // server has been closed
    if (server_ctx == NULL) {
        LOGE("[udp] invalid server");
        close_and_free_remote(EV_A_ remote_ctx);
        return;
    }

    struct sockaddr_storage src_addr;
    socklen_t src_addr_len = sizeof(struct sockaddr_storage);
    memset(&src_addr, 0, src_addr_len);

    buffer_t *buf = ss_malloc(sizeof(buffer_t));
    balloc(buf, buf_size);

    // recv
    r = recvfrom(remote_ctx->fd, buf->array, buf_size, 0, (struct sockaddr *)&src_addr, &src_addr_len);

    if (r == -1) {
        // error on recv
        // simply drop that packet
        ERROR("[udp] remote_recv_recvfrom");
        goto CLEAN_UP;
    } else if (r > packet_size) {
        LOGE("[udp] remote_recv_recvfrom fragmentation");
        goto CLEAN_UP;
    }

    buf->len = r;

#ifdef MODULE_LOCAL
    int err = ss_decrypt_all(buf, server_ctx->method, 0, buf_size);
    if (err) {
        // drop the packet silently
        goto CLEAN_UP;
    }

    //SSR beg
    if (server_ctx->protocol_plugin) {
        obfs_class *protocol_plugin = server_ctx->protocol_plugin;
        if (protocol_plugin->client_udp_post_decrypt) {
            buf->len = protocol_plugin->client_udp_post_decrypt(server_ctx->protocol, &buf->array, buf->len, &buf->capacity);
            if ((int)buf->len < 0) {
                LOGE("client_udp_post_decrypt");
                close_and_free_remote(EV_A_ remote_ctx);
                return;
            }
            if ( buf->len == 0 )
                return;
        }
    }
    // SSR end

#ifdef MODULE_REDIR
    struct sockaddr_storage dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_storage));
    int len = parse_udprealy_header(buf->array, buf->len, NULL, NULL, &dst_addr);

    if (dst_addr.ss_family != AF_INET && dst_addr.ss_family != AF_INET6) {
        LOGI("[udp] ss-redir does not support domain name");
        goto CLEAN_UP;
    }

    if (verbose) {
        char src[SS_ADDRSTRLEN];
        char dst[SS_ADDRSTRLEN];
        strcpy(src, get_addr_str((struct sockaddr *)&src_addr));
        strcpy(dst, get_addr_str((struct sockaddr *)&dst_addr));
        LOGI("[udp] recv %s via %s", dst, src);
    }
#else
    int len = parse_udprealy_header(buf->array, buf->len, NULL, NULL, NULL);
#endif

    if (len == 0) {
        LOGI("[udp] error in parse header");
        // error in parse header
        goto CLEAN_UP;
    }

    // server may return using a different address type other than the type we
    // have used during sending
#if defined(MODULE_TUNNEL) || defined(MODULE_REDIR)
    // Construct packet
    buf->len -= len;
    memmove(buf->array, buf->array + len, buf->len);
#else
#ifdef ANDROID
    rx += buf->len;
#endif
    // Construct packet
    brealloc(buf, buf->len + 3, buf_size);
    memmove(buf->array + 3, buf->array, buf->len);
    memset(buf->array, 0, 3);
    buf->len += 3;
#endif

#endif

#ifdef MODULE_REMOTE

    rx += buf->len;

    char addr_header_buf[512];
    char *addr_header   = remote_ctx->addr_header;
    int addr_header_len = remote_ctx->addr_header_len;

    if (remote_ctx->af == AF_INET || remote_ctx->af == AF_INET6) {
        addr_header_len = construct_udprealy_header(&src_addr, addr_header_buf);
        addr_header     = addr_header_buf;
    }

    // Construct packet
    brealloc(buf, buf->len + addr_header_len, buf_size);
    memmove(buf->array + addr_header_len, buf->array, buf->len);
    memcpy(buf->array, addr_header, addr_header_len);
    buf->len += addr_header_len;

    int err = ss_encrypt_all(buf, server_ctx->method, 0, buf_size);
    if (err) {
        // drop the packet silently
        goto CLEAN_UP;
    }

#endif

    if (buf->len > packet_size) {
        LOGE("[udp] remote_recv_sendto fragmentation");
        goto CLEAN_UP;
    }

    size_t remote_src_addr_len = get_sockaddr_len((struct sockaddr *)&remote_ctx->src_addr);

#ifdef MODULE_REDIR

    size_t remote_dst_addr_len = get_sockaddr_len((struct sockaddr *)&dst_addr);

    int src_fd = socket(remote_ctx->src_addr.ss_family, SOCK_DGRAM, 0);
    if (src_fd < 0) {
        ERROR("[udp] remote_recv_socket");
        goto CLEAN_UP;
    }
    int opt = 1;
    if (setsockopt(src_fd, SOL_IP, IP_TRANSPARENT, &opt, sizeof(opt))) {
        ERROR("[udp] remote_recv_setsockopt");
        close(src_fd);
        goto CLEAN_UP;
    }
    if (setsockopt(src_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        ERROR("[udp] remote_recv_setsockopt");
        close(src_fd);
        goto CLEAN_UP;
    }
#ifdef IP_TOS
    // Set QoS flag
    int tos = 46;
    setsockopt(src_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
#endif
    if (bind(src_fd, (struct sockaddr *)&dst_addr, remote_dst_addr_len) != 0) {
        ERROR("[udp] remote_recv_bind");
        close(src_fd);
        goto CLEAN_UP;
    }

    int s = sendto(src_fd, buf->array, buf->len, 0,
                   (struct sockaddr *)&remote_ctx->src_addr, remote_src_addr_len);
    if (s == -1) {
        ERROR("[udp] remote_recv_sendto");
        close(src_fd);
        goto CLEAN_UP;
    }
    close(src_fd);

#else

    int s = sendto(server_ctx->fd, buf->array, buf->len, 0,
                   (struct sockaddr *)&remote_ctx->src_addr, remote_src_addr_len);
    if (s == -1) {
        ERROR("[udp] remote_recv_sendto");
        goto CLEAN_UP;
    }

#endif

    // handle the UDP packet successfully,
    // triger the timer
    ev_timer_again(EV_A_ & remote_ctx->watcher);

CLEAN_UP:

    bfree(buf);
    ss_free(buf);
}

static void
server_recv_cb(EV_P_ ev_io *w, int revents)
{
    server_ctx_t *server_ctx = (server_ctx_t *)w;
    struct sockaddr_storage src_addr;
    memset(&src_addr, 0, sizeof(struct sockaddr_storage));

    buffer_t *buf = ss_malloc(sizeof(buffer_t));
    balloc(buf, buf_size);

    socklen_t src_addr_len = sizeof(struct sockaddr_storage);
    unsigned int offset    = 0;

#ifdef MODULE_REDIR
    char control_buffer[64] = { 0 };
    struct msghdr msg;
    memset(&msg, 0, sizeof(struct msghdr));
    struct iovec iov[1];
    struct sockaddr_storage dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_storage));

    msg.msg_name       = &src_addr;
    msg.msg_namelen    = src_addr_len;
    msg.msg_control    = control_buffer;
    msg.msg_controllen = sizeof(control_buffer);

    iov[0].iov_base = buf->array;
    iov[0].iov_len  = buf_size;
    msg.msg_iov     = iov;
    msg.msg_iovlen  = 1;

    buf->len = recvmsg(server_ctx->fd, &msg, 0);
    if (buf->len == -1) {
        ERROR("[udp] server_recvmsg");
        goto CLEAN_UP;
    } else if (buf->len > packet_size) {
        ERROR("[udp] UDP server_recv_recvmsg fragmentation");
        goto CLEAN_UP;
    }

    if (get_dstaddr(&msg, &dst_addr)) {
        LOGE("[udp] unable to get dest addr");
        goto CLEAN_UP;
    }

    src_addr_len = msg.msg_namelen;
#else
    ssize_t r;
    r = recvfrom(server_ctx->fd, buf->array, buf_size,
                 0, (struct sockaddr *)&src_addr, &src_addr_len);

    if (r == -1) {
        // error on recv
        // simply drop that packet
        ERROR("[udp] server_recv_recvfrom");
        goto CLEAN_UP;
    } else if (r > packet_size) {
        ERROR("[udp] server_recv_recvfrom fragmentation");
        goto CLEAN_UP;
    }

    buf->len = r;
#endif

#ifdef MODULE_REMOTE
    tx += buf->len;

    int err = ss_decrypt_all(buf, server_ctx->method, server_ctx->auth, buf_size);
    if (err) {
        // drop the packet silently
        goto CLEAN_UP;
    }
#endif

#ifdef MODULE_LOCAL
#if !defined(MODULE_TUNNEL) && !defined(MODULE_REDIR)
#ifdef ANDROID
    tx += buf->len;
#endif
    uint8_t frag = *(uint8_t *)(buf->array + 2);
    offset += 3;
#endif
#endif

    /*
     *
     * SOCKS5 UDP Request
     * +----+------+------+----------+----------+----------+
     * |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
     * +----+------+------+----------+----------+----------+
     * | 2  |  1   |  1   | Variable |    2     | Variable |
     * +----+------+------+----------+----------+----------+
     *
     * SOCKS5 UDP Response
     * +----+------+------+----------+----------+----------+
     * |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
     * +----+------+------+----------+----------+----------+
     * | 2  |  1   |  1   | Variable |    2     | Variable |
     * +----+------+------+----------+----------+----------+
     *
     * shadowsocks UDP Request (before encrypted)
     * +------+----------+----------+----------+-------------+
     * | ATYP | DST.ADDR | DST.PORT |   DATA   |  HMAC-SHA1  |
     * +------+----------+----------+----------+-------------+
     * |  1   | Variable |    2     | Variable |     10      |
     * +------+----------+----------+----------+-------------+
     *
     * If ATYP & ONETIMEAUTH_FLAG(0x10) != 0, Authentication (HMAC-SHA1) is enabled.
     *
     * The key of HMAC-SHA1 is (IV + KEY) and the input is the whole packet.
     * The output of HMAC-SHA is truncated to 10 bytes (leftmost bits).
     *
     * shadowsocks UDP Response (before encrypted)
     * +------+----------+----------+----------+
     * | ATYP | DST.ADDR | DST.PORT |   DATA   |
     * +------+----------+----------+----------+
     * |  1   | Variable |    2     | Variable |
     * +------+----------+----------+----------+
     *
     * shadowsocks UDP Request and Response (after encrypted)
     * +-------+--------------+
     * |   IV  |    PAYLOAD   |
     * +-------+--------------+
     * | Fixed |   Variable   |
     * +-------+--------------+
     *
     */

#ifdef MODULE_REDIR
    if (verbose) {
        char src[SS_ADDRSTRLEN];
        char dst[SS_ADDRSTRLEN];
        strcpy(src, get_addr_str((struct sockaddr *)&src_addr));
        strcpy(dst, get_addr_str((struct sockaddr *)&dst_addr));
        LOGI("[udp] redir to %s from %s", dst, src);
    }

    char addr_header[512] = { 0 };
    int addr_header_len   = construct_udprealy_header(&dst_addr, addr_header);

    if (addr_header_len == 0) {
        LOGE("[udp] failed to parse tproxy addr");
        goto CLEAN_UP;
    }

    // reconstruct the buffer
    brealloc(buf, buf->len + addr_header_len, buf_size);
    memmove(buf->array + addr_header_len, buf->array, buf->len);
    memcpy(buf->array, addr_header, addr_header_len);
    buf->len += addr_header_len;

#elif MODULE_TUNNEL

    char addr_header[512] = { 0 };
    char *host            = server_ctx->tunnel_addr.host;
    char *port            = server_ctx->tunnel_addr.port;
    uint16_t port_num     = (uint16_t)atoi(port);
    uint16_t port_net_num = htons(port_num);
    int addr_header_len   = 0;

    struct cork_ip ip;
    if (cork_ip_init(&ip, host) != -1) {
        if (ip.version == 4) {
            // send as IPv4
            struct in_addr host_addr;
            memset(&host_addr, 0, sizeof(struct in_addr));
            int host_len = sizeof(struct in_addr);

            if (dns_pton(AF_INET, host, &host_addr) == -1) {
                FATAL("IP parser error");
            }
            addr_header[addr_header_len++] = 1;
            memcpy(addr_header + addr_header_len, &host_addr, host_len);
            addr_header_len += host_len;
        } else if (ip.version == 6) {
            // send as IPv6
            struct in6_addr host_addr;
            memset(&host_addr, 0, sizeof(struct in6_addr));
            int host_len = sizeof(struct in6_addr);

            if (dns_pton(AF_INET6, host, &host_addr) == -1) {
                FATAL("IP parser error");
            }
            addr_header[addr_header_len++] = 4;
            memcpy(addr_header + addr_header_len, &host_addr, host_len);
            addr_header_len += host_len;
        } else {
            FATAL("IP parser error");
        }
    } else {
        // send as domain
        int host_len = strlen(host);

        addr_header[addr_header_len++] = 3;
        addr_header[addr_header_len++] = host_len;
        memcpy(addr_header + addr_header_len, host, host_len);
        addr_header_len += host_len;
    }
    memcpy(addr_header + addr_header_len, &port_net_num, 2);
    addr_header_len += 2;

    // reconstruct the buffer
    brealloc(buf, buf->len + addr_header_len, buf_size);
    memmove(buf->array + addr_header_len, buf->array, buf->len);
    memcpy(buf->array, addr_header, addr_header_len);
    buf->len += addr_header_len;

#else

    char host[257] = { 0 };
    char port[64]  = { 0 };
    struct sockaddr_storage dst_addr;
    memset(&dst_addr, 0, sizeof(struct sockaddr_storage));

    int addr_header_len = parse_udprealy_header(buf->array + offset, buf->len - offset,
                                                host, port, &dst_addr);
    if (addr_header_len == 0) {
        // error in parse header
        goto CLEAN_UP;
    }

    char *addr_header = buf->array + offset;
#endif

#ifdef MODULE_LOCAL
    char *key = hash_key(server_ctx->remote_addr->sa_family, &src_addr);
#else
    char *key = hash_key(dst_addr.ss_family, &src_addr);
#endif

    struct cache *conn_cache = server_ctx->conn_cache;

    remote_ctx_t *remote_ctx = NULL;
    cache_lookup(conn_cache, key, HASH_KEY_LEN, (void *)&remote_ctx);

    if (remote_ctx != NULL) {
        if (sockaddr_cmp(&src_addr, &remote_ctx->src_addr, sizeof(src_addr))) {
            remote_ctx = NULL;
        }
    }

    // reset the timer
    if (remote_ctx != NULL) {
        ev_timer_again(EV_A_ & remote_ctx->watcher);
    }

    if (remote_ctx == NULL) {
        if (verbose) {
#ifdef MODULE_REDIR
            char src[SS_ADDRSTRLEN];
            char dst[SS_ADDRSTRLEN];
            strcpy(src, get_addr_str((struct sockaddr *)&src_addr));
            strcpy(dst, get_addr_str((struct sockaddr *)&dst_addr));
            LOGI("[udp] cache miss: %s <-> %s", dst, src);
#else
            LOGI("[udp] cache miss: %s:%s <-> %s", host, port,
                 get_addr_str((struct sockaddr *)&src_addr));
#endif
        }
    } else {
        if (verbose) {
#ifdef MODULE_REDIR
            char src[SS_ADDRSTRLEN];
            char dst[SS_ADDRSTRLEN];
            strcpy(src, get_addr_str((struct sockaddr *)&src_addr));
            strcpy(dst, get_addr_str((struct sockaddr *)&dst_addr));
            LOGI("[udp] cache hit: %s <-> %s", dst, src);
#else
            LOGI("[udp] cache hit: %s:%s <-> %s", host, port,
                 get_addr_str((struct sockaddr *)&src_addr));
#endif
        }
    }

#ifdef MODULE_LOCAL

#if !defined(MODULE_TUNNEL) && !defined(MODULE_REDIR)
    if (frag) {
        LOGE("[udp] drop a message since frag is not 0, but %d", frag);
        goto CLEAN_UP;
    }
#endif

    const struct sockaddr *remote_addr = server_ctx->remote_addr;
    const int remote_addr_len          = server_ctx->remote_addr_len;

    if (remote_ctx == NULL) {
        // Bind to any port
        int remotefd = create_remote_socket(remote_addr->sa_family == AF_INET6);
        if (remotefd < 0) {
            ERROR("[udp] udprelay bind() error");
            goto CLEAN_UP;
        }
        setnonblocking(remotefd);

#ifdef SO_NOSIGPIPE
        set_nosigpipe(remotefd);
#endif
#ifdef IP_TOS
        // Set QoS flag
        int tos = 46;
        setsockopt(remotefd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
#endif
#ifdef SET_INTERFACE
        if (server_ctx->iface) {
            if (setinterface(remotefd, server_ctx->iface) == -1)
                ERROR("setinterface");
        }
#endif

#ifdef ANDROID
        if (vpn) {
            if (protect_socket(remotefd) == -1) {
                ERROR("protect_socket");
                close(remotefd);
                goto CLEAN_UP;
            }
        }
#endif

        // Init remote_ctx
        remote_ctx                  = new_remote(remotefd, server_ctx);
        remote_ctx->src_addr        = src_addr;
        remote_ctx->af              = remote_addr->sa_family;
        remote_ctx->addr_header_len = addr_header_len;
        memcpy(remote_ctx->addr_header, addr_header, addr_header_len);

        // Add to conn cache
        cache_insert(conn_cache, key, HASH_KEY_LEN, (void *)remote_ctx);

        // Start remote io
        ev_io_start(EV_A_ & remote_ctx->io);
        ev_timer_start(EV_A_ & remote_ctx->watcher);
    }

    if (offset > 0) {
        buf->len -= offset;
        memmove(buf->array, buf->array + offset, buf->len);
    }

    if (server_ctx->auth) {
        buf->array[0] |= ONETIMEAUTH_FLAG;
    }

    // SSR beg
    if (server_ctx->protocol_plugin) {
        obfs_class *protocol_plugin = server_ctx->protocol_plugin;
        if (protocol_plugin->client_udp_pre_encrypt) {
            buf->len = protocol_plugin->client_udp_pre_encrypt(server_ctx->protocol, &buf->array, buf->len, &buf->capacity);
        }
    }
    //SSR end

    int err = ss_encrypt_all(buf, server_ctx->method, server_ctx->auth, buf->len);

    if (err) {
        // drop the packet silently
        goto CLEAN_UP;
    }

    if (buf->len > packet_size) {
        LOGE("[udp] server_recv_sendto fragmentation");
        goto CLEAN_UP;
    }

    int s = sendto(remote_ctx->fd, buf->array, buf->len, 0, remote_addr, remote_addr_len);

    if (s == -1) {
        ERROR("[udp] server_recv_sendto");
    }

#else

    int cache_hit  = 0;
    int need_query = 0;

    if (buf->len - addr_header_len > packet_size) {
        LOGE("[udp] server_recv_sendto fragmentation");
        goto CLEAN_UP;
    }

    if (remote_ctx != NULL) {
        cache_hit = 1;
        // detect destination mismatch
        if (remote_ctx->addr_header_len != addr_header_len
            || memcmp(addr_header, remote_ctx->addr_header, addr_header_len) != 0) {
            if (dst_addr.ss_family != AF_INET && dst_addr.ss_family != AF_INET6) {
                need_query = 1;
            }
        } else {
            memcpy(&dst_addr, &remote_ctx->dst_addr, sizeof(struct sockaddr_storage));
        }
    } else {
        if (dst_addr.ss_family == AF_INET || dst_addr.ss_family == AF_INET6) {
            int remotefd = create_remote_socket(dst_addr.ss_family == AF_INET6);
            if (remotefd != -1) {
                setnonblocking(remotefd);
#ifdef SO_BROADCAST
                set_broadcast(remotefd);
#endif
#ifdef SO_NOSIGPIPE
                set_nosigpipe(remotefd);
#endif
#ifdef IP_TOS
                // Set QoS flag
                int tos = 46;
                setsockopt(remotefd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));
#endif
#ifdef SET_INTERFACE
                if (server_ctx->iface) {
                    if (setinterface(remotefd, server_ctx->iface) == -1)
                        ERROR("setinterface");
                }
#endif
                remote_ctx                  = new_remote(remotefd, server_ctx);
                remote_ctx->src_addr        = src_addr;
                remote_ctx->server_ctx      = server_ctx;
                remote_ctx->addr_header_len = addr_header_len;
                memcpy(remote_ctx->addr_header, addr_header, addr_header_len);
                memcpy(&remote_ctx->dst_addr, &dst_addr, sizeof(struct sockaddr_storage));
            } else {
                ERROR("[udp] bind() error");
                goto CLEAN_UP;
            }
        }
    }

    if (remote_ctx != NULL && !need_query) {
        size_t addr_len = get_sockaddr_len((struct sockaddr *)&dst_addr);
        int s           = sendto(remote_ctx->fd, buf->array + addr_header_len,
                                 buf->len - addr_header_len, 0,
                                 (struct sockaddr *)&dst_addr, addr_len);

        if (s == -1) {
            ERROR("[udp] sendto_remote");
            if (!cache_hit) {
                close_and_free_remote(EV_A_ remote_ctx);
            }
        } else {
            if (!cache_hit) {
                // Add to conn cache
                remote_ctx->af = dst_addr.ss_family;
                char *key = hash_key(remote_ctx->af, &remote_ctx->src_addr);
                cache_insert(server_ctx->conn_cache, key, HASH_KEY_LEN, (void *)remote_ctx);

                ev_io_start(EV_A_ & remote_ctx->io);
                ev_timer_start(EV_A_ & remote_ctx->watcher);
            }
        }
    } else {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family   = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_protocol = IPPROTO_UDP;

        struct query_ctx *query_ctx = new_query_ctx(buf->array + addr_header_len,
                                                    buf->len - addr_header_len);
        query_ctx->server_ctx      = server_ctx;
        query_ctx->addr_header_len = addr_header_len;
        query_ctx->src_addr        = src_addr;
        memcpy(query_ctx->addr_header, addr_header, addr_header_len);

        if (need_query) {
            query_ctx->remote_ctx = remote_ctx;
        }

        struct ResolvQuery *query = resolv_query(host, query_resolve_cb,
                                                 NULL, query_ctx, htons(atoi(port)));
        if (query == NULL) {
            ERROR("[udp] unable to create DNS query");
            close_and_free_query(EV_A_ query_ctx);
            goto CLEAN_UP;
        }
        query_ctx->query = query;
    }
#endif

CLEAN_UP:
    bfree(buf);
    ss_free(buf);
}

void
free_cb(void *key, void *element)
{
    remote_ctx_t *remote_ctx = (remote_ctx_t *)element;

    if (verbose) {
        LOGI("[udp] one connection freed");
    }

    close_and_free_remote(EV_DEFAULT, remote_ctx);
}

int
init_udprelay(const char *server_host, const char *server_port,
#ifdef MODULE_LOCAL
              const struct sockaddr *remote_addr, const int remote_addr_len,
#ifdef MODULE_TUNNEL
              const ss_addr_t tunnel_addr,
#endif
#endif
              int mtu, int method, int auth, int timeout, const char *iface, const char *protocol, const char *protocol_param)
{
    // Initialize ev loop
    struct ev_loop *loop = EV_DEFAULT;

    // Initialize MTU
    if (mtu > 0) {
        packet_size = mtu - 1 - 28 - 2 - 64;
        buf_size    = packet_size * 2;
    }

    // Initialize cache
    struct cache *conn_cache;
    cache_create(&conn_cache, MAX_UDP_CONN_NUM, free_cb);

    // ////////////////////////////////////////////////
    // Setup server context

    // Bind to port
    int serverfd = create_server_socket(server_host, server_port);
    if (serverfd < 0) {
        FATAL("[udp] bind() error");
    }
    setnonblocking(serverfd);
    if (protocol != NULL && strcmp(protocol, "verify_sha1") == 0) {
        auth = 1;
        protocol = NULL;
    }

    server_ctx_t *server_ctx = new_server_ctx(serverfd);
#ifdef MODULE_REMOTE
    server_ctx->loop = loop;
#endif
    server_ctx->auth       = auth;
    server_ctx->timeout    = max(timeout, MIN_UDP_TIMEOUT);
    server_ctx->method     = method;
    server_ctx->iface      = iface;
    server_ctx->conn_cache = conn_cache;
#ifdef MODULE_LOCAL
    server_ctx->remote_addr     = remote_addr;
    server_ctx->remote_addr_len = remote_addr_len;
    //SSR beg
    server_ctx->protocol_plugin = new_obfs_class((char *)protocol);
    if (server_ctx->protocol_plugin) {
        server_ctx->protocol = server_ctx->protocol_plugin->new_obfs();
        server_ctx->protocol_global = server_ctx->protocol_plugin->init_data();
    }

    server_info _server_info;
    memset(&_server_info, 0, sizeof(server_info));
    strcpy(_server_info.host, inet_ntoa(((struct sockaddr_in*)remote_addr)->sin_addr));
    _server_info.port = ((struct sockaddr_in*)remote_addr)->sin_port;
    _server_info.port = _server_info.port >> 8 | _server_info.port << 8;
    _server_info.g_data = server_ctx->protocol_global;
    _server_info.param = (char *)protocol_param;
    _server_info.key = enc_get_key();
    _server_info.key_len = enc_get_key_len();

    if (server_ctx->protocol_plugin)
        server_ctx->protocol_plugin->set_server_info(server_ctx->protocol, &_server_info);
    //SSR end
#ifdef MODULE_TUNNEL
    server_ctx->tunnel_addr = tunnel_addr;
#endif
#endif

    ev_io_start(loop, &server_ctx->io);

    server_ctx_list[server_num++] = server_ctx;

    return 0;
}

void
free_udprelay()
{
    struct ev_loop *loop = EV_DEFAULT;
    while (server_num-- > 0) {
        server_ctx_t *server_ctx = server_ctx_list[server_num];

#ifdef MODULE_LOCAL
        //SSR beg
        if (server_ctx->protocol_plugin) {
            server_ctx->protocol_plugin->dispose(server_ctx->protocol);
            server_ctx->protocol = NULL;
            free_obfs_class(server_ctx->protocol_plugin);
            server_ctx->protocol_plugin = NULL;
        }
        //SSR end
#endif

        ev_io_stop(loop, &server_ctx->io);
        close(server_ctx->fd);
        cache_delete(server_ctx->conn_cache, 0);
        ss_free(server_ctx);
        server_ctx_list[server_num] = NULL;
    }
}
