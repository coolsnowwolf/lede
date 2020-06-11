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
     ap_ids.c

     Abstract:
     monitor intrusion detection condition

     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
 */
#ifdef IDS_SUPPORT

#include "rt_config.h"

#define IDS_EXEC_INTV          1000				/* 1 sec */


VOID RTMPIdsStart(
	IN PRTMP_ADAPTER	pAd)
{
	if (pAd->ApCfg.IDSTimerRunning == FALSE) {
		RTMPSetTimer(&pAd->ApCfg.IDSTimer, IDS_EXEC_INTV);
		pAd->ApCfg.IDSTimerRunning = TRUE;
	}
}

VOID RTMPIdsStop(
	IN PRTMP_ADAPTER	pAd)
{
	BOOLEAN     Cancelled;

	if (pAd->ApCfg.IDSTimerRunning == TRUE) {
		RTMPCancelTimer(&pAd->ApCfg.IDSTimer, &Cancelled);
		pAd->ApCfg.IDSTimerRunning = FALSE;
	}
}

#ifdef SYSTEM_LOG_SUPPORT
VOID RTMPHandleIdsEvent(
	IN PRTMP_ADAPTER	pAd)
{
	INT i, j;
	UINT32	FloodFrameCount[IW_FLOOD_EVENT_TYPE_NUM];
	UINT32	FloodFrameThreshold[IW_FLOOD_EVENT_TYPE_NUM];

	FloodFrameCount[0] = pAd->ApCfg.RcvdAuthCount;
	FloodFrameCount[1] = pAd->ApCfg.RcvdAssocReqCount;
	FloodFrameCount[2] = pAd->ApCfg.RcvdReassocReqCount;
	FloodFrameCount[3] = pAd->ApCfg.RcvdProbeReqCount;
	FloodFrameCount[4] = pAd->ApCfg.RcvdDisassocCount;
	FloodFrameCount[5] = pAd->ApCfg.RcvdDeauthCount;
	FloodFrameCount[6] = pAd->ApCfg.RcvdEapReqCount;
	FloodFrameThreshold[0] = pAd->ApCfg.AuthFloodThreshold;
	FloodFrameThreshold[1] = pAd->ApCfg.AssocReqFloodThreshold;
	FloodFrameThreshold[2] = pAd->ApCfg.ReassocReqFloodThreshold;
	FloodFrameThreshold[3] = pAd->ApCfg.ProbeReqFloodThreshold;
	FloodFrameThreshold[4] = pAd->ApCfg.DisassocFloodThreshold;
	FloodFrameThreshold[5] = pAd->ApCfg.DeauthFloodThreshold;
	FloodFrameThreshold[6] = pAd->ApCfg.EapReqFloodThreshold;

	/* trigger flooding traffic event */
	for (j = 0; j < IW_FLOOD_EVENT_TYPE_NUM; j++) {
		if ((FloodFrameThreshold[j] > 0) && (FloodFrameCount[j] > FloodFrameThreshold[j])) {
			RTMPSendWirelessEvent(pAd, IW_FLOOD_AUTH_EVENT_FLAG + j, NULL, MAIN_MBSSID, 0);
			/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("flooding traffic event(%d) - %d\n", IW_FLOOD_AUTH_EVENT_FLAG + j, FloodFrameCount[j])); */
		}
	}

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		UINT32	SpoofedFrameCount[IW_SPOOF_EVENT_TYPE_NUM];
		CHAR	RssiOfSpoofedFrame[IW_SPOOF_EVENT_TYPE_NUM];
		INT		k;

		SpoofedFrameCount[0] = pAd->ApCfg.MBSSID[i].RcvdConflictSsidCount;
		SpoofedFrameCount[1] = pAd->ApCfg.MBSSID[i].RcvdSpoofedAssocRespCount;
		SpoofedFrameCount[2] = pAd->ApCfg.MBSSID[i].RcvdSpoofedReassocRespCount;
		SpoofedFrameCount[3] = pAd->ApCfg.MBSSID[i].RcvdSpoofedProbeRespCount;
		SpoofedFrameCount[4] = pAd->ApCfg.MBSSID[i].RcvdSpoofedBeaconCount;
		SpoofedFrameCount[5] = pAd->ApCfg.MBSSID[i].RcvdSpoofedDisassocCount;
		SpoofedFrameCount[6] = pAd->ApCfg.MBSSID[i].RcvdSpoofedAuthCount;
		SpoofedFrameCount[7] = pAd->ApCfg.MBSSID[i].RcvdSpoofedDeauthCount;
		SpoofedFrameCount[8] = pAd->ApCfg.MBSSID[i].RcvdSpoofedUnknownMgmtCount;
		SpoofedFrameCount[9] = pAd->ApCfg.MBSSID[i].RcvdReplayAttackCount;
		RssiOfSpoofedFrame[0] = pAd->ApCfg.MBSSID[i].RssiOfRcvdConflictSsid;
		RssiOfSpoofedFrame[1] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAssocResp;
		RssiOfSpoofedFrame[2] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedReassocResp;
		RssiOfSpoofedFrame[3] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedProbeResp;
		RssiOfSpoofedFrame[4] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedBeacon;
		RssiOfSpoofedFrame[5] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDisassoc;
		RssiOfSpoofedFrame[6] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAuth;
		RssiOfSpoofedFrame[7] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDeauth;
		RssiOfSpoofedFrame[8] = pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedUnknownMgmt;
		RssiOfSpoofedFrame[9] = pAd->ApCfg.MBSSID[i].RssiOfRcvdReplayAttack;

		/* trigger spoofed attack event */
		for (k = 0; k < IW_SPOOF_EVENT_TYPE_NUM; k++) {
			if (SpoofedFrameCount[k] > 0) {
				RTMPSendWirelessEvent(pAd, IW_CONFLICT_SSID_EVENT_FLAG + k, NULL, i, RssiOfSpoofedFrame[k]);
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("spoofed attack event(%d) - %d\n", IW_CONFLICT_SSID_EVENT_FLAG + k, SpoofedFrameCount[k])); */
			}
		}
	}
}
#endif /* SYSTEM_LOG_SUPPORT */

VOID RTMPClearAllIdsCounter(
	IN PRTMP_ADAPTER	pAd)
{
	INT	i;

	pAd->ApCfg.RcvdAuthCount = 0;
	pAd->ApCfg.RcvdAssocReqCount = 0;
	pAd->ApCfg.RcvdReassocReqCount = 0;
	pAd->ApCfg.RcvdProbeReqCount = 0;
	pAd->ApCfg.RcvdDisassocCount = 0;
	pAd->ApCfg.RcvdDeauthCount = 0;
	pAd->ApCfg.RcvdEapReqCount = 0;
	pAd->ApCfg.RcvdMaliciousDataCount = 0;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		pAd->ApCfg.MBSSID[i].RcvdConflictSsidCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedAssocRespCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedReassocRespCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedProbeRespCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedBeaconCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedDisassocCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedAuthCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedDeauthCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdSpoofedUnknownMgmtCount = 0;
		pAd->ApCfg.MBSSID[i].RcvdReplayAttackCount = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdConflictSsid = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAssocResp = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedReassocResp = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedProbeResp = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedBeacon = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDisassoc = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAuth = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDeauth = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedUnknownMgmt = 0;
		pAd->ApCfg.MBSSID[i].RssiOfRcvdReplayAttack = 0;
	}
}

VOID RTMPIdsPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	pAd->ApCfg.IDSTimerRunning = FALSE;
#ifdef SYSTEM_LOG_SUPPORT

	/* when IDS occured, send out wireless event */
	if (pAd->CommonCfg.bWirelessEvent)
		RTMPHandleIdsEvent(pAd);

#endif /* SYSTEM_LOG_SUPPORT */
	/* clear all IDS counter */
	RTMPClearAllIdsCounter(pAd);

	/* set timer */
	if (pAd->ApCfg.IdsEnable) {
		RTMPSetTimer(&pAd->ApCfg.IDSTimer, IDS_EXEC_INTV);
		pAd->ApCfg.IDSTimerRunning = TRUE;
	}
}


/*
	========================================================================
	Routine Description:
		This routine is used to check if a rogue AP sent an 802.11 management
		frame to a client using our BSSID.

	Arguments:
		pAd	- Pointer to our adapter
		pHeader - Pointer to 802.11 header

	Return Value:
		TRUE	- This is a spoofed frame
		FALSE	- This isn't a spoofed frame

	========================================================================
*/
BOOLEAN RTMPSpoofedMgmtDetection(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK * rxblk)
{
	INT	i;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)rxblk->FC;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		/* Spoofed BSSID detection */
		if (NdisEqualMemory(rxblk->Addr2, pAd->ApCfg.MBSSID[i].wdev.bssid, MAC_ADDR_LEN)) {
			CHAR RcvdRssi;
			struct raw_rssi_info rssi_info;

			rssi_info.raw_rssi[0] = rxblk->rx_signal.raw_rssi[0];
			rssi_info.raw_rssi[1] = rxblk->rx_signal.raw_rssi[1];
			rssi_info.raw_rssi[2] = rxblk->rx_signal.raw_rssi[2];
			RcvdRssi = RTMPMaxRssi(pAd,
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_0),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_1),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_2));

			switch (FC->SubType) {
			case SUBTYPE_ASSOC_RSP:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedAssocRespCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAssocResp = RcvdRssi;
				break;

			case SUBTYPE_REASSOC_RSP:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedReassocRespCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedReassocResp = RcvdRssi;
				break;

			case SUBTYPE_PROBE_RSP:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedProbeRespCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedProbeResp = RcvdRssi;
				break;

			case SUBTYPE_BEACON:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedBeaconCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedBeacon = RcvdRssi;
				break;

			case SUBTYPE_DISASSOC:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedDisassocCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDisassoc = RcvdRssi;
				break;

			case SUBTYPE_AUTH:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedAuthCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedAuth = RcvdRssi;
				break;

			case SUBTYPE_DEAUTH:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedDeauthCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedDeauth = RcvdRssi;
				break;

			default:
				pAd->ApCfg.MBSSID[i].RcvdSpoofedUnknownMgmtCount++;
				pAd->ApCfg.MBSSID[i].RssiOfRcvdSpoofedUnknownMgmt = RcvdRssi;
				break;
			}

			return TRUE;
		}
	}

	return FALSE;
}


VOID RTMPConflictSsidDetection(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR			pSsid,
	IN UCHAR			SsidLen,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2)
{
	INT	i;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		/* Conflict SSID detection */
		if (SSID_EQUAL(pSsid, SsidLen, pAd->ApCfg.MBSSID[i].Ssid, pAd->ApCfg.MBSSID[i].SsidLen)) {
			CHAR RcvdRssi;
			struct raw_rssi_info rssi_info;

			rssi_info.raw_rssi[0] = Rssi0;
			rssi_info.raw_rssi[1] = Rssi1;
			rssi_info.raw_rssi[2] = Rssi2;
			RcvdRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &rssi_info, RSSI_IDX_0),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_1),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_2));
			pAd->ApCfg.MBSSID[i].RcvdConflictSsidCount++;
			pAd->ApCfg.MBSSID[i].RssiOfRcvdConflictSsid = RcvdRssi;
			return;
		}
	}
}


BOOLEAN RTMPReplayAttackDetection(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pAddr2,
	IN RX_BLK * rxblk)
{
	INT	i;

	for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
		/* Conflict SSID detection */
		if (NdisEqualMemory(pAddr2, pAd->ApCfg.MBSSID[i].wdev.bssid, MAC_ADDR_LEN)) {
			CHAR RcvdRssi;
			struct raw_rssi_info rssi_info;

			rssi_info.raw_rssi[0] = rxblk->rx_signal.raw_rssi[0];
			rssi_info.raw_rssi[1] = rxblk->rx_signal.raw_rssi[1];
			rssi_info.raw_rssi[2] = rxblk->rx_signal.raw_rssi[2];
			RcvdRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &rssi_info, RSSI_IDX_0),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_1),
								   ConvertToRssi(pAd, &rssi_info, RSSI_IDX_2));
			pAd->ApCfg.MBSSID[i].RcvdReplayAttackCount++;
			pAd->ApCfg.MBSSID[i].RssiOfRcvdReplayAttack = RcvdRssi;
			return TRUE;
		}
	}

	return FALSE;
}

VOID RTMPUpdateStaMgmtCounter(RTMP_ADAPTER *pAd, USHORT type)
{
	switch (type) {
	case SUBTYPE_ASSOC_REQ:
		pAd->ApCfg.RcvdAssocReqCount++;
		break;

	case SUBTYPE_REASSOC_REQ:
		pAd->ApCfg.RcvdReassocReqCount++;
		break;

	case SUBTYPE_PROBE_REQ:
		pAd->ApCfg.RcvdProbeReqCount++;
		break;

	case SUBTYPE_DISASSOC:
		pAd->ApCfg.RcvdDisassocCount++;
		break;

	case SUBTYPE_DEAUTH:
		pAd->ApCfg.RcvdDeauthCount++;
		break;

	case SUBTYPE_AUTH:
		pAd->ApCfg.RcvdAuthCount++;
		break;
	}

	/*
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdAssocReqCount=%d\n", pAd->ApCfg.RcvdAssocReqCount));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdReassocReqCount=%d\n", pAd->ApCfg.RcvdReassocReqCount));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdProbeReqCount=%d\n", pAd->ApCfg.RcvdProbeReqCount));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdDisassocCount=%d\n", pAd->ApCfg.RcvdDisassocCount));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdDeauthCount=%d\n", pAd->ApCfg.RcvdDeauthCount));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RcvdAuthCount=%d\n", pAd->ApCfg.RcvdAuthCount));
	*/
}

VOID rtmp_read_ids_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	/*IdsEnable */
	if (RTMPGetKeyParameter("IdsEnable", tmpbuf, 10, buffer, TRUE)) {
		if (os_str_tol(tmpbuf, 0, 10) == 1)
			pAd->ApCfg.IdsEnable = TRUE;
		else
			pAd->ApCfg.IdsEnable = FALSE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IDS is %s\n", pAd->ApCfg.IdsEnable ? "enabled" : "disabled"));
	}

	/*AuthFloodThreshold */
	if (RTMPGetKeyParameter("AuthFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.AuthFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AuthFloodThreshold = %d\n", pAd->ApCfg.AuthFloodThreshold));
	}

	/*AssocReqFloodThreshold */
	if (RTMPGetKeyParameter("AssocReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.AssocReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AssocReqFloodThreshold = %d\n", pAd->ApCfg.AssocReqFloodThreshold));
	}

	/*ReassocReqFloodThreshold */
	if (RTMPGetKeyParameter("ReassocReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.ReassocReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ReassocReqFloodThreshold = %d\n", pAd->ApCfg.ReassocReqFloodThreshold));
	}

	/*ProbeReqFloodThreshold */
	if (RTMPGetKeyParameter("ProbeReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.ProbeReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ProbeReqFloodThreshold = %d\n", pAd->ApCfg.ProbeReqFloodThreshold));
	}

	/*DisassocFloodThreshold */
	if (RTMPGetKeyParameter("DisassocFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DisassocFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DisassocFloodThreshold = %d\n", pAd->ApCfg.DisassocFloodThreshold));
	}

	/*DeauthFloodThreshold */
	if (RTMPGetKeyParameter("DeauthFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DeauthFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DeauthFloodThreshold = %d\n", pAd->ApCfg.DeauthFloodThreshold));
	}

	/*EapReqFloodThreshold */
	if (RTMPGetKeyParameter("EapReqFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.EapReqFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EapReqFloodThreshold = %d\n", pAd->ApCfg.EapReqFloodThreshold));
	}

	/* DataFloodThreshold  */
	if (RTMPGetKeyParameter("DataFloodThreshold", tmpbuf, 10, buffer, TRUE)) {
		pAd->ApCfg.DataFloodThreshold = os_str_tol(tmpbuf, 0, 10);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DataFloodThreshold = %d\n", pAd->ApCfg.DataFloodThreshold));
	}
}

#endif /* IDS_SUPPORT */
