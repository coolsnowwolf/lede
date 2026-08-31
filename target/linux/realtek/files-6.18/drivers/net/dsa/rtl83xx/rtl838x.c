// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl-otto/mach-rtl-otto.h>
#include <linux/etherdevice.h>
#include <linux/iopoll.h>
#include <net/nexthop.h>

#include "l3.h"
#include "rtl-otto.h"

#define RTL838X_VLAN_PORT_TAG_STS_UNTAG				0x0
#define RTL838X_VLAN_PORT_TAG_STS_TAGGED			0x1
#define RTL838X_VLAN_PORT_TAG_STS_PRIORITY_TAGGED		0x2

#define RTL838X_VLAN_PORT_TAG_STS_CTRL_BASE			0xA530
/* port 0-28 */
#define RTL838X_VLAN_PORT_TAG_STS_CTRL(port) \
	(RTL838X_VLAN_PORT_TAG_STS_CTRL_BASE + (port << 2))

#define RTL838X_VLAN_PORT_TAG_STS_CTRL_EGR_P_OTAG_KEEP_MASK	GENMASK(11, 10)
#define RTL838X_VLAN_PORT_TAG_STS_CTRL_EGR_P_ITAG_KEEP_MASK	GENMASK(9, 8)
#define RTL838X_VLAN_PORT_TAG_STS_CTRL_IGR_P_OTAG_KEEP_MASK	GENMASK(7, 6)
#define RTL838X_VLAN_PORT_TAG_STS_CTRL_IGR_P_ITAG_KEEP_MASK	GENMASK(5, 4)
#define RTL838X_VLAN_PORT_TAG_STS_CTRL_OTAG_STS_MASK		GENMASK(3, 2)
#define RTL838X_VLAN_PORT_TAG_STS_CTRL_ITAG_STS_MASK		GENMASK(1, 0)

/* see_dal_maple_acl_log2PhyTmplteField and src/app/diag_v2/src/diag_acl.c */
/* Definition of the RTL838X-specific template field IDs as used in the PIE */
enum template_field_id {
	TEMPLATE_FIELD_SPMMASK = 0,
	TEMPLATE_FIELD_SPM0 = 1,	/* Source portmask ports 0-15 */
	TEMPLATE_FIELD_SPM1 = 2,	/* Source portmask ports 16-28 */
	TEMPLATE_FIELD_RANGE_CHK = 3,
	TEMPLATE_FIELD_DMAC0 = 4,	/* Destination MAC [15:0] */
	TEMPLATE_FIELD_DMAC1 = 5,	/* Destination MAC [31:16] */
	TEMPLATE_FIELD_DMAC2 = 6,	/* Destination MAC [47:32] */
	TEMPLATE_FIELD_SMAC0 = 7,	/* Source MAC [15:0] */
	TEMPLATE_FIELD_SMAC1 = 8,	/* Source MAC [31:16] */
	TEMPLATE_FIELD_SMAC2 = 9,	/* Source MAC [47:32] */
	TEMPLATE_FIELD_ETHERTYPE = 10,	/* Ethernet typ */
	TEMPLATE_FIELD_OTAG = 11,	/* Outer VLAN tag */
	TEMPLATE_FIELD_ITAG = 12,	/* Inner VLAN tag */
	TEMPLATE_FIELD_SIP0 = 13,	/* IPv4 or IPv6 source IP[15:0] or ARP/RARP */
					/* source protocol address in header */
	TEMPLATE_FIELD_SIP1 = 14,	/* IPv4 or IPv6 source IP[31:16] or ARP/RARP */
	TEMPLATE_FIELD_DIP0 = 15,	/* IPv4 or IPv6 destination IP[15:0] */
	TEMPLATE_FIELD_DIP1 = 16,	/* IPv4 or IPv6 destination IP[31:16] */
	TEMPLATE_FIELD_IP_TOS_PROTO = 17, /* IPv4 TOS/IPv6 traffic class and */
					  /* IPv4 proto/IPv6 next header fields */
	TEMPLATE_FIELD_L34_HEADER = 18,	/* packet with extra tag and IPv6 with auth, dest, */
					/* frag, route, hop-by-hop option header, */
					/* IGMP type, TCP flag */
	TEMPLATE_FIELD_L4_SPORT = 19,	/* TCP/UDP source port */
	TEMPLATE_FIELD_L4_DPORT = 20,	/* TCP/UDP destination port */
	TEMPLATE_FIELD_ICMP_IGMP = 21,
	TEMPLATE_FIELD_IP_RANGE = 22,
	TEMPLATE_FIELD_FIELD_SELECTOR_VALID = 23, /* Field selector mask */
	TEMPLATE_FIELD_FIELD_SELECTOR_0 = 24,
	TEMPLATE_FIELD_FIELD_SELECTOR_1 = 25,
	TEMPLATE_FIELD_FIELD_SELECTOR_2 = 26,
	TEMPLATE_FIELD_FIELD_SELECTOR_3 = 27,
	TEMPLATE_FIELD_SIP2 = 28,	/* IPv6 source IP[47:32] */
	TEMPLATE_FIELD_SIP3 = 29,	/* IPv6 source IP[63:48] */
	TEMPLATE_FIELD_SIP4 = 30,	/* IPv6 source IP[79:64] */
	TEMPLATE_FIELD_SIP5 = 31,	/* IPv6 source IP[95:80] */
	TEMPLATE_FIELD_SIP6 = 32,	/* IPv6 source IP[111:96] */
	TEMPLATE_FIELD_SIP7 = 33,	/* IPv6 source IP[127:112] */
	TEMPLATE_FIELD_DIP2 = 34,	/* IPv6 destination IP[47:32] */
	TEMPLATE_FIELD_DIP3 = 35,	/* IPv6 destination IP[63:48] */
	TEMPLATE_FIELD_DIP4 = 36,	/* IPv6 destination IP[79:64] */
	TEMPLATE_FIELD_DIP5 = 37,	/* IPv6 destination IP[95:80] */
	TEMPLATE_FIELD_DIP6 = 38,	/* IPv6 destination IP[111:96] */
	TEMPLATE_FIELD_DIP7 = 39,	/* IPv6 destination IP[127:112] */
	TEMPLATE_FIELD_FWD_VID = 40,	/* Forwarding VLAN-ID */
	TEMPLATE_FIELD_FLOW_LABEL = 41,
};

/* The RTL838X SoCs use 5 fixed templates with definitions for which data fields are to
 * be copied from the Ethernet Frame header into the 12 User-definable fields of the Packet
 * Inspection Engine's buffer. The following defines the field contents for each of the fixed
 * templates. Additionally, 3 user-definable templates can be set up via the definitions
 * in RTL838X_ACL_TMPLTE_CTRL control registers.
 * TODO: See all src/app/diag_v2/src/diag_pie.c
 */
#define N_FIXED_TEMPLATES 5
static enum template_field_id fixed_templates[N_FIXED_TEMPLATES][N_FIXED_FIELDS] = {
	{
	  TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1, TEMPLATE_FIELD_OTAG,
	  TEMPLATE_FIELD_SMAC0, TEMPLATE_FIELD_SMAC1, TEMPLATE_FIELD_SMAC2,
	  TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
	  TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_ITAG, TEMPLATE_FIELD_RANGE_CHK
	}, {
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
	  TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_L4_SPORT,
	  TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_ICMP_IGMP, TEMPLATE_FIELD_ITAG,
	  TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1
	}, {
	  TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
	  TEMPLATE_FIELD_ITAG, TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_IP_TOS_PROTO,
	  TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_SIP0,
	  TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0, TEMPLATE_FIELD_DIP1
	}, {
	  TEMPLATE_FIELD_DIP0, TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_DIP2,
	  TEMPLATE_FIELD_DIP3, TEMPLATE_FIELD_DIP4, TEMPLATE_FIELD_DIP5,
	  TEMPLATE_FIELD_DIP6, TEMPLATE_FIELD_DIP7, TEMPLATE_FIELD_L4_DPORT,
	  TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_ICMP_IGMP, TEMPLATE_FIELD_IP_TOS_PROTO
	}, {
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_SIP2,
	  TEMPLATE_FIELD_SIP3, TEMPLATE_FIELD_SIP4, TEMPLATE_FIELD_SIP5,
	  TEMPLATE_FIELD_SIP6, TEMPLATE_FIELD_SIP7, TEMPLATE_FIELD_ITAG,
	  TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1
	},
};

const struct rtldsa_mib_list_item rtldsa_838x_mib_list[] = {
	MIB_LIST_ITEM("dot1dTpPortInDiscards", MIB_ITEM(MIB_REG_STD, 0xec, 1)),
	MIB_LIST_ITEM("ifOutDiscards", MIB_ITEM(MIB_REG_STD, 0xd0, 1)),
	MIB_LIST_ITEM("DropEvents", MIB_ITEM(MIB_REG_STD, 0xa8, 1)),
	MIB_LIST_ITEM("tx_BroadcastPkts", MIB_ITEM(MIB_REG_STD, 0xa4, 1)),
	MIB_LIST_ITEM("tx_MulticastPkts", MIB_ITEM(MIB_REG_STD, 0xa0, 1)),
	MIB_LIST_ITEM("tx_UndersizePkts", MIB_ITEM(MIB_REG_STD, 0x98, 1)),
	MIB_LIST_ITEM("rx_UndersizeDropPkts", MIB_ITEM(MIB_REG_STD, 0x90, 1)),
	MIB_LIST_ITEM("tx_OversizePkts", MIB_ITEM(MIB_REG_STD, 0x8c, 1)),
	MIB_LIST_ITEM("Collisions", MIB_ITEM(MIB_REG_STD, 0x7c, 1)),
	MIB_LIST_ITEM("rx_MacDiscards", MIB_ITEM(MIB_REG_STD, 0x40, 1))
};

const struct rtldsa_mib_desc rtldsa_838x_mib_desc = {
	.symbol_errors = MIB_ITEM(MIB_REG_STD, 0xb8, 1),

	.if_in_octets = MIB_ITEM(MIB_REG_STD, 0xf8, 2),
	.if_out_octets = MIB_ITEM(MIB_REG_STD, 0xf0, 2),
	.if_in_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xe8, 1),
	.if_in_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe4, 1),
	.if_in_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xe0, 1),
	.if_out_ucast_pkts = MIB_ITEM(MIB_REG_STD, 0xdc, 1),
	.if_out_mcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd8, 1),
	.if_out_bcast_pkts = MIB_ITEM(MIB_REG_STD, 0xd4, 1),
	.if_out_discards = MIB_ITEM(MIB_REG_STD, 0xd0, 1),
	.single_collisions = MIB_ITEM(MIB_REG_STD, 0xcc, 1),
	.multiple_collisions = MIB_ITEM(MIB_REG_STD, 0xc8, 1),
	.deferred_transmissions = MIB_ITEM(MIB_REG_STD, 0xc4, 1),
	.late_collisions = MIB_ITEM(MIB_REG_STD, 0xc0, 1),
	.excessive_collisions = MIB_ITEM(MIB_REG_STD, 0xbc, 1),
	.crc_align_errors = MIB_ITEM(MIB_REG_STD, 0x9c, 1),

	.unsupported_opcodes = MIB_ITEM(MIB_REG_STD, 0xb4, 1),

	.rx_undersize_pkts = MIB_ITEM(MIB_REG_STD, 0x94, 1),
	.rx_oversize_pkts = MIB_ITEM(MIB_REG_STD, 0x88, 1),
	.rx_fragments = MIB_ITEM(MIB_REG_STD, 0x84, 1),
	.rx_jabbers = MIB_ITEM(MIB_REG_STD, 0x80, 1),

	.tx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x78, 1),
		MIB_ITEM(MIB_REG_STD, 0x70, 1),
		MIB_ITEM(MIB_REG_STD, 0x68, 1),
		MIB_ITEM(MIB_REG_STD, 0x60, 1),
		MIB_ITEM(MIB_REG_STD, 0x58, 1),
		MIB_ITEM(MIB_REG_STD, 0x50, 1),
		MIB_ITEM(MIB_REG_STD, 0x48, 1)
	},
	.rx_pkts = {
		MIB_ITEM(MIB_REG_STD, 0x74, 1),
		MIB_ITEM(MIB_REG_STD, 0x6c, 1),
		MIB_ITEM(MIB_REG_STD, 0x64, 1),
		MIB_ITEM(MIB_REG_STD, 0x5c, 1),
		MIB_ITEM(MIB_REG_STD, 0x54, 1),
		MIB_ITEM(MIB_REG_STD, 0x4c, 1),
		MIB_ITEM(MIB_REG_STD, 0x44, 1)
	},
	.rmon_ranges = {
		{ 0, 64 },
		{ 65, 127 },
		{ 128, 255 },
		{ 256, 511 },
		{ 512, 1023 },
		{ 1024, 1518 },
		{ 1519, 10000 }
	},

	.drop_events = MIB_ITEM(MIB_REG_STD, 0xa8, 1),
	.collisions = MIB_ITEM(MIB_REG_STD, 0x7c, 1),

	.rx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xb0, 1),
	.tx_pause_frames = MIB_ITEM(MIB_REG_STD, 0xac, 1),

	.list_count = ARRAY_SIZE(rtldsa_838x_mib_list),
	.list = rtldsa_838x_mib_list
};

void rtldsa_838x_print_matrix(void)
{
	unsigned volatile int *ptr8;

	ptr8 = RTL838X_SW_BASE + RTL838X_PORT_ISO_CTRL(0);
	for (int i = 0; i < 28; i += 8)
		pr_debug("> %8x %8x %8x %8x %8x %8x %8x %8x\n",
			 ptr8[i + 0], ptr8[i + 1], ptr8[i + 2], ptr8[i + 3],
			 ptr8[i + 4], ptr8[i + 5], ptr8[i + 6], ptr8[i + 7]);
	pr_debug("CPU_PORT> %8x\n", ptr8[28]);
}

static inline int rtl838x_port_iso_ctrl(int p)
{
	return RTL838X_PORT_ISO_CTRL(p);
}

static void rtl838x_vlan_tables_read(u32 vlan, struct rtldsa_vlan_info *info)
{
	u32 v;
	/* Read VLAN table (0) via register 0 */
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 0);

	rtl_table_read(r, vlan);
	info->member_ports = sw_r32(rtl_table_data(r, 0));
	v = sw_r32(rtl_table_data(r, 1));
	pr_debug("VLAN_READ %d: %016llx %08x\n", vlan, info->member_ports, v);
	rtl_table_release(r);

	info->profile_id = v & 0x7;
	info->hash_mc_fid = !!(v & 0x8);
	info->hash_uc_fid = !!(v & 0x10);
	info->fid = (v >> 5) & 0x3f;

	/* Read UNTAG table (0) via table register 1 */
	r = rtl_table_get(RTL8380_TBL_1, 0);
	rtl_table_read(r, vlan);
	info->untagged_ports = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);
}

static void rtl838x_vlan_set_tagged(u32 vlan, struct rtldsa_vlan_info *info)
{
	u32 v;
	/* Access VLAN table (0) via register 0 */
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 0);

	sw_w32(info->member_ports, rtl_table_data(r, 0));

	v = info->profile_id;
	v |= info->hash_mc_fid ? 0x8 : 0;
	v |= info->hash_uc_fid ? 0x10 : 0;
	v |= ((u32)info->fid) << 5;
	sw_w32(v, rtl_table_data(r, 1));

	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static void rtl838x_vlan_set_untagged(u32 vlan, u64 portmask)
{
	/* Access UNTAG table (0) via register 1 */
	struct table_reg *r = rtl_table_get(RTL8380_TBL_1, 0);

	sw_w32(portmask & RTL838X_MC_PMASK_ALL_PORTS, rtl_table_data(r, 0));
	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

/* Sets the L2 forwarding to be based on either the inner VLAN tag or the outer
 */
static void rtl838x_vlan_fwd_on_inner(int port, bool is_set)
{
	if (is_set)
		sw_w32_mask(BIT(port), 0, RTL838X_VLAN_PORT_FWD);
	else
		sw_w32_mask(0, BIT(port), RTL838X_VLAN_PORT_FWD);
}

static u64 rtl838x_l2_hash_seed(u64 mac, u32 vid)
{
	return mac << 12 | vid;
}

/* Applies the same hash algorithm as the one used currently by the ASIC to the seed
 * and returns a key into the L2 hash table
 */
static u32 rtl838x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 h1, h2, h3, h;

	if (sw_r32(priv->r->l2_ctrl_0) & 1) {
		h1 = (seed >> 11) & 0x7ff;
		h1 = ((h1 & 0x1f) << 6) | ((h1 >> 5) & 0x3f);

		h2 = (seed >> 33) & 0x7ff;
		h2 = ((h2 & 0x3f) << 5) | ((h2 >> 6) & 0x1f);

		h3 = (seed >> 44) & 0x7ff;
		h3 = ((h3 & 0x7f) << 4) | ((h3 >> 7) & 0xf);

		h = h1 ^ h2 ^ h3 ^ ((seed >> 55) & 0x1ff);
		h ^= ((seed >> 22) & 0x7ff) ^ (seed & 0x7ff);
	} else {
		h = ((seed >> 55) & 0x1ff) ^ ((seed >> 44) & 0x7ff) ^
		    ((seed >> 33) & 0x7ff) ^ ((seed >> 22) & 0x7ff) ^
		    ((seed >> 11) & 0x7ff) ^ (seed & 0x7ff);
	}

	return h;
}

static inline int rtl838x_mac_force_mode_ctrl(int p)
{
	return RTL838X_MAC_FORCE_MODE_CTRL + (p << 2);
}

static inline int rtl838x_mac_port_ctrl(int p)
{
	return RTL838X_MAC_PORT_CTRL(p);
}

static inline int rtl838x_l2_port_new_salrn(int p)
{
	return RTL838X_L2_PORT_NEW_SALRN(p);
}

static inline int rtl838x_l2_port_new_sa_fwd(int p)
{
	return RTL838X_L2_PORT_NEW_SA_FWD(p);
}

static int rtldsa_838x_get_mirror_config(struct rtldsa_mirror_config *config,
					 int group, int port)
{
	config->ctrl = RTL838X_MIR_CTRL + group * 4;
	config->spm = RTL838X_MIR_SPM_CTRL + group * 4;
	config->dpm = RTL838X_MIR_DPM_CTRL + group * 4;

	/* Enable mirroring to destination port */
	config->val = BIT(0);
	config->val |= port << 4;

	/* Enable mirroring to port across VLANs */
	config->val |= BIT(11);

	return 0;
}

static inline int rtl838x_trk_mbr_ctr(int group)
{
	return RTL838X_TRK_MBR_CTR + (group << 2);
}

/* Fills an L2 entry structure from the SoC registers */
static void rtl838x_fill_l2_entry(u32 r[], struct rtl838x_l2_entry *e)
{
	/* Table contains different entry types, we need to identify the right one:
	 * Check for MC entries, first
	 * In contrast to the RTL93xx SoCs, there is no valid bit, use heuristics to
	 * identify valid entries
	 */
	e->is_ip_mc = !!(r[0] & BIT(22));
	e->is_ipv6_mc = !!(r[0] & BIT(21));
	e->type = L2_INVALID;

	if (!e->is_ip_mc && !e->is_ipv6_mc) {
		e->mac[0] = (r[1] >> 20);
		e->mac[1] = (r[1] >> 12);
		e->mac[2] = (r[1] >> 4);
		e->mac[3] = (r[1] & 0xf) << 4 | (r[2] >> 28);
		e->mac[4] = (r[2] >> 20);
		e->mac[5] = (r[2] >> 12);

		e->rvid = r[2] & 0xfff;
		e->vid = r[0] & 0xfff;

		/* Is it a unicast entry? check multicast bit */
		if (!(e->mac[0] & 1)) {
			e->is_static = !!((r[0] >> 19) & 1);
			e->port = (r[0] >> 12) & 0x1f;
			e->block_da = !!(r[1] & BIT(30));
			e->block_sa = !!(r[1] & BIT(31));
			e->suspended = !!(r[1] & BIT(29));
			e->next_hop = !!(r[1] & BIT(28));
			if (e->next_hop) {
				pr_debug("Found next hop entry, need to read extra data\n");
				e->nh_vlan_target = !!(r[0] & BIT(9));
				e->nh_route_id = r[0] & 0x1ff;
				e->vid = e->rvid;
			}
			e->age = (r[0] >> 17) & 0x3;
			e->valid = true;

			/* A valid entry has one of multi-cast, aging, sa/da-blocking,
			 * next-hop or static entry bit set
			 */
			if (!(r[0] & 0x007c0000) && !(r[1] & 0xd0000000))
				e->valid = false;
			else
				e->type = L2_UNICAST;
		} else { /* L2 multicast */
			pr_debug("Got L2 MC entry: %08x %08x %08x\n", r[0], r[1], r[2]);
			e->valid = true;
			e->type = L2_MULTICAST;
			e->mc_portmask_index = (r[0] >> 12) & 0x1ff;
		}
	} else { /* IPv4 and IPv6 multicast */
		e->valid = true;
		e->mc_portmask_index = (r[0] >> 12) & 0x1ff;
		e->mc_gip = (r[1] << 20) | (r[2] >> 12);
		e->rvid = r[2] & 0xfff;
	}
	if (e->is_ip_mc)
		e->type = IP4_MULTICAST;
	if (e->is_ipv6_mc)
		e->type = IP6_MULTICAST;
}

/* Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry */
static void rtl838x_fill_l2_row(u32 r[], struct rtl838x_l2_entry *e)
{
	u64 mac = ether_addr_to_u64(e->mac);

	if (!e->valid) {
		r[0] = r[1] = r[2] = 0;
		return;
	}

	r[0] = e->is_ip_mc ? BIT(22) : 0;
	r[0] |= e->is_ipv6_mc ? BIT(21) : 0;

	if (!e->is_ip_mc && !e->is_ipv6_mc) {
		r[1] = mac >> 20;
		r[2] = (mac & 0xfffff) << 12;

		/* Is it a unicast entry? check multicast bit */
		if (!(e->mac[0] & 1)) {
			r[0] |= e->is_static ? BIT(19) : 0;
			r[0] |= (e->port & 0x3f) << 12;
			r[0] |= e->vid;
			r[1] |= e->block_da ? BIT(30) : 0;
			r[1] |= e->block_sa ? BIT(31) : 0;
			r[1] |= e->suspended ? BIT(29) : 0;
			r[2] |= e->rvid & 0xfff;
			if (e->next_hop) {
				r[1] |= BIT(28);
				r[0] |= e->nh_vlan_target ? BIT(9) : 0;
				r[0] |= e->nh_route_id & 0x1ff;
			}
			r[0] |= (e->age & 0x3) << 17;
		} else { /* L2 Multicast */
			r[0] |= (e->mc_portmask_index & 0x1ff) << 12;
			r[2] |= e->rvid & 0xfff;
			r[0] |= e->vid & 0xfff;
			pr_debug("FILL MC: %08x %08x %08x\n", r[0], r[1], r[2]);
		}
	} else { /* IPv4 and IPv6 multicast */
		r[0] |= (e->mc_portmask_index & 0x1ff) << 12;
		r[1] = e->mc_gip >> 20;
		r[2] = e->mc_gip << 12;
		r[2] |= e->rvid;
	}
}

/* Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl838x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 0); /* Access L2 Table 0 */
	u32 idx = (0 << 14) | (hash << 2) | pos; /* Search SRAM, with hash and at pos in bucket */

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl838x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	return (((u64)r[1]) << 32) | (r[2]);  /* mac and vid concatenated as hash seed */
}

static void rtl838x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 0);

	u32 idx = (0 << 14) | (hash << 2) | pos; /* Access SRAM, with hash and at pos in bucket */

	rtl838x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl838x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 1); /* Access L2 Table 1 */

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl838x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	pr_debug("Found in CAM: R1 %x R2 %x R3 %x\n", r[0], r[1], r[2]);

	/* Return MAC with concatenated VID ac concatenated ID */
	return (((u64)r[1]) << 32) | r[2];
}

static void rtl838x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 1); /* Access L2 Table 1 */

	rtl838x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl838x_read_mcast_pmask(int idx)
{
	u32 portmask;
	/* Read MC_PMSK (2) via register RTL8380_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	rtl_table_release(q);

	return portmask;
}

static void rtl838x_write_mcast_pmask(int idx, u64 portmask)
{
	/* Access MC_PMSK (2) via register RTL8380_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 2);

	sw_w32(((u32)portmask) & RTL838X_MC_PMASK_ALL_PORTS, rtl_table_data(q, 0));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static int
rtldsa_838x_vlan_profile_get(int idx, struct rtldsa_vlan_profile *profile)
{
	u32 p;

	if (idx < 0 || idx > RTL838X_VLAN_PROFILE_MAX)
		return -EINVAL;

	p = sw_r32(RTL838X_VLAN_PROFILE(idx));

	*profile = (struct rtldsa_vlan_profile) {
		.l2_learn = RTL838X_VLAN_L2_LEARN_EN_R(p),
		.unkn_mc_fld.pmsks_idx = {
			.l2 = RTL838X_VLAN_L2_UNKN_MC_FLD_PMSK(p),
			.ip = RTL838X_VLAN_IP4_UNKN_MC_FLD_PMSK(p),
			.ip6 = RTL838X_VLAN_IP6_UNKN_MC_FLD_PMSK(p),
		},
		.pmsk_is_idx = 1,
	};

	return 0;
}

static void rtl838x_vlan_profile_setup(int profile)
{
	u32 p = RTL838X_VLAN_L2_LEARN_EN(1) |
		RTL838X_VLAN_L2_UNKN_MC_FLD(MC_PMASK_ALL_PORTS_IDX) |
		RTL838X_VLAN_IP4_UNKN_MC_FLD(MC_PMASK_ALL_PORTS_IDX) |
		RTL838X_VLAN_IP6_UNKN_MC_FLD(MC_PMASK_ALL_PORTS_IDX);

	sw_w32(p, RTL838X_VLAN_PROFILE(profile));
}

static void rtl838x_l2_learning_setup(void)
{
	/* Set portmask for broadcast traffic and unknown unicast address flooding
	 * to the reserved entry in the portmask table used also for
	 * multicast flooding
	 */
	sw_w32(RTL838X_L2_BC_FLD(MC_PMASK_ALL_PORTS_IDX) |
	       RTL838X_L2_UNKN_UC_FLD(MC_PMASK_ALL_PORTS_IDX),
	       RTL838X_L2_FLD_PMSK);

	/* Enable learning constraint system-wide (bit 0), per-port (bit 1)
	 * and per vlan (bit 2)
	 */
	sw_w32(0x7, RTL838X_L2_LRN_CONSTRT_EN);

	/* Limit learning to maximum: 16k entries, after that just flood (bits 0-1) */
	sw_w32((0x3fff << 2) | 0, RTL838X_L2_LRN_CONSTRT);

	/* Do not trap ARP packets to CPU_PORT */
	sw_w32(0, RTL838X_SPCL_TRAP_ARP_CTRL);
}

static void rtl838x_enable_learning(int port, bool enable)
{
	/* Limit learning to maximum: 16k entries */

	sw_w32_mask(0x3fff << 2, enable ? (0x3fff << 2) : 0,
		    RTL838X_L2_PORT_LRN_CONSTRT + (port << 2));
}

static void rtl838x_enable_flood(int port, enum rtldsa_flood_type mode)
{
	/* 0: Forward
	 * 1: Disable
	 * 2: to CPU
	 * 3: Copy to CPU
	 */
	sw_w32_mask(0x3, mode,
		    RTL838X_L2_PORT_LRN_CONSTRT + (port << 2));
}

static void rtl838x_enable_mcast_flood(int port, bool enable)
{
}

static void rtl838x_enable_bcast_flood(int port, bool enable)
{
}

static void rtl838x_set_static_move_action(int port, bool forward)
{
	int shift = MV_ACT_PORT_SHIFT(port);
	u32 val = forward ? MV_ACT_FORWARD : MV_ACT_DROP;

	sw_w32_mask(MV_ACT_MASK << shift, val << shift,
		    RTL838X_L2_PORT_STATIC_MV_ACT(port));
}

static int rtldsa_838x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, int port)
{
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 2);
	int idx = 1 - (port / 16);
	int bit = 2 * (port % 16);
	int state;

	rtl_table_read(r, msti);
	state = (sw_r32(rtl_table_data(r, idx)) >> bit) & 0x3;
	rtl_table_release(r);

	return state;
}

static void rtl838x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, int port, int state)
{
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 2);
	int idx = 1 - (port / 16);
	int bit = 2 * (port % 16);

	rtl_table_read(r, msti);
	sw_w32_mask(0x3 << bit, state << bit, rtl_table_data(r, idx));
	rtl_table_write(r, msti);
	rtl_table_release(r);
}

static void rtl838x_traffic_set(int source, u64 dest_matrix)
{
	rtl838x_set_port_reg(dest_matrix, rtl838x_port_iso_ctrl(source));
}

static void rtl838x_traffic_enable(int source, int dest)
{
	rtl838x_mask_port_reg(0, BIT(dest), rtl838x_port_iso_ctrl(source));
}

static void rtl838x_traffic_disable(int source, int dest)
{
	rtl838x_mask_port_reg(BIT(dest), 0, rtl838x_port_iso_ctrl(source));
}

/* Enables or disables the EEE/EEEP capability of a port */
static void rtldsa_838x_set_mac_eee(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;

	/* This works only for Ethernet ports, and on the RTL838X, ports from 24 are SFP */
	if (port >= 24)
		return;

	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? 0x3 : 0x0;

	/* Set EEE state for 100 (bit 9) & 1000MBit (bit 10) */
	sw_w32_mask(0x3 << 9, v << 9, priv->r->mac_force_mode_ctrl(port));

	/* Set TX/RX EEE state */
	if (enable) {
		sw_w32_mask(0, BIT(port), RTL838X_EEE_PORT_TX_EN);
		sw_w32_mask(0, BIT(port), RTL838X_EEE_PORT_RX_EN);
	} else {
		sw_w32_mask(BIT(port), 0, RTL838X_EEE_PORT_TX_EN);
		sw_w32_mask(BIT(port), 0, RTL838X_EEE_PORT_RX_EN);
	}
	priv->ports[port].eee_enabled = enable;
}

static void rtl838x_init_eee(struct rtl838x_switch_priv *priv, bool enable)
{
	pr_debug("Setting up EEE, state: %d\n", enable);
	sw_w32_mask(0x4, 0, RTL838X_SMI_GLB_CTRL);

	/* Set timers for EEE */
	sw_w32(0x5001411, RTL838X_EEE_TX_TIMER_GIGA_CTRL);
	sw_w32(0x5001417, RTL838X_EEE_TX_TIMER_GELITE_CTRL);

	/* Enable EEE MAC support on ports */
	for (int i = 0; i < priv->r->cpu_port; i++) {
		if (priv->ports[i].phy)
			priv->r->set_mac_eee(priv, i, enable);
	}
	priv->eee_enabled = enable;
}

static void rtl838x_pie_lookup_enable(struct rtl838x_switch_priv *priv, int index)
{
	int block = index / PIE_BLOCK_SIZE;
	u32 block_state = sw_r32(RTL838X_ACL_BLK_LOOKUP_CTRL);

	/* Make sure rule-lookup is enabled in the block */
	if (!(block_state & BIT(block)))
		sw_w32(block_state | BIT(block), RTL838X_ACL_BLK_LOOKUP_CTRL);
}

static void rtl838x_pie_rule_del(struct rtl838x_switch_priv *priv, int index_from, int index_to)
{
	int block_from = index_from / PIE_BLOCK_SIZE;
	int block_to = index_to / PIE_BLOCK_SIZE;
	u32 v = (index_from << 1) | (index_to << 12) | BIT(0);
	u32 block_state;

	pr_debug("%s: from %d to %d\n", __func__, index_from, index_to);
	mutex_lock(&priv->reg_mutex);

	/* Remember currently active blocks */
	block_state = sw_r32(RTL838X_ACL_BLK_LOOKUP_CTRL);

	/* Make sure rule-lookup is disabled in the relevant blocks */
	for (int block = block_from; block <= block_to; block++) {
		if (block_state & BIT(block))
			sw_w32(block_state & (~BIT(block)), RTL838X_ACL_BLK_LOOKUP_CTRL);
	}

	/* Write from-to and execute bit into control register */
	sw_w32(v, RTL838X_ACL_CLR_CTRL);

	/* Wait until command has completed */
	do {
	} while (sw_r32(RTL838X_ACL_CLR_CTRL) & BIT(0));

	/* Re-enable rule lookup */
	for (int block = block_from; block <= block_to; block++) {
		if (!(block_state & BIT(block)))
			sw_w32(block_state | BIT(block), RTL838X_ACL_BLK_LOOKUP_CTRL);
	}

	mutex_unlock(&priv->reg_mutex);
}

/* Reads the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure and fills in the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL9310 has 2 more registers / fields and the physical field-ids
 * are specific to every platform.
 */
static void rtl838x_write_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		enum template_field_id field_type = t[i];
		u16 data = 0, data_m = 0;

		switch (field_type) {
		case TEMPLATE_FIELD_SPM0:
			data = pr->spm;
			data_m = pr->spm_m;
			break;
		case TEMPLATE_FIELD_SPM1:
			data = pr->spm >> 16;
			data_m = pr->spm_m >> 16;
			break;
		case TEMPLATE_FIELD_OTAG:
			data = pr->otag;
			data_m = pr->otag_m;
			break;
		case TEMPLATE_FIELD_SMAC0:
			data = pr->smac[4];
			data = (data << 8) | pr->smac[5];
			data_m = pr->smac_m[4];
			data_m = (data_m << 8) | pr->smac_m[5];
			break;
		case TEMPLATE_FIELD_SMAC1:
			data = pr->smac[2];
			data = (data << 8) | pr->smac[3];
			data_m = pr->smac_m[2];
			data_m = (data_m << 8) | pr->smac_m[3];
			break;
		case TEMPLATE_FIELD_SMAC2:
			data = pr->smac[0];
			data = (data << 8) | pr->smac[1];
			data_m = pr->smac_m[0];
			data_m = (data_m << 8) | pr->smac_m[1];
			break;
		case TEMPLATE_FIELD_DMAC0:
			data = pr->dmac[4];
			data = (data << 8) | pr->dmac[5];
			data_m = pr->dmac_m[4];
			data_m = (data_m << 8) | pr->dmac_m[5];
			break;
		case TEMPLATE_FIELD_DMAC1:
			data = pr->dmac[2];
			data = (data << 8) | pr->dmac[3];
			data_m = pr->dmac_m[2];
			data_m = (data_m << 8) | pr->dmac_m[3];
			break;
		case TEMPLATE_FIELD_DMAC2:
			data = pr->dmac[0];
			data = (data << 8) | pr->dmac[1];
			data_m = pr->dmac_m[0];
			data_m = (data_m << 8) | pr->dmac_m[1];
			break;
		case TEMPLATE_FIELD_ETHERTYPE:
			data = pr->ethertype;
			data_m = pr->ethertype_m;
			break;
		case TEMPLATE_FIELD_ITAG:
			data = pr->itag;
			data_m = pr->itag_m;
			break;
		case TEMPLATE_FIELD_RANGE_CHK:
			data = pr->field_range_check;
			data_m = pr->field_range_check_m;
			break;
		case TEMPLATE_FIELD_SIP0:
			if (pr->is_ipv6) {
				data = pr->sip6.s6_addr16[7];
				data_m = pr->sip6_m.s6_addr16[7];
			} else {
				data = pr->sip;
				data_m = pr->sip_m;
			}
			break;
		case TEMPLATE_FIELD_SIP1:
			if (pr->is_ipv6) {
				data = pr->sip6.s6_addr16[6];
				data_m = pr->sip6_m.s6_addr16[6];
			} else {
				data = pr->sip >> 16;
				data_m = pr->sip_m >> 16;
			}
			break;
		case TEMPLATE_FIELD_SIP2:
		case TEMPLATE_FIELD_SIP3:
		case TEMPLATE_FIELD_SIP4:
		case TEMPLATE_FIELD_SIP5:
		case TEMPLATE_FIELD_SIP6:
		case TEMPLATE_FIELD_SIP7:
			data = pr->sip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
			data_m = pr->sip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
			break;
		case TEMPLATE_FIELD_DIP0:
			if (pr->is_ipv6) {
				data = pr->dip6.s6_addr16[7];
				data_m = pr->dip6_m.s6_addr16[7];
			} else {
				data = pr->dip;
				data_m = pr->dip_m;
			}
			break;
		case TEMPLATE_FIELD_DIP1:
			if (pr->is_ipv6) {
				data = pr->dip6.s6_addr16[6];
				data_m = pr->dip6_m.s6_addr16[6];
			} else {
				data = pr->dip >> 16;
				data_m = pr->dip_m >> 16;
			}
			break;
		case TEMPLATE_FIELD_DIP2:
		case TEMPLATE_FIELD_DIP3:
		case TEMPLATE_FIELD_DIP4:
		case TEMPLATE_FIELD_DIP5:
		case TEMPLATE_FIELD_DIP6:
		case TEMPLATE_FIELD_DIP7:
			data = pr->dip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
			data_m = pr->dip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
			break;
		case TEMPLATE_FIELD_IP_TOS_PROTO:
			data = pr->tos_proto;
			data_m = pr->tos_proto_m;
			break;
		case TEMPLATE_FIELD_L4_SPORT:
			data = pr->sport;
			data_m = pr->sport_m;
			break;
		case TEMPLATE_FIELD_L4_DPORT:
			data = pr->dport;
			data_m = pr->dport_m;
			break;
		case TEMPLATE_FIELD_ICMP_IGMP:
			data = pr->icmp_igmp;
			data_m = pr->icmp_igmp_m;
			break;
		default:
			pr_debug("%s: unknown field %d\n", __func__, field_type);
			continue;
		}
		if (!(i % 2)) {
			r[5 - i / 2] = data;
			r[12 - i / 2] = data_m;
		} else {
			r[5 - i / 2] |= ((u32)data) << 16;
			r[12 - i / 2] |= ((u32)data_m) << 16;
		}
	}
}

/* Creates the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure by reading the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL9310 has 2 more registers / fields and the physical field-ids
 */
static void rtl838x_read_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		enum template_field_id field_type = t[i];
		u16 data, data_m;

		field_type = t[i];
		if (!(i % 2)) {
			data = r[5 - i / 2];
			data_m = r[12 - i / 2];
		} else {
			data = r[5 - i / 2] >> 16;
			data_m = r[12 - i / 2] >> 16;
		}

		switch (field_type) {
		case TEMPLATE_FIELD_SPM0:
			pr->spm = (pr->spn << 16) | data;
			pr->spm_m = (pr->spn << 16) | data_m;
			break;
		case TEMPLATE_FIELD_SPM1:
			pr->spm = data;
			pr->spm_m = data_m;
			break;
		case TEMPLATE_FIELD_OTAG:
			pr->otag = data;
			pr->otag_m = data_m;
			break;
		case TEMPLATE_FIELD_SMAC0:
			pr->smac[4] = data >> 8;
			pr->smac[5] = data;
			pr->smac_m[4] = data >> 8;
			pr->smac_m[5] = data;
			break;
		case TEMPLATE_FIELD_SMAC1:
			pr->smac[2] = data >> 8;
			pr->smac[3] = data;
			pr->smac_m[2] = data >> 8;
			pr->smac_m[3] = data;
			break;
		case TEMPLATE_FIELD_SMAC2:
			pr->smac[0] = data >> 8;
			pr->smac[1] = data;
			pr->smac_m[0] = data >> 8;
			pr->smac_m[1] = data;
			break;
		case TEMPLATE_FIELD_DMAC0:
			pr->dmac[4] = data >> 8;
			pr->dmac[5] = data;
			pr->dmac_m[4] = data >> 8;
			pr->dmac_m[5] = data;
			break;
		case TEMPLATE_FIELD_DMAC1:
			pr->dmac[2] = data >> 8;
			pr->dmac[3] = data;
			pr->dmac_m[2] = data >> 8;
			pr->dmac_m[3] = data;
			break;
		case TEMPLATE_FIELD_DMAC2:
			pr->dmac[0] = data >> 8;
			pr->dmac[1] = data;
			pr->dmac_m[0] = data >> 8;
			pr->dmac_m[1] = data;
			break;
		case TEMPLATE_FIELD_ETHERTYPE:
			pr->ethertype = data;
			pr->ethertype_m = data_m;
			break;
		case TEMPLATE_FIELD_ITAG:
			pr->itag = data;
			pr->itag_m = data_m;
			break;
		case TEMPLATE_FIELD_RANGE_CHK:
			pr->field_range_check = data;
			pr->field_range_check_m = data_m;
			break;
		case TEMPLATE_FIELD_SIP0:
			pr->sip = data;
			pr->sip_m = data_m;
			break;
		case TEMPLATE_FIELD_SIP1:
			pr->sip = (pr->sip << 16) | data;
			pr->sip_m = (pr->sip << 16) | data_m;
			break;
		case TEMPLATE_FIELD_SIP2:
			pr->is_ipv6 = true;
			/* Make use of limitiations on the position of the match values */
			ipv6_addr_set(&pr->sip6, pr->sip, r[5 - i / 2],
				      r[4 - i / 2], r[3 - i / 2]);
			ipv6_addr_set(&pr->sip6_m, pr->sip_m, r[5 - i / 2],
				      r[4 - i / 2], r[3 - i / 2]);
		case TEMPLATE_FIELD_SIP3:
		case TEMPLATE_FIELD_SIP4:
		case TEMPLATE_FIELD_SIP5:
		case TEMPLATE_FIELD_SIP6:
		case TEMPLATE_FIELD_SIP7:
			break;
		case TEMPLATE_FIELD_DIP0:
			pr->dip = data;
			pr->dip_m = data_m;
			break;
		case TEMPLATE_FIELD_DIP1:
			pr->dip = (pr->dip << 16) | data;
			pr->dip_m = (pr->dip << 16) | data_m;
			break;
		case TEMPLATE_FIELD_DIP2:
			pr->is_ipv6 = true;
			ipv6_addr_set(&pr->dip6, pr->dip, r[5 - i / 2],
				      r[4 - i / 2], r[3 - i / 2]);
			ipv6_addr_set(&pr->dip6_m, pr->dip_m, r[5 - i / 2],
				      r[4 - i / 2], r[3 - i / 2]);
		case TEMPLATE_FIELD_DIP3:
		case TEMPLATE_FIELD_DIP4:
		case TEMPLATE_FIELD_DIP5:
		case TEMPLATE_FIELD_DIP6:
		case TEMPLATE_FIELD_DIP7:
			break;
		case TEMPLATE_FIELD_IP_TOS_PROTO:
			pr->tos_proto = data;
			pr->tos_proto_m = data_m;
			break;
		case TEMPLATE_FIELD_L4_SPORT:
			pr->sport = data;
			pr->sport_m = data_m;
			break;
		case TEMPLATE_FIELD_L4_DPORT:
			pr->dport = data;
			pr->dport_m = data_m;
			break;
		case TEMPLATE_FIELD_ICMP_IGMP:
			pr->icmp_igmp = data;
			pr->icmp_igmp_m = data_m;
			break;
		default:
			pr_debug("%s: unknown field %d\n", __func__, field_type);
		}
	}
}

static void rtl838x_read_pie_fixed_fields(u32 r[], struct pie_rule *pr)
{
	pr->spmmask_fix = (r[6] >> 22) & 0x3;
	pr->spn = (r[6] >> 16) & 0x3f;
	pr->mgnt_vlan = (r[6] >> 15) & 1;
	pr->dmac_hit_sw = (r[6] >> 14) & 1;
	pr->not_first_frag = (r[6] >> 13) & 1;
	pr->frame_type_l4 = (r[6] >> 10) & 7;
	pr->frame_type = (r[6] >> 8) & 3;
	pr->otag_fmt = (r[6] >> 7) & 1;
	pr->itag_fmt = (r[6] >> 6) & 1;
	pr->otag_exist = (r[6] >> 5) & 1;
	pr->itag_exist = (r[6] >> 4) & 1;
	pr->frame_type_l2 = (r[6] >> 2) & 3;
	pr->tid = r[6] & 3;

	pr->spmmask_fix_m = (r[13] >> 22) & 0x3;
	pr->spn_m = (r[13] >> 16) & 0x3f;
	pr->mgnt_vlan_m = (r[13] >> 15) & 1;
	pr->dmac_hit_sw_m = (r[13] >> 14) & 1;
	pr->not_first_frag_m = (r[13] >> 13) & 1;
	pr->frame_type_l4_m = (r[13] >> 10) & 7;
	pr->frame_type_m = (r[13] >> 8) & 3;
	pr->otag_fmt_m = (r[13] >> 7) & 1;
	pr->itag_fmt_m = (r[13] >> 6) & 1;
	pr->otag_exist_m = (r[13] >> 5) & 1;
	pr->itag_exist_m = (r[13] >> 4) & 1;
	pr->frame_type_l2_m = (r[13] >> 2) & 3;
	pr->tid_m = r[13] & 3;

	pr->valid = r[14] & BIT(31);
	pr->cond_not = r[14] & BIT(30);
	pr->cond_and1 = r[14] & BIT(29);
	pr->cond_and2 = r[14] & BIT(28);
	pr->ivalid = r[14] & BIT(27);

	pr->drop = (r[17] >> 14) & 3;
	pr->fwd_sel = r[17] & BIT(13);
	pr->ovid_sel = r[17] & BIT(12);
	pr->ivid_sel = r[17] & BIT(11);
	pr->flt_sel = r[17] & BIT(10);
	pr->log_sel = r[17] & BIT(9);
	pr->rmk_sel = r[17] & BIT(8);
	pr->meter_sel = r[17] & BIT(7);
	pr->tagst_sel = r[17] & BIT(6);
	pr->mir_sel = r[17] & BIT(5);
	pr->nopri_sel = r[17] & BIT(4);
	pr->cpupri_sel = r[17] & BIT(3);
	pr->otpid_sel = r[17] & BIT(2);
	pr->itpid_sel = r[17] & BIT(1);
	pr->shaper_sel = r[17] & BIT(0);
}

static void rtl838x_write_pie_fixed_fields(u32 r[],  struct pie_rule *pr)
{
	r[6] = ((u32)(pr->spmmask_fix & 0x3)) << 22;
	r[6] |= ((u32)(pr->spn & 0x3f)) << 16;
	r[6] |= pr->mgnt_vlan ? BIT(15) : 0;
	r[6] |= pr->dmac_hit_sw ? BIT(14) : 0;
	r[6] |= pr->not_first_frag ? BIT(13) : 0;
	r[6] |= ((u32)(pr->frame_type_l4 & 0x7)) << 10;
	r[6] |= ((u32)(pr->frame_type & 0x3)) << 8;
	r[6] |= pr->otag_fmt ? BIT(7) : 0;
	r[6] |= pr->itag_fmt ? BIT(6) : 0;
	r[6] |= pr->otag_exist ? BIT(5) : 0;
	r[6] |= pr->itag_exist ? BIT(4) : 0;
	r[6] |= ((u32)(pr->frame_type_l2 & 0x3)) << 2;
	r[6] |= ((u32)(pr->tid & 0x3));

	r[13] = ((u32)(pr->spmmask_fix_m & 0x3)) << 22;
	r[13] |= ((u32)(pr->spn_m & 0x3f)) << 16;
	r[13] |= pr->mgnt_vlan_m ? BIT(15) : 0;
	r[13] |= pr->dmac_hit_sw_m ? BIT(14) : 0;
	r[13] |= pr->not_first_frag_m ? BIT(13) : 0;
	r[13] |= ((u32)(pr->frame_type_l4_m & 0x7)) << 10;
	r[13] |= ((u32)(pr->frame_type_m & 0x3)) << 8;
	r[13] |= pr->otag_fmt_m ? BIT(7) : 0;
	r[13] |= pr->itag_fmt_m ? BIT(6) : 0;
	r[13] |= pr->otag_exist_m ? BIT(5) : 0;
	r[13] |= pr->itag_exist_m ? BIT(4) : 0;
	r[13] |= ((u32)(pr->frame_type_l2_m & 0x3)) << 2;
	r[13] |= ((u32)(pr->tid_m & 0x3));

	r[14] = pr->valid ? BIT(31) : 0;
	r[14] |= pr->cond_not ? BIT(30) : 0;
	r[14] |= pr->cond_and1 ? BIT(29) : 0;
	r[14] |= pr->cond_and2 ? BIT(28) : 0;
	r[14] |= pr->ivalid ? BIT(27) : 0;

	if (pr->drop)
		r[17] = 0x1 << 14;	/* Standard drop action */
	else
		r[17] = 0;
	r[17] |= pr->fwd_sel ? BIT(13) : 0;
	r[17] |= pr->ovid_sel ? BIT(12) : 0;
	r[17] |= pr->ivid_sel ? BIT(11) : 0;
	r[17] |= pr->flt_sel ? BIT(10) : 0;
	r[17] |= pr->log_sel ? BIT(9) : 0;
	r[17] |= pr->rmk_sel ? BIT(8) : 0;
	r[17] |= pr->meter_sel ? BIT(7) : 0;
	r[17] |= pr->tagst_sel ? BIT(6) : 0;
	r[17] |= pr->mir_sel ? BIT(5) : 0;
	r[17] |= pr->nopri_sel ? BIT(4) : 0;
	r[17] |= pr->cpupri_sel ? BIT(3) : 0;
	r[17] |= pr->otpid_sel ? BIT(2) : 0;
	r[17] |= pr->itpid_sel ? BIT(1) : 0;
	r[17] |= pr->shaper_sel ? BIT(0) : 0;
}

static int rtl838x_write_pie_action(u32 r[],  struct pie_rule *pr)
{
	u16 *aif = (u16 *)&r[17];
	u16 data;
	int fields_used = 0;

	aif--;

	pr_debug("%s, at %08x\n", __func__, (u32)aif);
	/* Multiple actions can be linked to a match of a PIE rule,
	 * they have different precedence depending on their type and this precedence
	 * defines which Action Information Field (0-4) in the IACL table stores
	 * the additional data of the action (like e.g. the port number a packet is
	 * forwarded to)
	 */
	/* TODO: count bits in selectors to limit to a maximum number of actions */
	if (pr->fwd_sel) { /* Forwarding action */
		data = pr->fwd_act << 13;
		data |= pr->fwd_data;
		data |= pr->bypass_all ? BIT(12) : 0;
		data |= pr->bypass_ibc_sc ? BIT(11) : 0;
		data |= pr->bypass_igr_stp ? BIT(10) : 0;
		*aif-- = data;
		fields_used++;
	}

	if (pr->ovid_sel) { /* Outer VID action */
		data = (pr->ovid_act & 0x3) << 12;
		data |= pr->ovid_data;
		*aif-- = data;
		fields_used++;
	}

	if (pr->ivid_sel) { /* Inner VID action */
		data = (pr->ivid_act & 0x3) << 12;
		data |= pr->ivid_data;
		*aif-- = data;
		fields_used++;
	}

	if (pr->flt_sel) { /* Filter action */
		*aif-- = pr->flt_data;
		fields_used++;
	}

	if (pr->log_sel) { /* Log action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->log_data;
		fields_used++;
	}

	if (pr->rmk_sel) { /* Remark action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->rmk_data;
		fields_used++;
	}

	if (pr->meter_sel) { /* Meter action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->meter_data;
		fields_used++;
	}

	if (pr->tagst_sel) { /* Egress Tag Status action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->tagst_data;
		fields_used++;
	}

	if (pr->mir_sel) { /* Mirror action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->mir_data;
		fields_used++;
	}

	if (pr->nopri_sel) { /* Normal Priority action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->nopri_data;
		fields_used++;
	}

	if (pr->cpupri_sel) { /* CPU Priority action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->nopri_data;
		fields_used++;
	}

	if (pr->otpid_sel) { /* OTPID action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->otpid_data;
		fields_used++;
	}

	if (pr->itpid_sel) { /* ITPID action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->itpid_data;
		fields_used++;
	}

	if (pr->shaper_sel) { /* Traffic shaper action */
		if (fields_used >= 4)
			return -1;
		*aif-- = pr->shaper_data;
		fields_used++;
	}

	return 0;
}

static void rtl838x_read_pie_action(u32 r[],  struct pie_rule *pr)
{
	u16 *aif = (u16 *)&r[17];

	aif--;

	pr_debug("%s, at %08x\n", __func__, (u32)aif);
	if (pr->drop)
		pr_debug("%s: Action Drop: %d", __func__, pr->drop);

	if (pr->fwd_sel) { /* Forwarding action */
		pr->fwd_act = *aif >> 13;
		pr->fwd_data = *aif--;
		pr->bypass_all = pr->fwd_data & BIT(12);
		pr->bypass_ibc_sc = pr->fwd_data & BIT(11);
		pr->bypass_igr_stp = pr->fwd_data & BIT(10);
		if (pr->bypass_all || pr->bypass_ibc_sc || pr->bypass_igr_stp)
			pr->bypass_sel = true;
	}
	if (pr->ovid_sel) /* Outer VID action */
		pr->ovid_data = *aif--;
	if (pr->ivid_sel) /* Inner VID action */
		pr->ivid_data = *aif--;
	if (pr->flt_sel) /* Filter action */
		pr->flt_data = *aif--;
	if (pr->log_sel) /* Log action */
		pr->log_data = *aif--;
	if (pr->rmk_sel) /* Remark action */
		pr->rmk_data = *aif--;
	if (pr->meter_sel) /* Meter action */
		pr->meter_data = *aif--;
	if (pr->tagst_sel) /* Egress Tag Status action */
		pr->tagst_data = *aif--;
	if (pr->mir_sel) /* Mirror action */
		pr->mir_data = *aif--;
	if (pr->nopri_sel) /* Normal Priority action */
		pr->nopri_data = *aif--;
	if (pr->cpupri_sel) /* CPU Priority action */
		pr->nopri_data = *aif--;
	if (pr->otpid_sel) /* OTPID action */
		pr->otpid_data = *aif--;
	if (pr->itpid_sel) /* ITPID action */
		pr->itpid_data = *aif--;
	if (pr->shaper_sel) /* Traffic shaper action */
		pr->shaper_data = *aif--;
}

static void rtl838x_pie_rule_dump_raw(u32 r[])
{
	pr_debug("Raw IACL table entry:\n");
	pr_debug("Match  : %08x %08x %08x %08x %08x %08x\n", r[0], r[1], r[2], r[3], r[4], r[5]);
	pr_debug("Fixed  : %08x\n", r[6]);
	pr_debug("Match M: %08x %08x %08x %08x %08x %08x\n", r[7], r[8], r[9], r[10], r[11], r[12]);
	pr_debug("Fixed M: %08x\n", r[13]);
	pr_debug("AIF    : %08x %08x %08x\n", r[14], r[15], r[16]);
	pr_debug("Sel    : %08x\n", r[17]);
}

// Currently not used
// static void rtl838x_pie_rule_dump(struct  pie_rule *pr)
// {
// 	pr_debug("Drop: %d, fwd: %d, ovid: %d, ivid: %d, flt: %d, log: %d, rmk: %d, meter: %d tagst: %d, mir: %d, nopri: %d, cpupri: %d, otpid: %d, itpid: %d, shape: %d\n",
// 		pr->drop, pr->fwd_sel, pr->ovid_sel, pr->ivid_sel, pr->flt_sel, pr->log_sel, pr->rmk_sel, pr->log_sel, pr->tagst_sel, pr->mir_sel, pr->nopri_sel,
// 		pr->cpupri_sel, pr->otpid_sel, pr->itpid_sel, pr->shaper_sel);
// 	if (pr->fwd_sel)
// 		pr_debug("FWD: %08x\n", pr->fwd_data);
// 	pr_debug("TID: %x, %x\n", pr->tid, pr->tid_m);
// }

static int rtl838x_pie_rule_read(struct rtl838x_switch_priv *priv, int idx, struct  pie_rule *pr)
{
	/* Read IACL table (1) via register 0 */
	struct table_reg *q = rtl_table_get(RTL8380_TBL_0, 1);
	u32 r[18];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL838X_ACL_BLK_TMPLTE_CTRL(block));

	memset(pr, 0, sizeof(*pr));
	rtl_table_read(q, idx);
	for (int i = 0; i < 18; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl838x_read_pie_fixed_fields(r, pr);
	if (!pr->valid)
		return 0;

	pr_debug("%s: template_selectors %08x, tid: %d\n", __func__, t_select, pr->tid);
	rtl838x_pie_rule_dump_raw(r);

	rtl838x_read_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 3)) & 0x7]);

	rtl838x_read_pie_action(r, pr);

	return 0;
}

static int rtl838x_pie_rule_write(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr)
{
	/* Access IACL table (1) via register 0 */
	struct table_reg *q = rtl_table_get(RTL8380_TBL_0, 1);
	u32 r[18];
	int err;
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL838X_ACL_BLK_TMPLTE_CTRL(block));

	pr_debug("%s: %d, t_select: %08x\n", __func__, idx, t_select);

	for (int i = 0; i < 18; i++)
		r[i] = 0;

	if (!pr->valid) {
		err = -EINVAL;
		pr_err("Rule invalid\n");
		goto errout;
	}

	rtl838x_write_pie_fixed_fields(r, pr);

	pr_debug("%s: template %d\n", __func__, (t_select >> (pr->tid * 3)) & 0x7);
	rtl838x_write_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 3)) & 0x7]);

	err = rtl838x_write_pie_action(r, pr);
	if (err) {
		pr_err("Rule actions too complex\n");
		goto errout;
	}

/*	rtl838x_pie_rule_dump_raw(r); */

	for (int i = 0; i < 18; i++)
		sw_w32(r[i], rtl_table_data(q, i));

errout:
	rtl_table_write(q, idx);
	rtl_table_release(q);

	return err;
}

static bool rtl838x_pie_templ_has(int t, enum template_field_id field_type)
{
	enum template_field_id ft;

	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		ft = fixed_templates[t][i];
		if (field_type == ft)
			return true;
	}

	return false;
}

static int rtl838x_pie_verify_template(struct rtl838x_switch_priv *priv,
				       struct pie_rule *pr, int t, int block)
{
	int i;

	if (!pr->is_ipv6 && pr->sip_m && !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_SIP0))
		return -1;

	if (!pr->is_ipv6 && pr->dip_m && !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_DIP0))
		return -1;

	if (pr->is_ipv6) {
		if ((pr->sip6_m.s6_addr32[0] ||
		     pr->sip6_m.s6_addr32[1] ||
		     pr->sip6_m.s6_addr32[2] ||
		     pr->sip6_m.s6_addr32[3]) &&
		    !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_SIP2))
			return -1;
		if ((pr->dip6_m.s6_addr32[0] ||
		     pr->dip6_m.s6_addr32[1] ||
		     pr->dip6_m.s6_addr32[2] ||
		     pr->dip6_m.s6_addr32[3]) &&
		    !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_DIP2))
			return -1;
	}

	if (ether_addr_to_u64(pr->smac) && !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_SMAC0))
		return -1;

	if (ether_addr_to_u64(pr->dmac) && !rtl838x_pie_templ_has(t, TEMPLATE_FIELD_DMAC0))
		return -1;

	/* TODO: Check more */

	i = find_first_zero_bit(&priv->pie_use_bm[block * 4], PIE_BLOCK_SIZE);

	if (i >= PIE_BLOCK_SIZE)
		return -1;

	return i + PIE_BLOCK_SIZE * block;
}

static int rtl838x_pie_rule_add(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx, block, j;

	pr_debug("In %s\n", __func__);

	mutex_lock(&priv->pie_mutex);

	for (block = 0; block < priv->r->n_pie_blocks; block++) {
		for (j = 0; j < 3; j++) {
			int t = (sw_r32(RTL838X_ACL_BLK_TMPLTE_CTRL(block)) >> (j * 3)) & 0x7;

			pr_debug("Testing block %d, template %d, template id %d\n", block, j, t);
			idx = rtl838x_pie_verify_template(priv, pr, t, block);
			if (idx >= 0)
				break;
		}
		if (j < 3)
			break;
	}

	if (block >= priv->r->n_pie_blocks) {
		mutex_unlock(&priv->pie_mutex);
		return -EOPNOTSUPP;
	}

	pr_debug("Using block: %d, index %d, template-id %d\n", block, idx, j);
	set_bit(idx, priv->pie_use_bm);

	pr->valid = true;
	pr->tid = j;  /* Mapped to template number */
	pr->tid_m = 0x3;
	pr->id = idx;

	rtl838x_pie_lookup_enable(priv, idx);
	rtl838x_pie_rule_write(priv, idx, pr);

	mutex_unlock(&priv->pie_mutex);

	return 0;
}

static void rtl838x_pie_rule_rm(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx = pr->id;

	rtl838x_pie_rule_del(priv, idx, idx);
	clear_bit(idx, priv->pie_use_bm);
}

/* Initializes the Packet Inspection Engine:
 * powers it up, enables default matching templates for all blocks
 * and clears all rules possibly installed by u-boot
 */
static void rtl838x_pie_init(struct rtl838x_switch_priv *priv)
{
	u32 template_selectors;

	mutex_init(&priv->pie_mutex);

	/* Enable ACL lookup on all ports, including CPU_PORT */
	for (int i = 0; i <= priv->r->cpu_port; i++)
		sw_w32(1, RTL838X_ACL_PORT_LOOKUP_CTRL(i));

	/* Power on all PIE blocks */
	for (int i = 0; i < priv->r->n_pie_blocks; i++)
		sw_w32_mask(0, BIT(i), RTL838X_ACL_BLK_PWR_CTRL);

	/* Include IPG in metering */
	sw_w32(1, RTL838X_METER_GLB_CTRL);

	/* Delete all present rules */
	rtl838x_pie_rule_del(priv, 0, priv->r->n_pie_blocks * PIE_BLOCK_SIZE - 1);

	/* Routing bypasses source port filter */
	sw_w32_mask(0, 1, RTL838X_DMY_REG27);

	/* Enable predefined templates 0, 1 and 2 for even blocks */
	template_selectors = 0 | (1 << 3) | (2 << 6);
	for (int i = 0; i < 6; i += 2)
		sw_w32(template_selectors, RTL838X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 0, 3 and 4 (IPv6 support) for odd blocks */
	template_selectors = 0 | (3 << 3) | (4 << 6);
	for (int i = 1; i < priv->r->n_pie_blocks; i += 2)
		sw_w32(template_selectors, RTL838X_ACL_BLK_TMPLTE_CTRL(i));

	/* Group each pair of physical blocks together to a logical block */
	sw_w32(0b10101010101, RTL838X_ACL_BLK_GROUP_CTRL);
}

static u32 rtl838x_packet_cntr_read(int counter)
{
	u32 v;

	/* Read LOG table (3) via register RTL8380_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 3);

	pr_debug("In %s, id %d\n", __func__, counter);
	rtl_table_read(r, counter / 2);

	pr_debug("Registers: %08x %08x\n",
		 sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)));
	/* The table has a size of 2 registers */
	if (counter % 2)
		v = sw_r32(rtl_table_data(r, 0));
	else
		v = sw_r32(rtl_table_data(r, 1));

	rtl_table_release(r);

	return v;
}

static void rtl838x_packet_cntr_clear(int counter)
{
	/* Access LOG table (3) via register RTL8380_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 3);

	pr_debug("In %s, id %d\n", __func__, counter);
	/* The table has a size of 2 registers */
	if (counter % 2)
		sw_w32(0, rtl_table_data(r, 0));
	else
		sw_w32(0, rtl_table_data(r, 1));

	rtl_table_write(r, counter / 2);

	rtl_table_release(r);
}

static void rtl838x_vlan_port_keep_tag_set(int port, bool keep_outer, bool keep_inner)
{
	sw_w32(FIELD_PREP(RTL838X_VLAN_PORT_TAG_STS_CTRL_OTAG_STS_MASK,
			  keep_outer ? RTL838X_VLAN_PORT_TAG_STS_TAGGED : RTL838X_VLAN_PORT_TAG_STS_UNTAG) |
	       FIELD_PREP(RTL838X_VLAN_PORT_TAG_STS_CTRL_ITAG_STS_MASK,
			  keep_inner ? RTL838X_VLAN_PORT_TAG_STS_TAGGED : RTL838X_VLAN_PORT_TAG_STS_UNTAG),
	       RTL838X_VLAN_PORT_TAG_STS_CTRL(port));
}

static void rtl838x_vlan_port_pvidmode_set(int port, enum pbvlan_type type, enum pbvlan_mode mode)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0x3, mode, RTL838X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0x3 << 14, mode << 14, RTL838X_VLAN_PORT_PB_VLAN + (port << 2));
}

static void rtl838x_vlan_port_pvid_set(int port, enum pbvlan_type type, int pvid)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0xfff << 2, pvid << 2, RTL838X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0xfff << 16, pvid << 16, RTL838X_VLAN_PORT_PB_VLAN + (port << 2));
}

static int rtldsa_838x_fast_age(struct rtl838x_switch_priv *priv, int port, int vid)
{
	u32 val;

	val = BIT(26) | BIT(23) | (port << 5);
	if (vid >= 0)
		val |= BIT(24) | (vid << 10);

	sw_w32(val, priv->r->l2_tbl_flush_ctrl);
	do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & BIT(26));

	return 0;
}

static int rtl838x_set_ageing_time(unsigned long msec)
{
	int t = sw_r32(RTL838X_L2_CTRL_1);

	t &= 0x7FFFFF;
	t = t * 128 / 625; /* Aging time in seconds. 0: L2 aging disabled */
	pr_debug("L2 AGING time: %d sec\n", t);

	t = (msec * 625 + 127000) / 128000;
	t = t > 0x7FFFFF ? 0x7FFFFF : t;
	sw_w32_mask(0x7FFFFF, t, RTL838X_L2_CTRL_1);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(RTL838X_L2_PORT_AGING_OUT));

	return 0;
}

static void rtl838x_set_igr_filter(int port, enum igr_filter state)
{
	sw_w32_mask(0x3 << ((port & 0xf) << 1), state << ((port & 0xf) << 1),
		    RTL838X_VLAN_PORT_IGR_FLTR + (((port >> 4) << 2)));
}

static void rtl838x_set_egr_filter(int port, enum egr_filter state)
{
	sw_w32_mask(0x1 << (port % 0x1d), state << (port % 0x1d),
		    RTL838X_VLAN_PORT_EGR_FLTR + (((port / 29) << 2)));
}

static int rtldsa_838x_set_distribution_algorithm(struct rtl838x_switch_priv *priv,
						  int group, int algoidx, u32 algomsk)
{
	algoidx &= 1; /* RTL838X only supports 2 concurrent algorithms */
	sw_w32_mask(1 << (group % 8), algoidx << (group % 8),
		    RTL838X_TRK_HASH_IDX_CTRL + ((group >> 3) << 2));
	sw_w32(algomsk, RTL838X_TRK_HASH_CTRL + (algoidx << 2));
	return 0;
}

static void rtl838x_set_receive_management_action(int port, rma_ctrl_t type, action_type_t action)
{
	switch (type) {
	case BPDU:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL838X_RMA_BPDU_CTRL + ((port >> 4) << 2));
		break;
	case PTP:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL838X_RMA_PTP_CTRL + ((port >> 4) << 2));
		break;
	case LLDP:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL838X_RMA_LLDP_CTRL + ((port >> 4) << 2));
		break;
	default:
		break;
	}
}

static void
rtldsa_838x_vlan_profile_dump(struct rtl838x_switch_priv *priv, int idx)
{
	struct rtldsa_vlan_profile p;

	if (rtldsa_838x_vlan_profile_get(idx, &p) < 0)
		return;

	dev_dbg(priv->dev,
		"VLAN profile %d: L2 learning: %d, UNKN L2MC FLD PMSK %d, UNKN IPMC FLD PMSK %d, UNKN IPv6MC FLD PMSK: %d\n", idx,
		p.l2_learn, p.unkn_mc_fld.pmsks_idx.l2,
		p.unkn_mc_fld.pmsks_idx.ip, p.unkn_mc_fld.pmsks_idx.ip6);
}

static int rtldsa_838x_lag_set_port_members(struct rtl838x_switch_priv *priv, int group,
					    u64 members, struct netdev_lag_upper_info *info)
{
	priv->lags_port_members[group] = members;

	priv->r->set_port_reg_be(priv->lags_port_members[group],
				 priv->r->trk_mbr_ctr(group));

	return 0;
}

int rtldsa_83xx_lag_setup_algomask(struct rtl838x_switch_priv *priv, int group,
				   struct netdev_lag_upper_info *info);

static void rtldsa_838x_stat_init(struct rtl838x_switch_priv *priv)
{
	/* Enable statistics module: all counters plus debug */
	sw_w32_mask(0, 3, RTL838X_STAT_CTRL);
}

const struct rtldsa_config rtldsa_838x_cfg = {
	.switch_ops = &rtldsa_83xx_switch_ops,
	.phylink_mac_ops = &rtldsa_83xx_phylink_mac_ops,
	.spanning_tree_ctrl = RTL838X_VLAN_STP_CTRL,
	.l2_bucket_size = 4,
	.n_mst = 64,
	.num_lag_ids = 8,
	.cpu_port = RTL838X_CPU_PORT,
	.fib_entries = 8192,
	.mask_port_reg_be = rtl838x_mask_port_reg,
	.set_port_reg_be = rtl838x_set_port_reg,
	.get_port_reg_be = rtl838x_get_port_reg,
	.mask_port_reg_le = rtl838x_mask_port_reg,
	.set_port_reg_le = rtl838x_set_port_reg,
	.get_port_reg_le = rtl838x_get_port_reg,
	.stat_port_rst = RTL838X_STAT_PORT_RST,
	.stat_rst = RTL838X_STAT_RST,
	.stat_init = rtldsa_838x_stat_init,
	.stat_port_std_mib = RTL838X_STAT_PORT_STD_MIB,
	.mib_desc = &rtldsa_838x_mib_desc,
	.stat_counters_lock = rtldsa_counters_lock_register,
	.stat_counters_unlock = rtldsa_counters_unlock_register,
	.stat_update_counters_atomically = rtldsa_update_counters_atomically,
	.stat_counter_poll_interval = RTLDSA_COUNTERS_POLL_INTERVAL,
	.port_iso_ctrl = rtl838x_port_iso_ctrl,
	.traffic_enable = rtl838x_traffic_enable,
	.traffic_disable = rtl838x_traffic_disable,
	.traffic_set = rtl838x_traffic_set,
	.l2_ctrl_0 = RTL838X_L2_CTRL_0,
	.l2_ctrl_1 = RTL838X_L2_CTRL_1,
	.high_res_l2_age = true,
	.self_mac_trap_ctrl = RTL838X_SPCL_TRAP_SWITCH_MAC_CTRL,
	.l2_port_aging_out = RTL838X_L2_PORT_AGING_OUT,
	.set_ageing_time = rtl838x_set_ageing_time,
	.l2_tbl_flush_ctrl = RTL838X_L2_TBL_FLUSH_CTRL,
	.isr_glb_src = RTL838X_ISR_GLB_SRC,
	.isr_port_link_sts_chg = RTL838X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL838X_IMR_PORT_LINK_STS_CHG,
	.imr_glb = RTL838X_IMR_GLB,
	.n_counters = 128,
	.n_pie_blocks = 12,
	.port_ignore = 0x1f,
	.vlan_tables_read = rtl838x_vlan_tables_read,
	.vlan_set_tagged = rtl838x_vlan_set_tagged,
	.vlan_set_untagged = rtl838x_vlan_set_untagged,
	.mac_force_mode_mask = RTL83XX_FORCE_EN | RTL83XX_FORCE_LINK_EN,
	.mac_force_mode_ctrl = rtl838x_mac_force_mode_ctrl,
	.mac_link_sts = RTL838X_MAC_LINK_STS,
	.vlan_profile_get = rtldsa_838x_vlan_profile_get,
	.vlan_profile_dump = rtldsa_838x_vlan_profile_dump,
	.vlan_profile_setup = rtl838x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl838x_vlan_fwd_on_inner,
	.set_vlan_igr_filter = rtl838x_set_igr_filter,
	.set_vlan_egr_filter = rtl838x_set_egr_filter,
	.enable_learning = rtl838x_enable_learning,
	.enable_flood = rtl838x_enable_flood,
	.enable_mcast_flood = rtl838x_enable_mcast_flood,
	.enable_bcast_flood = rtl838x_enable_bcast_flood,
	.set_static_move_action = rtl838x_set_static_move_action,
	.stp_get = rtldsa_838x_stp_get,
	.stp_set = rtl838x_stp_set,
	.mac_port_ctrl = rtl838x_mac_port_ctrl,
	.l2_port_new_salrn = rtl838x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl838x_l2_port_new_sa_fwd,
	.get_mirror_config = rtldsa_838x_get_mirror_config,
	.print_matrix = rtldsa_838x_print_matrix,
	.read_l2_entry_using_hash = rtl838x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl838x_write_l2_entry_using_hash,
	.read_cam = rtl838x_read_cam,
	.write_cam = rtl838x_write_cam,
	.vlan_port_keep_tag_set = rtl838x_vlan_port_keep_tag_set,
	.vlan_port_pvidmode_set = rtl838x_vlan_port_pvidmode_set,
	.vlan_port_pvid_set = rtl838x_vlan_port_pvid_set,
	.fast_age = rtldsa_838x_fast_age,
	.trk_mbr_ctr = rtl838x_trk_mbr_ctr,
	.rma_bpdu_fld_pmask = RTL838X_RMA_BPDU_FLD_PMSK,
	.spcl_trap_eapol_ctrl = RTL838X_SPCL_TRAP_EAPOL_CTRL,
	.init_eee = rtl838x_init_eee,
	.set_mac_eee = rtldsa_838x_set_mac_eee,
	.l2_hash_seed = rtl838x_l2_hash_seed,
	.l2_hash_key = rtl838x_l2_hash_key,
	.read_mcast_pmask = rtl838x_read_mcast_pmask,
	.write_mcast_pmask = rtl838x_write_mcast_pmask,
	.pie_init = rtl838x_pie_init,
	.pie_rule_read = rtl838x_pie_rule_read,
	.pie_rule_write = rtl838x_pie_rule_write,
	.pie_rule_add = rtl838x_pie_rule_add,
	.pie_rule_rm = rtl838x_pie_rule_rm,
	.l2_learning_setup = rtl838x_l2_learning_setup,
	.packet_cntr_read = rtl838x_packet_cntr_read,
	.packet_cntr_clear = rtl838x_packet_cntr_clear,
	.set_receive_management_action = rtl838x_set_receive_management_action,
	.get_egress_rate = rtldsa_838x_get_egress_rate,
	.set_egress_rate = rtldsa_838x_set_egress_rate,
	.qos_init = rtldsa_838x_qos_init,
	.lag_set_distribution_algorithm = rtldsa_838x_set_distribution_algorithm,
	.lag_set_port_members = rtldsa_838x_lag_set_port_members,
	.lag_setup_algomask = rtldsa_83xx_lag_setup_algomask,
};
