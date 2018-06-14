/*
 *  MikroTik RouterBOARD 750 definitions
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#ifndef _MACH_RB750_H
#define _MACH_RB750_H

#include <linux/bitops.h>

#define RB750_GPIO_LVC573_LE	0	/* Latch enable on LVC573 */
#define RB750_GPIO_NAND_IO0	1	/* NAND I/O 0 */
#define RB750_GPIO_NAND_IO1	2	/* NAND I/O 1 */
#define RB750_GPIO_NAND_IO2	3	/* NAND I/O 2 */
#define RB750_GPIO_NAND_IO3	4	/* NAND I/O 3 */
#define RB750_GPIO_NAND_IO4	5	/* NAND I/O 4 */
#define RB750_GPIO_NAND_IO5	6	/* NAND I/O 5 */
#define RB750_GPIO_NAND_IO6	7	/* NAND I/O 6 */
#define RB750_GPIO_NAND_IO7	8	/* NAND I/O 7 */
#define RB750_GPIO_NAND_NCE	11	/* NAND Chip Enable (active low) */
#define RB750_GPIO_NAND_RDY	12	/* NAND Ready */
#define RB750_GPIO_NAND_CLE	14	/* NAND Command Latch Enable */
#define RB750_GPIO_NAND_ALE	15	/* NAND Address Latch Enable */
#define RB750_GPIO_NAND_NRE	16	/* NAND Read Enable (active low) */
#define RB750_GPIO_NAND_NWE	17	/* NAND Write Enable (active low) */

#define RB750_GPIO_BTN_RESET	1
#define RB750_GPIO_SPI_CS0	2
#define RB750_GPIO_LED_ACT	12
#define RB750_GPIO_LED_PORT1	13
#define RB750_GPIO_LED_PORT2	14
#define RB750_GPIO_LED_PORT3	15
#define RB750_GPIO_LED_PORT4	16
#define RB750_GPIO_LED_PORT5	17

#define RB750_LED_ACT		BIT(RB750_GPIO_LED_ACT)
#define RB750_LED_PORT1		BIT(RB750_GPIO_LED_PORT1)
#define RB750_LED_PORT2		BIT(RB750_GPIO_LED_PORT2)
#define RB750_LED_PORT3		BIT(RB750_GPIO_LED_PORT3)
#define RB750_LED_PORT4		BIT(RB750_GPIO_LED_PORT4)
#define RB750_LED_PORT5		BIT(RB750_GPIO_LED_PORT5)
#define RB750_NAND_NCE		BIT(RB750_GPIO_NAND_NCE)

#define RB750_LVC573_LE		BIT(RB750_GPIO_LVC573_LE)

#define RB750_LED_BITS	(RB750_LED_PORT1 | RB750_LED_PORT2 | RB750_LED_PORT3 | \
			 RB750_LED_PORT4 | RB750_LED_PORT5 | RB750_LED_ACT)

#define RB7XX_GPIO_NAND_NCE	0
#define RB7XX_GPIO_MON		9
#define RB7XX_GPIO_LED_ACT	11
#define RB7XX_GPIO_USB_POWERON	13

#define RB7XX_NAND_NCE		BIT(RB7XX_GPIO_NAND_NCE)
#define RB7XX_LED_ACT		BIT(RB7XX_GPIO_LED_ACT)
#define RB7XX_MONITOR		BIT(RB7XX_GPIO_MON)
#define RB7XX_USB_POWERON	BIT(RB7XX_GPIO_USB_POWERON)

struct rb750_led_data {
	char	*name;
	char	*default_trigger;
	u32	mask;
	int	active_low;
};

struct rb750_led_platform_data {
	int			num_leds;
	struct rb750_led_data	*leds;
	void			(*latch_change)(u32 clear, u32 set);
};

struct rb7xx_nand_platform_data {
	u32 nce_line;

	void (*enable_pins)(void);
	void (*disable_pins)(void);
	void (*latch_change)(u32, u32);
};

#endif /* _MACH_RB750_H */