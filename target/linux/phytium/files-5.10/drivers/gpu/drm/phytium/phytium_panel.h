/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PANEL_H__
#define __PHYTIUM_PANEL_H__
#include "phytium_dp.h"

#define PHYTIUM_MAX_BL_LEVEL		0xFF

struct phytium_panel {
	struct drm_device *dev;
	bool backlight_enabled;
	bool power_enabled;
	bool reserve1[2];
	unsigned int min;
	unsigned int level;
	unsigned int max;
	struct backlight_device *bl_device;
	void (*setup_backlight)(struct phytium_panel *panel);
	uint32_t (*get_backlight)(struct phytium_panel *panel);
	int (*set_backlight)(struct phytium_panel *panel, uint32_t level);
	void (*disable_backlight)(struct phytium_panel *panel);
	void (*enable_backlight)(struct phytium_panel *panel);
	void (*poweron)(struct phytium_panel *panel);
	void (*poweroff)(struct phytium_panel *panel);
	struct mutex panel_lock;
	uint32_t panel_power_up_delay;
	uint32_t backlight_on_delay;
	uint32_t backlight_off_delay;
	uint32_t panel_power_down_delay;
	uint32_t panel_power_cycle_delay;
};

void phytium_dp_panel_init_backlight_funcs(struct phytium_dp_device *phytium_dp);
void phytium_panel_release_backlight_funcs(struct phytium_dp_device *phytium_dp);
int phytium_edp_backlight_device_register(struct phytium_dp_device *phytium_dp);
void phytium_edp_backlight_device_unregister(struct phytium_dp_device *phytium_dp);
void phytium_panel_enable_backlight(struct phytium_panel *panel);
void phytium_panel_disable_backlight(struct phytium_panel *panel);
void phytium_panel_poweron(struct phytium_panel *panel);
void phytium_panel_poweroff(struct phytium_panel *panel);

#endif /* __PHYTIUM_PANEL_H__ */
