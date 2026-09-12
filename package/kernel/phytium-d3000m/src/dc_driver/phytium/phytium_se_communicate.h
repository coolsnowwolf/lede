/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */
#ifndef __PHYTIUM_SE_COMMUNICATE_H_
#define __PHYTIUM_SE_COMMUNICATE_H_

#include "phytium_dp.h"

#define SE_REG_BASE		0x32c080
#define SE_AP_STAT		0x0
#define SE_AP_SET		0x4
#define SE_AP_CLEAR		0x8
#define SE_SCP_STAT		0x0c
#define SE_SCP_SET		0x10
#define SE_SCP_CLEAR	0x14
#define SE_PAYLOAD_0		0x18
#define	SE_PAYLOAD_1		0x1c

#define DCDP_POWER_PROTOCAL_ID	0x11
#define DCDP_POWER_MESSAGE_ID	0x04
#define DCDP_POWER_PAYLOAD1_RESERVEE	(1 << 30)
#define DCDP_POWER_STATE_ENABLE	0x08
#define DCDP_POWER_STATE_DISABLE	0x00

#define DCDP_CHANGE_PXL_CLK_PROTOCAL_ID	0x14
#define DCDP_CHANGE_PXL_CLK_MESSAGE_ID	0x05

void phytium_display_power_request_se(struct ftd330_drm_private *priv, bool enable, uint32_t display_id);
void phytium_change_pxlclk_se(struct ftd330_drm_private *priv, uint32_t display_id, uint32_t pixel_clk);

#ifdef CONFIG_PHYTIUM_LOW_FPS
void phytium_change_edp_lowfps_pxlclk_se(struct ftd330_drm_private *priv,
											uint32_t physical_display_id, uint32_t pixel_clk);
#endif
#endif
