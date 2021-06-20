/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>

#include "mtk_eth_soc.h"
#include "mdio.h"

static int fe_mdio_reset(struct mii_bus *bus)
{
	/* TODO */
	return 0;
}

static void fe_phy_link_adjust(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	spin_lock_irqsave(&priv->phy->lock, flags);
	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_node[i]) {
			struct phy_device *phydev = priv->phy->phy[i];
			int status_change = 0;

			if (phydev->link)
				if (priv->phy->duplex[i] != phydev->duplex ||
				    priv->phy->speed[i] != phydev->speed)
					status_change = 1;

			if (phydev->link != priv->link[i])
				status_change = 1;

			switch (phydev->speed) {
			case SPEED_1000:
			case SPEED_100:
			case SPEED_10:
				priv->link[i] = phydev->link;
				priv->phy->duplex[i] = phydev->duplex;
				priv->phy->speed[i] = phydev->speed;

				if (status_change &&
				    priv->soc->mdio_adjust_link)
					priv->soc->mdio_adjust_link(priv, i);
				break;
			}
		}
	}
	spin_unlock_irqrestore(&priv->phy->lock, flags);
}

int fe_connect_phy_node(struct fe_priv *priv, struct device_node *phy_node, int port)
{
	const __be32 *_phy_addr = NULL;
	struct phy_device *phydev;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	int phy_mode;
#else
	phy_interface_t phy_mode = PHY_INTERFACE_MODE_NA;
#endif

	_phy_addr = of_get_property(phy_node, "reg", NULL);

	if (!_phy_addr || (be32_to_cpu(*_phy_addr) >= 0x20)) {
		pr_err("%s: invalid phy id\n", phy_node->name);
		return -EINVAL;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	phy_mode = of_get_phy_mode(phy_node);
	if (phy_mode < 0) {
#else
	of_get_phy_mode(phy_node, &phy_mode);
	if (phy_mode == PHY_INTERFACE_MODE_NA) {
#endif
		dev_err(priv->dev, "incorrect phy-mode %d\n", phy_mode);
		priv->phy->phy_node[port] = NULL;
		return -EINVAL;
	}

	phydev = of_phy_connect(priv->netdev, phy_node, fe_phy_link_adjust,
				0, phy_mode);
	if (!phydev) {
		dev_err(priv->dev, "could not connect to PHY\n");
		priv->phy->phy_node[port] = NULL;
		return -ENODEV;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	phydev->supported &= PHY_GBIT_FEATURES;
	phydev->advertising = phydev->supported;
#else
	phy_set_max_speed(phydev, SPEED_1000);
	linkmode_copy(phydev->advertising, phydev->supported);
#endif
	phydev->no_auto_carrier_off = 1;

	dev_info(priv->dev,
		 "connected port %d to PHY at %s [uid=%08x, driver=%s]\n",
		 port, dev_name(&phydev->mdio.dev), phydev->phy_id,
		 phydev->drv->name);

	priv->phy->phy[port] = phydev;
	priv->link[port] = 0;

	return 0;
}

static void phy_init(struct fe_priv *priv, struct phy_device *phy)
{
	phy_attach(priv->netdev, dev_name(&phy->mdio.dev), PHY_INTERFACE_MODE_MII);

	phy->autoneg = AUTONEG_ENABLE;
	phy->speed = 0;
	phy->duplex = 0;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
	phy->supported &= IS_ENABLED(CONFIG_NET_RALINK_MDIO_MT7620) ?
			PHY_GBIT_FEATURES : PHY_BASIC_FEATURES;
	phy->advertising = phy->supported | ADVERTISED_Autoneg;
#else
	phy_set_max_speed(phy, IS_ENABLED(CONFIG_NET_RALINK_MDIO_MT7620) ?
				       SPEED_1000 :
				       SPEED_100);
	linkmode_copy(phy->advertising, phy->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, phy->advertising);
#endif

	phy_start_aneg(phy);
}

static int fe_phy_connect(struct fe_priv *priv)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_node[i]) {
			if (!priv->phy_dev) {
				priv->phy_dev = priv->phy->phy[i];
				priv->phy_flags = FE_PHY_FLAG_PORT;
			}
		} else if (priv->mii_bus) {
			struct phy_device *phydev;

			phydev = mdiobus_get_phy(priv->mii_bus, i);
			if (!phydev || phydev->attached_dev)
				continue;

			phy_init(priv, phydev);
			if (!priv->phy_dev) {
				priv->phy_dev = mdiobus_get_phy(priv->mii_bus, i);
				priv->phy_flags = FE_PHY_FLAG_ATTACH;
			}
		}
	}

	return 0;
}

static void fe_phy_disconnect(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++)
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_disconnect(priv->phy->phy[i]);
		} else if (priv->mii_bus && mdiobus_get_phy(priv->mii_bus, i)) {
			phy_detach(mdiobus_get_phy(priv->mii_bus, i));
		}
}

static void fe_phy_start(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++) {
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 1;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_start(priv->phy->phy[i]);
		}
	}
}

static void fe_phy_stop(struct fe_priv *priv)
{
	unsigned long flags;
	int i;

	for (i = 0; i < 8; i++)
		if (priv->phy->phy_fixed[i]) {
			spin_lock_irqsave(&priv->phy->lock, flags);
			priv->link[i] = 0;
			if (priv->soc->mdio_adjust_link)
				priv->soc->mdio_adjust_link(priv, i);
			spin_unlock_irqrestore(&priv->phy->lock, flags);
		} else if (priv->phy->phy[i]) {
			phy_stop(priv->phy->phy[i]);
		}
}

static struct fe_phy phy_ralink = {
	.connect = fe_phy_connect,
	.disconnect = fe_phy_disconnect,
	.start = fe_phy_start,
	.stop = fe_phy_stop,
};

int fe_mdio_init(struct fe_priv *priv)
{
	struct device_node *mii_np;
	int err;

	if (!priv->soc->mdio_read || !priv->soc->mdio_write)
		return 0;

	spin_lock_init(&phy_ralink.lock);
	priv->phy = &phy_ralink;

	mii_np = of_get_child_by_name(priv->dev->of_node, "mdio-bus");
	if (!mii_np) {
		dev_err(priv->dev, "no %s child node found", "mdio-bus");
		return -ENODEV;
	}

	if (!of_device_is_available(mii_np)) {
		err = 0;
		goto err_put_node;
	}

	priv->mii_bus = mdiobus_alloc();
	if (!priv->mii_bus) {
		err = -ENOMEM;
		goto err_put_node;
	}

	priv->mii_bus->name = "mdio";
	priv->mii_bus->read = priv->soc->mdio_read;
	priv->mii_bus->write = priv->soc->mdio_write;
	priv->mii_bus->reset = fe_mdio_reset;
	priv->mii_bus->priv = priv;
	priv->mii_bus->parent = priv->dev;

	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%s", mii_np->name);
	err = of_mdiobus_register(priv->mii_bus, mii_np);
	if (err)
		goto err_free_bus;

	return 0;

err_free_bus:
	kfree(priv->mii_bus);
err_put_node:
	of_node_put(mii_np);
	priv->mii_bus = NULL;
	return err;
}

void fe_mdio_cleanup(struct fe_priv *priv)
{
	if (!priv->mii_bus)
		return;

	mdiobus_unregister(priv->mii_bus);
	of_node_put(priv->mii_bus->dev.of_node);
	kfree(priv->mii_bus);
}
