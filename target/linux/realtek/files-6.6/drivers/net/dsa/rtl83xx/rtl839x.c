// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include <linux/etherdevice.h>

#include "rtl83xx.h"

#define RTL839X_VLAN_PORT_TAG_STS_UNTAG				0x0
#define RTL839X_VLAN_PORT_TAG_STS_TAGGED			0x1
#define RTL839X_VLAN_PORT_TAG_STS_PRIORITY_TAGGED		0x2

#define RTL839X_VLAN_PORT_TAG_STS_CTRL_BASE			0x6828
/* port 0-52 */
#define RTL839X_VLAN_PORT_TAG_STS_CTRL(port) \
		RTL839X_VLAN_PORT_TAG_STS_CTRL_BASE + (port << 2)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_OTAG_STS_MASK		GENMASK(7,6)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_ITAG_STS_MASK		GENMASK(5,4)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_EGR_P_OTAG_KEEP_MASK	GENMASK(3,3)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_EGR_P_ITAG_KEEP_MASK	GENMASK(2,2)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_IGR_P_OTAG_KEEP_MASK	GENMASK(1,1)
#define RTL839X_VLAN_PORT_TAG_STS_CTRL_IGR_P_ITAG_KEEP_MASK	GENMASK(0,0)

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;

/* Definition of the RTL839X-specific template field IDs as used in the PIE */
enum template_field_id {
	TEMPLATE_FIELD_SPMMASK = 0,
	TEMPLATE_FIELD_SPM0 = 1,		/* Source portmask ports 0-15 */
	TEMPLATE_FIELD_SPM1 = 2,		/* Source portmask ports 16-31 */
	TEMPLATE_FIELD_SPM2 = 3,		/* Source portmask ports 32-47 */
	TEMPLATE_FIELD_SPM3 = 4,		/* Source portmask ports 48-56 */
	TEMPLATE_FIELD_DMAC0 = 5,		/* Destination MAC [15:0] */
	TEMPLATE_FIELD_DMAC1 = 6,		/* Destination MAC [31:16] */
	TEMPLATE_FIELD_DMAC2 = 7,		/* Destination MAC [47:32] */
	TEMPLATE_FIELD_SMAC0 = 8,		/* Source MAC [15:0] */
	TEMPLATE_FIELD_SMAC1 = 9,		/* Source MAC [31:16] */
	TEMPLATE_FIELD_SMAC2 = 10,		/* Source MAC [47:32] */
	TEMPLATE_FIELD_ETHERTYPE = 11,		/* Ethernet frame type field */
	/* Field-ID 12 is not used */
	TEMPLATE_FIELD_OTAG = 13,
	TEMPLATE_FIELD_ITAG = 14,
	TEMPLATE_FIELD_SIP0 = 15,
	TEMPLATE_FIELD_SIP1 = 16,
	TEMPLATE_FIELD_DIP0 = 17,
	TEMPLATE_FIELD_DIP1 = 18,
	TEMPLATE_FIELD_IP_TOS_PROTO = 19,
	TEMPLATE_FIELD_IP_FLAG = 20,
	TEMPLATE_FIELD_L4_SPORT = 21,
	TEMPLATE_FIELD_L4_DPORT = 22,
	TEMPLATE_FIELD_L34_HEADER = 23,
	TEMPLATE_FIELD_ICMP_IGMP = 24,
	TEMPLATE_FIELD_VID_RANG0 = 25,
	TEMPLATE_FIELD_VID_RANG1 = 26,
	TEMPLATE_FIELD_L4_PORT_RANG = 27,
	TEMPLATE_FIELD_FIELD_SELECTOR_VALID = 28,
	TEMPLATE_FIELD_FIELD_SELECTOR_0 = 29,
	TEMPLATE_FIELD_FIELD_SELECTOR_1 = 30,
	TEMPLATE_FIELD_FIELD_SELECTOR_2 = 31,
	TEMPLATE_FIELD_FIELD_SELECTOR_3 = 32,
	TEMPLATE_FIELD_FIELD_SELECTOR_4 = 33,
	TEMPLATE_FIELD_FIELD_SELECTOR_5 = 34,
	TEMPLATE_FIELD_SIP2 = 35,
	TEMPLATE_FIELD_SIP3 = 36,
	TEMPLATE_FIELD_SIP4 = 37,
	TEMPLATE_FIELD_SIP5 = 38,
	TEMPLATE_FIELD_SIP6 = 39,
	TEMPLATE_FIELD_SIP7 = 40,
	TEMPLATE_FIELD_OLABEL = 41,
	TEMPLATE_FIELD_ILABEL = 42,
	TEMPLATE_FIELD_OILABEL = 43,
	TEMPLATE_FIELD_DPMMASK = 44,
	TEMPLATE_FIELD_DPM0 = 45,
	TEMPLATE_FIELD_DPM1 = 46,
	TEMPLATE_FIELD_DPM2 = 47,
	TEMPLATE_FIELD_DPM3 = 48,
	TEMPLATE_FIELD_L2DPM0 = 49,
	TEMPLATE_FIELD_L2DPM1 = 50,
	TEMPLATE_FIELD_L2DPM2 = 51,
	TEMPLATE_FIELD_L2DPM3 = 52,
	TEMPLATE_FIELD_IVLAN = 53,
	TEMPLATE_FIELD_OVLAN = 54,
	TEMPLATE_FIELD_FWD_VID = 55,
	TEMPLATE_FIELD_DIP2 = 56,
	TEMPLATE_FIELD_DIP3 = 57,
	TEMPLATE_FIELD_DIP4 = 58,
	TEMPLATE_FIELD_DIP5 = 59,
	TEMPLATE_FIELD_DIP6 = 60,
	TEMPLATE_FIELD_DIP7 = 61,
};

/* Number of fixed templates predefined in the SoC */
#define N_FIXED_TEMPLATES 5
static enum template_field_id fixed_templates[N_FIXED_TEMPLATES][N_FIXED_FIELDS] =
{
	{
	  TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1, TEMPLATE_FIELD_ITAG,
	  TEMPLATE_FIELD_SMAC0, TEMPLATE_FIELD_SMAC1, TEMPLATE_FIELD_SMAC2,
	  TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
	  TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	}, {
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
	  TEMPLATE_FIELD_DIP1,TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_L4_SPORT,
	  TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_ICMP_IGMP, TEMPLATE_FIELD_SPM0,
	  TEMPLATE_FIELD_SPM1, TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
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
	  TEMPLATE_FIELD_SIP6, TEMPLATE_FIELD_SIP7, TEMPLATE_FIELD_SPM0,
	  TEMPLATE_FIELD_SPM1, TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	},
};

void rtl839x_print_matrix(void)
{
	volatile u64 *ptr9;

	ptr9 = RTL838X_SW_BASE + RTL839X_PORT_ISO_CTRL(0);
	for (int i = 0; i < 52; i += 4)
		pr_debug("> %16llx %16llx %16llx %16llx\n",
			ptr9[i + 0], ptr9[i + 1], ptr9[i + 2], ptr9[i + 3]);
	pr_debug("CPU_PORT> %16llx\n", ptr9[52]);
}

static inline int rtl839x_port_iso_ctrl(int p)
{
	return RTL839X_PORT_ISO_CTRL(p);
}

static inline void rtl839x_exec_tbl0_cmd(u32 cmd)
{
	sw_w32(cmd, RTL839X_TBL_ACCESS_CTRL_0);
	do { } while (sw_r32(RTL839X_TBL_ACCESS_CTRL_0) & BIT(16));
}

static inline void rtl839x_exec_tbl1_cmd(u32 cmd)
{
	sw_w32(cmd, RTL839X_TBL_ACCESS_CTRL_1);
	do { } while (sw_r32(RTL839X_TBL_ACCESS_CTRL_1) & BIT(16));
}

inline void rtl839x_exec_tbl2_cmd(u32 cmd)
{
	sw_w32(cmd, RTL839X_TBL_ACCESS_CTRL_2);
	do { } while (sw_r32(RTL839X_TBL_ACCESS_CTRL_2) & (1 << 9));
}

static inline int rtl839x_tbl_access_data_0(int i)
{
	return RTL839X_TBL_ACCESS_DATA_0(i);
}

static void rtl839x_vlan_tables_read(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 u, v, w;
	/* Read VLAN table (0) via register 0 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_0, 0);

	rtl_table_read(r, vlan);
	u = sw_r32(rtl_table_data(r, 0));
	v = sw_r32(rtl_table_data(r, 1));
	w = sw_r32(rtl_table_data(r, 2));
	rtl_table_release(r);

	info->tagged_ports = u;
	info->tagged_ports = (info->tagged_ports << 21) | ((v >> 11) & 0x1fffff);
	info->profile_id = w >> 30 | ((v & 1) << 2);
	info->hash_mc_fid = !!(w & BIT(2));
	info->hash_uc_fid = !!(w & BIT(3));
	info->fid = (v >> 3) & 0xff;

	/* Read UNTAG table (0) via table register 1 */
	r = rtl_table_get(RTL8390_TBL_1, 0);
	rtl_table_read(r, vlan);
	u = sw_r32(rtl_table_data(r, 0));
	v = sw_r32(rtl_table_data(r, 1));
	rtl_table_release(r);

	info->untagged_ports = u;
	info->untagged_ports = (info->untagged_ports << 21) | ((v >> 11) & 0x1fffff);
}

static void rtl839x_vlan_set_tagged(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 u, v, w;
	/* Access VLAN table (0) via register 0 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_0, 0);

	u = info->tagged_ports >> 21;
	v = info->tagged_ports << 11;
	v |= ((u32)info->fid) << 3;
	v |= info->hash_uc_fid ? BIT(2) : 0;
	v |= info->hash_mc_fid ? BIT(1) : 0;
	v |= (info->profile_id & 0x4) ? 1 : 0;
	w = ((u32)(info->profile_id & 3)) << 30;

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));
	sw_w32(w, rtl_table_data(r, 2));

	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static void rtl839x_vlan_set_untagged(u32 vlan, u64 portmask)
{
	u32 u, v;

	/* Access UNTAG table (0) via table register 1 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 0);

	u = portmask >> 21;
	v = portmask << 11;

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));
	rtl_table_write(r, vlan);

	rtl_table_release(r);
}

/* Sets the L2 forwarding to be based on either the inner VLAN tag or the outer */
static void rtl839x_vlan_fwd_on_inner(int port, bool is_set)
{
	if (is_set)
		rtl839x_mask_port_reg_be(BIT_ULL(port), 0ULL, RTL839X_VLAN_PORT_FWD);
	else
		rtl839x_mask_port_reg_be(0ULL, BIT_ULL(port), RTL839X_VLAN_PORT_FWD);
}

/* Hash seed is vid (actually rvid) concatenated with the MAC address */
static u64 rtl839x_l2_hash_seed(u64 mac, u32 vid)
{
	u64 v = vid;

	v <<= 48;
	v |= mac;

	return v;
}

/* Applies the same hash algorithm as the one used currently by the ASIC to the seed
 * and returns a key into the L2 hash table
 */
static u32 rtl839x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 h1, h2, h;

	if (sw_r32(priv->r->l2_ctrl_0) & 1) {
		h1 = (u32) (((seed >> 60) & 0x3f) ^ ((seed >> 54) & 0x3f) ^
		           ((seed >> 36) & 0x3f) ^ ((seed >> 30) & 0x3f) ^
		           ((seed >> 12) & 0x3f) ^ ((seed >> 6) & 0x3f));
		h2 = (u32) (((seed >> 48) & 0x3f) ^ ((seed >> 42) & 0x3f) ^
		           ((seed >> 24) & 0x3f) ^ ((seed >> 18) & 0x3f) ^
		           (seed & 0x3f));
		h = (h1 << 6) | h2;
	} else {
		h = (seed >> 60) ^
		    ((((seed >> 48) & 0x3f) << 6) | ((seed >> 54) & 0x3f)) ^
		    ((seed >> 36) & 0xfff) ^ ((seed >> 24) & 0xfff) ^
		    ((seed >> 12) & 0xfff) ^ (seed & 0xfff);
	}

	return h;
}

static inline int rtl839x_mac_force_mode_ctrl(int p)
{
	return RTL839X_MAC_FORCE_MODE_CTRL + (p << 2);
}

static inline int rtl839x_mac_port_ctrl(int p)
{
	return RTL839X_MAC_PORT_CTRL(p);
}

static inline int rtl839x_l2_port_new_salrn(int p)
{
	return RTL839X_L2_PORT_NEW_SALRN(p);
}

static inline int rtl839x_l2_port_new_sa_fwd(int p)
{
	return RTL839X_L2_PORT_NEW_SA_FWD(p);
}

static inline int rtl839x_mac_link_spd_sts(int p)
{
	return RTL839X_MAC_LINK_SPD_STS(p);
}

static inline int rtl839x_trk_mbr_ctr(int group)
{
	return RTL839X_TRK_MBR_CTR + (group << 3);
}

static void rtl839x_fill_l2_entry(u32 r[], struct rtl838x_l2_entry *e)
{
	/* Table contains different entry types, we need to identify the right one:
	 * Check for MC entries, first
	 */
	e->is_ip_mc = !!(r[2] & BIT(31));
	e->is_ipv6_mc = !!(r[2] & BIT(30));
	e->type = L2_INVALID;
	if (!e->is_ip_mc && !e->is_ipv6_mc) {
		e->mac[0] = (r[0] >> 12);
		e->mac[1] = (r[0] >> 4);
		e->mac[2] = ((r[1] >> 28) | (r[0] << 4));
		e->mac[3] = (r[1] >> 20);
		e->mac[4] = (r[1] >> 12);
		e->mac[5] = (r[1] >> 4);

		e->vid = (r[2] >> 4) & 0xfff;
		e->rvid = (r[0] >> 20) & 0xfff;

		/* Is it a unicast entry? check multicast bit */
		if (!(e->mac[0] & 1)) {
			e->is_static = !!((r[2] >> 18) & 1);
			e->port = (r[2] >> 24) & 0x3f;
			e->block_da = !!(r[2] & (1 << 19));
			e->block_sa = !!(r[2] & (1 << 20));
			e->suspended = !!(r[2] & (1 << 17));
			e->next_hop = !!(r[2] & (1 << 16));
			if (e->next_hop) {
				pr_debug("Found next hop entry, need to read data\n");
				e->nh_vlan_target = !!(r[2] & BIT(15));
				e->nh_route_id = (r[2] >> 4) & 0x1ff;
				e->vid = e->rvid;
			}
			e->age = (r[2] >> 21) & 3;
			e->valid = true;
			if (!(r[2] & 0xc0fd0000)) /* Check for valid entry */
				e->valid = false;
			else
				e->type = L2_UNICAST;
		} else {
			e->valid = true;
			e->type = L2_MULTICAST;
			e->mc_portmask_index = (r[2] >> 6) & 0xfff;
			e->vid = e->rvid;
		}
	} else { /* IPv4 and IPv6 multicast */
		e->vid = e->rvid = (r[0] << 20) & 0xfff;
		e->mc_gip = r[1];
		e->mc_portmask_index = (r[2] >> 6) & 0xfff;
	}
	if (e->is_ip_mc) {
		e->valid = true;
		e->type = IP4_MULTICAST;
	}
	if (e->is_ipv6_mc) {
		e->valid = true;
		e->type = IP6_MULTICAST;
	}
	/* pr_info("%s: vid %d, rvid: %d\n", __func__, e->vid, e->rvid); */
}

/* Fills the 3 SoC table registers r[] with the information in the rtl838x_l2_entry */
static void rtl839x_fill_l2_row(u32 r[], struct rtl838x_l2_entry *e)
{
	if (!e->valid) {
		r[0] = r[1] = r[2] = 0;
		return;
	}

	r[2] = e->is_ip_mc ? BIT(31) : 0;
	r[2] |= e->is_ipv6_mc ? BIT(30) : 0;

	if (!e->is_ip_mc  && !e->is_ipv6_mc) {
		r[0] = ((u32)e->mac[0]) << 12;
		r[0] |= ((u32)e->mac[1]) << 4;
		r[0] |= ((u32)e->mac[2]) >> 4;
		r[1] = ((u32)e->mac[2]) << 28;
		r[1] |= ((u32)e->mac[3]) << 20;
		r[1] |= ((u32)e->mac[4]) << 12;
		r[1] |= ((u32)e->mac[5]) << 4;

		if (!(e->mac[0] & 1)) { /* Not multicast */
			r[2] |= e->is_static ? BIT(18) : 0;
			r[0] |= ((u32)e->rvid) << 20;
			r[2] |= e->port << 24;
			r[2] |= e->block_da ? BIT(19) : 0;
			r[2] |= e->block_sa ? BIT(20) : 0;
			r[2] |= e->suspended ? BIT(17) : 0;
			r[2] |= ((u32)e->age) << 21;
			if (e->next_hop) {
				r[2] |= BIT(16);
				r[2] |= e->nh_vlan_target ? BIT(15) : 0;
				r[2] |= (e->nh_route_id & 0x7ff) << 4;
			} else {
				r[2] |= e->vid << 4;
			}
			pr_debug("Write L2 NH: %08x %08x %08x\n", r[0], r[1], r[2]);
		} else {  /* L2 Multicast */
			r[0] |= ((u32)e->rvid) << 20;
			r[2] |= ((u32)e->mc_portmask_index) << 6;
		}
	} else { /* IPv4 or IPv6 MC entry */
		r[0] = ((u32)e->rvid) << 20;
		r[1] = e->mc_gip;
		r[2] |= ((u32)e->mc_portmask_index) << 6;
	}
}

/* Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl839x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 0);
	u32 idx = (0 << 14) | (hash << 2) | pos; /* Search SRAM, with hash and at pos in bucket */

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl839x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	return rtl839x_l2_hash_seed(ether_addr_to_u64(&e->mac[0]), e->rvid);
}

static void rtl839x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 0);

	u32 idx = (0 << 14) | (hash << 2) | pos; /* Access SRAM, with hash and at pos in bucket */

	rtl839x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl839x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 1); /* Access L2 Table 1 */

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl839x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	pr_debug("Found in CAM: R1 %x R2 %x R3 %x\n", r[0], r[1], r[2]);

	/* Return MAC with concatenated VID ac concatenated ID */
	return rtl839x_l2_hash_seed(ether_addr_to_u64(&e->mac[0]), e->rvid);
}

static void rtl839x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 1); /* Access L2 Table 1 */

	rtl839x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl839x_read_mcast_pmask(int idx)
{
	u64 portmask;
	/* Read MC_PMSK (2) via register RTL8390_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	portmask <<= 32;
	portmask |= sw_r32(rtl_table_data(q, 1));
	portmask >>= 11;  /* LSB is bit 11 in data registers */
	rtl_table_release(q);

	return portmask;
}

static void rtl839x_write_mcast_pmask(int idx, u64 portmask)
{
	/* Access MC_PMSK (2) via register RTL8380_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 2);

	portmask <<= 11; /* LSB is bit 11 in data registers */
	sw_w32((u32)(portmask >> 32), rtl_table_data(q, 0));
	sw_w32((u32)((portmask & 0xfffff800)), rtl_table_data(q, 1));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static void rtl839x_vlan_profile_setup(int profile)
{
	u32 p[2];
	u32 pmask_id = UNKNOWN_MC_PMASK;

	p[0] = pmask_id; /* Use portmaks 0xfff for unknown IPv6 MC flooding */
	/* Enable L2 Learning BIT 0, portmask UNKNOWN_MC_PMASK for IP/L2-MC traffic flooding */
	p[1] = 1 | pmask_id << 1 | pmask_id << 13;

	sw_w32(p[0], RTL839X_VLAN_PROFILE(profile));
	sw_w32(p[1], RTL839X_VLAN_PROFILE(profile) + 4);

	rtl839x_write_mcast_pmask(UNKNOWN_MC_PMASK, 0x001fffffffffffff);
}

u64 rtl839x_traffic_get(int source)
{
	return rtl839x_get_port_reg_be(rtl839x_port_iso_ctrl(source));
}

void rtl839x_traffic_set(int source, u64 dest_matrix)
{
	rtl839x_set_port_reg_be(dest_matrix, rtl839x_port_iso_ctrl(source));
}

void rtl839x_traffic_enable(int source, int dest)
{
	rtl839x_mask_port_reg_be(0, BIT_ULL(dest), rtl839x_port_iso_ctrl(source));
}

void rtl839x_traffic_disable(int source, int dest)
{
	rtl839x_mask_port_reg_be(BIT_ULL(dest), 0, rtl839x_port_iso_ctrl(source));
}

static void rtl839x_l2_learning_setup(void)
{
	/* Set portmask for broadcast (offset bit 12) and unknown unicast (offset 0)
	 * address flooding to the reserved entry in the portmask table used
	 * also for multicast flooding */
	sw_w32(UNKNOWN_MC_PMASK << 12 | UNKNOWN_MC_PMASK, RTL839X_L2_FLD_PMSK);

	/* Limit learning to maximum: 32k entries, after that just flood (bits 0-1) */
	sw_w32((0x7fff << 2) | 0, RTL839X_L2_LRN_CONSTRT);

	/* Do not trap ARP packets to CPU_PORT */
	sw_w32(0, RTL839X_SPCL_TRAP_ARP_CTRL);
}

static void rtl839x_enable_learning(int port, bool enable)
{
	/* Limit learning to maximum: 32k entries */

	sw_w32_mask(0x7fff << 2, enable ? (0x7fff << 2) : 0,
		    RTL839X_L2_PORT_LRN_CONSTRT + (port << 2));
}

static void rtl839x_enable_flood(int port, bool enable)
{
	/* 0: Forward
	 * 1: Disable
	 * 2: to CPU
	 * 3: Copy to CPU
	 */
	sw_w32_mask(0x3, enable ? 0 : 1,
		    RTL839X_L2_PORT_LRN_CONSTRT + (port << 2));
}

static void rtl839x_enable_mcast_flood(int port, bool enable)
{

}

static void rtl839x_enable_bcast_flood(int port, bool enable)
{

}

static void rtl839x_set_static_move_action(int port, bool forward)
{
	int shift = MV_ACT_PORT_SHIFT(port);
	u32 val = forward ? MV_ACT_FORWARD : MV_ACT_DROP;

	sw_w32_mask(MV_ACT_MASK << shift, val << shift,
		    RTL839X_L2_PORT_STATIC_MV_ACT(port));
}

irqreturn_t rtl839x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 status = sw_r32(RTL839X_ISR_GLB_SRC);
	u64 ports = rtl839x_get_port_reg_le(RTL839X_ISR_PORT_LINK_STS_CHG);
	u64 link;

	/* Clear status */
	rtl839x_set_port_reg_le(ports, RTL839X_ISR_PORT_LINK_STS_CHG);
	pr_debug("RTL8390 Link change: status: %x, ports %llx\n", status, ports);

	for (int i = 0; i < RTL839X_CPU_PORT; i++) {
		if (ports & BIT_ULL(i)) {
			link = rtl839x_get_port_reg_le(RTL839X_MAC_LINK_STS);
			if (link & BIT_ULL(i))
				dsa_port_phylink_mac_change(ds, i, true);
			else
				dsa_port_phylink_mac_change(ds, i, false);
		}
	}

	return IRQ_HANDLED;
}

/* TODO: unused */
int rtl8390_sds_power(int mac, int val)
{
	u32 offset = (mac == 48) ? 0x0 : 0x100;
	u32 mode = val ? 0 : 1;

	pr_debug("In %s: mac %d, set %d\n", __func__, mac, val);

	if ((mac != 48) && (mac != 49)) {
		pr_err("%s: not an SFP port: %d\n", __func__, mac);
		return -1;
	}

	/* Set bit 1003. 1000 starts at 7c */
	sw_w32_mask(BIT(11), mode << 11, RTL839X_SDS12_13_PWR0 + offset);

	return 0;
}

static int rtl839x_smi_wait_op(int timeout)
{
	int ret = 0;
	u32 val;

	ret = readx_poll_timeout(sw_r32, RTL839X_PHYREG_ACCESS_CTRL,
				 val, !(val & 0x1), 20, timeout);
	if (ret)
		pr_err("%s: timeout\n", __func__);

	return ret;
}

int rtl839x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;
	int err = 0;

	if (port > 63 || page > 8191 || reg > 31)
		return -ENOTSUPP;

	/* Take bug on RTL839x Rev <= C into account */
	if (port >= RTL839X_CPU_PORT)
		return -EIO;

	mutex_lock(&smi_lock);

	sw_w32_mask(0xffff0000, port << 16, RTL839X_PHYREG_DATA_CTRL);
	v = reg << 5 | page << 10 | ((page == 0x1fff) ? 0x1f : 0) << 23;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	sw_w32(0x1ff, RTL839X_PHYREG_CTRL);

	v |= 1;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	err = rtl839x_smi_wait_op(100000);
	if (err)
		goto errout;

	*val = sw_r32(RTL839X_PHYREG_DATA_CTRL) & 0xffff;

errout:
	mutex_unlock(&smi_lock);

	return err;
}

int rtl839x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	int err = 0;

	val &= 0xffff;
	if (port > 63 || page > 8191 || reg > 31)
		return -ENOTSUPP;

	/* Take bug on RTL839x Rev <= C into account */
	if (port >= RTL839X_CPU_PORT)
		return -EIO;

	mutex_lock(&smi_lock);

	/* Set PHY to access */
	rtl839x_set_port_reg_le(BIT_ULL(port), RTL839X_PHYREG_PORT_CTRL);

	sw_w32_mask(0xffff0000, val << 16, RTL839X_PHYREG_DATA_CTRL);

	v = reg << 5 | page << 10 | ((page == 0x1fff) ? 0x1f : 0) << 23;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	sw_w32(0x1ff, RTL839X_PHYREG_CTRL);

	v |= BIT(3) | 1; /* Write operation and execute */
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	err = rtl839x_smi_wait_op(100000);
	if (err)
		goto errout;

	if (sw_r32(RTL839X_PHYREG_ACCESS_CTRL) & 0x2)
		err = -EIO;

errout:
	mutex_unlock(&smi_lock);

	return err;
}

/* Read an mmd register of the PHY */
int rtl839x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val)
{
	int err = 0;
	u32 v;

	/* Take bug on RTL839x Rev <= C into account */
	if (port >= RTL839X_CPU_PORT)
		return -EIO;

	mutex_lock(&smi_lock);

	/* Set PHY to access */
	sw_w32_mask(0xffff << 16, port << 16, RTL839X_PHYREG_DATA_CTRL);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL839X_PHYREG_MMD_CTRL);

	v = BIT(2) | BIT(0); /* MMD-access | EXEC */
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	err = rtl839x_smi_wait_op(100000);
	if (err)
		goto errout;

	/* There is no error-checking via BIT 1 of v, as it does not seem to be set correctly */
	*val = (sw_r32(RTL839X_PHYREG_DATA_CTRL) & 0xffff);
	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, *val, err);

errout:
	mutex_unlock(&smi_lock);

	return err;
}

/* Write to an mmd register of the PHY */
int rtl839x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val)
{
	int err = 0;
	u32 v;

	/* Take bug on RTL839x Rev <= C into account */
	if (port >= RTL839X_CPU_PORT)
		return -EIO;

	mutex_lock(&smi_lock);

	/* Set PHY to access */
	rtl839x_set_port_reg_le(BIT_ULL(port), RTL839X_PHYREG_PORT_CTRL);

	/* Set data to write */
	sw_w32_mask(0xffff << 16, val << 16, RTL839X_PHYREG_DATA_CTRL);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL839X_PHYREG_MMD_CTRL);

	v = BIT(3) | BIT(2) | BIT(0); /* WRITE | MMD-access | EXEC */
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	err = rtl839x_smi_wait_op(100000);
	if (err)
		goto errout;

	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, val, err);

errout:
	mutex_unlock(&smi_lock);

	return err;
}

void rtl8390_get_version(struct rtl838x_switch_priv *priv)
{
	u32 info, model;

	sw_w32_mask(0xf << 28, 0xa << 28, RTL839X_CHIP_INFO);
	info = sw_r32(RTL839X_CHIP_INFO);

	model = sw_r32(RTL839X_MODEL_NAME_INFO);
	priv->version = RTL8390_VERSION_A + ((model & 0x3f) >> 1);

	pr_info("RTL839X Chip-Info: %x, version %c\n", info, priv->version);
}

void rtl839x_vlan_profile_dump(int profile)
{
	u32 p[2];

	if (profile < 0 || profile > 7)
		return;

	p[0] = sw_r32(RTL839X_VLAN_PROFILE(profile));
	p[1] = sw_r32(RTL839X_VLAN_PROFILE(profile) + 4);

	pr_info("VLAN profile %d: L2 learning: %d, UNKN L2MC FLD PMSK %d, \
		UNKN IPMC FLD PMSK %d, UNKN IPv6MC FLD PMSK: %d",
		profile, p[1] & 1, (p[1] >> 1) & 0xfff, (p[1] >> 13) & 0xfff,
		(p[0]) & 0xfff);
	pr_info("VLAN profile %d: raw %08x, %08x\n", profile, p[0], p[1]);
}

static void rtl839x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 16 | /* Execute cmd */
	          0 << 15 | /* Read */
	          5 << 12 | /* Table type 0b101 */
	          (msti & 0xfff);
	priv->r->exec_tbl0_cmd(cmd);

	for (int i = 0; i < 4; i++)
		port_state[i] = sw_r32(priv->r->tbl_access_data_0(i));
}

static void rtl839x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 16 | /* Execute cmd */
	          1 << 15 | /* Write */
	          5 << 12 | /* Table type 0b101 */
	          (msti & 0xfff);
	for (int i = 0; i < 4; i++)
		sw_w32(port_state[i], priv->r->tbl_access_data_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

/* Enables or disables the EEE/EEEP capability of a port */
void rtl839x_port_eee_set(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;

	/* This works only for Ethernet ports, and on the RTL839X, ports above 47 are SFP */
	if (port >= 48)
		return;

	enable = true;
	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? 0xf : 0x0;

	/* Set EEE for 100, 500, 1000MBit and 10GBit */
	sw_w32_mask(0xf << 8, v << 8, rtl839x_mac_force_mode_ctrl(port));

	/* Set TX/RX EEE state */
	v = enable ? 0x3 : 0x0;
	sw_w32(v, RTL839X_EEE_CTRL(port));

	priv->ports[port].eee_enabled = enable;
}

/* Get EEE own capabilities and negotiation result */
int rtl839x_eee_port_ability(struct rtl838x_switch_priv *priv, struct ethtool_eee *e, int port)
{
	u64 link, a;

	if (port >= 48)
		return 0;

	link = rtl839x_get_port_reg_le(RTL839X_MAC_LINK_STS);
	if (!(link & BIT_ULL(port)))
		return 0;

	if (sw_r32(rtl839x_mac_force_mode_ctrl(port)) & BIT(8))
		e->advertised |= ADVERTISED_100baseT_Full;

	if (sw_r32(rtl839x_mac_force_mode_ctrl(port)) & BIT(10))
		e->advertised |= ADVERTISED_1000baseT_Full;

	a = rtl839x_get_port_reg_le(RTL839X_MAC_EEE_ABLTY);
	pr_info("Link partner: %016llx\n", a);
	if (rtl839x_get_port_reg_le(RTL839X_MAC_EEE_ABLTY) & BIT_ULL(port)) {
		e->lp_advertised = ADVERTISED_100baseT_Full;
		e->lp_advertised |= ADVERTISED_1000baseT_Full;
		return 1;
	}

	return 0;
}

static void rtl839x_init_eee(struct rtl838x_switch_priv *priv, bool enable)
{
	pr_info("Setting up EEE, state: %d\n", enable);

	/* Set wake timer for TX and pause timer both to 0x21 */
	sw_w32_mask(0xff << 20| 0xff, 0x21 << 20| 0x21, RTL839X_EEE_TX_TIMER_GELITE_CTRL);
	/* Set pause wake timer for GIGA-EEE to 0x11 */
	sw_w32_mask(0xff << 20, 0x11 << 20, RTL839X_EEE_TX_TIMER_GIGA_CTRL);
	/* Set pause wake timer for 10GBit ports to 0x11 */
	sw_w32_mask(0xff << 20, 0x11 << 20, RTL839X_EEE_TX_TIMER_10G_CTRL);

	/* Setup EEE on all ports */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			rtl839x_port_eee_set(priv, i, enable);
	}
	priv->eee_enabled = enable;
}

static void rtl839x_pie_lookup_enable(struct rtl838x_switch_priv *priv, int index)
{
	int block = index / PIE_BLOCK_SIZE;

	sw_w32_mask(0, BIT(block), RTL839X_ACL_BLK_LOOKUP_CTRL);
}

/* Delete a range of Packet Inspection Engine rules */
static int rtl839x_pie_rule_del(struct rtl838x_switch_priv *priv, int index_from, int index_to)
{
	u32 v = (index_from << 1)| (index_to << 13 ) | BIT(0);

	pr_debug("%s: from %d to %d\n", __func__, index_from, index_to);
	mutex_lock(&priv->reg_mutex);

	/* Write from-to and execute bit into control register */
	sw_w32(v, RTL839X_ACL_CLR_CTRL);

	/* Wait until command has completed */
	do {
	} while (sw_r32(RTL839X_ACL_CLR_CTRL) & BIT(0));

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

/* Reads the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure and fills in the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL9310 has 2 more registers / fields and the physical field-ids are different
 * on all SoCs
 * On the RTL8390 the template mask registers are not word-aligned!
 */
static void rtl839x_write_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
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
		case TEMPLATE_FIELD_SPM2:
			data = pr->spm >> 32;
			data_m = pr->spm_m >> 32;
			break;
		case TEMPLATE_FIELD_SPM3:
			data = pr->spm >> 48;
			data_m = pr->spm_m >> 48;
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
			pr_info("%s: unknown field %d\n", __func__, field_type);
		}

		/* On the RTL8390, the mask fields are not word aligned! */
		if (!(i % 2)) {
			r[5 - i / 2] = data;
			r[12 - i / 2] |= ((u32)data_m << 8);
		} else {
			r[5 - i / 2] |= ((u32)data) << 16;
			r[12 - i / 2] |= ((u32)data_m) << 24;
			r[11 - i / 2] |= ((u32)data_m) >> 8;
		}
	}
}

/* Creates the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure by reading the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL9310 has 2 more registers / fields and the physical field-ids
 * On the RTL8390 the template mask registers are not word-aligned!
 */
void rtl839x_read_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		enum template_field_id field_type = t[i];
		u16 data, data_m;

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
			pr_info("%s: unknown field %d\n", __func__, field_type);
		}
	}
}

static void rtl839x_read_pie_fixed_fields(u32 r[], struct pie_rule *pr)
{
	pr->spmmask_fix = (r[6] >> 30) & 0x3;
	pr->spn = (r[6] >> 24) & 0x3f;
	pr->mgnt_vlan = (r[6] >> 23) & 1;
	pr->dmac_hit_sw = (r[6] >> 22) & 1;
	pr->not_first_frag = (r[6] >> 21) & 1;
	pr->frame_type_l4 = (r[6] >> 18) & 7;
	pr->frame_type = (r[6] >> 16) & 3;
	pr->otag_fmt = (r[6] >> 15) & 1;
	pr->itag_fmt = (r[6] >> 14) & 1;
	pr->otag_exist = (r[6] >> 13) & 1;
	pr->itag_exist = (r[6] >> 12) & 1;
	pr->frame_type_l2 = (r[6] >> 10) & 3;
	pr->tid = (r[6] >> 8) & 3;

	pr->spmmask_fix_m = (r[12] >> 6) & 0x3;
	pr->spn_m = r[12]  & 0x3f;
	pr->mgnt_vlan_m = (r[13] >> 31) & 1;
	pr->dmac_hit_sw_m = (r[13] >> 30) & 1;
	pr->not_first_frag_m = (r[13] >> 29) & 1;
	pr->frame_type_l4_m = (r[13] >> 26) & 7;
	pr->frame_type_m = (r[13] >> 24) & 3;
	pr->otag_fmt_m = (r[13] >> 23) & 1;
	pr->itag_fmt_m = (r[13] >> 22) & 1;
	pr->otag_exist_m = (r[13] >> 21) & 1;
	pr->itag_exist_m = (r[13] >> 20) & 1;
	pr->frame_type_l2_m = (r[13] >> 18) & 3;
	pr->tid_m = (r[13] >> 16) & 3;

	pr->valid = r[13] & BIT(15);
	pr->cond_not = r[13] & BIT(14);
	pr->cond_and1 = r[13] & BIT(13);
	pr->cond_and2 = r[13] & BIT(12);
}

static void rtl839x_write_pie_fixed_fields(u32 r[],  struct pie_rule *pr)
{
	r[6] = ((u32) (pr->spmmask_fix & 0x3)) << 30;
	r[6] |= ((u32) (pr->spn & 0x3f)) << 24;
	r[6] |= pr->mgnt_vlan ? BIT(23) : 0;
	r[6] |= pr->dmac_hit_sw ? BIT(22) : 0;
	r[6] |= pr->not_first_frag ? BIT(21) : 0;
	r[6] |= ((u32) (pr->frame_type_l4 & 0x7)) << 18;
	r[6] |= ((u32) (pr->frame_type & 0x3)) << 16;
	r[6] |= pr->otag_fmt ? BIT(15) : 0;
	r[6] |= pr->itag_fmt ? BIT(14) : 0;
	r[6] |= pr->otag_exist ? BIT(13) : 0;
	r[6] |= pr->itag_exist ? BIT(12) : 0;
	r[6] |= ((u32) (pr->frame_type_l2 & 0x3)) << 10;
	r[6] |= ((u32) (pr->tid & 0x3)) << 8;

	r[12] |= ((u32) (pr->spmmask_fix_m & 0x3)) << 6;
	r[12] |= (u32) (pr->spn_m & 0x3f);
	r[13] |= pr->mgnt_vlan_m ? BIT(31) : 0;
	r[13] |= pr->dmac_hit_sw_m ? BIT(30) : 0;
	r[13] |= pr->not_first_frag_m ? BIT(29) : 0;
	r[13] |= ((u32) (pr->frame_type_l4_m & 0x7)) << 26;
	r[13] |= ((u32) (pr->frame_type_m & 0x3)) << 24;
	r[13] |= pr->otag_fmt_m ? BIT(23) : 0;
	r[13] |= pr->itag_fmt_m ? BIT(22) : 0;
	r[13] |= pr->otag_exist_m ? BIT(21) : 0;
	r[13] |= pr->itag_exist_m ? BIT(20) : 0;
	r[13] |= ((u32) (pr->frame_type_l2_m & 0x3)) << 18;
	r[13] |= ((u32) (pr->tid_m & 0x3)) << 16;

	r[13] |= pr->valid ? BIT(15) : 0;
	r[13] |= pr->cond_not ? BIT(14) : 0;
	r[13] |= pr->cond_and1 ? BIT(13) : 0;
	r[13] |= pr->cond_and2 ? BIT(12) : 0;
}

static void rtl839x_write_pie_action(u32 r[],  struct pie_rule *pr)
{
	if (pr->drop) {
		r[13] |= 0x9;	/* Set ACT_MASK_FWD & FWD_ACT = DROP */
		r[13] |= BIT(3);
	} else {
		r[13] |= pr->fwd_sel ? BIT(3) : 0;
		r[13] |= pr->fwd_act;
	}
	r[13] |= pr->bypass_sel ? BIT(11) : 0;
	r[13] |= pr->mpls_sel ? BIT(10) : 0;
	r[13] |= pr->nopri_sel ? BIT(9) : 0;
	r[13] |= pr->ovid_sel ? BIT(8) : 0;
	r[13] |= pr->ivid_sel ? BIT(7) : 0;
	r[13] |= pr->meter_sel ? BIT(6) : 0;
	r[13] |= pr->mir_sel ? BIT(5) : 0;
	r[13] |= pr->log_sel ? BIT(4) : 0;

	r[14] |= ((u32)(pr->fwd_data & 0x3fff)) << 18;
	r[14] |= pr->log_octets ? BIT(17) : 0;
	r[14] |= ((u32)(pr->log_data & 0x7ff)) << 4;
	r[14] |= (pr->mir_data & 0x3) << 3;
	r[14] |= ((u32)(pr->meter_data >> 7)) & 0x7;
	r[15] |= (u32)(pr->meter_data) << 26;
	r[15] |= ((u32)(pr->ivid_act) << 23) & 0x3;
	r[15] |= ((u32)(pr->ivid_data) << 9) & 0xfff;
	r[15] |= ((u32)(pr->ovid_act) << 6) & 0x3;
	r[15] |= ((u32)(pr->ovid_data) >> 4) & 0xff;
	r[16] |= ((u32)(pr->ovid_data) & 0xf) << 28;
	r[16] |= ((u32)(pr->nopri_data) & 0x7) << 20;
	r[16] |= ((u32)(pr->mpls_act) & 0x7) << 20;
	r[16] |= ((u32)(pr->mpls_lib_idx) & 0x7) << 20;
	r[16] |= pr->bypass_all ? BIT(9) : 0;
	r[16] |= pr->bypass_igr_stp ? BIT(8) : 0;
	r[16] |= pr->bypass_ibc_sc ? BIT(7) : 0;
}

static void rtl839x_read_pie_action(u32 r[],  struct pie_rule *pr)
{
	if (r[13] & BIT(3)) { /* ACT_MASK_FWD set, is it a drop? */
		if ((r[14] & 0x7) == 1) {
			pr->drop = true;
		} else {
			pr->fwd_sel = true;
			pr->fwd_act = r[14] & 0x7;
		}
	}

	pr->bypass_sel = r[13] & BIT(11);
	pr->mpls_sel = r[13] & BIT(10);
	pr->nopri_sel = r[13] & BIT(9);
	pr->ovid_sel = r[13] & BIT(8);
	pr->ivid_sel = r[13] & BIT(7);
	pr->meter_sel = r[13] & BIT(6);
	pr->mir_sel = r[13] & BIT(5);
	pr->log_sel = r[13] & BIT(4);

	/* TODO: Read in data fields */

	pr->bypass_all = r[16] & BIT(9);
	pr->bypass_igr_stp = r[16] & BIT(8);
	pr->bypass_ibc_sc = r[16] & BIT(7);
}

void rtl839x_pie_rule_dump_raw(u32 r[])
{
	pr_info("Raw IACL table entry:\n");
	pr_info("Match  : %08x %08x %08x %08x %08x %08x\n", r[0], r[1], r[2], r[3], r[4], r[5]);
	pr_info("Fixed  : %06x\n", r[6] >> 8);
	pr_info("Match M: %08x %08x %08x %08x %08x %08x\n",
		(r[6] << 24) | (r[7] >> 8), (r[7] << 24) | (r[8] >> 8), (r[8] << 24) | (r[9] >> 8),
		(r[9] << 24) | (r[10] >> 8), (r[10] << 24) | (r[11] >> 8),
		(r[11] << 24) | (r[12] >> 8));
	pr_info("R[13]:   %08x\n", r[13]);
	pr_info("Fixed M: %06x\n", ((r[12] << 16) | (r[13] >> 16)) & 0xffffff);
	pr_info("Valid / not / and1 / and2 : %1x\n", (r[13] >> 12) & 0xf);
	pr_info("r 13-16: %08x %08x %08x %08x\n", r[13], r[14], r[15], r[16]);
}

void rtl839x_pie_rule_dump(struct  pie_rule *pr)
{
	pr_info("Drop: %d, fwd: %d, ovid: %d, ivid: %d, flt: %d, log: %d, rmk: %d, meter: %d tagst: %d, mir: %d, nopri: %d, cpupri: %d, otpid: %d, itpid: %d, shape: %d\n",
		pr->drop, pr->fwd_sel, pr->ovid_sel, pr->ivid_sel, pr->flt_sel, pr->log_sel, pr->rmk_sel, pr->log_sel, pr->tagst_sel, pr->mir_sel, pr->nopri_sel,
		pr->cpupri_sel, pr->otpid_sel, pr->itpid_sel, pr->shaper_sel);
	if (pr->fwd_sel)
		pr_info("FWD: %08x\n", pr->fwd_data);
	pr_info("TID: %x, %x\n", pr->tid, pr->tid_m);
}

static int rtl839x_pie_rule_read(struct rtl838x_switch_priv *priv, int idx, struct  pie_rule *pr)
{
	/* Read IACL table (2) via register 0 */
	struct table_reg *q = rtl_table_get(RTL8380_TBL_0, 2);
	u32 r[17];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL839X_ACL_BLK_TMPLTE_CTRL(block));

	memset(pr, 0, sizeof(*pr));
	rtl_table_read(q, idx);
	for (int i = 0; i < 17; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl839x_read_pie_fixed_fields(r, pr);
	if (!pr->valid)
		return 0;

	pr_debug("%s: template_selectors %08x, tid: %d\n", __func__, t_select, pr->tid);
	rtl839x_pie_rule_dump_raw(r);

	rtl839x_read_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 3)) & 0x7]);

	rtl839x_read_pie_action(r, pr);

	return 0;
}

static int rtl839x_pie_rule_write(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr)
{
	/* Access IACL table (2) via register 0 */
	struct table_reg *q = rtl_table_get(RTL8390_TBL_0, 2);
	u32 r[17];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL839X_ACL_BLK_TMPLTE_CTRL(block));

	pr_debug("%s: %d, t_select: %08x\n", __func__, idx, t_select);

	for (int i = 0; i < 17; i++)
		r[i] = 0;

	if (!pr->valid) {
		rtl_table_write(q, idx);
		rtl_table_release(q);
		return 0;
	}
	rtl839x_write_pie_fixed_fields(r, pr);

	pr_debug("%s: template %d\n", __func__, (t_select >> (pr->tid * 3)) & 0x7);
	rtl839x_write_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 3)) & 0x7]);

	rtl839x_write_pie_action(r, pr);

/*	rtl839x_pie_rule_dump_raw(r); */

	for (int i = 0; i < 17; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);

	return 0;
}

static bool rtl839x_pie_templ_has(int t, enum template_field_id field_type)
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		enum template_field_id ft = fixed_templates[t][i];
		if (field_type == ft)
			return true;
	}

	return false;
}

static int rtl839x_pie_verify_template(struct rtl838x_switch_priv *priv,
				       struct pie_rule *pr, int t, int block)
{
	int i;

	if (!pr->is_ipv6 && pr->sip_m && !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_SIP0))
		return -1;

	if (!pr->is_ipv6 && pr->dip_m && !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_DIP0))
		return -1;

	if (pr->is_ipv6) {
		if ((pr->sip6_m.s6_addr32[0] ||
		     pr->sip6_m.s6_addr32[1] ||
		     pr->sip6_m.s6_addr32[2] ||
		     pr->sip6_m.s6_addr32[3]) &&
		    !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_SIP2))
			return -1;
		if ((pr->dip6_m.s6_addr32[0] ||
		     pr->dip6_m.s6_addr32[1] ||
		     pr->dip6_m.s6_addr32[2] ||
		     pr->dip6_m.s6_addr32[3]) &&
		    !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_DIP2))
			return -1;
	}

	if (ether_addr_to_u64(pr->smac) && !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_SMAC0))
		return -1;

	if (ether_addr_to_u64(pr->dmac) && !rtl839x_pie_templ_has(t, TEMPLATE_FIELD_DMAC0))
		return -1;

	/* TODO: Check more */

	i = find_first_zero_bit(&priv->pie_use_bm[block * 4], PIE_BLOCK_SIZE);

	if (i >= PIE_BLOCK_SIZE)
		return -1;

	return i + PIE_BLOCK_SIZE * block;
}

static int rtl839x_pie_rule_add(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx, block, j, t;
	int min_block = 0;
	int max_block = priv->n_pie_blocks / 2;

	if (pr->is_egress) {
		min_block = max_block;
		max_block = priv->n_pie_blocks;
	}

	mutex_lock(&priv->pie_mutex);

	for (block = min_block; block < max_block; block++) {
		for (j = 0; j < 2; j++) {
			t = (sw_r32(RTL839X_ACL_BLK_TMPLTE_CTRL(block)) >> (j * 3)) & 0x7;
			idx = rtl839x_pie_verify_template(priv, pr, t, block);
			if (idx >= 0)
				break;
		}
		if (j < 2)
			break;
	}

	if (block >= priv->n_pie_blocks) {
		mutex_unlock(&priv->pie_mutex);
		return -EOPNOTSUPP;
	}

	set_bit(idx, priv->pie_use_bm);

	pr->valid = true;
	pr->tid = j;  /* Mapped to template number */
	pr->tid_m = 0x3;
	pr->id = idx;

	rtl839x_pie_lookup_enable(priv, idx);
	rtl839x_pie_rule_write(priv, idx, pr);

	mutex_unlock(&priv->pie_mutex);

	return 0;
}

static void rtl839x_pie_rule_rm(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx = pr->id;

	rtl839x_pie_rule_del(priv, idx, idx);
	clear_bit(idx, priv->pie_use_bm);
}

static void rtl839x_pie_init(struct rtl838x_switch_priv *priv)
{
	u32 template_selectors;

	mutex_init(&priv->pie_mutex);

	/* Power on all PIE blocks */
	for (int i = 0; i < priv->n_pie_blocks; i++)
		sw_w32_mask(0, BIT(i), RTL839X_PS_ACL_PWR_CTRL);

	/* Set ingress and egress ACL blocks to 50/50: first Egress block is 9 */
	sw_w32_mask(0x1f, 9, RTL839X_ACL_CTRL);  /* Writes 9 to cutline field */

	/* Include IPG in metering */
	sw_w32(1, RTL839X_METER_GLB_CTRL);

	/* Delete all present rules */
	rtl839x_pie_rule_del(priv, 0, priv->n_pie_blocks * PIE_BLOCK_SIZE - 1);

	/* Enable predefined templates 0, 1 for blocks 0-2 */
	template_selectors = 0 | (1 << 3);
	for (int i = 0; i < 3; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for blocks 3-5 */
	template_selectors = 2 | (3 << 3);
	for (int i = 3; i < 6; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 1, 4 for blocks 6-8 */
	template_selectors = 2 | (3 << 3);
	for (int i = 6; i < 9; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 0, 1 for blocks 9-11 */
	template_selectors = 0 | (1 << 3);
	for (int i = 9; i < 12; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for blocks 12-14 */
	template_selectors = 2 | (3 << 3);
	for (int i = 12; i < 15; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 1, 4 for blocks 15-17 */
	template_selectors = 2 | (3 << 3);
	for (int i = 15; i < 18; i++)
		sw_w32(template_selectors, RTL839X_ACL_BLK_TMPLTE_CTRL(i));
}

static u32 rtl839x_packet_cntr_read(int counter)
{
	u32 v;

	/* Read LOG table (4) via register RTL8390_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_0, 4);

	pr_debug("In %s, id %d\n", __func__, counter);
	rtl_table_read(r, counter / 2);

	/* The table has a size of 2 registers */
	if (counter % 2)
		v = sw_r32(rtl_table_data(r, 0));
	else
		v = sw_r32(rtl_table_data(r, 1));

	rtl_table_release(r);

	return v;
}

static void rtl839x_packet_cntr_clear(int counter)
{
	/* Access LOG table (4) via register RTL8390_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_0, 4);

	pr_debug("In %s, id %d\n", __func__, counter);
	/* The table has a size of 2 registers */
	if (counter % 2)
		sw_w32(0, rtl_table_data(r, 0));
	else
		sw_w32(0, rtl_table_data(r, 1));

	rtl_table_write(r, counter / 2);

	rtl_table_release(r);
}

static void rtl839x_route_read(int idx, struct rtl83xx_route *rt)
{
	u64 v;
	/* Read ROUTING table (2) via register RTL8390_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 2);

	pr_debug("In %s\n", __func__);
	rtl_table_read(r, idx);

	/* The table has a size of 2 registers */
	v = sw_r32(rtl_table_data(r, 0));
	v <<= 32;
	v |= sw_r32(rtl_table_data(r, 1));
	rt->switch_mac_id = (v >> 12) & 0xf;
	rt->nh.gw = v >> 16;

	rtl_table_release(r);
}

static void rtl839x_route_write(int idx, struct rtl83xx_route *rt)
{
	u32 v;

	/* Read ROUTING table (2) via register RTL8390_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 2);

	pr_debug("In %s\n", __func__);
	sw_w32(rt->nh.gw >> 16, rtl_table_data(r, 0));
	v = rt->nh.gw << 16;
	v |= rt->switch_mac_id << 12;
	sw_w32(v, rtl_table_data(r, 1));
	rtl_table_write(r, idx);

	rtl_table_release(r);
}

/* Configure the switch's own MAC addresses used when routing packets */
static void rtl839x_setup_port_macs(struct rtl838x_switch_priv *priv)
{
	struct net_device *dev;
	u64 mac;

	pr_debug("%s: got port %08x\n", __func__, (u32)priv->ports[priv->cpu_port].dp);
	dev = priv->ports[priv->cpu_port].dp->slave;
	mac = ether_addr_to_u64(dev->dev_addr);

	for (int i = 0; i < 15; i++) {
		mac++;  /* BUG: VRRP for testing */
		sw_w32(mac >> 32, RTL839X_ROUTING_SA_CTRL + i * 8);
		sw_w32(mac, RTL839X_ROUTING_SA_CTRL + i * 8 + 4);
	}
}

int rtl839x_l3_setup(struct rtl838x_switch_priv *priv)
{
	rtl839x_setup_port_macs(priv);

	return 0;
}

void rtl839x_vlan_port_keep_tag_set(int port, bool keep_outer, bool keep_inner)
{
	sw_w32(FIELD_PREP(RTL839X_VLAN_PORT_TAG_STS_CTRL_OTAG_STS_MASK,
			  keep_outer ? RTL839X_VLAN_PORT_TAG_STS_TAGGED : RTL839X_VLAN_PORT_TAG_STS_UNTAG) |
	       FIELD_PREP(RTL839X_VLAN_PORT_TAG_STS_CTRL_ITAG_STS_MASK,
			  keep_inner ? RTL839X_VLAN_PORT_TAG_STS_TAGGED : RTL839X_VLAN_PORT_TAG_STS_UNTAG),
	       RTL839X_VLAN_PORT_TAG_STS_CTRL(port));
}

void rtl839x_vlan_port_pvidmode_set(int port, enum pbvlan_type type, enum pbvlan_mode mode)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0x3, mode, RTL839X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0x3 << 14, mode << 14, RTL839X_VLAN_PORT_PB_VLAN + (port << 2));
}

void rtl839x_vlan_port_pvid_set(int port, enum pbvlan_type type, int pvid)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0xfff << 2, pvid << 2, RTL839X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0xfff << 16, pvid << 16, RTL839X_VLAN_PORT_PB_VLAN + (port << 2));
}

static int rtl839x_set_ageing_time(unsigned long msec)
{
	int t = sw_r32(RTL839X_L2_CTRL_1);

	t &= 0x1FFFFF;
	t = t * 3 / 5; /* Aging time in seconds. 0: L2 aging disabled */
	pr_debug("L2 AGING time: %d sec\n", t);

	t = (msec * 5 + 2000) / 3000;
	t = t > 0x1FFFFF ? 0x1FFFFF : t;
	sw_w32_mask(0x1FFFFF, t, RTL839X_L2_CTRL_1);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(RTL839X_L2_PORT_AGING_OUT));

	return 0;
}

static void rtl839x_set_igr_filter(int port,  enum igr_filter state)
{
	sw_w32_mask(0x3 << ((port & 0xf)<<1), state << ((port & 0xf)<<1),
		    RTL839X_VLAN_PORT_IGR_FLTR + (((port >> 4) << 2)));
}

static void rtl839x_set_egr_filter(int port,  enum egr_filter state)
{
	sw_w32_mask(0x1 << (port % 0x20), state << (port % 0x20),
			RTL839X_VLAN_PORT_EGR_FLTR + (((port >> 5) << 2)));
}

void rtl839x_set_distribution_algorithm(int group, int algoidx, u32 algomsk)
{
	sw_w32_mask(3 << ((group & 0xf) << 1), algoidx << ((group & 0xf) << 1),
		    RTL839X_TRK_HASH_IDX_CTRL + ((group >> 4) << 2));
	sw_w32(algomsk, RTL839X_TRK_HASH_CTRL + (algoidx << 2));
}

void rtl839x_set_receive_management_action(int port, rma_ctrl_t type, action_type_t action)
{
	switch(type) {
	case BPDU:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL839X_RMA_BPDU_CTRL + ((port >> 4) << 2));
		break;
	case PTP:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL839X_RMA_PTP_CTRL + ((port >> 4) << 2));
		break;
	case LLDP:
		sw_w32_mask(3 << ((port & 0xf) << 1), (action & 0x3) << ((port & 0xf) << 1),
			    RTL839X_RMA_LLDP_CTRL + ((port >> 4) << 2));
		break;
	default:
		break;
	}
}

const struct rtl838x_reg rtl839x_reg = {
	.mask_port_reg_be = rtl839x_mask_port_reg_be,
	.set_port_reg_be = rtl839x_set_port_reg_be,
	.get_port_reg_be = rtl839x_get_port_reg_be,
	.mask_port_reg_le = rtl839x_mask_port_reg_le,
	.set_port_reg_le = rtl839x_set_port_reg_le,
	.get_port_reg_le = rtl839x_get_port_reg_le,
	.stat_port_rst = RTL839X_STAT_PORT_RST,
	.stat_rst = RTL839X_STAT_RST,
	.stat_port_std_mib = RTL839X_STAT_PORT_STD_MIB,
	.traffic_enable = rtl839x_traffic_enable,
	.traffic_disable = rtl839x_traffic_disable,
	.traffic_get = rtl839x_traffic_get,
	.traffic_set = rtl839x_traffic_set,
	.port_iso_ctrl = rtl839x_port_iso_ctrl,
	.l2_ctrl_0 = RTL839X_L2_CTRL_0,
	.l2_ctrl_1 = RTL839X_L2_CTRL_1,
	.l2_port_aging_out = RTL839X_L2_PORT_AGING_OUT,
	.set_ageing_time = rtl839x_set_ageing_time,
	.smi_poll_ctrl = RTL839X_SMI_PORT_POLLING_CTRL,
	.l2_tbl_flush_ctrl = RTL839X_L2_TBL_FLUSH_CTRL,
	.exec_tbl0_cmd = rtl839x_exec_tbl0_cmd,
	.exec_tbl1_cmd = rtl839x_exec_tbl1_cmd,
	.tbl_access_data_0 = rtl839x_tbl_access_data_0,
	.isr_glb_src = RTL839X_ISR_GLB_SRC,
	.isr_port_link_sts_chg = RTL839X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL839X_IMR_PORT_LINK_STS_CHG,
	.imr_glb = RTL839X_IMR_GLB,
	.vlan_tables_read = rtl839x_vlan_tables_read,
	.vlan_set_tagged = rtl839x_vlan_set_tagged,
	.vlan_set_untagged = rtl839x_vlan_set_untagged,
	.vlan_profile_dump = rtl839x_vlan_profile_dump,
	.vlan_profile_setup = rtl839x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl839x_vlan_fwd_on_inner,
	.vlan_port_keep_tag_set = rtl839x_vlan_port_keep_tag_set,
	.vlan_port_pvidmode_set = rtl839x_vlan_port_pvidmode_set,
	.vlan_port_pvid_set = rtl839x_vlan_port_pvid_set,
	.set_vlan_igr_filter = rtl839x_set_igr_filter,
	.set_vlan_egr_filter = rtl839x_set_egr_filter,
	.enable_learning = rtl839x_enable_learning,
	.enable_flood = rtl839x_enable_flood,
	.enable_mcast_flood = rtl839x_enable_mcast_flood,
	.enable_bcast_flood = rtl839x_enable_bcast_flood,
	.set_static_move_action = rtl839x_set_static_move_action,
	.stp_get = rtl839x_stp_get,
	.stp_set = rtl839x_stp_set,
	.mac_force_mode_ctrl = rtl839x_mac_force_mode_ctrl,
	.mac_port_ctrl = rtl839x_mac_port_ctrl,
	.l2_port_new_salrn = rtl839x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl839x_l2_port_new_sa_fwd,
	.mir_ctrl = RTL839X_MIR_CTRL,
	.mir_dpm = RTL839X_MIR_DPM_CTRL,
	.mir_spm = RTL839X_MIR_SPM_CTRL,
	.mac_link_sts = RTL839X_MAC_LINK_STS,
	.mac_link_dup_sts = RTL839X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts = rtl839x_mac_link_spd_sts,
	.mac_rx_pause_sts = RTL839X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts = RTL839X_MAC_TX_PAUSE_STS,
	.read_l2_entry_using_hash = rtl839x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl839x_write_l2_entry_using_hash,
	.read_cam = rtl839x_read_cam,
	.write_cam = rtl839x_write_cam,
	.trk_mbr_ctr = rtl839x_trk_mbr_ctr,
	.rma_bpdu_fld_pmask = RTL839X_RMA_BPDU_FLD_PMSK,
	.spcl_trap_eapol_ctrl = RTL839X_SPCL_TRAP_EAPOL_CTRL,
	.init_eee = rtl839x_init_eee,
	.port_eee_set = rtl839x_port_eee_set,
	.eee_port_ability = rtl839x_eee_port_ability,
	.l2_hash_seed = rtl839x_l2_hash_seed,
	.l2_hash_key = rtl839x_l2_hash_key,
	.read_mcast_pmask = rtl839x_read_mcast_pmask,
	.write_mcast_pmask = rtl839x_write_mcast_pmask,
	.pie_init = rtl839x_pie_init,
	.pie_rule_read = rtl839x_pie_rule_read,
	.pie_rule_write = rtl839x_pie_rule_write,
	.pie_rule_add = rtl839x_pie_rule_add,
	.pie_rule_rm = rtl839x_pie_rule_rm,
	.l2_learning_setup = rtl839x_l2_learning_setup,
	.packet_cntr_read = rtl839x_packet_cntr_read,
	.packet_cntr_clear = rtl839x_packet_cntr_clear,
	.route_read = rtl839x_route_read,
	.route_write = rtl839x_route_write,
	.l3_setup = rtl839x_l3_setup,
	.set_distribution_algorithm = rtl839x_set_distribution_algorithm,
	.set_receive_management_action = rtl839x_set_receive_management_action,
};
