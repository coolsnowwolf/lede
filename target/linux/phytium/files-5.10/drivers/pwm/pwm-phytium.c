// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium PWM driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/export.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/acpi.h>

#define REG_TCNT		0x00
#define REG_TCTRL		0x04
#define REG_STAT		0x08

#define REG_TPERIOD		0x0c
#define REG_PWMCTRL		0x10
#define REG_PWMCCR		0x14

#define TCTRL_DIV_MASK		0x1ff8
#define TCTRL_PWMMOD_MASK	0x4
#define TCTRL_CAPMOD_MASK	0x3
#define PWM_PERIOD_MASK		0xffff
#define PWM_DUTY_MASK		0xffff
#define PWM_MODE_MASK		0x4
#define PWM_CTRL_INIT		0xc4

#define PWM_NUM  2

#define REG_DBCTRL		0x00
#define REG_DBCLY		0x04
#define PWM_UPDBCLY_MASK	0x3ff
#define PWM_DWDBCLY_MASK	0xffc00
#define PWM_DB_POLARITY_MASK	0xc

#define PWM_N(x)		((0x400)*(x))
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

struct phytium_pwm_param {
	int cntmod;
	int dutymod;
	unsigned int div;
	unsigned int updbcly;
	unsigned int dwdbcly;
	unsigned int dbpolarity;
};

struct phytium_pwm_variant {
	u8 rst_mask;
	u8 div;
	int counter_mode;
	int periodns;
	int duty_ns;
	int pwm_mode;
	u8 duty_mode;
	int updbcly;
	int dwdbcly;
};

struct phytium_pwm_channel {
	u32 period_ns;
	u32 duty_ns;
	u32 tin_ns;
};

struct phytium_pwm_chip {
	struct pwm_chip chip;
	struct pwm_state state_pm[PWM_NUM];
	struct phytium_pwm_variant variant;
	struct phytium_pwm_state state;
	u8 inverter_mask;
	u8 disabled_mask;
	int db_init;
	void __iomem *base;
	void __iomem *base1;
	struct phytium_pwm_param parameter[MAX_PARAMETER];
	unsigned int num_parameters;

	unsigned long clk_rate;
	struct clk *base_clk;
};

static inline struct phytium_pwm_chip *to_phytium_pwm_chip(struct pwm_chip *chip)
{
	return container_of(chip, struct phytium_pwm_chip, chip);
}


static void pwm_phytium_free(struct pwm_chip *chip, struct pwm_device *pwm)
{
	devm_kfree(chip->dev, pwm_get_chip_data(pwm));
	pwm_set_chip_data(pwm, NULL);
}

static int pwm_phytium_enable(struct pwm_chip *chip, struct pwm_device *pwm, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base + PWM_N(n) + REG_TCTRL);
	reg |= 0x2;
	our_chip->state_pm[n].enabled = 1;
	writel(reg, our_chip->base + PWM_N(n) + REG_TCTRL);

	return 0;
}

static void pwm_phytium_disable(struct pwm_chip *chip, struct pwm_device *pwm, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base + PWM_N(n) + REG_TCTRL);
	reg &= 0xfffffffd;
	our_chip->state_pm[n].enabled = 0;
	writel(reg, our_chip->base + PWM_N(n) + REG_TCTRL);
}

static void pwm_phytium_dutymod(struct pwm_chip *chip, int dutymod, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base + PWM_N(n) +  REG_PWMCTRL);

	if (dutymod == 0)
		reg &= 0xfffffeff;
	else if (dutymod == 1)
		reg |= 0x100;

	writel(reg, our_chip->base + PWM_N(n) +  REG_PWMCTRL);
}

static void pwm_phytium_set_div(struct pwm_chip *chip, unsigned int div, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base + PWM_N(n) + REG_TCTRL);
	reg &= 0xffff;
	reg |= (div<<16);
	writel(reg, our_chip->base + PWM_N(n) + REG_TCTRL);
}

static void pwm_phytium_set_tmode(struct pwm_chip *chip, int tmode, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base + PWM_N(n) + REG_TCTRL);
	if (tmode == 0)
		reg &= 0xfffffffb;
	else if (tmode == 1)
		reg |= 0x4;

	writel(reg, our_chip->base + PWM_N(n) + REG_TCTRL);
}

static void pwm_phytium_set_periodns(struct pwm_chip *chip, unsigned int periodns, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;
	int div = our_chip->state.div;
	u64 cycles;

	cycles = our_chip->clk_rate;
	cycles *= (periodns / (div + 1));
	do_div(cycles, NSEC_PER_SEC);

	reg = readl(our_chip->base + PWM_N(n) + REG_TPERIOD);
	cycles = (cycles & PWM_PERIOD_MASK) - 0x1;
	our_chip->state_pm[n].period = cycles;

	writel(cycles, our_chip->base + PWM_N(n) + REG_TPERIOD);
}

static void pwm_phytium_set_duty(struct pwm_chip *chip, unsigned int duty, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;
	int div = our_chip->state.div;
	u64 cycles;

	cycles = our_chip->clk_rate;
	cycles *= (duty / (div + 1));
	do_div(cycles, NSEC_PER_SEC);

	reg = readl(our_chip->base + PWM_N(n) + REG_PWMCCR);
	cycles = (cycles & PWM_DUTY_MASK) - 0x1;
	our_chip->state_pm[n].duty_cycle = cycles;

	writel(cycles, our_chip->base + PWM_N(n) + REG_PWMCCR);
}

static int pwm_phytium_set_dbcly(struct pwm_chip *chip, unsigned int updbcly, unsigned int dwdbcly)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;
	u64 dbcly, cycles, upcycles, dwcycles;

	reg = readl(our_chip->base + REG_TPERIOD);
	cycles = our_chip->clk_rate;
	dbcly &= 0x0;
	if (updbcly) {
		upcycles = cycles * updbcly;
		do_div(upcycles, NSEC_PER_SEC);

		if (upcycles < reg)
			dbcly |= (upcycles & PWM_UPDBCLY_MASK);
		else
			return -EINVAL;
	}

	if (dwdbcly) {
		dwcycles = cycles * dwdbcly;
		do_div(dwcycles, NSEC_PER_SEC);

		if (dwcycles < reg)
			dbcly |= ((dwcycles << 10) & PWM_DWDBCLY_MASK);
		else
			return -EINVAL;
	}

	writel(dbcly, our_chip->base1 + REG_DBCLY);

	reg = readl(our_chip->base1 + REG_DBCTRL);
	reg |= 0x30;
	writel(reg, our_chip->base1 + REG_DBCTRL);

	return 0;
}

static void pwm_phytium_set_dbpolarity(struct pwm_chip *chip, unsigned int db_polarity)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 reg;

	reg = readl(our_chip->base1 + REG_DBCTRL);
	reg &= 0x33;
	reg |= ((db_polarity<<2) & PWM_DB_POLARITY_MASK);
	writel(reg, our_chip->base1 + REG_DBCTRL);
}

static int pwm_phytium_init(struct pwm_chip *chip, struct pwm_device *pwm, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);

	writel(PWM_CTRL_INIT, our_chip->base + PWM_N(n) + REG_PWMCTRL);

	pwm_phytium_dutymod(chip, our_chip->state.dutymod, n);
	pwm_phytium_set_div(chip, our_chip->state.div, n);
	pwm_phytium_set_tmode(chip, our_chip->state.cntmod, n);

	return 0;
}

static int pwm_phytium_db_init(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);

	pwm_phytium_set_dbcly(chip, our_chip->state.updbcly, our_chip->state.dwdbcly);
	pwm_phytium_set_dbpolarity(chip, our_chip->state.dbpolarity);

	return 0;
}

static int __pwm_phytium_config(struct pwm_chip *chip, struct pwm_device *pwm)
{
	pwm_phytium_init(chip, pwm, 0);
	pwm_phytium_init(chip, pwm, 1);
	return 0;
}

static int pwm_phytium_set_polarity(struct pwm_chip *chip, enum pwm_polarity polarity, int n)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	u32 value;

	value = readl(our_chip->base + PWM_N(n) + REG_PWMCTRL);

	if (polarity == PWM_POLARITY_INVERSED) {
		value &= 0xffffff0f;
		value |= 0x30;
	} else if (polarity == PWM_POLARITY_NORMAL) {
		value &= 0xffffff0f;
		value |= 0x40;
	}

	our_chip->state_pm[n].polarity = polarity;
	writel(value, our_chip->base + PWM_N(n) + REG_PWMCTRL);

	return 0;
}

static int pwm_phytium_apply(struct pwm_chip *chip, struct pwm_device *pwm,
			const struct pwm_state *state)
{
	struct phytium_pwm_chip *phytium_pwm = to_phytium_pwm_chip(chip);
	struct pwm_state cstate;
	u32 reg;
	int n;

	pwm_get_state(pwm, &cstate);

	n = pwm->hwpwm & BIT(0);

	if ((state->polarity != cstate.polarity) && !state->enabled)
		pwm_phytium_set_polarity(chip, state->polarity, n);

	if (state->enabled && !cstate.enabled)
		pwm_phytium_enable(chip, pwm, n);

	if (!state->enabled && cstate.enabled)
		pwm_phytium_disable(chip, pwm, n);

	if (state->period != cstate.period) {
		pwm_phytium_set_periodns(chip, state->period, n);
		if ((phytium_pwm->db_init == 1) && (n == 0))
			pwm_phytium_db_init(chip, pwm);
	}

	if (state->duty_cycle != cstate.duty_cycle) {
		if (phytium_pwm->state.dutymod == true) {
			reg = readl(phytium_pwm->base + PWM_N(n) + REG_STAT);
			if ((reg & 0x8) != 0x8)
				pwm_phytium_set_duty(chip, state->duty_cycle, n);
		} else {
			pwm_phytium_set_duty(chip, state->duty_cycle, n);
		}
	}

	return 0;
}

static int pwm_phytium_request(struct pwm_chip *chip, struct pwm_device *pwm)
{
	struct phytium_pwm_chip *our_chip = to_phytium_pwm_chip(chip);
	struct phytium_pwm_channel *our_chan;

	our_chan = devm_kzalloc(chip->dev, sizeof(*our_chan), GFP_KERNEL);
	if (!our_chan)
		return -ENOMEM;

	pwm_set_chip_data(pwm, our_chan);
	__pwm_phytium_config(&our_chip->chip, our_chip->chip.pwms);

	return 0;
}

static const struct pwm_ops pwm_phytium_ops = {
	.request = pwm_phytium_request,
	.free    = pwm_phytium_free,
	.apply   = pwm_phytium_apply,
	.owner   = THIS_MODULE,
};

static int phytium_pwm_set_parameter(struct phytium_pwm_chip *priv)
{
	unsigned int i;

	for (i = 0; i < priv->num_parameters; i++) {
		if (priv->parameter[i].updbcly > 0 || priv->parameter[i].dwdbcly > 0) {
			priv->db_init = 1;
			priv->state.db_rst = 1;
		}

		priv->state.cntmod = priv->parameter[i].cntmod;
		priv->state.dutymod = priv->parameter[i].dutymod;
		priv->state.div = priv->parameter[i].div;
		priv->state.updbcly = priv->parameter[i].updbcly;
		priv->state.dwdbcly = priv->parameter[i].dwdbcly;
		priv->state.dbpolarity = priv->parameter[i].dbpolarity;
	}
	priv->state.rst = 1;

	return 0;
}

static int pwm_phytium_probe_parameter(struct phytium_pwm_chip *priv,
				       struct fwnode_handle *np)
{
	int nb, ret, array_size;
	unsigned int i;

	array_size = fwnode_property_read_u32_array(np, "phytium,db", NULL, 0);
	nb = array_size / (sizeof(struct phytium_pwm_param) / sizeof(u32));
	if (nb <= 0 || nb > MAX_PARAMETER)
		return -EINVAL;

	priv->num_parameters = nb;
	ret = fwnode_property_read_u32_array(np, "phytium,db",
					 (u32 *)priv->parameter, array_size);
	if (ret)
		return ret;

	for (i = 0; i < priv->num_parameters; i++) {
		if (priv->parameter[i].cntmod > 1 ||
			priv->parameter[i].dutymod > 1 ||
			priv->parameter[i].div > 4096 ||
			priv->parameter[i].dbpolarity > 3)
			return -EINVAL;
	}

	return phytium_pwm_set_parameter(priv);
}
static int pwm_phytium_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fwnode_handle *np = dev_fwnode(dev);
	struct phytium_pwm_chip *chip;
	struct resource *res;
	int ret;

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);

	if (chip == NULL)
		return -ENOMEM;

	chip->chip.dev = &pdev->dev;
	chip->chip.ops = &pwm_phytium_ops;
	chip->chip.base = -1;
	chip->chip.npwm = PWM_NUM;
	chip->inverter_mask = BIT(PWM_NUM) - 1;

	if (dev->of_node) {
		chip->chip.of_xlate = of_pwm_xlate_with_flags;
		chip->chip.of_pwm_n_cells = 3;
	}
	ret = pwm_phytium_probe_parameter(chip, np);
	if (ret) {
		dev_err(dev, "failed to set parameter\n");
		return ret;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	chip->base1 = devm_ioremap_resource(&pdev->dev, res);
	chip->base = (chip->base1 + 0x400);

	if (IS_ERR(chip->base)) {
		dev_err(dev, "failed to get base_addr\n");
		return PTR_ERR(chip->base);
	}
	if (dev->of_node) {
	chip->base_clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(chip->base_clk)) {
		dev_err(dev, "failed to get clk\n");
		return PTR_ERR(chip->base_clk);
	}

	ret = clk_prepare_enable(chip->base_clk);
	if (ret < 0) {
		dev_err(dev, "failed to enable clk\n");
		return ret;
	}
	chip->clk_rate = clk_get_rate(chip->base_clk);
	} else if (has_acpi_companion(dev)){
               if(fwnode_property_read_u32(dev_fwnode(dev),"clock-frequency", (u32 *)&(chip->clk_rate) ) <0)
                       chip->clk_rate = 50000000;
	}
	platform_set_drvdata(pdev, chip);

	ret = pwmchip_add(&chip->chip);

	if (ret < 0) {
		dev_err(dev, "failed to register PWM chip\n");
		return ret;
	}

	return 0;
}

static int pwm_phytium_remove(struct platform_device *pdev)
{
	struct phytium_pwm_chip *chip = platform_get_drvdata(pdev);

	pwmchip_remove(&chip->chip);

	clk_disable_unprepare(chip->base_clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int pwm_phytium_pm_init(struct phytium_pwm_chip *priv)
{
	int i;

	__pwm_phytium_config(&priv->chip, priv->chip.pwms);
	for (i = 0; i < priv->chip.npwm; i++) {
		writel(priv->state_pm[i].period, priv->base + PWM_N(i) + REG_TPERIOD);
		if ((priv->db_init == 1) && (i == 0))
			pwm_phytium_db_init(&priv->chip, priv->chip.pwms);
		writel(priv->state_pm[i].duty_cycle, priv->base + PWM_N(i) +  REG_PWMCTRL);
		pwm_phytium_set_polarity(&priv->chip, priv->state_pm[i].polarity, i);
		if (priv->state_pm[i].enabled)
			pwm_phytium_enable(&priv->chip, priv->chip.pwms, i);
	}

	return 0;
}

static int pwm_phytium_suspend(struct device *dev)
{
	return 0;
}

static int pwm_phytium_resume(struct device *dev)
{
	struct phytium_pwm_chip *priv = dev_get_drvdata(dev);

	pwm_phytium_pm_init(priv);
	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_pwm_dev_pm_ops, pwm_phytium_suspend, pwm_phytium_resume);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_pwm_acpi_ids[] = {
       { "PHYT0029", 0 },
       { /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_pwm_acpi_ids);
#endif

static const struct of_device_id phytium_pwm_matches[] = {
	{ .compatible = "phytium,pwm" },
	{},
};
MODULE_DEVICE_TABLE(of, phytium_pwm_matches);

static struct platform_driver pwm_phytium_driver = {
	.driver = {
		.name = "phytium-pwm",
		.pm = &phytium_pwm_dev_pm_ops,
		.of_match_table = phytium_pwm_matches,
		.acpi_match_table = ACPI_PTR(phytium_pwm_acpi_ids),
	},
	.probe = pwm_phytium_probe,
	.remove = pwm_phytium_remove,
};
module_platform_driver(pwm_phytium_driver);

MODULE_DESCRIPTION("Phytium SoC PWM driver");
MODULE_AUTHOR("Yang Liu <yangliu2021@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
