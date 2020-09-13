/*
 * Copyright (c) 2015 - 2016, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/string.h>
#include "edma.h"

struct edma_ethtool_stats {
	uint8_t stat_string[ETH_GSTRING_LEN];
	uint32_t stat_offset;
};

#define EDMA_STAT(m)    offsetof(struct edma_ethtool_statistics, m)
#define DRVINFO_LEN	32

/* Array of strings describing statistics
 */
static const struct edma_ethtool_stats edma_gstrings_stats[] = {
	{"tx_q0_pkt", EDMA_STAT(tx_q0_pkt)},
	{"tx_q1_pkt", EDMA_STAT(tx_q1_pkt)},
	{"tx_q2_pkt", EDMA_STAT(tx_q2_pkt)},
	{"tx_q3_pkt", EDMA_STAT(tx_q3_pkt)},
	{"tx_q4_pkt", EDMA_STAT(tx_q4_pkt)},
	{"tx_q5_pkt", EDMA_STAT(tx_q5_pkt)},
	{"tx_q6_pkt", EDMA_STAT(tx_q6_pkt)},
	{"tx_q7_pkt", EDMA_STAT(tx_q7_pkt)},
	{"tx_q8_pkt", EDMA_STAT(tx_q8_pkt)},
	{"tx_q9_pkt", EDMA_STAT(tx_q9_pkt)},
	{"tx_q10_pkt", EDMA_STAT(tx_q10_pkt)},
	{"tx_q11_pkt", EDMA_STAT(tx_q11_pkt)},
	{"tx_q12_pkt", EDMA_STAT(tx_q12_pkt)},
	{"tx_q13_pkt", EDMA_STAT(tx_q13_pkt)},
	{"tx_q14_pkt", EDMA_STAT(tx_q14_pkt)},
	{"tx_q15_pkt", EDMA_STAT(tx_q15_pkt)},
	{"tx_q0_byte", EDMA_STAT(tx_q0_byte)},
	{"tx_q1_byte", EDMA_STAT(tx_q1_byte)},
	{"tx_q2_byte", EDMA_STAT(tx_q2_byte)},
	{"tx_q3_byte", EDMA_STAT(tx_q3_byte)},
	{"tx_q4_byte", EDMA_STAT(tx_q4_byte)},
	{"tx_q5_byte", EDMA_STAT(tx_q5_byte)},
	{"tx_q6_byte", EDMA_STAT(tx_q6_byte)},
	{"tx_q7_byte", EDMA_STAT(tx_q7_byte)},
	{"tx_q8_byte", EDMA_STAT(tx_q8_byte)},
	{"tx_q9_byte", EDMA_STAT(tx_q9_byte)},
	{"tx_q10_byte", EDMA_STAT(tx_q10_byte)},
	{"tx_q11_byte", EDMA_STAT(tx_q11_byte)},
	{"tx_q12_byte", EDMA_STAT(tx_q12_byte)},
	{"tx_q13_byte", EDMA_STAT(tx_q13_byte)},
	{"tx_q14_byte", EDMA_STAT(tx_q14_byte)},
	{"tx_q15_byte", EDMA_STAT(tx_q15_byte)},
	{"rx_q0_pkt", EDMA_STAT(rx_q0_pkt)},
	{"rx_q1_pkt", EDMA_STAT(rx_q1_pkt)},
	{"rx_q2_pkt", EDMA_STAT(rx_q2_pkt)},
	{"rx_q3_pkt", EDMA_STAT(rx_q3_pkt)},
	{"rx_q4_pkt", EDMA_STAT(rx_q4_pkt)},
	{"rx_q5_pkt", EDMA_STAT(rx_q5_pkt)},
	{"rx_q6_pkt", EDMA_STAT(rx_q6_pkt)},
	{"rx_q7_pkt", EDMA_STAT(rx_q7_pkt)},
	{"rx_q0_byte", EDMA_STAT(rx_q0_byte)},
	{"rx_q1_byte", EDMA_STAT(rx_q1_byte)},
	{"rx_q2_byte", EDMA_STAT(rx_q2_byte)},
	{"rx_q3_byte", EDMA_STAT(rx_q3_byte)},
	{"rx_q4_byte", EDMA_STAT(rx_q4_byte)},
	{"rx_q5_byte", EDMA_STAT(rx_q5_byte)},
	{"rx_q6_byte", EDMA_STAT(rx_q6_byte)},
	{"rx_q7_byte", EDMA_STAT(rx_q7_byte)},
	{"tx_desc_error", EDMA_STAT(tx_desc_error)},
	{"rx_alloc_fail_ctr", EDMA_STAT(rx_alloc_fail_ctr)},
};

#define EDMA_STATS_LEN ARRAY_SIZE(edma_gstrings_stats)

/* edma_get_strset_count()
 *	Get strset count
 */
static int edma_get_strset_count(struct net_device *netdev,
				 int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return EDMA_STATS_LEN;
	default:
		netdev_dbg(netdev, "%s: Invalid string set", __func__);
		return -EOPNOTSUPP;
	}
}


/* edma_get_strings()
 *	get stats string
 */
static void edma_get_strings(struct net_device *netdev, uint32_t stringset,
			     uint8_t *data)
{
	uint8_t *p = data;
	uint32_t i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < EDMA_STATS_LEN; i++) {
			memcpy(p, edma_gstrings_stats[i].stat_string,
				min((size_t)ETH_GSTRING_LEN,
				    strlen(edma_gstrings_stats[i].stat_string)
				    + 1));
			p += ETH_GSTRING_LEN;
		}
		break;
	}
}

/* edma_get_ethtool_stats()
 *	Get ethtool statistics
 */
static void edma_get_ethtool_stats(struct net_device *netdev,
				   struct ethtool_stats *stats, uint64_t *data)
{
	struct edma_adapter *adapter = netdev_priv(netdev);
	struct edma_common_info *edma_cinfo = adapter->edma_cinfo;
	int i;
	uint8_t *p = NULL;

	edma_read_append_stats(edma_cinfo);

	for(i = 0; i < EDMA_STATS_LEN; i++) {
		p = (uint8_t *)&(edma_cinfo->edma_ethstats) +
			edma_gstrings_stats[i].stat_offset;
		data[i] = *(uint32_t *)p;
	}
}

/* edma_get_drvinfo()
 *	get edma driver info
 */
static void edma_get_drvinfo(struct net_device *dev,
			     struct ethtool_drvinfo *info)
{
	strlcpy(info->driver, "ess_edma", DRVINFO_LEN);
	strlcpy(info->bus_info, "axi", ETHTOOL_BUSINFO_LEN);
}

/* edma_nway_reset()
 *	Reset the phy, if available.
 */
static int edma_nway_reset(struct net_device *netdev)
{
	return -EINVAL;
}

/* edma_get_wol()
 *	get wake on lan info
 */
static void edma_get_wol(struct net_device *netdev,
			 struct ethtool_wolinfo *wol)
{
	wol->supported = 0;
	wol->wolopts = 0;
}

/* edma_get_msglevel()
 *	get message level.
 */
static uint32_t edma_get_msglevel(struct net_device *netdev)
{
	return 0;
}

/* edma_get_settings()
 *	Get edma settings
 */
static int edma_get_settings(struct net_device *netdev,
			     struct ethtool_link_ksettings *cmd)
{
	struct edma_adapter *adapter = netdev_priv(netdev);

	if (adapter->poll_required) {
		struct phy_device *phydev = NULL;
		uint16_t phyreg;

		if ((adapter->forced_speed != SPEED_UNKNOWN)
			&& !(adapter->poll_required))
			return -EPERM;

		phydev = adapter->phydev;

		linkmode_copy(cmd->link_modes.advertising, phydev->advertising);
		linkmode_copy(cmd->link_modes.supported, phydev->supported);

		cmd->base.autoneg = phydev->autoneg;

		if (adapter->link_state == __EDMA_LINKDOWN) {
			cmd->base.speed =  SPEED_UNKNOWN;
			cmd->base.duplex = DUPLEX_UNKNOWN;
		} else {
			cmd->base.speed = phydev->speed;
			cmd->base.duplex = phydev->duplex;
		}

		cmd->base.phy_address = adapter->phy_mdio_addr;

		phyreg = (uint16_t)phy_read(adapter->phydev, MII_LPA);
		if (phyreg & LPA_10HALF)
			linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT, 
								cmd->link_modes.lp_advertising);

		if (phyreg & LPA_10FULL)
			linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT, 
								cmd->link_modes.lp_advertising);

		if (phyreg & LPA_100HALF)
			linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT, 
								cmd->link_modes.lp_advertising);

		if (phyreg & LPA_100FULL)
			linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, 
								cmd->link_modes.lp_advertising);

		phyreg = (uint16_t)phy_read(adapter->phydev, MII_STAT1000);
		if (phyreg & LPA_1000HALF)
			linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Half_BIT, 
								cmd->link_modes.lp_advertising);

		if (phyreg & LPA_1000FULL)
			linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, 
								cmd->link_modes.lp_advertising);
	} else {
		/* If the speed/duplex for this GMAC is forced and we
		 * are not polling for link state changes, return the
		 * values as specified by platform. This will be true
		 * for GMACs connected to switch, and interfaces that
		 * do not use a PHY.
		 */
		if (!(adapter->poll_required)) {
			if (adapter->forced_speed != SPEED_UNKNOWN) {
				/* set speed and duplex */
				cmd->base.speed = SPEED_1000;
				cmd->base.duplex = DUPLEX_FULL;

				/* Populate capabilities advertised by self */
				linkmode_zero(cmd->link_modes.advertising);
				cmd->base.autoneg = 0;
				cmd->base.port = PORT_TP;
				cmd->base.transceiver = XCVR_EXTERNAL;
			} else {
				/* non link polled and non
				 * forced speed/duplex interface
				 */
				return -EIO;
			}
		}
	}

	return 0;
}

/* edma_set_settings()
 *	Set EDMA settings
 */
static int edma_set_settings(struct net_device *netdev,
			    const struct ethtool_link_ksettings *cmd)
{
	struct edma_adapter *adapter = netdev_priv(netdev);
	struct phy_device *phydev = NULL;

	if ((adapter->forced_speed != SPEED_UNKNOWN) &&
	     !adapter->poll_required)
		return -EPERM;

	phydev = adapter->phydev;
	linkmode_copy(phydev->advertising, cmd->link_modes.advertising);
	linkmode_copy(phydev->supported, cmd->link_modes.supported);
	phydev->autoneg = cmd->base.autoneg;
	phydev->speed = cmd->base.speed;
	phydev->duplex = cmd->base.duplex;

	genphy_config_aneg(phydev);

	return 0;
}

/* edma_get_coalesce
 *	get interrupt mitigation
 */
static int edma_get_coalesce(struct net_device *netdev,
			     struct ethtool_coalesce *ec)
{
	u32 reg_val;

	edma_get_tx_rx_coalesce(&reg_val);

	/* We read the Interrupt Moderation Timer(IMT) register value,
	 * use lower 16 bit for rx and higher 16 bit for Tx. We do a
	 * left shift by 1, because IMT resolution timer is 2usecs.
	 * Hence the value given by the register is multiplied by 2 to
	 * get the actual time in usecs.
	 */
	ec->tx_coalesce_usecs = (((reg_val >> 16) & 0xffff) << 1);
	ec->rx_coalesce_usecs = ((reg_val & 0xffff) << 1);

	return 0;
}

/* edma_set_coalesce
 *	set interrupt mitigation
 */
static int edma_set_coalesce(struct net_device *netdev,
			     struct ethtool_coalesce *ec)
{
	if (ec->tx_coalesce_usecs)
		edma_change_tx_coalesce(ec->tx_coalesce_usecs);
	if (ec->rx_coalesce_usecs)
		edma_change_rx_coalesce(ec->rx_coalesce_usecs);

	return 0;
}

/* edma_set_priv_flags()
 *	Set EDMA private flags
 */
static int edma_set_priv_flags(struct net_device *netdev, u32 flags)
{
	return 0;
}

/* edma_get_priv_flags()
 *	get edma driver flags
 */
static u32 edma_get_priv_flags(struct net_device *netdev)
{
	return 0;
}

/* edma_get_ringparam()
 *	get ring size
 */
static void edma_get_ringparam(struct net_device *netdev,
			       struct ethtool_ringparam *ring)
{
	struct edma_adapter *adapter = netdev_priv(netdev);
	struct edma_common_info *edma_cinfo = adapter->edma_cinfo;

	ring->tx_max_pending = edma_cinfo->tx_ring_count;
	ring->rx_max_pending = edma_cinfo->rx_ring_count;
}

/* Ethtool operations
 */
static const struct ethtool_ops edma_ethtool_ops = {
	.get_drvinfo = &edma_get_drvinfo,
	.get_link = &ethtool_op_get_link,
	.get_msglevel = &edma_get_msglevel,
	.nway_reset = &edma_nway_reset,
	.get_wol = &edma_get_wol,
	.get_link_ksettings = &edma_get_settings,
	.set_link_ksettings = &edma_set_settings,
	.get_strings = &edma_get_strings,
	.get_sset_count = &edma_get_strset_count,
	.get_ethtool_stats = &edma_get_ethtool_stats,
	.get_coalesce = &edma_get_coalesce,
	.set_coalesce = &edma_set_coalesce,
	.get_priv_flags = edma_get_priv_flags,
	.set_priv_flags = edma_set_priv_flags,
	.get_ringparam = edma_get_ringparam,
};

/* edma_set_ethtool_ops
 *	Set ethtool operations
 */
void edma_set_ethtool_ops(struct net_device *netdev)
{
	netdev->ethtool_ops = &edma_ethtool_ops;
}
