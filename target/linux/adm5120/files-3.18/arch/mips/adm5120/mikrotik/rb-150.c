/*
 *  Mikrotik RouterBOARD 150 support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

#define RB150_NAND_BASE		0x1FC80000
#define RB150_NAND_SIZE		1

#define RB150_GPIO_NAND_READY	ADM5120_GPIO_PIN0
#define RB150_GPIO_NAND_NCE	ADM5120_GPIO_PIN1
#define RB150_GPIO_NAND_CLE	ADM5120_GPIO_P2L2
#define RB150_GPIO_NAND_ALE	ADM5120_GPIO_P3L2
#define RB150_GPIO_RESET_BUTTON	ADM5120_GPIO_PIN1 /* FIXME */

#define RB150_NAND_DELAY	100

#define RB150_NAND_WRITE(v) \
	writeb((v), (void __iomem *)KSEG1ADDR(RB150_NAND_BASE))

static struct resource rb150_nand_resources[] __initdata = {
	[0] = {
		.start	= RB150_NAND_BASE,
		.end	= RB150_NAND_BASE + RB150_NAND_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct gpio_led rb150_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_P0L2, "user",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1, "lan1_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan1_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L1, "lan5_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan5_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L1, "lan4_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan4_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L1, "lan3_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan3_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L1, "lan2_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "lan2_led2",	NULL),
};

static u8 rb150_vlans[6] __initdata = {
	0x7F, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int rb150_nand_dev_ready(struct mtd_info *mtd)
{
	return gpio_get_value(RB150_GPIO_NAND_READY);
}

static void rb150_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
		unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		gpio_set_value(RB150_GPIO_NAND_CLE, (ctrl & NAND_CLE) ? 1 : 0);
		gpio_set_value(RB150_GPIO_NAND_ALE, (ctrl & NAND_ALE) ? 1 : 0);
		gpio_set_value(RB150_GPIO_NAND_NCE, (ctrl & NAND_NCE) ? 0 : 1);
	}

	udelay(RB150_NAND_DELAY);

	if (cmd != NAND_CMD_NONE)
		RB150_NAND_WRITE(cmd);
}

static void __init rb150_add_device_nand(void)
{
	struct platform_device *pdev;
	int err;

	/* setup GPIO pins for NAND flash chip */
	gpio_request(RB150_GPIO_NAND_READY, "nand-ready");
	gpio_direction_input(RB150_GPIO_NAND_READY);
	gpio_request(RB150_GPIO_NAND_NCE, "nand-nce");
	gpio_direction_output(RB150_GPIO_NAND_NCE, 1);
	gpio_request(RB150_GPIO_NAND_CLE, "nand-cle");
	gpio_direction_output(RB150_GPIO_NAND_CLE, 0);
	gpio_request(RB150_GPIO_NAND_ALE, "nand-ale");
	gpio_direction_output(RB150_GPIO_NAND_ALE, 0);

	pdev = platform_device_alloc("gen_nand", -1);
	if (!pdev)
		goto err_out;

	err = platform_device_add_resources(pdev, rb150_nand_resources,
					ARRAY_SIZE(rb150_nand_resources));
	if (err)
		goto err_put;


	rb1xx_nand_data.ctrl.cmd_ctrl = rb150_nand_cmd_ctrl;
	rb1xx_nand_data.ctrl.dev_ready = rb150_nand_dev_ready;

	err = platform_device_add_data(pdev, &rb1xx_nand_data,
					sizeof(rb1xx_nand_data));
	if (err)
		goto err_put;

	err = platform_device_add(pdev);
	if (err)
		goto err_put;

	return;

err_put:
	platform_device_put(pdev);
err_out:
	return;
}

static void __init rb150_setup(void)
{
	rb1xx_gpio_buttons[0].gpio = RB150_GPIO_RESET_BUTTON;
	rb1xx_generic_setup();
	rb150_add_device_nand();

	adm5120_add_device_gpio_leds(ARRAY_SIZE(rb150_gpio_leds),
					rb150_gpio_leds);
	adm5120_add_device_switch(5, rb150_vlans);
}

MIPS_MACHINE(MACH_ADM5120_RB_150, "miniROUTER", "Mikrotik RouterBOARD 150",
	     rb150_setup);
