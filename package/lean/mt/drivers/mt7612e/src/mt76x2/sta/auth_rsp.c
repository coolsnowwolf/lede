/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	auth_rsp.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John		2004-10-1		copy from RT2560
*/
#include "rt_config.h"

/*
    ==========================================================================
    Description:
        authentication state machine init procedure
    Parameters:
        Sm - the state machine
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID AuthRspStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN PSTATE_MACHINE Sm,
	IN STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, Trans, MAX_AUTH_RSP_STATE, MAX_AUTH_RSP_MSG,
			 (STATE_MACHINE_FUNC) Drop, AUTH_RSP_IDLE,
			 AUTH_RSP_MACHINE_BASE);

	/* column 1 */
	StateMachineSetAction(Sm, AUTH_RSP_IDLE, MT2_PEER_DEAUTH,
			      (STATE_MACHINE_FUNC) PeerDeauthAction);

	/* column 2 */
	StateMachineSetAction(Sm, AUTH_RSP_WAIT_CHAL, MT2_PEER_DEAUTH,
			      (STATE_MACHINE_FUNC) PeerDeauthAction);

}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
*/
VOID PeerAuthSimpleRspGenAndSend(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHdr80211,
	IN USHORT Alg,
	IN USHORT Seq,
	IN USHORT Reason,
	IN USHORT Status)
{
	HEADER_802_11 AuthHdr;
	ULONG FrameLen = 0;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;

	if (Reason != MLME_SUCCESS) {
		DBGPRINT(RT_DEBUG_TRACE, ("Peer AUTH fail...\n"));
		return;
	}

	/*Get an unused nonpaged memory */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("Send AUTH response (seq#2)...\n"));
	MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pHdr80211->Addr2,
						pAd->CurrentAddress,
						pAd->MlmeAux.Bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof (HEADER_802_11),
			  &AuthHdr, 2, &Alg, 2, &Seq, 2, &Reason, END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
*/
VOID PeerDeauthAction(
	IN PRTMP_ADAPTER pAd,
	IN PMLME_QUEUE_ELEM Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	USHORT Reason;
	BOOLEAN bDoIterate = FALSE;

	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason)) {
		if (INFRA_ON(pAd)
		    && (MAC_ADDR_EQUAL(Addr1, pAd->CurrentAddress)
			|| MAC_ADDR_EQUAL(Addr1, BROADCAST_ADDR))
		    && MAC_ADDR_EQUAL(Addr2, pAd->CommonCfg.Bssid)
		    && MAC_ADDR_EQUAL(Addr3, pAd->CommonCfg.Bssid)
#ifdef DOT11R_FT_SUPPORT
		    && ((pAd->StaCfg.Dot11RCommInfo.bInMobilityDomain == FALSE)
			|| (pAd->StaCfg.Dot11RCommInfo.FtRspSuccess == 0))
#endif /* DOT11R_FT_SUPPORT */
		    ) {
			struct wifi_dev *wdev = &pAd->StaCfg.wdev;

			DBGPRINT(RT_DEBUG_TRACE,
				 ("AUTH_RSP - receive DE-AUTH from our AP (Reason=%d)\n",
				  Reason));

			if (Reason == REASON_4_WAY_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
						      IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG,
						      NULL, 0, 0);

			if (Reason == REASON_GROUP_KEY_HS_TIMEOUT)
				RTMPSendWirelessEvent(pAd,
						      IW_GROUP_HS_TIMEOUT_EVENT_FLAG,
						      NULL, 0, 0);

#ifdef WAPI_SUPPORT
			WAPI_InternalCmdAction(pAd,
					       wdev->AuthMode,
					       BSS0,
					       Addr2, WAI_MLME_DISCONNECT);
#endif /* WAPI_SUPPORT */

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev,
						RT_WLAN_EVENT_CGIWAP, -1, NULL,
						NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

			/* send wireless event - for deauthentication */
			RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, NULL,
					      BSS0, 0);



#ifdef WPA_SUPPLICANT_SUPPORT
			if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE)
			    && (wdev->AuthMode == Ndis802_11AuthModeWPA2)
			    && (wdev->PortSecured == WPA_802_1X_PORT_SECURED))
				pAd->StaCfg.wpa_supplicant_info.bLostAp = TRUE;
#endif /* WPA_SUPPLICANT_SUPPORT */

			/*
			   Some customer would set AP1 & AP2 same SSID, AuthMode & EncrypType but different WPAPSK,
			   therefore we need to do iterate here.
			 */
			if ((wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			    &&
			    ((wdev->AuthMode == Ndis802_11AuthModeWPAPSK)
			     || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
#ifdef WSC_STA_SUPPORT
			    && (pAd->StaCfg.WscControl.WscState < WSC_STATE_LINK_UP)
#endif /* WSC_STA_SUPPORT */
			    )
				bDoIterate = TRUE;

			LinkDown(pAd, TRUE);

			if (bDoIterate) {
				pAd->MlmeAux.BssIdx++;
				IterateOnBssTab(pAd);
			}

		}
#ifdef ADHOC_WPA2PSK_SUPPORT
		else if (ADHOC_ON(pAd)
			 && (MAC_ADDR_EQUAL(Addr1, pAd->CurrentAddress)
			     || MAC_ADDR_EQUAL(Addr1, BROADCAST_ADDR))) {
			MAC_TABLE_ENTRY *pEntry;

			pEntry = MacTableLookup(pAd, Addr2);
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

			DBGPRINT(RT_DEBUG_TRACE,
				 ("AUTH_RSP - receive DE-AUTH from %02x:%02x:%02x:%02x:%02x:%02x \n",
				  PRINT_MAC(Addr2)));
		}
#endif /* ADHOC_WPA2PSK_SUPPORT */
	} else {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("AUTH_RSP - PeerDeauthAction() sanity check fail\n"));
	}
}
