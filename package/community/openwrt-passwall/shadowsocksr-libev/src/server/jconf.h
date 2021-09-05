/*
 * jconf.h - Define the config data structure
 *
 * Copyright (C) 2013 - 2016, Max Lv <max.c.lv@gmail.com>
 *
 * This file is part of the shadowsocks-libev.
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

#ifndef _JCONF_H
#define _JCONF_H

#define MAX_PORT_NUM 1024
#define MAX_REMOTE_NUM 10
#define MAX_CONF_SIZE 128 * 1024
#define MAX_DNS_NUM 4
#define MAX_CONNECT_TIMEOUT 10
#define MAX_REQUEST_TIMEOUT 60
#define MIN_UDP_TIMEOUT 10

#define TCP_ONLY     0
#define TCP_AND_UDP  1
#define UDP_ONLY     3

typedef struct {
    char *host;
    char *port;
} ss_addr_t;

typedef struct {
    char *port;
    char *password;
} ss_port_password_t;

typedef struct {
    int remote_num;
    ss_addr_t remote_addr[MAX_REMOTE_NUM];
    int port_password_num;
    ss_port_password_t port_password[MAX_PORT_NUM];
    char *remote_port;
    char *local_addr;
    char *local_port;
    char *password;
    char *protocol; // SSR
    char *protocol_param; // SSR
    char *method;
    char *obfs; // SSR
    char *obfs_param; // SSR
    char *timeout;
    char *user;
    int auth;
    int fast_open;
    int nofile;
    char *nameserver;
    char *tunnel_address;
    int mode;
    int mtu;
    int mptcp;
    int ipv6_first;
} jconf_t;

jconf_t *read_jconf(const char *file);
void parse_addr(const char *str, ss_addr_t *addr);
void free_addr(ss_addr_t *addr);

#endif // _JCONF_H
