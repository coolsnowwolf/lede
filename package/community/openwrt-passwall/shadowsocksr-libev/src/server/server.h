/*
 * server.h - Define shadowsocks server's buffers and callbacks
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

#ifndef _SERVER_H
#define _SERVER_H

#include <ev.h>
#include <time.h>
#include <libcork/ds.h>

#include "encrypt.h"
#include "jconf.h"
#include "resolv.h"
#include "obfs.h"
#include "protocol.h"

#include "common.h"

typedef struct listen_ctx {
    ev_io io;
    int fd;
    int timeout;
    int method;
    char *iface;
    struct ev_loop *loop;

    // SSR
    char *protocol_name;
    char *protocol_param;
    char *obfs_name;
    char *obfs_param;
    void **list_protocol_global;
    void **list_obfs_global;
} listen_ctx_t;

typedef struct server_ctx {
    ev_io io;
    ev_timer watcher;
    int connected;
    struct server *server;
} server_ctx_t;

typedef struct server {
    int fd;
    int stage;
    buffer_t *buf;
    ssize_t buf_capacity;
    buffer_t *header_buf;

    int auth;
    struct chunk *chunk;

    struct enc_ctx *e_ctx;
    struct enc_ctx *d_ctx;
    struct server_ctx *recv_ctx;
    struct server_ctx *send_ctx;
    struct listen_ctx *listen_ctx;
    struct remote *remote;

    struct ResolvQuery *query;

    struct cork_dllist_item entries;

    // SSR
    obfs *protocol;
    obfs *obfs;
    obfs_class *protocol_plugin;
    obfs_class *obfs_plugin;
    int obfs_compatible_state;
    int protocol_compatible_state;
} server_t;

typedef struct query {
    server_t *server;
    char hostname[257];
} query_t;

typedef struct remote_ctx {
    ev_io io;
    int connected;
    struct remote *remote;
} remote_ctx_t;

typedef struct remote {
    int fd;
    buffer_t *buf;
    ssize_t buf_capacity;
    struct remote_ctx *recv_ctx;
    struct remote_ctx *send_ctx;
    struct server *server;

    // SSR
    int remote_index;
} remote_t;

#endif // _SERVER_H
