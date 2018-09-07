/*
 *  ADM5120 GPIO wrappers for arch-neutral GPIO calls
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ASM_MIPS_MACH_ADM5120_GPIO_H
#define _ASM_MIPS_MACH_ADM5120_GPIO_H

#define ARCH_NR_GPIOS	64

#include <asm-generic/gpio.h>

#include <asm/mach-adm5120/adm5120_switch.h>

#define ADM5120_GPIO_PIN0	0
#define ADM5120_GPIO_PIN1	1
#define ADM5120_GPIO_PIN2	2
#define ADM5120_GPIO_PIN3	3
#define ADM5120_GPIO_PIN4	4
#define ADM5120_GPIO_PIN5	5
#define ADM5120_GPIO_PIN6	6
#define ADM5120_GPIO_PIN7	7
#define ADM5120_GPIO_P0L0	8
#define ADM5120_GPIO_P0L1	9
#define ADM5120_GPIO_P0L2	10
#define ADM5120_GPIO_P1L0	11
#define ADM5120_GPIO_P1L1	12
#define ADM5120_GPIO_P1L2	13
#define ADM5120_GPIO_P2L0	14
#define ADM5120_GPIO_P2L1	15
#define ADM5120_GPIO_P2L2	16
#define ADM5120_GPIO_P3L0	17
#define ADM5120_GPIO_P3L1	18
#define ADM5120_GPIO_P3L2	19
#define ADM5120_GPIO_P4L0	20
#define ADM5120_GPIO_P4L1	21
#define ADM5120_GPIO_P4L2	22
#define ADM5120_GPIO_MAX	22
#define ADM5120_GPIO_COUNT	ADM5120_GPIO_MAX+1

#define ADM5120_GPIO_LOW	0
#define ADM5120_GPIO_HIGH	1

#define ADM5120_GPIO_SWITCH	0x10
#define ADM5120_GPIO_FLASH	(ADM5120_GPIO_SWITCH | LED_MODE_FLASH)
#define ADM5120_GPIO_LINK	(ADM5120_GPIO_SWITCH | LED_MODE_LINK)
#define ADM5120_GPIO_SPEED	(ADM5120_GPIO_SWITCH | LED_MODE_SPEED)
#define ADM5120_GPIO_DUPLEX	(ADM5120_GPIO_SWITCH | LED_MODE_DUPLEX)
#define ADM5120_GPIO_ACT	(ADM5120_GPIO_SWITCH | LED_MODE_ACT)
#define ADM5120_GPIO_COLL	(ADM5120_GPIO_SWITCH | LED_MODE_COLL)
#define ADM5120_GPIO_LINK_ACT	(ADM5120_GPIO_SWITCH | LED_MODE_LINK_ACT)
#define ADM5120_GPIO_DUPLEX_COLL (ADM5120_GPIO_SWITCH | LED_MODE_DUPLEX_COLL)
#define ADM5120_GPIO_10M_ACT	(ADM5120_GPIO_SWITCH | LED_MODE_10M_ACT)
#define ADM5120_GPIO_100M_ACT	(ADM5120_GPIO_SWITCH | LED_MODE_100M_ACT)

extern int  __adm5120_gpio0_get_value(unsigned gpio);
extern void __adm5120_gpio0_set_value(unsigned gpio, int value);
extern int  __adm5120_gpio1_get_value(unsigned gpio);
extern void __adm5120_gpio1_set_value(unsigned gpio, int value);
extern int  adm5120_gpio_to_irq(unsigned gpio);
extern int  adm5120_irq_to_gpio(unsigned irq);

static inline int gpio_get_value(unsigned gpio)
{
	int ret;

	switch (gpio) {
	case ADM5120_GPIO_PIN0 ... ADM5120_GPIO_PIN7:
		ret = __adm5120_gpio0_get_value(gpio);
		break;
	case ADM5120_GPIO_P0L0 ... ADM5120_GPIO_P4L2:
		ret = __adm5120_gpio1_get_value(gpio - ADM5120_GPIO_P0L0);
		break;
	default:
		ret = __gpio_get_value(gpio);
		break;
	}

	return ret;
}

static inline void gpio_set_value(unsigned gpio, int value)
{
	switch (gpio) {
	case ADM5120_GPIO_PIN0 ... ADM5120_GPIO_PIN7:
		__adm5120_gpio0_set_value(gpio, value);
		break;
	case ADM5120_GPIO_P0L0 ... ADM5120_GPIO_P4L2:
		__adm5120_gpio1_set_value(gpio - ADM5120_GPIO_P0L0, value);
		break;
	default:
		__gpio_set_value(gpio, value);
		break;
	}
}

static inline int gpio_to_irq(unsigned gpio)
{
	return adm5120_gpio_to_irq(gpio);
}

static inline int irq_to_gpio(unsigned irq)
{
	return adm5120_irq_to_gpio(irq);
}

#define gpio_cansleep	__gpio_cansleep

#endif /* _ASM_MIPS_MACH_ADM5120_GPIO_H */
