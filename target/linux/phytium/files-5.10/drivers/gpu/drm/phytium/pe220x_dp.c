// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium display port DRM driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include "phytium_display_drv.h"
#include "pe220x_reg.h"
#include "phytium_dp.h"
#include "pe220x_dp.h"

static uint8_t pe220x_dp_source_lane_count[2] = {1, 1};

/* [reg][ling_rate 1.62->8.1] */
static int vco_val[12][4] = {
	{0x0509, 0x0509, 0x0509, 0x0509}, /* CP_PADJ		 */
	{0x0f00, 0x0f00, 0x0f00, 0x0f00}, /* CP_IADJ		 */
	{0x0F08, 0x0F08, 0x0F08, 0x0F08}, /* FILT_PADJ		 */
	{0x0061, 0x006C, 0x006C, 0x0051}, /* INTDIV		 */
	{0x3333, 0x0000, 0x0000, 0x0000}, /* FRACDIVL		 */
	{0x0000, 0x0000, 0x0000, 0x0000}, /* FRACDIVH		 */
	{0x0042, 0x0048, 0x0048, 0x0036}, /* HIGH_THR		 */
	{0x0002, 0x0002, 0x0002, 0x0002}, /* PDIAG_CTRL		 */
	{0x0c5e, 0x0c5e, 0x0c5e, 0x0c5e}, /* VCOCAL_PLLCNT_START */
	{0x00c7, 0x00c7, 0x00c7, 0x00c7}, /* LOCK_PEFCNT	 */
	{0x00c7, 0x00c7, 0x00c7, 0x00c7}, /* LOCK_PLLCNT_START	 */
	{0x0005, 0x0005, 0x0005, 0x0005}, /* LOCK_PLLCNT_THR	 */
};

/* [link_rate][swing][emphasis] */
static int mgnfs_val[4][4][4] = {
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
		{0x001f, 0x0013, 0x005, 0x0000},
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

/* [link_rate][swing][emphasis] */
static int cpost_val[4][4][4] = {
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
		{0x0005, 0x0014, 0x0022, 0x002e},
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

static int pe220x_dp_hw_set_phy_lane_and_rate(struct phytium_dp_device *phytium_dp,
					     uint8_t link_lane_count, uint32_t link_rate)
{
	int port = phytium_dp->port%2;
	int i = 0, data, tmp, tmp1, index = 0, mask = 0;
	int timeout = 500, ret = 0;

	/* set pma powerdown */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (A3_POWERDOWN3 << (i * A3_POWERDOWN3_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA0_POWER(port), data);

	/* lane pll disable */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++) {
		data |= (PLL_EN << (i * PLL_EN_SHIFT));
		mask |= (((1<<PLL_EN_SHIFT) - 1) << (i * PLL_EN_SHIFT));
	}
	data &= ~mask;
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL_EN(port), data);

	/* pma pll disable */
	data = CONTROL_ENABLE & (~CONTROL_ENABLE_MASK);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA_CONTROL(port), data);

	/* read pma pll disable state */
	mdelay(2);
	phytium_phy_readl(phytium_dp, PE220X_PHY_PMA_CONTROL2(port));

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
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_CLK_SEL(port), tmp);
	phytium_phy_writel(phytium_dp, PE220X_PHY_HSCLK0_SEL(port), HSCLK_LINK_0);
	phytium_phy_writel(phytium_dp, PE220X_PHY_HSCLK0_DIV(port), tmp1);

	/* config digital pll for link0 */
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLLDRC0_CTRL(port), PLLDRC_LINK0);

	/* common for all rate */
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_DSM_M0(port), PLL0_DSM_M0);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_VCOCAL_START(port),
			   PLL0_VCOCAL_START);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_VCOCAL_CTRL(port),
			   PLL0_VCOCAL_CTRL);

	/* different for all rate */
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_CP_PADJ(port),
			   vco_val[0][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_CP_IADJ(port),
			   vco_val[1][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_CP_FILT_PADJ(port),
			   vco_val[2][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_INTDIV(port),
			   vco_val[3][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_FRACDIVL(port),
			   vco_val[4][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_FRACDIVH(port),
			   vco_val[5][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_HIGH_THR(port),
			   vco_val[6][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_PDIAG_CTRL(port),
			   vco_val[7][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_VCOCAL_PLLCNT_START(port),
			   vco_val[8][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_LOCK_PEFCNT(port),
			   vco_val[9][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_LOCK_PLLCNT_START(port),
			   vco_val[10][index]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_LOCK_PLLCNT_THR(port),
			   vco_val[11][index]);

	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_PSC_A0(port),
			   PLL0_TX_PSC_A0);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_PSC_A2(port),
			   PLL0_TX_PSC_A2);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_PSC_A3(port),
			   PLL0_TX_PSC_A3);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_PSC_A0(port),
			   PLL0_RX_PSC_A0);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_PSC_A2(port),
			   PLL0_RX_PSC_A2);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_PSC_A3(port),
			   PLL0_RX_PSC_A3);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_PSC_CAL(port),
			   PLL0_RX_PSC_CAL);

	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_XCVR_CTRL(port),
			   PLL0_XCVR_CTRL);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_GCSM1_CTRL(port),
			   PLL0_RX_GCSM1_CTRL);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_GCSM2_CTRL(port),
			   PLL0_RX_GCSM2_CTRL);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_RX_PERGCSM_CTRL(port),
			   PLL0_RX_PERGCSM_CTRL);

	/* pma pll enable */
	data = CONTROL_ENABLE;
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA_CONTROL(port), data);

	/* lane pll enable */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (PLL_EN << (i * PLL_EN_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL_EN(port), data);

	/* set pma power active */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (A0_ACTIVE << (i * A0_ACTIVE_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA0_POWER(port), data);

	mask = PLL0_LOCK_DONE;
	do {
		mdelay(1);
		timeout--;
		tmp = phytium_phy_readl(phytium_dp, PE220X_PHY_PMA_CONTROL2(port));
	} while ((!(tmp & mask)) && timeout);

	if (timeout == 0) {
		DRM_ERROR("dp(%d) phy pll lock failed\n", port);
		ret = -1;
	}
	udelay(1);

	return ret;
}

static void pe220x_dp_hw_set_phy_lane_setting(struct phytium_dp_device *phytium_dp,
					     uint32_t link_rate, uint8_t train_set)
{
	int port = phytium_dp->port % 3;
	int voltage_swing = 0;
	int pre_emphasis = 0, link_rate_index = 0;

	switch (train_set & DP_TRAIN_VOLTAGE_SWING_MASK) {
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_1:
		voltage_swing = 1;
		break;
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_2:
		voltage_swing = 2;
		break;
	case DP_TRAIN_VOLTAGE_SWING_LEVEL_3:
		voltage_swing = 3;
		break;
	default:
		voltage_swing = 0;
		break;
	}

	switch (train_set & DP_TRAIN_PRE_EMPHASIS_MASK) {
	case DP_TRAIN_PRE_EMPH_LEVEL_1:
		pre_emphasis = 1;
		break;
	case DP_TRAIN_PRE_EMPH_LEVEL_2:
		pre_emphasis = 2;
		break;
	case DP_TRAIN_PRE_EMPH_LEVEL_3:
		pre_emphasis = 3;
		break;
	default:
		pre_emphasis = 0;
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

	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_DIAG_ACYA(port), LOCK);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_TXCC_CTRL(port), TX_TXCC_CTRL);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_DRV(port), TX_DRV);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_MGNFS(port),
			   mgnfs_val[link_rate_index][voltage_swing][pre_emphasis]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_CPOST(port),
			   cpost_val[link_rate_index][voltage_swing][pre_emphasis]);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL0_TX_DIAG_ACYA(port), UNLOCK);
}

static int pe220x_dp_hw_init_phy(struct phytium_dp_device *phytium_dp)
{
	int port = phytium_dp->port;
	int i = 0, data, tmp, mask;
	int timeout = 500, ret = 0;

	phytium_phy_writel(phytium_dp, PE220X_PHY_APB_RESET(port), APB_RESET);
	phytium_phy_writel(phytium_dp, PE220X_PHY_PIPE_RESET(port), RESET);

	/* config lane to dp mode */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (LANE_BIT << (i * LANE_BIT_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_MODE(port), data);

	/* pll clock enable */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (PLL_EN << (i * PLL_EN_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL_EN(port), data);

	/* config input 20 bit */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (BIT_20 << (i * BIT_20_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA_WIDTH(port), data);

	/* config lane active power state */
	data = 0;
	for (i = 0; i < phytium_dp->source_max_lane_count; i++)
		data |= (A0_ACTIVE << (i * A0_ACTIVE_SHIFT));
	phytium_phy_writel(phytium_dp, PE220X_PHY_PMA0_POWER(port), data);

	/* link reset */
	phytium_phy_writel(phytium_dp, PE220X_PHY_LINK_RESET(port), LINK_RESET);

	phytium_phy_writel(phytium_dp, PE220X_PHY_SGMII_DPSEL_INIT(port), DP_SEL);

	/* config single link */
	phytium_phy_writel(phytium_dp, PE220X_PHY_PLL_CFG(port), SINGLE_LINK);

	/* pipe reset */
	phytium_phy_writel(phytium_dp, PE220X_PHY_PIPE_RESET(port), RESET_DEASSERT);

	mask = PLL0_LOCK_DONE;
	do {
		mdelay(1);
		timeout--;
		tmp = phytium_phy_readl(phytium_dp, PE220X_PHY_PMA_CONTROL2(port));
	} while ((!(tmp & mask)) && timeout);

	if (timeout == 0) {
		DRM_ERROR("reset dp(%d) phy failed\n", port);
		ret = -1;
	}
	udelay(1);

	return ret;
}

static void pe220x_dp_hw_poweron_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | PANEL_POWER_ENABLE,
			   0, PE220X_DC_CMD_REGISTER(port));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(port),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to poweron panel\n", __func__);
}

static void pe220x_dp_hw_poweroff_panel(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | PANEL_POWER_DISABLE,
			   0, PE220X_DC_CMD_REGISTER(port));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(port),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to poweroff panel\n", __func__);
}

static void pe220x_dp_hw_enable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port, ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | BACKLIGHT_ENABLE,
			   0, PE220X_DC_CMD_REGISTER(port));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(port),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to enable backlight\n", __func__);
}

static void pe220x_dp_hw_disable_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int ret = 0;

	phytium_writel_reg(priv, FLAG_REQUEST | CMD_BACKLIGHT | BACKLIGHT_DISABLE,
			   0, PE220X_DC_CMD_REGISTER(port));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(port),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to disable backlight\n", __func__);
}

static uint32_t pe220x_dp_hw_get_backlight(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int config;
	uint32_t group_offset = priv->address_transform_base;

	config = phytium_readl_reg(priv, group_offset, PE220X_DC_ADDRESS_TRANSFORM_BACKLIGHT_VALUE);
	return ((config >> BACKLIGHT_VALUE_SHIFT) & BACKLIGHT_VALUE_MASK);
}

static int pe220x_dp_hw_set_backlight(struct phytium_dp_device *phytium_dp, uint32_t level)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	int config = 0;
	int ret = 0;

	if (level > PE220X_DP_BACKLIGHT_MAX) {
		ret = -EINVAL;
		goto out;
	}

	config = FLAG_REQUEST | CMD_BACKLIGHT | ((level & BACKLIGHT_MASK) << BACKLIGHT_SHIFT);
	phytium_writel_reg(priv, config, 0, PE220X_DC_CMD_REGISTER(port));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(port),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to set backlight\n", __func__);
out:
	return ret;
}

bool pe220x_dp_hw_spread_is_enable(struct phytium_dp_device *phytium_dp)
{
	return false;
}

int pe220x_dp_hw_reset(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev = phytium_dp->dev;
	struct phytium_display_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];

	phytium_writel_reg(priv, DP_RESET, group_offset, PE220X_DP_CONTROLLER_RESET);
	udelay(500);
	phytium_writel_reg(priv, AUX_CLK_DIVIDER_100, group_offset, PHYTIUM_DP_AUX_CLK_DIVIDER);
	phytium_writel_reg(priv, SUPPORT_EDP_1_4, group_offset, PHYTIUM_EDP_CRC_ENABLE);

	return 0;
}

uint8_t pe220x_dp_hw_get_source_lane_count(struct phytium_dp_device *phytium_dp)
{
	return pe220x_dp_source_lane_count[phytium_dp->port];
}

static struct phytium_dp_func pe220x_dp_funcs = {
	.dp_hw_get_source_lane_count = pe220x_dp_hw_get_source_lane_count,
	.dp_hw_reset = pe220x_dp_hw_reset,
	.dp_hw_spread_is_enable = pe220x_dp_hw_spread_is_enable,
	.dp_hw_set_backlight = pe220x_dp_hw_set_backlight,
	.dp_hw_get_backlight = pe220x_dp_hw_get_backlight,
	.dp_hw_disable_backlight = pe220x_dp_hw_disable_backlight,
	.dp_hw_enable_backlight = pe220x_dp_hw_enable_backlight,
	.dp_hw_poweroff_panel = pe220x_dp_hw_poweroff_panel,
	.dp_hw_poweron_panel = pe220x_dp_hw_poweron_panel,
	.dp_hw_init_phy = pe220x_dp_hw_init_phy,
	.dp_hw_set_phy_lane_setting = pe220x_dp_hw_set_phy_lane_setting,
	.dp_hw_set_phy_lane_and_rate = pe220x_dp_hw_set_phy_lane_and_rate,
};

void pe220x_dp_func_register(struct phytium_dp_device *phytium_dp)
{
	phytium_dp->funcs = &pe220x_dp_funcs;
}
