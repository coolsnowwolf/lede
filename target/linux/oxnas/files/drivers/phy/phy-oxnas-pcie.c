// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2019 Daniel Golle <daniel@makrotopia.org>
 *
 */

#include <dt-bindings/phy/phy.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/mfd/syscon.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>

#define ADDR_VAL(val)	((val) & 0xFFFF)
#define DATA_VAL(val)	((val) & 0xFFFF)

#define SYS_CTRL_HCSL_CTRL_REGOFFSET	0x114

enum {
	HCSL_BIAS_ON = BIT(0),
	HCSL_PCIE_EN = BIT(1),
	HCSL_PCIEA_EN = BIT(2),
	HCSL_PCIEB_EN = BIT(3),
};

enum {
	/* pcie phy reg offset */
	PHY_ADDR = 0,
	PHY_DATA = 4,
	/* phy data reg bits */
	READ_EN = BIT(16),
	WRITE_EN = BIT(17),
	CAP_DATA = BIT(18),
};

struct oxnas_pcie_phy {
	struct device *dev;
	void __iomem *membase;
	const struct phy_ops *ops;
	struct regmap *sys_ctrl;
	struct reset_control *rstc;
};

static int oxnas_pcie_phy_init(struct phy *phy)
{
	struct oxnas_pcie_phy *pciephy = phy_get_drvdata(phy);
	int ret;

	/* generate clocks from HCSL buffers, shared parts */
	regmap_write(pciephy->sys_ctrl, SYS_CTRL_HCSL_CTRL_REGOFFSET, HCSL_BIAS_ON|HCSL_PCIE_EN);

	/* Ensure PCIe PHY is properly reset */
	ret = reset_control_reset(pciephy->rstc);

	if (ret) {
		dev_err(pciephy->dev, "phy reset failed %d\n", ret);
		return ret;
	}

	return 0;
}

static int oxnas_pcie_phy_power_on(struct phy *phy)
{
	struct oxnas_pcie_phy *pciephy = phy_get_drvdata(phy);

	/* Enable PCIe Pre-Emphasis: What these value means? */
	writel(ADDR_VAL(0x0014), pciephy->membase + PHY_ADDR);
	writel(DATA_VAL(0xce10) | CAP_DATA, pciephy->membase + PHY_DATA);
	writel(DATA_VAL(0xce10) | WRITE_EN, pciephy->membase + PHY_DATA);

	writel(ADDR_VAL(0x2004), pciephy->membase + PHY_ADDR);
	writel(DATA_VAL(0x82c7) | CAP_DATA, pciephy->membase + PHY_DATA);
	writel(DATA_VAL(0x82c7) | WRITE_EN, pciephy->membase + PHY_DATA);

	return 0;
}

static const struct phy_ops ops = {
	.init           = oxnas_pcie_phy_init,
	.power_on       = oxnas_pcie_phy_power_on,
	.owner          = THIS_MODULE,
};

static int oxnas_pcie_phy_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct phy *generic_phy;
	struct phy_provider *phy_provider;
	struct oxnas_pcie_phy *pciephy;
	struct regmap *sys_ctrl;
	struct reset_control *rstc;
	void __iomem *membase;

	membase = of_iomap(np, 0);
	if (IS_ERR(membase))
		return PTR_ERR(membase);

	sys_ctrl = syscon_regmap_lookup_by_compatible("oxsemi,ox820-sys-ctrl");
	if (IS_ERR(sys_ctrl))
		return PTR_ERR(sys_ctrl);

	rstc = devm_reset_control_get_shared(dev, "phy");
	if (IS_ERR(rstc))
		return PTR_ERR(rstc);

	pciephy = devm_kzalloc(dev, sizeof(*pciephy), GFP_KERNEL);
	if (!pciephy)
		return -ENOMEM;

	pciephy->sys_ctrl = sys_ctrl;
	pciephy->rstc = rstc;
	pciephy->membase = membase;
	pciephy->dev = dev;
	pciephy->ops = &ops;

	generic_phy = devm_phy_create(dev, dev->of_node, pciephy->ops);
	if (IS_ERR(generic_phy)) {
		dev_err(dev, "failed to create PHY\n");
		return PTR_ERR(generic_phy);
	}

	phy_set_drvdata(generic_phy, pciephy);
	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);

	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id oxnas_pcie_phy_id_table[] = {
	{ .compatible = "oxsemi,ox820-pcie-phy" },
	{ },
};

static struct platform_driver oxnas_pcie_phy_driver = {
	.probe		= oxnas_pcie_phy_probe,
	.driver		= {
		.name		= "ox820-pcie-phy",
		.of_match_table	= oxnas_pcie_phy_id_table,
	},
};

builtin_platform_driver(oxnas_pcie_phy_driver);
