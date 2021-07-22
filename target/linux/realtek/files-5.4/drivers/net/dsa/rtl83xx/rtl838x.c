// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx.h"

extern struct mutex smi_lock;

void rtl838x_print_matrix(void)
{
	unsigned volatile int *ptr8;
	int i;

	ptr8 = RTL838X_SW_BASE + RTL838X_PORT_ISO_CTRL(0);
	for (i = 0; i < 28; i += 8)
		pr_debug("> %8x %8x %8x %8x %8x %8x %8x %8x\n",
			ptr8[i + 0], ptr8[i + 1], ptr8[i + 2], ptr8[i + 3],
			ptr8[i + 4], ptr8[i + 5], ptr8[i + 6], ptr8[i + 7]);
	pr_debug("CPU_PORT> %8x\n", ptr8[28]);
}

static inline int rtl838x_port_iso_ctrl(int p)
{
	return RTL838X_PORT_ISO_CTRL(p);
}

static inline void rtl838x_exec_tbl0_cmd(u32 cmd)
{
	sw_w32(cmd, RTL838X_TBL_ACCESS_CTRL_0);
	do { } while (sw_r32(RTL838X_TBL_ACCESS_CTRL_0) & BIT(15));
}

static inline void rtl838x_exec_tbl1_cmd(u32 cmd)
{
	sw_w32(cmd, RTL838X_TBL_ACCESS_CTRL_1);
	do { } while (sw_r32(RTL838X_TBL_ACCESS_CTRL_1) & BIT(15));
}

static inline int rtl838x_tbl_access_data_0(int i)
{
	return RTL838X_TBL_ACCESS_DATA_0(i);
}

static void rtl838x_vlan_tables_read(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v;
	// Read VLAN table (0) via register 0
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 0);

	rtl_table_read(r, vlan);
	info->tagged_ports = sw_r32(rtl_table_data(r, 0));
	v = sw_r32(rtl_table_data(r, 1));
	pr_debug("VLAN_READ %d: %016llx %08x\n", vlan, info->tagged_ports, v);
	rtl_table_release(r);

	info->profile_id = v & 0x7;
	info->hash_mc_fid = !!(v & 0x8);
	info->hash_uc_fid = !!(v & 0x10);
	info->fid = (v >> 5) & 0x3f;

	// Read UNTAG table (0) via table register 1
	r = rtl_table_get(RTL8380_TBL_1, 0);
	rtl_table_read(r, vlan);
	info->untagged_ports = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);
}

static void rtl838x_vlan_set_tagged(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v;
	// Access VLAN table (0) via register 0
	struct table_reg *r = rtl_table_get(RTL8380_TBL_0, 0);

	sw_w32(info->tagged_ports, rtl_table_data(r, 0));

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
	// Access UNTAG table (0) via register 1
	struct table_reg *r = rtl_table_get(RTL8380_TBL_1, 0);

	sw_w32(portmask & 0x1fffffff, rtl_table_data(r, 0));
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

/*
 * Applies the same hash algorithm as the one used currently by the ASIC to the seed
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
		h = ((seed >> 55) & 0x1ff) ^ ((seed >> 44) & 0x7ff)
			^ ((seed >> 33) & 0x7ff) ^ ((seed >> 22) & 0x7ff)
			^ ((seed >> 11) & 0x7ff) ^ (seed & 0x7ff);
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

static inline int rtl838x_mac_link_spd_sts(int p)
{
	return RTL838X_MAC_LINK_SPD_STS(p);
}

inline static int rtl838x_trk_mbr_ctr(int group)
{
	return RTL838X_TRK_MBR_CTR + (group << 2);
}

/*
 * Fills an L2 entry structure from the SoC registers
 */
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
				pr_info("Found next hop entry, need to read extra data\n");
				e->nh_vlan_target = !!(r[0] & BIT(9));
				e->nh_route_id = r[0] & 0x1ff;
			}
			e->age = (r[0] >> 17) & 0x3;
			e->valid = true;
			
			/* A valid entry has one of mutli-cast, aging, sa/da-blocking,
			 * next-hop or static entry bit set */
			if (!(r[0] & 0x007c0000) && !(r[1] & 0xd0000000))
				e->valid = false;
			else
				e->type = L2_UNICAST;
		} else { // L2 multicast
			pr_info("Got L2 MC entry: %08x %08x %08x\n", r[0], r[1], r[2]);
			e->valid = true;
			e->type = L2_MULTICAST;
			e->mc_portmask_index = (r[0] >> 12) & 0x1ff;
		}
	} else { // IPv4 and IPv6 multicast
		e->valid = true;
		e->mc_portmask_index = (r[0] >> 12) & 0x1ff;
		e->mc_gip = r[1];
		e->mc_sip = r[2];
		e->rvid = r[0] & 0xfff;
	}
	if (e->is_ip_mc)
		e->type = IP4_MULTICAST;
	if (e->is_ipv6_mc)
		e->type = IP6_MULTICAST;
}

/*
 * Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry
 */
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
				r[0] |= e->nh_route_id &0x1ff;
			}
			r[0] |= (e->age & 0x3) << 17;
		} else { // L2 Multicast
			r[0] |= (e->mc_portmask_index & 0x1ff) << 12;
			r[2] |= e->rvid & 0xfff;
			r[0] |= e->vid & 0xfff;
			pr_info("FILL MC: %08x %08x %08x\n", r[0], r[1], r[2]);
		}
	} else { // IPv4 and IPv6 multicast
		r[1] = e->mc_gip;
		r[2] = e->mc_sip;
		r[0] |= e->rvid;
	}
}

/*
 * Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl838x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u64 entry;
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 0); // Access L2 Table 0
	u32 idx = (0 << 14) | (hash << 2) | pos; // Search SRAM, with hash and at pos in bucket
	int i;

	rtl_table_read(q, idx);
	for (i= 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl838x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	entry = (((u64) r[1]) << 32) | (r[2] & 0xfffff000) | (r[0] & 0xfff);
	return entry;
}

static void rtl838x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 0);
	int i;

	u32 idx = (0 << 14) | (hash << 2) | pos; // Access SRAM, with hash and at pos in bucket

	rtl838x_fill_l2_row(r, e);

	for (i= 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl838x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	u64 entry;
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 1); // Access L2 Table 1
	int i;

	rtl_table_read(q, idx);
	for (i= 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl838x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	pr_debug("Found in CAM: R1 %x R2 %x R3 %x\n", r[0], r[1], r[2]);

	// Return MAC with concatenated VID ac concatenated ID
	entry = (((u64) r[1]) << 32) | (r[2] & 0xfffff000) | (r[0] & 0xfff);
	return entry;
}

static void rtl838x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 1); // Access L2 Table 1
	int i;

	rtl838x_fill_l2_row(r, e);

	for (i= 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl838x_read_mcast_pmask(int idx)
{
	u32 portmask;
	// Read MC_PMSK (2) via register RTL8380_TBL_L2
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	rtl_table_release(q);

	return portmask;
}

static void rtl838x_write_mcast_pmask(int idx, u64 portmask)
{
	// Access MC_PMSK (2) via register RTL8380_TBL_L2
	struct table_reg *q = rtl_table_get(RTL8380_TBL_L2, 2);

	sw_w32(((u32)portmask) & 0x1fffffff, rtl_table_data(q, 0));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static void rtl838x_vlan_profile_setup(int profile)
{
	u32 pmask_id = UNKNOWN_MC_PMASK;
	// Enable L2 Learning BIT 0, portmask UNKNOWN_MC_PMASK for unknown MC traffic flooding
	u32 p = 1 | pmask_id << 1 | pmask_id << 10 | pmask_id << 19;

	sw_w32(p, RTL838X_VLAN_PROFILE(profile));

	/* RTL8380 and RTL8390 use an index into the portmask table to set the
	 * unknown multicast portmask, setup a default at a safe location
	 * On RTL93XX, the portmask is directly set in the profile,
	 * see e.g. rtl9300_vlan_profile_setup
	 */
	rtl838x_write_mcast_pmask(UNKNOWN_MC_PMASK, 0x1fffffff);
}

static inline int rtl838x_vlan_port_egr_filter(int port)
{
	return RTL838X_VLAN_PORT_EGR_FLTR;
}

static inline int rtl838x_vlan_port_igr_filter(int port)
{
	return RTL838X_VLAN_PORT_IGR_FLTR(port);
}

static void rtl838x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	int i;
	u32 cmd = 1 << 15 /* Execute cmd */
		| 1 << 14 /* Read */
		| 2 << 12 /* Table type 0b10 */
		| (msti & 0xfff);
	priv->r->exec_tbl0_cmd(cmd);

	for (i = 0; i < 2; i++)
		port_state[i] = sw_r32(priv->r->tbl_access_data_0(i));
}

static void rtl838x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	int i;
	u32 cmd = 1 << 15 /* Execute cmd */
		| 0 << 14 /* Write */
		| 2 << 12 /* Table type 0b10 */
		| (msti & 0xfff);

	for (i = 0; i < 2; i++)
		sw_w32(port_state[i], priv->r->tbl_access_data_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

u64 rtl838x_traffic_get(int source)
{
	return rtl838x_get_port_reg(rtl838x_port_iso_ctrl(source));
}

void rtl838x_traffic_set(int source, u64 dest_matrix)
{
	rtl838x_set_port_reg(dest_matrix, rtl838x_port_iso_ctrl(source));
}

void rtl838x_traffic_enable(int source, int dest)
{
	rtl838x_mask_port_reg(0, BIT(dest), rtl838x_port_iso_ctrl(source));
}

void rtl838x_traffic_disable(int source, int dest)
{
	rtl838x_mask_port_reg(BIT(dest), 0, rtl838x_port_iso_ctrl(source));
}

/*
 * Enables or disables the EEE/EEEP capability of a port
 */
static void rtl838x_port_eee_set(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;

	// This works only for Ethernet ports, and on the RTL838X, ports from 24 are SFP
	if (port >= 24)
		return;

	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? 0x3 : 0x0;

	// Set EEE state for 100 (bit 9) & 1000MBit (bit 10)
	sw_w32_mask(0x3 << 9, v << 9, priv->r->mac_force_mode_ctrl(port));

	// Set TX/RX EEE state
	if (enable) {
		sw_w32_mask(0, BIT(port), RTL838X_EEE_PORT_TX_EN);
		sw_w32_mask(0, BIT(port), RTL838X_EEE_PORT_RX_EN);
	} else {
		sw_w32_mask(BIT(port), 0, RTL838X_EEE_PORT_TX_EN);
		sw_w32_mask(BIT(port), 0, RTL838X_EEE_PORT_RX_EN);
	}
	priv->ports[port].eee_enabled = enable;
}


/*
 * Get EEE own capabilities and negotiation result
 */
static int rtl838x_eee_port_ability(struct rtl838x_switch_priv *priv,
				    struct ethtool_eee *e, int port)
{
	u64 link;

	if (port >= 24)
		return 0;

	link = rtl839x_get_port_reg_le(RTL838X_MAC_LINK_STS);
	if (!(link & BIT(port)))
		return 0;

	if (sw_r32(rtl838x_mac_force_mode_ctrl(port)) & BIT(9))
		e->advertised |= ADVERTISED_100baseT_Full;

	if (sw_r32(rtl838x_mac_force_mode_ctrl(port)) & BIT(10))
		e->advertised |= ADVERTISED_1000baseT_Full;

	if (sw_r32(RTL838X_MAC_EEE_ABLTY) & BIT(port)) {
		e->lp_advertised = ADVERTISED_100baseT_Full;
		e->lp_advertised |= ADVERTISED_1000baseT_Full;
		return 1;
	}

	return 0;
}

static void rtl838x_init_eee(struct rtl838x_switch_priv *priv, bool enable)
{
	int i;

	pr_info("Setting up EEE, state: %d\n", enable);
	sw_w32_mask(0x4, 0, RTL838X_SMI_GLB_CTRL);

	/* Set timers for EEE */
	sw_w32(0x5001411, RTL838X_EEE_TX_TIMER_GIGA_CTRL);
	sw_w32(0x5001417, RTL838X_EEE_TX_TIMER_GELITE_CTRL);

	// Enable EEE MAC support on ports
	for (i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			rtl838x_port_eee_set(priv, i, enable);
	}
	priv->eee_enabled = enable;
}

const struct rtl838x_reg rtl838x_reg = {
	.mask_port_reg_be = rtl838x_mask_port_reg,
	.set_port_reg_be = rtl838x_set_port_reg,
	.get_port_reg_be = rtl838x_get_port_reg,
	.mask_port_reg_le = rtl838x_mask_port_reg,
	.set_port_reg_le = rtl838x_set_port_reg,
	.get_port_reg_le = rtl838x_get_port_reg,
	.stat_port_rst = RTL838X_STAT_PORT_RST,
	.stat_rst = RTL838X_STAT_RST,
	.stat_port_std_mib = RTL838X_STAT_PORT_STD_MIB,
	.port_iso_ctrl = rtl838x_port_iso_ctrl,
	.traffic_enable = rtl838x_traffic_enable,
	.traffic_disable = rtl838x_traffic_disable,
	.traffic_get = rtl838x_traffic_get,
	.traffic_set = rtl838x_traffic_set,
	.l2_ctrl_0 = RTL838X_L2_CTRL_0,
	.l2_ctrl_1 = RTL838X_L2_CTRL_1,
	.l2_port_aging_out = RTL838X_L2_PORT_AGING_OUT,
	.smi_poll_ctrl = RTL838X_SMI_POLL_CTRL,
	.l2_tbl_flush_ctrl = RTL838X_L2_TBL_FLUSH_CTRL,
	.exec_tbl0_cmd = rtl838x_exec_tbl0_cmd,
	.exec_tbl1_cmd = rtl838x_exec_tbl1_cmd,
	.tbl_access_data_0 = rtl838x_tbl_access_data_0,
	.isr_glb_src = RTL838X_ISR_GLB_SRC,
	.isr_port_link_sts_chg = RTL838X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL838X_IMR_PORT_LINK_STS_CHG,
	.imr_glb = RTL838X_IMR_GLB,
	.vlan_tables_read = rtl838x_vlan_tables_read,
	.vlan_set_tagged = rtl838x_vlan_set_tagged,
	.vlan_set_untagged = rtl838x_vlan_set_untagged,
	.mac_force_mode_ctrl = rtl838x_mac_force_mode_ctrl,
	.vlan_profile_dump = rtl838x_vlan_profile_dump,
	.vlan_profile_setup = rtl838x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl838x_vlan_fwd_on_inner,
	.stp_get = rtl838x_stp_get,
	.stp_set = rtl838x_stp_set,
	.mac_port_ctrl = rtl838x_mac_port_ctrl,
	.l2_port_new_salrn = rtl838x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl838x_l2_port_new_sa_fwd,
	.mir_ctrl = RTL838X_MIR_CTRL,
	.mir_dpm = RTL838X_MIR_DPM_CTRL,
	.mir_spm = RTL838X_MIR_SPM_CTRL,
	.mac_link_sts = RTL838X_MAC_LINK_STS,
	.mac_link_dup_sts = RTL838X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts = rtl838x_mac_link_spd_sts,
	.mac_rx_pause_sts = RTL838X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts = RTL838X_MAC_TX_PAUSE_STS,
	.read_l2_entry_using_hash = rtl838x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl838x_write_l2_entry_using_hash,
	.read_cam = rtl838x_read_cam,
	.write_cam = rtl838x_write_cam,
	.vlan_port_egr_filter = RTL838X_VLAN_PORT_EGR_FLTR,
	.vlan_port_igr_filter = RTL838X_VLAN_PORT_IGR_FLTR(0),
	.vlan_port_pb = RTL838X_VLAN_PORT_PB_VLAN,
	.vlan_port_tag_sts_ctrl = RTL838X_VLAN_PORT_TAG_STS_CTRL,
	.trk_mbr_ctr = rtl838x_trk_mbr_ctr,
	.rma_bpdu_fld_pmask = RTL838X_RMA_BPDU_FLD_PMSK,
	.spcl_trap_eapol_ctrl = RTL838X_SPCL_TRAP_EAPOL_CTRL,
	.init_eee = rtl838x_init_eee,
	.port_eee_set = rtl838x_port_eee_set,
	.eee_port_ability = rtl838x_eee_port_ability,
	.l2_hash_seed = rtl838x_l2_hash_seed, 
	.l2_hash_key = rtl838x_l2_hash_key,
	.read_mcast_pmask = rtl838x_read_mcast_pmask,
	.write_mcast_pmask = rtl838x_write_mcast_pmask,
};

irqreturn_t rtl838x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 status = sw_r32(RTL838X_ISR_GLB_SRC);
	u32 ports = sw_r32(RTL838X_ISR_PORT_LINK_STS_CHG);
	u32 link;
	int i;

	/* Clear status */
	sw_w32(ports, RTL838X_ISR_PORT_LINK_STS_CHG);
	pr_info("RTL8380 Link change: status: %x, ports %x\n", status, ports);

	for (i = 0; i < 28; i++) {
		if (ports & BIT(i)) {
			link = sw_r32(RTL838X_MAC_LINK_STS);
			if (link & BIT(i))
				dsa_port_phylink_mac_change(ds, i, true);
			else
				dsa_port_phylink_mac_change(ds, i, false);
		}
	}
	return IRQ_HANDLED;
}

int rtl838x_smi_wait_op(int timeout)
{
	do {
		timeout--;
		udelay(10);
	} while ((sw_r32(RTL838X_SMI_ACCESS_PHY_CTRL_1) & 0x1) && (timeout >= 0));
	if (timeout <= 0)
		return -1;
	return 0;
}

/*
 * Reads a register in a page from the PHY
 */
int rtl838x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;
	u32 park_page;

	if (port > 31) {
		*val = 0xffff;
		return 0;
	}

	if (page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	sw_w32_mask(0xffff0000, port << 16, RTL838X_SMI_ACCESS_PHY_CTRL_2);

	park_page = sw_r32(RTL838X_SMI_ACCESS_PHY_CTRL_1) & ((0x1f << 15) | 0x2);
	v = reg << 20 | page << 3;
	sw_w32(v | park_page, RTL838X_SMI_ACCESS_PHY_CTRL_1);
	sw_w32_mask(0, 1, RTL838X_SMI_ACCESS_PHY_CTRL_1);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	*val = sw_r32(RTL838X_SMI_ACCESS_PHY_CTRL_2) & 0xffff;

	mutex_unlock(&smi_lock);
	return 0;

timeout:
	mutex_unlock(&smi_lock);
	return -ETIMEDOUT;
}

/*
 * Write to a register in a page of the PHY
 */
int rtl838x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	u32 park_page;

	val &= 0xffff;
	if (port > 31 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);
	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	sw_w32(BIT(port), RTL838X_SMI_ACCESS_PHY_CTRL_0);
	mdelay(10);

	sw_w32_mask(0xffff0000, val << 16, RTL838X_SMI_ACCESS_PHY_CTRL_2);

	park_page = sw_r32(RTL838X_SMI_ACCESS_PHY_CTRL_1) & ((0x1f << 15) | 0x2);
	v = reg << 20 | page << 3 | 0x4;
	sw_w32(v | park_page, RTL838X_SMI_ACCESS_PHY_CTRL_1);
	sw_w32_mask(0, 1, RTL838X_SMI_ACCESS_PHY_CTRL_1);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	mutex_unlock(&smi_lock);
	return 0;

timeout:
	mutex_unlock(&smi_lock);
	return -ETIMEDOUT;
}

/*
 * Read an mmd register of a PHY
 */
int rtl838x_read_mmd_phy(u32 port, u32 addr, u32 reg, u32 *val)
{
	u32 v;

	mutex_lock(&smi_lock);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	sw_w32(1 << port, RTL838X_SMI_ACCESS_PHY_CTRL_0);
	mdelay(10);

	sw_w32_mask(0xffff0000, port << 16, RTL838X_SMI_ACCESS_PHY_CTRL_2);

	v = addr << 16 | reg;
	sw_w32(v, RTL838X_SMI_ACCESS_PHY_CTRL_3);

	/* mmd-access | read | cmd-start */
	v = 1 << 1 | 0 << 2 | 1;
	sw_w32(v, RTL838X_SMI_ACCESS_PHY_CTRL_1);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	*val = sw_r32(RTL838X_SMI_ACCESS_PHY_CTRL_2) & 0xffff;

	mutex_unlock(&smi_lock);
	return 0;

timeout:
	mutex_unlock(&smi_lock);
	return -ETIMEDOUT;
}

/*
 * Write to an mmd register of a PHY
 */
int rtl838x_write_mmd_phy(u32 port, u32 addr, u32 reg, u32 val)
{
	u32 v;

	pr_debug("MMD write: port %d, dev %d, reg %d, val %x\n", port, addr, reg, val);
	val &= 0xffff;
	mutex_lock(&smi_lock);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	sw_w32(1 << port, RTL838X_SMI_ACCESS_PHY_CTRL_0);
	mdelay(10);

	sw_w32_mask(0xffff0000, val << 16, RTL838X_SMI_ACCESS_PHY_CTRL_2);

	sw_w32_mask(0x1f << 16, addr << 16, RTL838X_SMI_ACCESS_PHY_CTRL_3);
	sw_w32_mask(0xffff, reg, RTL838X_SMI_ACCESS_PHY_CTRL_3);
	/* mmd-access | write | cmd-start */
	v = 1 << 1 | 1 << 2 | 1;
	sw_w32(v, RTL838X_SMI_ACCESS_PHY_CTRL_1);

	if (rtl838x_smi_wait_op(10000))
		goto timeout;

	mutex_unlock(&smi_lock);
	return 0;

timeout:
	mutex_unlock(&smi_lock);
	return -ETIMEDOUT;
}

void rtl8380_get_version(struct rtl838x_switch_priv *priv)
{
	u32 rw_save, info_save;
	u32 info;

	rw_save = sw_r32(RTL838X_INT_RW_CTRL);
	sw_w32(rw_save | 0x3, RTL838X_INT_RW_CTRL);

	info_save = sw_r32(RTL838X_CHIP_INFO);
	sw_w32(info_save | 0xA0000000, RTL838X_CHIP_INFO);

	info = sw_r32(RTL838X_CHIP_INFO);
	sw_w32(info_save, RTL838X_CHIP_INFO);
	sw_w32(rw_save, RTL838X_INT_RW_CTRL);

	if ((info & 0xFFFF) == 0x6275) {
		if (((info >> 16) & 0x1F) == 0x1)
			priv->version = RTL8380_VERSION_A;
		else if (((info >> 16) & 0x1F) == 0x2)
			priv->version = RTL8380_VERSION_B;
		else
			priv->version = RTL8380_VERSION_B;
	} else {
		priv->version = '-';
	}
}

void rtl838x_vlan_profile_dump(int profile)
{
	u32 p;

	if (profile < 0 || profile > 7)
		return;

	p = sw_r32(RTL838X_VLAN_PROFILE(profile));

	pr_info("VLAN profile %d: L2 learning: %d, UNKN L2MC FLD PMSK %d, \
		UNKN IPMC FLD PMSK %d, UNKN IPv6MC FLD PMSK: %d",
		profile, p & 1, (p >> 1) & 0x1ff, (p >> 10) & 0x1ff, (p >> 19) & 0x1ff);
}

void rtl8380_sds_rst(int mac)
{
	u32 offset = (mac == 24) ? 0 : 0x100;

	sw_w32_mask(1 << 11, 0, RTL838X_SDS4_FIB_REG0 + offset);
	sw_w32_mask(0x3, 0, RTL838X_SDS4_REG28 + offset);
	sw_w32_mask(0x3, 0x3, RTL838X_SDS4_REG28 + offset);
	sw_w32_mask(0, 0x1 << 6, RTL838X_SDS4_DUMMY0 + offset);
	sw_w32_mask(0x1 << 6, 0, RTL838X_SDS4_DUMMY0 + offset);
	pr_debug("SERDES reset: %d\n", mac);
}

int rtl8380_sds_power(int mac, int val)
{
	u32 mode = (val == 1) ? 0x4 : 0x9;
	u32 offset = (mac == 24) ? 5 : 0;

	if ((mac != 24) && (mac != 26)) {
		pr_err("%s: not a fibre port: %d\n", __func__, mac);
		return -1;
	}

	sw_w32_mask(0x1f << offset, mode << offset, RTL838X_SDS_MODE_SEL);

	rtl8380_sds_rst(mac);

	return 0;
}
