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

#ifndef _MYLOADER_H_
#define _MYLOADER_H_

/* Myloader specific magic numbers */
#define MYLO_MAGIC_SYS_PARAMS	0x20021107
#define MYLO_MAGIC_PARTITIONS	0x20021103
#define MYLO_MAGIC_BOARD_PARAMS	0x20021103

/* Vendor ID's (seems to be same as the PCI vendor ID's) */
#define VENID_COMPEX		0x11F6

/* Devices based on the ADM5120 */
#define DEVID_COMPEX_NP27G	0x0078
#define DEVID_COMPEX_NP28G	0x044C
#define DEVID_COMPEX_NP28GHS	0x044E
#define DEVID_COMPEX_WP54Gv1C	0x0514
#define DEVID_COMPEX_WP54G	0x0515
#define DEVID_COMPEX_WP54AG	0x0546
#define DEVID_COMPEX_WPP54AG	0x0550
#define DEVID_COMPEX_WPP54G	0x0555

/* Devices based on the Atheros AR2317 */
#define DEVID_COMPEX_NP25G	0x05E6
#define DEVID_COMPEX_WPE53G	0x05DC

/* Devices based on the Atheros AR71xx */
#define DEVID_COMPEX_WP543	0x0640
#define DEVID_COMPEX_WPE72	0x0672

/* Devices based on the IXP422 */
#define DEVID_COMPEX_WP18	0x047E
#define DEVID_COMPEX_NP18A	0x0489

/* Other devices */
#define DEVID_COMPEX_NP26G8M	0x03E8
#define DEVID_COMPEX_NP26G16M	0x03E9

struct mylo_partition {
	uint16_t	flags;	/* partition flags */
	uint16_t	type;	/* type of the partition */
	uint32_t	addr;	/* relative address of the partition from the
				   flash start */
	uint32_t	size;	/* size of the partition in bytes */
	uint32_t	param;	/* if this is the active partition, the
				   MyLoader load code to this address */
};

#define PARTITION_FLAG_ACTIVE	0x8000 /* this is the active partition,
					* MyLoader loads firmware from here */
#define PARTITION_FLAG_ISRAM	0x2000 /* FIXME: this is a RAM partition? */
#define PARTIIION_FLAG_RAMLOAD	0x1000 /* FIXME: load this partition into the RAM? */
#define PARTITION_FLAG_PRELOAD	0x0800 /* the partition data preloaded to RAM
					* before decompression */
#define PARTITION_FLAG_LZMA	0x0100 /* partition data compressed by LZMA */
#define PARTITION_FLAG_HAVEHDR  0x0002 /* the partition data have a header */

#define PARTITION_TYPE_FREE	0
#define PARTITION_TYPE_USED	1

#define MYLO_MAX_PARTITIONS	8	/* maximum number of partitions in the
					   partition table */

struct mylo_partition_table {
	uint32_t	magic;		/* must be MYLO_MAGIC_PARTITIONS */
	uint32_t	res0;		/* unknown/unused */
	uint32_t	res1;		/* unknown/unused */
	uint32_t 	res2;		/* unknown/unused */
	struct mylo_partition partitions[MYLO_MAX_PARTITIONS];
};

struct mylo_partition_header {
	uint32_t	len;		/* length of the partition data */
	uint32_t	crc;		/* CRC value of the partition data */
};

struct mylo_system_params {
	uint32_t	magic;		/* must be MYLO_MAGIC_SYS_PARAMS */
	uint32_t	res0;
	uint32_t	res1;
	uint32_t	mylo_ver;
	uint16_t	vid;		/* Vendor ID */
	uint16_t	did;		/* Device ID */
	uint16_t	svid;		/* Sub Vendor ID */
	uint16_t	sdid;		/* Sub Device ID */
	uint32_t	rev;		/* device revision */
	uint32_t	fwhi;
	uint32_t	fwlo;
	uint32_t	tftp_addr;
	uint32_t	prog_start;
	uint32_t	flash_size;	/* size of boot FLASH in bytes */
	uint32_t	dram_size;	/* size of onboard RAM in bytes */
};

struct mylo_eth_addr {
	uint8_t	mac[6];
	uint8_t	csum[2];
};

#define MYLO_ETHADDR_COUNT	8	/* maximum number of ethernet address
					   in the board parameters */

struct mylo_board_params {
	uint32_t	magic;	/* must be MYLO_MAGIC_BOARD_PARAMS */
	uint32_t	res0;
	uint32_t	res1;
	uint32_t	res2;
	struct mylo_eth_addr addr[MYLO_ETHADDR_COUNT];
};

#endif /* _MYLOADER_H_*/
