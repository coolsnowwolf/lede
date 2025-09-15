/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PX210_REG_H__
#define __PX210_REG_H__

#include "phytium_reg.h"

/******************************dc register start******************************************/
#define PX210_DC_CLOCK_CONTROL				0x0000
	#define SOFT_RESET					(1<<12)
#define PX210_DC_CLOCK_IDLE				0x0004
	#define IS_IDLE						(1<<16)
/******************************dc register end********************************************/

/******************************dcreq register start**************************************/
#define PX210_DCREQ_PLANE0_ADDR_START			0x00
#define PX210_DCREQ_PLANE0_ADDR_END			0x04
#define PX210_DCREQ_PLANE1_ADDR_START			0x08
#define PX210_DCREQ_PLANE1_ADDR_END			0x0c
#define	PX210_DCREQ_PLANE0_CONFIG			0x10
	#define DCREQ_NO_LOSSY					(0 << 0)
	#define DCREQ_LOSSY					(1 << 0)
	#define DCREQ_TILE_TYPE_MASK				(0x3 << 1)
	#define DCREQ_TILE_TYPE_MODE0				(0x1 << 1)
	#define DCREQ_TILE_TYPE_MODE3				(0x2 << 1)
	#define	DCREQ_COLOURFORMAT_MASK				(0x7f << 8)
	#define DCREQ_COLOURFORMAT_RGB565			(0x5 << 8)
	#define DCREQ_COLOURFORMAT_ARGB1555			(0x4 << 8)
	#define DCREQ_COLOURFORMAT_ARGB4444			(0x02 << 8)
	#define	DCREQ_COLOURFORMAT_BGRA8888			(0x29 << 8)
	#define DCREQ_COLOURFORMAT_ARGB2101010			(0xe << 8)
	#define DCREQ_COLOURFORMAT_YUYV				(0x59 << 8)
	#define DCREQ_COLOURFORMAT_UYVY				(0x5b << 8)
	#define	DCREQ_ARGBSWIZZLE_MASK				(0xf << 4)
	#define	DCREQ_ARGBSWIZZLE_ARGB				(0X0 << 4)
	#define	DCREQ_ARGBSWIZZLE_BGRA				(0XC << 4)
	#define	DCREQ_MODE_MASK					(1 << 16)
	#define DCREQ_MODE_LINEAR				(0 << 16)
	#define DCREQ_MODE_TILE					(1 << 16)
#define	PX210_DCREQ_PLANE1_CONFIG(pipe)			0x14
#define PX210_DCREQ_PLANE0_CLEAR_COLOR_L			0x18
#define PX210_DCREQ_PLANE0_CLEAR_COLOR_H			0x1C
#define PX210_DCREQ_PLANE1_CLEAR_COLOR_L			0x20
#define PX210_DCREQ_PLANE1_CLEAR_COLOR_H			0x24
#define PX210_DCREQ_CMD_REGISTER				0x38
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
#define PX210_DCREQ_FBCD_CLOCK_CONFIG			0x3c
#define PX210_DCREQ_PIX_DMA_PREFIX			0x50
	#define PREFIX_MASK					0xff
	#define PREFIX_SHIFT					32
#define PX210_DCREQ_FRAME_START				0x54
#define PX210_DCREQ_FILTER_CONFIG			0x58
#define PX210_DCREQ_CONTROL				0x5C
	#define DC_REQ_ENABLE					(1<<0)
#define PX210_DCREQ_MSI_CLEAR				0x60
	#define MSI_CLEAR					0x0
#define	PX210_DCREQ_RESET				0x68
	#define DCREQ_RESET					(0x3 << 0)
	#define DCREQ_RESET_MASK				0x3
#define PX210_DCREQ_PLAN					0x94
	#define DCREQ_PLAN_A					0x0
	#define DCREQ_PLAN_B					0X5
/******************************dcreq register end**************************************/

/******************************address transform register start**************************/
#define PX210_GPU_ADDRESS_TRANSFORM_SRC_ADDR		0x0
#define PX210_GPU_ADDRESS_TRANSFORM_SIZE			0x4
#define PX210_GPU_ADDRESS_TRANSFORM_DST_ADDR		0x8

#define PX210_DC_ADDRESS_TRANSFORM_SRC_ADDR		0x24
	#define SRC_ADDR_OFFSET					22
	#define SRC_ADDR_MASK					0xffffffffff
#define PX210_DC_ADDRESS_TRANSFORM_SIZE			0x28
	#define ADDRESS_TRANSFORM_ENABLE			(0x1 << 31)
	#define SIZE_OFFSET					22
#define PX210_DC_ADDRESS_TRANSFORM_DST_ADDR		0x2c
	#define DST_ADDR_OFFSET					22
#define PX210_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS	0x48
	#define DC_DP_RESET_STATUS(pipe)			(1 << pipe)
	#define DP_SPREAD_ENABLE(pipe)				(0x8 << pipe)
#define PX210_DC_ADDRESS_TRANSFORM_BACKLIGHT_VALUE	0x4c
	#define BACKLIGHT_VALUE_MASK				(0x7f)
	#define BACKLIGHT_VALUE_SHIFT				16
/******************************address transform register end**************************/

/******************************phy register start******************************************/
/* self define */
#define PX210_PHY0_PIPE_RESET					0x40104
	#define RESET						0x0
	#define RESET_DEASSERT					0x1
#define PX210_PHY1_PIPE_RESET				0x100100
	#define PHY1_PIPE_RESET					0x0
	#define PHY1_PIPE_RESET_DEASSERT			0x4

#define PX210_PHY1_EN_REFCLK				0x100070

#define	PX210_PHY0_MODE					0x40088
	#define LANE_BIT					(0x3)
	#define LANE_BIT_SHIFT					0x2
#define	PX210_PHY1_SEL					0x100004
	#define PHY1_DP_LANE_BIT				0x1
	#define PHY1_DP_LANE_BIT_SHIFT				2

#define	PX210_PHY0_LINK_CFG				0x40044
	#define LANE_MASTER					0x1
	#define LANE_MASTER_SHIFT				1

#define PX210_PHY0_PLL_EN				0x40010
	#define PLL_EN						0x1
	#define PLL_EN_SHIFT					1
#define PX210_PHY0_PMA_WIDTH				0x40020
	#define	BIT_20						0x5
	#define	BIT_20_SHIFT					4

#define PX210_PHY0_PMA0_POWER				0x40014
#define PX210_PHY0_PMA1_POWER				0x40018
	#define	A0_ACTIVE					0x1
	#define	A0_ACTIVE_SHIFT					8
	#define	A3_POWERDOWN3					0x8
	#define	A3_POWERDOWN3_SHIFT				8

#define PX210_PHY1_PMA_MISC				0x1000a0
	#define PHY1_PLL_EN					0x1
	#define PHY1_PLL_EN_MASK				1
	#define PHY1_PLL_EN_SHIFT				8
	#define PHY1_BIT_20					0x5
	#define PHY1_BIT_20_SHIFT				9
	#define PHY1_A0_ACTIVE					0x1
	#define PHY1_A0_ACTIVE_SHIFT				2
	#define PHY1_A0_ACTIVE_MASK				0x3f
	#define PHY1_A3_POWERDOWN3				0x8
	#define PHY1_A3_POWERDOWN3_MASK				0x3f
	#define PHY1_A3_POWERDOWN3_SHIFT			2

#define PX210_PHY0_LINK_RESET				0x40108
	#define LINK_RESET					0x1
	#define LINK_RESET_MASK					0x1
	#define LINTK_RESET_SHIFT				0x1

#define PX210_PHY0_APB_RESET				0x40100
	#define	APB_RESET					0x1
#define PX210_PHY1_APB_RESET				0x100104
	#define PHY1_APB_RESET					0x4

/* phy origin register */
#define PX210_PHY0_PLL_CFG				0x30038
#define PX210_PHY1_PLL_CFG				0xb0038
	#define SINGLE_LINK					0x0
	#define DOUBLE_LINK					0x2

#define PX210_PHY0_PMA_CONTROL				0x3800c
#define PX210_PHY1_PMA_CONTROL				0xb800c
	#define CONTROL_ENABLE					0x1
	#define CONTROL_ENABLE_MASK				0x1
	#define CONTROL_ENABLE_SHIFT				0x1

#define PX210_PHY0_PMA_CONTROL2				0x38004
#define PX210_PHY1_PMA_CONTROL2				0xb8004
	#define PLL0_LOCK_DONE					(0x1 << 6)
	#define PLL1_LOCK_DONE					(0x1 << 7)

#define PX210_PHY0_PLL0_CLK_SEL				0X684
#define PX210_PHY0_PLL1_CLK_SEL				0x704
#define PX210_PHY1_PLL_CLK_SEL				0X80684
	#define PLL_LINK_RATE_162000				0xf01
	#define PLL_LINK_RATE_270000				0x701
	#define PLL_LINK_RATE_540000				0x301
	#define PLL_LINK_RATE_810000				0x200

#define PX210_PHY0_HSCLK0_SEL				0x18398
#define PX210_PHY0_HSCLK1_SEL				0x1a398
#define PX210_PHY1_HSCLK_SEL				0x90398
	#define HSCLK_LINK_0					0x0
	#define HSCLK_LINK_1					0x1

#define PX210_PHY0_HSCLK0_DIV				0x1839c
#define PX210_PHY0_HSCLK1_DIV				0x1a39c
#define PX210_PHY1_HSCLK_DIV				0x9039c
	#define HSCLK_LINK_RATE_162000				0x2
	#define HSCLK_LINK_RATE_270000				0x1
	#define HSCLK_LINK_RATE_540000				0x0
	#define HSCLK_LINK_RATE_810000				0x0

#define PX210_PHY0_PLLDRC0_CTRL				0x18394
#define PX210_PHY0_PLLDRC1_CTRL				0x1a394
#define PX210_PHY1_PLLDRC_CTRL				0x90394
	#define PLLDRC_LINK0					0x1
	#define PLLDRC_LINK1					0x9

#define PX210_PHY0_PLL0_DSM_M0				0x250
#define PX210_PHY1_PLL0_DSM_M0				0x80250
	#define PLL0_DSM_M0					0x4
#define PX210_PHY0_PLL0_VCOCAL_START			0x218
#define PX210_PHY1_PLL0_VCOCAL_START			0x80218
	#define PLL0_VCOCAL_START				0xc5e
#define PX210_PHY0_PLL0_VCOCAL_CTRL			0x208
#define PX210_PHY1_PLL0_VCOCAL_CTRL			0x80208
	#define PLL0_VCOCAL_CTRL				0x3

#define PX210_PHY0_PLL1_DSM_M0				0x350
	#define PLL1_DSM_M0					0x4
#define PX210_PHY0_PLL1_VCOCAL_START			0x318
	#define PLL1_VCOCAL_START				0xc5e
#define PX210_PHY0_PLL1_VCOCAL_CTRL			0x308
	#define PLL1_VCOCAL_CTRL				0x3

#define PX210_PHY0_PLL0_CP_PADJ				0x690
#define PX210_PHY0_PLL0_CP_IADJ				0x694
#define PX210_PHY0_PLL0_CP_FILT_PADJ			0x698
#define PX210_PHY0_PLL0_INTDIV				0x240
#define PX210_PHY0_PLL0_FRACDIVL				0x244
#define PX210_PHY0_PLL0_FRACDIVH				0x248
#define PX210_PHY0_PLL0_HIGH_THR				0x24c
#define PX210_PHY0_PLL0_PDIAG_CTRL			0x680
#define PX210_PHY0_PLL0_VCOCAL_PLLCNT_START		0x220
#define PX210_PHY0_PLL0_LOCK_PEFCNT			0x270
#define PX210_PHY0_PLL0_LOCK_PLLCNT_START		0x278
#define PX210_PHY0_PLL0_LOCK_PLLCNT_THR			0x27c

#define PX210_PHY0_PLL1_CP_PADJ				0x710
#define PX210_PHY0_PLL1_CP_IADJ				0x714
#define PX210_PHY0_PLL1_CP_FILT_PADJ			0x718
#define PX210_PHY0_PLL1_INTDIV				0x340
#define PX210_PHY0_PLL1_FRACDIVL				0x344
#define PX210_PHY0_PLL1_FRACDIVH				0x348
#define PX210_PHY0_PLL1_HIGH_THR				0x34c
#define PX210_PHY0_PLL1_PDIAG_CTRL			0x700
#define PX210_PHY0_PLL1_VCOCAL_PLLCNT_START		0x320
#define PX210_PHY0_PLL1_LOCK_PEFCNT			0x370
#define PX210_PHY0_PLL1_LOCK_PLLCNT_START		0x378
#define PX210_PHY0_PLL1_LOCK_PLLCNT_THR			0x37c

#define PX210_PHY1_PLL0_CP_PADJ				0x80690
#define PX210_PHY1_PLL0_CP_IADJ				0x80694
#define PX210_PHY1_PLL0_CP_FILT_PADJ			0x80698
#define PX210_PHY1_PLL0_INTDIV				0x80240
#define PX210_PHY1_PLL0_FRACDIVL				0x80244
#define PX210_PHY1_PLL0_FRACDIVH				0x80248
#define PX210_PHY1_PLL0_HIGH_THR				0x8024c
#define PX210_PHY1_PLL0_PDIAG_CTRL			0x80680
#define PX210_PHY1_PLL0_VCOCAL_PLLCNT_START		0x80220
#define PX210_PHY1_PLL0_LOCK_PEFCNT			0x80270
#define PX210_PHY1_PLL0_LOCK_PLLCNT_START		0x80278
#define PX210_PHY1_PLL0_LOCK_PLLCNT_THR			0x8027c

#define	PX210_PHY0_PLL0_TX_PSC_A0			0x18400
#define	PX210_PHY1_PLL0_TX_PSC_A0			0x90400
	#define PLL0_TX_PSC_A0					0xfb
#define	PX210_PHY0_PLL0_TX_PSC_A2			0x18408
#define	PX210_PHY1_PLL0_TX_PSC_A2			0x90408
	#define PLL0_TX_PSC_A2					0x4aa
#define	PX210_PHY0_PLL0_TX_PSC_A3			0x1840c
#define	PX210_PHY1_PLL0_TX_PSC_A3			0x9040c
	#define PLL0_TX_PSC_A3					0x4aa
#define	PX210_PHY0_PLL0_RX_PSC_A0			0x28000
#define	PX210_PHY1_PLL0_RX_PSC_A0			0xa0000
	#define PLL0_RX_PSC_A0					0x0
#define	PX210_PHY0_PLL0_RX_PSC_A2			0x28008
#define	PX210_PHY1_PLL0_RX_PSC_A2			0xa0008
	#define PLL0_RX_PSC_A2					0x0
#define	PX210_PHY0_PLL0_RX_PSC_A3			0x2800C
#define	PX210_PHY1_PLL0_RX_PSC_A3			0xa000C
	#define PLL0_RX_PSC_A3					0x0
#define	PX210_PHY0_PLL0_RX_PSC_CAL			0x28018
#define	PX210_PHY1_PLL0_RX_PSC_CAL			0xa0018
	#define PLL0_RX_PSC_CAL					0x0

#define	PX210_PHY0_PLL1_TX_PSC_A0			0x1a400
	#define PLL1_TX_PSC_A0					0xfb
#define	PX210_PHY0_PLL1_TX_PSC_A2			0x1a408
	#define PLL1_TX_PSC_A2					0x4aa
#define	PX210_PHY0_PLL1_TX_PSC_A3			0x1a40c
	#define PLL1_TX_PSC_A3					0x4aa
#define	PX210_PHY0_PLL1_RX_PSC_A0			0x2a000
	#define PLL1_RX_PSC_A0					0x0
#define	PX210_PHY0_PLL1_RX_PSC_A2			0x2a008
	#define PLL1_RX_PSC_A2					0x0
#define	PX210_PHY0_PLL1_RX_PSC_A3			0x2a00C
	#define PLL1_RX_PSC_A3					0x0
#define	PX210_PHY0_PLL1_RX_PSC_CAL			0x2a018
	#define PLL1_RX_PSC_CAL					0x0

#define PX210_PHY0_PLL0_XCVR_CTRL			0x183a8
#define PX210_PHY1_PLL0_XCVR_CTRL			0x903a8
	#define PLL0_XCVR_CTRL					0xf
#define PX210_PHY0_PLL1_XCVR_CTRL			0x1a3a8
	#define PLL1_XCVR_CTRL					0xf

#define PX210_PHY0_PLL0_RX_GCSM1_CTRL			0x28420
#define PX210_PHY1_PLL0_RX_GCSM1_CTRL			0xa0420
	#define PLL0_RX_GCSM1_CTRL				0x0
#define PX210_PHY0_PLL0_RX_GCSM2_CTRL			0x28440
#define PX210_PHY1_PLL0_RX_GCSM2_CTRL			0xa0440
	#define PLL0_RX_GCSM2_CTRL				0x0
#define PX210_PHY0_PLL0_RX_PERGCSM_CTRL			0x28460
#define PX210_PHY1_PLL0_RX_PERGCSM_CTRL			0xa0460
	#define PLL0_RX_PERGCSM_CTRL				0x0

#define PX210_PHY0_PLL1_RX_GCSM1_CTRL			0x2a420
	#define PLL1_RX_GCSM1_CTRL				0x0
#define PX210_PHY0_PLL1_RX_GCSM2_CTRL			0x2a440
	#define PLL1_RX_GCSM2_CTRL				0x0
#define PX210_PHY0_PLL1_RX_PERGCSM_CTRL			0x2a460
	#define PLL1_RX_PERGCSM_CTRL				0x0

/* swing and emphasis */
#define PX210_PHY0_PLL0_TX_DIAG_ACYA			0x1879c
#define PX210_PHY0_PLL1_TX_DIAG_ACYA			0x1a79c
#define PX210_PHY1_PLL0_TX_DIAG_ACYA			0x9079c
	#define LOCK						1
	#define UNLOCK						0

#define PX210_PHY0_PLL0_TX_TXCC_CTRL			0x18100
#define PX210_PHY0_PLL1_TX_TXCC_CTRL			0x1a100
#define PX210_PHY1_PLL0_TX_TXCC_CTRL			0x90100
	#define TX_TXCC_CTRL					0x8a4

#define PX210_PHY0_PLL0_TX_DRV				0x18318
#define PX210_PHY0_PLL1_TX_DRV				0x1a318
#define PX210_PHY1_PLL0_TX_DRV				0x90318
	#define TX_DRV						0x3

#define PX210_PHY0_PLL0_TX_MGNFS				0x18140
#define PX210_PHY0_PLL1_TX_MGNFS				0x1a140
#define PX210_PHY1_PLL0_TX_MGNFS				0x90140

#define PX210_PHY0_PLL0_TX_CPOST				0x18130
#define PX210_PHY0_PLL1_TX_CPOST				0x1a130
#define PX210_PHY0_PLL1_TX_CPOST1			0x1a13c
#define PX210_PHY1_PLL0_TX_CPOST				0x90130

/******************************phy register end********************************************/
#endif /* __PX210_REG_H__ */
