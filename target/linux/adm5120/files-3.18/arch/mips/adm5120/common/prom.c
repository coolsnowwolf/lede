/*
 *  ADM5120 specific prom routines
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/mm.h>

#include <asm/bootinfo.h>
#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_info.h>

#include <prom/cfe.h>
#include <prom/generic.h>
#include <prom/routerboot.h>
#include <prom/myloader.h>
#include <prom/zynos.h>

unsigned int adm5120_prom_type	= ADM5120_PROM_GENERIC;

struct board_desc {
	unsigned long	mach_type;
	char		*name;
};

#define DEFBOARD(n, mt) { .mach_type = (mt), .name = (n)}
static struct board_desc common_boards[] __initdata = {
	/* Cellvision/SparkLAN boards */
	DEFBOARD("CAS-630",	MACH_ADM5120_CAS630),
	DEFBOARD("CAS-670",	MACH_ADM5120_CAS670),
	DEFBOARD("CAS-700",	MACH_ADM5120_CAS700),
	DEFBOARD("CAS-771",	MACH_ADM5120_CAS771),
	DEFBOARD("CAS-790",	MACH_ADM5120_CAS790),
	DEFBOARD("CAS-861",	MACH_ADM5120_CAS861),
	DEFBOARD("NFS-101U",	MACH_ADM5120_NFS101U),
	/* Compex boards */
	DEFBOARD("WP54G-WRT",	MACH_ADM5120_WP54G_WRT),
	/* Edimax boards */
	DEFBOARD("BR-6104K",	MACH_ADM5120_BR6104K),
	DEFBOARD("BR-6104KP",	MACH_ADM5120_BR6104KP),
	DEFBOARD("BR-6104WG",	MACH_ADM5120_BR61X4WG),
	DEFBOARD("BR-6114WG",	MACH_ADM5120_BR61X4WG),
	/* Infineon boards */
	DEFBOARD("EASY 5120P-ATA",	MACH_ADM5120_EASY5120PATA),
	DEFBOARD("EASY 5120-RT",	MACH_ADM5120_EASY5120RT),
	DEFBOARD("EASY 5120-WVoIP",	MACH_ADM5120_EASY5120WVOIP),
	DEFBOARD("EASY 83000",		MACH_ADM5120_EASY83000),
	/* Mikrotik RouterBOARDs */
	DEFBOARD("111",		MACH_ADM5120_RB_11X),
	DEFBOARD("112",		MACH_ADM5120_RB_11X),
	DEFBOARD("133",		MACH_ADM5120_RB_133),
	DEFBOARD("133C",	MACH_ADM5120_RB_133C),
	DEFBOARD("133C3",	MACH_ADM5120_RB_133C),
	DEFBOARD("150",		MACH_ADM5120_RB_153), /* it's intentional */
	DEFBOARD("153",		MACH_ADM5120_RB_153),
	DEFBOARD("192",		MACH_ADM5120_RB_192),
	DEFBOARD("miniROUTER",	MACH_ADM5120_RB_150),
	/* OSBRiDGE boards */
	DEFBOARD("OSBRiDGE 5GXi",	MACH_ADM5120_5GXI),
	/* Motorola boards */
	DEFBOARD("Powerline MU Gateway", MACH_ADM5120_PMUGW),
	/* Generic EB-214A */
	DEFBOARD("ADM5120",	MACH_ADM5120_EB_214A),
};

static unsigned long __init find_machtype_byname(char *name)
{
	unsigned long ret;
	int i;

	ret = MACH_ADM5120_GENERIC;
	if (name == NULL)
		goto out;

	if (*name == '\0')
		goto out;

	for (i = 0; i < ARRAY_SIZE(common_boards); i++) {
		if (strcmp(common_boards[i].name, name) == 0) {
			ret = common_boards[i].mach_type;
			break;
		}
	}

out:
	return ret;
}

static unsigned long __init detect_machtype_routerboot(void)
{
	char *name;

	name = routerboot_get_boardname();
	return find_machtype_byname(name);
}

static unsigned long __init detect_machtype_generic(void)
{
	char *name;

	name = generic_prom_getenv("board_name");
	return find_machtype_byname(name);
}

unsigned long __init detect_machtype_cfe(void)
{
	char *name;

	name = cfe_getenv("BOARD_NAME");
	return find_machtype_byname(name);
}

static struct {
	unsigned long	mach_type;
	u16		vendor_id;
	u16		board_id;
} zynos_boards[] __initdata = {
#define ZYNOS_BOARD(vi, bi, mt) \
	{.vendor_id = (vi), .board_id = (bi), .mach_type = (mt)}

#define ZYXEL_BOARD(bi, mt) ZYNOS_BOARD(ZYNOS_VENDOR_ID_ZYXEL, bi, mt)
#define DLINK_BOARD(bi, mt) ZYNOS_BOARD(ZYNOS_VENDOR_ID_DLINK, bi, mt)
#define LUCENT_BOARD(bi, mt) ZYNOS_BOARD(ZYNOS_VENDOR_ID_LUCENT, bi, mt)
	ZYXEL_BOARD(ZYNOS_BOARD_HS100,	MACH_ADM5120_HS100),
	ZYXEL_BOARD(ZYNOS_BOARD_P334U,	MACH_ADM5120_P334U),
	ZYXEL_BOARD(ZYNOS_BOARD_P334W,	MACH_ADM5120_P334W),
	ZYXEL_BOARD(ZYNOS_BOARD_P334WH,	MACH_ADM5120_P334WH),
	ZYXEL_BOARD(ZYNOS_BOARD_P334WHD, MACH_ADM5120_P334WHD),
	ZYXEL_BOARD(ZYNOS_BOARD_P334WT,	MACH_ADM5120_P334WT),
	ZYXEL_BOARD(ZYNOS_BOARD_P334WT_ALT,	MACH_ADM5120_P334WT),
	ZYXEL_BOARD(ZYNOS_BOARD_P335,	MACH_ADM5120_P335),
	ZYXEL_BOARD(ZYNOS_BOARD_P335PLUS, MACH_ADM5120_P335PLUS),
	ZYXEL_BOARD(ZYNOS_BOARD_P335U,	MACH_ADM5120_P335U)
};

static unsigned long __init detect_machtype_bootbase(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(zynos_boards); i++) {
		if (zynos_boards[i].vendor_id == bootbase_info.vendor_id &&
			zynos_boards[i].board_id == bootbase_info.board_id) {
			return zynos_boards[i].mach_type;
			break;
		}
	}

	printk(KERN_WARNING "Unknown ZyXEL model (%u)\n",
				bootbase_info.board_id);
	return MACH_ADM5120_GENERIC;
}

static struct {
	unsigned long	mach_type;
	u16	vid;
	u16	did;
	u16	svid;
	u16	sdid;
} mylo_boards[]  __initdata = {
#define MYLO_BOARD(v, d, sv, sd, mt) \
	{.vid = (v), .did = (d), .svid = (sv), .sdid = (sd), .mach_type = (mt)}
#define COMPEX_BOARD(d, mt) \
	MYLO_BOARD(VENID_COMPEX, (d), VENID_COMPEX, (d), (mt))

	COMPEX_BOARD(DEVID_COMPEX_NP27G, MACH_ADM5120_NP27G),
	COMPEX_BOARD(DEVID_COMPEX_NP28G, MACH_ADM5120_NP28G),
	COMPEX_BOARD(DEVID_COMPEX_NP28GHS, MACH_ADM5120_NP28GHS),
	COMPEX_BOARD(DEVID_COMPEX_WP54G, MACH_ADM5120_WP54),
	COMPEX_BOARD(DEVID_COMPEX_WP54Gv1C, MACH_ADM5120_WP54Gv1C),
	COMPEX_BOARD(DEVID_COMPEX_WP54AG, MACH_ADM5120_WP54),
	COMPEX_BOARD(DEVID_COMPEX_WPP54G, MACH_ADM5120_WP54),
	COMPEX_BOARD(DEVID_COMPEX_WPP54AG, MACH_ADM5120_WP54),
};

static unsigned long __init detect_machtype_myloader(void)
{
	unsigned long ret;
	int i;

	ret = MACH_ADM5120_GENERIC;
	for (i = 0; i < ARRAY_SIZE(mylo_boards); i++) {
		if (mylo_boards[i].vid == myloader_info.vid &&
			mylo_boards[i].did == myloader_info.did &&
			mylo_boards[i].svid == myloader_info.svid &&
			mylo_boards[i].sdid == myloader_info.sdid) {
			ret = mylo_boards[i].mach_type;
			break;
		}
	}

	return ret;
}

static void __init prom_detect_machtype(void)
{
	if (bootbase_present()) {
		adm5120_prom_type = ADM5120_PROM_BOOTBASE;
		mips_machtype = detect_machtype_bootbase();
		return;
	}

	if (cfe_present()) {
		adm5120_prom_type = ADM5120_PROM_CFE;
		mips_machtype = detect_machtype_cfe();
		return;
	}

	if (myloader_present()) {
		adm5120_prom_type = ADM5120_PROM_MYLOADER;
		mips_machtype = detect_machtype_myloader();
		return;
	}

	if (routerboot_present()) {
		adm5120_prom_type = ADM5120_PROM_ROUTERBOOT;
		mips_machtype = detect_machtype_routerboot();
		return;
	}

	if (generic_prom_present()) {
		adm5120_prom_type = ADM5120_PROM_GENERIC;
		mips_machtype = detect_machtype_generic();
		return;
	}

	mips_machtype = MACH_ADM5120_GENERIC;
}

#ifdef CONFIG_IMAGE_CMDLINE_HACK
extern char __image_cmdline[];

static void __init prom_init_cmdline(void)
{
	char *cmd;

	/* init command line, register a default kernel command line */
	cmd = __image_cmdline;
	if (strlen(cmd) > 0)
		strlcpy(arcs_cmdline, cmd, sizeof(arcs_cmdline));

}
#else
static inline void prom_init_cmdline(void) {}
#endif /* CONFIG_IMAGE_CMDLINE_HACK */

void __init prom_init(void)
{
	prom_detect_machtype();
	prom_init_cmdline();
}

void __init prom_free_prom_memory(void)
{
	/* We do not have to prom memory to free */
}
