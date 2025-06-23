// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>

#include "rtl83xx.h"

#define RTL930X_VLAN_PORT_TAG_STS_INTERNAL			0x0
#define RTL930X_VLAN_PORT_TAG_STS_UNTAG				0x1
#define RTL930X_VLAN_PORT_TAG_STS_TAGGED			0x2
#define RTL930X_VLAN_PORT_TAG_STS_PRIORITY_TAGGED		0x3

#define RTL930X_VLAN_PORT_TAG_STS_CTRL_BASE			0xCE24
/* port 0-28 */
#define RTL930X_VLAN_PORT_TAG_STS_CTRL(port) \
		RTL930X_VLAN_PORT_TAG_STS_CTRL_BASE + (port << 2)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_OTAG_STS_MASK	GENMASK(7,6)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_ITAG_STS_MASK	GENMASK(5,4)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_P_OTAG_KEEP_MASK	GENMASK(3,3)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_P_ITAG_KEEP_MASK	GENMASK(2,2)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_IGR_P_OTAG_KEEP_MASK	GENMASK(1,1)
#define RTL930X_VLAN_PORT_TAG_STS_CTRL_IGR_P_ITAG_KEEP_MASK	GENMASK(0,0)

#define RTL930X_LED_GLB_ACTIVE_LOW				BIT(22)

#define RTL930X_LED_SETX_0_CTRL(x) (RTL930X_LED_SET0_0_CTRL - (x * 8))
#define RTL930X_LED_SETX_1_CTRL(x) (RTL930X_LED_SETX_0_CTRL(x) - 4)

/* get register for given set and led in the set */
#define RTL930X_LED_SETX_LEDY(x,y) (RTL930X_LED_SETX_0_CTRL(x) - 4 * (y / 2))

/* get shift for given led in any set */
#define RTL930X_LED_SET_LEDX_SHIFT(x) (16 * (x % 2))

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;

/* Definition of the RTL930X-specific template field IDs as used in the PIE */
enum template_field_id {
	TEMPLATE_FIELD_SPM0 = 0,		/* Source portmask ports 0-15 */
	TEMPLATE_FIELD_SPM1 = 1,		/* Source portmask ports 16-31 */
	TEMPLATE_FIELD_DMAC0 = 2,		/* Destination MAC [15:0] */
	TEMPLATE_FIELD_DMAC1 = 3,		/* Destination MAC [31:16] */
	TEMPLATE_FIELD_DMAC2 = 4,		/* Destination MAC [47:32] */
	TEMPLATE_FIELD_SMAC0 = 5,		/* Source MAC [15:0] */
	TEMPLATE_FIELD_SMAC1 = 6,		/* Source MAC [31:16] */
	TEMPLATE_FIELD_SMAC2 = 7,		/* Source MAC [47:32] */
	TEMPLATE_FIELD_ETHERTYPE = 8,		/* Ethernet frame type field */
	TEMPLATE_FIELD_OTAG = 9,
	TEMPLATE_FIELD_ITAG = 10,
	TEMPLATE_FIELD_SIP0 = 11,
	TEMPLATE_FIELD_SIP1 = 12,
	TEMPLATE_FIELD_DIP0 = 13,
	TEMPLATE_FIELD_DIP1 = 14,
	TEMPLATE_FIELD_IP_TOS_PROTO = 15,
	TEMPLATE_FIELD_L4_SPORT = 16,
	TEMPLATE_FIELD_L4_DPORT = 17,
	TEMPLATE_FIELD_L34_HEADER = 18,
	TEMPLATE_FIELD_TCP_INFO = 19,
	TEMPLATE_FIELD_FIELD_SELECTOR_VALID = 20,
	TEMPLATE_FIELD_FIELD_SELECTOR_0 = 21,
	TEMPLATE_FIELD_FIELD_SELECTOR_1 = 22,
	TEMPLATE_FIELD_FIELD_SELECTOR_2 = 23,
	TEMPLATE_FIELD_FIELD_SELECTOR_3 = 24,
	TEMPLATE_FIELD_FIELD_SELECTOR_4 = 25,
	TEMPLATE_FIELD_FIELD_SELECTOR_5 = 26,
	TEMPLATE_FIELD_SIP2 = 27,
	TEMPLATE_FIELD_SIP3 = 28,
	TEMPLATE_FIELD_SIP4 = 29,
	TEMPLATE_FIELD_SIP5 = 30,
	TEMPLATE_FIELD_SIP6 = 31,
	TEMPLATE_FIELD_SIP7 = 32,
	TEMPLATE_FIELD_DIP2 = 33,
	TEMPLATE_FIELD_DIP3 = 34,
	TEMPLATE_FIELD_DIP4 = 35,
	TEMPLATE_FIELD_DIP5 = 36,
	TEMPLATE_FIELD_DIP6 = 37,
	TEMPLATE_FIELD_DIP7 = 38,
	TEMPLATE_FIELD_PKT_INFO = 39,
	TEMPLATE_FIELD_FLOW_LABEL = 40,
	TEMPLATE_FIELD_DSAP_SSAP = 41,
	TEMPLATE_FIELD_SNAP_OUI = 42,
	TEMPLATE_FIELD_FWD_VID = 43,
	TEMPLATE_FIELD_RANGE_CHK = 44,
	TEMPLATE_FIELD_VLAN_GMSK = 45,		/* VLAN Group Mask/IP range check */
	TEMPLATE_FIELD_DLP = 46,
	TEMPLATE_FIELD_META_DATA = 47,
	TEMPLATE_FIELD_SRC_FWD_VID = 48,
	TEMPLATE_FIELD_SLP = 49,
};

/* The meaning of TEMPLATE_FIELD_VLAN depends on phase and the configuration in
 * RTL930X_PIE_CTRL. We use always the same definition and map to the inner VLAN tag:
 */
#define TEMPLATE_FIELD_VLAN TEMPLATE_FIELD_ITAG

/* Number of fixed templates predefined in the RTL9300 SoC */
#define N_FIXED_TEMPLATES 5
/* RTL9300 specific predefined templates */
static enum template_field_id fixed_templates[N_FIXED_TEMPLATES][N_FIXED_FIELDS] =
{
	{
	  TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
	  TEMPLATE_FIELD_SMAC0, TEMPLATE_FIELD_SMAC1, TEMPLATE_FIELD_SMAC2,
	  TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_DSAP_SSAP,
	  TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1
	}, {
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
	  TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_TCP_INFO,
	  TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_VLAN,
	  TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1
	}, {
	  TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
	  TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_IP_TOS_PROTO,
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
	  TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT
	}, {
	  TEMPLATE_FIELD_DIP0, TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_DIP2,
	  TEMPLATE_FIELD_DIP3, TEMPLATE_FIELD_DIP4, TEMPLATE_FIELD_DIP5,
	  TEMPLATE_FIELD_DIP6, TEMPLATE_FIELD_DIP7, TEMPLATE_FIELD_IP_TOS_PROTO,
	  TEMPLATE_FIELD_TCP_INFO, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT
	}, {
	  TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_SIP2,
	  TEMPLATE_FIELD_SIP3, TEMPLATE_FIELD_SIP4, TEMPLATE_FIELD_SIP5,
	  TEMPLATE_FIELD_SIP6, TEMPLATE_FIELD_SIP7, TEMPLATE_FIELD_VLAN,
	  TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM1, TEMPLATE_FIELD_SPM1
	},
};

void rtl930x_print_matrix(void)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 6);

	for (int i = 0; i < 29; i++) {
		rtl_table_read(r, i);
		pr_debug("> %08x\n", sw_r32(rtl_table_data(r, 0)));
	}
	rtl_table_release(r);
}

inline void rtl930x_exec_tbl0_cmd(u32 cmd)
{
	sw_w32(cmd, RTL930X_TBL_ACCESS_CTRL_0);
	do { } while (sw_r32(RTL930X_TBL_ACCESS_CTRL_0) & (1 << 17));
}

inline void rtl930x_exec_tbl1_cmd(u32 cmd)
{
	sw_w32(cmd, RTL930X_TBL_ACCESS_CTRL_1);
	do { } while (sw_r32(RTL930X_TBL_ACCESS_CTRL_1) & (1 << 17));
}

inline int rtl930x_tbl_access_data_0(int i)
{
	return RTL930X_TBL_ACCESS_DATA_0(i);
}

static inline int rtl930x_l2_port_new_salrn(int p)
{
	return RTL930X_L2_PORT_SALRN(p);
}

static inline int rtl930x_l2_port_new_sa_fwd(int p)
{
	/* TODO: The definition of the fields changed, because of the master-cpu in a stack */
	return RTL930X_L2_PORT_NEW_SA_FWD(p);
}

inline static int rtl930x_trk_mbr_ctr(int group)
{
	return RTL930X_TRK_MBR_CTRL + (group << 2);
}

static void rtl930x_vlan_tables_read(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v, w;
	/* Read VLAN table (1) via register 0 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 1);

	rtl_table_read(r, vlan);
	v = sw_r32(rtl_table_data(r, 0));
	w = sw_r32(rtl_table_data(r, 1));
	pr_debug("VLAN_READ %d: %08x %08x\n", vlan, v, w);
	rtl_table_release(r);

	info->tagged_ports = v >> 3;
	info->profile_id = (w >> 24) & 7;
	info->hash_mc_fid = !!(w & BIT(27));
	info->hash_uc_fid = !!(w & BIT(28));
	info->fid = ((v & 0x7) << 3) | ((w >> 29) & 0x7);

	/* Read UNTAG table via table register 2 */
	r = rtl_table_get(RTL9300_TBL_2, 0);
	rtl_table_read(r, vlan);
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);

	info->untagged_ports = v >> 3;
}

static void rtl930x_vlan_set_tagged(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v, w;
	/* Access VLAN table (1) via register 0 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 1);

	v = info->tagged_ports << 3;
	v |= ((u32)info->fid) >> 3;

	w = ((u32)info->fid) << 29;
	w |= info->hash_mc_fid ? BIT(27) : 0;
	w |= info->hash_uc_fid ? BIT(28) : 0;
	w |= info->profile_id << 24;

	sw_w32(v, rtl_table_data(r, 0));
	sw_w32(w, rtl_table_data(r, 1));

	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

void rtl930x_vlan_profile_dump(int profile)
{
	u32 p[5];

	if (profile < 0 || profile > 7)
		return;

	p[0] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile));
	p[1] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile) + 4);
	p[2] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile) + 8) & 0x1FFFFFFF;
	p[3] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile) + 12) & 0x1FFFFFFF;
	p[4] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile) + 16) & 0x1FFFFFFF;

	pr_debug("VLAN %d: L2 learn: %d; Unknown MC PMasks: L2 %0x, IPv4 %0x, IPv6: %0x",
		profile, p[0] & (3 << 21), p[2], p[3], p[4]);
	pr_debug("  Routing enabled: IPv4 UC %c, IPv6 UC %c, IPv4 MC %c, IPv6 MC %c\n",
		p[0] & BIT(17) ? 'y' : 'n', p[0] & BIT(16) ? 'y' : 'n',
		p[0] & BIT(13) ? 'y' : 'n', p[0] & BIT(12) ? 'y' : 'n');
	pr_debug("  Bridge enabled: IPv4 MC %c, IPv6 MC %c,\n",
		p[0] & BIT(15) ? 'y' : 'n', p[0] & BIT(14) ? 'y' : 'n');
	pr_debug("VLAN profile %d: raw %08x %08x %08x %08x %08x\n",
		profile, p[0], p[1], p[2], p[3], p[4]);
}

static void rtl930x_vlan_set_untagged(u32 vlan, u64 portmask)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_2, 0);

	sw_w32(portmask << 3, rtl_table_data(r, 0));
	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

/* Sets the L2 forwarding to be based on either the inner VLAN tag or the outer */
static void rtl930x_vlan_fwd_on_inner(int port, bool is_set)
{
	/* Always set all tag modes to fwd based on either inner or outer tag */
	if (is_set)
		sw_w32_mask(0xf, 0, RTL930X_VLAN_PORT_FWD + (port << 2));
	else
		sw_w32_mask(0, 0xf, RTL930X_VLAN_PORT_FWD + (port << 2));
}

static void rtl930x_vlan_profile_setup(int profile)
{
	u32 p[5];

	pr_debug("In %s\n", __func__);
	p[0] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile));
	p[1] = sw_r32(RTL930X_VLAN_PROFILE_SET(profile) + 4);

	/* Enable routing of Ipv4/6 Unicast and IPv4/6 Multicast traffic */
	p[0] |= BIT(17) | BIT(16) | BIT(13) | BIT(12);
	p[2] = 0x1fffffff; /* L2 unknown MC flooding portmask all ports, including the CPU-port */
	p[3] = 0x1fffffff; /* IPv4 unknown MC flooding portmask */
	p[4] = 0x1fffffff; /* IPv6 unknown MC flooding portmask */

	sw_w32(p[0], RTL930X_VLAN_PROFILE_SET(profile));
	sw_w32(p[1], RTL930X_VLAN_PROFILE_SET(profile) + 4);
	sw_w32(p[2], RTL930X_VLAN_PROFILE_SET(profile) + 8);
	sw_w32(p[3], RTL930X_VLAN_PROFILE_SET(profile) + 12);
	sw_w32(p[4], RTL930X_VLAN_PROFILE_SET(profile) + 16);
}

static void rtl930x_l2_learning_setup(void)
{
	/* Portmask for flooding broadcast traffic */
	sw_w32(0x1fffffff, RTL930X_L2_BC_FLD_PMSK);

	/* Portmask for flooding unicast traffic with unknown destination */
	sw_w32(0x1fffffff, RTL930X_L2_UNKN_UC_FLD_PMSK);

	/* Limit learning to maximum: 32k entries, after that just flood (bits 0-1) */
	sw_w32((0x7fff << 2) | 0, RTL930X_L2_LRN_CONSTRT_CTRL);
}

static void rtl930x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 17 | /* Execute cmd */
	          0 << 16 | /* Read */
	          4 << 12 | /* Table type 0b10 */
	          (msti & 0xfff);
	priv->r->exec_tbl0_cmd(cmd);

	for (int i = 0; i < 2; i++)
		port_state[i] = sw_r32(RTL930X_TBL_ACCESS_DATA_0(i));
	pr_debug("MSTI: %d STATE: %08x, %08x\n", msti, port_state[0], port_state[1]);
}

static void rtl930x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 17 | /* Execute cmd */
	          1 << 16 | /* Write */
	          4 << 12 | /* Table type 4 */
	          (msti & 0xfff);

	for (int i = 0; i < 2; i++)
		sw_w32(port_state[i], RTL930X_TBL_ACCESS_DATA_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

static inline int rtl930x_mac_force_mode_ctrl(int p)
{
	return RTL930X_MAC_FORCE_MODE_CTRL + (p << 2);
}

static inline int rtl930x_mac_port_ctrl(int p)
{
	return RTL930X_MAC_L2_PORT_CTRL(p);
}

static inline int rtl930x_mac_link_spd_sts(int p)
{
	return RTL930X_MAC_LINK_SPD_STS(p);
}

static u64 rtl930x_l2_hash_seed(u64 mac, u32 vid)
{
	u64 v = vid;

	v <<= 48;
	v |= mac;

	return v;
}

/* Calculate both the block 0 and the block 1 hash by applyingthe same hash
 * algorithm as the one used currently by the ASIC to the seed, and return
 * both hashes in the lower and higher word of the return value since only 12 bit of
 * the hash are significant
 */
static u32 rtl930x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 k0, k1, h1, h2, h;

	k0 = (u32) (((seed >> 55) & 0x1f) ^
	           ((seed >> 44) & 0x7ff) ^
	           ((seed >> 33) & 0x7ff) ^
	           ((seed >> 22) & 0x7ff) ^
	           ((seed >> 11) & 0x7ff) ^
	           (seed & 0x7ff));

	h1 = (seed >> 11) & 0x7ff;
	h1 = ((h1 & 0x1f) << 6) | ((h1 >> 5) & 0x3f);

	h2 = (seed >> 33) & 0x7ff;
	h2 = ((h2 & 0x3f) << 5)| ((h2 >> 6) & 0x3f);

	k1 = (u32) (((seed << 55) & 0x1f) ^
	           ((seed >> 44) & 0x7ff) ^
	           h2 ^
	           ((seed >> 22) & 0x7ff) ^
	           h1 ^
	           (seed & 0x7ff));

	/* Algorithm choice for block 0 */
	if (sw_r32(RTL930X_L2_CTRL) & BIT(0))
		h = k1;
	else
		h = k0;

	/* Algorithm choice for block 1
	 * Since k0 and k1 are < 2048, adding 2048 will offset the hash into the second
	 * half of hash-space
	 * 2048 is in fact the hash-table size 16384 divided by 4 hashes per bucket
	 * divided by 2 to divide the hash space in 2
	 */
	if (sw_r32(RTL930X_L2_CTRL) & BIT(1))
		h |= (k1 + 2048) << 16;
	else
		h |= (k0 + 2048) << 16;

	return h;
}

/* Fills an L2 entry structure from the SoC registers */
static void rtl930x_fill_l2_entry(u32 r[], struct rtl838x_l2_entry *e)
{
	pr_debug("In %s valid?\n", __func__);
	e->valid = !!(r[2] & BIT(31));
	if (!e->valid)
		return;

	pr_debug("In %s is valid\n", __func__);
	e->is_ip_mc = false;
	e->is_ipv6_mc = false;

	/* TODO: Is there not a function to copy directly MAC memory? */
	e->mac[0] = (r[0] >> 24);
	e->mac[1] = (r[0] >> 16);
	e->mac[2] = (r[0] >> 8);
	e->mac[3] = r[0];
	e->mac[4] = (r[1] >> 24);
	e->mac[5] = (r[1] >> 16);

	e->next_hop = !!(r[2] & BIT(12));
	e->rvid = r[1] & 0xfff;

	/* Is it a unicast entry? check multicast bit */
	if (!(e->mac[0] & 1)) {
		e->type = L2_UNICAST;
		e->is_static = !!(r[2] & BIT(14));
		e->port = (r[2] >> 20) & 0x3ff;
		/* Check for trunk port */
		if (r[2] & BIT(30)) {
			e->is_trunk = true;
			e->stack_dev = (e->port >> 9) & 1;
			e->trunk = e->port & 0x3f;
		} else {
			e->is_trunk = false;
			e->stack_dev = (e->port >> 6) & 0xf;
			e->port = e->port & 0x3f;
		}

		e->block_da = !!(r[2] & BIT(15));
		e->block_sa = !!(r[2] & BIT(16));
		e->suspended = !!(r[2] & BIT(13));
		e->age = (r[2] >> 17) & 3;
		e->valid = true;
		/* the UC_VID field in hardware is used for the VID or for the route id */
		if (e->next_hop) {
			e->nh_route_id = r[2] & 0x7ff;
			e->vid = 0;
		} else {
			e->vid = r[2] & 0xfff;
			e->nh_route_id = 0;
		}
	} else {
		e->valid = true;
		e->type = L2_MULTICAST;
		e->mc_portmask_index = (r[2] >> 16) & 0x3ff;
	}
}

/* Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry */
static void rtl930x_fill_l2_row(u32 r[], struct rtl838x_l2_entry *e)
{
	u32 port;

	if (!e->valid) {
		r[0] = r[1] = r[2] = 0;
		return;
	}

	r[2] = BIT(31);	/* Set valid bit */

	r[0] = ((u32)e->mac[0]) << 24 |
	       ((u32)e->mac[1]) << 16 |
	       ((u32)e->mac[2]) << 8 |
	       ((u32)e->mac[3]);
	r[1] = ((u32)e->mac[4]) << 24 |
	       ((u32)e->mac[5]) << 16;

	r[2] |= e->next_hop ? BIT(12) : 0;

	if (e->type == L2_UNICAST) {
		r[2] |= e->is_static ? BIT(14) : 0;
		r[1] |= e->rvid & 0xfff;
		r[2] |= (e->port & 0x3ff) << 20;
		if (e->is_trunk) {
			r[2] |= BIT(30);
			port = e->stack_dev << 9 | (e->port & 0x3f);
		} else {
			port = (e->stack_dev & 0xf) << 6;
			port |= e->port & 0x3f;
		}
		r[2] |= port << 20;
		r[2] |= e->block_da ? BIT(15) : 0;
		r[2] |= e->block_sa ? BIT(17) : 0;
		r[2] |= e->suspended ? BIT(13) : 0;
		r[2] |= (e->age & 0x3) << 17;
		/* the UC_VID field in hardware is used for the VID or for the route id */
		if (e->next_hop)
			r[2] |= e->nh_route_id & 0x7ff;
		else
			r[2] |= e->vid & 0xfff;
	} else { /* L2_MULTICAST */
		r[2] |= (e->mc_portmask_index & 0x3ff) << 16;
		r[2] |= e->mc_mac_index & 0x7ff;
	}
}

/* Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl930x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 0);
	u32 idx;
	u64 mac;
	u64 seed;

	pr_debug("%s: hash %08x, pos: %d\n", __func__, hash, pos);

	/* On the RTL93xx, 2 different hash algorithms are used making it a
	 * total of 8 buckets that need to be searched, 4 for each hash-half
	 * Use second hash space when bucket is between 4 and 8
	 */
	if (pos >= 4) {
		pos -= 4;
		hash >>= 16;
	} else {
		hash &= 0xffff;
	}

	idx = (0 << 14) | (hash << 2) | pos; /* Search SRAM, with hash and at pos in bucket */
	pr_debug("%s: NOW hash %08x, pos: %d\n", __func__, hash, pos);

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl930x_fill_l2_entry(r, e);

	pr_debug("%s: valid: %d, nh: %d\n", __func__, e->valid, e->next_hop);
	if (!e->valid)
		return 0;

	mac = ((u64)e->mac[0]) << 40 |
	      ((u64)e->mac[1]) << 32 |
	      ((u64)e->mac[2]) << 24 |
	      ((u64)e->mac[3]) << 16 |
	      ((u64)e->mac[4]) << 8 |
	      ((u64)e->mac[5]);

	seed = rtl930x_l2_hash_seed(mac, e->rvid);
	pr_debug("%s: mac %016llx, seed %016llx\n", __func__, mac, seed);

	/* return vid with concatenated mac as unique id */
	return seed;
}

static void rtl930x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 0);
	u32 idx = (0 << 14) | (hash << 2) | pos; /* Access SRAM, with hash and at pos in bucket */

	pr_debug("%s: hash %d, pos %d\n", __func__, hash, pos);
	pr_debug("%s: index %d -> mac %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, idx,
		e->mac[0], e->mac[1], e->mac[2], e->mac[3],e->mac[4],e->mac[5]);

	rtl930x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl930x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 1);

	rtl_table_read(q, idx);
	for (int i = 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl930x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	/* return mac with concatenated vid as unique id */
	return ((u64)r[0] << 28) | ((r[1] & 0xffff0000) >> 4) | e->vid;
}

static void rtl930x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 1); /* Access L2 Table 1 */

	rtl930x_fill_l2_row(r, e);

	for (int i = 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl930x_read_mcast_pmask(int idx)
{
	u32 portmask;
	/* Read MC_PORTMASK (2) via register RTL9300_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	portmask >>= 3;
	rtl_table_release(q);

	pr_debug("%s: Index idx %d has portmask %08x\n", __func__, idx, portmask);

	return portmask;
}

static void rtl930x_write_mcast_pmask(int idx, u64 portmask)
{
	u32 pm = portmask;

	/* Access MC_PORTMASK (2) via register RTL9300_TBL_L2 */
	struct table_reg *q = rtl_table_get(RTL9300_TBL_L2, 2);

	pr_debug("%s: Index idx %d has portmask %08x\n", __func__, idx, pm);
	pm <<= 3;
	sw_w32(pm, rtl_table_data(q, 0));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl930x_traffic_get(int source)
{
	u32 v;
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 6);

	rtl_table_read(r, source);
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);
	v = v >> 3;

	return v;
}

/* Enable traffic between a source port and a destination port matrix */
static void rtl930x_traffic_set(int source, u64 dest_matrix)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 6);

	sw_w32((dest_matrix << 3), rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl930x_traffic_enable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 6);
	rtl_table_read(r, source);
	sw_w32_mask(0, BIT(dest + 3), rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl930x_traffic_disable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 6);
	rtl_table_read(r, source);
	sw_w32_mask(BIT(dest + 3), 0, rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

void rtl9300_dump_debug(void)
{
	u16 r = RTL930X_STAT_PRVTE_DROP_COUNTER0;

	for (int i = 0; i < 10; i ++) {
		pr_debug("# %d %08x %08x %08x %08x %08x %08x %08x %08x\n", i * 8,
			sw_r32(r), sw_r32(r + 4), sw_r32(r + 8), sw_r32(r + 12),
			sw_r32(r + 16), sw_r32(r + 20), sw_r32(r + 24), sw_r32(r + 28));
		r += 32;
	}
	pr_debug("# %08x %08x %08x %08x %08x\n",
		sw_r32(r), sw_r32(r + 4), sw_r32(r + 8), sw_r32(r + 12), sw_r32(r + 16));
	rtl930x_print_matrix();
	pr_debug("RTL930X_L2_PORT_SABLK_CTRL: %08x, RTL930X_L2_PORT_DABLK_CTRL %08x\n",
		sw_r32(RTL930X_L2_PORT_SABLK_CTRL), sw_r32(RTL930X_L2_PORT_DABLK_CTRL)

	);
}

irqreturn_t rtl930x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 ports = sw_r32(RTL930X_ISR_PORT_LINK_STS_CHG);
	u32 link;

	/* Clear status */
	sw_w32(ports, RTL930X_ISR_PORT_LINK_STS_CHG);

	for (int i = 0; i < 28; i++) {
		if (ports & BIT(i)) {
			/* Read the register twice because of issues with latency at least
			 * with the external RTL8226 PHY on the XGS1210
			 */
			link = sw_r32(RTL930X_MAC_LINK_STS);
			link = sw_r32(RTL930X_MAC_LINK_STS);
			if (link & BIT(i))
				dsa_port_phylink_mac_change(ds, i, true);
			else
				dsa_port_phylink_mac_change(ds, i, false);
		}
	}

	return IRQ_HANDLED;
}

int rtl930x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	int err = 0;

	pr_debug("%s: port %d, page: %d, reg: %x, val: %x\n", __func__, port, page, reg, val);

	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	val &= 0xffff;
	mutex_lock(&smi_lock);

	sw_w32(BIT(port), RTL930X_SMI_ACCESS_PHY_CTRL_0);
	sw_w32_mask(0xffff << 16, val << 16, RTL930X_SMI_ACCESS_PHY_CTRL_2);
	v = reg << 20 | page << 3 | 0x1f << 15 | BIT(2) | BIT(0);
	sw_w32(v, RTL930X_SMI_ACCESS_PHY_CTRL_1);

	do {
		v = sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_1);
	} while (v & 0x1);

	if (v & 0x2)
		err = -EIO;

	mutex_unlock(&smi_lock);

	return err;
}

int rtl930x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;
	int err = 0;

	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	sw_w32_mask(0xffff << 16, port << 16, RTL930X_SMI_ACCESS_PHY_CTRL_2);
	v = reg << 20 | page << 3 | 0x1f << 15 | 1;
	sw_w32(v, RTL930X_SMI_ACCESS_PHY_CTRL_1);

	do {
		v = sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_1);
	} while ( v & 0x1);

	if (v & BIT(25)) {
		pr_debug("Error reading phy %d, register %d\n", port, reg);
		err = -EIO;
	}
	*val = (sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_2) & 0xffff);

	pr_debug("%s: port %d, page: %d, reg: %x, val: %x\n", __func__, port, page, reg, *val);

	mutex_unlock(&smi_lock);

	return err;
}

/* Write to an mmd register of the PHY */
int rtl930x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val)
{
	int err = 0;
	u32 v;

	mutex_lock(&smi_lock);

	/* Set PHY to access */
	sw_w32(BIT(port), RTL930X_SMI_ACCESS_PHY_CTRL_0);

	/* Set data to write */
	sw_w32_mask(0xffff << 16, val << 16, RTL930X_SMI_ACCESS_PHY_CTRL_2);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL930X_SMI_ACCESS_PHY_CTRL_3);

	v = BIT(2) | BIT(1) | BIT(0); /* WRITE | MMD-access | EXEC */
	sw_w32(v, RTL930X_SMI_ACCESS_PHY_CTRL_1);

	do {
		v = sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_1);
	} while (v & BIT(0));

	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, val, err);
	mutex_unlock(&smi_lock);
	return err;
}

/* Read an mmd register of the PHY */
int rtl930x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val)
{
	int err = 0;
	u32 v;

	mutex_lock(&smi_lock);

	/* Set PHY to access */
	sw_w32_mask(0xffff << 16, port << 16, RTL930X_SMI_ACCESS_PHY_CTRL_2);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL930X_SMI_ACCESS_PHY_CTRL_3);

	v = BIT(1) | BIT(0); /* MMD-access | EXEC */
	sw_w32(v, RTL930X_SMI_ACCESS_PHY_CTRL_1);

	do {
		v = sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_1);
	} while (v & BIT(0));
	/* There is no error-checking via BIT 25 of v, as it does not seem to be set correctly */
	*val = (sw_r32(RTL930X_SMI_ACCESS_PHY_CTRL_2) & 0xffff);
	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, *val, err);

	mutex_unlock(&smi_lock);

	return err;
}

/* Calculate both the block 0 and the block 1 hash, and return in
 * lower and higher word of the return value since only 12 bit of
 * the hash are significant
 */
u32 rtl930x_hash(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 k0, k1, h1, h2, h;

	k0 = (u32) (((seed >> 55) & 0x1f) ^
	            ((seed >> 44) & 0x7ff) ^
	            ((seed >> 33) & 0x7ff) ^
	            ((seed >> 22) & 0x7ff) ^
	            ((seed >> 11) & 0x7ff) ^
	            (seed & 0x7ff));

	h1 = (seed >> 11) & 0x7ff;
	h1 = ((h1 & 0x1f) << 6) | ((h1 >> 5) & 0x3f);

	h2 = (seed >> 33) & 0x7ff;
	h2 = ((h2 & 0x3f) << 5) | ((h2 >> 6) & 0x3f);

	k1 = (u32) (((seed << 55) & 0x1f) ^
	           ((seed >> 44) & 0x7ff) ^
	           h2 ^
	           ((seed >> 22) & 0x7ff) ^
	           h1 ^
	           (seed & 0x7ff));

	/* Algorithm choice for block 0 */
	if (sw_r32(RTL930X_L2_CTRL) & BIT(0))
		h = k1;
	else
		h = k0;

	/* Algorithm choice for block 1
	 * Since k0 and k1 are < 2048, adding 2048 will offset the hash into the second
	 * half of hash-space
	 * 2048 is in fact the hash-table size 16384 divided by 4 hashes per bucket
	 * divided by 2 to divide the hash space in 2
	 */
	if (sw_r32(RTL930X_L2_CTRL) & BIT(1))
		h |= (k1 + 2048) << 16;
	else
		h |= (k0 + 2048) << 16;

	return h;
}

/* Enables or disables the EEE/EEEP capability of a port */
static void rtldsa_930x_set_mac_eee(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;

	/* This works only for Ethernet ports, and on the RTL930X, ports from 26 are SFP */
	if (port >= 26)
		return;

	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? 0x3f : 0x0;

	/* Set EEE/EEEP state for 100, 500, 1000MBit and 2.5, 5 and 10GBit */
	sw_w32_mask(0, v << 10, rtl930x_mac_force_mode_ctrl(port));

	/* Set TX/RX EEE state */
	v = enable ? 0x3 : 0x0;
	sw_w32(v, RTL930X_EEE_CTRL(port));

	priv->ports[port].eee_enabled = enable;
}

static void rtl930x_init_eee(struct rtl838x_switch_priv *priv, bool enable)
{
	pr_debug("Setting up EEE, state: %d\n", enable);

	/* Setup EEE on all ports */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			priv->r->set_mac_eee(priv, i, enable);
	}

	priv->eee_enabled = enable;
}
#define HASH_PICK(val, lsb, len)   ((val & (((1 << len) - 1) << lsb)) >> lsb)

static u32 rtl930x_l3_hash4(u32 ip, int algorithm, bool move_dip)
{
	u32 rows[4];
	u32 hash;
	u32 s0, s1, pH;

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

// Currently not used
// static u32 rtl930x_l3_hash6(struct in6_addr *ip6, int algorithm, bool move_dip)
// {
// 	u32 rows[16];
// 	u32 hash;
// 	u32 s0, s1, pH;

// 	rows[0] = (HASH_PICK(ip6->s6_addr[0], 6, 2) << 0);
// 	rows[1] = (HASH_PICK(ip6->s6_addr[0], 0, 6) << 3) | HASH_PICK(ip6->s6_addr[1], 5, 3);
// 	rows[2] = (HASH_PICK(ip6->s6_addr[1], 0, 5) << 4) | HASH_PICK(ip6->s6_addr[2], 4, 4);
// 	rows[3] = (HASH_PICK(ip6->s6_addr[2], 0, 4) << 5) | HASH_PICK(ip6->s6_addr[3], 3, 5);
// 	rows[4] = (HASH_PICK(ip6->s6_addr[3], 0, 3) << 6) | HASH_PICK(ip6->s6_addr[4], 2, 6);
// 	rows[5] = (HASH_PICK(ip6->s6_addr[4], 0, 2) << 7) | HASH_PICK(ip6->s6_addr[5], 1, 7);
// 	rows[6] = (HASH_PICK(ip6->s6_addr[5], 0, 1) << 8) | HASH_PICK(ip6->s6_addr[6], 0, 8);
// 	rows[7] = (HASH_PICK(ip6->s6_addr[7], 0, 8) << 1) | HASH_PICK(ip6->s6_addr[8], 7, 1);
// 	rows[8] = (HASH_PICK(ip6->s6_addr[8], 0, 7) << 2) | HASH_PICK(ip6->s6_addr[9], 6, 2);
// 	rows[9] = (HASH_PICK(ip6->s6_addr[9], 0, 6) << 3) | HASH_PICK(ip6->s6_addr[10], 5, 3);
// 	rows[10] = (HASH_PICK(ip6->s6_addr[10], 0, 5) << 4) | HASH_PICK(ip6->s6_addr[11], 4, 4);
// 	if (!algorithm) {
// 		rows[11] = (HASH_PICK(ip6->s6_addr[11], 0, 4) << 5) |
// 		           (HASH_PICK(ip6->s6_addr[12], 3, 5) << 0);
// 		rows[12] = (HASH_PICK(ip6->s6_addr[12], 0, 3) << 6) |
// 		           (HASH_PICK(ip6->s6_addr[13], 2, 6) << 0);
// 		rows[13] = (HASH_PICK(ip6->s6_addr[13], 0, 2) << 7) |
// 		           (HASH_PICK(ip6->s6_addr[14], 1, 7) << 0);
// 		if (!move_dip) {
// 			rows[14] = (HASH_PICK(ip6->s6_addr[14], 0, 1) << 8) |
// 			           (HASH_PICK(ip6->s6_addr[15], 0, 8) << 0);
// 		}
// 		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3] ^ rows[4] ^
// 		       rows[5] ^ rows[6] ^ rows[7] ^ rows[8] ^ rows[9] ^
// 		       rows[10] ^ rows[11] ^ rows[12] ^ rows[13] ^ rows[14];
// 	} else {
// 		rows[11] = (HASH_PICK(ip6->s6_addr[11], 0, 4) << 5);
// 		rows[12] = (HASH_PICK(ip6->s6_addr[12], 3, 5) << 0);
// 		rows[13] = (HASH_PICK(ip6->s6_addr[12], 0, 3) << 6) |
// 		           HASH_PICK(ip6->s6_addr[13], 2, 6);
// 		rows[14] = (HASH_PICK(ip6->s6_addr[13], 0, 2) << 7) |
// 		           HASH_PICK(ip6->s6_addr[14], 1, 7);
// 		if (!move_dip) {
// 			rows[15] = (HASH_PICK(ip6->s6_addr[14], 0, 1) << 8) |
// 			           (HASH_PICK(ip6->s6_addr[15], 0, 8) << 0);
// 		}
// 		s0 = rows[12] + rows[13] + rows[14];
// 		s1 = (s0 & 0x1ff) + ((s0 & (0x1ff << 9)) >> 9);
// 		pH = (s1 & 0x1ff) + ((s1 & (0x1ff << 9)) >> 9);
// 		hash = rows[0] ^ rows[1] ^ rows[2] ^ rows[3] ^ rows[4] ^
// 		       rows[5] ^ rows[6] ^ rows[7] ^ rows[8] ^ rows[9] ^
// 		       rows[10] ^ rows[11] ^ pH ^ rows[15];
// 	}
// 	return hash;
// }

/* Read a prefix route entry from the L3_PREFIX_ROUTE_IPUC table
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl930x_route_read(int idx, struct rtl83xx_route *rt)
{
	u32 v, ip4_m;
	bool host_route, default_route;
	struct in6_addr ip6_m;

	/* Read L3_PREFIX_ROUTE_IPUC table (2) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 2);

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
	pr_debug("%s: host route %d, default_route %d\n", __func__, host_route, default_route);

	switch (rt->attr.type) {
	case 0: /* IPv4 Unicast route */
		rt->dst_ip = sw_r32(rtl_table_data(r, 4));
		ip4_m = sw_r32(rtl_table_data(r, 9));
		pr_debug("%s: Read ip4 mask: %08x\n", __func__, ip4_m);
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
			rt->prefix_len = find_last_bit((unsigned long int *)&ip6_m.s6_addr32,
							 128);
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
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
	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);
out:
	rtl_table_release(r);
}

static void rtl930x_net6_mask(int prefix_len, struct in6_addr *ip6_m)
{
	int o, b;
	/* Define network mask */
	o = prefix_len >> 3;
	b = prefix_len & 0x7;
	memset(ip6_m->s6_addr, 0xff, o);
	ip6_m->s6_addr[o] |= b ? 0xff00 >> b : 0x00;
}

/* Read a host route entry from the table using its index
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl930x_host_route_read(int idx, struct rtl83xx_route *rt)
{
	u32 v;
	/* Read L3_HOST_ROUTE_IPUC table (1) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 1);

	idx = ((idx / 6) * 8) + (idx % 6);

	pr_debug("In %s, physical index %d\n", __func__, idx);
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
		pr_warn("%s: route type not supported\n", __func__);
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
	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	pr_debug("%s: Destination: %pI4\n", __func__, &rt->dst_ip);

out:
	rtl_table_release(r);
}

/* Write a host route entry from the table using its index
 * We currently only support IPv4 and IPv6 unicast route
 */
static void rtl930x_host_route_write(int idx, struct rtl83xx_route *rt)
{
	u32 v;
	/* Access L3_HOST_ROUTE_IPUC table (1) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 1);
	/* The table has a size of 5 (for UC, 11 for MC) registers */

	idx = ((idx / 6) * 8) + (idx % 6);

	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: next_hop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action, rt->attr.ttl_dec, rt->attr.ttl_check,
		rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);

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
		pr_warn("%s: route type not supported\n", __func__);
		goto out;
	}

	rtl_table_write(r, idx);

out:
	rtl_table_release(r);
}

/* Look up the index of a prefix route in the routing table CAM for unicast IPv4/6 routes
 * using hardware offload.
 */
static int rtl930x_route_lookup_hw(struct rtl83xx_route *rt)
{
	u32 ip4_m, v;
	struct in6_addr ip6_m;

	if (rt->attr.type == 1 || rt->attr.type == 3) /* Hardware only supports UC routes */
		return -1;

	sw_w32_mask(0x3 << 19, rt->attr.type, RTL930X_L3_HW_LU_KEY_CTRL);
	if (rt->attr.type) { /* IPv6 */
		rtl930x_net6_mask(rt->prefix_len, &ip6_m);
		for (int i = 0; i < 4; i++)
			sw_w32(rt->dst_ip6.s6_addr32[0] & ip6_m.s6_addr32[0],
			       RTL930X_L3_HW_LU_KEY_IP_CTRL + (i << 2));
	} else { /* IPv4 */
		ip4_m = inet_make_mask(rt->prefix_len);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL + 4);
		sw_w32(0, RTL930X_L3_HW_LU_KEY_IP_CTRL + 8);
		v = rt->dst_ip & ip4_m;
		pr_debug("%s: searching for %pI4\n", __func__, &v);
		sw_w32(v, RTL930X_L3_HW_LU_KEY_IP_CTRL + 12);
	}

	/* Execute CAM lookup in SoC */
	sw_w32(BIT(15), RTL930X_L3_HW_LU_CTRL);

	/* Wait until execute bit clears and result is ready */
	do {
		v = sw_r32(RTL930X_L3_HW_LU_CTRL);
	} while (v & BIT(15));

	pr_debug("%s: found: %d, index: %d\n", __func__, !!(v & BIT(14)), v & 0x1ff);

	/* Test if search successful (BIT 14 set) */
	if (v & BIT(14))
		return v & 0x1ff;

	return -1;
}

static int rtl930x_find_l3_slot(struct rtl83xx_route *rt, bool must_exist)
{
	int slot_width, algorithm, addr, idx;
	u32 hash;
	struct rtl83xx_route route_entry;

	/* IPv6 entries take up 3 slots */
	slot_width = (rt->attr.type == 0) || (rt->attr.type == 2) ? 1 : 3;

	for (int t = 0; t < 2; t++) {
		algorithm = (sw_r32(RTL930X_L3_HOST_TBL_CTRL) >> (2 + t)) & 0x1;
		hash = rtl930x_l3_hash4(rt->dst_ip, algorithm, false);

		pr_debug("%s: table %d, algorithm %d, hash %04x\n", __func__, t, algorithm, hash);

		for (int s = 0; s < 6; s += slot_width) {
			addr = (t << 12) | ((hash & 0x1ff) << 3) | s;
			pr_debug("%s physical address %d\n", __func__, addr);
			idx = ((addr / 8) * 6) + (addr % 8);
			pr_debug("%s logical address %d\n", __func__, idx);

			rtl930x_host_route_read(idx, &route_entry);
			pr_debug("%s route valid %d, route dest: %pI4, hit %d\n", __func__,
				rt->attr.valid, &rt->dst_ip, rt->attr.hit);
			if (!must_exist && rt->attr.valid)
				return idx;
			if (must_exist && route_entry.dst_ip == rt->dst_ip)
				return idx;
		}
	}

	return -1;
}

/* Write a prefix route into the routing table CAM at position idx
 * Currently only IPv4 and IPv6 unicast routes are supported
 */
static void rtl930x_route_write(int idx, struct rtl83xx_route *rt)
{
	u32 v, ip4_m;
	struct in6_addr ip6_m;
	/* Access L3_PREFIX_ROUTE_IPUC table (2) via register RTL9300_TBL_1 */
	/* The table has a size of 11 registers (20 for MC) */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 2);

	pr_debug("%s: index %d is valid: %d\n", __func__, idx, rt->attr.valid);
	pr_debug("%s: nexthop: %d, hit: %d, action :%d, ttl_dec %d, ttl_check %d, dst_null %d\n",
		__func__, rt->nh.id, rt->attr.hit, rt->attr.action,
		rt->attr.ttl_dec, rt->attr.ttl_check, rt->attr.dst_null);
	pr_debug("%s: GW: %pI4, prefix_len: %d\n", __func__, &rt->dst_ip, rt->prefix_len);

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

		rtl930x_net6_mask(rt->prefix_len, &ip6_m);

		sw_w32(ip6_m.s6_addr32[0], rtl_table_data(r, 6));
		sw_w32(ip6_m.s6_addr32[1], rtl_table_data(r, 7));
		sw_w32(ip6_m.s6_addr32[2], rtl_table_data(r, 8));
		sw_w32(ip6_m.s6_addr32[3], rtl_table_data(r, 9));
		break;
	case 1: /* IPv4 Multicast route */
	case 3: /* IPv6 Multicast route */
		pr_warn("%s: route type not supported\n", __func__);
		rtl_table_release(r);
		return;
	}
	sw_w32(v, rtl_table_data(r, 10));

	pr_debug("%s: %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x\n", __func__,
		sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		sw_r32(rtl_table_data(r, 6)), sw_r32(rtl_table_data(r, 7)), sw_r32(rtl_table_data(r, 8)),
		sw_r32(rtl_table_data(r, 9)), sw_r32(rtl_table_data(r, 10)));

	rtl_table_write(r, idx);
	rtl_table_release(r);
}


/* Get the destination MAC and L3 egress interface ID of a nexthop entry from
 * the SoC's L3_NEXTHOP table
 */
static void rtl930x_get_l3_nexthop(int idx, u16 *dmac_id, u16 *interface)
{
	u32 v;
	/* Read L3_NEXTHOP table (3) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 3);

	rtl_table_read(r, idx);
	/* The table has a size of 1 register */
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);

	*dmac_id = (v >> 7) & 0x7fff;
	*interface = v & 0x7f;
}

// Currently not used
// static int rtl930x_l3_mtu_del(struct rtl838x_switch_priv *priv, int mtu)
// {
// 	int i;

// 	for (i = 0; i < MAX_INTF_MTUS; i++) {
// 		if (mtu == priv->intf_mtus[i])
// 			break;
// 	}
// 	if (i >= MAX_INTF_MTUS || !priv->intf_mtu_count[i]) {
// 		pr_err("%s: No MTU slot found for MTU: %d\n", __func__, mtu);
// 		return -EINVAL;
// 	}

// 	priv->intf_mtu_count[i]--;
// }

// Currently not used
// static int rtl930x_l3_mtu_add(struct rtl838x_switch_priv *priv, int mtu)
// {
// 	int i, free_mtu;
// 	int mtu_id;

// 	/* Try to find an existing mtu-value or a free slot */
// 	free_mtu = MAX_INTF_MTUS;
// 	for (i = 0; i < MAX_INTF_MTUS && priv->intf_mtus[i] != mtu; i++) {
// 		if ((!priv->intf_mtu_count[i]) && (free_mtu == MAX_INTF_MTUS))
// 			free_mtu = i;
// 	}
// 	i = (i < MAX_INTF_MTUS) ? i : free_mtu;
// 	if (i < MAX_INTF_MTUS) {
// 		mtu_id = i;
// 	} else {
// 		pr_err("%s: No free MTU slot available!\n", __func__);
// 		return -EINVAL;
// 	}

// 	priv->intf_mtus[i] = mtu;
// 	pr_debug("Writing MTU %d to slot %d\n", priv->intf_mtus[i], i);
// 	/* Set MTU-value of the slot TODO: distinguish between IPv4/IPv6 routes / slots */
// 	sw_w32_mask(0xffff << ((i % 2) * 16), priv->intf_mtus[i] << ((i % 2) * 16),
// 		    RTL930X_L3_IP_MTU_CTRL(i));
// 	sw_w32_mask(0xffff << ((i % 2) * 16), priv->intf_mtus[i] << ((i % 2) * 16),
// 		    RTL930X_L3_IP6_MTU_CTRL(i));

// 	priv->intf_mtu_count[i]++;

// 	return mtu_id;
// }


// Currently not used
// /* Creates an interface for a route by setting up the HW tables in the SoC
// static int rtl930x_l3_intf_add(struct rtl838x_switch_priv *priv, struct rtl838x_l3_intf *intf)
// {
// 	int i, intf_id, mtu_id;
// 	/* number of MTU-values < 16384 *\/

// 	/* Use the same IPv6 mtu as the ip4 mtu for this route if unset */
// 	intf->ip6_mtu = intf->ip6_mtu ? intf->ip6_mtu : intf->ip4_mtu;

// 	mtu_id = rtl930x_l3_mtu_add(priv, intf->ip4_mtu);
// 	pr_debug("%s: added mtu %d with mtu-id %d\n", __func__, intf->ip4_mtu, mtu_id);
// 	if (mtu_id < 0)
// 		return -ENOSPC;
// 	intf->ip4_mtu_id = mtu_id;
// 	intf->ip6_mtu_id = mtu_id;

// 	for (i = 0; i < MAX_INTERFACES; i++) {
// 		if (!priv->interfaces[i])
// 			break;
// 	}
// 	if (i >= MAX_INTERFACES) {
// 		pr_err("%s: cannot find free interface entry\n", __func__);
// 		return -EINVAL;
// 	}
// 	intf_id = i;
// 	priv->interfaces[i] = kzalloc(sizeof(struct rtl838x_l3_intf), GFP_KERNEL);
// 	if (!priv->interfaces[i]) {
// 		pr_err("%s: no memory to allocate new interface\n", __func__);
// 		return -ENOMEM;
// 	}
// }

/* Set the destination MAC and L3 egress interface ID for a nexthop entry in the SoC's
 * L3_NEXTHOP table. The nexthop entry is identified by idx.
 * dmac_id is the reference to the L2 entry in the L2 forwarding table, special values are
 * 0x7ffe: TRAP2CPU
 * 0x7ffd: TRAP2MASTERCPU
 * 0x7fff: DMAC_ID_DROP
 */
static void rtl930x_set_l3_nexthop(int idx, u16 dmac_id, u16 interface)
{
	/* Access L3_NEXTHOP table (3) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 3);

	pr_debug("%s: Writing to L3_NEXTHOP table, index %d, dmac_id %d, interface %d\n",
		__func__, idx, dmac_id, interface);
	sw_w32(((dmac_id & 0x7fff) << 7) | (interface & 0x7f), rtl_table_data(r, 0));

	pr_debug("%s: %08x\n", __func__, sw_r32(rtl_table_data(r,0)));
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

static void rtl930x_pie_lookup_enable(struct rtl838x_switch_priv *priv, int index)
{
	int block = index / PIE_BLOCK_SIZE;

	sw_w32_mask(0, BIT(block), RTL930X_PIE_BLK_LOOKUP_CTRL);
}

/* Reads the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure and fills in the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL9310 has 2 more registers / fields and the physical field-ids are different
 * on all SoCs
 * On the RTL9300 the mask fields are not word-aligend!
 */
static void rtl930x_write_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
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
		case TEMPLATE_FIELD_DSAP_SSAP:
			data = pr->dsap_ssap;
			data_m = pr->dsap_ssap_m;
			break;
		case TEMPLATE_FIELD_TCP_INFO:
			data = pr->tcp_info;
			data_m = pr->tcp_info_m;
			break;
		case TEMPLATE_FIELD_RANGE_CHK:
			pr_debug("TEMPLATE_FIELD_RANGE_CHK: not configured\n");
			break;
		default:
			pr_debug("%s: unknown field %d\n", __func__, field_type);
		}

		/* On the RTL9300, the mask fields are not word aligned! */
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

// Currently not used
// static void rtl930x_read_pie_fixed_fields(u32 r[], struct pie_rule *pr)
// {
// 	pr->stacking_port = r[6] & BIT(31);
// 	pr->spn = (r[6] >> 24) & 0x7f;
// 	pr->mgnt_vlan = r[6] & BIT(23);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw = r[6] & BIT(22);
// 	else
// 		pr->content_too_deep = r[6] & BIT(22);
// 	pr->not_first_frag = r[6]  & BIT(21);
// 	pr->frame_type_l4 = (r[6] >> 18) & 7;
// 	pr->frame_type = (r[6] >> 16) & 3;
// 	pr->otag_fmt = (r[6] >> 15) & 1;
// 	pr->itag_fmt = (r[6] >> 14) & 1;
// 	pr->otag_exist = (r[6] >> 13) & 1;
// 	pr->itag_exist = (r[6] >> 12) & 1;
// 	pr->frame_type_l2 = (r[6] >> 10) & 3;
// 	pr->igr_normal_port = (r[6] >> 9) & 1;
// 	pr->tid = (r[6] >> 8) & 1;

// 	pr->stacking_port_m = r[12] & BIT(7);
// 	pr->spn_m = r[12]  & 0x7f;
// 	pr->mgnt_vlan_m = r[13] & BIT(31);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw_m = r[13] & BIT(30);
// 	else
// 		pr->content_too_deep_m = r[13] & BIT(30);
// 	pr->not_first_frag_m = r[13] & BIT(29);
// 	pr->frame_type_l4_m = (r[13] >> 26) & 7;
// 	pr->frame_type_m = (r[13] >> 24) & 3;
// 	pr->otag_fmt_m = r[13] & BIT(23);
// 	pr->itag_fmt_m = r[13] & BIT(22);
// 	pr->otag_exist_m = r[13] & BIT(21);
// 	pr->itag_exist_m = r[13] & BIT (20);
// 	pr->frame_type_l2_m = (r[13] >> 18) & 3;
// 	pr->igr_normal_port_m = r[13] & BIT(17);
// 	pr->tid_m = (r[13] >> 16) & 1;

// 	pr->valid = r[13] & BIT(15);
// 	pr->cond_not = r[13] & BIT(14);
// 	pr->cond_and1 = r[13] & BIT(13);
// 	pr->cond_and2 = r[13] & BIT(12);
// }

static void rtl930x_write_pie_fixed_fields(u32 r[],  struct pie_rule *pr)
{
	r[6] = pr->stacking_port ? BIT(31) : 0;
	r[6] |= ((u32) (pr->spn & 0x7f)) << 24;
	r[6] |= pr->mgnt_vlan ? BIT(23) : 0;
	if (pr->phase == PHASE_IACL)
		r[6] |= pr->dmac_hit_sw ? BIT(22) : 0;
	else
		r[6] |= pr->content_too_deep ? BIT(22) : 0;
	r[6] |= pr->not_first_frag ? BIT(21) : 0;
	r[6] |= ((u32) (pr->frame_type_l4 & 0x7)) << 18;
	r[6] |= ((u32) (pr->frame_type & 0x3)) << 16;
	r[6] |= pr->otag_fmt ? BIT(15) : 0;
	r[6] |= pr->itag_fmt ? BIT(14) : 0;
	r[6] |= pr->otag_exist ? BIT(13) : 0;
	r[6] |= pr->itag_exist ? BIT(12) : 0;
	r[6] |= ((u32) (pr->frame_type_l2 & 0x3)) << 10;
	r[6] |= pr->igr_normal_port ? BIT(9) : 0;
	r[6] |= ((u32) (pr->tid & 0x1)) << 8;

	r[12] |= pr->stacking_port_m ? BIT(7) : 0;
	r[12] |= (u32) (pr->spn_m & 0x7f);
	r[13] |= pr->mgnt_vlan_m ? BIT(31) : 0;
	if (pr->phase == PHASE_IACL)
		r[13] |= pr->dmac_hit_sw_m ? BIT(30) : 0;
	else
		r[13] |= pr->content_too_deep_m ? BIT(30) : 0;
	r[13] |= pr->not_first_frag_m ? BIT(29) : 0;
	r[13] |= ((u32) (pr->frame_type_l4_m & 0x7)) << 26;
	r[13] |= ((u32) (pr->frame_type_m & 0x3)) << 24;
	r[13] |= pr->otag_fmt_m ? BIT(23) : 0;
	r[13] |= pr->itag_fmt_m ? BIT(22) : 0;
	r[13] |= pr->otag_exist_m ? BIT(21) : 0;
	r[13] |= pr->itag_exist_m ? BIT(20) : 0;
	r[13] |= ((u32) (pr->frame_type_l2_m & 0x3)) << 18;
	r[13] |= pr->igr_normal_port_m ? BIT(17) : 0;
	r[13] |= ((u32) (pr->tid_m & 0x1)) << 16;

	r[13] |= pr->valid ? BIT(15) : 0;
	r[13] |= pr->cond_not ? BIT(14) : 0;
	r[13] |= pr->cond_and1 ? BIT(13) : 0;
	r[13] |= pr->cond_and2 ? BIT(12) : 0;
}

static void rtl930x_write_pie_action(u32 r[],  struct pie_rule *pr)
{
	/* Either drop or forward */
	if (pr->drop) {
		r[14] |= BIT(24) | BIT(25) | BIT(26); /* Do Green, Yellow and Red drops */
		/* Actually DROP, not PERMIT in Green / Yellow / Red */
		r[14] |= BIT(23) | BIT(22) | BIT(20);
	} else {
		r[14] |= pr->fwd_sel ? BIT(27) : 0;
		r[14] |= pr->fwd_act << 18;
		r[14] |= BIT(14); /* We overwrite any drop */
	}
	if (pr->phase == PHASE_VACL)
		r[14] |= pr->fwd_sa_lrn ? BIT(15) : 0;
	r[13] |= pr->bypass_sel ? BIT(5) : 0;
	r[13] |= pr->nopri_sel ? BIT(4) : 0;
	r[13] |= pr->tagst_sel ? BIT(3) : 0;
	r[13] |= pr->ovid_sel ? BIT(1) : 0;
	r[14] |= pr->ivid_sel ? BIT(31) : 0;
	r[14] |= pr->meter_sel ? BIT(30) : 0;
	r[14] |= pr->mir_sel ? BIT(29) : 0;
	r[14] |= pr->log_sel ? BIT(28) : 0;

	r[14] |= ((u32)(pr->fwd_data & 0x3fff)) << 3;
	r[15] |= pr->log_octets ? BIT(31) : 0;
	r[15] |= (u32)(pr->meter_data) << 23;

	r[15] |= ((u32)(pr->ivid_act) << 21) & 0x3;
	r[15] |= ((u32)(pr->ivid_data) << 9) & 0xfff;
	r[16] |= ((u32)(pr->ovid_act) << 30) & 0x3;
	r[16] |= ((u32)(pr->ovid_data) & 0xfff) << 16;
	r[16] |= (pr->mir_data & 0x3) << 6;
	r[17] |= ((u32)(pr->tagst_data) & 0xf) << 28;
	r[17] |= ((u32)(pr->nopri_data) & 0x7) << 25;
	r[17] |= pr->bypass_ibc_sc ? BIT(16) : 0;
}

void rtl930x_pie_rule_dump_raw(u32 r[])
{
	pr_debug("Raw IACL table entry:\n");
	pr_debug("r 0 - 7: %08x %08x %08x %08x %08x %08x %08x %08x\n",
		r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7]);
	pr_debug("r 8 - 15: %08x %08x %08x %08x %08x %08x %08x %08x\n",
		r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15]);
	pr_debug("r 16 - 18: %08x %08x %08x\n", r[16], r[17], r[18]);
	pr_debug("Match  : %08x %08x %08x %08x %08x %08x\n", r[0], r[1], r[2], r[3], r[4], r[5]);
	pr_debug("Fixed  : %06x\n", r[6] >> 8);
	pr_debug("Match M: %08x %08x %08x %08x %08x %08x\n",
		(r[6] << 24) | (r[7] >> 8), (r[7] << 24) | (r[8] >> 8), (r[8] << 24) | (r[9] >> 8),
		(r[9] << 24) | (r[10] >> 8), (r[10] << 24) | (r[11] >> 8),
		(r[11] << 24) | (r[12] >> 8));
	pr_debug("R[13]:   %08x\n", r[13]);
	pr_debug("Fixed M: %06x\n", ((r[12] << 16) | (r[13] >> 16)) & 0xffffff);
	pr_debug("Valid / not / and1 / and2 : %1x\n", (r[13] >> 12) & 0xf);
	pr_debug("r 13-16: %08x %08x %08x %08x\n", r[13], r[14], r[15], r[16]);
}

static int rtl930x_pie_rule_write(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr)
{
	/* Access IACL table (2) via register 0 */
	struct table_reg *q = rtl_table_get(RTL9300_TBL_0, 2);
	u32 r[19];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL930X_PIE_BLK_TMPLTE_CTRL(block));

	pr_debug("%s: %d, t_select: %08x\n", __func__, idx, t_select);

	for (int i = 0; i < 19; i++)
		r[i] = 0;

	if (!pr->valid) {
		rtl_table_write(q, idx);
		rtl_table_release(q);
		return 0;
	}
	rtl930x_write_pie_fixed_fields(r, pr);

	pr_debug("%s: template %d\n", __func__, (t_select >> (pr->tid * 4)) & 0xf);
	rtl930x_write_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 4)) & 0xf]);

	rtl930x_write_pie_action(r, pr);

/*	rtl930x_pie_rule_dump_raw(r); */

	for (int i = 0; i < 19; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);

	return 0;
}

static bool rtl930x_pie_templ_has(int t, enum template_field_id field_type)
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		enum template_field_id ft = fixed_templates[t][i];
		if (field_type == ft)
			return true;
	}

	return false;
}

/* Verify that the rule pr is compatible with a given template t in block block
 * Note that this function is SoC specific since the values of e.g. TEMPLATE_FIELD_SIP0
 * depend on the SoC
 */
static int rtl930x_pie_verify_template(struct rtl838x_switch_priv *priv,
				       struct pie_rule *pr, int t, int block)
{
	int i;

	if (!pr->is_ipv6 && pr->sip_m && !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_SIP0))
		return -1;

	if (!pr->is_ipv6 && pr->dip_m && !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_DIP0))
		return -1;

	if (pr->is_ipv6) {
		if ((pr->sip6_m.s6_addr32[0] ||
		     pr->sip6_m.s6_addr32[1] ||
		     pr->sip6_m.s6_addr32[2] ||
		     pr->sip6_m.s6_addr32[3]) &&
		    !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_SIP2))
			return -1;
		if ((pr->dip6_m.s6_addr32[0] ||
		     pr->dip6_m.s6_addr32[1] ||
		     pr->dip6_m.s6_addr32[2] ||
		     pr->dip6_m.s6_addr32[3]) &&
		    !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_DIP2))
			return -1;
	}

	if (ether_addr_to_u64(pr->smac) && !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_SMAC0))
		return -1;

	if (ether_addr_to_u64(pr->dmac) && !rtl930x_pie_templ_has(t, TEMPLATE_FIELD_DMAC0))
		return -1;

	/* TODO: Check more */

	i = find_first_zero_bit(&priv->pie_use_bm[block * 4], PIE_BLOCK_SIZE);

	if (i >= PIE_BLOCK_SIZE)
		return -1;

	return i + PIE_BLOCK_SIZE * block;
}

static int rtl930x_pie_rule_add(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx, block, j, t;
	int min_block = 0;
	int max_block = priv->n_pie_blocks / 2;

	if (pr->is_egress) {
		min_block = max_block;
		max_block = priv->n_pie_blocks;
	}
	pr_debug("In %s\n", __func__);

	mutex_lock(&priv->pie_mutex);

	for (block = min_block; block < max_block; block++) {
		for (j = 0; j < 2; j++) {
			t = (sw_r32(RTL930X_PIE_BLK_TMPLTE_CTRL(block)) >> (j * 4)) & 0xf;
			pr_debug("Testing block %d, template %d, template id %d\n", block, j, t);
			pr_debug("%s: %08x\n",
				__func__, sw_r32(RTL930X_PIE_BLK_TMPLTE_CTRL(block)));
			idx = rtl930x_pie_verify_template(priv, pr, t, block);
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

	pr_debug("Using block: %d, index %d, template-id %d\n", block, idx, j);
	set_bit(idx, priv->pie_use_bm);

	pr->valid = true;
	pr->tid = j;  /* Mapped to template number */
	pr->tid_m = 0x1;
	pr->id = idx;

	rtl930x_pie_lookup_enable(priv, idx);
	rtl930x_pie_rule_write(priv, idx, pr);

	mutex_unlock(&priv->pie_mutex);
	return 0;
}

/* Delete a range of Packet Inspection Engine rules */
static int rtl930x_pie_rule_del(struct rtl838x_switch_priv *priv, int index_from, int index_to)
{
	u32 v = (index_from << 1)| (index_to << 12 ) | BIT(0);

	pr_debug("%s: from %d to %d\n", __func__, index_from, index_to);
	mutex_lock(&priv->reg_mutex);

	/* Write from-to and execute bit into control register */
	sw_w32(v, RTL930X_PIE_CLR_CTRL);

	/* Wait until command has completed */
	do {
	} while (sw_r32(RTL930X_PIE_CLR_CTRL) & BIT(0));

	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static void rtl930x_pie_rule_rm(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx = pr->id;

	rtl930x_pie_rule_del(priv, idx, idx);
	clear_bit(idx, priv->pie_use_bm);
}

static void rtl930x_pie_init(struct rtl838x_switch_priv *priv)
{
	u32 template_selectors;

	mutex_init(&priv->pie_mutex);

	pr_debug("%s\n", __func__);
	/* Enable ACL lookup on all ports, including CPU_PORT */
	for (int i = 0; i <= priv->cpu_port; i++)
		sw_w32(1, RTL930X_ACL_PORT_LOOKUP_CTRL(i));

	/* Include IPG in metering */
	sw_w32_mask(0, 1, RTL930X_METER_GLB_CTRL);

	/* Delete all present rules, block size is 128 on all SoC families */
	rtl930x_pie_rule_del(priv, 0, priv->n_pie_blocks * 128 - 1);

	/* Assign blocks 0-7 to VACL phase (bit = 0), blocks 8-15 to IACL (bit = 1) */
	sw_w32(0xff00, RTL930X_PIE_BLK_PHASE_CTRL);

	/* Enable predefined templates 0, 1 for first quarter of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = 0; i < priv->n_pie_blocks / 4; i++)
		sw_w32(template_selectors, RTL930X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for second quarter of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks / 4; i < priv->n_pie_blocks / 2; i++)
		sw_w32(template_selectors, RTL930X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 0, 1 for third half of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = priv->n_pie_blocks / 2; i < priv->n_pie_blocks * 3 / 4; i++)
		sw_w32(template_selectors, RTL930X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for fourth quater of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks * 3 / 4; i < priv->n_pie_blocks; i++)
		sw_w32(template_selectors, RTL930X_PIE_BLK_TMPLTE_CTRL(i));

}

/* Sets up an egress interface for L3 actions
 * Actions for ip4/6_icmp_redirect, ip4/6_pbr_icmp_redirect are:
 * 0: FORWARD, 1: DROP, 2: TRAP2CPU, 3: COPY2CPU, 4: TRAP2MASTERCPU 5: COPY2MASTERCPU
 * 6: HARDDROP
 * idx is the index in the HW interface table: idx < 0x80
 */
static void rtl930x_set_l3_egress_intf(int idx, struct rtl838x_l3_intf *intf)
{
	u32 u, v;
	/* Read L3_EGR_INTF table (4) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 4);

	/* The table has 2 registers */
	u = (intf->vid & 0xfff) << 9;
	u |= (intf->smac_idx & 0x3f) << 3;
	u |= (intf->ip4_mtu_id & 0x7);

	v = (intf->ip6_mtu_id & 0x7) << 28;
	v |= (intf->ttl_scope & 0xff) << 20;
	v |= (intf->hl_scope & 0xff) << 12;
	v |= (intf->ip4_icmp_redirect & 0x7) << 9;
	v |= (intf->ip6_icmp_redirect & 0x7)<< 6;
	v |= (intf->ip4_pbr_icmp_redirect & 0x7) << 3;
	v |= (intf->ip6_pbr_icmp_redirect & 0x7);

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));

	pr_debug("%s writing to index %d: %08x %08x\n", __func__, idx, u, v);
	rtl_table_write(r, idx & 0x7f);
	rtl_table_release(r);
}

/* Reads a MAC entry for L3 termination as entry point for routing
 * from the hardware table
 * idx is the index into the L3_ROUTER_MAC table
 */
static void rtl930x_get_l3_router_mac(u32 idx, struct rtl93xx_rt_mac *m)
{
	u32 v, w;
	/* Read L3_ROUTER_MAC table (0) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 0);

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

/* Writes a MAC entry for L3 termination as entry point for routing
 * into the hardware table
 * idx is the index into the L3_ROUTER_MAC table
 */
static void rtl930x_set_l3_router_mac(u32 idx, struct rtl93xx_rt_mac *m)
{
	u32 v, w;
	/* Read L3_ROUTER_MAC table (0) via register RTL9300_TBL_1 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_1, 0);

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

	pr_debug("%s writing index %d: %08x %08x %08x %08x %08x %08x %08x\n", __func__, idx,
		sw_r32(rtl_table_data(r, 0)), sw_r32(rtl_table_data(r, 1)), sw_r32(rtl_table_data(r, 2)),
		sw_r32(rtl_table_data(r, 3)), sw_r32(rtl_table_data(r, 4)), sw_r32(rtl_table_data(r, 5)),
		sw_r32(rtl_table_data(r, 6))
	);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Get the Destination-MAC of an L3 egress interface or the Source MAC for routed packets
 * from the SoC's L3_EGR_INTF_MAC table
 * Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
static u64 rtl930x_get_l3_egress_mac(u32 idx)
{
	u64 mac;
	/* Read L3_EGR_INTF_MAC table (2) via register RTL9300_TBL_2 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_2, 2);

	rtl_table_read(r, idx);
	/* The table has a size of 2 registers */
	mac = sw_r32(rtl_table_data(r, 0));
	mac <<= 32;
	mac |= sw_r32(rtl_table_data(r, 1));
	rtl_table_release(r);

	return mac;
}

/* Set the Destination-MAC of a route or the Source MAC of an L3 egress interface
 * in the SoC's L3_EGR_INTF_MAC table
 * Indexes 0-2047 are DMACs, 2048+ are SMACs
 */
static void rtl930x_set_l3_egress_mac(u32 idx, u64 mac)
{
	/* Access L3_EGR_INTF_MAC table (2) via register RTL9300_TBL_2 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_2, 2);

	/* The table has a size of 2 registers */
	sw_w32(mac >> 32, rtl_table_data(r, 0));
	sw_w32(mac, rtl_table_data(r, 1));

	pr_debug("%s: setting index %d to %016llx\n", __func__, idx, mac);
	rtl_table_write(r, idx);
	rtl_table_release(r);
}

/* Configure L3 routing settings of the device:
 * - MTUs
 * - Egress interface
 * - The router's MAC address on which routed packets are expected
 * - MAC addresses used as source macs of routed packets
 */
static int rtl930x_l3_setup(struct rtl838x_switch_priv *priv)
{
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
		rtl930x_set_l3_egress_mac(L3_EGRESS_DMACS + i, 0ULL);

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
	set_bit(0, priv->route_use_bm);

	return 0;
}

static u32 rtl930x_packet_cntr_read(int counter)
{
	u32 v;

	/* Read LOG table (3) via register RTL9300_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 3);

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

static void rtl930x_packet_cntr_clear(int counter)
{
	/* Access LOG table (3) via register RTL9300_TBL_0 */
	struct table_reg *r = rtl_table_get(RTL9300_TBL_0, 3);

	pr_debug("In %s, id %d\n", __func__, counter);
	/* The table has a size of 2 registers */
	if (counter % 2)
		sw_w32(0, rtl_table_data(r, 0));
	else
		sw_w32(0, rtl_table_data(r, 1));

	rtl_table_write(r, counter / 2);

	rtl_table_release(r);
}

static void rtl930x_vlan_port_keep_tag_set(int port, bool keep_outer, bool keep_inner)
{
	sw_w32(FIELD_PREP(RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_OTAG_STS_MASK,
			  keep_outer ? RTL930X_VLAN_PORT_TAG_STS_TAGGED : RTL930X_VLAN_PORT_TAG_STS_UNTAG) |
	       FIELD_PREP(RTL930X_VLAN_PORT_TAG_STS_CTRL_EGR_ITAG_STS_MASK,
			  keep_inner ? RTL930X_VLAN_PORT_TAG_STS_TAGGED : RTL930X_VLAN_PORT_TAG_STS_UNTAG),
	       RTL930X_VLAN_PORT_TAG_STS_CTRL(port));
}

static void rtl930x_vlan_port_pvidmode_set(int port, enum pbvlan_type type, enum pbvlan_mode mode)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0x3, mode, RTL930X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0x3 << 14, mode << 14 ,RTL930X_VLAN_PORT_PB_VLAN + (port << 2));
}

static void rtl930x_vlan_port_pvid_set(int port, enum pbvlan_type type, int pvid)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0xfff << 2, pvid << 2, RTL930X_VLAN_PORT_PB_VLAN + (port << 2));
	else
		sw_w32_mask(0xfff << 16, pvid << 16, RTL930X_VLAN_PORT_PB_VLAN + (port << 2));
}

static int rtl930x_set_ageing_time(unsigned long msec)
{
	int t = sw_r32(RTL930X_L2_AGE_CTRL);

	t &= 0x1FFFFF;
	t = (t * 7) / 10;
	pr_debug("L2 AGING time: %d sec\n", t);

	t = (msec / 100 + 6) / 7;
	t = t > 0x1FFFFF ? 0x1FFFFF : t;
	sw_w32_mask(0x1FFFFF, t, RTL930X_L2_AGE_CTRL);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(RTL930X_L2_PORT_AGE_CTRL));

	return 0;
}

static void rtl930x_set_igr_filter(int port,  enum igr_filter state)
{
	sw_w32_mask(0x3 << ((port & 0xf)<<1), state << ((port & 0xf)<<1),
		    RTL930X_VLAN_PORT_IGR_FLTR + (((port >> 4) << 2)));
}

static void rtl930x_set_egr_filter(int port,  enum egr_filter state)
{
	sw_w32_mask(0x1 << (port % 0x1D), state << (port % 0x1D),
		    RTL930X_VLAN_PORT_EGR_FLTR + (((port / 29) << 2)));
}

static void rtl930x_set_distribution_algorithm(int group, int algoidx, u32 algomsk)
{
	u32 l3shift = 0;
	u32 newmask = 0;

	/* TODO: for now we set algoidx to 0 */
	algoidx = 0;
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SIP_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_DIP_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT) {
		l3shift = 4;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT;
	}

	if (l3shift == 4) {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT;

		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT;
	} else  {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT;
	}

	sw_w32(newmask << l3shift, RTL930X_TRK_HASH_CTRL + (algoidx << 2));
}

static void rtl930x_led_init(struct rtl838x_switch_priv *priv)
{
	struct device_node *node;
	u32 pm = 0;

	pr_debug("%s called\n", __func__);
	node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300-leds");
	if (!node) {
		pr_debug("%s No compatible LED node found\n", __func__);
		return;
	}

	for (int set = 0; set < 4; set++) {
		char set_name[16] = {0};
		u32 set_config[4];
		int leds_in_this_set = 0;

		/* Reset LED set configuration */
		sw_w32(0, RTL930X_LED_SETX_0_CTRL(set));
		sw_w32(0, RTL930X_LED_SETX_1_CTRL(set));

		/**
		 * Each led set has 4 number of leds, and each LED is configured with 16 bits
		 * So each 32bit register holds configuration for 2 leds
		 * And therefore each set requires 2 registers for configuring 4 LEDs
		 *
		*/
		sprintf(set_name, "led_set%d", set);
		leds_in_this_set = of_property_count_u32_elems(node, set_name);

		if (leds_in_this_set == 0 || leds_in_this_set > sizeof(set_config)) {
			pr_err("%s led_set configuration invalid skipping over this set\n", __func__);
			continue;
		}

		if (of_property_read_u32_array(node, set_name, set_config, leds_in_this_set)) {
			break;
		}

		/* Write configuration as per number of LEDs */
		for (int i=0, led = leds_in_this_set-1; led >= 0; led--,i++) {
			sw_w32_mask(0xffff << RTL930X_LED_SET_LEDX_SHIFT(led),
						(0xffff & set_config[i]) << RTL930X_LED_SET_LEDX_SHIFT(led),
						RTL930X_LED_SETX_LEDY(set, led));
		}
	}

	for (int i = 0; i < priv->cpu_port; i++) {
		int pos = (i << 1) % 32;
		u32 set;

		sw_w32_mask(0x3 << pos, 0, RTL930X_LED_PORT_FIB_SET_SEL_CTRL(i));
		sw_w32_mask(0x3 << pos, 0, RTL930X_LED_PORT_COPR_SET_SEL_CTRL(i));

		if (!priv->ports[i].phy)
			continue;

		/* 0x0 = 1 led, 0x1 = 2 leds, 0x2 = 3 leds, 0x3 = 4 leds per port */
		sw_w32_mask(0x3 << pos, (priv->ports[i].leds_on_this_port -1) << pos, RTL930X_LED_PORT_NUM_CTRL(i));

		pm |= BIT(i);

		set = priv->ports[i].led_set;
		sw_w32_mask(0, set << pos, RTL930X_LED_PORT_COPR_SET_SEL_CTRL(i));
		sw_w32_mask(0, set << pos, RTL930X_LED_PORT_FIB_SET_SEL_CTRL(i));
	}

	/* Set LED mode to serial (0x1) */
	sw_w32_mask(0x3, 0x1, RTL930X_LED_GLB_CTRL);

	/* Set LED active state */
	if (of_property_read_bool(node, "active-low"))
		sw_w32_mask(RTL930X_LED_GLB_ACTIVE_LOW, 0, RTL930X_LED_GLB_CTRL);
	else
		sw_w32_mask(0, RTL930X_LED_GLB_ACTIVE_LOW, RTL930X_LED_GLB_CTRL);

	/* Set port type masks */
	sw_w32(pm, RTL930X_LED_PORT_COPR_MASK_CTRL);
	sw_w32(pm, RTL930X_LED_PORT_FIB_MASK_CTRL);
	sw_w32(pm, RTL930X_LED_PORT_COMBO_MASK_CTRL);

	for (int i = 0; i < 24; i++)
		pr_debug("%s %08x: %08x\n",__func__, 0xbb00cc00 + i * 4, sw_r32(0xcc00 + i * 4));
}

const struct rtl838x_reg rtl930x_reg = {
	.mask_port_reg_be = rtl838x_mask_port_reg,
	.set_port_reg_be = rtl838x_set_port_reg,
	.get_port_reg_be = rtl838x_get_port_reg,
	.mask_port_reg_le = rtl838x_mask_port_reg,
	.set_port_reg_le = rtl838x_set_port_reg,
	.get_port_reg_le = rtl838x_get_port_reg,
	.stat_port_rst = RTL930X_STAT_PORT_RST,
	.stat_rst = RTL930X_STAT_RST,
	.stat_port_std_mib = RTL930X_STAT_PORT_MIB_CNTR,
	.traffic_enable = rtl930x_traffic_enable,
	.traffic_disable = rtl930x_traffic_disable,
	.traffic_get = rtl930x_traffic_get,
	.traffic_set = rtl930x_traffic_set,
	.l2_ctrl_0 = RTL930X_L2_CTRL,
	.l2_ctrl_1 = RTL930X_L2_AGE_CTRL,
	.l2_port_aging_out = RTL930X_L2_PORT_AGE_CTRL,
	.set_ageing_time = rtl930x_set_ageing_time,
	.smi_poll_ctrl = RTL930X_SMI_POLL_CTRL, /* TODO: Difference to RTL9300_SMI_PRVTE_POLLING_CTRL */
	.l2_tbl_flush_ctrl = RTL930X_L2_TBL_FLUSH_CTRL,
	.exec_tbl0_cmd = rtl930x_exec_tbl0_cmd,
	.exec_tbl1_cmd = rtl930x_exec_tbl1_cmd,
	.tbl_access_data_0 = rtl930x_tbl_access_data_0,
	.isr_glb_src = RTL930X_ISR_GLB,
	.isr_port_link_sts_chg = RTL930X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL930X_IMR_PORT_LINK_STS_CHG,
	.imr_glb = RTL930X_IMR_GLB,
	.vlan_tables_read = rtl930x_vlan_tables_read,
	.vlan_set_tagged = rtl930x_vlan_set_tagged,
	.vlan_set_untagged = rtl930x_vlan_set_untagged,
	.vlan_profile_dump = rtl930x_vlan_profile_dump,
	.vlan_profile_setup = rtl930x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl930x_vlan_fwd_on_inner,
	.set_vlan_igr_filter = rtl930x_set_igr_filter,
	.set_vlan_egr_filter = rtl930x_set_egr_filter,
	.stp_get = rtl930x_stp_get,
	.stp_set = rtl930x_stp_set,
	.mac_force_mode_ctrl = rtl930x_mac_force_mode_ctrl,
	.mac_port_ctrl = rtl930x_mac_port_ctrl,
	.l2_port_new_salrn = rtl930x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl930x_l2_port_new_sa_fwd,
	.mir_ctrl = RTL930X_MIR_CTRL,
	.mir_dpm = RTL930X_MIR_DPM_CTRL,
	.mir_spm = RTL930X_MIR_SPM_CTRL,
	.mac_link_sts = RTL930X_MAC_LINK_STS,
	.mac_link_dup_sts = RTL930X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts = rtl930x_mac_link_spd_sts,
	.mac_rx_pause_sts = RTL930X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts = RTL930X_MAC_TX_PAUSE_STS,
	.read_l2_entry_using_hash = rtl930x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl930x_write_l2_entry_using_hash,
	.read_cam = rtl930x_read_cam,
	.write_cam = rtl930x_write_cam,
	.vlan_port_keep_tag_set = rtl930x_vlan_port_keep_tag_set,
	.vlan_port_pvidmode_set = rtl930x_vlan_port_pvidmode_set,
	.vlan_port_pvid_set = rtl930x_vlan_port_pvid_set,
	.trk_mbr_ctr = rtl930x_trk_mbr_ctr,
	.rma_bpdu_fld_pmask = RTL930X_RMA_BPDU_FLD_PMSK,
	.init_eee = rtl930x_init_eee,
	.set_mac_eee = rtldsa_930x_set_mac_eee,
	.l2_hash_seed = rtl930x_l2_hash_seed,
	.l2_hash_key = rtl930x_l2_hash_key,
	.read_mcast_pmask = rtl930x_read_mcast_pmask,
	.write_mcast_pmask = rtl930x_write_mcast_pmask,
	.pie_init = rtl930x_pie_init,
	.pie_rule_write = rtl930x_pie_rule_write,
	.pie_rule_add = rtl930x_pie_rule_add,
	.pie_rule_rm = rtl930x_pie_rule_rm,
	.l2_learning_setup = rtl930x_l2_learning_setup,
	.packet_cntr_read = rtl930x_packet_cntr_read,
	.packet_cntr_clear = rtl930x_packet_cntr_clear,
	.route_read = rtl930x_route_read,
	.route_write = rtl930x_route_write,
	.host_route_write = rtl930x_host_route_write,
	.l3_setup = rtl930x_l3_setup,
	.set_l3_nexthop = rtl930x_set_l3_nexthop,
	.get_l3_nexthop = rtl930x_get_l3_nexthop,
	.get_l3_egress_mac = rtl930x_get_l3_egress_mac,
	.set_l3_egress_mac = rtl930x_set_l3_egress_mac,
	.find_l3_slot = rtl930x_find_l3_slot,
	.route_lookup_hw = rtl930x_route_lookup_hw,
	.get_l3_router_mac = rtl930x_get_l3_router_mac,
	.set_l3_router_mac = rtl930x_set_l3_router_mac,
	.set_l3_egress_intf = rtl930x_set_l3_egress_intf,
	.set_distribution_algorithm = rtl930x_set_distribution_algorithm,
	.led_init = rtl930x_led_init,
};
