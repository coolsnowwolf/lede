/*
 *  Copyright (C) 2007-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_INFO_H
#define _MACH_ADM5120_INFO_H

#include <linux/types.h>

extern unsigned int adm5120_prom_type;
#define ADM5120_PROM_GENERIC	0
#define ADM5120_PROM_CFE	1
#define ADM5120_PROM_MYLOADER	2
#define ADM5120_PROM_ROUTERBOOT	3
#define ADM5120_PROM_BOOTBASE	4
#define ADM5120_PROM_UBOOT	5
#define ADM5120_PROM_LAST	5

extern unsigned int adm5120_product_code;
extern unsigned int adm5120_revision;
extern unsigned int adm5120_nand_boot;

extern unsigned long adm5120_speed;
#define ADM5120_SPEED_175	175000000
#define ADM5120_SPEED_200	200000000
#define ADM5120_SPEED_225	225000000
#define ADM5120_SPEED_250	250000000

extern unsigned int adm5120_package;
#define ADM5120_PACKAGE_PQFP	0
#define ADM5120_PACKAGE_BGA	1

extern unsigned long adm5120_memsize;

enum {
	MACH_ADM5120_GENERIC = 0,	/* Generic board */
	MACH_ADM5120_5GXI,		/* OSBRiDGE 5GXi/5XLi */
	MACH_ADM5120_BR6104K,		/* Edimax BR-6104K */
	MACH_ADM5120_BR6104KP,		/* Edimax BR-6104KP */
	MACH_ADM5120_BR61X4WG,		/* Edimax BR-6104Wg/BR-6114WG */
	MACH_ADM5120_CAS630,		/* Cellvision CAS-630/630W */
	MACH_ADM5120_CAS670,		/* Cellvision CAS-670/670W */
	MACH_ADM5120_CAS700,		/* Cellvision CAS-700/700W */
	MACH_ADM5120_CAS771,		/* Cellvision CAS-771/771W */
	MACH_ADM5120_CAS790,		/* Cellvision CAS-790 */
	MACH_ADM5120_CAS861,		/* Cellvision CAS-861/861W */
	MACH_ADM5120_EASY5120PATA,	/* Infineon EASY 5120P-ATA */
	MACH_ADM5120_EASY5120RT,	/* Infineon EASY 5120-RT */
	MACH_ADM5120_EASY5120WVOIP,	/* Infineon EASY 5120-WVoIP */
	MACH_ADM5120_EASY83000,		/* Infineon EASY-83000 */
	MACH_ADM5120_ES2108,		/* ZyXEL Ethernet Switch 2108 */
	MACH_ADM5120_ES2108F,		/* ZyXEL Ethernet Switch 2108-F */
	MACH_ADM5120_ES2108G,		/* ZyXEL Ethernet Switch 2108-G */
	MACH_ADM5120_ES2108LC,		/* ZyXEL Ethernet Switch 2108-LC */
	MACH_ADM5120_ES2108PWR,		/* ZyXEL Ethernet Switch 2108-PWR */
	MACH_ADM5120_ES2024A,		/* ZyXEL Ethernet Switch 2024A */
	MACH_ADM5120_ES2024PWR,		/* ZyXEL Ethernet Switch 2024PWR */
	MACH_ADM5120_HS100,		/* ZyXEL HomeSafe 100/100W */
	MACH_ADM5120_NFS101U,		/* Cellvision NFS-101U/101WU */
	MACH_ADM5120_NFS202U,		/* Cellvision NFS-202U/202WU */
	MACH_ADM5120_NP28G,		/* Compex NP28G */
	MACH_ADM5120_NP28GHS,		/* Compex NP28G HotSpot */
	MACH_ADM5120_NP27G,		/* Compex NP27G */
	MACH_ADM5120_RB_11X,		/* Mikrotik RouterBOARD 111/112 */
	MACH_ADM5120_RB_133,		/* Mikrotik RouterBOARD 133 */
	MACH_ADM5120_RB_133C,		/* Mikrotik RouterBOARD 133c */
	MACH_ADM5120_RB_150,		/* Mikrotik RouterBOARD 150 */
	MACH_ADM5120_RB_153,		/* Mikrotik RouterBOARD 153 */
	MACH_ADM5120_RB_192,		/* Mikrotik RouterBOARD 192 */
	MACH_ADM5120_P334U,		/* ZyXEL Prestige 334U */
	MACH_ADM5120_P334W,		/* ZyXEL Prestige 334W */
	MACH_ADM5120_P334WH,		/* ZyXEL Prestige 334WH */
	MACH_ADM5120_P334WHD,		/* ZyXEL Prestige 334WHD */
	MACH_ADM5120_P334WT,		/* ZyXEL Prestige 334WT */
	MACH_ADM5120_P335,		/* ZyXEL Prestige 335/335WT */
	MACH_ADM5120_P335PLUS,		/* ZyXEL Prestige 335Plus */
	MACH_ADM5120_P335U,		/* ZyXEL Prestige 335U */
	MACH_ADM5120_PMUGW,		/* Motorola Powerline MU Gateway */
	MACH_ADM5120_WP54,		/* Compex WP54G/WP54AG/WPP54G/WPP54AG */
	MACH_ADM5120_WP54G_WRT,		/* Compex WP54G-WRT */
	MACH_ADM5120_WP54Gv1C,		/* Compex WP54G version 1C */
	MACH_ADM5120_EB_214A,		/* Generic EB-214A */
};

/*
 * TODO:remove adm5120_eth* variables when the switch driver will be
 *	converted into a real platform driver
 */
extern unsigned int adm5120_eth_num_ports;
extern unsigned char adm5120_eth_macs[6][6];
extern unsigned char adm5120_eth_vlans[6];

extern void adm5120_soc_init(void) __init;
extern void adm5120_mem_init(void) __init;
extern void adm5120_ndelay(u32 ns);

extern void (*adm5120_board_reset)(void);

extern void adm5120_gpio_init(void) __init;
extern void adm5120_gpio_csx0_enable(void) __init;
extern void adm5120_gpio_csx1_enable(void) __init;
extern void adm5120_gpio_ew_enable(void) __init;

static inline int adm5120_package_pqfp(void)
{
	return (adm5120_package == ADM5120_PACKAGE_PQFP);
}

static inline int adm5120_package_bga(void)
{
	return (adm5120_package == ADM5120_PACKAGE_BGA);
}

static inline int adm5120_has_pci(void)
{
	return (adm5120_package == ADM5120_PACKAGE_BGA);
}

static inline int adm5120_has_gmii(void)
{
	return (adm5120_package == ADM5120_PACKAGE_BGA);
}

#endif /* _MACH_ADM5120_INFO_H */
