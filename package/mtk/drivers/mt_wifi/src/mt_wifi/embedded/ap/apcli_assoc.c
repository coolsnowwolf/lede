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
	apcli_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-6-23		modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#ifdef APCLI_CERT_SUPPORT
static VOID ApCliWpaDisassocApAndBlockAssoc(
	IN  PVOID SystemSpecific1,
	IN  PVOID FunctionContext,
	IN  PVOID SystemSpecific2,
	IN  PVOID SystemSpecific3);
#endif /* APCLI_CERT_SUPPORT */

static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN USHORT IfIndex,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE * pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE * pAddHtInfo);

static VOID set_mlme_rsn_ie(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PMAC_TABLE_ENTRY pEntry);

DECLARE_TIMER_FUNCTION(ApCliAssocTimeout);
BUILD_TIMER_FUNCTION(ApCliAssocTimeout);
#ifdef APCLI_CERT_SUPPORT
DECLARE_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc);
BUILD_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc);
#endif /* APCLI_CERT_SUPPORT */

#ifdef FAST_EAPOL_WAR
static VOID ApCliAssocDeleteMacEntry(
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
			pApCliEntry->pre_entry_alloc = FALSE;
		}
	}
}
#endif /*FAST_EAPOL_WAR*/
/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    Note:
	The state machine looks like the following
    ==========================================================================
 */
VOID ApCliAssocStateMachineInit(
	IN	PRTMP_ADAPTER	pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
	PAPCLI_STRUCT	pApCliEntry;

	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans,
					 APCLI_MAX_ASSOC_STATE, APCLI_MAX_ASSOC_MSG,
					 (STATE_MACHINE_FUNC)Drop, APCLI_ASSOC_IDLE,
					 APCLI_ASSOC_MACHINE_BASE);
	/* first column */
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAssocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDisassocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
	/* second column */
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_ASSOC_REQ,
						  (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAssoc);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_DISASSOC_REQ,
						  (STATE_MACHINE_FUNC)ApCliInvalidStateWhenDisassociate);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_DISASSOC_REQ,
						  (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliPeerAssocRspAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_ASSOC_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAssocTimeoutAction);

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		pAd->ApCfg.ApCliTab[i].AssocCurrState = APCLI_ASSOC_IDLE;
		/* timer init */
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
		RTMPInitTimer(pAd,
					  &pAd->ApCfg.ApCliTab[i].MlmeAux.ApCliAssocTimer,
					  GET_TIMER_FUNCTION(ApCliAssocTimeout),
					  (PVOID)pApCliEntry,
					  FALSE);
#ifdef APCLI_CERT_SUPPORT
		RTMPInitTimer(pAd,
					  &pAd->ApCfg.ApCliTab[i].MlmeAux.WpaDisassocAndBlockAssocTimer,
					  GET_TIMER_FUNCTION(ApCliWpaDisassocApAndBlockAssoc),
					  (PVOID)pApCliEntry,
					  FALSE);
#endif /* APCLI_CERT_SUPPORT */
	}
}

/*
    ==========================================================================
    Description:
	Association timeout procedure. After association timeout, this function
	will be called and it will put a message into the MLME queue
    Parameters:
	Standard timer parameters
    ==========================================================================
 */
static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	APCLI_STRUCT	*pAPCli = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)pAPCli->pAd;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI_ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n"));
	MlmeEnqueue(pAd,
				APCLI_ASSOC_STATE_MACHINE,
				APCLI_MT2_ASSOC_TIMEOUT,
				0,
				NULL,
				pAPCli->ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

#ifdef APCLI_CERT_SUPPORT
static VOID ApCliWpaDisassocApAndBlockAssoc(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	APCLI_STRUCT *pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;

	pApCliEntry->bBlockAssoc = TRUE;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("(%s) disassociate with current AP after sending second continuous EAPOL frame.\n", __func__));
	DisassocParmFill(pAd, &DisassocReq, pApCliEntry->MlmeAux.Bssid, REASON_MIC_FAILURE);
	MlmeEnqueue(pAd,
				APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_MLME_DISASSOC_REQ,
				sizeof(MLME_DISASSOC_REQ_STRUCT),
				&DisassocReq,
				0);
	RTMP_MLME_HANDLER(pAd);
	pApCliEntry->MicErrCnt = 0;
}
#endif /* APCLI_CERT_SUPPORT */

/*
    ==========================================================================
    Description:
	mlme assoc req handling procedure
    Parameters:
	Adapter - Adapter pointer
	Elem - MLME Queue Element
    Pre:
	the station has been authenticated and the following information is stored in the config
	    -# SSID
	    -# supported rates and their length
    Post  :
	-# An association request frame is generated and sent to the air
	-# Association timer starts
	-# Association state -> ASSOC_WAIT_RSP

    ==========================================================================
 */
static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	NDIS_STATUS		 NStatus;
	BOOLEAN          Cancelled;
	UCHAR            ApAddr[6];
	HEADER_802_11    AssocHdr;
	UCHAR            WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	USHORT           ListenIntv;
	ULONG            Timeout;
	USHORT           CapabilityInfo;
	PUCHAR           pOutBuffer = NULL;
	ULONG            FrameLen = 0;
	ULONG            tmp;
	UCHAR            SsidIe    = IE_SSID;
	UCHAR            SupRateIe = IE_SUPP_RATES;
	UCHAR            ExtRateIe = IE_EXT_SUPP_RATES;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	struct _RTMP_CHIP_CAP *cap;
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

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
		pCurrState = &pReptEntry->AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

	/* Block all authentication request durning WPA block period */
	if (apcli_entry->bBlockAssoc == TRUE) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("APCLI_ASSOC - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	} else if (MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv)) {
		/* RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled); */
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPCancelTimer(&pReptEntry->ApCliAssocTimer, &Cancelled);
		else
#endif /* MAC_REPEATER_SUPPORT */
			RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled);

		/* allocate and send out AssocRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */

		if (NStatus != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("APCLI_ASSOC - ApCliMlmeAssocReqAction() allocate memory failed\n"));
			*pCurrState = APCLI_ASSOC_IDLE;
			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
						sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			return;
		}

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI_ASSOC - Send ASSOC request...\n"));
		ApCliMgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr, ifIndex);
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AssocHdr.Addr2, pReptEntry->CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
						  sizeof(HEADER_802_11),    &AssocHdr,
						  2,                        &CapabilityInfo,
						  2,                        &ListenIntv,
						  1,                        &SsidIe,
						  1,                        &apcli_entry->MlmeAux.SsidLen,
						  apcli_entry->MlmeAux.SsidLen,     apcli_entry->MlmeAux.Ssid,
						  1,                        &SupRateIe,
						  1,                        &apcli_entry->MlmeAux.SupRateLen,
						  apcli_entry->MlmeAux.SupRateLen,  apcli_entry->MlmeAux.SupRate,
						  END_OF_ARGS);

		if (apcli_entry->MlmeAux.ExtRateLen != 0) {
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  1,                        &ExtRateIe,
							  1,                        &apcli_entry->MlmeAux.ExtRateLen,
							  apcli_entry->MlmeAux.ExtRateLen,  apcli_entry->MlmeAux.ExtRate,
							  END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11_N_SUPPORT

		/*
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP &&
			IS_INVALID_HT_SECURITY(apcli_entry->MlmeAux.PairwiseCipher)) {
			/* Force to None-HT mode due to WiFi 11n policy */
			apcli_entry->MlmeAux.HtCapabilityLen = 0;
#ifdef DOT11_VHT_AC
			apcli_entry->MlmeAux.vht_cap_len = 0;
#endif /* DOT11_VHT_AC */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s : Force AP-client as Non-HT mode\n", __func__));
		}

		/* HT */
		if ((apcli_entry->MlmeAux.HtCapabilityLen > 0) &&
			WMODE_CAP_N(apcli_entry->wdev.PhyMode)) {
			ULONG TmpLen;
			HT_CAPABILITY_IE HtCapabilityTmp;
#ifdef DOT11_VHT_AC
			struct _build_ie_info vht_ie_info;
#endif /* DOT11_VHT_AC */
			NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(&HtCapabilityTmp, &apcli_entry->MlmeAux.HtCapability, apcli_entry->MlmeAux.HtCapabilityLen);
#ifdef TXBF_SUPPORT

			if (HcIsBfCapSupport(wdev) == FALSE) {
				UCHAR ucEBfCap;

				ucEBfCap = wlan_config_get_etxbf(wdev);
				wlan_config_set_etxbf(wdev, SUBF_OFF);
				mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
				wlan_config_set_etxbf(wdev, ucEBfCap);
			}

#ifdef MAC_REPEATER_SUPPORT
			else if (pAd->ApCfg.bMACRepeaterEn) {
				UCHAR ucEBfCap;

				ucEBfCap = wlan_config_get_etxbf(wdev);
				if (cap->FlgHwTxBfCap & TXBF_AID_HW_LIMIT) {
					wlan_config_set_etxbf(wdev, SUBF_BFER);

					/* Just first cloned STA has full BF capability */
					if ((pReptEntry != NULL) && (pAd->fgClonedStaWithBfeeSelected == FALSE)) {
						MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;

						pEntry = MacTableLookup(pAd, pReptEntry->OriginalAddress);

						if (pEntry == NULL) {
							MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : HT This entry isn't belong to cloned STA!!!===============\n",
								__func__));
						}

						if ((pEntry) && (HcIsBfCapSupport(pEntry->wdev) == TRUE)) {
							wlan_config_set_etxbf(wdev, SUBF_ALL);

							if (WMODE_HT_ONLY(apcli_entry->wdev.PhyMode)) {
								pAd->fgClonedStaWithBfeeSelected = TRUE;
								pAd->ReptClonedStaEntry_CliIdx   = CliIdx;
							}

							/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : HT Cloned STA's CliIdx = %d===============\n", */
							/* __FUNCTION__, CliIdx)); */
						}
					}
				}
				mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
				wlan_config_set_etxbf(wdev, ucEBfCap);
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* TXBF_SUPPORT */
#ifdef APCLI_CERT_SUPPORT

			if (pAd->bApCliCertTest == TRUE)
				HtCapabilityTmp.HtCapInfo.GF = 1;

#endif /* APCLI_CERT_SUPPORT */
#ifdef RT_BIG_ENDIAN
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* RT_BIG_ENDINA */
			MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
							  1,                           &HtCapIe,
							  1,                           &apcli_entry->MlmeAux.HtCapabilityLen,
							  apcli_entry->MlmeAux.HtCapabilityLen, &HtCapabilityTmp,
							  END_OF_ARGS);
			FrameLen += TmpLen;
#ifdef DOT11_VHT_AC
			vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			vht_ie_info.frame_subtype = SUBTYPE_ASSOC_REQ;
			vht_ie_info.channel = apcli_entry->wdev.channel;
			vht_ie_info.phy_mode = apcli_entry->wdev.PhyMode;
			vht_ie_info.wdev = &apcli_entry->wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			ucETxBfCap = wlan_config_get_etxbf(wdev);

			if (HcIsBfCapSupport(wdev) == FALSE)
				wlan_config_set_etxbf(wdev, SUBF_OFF);

#ifdef MAC_REPEATER_SUPPORT
			else if (pAd->ApCfg.bMACRepeaterEn) {
				if (cap->FlgHwTxBfCap & TXBF_AID_HW_LIMIT) {
					wlan_config_set_etxbf(wdev, SUBF_BFER);

					/* Just first cloned STA has full BF capability */
					if ((pReptEntry != NULL) && (pAd->fgClonedStaWithBfeeSelected == FALSE)) {
						MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;

						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : OriginalAddress[0~5] = %x:%x:%x:%x:%x:%x\n",
								__func__,
								pReptEntry->OriginalAddress[0], pReptEntry->OriginalAddress[1],
								pReptEntry->OriginalAddress[2], pReptEntry->OriginalAddress[3],
								pReptEntry->OriginalAddress[4], pReptEntry->OriginalAddress[5]));
						pEntry = MacTableLookup(pAd, pReptEntry->OriginalAddress);
						if ((pEntry) && (HcIsBfCapSupport(pEntry->wdev) == TRUE)) {
							wlan_config_set_etxbf(wdev, SUBF_ALL);
							pAd->fgClonedStaWithBfeeSelected = TRUE;
							pAd->ReptClonedStaEntry_CliIdx   = CliIdx;
							/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : VHT Cloned STA's CliIdx = %d===============\n", */
							/* __FUNCTION__, CliIdx)); */
						}
					}
				}

			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
			FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
			wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
		}

#ifdef DOT11N_DRAFT3

		if (apcli_entry->MlmeAux.HtCapabilityLen > 0) {
			ULONG TmpLen, infoPos;
			EXT_CAP_INFO_ELEMENT extCapInfo;
			UCHAR extInfoLen;
			PUCHAR pInfo;
			BOOLEAN	bNeedAppendExtIE = FALSE;

			extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
			NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef APCLI_CERT_SUPPORT

			if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
				WMODE_CAP_N(wdev->PhyMode) &&
				(wdev->channel <= 14) &&
				(pAd->bApCliCertTest == TRUE)) {
				extCapInfo.BssCoexistMgmtSupport = 1;
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s: BssCoexistMgmtSupport = 1\n", __func__));
			}

#endif /* APCLI_CERT_SUPPORT */
#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode) &&
				(wdev->channel > 14))
				extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
			pInfo = (PUCHAR)(&extCapInfo);

			for (infoPos = 0; infoPos < extInfoLen; infoPos++) {
				if (pInfo[infoPos] != 0) {
					bNeedAppendExtIE = TRUE;
					break;
				}
			}

			if (bNeedAppendExtIE == TRUE) {
				for (infoPos = (extInfoLen - 1); infoPos >= EXT_CAP_MIN_SAFE_LENGTH; infoPos--) {
					if (pInfo[infoPos] == 0)
						extInfoLen--;
					else
						break;
				}
#ifdef RT_BIG_ENDIAN
				RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								  1, &ExtCapIe,
								  1, &extInfoLen,
								  extInfoLen,		&extCapInfo,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
		/*
			add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
			Case I: (Aggregation + Piggy-Back)
				1. user enable aggregation, AND
				2. Mac support piggy-back
				3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
			Case II: (Aggregation)
				1. user enable aggregation, AND
				2. AP annouces it's AGGREGATION-capable in BEACON
		*/
		FrameLen += build_vendor_ie(pAd, wdev, pOutBuffer + FrameLen, VIE_ASSOC_REQ
								   );

		if (apcli_entry->MlmeAux.APEdcaParm.bValid) {
			if (apcli_entry->wdev.UapsdInfo.bAPSDCapable &&
				apcli_entry->MlmeAux.APEdcaParm.bAPSDCapable) {
				QBSS_STA_INFO_PARM QosInfo;

				NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(PUCHAR)&QosInfo;
			} else {
				/* The Parameter Set Count is set to ��0�� in the association request frames */
				/* WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		apcli_entry->ReqVarIELen = 0;
		NdisZeroMemory(apcli_entry->ReqVarIEs, MAX_VIE_LEN);

		if ((apcli_entry->wpa_supplicant_info.WpaSupplicantUP & 0x7F) ==  WPA_SUPPLICANT_ENABLE) {
			ULONG TmpWpaAssocIeLen = 0;

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:: APCLI WPA_ASSOC_IE FROM SUPPLICANT\n", __func__));
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpWpaAssocIeLen,
							  apcli_entry->wpa_supplicant_info.WpaAssocIeLen, apcli_entry->wpa_supplicant_info.pWpaAssocIe,
							  END_OF_ARGS);
			FrameLen += TmpWpaAssocIeLen;
			VarIesOffset = 0;
			NdisMoveMemory(apcli_entry->ReqVarIEs + VarIesOffset,
						   apcli_entry->wpa_supplicant_info.pWpaAssocIe, apcli_entry->wpa_supplicant_info.WpaAssocIeLen);
			VarIesOffset += apcli_entry->wpa_supplicant_info.WpaAssocIeLen;
			/* Set Variable IEs Length */
			apcli_entry->ReqVarIELen = VarIesOffset;
		} else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

			/* Append RSN_IE when WPAPSK OR WPA2PSK, */
			if (IS_AKM_PSK(apcli_entry->MlmeAux.AKMMap)
#ifdef WSC_AP_SUPPORT
				&& ((apcli_entry->wdev.WscControl.WscConfMode == WSC_DISABLE)
					|| ((apcli_entry->wdev.WscControl.WscConfMode != WSC_DISABLE)
						&& !(apcli_entry->wdev.WscControl.bWscTrigger)))
#endif /* WSC_AP_SUPPORT */
			   ) {
				{
					ULONG TempLen = 0;
					CHAR rsne_idx = 0;
					{ /* Todo by Eddy: It's not good code */
						struct _SECURITY_CONFIG *pSecConfig = &apcli_entry->wdev.SecConfig;
						UINT32 AKMMap = pSecConfig->AKMMap;
						UINT32 PairwiseCipher = pSecConfig->PairwiseCipher;
						UINT32 GroupCipher = pSecConfig->GroupCipher;

						pSecConfig->AKMMap = apcli_entry->MlmeAux.AKMMap;
						pSecConfig->PairwiseCipher = apcli_entry->MlmeAux.PairwiseCipher;
						pSecConfig->GroupCipher = apcli_entry->MlmeAux.GroupCipher;
#ifdef DOT11W_PMF_SUPPORT
						/* Need to fill the pSecConfig->PmfCfg to let WPAMakeRSNIE() generate correct RSNIE*/
						{
							RSN_CAPABILITIES RsnCap;

							NdisMoveMemory(&RsnCap, &apcli_entry->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
							RsnCap.word = cpu2le16(RsnCap.word);
							/* init to FALSE */
							pSecConfig->PmfCfg.UsePMFConnect = FALSE;
							pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;

							/*mismatch case*/
							if (((pSecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
								|| ((pSecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
								pSecConfig->PmfCfg.UsePMFConnect = FALSE;
								pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
							}

							if ((pSecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
								pSecConfig->PmfCfg.UsePMFConnect = TRUE;

								if ((apcli_entry->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
									pSecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;
							}

							pSecConfig->PmfCfg.igtk_cipher = apcli_entry->MlmeAux.IntegrityGroupCipher;
						}
#endif /* DOT11W_PMF_SUPPORT */
						WPAMakeRSNIE(apcli_entry->wdev.wdev_type, pSecConfig, NULL);
						pSecConfig->AKMMap = AKMMap;
						pSecConfig->PairwiseCipher = PairwiseCipher;
						pSecConfig->GroupCipher = GroupCipher;
					}

					for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
						if (apcli_entry->wdev.SecConfig.RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
							continue;

						MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
										  1, &apcli_entry->wdev.SecConfig.RSNE_EID[rsne_idx][0],
										  1, &apcli_entry->wdev.SecConfig.RSNE_Len[rsne_idx],
										  apcli_entry->wdev.SecConfig.RSNE_Len[rsne_idx], &apcli_entry->wdev.SecConfig.RSNE_Content[rsne_idx][0],
										  END_OF_ARGS);
						FrameLen += TempLen;
					}
				}
			}

#ifdef WSC_AP_SUPPORT

		/* Add WSC IE if we are connecting to WSC AP */
		if ((pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.bWscTrigger)) {
			UCHAR *pWscBuf = NULL, WscIeLen = 0;
			ULONG WscTmpLen = 0;

			os_alloc_mem(pAd, (UCHAR **) &pWscBuf, 512);

			if (pWscBuf != NULL) {
				NdisZeroMemory(pWscBuf, 512);
				WscBuildAssocReqIE(&pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl, pWscBuf, &WscIeLen);
				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &WscTmpLen, WscIeLen, pWscBuf,
								  END_OF_ARGS);
				FrameLen += WscTmpLen;
				os_free_mem(pWscBuf);
			} else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s:: WscBuf Allocate failed!\n",
						 __func__));
		}

#endif /* WSC_AP_SUPPORT */
#ifdef FAST_EAPOL_WAR
		/*
		 * insert WTBL here,unicast wcid can be found after associate request sent out
		 * The purpose is to let eapol packet can be received in rx tasklet.
		 * Otherwisw, eapol packet will be dropped since mac entry does not inserted yet
		 */
		{
			PAPCLI_STRUCT pApCliEntry;
			PMAC_TABLE_ENTRY pMacEntry = NULL;

			pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef MAC_REPEATER_SUPPORT

			if ((pAd->ApCfg.bMACRepeaterEn) &&
				(IS_HIF_TYPE(pAd, HIF_MT)) &&
				(CliIdx != 0xFF)
			   ) {
				if (pReptEntry->pre_entry_alloc == TRUE)
					ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);

				{
					pMacEntry = MacTableInsertEntry(
									pAd,
									(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
									pReptEntry->wdev,
									ENTRY_REPEATER,
									OPMODE_AP,
									TRUE);

					if (pMacEntry) {
						pReptEntry->MacTabWCID = pMacEntry->wcid;
						pReptEntry->pre_entry_alloc = TRUE;
					} else {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("repeater pEntry insert fail"));
						MlmeFreeMemory(pOutBuffer);
						return;
					}
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				if (pApCliEntry->pre_entry_alloc == TRUE)
					ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);

				{
					pMacEntry = MacTableInsertEntry(
									pAd,
									(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
									&pApCliEntry->wdev,
									ENTRY_APCLI,
									OPMODE_AP,
									TRUE);

					if (pMacEntry) {
						pApCliEntry->pre_entry_alloc = TRUE;
						pApCliEntry->MacTabWCID = pMacEntry->wcid;
					} else {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("apcli pEntry insert fail"));
						MlmeFreeMemory(pOutBuffer);
						return;
					}
				}
			}
		}
#endif /* FAST_EAPOL_WAR */

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			RTMPSetTimer(&pReptEntry->ApCliAssocTimer, Timeout);
		else
#endif /* MAC_REPEATER_SUPPORT */
			RTMPSetTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, Timeout);

		*pCurrState = APCLI_ASSOC_WAIT_RSP;
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("APCLI_ASSOC - ApCliMlmeAssocReqAction() sanity check failed. BUG!!!!!!\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
}

/*
    ==========================================================================
    Description:
	Upper layer issues disassoc request
    Parameters:
	Elem -
    ==========================================================================
 */
static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
	HEADER_802_11         DisassocHdr;
	PUCHAR                 pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* FAST_EAPOL_WAR || MAC_REPEATER_SUPPORT*/

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
		pCurrState = &pReptEntry->AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	/* skip sanity check */
	pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);
	/* allocate and send out DeassocReq frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("APCLI_ASSOC - ApCliMlmeDisassocReqAction() allocate memory failed\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = CliIdx;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		return;
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("APCLI_ASSOC - Send DISASSOC request [BSSID::%02x:%02x:%02x:%02x:%02x:%02x]\n",
			  pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
			  pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5]));
	ApCliMgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr, ifIndex);
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.pRepeaterCliPool[CliIdx].CurrentAddress);

#endif /* MAC_REPEATER_SUPPORT */
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),	&DisassocHdr,
					  2,						&pDisassocReq->Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	*pCurrState = APCLI_ASSOC_IDLE;
	ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
}


/*
    ==========================================================================
    Description:
	peer sends assoc rsp back
    Parameters:
	Elme - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	BOOLEAN				Cancelled;
	USHORT				CapabilityInfo, Status, Aid;
	UCHAR				SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR				ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR				Addr2[MAC_ADDR_LEN];
	EDCA_PARM			EdcaParm;
	UCHAR				CkipFlag;
	APCLI_CTRL_MSG_STRUCT	ApCliCtrlMsg;
	HT_CAPABILITY_IE	HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR				HtCapabilityLen;
	UCHAR				AddHtInfoLen;
	UCHAR				NewExtChannelOffset = 0xff;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif  /* defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT) */
	IE_LISTS *ie_list = NULL;

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
		pCurrState = &pReptEntry->AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	pEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("%s():mem alloc failed!\n", __func__));
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));

	if (ApCliPeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2,
								&CapabilityInfo, &Status, &Aid, SupRate,
								&SupRateLen, ExtRate, &ExtRateLen, &HtCapability,
								&AddHtInfo, &HtCapabilityLen, &AddHtInfoLen, &NewExtChannelOffset,
								&EdcaParm, &CkipFlag, ie_list)) {
		/* The frame is for me ? */
		if (MAC_ADDR_EQUAL(Addr2, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid)) {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
			PFRAME_802_11 pFrame =  (PFRAME_802_11) (Elem->Msg);
			/* Store the AssocRsp Frame to wpa_supplicant via CFG80211 */
			NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ResVarIEs, MAX_VIE_LEN);
			pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen = 0;
			pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen = Elem->MsgLen - 6 - sizeof(HEADER_802_11);
			NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].ResVarIEs, &pFrame->Octet[6], pAd->ApCfg.ApCliTab[ifIndex].ResVarIELen);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("APCLI_ASSOC - receive ASSOC_RSP to me (status=%d)\n",
					 Status));


#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != 0xFF)
				RTMPCancelTimer(&pReptEntry->ApCliAssocTimer, &Cancelled);
			else
#endif /* MAC_REPEATER_SUPPORT */
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAssocTimer, &Cancelled);


			if (Status == MLME_SUCCESS) {
				/* go to procedure listed on page 376 */
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				{
					ApCliAssocPostProc(pAd, Addr2, CapabilityInfo, ifIndex, SupRate, SupRateLen,
									   ExtRate, ExtRateLen, &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);
					pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Aid = Aid;
#ifdef DOT11_VHT_AC
					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_op, sizeof(VHT_OP_IE));
					pApCliEntry->MlmeAux.vht_cap_len = 0;
					pApCliEntry->MlmeAux.vht_op_len = 0;

					if (WMODE_CAP_AC(pApCliEntry->wdev.PhyMode) && ie_list->vht_cap_len && ie_list->vht_op_len) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("There is vht le at Assoc Rsp ifIndex=%d vht_cap_len=%d\n",
								 ifIndex, ie_list->vht_cap_len));
						NdisMoveMemory(&pApCliEntry->MlmeAux.vht_cap, &(ie_list->vht_cap), ie_list->vht_cap_len);
						pApCliEntry->MlmeAux.vht_cap_len = ie_list->vht_cap_len;
						NdisMoveMemory(&pApCliEntry->MlmeAux.vht_op, &(ie_list->vht_op), ie_list->vht_op_len);
						pApCliEntry->MlmeAux.vht_op_len = ie_list->vht_op_len;
					}

#endif /* DOT11_VHT_AC */
				}


#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx != 0xFF)
					pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
#endif /* MAC_REPEATER_SUPPORT */
				set_mlme_rsn_ie(pAd, &pApCliEntry->wdev, pEntry);

				/* For Repeater get correct wmm valid setting */
				pApCliEntry->MlmeAux.APEdcaParm.bValid = EdcaParm.bValid;
				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = CliIdx;
				ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
				RTMP_MLME_HANDLER(pAd);
			} else {
#ifdef FAST_EAPOL_WAR
				ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
				ApCliCtrlMsg.Status = Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
				RTMP_MLME_HANDLER(pAd);
			}

			*pCurrState = APCLI_ASSOC_IDLE;
		}
	} else
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("APCLI_ASSOC - ApCliPeerAssocRspAction() sanity check fail\n"));

	if (ie_list)
		os_free_mem(ie_list);
}

/*
    ==========================================================================
    Description:
	left part of IEEE 802.11/1999 p.374
    Parameters:
	Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	USHORT        Reason;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
	ULONG *pDisconnect_Sub_Reason = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */

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
		pCurrState = &pReptEntry->AssocCurrState;
		pDisconnect_Sub_Reason = &pReptEntry->Disconnect_Sub_Reason;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;
		pDisconnect_Sub_Reason = &pAd->ApCfg.ApCliTab[ifIndex].Disconnect_Sub_Reason;
	}

	if (PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason)) {
#ifdef FAST_EAPOL_WAR
		ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */

		if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, Addr2)) {
			*pCurrState = APCLI_ASSOC_IDLE;
#ifdef MAC_REPEATER_SUPPORT
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			*pDisconnect_Sub_Reason = (ULONG)Reason;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
			RTMP_MLME_HANDLER(pAd);
		}
	} else
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("APCLI_ASSOC - ApCliPeerDisassocAction() sanity check fail\n"));
}

/*
    ==========================================================================
    Description:
	what the state machine will do after assoc timeout
    ==========================================================================
 */
static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("APCLI_ASSOC - ApCliAssocTimeoutAction\n"));

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
		pCurrState = &pReptEntry->AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	*pCurrState = APCLI_ASSOC_IDLE;
#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd,
				APCLI_CTRL_STATE_MACHINE,
				APCLI_CTRL_ASSOC_REQ_TIMEOUT,
				0,
				NULL,
				ifIndex);
}

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */

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
		pCurrState = &pAd->ApCfg.pRepeaterCliPool[CliIdx].AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("APCLI_ASSOC - ApCliInvalidStateWhenAssoc(state=%ld), reset APCLI_ASSOC state machine\n",
			  *pCurrState));
#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	*pCurrState = APCLI_ASSOC_IDLE;
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd,
				APCLI_CTRL_STATE_MACHINE,
				APCLI_CTRL_ASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT),
				&ApCliCtrlMsg,
				ifIndex);
	RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#if defined(FAST_EAPOL_WAR) || defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* defined(FAST_EAPOL_WAR) || defined (MAC_REPEATER_SUPPORT) */

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
		pCurrState = &pAd->ApCfg.pRepeaterCliPool[CliIdx].AssocCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

#ifdef FAST_EAPOL_WAR
	ApCliAssocDeleteMacEntry(pAd, ifIndex, CliIdx);
#endif /* FAST_EAPOL_WAR */
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("APCLI_ASSOC - InvalidStateWhenApCliDisassoc(state=%ld), reset APCLI_ASSOC state machine\n",
			  *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
}

static VOID set_mlme_rsn_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PMAC_TABLE_ENTRY pEntry)
{
	ULONG Idx;

	/* Set New WPA information */
	Idx = BssTableSearch(&pAd->ScanTab, pEntry->Addr, wdev->channel);
	if (Idx == BSS_NOT_FOUND) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ASSOC - Can't find BSS after receiving Assoc response\n"));
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if (IS_AKM_WPA_CAPABILITY(wdev->SecConfig.AKMMap)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (pAd->ScanTab.BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;

			pVIE = pAd->ScanTab.BssEntry[Idx].VarIEs;
			len = pAd->ScanTab.BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;
				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& (IS_AKM_WPA1(wdev->SecConfig.AKMMap)
					|| IS_AKM_WPA1PSK(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():=> Store RSN_IE for WPA SM negotiation\n", __func__));
				}
				/* For WPA2/WPA2PSK */
				else if ((pEid->Eid == IE_RSN)
					 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					 && (IS_AKM_WPA2(wdev->SecConfig.AKMMap)
						 || IS_AKM_WPA2PSK(wdev->SecConfig.AKMMap))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():=> Store RSN_IE for WPA2 SM negotiation\n", __func__));
				}


				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

		}

		if (pEntry->RSNIE_Len == 0) {
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():=> no RSN_IE\n", __func__));
		} else {
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
		}
	}

}

/*
    ==========================================================================
    Description:
	procedures on IEEE 802.11/1999 p.376
    Parametrs:
    ==========================================================================
 */
static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr2,
	IN USHORT CapabilityInfo,
	IN USHORT IfIndex,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen,
	IN ADD_HT_INFO_IE *pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;

	if (IfIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];
	pApCliEntry->MlmeAux.BssType = BSS_INFRA;
	pApCliEntry->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));
	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, SupRate, SupRateLen);
	RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &(pApCliEntry->MlmeAux.SupRateLen), pApCliEntry->wdev.PhyMode);
	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, ExtRate, ExtRateLen);
	RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &(pApCliEntry->MlmeAux.ExtRateLen), pApCliEntry->wdev.PhyMode);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 (HtCapabilityLen ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __func__));
#ifdef DOT11_N_SUPPORT

	if (HtCapabilityLen > 0 && WMODE_CAP_N(pApCliEntry->wdev.PhyMode))
		ApCliCheckHt(pAd, IfIndex, pHtCapability, pAddHtInfo);

#endif /* DOT11_N_SUPPORT */
}

#endif /* APCLI_SUPPORT */

