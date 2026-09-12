/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium display port DRM driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DP_H__
#define __FTD330_DP_H__

#define FTD330_DP_BACKLIGHT_MAX				100
#define FTD330_DP_BACKLIGHT_MIN				0

#define MAP_BL 1
#define BL_DEBUG 0
#define PHY_MATRIX_DEBUG 0

struct dp_drv_phy_matrix {
    int dp0_drv_mgnfs_val[4][4][4];
    int dp1_drv_mgnfs_val[4][4][4];
    int dp2_drv_mgnfs_val[4][4][4];

    int dp0_drv_cpost_val[4][4][4];
    int dp1_drv_cpost_val[4][4][4];
    int dp2_drv_cpost_val[4][4][4];
};

struct edp_drv_backlight_map {
    int edp_bright_to_pwm[101];
    int edp_pwm_to_bright[101];
};

extern struct dp_drv_phy_matrix dp_drv_phy_val;
extern struct edp_drv_backlight_map edp_drv_bl_map;

extern int inv_gamma_val[101];
extern int gamma_val[101];
void ftd330_dp_func_register(struct phytium_dp_device *phytium_dp);
void ftd330_dp_poweron_panel(struct ftd330_drm_private *priv, int port);
bool ftd330_dp_hw_panel_is_poweron(struct phytium_dp_device *phytium_dp);
#endif /* __FTD330_DP_H__ */

