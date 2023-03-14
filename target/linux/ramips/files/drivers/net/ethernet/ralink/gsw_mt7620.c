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

#include <linux/module.h>
#include <linux/mii.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>

#include <ralink_regs.h>

#include "mtk_eth_soc.h"
#include "gsw_mt7620.h"

void mtk_switch_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg)
{
	iowrite32(val, gsw->base + reg);
}

u32 mtk_switch_r32(struct mt7620_gsw *gsw, unsigned reg)
{
	return ioread32(gsw->base + reg);
}

static irqreturn_t gsw_interrupt_mt7620(int irq, void *_priv)
{
	struct fe_priv *priv = (struct fe_priv *)_priv;
	struct mt7620_gsw *gsw = (struct mt7620_gsw *)priv->soc->swpriv;
	u32 status;
	int i, max = (gsw->port4_ephy) ? (4) : (3);

	status = mtk_switch_r32(gsw, GSW_REG_ISR);
	if (status & PORT_IRQ_ST_CHG)
		for (i = 0; i <= max; i++) {
			u32 status = mtk_switch_r32(gsw, GSW_REG_PORT_STATUS(i));
			int link = status & 0x1;

			if (link != priv->link[i])
				mt7620_print_link_state(priv, i, link,
							(status >> 2) & 3,
							(status & 0x2));

			priv->link[i] = link;
		}
	mt7620_handle_carrier(priv);
	mtk_switch_w32(gsw, status, GSW_REG_ISR);

	return IRQ_HANDLED;
}

static void mt7620_ephy_init(struct mt7620_gsw *gsw)
{
	u32 i;
	u32 val;
	u32 is_BGA = (rt_sysc_r32(SYSC_REG_CHIP_REV_ID) >> 16) & 1;

	if (gsw->ephy_disable) {
		mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_GPC1) |
			(gsw->ephy_base << 16) | (0x1f << 24),
			GSW_REG_GPC1);

		pr_info("gsw: internal ephy disabled\n");

		return;
	} else if (gsw->ephy_base) {
		mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_GPC1) |
			(gsw->ephy_base << 16),
			GSW_REG_GPC1);
		fe_reset(MT7620A_RESET_EPHY);

		pr_info("gsw: ephy base address: %d\n", gsw->ephy_base);
	}

	/* global page 4 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0x4000);

	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 17, 0x7444);
	if (is_BGA)
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 19, 0x0114);
	else
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 19, 0x0117);

	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 22, 0x10cf);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 25, 0x6212);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 26, 0x0777);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 29, 0x4000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 28, 0xc077);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 24, 0x0000);

	/* global page 3 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0x3000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 17, 0x4838);

	/* global page 2 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0x2000);
	if (is_BGA) {
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 21, 0x0515);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 22, 0x0053);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 23, 0x00bf);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 24, 0x0aaf);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 25, 0x0fad);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 26, 0x0fc1);
	} else {
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 21, 0x0517);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 22, 0x0fd2);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 23, 0x00bf);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 24, 0x0aab);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 25, 0x00ae);
		_mt7620_mii_write(gsw, gsw->ephy_base + 1, 26, 0x0fff);
	}
	/* global page 1 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0x1000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 17, 0xe7f8);

	/* turn on all PHYs */
	for (i = 0; i <= 4; i++) {
		val = _mt7620_mii_read(gsw, gsw->ephy_base + i, MII_BMCR);
		val &= ~BMCR_PDOWN;
		val |= BMCR_ANRESTART | BMCR_ANENABLE | BMCR_SPEED100;
		_mt7620_mii_write(gsw, gsw->ephy_base + i, MII_BMCR, val);
	}

	/* global page 0 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0x8000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 0, 30, 0xa000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 30, 0xa000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 2, 30, 0xa000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 3, 30, 0xa000);

	_mt7620_mii_write(gsw, gsw->ephy_base + 0, 4, 0x05e1);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 4, 0x05e1);
	_mt7620_mii_write(gsw, gsw->ephy_base + 2, 4, 0x05e1);
	_mt7620_mii_write(gsw, gsw->ephy_base + 3, 4, 0x05e1);

	/* global page 2 */
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 31, 0xa000);
	_mt7620_mii_write(gsw, gsw->ephy_base + 0, 16, 0x1111);
	_mt7620_mii_write(gsw, gsw->ephy_base + 1, 16, 0x1010);
	_mt7620_mii_write(gsw, gsw->ephy_base + 2, 16, 0x1515);
	_mt7620_mii_write(gsw, gsw->ephy_base + 3, 16, 0x0f0f);

	/* setup port 4 */
	if (gsw->port4_ephy) {
		val = rt_sysc_r32(SYSC_REG_CFG1);

		val |= 3 << 14;
		rt_sysc_w32(val, SYSC_REG_CFG1);
		_mt7620_mii_write(gsw, gsw->ephy_base + 4, 30, 0xa000);
		_mt7620_mii_write(gsw, gsw->ephy_base + 4, 4, 0x05e1);
		_mt7620_mii_write(gsw, gsw->ephy_base + 4, 16, 0x1313);
		pr_info("gsw: setting port4 to ephy mode\n");
	}
}

static void mt7620_mac_init(struct mt7620_gsw *gsw)
{
	/* Internal ethernet requires PCIe RC mode */
	rt_sysc_w32(rt_sysc_r32(SYSC_REG_CFG1) | PCIE_RC_MODE, SYSC_REG_CFG1);

	/* Keep Global Clocks on Idle traffic */
	mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_CKGCR) & ~(0x3 << 4), GSW_REG_CKGCR);

	/* Set Port 6 to Force Link 1G, Flow Control ON */
	mtk_switch_w32(gsw, 0x5e33b, GSW_REG_PORT_PMCR(6));

	/* Set Port 6 as CPU Port */
	mtk_switch_w32(gsw, 0x7f7f7fe0, 0x0010);

	/* Enable MIB stats */
	mtk_switch_w32(gsw, mtk_switch_r32(gsw, GSW_REG_MIB_CNT_EN) | (1 << 1), GSW_REG_MIB_CNT_EN);
}

static const struct of_device_id mediatek_gsw_match[] = {
	{ .compatible = "mediatek,mt7620-gsw" },
	{},
};
MODULE_DEVICE_TABLE(of, mediatek_gsw_match);

int mtk_gsw_init(struct fe_priv *priv)
{
	struct device_node *eth_node = priv->dev->of_node;
	struct device_node *phy_node, *mdiobus_node;
	struct device_node *np = priv->switch_np;
	struct platform_device *pdev = of_find_device_by_node(np);
	struct mt7620_gsw *gsw;
	const __be32 *id;
	u8 val;

	if (!pdev)
		return -ENODEV;

	if (!of_device_is_compatible(np, mediatek_gsw_match->compatible))
		return -EINVAL;

	gsw = platform_get_drvdata(pdev);
	priv->soc->swpriv = gsw;

	gsw->ephy_disable = of_property_read_bool(np, "mediatek,ephy-disable");

	mdiobus_node = of_get_child_by_name(eth_node, "mdio-bus");
	if (mdiobus_node) {
		for_each_child_of_node(mdiobus_node, phy_node) {
			id = of_get_property(phy_node, "reg", NULL);
			if (id && (be32_to_cpu(*id) == 0x1f))
				gsw->ephy_disable = true;
		}

		of_node_put(mdiobus_node);
	}

	gsw->port4_ephy = !of_property_read_bool(np, "mediatek,port4-gmac");

	if (of_property_read_u8(np, "mediatek,ephy-base", &val) == 0)
		gsw->ephy_base = val;
	else
		gsw->ephy_base = 0;

	mt7620_mac_init(gsw);

	mt7620_ephy_init(gsw);

	if (gsw->irq) {
		request_irq(gsw->irq, gsw_interrupt_mt7620, 0,
			    "gsw", priv);
		mtk_switch_w32(gsw, ~PORT_IRQ_ST_CHG, GSW_REG_IMR);
	}

	return 0;
}

static int mt7620_gsw_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct mt7620_gsw *gsw;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct mt7620_gsw), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

	gsw->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gsw->base))
		return PTR_ERR(gsw->base);

	gsw->dev = &pdev->dev;

	gsw->irq = platform_get_irq(pdev, 0);

	platform_set_drvdata(pdev, gsw);

	return 0;
}

static int mt7620_gsw_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver gsw_driver = {
	.probe = mt7620_gsw_probe,
	.remove = mt7620_gsw_remove,
	.driver = {
		.name = "mt7620-gsw",
		.owner = THIS_MODULE,
		.of_match_table = mediatek_gsw_match,
	},
};

module_platform_driver(gsw_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("GBit switch driver for Mediatek MT7620 SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
