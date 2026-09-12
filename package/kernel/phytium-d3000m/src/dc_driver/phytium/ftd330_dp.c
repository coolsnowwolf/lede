// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium display port DRM driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic.h>
#include "ftd330_drv.h"
#include "phytium_dp_reg.h"
#include "phytium_dp.h"
#include "ftd330_dp.h"
#include "phytium_edp_pwm.h"

struct edp_drv_backlight_map edp_drv_bl_map = {
	.edp_bright_to_pwm =
		{1, 1, 1, 1, 1, 3, 3, 3, 3, 4,
		4, 4, 4, 5, 5, 6, 6, 6, 7, 7,
		7, 8, 8, 8, 8, 9, 9, 9, 10, 10,
		11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
		16, 16, 17, 17, 18, 18, 19, 19, 20, 21,
		22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		33, 34, 35, 36, 37, 39, 40, 41, 43, 44,
		46, 47, 49, 50, 52, 53, 55, 56, 58, 60,
		61, 63, 65, 66, 68, 70, 72, 74, 75, 77,
		79, 81, 83, 85, 87, 89, 91, 94, 96, 98, 99},

	.edp_pwm_to_bright =
		{1, 1, 6, 9, 11, 14, 16, 19, 23, 26, 29,
		31, 33, 35, 37, 39, 41, 43, 45, 47, 48,
		49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
		59, 60, 60, 61, 62, 63, 64, 64, 65, 66,
		67, 67, 68, 69, 70, 70, 71, 72, 72, 73,
		74, 74, 75, 76, 76, 77, 77, 78, 79, 79,
		80, 80, 81, 82, 82, 83, 83, 84, 84, 85,
		86, 86, 87, 87, 88, 88, 89, 89, 90, 90,
		91, 91, 92, 92, 93, 93, 94, 94, 95, 95,
		96, 96, 97, 97, 98, 98, 99, 99, 100, 100},
};

/* [reg][ling_rate 1.62->8.1] */
static int vco_val[12][4] = {
	{0x035d, 0x035d, 0x035d, 0x035d},// CP_PADJ
	{0x0e01, 0x0e01, 0x0e01, 0x0e01},// CP_IADJ
	{0x0d0a, 0x0d0a, 0x0d0a, 0x0d0a},// FILT_PADJ
	{0x0061, 0x006c, 0x006c, 0x0051},// INTDIV
	{0x3333, 0x0000, 0x0000, 0x0000},// FRACDIVL
	{0x0002, 0x0000, 0x0000, 0x0000},// FRACDIVH
	{0x0042, 0x0048, 0x0048, 0x0036},// HIGH_THR
	{0x0002, 0x0002, 0x0002, 0x0002},// PDIAG_CTRL
	{0x0c5e, 0x0c5e, 0x0c5e, 0x0c5e},// VCOCAL_PLLCNT_START
	{0x00c7, 0x00c7, 0x00c7, 0x00c7},// LOCK_PEFCNT
	{0x00c7, 0x00c7, 0x00c7, 0x00c7},// LOCK_PLLCNT_START
	{0x0003, 0x0003, 0x0003, 0x0003},// LOCK_PLLCNT_THR
};

struct dp_drv_phy_matrix dp_drv_phy_val = {
	.dp0_drv_mgnfs_val =// [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0018, 0x006, 0x0000, 0x0000},
			{0x000c, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0013, 0x006, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},

	.dp0_drv_cpost_val =// [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},

	.dp1_drv_mgnfs_val= // [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0018, 0x006, 0x0000, 0x0000},
			{0x000c, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0013, 0x006, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},

	.dp1_drv_cpost_val= // [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},

	.dp2_drv_mgnfs_val =// [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0026, 0x001f, 0x0012, 0x0000},
			{0x0013, 0x0013, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0018, 0x006, 0x0000, 0x0000},
			{0x000c, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0026, 0x0013, 0x005, 0x0000},
			{0x0013, 0x006, 0x0000, 0x0000},
			{0x0006, 0x0000, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},

	.dp2_drv_cpost_val = // [link_rate][swing][emphasis]
	{
		/* 1.62Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 2.7Gbps */
		{
			{0x0000, 0x0014, 0x0020, 0x002a},
			{0x0000, 0x0010, 0x001f, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 5.4Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		},

		/* 8.1Gbps */
		{
			{0x0000, 0x0014, 0x0022, 0x002e},
			{0x0000, 0x0013, 0x0020, 0x0000},
			{0x0000, 0x0013, 0x0000, 0x0000},
			{0x0000, 0x0000, 0x0000, 0x0000},
		}
	},
};

static int ftd330_dp_hw_set_phy_lane_and_rate(struct phytium_dp_device *phytium_dp,
							uint8_t link_lane_count,
							uint32_t link_rate)
{
	int i = 0, data, tmp, tmp1, index = 0, mask;
	int timeout = 500, ret = 0;

	if (phytium_dp->phy_mode == FTD330_PHY_MODE_DPX4 || phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE01) {
		/* set pma powerdown */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A3_POWERDOWN3 << i*A3_POWERDOWN3_SHIFT);
			mask |= (((1<<A3_POWERDOWN3_SHIFT) - 1) << (i*A3_POWERDOWN3_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);

		/* lane pll disable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PLL_EN);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, FTD330_PHY_PLL_EN, tmp);

		/* pma pll disable */
		mask = CONTROL_ENABLE;
		data = CONTROL_ENABLE;
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PMA_CONTROL, tmp);
		/* read pma pll disable state */
		mdelay(2);
		phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);

		/* config link rate */
		switch (link_rate) {
		case 810000:
			tmp = PLL_LINK_RATE_810000;
			tmp1 = HSCLK_LINK_RATE_810000;
			index = 3;
			break;
		case 540000:
			tmp = PLL_LINK_RATE_540000;
			tmp1 = HSCLK_LINK_RATE_540000;
			index = 2;
			break;
		case 270000:
			tmp = PLL_LINK_RATE_270000;
			tmp1 = HSCLK_LINK_RATE_270000;
			index = 1;
			break;
		case 162000:
			tmp = PLL_LINK_RATE_162000;
			tmp1 = HSCLK_LINK_RATE_162000;
			index = 0;
			break;
		default:
			DRM_ERROR("phytium dp rate(%d) not support\n", link_rate);
			tmp = PLL_LINK_RATE_162000;
			tmp1 = HSCLK_LINK_RATE_162000;
			index = 0;
			break;
		}
		/* config analog pll for link0 */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_CLK_SEL, tmp);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_HSCLK0_SEL, HSCLK_LINK_0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_HSCLK0_DIV, tmp1);

		/* config digital pll for link0 */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLLDRC0_CTRL, PLLDRC_LINK0);

		/* common for all rate */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_DSM_M0, PLL0_DSM_M0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_VCOCAL_START,
				   PLL0_VCOCAL_START);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_VCOCAL_CTRL,
				   PLL0_VCOCAL_CTRL);

		/* different for all rate . pll relative registers*/
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_CP_PADJ,
				   vco_val[0][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_CP_IADJ,
				   vco_val[1][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_CP_FILT_PADJ,
				   vco_val[2][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_INTDIV,
				   vco_val[3][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_FRACDIVL,
				   vco_val[4][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_FRACDIVH,
				   vco_val[5][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_HIGH_THR,
				   vco_val[6][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_PDIAG_CTRL,
				   vco_val[7][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_VCOCAL_PLLCNT_START,
				   vco_val[8][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_LOCK_PEFCNT,
				   vco_val[9][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_LOCK_PLLCNT_START,
				   vco_val[10][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_LOCK_PLLCNT_THR,
				   vco_val[11][index]);

		/*power relative registers*/
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_PSC_A0,
				   PLL0_TX_PSC_A0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_PSC_A1,
				   PLL0_TX_PSC_A1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_PSC_A2,
				   PLL0_TX_PSC_A2);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_PSC_A3,
				   PLL0_TX_PSC_A3);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PSC_A0,
				   PLL0_RX_PSC_A0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PSC_A1,
				   PLL0_RX_PSC_A1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PSC_A2,
				   PLL0_RX_PSC_A2);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PSC_A3,
				   PLL0_RX_PSC_A3);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PSC_CAL,
				   PLL0_RX_PSC_CAL);

		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_XCVR_CTRL,
				   PLL0_XCVR_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_GCSM1_CTRL,
				   PLL0_RX_GCSM1_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_GCSM2_CTRL,
				   PLL0_RX_GCSM2_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_RX_PERGCSM_CTRL,
				   PLL0_RX_PERGCSM_CTRL);

		phytium_phy_writel(phytium_dp, FTD330_TXPUCAL_TUNE, 0x3f);
		phytium_phy_writel(phytium_dp, FTD330_TXPDCAL_TUNE, 0x3f);

		/* pma pll enable */
		data = CONTROL_ENABLE;
		mask = CONTROL_ENABLE;
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PMA_CONTROL, tmp);

		/* lane pll enable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PLL_EN);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PLL_EN, tmp);

		/* set pma power active */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);

		mask = PLL0_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("dp(%d) phy pll lock failed\n", phytium_dp->port);
			ret = -1;
		}
		return ret;
	}else if (phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE23) {
		/* set pma powerdown */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A3_POWERDOWN3 << (i + 2)*A3_POWERDOWN3_SHIFT);
			mask |= (((1<<A3_POWERDOWN3_SHIFT) - 1) << ((i+2)*A3_POWERDOWN3_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);

		/* lane pll disable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			mask |= (((1<<PLL_EN_SHIFT) - 1) << ((i+2)*PLL_EN_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PLL_EN);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, FTD330_PHY_PLL_EN, tmp);

		/* pma pll disable */
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL);
		tmp &= ~BIT(1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PMA_CONTROL, tmp);
		/* read pma pll disable state */
		mdelay(2);
		phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);

		/* config link rate */
		switch (link_rate) {
		case 810000:
			tmp = PLL_LINK_RATE_810000;
			tmp1 = HSCLK_LINK_RATE_810000;
			index = 3;
			break;
		case 540000:
			tmp = PLL_LINK_RATE_540000;
			tmp1 = HSCLK_LINK_RATE_540000;
			index = 2;
			break;
		case 270000:
			tmp = PLL_LINK_RATE_270000;
			tmp1 = HSCLK_LINK_RATE_270000;
			index = 1;
			break;
		case 162000:
			tmp = PLL_LINK_RATE_162000;
			tmp1 = HSCLK_LINK_RATE_162000;
			index = 0;
			break;
		default:
			DRM_ERROR("phytium dp rate(%d) not support\n", link_rate);
			tmp = PLL_LINK_RATE_162000;
			tmp1 = HSCLK_LINK_RATE_162000;
			index = 0;
			break;
		}
		/* config analog pll for link0 */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_CLK_SEL, tmp);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_HSCLK1_SEL, HSCLK_LINK_1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_HSCLK1_DIV, tmp1);

		/* config digital pll for link0 */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLLDRC1_CTRL, PLLDRC_LINK1);

		/* common for all rate */
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_DSM_M0, PLL1_DSM_M0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_VCOCAL_START,
				   PLL1_VCOCAL_START);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_VCOCAL_CTRL,
				   PLL1_VCOCAL_CTRL);

		/* different for all rate . pll relative registers*/
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_CP_PADJ,
				   vco_val[0][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_CP_IADJ,
				   vco_val[1][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_CP_FILT_PADJ,
				   vco_val[2][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_INTDIV,
				   vco_val[3][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_FRACDIVL,
				   vco_val[4][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_FRACDIVH,
				   vco_val[5][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_HIGH_THR,
				   vco_val[6][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_PDIAG_CTRL,
				   vco_val[7][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_VCOCAL_PLLCNT_START,
				   vco_val[8][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_LOCK_PEFCNT,
				   vco_val[9][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_LOCK_PLLCNT_START,
				   vco_val[10][index]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_LOCK_PLLCNT_THR,
				   vco_val[11][index]);

		/*power relative registers*/
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_PSC_A0,
				   PLL1_TX_PSC_A0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_PSC_A1,
				   PLL1_TX_PSC_A1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_PSC_A2,
				   PLL1_TX_PSC_A2);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_PSC_A3,
				   PLL1_TX_PSC_A3);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PSC_A0,
				   PLL1_RX_PSC_A0);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PSC_A1,
				   PLL1_RX_PSC_A1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PSC_A2,
				   PLL1_RX_PSC_A2);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PSC_A3,
				   PLL1_RX_PSC_A3);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PSC_CAL,
				   PLL1_RX_PSC_CAL);

		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_XCVR_CTRL,
				   PLL1_XCVR_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_GCSM1_CTRL,
				   PLL1_RX_GCSM1_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_GCSM2_CTRL,
				   PLL1_RX_GCSM2_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_RX_PERGCSM_CTRL,
				   PLL1_RX_PERGCSM_CTRL);

		phytium_phy_writel(phytium_dp, FTD330_TXPUCAL_TUNE, 0x3f);
		phytium_phy_writel(phytium_dp, FTD330_TXPDCAL_TUNE, 0x3f);
		/* pma pll enable */
		data = PLL1_CONTROL_ENABLE;
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL);
		tmp |= data;
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PMA_CONTROL, tmp);

		/* lane pll enable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << (i+2)*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << ((i+2)*PLL_EN_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PLL_EN);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PLL_EN, tmp);

		/* set pma power active */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << (i+2)*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << ((i+2)*A0_ACTIVE_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);

		mask = PLL1_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("dp(%d) phy pll lock failed\n", phytium_dp->port);
			ret = -1;
		}
		return ret;
	} else {
		pr_err("invalid dp phy_mode\n");
		return -1;
	}
}


static void
ftd330_dp_hw_set_phy_lane_setting(struct phytium_dp_device *phytium_dp,
								uint32_t link_rate,
								uint8_t train_set)
{
	int voltage_swing = 0;
	int pre_emphasis = 0, link_rate_index = 0;

	switch (train_set & DP_TRAIN_VOLTAGE_SWING_MASK) {
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_0:
	default:
		voltage_swing = 0;
		break;
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_1:
		voltage_swing = 1;
		break;
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_2:
		voltage_swing = 2;
		break;
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_3:
		voltage_swing = 3;
		break;
	}
	switch (train_set & DP_TRAIN_PRE_EMPHASIS_MASK) {
	case DP_TRAIN_PRE_EMPH_LEVEL_0:
	default:
		pre_emphasis = 0;
		break;
	case DP_TRAIN_PRE_EMPH_LEVEL_1:
		pre_emphasis = 1;
		break;
	case DP_TRAIN_PRE_EMPH_LEVEL_2:
		pre_emphasis = 2;
		break;
	case DP_TRAIN_PRE_EMPH_LEVEL_3:
		pre_emphasis = 3;
		break;
	}

	switch (link_rate) {
	case 810000:
		link_rate_index = 3;
		break;
	case 540000:
		link_rate_index = 2;
		break;
	case 270000:
		link_rate_index = 1;
		break;
	case 162000:
		link_rate_index = 0;
		break;
	default:
		DRM_ERROR("phytium dp rate(%d) not support\n", link_rate);
		link_rate_index = 2;
		break;
	}

	if (phytium_dp->phy_mode == FTD330_PHY_MODE_DPX4 || phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE01) {
		
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_DIAG_ACYA, LOCK);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_TXCC_CTRL, TX_TXCC_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_DRV, TX_DRV);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_MGNFS,
				phytium_dp->phy_matrix.dp_mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_CPOST,
				phytium_dp->phy_matrix.dp_cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL0_TX_DIAG_ACYA, UNLOCK);
		
	} else if (phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE23) {
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_DIAG_ACYA, LOCK);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_TXCC_CTRL, TX_TXCC_CTRL);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_DRV, TX_DRV);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_MGNFS,
				phytium_dp->phy_matrix.dp_mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_CPOST,
				phytium_dp->phy_matrix.dp_cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL1_TX_DIAG_ACYA, UNLOCK);

	} else {
		   pr_err("invalid dp phy_mode\n");
	}
#if PHY_MATRIX_DEBUG
	pr_info("port %d, link_rate_idx is %d, swing idx is %d, emphasis is %d\n",
			phytium_dp->port, link_rate_index, voltage_swing, pre_emphasis);
	pr_info("mgnfs_val is %x, cpost_val is %x\n",
		phytium_dp->phy_matrix.dp_mgnfs_val[link_rate_index][voltage_swing][pre_emphasis],
		phytium_dp->phy_matrix.dp_cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
#endif
	return;

}

static int ftd330_dp_hw_init_phy(struct phytium_dp_device *phytium_dp)
{
	int i = 0, data, tmp, mask;
	int timeout = 500, ret = 0;


	if (!is_dp_powered(phytium_dp)) {
		return ret;
	}

	if (phytium_dp->phy_mode == FTD330_PHY_MODE_DPX4) {
		phytium_phy_writel(phytium_dp, FTD330_PHY_PIPE_SEL, SEL_32BIT);
		phytium_phy_writel(phytium_dp, FTD330_PHY_LANE_BUFFER, LANE_BUFFER_INIT);

		/* config lane master or slave */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (LANE_SLAVE << i*LANE_MASTER_SHIFT);
			mask |= (((1<<LANE_MASTER_SHIFT) - 1) << (i*LANE_MASTER_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_LINK_CFG);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_CFG, tmp);

		/* config lane to dp mode */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (LANE_BIT << i*LANE_BIT_SHIFT);
			mask |= (((1<<LANE_BIT_SHIFT) - 1) << (i*LANE_BIT_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_MODE);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_MODE, tmp);

		/* pll clock enable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PLL_EN);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PLL_EN, tmp);

		/* config input 20 bit */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (BIT_20 << i*BIT_20_SHIFT);
			mask |= (((1<<BIT_20_SHIFT) - 1) << (i*BIT_20_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA_WIDTH);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA_WIDTH, tmp);

		/* config lane active power state */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
		}

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);

		phytium_phy_writel(phytium_dp, FTD330_PHY_APB_RESET, RESET_DEASSERT);
		phytium_phy_writel(phytium_dp, FTD330_PHY_PIPE_RESET, RESET_DEASSERT);
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, RESET);

		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL_CFG, SINGLE_LINK);
		phytium_phy_writel(phytium_dp, FTD330_REG_AUX_CFG2, 0x1500);
		/* link reset */
		mask = (LINK_RESET_MASK << (0*LINTK_RESET_SHIFT)) |
			(LINK_RESET_MASK << (1*LINTK_RESET_SHIFT)) |
			(LINK_RESET_MASK << (2*LINTK_RESET_SHIFT)) |
			(LINK_RESET_MASK << (3*LINTK_RESET_SHIFT));
		data = (LINK_RESET << (0*LINTK_RESET_SHIFT)) |
			(LINK_RESET << (1*LINTK_RESET_SHIFT)) |
			(LINK_RESET << (2*LINTK_RESET_SHIFT)) |
			(LINK_RESET << (3*LINTK_RESET_SHIFT));
		tmp = (data & mask);
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, tmp);
			/* pipe reset */
		mask = PLL0_LOCK_DONE;

		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("reset dp(%d) phy failed\n", phytium_dp->port);
			ret = -1;
		}

		mdelay(10);
	} else if (phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE23) {
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_LINK_RESET);
		tmp &= ~(BIT(2) | BIT(3));
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, tmp);

		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PLL_CFG);
		tmp |= BIT(1);
		phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL_CFG, tmp);
		phytium_phy_writel(phytium_dp, FTD330_REG_AUX_CFG2, 0x1500);

		tmp = 0;
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_LINK_RESET);
		tmp |= BIT(2) | BIT(3);
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, tmp);
		/* pipe reset */
		mask = PLL1_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("reset dp(%d) phy failed\n", phytium_dp->port);
			ret = -1;
		}
		mdelay(10);
	} else if (phytium_dp->phy_mode == FTD330_PHY_MODE_DP_LANE01) {
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_LINK_RESET);
		tmp &= ~(BIT(0) | BIT(1));
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, tmp);

                tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PLL_CFG);
                tmp |= BIT(1);
                phytium_phy_writel(phytium_dp, FTD330_PHY0_PLL_CFG, tmp);
		phytium_phy_writel(phytium_dp, FTD330_REG_AUX_CFG2, 0x1500);

		tmp = 0;
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_LINK_RESET);
		tmp |= BIT(0) | BIT(1);
		phytium_phy_writel(phytium_dp, FTD330_PHY_LINK_RESET, tmp);
		/* pipe reset */
		mask = PLL0_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("reset dp(%d) phy failed\n", phytium_dp->port);
			ret = -1;
		}

		mdelay(10);
	} else {
		pr_err("invalid dp phy_mode\n");
	}

	return ret;
}

static bool
ftd330_dp_phy_power_is_A0_state(struct phytium_dp_device *phytium_dp)
{
	int i = 0, data, tmp, mask;

	data = 0;
	mask = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
		data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
		mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
	}
	tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
	tmp = tmp & mask;

	if (tmp == data)
		return true;
	return false;
}

static void
ftd330_dp_hw_config_phy_power(struct phytium_dp_device *phytium_dp, bool enable)
{
	int i = 0, data, tmp, mask;
	int timeout = 500;

	if (enable) {
		if (ftd330_dp_phy_power_is_A0_state(phytium_dp))
			return;

		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);
		mask = PLL0_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, FTD330_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("dp(%d) active phy pll lock failed\n", phytium_dp->port);
		}
		FTD330_LOG("power on dp-%d phy\n", phytium_dp->port);
	} else {
		if (!ftd330_dp_phy_power_is_A0_state(phytium_dp))
			return;

		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A3_POWERDOWN3 << i*A3_POWERDOWN3_SHIFT);
			mask |= (((1<<A3_POWERDOWN3_SHIFT) - 1) << (i*A3_POWERDOWN3_SHIFT));
		}
		tmp = phytium_phy_readl(phytium_dp, FTD330_PHY_PMA0_POWER);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, FTD330_PHY_PMA0_POWER, tmp);
		FTD330_LOG("power down dp-%d phy\n", phytium_dp->port);
	}
}

void ftd330_dp_poweron_panel(struct ftd330_drm_private *priv, int port)
{
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;
	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	if (reg & EDP_BL_PWR_ON) {
		return;
	}

	reg |= EDP_BL_PWR_ON;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
#if BL_DEBUG
	pr_info("%s set EDP_BACKLIGHT_CONTROL: 0x%08x\n", __func__, reg);
	pr_info("poweron panel\n");
#endif
}

bool ftd330_dp_hw_panel_is_poweron(struct phytium_dp_device *phytium_dp)
{
       struct drm_device *dev = phytium_dp->dev;
       struct ftd330_drm_private *priv = dev->dev_private;
       int port = phytium_dp->port;
       uint32_t group_offset = priv->dp_reg_base[port];
       uint32_t reg;

       reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);

       if (reg & EDP_BL_PWR_ON)
       	       return true;

       return false;
}


static void ftd330_dp_hw_poweron_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	if (reg & EDP_BL_PWR_ON) {
		return;
	}
	reg |= EDP_BL_PWR_ON;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
#if BL_DEBUG
	pr_info("%s set EDP_BACKLIGHT_CONTROL: 0x%08x\n", __func__, reg);
	pr_info("poweron panel\n");
#endif
}

static void ftd330_dp_hw_poweroff_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	reg &= ~EDP_BL_PWR_OFF_MASK;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
#if BL_DEBUG
	pr_info("poweroff panel\n");
#endif
}

static void ftd330_dp_hw_enable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_panel *panel = &phytium_dp->panel;
	struct phytium_bl_pwm_chip *pwm_chip = &panel->pwm_chip;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	pwm_phytium_enable(priv, pwm_chip);
	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	reg |= EDP_BL_ENABLE;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
#if BL_DEBUG
	pr_info("enable backlight\n");
#endif
}

static void ftd330_dp_hw_disable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_panel *panel = &phytium_dp->panel;
	struct phytium_bl_pwm_chip *pwm_chip = &panel->pwm_chip;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	FTD330_LOG_TRACE;
	pwm_phytium_disable(priv, pwm_chip);
	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	reg &= ~EDP_BL_DISABLE_MASK;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
#if BL_DEBUG
	pr_info("disable backlight\n");
#endif
}

static uint32_t ftd330_dp_hw_get_backlight(struct phytium_dp_device *phytium_dp)
{
	struct phytium_panel *panel = &phytium_dp->panel;
	uint32_t pwm_hw_level;
	struct phytium_bl_pwm_chip *pwm_chip = &panel->pwm_chip;
	#if MAP_BL
	uint32_t map_level;
	#endif

	pwm_hw_level = pwm_get_relative_duty_cycle(&pwm_chip->state_pm, 100);
#if BL_DEBUG
	pr_info("get backlight pwm_hw_level: %d\n", pwm_hw_level);
#endif
#if MAP_BL
	map_level = phytium_dp->edp_bl_map.edp_pwm_to_bright[pwm_hw_level];
	return map_level;
#endif
	return pwm_hw_level;
}

static int ftd330_dp_hw_set_backlight(struct phytium_dp_device *phytium_dp, uint32_t level)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_panel *panel = &phytium_dp->panel;
	struct phytium_bl_pwm_chip *pwm_chip = &panel->pwm_chip;
#if MAP_BL
	uint32_t pwm_hw_level;
#endif
	int ret = 0;

#if MAP_BL
	pwm_hw_level = phytium_dp->edp_bl_map.edp_bright_to_pwm[level];
	if (pwm_hw_level > panel->max || pwm_hw_level < panel->min) {
		ret = -EINVAL;
		goto out;
	}

	pwm_set_relative_duty_cycle(&pwm_chip->state_pm, pwm_hw_level, 100);
#if BL_DEBUG
	pr_info("Set level: %d, pwm hw level: %d\n", level, pwm_hw_level);
#if KERNEL_VERSION(5, 4, 18) >= LINUX_VERSION_CODE
	pr_info("Set duty: %u ns\n", pwm_chip->state_pm.duty_cycle);
#else
	pr_info("Set duty: %lld ns\n", pwm_chip->state_pm.duty_cycle);
#endif
#endif

#else
	if (level > panel->max || level < panel->min) {
		ret = -EINVAL;
		goto out;
	}

	pwm_set_relative_duty_cycle(&pwm_chip->state_pm, level, 100);
	pwm_chip->state_pm.enabled = true;
#if BL_DEBUG
#if KERNEL_VERSION(5, 4, 18) >= LINUX_VERSION_CODE
	pr_info("Set duty: %u ns\n", pwm_chip->state_pm.duty_cycle);
#else
	pr_info("Set duty: %lld ns\n", pwm_chip->state_pm.duty_cycle);
#endif
#endif
#endif
	pwm_phytium_set_duty(priv, pwm_chip, pwm_chip->state_pm.duty_cycle);

out:
	return ret;
}

static bool ftd330_dp_hw_spread_is_enable(struct phytium_dp_device *phytium_dp)
{
	return false;
}

static int ftd330_dp_hw_reset(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	if (phytium_dp->is_edp) {
	phytium_writel_reg(priv, DP_I2S_RESET, group_offset, ftd330_DP_CONTROLLER_RESET);
	} else {	
		phytium_writel_reg(priv, 0, group_offset, ftd330_DP_CONTROLLER_RESET);
	}
	udelay(500);
	phytium_writel_reg(priv, DP_RESET_RELEASE, group_offset, ftd330_DP_CONTROLLER_RESET);
	udelay(500);
	phytium_writel_reg(priv, AUX_CLK_DIVIDER_100, group_offset, PHYTIUM_DP_AUX_CLK_DIVIDER);

	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_DISABLE,
			group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	phytium_writel_reg(priv, (~VIRTUAL_SOURCE_0_ENABLE)&VIRTUAL_SOURCE_0_ENABLE_MASK,
			group_offset, PHYTIUM_INPUT_SOURCE_ENABLE);
	phytium_writel_reg(priv, SST_MST_SOURCE_0_DISABLE,
			group_offset, PHYTIUM_DP_VIDEO_STREAM_ENABLE);
	phytium_writel_reg(priv, VIRTUAL_SOURCE_0_ENABLE,
			group_offset, PHYTIUM_INPUT_SOURCE_ENABLE);
	phytium_writel_reg(priv, TRANSMITTER_OUTPUT_ENABLE,
			group_offset, PHYTIUM_DP_TRANSMITTER_OUTPUT_ENABLE);
	phytium_writel_reg(priv, SCRAMBLING_ENABLE, group_offset,
			PHYTIUM_DP_SCRAMBLING_DISABLE);
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
	phytium_writel_reg(priv, 0x3f, group_offset, PHYTIUM_DP_INTERRUPT_MASK);
#endif
	return 0;
}


static struct phytium_dp_func ftd330_dp_funcs = {
	.dp_hw_reset = ftd330_dp_hw_reset,
	.dp_hw_spread_is_enable = ftd330_dp_hw_spread_is_enable,
	.dp_hw_set_backlight = ftd330_dp_hw_set_backlight,
	.dp_hw_get_backlight = ftd330_dp_hw_get_backlight,
	.dp_hw_disable_backlight = ftd330_dp_hw_disable_backlight,
	.dp_hw_enable_backlight = ftd330_dp_hw_enable_backlight,
	.dp_hw_poweroff_panel = ftd330_dp_hw_poweroff_panel,
	.dp_hw_poweron_panel = ftd330_dp_hw_poweron_panel,
	.dp_hw_init_phy = ftd330_dp_hw_init_phy,
	.dp_hw_set_phy_lane_setting = ftd330_dp_hw_set_phy_lane_setting,
	.dp_hw_set_phy_lane_and_rate = ftd330_dp_hw_set_phy_lane_and_rate,
	.dp_hw_config_phy_power = ftd330_dp_hw_config_phy_power,
};

void ftd330_dp_func_register(struct phytium_dp_device *phytium_dp)
{
	phytium_dp->funcs = &ftd330_dp_funcs;
}
