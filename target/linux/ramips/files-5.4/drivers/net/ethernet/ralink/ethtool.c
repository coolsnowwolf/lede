/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include "mtk_eth_soc.h"

static const char fe_gdma_str[][ETH_GSTRING_LEN] = {
#define _FE(x...)	# x,
FE_STAT_REG_DECLARE
#undef _FE
};

static int fe_get_link_ksettings(struct net_device *ndev,
			   struct ethtool_link_ksettings *cmd)
{
	struct fe_priv *priv = netdev_priv(ndev);

	if (!priv->phy_dev)
		return -ENODEV;

	if (priv->phy_flags == FE_PHY_FLAG_ATTACH) {
		if (phy_read_status(priv->phy_dev))
			return -ENODEV;
	}

	phy_ethtool_ksettings_get(ndev->phydev, cmd);

	return 0;
}

static int fe_set_link_ksettings(struct net_device *ndev,
			   const struct ethtool_link_ksettings *cmd)
{
	struct fe_priv *priv = netdev_priv(ndev);

	if (!priv->phy_dev)
		goto out_sset;

	if (cmd->base.phy_address != priv->phy_dev->mdio.addr) {
		if (priv->phy->phy_node[cmd->base.phy_address]) {
			priv->phy_dev = priv->phy->phy[cmd->base.phy_address];
			priv->phy_flags = FE_PHY_FLAG_PORT;
		} else if (priv->mii_bus && mdiobus_get_phy(priv->mii_bus, cmd->base.phy_address)) {
			priv->phy_dev = mdiobus_get_phy(priv->mii_bus, cmd->base.phy_address);
			priv->phy_flags = FE_PHY_FLAG_ATTACH;
		} else {
			goto out_sset;
		}
	}

	return phy_ethtool_ksettings_set(ndev->phydev, cmd);

out_sset:
	return -ENODEV;
}

static void fe_get_drvinfo(struct net_device *dev,
			   struct ethtool_drvinfo *info)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_soc_data *soc = priv->soc;

	strlcpy(info->driver, priv->dev->driver->name, sizeof(info->driver));
	strlcpy(info->version, MTK_FE_DRV_VERSION, sizeof(info->version));
	strlcpy(info->bus_info, dev_name(priv->dev), sizeof(info->bus_info));

	if (soc->reg_table[FE_REG_FE_COUNTER_BASE])
		info->n_stats = ARRAY_SIZE(fe_gdma_str);
}

static u32 fe_get_msglevel(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	return priv->msg_enable;
}

static void fe_set_msglevel(struct net_device *dev, u32 value)
{
	struct fe_priv *priv = netdev_priv(dev);

	priv->msg_enable = value;
}

static int fe_nway_reset(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	if (!priv->phy_dev)
		goto out_nway_reset;

	return genphy_restart_aneg(priv->phy_dev);

out_nway_reset:
	return -EOPNOTSUPP;
}

static u32 fe_get_link(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int err;

	if (!priv->phy_dev)
		goto out_get_link;

	if (priv->phy_flags == FE_PHY_FLAG_ATTACH) {
		err = genphy_update_link(priv->phy_dev);
		if (err)
			goto out_get_link;
	}

	return priv->phy_dev->link;

out_get_link:
	return ethtool_op_get_link(dev);
}

static int fe_set_ringparam(struct net_device *dev,
			    struct ethtool_ringparam *ring)
{
	struct fe_priv *priv = netdev_priv(dev);

	if ((ring->tx_pending < 2) ||
	    (ring->rx_pending < 2) ||
	    (ring->rx_pending > MAX_DMA_DESC) ||
	    (ring->tx_pending > MAX_DMA_DESC))
		return -EINVAL;

	dev->netdev_ops->ndo_stop(dev);

	priv->tx_ring.tx_ring_size = BIT(fls(ring->tx_pending) - 1);
	priv->rx_ring.rx_ring_size = BIT(fls(ring->rx_pending) - 1);

	dev->netdev_ops->ndo_open(dev);

	return 0;
}

static void fe_get_ringparam(struct net_device *dev,
			     struct ethtool_ringparam *ring)
{
	struct fe_priv *priv = netdev_priv(dev);

	ring->rx_max_pending = MAX_DMA_DESC;
	ring->tx_max_pending = MAX_DMA_DESC;
	ring->rx_pending = priv->rx_ring.rx_ring_size;
	ring->tx_pending = priv->tx_ring.tx_ring_size;
}

static void fe_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	switch (stringset) {
	case ETH_SS_STATS:
		memcpy(data, *fe_gdma_str, sizeof(fe_gdma_str));
		break;
	}
}

static int fe_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(fe_gdma_str);
	default:
		return -EOPNOTSUPP;
	}
}

static void fe_get_ethtool_stats(struct net_device *dev,
				 struct ethtool_stats *stats, u64 *data)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_hw_stats *hwstats = priv->hw_stats;
	u64 *data_src, *data_dst;
	unsigned int start;
	int i;

	if (netif_running(dev) && netif_device_present(dev)) {
		if (spin_trylock(&hwstats->stats_lock)) {
			fe_stats_update(priv);
			spin_unlock(&hwstats->stats_lock);
		}
	}

	do {
		data_src = &hwstats->tx_bytes;
		data_dst = data;
		start = u64_stats_fetch_begin_irq(&hwstats->syncp);

		for (i = 0; i < ARRAY_SIZE(fe_gdma_str); i++)
			*data_dst++ = *data_src++;

	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));
}

static struct ethtool_ops fe_ethtool_ops = {
	.get_link_ksettings	= fe_get_link_ksettings,
	.set_link_ksettings	= fe_set_link_ksettings,
	.get_drvinfo		= fe_get_drvinfo,
	.get_msglevel		= fe_get_msglevel,
	.set_msglevel		= fe_set_msglevel,
	.nway_reset		= fe_nway_reset,
	.get_link		= fe_get_link,
	.set_ringparam		= fe_set_ringparam,
	.get_ringparam		= fe_get_ringparam,
};

void fe_set_ethtool_ops(struct net_device *netdev)
{
	struct fe_priv *priv = netdev_priv(netdev);
	struct fe_soc_data *soc = priv->soc;

	if (soc->reg_table[FE_REG_FE_COUNTER_BASE]) {
		fe_ethtool_ops.get_strings = fe_get_strings;
		fe_ethtool_ops.get_sset_count = fe_get_sset_count;
		fe_ethtool_ops.get_ethtool_stats = fe_get_ethtool_stats;
	}

	netdev->ethtool_ops = &fe_ethtool_ops;
}
