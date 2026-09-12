/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PANEL_H__
#define __PHYTIUM_PANEL_H__

#include "phytium_dp.h"
#include "phytium_edp_pwm.h"

#define PHYTIUM_MAX_8BIT_BL_LEVEL		0xFF
#define PHYTIUM_MAX_16BIT_BL_LEVEL		0xFFFF

struct phytium_edp_backlight_info {
	u8 pwmgen_bit_count;
	u8 pwm_freq_pre_divider;
	u8 current_mode;
	u16 max;

	bool lsb_reg_used : 1;
	bool aux_enable : 1;
	bool aux_set : 1;
	bool pwm_set : 1;
};

struct edp_drv_panel_timing {
	uint32_t drv_panel_power_up_delay;
	uint32_t drv_backlight_on_delay;
	uint32_t drv_backlight_off_delay;
	uint32_t drv_panel_power_down_delay;
	uint32_t drv_panel_power_cycle_delay;
};
extern struct edp_drv_panel_timing edp_drv_panel_time;

struct edp_panel_timing {
	uint32_t panel_power_up_delay;
	uint32_t backlight_on_delay;
	uint32_t backlight_off_delay;
	uint32_t panel_power_down_delay;
	uint32_t panel_power_cycle_delay;
};

struct phytium_panel {
	struct drm_device *dev;
	bool backlight_enabled;
	bool power_enabled;
	bool reserve1[2];
	unsigned int min;
	unsigned int level;
	unsigned int max;
	unsigned int save_level;
	struct backlight_device *bl_device;
	struct phytium_edp_backlight_info phytium_bl_info;
	void (*setup_backlight)(struct phytium_panel *panel);
	uint32_t (*get_backlight)(struct phytium_panel *panel);
	int (*set_backlight)(struct phytium_panel *panel, uint32_t level);
	void (*disable_backlight)(struct phytium_panel *panel);
	void (*enable_backlight)(struct phytium_panel *panel);
	void (*poweron)(struct phytium_panel *panel);
	void (*poweroff)(struct phytium_panel *panel);
	struct mutex panel_lock;
	struct edp_panel_timing edp_panel_time;
	struct phytium_bl_pwm_chip pwm_chip;
};

void phytium_dp_panel_init_backlight_funcs(struct phytium_dp_device *phytium_dp);
void phytium_panel_release_backlight_funcs(struct phytium_dp_device *phytium_dp);
int phytium_edp_backlight_device_register(struct phytium_dp_device *phytium_dp);
void phytium_edp_backlight_device_unregister(struct phytium_dp_device *phytium_dp);
void phytium_panel_enable_backlight(struct phytium_panel *panel);
void phytium_panel_disable_backlight(struct phytium_panel *panel);
void phytium_panel_poweron(struct phytium_panel *panel);
void phytium_panel_poweroff(struct phytium_panel *panel);
void phytium_dp_panel_release_backlight_funcs(struct phytium_dp_device *phytium_dp);
void phytium_edp_dpcd_backlight_init(struct phytium_dp_device *phytium_dp);
void phytium_dp_panel_init_panel_power_funcs(struct phytium_dp_device *phytium_dp);

#endif /* __PHYTIUM_PANEL_H__ */
