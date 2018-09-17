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
#include <linux/version.h>

static int ag71xx_ethtool_get_settings(struct net_device *dev,
				       struct ethtool_cmd *cmd)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct phy_device *phydev = ag->phy_dev;

	if (!phydev)
		return -ENODEV;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	return phy_ethtool_gset(phydev, cmd);
#else
	return phy_ethtool_ioctl(phydev, cmd);
#endif
}

static int ag71xx_ethtool_set_settings(struct net_device *dev,
				       struct ethtool_cmd *cmd)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct phy_device *phydev = ag->phy_dev;

	if (!phydev)
		return -ENODEV;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	return phy_ethtool_sset(phydev, cmd);
#else
	return phy_ethtool_ioctl(phydev, cmd);
#endif
}

static void ag71xx_ethtool_get_drvinfo(struct net_device *dev,
				       struct ethtool_drvinfo *info)
{
	struct ag71xx *ag = netdev_priv(dev);

	strcpy(info->driver, ag->pdev->dev.driver->name);
	strcpy(info->version, AG71XX_DRV_VERSION);
	strcpy(info->bus_info, dev_name(&ag->pdev->dev));
}

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

static void ag71xx_ethtool_get_ringparam(struct net_device *dev,
					 struct ethtool_ringparam *er)
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

static int ag71xx_ethtool_set_ringparam(struct net_device *dev,
					struct ethtool_ringparam *er)
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

struct ethtool_ops ag71xx_ethtool_ops = {
	.set_settings	= ag71xx_ethtool_set_settings,
	.get_settings	= ag71xx_ethtool_get_settings,
	.get_drvinfo	= ag71xx_ethtool_get_drvinfo,
	.get_msglevel	= ag71xx_ethtool_get_msglevel,
	.set_msglevel	= ag71xx_ethtool_set_msglevel,
	.get_ringparam	= ag71xx_ethtool_get_ringparam,
	.set_ringparam	= ag71xx_ethtool_set_ringparam,
	.get_link	= ethtool_op_get_link,
	.get_ts_info	= ethtool_op_get_ts_info,
};
