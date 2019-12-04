#ifndef __UNL_H
#define __UNL_H

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <stdbool.h>

struct unl {
	struct nl_sock *sock;
	struct nl_cache *cache;
	struct genl_family *family;
	char *family_name;
	int hdrlen;
	bool loop_done;
};

int unl_genl_init(struct unl *unl, const char *family);
void unl_free(struct unl *unl);

typedef int (*unl_cb)(struct nl_msg *, void *);

struct nl_msg *unl_genl_msg(struct unl *unl, int cmd, bool dump);
int unl_genl_request(struct unl *unl, struct nl_msg *msg, unl_cb handler, void *arg);
int unl_genl_request_single(struct unl *unl, struct nl_msg *msg, struct nl_msg **dest);
void unl_genl_loop(struct unl *unl, unl_cb handler, void *arg);

int unl_genl_multicast_id(struct unl *unl, const char *name);
int unl_genl_subscribe(struct unl *unl, const char *name);
int unl_genl_unsubscribe(struct unl *unl, const char *name);

int unl_nl80211_phy_lookup(const char *name);
int unl_nl80211_wdev_to_phy(struct unl *unl, int wdev);
struct nl_msg *unl_nl80211_phy_msg(struct unl *unl, int phy, int cmd, bool dump);
struct nl_msg *unl_nl80211_vif_msg(struct unl *unl, int dev, int cmd, bool dump);

static inline void unl_loop_done(struct unl *unl)
{
	unl->loop_done = true;
}

static inline struct nlattr *unl_find_attr(struct unl *unl, struct nl_msg *msg, int attr)
{
	return nlmsg_find_attr(nlmsg_hdr(msg), unl->hdrlen, attr);
}

#endif
