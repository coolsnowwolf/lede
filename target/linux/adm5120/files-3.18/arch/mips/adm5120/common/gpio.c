/*
 *  ADM5120 generic GPIO API support via GPIOLIB
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <asm/addrspace.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_switch.h>

#define GPIO_REG(r)	(void __iomem *)(KSEG1ADDR(ADM5120_SWITCH_BASE) + r)

struct gpio1_desc {
	void __iomem	*reg;		/* register address */
	u8		iv_shift;	/* shift amount for input bit */
	u8		mode_shift;	/* shift amount for mode bits */
};

#define GPIO1_DESC(p, l) {						\
		.reg = GPIO_REG(SWITCH_REG_PORT0_LED + ((p) * 4)),	\
		.iv_shift = LED0_IV_SHIFT + (l),			\
		.mode_shift = (l) * 4					\
	}

static struct gpio1_desc gpio1_table[15] = {
	GPIO1_DESC(0, 0), GPIO1_DESC(0, 1), GPIO1_DESC(0, 2),
	GPIO1_DESC(1, 0), GPIO1_DESC(1, 1), GPIO1_DESC(1, 2),
	GPIO1_DESC(2, 0), GPIO1_DESC(2, 1), GPIO1_DESC(2, 2),
	GPIO1_DESC(3, 0), GPIO1_DESC(3, 1), GPIO1_DESC(3, 2),
	GPIO1_DESC(4, 0), GPIO1_DESC(4, 1), GPIO1_DESC(4, 2)
};

static u32 gpio_conf2;

int adm5120_gpio_to_irq(unsigned gpio)
{
	int ret;

	switch (gpio) {
	case ADM5120_GPIO_PIN2:
		ret = ADM5120_IRQ_GPIO2;
		break;
	case ADM5120_GPIO_PIN4:
		ret = ADM5120_IRQ_GPIO4;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(adm5120_gpio_to_irq);

int adm5120_irq_to_gpio(unsigned irq)
{
	int ret;

	switch (irq) {
	case ADM5120_IRQ_GPIO2:
		ret = ADM5120_GPIO_PIN2;
		break;
	case ADM5120_IRQ_GPIO4:
		ret = ADM5120_GPIO_PIN4;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(adm5120_irq_to_gpio);

/*
 * Helpers for GPIO lines in GPIO_CONF0 register
 */
#define PIN_IM(p)	((1 << GPIO_CONF0_IM_SHIFT) << p)
#define PIN_IV(p)	((1 << GPIO_CONF0_IV_SHIFT) << p)
#define PIN_OE(p)	((1 << GPIO_CONF0_OE_SHIFT) << p)
#define PIN_OV(p)	((1 << GPIO_CONF0_OV_SHIFT) << p)

int __adm5120_gpio0_get_value(unsigned offset)
{
	void __iomem **reg;
	u32 t;

	reg = GPIO_REG(SWITCH_REG_GPIO_CONF0);

	t = __raw_readl(reg);
	if ((t & PIN_IM(offset)) != 0)
		t &= PIN_IV(offset);
	else
		t &= PIN_OV(offset);

	return (t) ? 1 : 0;
}
EXPORT_SYMBOL(__adm5120_gpio0_get_value);

void __adm5120_gpio0_set_value(unsigned offset, int value)
{
	void __iomem **reg;
	u32 t;

	reg = GPIO_REG(SWITCH_REG_GPIO_CONF0);

	t = __raw_readl(reg);
	if (value == 0)
		t &= ~(PIN_OV(offset));
	else
		t |= PIN_OV(offset);

	__raw_writel(t, reg);
}
EXPORT_SYMBOL(__adm5120_gpio0_set_value);

static int adm5120_gpio0_get_value(struct gpio_chip *chip, unsigned offset)
{
	return __adm5120_gpio0_get_value(offset);
}

static void adm5120_gpio0_set_value(struct gpio_chip *chip,
				    unsigned offset, int value)
{
	__adm5120_gpio0_set_value(offset, value);
}

static int adm5120_gpio0_direction_input(struct gpio_chip *chip,
					 unsigned offset)
{
	void __iomem **reg;
	u32 t;

	reg = GPIO_REG(SWITCH_REG_GPIO_CONF0);

	t = __raw_readl(reg);
	t &= ~(PIN_OE(offset));
	t |= PIN_IM(offset);
	__raw_writel(t, reg);

	return 0;
}

static int adm5120_gpio0_direction_output(struct gpio_chip *chip,
					  unsigned offset, int value)
{
	void __iomem **reg;
	u32 t;

	reg = GPIO_REG(SWITCH_REG_GPIO_CONF0);

	t = __raw_readl(reg);
	t &= ~(PIN_IM(offset) | PIN_OV(offset));
	t |= PIN_OE(offset);

	if (value)
		t |= PIN_OV(offset);

	__raw_writel(t, reg);

	return 0;
}

static struct gpio_chip adm5120_gpio0_chip = {
	.label			= "adm5120 gpio0",
	.get			= adm5120_gpio0_get_value,
	.set			= adm5120_gpio0_set_value,
	.direction_input	= adm5120_gpio0_direction_input,
	.direction_output	= adm5120_gpio0_direction_output,
	.base			= ADM5120_GPIO_PIN0,
	.ngpio			= ADM5120_GPIO_PIN7 - ADM5120_GPIO_PIN0 + 1,
};

int __adm5120_gpio1_get_value(unsigned offset)
{
	void __iomem **reg;
	u32 t, m;

	reg = gpio1_table[offset].reg;

	t = __raw_readl(reg);
	m = (t >> gpio1_table[offset].mode_shift) & LED_MODE_MASK;
	if (m == LED_MODE_INPUT)
		return (t >> gpio1_table[offset].iv_shift) & 1;

	if (m == LED_MODE_OUT_LOW)
		return 0;

	return 1;
}
EXPORT_SYMBOL(__adm5120_gpio1_get_value);

void __adm5120_gpio1_set_value(unsigned offset, int value)
{
	void __iomem **reg;
	u32 t, s;

	reg = gpio1_table[offset].reg;
	s = gpio1_table[offset].mode_shift;

	t = __raw_readl(reg);
	t &= ~(LED_MODE_MASK << s);

	switch (value) {
	case ADM5120_GPIO_LOW:
		t |= (LED_MODE_OUT_LOW << s);
		break;
	case ADM5120_GPIO_FLASH:
	case ADM5120_GPIO_LINK:
	case ADM5120_GPIO_SPEED:
	case ADM5120_GPIO_DUPLEX:
	case ADM5120_GPIO_ACT:
	case ADM5120_GPIO_COLL:
	case ADM5120_GPIO_LINK_ACT:
	case ADM5120_GPIO_DUPLEX_COLL:
	case ADM5120_GPIO_10M_ACT:
	case ADM5120_GPIO_100M_ACT:
		t |= ((value & LED_MODE_MASK) << s);
		break;
	default:
		t |= (LED_MODE_OUT_HIGH << s);
		break;
	}

	__raw_writel(t, reg);
}
EXPORT_SYMBOL(__adm5120_gpio1_set_value);

static int adm5120_gpio1_get_value(struct gpio_chip *chip, unsigned offset)
{
	return __adm5120_gpio1_get_value(offset);
}

static void adm5120_gpio1_set_value(struct gpio_chip *chip,
				    unsigned offset, int value)
{
	__adm5120_gpio1_set_value(offset, value);
}

static int adm5120_gpio1_direction_input(struct gpio_chip *chip,
					 unsigned offset)
{
	void __iomem **reg;
	u32 t;

	reg = gpio1_table[offset].reg;
	t = __raw_readl(reg);
	t &= ~(LED_MODE_MASK << gpio1_table[offset].mode_shift);
	__raw_writel(t, reg);

	return 0;
}

static int adm5120_gpio1_direction_output(struct gpio_chip *chip,
					  unsigned offset, int value)
{
	__adm5120_gpio1_set_value(offset, value);
	return 0;
}

static struct gpio_chip adm5120_gpio1_chip = {
	.label			= "adm5120 gpio1",
	.get			= adm5120_gpio1_get_value,
	.set			= adm5120_gpio1_set_value,
	.direction_input	= adm5120_gpio1_direction_input,
	.direction_output	= adm5120_gpio1_direction_output,
	.base			= ADM5120_GPIO_P0L0,
	.ngpio			= ADM5120_GPIO_P4L2 - ADM5120_GPIO_P0L0 + 1,
};

void __init adm5120_gpio_csx0_enable(void)
{
	gpio_conf2 |= GPIO_CONF2_CSX0;
	SW_WRITE_REG(SWITCH_REG_GPIO_CONF2, gpio_conf2);

	gpio_request(ADM5120_GPIO_PIN1, "CSX0");
}

void __init adm5120_gpio_csx1_enable(void)
{
	gpio_conf2 |= GPIO_CONF2_CSX1;
	SW_WRITE_REG(SWITCH_REG_GPIO_CONF2, gpio_conf2);

	gpio_request(ADM5120_GPIO_PIN3, "CSX1");
}

void __init adm5120_gpio_ew_enable(void)
{
	gpio_conf2 |= GPIO_CONF2_EW;
	SW_WRITE_REG(SWITCH_REG_GPIO_CONF2, gpio_conf2);

	gpio_request(ADM5120_GPIO_PIN0, "EW");
}

void __init adm5120_gpio_init(void)
{
	int err;

	SW_WRITE_REG(SWITCH_REG_GPIO_CONF2, gpio_conf2);

	if (adm5120_package_pqfp())
		adm5120_gpio0_chip.ngpio = 4;

	err = gpiochip_add(&adm5120_gpio0_chip);
	if (err)
		panic("cannot add ADM5120 GPIO0 chip, error=%d", err);

	err = gpiochip_add(&adm5120_gpio1_chip);
	if (err)
		panic("cannot add ADM5120 GPIO1 chip, error=%d", err);

}
