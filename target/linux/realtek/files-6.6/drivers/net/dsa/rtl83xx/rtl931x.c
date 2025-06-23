// SPDX-License-Identifier: GPL-2.0-only

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include <linux/etherdevice.h>

#include "rtl83xx.h"

#define RTL931X_VLAN_PORT_TAG_STS_INTERNAL			0x0
#define RTL931X_VLAN_PORT_TAG_STS_UNTAG				0x1
#define RTL931X_VLAN_PORT_TAG_STS_TAGGED			0x2
#define RTL931X_VLAN_PORT_TAG_STS_PRIORITY_TAGGED		0x3

#define RTL931X_VLAN_PORT_TAG_CTRL_BASE				0x4860
/* port 0-56 */
#define RTL931X_VLAN_PORT_TAG_CTRL(port) \
		RTL931X_VLAN_PORT_TAG_CTRL_BASE + (port << 2)
#define RTL931X_VLAN_PORT_TAG_EGR_OTAG_STS_MASK			GENMASK(13,12)
#define RTL931X_VLAN_PORT_TAG_EGR_ITAG_STS_MASK			GENMASK(11,10)
#define RTL931X_VLAN_PORT_TAG_EGR_OTAG_KEEP_MASK		GENMASK(9,9)
#define RTL931X_VLAN_PORT_TAG_EGR_ITAG_KEEP_MASK		GENMASK(8,8)
#define RTL931X_VLAN_PORT_TAG_IGR_OTAG_KEEP_MASK		GENMASK(7,7)
#define RTL931X_VLAN_PORT_TAG_IGR_ITAG_KEEP_MASK		GENMASK(6,6)
#define RTL931X_VLAN_PORT_TAG_OTPID_IDX_MASK			GENMASK(5,4)
#define RTL931X_VLAN_PORT_TAG_OTPID_KEEP_MASK			GENMASK(3,3)
#define RTL931X_VLAN_PORT_TAG_ITPID_IDX_MASK			GENMASK(2,1)
#define RTL931X_VLAN_PORT_TAG_ITPID_KEEP_MASK			GENMASK(0,0)

extern struct mutex smi_lock;
extern struct rtl83xx_soc_info soc_info;

/* Definition of the RTL931X-specific template field IDs as used in the PIE */
enum template_field_id {
	TEMPLATE_FIELD_SPM0 = 1,
	TEMPLATE_FIELD_SPM1 = 2,
	TEMPLATE_FIELD_SPM2 = 3,
	TEMPLATE_FIELD_SPM3 = 4,
	TEMPLATE_FIELD_DMAC0 = 9,
	TEMPLATE_FIELD_DMAC1 = 10,
	TEMPLATE_FIELD_DMAC2 = 11,
	TEMPLATE_FIELD_SMAC0 = 12,
	TEMPLATE_FIELD_SMAC1 = 13,
	TEMPLATE_FIELD_SMAC2 = 14,
	TEMPLATE_FIELD_ETHERTYPE = 15,
	TEMPLATE_FIELD_OTAG = 16,
	TEMPLATE_FIELD_ITAG = 17,
	TEMPLATE_FIELD_SIP0 = 18,
	TEMPLATE_FIELD_SIP1 = 19,
	TEMPLATE_FIELD_DIP0 = 20,
	TEMPLATE_FIELD_DIP1 = 21,
	TEMPLATE_FIELD_IP_TOS_PROTO = 22,
	TEMPLATE_FIELD_L4_SPORT = 23,
	TEMPLATE_FIELD_L4_DPORT = 24,
	TEMPLATE_FIELD_L34_HEADER = 25,
	TEMPLATE_FIELD_TCP_INFO = 26,
	TEMPLATE_FIELD_SIP2 = 34,
	TEMPLATE_FIELD_SIP3 = 35,
	TEMPLATE_FIELD_SIP4 = 36,
	TEMPLATE_FIELD_SIP5 = 37,
	TEMPLATE_FIELD_SIP6 = 38,
	TEMPLATE_FIELD_SIP7 = 39,
	TEMPLATE_FIELD_DIP2 = 42,
	TEMPLATE_FIELD_DIP3 = 43,
	TEMPLATE_FIELD_DIP4 = 44,
	TEMPLATE_FIELD_DIP5 = 45,
	TEMPLATE_FIELD_DIP6 = 46,
	TEMPLATE_FIELD_DIP7 = 47,
	TEMPLATE_FIELD_FLOW_LABEL = 49,
	TEMPLATE_FIELD_DSAP_SSAP = 50,
	TEMPLATE_FIELD_FWD_VID = 52,
	TEMPLATE_FIELD_RANGE_CHK = 53,
	TEMPLATE_FIELD_SLP = 55,
	TEMPLATE_FIELD_DLP = 56,
	TEMPLATE_FIELD_META_DATA = 57,
	TEMPLATE_FIELD_FIRST_MPLS1 = 60,
	TEMPLATE_FIELD_FIRST_MPLS2 = 61,
	TEMPLATE_FIELD_DPM3 = 8,
};

/* The meaning of TEMPLATE_FIELD_VLAN depends on phase and the configuration in
 * RTL931X_PIE_CTRL. We use always the same definition and map to the inner VLAN tag:
 */
#define TEMPLATE_FIELD_VLAN TEMPLATE_FIELD_ITAG

/* Number of fixed templates predefined in the RTL9300 SoC */
#define N_FIXED_TEMPLATES 5
/* RTL931x specific predefined templates */
static enum template_field_id fixed_templates[N_FIXED_TEMPLATES][N_FIXED_FIELDS_RTL931X] =
{
	{
		TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
		TEMPLATE_FIELD_SMAC0, TEMPLATE_FIELD_SMAC1, TEMPLATE_FIELD_SMAC2,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_DSAP_SSAP,
		TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	}, {
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
		TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_IP_TOS_PROTO, TEMPLATE_FIELD_TCP_INFO,
		TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT, TEMPLATE_FIELD_VLAN,
		TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	}, {
		TEMPLATE_FIELD_DMAC0, TEMPLATE_FIELD_DMAC1, TEMPLATE_FIELD_DMAC2,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_ETHERTYPE, TEMPLATE_FIELD_IP_TOS_PROTO,
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_DIP0,
		TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT,
		TEMPLATE_FIELD_META_DATA, TEMPLATE_FIELD_SLP
	}, {
		TEMPLATE_FIELD_DIP0, TEMPLATE_FIELD_DIP1, TEMPLATE_FIELD_DIP2,
		TEMPLATE_FIELD_DIP3, TEMPLATE_FIELD_DIP4, TEMPLATE_FIELD_DIP5,
		TEMPLATE_FIELD_DIP6, TEMPLATE_FIELD_DIP7, TEMPLATE_FIELD_IP_TOS_PROTO,
		TEMPLATE_FIELD_TCP_INFO, TEMPLATE_FIELD_L4_SPORT, TEMPLATE_FIELD_L4_DPORT,
		TEMPLATE_FIELD_RANGE_CHK, TEMPLATE_FIELD_SLP
	}, {
		TEMPLATE_FIELD_SIP0, TEMPLATE_FIELD_SIP1, TEMPLATE_FIELD_SIP2,
		TEMPLATE_FIELD_SIP3, TEMPLATE_FIELD_SIP4, TEMPLATE_FIELD_SIP5,
		TEMPLATE_FIELD_SIP6, TEMPLATE_FIELD_SIP7, TEMPLATE_FIELD_META_DATA,
		TEMPLATE_FIELD_VLAN, TEMPLATE_FIELD_SPM0, TEMPLATE_FIELD_SPM1,
		TEMPLATE_FIELD_SPM2, TEMPLATE_FIELD_SPM3
	},
};

inline void rtl931x_exec_tbl0_cmd(u32 cmd)
{
	sw_w32(cmd, RTL931X_TBL_ACCESS_CTRL_0);
	do { } while (sw_r32(RTL931X_TBL_ACCESS_CTRL_0) & (1 << 20));
}

inline void rtl931x_exec_tbl1_cmd(u32 cmd)
{
	sw_w32(cmd, RTL931X_TBL_ACCESS_CTRL_1);
	do { } while (sw_r32(RTL931X_TBL_ACCESS_CTRL_1) & (1 << 17));
}

inline int rtl931x_tbl_access_data_0(int i)
{
	return RTL931X_TBL_ACCESS_DATA_0(i);
}

static void rtl931x_vlan_profile_dump(int index)
{
	u64 profile[4];

	if (index < 0 || index > 15)
		return;

	profile[0] = sw_r32(RTL931X_VLAN_PROFILE_SET(index));
	profile[1] = (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 4) & 0x1FFFFFFFULL) << 32 |
	             (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 8) & 0xFFFFFFFF);
	profile[2] = (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 16) & 0x1FFFFFFFULL) << 32 |
	             (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 12) & 0xFFFFFFFF);
	profile[3] = (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 20) & 0x1FFFFFFFULL) << 32 |
	             (sw_r32(RTL931X_VLAN_PROFILE_SET(index) + 24) & 0xFFFFFFFF);

	pr_debug("VLAN %d: L2 learning: %d, L2 Unknown MultiCast Field %llx, \
		IPv4 Unknown MultiCast Field %llx, IPv6 Unknown MultiCast Field: %llx",
		index, (u32) (profile[0] & (3 << 14)), profile[1], profile[2], profile[3]);
}

static void rtl931x_stp_get(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 20 | /* Execute cmd */
	          0 << 19 | /* Read */
	          5 << 15 | /* Table type 0b101 */
	          (msti & 0x3fff);
	priv->r->exec_tbl0_cmd(cmd);

	for (int i = 0; i < 4; i++)
		port_state[i] = sw_r32(priv->r->tbl_access_data_0(i));
}

static void rtl931x_stp_set(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[])
{
	u32 cmd = 1 << 20 | /* Execute cmd */
	          1 << 19 | /* Write */
	          5 << 15 | /* Table type 0b101 */
	          (msti & 0x3fff);
	for (int i = 0; i < 4; i++)
		sw_w32(port_state[i], priv->r->tbl_access_data_0(i));
	priv->r->exec_tbl0_cmd(cmd);
}

inline static int rtl931x_trk_mbr_ctr(int group)
{
	return RTL931X_TRK_MBR_CTRL + (group << 2);
}

static void rtl931x_vlan_tables_read(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v, w, x, y;
	/* Read VLAN table (3) via register 0 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 3);

	rtl_table_read(r, vlan);
	v = sw_r32(rtl_table_data(r, 0));
	w = sw_r32(rtl_table_data(r, 1));
	x = sw_r32(rtl_table_data(r, 2));
	y = sw_r32(rtl_table_data(r, 3));
	rtl_table_release(r);

	pr_debug("VLAN_READ %d: %08x %08x %08x %08x\n", vlan, v, w, x, y);
	info->tagged_ports = ((u64) v) << 25 | (w >> 7);
	info->profile_id = (x >> 16) & 0xf;
	info->fid = w & 0x7f;				/* AKA MSTI depending on context */
	info->hash_uc_fid = !!(x & BIT(31));
	info->hash_mc_fid = !!(x & BIT(30));
	info->if_id = (x >> 20) & 0x3ff;
	info->profile_id = (x >> 16) & 0xf;
	info->multicast_grp_mask = x & 0xffff;
	if (x & BIT(31))
		info->l2_tunnel_list_id = y >> 18;
	else
		info->l2_tunnel_list_id = -1;
	pr_debug("%s read tagged %016llx, profile-id %d, uc %d, mc %d, intf-id %d\n", __func__,
		info->tagged_ports, info->profile_id, info->hash_uc_fid, info->hash_mc_fid,
		info->if_id);

	/* Read UNTAG table via table register 3 */
	r = rtl_table_get(RTL9310_TBL_3, 0);
	rtl_table_read(r, vlan);
	v = ((u64)sw_r32(rtl_table_data(r, 0))) << 25;
	v |= sw_r32(rtl_table_data(r, 1)) >> 7;
	rtl_table_release(r);

	info->untagged_ports = v;
}

static void rtl931x_vlan_set_tagged(u32 vlan, struct rtl838x_vlan_info *info)
{
	u32 v, w, x, y;
	/* Access VLAN table (1) via register 0 */
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 3);

	v = info->tagged_ports >> 25;
	w = (info->tagged_ports & 0x1fffff) << 7;
	w |= info->fid & 0x7f;
	x = info->hash_uc_fid ? BIT(31) : 0;
	x |= info->hash_mc_fid ? BIT(30) : 0;
	x |= info->if_id & 0x3ff << 20;
	x |= (info->profile_id & 0xf) << 16;
	x |= info->multicast_grp_mask & 0xffff;
	if (info->l2_tunnel_list_id >= 0) {
		y = info->l2_tunnel_list_id << 18;
		y |= BIT(31);
	} else {
		y = 0;
	}

	sw_w32(v, rtl_table_data(r, 0));
	sw_w32(w, rtl_table_data(r, 1));
	sw_w32(x, rtl_table_data(r, 2));
	sw_w32(y, rtl_table_data(r, 3));

	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static void rtl931x_vlan_set_untagged(u32 vlan, u64 portmask)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_3, 0);

	rtl839x_set_port_reg_be(portmask << 7, rtl_table_data(r, 0));
	rtl_table_write(r, vlan);
	rtl_table_release(r);
}

static inline int rtl931x_mac_force_mode_ctrl(int p)
{
	return RTL931X_MAC_FORCE_MODE_CTRL + (p << 2);
}

static inline int rtl931x_mac_link_spd_sts(int p)
{
	return RTL931X_MAC_LINK_SPD_STS + (((p >> 3) << 2));
}

static inline int rtl931x_mac_port_ctrl(int p)
{
	return RTL931X_MAC_L2_PORT_CTRL + (p << 7);
}

static inline int rtl931x_l2_port_new_salrn(int p)
{
	return RTL931X_L2_PORT_NEW_SALRN(p);
}

static inline int rtl931x_l2_port_new_sa_fwd(int p)
{
	return RTL931X_L2_PORT_NEW_SA_FWD(p);
}

irqreturn_t rtl931x_switch_irq(int irq, void *dev_id)
{
	struct dsa_switch *ds = dev_id;
	u32 status = sw_r32(RTL931X_ISR_GLB_SRC);
	u64 ports = rtl839x_get_port_reg_le(RTL931X_ISR_PORT_LINK_STS_CHG);
	u64 link;

	/* Clear status */
	rtl839x_set_port_reg_le(ports, RTL931X_ISR_PORT_LINK_STS_CHG);
	pr_debug("RTL931X Link change: status: %x, ports %016llx\n", status, ports);

	link = rtl839x_get_port_reg_le(RTL931X_MAC_LINK_STS);
	/* Must re-read this to get correct status */
	link = rtl839x_get_port_reg_le(RTL931X_MAC_LINK_STS);
	pr_debug("RTL931X Link change: status: %x, link status %016llx\n", status, link);

	for (int i = 0; i < 56; i++) {
		if (ports & BIT_ULL(i)) {
			if (link & BIT_ULL(i)) {
				pr_debug("%s port %d up\n", __func__, i);
				dsa_port_phylink_mac_change(ds, i, true);
			} else {
				pr_debug("%s port %d down\n", __func__, i);
				dsa_port_phylink_mac_change(ds, i, false);
			}
		}
	}

	return IRQ_HANDLED;
}

int rtl931x_write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	u32 v;
	int err = 0;

	val &= 0xffff;
	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);
	pr_debug("%s: writing to phy %d %d %d %d\n", __func__, port, page, reg, val);
	/* Clear both port registers */
	sw_w32(0, RTL931X_SMI_INDRT_ACCESS_CTRL_2);
	sw_w32(0, RTL931X_SMI_INDRT_ACCESS_CTRL_2 + 4);
	sw_w32_mask(0, BIT(port % 32), RTL931X_SMI_INDRT_ACCESS_CTRL_2 + (port / 32) * 4);

	sw_w32_mask(0xffff, val, RTL931X_SMI_INDRT_ACCESS_CTRL_3);

	v = reg << 6 | page << 11 ;
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	sw_w32(0x1ff, RTL931X_SMI_INDRT_ACCESS_CTRL_1);

	v |= BIT(4) | 1; /* Write operation and execute */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
	} while (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x1);

	if (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x2)
		err = -EIO;

	mutex_unlock(&smi_lock);

	return err;
}

int rtl931x_read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	u32 v;

	if (port > 63 || page > 4095 || reg > 31)
		return -ENOTSUPP;

	mutex_lock(&smi_lock);

	sw_w32(port << 5, RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL);

	v = reg << 6 | page << 11 | 1;
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
	} while (sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0) & 0x1);

	v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	*val = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_3);
	*val = (*val & 0xffff0000) >> 16;

	pr_debug("%s: port %d, page: %d, reg: %x, val: %x, v: %08x\n",
		__func__, port, page, reg, *val, v);

	mutex_unlock(&smi_lock);

	return 0;
}

/* Read an mmd register of the PHY */
int rtl931x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val)
{
	int err = 0;
	u32 v;
	/* Select PHY register type
	 * If select 1G/10G MMD register type, registers EXT_PAGE, MAIN_PAGE and REG settings are don’t care.
	 * 0x0  Normal register (Clause 22)
	 * 0x1: 1G MMD register (MMD via Clause 22 registers 13 and 14)
	 * 0x2: 10G MMD register (MMD via Clause 45)
	 */
	int type = 2;

	mutex_lock(&smi_lock);

	/* Set PHY to access via port-number */
	sw_w32(port << 5, RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | regnum, RTL931X_SMI_INDRT_ACCESS_MMD_CTRL);

	v = type << 2 | BIT(0); /* MMD-access-type | EXEC */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
		v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	} while (v & BIT(0));

	/* Check for error condition */
	if (v & BIT(1))
		err = -EIO;

	*val = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_3) >> 16;

	pr_debug("%s: port %d, dev: %x, regnum: %x, val: %x (err %d)\n", __func__,
		 port, devnum, regnum, *val, err);

	mutex_unlock(&smi_lock);

	return err;
}

/* Write to an mmd register of the PHY */
int rtl931x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val)
{
	int err = 0;
	u32 v;
	int type = 2;
	u64 pm;

	mutex_lock(&smi_lock);

	/* Set PHY to access via port-mask */
	pm = (u64)1 << port;
	sw_w32((u32)pm, RTL931X_SMI_INDRT_ACCESS_CTRL_2);
	sw_w32((u32)(pm >> 32), RTL931X_SMI_INDRT_ACCESS_CTRL_2 + 4);

	/* Set data to write */
	sw_w32_mask(0xffff, val, RTL931X_SMI_INDRT_ACCESS_CTRL_3);

	/* Set MMD device number and register to write to */
	sw_w32(devnum << 16 | regnum, RTL931X_SMI_INDRT_ACCESS_MMD_CTRL);

	v = BIT(4) | type << 2 | BIT(0); /* WRITE | MMD-access-type | EXEC */
	sw_w32(v, RTL931X_SMI_INDRT_ACCESS_CTRL_0);

	do {
		v = sw_r32(RTL931X_SMI_INDRT_ACCESS_CTRL_0);
	} while (v & BIT(0));

	pr_debug("%s: port %d, dev: %x, regnum: %x, val: %x (err %d)\n", __func__,
		 port, devnum, regnum, val, err);
	mutex_unlock(&smi_lock);

	return err;
}

void rtl931x_print_matrix(void)
{
	volatile u64 *ptr = RTL838X_SW_BASE + RTL839X_PORT_ISO_CTRL(0);

	for (int i = 0; i < 52; i += 4)
		pr_debug("> %16llx %16llx %16llx %16llx\n",
			ptr[i + 0], ptr[i + 1], ptr[i + 2], ptr[i + 3]);
	pr_debug("CPU_PORT> %16llx\n", ptr[52]);
}

void rtl931x_set_receive_management_action(int port, rma_ctrl_t type, action_type_t action)
{
	u32 value = 0;

	/* hack for value mapping */
	if (type == GRATARP && action == COPY2CPU)
		action = TRAP2MASTERCPU;

	switch(action) {
	case FORWARD:
		value = 0;
		break;
	case DROP:
		value = 1;
		break;
	case TRAP2CPU:
		value = 2;
		break;
	case TRAP2MASTERCPU:
		value = 3;
		break;
	case FLOODALL:
		value = 4;
		break;
	default:
		break;
	}

	switch(type) {
	case BPDU:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_BPDU_CTRL + ((port / 10) << 2));
	break;
	case PTP:
		/* udp */
		sw_w32_mask(3 << 2, value << 2, RTL931X_RMA_PTP_CTRL + (port << 2));
		/* eth2 */
		sw_w32_mask(3, value, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case PTP_UDP:
		sw_w32_mask(3 << 2, value << 2, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case PTP_ETH2:
		sw_w32_mask(3, value, RTL931X_RMA_PTP_CTRL + (port << 2));
	break;
	case LLDP:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_LLDP_CTRL + ((port / 10) << 2));
	break;
	case EAPOL:
		sw_w32_mask(7 << ((port % 10) * 3), value << ((port % 10) * 3), RTL931X_RMA_EAPOL_CTRL + ((port / 10) << 2));
	break;
	case GRATARP:
		sw_w32_mask(3 << ((port & 0xf) << 1), value << ((port & 0xf) << 1), RTL931X_TRAP_ARP_GRAT_PORT_ACT + ((port >> 4) << 2));
	break;
	}
}

static u64 rtl931x_traffic_get(int source)
{
	u32 v;
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 6);

	rtl_table_read(r, source);
	v = sw_r32(rtl_table_data(r, 0));
	rtl_table_release(r);
	v = v >> 3;

	return v;
}

/* Enable traffic between a source port and a destination port matrix */
static void rtl931x_traffic_set(int source, u64 dest_matrix)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 6);

	sw_w32((dest_matrix << 3), rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl931x_traffic_enable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 6);
	rtl_table_read(r, source);
	sw_w32_mask(0, BIT(dest + 3), rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static void rtl931x_traffic_disable(int source, int dest)
{
	struct table_reg *r = rtl_table_get(RTL9310_TBL_0, 6);
	rtl_table_read(r, source);
	sw_w32_mask(BIT(dest + 3), 0, rtl_table_data(r, 0));
	rtl_table_write(r, source);
	rtl_table_release(r);
}

static u64 rtl931x_l2_hash_seed(u64 mac, u32 vid)
{
	u64 v = vid;

	v <<= 48;
	v |= mac;

	return v;
}

/* Calculate both the block 0 and the block 1 hash by applyingthe same hash
 * algorithm as the one used currently by the ASIC to the seed, and return
 * both hashes in the lower and higher word of the return value since only 12 bit of
 * the hash are significant.
 */
static u32 rtl931x_l2_hash_key(struct rtl838x_switch_priv *priv, u64 seed)
{
	u32 h, h0, h1, h2, h3, h4, k0, k1;

	h0 = seed & 0xfff;
	h1 = (seed >> 12) & 0xfff;
	h2 = (seed >> 24) & 0xfff;
	h3 = (seed >> 36) & 0xfff;
	h4 = (seed >> 48) & 0xfff;
	h4 = ((h4 & 0x7) << 9) | ((h4 >> 3) & 0x1ff);
	k0 = h0 ^ h1 ^ h2 ^ h3 ^ h4;

	h0 = seed & 0xfff;
	h0 = ((h0 & 0x1ff) << 3) | ((h0 >> 9) & 0x7);
	h1 = (seed >> 12) & 0xfff;
	h1 = ((h1 & 0x3f) << 6) | ((h1 >> 6) & 0x3f);
	h2 = (seed >> 24) & 0xfff;
	h3 = (seed >> 36) & 0xfff;
	h3 = ((h3 & 0x3f) << 6) | ((h3 >> 6) & 0x3f);
	h4 = (seed >> 48) & 0xfff;
	k1 = h0 ^ h1 ^ h2 ^ h3 ^ h4;

	/* Algorithm choice for block 0 */
	if (sw_r32(RTL931X_L2_CTRL) & BIT(0))
		h = k1;
	else
		h = k0;

	/* Algorithm choice for block 1
	 * Since k0 and k1 are < 4096, adding 4096 will offset the hash into the second
	 * half of hash-space
	 * 4096 is in fact the hash-table size 32768 divided by 4 hashes per bucket
	 * divided by 2 to divide the hash space in 2
	 */
	if (sw_r32(RTL931X_L2_CTRL) & BIT(1))
		h |= (k1 + 4096) << 16;
	else
		h |= (k0 + 4096) << 16;

	return h;
}

/* Fills an L2 entry structure from the SoC registers */
static void rtl931x_fill_l2_entry(u32 r[], struct rtl838x_l2_entry *e)
{
	pr_debug("In %s valid?\n", __func__);
	e->valid = !!(r[0] & BIT(31));
	if (!e->valid)
		return;

	pr_debug("%s: entry valid, raw: %08x %08x %08x %08x\n", __func__, r[0], r[1], r[2], r[3]);
	e->is_ip_mc = false;
	e->is_ipv6_mc = false;

	e->mac[0] = r[0] >> 8;
	e->mac[1] = r[0];
	e->mac[2] = r[1] >> 24;
	e->mac[3] = r[1] >> 16;
	e->mac[4] = r[1] >> 8;
	e->mac[5] = r[1];

	e->is_open_flow = !!(r[0] & BIT(30));
	e->is_pe_forward = !!(r[0] & BIT(29));
	e->next_hop = !!(r[2] & BIT(30));
	e->rvid = (r[0] >> 16) & 0xfff;

	/* Is it a unicast entry? check multicast bit */
	if (!(e->mac[0] & 1)) {
		e->type = L2_UNICAST;
		e->is_l2_tunnel = !!(r[2] & BIT(31));
		e->is_static = !!(r[2] & BIT(13));
		e->port = (r[2] >> 19) & 0x3ff;
		/* Check for trunk port */
		if (r[2] & BIT(29)) {
			e->is_trunk = true;
			e->stack_dev = (e->port >> 9) & 1;
			e->trunk = e->port & 0x3f;
		} else {
			e->is_trunk = false;
			e->stack_dev = (e->port >> 6) & 0xf;
			e->port = e->port & 0x3f;
		}

		e->block_da = !!(r[2] & BIT(14));
		e->block_sa = !!(r[2] & BIT(15));
		e->suspended = !!(r[2] & BIT(12));
		e->age = (r[2] >> 16) & 3;

		/* the UC_VID field in hardware is used for the VID or for the route id */
		if (e->next_hop) {
			e->nh_route_id = r[2] & 0x7ff;
			e->vid = 0;
		} else {
			e->vid = r[2] & 0xfff;
			e->nh_route_id = 0;
		}
		if (e->is_l2_tunnel)
			e->l2_tunnel_id = ((r[2] & 0xff) << 4) | (r[3] >> 28);
		/* TODO: Implement VLAN conversion */
	} else {
		e->type = L2_MULTICAST;
		e->is_local_forward = !!(r[2] & BIT(31));
		e->is_remote_forward = !!(r[2] & BIT(17));
		e->mc_portmask_index = (r[2] >> 18) & 0xfff;
		e->l2_tunnel_list_id = (r[2] >> 4) & 0x1fff;
	}
}

/* Fills the 3 SoC table registers r[] with the information of in the rtl838x_l2_entry */
static void rtl931x_fill_l2_row(u32 r[], struct rtl838x_l2_entry *e)
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
static u64 rtl931x_read_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[4];
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 0);
	u32 idx;
	u64 mac;
	u64 seed;

	pr_debug("%s: hash %08x, pos: %d\n", __func__, hash, pos);

	/* On the RTL93xx, 2 different hash algorithms are used making it a total of
	 * 8 buckets that need to be searched, 4 for each hash-half
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
	for (int i = 0; i < 4; i++)
		r[i] = sw_r32(rtl_table_data(q, i));

	rtl_table_release(q);

	rtl931x_fill_l2_entry(r, e);

	pr_debug("%s: valid: %d, nh: %d\n", __func__, e->valid, e->next_hop);
	if (!e->valid)
		return 0;

	mac = ((u64)e->mac[0]) << 40 |
	      ((u64)e->mac[1]) << 32 |
	      ((u64)e->mac[2]) << 24 |
	      ((u64)e->mac[3]) << 16 |
	      ((u64)e->mac[4]) << 8 |
	      ((u64)e->mac[5]);

	seed = rtl931x_l2_hash_seed(mac, e->rvid);
	pr_debug("%s: mac %016llx, seed %016llx\n", __func__, mac, seed);

	/* return vid with concatenated mac as unique id */
	return seed;
}

static u64 rtl931x_read_cam(int idx, struct rtl838x_l2_entry *e)
{
	return 0;
}

static void rtl931x_write_cam(int idx, struct rtl838x_l2_entry *e)
{
}

static void rtl931x_write_l2_entry_using_hash(u32 hash, u32 pos, struct rtl838x_l2_entry *e)
{
	u32 r[4];
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 0);
	u32 idx = (0 << 14) | (hash << 2) | pos; /* Access SRAM, with hash and at pos in bucket */

	pr_debug("%s: hash %d, pos %d\n", __func__, hash, pos);
	pr_debug("%s: index %d -> mac %02x:%02x:%02x:%02x:%02x:%02x\n", __func__, idx,
		e->mac[0], e->mac[1], e->mac[2], e->mac[3],e->mac[4],e->mac[5]);

	rtl931x_fill_l2_row(r, e);
	pr_debug("%s: %d: %08x %08x %08x\n", __func__, idx, r[0], r[1], r[2]);

	for (int i = 0; i < 4; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);
}

static void rtl931x_vlan_fwd_on_inner(int port, bool is_set)
{
	/* Always set all tag modes to fwd based on either inner or outer tag */
	if (is_set)
		sw_w32_mask(0xf, 0, RTL931X_VLAN_PORT_FWD + (port << 2));
	else
		sw_w32_mask(0, 0xf, RTL931X_VLAN_PORT_FWD + (port << 2));
}

static void rtl931x_vlan_profile_setup(int profile)
{
	u32 p[7];

	pr_debug("In %s\n", __func__);

	if (profile > 15)
		return;

	p[0] = sw_r32(RTL931X_VLAN_PROFILE_SET(profile));

	/* Enable routing of Ipv4/6 Unicast and IPv4/6 Multicast traffic */
	/* p[0] |= BIT(17) | BIT(16) | BIT(13) | BIT(12); */
	p[0] |= 0x3 << 11; /* COPY2CPU */

	p[1] = 0x1FFFFFF; /* L2 unknwon MC flooding portmask all ports, including the CPU-port */
	p[2] = 0xFFFFFFFF;
	p[3] = 0x1FFFFFF; /* IPv4 unknwon MC flooding portmask */
	p[4] = 0xFFFFFFFF;
	p[5] = 0x1FFFFFF; /* IPv6 unknwon MC flooding portmask */
	p[6] = 0xFFFFFFFF;

	for (int i = 0; i < 7; i++)
		sw_w32(p[i], RTL931X_VLAN_PROFILE_SET(profile) + i * 4);
	pr_debug("Leaving %s\n", __func__);
}

static void rtl931x_l2_learning_setup(void)
{
	/* Portmask for flooding broadcast traffic */
	rtl839x_set_port_reg_be(0x1FFFFFFFFFFFFFF, RTL931X_L2_BC_FLD_PMSK);

	/* Portmask for flooding unicast traffic with unknown destination */
	rtl839x_set_port_reg_be(0x1FFFFFFFFFFFFFF, RTL931X_L2_UNKN_UC_FLD_PMSK);

	/* Limit learning to maximum: 64k entries, after that just flood (bits 0-2) */
	sw_w32((0xffff << 3) | FORWARD, RTL931X_L2_LRN_CONSTRT_CTRL);
}

static u64 rtl931x_read_mcast_pmask(int idx)
{
	u64 portmask;
	/* Read MC_PMSK (2) via register RTL9310_TBL_0 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 2);

	rtl_table_read(q, idx);
	portmask = sw_r32(rtl_table_data(q, 0));
	portmask <<= 32;
	portmask |= sw_r32(rtl_table_data(q, 1));
	portmask >>= 7;
	rtl_table_release(q);

	pr_debug("%s: Index idx %d has portmask %016llx\n", __func__, idx, portmask);

	return portmask;
}

static void rtl931x_write_mcast_pmask(int idx, u64 portmask)
{
	u64 pm = portmask;

	/* Access MC_PMSK (2) via register RTL9310_TBL_0 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_0, 2);

	pr_debug("%s: Index idx %d has portmask %016llx\n", __func__, idx, pm);
	pm <<= 7;
	sw_w32((u32)(pm >> 32), rtl_table_data(q, 0));
	sw_w32((u32)pm, rtl_table_data(q, 1));
	rtl_table_write(q, idx);
	rtl_table_release(q);
}


static int rtl931x_set_ageing_time(unsigned long msec)
{
	int t = sw_r32(RTL931X_L2_AGE_CTRL);

	t &= 0x1FFFFF;
	t = (t * 8) / 10;
	pr_debug("L2 AGING time: %d sec\n", t);

	t = (msec / 100 + 7) / 8;
	t = t > 0x1FFFFF ? 0x1FFFFF : t;
	sw_w32_mask(0x1FFFFF, t, RTL931X_L2_AGE_CTRL);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(RTL931X_L2_PORT_AGE_CTRL));

	return 0;
}
void rtl931x_sw_init(struct rtl838x_switch_priv *priv)
{
/*	rtl931x_sds_init(priv); */
}

static void rtl931x_pie_lookup_enable(struct rtl838x_switch_priv *priv, int index)
{
	int block = index / PIE_BLOCK_SIZE;

	sw_w32_mask(0, BIT(block), RTL931X_PIE_BLK_LOOKUP_CTRL);
}

/* Fills the data in the intermediate representation in the pie_rule structure
 * into a data field for a given template field field_type
 * TODO: This function looks very similar to the function of the rtl9300, but
 * since it uses the physical template_field_id, which are different for each
 * SoC and there are other field types, it is actually not. If we would also use
 * an intermediate representation for a field type, we would could have one
 * pie_data_fill function for all SoCs, provided we have also for each SoC a
 * function to map between physical and intermediate field type
 */
static int rtl931x_pie_data_fill(enum template_field_id field_type, struct pie_rule *pr, u16 *data, u16 *data_m)
{
	*data = *data_m = 0;

	switch (field_type) {
	case TEMPLATE_FIELD_SPM0:
		*data = pr->spm;
		*data_m = pr->spm_m;
		break;
	case TEMPLATE_FIELD_SPM1:
		*data = pr->spm >> 16;
		*data_m = pr->spm_m >> 16;
		break;
	case TEMPLATE_FIELD_OTAG:
		*data = pr->otag;
		*data_m = pr->otag_m;
		break;
	case TEMPLATE_FIELD_SMAC0:
		*data = pr->smac[4];
		*data = (*data << 8) | pr->smac[5];
		*data_m = pr->smac_m[4];
		*data_m = (*data_m << 8) | pr->smac_m[5];
		break;
	case TEMPLATE_FIELD_SMAC1:
		*data = pr->smac[2];
		*data = (*data << 8) | pr->smac[3];
		*data_m = pr->smac_m[2];
		*data_m = (*data_m << 8) | pr->smac_m[3];
		break;
	case TEMPLATE_FIELD_SMAC2:
		*data = pr->smac[0];
		*data = (*data << 8) | pr->smac[1];
		*data_m = pr->smac_m[0];
		*data_m = (*data_m << 8) | pr->smac_m[1];
		break;
	case TEMPLATE_FIELD_DMAC0:
		*data = pr->dmac[4];
		*data = (*data << 8) | pr->dmac[5];
		*data_m = pr->dmac_m[4];
		*data_m = (*data_m << 8) | pr->dmac_m[5];
		break;
	case TEMPLATE_FIELD_DMAC1:
		*data = pr->dmac[2];
		*data = (*data << 8) | pr->dmac[3];
		*data_m = pr->dmac_m[2];
		*data_m = (*data_m << 8) | pr->dmac_m[3];
		break;
	case TEMPLATE_FIELD_DMAC2:
		*data = pr->dmac[0];
		*data = (*data << 8) | pr->dmac[1];
		*data_m = pr->dmac_m[0];
		*data_m = (*data_m << 8) | pr->dmac_m[1];
		break;
	case TEMPLATE_FIELD_ETHERTYPE:
		*data = pr->ethertype;
		*data_m = pr->ethertype_m;
		break;
	case TEMPLATE_FIELD_ITAG:
		*data = pr->itag;
		*data_m = pr->itag_m;
		break;
	case TEMPLATE_FIELD_SIP0:
		if (pr->is_ipv6) {
			*data = pr->sip6.s6_addr16[7];
			*data_m = pr->sip6_m.s6_addr16[7];
		} else {
			*data = pr->sip;
			*data_m = pr->sip_m;
		}
		break;
	case TEMPLATE_FIELD_SIP1:
		if (pr->is_ipv6) {
			*data = pr->sip6.s6_addr16[6];
			*data_m = pr->sip6_m.s6_addr16[6];
		} else {
			*data = pr->sip >> 16;
			*data_m = pr->sip_m >> 16;
		}
		break;
	case TEMPLATE_FIELD_SIP2:
	case TEMPLATE_FIELD_SIP3:
	case TEMPLATE_FIELD_SIP4:
	case TEMPLATE_FIELD_SIP5:
	case TEMPLATE_FIELD_SIP6:
	case TEMPLATE_FIELD_SIP7:
		*data = pr->sip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
		*data_m = pr->sip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_SIP2)];
		break;
	case TEMPLATE_FIELD_DIP0:
		if (pr->is_ipv6) {
			*data = pr->dip6.s6_addr16[7];
			*data_m = pr->dip6_m.s6_addr16[7];
		} else {
			*data = pr->dip;
			*data_m = pr->dip_m;
		}
		break;
		case TEMPLATE_FIELD_DIP1:
		if (pr->is_ipv6) {
			*data = pr->dip6.s6_addr16[6];
			*data_m = pr->dip6_m.s6_addr16[6];
		} else {
			*data = pr->dip >> 16;
			*data_m = pr->dip_m >> 16;
		}
		break;
	case TEMPLATE_FIELD_DIP2:
	case TEMPLATE_FIELD_DIP3:
	case TEMPLATE_FIELD_DIP4:
	case TEMPLATE_FIELD_DIP5:
	case TEMPLATE_FIELD_DIP6:
	case TEMPLATE_FIELD_DIP7:
		*data = pr->dip6.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
		*data_m = pr->dip6_m.s6_addr16[5 - (field_type - TEMPLATE_FIELD_DIP2)];
		break;
	case TEMPLATE_FIELD_IP_TOS_PROTO:
		*data = pr->tos_proto;
		*data_m = pr->tos_proto_m;
		break;
	case TEMPLATE_FIELD_L4_SPORT:
		*data = pr->sport;
		*data_m = pr->sport_m;
		break;
	case TEMPLATE_FIELD_L4_DPORT:
		*data = pr->dport;
		*data_m = pr->dport_m;
		break;
	case TEMPLATE_FIELD_DSAP_SSAP:
		*data = pr->dsap_ssap;
		*data_m = pr->dsap_ssap_m;
		break;
	case TEMPLATE_FIELD_TCP_INFO:
		*data = pr->tcp_info;
		*data_m = pr->tcp_info_m;
		break;
	case TEMPLATE_FIELD_RANGE_CHK:
		pr_debug("TEMPLATE_FIELD_RANGE_CHK: not configured\n");
		break;
	default:
		pr_debug("%s: unknown field %d\n", __func__, field_type);
		return -1;
	}

	return 0;
}

/* Reads the intermediate representation of the templated match-fields of the
 * PIE rule in the pie_rule structure and fills in the raw data fields in the
 * raw register space r[].
 * The register space configuration size is identical for the RTL8380/90 and RTL9300,
 * however the RTL931X has 2 more registers / fields and the physical field-ids are different
 * on all SoCs
 * On the RTL9300 the mask fields are not word-aligend!
 */
static void rtl931x_write_pie_templated(u32 r[], struct pie_rule *pr, enum template_field_id t[])
{
	for (int i = 0; i < N_FIXED_FIELDS; i++) {
		u16 data, data_m;

		rtl931x_pie_data_fill(t[i], pr, &data, &data_m);

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

// Currently unused
// static void rtl931x_read_pie_fixed_fields(u32 r[], struct pie_rule *pr)
// {
// 	pr->mgnt_vlan = r[7] & BIT(31);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw = r[7] & BIT(30);
// 	else  /* TODO: EACL/VACL phase handling */
// 		pr->content_too_deep = r[7] & BIT(30);
// 	pr->not_first_frag = r[7]  & BIT(29);
// 	pr->frame_type_l4 = (r[7] >> 26) & 7;
// 	pr->frame_type = (r[7] >> 24) & 3;
// 	pr->otag_fmt = (r[7] >> 23) & 1;
// 	pr->itag_fmt = (r[7] >> 22) & 1;
// 	pr->otag_exist = (r[7] >> 21) & 1;
// 	pr->itag_exist = (r[7] >> 20) & 1;
// 	pr->frame_type_l2 = (r[7] >> 18) & 3;
// 	pr->igr_normal_port = (r[7] >> 17) & 1;
// 	pr->tid = (r[7] >> 16) & 1;

// 	pr->mgnt_vlan_m = r[14] & BIT(15);
// 	if (pr->phase == PHASE_IACL)
// 		pr->dmac_hit_sw_m = r[14] & BIT(14);
// 	else
// 		pr->content_too_deep_m = r[14] & BIT(14);
// 	pr->not_first_frag_m = r[14] & BIT(13);
// 	pr->frame_type_l4_m = (r[14] >> 10) & 7;
// 	pr->frame_type_m = (r[14] >> 8) & 3;
// 	pr->otag_fmt_m = r[14] & BIT(7);
// 	pr->itag_fmt_m = r[14] & BIT(6);
// 	pr->otag_exist_m = r[14] & BIT(5);
// 	pr->itag_exist_m = r[14] & BIT (4);
// 	pr->frame_type_l2_m = (r[14] >> 2) & 3;
// 	pr->igr_normal_port_m = r[14] & BIT(1);
// 	pr->tid_m = r[14] & 1;

// 	pr->valid = r[15] & BIT(31);
// 	pr->cond_not = r[15] & BIT(30);
// 	pr->cond_and1 = r[15] & BIT(29);
// 	pr->cond_and2 = r[15] & BIT(28);
// }

static void rtl931x_write_pie_fixed_fields(u32 r[],  struct pie_rule *pr)
{
	r[7] |= pr->mgnt_vlan ? BIT(31) : 0;
	if (pr->phase == PHASE_IACL)
		r[7] |= pr->dmac_hit_sw ? BIT(30) : 0;
	else
		r[7] |= pr->content_too_deep ? BIT(30) : 0;
	r[7] |= pr->not_first_frag ? BIT(29) : 0;
	r[7] |= ((u32) (pr->frame_type_l4 & 0x7)) << 26;
	r[7] |= ((u32) (pr->frame_type & 0x3)) << 24;
	r[7] |= pr->otag_fmt ? BIT(23) : 0;
	r[7] |= pr->itag_fmt ? BIT(22) : 0;
	r[7] |= pr->otag_exist ? BIT(21) : 0;
	r[7] |= pr->itag_exist ? BIT(20) : 0;
	r[7] |= ((u32) (pr->frame_type_l2 & 0x3)) << 18;
	r[7] |= pr->igr_normal_port ? BIT(17) : 0;
	r[7] |= ((u32) (pr->tid & 0x1)) << 16;

	r[14] |= pr->mgnt_vlan_m ? BIT(15) : 0;
	if (pr->phase == PHASE_IACL)
		r[14] |= pr->dmac_hit_sw_m ? BIT(14) : 0;
	else
		r[14] |= pr->content_too_deep_m ? BIT(14) : 0;
	r[14] |= pr->not_first_frag_m ? BIT(13) : 0;
	r[14] |= ((u32) (pr->frame_type_l4_m & 0x7)) << 10;
	r[14] |= ((u32) (pr->frame_type_m & 0x3)) << 8;
	r[14] |= pr->otag_fmt_m ? BIT(7) : 0;
	r[14] |= pr->itag_fmt_m ? BIT(6) : 0;
	r[14] |= pr->otag_exist_m ? BIT(5) : 0;
	r[14] |= pr->itag_exist_m ? BIT(4) : 0;
	r[14] |= ((u32) (pr->frame_type_l2_m & 0x3)) << 2;
	r[14] |= pr->igr_normal_port_m ? BIT(1) : 0;
	r[14] |= (u32) (pr->tid_m & 0x1);

	r[15] |= pr->valid ? BIT(31) : 0;
	r[15] |= pr->cond_not ? BIT(30) : 0;
	r[15] |= pr->cond_and1 ? BIT(29) : 0;
	r[15] |= pr->cond_and2 ? BIT(28) : 0;
}

static void rtl931x_write_pie_action(u32 r[],  struct pie_rule *pr)
{
	/* Either drop or forward */
	if (pr->drop) {
		r[15] |= BIT(11) | BIT(12) | BIT(13); /* Do Green, Yellow and Red drops */
		/* Actually DROP, not PERMIT in Green / Yellow / Red */
		r[16] |= BIT(27) | BIT(28) | BIT(29);
	} else {
		r[15] |= pr->fwd_sel ? BIT(14) : 0;
		r[16] |= pr->fwd_act << 24;
		r[16] |= BIT(21); /* We overwrite any drop */
	}
	if (pr->phase == PHASE_VACL)
		r[16] |= pr->fwd_sa_lrn ? BIT(22) : 0;
	r[15] |= pr->bypass_sel ? BIT(10) : 0;
	r[15] |= pr->nopri_sel ? BIT(21) : 0;
	r[15] |= pr->tagst_sel ? BIT(20) : 0;
	r[15] |= pr->ovid_sel ? BIT(18) : 0;
	r[15] |= pr->ivid_sel ? BIT(16) : 0;
	r[15] |= pr->meter_sel ? BIT(27) : 0;
	r[15] |= pr->mir_sel ? BIT(15) : 0;
	r[15] |= pr->log_sel ? BIT(26) : 0;

	r[16] |= ((u32)(pr->fwd_data & 0xfff)) << 9;
/*	r[15] |= pr->log_octets ? BIT(31) : 0; */
	r[15] |= (u32)(pr->meter_data) >> 2;
	r[16] |= (((u32)(pr->meter_data) >> 7) & 0x3) << 29;

	r[16] |= ((u32)(pr->ivid_act & 0x3)) << 21;
	r[15] |= ((u32)(pr->ivid_data & 0xfff)) << 9;
	r[16] |= ((u32)(pr->ovid_act & 0x3)) << 30;
	r[16] |= ((u32)(pr->ovid_data & 0xfff)) << 16;
	r[16] |= ((u32)(pr->mir_data & 0x3)) << 6;
	r[17] |= ((u32)(pr->tagst_data & 0xf)) << 28;
	r[17] |= ((u32)(pr->nopri_data & 0x7)) << 25;
	r[17] |= pr->bypass_ibc_sc ? BIT(16) : 0;
}

static void rtl931x_pie_rule_dump_raw(u32 r[])
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

static int rtl931x_pie_rule_write(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr)
{
	/* Access IACL table (0) via register 1, the table size is 4096 */
	struct table_reg *q = rtl_table_get(RTL9310_TBL_1, 0);
	u32 r[22];
	int block = idx / PIE_BLOCK_SIZE;
	u32 t_select = sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block));

	pr_debug("%s: %d, t_select: %08x\n", __func__, idx, t_select);

	for (int i = 0; i < 22; i++)
		r[i] = 0;

	if (!pr->valid) {
		rtl_table_write(q, idx);
		rtl_table_release(q);
		return 0;
	}
	rtl931x_write_pie_fixed_fields(r, pr);

	pr_debug("%s: template %d\n", __func__, (t_select >> (pr->tid * 4)) & 0xf);
	rtl931x_write_pie_templated(r, pr, fixed_templates[(t_select >> (pr->tid * 4)) & 0xf]);

	rtl931x_write_pie_action(r, pr);

	rtl931x_pie_rule_dump_raw(r);

	for (int i = 0; i < 22; i++)
		sw_w32(r[i], rtl_table_data(q, i));

	rtl_table_write(q, idx);
	rtl_table_release(q);

	return 0;
}

static bool rtl931x_pie_templ_has(int t, enum template_field_id field_type)
{
	for (int i = 0; i < N_FIXED_FIELDS_RTL931X; i++) {
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
static int rtl931x_pie_verify_template(struct rtl838x_switch_priv *priv,
				       struct pie_rule *pr, int t, int block)
{
	int i;

	if (!pr->is_ipv6 && pr->sip_m && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SIP0))
		return -1;

	if (!pr->is_ipv6 && pr->dip_m && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DIP0))
		return -1;

	if (pr->is_ipv6) {
		if ((pr->sip6_m.s6_addr32[0] ||
		     pr->sip6_m.s6_addr32[1] ||
		     pr->sip6_m.s6_addr32[2] ||
		     pr->sip6_m.s6_addr32[3]) &&
		    !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SIP2))
			return -1;
		if ((pr->dip6_m.s6_addr32[0] ||
		     pr->dip6_m.s6_addr32[1] ||
		     pr->dip6_m.s6_addr32[2] ||
		     pr->dip6_m.s6_addr32[3]) &&
		    !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DIP2))
			return -1;
	}

	if (ether_addr_to_u64(pr->smac) && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_SMAC0))
		return -1;

	if (ether_addr_to_u64(pr->dmac) && !rtl931x_pie_templ_has(t, TEMPLATE_FIELD_DMAC0))
		return -1;

	/* TODO: Check more */

	i = find_first_zero_bit(&priv->pie_use_bm[block * 4], PIE_BLOCK_SIZE);

	if (i >= PIE_BLOCK_SIZE)
		return -1;

	return i + PIE_BLOCK_SIZE * block;
}

static int rtl931x_pie_rule_add(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx, block, j;
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
			int t = (sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block)) >> (j * 4)) & 0xf;
			pr_debug("Testing block %d, template %d, template id %d\n", block, j, t);
			pr_debug("%s: %08x\n",
				__func__, sw_r32(RTL931X_PIE_BLK_TMPLTE_CTRL(block)));
			idx = rtl931x_pie_verify_template(priv, pr, t, block);
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

	rtl931x_pie_lookup_enable(priv, idx);
	rtl931x_pie_rule_write(priv, idx, pr);

	mutex_unlock(&priv->pie_mutex);

	return 0;
}

/* Delete a range of Packet Inspection Engine rules */
static int rtl931x_pie_rule_del(struct rtl838x_switch_priv *priv, int index_from, int index_to)
{
	u32 v = (index_from << 1)| (index_to << 13 ) | BIT(0);

	pr_debug("%s: from %d to %d\n", __func__, index_from, index_to);
	mutex_lock(&priv->reg_mutex);

	/* Write from-to and execute bit into control register */
	sw_w32(v, RTL931X_PIE_CLR_CTRL);

	/* Wait until command has completed */
	do {
	} while (sw_r32(RTL931X_PIE_CLR_CTRL) & BIT(0));

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtl931x_pie_rule_rm(struct rtl838x_switch_priv *priv, struct pie_rule *pr)
{
	int idx = pr->id;

	rtl931x_pie_rule_del(priv, idx, idx);
	clear_bit(idx, priv->pie_use_bm);
}

static void rtl931x_pie_init(struct rtl838x_switch_priv *priv)
{
	u32 template_selectors;

	mutex_init(&priv->pie_mutex);

	pr_debug("%s\n", __func__);
	/* Enable ACL lookup on all ports, including CPU_PORT */
	for (int i = 0; i <= priv->cpu_port; i++)
		sw_w32(1, RTL931X_ACL_PORT_LOOKUP_CTRL(i));

	/* Include IPG in metering */
	sw_w32_mask(0, 1, RTL931X_METER_GLB_CTRL);

	/* Delete all present rules, block size is 128 on all SoC families */
	rtl931x_pie_rule_del(priv, 0, priv->n_pie_blocks * 128 - 1);

	/* Assign first half blocks 0-7 to VACL phase, second half to IACL */
	/* 3 bits are used for each block, values for PIE blocks are */
	/* 6: Disabled, 0: VACL, 1: IACL, 2: EACL */
	/* And for OpenFlow Flow blocks: 3: Ingress Flow table 0, */
	/* 4: Ingress Flow Table 3, 5: Egress flow table 0 */
	for (int i = 0; i < priv->n_pie_blocks; i++) {
		int pos = (i % 10) * 3;
		u32 r = RTL931X_PIE_BLK_PHASE_CTRL + 4 * (i / 10);

		if (i < priv->n_pie_blocks / 2)
			sw_w32_mask(0x7 << pos, 0, r);
		else
			sw_w32_mask(0x7 << pos, 1 << pos, r);
	}

	/* Enable predefined templates 0, 1 for first quarter of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = 0; i < priv->n_pie_blocks / 4; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for second quarter of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks / 4; i < priv->n_pie_blocks / 2; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 0, 1 for third quater of all blocks */
	template_selectors = 0 | (1 << 4);
	for (int i = priv->n_pie_blocks / 2; i < priv->n_pie_blocks * 3 / 4; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

	/* Enable predefined templates 2, 3 for fourth quater of all blocks */
	template_selectors = 2 | (3 << 4);
	for (int i = priv->n_pie_blocks * 3 / 4; i < priv->n_pie_blocks; i++)
		sw_w32(template_selectors, RTL931X_PIE_BLK_TMPLTE_CTRL(i));

}

static int rtl931x_l3_setup(struct rtl838x_switch_priv *priv)
{
	return 0;
}

static void rtl931x_vlan_port_keep_tag_set(int port, bool keep_outer, bool keep_inner)
{
	sw_w32(FIELD_PREP(RTL931X_VLAN_PORT_TAG_EGR_OTAG_STS_MASK,
			  keep_outer ? RTL931X_VLAN_PORT_TAG_STS_TAGGED : RTL931X_VLAN_PORT_TAG_STS_UNTAG) |
	       FIELD_PREP(RTL931X_VLAN_PORT_TAG_EGR_ITAG_STS_MASK,
			  keep_inner ? RTL931X_VLAN_PORT_TAG_STS_TAGGED : RTL931X_VLAN_PORT_TAG_STS_UNTAG),
	       RTL931X_VLAN_PORT_TAG_CTRL(port));
}

static void rtl931x_vlan_port_pvidmode_set(int port, enum pbvlan_type type, enum pbvlan_mode mode)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0x3 << 12, mode << 12, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
	else
		sw_w32_mask(0x3 << 26, mode << 26, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
}

static void rtl931x_vlan_port_pvid_set(int port, enum pbvlan_type type, int pvid)
{
	if (type == PBVLAN_TYPE_INNER)
		sw_w32_mask(0xfff, pvid, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
	else
		sw_w32_mask(0xfff << 14, pvid << 14, RTL931X_VLAN_PORT_IGR_CTRL + (port << 2));
}

static void rtl931x_set_igr_filter(int port, enum igr_filter state)
{
	sw_w32_mask(0x3 << ((port & 0xf)<<1), state << ((port & 0xf)<<1),
		    RTL931X_VLAN_PORT_IGR_FLTR + (((port >> 4) << 2)));
}

static void rtl931x_set_egr_filter(int port,  enum egr_filter state)
{
	sw_w32_mask(0x1 << (port % 0x20), state << (port % 0x20),
		    RTL931X_VLAN_PORT_EGR_FLTR + (((port >> 5) << 2)));
}

static void rtl931x_set_distribution_algorithm(int group, int algoidx, u32 algomsk)
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
	} else {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT;
	}

	sw_w32(newmask << l3shift, RTL931X_TRK_HASH_CTRL + (algoidx << 2));
}

static void rtl931x_led_init(struct rtl838x_switch_priv *priv)
{
	u64 pm_copper = 0, pm_fiber = 0;
	struct device_node *node;

	pr_debug("%s called\n", __func__);
	node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300-leds");
	if (!node) {
		pr_debug("%s No compatible LED node found\n", __func__);
		return;
	}

	for (int i = 0; i < priv->cpu_port; i++) {
		int pos = (i << 1) % 32;
		u32 set;
		u32 v;

		sw_w32_mask(0x3 << pos, 0, RTL931X_LED_PORT_FIB_SET_SEL_CTRL(i));
		sw_w32_mask(0x3 << pos, 0, RTL931X_LED_PORT_COPR_SET_SEL_CTRL(i));

		if (!priv->ports[i].phy)
			continue;

		v = 0x1; /* Found on the EdgeCore, but we do not have any HW description */
		sw_w32_mask(0x3 << pos, v << pos, RTL931X_LED_PORT_NUM_CTRL(i));

		if (priv->ports[i].phy_is_integrated)
			pm_fiber |= BIT_ULL(i);
		else
			pm_copper |= BIT_ULL(i);

		set = priv->ports[i].led_set;
		sw_w32_mask(0, set << pos, RTL931X_LED_PORT_COPR_SET_SEL_CTRL(i));
		sw_w32_mask(0, set << pos, RTL931X_LED_PORT_FIB_SET_SEL_CTRL(i));
	}

	for (int i = 0; i < 4; i++) {
		const __be32 *led_set;
		char set_name[9];
		u32 setlen;
		u32 v;

		sprintf(set_name, "led_set%d", i);
		pr_debug(">%s<\n", set_name);
		led_set = of_get_property(node, set_name, &setlen);
		if (!led_set || setlen != 16)
			break;
		v = be32_to_cpup(led_set) << 16 | be32_to_cpup(led_set + 1);
		sw_w32(v, RTL931X_LED_SET0_0_CTRL - 4 - i * 8);
		v = be32_to_cpup(led_set + 2) << 16 | be32_to_cpup(led_set + 3);
		sw_w32(v, RTL931X_LED_SET0_0_CTRL - i * 8);
	}

	/* Set LED mode to serial (0x1) */
	sw_w32_mask(0x3, 0x1, RTL931X_LED_GLB_CTRL);

	rtl839x_set_port_reg_le(pm_copper, RTL931X_LED_PORT_COPR_MASK_CTRL);
	rtl839x_set_port_reg_le(pm_fiber, RTL931X_LED_PORT_FIB_MASK_CTRL);
	rtl839x_set_port_reg_le(pm_copper | pm_fiber, RTL931X_LED_PORT_COMBO_MASK_CTRL);

	for (int i = 0; i < 32; i++)
		pr_debug("%s %08x: %08x\n",__func__, 0xbb000600 + i * 4, sw_r32(0x0600 + i * 4));
}

const struct rtl838x_reg rtl931x_reg = {
	.mask_port_reg_be = rtl839x_mask_port_reg_be,
	.set_port_reg_be = rtl839x_set_port_reg_be,
	.get_port_reg_be = rtl839x_get_port_reg_be,
	.mask_port_reg_le = rtl839x_mask_port_reg_le,
	.set_port_reg_le = rtl839x_set_port_reg_le,
	.get_port_reg_le = rtl839x_get_port_reg_le,
	.stat_port_rst = RTL931X_STAT_PORT_RST,
	.stat_rst = RTL931X_STAT_RST,
	.stat_port_std_mib = 0,  /* Not defined */
	.traffic_enable = rtl931x_traffic_enable,
	.traffic_disable = rtl931x_traffic_disable,
	.traffic_get = rtl931x_traffic_get,
	.traffic_set = rtl931x_traffic_set,
	.l2_ctrl_0 = RTL931X_L2_CTRL,
	.l2_ctrl_1 = RTL931X_L2_AGE_CTRL,
	.l2_port_aging_out = RTL931X_L2_PORT_AGE_CTRL,
	.set_ageing_time = rtl931x_set_ageing_time,
	/* .smi_poll_ctrl does not exist */
	.l2_tbl_flush_ctrl = RTL931X_L2_TBL_FLUSH_CTRL,
	.exec_tbl0_cmd = rtl931x_exec_tbl0_cmd,
	.exec_tbl1_cmd = rtl931x_exec_tbl1_cmd,
	.tbl_access_data_0 = rtl931x_tbl_access_data_0,
	.isr_glb_src = RTL931X_ISR_GLB_SRC,
	.isr_port_link_sts_chg = RTL931X_ISR_PORT_LINK_STS_CHG,
	.imr_port_link_sts_chg = RTL931X_IMR_PORT_LINK_STS_CHG,
	/* imr_glb does not exist on RTL931X */
	.vlan_tables_read = rtl931x_vlan_tables_read,
	.vlan_set_tagged = rtl931x_vlan_set_tagged,
	.vlan_set_untagged = rtl931x_vlan_set_untagged,
	.vlan_profile_dump = rtl931x_vlan_profile_dump,
	.vlan_profile_setup = rtl931x_vlan_profile_setup,
	.vlan_fwd_on_inner = rtl931x_vlan_fwd_on_inner,
	.stp_get = rtl931x_stp_get,
	.stp_set = rtl931x_stp_set,
	.mac_force_mode_ctrl = rtl931x_mac_force_mode_ctrl,
	.mac_port_ctrl = rtl931x_mac_port_ctrl,
	.l2_port_new_salrn = rtl931x_l2_port_new_salrn,
	.l2_port_new_sa_fwd = rtl931x_l2_port_new_sa_fwd,
	.mir_ctrl = RTL931X_MIR_CTRL,
	.mir_dpm = RTL931X_MIR_DPM_CTRL,
	.mir_spm = RTL931X_MIR_SPM_CTRL,
	.mac_link_sts = RTL931X_MAC_LINK_STS,
	.mac_link_dup_sts = RTL931X_MAC_LINK_DUP_STS,
	.mac_link_spd_sts = rtl931x_mac_link_spd_sts,
	.mac_rx_pause_sts = RTL931X_MAC_RX_PAUSE_STS,
	.mac_tx_pause_sts = RTL931X_MAC_TX_PAUSE_STS,
	.read_l2_entry_using_hash = rtl931x_read_l2_entry_using_hash,
	.write_l2_entry_using_hash = rtl931x_write_l2_entry_using_hash,
	.read_cam = rtl931x_read_cam,
	.write_cam = rtl931x_write_cam,
	.vlan_port_keep_tag_set = rtl931x_vlan_port_keep_tag_set,
	.vlan_port_pvidmode_set = rtl931x_vlan_port_pvidmode_set,
	.vlan_port_pvid_set = rtl931x_vlan_port_pvid_set,
	.trk_mbr_ctr = rtl931x_trk_mbr_ctr,
	.set_vlan_igr_filter = rtl931x_set_igr_filter,
	.set_vlan_egr_filter = rtl931x_set_egr_filter,
	.set_distribution_algorithm = rtl931x_set_distribution_algorithm,
	.l2_hash_key = rtl931x_l2_hash_key,
	.read_mcast_pmask = rtl931x_read_mcast_pmask,
	.write_mcast_pmask = rtl931x_write_mcast_pmask,
	.pie_init = rtl931x_pie_init,
	.pie_rule_write = rtl931x_pie_rule_write,
	.pie_rule_add = rtl931x_pie_rule_add,
	.pie_rule_rm = rtl931x_pie_rule_rm,
	.l2_learning_setup = rtl931x_l2_learning_setup,
	.l3_setup = rtl931x_l3_setup,
	.led_init = rtl931x_led_init,
};
