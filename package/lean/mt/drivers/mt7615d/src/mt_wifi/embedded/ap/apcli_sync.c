/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sta_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"


static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	OUT PCHAR Ssid,
	IN USHORT ifIndex);

DECLARE_TIMER_FUNCTION(ApCliProbeTimeout);
BUILD_TIMER_FUNCTION(ApCliProbeTimeout);

/*
    ==========================================================================
    Description:
	The sync state machine,
    Parameters:
	Sm - pointer to the state machine
    Note:
	the state machine looks like the following
    ==========================================================================
 */
VOID ApCliSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
	PAPCLI_STRUCT	pApCliEntry;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans,
					 APCLI_MAX_SYNC_STATE, APCLI_MAX_SYNC_MSG,
					 (STATE_MACHINE_FUNC)Drop, APCLI_SYNC_IDLE,
					 APCLI_SYNC_MACHINE_BASE);
	/* column 1 */
	StateMachineSetAction(Sm, APCLI_SYNC_IDLE, APCLI_MT2_MLME_PROBE_REQ, (STATE_MACHINE_FUNC)ApCliMlmeProbeReqAction);
	/*column 2 */
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_MLME_PROBE_REQ,
						  (STATE_MACHINE_FUNC)ApCliInvalidStateWhenJoin);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PEER_PROBE_RSP,
						  (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtJoinAction);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PROBE_TIMEOUT,
						  (STATE_MACHINE_FUNC)ApCliProbeTimeoutAtJoinAction);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PEER_BEACON,
						  (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtJoinAction);

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		/* timer init */
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ProbeTimer, GET_TIMER_FUNCTION(ApCliProbeTimeout),
					  (PVOID)pApCliEntry, FALSE);
		pAd->ApCfg.ApCliTab[i].SyncCurrState = APCLI_SYNC_IDLE;
	}
}

/*
    ==========================================================================
    Description:
	Becaon timeout handler, executed in timer thread
    ==========================================================================
 */
static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	APCLI_STRUCT	 *pAPCli = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAPCli->pAd;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCli_SYNC - ProbeReqTimeout\n"));
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, pAPCli->ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
	MLME PROBE req state machine procedure
    ==========================================================================
 */
static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	BOOLEAN Cancelled;
	APCLI_MLME_JOIN_REQ_STRUCT *Info = (APCLI_MLME_JOIN_REQ_STRUCT *)(Elem->Msg);
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	APCLI_STRUCT *pApCliEntry = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	struct dev_rate_info *rate = &wdev->rate;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCli SYNC - ApCliMlmeProbeReqAction(Ssid %s)\n",
			 Info->Ssid));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	/* reset all the timers */
	RTMPCancelTimer(&(pApCliEntry->MlmeAux.ProbeTimer), &Cancelled);
	pApCliEntry->MlmeAux.Rssi = -9999;
#ifdef APCLI_CONNECTION_TRIAL

	if (pApCliEntry->TrialCh == 0)
		pApCliEntry->MlmeAux.Channel = wdev->channel;
	else
		pApCliEntry->MlmeAux.Channel = pApCliEntry->TrialCh;

#else
	/* TODO: Star, need to modify when Multi-STA Ready! */
	/*Assign Channel for APCLI*/
	pApCliEntry->MlmeAux.Channel = wdev->channel;
#endif /* APCLI_CONNECTION_TRIAL */
	pApCliEntry->MlmeAux.SupRateLen = rate->SupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, rate->SupRate, rate->SupRateLen);
	/* Prepare the default value for extended rate */
	pApCliEntry->MlmeAux.ExtRateLen = rate->ExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, rate->ExtRate, rate->ExtRateLen);
	RTMPSetTimer(&(pApCliEntry->MlmeAux.ProbeTimer), PROBE_TIMEOUT);
#ifdef APCLI_CONNECTION_TRIAL
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, pAd->ApCfg.ApCliTab[ifIndex].CfgSsid,
				   pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen);
#endif /* APCLI_CONNECTION_TRIAL */



#ifdef APCLI_OWE_SUPPORT
/*OWE:clear previously selected ssid and bssid */
	if (IS_AKM_OWE(wdev->SecConfig.AKMMap) && (pApCliEntry->owe_trans_ssid_len > 0)) {
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
	}
#endif


	ApCliEnqueueProbeRequest(pAd, Info->SsidLen, (PCHAR) Info->Ssid, ifIndex);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCli SYNC - Start Probe the SSID %s on channel =%d\n",
			 pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.Channel));
	*pCurrState = APCLI_JOIN_WAIT_PROBE_RSP;
}

/*
    ==========================================================================
    Description:
	When waiting joining the (I)BSS, beacon received from external
    ==========================================================================
 */
static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef DOT11_N_SUPPORT
	UCHAR CentralChannel;
#endif /* DOT11_N_SUPPORT */
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState;
	BCN_IE_LIST *ie_list = NULL;

#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	UCHAR Snr[4] = {0};
	CHAR  rssi[4] = {0};
	Snr[0] = ConvertToSnr(pAd, Elem->rssi_info.raw_Snr[0]);
	Snr[1] = ConvertToSnr(pAd, Elem->rssi_info.raw_Snr[1]);
	Snr[2] = ConvertToSnr(pAd, Elem->rssi_info.raw_Snr[2]);
	Snr[3] = ConvertToSnr(pAd, Elem->rssi_info.raw_Snr[3]);

	rssi[0] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0);
	rssi[1] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1);
	rssi[2] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2);
	rssi[3] = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_3);
#endif


	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);

	if (VarIE == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		goto LabelErr;
	}

	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s: Allocate ie_list fail!!!\n", __func__));
		goto LabelErr;
	}

	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;

	if (PeerBeaconAndProbeRspSanity(pAd,
									Elem->Msg,
									Elem->MsgLen,
									Elem->Channel,
									ie_list,
									&LenVIE,
									pVIE,
									TRUE,
									FALSE)) {
		/*
			BEACON from desired BSS/IBSS found. We should be able to decide most
			BSS parameters here.
			Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
				Do we need to receover back all parameters belonging to previous BSS?
			A. Should be not. There's no back-door recover to previous AP. It still need
				a new JOIN-AUTH-ASSOC sequence.
		*/
		INT ssidEqualFlag = FALSE;
		INT ssidEmptyFlag = FALSE;
		INT bssidEqualFlag = FALSE;
		INT bssidEmptyFlag = FALSE;
		INT matchFlag = FALSE;
		ULONG   Bssidx;
		CHAR Rssi = -127;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(WH_EZ_SETUP) || defined(APCLI_CFG80211_SUPPORT)
		CHAR Rssi0 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0);
		CHAR Rssi1 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1);
		CHAR Rssi2 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2);
		LONG RealRssi = (LONG)(RTMPMaxRssi(pAd, Rssi0, Rssi1, Rssi2));
#endif
		/* Update ScanTab */
		{
			/* discover new AP of this network, create BSS entry */
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, -Rssi, LenVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
						, Snr, rssi
#endif
						);

			if (Bssidx == BSS_NOT_FOUND) { /* return if BSS table full */
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ERROR: Driver ScanTable Full In Apcli ProbeRsp Join\n"));
				goto LabelErr;
			}

			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			pAd->ScanTab.BssEntry[Bssidx].MinSNR = Elem->Signal % 10;

			if (pAd->ScanTab.BssEntry[Bssidx].MinSNR == 0)
				pAd->ScanTab.BssEntry[Bssidx].MinSNR = -5;

			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].MacAddr, ie_list->Addr2, MAC_ADDR_LEN);
		}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)

		/* Determine primary channel by IE's DSPS rather than channel of received frame */
		if (ie_list->Channel != 0)
			Elem->Channel = ie_list->Channel;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Info: Update the SSID %s in Kernel Table\n", ie_list->Ssid));
		RT_CFG80211_SCANNING_INFORM(pAd, Bssidx, ie_list->Channel, (UCHAR *)Elem->Msg, Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA || APCLI_CFG80211_SUPPORT */
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef APCLI_OWE_SUPPORT
		if (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap) && (pApCliEntry->owe_trans_ssid_len > 0)) {
			BSS_ENTRY *popen_bss_entry = NULL;

			popen_bss_entry = &pAd->ScanTab.BssEntry[Bssidx];

			if (!MAC_ADDR_EQUAL(pApCliEntry->owe_trans_bssid, ie_list->Bssid)) {

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ERROR: OWE Transition AP BSSID not equal\n"));
				goto LabelErr;
			}
/*Validate that we are connecting wth the same OWE BSS that was mentioned in the OPEN BSS Transition IE*/
			if (popen_bss_entry->owe_trans_ie_len > 0) {
				UCHAR pair_ch = 0;
				UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
				UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
				UCHAR pair_band = 0;
				UCHAR pair_ssid_len = 0;

				extract_pair_owe_bss_info(popen_bss_entry->owe_trans_ie,
							 popen_bss_entry->owe_trans_ie_len,
							  pair_bssid,
							  pair_ssid,
							  &pair_ssid_len,
							  &pair_band,
							  &pair_ch);

				if (!MAC_ADDR_EQUAL(pApCliEntry->owe_trans_open_bssid, pair_bssid)
					|| !SSID_EQUAL(pApCliEntry->owe_trans_open_ssid, pApCliEntry->owe_trans_open_ssid_len, pair_ssid, pair_ssid_len)) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("OWE:Transition AP Validation Failed\n"));
					goto LabelErr;
				}
			}

		} else
#endif
		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR)) {
			if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid)) {

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ERROR: AP BSSID not equal\n"));
				goto LabelErr;
			}
		}

		/* Check the Probe-Rsp's Bssid. */
		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid);
		else
			bssidEmptyFlag = TRUE;

#ifdef WSC_AP_SUPPORT

		if ((pApCliEntry->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
				(pApCliEntry->wdev.WscControl.bWscTrigger == TRUE)) {
			if (pApCliEntry->wdev.WscControl.WscSsid.SsidLength != 0)
				ssidEqualFlag = SSID_EQUAL(pApCliEntry->wdev.WscControl.WscSsid.Ssid,
				pApCliEntry->wdev.WscControl.WscSsid.SsidLength, ie_list->Ssid, ie_list->SsidLen);
			else
				ssidEmptyFlag = TRUE;

			if (!MAC_ADDR_EQUAL(pApCliEntry->wdev.WscControl.WscBssid, ZERO_MAC_ADDR)) {
				bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->wdev.WscControl.WscBssid, ie_list->Bssid);
				bssidEmptyFlag = FALSE;
			}
		} else
#endif
#ifdef APCLI_OWE_SUPPORT
		if (pApCliEntry->owe_trans_ssid_len > 0) {
			ssidEqualFlag = SSID_EQUAL(pApCliEntry->owe_trans_ssid, pApCliEntry->owe_trans_ssid_len, ie_list->Ssid, ie_list->SsidLen);

			bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->owe_trans_bssid, ie_list->Bssid);


			if (ssidEqualFlag == TRUE)
					ssidEmptyFlag = FALSE;
			if (bssidEqualFlag == TRUE)
					bssidEmptyFlag = FALSE;

		} else
#endif
		{
			/* Check the Probe-Rsp's Ssid. */
			if (pApCliEntry->CfgSsidLen != 0)
				ssidEqualFlag = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, ie_list->Ssid, ie_list->SsidLen);
			else
				ssidEmptyFlag = TRUE;
		}

#ifdef WSC_AP_SUPPORT

		if ((pApCliEntry->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(pApCliEntry->wdev.WscControl.bWscTrigger == TRUE)) {
			/* bssid and ssid, Both match. */
			if (bssidEqualFlag && ssidEqualFlag)
				matchFlag = TRUE;
			/* ssid match but bssid doesn't be indicate. */
			else if (ssidEqualFlag && bssidEmptyFlag)
				matchFlag = TRUE;
			/* user doesn't indicate any bssid or ssid. AP-Clinet will auto pick a AP to join by most strong siganl strength. */
			else if (bssidEmptyFlag && ssidEmptyFlag)
				matchFlag = TRUE;
		} else
#endif /* WSC_AP_SUPPORT */
		{
			if (!bssidEmptyFlag) {
				if (bssidEqualFlag && ssidEqualFlag)
					matchFlag = TRUE;
				else
					matchFlag = FALSE;
			} else if (ssidEqualFlag)
				matchFlag = TRUE;
			else
				matchFlag = FALSE;
		}

		pApCliEntry->wdev.is_marvell_ap = ie_list->is_marvell_ap;

		pApCliEntry->wdev.is_atheros_ap = ie_list->is_atheros_ap;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("SYNC - bssidEqualFlag=%d, ssidEqualFlag=%d, matchFlag=%d\n",
				 bssidEqualFlag, ssidEqualFlag, matchFlag));

		if (matchFlag) {
			/* Validate RSN IE if necessary, then copy store this information */
			if ((LenVIE > 0)
#ifdef WSC_AP_SUPPORT
				&& ((pApCliEntry->wdev.WscControl.WscConfMode == WSC_DISABLE) ||
					(pApCliEntry->wdev.WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
				/* When using CFG80211 and trigger WPS, do not check security. */
				&& !(pApCliEntry->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA || APCLI_CFG80211_SUPPORT */
			   ) {
#ifdef DOT11W_PMF_SUPPORT
				{
					BSS_ENTRY *pInBss = NULL;

					/* init to zero */
					pApCliEntry->MlmeAux.RsnCap.word = 0;
					pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation = FALSE;

					pInBss = &pAd->ScanTab.BssEntry[Bssidx];

					if (pInBss) {
						NdisMoveMemory(&pApCliEntry->MlmeAux.RsnCap, &pInBss->RsnCapability, sizeof(RSN_CAPABILITIES));
						pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation = pInBss->IsSupportSHA256KeyDerivation;
					}
				}
#endif /* DOT11W_PMF_SUPPORT */

				if (ApCliValidateRSNIE(pAd, (PEID_STRUCT)pVIE, LenVIE, ifIndex, CAP_IS_PRIVACY_ON(ie_list->CapabilityInfo))) {
					pApCliEntry->MlmeAux.VarIELen = LenVIE;
					NdisMoveMemory(pApCliEntry->MlmeAux.VarIEs, pVIE, pApCliEntry->MlmeAux.VarIELen);
				} else {
					/* ignore this response */
					pApCliEntry->MlmeAux.VarIELen = 0;
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
							 ("ERROR: The RSN IE of this received Probe-resp is dis-match !!!!!!!!!!\n"));
#ifdef CONFIG_MAP_SUPPORT
					if (IS_MAP_TURNKEY_ENABLE(pAd)) {
						pApCliEntry->Enable = 0;
						wapp_send_apcli_association_change(WAPP_APCLI_DISASSOCIATED, pAd, pApCliEntry);
					}
#endif
					goto LabelErr;
				}
			} else {
				if (IS_AKM_WPA_CAPABILITY_Entry(&pApCliEntry->wdev)
#ifdef WSC_AP_SUPPORT
					&& ((pApCliEntry->wdev.WscControl.WscConfMode == WSC_DISABLE) ||
						(pApCliEntry->wdev.WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
#ifdef APCLI_OWE_SUPPORT
					&& !IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap)
#endif
				   ) {
					/* ignore this response */
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
							 ("ERROR: The received Probe-resp has empty RSN IE !!!!!!!!!!\n"));
					goto LabelErr;
				} else {
#ifdef WSC_AP_SUPPORT

					if ((pApCliEntry->wdev.WscControl.WscConfMode == WSC_DISABLE) ||
						(pApCliEntry->wdev.WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
					{
						struct _SECURITY_CONFIG *pSecConfig;
						UCHAR Privacy = CAP_IS_PRIVACY_ON(ie_list->CapabilityInfo);

						pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
						pSecConfig = &pApCliEntry->wdev.SecConfig;
						/* Clear the data for valid cases only */
						if ((IS_NO_SECURITY(pSecConfig) && (Privacy == 0)) ||
							(IS_SECURITY(pSecConfig) && (Privacy == 1))) {
							CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
							CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
							CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
						}
						if (IS_NO_SECURITY(pSecConfig) && (Privacy == 0)) {
							SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
							SET_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher);
							SET_CIPHER_NONE(pApCliEntry->MlmeAux.GroupCipher);
						} else if (IS_SECURITY(pSecConfig) && (Privacy == 1)) {
							/* WEP mode */
							if (IS_AKM_AUTOSWITCH(pApCliEntry->wdev.SecConfig.AKMMap))
								SET_AKM_AUTOSWITCH(pApCliEntry->MlmeAux.AKMMap);
							else if (IS_AKM_OPEN(pApCliEntry->wdev.SecConfig.AKMMap))
								SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
							else if (IS_AKM_SHARED(pApCliEntry->wdev.SecConfig.AKMMap))
								SET_AKM_SHARED(pApCliEntry->MlmeAux.AKMMap);
							else
								SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);

							SET_CIPHER_WEP(pApCliEntry->MlmeAux.PairwiseCipher);
							SET_CIPHER_WEP(pApCliEntry->MlmeAux.GroupCipher);
						}
#ifdef APCLI_OWE_SUPPORT
						else if (IS_AKM_OWE(pSecConfig->AKMMap) && (Privacy == 0)) {
							CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
							CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
							CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
							SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
							SET_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher);
							SET_CIPHER_NONE(pApCliEntry->MlmeAux.GroupCipher);

						}
#endif
						else {
							/* Ignoring the probe response with privacy
							 * not matching our configured Security config
							 * No need to process further */
							MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
											("Privacy mis-match !!!!!!!!!!\n"));
							goto LabelErr;
						}

						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
								 ("%s(): Candidate Security AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s\n",
								  __func__,
								  GetAuthModeStr(pApCliEntry->MlmeAux.AKMMap),
								  GetEncryModeStr(pApCliEntry->MlmeAux.PairwiseCipher),
								  GetEncryModeStr(pApCliEntry->MlmeAux.GroupCipher)));
					}
				}

				pApCliEntry->MlmeAux.VarIELen = 0;
			}

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					 ("SYNC - receive desired PROBE_RSP at JoinWaitProbeRsp... Channel = %d\n",
					  ie_list->Channel));

			/* if the Bssid doesn't be indicated then you need to decide which AP to connect by most strong Rssi signal strength. */
			if (bssidEqualFlag == FALSE) {
				/* caculate real rssi value. */
				CHAR Rssi0 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0);
				CHAR Rssi1 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1);
				CHAR Rssi2 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2);
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
				CHAR Rssi3 = ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_3);
				LONG RealRssi = (LONG)(RTMPMaxRssi(pAd, Rssi0, Rssi1, Rssi2, Rssi3));
#else
				LONG RealRssi = (LONG)(RTMPMaxRssi(pAd, Rssi0, Rssi1, Rssi2));
#endif

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("SYNC - previous Rssi = %ld current Rssi=%ld\n",
						 pApCliEntry->MlmeAux.Rssi, (LONG)RealRssi));

				if (pApCliEntry->MlmeAux.Rssi > (LONG)RealRssi)
					goto LabelErr;
				else
					pApCliEntry->MlmeAux.Rssi = RealRssi;
				if (ie_list->Channel != pApCliEntry->MlmeAux.Channel) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("SYNC - current ie channel=%d, apcli channel=%d!\n",
							 ie_list->Channel, pApCliEntry->MlmeAux.Channel));
					goto LabelErr;
				}
			} else {
				BOOLEAN Cancelled;

				RTMPCancelTimer(&pApCliEntry->MlmeAux.ProbeTimer, &Cancelled);
			}

			NdisMoveMemory(pApCliEntry->MlmeAux.Ssid, ie_list->Ssid, ie_list->SsidLen);
			pApCliEntry->MlmeAux.SsidLen = ie_list->SsidLen;
			NdisMoveMemory(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid, MAC_ADDR_LEN);
			pApCliEntry->MlmeAux.CapabilityInfo = ie_list->CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pApCliEntry->MlmeAux.BssType = ie_list->BssType;
			pApCliEntry->MlmeAux.BeaconPeriod = ie_list->BeaconPeriod;
			pApCliEntry->MlmeAux.Channel = ie_list->Channel;
			pApCliEntry->MlmeAux.AtimWin = ie_list->AtimWin;
			pApCliEntry->MlmeAux.CfpPeriod = ie_list->CfParm.CfpPeriod;
			pApCliEntry->MlmeAux.CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			NdisMoveMemory(&pApCliEntry->MlmeAux.vendor_ie,
						   &ie_list->vendor_ie, sizeof(struct _vendor_ie_cap));
			/* Copy AP's supported rate to MlmeAux for creating assoication request */
			/* Also filter out not supported rate */
			pApCliEntry->MlmeAux.SupRateLen = ie_list->SupRateLen;
			NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, ie_list->SupRate, ie_list->SupRateLen);
			RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->wdev.PhyMode);
			pApCliEntry->MlmeAux.ExtRateLen = ie_list->ExtRateLen;
			NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
			RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &pApCliEntry->MlmeAux.ExtRateLen, pApCliEntry->wdev.PhyMode);
#ifdef APCLI_CERT_SUPPORT

			/*  Get the ext capability info element */
			if (pAd->bApCliCertTest == TRUE) {
				NdisMoveMemory(&pApCliEntry->MlmeAux.ExtCapInfo, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
#ifdef DOT11_N_SUPPORT
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("\x1b[31m ApCliMlmeAux.ExtCapInfo=%d \x1b[m\n",
						 pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport));
#ifdef DOT11N_DRAFT3

				if ((pAd->CommonCfg.bBssCoexEnable == TRUE) && (ie_list->Channel <= 14))
					pAd->CommonCfg.ExtCapIE.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			}

#endif /* APCLI_CERT_SUPPORT */
#ifdef DOT11_N_SUPPORT
			NdisZeroMemory(pApCliEntry->RxMcsSet, sizeof(pApCliEntry->RxMcsSet));

			/* filter out un-supported ht rates */
			if ((ie_list->HtCapabilityLen > 0) &&
				(pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable) &&
				WMODE_CAP_N(pApCliEntry->wdev.PhyMode)) {
				RTMPZeroMemory(&pApCliEntry->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				pApCliEntry->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
				pApCliEntry->MlmeAux.HtCapabilityLen = ie_list->HtCapabilityLen;
				ApCliCheckHt(pAd, ifIndex, &ie_list->HtCapability, &ie_list->AddHtInfo);

				if (ie_list->AddHtInfoLen > 0) {
					CentralChannel = ie_list->AddHtInfo.ControlChan;
					/* Check again the Bandwidth capability of this AP. */
					CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
														   &ie_list->HtCapability);
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							 ("PeerBeaconAtJoinAction HT===>CentralCh = %d, ControlCh = %d\n",
							  CentralChannel, ie_list->AddHtInfo.ControlChan));
				}
			} else
#endif /* DOT11_N_SUPPORT */
			{
				RTMPZeroMemory(&pApCliEntry->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				RTMPZeroMemory(&pApCliEntry->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
				pApCliEntry->MlmeAux.HtCapabilityLen = 0;
			}

			ApCliUpdateMlmeRate(pAd, ifIndex);
#ifdef DOT11_N_SUPPORT

			/* copy QOS related information */
			if (WMODE_CAP_N(pApCliEntry->wdev.PhyMode)) {
				NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pApCliEntry->MlmeAux.APQbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pApCliEntry->MlmeAux.APQosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
			} else
#endif /* DOT11_N_SUPPORT */
			{
				NdisZeroMemory(&pApCliEntry->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pApCliEntry->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pApCliEntry->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n",
					 pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->MlmeAux.ExtRateLen));

			if (ie_list->AironetCellPowerLimit != 0xFF) {
				/* We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
				ChangeToCellPowerLimit(pAd, ie_list->AironetCellPowerLimit);
			} else {
				/* Used the default TX Power Percentage. */
				pAd->CommonCfg.ucTxPowerPercentage[BAND0] = pAd->CommonCfg.ucTxPowerDefault[BAND0];
#ifdef DBDC_MODE
				pAd->CommonCfg.ucTxPowerPercentage[BAND1] = pAd->CommonCfg.ucTxPowerDefault[BAND1];
#endif /* DBDC_MODE */
			}

#ifdef WSC_AP_SUPPORT
#ifdef DOT11_N_SUPPORT

			if ((pApCliEntry->wdev.WscControl.WscConfMode != WSC_DISABLE) &&
				(pApCliEntry->wdev.WscControl.bWscTrigger == TRUE)) {
				ADD_HTINFO RootApHtInfo;
				struct wifi_dev *wdev = &pApCliEntry->wdev;
				UCHAR ext_cha = wlan_config_get_ext_cha(wdev);
				UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

				RootApHtInfo = ie_list->AddHtInfo.AddHtInfo;

				if ((cfg_ht_bw == HT_BW_40) &&
					(RootApHtInfo.RecomWidth) &&
					(RootApHtInfo.ExtChanOffset != ext_cha)) {
					if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
						set_extcha_for_wdev(pAd, wdev, 1);
					else
						set_extcha_for_wdev(pAd, wdev, 0);
					*pCurrState = APCLI_SYNC_IDLE;
					pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState = APCLI_CTRL_DISCONNECTED;
					goto LabelErr;
				}
			}

#endif /* DOT11_N_SUPPORT */
#ifdef APCLI_CERT_SUPPORT

			if (pAd->bApCliCertTest == TRUE)
				pApCliEntry->wdev.channel = ie_list->Channel;

#endif /* APCLI_CERT_SUPPORT */
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_OWE_SUPPORT
			if ((bssidEqualFlag == TRUE)
				&& (pAd->ScanTab.BssEntry[Bssidx].owe_trans_ie_len > 0)
					&& (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap)
						&& (IS_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap)
							&& IS_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher)))) {

				BSS_ENTRY *popen_bss_entry = NULL;
				BSS_TABLE *pscan_tab = NULL;
				struct wifi_dev *papcli_wdev = NULL;


			/*If bssidEqualFlag is set then the probe timer has been cancelled ,*
				so we need to perform OWE trans IE parsing here in the case of Configured BSSID*/

				*pCurrState = APCLI_SYNC_IDLE;

				pscan_tab = &pAd->ScanTab;
				papcli_wdev = (struct wifi_dev *)&pApCliEntry->wdev;

				/*If Trans IE found then extract BSSID ,SSID ,Band and Channel*/
				popen_bss_entry = &pscan_tab->BssEntry[Bssidx];

				if (popen_bss_entry && (popen_bss_entry->owe_trans_ie_len > 0)) {


					UCHAR pair_ch = 0;
					UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
					UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
					UCHAR pair_band = 0;
					UCHAR pair_ssid_len = 0;



					extract_pair_owe_bss_info(popen_bss_entry->owe_trans_ie,
								 popen_bss_entry->owe_trans_ie_len,
								  pair_bssid,
								  pair_ssid,
								  &pair_ssid_len,
								  &pair_band,
								  &pair_ch);



					if (pair_ch != 0) {
						/*OWE Entry found ,update OweTransBssid and OweTranSsid*/


						/*OWE bss is on different channel*/

						if (BOARD_IS_5G_ONLY(pAd)) {

							UCHAR BandIdx = HcGetBandByWdev(papcli_wdev);
							CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

							/*Single Chip Dual band 5g only case*/

							if (MTChGrpChannelChk(pChCtrl, pair_ch)) {
								/*Channel is valid in the current channel group*/
								if (pair_ch != popen_bss_entry->Channel) {

							/*OWE send EVENT to host for OWE  indicating different channel*/
								wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																		OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
																		pair_bssid,
																		pair_ssid,
																		&pair_ssid_len,
																		&pair_band,
																		&pair_ch);

								MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
										("%s:%d Different channel same band\n", __func__, __LINE__));

								NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
								NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
								pApCliEntry->owe_trans_ssid_len = pair_ssid_len;


								CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
								CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
								CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
							/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
								BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

								MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
										 pair_ch));
								rtmp_set_channel(pAd, &pApCliEntry->wdev, pair_ch);
								/* if bw adjust,timeout this time */
								MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
								return;
								} else {
									/*Same Channel send directed probe request to OWE BSS*/
									/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

										NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
										pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

										NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
										pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;

										CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

										/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
										BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

										MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
										return;

								}
							} else {
								/*Channel not in group of current band , but entry exists so send event to host to trigger connection on other band*/
								wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																		OID_802_11_OWE_EVT_DIFF_BAND,
																		pair_bssid,
																		pair_ssid,
																		&pair_ssid_len,
																		&pair_band,
																		&pair_ch);
							}

						} else {

							/*Check if the OWE bss is on the same band as the CLI,then check if channel change required*/
							if ((WMODE_2G_ONLY(pApCliEntry->wdev.PhyMode) && (pair_ch <= 14))
								|| (WMODE_5G_ONLY(pApCliEntry->wdev.PhyMode) && (pair_ch > 14))) {

								if (pair_ch != popen_bss_entry->Channel) {
								/*OWE send EVENT to host for OWE  indicating different channel*/
									wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																			OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
																			pair_bssid,
																			pair_ssid,
																			&pair_ssid_len,
																			&pair_band,
																			&pair_ch);

								MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:%d Different channel same band\n", __func__, __LINE__));

									NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
									pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

									NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
									pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;

									CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

									/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
									BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

									MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
											 pair_ch));
									rtmp_set_channel(pAd, &pApCliEntry->wdev, pair_ch);

									MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
									return;
								} else {
									/*Same Channel send directed probe request to OWE BSS*/
									/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

										NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
										pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

										NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
										pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;


										CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

										/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
										BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

										MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
										return;

								}
							} else {
								/*Channel not in group of current band , but entry exists so send event to host to trigger connection on other band*/
								wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																		OID_802_11_OWE_EVT_DIFF_BAND,
																		pair_bssid,
																		pair_ssid,
																		&pair_ssid_len,
																		&pair_band,
																		&pair_ch);

							}

						}
					} else {
				/*Same Channel send directed probe request to OWE BSS*/
				/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

						NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
						NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
						pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

						NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
						NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
						pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;


						CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
						CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
						CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

						/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
						BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);


						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
						return;

					}


				}

			} else

#endif
			if (bssidEqualFlag == TRUE) {
				*pCurrState = APCLI_SYNC_IDLE;
				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
				/* follow root ap setting while ApCliAutoConnectRunning is active */
				if (((pAd->ApCfg.ApCliAutoConnectRunning[ifIndex] == TRUE) &&
							(pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIndex] < 3))
#ifdef BT_APCLI_SUPPORT
						|| (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE)
#endif
				)
#endif /*APCLI_CFG80211_SUPPORT*/
				{

					ULONG Bssidx = 0;

					Bssidx = BssTableSearch(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->wdev.channel);

					if (Bssidx != BSS_NOT_FOUND) {
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s[%d]Bssidx:%lu\n", __func__, __LINE__, Bssidx));

						if (pAd->ScanTab.BssEntry[Bssidx].SsidLen)
							MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Root AP SSID: %s\n", pAd->ScanTab.BssEntry[Bssidx].Ssid));

						if (ApCliAutoConnectBWAdjust(pAd, &pApCliEntry->wdev, &pAd->ScanTab.BssEntry[Bssidx])) {
							pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIndex]++;
							MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
									 pAd->ScanTab.BssEntry[Bssidx].Channel));
							rtmp_set_channel(pAd, &pApCliEntry->wdev, pAd->ScanTab.BssEntry[Bssidx].Channel);
							goto LabelErr;
						}

#endif /* APCLI_AUTO_BW_TMP */
					} else {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s[%d]Can not find BssEntry\n", __func__, __LINE__));
						goto LabelErr;
					}
				}

#endif
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
#ifdef BT_APCLI_SUPPORT
			else {
				if (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE) {
					ULONG Bssidx = 0;

					Bssidx = BssTableSearch(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid,
							pApCliEntry->wdev.channel);
					if (ApCliAutoConnectBWAdjust(pAd, &pApCliEntry->wdev,
								&pAd->ScanTab.BssEntry[Bssidx])) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
								("Switch to channel :%d\n",
								 pAd->ScanTab.BssEntry[Bssidx].Channel));
						rtmp_set_channel(pAd, &pApCliEntry->wdev,
								pAd->ScanTab.BssEntry[Bssidx].Channel);
						goto LabelErr;
					}
				}
			}
#endif
		}

	}

LabelErr:

	if (VarIE != NULL)
		os_free_mem(VarIE);

	if (ie_list != NULL)
		os_free_mem(ie_list);
}

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCtrl_CurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#endif
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef APCLI_OWE_SUPPORT
	UCHAR		entryIdx = 0;
	BSS_TABLE		*pscan_tab = NULL, *pssid_bss_tab = NULL;
	struct wifi_dev *papcli_wdev = NULL;
#endif
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI_SYNC - ProbeTimeoutAtJoinAction\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef APCLI_OWE_SUPPORT
	papcli_wdev = (struct wifi_dev *)&pApCliEntry->wdev;
	pscan_tab = &pAd->ScanTab;
	pssid_bss_tab = &pApCliEntry->MlmeAux.owe_bss_tab;
	pssid_bss_tab->BssNr = 0;

	/*
		Find out APs with OWE transition IE and store them in the owe_bss_tab*
	*/
	if (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap)) {



		for (entryIdx = 0; entryIdx < pscan_tab->BssNr; entryIdx++) {
			BSS_ENTRY *pBssEntry = &pscan_tab->BssEntry[entryIdx];

			if (pBssEntry->Channel == 0)
				continue;

			if ((pBssEntry->owe_trans_ie_len > 0) &&
				(pssid_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("%s:OWE Table %d:Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n", __func__, __LINE__,
						 PRINT_MAC(pBssEntry->Bssid)));
				NdisMoveMemory(&pssid_bss_tab->BssEntry[pssid_bss_tab->BssNr++],
											   pBssEntry, sizeof(BSS_ENTRY));

			}
		}
	}
	if (pssid_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)
		NdisZeroMemory(&pssid_bss_tab->BssEntry[pssid_bss_tab->BssNr], sizeof(BSS_ENTRY));

#endif/* APCLI_OWE_SUPPORT */

	*pCurrState = SYNC_IDLE;
#ifdef APCLI_CONNECTION_TRIAL

	if (ifIndex == (pAd->ApCfg.ApCliNum - 1)) /* last interface is for connection trial */
		*pCtrl_CurrState = APCLI_CTRL_DISCONNECTED;

#endif /* APCLI_CONNECTION_TRIAL */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI_SYNC - MlmeAux.Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 PRINT_MAC(pApCliEntry->MlmeAux.Bssid)));
#ifdef APCLI_CONNECTION_TRIAL

	if (!MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR) &&
		(*pCtrl_CurrState != APCLI_CTRL_DISCONNECTED))
#else
	if (!MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR))
#endif
	{
		ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */



#ifdef APCLI_OWE_SUPPORT
		/*Find Open BSS with Trans IE*/
		if (IS_AKM_OWE(papcli_wdev->SecConfig.AKMMap)
				&&  (IS_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap)
					&& IS_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher))) {

			ULONG bssidx = 0;
			BSS_ENTRY *popen_bss_entry = NULL;

			/*Find BSS entry for Open bss from owe_bss_tab*/
			bssidx = BssTableSearch(pssid_bss_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

			if (bssidx != BSS_NOT_FOUND) {
			/*If Trans IE found then extract BSSID ,SSID ,Band and Channel*/
				popen_bss_entry = &pssid_bss_tab->BssEntry[bssidx];
				if (popen_bss_entry && (popen_bss_entry->owe_trans_ie_len > 0)) {


						UCHAR pair_ch = 0;
						UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
						UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
						UCHAR pair_band = 0;
						UCHAR pair_ssid_len = 0;



						extract_pair_owe_bss_info(popen_bss_entry->owe_trans_ie,
									 popen_bss_entry->owe_trans_ie_len,
									  pair_bssid,
									  pair_ssid,
									  &pair_ssid_len,
									  &pair_band,
									  &pair_ch);



						if (pair_ch != 0) {
							/*OWE Entry found ,update OweTransBssid and OweTranSsid*/


							/*OWE bss is on different channel*/

							if (BOARD_IS_5G_ONLY(pAd)) {

								UCHAR BandIdx = HcGetBandByWdev(papcli_wdev);
								CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

								/*Single Chip Dual band 5g only case*/

								if (MTChGrpChannelChk(pChCtrl, pair_ch)) {
									/*Channel is valid in the current channel group*/
								if (pair_ch != popen_bss_entry->Channel) {

								/*OWE send EVENT to host for OWE  indicating different channel*/
									wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																			OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
																			pair_bssid,
																			pair_ssid,
																			&pair_ssid_len,
																			&pair_band,
																			&pair_ch);

									MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
											("%s:%d Different channel same band\n", __func__, __LINE__));

									NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
									pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

									NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
									pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;


									CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
								/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
									BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

									MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
											 pair_ch));
									rtmp_set_channel(pAd, &pApCliEntry->wdev, pair_ch);
									/* if bw adjust,timeout this time */
									MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
									return;
								} else {
									/*Same Channel send directed probe request to OWE BSS*/
									/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

										NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
										pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

										NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
										pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;

										CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

										/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
										BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);


										MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
										return;

									}
								} else {
									/*Channel not in group of current band , but entry exists so send event to host to trigger connection on other band*/
									wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																			OID_802_11_OWE_EVT_DIFF_BAND,
																			pair_bssid,
																			pair_ssid,
																			&pair_ssid_len,
																			&pair_band,
																			&pair_ch);
								}

							} else {

								/*Check if the OWE bss is on the same band as the CLI,then check if channel change required*/
								if ((WMODE_2G_ONLY(pApCliEntry->wdev.PhyMode) && (pair_ch <= 14))
									|| (WMODE_5G_ONLY(pApCliEntry->wdev.PhyMode) && (pair_ch > 14))) {

								if (pair_ch != popen_bss_entry->Channel) {
									/*OWE send EVENT to host for OWE  indicating different channel*/
										wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																				OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
																				pair_bssid,
																				pair_ssid,
																				&pair_ssid_len,
																				&pair_band,
																				&pair_ch);

									MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:%d Different channel same band\n", __func__, __LINE__));

										NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
										pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

										NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
										NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
										pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;


										CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
										CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

										/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
										BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);

										MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
												 pair_ch));
										rtmp_set_channel(pAd, &pApCliEntry->wdev, pair_ch);

										MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
										return;
								} else {
									/*Same Channel send directed probe request to OWE BSS*/
									/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

									NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
									pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

									NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
									NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
									pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;


									CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
									CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

									/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
									BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);
									MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
									return;

									}
								} else {
									/*Channel not in group of current band , but entry exists so send event to host to trigger connection on other band*/
									wext_send_owe_trans_chan_event(papcli_wdev->if_dev,
																			OID_802_11_OWE_EVT_DIFF_BAND,
																			pair_bssid,
																			pair_ssid,
																			&pair_ssid_len,
																			&pair_band,
																			&pair_ch);

								}

							}
						} else {
					/*Same Channel send directed probe request to OWE BSS*/
					/*Update the Owe transtion Bssid and Ssid that will used for a directed probe request to OWE AP*/

						NdisMoveMemory(&pApCliEntry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
						NdisMoveMemory(&pApCliEntry->owe_trans_ssid, pair_ssid, pair_ssid_len);
						pApCliEntry->owe_trans_ssid_len = pair_ssid_len;

						NdisMoveMemory(&pApCliEntry->owe_trans_open_bssid, popen_bss_entry->Bssid, MAC_ADDR_LEN);
						NdisMoveMemory(&pApCliEntry->owe_trans_open_ssid, popen_bss_entry->Ssid, popen_bss_entry->SsidLen);
						pApCliEntry->owe_trans_open_ssid_len = popen_bss_entry->SsidLen;

						CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
						CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
						CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

						/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
						BssTableDeleteEntry(pscan_tab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.Channel);


						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
						return;

						}


				}


			}
		}
#endif
#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifndef APCLI_CFG80211_SUPPORT
		/* follow root ap setting while ApCliAutoConnectRunning is active */
		if (((pAd->ApCfg.ApCliAutoConnectRunning[ifIndex] == TRUE) &&
					(pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIndex] < 3))
#ifdef BT_APCLI_SUPPORT
				|| (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE)
#endif
		)
#endif /* APCLI_CFG80211_SUPPORT */
		    {
			ULONG Bssidx = 0;

			Bssidx = BssTableSearch(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->wdev.channel);

			if (Bssidx != BSS_NOT_FOUND) {
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s[%d]Bssidx:%lu\n", __func__, __LINE__, Bssidx));

				if (pAd->ScanTab.BssEntry[Bssidx].SsidLen)
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Root AP SSID: %s\n", pAd->ScanTab.BssEntry[Bssidx].Ssid));

				if (ApCliAutoConnectBWAdjust(pAd, &pApCliEntry->wdev, &pAd->ScanTab.BssEntry[Bssidx])) {
					pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIndex]++;
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n",
							 pAd->ScanTab.BssEntry[Bssidx].Channel));
					rtmp_set_channel(pAd, &pApCliEntry->wdev, pAd->ScanTab.BssEntry[Bssidx].Channel);
					/* if bw adjust,timeout this time */
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
					return;
				}

#endif /* APCLI_AUTO_BW_TMP */
			} else {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s[%d]Can not find BssEntry\n", __func__, __LINE__));
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
				return;
			}
		}

#endif
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	} else
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
}

/*
    ==========================================================================
    Description:
    ==========================================================================
 */
static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	*pCurrState = APCLI_SYNC_IDLE;
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("APCLI_AYNC - ApCliInvalidStateWhenJoin(state=%ld). Reset SYNC machine\n", *pCurrState));
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	OUT PCHAR Ssid,
	IN USHORT ifIndex)
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;
	UCHAR           SsidIe    = IE_SSID;
	UCHAR           SupRateIe = IE_SUPP_RATES;
	UCHAR ssidLen;
	CHAR ssid[MAX_LEN_OF_SSID];
	APCLI_STRUCT *pApCliEntry = NULL;
	struct wifi_dev *wdev = NULL;
	struct _build_ie_info ie_info;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("force out a ProbeRequest ...\n"));
	if (ifIndex >= MAX_APCLI_NUM)
			return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	wdev = &pApCliEntry->wdev;
	ie_info.frame_subtype = SUBTYPE_PROBE_REQ;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = wdev->PhyMode;
	ie_info.wdev = wdev;

	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */

	if (NState != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("EnqueueProbeRequest() allocate memory fail\n"));
		return;
	}

#ifdef APCLI_OWE_SUPPORT
	if (IS_AKM_OWE(pApCliEntry->wdev.SecConfig.AKMMap) && (pApCliEntry->owe_trans_ssid_len > 0))
		ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
							pAd->ApCfg.ApCliTab[ifIndex].owe_trans_bssid, pAd->ApCfg.ApCliTab[ifIndex].owe_trans_bssid, ifIndex);
	else
#endif
	if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, ZERO_MAC_ADDR))
		ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
							  BROADCAST_ADDR, BROADCAST_ADDR, ifIndex);
	else
		ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
							  pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, ifIndex);

	ssidLen = SsidLen;
	NdisZeroMemory(ssid, MAX_LEN_OF_SSID);
	NdisMoveMemory(ssid, Ssid, ssidLen);
	/* this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse */
	MakeOutgoingFrame(pOutBuffer,		&FrameLen,
					  sizeof(HEADER_802_11),			&Hdr80211,
					  1,								&SsidIe,
					  1,								&ssidLen,
					  ssidLen,						ssid,
					  1,								&SupRateIe,
					  1,								&pApCliEntry->MlmeAux.SupRateLen,
					  pApCliEntry->MlmeAux.SupRateLen,		pApCliEntry->MlmeAux.SupRate,
					  END_OF_ARGS);

	/* Add the extended rate IE */
	if (pApCliEntry->MlmeAux.ExtRateLen != 0) {
		ULONG            tmp;

		MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
						  1,                        &ExtRateIe,
						  1,                        &pApCliEntry->MlmeAux.ExtRateLen,
						  pApCliEntry->MlmeAux.ExtRateLen,  pApCliEntry->MlmeAux.ExtRate,
						  END_OF_ARGS);
		FrameLen += tmp;
	}

	if (WMODE_CAP_N(wdev->PhyMode)) {
		ie_info.is_draft_n_type = FALSE;
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);

#ifdef DOT11_VHT_AC
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		ucETxBfCap = wlan_config_get_etxbf(wdev);

		if (HcIsBfCapSupport(&pApCliEntry->wdev) == FALSE)
			wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		FrameLen += build_vht_ies(pAd, &ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
	}
#ifdef WSC_INCLUDED
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_INCLUDED */
	ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
	FrameLen +=  build_extra_ie(pAd, &ie_info);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}

#endif /* APCLI_SUPPORT */

