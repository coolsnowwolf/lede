/*
 *  Infineon Reference Boards
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

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_platform.h>
#include <asm/mach-adm5120/adm5120_info.h>

extern void easy_setup_pqfp(void) __init;
extern void easy_setup_bga(void) __init;
