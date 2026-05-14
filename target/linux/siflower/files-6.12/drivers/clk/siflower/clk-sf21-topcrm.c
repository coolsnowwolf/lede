// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/compiler.h>
#include <linux/clk-provider.h>
#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/rational.h>
#include <linux/spinlock.h>
#include <linux/bug.h>
#include <dt-bindings/clock/siflower,sf21-topcrm.h>

struct sf_clk_common {
	void __iomem	*base;
	spinlock_t	*lock;
	struct clk_hw	hw;
};

#define SF_CLK_COMMON(_name, _parents, _op, _flags)			\
	{								\
		.hw.init = CLK_HW_INIT_PARENTS(_name, _parents,		\
						    _op, _flags),	\
	}

static inline struct sf_clk_common *hw_to_sf_clk_common(struct clk_hw *hw)
{
	return container_of(hw, struct sf_clk_common, hw);
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
	u32 val;

	val = sf_readl(priv, reg);
	val &= ~clr;
	val |= set;
	sf_writel(priv, reg, val);
}

#define PLL_CMN_CFG1		0x0
#define  PLL_CMN_BYPASS		BIT(27)
#define  PLL_CMN_PD		BIT(26)
#define  PLL_CMN_FBDIV		GENMASK(25, 14)
#define  PLL_CMN_FBDIV_BITS	(25 - 14 + 1)
#define  PLL_CMN_POSTDIV_PD	BIT(13)
#define  PLL_CMN_VCO_PD		BIT(12)
#define  PLL_CMN_POSTDIV1	GENMASK(11, 9)
#define  PLL_CMN_POSTDIV2	GENMASK(8, 6)
#define  PLL_CMN_REFDIV		GENMASK(5, 0)
#define  PLL_CMN_REFDIV_BITS	6

#define PLL_CMN_LOCK		0xc8
#define PLL_DDR_LOCK		0xcc
#define PLL_PCIE_LOCK		0xd4

#define CFG_LOAD		0x100
#define  CFG_LOAD_PCIE_PLL	BIT(4)
#define  CFG_LOAD_DDR_PLL	BIT(2)
#define  CFG_LOAD_CMN_PLL	BIT(1)
#define  CFG_LOAD_DIV		BIT(0)

static unsigned long sf21_cmnpll_vco_recalc_rate(struct clk_hw *hw,
						      unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	u32 cfg = sf_readl(priv, PLL_CMN_CFG1);
	unsigned long refdiv = FIELD_GET(PLL_CMN_REFDIV, cfg);
	unsigned long fbdiv = FIELD_GET(PLL_CMN_FBDIV, cfg);

	return (parent_rate / refdiv) * fbdiv;
}

static long sf21_cmnpll_vco_round_rate(struct clk_hw *hw,
					    unsigned long rate,
					    unsigned long *parent_rate)
{
	unsigned long fbdiv, refdiv;

	rational_best_approximation(rate, *parent_rate,
				    BIT(PLL_CMN_FBDIV_BITS) - 1,
				    BIT(PLL_CMN_REFDIV_BITS) - 1, &fbdiv,
				    &refdiv);
	return (*parent_rate / refdiv) * fbdiv;
}

static int sf21_cmnpll_vco_set_rate(struct clk_hw *hw, unsigned long rate,
					 unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	unsigned long flags;
	unsigned long fbdiv, refdiv;

	rational_best_approximation(rate, parent_rate,
				    BIT(PLL_CMN_FBDIV_BITS) - 1,
				    BIT(PLL_CMN_REFDIV_BITS) - 1, &fbdiv,
				    &refdiv);

	spin_lock_irqsave(priv->lock, flags);

	sf_rmw(priv, PLL_CMN_CFG1, 0, PLL_CMN_BYPASS);
	sf_writel(priv, CFG_LOAD, CFG_LOAD_CMN_PLL);
	sf_writel(priv, CFG_LOAD, 0);

	sf_rmw(priv, PLL_CMN_CFG1, PLL_CMN_REFDIV | PLL_CMN_FBDIV | PLL_CMN_PD,
	       FIELD_PREP(PLL_CMN_REFDIV, refdiv) |
		       FIELD_PREP(PLL_CMN_FBDIV, fbdiv));
	sf_writel(priv, CFG_LOAD, CFG_LOAD_CMN_PLL);
	sf_writel(priv, CFG_LOAD, 0);

	while (!(sf_readl(priv, PLL_CMN_LOCK) & 1))
		cpu_relax();

	sf_rmw(priv, PLL_CMN_CFG1, PLL_CMN_BYPASS, 0);
	sf_writel(priv, CFG_LOAD, CFG_LOAD_CMN_PLL);
	sf_writel(priv, CFG_LOAD, 0);

	spin_unlock_irqrestore(priv->lock, flags);
	return 0;
}

static const struct clk_ops sf21_cmnpll_vco_ops = {
	.recalc_rate = sf21_cmnpll_vco_recalc_rate,
	.round_rate = sf21_cmnpll_vco_round_rate,
	.set_rate = sf21_cmnpll_vco_set_rate,
};

static const char *const clk_pll_parents[] = { "xin25m" };

static struct sf_clk_common cmnpll_vco = SF_CLK_COMMON(
	"cmnpll_vco", clk_pll_parents, &sf21_cmnpll_vco_ops, 0);

static unsigned long sf21_dualdiv_round_rate(
	unsigned long rate, unsigned long parent_rate,
	unsigned int range, unsigned int *diva, unsigned int *divb)
{
	unsigned int div = DIV_ROUND_CLOSEST(parent_rate, rate);
	unsigned int best_diff, da, db, cur_div, cur_diff;

	if (div <= 1) {
		*diva = 1;
		*divb = 1;
		return parent_rate;
	}

	best_diff = div - 1;
	*diva = 1;
	*divb = 1;

	for (da = 1; da <= range; da++) {
		db = DIV_ROUND_CLOSEST(div, da);
		if (db > da)
			db = da;

		cur_div = da * db;
		if (div > cur_div)
			cur_diff = div - cur_div;
		else
			cur_diff = cur_div - div;

		if (cur_diff < best_diff) {
			best_diff = cur_diff;
			*diva = da;
			*divb = db;
		}
		if (cur_diff == 0)
			break;
	}

	return parent_rate / *diva / *divb;
}

static long sf21_cmnpll_postdiv_round_rate(struct clk_hw *hw,
						unsigned long rate,
						unsigned long *parent_rate)
{
	unsigned int diva, divb;

	return sf21_dualdiv_round_rate(rate, *parent_rate, 7, &diva,
					    &divb);
}

static int sf21_cmnpll_postdiv_set_rate(struct clk_hw *hw,
					     unsigned long rate,
					     unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	unsigned int diva, divb;
	unsigned long flags;

	sf21_dualdiv_round_rate(rate, parent_rate, 7, &diva, &divb);

	spin_lock_irqsave(priv->lock, flags);
	sf_rmw(priv, PLL_CMN_CFG1, PLL_CMN_POSTDIV1 | PLL_CMN_POSTDIV2,
	       FIELD_PREP(PLL_CMN_POSTDIV1, diva) |
		       FIELD_PREP(PLL_CMN_POSTDIV2, divb));
	sf_writel(priv, CFG_LOAD, CFG_LOAD_CMN_PLL);
	sf_writel(priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(priv->lock, flags);
	return 0;
}

static unsigned long
sf21_cmnpll_postdiv_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	u32 cfg = sf_readl(priv, PLL_CMN_CFG1);
	unsigned long div1 = FIELD_GET(PLL_CMN_POSTDIV1, cfg);
	unsigned long div2 = FIELD_GET(PLL_CMN_POSTDIV2, cfg);

	return parent_rate / div1 / div2;
}

static const struct clk_ops sf21_cmnpll_postdiv_ops = {
	.recalc_rate = sf21_cmnpll_postdiv_recalc_rate,
	.round_rate = sf21_cmnpll_postdiv_round_rate,
	.set_rate = sf21_cmnpll_postdiv_set_rate,
};

static const char *const clk_cmnpll_postdiv_parents[] = { "cmnpll_vco" };

static struct sf_clk_common cmnpll_postdiv =
	SF_CLK_COMMON("cmnpll_postdiv", clk_cmnpll_postdiv_parents,
		      &sf21_cmnpll_postdiv_ops, 0);

#define PLL_DDR_CFG1		0x18
#define  PLL_DDR_BYPASS		BIT(23)
#define  PLL_DDR_PLLEN		BIT(22)
#define  PLL_DDR_4PHASEEN	BIT(21)
#define  PLL_DDR_POSTDIVEN	BIT(20)
#define  PLL_DDR_DSMEN		BIT(19)
#define  PLL_DDR_DACEN		BIT(18)
#define  PLL_DDR_DSKEWCALBYP	BIT(17)
#define  PLL_DDR_DSKEWCALCNT	GENMASK(16, 14)
#define  PLL_DDR_DSKEWCALEN	BIT(13)
#define  PLL_DDR_DSKEWCALIN	GENMASK(12, 1)
#define  PLL_DDR_DSKEWFASTCAL	BIT(0)

#define PLL_DDR_CFG2		0x1c
#define  PLL_DDR_POSTDIV1	GENMASK(29, 27)
#define  PLL_DDR_POSTDIV2	GENMASK(26, 24)
#define  PLL_DDR_FRAC		GENMASK(23, 0)

#define PLL_DDR_CFG3		0x20
#define  PLL_DDR_FBDIV		GENMASK(17, 6)
#define  PLL_DDR_REFDIV		GENMASK(5, 0)

static unsigned long
sf21_ddrpll_postdiv_recalc_rate(struct clk_hw *hw,
				     unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	u32 cfg2 = sf_readl(priv, PLL_DDR_CFG2);
	u32 postdiv1 = FIELD_GET(PLL_DDR_POSTDIV1, cfg2);
	u32 postdiv2 = FIELD_GET(PLL_DDR_POSTDIV2, cfg2);
	u32 cfg3 = sf_readl(priv, PLL_DDR_CFG3);
	u32 fbdiv = FIELD_GET(PLL_DDR_FBDIV, cfg3);
	u32 refdiv = FIELD_GET(PLL_DDR_REFDIV, cfg3);

	return (parent_rate / refdiv) * fbdiv / postdiv1 / postdiv2;
}

static const struct clk_ops sf21_ddrpll_postdiv_ops = {
	.recalc_rate = sf21_ddrpll_postdiv_recalc_rate,
};

static struct sf_clk_common ddrpll_postdiv = SF_CLK_COMMON(
	"ddrpll_postdiv", clk_pll_parents, &sf21_ddrpll_postdiv_ops, 0);

#define PLL_PCIE_CFG1		0x4c
#define  PLL_PCIE_PLLEN		BIT(31)
#define  PLL_PCIE_POSTDIV0PRE	BIT(30)
#define  PLL_PCIE_REFDIV	GENMASK(29, 24)
#define  PLL_PCIE_FRAC		GENMASK(23, 0)

#define PLL_PCIE_CFG2		0x50
#define  PLL_PCIE_FOUTEN(i)	BIT(28 + (i))
#define  PLL_PCIE_BYPASS(i)	BIT(24 + (i))
#define  PLL_PCIE_PDIVA_OFFS(i)	(21 - 6 * (i))
#define  PLL_PCIE_PDIVB_OFFS(i)	(18 - 6 * (i))
#define  PLL_PCIE_PDIV_MASK	GENMASK(2, 0)

#define PLL_PCIE_CFG3		0x54
#define  PLL_PCIE_DSKEWFASTCAL	BIT(31)
#define  PLL_PCIE_DACEN		BIT(30)
#define  PLL_PCIE_DSMEN		BIT(29)
#define  PLL_PCIE_DSKEWCALEN	BIT(28)
#define  PLL_PCIE_DSKEWCALBYP	BIT(27)
#define  PLL_PCIE_DSKEWCALCNT	GENMASK(26, 24)
#define  PLL_PCIE_DSKEWCALIN	GENMASK(23, 12)
#define  PLL_PCIE_FBDIV		GENMASK(11, 0)

static unsigned long
sf21_pciepll_vco_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	u32 cfg1 = sf_readl(priv, PLL_PCIE_CFG1);
	unsigned long refdiv = FIELD_GET(PLL_PCIE_REFDIV, cfg1);
	u32 cfg3 = sf_readl(priv, PLL_PCIE_CFG3);
	unsigned long fbdiv = FIELD_GET(PLL_PCIE_FBDIV, cfg3);

	return (parent_rate / refdiv) * fbdiv / 4;
}

static int sf21_pciepll_vco_enable(struct clk_hw *hw)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	unsigned long flags;

	spin_lock_irqsave(priv->lock, flags);
	sf_rmw(priv, PLL_PCIE_CFG1, 0, PLL_PCIE_PLLEN);
	sf_writel(priv, CFG_LOAD, CFG_LOAD_PCIE_PLL);
	sf_writel(priv, CFG_LOAD, 0);
	while (!(sf_readl(priv, PLL_PCIE_LOCK)))
		;
	spin_unlock_irqrestore(priv->lock, flags);
	return 0;
}

static void sf21_pciepll_vco_disable(struct clk_hw *hw)
{
	struct sf_clk_common *priv = hw_to_sf_clk_common(hw);
	unsigned long flags;

	spin_lock_irqsave(priv->lock, flags);
	sf_rmw(priv, PLL_PCIE_CFG1, PLL_PCIE_PLLEN, 0);
	sf_writel(priv, CFG_LOAD, CFG_LOAD_PCIE_PLL);
	sf_writel(priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(priv->lock, flags);
}

static const struct clk_ops sf21_pciepll_vco_ops = {
	.enable = sf21_pciepll_vco_enable,
	.disable = sf21_pciepll_vco_disable,
	.recalc_rate = sf21_pciepll_vco_recalc_rate,
};

static struct sf_clk_common pciepll_vco =
	SF_CLK_COMMON("pciepll_vco", clk_pll_parents,
		      &sf21_pciepll_vco_ops, CLK_SET_RATE_GATE);

struct sf21_pciepll_fout {
	struct sf_clk_common common;
	u8 index;
};

static int sf21_pciepll_fout_enable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_pciepll_fout *priv =
		container_of(cmn_priv, struct sf21_pciepll_fout, common);
	unsigned long flags;

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, PLL_PCIE_CFG2, 0, PLL_PCIE_FOUTEN(priv->index));
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_PCIE_PLL);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
	return 0;
}

static void sf21_pciepll_fout_disable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_pciepll_fout *priv =
		container_of(cmn_priv, struct sf21_pciepll_fout, common);
	unsigned long flags;

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, PLL_PCIE_CFG2, PLL_PCIE_FOUTEN(priv->index), 0);
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_PCIE_PLL);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
}

static long sf21_pciepll_fout_round_rate(struct clk_hw *hw,
					      unsigned long rate,
					      unsigned long *parent_rate)
{
	unsigned int diva, divb;

	return sf21_dualdiv_round_rate(rate, *parent_rate, 8, &diva,
					    &divb);
}

static int sf21_pciepll_fout_set_rate(struct clk_hw *hw,
					   unsigned long rate,
					   unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_pciepll_fout *priv =
		container_of(cmn_priv, struct sf21_pciepll_fout, common);
	unsigned int diva, divb;
	unsigned long flags;

	sf21_dualdiv_round_rate(rate, parent_rate, 8, &diva, &divb);

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, PLL_PCIE_CFG2,
	       (PLL_PCIE_PDIV_MASK << PLL_PCIE_PDIVA_OFFS(priv->index)) |
		       (PLL_PCIE_PDIV_MASK << PLL_PCIE_PDIVB_OFFS(priv->index)),
	       ((diva - 1) << PLL_PCIE_PDIVA_OFFS(priv->index)) |
		       ((divb - 1) << PLL_PCIE_PDIVB_OFFS(priv->index)));
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_PCIE_PLL);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
	return 0;
}

static unsigned long
sf21_pciepll_fout_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_pciepll_fout *priv =
		container_of(cmn_priv, struct sf21_pciepll_fout, common);
	int idx = priv->index;
	u32 cfg2 = sf_readl(cmn_priv, PLL_PCIE_CFG2);
	ulong pdiva = (cfg2 >> PLL_PCIE_PDIVA_OFFS(idx)) & PLL_PCIE_PDIV_MASK;
	ulong pdivb = (cfg2 >> PLL_PCIE_PDIVB_OFFS(idx)) & PLL_PCIE_PDIV_MASK;

	return parent_rate / (pdiva + 1) / (pdivb + 1);
}

static const struct clk_ops sf21_pciepll_fout_ops = {
	.enable = sf21_pciepll_fout_enable,
	.disable = sf21_pciepll_fout_disable,
	.recalc_rate = sf21_pciepll_fout_recalc_rate,
	.round_rate = sf21_pciepll_fout_round_rate,
	.set_rate = sf21_pciepll_fout_set_rate,
};

static const char * const clk_pciepll_fout_parents[] = { "pciepll_vco" };

#define SF21_PCIEPLL_FOUT(_name, _idx, _flags)			\
	struct sf21_pciepll_fout _name = {				\
		.common = SF_CLK_COMMON(#_name,				\
					clk_pciepll_fout_parents,	\
					&sf21_pciepll_fout_ops,	\
					_flags),			\
		.index = _idx,						\
	}

static SF21_PCIEPLL_FOUT(pciepll_fout0, 0, 0);
static SF21_PCIEPLL_FOUT(pciepll_fout1, 1, 0);
static SF21_PCIEPLL_FOUT(pciepll_fout2, 2, 0);
static SF21_PCIEPLL_FOUT(pciepll_fout3, 3, 0);

struct sf21_clk_muxdiv {
	struct sf_clk_common common;
	u16 mux;
	u16 en;
	u8 div_reg;
	u8 div_offs;
};

#define CRM_CLK_SEL(_x)		((_x) * 4 + 0x80)
#define CRM_CLK_EN		0x8c
#define CRM_CLK_DIV(_x)		((_x) * 4 + 0x94)
#define  CRM_CLK_DIV_MASK	GENMASK(7, 0)

static unsigned long sf21_muxdiv_recalc_rate(struct clk_hw *hw,
						  unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	ulong div_reg = CRM_CLK_DIV(priv->div_reg);
	u16 div_offs = priv->div_offs;
	u16 div_val = (sf_readl(cmn_priv, div_reg) >> div_offs) &
		      CRM_CLK_DIV_MASK;
	div_val += 1;
	return parent_rate / div_val;
}

static int sf21_muxdiv_determine_rate(struct clk_hw *hw,
					   struct clk_rate_request *req)
{
	unsigned int div;

	div = DIV_ROUND_CLOSEST(req->best_parent_rate, req->rate);
	if (!div)
		div = 1;
	else if (div > CRM_CLK_DIV_MASK + 1)
		div = CRM_CLK_DIV_MASK + 1;

	req->rate = req->best_parent_rate / div;
	return 0;
}

static int sf21_muxdiv_set_rate(struct clk_hw *hw, unsigned long rate,
				     unsigned long parent_rate)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	ulong div_reg = CRM_CLK_DIV(priv->div_reg);
	u16 div_offs = priv->div_offs;
	unsigned long flags;
	unsigned int div;

	div = DIV_ROUND_CLOSEST(parent_rate, rate);
	if (div < 1)
		div = 1;
	else if (div > CRM_CLK_DIV_MASK + 1)
		div = CRM_CLK_DIV_MASK + 1;
	div -= 1;

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, div_reg, CRM_CLK_DIV_MASK << div_offs,
	       div << div_offs);
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_DIV);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
	return 0;
}

static int sf21_muxdiv_enable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	unsigned long flags;

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, CRM_CLK_EN, 0, BIT(priv->en));
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_DIV);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
	return 0;
}

static void sf21_muxdiv_disable(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	unsigned long flags;

	spin_lock_irqsave(cmn_priv->lock, flags);
	sf_rmw(cmn_priv, CRM_CLK_EN, BIT(priv->en), 0);
	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_DIV);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
}

static int sf21_muxdiv_is_enabled(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	u32 reg_val = sf_readl(cmn_priv, CRM_CLK_EN);

	return reg_val & (BIT(priv->en)) ? 1 : 0;
}

static u8 sf21_muxdiv_get_parent(struct clk_hw *hw)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	ulong mux_reg = CRM_CLK_SEL(priv->mux / 32);
	u16 mux_offs = priv->mux % 32;
	u32 reg_val = sf_readl(cmn_priv, mux_reg);

	return reg_val & BIT(mux_offs) ? 1 : 0;
}

static int sf21_muxdiv_set_parent(struct clk_hw *hw, u8 index)
{
	struct sf_clk_common *cmn_priv = hw_to_sf_clk_common(hw);
	struct sf21_clk_muxdiv *priv =
		container_of(cmn_priv, struct sf21_clk_muxdiv, common);
	ulong mux_reg = CRM_CLK_SEL(priv->mux / 32);
	u16 mux_offs = priv->mux % 32;
	unsigned long flags;

	spin_lock_irqsave(cmn_priv->lock, flags);
	if (index)
		sf_rmw(cmn_priv, mux_reg, 0, BIT(mux_offs));
	else
		sf_rmw(cmn_priv, mux_reg, BIT(mux_offs), 0);

	sf_writel(cmn_priv, CFG_LOAD, CFG_LOAD_DIV);
	sf_writel(cmn_priv, CFG_LOAD, 0);
	spin_unlock_irqrestore(cmn_priv->lock, flags);
	return 0;
}

static const struct clk_ops sf21_clk_muxdiv_ops = {
	.enable = sf21_muxdiv_enable,
	.disable = sf21_muxdiv_disable,
	.is_enabled = sf21_muxdiv_is_enabled,
	.recalc_rate = sf21_muxdiv_recalc_rate,
	.determine_rate = sf21_muxdiv_determine_rate,
	.set_rate = sf21_muxdiv_set_rate,
	.get_parent = sf21_muxdiv_get_parent,
	.set_parent = sf21_muxdiv_set_parent,
};

#define SF21_MUXDIV(_name, _parents,				\
			 _mux, _div_reg, _div_offs, _en, _flags)	\
	struct sf21_clk_muxdiv _name = {				\
		.common = SF_CLK_COMMON(#_name, _parents,		\
					&sf21_clk_muxdiv_ops,	\
					_flags),			\
		.mux = _mux,						\
		.en = _en,						\
		.div_reg = _div_reg,					\
		.div_offs = _div_offs,					\
	}

static const char *const clk_periph_parents[] = { "cmnpll_postdiv",
						  "ddrpll_postdiv" };
static const char *const clk_ddr_parents[] = { "ddrpll_postdiv",
					       "cmnpll_postdiv" };
static const char *const clk_gmac_usb_parents[] = { "cmnpll_vco",
						    "ddrpll_postdiv" };

static SF21_MUXDIV(muxdiv_cpu, clk_periph_parents, 1, 0, 0, 0, CLK_IGNORE_UNUSED);
static SF21_MUXDIV(muxdiv_pic, clk_periph_parents, 3, 3, 16, 1, CLK_IGNORE_UNUSED);
static SF21_MUXDIV(muxdiv_axi, clk_periph_parents, 5, 0, 8, 2, CLK_IS_CRITICAL);
static SF21_MUXDIV(muxdiv_ahb, clk_periph_parents, 7, 0, 16, 3, CLK_IS_CRITICAL);
static SF21_MUXDIV(muxdiv_apb, clk_periph_parents, 9, 0, 24, 4, CLK_IS_CRITICAL);
static SF21_MUXDIV(muxdiv_uart, clk_periph_parents, 11, 1, 0, 5, 0);
static SF21_MUXDIV(muxdiv_iram, clk_periph_parents, 13, 1, 8, 6, 0);
static SF21_MUXDIV(muxdiv_npu, clk_periph_parents, 17, 1, 24, 8, 0);
static SF21_MUXDIV(muxdiv_ddrphy, clk_ddr_parents, 19, 2, 0, 9, CLK_IS_CRITICAL);
static SF21_MUXDIV(muxdiv_ddr_bypass, clk_ddr_parents, 21, 3, 0, 10, CLK_IS_CRITICAL);
static SF21_MUXDIV(muxdiv_ethtsu, clk_periph_parents, 25, 2, 16, 12, 0);
static SF21_MUXDIV(muxdiv_gmac_byp_ref, clk_gmac_usb_parents, 27, 2, 24, 13, 0);
static SF21_MUXDIV(muxdiv_usb, clk_gmac_usb_parents, 33, 1, 16, 24, 0);
static SF21_MUXDIV(muxdiv_usbphy, clk_gmac_usb_parents, 35, 2, 8, 25, 0);
static SF21_MUXDIV(muxdiv_serdes_csr, clk_periph_parents, 47, 5, 0, 20, 0);
static SF21_MUXDIV(muxdiv_crypt_csr, clk_periph_parents, 49, 5, 8, 21, 0);
static SF21_MUXDIV(muxdiv_crypt_app, clk_periph_parents, 51, 5, 16, 22, 0);
static SF21_MUXDIV(muxdiv_irom, clk_periph_parents, 53, 5, 24, 23, CLK_IS_CRITICAL);

static int sf21_mux_determine_rate(struct clk_hw *hw,
					struct clk_rate_request *req)
{
	req->rate = req->best_parent_rate;
	return 0;
}

static const struct clk_ops sf21_clk_mux_ops = {
	.get_parent = sf21_muxdiv_get_parent,
	.set_parent = sf21_muxdiv_set_parent,
	.determine_rate = sf21_mux_determine_rate,
};

#define SF21_MUX(_name, _parents, _mux, _flags)			\
	struct sf21_clk_muxdiv _name = {				\
		.common = SF_CLK_COMMON(#_name, _parents,		\
					&sf21_clk_mux_ops,		\
					_flags),			\
		.mux = _mux,						\
		.en = 0,						\
		.div_reg = 0,						\
		.div_offs = 0,						\
	}

static const char * const clk_boot_parents[] = { "muxdiv_irom", "xin25m" };

static SF21_MUX(mux_boot, clk_boot_parents, 30, CLK_IS_CRITICAL);

static const struct clk_ops sf21_clk_div_ops = {
	.recalc_rate = sf21_muxdiv_recalc_rate,
	.determine_rate = sf21_muxdiv_determine_rate,
	.set_rate = sf21_muxdiv_set_rate,
};

#define SF21_DIV(_name, _parents, _div_reg, _div_offs, _flags)	\
	struct sf21_clk_muxdiv _name = {				\
		.common = SF_CLK_COMMON(#_name, _parents,		\
					&sf21_clk_div_ops,		\
					_flags),			\
		.mux = 0,						\
		.en = 0,						\
		.div_reg = _div_reg,					\
		.div_offs = _div_offs,					\
	}

static SF21_DIV(div_pvt, clk_pll_parents, 3, 8, 0);
static SF21_DIV(div_pll_test, clk_pll_parents, 3, 24, 0);

static const struct clk_ops sf21_clk_gate_ops = {
	.enable = sf21_muxdiv_enable,
	.disable = sf21_muxdiv_disable,
	.is_enabled = sf21_muxdiv_is_enabled,
};

#define SF21_GATE(_name, _parents, _en, _flags)			\
	struct sf21_clk_muxdiv _name = {				\
		.common = SF_CLK_COMMON(#_name,				\
					_parents,			\
					&sf21_clk_gate_ops,	\
					_flags),			\
		.mux = 0,						\
		.en = _en,						\
		.div_reg = 0,						\
		.div_offs = 0,						\
	}

static const char * const clk_pcie_parents[] = { "pciepll_fout1" };

static SF21_GATE(pcie_refclk_n, clk_pcie_parents, 15, 0);
static SF21_GATE(pcie_refclk_p, clk_pcie_parents, 16, 0);

static struct clk_hw_onecell_data sf21_hw_clks = {
	.num = CLK_MAX,
	.hws = {
		[CLK_CMNPLL_VCO] = &cmnpll_vco.hw,
		[CLK_CMNPLL_POSTDIV] = &cmnpll_postdiv.hw,
		[CLK_DDRPLL_POSTDIV] = &ddrpll_postdiv.hw,
		[CLK_PCIEPLL_VCO] = &pciepll_vco.hw,
		[CLK_PCIEPLL_FOUT0] = &pciepll_fout0.common.hw,
		[CLK_PCIEPLL_FOUT1] = &pciepll_fout1.common.hw,
		[CLK_PCIEPLL_FOUT2] = &pciepll_fout2.common.hw,
		[CLK_PCIEPLL_FOUT3] = &pciepll_fout3.common.hw,
		[CLK_CPU] = &muxdiv_cpu.common.hw,
		[CLK_PIC] = &muxdiv_pic.common.hw,
		[CLK_AXI] = &muxdiv_axi.common.hw,
		[CLK_AHB] = &muxdiv_ahb.common.hw,
		[CLK_APB] = &muxdiv_apb.common.hw,
		[CLK_UART] = &muxdiv_uart.common.hw,
		[CLK_IRAM] = &muxdiv_iram.common.hw,
		[CLK_NPU] = &muxdiv_npu.common.hw,
		[CLK_DDRPHY_REF] = &muxdiv_ddrphy.common.hw,
		[CLK_DDR_BYPASS] = &muxdiv_ddr_bypass.common.hw,
		[CLK_ETHTSU] = &muxdiv_ethtsu.common.hw,
		[CLK_GMAC_BYP_REF] = &muxdiv_gmac_byp_ref.common.hw,
		[CLK_USB] = &muxdiv_usb.common.hw,
		[CLK_USBPHY] = &muxdiv_usbphy.common.hw,
		[CLK_SERDES_CSR] = &muxdiv_serdes_csr.common.hw,
		[CLK_CRYPT_CSR] = &muxdiv_crypt_csr.common.hw,
		[CLK_CRYPT_APP] = &muxdiv_crypt_app.common.hw,
		[CLK_IROM] = &muxdiv_irom.common.hw,
		[CLK_BOOT] = &mux_boot.common.hw,
		[CLK_PVT] = &div_pvt.common.hw,
		[CLK_PLL_TEST] = &div_pll_test.common.hw,
		[CLK_PCIE_REFN] = &pcie_refclk_n.common.hw,
		[CLK_PCIE_REFP] = &pcie_refclk_p.common.hw,
	}
};

struct sf21_clk_ctrl {
	void __iomem *base;
	spinlock_t lock;
};

static void __init sf21_topcrm_init(struct device_node *node)
{
	struct sf21_clk_ctrl *ctrl;
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

	for (i = 0; i < sf21_hw_clks.num; i++) {
		struct clk_hw *hw = sf21_hw_clks.hws[i];
		struct sf_clk_common *common;

		if (!hw)
			continue;
		common = hw_to_sf_clk_common(hw);
		common->base = ctrl->base;
		common->lock = &ctrl->lock;
		ret = clk_hw_register(NULL, hw);
		if (ret) {
			pr_err("Couldn't register clock %d\n", i);
			goto err;
		}
	}

	ret = of_clk_add_hw_provider(node, of_clk_hw_onecell_get,
				     &sf21_hw_clks);
	if (ret) {
		pr_err("failed to add hw provider.\n");
		goto err;
	}
	return;
err:
	iounmap(ctrl->base);
}

CLK_OF_DECLARE(sf21_topcrm, "siflower,sf21-topcrm", sf21_topcrm_init);
