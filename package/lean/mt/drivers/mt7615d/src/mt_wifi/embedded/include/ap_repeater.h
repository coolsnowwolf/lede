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
    ap_repeater.h

    Abstract:
    repeater function related definition collection.

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    Carter.Chen 2015-April-14   init version.
*/
#ifndef __AP_REPEATER_H__
#define __AP_REPEATER_H__

#include    "rtmp.h"

#define GET_MAX_REPEATER_ENTRY_NUM(_pChipCap)    _pChipCap->MaxRepeaterNum

VOID RepeaterCtrlInit(RTMP_ADAPTER *pAd);
VOID RepeaterCtrlExit(RTMP_ADAPTER *pAd);
VOID CliLinkMapInit(RTMP_ADAPTER *pAd);

enum _REPEATER_MLME_ENQ_IDX_CHK_TBL {
	REPT_MLME_START_IDX = 64,
	REPT_MLME_LAST_IDX = 95,
	REPT_MLME_MAX_IDX = 96,/*shall not over this.*/
};

enum _REPT_ENTRY_CONNT_STATE {
	REPT_ENTRY_DISCONNT = 0,
	REPT_ENTRY_CONNTING = 1,
	REPT_ENTRY_CONNTED = 2,
};

enum _REPEATER_MAC_ADDR_RULE_TYPE {
	FOLLOW_CLI_LINK_MAC_ADDR_OUI = 0,
	CASUALLY_DEFINE_MAC_ADDR = 1,
	VENDOR_DEFINED_MAC_ADDR_OUI = 2,
};

enum _REPEATER_TX_PKT_CHECK_RESULT {
	REPEATER_ENTRY_EXIST = 0,
	INSERT_REPT_ENTRY = 1,
	USE_CLI_LINK_INFO = 2,
	INSERT_REPT_ENTRY_AND_ALLOW = 3,
};

/* IOCTL*/
INT Show_ReptTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID RepeaterLinkMonitor(RTMP_ADAPTER *pAd);

UINT32 ReptTxPktCheckHandler(
	RTMP_ADAPTER *pAd,
	IN struct wifi_dev *cli_link_wdev,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR * pWcid);

VOID RepeaterFillMlmeParaThenEnq(
	RTMP_ADAPTER *pAd,
	ULONG Machine,
	ULONG MsgType,
	REPEATER_CLIENT_ENTRY *pReptEntry);

INT AsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable);

REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN VOID *pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad);

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PVOID pData,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad);

VOID RTMPInsertRepeaterEntry(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PUCHAR pAddr);

VOID RTMPRemoveRepeaterEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR func_tb_idx,
	IN UCHAR CliIdx);

VOID RTMPRepeaterReconnectionCheck(
	IN RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY * RTMPInsertRepeaterMacEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  PUCHAR pAddr,
	IN  struct wifi_dev *wdev,
	IN  UCHAR apidx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll);

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr);

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr);

VOID InsertIgnoreAsRepeaterEntryTable(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr);

BOOLEAN RepeaterRemoveIngoreEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR idx,
	IN UCHAR * pAddr);

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);


VOID UpdateMbssCliLinkMap(
	RTMP_ADAPTER *pAd,
	UCHAR MbssIdx,
	struct wifi_dev *cli_link_wdev,
	struct wifi_dev *mbss_link_wdev);

REPEATER_CLIENT_ENTRY *lookup_rept_entry(RTMP_ADAPTER *pAd, PUCHAR address);

#endif  /* __AP_REPEATER_H__ */
