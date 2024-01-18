// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_dp_helper.h>
#include <drm/drm_modes.h>
#include "phytium_display_drv.h"
#include "phytium_dp.h"
#include "phytium_panel.h"

static int
phytium_dp_aux_set_backlight(struct phytium_panel *panel, unsigned int level)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);
	unsigned char vals[2] = { 0x0 };

	vals[0] = level;
	if (phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_BYTE_COUNT) {
		vals[0] = (level & 0xFF00) >> 8;
		vals[1] = (level & 0xFF);
	}

	if (drm_dp_dpcd_write(&phytium_dp->aux, DP_EDP_BACKLIGHT_BRIGHTNESS_MSB,
			       vals, sizeof(vals)) < 0) {
		DRM_DEBUG_KMS("Failed to write aux backlight level\n");
		return -EIO;
	}

	return 0;
}

static unsigned int phytium_dp_aux_get_backlight(struct phytium_panel *panel)
{
	unsigned char read_val[2] = { 0x0 };
	unsigned char level = 0;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	if (drm_dp_dpcd_read(&phytium_dp->aux, DP_EDP_BACKLIGHT_BRIGHTNESS_MSB,
			     &read_val, sizeof(read_val)) < 0) {
		DRM_DEBUG_KMS("Failed to read DPCD register 0x%x\n",
			       DP_EDP_BACKLIGHT_BRIGHTNESS_MSB);
		return 0;
	}

	level = read_val[0];
	if (phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_BYTE_COUNT)
		level = (read_val[0] << 8 | read_val[1]);

	return level;
}

static void set_aux_backlight_enable(struct phytium_panel *panel, bool enable)
{
	u8 reg_val = 0;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	if (!(phytium_dp->edp_dpcd[1] & DP_EDP_BACKLIGHT_AUX_ENABLE_CAP))
		return;

	if (drm_dp_dpcd_readb(&phytium_dp->aux, DP_EDP_DISPLAY_CONTROL_REGISTER,
			      &reg_val) < 0) {
		DRM_DEBUG_KMS("Failed to read DPCD register 0x%x\n",
				     DP_EDP_DISPLAY_CONTROL_REGISTER);
		return;
	}

	if (enable)
		reg_val |= DP_EDP_BACKLIGHT_ENABLE;
	else
		reg_val &= ~(DP_EDP_BACKLIGHT_ENABLE);

	if (drm_dp_dpcd_writeb(&phytium_dp->aux, DP_EDP_DISPLAY_CONTROL_REGISTER,
			       reg_val) != 1) {
		DRM_DEBUG_KMS("Failed to %s aux backlight\n",
			       enable ? "enable" : "disable");
	}
}

static void phytium_dp_aux_enable_backlight(struct phytium_panel *panel)
{
	unsigned char dpcd_buf, new_dpcd_buf, edp_backlight_mode;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	if (drm_dp_dpcd_readb(&phytium_dp->aux,
			      DP_EDP_BACKLIGHT_MODE_SET_REGISTER, &dpcd_buf) != 1) {
		DRM_DEBUG_KMS("Failed to read DPCD register 0x%x\n",
			       DP_EDP_BACKLIGHT_MODE_SET_REGISTER);
		return;
	}

	new_dpcd_buf = dpcd_buf;
	edp_backlight_mode = dpcd_buf & DP_EDP_BACKLIGHT_CONTROL_MODE_MASK;

	switch (edp_backlight_mode) {
	case DP_EDP_BACKLIGHT_CONTROL_MODE_PWM:
	case DP_EDP_BACKLIGHT_CONTROL_MODE_PRESET:
	case DP_EDP_BACKLIGHT_CONTROL_MODE_PRODUCT:
		new_dpcd_buf &= ~DP_EDP_BACKLIGHT_CONTROL_MODE_MASK;
		new_dpcd_buf |= DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD;
		break;

	/* Do nothing when it is already DPCD mode */
	case DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD:
	default:
		break;
	}

	if (new_dpcd_buf != dpcd_buf) {
		if (drm_dp_dpcd_writeb(&phytium_dp->aux,
			DP_EDP_BACKLIGHT_MODE_SET_REGISTER, new_dpcd_buf) < 0) {
			DRM_DEBUG_KMS("Failed to write aux backlight mode\n");
		}
	}

	set_aux_backlight_enable(panel, true);
	phytium_dp_aux_set_backlight(panel, panel->level);
}

static void phytium_dp_aux_disable_backlight(struct phytium_panel *panel)
{
	set_aux_backlight_enable(panel, false);
}

static void phytium_dp_aux_setup_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	if (phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_BYTE_COUNT)
		phytium_dp->panel.max = 0xFFFF;
	else
		phytium_dp->panel.max = 0xFF;

	phytium_dp->panel.min = 0;
	phytium_dp->panel.level = phytium_dp_aux_get_backlight(panel);
	phytium_dp->panel.backlight_enabled = (phytium_dp->panel.level != 0);
}

static void phytium_dp_hw_poweron_panel(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	phytium_dp->funcs->dp_hw_poweron_panel(phytium_dp);
}

static void phytium_dp_hw_poweroff_panel(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	phytium_dp->funcs->dp_hw_poweroff_panel(phytium_dp);
}

static int
phytium_dp_hw_set_backlight(struct phytium_panel *panel, uint32_t level)
{
	int ret;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	ret = phytium_dp->funcs->dp_hw_set_backlight(phytium_dp, level);

	return ret;
}

static uint32_t phytium_dp_hw_get_backlight(struct phytium_panel *panel)
{
	uint32_t ret;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	ret = phytium_dp->funcs->dp_hw_get_backlight(phytium_dp);

	return ret;
}

static void phytium_dp_hw_enable_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	phytium_dp->funcs->dp_hw_set_backlight(phytium_dp, phytium_dp->panel.level);
	phytium_dp->funcs->dp_hw_enable_backlight(phytium_dp);
}

static void phytium_dp_hw_disable_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	phytium_dp->funcs->dp_hw_disable_backlight(phytium_dp);
}

static void phytium_dp_hw_setup_backlight(struct phytium_panel *panel)
{
	struct drm_device *dev = panel->dev;
	struct phytium_display_private *priv = dev->dev_private;

	panel->max = priv->info.backlight_max;
	panel->min = 0;
	panel->level = phytium_dp_hw_get_backlight(panel);
}

void phytium_dp_panel_init_backlight_funcs(struct phytium_dp_device *phytium_dp)
{
	if (phytium_dp->edp_dpcd[1] & DP_EDP_TCON_BACKLIGHT_ADJUSTMENT_CAP &&
	   (phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_AUX_SET_CAP) &&
	   !(phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_PWM_PIN_CAP)) {
		DRM_DEBUG_KMS("AUX Backlight Control Supported!\n");
		phytium_dp->panel.setup_backlight = phytium_dp_aux_setup_backlight;
		phytium_dp->panel.enable_backlight = phytium_dp_aux_enable_backlight;
		phytium_dp->panel.disable_backlight = phytium_dp_aux_disable_backlight;
		phytium_dp->panel.set_backlight = phytium_dp_aux_set_backlight;
		phytium_dp->panel.get_backlight = phytium_dp_aux_get_backlight;
	} else {
		DRM_DEBUG_KMS("SE Backlight Control Supported!\n");
		phytium_dp->panel.setup_backlight = phytium_dp_hw_setup_backlight;
		phytium_dp->panel.enable_backlight = phytium_dp_hw_enable_backlight;
		phytium_dp->panel.disable_backlight = phytium_dp_hw_disable_backlight;
		phytium_dp->panel.set_backlight = phytium_dp_hw_set_backlight;
		phytium_dp->panel.get_backlight = phytium_dp_hw_get_backlight;
	}
	phytium_dp->panel.poweron = phytium_dp_hw_poweron_panel;
	phytium_dp->panel.poweroff = phytium_dp_hw_poweroff_panel;
	mutex_init(&phytium_dp->panel.panel_lock);
	phytium_dp->panel.dev = phytium_dp->dev;

	/* Upper limits from eDP 1.3 spec */
	phytium_dp->panel.panel_power_up_delay = 210; /* t1_t3 */
	phytium_dp->panel.backlight_on_delay = 50; /* t7 */
	phytium_dp->panel.backlight_off_delay = 50;
	phytium_dp->panel.panel_power_down_delay = 0; /* t10 */
	phytium_dp->panel.panel_power_cycle_delay = 510; /* t11 + t12 */
}

void phytium_dp_panel_release_backlight_funcs(struct phytium_dp_device *phytium_dp)
{
	phytium_dp->panel.setup_backlight = NULL;
	phytium_dp->panel.enable_backlight = NULL;
	phytium_dp->panel.disable_backlight = NULL;
	phytium_dp->panel.set_backlight = NULL;
	phytium_dp->panel.get_backlight = NULL;
	phytium_dp->panel.poweron = NULL;
	phytium_dp->panel.poweroff = NULL;
}

void phytium_panel_enable_backlight(struct phytium_panel *panel)
{

	if (panel->enable_backlight) {
		mutex_lock(&panel->panel_lock);
		msleep(panel->backlight_on_delay);
		panel->enable_backlight(panel);
		panel->backlight_enabled = true;
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_disable_backlight(struct phytium_panel *panel)
{
	if (panel->disable_backlight) {
		mutex_lock(&panel->panel_lock);
		panel->disable_backlight(panel);
		panel->backlight_enabled = false;
		msleep(panel->backlight_off_delay);
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_poweron(struct phytium_panel *panel)
{
	if (panel->poweron) {
		mutex_lock(&panel->panel_lock);
		panel->poweron(panel);
		panel->power_enabled = true;
		msleep(panel->panel_power_up_delay);
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_poweroff(struct phytium_panel *panel)
{
	if (panel->poweroff) {
		mutex_lock(&panel->panel_lock);
		msleep(panel->panel_power_down_delay);
		panel->poweroff(panel);
		panel->power_enabled = false;
		mutex_unlock(&panel->panel_lock);
	}
}

static uint32_t phytium_scale(uint32_t source_val,
		      uint32_t source_min, uint32_t source_max,
		      uint32_t target_min, uint32_t target_max)
{
	uint64_t target_val;

	WARN_ON(source_min > source_max);
	WARN_ON(target_min > target_max);

	/* defensive */
	source_val = clamp(source_val, source_min, source_max);

	/* avoid overflows */
	target_val = mul_u32_u32(source_val - source_min, target_max - target_min);
	target_val = DIV_ROUND_CLOSEST_ULL(target_val, source_max - source_min);
	target_val += target_min;

	return target_val;
}

static inline uint32_t
phytium_scale_hw_to_user(struct phytium_panel *panel, uint32_t hw_level, uint32_t user_max)
{
	return phytium_scale(hw_level, panel->min, panel->max,
			      0, user_max);
}

static inline uint32_t
phytium_scale_user_to_hw(struct phytium_panel *panel, u32 user_level, u32 user_max)
{
	return phytium_scale(user_level, 0, user_max,
			      panel->min, panel->max);
}

static int phytium_backlight_device_update_status(struct backlight_device *bd)
{
	struct phytium_panel *panel = bl_get_data(bd);
	struct drm_device *dev = panel->dev;
	uint32_t hw_level = 0;
	int ret = 0;

	DRM_DEBUG_KMS("updating phytium_backlight, brightness=%d/%d\n",
		       bd->props.brightness, bd->props.max_brightness);
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);
	hw_level = phytium_scale_user_to_hw(panel, bd->props.brightness, bd->props.max_brightness);

	if ((panel->set_backlight) && (panel->backlight_enabled)) {
		mutex_lock(&panel->panel_lock);
		ret = panel->set_backlight(panel, hw_level);
		panel->level = hw_level;
		mutex_unlock(&panel->panel_lock);
	}
	drm_modeset_unlock(&dev->mode_config.connection_mutex);

	return ret;
}

static int phytium_backlight_device_get_brightness(struct backlight_device *bd)
{
	struct phytium_panel *panel = bl_get_data(bd);
	struct drm_device *dev = panel->dev;
	uint32_t hw_level = 0;
	int ret;

	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);
	if (panel->get_backlight && panel->backlight_enabled) {
		mutex_lock(&panel->panel_lock);
		hw_level = panel->get_backlight(panel);
		panel->level = hw_level;
		mutex_unlock(&panel->panel_lock);
	}
	drm_modeset_unlock(&dev->mode_config.connection_mutex);
	ret = phytium_scale_hw_to_user(panel, hw_level, bd->props.max_brightness);
	DRM_DEBUG_KMS("get phytium_backlight, brightness=%d/%d\n",
		       ret, bd->props.max_brightness);

	return ret;
}

static const struct backlight_ops phytium_backlight_device_ops = {
	.update_status = phytium_backlight_device_update_status,
	.get_brightness = phytium_backlight_device_get_brightness,
};

int phytium_edp_backlight_device_register(struct phytium_dp_device *phytium_dp)
{
	struct backlight_properties props;
	char bl_name[16];

	if (phytium_dp->panel.setup_backlight) {
		mutex_lock(&phytium_dp->panel.panel_lock);
		phytium_dp->panel.setup_backlight(&phytium_dp->panel);
		mutex_unlock(&phytium_dp->panel.panel_lock);
	} else {
		return -EINVAL;
	}

	memset(&props, 0, sizeof(props));
	props.max_brightness = PHYTIUM_MAX_BL_LEVEL;
	props.type = BACKLIGHT_RAW;
	props.brightness = phytium_scale_hw_to_user(&phytium_dp->panel, phytium_dp->panel.level,
						    props.max_brightness);
	snprintf(bl_name, sizeof(bl_name), "phytium_bl%d", phytium_dp->port);

	phytium_dp->panel.bl_device =
		backlight_device_register(bl_name,
					  phytium_dp->connector.kdev,
					  &phytium_dp->panel,
					  &phytium_backlight_device_ops,
					  &props);

	if (IS_ERR(phytium_dp->panel.bl_device)) {
		DRM_ERROR("Failed to register backlight: %ld\n",
			    PTR_ERR(phytium_dp->panel.bl_device));
		phytium_dp->panel.bl_device = NULL;
		return -ENODEV;
	}

	DRM_DEBUG_KMS("Connector %s backlight sysfs interface registered\n",
		       phytium_dp->connector.name);

	return 0;
}

void phytium_edp_backlight_device_unregister(struct phytium_dp_device *phytium_dp)
{
	if (phytium_dp->panel.bl_device) {
		backlight_device_unregister(phytium_dp->panel.bl_device);
		phytium_dp->panel.bl_device = NULL;
	}
}
