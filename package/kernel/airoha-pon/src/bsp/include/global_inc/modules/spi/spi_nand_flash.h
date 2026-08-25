 /***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

/*======================================================================================
 * MODULE NAME: spi
 * FILE NAME: spi_nand_flash.h
 * DATE: 2014/11/21
 * VERSION: 1.00
 * PURPOSE: To Provide SPI NAND Access interface.
 * NOTES:
 *
 * AUTHOR : Chuck Kuo         REVIEWED by
 *
 * FUNCTIONS  
 *      SPI_NAND_Flash_Init             To provide interface for SPI NAND init. 
 *      SPI_NAND_Flash_Get_Flash_Info   To get system current flash info. 
 *      SPI_NAND_Flash_Write_Nbyte      To provide interface for Write N Bytes into SPI NAND Flash. 
 *      SPI_NAND_Flash_Read_NByte       To provide interface for Read N Bytes from SPI NAND Flash. 
 *      SPI_NAND_Flash_Erase            To provide interface for Erase SPI NAND Flash. 
 *      SPI_NAND_Flash_Read_Byte        To provide interface for read 1 Bytes from SPI NAND Flash. 
 *      SPI_NAND_Flash_Read_DWord       To provide interface for read Double Word from SPI NAND Flash. 
 *
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 * Version 1.00 - Date 2014/11/21 By Chuck Kuo
 * ** This is the first versoin for creating to support the functions of
 *    current module.
 *
 *======================================================================================
 */

#ifndef __SPI_NAND_FLASH_H__
    #define __SPI_NAND_FLASH_H__

#ifndef FLASH_TABLE_OPEN
/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */
#include "asm/types.h"
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
#include <linux/mtd/rawnand.h>
#else
#include <linux/mtd/nand.h>
#endif
#endif
	
#else
#include <stdio.h>
#include <string.h>
#define u8 unsigned char
#define u32 unsigned int
#define u64 unsigned long
#define uint32_t u32
#define LINUX_VERSION_CODE 263213
#define KERNEL_VERSION(a,b,c) (((a)<<16)+((b)<<8)+(c))
#endif

#include "spi/nand_flash_otp.h"

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX				(32)
#define SPI_NAND_FLASH_THRESHOLD_VALUE_MAX				(3)
#define SPI_NAND_FLASH_EXT_DATA_MAX						(3)

#define UNKNOW_PAGE										~(0)
#define _SPI_NAND_PAGE_SIZE								(4096)
#define _SPI_NAND_OOB_SIZE								(256)
#define _SPI_NAND_CACHE_SIZE							(_SPI_NAND_PAGE_SIZE+_SPI_NAND_OOB_SIZE)

#if defined(TCSUPPORT_CPU_ARMV8) || defined(CONFIG_ECNT_UBOOT)
/* at GDMP SRAM address 16KB */
#define CACHE_ADDR_INFO_START_ADDR						(0x1FA40000)
#define SPI_NAND_CONTROLLER_ECC_INFO_SRAM_ADDR_OFFSET	(0x4000)
#else
#define CACHE_ADDR_INFO_START_ADDR						(0x9FA40000)
#define SPI_NAND_CONTROLLER_ECC_INFO_SRAM_ADDR_OFFSET	(0x00)
#endif

/* spi nand info, [31:16] = controllerECC ability, bits[15:0] = spare area size per sector */
#define SPI_NAND_CONTROLLER_ECC_INFO_SRAM_ADDR_INFO		(CACHE_ADDR_INFO_START_ADDR + 0x00)
#define SPI_NAND_CONTROLLER_ECC_INFO_FLASH_ADDR_INFO	(0x28)

#define MAX_SPI_NAND_FLASH_EXTEND_DEV_ID_LEN			(1)
#define _SPI_NAND_DUMMY_EXTEND_DEVICE_ID				{0, {0x0, 0x0, 0x0}}

/* SPI NAND Size Define */
#define _SPI_NAND_PAGE_SIZE_512				0x0200
#define _SPI_NAND_PAGE_SIZE_2KBYTE			0x0800	
#define _SPI_NAND_PAGE_SIZE_4KBYTE			0x1000
#define _SPI_NAND_OOB_SIZE_64BYTE			0x40
#define _SPI_NAND_OOB_SIZE_120BYTE			0x78
#define _SPI_NAND_OOB_SIZE_128BYTE			0x80
#define _SPI_NAND_OOB_SIZE_224BYTE			0xE0
#define _SPI_NAND_OOB_SIZE_256BYTE			0x100
#define _SPI_NAND_BLOCK_SIZE_128KBYTE		0x20000
#define _SPI_NAND_BLOCK_SIZE_256KBYTE		0x40000
#define _SPI_NAND_BLOCK_SIZE_512KBYTE		0x80000
#define _SPI_NAND_CHIP_SIZE_512MBIT			0x04000000
#define _SPI_NAND_CHIP_SIZE_1GBIT			0x08000000
#define _SPI_NAND_CHIP_SIZE_2GBIT			0x10000000
#define _SPI_NAND_CHIP_SIZE_4GBIT			0x20000000
#define _SPI_NAND_CHIP_SIZE_8GBIT			0x40000000
#define _SPI_NAND_CHIP_SIZE_16GBIT			0x80000000

/* SPI NAND Manufacturers ID */
#define _SPI_NAND_MANUFACTURER_ID_GIGADEVICE	0xC8
#define _SPI_NAND_MANUFACTURER_ID_WINBOND		0xEF
#define _SPI_NAND_MANUFACTURER_ID_ESMT			0xC8
#define _SPI_NAND_MANUFACTURER_ID_MXIC			0xC2
#define _SPI_NAND_MANUFACTURER_ID_ZENTEL		0xC8
#define _SPI_NAND_MANUFACTURER_ID_ETRON			0xD5
#define _SPI_NAND_MANUFACTURER_ID_TOSHIBA		0x98
#define _SPI_NAND_MANUFACTURER_ID_MICRON		0x2C
#define _SPI_NAND_MANUFACTURER_ID_HEYANG		0xC9
#define _SPI_NAND_MANUFACTURER_ID_PN			0xA1
#define _SPI_NAND_MANUFACTURER_ID_ATO			0x9B
#define _SPI_NAND_MANUFACTURER_ID_FM			0xA1
#define _SPI_NAND_MANUFACTURER_ID_XTX			0x0B
#define _SPI_NAND_MANUFACTURER_ID_MIRA			0xC8
#define _SPI_NAND_MANUFACTURER_ID_BIWIN			0xBC
#define _SPI_NAND_MANUFACTURER_ID_FORESEE		0xCD
#define _SPI_NAND_MANUFACTURER_ID_SAMSUNG		0xEC
#define _SPI_NAND_MANUFACTURER_ID_SKYHIGH		0x01

/* SPI NAND Device ID */
#define _SPI_NAND_DEVICE_ID_GD5E1GQ4UBYIG	0x11
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UAYIG	0xF1
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UBYIG	0xD1
#define _SPI_NAND_DEVICE_ID_GD5F1GQ4UCYIG	0xB1
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UBYIG	0xD2
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UE9IS	0xD5
#define _SPI_NAND_DEVICE_ID_GD5F2GQ4UCYIG	0xB2
#define _SPI_NAND_DEVICE_ID_GD5F4GQ4UBYIG	0xD4
#define _SPI_NAND_DEVICE_ID_GD5F4GQ4UCYIG	0xB4
#define _SPI_NAND_DEVICE_ID_F50L512M41A		0x20
#define _SPI_NAND_DEVICE_ID_F50L1G41A0		0x21
#define _SPI_NAND_DEVICE_ID_F50L1G41LB		0x01
#define _SPI_NAND_DEVICE_ID_F50L2G41LB		0x0A
#define _SPI_NAND_DEVICE_ID_W25N01GV		0xAA
#define _SPI_NAND_DEVICE_ID_W25N01GV_EXTEND	{1, {0x21, 0, 0}}
#if 0
#define _SPI_NAND_DEVICE_ID_W25N01KV		0xAE
#define _SPI_NAND_DEVICE_ID_W25N01KV_EXTEND	{1, {0x21, 0, 0}}
#endif
#define _SPI_NAND_DEVICE_ID_W25N02KV		0xAA
#define _SPI_NAND_DEVICE_ID_W25N02KV_EXTEND	{1, {0x22, 0, 0}}
#define _SPI_NAND_DEVICE_ID_W25N04KV		0xAA
#define _SPI_NAND_DEVICE_ID_W25N04KV_EXTEND	{1, {0x23, 0, 0}}
#define _SPI_NAND_DEVICE_ID_W25M02GV		0xAB
#define _SPI_NAND_DEVICE_ID_W25N02GV_EXTEND	{1, {0x21, 0, 0}}
#define _SPI_NAND_DEVICE_ID_MXIC35LF1GE4AB	0x12
#define _SPI_NAND_DEVICE_ID_MXIC35LF2GE4AB	0x22
#define _SPI_NAND_DEVICE_ID_MXIC35LF2GE4AD  0x26
#define _SPI_NAND_DEVICE_ID_MXIC35LF2G14AC	0x20
#define _SPI_NAND_DEVICE_ID_MXIC35LF4GE4AB	0x32
#define _SPI_NAND_DEVICE_ID_MXIC35LF1G24AD	0x14
#define _SPI_NAND_DEVICE_ID_A5U12A21ASC		0x20
#define _SPI_NAND_DEVICE_ID_A5U1GA21BWS		0x21
#define _SPI_NAND_DEVICE_ID_EM73C044SNA		0x19
#define _SPI_NAND_DEVICE_ID_EM73C044SNB		0x11
#define _SPI_NAND_DEVICE_ID_EM73C044SND		0x1D
#define _SPI_NAND_DEVICE_ID_EM73C044SNF		0x09
#define _SPI_NAND_DEVICE_ID_EM73C044VCA		0x18
#define _SPI_NAND_DEVICE_ID_EM73C044VCD		0x1C
#define _SPI_NAND_DEVICE_ID_EM73D044SNA		0x12
#define _SPI_NAND_DEVICE_ID_EM73D044SNC		0x0A
#define _SPI_NAND_DEVICE_ID_EM73D044SND		0x1E
#define _SPI_NAND_DEVICE_ID_EM73D044SNF		0x10
#define _SPI_NAND_DEVICE_ID_EM73D044VCA		0x13
#define _SPI_NAND_DEVICE_ID_EM73D044VCB		0x14
#define _SPI_NAND_DEVICE_ID_EM73D044VCD		0x17
#define _SPI_NAND_DEVICE_ID_EM73D044VCG		0x1F
#define _SPI_NAND_DEVICE_ID_EM73D044VCH		0x1B
#define _SPI_NAND_DEVICE_ID_EM73E044SNA		0x03
#define _SPI_NAND_DEVICE_ID_EM73E044VCA		0x2C
#define _SPI_NAND_DEVICE_ID_EM73E044VCB		0x2F
#define _SPI_NAND_DEVICE_ID_EM73F044SNA		0x24
#define _SPI_NAND_DEVICE_ID_TC58CVG0S3H		0xC2
#define _SPI_NAND_DEVICE_ID_TC58CVG1S3H		0xCB
#define _SPI_NAND_DEVICE_ID_TC58CVG2S0H		0xCD
#define _SPI_NAND_DEVICE_ID_MT29F1G01		0x14
#define _SPI_NAND_DEVICE_ID_MT29F2G01		0x24
#define _SPI_NAND_DEVICE_ID_MT29F4G01AD		0x34
#define _SPI_NAND_DEVICE_ID_MT29F4G01		0x36
#define _SPI_NAND_DEVICE_ID_MT29F4G01_33V	0x34

#define _SPI_NAND_DEVICE_ID_HYF1GQ4UAACAE	0x51
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UAACAE	0x52
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UHCCAE	0x5A
#define _SPI_NAND_DEVICE_ID_HYF4GQ4UAACBE	0xD4
#define _SPI_NAND_DEVICE_ID_HYF1GQ4UDACAE	0x21
#define _SPI_NAND_DEVICE_ID_HYF2GQ4UDACAE	0x22
#define _SPI_NAND_DEVICE_ID_PN26G01AWSIUG	0xE1
#define _SPI_NAND_DEVICE_ID_PN26G02AWSIUG	0xE2
#define _SPI_NAND_DEVICE_ID_ATO25D1GA		0x12
#define _SPI_NAND_DEVICE_ID_FM25S01			0xA1
#define _SPI_NAND_DEVICE_ID_FM25G01B		0xD1
#define _SPI_NAND_DEVICE_ID_FM25G02B		0xD2
#define _SPI_NAND_DEVICE_ID_FM25G02			0xF2
#define _SPI_NAND_DEVICE_ID_XT26G02B		0xF2
#define _SPI_NAND_DEVICE_ID_XT26G01A		0xE1
#define _SPI_NAND_DEVICE_ID_XT26G02A		0xE2
#define _SPI_NAND_DEVICE_ID_PSU1GS20BN		0x21
#define _SPI_NAND_DEVICE_ID_BWJX08U			0xB1
#define _SPI_NAND_DEVICE_ID_BWET08U			0xB2
#define _SPI_NAND_DEVICE_ID_FS35ND01GD1F1	0xA1
#define _SPI_NAND_DEVICE_ID_FS35ND02GS2F1	0xA2
#define _SPI_NAND_DEVICE_ID_FS35ND02GD1F1	0xB2
#define _SPI_NAND_DEVICE_ID_S35ML02G3		0x25

/* SPI NAND register address of command set */
#define _SPI_NAND_ADDR_EXTEND_BFD			0x10	/* Address of extend ECC Bit Flip count Detection*/
#define _SPI_NAND_ADDR_ECC					0x90	/* Address of ECC Config */
#define _SPI_NAND_ADDR_PROTECTION			0xA0	/* Address of protection */
#define _SPI_NAND_ADDR_FEATURE				0xB0	/* Address of feature */
#define _SPI_NAND_ADDR_STATUS				0xC0	/* Address of status */
#define _SPI_NAND_ADDR_FEATURE_4			0xD0	/* Address of status 4 */
#define _SPI_NAND_ADDR_STATUS_5				0xE0	/* Address of status 5 */
#define _SPI_NAND_ADDR_STATUS_6				0xF0	/* Address of status 6 */

#define _SPI_NAND_CHECK_ECC_THROSHOLD_BY_FLASH	(1)
#define _SPI_NAND_CHECK_ECC_THROSHOLD_BY_REG	(2)
#define _SPI_NAND_CHECK_ECC_THROSHOLD_BY_2REG	(3)

/* TYPE DECLARATIONS ----------------------------------------------------------------- */
typedef enum{
	SPI_NAND_FLASH_READ_DUMMY_BYTE_PREPEND,
	SPI_NAND_FLASH_READ_DUMMY_BYTE_APPEND,
	
	SPI_NAND_FLASH_READ_DUMMY_BYTE_DEF_NO
	
} SPI_NAND_FLASH_READ_DUMMY_BYTE_T;

typedef enum{
	SPI_NAND_FLASH_RTN_NO_ERROR =0,
	SPI_NAND_FLASH_RTN_PROBE_ERROR,
	SPI_NAND_FLASH_RTN_ALIGNED_CHECK_FAIL,
	SPI_NAND_FLASH_RTN_DETECTED_BAD_BLOCK,
	SPI_NAND_FLASH_RTN_ERASE_FAIL,
	SPI_NAND_FLASH_RTN_PROGRAM_FAIL,
	SPI_NAND_FLASH_RTN_NFI_FAIL,
	SPI_NAND_FLASH_RTN_ECC_DECODE_FAIL,
	SPI_NAND_FLASH_RTN_ENABLE_ECC_FAIL,
	SPI_NAND_FLASH_RTN_DISABLE_ECC_FAIL,
	SPI_NAND_FLASH_RTN_TIMEOUT,
#if defined(TCSUPPORT_PARALLEL_NAND)
	SPI_NAND_FLASH_RTN_CMD_ABORT,
#endif
	SPI_NAND_FLASH_RTN_ECC_EXCEEDED_THRESHOLD,

	SPI_NAND_FLASH_RTN_DEF_NO
} SPI_NAND_FLASH_RTN_T;

typedef enum{
	SPI_NAND_FLASH_READ_SPEED_MODE_SINGLE =0,
	SPI_NAND_FLASH_READ_SPEED_MODE_DUAL,
	SPI_NAND_FLASH_READ_SPEED_MODE_QUAD,
	
	SPI_NAND_FLASH_READ_SPEED_MODE_DEF_NO	
} SPI_NAND_FLASH_READ_SPEED_MODE_T;


typedef enum{
	SPI_NAND_FLASH_WRITE_SPEED_MODE_SINGLE =0,
	SPI_NAND_FLASH_WRITE_SPEED_MODE_QUAD,
	
	SPI_NAND_FLASH_WRITE_SPEED_MODE_DEF_NO	
} SPI_NAND_FLASH_WRITE_SPEED_MODE_T;

typedef enum{
	SPI_NAND_FLASH_WRITE_LOAD_FIRST =0,
	SPI_NAND_FLASH_WRITE_EN_FIRST,
} SPI_NAND_FLASH_WRITE_EN_TYPE_T;

typedef enum{
	SPI_NAND_FLASH_DEBUG_LEVEL_0 =0,
	SPI_NAND_FLASH_DEBUG_LEVEL_1,
	SPI_NAND_FLASH_DEBUG_LEVEL_2,
	
	SPI_NAND_FLASH_DEBUG_LEVEL_DEF_NO	
} SPI_NAND_FLASH_DEBUG_LEVEL_T;

typedef enum{
	SPI_NAND_EXTEND_DEVICE_ID_UNMATCH =0,
	SPI_NAND_EXTEND_DEVICE_ID_MATCH,
} SPI_NAND_EXTEND_DEVICE_ID_STATUS_T;

typedef enum{
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MATCH = 0,
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_ECC_MISMATCH,
} UBIFS_BLANK_PAGE_ECC_T;

typedef enum{
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_SUCCESS = 0,
	SPI_NAND_FLASH_UBIFS_BLANK_PAGE_FIXUP_FAIL,
} UBIFS_BLANK_PAGE_FIXUP_T;

/* Bitwise */
#define SPI_NAND_FLASH_FEATURE_NONE			( 0x00 )
#define SPI_NAND_FLASH_PLANE_SELECT_HAVE	( 0x01 << 0 )
#define SPI_NAND_FLASH_DIE_SELECT_1_HAVE	( 0x01 << 1 )
#define SPI_NAND_FLASH_DIE_SELECT_2_HAVE	( 0x01 << 2 )
#define SPI_NAND_FLASH_NO_ECC_STATUS_HAVE	( 0x01 << 3 )
#define SPI_NAND_FLASH_DISABLE_CONTINUOUS_RD	( 0x01 << 4 )
#define SPI_NAND_FLASH_ERASE_STATISTICS			( 0x01 << 5 )
#define SPI_NAND_FLASH_BLANK_PAGE_FIXUP			( 0x01 << 6 )
#define SPI_NAND_FLASH_OOB_RESERVE_FOR_BMT		( 0x01 << 7 )
#define SPI_NAND_FLASH_READ_ECC_ERROR_BIT_CHECK	( 0x01 << 8 )

#if defined(TCSUPPORT_PARALLEL_NAND)
#define PARALLEL_NAND_FLASH_FEATURE_SHIFT	(16)
#define PARALLEL_NAND_FLASH_2CE				((0x01 << 0) << PARALLEL_NAND_FLASH_FEATURE_SHIFT)

#ifdef TCSUPPORT_CPU_ARMV8
/* 7581 change clk_apb from 112.5M to 150M. Need to modify flash timming setting and get setting from DE. */
#define PARALLEL_NAND_FLASH_TIMING			0x40044326
#else
#define PARALLEL_NAND_FLASH_TIMING			0x44333
#endif
#endif

#define READ_ECC_CHECK_ADDR_INDEX			(0)
#define READ_ECC_CHECK_EXT_DATA_INDEX		(1)
#define READ_ECC_CHECK_EXT_DATA2_INDEX		(2)

struct spi_nand_flash_oobfree{
	u32	offset;
	u32	len;
};

struct spi_nand_flash_ooblayout
{	
	unsigned int	oobsize;
	struct spi_nand_flash_oobfree oobfree[SPI_NAND_FLASH_OOB_FREE_ENTRY_MAX];
};	

struct SPI_NAND_ECC_FAIL_CHECK_INFO_T {
	u8	ecc_check_mask;
	u8	ecc_uncorrected_value;
};

struct SPI_NAND_UNLOCK_BLOCK_INFO_T {
	u8	unlock_block_mask;
	u8	unlock_block_value;
};

struct SPI_NAND_QUAD_EN_INFO_T {
	u8	quad_en_mask;
	u8	quad_en_value;
};

struct SPI_NAND_ECC_EN_INFO_T {
	u8	ecc_en_addr;
	u8	ecc_en_mask;
	u8	ecc_en_value;
};

struct SPI_NAND_EXTEND_ID_T {
	const u8	extend_len;
	const u8	extend_id[3];
};

struct SPI_NAND_READ_ECC_CHECK_T {
	const u8	type;
	const u8	ecc_threhsold_value[SPI_NAND_FLASH_THRESHOLD_VALUE_MAX];
	const u8	ext_data[SPI_NAND_FLASH_EXT_DATA_MAX];
};

struct SPI_NAND_FLASH_INFO_T {
#if defined(TCSUPPORT_BL2_OPTIMIZATION) && defined(IMAGE_BL2)
	u8								mfr_id;
	u8								dev_id;
	const u8								*ptr_name;								
#else
	const u8								mfr_id;
	const u8								dev_id;
	const u8								*ptr_name;
#endif
	u32										device_size;	/* Flash total Size */
	u32										page_size;		/* Page Size 		*/
	u32										erase_size;		/* Block Size 		*/
	u32										oob_size;		/* Spare Area (OOB) Size */
	SPI_NAND_FLASH_READ_DUMMY_BYTE_T		dummy_mode;
	SPI_NAND_FLASH_READ_SPEED_MODE_T		read_mode;
	struct spi_nand_flash_ooblayout			*oob_free_layout;
	u8										die_num;
	SPI_NAND_FLASH_WRITE_SPEED_MODE_T		write_mode;
	u32										feature;
	struct SPI_NAND_ECC_FAIL_CHECK_INFO_T	ecc_fail_check_info;
	SPI_NAND_FLASH_WRITE_EN_TYPE_T			write_en_type;
	struct SPI_NAND_UNLOCK_BLOCK_INFO_T		unlock_block_info;
	struct SPI_NAND_QUAD_EN_INFO_T			quad_en;
	struct SPI_NAND_ECC_EN_INFO_T			ecc_en;
	char									otp_page_num;
#if defined(TCSUPPORT_PARALLEL_NAND)
	u32										ext_id;
	u32										timing_setting;
	u8										min_ecc_req;
	u8										addr_cycle;
#endif
	struct SPI_NAND_EXTEND_ID_T				extend_dev_id;
	u8										soc_ecc_ability;
	struct SPI_NAND_READ_ECC_CHECK_T		read_ecc_ceck;
};

#if defined(TCSUPPORT_BL2_OPTIMIZATION) && defined(IMAGE_BL2)
extern struct SPI_NAND_FLASH_INFO_T spi_nand_flash_tables[1];
#else
extern const struct SPI_NAND_FLASH_INFO_T spi_nand_flash_tables[];
#endif

#if defined(TCSUPPORT_BL2_OPTIMIZATION)

struct bl2_flash_H
{
	unsigned int flash_entry;
	unsigned int flash_name_off;
	unsigned int flash_oob_off;
	unsigned int parallel_entry;
	unsigned int parallel_name_off;
	unsigned int parallel_oob_off;
};
#endif


extern SPI_NAND_FLASH_RTN_T scan_spi_nand_table(struct SPI_NAND_FLASH_INFO_T *ptr_rtn_device_t);
extern struct spi_nand_flash_ooblayout ooblayout_spi_controller_ecc;

struct nand_info {
	int mfr_id;
	int dev_id;
	char *name;
	int numchips;
	int chip_shift;
	int page_shift;
	int erase_shift;
	int oob_shift;
	int badblockpos;
	int opcode_type;
};

struct ra_nand_chip {
	struct nand_info *flash;
};

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
	struct spinand_info {
	        struct nand_ecclayout	*ecclayout;
	        struct spi_device		*spi;
	        void					*priv;
	};

	struct spinand_state {
	        uint32_t        col;
	        uint32_t        row;
	        int             buf_idx;
	        u8              *buf;
			uint32_t		buf_len;
			int				oob_idx;
			u8 				*oob_buf;
			uint32_t		oob_buf_len;
			uint32_t		command;
	};
#ifndef FLASH_TABLE_OPEN
	struct en7512_spinand_host
	{
		struct nand_chip		nand_chip;
		struct mtd_info			mtd;
		struct nand_ecclayout	*ecclayout;
		struct spinand_state	state;
		void					*priv;
	};
#endif	
#endif

typedef struct
{
   unsigned short id;          
   unsigned int ext_id; 
   unsigned char  addr_cycle;
   unsigned char  iowidth;
   unsigned short totalsize;   
   unsigned short blocksize;
   unsigned short pagesize;
   unsigned int timmingsetting;
   char devciename[14];
   unsigned int advancedmode;   
}flashdev_info;




/* EXPORTED SUBPROGRAM SPECIFICATION ------------------------------------------------- */

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init( long  rom_base )
 * PURPOSE : To provide interface for SPI NAND init.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : rom_base - The rom_base variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Init( u32   rom_base );

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info( struct SPI_NAND_FLASH_INFO_T    *ptr_rtn_into_t )
 * PURPOSE : To get system current flash info.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: ptr_rtn_into_t  - A pointer to the structure of the ptr_rtn_into_t variable.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2015/01/14 by Chuck Kuo - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
#if !defined(BOOTROM_EXT)
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Get_Flash_Info( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t);

SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Set_Flash_Info( struct SPI_NAND_FLASH_INFO_T *ptr_rtn_into_t);
#endif

/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32    dst_addr,
 *                                                            u32    len,
 *                                                            u32    *ptr_rtn_len,
 *                                                            u8*    ptr_buf      )
 * PURPOSE : To provide interface for Write N Bytes into SPI NAND Flash.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *           buf      - The buf variable of this function.
 *   OUTPUT: rtn_len  - The rtn_len variable of this function.
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/15 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Write_Nbyte( u32								dst_addr,
                                                 u32    							len,
                                                 u32    							*ptr_rtn_len,
                                                 u8									*ptr_buf,      
                                                 SPI_NAND_FLASH_WRITE_SPEED_MODE_T 	speed_mode	);

/*------------------------------------------------------------------------------------
 * FUNCTION: int SPI_NAND_Flash_Read_NByte( long     addr,
 *                                          long     len,
 *                                          long     *retlen,
 *                                          char     *buf    )
 * PURPOSE : To provide interface for Read N Bytes from SPI NAND Flash.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr    - The addr variable of this function.
 *           len     - The len variable of this function.
 *           retlen  - The retlen variable of this function.
 *           buf     - The buf variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
u32 SPI_NAND_Flash_Read_NByte(	
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
								u64 							addr,
							#else
								u32 							addr,
							#endif
								u32								 len, 
								u32								 *retlen, 
								u8								 *buf, 
								SPI_NAND_FLASH_READ_SPEED_MODE_T speed_mode,
								SPI_NAND_FLASH_RTN_T *status);


/*------------------------------------------------------------------------------------
 * FUNCTION: SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase( u32  dst_addr,
 *                                                      u32  len      )
 * PURPOSE : To provide interface for Erase SPI NAND Flash.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : dst_addr - The dst_addr variable of this function.
 *           len      - The len variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/17 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Erase( u32  dst_addr,
                                           u32  len      );

/*------------------------------------------------------------------------------------
 * FUNCTION: char SPI_NAND_Flash_Read_Byte( long     addr )
 * PURPOSE : To provide interface for read 1 Bytes from SPI NAND Flash.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
unsigned char SPI_NAND_Flash_Read_Byte(
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
										u64 addr,
									#else
										u32 addr,
									#endif
										SPI_NAND_FLASH_RTN_T *status);


/*------------------------------------------------------------------------------------
 * FUNCTION: long SPI_NAND_Flash_Read_DWord( long    addr )
 * PURPOSE : To provide interface for read Double Word from SPI NAND Flash.
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : addr - The addr variable of this function.
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2014/12/12 by Chuck Kuo - The first revision for this function.
 *

 *------------------------------------------------------------------------------------
 */
unsigned long SPI_NAND_Flash_Read_DWord(
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30)
										u64 addr,
									#else
										u32 addr,
									#endif
										SPI_NAND_FLASH_RTN_T *status);

/*------------------------------------------------------------------------------------
 * FUNCTION: void SPI_NAND_Flash_Clear_Read_Cache_Data( void )
 * PURPOSE : To clear the cache data for read. 
 *           (The next time to read data will get data from flash chip certainly.)
 * AUTHOR  : Chuck Kuo
 * CALLED BY
 *   -
 * CALLS
 *   -
 * PARAMs  :
 *   INPUT : None
 *   OUTPUT: None
 * RETURN  : SPI_RTN_NO_ERROR - Successful.   Otherwise - Failed.
 * NOTES   :
 * MODIFICTION HISTORY:
 * Date 2015/01/21 by Chuck Kuo - The first revision for this function.
 *
 *------------------------------------------------------------------------------------
 */
void SPI_NAND_Flash_Clear_Read_Cache_Data( void );

void SPI_NAND_Flash_Set_DmaMode( u32 input );
void SPI_NAND_Flash_Get_DmaMode( u32 *val );
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Enable_OnDie_ECC( void );
SPI_NAND_FLASH_RTN_T SPI_NAND_Flash_Disable_OnDie_ECC( void );

void spi_nand_select_die (u32 page_number);
SPI_NAND_FLASH_RTN_T spi_nand_protocol_set_feature( u8 addr, u8 data );
SPI_NAND_FLASH_RTN_T spi_nand_protocol_get_feature( u8 addr, u8 *ptr_rtn_data );
SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_enable( void );
SPI_NAND_FLASH_RTN_T spi_nand_protocol_write_disable( void );
SPI_NAND_FLASH_RTN_T spi_nand_protocol_program_execute ( u32 addr );

void spi_nand_protocol_set_otp(NAND_FLASH_OTP_ENABLE_T enable);

#if	defined(TCSUPPORT_NAND_BMT)
#if (!defined(LZMA_IMG) && !defined(BOOTROM_EXT)) || defined(TCSUPPORT_BB_256KB)
int en7512_nand_exec_read_page(u32 page, u8* date, u8* oob);
int en7512_nand_check_block_bad(u32 offset, u32 bmt_block);
int en7512_nand_erase(u32 offset);
int en7512_nand_mark_badblock(u32 offset, u32 bmt_block);
int en7512_nand_exec_write_page(u32 page, u8 *dat, u8 *oob);
#endif
#endif

#endif /* ifndef __SPI_NAND_FLASH_H__ */
/* End of [spi_nand_flash.h] package */

