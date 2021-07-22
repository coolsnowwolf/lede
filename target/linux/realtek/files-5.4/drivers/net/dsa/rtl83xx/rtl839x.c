// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx.h"

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;

void rtl839x_print_matrix(void)
{
	volatile u64 *ptr9;
	int i;

	ptr9 = RTL838X_SW_BASE + RTL839X_PORT_ISO_CTRL(0);
	for (i = 0; i < 52; i += 4)
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
	// Read VLAN table (0) via register 0
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

	// Read UNTAG table (0) via table register 1
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
	// Access VLAN table (0) via register 0
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

	// Access UNTAG table (0) via table register 1
	struct table_reg *r = rtl_table_get(RTL8390_TBL_1, 0);

	u = portmask >> 21;
	v = portmask << 11;

	sw_w32(u, rtl_table_data(r, 0));
	sw_w32(v, rtl_table_data(r, 1));
	rtl_table_write(r, vlan);

	rtl_table_release(r);
}

/* Sets the L2 forwarding to be based on either the inner VLAN tag or the outer
 */
static void rtl839x_vlan_fwd_on_inner(int port, bool is_set)
{
	if (is_set)
		rtl839x_mask_port_reg_be(BIT_ULL(port), 0ULL, RTL839X_VLAN_PORT_FWD);
	else
		rtl839x_mask_port_reg_be(0ULL, BIT_ULL(port), RTL839X_VLAN_PORT_FWD);
}

/*
 * Hash seed is vid (actually rvid) concatenated with the MAC address
 */
static u64 rtl839x_l2_hash_seed(u64 mac, u32 vid)
{
	u64 v = vid;

	v <<= 48;
	v |= mac;

	return v;
}

/*
 * Applies the same hash algorithm as the one used currently by the ASIC to the seed
 * and returns a key into the L2 hash table
 */
static u32 rtl839x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 h1, h2, h;

	if (sw_r32(priv->r->l2_ctrl_0) & 1) {
		h1 = (u32) (((seed >> 60) & 0x3f) ^ ((seed >> 54) & 0x3f)
				^ ((seed >> 36) & 0x3f) ^ ((seed >> 30) & 0x3f)
				^ ((seed >> 12) & 0x3f) ^ ((seed >> 6) & 0x3f));
		h2 = (u32) (((seed >> 48) & 0x3f) ^ ((seed >> 42) & 0x3f)
				^ ((seed >> 24) & 0x3f) ^ ((seed >> 18) & 0x3f)
				^ (seed & 0x3f));
		h = (h1 << 6) | h2;
	} else {
		h = (seed >> 60)
			^ ((((seed >> 48) & 0x3f) << 6) | ((seed >> 54) & 0x3f))
			^ ((seed >> 36) & 0xfff) ^ ((seed >> 24) & 0xfff)
			^ ((seed >> 12) & 0xfff) ^ (seed & 0xfff);
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
	if (!e->is_ip_mc) {
		e->mac[0] = (r[0] >> 12);
		e->mac[1] = (r[0] >> 4);
		e->mac[2] = ((r[1] >> 28) | (r[0] << 4));
		e->mac[3] = (r[1] >> 20);
		e->mac[4] = (r[1] >> 12);
		e->mac[5] = (r[1] >> 4);

		/* Is it a unicast entry? check multicast bit */
		if (!(e->mac[0] & 1)) {
			e->is_static = !!((r[2] >> 18) & 1);
			e->vid = (r[2] >> 4) & 0xfff;
			e->rvid = (r[0] >> 20) & 0xfff;
			e->port = (r[2] >> 24) & 0x3f;
			e->block_da = !!(r[2] & (1 << 19));
			e->block_sa = !!(r[2] & (1 << 20));
			e->suspended = !!(r[2] & (1 << 17));
			e->next_hop = !!(r[2] & (1 << 16));
			if (e->next_hop)
				pr_info("Found next hop entry, need to read data\n");
			e->age = (r[2] >> 21) & 3;
			e->valid = true;
			if (!(r[2] & 0xc0fd0000)) /* Check for valid entry */
				e->valid = false;
			else
				e->type = L2_UNICAST;
		} else {
			e->valid = true;
			e->type = L2_MULTICAST;
			e->mc_portmask_index = (r[2]>>6) & 0xfff;
		}
	}
	if (e->is_ip_mc) {
		e->valid = true;
		e->type = IP4_MULTICAST;
	}
	if (e->is_ipv6_mc) {
		e->valid = true;
		e->type = IP6_MULTICAST;
	}
}

/*
 * Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry
 */
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

		if (!(e->mac[0] & 1)) { // Not multicast
			r[2] |= e->is_static ? BIT(18) : 0;
			r[2] |= e->vid << 4;
			r[0] |= ((u32)e->rvid) << 20;
			r[2] |= e->port << 24;
			r[2] |= e->block_da ? BIT(19) : 0;
			r[2] |= e->block_sa ? BIT(20) : 0;
			r[2] |= e->suspended ? BIT(17) : 0;
			if (e->next_hop) {
				r[2] |= BIT(16);
				r[2] |= e->nh_vlan_target ? BIT(15) : 0;
				r[2] |= (e->nh_route_id & 0x7ff) << 4;
			}
			r[2] |= ((u32)e->age) << 21;
		} else {  // L2 Multicast
			r[0] |= ((u32)e->rvid) << 20;
			r[2] |= ((u32)e->mc_portmask_index) << 6;
			pr_debug("Write L2 MC entry: %08x %08x %08x\n", r[0], r[1], r[2]);
		}
	} else { // IPv4 or IPv6 MC entry
		r[0] = ((u32)e->rvid) << 20;
		r[2] |= ((u32)e->mc_portmask_index) << 6;
		r[1] = e->mc_gip;
	}
}

/*
 * Read an L2 UC or MC entry out of a hash bucket of the L2 forwarding table
 * hash is the id of the bucket and pos is the position of the entry in that bucket
 * The data read from the SoC is filled into rtl838x_l2_entry
 */
static u64 rtl839x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 0);
	u32 idx = (0 << 14) | (hash << 2) | pos; // Search SRAM, with hash and at pos in bucket
	int i;

	rtl_table_read(q, idx);
	for (i= 0; i < 3; i++)
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
	int i;

	u32 idx = (0 << 14) | (hash << 2) | pos; // Access SRAM, with hash and at pos in bucket

	rtl839x_fill_l2_row(r, e);

	for (i= 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl839x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 1); // Access L2 Table 1
	int i;

	rtl_table_read(q, idx);
	for (i= 0; i < 3; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl839x_fill_l2_entry(r, e);
	if (!e->valid)
		return 0;

	pr_debug("Found in CAM: R1 %x R2 %x R3 %x\n", r[0], r[1], r[2]);

	// Return MAC with concatenated VID ac concatenated ID
	return rtl839x_l2_hash_seed(ether_addr_to_u64(&e->mac[0]), e->rvid);
}

static void rtl839x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
	u32 r[3];
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 1); // Access L2 Table 1
	int i;

	rtl839x_fill_l2_row(r, e);

	for (i= 0; i < 3; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static u64 rtl839x_read_mcast_pmask(int idx)
{
	u64 portmask;
	// Read MC_PMSK (2) via register RTL8390_TBL_L2
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	portmask <<= 32;
	portmask |= sw_r32(rtl_table_data(q, 1));
	portmask >>= 11;  // LSB is bit 11 in data registers
	rtl_table_release(q);

	return portmask;
}

static void rtl839x_write_mcast_pmask(int idx, u64 portmask)
{
	// Access MC_PMSK (2) via register RTL8380_TBL_L2
	struct table_reg *q = rtl_table_get(RTL8390_TBL_L2, 2);

	portmask <<= 11; // LSB is bit 11 in data registers
	sw_w32((u32)(portmask >> 32), rtl_table_data(q, 0));
	sw_w32((u32)((portmask & 0xfffff800)), rtl_table_data(q, 1));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static void rtl839x_vlan_profile_setup(int profile)
{
	u32 p[2];
	u32 pmask_id = UNKNOWN_MC_PMASK;

	p[0] = pmask_id; // Use portmaks 0xfff for unknown IPv6 MC flooding
	// Enable L2 Learning BIT 0, portmask UNKNOWN_MC_PMASK for IP/L2-MC traffic flooding
	p[1] = 1 | pmask_id << 1 | pmask_id << 13;

	sw_w32(p[0], RTL839X_VLAN_PROFILE(profile));
	sw_w32(p[1], RTL839X_VLAN_PROFILE(profile) + 4);

	rtl839x_write_mcast_pmask(UNKNOWN_MC_PMASK, 0x001fffffffffffff);
}

static inline int rtl839x_vlan_port_egr_filter(int port)
{
	return RTL839X_VLAN_PORT_EGR_FLTR(port);
}

static inline int rtl839x_vlan_port_igr_filter(int port)
{
	return RTL839X_VLAN_PORT_IGR_FLTR(port);
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

irqreturn_t rtl839x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 status = sw_r32(RTL839X_ISR_GLB_SRC);
	u64 ports = rtl839x_get_port_reg_le(RTL839X_ISR_PORT_LINK_STS_CHG);
	u64 link;
	int i;

	/* Clear status */
	rtl839x_set_port_reg_le(ports, RTL839X_ISR_PORT_LINK_STS_CHG);
	pr_debug("RTL8390 Link change: status: %x, ports %llx\n", status, ports);

	for (i = 0; i < RTL839X_CPU_PORT; i++) {
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

// TODO: unused
int rtl8390_sds_power(int mac, int val)
{
	u32 offset = (mac == 48) ? 0x0 : 0x100;
	u32 mode = val ? 0 : 1;

	pr_debug("In %s: mac %d, set %d\n", __func__, mac, val);

	if ((mac != 48) && (mac != 49)) {
		pr_err("%s: not an SFP port: %d\n", __func__, mac);
		return -1;
	}

	// Set bit 1003. 1000 starts at 7c
	sw_w32_mask(BIT(11), mode << 11, RTL839X_SDS12_13_PWR0 + offset);

	return 0;
}

int rtl839x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;

	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	sw_w32_mask(0xffff0000, port << 16, RTL839X_PHYREG_DATA_CTRL);
	v = reg << 5 | page << 10 | ((page == 0x1fff) ? 0x1f : 0) << 23;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	sw_w32(0x1ff, RTL839X_PHYREG_CTRL);

	v |= 1;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	do {
	} while (sw_r32(RTL839X_PHYREG_ACCESS_CTRL) & 0x1);

	*val = sw_r32(RTL839X_PHYREG_DATA_CTRL) & 0xffff;

	mutex_unlock(&smi_lock);
	return 0;
}

int rtl839x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	int err = 0;

	val &= 0xffff;
	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	// Set PHY to access
	rtl839x_set_port_reg_le(BIT_ULL(port), RTL839X_PHYREG_PORT_CTRL);

	sw_w32_mask(0xffff0000, val << 16, RTL839X_PHYREG_DATA_CTRL);

	v = reg << 5 | page << 10 | ((page == 0x1fff) ? 0x1f : 0) << 23;
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	sw_w32(0x1ff, RTL839X_PHYREG_CTRL);

	v |= BIT(3) | 1; /* Write operation and execute */
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	do {
	} while (sw_r32(RTL839X_PHYREG_ACCESS_CTRL) & 0x1);

	if (sw_r32(RTL839X_PHYREG_ACCESS_CTRL) & 0x2)
		err = -EIO;

	mutex_unlock(&smi_lock);
	return err;
}

/*
 * Read an mmd register of the PHY
 */
int rtl839x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val)
{
	int err = 0;
	u32 v;

	mutex_lock(&smi_lock);

	// Set PHY to access
	sw_w32_mask(0xffff << 16, port << 16, RTL839X_PHYREG_DATA_CTRL);

	// Set MMD device number and register to write to
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL839X_PHYREG_MMD_CTRL);

	v = BIT(2) | BIT(0); // MMD-access | EXEC
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	do {
		v = sw_r32(RTL839X_PHYREG_ACCESS_CTRL);
	} while (v & BIT(0));
	// There is no error-checking via BIT 1 of v, as it does not seem to be set correctly
	*val = (sw_r32(RTL839X_PHYREG_DATA_CTRL) & 0xffff);
	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, *val, err);

	mutex_unlock(&smi_lock);

	return err;
}

/*
 * Write to an mmd register of the PHY
 */
int rtl839x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val)
{
	int err = 0;
	u32 v;

	mutex_lock(&smi_lock);

	// Set PHY to access
	rtl839x_set_port_reg_le(BIT_ULL(port), RTL839X_PHYREG_PORT_CTRL);

	// Set data to write
	sw_w32_mask(0xffff << 16, val << 16, RTL839X_PHYREG_DATA_CTRL);

	// Set MMD device number and register to write to
	sw_w32(devnum << 16 | (regnum & 0xffff), RTL839X_PHYREG_MMD_CTRL);

	v = BIT(3) | BIT(2) | BIT(0); // WRITE | MMD-access | EXEC
	sw_w32(v, RTL839X_PHYREG_ACCESS_CTRL);

	do {
		v = sw_r32(RTL839X_PHYREG_ACCESS_CTRL);
	} while (v & BIT(0));

	pr_debug("%s: port %d, regnum: %x, val: %x (err %d)\n", __func__, port, regnum, val, err);
	mutex_unlock(&smi_lock);
	return err;
}

void rtl8390_get_version(struct rtl838x_switch_priv *priv)
{
	u32 info;

	sw_w32_mask(0xf << 28, 0xa << 28, RTL839X_CHIP_INFO);
	info = sw_r32(RTL839X_CHIP_INFO);
	pr_debug("Chip-Info: %x\n", info);
	priv->version = RTL8390_VERSION_A;
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
	int i;
	u32 cmd = 1 << 16 /* Execute cmd */
		| 0 << 15 /* Read */
		| 5 << 12 /* Table type 0b101 */
		| (msti & 0xfff);
	priv->r->exec_tbl0_cmd(cmd);

	for (i = 0; i < 4; i++)
		port_state[i] = sw_r32(priv->r->tbl_access_data_0(i));
}

static void rtl839x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	int i;
	u32 cmd = 1 << 16 /* Execute cmd */
		| 1 << 15 /* Write */
		| 5 << 12 /* Table type 0b101 */
		| (msti & 0xfff);
	for (i = 0; i < 4; i++)
		sw_w32(port_state[i], priv->r->tbl_access_data_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

/*
 * Enables or disables the EEE/EEEP capability of a port
 */
void rtl839x_port_eee_set(struct rtl838x_switch_priv *priv, int port, bool enable)
{
	u32 v;

	// This works only for Ethernet ports, and on the RTL839X, ports above 47 are SFP
	if (port >= 48)
		return;

	enable = true;
	pr_debug("In %s: setting port %d to %d\n", __func__, port, enable);
	v = enable ? 0xf : 0x0;

	// Set EEE for 100, 500, 1000MBit and 10GBit
	sw_w32_mask(0xf << 8, v << 8, rtl839x_mac_force_mode_ctrl(port));

	// Set TX/RX EEE state
	v = enable ? 0x3 : 0x0;
	sw_w32(v, RTL839X_EEE_CTRL(port));

	priv->ports[port].eee_enabled = enable;
}

/*
 * Get EEE own capabilities and negotiation result
 */
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
	int i;

	pr_info("Setting up EEE, state: %d\n", enable);

	// Set wake timer for TX and pause timer both to 0x21
	sw_w32_mask(0xff << 20| 0xff, 0x21 << 20| 0x21, RTL839X_EEE_TX_TIMER_GELITE_CTRL);
	// Set pause wake timer for GIGA-EEE to 0x11
	sw_w32_mask(0xff << 20, 0x11 << 20, RTL839X_EEE_TX_TIMER_GIGA_CTRL);
	// Set pause wake timer for 10GBit ports to 0x11
	sw_w32_mask(0xff << 20, 0x11 << 20, RTL839X_EEE_TX_TIMER_10G_CTRL);

	// Setup EEE on all ports
	for (i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			rtl839x_port_eee_set(priv, i, enable);
	}
	priv->eee_enabled = enable;
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
	.vlan_port_egr_filter = RTL839X_VLAN_PORT_EGR_FLTR(0),
	.vlan_port_igr_filter = RTL839X_VLAN_PORT_IGR_FLTR(0),
	.vlan_port_pb = RTL839X_VLAN_PORT_PB_VLAN,
	.vlan_port_tag_sts_ctrl = RTL839X_VLAN_PORT_TAG_STS_CTRL,
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
};
