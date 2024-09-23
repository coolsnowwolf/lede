#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/compiler.h>
#include <linux/clk-provider.h>
#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/bug.h>
#include <dt-bindings/clock/siflower,sf19a2890-clk.h>

#define REG_PLL_BASE		0x0
#define REG_PLL_PD		0x0
#define  PLL_PD			BIT(0)
#define  PLL_PD_VCO		BIT(1)
#define  PLL_PD_POSTDIV		BIT(2)
#define  PLL_PD_4PHASE		BIT(3)
#define  PLL_PD_DAC		BIT(4)
#define  PLL_PD_DSM		BIT(5)

/*
 * PLL_PARAM is a 48-bit value put into 6 registers, 8-bit per register:
 * REFDIV = PLL_PARAM[47:42]
 * POSTDIV2 = PLL_PARAM[41:39]
 * POSTDIV1 = PLL_PARAM[38:36]
 * FRAC = PLL_PARAM[35:12]
 * FBDIV = PLL_PARAM[11:0]
 */

#define REG_PLL_PARAM(_x)	(0x4 + (_x) * 4)
#define  PLL_REFDIV_HI		47
#define  PLL_REFDIV_LO		42
#define  PLL_POSTDIV2_HI	41
#define  PLL_POSTDIV2_LO	39
#define  PLL_POSTDIV1_HI	38
#define  PLL_POSTDIV1_LO	36
#define  PLL_FRAC_HI		35
#define  PLL_FRAC_LO		12
#define  PLL_FRAC_BITS		(PLL_FRAC_HI - PLL_FRAC_LO + 1)
#define  PLL_FBDIV_HI		11
#define  PLL_FBDIV_LO		0

#define REG_PLL_CFG 0x1c
#define  PLL_CFG_BYPASS		BIT(0)
#define  PLL_CFG_SRC		GENMASK(2, 1)
#define  PLL_CFG_OCLK_SEL	BIT(3)
#define  PLL_CFG_OCLK_GATE	BIT(4)
#define  PLL_CFG_LOAD_DIVS	BIT(5)

#define REG_PLL_LOCK 0x20

/*
 * Read-only register indicating the value of the hardware clock source
 * override pin. When the first bit of this register is set, PLL clock
 * source is forced to the 40M oscillator regardless of PLL_CFG_SRC
 * value.
 */
#define REG_PLL_SRC_OVERRIDE 0x24

struct sf_clk_common {
	void __iomem *base;
	spinlock_t *lock;
	struct clk_hw hw;
};

struct sf19a2890_clk {
	struct sf_clk_common common;
	ulong offset;
};

#define SF_CLK_COMMON(_name, _parents, _op, _flags)                           \
	{                                                                     \
		.hw.init = CLK_HW_INIT_PARENTS(_name, _parents, _op, _flags), \
	}

static inline struct sf_clk_common *hw_to_sf_clk_common(struct clk_hw *hw)
{
	return container_of(hw, struct sf_clk_common, hw);
}

static inline struct sf19a2890_clk *cmn_to_clk(struct sf_clk_common *cmn_priv)
{
	return container_of(cmn_priv, struct sf19a2890_clk, common);
}

static inline u32 sf_readl(struct sf_clk_common *priv, u32 reg)
{
	return readl(priv->base + reg);
}

static inline void sf_writel(struct sf_clk_common *priv, u32 reg, u32 val)
{
	return writel(val, priv->base + reg);
}

static inline void sf_rmw(struct sf_clk_common *priv, u32 reg, u32 clr, u32 set)
{
	u32 val = sf_readl(priv, reg);

	val &= ~clr;
	val |= set;
	sf_writel(priv, reg, val);
}

static u32 sf_pll_param_get(struct sf19a2890_clk *priv, u32 hi, u32 lo)
{
	struct sf_clk_common *cmn = &priv->common;
	u32 ret = 0;
	int reg_hi = hi / 8;
	int reg_lo = lo / 8;
	u32 reg_hi_pos = hi % 8;
	u32 reg_lo_pos = lo % 8;
	int i;

	if (reg_hi == reg_lo) {
		u32 mask = (BIT(reg_hi_pos - reg_lo_pos + 1)) - 1;
		u32 reg_val =
			sf_readl(cmn, priv->offset + REG_PLL_PARAM(reg_hi));
		return (reg_val >> reg_lo_pos) & mask;
	}

	ret = sf_readl(cmn, priv->offset + REG_PLL_PARAM(reg_hi)) &
	      (BIT(reg_hi_pos + 1) - 1);
	for (i = reg_hi - 1; i > reg_lo; i--)
		ret = (ret << 8) |
		      sf_readl(cmn, priv->offset + REG_PLL_PARAM(i));
	ret = (ret << (8 - reg_lo_pos)) |
	      (sf_readl(cmn, priv->offset + REG_PLL_PARAM(reg_lo)) >>
	       reg_lo_pos);

	return ret;
}

static unsigned long sf19a28_pll_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	u32 refdiv = sf_pll_param_get(priv, PLL_REFDIV_HI, PLL_REFDIV_LO);
	u32 fbdiv = sf_pll_param_get(priv, PLL_FBDIV_HI, PLL_FBDIV_LO);
	u32 postdiv1 = sf_pll_param_get(priv, PLL_POSTDIV1_HI, PLL_POSTDIV1_LO);
	u32 postdiv2 = sf_pll_param_get(priv, PLL_POSTDIV2_HI, PLL_POSTDIV2_LO);
	u32 pll_pd = sf_readl(cmn_priv, PLL_PD);
	u32 ref = parent_rate / refdiv;
	u32 rate = ref * fbdiv;
	u32 frac;
	u64 frac_rate;

	if (!(pll_pd & PLL_PD_DSM)) {
		frac = sf_pll_param_get(priv, PLL_FRAC_HI, PLL_FRAC_LO);
		frac_rate = ((u64)rate * frac) >> PLL_FRAC_BITS;
		rate += frac_rate;
	}
	rate = rate / postdiv1 / postdiv2;
	return rate;
}

static u8 sf19a28_pll_get_parent(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	u32 cfg;

	if (sf_readl(cmn_priv, priv->offset + REG_PLL_SRC_OVERRIDE))
		return 1;
	cfg = sf_readl(cmn_priv, priv->offset + REG_PLL_CFG);
	return (FIELD_GET(PLL_CFG_SRC, cfg) == 1);
}

static const struct clk_ops sf19a28_pll_ops = {
	.recalc_rate = sf19a28_pll_recalc_rate,
	.get_parent = sf19a28_pll_get_parent,
};

static const char * const clk_pll_parents[] = { "osc12m", "osc40m" };

#define SF19A28_PLL(_name, _offset, _flags)				\
	struct sf19a2890_clk _name = {					\
		.common = SF_CLK_COMMON(#_name, clk_pll_parents,	\
					&sf19a28_pll_ops, _flags),	\
		.offset = REG_PLL_BASE + _offset,			\
	}

static SF19A28_PLL(pll_cpu, 0x0, 0);
static SF19A28_PLL(pll_ddr, 0x40, 0);
static SF19A28_PLL(pll_cmn, 0x80, 0);

#define REG_MUXDIV_BASE		0x400
#define REG_MUXDIV_CFG		0x0
#define  MUXDIV_USE_NCO		BIT(3)
#define  MUXDIV_SRC_SEL		GENMASK(2, 0)
#define REG_MUXDIV_RATIO	0x4
#define  MUXDIV_RATIO_MAX	0xff
#define REG_MUXDIV_NCO_V	0x8
#define REG_MUXDIV_EN		0xc
#define REG_MUXDIV_XN_DIV_RATIO	0x10
#define  MUXDIV_XN_DIV_MAX	3

static unsigned long sf19a28_muxdiv_recalc_rate(struct clk_hw *hw,
						unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	u32 div;

	div = sf_readl(cmn_priv, priv->offset + REG_MUXDIV_RATIO) + 1;
	return parent_rate / div;
}

int sf19a28_muxdiv_determine_rate(struct clk_hw *hw, struct clk_rate_request *req)
{
	unsigned int div;

	div = DIV_ROUND_CLOSEST(req->best_parent_rate, req->rate);
	if (!div)
		div = 1;
	else if (div > MUXDIV_RATIO_MAX + 1)
		div = MUXDIV_RATIO_MAX + 1;

	req->rate = req->best_parent_rate / div;
	return 0;
}

static int sf19a28_muxdiv_set_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	unsigned int div;

	div = DIV_ROUND_CLOSEST(parent_rate, rate);
	if (div < 1)
		div = 1;
	else if (div > MUXDIV_RATIO_MAX + 1)
		div = MUXDIV_RATIO_MAX + 1;
	div -= 1;

	sf_writel(cmn_priv, priv->offset + REG_MUXDIV_RATIO, div);

	return 0;
}

static int sf19a28_muxdiv_enable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);

	sf_writel(cmn_priv, priv->offset + REG_MUXDIV_EN, 1);
	return 0;
}

static void sf19a28_muxdiv_disable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);

	sf_writel(cmn_priv, priv->offset + REG_MUXDIV_EN, 0);
}

static int sf19a28_muxdiv_is_enabled(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);

	return !!sf_readl(cmn_priv, priv->offset + REG_MUXDIV_EN);
}

static u8 sf19a28_muxdiv_get_parent(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	u32 cfg = sf_readl(cmn_priv, priv->offset + REG_MUXDIV_CFG);
	u32 src = FIELD_GET(MUXDIV_SRC_SEL, cfg);

	if (src <= 2)
		return src;
	if (src == 4)
		return 3;
	return 4;
}

static int sf19a28_muxdiv_set_parent(struct clk_hw *hw, u8 index)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf19a2890_clk *priv = cmn_to_clk(cmn_priv);
	u32 src;

	if (index <= 2)
		src = index;
	else if (index == 3)
		src = 4;
	else
		src = 6;
	sf_writel(cmn_priv, priv->offset + REG_MUXDIV_CFG, src);
	return 0;
}

static const char * const clk_muxdiv_parents[] = { "pll_cpu", "pll_ddr", "pll_cmn",
					    "osc12m", "osc40m" };

static const struct clk_ops sf19a28_muxdiv_ops = {
	.recalc_rate = sf19a28_muxdiv_recalc_rate,
	.determine_rate = sf19a28_muxdiv_determine_rate,
	.set_rate = sf19a28_muxdiv_set_rate,
	.enable = sf19a28_muxdiv_enable,
	.disable = sf19a28_muxdiv_disable,
	.is_enabled = sf19a28_muxdiv_is_enabled,
	.get_parent = sf19a28_muxdiv_get_parent,
	.set_parent = sf19a28_muxdiv_set_parent,
};

#define SF19A28_MUXDIV(_name, _offset, _flags)				\
	struct sf19a2890_clk _name = {					\
		.common = SF_CLK_COMMON(#_name, clk_muxdiv_parents,	\
					&sf19a28_muxdiv_ops, _flags),	\
		.offset = REG_MUXDIV_BASE + _offset,			\
	}

static SF19A28_MUXDIV(muxdiv_bus1, 0x0, CLK_IS_CRITICAL);
static SF19A28_MUXDIV(muxdiv_bus2, 0x20, CLK_IS_CRITICAL);
static SF19A28_MUXDIV(muxdiv_bus3, 0x40, CLK_IS_CRITICAL);
static SF19A28_MUXDIV(muxdiv_cpu, 0x100, CLK_SET_RATE_PARENT | CLK_SET_RATE_NO_REPARENT);
static SF19A28_MUXDIV(muxdiv_pbus, 0x120, CLK_IS_CRITICAL);
static SF19A28_MUXDIV(muxdiv_mem_phy, 0x140, CLK_IS_CRITICAL);
static SF19A28_MUXDIV(muxdiv_uart, 0x180, 0);
static SF19A28_MUXDIV(muxdiv_eth_ref, 0x200, 0);
static SF19A28_MUXDIV(muxdiv_eth_byp_ref, 0x220, 0);
static SF19A28_MUXDIV(muxdiv_eth_tsu, 0x240, 0);
static SF19A28_MUXDIV(muxdiv_gmac_byp_ref, 0x260, 0);
static SF19A28_MUXDIV(muxdiv_m6250_0, 0x280, 0);
static SF19A28_MUXDIV(muxdiv_m6250_1, 0x2a0, 0);
static SF19A28_MUXDIV(muxdiv_wlan24_plf, 0x2c0, 0);
static SF19A28_MUXDIV(muxdiv_wlan5_plf, 0x2e0, 0);
static SF19A28_MUXDIV(muxdiv_usbphy_ref, 0x300, 0);
static SF19A28_MUXDIV(muxdiv_tclk, 0x320, 0);
static SF19A28_MUXDIV(muxdiv_npu_pe, 0x340, 0);

static struct clk_hw_onecell_data sf19a2890_hw_clks = {
	.num = CLK_SF19A2890_MAX,
	.hws = {
		[CLK_PLL_CPU] = &pll_cpu.common.hw,
		[CLK_PLL_DDR] = &pll_ddr.common.hw,
		[CLK_PLL_CMN] = &pll_cmn.common.hw,
		[CLK_MUXDIV_BUS1] = &muxdiv_bus1.common.hw,
		[CLK_MUXDIV_BUS2] = &muxdiv_bus2.common.hw,
		[CLK_MUXDIV_BUS3] = &muxdiv_bus3.common.hw,
		[CLK_MUXDIV_CPU] = &muxdiv_cpu.common.hw,
		[CLK_MUXDIV_PBUS] = &muxdiv_pbus.common.hw,
		[CLK_MUXDIV_MEM_PHY] = &muxdiv_mem_phy.common.hw,
		[CLK_MUXDIV_UART] = &muxdiv_uart.common.hw,
		[CLK_MUXDIV_ETH_REF] = &muxdiv_eth_ref.common.hw,
		[CLK_MUXDIV_ETH_BYP_REF] = &muxdiv_eth_byp_ref.common.hw,
		[CLK_MUXDIV_ETH_TSU] = &muxdiv_eth_tsu.common.hw,
		[CLK_MUXDIV_GMAC_BYP_REF] = &muxdiv_gmac_byp_ref.common.hw,
		[CLK_MUXDIV_M6250_0] = &muxdiv_m6250_0.common.hw,
		[CLK_MUXDIV_M6250_1] = &muxdiv_m6250_1.common.hw,
		[CLK_MUXDIV_WLAN24_PLF] = &muxdiv_wlan24_plf.common.hw,
		[CLK_MUXDIV_WLAN5_PLF] = &muxdiv_wlan5_plf.common.hw,
		[CLK_MUXDIV_USBPHY_REF] = &muxdiv_usbphy_ref.common.hw,
		[CLK_MUXDIV_TCLK] = &muxdiv_tclk.common.hw,
		[CLK_MUXDIV_NPU_PE_CLK] = &muxdiv_npu_pe.common.hw,
	},
};


struct sf19a2890_clk_ctrl {
	void __iomem *base;
	spinlock_t lock;
};

static void __init sf19a2890_clk_init(struct device_node *node)
{
	struct sf19a2890_clk_ctrl *ctrl;
	int i, ret;

	ctrl = kzalloc(sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return;

	ctrl->base = of_iomap(node, 0);
	if (!ctrl->base) {
		pr_err("failed to map resources.\n");
		return;
	}

	spin_lock_init(&ctrl->lock);

	for (i = 0; i < sf19a2890_hw_clks.num; i++) {
		struct clk_hw *hw = sf19a2890_hw_clks.hws[i];
		struct sf_clk_common *common;

		if (!hw)
			continue;
		common = hw_to_sf_clk_common(hw);
		common->base = ctrl->base;
		common->lock = &ctrl->lock;
		ret = clk_hw_register(NULL, hw);
		if (ret) {
			pr_err("Couldn't register clock %d: %d\n", i, ret);
			goto err;
		}
	}

	ret = of_clk_add_hw_provider(node, of_clk_hw_onecell_get, &sf19a2890_hw_clks);
	if (ret) {
		pr_err("failed to add hw provider.\n");
		goto err;
	}
	return;
err:
	iounmap(ctrl->base);
}

CLK_OF_DECLARE(sf19a2890_clk, "siflower,sf19a2890-clk", sf19a2890_clk_init);
