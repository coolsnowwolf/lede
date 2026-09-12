/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_EDP_PWM_H__
#define __PHYTIUM_EDP_PWM_H__

#include <linux/pwm.h>
#include "phytium_dp.h"

#define MAX_PARAMETER 2

struct phytium_pwm_state {
	int rst;
	int cntmod;
	int dutymod;
	unsigned int div;
	int db_rst;
	unsigned int updbcly;
	unsigned int dwdbcly;
	unsigned int dbpolarity;
};

struct phytium_bl_pwm_chip {
	struct pwm_state state_pm;
	struct phytium_pwm_state state;
	unsigned int num_parameters;
	int db_init;
	uint64_t set_periodns;
	uint64_t clk_rate_hz;
};

void pwm_phytium_init(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip);
int phytium_pwm_set_parameter(struct ftd330_drm_private *priv, struct phytium_dp_device *phytium_dp);
int phytium_pwm_init_from_acpi(struct ftd330_drm_private *priv, struct phytium_dp_device *phytium_dp);
int pwm_phytium_set_duty(struct ftd330_drm_private *priv,
					 struct phytium_bl_pwm_chip *pwm_chip,
					 unsigned int duty);
void pwm_phytium_disable(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip);
void pwm_phytium_enable(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip);
#endif /*__PHYTIUM_EDP_PWM_H__ */