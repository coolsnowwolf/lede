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
	mt76x0.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MT76X0_H__
#define __MT76X0_H__

#include "../mcu/andes_core.h"
#include "../mcu/andes_rlt.h"

struct _RTMP_ADAPTER;

/* 
	R37
	R36
	R35
	R34
	R33
	R32<7:5>
	R32<4:0> pll_den: (Denomina - 8)
	R31<7:5>
	R31<4:0> pll_k(Nominator)
	R30<7> sdm_reset_n
	R30<6:2> sdmmash_prbs,sin
	R30<1> sdm_bp
	R30<0> R29<7:0> (hex) pll_n
	R28<7:6> isi_iso
	R28<5:4> pfd_dly
	R28<3:2> clksel option
	R28<1:0> R27<7:0> R26<7:0> (hex) sdm_k
	R24<1:0> xo_div
*/
typedef struct _MT76x0_FREQ_ITEM {
	UINT8 Channel;
	UINT32 Band;
	UINT8 pllR37;
	UINT8 pllR36;
	UINT8 pllR35;
	UINT8 pllR34;
	UINT8 pllR33;
	UINT8 pllR32_b7b5;
	UINT8 pllR32_b4b0; /* PLL_DEN */
	UINT8 pllR31_b7b5;
	UINT8 pllR31_b4b0; /* PLL_K */
	UINT8 pllR30_b7; /* sdm_reset_n */
	UINT8 pllR30_b6b2; /* sdmmash_prbs,sin */
	UINT8 pllR30_b1; /* sdm_bp */
	UINT16 pll_n; /* R30<0>, R29<7:0> (hex) */	
	UINT8 pllR28_b7b6; /* isi,iso */
	UINT8 pllR28_b5b4; /* pfd_dly */
	UINT8 pllR28_b3b2; /* clksel option */
	UINT32 Pll_sdm_k; /* R28<1:0>, R27<7:0>, R26<7:0> (hex) SDM_k */
	UINT8 pllR24_b1b0; /* xo_div */
} MT76x0_FREQ_ITEM;

#define RF_G_BAND 		0x0100
#define RF_A_BAND 		0x0200
#define RF_A_BAND_LB	0x0400
#define RF_A_BAND_MB	0x0800
#define RF_A_BAND_HB	0x1000
#define RF_A_BAND_11J	0x2000
typedef struct _RT6590_RF_SWITCH_ITEM {
	UCHAR Bank;
	UCHAR Register;
	UINT32 BwBand; /* (BW_20, BW_40, BW_80) | (G_Band, A_Band_LB, A_Band_MB, A_Band_HB) */
	UCHAR Value;
} MT76x0_RF_SWITCH_ITEM, *PMT76x0_RF_SWITCH_ITEM;

typedef struct _MT76x0_BBP_Table {
	UINT32 BwBand; /* (BW_20, BW_40, BW_80) | (G_Band, A_Band_LB, A_Band_MB, A_Band_HB) */
	RTMP_REG_PAIR RegDate;
} MT76x0_BBP_Table, *PMT76x0_BBP_Table;

typedef struct _MT76x0_RATE_PWR_ITEM {
	CHAR MCS_Power;
	UCHAR RF_PA_Mode;
} MT76x0_RATE_PWR_ITEM, *PMT76x0_RATE_PWR_ITEM;

typedef struct _MT76x0_RATE_PWR_TABLE {
	MT76x0_RATE_PWR_ITEM CCK[4];
	MT76x0_RATE_PWR_ITEM OFDM[8];
	MT76x0_RATE_PWR_ITEM HT[8];
	MT76x0_RATE_PWR_ITEM VHT[10];
	MT76x0_RATE_PWR_ITEM STBC[8];
	MT76x0_RATE_PWR_ITEM MCS32;
} MT76x0_RATE_PWR_Table, *PMT76x0_RATE_PWR_Table;

VOID MT76x0_Init(struct _RTMP_ADAPTER *pAd);
INT MT76x0_ReadChannelPwr(struct _RTMP_ADAPTER *pAd);
INT MT76x0_DisableTxRx(
	struct _RTMP_ADAPTER *pAd,
	UCHAR Level);

#ifdef DBG
VOID MT76x0_ShowDmaIndexCupIndex(
	struct _RTMP_ADAPTER *pAd);
#endif /* DBG */

void mt76x0_read_per_rate_tx_pwr(struct _RTMP_ADAPTER *pAd);

void mt76x0_antenna_sel_ctl(struct _RTMP_ADAPTER *ad);

void mt76x0_vco_calibration(struct _RTMP_ADAPTER *ad, UINT8 channel);

void mt76x0_calibration(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR channel,
	IN BOOLEAN bPowerOn,
	IN BOOLEAN bFullCal,
	IN BOOLEAN bSaveCal);

void mt76x0_temp_sensor(struct _RTMP_ADAPTER *ad);

void mt76x0_temp_trigger_cal(struct _RTMP_ADAPTER *ad);

#ifdef DFS_SUPPORT
VOID MT76x0_DFS_CR_Init(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* DFS_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT
VOID MT76x0_ReadFlashAndInitAsic(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_MAC_PCI
VOID MT76x0_InitPCIeLinkCtrlValue(
	IN struct _RTMP_ADAPTER *pAd);

VOID MT76x0_PciMlmeRadioOFF(
	IN struct _RTMP_ADAPTER *pAd);

VOID MT76x0_PciMlmeRadioOn(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_PCI */

INT Set_AntennaSelect_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID MT76x0_MakeUpRatePwrTable(
	IN struct _RTMP_ADAPTER *pAd);

VOID mt76x0_asic_adjust_tx_power(
	IN struct _RTMP_ADAPTER *pAd);

#ifdef RTMP_TEMPERATURE_COMPENSATION
void mt76x0_temp_tx_alc(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#ifdef MT76x0_TSSI_CAL_COMPENSATION
VOID MT76x0_TSSI_DC_Calibration(
	IN struct _RTMP_ADAPTER *pAd);

VOID MT76x0_IntTxAlcProcess(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* MT76x0_TSSI_CAL_COMPENSATION */

#ifdef SINGLE_SKU_V2
UCHAR MT76x0_GetSkuChannelBasePwr(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR channel);

UCHAR MT76x0_UpdateSkuPwr(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR channel);
#endif /* SINGLE_SKU_V2 */

void mt76x0_read_tx_alc_info_from_eeprom(struct _RTMP_ADAPTER *pAd);

void mt76x0_adjust_per_rate_pwr(struct _RTMP_ADAPTER *ad);

#endif /* __MT76x0_H__ */

