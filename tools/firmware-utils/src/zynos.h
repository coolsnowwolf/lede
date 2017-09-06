/*
 *
 *  Copyright (C) 2007-2008 OpenWrt.org
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg at openwrt.org>
 *
 *  This code was based on the information of the ZyXEL's firmware
 *  image format written by Kolja Waschk, can be found at:
 *  http://www.ixo.de/info/zyxel_uclinux
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ZYNOS_H
#define _ZYNOS_H

#define BOOTBASE_NAME_LEN	32
#define BOOTBASE_MAC_LEN	6
#define BOOTBASE_FEAT_LEN	22

#define BOOTEXT_DEF_SIZE	0x18000

struct zyn_bootbase_info {
	char		vendor[BOOTBASE_NAME_LEN]; /* Vendor name */
	char		model[BOOTBASE_NAME_LEN]; /* Model name */
	uint32_t	bootext_addr;	/* absolute address of the Boot Extension */
	uint16_t	res0;		/* reserved/unknown */
	uint8_t		sys_type;	/* system type */
	uint8_t		res1;		/* reserved/unknown */
	uint16_t	model_id;	/* model id */
	uint8_t		feat_other[BOOTBASE_FEAT_LEN]; /* other feature bits */
	uint8_t		feat_main;	/* main feature bits */
	uint8_t		res2;		/* reserved/unknown */
	uint8_t		mac[BOOTBASE_MAC_LEN]; /* mac address */
	uint8_t		country;	/* default country code */
	uint8_t		dbgflag;	/* debug flag */
} __attribute__((packed));

#define ROMBIN_SIG_LEN	3
#define ROMBIN_VER_LEN	15

struct zyn_rombin_hdr {
	uint32_t	addr;		/* load address of the object */
	uint16_t	res0;		/* unknown/unused */
	char		sig[ROMBIN_SIG_LEN];	/* magic, must be "SIG" */
	uint8_t		type;		/* type of the object */
	uint32_t	osize;		/* size of the uncompressed data */
	uint32_t	csize;		/* size of the compressed data */
	uint8_t		flags;		/* various flags */
	uint8_t		res1;		/* unknown/unused */
	uint16_t	ocsum;		/* csum of the uncompressed data */
	uint16_t	ccsum;		/* csum of the compressed data */
	char		ver[ROMBIN_VER_LEN];
	uint32_t	mmap_addr;	/* address of the Memory Map Table*/
	uint32_t	res2;		/* unknown/unused*/
	uint8_t		res3;		/* unknown/unused*/
} __attribute__((packed));

#define ROMBIN_SIGNATURE	"SIG"

/* Rombin flag bits */
#define ROMBIN_FLAG_01		0x01
#define ROMBIN_FLAG_02		0x02
#define ROMBIN_FLAG_04		0x04
#define ROMBIN_FLAG_08		0x08
#define ROMBIN_FLAG_10		0x10
#define ROMBIN_FLAG_CCSUM	0x20	/* compressed checksum is valid */
#define ROMBIN_FLAG_OCSUM	0x40	/* original checksum is valid */
#define ROMBIN_FLAG_COMPRESSED	0x80	/* the binary is compressed */

/* Object types */
#define OBJECT_TYPE_ROMIMG	0x01
#define OBJECT_TYPE_ROMBOOT	0x02
#define OBJECT_TYPE_BOOTEXT	0x03
#define OBJECT_TYPE_ROMBIN	0x04
#define OBJECT_TYPE_ROMDIR	0x05
#define OBJECT_TYPE_6		0x06
#define OBJECT_TYPE_ROMMAP	0x07
#define OBJECT_TYPE_RAM		0x80
#define OBJECT_TYPE_RAMCODE	0x81
#define OBJECT_TYPE_RAMBOOT	0x82

/*
 * Memory Map Table header
 */
struct zyn_mmt_hdr {
	uint16_t	count;
	uint32_t	user_start;
	uint32_t	user_end;
	uint16_t	csum;
	uint8_t		res[12];
} __attribute__((packed));

#define OBJECT_NAME_LEN		8

struct zyn_mmt_item {
	uint8_t		type;	/* type of the object */
	uint8_t		name[OBJECT_NAME_LEN]; /* name of the object */
	uint8_t		res0;	/* unused/unknown */
	uint32_t	addr;
	uint32_t	size;	/* size of the object */
	uint8_t		res1[3]; /* unused/unknown */
	uint8_t		type2;
} __attribute__((packed));

/*
 * Vendor IDs
 */
#define ZYNOS_VENDOR_ID_ZYXEL	0
#define ZYNOS_VENDOR_ID_NETGEAR	1
#define ZYNOS_VENDOR_ID_DLINK	2
#define ZYNOS_VENDOR_ID_03	3
#define ZYNOS_VENDOR_ID_LUCENT	4
#define ZYNOS_VENDOR_ID_O2	10

/*
 * Model IDs (in big-endian format)
 */
#define MID(x)	(((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8)

/*
 * Infineon/ADMtek ADM5120 based models
 */
#define ZYNOS_MODEL_ES_2024A		MID(  221)
#define ZYNOS_MODEL_ES_2024PWR		MID( 4097)
#define ZYNOS_MODEL_ES_2108		MID(61952)
#define ZYNOS_MODEL_ES_2108_F		MID(44801)
#define ZYNOS_MODEL_ES_2108_G		MID(62208)
#define ZYNOS_MODEL_ES_2108_LC		MID(64512)
#define ZYNOS_MODEL_ES_2108PWR		MID(62464)
#define ZYNOS_MODEL_HS_100		MID(61855)
#define ZYNOS_MODEL_HS_100W		ZYNOS_MODEL_HS_100
#define ZYNOS_MODEL_P_334		MID(62879)
#define ZYNOS_MODEL_P_334U		MID(56735)
#define ZYNOS_MODEL_P_334W		MID(62367)
#define ZYNOS_MODEL_P_334WH		MID(57344)
#define ZYNOS_MODEL_P_334WHD		MID(57600)
#define ZYNOS_MODEL_P_334WT		MID(61343)
#define ZYNOS_MODEL_P_335		MID(60831)
#define ZYNOS_MODEL_P_335PLUS		MID( 9472)
#define ZYNOS_MODEL_P_335U		MID(56479)
#define ZYNOS_MODEL_P_335WT		ZYNOS_MODEL_P_335

/*
 * Texas Instruments AR7 based models
 */
#define ZYNOS_MODEL_P_2602H_61C		MID( 3229)
#define ZYNOS_MODEL_P_2602H_63C		MID( 3485)
#define ZYNOS_MODEL_P_2602H_D1A		/* n.a. */
#define ZYNOS_MODEL_P_2602H_D3A		/* n.a. */
#define ZYNOS_MODEL_P_2602HW_61C	/* n.a. */
#define ZYNOS_MODEL_P_2602HW_63		/* n.a. */
#define ZYNOS_MODEL_P_2602HW_63C	ZYNOS_MODEL_P_2602H_63C
#define ZYNOS_MODEL_P_2602HW_D1A	MID( 6301)
#define ZYNOS_MODEL_P_2602HW_D3A	/* n.a. */
#define ZYNOS_MODEL_P_2602HWL_61	MID( 1181)
#define ZYNOS_MODEL_P_2602HWL_61C	ZYNOS_MODEL_P_2602H_61C
#define ZYNOS_MODEL_P_2602HWL_63C	ZYNOS_MODEL_P_2602H_63C
#define ZYNOS_MODEL_P_2602HWL_D1A	ZYNOS_MODEL_P_2602HW_D1A
#define ZYNOS_MODEL_P_2602HWL_D3A	MID( 7581)
#define ZYNOS_MODEL_P_2602HWN_D7A	MID(30464)
#define ZYNOS_MODEL_P_2602HWNLI_D7A	MID( 6813)

#define ZYNOS_MODEL_P_2602R_61		MID( 2205)
#define ZYNOS_MODEL_P_2602R_63		MID( 3997)
#define ZYNOS_MODEL_P_2602R_D1A		/* n.a. */
#define ZYNOS_MODEL_P_2602R_D3A		/* n.a. */
#define ZYNOS_MODEL_P_2602RL_D1A	MID( 6045)
#define ZYNOS_MODEL_P_2602RL_D3A	MID( 7069)

#define ZYNOS_MODEL_P_660H_61		MID(19346)
#define ZYNOS_MODEL_P_660H_63		MID(22162)
#define ZYNOS_MODEL_P_660H_67		/* n.a. */
#define ZYNOS_MODEL_P_660H_D1		MID( 7066)
#define ZYNOS_MODEL_P_660H_D3		MID(13210)

#define ZYNOS_MODEL_P_660HW_61		ZYNOS_MODEL_P_660H_61
#define ZYNOS_MODEL_P_660HW_63		ZYNOS_MODEL_P_660H_63
#define ZYNOS_MODEL_P_660HW_67		ZYNOS_MODEL_P_660HW_63
#define ZYNOS_MODEL_P_660HW_D1		MID( 9114)
#define ZYNOS_MODEL_P_660HW_D3		MID(12698)

#define ZYNOS_MODEL_P_660R_61		MID(20882)
#define ZYNOS_MODEL_P_660R_61C		MID( 1178)
#define ZYNOS_MODEL_P_660R_63		MID(21138)
#define ZYNOS_MODEL_P_660R_63C		MID(  922)
#define ZYNOS_MODEL_P_660R_67		ZYNOS_MODEL_P_660R_63
#define ZYNOS_MODEL_P_660R_67C		/* n.a. */
#define ZYNOS_MODEL_P_660R_D1		MID( 7322)
#define ZYNOS_MODEL_P_660R_D3		MID(10138)

#define ZYNOS_MODEL_P_661H_61		MID(19346)
#define ZYNOS_MODEL_P_661H_63		MID( 1946)
#define ZYNOS_MODEL_P_661H_D1		MID(10650)
#define ZYNOS_MODEL_P_661H_D3		MID(12442)

#define ZYNOS_MODEL_P_661HW_61		ZYNOS_MODEL_P_661H_61
#define ZYNOS_MODEL_P_661HW_63		ZYNOS_MODEL_P_661H_63
#define ZYNOS_MODEL_P_661HW_D1		MID(10906)
#define ZYNOS_MODEL_P_661HW_D3		MID(14746)

#define ZYNOS_MODEL_P_662H_61		MID(22418)
#define ZYNOS_MODEL_P_662H_63		/* n.a. */
#define ZYNOS_MODEL_P_662H_67		/* n.a. */
#define ZYNOS_MODEL_P_662H_D1		/* n.a. */
#define ZYNOS_MODEL_P_662H_D3		/* n.a. */

#define ZYNOS_MODEL_P_662HW_61		/* n.a. */
#define ZYNOS_MODEL_P_662HW_63		MID(22674)
#define ZYNOS_MODEL_P_662HW_67		/* n.a. */
#define ZYNOS_MODEL_P_662HW_D1		MID(10394)
#define ZYNOS_MODEL_P_662HW_D3		MID(12954)

/* OEM boards */
#define ZYNOS_MODEL_O2SURF		ZYNOS_MODEL_P_2602HWN_D7A

/* Atheros AR2318 based boards */
#define ZYNOS_MODEL_NBG_318S		MID(59392)

/* Atheros AR71xx based boards */
#define ZYNOS_MODEL_NBG_460N		MID(61441)

#endif /* _ZYNOS_H */
