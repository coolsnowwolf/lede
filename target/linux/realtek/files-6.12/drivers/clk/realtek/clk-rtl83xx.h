/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Realtek RTL83XX clock headers
 * Copyright (C) 2022 Markus Stockhausen <markus.stockhausen@gmx.de>
 */

/*
 * Switch registers (e.g. PLL)
 */

#define RTL_SW_CORE_BASE			(0xbb000000)

#define RTL838X_PLL_GLB_CTRL			(0x0fc0)
#define RTL838X_PLL_CPU_CTRL0			(0x0fc4)
#define RTL838X_PLL_CPU_CTRL1			(0x0fc8)
#define RTL838X_PLL_LXB_CTRL0			(0x0fd0)
#define RTL838X_PLL_LXB_CTRL1			(0x0fd4)
#define RTL838X_PLL_MEM_CTRL0			(0x0fdc)
#define RTL838X_PLL_MEM_CTRL1			(0x0fe0)

#define RTL839X_PLL_GLB_CTRL			(0x0024)
#define RTL839X_PLL_CPU_CTRL0			(0x0028)
#define RTL839X_PLL_CPU_CTRL1			(0x002c)
#define RTL839X_PLL_LXB_CTRL0			(0x0038)
#define RTL839X_PLL_LXB_CTRL1			(0x003c)
#define RTL839X_PLL_MEM_CTRL0			(0x0048)
#define RTL839X_PLL_MEM_CTRL1			(0x004c)

#define RTL_PLL_CTRL0_CMU_SEL_PREDIV(v)		(((v) >> 0) & 0x3)
#define RTL_PLL_CTRL0_CMU_SEL_DIV4(v)		(((v) >> 2) & 0x1)
#define RTL_PLL_CTRL0_CMU_NCODE_IN(v)		(((v) >> 4) & 0xff)
#define RTL_PLL_CTRL0_CMU_DIVN2(v)		(((v) >> 12) & 0xff)

#define RTL838X_GLB_CTRL_EN_CPU_PLL_MASK	(1 << 0)
#define RTL838X_GLB_CTRL_EN_LXB_PLL_MASK	(1 << 1)
#define RTL838X_GLB_CTRL_EN_MEM_PLL_MASK	(1 << 2)
#define RTL838X_GLB_CTRL_CPU_PLL_READY_MASK	(1 << 8)
#define RTL838X_GLB_CTRL_LXB_PLL_READY_MASK	(1 << 9)
#define RTL838X_GLB_CTRL_MEM_PLL_READY_MASK	(1 << 10)
#define RTL838X_GLB_CTRL_CPU_PLL_SC_MUX_MASK	(1 << 12)

#define RTL838X_PLL_CTRL1_CMU_DIVN2_SELB(v)	(((v) >> 26) & 0x1)
#define RTL838X_PLL_CTRL1_CMU_DIVN3_SEL(v)	(((v) >> 27) & 0x3)

#define RTL839X_GLB_CTRL_CPU_CLKSEL_MASK	(1 << 11)
#define RTL839X_GLB_CTRL_MEM_CLKSEL_MASK	(1 << 12)
#define RTL839X_GLB_CTRL_LXB_CLKSEL_MASK	(1 << 13)

#define RTL839X_PLL_CTRL1_CMU_DIVN2_SELB(v)	(((v) >> 2) & 0x1)
#define RTL839X_PLL_CTRL1_CMU_DIVN3_SEL(v)	(((v) >> 0) & 0x3)

/*
 * Core registers (e.g. memory controller)
 */

#define RTL_SOC_BASE				(0xB8000000)

#define RTL_MC_MCR				(0x1000)
#define RTL_MC_DCR				(0x1004)
#define RTL_MC_DTR0				(0x1008)
#define RTL_MC_DTR1				(0x100c)
#define RTL_MC_DTR2				(0x1010)
#define RTL_MC_DMCR				(0x101c)
#define RTL_MC_DACCR				(0x1500)
#define RTL_MC_DCDR				(0x1060)

#define RTL_MC_MCR_DRAMTYPE(v)			((((v) >> 28) & 0xf) + 1)
#define RTL_MC_DCR_BUSWIDTH(v)			(8 << (((v) >> 24) & 0xf))

/*
 * Other stuff
 */

#define RTL_SRAM_MARKER				(0x5eaf00d5)
#define RTL_SRAM_BASE				(0x9f000000)
