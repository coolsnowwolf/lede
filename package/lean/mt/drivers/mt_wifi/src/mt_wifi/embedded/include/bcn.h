/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    bcn.h

    Abstract:
    bcn related fucntions definition.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter      2014-1121     Created.
*/

#ifndef __BCN_H__
#define __BCN_H__


#define MAX_BEACON_LENGTH       (sizeof(HEADER_802_11) + \
								 TIMESTAMP_FIELD_LEN + \
								 BEACON_INTERVAL_FIELD_LEN + \
								 CAP_INFO_FIELD_LEN + \
								 MAX_IE_LENGTH)

enum BCN_UPDATE_REASON {
	BCN_UPDATE_INIT		= 0,	/* beacon resource initial. */
	BCN_UPDATE_IF_STATE_CHG	= 1,	/* there is interface up or down, check related TXD handle. */
	BCN_UPDATE_IE_CHG		= 2,	/* simple IE change, just update the corresponding interface content. */
	BCN_UPDATE_AP_RENEW	= 3,	/* prepared All beacon for All active interface. */
	BCN_UPDATE_PRETBTT		= 4,	/* update function routine, source could from INT isr or timer or event notify. */
	BCN_UPDATE_ENABLE_TX	= 5,	/* Enable Beacon TX. */
	BCN_UPDATE_DISABLE_TX	= 6,	/* Disable Beacon TX. */
	BCN_UPDATE_TIM		= 7,	/* TIM preparing */
};

typedef enum _ENUM_UPDATE_PKT_TYPE_T {
	PKT_BCN = 0,
	PKT_TIM
} ENUM_UPDATE_PKT_TYPE_T;

typedef enum _BCN_GEN_METHOD {
	BCN_GEN_BY_HW_SHARED_MEM = 0,   /* RT chip */
	BCN_GEN_BY_HOST_IN_PRETBTT,     /* MT_chip with small size fw */
	BCN_GEN_BY_FW,                  /* MT_chip with large size fw */
	BCN_GEN_BY_HOST_TOUCH_PSE_CTNT  /* TODO:MT_chip don't free bcn in BcnQ, recycle update by Host */
} BCN_GEN_METHOD;

VOID write_tmac_info_beacon(
	RTMP_ADAPTER * pAd,
	struct wifi_dev *wdev,
	UCHAR *tmac_buf,
	HTTRANSMIT_SETTING *BeaconTransmit,
	ULONG frmLen);

INT bcn_buf_deinit(RTMP_ADAPTER *pAd, BCN_BUF_STRUC *bcn_info);

INT bcn_buf_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT clearHwBcnTxMem(RTMP_ADAPTER *pAd);

ULONG ComposeBcnPktHead(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pBeaconFrame);
VOID BcnCheck(RTMP_ADAPTER *pAd);

#ifdef CONFIG_AP_SUPPORT
INT BcnTimUpdate(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *ptr);
#endif
VOID ComposeBcnPktTail(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, ULONG *pFrameLen, UCHAR *pBeaconFrame);

UINT16 MakeBeacon(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine);

BOOLEAN BeaconTransmitRequired(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine);

VOID UpdateBeaconHandler(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UCHAR BCN_UPDATE_REASON);

BOOLEAN UpdateBeaconProc(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	BOOLEAN UpdateRoutine,
	UCHAR UpdatePktType,
	BOOLEAN bMakeBeacon);

VOID updateBeaconRoutineCase(
	RTMP_ADAPTER *pAd,
	BOOLEAN UpdateAfterTim);

#endif  /* __BCN_H__ */
