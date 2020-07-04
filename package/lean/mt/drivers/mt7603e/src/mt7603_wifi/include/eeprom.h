/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	eeprom.h

	Abstract:
	Miniport header file for eeprom related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

#ifdef MT_MAC
#include "eeprom/mt_e2p_def.h"
#endif

/* For ioctl check usage */
#define EEPROM_IS_PROGRAMMED		0x80

#define E2P_NONE					0x00
#define E2P_EFUSE_MODE				0x01
#define E2P_FLASH_MODE				0x02
#define E2P_EEPROM_MODE			0x03
#define E2P_BIN_MODE				0x04
#define NUM_OF_E2P_MODE			0x05

#ifdef RTMP_MAC_PCI
#define MAX_EEPROM_BIN_FILE_SIZE	512
#else
#define MAX_EEPROM_BIN_FILE_SIZE	1024
#endif /* !RTMP_MAC_PCI */

#define EEPROM_SIZE					0x200

#define EEPROM_DEFULT_BIN_FILE	"RT30xxEEPROM.bin"
#ifdef BB_SOC
#define BIN_FILE_PATH				"/etc/RT30xxEEPROM.bin"
#else
//#define BIN_FILE_PATH				"/tmp/RT30xxEEPROM.bin"
#define BIN_FILE_PATH				"/etc/RT30xxEEPROM.bin"
#endif /* BB_SOC */

#define EEPROM_1ST_FILE_DIR		"/etc_ro/Wireless/iNIC/"
#define EEPROM_2ND_FILE_DIR		"/etc_ro/Wireless/iNIC/"

#ifndef EEPROM_DEFAULT_FILE_PATH
/* RFIC 2820 */
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT2880_RT2820_AP_2T3R_V1_6.bin"
#endif /* EEPROM_DEFAULT_FILE_PATH */



#ifdef RTMP_FLASH_SUPPORT
#define NVRAM_OFFSET				0x30000

#if defined (CONFIG_RT2880_FLASH_32M)
#define DEFAULT_RF_OFFSET					0x1FE0000
#else
#define DEFAULT_RF_OFFSET					0x40000
#endif

#define SECOND_RF_OFFSET					0x48000
#endif /* RTMP_FLASH_SUPPORT */

typedef struct eeprom_flash_info_t {
	UINT32	offset;
	UINT32	size;

	RTMP_STRING	bin_name[L1PROFILE_ATTRNAME_LEN];

} eeprom_flash_info;


#ifdef RT_BIG_ENDIAN
typedef	union _EEPROM_WORD_STRUC {
	struct {
		UCHAR	Byte1;				// High Byte
		UCHAR	Byte0;				// Low Byte
	} field;
	USHORT	word;
} EEPROM_WORD_STRUC;
#else
typedef	union _EEPROM_WORD_STRUC {
	struct {
		UCHAR	Byte0;
		UCHAR	Byte1;
	} field;
	USHORT	word;
} EEPROM_WORD_STRUC;
#endif


/* ------------------------------------------------------------------- */
/*  E2PROM data layout */
/* ------------------------------------------------------------------- */

/* Board type */

#define BOARD_TYPE_MINI_CARD		0	/* Mini card */
#define BOARD_TYPE_USB_PEN		1	/* USB pen */

/*
	EEPROM antenna select format
*/

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
		USHORT CoexBit:1;
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MSidebandForA:1;
		USHORT BW40MSidebandForG:1;
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 5G */
		USHORT ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT HardwareRadioControl:1;	/* Whether RF is controlled by driver or HW. 1:enable hw control, 0:disable */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#else
typedef union _EEPROM_NIC_CINFIG2_STRUC {
	struct {
		USHORT HardwareRadioControl:1;	/* 1:enable, 0:disable */
		USHORT DynamicTxAgcControl:1;	/* */
		USHORT ExternalLNAForG:1;	/* external LNA enable for 2.4G */
		USHORT ExternalLNAForA:1;	/* external LNA enable for 5G */
		USHORT CardbusAcceleration:1;	/* !!! NOTE: 0 - enable, 1 - disable */
		USHORT BW40MSidebandForG:1;
		USHORT BW40MSidebandForA:1;
		USHORT EnableWPSPBC:1;	/* WPS PBC Control bit */
		USHORT BW40MAvailForG:1;	/* 0:enable, 1:disable */
		USHORT BW40MAvailForA:1;	/* 0:enable, 1:disable */
		USHORT Rsv1:1;	/* must be 0 */
		USHORT AntDiversity:1;	/* Antenna diversity */
		USHORT AntOpt:1;	/* Fix Antenna Option: 0:Main; 1: Aux */
		USHORT bInternalTxALC:1;	/* Internal Tx ALC */
		USHORT CoexBit:1;
		USHORT DACTestBit:1;	/* control if driver should patch the DAC issue */
	} field;
	USHORT word;
} EEPROM_NIC_CONFIG2_STRUC, *PEEPROM_NIC_CONFIG2_STRUC;
#endif





/*
	TX_PWR Value valid range 0xFA(-6) ~ 0x24(36)
*/
#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte1;	/* High Byte */
		signed char Byte0;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#else
typedef union _EEPROM_TX_PWR_STRUC {
	struct {
		signed char Byte0;	/* Low Byte */
		signed char Byte1;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_TX_PWR_STRUC, *PEEPROM_TX_PWR_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR Version;	/* High Byte */
		UCHAR FaeReleaseNumber;	/* Low Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#else
typedef union _EEPROM_VERSION_STRUC {
	struct {
		UCHAR FaeReleaseNumber;	/* Low Byte */
		UCHAR Version;	/* High Byte */
	} field;
	USHORT word;
} EEPROM_VERSION_STRUC, *PEEPROM_VERSION_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT Rsvd:3;	/* Reserved */
		USHORT LedMode:5;	/* Led mode. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#else
typedef union _EEPROM_LED_STRUC {
	struct {
		USHORT PolarityRDY_G:1;	/* Polarity RDY_G setting. */
		USHORT PolarityRDY_A:1;	/* Polarity RDY_A setting. */
		USHORT PolarityACT:1;	/* Polarity ACT setting. */
		USHORT PolarityGPIO_0:1;	/* Polarity GPIO#0 setting. */
		USHORT PolarityGPIO_1:1;	/* Polarity GPIO#1 setting. */
		USHORT PolarityGPIO_2:1;	/* Polarity GPIO#2 setting. */
		USHORT PolarityGPIO_3:1;	/* Polarity GPIO#3 setting. */
		USHORT PolarityGPIO_4:1;	/* Polarity GPIO#4 setting. */
		USHORT LedMode:5;	/* Led mode. */
		USHORT Rsvd:3;	/* Reserved */
	} field;
	USHORT word;
} EEPROM_LED_STRUC, *PEEPROM_LED_STRUC;
#endif

#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR TxPowerEnable:1;	/* Enable */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#else
typedef union _EEPROM_TXPOWER_DELTA_STRUC {
	struct {
		UCHAR DeltaValue:6;	/* Tx Power dalta value (MAX=4) */
		UCHAR Type:1;	/* 1: plus the delta value, 0: minus the delta value */
		UCHAR TxPowerEnable:1;	/* Enable */
	} field;
	UCHAR value;
} EEPROM_TXPOWER_DELTA_STRUC, *PEEPROM_TXPOWER_DELTA_STRUC;
#endif


#ifdef RT_BIG_ENDIAN
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte1;	/* High Byte */
		UCHAR	Byte0;	/* Low Byte */
	} field;
	
	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#else
typedef union _EEPROM_TX_PWR_OFFSET_STRUC
{
	struct
	{
		UCHAR	Byte0;	/* Low Byte */
		UCHAR	Byte1;	/* High Byte */
	} field;

	USHORT		word;
} EEPROM_TX_PWR_OFFSET_STRUC, *PEEPROM_TX_PWR_OFFSET_STRUC;
#endif /* RT_BIG_ENDIAN */

#define NIC_CONFIGURE_0 0x34
#define EXTERNAL_PA_MASK (0x3 << 8)
#define GET_PA_TYPE(p) (((p) & EXTERNAL_PA_MASK) >> 8)

#define NIC_CONFIGURE_1 0x36
#define INTERNAL_TX_ALC_EN (1 << 13)

#define XTAL_TRIM1 0x3A
#define XTAL_TRIM1_DIP_SELECTION (1 << 7)
#define XTAL_TRIM1_MASK (0x7F)

#define G_BAND_20_40_BW_PWR_DELTA 0x50
#define G_BAND_20_40_BW_PWR_DELTA_MASK (0x3f)
#define G_BAND_20_40_BW_PWR_DELTA_SIGN (1 << 6)
#define G_BAND_20_40_BW_PWR_DELTA_EN (1 << 7)
#define A_BAND_20_40_BW_PWR_DELTA_MASK (0x3f << 8)
#define A_BAND_20_40_BW_PWR_DELTA_SIGN (1 << 14)
#define A_BAND_20_40_BW_PWR_DELTA_EN (1 << 15)

enum EEPROM_STORAGE_TYPE{
	EEPROM_PROM = 0,
	EEPROM_EFUSE = 1,
	EEPROM_FLASH = 2,
};

#define A_BAND_20_80_BW_PWR_DELTA 0x52
#define A_BAND_20_80_BW_PWR_DELTA_MASK (0x3f)
#define A_BAND_20_80_BW_PWR_DELTA_SIGN (1 << 6)
#define A_BAND_20_80_BW_PWR_DELTA_EN (1 << 7)
#define G_BAND_EXT_PA_SETTING_MASK (0x7f << 8)
#define G_BAND_EXT_PA_SETTING_EN (1 << 15)

#define A_BAND_EXT_PA_SETTING 0x54
#define A_BAND_EXT_PA_SETTING_MASK (0x7f)
#define A_BAND_EXT_PA_SETTING_EN (1 << 7)
#define TEMP_SENSOR_CAL_MASK (0x7f << 8)
#define TEMP_SENSOR_CAL_EN (1 << 15)

#define TX0_G_BAND_TSSI_SLOPE 0x56
#define TX0_G_BAND_TSSI_SLOPE_MASK (0xff)
#define TX0_G_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define TX0_G_BAND_TARGET_PWR 0x58
#define TX0_G_BAND_TARGET_PWR_MASK (0xff)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define TX0_G_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define TX0_G_BAND_CHL_PWR_DELTA_MID 0x5A
#define TX0_G_BAND_CHL_PWR_DELTA_MID_MASK (0x3f)
#define TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN (1 << 6)
#define TX0_G_BAND_CHL_PWR_DELTA_MID_EN (1 << 7)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define TX0_G_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define TX1_G_BAND_TSSI_SLOPE 0x5C
#define TX1_G_BAND_TSSI_SLOPE_MASK (0xff)
#define TX1_G_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define TX1_G_BAND_TARGET_PWR 0x5E
#define TX1_G_BAND_TARGET_PWR_MASK (0xff)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define TX1_G_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define TX1_G_BAND_CHL_PWR_DELTA_MID 0x60
#define TX1_G_BAND_CHL_PWR_DELTA_MID_MASK (0x3f)
#define TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN (1 << 6)
#define TX1_G_BAND_CHL_PWR_DELTA_MID_EN (1 << 7)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define TX1_G_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP0_TX0_A_BAND_TSSI_SLOPE 0x62
#define GRP0_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP0_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP0_TX0_A_BAND_TARGET_PWR 0x64
#define GRP0_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI 0x66
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP1_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP1_TX0_A_BAND_TSSI_OFFSET 0x68
#define GRP1_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP1_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW 0x6A
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP2_TX0_A_BAND_TSSI_SLOPE 0x6C
#define GRP2_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP2_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP2_TX0_A_BAND_TARGET_PWR 0x6E
#define GRP2_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI 0x70
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP3_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP3_TX0_A_BAND_TSSI_OFFSET 0x72
#define GRP3_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP3_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW 0x74
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP4_TX0_A_BAND_TSSI_SLOPE 0x76
#define GRP4_TX0_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP4_TX0_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP4_TX0_A_BAND_TARGET_PWR 0x78
#define GRP4_TX0_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI 0x7A
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP5_TX0_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP5_TX0_A_BAND_TSSI_OFFSET 0x7C
#define GRP5_TX0_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP5_TX0_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW 0X7E
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP0_TX1_A_BAND_TSSI_SLOPE 0x80
#define GRP0_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP0_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP0_TX1_A_BAND_TARGET_PWR 0x82
#define GRP0_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI 0x84
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP1_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP1_TX1_A_BAND_TSSI_OFFSET 0x86
#define GRP1_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP1_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x88
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP2_TX1_A_BAND_TSSI_SLOPE 0x8A
#define GRP2_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP2_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP2_TX1_A_BAND_TARGET_PWR 0x8C
#define GRP2_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI 0x8E
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP3_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP3_TX1_A_BAND_TSSI_OFFSET 0x90
#define GRP3_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP3_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x92
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define GRP4_TX1_A_BAND_TSSI_SLOPE 0x94
#define GRP4_TX1_A_BAND_TSSI_SLOPE_MASK (0xff)
#define GRP4_TX1_A_BAND_TSSI_OFFSET_MASK (0xff << 8)

#define GRP4_TX1_A_BAND_TARGET_PWR 0x96
#define GRP4_TX1_A_BAND_TARGET_PWR_MASK (0xff)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f << 8)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 14)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 15)

#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI 0x98
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 6)
#define GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 7)
#define GRP5_TX1_A_BAND_TSSI_SLOPE_MASK (0xff << 8)

#define GRP5_TX1_A_BAND_TSSI_OFFSET 0x9A
#define GRP5_TX1_A_BAND_TSSI_OFFSET_MASK (0xff)
#define GRP5_TX1_A_BAND_TARGET_PWR_MASK (0xff << 8)

#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW 0x9C
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK (0x3f)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN (1 << 6)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN (1 << 7)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK (0x3f << 8)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN (1 << 14)
#define GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_EN (1 << 15)

#define G_BAND_BANDEDGE_PWR_BACK_OFF 0x9E
#define G_BAND_BANDEDGE_PWR_BACK_OFF_MASK (0x7f)
#define G_BAND_BANDEDGE_PWR_BACK_OFF_EN (1 << 7)
#define XTAL_TRIM2_MASK (0x7f << 8) 
#define XTAL_TRIM2_DIP_SELECTION (1 << 15)

#define TX_PWR_CCK_1_2M 0xA0
#define TX_PWR_CCK_1_2M_MASK (0x3f)
#define TX_PWR_CCK_1_2M_SIGN (1 << 6)
#define TX_PWR_CCK_1_2M_EN (1 << 7)
#define TX_PWR_CCK_5_11M_MASK (0x3f << 8)
#define TX_PWR_CCK_5_11M_SIGN (1 << 14)
#define TX_PWR_CCK_5_11M_EN (1 << 15)

#define TX_PWR_G_BAND_OFDM_6_9M 0xA2
#define TX_PWR_G_BAND_OFDM_6_9M_MASK (0x3f)
#define TX_PWR_G_BAND_OFDM_6_9M_SIGN (1 << 6)
#define TX_PWR_G_BAND_OFDM_6_9M_EN (1 << 7)
#define TX_PWR_G_BAND_OFDM_12_18M_MASK (0x3f << 8)
#define TX_PWR_G_BAND_OFDM_12_18M_SIGN (1 << 14)
#define TX_PWR_G_BAND_OFDM_12_18M_EN (1 << 15)

#define TX_PWR_G_BAND_OFDM_24_36M 0xA4
#define TX_PWR_G_BAND_OFDM_24_36M_MASK (0x3f)
#define TX_PWR_G_BAND_OFDM_24_36M_SIGN (1 << 6)
#define TX_PWR_G_BAND_OFDM_24_36M_EN (1 << 7)
#define TX_PWR_G_BAND_OFDM_48_54M_MASK (0x3f << 8)
#define TX_PWR_G_BAND_OFDM_48_54M_SIGN (1 << 14)
#define TX_PWR_G_BAND_OFDM_48_54M_EN (1 << 15)

#define TX_PWR_HT_MCS_0_1 0xA6
#define TX_PWR_HT_MCS_0_1_MASK (0x3f)
#define TX_PWR_HT_MCS_0_1_SIGN (1 << 6)
#define TX_PWR_HT_MCS_0_1_EN (1 << 7)
#define TX_PWR_HT_MCS_2_3_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_2_3_SIGN (1 << 14)
#define TX_PWR_HT_MCS_2_3_EN (1 << 15)

#define TX_PWR_HT_MCS_4_5 0xA8
#define TX_PWR_HT_MCS_4_5_MASK (0x3f)
#define TX_PWR_HT_MCS_4_5_SIGN (1 << 6)
#define TX_PWR_HT_MCS_4_5_EN (1 << 7)
#define TX_PWR_HT_MCS_6_7_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_6_7_SIGN (1 << 14)
#define TX_PWR_HT_MCS_6_7_EN (1 << 15)

#define TX_PWR_HT_MCS_8_9 0xAA
#define TX_PWR_HT_MCS_8_9_MASK (0x3f)
#define TX_PWR_HT_MCS_8_9_SIGN (1 << 6)
#define TX_PWR_HT_MCS_8_9_EN (1 << 7)
#define TX_PWR_HT_MCS_10_11_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_10_11_SIGN (1 << 14)
#define TX_PWR_HT_MCS_10_11_EN (1 << 15)

#define TX_PWR_HT_MCS_12_13 0xAC
#define TX_PWR_HT_MCS_12_13_MASK (0x3f)
#define TX_PWR_HT_MCS_12_13_SIGN (1 << 6)
#define TX_PWR_HT_MCS_12_13_EN (1 << 7)
#define TX_PWR_HT_MCS_14_15_MASK (0x3f << 8)
#define TX_PWR_HT_MCS_14_15_SIGN (1 << 14)
#define TX_PWR_HT_MCS_14_15_EN (1 << 15)

#define CONFIG_G_BAND_CHL 0xB0
#define CONFIG_G_BAND_CHL_GRP1_MASK (0xff)
#define CONFIG_G_BAND_CHL_GRP2_MASK (0xff << 8)

#define TX_PWR_A_BAND_OFDM_6_9M 0xB2
#define TX_PWR_A_BAND_OFDM_6_9M_MASK (0x3f)
#define TX_PWR_A_BAND_OFDM_6_9M_SIGN (1 << 6)
#define TX_PWR_A_BAND_OFDM_6_9M_EN (1 << 7)
#define TX_PWR_A_BAND_OFDM_12_18M_MASK (0x3f << 8)
#define TX_PWR_A_BAND_OFDM_12_18M_SIGN (1 << 14)
#define TX_PWR_A_BAND_OFDM_12_18M_EN (1 << 15)

#define TX_PWR_A_BAND_OFDM_24_36M 0xB4
#define TX_PWR_A_BAND_OFDM_24_36M_MASK (0x3f)
#define TX_PWR_A_BAND_OFDM_24_36M_SIGN (1 << 6)
#define TX_PWR_A_BAND_OFDM_24_36M_EN (1 << 7)
#define TX_PWR_A_BAND_OFDM_48_54M_MASK (0x3f << 8)
#define TX_PWR_A_BAND_OFDM_48_54M_SIGN (1 << 14)
#define TX_PWR_A_BAND_OFDM_48_54M_EN (1 << 15)

#define CONFIG1_A_BAND_CHL 0xB6
#define CONFIG1_A_BAND_CHL_GRP1_MASK 0xff
#define CONFIG1_A_BAND_CHL_GRP2_MASK (0xff << 8)

#define CONFIG2_A_BAND_CHL 0xB8
#define CONFIG2_A_BAND_CHL_GRP1_MASK (0xff)
#define CONFIG2_A_BAND_CHL_GRP2_MASK (0xff << 8)

#define TX_PWR_VHT_MCS_0_1 0xBA
#define TX_PWR_VHT_MCS_0_1_MASK (0x3f)
#define TX_PWR_VHT_MCS_0_1_SIGN (1 << 6)
#define TX_PWR_VHT_MCS_0_1_EN (1 << 7)
#define TX_PWR_VHT_MCS_2_3_MASK (0x3f << 8)
#define TX_PWR_VHT_MCS_2_3_SIGN (1 << 14)
#define TX_PWR_VHT_MCS_2_3_EN (1 << 15)

#define TX_PWR_VHT_MCS_4_5 0xBC
#define TX_PWR_VHT_MCS_4_5_MASK (0x3f)
#define TX_PWR_VHT_MCS_4_5_SIGN (1 << 6)
#define TX_PWR_VHT_MCS_4_5_EN (1 << 7)
#define TX_PWR_VHT_MCS_6_7_MASK (0x3f << 8)
#define TX_PWR_VHT_MCS_6_7_SIGN (1 << 14)
#define TX_PWR_VHT_MCS_6_7_EN (1 << 15)

#define TX_PWR_5G_VHT_MCS_8_9 0xBE
#define TX_PWR_5G_VHT_MCS_8_9_MASK (0x3f)
#define TX_PWR_5G_VHT_MCS_8_9_SIGN (1 << 6)
#define TX_PWR_5G_VHT_MCS_8_9_EN (1 << 7)
#define TX_PWR_2G_VHT_MCS_8_9_MASK (0x3f << 8)
#define TX_PWR_2G_VHT_MCS_8_9_SIGN (1 << 14)
#define TX_PWR_2G_VHT_MCS_8_9_EN (1 << 15)

#define RF_2G_RX_HIGH_GAIN 0xF8
#define RF0_2G_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_2G_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_2G_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_2G_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP0_1_RX_HIGH_GAIN 0xFA
#define RF0_5G_GRP0_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP0_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP0_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP0_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP1_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP1_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP1_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP1_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP2_3_RX_HIGH_GAIN 0xFC
#define RF0_5G_GRP2_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP2_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP2_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP2_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP3_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP3_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP3_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP3_RX_HIGH_GAIN_SIGN (1 << 15)

#define RF_5G_GRP4_5_RX_HIGH_GAIN 0xFE
#define RF0_5G_GRP4_RX_HIGH_GAIN_MASK (0x07)
#define RF0_5G_GRP4_RX_HIGH_GAIN_SIGN (1 << 3)
#define RF1_5G_GRP4_RX_HIGH_GAIN_MASK (0x07 << 4)
#define RF1_5G_GRP4_RX_HIGH_GAIN_SIGN (1 << 7)
#define RF0_5G_GRP5_RX_HIGH_GAIN_MASK (0x07 << 8)
#define RF0_5G_GRP5_RX_HIGH_GAIN_SIGN (1 << 11)
#define RF1_5G_GRP5_RX_HIGH_GAIN_MASK (0x07 << 12)
#define RF1_5G_GRP5_RX_HIGH_GAIN_SIGN (1 << 15)

#define BT_RCAL_RESULT 0x138
#define BT_VCDL_CALIBRATION 0x13C
#define BT_PMUCFG 0x13E

struct _RTMP_ADAPTER;

#ifdef RTMP_PCI_SUPPORT
/*************************************************************************
  *	Public function declarations for prom-based chipset
  ************************************************************************/
BOOLEAN rtmp_ee_prom_read16(struct _RTMP_ADAPTER *pAd, UINT16 Offset, UINT16 *pVal);
int rtmp_ee_prom_write16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT val);
#endif /* RTMP_PCI_SUPPORT */




#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
/*************************************************************************
  *	Public function declarations for flash-based chipset
  ************************************************************************/
NDIS_STATUS rtmp_nv_init(struct _RTMP_ADAPTER *pAd);
RTMP_STRING *get_dev_eeprom_binary(VOID *pvAd);
UINT get_dev_eeprom_offset(VOID *pvAd);
UINT32 get_dev_eeprom_size(VOID *pvAd);
BOOLEAN rtmp_ee_flash_read(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pVal);
int rtmp_ee_flash_write(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT data);
VOID rtmp_ee_flash_read_all(struct _RTMP_ADAPTER *pAd, USHORT *Data);
VOID rtmp_ee_flash_write_all(struct _RTMP_ADAPTER *pAd, USHORT *Data);
#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */


#ifdef RTMP_EFUSE_SUPPORT
/*************************************************************************
  *	Public function declarations for efuse-based chipset
  ************************************************************************/
BOOLEAN rtmp_ee_efuse_read16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pVal);
int rtmp_ee_efuse_write16(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT data);
INT rtmp_ee_write_to_efuse(struct _RTMP_ADAPTER *pAd);

NTSTATUS eFuseRead(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pData, USHORT len);
NTSTATUS eFuseWrite(struct _RTMP_ADAPTER *pAd, USHORT Offset, USHORT *pData, USHORT len);

VOID eFuseGetFreeBlockCount(struct _RTMP_ADAPTER *pAd, UINT *EfuseFreeBlock);

INT eFuse_init(struct _RTMP_ADAPTER *pAd);
INT efuse_probe(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_EFUSE_SUPPORT */

/*************************************************************************
  *	Public function declarations for using BIN buffer mode to access eeprom contents
  ************************************************************************/
BOOLEAN rtmp_ee_bin_read16(struct _RTMP_ADAPTER *pAd, UINT16 Offset, UINT16 *pValue);

INT rtmp_ee_bin_write16(
	IN struct _RTMP_ADAPTER *pAd,
	IN USHORT 			Offset, 
	IN USHORT 			data);

INT rtmp_ee_load_from_bin(
	IN struct _RTMP_ADAPTER *pAd);

INT rtmp_ee_write_to_bin(
	IN struct _RTMP_ADAPTER *pAd);

INT rtmp_ee_write_to_prom(struct _RTMP_ADAPTER *pAd);
	
INT Set_LoadEepromBufferFromBin_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_EepromBufferWriteBack_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);


/*************************************************************************
  *	Public function declarations for prom operation callback functions setting
  ************************************************************************/
INT RtmpChipOpsEepromHook(struct _RTMP_ADAPTER *pAd, INT infType,INT forceMode);

INT NICReadEEPROMParameters(struct _RTMP_ADAPTER *pAd, RTMP_STRING *mac_addr);
BOOLEAN rt28xx_eeprom_read16(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT *value);

#endif /* __EEPROM_H__ */
