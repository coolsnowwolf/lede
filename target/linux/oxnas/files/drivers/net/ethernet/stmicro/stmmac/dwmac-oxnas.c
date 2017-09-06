/* Copyright OpenWrt.org (C) 2015.
 * Copyright Altera Corporation (C) 2014. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Adopted from dwmac-socfpga.c
 * Based on code found in mach-oxnas.c
 */

#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/stmmac.h>

#include <mach/hardware.h>

#include "stmmac.h"
#include "stmmac_platform.h"

struct oxnas_gmac {
	struct clk *clk;
};

static int oxnas_gmac_init(struct platform_device *pdev, void *priv)
{
	struct oxnas_gmac *bsp_priv = priv;
	int ret = 0;
	unsigned value;

	ret = device_reset(&pdev->dev);
	if (ret)
		return ret;

	if (IS_ERR(bsp_priv->clk))
		return PTR_ERR(bsp_priv->clk);
	clk_prepare_enable(bsp_priv->clk);

	value = readl(SYS_CTRL_GMAC_CTRL);

	/* Enable GMII_GTXCLK to follow GMII_REFCLK, required for gigabit PHY */
	value |= BIT(SYS_CTRL_GMAC_CKEN_GTX);
	/* Use simple mux for 25/125 Mhz clock switching */
	value |= BIT(SYS_CTRL_GMAC_SIMPLE_MUX);
	/* set auto switch tx clock source */
	value |= BIT(SYS_CTRL_GMAC_AUTO_TX_SOURCE);
	/* enable tx & rx vardelay */
	value |= BIT(SYS_CTRL_GMAC_CKEN_TX_OUT);
	value |= BIT(SYS_CTRL_GMAC_CKEN_TXN_OUT);
	value |= BIT(SYS_CTRL_GMAC_CKEN_TX_IN);
	value |= BIT(SYS_CTRL_GMAC_CKEN_RX_OUT);
	value |= BIT(SYS_CTRL_GMAC_CKEN_RXN_OUT);
	value |= BIT(SYS_CTRL_GMAC_CKEN_RX_IN);
	writel(value, SYS_CTRL_GMAC_CTRL);

	/* set tx & rx vardelay */
	value = 0;
	value |= SYS_CTRL_GMAC_TX_VARDELAY(4);
	value |= SYS_CTRL_GMAC_TXN_VARDELAY(2);
	value |= SYS_CTRL_GMAC_RX_VARDELAY(10);
	value |= SYS_CTRL_GMAC_RXN_VARDELAY(8);
	writel(value, SYS_CTRL_GMAC_DELAY_CTRL);

	return 0;
}

static void oxnas_gmac_exit(struct platform_device *pdev, void *priv)
{
	struct reset_control *rstc;

	clk_disable_unprepare(priv);
	devm_clk_put(&pdev->dev, priv);

	rstc = reset_control_get(&pdev->dev, NULL);
	if (!IS_ERR(rstc)) {
		reset_control_assert(rstc);
		reset_control_put(rstc);
	}
}

static int oxnas_gmac_probe(struct platform_device *pdev)
{
	struct plat_stmmacenet_data *plat_dat;
	struct stmmac_resources stmmac_res;
	int			ret;
	struct device		*dev = &pdev->dev;
	struct oxnas_gmac	*bsp_priv;

	bsp_priv = devm_kzalloc(dev, sizeof(*bsp_priv), GFP_KERNEL);
	if (!bsp_priv)
		return -ENOMEM;
	bsp_priv->clk = devm_clk_get(dev, "gmac");
	if (IS_ERR(bsp_priv->clk))
		return PTR_ERR(bsp_priv->clk);

	ret = stmmac_get_platform_resources(pdev, &stmmac_res);
	if (ret)
		return ret;

	plat_dat = stmmac_probe_config_dt(pdev, &stmmac_res.mac);
	if (IS_ERR(plat_dat))
		return PTR_ERR(plat_dat);

	plat_dat->bsp_priv = bsp_priv;
	plat_dat->init = oxnas_gmac_init;
	plat_dat->exit = oxnas_gmac_exit;

	ret = oxnas_gmac_init(pdev, bsp_priv);
	if (ret)
		return ret;

	return stmmac_dvr_probe(dev, plat_dat, &stmmac_res);
}

static const struct of_device_id oxnas_gmac_match[] = {
	{ .compatible = "plxtech,nas782x-gmac" },
	{ }
};
MODULE_DEVICE_TABLE(of, oxnas_gmac_match);

static struct platform_driver oxnas_gmac_driver = {
	.probe  = oxnas_gmac_probe,
	.remove = stmmac_pltfr_remove,
	.driver = {
		.name		= "oxnas-gmac",
		.pm		= &stmmac_pltfr_pm_ops,
		.of_match_table = oxnas_gmac_match,
	},
};
module_platform_driver(oxnas_gmac_driver);

MODULE_LICENSE("GPL v2");
