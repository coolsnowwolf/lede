/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include "ag71xx.h"

struct ag71xx_statistic {
	unsigned short offset;
	u32 mask;
	const char name[ETH_GSTRING_LEN];
};

static const struct ag71xx_statistic ag71xx_statistics[] = {
	{ 0x0080, GENMASK(17, 0), "Tx/Rx 64 Byte", },
	{ 0x0084, GENMASK(17, 0), "Tx/Rx 65-127 Byte", },
	{ 0x0088, GENMASK(17, 0), "Tx/Rx 128-255 Byte", },
	{ 0x008C, GENMASK(17, 0), "Tx/Rx 256-511 Byte", },
	{ 0x0090, GENMASK(17, 0), "Tx/Rx 512-1023 Byte", },
	{ 0x0094, GENMASK(17, 0), "Tx/Rx 1024-1518 Byte", },
	{ 0x0098, GENMASK(17, 0), "Tx/Rx 1519-1522 Byte VLAN", },
	{ 0x009C, GENMASK(23, 0), "Rx Byte", },
	{ 0x00A0, GENMASK(17, 0), "Rx Packet", },
	{ 0x00A4, GENMASK(11, 0), "Rx FCS Error", },
	{ 0x00A8, GENMASK(17, 0), "Rx Multicast Packet", },
	{ 0x00AC, GENMASK(21, 0), "Rx Broadcast Packet", },
	{ 0x00B0, GENMASK(17, 0), "Rx Control Frame Packet", },
	{ 0x00B4, GENMASK(11, 0), "Rx Pause Frame Packet", },
	{ 0x00B8, GENMASK(11, 0), "Rx Unknown OPCode Packet", },
	{ 0x00BC, GENMASK(11, 0), "Rx Alignment Error", },
	{ 0x00C0, GENMASK(15, 0), "Rx Frame Length Error", },
	{ 0x00C4, GENMASK(11, 0), "Rx Code Error", },
	{ 0x00C8, GENMASK(11, 0), "Rx Carrier Sense Error", },
	{ 0x00CC, GENMASK(11, 0), "Rx Undersize Packet", },
	{ 0x00D0, GENMASK(11, 0), "Rx Oversize Packet", },
	{ 0x00D4, GENMASK(11, 0), "Rx Fragments", },
	{ 0x00D8, GENMASK(11, 0), "Rx Jabber", },
	{ 0x00DC, GENMASK(11, 0), "Rx Dropped Packet", },
	{ 0x00E0, GENMASK(23, 0), "Tx Byte", },
	{ 0x00E4, GENMASK(17, 0), "Tx Packet", },
	{ 0x00E8, GENMASK(17, 0), "Tx Multicast Packet", },
	{ 0x00EC, GENMASK(17, 0), "Tx Broadcast Packet", },
	{ 0x00F0, GENMASK(11, 0), "Tx Pause Control Frame", },
	{ 0x00F4, GENMASK(11, 0), "Tx Deferral Packet", },
	{ 0x00F8, GENMASK(11, 0), "Tx Excessive Deferral Packet", },
	{ 0x00FC, GENMASK(11, 0), "Tx Single Collision Packet", },
	{ 0x0100, GENMASK(11, 0), "Tx Multiple Collision", },
	{ 0x0104, GENMASK(11, 0), "Tx Late Collision Packet", },
	{ 0x0108, GENMASK(11, 0), "Tx Excessive Collision Packet", },
	{ 0x010C, GENMASK(12, 0), "Tx Total Collision", },
	{ 0x0110, GENMASK(11, 0), "Tx Pause Frames Honored", },
	{ 0x0114, GENMASK(11, 0), "Tx Drop Frame", },
	{ 0x0118, GENMASK(11, 0), "Tx Jabber Frame", },
	{ 0x011C, GENMASK(11, 0), "Tx FCS Error", },
	{ 0x0120, GENMASK(11, 0), "Tx Control Frame", },
	{ 0x0124, GENMASK(11, 0), "Tx Oversize Frame", },
	{ 0x0128, GENMASK(11, 0), "Tx Undersize Frame", },
	{ 0x012C, GENMASK(11, 0), "Tx Fragment", },
};

static u32 ag71xx_ethtool_get_msglevel(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);

	return ag->msg_enable;
}

static void ag71xx_ethtool_set_msglevel(struct net_device *dev, u32 msg_level)
{
	struct ag71xx *ag = netdev_priv(dev);

	ag->msg_enable = msg_level;
}

static void
ag71xx_ethtool_get_ringparam(struct net_device *dev,
			     struct ethtool_ringparam *er,
			     struct kernel_ethtool_ringparam *kernel_ring,
			     struct netlink_ext_ack *extack)
{
	struct ag71xx *ag = netdev_priv(dev);

	er->tx_max_pending = AG71XX_TX_RING_SIZE_MAX;
	er->rx_max_pending = AG71XX_RX_RING_SIZE_MAX;
	er->rx_mini_max_pending = 0;
	er->rx_jumbo_max_pending = 0;

	er->tx_pending = BIT(ag->tx_ring.order);
	er->rx_pending = BIT(ag->rx_ring.order);
	er->rx_mini_pending = 0;
	er->rx_jumbo_pending = 0;

	if (ag->tx_ring.desc_split)
		er->tx_pending /= AG71XX_TX_RING_DS_PER_PKT;
}

static int
ag71xx_ethtool_set_ringparam(struct net_device *dev,
			     struct ethtool_ringparam *er,
			     struct kernel_ethtool_ringparam *kernel_ring,
			     struct netlink_ext_ack *extack)
{
	struct ag71xx *ag = netdev_priv(dev);
	unsigned tx_size;
	unsigned rx_size;
	int err = 0;

	if (er->rx_mini_pending != 0||
	    er->rx_jumbo_pending != 0 ||
	    er->rx_pending == 0 ||
	    er->tx_pending == 0)
		return -EINVAL;

	tx_size = er->tx_pending < AG71XX_TX_RING_SIZE_MAX ?
		  er->tx_pending : AG71XX_TX_RING_SIZE_MAX;

	rx_size = er->rx_pending < AG71XX_RX_RING_SIZE_MAX ?
		  er->rx_pending : AG71XX_RX_RING_SIZE_MAX;

	if (netif_running(dev)) {
		err = dev->netdev_ops->ndo_stop(dev);
		if (err)
			return err;
	}

	if (ag->tx_ring.desc_split)
		tx_size *= AG71XX_TX_RING_DS_PER_PKT;

	ag->tx_ring.order = ag71xx_ring_size_order(tx_size);
	ag->rx_ring.order = ag71xx_ring_size_order(rx_size);

	if (netif_running(dev))
		err = dev->netdev_ops->ndo_open(dev);

	return err;
}

static int ag71xx_ethtool_nway_reset(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct phy_device *phydev = ag->phy_dev;

	if (!phydev)
		return -ENODEV;

	return genphy_restart_aneg(phydev);
}

static void ag71xx_ethtool_get_strings(struct net_device *netdev, u32 sset,
				       u8 *data)
{
	if (sset == ETH_SS_STATS) {
		int i;

		for (i = 0; i < ARRAY_SIZE(ag71xx_statistics); i++)
			ethtool_puts(&data, ag71xx_statistics[i].name);
	}
}

static void ag71xx_ethtool_get_stats(struct net_device *ndev,
				     struct ethtool_stats *stats, u64 *data)
{
	struct ag71xx *ag = netdev_priv(ndev);
	int i;

	for (i = 0; i < ARRAY_SIZE(ag71xx_statistics); i++)
		*data++ = ag71xx_rr(ag, ag71xx_statistics[i].offset)
				& ag71xx_statistics[i].mask;
}

static int ag71xx_ethtool_get_sset_count(struct net_device *ndev, int sset)
{
	if (sset == ETH_SS_STATS)
		return ARRAY_SIZE(ag71xx_statistics);
	return -EOPNOTSUPP;
}

struct ethtool_ops ag71xx_ethtool_ops = {
	.get_msglevel	= ag71xx_ethtool_get_msglevel,
	.set_msglevel	= ag71xx_ethtool_set_msglevel,
	.get_ringparam	= ag71xx_ethtool_get_ringparam,
	.set_ringparam	= ag71xx_ethtool_set_ringparam,
	.get_link_ksettings = phy_ethtool_get_link_ksettings,
	.set_link_ksettings = phy_ethtool_set_link_ksettings,
	.get_link	= ethtool_op_get_link,
	.get_ts_info	= ethtool_op_get_ts_info,
	.nway_reset	= ag71xx_ethtool_nway_reset,
	.get_strings		= ag71xx_ethtool_get_strings,
	.get_ethtool_stats	= ag71xx_ethtool_get_stats,
	.get_sset_count		= ag71xx_ethtool_get_sset_count,
};
