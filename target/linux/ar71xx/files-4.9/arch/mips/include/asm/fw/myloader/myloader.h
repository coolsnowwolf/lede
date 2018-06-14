/*
 *  Compex's MyLoader specific definitions
 *
 *  Copyright (C) 2006-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ASM_MIPS_FW_MYLOADER_H
#define _ASM_MIPS_FW_MYLOADER_H

#include <linux/myloader.h>

struct myloader_info {
	uint32_t	vid;
	uint32_t	did;
	uint32_t	svid;
	uint32_t	sdid;
	uint8_t		macs[MYLO_ETHADDR_COUNT][6];
};

#ifdef CONFIG_MYLOADER
extern struct myloader_info *myloader_get_info(void) __init;
#else
static inline struct myloader_info *myloader_get_info(void)
{
	return NULL;
}
#endif /* CONFIG_MYLOADER */

#endif /* _ASM_MIPS_FW_MYLOADER_H */
