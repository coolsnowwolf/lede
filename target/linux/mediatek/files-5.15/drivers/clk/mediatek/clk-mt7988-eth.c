// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2023 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 * Author: Xiufeng Li <Xiufeng.Li@mediatek.com>
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include "clk-mtk.h"
#include "clk-gate.h"
#include <dt-bindings/clock/mediatek,mt7988-clk.h>

static const struct mtk_gate_regs ethdma_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETHDMA(_id, _name, _parent, _shift)                              \
	{                                                                     \
		.id = _id, .name = _name, .parent_name = _parent,             \
		.regs = &ethdma_cg_regs, .shift = _shift,                     \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                       \
	}

static const struct mtk_gate ethdma_clks[] = {
	GATE_ETHDMA(CLK_ETHDMA_XGP1_EN, "ethdma_xgp1_en", "top_xtal", 0),
	GATE_ETHDMA(CLK_ETHDMA_XGP2_EN, "ethdma_xgp2_en", "top_xtal", 1),
	GATE_ETHDMA(CLK_ETHDMA_XGP3_EN, "ethdma_xgp3_en", "top_xtal", 2),
	GATE_ETHDMA(CLK_ETHDMA_FE_EN, "ethdma_fe_en", "netsys_2x_sel", 6),
	GATE_ETHDMA(CLK_ETHDMA_GP2_EN, "ethdma_gp2_en", "top_xtal", 7),
	GATE_ETHDMA(CLK_ETHDMA_GP1_EN, "ethdma_gp1_en", "top_xtal", 8),
	GATE_ETHDMA(CLK_ETHDMA_GP3_EN, "ethdma_gp3_en", "top_xtal", 10),
	GATE_ETHDMA(CLK_ETHDMA_ESW_EN, "ethdma_esw_en", "netsys_gsw_sel", 16),
	GATE_ETHDMA(CLK_ETHDMA_CRYPT0_EN, "ethdma_crypt0_en", "eip197_sel",
		    29),
};

static int clk_mt7988_ethsys_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return -ENOMEM;
	}

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(ethdma_clks));

	if (!clk_data)
		return -ENOMEM;

	mtk_clk_register_gates(node, ethdma_clks, ARRAY_SIZE(ethdma_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	if (r) {
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
		goto free_data;
	}
	return r;

free_data:
	mtk_free_clk_data(clk_data);
	return r;
}

static const struct mtk_gate_regs sgmii0_cg_regs = {
	.set_ofs = 0xe4,
	.clr_ofs = 0xe4,
	.sta_ofs = 0xe4,
};

#define GATE_SGMII0(_id, _name, _parent, _shift)                              \
	{                                                                     \
		.id = _id, .name = _name, .parent_name = _parent,             \
		.regs = &sgmii0_cg_regs, .shift = _shift,                     \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                       \
	}

static const struct mtk_gate sgmii0_clks[] = {
	GATE_SGMII0(CLK_SGM0_TX_EN, "sgm0_tx_en", "top_xtal", 2),
	GATE_SGMII0(CLK_SGM0_RX_EN, "sgm0_rx_en", "top_xtal", 3),
};

static int clk_mt7988_sgmii0_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return -ENOMEM;
	}

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(sgmii0_clks));

	if (!clk_data)
		return -ENOMEM;

	mtk_clk_register_gates(node, sgmii0_clks, ARRAY_SIZE(sgmii0_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	if (r) {
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
		goto free_data;
	}
	return r;

free_data:
	mtk_free_clk_data(clk_data);
	return r;
}

static const struct mtk_gate_regs sgmii1_cg_regs = {
	.set_ofs = 0xe4,
	.clr_ofs = 0xe4,
	.sta_ofs = 0xe4,
};

#define GATE_SGMII1(_id, _name, _parent, _shift)                              \
	{                                                                     \
		.id = _id, .name = _name, .parent_name = _parent,             \
		.regs = &sgmii1_cg_regs, .shift = _shift,                     \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                       \
	}

static const struct mtk_gate sgmii1_clks[] = {
	GATE_SGMII1(CLK_SGM1_TX_EN, "sgm1_tx_en", "top_xtal", 2),
	GATE_SGMII1(CLK_SGM1_RX_EN, "sgm1_rx_en", "top_xtal", 3),
};

static int clk_mt7988_sgmii1_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return -ENOMEM;
	}

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(sgmii1_clks));

	if (!clk_data)
		return -ENOMEM;

	mtk_clk_register_gates(node, sgmii1_clks, ARRAY_SIZE(sgmii1_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	if (r) {
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
		goto free_data;
	}
	return r;

free_data:
	mtk_free_clk_data(clk_data);
	return r;
}

static const struct mtk_gate_regs ethwarp_cg_regs = {
	.set_ofs = 0x14,
	.clr_ofs = 0x14,
	.sta_ofs = 0x14,
};

#define GATE_ETHWARP(_id, _name, _parent, _shift)                             \
	{                                                                     \
		.id = _id, .name = _name, .parent_name = _parent,             \
		.regs = &ethwarp_cg_regs, .shift = _shift,                    \
		.ops = &mtk_clk_gate_ops_no_setclr_inv,                       \
	}

static const struct mtk_gate ethwarp_clks[] = {
	GATE_ETHWARP(CLK_ETHWARP_WOCPU2_EN, "ethwarp_wocpu2_en",
		     "netsys_mcu_sel", 13),
	GATE_ETHWARP(CLK_ETHWARP_WOCPU1_EN, "ethwarp_wocpu1_en",
		     "netsys_mcu_sel", 14),
	GATE_ETHWARP(CLK_ETHWARP_WOCPU0_EN, "ethwarp_wocpu0_en",
		     "netsys_mcu_sel", 15),
};

static int clk_mt7988_ethwarp_probe(struct platform_device *pdev)
{
	struct clk_onecell_data *clk_data;
	struct device_node *node = pdev->dev.of_node;
	int r;
	void __iomem *base;

	base = of_iomap(node, 0);
	if (!base) {
		pr_err("%s(): ioremap failed\n", __func__);
		return -ENOMEM;
	}

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(ethwarp_clks));

	if (!clk_data)
		return -ENOMEM;

	mtk_clk_register_gates(node, ethwarp_clks, ARRAY_SIZE(ethwarp_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	if (r) {
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
		goto free_data;
	}
	return r;

free_data:
	mtk_free_clk_data(clk_data);
	return r;
}

static const struct of_device_id of_match_clk_mt7988_ethsys[] = {
	{
		.compatible = "mediatek,mt7988-ethsys",
	},
	{}
};

static struct platform_driver clk_mt7988_ethsys_drv = {
	.probe = clk_mt7988_ethsys_probe,
	.driver = {
		.name = "clk-mt7988-ethsys",
		.of_match_table = of_match_clk_mt7988_ethsys,
	},
};
builtin_platform_driver(clk_mt7988_ethsys_drv);

static const struct of_device_id of_match_clk_mt7988_sgmii0[] = {
	{
		.compatible = "mediatek,mt7988-sgmiisys_0",
	},
	{}
};

static struct platform_driver clk_mt7988_sgmii0_drv = {
	.probe = clk_mt7988_sgmii0_probe,
	.driver = {
		.name = "clk-mt7988-sgmiisys_0",
		.of_match_table = of_match_clk_mt7988_sgmii0,
	},
};
builtin_platform_driver(clk_mt7988_sgmii0_drv);

static const struct of_device_id of_match_clk_mt7988_sgmii1[] = {
	{
		.compatible = "mediatek,mt7988-sgmiisys_1",
	},
	{}
};

static struct platform_driver clk_mt7988_sgmii1_drv = {
	.probe = clk_mt7988_sgmii1_probe,
	.driver = {
		.name = "clk-mt7988-sgmiisys_1",
		.of_match_table = of_match_clk_mt7988_sgmii1,
	},
};
builtin_platform_driver(clk_mt7988_sgmii1_drv);

static const struct of_device_id of_match_clk_mt7988_ethwarp[] = {
	{
		.compatible = "mediatek,mt7988-ethwarp",
	},
	{}
};

static struct platform_driver clk_mt7988_ethwarp_drv = {
	.probe = clk_mt7988_ethwarp_probe,
	.driver = {
		.name = "clk-mt7988-ethwarp",
		.of_match_table = of_match_clk_mt7988_ethwarp,
	},
};
builtin_platform_driver(clk_mt7988_ethwarp_drv);