/*
 *  Cellvision/SparkLAN boards
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
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/etherdevice.h>

#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_platform.h>

extern void cellvision_mac_setup(void) __init;

extern void cas6xx_flash_setup(void) __init;
extern void cas7xx_flash_setup(void) __init;
extern void cas6xx_setup(void) __init;
extern void cas7xx_setup(void) __init;
