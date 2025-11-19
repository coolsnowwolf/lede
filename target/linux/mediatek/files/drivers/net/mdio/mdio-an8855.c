// SPDX-License-Identifier: GPL-2.0+
/*
 * MDIO passthrough driver for Airoha AN8855 Switch
 */

#include <linux/mfd/airoha-an8855-mfd.h>
#include <linux/module.h>
#include <linux/of_mdio.h>
#include <linux/platform_device.h>

static int an855_phy_restore_page(struct an8855_mfd_priv *priv,
				  int phy) __must_hold(&priv->bus->mdio_lock)
{
	/* Check PHY page only for addr shared with switch */
	if (phy != priv->switch_addr)
		return 0;

	/* Don't restore page if it's not set to switch page */
	if (priv->current_page != FIELD_GET(AN8855_PHY_PAGE,
					    AN8855_PHY_PAGE_EXTENDED_4))
		return 0;

	/* Restore page to 0, PHY might change page right after but that
	 * will be ignored as it won't be a switch page.
	 */
	return an8855_mii_set_page(priv, phy, AN8855_PHY_PAGE_STANDARD);
}

static int an8855_phy_read(struct mii_bus *bus, int phy, int regnum)
{
	struct an8855_mfd_priv *priv = bus->priv;
	struct mii_bus *real_bus = priv->bus;
	int ret;

	mutex_lock_nested(&real_bus->mdio_lock, MDIO_MUTEX_NESTED);

	ret = an855_phy_restore_page(priv, phy);
	if (ret)
		goto exit;

	ret = __mdiobus_read(real_bus, phy, regnum);
exit:
	mutex_unlock(&real_bus->mdio_lock);

	return ret;
}

static int an8855_phy_write(struct mii_bus *bus, int phy, int regnum, u16 val)
{
	struct an8855_mfd_priv *priv = bus->priv;
	struct mii_bus *real_bus = priv->bus;
	int ret;

	mutex_lock_nested(&real_bus->mdio_lock, MDIO_MUTEX_NESTED);

	ret = an855_phy_restore_page(priv, phy);
	if (ret)
		goto exit;

	ret = __mdiobus_write(real_bus, phy, regnum, val);
exit:
	mutex_unlock(&real_bus->mdio_lock);

	return ret;
}

static int an8855_mdio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct an8855_mfd_priv *priv;
	struct mii_bus *bus;
	int ret;

	/* Get priv of MFD */
	priv = dev_get_drvdata(dev->parent);

	bus = devm_mdiobus_alloc(dev);
	if (!bus)
		return -ENOMEM;

	bus->priv = priv;
	bus->name = KBUILD_MODNAME "-mii";
	snprintf(bus->id, MII_BUS_ID_SIZE, KBUILD_MODNAME "-%d",
		 priv->switch_addr);
	bus->parent = dev;
	bus->read = an8855_phy_read;
	bus->write = an8855_phy_write;

	ret = devm_of_mdiobus_register(dev, bus, dev->of_node);
	if (ret)
		return dev_err_probe(dev, ret, "failed to register MDIO bus\n");

	return ret;
}

static const struct of_device_id an8855_mdio_of_match[] = {
	{ .compatible = "airoha,an8855-mdio", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, an8855_mdio_of_match);

static struct platform_driver an8855_mdio_driver = {
	.probe	= an8855_mdio_probe,
	.driver = {
		.name = "an8855-mdio",
		.of_match_table = an8855_mdio_of_match,
	},
};
module_platform_driver(an8855_mdio_driver);

MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_DESCRIPTION("Driver for AN8855 MDIO passthrough");
MODULE_LICENSE("GPL");
