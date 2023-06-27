// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021 MediaTek Inc.
 * Author: Sam Shih <sam.shih@mediatek.com>
 * Author: Wenzhen Yu <wenzhen.yu@mediatek.com>
 * Author: Jianhui Zhao <zhaojh329@gmail.com>
 * Author: Daniel Golle <daniel@makrotopia.org>
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include "clk-mtk.h"
#include "clk-gate.h"

#include <dt-bindings/clock/mediatek,mt7981-clk.h>

static const struct mtk_gate_regs sgmii0_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII0(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &sgmii0_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate sgmii0_clks[] __initconst = {
	GATE_SGMII0(CLK_SGM0_TX_EN, "sgm0_tx_en", "usb_tx250m", 2),
	GATE_SGMII0(CLK_SGM0_RX_EN, "sgm0_rx_en", "usb_eq_rx250m", 3),
	GATE_SGMII0(CLK_SGM0_CK0_EN, "sgm0_ck0_en", "usb_ln0", 4),
	GATE_SGMII0(CLK_SGM0_CDR_CK0_EN, "sgm0_cdr_ck0_en", "usb_cdr", 5),
};

static const struct mtk_gate_regs sgmii1_cg_regs = {
	.set_ofs = 0xE4,
	.clr_ofs = 0xE4,
	.sta_ofs = 0xE4,
};

#define GATE_SGMII1(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &sgmii1_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate sgmii1_clks[] __initconst = {
	GATE_SGMII1(CLK_SGM1_TX_EN, "sgm1_tx_en", "usb_tx250m", 2),
	GATE_SGMII1(CLK_SGM1_RX_EN, "sgm1_rx_en", "usb_eq_rx250m", 3),
	GATE_SGMII1(CLK_SGM1_CK1_EN, "sgm1_ck1_en", "usb_ln0", 4),
	GATE_SGMII1(CLK_SGM1_CDR_CK1_EN, "sgm1_cdr_ck1_en", "usb_cdr", 5),
};

static const struct mtk_gate_regs eth_cg_regs = {
	.set_ofs = 0x30,
	.clr_ofs = 0x30,
	.sta_ofs = 0x30,
};

#define GATE_ETH(_id, _name, _parent, _shift) {	\
		.id = _id,				\
		.name = _name,				\
		.parent_name = _parent,			\
		.regs = &eth_cg_regs,			\
		.shift = _shift,			\
		.ops = &mtk_clk_gate_ops_no_setclr_inv,	\
	}

static const struct mtk_gate eth_clks[] __initconst = {
	GATE_ETH(CLK_ETH_FE_EN, "eth_fe_en", "netsys_2x", 6),
	GATE_ETH(CLK_ETH_GP2_EN, "eth_gp2_en", "sgm_325m", 7),
	GATE_ETH(CLK_ETH_GP1_EN, "eth_gp1_en", "sgm_325m", 8),
	GATE_ETH(CLK_ETH_WOCPU0_EN, "eth_wocpu0_en", "netsys_wed_mcu", 15),
};

static void __init mtk_sgmiisys_0_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(sgmii0_clks));

	mtk_clk_register_gates(node, sgmii0_clks, ARRAY_SIZE(sgmii0_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);
	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_0, "mediatek,mt7981-sgmiisys_0",
	       mtk_sgmiisys_0_init);

static void __init mtk_sgmiisys_1_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(sgmii1_clks));

	mtk_clk_register_gates(node, sgmii1_clks, ARRAY_SIZE(sgmii1_clks),
			       clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_sgmiisys_1, "mediatek,mt7981-sgmiisys_1",
	       mtk_sgmiisys_1_init);

static void __init mtk_ethsys_init(struct device_node *node)
{
	struct clk_onecell_data *clk_data;
	int r;

	clk_data = mtk_alloc_clk_data(ARRAY_SIZE(eth_clks));

	mtk_clk_register_gates(node, eth_clks, ARRAY_SIZE(eth_clks), clk_data);

	r = of_clk_add_provider(node, of_clk_src_onecell_get, clk_data);

	if (r)
		pr_err("%s(): could not register clock provider: %d\n",
		       __func__, r);
}
CLK_OF_DECLARE(mtk_ethsys, "mediatek,mt7981-ethsys", mtk_ethsys_init);
