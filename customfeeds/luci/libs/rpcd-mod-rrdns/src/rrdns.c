/*
 * rrdns - Rapid Reverse DNS lookup plugin for the UBUS RPC server
 *
 *   Copyright (C) 2016 Jo-Philipp Wich <jow@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/nameser.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <resolv.h>

#include <libubox/avl.h>
#include <libubox/usock.h>
#include <libubox/uloop.h>

#include <rpcd/plugin.h>

#include "rrdns.h"


enum {
	RPC_L_ADDRS,
	RPC_L_TIMEOUT,
	RPC_L_SERVER,
	RPC_L_PORT,
	RPC_L_LIMIT,
	__RPC_L_MAX,
};

static const struct blobmsg_policy rpc_lookup_policy[__RPC_L_MAX] = {
	[RPC_L_ADDRS]   = { .name = "addrs",   .type = BLOBMSG_TYPE_ARRAY  },
	[RPC_L_TIMEOUT] = { .name = "timeout", .type = BLOBMSG_TYPE_INT32  },
	[RPC_L_SERVER]  = { .name = "server",  .type = BLOBMSG_TYPE_STRING },
	[RPC_L_PORT]    = { .name = "port",    .type = BLOBMSG_TYPE_INT16  },
	[RPC_L_LIMIT]   = { .name = "limit",   .type = BLOBMSG_TYPE_INT32  },
};


static int
rrdns_cmp_id(const void *k1, const void *k2, void *ptr)
{
	const uint16_t *id1 = k1, *id2 = k2;
	return (*id1 - *id2);
}

static int
rrdns_cmp_addr(const void *k1, const void *k2, void *ptr)
{
	const struct in6_addr *a1 = k1, *a2 = k2;
	return memcmp(a1, a2, sizeof(*a1));
}

static int
rrdns_parse_response(struct rrdns_context *rctx)
{
	int n, len;
	uint16_t id;
	struct rrdns_request *req;
	unsigned char res[512];
	char buf[INET6_ADDRSTRLEN], dname[MAXDNAME];
	HEADER *hdr;
	ns_msg handle;
	ns_rr rr;

	len = recv(rctx->socket.fd, res, sizeof(res), 0);

	if (len < sizeof(*hdr))
		return -ENODATA;

	hdr = (HEADER *)res;
	id  = hdr->id;
	req = avl_find_element(&rctx->request_ids, &id, req, by_id);

	if (!req)
		return -ENOENT;

	avl_delete(&rctx->request_ids, &req->by_id);

	if (ns_initparse(res, len, &handle))
		return -EINVAL;

	for (n = 0; n < ns_msg_count(handle, ns_s_an); n++) {
		if (ns_parserr(&handle, ns_s_an, n, &rr))
			return -EINVAL;

		if (ns_rr_type(rr) != ns_t_ptr)
			continue;

		if (ns_name_uncompress(ns_msg_base(handle), ns_msg_end(handle),
		                       ns_rr_rdata(rr), dname, sizeof(dname)) < 0)
			return -EINVAL;

		inet_ntop(req->family, &req->addr, buf, sizeof(buf));
		blobmsg_add_string(&rctx->blob, buf, dname);
	}

	return 0;
}

static int
rrdns_next_query(struct rrdns_context *rctx)
{
	const char *addr = NULL, *hex = "0123456789abcdef";
	struct rrdns_request *req;
	int i, alen, family;
	char *p, dname[73];

	union {
		unsigned char uchar[4];
		struct in6_addr in6;
		struct in_addr in;
	} a = { };

	union {
		unsigned char buf[512];
		HEADER hdr;
	} msg;

	if (rctx->addr_rem > 0 &&
	    blob_pad_len(rctx->addr_cur) <= rctx->addr_rem &&
	    blob_pad_len(rctx->addr_cur) >= sizeof(struct blob_attr)) {

		addr = blobmsg_get_string(rctx->addr_cur);
		rctx->addr_rem -= blob_pad_len(rctx->addr_cur);
		rctx->addr_cur = blob_next(rctx->addr_cur);
	}

	if (!addr)
		return 0;

	if (inet_pton(AF_INET6, addr, &a.in6)) {
		memset(dname, 0, sizeof(dname));

		for (i = 0, p = dname; i < 16; i++) {
			*p++ = hex[a.in6.s6_addr[15-i] % 16];
			*p++ = '.';
			*p++ = hex[a.in6.s6_addr[15-i] / 16];
			*p++ = '.';
		}

		p += snprintf(p, p - dname - 1, "ip6.arpa");

		family = AF_INET6;
		alen = p - dname;
	}
	else if (inet_pton(AF_INET, addr, &a.in)) {
		family = AF_INET;
		alen = snprintf(dname, sizeof(dname), "%u.%u.%u.%u.in-addr.arpa",
		                a.uchar[3], a.uchar[2], a.uchar[1], a.uchar[0]);
	}
	else {
		return -EINVAL;
	}

	alen = res_mkquery(QUERY, dname, C_IN, T_PTR, NULL, 0, NULL,
	                   msg.buf, sizeof(msg.buf));

	if (alen < 0)
		return alen;

	if (avl_find(&rctx->request_addrs, &a.in6))
		return -ENOTUNIQ;

	if (send(rctx->socket.fd, msg.buf, alen, 0) != alen)
		return -errno;

	req = calloc(1, sizeof(*req));

	if (!req)
		return -ENOMEM;

	req->id = msg.hdr.id;
	req->by_id.key = &req->id;
	avl_insert(&rctx->request_ids, &req->by_id);

	req->family = family;
	req->addr.in6 = a.in6;
	req->by_addr.key = &req->addr.in6;
	avl_insert(&rctx->request_addrs, &req->by_addr);

	return 0;
}

static void
rdns_shutdown(struct rrdns_context *rctx)
{
	struct rrdns_request *req, *tmp;

	uloop_timeout_cancel(&rctx->timeout);
	uloop_fd_delete(&rctx->socket);

	close(rctx->socket.fd);

	ubus_send_reply(rctx->context, &rctx->request, rctx->blob.head);
	ubus_complete_deferred_request(rctx->context, &rctx->request,
	                               UBUS_STATUS_OK);

	avl_remove_all_elements(&rctx->request_addrs, req, by_addr, tmp)
		free(req);

	blob_buf_free(&rctx->blob);
	free(rctx);
}

static void
rrdns_handle_timeout(struct uloop_timeout *utm)
{
	struct rrdns_context *rctx =
		container_of(utm, struct rrdns_context, timeout);

	rdns_shutdown(rctx);
}

static void
rrdns_handle_response(struct uloop_fd *ufd, unsigned int ev)
{
	struct rrdns_context *rctx =
		container_of(ufd, struct rrdns_context, socket);

	int err = rrdns_parse_response(rctx);

	if (err != -ENODATA && err != -ENOENT)
		rrdns_next_query(rctx);

	if (avl_is_empty(&rctx->request_ids))
		rdns_shutdown(rctx);
}

static char *
rrdns_find_nameserver(void)
{
	static char line[2*INET6_ADDRSTRLEN];
	struct in6_addr in6;
	FILE *resolvconf;
	char *p;

	resolvconf = fopen("/etc/resolv.conf", "r");

	if (!resolvconf)
		return NULL;

	while (fgets(line, sizeof(line), resolvconf)) {
		p = strtok(line, " \t");

		if (!p || strcmp(p, "nameserver"))
			continue;

		p = strtok(NULL, " \t\r\n");

		if (!p)
			continue;

		if (!inet_pton(AF_INET6, p, &in6) && !inet_pton(AF_INET, p, &in6))
			continue;

		fclose(resolvconf);
		return p;
	}

	fclose(resolvconf);
	return NULL;
}

static int
rpc_rrdns_lookup(struct ubus_context *ctx, struct ubus_object *obj,
	             struct ubus_request_data *req, const char *method,
	             struct blob_attr *msg)
{
	int port = 53, limit = RRDNS_DEF_LIMIT, timeout = RRDNS_DEF_TIMEOUT;
	struct blob_attr *tb[__RPC_L_MAX];
	struct rrdns_context *rctx;
	const char *server = NULL;

	blobmsg_parse(rpc_lookup_policy, __RPC_L_MAX, tb,
	              blob_data(msg), blob_len(msg));

	if (tb[RPC_L_PORT])
		port = blobmsg_get_u16(tb[RPC_L_PORT]);

	if (tb[RPC_L_LIMIT])
		limit = blobmsg_get_u32(tb[RPC_L_LIMIT]);

	if (tb[RPC_L_TIMEOUT])
		timeout = blobmsg_get_u32(tb[RPC_L_TIMEOUT]);

	if (tb[RPC_L_SERVER])
		server = blobmsg_get_string(tb[RPC_L_SERVER]);


	if (!tb[RPC_L_ADDRS])
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (port <= 0)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (limit <= 0 || limit > RRDNS_MAX_LIMIT)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (timeout <= 0 || timeout > RRDNS_MAX_TIMEOUT)
		return UBUS_STATUS_INVALID_ARGUMENT;


	if (!server || !*server)
		server = rrdns_find_nameserver();

	if (!server)
		return UBUS_STATUS_NOT_FOUND;

	rctx = calloc(1, sizeof(*rctx));

	if (!rctx)
		return UBUS_STATUS_UNKNOWN_ERROR;

	rctx->socket.fd = usock(USOCK_UDP, server, usock_port(port));

	if (rctx->socket.fd < 0) {
		free(rctx);
		return UBUS_STATUS_UNKNOWN_ERROR;
	}

	rctx->context = ctx;
	rctx->addr_cur = blobmsg_data(tb[RPC_L_ADDRS]);
	rctx->addr_rem = blobmsg_data_len(tb[RPC_L_ADDRS]);

	avl_init(&rctx->request_ids, rrdns_cmp_id, false, NULL);
	avl_init(&rctx->request_addrs, rrdns_cmp_addr, false, NULL);

	rctx->timeout.cb = rrdns_handle_timeout;
	uloop_timeout_set(&rctx->timeout, timeout);

	rctx->socket.cb = rrdns_handle_response;
	uloop_fd_add(&rctx->socket, ULOOP_READ);

	blob_buf_init(&rctx->blob, 0);

	while (limit--)
		rrdns_next_query(rctx);

	ubus_defer_request(ctx, req, &rctx->request);

	return UBUS_STATUS_OK;
}


static int
rpc_rrdns_api_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	static const struct ubus_method rrdns_methods[] = {
		UBUS_METHOD("lookup", rpc_rrdns_lookup, rpc_lookup_policy),
	};

	static struct ubus_object_type rrdns_type =
		UBUS_OBJECT_TYPE("rpcd-rrdns", rrdns_methods);

	static struct ubus_object obj = {
		.name = "network.rrdns",
		.type = &rrdns_type,
		.methods = rrdns_methods,
		.n_methods = ARRAY_SIZE(rrdns_methods),
	};

	return ubus_add_object(ctx, &obj);
}

struct rpc_plugin rpc_plugin = {
	.init = rpc_rrdns_api_init
};
