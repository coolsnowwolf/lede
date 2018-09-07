/*
 *  ZyNOS (ZyXEL's Networking OS) definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ZYNOS_H
#define _ZYNOS_H

#define ZYNOS_NAME_LEN		32
#define ZYNOS_FEAT_BYTES	22
#define ZYNOS_MAC_LEN		6

struct zynos_board_info {
	unsigned char	vendor[ZYNOS_NAME_LEN];
	unsigned char	product[ZYNOS_NAME_LEN];
	u32		bootext_addr;
	u32		res0;
	u16		board_id;
	u8		res1[6];
	u8		feat_other[ZYNOS_FEAT_BYTES];
	u8		feat_main;
	u8		res2;
	u8		mac[ZYNOS_MAC_LEN];
	u8		country;
	u8		dbgflag;
} __attribute__ ((packed));

/*
 * Vendor IDs
 */
#define ZYNOS_VENDOR_ID_ZYXEL	0
#define ZYNOS_VENDOR_ID_NETGEAR	1
#define ZYNOS_VENDOR_ID_DLINK	2
#define ZYNOS_VENDOR_ID_OTHER	3
#define ZYNOS_VENDOR_ID_LUCENT	4

/*
 * Vendor names
 */
#define ZYNOS_VENDOR_DLINK	"D-Link"
#define ZYNOS_VENDOR_LUCENT	"LUCENT"
#define ZYNOS_VENDOR_NETGEAR	"NetGear"
#define ZYNOS_VENDOR_ZYXEL	"ZyXEL"

/*
 * Board IDs (big-endian)
 */
#define ZYNOS_BOARD_ES2108	0x00F2	/* Ethernet Switch 2108 */
#define ZYNOS_BOARD_ES2108F	0x01AF	/* Ethernet Switch 2108-F */
#define ZYNOS_BOARD_ES2108G	0x00F3	/* Ethernet Switch 2108-G */
#define ZYNOS_BOARD_ES2108LC	0x00FC	/* Ethernet Switch 2108-LC */
#define ZYNOS_BOARD_ES2108PWR	0x00F4	/* Ethernet Switch 2108PWR */
#define ZYNOS_BOARD_HS100	0x9FF1	/* HomeSafe 100/100W */
#define ZYNOS_BOARD_P334	0x9FF5	/* Prestige 334 */
#define ZYNOS_BOARD_P334U	0x9FDD	/* Prestige 334U */
#define ZYNOS_BOARD_P334W	0x9FF3	/* Prestige 334W */
#define ZYNOS_BOARD_P334WH	0x00E0	/* Prestige 334WH */
#define ZYNOS_BOARD_P334WHD	0x00E1	/* Prestige 334WHD */
#define ZYNOS_BOARD_P334WT	0x9FEF	/* Prestige 334WT */
#define ZYNOS_BOARD_P334WT_ALT	0x9F02	/* Prestige 334WT alternative*/
#define ZYNOS_BOARD_P335	0x9FED	/* Prestige 335/335WT */
#define ZYNOS_BOARD_P335PLUS	0x0025	/* Prestige 335Plus */
#define ZYNOS_BOARD_P335U	0x9FDC	/* Prestige 335U */

/*
 * Some magic numbers (big-endian)
 */
#define ZYNOS_MAGIC_DBGAREA1	0x48646267	/* "Hdbg" */
#define ZYNOS_MAGIC_DBGAREA2	0x61726561	/* "area" */

struct bootbase_info {
	u16		vendor_id;
	u16		board_id;
	u8		mac[6];
};

extern struct bootbase_info bootbase_info;
extern int bootbase_present(void) __init;

#endif /* _ZYNOS_H */
