// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SWMAC specific glue layer
 *
 * Copyright (c) 2022-2023, Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#include "stmmac.h"
#include "stmmac_platform.h"

static int phytium_get_mac_mode(struct fwnode_handle *fwnode)
{
	const char *pm;
	int err, i;

	err = fwnode_property_read_string(fwnode, "mac-mode", &pm);
	if (err < 0)
		return err;

	for (i = 0; i < PHY_INTERFACE_MODE_MAX; i++) {
		if (!strcasecmp(pm, phy_modes(i)))
			return i;
	}

	return -ENODEV;
}

static int phytium_dwmac_acpi_phy(struct plat_stmmacenet_data *plat,
		struct fwnode_handle *np, struct device *dev)
{
	plat->mdio_bus_data = devm_kzalloc(dev,
			sizeof(struct stmmac_mdio_bus_data),
			GFP_KERNEL);

	if (!plat->mdio_bus_data)
		return -ENOMEM;

	return 0;
}

static int phytium_dwmac_probe(struct platform_device *pdev)
{
	struct fwnode_handle *fwnode = dev_fwnode(&pdev->dev);
	struct plat_stmmacenet_data *plat;
	struct stmmac_resources stmmac_res;
	struct device_node *np = pdev->dev.of_node;
	u64 clk_freq;
	char clk_name[20];
	int ret;

	plat = devm_kzalloc(&pdev->dev, sizeof(*plat), GFP_KERNEL);
	if (!plat)
		return -ENOMEM;

	plat->dma_cfg = devm_kzalloc(&pdev->dev, sizeof(*plat->dma_cfg), GFP_KERNEL);
	if (!plat->dma_cfg)
		return -ENOMEM;

	plat->axi = devm_kzalloc(&pdev->dev, sizeof(*plat->axi), GFP_KERNEL);
	if (!plat->axi)
		return -ENOMEM;

	plat->phy_interface = device_get_phy_mode(&pdev->dev);
	if (plat->phy_interface < 0)
		return plat->phy_interface;

	plat->interface = phytium_get_mac_mode(fwnode);
	if (plat->interface < 0)
		plat->interface = plat->phy_interface;

	/* Configure PHY if using device-tree */
	if (pdev->dev.of_node) {
		plat->phy_node = of_parse_phandle(np, "phy-handle", 0);
		plat->phylink_node = np;
	}

	if (pdev->dev.of_node) {
		plat->bus_id = of_alias_get_id(np, "ethernet");
		if (plat->bus_id < 0)
			plat->bus_id = 0;
	} else if (fwnode_property_read_u32(fwnode, "bus_id", &plat->bus_id)) {
		plat->bus_id = 2;
	}

	plat->phy_addr = -1;
	plat->clk_csr = -1;
	plat->has_gmac = 1;
	plat->enh_desc = 1;
	plat->bugged_jumbo = 1;
	plat->pmt = 1;
	plat->force_sf_dma_mode = 1;

	if (fwnode_property_read_u32(fwnode, "max-speed", &plat->max_speed))
		plat->max_speed = -1;

	if (fwnode_property_read_u32(fwnode, "max-frame-size", &plat->maxmtu))
		plat->maxmtu = JUMBO_LEN;

	if (fwnode_property_read_u32(fwnode, "snps,multicast-filter-bins",
				     &plat->multicast_filter_bins))
		plat->multicast_filter_bins = HASH_TABLE_SIZE;

	if (fwnode_property_read_u32(fwnode, "snps,perfect-filter-entries",
				     &plat->unicast_filter_entries))
		plat->unicast_filter_entries = 1;

	if (fwnode_property_read_u32(fwnode, "tx-fifo-depth", &plat->tx_fifo_size))
		plat->tx_fifo_size = 0x1000;

	if (fwnode_property_read_u32(fwnode, "rx-fifo-depth", &plat->rx_fifo_size))
		plat->rx_fifo_size = 0x1000;

	if (phytium_dwmac_acpi_phy(plat, fwnode, &pdev->dev))
		return -ENODEV;

	plat->rx_queues_to_use = 1;
	plat->tx_queues_to_use = 1;
	plat->rx_queues_cfg[0].mode_to_use = MTL_QUEUE_DCB;
	plat->tx_queues_cfg[0].mode_to_use = MTL_QUEUE_DCB;

	if (fwnode_property_read_u64(fwnode, "clock-frequency", &clk_freq))
		clk_freq = 125000000;

	/* Set system clock */
	snprintf(clk_name, sizeof(clk_name), "%s-%d", "stmmaceth", plat->bus_id);

	plat->stmmac_clk = clk_register_fixed_rate(&pdev->dev, clk_name, NULL, 0, clk_freq);
	if (IS_ERR(plat->stmmac_clk)) {
		dev_warn(&pdev->dev, "Fail to register stmmac-clk\n");
		plat->stmmac_clk = NULL;
	}

	ret = clk_prepare_enable(plat->stmmac_clk);
	if (ret) {
		clk_unregister_fixed_rate(plat->stmmac_clk);
		return ret;
	}

	plat->clk_ptp_rate = clk_get_rate(plat->stmmac_clk);
	plat->clk_ptp_ref = NULL;

	if (fwnode_property_read_u32(fwnode, "snps,pbl", &plat->dma_cfg->pbl))
		plat->dma_cfg->pbl = 16;

	fwnode_property_read_u32(fwnode, "snps,txpbl", &plat->dma_cfg->txpbl);
	fwnode_property_read_u32(fwnode, "snps,rxpbl", &plat->dma_cfg->rxpbl);

	plat->dma_cfg->pblx8 = !fwnode_property_read_bool(fwnode, "snps,no-pbl-x8");
	plat->dma_cfg->aal = fwnode_property_read_bool(fwnode, "snps,aal");
	plat->dma_cfg->fixed_burst = fwnode_property_read_bool(fwnode, "snps,fixed-burst");
	plat->dma_cfg->mixed_burst = fwnode_property_read_bool(fwnode, "snps,mixed-burst");

	plat->axi->axi_lpi_en = false;
	plat->axi->axi_xit_frm = false;
	plat->axi->axi_wr_osr_lmt = 7;
	plat->axi->axi_rd_osr_lmt = 7;
	plat->axi->axi_blen[0] = 16;

	memset(&stmmac_res, 0, sizeof(stmmac_res));
	stmmac_res.addr = devm_platform_ioremap_resource(pdev, 0);
	stmmac_res.irq = platform_get_irq(pdev, 0);
	if (stmmac_res.irq < 0) {
		dev_err(&pdev->dev, "IRQ not found.\n");
		return -ENXIO;
	}
	stmmac_res.wol_irq = stmmac_res.irq;
	stmmac_res.lpi_irq = -1;

	return  stmmac_dvr_probe(&pdev->dev, plat, &stmmac_res);
}

int phytium_dwmac_remove(struct platform_device *pdev)
{
	int ret;
	struct net_device *ndev = platform_get_drvdata(pdev);
	struct stmmac_priv *priv = netdev_priv(ndev);
	struct plat_stmmacenet_data *plat = priv->plat;

	ret = stmmac_pltfr_remove(pdev);
	clk_unregister_fixed_rate(plat->stmmac_clk);
	return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id phytium_dwmac_of_match[] = {
	{ .compatible = "phytium,gmac" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_dwmac_of_match);
#endif

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_dwmac_acpi_ids[] = {
	{ .id = "PHYT0004" },
	{}
};
MODULE_DEVICE_TABLE(acpi, phytium_dwmac_acpi_ids);
#endif

static struct platform_driver phytium_dwmac_driver = {
	.probe = phytium_dwmac_probe,
	.remove = phytium_dwmac_remove,
	.driver = {
		.name		= "phytium-dwmac",
		.pm		= &stmmac_pltfr_pm_ops,
		.of_match_table	= of_match_ptr(phytium_dwmac_of_match),
		.acpi_match_table = ACPI_PTR(phytium_dwmac_acpi_ids),
	},
};
module_platform_driver(phytium_dwmac_driver);

MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium DWMAC specific glue layer");
MODULE_LICENSE("GPL v2");
