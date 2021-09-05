/*
 * auc - attendedsysUpgrade CLI
 * Copyright (C) 2017-2021 Daniel Golle <daniel@makrotopia.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _GNU_SOURCE
#ifndef AUC_VERSION
#define AUC_VERSION "unknown"
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <glob.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#include <uci.h>
#include <uci_blob.h>
#include <json-c/json.h>
#include <libubox/ulog.h>
#include <libubox/list.h>
#include <libubox/vlist.h>
#include <libubox/blobmsg_json.h>
#include <libubox/avl-cmp.h>
#include <libubox/uclient.h>
#include <libubox/uclient-utils.h>
#include <libubus.h>

#define REQ_TIMEOUT 15

#define API_BRANCHES "branches"
#define API_INDEX "index"
#define API_JSON "json"
#define API_JSON_EXT "." API_JSON
#define API_PACKAGES "packages"
#define API_REQUEST "api/build"
#define API_STATUS_QUEUED "queued"
#define API_STATUS_STARTED "started"
#define API_STORE "store"
#define API_TARGETS "targets"

#define PUBKEY_PATH "/etc/opkg/keys"
#define SHA256SUM "/bin/busybox sha256sum"

#ifdef AUC_DEBUG
#define DPRINTF(...) if (debug) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...)
#endif

static const char server_issues[]="https://github.com/aparcar/asu/issues";

static struct ubus_context *ctx;
static struct uclient *ucl = NULL;
static char user_agent[80];
static char *serverurl;
static int upgrade_packages;
static struct ustream_ssl_ctx *ssl_ctx;
static const struct ustream_ssl_ops *ssl_ops;
static off_t out_bytes;
static off_t out_len;
static off_t out_offset;
static bool cur_resume;
static int output_fd = -1;
static bool retry = false;
static char *board_name = NULL;
static char *target = NULL;
static char *distribution = NULL, *version = NULL, *revision = NULL;
static int uptodate;
static char *filename = NULL;
static int rc;

struct branch {
	struct list_head list;
	char *name;
	char *git_branch;
	char *version;
	char *version_code;
	char *version_number;
	bool snapshot;
	time_t release_time;
	time_t eol_time;
	char *path;
	char *path_packages;
	char *arch_packages;
	char *pubkey;
	char *updates;
	char **repos;
	char **extra_repos;
	char **extra_repos_names;
	char **default_packages;
	char **device_packages;
};
static LIST_HEAD(branches);

struct avl_pkg {
	struct avl_node avl;
	char *name;
	char *version;
};
static struct avl_tree pkg_tree = AVL_TREE_INIT(pkg_tree, avl_strcmp, false, NULL);

#ifdef AUC_DEBUG
static int debug = 0;
#endif

/*
 * policy for ubus call system board
 * see procd/system.c
 */
enum {
	BOARD_BOARD_NAME,
	BOARD_RELEASE,
	__BOARD_MAX,
};

static const struct blobmsg_policy board_policy[__BOARD_MAX] = {
	[BOARD_BOARD_NAME] = { .name = "board_name", .type = BLOBMSG_TYPE_STRING },
	[BOARD_RELEASE] = { .name = "release", .type = BLOBMSG_TYPE_TABLE },
};

/*
 * policy for release information in system board reply
 * see procd/system.c
 */
enum {
	RELEASE_DISTRIBUTION,
	RELEASE_REVISION,
	RELEASE_TARGET,
	RELEASE_VERSION,
	__RELEASE_MAX,
};

static const struct blobmsg_policy release_policy[__RELEASE_MAX] = {
	[RELEASE_DISTRIBUTION] = { .name = "distribution", .type = BLOBMSG_TYPE_STRING },
	[RELEASE_REVISION] = { .name = "revision", .type = BLOBMSG_TYPE_STRING },
	[RELEASE_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
	[RELEASE_VERSION] = { .name = "version", .type = BLOBMSG_TYPE_STRING },
};

/*
 * policy for package list returned from rpc-sys or from server
 * see rpcd/sys.c and ASU sources
 */
enum {
	PACKAGES_ARCHITECTURE,
	PACKAGES_PACKAGES,
	__PACKAGES_MAX,
};

static const struct blobmsg_policy packages_policy[__PACKAGES_MAX] = {
	[PACKAGES_ARCHITECTURE] = { .name = "architecture", .type = BLOBMSG_TYPE_STRING },
	[PACKAGES_PACKAGES] = { .name = "packages", .type = BLOBMSG_TYPE_TABLE },
};

/*
 * policy for upgrade_test
 * see rpcd/sys.c
 */
enum {
	UPGTEST_CODE,
	UPGTEST_STDERR,
	__UPGTEST_MAX,
};

static const struct blobmsg_policy upgtest_policy[__UPGTEST_MAX] = {
	[UPGTEST_CODE] = { .name = "code", .type = BLOBMSG_TYPE_INT32 },
	[UPGTEST_STDERR] = { .name = "stderr", .type = BLOBMSG_TYPE_STRING },
};

/*
 * policy for branches.json
 */
enum {
	BRANCH_DATE_EOL,
	BRANCH_DATE_RELEASE,
	BRANCH_ENABLED,
	BRANCH_EXTRA_REPOS,
	BRANCH_GIT_BRANCH,
	BRANCH_NAME,
	BRANCH_PATH,
	BRANCH_PATH_PACKAGES,
	BRANCH_PUBKEY,
	BRANCH_REPOS,
	BRANCH_SNAPSHOT,
	BRANCH_TARGETS,
	BRANCH_UPDATES,
	BRANCH_VERSIONS,
	__BRANCH_MAX,
};

static const struct blobmsg_policy branches_policy[__BRANCH_MAX] = {
	[BRANCH_DATE_EOL] = { .name = "eol", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_DATE_RELEASE] = { .name = "release_date", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_ENABLED] = { .name = "enabled", .type = BLOBMSG_TYPE_BOOL },
	[BRANCH_EXTRA_REPOS] = { .name = "extra_repos", .type = BLOBMSG_TYPE_TABLE },
	[BRANCH_GIT_BRANCH] = { .name = "git_branch", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_PATH] = { .name = "path", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_PATH_PACKAGES] = { .name = "path_packages", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_PUBKEY] = { .name = "pubkey", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_REPOS] = { .name = "repos", .type = BLOBMSG_TYPE_ARRAY },
	[BRANCH_SNAPSHOT] = { .name = "snapshot", .type = BLOBMSG_TYPE_BOOL },
	[BRANCH_TARGETS] = { .name = "targets", .type = BLOBMSG_TYPE_TABLE },
	[BRANCH_UPDATES] = { .name = "updates", .type = BLOBMSG_TYPE_STRING },
	[BRANCH_VERSIONS] = { .name = "versions", .type = BLOBMSG_TYPE_ARRAY },
};

/*
 * shared policy for target.json and server image request reply
 */
enum {
	TARGET_ARCH_PACKAGES,
	TARGET_BINDIR,
	TARGET_BUILD_AT,
	TARGET_DEFAULT_PACKAGES,
	TARGET_DEVICE_PACKAGES,
	TARGET_ENQUEUED_AT,
	TARGET_IMAGE_PREFIX,
	TARGET_IMAGES,
	TARGET_MESSAGE,
	TARGET_MANIFEST,
	TARGET_METADATA_VERSION,
	TARGET_REQUEST_HASH,
	TARGET_SOURCE_DATE_EPOCH,
	TARGET_QUEUE_POSITION,
	TARGET_STATUS,
	TARGET_STDERR,
	TARGET_STDOUT,
	TARGET_SUPPORTED_DEVICES,
	TARGET_TARGET,
	TARGET_TITLES,
	TARGET_VERSION_CODE,
	TARGET_VERSION_NUMBER,
	__TARGET_MAX,
};

static const struct blobmsg_policy target_policy[__TARGET_MAX] = {
	[TARGET_ARCH_PACKAGES] = { .name = "arch_packages", .type = BLOBMSG_TYPE_STRING },
	[TARGET_BINDIR] = { .name = "bin_dir", .type = BLOBMSG_TYPE_STRING },
	[TARGET_BUILD_AT] = { .name = "built_at", .type = BLOBMSG_TYPE_STRING },
	[TARGET_DEFAULT_PACKAGES] = { .name = "default_packages", .type = BLOBMSG_TYPE_ARRAY },
	[TARGET_DEVICE_PACKAGES] = { .name = "device_packages", .type = BLOBMSG_TYPE_ARRAY },
	[TARGET_ENQUEUED_AT] = { .name = "enqueued_at", .type = BLOBMSG_TYPE_STRING },
	[TARGET_IMAGE_PREFIX] = { .name = "image_prefix", .type = BLOBMSG_TYPE_STRING },
	[TARGET_IMAGES] = { .name = "images", .type = BLOBMSG_TYPE_ARRAY },
	[TARGET_MANIFEST] = { .name = "manifest", .type = BLOBMSG_TYPE_TABLE },
	[TARGET_MESSAGE] = { .name = "message", .type = BLOBMSG_TYPE_STRING },
	[TARGET_METADATA_VERSION] = { .name = "metadata_version", .type = BLOBMSG_TYPE_INT32 },
	[TARGET_REQUEST_HASH] = { .name = "request_hash", .type = BLOBMSG_TYPE_STRING },
	[TARGET_SOURCE_DATE_EPOCH] = { .name = "source_date_epoch", .type = BLOBMSG_TYPE_STRING },
	[TARGET_QUEUE_POSITION] = { .name = "queue_position", .type = BLOBMSG_TYPE_INT32 },
	[TARGET_STATUS] = { .name = "status", .type = BLOBMSG_TYPE_STRING },
	[TARGET_STDERR] = { .name = "stderr", .type = BLOBMSG_TYPE_STRING },
	[TARGET_STDOUT] = { .name = "stdout", .type = BLOBMSG_TYPE_STRING },
	[TARGET_SUPPORTED_DEVICES] = { .name = "supported_devices", .type = BLOBMSG_TYPE_ARRAY },
	[TARGET_TARGET] = { .name = "target", .type = BLOBMSG_TYPE_STRING },
	[TARGET_TITLES] = { .name = "titles", .type = BLOBMSG_TYPE_ARRAY },
	[TARGET_VERSION_CODE] = { .name = "version_code", .type = BLOBMSG_TYPE_STRING },
	[TARGET_VERSION_NUMBER] = { .name = "version_number", .type = BLOBMSG_TYPE_STRING },
};

/*
 * policy for images object in target
 */
enum {
	IMAGES_FILESYSTEM,
	IMAGES_NAME,
	IMAGES_SHA256,
	IMAGES_TYPE,
	__IMAGES_MAX,
};

static const struct blobmsg_policy images_policy[__IMAGES_MAX] = {
	[IMAGES_FILESYSTEM] = { .name = "filesystem", .type = BLOBMSG_TYPE_STRING },
	[IMAGES_NAME] = { .name = "name", .type = BLOBMSG_TYPE_STRING },
	[IMAGES_SHA256] = { .name = "sha256", .type = BLOBMSG_TYPE_STRING },
	[IMAGES_TYPE] = { .name = "type", .type = BLOBMSG_TYPE_STRING },
};

/*
 * generic policy for HTTP JSON reply
 */
enum {
	REPLY_ARRAY,
	REPLY_OBJECT,
	__REPLY_MAX,
};

static const struct blobmsg_policy reply_policy[__REPLY_MAX] = {
	[REPLY_ARRAY] = { .name = "reply", .type = BLOBMSG_TYPE_ARRAY },
	[REPLY_OBJECT] = { .name = "reply", .type = BLOBMSG_TYPE_TABLE },
};

/*
 * policy for HTTP headers received from server
 */
enum {
	H_LEN,
	H_RANGE,
	H_UNKNOWN_PACKAGE,
	__H_MAX
};

static const struct blobmsg_policy header_policy[__H_MAX] = {
	[H_LEN] = { .name = "content-length", .type = BLOBMSG_TYPE_STRING },
	[H_RANGE] = { .name = "content-range", .type = BLOBMSG_TYPE_STRING },
	[H_UNKNOWN_PACKAGE] = { .name = "x-unknown-package", .type = BLOBMSG_TYPE_STRING },
};

/*
 * load serverurl from UCI
 */
static int load_config() {
	struct uci_context *uci_ctx;
	struct uci_package *uci_attendedsysupgrade;
	struct uci_section *uci_s;

	uci_ctx = uci_alloc_context();
	if (!uci_ctx)
		return -1;

	uci_ctx->flags &= ~UCI_FLAG_STRICT;

	if (uci_load(uci_ctx, "attendedsysupgrade", &uci_attendedsysupgrade) ||
	    !uci_attendedsysupgrade) {
		fprintf(stderr, "Failed to load attendedsysupgrade config\n");
		return -1;
	}

	uci_s = uci_lookup_section(uci_ctx, uci_attendedsysupgrade, "server");
	if (!uci_s) {
		fprintf(stderr, "Failed to read server url from config\n");
		return -1;
	}
	serverurl = strdup(uci_lookup_option_string(uci_ctx, uci_s, "url"));

	uci_s = uci_lookup_section(uci_ctx, uci_attendedsysupgrade, "client");
	if (!uci_s) {
		fprintf(stderr, "Failed to read client config\n");
		return -1;
	}
	upgrade_packages = atoi(uci_lookup_option_string(uci_ctx, uci_s, "upgrade_packages"));

	uci_free_context(uci_ctx);

	return 0;
}

/*
 * libdpkg - Debian packaging suite library routines
 * vercmp.c - comparison of version numbers
 *
 * Copyright (C) 1995 Ian Jackson <iwj10@cus.cam.ac.uk>
 */

/* assume ascii; warning: evaluates x multiple times! */
#define order(x) ((x) == '~' ? -1 \
		: isdigit((x)) ? 0 \
		: !(x) ? 0 \
		: isalpha((x)) ? (x) \
		: (x) + 256)

static int verrevcmp(const char *val, const char *ref)
{
	if (!val)
		val = "";
	if (!ref)
		ref = "";

	while (*val || *ref) {
		int first_diff = 0;

		while ((*val && !isdigit(*val)) || (*ref && !isdigit(*ref))) {
			int vc = order(*val), rc = order(*ref);
			if (vc != rc)
				return vc - rc;
			val++;
			ref++;
		}

		while (*val == '0')
			val++;
		while (*ref == '0')
			ref++;
		while (isdigit(*val) && isdigit(*ref)) {
			if (!first_diff)
				first_diff = *val - *ref;
			val++;
			ref++;
		}
		if (isdigit(*val))
			return 1;
		if (isdigit(*ref))
			return -1;
		if (first_diff)
			return first_diff;
	}
	return 0;
}


/**
 * UBUS response callbacks
 */
/*
 * rpc-sys packagelist
 * append array of package names to blobbuf given in req->priv
 */
#define ANSI_ESC "\x1b"
#define ANSI_COLOR_RESET ANSI_ESC "[0m"
#define ANSI_COLOR_RED ANSI_ESC "[1;31m"
#define ANSI_COLOR_GREEN ANSI_ESC "[1;32m"
#define ANSI_CURSOR_SAFE "[s"
#define ANSI_CURSOR_RESTORE "[u"
#define ANSI_ERASE_LINE "[K"

#define PKG_UPGRADE 0x1
#define PKG_DOWNGRADE 0x2
#define PKG_NOT_FOUND 0x4
#define PKG_ERROR 0x8

static inline bool is_builtin_pkg(const char *pkgname)
{
	return !strcmp(pkgname, "libc") ||
		!strcmp(pkgname, "librt") ||
		!strcmp(pkgname, "libpthread") ||
		!strcmp(pkgname, "kernel");
}

static void pkglist_check_cb(struct ubus_request *req, int type, struct blob_attr *msg)
{
	int *status = (int *)req->priv;
	struct blob_attr *tb[__PACKAGES_MAX], *cur;
	struct avl_pkg *pkg;
	int rem;
	int cmpres;

	blobmsg_parse(packages_policy, __PACKAGES_MAX, tb, blobmsg_data(msg), blobmsg_len(msg));

	if (!tb[PACKAGES_PACKAGES])
		return;

	blobmsg_for_each_attr(cur, tb[PACKAGES_PACKAGES], rem) {
		if (is_builtin_pkg(blobmsg_name(cur)))
			continue;

		pkg = avl_find_element(&pkg_tree, blobmsg_name(cur), pkg, avl);
		if (!pkg) {
			fprintf(stderr, "installed package %s cannot be found in remote list!\n", blobmsg_name(cur));
			*status |= PKG_NOT_FOUND;
			continue;
		}

		cmpres = verrevcmp(blobmsg_get_string(cur), pkg->version);
		if (cmpres < 0)
			*status |= PKG_UPGRADE;

		if (cmpres > 0)
			*status |= PKG_DOWNGRADE;

		if (cmpres
#ifdef AUC_DEBUG
		|| debug
#endif
			)
			fprintf(stderr, " %s: %s%s -> %s%s\n", blobmsg_name(cur),
				(!cmpres)?"":(cmpres > 0)?ANSI_COLOR_RED:ANSI_COLOR_GREEN,
				blobmsg_get_string(cur), pkg->version,
				(cmpres)?ANSI_COLOR_RESET:"");
	}
}

/*
 * rpc-sys packagelist
 * append array of package names to blobbuf given in req->priv
 */
static void pkglist_req_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	struct blob_buf *buf = (struct blob_buf *)req->priv;
	struct blob_attr *tb[__PACKAGES_MAX];
	struct blob_attr *cur;
	int rem;
	struct avl_pkg *pkg;
	void *table;

	blobmsg_parse(packages_policy, __PACKAGES_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[PACKAGES_PACKAGES]) {
		fprintf(stderr, "No packagelist received\n");
		return;
	}

	table = blobmsg_open_table(buf, "packages_versions");

	blobmsg_for_each_attr(cur, tb[PACKAGES_PACKAGES], rem) {
		if (is_builtin_pkg(blobmsg_name(cur)))
			continue;

		pkg = avl_find_element(&pkg_tree, blobmsg_name(cur), pkg, avl);
		if (!pkg)
			continue;

		blobmsg_add_string(buf, blobmsg_name(cur), pkg->version);
	}
	blobmsg_close_table(buf, table);
};

/*
 * system board
 * append append board information to blobbuf given in req->priv
 * populate board and release global strings
 */
static void board_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	struct blob_buf *buf = (struct blob_buf *)req->priv;
	struct blob_attr *tb[__BOARD_MAX];
	struct blob_attr *rel[__RELEASE_MAX];

	blobmsg_parse(board_policy, __BOARD_MAX, tb, blob_data(msg), blob_len(msg));


	if (!tb[BOARD_RELEASE]) {
		fprintf(stderr, "No release received\n");
		rc=-ENODATA;
		return;
	}

	blobmsg_parse(release_policy, __RELEASE_MAX, rel,
			blobmsg_data(tb[BOARD_RELEASE]), blobmsg_data_len(tb[BOARD_RELEASE]));

	if (!rel[RELEASE_TARGET] ||
	    !rel[RELEASE_DISTRIBUTION] ||
	    !rel[RELEASE_VERSION] ||
	    !rel[RELEASE_REVISION]) {
		fprintf(stderr, "No release information received\n");
		rc=-ENODATA;
		return;
	}

	target = strdup(blobmsg_get_string(rel[RELEASE_TARGET]));
	distribution = strdup(blobmsg_get_string(rel[RELEASE_DISTRIBUTION]));
	version = strdup(blobmsg_get_string(rel[RELEASE_VERSION]));
	revision = strdup(blobmsg_get_string(rel[RELEASE_REVISION]));

	if (!strcmp(target, "x86/64") || !strcmp(target, "x86/generic")) {
		/*
		 * ugly work-around ahead:
		 * ignore board name on generic x86 targets, as image name is always 'generic'
		 */
		board_name = strdup("generic");
	} else {
		if (!tb[BOARD_BOARD_NAME]) {
			fprintf(stderr, "No board name received\n");
			rc=-ENODATA;
			return;
		}
		board_name = strdup(blobmsg_get_string(tb[BOARD_BOARD_NAME]));
	}

	blobmsg_add_string(buf, "distro", distribution);
	blobmsg_add_string(buf, "target", target);
	blobmsg_add_string(buf, "version", version);
	blobmsg_add_string(buf, "revision", revision);
}

/*
 * rpc-sys upgrade_test
 * check if downloaded file is accepted by sysupgrade
 */
static void upgtest_cb(struct ubus_request *req, int type, struct blob_attr *msg) {
	int *valid = (int *)req->priv;
	struct blob_attr *tb[__UPGTEST_MAX];

	blobmsg_parse(upgtest_policy, __UPGTEST_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[UPGTEST_CODE]) {
		fprintf(stderr, "No sysupgrade test return code received\n");
		return;
	}

	*valid = (blobmsg_get_u32(tb[UPGTEST_CODE]) == 0)?1:0;

	if (tb[UPGTEST_STDERR])
		fprintf(stderr, "%s", blobmsg_get_string(tb[UPGTEST_STDERR]));
	else if (*valid == 0)
		fprintf(stderr, "image verification failed\n");
	else
		fprintf(stderr, "image verification succeeded\n");
};

/**
 * uclient stuff
 */
static int open_output_file(const char *path, uint64_t resume_offset)
{
	char *filename = NULL;
	int flags;
	int ret;

	if (cur_resume)
		flags = O_RDWR;
	else
		flags = O_WRONLY | O_EXCL;

	flags |= O_CREAT;

	filename = uclient_get_url_filename(path, "firmware.bin");

	fprintf(stderr, "Writing to '%s'\n", filename);
	ret = open(filename, flags, 0644);
	if (ret < 0)
		goto free;

	if (resume_offset &&
	    lseek(ret, resume_offset, SEEK_SET) < 0) {
		fprintf(stderr, "Failed to seek %"PRIu64" bytes in output file\n", resume_offset);
		close(ret);
		ret = -1;
		goto free;
	}

	out_offset = resume_offset;
	out_bytes += resume_offset;

free:
	free(filename);
	return ret;
}

struct jsonblobber {
	json_tokener *tok;
	struct blob_buf *outbuf;
};

static void request_done(struct uclient *cl)
{
	struct jsonblobber *jsb = (struct jsonblobber *)cl->priv;
	if (jsb) {
		json_tokener_free(jsb->tok);
		free(jsb);
	};

	uclient_disconnect(cl);
	uloop_end();
}

static void header_done_cb(struct uclient *cl)
{
	struct blob_attr *tb[__H_MAX];
	uint64_t resume_offset = 0, resume_end, resume_size;

	if (uclient_http_redirect(cl)) {
		fprintf(stderr, "Redirected to %s on %s\n", cl->url->location, cl->url->host);

		return;
	}

	if (cl->status_code == 204 && cur_resume) {
		/* Resume attempt failed, try normal download */
		cur_resume = false;
		//init_request(cl);
		return;
	}

	DPRINTF("status code: %d\n", cl->status_code);
	DPRINTF("headers:\n%s\n", blobmsg_format_json_indent(cl->meta, true, 0));
	blobmsg_parse(header_policy, __H_MAX, tb, blob_data(cl->meta), blob_len(cl->meta));

	switch (cl->status_code) {
	case 400:
		request_done(cl);
		rc=-ESRCH;
		break;
	case 409:
		fprintf(stderr, "Conflicting packages requested\n");
		request_done(cl);
		rc=-EINVAL;
		break;
	case 412:
		fprintf(stderr, "%s target %s (%s) not found. Please report this at %s\n",
			distribution, target, board_name, server_issues);
		request_done(cl);
		rc=-ENOSYS;
		break;
	case 413:
		fprintf(stderr, "image too big.\n");
		rc=-E2BIG;
		request_done(cl);
		break;
	case 416:
		fprintf(stderr, "File download already fully retrieved; nothing to do.\n");
		request_done(cl);
		break;
	case 422:
		fprintf(stderr, "unknown package '%s' requested.\n",
			blobmsg_get_string(tb[H_UNKNOWN_PACKAGE]));
		rc=-ENOPKG;
		request_done(cl);
		break;
	case 501:
		fprintf(stderr, "ImageBuilder didn't produce sysupgrade file.\n");
		rc=-ENODATA;
		request_done(cl);
		break;
	case 204:
		fprintf(stdout, "system is up to date.\n");
		uptodate=1;
		rc=0;
		request_done(cl);
		break;
	case 206:
		if (!cur_resume) {
			fprintf(stderr, "Error: Partial content received, full content requested\n");
			request_done(cl);
			break;
		}

		if (!tb[H_RANGE]) {
			fprintf(stderr, "Content-Range header is missing\n");
			break;
		}

		if (sscanf(blobmsg_get_string(tb[H_RANGE]),
			   "bytes %"PRIu64"-%"PRIu64"/%"PRIu64,
			   &resume_offset, &resume_end, &resume_size) != 3) {
			fprintf(stderr, "Content-Range header is invalid\n");
			break;
		}
	case 202:
		retry = true;
		break;
	case 200:
		if (cl->priv)
			break;

		if (tb[H_LEN])
			out_len = strtoul(blobmsg_get_string(tb[H_LEN]), NULL, 10);

		output_fd = open_output_file(cl->url->location, resume_offset);
		if (output_fd < 0) {
			perror("Cannot open output file");
			request_done(cl);
		}
		break;
	case 500:
		/* server may reply JSON object */
		break;

	default:
		DPRINTF("HTTP error %d\n", cl->status_code);
		request_done(cl);
		break;
	}
}

static void read_data_cb(struct uclient *cl)
{
	char buf[256];
	int len;
	json_object *jsobj;
	struct blob_buf *outbuf = NULL;
	json_tokener *tok = NULL;
	struct jsonblobber *jsb = (struct jsonblobber *)cl->priv;

	if (!jsb) {
		while (1) {
			len = uclient_read(cl, buf, sizeof(buf));
			if (!len)
				return;

			out_bytes += len;
			write(output_fd, buf, len);
		}
		return;
	}

	outbuf = jsb->outbuf;
	tok = jsb->tok;

	while (1) {
		len = uclient_read(cl, buf, sizeof(buf));
		if (!len)
			break;

		out_bytes += len;

		jsobj = json_tokener_parse_ex(tok, buf, len);

		if (json_tokener_get_error(tok) == json_tokener_continue)
			continue;

		if (json_tokener_get_error(tok) != json_tokener_success)
			break;

		if (jsobj)
		{
			blobmsg_add_json_element(outbuf, "reply", jsobj);

			json_object_put(jsobj);
			break;
		}
	}
}

static void eof_cb(struct uclient *cl)
{
	if (!cl->data_eof && !uptodate) {
		fprintf(stderr, "Connection reset prematurely\n");
	}
	request_done(cl);
}

static void handle_uclient_error(struct uclient *cl, int code)
{
	const char *type = "Unknown error";

	switch(code) {
	case UCLIENT_ERROR_CONNECT:
		type = "Connection failed";
		break;
	case UCLIENT_ERROR_TIMEDOUT:
		type = "Connection timed out";
		break;
	case UCLIENT_ERROR_SSL_INVALID_CERT:
		type = "Invalid SSL certificate";
		break;
	case UCLIENT_ERROR_SSL_CN_MISMATCH:
		type = "Server hostname does not match SSL certificate";
		break;
	default:
		break;
	}

	fprintf(stderr, "Connection error: %s\n", type);

	request_done(cl);
}

static const struct uclient_cb check_cb = {
	.header_done = header_done_cb,
	.data_read = read_data_cb,
	.data_eof = eof_cb,
	.error = handle_uclient_error,
};

static int server_request(const char *url, struct blob_buf *inbuf, struct blob_buf *outbuf) {
	struct jsonblobber *jsb = NULL;
	int rc = -ENOENT;
	char *post_data;
	out_offset = 0;
	out_bytes = 0;
	out_len = 0;

#ifdef AUC_DEBUG
	if (debug)
		fprintf(stderr, "Requesting URL: %s\n", url);
#endif

	if (outbuf) {
		jsb = malloc(sizeof(struct jsonblobber));
		jsb->outbuf = outbuf;
		jsb->tok = json_tokener_new();
	};

	if (!ucl) {
		ucl = uclient_new(url, NULL, &check_cb);
		uclient_http_set_ssl_ctx(ucl, ssl_ops, ssl_ctx, 1);
		ucl->timeout_msecs = REQ_TIMEOUT * 1000;
	} else {
		uclient_set_url(ucl, url, NULL);
	}

	ucl->priv = jsb;

	rc = uclient_connect(ucl);
	if (rc)
		return rc;

	rc = uclient_http_set_request_type(ucl, inbuf?"POST":"GET");
	if (rc)
		return rc;

	uclient_http_reset_headers(ucl);
	uclient_http_set_header(ucl, "User-Agent", user_agent);
	if (inbuf) {
		uclient_http_set_header(ucl, "Content-Type", "application/json");
		post_data = blobmsg_format_json(inbuf->head, true);
		uclient_write(ucl, post_data, strlen(post_data));
	}
	rc = uclient_request(ucl);
	if (rc)
		return rc;

	uloop_run();

	return 0;
}

/**
 * ustream-ssl
 */
static int init_ustream_ssl(void) {
	void *dlh;
	glob_t gl;
	int i;

	dlh = dlopen("libustream-ssl.so", RTLD_LAZY | RTLD_LOCAL);
	if (!dlh)
		return -ENOENT;

	ssl_ops = dlsym(dlh, "ustream_ssl_ops");
	if (!ssl_ops)
		return -ENOENT;

	ssl_ctx = ssl_ops->context_new(false);

	glob("/etc/ssl/certs/*.crt", 0, NULL, &gl);
	if (!gl.gl_pathc)
		return -ENOKEY;

	for (i = 0; i < gl.gl_pathc; i++)
		ssl_ops->context_add_ca_crt_file(ssl_ctx, gl.gl_pathv[i]);

	return 0;
}

static int ask_user(void)
{
	fprintf(stderr, "Are you sure you want to continue the upgrade process? [N/y] ");
	if (getchar() != 'y')
		return -EINTR;
	return 0;
}

static char* alloc_replace_var(char *in, const char *var, const char *replace)
{
	char *tmp = in;
	char *res = NULL;
	char *eptr;

	while ((tmp = strchr(tmp, '{'))) {
		++tmp;
		eptr = strchr(tmp, '}');
		if (!eptr)
			return NULL;

		if (!strncmp(tmp, var, (unsigned int)(eptr - tmp))) {
			asprintf(&res, "%.*s%s%s",
				(unsigned int)(tmp - in) - 1, in, replace, eptr + 1);
			break;
		}
	}

	if (!res)
		res = strdup(in);

	return res;
}

static time_t parse_reldate(char *str)
{
	int m, d, y;

	if (!str ||
	    strlen(str) != 10 ||
	    sscanf(str, "%4d-%2d-%2d", &y, &m, &d) != 3)
		return (time_t)0;

	struct tm time = {
		.tm_mday = d,
		.tm_mon = m - 1,
		.tm_year = y - 1900,
	};
	return timegm(&time);
}

static int json_to_string_arrays(struct blob_attr *j, char ***vars, char ***names)
{
	int i = 0;
	struct blob_attr *cur;
	int rem;

	if (j) {
		i = blobmsg_check_array(j, BLOBMSG_TYPE_STRING);
		if (i < 0)
			return i;
	}

	if (i > 0) {
		*vars = calloc((i + 1), sizeof(char *));
		if (names)
			*names = calloc((i + 1), sizeof(char *));

		i = 0;
		blobmsg_for_each_attr(cur, j, rem) {
			if (names)
				(*names)[i] = strdup(blobmsg_name(cur));

			(*vars)[i++] = strdup(blobmsg_get_string(cur));
		}
	}

	return i;
}

static int request_target(struct branch *br, char *url)
{
	static struct blob_buf boardbuf;
	struct blob_attr *tbr[__REPLY_MAX], *tb[__TARGET_MAX];

	blobmsg_buf_init(&boardbuf);

	if ((rc = server_request(url, NULL, &boardbuf))) {
		blob_buf_free(&boardbuf);
		return rc;
	}

	blobmsg_parse(reply_policy, __REPLY_MAX, tbr, blob_data(boardbuf.head), blob_len(boardbuf.head));

	if (!tbr[REPLY_OBJECT])
		return -ENODATA;

	blobmsg_parse(target_policy, __TARGET_MAX, tb, blobmsg_data(tbr[REPLY_OBJECT]), blobmsg_len(tbr[REPLY_OBJECT]));

	if (!tb[TARGET_METADATA_VERSION] ||
	    !tb[TARGET_ARCH_PACKAGES] ||
	    !tb[TARGET_DEFAULT_PACKAGES] ||
	    !tb[TARGET_DEVICE_PACKAGES] ||
	    !tb[TARGET_IMAGE_PREFIX] ||
	    !tb[TARGET_IMAGES] ||
	    !tb[TARGET_SOURCE_DATE_EPOCH] ||
	    !tb[TARGET_SUPPORTED_DEVICES] ||
	    !tb[TARGET_TARGET]) {
		blob_buf_free(&boardbuf);
		return -ENODATA;
	}

	if (blobmsg_get_u32(tb[TARGET_METADATA_VERSION]) != 1) {
		blob_buf_free(&boardbuf);
		return -EPFNOSUPPORT;
	}

	if (strcmp(blobmsg_get_string(tb[TARGET_TARGET]), target))
		return -EINVAL;

	if (strcmp(blobmsg_get_string(tb[TARGET_ARCH_PACKAGES]), br->arch_packages))
		return -EINVAL;

	json_to_string_arrays(tb[TARGET_DEFAULT_PACKAGES], &br->default_packages, NULL);
	json_to_string_arrays(tb[TARGET_DEVICE_PACKAGES], &br->device_packages, NULL);

	if (tb[TARGET_VERSION_CODE])
		br->version_code = strdup(blobmsg_get_string(tb[TARGET_VERSION_CODE]));

	if (tb[TARGET_VERSION_NUMBER])
		br->version_number = strdup(blobmsg_get_string(tb[TARGET_VERSION_NUMBER]));

	blob_buf_free(&boardbuf);
	return 0;
};

static char* validate_target(struct blob_attr *branch)
{
	struct blob_attr *cur;
	int rem;

	blobmsg_for_each_attr(cur, branch, rem)
		if (!strcmp(blobmsg_name(cur), target))
			return strdup(blobmsg_get_string(cur));

	return NULL;
}

static void process_branch(struct blob_attr *branch, bool only_active)
{
	struct blob_attr *tb[__BRANCH_MAX];
	struct blob_attr *curver;
	int remver;
	struct branch *br;
	char *tmp, *arch_packages, *board_json_file;
	const char *brname;

	blobmsg_parse(branches_policy, __BRANCH_MAX, tb, blobmsg_data(branch), blobmsg_len(branch));

	/* mandatory fields */
	if (!(tb[BRANCH_ENABLED] && blobmsg_get_bool(tb[BRANCH_ENABLED]) &&
		tb[BRANCH_NAME] && tb[BRANCH_PATH]) && tb[BRANCH_PATH_PACKAGES] &&
		tb[BRANCH_UPDATES] && tb[BRANCH_PUBKEY] && tb[BRANCH_REPOS] &&
		tb[BRANCH_VERSIONS] && tb[BRANCH_TARGETS])
		return;

	brname = blobmsg_get_string(tb[BRANCH_NAME]);
	if (only_active && strncmp(brname, version, strlen(brname)))
		return;

	/* check if target is offered in branch and get arch_packages */
	arch_packages = validate_target(tb[BRANCH_TARGETS]);
	if (!arch_packages)
		return;

	/* add each version of the branch */
	blobmsg_for_each_attr(curver, tb[BRANCH_VERSIONS], remver) {
		br = malloc(sizeof(struct branch));

		br->snapshot = tb[BRANCH_SNAPSHOT] && blobmsg_get_bool(tb[BRANCH_SNAPSHOT]);
		if (tb[BRANCH_GIT_BRANCH])
			br->git_branch = strdup(blobmsg_get_string(tb[BRANCH_GIT_BRANCH]));

		br->name = strdup(blobmsg_get_string(tb[BRANCH_NAME]));
		br->path = strdup(blobmsg_get_string(tb[BRANCH_PATH]));
		br->path_packages = strdup(blobmsg_get_string(tb[BRANCH_PATH_PACKAGES]));
		br->pubkey = strdup(blobmsg_get_string(tb[BRANCH_PUBKEY]));
		br->updates = strdup(blobmsg_get_string(tb[BRANCH_UPDATES]));
		br->release_time = parse_reldate(blobmsg_get_string(tb[BRANCH_DATE_RELEASE]));
		br->eol_time = parse_reldate(blobmsg_get_string(tb[BRANCH_DATE_RELEASE]));

		json_to_string_arrays(tb[BRANCH_REPOS], &br->repos, NULL);
		json_to_string_arrays(tb[BRANCH_EXTRA_REPOS], &br->extra_repos, &br->extra_repos_names);

		br->version = strdup(blobmsg_get_string(curver));
		br->path = alloc_replace_var(blobmsg_get_string(tb[BRANCH_PATH]), "version", br->version);
		br->path_packages = alloc_replace_var(blobmsg_get_string(tb[BRANCH_PATH_PACKAGES]), "branch", br->name);
		br->arch_packages = arch_packages;
		if (!br->path || !br->path_packages) {
			free(br);
			continue;
		}

		asprintf(&board_json_file, "%s/%s/%s/%s/%s/%s%s", serverurl, API_JSON,
			br->path, API_TARGETS, target, board_name, API_JSON_EXT);
		tmp = board_json_file;
		while ((tmp = strchr(tmp, ',')))
			*tmp = '_';

		if (request_target(br, board_json_file)) {
			free(board_json_file);
			free(br);
			continue;
		}

		free(board_json_file);
		list_add_tail(&br->list, &branches);
	}
}

static int request_branches(bool only_active)
{
	static struct blob_buf brbuf;
	struct blob_attr *cur;
	struct blob_attr *tb[__REPLY_MAX];
	int rem;
	char url[256];

	blobmsg_buf_init(&brbuf);
	snprintf(url, sizeof(url), "%s/%s/%s%s", serverurl, API_JSON,
		API_BRANCHES, API_JSON_EXT);

	if ((rc = server_request(url, NULL, &brbuf))) {
		blob_buf_free(&brbuf);
		return rc;
	};

	blobmsg_parse(reply_policy, __REPLY_MAX, tb, blob_data(brbuf.head), blob_len(brbuf.head));

	if (!tb[REPLY_ARRAY])
		return -ENODATA;

	blobmsg_for_each_attr(cur, tb[REPLY_ARRAY], rem)
		process_branch(cur, only_active);

	blob_buf_free(&brbuf);

	return 0;
}

static struct branch *select_branch(char *name, char *select_version)
{
	struct branch *br, *abr = NULL;

	if (!name)
		name = version;

	list_for_each_entry(br, &branches, list) {
		/* if branch name doesn't match version *prefix*, skip */
		if (strncmp(br->name, name, strlen(br->name)))
			continue;

		if (select_version) {
			if (!strcmp(br->version, select_version)) {
				abr = br;
				break;
			}
		} else {
			/* if we are on a snapshot branch, stay there */
			if (strcasestr(version, "snapshot")) {
				if (strcasestr(br->version, "snapshot")) {
					abr = br;
					break;
				}
			} else {
				if (!abr || (verrevcmp(br->version, abr->version) > 0))
					abr = br;
			}
		}
	}

	return abr;
}

static int add_upg_packages(struct blob_attr *reply, char *arch)
{
	struct blob_attr *tbr[__REPLY_MAX];
	struct blob_attr *tba[__PACKAGES_MAX];
	struct blob_attr *packages;
	struct blob_attr *cur;
	int rem;
	struct avl_pkg *avpk;

	blobmsg_parse(reply_policy, __REPLY_MAX, tbr, blob_data(reply), blob_len(reply));

	if (!tbr[REPLY_OBJECT])
			return -ENODATA;

	if (arch) {
		blobmsg_parse(packages_policy, __PACKAGES_MAX, tba, blobmsg_data(tbr[REPLY_OBJECT]), blobmsg_len(tbr[REPLY_OBJECT]));
		if (!tba[PACKAGES_ARCHITECTURE] ||
		    !tba[PACKAGES_PACKAGES])
			return -ENODATA;

		if (strcmp(blobmsg_get_string(tba[PACKAGES_ARCHITECTURE]), arch))
			return -EBADMSG;

		packages = tba[PACKAGES_PACKAGES];
	} else {
		packages = tbr[REPLY_OBJECT];
	}

	blobmsg_for_each_attr(cur, packages, rem) {
		avpk = malloc(sizeof(struct avl_pkg));
		avpk->name = strdup(blobmsg_name(cur));
		avpk->version = strdup(blobmsg_get_string(cur));
		avpk->avl.key = avpk->name;
		if (!avpk->name || !avpk->version || avl_insert(&pkg_tree, &avpk->avl)) {
			fprintf(stderr, "failed to insert package %s (%s)!\n", blobmsg_name(cur), blobmsg_get_string(cur));
			if (avpk->name)
				free(avpk->name);

			if (avpk->version)
				free(avpk->version);

			free(avpk);
			return -ENOMEM;
		}
	}

	return 0;
}

static int request_packages(struct branch *branch)
{
	static struct blob_buf pkgbuf, archpkgbuf;
	char url[256];
	int ret;

	fprintf(stderr, "Requesting package lists...\n");

	blobmsg_buf_init(&archpkgbuf);
	snprintf(url, sizeof(url), "%s/%s/%s/%s/%s/%s%s", serverurl, API_JSON,
		branch->path, API_TARGETS, target, API_INDEX, API_JSON_EXT);
	if ((rc = server_request(url, NULL, &archpkgbuf))) {
		blob_buf_free(&archpkgbuf);
		return rc;
	};

	ret = add_upg_packages(archpkgbuf.head, branch->arch_packages);
	blob_buf_free(&archpkgbuf);

	if (ret)
		return ret;

	blobmsg_buf_init(&pkgbuf);
	snprintf(url, sizeof(url), "%s/%s/%s/%s/%s-%s%s", serverurl, API_JSON,
		branch->path, API_PACKAGES, branch->arch_packages, API_INDEX, API_JSON_EXT);
	if ((rc = server_request(url, NULL, &pkgbuf))) {
		blob_buf_free(&archpkgbuf);
		blob_buf_free(&pkgbuf);
		return rc;
	};

	ret = add_upg_packages(pkgbuf.head, NULL);
	blob_buf_free(&pkgbuf);

	return ret;
}


static int check_installed_packages(struct blob_attr *pkgs)
{
	static struct blob_buf allpkg;
	uint32_t id;
	int status = 0;

	blob_buf_init(&allpkg, 0);
	blobmsg_add_u8(&allpkg, "all", 1);
	blobmsg_add_string(&allpkg, "dummy", "foo");
	if (ubus_lookup_id(ctx, "rpc-sys", &id) ||
	    ubus_invoke(ctx, id, "packagelist", allpkg.head, pkglist_check_cb, &status, 3000)) {
		fprintf(stderr, "cannot request packagelist from rpcd\n");
		status |= PKG_ERROR;
	}

	return status;
}

static int req_add_selected_packages(struct blob_buf *req)
{
	static struct blob_buf allpkg;
	uint32_t id;

	blob_buf_init(&allpkg, 0);
	blobmsg_add_u8(&allpkg, "all", 0);
	blobmsg_add_string(&allpkg, "dummy", "foo");
	if (ubus_lookup_id(ctx, "rpc-sys", &id) ||
	    ubus_invoke(ctx, id, "packagelist", allpkg.head, pkglist_req_cb, req, 3000)) {
		fprintf(stderr, "cannot request packagelist from rpcd\n");
		return -EFAULT;
	}

	return 0;
}

static int select_image(struct blob_attr *images, char **image_name, char **image_sha256)
{
	struct blob_attr *tb[__IMAGES_MAX];
	struct blob_attr *cur;
	int rem, ret = -ENOENT;

	blobmsg_for_each_attr(cur, images, rem) {
		blobmsg_parse(images_policy, __IMAGES_MAX, tb, blobmsg_data(cur), blobmsg_len(cur));
		if (!tb[IMAGES_FILESYSTEM] ||
		    !tb[IMAGES_NAME] ||
		    !tb[IMAGES_TYPE] ||
		    !tb[IMAGES_SHA256])
			continue;

		if (!strcmp(blobmsg_get_string(tb[IMAGES_TYPE]), "sysupgrade")) {
			*image_name = strdup(blobmsg_get_string(tb[IMAGES_NAME]));
			*image_sha256 = strdup(blobmsg_get_string(tb[IMAGES_SHA256]));
			ret = 0;
			break;
		}
	}

	return ret;
}

static bool validate_sha256(char *filename, char *sha256str)
{
	char *cmd = calloc(strlen(SHA256SUM) + 1 + strlen(filename) + 1, sizeof(char));
	size_t reslen = (64 + 2 + strlen(filename) + 1) * sizeof(char);
	char *resstr = malloc(reslen);
	FILE *f;
	bool ret = false;

	strcpy(cmd, SHA256SUM);
	strcat(cmd, " ");
	strcat(cmd, filename);

	f = popen(cmd, "r");
	if (!f)
		goto sha256free;

	if (fread(resstr, reslen, 1, f) < 1)
		goto sha256close;

	if (!strncmp(sha256str, resstr, 64))
		ret = true;

sha256close:
	fflush(f);
	fclose(f);
sha256free:
	free(cmd);
	free(resstr);

	return ret;
}

static inline bool status_delay(const char *status)
{
	return !strcmp(API_STATUS_QUEUED, status) ||
	       !strcmp(API_STATUS_STARTED, status);
}

/* this main function is too big... todo: split */
int main(int args, char *argv[]) {
	static struct blob_buf checkbuf, infobuf, reqbuf, imgbuf, upgbuf;
	struct branch *branch;
	uint32_t id;
	int valid;
	char url[256];
	char *sanetized_board_name, *image_name, *image_sha256, *tmp;
	struct blob_attr *tbr[__REPLY_MAX];
	struct blob_attr *tb[__TARGET_MAX] = {}; /* make sure tb is NULL initialized even if blobmsg_parse isn't called */
	struct stat imgstat;
	int check_only = 0;
	int retry_delay = 0;
	int upg_check = 0;
	int revcmp;
	unsigned char argc = 1;
	bool force = false, use_get = false, in_queue = false;

	snprintf(user_agent, sizeof(user_agent), "%s (%s)", argv[0], AUC_VERSION);
	fprintf(stdout, "%s\n", user_agent);

	while (argc<args) {
		if (!strncmp(argv[argc], "-h", 3) ||
		    !strncmp(argv[argc], "--help", 7)) {
			fprintf(stdout, "%s: Attended sysUpgrade CLI client\n", argv[0]);
			fprintf(stdout, "Usage: auc [-d] [-h]\n");
			fprintf(stdout, " -c\tonly check if system is up-to-date\n");
			fprintf(stdout, " -f\tuse force\n");
#ifdef AUC_DEBUG
			fprintf(stdout, " -d\tenable debugging output\n");
#endif
			fprintf(stdout, " -h\toutput help\n");
			return 0;
		}

#ifdef AUC_DEBUG
		if (!strncmp(argv[argc], "-d", 3))
			debug = 1;
#endif
		if (!strncmp(argv[argc], "-c", 3))
			check_only = 1;

		if (!strncmp(argv[argc], "-f", 3))
			force = true;

		argc++;
	};

	if (load_config()) {
		rc=-EFAULT;
		goto freeubus;
	}

	if (chdir("/tmp")) {
		rc=-EFAULT;
		goto freeconfig;
	}

	if (!strncmp(serverurl, "https", 5)) {
		rc = init_ustream_ssl();
		if (rc == -2) {
			fprintf(stderr, "No CA certificates loaded, please install ca-certificates\n");
			rc=-1;
			goto freessl;
		}

		if (rc || !ssl_ctx) {
			fprintf(stderr, "SSL support not available, please install ustream-ssl\n");
			rc=-EPROTONOSUPPORT;
			goto freessl;
		}
	}

	uloop_init();
	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "failed to connect to ubus.\n");
		return -1;
	}

	blobmsg_buf_init(&checkbuf);
	blobmsg_buf_init(&infobuf);
	blobmsg_buf_init(&reqbuf);
	blobmsg_buf_init(&imgbuf);
	/* ubus requires BLOBMSG_TYPE_UNSPEC */
	blob_buf_init(&upgbuf, 0);

	if (ubus_lookup_id(ctx, "system", &id) ||
	    ubus_invoke(ctx, id, "board", NULL, board_cb, &checkbuf, 3000)) {
		fprintf(stderr, "cannot request board info from procd\n");
		rc=-EFAULT;
		goto freebufs;
	}

	fprintf(stdout, "Running:   %s %s on %s (%s)\n", version, revision, target, board_name);

	if (request_branches(true)) {
		rc=-ENETUNREACH;
		goto freeboard;
	}

	branch = select_branch(NULL, NULL);
	if (!branch) {
		rc=-EINVAL;
		goto freebranches;
	}

	fprintf(stdout, "Available: %s %s\n", branch->version_number, branch->version_code);

	revcmp = strcmp(revision, branch->version_code);
	if (revcmp < 0)
			upg_check |= PKG_UPGRADE;
	else if (revcmp > 0)
			upg_check |= PKG_DOWNGRADE;

	if ((rc = request_packages(branch)))
		goto freebranches;

	upg_check |= check_installed_packages(reqbuf.head);
	if (upg_check & PKG_ERROR) {
		rc=-ENOPKG;
		goto freebranches;
	}
	if (!upg_check && !force) {
		fprintf(stderr, "Nothing to be updated. Use '-f' to force.\n");
		rc=0;
		goto freebranches;
	};

	if (!force && (upg_check & PKG_DOWNGRADE)) {
		fprintf(stderr, "Refusing to downgrade. Use '-f' to force.\n");
		rc=-ENOTRECOVERABLE;
		goto freebranches;
	};

	if (check_only)
		goto freebranches;

	rc = ask_user();
	if (rc)
		goto freebranches;

	blobmsg_add_string(&reqbuf, "version", branch->version);
	blobmsg_add_string(&reqbuf, "version_code", branch->version_code);
	blobmsg_add_string(&reqbuf, "target", target);

	sanetized_board_name = strdup(board_name);
	tmp = sanetized_board_name;
	while ((tmp = strchr(tmp, ',')))
		*tmp = '_';

	blobmsg_add_string(&reqbuf, "profile", sanetized_board_name);
	blobmsg_add_u8(&reqbuf, "diff_packages", 1);

	req_add_selected_packages(&reqbuf);

	snprintf(url, sizeof(url), "%s/%s", serverurl, API_REQUEST);

	use_get = false;
	do {
		retry = false;

		DPRINTF("requesting from %s\n%s%s", url, use_get?"":blobmsg_format_json_indent(reqbuf.head, true, 0), use_get?"":"\n");

		rc = server_request(url, use_get?NULL:&reqbuf, &imgbuf);
		if (rc)
			break;

		blobmsg_parse(reply_policy, __REPLY_MAX, tbr, blob_data(imgbuf.head), blob_len(imgbuf.head));
		if (!tbr[REPLY_OBJECT])
			break;

		blobmsg_parse(target_policy, __TARGET_MAX, tb, blobmsg_data(tbr[REPLY_OBJECT]), blobmsg_len(tbr[REPLY_OBJECT]));

		if (tb[TARGET_REQUEST_HASH] && tb[TARGET_STATUS]) {
			if (status_delay(blobmsg_get_string(tb[TARGET_STATUS]))) {
				if (!retry_delay)
					fputs("Requesting build", stderr);

				retry_delay = 2;
				if (tb[TARGET_QUEUE_POSITION]) {
					fprintf(stderr, "%s%s (position in queue: %d)",
						ANSI_ESC, in_queue?ANSI_CURSOR_RESTORE:ANSI_CURSOR_SAFE,
						blobmsg_get_u32(tb[TARGET_QUEUE_POSITION]));
					in_queue = true;
				} else {
					if (in_queue)
						fprintf(stderr, "%s%s%s%s",
							ANSI_ESC, ANSI_CURSOR_RESTORE,
							ANSI_ESC, ANSI_ERASE_LINE);
					fputc('.', stderr);
					in_queue = false;
				}
			} else {
				retry_delay = 0;
			}
			if (!use_get) {
				snprintf(url, sizeof(url), "%s/%s/%s", serverurl,
					 API_REQUEST,
					 blobmsg_get_string(tb[TARGET_REQUEST_HASH]));
				DPRINTF("polling via GET %s\n", url);
			}
			retry = true;
			use_get = true;
		} else if (retry_delay) {
			fputc('\n', stderr);
			retry_delay = 0;
		}

#ifdef AUC_DEBUG
		if (debug && tb[TARGET_STDOUT])
			fputs(blobmsg_get_string(tb[TARGET_STDOUT]), stdout);

		if (debug && tb[TARGET_STDERR])
			fputs(blobmsg_get_string(tb[TARGET_STDERR]), stderr);
#endif

		if (retry) {
			blob_buf_free(&imgbuf);
			blobmsg_buf_init(&imgbuf);
			sleep(retry_delay);
		}
	} while(retry);

	free(sanetized_board_name);

	if (!tb[TARGET_IMAGES] || !tb[TARGET_BINDIR]) {
		if (!rc)
			rc=-EBADMSG;
		goto freebranches;
	}

	if ((rc = select_image(tb[TARGET_IMAGES], &image_name, &image_sha256)))
		goto freebranches;

	snprintf(url, sizeof(url), "%s/%s/%s/%s", serverurl, API_STORE,
	         blobmsg_get_string(tb[TARGET_BINDIR]),
	         image_name);

	DPRINTF("downloading image from %s\n", url);
	rc = server_request(url, NULL, NULL);
	if (rc)
		goto freebranches;

	filename = uclient_get_url_filename(url, "firmware.bin");

	if (stat(filename, &imgstat)) {
		fprintf(stderr, "image download failed\n");
		rc=-EPIPE;
		goto freebranches;
	}

	if ((intmax_t)imgstat.st_size != out_len) {
		fprintf(stderr, "file size mismatch\n");
		unlink(filename);
		rc=-EMSGSIZE;
		goto freebranches;
	}

	if (!validate_sha256(filename, image_sha256)) {
		fprintf(stderr, "sha256 mismatch\n");
		unlink(filename);
		rc=-EBADMSG;
		goto freebranches;
	}

	if (strcmp(filename, "firmware.bin")) {
		if (rename(filename, "firmware.bin")) {
			fprintf(stderr, "can't rename to firmware.bin\n");
			unlink(filename);
			rc=-errno;
			goto freebranches;
		}
	}

	valid = 0;
	if (ubus_lookup_id(ctx, "rpc-sys", &id) ||
	    ubus_invoke(ctx, id, "upgrade_test", NULL, upgtest_cb, &valid, 15000)) {
		rc=-EFAULT;
		goto freebranches;
	}

	if (!valid) {
		rc=-EINVAL;
		goto freebranches;
	}

	fprintf(stdout, "invoking sysupgrade\n");
	blobmsg_add_u8(&upgbuf, "keep", 1);
	ubus_invoke(ctx, id, "upgrade_start", upgbuf.head, NULL, NULL, 120000);
	sleep(10);

freebranches:
	if (rc && tb[TARGET_STDOUT]
#ifdef AUC_DEBUG
	    && !debug
#endif
	    )
		fputs(blobmsg_get_string(tb[TARGET_STDOUT]), stdout);
	if (rc && tb[TARGET_STDERR]
#ifdef AUC_DEBUG
	    && !debug
#endif
	    )
		fputs(blobmsg_get_string(tb[TARGET_STDERR]), stderr);

	if (tb[TARGET_MESSAGE]) {
		fputs(blobmsg_get_string(tb[TARGET_MESSAGE]), stderr);
		fputc('\n', stderr);
	}

	/* ToDo */
freeboard:
	free(board_name);
	free(target);
	free(distribution);
	free(version);

freebufs:
	blob_buf_free(&checkbuf);
	blob_buf_free(&infobuf);
	blob_buf_free(&reqbuf);
	blob_buf_free(&imgbuf);
	blob_buf_free(&upgbuf);

freessl:
	if (ssl_ctx)
		ssl_ops->context_free(ssl_ctx);

freeconfig:
	free(serverurl);

freeubus:
	uloop_done();
	ubus_free(ctx);

	if (ucl)
		uclient_free(ucl);

	if (rc)
		fprintf(stderr, "%s (%d)\n", strerror(-1 * rc), -1 * rc);

	return rc;
}
