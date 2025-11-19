// SPDX-License-Identifier: GPL-2.0-only
/*
 * Siflower SF19A2890 GMAC glue layer
 * SF19A2890 GMAC is a DWMAC 3.73a with a custom HNAT engine
 * between its MAC and DMA engine.
 *
 * Copyright (C) 2024 Chuanhong Guo <gch981213@gmail.com>
 */

#include <linux/of_net.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/nvmem-consumer.h>

#include "stmmac.h"
#include "stmmac_platform.h"

struct sf19a2890_gmac_priv {
	struct device *dev;
	void __iomem *gmac_cfg;
	struct clk *gmac_byp_ref_clk;
};

#define REG_MISC		0x0
#define  MISC_PHY_INTF_SEL	GENMASK(2, 0)
#define   PHY_IF_GMII_MII	0
#define   PHY_IF_RGMII		1
#define   PHY_IF_RMII		4
#define  MISC_PTP_AUX_TS_TRIG	BIT(3)
#define  MISC_SBD_FLOWCTRL	BIT(4)
#define  CLK_RMII_OEN		BIT(5)

#define REG_CLK_TX_DELAY	0x4
#define REG_CLK_RX_PHY_DELAY	0x8
#define REG_CLK_RX_PHY_DELAY_EN	0xc

/* Siflower stores RGMII delay as a 4-byte hex string in MTD. */
#define SFGMAC_DELAY_STR_LEN	4
static int sfgmac_set_delay_from_nvmem(struct sf19a2890_gmac_priv *priv)
{
	struct device_node *np = priv->dev->of_node;
	int ret = 0;
	struct nvmem_cell *cell;
	const void *data;
	size_t retlen;
	u16 gmac_delay;
	u8 delay_tx, delay_rx;

	cell = of_nvmem_cell_get(np, "rgmii-delay");
	if (IS_ERR(cell))
		return PTR_ERR(cell);

	data = nvmem_cell_read(cell, &retlen);
	nvmem_cell_put(cell);

	if (IS_ERR(data))
		return PTR_ERR(data);

	if (retlen < SFGMAC_DELAY_STR_LEN) {
		ret = -EINVAL;
		goto exit;
	}

	ret = kstrtou16(data, 16, &gmac_delay);
	if (ret == 0) {
		delay_tx = (gmac_delay >> 8) & 0xff;
		delay_rx = gmac_delay & 0xff;
		writel(delay_tx, priv->gmac_cfg + REG_CLK_TX_DELAY);
		writel(delay_rx, priv->gmac_cfg + REG_CLK_RX_PHY_DELAY);
		if (delay_rx)
			writel(1, priv->gmac_cfg + REG_CLK_RX_PHY_DELAY_EN);
	}

exit:
	kfree(data);

	return ret;
}

static int sfgmac_setup_phy_interface(struct sf19a2890_gmac_priv *priv)
{
	phy_interface_t phy_iface;
	int mode;
	u32 reg;

	of_get_phy_mode(priv->dev->of_node, &phy_iface);
	switch (phy_iface) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_GMII:
		mode = PHY_IF_GMII_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		mode = PHY_IF_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = PHY_IF_RGMII;
		break;
	default:
		return -EOPNOTSUPP;
	}
	reg = readl(priv->gmac_cfg + REG_MISC);
	reg &= ~MISC_PHY_INTF_SEL;
	reg |= FIELD_PREP(MISC_PHY_INTF_SEL, mode);
	writel(reg, priv->gmac_cfg + REG_MISC);
	return 0;
}

static int sf19a2890_gmac_probe(struct platform_device *pdev)
{
	struct plat_stmmacenet_data *plat_dat;
	struct sf19a2890_gmac_priv *priv;
	struct stmmac_resources stmmac_res;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;

	priv->gmac_byp_ref_clk = devm_clk_get_enabled(&pdev->dev, "gmac_byp_ref");
	if (IS_ERR(priv->gmac_byp_ref_clk))
		return PTR_ERR(priv->gmac_byp_ref_clk);

	priv->gmac_cfg = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(priv->gmac_cfg)) {
		dev_err(&pdev->dev, "failed to map regs for gmac config.\n");
		return PTR_ERR(priv->gmac_cfg);
	}

	ret = sfgmac_set_delay_from_nvmem(priv);
	if (ret == -EPROBE_DEFER)
		return -EPROBE_DEFER;

	ret = sfgmac_setup_phy_interface(priv);
	if (ret)
		return ret;

	ret = stmmac_get_platform_resources(pdev, &stmmac_res);
	if (ret)
		return ret;

	plat_dat = stmmac_probe_config_dt(pdev, stmmac_res.mac);
	if (IS_ERR(plat_dat)) {
		dev_err(&pdev->dev, "dt configuration failed\n");
		return PTR_ERR(plat_dat);
	}

	plat_dat->bsp_priv = priv;
	/* This DWMAC has PCSSEL set, but it's not SGMII capable, and doesn't
	 * return anything in PCS registers under RGMII mode.
	 * Set this flag to bypass reading pcs regs stmmac_ethtool_get_link_ksettings.
	 * No idea if it's correct or not.
	 */
	plat_dat->flags |= STMMAC_FLAG_HAS_INTEGRATED_PCS;

	ret = stmmac_pltfr_probe(pdev, plat_dat, &stmmac_res);
	if (ret)
		goto err_remove_config_dt;

	return 0;

err_remove_config_dt:
	if (pdev->dev.of_node)
		stmmac_remove_config_dt(pdev, plat_dat);

	return ret;
}

static const struct of_device_id dwmac_sf19a2890_match[] = {
	{ .compatible = "siflower,sf19a2890-gmac"},
	{ }
};
MODULE_DEVICE_TABLE(of, dwmac_sf19a2890_match);

static struct platform_driver sf19a2890_gmac_driver = {
	.probe  = sf19a2890_gmac_probe,
	.remove_new = stmmac_pltfr_remove,
	.driver = {
		.name           = "sf19a2890-gmac",
		.pm		= &stmmac_pltfr_pm_ops,
		.of_match_table = dwmac_sf19a2890_match,
	},
};
module_platform_driver(sf19a2890_gmac_driver);

MODULE_DESCRIPTION("SF19A2890 GMAC driver");
MODULE_LICENSE("GPL");
