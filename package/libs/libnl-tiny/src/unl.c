#define _GNU_SOURCE
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <netlink/genl/family.h>
#include <sys/types.h>
#include <net/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/nl80211.h>

#include "unl.h"

static int unl_init(struct unl *unl)
{
	unl->sock = nl_socket_alloc();
	if (!unl->sock)
		return -1;

	return 0;
}

int unl_genl_init(struct unl *unl, const char *family)
{
	memset(unl, 0, sizeof(*unl));

	if (unl_init(unl))
		goto error_out;

	unl->hdrlen = NLMSG_ALIGN(sizeof(struct genlmsghdr));
	unl->family_name = strdup(family);
	if (!unl->family_name)
		goto error;

	if (genl_connect(unl->sock))
		goto error;

	if (genl_ctrl_alloc_cache(unl->sock, &unl->cache))
		goto error;

	unl->family = genl_ctrl_search_by_name(unl->cache, family);
	if (!unl->family)
		goto error;

	return 0;

error:
	unl_free(unl);
error_out:
	return -1;
}

void unl_free(struct unl *unl)
{
	if (unl->family_name)
		free(unl->family_name);

	if (unl->sock)
		nl_socket_free(unl->sock);

	if (unl->cache)
		nl_cache_free(unl->cache);

	memset(unl, 0, sizeof(*unl));
}

static int
ack_handler(struct nl_msg *msg, void *arg)
{
	int *err = arg;
	*err = 0;
	return NL_STOP;
}

static int
finish_handler(struct nl_msg *msg, void *arg)
{
	int *err = arg;
	*err = 0;
	return NL_SKIP;
}

static int
error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg)
{
	int *ret = arg;
	*ret = err->error;
	return NL_SKIP;
}

struct nl_msg *unl_genl_msg(struct unl *unl, int cmd, bool dump)
{
	struct nl_msg *msg;
	int flags = 0;

	msg = nlmsg_alloc();
	if (!msg)
		goto out;

	if (dump)
		flags |= NLM_F_DUMP;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ,
		    genl_family_get_id(unl->family), 0, flags, cmd, 0);

out:
	return msg;
}

int unl_genl_request(struct unl *unl, struct nl_msg *msg, unl_cb handler, void *arg)
{
	struct nl_cb *cb;
	int err;

	cb = nl_cb_alloc(NL_CB_CUSTOM);
	err = nl_send_auto_complete(unl->sock, msg);
	if (err < 0)
		goto out;

	err = 1;
	nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
	if (handler)
		nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handler, arg);

	while (err > 0)
		nl_recvmsgs(unl->sock, cb);

out:
	nlmsg_free(msg);
	nl_cb_put(cb);
	return err;
}

static int request_single_cb(struct nl_msg *msg, void *arg)
{
	struct nl_msg **dest = arg;

	if (!*dest) {
		nlmsg_get(msg);
		*dest = msg;
	}
	return NL_SKIP;
}

int unl_genl_request_single(struct unl *unl, struct nl_msg *msg, struct nl_msg **dest)
{
	*dest = NULL;
	return unl_genl_request(unl, msg, request_single_cb, dest);
}

static int no_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

void unl_genl_loop(struct unl *unl, unl_cb handler, void *arg)
{
	struct nl_cb *cb;

	cb = nl_cb_alloc(NL_CB_CUSTOM);
	unl->loop_done = false;
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, handler, arg);

	while (!unl->loop_done)
		nl_recvmsgs(unl->sock, cb);

	nl_cb_put(cb);
}

int unl_genl_multicast_id(struct unl *unl, const char *name)
{
	struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX + 1];
	struct nlattr *groups, *group;
	struct nl_msg *msg;
	int ctrlid;
	int ret = -1;
	int rem;

	msg = nlmsg_alloc();
	if (!msg)
		return -1;

	ctrlid = genl_ctrl_resolve(unl->sock, "nlctrl");
	genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, unl->family_name);
	unl_genl_request_single(unl, msg, &msg);
	if (!msg)
		return -1;

	groups = unl_find_attr(unl, msg, CTRL_ATTR_MCAST_GROUPS);
	if (!groups)
		goto nla_put_failure;

	nla_for_each_nested(group, groups, rem) {
		const char *gn;

		nla_parse(tb, CTRL_ATTR_MCAST_GRP_MAX, nla_data(group),
			  nla_len(group), NULL);

		if (!tb[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb[CTRL_ATTR_MCAST_GRP_ID])
			continue;

		gn = nla_data(tb[CTRL_ATTR_MCAST_GRP_NAME]);
		if (strcmp(gn, name) != 0)
			continue;

		ret = nla_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	}

nla_put_failure:
	nlmsg_free(msg);
	return ret;
}

int unl_genl_subscribe(struct unl *unl, const char *name)
{
	int mcid;

	mcid = unl_genl_multicast_id(unl, name);
	if (mcid < 0)
		return mcid;

	return nl_socket_add_membership(unl->sock, mcid);
}

int unl_genl_unsubscribe(struct unl *unl, const char *name)
{
	int mcid;

	mcid = unl_genl_multicast_id(unl, name);
	if (mcid < 0)
		return mcid;

	return nl_socket_drop_membership(unl->sock, mcid);
}

int unl_nl80211_phy_lookup(const char *name)
{
	char buf[32];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/%s/index", name);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	return atoi(buf);
}

int unl_nl80211_wdev_to_phy(struct unl *unl, int wdev)
{
	struct nl_msg *msg;
	struct nlattr *attr;
	int ret = -1;

	msg = unl_genl_msg(unl, NL80211_CMD_GET_INTERFACE, false);
	if (!msg)
		return -1;

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	if (unl_genl_request_single(unl, msg, &msg) < 0)
		return -1;

	attr = unl_find_attr(unl, msg, NL80211_ATTR_WIPHY);
	if (!attr)
		goto out;

	ret = nla_get_u32(attr);
out:
nla_put_failure:
	nlmsg_free(msg);
	return ret;
}


