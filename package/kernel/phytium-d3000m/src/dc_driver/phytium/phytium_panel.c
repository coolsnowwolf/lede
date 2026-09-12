// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#include <linux/version.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
#include <drm/display/drm_dp.h>
#include <drm/display/drm_dp_helper.h>
#else
#include <drm/drm_dp_helper.h>
#endif
#include <drm/drm_modes.h>
#include "phytium_dp_reg.h"
#include "phytium_dp.h"
#include "phytium_panel.h"
#include "phytium_edp_pwm.h"
#include "ftd330_dp.h"

/* Upper limits from eDP 1.3 spec */
struct edp_drv_panel_timing edp_drv_panel_time = {
	.drv_panel_power_up_delay = 210,	/* t1_t3 */
	.drv_backlight_on_delay = 50,	/* t7 */
	.drv_backlight_off_delay = 50,
	.drv_panel_power_down_delay = 500,	/* t10 */
	.drv_panel_power_cycle_delay = 510	/* t11 + t12 */
};

static int
phytium_edp_backlight_probe_max(struct drm_dp_aux *aux, struct phytium_edp_backlight_info *bl,
			    u16 driver_pwm_freq_hz, const u8 edp_dpcd[EDP_DISPLAY_CTL_CAP_SIZE])
{
	int fxp, fxp_min, fxp_max, fxp_actual, f = 1;
	int ret;
	u8 pn, pn_min, pn_max;

	if (!bl->aux_set)
		return 0;

	ret = drm_dp_dpcd_readb(aux, DP_EDP_PWMGEN_BIT_COUNT, &pn);
	if (ret != 1) {
		DRM_DEBUG_KMS("%s: Failed to read pwmgen bit count cap: %d\n",
			    aux->name, ret);
		return -ENODEV;
	}

	pn &= DP_EDP_PWMGEN_BIT_COUNT_MASK;
	bl->max = (1 << pn) - 1;
	if (!driver_pwm_freq_hz)
		return 0;

	/*
	 * Set PWM Frequency divider to match desired frequency provided by the driver.
	 * The PWM Frequency is calculated as 27Mhz / (F x P).
	 * - Where F = PWM Frequency Pre-Divider value programmed by field 7:0 of the
	 *             EDP_BACKLIGHT_FREQ_SET register (DPCD Address 00728h)
	 * - Where P = 2^Pn, where Pn is the value programmed by field 4:0 of the
	 *             EDP_PWMGEN_BIT_COUNT register (DPCD Address 00724h)
	 */

	/* Find desired value of (F x P)
	 * Note that, if F x P is out of supported range, the maximum value or minimum value will
	 * applied automatically. So no need to check that.
	 */
	fxp = DIV_ROUND_CLOSEST(1000 * DP_EDP_BACKLIGHT_FREQ_BASE_KHZ, driver_pwm_freq_hz);

	/* Use highest possible value of Pn for more granularity of brightness adjustment while
	 * satisfying the conditions below.
	 * - Pn is in the range of Pn_min and Pn_max
	 * - F is in the range of 1 and 255
	 * - FxP is within 25% of desired value.
	 *   Note: 25% is arbitrary value and may need some tweak.
	 */
	ret = drm_dp_dpcd_readb(aux, DP_EDP_PWMGEN_BIT_COUNT_CAP_MIN, &pn_min);
	if (ret != 1) {
		DRM_DEBUG_KMS("%s: Failed to read pwmgen bit count cap min: %d\n",
			    aux->name, ret);
		return 0;
	}
	ret = drm_dp_dpcd_readb(aux, DP_EDP_PWMGEN_BIT_COUNT_CAP_MAX, &pn_max);
	if (ret != 1) {
		DRM_DEBUG_KMS("%s: Failed to read pwmgen bit count cap max: %d\n",
			    aux->name, ret);
		return 0;
	}
	pn_min &= DP_EDP_PWMGEN_BIT_COUNT_MASK;
	pn_max &= DP_EDP_PWMGEN_BIT_COUNT_MASK;

	/* Ensure frequency is within 25% of desired value */
	fxp_min = DIV_ROUND_CLOSEST(fxp * 3, 4);
	fxp_max = DIV_ROUND_CLOSEST(fxp * 5, 4);
	if (fxp_min < (1 << pn_min) || (255 << pn_max) < fxp_max) {
		DRM_DEBUG_KMS(
			    "%s: Driver defined backlight frequency (%d) out of range\n",
			    aux->name, driver_pwm_freq_hz);
		return 0;
	}

	for (pn = pn_max; pn >= pn_min; pn--) {
		f = clamp(DIV_ROUND_CLOSEST(fxp, 1 << pn), 1, 255);
		fxp_actual = f << pn;
		if (fxp_min <= fxp_actual && fxp_actual <= fxp_max)
			break;
	}

	ret = drm_dp_dpcd_writeb(aux, DP_EDP_PWMGEN_BIT_COUNT, pn);
	if (ret != 1) {
		DRM_DEBUG_KMS("%s: Failed to write aux pwmgen bit count: %d\n",
			    aux->name, ret);
		return 0;
	}
	bl->pwmgen_bit_count = pn;
	bl->max = (1 << pn) - 1;

	if (edp_dpcd[2] & DP_EDP_BACKLIGHT_FREQ_AUX_SET_CAP) {
		bl->pwm_freq_pre_divider = f;
		DRM_DEBUG_KMS("%s: Using backlight frequency from driver (%dHz)\n",
			    aux->name, driver_pwm_freq_hz);
	}

	return 0;
}

static int
phytium_edp_backlight_probe_state(struct drm_dp_aux *aux,
									struct phytium_edp_backlight_info *bl)
{
	int ret;
	u8 buf[2];
	u8 mode_reg;

	ret = drm_dp_dpcd_readb(aux, DP_EDP_BACKLIGHT_MODE_SET_REGISTER, &mode_reg);
	if (ret != 1) {
		DRM_DEBUG_KMS("%s: Failed to read backlight mode: %d\n",
			    aux->name, ret);
		return ret < 0 ? ret : -EIO;
	}

	bl->current_mode = (mode_reg & DP_EDP_BACKLIGHT_CONTROL_MODE_MASK);
	if (!bl->aux_set)
		return 0;

	if (bl->current_mode == DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD) {
		int size = 1 + bl->lsb_reg_used;

		ret = drm_dp_dpcd_read(aux, DP_EDP_BACKLIGHT_BRIGHTNESS_MSB, buf, size);
		if (ret != size) {
			DRM_DEBUG_KMS("%s: Failed to read backlight level: %d\n",
				    aux->name, ret);
			return ret < 0 ? ret : -EIO;
		}

		if (bl->lsb_reg_used)
			return (buf[0] << 8) | buf[1];
		else
			return buf[0];
	}

	/*
	 * If we're not in DPCD control mode yet, the programmed brightness value is meaningless and
	 * the driver should assume max brightness
	 */
	return bl->max;
}

static void phytium_dp_aux_set_dpcd_control_mode(struct phytium_panel *panel, u8 mode)
{
	unsigned char dpcd_buf;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	if (panel->phytium_bl_info.current_mode != mode) {
		if (drm_dp_dpcd_readb(&phytium_dp->aux,
			DP_EDP_BACKLIGHT_MODE_SET_REGISTER, &dpcd_buf) != 1) {
			DRM_DEBUG_KMS("Failed to read DPCD register 0x%x\n",
			DP_EDP_BACKLIGHT_MODE_SET_REGISTER);
		}
		dpcd_buf &= ~DP_EDP_BACKLIGHT_CONTROL_MODE_MASK;
		dpcd_buf |= mode;
		if (drm_dp_dpcd_writeb(&phytium_dp->aux,
			DP_EDP_BACKLIGHT_MODE_SET_REGISTER, dpcd_buf) < 0) {
			DRM_DEBUG_KMS("Failed to write aux backlight mode\n");
		}
		panel->phytium_bl_info.current_mode = mode;
		DRM_DEBUG_KMS("Set aux backlight mode = DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD\n");
	}
}

static int
phytium_edp_backlight_init(struct drm_dp_aux *aux, struct phytium_edp_backlight_info *bl,
		       u16 driver_pwm_freq_hz, const u8 edp_dpcd[EDP_DISPLAY_CTL_CAP_SIZE],
		       u16 *current_level)
{
	int ret;

	if (edp_dpcd[1] & DP_EDP_BACKLIGHT_AUX_ENABLE_CAP)
		bl->aux_enable = true;
	if (edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_AUX_SET_CAP)
		bl->aux_set = true;
	if (edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_BYTE_COUNT)
		bl->lsb_reg_used = true;
	if (edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_PWM_PIN_CAP)
		bl->pwm_set = true;

	/* Sanity check caps */
	if (!bl->aux_set && !bl->pwm_set) {
		DRM_DEBUG_KMS("%s: Panel supports neither AUX or PWM brightness control? Aborting\n",
			    aux->name);
		return -EINVAL;
	}

	ret = phytium_edp_backlight_probe_max(aux, bl, driver_pwm_freq_hz, edp_dpcd);
	if (ret < 0)
		return ret;

	ret = phytium_edp_backlight_probe_state(aux, bl);
	if (ret < 0)
		return ret;
	*current_level = ret;

	DRM_DEBUG_KMS("%s: Found backlight: aux_set=%d aux_enable=%d pwm_set=%d mode=%d\n",
		aux->name, bl->aux_set, bl->aux_enable, bl->pwm_set, bl->current_mode);
	if (bl->aux_set) {
		DRM_DEBUG_KMS("%s: Backlight caps: level=%d/%d pwm_freq_pre_divider=%d lsb_reg_used=%d\n",
			aux->name, *current_level, bl->max, bl->pwm_freq_pre_divider,
			bl->lsb_reg_used);
	}

	return 0;
}

void
phytium_edp_dpcd_backlight_init(struct phytium_dp_device *phytium_dp)
{
	struct phytium_edp_backlight_info *phytium_bl_info = &phytium_dp->panel.phytium_bl_info;
	u16 current_level;
	u16 driver_pwm_freq_hz = 1000;
	int ret;
	FTD330_LOG_TRACE;

	if (!(phytium_dp->edp_dpcd[1] & DP_EDP_TCON_BACKLIGHT_ADJUSTMENT_CAP)) {
		DRM_INFO("DP AUX backlight is not supported\n");
		return;
	}

	ret = phytium_edp_backlight_init(&phytium_dp->aux, phytium_bl_info,
				     driver_pwm_freq_hz, phytium_dp->edp_dpcd,
				     &current_level);
	if (ret < 0)
		DRM_ERROR("DRM edp backlight init falied ret = %d\n", ret);
}
static int
phytium_dp_aux_set_backlight(struct phytium_panel *panel, unsigned int level)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);
	unsigned char vals[2] = { 0x0 };

	FTD330_LOG_TRACE;

	phytium_edp_backlight_probe_state(&phytium_dp->aux, &panel->phytium_bl_info);
	phytium_dp_aux_set_dpcd_control_mode(panel,
										DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD);
	vals[0] = level;
	if (panel->phytium_bl_info.lsb_reg_used) {
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
	uint32_t level = 0;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	if (drm_dp_dpcd_read(&phytium_dp->aux, DP_EDP_BACKLIGHT_BRIGHTNESS_MSB,
			     &read_val, sizeof(read_val)) < 0) {
		DRM_DEBUG_KMS("Failed to read DPCD register 0x%x\n",
			       DP_EDP_BACKLIGHT_BRIGHTNESS_MSB);
		return 0;
	}

	level = read_val[0];
	if (panel->phytium_bl_info.lsb_reg_used)
		level = (read_val[0] << 8 | read_val[1]);

	return level;
}

static void set_aux_backlight_enable(struct phytium_panel *panel, bool enable)
{
	u8 reg_val = 0;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	if (!panel->phytium_bl_info.aux_enable)
		return;

	/* we need to skip communicate with panel if panel
	 * is power off.
	 * */
	if (!ftd330_dp_hw_panel_is_poweron(phytium_dp)) {
		return;
	}

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
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	FTD330_LOG_TRACE;
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
		new_dpcd_buf &= ~DP_EDP_BACKLIGHT_CONTROL_MODE_MASK;
		new_dpcd_buf |= DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD;
		break;
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
	drm_dp_dpcd_readb(&phytium_dp->aux,
			      DP_EDP_BACKLIGHT_MODE_SET_REGISTER, &dpcd_buf);

	/* For some OLED panels, such as Samsung,
	   AUX enables the backlight while requiring
	   gpio to pull high at the same time
	   to meet hardware timing requirements. */
	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	reg |= EDP_BL_ENABLE;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);

	set_aux_backlight_enable(panel, true);
	phytium_dp_aux_set_backlight(panel, panel->level);
}

static void phytium_dp_aux_disable_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	uint32_t group_offset = priv->dp_reg_base[port];
	uint32_t reg;

	FTD330_LOG_TRACE;
	set_aux_backlight_enable(panel, false);

	/* For some OLED panels, such as Samsung,
	   AUX disables the backlight while requiring
	   gpio to pull low at the same time
	   to meet hardware timing requirements. */
	reg = phytium_readl_reg(priv, group_offset, EDP_BACKLIGHT_CONTROL);
	reg &= ~EDP_BL_DISABLE_MASK;
	phytium_writel_reg(priv, reg, group_offset, EDP_BACKLIGHT_CONTROL);
}

static void phytium_dp_aux_setup_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	phytium_dp->panel.max = panel->phytium_bl_info.max;

	phytium_dp->panel.min = 0;
	phytium_dp->panel.level = phytium_dp_aux_get_backlight(panel);
	phytium_dp->panel.backlight_enabled = (phytium_dp->panel.level != 0);
	if (phytium_dp->panel.level <= phytium_dp->panel.min)
		phytium_dp->panel.level = phytium_dp->panel.max;

	phytium_edp_backlight_probe_state(&phytium_dp->aux, &panel->phytium_bl_info);
	if (panel->phytium_bl_info.current_mode ==
		DP_EDP_BACKLIGHT_CONTROL_MODE_PWM ||
		panel->phytium_bl_info.current_mode ==
		DP_EDP_BACKLIGHT_CONTROL_MODE_PRESET)
			phytium_dp_aux_set_dpcd_control_mode(panel,
									DP_EDP_BACKLIGHT_CONTROL_MODE_DPCD);

}

static void phytium_dp_hw_poweron_panel(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	phytium_dp->funcs->dp_hw_poweron_panel(phytium_dp);
}

static void phytium_dp_hw_poweroff_panel(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	phytium_dp->funcs->dp_hw_poweroff_panel(phytium_dp);
}

static int
phytium_dp_hw_set_backlight(struct phytium_panel *panel, uint32_t level)
{
	int ret;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	ret = phytium_dp->funcs->dp_hw_set_backlight(phytium_dp, level);

	return ret;
}

static uint32_t phytium_dp_hw_get_backlight(struct phytium_panel *panel)
{
	uint32_t ret;
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	ret = phytium_dp->funcs->dp_hw_get_backlight(phytium_dp);

	return ret;
}

static void phytium_dp_hw_enable_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	if (!panel->phytium_bl_info.aux_set && panel->phytium_bl_info.pwm_set) {
		phytium_dp->funcs->dp_hw_set_backlight(phytium_dp, phytium_dp->panel.level);
	}
	phytium_dp->funcs->dp_hw_enable_backlight(phytium_dp);
}

static void phytium_dp_hw_disable_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	phytium_dp->funcs->dp_hw_disable_backlight(phytium_dp);
}

static void phytium_dp_hw_setup_backlight(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);
	struct drm_device *dev = phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct phytium_bl_pwm_chip *pwm_chip = &panel->pwm_chip;
	uint32_t group_offset = priv->edp_pwm_base;
	uint32_t reg;

	FTD330_LOG_TRACE;
	panel->max = priv->info.backlight_max = 99;
	panel->min = priv->info.backlight_min = 1;

	if (pwm_chip->state_pm.enabled) {
		/* PWM is already enabled, use existing settings */
		panel->level = pwm_get_relative_duty_cycle(&pwm_chip->state_pm, 100);
		panel->backlight_enabled = true;

		DRM_DEBUG_KMS("PWM already enabled at freq %ld, level %d\n",
				NSEC_PER_SEC / (unsigned long)pwm_chip->state_pm.period,
				panel->level);
	} else {
		/*set up PWM*/
		pwm_phytium_init(priv, pwm_chip);

		pwm_set_relative_duty_cycle(&pwm_chip->state_pm, 50, 100);
#if DEBUG_LOG
#if KERNEL_VERSION(5, 4, 18) >= LINUX_VERSION_CODE
        pr_info("Set duty: %u ns\n", pwm_chip->state_pm.duty_cycle);
#else
        pr_info("Set duty: %lld ns\n", pwm_chip->state_pm.duty_cycle);
#endif
#endif
		if (pwm_chip->state.dutymod == true) {
			reg = phytium_readl_reg(priv, group_offset, PWM_STATE);
			if ((reg & 0x8) != 0x8)
				pwm_phytium_set_duty(priv, pwm_chip, pwm_chip->state_pm.duty_cycle);
		} else {
			pwm_phytium_set_duty(priv, pwm_chip, pwm_chip->state_pm.duty_cycle);
		}

		panel->level = phytium_dp_hw_get_backlight(panel);
	}
}

void phytium_dp_panel_init_backlight_funcs(struct phytium_dp_device *phytium_dp)
{
	struct phytium_edp_backlight_info *phytium_bl_info = &phytium_dp->panel.phytium_bl_info;

	if (phytium_dp->edp_dpcd[1] & DP_EDP_TCON_BACKLIGHT_ADJUSTMENT_CAP &&
	   (phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_AUX_SET_CAP) &&
	   !(phytium_dp->edp_dpcd[2] & DP_EDP_BACKLIGHT_BRIGHTNESS_PWM_PIN_CAP)) {
		DRM_DEBUG_KMS("AUX Backlight Control Supported!\n");
		phytium_dp->panel.setup_backlight = phytium_dp_aux_setup_backlight;
		phytium_dp->panel.set_backlight = phytium_dp_aux_set_backlight;
		phytium_dp->panel.get_backlight = phytium_dp_aux_get_backlight;
	} else {
		DRM_DEBUG_KMS("PWM Backlight Control Supported!\n");
		phytium_dp->panel.setup_backlight = phytium_dp_hw_setup_backlight;
		phytium_dp->panel.set_backlight = phytium_dp_hw_set_backlight;
		phytium_dp->panel.get_backlight = phytium_dp_hw_get_backlight;
	}

	if (phytium_bl_info->aux_enable) {
		DRM_DEBUG_KMS("Using AUX to enable/disable backlight\n");
		phytium_dp->panel.enable_backlight = phytium_dp_aux_enable_backlight;
		phytium_dp->panel.disable_backlight = phytium_dp_aux_disable_backlight;
	} else {
		DRM_DEBUG_KMS("Using gpio to enable/disable backlight\n");
		phytium_dp->panel.enable_backlight = phytium_dp_hw_enable_backlight;
		phytium_dp->panel.disable_backlight = phytium_dp_hw_disable_backlight;
	}
}

static void set_edp_panel_time(struct edp_panel_timing *panel_time,
							struct edp_bios_panel_timing *edp_bios_panel_time,
							int port, bool set_from_drv) {
	if (set_from_drv) {
		panel_time->panel_power_up_delay =
									edp_drv_panel_time.drv_panel_power_up_delay;
		panel_time->backlight_on_delay =
									edp_drv_panel_time.drv_backlight_on_delay;
		panel_time->backlight_off_delay =
									edp_drv_panel_time.drv_backlight_off_delay;
		panel_time->panel_power_down_delay =
									edp_drv_panel_time.drv_panel_power_down_delay;
		panel_time->panel_power_cycle_delay =
									edp_drv_panel_time.drv_panel_power_cycle_delay;
		pr_info("port %d uses drv panel time parameter\n", port);
	} else {
		panel_time->panel_power_up_delay =
									edp_bios_panel_time->panel_power_up_delay[port][0];
		panel_time->backlight_on_delay =
									edp_bios_panel_time->backlight_on_delay[port][0];
		panel_time->backlight_off_delay =
									edp_bios_panel_time->backlight_off_delay[port][0];
		panel_time->panel_power_down_delay =
									edp_bios_panel_time->panel_power_down_delay[port][0];
		panel_time->panel_power_cycle_delay =
									edp_bios_panel_time->panel_power_cycle_delay[port][0];
		pr_info("port %d uses acpi panel time parameter\n", port);
	}
}

void phytium_dp_panel_init_panel_power_funcs(struct phytium_dp_device *phytium_dp)
{
	struct drm_device *dev =  phytium_dp->dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	int port = phytium_dp->port;
	struct bios_table_info *bios_info = &priv->info.bios_info;
	struct edp_panel_timing *panel_time = &phytium_dp->panel.edp_panel_time;

	phytium_dp->panel.poweron = phytium_dp_hw_poweron_panel;
	phytium_dp->panel.poweroff = phytium_dp_hw_poweroff_panel;

	phytium_dp->panel.enable_backlight = phytium_dp_hw_enable_backlight;
	phytium_dp->panel.disable_backlight = phytium_dp_hw_disable_backlight;

	mutex_init(&phytium_dp->panel.panel_lock);
	phytium_dp->panel.dev = phytium_dp->dev;

	if (port == 0 && !bios_info->edp0_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, NULL, port, true);
	} else if (port == 0 && bios_info->edp0_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, &bios_info->edp_bios_panel_time,
										port, false);
	}
	if (port == 1 && !bios_info->edp1_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, NULL, port, true);
	} else if (port == 1 && bios_info->edp1_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, &bios_info->edp_bios_panel_time,
										port, false);
	}
	if (port == 2 && !bios_info->edp2_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, NULL, port, true);
	} else if (port == 2 && bios_info->edp2_bios_panel_time_valied) {
		set_edp_panel_time(panel_time, &bios_info->edp_bios_panel_time,
										port, false);
	}

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
	FTD330_LOG_TRACE;
	if (panel->enable_backlight) {
		mutex_lock(&panel->panel_lock);
		msleep(panel->edp_panel_time.backlight_on_delay);
		panel->enable_backlight(panel);
		panel->backlight_enabled = true;
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_disable_backlight(struct phytium_panel *panel)
{
	FTD330_LOG_TRACE;
	if (panel->disable_backlight) {
		mutex_lock(&panel->panel_lock);
		panel->disable_backlight(panel);
		panel->backlight_enabled = false;
		msleep(panel->edp_panel_time.backlight_off_delay);
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_poweron(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	if (panel->poweron) {
		mutex_lock(&panel->panel_lock);
		if (!ftd330_dp_hw_panel_is_poweron(phytium_dp)) {
			panel->poweron(panel);
			panel->power_enabled = true;
			msleep(panel->edp_panel_time.panel_power_up_delay);
		} else {
			panel->power_enabled = true;
		}
		mutex_unlock(&panel->panel_lock);
	}
}

void phytium_panel_poweroff(struct phytium_panel *panel)
{
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);

	FTD330_LOG_TRACE;
	if (panel->poweroff) {
		mutex_lock(&panel->panel_lock);
		if (ftd330_dp_hw_panel_is_poweron(phytium_dp)) {
			panel->poweroff(panel);
			msleep(panel->edp_panel_time.panel_power_down_delay);
			panel->power_enabled = false;
		} else {
			panel->power_enabled = false;
		}
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
	struct phytium_dp_device *phytium_dp = panel_to_dp_device(panel);
	struct drm_device *dev = panel->dev;
	uint32_t hw_level = 0;
	int ret = 0;
#if BL_DEBUG
	pr_info("updating phytium_backlight, brightness=%d/%d\n",
		       bd->props.brightness, bd->props.max_brightness);
#endif
	drm_modeset_lock(&dev->mode_config.connection_mutex, NULL);
	hw_level = phytium_scale_user_to_hw(panel, bd->props.brightness, bd->props.max_brightness);
	if (!panel->phytium_bl_info.aux_set && panel->phytium_bl_info.pwm_set) {
#if MAP_BL
		panel->level = phytium_dp->edp_bl_map.edp_pwm_to_bright[hw_level];
#else
		panel->level = hw_level;
#endif
	}
	else {
		panel->level = hw_level;
	}
#if BL_DEBUG
	pr_info("Update status get hw level = %d, panel level = %d\n", hw_level, panel->level);
#endif
	if ((panel->set_backlight) && (panel->backlight_enabled)) {
		mutex_lock(&panel->panel_lock);
		ret = panel->set_backlight(panel, panel->level);
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
	if (phytium_dp->panel.phytium_bl_info.max)
		props.max_brightness = phytium_dp->panel.phytium_bl_info.max;
	else
		props.max_brightness = PHYTIUM_MAX_8BIT_BL_LEVEL;
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
