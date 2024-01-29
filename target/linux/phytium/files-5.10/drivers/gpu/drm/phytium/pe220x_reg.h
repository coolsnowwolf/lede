/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium Pe220x display engine register
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PE220X_REG_H__
#define __PE220X_REG_H__

#include "phytium_reg.h"

/* dc register */
#define PE220X_DC_CLOCK_CONTROL				0x0000
#define DC1_CORE_RESET					(1<<18)
#define DC0_CORE_RESET					(1<<17)
#define AXI_RESET					(1<<16)
#define AHB_RESET					(1<<12)

#define PE220X_DC_CMD_REGISTER(pipe)			(PE220X_DC_BASE(0) + 0x00F0 + 0x4*(pipe))
#define FLAG_REPLY					(1<<31)
#define FLAG_REQUEST					(1<<30)
#define CMD_PIXEL_CLOCK					(0x0 << 28)
#define CMD_BACKLIGHT					(0x1 << 28)
#define CMD_DC_DP_RESET					(0x3 << 28)
#define BACKLIGHT_SHIFT					21
#define BACKLIGHT_MASK					0x7f
#define BACKLIGHT_MAX					100
#define BACKLIGHT_ENABLE				(101 << BACKLIGHT_SHIFT)
#define BACKLIGHT_DISABLE				(102 << BACKLIGHT_SHIFT)
#define PANEL_POWER_ENABLE				(103 << BACKLIGHT_SHIFT)
#define PANEL_POWER_DISABLE				(104 << BACKLIGHT_SHIFT)
#define PIXEL_CLOCK_MASK				(0x1fffff)

#define PE220X_DC_FRAMEBUFFER_Y_HI_ADDRESS		0x1404
#define PREFIX_MASK					0xff
#define PREFIX_SHIFT					32

#define	PE220X_DC_CURSOR_HI_ADDRESS			0x1490
#define CURSOR_PREFIX_MASK				0xff
#define CURSOR_PREFIX_SHIFT				32

#define PE220X_DC_FRAMEBUFFER_U_HI_ADDRESS		0x1534
#define U_PREFIX_MASK					0xff
#define U_PREFIX_SHIFT					32

#define PE220X_DC_FRAMEBUFFER_V_HI_ADDRESS		0x153c
#define V_PREFIX_MASK					0xff
#define V_PREFIX_SHIFT					32

/* dp register */
#define PE220X_DP_CONTROLLER_RESET			0x0850
#define DP_RESET					0x1

/* address transform register */
#define PE220X_DC_ADDRESS_TRANSFORM_SRC_ADDR		0x0
#define SRC_ADDR_OFFSET					22
#define SRC_ADDR_MASK					0xffffffffff

#define PE220X_DC_ADDRESS_TRANSFORM_SIZE			0x4
#define ADDRESS_TRANSFORM_ENABLE			(0x1 << 31)
#define SIZE_OFFSET					22

#define PE220X_DC_ADDRESS_TRANSFORM_DST_ADDR		0x8
#define DST_ADDR_OFFSET					22

#define PE220X_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS	0x48
#define DC_DP_RESET_STATUS(pipe)			(1 << pipe)
#define DP_SPREAD_ENABLE(pipe)				(0x8 << pipe)

#define PE220X_DC_ADDRESS_TRANSFORM_BACKLIGHT_VALUE	0x4c
#define BACKLIGHT_VALUE_MASK				(0x7f)
#define BACKLIGHT_VALUE_SHIFT				16

/* phy register start */
#define PE220X_PHY_BASE(pipe)				(0x100000*pipe)

#define PE220X_PHY_PIPE_RESET(pipe)			(PE220X_PHY_BASE(pipe) + 0x40254)
#define RESET						0x0
#define RESET_DEASSERT					0x1

#define PE220X_PHY_MODE(pipe)				(PE220X_PHY_BASE(pipe) + 0x40034)
#define LANE_BIT					(0x3)
#define LANE_BIT_SHIFT					0x2

#define PE220X_PHY_LINK_CFG(pipe)			(PE220X_PHY_BASE(pipe) + 0x40044)
#define LANE_MASTER					0x1
#define LANE_MASTER_SHIFT				1

#define PE220X_PHY_PLL_EN(pipe)				(PE220X_PHY_BASE(pipe) + 0x40214)
#define PLL_EN						0x1
#define PLL_EN_SHIFT					1

#define PE220X_PHY_PMA_WIDTH(pipe)			(PE220X_PHY_BASE(pipe) + 0x4021c)
#define	BIT_20						0x5
#define	BIT_20_SHIFT					4

#define PE220X_PHY_PLL_SOURCE_SEL(pipe)			(PE220X_PHY_BASE(pipe) + 0x4004C)

#define PE220X_PHY_PMA0_POWER(pipe)			(PE220X_PHY_BASE(pipe) + 0x402bc)
#define	A0_ACTIVE					0x1
#define	A0_ACTIVE_SHIFT					8
#define	A3_POWERDOWN3					0x8
#define	A3_POWERDOWN3_SHIFT				8

#define PE220X_PHY_LINK_RESET(pipe)			(PE220X_PHY_BASE(pipe) + 0x40258)
#define LINK_RESET					0x1
#define LINK_RESET_MASK					0x1
#define LINTK_RESET_SHIFT				0x1

#define PE220X_PHY_SGMII_DPSEL_INIT(pipe)		(PE220X_PHY_BASE(pipe) + 0x40260)
#define DP_SEL						0x1

#define PE220X_PHY_APB_RESET(pipe)			(PE220X_PHY_BASE(pipe) + 0x40250)
#define	APB_RESET					0x1

/* phy origin register */
#define PE220X_PHY_PLL_CFG(pipe)				(PE220X_PHY_BASE(pipe) + 0x30038)
#define SINGLE_LINK					0x0

#define PE220X_PHY_PMA_CONTROL(pipe)			(PE220X_PHY_BASE(pipe) + 0x3800c)
#define CONTROL_ENABLE					0x1
#define CONTROL_ENABLE_MASK				0x1
#define CONTROL_ENABLE_SHIFT				0x1

#define PE220X_PHY_PMA_CONTROL2(pipe)			(PE220X_PHY_BASE(pipe) + 0x38004)
#define PLL0_LOCK_DONE					(0x1 << 6)

#define PE220X_PHY_PLL0_CLK_SEL(pipe)			(PE220X_PHY_BASE(pipe) + 0X684)
#define PLL_LINK_RATE_162000				0xf01
#define PLL_LINK_RATE_270000				0x701
#define PLL_LINK_RATE_540000				0x301
#define PLL_LINK_RATE_810000				0x200

#define PE220X_PHY_HSCLK0_SEL(pipe)			(PE220X_PHY_BASE(pipe) + 0x18398)
#define HSCLK_LINK_0					0x0
#define HSCLK_LINK_1					0x1

#define PE220X_PHY_HSCLK0_DIV(pipe)			(PE220X_PHY_BASE(pipe) + 0x1839c)
#define HSCLK_LINK_RATE_162000				0x2
#define HSCLK_LINK_RATE_270000				0x1
#define HSCLK_LINK_RATE_540000				0x0
#define HSCLK_LINK_RATE_810000				0x0

#define PE220X_PHY_PLLDRC0_CTRL(pipe)			(PE220X_PHY_BASE(pipe) + 0x18394)
#define PLLDRC_LINK0					0x1
#define PLLDRC_LINK1					0x9

#define PE220X_PHY_PLL0_DSM_M0(pipe)			(PE220X_PHY_BASE(pipe) + 0x250)
#define PLL0_DSM_M0					0x4
#define PE220X_PHY_PLL0_VCOCAL_START(pipe)		(PE220X_PHY_BASE(pipe) + 0x218)
#define PLL0_VCOCAL_START				0xc5e
#define PE220X_PHY_PLL0_VCOCAL_CTRL(pipe)		(PE220X_PHY_BASE(pipe) + 0x208)
#define PLL0_VCOCAL_CTRL				0x3

#define PE220X_PHY_PLL0_CP_PADJ(pipe)			(PE220X_PHY_BASE(pipe) + 0x690)
#define PE220X_PHY_PLL0_CP_IADJ(pipe)			(PE220X_PHY_BASE(pipe) + 0x694)
#define PE220X_PHY_PLL0_CP_FILT_PADJ(pipe)		(PE220X_PHY_BASE(pipe) + 0x698)
#define PE220X_PHY_PLL0_INTDIV(pipe)			(PE220X_PHY_BASE(pipe) + 0x240)
#define PE220X_PHY_PLL0_FRACDIVL(pipe)			(PE220X_PHY_BASE(pipe) + 0x244)
#define PE220X_PHY_PLL0_FRACDIVH(pipe)			(PE220X_PHY_BASE(pipe) + 0x248)
#define PE220X_PHY_PLL0_HIGH_THR(pipe)			(PE220X_PHY_BASE(pipe) + 0x24c)
#define PE220X_PHY_PLL0_PDIAG_CTRL(pipe)			(PE220X_PHY_BASE(pipe) + 0x680)
#define PE220X_PHY_PLL0_VCOCAL_PLLCNT_START(pipe)	(PE220X_PHY_BASE(pipe) + 0x220)
#define PE220X_PHY_PLL0_LOCK_PEFCNT(pipe)		(PE220X_PHY_BASE(pipe) + 0x270)
#define PE220X_PHY_PLL0_LOCK_PLLCNT_START(pipe)		(PE220X_PHY_BASE(pipe) + 0x278)
#define PE220X_PHY_PLL0_LOCK_PLLCNT_THR(pipe)		(PE220X_PHY_BASE(pipe) + 0x27c)

#define	PE220X_PHY_PLL0_TX_PSC_A0(pipe)			(PE220X_PHY_BASE(pipe) + 0x18400)
#define PLL0_TX_PSC_A0					0xfb
#define	PE220X_PHY_PLL0_TX_PSC_A2(pipe)			(PE220X_PHY_BASE(pipe) + 0x18408)
#define PLL0_TX_PSC_A2					0x4aa
#define	PE220X_PHY_PLL0_TX_PSC_A3(pipe)			(PE220X_PHY_BASE(pipe) + 0x1840c)
#define PLL0_TX_PSC_A3					0x4aa
#define	PE220X_PHY_PLL0_RX_PSC_A0(pipe)			(PE220X_PHY_BASE(pipe) + 0x28000)
#define PLL0_RX_PSC_A0					0x0
#define	PE220X_PHY_PLL0_RX_PSC_A2(pipe)			(PE220X_PHY_BASE(pipe) + 0x28008)
#define PLL0_RX_PSC_A2					0x0
#define	PE220X_PHY_PLL0_RX_PSC_A3(pipe)			(PE220X_PHY_BASE(pipe) + 0x2800C)
#define PLL0_RX_PSC_A3					0x0
#define	PE220X_PHY_PLL0_RX_PSC_CAL(pipe)			(PE220X_PHY_BASE(pipe) + 0x28018)
#define PLL0_RX_PSC_CAL					0x0

#define PE220X_PHY_PLL0_XCVR_CTRL(pipe)			(PE220X_PHY_BASE(pipe) + 0x183a8)
#define PLL0_XCVR_CTRL					0xf

#define PE220X_PHY_PLL0_RX_GCSM1_CTRL(pipe)		(PE220X_PHY_BASE(pipe) + 0x28420)
#define PLL0_RX_GCSM1_CTRL				0x0
#define PE220X_PHY_PLL0_RX_GCSM2_CTRL(pipe)		(PE220X_PHY_BASE(pipe) + 0x28440)
#define PLL0_RX_GCSM2_CTRL				0x0
#define PE220X_PHY_PLL0_RX_PERGCSM_CTRL(pipe)		(PE220X_PHY_BASE(pipe) + 0x28460)
#define PLL0_RX_PERGCSM_CTRL				0x0

/* swing and emphasis */
#define PE220X_PHY_PLL0_TX_DIAG_ACYA(pipe)		(PE220X_PHY_BASE(pipe) + 0x1879c)
#define LOCK						1
#define UNLOCK						0

#define PE220X_PHY_PLL0_TX_TXCC_CTRL(pipe)		(PE220X_PHY_BASE(pipe) + 0x18100)
#define TX_TXCC_CTRL					0x8a4

#define PE220X_PHY_PLL0_TX_DRV(pipe)			(PE220X_PHY_BASE(pipe) + 0x18318)
#define TX_DRV						0x3

#define PE220X_PHY_PLL0_TX_MGNFS(pipe)			(PE220X_PHY_BASE(pipe) + 0x18140)

#define PE220X_PHY_PLL0_TX_CPOST(pipe)			(PE220X_PHY_BASE(pipe) + 0x18130)

#endif /* __PE220X_REG_H__ */
