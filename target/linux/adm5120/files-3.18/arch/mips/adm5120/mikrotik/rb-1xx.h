/*
 *  Mikrotik RouterBOARD 1xx series support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/etherdevice.h>

#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_nand.h>
#include <asm/mach-adm5120/adm5120_platform.h>
#include <asm/mach-adm5120/adm5120_info.h>

#include <prom/routerboot.h>

extern struct platform_nand_data rb1xx_nand_data __initdata;
extern struct gpio_keys_button rb1xx_gpio_buttons[] __initdata;

extern void rb1xx_add_device_flash(void) __init;
extern void rb1xx_add_device_nand(void) __init;
extern void rb1xx_generic_setup(void) __init;
