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

#include <linux/of_mdio.h>
#include "ag71xx.h"

static void ag71xx_phy_link_adjust(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct phy_device *phydev = ag->phy_dev;
	unsigned long flags;
	int status_change = 0;

	spin_lock_irqsave(&ag->lock, flags);

	if (phydev->link) {
		if (ag->duplex != phydev->duplex
		    || ag->speed != phydev->speed) {
			status_change = 1;
		}
	}

	if (phydev->link != ag->link)
		status_change = 1;

	ag->link = phydev->link;
	ag->duplex = phydev->duplex;
	ag->speed = phydev->speed;

	if (status_change)
		ag71xx_link_adjust(ag);

	spin_unlock_irqrestore(&ag->lock, flags);
}

int ag71xx_phy_connect(struct ag71xx *ag)
{
	struct device_node *np = ag->pdev->dev.of_node;
	struct device_node *phy_node;
	int ret;

	if (of_phy_is_fixed_link(np)) {
		ret = of_phy_register_fixed_link(np);
		if (ret < 0) {
			dev_err(&ag->pdev->dev,
				"Failed to register fixed PHY link: %d\n", ret);
			return ret;
		}

		phy_node = of_node_get(np);
	} else {
		phy_node = of_parse_phandle(np, "phy-handle", 0);
	}

	if (!phy_node) {
		dev_err(&ag->pdev->dev,
			"Could not find valid phy node\n");
		return -ENODEV;
	}

	ag->phy_dev = of_phy_connect(ag->dev, phy_node, ag71xx_phy_link_adjust,
				     0, ag->phy_if_mode);

	of_node_put(phy_node);

	if (!ag->phy_dev) {
		dev_err(&ag->pdev->dev,
			"Could not connect to PHY device. Deferring probe.\n");
		return -EPROBE_DEFER;
	}

	dev_info(&ag->pdev->dev, "connected to PHY at %s [uid=%08x, driver=%s]\n",
		    phydev_name(ag->phy_dev),
		    ag->phy_dev->phy_id, ag->phy_dev->drv->name);

	return 0;
}

void ag71xx_phy_disconnect(struct ag71xx *ag)
{
	phy_disconnect(ag->phy_dev);
}
