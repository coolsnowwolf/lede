/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_rbs.h

    Abstract:
	Ralink SoC Internal Bus related definitions and data dtructures

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RTMP_RBUS_H__
#define __RTMP_RBUS_H__

#include "iface/rtmp_inf_pcirbs.h"

/*************************************************************************
  *
  *	Device hardware/ Interface related definitions.
  *
  ************************************************************************/

#define RTMP_MAC_IRQ_NUM		4


/*************************************************************************
  *
  *	EEPROM Related definitions
  *
  ************************************************************************/
#if defined(CONFIG_RALINK_RT3050_1T1R)
#if defined(CONFIG_RALINK_RT3350)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3350_AP_1T1R_V1_0.bin"
#else
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3050_AP_1T1R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3350 */
#endif /* CONFIG_RALINK_RT3050_1T1R */

#if defined(CONFIG_RALINK_RT3051_1T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3051_AP_1T2R_V1_0.bin"
#endif /* CONFIG_RALINK_RT3051_1T2R */

#if defined(CONFIG_RALINK_RT3052_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3052_AP_2T2R_V1_1.bin"
#endif /* CONFIG_RALINK_RT3052_2T2R */

#if defined(CONFIG_RALINK_RT3883_3T3R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3883_AP_3T3R_V0_1.bin"
#endif /* CONFIG_RALINK_RT3883_3T3R */

#if defined(CONFIG_RALINK_RT3662_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3662_AP_2T2R_V0_0.bin"
#endif /* CONFIG_RALINK_RT3662_2T2R */

#if defined(CONFIG_RALINK_RT3352_2T2R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT3352_AP_2T2R-4L_V12.BIN"
#endif /* CONFIG_RALINK_RT3352_2T2R */

#if defined(CONFIG_RALINK_RT5350_1T1R)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT5350_AP_1T1R_V1_0.bin"
#endif /* CONFIG_RALINK_RT5350_1T1R */

#if defined(CONFIG_RT2860V2_2850)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/RT2880_RT2850_AP_2T3R_V1_6.bin"
#endif /* CONFIG_RT2860V2_2850 */

#if defined(CONFIG_RALINK_RT6352)  || defined(CONFIG_RALINK_MT7620)
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/MT7620_AP_2T2R-4L_V15.BIN"
#endif /* defined (CONFIG_RALINK_RT6352)  || defined (CONFIG_RALINK_MT7620) */

#if defined(CONFIG_RALINK_MT7612E)
#if defined(CONFIG_RALINK_MT7620) /* TODO: PATH */
#undef EEPROM_DEFAULT_FILE_PATH
#endif
#define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/MT7662E2_EEPROM_20130527.bin"
/* #define EEPROM_DEFAULT_FILE_PATH                     "/etc_ro/wlan/MT7662E2_EEPROM_20130903_ePA.bin" */
#endif /* CONFIG_RALINK_MT7612E */


#if defined(CONFIG_RT2880_FLASH_32M)
#define MTD_NUM_FACTORY 5
#else
#define MTD_NUM_FACTORY 2
#endif




#ifdef LINUX
/*************************************************************************
  *
  *	Device Tx/Rx related definitions.
  *
  ************************************************************************/

#endif /* LINUX */

#endif /* __RTMP_RBUS_H__ */

