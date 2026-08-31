// SPDX-License-Identifier: GPL-2.0-only

#include <linux/if_vlan.h>
#include <linux/inetdevice.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/rhashtable.h>
#include <net/arp.h>
#include <net/fib_notifier.h>
#include <net/ip6_fib.h>
#include <net/netevent.h>
#include <net/nexthop.h>
#include <uapi/linux/rtnetlink.h>

#include "l3.h"
#include "rtl-otto.h"

static const struct rhashtable_params otto_l3_route_ht_params = {
	.key_len     = sizeof(u32),
	.key_offset  = offsetof(struct otto_l3_route, gw_ip),
	.head_offset = offsetof(struct otto_l3_route, linkage),
};

struct otto_l3_net_event_work {
	struct work_struct work;
	struct otto_l3_ctrl *ctrl;
	u64 mac;
	u32 gw_addr;
};

struct otto_l3_fib_event_work {
	struct work_struct work;
	struct otto_l3_ctrl *ctrl;
	union {
		struct fib_entry_notifier_info fen_info;
		struct fib6_entry_notifier_info fen6_info;
		struct fib_rule_notifier_info fr_info;
	};
	bool is_fib6;
	unsigned long event;
};

struct otto_l3_walk_data {
	struct otto_l3_ctrl *ctrl;
	int port;
};

static void otto_l3_838x_route_read(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL8380_TBL_1, 2);

	rtl_table_read(r, idx);

	/* The table has a size of 2 registers */
	rt->nh.gw = sw_r32(rtl_table_data(r, 0));
	rt->nh.gw <<= 32;
	rt->nh.gw |= sw_r32(rtl_table_data(r, 1));

	rtl_table_release(r);
}

static void otto_l3_838x_route_write(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL8380_TBL_1, 2);

	sw_w32(rt->nh.gw >> 32, rtl_table_data(r, 0));
	sw_w32(rt->nh.gw, rtl_table_data(r, 1));
	rtl_table_write(r, idx);

	rtl_table_release(r);
}

static void otto_l3_839x_route_read(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 2);
	u64 v;

	rtl_table_read(r, idx);

	/* The table has a size of 2 registers */
	v = sw_r32(rtl_table_data(r, 0));
	v <<= 32;
	v |= sw_r32(rtl_table_data(r, 1));
	rt->switch_mac_id = (v >> 12) & 0xf;
	rt->nh.gw = v >> 16;

	rtl_table_release(r);
}

static void otto_l3_839x_route_write(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 2);
	u32 v;

	sw_w32(rt->nh.gw >> 16, rtl_table_data(r, 0));
	v = rt->nh.gw << 16;
	v |= rt->switch_mac_id << 12;
	sw_w32(v, rtl_table_data(r, 1));
	rtl_table_write(r, idx);

	rtl_table_release(r);
}

static void otto_l3_839x_setup_port_macs(struct otto_l3_ctrl *ctrl)
{
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct net_device *dev;
	u64 mac;

	/* Configure the switch's own MAC addresses used when routing packets */
	dev_dbg(ctrl->dev, "got port %08x\n", (u32)priv->ports[priv->r->cpu_port].dp);
	dev = priv->ports[priv->r->cpu_port].dp->user;
	mac = ether_addr_to_u64(dev->dev_addr);

	for (int i = 0; i < 15; i++) {
		mac++;  /* BUG: VRRP for testing */
		sw_w32(mac >> 32, RTL839X_ROUTING_SA_CTRL + i * 8);
		sw_w32(mac, RTL839X_ROUTING_SA_CTRL + i * 8 + 4);
	}
}

static int otto_l3_839x_setup(struct otto_l3_ctrl *ctrl)
{
	otto_l3_839x_setup_port_macs(ctrl);

	return 0;
}

__maybe_unused
static u32 otto_l3_930x_hash4(u32 ip, int algorithm, bool move_dip)
{
	u32 s0, s1, pH;
	u32 rows[4];
	u32 hash;

	memset(rows, 0, sizeof(rows));

	rows[0] = HASH_PICK(ip, 27, 5);
	rows[1] = HASH_PICK(ip, 18, 9);
	rows[2] = HASH_PICK(ip, 9, 9);

	if (!move_dip)
		rows[3] = HASH_PICK(ip, 0, 9);

	if (!algorithm) {
		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3];
	} else {
		s0 = rows[0] + rows[1] + rows[2];
		s1 = (s0 & 0x1ff) + ((s0 & (0x1ff << 9)) >> 9);
		pH = (s1 & 0x1ff) + ((s1 & (0x1ff << 9)) >> 9);
		hash = pH ^ rows[3];
	}
	return hash;
}

/*
 * Get the Destination-MAC of an L3 egress interface or the Source MAC for routed packets
 * from the SoC's L3_EGR_INTF_MAC table. Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
__maybe_unused
static u64 otto_l3_930x_get_egress_mac(struct otto_l3_ctrl *ctrl, u32 idx)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_2, 2);
	u64 mac;

	rtl_table_read(r, idx);
	/* The table has a size of 2 registers */
	mac = sw_r32(rtl_table_data(r, 0));
	mac <<= 32;
	mac |= sw_r32(rtl_table_data(r, 1));
	rtl_table_release(r);

	return mac;
}

/* Set the Destination-MAC of a route or the Source MAC of an L3 egress interface
 * in the SoC's L3_EGR_INTF_MAC table. Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
__maybe_unused
static void otto_l3_930x_set_egress_mac(struct otto_l3_ctrl *ctrl, u32 idx, u64 mac)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_2, 2);

	/* The table has a size of 2 registers */
	sw_w32(mac >> 32, rtl_table_data(r, 0));
	sw_w32(mac, rtl_table_data(r, 1));

	dev_dbg(ctrl->dev, "setting index %d to %016llx\n", idx, mac);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Read a host route entry from the table using its index. Only IPv4 and IPv6 unicast supported */
__maybe_unused
static void otto_l3_930x_host_route_read(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 1);
	u32 v;

	idx = ((idx / 6) * 8) + (idx % 6);

	rtl_table_read(r, idx);
	/* The table has a size of 5 (for UC, 11 for MC) registers */
	v = sw_r32(rtl_table_data(r, 0));
	rt->attr.valid = !!(v & BIT(31));
	if (!rt->attr.valid)
		goto out;
	rt->attr.type = (v >> 29) & 0x3;
	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		rt->dst_ip = sw_r32(rtl_table_data(r, 4));
		break;
	case 2: /* IPv6 Unicast route */
		ipv6_addr_set(&rt->dst_ip6,
			      sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 2)),
			      sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 0)));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		dev_warn(ctrl->dev, "route type not supported\n");
		goto out;
	}

	rt->attr.hit = !!(v & BIT(20));
	rt->attr.dst_null = !!(v & BIT(19));
	rt->attr.action = (v >> 17) & 3;
	rt->nh.id = (v >> 6) & 0x7ff;
	rt->attr.ttl_dec = !!(v & BIT(5));
	rt->attr.ttl_check = !!(v & BIT(4));
	rt->attr.qos_as = !!(v & BIT(3));
	rt->attr.qos_prio =  v & 0x7;
	dev_dbg(ctrl->dev, "index %d is valid: %d\n", idx, rt->attr.valid);
	dev_dbg(ctrl->dev, "next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	dev_dbg(ctrl->dev, "Destination: %pI4\n", &rt->dst_ip);

out:
	rtl_table_release(r);
}

/* Write a host route entry from the table using its index. Only unicast routes supported */
__maybe_unused
static void otto_l3_930x_host_route_write(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	/* The table has a size of 5 (for UC, 11 for MC) registers */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 1);
	u32 v;

	idx = ((idx / 6) * 8) + (idx % 6);

	dev_dbg(ctrl->dev, "index %d is valid: %d\n", idx, rt->attr.valid);
	dev_dbg(ctrl->dev, "next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	dev_dbg(ctrl->dev, "GW: %pI4, prefix_len: %d\n", &rt->dst_ip, rt->prefix_len);

	v = BIT(31); /* Entry is valid */
	v |= (rt->attr.type & 0x3) << 29;
	v |= rt->attr.hit ? BIT(20) : 0;
	v |= rt->attr.dst_null ? BIT(19) : 0;
	v |= (rt->attr.action & 0x3) << 17;
	v |= (rt->nh.id & 0x7ff) << 6;
	v |= rt->attr.ttl_dec ? BIT(5) : 0;
	v |= rt->attr.ttl_check ? BIT(4) : 0;
	v |= rt->attr.qos_as ? BIT(3) : 0;
	v |= rt->attr.qos_prio & 0x7;

	sw_w32(v, rtl_table_data(r, 0));
	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		sw_w32(0, rtl_table_data(r, 1));
		sw_w32(0, rtl_table_data(r, 2));
		sw_w32(0, rtl_table_data(r, 3));
		sw_w32(rt->dst_ip, rtl_table_data(r, 4));
		break;
	case 2: /* IPv6 Unicast route */
		sw_w32(rt->dst_ip6.s6_addr32[0], rtl_table_data(r, 1));
		sw_w32(rt->dst_ip6.s6_addr32[1], rtl_table_data(r, 2));
		sw_w32(rt->dst_ip6.s6_addr32[2], rtl_table_data(r, 3));
		sw_w32(rt->dst_ip6.s6_addr32[3], rtl_table_data(r, 4));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		dev_warn(ctrl->dev, "route type not supported\n");
		goto out;
	}

	rtl_table_write(r, idx);

out:
	rtl_table_release(r);
}

__maybe_unused
static int otto_l3_930x_find_slot(struct otto_l3_ctrl *ctrl, struct otto_l3_route *rt, bool must_exist)
{
	int slot_width, algorithm, addr, idx;
	struct otto_l3_route route_entry;
	u32 hash;

	/* IPv6 entries take up 3 slots */
	slot_width = (rt->attr.type == 0) || (rt->attr.type == 2) ? 1 : 3;

	for (int t = 0; t < 2; t++) {
		algorithm = (sw_r32(RTL930X_L3_HOST_TBL_CTRL) >> (2 + t)) & 0x1;
		hash = otto_l3_930x_hash4(rt->dst_ip, algorithm, false);

		dev_dbg(ctrl->dev, "table %d, algorithm %d, hash %04x\n", t, algorithm, hash);

		for (int s = 0; s < 6; s += slot_width) {
			addr = (t << 12) | ((hash & 0x1ff) << 3) | s;
			dev_dbg(ctrl->dev, "physical address %d\n", addr);
			idx = ((addr / 8) * 6) + (addr % 8);
			dev_dbg(ctrl->dev, "logical address %d\n", idx);

			otto_l3_930x_host_route_read(ctrl, idx, &route_entry);
			dev_dbg(ctrl->dev, "route valid %d, route dest: %pI4, hit %d\n",
				rt->attr.valid, &rt->dst_ip, rt->attr.hit);
			if (!must_exist && rt->attr.valid)
				return idx;
			if (must_exist && route_entry.dst_ip == rt->dst_ip)
				return idx;
		}
	}

	return -1;
}

/*
 * Reads a MAC entry for L3 termination as entry point for routing from the hardware table.
 * idx is the index into the L3_ROUTER_MAC table
 */
__maybe_unused
static void otto_l3_930x_get_router_mac(struct otto_l3_ctrl *ctrl,
					u32 idx, struct otto_l3_router_mac *m)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 0);
	u32 v, w;

	rtl_table_read(r, idx);
	/* The table has a size of 7 registers, 64 entries */
	v = sw_r32(rtl_table_data(r, 0));
	w = sw_r32(rtl_table_data(r, 3));
	m->valid = !!(v & BIT(20));
	if (!m->valid)
		goto out;

	m->p_type = !!(v & BIT(19));
	m->p_id = (v >> 13) & 0x3f;  /* trunk id of port */
	m->vid = v & 0xfff;
	m->vid_mask = w & 0xfff;
	m->action = sw_r32(rtl_table_data(r, 6)) & 0x7;
	m->mac_mask = ((((u64)sw_r32(rtl_table_data(r, 5))) << 32) & 0xffffffffffffULL) |
		      (sw_r32(rtl_table_data(r, 4)));
	m->mac = ((((u64)sw_r32(rtl_table_data(r, 1))) << 32) & 0xffffffffffffULL) |
		 (sw_r32(rtl_table_data(r, 2)));
	/* Bits L3_INTF and BMSK_L3_INTF are 0 */

out:
	rtl_table_release(r);
}

/*
 * Writes a MAC entry for L3 termination as entry point for routing into the hardware table
 * idx is the index into the L3_ROUTER_MAC table
 */
__maybe_unused
static void otto_l3_930x_set_router_mac(struct otto_l3_ctrl *ctrl,
					u32 idx, struct otto_l3_router_mac *m)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 0);
	u32 v, w;

	/* The table has a size of 7 registers, 64 entries */
	v = BIT(20); /* mac entry valid, port type is 0: individual */
	v |= (m->p_id & 0x3f) << 13;
	v |= (m->vid & 0xfff); /* Set the interface_id to the vlan id */

	w = m->vid_mask;
	w |= (m->p_id_mask & 0x3f) << 13;

	sw_w32(v, rtl_table_data(r, 0));
	sw_w32(w, rtl_table_data(r, 3));

	/* Set MAC address, L3_INTF (bit 12 in register 1) needs to be 0 */
	sw_w32((u32)(m->mac), rtl_table_data(r, 2));
	sw_w32(m->mac >> 32, rtl_table_data(r, 1));

	/* Set MAC address mask, BMSK_L3_INTF (bit 12 in register 5) needs to be 0 */
	sw_w32((u32)(m->mac_mask >> 32), rtl_table_data(r, 4));
	sw_w32((u32)m->mac_mask, rtl_table_data(r, 5));

	sw_w32(m->action & 0x7, rtl_table_data(r, 6));

	dev_dbg(ctrl->dev, "writing index %d: %08x %08x %08x %08x %08x %08x %08x\n", idx,
		 sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		 sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		 sw_r32(rtl_table_data(r, 6))
	);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Destination MAC and L3 egress interface ID of a nexthop entry from the SoC's L3_NEXTHOP table */
__maybe_unused
static void otto_l3_930x_get_nexthop(struct otto_l3_ctrl *ctrl,
				     int idx, u16 *dmac_id, u16 *interface)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 3);
	u32 v;

	rtl_table_read(r, idx);
	/* The table has a size of 1 register */
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);

	*dmac_id = (v >> 7) & 0x7fff;
	*interface = v & 0x7f;
}

/*
 * Set the destination MAC and L3 egress interface ID for a nexthop entry in the SoC's L3_NEXTHOP
 * table. The nexthop entry is identified by idx. dmac_id is the reference to the L2 entry in the
 * L2 forwarding table, special values are
 * 0x7ffe: TRAP2CPU
 * 0x7ffd: TRAP2MASTERCPU
 * 0x7fff: DMAC_ID_DROP
 */
__maybe_unused
static void otto_l3_930x_set_nexthop(struct otto_l3_ctrl *ctrl,
				     int idx, u16 dmac_id, u16 interface)
{
	/* Access L3_NEXTHOP table (3) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 3);

	dev_dbg(ctrl->dev, "Writing to L3_NEXTHOP table, index %d, dmac_id %d, interface %d\n",
		idx, dmac_id, interface);
	sw_w32(((dmac_id & 0x7fff) << 7) | (interface & 0x7f), rtl_table_data(r, 0));

	dev_dbg(ctrl->dev, "value at index 0: %08x\n", sw_r32(rtl_table_data(r, 0)));
	rtl_table_write(r, idx);
	rtl_table_release(r);
}


/* Read a prefix route entry from the L3_PREFIX_ROUTE_IPUC table
 * We currently only support IPv4 and IPv6 unicast route
 */
__maybe_unused
static void otto_l3_930x_route_read(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 2);
	bool host_route, default_route;
	struct in6_addr ip6_m;
	u32 v, ip4_m;

	dev_dbg(ctrl->dev, "%s\n", __func__);

	rtl_table_read(r, idx);
	/* The table has a size of 11 registers */
	rt->attr.valid = !!(sw_r32(rtl_table_data(r, 0)) & BIT(31));
	if (!rt->attr.valid)
		goto out;

	rt->attr.type = (sw_r32(rtl_table_data(r, 0)) >> 29) & 0x3;

	v = sw_r32(rtl_table_data(r, 10));
	host_route = !!(v & BIT(21));
	default_route = !!(v & BIT(20));
	rt->prefix_len = -1;
	dev_dbg(ctrl->dev, "host route %d, default_route %d\n", host_route, default_route);

	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		rt->dst_ip = sw_r32(rtl_table_data(r, 4));
		ip4_m = sw_r32(rtl_table_data(r, 9));
		dev_dbg(ctrl->dev, "Read ip4 mask: %08x\n", ip4_m);
		rt->prefix_len = host_route ? 32 : -1;
		rt->prefix_len = (rt->prefix_len < 0 && default_route) ? 0 : -1;
		if (rt->prefix_len < 0)
			rt->prefix_len = inet_mask_len(ip4_m);
		break;
	case 2: /* IPv6 Unicast route */
		ipv6_addr_set(&rt->dst_ip6,
			      sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
			      sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)));
		ipv6_addr_set(&ip6_m,
			      sw_r32(rtl_table_data(r, 6)), sw_r32(rtl_table_data(r, 7)),
			      sw_r32(rtl_table_data(r, 8)), sw_r32(rtl_table_data(r, 9)));
		rt->prefix_len = host_route ? 128 : 0;
		rt->prefix_len = (rt->prefix_len < 0 && default_route) ? 0 : -1;
		if (rt->prefix_len < 0)
			rt->prefix_len = find_last_bit((unsigned long *)&ip6_m.s6_addr32,
						       128);
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		dev_warn(ctrl->dev, "route type not supported\n");
		goto out;
	}

	rt->attr.hit = !!(v & BIT(22));
	rt->attr.action = (v >> 18) & 3;
	rt->nh.id = (v >> 7) & 0x7ff;
	rt->attr.ttl_dec = !!(v & BIT(6));
	rt->attr.ttl_check = !!(v & BIT(5));
	rt->attr.dst_null = !!(v & BIT(4));
	rt->attr.qos_as = !!(v & BIT(3));
	rt->attr.qos_prio =  v & 0x7;
	dev_dbg(ctrl->dev, "index %d is valid: %d\n", idx, rt->attr.valid);
	dev_dbg(ctrl->dev, "next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	dev_dbg(ctrl->dev, "GW: %pI4, prefix_len: %d\n", &rt->dst_ip, rt->prefix_len);
out:
	rtl_table_release(r);
}

__maybe_unused
static void otto_l3_930x_net6_mask(int prefix_len, struct in6_addr *ip6_m)
{
	int o, b;
	/* Define network mask */
	o = prefix_len >> 3;
	b = prefix_len & 0x7;
	memset(ip6_m->s6_addr, 0xff, o);
	ip6_m->s6_addr[o] |= b ? 0xff00 >> b : 0x00;
}

/*
 * Look up the index of a prefix route in the routing table CAM for unicast IPv4/6 routes
 * using hardware offload.
 */
__maybe_unused
static int otto_l3_930x_route_lookup_hw(struct otto_l3_ctrl *ctrl, struct otto_l3_route *rt)
{
	struct in6_addr ip6_m;
	u32 ip4_m, v;

	if (rt->attr.type == 1 || rt->attr.type == 3) /* Hardware only supports UC routes */
		return -1;

	sw_w32_mask(0x3 << 19, rt->attr.type, RTL930X_L3_HW_LU_KEY_CTRL);
	if (rt->attr.type) { /* IPv6 */
		otto_l3_930x_net6_mask(rt->prefix_len, &ip6_m);
		for (int i = 0; i < 4; i++)
			sw_w32(rt->dst_ip6.s6_addr32[0] & ip6_m.s6_addr32[0],
			       RTL930X_L3_HW_LU_KEY_IP_CTRL + (i << 2));
	} else { /* IPv4 */
		ip4_m = inet_make_mask(rt->prefix_len);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL + 4);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL + 8);
		v = rt->dst_ip & ip4_m;
		dev_dbg(ctrl->dev, "searching for %pI4\n", &v);
		sw_w32(v, RTL930X_L3_HW_LU_KEY_IP_CTRL + 12);
	}

	/* Execute CAM lookup in SoC */
	sw_w32(BIT(15), RTL930X_L3_HW_LU_CTRL);

	/* Wait until execute bit clears and result is ready */
	do {
		v = sw_r32(RTL930X_L3_HW_LU_CTRL);
	} while (v & BIT(15));

	dev_dbg(ctrl->dev, "found: %d, index: %d\n", !!(v & BIT(14)), v & 0x1ff);

	/* Test if search successful (BIT 14 set) */
	if (v & BIT(14))
		return v & 0x1ff;

	return -1;
}

/* Write a prefix route into the routing table CAM at position idx
 * Currently only IPv4 and IPv6 unicast routes are supported
 */
__maybe_unused
static void otto_l3_930x_route_write(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_route *rt)
{
	/* Access L3_PREFIX_ROUTE_IPUC table (2) via register RTL9300_TBL_1 */
	/* The table has a size of 11 registers (20 for MC) */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 2);
	struct in6_addr ip6_m;
	u32 v, ip4_m;

	dev_dbg(ctrl->dev, "%s\n", __func__);
	dev_dbg(ctrl->dev, "index %d is valid: %d\n", idx, rt->attr.valid);
	dev_dbg(ctrl->dev, "nexthop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	dev_dbg(ctrl->dev, "GW: %pI4, prefix_len: %d\n", &rt->dst_ip, rt->prefix_len);

	v = rt->attr.valid ? BIT(31) : 0;
	v |= (rt->attr.type & 0x3) << 29;
	sw_w32(v, rtl_table_data(r, 0));

	v = rt->attr.hit ? BIT(22) : 0;
	v |= (rt->attr.action & 0x3) << 18;
	v |= (rt->nh.id & 0x7ff) << 7;
	v |= rt->attr.ttl_dec ? BIT(6) : 0;
	v |= rt->attr.ttl_check ? BIT(5) : 0;
	v |= rt->attr.dst_null ? BIT(6) : 0;
	v |= rt->attr.qos_as ? BIT(6) : 0;
	v |= rt->attr.qos_prio & 0x7;
	v |= rt->prefix_len == 0 ? BIT(20) : 0; /* set default route bit */

	/* set bit mask for entry type always to 0x3 */
	sw_w32(0x3 << 29, rtl_table_data(r, 5));

	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		sw_w32(0, rtl_table_data(r, 1));
		sw_w32(0, rtl_table_data(r, 2));
		sw_w32(0, rtl_table_data(r, 3));
		sw_w32(rt->dst_ip, rtl_table_data(r, 4));

		v |= rt->prefix_len == 32 ? BIT(21) : 0; /* set host-route bit */
		ip4_m = inet_make_mask(rt->prefix_len);
		sw_w32(0, rtl_table_data(r, 6));
		sw_w32(0, rtl_table_data(r, 7));
		sw_w32(0, rtl_table_data(r, 8));
		sw_w32(ip4_m, rtl_table_data(r, 9));
		break;
	case 2: /* IPv6 Unicast route */
		sw_w32(rt->dst_ip6.s6_addr32[0], rtl_table_data(r, 1));
		sw_w32(rt->dst_ip6.s6_addr32[1], rtl_table_data(r, 2));
		sw_w32(rt->dst_ip6.s6_addr32[2], rtl_table_data(r, 3));
		sw_w32(rt->dst_ip6.s6_addr32[3], rtl_table_data(r, 4));

		v |= rt->prefix_len == 128 ? BIT(21) : 0; /* set host-route bit */

		otto_l3_930x_net6_mask(rt->prefix_len, &ip6_m);

		sw_w32(ip6_m.s6_addr32[0], rtl_table_data(r, 6));
		sw_w32(ip6_m.s6_addr32[1], rtl_table_data(r, 7));
		sw_w32(ip6_m.s6_addr32[2], rtl_table_data(r, 8));
		sw_w32(ip6_m.s6_addr32[3], rtl_table_data(r, 9));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		dev_warn(ctrl->dev, "route type not supported\n");
		rtl_table_release(r);
		return;
	}
	sw_w32(v, rtl_table_data(r, 10));

	dev_dbg(ctrl->dev, "%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n",
		 sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		 sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		 sw_r32(rtl_table_data(r, 6)), sw_r32(rtl_table_data(r, 7)), sw_r32(rtl_table_data(r, 8)),
		 sw_r32(rtl_table_data(r, 9)), sw_r32(rtl_table_data(r, 10)));

	rtl_table_write(r, idx);
	rtl_table_release(r);
}

static int otto_l3_port_lower_walk(struct net_device *lower, struct netdev_nested_priv *_priv)
{
	struct otto_l3_walk_data *data = (struct otto_l3_walk_data *)_priv->data;
	struct rtl838x_switch_priv *priv = data->ctrl->priv;
	struct otto_l3_ctrl *ctrl = data->ctrl;
	int ret = 0;
	int index;

	index = rtl83xx_port_is_under(lower, priv);
	data->port = index;
	if (index >= 0) {
		dev_dbg(ctrl->dev, "Found DSA-port, index %d\n", index);
		ret = 1;
	}

	return ret;
}

static int otto_l3_port_dev_lower_find(struct net_device *dev, struct otto_l3_ctrl *ctrl)
{
	struct otto_l3_walk_data data;
	struct netdev_nested_priv _priv;

	data.ctrl = ctrl;
	data.port = 0;
	_priv.data = (void *)&data;

	netdev_walk_all_lower_dev(dev, otto_l3_port_lower_walk, &_priv);

	return data.port;
}

/* On the RTL93xx, an L3 termination endpoint MAC address on which the router waits
 * for packets to be routed needs to be allocated.
 */
static int otto_l3_alloc_router_mac(struct otto_l3_ctrl *ctrl, u64 mac)
{
	struct otto_l3_router_mac m;
	int free_mac = -1;

	mutex_lock(ctrl->lock);
	for (int i = 0; i < MAX_ROUTER_MACS; i++) {
		ctrl->cfg->get_router_mac(ctrl, i, &m);
		if (free_mac < 0 && !m.valid) {
			free_mac = i;
			continue;
		}
		if (m.valid && m.mac == mac) {
			free_mac = i;
			break;
		}
	}

	if (free_mac < 0) {
		dev_err(ctrl->dev, "No free router MACs, cannot offload\n");
		mutex_unlock(ctrl->lock);
		return -1;
	}

	m.valid = true;
	m.mac = mac;
	m.p_type = 0; /* An individual port, not a trunk port */
	m.p_id = 0x3f;			/* Listen on any port */
	m.p_id_mask = 0;
	m.vid = 0;			/* Listen on any VLAN... */
	m.vid_mask = 0;			/* ... so mask needs to be 0 */
	m.mac_mask = 0xffffffffffffULL;	/* We want an exact match of the interface MAC */
	m.action = L3_FORWARD;		/* Route the packet */
	ctrl->cfg->set_router_mac(ctrl, free_mac, &m);

	mutex_unlock(ctrl->lock);

	return 0;
}

/*
 * Sets up an egress interface for L3 actions Actions for ip4/6_icmp_redirect, ip4/6_pbr_icmp_redirect are:
 * 0: FORWARD, 1: DROP, 2: TRAP2CPU, 3: COPY2CPU, 4: TRAP2MASTERCPU, 5: COPY2MASTERCPU,  6: HARDDROP
 * idx is the index in the HW interface table: idx < 0x80
 */
__maybe_unused
static void otto_l3_930x_set_egress_intf(struct otto_l3_ctrl *ctrl, int idx, struct otto_l3_intf *intf)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 4);
	u32 u, v;

	/* The table has 2 registers */
	u = (intf->vid & 0xfff) << 9;
	u |= (intf->smac_idx & 0x3f) << 3;
	u |= (intf->ip4_mtu_id & 0x7);

	v = (intf->ip6_mtu_id & 0x7) << 28;
	v |= (intf->ttl_scope & 0xff) << 20;
	v |= (intf->hl_scope & 0xff) << 12;
	v |= (intf->ip4_icmp_redirect & 0x7) << 9;
	v |= (intf->ip6_icmp_redirect & 0x7) << 6;
	v |= (intf->ip4_pbr_icmp_redirect & 0x7) << 3;
	v |= (intf->ip6_pbr_icmp_redirect & 0x7);

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));

	dev_dbg(ctrl->dev, "writing to index %d: %08x %08x\n", idx, u, v);
	rtl_table_write(r, idx & 0x7f);
	rtl_table_release(r);
}

/* Configure L3 routing settings of the device:
 * - MTUs
 * - Egress interface
 * - The router's MAC address on which routed packets are expected
 * - MAC addresses used as source macs of routed packets
 */
__maybe_unused
static int otto_l3_930x_setup(struct otto_l3_ctrl *ctrl)
{
	struct rtl838x_switch_priv *priv = ctrl->priv;

	/* Setup MTU with id 0 for default interface */
	for (int i = 0; i < MAX_INTF_MTUS; i++)
		priv->intf_mtu_count[i] = priv->intf_mtus[i] = 0;

	priv->intf_mtu_count[0] = 0; /* Needs to stay forever */
	priv->intf_mtus[0] = DEFAULT_MTU;
	sw_w32_mask(0xffff, DEFAULT_MTU, RTL930X_L3_IP_MTU_CTRL(0));
	sw_w32_mask(0xffff, DEFAULT_MTU, RTL930X_L3_IP6_MTU_CTRL(0));
	priv->intf_mtus[1] = DEFAULT_MTU;
	sw_w32_mask(0xffff0000, DEFAULT_MTU << 16, RTL930X_L3_IP_MTU_CTRL(0));
	sw_w32_mask(0xffff0000, DEFAULT_MTU << 16, RTL930X_L3_IP6_MTU_CTRL(0));

	sw_w32_mask(0xffff, DEFAULT_MTU, RTL930X_L3_IP_MTU_CTRL(1));
	sw_w32_mask(0xffff, DEFAULT_MTU, RTL930X_L3_IP6_MTU_CTRL(1));
	sw_w32_mask(0xffff0000, DEFAULT_MTU << 16, RTL930X_L3_IP_MTU_CTRL(1));
	sw_w32_mask(0xffff0000, DEFAULT_MTU << 16, RTL930X_L3_IP6_MTU_CTRL(1));

	/* Clear all source port MACs */
	for (int i = 0; i < MAX_SMACS; i++)
		otto_l3_930x_set_egress_mac(ctrl, L3_EGRESS_DMACS + i, 0ULL);

	/* Configure the default L3 hash algorithm */
	sw_w32_mask(BIT(2), 0, RTL930X_L3_HOST_TBL_CTRL);  /* Algorithm selection 0 = 0 */
	sw_w32_mask(0, BIT(3), RTL930X_L3_HOST_TBL_CTRL);  /* Algorithm selection 1 = 1 */

	pr_debug("L3_IPUC_ROUTE_CTRL %08x, IPMC_ROUTE %08x, IP6UC_ROUTE %08x, IP6MC_ROUTE %08x\n",
		 sw_r32(RTL930X_L3_IPUC_ROUTE_CTRL), sw_r32(RTL930X_L3_IPMC_ROUTE_CTRL),
		 sw_r32(RTL930X_L3_IP6UC_ROUTE_CTRL), sw_r32(RTL930X_L3_IP6MC_ROUTE_CTRL));
	sw_w32_mask(0, 1, RTL930X_L3_IPUC_ROUTE_CTRL);
	sw_w32_mask(0, 1, RTL930X_L3_IP6UC_ROUTE_CTRL);
	sw_w32_mask(0, 1, RTL930X_L3_IPMC_ROUTE_CTRL);
	sw_w32_mask(0, 1, RTL930X_L3_IP6MC_ROUTE_CTRL);

	sw_w32(0x00002001, RTL930X_L3_IPUC_ROUTE_CTRL);
	sw_w32(0x00014581, RTL930X_L3_IP6UC_ROUTE_CTRL);
	sw_w32(0x00000501, RTL930X_L3_IPMC_ROUTE_CTRL);
	sw_w32(0x00012881, RTL930X_L3_IP6MC_ROUTE_CTRL);

	pr_debug("L3_IPUC_ROUTE_CTRL %08x, IPMC_ROUTE %08x, IP6UC_ROUTE %08x, IP6MC_ROUTE %08x\n",
		 sw_r32(RTL930X_L3_IPUC_ROUTE_CTRL), sw_r32(RTL930X_L3_IPMC_ROUTE_CTRL),
		 sw_r32(RTL930X_L3_IP6UC_ROUTE_CTRL), sw_r32(RTL930X_L3_IP6MC_ROUTE_CTRL));

	/* Trap non-ip traffic to the CPU-port (e.g. ARP so we stay reachable) */
	sw_w32_mask(0x3 << 8, 0x1 << 8, RTL930X_L3_IP_ROUTE_CTRL);
	pr_debug("L3_IP_ROUTE_CTRL %08x\n", sw_r32(RTL930X_L3_IP_ROUTE_CTRL));

	/* PORT_ISO_RESTRICT_ROUTE_CTRL? */

	/* Do not use prefix route 0 because of HW limitations */
	set_bit(0, ctrl->route_use_bm);

	return 0;
}

static int otto_l3_alloc_egress_intf(struct otto_l3_ctrl *ctrl, u64 mac, int vlan)
{
	struct otto_l3_intf intf;
	int free_mac = -1;
	u64 m;

	mutex_lock(ctrl->lock);
	for (int i = 0; i < MAX_SMACS; i++) {
		m = ctrl->cfg->get_egress_mac(ctrl, L3_EGRESS_DMACS + i);
		if (free_mac < 0 && !m) {
			free_mac = i;
			continue;
		}
		if (m == mac) {
			mutex_unlock(ctrl->lock);
			return i;
		}
	}

	if (free_mac < 0) {
		dev_err(ctrl->dev, "No free egress interface, cannot offload\n");
		return -1;
	}

	/* Set up default egress interface 1 */
	intf.vid = vlan;
	intf.smac_idx = free_mac;
	intf.ip4_mtu_id = 1;
	intf.ip6_mtu_id = 1;
	intf.ttl_scope = 1; /* TTL */
	intf.hl_scope = 1;  /* Hop Limit */
	intf.ip4_icmp_redirect = intf.ip6_icmp_redirect = 2;  /* FORWARD */
	intf.ip4_pbr_icmp_redirect = intf.ip6_pbr_icmp_redirect = 2; /* FORWARD; */
	ctrl->cfg->set_egress_intf(ctrl, free_mac, &intf);

	ctrl->cfg->set_egress_mac(ctrl, L3_EGRESS_DMACS + free_mac, mac);

	mutex_unlock(ctrl->lock);

	return free_mac;
}

/* Updates an L3 next hop entry in the ROUTING table */
static int otto_l3_nexthop_update(struct otto_l3_ctrl *ctrl, __be32 ip_addr, u64 mac)
{
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct rhlist_head *tmp, *list;
	struct otto_l3_route *r;

	rcu_read_lock();
	list = rhltable_lookup(&ctrl->routes, &ip_addr, otto_l3_route_ht_params);
	if (!list) {
		rcu_read_unlock();
		return -ENOENT;
	}

	rhl_for_each_entry_rcu(r, tmp, list, linkage) {
		dev_dbg(ctrl->dev, "%s: Setting up fwding: ip %pI4, GW mac %016llx\n",
			__func__, &ip_addr, mac);

		/* Reads the ROUTING table entry associated with the route */
		ctrl->cfg->route_read(ctrl, r->id, r);
		dev_dbg(ctrl->dev, "Route with id %d to %pI4 / %d\n",
			r->id, &r->dst_ip, r->prefix_len);

		r->nh.mac = r->nh.gw = mac;
		r->nh.port = priv->r->port_ignore;
		r->nh.id = r->id;

		/* Do we need to explicitly add a DMAC entry with the route's nh index? */
		if (ctrl->cfg->set_egress_mac)
			ctrl->cfg->set_egress_mac(ctrl, r->id, mac);

		/* Update ROUTING table: map gateway-mac and switch-mac id to route id */
		rtl83xx_l2_nexthop_add(priv, &r->nh);

		r->attr.valid = true;
		r->attr.action = ROUTE_ACT_FORWARD;
		r->attr.type = 0;
		r->attr.hit = false; /* Reset route-used indicator */

		/* Add PIE entry with dst_ip and prefix_len */
		r->pr.dip = r->dst_ip;
		r->pr.dip_m = inet_make_mask(r->prefix_len);

		if (r->is_host_route) {
			int slot = ctrl->cfg->find_slot(ctrl, r, false);

			dev_info(ctrl->dev, "Got slot for route: %d\n", slot);
			ctrl->cfg->host_route_write(ctrl, slot, r);
		} else {
			ctrl->cfg->route_write(ctrl, r->id, r);
			r->pr.fwd_sel = true;
			r->pr.fwd_data = r->nh.l2_id;
			r->pr.fwd_act = PIE_ACT_ROUTE_UC;
		}

		if (ctrl->cfg->set_nexthop)
			ctrl->cfg->set_nexthop(ctrl, r->nh.id, r->nh.l2_id, r->nh.if_id);

		if (r->pr.id < 0) {
			r->pr.packet_cntr = rtl83xx_packet_cntr_alloc(priv);
			if (r->pr.packet_cntr >= 0) {
				dev_info(ctrl->dev, "Using packet counter %d\n",
					 r->pr.packet_cntr);
				r->pr.log_sel = true;
				r->pr.log_data = r->pr.packet_cntr;
			}
			priv->r->pie_rule_add(priv, &r->pr);
		} else {
			int pkts = priv->r->packet_cntr_read(r->pr.packet_cntr);

			dev_dbg(ctrl->dev, "total packets: %d\n", pkts);

			priv->r->pie_rule_write(priv, r->pr.id, &r->pr);
		}
	}
	rcu_read_unlock();

	return 0;
}

static int otto_l3_port_ipv4_resolve(struct otto_l3_ctrl *ctrl,
				     struct net_device *dev, __be32 ip_addr)
{
	struct neighbour *n = neigh_lookup(&arp_tbl, &ip_addr, dev);
	int err = 0;
	u64 mac;

	if (!n) {
		n = neigh_create(&arp_tbl, &ip_addr, dev);
		if (IS_ERR(n))
			return PTR_ERR(n);
	}

	/* If the neigh is already resolved, then go ahead and
	 * install the entry, otherwise start the ARP process to
	 * resolve the neigh.
	 */
	if (n->nud_state & NUD_VALID) {
		mac = ether_addr_to_u64(n->ha);
		dev_info(ctrl->dev, "resolved mac: %016llx\n", mac);
		otto_l3_nexthop_update(ctrl, ip_addr, mac);
	} else {
		dev_info(ctrl->dev, "need to wait\n");
		neigh_event_send(n, NULL);
	}

	neigh_release(n);

	return err;
}

static void otto_l3_route_remove(struct otto_l3_ctrl *ctrl, struct otto_l3_route *r)
{
	int id;

	if (rhltable_remove(&ctrl->routes, &r->linkage, otto_l3_route_ht_params))
		dev_warn(ctrl->dev, "Could not remove route\n");

	if (r->is_host_route) {
		id = ctrl->cfg->find_slot(ctrl, r, false);
		dev_dbg(ctrl->dev, "Got id for host route: %d\n", id);
		r->attr.valid = false;
		ctrl->cfg->host_route_write(ctrl, id, r);
		clear_bit(r->id - MAX_ROUTES, ctrl->host_route_use_bm);
	} else {
		/* If there is a HW representation of the route, delete it */
		if (ctrl->cfg->route_lookup_hw) {
			id = ctrl->cfg->route_lookup_hw(ctrl, r);
			dev_info(ctrl->dev, "Got id for prefix route: %d\n", id);
			r->attr.valid = false;
			ctrl->cfg->route_write(ctrl, id, r);
		}
		clear_bit(r->id, ctrl->route_use_bm);
	}

	kfree(r);
}

static struct otto_l3_route *otto_l3_host_route_alloc(struct otto_l3_ctrl *ctrl, u32 ip)
{
	struct otto_l3_route *r;
	int idx = 0, err;

	mutex_lock(ctrl->lock);

	idx = find_first_zero_bit(ctrl->host_route_use_bm, MAX_HOST_ROUTES);
	dev_dbg(ctrl->dev, "id: %d, ip %pI4\n", idx, &ip);

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (!r) {
		mutex_unlock(ctrl->lock);
		return r;
	}

	/* We require a unique route ID irrespective of whether it is a prefix or host
	 * route (on RTL93xx) as we use this ID to associate a DMAC and next-hop entry
	 */
	r->id = idx + MAX_ROUTES;

	r->gw_ip = ip;
	r->pr.id = -1; /* We still need to allocate a rule in HW */
	r->is_host_route = true;

	err = rhltable_insert(&ctrl->routes, &r->linkage, otto_l3_route_ht_params);
	if (err) {
		dev_err(ctrl->dev, "Could not insert new rule\n");
		mutex_unlock(ctrl->lock);
		goto out_free;
	}

	set_bit(idx, ctrl->host_route_use_bm);

	mutex_unlock(ctrl->lock);

	return r;

out_free:
	kfree(r);

	return NULL;
}

static struct otto_l3_route *otto_l3_route_alloc(struct otto_l3_ctrl *ctrl, u32 ip)
{
	struct otto_l3_route *r;
	int idx = 0, err;

	mutex_lock(ctrl->lock);

	idx = find_first_zero_bit(ctrl->route_use_bm, MAX_ROUTES);
	dev_dbg(ctrl->dev, "id: %d, ip %pI4\n", idx, &ip);

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (!r) {
		mutex_unlock(ctrl->lock);
		return r;
	}

	r->id = idx;
	r->gw_ip = ip;
	r->pr.id = -1; /* We still need to allocate a rule in HW */
	r->is_host_route = false;

	err = rhltable_insert(&ctrl->routes, &r->linkage, otto_l3_route_ht_params);
	if (err) {
		dev_err(ctrl->dev, "Could not insert new rule\n");
		mutex_unlock(ctrl->lock);
		goto out_free;
	}

	set_bit(idx, ctrl->route_use_bm);

	mutex_unlock(ctrl->lock);

	return r;

out_free:
	kfree(r);

	return NULL;
}

static int otto_l3_fib_check_v4(struct otto_l3_ctrl *ctrl,
				struct fib_entry_notifier_info *info,
				enum fib_event_type event)
{
	struct net_device *ndev = fib_info_nh(info->fi, 0)->fib_nh_dev;
	int vlan = is_vlan_dev(ndev) ? vlan_dev_vlan_id(ndev) : 0;
	struct fib_nh *nh = fib_info_nh(info->fi, 0);
	char gw_message[32] = "";

	if (nh->fib_nh_gw4)
		snprintf(gw_message, sizeof(gw_message), "via %pI4 ", &nh->fib_nh_gw4);

	dev_info(ctrl->dev, "%s IPv4 route %pI4/%d %s(VLAN %d, MAC %pM)\n",
		 event == FIB_EVENT_ENTRY_ADD ? "add" : "delete",
		 &info->dst, info->dst_len, gw_message, vlan, ndev->dev_addr);

	if ((info->type == RTN_BROADCAST) || ipv4_is_loopback(info->dst) || !info->dst) {
		dev_warn(ctrl->dev, "skip loopback/broadcast addresses and default routes\n");
		return -EINVAL;
	}

	return 0;
}

static int otto_l3_fib_add_v4(struct otto_l3_ctrl *ctrl, struct fib_entry_notifier_info *info)
{
	struct net_device *ndev = fib_info_nh(info->fi, 0)->fib_nh_dev;
	int vlan = is_vlan_dev(ndev) ? vlan_dev_vlan_id(ndev) : 0;
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct fib_nh *nh = fib_info_nh(info->fi, 0);
	struct otto_l3_route *route;
	int port;

	if (otto_l3_fib_check_v4(ctrl, info, FIB_EVENT_ENTRY_ADD))
		return 0;

	port = otto_l3_port_dev_lower_find(ndev, ctrl);
	if (port < 0) {
		dev_err(ctrl->dev, "lower interface %s not found\n", ndev->name);
		return -ENODEV;
	}

	/* Allocate route or host-route entry (if hardware supports this) */
	if (info->dst_len == 32 && ctrl->cfg->host_route_write)
		route = otto_l3_host_route_alloc(ctrl, nh->fib_nh_gw4);
	else
		route = otto_l3_route_alloc(ctrl, nh->fib_nh_gw4);

	if (route)
		dev_info(ctrl->dev, "route hashtable extended for gw %pI4\n", &nh->fib_nh_gw4);
	else {
		dev_err(ctrl->dev, "could not extend route hashtable for gw %pI4\n",
			&nh->fib_nh_gw4);
		return -ENOSPC;
	}

	route->dst_ip = info->dst;
	route->prefix_len = info->dst_len;
	route->nh.rvid = vlan;

	if (ctrl->cfg->set_router_mac) {
		u64 mac = ether_addr_to_u64(ndev->dev_addr);

		dev_dbg(ctrl->dev, "Local route and router MAC %pM\n", ndev->dev_addr);
		if (otto_l3_alloc_router_mac(ctrl, mac))
			goto out_free_rt;

		/* vid = 0: Do not care about VID */
		route->nh.if_id = otto_l3_alloc_egress_intf(ctrl, mac, vlan);
		if (route->nh.if_id < 0)
			goto out_free_rmac;

		if (!nh->fib_nh_gw4) {
			int slot;

			route->nh.mac = mac;
			route->nh.port = priv->r->port_ignore;
			route->attr.valid = true;
			route->attr.action = ROUTE_ACT_TRAP2CPU;
			route->attr.type = 0;

			slot = ctrl->cfg->find_slot(ctrl, route, false);
			dev_dbg(ctrl->dev, "Got slot for route: %d\n", slot);
			ctrl->cfg->host_route_write(ctrl, slot, route);
		}
	}

	/* We need to resolve the mac address of the GW */
	if (nh->fib_nh_gw4)
		otto_l3_port_ipv4_resolve(ctrl, ndev, nh->fib_nh_gw4);

	nh->fib_nh_flags |= RTNH_F_OFFLOAD;

	return 0;

out_free_rmac:
out_free_rt:
	return 0;
}

static int otto_l3_fib_del_v4(struct otto_l3_ctrl *ctrl, struct fib_entry_notifier_info *info)
{
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct fib_nh *nh = fib_info_nh(info->fi, 0);
	struct rhlist_head *tmp, *list;
	struct otto_l3_route *route;

	if (otto_l3_fib_check_v4(ctrl, info, FIB_EVENT_ENTRY_DEL))
		return 0;

	rcu_read_lock();
	list = rhltable_lookup(&ctrl->routes, &nh->fib_nh_gw4, otto_l3_route_ht_params);
	if (!list) {
		rcu_read_unlock();
		dev_err(ctrl->dev, "no such gateway: %pI4\n", &nh->fib_nh_gw4);
		return -ENOENT;
	}
	rhl_for_each_entry_rcu(route, tmp, list, linkage) {
		if (route->dst_ip == info->dst && route->prefix_len == info->dst_len) {
			dev_info(ctrl->dev, "found a route with id %d, nh-id %d\n",
				 route->id, route->nh.id);
			break;
		}
	}
	rcu_read_unlock();

	rtl83xx_l2_nexthop_rm(priv, &route->nh);

	dev_info(ctrl->dev, "releasing packet counter %d\n", route->pr.packet_cntr);
	set_bit(route->pr.packet_cntr, priv->packet_cntr_use_bm);
	priv->r->pie_rule_rm(priv, &route->pr);

	otto_l3_route_remove(ctrl, route);

	nh->fib_nh_flags &= ~RTNH_F_OFFLOAD;

	return 0;
}

static int otto_l3_fib_add_v6(struct otto_l3_ctrl *ctrl, struct fib6_entry_notifier_info *info)
{
	dev_dbg(ctrl->dev, "In %s\n", __func__);
/*	nh->fib_nh_flags |= RTNH_F_OFFLOAD; */

	return 0;
}

static void otto_l3_fib_event_work_do(struct work_struct *work)
{
	struct otto_l3_fib_event_work *fib_work =
		container_of(work, struct otto_l3_fib_event_work, work);
	struct otto_l3_ctrl *ctrl = fib_work->ctrl;
	struct fib_rule *rule;
	int err;

	/* Protect internal structures from changes */
	rtnl_lock();
	dev_dbg(ctrl->dev, "doing work, event %ld\n", fib_work->event);
	switch (fib_work->event) {
	case FIB_EVENT_ENTRY_ADD:
	case FIB_EVENT_ENTRY_REPLACE:
	case FIB_EVENT_ENTRY_APPEND:
		if (fib_work->is_fib6)
			err = otto_l3_fib_add_v6(ctrl, &fib_work->fen6_info);
		else
			err = otto_l3_fib_add_v4(ctrl, &fib_work->fen_info);
		if (err)
			dev_err(ctrl->dev, "fib_add() failed\n");

		fib_info_put(fib_work->fen_info.fi);
		break;
	case FIB_EVENT_ENTRY_DEL:
		err = otto_l3_fib_del_v4(ctrl, &fib_work->fen_info);
		if (err)
			dev_err(ctrl->dev, "fib_del() failed\n");

		fib_info_put(fib_work->fen_info.fi);
		break;
	case FIB_EVENT_RULE_ADD:
	case FIB_EVENT_RULE_DEL:
		rule = fib_work->fr_info.rule;
		if (!fib4_rule_default(rule))
			dev_err(ctrl->dev, "FIB4 default rule failed\n");
		fib_rule_put(rule);
		break;
	}
	rtnl_unlock();
	kfree(fib_work);
}


/* Called with rcu_read_lock() */
static int otto_l3_fib_notifier(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct otto_l3_ctrl *ctrl = container_of(this, struct otto_l3_ctrl, fib_nb);
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct otto_l3_fib_event_work *fib_work;
	struct fib_notifier_info *info = ptr;

	if ((info->family != AF_INET && info->family != AF_INET6 &&
	     info->family != RTNL_FAMILY_IPMR &&
	     info->family != RTNL_FAMILY_IP6MR))
		return NOTIFY_DONE;

	/* ignore FIB events for HW with missing L3 offloading implementation */
	if (!ctrl->cfg->setup)
		return NOTIFY_DONE;

	fib_work = kzalloc(sizeof(*fib_work), GFP_ATOMIC);
	if (!fib_work)
		return NOTIFY_BAD;

	INIT_WORK(&fib_work->work, otto_l3_fib_event_work_do);
	fib_work->ctrl = ctrl;
	fib_work->event = event;
	fib_work->is_fib6 = false;

	switch (event) {
	case FIB_EVENT_ENTRY_ADD:
	case FIB_EVENT_ENTRY_REPLACE:
	case FIB_EVENT_ENTRY_APPEND:
	case FIB_EVENT_ENTRY_DEL:
		dev_dbg(ctrl->dev, "FIB_ENTRY ADD/DEL, event %ld\n", event);
		if (info->family == AF_INET) {
			struct fib_entry_notifier_info *fen_info = ptr;

			if (fen_info->fi->fib_nh_is_v6) {
				NL_SET_ERR_MSG_MOD(info->extack,
						   "IPv6 gateway with IPv4 route is not supported");
				kfree(fib_work);
				return notifier_from_errno(-EINVAL);
			}

			memcpy(&fib_work->fen_info, ptr, sizeof(fib_work->fen_info));
			/* Take referece on fib_info to prevent it from being
			 * freed while work is queued. Release it afterwards.
			 */
			fib_info_hold(fib_work->fen_info.fi);

		} else if (info->family == AF_INET6) {
			//struct fib6_entry_notifier_info *fen6_info = ptr;
			dev_warn(ctrl->dev, "FIB_RULE ADD/DEL for IPv6 not supported\n");
			kfree(fib_work);
			return NOTIFY_DONE;
		}
		break;

	case FIB_EVENT_RULE_ADD:
	case FIB_EVENT_RULE_DEL:
		dev_dbg(ctrl->dev, "FIB_RULE ADD/DEL, event: %ld\n", event);
		memcpy(&fib_work->fr_info, ptr, sizeof(fib_work->fr_info));
		fib_rule_get(fib_work->fr_info.rule);
		break;
	}

	queue_work(priv->wq, &fib_work->work);

	return NOTIFY_DONE;
}

static void otto_l3_net_event_work_do(struct work_struct *work)
{
	struct otto_l3_net_event_work *net_work =
		container_of(work, struct otto_l3_net_event_work, work);

	otto_l3_nexthop_update(net_work->ctrl, net_work->gw_addr, net_work->mac);

	kfree(net_work);
}

static int otto_l3_netevent_notifier(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct otto_l3_ctrl *ctrl = container_of(this, struct otto_l3_ctrl, ne_nb);
	struct rtl838x_switch_priv *priv = ctrl->priv;
	struct otto_l3_net_event_work *net_work;
	struct neighbour *n = ptr;
	struct net_device *dev;
	int err, port;

	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
		/* ignore events for HW with missing L3 offloading implementation */
		if (!ctrl->cfg->setup)
			return NOTIFY_DONE;

		if (n->tbl != &arp_tbl)
			return NOTIFY_DONE;
		dev = n->dev;
		port = otto_l3_port_dev_lower_find(dev, ctrl);
		if (port < 0 || !(n->nud_state & NUD_VALID)) {
			dev_dbg(ctrl->dev, "Neigbour invalid, not updating\n");
			return NOTIFY_DONE;
		}

		net_work = kzalloc(sizeof(*net_work), GFP_ATOMIC);
		if (!net_work)
			return NOTIFY_BAD;

		INIT_WORK(&net_work->work, otto_l3_net_event_work_do);
		net_work->ctrl = ctrl;

		net_work->mac = ether_addr_to_u64(n->ha);
		net_work->gw_addr = *(__be32 *)n->primary_key;

		dev_dbg(ctrl->dev, "updating neighbour on port %d, mac %016llx\n",
			port, net_work->mac);
		queue_work(priv->wq, &net_work->work);
		if (err)
			netdev_warn(dev, "failed to handle neigh update (err %d)\n", err);
		break;
	}

	return NOTIFY_DONE;
}

const struct otto_l3_config otto_l3_838x_cfg = {
	.route_read = otto_l3_838x_route_read,
	.route_write = otto_l3_838x_route_write,
};

const struct otto_l3_config otto_l3_839x_cfg = {
	.route_read = otto_l3_839x_route_read,
	.route_write = otto_l3_839x_route_write,
	.setup = otto_l3_839x_setup,
};

const struct otto_l3_config otto_l3_930x_cfg = {
#ifdef CONFIG_NET_DSA_RTL83XX_RTL930X_L3_OFFLOAD
	.find_slot = otto_l3_930x_find_slot,
	.get_egress_mac = otto_l3_930x_get_egress_mac,
	.set_egress_mac = otto_l3_930x_set_egress_mac,
	.set_egress_intf = otto_l3_930x_set_egress_intf,
	.host_route_write = otto_l3_930x_host_route_write,
	.get_router_mac = otto_l3_930x_get_router_mac,
	.set_router_mac = otto_l3_930x_set_router_mac,
	.get_nexthop = otto_l3_930x_get_nexthop,
	.set_nexthop = otto_l3_930x_set_nexthop,
	.route_lookup_hw = otto_l3_930x_route_lookup_hw,
	.route_read = otto_l3_930x_route_read,
	.route_write = otto_l3_930x_route_write,
	.setup = otto_l3_930x_setup,
#endif
};

const struct otto_l3_config otto_l3_931x_cfg = {
};

static const struct of_device_id otto_l3_of_ids[] = {
	{ .compatible = "realtek,rtl8380-switch", .data = &otto_l3_838x_cfg, },
	{ .compatible = "realtek,rtl8392-switch", .data = &otto_l3_839x_cfg, },
	{ .compatible = "realtek,rtl9301-switch", .data = &otto_l3_930x_cfg, },
	{ .compatible = "realtek,rtl9311-switch", .data = &otto_l3_931x_cfg, },
	{ /* sentinel */ }
};

void otto_l3_remove(struct rtl838x_switch_priv *priv)
{
	struct otto_l3_ctrl *ctrl = priv->l3_ctrl;

	if (ctrl->ne_nb.notifier_call) {
		unregister_netevent_notifier(&ctrl->ne_nb);
		ctrl->ne_nb.notifier_call = NULL;
	}
	if (ctrl->fib_nb.notifier_call) {
		unregister_fib_notifier(&init_net, &ctrl->fib_nb);
		ctrl->fib_nb.notifier_call = NULL;
	}
}

int otto_l3_probe(struct device *dev, struct rtl838x_switch_priv *priv)
{
	const struct of_device_id *match;
	struct otto_l3_ctrl *ctrl;
	int err;

	ctrl = devm_kzalloc(dev, sizeof(struct otto_l3_ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;
	priv->l3_ctrl = ctrl;
	ctrl->priv = priv;
	ctrl->dev = priv->dev;
	/* For now share the register access lock with the DSA driver */
	ctrl->lock = &priv->reg_mutex;

	match = of_match_node(otto_l3_of_ids, dev->of_node);
	if (!match)
		return dev_err_probe(dev, -EINVAL, "No compatible configuration found\n");
	ctrl->cfg = match->data;

	if (ctrl->cfg->setup) {
		err = ctrl->cfg->setup(ctrl);
		if (err)
			return dev_err_probe(dev, err, "device specific L3 setup failed\n");
	}

	/* Initialize hash table for L3 routing */
	rhltable_init(&ctrl->routes, &otto_l3_route_ht_params);

	/*
	 * Register netevent notifier callback to catch notifications about neighboring changes
	 * to update nexthop entries for L3 routing.
	 */
	ctrl->ne_nb.notifier_call = otto_l3_netevent_notifier;
	err = register_netevent_notifier(&ctrl->ne_nb);
	if (err) {
		ctrl->ne_nb.notifier_call = NULL;
		return dev_err_probe(dev, err, "Failed to register netevent notifier\n");
	}

	/*
	 * Register Forwarding Information Base notifier to offload routes where possible. Only
	 * FIBs pointing to our own netdevs are programmed into the device, so no need to pass a
	 * callback.
	 */
	ctrl->fib_nb.notifier_call = otto_l3_fib_notifier;
	err = register_fib_notifier(&init_net, &ctrl->fib_nb, NULL, NULL);
	if (err) {
		ctrl->fib_nb.notifier_call = NULL;
		otto_l3_remove(priv);
		return dev_err_probe(dev, err, "Failed to register fib event notifier\n");
	}

	return 0;
}
