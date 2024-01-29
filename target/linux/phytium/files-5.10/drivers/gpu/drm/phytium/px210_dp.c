// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include "phytium_display_drv.h"
#include "px210_reg.h"
#include "phytium_dp.h"
#include "px210_dp.h"

static uint8_t px210_dp_source_lane_count[3] = {4, 4, 1};

/* [reg][ling_rate 1.62->8.1] */
static int vco_val[12][4] = {
	{0x0509, 0x0509, 0x0509, 0x0509}, // CP_PADJ
	{0x0f00, 0x0f00, 0x0f00, 0x0f00}, // CP_IADJ
	{0x0F08, 0x0F08, 0x0F08, 0x0F08}, // FILT_PADJ
	{0x0061, 0x006C, 0x006C, 0x0051}, // INTDIV
	{0x3333, 0x0000, 0x0000, 0x0000}, // FRACDIVL
	{0x0000, 0x0000, 0x0000, 0x0000}, // FRACDIVH
	{0x0042, 0x0048, 0x0048, 0x0036}, // HIGH_THR
	{0x0002, 0x0002, 0x0002, 0x0002}, // PDIAG_CTRL
	{0x0c5e, 0x0c5e, 0x0c5e, 0x0c5e}, // VCOCAL_PLLCNT_START
	{0x00c7, 0x00c7, 0x00c7, 0x00c7}, // LOCK_PEFCNT
	{0x00c7, 0x00c7, 0x00c7, 0x00c7}, // LOCK_PLLCNT_START
	{0x0005, 0x0005, 0x0005, 0x0005}, // LOCK_PLLCNT_THR
};

static int mgnfs_val[4][4][4] = // [link_rate][swing][emphasis]
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
	},
};

static int cpost_val[4][4][4] = // [link_rate][swing][emphasis]
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
	},
};

static int px210_dp_hw_set_phy_lane_and_rate(struct phytium_dp_device *phytium_dp,
							uint8_t link_lane_count,
							uint32_t link_rate)
{
	int port = phytium_dp->port%3;
	int i = 0, data, tmp, tmp1, index = 0, mask;
	int timeout = 500, ret = 0;

	if (port == 0 || port == 1) {
		/* set pma powerdown */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A3_POWERDOWN3 << i*A3_POWERDOWN3_SHIFT);
			mask |= (((1<<A3_POWERDOWN3_SHIFT) - 1) << (i*A3_POWERDOWN3_SHIFT));
		}
		if (port == 0) {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA0_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA0_POWER, tmp);
		} else {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA1_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA1_POWER, tmp);
		}

		/* lane pll disable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}
		mask = (mask << (port*PLL_EN_SHIFT*4));
		data = (data << (port*PLL_EN_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PLL_EN);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL_EN, tmp);

		/* pma pll disable */
		mask = (CONTROL_ENABLE << (port*CONTROL_ENABLE_SHIFT));
		data = (CONTROL_ENABLE << (port*CONTROL_ENABLE_SHIFT));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_CONTROL);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, PX210_PHY0_PMA_CONTROL, tmp);

		/* read pma pll disable state */
		mdelay(2);
		phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_CONTROL2);

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
		if (port == 0) {
			/* config analog pll for link0 */
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_CLK_SEL, tmp);
			phytium_phy_writel(phytium_dp, PX210_PHY0_HSCLK0_SEL, HSCLK_LINK_0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_HSCLK0_DIV, tmp1);

			/* config digital pll for link0 */
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLLDRC0_CTRL, PLLDRC_LINK0);

			/* common for all rate */
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_DSM_M0, PLL0_DSM_M0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_VCOCAL_START,
					   PLL0_VCOCAL_START);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_VCOCAL_CTRL,
					   PLL0_VCOCAL_CTRL);

			/* different for all rate */
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_CP_PADJ,
					   vco_val[0][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_CP_IADJ,
					   vco_val[1][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_CP_FILT_PADJ,
					   vco_val[2][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_INTDIV,
					   vco_val[3][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_FRACDIVL,
					   vco_val[4][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_FRACDIVH,
					   vco_val[5][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_HIGH_THR,
					   vco_val[6][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_PDIAG_CTRL,
					   vco_val[7][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_VCOCAL_PLLCNT_START,
					   vco_val[8][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_LOCK_PEFCNT,
					   vco_val[9][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_LOCK_PLLCNT_START,
					   vco_val[10][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_LOCK_PLLCNT_THR,
					   vco_val[11][index]);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_PSC_A0,
					   PLL0_TX_PSC_A0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_PSC_A2,
					   PLL0_TX_PSC_A2);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_PSC_A3,
					   PLL0_TX_PSC_A3);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_PSC_A0,
					   PLL0_RX_PSC_A0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_PSC_A2,
					   PLL0_RX_PSC_A2);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_PSC_A3,
					   PLL0_RX_PSC_A3);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_PSC_CAL,
					   PLL0_RX_PSC_CAL);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_XCVR_CTRL,
					   PLL0_XCVR_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_GCSM1_CTRL,
					   PLL0_RX_GCSM1_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_GCSM2_CTRL,
					   PLL0_RX_GCSM2_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_RX_PERGCSM_CTRL,
					   PLL0_RX_PERGCSM_CTRL);
		} else if (port == 1) {
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_CLK_SEL, tmp);
			phytium_phy_writel(phytium_dp, PX210_PHY0_HSCLK1_SEL, HSCLK_LINK_1);
			phytium_phy_writel(phytium_dp, PX210_PHY0_HSCLK1_DIV, tmp1);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLLDRC1_CTRL, PLLDRC_LINK1);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_DSM_M0, PLL1_DSM_M0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_VCOCAL_START,
					   PLL1_VCOCAL_START);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_VCOCAL_CTRL,
					   PLL1_VCOCAL_CTRL);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_CP_PADJ,
					   vco_val[0][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_CP_IADJ,
					   vco_val[1][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_CP_FILT_PADJ,
					   vco_val[2][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_INTDIV,
					   vco_val[3][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_FRACDIVL,
					   vco_val[4][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_FRACDIVH,
					   vco_val[5][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_HIGH_THR,
					   vco_val[6][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_PDIAG_CTRL,
					   vco_val[7][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_VCOCAL_PLLCNT_START,
					   vco_val[8][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_LOCK_PEFCNT,
					   vco_val[9][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_LOCK_PLLCNT_START,
					   vco_val[10][index]);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_LOCK_PLLCNT_THR,
					   vco_val[11][index]);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_PSC_A0,
					   PLL1_TX_PSC_A0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_PSC_A2,
					   PLL1_TX_PSC_A2);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_PSC_A3,
					   PLL1_TX_PSC_A3);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_PSC_A0,
					   PLL1_RX_PSC_A0);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_PSC_A2,
					   PLL1_RX_PSC_A2);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_PSC_A3,
					   PLL1_RX_PSC_A3);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_PSC_CAL,
					   PLL1_RX_PSC_CAL);

			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_XCVR_CTRL,
					   PLL1_XCVR_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_GCSM1_CTRL,
					   PLL1_RX_GCSM1_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_GCSM2_CTRL,
					   PLL1_RX_GCSM2_CTRL);
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_RX_PERGCSM_CTRL,
					   PLL1_RX_PERGCSM_CTRL);
		}

		/* pma pll enable */
		data = 0;
		mask = 0;
		mask = (CONTROL_ENABLE << (port*CONTROL_ENABLE_SHIFT));
		data = (CONTROL_ENABLE << (port*CONTROL_ENABLE_SHIFT));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_CONTROL);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_PMA_CONTROL, tmp);

		/* lane pll enable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}
		mask = (mask << (port*PLL_EN_SHIFT*4));
		data = (data << (port*PLL_EN_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PLL_EN);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL_EN, tmp);

		/* set pma power active */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
		}
		if (port == 0) {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA0_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA0_POWER, tmp);
			mask = PLL0_LOCK_DONE;
		} else {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA1_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA1_POWER, tmp);
			mask = PLL1_LOCK_DONE;
		}

		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("dp(%d) phy pll lock failed\n", port);
			ret = -1;
		}
		udelay(1);
	} else {
		/* set pma powerdown */
		mask = PHY1_A3_POWERDOWN3_MASK << PHY1_A3_POWERDOWN3_SHIFT;
		data = PHY1_A3_POWERDOWN3 << PHY1_A3_POWERDOWN3_SHIFT;
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_MISC);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_MISC, tmp);

		/* lane pll disable */
		mask = PHY1_PLL_EN_MASK << PHY1_PLL_EN_SHIFT;
		data = PHY1_PLL_EN << PHY1_PLL_EN_SHIFT;
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_MISC);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_MISC, tmp);

		/* pma pll disable */
		mask = (CONTROL_ENABLE << 0*CONTROL_ENABLE_SHIFT); // link config
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_CONTROL);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_CONTROL, tmp);

		/* read pma pll disable state */
		mdelay(2);
		phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_CONTROL2);

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
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL_CLK_SEL, tmp);
		phytium_phy_writel(phytium_dp, PX210_PHY1_HSCLK_SEL, HSCLK_LINK_0);
		phytium_phy_writel(phytium_dp, PX210_PHY1_HSCLK_DIV, tmp1);

		/* config digital pll for link0 */
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLLDRC_CTRL, PLLDRC_LINK0);

		/* common for all rate */
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_DSM_M0, PLL0_DSM_M0);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_VCOCAL_START, PLL0_VCOCAL_START);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_VCOCAL_CTRL, PLL0_VCOCAL_CTRL);

		/* different for all rate */
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_CP_PADJ, vco_val[0][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_CP_IADJ, vco_val[1][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_CP_FILT_PADJ, vco_val[2][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_INTDIV, vco_val[3][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_FRACDIVL, vco_val[4][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_FRACDIVH, vco_val[5][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_HIGH_THR, vco_val[6][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_PDIAG_CTRL, vco_val[7][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_VCOCAL_PLLCNT_START,
				   vco_val[8][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_LOCK_PEFCNT, vco_val[9][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_LOCK_PLLCNT_START,
				   vco_val[10][index]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_LOCK_PLLCNT_THR,
				   vco_val[11][index]);

		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_PSC_A0, PLL0_TX_PSC_A0);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_PSC_A2, PLL0_TX_PSC_A2);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_PSC_A3, PLL0_TX_PSC_A3);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_PSC_A0, PLL0_RX_PSC_A0);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_PSC_A2, PLL0_RX_PSC_A2);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_PSC_A3, PLL0_RX_PSC_A3);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_PSC_CAL, PLL0_RX_PSC_CAL);

		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_XCVR_CTRL, PLL0_XCVR_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_GCSM1_CTRL,
				   PLL0_RX_GCSM1_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_GCSM2_CTRL,
				   PLL0_RX_GCSM2_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_RX_PERGCSM_CTRL,
				   PLL0_RX_PERGCSM_CTRL);

		/* pma pll enable */
		data = 0;
		mask = 0;
		mask = (CONTROL_ENABLE << (0*CONTROL_ENABLE_SHIFT));
		data = (CONTROL_ENABLE << (0*CONTROL_ENABLE_SHIFT));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_CONTROL);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_CONTROL, tmp);

		/* lane pll enable */
		mask = PHY1_PLL_EN_MASK << PHY1_PLL_EN_SHIFT;
		data = PHY1_PLL_EN << PHY1_PLL_EN_SHIFT;
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_MISC);
		tmp = ((tmp & (~mask)) | data);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_MISC, tmp);

		/* set pma power active */
		mask = PHY1_A0_ACTIVE_MASK << PHY1_A0_ACTIVE_SHIFT;
		data = PHY1_A0_ACTIVE << PHY1_A0_ACTIVE_SHIFT;
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_MISC);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_MISC, tmp);

		mask = PLL0_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("dp(%d) phy pll lock failed\n", port);
			ret = -1;
		}
		udelay(1);
	}

	return ret;
}

static void px210_dp_hw_set_phy_lane_setting(struct phytium_dp_device *phytium_dp,
						      uint32_t link_rate,
						      uint8_t train_set)
{
	int port = phytium_dp->port%3;
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

	if (port == 0) {
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_DIAG_ACYA, LOCK);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_TXCC_CTRL, TX_TXCC_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_DRV, TX_DRV);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_MGNFS,
				mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_CPOST,
				cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL0_TX_DIAG_ACYA, UNLOCK);

	} else if (port == 1) {
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_DIAG_ACYA, LOCK);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_TXCC_CTRL, TX_TXCC_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_DRV, TX_DRV);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_MGNFS,
				mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_CPOST,
				cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_CPOST1,
				cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL1_TX_DIAG_ACYA, UNLOCK);
	} else {
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_DIAG_ACYA, LOCK);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_TXCC_CTRL, TX_TXCC_CTRL);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_DRV, TX_DRV);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_MGNFS,
				mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_CPOST,
				cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL0_TX_DIAG_ACYA, UNLOCK);
	}
}

static int px210_dp_hw_init_phy(struct phytium_dp_device *phytium_dp)
{
	int port = phytium_dp->port;
	int i = 0, data, tmp, mask;
	int timeout = 500, ret = 0;

	if (port == 0 || port == 1) {
		phytium_phy_writel(phytium_dp, PX210_PHY0_APB_RESET, APB_RESET);

		phytium_phy_writel(phytium_dp, PX210_PHY0_PIPE_RESET, RESET);

		/* config lane to dp mode */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (LANE_BIT << i*LANE_BIT_SHIFT);
			mask |= (((1<<LANE_BIT_SHIFT) - 1) << (i*LANE_BIT_SHIFT));
		}
		mask = (mask << (port*LANE_BIT_SHIFT*4));
		data = (data << (port*LANE_BIT_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_MODE);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_MODE, tmp);

		/* config lane master or slave */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (LANE_MASTER << i*LANE_MASTER_SHIFT);
			mask |= (((1<<LANE_MASTER_SHIFT) - 1) << (i*LANE_MASTER_SHIFT));
		}
		mask = (mask << (port*LANE_MASTER_SHIFT*4));
		data = (data << (port*LANE_MASTER_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_LINK_CFG);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_LINK_CFG, tmp);

		/* pll clock enable */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (PLL_EN << i*PLL_EN_SHIFT);
			mask |= (((1<<PLL_EN_SHIFT) - 1) << (i*PLL_EN_SHIFT));
		}
		mask = (mask << (port*PLL_EN_SHIFT*4));
		data = (data << (port*PLL_EN_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PLL_EN);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_PLL_EN, tmp);

		/* config input 20 bit */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (BIT_20 << i*BIT_20_SHIFT);
			mask |= (((1<<BIT_20_SHIFT) - 1) << (i*BIT_20_SHIFT));
		}
		mask = (mask << (port*BIT_20_SHIFT*4));
		data = (data << (port*BIT_20_SHIFT*4));
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_WIDTH);
		tmp = (tmp & (~mask)) | data;
		phytium_phy_writel(phytium_dp, PX210_PHY0_PMA_WIDTH, tmp);

		/* config lane active power state */
		data = 0;
		mask = 0;
		for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
			data |= (A0_ACTIVE << i*A0_ACTIVE_SHIFT);
			mask |= (((1<<A0_ACTIVE_SHIFT) - 1) << (i*A0_ACTIVE_SHIFT));
		}
		if (port == 0) {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA0_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA0_POWER, tmp);
		} else {
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA1_POWER);
			tmp = (tmp & (~mask)) | data;
			phytium_phy_writel(phytium_dp, PX210_PHY0_PMA1_POWER, tmp);
		}

		/* link reset */
		mask = (LINK_RESET_MASK << (0*LINTK_RESET_SHIFT)) |
			(LINK_RESET_MASK << (1*LINTK_RESET_SHIFT));
		data = (LINK_RESET << (0*LINTK_RESET_SHIFT)) |
			(LINK_RESET << (1*LINTK_RESET_SHIFT));
		tmp = (data & mask);
		phytium_phy_writel(phytium_dp, PX210_PHY0_LINK_RESET, tmp);

		/* config double link */
		if (port == 0)
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL_CFG, SINGLE_LINK);
		else if (port == 1)
			phytium_phy_writel(phytium_dp, PX210_PHY0_PLL_CFG, DOUBLE_LINK);

		/* pipe reset */
		phytium_phy_writel(phytium_dp, PX210_PHY0_PIPE_RESET, RESET_DEASSERT);

		if (port ==  0)
			mask = PLL0_LOCK_DONE;
		else
			mask = PLL1_LOCK_DONE;

		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY0_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("reset dp(%d) phy failed\n", port);
			ret = -1;
		}
		udelay(1);
	} else {
		/* APB reset */
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_APB_RESET);
		phytium_phy_writel(phytium_dp, PX210_PHY1_APB_RESET, tmp & (~PHY1_APB_RESET));
		phytium_phy_writel(phytium_dp, PX210_PHY1_APB_RESET, tmp | PHY1_APB_RESET);

		/* pipe reset */
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PIPE_RESET);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PIPE_RESET,
				   tmp & (~PHY1_PIPE_RESET_DEASSERT));

		/* config MODE/SEL to dp */
		mask = PHY1_DP_LANE_BIT << PHY1_DP_LANE_BIT_SHIFT;
		data = PHY1_DP_LANE_BIT << PHY1_DP_LANE_BIT_SHIFT;
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_SEL);
		tmp = (tmp & (~mask));
		phytium_phy_writel(phytium_dp, PX210_PHY1_SEL, tmp);

		/* config pll clock enable,  input 20 bit, lane active power state */
		tmp = (PHY1_PLL_EN << PHY1_PLL_EN_SHIFT) | (PHY1_BIT_20 << PHY1_BIT_20_SHIFT)
			| (PHY1_A0_ACTIVE << PHY1_A0_ACTIVE_SHIFT);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PMA_MISC, tmp);

		/* config single link */
		phytium_phy_writel(phytium_dp, PX210_PHY1_PLL_CFG, SINGLE_LINK);

		/* pipe reset */
		tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PIPE_RESET);
		phytium_phy_writel(phytium_dp, PX210_PHY1_PIPE_RESET,
				   tmp | PHY1_PIPE_RESET_DEASSERT);

		mask = PLL0_LOCK_DONE;
		do {
			mdelay(1);
			timeout--;
			tmp = phytium_phy_readl(phytium_dp, PX210_PHY1_PMA_CONTROL2);
		} while ((!(tmp & mask)) && timeout);

		if (timeout == 0) {
			DRM_ERROR("reset dp(%d) phy failed\n", port);
			ret = -1;
		}
		udelay(1);
	}

	mdelay(10);

	return ret;
}

static void px210_dp_hw_poweron_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dcreq_reg_base[port];
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | PANEL_POWER_ENABLE,
			   group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to poweron panel\n", __func__);
}

static void px210_dp_hw_poweroff_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dcreq_reg_base[port];
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | PANEL_POWER_DISABLE,
			   group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to poweroff panel\n", __func__);
}

static void px210_dp_hw_enable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, ret = 0;
	uint32_t group_offset = priv->dcreq_reg_base[port];

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | BACKLIGHT_ENABLE,
			   group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to enable backlight\n", __func__);
}

static void px210_dp_hw_disable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dcreq_reg_base[port];
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | BACKLIGHT_DISABLE,
			   group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to disable backlight\n", __func__);
}

static uint32_t px210_dp_hw_get_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int config;
	uint32_t group_offset = priv->address_transform_base;

	config = phytium_readl_reg(priv, group_offset, PX210_DC_ADDRESS_TRANSFORM_BACKLIGHT_VALUE);
	return ((config >> BACKLIGHT_VALUE_SHIFT) & BACKLIGHT_VALUE_MASK);
}

static int px210_dp_hw_set_backlight(struct phytium_dp_device *phytium_dp, uint32_t level)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dcreq_reg_base[port];
	int config = 0;
	int ret = 0;

	if (level > PX210_DP_BACKLIGHT_MAX) {
		ret = -EINVAL;
		goto out;
	}

	config = FLAG_REQUEST | CMD_BACKLIGHT | ((level & BACKLIGHT_MASK) << BACKLIGHT_SHIFT);
	phytium_writel_reg(priv, config, group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to set backlight\n", __func__);

out:
	return ret;
}

bool px210_dp_hw_spread_is_enable(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, config;
	uint32_t group_offset = priv->address_transform_base;

	config = phytium_readl_reg(priv, group_offset, PX210_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS);

	return ((config & DP_SPREAD_ENABLE(port)) ? true:false);
}

int px210_dp_hw_reset(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int timeout = 100, config, ret = 0;
	uint32_t group_offset = priv->address_transform_base;
	uint32_t group_offset_dp = priv->dp_reg_base[port];

	config = phytium_readl_reg(priv, group_offset, PX210_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS);
	config &= (~DC_DP_RESET_STATUS(port));

	phytium_writel_reg(priv, config, group_offset, PX210_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS);
	phytium_writel_reg(priv, FLAG_REQUEST | CMD_DC_DP_RESET,
			   priv->dcreq_reg_base[port], PX210_DCREQ_CMD_REGISTER);
	do {
		mdelay(10);
		timeout--;
		config = phytium_readl_reg(priv, group_offset,
					   PX210_DC_ADDRESS_TRANSFORM_DP_RESET_STATUS);
		if (config & DC_DP_RESET_STATUS(port))
			break;
	} while (timeout);
	if (timeout == 0) {
		DRM_ERROR("reset dc/dp pipe(%d) failed\n", port);
		ret = -1;
	}

	phytium_writel_reg(priv, AUX_CLK_DIVIDER, group_offset_dp, PHYTIUM_DP_AUX_CLK_DIVIDER);

	return ret;
}

uint8_t px210_dp_hw_get_source_lane_count(struct phytium_dp_device *phytium_dp)
{
	return px210_dp_source_lane_count[phytium_dp->port];
}

static struct phytium_dp_func px210_dp_funcs = {
	.dp_hw_get_source_lane_count = px210_dp_hw_get_source_lane_count,
	.dp_hw_reset = px210_dp_hw_reset,
	.dp_hw_spread_is_enable = px210_dp_hw_spread_is_enable,
	.dp_hw_set_backlight = px210_dp_hw_set_backlight,
	.dp_hw_get_backlight = px210_dp_hw_get_backlight,
	.dp_hw_disable_backlight = px210_dp_hw_disable_backlight,
	.dp_hw_enable_backlight = px210_dp_hw_enable_backlight,
	.dp_hw_poweroff_panel = px210_dp_hw_poweroff_panel,
	.dp_hw_poweron_panel = px210_dp_hw_poweron_panel,
	.dp_hw_init_phy = px210_dp_hw_init_phy,
	.dp_hw_set_phy_lane_setting = px210_dp_hw_set_phy_lane_setting,
	.dp_hw_set_phy_lane_and_rate = px210_dp_hw_set_phy_lane_and_rate,
};

void px210_dp_func_register(struct phytium_dp_device *phytium_dp)
{
	phytium_dp->funcs = &px210_dp_funcs;
}
