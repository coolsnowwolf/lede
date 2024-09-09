// SPDX-License-Identifier: GPL-2.0-only

#include <net/dsa.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <asm/mach-rtl838x/mach-rtl83xx.h>

#include "rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;

static void rtl83xx_init_stats(struct rtl838x_switch_priv *priv)
{
	mutex_lock(&priv->reg_mutex);

	/* Enable statistics module: all counters plus debug.
	 * On RTL839x all counters are enabled by default
	 */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32_mask(0, 3, RTL838X_STAT_CTRL);

	/* Reset statistics counters */
	sw_w32_mask(0, 1, priv->r->stat_rst);

	mutex_unlock(&priv->reg_mutex);
}

static void rtl83xx_enable_phy_polling(struct rtl838x_switch_priv *priv)
{
	u64 v = 0;

	msleep(1000);
	/* Enable all ports with a PHY, including the SFP-ports */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy)
			v |= BIT_ULL(i);
	}

	pr_info("%s: %16llx\n", __func__, v);
	priv->r->set_port_reg_le(v, priv->r->smi_poll_ctrl);

	/* PHY update complete, there is no global PHY polling enable bit on the 9300 */
	if (priv->family_id == RTL8390_FAMILY_ID)
		sw_w32_mask(0, BIT(7), RTL839X_SMI_GLB_CTRL);
	else if(priv->family_id == RTL9300_FAMILY_ID)
		sw_w32_mask(0, 0x8000, RTL838X_SMI_GLB_CTRL);
}

const struct rtl83xx_mib_desc rtl83xx_mib[] = {
	MIB_DESC(2, 0xf8, "ifInOctets"),
	MIB_DESC(2, 0xf0, "ifOutOctets"),
	MIB_DESC(1, 0xec, "dot1dTpPortInDiscards"),
	MIB_DESC(1, 0xe8, "ifInUcastPkts"),
	MIB_DESC(1, 0xe4, "ifInMulticastPkts"),
	MIB_DESC(1, 0xe0, "ifInBroadcastPkts"),
	MIB_DESC(1, 0xdc, "ifOutUcastPkts"),
	MIB_DESC(1, 0xd8, "ifOutMulticastPkts"),
	MIB_DESC(1, 0xd4, "ifOutBroadcastPkts"),
	MIB_DESC(1, 0xd0, "ifOutDiscards"),
	MIB_DESC(1, 0xcc, ".3SingleCollisionFrames"),
	MIB_DESC(1, 0xc8, ".3MultipleCollisionFrames"),
	MIB_DESC(1, 0xc4, ".3DeferredTransmissions"),
	MIB_DESC(1, 0xc0, ".3LateCollisions"),
	MIB_DESC(1, 0xbc, ".3ExcessiveCollisions"),
	MIB_DESC(1, 0xb8, ".3SymbolErrors"),
	MIB_DESC(1, 0xb4, ".3ControlInUnknownOpcodes"),
	MIB_DESC(1, 0xb0, ".3InPauseFrames"),
	MIB_DESC(1, 0xac, ".3OutPauseFrames"),
	MIB_DESC(1, 0xa8, "DropEvents"),
	MIB_DESC(1, 0xa4, "tx_BroadcastPkts"),
	MIB_DESC(1, 0xa0, "tx_MulticastPkts"),
	MIB_DESC(1, 0x9c, "CRCAlignErrors"),
	MIB_DESC(1, 0x98, "tx_UndersizePkts"),
	MIB_DESC(1, 0x94, "rx_UndersizePkts"),
	MIB_DESC(1, 0x90, "rx_UndersizedropPkts"),
	MIB_DESC(1, 0x8c, "tx_OversizePkts"),
	MIB_DESC(1, 0x88, "rx_OversizePkts"),
	MIB_DESC(1, 0x84, "Fragments"),
	MIB_DESC(1, 0x80, "Jabbers"),
	MIB_DESC(1, 0x7c, "Collisions"),
	MIB_DESC(1, 0x78, "tx_Pkts64Octets"),
	MIB_DESC(1, 0x74, "rx_Pkts64Octets"),
	MIB_DESC(1, 0x70, "tx_Pkts65to127Octets"),
	MIB_DESC(1, 0x6c, "rx_Pkts65to127Octets"),
	MIB_DESC(1, 0x68, "tx_Pkts128to255Octets"),
	MIB_DESC(1, 0x64, "rx_Pkts128to255Octets"),
	MIB_DESC(1, 0x60, "tx_Pkts256to511Octets"),
	MIB_DESC(1, 0x5c, "rx_Pkts256to511Octets"),
	MIB_DESC(1, 0x58, "tx_Pkts512to1023Octets"),
	MIB_DESC(1, 0x54, "rx_Pkts512to1023Octets"),
	MIB_DESC(1, 0x50, "tx_Pkts1024to1518Octets"),
	MIB_DESC(1, 0x4c, "rx_StatsPkts1024to1518Octets"),
	MIB_DESC(1, 0x48, "tx_Pkts1519toMaxOctets"),
	MIB_DESC(1, 0x44, "rx_Pkts1519toMaxOctets"),
	MIB_DESC(1, 0x40, "rxMacDiscards")
};


/* DSA callbacks */


static enum dsa_tag_protocol rtl83xx_get_tag_protocol(struct dsa_switch *ds,
						      int port,
						      enum dsa_tag_protocol mprot)
{
	/* The switch does not tag the frames, instead internally the header
	 * structure for each packet is tagged accordingly.
	 */
	return DSA_TAG_PROTO_TRAILER;
}

static void rtl83xx_vlan_set_pvid(struct rtl838x_switch_priv *priv,
				  int port, int pvid)
{
	/* Set both inner and outer PVID of the port */
	priv->r->vlan_port_pvid_set(port, PBVLAN_TYPE_INNER, pvid);
	priv->r->vlan_port_pvid_set(port, PBVLAN_TYPE_OUTER, pvid);
	priv->r->vlan_port_pvidmode_set(port, PBVLAN_TYPE_INNER,
					PBVLAN_MODE_UNTAG_AND_PRITAG);
	priv->r->vlan_port_pvidmode_set(port, PBVLAN_TYPE_OUTER,
					PBVLAN_MODE_UNTAG_AND_PRITAG);

	priv->ports[port].pvid = pvid;
}

/* Initialize all VLANS */
static void rtl83xx_vlan_setup(struct rtl838x_switch_priv *priv)
{
	struct rtl838x_vlan_info info;

	pr_info("In %s\n", __func__);

	priv->r->vlan_profile_setup(0);
	priv->r->vlan_profile_setup(1);
	pr_info("UNKNOWN_MC_PMASK: %016llx\n", priv->r->read_mcast_pmask(UNKNOWN_MC_PMASK));
	priv->r->vlan_profile_dump(0);

	info.fid = 0;			/* Default Forwarding ID / MSTI */
	info.hash_uc_fid = false;	/* Do not build the L2 lookup hash with FID, but VID */
	info.hash_mc_fid = false;	/* Do the same for Multicast packets */
	info.profile_id = 0;		/* Use default Vlan Profile 0 */
	info.tagged_ports = 0;		/* Initially no port members */
	if (priv->family_id == RTL9310_FAMILY_ID) {
		info.if_id = 0;
		info.multicast_grp_mask = 0;
		info.l2_tunnel_list_id = -1;
	}

	/* Initialize normal VLANs 1-4095 */
	for (int i = 1; i < MAX_VLANS; i ++)
		priv->r->vlan_set_tagged(i, &info);

	/*
	 * Initialize the special VLAN 0 and reset PVIDs. The CPU port PVID
	 * is applied to packets from the CPU for untagged destinations,
	 * regardless if the actual ingress VID. Any port with untagged
	 * egress VLAN(s) must therefore be a member of VLAN 0 to support
	 * CPU port as ingress when VLAN filtering is enabled.
	 */
	for (int i = 0; i <= priv->cpu_port; i++) {
		rtl83xx_vlan_set_pvid(priv, i, 0);
		info.tagged_ports |= BIT_ULL(i);
	}
	priv->r->vlan_set_tagged(0, &info);

	/* Set forwarding action based on inner VLAN tag */
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->vlan_fwd_on_inner(i, true);
}

static void rtl83xx_setup_bpdu_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->set_receive_management_action(i, BPDU, TRAP2CPU);
}

static void rtl83xx_setup_lldp_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->cpu_port; i++)
		priv->r->set_receive_management_action(i, LLDP, TRAP2CPU);
}

static void rtl83xx_port_set_salrn(struct rtl838x_switch_priv *priv,
				   int port, bool enable)
{
	int shift = SALRN_PORT_SHIFT(port);
	int val = enable ? SALRN_MODE_HARDWARE : SALRN_MODE_DISABLED;

	sw_w32_mask(SALRN_MODE_MASK << shift, val << shift,
		    priv->r->l2_port_new_salrn(port));
}

static int rtl83xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s called\n", __func__);

	/* Disable MAC polling the PHY so that we can start configuration */
	priv->r->set_port_reg_le(0ULL, priv->r->smi_poll_ctrl);

	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy) {
			priv->ports[i].pm = BIT_ULL(priv->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->cpu_port, BIT_ULL(priv->cpu_port));

	/* For standalone ports, forward packets even if a static fdb
	 * entry for the source address exists on another port.
	 */
	if (priv->r->set_static_move_action) {
		for (int i = 0; i <= priv->cpu_port; i++)
			priv->r->set_static_move_action(i, true);
	}

	if (priv->family_id == RTL8380_FAMILY_ID)
		rtl838x_print_matrix();
	else
		rtl839x_print_matrix();

	rtl83xx_init_stats(priv);

	rtl83xx_vlan_setup(priv);

	rtl83xx_setup_bpdu_traps(priv);
	rtl83xx_setup_lldp_traps(priv);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtl83xx_port_set_salrn(priv, priv->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	/* Make sure all frames sent to the switch's MAC are trapped to the CPU-port
	 *  0: FWD, 1: DROP, 2: TRAP2CPU
	 */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32(0x2, RTL838X_SPCL_TRAP_SWITCH_MAC_CTRL);
	else
		sw_w32(0x2, RTL839X_SPCL_TRAP_SWITCH_MAC_CTRL);

	/* Enable MAC Polling PHY again */
	rtl83xx_enable_phy_polling(priv);
	pr_debug("Please wait until PHY is settled\n");
	msleep(1000);
	priv->r->pie_init(priv);

	return 0;
}

static int rtl93xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_info("%s called\n", __func__);

	/* Disable MAC polling the PHY so that we can start configuration */
	if (priv->family_id == RTL9300_FAMILY_ID)
		sw_w32(0, RTL930X_SMI_POLL_CTRL);

	if (priv->family_id == RTL9310_FAMILY_ID) {
		sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL);
		sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL + 4);
	}

	/* Disable all ports except CPU port */
	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy) {
			priv->ports[i].pm = BIT_ULL(priv->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->cpu_port, BIT_ULL(priv->cpu_port));

	rtl930x_print_matrix();

	/* TODO: Initialize statistics */

	rtl83xx_vlan_setup(priv);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtl83xx_port_set_salrn(priv, priv->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	rtl83xx_enable_phy_polling(priv);

	priv->r->pie_init(priv);

	priv->r->led_init(priv);

	return 0;
}

static int rtl93xx_get_sds(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct device_node *dn;
	u32 sds_num;

	if (!dev)
		return -1;
	if (dev->of_node) {
		dn = dev->of_node;
		if (of_property_read_u32(dn, "sds", &sds_num))
			sds_num = -1;
	} else {
		dev_err(dev, "No DT node.\n");
		return -1;
	}

	return sds_num;
}

static int rtl83xx_pcs_validate(struct phylink_pcs *pcs,
				unsigned long *supported,
				const struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	pr_debug("In %s port %d, state is %d", __func__, port, state->interface);

	if (!phy_interface_mode_is_rgmii(state->interface) &&
	    state->interface != PHY_INTERFACE_MODE_NA &&
	    state->interface != PHY_INTERFACE_MODE_1000BASEX &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    state->interface != PHY_INTERFACE_MODE_QSGMII &&
	    state->interface != PHY_INTERFACE_MODE_INTERNAL &&
	    state->interface != PHY_INTERFACE_MODE_SGMII) {
		bitmap_zero(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
		dev_err(priv->ds->dev,
			"Unsupported interface: %d for port %d\n",
			state->interface, port);
		return -EINVAL;
	}

	/* Allow all the expected bits */
	phylink_set(mask, Autoneg);
	phylink_set_port_modes(mask);
	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	/* With the exclusion of MII and Reverse MII, we support Gigabit,
	 * including Half duplex
	 */
	if (state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseT_Half);
	}

	/* On both the 8380 and 8382, ports 24-27 are SFP ports */
	if (port >= 24 && port <= 27 && priv->family_id == RTL8380_FAMILY_ID)
		phylink_set(mask, 1000baseX_Full);

	/* On the RTL839x family of SoCs, ports 48 to 51 are SFP ports */
	if (port >= 48 && port <= 51 && priv->family_id == RTL8390_FAMILY_ID)
		phylink_set(mask, 1000baseX_Full);

	phylink_set(mask, 10baseT_Half);
	phylink_set(mask, 10baseT_Full);
	phylink_set(mask, 100baseT_Half);
	phylink_set(mask, 100baseT_Full);

	bitmap_and(supported, supported, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);

	return 0;
}

static int rtl93xx_pcs_validate(struct phylink_pcs *pcs,
				unsigned long *supported,
				const struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	pr_debug("In %s port %d, state is %d (%s)", __func__, port, state->interface,
		 phy_modes(state->interface));

	if (!phy_interface_mode_is_rgmii(state->interface) &&
	    state->interface != PHY_INTERFACE_MODE_NA &&
	    state->interface != PHY_INTERFACE_MODE_1000BASEX &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    state->interface != PHY_INTERFACE_MODE_QSGMII &&
	    state->interface != PHY_INTERFACE_MODE_XGMII &&
	    state->interface != PHY_INTERFACE_MODE_HSGMII &&
	    state->interface != PHY_INTERFACE_MODE_10GBASER &&
	    state->interface != PHY_INTERFACE_MODE_10GKR &&
	    state->interface != PHY_INTERFACE_MODE_USXGMII &&
	    state->interface != PHY_INTERFACE_MODE_INTERNAL &&
	    state->interface != PHY_INTERFACE_MODE_SGMII) {
		bitmap_zero(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
		dev_err(priv->ds->dev,
			"Unsupported interface: %d for port %d\n",
			state->interface, port);
		return -EINVAL;
	}

	/* Allow all the expected bits */
	phylink_set(mask, Autoneg);
	phylink_set_port_modes(mask);
	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	/* With the exclusion of MII and Reverse MII, we support Gigabit,
	 * including Half duplex
	 */
	if (state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseT_Half);
	}

	/* Internal phys of the RTL93xx family provide 10G */
	if (priv->ports[port].phy_is_integrated &&
	    state->interface == PHY_INTERFACE_MODE_1000BASEX) {
		phylink_set(mask, 1000baseX_Full);
	} else if (priv->ports[port].phy_is_integrated) {
		phylink_set(mask, 1000baseX_Full);
		phylink_set(mask, 10000baseKR_Full);
		phylink_set(mask, 10000baseSR_Full);
		phylink_set(mask, 10000baseCR_Full);
	}
	if (state->interface == PHY_INTERFACE_MODE_INTERNAL) {
		phylink_set(mask, 1000baseX_Full);
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 10000baseKR_Full);
		phylink_set(mask, 10000baseT_Full);
		phylink_set(mask, 10000baseSR_Full);
		phylink_set(mask, 10000baseCR_Full);
	}

	if (state->interface == PHY_INTERFACE_MODE_USXGMII) {
		phylink_set(mask, 2500baseT_Full);
		phylink_set(mask, 5000baseT_Full);
		phylink_set(mask, 10000baseT_Full);
	}

	phylink_set(mask, 10baseT_Half);
	phylink_set(mask, 10baseT_Full);
	phylink_set(mask, 100baseT_Half);
	phylink_set(mask, 100baseT_Full);

	bitmap_and(supported, supported, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);
	pr_debug("%s leaving supported: %*pb", __func__, __ETHTOOL_LINK_MODE_MASK_NBITS, supported);

	return 0;
}

static void rtl83xx_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	u64 speed;
	u64 link;

	if (port < 0 || port > priv->cpu_port) {
		state->link = false;
		return;
	}

	state->link = 0;
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	if (link & BIT_ULL(port))
		state->link = 1;
	pr_debug("%s: link state port %d: %llx\n", __func__, port, link & BIT_ULL(port));

	state->duplex = 0;
	if (priv->r->get_port_reg_le(priv->r->mac_link_dup_sts) & BIT_ULL(port))
		state->duplex = 1;

	speed = priv->r->get_port_reg_le(priv->r->mac_link_spd_sts(port));
	speed >>= (port % 16) << 1;
	switch (speed & 0x3) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
		state->speed = SPEED_1000;
		break;
	case 3:
		if (priv->family_id == RTL9300_FAMILY_ID
			&& (port == 24 || port == 26)) /* Internal serdes */
			state->speed = SPEED_2500;
		else
			state->speed = SPEED_100; /* Is in fact 500Mbit */
	}

	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (priv->r->get_port_reg_le(priv->r->mac_rx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_RX;
	if (priv->r->get_port_reg_le(priv->r->mac_tx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_TX;
}

static void rtl93xx_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	struct rtl838x_pcs *rtpcs = container_of(pcs, struct rtl838x_pcs, pcs);
	struct rtl838x_switch_priv *priv = rtpcs->priv;
	int port = rtpcs->port;
	u64 speed;
	u64 link;
	u64 media;

	if (port < 0 || port > priv->cpu_port) {
		state->link = false;
		return;
	}

	/* On the RTL9300 for at least the RTL8226B PHY, the MAC-side link
	 * state needs to be read twice in order to read a correct result.
	 * This would not be necessary for ports connected e.g. to RTL8218D
	 * PHYs.
	 */
	state->link = 0;
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	if (link & BIT_ULL(port))
		state->link = 1;

	if (priv->family_id == RTL9310_FAMILY_ID)
		media = priv->r->get_port_reg_le(RTL931X_MAC_LINK_MEDIA_STS);

	if (priv->family_id == RTL9300_FAMILY_ID)
		media = sw_r32(RTL930X_MAC_LINK_MEDIA_STS);

	if (media & BIT_ULL(port))
		state->link = 1;

	pr_debug("%s: link state port %d: %llx, media %llx\n", __func__, port,
		 link & BIT_ULL(port), media);

	state->duplex = 0;
	if (priv->r->get_port_reg_le(priv->r->mac_link_dup_sts) & BIT_ULL(port))
		state->duplex = 1;

	speed = priv->r->get_port_reg_le(priv->r->mac_link_spd_sts(port));
	speed >>= (port % 8) << 2;
	switch (speed & 0xf) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
	case 7:
		state->speed = SPEED_1000;
		break;
	case 4:
		state->speed = SPEED_10000;
		break;
	case 5:
	case 8:
		state->speed = SPEED_2500;
		break;
	case 6:
		state->speed = SPEED_5000;
		break;
	default:
		pr_err("%s: unknown speed: %d\n", __func__, (u32)speed & 0xf);
	}

	if (priv->family_id == RTL9310_FAMILY_ID
		&& (port >= 52 && port <= 55)) { /* Internal serdes */
			state->speed = SPEED_10000;
			state->link = 1;
			state->duplex = 1;
	}

	pr_debug("%s: speed is: %d %d\n", __func__, (u32)speed & 0xf, state->speed);
	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (priv->r->get_port_reg_le(priv->r->mac_rx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_RX;
	if (priv->r->get_port_reg_le(priv->r->mac_tx_pause_sts) & BIT_ULL(port))
		state->pause |= MLO_PAUSE_TX;
}

static int rtl83xx_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			      phy_interface_t interface,
			      const unsigned long *advertising,
			      bool permit_pause_to_mac)
{
	return 0;
}

static void rtl83xx_pcs_an_restart(struct phylink_pcs *pcs)
{
/* No restart functionality existed before we migrated to pcs */
}

static struct phylink_pcs *rtl83xx_phylink_mac_select_pcs(struct dsa_switch *ds,
							  int port,
							  phy_interface_t interface)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	return &priv->pcs[port].pcs;
}

static void rtl83xx_config_interface(int port, phy_interface_t interface)
{
	u32 old, int_shift, sds_shift;

	switch (port) {
	case 24:
		int_shift = 0;
		sds_shift = 5;
		break;
	case 26:
		int_shift = 3;
		sds_shift = 0;
		break;
	default:
		return;
	}

	old = sw_r32(RTL838X_SDS_MODE_SEL);
	switch (interface) {
	case PHY_INTERFACE_MODE_1000BASEX:
		if ((old >> sds_shift & 0x1f) == 4)
			return;
		sw_w32_mask(0x7 << int_shift, 1 << int_shift, RTL838X_INT_MODE_CTRL);
		sw_w32_mask(0x1f << sds_shift, 4 << sds_shift, RTL838X_SDS_MODE_SEL);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		if ((old >> sds_shift & 0x1f) == 2)
			return;
		sw_w32_mask(0x7 << int_shift, 2 << int_shift, RTL838X_INT_MODE_CTRL);
		sw_w32_mask(0x1f << sds_shift, 2 << sds_shift, RTL838X_SDS_MODE_SEL);
		break;
	default:
		return;
	}
	pr_debug("configured port %d for interface %s\n", port, phy_modes(interface));
}

static void rtl83xx_phylink_get_caps(struct dsa_switch *ds, int port,
				     struct phylink_config *config)
{
/*
 * This capability check will need some love. Depending on the model and the port
 * different link modes are supported. For now just enable all required values
 * so that we can make use of the ports.
 */
	__set_bit(PHY_INTERFACE_MODE_INTERNAL, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_GMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_QSGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_SGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_XGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_USXGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->supported_interfaces);
}

static void rtl83xx_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	struct dsa_port *dp = dsa_to_port(ds, port);
	struct rtl838x_switch_priv *priv = ds->priv;
	u32 mcr;

	pr_debug("%s port %d, mode %x\n", __func__, port, mode);

	/* currently only needed for RTL8380 */
	if (priv->family_id != RTL8380_FAMILY_ID)
		return;

	if (dsa_port_is_cpu(dp)) {
		/* allow CRC errors on CPU-port */
		sw_w32_mask(0, 0x8, priv->r->mac_port_ctrl(port));
		return;
	}

	mcr = sw_r32(priv->r->mac_force_mode_ctrl(port));
	if (mode == MLO_AN_PHY || phylink_autoneg_inband(mode)) {
		pr_debug("port %d PHY autonegotiates\n", port);
		rtl83xx_config_interface(port, state->interface);
		mcr |= RTL838X_NWAY_EN;
	} else {
		mcr &= ~RTL838X_NWAY_EN;
	}
	sw_w32(mcr, priv->r->mac_force_mode_ctrl(port));
}

static void rtl931x_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int sds_num;
	u32 reg, band;

	sds_num = priv->ports[port].sds_num;
	pr_info("%s: speed %d sds_num %d\n", __func__, state->speed, sds_num);

	switch (state->interface) {
	case PHY_INTERFACE_MODE_HSGMII:
		pr_info("%s setting mode PHY_INTERFACE_MODE_HSGMII\n", __func__);
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_HSGMII);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_HSGMII);
		band = rtl931x_sds_cmu_band_set(sds_num, true, 62, PHY_INTERFACE_MODE_HSGMII);
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_1000BASEX);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_1000BASEX);
		break;
	case PHY_INTERFACE_MODE_XGMII:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_XGMII);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_XGMII);
		break;
	case PHY_INTERFACE_MODE_10GBASER:
	case PHY_INTERFACE_MODE_10GKR:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_10GBASER);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_10GBASER);
		break;
	case PHY_INTERFACE_MODE_USXGMII:
		/* Translates to MII_USXGMII_10GSXGMII */
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_USXGMII);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_USXGMII);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		pr_info("%s setting mode PHY_INTERFACE_MODE_SGMII\n", __func__);
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_SGMII);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_SGMII);
		band = rtl931x_sds_cmu_band_set(sds_num, true, 62, PHY_INTERFACE_MODE_SGMII);
		break;
	case PHY_INTERFACE_MODE_QSGMII:
		band = rtl931x_sds_cmu_band_get(sds_num, PHY_INTERFACE_MODE_QSGMII);
		rtl931x_sds_init(sds_num, PHY_INTERFACE_MODE_QSGMII);
		break;
	default:
		pr_err("%s: unknown serdes mode: %s\n",
			__func__, phy_modes(state->interface));
		return;
	}

	reg = sw_r32(priv->r->mac_force_mode_ctrl(port));
	pr_info("%s reading FORCE_MODE_CTRL: %08x\n", __func__, reg);

	reg &= ~(RTL931X_DUPLEX_MODE | RTL931X_FORCE_EN | RTL931X_FORCE_LINK_EN);

	reg &= ~(0xf << 12);
	reg |= 0x2 << 12; /* Set SMI speed to 0x2 */

	reg |= RTL931X_TX_PAUSE_EN | RTL931X_RX_PAUSE_EN;

	if (priv->lagmembers & BIT_ULL(port))
		reg |= RTL931X_DUPLEX_MODE;

	if (state->duplex == DUPLEX_FULL)
		reg |= RTL931X_DUPLEX_MODE;

	sw_w32(reg, priv->r->mac_force_mode_ctrl(port));

}

static void rtl93xx_phylink_mac_config(struct dsa_switch *ds, int port,
					unsigned int mode,
					const struct phylink_link_state *state)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int sds_num;

	/* Nothing to be done for the CPU-port */
	if (port == priv->cpu_port)
		return;

	if (priv->family_id == RTL9310_FAMILY_ID)
		return rtl931x_phylink_mac_config(ds, port, mode, state);

	sds_num = priv->ports[port].sds_num;
	pr_info("%s SDS is %d\n", __func__, sds_num);
	if (sds_num >= 0 &&
	    (state->interface == PHY_INTERFACE_MODE_1000BASEX ||
	     state->interface == PHY_INTERFACE_MODE_10GBASER))
		rtl9300_serdes_setup(port, sds_num, state->interface);
}

static void rtl83xx_phylink_mac_link_down(struct dsa_switch *ds, int port,
				     unsigned int mode,
				     phy_interface_t interface)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int mask = 0;

	/* Stop TX/RX to port */
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(port));

	/* No longer force link */
	mask = RTL83XX_FORCE_EN | RTL83XX_FORCE_LINK_EN;
	sw_w32_mask(mask, 0, priv->r->mac_force_mode_ctrl(port));
}

static void rtl93xx_phylink_mac_link_down(struct dsa_switch *ds, int port,
				     unsigned int mode,
				     phy_interface_t interface)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u32 v = 0;

	/* Stop TX/RX to port */
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(port));

	/* No longer force link */
	if (priv->family_id == RTL9300_FAMILY_ID)
		v = RTL930X_FORCE_EN | RTL930X_FORCE_LINK_EN;
	else if (priv->family_id == RTL9310_FAMILY_ID)
		v = RTL931X_FORCE_EN | RTL931X_FORCE_LINK_EN;
	sw_w32_mask(v, 0, priv->r->mac_force_mode_ctrl(port));
}

static void rtl83xx_phylink_mac_link_up(struct dsa_switch *ds, int port,
				   unsigned int mode,
				   phy_interface_t interface,
				   struct phy_device *phydev,
				   int speed, int duplex,
				   bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_to_port(ds, port);
	struct rtl838x_switch_priv *priv = ds->priv;
	u32 mcr, spdsel;

	if (speed == SPEED_1000)
		spdsel = RTL_SPEED_1000;
	else if (speed == SPEED_100)
		spdsel = RTL_SPEED_100;
	else
		spdsel = RTL_SPEED_10;

	mcr = sw_r32(priv->r->mac_force_mode_ctrl(port));

	if (priv->family_id == RTL8380_FAMILY_ID) {
		mcr &= ~RTL838X_RX_PAUSE_EN;
		mcr &= ~RTL838X_TX_PAUSE_EN;
		mcr &= ~RTL838X_DUPLEX_MODE;
		mcr &= ~RTL838X_SPEED_MASK;
		mcr |= RTL83XX_FORCE_LINK_EN;
		mcr |= spdsel << RTL838X_SPEED_SHIFT;

		if (tx_pause)
			mcr |= RTL838X_TX_PAUSE_EN;
		if (rx_pause)
			mcr |= RTL838X_RX_PAUSE_EN;
		if (duplex == DUPLEX_FULL || priv->lagmembers & BIT_ULL(port))
			mcr |= RTL838X_DUPLEX_MODE;
		if (dsa_port_is_cpu(dp))
			mcr |= RTL83XX_FORCE_EN;

	} else if (priv->family_id == RTL8390_FAMILY_ID) {
		mcr &= ~RTL839X_RX_PAUSE_EN;
		mcr &= ~RTL839X_TX_PAUSE_EN;
		mcr &= ~RTL839X_DUPLEX_MODE;
		mcr &= ~RTL839X_SPEED_MASK;
		mcr |= RTL83XX_FORCE_LINK_EN;
		mcr |= spdsel << RTL839X_SPEED_SHIFT;

		if (tx_pause)
			mcr |= RTL839X_TX_PAUSE_EN;
		if (rx_pause)
			mcr |= RTL839X_RX_PAUSE_EN;
		if (duplex == DUPLEX_FULL || priv->lagmembers & BIT_ULL(port))
			mcr |= RTL839X_DUPLEX_MODE;
		if (dsa_port_is_cpu(dp))
			mcr |= RTL83XX_FORCE_EN;
	}

	pr_debug("%s port %d, mode %x, speed %d, duplex %d, txpause %d, rxpause %d: set mcr=%08x\n",
		__func__, port, mode, speed, duplex, tx_pause, rx_pause, mcr);
	sw_w32(mcr, priv->r->mac_force_mode_ctrl(port));

	/* Restart TX/RX to port */
	sw_w32_mask(0, 0x3, priv->r->mac_port_ctrl(port));
}

static void rtl93xx_phylink_mac_link_up(struct dsa_switch *ds, int port,
				   unsigned int mode,
				   phy_interface_t interface,
				   struct phy_device *phydev,
				   int speed, int duplex,
				   bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_to_port(ds, port);
	struct rtl838x_switch_priv *priv = ds->priv;
	u32 mcr, spdsel;

	if (speed == SPEED_10000)
		spdsel = RTL_SPEED_10000;
	else if (speed == SPEED_5000)
		spdsel = RTL_SPEED_5000;
	else if (speed == SPEED_2500)
		spdsel = RTL_SPEED_2500;
	else if (speed == SPEED_1000)
		spdsel = RTL_SPEED_1000;
	else if (speed == SPEED_100)
		spdsel = RTL_SPEED_100;
	else
		spdsel = RTL_SPEED_10;

	mcr = sw_r32(priv->r->mac_force_mode_ctrl(port));

	if (priv->family_id == RTL9300_FAMILY_ID) {
		mcr &= ~RTL930X_RX_PAUSE_EN;
		mcr &= ~RTL930X_TX_PAUSE_EN;
		mcr &= ~RTL930X_DUPLEX_MODE;
		mcr &= ~RTL930X_SPEED_MASK;
		mcr |= RTL930X_FORCE_LINK_EN;
		mcr |= spdsel << RTL930X_SPEED_SHIFT;

		if (tx_pause)
			mcr |= RTL930X_TX_PAUSE_EN;
		if (rx_pause)
			mcr |= RTL930X_RX_PAUSE_EN;
		if (duplex == DUPLEX_FULL || priv->lagmembers & BIT_ULL(port))
			mcr |= RTL930X_DUPLEX_MODE;
		if (dsa_port_is_cpu(dp) || !priv->ports[port].phy_is_integrated)
			mcr |= RTL930X_FORCE_EN;
	}

	pr_debug("%s port %d, mode %x, speed %d, duplex %d, txpause %d, rxpause %d: set mcr=%08x\n",
		__func__, port, mode, speed, duplex, tx_pause, rx_pause, mcr);
	sw_w32(mcr, priv->r->mac_force_mode_ctrl(port));

	/* Restart TX/RX to port */
	sw_w32_mask(0, 0x3, priv->r->mac_port_ctrl(port));
}

static void rtl83xx_get_strings(struct dsa_switch *ds,
				int port, u32 stringset, u8 *data)
{
	if (stringset != ETH_SS_STATS)
		return;

	for (int i = 0; i < ARRAY_SIZE(rtl83xx_mib); i++)
		ethtool_puts(&data, rtl83xx_mib[i].name);
}

static void rtl83xx_get_ethtool_stats(struct dsa_switch *ds, int port,
				      uint64_t *data)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtl83xx_mib_desc *mib;
	u64 h;

	for (int i = 0; i < ARRAY_SIZE(rtl83xx_mib); i++) {
		mib = &rtl83xx_mib[i];

		data[i] = sw_r32(priv->r->stat_port_std_mib + (port << 8) + 252 - mib->offset);
		if (mib->size == 2) {
			h = sw_r32(priv->r->stat_port_std_mib + (port << 8) + 248 - mib->offset);
			data[i] |= h << 32;
		}
	}
}

static int rtl83xx_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
	if (sset != ETH_SS_STATS)
		return 0;

	return ARRAY_SIZE(rtl83xx_mib);
}

static int rtl83xx_mc_group_alloc(struct rtl838x_switch_priv *priv, int port)
{
	int mc_group = find_first_zero_bit(priv->mc_group_bm, MAX_MC_GROUPS - 1);
	u64 portmask;

	if (mc_group >= MAX_MC_GROUPS - 1)
		return -1;

	set_bit(mc_group, priv->mc_group_bm);
	portmask = BIT_ULL(port);
	priv->r->write_mcast_pmask(mc_group, portmask);

	return mc_group;
}

static u64 rtl83xx_mc_group_add_port(struct rtl838x_switch_priv *priv, int mc_group, int port)
{
	u64 portmask = priv->r->read_mcast_pmask(mc_group);

	pr_debug("%s: %d\n", __func__, port);

	portmask |= BIT_ULL(port);
	priv->r->write_mcast_pmask(mc_group, portmask);

	return portmask;
}

static u64 rtl83xx_mc_group_del_port(struct rtl838x_switch_priv *priv, int mc_group, int port)
{
	u64 portmask = priv->r->read_mcast_pmask(mc_group);

	pr_debug("%s: %d\n", __func__, port);

	portmask &= ~BIT_ULL(port);
	priv->r->write_mcast_pmask(mc_group, portmask);
	if (!portmask)
		clear_bit(mc_group, priv->mc_group_bm);

	return portmask;
}

static int rtl83xx_port_enable(struct dsa_switch *ds, int port,
				struct phy_device *phydev)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 v;

	pr_debug("%s: %x %d", __func__, (u32) priv, port);
	priv->ports[port].enable = true;

	/* enable inner tagging on egress, do not keep any tags */
	priv->r->vlan_port_keep_tag_set(port, 0, 1);

	if (dsa_is_cpu_port(ds, port))
		return 0;

	/* add port to switch mask of CPU_PORT */
	priv->r->traffic_enable(priv->cpu_port, port);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	/* add all other ports in the same bridge to switch mask of port */
	v = priv->r->traffic_get(port);
	v |= priv->ports[port].pm;
	priv->r->traffic_set(port, v);

	/* TODO: Figure out if this is necessary */
	if (priv->family_id == RTL9300_FAMILY_ID) {
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_SABLK_CTRL);
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_DABLK_CTRL);
	}

	if (priv->ports[port].sds_num < 0)
		priv->ports[port].sds_num = rtl93xx_get_sds(phydev);

	return 0;
}

static void rtl83xx_port_disable(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 v;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);
	/* you can only disable user ports */
	if (!dsa_is_user_port(ds, port))
		return;

	/* BUG: This does not work on RTL931X */
	/* remove port from switch mask of CPU_PORT */
	priv->r->traffic_disable(priv->cpu_port, port);

	/* remove all other ports in the same bridge from switch mask of port */
	v = priv->r->traffic_get(port);
	v &= ~priv->ports[port].pm;
	priv->r->traffic_set(port, v);

	priv->ports[port].enable = false;
}

static int rtl83xx_set_mac_eee(struct dsa_switch *ds, int port,
			       struct ethtool_eee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (e->eee_enabled && !priv->eee_enabled) {
		pr_info("Globally enabling EEE\n");
		priv->r->init_eee(priv, true);
	}

	priv->r->port_eee_set(priv, port, e->eee_enabled);

	if (e->eee_enabled)
		pr_info("Enabled EEE for port %d\n", port);
	else
		pr_info("Disabled EEE for port %d\n", port);

	return 0;
}

static int rtl83xx_get_mac_eee(struct dsa_switch *ds, int port,
			       struct ethtool_eee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	e->supported = SUPPORTED_100baseT_Full | SUPPORTED_1000baseT_Full;

	priv->r->eee_port_ability(priv, e, port);

	e->eee_enabled = priv->ports[port].eee_enabled;

	e->eee_active = !!(e->advertised & e->lp_advertised);

	return 0;
}

static int rtl93xx_get_mac_eee(struct dsa_switch *ds, int port,
			       struct ethtool_eee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	e->supported = SUPPORTED_100baseT_Full |
	               SUPPORTED_1000baseT_Full |
	               SUPPORTED_2500baseX_Full;

	priv->r->eee_port_ability(priv, e, port);

	e->eee_enabled = priv->ports[port].eee_enabled;

	e->eee_active = !!(e->advertised & e->lp_advertised);

	return 0;
}

static int rtl83xx_set_ageing_time(struct dsa_switch *ds, unsigned int msec)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->set_ageing_time(msec);

	return 0;
}

static int rtl83xx_port_bridge_join(struct dsa_switch *ds, int port,
				    struct dsa_bridge bridge,
				    bool *tx_fwd_offload,
				    struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 port_bitmap = BIT_ULL(priv->cpu_port), v;

	pr_debug("%s %x: %d %llx", __func__, (u32)priv, port, port_bitmap);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < ds->num_ports; i++) {
		/* Add this port to the port matrix of the other ports in the
		 * same bridge. If the port is disabled, port matrix is kept
		 * and not being setup until the port becomes enabled.
		 */
		if (dsa_is_user_port(ds, i) && !priv->is_lagmember[i] && i != port) {
			if (!dsa_port_offloads_bridge(dsa_to_port(ds, i), &bridge))
				continue;
			if (priv->ports[i].enable)
				priv->r->traffic_enable(i, port);

			priv->ports[i].pm |= BIT_ULL(port);
			port_bitmap |= BIT_ULL(i);
		}
	}

	/* Add all other ports to this port matrix. */
	if (priv->ports[port].enable) {
		priv->r->traffic_enable(priv->cpu_port, port);
		v = priv->r->traffic_get(port);
		v |= port_bitmap;
		priv->r->traffic_set(port, v);
	}
	priv->ports[port].pm |= port_bitmap;

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, false);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtl83xx_port_bridge_leave(struct dsa_switch *ds, int port,
				      struct dsa_bridge bridge)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 port_bitmap = 0, v;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);
	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < ds->num_ports; i++) {
		/* Remove this port from the port matrix of the other ports
		 * in the same bridge. If the port is disabled, port matrix
		 * is kept and not being setup until the port becomes enabled.
		 * And the other port's port matrix cannot be broken when the
		 * other port is still a VLAN-aware port.
		 */
		if (dsa_is_user_port(ds, i) && i != port) {
			if (!dsa_port_offloads_bridge(dsa_to_port(ds, i), &bridge))
				continue;
			if (priv->ports[i].enable)
				priv->r->traffic_disable(i, port);

			priv->ports[i].pm &= ~BIT_ULL(port);
			port_bitmap |= BIT_ULL(i);
		}
	}

	/* Remove all other ports from this port matrix. */
	if (priv->ports[port].enable) {
		v = priv->r->traffic_get(port);
		v &= ~port_bitmap;
		priv->r->traffic_set(port, v);
	}
	priv->ports[port].pm &= ~port_bitmap;

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, true);

	mutex_unlock(&priv->reg_mutex);
}

void rtl83xx_port_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
	u32 msti = 0;
	u32 port_state[4];
	int index, bit;
	int pos = port;
	struct rtl838x_switch_priv *priv = ds->priv;
	int n = priv->port_width << 1;

	/* Ports above or equal CPU port can never be configured */
	if (port >= priv->cpu_port)
		return;

	mutex_lock(&priv->reg_mutex);

	/* For the RTL839x and following, the bits are left-aligned, 838x and 930x
	 * have 64 bit fields, 839x and 931x have 128 bit fields
	 */
	if (priv->family_id == RTL8390_FAMILY_ID)
		pos += 12;
	if (priv->family_id == RTL9300_FAMILY_ID)
		pos += 3;
	if (priv->family_id == RTL9310_FAMILY_ID)
		pos += 8;

	index = n - (pos >> 4) - 1;
	bit = (pos << 1) % 32;

	priv->r->stp_get(priv, msti, port_state);

	pr_debug("Current state, port %d: %d\n", port, (port_state[index] >> bit) & 3);
	port_state[index] &= ~(3 << bit);

	switch (state) {
	case BR_STATE_DISABLED: /* 0 */
		port_state[index] |= (0 << bit);
		break;
	case BR_STATE_BLOCKING:  /* 4 */
	case BR_STATE_LISTENING: /* 1 */
		port_state[index] |= (1 << bit);
		break;
	case BR_STATE_LEARNING: /* 2 */
		port_state[index] |= (2 << bit);
		break;
	case BR_STATE_FORWARDING: /* 3 */
		port_state[index] |= (3 << bit);
	default:
		break;
	}

	priv->r->stp_set(priv, msti, port_state);

	mutex_unlock(&priv->reg_mutex);
}

void rtl83xx_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int s = priv->family_id == RTL8390_FAMILY_ID ? 2 : 0;

	pr_debug("FAST AGE port %d\n", port);
	mutex_lock(&priv->reg_mutex);
	/* RTL838X_L2_TBL_FLUSH_CTRL register bits, 839x has 1 bit larger
	 * port fields:
	 * 0-4: Replacing port
	 * 5-9: Flushed/replaced port
	 * 10-21: FVID
	 * 22: Entry types: 1: dynamic, 0: also static
	 * 23: Match flush port
	 * 24: Match FVID
	 * 25: Flush (0) or replace (1) L2 entries
	 * 26: Status of action (1: Start, 0: Done)
	 */
	sw_w32(1 << (26 + s) | 1 << (23 + s) | port << (5 + (s / 2)), priv->r->l2_tbl_flush_ctrl);

	do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & BIT(26 + s));

	mutex_unlock(&priv->reg_mutex);
}

void rtl931x_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_info("%s port %d\n", __func__, port);
	mutex_lock(&priv->reg_mutex);
	sw_w32(port << 11, RTL931X_L2_TBL_FLUSH_CTRL + 4);

	sw_w32(BIT(24) | BIT(28), RTL931X_L2_TBL_FLUSH_CTRL);

	do { } while (sw_r32(RTL931X_L2_TBL_FLUSH_CTRL) & BIT (28));

	mutex_unlock(&priv->reg_mutex);
}

void rtl930x_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (priv->family_id == RTL9310_FAMILY_ID)
		return rtl931x_fast_age(ds, port);

	pr_debug("FAST AGE port %d\n", port);
	mutex_lock(&priv->reg_mutex);
	sw_w32(port << 11, RTL930X_L2_TBL_FLUSH_CTRL + 4);

	sw_w32(BIT(26) | BIT(30), RTL930X_L2_TBL_FLUSH_CTRL);

	do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & BIT(30));

	mutex_unlock(&priv->reg_mutex);
}

static int rtl83xx_vlan_filtering(struct dsa_switch *ds, int port,
				  bool vlan_filtering,
				  struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s: port %d\n", __func__, port);
	mutex_lock(&priv->reg_mutex);

	if (vlan_filtering) {
		/* Enable ingress and egress filtering
		 * The VLAN_PORT_IGR_FILTER register uses 2 bits for each port to define
		 * the filter action:
		 * 0: Always Forward
		 * 1: Drop packet
		 * 2: Trap packet to CPU port
		 * The Egress filter used 1 bit per state (0: DISABLED, 1: ENABLED)
		 */
		if (port != priv->cpu_port) {
			priv->r->set_vlan_igr_filter(port, IGR_DROP);
			priv->r->set_vlan_egr_filter(port, EGR_ENABLE);
		}
		else {
			priv->r->set_vlan_igr_filter(port, IGR_TRAP);
			priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
		}

	} else {
		/* Disable ingress and egress filtering */
		if (port != priv->cpu_port)
			priv->r->set_vlan_igr_filter(port, IGR_FORWARD);

		priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
	}

	/* Do we need to do something to the CPU-Port, too? */
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtl83xx_vlan_prepare(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_vlan *vlan)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->vlan_tables_read(0, &info);

	pr_debug("VLAN 0: Tagged ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		info.tagged_ports, info.untagged_ports, info.profile_id,
		info.hash_mc_fid, info.hash_uc_fid, info.fid);

	priv->r->vlan_tables_read(1, &info);
	pr_debug("VLAN 1: Tagged ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		info.tagged_ports, info.untagged_ports, info.profile_id,
		info.hash_mc_fid, info.hash_uc_fid, info.fid);
	priv->r->vlan_set_untagged(1, info.untagged_ports);
	pr_debug("SET: Untagged ports, VLAN %d: %llx\n", 1, info.untagged_ports);

	priv->r->vlan_set_tagged(1, &info);
	pr_debug("SET: Tagged ports, VLAN %d: %llx\n", 1, info.tagged_ports);

	return 0;
}

static int rtl83xx_vlan_add(struct dsa_switch *ds, int port,
			    const struct switchdev_obj_port_vlan *vlan,
			    struct netlink_ext_ack *extack)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	int err;

	pr_debug("%s port %d, vid %d, flags %x\n",
		__func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid) return 0;

	if (vlan->vid > 4095) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	err = rtl83xx_vlan_prepare(ds, port, vlan);
	if (err)
		return err;

	mutex_lock(&priv->reg_mutex);

	/*
	 * Realtek switches copy frames as-is to/from the CPU. For a proper
	 * VLAN handling the 12 bit RVID field (= VLAN id) for incoming traffic
	 * and the 1 bit RVID_SEL field (0 = use inner tag, 1 = use outer tag)
	 * for outgoing traffic of the CPU tag structure need to be handled. As
	 * of now no such logic is in place. So for the CPU port keep the fixed
	 * PVID=0 from initial setup in place and ignore all subsequent settings.
	 */
	if (port != priv->cpu_port) {
		if (vlan->flags & BRIDGE_VLAN_INFO_PVID)
			rtl83xx_vlan_set_pvid(priv, port, vlan->vid);
		else if (priv->ports[port].pvid == vlan->vid)
			rtl83xx_vlan_set_pvid(priv, port, 0);
	}

	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* new VLAN? */
	if (!info.tagged_ports) {
		info.fid = 0;
		info.hash_mc_fid = false;
		info.hash_uc_fid = false;
		info.profile_id = 0;
	}

	/* sanitize untagged_ports - must be a subset */
	if (info.untagged_ports & ~info.tagged_ports)
		info.untagged_ports = 0;

	info.tagged_ports |= BIT_ULL(port);
	if (vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED)
		info.untagged_ports |= BIT_ULL(port);
	else
		info.untagged_ports &= ~BIT_ULL(port);

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Tagged ports, VLAN %d: %llx\n", vlan->vid, info.tagged_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtl83xx_vlan_del(struct dsa_switch *ds, int port,
			    const struct switchdev_obj_port_vlan *vlan)
{
	struct rtl838x_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	u16 pvid;

	pr_debug("%s: port %d, vid %d, flags %x\n",
		__func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid) return 0;

	if (vlan->vid > 4095) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	mutex_lock(&priv->reg_mutex);
	pvid = priv->ports[port].pvid;

	/* Reset to default if removing the current PVID */
	if (vlan->vid == pvid) {
		rtl83xx_vlan_set_pvid(priv, port, 0);
	}
	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* remove port from both tables */
	info.untagged_ports &= (~BIT_ULL(port));
	info.tagged_ports &= (~BIT_ULL(port));

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Tagged ports, VLAN %d: %llx\n", vlan->vid, info.tagged_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtl83xx_setup_l2_uc_entry(struct rtl838x_l2_entry *e, int port, int vid, u64 mac)
{
	memset(e, 0, sizeof(*e));

	e->type = L2_UNICAST;
	e->valid = true;

	e->age = 3;
	e->is_static = true;

	e->port = port;

	e->rvid = e->vid = vid;
	e->is_ip_mc = e->is_ipv6_mc = false;
	u64_to_ether_addr(mac, e->mac);
}

static void rtl83xx_setup_l2_mc_entry(struct rtl838x_l2_entry *e, int vid, u64 mac, int mc_group)
{
	memset(e, 0, sizeof(*e));

	e->type = L2_MULTICAST;
	e->valid = true;

	e->mc_portmask_index = mc_group;

	e->rvid = e->vid = vid;
	e->is_ip_mc = e->is_ipv6_mc = false;
	u64_to_ether_addr(mac, e->mac);
}

/* Uses the seed to identify a hash bucket in the L2 using the derived hash key and then loops
 * over the entries in the bucket until either a matching entry is found or an empty slot
 * Returns the filled in rtl838x_l2_entry and the index in the bucket when an entry was found
 * when an empty slot was found and must exist is false, the index of the slot is returned
 * when no slots are available returns -1
 */
static int rtl83xx_find_l2_hash_entry(struct rtl838x_switch_priv *priv, u64 seed,
				     bool must_exist, struct rtl838x_l2_entry *e)
{
	int idx = -1;
	u32 key = priv->r->l2_hash_key(priv, seed);
	u64 entry;

	pr_debug("%s: using key %x, for seed %016llx\n", __func__, key, seed);
	/* Loop over all entries in the hash-bucket and over the second block on 93xx SoCs */
	for (int i = 0; i < priv->l2_bucket_size; i++) {
		entry = priv->r->read_l2_entry_using_hash(key, i, e);
		pr_debug("valid %d, mac %016llx\n", e->valid, ether_addr_to_u64(&e->mac[0]));
		if (must_exist && !e->valid)
			continue;
		if (!e->valid || ((entry & 0x0fffffffffffffffULL) == seed)) {
			idx = i > 3 ? ((key >> 14) & 0xffff) | i >> 1 : ((key << 2) | i) & 0xffff;
			break;
		}
	}

	return idx;
}

/* Uses the seed to identify an entry in the CAM by looping over all its entries
 * Returns the filled in rtl838x_l2_entry and the index in the CAM when an entry was found
 * when an empty slot was found the index of the slot is returned
 * when no slots are available returns -1
 */
static int rtl83xx_find_l2_cam_entry(struct rtl838x_switch_priv *priv, u64 seed,
				     bool must_exist, struct rtl838x_l2_entry *e)
{
	int idx = -1;
	u64 entry;

	for (int i = 0; i < 64; i++) {
		entry = priv->r->read_cam(i, e);
		if (!must_exist && !e->valid) {
			if (idx < 0) /* First empty entry? */
				idx = i;
			break;
		} else if ((entry & 0x0fffffffffffffffULL) == seed) {
			pr_debug("Found entry in CAM\n");
			idx = i;
			break;
		}
	}

	return idx;
}

static int rtl83xx_port_fdb_add(struct dsa_switch *ds, int port,
				const unsigned char *addr, u16 vid,
				const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtl83xx_find_l2_hash_entry(priv, seed, false, &e);

	/* Found an existing or empty entry */
	if (idx >= 0) {
		rtl83xx_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Hash buckets full, try CAM */
	idx = rtl83xx_find_l2_cam_entry(priv, seed, false, &e);

	if (idx >= 0) {
		rtl83xx_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_cam(idx, &e);
		goto out;
	}

	err = -ENOTSUPP;

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtl83xx_port_fdb_del(struct dsa_switch *ds, int port,
				const unsigned char *addr, u16 vid,
				const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);

	pr_debug("In %s, mac %llx, vid: %d\n", __func__, mac, vid);
	mutex_lock(&priv->reg_mutex);

	idx = rtl83xx_find_l2_hash_entry(priv, seed, true, &e);

	if (idx >= 0) {
		pr_debug("Found entry index %d, key %d and bucket %d\n", idx, idx >> 2, idx & 3);
		e.valid = false;
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Check CAM for spillover from hash buckets */
	idx = rtl83xx_find_l2_cam_entry(priv, seed, true, &e);

	if (idx >= 0) {
		e.valid = false;
		priv->r->write_cam(idx, &e);
		goto out;
	}
	err = -ENOENT;

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtl83xx_port_fdb_dump(struct dsa_switch *ds, int port,
				 dsa_fdb_dump_cb_t *cb, void *data)
{
	struct rtl838x_l2_entry e;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);

	for (int i = 0; i < priv->fib_entries; i++) {
		priv->r->read_l2_entry_using_hash(i >> 2, i & 0x3, &e);

		if (!e.valid)
			continue;

		if (e.port == port || e.port == RTL930X_PORT_IGNORE)
			cb(e.mac, e.vid, e.is_static, data);

		if (!((i + 1) % 64))
			cond_resched();
	}

	for (int i = 0; i < 64; i++) {
		priv->r->read_cam(i, &e);

		if (!e.valid)
			continue;

		if (e.port == port)
			cb(e.mac, e.vid, e.is_static, data);
	}

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtl83xx_port_mdb_add(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_mdb *mdb,
				const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(mdb->addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	int vid = mdb->vid;
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	int mc_group;

	if (priv->id >= 0x9300)
		return -EOPNOTSUPP;

	pr_debug("In %s port %d, mac %llx, vid: %d\n", __func__, port, mac, vid);

	if (priv->is_lagmember[port]) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return -EINVAL;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtl83xx_find_l2_hash_entry(priv, seed, false, &e);

	/* Found an existing or empty entry */
	if (idx >= 0) {
		if (e.valid) {
			pr_debug("Found an existing entry %016llx, mc_group %d\n",
				ether_addr_to_u64(e.mac), e.mc_portmask_index);
			rtl83xx_mc_group_add_port(priv, e.mc_portmask_index, port);
		} else {
			pr_debug("New entry for seed %016llx\n", seed);
			mc_group = rtl83xx_mc_group_alloc(priv, port);
			if (mc_group < 0) {
				err = -ENOTSUPP;
				goto out;
			}
			rtl83xx_setup_l2_mc_entry(&e, vid, mac, mc_group);
			priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		}
		goto out;
	}

	/* Hash buckets full, try CAM */
	idx = rtl83xx_find_l2_cam_entry(priv, seed, false, &e);

	if (idx >= 0) {
		if (e.valid) {
			pr_debug("Found existing CAM entry %016llx, mc_group %d\n",
				 ether_addr_to_u64(e.mac), e.mc_portmask_index);
			rtl83xx_mc_group_add_port(priv, e.mc_portmask_index, port);
		} else {
			pr_debug("New entry\n");
			mc_group = rtl83xx_mc_group_alloc(priv, port);
			if (mc_group < 0) {
				err = -ENOTSUPP;
				goto out;
			}
			rtl83xx_setup_l2_mc_entry(&e, vid, mac, mc_group);
			priv->r->write_cam(idx, &e);
		}
		goto out;
	}

	err = -ENOTSUPP;

out:
	mutex_unlock(&priv->reg_mutex);
	if (err)
		dev_err(ds->dev, "failed to add MDB entry\n");

	return err;
}

int rtl83xx_port_mdb_del(struct dsa_switch *ds, int port,
			 const struct switchdev_obj_port_mdb *mdb,
			 const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(mdb->addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	int vid = mdb->vid;
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	u64 portmask;

	pr_debug("In %s, port %d, mac %llx, vid: %d\n", __func__, port, mac, vid);

	if (priv->is_lagmember[port]) {
		pr_info("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtl83xx_find_l2_hash_entry(priv, seed, true, &e);

	if (idx >= 0) {
		pr_debug("Found entry index %d, key %d and bucket %d\n", idx, idx >> 2, idx & 3);
		portmask = rtl83xx_mc_group_del_port(priv, e.mc_portmask_index, port);
		if (!portmask) {
			e.valid = false;
			priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		}
		goto out;
	}

	/* Check CAM for spillover from hash buckets */
	idx = rtl83xx_find_l2_cam_entry(priv, seed, true, &e);

	if (idx >= 0) {
		portmask = rtl83xx_mc_group_del_port(priv, e.mc_portmask_index, port);
		if (!portmask) {
			e.valid = false;
			priv->r->write_cam(idx, &e);
		}
		goto out;
	}
	/* TODO: Re-enable with a newer kernel: err = -ENOENT; */

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtl83xx_port_mirror_add(struct dsa_switch *ds, int port,
				   struct dsa_mall_mirror_tc_entry *mirror,
				   bool ingress, struct netlink_ext_ack *extack)
{
	/* We support 4 mirror groups, one destination port per group */
	int group;
	struct rtl838x_switch_priv *priv = ds->priv;
	int ctrl_reg, dpm_reg, spm_reg;

	pr_debug("In %s\n", __func__);

	for (group = 0; group < 4; group++) {
		if (priv->mirror_group_ports[group] == mirror->to_local_port)
			break;
	}
	if (group >= 4) {
		for (group = 0; group < 4; group++) {
			if (priv->mirror_group_ports[group] < 0)
				break;
		}
	}

	if (group >= 4)
		return -ENOSPC;

	ctrl_reg = priv->r->mir_ctrl + group * 4;
	dpm_reg = priv->r->mir_dpm + group * 4 * priv->port_width;
	spm_reg = priv->r->mir_spm + group * 4 * priv->port_width;

	pr_debug("Using group %d\n", group);
	mutex_lock(&priv->reg_mutex);

	if (priv->family_id == RTL8380_FAMILY_ID) {
		/* Enable mirroring to port across VLANs (bit 11) */
		sw_w32(1 << 11 | (mirror->to_local_port << 4) | 1, ctrl_reg);
	} else {
		/* Enable mirroring to destination port */
		sw_w32((mirror->to_local_port << 4) | 1, ctrl_reg);
	}

	if (ingress && (priv->r->get_port_reg_be(spm_reg) & (1ULL << port))) {
		mutex_unlock(&priv->reg_mutex);
		return -EEXIST;
	}
	if ((!ingress) && (priv->r->get_port_reg_be(dpm_reg) & (1ULL << port))) {
		mutex_unlock(&priv->reg_mutex);
		return -EEXIST;
	}

	if (ingress)
		priv->r->mask_port_reg_be(0, 1ULL << port, spm_reg);
	else
		priv->r->mask_port_reg_be(0, 1ULL << port, dpm_reg);

	priv->mirror_group_ports[group] = mirror->to_local_port;
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtl83xx_port_mirror_del(struct dsa_switch *ds, int port,
				    struct dsa_mall_mirror_tc_entry *mirror)
{
	int group = 0;
	struct rtl838x_switch_priv *priv = ds->priv;
	int ctrl_reg, dpm_reg, spm_reg;

	pr_debug("In %s\n", __func__);
	for (group = 0; group < 4; group++) {
		if (priv->mirror_group_ports[group] == mirror->to_local_port)
			break;
	}
	if (group >= 4)
		return;

	ctrl_reg = priv->r->mir_ctrl + group * 4;
	dpm_reg = priv->r->mir_dpm + group * 4 * priv->port_width;
	spm_reg = priv->r->mir_spm + group * 4 * priv->port_width;

	mutex_lock(&priv->reg_mutex);
	if (mirror->ingress) {
		/* Ingress, clear source port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, spm_reg);
	} else {
		/* Egress, clear destination port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, dpm_reg);
	}

	if (!(sw_r32(spm_reg) || sw_r32(dpm_reg))) {
		priv->mirror_group_ports[group] = -1;
		sw_w32(0, ctrl_reg);
	}

	mutex_unlock(&priv->reg_mutex);
}

static int rtl83xx_port_pre_bridge_flags(struct dsa_switch *ds, int port, struct switchdev_brport_flags flags, struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	unsigned long features = 0;
	pr_debug("%s: %d %lX\n", __func__, port, flags.val);
	if (priv->r->enable_learning)
		features |= BR_LEARNING;
	if (priv->r->enable_flood)
		features |= BR_FLOOD;
	if (priv->r->enable_mcast_flood)
		features |= BR_MCAST_FLOOD;
	if (priv->r->enable_bcast_flood)
		features |= BR_BCAST_FLOOD;
	if (flags.mask & ~(features))
		return -EINVAL;

	return 0;
}

static int rtl83xx_port_bridge_flags(struct dsa_switch *ds, int port, struct switchdev_brport_flags flags, struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s: %d %lX\n", __func__, port, flags.val);
	if (priv->r->enable_learning && (flags.mask & BR_LEARNING))
		priv->r->enable_learning(port, !!(flags.val & BR_LEARNING));

	if (priv->r->enable_flood && (flags.mask & BR_FLOOD))
		priv->r->enable_flood(port, !!(flags.val & BR_FLOOD));

	if (priv->r->enable_mcast_flood && (flags.mask & BR_MCAST_FLOOD))
		priv->r->enable_mcast_flood(port, !!(flags.val & BR_MCAST_FLOOD));

	if (priv->r->enable_bcast_flood && (flags.mask & BR_BCAST_FLOOD))
		priv->r->enable_bcast_flood(port, !!(flags.val & BR_BCAST_FLOOD));

	return 0;
}

static bool rtl83xx_lag_can_offload(struct dsa_switch *ds,
				      struct net_device *lag,
				      struct netdev_lag_upper_info *info)
{
	int id;

	id = dsa_lag_id(ds->dst, lag);
	if (id < 0 || id >= ds->num_lag_ids)
		return false;

	if (info->tx_type != NETDEV_LAG_TX_TYPE_HASH) {
		return false;
	}
	if (info->hash_type != NETDEV_LAG_HASH_L2 && info->hash_type != NETDEV_LAG_HASH_L23)
		return false;

	return true;
}

static int rtl83xx_port_lag_change(struct dsa_switch *ds, int port)
{
	pr_debug("%s: %d\n", __func__, port);
	/* Nothing to be done... */

	return 0;
}

static int rtl83xx_port_lag_join(struct dsa_switch *ds,
				  int port,
				  struct dsa_lag lag,
				  struct netdev_lag_upper_info *info,
				  struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int i, err = 0;

	if (!rtl83xx_lag_can_offload(ds, lag.dev, info))
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	for (i = 0; i < priv->n_lags; i++) {
		if ((!priv->lag_devs[i]) || (priv->lag_devs[i] == lag.dev))
			break;
	}
	if (port >= priv->cpu_port) {
		err = -EINVAL;
		goto out;
	}
	pr_info("port_lag_join: group %d, port %d\n",i, port);
	if (!priv->lag_devs[i])
		priv->lag_devs[i] = lag.dev;

	if (priv->lag_primary[i] == -1) {
		priv->lag_primary[i] = port;
	} else
		priv->is_lagmember[port] = 1;

	priv->lagmembers |= (1ULL << port);

	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_add(priv->ds, i, port, info);
	if (err) {
		err = -EINVAL;
		goto out;
	}

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtl83xx_port_lag_leave(struct dsa_switch *ds, int port,
				  struct dsa_lag lag)
{
	int i, group = -1, err;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);
	for (i = 0; i < priv->n_lags; i++) {
		if (priv->lags_port_members[i] & BIT_ULL(port)) {
			group = i;
			break;
		}
	}

	if (group == -1) {
		pr_info("port_lag_leave: port %d is not a member\n", port);
		err = -EINVAL;
		goto out;
	}

	if (port >= priv->cpu_port) {
		err = -EINVAL;
		goto out;
	}
	pr_info("port_lag_del: group %d, port %d\n",group, port);
	priv->lagmembers &=~ (1ULL << port);
	priv->lag_primary[i] = -1;
	priv->is_lagmember[port] = 0;
	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_del(priv->ds, group, port);
	if (err) {
		err = -EINVAL;
		goto out;
	}
	if (!priv->lags_port_members[i])
		priv->lag_devs[i] = NULL;

out:
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

int dsa_phy_read(struct dsa_switch *ds, int phy_addr, int phy_reg)
{
	u32 val;
	u32 offset = 0;
	struct rtl838x_switch_priv *priv = ds->priv;

	if ((phy_addr >= 24) &&
	    (phy_addr <= 27) &&
	    (priv->ports[24].phy == PHY_RTL838X_SDS)) {
		if (phy_addr == 26)
			offset = 0x100;
		val = sw_r32(RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2)) & 0xffff;
		return val;
	}

	read_phy(phy_addr, 0, phy_reg, &val);
	return val;
}

int dsa_phy_write(struct dsa_switch *ds, int phy_addr, int phy_reg, u16 val)
{
	u32 offset = 0;
	struct rtl838x_switch_priv *priv = ds->priv;

	if ((phy_addr >= 24) &&
	    (phy_addr <= 27) &&
	    (priv->ports[24].phy == PHY_RTL838X_SDS)) {
		if (phy_addr == 26)
			offset = 0x100;
		sw_w32(val, RTL838X_SDS4_FIB_REG0 + offset + (phy_reg << 2));
		return 0;
	}
	return write_phy(phy_addr, 0, phy_reg, val);
}

const struct phylink_pcs_ops rtl83xx_pcs_ops = {
	.pcs_an_restart		= rtl83xx_pcs_an_restart,
	.pcs_validate		= rtl83xx_pcs_validate,
	.pcs_get_state		= rtl83xx_pcs_get_state,
	.pcs_config		= rtl83xx_pcs_config,
};

const struct dsa_switch_ops rtl83xx_switch_ops = {
	.get_tag_protocol	= rtl83xx_get_tag_protocol,
	.setup			= rtl83xx_setup,

	.phy_read		= dsa_phy_read,
	.phy_write		= dsa_phy_write,

	.phylink_get_caps	= rtl83xx_phylink_get_caps,
	.phylink_mac_config	= rtl83xx_phylink_mac_config,
	.phylink_mac_link_down	= rtl83xx_phylink_mac_link_down,
	.phylink_mac_link_up	= rtl83xx_phylink_mac_link_up,
	.phylink_mac_select_pcs	= rtl83xx_phylink_mac_select_pcs,

	.get_strings		= rtl83xx_get_strings,
	.get_ethtool_stats	= rtl83xx_get_ethtool_stats,
	.get_sset_count		= rtl83xx_get_sset_count,

	.port_enable		= rtl83xx_port_enable,
	.port_disable		= rtl83xx_port_disable,

	.get_mac_eee		= rtl83xx_get_mac_eee,
	.set_mac_eee		= rtl83xx_set_mac_eee,

	.set_ageing_time	= rtl83xx_set_ageing_time,
	.port_bridge_join	= rtl83xx_port_bridge_join,
	.port_bridge_leave	= rtl83xx_port_bridge_leave,
	.port_stp_state_set	= rtl83xx_port_stp_state_set,
	.port_fast_age		= rtl83xx_fast_age,

	.port_vlan_filtering	= rtl83xx_vlan_filtering,
	.port_vlan_add		= rtl83xx_vlan_add,
	.port_vlan_del		= rtl83xx_vlan_del,

	.port_fdb_add		= rtl83xx_port_fdb_add,
	.port_fdb_del		= rtl83xx_port_fdb_del,
	.port_fdb_dump		= rtl83xx_port_fdb_dump,

	.port_mdb_add		= rtl83xx_port_mdb_add,
	.port_mdb_del		= rtl83xx_port_mdb_del,

	.port_mirror_add	= rtl83xx_port_mirror_add,
	.port_mirror_del	= rtl83xx_port_mirror_del,

	.port_lag_change	= rtl83xx_port_lag_change,
	.port_lag_join		= rtl83xx_port_lag_join,
	.port_lag_leave		= rtl83xx_port_lag_leave,

	.port_pre_bridge_flags	= rtl83xx_port_pre_bridge_flags,
	.port_bridge_flags	= rtl83xx_port_bridge_flags,
};

const struct phylink_pcs_ops rtl93xx_pcs_ops = {
	.pcs_an_restart		= rtl83xx_pcs_an_restart,
	.pcs_validate		= rtl93xx_pcs_validate,
	.pcs_get_state		= rtl93xx_pcs_get_state,
	.pcs_config		= rtl83xx_pcs_config,
};

const struct dsa_switch_ops rtl930x_switch_ops = {
	.get_tag_protocol	= rtl83xx_get_tag_protocol,
	.setup			= rtl93xx_setup,

	.phy_read		= dsa_phy_read,
	.phy_write		= dsa_phy_write,

	.phylink_get_caps	= rtl83xx_phylink_get_caps,
	.phylink_mac_config	= rtl93xx_phylink_mac_config,
	.phylink_mac_link_down	= rtl93xx_phylink_mac_link_down,
	.phylink_mac_link_up	= rtl93xx_phylink_mac_link_up,
	.phylink_mac_select_pcs	= rtl83xx_phylink_mac_select_pcs,

	.get_strings		= rtl83xx_get_strings,
	.get_ethtool_stats	= rtl83xx_get_ethtool_stats,
	.get_sset_count		= rtl83xx_get_sset_count,

	.port_enable		= rtl83xx_port_enable,
	.port_disable		= rtl83xx_port_disable,

	.get_mac_eee		= rtl93xx_get_mac_eee,
	.set_mac_eee		= rtl83xx_set_mac_eee,

	.set_ageing_time	= rtl83xx_set_ageing_time,
	.port_bridge_join	= rtl83xx_port_bridge_join,
	.port_bridge_leave	= rtl83xx_port_bridge_leave,
	.port_stp_state_set	= rtl83xx_port_stp_state_set,
	.port_fast_age		= rtl930x_fast_age,

	.port_vlan_filtering	= rtl83xx_vlan_filtering,
	.port_vlan_add		= rtl83xx_vlan_add,
	.port_vlan_del		= rtl83xx_vlan_del,

	.port_fdb_add		= rtl83xx_port_fdb_add,
	.port_fdb_del		= rtl83xx_port_fdb_del,
	.port_fdb_dump		= rtl83xx_port_fdb_dump,

	.port_mdb_add		= rtl83xx_port_mdb_add,
	.port_mdb_del		= rtl83xx_port_mdb_del,

	.port_lag_change	= rtl83xx_port_lag_change,
	.port_lag_join		= rtl83xx_port_lag_join,
	.port_lag_leave		= rtl83xx_port_lag_leave,

	.port_pre_bridge_flags	= rtl83xx_port_pre_bridge_flags,
	.port_bridge_flags	= rtl83xx_port_bridge_flags,
};
