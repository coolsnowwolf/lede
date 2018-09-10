/*
 * Infineon/ADMTek ADM8668 WildPass GPIO support
 *
 * Copyright (C) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under the terms of GPLv2.
 *
 */
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/io.h>

#include <adm8668.h>

#define GPIO_MASK	0x3f

#define GPIO_IN_OFS	0
#define GPIO_OUT_OFS	6
#define GPIO_OE_OFS	12

struct adm8668_gpio_chip {
	void __iomem	*base;
	struct gpio_chip chip;
};

static int adm8668_gpio_dir_out(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct adm8668_gpio_chip *c =
		container_of(chip, struct adm8668_gpio_chip, chip);
	u32 mask;

	/* clear input, set output enable and output value */
	mask = __raw_readl(c->base);
	mask &= ~(1 << offset);
	mask |= (1 << (offset + GPIO_OE_OFS));
	if (value)
		mask |= (1 << (offset + GPIO_OUT_OFS));
	else
		mask &= ~(1 << (offset + GPIO_OUT_OFS));
	__raw_writel(mask, c->base);

	return 0;
}

static int adm8668_gpio_dir_in(struct gpio_chip *chip,
				unsigned offset)
{
	struct adm8668_gpio_chip *c =
		container_of(chip, struct adm8668_gpio_chip, chip);
	u32 mask;

	mask = __raw_readl(c->base);
	mask &= ~(((1 << (offset + GPIO_OE_OFS)) | (1 << (offset + GPIO_OUT_OFS))));
	mask |= (1 << offset);
	__raw_writel(mask, c->base);

	return 0;
}

static void adm8668_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct adm8668_gpio_chip *c =
		container_of(chip, struct adm8668_gpio_chip, chip);
	u32 mask;

	mask = __raw_readl(c->base);
	if (value)
		mask |= (1 << (offset + GPIO_OUT_OFS));
	else
		mask &= ~(1 << (offset + GPIO_OUT_OFS));
	__raw_writel(mask, c->base);
}

static int adm8668_gpio_get(struct gpio_chip *chip,
				unsigned offset)
{
	struct adm8668_gpio_chip *c =
		container_of(chip, struct adm8668_gpio_chip, chip);
	u32 value;

	value = __raw_readl(c->base) & GPIO_MASK;

	return value & (1 << offset);
}

static struct adm8668_gpio_chip adm8668_gpio_cpu = {
	.base = (void __iomem *)KSEG1ADDR(ADM8668_CONFIG_BASE + CRGPIO_REG),
	.chip = {
		.label			= "adm8668-cpu-gpio",
		.direction_output	= adm8668_gpio_dir_out,
		.direction_input	= adm8668_gpio_dir_in,
		.set			= adm8668_gpio_set,
		.get			= adm8668_gpio_get,
		.ngpio			= 6,
	},
};

static struct adm8668_gpio_chip adm8668_gpio_wlan = {
	.base = (void __iomem *)KSEG1ADDR(ADM8668_WLAN_BASE + GPIO_REG),
	.chip = {
		.label			= "adm8668-wlan-gpio",
		.direction_output	= adm8668_gpio_dir_out,
		.direction_input	= adm8668_gpio_dir_in,
		.set			= adm8668_gpio_set,
		.get			= adm8668_gpio_get,
		.ngpio			= 6,
		.base			= 6,
	},
};

static int __init adm8668_gpio_init(void)
{
	int ret;

	ret = gpiochip_add(&adm8668_gpio_cpu.chip);
	if (ret)
		return ret;

	return gpiochip_add(&adm8668_gpio_wlan.chip);
}
arch_initcall(adm8668_gpio_init);
