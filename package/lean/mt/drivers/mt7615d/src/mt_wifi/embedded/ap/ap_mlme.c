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
    mlme.c

    Abstract:
    Major MLME state machiones here

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#include <stdarg.h>

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
extern UCHAR  ZeroSsid[32];
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef DOT11_N_SUPPORT

int DetectOverlappingPeriodicRound;


#ifdef DOT11N_DRAFT3
VOID Bss2040CoexistTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	int apidx;
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Bss2040CoexistTimeOut(): Recovery to original setting!\n"));
	/* Recovery to original setting when next DTIM Interval. */
	pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_TIMER_FIRED);
	NdisZeroMemory(&pAd->CommonCfg.LastBSSCoexist2040, sizeof(BSS_2040_COEXIST_IE));
	pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

	if (pAd->CommonCfg.bBssCoexEnable == FALSE) {
		/* TODO: Find a better way to handle this when the timer is fired and we disable the bBssCoexEable support!! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Bss2040CoexistTimeOut(): bBssCoexEnable is FALSE, return directly!\n"));
		return;
	}

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
		SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID_TO_REMOVE, apidx, 0);
}
#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */


VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
#ifdef DOT11_N_SUPPORT
	PRTMP_ADAPTER	pAd = (RTMP_ADAPTER *)FunctionContext;
	struct freq_oper oper;
	BOOLEAN bSupport2G = HcIsRfSupport(pAd, RFIC_24GHZ);
	int i;
	struct wifi_dev *wdev;
	UCHAR cfg_ht_bw;
	UCHAR cfg_ext_cha;

	if (DetectOverlappingPeriodicRound == 0) {
		/* switch back 20/40 */
		if (bSupport2G) {
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev = &pAd->ApCfg.MBSSID[i].wdev;
				cfg_ht_bw = wlan_config_get_ht_bw(wdev);
				if (wmode_2_rfic(wdev->PhyMode) == RFIC_24GHZ && (cfg_ht_bw == HT_BW_40)) {
					cfg_ext_cha = wlan_config_get_ext_cha(wdev);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, cfg_ext_cha);
				}
			}
		}
	} else {
		if ((DetectOverlappingPeriodicRound == 25) || (DetectOverlappingPeriodicRound == 1)) {
			if (hc_radio_query_by_rf(pAd, RFIC_24GHZ, &oper) != HC_STATUS_OK) {
				return;
			}
			if (oper.ht_bw == HT_BW_40) {
				SendBeaconRequest(pAd, 1);
				SendBeaconRequest(pAd, 2);
				SendBeaconRequest(pAd, 3);
			}
		}

		DetectOverlappingPeriodicRound--;
	}

#endif /* DOT11_N_SUPPORT */
}

/*
    ==========================================================================
    Description:
	This routine is executed every second -
	1. Decide the overall channel quality
	2. Check if need to upgrade the TX rate to any client
	3. perform MAC table maintenance, including ageout no-traffic clients,
	   and release packet buffer in PSQ is fail to TX in time.
    ==========================================================================
 */
VOID APMlmePeriodicExec(
	PRTMP_ADAPTER pAd)
{
#ifdef A_BAND_SUPPORT
	BOOLEAN bSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);

#ifdef MT_DFS_SUPPORT
	USHORT ChannelMovingTime;
#endif
#endif /*A_BAND_SUPPORT*/
#ifdef A4_CONN
	UCHAR mbss_idx;
#endif
#ifdef CUSTOMER_DCC_FEATURE
	if (pAd->AllowedStaList.StaCount > 0)
		RemoveOldStaList(pAd);
	APResetStreamingStatus(pAd);
#endif
#ifdef CUSTOMER_DCC_FEATURE
	if (pAd->ApEnableBeaconTable == TRUE)
		RemoveOldBssEntry(pAd);
#endif

	/*
		Reqeust by David 2005/05/12
		It make sense to disable Adjust Tx Power on AP mode, since we can't
		take care all of the client's situation
		ToDo: need to verify compatibility issue with WiFi product.
	*/
#ifdef CARRIER_DETECTION_SUPPORT

	if (isCarrierDetectExist(pAd) == TRUE) {
		PCARRIER_DETECTION_STRUCT pCarrierDetect = &pAd->CommonCfg.CarrierDetect;

		if (pCarrierDetect->OneSecIntCount < pCarrierDetect->CarrierGoneThreshold) {
			pCarrierDetect->CD_State = CD_NORMAL;
			pCarrierDetect->recheck = pCarrierDetect->recheck1;

			if (pCarrierDetect->Debug != DBG_LVL_TRACE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Carrier gone\n"));
				/* start all TX actions. */
				UpdateBeaconHandler(
					pAd,
					NULL,
					BCN_UPDATE_AP_RENEW);
				AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0, OPMODE_AP);
			} else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Carrier gone\n"));
		}

		pCarrierDetect->OneSecIntCount = 0;
	}

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef VOW_SUPPORT
	vow_display_info_periodic(pAd);
#endif /* VOW_SUPPORT */
#ifdef RED_SUPPORT
	if (pAd->red_debug_en && (pAd->red_have_cr4 == FALSE))
		appShowRedDebugMessage(pAd);
#endif /* RED_SUPPORT */
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.dbg_en)
		app_show_fq_dbgmsg(pAd);
#endif /* RRSCH_SUPPORT */

	RTMP_CHIP_HIGH_POWER_TUNING(pAd, &pAd->ApCfg.RssiSample);
	/* Disable Adjust Tx Power for WPA WiFi-test. */
	/* Because high TX power results in the abnormal disconnection of Intel BG-STA. */
	/*#ifndef WIFI_TEST */
	/*	if (pAd->CommonCfg.bWiFiTest == FALSE) */
	/* for SmartBit 64-byte stream test */
	/* removed based on the decision of Ralink congress at 2011/7/06 */
	/*	if (pAd->MacTab.Size > 0) */
	RTMP_CHIP_ASIC_ADJUST_TX_POWER(pAd);
	/*#endif // WIFI_TEST */
	RTMP_CHIP_ASIC_TEMPERATURE_COMPENSATION(pAd);
	/* walk through MAC table, see if switching TX rate is required */

	/* MAC table maintenance */
	if (pAd->Mlme.PeriodicRound % MLME_TASK_EXEC_MULTIPLE == 0) {
		/* one second timer */
		MacTableMaintenance(pAd);

		RTMPMaintainPMKIDCache(pAd);
#ifdef WDS_SUPPORT
		WdsTableMaintenance(pAd);
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		CliWds_ProxyTabMaintain(pAd);
#endif /* CLIENT_WDS */
#ifdef A4_CONN
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++)
			a4_proxy_maintain(pAd, mbss_idx);
		pAd->a4_need_refresh = FALSE;
#endif /* A4_CONN */

#ifdef WIFI_DIAG
		DiagApMlmeOneSecProc(pAd);
#endif

	}

#ifdef AP_SCAN_SUPPORT
	AutoChannelSelCheck(pAd);
#endif /* AP_SCAN_SUPPORT */
#ifdef APCLI_SUPPORT

	if (pAd->Mlme.OneSecPeriodicRound % 2 == 0)
		ApCliIfMonitor(pAd);

	if ((pAd->Mlme.OneSecPeriodicRound % 2 == 1
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		&& (pAd->ApCfg.ApCliAutoConnectChannelSwitching == FALSE)
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	   ) ||
		(pAd->Mlme.OneSecPeriodicRound % 2 == 1
#ifdef CONFIG_MAP_SUPPORT
		&& (IS_MAP_TURNKEY_ENABLE(pAd))
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	   )) {
		ApCliIfUp(pAd);
	}

	{
		INT loop;
		ULONG Now32;

#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn)
			RTMPRepeaterReconnectionCheck(pAd);

#endif /* MAC_REPEATER_SUPPORT */

		NdisGetSystemUpTime(&Now32);

		for (loop = 0; loop < MAX_APCLI_NUM; loop++) {
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
#ifdef APCLI_CERT_SUPPORT

			if ((pApCliEntry->bBlockAssoc == TRUE) &&
				RTMP_TIME_AFTER(Now32, pApCliEntry->LastMicErrorTime + (60*OS_HZ)))
				pAd->ApCfg.ApCliTab[loop].bBlockAssoc = FALSE;

#endif /* APCLI_CERT_SUPPORT */

			if ((pApCliEntry->Valid == TRUE)
				&& (VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))) {
				/* update channel quality for Roaming and UI LinkQuality display */
				MlmeCalculateChannelQuality(pAd,
											&pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
			}
		}
	}
#endif /* APCLI_SUPPORT */
#ifdef DOT11_N_SUPPORT
		{
			INT IdBss = 0;
			UCHAR ht_protect_en = 1;
			BSS_STRUCT *pMbss = NULL;

			for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
				pMbss = &pAd->ApCfg.MBSSID[IdBss];

				if ((pMbss) && (&pMbss->wdev) && (pMbss->wdev.DevInfo.Active)) {
					ht_protect_en = wlan_config_get_ht_protect_en(&pMbss->wdev);
					if (ht_protect_en) {
						ApUpdateCapabilityAndErpIe(pAd, pMbss);
						APUpdateOperationMode(pAd, &pMbss->wdev);
					}
				}
			}
		}
#endif /* DOT11_N_SUPPORT */

#ifdef A_BAND_SUPPORT
	if (bSupport5G && (pAd->CommonCfg.bIEEE80211H == 1)) {
		INT IdBss = 0;
		BOOLEAN BandInCac[DBDC_BAND_NUM];
		UCHAR i;
		BSS_STRUCT *pMbss = NULL;
		struct DOT11_H *pDot11hTest = NULL;
		struct wifi_dev *wdev;
		UCHAR BandIdx;

		for (i = 0; i < DBDC_BAND_NUM; i++)
			BandInCac[i] = FALSE;

		for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			wdev = &pMbss->wdev;

			if ((pMbss == NULL) || (wdev == NULL))
				continue;
			if (wdev->pHObj == NULL)
				continue;

			BandIdx = HcGetBandByWdev(wdev);
			pDot11hTest = &pAd->Dot11_H[BandIdx];

			if (pDot11hTest == NULL)
				continue;
#ifdef MT_DFS_SUPPORT
			if (pDot11hTest->RDMode == RD_SILENCE_MODE) {
				if (BandInCac[BandIdx] == TRUE)
					continue;
				else
					BandInCac[BandIdx] = TRUE;
#ifdef BACKGROUND_SCAN_SUPPORT
				if (IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) == TRUE)
					ChannelMovingTime = pDot11hTest->DfsZeroWaitChMovingTime;
				else
#endif
				{
					ChannelMovingTime = pDot11hTest->ChMovingTime;
				}

				if (pDot11hTest->RDCount++ > ChannelMovingTime) {
					pDot11hTest->RDCount = 0;
					MlmeEnqueue(pAd, DFS_STATE_MACHINE, DFS_CAC_END, 0, NULL, HcGetBandByWdev(wdev));
					AsicSetSyncModeAndEnable(pAd, pAd->CommonCfg.BeaconPeriod, HW_BSSID_0,  OPMODE_AP);
					pDot11hTest->RDMode = RD_NORMAL_MODE;
				}
			} else
#endif
			{
				pDot11hTest->InServiceMonitorCount++;
			}
		}
	}
#endif /* A_BAND_SUPPORT */

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyCountDown(pAd);
	DfsOutBandCacCountUpdate(pAd);
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	DfsV10W56APDownTimeCountDown(pAd);
#endif
#endif
#ifdef MBO_SUPPORT
	MboCheckBssTermination(pAd);
#endif /* MBO_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_R1KHInfoMaintenance(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#ifdef APCLI_CERT_SUPPORT
	/* Perform 20/40 BSS COEX scan every Dot11BssWidthTriggerScanInt	*/
	{
		UCHAR apcli2Gidx = 0;
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			apcli2Gidx = 1;
#endif
		if (APCLI_IF_UP_CHECK(pAd, apcli2Gidx) && (pAd->bApCliCertTest == TRUE)) {
			if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) &&
				(pAd->CommonCfg.Dot11BssWidthTriggerScanInt != 0) &&
				((pAd->Mlme.OneSecPeriodicRound % pAd->CommonCfg.Dot11BssWidthTriggerScanInt) == (pAd->CommonCfg.Dot11BssWidthTriggerScanInt-1))) {
#ifdef MT7615
				if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
					PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[BSS0];
					MAC_TABLE_ENTRY *pEntry = NULL;
					STA_TR_ENTRY *tr_entry = NULL;
					UINT tx_tp = 0;
					UINT rx_tp = 0;

					if (pApCliEntry->Valid == TRUE) {
						pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
						tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
					}

					if ((pEntry) && IS_ENTRY_APCLI(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
						tx_tp = ((pApCliEntry->OneSecTxBytes) >> BYTES_PER_SEC_TO_MBPS);
						rx_tp = ((pApCliEntry->OneSecRxBytes) >> BYTES_PER_SEC_TO_MBPS);
					}

					/* Check last scan time at least 30 seconds from now.		*/
					/* Check traffic is less than about 1.5~2Mbps.*/
					/* it might cause data lost if we enqueue scanning.*/
					/* This criteria needs to be considered*/

					if ((tx_tp < 1) && (rx_tp < 1)) {
						MLME_SCAN_REQ_STRUCT ScanReq;
						/* Fill out stuff for scan request and kick to scan*/
						ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
						/* Before scan, reset trigger event table. */
						TriEventInit(pAd);
						MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, (ULONG)(&pApCliEntry->wdev));
						/* Set InfoReq = 1, So after scan , alwats sebd 20/40 Coexistence frame to AP*/
						pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
						RTMP_MLME_HANDLER(pAd);
					}
				} else
#endif /* MT7615 */
				{
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("MMCHK - LastOneSecTotalTxCount/LastOneSecRxOkDataCnt  = %d/%d\n",
						pAd->RalinkCounters.LastOneSecTotalTxCount,
						pAd->RalinkCounters.LastOneSecRxOkDataCnt));

					/* Check last scan time at least 30 seconds from now.		*/
					/* Check traffic is less than about 1.5~2Mbps.*/
					/* it might cause data lost if we enqueue scanning.*/
					/* This criteria needs to be considered*/
					if ((pAd->RalinkCounters.LastOneSecTotalTxCount < 70) && (pAd->RalinkCounters.LastOneSecRxOkDataCnt < 70)) {
						MLME_SCAN_REQ_STRUCT ScanReq;
						/* Fill out stuff for scan request and kick to scan*/
						ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
						/* Before scan, reset trigger event table. */
						TriEventInit(pAd);
						MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
						/* Set InfoReq = 1, So after scan , alwats sebd 20/40 Coexistence frame to AP*/
						pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
						RTMP_MLME_HANDLER(pAd);
					}

					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("MMCHK - LastOneSecTotalTxCount/LastOneSecRxOkDataCnt	= %d/%d\n",
						pAd->RalinkCounters.LastOneSecTotalTxCount,
						pAd->RalinkCounters.LastOneSecRxOkDataCnt));
				}
			}
		}
	}
#endif /* APCLI_CERT_SUPPORT */
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef BAND_STEERING
	BndStrgHeartBeatMonitor(pAd);
#endif
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN APMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType)
{
	USHORT Seq;
#ifdef DOT11_SAE_SUPPORT
	USHORT Alg;
#endif /* DOT11_SAE_SUPPORT */
	UCHAR  EAPType;
	BOOLEAN     Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
	PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;

#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	/*
		TODO:
		only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid;
		otherwise, ignore this frame
	*/

	/* wpa EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		WSC_CTRL *wsc_ctrl;
		struct wifi_dev *wdev;

		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry) {
			wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

			if (pEntry->bWscCapable
				|| IS_AKM_OPEN(wdev->SecConfig.AKMMap)
				|| IS_AKM_SHARED(wdev->SecConfig.AKMMap)
				|| IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap)) {
				/*
					WSC AP only can service one WSC STA in one WPS session.
					Forward this EAP packet to WSC SM if this EAP packets is from
					WSC STA that WSC AP services or WSC AP doesn't service any
					WSC STA now.
				*/
				wsc_ctrl = &wdev->WscControl;

				if ((MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, pEntry->Addr) ||
					 MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)) &&
					IS_ENTRY_CLIENT(pEntry) &&
					(wsc_ctrl->WscConfMode != WSC_DISABLE)) {
					*Machine = WSC_STATE_MACHINE;
					EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
					EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
					Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
				}
			}
		}

#endif /* WSC_AP_SUPPORT */

		if (!Return) {
			*Machine = WPA_STATE_MACHINE;
			EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
			Return = WpaMsgTypeSubst(EAPType, (INT *) MsgType);
		}

		return Return;
	}

	if (pFrame->Hdr.FC.Type != FC_TYPE_MGMT)
		return FALSE;

	switch (pFrame->Hdr.FC.SubType) {
	case SUBTYPE_ASSOC_REQ:
		*Machine = AP_ASSOC_STATE_MACHINE;
		*MsgType = APMT2_PEER_ASSOC_REQ;
		break;

	/*
	case SUBTYPE_ASSOC_RSP:
		*Machine = AP_ASSOC_STATE_MACHINE;
		*MsgType = APMT2_PEER_ASSOC_RSP;
		break;
	*/
	case SUBTYPE_REASSOC_REQ:
		*Machine = AP_ASSOC_STATE_MACHINE;
		*MsgType = APMT2_PEER_REASSOC_REQ;
		break;

	/*
	case SUBTYPE_REASSOC_RSP:
		*Machine = AP_ASSOC_STATE_MACHINE;
		*MsgType = APMT2_PEER_REASSOC_RSP;
		break;
	*/
	case SUBTYPE_PROBE_REQ:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_PROBE_REQ;
		break;

	/* For Active Scan */
	case SUBTYPE_PROBE_RSP:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_PROBE_RSP;
		break;

	case SUBTYPE_BEACON:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_BEACON;
		break;

	/*
	case SUBTYPE_ATIM:
		*Machine = AP_SYNC_STATE_MACHINE;
		*MsgType = APMT2_PEER_ATIM;
		break;
	*/
	case SUBTYPE_DISASSOC:
		*Machine = AP_ASSOC_STATE_MACHINE;
		*MsgType = APMT2_PEER_DISASSOC_REQ;
		break;

	case SUBTYPE_AUTH:
		/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
#ifdef DOT11_SAE_SUPPORT
		NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* DOT11_SAE_SUPPORT */
		NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
		*Machine = AP_AUTH_STATE_MACHINE;

		if (Seq == 1
#ifdef DOT11_SAE_SUPPORT
			|| (Alg == AUTH_MODE_SAE && Seq == 2)
#endif /* DOT11_SAE_SUPPORT */
			)
			*MsgType = APMT2_PEER_AUTH_REQ;
		else if (Seq == 3)
			*MsgType = APMT2_PEER_AUTH_CONFIRM;
		else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("wrong AUTH seq=%d Octet=%02x %02x %02x %02x %02x %02x %02x %02x\n",
					 Seq,
					 pFrame->Octet[0], pFrame->Octet[1], pFrame->Octet[2], pFrame->Octet[3],
					 pFrame->Octet[4], pFrame->Octet[5], pFrame->Octet[6], pFrame->Octet[7]));
			return FALSE;
		}

		break;

	case SUBTYPE_DEAUTH:
		*Machine = AP_AUTH_STATE_MACHINE; /*AP_AUTH_RSP_STATE_MACHINE;*/
		*MsgType = APMT2_PEER_DEAUTH;
		break;

	case SUBTYPE_ACTION:
	case SUBTYPE_ACTION_NO_ACK:
		*Machine = ACTION_STATE_MACHINE;
		/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
			if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG)
				*MsgType = MT2_ACT_INVALID;
			else
				*MsgType = (pFrame->Octet[0]&0x7F);

		break;

	default:
		return FALSE;
	}

	return TRUE;
}


/*
    ========================================================================
    Routine Description:
	Periodic evaluate antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicEvaluateRxAnt(
	IN PRTMP_ADAPTER	pAd)
{
	ULONG	TxTotalCnt;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.CD_State == CD_SILENCE)
		return;

#endif /* CARRIER_DETECTION_SUPPORT */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);
	TxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
				 pAd->RalinkCounters.OneSecTxRetryOkCount +
				 pAd->RalinkCounters.OneSecTxFailCount;

	if (TxTotalCnt > 50) {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 20);
		pAd->Mlme.bLowThroughput = FALSE;
	} else {
		RTMPSetTimer(&pAd->Mlme.RxAntEvalTimer, 300);
		pAd->Mlme.bLowThroughput = TRUE;
	}
}

/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd)
{
	CHAR rssi[3], *target_rssi;
#ifdef CONFIG_ATE

	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */

	/* if the traffic is low, use average rssi as the criteria */
	if (pAd->Mlme.bLowThroughput == TRUE)
		target_rssi = &pAd->ApCfg.RssiSample.LastRssi[0];
	else
		target_rssi = &pAd->ApCfg.RssiSample.AvgRssi[0];

	NdisMoveMemory(&rssi[0], target_rssi, 3);
	/* Disable the below to fix 1T/2R issue. It's suggested by Rory at 2007/7/11. */
	bbp_set_rxpath(pAd, pAd->Mlme.RealRxPath);
}


/*
    ========================================================================
    Routine Description:
	After evaluation, check antenna link status

    Arguments:
	pAd         - Adapter pointer

    Return Value:
	None

    ========================================================================
*/
VOID	APAsicAntennaAvg(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	              AntSelect,
	IN	SHORT * RssiAvg)
{
	SHORT	realavgrssi;
	LONG         realavgrssi1;
	ULONG	recvPktNum = pAd->RxAnt.RcvPktNum[AntSelect];

	realavgrssi1 = pAd->RxAnt.Pair1AvgRssiGroup1[AntSelect];

	if (realavgrssi1 == 0) {
		*RssiAvg = 0;
		return;
	}

	realavgrssi = (SHORT) (realavgrssi1 / recvPktNum);
	pAd->RxAnt.Pair1AvgRssiGroup1[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[0] = 0;
	pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
	pAd->RxAnt.RcvPktNum[0] = 0;
	pAd->RxAnt.RcvPktNum[1] = 0;
	*RssiAvg = realavgrssi - 256;
}

