/*
 * acl.c - Manage the ACL (Access Control List)
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

#include <ipset/ipset.h>
#include <ctype.h>

#include "rule.h"
#include "utils.h"
#include "cache.h"
#include "acl.h"

static struct ip_set white_list_ipv4;
static struct ip_set white_list_ipv6;

static struct ip_set black_list_ipv4;
static struct ip_set black_list_ipv6;

static struct cork_dllist black_list_rules;
static struct cork_dllist white_list_rules;

static int acl_mode = BLACK_LIST;

static struct cache *block_list;

static struct ip_set outbound_block_list_ipv4;
static struct ip_set outbound_block_list_ipv6;
static struct cork_dllist outbound_block_list_rules;

#ifdef __linux__

#include <unistd.h>
#include <stdio.h>

#define NO_FIREWALL_MODE 0
#define IPTABLES_MODE    1
#define FIREWALLD_MODE   2

static FILE *shell_stdin;
static int mode = NO_FIREWALL_MODE;

static char chain_name[64];
static char *iptables_init_chain =
    "iptables -N %s; iptables -F %s; iptables -A OUTPUT -p tcp --tcp-flags RST RST -j %s";
static char *iptables_remove_chain =
    "iptables -D OUTPUT -p tcp --tcp-flags RST RST -j %s; iptables -F %s; iptables -X %s";
static char *iptables_add_rule    = "iptables -A %s -d %s -j DROP";
static char *iptables_remove_rule = "iptables -D %s -d %s -j DROP";

static char *ip6tables_init_chain =
    "ip6tables -N %s; ip6tables -F %s; ip6tables -A OUTPUT -p tcp --tcp-flags RST RST -j %s";
static char *ip6tables_remove_chain =
    "ip6tables -D OUTPUT -p tcp --tcp-flags RST RST -j %s; ip6tables -F %s; ip6tables -X %s";
static char *ip6tables_add_rule    = "ip6tables -A %s -d %s -j DROP";
static char *ip6tables_remove_rule = "ip6tables -D %s -d %s -j DROP";

static char *firewalld_init_chain =
    "firewall-cmd --direct --add-chain ipv4 filter %s; \
     firewall-cmd --direct --passthrough ipv4 -F %s; \
     firewall-cmd --direct --passthrough ipv4 -A OUTPUT -p tcp --tcp-flags RST RST -j %s";
static char *firewalld_remove_chain =
    "firewall-cmd --direct --passthrough ipv4 -D OUTPUT -p tcp --tcp-flags RST RST -j %s; \
     firewall-cmd --direct --passthrough ipv4 -F %s; \
     firewall-cmd --direct --remove-chain ipv4 filter %s";
static char *firewalld_add_rule    = "firewall-cmd --direct --passthrough ipv4 -A %s -d %s -j DROP";
static char *firewalld_remove_rule = "firewall-cmd --direct --passthrough ipv4 -D %s -d %s -j DROP";

static char *firewalld6_init_chain =
    "firewall-cmd --direct --add-chain ipv6 filter %s; \
     firewall-cmd --direct --passthrough ipv6 -F %s; \
     firewall-cmd --direct --passthrough ipv6 -A OUTPUT -p tcp --tcp-flags RST RST -j %s";
static char *firewalld6_remove_chain =
    "firewall-cmd --direct --passthrough ipv6 -D OUTPUT -p tcp --tcp-flags RST RST -j %s; \
     firewall-cmd --direct --passthrough ipv6 -F %s; \
     firewall-cmd --direct --remove-chain ipv6 filter %s";
static char *firewalld6_add_rule    = "firewall-cmd --direct --passthrough ipv6 -A %s -d %s -j DROP";
static char *firewalld6_remove_rule = "firewall-cmd --direct --passthrough ipv6 -D %s -d %s -j DROP";

static int
run_cmd(const char *cmd)
{
    int ret = 0;
    char cmdstring[256];

    sprintf(cmdstring, "%s\n", cmd);
    size_t len = strlen(cmdstring);

    if (shell_stdin != NULL) {
        ret = fwrite(cmdstring, 1, len, shell_stdin);
        fflush(shell_stdin);
    }

    return ret == len;
}

static int
init_firewall()
{
    int ret = 0;
    char cli[256];
    FILE *fp;

    if (getuid() != 0)
        return -1;

    sprintf(cli, "firewall-cmd --version 2>&1");
    fp = popen(cli, "r");

    if (fp == NULL)
        return -1;

    if (pclose(fp) == 0) {
        mode = FIREWALLD_MODE;
    } else {
        /* Check whether we have permission to operate iptables.
	 * Note that checking `iptables --version` is insufficient:
         * eg, running within a child user namespace.
	 */
        sprintf(cli, "iptables -L 2>&1");
        fp = popen(cli, "r");
        if (fp == NULL)
            return -1;
        if (pclose(fp) == 0)
            mode = IPTABLES_MODE;
    }

    sprintf(chain_name, "SHADOWSOCKS_LIBEV_%d", getpid());

    if (mode == FIREWALLD_MODE) {
        sprintf(cli, firewalld6_init_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
        sprintf(cli, firewalld_init_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
    } else if (mode == IPTABLES_MODE) {
        sprintf(cli, ip6tables_init_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
        sprintf(cli, iptables_init_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
    }

    shell_stdin = popen("/bin/sh", "w");

    return ret;
}

static int
reset_firewall()
{
    int ret = 0;
    char cli[256];

    if (getuid() != 0)
        return -1;

    if (mode == IPTABLES_MODE) {
        sprintf(cli, ip6tables_remove_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
        sprintf(cli, iptables_remove_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
    } else if (mode == FIREWALLD_MODE) {
        sprintf(cli, firewalld6_remove_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
        sprintf(cli, firewalld_remove_chain, chain_name, chain_name, chain_name);
        ret |= system(cli);
    }

    if (shell_stdin != NULL) {
        run_cmd("exit 0");
        pclose(shell_stdin);
    }

    return ret;
}

static int
set_firewall_rule(char *addr, int add)
{
    char cli[256];
    struct cork_ip ip;

    if (getuid() != 0)
        return -1;

    if (cork_ip_init(&ip, addr))
        return -1;

    if (add) {
        if (mode == IPTABLES_MODE)
            sprintf(cli, ip.version == 4 ? iptables_add_rule : ip6tables_add_rule,
                    chain_name, addr);
        else if (mode == FIREWALLD_MODE)
            sprintf(cli, ip.version == 4 ? firewalld_add_rule : firewalld6_add_rule,
                    chain_name, addr);
        return run_cmd(cli);
    } else {
        if (mode == IPTABLES_MODE)
            sprintf(cli, ip.version == 4 ? iptables_remove_rule : ip6tables_remove_rule,
                    chain_name, addr);
        else if (mode == FIREWALLD_MODE)
            sprintf(cli, ip.version == 4 ? firewalld_remove_rule : firewalld6_remove_rule,
                    chain_name, addr);
        return run_cmd(cli);
    }

    return 0;
}

static void
free_firewall_rule(void *key, void *element)
{
    if (key == NULL)
        return;
    char *addr = (char *)key;
    set_firewall_rule(addr, 0);
    ss_free(element);
}

#endif

void
init_block_list(int firewall)
{
    // Initialize cache
#ifdef __linux__
    if (firewall)
        init_firewall();
    else
        mode = NO_FIREWALL_MODE;
    cache_create(&block_list, 256, free_firewall_rule);
#else
    cache_create(&block_list, 256, NULL);
#endif
}

void
free_block_list()
{
#ifdef __linux__
    if (mode != NO_FIREWALL_MODE)
        reset_firewall();
#endif
    cache_clear(block_list, 0); // Remove all items
}

int
remove_from_block_list(char *addr)
{
    size_t addr_len = strlen(addr);
    return cache_remove(block_list, addr, addr_len);
}

void
clear_block_list()
{
    cache_clear(block_list, 3600); // Clear items older than 1 hour
}

int
check_block_list(char *addr)
{
    size_t addr_len = strlen(addr);

    if (cache_key_exist(block_list, addr, addr_len)) {
        int *count = NULL;
        cache_lookup(block_list, addr, addr_len, &count);

        if (count != NULL && *count > MAX_TRIES)
            return 1;
    }

    return 0;
}

int
update_block_list(char *addr, int err_level)
{
    size_t addr_len = strlen(addr);

    if (cache_key_exist(block_list, addr, addr_len)) {
        int *count = NULL;
        cache_lookup(block_list, addr, addr_len, &count);
        if (count != NULL) {
            if (*count > MAX_TRIES)
                return 1;
            (*count) += err_level;
        }
    } else if (err_level > 0) {
        int *count = (int *)ss_malloc(sizeof(int));
        *count = 1;
        cache_insert(block_list, addr, addr_len, count);
#ifdef __linux__
        if (mode != NO_FIREWALL_MODE)
            set_firewall_rule(addr, 1);
#endif
    }

    return 0;
}

static void
parse_addr_cidr(const char *str, char *host, int *cidr)
{
    int ret = -1, n = 0;
    char *pch;

    pch = strchr(str, '/');
    while (pch != NULL) {
        n++;
        ret = pch - str;
        pch = strchr(pch + 1, '/');
    }
    if (ret == -1) {
        strcpy(host, str);
        *cidr = -1;
    } else {
        memcpy(host, str, ret);
        host[ret] = '\0';
        *cidr     = atoi(str + ret + 1);
    }
}

char *
trimwhitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace(*str))
        str++;

    if (*str == 0)   // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

int
init_acl(const char *path)
{
    // initialize ipset
    ipset_init_library();

    ipset_init(&white_list_ipv4);
    ipset_init(&white_list_ipv6);
    ipset_init(&black_list_ipv4);
    ipset_init(&black_list_ipv6);
    ipset_init(&outbound_block_list_ipv4);
    ipset_init(&outbound_block_list_ipv6);

    cork_dllist_init(&black_list_rules);
    cork_dllist_init(&white_list_rules);
    cork_dllist_init(&outbound_block_list_rules);

    struct ip_set *list_ipv4  = &black_list_ipv4;
    struct ip_set *list_ipv6  = &black_list_ipv6;
    struct cork_dllist *rules = &black_list_rules;

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        LOGE("Invalid acl path.");
        return -1;
    }

    char buf[257];
    while (!feof(f))
        if (fgets(buf, 256, f)) {
            // Trim the newline
            int len = strlen(buf);
            if (len > 0 && buf[len - 1] == '\n') {
                buf[len - 1] = '\0';
            }

            char *line = trimwhitespace(buf);

            // Skip comments
            if (line[0] == '#') {
                continue;
            }

            if (strlen(line) == 0) {
                continue;
            }

            if (strcmp(line, "[outbound_block_list]") == 0) {
                list_ipv4 = &outbound_block_list_ipv4;
                list_ipv6 = &outbound_block_list_ipv6;
                rules     = &outbound_block_list_rules;
                continue;
            } else if (strcmp(line, "[black_list]") == 0
                       || strcmp(line, "[bypass_list]") == 0) {
                list_ipv4 = &black_list_ipv4;
                list_ipv6 = &black_list_ipv6;
                rules     = &black_list_rules;
                continue;
            } else if (strcmp(line, "[white_list]") == 0
                       || strcmp(line, "[proxy_list]") == 0) {
                list_ipv4 = &white_list_ipv4;
                list_ipv6 = &white_list_ipv6;
                rules     = &white_list_rules;
                continue;
            } else if (strcmp(line, "[reject_all]") == 0
                       || strcmp(line, "[bypass_all]") == 0) {
                acl_mode = WHITE_LIST;
                continue;
            } else if (strcmp(line, "[accept_all]") == 0
                       || strcmp(line, "[proxy_all]") == 0) {
                acl_mode = BLACK_LIST;
                continue;
            }

            char host[257];
            int cidr;
            parse_addr_cidr(line, host, &cidr);

            struct cork_ip addr;
            int err = cork_ip_init(&addr, host);
            if (!err) {
                if (addr.version == 4) {
                    if (cidr >= 0) {
                        ipset_ipv4_add_network(list_ipv4, &(addr.ip.v4), cidr);
                    } else {
                        ipset_ipv4_add(list_ipv4, &(addr.ip.v4));
                    }
                } else if (addr.version == 6) {
                    if (cidr >= 0) {
                        ipset_ipv6_add_network(list_ipv6, &(addr.ip.v6), cidr);
                    } else {
                        ipset_ipv6_add(list_ipv6, &(addr.ip.v6));
                    }
                }
            } else {
                rule_t *rule = new_rule();
                accept_rule_arg(rule, line);
                init_rule(rule);
                add_rule(rules, rule);
            }
        }

    fclose(f);

    return 0;
}

void
free_rules(struct cork_dllist *rules)
{
    struct cork_dllist_item *iter;
    while ((iter = cork_dllist_head(rules)) != NULL) {
        rule_t *rule = cork_container_of(iter, rule_t, entries);
        remove_rule(rule);
    }
}

void
free_acl(void)
{
    ipset_done(&black_list_ipv4);
    ipset_done(&black_list_ipv6);
    ipset_done(&white_list_ipv4);
    ipset_done(&white_list_ipv6);

    free_rules(&black_list_rules);
    free_rules(&white_list_rules);
}

int
get_acl_mode(void)
{
    return acl_mode;
}

/*
 * Return 0,  if not match.
 * Return 1,  if match black list.
 * Return -1, if match white list.
 */
int
acl_match_host(const char *host)
{
    struct cork_ip addr;
    int ret = 0;
    int err = cork_ip_init(&addr, host);

    if (err) {
        int host_len = strlen(host);
        if (lookup_rule(&black_list_rules, host, host_len) != NULL)
            ret = 1;
        else if (lookup_rule(&white_list_rules, host, host_len) != NULL)
            ret = -1;
        return ret;
    }

    if (addr.version == 4) {
        if (ipset_contains_ipv4(&black_list_ipv4, &(addr.ip.v4)))
            ret = 1;
        else if (ipset_contains_ipv4(&white_list_ipv4, &(addr.ip.v4)))
            ret = -1;
    } else if (addr.version == 6) {
        if (ipset_contains_ipv6(&black_list_ipv6, &(addr.ip.v6)))
            ret = 1;
        else if (ipset_contains_ipv6(&white_list_ipv6, &(addr.ip.v6)))
            ret = -1;
    }

    return ret;
}

int
acl_add_ip(const char *ip)
{
    struct cork_ip addr;
    int err = cork_ip_init(&addr, ip);
    if (err) {
        return -1;
    }

    if (addr.version == 4) {
        ipset_ipv4_add(&black_list_ipv4, &(addr.ip.v4));
    } else if (addr.version == 6) {
        ipset_ipv6_add(&black_list_ipv6, &(addr.ip.v6));
    }

    return 0;
}

int
acl_remove_ip(const char *ip)
{
    struct cork_ip addr;
    int err = cork_ip_init(&addr, ip);
    if (err) {
        return -1;
    }

    if (addr.version == 4) {
        ipset_ipv4_remove(&black_list_ipv4, &(addr.ip.v4));
    } else if (addr.version == 6) {
        ipset_ipv6_remove(&black_list_ipv6, &(addr.ip.v6));
    }

    return 0;
}

/*
 * Return 0,  if not match.
 * Return 1,  if match black list.
 */
int
outbound_block_match_host(const char *host)
{
    struct cork_ip addr;
    int ret = 0;
    int err = cork_ip_init(&addr, host);

    if (err) {
        int host_len = strlen(host);
        if (lookup_rule(&outbound_block_list_rules, host, host_len) != NULL)
            ret = 1;
        return ret;
    }

    if (addr.version == 4) {
        if (ipset_contains_ipv4(&outbound_block_list_ipv4, &(addr.ip.v4)))
            ret = 1;
    } else if (addr.version == 6) {
        if (ipset_contains_ipv6(&outbound_block_list_ipv6, &(addr.ip.v6)))
            ret = 1;
    }

    return ret;
}
