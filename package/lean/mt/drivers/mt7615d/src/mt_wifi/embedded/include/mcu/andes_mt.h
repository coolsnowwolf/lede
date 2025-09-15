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
	andes_mt.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_MT_H__
#define __ANDES_MT_H__

#include "mcu.h"
#include "mcu/mt_cmd.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

#define GET_EVENT_FW_RXD_LENGTH(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.length)
#define GET_EVENT_FW_RXD_PKT_TYPE_ID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.pkt_type_id)
#define GET_EVENT_FW_RXD_EID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_1.field.eid)

#define GET_EVENT_FW_RXD_SEQ_NUM(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_1.field.seq_num)

#define GET_EVENT_FW_RXD_EXT_EID(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_2.field.ext_eid)

#define IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(m) \
	((((struct cmd_msg *)(m))->attr.ctrl.expect_size == MT_IGNORE_PAYLOAD_LEN_CHECK) \
	? TRUE : FALSE)
#define GET_EVENT_HDR_ADDR(net_pkt) \
	(GET_OS_PKT_DATAPTR(net_pkt) + sizeof(EVENT_RXD))


#define GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd) \
	(((EVENT_RXD *)(event_rxd))->fw_rxd_0.field.length - sizeof(EVENT_RXD))


struct _RTMP_ADAPTER;
struct cmd_msg;

#define BFBACKOFF_TABLE_SIZE            10
#define BFBACKOFF_BBPCR_SIZE             6
#define RATE_POWER_TMAC_SIZE             8
#define CR_COLUMN_SIZE                   4

VOID AndesMTFillCmdHeaderWithTXD(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
VOID AndesMTRxEventHandler(struct _RTMP_ADAPTER *pAd, UCHAR *data);
INT32 AndesMTLoadFw(struct _RTMP_ADAPTER *pAd);
INT32 AndesMTEraseFw(struct _RTMP_ADAPTER *pAd);

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
INT32 AndesMTPciKickOutCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
INT32 AndesMTPciKickOutCmdMsgFwDlRing(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
INT32 AndesRestartCheck(struct _RTMP_ADAPTER *pAd);
#endif /* defined(MT7615) || defined(MT7622) */
VOID AndesMTPciFwInit(struct _RTMP_ADAPTER *pAd);
VOID AndesMTPciFwExit(struct _RTMP_ADAPTER *pAd);
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */



#ifdef TXBF_SUPPORT
VOID ExtEventBfStatusRead(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif

#ifdef LED_CONTROL_SUPPORT
#if defined(MT7615) || defined(MT7622)
INT AndesLedEnhanceOP(
	struct _RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode
);
#else
INT AndesLedEnhanceOP(
	struct _RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode
);
#endif
#endif

INT32 AndesMTLoadRomPatch(struct _RTMP_ADAPTER *ad);
INT32 AndesMTEraseRomPatch(struct _RTMP_ADAPTER *ad);

#ifdef WIFI_SPECTRUM_SUPPORT
NTSTATUS WifiSpectrumRawDataHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
VOID ExtEventWifiSpectrumRawDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
NTSTATUS ICapRawDataHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
VOID ExtEventICap96BitDataParser(struct _RTMP_ADAPTER  *pAd);
VOID ExtEventICap96BitRawDataHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
VOID ExtEventICapStatusHandler(struct _RTMP_ADAPTER *pAd, UINT8 *pData, UINT32 Length);
#endif /* INTERNAL_CAPTURE_SUPPORT */

VOID EventTxPowerHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerEPAInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalStateShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventPowerTableShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventTxPowerCompTable(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
VOID EventThermalCompTableShowInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
NTSTATUS EventTxvBbpPowerInfo(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
NTSTATUS PreCalTxLPFStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalTxIQStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalTxDCStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalRxFIStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
NTSTATUS PreCalRxFDStoreProcHandler(struct _RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#if defined(RED_SUPPORT) && (defined(MT7622) || defined(P18) || defined(MT7663) || defined(MT7615))
VOID ExtEventMpduTimeHandler(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
#endif
typedef struct _TX_RATE_POWER_TABLE_T {
	UINT8  TxRateModulation;
	UINT8  CRValue;
	CHAR   PowerDecimal;
} TX_RATE_POWER_TABLE_T, *P_TX_RATE_POWER_TABLE_T;

typedef struct _TX_POWER_BOUND_TABLE_T {
	UINT8  MaxMinType;
	UINT8  CRValue;
	CHAR   PowerDecimal;
} TX_POWER_BOUND_TABLE_T, *P_TX_POWER_BOUND_TABLE_T;

typedef enum _TX_POWER_SKU_TABLE {
	CCK1M2M,
	CCK5M11M,
	OFDM6M9M,
	OFDM12M18M,
	OFDM24M36M,
	OFDM48M,
	OFDM54M,
	HT20M0,
	HT20M32,
	HT20M1M2,
	HT20M3M4,
	HT20M5,
	HT20M6,
	HT20M7,
	HT40M0,
	HT40M32,
	HT40M1M2,
	HT40M3M4,
	HT40M5,
	HT40M6,
	HT40M7,
	VHT20M0,
	VHT20M1M2,
	VHT20M3M4,
	VHT20M5M6,
	VHT20M7,
	VHT20M8,
	VHT20M9,
	VHT40M0,
	VHT40M1M2,
	VHT40M3M4,
	VHT40M5M6,
	VHT40M7,
	VHT40M8,
	VHT40M9,
	VHT80M0,
	VHT80M1M2,
	VHT80M3M4,
	VHT80M5M6,
	VHT80M7,
	VHT80M8,
	VHT80M9,
	VHT160M0,
	VHT160M1M2,
	VHT160M3M4,
	VHT160M5M6,
	VHT160M7,
	VHT160M8,
	VHT160M9,
    TXPOWER_1SS_OFFSET,
    TXPOWER_2SS_OFFSET,
    TXPOWER_3SS_OFFSET,
    TXPOWER_4SS_OFFSET
} TX_POWER_SKU_TABLE, *P_TX_POWER_SKU_TABLE;


typedef enum _ENUM_MAX_MIN_TYPE_T {
	MAX_POWER_BAND0,
	MIN_POWER_BAND0,
	MAX_POWER_BAND1,
	MIN_POWER_BAND1
} ENUM_MAX_MIN_TYPE_T, *P_ENUM_MAX_MIN_TYPE_T;

typedef enum _ENUM_TX_RATE_MODULATION_T {
	OFDM_48M,
	OFDM_24M_36M,
	OFDM_12M_18M,
	OFDM_6M_9M,
	HT20_MCS5,
	HT20_MCS3_4,
	HT20_MCS1_2,
	HT20_MCS0,
	HT40_MCS5,
	HT40_MCS3_4,
	HT40_MCS1_2,
	HT40_MCS0,
	HT40_MCS32,
	CCK_5M11M,
	OFDM_54M,
	CCK_1M2M,
	HT40_MCS7,
	HT40_MCS6,
	HT20_MCS7,
	HT20_MCS6,
	VHT20_MCS5_6,
	VHT20_MCS3_4,
	VHT20_MCS1_2,
	VHT20_MCS0,
	VHT20_MCS9,
	VHT20_MCS8,
	VHT20_MCS7,
	VHT160,
	VHT80,
	VHT40
} ENUM_TX_RATE_MODULATION_T, *P_ENUM_TX_RATE_MODULATION_T;


#endif /* __ANDES_MT_H__ */

