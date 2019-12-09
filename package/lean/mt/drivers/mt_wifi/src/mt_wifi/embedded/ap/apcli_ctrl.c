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
	apcli_ctrl.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/
#ifdef APCLI_SUPPORT

#include "rt_config.h"


static VOID ApCliCtrlJoinReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlJoinReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlProbeRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAuthRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAuth2RspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAuthReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAuth2ReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlDeAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlAssocReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlDisconnectReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlPeerDeAssocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliCtrlDeAssocAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static
 VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliWpaMicFailureReportFrame(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

#ifdef APCLI_CERT_SUPPORT
static VOID ApCliCtrlScanDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);
#endif /* APCLI_CERT_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialConnectTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialPhase2TimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliTrialConnectRetryTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);
static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(ApCliTrialConnectTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectTimeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout);
DECLARE_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);
BUILD_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout);


#endif /* APCLI_CONNECTION_TRIAL */
/*
    ==========================================================================
    Description:
	The apcli ctrl state machine,
    Parameters:
	Sm - pointer to the state machine
    Note:
	the state machine looks like the following
    ==========================================================================
 */
VOID ApCliCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
#ifdef APCLI_CONNECTION_TRIAL
	PAPCLI_STRUCT	pApCliEntry;
#endif /* APCLI_CONNECTION_TRIAL */
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans,
					 APCLI_MAX_CTRL_STATE, APCLI_MAX_CTRL_MSG,
					 (STATE_MACHINE_FUNC)Drop, APCLI_CTRL_DISCONNECTED,
					 APCLI_CTRL_MACHINE_BASE);
	/* disconnected state */
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_JOIN_REQ, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqAction);
	/* probe state */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_JOIN_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);

	/* auth state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuthRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuthReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* auth2 state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuth2RspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuth2ReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
	/* assoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAssocRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAssocReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* deassoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_DEASSOC, APCLI_CTRL_DEASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocRspAction);
	/* connected state */
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAuthAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_MT2_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_MIC_FAILURE_REPORT_FRAME, (STATE_MACHINE_FUNC)ApCliWpaMicFailureReportFrame);
#ifdef APCLI_CERT_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_SCAN_DONE, (STATE_MACHINE_FUNC)ApCliCtrlScanDoneAction);
#endif /* APCLI_CERT_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_TRIAL_CONNECT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_TRIAL_CONNECT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);
	StateMachineSetAction(Sm, APCLI_CTRL_TRIAL_TRIGGERED, APCLI_CTRL_JOIN_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlTrialConnectAction);/* for retry */
	/* Trial Connect Timer Timeout handling */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectTimeoutAction);
	/* Trial Phase2 Timer Timeout handling */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialPhase2TimeoutAction);
	/* Trial Retry Timer Timeout handling */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, (STATE_MACHINE_FUNC)ApCliTrialConnectRetryTimeoutAction);
#endif /* APCLI_CONNECTION_TRIAL */

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.ApCliTab[i].CtrlCurrState = APCLI_CTRL_DISCONNECTED;
		pAd->ApCfg.ApCliTab[i].LinkDownReason = APCLI_LINKDOWN_NONE;
		pAd->ApCfg.ApCliTab[i].Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
#ifdef APCLI_CONNECTION_TRIAL
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
		/* timer init */
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectTimer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectTimeout),
					  (PVOID)pApCliEntry,
					  FALSE);
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectPhase2Timer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectPhase2Timeout),
					  (PVOID)pApCliEntry,
					  FALSE);
		RTMPInitTimer(pAd,
					  &pApCliEntry->TrialConnectRetryTimer,
					  GET_TIMER_FUNCTION(ApCliTrialConnectRetryTimeout),
					  pApCliEntry,
					  FALSE);
#endif /* APCLI_CONNECTION_TRIAL */
	}
}

#ifdef APCLI_CONNECTION_TRIAL
static VOID ApCliTrialConnectTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;
	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectPhase2Timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;
	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_PHASE2_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectRetryTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;
	UCHAR ifIndex = pApCliEntry->ifIndex;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_RETRY_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectionBeaconControl(PRTMP_ADAPTER pAd, BOOLEAN start)
{
	struct wifi_dev *wdev;
	INT IdBss;
	INT MaxNumBss;

	MaxNumBss = pAd->ApCfg.BssidNum;

	for (IdBss = 0; IdBss < MaxNumBss; IdBss++) {
		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;

		if (WDEV_BSS_STATE(wdev) == BSS_READY) {
			UpdateBeaconHandler(
				pAd,
				wdev,
				(start) ? BCN_UPDATE_ENABLE_TX : BCN_UPDATE_DISABLE_TX);
		}
	}
}

static VOID ApCliTrialConnectTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT	pApCliEntry;
	UCHAR	ifIndex;
	PULONG	pCurrState;
	UCHAR ch;
	struct freq_oper oper;

	ifIndex = pElem->Priv;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pCurrState = &pApCliEntry->CtrlCurrState;
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(&pApCliEntry->wdev, &oper);
	ch = oper.prim_ch;
	if (pApCliEntry->TrialCh != ch) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s[%d]Change ch %d to %d(%d)\n\r",
				__func__, __LINE__,
				pApCliEntry->TrialCh, pApCliEntry->wdev.channel, ch));
		pApCliEntry->wdev.channel = ch;
		wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
		/* TBD regenerate beacon? */
		ApCliTrialConnectionBeaconControl(pAd, TRUE);
	}

	if (*pCurrState == APCLI_CTRL_ASSOC) {
		/* trialConnectTimeout, and currect status is ASSOC, */
		/* it means we got Auth Resp from new root AP already, */
		/* we shall serve the origin channel traffic first, */
		/* and jump back to trial channel to issue Assoc Req later, */
		/* and finish four way-handshake if need. */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ApCliTrialConnectTimeout APCLI_CTRL_ASSOC set TrialConnectPhase2Timer\n", __func__));
		RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer), TRIAL_TIMEOUT);
	} else {
		/* RTMPCancelTimer(&(pApCliEntry->ApCliMlmeAux.ProbeTimer), &Cancelled); */
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= 10) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->Enable = FALSE;
		} else {
			/* trial connection probe fail                                               */
			/* change apcli sync state machine to idle state to reset sync state machine */
			PULONG pSync_CurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
			*pSync_CurrState = APCLI_SYNC_IDLE;
			*pCurrState = APCLI_CTRL_DISCONNECTED;/* Disconnected State will bring the next probe req, auth req. */
		}
	}
}

static VOID ApCliTrialPhase2TimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	UCHAR ifIndex;
	struct wifi_dev *wdev;
	struct freq_oper oper;

	ifIndex = pElem->Priv;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(wdev, &oper);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s,\nJump back to trial channel:%d\nto issue Assoc Req to new root AP\n",
			 __func__, pApCliEntry->TrialCh));

	if (pApCliEntry->TrialCh != oper.prim_ch) {
		/* TBD disable beacon? */
		ApCliTrialConnectionBeaconControl(pAd, FALSE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s[%d]Change ch %d to %d\n\r",
				  __func__, __LINE__,
				  wdev->channel, pApCliEntry->TrialCh));
		/* switch channel to trial channel */
		wlan_operate_scan(wdev, pApCliEntry->TrialCh);
	}

	ApCliLinkDown(pAd, ifIndex);

	/* if (wdev->AuthMode >= Ndis802_11AuthModeWPA) */
	if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)) {
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer), (800 + pApCliEntry->NewRootApRetryCnt*200));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectRetryTimer(%d ms)\n", __func__, (800 + pApCliEntry->NewRootApRetryCnt*200)));
	} else {
		RTMPSetTimer(&(pApCliEntry->TrialConnectRetryTimer), TRIAL_TIMEOUT);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectRetryTimer(%d ms)\n", __func__, TRIAL_TIMEOUT));
	}

	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[pApCliEntry->ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[pApCliEntry->ifIndex].MlmeAux.CapabilityInfo,
				  ASSOC_TIMEOUT, 5);
	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
				sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, pApCliEntry->ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliTrialConnectRetryTimeoutAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *pElem)
{
	PAPCLI_STRUCT pApCliEntry;
	PULONG pCurrState;
	UCHAR ifIndex;
	UCHAR ch;
	PMAC_TABLE_ENTRY pMacEntry;
	STA_TR_ENTRY *tr_entry;
	struct freq_oper oper;
	/* PMAC_TABLE_ENTRY pOldRootAp = &pApCliEntry->oldRootAP; */
	ifIndex = pElem->Priv;
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pMacEntry = MacTableLookup(pAd, pApCliEntry->MlmeAux.Bssid);
	/*query physical radio setting by wdev*/
	hc_radio_query_by_wdev(&pApCliEntry->wdev, &oper);

	if (pMacEntry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ApCli_SYNC - %s, no CfgApCliBssid in mactable!\n", __func__));
		/* *pCurrState = APCLI_CTRL_DISCONNECTED; */
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt >= 10) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			ApCliLinkDown(pAd, ifIndex);
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->Enable = FALSE;
		}

		if (pApCliEntry->TrialCh != oper.prim_ch) {
			ch = oper.prim_ch;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, pApCliEntry->wdev.channel, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, TRUE);
		}

		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return;
	}

	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) && (*pCurrState == APCLI_CTRL_CONNECTED)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, new rootAP connected!!\n", __func__));
		/* connected to new ap ok, change common channel to new channel */
		/* AsicSetApCliBssid(pAd, pApCliEntry->ApCliMlmeAux.Bssid, 1); */
		/* MacTableDeleteEntry(pAd, pApCliEntry->MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID)); */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCli_SYNC - %s, jump back to origin channel to wait for User's operation!\n", __func__));

		if (pApCliEntry->TrialCh != oper.prim_ch) {
			ch = oper.prim_ch;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, pApCliEntry->wdev.channel, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, TRUE);
		}

		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
		pApCliEntry->CfgSsidLen = 0;
		pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
		/* pApCliEntry->Enable = FALSE; */
		/* sprintf(tempBuf, "%d", pApCliEntry->TrialCh); */
		/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Follow new rootAP Switch to channel :%s\n", tempBuf)); */
		/* Set_Channel_Proc(pAd, tempBuf);//APStartUp will regenerate beacon. */
		pApCliEntry->TrialCh = 0;
	} else {
		/*
		   Apcli does not connect to new root ap successfully yet,
		   jump back to origin channel to serve old rootap traffic.
		   re-issue assoc_req to go later.
		*/
		/* pApCliEntry->MacTabWCID = pOldRootAp->Aid; */
		pApCliEntry->NewRootApRetryCnt++;

		if (pApCliEntry->NewRootApRetryCnt < 10) {
			if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) && (*pCurrState == APCLI_CTRL_CONNECTED))
				; /* *pCurrState = APCLI_CTRL_DISCONNECTED; */
			else
				RTMPSetTimer(&(pApCliEntry->TrialConnectPhase2Timer), TRIAL_TIMEOUT);
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RetryCnt:%d, pCurrState = %lu,\n", __func__, pApCliEntry->NewRootApRetryCnt, *pCurrState));
			pApCliEntry->TrialCh = 0;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid, MAX_LEN_OF_SSID);/* cleanup CfgSsid. */
			pApCliEntry->CfgSsidLen = 0;
			pApCliEntry->NewRootApRetryCnt = 0;/* cleanup retry count */
			pApCliEntry->Enable = FALSE;
			ApCliLinkDown(pAd, ifIndex);
		}

		if (pApCliEntry->TrialCh != oper.prim_ch) {
			ch = oper.prim_ch;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s[%d]Change ch %d to %d(%d)\n\r",
					__func__, __LINE__,
					pApCliEntry->TrialCh, pApCliEntry->wdev.channel, ch));
			/* switch channel to orignal channel */
			pApCliEntry->wdev.channel = ch;
			wlan_operate_set_prim_ch(&pApCliEntry->wdev, ch);
			/* TBD enable beacon? */
			ApCliTrialConnectionBeaconControl(pAd, TRUE);
		}

		if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) && (*pCurrState == APCLI_CTRL_CONNECTED))
			*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
}
#endif /* APCLI_CONNECTION_TRIAL */

/*
    ==========================================================================
    Description:
	APCLI MLME JOIN req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlJoinReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#ifdef WSC_AP_SUPPORT
	PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl;
#endif /* WSC_AP_SUPPORT */
	struct wifi_dev *wdev = NULL;
	PULONG pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;

	wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Start Probe Req.\n", __func__));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd, wdev) == TRUE)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));


	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

#ifdef WSC_AP_SUPPORT

	if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
		(pWpsCtrl->bWscTrigger == TRUE)) {
		ULONG bss_idx = 0;

		NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
		JoinReq.SsidLen = pWpsCtrl->WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pWpsCtrl->WscSsid.Ssid, JoinReq.SsidLen);
		if (pWpsCtrl->WscMode == 1) { /* PIN */
			INT old_conf_mode = pWpsCtrl->WscConfMode;
			UCHAR channel = wdev->channel, RootApChannel;
#ifdef AP_SCAN_SUPPORT
			if ((pWpsCtrl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) &&
				pAd->ScanCtrl.PartialScan.bScanning &&
				pAd->ScanCtrl.PartialScan.LastScanChannel != 0)
				return;
#endif /* AP_SCAN_SUPPORT */

			bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PUCHAR)(JoinReq.Ssid), JoinReq.SsidLen);

			if (bss_idx == BSS_NOT_FOUND) {
#ifdef AP_SCAN_SUPPORT
				if (pWpsCtrl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
					if (!pAd->ScanCtrl.PartialScan.bScanning &&
						(pAd->ScanCtrl.PartialScan.LastScanChannel == 0)) {
						pAd->ScanCtrl.PartialScan.pwdev = wdev;
						pAd->ScanCtrl.PartialScan.bScanning = TRUE;
					}
				}
#endif /* AP_SCAN_SUPPORT */

				ApSiteSurvey_by_wdev(pAd, NULL, SCAN_WSC_ACTIVE, FALSE, wdev);
				return;
			}
			RootApChannel = pAd->ScanTab.BssEntry[bss_idx].Channel;
			if (RootApChannel != channel) {
				rtmp_set_channel(pAd, wdev, RootApChannel);
				wdev->channel = RootApChannel;
				/*
				   ApStop will call WscStop, we need to reset WscConfMode, WscMode & bWscTrigger here.
				   */
				pWpsCtrl->WscState = WSC_STATE_START;
				pWpsCtrl->WscStatus = STATUS_WSC_START_ASSOC;
				pWpsCtrl->WscMode = 1;
				pWpsCtrl->WscConfMode = old_conf_mode;
				pWpsCtrl->bWscTrigger = TRUE;
				return;
			}
		}
	} else
#endif /* WSC_AP_SUPPORT */
	{
			if (pApCliEntry->CfgSsidLen != 0) {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
				ULONG bss_idx = BSS_NOT_FOUND;

				bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PCHAR)pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);

				if (bss_idx == BSS_NOT_FOUND) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s::  can't find SSID[%s] in ScanTab.\n", __func__, pApCliEntry->CfgSsid));
					*pCurrState = APCLI_CTRL_PROBE;
					CFG80211_checkScanTable(pAd);
					RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, JoinReq.Bssid, NULL, 0, NULL, 0, 0);
					return;
				}

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s::  find SSID[%ld][%s] channel[%d-%d] in ScanTab.\n", __func__,
						 bss_idx, pApCliEntry->CfgSsid, pAd->ScanTab.BssEntry[bss_idx].Channel,
						 pAd->ScanTab.BssEntry[bss_idx].CentralChannel));
				/* TODO */
				/* BssSearch Table has found the pEntry, send Prob Req. directly */
				/* if (wdev->channel != pAd->ScanTab.BssEntry[bss_idx].Channel) */
				{
					pApCliEntry->MlmeAux.Channel = pAd->ScanTab.BssEntry[bss_idx].Channel;
#ifdef CONFIG_MULTI_CHANNEL
					/* should be check and update in in asso to check ==> ApCliCheckHt() */
					pApCliEntry->wdev.channel = pApCliEntry->MlmeAux.Channel;
					wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
#endif /* CONFIG_MULTI_CHANNEL */
					wlan_operate_set_prim_ch(&pApCliEntry->wdev, pApCliEntry->wdev.channel);
				}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */


			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
				NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);

		}
	}
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, JoinReq.Ssid, PRINT_MAC(JoinReq.Bssid)));
	*pCurrState = APCLI_CTRL_PROBE;
	*pLinkDownReason = APCLI_LINKDOWN_NONE;
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
				sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);
}

/*
    ==========================================================================
    Description:
	APCLI MLME JOIN req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlJoinReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Probe Req Timeout.\n", __func__));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd, &pAd->ApCfg.ApCliTab[ifIndex].wdev) == TRUE) {
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		return;
	}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pApCliEntry->ProbeReqCnt++;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Probe Req Timeout. ProbeReqCnt=%d\n",
			 __func__, pApCliEntry->ProbeReqCnt));
	if (pApCliEntry->ProbeReqCnt > 7) {
		/*
			if exceed the APCLI_MAX_PROBE_RETRY_NUM (7),
			switch to try next candidate AP.
		*/
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pApCliEntry->ProbeReqCnt = 0;

		/* Driver Trigger New Scan Mode for Sigma DUT usage */
		if (pAd->ApCfg.ApCliAutoConnectType[apcli_ifIndex] == TRIGGER_SCAN_BY_DRIVER &&
			pAd->ScanCtrl.PartialScan.bScanning == FALSE
		   ) {
			if (pApCliEntry->CfgSsidLen) {
				NDIS_802_11_SSID Ssid;

				NdisCopyMemory(Ssid.Ssid, &pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen);
				Ssid.SsidLength = pApCliEntry->CfgSsidLen;
				NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, MAC_ADDR_LEN);
				pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] = TRUE;
				pAd->ApCfg.ApCliAutoBWAdjustCnt[apcli_ifIndex] = 0;
				ApSiteSurvey_by_wdev(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pApCliEntry->wdev);
					return;
			}
		}

		if (pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE &&
			(pAd->ApCfg.ApCliAutoConnectType[apcli_ifIndex] != TRIGGER_SCAN_BY_DRIVER))
			ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

		return;
	}
#else


#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	/* stay in same state. */
	*pCurrState = APCLI_CTRL_PROBE;
	/* continue Probe Req. */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Send Probe Req.\n", __func__));
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

#ifdef WSC_AP_SUPPORT

	if ((pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.bWscTrigger == TRUE)) {
		NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
		JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscSsid.Ssid, JoinReq.SsidLen);
	} else
#endif /* WSC_AP_SUPPORT */
	{

		if (pApCliEntry->CfgSsidLen != 0) {
			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}


	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
			 JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
				sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;
	
}

/*
    ==========================================================================
    Description:
	APCLI MLME Probe Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlProbeRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	PAPCLI_STRUCT pApCliEntry;
	MLME_AUTH_REQ_STRUCT AuthReq;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   ) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s, wrong ifIndex:%d\n", __func__, ifIndex));
		return;
	}

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptCliEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

	/*Carter: Tricky point, rept used CliLink's idx to enq mlme.*/
	if (Info->CliIdx != 0xFF) {
		CliIdx = Info->CliIdx;
		pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptCliEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

#endif /* MAC_REPEATER_SUPPORT */
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;


#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		pCurrState = &pReptCliEntry->CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if (Status == MLME_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():ProbeResp success. SSID=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, pApCliEntry->Ssid, PRINT_MAC(pApCliEntry->MlmeAux.Bssid)));
		*pCurrState = APCLI_CTRL_AUTH;
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			pReptCliEntry->AuthReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->AuthReqCnt = 0;

		COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);

		/* start Authentication Req. */
		/* If AuthMode is Auto, try shared key first */
		if (IS_AKM_SHARED(pApCliEntry->MlmeAux.AKMMap) ||
			IS_AKM_AUTOSWITCH(pApCliEntry->MlmeAux.AKMMap))
			AuthReq.Alg = Ndis802_11AuthModeShared;
		else
			AuthReq.Alg = Ndis802_11AuthModeOpen;


		AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
		AuthReq.BssIdx = ifIndex;
		AuthReq.CliIdx = CliIdx;

		if (CliIdx != 0xFF) {
			ifIndex = (REPT_MLME_START_IDX + CliIdx);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
					 ("(%s) Repeater Cli Trigger Auth Req ifIndex = %d, CliIdx = %d !!!\n",
					  __func__, ifIndex, CliIdx));
		}

#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
					sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
		RTMP_MLME_HANDLER(pAd);
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Probe response fail.\n", __func__));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		   )
			ApCliSwitchCandidateAP(pAd, wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME AUTH Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuthRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	MLME_AUTH_REQ_STRUCT AuthReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

#endif /* MAC_REPEATER_SUPPORT */
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 ("(%s) Repeater Cli Receive ifIndex = %d, CliIdx = %d !!!\n",
				  __func__, ifIndex, CliIdx));
		pCurrState = &pReptEntry->CtrlCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if (Status == MLME_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("(%s) Auth Rsp Success.\n",
				  __func__));
		*pCurrState = APCLI_CTRL_ASSOC;
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			pReptEntry->AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->AssocReqCnt = 0;

#ifdef APCLI_CONNECTION_TRIAL

		/* if connection trying, wait until trialtimeout and enqueue Assoc REQ then. */
		/* TrialCh == 0 means trial has not been triggered. */
		if (pApCliEntry->TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */
			AssocParmFill(pAd,
						  &AssocReq,
						  pApCliEntry->MlmeAux.Bssid,
						  pApCliEntry->MlmeAux.CapabilityInfo,
						  ASSOC_TIMEOUT,
						  5);
#ifdef APCLI_CONNECTION_TRIAL
		}

#endif /* APCLI_CONNECTION_TRIAL */
#ifdef APCLI_CONNECTION_TRIAL

		/* if connection trying, wait until trialtimeout and enqueue Assoc REQ then. */
		/* TrialCh == 0 means trial has not been triggered. */
		if (pApCliEntry->TrialCh == 0) {
#endif /* APCLI_CONNECTION_TRIAL */
#ifdef MAC_REPEATER_SUPPORT
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
						sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
			RTMP_MLME_HANDLER(pAd);
#ifdef APCLI_CONNECTION_TRIAL
		}

#endif /* APCLI_CONNECTION_TRIAL */
	} else {
		if (IS_AKM_AUTOSWITCH(pApCliEntry->wdev.SecConfig.AKMMap) &&
			(pAd->ApCfg.ApCliTab[ifIndex].bBlockAssoc == FALSE)) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Shared Auth Rsp Failure.\n", __func__));
			*pCurrState = APCLI_CTRL_AUTH_2;
			/* start Second Authentication Req. */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) IN WEPAUTO Mode, Start Second Auth Req with OPEN.\n", __func__));
			COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
			AuthReq.Alg = Ndis802_11AuthModeOpen;
			AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
			AuthReq.BssIdx = ifIndex;
			AuthReq.CliIdx = CliIdx;
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
						sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
		} else {
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != 0xFF)
				pReptEntry->AuthReqCnt = 0;
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pApCliEntry->AuthReqCnt = 0;
			}

			*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT

			if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			   )
				ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */


		}
	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME AUTH2 Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuth2RspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

#endif /* MAC_REPEATER_SUPPORT */
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		pCurrState = &pReptEntry->CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if (Status == MLME_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Auth2 Rsp Success.\n", __func__));
		*pCurrState = APCLI_CTRL_ASSOC;
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			pReptEntry->AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->AssocReqCnt = 0;

		AssocParmFill(pAd, &AssocReq, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.CapabilityInfo,
					  ASSOC_TIMEOUT, 5);
#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
					sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
		RTMP_MLME_HANDLER(pAd);
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Apcli Auth Rsp Failure.\n", __func__));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		   )
			ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME Auth Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuthReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_AUTH_REQ_STRUCT AuthReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		pCurrState = &pReptEntry->CtrlCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) Auth Req Timeout.\n", __func__));
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF) {
		pReptEntry->AuthReqCnt++;

		if (pReptEntry->AuthReqCnt > 5) {
			/* TODO: recycle pReptEntry. */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("(%s) Rept CliIdx:%d Auth Req Timeout over 5 times.\n",
					  __func__, CliIdx));
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pReptEntry->AuthReqCnt = 0;
			return;
		}
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AuthReqCnt++;

		if (pApCliEntry->AuthReqCnt > 5) {
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->AuthReqCnt = 0;
#ifdef APCLI_AUTO_CONNECT_SUPPORT

			if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			   )
				ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			return;
		}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_AUTH;
	/* retry Authentication. */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Retry Auth Req.\n", __func__));
	COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
	AuthReq.Alg = pApCliEntry->MlmeAux.Alg; /*Ndis802_11AuthModeOpen; */
	AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
	AuthReq.BssIdx = ifIndex;
	AuthReq.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
				sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
}

/*
    ==========================================================================
    Description:
	APCLI MLME Auth2 Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAuth2ReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
}

#ifdef FAST_EAPOL_WAR
static VOID ApCliCtrlDeleteMacEntry(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR ifIndex,
	IN  UCHAR CliIdx)
{
	PAPCLI_STRUCT pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY pReptEntry = NULL;

	if (CliIdx != 0xff) {
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	} else
#endif
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	}

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF) {
		if ((pReptEntry->pre_entry_alloc == TRUE) &&
			(pReptEntry->CliValid == FALSE)) {
			UCHAR	MacTabWCID;

			MacTabWCID = pReptEntry->MacTabWCID;
			MacTableDeleteEntry(pAd, MacTabWCID, pAd->MacTab.Content[MacTabWCID].Addr);
			pReptEntry->MacTabWCID = 0xFF;
			pReptEntry->pre_entry_alloc = FALSE;
		}
	} else
#endif
	{
		if ((pApCliEntry->pre_entry_alloc == TRUE) &&
			(pApCliEntry->Valid == FALSE)) {
			UCHAR	MacTabWCID;

			MacTabWCID = pApCliEntry->MacTabWCID;
			MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, MacTabWCID));
			pApCliEntry->MacTabWCID = 0;
			pApCliEntry->pre_entry_alloc = FALSE;
		}
	}
}
#endif /* FAST_EAPOL_WAR */

/*
    ==========================================================================
    Description:
	APCLI MLME ASSOC RSP state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */
#if defined(FAST_EAPOL_WAR) || defined(APCLI_AUTO_CONNECT_SUPPORT)
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* defined(FAST_EAPOL_WAR) || defined(APCLI_AUTO_CONNECT_SUPPORT) */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
#if defined(FAST_EAPOL_WAR) || defined(APCLI_AUTO_CONNECT_SUPPORT)
		apcli_ifIndex = ifIndex;
#endif /* defined(FAST_EAPOL_WAR) || defined(APCLI_AUTO_CONNECT_SUPPORT) */
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 ("(%s) Repeater Cli Receive Assoc Rsp ifIndex = %d, CliIdx = %d.\n",
				  __func__, ifIndex, CliIdx));
		pCurrState = &pReptEntry->CtrlCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (Status == MLME_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO,
				 ("(%s) apCliIf = %d, Receive Assoc Rsp Success.\n",
				  __func__, ifIndex));
#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		if (ApCliLinkUp(pAd, ifIndex)) {
			*pCurrState = APCLI_CTRL_CONNECTED;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
			CFG80211_checkScanTable(pAd);
			RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
												   pApCliEntry->ReqVarIEs, pApCliEntry->ReqVarIELen,
												   pApCliEntry->ResVarIEs, pApCliEntry->ResVarIELen, TRUE);

			if (pAd->cfg80211_ctrl.bP2pCliPmEnable == TRUE)
				CmdP2pNoaOffloadCtrl(pAd, P2P_NOA_RX_ON);

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
		} else {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) apCliIf = %d, Insert Remote AP to MacTable failed.\n", __func__,  ifIndex));
			/* Reset the apcli interface as disconnected and Invalid. */
			*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != 0xFF)
				pAd->ApCfg.pRepeaterCliPool[CliIdx].CliValid = FALSE;
			else
#endif /* MAC_REPEATER_SUPPORT */
				pApCliEntry->Valid = FALSE;

#ifdef FAST_EAPOL_WAR
			ApCliCtrlDeleteMacEntry(pAd, apcli_ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
#ifdef APCLI_AUTO_CONNECT_SUPPORT

			if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			   )
				ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
			CFG80211_checkScanTable(pAd);
			RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
												   NULL, 0, NULL, 0, 0);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s) apCliIf = %d, Receive Assoc Rsp Failure.\n", __func__,  ifIndex));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		/* set the apcli interface be valid. */
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			pAd->ApCfg.pRepeaterCliPool[CliIdx].CliValid = FALSE;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->Valid = FALSE;

#ifdef FAST_EAPOL_WAR
		ApCliCtrlDeleteMacEntry(pAd, apcli_ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
#ifdef APCLI_AUTO_CONNECT_SUPPORT

		if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			&& (pApCliEntry->bBlockAssoc == FALSE))
			ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CFG80211_checkScanTable(pAd);
		RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(pAd, pApCliEntry->MlmeAux.Bssid,
											   NULL, 0, NULL, 0, 0);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */


	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME DeASSOC RSP state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDeAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	APCLI_CTRL_MSG_STRUCT *Info = (APCLI_CTRL_MSG_STRUCT *)(Elem->Msg);
	USHORT Status = Info->Status;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	PULONG pLinkDownReason = NULL;

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
		pCurrState = &pReptEntry->CtrlCurrState;
		pLinkDownReason = &pReptEntry->LinkDownReason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
		pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (Status == MLME_SUCCESS)
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Receive DeAssoc Rsp Success.\n", __func__));
	else
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Receive DeAssoc Rsp Failure.\n", __func__));

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		bValid = pReptEntry->CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	if (bValid) {
		*pLinkDownReason = APCLI_LINKDOWN_PEER_DEASSOC_RSP;
		ApCliLinkDown(pAd, ifIndex);
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;
}

/*
    ==========================================================================
    Description:
	APCLI MLME Assoc Req timeout state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlAssocReqTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MLME_ASSOC_REQ_STRUCT  AssocReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) Assoc Req Timeout.\n", __func__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
		pCurrState = &pReptEntry->CtrlCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	/* give up to retry authentication req after retry it 5 times. */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF) {
		pReptEntry->AssocReqCnt++;

		if (pReptEntry->AssocReqCnt > 5) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					 ("(%s) Rept CliIdx:%d, Assoc Req Timeout over 5 times\n", __func__, CliIdx));
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pReptEntry->AssocReqCnt = 0;
			return;
		}
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AssocReqCnt++;
		if (pApCliEntry->AssocReqCnt > 5) {
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->AssocReqCnt = 0;
#ifdef APCLI_AUTO_CONNECT_SUPPORT

			if ((pAd->ApCfg.ApCliAutoConnectRunning[ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			   )
				ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			return;
		}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_ASSOC;
	/* retry Association Req. */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) Retry Association Req.\n", __func__));
	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.CapabilityInfo,
				  ASSOC_TIMEOUT, 5);
#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
				sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
}

/*
    ==========================================================================
    Description:
	APCLI MLME Disconnect Rsp state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDisconnectReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	PULONG pLinkDownReason = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) MLME Request disconnect.\n", __func__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
		pCurrState = &pReptEntry->CtrlCurrState;
		pLinkDownReason = &pReptEntry->LinkDownReason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
		pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;
	}

	if (ifIndex >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s)(%d) ifIndex = %d is out of boundary\n", __func__, __LINE__, ifIndex));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		bValid = pReptEntry->CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) 2. Before do ApCliLinkDown. ifIndex = %d, CliIdx = %d, bValid = %d\n", __func__, ifIndex, CliIdx, bValid));
	else
#endif
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) 2. Before do ApCliLinkDown. ifIndex = %d, bValid = %d\n", __func__, ifIndex, bValid));

	if (bValid) {
#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		*pLinkDownReason = APCLI_LINKDOWN_DISCONNECT_REQ;
		ApCliLinkDown(pAd, ifIndex);
	}

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		if (ifIndex >= MAX_APCLI_NUM) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s)(%d) ifIndex = %d is out of boundary\n", __func__, __LINE__, ifIndex));
			return;
		}

		pApCliEntry->Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME Peer DeAssoc Req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlPeerDeAssocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR index;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	UCHAR CliIdx = 0xFF;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex = (USHORT)(Elem->Priv);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	PULONG pLinkDownReason = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) Peer DeAssoc Req.\n", __func__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = ifIndex;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		pCurrState = &pReptEntry->CtrlCurrState;
		pLinkDownReason = &pReptEntry->LinkDownReason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
		pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT

	/*check the rept entry which is linked to the CliLink should be disabled, too.*/
	if (CliIdx == 0xFF) {
		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			for (index = 0; index < GET_MAX_REPEATER_ENTRY_NUM(cap); index++) {
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[index];

				if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &pApCliEntry->wdev)) {
					if (pReptEntry->CliValid) {
						pReptEntry->LinkDownReason = APCLI_LINKDOWN_PEER_DEASSOC_REQ;
						ApCliLinkDown(pAd, (REPT_MLME_START_IDX + index));
					}
				}
			}
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		bValid = pReptEntry->CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

	if (bValid) {
		*pLinkDownReason = APCLI_LINKDOWN_PEER_DEASSOC_REQ;
		ApCliLinkDown(pAd, (UCHAR)(Elem->Priv));
	}

#ifdef APCLI_AUTO_CONNECT_SUPPORT

	if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	   ) {
		STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];

		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			ApCliSwitchCandidateAP(pAd, &pApCliEntry->wdev);
	}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
}

/*
    ==========================================================================
    Description:
	APCLI MLME Disconnect Req state machine procedure
    ==========================================================================
 */
static VOID ApCliCtrlDeAssocAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	PULONG pLinkDownReason = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) MLME Request Disconnect.\n", __func__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
		pCurrState = &pReptEntry->CtrlCurrState;
		pLinkDownReason = &pReptEntry->LinkDownReason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
		pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		bValid = pReptEntry->CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_DISASSOC_REQ,
		sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, (ULONG)(Elem->Priv));
	RTMP_MLME_HANDLER(pAd);

	if (bValid) {
		*pLinkDownReason = APCLI_LINKDOWN_DEASSOC_REQ;
		ApCliLinkDown(pAd, (UCHAR)(Elem->Priv));
	}

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DEASSOC;
}


/*
    ==========================================================================
    Description:
	APCLI MLME Disconnect Req state machine procedure
    ==========================================================================
 */
static
VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
	PULONG pLinkDownReason = NULL;
	MLME_AUX *apcli_mlme_aux;
	MLME_QUEUE_ELEM *mlmeDeauth_Elem;

#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) MLME Request Disconnect.\n", __func__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   )
		return;

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
		ifIndex = pReptEntry->MatchApCliIdx;
		pCurrState = &pReptEntry->CtrlCurrState;
		pLinkDownReason = &pReptEntry->LinkDownReason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
		pLinkDownReason = &pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	apcli_mlme_aux = &pAd->ApCfg.ApCliTab[ifIndex].MlmeAux;

	/* Fill in the related information */
	DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
	COPY_MAC_ADDR(DeAuthFrame.Addr, apcli_mlme_aux->Bssid);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		bValid = pReptEntry->CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

		os_alloc_mem(pAd, (UCHAR **)&mlmeDeauth_Elem, sizeof(MLME_QUEUE_ELEM));
	
		if(mlmeDeauth_Elem == NULL)
		{
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Send Deauth Failed!!\n", __func__));
			return;
		}
		mlmeDeauth_Elem->Priv = Elem->Priv;
		NdisCopyMemory(mlmeDeauth_Elem->Msg, &DeAuthFrame, sizeof(MLME_DEAUTH_REQ_STRUCT));
		mlmeDeauth_Elem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
		ApCliMlmeDeauthReqAction(pAd, mlmeDeauth_Elem);
		os_free_mem(mlmeDeauth_Elem);

	if (bValid) {
		*pLinkDownReason = APCLI_LINKDOWN_DEAUTH_REQ;
		ApCliLinkDown(pAd, (UCHAR)(Elem->Priv));
	}

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;
		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(apcli_mlme_aux->Bssid, MAC_ADDR_LEN);
		apcli_mlme_aux->SsidLen = 0;
		NdisZeroMemory(apcli_mlme_aux->Ssid, MAX_LEN_OF_SSID);
		apcli_mlme_aux->Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;
}

static VOID ApCliWpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN  MLME_QUEUE_ELEM *Elem)
{
	PUCHAR	pOutBuffer = NULL;
	UCHAR	Header802_3[14];
	ULONG	FrameLen = 0;
	UCHAR	*mpool;
	PEAPOL_PACKET	pPacket;
	UCHAR	Mic[16];
	BOOLEAN	bUnicast;
	UCHAR	Wcid;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliWpaMicFailureReportFrame ----->\n"));
	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	bUnicast = (Elem->Msg[0] == 1 ? TRUE:FALSE);
	pAd->Sequence = ((pAd->Sequence) + 1) & (MAX_SEQ_NUMBER);
	/* init 802.3 header and Fill Packet */
	pMacEntry = &pAd->MacTab.Content[apcli_entry->MacTabWCID];

	if (!IS_ENTRY_APCLI(pMacEntry)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) pMacEntry is not ApCli Entry\n", __func__));
		return;
	}

	Wcid =  apcli_entry->MacTabWCID;
	MAKE_802_3_HEADER(Header802_3, pAd->MacTab.Content[Wcid].Addr, wdev->if_addr, EAPOL);
	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("!!!(%s) : no memory!!!\n", __func__));
		return;
	}

	pPacket = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pPacket, TX_EAPOL_BUFFER);
	pPacket->ProVer	= EAPOL_VER;
	pPacket->ProType	= EAPOLKey;
	pPacket->KeyDesc.Type = WPA1_KEY_DESC;
	/* Request field presented */
	pPacket->KeyDesc.KeyInfo.Request = 1;

	if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher))
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 2;
	else      /* TKIP */
		pPacket->KeyDesc.KeyInfo.KeyDescVer = 1;

	pPacket->KeyDesc.KeyInfo.KeyType = (bUnicast ? PAIRWISEKEY : GROUPKEY);
	/* KeyMic field presented */
	pPacket->KeyDesc.KeyInfo.KeyMic  = 1;
	/* Error field presented */
	pPacket->KeyDesc.KeyInfo.Error  = 1;
	/* Update packet length after decide Key data payload */
	SET_UINT16_TO_ARRARY(pPacket->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);
	/* Key Replay Count */
	NdisMoveMemory(pPacket->KeyDesc.ReplayCounter, apcli_entry->ReplayCounter, LEN_KEY_DESC_REPLAY);
	/* inc_byte_array(apcli_entry->ReplayCounter, 8); */
	ADD_ONE_To_64BIT_VAR(apcli_entry->ReplayCounter);
	/* Convert to little-endian format. */
	*((USHORT *)&pPacket->KeyDesc.KeyInfo) = cpu2le16(*((USHORT *)&pPacket->KeyDesc.KeyInfo));
	MlmeAllocateMemory(pAd, (PUCHAR *)&pOutBuffer);  /* allocate memory */

	if (pOutBuffer == NULL) {
		os_free_mem(mpool);
		return;
	}

	/*
	   Prepare EAPOL frame for MIC calculation
	   Be careful, only EAPOL frame is counted for MIC calculation
	*/
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4,   pPacket,
					  END_OF_ARGS);
	/*
	   Prepare EAPOL frame for MIC calculation
	   Be careful, only EAPOL frame is counted for MIC calculation
	*/
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4,   pPacket,
					  END_OF_ARGS);
	/* Prepare and Fill MIC value */
	NdisZeroMemory(Mic, sizeof(Mic));

	if (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher)) {
		/* AES */
		UCHAR digest[20] = {0};

		RT_HMAC_SHA1(pMacEntry->SecConfig.PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(Mic, digest, LEN_KEY_DESC_MIC);
	} else {
		/* TKIP */
		RT_HMAC_MD5(pMacEntry->SecConfig.PTK, LEN_PTK_KCK, pOutBuffer, FrameLen, Mic, MD5_DIGEST_SIZE);
	}

	NdisMoveMemory(pPacket->KeyDesc.KeyMicAndData, Mic, LEN_KEY_DESC_MIC);
	/* copy frame to Tx ring and send MIC failure report frame to authenticator */
	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[Wcid],
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)pPacket,
					  CONV_ARRARY_TO_UINT16(pPacket->Body_Len) + 4, FALSE);
	MlmeFreeMemory((PUCHAR)pOutBuffer);
	os_free_mem(mpool);
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliWpaMicFailureReportFrame <-----\n"));
}


#ifdef APCLI_CERT_SUPPORT
static VOID ApCliCtrlScanDoneAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
#ifdef DOT11N_DRAFT3
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR i;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/* AP sent a 2040Coexistence mgmt frame, then station perform a scan, and then send back the respone. */
	if ((pAd->CommonCfg.BSSCoexist2040.field.InfoReq == 1)
		&& OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Update2040CoexistFrameAndNotify @%s\n", __func__));

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			if (IS_ENTRY_APCLI(&pAd->MacTab.Content[i]) && (pAd->MacTab.Content[i].func_tb_idx == ifIndex)) {
				Update2040CoexistFrameAndNotify(pAd, i, TRUE);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
				if (cap->fgRateAdaptFWOffload == TRUE) {
					CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;
					NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
					rRaParam.u4Field = RA_PARAM_HT_2040_COEX;
					RAParamUpdate(pAd, &pAd->MacTab.Content[i], &rRaParam);
				}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
			}
		}
	}

#endif /* DOT11N_DRAFT3 */
}
#endif /* APCLI_CERT_SUPPORT */

#ifdef APCLI_CONNECTION_TRIAL
/*
    ==========================================================================
    Description:
	APCLI trigger JOIN req state machine procedure
 for connect the another rootAP
    ==========================================================================
 */
static VOID ApCliCtrlTrialConnectAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	APCLI_MLME_JOIN_REQ_STRUCT JoinReq;
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	BOOLEAN	Cancelled;
	struct wifi_dev *wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
	struct freq_oper oper;

	hc_radio_query_by_wdev(wdev, &oper);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Start Probe Req Trial.\n", __func__));

	if (ifIndex >= MAX_APCLI_NUM || ifIndex == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Index: %d error!!\n", __func__, ifIndex));
		return;
	}

	if (ApScanRunning(pAd, wdev) == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Ap Scanning.....\n", __func__));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (!(pApCliEntry->TrialCh)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Didn't assign the RootAP channel\n", __func__));
		return;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) channel TrialConnectTimer\n", __func__));
	RTMPCancelTimer(&(pApCliEntry->TrialConnectTimer), &Cancelled);

	if (pApCliEntry->TrialCh != oper.prim_ch) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Jump to CH:%d\n", __func__, pApCliEntry->TrialCh));
		/* TBD disable beacon? */
		ApCliTrialConnectionBeaconControl(pAd, FALSE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s[%d]Change ch %d to %d\n\r",
				__func__, __LINE__,
				pApCliEntry->wdev.channel, pApCliEntry->TrialCh));
		/* switch channel to trial channel */
		wlan_operate_scan(wdev, pApCliEntry->TrialCh);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) set  TrialConnectTimer(%d ms)\n", __func__, TRIAL_TIMEOUT));
	RTMPSetTimer(&(pApCliEntry->TrialConnectTimer), TRIAL_TIMEOUT);

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);

	if (pApCliEntry->CfgSsidLen != 0) {
		JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
		NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
			 JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));
	*pCurrState = APCLI_CTRL_PROBE;
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
				sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);
}
#endif /* APCLI_CONNECTION_TRIAL */
#endif /* APCLI_SUPPORT */

