/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/sizes.h>
#include <linux/of_address.h>
#include "ag71xx.h"

static void ag71xx_of_set(struct device_node *np, const char *prop,
			  u32 *reg, u32 shift, u32 mask)
{
	u32 val;

	if (of_property_read_u32(np, prop, &val))
		return;

	*reg &= ~(mask << shift);
	*reg |= ((val & mask) << shift);
}

static void ag71xx_of_bit(struct device_node *np, const char *prop,
			  u32 *reg, u32 mask)
{
	u32 val;

	if (of_property_read_u32(np, prop, &val))
		return;

	if (val)
		*reg |= mask;
	else
		*reg &= ~mask;
}

static void ag71xx_setup_gmac_933x(struct device_node *np, void __iomem *base)
{
	u32 val = __raw_readl(base + AR933X_GMAC_REG_ETH_CFG);

	ag71xx_of_bit(np, "switch-phy-swap", &val, AR933X_ETH_CFG_SW_PHY_SWAP);
	ag71xx_of_bit(np, "switch-phy-addr-swap", &val,
		AR933X_ETH_CFG_SW_PHY_ADDR_SWAP);

	__raw_writel(val, base + AR933X_GMAC_REG_ETH_CFG);
}

static void ag71xx_setup_gmac_934x(struct device_node *np, void __iomem *base)
{
	u32 val = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	ag71xx_of_bit(np, "rgmii-gmac0", &val, AR934X_ETH_CFG_RGMII_GMAC0);
	ag71xx_of_bit(np, "mii-gmac0", &val, AR934X_ETH_CFG_MII_GMAC0);
	ag71xx_of_bit(np, "gmii-gmac0", &val, AR934X_ETH_CFG_GMII_GMAC0);
	ag71xx_of_bit(np, "switch-phy-swap", &val, AR934X_ETH_CFG_SW_PHY_SWAP);
	ag71xx_of_bit(np, "switch-only-mode", &val,
		AR934X_ETH_CFG_SW_ONLY_MODE);
	ag71xx_of_set(np, "rxdv-delay", &val,
		      AR934X_ETH_CFG_RDV_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "rxd-delay", &val,
		      AR934X_ETH_CFG_RXD_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "txd-delay", &val,
		      AR934X_ETH_CFG_TXD_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "txen-delay", &val,
		      AR934X_ETH_CFG_TXE_DELAY_SHIFT, 0x3);

	__raw_writel(val, base + AR934X_GMAC_REG_ETH_CFG);
}

static void ag71xx_setup_gmac_955x(struct device_node *np, void __iomem *base)
{
	u32 val = __raw_readl(base + QCA955X_GMAC_REG_ETH_CFG);

	ag71xx_of_bit(np, "rgmii-enabled", &val, QCA955X_ETH_CFG_RGMII_EN);
	ag71xx_of_bit(np, "ge0-sgmii", &val, QCA955X_ETH_CFG_GE0_SGMII);
	ag71xx_of_set(np, "txen-delay", &val, QCA955X_ETH_CFG_TXE_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "txd-delay", &val, QCA955X_ETH_CFG_TXD_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "rxdv-delay", &val, QCA955X_ETH_CFG_RDV_DELAY_SHIFT, 0x3);
	ag71xx_of_set(np, "rxd-delay", &val, QCA955X_ETH_CFG_RXD_DELAY_SHIFT, 0x3);

	__raw_writel(val, base + QCA955X_GMAC_REG_ETH_CFG);
}

static void ag71xx_setup_gmac_956x(struct device_node *np, void __iomem *base)
{
	u32 val = __raw_readl(base + QCA956X_GMAC_REG_ETH_CFG);

	ag71xx_of_bit(np, "switch-phy-swap", &val, QCA956X_ETH_CFG_SW_PHY_SWAP);
	ag71xx_of_bit(np, "switch-phy-addr-swap", &val,
		QCA956X_ETH_CFG_SW_PHY_ADDR_SWAP);

	__raw_writel(val, base + QCA956X_GMAC_REG_ETH_CFG);
}

int ag71xx_setup_gmac(struct device_node *np)
{
	struct device_node *np_dev;
	void __iomem *base;
	int err = 0;

	np = of_get_child_by_name(np, "gmac-config");
	if (!np)
		return 0;

	np_dev = of_parse_phandle(np, "device", 0);
	if (!np_dev)
		goto out;

	base = of_iomap(np_dev, 0);
	if (!base) {
		pr_err("%pOF: can't map GMAC registers\n", np_dev);
		err = -ENOMEM;
		goto err_iomap;
	}

	if (of_device_is_compatible(np_dev, "qca,ar9330-gmac"))
		ag71xx_setup_gmac_933x(np, base);
	else if (of_device_is_compatible(np_dev, "qca,ar9340-gmac"))
		ag71xx_setup_gmac_934x(np, base);
	else if (of_device_is_compatible(np_dev, "qca,qca9550-gmac"))
		ag71xx_setup_gmac_955x(np, base);
	else if (of_device_is_compatible(np_dev, "qca,qca9560-gmac"))
		ag71xx_setup_gmac_956x(np, base);

	iounmap(base);

err_iomap:
	of_node_put(np_dev);
out:
	of_node_put(np);
	return err;
}
