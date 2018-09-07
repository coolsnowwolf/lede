/*
 *  Mikrotik's RouterBOOT definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _PROM_ROUTERBOOT_H_
#define _PROM_ROUTERBOOT_H_

struct rb_bios_settings {
	u32	hs_offs; /* hard settings offset */
	u32	hs_size; /* hard settings size */
	u32	fw_offs; /* firmware offset */
	u32	ss_offs; /* soft settings offset */
	u32	ss_size; /* soft settings size */
};

struct rb_hard_settings {
	char	*name;		/* board name */
	char	*bios_ver;	/* BIOS version */
	u32	mem_size;	/* memory size in bytes */
	u32	mac_count;	/* number of mac addresses */
	u8	*mac_base;	/* mac address base */
};

extern int routerboot_present(void) __init;
extern char *routerboot_get_boardname(void);

extern struct rb_hard_settings rb_hs;

#endif /* _PROM_ROUTERBOOT_H_ */
