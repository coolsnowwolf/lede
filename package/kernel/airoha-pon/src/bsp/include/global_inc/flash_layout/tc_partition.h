/************************************************************************
 *
 *	Copyright (C) 2010 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
#ifndef TC_PARTITION_H
#define TC_PARTITION_H
#ifdef TCSUPPORT_MTD_ENCHANCEMENT
/*
note:		
	1.the read base address of reserve area need to compute by (flash total size -flash erase size * reverse block num) 
		-the base addrass can be obtained by mtd ioctl
		-for example :flash total size is 32 m,flash erase size is 64k,reverse block num is 1,so the read base address of 
		reserve area is 0x3fff0000
	2.reverse block num is according to compile option TCSUPPORT_RESERVEAREA_BLOCK
	TCSUPPORT_RESERVEAREA_BLOCK==1 --------reverse block num is 1
	TCSUPPORT_RESERVEAREA_BLOCK==2 --------reverse block num is 2
	TCSUPPORT_RESERVEAREA_BLOCK==3 --------reverse block num is 3
	TCSUPPORT_RESERVEAREA_BLOCK==4 --------reverse block num is 4
	3.if you modify the size or offset of the certain sector or you add a new sector,you must modify the reserve area table in order to let
	other people be clear.
*/

#define TC_FLASH_READ_CMD		"/userfs/bin/mtd readflash %s %lu %lu %s"
#define TC_FLASH_WRITE_CMD	"/userfs/bin/mtd writeflash %s %lu %lu %s"

#define TC_FLASH_ERASE_SECTOR_CMD "/userfs/bin/mtd erasesector %lu %s"

#define RESERVEAREA_NAME "reservearea"

#if defined(TCSUPPORT_MT7570)
    #define BOB_CAL_LEN 0xE1
#else
    #define BOB_CAL_LEN 0x400
#endif

#ifdef TCSUPPORT_MT7570
#if defined (TCSUPPORT_WLAN_MT7615_11N) || defined (TCSUPPORT_DUAL_WLAN_MT7615E) || defined (TCSUPPORT_WLAN_MT7615D)
#ifdef TCSUPPORT_NAND_BMT
#define BOB_RA_OFFSET_OLD_MODE  656896
#else 
#define BOB_RA_OFFSET_OLD_MODE  329216
#endif
#else 
#if (TCSUPPORT_RESERVEAREA_BLOCK==4)
#define BOB_RA_OFFSET_OLD_MODE 198144
#else
#define BOB_RA_OFFSET_OLD_MODE  1536
#endif
#endif
#endif


#define NAND_FLASH_BLOCK_SIZE (0x20000)  //this define can be changed baccording to type of nandflash (0x10000/0x20000/0x40000)

#if defined(TCSUPPORT_OPENWRT) || defined(RDKB_BUILD)

#ifdef TCSUPPORT_NAND_FLASH
#define RESERVEAREA_ERASE_SIZE 0x40000 //this define should be changed baccording to  flash erase size
#endif
#define RESERVEAREA_BLOCK_BASE 0
/*
---------------------------------------------------------------------------------
@reserve area table 6 BLOCK@
|sector		name						cover area				note
|1.			openwrt reserve				0~0x3fbff				255k	
|1.2        qdmadscp.conf               0x3fc00~0x3ffff		    1k																
|2.			Wi-Fi 6G cali				0x40000~0x23ffff		256*8k = 2M
|3.1		18.06 2.4 cali				0x240000~0x240fff			4k
|3.2		18.06 5G cali				0x241000~0x241fff			4k
|3.3		NULL 						0x242000~0x25ffff			120k
//------------------for openwrt version > 18.06
|3.4		bob          		        0x250000~0x27ffff			128k
|4			2.4G cali					0x280000~0x2bffff			256k
|5			5G cali						0x2c0000~0x2fffff			256k
|6			prolinecmd					0x300000~0x33ffff			256k
|7			bootflag      				0x340000~0x37ffff			256k
------------------------------------------------------------------------------------
*/

#define RESERVEAREA_TOTAL_SIZE RESERVEAREA_ERASE_SIZE*TCSUPPORT_RESERVEAREA_BLOCK

/*openwrt reserver 256k*/
#define OPENWRT_RESERVE_SIZE RESERVEAREA_ERASE_SIZE
#define OPENWRT_RESERVE_OFFSET RESERVEAREA_BLOCK_BASE

/*qdma dscp info*/
#define QDAMDSCP_RA_SIZE 0x400
#define QDAMDSCP_RA_OFFSET  (EEPROM_WIFI_6G_OFFSET-QDAMDSCP_RA_SIZE)

/*Wi-Fi 6G part*/
#define EEPROM_WIFI_6G_SIZE (RESERVEAREA_ERASE_SIZE * 8)
#define EEPROM_WIFI_6G_OFFSET (OPENWRT_18_06_CALI_OFFSET - EEPROM_WIFI_6G_SIZE)

/*openwrt 2.4G,5G,NULL for 18.06*/
#define OPENWRT_18_06_CALI_SIZE 0x20000
#define  OPENWRT_18_06_CALI_OFFSET EEPROM_791511N_RA_OFFSET-RESERVEAREA_ERASE_SIZE
/*bob info*/
#define BOB_RA_SIZE 0x20000
#define BOB_RA_OFFSET  (OPENWRT_18_06_CALI_OFFSET+OPENWRT_18_06_CALI_SIZE)

/*2.4G part*/
#define EEPROM_791511N_RA_SIZE RESERVEAREA_ERASE_SIZE
#define EEPROM_791511N_RA_OFFSET (EEPROM_RA_791511AC_OFFSET-RESERVEAREA_ERASE_SIZE)

/*7915 5G part*/
#define EEPROM_791511AC_RA_SIZE RESERVEAREA_ERASE_SIZE    
#define EEPROM_RA_791511AC_OFFSET (PROLINE_CWMPPARA_RA_OFFSET-RESERVEAREA_ERASE_SIZE )

/*prolinecmd part*/
#define PROLINE_CWMPPARA_RA_SIZE RESERVEAREA_ERASE_SIZE
#define PROLINE_CWMPPARA_RA_OFFSET (IMG_BOOT_FLAG_OFFSET-RESERVEAREA_ERASE_SIZE)

/*image boot flag*/
#define IMG_BOOT_FLAG_SIZE 	1
#define IMG_BOOT_FLAG_OFFSET  	(RESERVEAREA_TOTAL_SIZE-RESERVEAREA_ERASE_SIZE)

#endif
#endif
#ifndef TCSUPPORT_RESERVEAREA_EXTEND
#define IMG_BOOT_FLAG_SIZE 	1
#if defined(TCSUPPORT_CT)
#define IMG_BOOT_FLAG_OFFSET  	(CERM1_RA_OFFSET - IMG_BOOT_FLAG_SIZE)
#else
#define IMG_BOOT_FLAG_OFFSET  	(MRD_RA_OFFSET - IMG_BOOT_FLAG_SIZE)
#endif
#endif

#endif
