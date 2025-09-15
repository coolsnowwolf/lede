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
    ap_wds.h

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
    Fonchi    02-13-2007      created
*/


#ifndef _AP_WDS_H_
#define _AP_WDS_H_

#define WDS_ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)


static inline BOOLEAN WDS_IF_UP_CHECK(
	IN  PRTMP_ADAPTER   pAd,
	IN  ULONG ifidx)
{
	if ((pAd->WdsTab.flg_wds_init != TRUE) ||
		(ifidx >= MAX_WDS_ENTRY))
		return FALSE;

	/*	if(RTMP_OS_NETDEV_STATE_RUNNING(pAd->WdsTab.WdsEntry[ifidx].dev)) */
	/* Patch for wds ,when dirver call apmlmeperiod => APMlmeDynamicTxRateSwitching check if wds device ready */
	if ((pAd->WdsTab.WdsEntry[ifidx].wdev.if_dev != NULL) && (RTMP_OS_NETDEV_STATE_RUNNING(pAd->WdsTab.WdsEntry[ifidx].wdev.if_dev)))
		return TRUE;

	return FALSE;
}

INT WdsEntryAlloc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

VOID WdsEntryDel(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

MAC_TABLE_ENTRY *MacTableInsertWDSEntry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR pAddr,
	UINT WdsTabIdx);

BOOLEAN MacTableDeleteWDSEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid);

MAC_TABLE_ENTRY *WdsTableLookupByWcid(
	IN  PRTMP_ADAPTER   pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount);

MAC_TABLE_ENTRY *WdsTableLookup(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pAddr,
	IN BOOLEAN bResetIdelCount);

MAC_TABLE_ENTRY *FindWdsEntry(
	IN PRTMP_ADAPTER	pAd,
	IN struct _RX_BLK *pRxBlk);

VOID WdsTableMaintenance(
	IN PRTMP_ADAPTER    pAd);

VOID AsicUpdateWdsRxWCIDTable(
	IN PRTMP_ADAPTER pAd,
	IN UINT WdsTabIdx);

VOID AsicUpdateWdsEncryption(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid);

VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BOOLEAN bWmmCapable,
	IN BCN_IE_LIST * ie_list);

VOID APWdsInitialize(RTMP_ADAPTER *pAd);

INT	Show_WdsTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID rtmp_read_wds_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);

VOID WDS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps);
VOID WDS_Remove(RTMP_ADAPTER *pAd);
BOOLEAN WDS_StatsGet(RTMP_ADAPTER *pAd, RT_CMD_STATS *pStats);
VOID AP_WDS_KeyNameMakeUp(RTMP_STRING *pKey, UINT32 KeyMaxSize, INT KeyId);

INT wds_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt);

INT wds_rx_foward_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);

UCHAR WdsPhyOpModeToSuppPhyMode(
	IN PRTMP_ADAPTER pAd,
	IN PRT_802_11_WDS_ENTRY pWdsEntry);

UCHAR WdsPhyOpModeToSuppRateMode(
	IN PRTMP_ADAPTER pAd,
	IN PRT_802_11_WDS_ENTRY pWdsEntry);

/*
	==========================================================================
	Description:
		Check the WDS Entry is valid or not.
	==========================================================================
 */

BOOLEAN ValidWdsEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR WdsIndex);

INT wds_inf_open(struct wifi_dev *wdev);
INT wds_inf_close(struct wifi_dev *wdev);

BOOLEAN WdsMsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT PINT Machine,
	OUT PINT MsgType);

VOID ap_wds_rcv_uc_data_action(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem);

VOID ap_wds_bss_linkdown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem);

/* ap_wds.c */
VOID WdsStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

#endif /* _AP_WDS_H_ */

