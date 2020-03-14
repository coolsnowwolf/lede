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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/
#ifndef __TX_PWR_H__
#define __TX_PWR_H__

#ifdef MT_MAC
#include "eeprom/mt_tx_pwr.h"
#endif

#define DEFAULT_BO              4
#define LIN2DB_ERROR_CODE       (-10000)

#define G_BAND_LOW 0
#define G_BAND_MID 1
#define G_BAND_HI 2

#define A_BAND_LOW 0
#define A_BAND_HI 1

#ifdef SINGLE_SKU_V2
#define	SKU_PHYMODE_CCK_1M_2M				0
#define	SKU_PHYMODE_CCK_5M_11M				1
#define	SKU_PHYMODE_OFDM_6M_9M				2
#define	SKU_PHYMODE_OFDM_12M_18M			3
#define	SKU_PHYMODE_OFDM_24M_36M			4
#define	SKU_PHYMODE_OFDM_48M_54M			5
#define	SKU_PHYMODE_HT_MCS0_MCS1			6
#define	SKU_PHYMODE_HT_MCS2_MCS3			7
#define	SKU_PHYMODE_HT_MCS4_MCS5			8
#define	SKU_PHYMODE_HT_MCS6_MCS7			9
#define	SKU_PHYMODE_HT_MCS8_MCS9			10
#define	SKU_PHYMODE_HT_MCS10_MCS11			11
#define	SKU_PHYMODE_HT_MCS12_MCS13			12
#define	SKU_PHYMODE_HT_MCS14_MCS15			13
#define	SKU_PHYMODE_STBC_MCS0_MCS1			14
#define	SKU_PHYMODE_STBC_MCS2_MCS3			15
#define	SKU_PHYMODE_STBC_MCS4_MCS5			16
#define	SKU_PHYMODE_STBC_MCS6_MCS7			17
#endif /* SINGLE_SKU_V2 */

VOID LoadTssiInfoFromEEPROM(struct _RTMP_ADAPTER *pAd);
INT32 get_low_mid_hi_index(UINT8 Channel);
#endif /* __TX_PWR_H__ */
