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
	rt6352.h
 
    Abstract:
	2*2 Wireless Chip SoC

    Revision History:
    Who			When			What
    ---------	----------		----------------------------------------------
	Arvin Tai		20120301		Initial version
 */

#ifndef __RT6352_H__
#define __RT6352_H__

#ifdef RT6352

#ifndef RTMP_RBUS_SUPPORT
#error "For RT3883, you should define the compile flag -DRTMP_RBUS_SUPPORT"
#endif

#ifndef RTMP_MAC_PCI
#error "For RT3883, you should define the compile flag -DRTMP_MAC_PCI"
#endif

struct _RTMP_ADAPTER;

/* */
/* Device ID & Vendor ID, these values should match EEPROM value */
/* */
#define RTMP_MAC_CSR_ADDR	0xB0180000
#define RTMP_FLASH_BASE_ADDR	0xbc000000


extern REG_PAIR RT6352_RFCentralRegTable[];
extern UCHAR RT6352_NUM_RF_CENTRAL_REG_PARMS;
extern REG_PAIR RT6352_RFChannelRegTable[];
extern UCHAR RT6352_NUM_RF_CHANNEL_REG_PARMS;
extern REG_PAIR RT6352_RFChannelRegE2Table[];
extern UCHAR RT6352_NUM_RF_CHANNEL_E2_REG_PARMS;
extern REG_PAIR RT6352_RFDCCalRegTable[];
extern UCHAR RT6352_NUM_RF_DCCAL_REG_PARMS;

extern REG_PAIR RT6352_BBPRegTable[];
extern UCHAR RT6352_NUM_BBP_REG_PARMS;
extern RTMP_REG_PAIR RT6352_MACRegTable[];
extern UCHAR RT6352_NUM_MAC_REG_PARMS;

#if defined (CONFIG_RALINK_RT6352)  || defined (CONFIG_RALINK_MT7620)
#define PROCREG_DIR             "rt6352"
#endif /* defined (CONFIG_RALINK_RT6352)  || defined (CONFIG_RALINK_MT7620) */

#ifdef RTMP_TEMPERATURE_COMPENSATION
#define RT6352_EEPROM_TSSI_24G_READ(__pAd)											\
{																					\
	EEPROM_TX_PWR_STRUC __Power;													\
	USHORT __Value;																	\
	CHAR __Offset;																	\
	__Offset = __pAd->TemperatureRef25C;											\
	__pAd->TssiCalibratedOffset = __Offset;											\
	RT28xx_EEPROM_READ16(__pAd, EEPROM_G_TSSI_BOUND1, __Power.word);				\
	__pAd->TssiMinusBoundaryG[7] = __Power.field.Byte0;								\
	__pAd->TssiMinusBoundaryG[6] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 2), __Power.word);\
	__pAd->TssiMinusBoundaryG[5] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryG[4] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 4), __Power.word);\
	__pAd->TssiMinusBoundaryG[3] = __Power.field.Byte0;						\
	__pAd->TssiMinusBoundaryG[2] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 6), __Power.word);\
	__pAd->TssiMinusBoundaryG[1] = __Power.field.Byte0;						\
	__pAd->TssiRefG = __Power.field.Byte1;									\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 10), __Power.word);\
	__pAd->TssiPlusBoundaryG[1] = __Power.field.Byte0;						\
	__pAd->TssiPlusBoundaryG[2] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 12), __Power.word);\
	__pAd->TssiPlusBoundaryG[3] = __Power.field.Byte0;						\
	__pAd->TssiPlusBoundaryG[4] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 14), __Power.word);\
	__pAd->TssiPlusBoundaryG[5] = __Power.field.Byte0;						\
	__pAd->TssiPlusBoundaryG[6] = __Power.field.Byte1;						\
	RT28xx_EEPROM_READ16(__pAd, (EEPROM_G_TSSI_BOUND1 + 16), __Value);\
	__pAd->TssiPlusBoundaryG[7] = __Power.field.Byte0;						\
	__pAd->TxAgcStepG = ((__Value & 0xFF00) >> 8);							\
	__pAd->TxAgcCompensateG = 0;											\
	__pAd->TssiMinusBoundaryG[0] = __pAd->TssiRefG;							\
	__pAd->TssiPlusBoundaryG[0]  = __pAd->TssiRefG;							\
	if (__pAd->TssiRefG == 0xFF)											\
		__pAd->bAutoTxAgcG = FALSE;											\
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

typedef struct _RT635x_FREQUENCY_ITEM {
	UCHAR  Channel;
	UCHAR  Rdiv;
	UINT16 N;
	UCHAR  K;
	UCHAR  D;
	UINT32 Ksd;
} RT635x_FREQUENCY_ITEM, *PRT635x_FREQUENCY_ITEM;

VOID RT6352_Init(
	IN struct _RTMP_ADAPTER	*pAd);

VOID RT6352_RTMPReadTxPwrPerRate(
	IN struct _RTMP_ADAPTER	*pAd);

VOID RT6352_AsicAdjustTxPower(
	IN struct _RTMP_ADAPTER *pAd);

#ifdef RTMP_TEMPERATURE_CALIBRATION
VOID RT6352_Temperature_Init(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT6352_TemperatureCalibration(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */

#ifdef RTMP_TEMPERATURE_COMPENSATION
VOID RT6352_TssiTableAdjust(
	IN struct _RTMP_ADAPTER	*pAd);

VOID RT6352_TssiMpAdjust(
	IN struct _RTMP_ADAPTER	*pAd);

BOOLEAN RT6352_TemperatureCompensation(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN bResetTssiInfo);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
VOID RT6352_Init_ExtPA_ExtLNA(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN ReInit);

VOID RT6352_Restore_RF_BBP(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT6352_ReCalibration(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */

#ifdef SINGLE_SKU_V2
VOID RT6352_InitSkuRateDiffTable(
	IN struct _RTMP_ADAPTER *pAd);

UCHAR RT6352_GetSkuChannelBasePwr(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR channel);
	
CHAR RT6352_AdjustChannelPwr(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN CHAR TotalDeltaPower,
	OUT CHAR *PreDiff);

VOID RT6352_AdjustPerRatePwr(
	IN struct _RTMP_ADAPTER 	*pAd,
	IN CHAR channelpower);

UCHAR GetSkuPerRatePwr(
	IN struct _RTMP_ADAPTER *pAd,
	IN CHAR phymode,
	IN UCHAR channel,
	IN UCHAR bw,
	IN INT32 paValue);
#endif /* SINGLE_SKU_V2 */

#ifdef DYNAMIC_VGA_SUPPORT
VOID rt6352_dynamic_vga_enable(
	IN struct _RTMP_ADAPTER *pAd);

VOID rt6352_dynamic_vga_disable(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* DYNAMIC_VGA_SUPPORT */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
#define GROUP1_2G 0
#define GROUP2_2G 1
#define GROUP3_2G 2
#define GROUPS_5G 3

#define GET_2G_CHANNEL_GROUP(_channel)	((_channel <= 4) ?	\
			(GROUP1_2G) : ((_channel <= 8) ?				\
			(GROUP2_2G) : ((_channel <= 14) ?				\
			(GROUP3_2G) : (GROUPS_5G))))
#define max_ant	2
#define DEFAULT_BO              4
#define LIN2DB_ERROR_CODE       (-10000)
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

#ifdef RTMP_INTERNAL_TX_ALC
INT16 RT6352_lin2dBd(
	IN	unsigned short linearValue);

CHAR SignedExtension6To8(
	IN CHAR org_value);

VOID RT635xGetTssiInfo(
	IN struct _RTMP_ADAPTER *pAd);

INT RT635xTssiDcCalibration(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT635xInitMcsPowerTable(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT635xInitRfPaModeTable(
	IN struct _RTMP_ADAPTER *pAd);

VOID RT635xTssiCompensation(
	IN struct _RTMP_ADAPTER *pAd,
	IN CHAR	percent_delta);

BOOLEAN RT635xTriggerTssiCompensation(
	IN struct _RTMP_ADAPTER *pAd,
	IN CHAR	percent_delta);

BOOLEAN RT635xCheckTssiCompensation(
	IN struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef ED_MONITOR
VOID RT6352_set_ed_cca(
	IN struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif /* ED_MONITOR */

#endif /* RT6352 */
#endif /*__RT6352_H__ */
/* End of rt6352.h */

