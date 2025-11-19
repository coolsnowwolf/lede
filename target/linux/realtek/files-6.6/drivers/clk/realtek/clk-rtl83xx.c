// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL83XX clock driver
 * Copyright (C) 2022 Markus Stockhausen <markus.stockhausen@gmx.de>
 *
 * This driver provides basic clock support for the central core clock unit (CCU) and its PLLs
 * inside the RTL838X and RTL8389X SOC. Currently CPU, memory and LXB clock information can be
 * accessed. To make use of the driver add the following devices and configurations at the
 * appropriate locations to the DT.
 *
 * #include <dt-bindings/clock/rtl83xx-clk.h>
 *
 * sram0: sram@9f000000 {
 *   compatible = "mmio-sram";
 *   reg = <0x9f000000 0x18000>;
 *   #address-cells = <1>;
 *   #size-cells = <1>;
 *   ranges = <0 0x9f000000 0x18000>;
 * };
 *
 * osc: oscillator {
 *   compatible = "fixed-clock";
 *   #clock-cells = <0>;
 *   clock-frequency = <25000000>;
 * };
 *
 * ccu: clock-controller {
 *   compatible = "realtek,rtl8380-clock";
 *   #clock-cells = <1>;
 *   clocks = <&osc>;
 *   clock-names = "ref_clk";
 * };
 *
 *
 * The SRAM part is needed to be able to set clocks. When changing clocks the code must not run
 * from DRAM. Otherwise system might freeze. Take care to adjust CCU compatibility, SRAM address
 * and size to the target SOC device. Afterwards one can access/identify the clocks in the other
 * DT devices with <&ccu CLK_CPU>, <&ccu CLK_MEM> or <&ccu CLK_LXB>. Additionally the clocks can
 * be used inside the kernel with
 *
 * cpu_clk = clk_get(NULL, "cpu_clk");
 * mem_clk = clk_get(NULL, "mem_clk");
 * lxb_clk = clk_get(NULL, "lxb_clk");
 *
 * This driver can be directly used by the DT based cpufreq driver (CONFIG_CPUFREQ_DT) if CPU
 * references the right clock and sane operating points (OPP) are provided. E.g.
 *
 * cpu@0 {
 *   compatible = "mips,mips4KEc";
 *   reg = <0>;
 *   clocks = <&ccu CLK_CPU>;
 *   operating-points-v2 = <&cpu_opp_table>;
 * };
 *
 * cpu_opp_table: opp-table-0 {
 *   compatible = "operating-points-v2";
 *   opp-shared;
 *   opp00 {
 *     opp-hz = /bits/ 64 <425000000>;
 *   };
 *   ...
 * }
 */

#include <asm/cacheflush.h>
#include <asm/mipsmtregs.h>
#include <dt-bindings/clock/rtl83xx-clk.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/genalloc.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "clk-rtl83xx.h"

#define read_sw(reg)		ioread32(((void *)RTL_SW_CORE_BASE) + reg)
#define read_soc(reg)		ioread32(((void *)RTL_SOC_BASE) + reg)

#define write_sw(val, reg)	iowrite32(val, ((void *)RTL_SW_CORE_BASE) + reg)
#define write_soc(val, reg)	iowrite32(val, ((void *)RTL_SOC_BASE) + reg)

/*
 * some hardware specific definitions
 */

#define SOC_RTL838X		0
#define SOC_RTL839X		1
#define SOC_COUNT		2

#define MEM_DDR1		1
#define MEM_DDR2		2
#define MEM_DDR3		3

#define REG_CTRL0		0
#define REG_CTRL1		1
#define REG_COUNT		2

#define OSC_RATE		25000000

static const int rtcl_regs[SOC_COUNT][REG_COUNT][CLK_COUNT] = {
	{
		{ RTL838X_PLL_CPU_CTRL0, RTL838X_PLL_MEM_CTRL0, RTL838X_PLL_LXB_CTRL0 },
		{ RTL838X_PLL_CPU_CTRL1, RTL838X_PLL_MEM_CTRL1, RTL838X_PLL_LXB_CTRL1 },
	}, {
		{ RTL839X_PLL_CPU_CTRL0, RTL839X_PLL_MEM_CTRL0, RTL839X_PLL_LXB_CTRL0 },
		{ RTL839X_PLL_CPU_CTRL1, RTL839X_PLL_MEM_CTRL1, RTL839X_PLL_LXB_CTRL1 },
	}
};

#define RTCL_REG_SET(_rate, _ctrl0, _ctrl1)	\
	{					\
		.rate = _rate,			\
		.ctrl0 = _ctrl0,		\
		.ctrl1 = _ctrl1,		\
	}

struct rtcl_reg_set {
	unsigned int rate;
	unsigned int ctrl0;
	unsigned int ctrl1;
};

/*
 * The following configuration tables are valid operation points for their
 * corresponding PLLs. The magic numbers are precalculated mulitpliers and
 * dividers to keep the driver simple. They also provide rates outside the
 * allowed physical specifications. E.g. DDR3 memory has a lower limit of 303
 * MHz or the CPU might get unstable if set to anything above its startup
 * frequency. Additionally the Realtek SOCs tend to expect CPU speed larger
 * than MEM speed larger than LXB speed. The caller or DT configuration must
 * take care that only valid operating points are selected.
 */

static const struct rtcl_reg_set rtcl_838x_cpu_reg_set[] = {
	RTCL_REG_SET(300000000, 0x045c8, 0x1414530e),
	RTCL_REG_SET(325000000, 0x04648, 0x1414530e),
	RTCL_REG_SET(350000000, 0x046c8, 0x1414530e),
	RTCL_REG_SET(375000000, 0x04748, 0x1414530e),
	RTCL_REG_SET(400000000, 0x045c8, 0x0c14530e),
	RTCL_REG_SET(425000000, 0x04628, 0x0c14530e),
	RTCL_REG_SET(450000000, 0x04688, 0x0c14530e),
	RTCL_REG_SET(475000000, 0x046e8, 0x0c14530e),
	RTCL_REG_SET(500000000, 0x04748, 0x0c14530e),
	RTCL_REG_SET(525000000, 0x047a8, 0x0c14530e),
	RTCL_REG_SET(550000000, 0x04808, 0x0c14530e),
	RTCL_REG_SET(575000000, 0x04868, 0x0c14530e),
	RTCL_REG_SET(600000000, 0x048c8, 0x0c14530e),
	RTCL_REG_SET(625000000, 0x04928, 0x0c14530e)
};

static const struct rtcl_reg_set rtcl_838x_mem_reg_set[] = {
	RTCL_REG_SET(200000000, 0x041bc, 0x14018C80),
	RTCL_REG_SET(225000000, 0x0417c, 0x0c018C80),
	RTCL_REG_SET(250000000, 0x041ac, 0x0c018C80),
	RTCL_REG_SET(275000000, 0x0412c, 0x04018C80),
	RTCL_REG_SET(300000000, 0x0414c, 0x04018c80),
	RTCL_REG_SET(325000000, 0x0416c, 0x04018c80),
	RTCL_REG_SET(350000000, 0x0418c, 0x04018c80),
	RTCL_REG_SET(375000000, 0x041ac, 0x04018c80)
};

static const struct rtcl_reg_set rtcl_838x_lxb_reg_set[] = {
	RTCL_REG_SET(100000000, 0x043c8, 0x001ad30e),
	RTCL_REG_SET(125000000, 0x043c8, 0x001ad30e),
	RTCL_REG_SET(150000000, 0x04508, 0x1c1ad30e),
	RTCL_REG_SET(175000000, 0x04508, 0x1c1ad30e),
	RTCL_REG_SET(200000000, 0x047c8, 0x001ad30e)
};

static const struct rtcl_reg_set rtcl_839x_cpu_reg_set[] = {
	RTCL_REG_SET(400000000, 0x0414c, 0x00000005),
	RTCL_REG_SET(425000000, 0x041ec, 0x00000006),
	RTCL_REG_SET(450000000, 0x0417c, 0x00000005),
	RTCL_REG_SET(475000000, 0x0422c, 0x00000006),
	RTCL_REG_SET(500000000, 0x041ac, 0x00000005),
	RTCL_REG_SET(525000000, 0x0426c, 0x00000006),
	RTCL_REG_SET(550000000, 0x0412c, 0x00000004),
	RTCL_REG_SET(575000000, 0x042ac, 0x00000006),
	RTCL_REG_SET(600000000, 0x0414c, 0x00000004),
	RTCL_REG_SET(625000000, 0x042ec, 0x00000006),
	RTCL_REG_SET(650000000, 0x0416c, 0x00000004),
	RTCL_REG_SET(675000000, 0x04324, 0x00000006),
	RTCL_REG_SET(700000000, 0x0418c, 0x00000004),
	RTCL_REG_SET(725000000, 0x0436c, 0x00000006),
	RTCL_REG_SET(750000000, 0x0438c, 0x00000006),
	RTCL_REG_SET(775000000, 0x043ac, 0x00000006),
	RTCL_REG_SET(800000000, 0x043cc, 0x00000006),
	RTCL_REG_SET(825000000, 0x043ec, 0x00000006),
	RTCL_REG_SET(850000000, 0x0440c, 0x00000006)
};

static const struct rtcl_reg_set rtcl_839x_mem_reg_set[] = {
	RTCL_REG_SET(100000000, 0x041cc, 0x00000000),
	RTCL_REG_SET(125000000, 0x041ac, 0x00000007),
	RTCL_REG_SET(150000000, 0x0414c, 0x00000006),
	RTCL_REG_SET(175000000, 0x0418c, 0x00000006),
	RTCL_REG_SET(200000000, 0x041cc, 0x00000006),
	RTCL_REG_SET(225000000, 0x0417c, 0x00000005),
	RTCL_REG_SET(250000000, 0x041ac, 0x00000005),
	RTCL_REG_SET(275000000, 0x0412c, 0x00000004),
	RTCL_REG_SET(300000000, 0x0414c, 0x00000004),
	RTCL_REG_SET(325000000, 0x0416c, 0x00000004),
	RTCL_REG_SET(350000000, 0x0418c, 0x00000004),
	RTCL_REG_SET(375000000, 0x041ac, 0x00000004),
	RTCL_REG_SET(400000000, 0x041cc, 0x00000004)
};

static const struct rtcl_reg_set rtcl_839x_lxb_reg_set[] = {
	RTCL_REG_SET(50000000, 0x1414c, 0x00000003),
	RTCL_REG_SET(100000000, 0x0814c, 0x00000003),
	RTCL_REG_SET(150000000, 0x0414c, 0x00000003),
	RTCL_REG_SET(200000000, 0x0414c, 0x00000007)
};

struct rtcl_rtab_set {
	int count;
	const struct rtcl_reg_set *rset;
};

#define RTCL_RTAB_SET(_rset)			\
	{					\
		.count = ARRAY_SIZE(_rset),	\
		.rset = _rset,			\
	}

static const struct rtcl_rtab_set rtcl_rtab_set[SOC_COUNT][CLK_COUNT] = {
	{
		RTCL_RTAB_SET(rtcl_838x_cpu_reg_set),
		RTCL_RTAB_SET(rtcl_838x_mem_reg_set),
		RTCL_RTAB_SET(rtcl_838x_lxb_reg_set)
	}, {
		RTCL_RTAB_SET(rtcl_839x_cpu_reg_set),
		RTCL_RTAB_SET(rtcl_839x_mem_reg_set),
		RTCL_RTAB_SET(rtcl_839x_lxb_reg_set)
	}
};

#define RTCL_ROUND_SET(_min, _max, _step)	\
	{					\
		.min = _min,			\
		.max = _max,			\
		.step = _step,			\
	}

struct rtcl_round_set {
	unsigned long min;
	unsigned long max;
	unsigned long step;
};

static const struct rtcl_round_set rtcl_round_set[SOC_COUNT][CLK_COUNT] = {
	{
		RTCL_ROUND_SET(300000000, 625000000, 25000000),
		RTCL_ROUND_SET(200000000, 375000000, 25000000),
		RTCL_ROUND_SET(100000000, 200000000, 25000000)
	}, {
		RTCL_ROUND_SET(400000000, 850000000, 25000000),
		RTCL_ROUND_SET(100000000, 400000000, 25000000),
		RTCL_ROUND_SET(50000000, 200000000, 50000000)
	}
};

static const int rtcl_divn3[] = { 2, 3, 4, 6 };
static const int rtcl_xdiv[] = { 2, 4, 2 };

/*
 * module data structures
 */

#define RTCL_CLK_INFO(_idx, _name, _pname, _dname)	\
	{						\
		.idx = _idx,				\
		.name = _name,				\
		.parent_name = _pname,			\
		.display_name = _dname,			\
	}

struct rtcl_clk_info {
	unsigned int idx;
	const char *name;
	const char *parent_name;
	const char *display_name;
};

struct rtcl_clk {
	struct clk_hw hw;
	unsigned int idx;
	unsigned long min;
	unsigned long max;
	unsigned long rate;
	unsigned long startup;
};

static const struct rtcl_clk_info rtcl_clk_info[CLK_COUNT] = {
	RTCL_CLK_INFO(CLK_CPU, "cpu_clk", "ref_clk", "CPU"),
	RTCL_CLK_INFO(CLK_MEM, "mem_clk", "ref_clk", "MEM"),
	RTCL_CLK_INFO(CLK_LXB, "lxb_clk", "ref_clk", "LXB")
};

struct rtcl_dram {
	int type;
	int buswidth;
};

struct rtcl_sram {
	int *pmark;
	unsigned long vbase;
};

struct rtcl_ccu {
	spinlock_t lock;
	unsigned int soc;
	struct rtcl_sram sram;
	struct rtcl_dram dram;
	struct device_node *np;
	struct platform_device *pdev;
	struct rtcl_clk clks[CLK_COUNT];
};

struct rtcl_ccu *rtcl_ccu;

#define rtcl_hw_to_clk(_hw) container_of(_hw, struct rtcl_clk, hw)

/*
 * SRAM relocatable assembler functions. The dram() parts point to normal kernel
 * memory while the sram() parts are the same functions but relocated to SRAM.
 */

extern void	rtcl_838x_dram_start(void);
extern int	rtcl_838x_dram_size;

extern void	(*rtcl_838x_dram_set_rate)(int clk_idx, int ctrl0, int ctrl1);
static void	(*rtcl_838x_sram_set_rate)(int clk_idx, int ctrl0, int ctrl1);

extern void	rtcl_839x_dram_start(void);
extern int	rtcl_839x_dram_size;

extern void	(*rtcl_839x_dram_set_rate)(int clk_idx, int ctrl0, int ctrl1);
static void	(*rtcl_839x_sram_set_rate)(int clk_idx, int ctrl0, int ctrl1);

/*
 * clock setter/getter functions
 */

static unsigned long rtcl_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	struct rtcl_clk *clk = rtcl_hw_to_clk(hw);
	unsigned int ctrl0, ctrl1, div1, div2, cmu_ncode_in;
	unsigned int cmu_sel_prediv, cmu_sel_div4, cmu_divn2, cmu_divn2_selb, cmu_divn3_sel;

	if ((clk->idx >= CLK_COUNT) || (!rtcl_ccu) || (rtcl_ccu->soc >= SOC_COUNT))
		return 0;

	ctrl0 = read_sw(rtcl_regs[rtcl_ccu->soc][REG_CTRL0][clk->idx]);
	ctrl1 = read_sw(rtcl_regs[rtcl_ccu->soc][REG_CTRL1][clk->idx]);

	cmu_sel_prediv = 1 << RTL_PLL_CTRL0_CMU_SEL_PREDIV(ctrl0);
	cmu_sel_div4 = RTL_PLL_CTRL0_CMU_SEL_DIV4(ctrl0) ? 4 : 1;
	cmu_ncode_in = RTL_PLL_CTRL0_CMU_NCODE_IN(ctrl0) + 4;
	cmu_divn2 = RTL_PLL_CTRL0_CMU_DIVN2(ctrl0) + 4;

	switch (rtcl_ccu->soc) {
	case SOC_RTL838X:
		if ((ctrl0 == 0) && (ctrl1 == 0) && (clk->idx == CLK_LXB))
			return 200000000;

		cmu_divn2_selb = RTL838X_PLL_CTRL1_CMU_DIVN2_SELB(ctrl1);
		cmu_divn3_sel = rtcl_divn3[RTL838X_PLL_CTRL1_CMU_DIVN3_SEL(ctrl1)];
		break;
	case SOC_RTL839X:
		cmu_divn2_selb = RTL839X_PLL_CTRL1_CMU_DIVN2_SELB(ctrl1);
		cmu_divn3_sel = rtcl_divn3[RTL839X_PLL_CTRL1_CMU_DIVN3_SEL(ctrl1)];
		break;
	}
	div1 = cmu_divn2_selb ? cmu_divn3_sel : cmu_divn2;
	div2 = rtcl_xdiv[clk->idx];

	return (((parent_rate / 16) * cmu_ncode_in) / (div1 * div2)) *
		cmu_sel_prediv * cmu_sel_div4 * 16;
}

static int rtcl_838x_set_rate(int clk_idx, const struct rtcl_reg_set *reg)
{
	unsigned long irqflags;
/*
 * Runtime of this function (including locking)
 * CPU: up to 14000 cycles / up to 56 us at 250 MHz (half default speed)
 */
	spin_lock_irqsave(&rtcl_ccu->lock, irqflags);
	rtcl_838x_sram_set_rate(clk_idx, reg->ctrl0, reg->ctrl1);
	spin_unlock_irqrestore(&rtcl_ccu->lock, irqflags);

	return 0;
}

static int rtcl_839x_set_rate(int clk_idx, const struct rtcl_reg_set *reg)
{
	unsigned long vpflags;
	unsigned long irqflags;
/*
 * Runtime of this function (including locking)
 * CPU: up to 31000 cycles / up to 89 us at 350 MHz (half default speed)
 */
	spin_lock_irqsave(&rtcl_ccu->lock, irqflags);
	vpflags = dvpe();
	rtcl_839x_sram_set_rate(clk_idx, reg->ctrl0, reg->ctrl1);
	evpe(vpflags);
	spin_unlock_irqrestore(&rtcl_ccu->lock, irqflags);

	return 0;
}

static int rtcl_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
	int tab_idx;
	struct rtcl_clk *clk = rtcl_hw_to_clk(hw);
	const struct rtcl_rtab_set *rtab = &rtcl_rtab_set[rtcl_ccu->soc][clk->idx];
	const struct rtcl_round_set *round = &rtcl_round_set[rtcl_ccu->soc][clk->idx];

	if ((parent_rate != OSC_RATE) || (!rtcl_ccu->sram.vbase))
		return -EINVAL;
/*
 * Currently we do not know if SRAM is stable on these devices. Maybe someone
 * changes memory in this region and does not care about proper allocation. So
 * check if something might go wrong.
 */
	if (unlikely(*rtcl_ccu->sram.pmark != RTL_SRAM_MARKER)) {
		dev_err(&rtcl_ccu->pdev->dev, "SRAM code lost\n");
		return -EINVAL;
	}

	tab_idx = (rate - round->min) / round->step;
	if ((tab_idx < 0) || (tab_idx >= rtab->count) || (rtab->rset[tab_idx].rate != rate))
		return -EINVAL;

	rtcl_ccu->clks[clk->idx].rate = rate;

	switch (rtcl_ccu->soc) {
	case SOC_RTL838X:
		return rtcl_838x_set_rate(clk->idx, &rtab->rset[tab_idx]);
	case SOC_RTL839X:
		return rtcl_839x_set_rate(clk->idx, &rtab->rset[tab_idx]);
	}

	return -ENXIO;
}

static long rtcl_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *parent_rate)
{
	struct rtcl_clk *clk = rtcl_hw_to_clk(hw);
	unsigned long rrate = max(clk->min, min(clk->max, rate));
	const struct rtcl_round_set *round = &rtcl_round_set[rtcl_ccu->soc][clk->idx];

	rrate = ((rrate + (round->step >> 1)) / round->step) * round->step;
	rrate -= (rrate > clk->max) ? round->step : 0;
	rrate += (rrate < clk->min) ? round->step : 0;

	return rrate;
}

/*
 * Initialization functions to register the CCU and its clocks
 */

#define RTCL_SRAM_FUNC(SOC, PBASE, FN) ({				\
        rtcl_##SOC##_sram_##FN = ((void *)&rtcl_##SOC##_dram_##FN -	\
                                  (void *)&rtcl_##SOC##_dram_start) +	\
                                  (void *)PBASE; })

static const struct clk_ops rtcl_clk_ops = {
	.set_rate = rtcl_set_rate,
	.round_rate = rtcl_round_rate,
	.recalc_rate = rtcl_recalc_rate,
};

static int rtcl_ccu_create(struct device_node *np)
{
	int soc;

	if (of_device_is_compatible(np, "realtek,rtl8380-clock"))
		soc = SOC_RTL838X;
	else if (of_device_is_compatible(np, "realtek,rtl8390-clock"))
		soc = SOC_RTL839X;
	else
		return -ENXIO;

	rtcl_ccu = kzalloc(sizeof(*rtcl_ccu), GFP_KERNEL);
	if (IS_ERR(rtcl_ccu))
		return -ENOMEM;

	rtcl_ccu->np = np;
	rtcl_ccu->soc = soc;
	rtcl_ccu->dram.type = RTL_MC_MCR_DRAMTYPE(read_soc(RTL_MC_MCR));
	rtcl_ccu->dram.buswidth = RTL_MC_DCR_BUSWIDTH(read_soc(RTL_MC_DCR));
	spin_lock_init(&rtcl_ccu->lock);

	return 0;
}

int rtcl_register_clkhw(int clk_idx)
{
	int ret;
	struct clk *clk;
	struct clk_init_data hw_init = { };
	struct rtcl_clk *rclk = &rtcl_ccu->clks[clk_idx];
	struct clk_parent_data parent_data = { .fw_name = rtcl_clk_info[clk_idx].parent_name };

	rclk->idx = clk_idx;
	rclk->hw.init = &hw_init;

	hw_init.num_parents = 1;
	hw_init.ops = &rtcl_clk_ops;
	hw_init.parent_data = &parent_data;
	hw_init.name = rtcl_clk_info[clk_idx].name;

	ret = of_clk_hw_register(rtcl_ccu->np, &rclk->hw);
	if (ret)
		return ret;

	clk_hw_register_clkdev(&rclk->hw, rtcl_clk_info[clk_idx].name, NULL);

	clk = clk_get(NULL, rtcl_clk_info[clk_idx].name);
	rclk->startup = clk_get_rate(clk);
	clk_put(clk);

	switch (clk_idx) {
	case CLK_CPU:
		rclk->min = rtcl_round_set[rtcl_ccu->soc][clk_idx].min;
		rclk->max = rtcl_round_set[rtcl_ccu->soc][clk_idx].max;
		break;
	default:
/*
 * TODO: This driver supports PLL reclocking and nothing else. Additional
 * required steps for non CPU PLLs are missing. E.g. if we want to change memory
 * clocks the right way we must adapt a lot of other settings. This includes
 * MCR and DTRx timing registers (0xb80001000, 0xb8001008, ...) and a DLL reset
 * so that hardware operates in the allowed limits. This is far too complex
 * without official support. Avoid this for now.
 */
		rclk->min = rclk->max = rclk->startup;
		break;
	}

	return 0;
}

static struct clk_hw *rtcl_get_clkhw(struct of_phandle_args *clkspec, void *prv)
{
	unsigned int idx = clkspec->args[0];

	if (idx >= CLK_COUNT) {
		pr_err("%s: Invalid index %u\n", __func__, idx);
		return ERR_PTR(-EINVAL);
	}

	return &rtcl_ccu->clks[idx].hw;
}

static int rtcl_ccu_register_clocks(void)
{
	int clk_idx, ret;

	for (clk_idx = 0; clk_idx < CLK_COUNT; clk_idx++) {
		ret = rtcl_register_clkhw(clk_idx);
		if (ret) {
			pr_err("%s: Couldn't register %s clock\n",
				__func__, rtcl_clk_info[clk_idx].display_name);
			goto err_hw_unregister;
		}
	}

	ret = of_clk_add_hw_provider(rtcl_ccu->np, rtcl_get_clkhw, rtcl_ccu);
	if (ret) {
		pr_err("%s: Couldn't register clock provider of %s\n",
			__func__, of_node_full_name(rtcl_ccu->np));
		goto err_hw_unregister;
	}

	return 0;

err_hw_unregister:
	for (--clk_idx; clk_idx >= 0; --clk_idx)
		clk_hw_unregister(&rtcl_ccu->clks[clk_idx].hw);

	return ret;
}

int rtcl_init_sram(void)
{
	struct gen_pool *sram_pool;
	phys_addr_t sram_pbase;
	unsigned long sram_vbase;
	struct device_node *node;
	struct platform_device *pdev = NULL;
	void *dram_start;
	int dram_size;
	const char *wrn = ", rate setting disabled.\n";

	switch (rtcl_ccu->soc) {
	case SOC_RTL838X:
		dram_start = &rtcl_838x_dram_start;
		dram_size = rtcl_838x_dram_size;
		break;
	case SOC_RTL839X:
		dram_start = &rtcl_839x_dram_start;
		dram_size = rtcl_839x_dram_size;
		break;
	default:
		return -ENXIO;
	}

	for_each_compatible_node(node, NULL, "mmio-sram") {
		pdev = of_find_device_by_node(node);
		if (pdev) {
			of_node_put(node);
			break;
		}
	}

	if (!pdev) {
		dev_warn(&rtcl_ccu->pdev->dev, "no SRAM device found%s", wrn);
		return -ENXIO;
	}

	sram_pool = gen_pool_get(&pdev->dev, NULL);
	if (!sram_pool) {
		dev_warn(&rtcl_ccu->pdev->dev, "SRAM pool unavailable%s", wrn);
		goto err_put_device;
	}

	sram_vbase = gen_pool_alloc(sram_pool, dram_size);
	if (!sram_vbase) {
		dev_warn(&rtcl_ccu->pdev->dev, "can not allocate SRAM%s", wrn);
		goto err_put_device;
	}

	sram_pbase = gen_pool_virt_to_phys(sram_pool, sram_vbase);
	memcpy((void *)sram_pbase, dram_start, dram_size);
	flush_icache_range((unsigned long)sram_pbase, (unsigned long)(sram_pbase + dram_size));

	switch (rtcl_ccu->soc) {
	case SOC_RTL838X:
		RTCL_SRAM_FUNC(838x, sram_pbase, set_rate);
		break;
	case SOC_RTL839X:
		RTCL_SRAM_FUNC(839x, sram_pbase, set_rate);
		break;
	}

	rtcl_ccu->sram.pmark = (int *)((void *)sram_pbase + (dram_size - 4));
	rtcl_ccu->sram.vbase = sram_vbase;

	return 0;

err_put_device:
	put_device(&pdev->dev);

	return -ENXIO;
}

void rtcl_ccu_log_early(void)
{
	char meminfo[80], clkinfo[255], msg[255] = "rtl83xx-clk: initialized";

	sprintf(meminfo, " (%d Bit DDR%d)", rtcl_ccu->dram.buswidth, rtcl_ccu->dram.type);
	for (int clk_idx = 0; clk_idx < CLK_COUNT; clk_idx++) {
		sprintf(clkinfo, ", %s %lu MHz", rtcl_clk_info[clk_idx].display_name,
			rtcl_ccu->clks[clk_idx].startup / 1000000);
		if (clk_idx == CLK_MEM)
			strcat(clkinfo, meminfo);
		strcat(msg, clkinfo);
	}
	pr_info("%s\n", msg);
}

void rtcl_ccu_log_late(void)
{
	struct rtcl_clk *rclk;
	bool overclock = false;
	char clkinfo[80], msg[255] = "rate setting enabled";

	for (int clk_idx = 0; clk_idx < CLK_COUNT; clk_idx++) {
		rclk = &rtcl_ccu->clks[clk_idx];
		overclock |= rclk->max > rclk->startup;
		sprintf(clkinfo, ", %s %lu-%lu MHz", rtcl_clk_info[clk_idx].display_name,
			rclk->min / 1000000, rclk->max / 1000000);
		strcat(msg, clkinfo);
	}
	if (overclock)
		strcat(msg, ", OVERCLOCK AT OWN RISK");

	dev_info(&rtcl_ccu->pdev->dev, "%s\n", msg);
}

/*
 * Early registration: This module provides core startup clocks that are needed
 * for generic SOC init and for further builtin devices (e.g. UART). Register
 * asap via clock framework.
 */

static void __init rtcl_probe_early(struct device_node *np)
{
	if (rtcl_ccu_create(np))
		return;

	if (rtcl_ccu_register_clocks())
		kfree(rtcl_ccu);
	else
		rtcl_ccu_log_early();
}

CLK_OF_DECLARE_DRIVER(rtl838x_clk, "realtek,rtl8380-clock", rtcl_probe_early);
CLK_OF_DECLARE_DRIVER(rtl839x_clk, "realtek,rtl8390-clock", rtcl_probe_early);

/*
 * Late registration: Finally register as normal platform driver. At this point
 * we can make use of other modules like SRAM.
 */

static const struct of_device_id rtcl_dt_ids[] = {
	{ .compatible = "realtek,rtl8380-clock" },
	{ .compatible = "realtek,rtl8390-clock" },
	{}
};

static int rtcl_probe_late(struct platform_device *pdev)
{
	int ret;

	if (!rtcl_ccu) {
		dev_err(&pdev->dev, "early initialization not run");
		return -ENXIO;
	}
	rtcl_ccu->pdev = pdev;
	ret = rtcl_init_sram();
	if (ret)
		return ret;

	rtcl_ccu_log_late();

	return 0;
}

static struct platform_driver rtcl_platform_driver = {
	.driver = {
		.name = "rtl83xx-clk",
		.of_match_table = rtcl_dt_ids,
	},
	.probe = rtcl_probe_late,
};

static int __init rtcl_init_subsys(void)
{
	return platform_driver_register(&rtcl_platform_driver);
}

/*
 * The driver does not know when SRAM module has finally loaded. With an
 * arch_initcall() we might overtake SRAM initialization. Be polite and give the
 * system a little more time.
 */

subsys_initcall(rtcl_init_subsys);
