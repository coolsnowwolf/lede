/*
 * Copyright (C) 2010 Broadcom
 * Copyright (C) 2012 Stephen Warren
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/stringify.h>
#include <linux/reset.h>
#include <linux/io.h>
#include <mach/hardware.h>
#include <mach/utils.h>

#define MHZ (1000 * 1000)

struct clk_oxnas_pllb {
	struct clk_hw hw;
	struct device_node *devnode;
	struct reset_control *rstc;
};

#define to_clk_oxnas_pllb(_hw) container_of(_hw, struct clk_oxnas_pllb, hw)

static unsigned long plla_clk_recalc_rate(struct clk_hw *hw,
	unsigned long parent_rate)
{
	unsigned long fin = parent_rate;
	unsigned long pll0;
	unsigned long fbdiv, refdiv, outdiv;

	pll0 = readl_relaxed(SYS_CTRL_PLLA_CTRL0);
	refdiv = (pll0 >> PLLA_REFDIV_SHIFT) & PLLA_REFDIV_MASK;
	refdiv += 1;
	outdiv = (pll0 >> PLLA_OUTDIV_SHIFT) & PLLA_OUTDIV_MASK;
	outdiv += 1;
	fbdiv = readl_relaxed(SYS_CTRL_PLLA_CTRL1);

	/* seems we will not be here when pll is bypassed, so ignore this
	 * case */

	return fin / MHZ * fbdiv / (refdiv * outdiv) / 32768 * MHZ;
}

static const char *pll_clk_parents[] = {
	"oscillator",
};

static struct clk_ops plla_ops = {
	.recalc_rate = plla_clk_recalc_rate,
};

static struct clk_init_data clk_plla_init = {
	.name = "plla",
	.ops = &plla_ops,
	.parent_names = pll_clk_parents,
	.num_parents = ARRAY_SIZE(pll_clk_parents),
};

static struct clk_hw plla_hw = {
	.init = &clk_plla_init,
};

static int pllb_clk_is_prepared(struct clk_hw *hw)
{
	struct clk_oxnas_pllb *pllb = to_clk_oxnas_pllb(hw);

	return !!pllb->rstc;
}

static int pllb_clk_prepare(struct clk_hw *hw)
{
	struct clk_oxnas_pllb *pllb = to_clk_oxnas_pllb(hw);

	pllb->rstc = of_reset_control_get(pllb->devnode, NULL);

	return IS_ERR(pllb->rstc) ? PTR_ERR(pllb->rstc) : 0;
}

static void pllb_clk_unprepare(struct clk_hw *hw)
{
	struct clk_oxnas_pllb *pllb = to_clk_oxnas_pllb(hw);

	BUG_ON(IS_ERR(pllb->rstc));

	reset_control_put(pllb->rstc);
	pllb->rstc = NULL;
}

static int pllb_clk_enable(struct clk_hw *hw)
{
	struct clk_oxnas_pllb *pllb = to_clk_oxnas_pllb(hw);

	BUG_ON(IS_ERR(pllb->rstc));

	/* put PLL into bypass */
	oxnas_register_set_mask(SEC_CTRL_PLLB_CTRL0, BIT(PLLB_BYPASS));
	wmb();
	udelay(10);
	reset_control_assert(pllb->rstc);
	udelay(10);
	/* set PLL B control information */
	writel((1 << PLLB_ENSAT) | (1 << PLLB_OUTDIV) | (2 << PLLB_REFDIV),
				SEC_CTRL_PLLB_CTRL0);
	reset_control_deassert(pllb->rstc);
	udelay(100);
	oxnas_register_clear_mask(SEC_CTRL_PLLB_CTRL0, BIT(PLLB_BYPASS));

	return 0;
}

static void pllb_clk_disable(struct clk_hw *hw)
{
	struct clk_oxnas_pllb *pllb = to_clk_oxnas_pllb(hw);

	BUG_ON(IS_ERR(pllb->rstc));

	/* put PLL into bypass */
	oxnas_register_set_mask(SEC_CTRL_PLLB_CTRL0, BIT(PLLB_BYPASS));
	wmb();
	udelay(10);

	reset_control_assert(pllb->rstc);
}

static struct clk_ops pllb_ops = {
	.prepare = pllb_clk_prepare,
	.unprepare = pllb_clk_unprepare,
	.is_prepared = pllb_clk_is_prepared,
	.enable = pllb_clk_enable,
	.disable = pllb_clk_disable,
};

static struct clk_init_data clk_pllb_init = {
	.name = "pllb",
	.ops = &pllb_ops,
	.parent_names = pll_clk_parents,
	.num_parents = ARRAY_SIZE(pll_clk_parents),
};


/* standard gate clock */
struct clk_std {
	struct clk_hw hw;
	signed char bit;
};

#define NUM_STD_CLKS 17
#define to_stdclk(_hw) container_of(_hw, struct clk_std, hw)

static int std_clk_is_enabled(struct clk_hw *hw)
{
	struct clk_std *std = to_stdclk(hw);

	return readl_relaxed(SYSCTRL_CLK_STAT) & BIT(std->bit);
}

static int std_clk_enable(struct clk_hw *hw)
{
	struct clk_std *std = to_stdclk(hw);

	writel(BIT(std->bit), SYS_CTRL_CLK_SET_CTRL);
	return 0;
}

static void std_clk_disable(struct clk_hw *hw)
{
	struct clk_std *std = to_stdclk(hw);

	writel(BIT(std->bit), SYS_CTRL_CLK_CLR_CTRL);
}

static struct clk_ops std_clk_ops = {
	.enable = std_clk_enable,
	.disable = std_clk_disable,
	.is_enabled = std_clk_is_enabled,
};

static const char *std_clk_parents[] = {
	"oscillator",
};

static const char *eth_parents[] = {
	"gmacclk",
};

#define DECLARE_STD_CLKP(__clk, __bit, __parent)	\
static struct clk_init_data clk_##__clk##_init = {	\
	.name = __stringify(__clk),			\
	.ops = &std_clk_ops,				\
	.parent_names = __parent,		\
	.num_parents = ARRAY_SIZE(__parent),	\
};							\
							\
static struct clk_std clk_##__clk = {			\
	.bit = __bit,					\
	.hw = {						\
		.init = &clk_##__clk##_init,		\
	},						\
}

#define DECLARE_STD_CLK(__clk, __bit) DECLARE_STD_CLKP(__clk, __bit, \
							std_clk_parents)

DECLARE_STD_CLK(leon, 0);
DECLARE_STD_CLK(dma_sgdma, 1);
DECLARE_STD_CLK(cipher, 2);
DECLARE_STD_CLK(sd, 3);
DECLARE_STD_CLK(sata, 4);
DECLARE_STD_CLK(audio, 5);
DECLARE_STD_CLK(usbmph, 6);
DECLARE_STD_CLKP(etha, 7, eth_parents);
DECLARE_STD_CLK(pciea, 8);
DECLARE_STD_CLK(static, 9);
DECLARE_STD_CLK(ethb, 10);
DECLARE_STD_CLK(pcieb, 11);
DECLARE_STD_CLK(ref600, 12);
DECLARE_STD_CLK(usbdev, 13);

struct clk_hw *std_clk_hw_tbl[] = {
	&clk_leon.hw,
	&clk_dma_sgdma.hw,
	&clk_cipher.hw,
	&clk_sd.hw,
	&clk_sata.hw,
	&clk_audio.hw,
	&clk_usbmph.hw,
	&clk_etha.hw,
	&clk_pciea.hw,
	&clk_static.hw,
	&clk_ethb.hw,
	&clk_pcieb.hw,
	&clk_ref600.hw,
	&clk_usbdev.hw,
};

struct clk *std_clk_tbl[ARRAY_SIZE(std_clk_hw_tbl)];

static struct clk_onecell_data std_clk_data;

void __init oxnas_init_stdclk(struct device_node *np)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(std_clk_hw_tbl); i++) {
		std_clk_tbl[i] = clk_register(NULL, std_clk_hw_tbl[i]);
		BUG_ON(IS_ERR(std_clk_tbl[i]));
	}
	std_clk_data.clks = std_clk_tbl;
	std_clk_data.clk_num = ARRAY_SIZE(std_clk_tbl);
	of_clk_add_provider(np, of_clk_src_onecell_get, &std_clk_data);
}
CLK_OF_DECLARE(oxnas_pllstd, "plxtech,nas782x-stdclk", oxnas_init_stdclk);

void __init oxnas_init_plla(struct device_node *np)
{
	struct clk *clk;

	clk = clk_register(NULL, &plla_hw);
	BUG_ON(IS_ERR(clk));
	/* mark it as enabled */
	clk_prepare_enable(clk);
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
}
CLK_OF_DECLARE(oxnas_plla, "plxtech,nas782x-plla", oxnas_init_plla);

void __init oxnas_init_pllb(struct device_node *np)
{
	struct clk *clk;
	struct clk_oxnas_pllb *pllb;

	pllb = kmalloc(sizeof(*pllb), GFP_KERNEL);
	BUG_ON(!pllb);

	pllb->hw.init = &clk_pllb_init;
	pllb->devnode = np;
	pllb->rstc = NULL;

	clk = clk_register(NULL, &pllb->hw);
	BUG_ON(IS_ERR(clk));
	of_clk_add_provider(np, of_clk_src_simple_get, clk);
}
CLK_OF_DECLARE(oxnas_pllb, "plxtech,nas782x-pllb", oxnas_init_pllb);
