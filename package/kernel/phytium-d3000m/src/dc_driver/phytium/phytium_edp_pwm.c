// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#include "phytium_dp.h"
#include "ftd330_dp.h"
#include "phytium_dp_reg.h"

int phytium_pwm_set_parameter(struct ftd330_drm_private *priv, struct phytium_dp_device *phytium_dp)
{
	struct phytium_bl_pwm_chip *pwm_chip;
	int ret = 0;

	pwm_chip = &phytium_dp->panel.pwm_chip;

	if (priv->info.pwm_periodns)
		pwm_chip->set_periodns = priv->info.pwm_periodns;
	else
		return -EINVAL;

	if (priv->info.pwm_div)
		pwm_chip->state.div = priv->info.pwm_div;
	else
		return -EINVAL;

	if (priv->info.pwm_clk_rate)
		pwm_chip->clk_rate_hz = priv->info.pwm_clk_rate;
	else
		return -EINVAL;
#if BL_DEBUG
	pr_info("set periodns: %lld\n", pwm_chip->set_periodns);
	pr_info("state div: %d\n", pwm_chip->state.div);
	pr_info("clk rate: %lld\n", pwm_chip->clk_rate_hz);
#endif

	return ret;
}

static int pwm_phytium_reset(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip)
{
	uint32_t reg;
	int timeout = 10;
	uint32_t group_offset = priv->edp_pwm_base;

	phytium_writel_reg(priv, 0x1, group_offset, PWM_TIM_CTRL);
	/* check reset complete */
	do {
		reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	} while ((reg & 0x1) && (0 <= --timeout));

	if (timeout < 0) {
		DRM_ERROR("%s:%d reset pwm timeout", __func__, __LINE__);
		return -1;
	}

	return 0;
}

void pwm_phytium_disable(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip)
{
	uint32_t group_offset = priv->edp_pwm_base;
	uint32_t reg;

	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	reg &= 0xfffffffd;
	pwm_chip->state_pm.enabled = 0;
	phytium_writel_reg(priv, reg, group_offset, PWM_TIM_CTRL);
#if BL_DEBUG
	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	pr_info("read PWM_TIM_CTRL: 0x%08x\n", reg);
#endif

}

static void
pwm_phytium_set_periodns(struct ftd330_drm_private *priv,
							struct phytium_bl_pwm_chip *pwm_chip,
							unsigned int periodns)
{
	int div = pwm_chip->state.div;
	uint64_t cycles = pwm_chip->clk_rate_hz;
	uint32_t group_offset = priv->edp_pwm_base;
#if BL_DEBUG
	uint32_t reg;
#endif
	cycles *= (periodns / (div + 1));
	do_div(cycles, NSEC_PER_SEC);

	cycles = (cycles & PWM_PERIOD_MASK) - 0x1;
	pwm_chip->state_pm.period = periodns;

	phytium_writel_reg(priv, cycles, group_offset, PWM_PERIOD);

#if BL_DEBUG
	reg = phytium_readl_reg(priv, group_offset, PWM_PERIOD);
	pr_info("write period: %lld\n", cycles);
	pr_info("read PWM_PERIOD: 0x%08x\n", reg);
#endif
}

int
pwm_phytium_set_duty(struct ftd330_drm_private *priv,
					 struct phytium_bl_pwm_chip *pwm_chip,
					 unsigned int duty)
{
	int div = pwm_chip->state.div;
	uint64_t cycles = pwm_chip->clk_rate_hz;
	uint32_t group_offset = priv->edp_pwm_base;
	int ret = 0;
	uint16_t pwm_period_ccr;
#if BL_DEBUG
	uint32_t reg;
#endif

	pwm_period_ccr = (uint16_t)phytium_readl_reg(priv, group_offset, PWM_PERIOD);

	cycles *= (duty / (div + 1));
	do_div(cycles, NSEC_PER_SEC);
	cycles = (cycles & PWM_DUTY_MASK) - 0x1;

	if (cycles > pwm_period_ccr) {
		DRM_DEBUG_KMS("%s cycles %lld > pwm_period_ccr %d\n", __func__, cycles, pwm_period_ccr);
		cycles = pwm_period_ccr;
	}
	if (cycles < 1) {
		DRM_DEBUG_KMS("%s set cycles %lld < min_value:1\n", __func__, cycles);
		cycles = 1;
	}

	phytium_writel_reg(priv, cycles, group_offset, PWM_CCR);

#if BL_DEBUG
	reg = phytium_readl_reg(priv, group_offset, PWM_CCR);
	pr_info("write duty: %lld\n", cycles);
	pr_info("read PWM_CCR: 0x%08x\n", reg);
#endif
	return ret;
}

void pwm_phytium_enable(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip)
{
	uint32_t reg;
	uint32_t group_offset = priv->edp_pwm_base;

	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	reg |= 0x2;
	pwm_chip->state_pm.enabled = 1;
	phytium_writel_reg(priv, reg, group_offset, PWM_TIM_CTRL);
#if BL_DEBUG
	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	pr_info("read PWM_TIM_CTRL: 0x%08x\n", reg);
#endif
}

void pwm_phytium_init(struct ftd330_drm_private *priv, struct phytium_bl_pwm_chip *pwm_chip)
{
	uint32_t reg;
	uint32_t group_offset = priv->edp_pwm_base;

	/*1. reset*/
	if (pwm_phytium_reset(priv, pwm_chip))
		return;

	/*2. disable */
	pwm_phytium_disable(priv, pwm_chip);

	/*3.set time mode*/
	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	if (pwm_chip->state.cntmod == 0) //bit[2] tim_ctrl Mode is modulo
		reg &= 0xfffffffb;
	else if (pwm_chip->state.cntmod == 1)
		reg |= 0x4;

	/*4. enable time interrupt*/
	reg |= 0x30; //bit[4],bit[5]

	/*5. set div*/
	reg &= 0xffff;
	reg |= (pwm_chip->state.div<<16);
	phytium_writel_reg(priv, reg, group_offset, PWM_TIM_CTRL);

	/*6. set period*/
	pwm_phytium_set_periodns(priv, pwm_chip, pwm_chip->set_periodns);

	/*7. set pwm mode*/
	reg = phytium_readl_reg(priv, group_offset, PWM_CTRL);
	reg |= 0x4; //bit[2], only support compare mode

	/*8. enable pwm interrupt*/
	reg |= 0x208; //bit[9],bit[3]

	/*9. pwm CMP mode*/
	reg |= 0x40; //CMP=100

	/*10. set duty mode*/
	if (pwm_chip->state.dutymod == 0) // bit[8] duty src from pwm ccr
		reg &= 0xfffffeff;
	else if (pwm_chip->state.dutymod == 1)
		reg |= 0x100;

	phytium_writel_reg(priv, reg, group_offset, PWM_CTRL);

#if BL_DEBUG
	reg = phytium_readl_reg(priv, group_offset, PWM_TIM_CTRL);
	pr_info("%s set PWM_TIM_CTRL: 0x%08x\n", __func__, reg);
	reg = phytium_readl_reg(priv, group_offset, PWM_CTRL);
	pr_info("%s set PWM_CTRL: 0x%08x\n", __func__, reg);
#endif
}
