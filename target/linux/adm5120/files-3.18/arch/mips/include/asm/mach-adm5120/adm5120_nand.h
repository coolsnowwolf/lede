/*
 *  ADM5120 NAND interface definitions
 *
 *  This header file defines the hardware registers of the ADM5120 SoC
 *  built-in NAND interface.
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  NAND interface routines was based on a driver for Linux 2.6.19+ which
 *  was derived from the driver for Linux 2.4.xx published by Mikrotik for
 *  their RouterBoard 1xx and 5xx series boards.
 *    Copyright (C) 2007 David Goodenough <david.goodenough@linkchoose.co.uk>
 *    Copyright (C) 2007 Florian Fainelli <florian@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_NAND_H
#define _MACH_ADM5120_NAND_H

#include <linux/types.h>
#include <linux/io.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>

/* NAND control registers */
#define NAND_REG_DATA		0x0 /* data register */
#define NAND_REG_SET_CEn	0x1 /* CE# low */
#define NAND_REG_CLR_CEn	0x2 /* CE# high */
#define NAND_REG_CLR_CLE	0x3 /* CLE low */
#define NAND_REG_SET_CLE	0x4 /* CLE high */
#define NAND_REG_CLR_ALE	0x5 /* ALE low */
#define NAND_REG_SET_ALE	0x6 /* ALE high */
#define NAND_REG_SET_SPn	0x7 /* SP# low (use spare area) */
#define NAND_REG_CLR_SPn	0x8 /* SP# high (do not use spare area) */
#define NAND_REG_SET_WPn	0x9 /* WP# low */
#define NAND_REG_CLR_WPn	0xA /* WP# high */
#define NAND_REG_STATUS		0xB /* Status register */

#define ADM5120_NAND_STATUS_READY	0x80

#define NAND_READ_REG(r) \
	readb((void __iomem *)KSEG1ADDR(ADM5120_NAND_BASE) + (r))
#define NAND_WRITE_REG(r, v) \
	writeb((v), (void __iomem *)KSEG1ADDR(ADM5120_NAND_BASE) + (r))

/*-------------------------------------------------------------------------*/

static inline void adm5120_nand_enable(void)
{
	SW_WRITE_REG(SWITCH_REG_BW_CNTL1, BW_CNTL1_NAND_ENABLE);
	SW_WRITE_REG(SWITCH_REG_BOOT_DONE, 1);
}

static inline void adm5120_nand_set_wpn(unsigned int set)
{
	NAND_WRITE_REG((set) ? NAND_REG_SET_WPn : NAND_REG_CLR_WPn, 1);
}

static inline void adm5120_nand_set_spn(unsigned int set)
{
	NAND_WRITE_REG((set) ? NAND_REG_SET_SPn : NAND_REG_CLR_SPn, 1);
}

static inline void adm5120_nand_set_cle(unsigned int set)
{
	NAND_WRITE_REG((set) ? NAND_REG_SET_CLE : NAND_REG_CLR_CLE, 1);
}

static inline void adm5120_nand_set_ale(unsigned int set)
{
	NAND_WRITE_REG((set) ? NAND_REG_SET_ALE : NAND_REG_CLR_ALE, 1);
}

static inline void adm5120_nand_set_cen(unsigned int set)
{
	NAND_WRITE_REG((set) ? NAND_REG_SET_CEn : NAND_REG_CLR_CEn, 1);
}

static inline u8 adm5120_nand_get_status(void)
{
	return NAND_READ_REG(NAND_REG_STATUS);
}

#endif /* _MACH_ADM5120_NAND_H */
