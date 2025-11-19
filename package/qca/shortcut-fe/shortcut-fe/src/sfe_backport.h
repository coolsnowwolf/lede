/*
 * sfe_backport.h
 *	Shortcut forwarding engine compatible header file.
 *
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#include <net/netfilter/nf_conntrack_timeout.h>
#else
enum udp_conntrack {
	UDP_CT_UNREPLIED,
	UDP_CT_REPLIED,
	UDP_CT_MAX
};

static inline unsigned int *
nf_ct_timeout_lookup(struct net *net, struct nf_conn *ct,
		     struct nf_conntrack_l4proto *l4proto)
{
#ifdef CONFIG_NF_CONNTRACK_TIMEOUT
	struct nf_conn_timeout *timeout_ext;
	unsigned int *timeouts;

	timeout_ext = nf_ct_timeout_find(ct);
	if (timeout_ext)
		timeouts = NF_CT_TIMEOUT_EXT_DATA(timeout_ext);
	else
		timeouts = l4proto->get_timeouts(net);

	return timeouts;
#else
	return l4proto->get_timeouts(net);
#endif /*CONFIG_NF_CONNTRACK_TIMEOUT*/
}
#endif /*KERNEL_VERSION(3, 7, 0)*/
#endif /*KERNEL_VERSION(3, 4, 0)*/

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define sfe_define_post_routing_hook(FN_NAME, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
static unsigned int FN_NAME(void *priv, \
			    struct sk_buff *SKB, \
			    const struct nf_hook_state *state)
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
#define sfe_define_post_routing_hook(FN_NAME, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
static unsigned int FN_NAME(const struct nf_hook_ops *OPS, \
			    struct sk_buff *SKB, \
			    const struct net_device *UNUSED, \
			    const struct net_device *OUT, \
			    int (*OKFN)(struct sk_buff *))
#else
#define sfe_define_post_routing_hook(FN_NAME, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
static unsigned int FN_NAME(unsigned int HOOKNUM, \
			    struct sk_buff *SKB, \
			    const struct net_device *UNUSED, \
			    const struct net_device *OUT, \
			    int (*OKFN)(struct sk_buff *))
#endif

#define sfe_cm_ipv4_post_routing_hook(HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
	sfe_define_post_routing_hook(__sfe_cm_ipv4_post_routing_hook, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN)
#define sfe_cm_ipv6_post_routing_hook(HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
	sfe_define_post_routing_hook(__sfe_cm_ipv6_post_routing_hook, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN)
#define fast_classifier_ipv4_post_routing_hook(HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
	sfe_define_post_routing_hook(__fast_classifier_ipv4_post_routing_hook, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN)
#define fast_classifier_ipv6_post_routing_hook(HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN) \
	sfe_define_post_routing_hook(__fast_classifier_ipv6_post_routing_hook, HOOKNUM, OPS, SKB, UNUSED, OUT, OKFN)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define SFE_IPV4_NF_POST_ROUTING_HOOK(fn) \
	{						\
		.hook = fn,				\
		.pf = NFPROTO_IPV4,			\
		.hooknum = NF_INET_POST_ROUTING,	\
		.priority = NF_IP_PRI_NAT_SRC + 1,	\
	}
#else
#define SFE_IPV4_NF_POST_ROUTING_HOOK(fn) \
	{						\
		.hook = fn,				\
		.owner = THIS_MODULE,			\
		.pf = NFPROTO_IPV4,			\
		.hooknum = NF_INET_POST_ROUTING,	\
		.priority = NF_IP_PRI_NAT_SRC + 1,	\
	}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#define SFE_IPV6_NF_POST_ROUTING_HOOK(fn) \
	{						\
		.hook = fn,				\
		.pf = NFPROTO_IPV6,			\
		.hooknum = NF_INET_POST_ROUTING,	\
		.priority = NF_IP_PRI_NAT_SRC + 1,	\
	}
#else
#define SFE_IPV6_NF_POST_ROUTING_HOOK(fn) \
	{						\
		.hook = fn,				\
		.owner = THIS_MODULE,			\
		.pf = NFPROTO_IPV6,			\
		.hooknum = NF_INET_POST_ROUTING,	\
		.priority = NF_IP6_PRI_NAT_SRC + 1,	\
	}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 3, 0))
#define SFE_NF_CT_DEFAULT_ZONE (&nf_ct_zone_dflt)
#else
#define SFE_NF_CT_DEFAULT_ZONE NF_CT_DEFAULT_ZONE
#endif

/*
 * sfe_dev_get_master
 * 	get master of bridge port, and hold it
 */
static inline struct net_device *sfe_dev_get_master(struct net_device *dev)
{
	struct net_device *master;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	rcu_read_lock();
	master = netdev_master_upper_dev_get_rcu(dev);
	if (master)
		dev_hold(master);

	rcu_read_unlock();
#else
	master = dev->master;
	if (master)
		dev_hold(master);
#endif
	return master;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
#define SFE_DEV_EVENT_PTR(PTR) netdev_notifier_info_to_dev(PTR)
#else
#define SFE_DEV_EVENT_PTR(PTR) (struct net_device *)(PTR)
#endif

/*
 * declare function sfe_cm_device_event
 */
int sfe_cm_recv(struct sk_buff *skb);
int sfe_cm_device_event(struct notifier_block *this, unsigned long event, void *ptr);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
#define SFE_NF_CONN_ACCT(NM) struct nf_conn_acct *NM
#else
#define SFE_NF_CONN_ACCT(NM) struct nf_conn_counter *NM
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
#define SFE_ACCT_COUNTER(NM) ((NM)->counter)
#else
#define SFE_ACCT_COUNTER(NM) (NM)
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#define sfe_hash_for_each_possible(name, obj, node, member, key) \
	hash_for_each_possible(name, obj, member, key)
#else
#define sfe_hash_for_each_possible(name, obj, node, member, key) \
	hash_for_each_possible(name, obj, node, member, key)
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#define sfe_hash_for_each(name, bkt, node, obj, member) \
	hash_for_each(name, bkt, obj, member)
#else
#define sfe_hash_for_each(name, bkt, node, obj, member) \
	hash_for_each(name, bkt, node, obj, member)
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
#define sfe_dst_get_neighbour(dst, daddr) dst_neigh_lookup(dst, addr)
#else
static inline struct neighbour *
sfe_dst_get_neighbour(struct dst_entry *dst, void *daddr)
{
	struct neighbour *neigh = dst_get_neighbour_noref(dst);

	if (neigh)
		neigh_hold(neigh);

	return neigh;
}
#endif
