/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _OTTO_L3_H
#define _OTTO_L3_H

#include "rtl-otto.h"

#define MAX_HOST_ROUTES		1536
#define MAX_ROUTES		512
#define MAX_INTERFACES		100

#define HASH_PICK(val, lsb, len) ((val & (((1 << len) - 1) << lsb)) >> lsb)

/* An entry in the RTL93XX SoC's ROUTER_MAC tables setting up a termination point
 * for the L3 routing system. Packets arriving and matching an entry in this table
 * will be considered for routing.
 * Mask fields state whether the corresponding data fields matter for matching
 */
struct otto_l3_router_mac {
	bool valid;	/* Valid or not */
	bool p_type;	/* Individual (0) or trunk (1) port */
	bool p_mask;	/* Whether the port type is used */
	u8 p_id;
	u8 p_id_mask;	/* Mask for the port */
	u8 action;	/* Routing action performed: 0: FORWARD, 1: DROP, 2: TRAP2CPU */
			/*   3: COPY2CPU, 4: TRAP2MASTERCPU, 5: COPY2MASTERCPU, 6: HARDDROP */
	u16 vid;
	u16 vid_mask;
	u64 mac;	/* MAC address used as source MAC in the routed packet */
	u64 mac_mask;
};

struct otto_l3_intf {
	u16 vid;
	u8 smac_idx;
	u8 ip4_mtu_id;
	u8 ip6_mtu_id;
	u16 ip4_mtu;
	u16 ip6_mtu;
	u8 ttl_scope;
	u8 hl_scope;
	u8 ip4_icmp_redirect;
	u8 ip6_icmp_redirect;
	u8 ip4_pbr_icmp_redirect;
	u8 ip6_pbr_icmp_redirect;
};

struct otto_l3_route_attr {
	bool valid;
	bool hit;
	bool ttl_dec;
	bool ttl_check;
	bool dst_null;
	bool qos_as;
	u8 qos_prio;
	u8 type;
	u8 action;
};

struct otto_l3_nexthop {
	u16 id;		/* ID: L3_NEXT_HOP table-index or route-index set in L2_NEXT_HOP */
	u32 dev_id;
	u16 port;
	u16 vid;	/* VLAN-ID for L2 table entry (saved from L2-UC entry) */
	u16 rvid;	/* Relay VID/FID for the L2 table entry */
	u64 mac;	/* The MAC address of the entry in the L2_NEXT_HOP table */
	u16 mac_id;
	u16 l2_id;	/* Index of this next hop forwarding entry in L2 FIB table */
	u64 gw;		/* The gateway MAC address packets are forwarded to */
	int if_id;	/* Interface (into L3_EGR_INTF_IDX) */
};

struct otto_l3_route {
	u32 gw_ip;			/* IP of the route's gateway */
	u32 dst_ip;			/* IP of the destination net */
	struct in6_addr dst_ip6;
	int prefix_len;			/* Network prefix len of the destination net */
	bool is_host_route;
	int id;				/* ID number of this route */
	struct rhlist_head linkage;
	u16 switch_mac_id;		/* Index into switch's own MACs, RTL839X only */
	struct otto_l3_nexthop nh;
	struct pie_rule pr;
	struct otto_l3_route_attr attr;
};

struct otto_l3_config {
	int (*find_slot)(struct otto_l3_ctrl *ctrl, struct otto_l3_route *rt, bool must_exist);
	void (*set_egress_intf)(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_intf *intf);
	u64 (*get_egress_mac)(struct otto_l3_ctrl *ctrl, u32 idx);
	void (*set_egress_mac)(struct otto_l3_ctrl *ctrl, u32 idx, u64 mac);
	void (*host_route_write)(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt);
	void (*get_router_mac)(struct otto_l3_ctrl *ctrl, u32 idx, struct otto_l3_router_mac *m);
	void (*set_router_mac)(struct otto_l3_ctrl *ctrl, u32 idx, struct otto_l3_router_mac *m);
	void (*get_nexthop)(struct otto_l3_ctrl *ctrl, int idx, u16 *dmac_id, u16 *interface);
	void (*set_nexthop)(struct otto_l3_ctrl *ctrl, int idx, u16 dmac_id, u16 interface);
	int (*route_lookup_hw)(struct otto_l3_ctrl *ctrl, struct otto_l3_route *rt);
	void (*route_read)(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt);
	void (*route_write)(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt);
	int (*setup)(struct otto_l3_ctrl *ctrl);
};

struct otto_l3_ctrl {
	const struct otto_l3_config *cfg;
	struct device *dev;
	struct rtl838x_switch_priv *priv;
	struct notifier_block fib_nb;
	struct notifier_block ne_nb;
	struct rhltable routes;
	unsigned long route_use_bm[MAX_ROUTES / 32];
	unsigned long host_route_use_bm[MAX_HOST_ROUTES / 32];
	struct otto_l3_intf *interfaces[MAX_INTERFACES];
	struct mutex *lock; /* protect register access */
};

int otto_l3_probe(struct device *dev, struct rtl838x_switch_priv *priv);
void otto_l3_remove(struct rtl838x_switch_priv *priv);

#endif /* _OTTO_L3_H */
