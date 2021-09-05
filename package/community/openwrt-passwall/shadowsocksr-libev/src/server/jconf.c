/*
 * jconf.c - Parse the JSON format config file
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "jconf.h"
#include "json.h"
#include "string.h"

#include <libcork/core.h>

#define check_json_value_type(value, expected_type, message) \
    do { \
        if ((value)->type != (expected_type)) \
            FATAL((message)); \
    } while(0)

static char *
to_string(const json_value *value)
{
    if (value->type == json_string) {
        return ss_strndup(value->u.string.ptr, value->u.string.length);
    } else if (value->type == json_integer) {
        return strdup(ss_itoa(value->u.integer));
    } else if (value->type == json_null) {
        return "null";
    } else {
        LOGE("%d", value->type);
        FATAL("Invalid config format.");
    }
    return 0;
}

void
free_addr(ss_addr_t *addr)
{
    ss_free(addr->host);
    ss_free(addr->port);
}

void
parse_addr(const char *str, ss_addr_t *addr)
{
    int ipv6 = 0, ret = -1, n = 0;
    char *pch;

    struct cork_ip ip;
    if (cork_ip_init(&ip, str) != -1) {
        addr->host = strdup(str);
        addr->port = NULL;
        return;
    }

    pch = strchr(str, ':');
    while (pch != NULL) {
        n++;
        ret = pch - str;
        pch = strchr(pch + 1, ':');
    }
    if (n > 1) {
        ipv6 = 1;
        if (str[ret - 1] != ']') {
            ret = -1;
        }
    }

    if (ret == -1) {
        if (ipv6) {
            addr->host = ss_strndup(str + 1, strlen(str) - 2);
        } else {
            addr->host = strdup(str);
        }
        addr->port = NULL;
    } else {
        if (ipv6) {
            addr->host = ss_strndup(str + 1, ret - 2);
        } else {
            addr->host = ss_strndup(str, ret);
        }
        addr->port = strdup(str + ret + 1);
    }
}

jconf_t *
read_jconf(const char *file)
{
    static jconf_t conf;

    memset(&conf, 0, sizeof(jconf_t));

    char *buf;
    json_value *obj;

    FILE *f = fopen(file, "rb");
    if (f == NULL) {
        FATAL("Invalid config path.");
    }

    fseek(f, 0, SEEK_END);
    long pos = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (pos >= MAX_CONF_SIZE) {
        FATAL("Too large config file.");
    }

    buf = ss_malloc(pos + 1);
    if (buf == NULL) {
        FATAL("No enough memory.");
    }

    int nread = fread(buf, pos, 1, f);
    if (!nread) {
        FATAL("Failed to read the config file.");
    }
    fclose(f);

    buf[pos] = '\0'; // end of string

    json_settings settings = { 0UL, 0, NULL, NULL, NULL };
    char error_buf[512];
    obj = json_parse_ex(&settings, buf, pos, error_buf);

    if (obj == NULL) {
        FATAL(error_buf);
    }

    if (obj->type == json_object) {
        unsigned int i, j;
        for (i = 0; i < obj->u.object.length; i++) {
            char *name        = obj->u.object.values[i].name;
            json_value *value = obj->u.object.values[i].value;
            if (strcmp(name, "server") == 0) {
                if (value->type == json_array) {
                    for (j = 0; j < value->u.array.length; j++) {
                        if (j >= MAX_REMOTE_NUM) {
                            break;
                        }
                        json_value *v = value->u.array.values[j];
                        char *addr_str = to_string(v);
                        parse_addr(addr_str, conf.remote_addr + j);
                        ss_free(addr_str);
                        conf.remote_num = j + 1;
                    }
                } else if (value->type == json_string) {
                    conf.remote_addr[0].host = to_string(value);
                    conf.remote_addr[0].port = NULL;
                    conf.remote_num          = 1;
                }
            } else if (strcmp(name, "port_password") == 0) {
                if (value->type == json_object) {
                    for (j = 0; j < value->u.object.length; j++) {
                        if (j >= MAX_PORT_NUM) {
                            break;
                        }
                        json_value *v = value->u.object.values[j].value;
                        if (v->type == json_string) {
                            conf.port_password[j].port = ss_strndup(value->u.object.values[j].name,
                                                                    value->u.object.values[j].name_length);
                            conf.port_password[j].password = to_string(v);
                            conf.port_password_num         = j + 1;
                        }
                    }
                }
            } else if (strcmp(name, "server_port") == 0) {
                conf.remote_port = to_string(value);
            } else if (strcmp(name, "local_address") == 0) {
                conf.local_addr = to_string(value);
            } else if (strcmp(name, "local_port") == 0) {
                conf.local_port = to_string(value);
            } else if (strcmp(name, "password") == 0) {
                conf.password = to_string(value);
            } else if (strcmp(name, "protocol") == 0) { // SSR
                conf.protocol = to_string(value);
            } else if (strcmp(name, "protocol_param") == 0) { // SSR
                conf.protocol_param = to_string(value);
            } else if (strcmp(name, "method") == 0) {
                conf.method = to_string(value);
            } else if (strcmp(name, "obfs") == 0) { // SSR
                conf.obfs = to_string(value);
            } else if (strcmp(name, "obfs_param") == 0) { // SSR
                conf.obfs_param = to_string(value);
            } else if (strcmp(name, "timeout") == 0) {
                conf.timeout = to_string(value);
            } else if (strcmp(name, "user") == 0) {
                conf.user = to_string(value);
            } else if (strcmp(name, "fast_open") == 0) {
                check_json_value_type(value, json_boolean,
                        "invalid config file: option 'fast_open' must be a boolean");
                conf.fast_open = value->u.boolean;
            } else if (strcmp(name, "auth") == 0) {
                check_json_value_type(value, json_boolean,
                        "invalid config file: option 'auth' must be a boolean");
                conf.auth = value->u.boolean;
            } else if (strcmp(name, "nofile") == 0) {
                check_json_value_type(value, json_integer,
                    "invalid config file: option 'nofile' must be an integer");
                conf.nofile = value->u.integer;
            } else if (strcmp(name, "nameserver") == 0) {
                conf.nameserver = to_string(value);
            } else if (strcmp(name, "tunnel_address") == 0) {
                conf.tunnel_address = to_string(value);
            } else if (strcmp(name, "mode") == 0) {
                char *mode_str = to_string(value);

                if (strcmp(mode_str, "tcp_only") == 0)
                    conf.mode = TCP_ONLY;
                else if (strcmp(mode_str, "tcp_and_udp") == 0)
                    conf.mode = TCP_AND_UDP;
                else if (strcmp(mode_str, "udp_only") == 0)
                    conf.mode = UDP_ONLY;
                else
                    LOGI("ignore unknown mode: %s, use tcp_only as fallback",
                         mode_str);
                ss_free(mode_str);
            } else if (strcmp(name, "mtu") == 0) {
                check_json_value_type(value, json_integer,
                    "invalid config file: option 'mtu' must be an integer");
                conf.mtu = value->u.integer;
            } else if (strcmp(name, "mptcp") == 0) {
                check_json_value_type(value, json_boolean,
                    "invalid config file: option 'mptcp' must be a boolean");
                conf.mptcp = value->u.boolean;
            } else if (strcmp(name, "ipv6_first") == 0) {
                check_json_value_type(value, json_boolean,
                    "invalid config file: option 'ipv6_first' must be a boolean");
                conf.ipv6_first = value->u.boolean;
            }
        }
    } else {
        FATAL("Invalid config file");
    }

    ss_free(buf);
    json_value_free(obj);
    return &conf;
}
