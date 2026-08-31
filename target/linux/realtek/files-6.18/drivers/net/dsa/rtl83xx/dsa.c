// SPDX-License-Identifier: GPL-2.0-only

#include <net/dsa.h>
#include <linux/etherdevice.h>
#include <linux/if_bridge.h>
#include <linux/pcs/pcs.h>
#include <asm/mach-rtl-otto/mach-rtl-otto.h>

#include "rtl-otto.h"

static const u8 ipv4_ll_mcast_addr_base[ETH_ALEN] = {
	0x01, 0x00, 0x5e, 0x00, 0x00, 0x00
};

static const u8 ipv4_ll_mcast_addr_mask[ETH_ALEN] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};

static const u8 ipv6_all_hosts_mcast_addr_base[ETH_ALEN] = {
	0x33, 0x33, 0x00, 0x00, 0x00, 0x01
};

static const u8 ipv6_all_hosts_mcast_addr_mask[ETH_ALEN] = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static void rtldsa_init_counters(struct rtl838x_switch_priv *priv);
static void rtldsa_port_xstp_state_set(struct rtl838x_switch_priv *priv, int port,
				       u8 state, u16 mst_slot);

static void rtldsa_init_stats(struct rtl838x_switch_priv *priv)
{
	mutex_lock(&priv->reg_mutex);

	if (priv->r->stat_init)
		priv->r->stat_init(priv);

	/* Reset statistics counters */
	sw_w32_mask(0, 1, priv->r->stat_rst);

	mutex_unlock(&priv->reg_mutex);
}

/* DSA callbacks */

static enum dsa_tag_protocol rtldsa_get_tag_protocol(struct dsa_switch *ds,
						     int port,
						     enum dsa_tag_protocol mprot)
{
	/* The switch does not tag the frames, instead internally the header
	 * structure for each packet is tagged accordingly.
	 */
	return DSA_TAG_PROTO_RTL_OTTO;
}

static void rtldsa_vlan_set_pvid(struct rtl838x_switch_priv *priv,
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

static void rtldsa_83xx_mc_pmasks_setup(struct rtl838x_switch_priv *priv)
{
	/* RTL8380 and RTL8390 use an index into the portmask table to set the
	 * unknown multicast portmask, setup a default at a safe location
	 * On RTL93XX, the portmask is directly set in the profile,
	 * see e.g. rtl9300_vlan_profile_setup
	 */
	priv->r->write_mcast_pmask(MC_PMASK_ALL_PORTS_IDX, ~0);
}

/* Initialize all VLANS */
static void rtldsa_vlan_setup(struct rtl838x_switch_priv *priv)
{
	struct rtldsa_vlan_info info = {
		.l2_tunnel_list_id = -1,
	};

	pr_info("In %s\n", __func__);

	priv->r->vlan_profile_setup(0);
	priv->r->vlan_profile_dump(priv, 0);

	/* Initialize normal VLANs 1-4095 */
	for (int i = 1; i < MAX_VLANS; i++)
		priv->r->vlan_set_tagged(i, &info);

	/*
	 * Initialize the special VLAN 0 and reset PVIDs. The CPU port PVID
	 * is applied to packets from the CPU for untagged destinations,
	 * regardless if the actual ingress VID. Any port with untagged
	 * egress VLAN(s) must therefore be a member of VLAN 0 to support
	 * CPU port as ingress when VLAN filtering is enabled.
	 */
	for (int i = 0; i <= priv->r->cpu_port; i++) {
		rtldsa_vlan_set_pvid(priv, i, 0);
		info.member_ports |= BIT_ULL(i);
	}
	priv->r->vlan_set_tagged(0, &info);

	/* Set forwarding action based on inner VLAN tag */
	for (int i = 0; i < priv->r->cpu_port; i++)
		priv->r->vlan_fwd_on_inner(i, true);
}

static void rtldsa_setup_bpdu_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->r->cpu_port; i++)
		priv->r->set_receive_management_action(i, BPDU, TRAP2CPU);
}

static void rtldsa_setup_lldp_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->r->cpu_port; i++)
		priv->r->set_receive_management_action(i, LLDP, TRAP2CPU);
}

static void rtldsa_setup_eapol_traps(struct rtl838x_switch_priv *priv)
{
	for (int i = 0; i < priv->r->cpu_port; i++)
		priv->r->set_receive_management_action(i, EAPOL, TRAP2CPU);
}

static void rtldsa_port_set_salrn(struct rtl838x_switch_priv *priv,
				  int port, bool enable)
{
	int shift = SALRN_PORT_SHIFT(port);
	int val = enable ? SALRN_MODE_HARDWARE : SALRN_MODE_DISABLED;

	sw_w32_mask(SALRN_MODE_MASK << shift, val << shift,
		    priv->r->l2_port_new_salrn(port));
}

static int rtldsa_83xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s called\n", __func__);

	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->r->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->r->cpu_port; i++) {
		if (priv->ports[i].phy || priv->ports[i].has_pcs) {
			priv->ports[i].pm = BIT_ULL(priv->r->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->r->cpu_port, BIT_ULL(priv->r->cpu_port));

	/* For standalone ports, forward packets even if a static fdb
	 * entry for the source address exists on another port.
	 */
	if (priv->r->set_static_move_action) {
		for (int i = 0; i <= priv->r->cpu_port; i++)
			priv->r->set_static_move_action(i, true);
	}

	priv->r->print_matrix();
	rtldsa_init_stats(priv);
	rtldsa_init_counters(priv);

	rtldsa_83xx_mc_pmasks_setup(priv);
	rtldsa_vlan_setup(priv);

	rtldsa_setup_bpdu_traps(priv);
	rtldsa_setup_lldp_traps(priv);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtldsa_port_set_salrn(priv, priv->r->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	/* Make sure all frames sent to the switch's MAC are trapped to the CPU-port
	 *  0: FWD, 1: DROP, 2: TRAP2CPU
	 */
	sw_w32(0x2, priv->r->self_mac_trap_ctrl);

	priv->r->pie_init(priv);

	return 0;
}

static int rtldsa_93xx_setup(struct dsa_switch *ds)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_info("%s called\n", __func__);

	/* Disable all ports except CPU port */
	for (int i = 0; i < ds->num_ports; i++)
		priv->ports[i].enable = false;
	priv->ports[priv->r->cpu_port].enable = true;

	/* Configure ports so they are disabled by default, but once enabled
	 * they will work in isolated mode (only traffic between port and CPU).
	 */
	for (int i = 0; i < priv->r->cpu_port; i++) {
		if (priv->ports[i].phy || priv->ports[i].has_pcs) {
			priv->ports[i].pm = BIT_ULL(priv->r->cpu_port);
			priv->r->traffic_set(i, BIT_ULL(i));
		}
	}
	priv->r->traffic_set(priv->r->cpu_port, BIT_ULL(priv->r->cpu_port));
	priv->r->print_matrix();

	rtldsa_init_stats(priv);
	rtldsa_init_counters(priv);

	rtldsa_vlan_setup(priv);

	rtldsa_setup_bpdu_traps(priv);
	rtldsa_setup_lldp_traps(priv);
	rtldsa_setup_eapol_traps(priv);

	ds->configure_vlan_while_not_filtering = true;

	priv->r->l2_learning_setup();

	rtldsa_port_set_salrn(priv, priv->r->cpu_port, false);
	ds->assisted_learning_on_cpu_port = true;

	priv->r->pie_init(priv);
	priv->r->led_init(priv);

	return 0;
}

static int rtldsa_phylink_fill_available_pcs(struct phylink_config *config,
					     struct phylink_pcs **available_pcs,
					     unsigned int num_available_pcs)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);

	return fwnode_phylink_pcs_parse(of_fwnode_handle(dp->dn),
					available_pcs, &num_available_pcs);
}

static void rtldsa_83xx_phylink_get_caps(struct dsa_switch *ds, int port,
					 struct phylink_config *config)
{
	struct dsa_port *dp = dsa_to_port(ds, port);

	/*
	 * TODO: This needs to take into account the MAC to SERDES mapping and the
	 * specific SoC capabilities. Right now we just assume all RTL83xx ports
	 * support up to 1G standalone and QSGMII as that covers most real-world
	 * use cases.
	 */
	config->mac_capabilities = MAC_ASYM_PAUSE | MAC_SYM_PAUSE | MAC_10 | MAC_100 |
				   MAC_1000FD;

	__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_GMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_INTERNAL, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_SGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_QSGMII, config->supported_interfaces);

	if (!fwnode_phylink_pcs_parse(of_fwnode_handle(dp->dn), NULL,
				      &config->num_available_pcs)) {
		config->fill_available_pcs = rtldsa_phylink_fill_available_pcs;
		__set_bit(PHY_INTERFACE_MODE_SGMII, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_QSGMII, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->pcs_interfaces);
	}
}

static void rtldsa_93xx_phylink_get_caps(struct dsa_switch *ds, int port,
					 struct phylink_config *config)
{
	struct dsa_port *dp = dsa_to_port(ds, port);

	/*
	 * TODO: This needs to take into account the MAC to SERDES mapping and the
	 * specific SoC capabilities. Right now we just assume all RTL93xx ports
	 * support up to 10G standalone and up to USXGMII as that covers most
	 * real-world use cases.
	 */
	config->mac_capabilities = MAC_ASYM_PAUSE | MAC_SYM_PAUSE | MAC_10 | MAC_100 |
				   MAC_1000FD | MAC_2500FD | MAC_5000FD | MAC_10000FD;

	__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_GMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_INTERNAL, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_SGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_QSGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_10GBASER, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_2500BASEX, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_USXGMII, config->supported_interfaces);
	__set_bit(PHY_INTERFACE_MODE_10G_QXGMII, config->supported_interfaces);

	if (!fwnode_phylink_pcs_parse(of_fwnode_handle(dp->dn), NULL,
				      &config->num_available_pcs)) {
		config->fill_available_pcs = rtldsa_phylink_fill_available_pcs;
		__set_bit(PHY_INTERFACE_MODE_SGMII, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_QSGMII, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_1000BASEX, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_2500BASEX, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_USXGMII, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_10GBASER, config->pcs_interfaces);
		__set_bit(PHY_INTERFACE_MODE_10G_QXGMII, config->pcs_interfaces);
	}
}

static void rtldsa_83xx_phylink_mac_config(struct phylink_config *config,
					   unsigned int mode,
					   const struct phylink_link_state *state)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl838x_switch_priv *priv = dp->ds->priv;
	int port = dp->index;
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

		mcr |= RTL838X_NWAY_EN;
	} else {
		mcr &= ~RTL838X_NWAY_EN;
	}
	sw_w32(mcr, priv->r->mac_force_mode_ctrl(port));
}

static void rtldsa_93xx_phylink_mac_config(struct phylink_config *config,
					   unsigned int mode,
					   const struct phylink_link_state *state)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl838x_switch_priv *priv = dp->ds->priv;
	int port = dp->index;

	/* Nothing to be done for the CPU-port */
	if (port == priv->r->cpu_port)
		return;

	/* Disable MAC completely */
	sw_w32(0, priv->r->mac_force_mode_ctrl(port));
}

static void rtldsa_phylink_mac_link_down(struct phylink_config *config,
					 unsigned int mode,
					 phy_interface_t interface)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl838x_switch_priv *priv = dp->ds->priv;
	int port = dp->index;

	/* Stop TX/RX to port */
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(port));

	/* No longer force link */
	sw_w32_mask(priv->r->mac_force_mode_mask, 0,
		    priv->r->mac_force_mode_ctrl(port));
}

static void rtldsa_83xx_phylink_mac_link_up(struct phylink_config *config,
					    struct phy_device *phydev,
					    unsigned int mode,
					    phy_interface_t interface,
					    int speed, int duplex,
					    bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl838x_switch_priv *priv = dp->ds->priv;
	int port = dp->index;
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

static void rtldsa_93xx_phylink_mac_link_up(struct phylink_config *config,
					    struct phy_device *phydev,
					    unsigned int mode,
					    phy_interface_t interface,
					    int speed, int duplex,
					    bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct rtl838x_switch_priv *priv = dp->ds->priv;
	int port = dp->index;
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
		if (dsa_port_is_cpu(dp) || priv->ports[port].phy)
			mcr |= RTL930X_FORCE_EN;
	}

	pr_debug("%s port %d, mode %x, speed %d, duplex %d, txpause %d, rxpause %d: set mcr=%08x\n",
		 __func__, port, mode, speed, duplex, tx_pause, rx_pause, mcr);
	sw_w32(mcr, priv->r->mac_force_mode_ctrl(port));

	/* Restart TX/RX to port */
	sw_w32_mask(0, 0x3, priv->r->mac_port_ctrl(port));
}

static bool rtldsa_read_mib_item(struct rtl838x_switch_priv *priv, int port,
				 const struct rtldsa_mib_item *mib_item,
				 u64 *data)
{
	u32 high1, high2;
	int reg, reg_offset, addr_low;

	switch (mib_item->reg) {
	case MIB_REG_STD:
		reg = priv->r->stat_port_std_mib;
		reg_offset = 256;
		break;
	case MIB_REG_PRV:
		reg = priv->r->stat_port_prv_mib;
		reg_offset = 128;
		break;
	case MIB_TBL_STD:
	case MIB_TBL_PRV:
		if (!priv->r->stat_port_table_read)
			return false;

		*data = priv->r->stat_port_table_read(port, mib_item->size, mib_item->offset,
						      mib_item->reg == MIB_TBL_PRV);

		return true;
	default:
		return false;
	}

	addr_low = reg + (port + 1) * reg_offset - 4 - mib_item->offset;

	if (mib_item->size == 2) {
		high1 = sw_r32(addr_low - 4);
		*data = sw_r32(addr_low);
		high2 = sw_r32(addr_low - 4);
		if (high1 != high2) {
			/* Low must have wrapped and overflowed into high, read again */
			*data = sw_r32(addr_low);
		}
		*data |= (u64)high2 << 32;
	} else {
		*data = sw_r32(addr_low);
	}

	return true;
}

static void rtldsa_update_counter(struct rtl838x_switch_priv *priv, int port,
				  struct rtldsa_counter *counter,
				  const struct rtldsa_mib_item *mib_item)
{
	u64 val;
	u32 val32, diff;

	if (!rtldsa_read_mib_item(priv, port, mib_item, &val))
		return;

	if (mib_item->size == 2) {
		counter->val = val;
	} else {
		val32 = (u32)val;
		diff = val32 - counter->last;
		counter->val += diff;
		counter->last = val32;
	}
}

static void rtldsa_update_link_stat(struct rtnl_link_stats64 *s,
				    const struct rtldsa_counter_state *counters)
{
	s->rx_packets = counters->if_in_ucast_pkts.val +
			counters->if_in_mcast_pkts.val +
			counters->if_in_bcast_pkts.val +
			counters->rx_pkts_over_max_octets.val;

	s->tx_packets = counters->if_out_ucast_pkts.val +
			counters->if_out_mcast_pkts.val +
			counters->if_out_bcast_pkts.val -
			counters->if_out_discards.val;

	/* Subtract FCS for each packet, and pause frames */
	s->rx_bytes = counters->if_in_octets.val -
		      4 * s->rx_packets -
		      64 * counters->rx_pause_frames.val;
	s->tx_bytes = counters->if_out_octets.val -
		      4 * s->tx_packets -
		      64 * counters->tx_pause_frames.val;

	s->collisions = counters->collisions.val;

	s->rx_dropped = counters->drop_events.val;
	s->tx_dropped = counters->if_out_discards.val;

	s->rx_crc_errors = counters->crc_align_errors.val;
	s->rx_errors = s->rx_crc_errors;

	s->tx_aborted_errors = counters->excessive_collisions.val;
	s->tx_window_errors = counters->late_collisions.val;
	s->tx_errors = s->tx_aborted_errors + s->tx_window_errors;
}

static void rtldsa_update_port_counters(struct rtl838x_switch_priv *priv, int port)
{
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;
	const struct rtldsa_mib_desc *mib_desc;
	ktime_t now;

	mib_desc = priv->r->mib_desc;

	/* Prevent unnecessary updates when the user accesses different stats quickly.
	 * This compensates a bit for always updating all stats, even when just a
	 * subset is actually requested.
	 */
	now = ktime_get();
	if (ktime_before(now, ktime_add_ms(counters->last_update, 100)))
		return;
	counters->last_update = now;

	rtldsa_update_counter(priv, port, &counters->symbol_errors,
			      &mib_desc->symbol_errors);

	rtldsa_update_counter(priv, port, &counters->if_in_octets,
			      &mib_desc->if_in_octets);
	rtldsa_update_counter(priv, port, &counters->if_out_octets,
			      &mib_desc->if_out_octets);
	rtldsa_update_counter(priv, port, &counters->if_in_ucast_pkts,
			      &mib_desc->if_in_ucast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_in_mcast_pkts,
			      &mib_desc->if_in_mcast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_in_bcast_pkts,
			      &mib_desc->if_in_bcast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_out_ucast_pkts,
			      &mib_desc->if_out_ucast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_out_mcast_pkts,
			      &mib_desc->if_out_mcast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_out_bcast_pkts,
			      &mib_desc->if_out_bcast_pkts);
	rtldsa_update_counter(priv, port, &counters->if_out_discards,
			      &mib_desc->if_out_discards);
	rtldsa_update_counter(priv, port, &counters->single_collisions,
			      &mib_desc->single_collisions);
	rtldsa_update_counter(priv, port, &counters->multiple_collisions,
			      &mib_desc->multiple_collisions);
	rtldsa_update_counter(priv, port, &counters->deferred_transmissions,
			      &mib_desc->deferred_transmissions);
	rtldsa_update_counter(priv, port, &counters->late_collisions,
			      &mib_desc->late_collisions);
	rtldsa_update_counter(priv, port, &counters->excessive_collisions,
			      &mib_desc->excessive_collisions);
	rtldsa_update_counter(priv, port, &counters->crc_align_errors,
			      &mib_desc->crc_align_errors);
	rtldsa_update_counter(priv, port, &counters->rx_pkts_over_max_octets,
			      &mib_desc->rx_pkts_over_max_octets);

	rtldsa_update_counter(priv, port, &counters->unsupported_opcodes,
			      &mib_desc->unsupported_opcodes);

	rtldsa_update_counter(priv, port, &counters->rx_undersize_pkts,
			      &mib_desc->rx_undersize_pkts);
	rtldsa_update_counter(priv, port, &counters->rx_oversize_pkts,
			      &mib_desc->rx_oversize_pkts);
	rtldsa_update_counter(priv, port, &counters->rx_fragments,
			      &mib_desc->rx_fragments);
	rtldsa_update_counter(priv, port, &counters->rx_jabbers,
			      &mib_desc->rx_jabbers);

	for (int i = 0; i < ARRAY_SIZE(mib_desc->tx_pkts); i++) {
		if (mib_desc->tx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rtldsa_update_counter(priv, port, &counters->tx_pkts[i],
				      &mib_desc->tx_pkts[i]);
	}
	for (int i = 0; i < ARRAY_SIZE(mib_desc->rx_pkts); i++) {
		if (mib_desc->rx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rtldsa_update_counter(priv, port, &counters->rx_pkts[i],
				      &mib_desc->rx_pkts[i]);
	}

	rtldsa_update_counter(priv, port, &counters->drop_events,
			      &mib_desc->drop_events);
	rtldsa_update_counter(priv, port, &counters->collisions,
			      &mib_desc->collisions);

	rtldsa_update_counter(priv, port, &counters->rx_pause_frames,
			      &mib_desc->rx_pause_frames);
	rtldsa_update_counter(priv, port, &counters->tx_pause_frames,
			      &mib_desc->tx_pause_frames);

	/* prepare get_stats64 reply without requiring caller waiting for mutex */
	spin_lock(&counters->link_stat_lock);
	rtldsa_update_link_stat(&counters->link_stat, counters);
	spin_unlock(&counters->link_stat_lock);
}

void rtldsa_counters_lock_register(struct rtl838x_switch_priv *priv, int port)
	__acquires(&priv->ports[port].counters.lock)
{
	spin_lock(&priv->ports[port].counters.lock);
}

void rtldsa_counters_unlock_register(struct rtl838x_switch_priv *priv, int port)
	__releases(&priv->ports[port].counters.lock)
{
	spin_unlock(&priv->ports[port].counters.lock);
}

void rtldsa_counters_lock_table(struct rtl838x_switch_priv *priv, int port __maybe_unused)
	__acquires(&priv->counters_lock)
{
	mutex_lock(&priv->counters_lock);
}

void rtldsa_counters_unlock_table(struct rtl838x_switch_priv *priv, int port __maybe_unused)
	__releases(&priv->ports[port].counters.lock)
{
	mutex_unlock(&priv->counters_lock);
}

static void rtldsa_counters_lock(struct rtl838x_switch_priv *priv, int port)
{
	priv->r->stat_counters_lock(priv, port);
}

static void rtldsa_counters_unlock(struct rtl838x_switch_priv *priv, int port)
{
	priv->r->stat_counters_unlock(priv, port);
}

static void rtldsa_poll_counters(struct work_struct *work)
{
	struct rtl838x_switch_priv *priv = container_of(to_delayed_work(work),
							struct rtl838x_switch_priv,
							counters_work);

	for (int port = 0; port < priv->r->cpu_port; port++) {
		if (!priv->ports[port].phy && !priv->ports[port].has_pcs)
			continue;

		rtldsa_counters_lock(priv, port);
		rtldsa_update_port_counters(priv, port);
		rtldsa_counters_unlock(priv, port);
	}

	queue_delayed_work(priv->wq, &priv->counters_work,
			   priv->r->stat_counter_poll_interval);
}

static void rtldsa_init_counters(struct rtl838x_switch_priv *priv)
{
	struct rtldsa_counter_state *counters;

	for (int port = 0; port < priv->r->cpu_port; port++) {
		if (!priv->ports[port].phy && !priv->ports[port].has_pcs)
			continue;

		counters = &priv->ports[port].counters;

		memset(counters, 0, sizeof(*counters));
		spin_lock_init(&counters->lock);
		spin_lock_init(&counters->link_stat_lock);
	}

	INIT_DELAYED_WORK(&priv->counters_work, rtldsa_poll_counters);
	queue_delayed_work(priv->wq, &priv->counters_work,
			   priv->r->stat_counter_poll_interval);
}

static void rtldsa_get_strings(struct dsa_switch *ds,
			       int port, u32 stringset, u8 *data)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtldsa_mib_desc *mib_desc;

	if (stringset != ETH_SS_STATS)
		return;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	mib_desc = priv->r->mib_desc;

	for (int i = 0; i < mib_desc->list_count; i++)
		ethtool_puts(&data, mib_desc->list[i].name);
}

static void rtldsa_get_ethtool_stats(struct dsa_switch *ds, int port,
				     u64 *data)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtldsa_mib_desc *mib_desc;
	const struct rtldsa_mib_item *mib_item;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	mib_desc = priv->r->mib_desc;
	for (int i = 0; i < mib_desc->list_count; i++) {
		mib_item = &mib_desc->list[i].item;
		rtldsa_read_mib_item(priv, port, mib_item, &data[i]);
	}
}

static int rtldsa_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (sset != ETH_SS_STATS)
		return 0;

	if (port < 0 || port >= priv->r->cpu_port)
		return 0;

	return priv->r->mib_desc->list_count;
}

static void rtldsa_get_eth_phy_stats(struct dsa_switch *ds, int port,
				     struct ethtool_eth_phy_stats *phy_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	rtldsa_counters_lock(priv, port);

	rtldsa_update_port_counters(priv, port);

	phy_stats->SymbolErrorDuringCarrier = counters->symbol_errors.val;

	rtldsa_counters_unlock(priv, port);
}

static void rtldsa_get_eth_mac_stats(struct dsa_switch *ds, int port,
				     struct ethtool_eth_mac_stats *mac_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	rtldsa_counters_lock(priv, port);

	rtldsa_update_port_counters(priv, port);

	/* Frame and octet counters are calculated based on RFC3635, while also
	 * taking into account that the behaviour of the hardware counters differs
	 * in some places.
	 */

	mac_stats->FramesReceivedOK = counters->if_in_ucast_pkts.val +
				      counters->if_in_mcast_pkts.val +
				      counters->if_in_bcast_pkts.val +
				      counters->rx_pause_frames.val +
				      counters->rx_pkts_over_max_octets.val;

	mac_stats->FramesTransmittedOK = counters->if_out_ucast_pkts.val +
					 counters->if_out_mcast_pkts.val +
					 counters->if_out_bcast_pkts.val +
					 counters->tx_pause_frames.val -
					 counters->if_out_discards.val;

	mac_stats->OctetsReceivedOK = counters->if_in_octets.val -
				      18 * mac_stats->FramesReceivedOK;
	mac_stats->OctetsTransmittedOK = counters->if_out_octets.val -
					 18 * mac_stats->FramesTransmittedOK;

	mac_stats->SingleCollisionFrames = counters->single_collisions.val;
	mac_stats->MultipleCollisionFrames = counters->multiple_collisions.val;
	mac_stats->FramesWithDeferredXmissions = counters->deferred_transmissions.val;
	mac_stats->LateCollisions = counters->late_collisions.val;
	mac_stats->FramesAbortedDueToXSColls = counters->excessive_collisions.val;

	mac_stats->FrameCheckSequenceErrors = counters->crc_align_errors.val;

	rtldsa_counters_unlock(priv, port);
}

static void rtldsa_get_eth_ctrl_stats(struct dsa_switch *ds, int port,
				      struct ethtool_eth_ctrl_stats *ctrl_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	rtldsa_counters_lock(priv, port);

	rtldsa_update_port_counters(priv, port);

	ctrl_stats->UnsupportedOpcodesReceived = counters->unsupported_opcodes.val;

	rtldsa_counters_unlock(priv, port);
}

static void rtldsa_get_rmon_stats(struct dsa_switch *ds, int port,
				  struct ethtool_rmon_stats *rmon_stats,
				  const struct ethtool_rmon_hist_range **ranges)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	const struct rtldsa_mib_desc *mib_desc;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	mib_desc = priv->r->mib_desc;

	rtldsa_counters_lock(priv, port);

	rtldsa_update_port_counters(priv, port);

	rmon_stats->undersize_pkts = counters->rx_undersize_pkts.val;
	rmon_stats->oversize_pkts = counters->rx_oversize_pkts.val;
	rmon_stats->fragments = counters->rx_fragments.val;
	rmon_stats->jabbers = counters->rx_jabbers.val;

	for (int i = 0; i < ARRAY_SIZE(mib_desc->rx_pkts); i++) {
		if (mib_desc->rx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rmon_stats->hist[i] = counters->rx_pkts[i].val;
	}

	for (int i = 0; i < ARRAY_SIZE(mib_desc->tx_pkts); i++) {
		if (mib_desc->tx_pkts[i].reg == MIB_REG_INVALID)
			break;

		rmon_stats->hist_tx[i] = counters->tx_pkts[i].val;
	}

	*ranges = mib_desc->rmon_ranges;

	rtldsa_counters_unlock(priv, port);
}

void rtldsa_update_counters_atomically(struct rtl838x_switch_priv *priv, int port)
{
	rtldsa_counters_lock(priv, port);
	rtldsa_update_port_counters(priv, port);
	rtldsa_counters_unlock(priv, port);
}

static void rtldsa_get_stats64(struct dsa_switch *ds, int port,
			       struct rtnl_link_stats64 *s)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	if (priv->r->stat_update_counters_atomically)
		priv->r->stat_update_counters_atomically(priv, port);

	/* retrieve prepared return data without potentially sleeping via mutex */
	spin_lock(&counters->link_stat_lock);
	memcpy(s, &counters->link_stat, sizeof(*s));
	spin_unlock(&counters->link_stat_lock);
}

static void rtldsa_get_pause_stats(struct dsa_switch *ds, int port,
				   struct ethtool_pause_stats *pause_stats)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_counter_state *counters = &priv->ports[port].counters;

	if (port < 0 || port >= priv->r->cpu_port)
		return;

	rtldsa_counters_lock(priv, port);

	rtldsa_update_port_counters(priv, port);

	pause_stats->tx_pause_frames = counters->tx_pause_frames.val;
	pause_stats->rx_pause_frames = counters->rx_pause_frames.val;

	rtldsa_counters_unlock(priv, port);
}

static int rtldsa_mc_group_alloc(struct rtl838x_switch_priv *priv, int port)
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

static u64 rtldsa_mc_group_add_port(struct rtl838x_switch_priv *priv, int mc_group, int port)
{
	u64 portmask = priv->r->read_mcast_pmask(mc_group);

	pr_debug("%s: %d\n", __func__, port);

	portmask |= BIT_ULL(port);
	priv->r->write_mcast_pmask(mc_group, portmask);

	return portmask;
}

static u64 rtldsa_mc_group_del_port(struct rtl838x_switch_priv *priv, int mc_group, int port)
{
	u64 portmask = priv->r->read_mcast_pmask(mc_group);

	pr_debug("%s: %d\n", __func__, port);

	portmask &= ~BIT_ULL(port);
	priv->r->write_mcast_pmask(mc_group, portmask);
	if (!portmask)
		clear_bit(mc_group, priv->mc_group_bm);

	return portmask;
}

static int rtldsa_port_enable(struct dsa_switch *ds, int port, struct phy_device *phydev)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s: %x %d", __func__, (u32)priv, port);
	priv->ports[port].enable = true;

	/* enable inner tagging on egress, do not keep any tags */
	priv->r->vlan_port_keep_tag_set(port, 0, 1);

	if (dsa_is_cpu_port(ds, port))
		return 0;

	/* add port to switch mask of CPU_PORT */
	priv->r->traffic_enable(priv->r->cpu_port, port);

	/* add all other ports in the same bridge to switch mask of port */
	priv->r->traffic_set(port, priv->ports[port].pm);

	/* TODO: Figure out if this is necessary */
	if (priv->family_id == RTL9300_FAMILY_ID) {
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_SABLK_CTRL);
		sw_w32_mask(0, BIT(port), RTL930X_L2_PORT_DABLK_CTRL);
	}

	return 0;
}

static void rtldsa_port_disable(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);
	/* you can only disable user ports */
	if (!dsa_is_user_port(ds, port))
		return;

	/* BUG: This does not work on RTL931X */
	/* remove port from switch mask of CPU_PORT */
	priv->r->traffic_disable(priv->r->cpu_port, port);

	/* remove all other ports from switch mask of port */
	priv->r->traffic_set(port, 0);

	priv->ports[port].enable = false;
}

static bool rtldsa_support_eee(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	/* TODO: do this better */
	return (priv->ports[port].phy != 0);
}

static int rtldsa_set_mac_eee(struct dsa_switch *ds, int port, struct ethtool_keee *e)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (e->eee_enabled && !priv->eee_enabled) {
		pr_info("Globally enabling EEE\n");
		priv->r->init_eee(priv, true);
	}

	priv->r->set_mac_eee(priv, port, e->eee_enabled);

	if (e->eee_enabled)
		pr_info("Enabled EEE for port %d\n", port);
	else
		pr_info("Disabled EEE for port %d\n", port);

	return 0;
}

static int rtldsa_set_ageing_time(struct dsa_switch *ds, unsigned int msec)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->set_ageing_time(msec);

	return 0;
}

/**
 * rtldsa_mst_init() - Initialize newly "allocated" MST HW slot
 * @priv: private data of rtldsa switch
 * @mst_slot: MST slot of MSTI
 */
static void rtldsa_mst_init(struct rtl838x_switch_priv *priv, u16 mst_slot)
			    __must_hold(&priv->reg_mutex)
{
	struct dsa_port *dp;
	unsigned int port;
	u8 state;

	dsa_switch_for_each_user_port(dp, priv->ds) {
		if (dp->bridge)
			state = BR_STATE_DISABLED;
		else
			state = dp->stp_state;

		port = dp->index;

		rtldsa_port_xstp_state_set(priv, port, state, mst_slot);
	}
}

/**
 * rtldsa_mst_find() - Find HW MST slot for MSTI (without reference counting)
 * @priv: private data of rtldsa switch
 * @msti: MSTI to search
 *
 * Return: found HW slot (unmodified reference count) or negative encoded error value
 */
static int rtldsa_mst_find(struct rtl838x_switch_priv *priv, u16 msti)
			   __must_hold(&priv->reg_mutex)
{
	unsigned int i;

	/* CIST is always mapped to 0 */
	if (msti == 0)
		return 0;

	if (msti > 4095)
		return -EINVAL;

	/* search for existing entry */
	for (i = 0; i < priv->r->n_mst - 1; i++) {
		if (priv->msts[i].msti != msti)
			continue;

		return i + 1;
	}

	return -ENOENT;
}

/**
 * rtldsa_mst_get() - Get (or allocate) HW MST slot for MSTI
 * @priv: private data of rtldsa switch
 * @msti: MSTI for which a HW slot is needed
 *
 * Return: allocated slot (with increased reference count) or negative encoded error value
 */
static int rtldsa_mst_get(struct rtl838x_switch_priv *priv, u16 msti)
			  __must_hold(&priv->reg_mutex)
{
	unsigned int i;
	int ret;

	ret = rtldsa_mst_find(priv, msti);

	/* CIST doesn't need reference counting */
	if (ret == 0)
		return ret;

	/* valid HW slot was found - refcount needs to be adjusted */
	if (ret > 0) {
		u16 index = ret - 1;

		kref_get(&priv->msts[index].refcount);
		return ret;
	}

	/* any error except "no entry found" cannot be handled */
	if (ret != -ENOENT)
		return ret;

	/* search for free slot */
	for (i = 0; i < priv->r->n_mst - 1; i++) {
		if (priv->msts[i].msti != 0)
			continue;

		kref_init(&priv->msts[i].refcount);
		priv->msts[i].msti = msti;

		rtldsa_mst_init(priv, i + 1);
		return i + 1;
	}

	return -ENOSPC;
}

/**
 * rtldsa_mst_recycle_slot() - Try to recycle old MST slot in case of -ENOSPC of rtldsa_mst_get()
 * @priv: private data of rtldsa switch
 * @msti: MSTI for which a HW slot is needed
 * @old_mst_slot: old mst slot which will be released "soon"
 *
 * If a VLAN should be moved from one MSTI to another one, it is possible that there are currently
 * not enough slots still available to perform a get+put operation. But if this slot is used
 * by a single VLAN anyway, it is not needed to really allocate a new slow - reassigning it to
 * the new MSTI is good enough.
 *
 * This is only allowed when holding the reg_mutex over both calls rtldsa_mst_get() and
 * rtldsa_mst_recycle(). After a rtldsa_mst_recycle() call, rtldsa_mst_put_slot() must no longer
 * be called for @old_mst_slot.
 *
 * Return: allocated slot (with increased reference count) or negative encoded error value
 */
static int rtldsa_mst_recycle_slot(struct rtl838x_switch_priv *priv, u16 msti, u16 old_mst_slot)
				   __must_hold(&priv->reg_mutex)
{
	u16 index;

	/* CIST is always mapped to 0 */
	if (msti == 0)
		return 0;

	if (old_mst_slot == 0)
		return -ENOSPC;

	if (msti > 4095)
		return -EINVAL;

	if (old_mst_slot >= priv->r->n_mst)
		return -EINVAL;

	index = old_mst_slot - 1;

	/* this slot is unused - should not happen because rtldsa_mst_get() searches for it */
	if (priv->msts[index].msti == 0)
		return -EINVAL;

	/* it is only allowed to swap when no other VLAN is using this MST slot */
	if (kref_read(&priv->msts[index].refcount) != 1)
		return -ENOSPC;

	priv->msts[index].msti = msti;
	return old_mst_slot;
}

static void rtldsa_mst_release_slot(struct kref *ref)
{
	struct rtldsa_mst *slot = container_of(ref, struct rtldsa_mst, refcount);

	slot->msti = 0;
}

/**
 * rtldsa_mst_put_slot() - Decrement VLAN use counter for MST slot
 * @priv: private data of rtldsa switch
 * @mst_slot: MST slot which should be put
 *
 * Return: false when MST slot reference counter was only decreased or an invalid @mst_slot was
 * given, true when @mst_slot is now unused
 */
static bool rtldsa_mst_put_slot(struct rtl838x_switch_priv *priv, u16 mst_slot)
				__must_hold(&priv->reg_mutex)
{
	unsigned int index;

	/* CIST is always mapped to 0 and cannot be put */
	if (mst_slot == 0)
		return 0;

	if (mst_slot >= priv->r->n_mst)
		return 0;

	index = mst_slot - 1;

	/* this slot is unused and must not release a reference */
	if (priv->msts[index].msti == 0)
		return 0;

	return kref_put(&priv->msts[index].refcount, rtldsa_mst_release_slot);
}

/**
 * rtldsa_mst_replace() - Get HW slot for @msti and drop old HW slot
 * @priv: private data of rtldsa switch
 * @msti: MSTI for which a HW slot is needed
 * @old_mst_slot: old mst slot which will no longer be assigned to VLAN
 *
 * Return: allocated slot (with increased reference count) or negative encoded error value
 */
static int rtldsa_mst_replace(struct rtl838x_switch_priv *priv, u16 msti, u16 old_mst_slot)
			      __must_hold(&priv->reg_mutex)
{
	int mst_slot_new;

	mst_slot_new = rtldsa_mst_get(priv, msti);
	if (mst_slot_new == -ENOSPC)
		return rtldsa_mst_recycle_slot(priv, msti, old_mst_slot);

	/* directly return errors and don't free old slot */
	if (mst_slot_new < 0)
		return mst_slot_new;

	rtldsa_mst_put_slot(priv, old_mst_slot);

	return mst_slot_new;
}

static void rtldsa_update_port_member(struct rtl838x_switch_priv *priv, int port,
				      const struct net_device *bridge_dev, bool join)
				      __must_hold(&priv->reg_mutex)
{
	struct dsa_port *dp = dsa_to_port(priv->ds, port);
	struct rtldsa_port *p = &priv->ports[port];
	struct dsa_port *cpu_dp = dp->cpu_dp;
	u64 port_mask = BIT_ULL(cpu_dp->index);
	struct rtldsa_port *other_p;
	struct dsa_port *other_dp;
	int other_port;
	bool isolated;

	dsa_switch_for_each_user_port(other_dp, priv->ds) {
		other_port = other_dp->index;
		other_p = &priv->ports[other_port];

		if (dp == other_dp)
			continue;

		if (!dsa_port_offloads_bridge_dev(other_dp, bridge_dev))
			continue;

		isolated = p->isolated && other_p->isolated;

		if (join && !isolated) {
			port_mask |= BIT_ULL(other_port);
			other_p->pm |= BIT_ULL(port);
		} else {
			other_p->pm &= ~BIT_ULL(port);
		}

		if (other_p->enable)
			priv->r->traffic_set(other_port, other_p->pm);
	}

	p->pm = port_mask;

	if (p->enable)
		priv->r->traffic_set(port, port_mask);
}

static int rtldsa_port_bridge_join(struct dsa_switch *ds, int port, struct dsa_bridge bridge,
				   bool *tx_fwd_offload, struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	unsigned int i;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);

	/* reset to default flags for new net_bridge_port */
	priv->ports[port].isolated = false;
	priv->ports[port].cached_flags = 0;

	mutex_lock(&priv->reg_mutex);

	rtldsa_update_port_member(priv, port, bridge.dev, true);

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, false);

	/* Set to disabled in all MSTs, common code will take care of CIST */
	for (i = 1; i < priv->r->n_mst; i++)
		rtldsa_port_xstp_state_set(priv, port, BR_STATE_DISABLED, i);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static void rtldsa_port_bridge_leave(struct dsa_switch *ds, int port, struct dsa_bridge bridge)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	unsigned int i;

	pr_debug("%s %x: %d", __func__, (u32)priv, port);

	mutex_lock(&priv->reg_mutex);

	rtldsa_update_port_member(priv, port, bridge.dev, false);

	if (priv->r->set_static_move_action)
		priv->r->set_static_move_action(port, true);

	/* Set to forwarding in all MSTs, common code will take care of CIST */
	for (i = 1; i < priv->r->n_mst; i++)
		rtldsa_port_xstp_state_set(priv, port, BR_STATE_FORWARDING, i);

	mutex_unlock(&priv->reg_mutex);
}

static void rtldsa_port_xstp_state_set(struct rtl838x_switch_priv *priv, int port,
				       u8 state, u16 mst_slot)
				       __must_hold(&priv->reg_mutex)
{
	int hw_state;

	if (port >= priv->r->cpu_port)
		return;

	switch (state) {
	case BR_STATE_DISABLED:
		hw_state = 0;
		break;
	case BR_STATE_BLOCKING:
	case BR_STATE_LISTENING:
		hw_state = 1;
		break;
	case BR_STATE_LEARNING:
		hw_state = 2;
		break;
	case BR_STATE_FORWARDING:
		hw_state = 3;
		break;
	default:
		dev_err(priv->dev, "stp state %d not supported\n", state);
		return;
	}

	priv->r->stp_set(priv, mst_slot, port, hw_state);
}

void rtldsa_port_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct dsa_port *dp = dsa_to_port(ds, port);
	unsigned int i;

	mutex_lock(&priv->reg_mutex);
	rtldsa_port_xstp_state_set(priv, port, state, 0);

	if (dp->bridge)
		goto unlock;

	/* for unbridged ports, also force the same state to the MSTIs */
	for (i = 1; i < priv->r->n_mst; i++)
		rtldsa_port_xstp_state_set(priv, port, state, i);

unlock:
	mutex_unlock(&priv->reg_mutex);
}

static int rtldsa_port_mst_state_set(struct dsa_switch *ds, int port,
				     const struct switchdev_mst_state *st)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int mst_slot;

	mutex_lock(&priv->reg_mutex);

	mst_slot = rtldsa_mst_find(priv, st->msti);
	if (mst_slot < 0) {
		mutex_unlock(&priv->reg_mutex);
		return mst_slot;
	}

	rtldsa_port_xstp_state_set(priv, port, st->state, mst_slot);
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_vlan_filtering(struct dsa_switch *ds, int port,
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
		if (port != priv->r->cpu_port) {
			priv->r->set_vlan_igr_filter(port, IGR_DROP);
			priv->r->set_vlan_egr_filter(port, EGR_ENABLE);
		} else {
			priv->r->set_vlan_igr_filter(port, IGR_TRAP);
			priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
		}

	} else {
		/* Disable ingress and egress filtering */
		if (port != priv->r->cpu_port)
			priv->r->set_vlan_igr_filter(port, IGR_FORWARD);

		priv->r->set_vlan_egr_filter(port, EGR_DISABLE);
	}

	/* Do we need to do something to the CPU-Port, too? */
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_vlan_prepare(struct dsa_switch *ds, int port,
			       const struct switchdev_obj_port_vlan *vlan)
{
	struct rtldsa_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;

	priv->r->vlan_tables_read(0, &info);

	pr_debug("VLAN 0: Member ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		 info.member_ports, info.untagged_ports, info.profile_id,
		 info.hash_mc_fid, info.hash_uc_fid, info.fid);

	priv->r->vlan_tables_read(1, &info);
	pr_debug("VLAN 1: Member ports %llx, untag %llx, profile %d, MC# %d, UC# %d, FID %x\n",
		 info.member_ports, info.untagged_ports, info.profile_id,
		 info.hash_mc_fid, info.hash_uc_fid, info.fid);
	priv->r->vlan_set_untagged(1, info.untagged_ports);
	pr_debug("SET: Untagged ports, VLAN %d: %llx\n", 1, info.untagged_ports);

	priv->r->vlan_set_tagged(1, &info);
	pr_debug("SET: Member ports, VLAN %d: %llx\n", 1, info.member_ports);

	return 0;
}

static int rtldsa_vlan_add(struct dsa_switch *ds, int port,
			   const struct switchdev_obj_port_vlan *vlan,
			   struct netlink_ext_ack *extack)
{
	struct rtldsa_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	int err;

	pr_debug("%s port %d, vid %d, flags %x\n",
		 __func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid)
		return 0;

	if (vlan->vid >= MAX_VLANS) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	err = rtldsa_vlan_prepare(ds, port, vlan);
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
	if (port != priv->r->cpu_port) {
		if (vlan->flags & BRIDGE_VLAN_INFO_PVID)
			rtldsa_vlan_set_pvid(priv, port, vlan->vid);
		else if (priv->ports[port].pvid == vlan->vid)
			rtldsa_vlan_set_pvid(priv, port, 0);
	}

	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* new VLAN? */
	if (!info.member_ports) {
		info.fid = 0;
		info.hash_mc_fid = false;
		info.hash_uc_fid = false;
		info.profile_id = 0;
	}

	/* sanitize untagged_ports - must be a subset */
	if (info.untagged_ports & ~info.member_ports)
		info.untagged_ports = 0;

	info.member_ports |= BIT_ULL(port);
	if (vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED)
		info.untagged_ports |= BIT_ULL(port);
	else
		info.untagged_ports &= ~BIT_ULL(port);

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Member ports, VLAN %d: %llx\n", vlan->vid, info.member_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtldsa_vlan_del(struct dsa_switch *ds, int port,
			   const struct switchdev_obj_port_vlan *vlan)
{
	struct rtldsa_vlan_info info;
	struct rtl838x_switch_priv *priv = ds->priv;
	u16 pvid;

	pr_debug("%s: port %d, vid %d, flags %x\n",
		 __func__, port, vlan->vid, vlan->flags);

	/* Let no one mess with our special VLAN 0 */
	if (!vlan->vid)
		return 0;

	if (vlan->vid >= MAX_VLANS) {
		dev_err(priv->dev, "VLAN out of range: %d", vlan->vid);
		return -ENOTSUPP;
	}

	mutex_lock(&priv->reg_mutex);
	pvid = priv->ports[port].pvid;

	/* Reset to default if removing the current PVID */
	if (vlan->vid == pvid)
		rtldsa_vlan_set_pvid(priv, port, 0);

	/* Get port memberships of this vlan */
	priv->r->vlan_tables_read(vlan->vid, &info);

	/* remove port from both tables */
	info.untagged_ports &= (~BIT_ULL(port));
	info.member_ports &= (~BIT_ULL(port));

	/* VLANs without members are set back (implicitly) to CIST by DSA */
	if (!info.member_ports) {
		u16 mst = info.fid;

		info.fid = 0;

		rtldsa_mst_put_slot(priv, mst);
	}

	priv->r->vlan_set_untagged(vlan->vid, info.untagged_ports);
	pr_debug("Untagged ports, VLAN %d: %llx\n", vlan->vid, info.untagged_ports);

	priv->r->vlan_set_tagged(vlan->vid, &info);
	pr_debug("Member ports, VLAN %d: %llx\n", vlan->vid, info.member_ports);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

void rtldsa_port_fast_age(struct dsa_switch *ds, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (!priv->r->fast_age)
		return;

	mutex_lock(&priv->reg_mutex);
	priv->r->fast_age(priv, port, -1);
	mutex_unlock(&priv->reg_mutex);
}

static int rtldsa_port_vlan_fast_age(struct dsa_switch *ds, int port, u16 vid)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int ret;

	if (!priv->r->fast_age)
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);
	ret = priv->r->fast_age(priv, port, vid);
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int rtldsa_vlan_msti_set(struct dsa_switch *ds, struct dsa_bridge bridge,
				const struct switchdev_vlan_msti *msti)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_vlan_info info;
	u16 mst_slot_old;
	int mst_slot;

	priv->r->vlan_tables_read(msti->vid, &info);
	mst_slot_old = info.fid;

	/* find HW slot for MSTI */
	mutex_lock(&priv->reg_mutex);
	mst_slot = rtldsa_mst_replace(priv, msti->msti, mst_slot_old);
	mutex_unlock(&priv->reg_mutex);

	if (mst_slot < 0)
		return mst_slot;

	info.fid = mst_slot;
	priv->r->vlan_set_tagged(msti->vid, &info);

	return 0;
}

static void rtldsa_setup_l2_uc_entry(struct rtl838x_l2_entry *e, int port,
				     int vid, u64 mac)
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

static void rtldsa_setup_l2_mc_entry(struct rtl838x_l2_entry *e, int vid, u64 mac, int mc_group)
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
static int rtldsa_find_l2_hash_entry(struct rtl838x_switch_priv *priv, u64 seed,
				     bool must_exist, struct rtl838x_l2_entry *e)
{
	int idx = -1;
	u32 key = priv->r->l2_hash_key(priv, seed);
	u64 entry;

	pr_debug("%s: using key %x, for seed %016llx\n", __func__, key, seed);
	/* Loop over all entries in the hash-bucket and over the second block on 93xx SoCs */
	for (int i = 0; i < priv->r->l2_bucket_size; i++) {
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
static int rtldsa_find_l2_cam_entry(struct rtl838x_switch_priv *priv, u64 seed,
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

/**
 * rtldsa_find_lag_group_from_port() - Find lag group of current port
 * @priv: private data of rtldsa switch
 * @port: port id of potential LAG member
 * Return: -ENOENT when port does not belong to any lag group, lag id otherwise
 */
static int rtldsa_find_lag_group_from_port(struct rtl838x_switch_priv *priv, int port)
{
	if (!(priv->lagmembers & BIT_ULL(port)))
		return -ENOENT;

	/* port is a lag member */
	for (int lag_group = 0; lag_group < MAX_LAGS; lag_group++) {
		if (priv->lags_port_members[lag_group] & BIT_ULL(port))
			return lag_group;
	}

	return -ENOENT;
}

/**
 * rtldsa_93xx_prepare_lag_fdb() - Prepare fdb entry for LAG
 * @e: L2 entry data
 * @lag_group: lag id of the trunk group
 */
inline void rtldsa_93xx_prepare_lag_fdb(struct rtl838x_l2_entry *e, int lag_group)
{
	if (e && lag_group >= 0) {
		e->is_trunk = true;
		e->trunk = lag_group;
	}
}

static int rtldsa_port_fdb_add(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	int lag_group = rtldsa_find_lag_group_from_port(priv, port);

	if (lag_group >= 0 && priv->r->prepare_lag_fdb) {
		priv->r->prepare_lag_fdb(&e, lag_group);
	} else {
		if (priv->lag_non_primary & BIT_ULL(port)) {
			pr_debug("%s: %d is lag slave but prepare_lag_fdb is not supported. ignore\n",
				 __func__, port);
			return 0;
		}
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, false, &e);

	/* Found an existing or empty entry */
	if (idx >= 0) {
		rtldsa_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Hash buckets full, try CAM */
	idx = rtldsa_find_l2_cam_entry(priv, seed, false, &e);

	if (idx >= 0) {
		rtldsa_setup_l2_uc_entry(&e, port, vid, mac);
		priv->r->write_cam(idx, &e);
		goto out;
	}

	err = -ENOTSUPP;

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_port_fdb_del(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       const struct dsa_db db)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	u64 mac = ether_addr_to_u64(addr);
	struct rtl838x_l2_entry e;
	int err = 0, idx;
	u64 seed = priv->r->l2_hash_seed(mac, vid);
	int lag_group = rtldsa_find_lag_group_from_port(priv, port);

	if (lag_group >= 0 && priv->r->prepare_lag_fdb)
		priv->r->prepare_lag_fdb(&e, lag_group);

	pr_debug("In %s, mac %llx, vid: %d\n", __func__, mac, vid);

	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, true, &e);

	if (idx >= 0) {
		pr_debug("Found entry index %d, key %d and bucket %d\n", idx, idx >> 2, idx & 3);
		e.valid = false;
		priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		goto out;
	}

	/* Check CAM for spillover from hash buckets */
	idx = rtldsa_find_l2_cam_entry(priv, seed, true, &e);

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

static int rtldsa_port_fdb_dump(struct dsa_switch *ds, int port,
				dsa_fdb_dump_cb_t *cb, void *data)
{
	struct rtl838x_l2_entry e;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);

	for (int i = 0; i < priv->r->fib_entries; i++) {
		priv->r->read_l2_entry_using_hash(i >> 2, i & 0x3, &e);

		if (!e.valid)
			continue;

		// Ignore trunk fdb entries
		if (e.is_trunk)
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

		// Ignore trunk fdb entries
		if (e.is_trunk)
			continue;

		if (e.port == port)
			cb(e.mac, e.vid, e.is_static, data);
	}

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static bool rtldsa_mac_is_unsnoop(const unsigned char *addr)
{
	/*
	 * RFC4541, section 2.1.2.2 + section 3:
	 * Unsnoopable address ranges must always be flooded.
	 *
	 * mapped MAC for 224.0.0.x -> 01:00:5e:00:00:xx
	 * mapped MAC for ff02::1 -> 33:33:00:00:00:01
	 */
	if (ether_addr_equal_masked(addr, ipv4_ll_mcast_addr_base,
				    ipv4_ll_mcast_addr_mask) ||
	    ether_addr_equal_masked(addr, ipv6_all_hosts_mcast_addr_base,
				    ipv6_all_hosts_mcast_addr_mask))
		return true;

	return false;
}

static int rtldsa_83xx_port_mdb_add(struct dsa_switch *ds, int port,
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

	pr_debug("In %s port %d, mac %llx, vid: %d\n", __func__, port, mac, vid);

	if (priv->lag_non_primary & BIT_ULL(port)) {
		pr_debug("%s: %d is lag slave. ignore\n", __func__, port);
		return -EINVAL;
	}

	if (rtldsa_mac_is_unsnoop(mdb->addr)) {
		dev_dbg(priv->dev,
			"%s: %pM might belong to an unsnoopable IP. ignore\n",
			__func__, mdb->addr);
		return -EADDRNOTAVAIL;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, false, &e);

	/* Found an existing or empty entry */
	if (idx >= 0) {
		if (e.valid) {
			pr_debug("Found an existing entry %016llx, mc_group %d\n",
				 ether_addr_to_u64(e.mac), e.mc_portmask_index);
			rtldsa_mc_group_add_port(priv, e.mc_portmask_index, port);
		} else {
			pr_debug("New entry for seed %016llx\n", seed);
			mc_group = rtldsa_mc_group_alloc(priv, port);
			if (mc_group < 0) {
				err = -ENOTSUPP;
				goto out;
			}
			rtldsa_setup_l2_mc_entry(&e, vid, mac, mc_group);
			priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		}
		goto out;
	}

	/* Hash buckets full, try CAM */
	idx = rtldsa_find_l2_cam_entry(priv, seed, false, &e);

	if (idx >= 0) {
		if (e.valid) {
			pr_debug("Found existing CAM entry %016llx, mc_group %d\n",
				 ether_addr_to_u64(e.mac), e.mc_portmask_index);
			rtldsa_mc_group_add_port(priv, e.mc_portmask_index, port);
		} else {
			pr_debug("New entry\n");
			mc_group = rtldsa_mc_group_alloc(priv, port);
			if (mc_group < 0) {
				err = -ENOTSUPP;
				goto out;
			}
			rtldsa_setup_l2_mc_entry(&e, vid, mac, mc_group);
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
static int rtldsa_93xx_port_mdb_add(struct dsa_switch *ds, int port,
				    const struct switchdev_obj_port_mdb *mdb,
				    const struct dsa_db db)
{
	return -EOPNOTSUPP;
}

static int rtldsa_port_mdb_del(struct dsa_switch *ds, int port,
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

	if (priv->lag_non_primary & BIT_ULL(port)) {
		pr_info("%s: %d is lag slave. ignore\n", __func__, port);
		return 0;
	}

	if (rtldsa_mac_is_unsnoop(mdb->addr)) {
		dev_dbg(priv->dev,
			"%s: %pM might belong to an unsnoopable IP. ignore\n",
			__func__, mdb->addr);
		return 0;
	}

	mutex_lock(&priv->reg_mutex);

	idx = rtldsa_find_l2_hash_entry(priv, seed, true, &e);

	if (idx >= 0) {
		pr_debug("Found entry index %d, key %d and bucket %d\n", idx, idx >> 2, idx & 3);
		portmask = rtldsa_mc_group_del_port(priv, e.mc_portmask_index, port);
		if (!portmask) {
			e.valid = false;
			priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);
		}
		goto out;
	}

	/* Check CAM for spillover from hash buckets */
	idx = rtldsa_find_l2_cam_entry(priv, seed, true, &e);

	if (idx >= 0) {
		portmask = rtldsa_mc_group_del_port(priv, e.mc_portmask_index, port);
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

static int rtldsa_port_mirror_add(struct dsa_switch *ds, int port,
				  struct dsa_mall_mirror_tc_entry *mirror,
				  bool ingress, struct netlink_ext_ack *extack)
{
	/* We support 4 mirror groups, one destination port per group */
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_mirror_config config;
	int err = 0;
	int pm_reg;
	int group;
	int r;

	if (!priv->r->get_mirror_config)
		return -EOPNOTSUPP;

	pr_debug("In %s\n", __func__);

	mutex_lock(&priv->reg_mutex);

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

	if (group >= 4) {
		err = -ENOSPC;
		goto out_unlock;
	}

	pr_debug("Using group %d\n", group);

	r = priv->r->get_mirror_config(&config, group, mirror->to_local_port);
	if (r < 0) {
		err = r;
		goto out_unlock;
	}

	if (ingress)
		pm_reg = config.spm;
	else
		pm_reg = config.dpm;

	sw_w32(config.val, config.ctrl);

	if (priv->r->get_port_reg_be(pm_reg) & (1ULL << port)) {
		err = -EEXIST;
		goto out_unlock;
	}

	priv->r->mask_port_reg_be(0, 1ULL << port, pm_reg);
	priv->mirror_group_ports[group] = mirror->to_local_port;

out_unlock:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static void rtldsa_port_mirror_del(struct dsa_switch *ds, int port,
				   struct dsa_mall_mirror_tc_entry *mirror)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_mirror_config config;
	int group = 0;
	int r;

	if (!priv->r->get_mirror_config)
		return;

	pr_debug("In %s\n", __func__);

	mutex_lock(&priv->reg_mutex);

	for (group = 0; group < 4; group++) {
		if (priv->mirror_group_ports[group] == mirror->to_local_port)
			break;
	}
	if (group >= 4)
		goto out_unlock;

	r = priv->r->get_mirror_config(&config, group, mirror->to_local_port);
	if (r < 0)
		goto out_unlock;

	if (mirror->ingress) {
		/* Ingress, clear source port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, config.spm);
	} else {
		/* Egress, clear destination port matrix */
		priv->r->mask_port_reg_be(1ULL << port, 0, config.dpm);
	}

	if (!(sw_r32(config.spm) || sw_r32(config.dpm))) {
		priv->mirror_group_ports[group] = -1;
		sw_w32(0, config.ctrl);
	}

out_unlock:
	mutex_unlock(&priv->reg_mutex);
}

static int rtldsa_port_pre_bridge_flags(struct dsa_switch *ds, int port,
					struct switchdev_brport_flags flags,
					struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	unsigned long features = BR_ISOLATED;

	pr_debug("%s: %d %lX\n", __func__, port, flags.val);
	if (priv->r->enable_learning)
		features |= BR_LEARNING;

	if (priv->r->enable_flood)
		features |= BR_FLOOD;

	if (priv->r->enable_l2_new_sa_fwd)
		features |= BR_PORT_LOCKED;

	if (priv->r->enable_mcast_flood)
		features |= BR_MCAST_FLOOD;
	if (priv->r->enable_bcast_flood)
		features |= BR_BCAST_FLOOD;
	if (flags.mask & ~(features))
		return -EINVAL;

	return 0;
}

/* dsa_port_fast_age() is DSA internal, repeat its bridge notification here. */
static void rtldsa_port_fast_age_notify(struct dsa_port *dp)
{
	struct net_device *brport_dev = dsa_port_to_bridge_port(dp);
	struct switchdev_notifier_fdb_info info = {
		.vid = 0, /* all VLANs */
	};

	rtldsa_port_fast_age(dp->ds, dp->index);

	if (!brport_dev)
		return;

	call_switchdev_notifiers(SWITCHDEV_FDB_FLUSH_TO_BRIDGE, brport_dev,
				 &info.info, NULL);
}

static int rtldsa_port_bridge_flags(struct dsa_switch *ds, int port,
				    struct switchdev_brport_flags flags,
				    struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct dsa_port *dp = dsa_to_port(ds, port);
	enum rtldsa_flood_type new_sa_fwd;
	unsigned long cached_flags;

	pr_debug("%s: %d %lX\n", __func__, port, flags.val);

	priv->ports[port].cached_flags &= ~flags.mask;
	priv->ports[port].cached_flags |= flags.val & flags.mask;

	cached_flags = priv->ports[port].cached_flags;

	if (cached_flags & BR_PORT_LOCKED) {
		/* A locked port must not learn addresses on its own, and the
		 * entries it learned before are no longer authorized.
		 */
		rtldsa_port_set_salrn(priv, port, false);

		if (flags.mask & BR_PORT_LOCKED)
			rtldsa_port_fast_age_notify(dp);
	} else {
		rtldsa_port_set_salrn(priv, port, !!(cached_flags & BR_LEARNING));
	}

	priv->ports[port].flood_type = (cached_flags & BR_FLOOD) ?
				       RTLDSA_FLOOD_TYPE_FORWARD :
				       RTLDSA_FLOOD_TYPE_DROP;

	if (priv->r->enable_flood)
		priv->r->enable_flood(port, priv->ports[port].flood_type);

	/* Trap frames with an unknown source address on a locked port to the
	 * CPU, so that an authenticator can inspect them and add an FDB entry.
	 */
	new_sa_fwd = (cached_flags & BR_PORT_LOCKED) ? RTLDSA_FLOOD_TYPE_TRAP2CPU :
						       RTLDSA_FLOOD_TYPE_FORWARD;

	if (priv->r->enable_l2_new_sa_fwd)
		priv->r->enable_l2_new_sa_fwd(port, new_sa_fwd);

	if (priv->r->enable_learning)
		priv->r->enable_learning(port, !!(cached_flags & BR_LEARNING));

	if (priv->r->enable_mcast_flood)
		priv->r->enable_mcast_flood(port, !!(cached_flags & BR_MCAST_FLOOD));

	if (priv->r->enable_bcast_flood)
		priv->r->enable_bcast_flood(port, !!(cached_flags & BR_BCAST_FLOOD));

	if (flags.mask & BR_ISOLATED) {
		struct net_device *bridge_dev = dsa_port_bridge_dev_get(dp);

		priv->ports[port].isolated = !!(cached_flags & BR_ISOLATED);

		mutex_lock(&priv->reg_mutex);
		rtldsa_update_port_member(priv, port, bridge_dev, true);
		mutex_unlock(&priv->reg_mutex);
	}

	return 0;
}

static bool rtldsa_83xx_lag_can_offload(struct dsa_switch *ds,
					struct net_device *lag,
					struct netdev_lag_upper_info *info)
{
	int id;

	id = dsa_lag_id(ds->dst, lag);
	if (id < 0 || id >= ds->num_lag_ids)
		return false;

	if (info->tx_type != NETDEV_LAG_TX_TYPE_HASH)
		return false;

	if (info->hash_type != NETDEV_LAG_HASH_L2 && info->hash_type != NETDEV_LAG_HASH_L23)
		return false;

	return true;
}

static int rtldsa_port_lag_change(struct dsa_switch *ds, int port)
{
	struct dsa_port *dp = dsa_to_port(ds, port);
	struct rtl838x_switch_priv *priv = ds->priv;
	int lag_group;
	int ret;

	if (!dp)
		return -EINVAL;

	lag_group = rtldsa_find_lag_group_from_port(priv, port);
	if (lag_group < 0)
		return lag_group;

	if (priv->r->lag_set_port_members) {
		/* Set same port members again, the function should check against
		 * lag_tx_enabled and set egress ports accordingly.
		 */
		ret = priv->r->lag_set_port_members(priv, lag_group,
						    priv->lags_port_members[lag_group],
						    NULL);
		if (ret)
			return ret;
	}

	return 0;
}

static int rtldsa_port_lag_join(struct dsa_switch *ds,
				int port,
				struct dsa_lag lag,
				struct netdev_lag_upper_info *info,
				struct netlink_ext_ack *extack)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int err = 0;
	int group;

	if (!rtldsa_83xx_lag_can_offload(ds, lag.dev, info))
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	if (port >= priv->r->cpu_port) {
		err = -EINVAL;
		goto out;
	}

	group = dsa_lag_id(ds->dst, lag.dev);

	pr_info("port_lag_join: group %d, port %d\n", group, port);

	if (priv->lag_primary[group] == -1)
		priv->lag_primary[group] = port;
	else
		priv->lag_non_primary |= BIT_ULL(port);

	priv->lagmembers |= BIT_ULL(port);

	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_add(priv->ds, group, port, info);
	if (err) {
		err = -EINVAL;
		goto out;
	}

out:
	mutex_unlock(&priv->reg_mutex);

	return err;
}

static int rtldsa_port_lag_leave(struct dsa_switch *ds, int port,
				 struct dsa_lag lag)
{
	int group, err;
	struct rtl838x_switch_priv *priv = ds->priv;

	mutex_lock(&priv->reg_mutex);

	group = dsa_lag_id(ds->dst, lag.dev);
	if (group == -1) {
		pr_info("port_lag_leave: group %d not set\n", port);
		err = -EINVAL;
		goto out;
	}

	if (port >= priv->r->cpu_port) {
		err = -EINVAL;
		goto out;
	}
	pr_info("port_lag_del: group %d, port %d\n", group, port);
	priv->lagmembers &= ~BIT_ULL(port);
	priv->lag_non_primary &= ~BIT_ULL(port);
	pr_debug("lag_members = %llX\n", priv->lagmembers);
	err = rtl83xx_lag_del(priv->ds, group, port);
	if (err) {
		err = -EINVAL;
		goto out;
	}

	/* To re-elect primary interface, just remove the first interface in
	 * this-group's interfaces from non-primary
	 */
	if (priv->lags_port_members[group]) {
		priv->lag_primary[group] = fls64(priv->lags_port_members[group]);
		priv->lag_non_primary &= ~BIT_ULL(priv->lag_primary[group]);
	}

	/* No need to update fdb entries since they make use of trunk_id for entry.
	 * The primary interface is only calculated at time of
	 * port_fdb_dump
	 */

out:
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static const struct flow_action_entry *rtldsa_rate_policy_extract(struct flow_cls_offload *cls)
{
	struct flow_rule *rule;

	/* only simple rules with a single action are supported */
	rule = flow_cls_offload_flow_rule(cls);

	if (!flow_action_basic_hw_stats_check(&cls->rule->action,
					      cls->common.extack))
		return NULL;

	if (!flow_offload_has_one_action(&rule->action))
		return NULL;

	return &rule->action.entries[0];
}

static bool rtldsa_port_rate_police_validate(const struct flow_action_entry *act)
{
	if (!act)
		return false;

	/* only allow action which just limit rate with by dropping packets */
	if (act->id != FLOW_ACTION_POLICE)
		return false;

	if (act->police.rate_pkt_ps > 0)
		return false;

	if (act->police.exceed.act_id != FLOW_ACTION_DROP)
		return false;

	if (act->police.notexceed.act_id != FLOW_ACTION_ACCEPT)
		return false;

	return true;
}

static int rtldsa_cls_flower_add(struct dsa_switch *ds, int port,
				 struct flow_cls_offload *cls,
				 bool ingress)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_port *p = &priv->ports[port];
	const struct flow_action_entry *act;
	int ret;

	if (!priv->r->port_rate_police_add)
		return -EOPNOTSUPP;

	/* the single action must be a rate/bandwidth limiter */
	act = rtldsa_rate_policy_extract(cls);

	if (!rtldsa_port_rate_police_validate(act))
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	/* only allow one offloaded police for ingress/egress */
	if (ingress && p->rate_police_ingress) {
		ret = -EOPNOTSUPP;
		goto unlock;
	}

	if (!ingress && p->rate_police_egress) {
		ret = -EOPNOTSUPP;
		goto unlock;
	}

	ret = priv->r->port_rate_police_add(ds, port, act, ingress);
	if (ret < 0)
		goto unlock;

	if (ingress)
		p->rate_police_ingress = true;
	else
		p->rate_police_egress = true;

unlock:
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int rtldsa_cls_flower_del(struct dsa_switch *ds, int port,
				 struct flow_cls_offload *cls,
				 bool ingress)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	struct rtldsa_port *p = &priv->ports[port];
	int ret;

	if (!priv->r->port_rate_police_del)
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);

	ret = priv->r->port_rate_police_del(ds, port, cls, ingress);
	if (ret < 0)
		goto unlock;

	if (ingress)
		p->rate_police_ingress = false;
	else
		p->rate_police_egress = false;

unlock:
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

const struct phylink_mac_ops rtldsa_83xx_phylink_mac_ops = {
	.mac_config		= rtldsa_83xx_phylink_mac_config,
	.mac_link_down		= rtldsa_phylink_mac_link_down,
	.mac_link_up		= rtldsa_83xx_phylink_mac_link_up,
};

const struct dsa_switch_ops rtldsa_83xx_switch_ops = {
	.get_tag_protocol	= rtldsa_get_tag_protocol,
	.setup			= rtldsa_83xx_setup,

	.phylink_get_caps	= rtldsa_83xx_phylink_get_caps,

	.get_strings		= rtldsa_get_strings,
	.get_ethtool_stats	= rtldsa_get_ethtool_stats,
	.get_sset_count		= rtldsa_get_sset_count,
	.get_eth_phy_stats	= rtldsa_get_eth_phy_stats,
	.get_eth_mac_stats	= rtldsa_get_eth_mac_stats,
	.get_eth_ctrl_stats	= rtldsa_get_eth_ctrl_stats,
	.get_rmon_stats		= rtldsa_get_rmon_stats,
	.get_stats64		= rtldsa_get_stats64,
	.get_pause_stats	= rtldsa_get_pause_stats,

	.port_enable		= rtldsa_port_enable,
	.port_disable		= rtldsa_port_disable,

	.support_eee		= rtldsa_support_eee,
	.set_mac_eee		= rtldsa_set_mac_eee,

	.set_ageing_time	= rtldsa_set_ageing_time,
	.port_bridge_join	= rtldsa_port_bridge_join,
	.port_bridge_leave	= rtldsa_port_bridge_leave,
	.port_stp_state_set	= rtldsa_port_stp_state_set,
	.port_fast_age		= rtldsa_port_fast_age,
	.port_mst_state_set	= rtldsa_port_mst_state_set,

	.port_vlan_filtering	= rtldsa_vlan_filtering,
	.port_vlan_add		= rtldsa_vlan_add,
	.port_vlan_del		= rtldsa_vlan_del,
	.port_vlan_fast_age	= rtldsa_port_vlan_fast_age,
	.vlan_msti_set		= rtldsa_vlan_msti_set,

	.port_fdb_add		= rtldsa_port_fdb_add,
	.port_fdb_del		= rtldsa_port_fdb_del,
	.port_fdb_dump		= rtldsa_port_fdb_dump,

	.port_mdb_add		= rtldsa_83xx_port_mdb_add,
	.port_mdb_del		= rtldsa_port_mdb_del,

	.port_mirror_add	= rtldsa_port_mirror_add,
	.port_mirror_del	= rtldsa_port_mirror_del,

	.port_lag_change	= rtldsa_port_lag_change,
	.port_lag_join		= rtldsa_port_lag_join,
	.port_lag_leave		= rtldsa_port_lag_leave,

	.port_pre_bridge_flags	= rtldsa_port_pre_bridge_flags,
	.port_bridge_flags	= rtldsa_port_bridge_flags,
};

const struct phylink_mac_ops rtldsa_93xx_phylink_mac_ops = {
	.mac_config		= rtldsa_93xx_phylink_mac_config,
	.mac_link_down		= rtldsa_phylink_mac_link_down,
	.mac_link_up		= rtldsa_93xx_phylink_mac_link_up,
};

const struct dsa_switch_ops rtldsa_93xx_switch_ops = {
	.get_tag_protocol	= rtldsa_get_tag_protocol,
	.setup			= rtldsa_93xx_setup,

	.phylink_get_caps	= rtldsa_93xx_phylink_get_caps,

	.get_strings		= rtldsa_get_strings,
	.get_ethtool_stats	= rtldsa_get_ethtool_stats,
	.get_sset_count		= rtldsa_get_sset_count,
	.get_eth_phy_stats	= rtldsa_get_eth_phy_stats,
	.get_eth_mac_stats	= rtldsa_get_eth_mac_stats,
	.get_eth_ctrl_stats	= rtldsa_get_eth_ctrl_stats,
	.get_rmon_stats		= rtldsa_get_rmon_stats,
	.get_stats64		= rtldsa_get_stats64,
	.get_pause_stats	= rtldsa_get_pause_stats,

	.port_enable		= rtldsa_port_enable,
	.port_disable		= rtldsa_port_disable,

	.support_eee		= rtldsa_support_eee,
	.set_mac_eee		= rtldsa_set_mac_eee,

	.set_ageing_time	= rtldsa_set_ageing_time,
	.port_bridge_join	= rtldsa_port_bridge_join,
	.port_bridge_leave	= rtldsa_port_bridge_leave,
	.port_stp_state_set	= rtldsa_port_stp_state_set,
	.port_fast_age		= rtldsa_port_fast_age,
	.port_mst_state_set	= rtldsa_port_mst_state_set,

	.port_vlan_filtering	= rtldsa_vlan_filtering,
	.port_vlan_add		= rtldsa_vlan_add,
	.port_vlan_del		= rtldsa_vlan_del,
	.port_vlan_fast_age	= rtldsa_port_vlan_fast_age,
	.vlan_msti_set		= rtldsa_vlan_msti_set,

	.port_fdb_add		= rtldsa_port_fdb_add,
	.port_fdb_del		= rtldsa_port_fdb_del,
	.port_fdb_dump		= rtldsa_port_fdb_dump,

	.port_mdb_add		= rtldsa_93xx_port_mdb_add,
	.port_mdb_del		= rtldsa_port_mdb_del,

	.port_mirror_add	= rtldsa_port_mirror_add,
	.port_mirror_del	= rtldsa_port_mirror_del,

	.port_lag_change	= rtldsa_port_lag_change,
	.port_lag_join		= rtldsa_port_lag_join,
	.port_lag_leave		= rtldsa_port_lag_leave,

	.port_pre_bridge_flags	= rtldsa_port_pre_bridge_flags,
	.port_bridge_flags	= rtldsa_port_bridge_flags,

	.cls_flower_add		= rtldsa_cls_flower_add,
	.cls_flower_del		= rtldsa_cls_flower_del,
};
