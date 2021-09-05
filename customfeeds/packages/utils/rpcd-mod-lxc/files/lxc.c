/*
 * rpcd-lxc-plugin
 *
 * Copyright (C) 2014 Cisco Systems, Inc.
 * Author: Luka Perkov <luka@openwrt.org>
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

#include <libubus.h>

#include <lxc/lxccontainer.h>

#include <rpcd/plugin.h>

static struct blob_buf buf;

struct rpc_lxc {
	/* ubus options */
	char *name;
	char *config;
	/* lxc container */
	struct lxc_container *container;
};

enum {
	RPC_LXC_NAME,
	RPC_LXC_CONFIG,
	__RPC_LXC_MAX,
};

enum {
	RPC_LXC_SHUTDOWN_NAME,
	RPC_LXC_SHUTDOWN_CONFIG,
	RPC_LXC_SHUTDOWN_TIMEOUT,
	__RPC_LXC_SHUTDOWN_MAX,
};

enum {
	RPC_LXC_RENAME_NAME,
	RPC_LXC_RENAME_CONFIG,
	RPC_LXC_RENAME_NEWNAME,
	__RPC_LXC_RENAME_MAX,
};

enum {
	RPC_LXC_CREATE_NAME,
	RPC_LXC_CREATE_CONFIG,
	RPC_LXC_CREATE_TEMPLATE,
	RPC_LXC_CREATE_FLAGS,
	RPC_LXC_CREATE_ARGS,
	__RPC_LXC_CREATE_MAX,
};

static const struct blobmsg_policy rpc_lxc_min_policy[__RPC_LXC_MAX] = {
	[RPC_LXC_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy rpc_lxc_shutdown_policy[__RPC_LXC_SHUTDOWN_MAX] = {
	[RPC_LXC_SHUTDOWN_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_SHUTDOWN_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_SHUTDOWN_TIMEOUT] = { .name = "timeout", .type = BLOBMSG_TYPE_INT32 },
};

static const struct blobmsg_policy rpc_lxc_rename_policy[__RPC_LXC_RENAME_MAX] = {
	[RPC_LXC_RENAME_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_RENAME_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_RENAME_NEWNAME] = { .name = "newname", .type = BLOBMSG_TYPE_STRING },
};

static const struct blobmsg_policy rpc_lxc_create_policy[__RPC_LXC_CREATE_MAX] = {
	[RPC_LXC_CREATE_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_CREATE_CONFIG] = { .name = "config", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_CREATE_TEMPLATE] = { .name = "template", .type = BLOBMSG_TYPE_STRING },
	[RPC_LXC_CREATE_FLAGS] = { .name = "flags", .type = BLOBMSG_TYPE_INT32 },
	[RPC_LXC_CREATE_ARGS] = { .name = "args", .type = BLOBMSG_TYPE_ARRAY },
};

static struct rpc_lxc *
rpc_lxc_init(struct blob_attr *tb[__RPC_LXC_MAX])
{
	struct rpc_lxc *l = NULL;

	l = calloc(1, sizeof(struct rpc_lxc));
	if (!l) return NULL;

	if (tb[RPC_LXC_NAME]) {
		l->name = blobmsg_data(tb[RPC_LXC_NAME]);
	} else {
		goto error;
	}

	if (tb[RPC_LXC_CONFIG]) {
		l->config = blobmsg_data(tb[RPC_LXC_CONFIG]);
	} else {
		l->config = NULL;
	}

	l->container = lxc_container_new(l->name, l->config);
	if (!l->container) {
		goto error;
	}

	return l;
error:
	free(l);
	return NULL;
}

static void
rpc_lxc_done(struct rpc_lxc *l)
{
	if (l) {
		lxc_container_put(l->container);
		free(l);
	}

	return;
}

static int
rpc_lxc_start(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->start(l->container, 0, NULL)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}


static int
rpc_lxc_reboot(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->reboot(l->container)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_shutdown(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_SHUTDOWN_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_shutdown_policy, __RPC_LXC_SHUTDOWN_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	/* define default timeout */
	int timeout = 30;
	if (tb[RPC_LXC_SHUTDOWN_TIMEOUT]) {
		timeout = blobmsg_get_u32(tb[RPC_LXC_SHUTDOWN_TIMEOUT]);
	}

	if (!l->container->shutdown(l->container, timeout)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_stop(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->stop(l->container)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_freeze(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->freeze(l->container)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_unfreeze(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->unfreeze(l->container)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_info(struct ubus_context *ctx, struct ubus_object *obj,
	     struct ubus_request_data *req, const char *method,
	     struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_RENAME_MAX];
	struct rpc_lxc *l = NULL;
	char **addresses;
	void *k;
	pid_t initpid;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l)
		return UBUS_STATUS_INVALID_ARGUMENT;

	if (!l->container->is_running(l->container) &&
	    !l->container->is_defined(l->container))
		return UBUS_STATUS_NOT_FOUND;

	blob_buf_init(&buf, 0);

	blobmsg_add_string(&buf, "name", l->container->name);

	blobmsg_add_string(&buf, "state", l->container->state(l->container));

	initpid = l->container->init_pid(l->container);
	if (initpid >= 0)
		blobmsg_add_u32(&buf, "pid", initpid);

	k = blobmsg_open_array(&buf, "ips");
	addresses = l->container->get_ips(l->container, NULL, NULL, 0);
	if (addresses) {
		int i;

		for (i = 0; addresses[i]; i++)
			blobmsg_add_string(&buf, "ip", addresses[i]);
	}
	blobmsg_close_array(&buf, k);

	ubus_send_reply(ctx, req, buf.head);
	rpc_lxc_done(l);

	return UBUS_STATUS_OK;
}

static int
rpc_lxc_rename(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_RENAME_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_rename_policy, __RPC_LXC_RENAME_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (!tb[RPC_LXC_RENAME_NEWNAME]) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	if (l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	char *newname = blobmsg_data(tb[RPC_LXC_RENAME_NEWNAME]);
	if (!newname || !l->container->rename(l->container, newname)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_create(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_CREATE_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	char *template = "none";
	int flags = 0;
	char **args = NULL;

	blobmsg_parse(rpc_lxc_create_policy, __RPC_LXC_CREATE_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (tb[RPC_LXC_CREATE_TEMPLATE])
		template = blobmsg_data(tb[RPC_LXC_CREATE_TEMPLATE]);

	if (tb[RPC_LXC_CREATE_FLAGS])
		flags = blobmsg_get_u32(tb[RPC_LXC_CREATE_FLAGS]);

	if (tb[RPC_LXC_CREATE_ARGS]) {
		struct blob_attr *cur;
		int num, rem;

		num = blobmsg_check_array(tb[RPC_LXC_CREATE_ARGS], BLOBMSG_TYPE_STRING);

		// trailing NULL is needed
		args = calloc(num + 1, sizeof(char *));
		if (!args) {
			rc = UBUS_STATUS_UNKNOWN_ERROR;
			goto out;
		}

		num = 0;
		blobmsg_for_each_attr(cur, tb[RPC_LXC_CREATE_ARGS], rem)
		{
			if (blobmsg_type(cur) != BLOBMSG_TYPE_STRING)
				continue;

			args[num++] = (char *) blobmsg_data(cur);
		}
	}

	if (!l->container->create(l->container, template, NULL, NULL, flags, args)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	free(args);
	return rc;
}

static int
rpc_lxc_destroy(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[__RPC_LXC_MAX];
	struct rpc_lxc *l = NULL;
	int rc;

	blobmsg_parse(rpc_lxc_min_policy, __RPC_LXC_MAX, tb, blob_data(msg), blob_len(msg));

	l = rpc_lxc_init(tb);
	if (!l) return UBUS_STATUS_INVALID_ARGUMENT;

	if (l->container->is_running(l->container)) {
		rc = UBUS_STATUS_UNKNOWN_ERROR;
		goto out;
	}

	if (!l->container->destroy(l->container)) {
		rc = UBUS_STATUS_INVALID_ARGUMENT;
		goto out;
	}

	rc = UBUS_STATUS_OK;
out:
	rpc_lxc_done(l);
	return rc;
}

static int
rpc_lxc_list(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{

	blob_buf_init(&buf, 0);

	int rc;
	char **names;
	struct lxc_container **cret;

	rc = list_all_containers(NULL, &names, &cret);
	if (rc == -1)
		return UBUS_STATUS_UNKNOWN_ERROR;

	for (int i = 0; i < rc; i++) {
		struct lxc_container *c = cret[i];
		blobmsg_add_string(&buf, names[i], c->state(c));

		free(names[i]);
		lxc_container_put(c);
	}

	ubus_send_reply(ctx, req, buf.head);

	return UBUS_STATUS_OK;
}

static int
rpc_lxc_api_init(const struct rpc_daemon_ops *o, struct ubus_context *ctx)
{
	static const struct ubus_method lxc_methods[] = {
		UBUS_METHOD("start", rpc_lxc_start, rpc_lxc_min_policy),
		UBUS_METHOD("reboot", rpc_lxc_reboot, rpc_lxc_min_policy),
		UBUS_METHOD("shutdown", rpc_lxc_shutdown, rpc_lxc_shutdown_policy),
		UBUS_METHOD("stop", rpc_lxc_stop, rpc_lxc_min_policy),
		UBUS_METHOD("freeze", rpc_lxc_freeze, rpc_lxc_min_policy),
		UBUS_METHOD("unfreeze", rpc_lxc_unfreeze, rpc_lxc_min_policy),
		UBUS_METHOD("info", rpc_lxc_info, rpc_lxc_min_policy),
		UBUS_METHOD("rename", rpc_lxc_rename, rpc_lxc_rename_policy),
		UBUS_METHOD("create", rpc_lxc_create, rpc_lxc_create_policy),
		UBUS_METHOD("destroy", rpc_lxc_destroy, rpc_lxc_min_policy),
		UBUS_METHOD_NOARG("list", rpc_lxc_list),
	};

	static struct ubus_object_type lxc_type =
		UBUS_OBJECT_TYPE("luci-rpc-lxc", lxc_methods);

	static struct ubus_object obj = {
		.name = "lxc",
		.type = &lxc_type,
		.methods = lxc_methods,
		.n_methods = ARRAY_SIZE(lxc_methods),
	};

	return ubus_add_object(ctx, &obj);
}

struct rpc_plugin rpc_plugin = {
	.init = rpc_lxc_api_init
};
