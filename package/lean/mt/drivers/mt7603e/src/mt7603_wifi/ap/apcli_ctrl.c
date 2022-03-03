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

#ifdef WH_EZ_SETUP
#include "easy_setup/ez_mod_hooks.h"
#endif
#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_conn_perm_lock;
#endif
#endif

static VOID ApCliCtrlJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlJoinReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlProbeRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuthRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuth2RspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuthReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAuth2ReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDeAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlAssocReqTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDisconnectReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlPeerDeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliCtrlDeAssocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

#ifndef WH_EZ_SETUP
static
#endif
VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

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
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR j;
#endif /* MAC_REPEATER_SUPPORT */

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_CTRL_STATE, APCLI_MAX_CTRL_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_CTRL_DISCONNECTED,
		APCLI_CTRL_MACHINE_BASE);

	/* disconnected state */
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_JOIN_REQ, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_DISCONNECTED, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#endif /*MAC_REPEATER_SUPPORT*/
	/* probe state */
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_JOIN_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlJoinReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#endif /*MAC_REPEATER_SUPPORT*/
#ifdef WH_EZ_SETUP
	StateMachineSetAction(Sm, APCLI_CTRL_PROBE, APCLI_CTRL_JOIN_FAIL, (STATE_MACHINE_FUNC)ez_ApCliCtrlJoinFailAction);
#endif

	/* auth state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuthRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuthReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#endif /*MAC_REPEATER_SUPPORT*/
 	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
#ifdef WH_EZ_SETUP
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH, APCLI_CTRL_JOIN_FAIL, (STATE_MACHINE_FUNC)ez_ApCliCtrlJoinFailAction);
#endif

	/* auth2 state */
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAuth2RspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_AUTH_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAuth2ReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDisconnectReqAction);
#endif /* MAC_REPEATER_SUPPORT */
 	StateMachineSetAction(Sm, APCLI_CTRL_AUTH_2, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);

	/* assoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlAssocRspAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_ASSOC_REQ_TIMEOUT, (STATE_MACHINE_FUNC)ApCliCtrlAssocReqTimeoutAction);
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDeAssocAction);
#endif /* MAC_REPEATER_SUPPORT */
 	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_PEER_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
#ifdef WH_EZ_SETUP
	StateMachineSetAction(Sm, APCLI_CTRL_ASSOC, APCLI_CTRL_JOIN_FAIL, (STATE_MACHINE_FUNC)ez_ApCliCtrlJoinFailAction);
#endif

	/* deassoc state */
	StateMachineSetAction(Sm, APCLI_CTRL_DEASSOC, APCLI_CTRL_DEASSOC_RSP, (STATE_MACHINE_FUNC)ApCliCtrlDeAssocRspAction);

	/* connected state */
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_DISCONNECT_REQ, (STATE_MACHINE_FUNC)ApCliCtrlDeAuthAction);
#ifdef MAC_REPEATER_SUPPORT
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_DEL_MACREPENTRY,
			(STATE_MACHINE_FUNC)ApCliCtrlDeAuthAction);
#endif /* MAC_REPEATER_SUPPORT */
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_PEER_DISCONNECT_REQ,
			(STATE_MACHINE_FUNC)ApCliCtrlPeerDeAssocReqAction);
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_MT2_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliCtrlProbeRspAction);
#ifdef WH_EZ_SETUP
	StateMachineSetAction(Sm, APCLI_CTRL_CONNECTED, APCLI_CTRL_JOIN_FAIL, (STATE_MACHINE_FUNC)ez_ApCliCtrlJoinFailAction);
#endif

	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].CtrlCurrState = APCLI_CTRL_DISCONNECTED;

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].pAd = pAd;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchApCliIdx = i;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchLinkIdx = j;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].AuthCurrState = APCLI_CTRL_DISCONNECTED;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CliConnectState = 0;
		}
#endif /* MAC_REPEATER_SUPPORT */
	}

	return;
}


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
	PWSC_CTRL	pWpsCtrl = &pAd->ApCfg.ApCliTab[ifIndex].WscControl;
#endif /* WSC_AP_SUPPORT */

struct wifi_dev *wdev;
#ifdef WH_EZ_SETUP
#ifndef NEW_CONNECTION_ALGO
		UCHAR ez_connection;
#endif
#endif /* WH_EZ_SETUP */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Start Probe Req.\n", __FUNCTION__));
	if (ifIndex >= MAX_APCLI_NUM)
		return;

	if (ApScanRunning(pAd) == TRUE)
		return;

	wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(wdev)) {
#ifdef EZ_REGROUP_SUPPORT
				if(wdev->ez_driver_params.regrp_mode == REGRP_MODE_BLOCKED){
					EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,("ApCliCtrlJoinReqAction: Join block by Regrp wdev_idx=%d\n", wdev->wdev_idx));
					return;
				}
#endif
				if (ez_update_connection_permission(wdev->sys_handle, wdev, EZ_DISALLOW_ALL_ALLOW_ME)) {
				} else {
					return;
				}
			}
#endif

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

#ifdef WH_EZ_SETUP
		// Ensure start with empty table & complete all candidate attempt before operation is terminated
		if(IS_EZ_SETUP_ENABLED(wdev))
			BssTableInit(&pApCliEntry->MlmeAux.SsidBssTab); 
#endif

#ifdef APCLI_OWE_SUPPORT
	if (pApCliEntry->wdev.AuthMode == Ndis802_11AuthModeOWE)
		BssTableInit(&pApCliEntry->MlmeAux.owe_bss_tab);
#endif

	if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
	{
		COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
	}

#ifdef WSC_AP_SUPPORT
    if ((pWpsCtrl->WscConfMode != WSC_DISABLE) &&
		(pWpsCtrl->bWscTrigger == TRUE))
    {
    	ULONG bss_idx = 0;
        NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
        JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, JoinReq.SsidLen);
#ifdef WH_EZ_SETUP		
		if (IS_EZ_SETUP_ENABLED(wdev)) {
			COPY_MAC_ADDR(pApCliEntry->CfgApCliBssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscBssid);
			COPY_MAC_ADDR(JoinReq.Bssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscBssid);
		}
#endif		

		if (pWpsCtrl->WscMode == 1) /* PIN */
		{
#ifdef AP_SCAN_SUPPORT
			if ((pWpsCtrl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) &&
					pAd->ScanCtrl.PartialScan.bScanning &&
					pAd->ScanCtrl.PartialScan.LastScanChannel != 0)
				return;
#endif /* AP_SCAN_SUPPORT */

			bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PUCHAR)(JoinReq.Ssid), JoinReq.SsidLen);
			if (bss_idx == BSS_NOT_FOUND)
			{
#ifdef AP_SCAN_SUPPORT
				if (pWpsCtrl->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
					if (!pAd->ScanCtrl.PartialScan.bScanning &&
						(pAd->ScanCtrl.PartialScan.LastScanChannel == 0)) {
						pAd->ScanCtrl.PartialScan.pwdev = wdev;
						pAd->ScanCtrl.PartialScan.bScanning = TRUE;
					}
				}
#endif /* AP_SCAN_SUPPORT */

				ApSiteSurvey(pAd, NULL, SCAN_WSC_ACTIVE, FALSE, wdev);

#ifdef WH_EZ_SETUP				
				if (IS_EZ_SETUP_ENABLED(wdev) && ez_update_connection_permission(pAd, wdev, EZ_ALLOW_ALL))
				{
					return;
				} else {
					if (IS_EZ_SETUP_ENABLED(wdev))
						ASSERT(FALSE);
				}
#endif	
				return;
			}
			else
			{
				INT old_conf_mode = pWpsCtrl->WscConfMode;
				ADD_HTINFO	RootApHtInfo, ApHtInfo;
				UCHAR channel = pAd->CommonCfg.Channel, RootApChannel = pAd->ScanTab.BssEntry[bss_idx].Channel;
				UCHAR RootApCentralChannel = pAd->ScanTab.BssEntry[bss_idx].CentralChannel;
				ApHtInfo = pAd->CommonCfg.AddHTInfo.AddHtInfo;
				RootApHtInfo = pAd->ScanTab.BssEntry[bss_idx].AddHtInfo.AddHtInfo;

				if ((RootApChannel != channel) ||
					((RootApCentralChannel != RootApChannel) &&
					 (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
					 (ApHtInfo.ExtChanOffset != RootApHtInfo.ExtChanOffset)))
				{
					RTMP_STRING ChStr[5] = {0};
					if (pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40)
					{
						if (RootApHtInfo.ExtChanOffset == EXTCHA_ABOVE)
							Set_HtExtcha_Proc(pAd, "1");
						else
							Set_HtExtcha_Proc(pAd, "0");
					}
					snprintf(ChStr, sizeof(ChStr), "%d", pAd->ScanTab.BssEntry[bss_idx].Channel);
#ifdef WH_EZ_SETUP
					if(IS_EZ_SETUP_ENABLED(wdev))
					{
						wdev->ez_driver_params.do_not_restart_interfaces = TRUE;
						rtmp_set_channel(pAd, wdev, pAd->ScanTab.BssEntry[bss_idx].Channel);
					} else
#endif
					{
						Set_Channel_Proc(pAd, ChStr);
					}
#ifdef WH_EZ_SETUP
					if(IS_EZ_SETUP_ENABLED(wdev))
					{
						wdev->ez_driver_params.do_not_restart_interfaces = FALSE;
					}
#endif

					/*
						ApStop will call WscStop, we need to reset WscConfMode, WscMode & bWscTrigger here.
					*/

					pWpsCtrl->WscState = WSC_STATE_START;
					pWpsCtrl->WscStatus = STATUS_WSC_START_ASSOC;
					pWpsCtrl->WscMode = 1;
					pWpsCtrl->WscConfMode = old_conf_mode;
					pWpsCtrl->bWscTrigger = TRUE;
#ifdef WH_EZ_SETUP					
					if (IS_EZ_SETUP_ENABLED(wdev) && ez_update_connection_permission(pAd, wdev, EZ_ALLOW_ALL))
					{
						return;
					} else {
						if (IS_EZ_SETUP_ENABLED(wdev)){
							ASSERT(FALSE);
						}
					} 
#endif				
					return;
				}				
			}
		}
    }
    else
#endif /* WSC_AP_SUPPORT */
	{
#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev) && !wdev->ez_driver_params.ez_wps_reconnect) {
			pApCliEntry->MlmeAux.attempted_candidate_index = EZ_INDEX_NOT_FOUND;
#ifdef NEW_CONNECTION_ALGO
			//ez_connection = TRUE;
		
			if (ez_is_connection_allowed(wdev) == TRUE) {
				if (ez_apcli_search_best_ap(pAd,pApCliEntry,ifIndex) == FALSE)
				{
					EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("(%s) No provider found.\n", __FUNCTION__));
					if (!ez_update_connection_permission(pAd, wdev, EZ_ALLOW_ALL))
					{
						ASSERT(FALSE);
					}
					return;
				}
			}
#else
			ez_connection = FALSE;
			if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev) && 
				pApCliEntry->wdev.ez_driver_params.keep_finding_provider) {
				ez_connection = TRUE;
				if (MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR) &&
					(ez_search_provider(pAd, pApCliEntry, ifIndex) == FALSE)) {
					EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
						("(%s) No provider found.\n", __FUNCTION__));
					return;
				}
			}
#endif
			if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			{
				COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
			}
		}
		if (IS_EZ_SETUP_ENABLED(wdev) && wdev->ez_driver_params.ez_wps_reconnect) {
			COPY_MAC_ADDR(pApCliEntry->CfgApCliBssid, wdev->ez_driver_params.ez_wps_bssid);	
			COPY_MAC_ADDR(JoinReq.Bssid, wdev->ez_driver_params.ez_wps_bssid);	
		}
			
#endif /* WH_EZ_SETUP */

#ifdef APCLI_OWE_SUPPORT
	/*owe_trans_ssid present then update join request with OWE ssid, bssid parameters */
		if (pApCliEntry->owe_trans_ssid_len > 0) {

			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->owe_trans_ssid, pApCliEntry->owe_trans_ssid_len);
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->owe_trans_bssid);
		} else
#endif

		if (pApCliEntry->CfgSsidLen != 0)
		{

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(wdev) && !wdev->ez_driver_params.ez_wps_reconnect)
			{
#ifndef NEW_CONNECTION_ALGO
			//	COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
				if (ez_connection == FALSE) {
					if (ez_avoid_looping_connection(pAd, pApCliEntry, ifIndex) == FALSE)
						return;
					else
						COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
				}
#endif
#ifdef NEW_CONNECTION_ALGO
				  COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
#endif
			}
#endif/* WH_EZ_SETUP */

			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
#ifdef WH_EZ_SETUP	// Fix made to pass SSID properly
			if( IS_EZ_SETUP_ENABLED(wdev) ){
				NdisMoveMemory((JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
			}
			else
#endif

			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__, JoinReq.Ssid, PRINT_MAC(JoinReq.Bssid)));

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev))
	{
		if(IS_SINGLE_CHIP_DBDC(pAd))
		{
			if (!(ez_update_connection_permission(pAd, wdev, EZ_DISALLOW_ALL_ALLOW_ME))){
				ASSERT(FALSE);
				return;
			}
		}
#ifdef DUAl_CHIP
		else
		{
			if (!(ez_update_connection_permission(pAd, wdev, EZ_DISALLOW_ALL_ALLOW_ME))) 
							return;
		}
#endif			
	}
#endif	

	*pCurrState = APCLI_CTRL_PROBE;

	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;
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
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#ifdef WH_EZ_SETUP
	BOOLEAN attempt_same_peer = TRUE;
#endif /* WH_EZ_SETUP */

#ifdef WH_EZ_SETUP
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		if(IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Probe Req Timeout.\n", __FUNCTION__));
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Req Timeout.\n", __FUNCTION__));

	if (ifIndex >= MAX_APCLI_NUM)
		return;
#ifdef WH_EZ_SETUP
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if(IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)){
		EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) Probe Req Timeout.\n", __FUNCTION__));
#ifdef EZ_REGROUP_SUPPORT
		if(pApCliEntry->wdev.ez_driver_params.regrp_mode == REGRP_MODE_BLOCKED){
			EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) Conn blocked for regroup wdev_idx:%d.\n",
				__FUNCTION__, pApCliEntry->wdev.wdev_idx));
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			goto Fail;
		}
#endif
	}
#endif

	if (ApScanRunning(pAd) == TRUE)
	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[ifIndex].wdev)){
			goto Fail;
		}
		else
#endif
		return;
	}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pApCliEntry->ProbeReqCnt++;
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Req Timeout. ProbeReqCnt=%d\n",
				__FUNCTION__, pApCliEntry->ProbeReqCnt));
#ifdef WH_EZ_SETUP
	if((IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)) && ( pApCliEntry->ProbeReqCnt > 2 )) 
		attempt_same_peer = ez_probe_count_handle(pApCliEntry); 	
	else
#endif

	if (pApCliEntry->ProbeReqCnt > 7)
	{
		/*
			if exceed the APCLI_MAX_PROBE_RETRY_NUM (7),
			switch to try next candidate AP.
		*/
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisZeroMemory(JoinReq.Bssid, MAC_ADDR_LEN);
		NdisZeroMemory(pApCliEntry->CfgApCliBssid, MAC_ADDR_LEN);
		pApCliEntry->ProbeReqCnt = 0;
#ifdef APCLI_OWE_SUPPORT
		apcli_reset_owe_parameters(pAd, ifIndex);
#endif

		
		if (pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
			ApCliSwitchCandidateAP(pAd);
		return;
	}
#else

#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
			attempt_same_peer = FALSE;
#endif
	
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_PROBE;

	/* retry Probe Req. */
#ifdef WH_EZ_SETUP
	if(!IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
#endif	
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Probe Req.\n", __FUNCTION__));

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	NdisZeroMemory(&JoinReq, sizeof(APCLI_MLME_JOIN_REQ_STRUCT));

#ifdef WH_EZ_SETUP
	if (!IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
#endif
		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		{
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
		}

#ifdef WSC_AP_SUPPORT
    if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
    {
        NdisZeroMemory(JoinReq.Ssid, MAX_LEN_OF_SSID);
        JoinReq.SsidLen = pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.SsidLength;
		NdisMoveMemory(JoinReq.Ssid, pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscSsid.Ssid, JoinReq.SsidLen);
    }
    else
#endif /* WSC_AP_SUPPORT */
	{
#ifdef WH_EZ_SETUP
		if((IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)) && (!attempt_same_peer) && !pApCliEntry->wdev.ez_driver_params.ez_wps_reconnect){
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" %s(apcli%d) Band = %d-->\n", __FUNCTION__, pApCliEntry->wdev.func_idx, pApCliEntry->wdev.channel > 14));
		
			if (ez_join_timeout_handle(pAd, ifIndex)== FALSE)
				goto Fail;
		}
			
		if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev) && pApCliEntry->wdev.ez_driver_params.ez_wps_reconnect) {
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->wdev.ez_driver_params.ez_wps_bssid);	
		}
#endif

#ifdef APCLI_OWE_SUPPORT
/*Configure OWE ssid and bssid Join request parameters */
		if (pApCliEntry->wdev.AuthMode == Ndis802_11AuthModeOWE && (pApCliEntry->owe_trans_ssid_len != 0)) {
			JoinReq.SsidLen = pApCliEntry->owe_trans_ssid_len;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->owe_trans_ssid, JoinReq.SsidLen);
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->owe_trans_bssid);
		} else
#endif
		if (pApCliEntry->CfgSsidLen != 0)
		{
			JoinReq.SsidLen = pApCliEntry->CfgSsidLen;
			NdisMoveMemory(&(JoinReq.Ssid), pApCliEntry->CfgSsid, JoinReq.SsidLen);
		}
	}
#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
	{
		if (!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		{
			COPY_MAC_ADDR(JoinReq.Bssid, pApCliEntry->CfgApCliBssid);
		}

		if(IS_SINGLE_CHIP_DBDC(pAd))
		{
			if (!(ez_update_connection_permission(pAd, &pApCliEntry->wdev, EZ_DISALLOW_ALL_ALLOW_ME))){
				ASSERT(FALSE);
				*pCurrState = APCLI_CTRL_DISCONNECTED;
				goto Fail;
			}
		}
	}
#endif	

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe Ssid=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
		__FUNCTION__, JoinReq.Ssid, JoinReq.Bssid[0], JoinReq.Bssid[1], JoinReq.Bssid[2],
		JoinReq.Bssid[3], JoinReq.Bssid[4], JoinReq.Bssid[5]));
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_PROBE_REQ,
		sizeof(APCLI_MLME_JOIN_REQ_STRUCT), &JoinReq, ifIndex);

	return;

#ifdef WH_EZ_SETUP
Fail:

	if (IS_EZ_SETUP_ENABLED((&pAd->ApCfg.ApCliTab[ifIndex].wdev))){

		ez_restore_channel_config(&pAd->ApCfg.MBSSID[ifIndex].wdev);
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid,MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = 0;
		pAd->ApCfg.ApCliTab[ifIndex].CfgSsidLen = pAd->ApCfg.MBSSID[ifIndex].SsidLen;
		NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgSsid,pAd->ApCfg.MBSSID[ifIndex].Ssid,pAd->ApCfg.MBSSID[ifIndex].SsidLen);
		//Clear scan entry as Join timeout may happen if bss entry is stale.
		BssTableInit(&pAd->ScanTab);
		*pCurrState = APCLI_CTRL_DISCONNECTED;
		if (!ez_update_connection_permission(pAd,&pAd->ApCfg.ApCliTab[ifIndex].wdev,EZ_ALLOW_ALL))
		{
			ASSERT(FALSE);
		}

	}
#endif
	
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
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}

	if (Info->CliIdx != 0xFF)
		CliIdx = Info->CliIdx;
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev) 
		&& pApCliEntry->MlmeAux.support_easy_setup) {
#ifdef NEW_CONNECTION_ALGO
		if (ez_is_connection_allowed(wdev) == FALSE)
			Status = MLME_ASSOC_REJ_TEMPORARILY;
#endif
	}
#endif /* WH_EZ_SETUP */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pApCliEntry->CtrlCurrState;

	if (Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():ProbeResp success. SSID=%s, Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
					__FUNCTION__, pApCliEntry->Ssid, PRINT_MAC(pApCliEntry->MlmeAux.Bssid)));

		*pCurrState = APCLI_CTRL_AUTH;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->AuthReqCnt = 0;

		COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);

		/* start Authentication Req. */
		/* If AuthMode is Auto, try shared key first */
#ifdef APCLI_SAE_SUPPORT
		if (pApCliEntry->pre_mac_entry.AuthMode == Ndis802_11AuthModeWPA3PSK) {
			UCHAR if_addr[MAC_ADDR_LEN];
			UCHAR pmkid[80];
			UCHAR pmk[LEN_PMK];
			UCHAR has_pmkid = FALSE;

			NdisZeroMemory(if_addr, MAC_ADDR_LEN);
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				NdisCopyMemory(if_addr, &pApCliEntry->RepeaterCli[CliIdx].CurrentAddress, MAC_ADDR_LEN);
			else
#endif /* MAC_REPEATER_SUPPORT */
				NdisCopyMemory(if_addr, &pApCliEntry->wdev.if_addr, MAC_ADDR_LEN);

			if (sae_get_pmk_cache(&pAd->SaeCfg, if_addr, pApCliEntry->MlmeAux.Bssid, pmkid, pmk)) {
				apcli_add_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid,
					pmkid, pmk, LEN_PMK, ifIndex
#ifdef MAC_REPEATER_SUPPORT
					, CliIdx
#endif
					);
				has_pmkid = TRUE;
			}
			if (has_pmkid == TRUE) {
				COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
				AuthReq.Alg = AUTH_MODE_OPEN;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
						("(%s) - use pmkid\n", __func__));
			} else {
			COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
			AuthReq.Alg = AUTH_MODE_SAE;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
					("(%s) - use SAE\n", __func__));
			}
		} else

#endif/* APCLI_SAE_SUPPORT */

		if ((wdev->AuthMode == Ndis802_11AuthModeShared) ||
			(wdev->AuthMode == Ndis802_11AuthModeAutoSwitch))
			AuthReq.Alg = Ndis802_11AuthModeShared;
		else
			AuthReq.Alg = Ndis802_11AuthModeOpen;

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			&& pApCliEntry->MlmeAux.support_easy_setup) {
			AuthReq.Alg = AUTH_MODE_EZ;
		}
#endif /* WH_EZ_SETUP */

		AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
		AuthReq.BssIdx = ifIndex;
		AuthReq.CliIdx = CliIdx;
		if (CliIdx != 0xFF)
		{
			ifIndex = (64 + 16*ifIndex + CliIdx);
			DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Trigger Auth Req ifIndex = %d, CliIdx = %d !!!\n",
							__FUNCTION__, ifIndex, CliIdx));
		}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_SAE_SUPPORT
		if (AuthReq.Alg == AUTH_MODE_SAE)
			MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_SAE_AUTH_REQ,
				sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
		else
#endif

		MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
			sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Probe respond fail.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)){
			*pCurrState = APCLI_CTRL_PROBE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
		}
#endif /* WH_EZ_SETUP */

	}

	return;
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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	PULONG pAssocCurrState = NULL;
	USHORT Timeout = ASSOC_TIMEOUT;
#endif
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED((&pApCliEntry->wdev)) 
		&& pApCliEntry->MlmeAux.support_easy_setup){
#ifdef NEW_CONNECTION_ALGO
		if (ez_is_connection_allowed(&pApCliEntry->wdev) == FALSE)
			Status = MLME_ASSOC_REJ_TEMPORARILY;
#endif
	}
#endif /* WH_EZ_SETUP */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Receive ifIndex = %d, CliIdx = %d !!!\n",
						__FUNCTION__, ifIndex, CliIdx));

		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAssocCurrState = &pApCliEntry->RepeaterCli[CliIdx].AssocCurrState;
#endif
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pApCliEntry->CtrlCurrState;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAssocCurrState = &pApCliEntry->AssocCurrState;
#endif
	}


	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Rsp Success.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AssocReqCnt = 0;
		else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->AssocReqCnt = 0;

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		if (pApCliEntry->pre_mac_entry.AuthMode == Ndis802_11AuthModeWPA3PSK
			|| pApCliEntry->pre_mac_entry.AuthMode == Ndis802_11AuthModeOWE) {
			pAssocCurrState = ASSOC_IDLE;
			Timeout = 5000;
		}
#endif


		AssocParmFill(pAd, &AssocReq, pApCliEntry->MlmeAux.Bssid, pApCliEntry->MlmeAux.CapabilityInfo,
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			Timeout
#else
			ASSOC_TIMEOUT
#endif
			, 5);

#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ,
			sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);
	} 
	else
	{
		if (pApCliEntry->wdev.AuthMode == Ndis802_11AuthModeAutoSwitch)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Rsp Failure.\n", __FUNCTION__));

			*pCurrState = APCLI_CTRL_AUTH_2;

			/* start Second Authentication Req. */
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Start Second Auth Rep.\n", __FUNCTION__));
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
		}
		else
		{
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pApCliEntry->AuthReqCnt = 0;
			}
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
				ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)){
				*pCurrState = APCLI_CTRL_AUTH;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
			}
#endif /* WH_EZ_SETUP */

		}
	}

	return;
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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	PULONG pAssocCurrState = NULL;
#endif
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}
#endif /* MAC_REPEATER_SUPPORT */

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF) {
		pCurrState = &pApCliEntry->RepeaterCli[CliIdx].CtrlCurrState;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAssocCurrState = &pApCliEntry->RepeaterCli[CliIdx].AssocCurrState;
#endif
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pApCliEntry->CtrlCurrState;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAssocCurrState = &pApCliEntry->AssocCurrState;
#endif
	}
	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth2 Rsp Success.\n", __FUNCTION__));
		*pCurrState = APCLI_CTRL_ASSOC;

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			pApCliEntry->RepeaterCli[CliIdx].AssocReqCnt = 0;
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
	} 
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Apcli Auth Rsp Failure.\n", __FUNCTION__));

		*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
	}

	return;
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
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Auth Req Timeout.\n", __FUNCTION__));

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt++;

		if (pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pApCliEntry->RepeaterCli[CliIdx].AuthReqCnt = 0;
			/* allow other packet trigger auth request agian */
			RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);

			return;
		}
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AuthReqCnt++;

#ifdef WH_EZ_SETUP
		if ((IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)) && (pApCliEntry->AuthReqCnt > 2)) // allow two retries
		{
			*pCurrState = APCLI_CTRL_AUTH; // redundant??
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
			return;
		}
		else
#endif /* WH_EZ_SETUP */

		if (pApCliEntry->AuthReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			NdisZeroMemory(pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
			NdisZeroMemory(pApCliEntry->MlmeAux.Ssid, MAX_LEN_OF_SSID);
			pApCliEntry->AuthReqCnt = 0;
#ifdef APCLI_OWE_SUPPORT
			apcli_reset_owe_parameters(pAd, ifIndex);
#endif
		
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			return;
	}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_AUTH;

	/* retry Authentication. */
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Auth Req.\n", __FUNCTION__));
	COPY_MAC_ADDR(AuthReq.Addr, pApCliEntry->MlmeAux.Bssid);
	AuthReq.Alg = pApCliEntry->MlmeAux.Alg; /*Ndis802_11AuthModeOpen; */

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		&& pApCliEntry->MlmeAux.support_easy_setup) {
		AuthReq.Alg = AUTH_MODE_EZ;
	}
#endif /* WH_EZ_SETUP */

	AuthReq.Timeout = AUTH_TIMEOUT;
#ifdef MAC_REPEATER_SUPPORT
	AuthReq.BssIdx = ifIndex;
	AuthReq.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_MLME_AUTH_REQ,
		sizeof(MLME_AUTH_REQ_STRUCT), &AuthReq, ifIndex);

	return;
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
	return;
}

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
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);

		DBGPRINT(RT_DEBUG_ERROR, ("(%s) Repeater Cli Receive Assoc Rsp ifIndex = %d, CliIdx = %d.\n",
					__FUNCTION__, ifIndex, CliIdx));
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)
		&& pApCliEntry->MlmeAux.support_easy_setup) {
#ifdef NEW_CONNECTION_ALGO
				if (ez_is_connection_allowed(&pApCliEntry->wdev) == FALSE)
					Status = MLME_ASSOC_REJ_TEMPORARILY;
#endif
	}
#endif /* WH_EZ_SETUP */

	if(Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Success.\n", __FUNCTION__, ifIndex));


#ifdef MAC_REPEATER_SUPPORT
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		if (ApCliLinkUp(pAd, ifIndex))
		{
			*pCurrState = APCLI_CTRL_CONNECTED;

			
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Insert Remote AP to MacTable failed.\n", __FUNCTION__,  ifIndex));
			/* Reset the apcli interface as disconnected and Invalid. */
			*pCurrState = APCLI_CTRL_DISCONNECTED;
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				ifIndex = ((ifIndex - 64) / 16);
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			pApCliEntry->Valid = FALSE;
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
				ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */


		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) apCliIf = %d, Receive Assoc Rsp Failure.\n", __FUNCTION__,  ifIndex));

		*pCurrState = APCLI_CTRL_DISCONNECTED;

		/* set the apcli interface be valid. */
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
		{
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		pApCliEntry->Valid = FALSE;

#ifdef APCLI_SAE_SUPPORT
		if (Status == MLME_INVALID_PMKID) {
			UCHAR if_addr[MAC_ADDR_LEN];
			INT CachedIdx;
			UCHAR pmkid[80];
			UCHAR pmk[LEN_PMK];

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				NdisCopyMemory(if_addr, &pApCliEntry->RepeaterCli[CliIdx].CurrentAddress, MAC_ADDR_LEN);
			else
#endif /* MAC_REPEATER_SUPPORT */
				NdisCopyMemory(if_addr, &pApCliEntry->wdev.if_addr, MAC_ADDR_LEN);

/*Received PMK invalid status from AP delete entry from SavedPMK and delete SAE instance*/


			if (pApCliEntry->pre_mac_entry.AuthMode == Ndis802_11AuthModeWPA3PSK &&
				sae_get_pmk_cache(&pAd->SaeCfg, if_addr, pApCliEntry->MlmeAux.Bssid, pmkid, pmk)
				) {

				CachedIdx = apcli_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex
#ifdef MAC_REPEATER_SUPPORT
					, CliIdx
#endif
					);

				if (CachedIdx != INVALID_PMKID_IDX) {
					SAE_INSTANCE *pSaeIns =
						search_sae_instance(&pAd->SaeCfg, if_addr, pApCliEntry->MlmeAux.Bssid);

					MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
						("[SAE]Reconnection falied with pmkid ,delete cache entry and sae instance\n"));
					if (pSaeIns != NULL)
						delete_sae_instance(pSaeIns);

					apcli_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex
#ifdef MAC_REPEATER_SUPPORT
						, CliIdx
#endif
						);
				}
			}
		}
#endif/*APCLI_SAE_SUPPORT*/

		
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */


#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)){
			*pCurrState = APCLI_CTRL_ASSOC;
			if(Status == MLME_EZ_CONNECTION_LOOP)
				ez_initiate_new_scan_hook(pApCliEntry->wdev.ez_driver_params.ez_ad);
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
		}
#endif

	}

	return;
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
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if (Status == MLME_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Receive DeAssoc Rsp Success.\n", __FUNCTION__));
	} else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) Receive DeAssoc Rsp Failure.\n", __FUNCTION__));
	}

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);
	
	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
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
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Assoc Req Timeout.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* give up to retry authentication req after retry it 5 times. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx !=0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt++;
		if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt > 5)
		{
			*pCurrState = APCLI_CTRL_DISCONNECTED;
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocReqCnt = 0;
			/* allow other packet trigger auth request agian */
			RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);

			return;
		}
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->AssocReqCnt++;

#ifdef WH_EZ_SETUP
		if ((IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)) && (pApCliEntry->AssocReqCnt > 2)) // allow two assoc retries
		{
			*pCurrState = APCLI_CTRL_ASSOC; // redundant
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
			return;
		}
		else
#endif
		{
			if (pApCliEntry->AssocReqCnt > 5)
			{
				*pCurrState = APCLI_CTRL_DISCONNECTED;
				NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
				NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
				pApCliEntry->AssocReqCnt = 0;
			
#ifdef APCLI_AUTO_CONNECT_SUPPORT
				if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
					&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
					)
					ApCliSwitchCandidateAP(pAd);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
				return;
			}
		}
	}

	/* stay in same state. */
	*pCurrState = APCLI_CTRL_ASSOC;

	/* retry Association Req. */
	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Retry Association Req.\n", __FUNCTION__));
	AssocParmFill(pAd, &AssocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.CapabilityInfo,
		ASSOC_TIMEOUT, 5);

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_ASSOC_REQ, 
		sizeof(MLME_ASSOC_REQ_STRUCT), &AssocReq, ifIndex);

	return;
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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	PULONG pAuthCurrState = NULL;
#endif
	BOOLEAN bValid = FALSE;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAuthCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
#endif
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		pAuthCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;
#endif
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_ERROR, ("(%s) 2. Before do ApCliLinkDown.\n", __FUNCTION__));
	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
	}
	else
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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		*pAuthCurrState = AUTH_REQ_IDLE;
#endif
		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}

	return;
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
		UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) Peer DeAssoc Req.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
	{
		UCHAR index;
		BOOLEAN Cancelled;

		for(index = 0; index < MAX_EXT_MAC_ADDR_SIZE; index++)
		{
			if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].CliEnable)
			{
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].ApCliAuthTimer, &Cancelled);
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].ApCliAssocTimer, &Cancelled);
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[index].CliValid)
					ApCliLinkDown(pAd, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + index));
				RTMPRemoveRepeaterEntry(pAd, ifIndex, index);
			}
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
	bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
	if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
	{
		STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pApCliEntry->MacTabWCID];
		if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			ApCliSwitchCandidateAP(pAd);
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */	

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);		
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
	pApCliEntry->Valid = FALSE;

	/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
#ifdef APCLI_OWE_SUPPORT
		apcli_reset_owe_parameters(pAd, ifIndex);
#endif

	}

#ifdef WH_EZ_SETUP
	if( (IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)) && 
		( (*pCurrState == APCLI_CTRL_AUTH) || (*pCurrState == APCLI_CTRL_AUTH_2) ||(*pCurrState == APCLI_CTRL_ASSOC) || 
		  (pApCliEntry->MlmeAux.attempted_candidate_index != EZ_INDEX_NOT_FOUND)
#ifdef WSC_AP_SUPPORT
		  || ((pApCliEntry->WscControl.WscConfMode != WSC_DISABLE) && (pApCliEntry->WscControl.bWscTrigger == TRUE))
#endif	  
		  || (pApCliEntry->wdev.ez_driver_params.ez_wps_reconnect)
		)
	  )
	{
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("(%s) Connection attempt failed by peer\n", __FUNCTION__));		
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_FAIL,0, NULL, ifIndex);
		RTMP_MLME_HANDLER(pAd);
	}else
#endif
	{
		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
	return;
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
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_DISASSOC_REQ,
		sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, ifIndex);

	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);		
	}
	else
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

	return;
}


/* 
    ==========================================================================
    Description:
        APCLI MLME Disconnect Req state machine procedure
    ==========================================================================
 */
#ifndef WH_EZ_SETUP
static
#endif
VOID ApCliCtrlDeAuthAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PAPCLI_STRUCT pApCliEntry;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
#ifdef WH_EZ_SETUP	
#ifdef EZ_NETWORK_MERGE_SUPPORT
		BOOLEAN deauth_in_same_context = FALSE;
#endif
#endif
	
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("(%s) MLME Request Disconnect.\n", __FUNCTION__));

#ifdef WH_EZ_SETUP
#ifdef EZ_NETWORK_MERGE_SUPPORT
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd) && (ifIndex & IMM_DISCONNECT))
		{
			EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ApCliCtrlDeAuthAction: Deauth In same context\n"));
			ifIndex &= (!IMM_DISCONNECT);
			Elem->Priv &= (!IMM_DISCONNECT);
			deauth_in_same_context = TRUE;
		}
#endif
#endif

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* Fill in the related information */
	DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
	COPY_MAC_ADDR(DeAuthFrame.Addr, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;
	else
#endif /* MAC_REPEATER_SUPPORT */
		bValid = pApCliEntry->Valid;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WH_EZ_SETUP
#ifdef EZ_NETWORK_MERGE_SUPPORT
	if (IS_EZ_SETUP_ENABLED((&pApCliEntry->wdev)) && deauth_in_same_context == TRUE)
	{
		MLME_QUEUE_ELEM *mlmeDeauth_Elem;
		os_alloc_mem(pAd, (UCHAR **)&mlmeDeauth_Elem, sizeof(MLME_QUEUE_ELEM));
	
		if(mlmeDeauth_Elem == NULL)
		{
			EZ_DEBUG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set::OID_802_11_DEAUTHENTICATION, Failed!!\n"));
			return;
		}
		mlmeDeauth_Elem->Priv = Elem->Priv;
		NdisCopyMemory(mlmeDeauth_Elem->Msg,&DeAuthFrame,sizeof(MLME_DEAUTH_REQ_STRUCT));
		mlmeDeauth_Elem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
		ApCliMlmeDeauthReqAction( pAd,mlmeDeauth_Elem);
		os_free_mem(NULL, mlmeDeauth_Elem);
	} else
#endif 
#endif 
	{
		MlmeEnqueue(pAd, 
				  APCLI_AUTH_STATE_MACHINE, 
				  APCLI_MT2_MLME_DEAUTH_REQ, 
				  sizeof(MLME_DEAUTH_REQ_STRUCT),
				  &DeAuthFrame, 
				  ifIndex);
	}
#ifdef APCLI_DOT11W_PMF_SUPPORT
	/* speed up the de-auth is send out before mac entry is delete for */
	/*PMF 5.3.3.3 trasmit encrypt de-auth/ dis-asso req*/
	/*Still have chance to delete mac entry before de-auth is sent out.*/
	RTMP_MLME_HANDLER(pAd);
#endif /* APCLI_DOT11W_PMF_SUPPORT */
	if (bValid)
		ApCliLinkDown(pAd, ifIndex);

	/* set the apcli interface be invalid. */
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		//RTMPDelRepeaterCliAsicEntry(pAd, CliIdx);
		RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;

		/* clear MlmeAux.Ssid and Bssid. */
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.SsidLen = 0;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Rssi = 0;
	}

	*pCurrState = APCLI_CTRL_DISCONNECTED;

	return;
}

#endif /* APCLI_SUPPORT */

