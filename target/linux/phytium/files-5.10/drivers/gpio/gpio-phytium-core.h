/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef _GPIO_PHYTIUM_H
#define _GPIO_PHYTIUM_H

#include <linux/gpio/driver.h>
#include <linux/spinlock.h>

#include "gpiolib.h"

#define GPIO_SWPORTA_DR		0x00 /* WR Port A Output Data Register */
#define GPIO_SWPORTA_DDR	0x04 /* WR Port A Data Direction Register */
#define GPIO_EXT_PORTA		0x08 /* RO Port A Input Data Register */
#define GPIO_SWPORTB_DR		0x0c /* WR Port B Output Data Register */
#define GPIO_SWPORTB_DDR	0x10 /* WR Port B Data Direction Register */
#define GPIO_EXT_PORTB		0x14 /* RO Port B Input Data Register */

#define GPIO_INTEN		0x18 /* WR Port A Interrput Enable Register */
#define GPIO_INTMASK		0x1c /* WR Port A Interrupt Mask Register */
#define GPIO_INTTYPE_LEVEL	0x20 /* WR Port A Interrupt Level Register */
#define GPIO_INT_POLARITY	0x24 /* WR Port A Interrupt Polarity Register */
#define GPIO_INTSTATUS		0x28 /* RO Port A Interrupt Status Register */
#define GPIO_RAW_INTSTATUS	0x2c /* RO Port A Raw Interrupt Status Register */
#define GPIO_LS_SYNC		0x30 /* WR Level-sensitive Synchronization Enable Register */
#define GPIO_DEBOUNCE		0x34 /* WR Debounce Enable Register */
#define GPIO_PORTA_EOI		0x38 /* WO Port A Clear Interrupt Register */

#define MAX_NPORTS		2
#define NGPIO_DEFAULT		8
#define NGPIO_MAX		32
#define GPIO_PORT_STRIDE	(GPIO_EXT_PORTB - GPIO_EXT_PORTA)

struct pin_loc {
	unsigned int port;
	unsigned int offset;
};

#ifdef CONFIG_PM_SLEEP
struct phytium_gpio_ctx {
	u32 swporta_dr;
	u32 swporta_ddr;
	u32 ext_porta;
	u32 swportb_dr;
	u32 swportb_ddr;
	u32 ext_portb;
	u32 inten;
	u32 intmask;
	u32 inttype_level;
	u32 int_polarity;
	u32 intstatus;
	u32 raw_intstatus;
	u32 ls_sync;
	u32 debounce;
};
#endif

struct phytium_gpio {
	raw_spinlock_t		lock;
	void __iomem		*regs;
	struct gpio_chip	gc;
	struct irq_chip		irq_chip;
	unsigned int		ngpio[2];
	int			irq[32];
#ifdef CONFIG_PM_SLEEP
	struct phytium_gpio_ctx	ctx;
#endif
};

int phytium_gpio_get(struct gpio_chip *gc, unsigned int offset);
void phytium_gpio_set(struct gpio_chip *gc, unsigned int offset, int value);

int phytium_gpio_get_direction(struct gpio_chip *gc, unsigned int offset);
int phytium_gpio_direction_input(struct gpio_chip *gc, unsigned int offset);
int phytium_gpio_direction_output(struct gpio_chip *gc, unsigned int offset, int value);

void phytium_gpio_irq_ack(struct irq_data *d);
void phytium_gpio_irq_mask(struct irq_data *d);
void phytium_gpio_irq_unmask(struct irq_data *d);
int phytium_gpio_irq_set_type(struct irq_data *d, unsigned int flow_type);
void phytium_gpio_irq_enable(struct irq_data *d);
void phytium_gpio_irq_disable(struct irq_data *d);
void phytium_gpio_irq_handler(struct irq_desc *desc);
int phytium_gpio_irq_set_affinity(struct irq_data *d, const struct cpumask *mask_val, bool force);
#endif
