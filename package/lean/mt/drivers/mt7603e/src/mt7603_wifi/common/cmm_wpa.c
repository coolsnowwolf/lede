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
	wpa.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Jan	Lee		03-07-22		Initial
	Paul Lin	03-11-28		Modify for supplicant
*/
#include "rt_config.h"

/* WPA OUI*/
UCHAR		OUI_WPA[3]				= {0x00, 0x50, 0xF2};
UCHAR		OUI_WPA_NONE_AKM[4]		= {0x00, 0x50, 0xF2, 0x00};
UCHAR       OUI_WPA_VERSION[4]      = {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_WEP40[4]      = {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_TKIP[4]     = {0x00, 0x50, 0xF2, 0x02};
UCHAR       OUI_WPA_CCMP[4]     = {0x00, 0x50, 0xF2, 0x04};
UCHAR       OUI_WPA_WEP104[4]      = {0x00, 0x50, 0xF2, 0x05};
UCHAR       OUI_WPA_8021X_AKM[4]	= {0x00, 0x50, 0xF2, 0x01};
UCHAR       OUI_WPA_PSK_AKM[4]      = {0x00, 0x50, 0xF2, 0x02};
/* WPA2 OUI*/
UCHAR		OUI_WPA2[3]				= {0x00, 0x0F, 0xAC};
UCHAR       OUI_WPA2_WEP40[4]   = {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_TKIP[4]        = {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_CCMP[4]        = {0x00, 0x0F, 0xAC, 0x04};
UCHAR       OUI_WPA2_8021X_AKM[4]   = {0x00, 0x0F, 0xAC, 0x01};
UCHAR       OUI_WPA2_PSK_AKM[4]   	= {0x00, 0x0F, 0xAC, 0x02};
UCHAR       OUI_WPA2_WEP104[4]   = {0x00, 0x0F, 0xAC, 0x05};
UCHAR       OUI_WPA2_1X_SHA256[4]   = {0x00, 0x0F, 0xAC, 0x05};
UCHAR       OUI_WPA2_PSK_SHA256[4]   = {0x00, 0x0F, 0xAC, 0x06};

#ifdef CONFIG_OWE_SUPPORT
UCHAR OUI_WPA2_CIPHER_CCMP128[4]       = {0x00, 0x0F, 0xAC, 0x04};
UCHAR OUI_WPA2_CIPHER_CCMP256[4]       = {0x00, 0x0F, 0xAC, 0x0A};
#endif

#ifdef DOT11_SAE_SUPPORT
UCHAR       OUI_WPA2_AKM_SAE_SHA256[4]   = {0x00, 0x0F, 0xAC, 0x08};
#endif
#ifdef CONFIG_OWE_SUPPORT
UCHAR       OUI_WPA2_AKM_OWE[4]  = {0x00, 0x0F, 0xAC, 0x12/*d'18*/};
#endif


#ifdef DOT11R_FT_SUPPORT
UCHAR		OUI_FT_8021X_AKM[4] 	= {0x00, 0x0F, 0xAC, 0x03};
UCHAR		OUI_FT_PSK_AKM[4] 		= {0x00, 0x0F, 0xAC, 0x04};
#endif /* DOT11R_FT_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
UCHAR		OSEN_IE[] = {0x50,0x6f,0x9a,0x12,0x00,0x0f,0xac,0x07,0x01,0x00,0x00,0x0f,0xac,0x04,0x01,0x00,0x50,0x6f,0x9a,0x01,0x00,0x00};
UCHAR		OSEN_IELEN = sizeof(OSEN_IE);
#endif

static VOID	ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN 	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	OUT PEAPOL_PACKET   pMsg);

static VOID WpaEAPPacketAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLASFAlertAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLLogoffAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem);

static VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem);

static VOID WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem);

static VOID WpaEAPOLTimeout(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);


/*
    ==========================================================================
    Description:
        association state machine init, including state transition and timer init
    Parameters:
        S - pointer to the association state machine
    ==========================================================================
 */
VOID WpaStateMachineInit(
    IN  PRTMP_ADAPTER   pAd,
    IN  STATE_MACHINE *S,
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_WPA_PTK_STATE, MAX_WPA_MSG, (STATE_MACHINE_FUNC)Drop, WPA_PTK, WPA_MACHINE_BASE);

    StateMachineSetAction(S, WPA_PTK, MT2_EAPPacket, (STATE_MACHINE_FUNC)WpaEAPPacketAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLStart, (STATE_MACHINE_FUNC)WpaEAPOLStartAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLLogoff, (STATE_MACHINE_FUNC)WpaEAPOLLogoffAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLKey, (STATE_MACHINE_FUNC)WpaEAPOLKeyAction);
    StateMachineSetAction(S, WPA_PTK, MT2_EAPOLASFAlert, (STATE_MACHINE_FUNC)WpaEAPOLASFAlertAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLTIMEOUT, (STATE_MACHINE_FUNC)WpaEAPOLTimeout);
}

/*
    ==========================================================================
    Description:
        this is state machine function.
        When receiving EAP packets which is  for 802.1x authentication use.
        Not use in PSK case
    Return:
    ==========================================================================
*/
VOID WpaEAPPacketAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID WpaEAPOLASFAlertAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

VOID WpaEAPOLLogoffAction(
    IN PRTMP_ADAPTER pAd,
    IN MLME_QUEUE_ELEM *Elem)
{
}

/*
    ==========================================================================
    Description:
       Start 4-way HS when rcv EAPOL_START which may create by our driver in assoc.c
    Return:
    ==========================================================================
*/
VOID WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{
    MAC_TABLE_ENTRY     *pEntry;
	STA_TR_ENTRY *tr_entry;
    PHEADER_802_11      pHeader;


    DBGPRINT(RT_DEBUG_TRACE, ("WpaEAPOLStartAction ===> \n"));

    pHeader = (PHEADER_802_11)Elem->Msg;

    /*For normaol PSK, we enqueue an EAPOL-Start command to trigger the process.*/
    if (Elem->MsgLen == 6)
        pEntry = MacTableLookup(pAd, Elem->Msg);
    else
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);
#ifdef WSC_AP_SUPPORT
        /*
            a WSC enabled AP must ignore EAPOL-Start frames received from clients that associated to
            the AP with an RSN IE or SSN IE indicating a WPA2-PSK/WPA-PSK authentication method in
            the assication request.  <<from page52 in Wi-Fi Simple Config Specification version 1.0g>>
        */
        if (pEntry &&
            (pEntry->func_tb_idx == MAIN_MBSSID) &&
            (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.WscConfMode != WSC_DISABLE) &&
            ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)) &&
			pEntry->bWscCapable)
        {
            DBGPRINT(RT_DEBUG_TRACE, ("WPS enabled AP: Ignore EAPOL-Start frames received from clients.\n"));
            return;
        }
#endif /* WSC_AP_SUPPORT */
    }

    if (pEntry)
    {
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		DBGPRINT(RT_DEBUG_TRACE, (" PortSecured(%d), WpaState(%d), AuthMode(%d), PMKID_CacheIdx(%d) \n",
					tr_entry->PortSecured, pEntry->WpaState, pEntry->AuthMode, pEntry->PMKID_CacheIdx));

        if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (pEntry->WpaState < AS_PTKSTART)
		&& ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef DOT11_SAE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
		|| ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))))
        {
            pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
            pEntry->WpaState = AS_INITPSK;
            tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
            NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
            pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;

            WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
        }
    }
}

/*
    ==========================================================================
    Description:
        This is state machine function.
        When receiving EAPOL packets which is  for 802.1x key management.
        Use both in WPA, and WPAPSK case.
        In this function, further dispatch to different functions according to the received packet.  3 categories are :
          1.  normal 4-way pairwisekey and 2-way groupkey handshake
          2.  MIC error (Countermeasures attack)  report packet from STA.
          3.  Request for pairwise/group key update from STA
    Return:
    ==========================================================================
*/

VOID WpaEAPOLTimeout(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY	*pEntry = NULL;

	pEntry = MacTableLookup(pAd, Elem->Msg);
	if ((pEntry) && IS_ENTRY_CLIENT(pEntry)) {
		PRTMP_ADAPTER pAd = NULL;

		pAd = (PRTMP_ADAPTER)pEntry->pAd;
		pEntry->ReTryCounter++;

		DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec---> ReTryCounter=%d, WpaState=%d\n",
			pEntry->ReTryCounter, pEntry->WpaState));
		switch (pEntry->AuthMode) {
		case Ndis802_11AuthModeWPA:
		case Ndis802_11AuthModeWPAPSK:
		case Ndis802_11AuthModeWPA2:
		case Ndis802_11AuthModeWPA2PSK:
		/* 1. GTK already retried, give up and disconnect client. */
		if (pEntry->ReTryCounter > (GROUP_MSG1_RETRY_TIMER_CTR + 3)) {
			/* send wireless event - for group key handshaking timeout */
			RTMPSendWirelessEvent(pAd, IW_GROUP_HS_TIMEOUT_EVENT_FLAG,
			pEntry->Addr, pEntry->wdev->wdev_idx, 0);

			DBGPRINT(RT_DEBUG_TRACE,
			("WPARetryExec::Group Key HS exceed retry count,Disassociate client, pEntry->ReTryCounter %d\n",
			pEntry->ReTryCounter));
			MlmeDeAuthAction(pAd, pEntry, REASON_GROUP_KEY_HS_TIMEOUT, FALSE);
		}
		/* 2. Retry GTK. */
		else if (pEntry->ReTryCounter > GROUP_MSG1_RETRY_TIMER_CTR) {

			DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::ReTry 2-way group-key Handshake\n"));
			if (pEntry->GTKState == REKEY_NEGOTIATING)
				WPAStart2WayGroupHS(pAd, pEntry);
		}
		/* 3. 4-way message 3 retried more than three times. Disconnect client */
		else if (pEntry->ReTryCounter > (PEER_MSG3_RETRY_TIMER_CTR + 3)) {
			/* send wireless event - for pairwise key handshaking timeout */
			RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr,
			pEntry->wdev->wdev_idx, 0);

			DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::MSG3 timeout, pEntry->ReTryCounter = %d\n",
				pEntry->ReTryCounter));
			MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);

		}
		/* 4. Retry 4 way message 3 */
		else if (pEntry->ReTryCounter >= PEER_MSG3_RETRY_TIMER_CTR) {
			DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::ReTry MSG3 of 4-way Handshake\n"));
			WPAPairMsg3Retry(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
		}
		/* 5. 4-way message 1 retried more than three times. Disconnect client */
		else if (pEntry->ReTryCounter > (PEER_MSG1_RETRY_TIMER_CTR + 3)) {
			/* send wireless event - for pairwise key handshaking timeout */
			RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr,
			pEntry->wdev->wdev_idx, 0);

			DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::MSG1 timeout, pEntry->ReTryCounter = %d\n",
				pEntry->ReTryCounter));
			MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
		}
		/* 6. Retry 4 way message 1, the last try, the timeout is 3 sec for EAPOL-Start */
		else if (pEntry->ReTryCounter == (PEER_MSG1_RETRY_TIMER_CTR + 3)) {
			DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::Retry MSG1, the last try\n"));
			WPAStart4WayHS(pAd, pEntry, PEER_MSG3_RETRY_EXEC_INTV);
		}
		/* 7. Retry 4 way message 1 */
		else if (pEntry->ReTryCounter < (PEER_MSG1_RETRY_TIMER_CTR + 3)) {
			if ((pEntry->WpaState == AS_PTKSTART) || (pEntry->WpaState == AS_INITPSK) ||
				(pEntry->WpaState == AS_INITPMK)) {
				DBGPRINT(RT_DEBUG_TRACE, ("WPARetryExec::ReTry MSG1 of 4-way Handshake\n"));
				WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
			}
		}
		break;

		default:
		break;

		}
	}
#ifdef APCLI_SUPPORT
	else if ((pEntry) && IS_ENTRY_APCLI(pEntry)) {
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA || pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) {
			PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

			if (pEntry->func_tb_idx < MAX_APCLI_NUM) {
				UCHAR ifIndex = pEntry->func_tb_idx;

				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n",
					__func__, ifIndex));
#ifdef MAC_REPEATER_SUPPORT
				if ((pEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn == TRUE))
					ifIndex = (64 + ifIndex*MAX_EXT_MAC_ADDR_SIZE + pEntry->MatchReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (pEntry->bReptCli))
					RTMP_MLME_HANDLER(pAd);
#endif /* MAC_REPEATER_SUPPORT */
			}
		}
	}
#endif /* APCLI_SUPPORT */
}

VOID WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd,
    IN MLME_QUEUE_ELEM  *Elem)
{
	MAC_TABLE_ENTRY     *pEntry;
	STA_TR_ENTRY *tr_entry;
	PHEADER_802_11      pHeader;
	PEAPOL_PACKET       pEapol_packet;
	KEY_INFO			peerKeyInfo;
	UINT				eapol_len;
#ifdef MAC_REPEATER_SUPPORT
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;


	DBGPRINT(RT_DEBUG_INFO, ("WpaEAPOLKeyAction ===>\n"));

	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	pEapol_packet = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	eapol_len = CONV_ARRARY_TO_UINT16(pEapol_packet->Body_Len) + LENGTH_EAPOL_H;

	NdisZeroMemory((PUCHAR)&peerKeyInfo, sizeof(peerKeyInfo));
	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));


	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
	}
#endif /* MAC_REPEATER_SUPPORT */

	do
	{
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF) {
			if(!VALID_WCID(pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID)) {
				DBGPRINT(RT_DEBUG_ERROR, ("Error: Invalid WCID=%d\n",
					pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID));
				break;
			}
			pEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID];
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
			pEntry = MacTableLookup(pAd, pHeader->Addr2);

		if (!pEntry || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
			break;

		if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
			break;

		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

		DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPoL-Key frame from STA %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pEntry->Addr)));

		if (eapol_len > Elem->MsgLen - hdr_len - LENGTH_802_1_H)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("The length of EAPoL packet is invalid \n"));
			break;
		}

		if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER2)) ||
			((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Key descripter does not match with WPA rule\n"));
			break;
		}

#ifdef DOT11R_FT_SUPPORT
		/* The value 3 shall be used for all EAPOL-Key frames to and from a STA when the negotiated */
		/* AKM is 00-0F-AC:3 or 00-0F-AC:4. It is a FT STA.		*/
		if (IS_FT_RSN_STA(pEntry))
		{
			if (peerKeyInfo.KeyDescVer != KEY_DESC_EXT)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version(%d) not match(FT) \n", peerKeyInfo.KeyDescVer));
				break;
			}
		}
		else
#endif /* DOT11R_FT_SUPPORT */
		/* The value 1 shall be used for all EAPOL-Key frames to and from a STA when */
		/* neither the group nor pairwise ciphers are CCMP for Key Descriptor 1.*/
		if ((pEntry->WepStatus == Ndis802_11TKIPEnable) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(TKIP) \n"));
			break;
		}
#ifdef DOT11W_PMF_SUPPORT
		else if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_USE_SHA256) &&
			(peerKeyInfo.KeyDescVer != KEY_DESC_EXT
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			&& peerKeyInfo.KeyDescVer != KEY_DESC_NOT_DEFINED
#endif
			)) {
			DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES-128/NOT_DEFINED)\n"));
		}
#endif  /* DOT11W_PMF_SUPPORT */
		/* The value 2 shall be used for all EAPOL-Key frames to and from a STA when */
		/* either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2 or 3.*/
		else if ((pEntry->WepStatus == Ndis802_11AESEnable)
				&& (peerKeyInfo.KeyDescVer != KEY_DESC_AES)
				&& (peerKeyInfo.KeyDescVer != KEY_DESC_EXT)
#ifdef CONFIG_HOTSPOT_R2                         
                        && (peerKeyInfo.KeyDescVer != KEY_DESC_OSEN)
#endif
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			&& (peerKeyInfo.KeyDescVer != KEY_DESC_NOT_DEFINED)
#endif
                        )
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES) pEntry->WepStatus=%d, peerKeyInfo.KeyDescVer=%d\n", pEntry->WepStatus, peerKeyInfo.KeyDescVer));
			break;
		}

		/* Check if this STA is in class 3 state and the WPA state is started 						*/
		if ((pEntry->Sst == SST_ASSOC) && (pEntry->WpaState >= AS_INITPSK))
		{
			/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator */
			/* or not.*/
			/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL-*/
			/* Key frame from the Authenticator must not have the Ack bit set.*/
			if (peerKeyInfo.KeyAck == 1)
			{
				/* The frame is snet by Authenticator. */
				/* So the Supplicant side shall handle this.*/

				if ((peerKeyInfo.Secure == 0) && (peerKeyInfo.Request == 0) &&
					(peerKeyInfo.Error == 0) && (peerKeyInfo.KeyType == PAIRWISEKEY))
				{
					/*
						Process
							1. the message 1 of 4-way HS in WPA or WPA2
							EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)
							2. the message 3 of 4-way HS in WPA
							EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
					*/
					if (peerKeyInfo.KeyMic == 0)
						PeerPairMsg1Action(pAd, pEntry, Elem);
					else
						PeerPairMsg3Action(pAd, pEntry, Elem);
				}
				else if ((peerKeyInfo.Secure == 1) &&
					(peerKeyInfo.KeyMic == 1) &&
					(peerKeyInfo.Request == 0) &&
					(peerKeyInfo.Error == 0))
				{
					/*
						Process
						1. the message 3 of 4-way HS in WPA2
							EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
						2. the message 1 of group KS in WPA or WPA2
							EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N])
					*/
					if (peerKeyInfo.KeyType == PAIRWISEKEY)
						PeerPairMsg3Action(pAd, pEntry, Elem);
					else
						PeerGroupMsg1Action(pAd, pEntry, Elem);
				}
			}
			else
			{
				/*
				The frame is snet by Supplicant.So the Authenticator
				side shall handle this.
				*/
#ifdef CONFIG_AP_SUPPORT
				if ((peerKeyInfo.KeyMic == 1) &&
					(peerKeyInfo.Request == 1) &&
					(peerKeyInfo.Error == 1))
				{
					/* The Supplicant uses a single Michael MIC Failure Report frame */
					/* to report a MIC failure event to the Authenticator. */
					/* A Michael MIC Failure Report is an EAPOL-Key frame with */
					/* the following Key Information field bits set to 1: */
					/* MIC bit, Error bit, Request bit, Secure bit.*/

					DBGPRINT(RT_DEBUG_ERROR, ("Received an Michael MIC Failure Report, active countermeasure \n"));
					RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
				}
				else
#endif /* CONFIG_AP_SUPPORT */
				if ((peerKeyInfo.Request == 0) &&
					(peerKeyInfo.Error == 0) &&
					(peerKeyInfo.KeyMic == 1))
				{
					if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/*
							EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data) Process:
								1. message 2 of 4-way HS in WPA or WPA2
								2. message 4 of 4-way HS in WPA
						*/
						if (CONV_ARRARY_TO_UINT16(pEapol_packet->KeyDesc.KeyDataLen) == 0)
							PeerPairMsg4Action(pAd, pEntry, Elem);
						else
							PeerPairMsg2Action(pAd, pEntry, Elem);
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0) */
						/* Process message 4 of 4-way HS in WPA2*/
						PeerPairMsg4Action(pAd, pEntry, Elem);
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY)
					{
						/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0)*/
						/* Process message 2 of Group key HS in WPA or WPA2 */
						PeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[hdr_len],
											(Elem->MsgLen - hdr_len));
					}
				}
#ifdef CONFIG_AP_SUPPORT
				else if ((peerKeyInfo.Request == 1) && (peerKeyInfo.Error == 0))
				{
					INT		i;
					UCHAR 	apidx = pEntry->func_tb_idx;

					/* Need to check KeyType for groupkey or pairwise key update, refer to 8021i P.114, */
					if (peerKeyInfo.KeyType == GROUPKEY)
					{
						UINT8 Wcid;
						BSS_STRUCT *mbss;
						struct wifi_dev *wdev;
						DBGPRINT(RT_DEBUG_TRACE, ("REQUEST=1, ERROR=0, update group key\n"));

						mbss = &pAd->ApCfg.MBSSID[apidx];
						wdev = &mbss->wdev;

						GenRandom(pAd, wdev->bssid, mbss->GNonce);
						wdev->DefaultKeyId = (wdev->DefaultKeyId == 1) ? 2 : 1;
						WpaDeriveGTK(mbss->GMK, mbss->GNonce,
									wdev->bssid, mbss->GTK, LEN_TKIP_GTK);

						for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
						{
							if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i])
								&& (pAd->MacTab.Content[i].WpaState == AS_PTKINITDONE)
								&& (pAd->MacTab.Content[i].func_tb_idx== apidx))
							{
								pAd->MacTab.Content[i].GTKState = REKEY_NEGOTIATING;
								WPAStart2WayGroupHS(pAd, &pAd->MacTab.Content[i]);
								pAd->MacTab.Content[i].ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
								/* retry timer is set inside WPAStart2WayGroupHS */
								//RTMPModTimer(&pAd->MacTab.Content[i].RetryTimer, PEER_MSG3_RETRY_EXEC_INTV); 
								
							}
						}

						/* Get a specific WCID to record this MBSS key attribute */
						GET_GroupKey_WCID(pAd, Wcid, apidx);

						WPAInstallSharedKey(pAd, wdev->GroupKeyWepStatus,
										apidx, wdev->DefaultKeyId, Wcid,
										TRUE, mbss->GTK, LEN_TKIP_GTK);
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("REQUEST=1, ERROR= 0, update pairwise key\n"));

						NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));

						/* clear this entry as no-security mode*/
						AsicRemovePairwiseKeyEntry(pAd, pEntry->wcid);

#ifdef MT_MAC
						if (pAd->chipCap.hif_type == HIF_MT)
							CmdProcAddRemoveKey(pAd, 1, pEntry->func_tb_idx, 0, pEntry->wcid, PAIRWISEKEYTABLE, &pEntry->PairwiseKey, pEntry->Addr);
#endif

						pEntry->Sst = SST_ASSOC;
						if (pEntry->AuthMode == Ndis802_11AuthModeWPA || pEntry->AuthMode == Ndis802_11AuthModeWPA2)
							pEntry->WpaState = AS_INITPMK;
						else if (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK ||
							pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK
#ifdef DOT11_SAE_SUPPORT
							|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
							|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
							)
							pEntry->WpaState = AS_INITPSK;

						pEntry->GTKState = REKEY_NEGOTIATING;
						pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
						pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
						tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
						NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));

						WPAStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
					}
				}
#endif /* CONFIG_AP_SUPPORT */
			}
		}
	}while(FALSE);
}

/*
	========================================================================

	Routine	Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware encryption before really
	sent out to air.

	Arguments:
		pAd		Pointer	to our adapter
		PNDIS_PACKET	Pointer to outgoing Ndis frame
		NumberOfFrag	Number of fragment required

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPToWirelessSta(
    IN PRTMP_ADAPTER pAd,
    IN PMAC_TABLE_ENTRY pEntry,
    IN PUCHAR pHeader802_3,
    IN UINT HdrLen,
    IN PUCHAR pData,
    IN UINT DataLen,
    IN BOOLEAN bClearFrame)
{
    PNDIS_PACKET pPacket;
    NDIS_STATUS Status;
	struct wifi_dev *wdev;

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		 && (!IS_ENTRY_TDLS(pEntry))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	))
		return;
	

	Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);
	if (Status != NDIS_STATUS_SUCCESS)
		return;


#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	
	if (bClearFrame == 0x80)
		RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, bClearFrame);
	else
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */
	RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, (bClearFrame ? 1 : 0));

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	if (IS_ENTRY_APCLI(pEntry))
	{
	}
	else
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{
	}

	RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->wcid);
	// TODO: shiang-usw, fix this!
	RTMP_SET_PACKET_WDEV(pPacket, pEntry->wdev->wdev_idx);
	RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

	/* send out the packet */
	wdev = pEntry->wdev;
	if (wdev && wdev->tx_pkt_handle)
		wdev->tx_pkt_handle(pAd, pPacket);
	else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid wdev(%p) or tx_pkt_handle(%p)!\n",
					__FUNCTION__, wdev, (wdev ? wdev->tx_pkt_handle : NULL)));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}

	if (!RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS |
									fRTMP_ADAPTER_RESET_IN_PROGRESS)))
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);

}

/*
    ==========================================================================
    Description:
        Check the validity of the received EAPoL frame
    Return:
        TRUE if all parameters are OK,
        FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerWpaMessageSanity(
    IN 	PRTMP_ADAPTER 		pAd,
    IN 	PEAPOL_PACKET 		pMsg,
    IN 	ULONG 				MsgLen,
    IN 	UCHAR				MsgType,
    IN 	MAC_TABLE_ENTRY  	*pEntry)
{
	UCHAR			mic[LEN_KEY_DESC_MIC], digest[80]; /*, KEYDATA[MAX_LEN_OF_RSNIE];*/
	UCHAR			*KEYDATA = NULL;
	BOOLEAN			bReplayDiff = FALSE;
	BOOLEAN			bWPA2 = FALSE;
	KEY_INFO		EapolKeyInfo;
	UCHAR			GroupKeyIndex = 0;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&KEYDATA, MAX_LEN_OF_RSNIE);
	if (KEYDATA == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return FALSE;
	}

	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, MAX_LEN_OF_RSNIE);
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));

	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
	(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef DOT11_SAE_SUPPORT
	|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
	|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
	)
		bWPA2 = TRUE;

	/* 0. Check MsgType*/
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("The message type is invalid(%d)! \n", MsgType));
		goto LabelErr;
	}

	/* 1. Replay counter check	*/
 	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)	/* For supplicant*/
    {
    	/* First validate replay counter, only accept message with larger replay counter.*/
		/* Let equal pass, some AP start with all zero replay counter*/
		UCHAR	ZeroReplay[LEN_KEY_DESC_REPLAY];

        NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
    	{
			bReplayDiff = TRUE;
    	}
 	}
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2)	/* For authenticator*/
	{
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard*/
    	if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY))
    	{
			bReplayDiff = TRUE;
    	}
	}

	/* Replay Counter different condition*/
	if (bReplayDiff)
	{
		/* send wireless event - for replay counter different*/
			RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

		if (MsgType < EAPOL_GROUP_MSG_1)
		{
           	DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
		}

		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
        goto LabelErr;
	}

	/* 2. Verify MIC except Pairwise Msg1*/
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		UCHAR			rcvd_mic[LEN_KEY_DESC_MIC];
		UINT			eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;

		/* Record the received MIC for check later*/
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
		NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);

        if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP)	/* TKIP*/
        {
            RT_HMAC_MD5(pEntry->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)	/* AES        */
        {
            RT_HMAC_SHA1(pEntry->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
            NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)	/* AES-128 */
        {
                        UINT mlen = AES_KEY128_LENGTH;
                        AES_CMAC((PUCHAR)pMsg, eapol_len, pEntry->PTK, LEN_PTK_KCK, mic, &mlen);
	}
#ifdef CONFIG_OWE_SUPPORT
		else if ((pEntry->AuthMode == Ndis802_11AuthModeOWE) &&
			(pEntry->key_deri_alg == SEC_KEY_DERI_SHA256)) {
			RT_HMAC_SHA256(pEntry->PTK, LEN_PTK_KEK,
					(PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC);
		} else if ((pEntry->AuthMode == Ndis802_11AuthModeOWE) &&
			(pEntry->key_deri_alg == SEC_KEY_DERI_SHA384)) {
			RT_HMAC_SHA384(pEntry->PTK,
					LEN_PTK_KCK_SHA384,
					(PUCHAR)pMsg,
					eapol_len,
					mic,
					LEN_KEY_DESC_MIC_SHA384);
		}
#endif
		else if (EapolKeyInfo.KeyDescVer == KEY_DESC_OSEN
#ifdef DOT11_SAE_SUPPORT
			|| EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED
#endif
			) {/* AES-128 */

			UINT mlen = AES_KEY128_LENGTH;

			AES_CMAC((PUCHAR)pMsg, eapol_len, pEntry->PTK, LEN_PTK_KCK, mic, &mlen);

			{
				unsigned char *tmp = (unsigned char *)pEntry->PTK;
				int k=0;
				printk("PTK=>");
				for(k=0;k<32;k++)
					printk("%02x", *(tmp+k));
				printk("\n");
			}
		}
        if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC))
        {
			/* send wireless event - for MIC different*/
				RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

			if (MsgType < EAPOL_GROUP_MSG_1)
			{
            	DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in pairwise msg %d of 4-way handshake!\n", MsgType));
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MIC Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
			}

			hex_dump("Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump("Desired  MIC", mic, LEN_KEY_DESC_MIC);

			goto LabelErr;
        }
	}

	/* 1. Decrypt the Key Data field if GTK is included.*/
	/* 2. Extract the context of the Key Data field if it exist.	 */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.*/
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.*/
	if (CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen) > 0)
	{
		/* Decrypt this field		*/
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		{
			if ((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT) || (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
				|| (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED)
#endif
			) {
				UINT aes_unwrap_len = 0;

				/* AES */
				AES_Key_Unwrap(pMsg->KeyDesc.KeyData,
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
							   &pEntry->PTK[LEN_PTK_KCK], LEN_PTK_KEK,
							   KEYDATA, &aes_unwrap_len);
				SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, aes_unwrap_len);
			}
			else
			{
				TKIP_GTK_KEY_UNWRAP(&pEntry->PTK[LEN_PTK_KCK],
									pMsg->KeyDesc.KeyIv,
									pMsg->KeyDesc.KeyData,
									CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
									KEYDATA);
			}

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;

		}
		else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2))
		{
			NdisMoveMemory(KEYDATA, pMsg->KeyDesc.KeyData, CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen));
		}
		else
		{

			goto LabelOK;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)*/
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2*/
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)*/
		if (!RTMPParseEapolKeyData(pAd, KEYDATA,
								  CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
		{
			goto LabelErr;
		}
	}

LabelOK:
	if (KEYDATA != NULL)
		os_free_mem(NULL, KEYDATA);
	return TRUE;

LabelErr:
	if (KEYDATA != NULL)
		os_free_mem(NULL, KEYDATA);
	return FALSE;
}


/*
    ==========================================================================
    Description:
        This is a function to initilize 4-way handshake

    Return:

    ==========================================================================
*/
VOID WPAStart4WayHS(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN ULONG			TimeInterval)
{
    UCHAR           Header802_3[14];
	UCHAR   		*mpool;
    PEAPOL_PACKET	pEapolFrame;
	PUINT8			pBssid = NULL;
	UCHAR			group_cipher = Ndis802_11WEPDisabled;
	STA_TR_ENTRY *tr_entry;
	//struct wifi_dev *wdev;
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *pMbss;
#endif /* CONFIG_AP_SUPPORT */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
	UINT8 pmk_len;
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("===> WPAStart4WayHS\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS |
							fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : The interface is closed...\n"));
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	        if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
	        {
			DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : The entry doesn't exist.\n"));
			return;
	        }

		pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;
		//wdev = pEntry->wdev;
		if (pEntry->wdev != &pMbss->wdev)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : cannot get binding wdev(%p).\n", pEntry->wdev));
			return;
        	}

		/* pointer to the corresponding position*/
		pBssid = pMbss->wdev.bssid;
		group_cipher = pMbss->wdev.GroupKeyWepStatus;
	}
#endif /* CONFIG_AP_SUPPORT */
	
	if(pEntry) {
		if(!pEntry->AuthAssocNotInProgressFlag) {
		    DBGPRINT(RT_DEBUG_TRACE,("===[%s] in if pEntry->AuthAssocNotInProgressFlag = %u as auth/assoc in progress thus not initiating 4way HS \n", 
			__FUNCTION__,  pEntry->AuthAssocNotInProgressFlag));
		    return;
		} else {
		   DBGPRINT(RT_DEBUG_TRACE,("===[%s] in else pEntry->AuthAssocNotInProgressFlag = %u \n",      
                      __FUNCTION__,  pEntry->AuthAssocNotInProgressFlag));
		}
	}
	
	if (pBssid == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : No corresponding Authenticator.\n"));
		return;
    }

	/* Check the status*/
    if ((pEntry->WpaState > AS_PTKSTART) || (pEntry->WpaState < AS_INITPMK))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : Not expect calling\n"));
        return;
    }

#ifdef WSC_AP_SUPPORT
    if (MAC_ADDR_EQUAL(pEntry->Addr, pMbss->WscControl.EntryAddr) &&
        pMbss->WscControl.EapMsgRunning)
    {
        pEntry->WpaState = AS_NOTUSE;
        DBGPRINT(RT_DEBUG_ERROR, ("This is a WSC-Enrollee. Not expect calling WPAStart4WayHS here \n"));
        return;
    }
#endif /* WSC_AP_SUPPORT */

	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

	/* Randomly generate ANonce */
	GenRandom(pAd, (UCHAR *)pBssid, pEntry->ANonce);

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 1*/
	/* EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)		*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_1,
					  0,					/* Default key index*/
					  pEntry->ANonce,
					  NULL,					/* TxRSC*/
					  NULL,					/* GTK*/
					  NULL,					/* RSNIE*/
					  0,					/* RSNIE length	*/
					  pEapolFrame);

#ifdef CONFIG_AP_SUPPORT
	/* If PMKID match in WPA2-enterprise mode, fill PMKID into Key data field and update PMK here	*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2
#ifdef DOT11_SAE_SUPPORT
		|| pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK
#endif
#ifdef CONFIG_OWE_SUPPORT
		|| pEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
		) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))
	{
			/* Fill in value for KDE */
		pEapolFrame->KeyDesc.KeyData[0] = 0xDD;
		pEapolFrame->KeyDesc.KeyData[2] = 0x00;
		pEapolFrame->KeyDesc.KeyData[3] = 0x0F;
		pEapolFrame->KeyDesc.KeyData[4] = 0xAC;
		pEapolFrame->KeyDesc.KeyData[5] = 0x04;

		NdisMoveMemory(&pEapolFrame->KeyDesc.KeyData[6], &pMbss->PMKIDCache.BSSIDInfo[pEntry->PMKID_CacheIdx].PMKID, LEN_PMKID);
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
		pmk_len = LEN_PMK;
#ifdef CONFIG_OWE_SUPPORT
		if (pEntry->key_deri_alg == SEC_KEY_DERI_SHA384)
			pmk_len = LEN_PMK_SHA384;
#endif
		NdisMoveMemory(&pEntry->PMK, &pMbss->PMKIDCache.BSSIDInfo[pEntry->PMKID_CacheIdx].PMK, pmk_len);
#else
		NdisMoveMemory(&pMbss->PMK, &pMbss->PMKIDCache.BSSIDInfo[pEntry->PMKID_CacheIdx].PMK, PMK_LEN);
#endif
		pEapolFrame->KeyDesc.KeyData[1] = 0x14;/* 4+LEN_PMKID*/
		INC_UINT16_TO_ARRARY(pEapolFrame->KeyDesc.KeyDataLen, 6 + LEN_PMKID);
		INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}
#ifdef DOT11W_PMF_SUPPORT
	else if ((NULL
#ifdef DOT11_SAE_SUPPORT
		|| pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK
#endif
#ifdef CONFIG_OWE_SUPPORT
		|| pEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
		)
					&& (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
        {
                UCHAR digest[80], PMK_key[20];
                PKEY_DESCRIPTER  pKeyDesc = &pEapolFrame->KeyDesc;

                pKeyDesc->KeyData[0] = 0xDD;
                pKeyDesc->KeyData[2] = 0x00;
                pKeyDesc->KeyData[3] = 0x0F;
                pKeyDesc->KeyData[4] = 0xAC;
                pKeyDesc->KeyData[5] = 0x04;

                NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
                NdisMoveMemory(&PMK_key[8], pMbss->wdev.bssid, MAC_ADDR_LEN);
                NdisMoveMemory(&PMK_key[14], pEntry->Addr, MAC_ADDR_LEN);
#ifdef DOT11_SAE_SUPPORT
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE, ("%s:[SAE]pmkid not found\n", __func__));
			if (!sae_get_pmk_cache(&pAd->SaeCfg, pMbss->wdev.bssid, pEntry->Addr, digest, NULL)) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, ("%s: derive pmkid fail\n", __func__));
				return;
			}
		}
#endif

#ifdef CONFIG_OWE_SUPPORT
		if (pEntry->AuthMode == Ndis802_11AuthModeOWE) {
			OWE_INFO *owe = &pEntry->owe;
		NdisMoveMemory(&pKeyDesc->KeyData[6], owe->pmkid, LEN_PMKID);
		} else
#endif /*CONFIG_OWE_SUPPORT*/
                NdisMoveMemory(&pKeyDesc->KeyData[6], digest, LEN_PMKID);

                pKeyDesc->KeyData[1] = 0x14;// 4+LEN_PMKID
                INC_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, 6 + LEN_PMKID);
                INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/* Make outgoing frame*/
    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
    RTMPToWirelessSta(pAd, pEntry, Header802_3,
					  LENGTH_802_3, (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4,
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	/* Trigger Retry Timer*/
    RTMPModTimer(&pEntry->RetryTimer, TimeInterval);

	/* Update State*/
	pEntry->WpaState = AS_PTKSTART;

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== WPAStart4WayHS: send Msg1 of 4-way \n"));

}

/*
	========================================================================

	Routine Description:
		Process Pairwise key Msg-1 of 4-way handshaking and send Msg-2

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body

	Return Value:
		None

	Note:

	========================================================================
*/
VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	UCHAR				PTK[80];
	UCHAR               Header802_3[14];
	PEAPOL_PACKET		pMsg1;
	UINT            	MsgLen;
	UCHAR   			*mpool;
    PEAPOL_PACKET		pEapolFrame;
	PUINT8				pCurrentAddr = NULL;
	PUINT8				pmk_ptr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;
#ifdef MAC_REPEATER_SUPPORT
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;
#ifdef A4_CONN
	PHEADER_802_11 pHeader;
#endif
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT) || \
	(defined(DOT11R_FT_SUPPORT) && defined(CONFIG_STA_SUPPORT))
	UINT8 ptk_len = LEN_AES_PTK;
#endif
	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg1Action \n"));

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
		return;

#ifdef A4_CONN
	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
        return;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			UINT IfIndex = 0;

			IfIndex = pEntry->func_tb_idx;
#ifdef MAC_REPEATER_SUPPORT
			if (ifIndex >= 64)
				CliIdx = ((ifIndex - 64) % 16);
#endif /* MAC_REPEATER_SUPPORT */

			if (IfIndex >= MAX_APCLI_NUM)
				return;

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].RepeaterCli[CliIdx].CurrentAddress;
			else
#endif /* MAC_REPEATER_SUPPORT */
			pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].wdev.if_addr;


#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			if (is_pmkid_cache_in_entry(pEntry)) {

				if (pEntry->AuthMode == Ndis802_11AuthModeOWE
					&& pEntry->key_deri_alg == SEC_KEY_DERI_SHA256) {
					if (pEntry->owe.last_try_group == pEntry->owe.curr_group)
						NdisMoveMemory(pEntry->PMK, pEntry->pmk_cache, LEN_PMK);
					else {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("===> %s:PMK Cache not valid as owe group changed from %d to %d\n",
							__func__, pEntry->owe.last_try_group,
							pEntry->owe.curr_group));
					return;
					}
				} else if (pEntry->AuthMode == Ndis802_11AuthModeOWE
					&& pEntry->key_deri_alg == SEC_KEY_DERI_SHA384) {
					if (pEntry->owe.last_try_group == pEntry->owe.curr_group)
						NdisMoveMemory(pEntry->PMK, pEntry->pmk_cache, LEN_PMK_SHA384);
					else {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("===> %s:PMK Cache not valid as owe group changed from %d to %d\n",
							__func__, pEntry->owe.last_try_group,
							pEntry->owe.curr_group));
					return;
					}
				} else {
					NdisMoveMemory(pEntry->PMK, pEntry->pmk_cache, LEN_PMK);
				}
			}
#endif
#ifdef APCLI_OWE_SUPPORT
			else if (pEntry->need_process_ecdh_ie == TRUE) {
				process_ecdh_element(pAd,
							pEntry,
							(EXT_ECDH_PARAMETER_IE *)&pEntry->ecdh_ie,
							pEntry->ecdh_ie.length,
							SUBTYPE_ASSOC_RSP, FALSE);
			}
#endif



#ifdef APCLI_OWE_SUPPORT
			if (pEntry->AuthMode == Ndis802_11AuthModeOWE
					&& pEntry->key_deri_alg == SEC_KEY_DERI_SHA384)
				ptk_len = LEN_OWE_PTK_SHA384;
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			if (pEntry->AuthMode == Ndis802_11AuthModeOWE
				|| pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
				pmk_ptr = pEntry->PMK;
			else
#endif
				pmk_ptr = pAd->ApCfg.ApCliTab[IfIndex].PMK;

			group_cipher = pAd->ApCfg.ApCliTab[IfIndex].GroupCipher;
			rsnie_ptr = pAd->ApCfg.ApCliTab[IfIndex].RSN_IE;
			rsnie_len = pAd->ApCfg.ApCliTab[IfIndex].RSNIE_Len;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	if (pCurrentAddr == NULL)
		return;

	/* Store the received frame*/
	pMsg1 = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 1 - Replay Counter*/
	if (PeerWpaMessageSanity(pAd, pMsg1, MsgLen, EAPOL_PAIR_MSG_1, pEntry) == FALSE)
		return;

	/* Store Replay counter, it will use to verify message 3 and construct message 2*/
	NdisMoveMemory(pEntry->R_Counter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Store ANonce*/
	NdisMoveMemory(pEntry->ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	/* Generate random SNonce*/
	GenRandom(pAd, (UCHAR *)pCurrentAddr, pEntry->SNonce);
	pEntry->AllowInsPTK = TRUE;
	pEntry->AllowUpdateRSC = FALSE;
	pEntry->LastGroupKeyId = 0;
	NdisZeroMemory(pEntry->LastGTK, MAX_LEN_GTK);

#ifdef DOT11R_FT_SUPPORT
	if (IS_FT_RSN_STA(pEntry))
	{
	}
	else
#endif /* DOT11R_FT_SUPPORT */

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)

	if (pEntry->AuthMode == Ndis802_11AuthModeOWE) {
		PMF_DerivePTK(pAd,
						(UCHAR *)pmk_ptr,
						pEntry->ANonce,
						pEntry->Addr,
						pEntry->SNonce,
						pCurrentAddr,
						PTK,
						ptk_len);
		NdisMoveMemory(pEntry->PTK, PTK, ptk_len);
		hex_dump("PTK", PTK, ptk_len);
	} else if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK) {
		PMF_DerivePTK(pAd,
						(UCHAR *)pmk_ptr,
						pEntry->ANonce,
						pEntry->Addr,
						pEntry->SNonce,
						pCurrentAddr,
						PTK,
						LEN_AES_PTK);
		NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
	} else
#endif

#ifdef DOT11W_PMF_SUPPORT
        if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_USE_SHA256)))
        {
		PMF_DerivePTK(pAd,
                        (UCHAR *)pmk_ptr,
                        pEntry->ANonce,
                        pEntry->Addr,
                        pEntry->SNonce,
                        pCurrentAddr,
                        PTK,
                        LEN_AES_PTK);   /* Must is 48 bytes */

                NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
                hex_dump("PTK", PTK, LEN_AES_PTK);
        }
        else
#endif /* DOT11W_PMF_SUPPORT */
	{
	    /* Calculate PTK(ANonce, SNonce)*/
	    WpaDerivePTK(pAd,
	    			pmk_ptr,
			     	pEntry->ANonce,
				 	pEntry->Addr,
				 	pEntry->SNonce,
				 	pCurrentAddr,
				    PTK,
				    LEN_PTK);

		/* Save key to PTK entry*/
		NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}

	/* Update WpaState*/
	pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 2*/
	/*  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2)*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_2,
					  0,				/* DefaultKeyIdx*/
					  pEntry->SNonce,
					  NULL,				/* TxRsc*/
					  NULL,				/* GTK*/
					  (UCHAR *)rsnie_ptr,
					  rsnie_len,
					  pEapolFrame);

	/* Make outgoing frame*/
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);

	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg1Action: send Msg2 of 4-way \n"));
}


/*
    ==========================================================================
    Description:
        When receiving the second packet of 4-way pairwisekey handshake.
    Return:
    ==========================================================================
*/
VOID PeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	UCHAR				PTK[80];
    BOOLEAN             Cancelled;
    //PHEADER_802_11      pHeader;
	UCHAR   			*mpool;
	PEAPOL_PACKET		pEapolFrame;
	PEAPOL_PACKET       pMsg2;
	UINT            	MsgLen;
    UCHAR               Header802_3[LENGTH_802_3];
	UCHAR 				TxTsc[6];
	PUINT8				pBssid = NULL;
	PUINT8				pmk_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;
#ifdef CONFIG_AP_SUPPORT
	UCHAR				apidx = 0;
#endif /* CONFIG_AP_SUPPORT */
	STA_TR_ENTRY *tr_entry;
#ifdef CONFIG_OWE_SUPPORT
	UINT8 len_ptk = LEN_AES_PTK;
#endif
	unsigned char hdr_len = LENGTH_802_11;
#ifdef A4_CONN
	PHEADER_802_11 pHeader;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg2Action \n"));

    if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
        return;

#ifdef A4_CONN
	/* pointer to 802.11 header*/
		pHeader = (PHEADER_802_11)Elem->Msg;

	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
        return;

    /* check Entry in valid State*/
    if (pEntry->WpaState < AS_PTKSTART)
        return;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct wifi_dev *wdev;
		if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
			return;
		else
			apidx = pEntry->func_tb_idx;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		pBssid = wdev->bssid;

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
		if ( (pEntry->AuthMode == Ndis802_11AuthModeOWE) || (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK) )
			pmk_ptr = pEntry->PMK;
		else
			pmk_ptr = pAd->ApCfg.MBSSID[apidx].PMK;
#else
		pmk_ptr = pAd->ApCfg.MBSSID[apidx].PMK;
#endif
		gtk_ptr = pAd->ApCfg.MBSSID[apidx].GTK;
		group_cipher = wdev->GroupKeyWepStatus;
		default_key = wdev->DefaultKeyId;

		/* Get Group TxTsc form Asic*/
		RTMPGetTxTscFromAsic(pAd, apidx, TxTsc);

		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
		{
			rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0];
			rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[0][0];
		}
		else
		{
			if ((wdev->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2))
			{
				rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[1];
				rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[1][0];
			}
			else
			{
#ifdef CONFIG_HOTSPOT_R2            	
            	if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.bASANEnable == 1)
            	{
            		printk("choose OSEN\n");
					rsnie_len = OSEN_IELEN;
					rsnie_ptr = OSEN_IE;            		
            	}
            	else
#endif
				{            		
					rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0];
					rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[0][0];
				}
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */


    /* pointer to 802.11 header*/
	//pHeader = (PHEADER_802_11)Elem->Msg;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pMsg2 = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Store SNonce*/
	NdisMoveMemory(pEntry->SNonce, pMsg2->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

#ifdef CONFIG_OWE_SUPPORT
	if ((pEntry->AuthMode == Ndis802_11AuthModeOWE) && (pEntry->key_deri_alg == SEC_KEY_DERI_SHA384))
		len_ptk = LEN_OWE_PTK_SHA384;
#endif
#ifdef DOT11R_FT_SUPPORT
	if (IS_FT_RSN_STA(pEntry))
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UINT8		ptk_len;

			FT_DerivePMKR0(pmk_ptr, LEN_PMK,
							 (PUINT8)pAd->ApCfg.MBSSID[apidx].Ssid,
							 pAd->ApCfg.MBSSID[apidx].SsidLen,
							 pAd->ApCfg.MBSSID[apidx].FtCfg.FtMdId,
							 pAd->ApCfg.MBSSID[apidx].FtCfg.FtR0khId,
							 pAd->ApCfg.MBSSID[apidx].FtCfg.FtR0khIdLen,
							 pEntry->Addr,
							 pEntry->FT_PMK_R0,
							 pEntry->FT_PMK_R0_NAME);

			FT_DerivePMKR1(pEntry->FT_PMK_R0,
							 pEntry->FT_PMK_R0_NAME,
							 pBssid, 	/* R1KHID*/
							 pEntry->Addr, 					/* S1KHID*/
							 pEntry->FT_PMK_R1,
							 pEntry->FT_PMK_R1_NAME);

			if (pEntry->WepStatus == Ndis802_11TKIPEnable)
				ptk_len = 32+32;
			else
				ptk_len = 32+16;

			FT_DerivePTK(pEntry->FT_PMK_R1,
						   pEntry->FT_PMK_R1_NAME,
						   pEntry->ANonce,
						   pEntry->SNonce,
						   pBssid, 				/* Bssid*/
						   pEntry->Addr, 								/* sta mac*/
						   ptk_len,
						   pEntry->PTK,
						   pEntry->PTK_NAME);

		}
#endif /* CONFIG_AP_SUPPORT */
	}
	else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK) {
		PMF_DerivePTK(pAd,
			(UCHAR *)pmk_ptr,
			pEntry->ANonce,
			(UCHAR *)pBssid,
			pEntry->SNonce,
			pEntry->Addr,
			PTK,
			LEN_AES_PTK);

			NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
			hex_dump("PTK", PTK, LEN_AES_PTK);
	} else
#endif
#ifdef CONFIG_OWE_SUPPORT
	if (pEntry->AuthMode == Ndis802_11AuthModeOWE) {
		PMF_DerivePTK(pAd,
			(UCHAR *)pmk_ptr,
			pEntry->ANonce,
			(UCHAR *)pBssid,
			pEntry->SNonce,
			pEntry->Addr,
			PTK,
			len_ptk);

		NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
		hex_dump("OWE PTK", PTK, LEN_AES_PTK);
	} else
#endif

#ifdef DOT11W_PMF_SUPPORT
        if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_USE_SHA256)))
        {
		PMF_DerivePTK(pAd,
                        (UCHAR *)pmk_ptr,
                        pEntry->ANonce,
                        (UCHAR *)pBssid,
                        pEntry->SNonce,
                        pEntry->Addr,
                        PTK,
                        LEN_AES_PTK);   /* Must is 48 bytes */

                NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
                hex_dump("PTK", PTK, LEN_AES_PTK);
        }
        else
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
	{
		printk("got msg2 derivePTK\n");
		PMF_DerivePTK(pAd,
                     (UCHAR *)pmk_ptr,
                     pEntry->ANonce,
                     (UCHAR *)pBssid,
                     pEntry->SNonce,
                     pEntry->Addr,
                     PTK,
                     LEN_AES_PTK);   /* Must is 48 bytes */
 
         NdisMoveMemory(pEntry->PTK, PTK, LEN_AES_PTK);
         hex_dump("PTK", PTK, LEN_AES_PTK);
	}
	else
#endif /* CONFIG_HOTSPOT_R2 */
	{
		/* Derive PTK*/
		if ((pmk_ptr == NULL) || (pBssid == NULL))
		{
			DBGPRINT(RT_DEBUG_ERROR,
					("%s: pmk_ptr or pBssid == NULL!\n", __FUNCTION__));
			return;
		}

		WpaDerivePTK(pAd,
					(UCHAR *)pmk_ptr,
					pEntry->ANonce, 		/* ANONCE*/
					(UCHAR *)pBssid,
					pEntry->SNonce, 		/* SNONCE*/
					pEntry->Addr,
					PTK,
					LEN_PTK);

    	NdisMoveMemory(pEntry->PTK, PTK, LEN_PTK);
	}

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pMsg2, MsgLen, EAPOL_PAIR_MSG_2, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG2_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif
		return;
	}

    do
    {
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
		UCHAR HSClientGTK[32];
#endif

		/* Allocate memory for input*/
		os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
		if (mpool == NULL)
	    {
	        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
	        return;
	    }

		pEapolFrame = (PEAPOL_PACKET)mpool;
		NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

        /* delete retry timer*/
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

		/* Change state*/
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

		/* Increment replay counter by 1*/
		ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
		if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable &&
			pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.DGAFDisable)
		{
			// Radom GTK for hotspot sation client */
			GenRandom(pAd, pEntry->Addr, HSClientGTK);
			gtk_ptr = HSClientGTK;

			DBGPRINT(RT_DEBUG_OFF, ("%s:Random unique GTK for each mobile device when dgaf disable\n", __FUNCTION__));
			hex_dump("GTK", gtk_ptr, 32);
		}
#endif

#ifdef MT_MAC
        if (pAd->chipCap.hif_type == HIF_MT)
		WPAInstallPairwiseKey(pAd, pEntry->func_tb_idx, pEntry, TRUE);
#endif
		/* Construct EAPoL message - Pairwise Msg 3*/
		ConstructEapolMsg(pEntry,
						  group_cipher,
						  EAPOL_PAIR_MSG_3,
						  default_key,
						  pEntry->ANonce,
						  TxTsc,
						  (UCHAR *)gtk_ptr,
						  (UCHAR *)rsnie_ptr,
						  rsnie_len,
						  pEapolFrame);

        /* Make outgoing frame*/
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
        RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
						  (PUCHAR)pEapolFrame,
						  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4,
						  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

        pEntry->ReTryCounter = PEER_MSG3_RETRY_TIMER_CTR;
		RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
		

		/* Update State*/
        pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

		os_free_mem(NULL, mpool);

    }while(FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg2Action: send Msg3 of 4-way \n"));
}

VOID WPAPairMsg3Retry(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN ULONG			TimeInterval
)
{
	BOOLEAN             Cancelled;
	PEAPOL_PACKET		pEapolFrame;
	UCHAR   			*mpool;
	PUINT8				pBssid = NULL;	
	PUINT8				gtk_ptr = NULL;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				rsnie_ptr = NULL;
	//PUINT8				pmk_ptr = NULL;
	UCHAR				rsnie_len = 0;
	UCHAR 				TxTsc[6];
	UCHAR               Header802_3[LENGTH_802_3];
#ifdef CONFIG_AP_SUPPORT
	UCHAR				apidx = 0;
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	UCHAR HSClientGTK[32];
#endif 
	STA_TR_ENTRY *tr_entry;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s \n",__FUNCTION__));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		struct wifi_dev *wdev;
		if (pEntry->func_tb_idx>= pAd->ApCfg.BssidNum)
			return;
		else
			apidx = pEntry->func_tb_idx;

		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		pBssid = wdev->bssid;
		//pmk_ptr = pAd->ApCfg.MBSSID[apidx].PMK;
		gtk_ptr = pAd->ApCfg.MBSSID[apidx].GTK;
		group_cipher = wdev->GroupKeyWepStatus;
		default_key = wdev->DefaultKeyId;

		/* Get Group TxTsc form Asic*/
		RTMPGetTxTscFromAsic(pAd, apidx, TxTsc);

		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
		{
			rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0];
			rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[0][0];
		}
		else
		{
			if ((wdev->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK) ||
				(wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2)) 
			{
				rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[1];
				rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[1][0];
			}
			else
			{
				rsnie_len = pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0];
				rsnie_ptr = &pAd->ApCfg.MBSSID[apidx].RSN_IE[0][0];
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
		return;
	}

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* delete retry timer*/
	RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

	/* Change state*/
	pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable &&
		pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.DGAFDisable)
	{
		// Radom GTK for hotspot sation client */
		GenRandom(pAd, pEntry->Addr, HSClientGTK);
		gtk_ptr = HSClientGTK;
		
		DBGPRINT(RT_DEBUG_OFF, ("%s:Random unique GTK for each mobile device when dgaf disable\n", __FUNCTION__));
		hex_dump("GTK", gtk_ptr, 32);
	}
#endif 
	/* Construct EAPoL message - Pairwise Msg 3*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_3,
					  default_key,
					  pEntry->ANonce,
					  TxTsc,
					  (UCHAR *)gtk_ptr,
					  (UCHAR *)rsnie_ptr,
					  rsnie_len,
					  pEapolFrame);
	
	/* Make outgoing frame*/
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);			
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
				  (PUCHAR)pEapolFrame,
				  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4,
				  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	RTMPSetTimer(&pEntry->RetryTimer, TimeInterval);

	/* Update State*/
	pEntry->WpaState = AS_PTKINIT_NEGOTIATING;

	os_free_mem(NULL, mpool);

}

/*
	========================================================================

	Routine Description:
		Process Pairwise key Msg 3 of 4-way handshaking and send Msg 4

	Arguments:
		pAd	Pointer	to our adapter
		Elem		Message body

	Return Value:
		None

	Note:

	========================================================================
*/
VOID PeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	//PHEADER_802_11		pHeader;
	UCHAR               Header802_3[14];
	UCHAR				*mpool;
	PEAPOL_PACKET		pEapolFrame;
	PEAPOL_PACKET		pMsg3;
	UINT            	MsgLen;
	PUINT8				pCurrentAddr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
#ifdef MAC_REPEATER_SUPPORT
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	STA_TR_ENTRY *tr_entry;
	UCHAR idx = 0;
	BOOLEAN bWPA2 = FALSE;
#if (defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)) || defined(MWDS)
	PAPCLI_STRUCT pApCliEntry;
#endif
	unsigned char hdr_len = LENGTH_802_11;
#ifdef A4_CONN
	PHEADER_802_11 pHeader;
#endif
	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
		|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
		)
		bWPA2 = TRUE;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg3Action \n"));

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
		return;

#ifdef A4_CONN
	pHeader = (PHEADER_802_11) Elem->Msg;

	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			UINT IfIndex = 0;

			IfIndex = pEntry->func_tb_idx;
#ifdef MAC_REPEATER_SUPPORT
			if (ifIndex >= 64)
				CliIdx = ((ifIndex - 64) % 16);
#endif /* MAC_REPEATER_SUPPORT */

			if (IfIndex >= MAX_APCLI_NUM)
				return;

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].RepeaterCli[CliIdx].CurrentAddress;
			else
#endif /* MAC_REPEATER_SUPPORT */
			pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].wdev.if_addr;
			group_cipher = pAd->ApCfg.ApCliTab[IfIndex].GroupCipher;

		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	if (pCurrentAddr == NULL)
		return;

	/* Record 802.11 header & the received EAPOL packet Msg3*/
	//pHeader	= (PHEADER_802_11) Elem->Msg;
	pMsg3 = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pMsg3, MsgLen, EAPOL_PAIR_MSG_3, pEntry) == FALSE)
		return;

	if ((pEntry->AllowInsPTK == TRUE) && bWPA2) {
		UCHAR kid = pEntry->LastGroupKeyId;

		if (unlikely(kid >= ARRAY_SIZE(pEntry->CCMP_BC_PN))) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s invalid key id %u\n", __func__, kid));
			return;
		}

		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC - 2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pMsg3->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->AllowUpdateRSC = FALSE;
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s(%d)-%d: update CCMP_BC_PN to %llu\n",
			__func__, pEntry->wcid, kid, pEntry->CCMP_BC_PN[kid]));
	}

	/* Save Replay counter, it will use construct message 4*/
	NdisMoveMemory(pEntry->R_Counter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Double check ANonce*/
	if (!NdisEqualMemory(pEntry->ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
	{
		return;
	}

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 4*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_4,
					  0,					/* group key index not used in message 4*/
					  NULL,					/* Nonce not used in message 4*/
					  NULL,					/* TxRSC not used in message 4*/
					  NULL,					/* GTK not used in message 4*/
					  NULL,					/* RSN IE not used in message 4*/
					  0,
					  pEapolFrame);

	/* Update WpaState*/
	pEntry->WpaState = AS_PTKINITDONE;
	/* Update pairwise key		*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry)) {
		if(pEntry->AllowInsPTK == TRUE) {
		 	APCliInstallPairwiseKey(pAd, pEntry);
			pEntry->AllowInsPTK = FALSE;
			pEntry->AllowUpdateRSC = TRUE;
			} else {
			DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : the M3 reinstall attack, skip install key\n",
											__func__));
			}
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* open 802.1x port control and privacy filter*/
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	if (tr_entry && (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK ||
		pEntry->AuthMode == Ndis802_11AuthModeWPA2
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		|| pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK
		|| pEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
		))
	{
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
		pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
		{
			ifIndex = ((ifIndex - 64) / 16);
			pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliConnectState = 2;
		}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Apcli auto connected:PeerPairMsg3Action() \n"));
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#if (defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)) || defined(MWDS)
			pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
#endif
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
			if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)/*For security AES case*/
				wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#if defined(MWDS) || defined(MAP_SUPPORT)
			if (pEntry && (pEntry->func_tb_idx < MAX_APCLI_NUM) && tr_entry) {
				if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
#ifdef MWDS
					MWDSAPCliPeerEnable(pAd, pApCliEntry, pEntry);
#endif
				}
			}
#endif /* defined(MWDS) || defined(MAP_SUPPORT) */
#ifdef WH_EVENT_NOTIFIER
            if (pEntry && tr_entry && 
                (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
            {
                EventHdlr pEventHdlrHook = NULL;
                pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
                if(pEventHdlrHook && pEntry->wdev)
                    pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
            }
#endif /* WH_EVENT_NOTIFIER */
#endif /* APLCI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("PeerPairMsg3Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));
	}
	else
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT
			/* Patch issue with gateway AP*/
			/* In WPA mode, AP doesn't send out message 1 of group-key HS.*/
			/* So, Supplicant shall maintain a timeout action to disconnect */
			/* this link.*/
			/* Todo - Does it need to apply to STA ?*/
			if (IS_ENTRY_APCLI(pEntry))
			 	RTMPSetTimer(&pEntry->RetryTimer, PEER_GROUP_KEY_UPDATE_INIV);
#endif /* APCLI_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* Init 802.3 header and send out*/
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3),
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, TRUE);

	os_free_mem(NULL, mpool);


#ifdef WH_EZ_SETUP
		if( (IS_EZ_SETUP_ENABLED(pEntry->wdev)) && (IS_ENTRY_APCLI(pEntry)) ){
		//! Should be done in handle pair four	
		//	EZ_UPDATE_CAPABILITY_INFO(pAd, EZ_SET_ACTION, CONNECTED, pEntry->func_tb_idx);
		//	ez_apclient_info_action_frame(pAd, pEntry->wdev);
#ifdef NEW_CONNECTION_ALGO
			/*APCLI has connected to a non-easy setup AP, allocate itself a new node number according to the MAC address of the root AP and its own aid.*/
			ez_handle_pairmsg4(pAd, pEntry);
#endif
		}
#endif /* WIDI_SUPPORT */


#ifdef APCLI_OWE_SUPPORT
		if (IS_ENTRY_APCLI(pEntry) && pEntry->AuthMode == Ndis802_11AuthModeOWE) {
			OWE_INFO *owe = &pEntry->owe;
			UINT pmk_len = LEN_PMK;
			UCHAR intf_idx = pEntry->func_tb_idx;
			UCHAR CliIdx = 0xff;

			if (pEntry->key_deri_alg == SEC_KEY_DERI_SHA384)
				pmk_len = LEN_PMK_SHA384;

#ifdef MAC_REPEATER_SUPPORT
			if (pEntry->bReptCli == TRUE) {
				intf_idx = pEntry->MatchReptCliIdx;
				CliIdx = pEntry->MatchReptCliIdx;
			}
#endif

			if (owe->pmkid) {
				apcli_add_pmkid_cache(pAd, pEntry->Addr, owe->pmkid,
					pEntry->PMK, pmk_len, intf_idx
#ifdef MAC_REPEATER_SUPPORT
					, CliIdx
#endif
					);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
					("Store PMKID for MAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(pEntry->Addr)));
				hex_dump("PMK cache ID", owe->pmkid, LEN_PMKID);
				hex_dump("PMK key", pEntry->PMK, pmk_len);
			}

		}
#endif

#ifdef APCLI_OWE_SUPPORT
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
			set_sae_instance_removable(&pAd->SaeCfg, pAd->CurrentAddress, pEntry->Addr);
#endif


	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg3Action: send Msg4 of 4-way \n"));
}

/*
    ==========================================================================
    Description:
        When receiving the last packet of 4-way pairwisekey handshake.
        Initilize 2-way groupkey handshake following.
    Return:
    ==========================================================================
*/
VOID PeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
	PEAPOL_PACKET   	pMsg4;
    //PHEADER_802_11      pHeader;
    UINT            	MsgLen;
    BOOLEAN             Cancelled;
#ifdef DBG
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
#endif /* DBG */
	STA_TR_ENTRY *tr_entry;
	unsigned char hdr_len = LENGTH_802_11;
#ifdef A4_CONN
	PHEADER_802_11 pHeader;
#endif

    DBGPRINT(RT_DEBUG_TRACE, ("===> PeerPairMsg4Action\n"));

    do
    {
        if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
            break;

#ifdef A4_CONN
		pHeader = (PHEADER_802_11)Elem->Msg;
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

		if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
			break;

        if (pEntry->WpaState < AS_PTKINIT_NEGOTIATING)
            break;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DBG
			UCHAR apidx = 0;
#endif /* DBG */
			if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
				break;
		    else
		    {
#ifdef DBG
				apidx = pEntry->func_tb_idx;
				group_cipher = pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus;
#endif /* DBG */
		    }


		}
#endif /* CONFIG_AP_SUPPORT */

        /* pointer to 802.11 header*/
        //pHeader = (PHEADER_802_11)Elem->Msg;

		/* skip 802.11_header(24-byte) and LLC_header(8) */
		pMsg4 = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
		MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

        /* Sanity Check peer Pairwise message 4 - Replay Counter, MIC*/
		if (PeerWpaMessageSanity(pAd, pMsg4, MsgLen, EAPOL_PAIR_MSG_4, pEntry) == FALSE) {
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG4_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif
			break;
		}

        /* 3. Install pairwise key */
#ifdef MT_MAC
        if (pAd->chipCap.hif_type != HIF_MT)
#endif
			WPAInstallPairwiseKey(pAd, pEntry->func_tb_idx, pEntry, TRUE);

        /* 4. upgrade state */
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
        pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
        pEntry->WpaState = AS_PTKINITDONE;
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef WSC_AP_SUPPORT
        if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.WscConfMode != WSC_DISABLE)
            WscInformFromWPA(pEntry);
#endif /* WSC_AP_SUPPORT */

		if (pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
			pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK
#ifdef DOT11_SAE_SUPPORT
			|| pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK
#endif
#ifdef CONFIG_OWE_SUPPORT
			|| pEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
		) {
			pEntry->GTKState = REKEY_ESTABLISHED;
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
			if (IS_FT_RSN_STA(pEntry))
			{
				PFT_R1HK_ENTRY pR1khEntry;
				PUINT8 	pUCipher = NULL;
				PUINT8	pAkm = NULL;
				UINT8	count;

				pUCipher = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, PAIRWISE_SUITE, &count);
				pAkm = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, AKM_SUITE, &count);

				/* Record the PMK-R0 related information */
				RTMPAddPMKIDCache(pAd,
								  pEntry->func_tb_idx,
								  pEntry->Addr,
								  pEntry->FT_PMK_R0_NAME,
								  pEntry->FT_PMK_R0
#ifdef CONFIG_OWE_SUPPORT
								, LEN_PMK
#endif
								  );

				/* Delete previous entry */
				pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);
				if (pR1khEntry != NULL)
				{
					FT_R1khEntryDelete(pAd, pR1khEntry);
				}
				/* Update R1KH table */
				if (pUCipher != NULL)
					NdisMoveMemory(pEntry->FT_UCipher, pUCipher, 4);
				if (pAkm != NULL)
					NdisMoveMemory(pEntry->FT_Akm, pAkm, 4);

				FT_R1khEntryInsert(pAd,
								   pEntry->FT_PMK_R0_NAME,
								   pEntry->FT_PMK_R1_NAME,
								   pEntry->FT_PMK_R1,
								   pUCipher,
								   pAkm,
								   (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod/OS_HZ),
								   FT_REASSOC_DEADLINE,
								   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].FtCfg.FtR0khId,
								   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].FtCfg.FtR0khIdLen,
								   pEntry->Addr);

#ifdef IAPP_SUPPORT
				{
					FT_KDP_EVT_ASSOC EvtAssoc;

					EvtAssoc.SeqNum = 0;
					NdisMoveMemory(EvtAssoc.MacAddr, pEntry->Addr, MAC_ADDR_LEN);
#ifdef WH_EZ_SETUP
					if (!IS_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev) /*&& !ez_is_triband()*/)
#endif
					{
						FT_KDP_EVENT_INFORM(pAd,
										pEntry->func_tb_idx,
										FT_KDP_SIG_FT_ASSOCIATION,
										&EvtAssoc,
										sizeof(EvtAssoc),
										NULL);
					}
				}
#endif /* IAPP_SUPPORT 				*/

				pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);
				if (pR1khEntry != NULL)
				{
					hex_dump("R1KHTab-R0KHID", pR1khEntry->R0khId, pR1khEntry->R0khIdLen);
					hex_dump("R1KHTab-PairwiseCipher", pR1khEntry->PairwisChipher, 4);
					hex_dump("R1KHTab-AKM", pR1khEntry->AkmSuite, 4);
					hex_dump("R1KHTab-PMKR0Name", pR1khEntry->PmkR0Name, 16);
					hex_dump("R1KHTab-PMKR1Name", pR1khEntry->PmkR1Name, 16);
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("The entry in R1KH table doesn't exist\n"));
				}
			}
			else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT1X_SUPPORT
			if (pEntry->AuthMode == Ndis802_11AuthModeWPA2)
        	{
        		UCHAR  PMK_key[20];
				UCHAR  digest[80];

				/* Calculate PMKID, refer to IEEE 802.11i-2004 8.5.1.2*/
				NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
				NdisMoveMemory(&PMK_key[8], pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN);
				NdisMoveMemory(&PMK_key[14], pEntry->Addr, MAC_ADDR_LEN);
				RT_HMAC_SHA1(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMK, PMK_LEN, PMK_key, 20, digest, SHA1_DIGEST_SIZE);
				RTMPAddPMKIDCache(pAd, pEntry->func_tb_idx, pEntry->Addr,
							digest, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMK
#ifdef CONFIG_OWE_SUPPORT
							, LEN_PMK
#endif
						);
				DBGPRINT(RT_DEBUG_TRACE, ("Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n", digest[0],digest[1],digest[2],digest[3],digest[4],digest[5]));
        	}
#endif /* DOT1X_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
			else if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
				set_sae_instance_removable(&pAd->SaeCfg,
								pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,
								pEntry->Addr);
#endif

#ifdef CONFIG_OWE_SUPPORT
			else if (pEntry->AuthMode == Ndis802_11AuthModeOWE) {
				INT CacheIdx;/* Key cache */

				if (pEntry->owe.pmkid) {
					UINT8 pmk_len = LEN_PMK;
					BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

					if (pEntry->key_deri_alg == SEC_KEY_DERI_SHA384)
						pmk_len = LEN_PMK_SHA384;

					CacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->func_tb_idx, pEntry->Addr);
					if ((CacheIdx == -1) ||
						((RTMPEqualMemory(pEntry->owe.pmkid,
						&pMbss->PMKIDCache.BSSIDInfo[CacheIdx].PMKID,
						pmk_len)) == 0)) {
						CacheIdx = -1;
					}
					hex_dump("store pmkid:", pEntry->owe.pmkid, LEN_PMKID);
					RTMPAddPMKIDCache(pAd,
							pEntry->func_tb_idx,
							pEntry->Addr,
							pEntry->owe.pmkid,
							pEntry->PMK,
							pmk_len);
					/* store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx); */
				}
			}
#endif /*CONFIG_OWE_SUPPORT*/


#endif /* CONFIG_AP_SUPPORT */
#if defined(MWDS) || defined(MAP_SUPPORT) || defined(WAPP_SUPPORT)
		if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
#ifdef MWDS
			MWDSAPPeerEnable(pAd, pEntry);
#endif

#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		}
#endif

#ifdef WH_EVENT_NOTIFIER
		if (pEntry && tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);
			if(pEventHdlrHook && pEntry->wdev)
				pEventHdlrHook(pAd, pEntry, Elem);
		}
#endif /* WH_EVENT_NOTIFIER */

			/* send wireless event - for set key done WPA2*/
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

#ifdef CONFIG_HOTSPOT_R2
		if (pEntry->IsWNMReqValid == TRUE)
		{
			struct wnm_req_data *req_data = pEntry->ReqData;
			Send_WNM_Notify_Req(pAd,
				     req_data->peer_mac_addr,
				     req_data->wnm_req,
				     req_data->wnm_req_len,
				     req_data->type);

			pEntry->IsWNMReqValid = FALSE;
			os_free_mem(NULL, req_data); 
			printk("!!!!msg 4 send wnm req\n");
		}
		if (pEntry->IsBTMReqValid == TRUE)
		{
			struct btm_req_data *req_data = pEntry->ReqbtmData;
			Send_BTM_Req(pAd,
				     req_data->peer_mac_addr,
				     req_data->btm_req,
				     req_data->btm_req_len);

			pEntry->IsBTMReqValid = FALSE;
			os_free_mem(NULL, req_data); 
			printk("!!!!msg 4 send btm req\n");
		}
#endif

#ifdef MBO_SUPPORT
		/* update STA bssid & security info to daemon */
		MboIndicateStaBssidInfo(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry->Addr);
#endif/* MBO_SUPPORT */
	        DBGPRINT(RT_DEBUG_TRACE, ("AP SETKEYS DONE - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
									pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
									pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
									group_cipher,
									GetEncryptType(group_cipher)));
		}
		else
		{
        	/* 5. init Group 2-way handshake if necessary.*/
	        WPAStart2WayGroupHS(pAd, pEntry);

        	pEntry->ReTryCounter = GROUP_MSG1_RETRY_TIMER_CTR;
			
        	/* retry timer is set inside WPAStart2WayGroupHS */
			//RTMPModTimer(&pEntry->RetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
			
		}
    }while(FALSE);

}

/*
    ==========================================================================
    Description:
        This is a function to send the first packet of 2-way groupkey handshake
    Return:

    ==========================================================================
*/
VOID WPAStart2WayGroupHS(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry)
{
    UCHAR               Header802_3[14];
	UCHAR   			TxTsc[6];
	UCHAR   			*mpool;
	PEAPOL_PACKET		pEapolFrame;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	UCHAR				default_key = 0;
	PUINT8				gnonce_ptr = NULL;
	PUINT8				gtk_ptr = NULL;
	PUINT8				pBssid = NULL;
    BOOLEAN				Cancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> WPAStart2WayGroupHS\n"));

    if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
        return;

	/* delete retry timer*/
	RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		UCHAR	apidx = 0;

		if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
			return;
	    else
			apidx = pEntry->func_tb_idx;

		group_cipher = pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus;
		default_key = pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId;
		gnonce_ptr = pAd->ApCfg.MBSSID[apidx].GNonce;
		gtk_ptr = pAd->ApCfg.MBSSID[apidx].GTK;
		pBssid = pAd->ApCfg.MBSSID[apidx].wdev.bssid;

		/* Get Group TxTsc form Asic*/
		RTMPGetTxTscFromAsic(pAd, apidx, TxTsc);
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

    /* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pEntry->R_Counter);

	/* Construct EAPoL message - Group Msg 1*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_GROUP_MSG_1,
					  default_key,
					  (UCHAR *)gnonce_ptr,
					  TxTsc,
					  (UCHAR *)gtk_ptr,
					  NULL,
					  0,
				  	  pEapolFrame);

	/* Make outgoing frame*/
	if (pBssid == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pBssid == NULL!\n", __FUNCTION__));
		return;
	}

    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);
    RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);

	os_free_mem(NULL, mpool);
	
	RTMPSetTimer(&pEntry->RetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
    DBGPRINT(RT_DEBUG_TRACE, ("<=== WPAStart2WayGroupHS : send out Group Message 1 \n"));

    return;
}

/*
	========================================================================

	Routine Description:
		Process Group key 2-way handshaking

	Arguments:
		pAd	Pointer	to our adapter
		Elem		Message body

	Return Value:
		None

	Note:

	========================================================================
*/
VOID	PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem)
{
    UCHAR               Header802_3[14];
	UCHAR				*mpool;
	PEAPOL_PACKET		pEapolFrame;
	PEAPOL_PACKET		pGroup;
	UINT            	MsgLen;
	UCHAR				default_key = 0;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
	PUINT8				pCurrentAddr = NULL;
#ifdef APCLI_SUPPORT
	BOOLEAN             Cancelled;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	STA_TR_ENTRY *tr_entry;
	UCHAR idx = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerGroupMsg1Action \n"));

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
        return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
		CliIdx = ((ifIndex - 64) % 16);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			UINT				IfIndex = 0;

			IfIndex = pEntry->func_tb_idx;
			if (IfIndex >= MAX_APCLI_NUM)
				return;

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].RepeaterCli[CliIdx].CurrentAddress;
			else
#endif /* MAC_REPEATER_SUPPORT */
			pCurrentAddr = pAd->ApCfg.ApCliTab[IfIndex].wdev.if_addr;
			group_cipher = pAd->ApCfg.ApCliTab[IfIndex].GroupCipher;
			default_key = pAd->ApCfg.ApCliTab[IfIndex].wdev.DefaultKeyId;

		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	if (pCurrentAddr == NULL)
		return;

	/* Process Group Message 1 frame. skip 802.11 header(24) & LLC_SNAP header(8)*/
	pGroup = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	/* Sanity Check peer group message 1 - Replay Counter, MIC, RSNIE*/
	if (PeerWpaMessageSanity(pAd, pGroup, MsgLen, EAPOL_GROUP_MSG_1, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG1_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_MIC_FAILURE);
#endif
		return;
	}

	if (pEntry->AllowUpdateRSC == TRUE) {
		UCHAR kid = pEntry->LastGroupKeyId;

		if (unlikely(kid >= ARRAY_SIZE(pEntry->CCMP_BC_PN))) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s invalid key id %u\n", __func__, kid));
			return;
		}

		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC - 2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pGroup->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->AllowUpdateRSC = FALSE;
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s(%d)-%d: update CCMP_BC_PN to %llu\n",
			__func__, pEntry->wcid, kid, pEntry->CCMP_BC_PN[kid]));
	}

	/* delete retry timer*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		/* Patch issue with gateway AP*/
		/* In WPA mode, AP doesn't send out message 1 of group-key HS.*/
		/* So, Supplicant shall maintain a timeout action to disconnect */
		/* this link.*/
		/* Todo - Does it need to apply to STA ?*/
		if (IS_ENTRY_APCLI(pEntry))
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Save Replay counter, it will use to construct message 2*/
	NdisMoveMemory(pEntry->R_Counter, pGroup->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);


	/* Construct EAPoL message - Group Msg 2*/
	ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_GROUP_MSG_2,
					  default_key,
					  NULL,					/* Nonce not used*/
					  NULL,					/* TxRSC not used*/
					  NULL,					/* GTK not used*/
					  NULL,					/* RSN IE not used*/
					  0,
					  pEapolFrame);

    /* open 802.1x port control and privacy filter*/
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		ifIndex = ((ifIndex - 64) / 16);
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliConnectState = 2;
	}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Apcli auto connected:PeerGroupMsg1Action() \n"));
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APLCI_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef WH_EVENT_NOTIFIER
		if(pEntry &&
		   IS_ENTRY_APCLI(pEntry))
		{
			STA_TR_ENTRY *tr_entry = NULL;
			EventHdlr pEventHdlrHook = NULL;
			if(VALID_TR_WCID(pEntry->wcid))
				tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			if(tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
				if(pEventHdlrHook && pEntry->wdev)
					pEventHdlrHook(pAd, pEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
			}
		}
#endif /* WH_EVENT_NOTIFIER */
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


	DBGPRINT(RT_DEBUG_TRACE, ("PeerGroupMsg1Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));

	/* init header and Fill Packet and send Msg 2 to authenticator	*/
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);


	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3),
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);

	os_free_mem(NULL, mpool);


	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerGroupMsg1Action: send group message 2\n"));
}


VOID EnqueueStartForPSKExec(
    IN PVOID SystemSpecific1,
    IN PVOID FunctionContext,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY     *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
	if(pEntry == NULL)
		return;

	if ((pEntry) && IS_ENTRY_CLIENT(pEntry) && (pEntry->WpaState < AS_PTKSTART))
	{
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

		switch (pEntry->EnqueueEapolStartTimerRunning)
		{
			case EAPOL_START_PSK:
				DBGPRINT(RT_DEBUG_TRACE, ("Enqueue EAPoL-Start-PSK for sta(%02x:%02x:%02x:%02x:%02x:%02x) \n", PRINT_MAC(pEntry->Addr)));

#ifdef EAPOL_QUEUE_SUPPORT
				EAPMlmeEnqueue(pAd, WPA_STATE_MACHINE, MT2_EAPOLStart, 6, &pEntry->Addr, 0);
#else /* EAPOL_QUEUE_SUPPORT */
				MlmeEnqueue(pAd, WPA_STATE_MACHINE, MT2_EAPOLStart, 6, &pEntry->Addr, 0);
#endif /* !EAPOL_QUEUE_SUPPORT */
				
				RTMP_MLME_HANDLER(pAd);
				break;
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT1X_SUPPORT
			case EAPOL_START_1X:
				DBGPRINT(RT_DEBUG_TRACE, ("Enqueue EAPoL-Start-1X for sta(%02x:%02x:%02x:%02x:%02x:%02x) \n", PRINT_MAC(pEntry->Addr)));

				DOT1X_EapTriggerAction(pAd, pEntry);
				break;
#endif /* DOT1X_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			default:
				break;

		}
	}
		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

}


VOID MlmeDeAuthAction(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
	IN USHORT           Reason,
	IN BOOLEAN          bDataFrameFirst)
{
    PUCHAR          pOutBuffer = NULL;
    ULONG           FrameLen = 0;
    HEADER_802_11   DeAuthHdr;
    NDIS_STATUS     NStatus;

    if (pEntry)
    {
        /* Send out a Deauthentication request frame*/
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS)
            return;

	/* send wireless event - for send disassication */
	RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

#ifdef ALL_NET_EVENT
		wext_send_event(pEntry->wdev->if_dev,
			pEntry->Addr,
			pEntry->bssid,
			pAd->CommonCfg.Channel,
			RTMPAvgRssi(pAd, &pEntry->RssiSample),
			FBT_LINK_OFFLINE_NOTIFY);
#endif /* ALL_NET_EVENT */

	DBGPRINT(RT_DEBUG_TRACE,
				("Send DEAUTH frame with ReasonCode(%d) to %02x:%02x:%02x:%02x:%02x:%02x \n",
				Reason, PRINT_MAC(pEntry->Addr)));

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
	}
#endif /* CONFIG_AP_SUPPORT */
        MakeOutgoingFrame(pOutBuffer, &FrameLen,
                          sizeof(HEADER_802_11), &DeAuthHdr,
                          2,  &Reason,
                          END_OF_ARGS);

#ifdef DOT11W_PMF_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE))
		{
			ULONG	TmpLen;
			UINT	res_len = LEN_CCMP_HDR + LEN_CCMP_MIC;
			UCHAR	res_buf[res_len];

			/* reserve a buffer for PMF CCMP calculation later */
			MakeOutgoingFrame(pOutBuffer + FrameLen,	&TmpLen,
	                          res_len,   				res_buf,
    	                      END_OF_ARGS);
			FrameLen += TmpLen;

			/* Indicate this is a unicast Robust management frame */
			DeAuthHdr.FC.Wep = 1;
		}
#endif /* DOT11_PMF_SUPPORT */


		if (bDataFrameFirst)
			MiniportMMRequest(pAd, MGMT_USE_QUEUE_FLAG, pOutBuffer, FrameLen);
		else
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_ERROR_MAX, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				Reason);
#endif

		/* ApLogEvent(pAd, pEntry->Addr, EVENT_DISASSOCIATED);*/
#ifdef CONFIG_HOTSPOT_R2
		if (!pEntry->IsKeep)
#endif /* CONFIG_HOTSPOT_R2 */
		MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
    }
}


/*
    ==========================================================================
    Description:
        When receiving the last packet of 2-way groupkey handshake.
    Return:
    ==========================================================================
*/
VOID PeerGroupMsg2Action(
    IN PRTMP_ADAPTER    pAd,
    IN MAC_TABLE_ENTRY  *pEntry,
    IN VOID             *Msg,
    IN UINT             MsgLen)
{
    UINT            	Len;
    PUCHAR          	pData;
    BOOLEAN         	Cancelled;
	PEAPOL_PACKET       pMsg2;
#ifdef DBG
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
#endif /* DBG */

	DBGPRINT(RT_DEBUG_TRACE, ("===> PeerGroupMsg2Action \n"));

    if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
        return;

    if (MsgLen < (LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
        return;

    if (pEntry->WpaState != AS_PTKINITDONE)
        return;


    do
    {

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DBG
			UCHAR	apidx = 0;
#endif /* DBG */
			if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)
				return;
		    else
		    {
#ifdef DBG
				apidx = pEntry->func_tb_idx;
				group_cipher = pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus;
#endif /* DBG */
		    }
		}
#endif /* CONFIG_AP_SUPPORT */

        pData = (PUCHAR)Msg;
		pMsg2 = (PEAPOL_PACKET) (pData + LENGTH_802_1_H);
        Len = MsgLen - LENGTH_802_1_H;

		/* Sanity Check peer group message 2 - Replay Counter, MIC*/
		if (PeerWpaMessageSanity(pAd, pMsg2, Len, EAPOL_GROUP_MSG_2, pEntry) == FALSE) {
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG2_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif
			break;
		}

        /* 3.  upgrade state*/

		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
        pEntry->GTKState = REKEY_ESTABLISHED;
#ifdef CONFIG_PUSH_SUPPORT
		if (VALID_TR_WCID(pEntry->wcid))
		{
			EventHdlr pEventHdlrHook = NULL;
			STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			if(tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_JOIN);
				if(pEventHdlrHook && pEntry->wdev)
					pEventHdlrHook(pAd, pEntry, pAd->CommonCfg.Channel);
			}
		}
#endif /* CONFIG_PUSH_SUPPORT */

		if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		{
			/* send wireless event - for set key done WPA2*/
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

			DBGPRINT(RT_DEBUG_TRACE, ("AP SETKEYS DONE - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
										group_cipher, GetEncryptType(group_cipher)));
		}
		else
		{
			/* send wireless event - for set key done WPA*/
				RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA1_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

        	DBGPRINT(RT_DEBUG_TRACE, ("AP SETKEYS DONE - WPA1, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n",
										pEntry->AuthMode, GetAuthMode(pEntry->AuthMode),
										pEntry->WepStatus, GetEncryptType(pEntry->WepStatus),
										group_cipher, GetEncryptType(group_cipher)));
		}
    }while(FALSE);
}

/*
	========================================================================

	Routine Description:
		Classify WPA EAP message type

	Arguments:
		EAPType		Value of EAP message type
		MsgType		Internal Message definition for MLME state machine

	Return Value:
		TRUE		Found appropriate message type
		FALSE		No appropriate message type

	IRQL = DISPATCH_LEVEL

	Note:
		All these constants are defined in wpa_cmm.h
		For supplicant, there is only EAPOL Key message avaliable

	========================================================================
*/
BOOLEAN	WpaMsgTypeSubst(
	IN	UCHAR	EAPType,
	OUT	INT		*MsgType)
{
	switch (EAPType)
	{
		case EAPPacket:
			*MsgType = MT2_EAPPacket;
			break;
		case EAPOLStart:
			*MsgType = MT2_EAPOLStart;
			break;
		case EAPOLLogoff:
			*MsgType = MT2_EAPOLLogoff;
			break;
		case EAPOLKey:
			*MsgType = MT2_EAPOLKey;
			break;
		case EAPOLASFAlert:
			*MsgType = MT2_EAPOLASFAlert;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

/**
 * inc_iv_byte - Increment arbitrary length byte array
 * @counter: Pointer to byte array
 * @len: Length of the counter in bytes
 *
 * This function increments the least byte of the counter by one and continues
 * rolling over to more significant bytes if the byte was incremented from
 * 0xff to 0x00.
 */
void inc_iv_byte(UCHAR *iv, UINT len, UINT cnt)
{
	int 	pos = 0;
	int 	carry = 0;
	UCHAR	pre_iv;

	while (pos < len)
	{
		pre_iv = iv[pos];

		if (carry == 1)
			iv[pos] ++;
		else
			iv[pos] += cnt;

		if (iv[pos] > pre_iv)
			break;

		carry = 1;
		pos++;
	}

	if (pos >= len)
		DBGPRINT(RT_DEBUG_WARN, ("!!! inc_iv_byte overflow !!!\n"));
}



/*
	========================================================================

	Routine Description:
		The pseudo-random function(PRF) that hashes various inputs to
		derive a pseudo-random value. To add liveness to the pseudo-random
		value, a nonce should be one of the inputs.

		It is used to generate PTK, GTK or some specific random value.

	Arguments:
		UCHAR	*key,		-	the key material for HMAC_SHA1 use
		INT		key_len		-	the length of key
		UCHAR	*prefix		-	a prefix label
		INT		prefix_len	-	the length of the label
		UCHAR	*data		-	a specific data with variable length
		INT		data_len	-	the length of a specific data
		INT		len			-	the output lenght

	Return Value:
		UCHAR	*output		-	the calculated result

	Note:
		802.11i-2004	Annex H.3

	========================================================================
*/
VOID	PRF(
	IN	UCHAR	*key,
	IN	INT		key_len,
	IN	UCHAR	*prefix,
	IN	INT		prefix_len,
	IN	UCHAR	*data,
	IN	INT		data_len,
	OUT	UCHAR	*output,
	IN	INT		len)
{
	INT		i;
    UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;

	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

    if (input == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!PRF: no memory!!!\n"));
        return;
    }

	/* Generate concatenation input*/
	NdisMoveMemory(input, prefix, prefix_len);

	/* Concatenate a single octet containing 0*/
	input[prefix_len] =	0;

	/* Concatenate specific data*/
	NdisMoveMemory(&input[prefix_len + 1], data, data_len);
	total_len =	prefix_len + 1 + data_len;

	/* Concatenate a single octet containing 0*/
	/* This octet shall be update later*/
	input[total_len] = 0;
	total_len++;

	/* Iterate to calculate the result by hmac-sha-1*/
	/* Then concatenate to last result*/
	for	(i = 0;	i <	(len + 19) / 20; i++)
	{
		RT_HMAC_SHA1(key, key_len, input, total_len, &output[currentindex], SHA1_DIGEST_SIZE);
		currentindex +=	20;

		/* update the last octet */
		input[total_len - 1]++;
	}
    os_free_mem(NULL, input);
}

/*
* F(P, S, c, i) = U1 xor U2 xor ... Uc
* U1 = PRF(P, S || Int(i))
* U2 = PRF(P, U1)
* Uc = PRF(P, Uc-1)
*/

static void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output)
{
    unsigned char digest[36], digest1[SHA1_DIGEST_SIZE];
    int i, j, len;

	len = strlen(password);

    /* U1 = PRF(P, S || int(i)) */
    memcpy(digest, ssid, ssidlength);
    digest[ssidlength] = (unsigned char)((count>>24) & 0xff);
    digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff);
    digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff);
    digest[ssidlength+3] = (unsigned char)(count & 0xff);
    RT_HMAC_SHA1((unsigned char*) password, len, digest, ssidlength+4, digest1, SHA1_DIGEST_SIZE); /* for WPA update*/

    /* output = U1 */
    memcpy(output, digest1, SHA1_DIGEST_SIZE);
    for (i = 1; i < iterations; i++)
    {
        /* Un = PRF(P, Un-1) */
        RT_HMAC_SHA1((unsigned char*) password, len, digest1, SHA1_DIGEST_SIZE, digest, SHA1_DIGEST_SIZE); /* for WPA update*/
        memcpy(digest1, digest, SHA1_DIGEST_SIZE);

        /* output = output xor Un */
        for (j = 0; j < SHA1_DIGEST_SIZE; j++)
        {
            output[j] ^= digest[j];
        }
    }
}

/*
* password - ascii string up to 63 characters in length
* ssid - octet string up to 32 octets
* ssidlength - length of ssid in octets
* output must be 40 octets in length and outputs 256 bits of key
*/
int RtmpPasswordHash(RTMP_STRING *password, PUCHAR ssid, INT ssidlength, PUCHAR output)
{
    if ((strlen(password) > 63) || (ssidlength > 32))
        return 0;

    F(password, ssid, ssidlength, 4096, 1, output);
    F(password, ssid, ssidlength, 4096, 2, &output[SHA1_DIGEST_SIZE]);
    return 1;
}

/*
	========================================================================

	Routine Description:
		The key derivation function(KDF) is defined in IEEE 802.11r/D9.0, 8.5.1.5.2

	Arguments:

	Return Value:

	Note:
		Output - KDF-Length (K, label, Context) where
		Input:    K, a 256-bit key derivation key
				  label, a string identifying the purpose of the keys derived using this KDF
				  Context, a bit string that provides context to identify the derived key
				  Length, the length of the derived key in bits
		Output: a Length-bit derived key

		result - ""
		iterations - (Length+255)/256
		do i = 1 to iterations
			result - result || HMAC-SHA256(K, i || label || Context || Length)
		od
		return first Length bits of result, and securely delete all unused bits

		In this algorithm, i and Length are encoded as 16-bit unsigned integers.

	========================================================================
*/
VOID	KDF(
	IN	PUINT8	key,
	IN	INT		key_len,
	IN	PUINT8	label,
	IN	INT		label_len,
	IN	PUINT8	data,
	IN	INT		data_len,
	OUT	PUINT8	output,
	IN	USHORT	len)
{
	USHORT	i;
    UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;
	UINT	len_in_bits = (len << 3);

	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("!!!KDF: no memory!!!\n"));
		return;
	}

	NdisZeroMemory(input, 1024);

	/* Initial concatenated value (i || label || Context || Length)*/
	/* concatenate 16-bit unsigned integer, its initial value is 1.	*/
	input[0] = 1;
	input[1] = 0;
	total_len = 2;

	/* concatenate a prefix string*/
	NdisMoveMemory(&input[total_len], label, label_len);
	total_len += label_len;

	/* concatenate the context*/
	NdisMoveMemory(&input[total_len], data, data_len);
	total_len += data_len;

	/* concatenate the length in bits (16-bit unsigned integer)*/
	input[total_len] = (len_in_bits & 0xFF);
	input[total_len + 1] = (len_in_bits & 0xFF00) >> 8;
	total_len += 2;

	for	(i = 1;	i <= ((len_in_bits + 255) / 256); i++)
	{
		/* HMAC-SHA256 derives output */
		RT_HMAC_SHA256((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], 32);

		currentindex +=	32; /* next concatenation location*/
		input[0]++;			/* increment octet count*/

	}
    os_free_mem(NULL, input);
}


/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	Note:

	========================================================================
*/
VOID RTMPDerivePMKID(
	IN	PUINT8			pAaddr,
	IN	PUINT8			pSpaddr,
	IN	PUINT8			pKey,
	IN	PUINT8			pAkm_oui,
	OUT	PUINT8			pPMKID)
{
	UCHAR	digest[80], text_buf[20];
	UINT8	text_len;

	/* Concatenate the text for PMKID calculation*/
	NdisMoveMemory(&text_buf[0], "PMK Name", 8);
	NdisMoveMemory(&text_buf[8], pAaddr, MAC_ADDR_LEN);
	NdisMoveMemory(&text_buf[14], pSpaddr, MAC_ADDR_LEN);
	text_len = 20;

#ifdef DOT11W_PMF_SUPPORT
/* todo pmf*/
	/* 	When the negotiated AKM is 00-0F-AC:5 or 00-0F-AC:6,
		HMAC-SHA-256 is used to calculate the PMKID */
	if (NdisEqualMemory(pAkm_oui, OUI_PMF_8021X_AKM, 4) ||
		NdisEqualMemory(pAkm_oui, OUI_PMF_PSK_AKM, 4))
	{
		RT_HMAC_SHA256(pKey, PMK_LEN, text_buf, text_len, digest, SHA256_DIGEST_SIZE);
	}
	else
#endif /* DOT11W_PMF_SUPPPORT */
	{
		RT_HMAC_SHA1(pKey, PMK_LEN, text_buf, text_len, digest, SHA1_DIGEST_SIZE);
	}

	/* Truncate the first 128-bit of output result */
	NdisMoveMemory(pPMKID, digest, LEN_PMKID);

}



/*
	========================================================================

	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.

	Arguments:
		pAd 	-	pointer to our pAdapter context
		PMK		-	pointer to PMK
		ANonce	-	pointer to ANonce
		AA		-	pointer to Authenticator Address
		SNonce	-	pointer to SNonce
		SA		-	pointer to Supplicant Address
		len		-	indicate the length of PTK (octet)

	Return Value:
		Output		pointer to the PTK

	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2

	========================================================================
*/
VOID WpaDerivePTK(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR	*PMK,
	IN	UCHAR	*ANonce,
	IN	UCHAR	*AA,
	IN	UCHAR	*SNonce,
	IN	UCHAR	*SA,
	OUT	UCHAR	*output,
	IN	UINT	len)
{
	UCHAR	concatenation[76];
	UINT	CurrPos = 0;
	UCHAR	temp[32];
	UCHAR	Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ',
						'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	/* initiate the concatenation input*/
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);
	CurrPos += 6;

	/* Get larger address*/
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);

	/* store the larger mac address for backward compatible of */
	/* ralink proprietary STA-key issue		*/
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);
	CurrPos += 6;

	/* Get smaller Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	CurrPos += 32;

	/* Get larger Nonce*/
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);	/* patch for ralink proprietary STA-key issue*/
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	CurrPos += 32;


	/* Use PRF to generate PTK*/
	PRF(PMK, LEN_PMK, Prefix, 22, concatenation, 76, output, len);

}

VOID WpaDeriveGTK(
    IN  UCHAR   *GMK,
    IN  UCHAR   *GNonce,
    IN  UCHAR   *AA,
    OUT UCHAR   *output,
    IN  UINT    len)
{
    UCHAR   concatenation[76];
    UINT    CurrPos=0;
    UCHAR   Prefix[19];
    UCHAR   temp[80];

    NdisMoveMemory(&concatenation[CurrPos], AA, 6);
    CurrPos += 6;

    NdisMoveMemory(&concatenation[CurrPos], GNonce , 32);
    CurrPos += 32;

    Prefix[0] = 'G';
    Prefix[1] = 'r';
    Prefix[2] = 'o';
    Prefix[3] = 'u';
    Prefix[4] = 'p';
    Prefix[5] = ' ';
    Prefix[6] = 'k';
    Prefix[7] = 'e';
    Prefix[8] = 'y';
    Prefix[9] = ' ';
    Prefix[10] = 'e';
    Prefix[11] = 'x';
    Prefix[12] = 'p';
    Prefix[13] = 'a';
    Prefix[14] = 'n';
    Prefix[15] = 's';
    Prefix[16] = 'i';
    Prefix[17] = 'o';
    Prefix[18] = 'n';

    PRF(GMK, PMK_LEN, Prefix,  19, concatenation, 38 , temp, len);
    NdisMoveMemory(output, temp, len);
}

/*
	========================================================================

	Routine Description:
		Generate random number by software.

	Arguments:
		pAd		-	pointer to our pAdapter context
		macAddr	-	pointer to local MAC address

	Return Value:

	Note:
		802.1ii-2004  Annex H.5

	========================================================================
*/
VOID	GenRandom(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			*macAddr,
	OUT	UCHAR			*random)
{
	INT		i, curr;
	UCHAR	local[80], KeyCounter[32];
	UCHAR	result[80];
	ULONG	CurrentTime;
	UCHAR	prefix[] = {'I', 'n', 'i', 't', ' ', 'C', 'o', 'u', 'n', 't', 'e', 'r'};

	/* Zero the related information*/
	NdisZeroMemory(result, 80);
	NdisZeroMemory(local, 80);
	NdisZeroMemory(KeyCounter, 32);

	for	(i = 0;	i <	32;	i++)
	{
		/* copy the local MAC address*/
		COPY_MAC_ADDR(local, macAddr);
		curr =	MAC_ADDR_LEN;

		/* concatenate the current time*/
		NdisGetSystemUpTime(&CurrentTime);
		NdisMoveMemory(&local[curr],  &CurrentTime,	sizeof(CurrentTime));
		curr +=	sizeof(CurrentTime);

		/* concatenate the last result*/
		NdisMoveMemory(&local[curr],  result, 32);
		curr +=	32;

		/* concatenate a variable */
		NdisMoveMemory(&local[curr],  &i,  2);
		curr +=	2;

		/* calculate the result*/
		PRF(KeyCounter, 32, prefix,12, local, curr, result, 32);
	}

	NdisMoveMemory(random, result,	32);
}

/*
	========================================================================

	Routine Description:
		Build cipher suite in RSN-IE.
		It only shall be called by RTMPMakeRSNIE.

	Arguments:
		pAd			-	pointer to our pAdapter context
    	ElementID	-	indicate the WPA1 or WPA2
    	WepStatus	-	indicate the encryption type
		bMixCipher	-	a boolean to indicate the pairwise cipher and group
						cipher are the same or not

	Return Value:

	Note:

	========================================================================
*/
static VOID RTMPMakeRsnIeCipher(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UINT			WepStatus,
#ifdef DOT11_SAE_SUPPORT
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
#endif
	IN	UCHAR			apidx,
	IN	BOOLEAN			bMixCipher,
	IN	UCHAR			FlexibleCipher,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	UCHAR	PairwiseCnt;

	*rsn_len = 0;

	/* decide WPA2 or WPA1	*/
	if (ElementID == Wpa2Ie)
	{
		RSNIE2	*pRsnie_cipher = (RSNIE2*)pRsnIe;

		/* Assign the verson as 1*/
		pRsnie_cipher->version = 1;

#ifdef DOT11_SAE_SUPPORT
	if (WepStatus == Ndis802_11TKIPEnable || WepStatus == Ndis802_11TKIPAESMix) {
		if (AuthMode == Ndis802_11AuthModeWPA3PSK ||
			AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("==> %s not support group cipher on SAE\n", __func__));
			return;
		}
	}
#endif

        switch (WepStatus)
        {
        	/* TKIP mode*/
            case Ndis802_11TKIPEnable:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;

			/* AES mode*/
            case Ndis802_11AESEnable:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CCMP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);
                *rsn_len = sizeof(RSNIE2);
                break;
#ifdef CONFIG_OWE_SUPPORT
	case Ndis802_11CCMP128Enable:
		NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP128, 4);
		pRsnie_cipher->ucount = 1;
		NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CIPHER_CCMP128, 4);
		*rsn_len = sizeof(RSNIE2);/* mixed TBD */
		break;
	case Ndis802_11CCMP256Enable:
		NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP256, 4);
		pRsnie_cipher->ucount = 1;
		NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CIPHER_CCMP256, 4);
		*rsn_len = sizeof(RSNIE2);
		break;
#endif
			/* TKIP-AES mix mode*/
            case Ndis802_11TKIPAESMix:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_TKIP, 4);

				PairwiseCnt = 1;
				/* Insert WPA2 TKIP as the first pairwise cipher */
				if (MIX_CIPHER_WPA2_TKIP_ON(FlexibleCipher))
				{
                	NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_TKIP, 4);
					/* Insert WPA2 AES as the secondary pairwise cipher*/
					if (MIX_CIPHER_WPA2_AES_ON(FlexibleCipher))
					{
						NdisMoveMemory(pRsnIe + sizeof(RSNIE2), OUI_WPA2_CCMP, 4);
						PairwiseCnt = 2;
					}
				}
				else
				{
					/* Insert WPA2 AES as the first pairwise cipher */
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA2_CCMP, 4);
				}

                pRsnie_cipher->ucount = PairwiseCnt;
                *rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
                break;
        }


		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
	else
	{
		RSNIE	*pRsnie_cipher = (RSNIE*)pRsnIe;

		/* Assign OUI and version*/
		NdisMoveMemory(pRsnie_cipher->oui, OUI_WPA_VERSION, 4);
        pRsnie_cipher->version = 1;

		switch (WepStatus)
		{
			/* TKIP mode*/
            case Ndis802_11TKIPEnable:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			/* AES mode*/
            case Ndis802_11AESEnable:
				if (bMixCipher)
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
				else
					NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_CCMP, 4);
                pRsnie_cipher->ucount = 1;
                NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);
                *rsn_len = sizeof(RSNIE);
                break;

			/* TKIP-AES mix mode*/
            case Ndis802_11TKIPAESMix:
                NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);

				PairwiseCnt = 1;
				/* Insert WPA TKIP as the first pairwise cipher */
				if (MIX_CIPHER_WPA_TKIP_ON(FlexibleCipher))
				{
                	NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_TKIP, 4);
					/* Insert WPA AES as the secondary pairwise cipher*/
					if (MIX_CIPHER_WPA_AES_ON(FlexibleCipher))
					{
						NdisMoveMemory(pRsnIe + sizeof(RSNIE), OUI_WPA_CCMP, 4);
						PairwiseCnt = 2;
					}
				}
				else
				{
					/* Insert WPA AES as the first pairwise cipher */
					NdisMoveMemory(pRsnie_cipher->ucast[0].oui, OUI_WPA_CCMP, 4);
				}

                pRsnie_cipher->ucount = PairwiseCnt;
                *rsn_len = sizeof(RSNIE) + (4 * (PairwiseCnt - 1));
                break;
        }


		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
	    pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}
}

/*
	========================================================================

	Routine Description:
		Build AKM suite in RSN-IE.
		It only shall be called by RTMPMakeRSNIE.

	Arguments:
		pAd			-	pointer to our pAdapter context
    	ElementID	-	indicate the WPA1 or WPA2
    	AuthMode	-	indicate the authentication mode
		apidx		-	indicate the interface index

	Return Value:

	Note:

	========================================================================
*/
static VOID RTMPMakeRsnIeAKM(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UINT			AuthMode,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSNIE_AUTH		*pRsnie_auth;
	UCHAR			AkmCnt = 1;		/* default as 1*/
#ifdef DOT11R_FT_SUPPORT
	BOOLEAN			bFtEnabled = FALSE;
	UINT8			FtAkmOui[8];
#endif /* DOT11R_FT_SUPPORT */

	pRsnie_auth = (RSNIE_AUTH*)(pRsnIe + (*rsn_len));

	/* decide WPA2 or WPA1	 */
	if (ElementID == Wpa2Ie)
	{
#ifdef DOT11R_FT_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (apidx < pAd->ApCfg.BssidNum)
			{
				/* Indicate the FT is enabld*/
				if (pAd->ApCfg.MBSSID[apidx].FtCfg.FtCapFlag.Dot11rFtEnable)
				{
					bFtEnabled = TRUE;
					AkmCnt = 2;
					if (AuthMode == Ndis802_11AuthModeWPA2 ||
						AuthMode == Ndis802_11AuthModeWPA1WPA2)
					{
						NdisMoveMemory(FtAkmOui, OUI_WPA2_8021X_AKM, 4);
						NdisMoveMemory(&FtAkmOui[4], OUI_FT_8021X_AKM, 4);
					}
					else if (AuthMode == Ndis802_11AuthModeWPA2PSK ||
							AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK)
					{
						NdisMoveMemory(FtAkmOui, OUI_WPA2_PSK_AKM, 4);
						NdisMoveMemory(&FtAkmOui[4], OUI_FT_PSK_AKM, 4);
					}
					else
						AkmCnt = 0;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11R_FT_SUPPORT */

		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA1WPA2:
#ifdef DOT11R_FT_SUPPORT
				if (bFtEnabled && (AkmCnt > 0))
				{
					/* append the primary AKM suite */
					NdisMoveMemory(pRsnie_auth->auth[0].oui, FtAkmOui, 4);

					/* Append the secondary AKM suite if it exists */
					if (AkmCnt == 2)
					{
						/* Get the offset of the end of 1st AKM suite */
						UCHAR ext_akm_offset = (*rsn_len) + sizeof(RSNIE_AUTH);

						NdisMoveMemory(pRsnIe + ext_akm_offset, &FtAkmOui[4] , 4);
					}
				}
				else
#endif /* DOT11R_FT_SUPPORT */
                	NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_8021X_AKM, 4);

#ifdef DOT11W_PMF_SUPPORT
#ifdef CONFIG_AP_SUPPORT
                IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
                {
                        if (pAd->ApCfg.MBSSID[apidx].PmfCfg.MFPR) {
                                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_1X_SHA256, 4);
                                DBGPRINT(RT_DEBUG_WARN, ("[PMF]%s: Insert 1X-SHA256 to AKM of RSNIE\n", __FUNCTION__));
                        } else if ((pAd->ApCfg.MBSSID[apidx].PmfCfg.MFPC) && (pAd->ApCfg.MBSSID[apidx].PmfCfg.PMFSHA256)) {
                                NdisMoveMemory(pRsnie_auth->auth[0].oui + (4*AkmCnt), OUI_WPA2_1X_SHA256, 4);
                                AkmCnt++;
                                DBGPRINT(RT_DEBUG_WARN, ("[PMF]%s: Insert 1X-SHA256 to AKM of RSNIE\n", __FUNCTION__));
                        }
                }
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */
                break;
#ifdef DOT11_SAE_SUPPORT
	case Ndis802_11AuthModeWPA3PSK:
		NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_AKM_SAE_SHA256, 4);
		AkmCnt = 1;/* FT WPA3 PSK is TBD */
		break;
#endif
#ifdef CONFIG_OWE_SUPPORT
	case Ndis802_11AuthModeOWE:
		NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_AKM_OWE, 4);
		AkmCnt = 1;/* FT WPA3 PSK is TBD */
		break;
#endif
#ifdef DOT11_SAE_SUPPORT
	case Ndis802_11AuthModeWPA2PSKWPA3PSK:
		AkmCnt = 0;
		NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_AKM, 4);
		AkmCnt++;
		NdisMoveMemory(pRsnie_auth->auth[0].oui + (4*AkmCnt), OUI_WPA2_AKM_SAE_SHA256, 4);
		AkmCnt++;
		break;
#endif
            case Ndis802_11AuthModeWPA2PSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:

#ifdef DOT11R_FT_SUPPORT
				if (bFtEnabled && (AkmCnt > 0))
				{
					/* append the primary AKM suite */
					NdisMoveMemory(pRsnie_auth->auth[0].oui, FtAkmOui, 4);

					/* Append the secondary AKM suite if it exists */
					if (AkmCnt == 2)
					{
						/* Get the offset of the end of 1st AKM suite */
						UCHAR ext_akm_offset = (*rsn_len) + sizeof(RSNIE_AUTH);

						NdisMoveMemory(pRsnIe + ext_akm_offset, &FtAkmOui[4] , 4);
					}
				}
				else
#endif /* DOT11R_FT_SUPPORT */
                	NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_AKM, 4);

#ifdef DOT11W_PMF_SUPPORT
#ifdef CONFIG_AP_SUPPORT
                IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
                {
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef APCLI_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
				PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[apidx - MIN_NET_DEVICE_FOR_APCLI];

				if (pApCliEntry) {
					DBGPRINT(RT_DEBUG_WARN,
						("[PMF]%s: IsSupportSHA256KeyDerivation = %d, MFPR = %d\n",
						__func__, pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation,
						pApCliEntry->MlmeAux.RsnCap.field.MFPR));

					if ((pApCliEntry->PmfCfg.PMFSHA256 &&
						pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation)
						|| (pApCliEntry->PmfCfg.MFPC &&
						pApCliEntry->MlmeAux.RsnCap.field.MFPR)
						|| (pApCliEntry->PmfCfg.MFPC &&
						pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation)) {
						NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_SHA256, 4);
						DBGPRINT(RT_DEBUG_WARN,
							("[PMF]%s: Insert PSK-SHA256 to AKM of RSNIE\n", __func__));
					}
				}
			} else
#endif /* APCLI_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
			{
				if (pAd->ApCfg.MBSSID[apidx].PmfCfg.MFPR) {
					NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA2_PSK_SHA256, 4);
					DBGPRINT(RT_DEBUG_WARN,
						("[PMF]%s: Insert PSK-SHA256 to AKM of RSNIE\n", __func__));
				} else if ((pAd->ApCfg.MBSSID[apidx].PmfCfg.MFPC) &&
					(pAd->ApCfg.MBSSID[apidx].PmfCfg.PMFSHA256)) {
					NdisMoveMemory(pRsnie_auth->auth[0].oui + (4*AkmCnt), OUI_WPA2_PSK_SHA256, 4);
					AkmCnt++;
					DBGPRINT(RT_DEBUG_WARN,
						("[PMF]%s: Insert PSK-SHA256 to AKM of RSNIE\n", __func__));
				}
			}
                }
#endif /* CONFIG_AP_SUPPORT */

#endif /* DOT11W_PMF_SUPPORT */
                break;
			default:
				AkmCnt = 0;
				break;

        }
	}
	else
	{
		switch (AuthMode)
        {
            case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPA1WPA2:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_8021X_AKM, 4);
                break;

            case Ndis802_11AuthModeWPAPSK:
            case Ndis802_11AuthModeWPA1PSKWPA2PSK:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_PSK_AKM, 4);
                break;

			case Ndis802_11AuthModeWPANone:
                NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_NONE_AKM, 4);
                break;
			default:
				AkmCnt = 0;
				break;
        }
	}

	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);

	/* update current RSNIE length*/
	(*rsn_len) += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));

}

/*
	========================================================================

	Routine Description:
		Build capability in RSN-IE.
		It only shall be called by RTMPMakeRSNIE.

	Arguments:
		pAd			-	pointer to our pAdapter context
    	ElementID	-	indicate the WPA1 or WPA2
		apidx		-	indicate the interface index

	Return Value:

	Note:

	========================================================================
*/
static VOID RTMPMakeRsnIeCap(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	RSN_CAPABILITIES    *pRSN_Cap;

	/* it could be ignored in WPA1 mode*/
	if (ElementID == WpaIe)
		return;

	pRSN_Cap = (RSN_CAPABILITIES*)(pRsnIe + (*rsn_len));

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
#ifdef APCLI_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[apidx - MIN_NET_DEVICE_FOR_APCLI];

			pRSN_Cap->field.MFPC = (pApCliEntry->PmfCfg.MFPC) ? 1 : 0;
			pRSN_Cap->field.MFPR = (pApCliEntry->PmfCfg.MFPR) ? 1 : 0;

			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: RSNIE Capability MFPC=%d, MFPR=%d\n",
							__func__, pRSN_Cap->field.MFPC, pRSN_Cap->field.MFPR));
		} else
#endif /* APCLI_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
		if (apidx < pAd->ApCfg.BssidNum)
		{
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];

#ifdef DOT1X_SUPPORT
        	pRSN_Cap->field.PreAuth = (pMbss->PreAuth == TRUE) ? 1 : 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
                        pRSN_Cap->field.MFPC = (pMbss->PmfCfg.MFPC) ? 1 : 0;
                        pRSN_Cap->field.MFPR = (pMbss->PmfCfg.MFPR) ? 1 : 0;
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: RSNIE Capability MFPC=%d, MFPR=%d\n", __FUNCTION__, pRSN_Cap->field.MFPC, pRSN_Cap->field.MFPR));
#endif /* DOT11W_PMF_SUPPORT */
		}
	}
#endif /* CONFIG_AP_SUPPORT */


	pRSN_Cap->word = cpu2le16(pRSN_Cap->word);

	(*rsn_len) += sizeof(RSN_CAPABILITIES);	/* update current RSNIE length*/

}

/*
	========================================================================

	Routine Description:
		Build PMKID in RSN-IE.
		It only shall be called by RTMPMakeRSNIE.

	Arguments:
		pAd			-	pointer to our pAdapter context
    	ElementID	-	indicate the WPA1 or WPA2
		apidx		-	indicate the interface index

	Return Value:

	Note:

	========================================================================
*/
#ifdef DOT11W_PMF_SUPPORT
static VOID RTMPInsertRsnIeZeroPMKID(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{
	PUINT8    	pBuf;
	UCHAR		ZeroPmkID[2] = {0x00, 0x00};

	/* it could be ignored in WPA1 mode*/
	if (ElementID == WpaIe)
		return;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef APCLI_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[apidx - MIN_NET_DEVICE_FOR_APCLI];

			if (pApCliEntry->PmfCfg.MFPC)
				goto InsertPMKIDCount;
		} else
#endif /* APCLI_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
		if (apidx < pAd->ApCfg.BssidNum)
		{
			BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
                        if (pMbss->PmfCfg.MFPC)
                                goto InsertPMKIDCount;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
        return;

InsertPMKIDCount:
	pBuf = (pRsnIe + (*rsn_len));

	NdisMoveMemory(pBuf, ZeroPmkID, 2);
	(*rsn_len) += 2;	/* update current RSNIE length	*/
}
#endif /* DOT11W_PMF_SUPPORT */

#if defined(CONFIG_OWE_SUPPORT) || defined(DOT11_SAE_SUPPORT)
static VOID RTMPInsertRsnIePMKID(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			ElementID,
	IN	UINT			AuthMode,
	IN	UCHAR			apidx,
	IN MAC_TABLE_ENTRY * pEntry,
	OUT	PUCHAR			pRsnIe,
	OUT	UCHAR			*rsn_len)
{

	PUINT8		pBuf;
#ifdef DOT11W_PMF_SUPPORT
	UCHAR ZeroPmkID[2] = {0x00, 0x00};
	BOOLEAN MFP_Enabled = FALSE;
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef APCLI_SUPPORT
	if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
		PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[apidx - MIN_NET_DEVICE_FOR_APCLI];

		MFP_Enabled = pApCliEntry->PmfCfg.MFPC;
	} else
#endif /* APCLI_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
	if (apidx < pAd->ApCfg.BssidNum) {
		BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];

		MFP_Enabled = pMbss->PmfCfg.MFPC;
	}
#endif

	if (ElementID == Wpa2Ie) {

#ifdef CONFIG_AP_SUPPORT
	pBuf = pRsnIe + (*rsn_len);
#ifdef DOT11_SAE_SUPPORT
	if ((AuthMode == Ndis802_11AuthModeWPA3PSK || AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK)
		&& pEntry && (IS_ENTRY_CLIENT(pEntry)
#ifdef APCLI_SAE_SUPPORT
		|| IS_ENTRY_APCLI(pEntry)
#endif
		) && is_pmkid_cache_in_entry(pEntry)) {

		UINT16	pmk_count = 1;

		pmk_count = cpu2le16(pmk_count);
		NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
		NdisMoveMemory(pBuf+sizeof(pmk_count), pEntry->pmkid, LEN_PMKID);
		(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: (SAE) including the PMKID.\n", __func__));
		}
#endif
#ifdef CONFIG_OWE_SUPPORT
	else if (AuthMode == Ndis802_11AuthModeOWE
		&& pEntry && (IS_ENTRY_CLIENT(pEntry)
#ifdef APCLI_OWE_SUPPORT
		|| IS_ENTRY_APCLI(pEntry)
#endif
		) && is_pmkid_cache_in_entry(pEntry)) {
		UINT16	pmk_count = 1;

		pmk_count = cpu2le16(pmk_count);
		NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
		NdisMoveMemory(pBuf+sizeof(pmk_count), pEntry->pmkid, LEN_PMKID);
		hex_dump("pmkid = ",  pEntry->pmkid, LEN_PMKID);
		(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: (OWE) including the PMKID.\n", __func__));
			}
#endif
#ifdef DOT11W_PMF_SUPPORT
		else if (MFP_Enabled) {
			NdisMoveMemory(pBuf, ZeroPmkID, 2);
			(*rsn_len) += 2;
		}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	}
}
#endif

/*
	========================================================================

	Routine Description:
		Build RSN IE context. It is not included element-ID and length.

	Arguments:
		pAd			-	pointer to our pAdapter context
    	AuthMode	-	indicate the authentication mode
    	WepStatus	-	indicate the encryption type
		apidx		-	indicate the interface index

	Return Value:

	Note:

	========================================================================
*/
VOID RTMPMakeRSNIE(RTMP_ADAPTER *pAd, UINT AuthMode, UINT WepStatus, UCHAR apidx)
{
	PUCHAR		pRsnIe = NULL;			/* primary RSNIE*/
	UCHAR 		*rsnielen_cur_p = 0;	/* the length of the primary RSNIE 		*/
#ifdef CONFIG_AP_SUPPORT
	PUCHAR		pRsnIe_ex = NULL;		/* secondary RSNIE, it's ONLY used in WPA-mix mode */
	BOOLEAN               bMixRsnIe = FALSE;      /* indicate WPA-mix mode is on or off*/
	UCHAR		s_offset;
#endif /* CONFIG_AP_SUPPORT */
	UCHAR		*rsnielen_ex_cur_p = 0;	/* the length of the secondary RSNIE	  	*/
	UCHAR		PrimaryRsnie;
	BOOLEAN		bMixCipher = FALSE;	/* indicate the pairwise and group cipher are different*/
	UCHAR		p_offset;
	WPA_MIX_PAIR_CIPHER FlexibleCipher = MIX_CIPHER_NOTUSE;	/* it provide the more flexible cipher combination in WPA-WPA2 and TKIPAES mode*/

	rsnielen_cur_p = NULL;
	rsnielen_ex_cur_p = NULL;

	do
	{

#ifdef APCLI_SUPPORT
		if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			UINT apcliIfidx = 0;

			/* Only support WPAPSK or WPA2PSK for AP-Client mode */
			{
				if ((AuthMode != Ndis802_11AuthModeWPAPSK) &&
					(AuthMode != Ndis802_11AuthModeWPA2PSK))
				return;
			}

			DBGPRINT(RT_DEBUG_TRACE,("==> RTMPMakeRSNIE(ApCli)\n"));

			apcliIfidx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			/* Initiate some related information */
			if (apcliIfidx < MAX_APCLI_NUM)
			{
				pAd->ApCfg.ApCliTab[apcliIfidx].RSNIE_Len = 0;
				NdisZeroMemory(pAd->ApCfg.ApCliTab[apcliIfidx].RSN_IE, MAX_LEN_OF_RSNIE);
				rsnielen_cur_p = &pAd->ApCfg.ApCliTab[apcliIfidx].RSNIE_Len;
				pRsnIe = pAd->ApCfg.ApCliTab[apcliIfidx].RSN_IE;

				bMixCipher = pAd->ApCfg.ApCliTab[apcliIfidx].bMixCipher;
				break;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("RTMPMakeRSNIE: invalid apcliIfidx(%d)\n", apcliIfidx));
				return;
			}
		}
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Sanity check for apidx */
			MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);
#ifdef HOSTAPD_SUPPORT
			if(pAd->ApCfg.MBSSID[apidx].Hostapd)
				return;
#endif /* HOSTAPD_SUPPORT */

			if ((AuthMode != Ndis802_11AuthModeWPA) &&
				(AuthMode != Ndis802_11AuthModeWPAPSK) &&
				(AuthMode != Ndis802_11AuthModeWPA2) &&
				(AuthMode != Ndis802_11AuthModeWPA2PSK) &&
				(AuthMode != Ndis802_11AuthModeWPA1WPA2) &&
				(AuthMode != Ndis802_11AuthModeWPA1PSKWPA2PSK)
#ifdef WAPI_SUPPORT
				&& (AuthMode != Ndis802_11AuthModeWAICERT)
				&& (AuthMode != Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
				&& (AuthMode != Ndis802_11AuthModeWPA3PSK)
				&& (AuthMode != Ndis802_11AuthModeWPA2PSKWPA3PSK)
#endif /* DOT11_SAE_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
				&& (AuthMode != Ndis802_11AuthModeOWE)
#endif /* DOT11_SAE_SUPPORT */
				)
				return;

			DBGPRINT(RT_DEBUG_TRACE,("==> RTMPMakeRSNIE(AP-ra%d)\n", apidx));

			/* decide the group key encryption type */
			if (WepStatus == Ndis802_11TKIPAESMix)
			{
				pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus = Ndis802_11TKIPEnable;
				FlexibleCipher = pAd->ApCfg.MBSSID[apidx].wdev.WpaMixPairCipher;
			}
			else
				pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus = WepStatus;

			/* Initiate some related information */
			pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0] = 0;
			pAd->ApCfg.MBSSID[apidx].RSNIE_Len[1] = 0;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[0], MAX_LEN_OF_RSNIE);
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].RSN_IE[1], MAX_LEN_OF_RSNIE);

			/* Pointer to the first RSNIE context */
			rsnielen_cur_p = &pAd->ApCfg.MBSSID[apidx].RSNIE_Len[0];
			pRsnIe = pAd->ApCfg.MBSSID[apidx].RSN_IE[0];

			/* Pointer to the secondary RSNIE context */
			rsnielen_ex_cur_p = &pAd->ApCfg.MBSSID[apidx].RSNIE_Len[1];
			pRsnIe_ex = pAd->ApCfg.MBSSID[apidx].RSN_IE[1];

			/* Decide whether the authentication mode is WPA1-WPA2 mixed mode */
			if ((AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
				(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
			{
				bMixRsnIe = TRUE;
			}
			break;
		}
#endif /* CONFIG_AP_SUPPORT */

	} while(FALSE);

	/* indicate primary RSNIE as WPA or WPA2*/
	if ((AuthMode == Ndis802_11AuthModeWPA) ||
		(AuthMode == Ndis802_11AuthModeWPAPSK) ||
		(AuthMode == Ndis802_11AuthModeWPANone) ||
		(AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
		(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
		PrimaryRsnie = WpaIe;
#ifdef WAPI_SUPPORT
	else if ((AuthMode == Ndis802_11AuthModeWAICERT) ||
			 (AuthMode == Ndis802_11AuthModeWAIPSK))
		PrimaryRsnie = WapiIe;
#endif /* WAPI_SUPPORT */
	else
		PrimaryRsnie = Wpa2Ie;

#ifdef WAPI_SUPPORT
	if (PrimaryRsnie == WapiIe)
	{
		RTMPInsertWapiIe(AuthMode, WepStatus, pRsnIe, &p_offset);
	}
	else
#endif /* WAPI_SUPPORT */
	{
		/* Build the primary RSNIE*/
		/* 1. insert cipher suite*/
		RTMPMakeRsnIeCipher(pAd, PrimaryRsnie, WepStatus,
#ifdef DOT11_SAE_SUPPORT
					AuthMode,
#endif
					apidx, bMixCipher, FlexibleCipher, pRsnIe, &p_offset);

		/* 2. insert AKM*/
		RTMPMakeRsnIeAKM(pAd, PrimaryRsnie, AuthMode, apidx, pRsnIe, &p_offset);

		/* 3. insert capability*/
		RTMPMakeRsnIeCap(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);

#ifdef DOT11W_PMF_SUPPORT
		/* 4. Insert PMKID */
		RTMPInsertRsnIeZeroPMKID(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);

		/* 5. Insert Group Management Cipher*/
		PMF_MakeRsnIeGMgmtCipher(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);
#endif /* DOT11W_PMF_SUPPORT */
	}

	/* 4. update the RSNIE length*/
	if (rsnielen_cur_p == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: rsnielen_cur_p == NULL!\n", __FUNCTION__));
		return;
	}

	*rsnielen_cur_p = p_offset;

	hex_dump("The primary RSNIE", pRsnIe, (*rsnielen_cur_p));

#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP)
		)
	{
		/* if necessary, build the secondary RSNIE*/
		if (bMixRsnIe)
		{
			/* 1. insert cipher suite*/
			RTMPMakeRsnIeCipher(pAd, Wpa2Ie, WepStatus,
#ifdef DOT11_SAE_SUPPORT
						AuthMode,
#endif
						apidx, bMixCipher, FlexibleCipher,
						pRsnIe_ex, &s_offset);

			/* 2. insert AKM*/
			RTMPMakeRsnIeAKM(pAd, Wpa2Ie, AuthMode, apidx, pRsnIe_ex, &s_offset);

			/* 3. insert capability*/
			RTMPMakeRsnIeCap(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);

#ifdef DOT11W_PMF_SUPPORT
			/* 4. Insert PMKID */
			RTMPInsertRsnIeZeroPMKID(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);

			/* 5. Insert Group Management Cipher*/
			PMF_MakeRsnIeGMgmtCipher(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);
#endif /* DOT11W_PMF_SUPPORT */

			/* Update the RSNIE length*/
			*rsnielen_ex_cur_p = s_offset;

			hex_dump("The secondary RSNIE", pRsnIe_ex, (*rsnielen_ex_cur_p));
		}
	}
#endif /* CONFIG_AP_SUPPORT */

}


/* For OWE, make RSNIE.*/
#if defined(CONFIG_OWE_SUPPORT) || defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
VOID WPAMakeEntryRSNIE(
	IN PRTMP_ADAPTER pAd,
	IN UINT AuthMode,
	IN UINT WepStatus,
	IN MAC_TABLE_ENTRY * pEntry)
{
		UCHAR apidx;
		PUCHAR		pRsnIe = NULL;			/* primary RSNIE*/
		UCHAR		*rsnielen_cur_p = 0;	/* the length of the primary RSNIE		*/
#ifdef CONFIG_AP_SUPPORT
		PUCHAR		pRsnIe_ex = NULL;		/* secondary RSNIE, it's ONLY used in WPA-mix mode */
		BOOLEAN     bMixRsnIe = FALSE;	  /* indicate WPA-mix mode is on or off*/
		UCHAR		s_offset;
#endif /* CONFIG_AP_SUPPORT */
		UCHAR		*rsnielen_ex_cur_p = 0; /* the length of the secondary RSNIE		*/
		UCHAR		PrimaryRsnie;
		BOOLEAN     bMixCipher = FALSE; /* indicate the pairwise and group cipher are different*/
		UCHAR		p_offset;
		/* it provide the more flexible cipher combination in WPA-WPA2 and TKIPAES mode*/
		WPA_MIX_PAIR_CIPHER FlexibleCipher = MIX_CIPHER_NOTUSE;

		rsnielen_cur_p = NULL;
		rsnielen_ex_cur_p = NULL;
		apidx = pEntry->func_tb_idx;

		do {

#ifdef APCLI_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
				UINT apcliIfidx = 0;

				/* Only support WPAPSK or WPA2PSK for AP-Client mode */
				{
					if ((AuthMode != Ndis802_11AuthModeWPAPSK) &&
						(AuthMode != Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
						&& (AuthMode != Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
						&& (AuthMode != Ndis802_11AuthModeOWE)
#endif
						)
					return;
				}

				DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPMakeRSNIE(ApCli)\n"));

				apcliIfidx = apidx - MIN_NET_DEVICE_FOR_APCLI;

				/* Initiate some related information */
				if (apcliIfidx < MAX_APCLI_NUM) {
					pAd->ApCfg.ApCliTab[apcliIfidx].RSNIE_Len = 0;
					NdisZeroMemory(pAd->ApCfg.ApCliTab[apcliIfidx].RSN_IE, MAX_LEN_OF_RSNIE);
					rsnielen_cur_p = &pAd->ApCfg.ApCliTab[apcliIfidx].RSNIE_Len;
					pRsnIe = pAd->ApCfg.ApCliTab[apcliIfidx].RSN_IE;

					bMixCipher = pAd->ApCfg.ApCliTab[apcliIfidx].bMixCipher;
					break;
				} else {
					DBGPRINT(RT_DEBUG_ERROR,
						("RTMPMakeRSNIE: invalid apcliIfidx(%d)\n", apcliIfidx));
					return;
				}
			}
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* Sanity check for apidx */
				MBSS_MR_APIDX_SANITY_CHECK(pAd, apidx);
#ifdef HOSTAPD_SUPPORT
				if (pAd->ApCfg.MBSSID[apidx].Hostapd)
					return;
#endif /* HOSTAPD_SUPPORT */

				if ((AuthMode != Ndis802_11AuthModeWPA) &&
					(AuthMode != Ndis802_11AuthModeWPAPSK) &&
					(AuthMode != Ndis802_11AuthModeWPA2) &&
					(AuthMode != Ndis802_11AuthModeWPA2PSK) &&
					(AuthMode != Ndis802_11AuthModeWPA1WPA2) &&
					(AuthMode != Ndis802_11AuthModeWPA1PSKWPA2PSK)
#ifdef WAPI_SUPPORT
					&& (AuthMode != Ndis802_11AuthModeWAICERT)
					&& (AuthMode != Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
					&& (AuthMode != Ndis802_11AuthModeWPA3PSK)
					&& (AuthMode != Ndis802_11AuthModeWPA2PSKWPA3PSK)
#endif /* DOT11_SAE_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
					&& (AuthMode != Ndis802_11AuthModeOWE)
#endif /* DOT11_SAE_SUPPORT */
					)
					return;

				DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPMakeRSNIE(AP-ra%d)\n", apidx));

				/* decide the group key encryption type */
				if (WepStatus == Ndis802_11TKIPAESMix) {
					pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus = Ndis802_11TKIPEnable;
					FlexibleCipher = pAd->ApCfg.MBSSID[apidx].wdev.WpaMixPairCipher;
				} else
					pAd->ApCfg.MBSSID[apidx].wdev.GroupKeyWepStatus = WepStatus;

				/* Initiate some related information */
				pEntry->RSNE_Len[0] = 0;
				pEntry->RSNE_Len[1] = 0;
				NdisZeroMemory(pEntry->RSNE_Content[0], MAX_LEN_OF_RSNIE);
				NdisZeroMemory(pEntry->RSNE_Content[1], MAX_LEN_OF_RSNIE);

				/* Pointer to the first RSNIE context */
				rsnielen_cur_p = &pEntry->RSNE_Len[0];
				pRsnIe = pEntry->RSNE_Content[0];

				/* Pointer to the secondary RSNIE context */
				rsnielen_ex_cur_p = &pEntry->RSNE_Len[1];
				pRsnIe_ex = pEntry->RSNE_Content[1];

				/* Decide whether the authentication mode is WPA1-WPA2 mixed mode */
				if ((AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
					(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
					bMixRsnIe = TRUE;
				break;
			}
#endif /* CONFIG_AP_SUPPORT */

		} while (FALSE);

		/* indicate primary RSNIE as WPA or WPA2*/
		if ((AuthMode == Ndis802_11AuthModeWPA) ||
			(AuthMode == Ndis802_11AuthModeWPAPSK) ||
			(AuthMode == Ndis802_11AuthModeWPANone) ||
			(AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
			(AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
			PrimaryRsnie = WpaIe;
#ifdef WAPI_SUPPORT
		else if ((AuthMode == Ndis802_11AuthModeWAICERT) ||
				 (AuthMode == Ndis802_11AuthModeWAIPSK))
			PrimaryRsnie = WapiIe;
#endif /* WAPI_SUPPORT */
		else
			PrimaryRsnie = Wpa2Ie;

#ifdef WAPI_SUPPORT
		if (PrimaryRsnie == WapiIe)
			RTMPInsertWapiIe(AuthMode, WepStatus, pRsnIe, &p_offset);
		else
#endif /* WAPI_SUPPORT */
		{
			/* Build the primary RSNIE*/
			/* 1. insert cipher suite*/
			RTMPMakeRsnIeCipher(pAd, PrimaryRsnie, WepStatus,
#ifdef DOT11_SAE_SUPPORT
						AuthMode,
#endif
						apidx, bMixCipher, FlexibleCipher, pRsnIe, &p_offset);

			/* 2. insert AKM*/
			RTMPMakeRsnIeAKM(pAd, PrimaryRsnie, AuthMode, apidx, pRsnIe, &p_offset);

			/* 3. insert capability*/
			RTMPMakeRsnIeCap(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);

			/* 4. Insert PMKID */
			RTMPInsertRsnIePMKID(pAd, PrimaryRsnie, AuthMode, apidx, pEntry, pRsnIe, &p_offset);
#ifdef DOT11W_PMF_SUPPORT
			/* 5. Insert Group Management Cipher*/
			PMF_MakeRsnIeGMgmtCipher(pAd, PrimaryRsnie, apidx, pRsnIe, &p_offset);
#endif /* DOT11W_PMF_SUPPORT */
		}

		/* 4. update the RSNIE length*/
		if (rsnielen_cur_p == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s: rsnielen_cur_p == NULL!\n", __func__));
			return;
		}

		*rsnielen_cur_p = p_offset;

		hex_dump("The primary RSNIE", pRsnIe, (*rsnielen_cur_p));

#ifdef CONFIG_AP_SUPPORT
		if ((pAd->OpMode == OPMODE_AP)
			) {
			/* if necessary, build the secondary RSNIE*/
			if (bMixRsnIe) {
				/* 1. insert cipher suite*/
				RTMPMakeRsnIeCipher(pAd, Wpa2Ie, WepStatus,
#ifdef DOT11_SAE_SUPPORT
							AuthMode,
#endif
							apidx, bMixCipher, FlexibleCipher,
							pRsnIe_ex, &s_offset);

				/* 2. insert AKM*/
				RTMPMakeRsnIeAKM(pAd, Wpa2Ie, AuthMode, apidx, pRsnIe_ex, &s_offset);

				/* 3. insert capability*/
				RTMPMakeRsnIeCap(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);

#ifdef DOT11W_PMF_SUPPORT
				/* 4. Insert PMKID */
				RTMPInsertRsnIeZeroPMKID(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);

				/* 5. Insert Group Management Cipher*/
				PMF_MakeRsnIeGMgmtCipher(pAd, Wpa2Ie, apidx, pRsnIe_ex, &s_offset);
#endif /* DOT11W_PMF_SUPPORT */

				/* Update the RSNIE length*/
				*rsnielen_ex_cur_p = s_offset;
				hex_dump("The secondary RSNIE", pRsnIe_ex, (*rsnielen_ex_cur_p));
			}
		}
#endif /* CONFIG_AP_SUPPORT */

	}
#endif

/*
    ==========================================================================
    Description:
		Check whether the received frame is EAP frame.

	Arguments:
		pAd				-	pointer to our pAdapter context
		pEntry			-	pointer to active entry
		pData			-	the received frame
		DataByteCount 	-	the received frame's length
		wdev_idx	-	indicate the interface index

    Return:
         TRUE 			-	This frame is EAP frame
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPCheckWPAframe(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR *pData,
	IN ULONG DataByteCount,
	IN UCHAR wdev_idx,
	IN BOOLEAN eth_frame)
{
	ULONG  min_len = (LENGTH_802_1_H + LENGTH_EAPOL_H);
#ifdef DBG
	ULONG Body_len;
#endif /* DBG */
	
	BOOLEAN Cancelled;

	ASSERT(wdev_idx <= WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX)
		return FALSE;
	do
	{
		if (eth_frame) {
			min_len = (LENGTH_802_3 + LENGTH_EAPOL_H);
			break;
		}

	} while (FALSE);

	if(DataByteCount < min_len)
		return FALSE;

	/* Skip LLC or ETH header */
	if (eth_frame == TRUE) {
		pData += LENGTH_802_3;
	} else {
		/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
		if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
		{
			pData += 6;
		}
	}

	/* Skip 2-bytes EAPoL type */
	if (NdisEqualMemory(EAPOL, pData, 2))
		pData += 2;
	else
		return FALSE;

	switch (*(pData+1))
	{
		case EAPPacket:
#ifdef DBG
			Body_len = (*(pData+2)<<8) | (*(pData+3));
#endif /* DBG */
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef IDS_SUPPORT
				if((*(pData+4)) == EAP_CODE_REQUEST)
					pAd->ApCfg.RcvdEapReqCount ++;
#endif /* IDS_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("Receive EAP-Packet frame, TYPE = 0, Length = %ld\n", Body_len));
			break;
		case EAPOLStart:
			DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Start frame, TYPE = 1 \n"));
			if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Cancel the EnqueueEapolStartTimerRunning \n"));
				RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			}
			break;
		case EAPOLLogoff:
			DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLLogoff frame, TYPE = 2 \n"));
			break;
		case EAPOLKey:
#ifdef DBG
			Body_len = (*(pData+2)<<8) | (*(pData+3));
			DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOL-Key frame, TYPE = 3, Length = %ld\n", Body_len));
#endif /* DBG */
			break;
		case EAPOLASFAlert:
			DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPOLASFAlert frame, TYPE = 4 \n"));
			break;
		default:
			return FALSE;
	}

	return TRUE;
}


/*
    ==========================================================================
    Description:
		Report the EAP message type

	Arguments:
		msg		-	EAPOL_PAIR_MSG_1
					EAPOL_PAIR_MSG_2
					EAPOL_PAIR_MSG_3
					EAPOL_PAIR_MSG_4
					EAPOL_GROUP_MSG_1
					EAPOL_GROUP_MSG_2

    Return:
         message type string

    ==========================================================================
*/
RTMP_STRING *GetEapolMsgType(CHAR msg)
{
    if(msg == EAPOL_PAIR_MSG_1)
        return "Pairwise Message 1";
    else if(msg == EAPOL_PAIR_MSG_2)
        return "Pairwise Message 2";
	else if(msg == EAPOL_PAIR_MSG_3)
        return "Pairwise Message 3";
	else if(msg == EAPOL_PAIR_MSG_4)
        return "Pairwise Message 4";
	else if(msg == EAPOL_GROUP_MSG_1)
        return "Group Message 1";
	else if(msg == EAPOL_GROUP_MSG_2)
        return "Group Message 2";
    else
    	return "Invalid Message";
}


/*
    ========================================================================

    Routine Description:
    Check Sanity RSN IE of EAPoL message

    Arguments:

    Return Value:


    ========================================================================
*/

/*
    ========================================================================

    Routine Description:
    Parse KEYDATA field.  KEYDATA[] May contain 2 RSN IE and optionally GTK.
    GTK  is encaptulated in KDE format at  p.83 802.11i D10

    Arguments:

    Return Value:

    Note:
        802.11i D10

    ========================================================================
*/
BOOLEAN RTMPParseEapolKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN	UCHAR			GroupKeyIndex,
	IN	UCHAR			MsgType,
	IN	BOOLEAN			bWPA2,
	IN  MAC_TABLE_ENTRY *pEntry)
{
    PUCHAR              pMyKeyData = pKeyData;
    UCHAR               KeyDataLength = KeyDataLen;
	UCHAR				GTK[MAX_LEN_GTK];
    UCHAR               GTKLEN = 0;
	UCHAR				DefaultIdx = 0;
	UCHAR				skip_offset = 0;


	NdisZeroMemory(GTK, MAX_LEN_GTK);

	/* Verify The RSN IE contained in pairewise_msg_2 && pairewise_msg_3 and skip it*/
	if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_3)
    {
		{
			if (bWPA2 && MsgType == EAPOL_PAIR_MSG_3)
			{
				/*WpaShowAllsuite(pMyKeyData, skip_offset);*/

				/* skip RSN IE*/
				pMyKeyData += skip_offset;
				KeyDataLength -= skip_offset;
				DBGPRINT(RT_DEBUG_TRACE, ("RTMPParseEapolKeyData ==> WPA2/WPA2PSK RSN IE matched in Msg 3, Length(%d) \n", skip_offset));
			}
			else
				return TRUE;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE,("RTMPParseEapolKeyData ==> KeyDataLength %d without RSN_IE \n", KeyDataLength));
	/*hex_dump("remain data", pMyKeyData, KeyDataLength);*/

#ifdef DOT11R_FT_SUPPORT
	if (IS_FT_RSN_STA(pEntry))
	{
		PEID_STRUCT         pEid;

		pEid = (PEID_STRUCT)pMyKeyData;
		if (pEid->Eid == IE_FT_MDIE)
		{
			/* Skip MDIE of FT*/
			/* todo - verify it*/
			pMyKeyData += (pEid->Len + 2);
			KeyDataLength -= (pEid->Len + 2);
		}
	}
#endif /* DOT11R_FT_SUPPORT */

	/* Parse KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/
	if (bWPA2 && (MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1))
	{
		PEID_STRUCT     pEid;

		pEid = (PEID_STRUCT) pMyKeyData;
		skip_offset = 0;
		while ((skip_offset + 2 + pEid->Len) <= KeyDataLength)
		{
			switch(pEid->Eid)
			{
				case WPA_KDE_TYPE:
					{
						PKDE_HDR	pKDE;

						pKDE = (PKDE_HDR)pEid;
						if (NdisEqualMemory(pKDE->OUI, OUI_WPA2, 3))
    					{
							if (pKDE->DataType == KDE_GTK)
							{
								PGTK_KDE pKdeGtk;

								pKdeGtk = (PGTK_KDE) &pKDE->octet[0];
								DefaultIdx = pKdeGtk->Kid;

								/* Get GTK length - refer to IEEE 802.11i-2004 p.82 */
								GTKLEN = pKDE->Len -6;
								if (GTKLEN < LEN_WEP64)
								{
									DBGPRINT(RT_DEBUG_ERROR, ("ERROR: GTK Key length is too short (%d) \n", GTKLEN));
        							return FALSE;
								}
								NdisMoveMemory(GTK, pKdeGtk->GTK, GTKLEN);
								DBGPRINT(RT_DEBUG_TRACE, ("GTK in KDE format ,DefaultKeyID=%d, KeyLen=%d \n", DefaultIdx, GTKLEN));
    						}
#ifdef DOT11W_PMF_SUPPORT
							else if (pKDE->DataType == KDE_IGTK
                                                                && (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
    						{
								if (PMF_ExtractIGTKKDE(
									pAd, &pKDE->octet[0], pKDE->Len - 4
#ifdef APCLI_DOT11W_PMF_SUPPORT
									, pEntry
#endif /* APCLI_DOT11W_PMF_SUPPORT */
									) == FALSE)
	        						return FALSE;
    						}
#endif /* DOT11W_PMF_SUPPORT */
						}
					}
					break;
			}
			skip_offset = skip_offset + 2 + pEid->Len;
	        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
		}

		/* skip KDE Info*/
		pMyKeyData += skip_offset;
		KeyDataLength -= skip_offset;
	}
	else if (!bWPA2 && MsgType == EAPOL_GROUP_MSG_1)
	{
		DefaultIdx = GroupKeyIndex;
		GTKLEN = KeyDataLength;
		NdisMoveMemory(GTK, pMyKeyData, KeyDataLength);
		DBGPRINT(RT_DEBUG_TRACE, ("GTK without KDE, DefaultKeyID=%d, KeyLen=%d \n", DefaultIdx, GTKLEN));
	}

	/* Sanity check - shared key index must be 0 ~ 3*/
	if (DefaultIdx > 3)
    {
     	DBGPRINT(RT_DEBUG_ERROR, ("ERROR: GTK Key index(%d) is invalid in %s %s \n", DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
        return FALSE;
    }

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_RTMP) && IS_ENTRY_APCLI(pEntry))
		{
			/* Prevent the GTK reinstall key attack */
			if (pEntry->LastGroupKeyId != DefaultIdx ||
				!NdisEqualMemory(pEntry->LastGTK, GTK, MAX_LEN_GTK)) {
				/* Set Group key material, TxMic and RxMic for AP-Client*/
				if (!APCliInstallSharedKey(pAd, GTK, GTKLEN, DefaultIdx, pEntry))
				{
					return FALSE;
				}
				pEntry->LastGroupKeyId = DefaultIdx;
				NdisMoveMemory(pEntry->LastGTK, GTK, MAX_LEN_GTK);
				pEntry->AllowUpdateRSC = TRUE;
			} else {
				DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : the Group reinstall attack, skip install key\n",
					__func__));
			}
		}
#ifdef MT_MAC
        else if ((pAd->chipCap.hif_type == HIF_MT) && IS_ENTRY_APCLI(pEntry))
        {
            UCHAR	IfIdx;
            APCLI_STRUCT *pApcli_entry;

            IfIdx = pEntry->func_tb_idx;
            ASSERT((IfIdx < MAX_APCLI_NUM));

            pApcli_entry = &pAd->ApCfg.ApCliTab[IfIdx];

			/* Prevent the GTK reinstall key attack */
			if (pEntry->LastGroupKeyId != DefaultIdx ||
				!NdisEqualMemory(pEntry->LastGTK, GTK, MAX_LEN_GTK)) {
				/* Set Group key material, TxMic and RxMic for AP-Client*/
				WPAInstallSharedKey(pAd,
					pApcli_entry->GroupCipher,
#ifdef MULTI_APCLI_SUPPORT
					pEntry->func_tb_idx,
#else /* MULTI_APCLI_SUPPORT */
					BSS0,
#endif /* !MULTI_APCLI_SUPPORT */
					DefaultIdx,
#ifdef MULTI_APCLI_SUPPORT
					APCLI_MCAST_WCID(IfIdx),
#else /* MULTI_APCLI_SUPPORT */
					APCLI_MCAST_WCID,
#endif /* !MULTI_APCLI_SUPPORT */
					FALSE,
					GTK,
					GTKLEN);
				pEntry->LastGroupKeyId = DefaultIdx;
				NdisMoveMemory(pEntry->LastGTK, GTK, MAX_LEN_GTK);
				pEntry->AllowUpdateRSC = TRUE;
			} else {
				DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : the Group reinstall attack, skip install key\n",
					__func__));
			}
        }
#endif /* MT_MAC */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;

}

/*
	========================================================================

	Routine Description:
		Construct KDE common format
		Its format is below,

		+--------------------+
		| Type (0xdd)		 |  1 octet
		+--------------------+
		| Length			 |	1 octet
		+--------------------+
		| OUI				 |  3 octets
		+--------------------+
		| Data Type			 |	1 octet
		+--------------------+

	Arguments:

	Return Value:

	Note:
		It's defined in IEEE 802.11-2007 Figure 8-25.

	========================================================================
*/
VOID WPA_ConstructKdeHdr(
	IN 	UINT8	data_type,
	IN 	UINT8 	data_len,
	OUT PUCHAR 	pBuf)
{
	PKDE_HDR	pHdr;

	pHdr = (PKDE_HDR)pBuf;

	NdisZeroMemory(pHdr, sizeof(KDE_HDR));

    pHdr->Type = WPA_KDE_TYPE;

	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */
	pHdr->Len = 4 + data_len;

	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;

}


/*
	========================================================================

	Routine Description:
		Construct EAPoL message for WPA handshaking
		Its format is below,

		+--------------------+
		| Protocol Version	 |  1 octet
		+--------------------+
		| Protocol Type		 |	1 octet
		+--------------------+
		| Body Length		 |  2 octets
		+--------------------+
		| Descriptor Type	 |	1 octet
		+--------------------+
		| Key Information    |	2 octets
		+--------------------+
		| Key Length	     |  1 octet
		+--------------------+
		| Key Repaly Counter |	8 octets
		+--------------------+
		| Key Nonce		     |  32 octets
		+--------------------+
		| Key IV			 |  16 octets
		+--------------------+
		| Key RSC			 |  8 octets
		+--------------------+
		| Key ID or Reserved |	8 octets
		+--------------------+
		| Key MIC			 |	16 octets
		+--------------------+
		| Key Data Length	 |	2 octets
		+--------------------+
		| Key Data			 |	n octets
		+--------------------+


	Arguments:
		pAd			Pointer	to our adapter

	Return Value:
		None

	Note:

	========================================================================
*/
VOID	ConstructEapolMsg(
	IN 	PMAC_TABLE_ENTRY	pEntry,
    IN 	UCHAR				GroupKeyWepStatus,
    IN 	UCHAR				MsgType,
    IN	UCHAR				DefaultKeyIdx,
	IN 	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
    OUT PEAPOL_PACKET       pMsg)
{
	BOOLEAN	bWPA2 = FALSE;
	UCHAR	KeyDescVer;

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef DOT11_SAE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
	)
		bWPA2 = TRUE;

    /* Init Packet and Fill header    */
    pMsg->ProVer = EAPOL_VER;
    pMsg->ProType = EAPOLKey;

	/* Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field*/
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);

	/* Fill in EAPoL descriptor*/
	if (bWPA2)
		pMsg->KeyDesc.Type = WPA2_KEY_DESC;
	else
		pMsg->KeyDesc.Type = WPA1_KEY_DESC;

#ifdef DOT11_SAE_SUPPORT
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
		KeyDescVer = KEY_DESC_NOT_DEFINED;
	else
#endif
#ifdef CONFIG_OWE_SUPPORT
	if (pEntry->AuthMode == Ndis802_11AuthModeOWE)
		KeyDescVer = KEY_DESC_NOT_DEFINED;
	else
#endif
	/* Key Descriptor Version (bits 0-2) specifies the key descriptor version type*/
#ifdef DOT11R_FT_SUPPORT
	if (IS_FT_RSN_STA(pEntry))
	{
		/* Todo-AlbertY : use AKM to detemine which Descriptor version is used */

		/* All EAPOL-Key frames to and from a STA when the negotiated */
		/* AKM is 00-0F-AC:3 or 00-0F-AC:4.*/
		KeyDescVer = KEY_DESC_EXT;
	}
	else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
        if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_USE_SHA256))
        {
		/* All EAPOL-Key frames to and from a STA when the negotiated */
		/* AKM is 00-0F-AC:3 or 00-0F-AC:4.*/
		KeyDescVer = KEY_DESC_EXT;
	}
	else
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
	{
		printk ("OSEN use sha256\n");
		//KeyDescVer = KEY_DESC_EXT;		
		KeyDescVer = KEY_DESC_OSEN; 
	}
	else
#endif /* CONFIG_HOTSPOT_R2 */
	{
		/* Fill in Key information, refer to IEEE Std 802.11i-2004 page 78 */
		/* When either the pairwise or the group cipher is AES, the KEY_DESC_AES shall be used.*/
		KeyDescVer = (((pEntry->WepStatus == Ndis802_11AESEnable) ||
		        		(GroupKeyWepStatus == Ndis802_11AESEnable)) ? (KEY_DESC_AES) : (KEY_DESC_TKIP));
	}

	pMsg->KeyDesc.KeyInfo.KeyDescVer = KeyDescVer;

	/* Specify Key Type as Group(0) or Pairwise(1)*/
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyType = GROUPKEY;
	else
		pMsg->KeyDesc.KeyInfo.KeyType = PAIRWISEKEY;

	/* Specify Key Index, only group_msg1_WPA1*/
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyIndex = DefaultKeyIdx;

	if (MsgType == EAPOL_PAIR_MSG_3)
		pMsg->KeyDesc.KeyInfo.Install = 1;

	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyAck = 1;

	if (MsgType != EAPOL_PAIR_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyMic = 1;

	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) ||
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
    {
       	pMsg->KeyDesc.KeyInfo.Secure = 1;
    }

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3) ||
		(MsgType == EAPOL_GROUP_MSG_1)))
    {
        pMsg->KeyDesc.KeyInfo.EKD_DL = 1;
    }

	/* key Information element has done. */
	*(USHORT *)(&pMsg->KeyDesc.KeyInfo) = cpu2le16(*(USHORT *)(&pMsg->KeyDesc.KeyInfo));

	/* Fill in Key Length*/
	if (bWPA2)
	{
		/* In WPA2 mode, the field indicates the length of pairwise key cipher, */
		/* so only pairwise_msg_1 and pairwise_msg_3 need to fill. */
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
			pMsg->KeyDesc.KeyLength[1] = ((pEntry->WepStatus == Ndis802_11TKIPEnable) ? LEN_TKIP_TK : LEN_AES_TK);
	}
	else if (!bWPA2)
	{
		if (MsgType >= EAPOL_GROUP_MSG_1)
		{
			/* the length of group key cipher*/
			pMsg->KeyDesc.KeyLength[1] = ((GroupKeyWepStatus == Ndis802_11TKIPEnable) ? LEN_TKIP_GTK : LEN_AES_GTK);
		}
		else
		{
			/* the length of pairwise key cipher*/
			pMsg->KeyDesc.KeyLength[1] = ((pEntry->WepStatus == Ndis802_11TKIPEnable) ? LEN_TKIP_TK : LEN_AES_TK);
		}
	}

 	/* Fill in replay counter        		*/
    NdisMoveMemory(pMsg->KeyDesc.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);

	/* Fill Key Nonce field		  */
	/* ANonce : pairwise_msg1 & pairwise_msg3*/
	/* SNonce : pairwise_msg2*/
	/* GNonce : group_msg1_wpa1	*/
	if ((MsgType <= EAPOL_PAIR_MSG_3) || ((!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))))
    	NdisMoveMemory(pMsg->KeyDesc.KeyNonce, KeyNonce, LEN_KEY_DESC_NONCE);

	/* Fill key IV - WPA2 as 0, WPA1 as random*/
	if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
	{
		/* Suggest IV be random number plus some number,*/
		NdisMoveMemory(pMsg->KeyDesc.KeyIv, &KeyNonce[16], LEN_KEY_DESC_IV);
        pMsg->KeyDesc.KeyIv[15] += 2;
	}

    /* Fill Key RSC field        */
    /* It contains the RSC for the GTK being installed.*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
	{
        NdisMoveMemory(pMsg->KeyDesc.KeyRsc, TxRSC, 6);
	}

	/* Clear Key MIC field for MIC calculation later   */
    NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);

	ConstructEapolKeyData(pEntry,
						  GroupKeyWepStatus,
						  KeyDescVer,
						  MsgType,
						  DefaultKeyIdx,
						  GTK,
						  RSNIE,
						  RSNIE_Len,
						  pMsg);

	/* Calculate MIC and fill in KeyMic Field except Pairwise Msg 1.*/
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		CalculateMIC(KeyDescVer, pEntry->PTK,
#ifdef CONFIG_OWE_SUPPORT
			pEntry->AuthMode, pEntry->key_deri_alg,
#endif
			pMsg);
	}

	DBGPRINT(RT_DEBUG_INFO, ("===> ConstructEapolMsg for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
	DBGPRINT(RT_DEBUG_INFO, ("	     Body length = %d \n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	DBGPRINT(RT_DEBUG_INFO, ("	     Key length  = %d \n", CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyLength)));


}

/*
	========================================================================

	Routine Description:
		Construct the Key Data field of EAPoL message

	Arguments:
		pAd			Pointer	to our adapter
		Elem		Message body

	Return Value:
		None

	Note:

	========================================================================
*/
VOID ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN 	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	OUT PEAPOL_PACKET   pMsg)
{
	UCHAR		*mpool = NULL, *Key_Data, *eGTK;
	ULONG		data_offset;
	BOOLEAN		bWPA2Capable = FALSE;
	BOOLEAN		GTK_Included = FALSE;
#ifdef DOT11R_FT_SUPPORT
	PRTMP_ADAPTER	pAd = pEntry->pAd;
#endif /* DOT11R_FT_SUPPORT */

	/* Choose WPA2 or not*/
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) ||
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef DOT11_SAE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
		|| (pEntry->AuthMode == Ndis802_11AuthModeOWE)
#endif
		)
		bWPA2Capable = TRUE;

	if (MsgType == EAPOL_PAIR_MSG_1 ||
		MsgType == EAPOL_PAIR_MSG_4 ||
		MsgType == EAPOL_GROUP_MSG_2)
		return;

	/* allocate memory pool*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

    if (mpool == NULL)
		return;

	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);

	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, 0);
	data_offset = 0;

	/* Encapsulate RSNIE in pairwise_msg2 & pairwise_msg3		*/
	if (RSNIE_LEN && ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		PUINT8	pmkid_ptr = NULL;
		UINT8 	pmkid_len = 0;

#ifdef DOT11R_FT_SUPPORT
		if (IS_FT_RSN_STA(pEntry))
		{
			pmkid_ptr = pEntry->FT_PMK_R1_NAME;
			pmkid_len = LEN_PMK_NAME;
		}
#endif /* DOT11R_FT_SUPPORT */

		RTMPInsertRSNIE(&Key_Data[data_offset],
						&data_offset,
						RSNIE,
						RSNIE_LEN,
						pmkid_ptr,
						pmkid_len);
	}

#ifdef DOT11R_FT_SUPPORT
	/* Encapsulate MDIE if FT is enabled*/
	if (IS_FT_RSN_STA(pEntry) && bWPA2Capable &&
		((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		/*	The MDIE shall be the same as those provided in
		 	the AP's (Re)association Response frame. */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			NdisMoveMemory(&Key_Data[data_offset], pEntry->InitialMDIE, 5);
			data_offset += 5;
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* DOT11R_FT_SUPPORT */

	/* Encapsulate GTK 		*/
	/* Only for pairwise_msg3_WPA2 and group_msg1*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable) || (MsgType == EAPOL_GROUP_MSG_1))
	{
		UINT8	gtk_len;

		/* Decide the GTK length */
		if (GroupKeyWepStatus == Ndis802_11AESEnable)
			gtk_len = LEN_AES_GTK;
		else
			gtk_len = LEN_TKIP_GTK;

		/* Insert GTK KDE format in WAP2 mode */
		if (bWPA2Capable)
		{
			/* Construct the common KDE format */
			WPA_ConstructKdeHdr(KDE_GTK, 2 + gtk_len, &Key_Data[data_offset]);
			data_offset += sizeof(KDE_HDR);

			/* GTK KDE format - 802.11i-2004  Figure-43x*/
	        Key_Data[data_offset] = (DefaultKeyIdx & 0x03);
	        Key_Data[data_offset + 1] = 0x00;	/* Reserved Byte*/
	        data_offset += 2;
		}

		/* Fill in GTK */
		NdisMoveMemory(&Key_Data[data_offset], GTK, gtk_len);
		data_offset += gtk_len;

#ifdef DOT11W_PMF_SUPPORT
		/* Insert IGTK KDE to Key_Data field */
		if ((bWPA2Capable)
                        && (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
		{
 			DBGPRINT(RT_DEBUG_WARN, ("[PMF]:%s - Insert IGTK\n", __FUNCTION__));
			PMF_InsertIGTKKDE(pEntry->pAd, pEntry->func_tb_idx, &Key_Data[data_offset], &data_offset);
		}
#endif /* DOT11W_PMF_SUPPORT */

		GTK_Included = TRUE;
	}

#ifdef DOT11R_FT_SUPPORT
	/* Encapsulate the related IE of FT when FT is enabled */
	if (IS_FT_RSN_STA(pEntry) && bWPA2Capable &&
		((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		/*	Encapsulate FTIE. The MDIE shall be the same
			as those provided in the AP's (Re)association Response frame. */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			NdisMoveMemory(&Key_Data[data_offset], pEntry->InitialFTIE, pEntry->InitialFTIE_Len);
			data_offset += pEntry->InitialFTIE_Len;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable)
			{
				/* Encapsulate TIE[ReassociationDeadline] */
				FT_InsertTimeoutIntervalIE(pAd,
									   &Key_Data[data_offset],
									   (PULONG)&data_offset,
									   REASSOC_DEADLINE_INTERVAL,
									   FT_REASSOC_DEADLINE);

				/* Encapsulate TIM[KeyLifetime] */
				FT_InsertTimeoutIntervalIE(pAd,
							   &Key_Data[data_offset],
							   (PULONG)&data_offset,
							   KEY_LIFETIME_INTERVAL,
							   (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod/OS_HZ));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
#endif /* DOT11R_FT_SUPPORT */


	/* If the Encrypted Key Data subfield (of the Key Information field)
	   is set, the entire Key Data field shall be encrypted. */
	/* This whole key-data field shall be encrypted if a GTK is included.*/
	/* Encrypt the data material in key data field with KEK*/
	if (GTK_Included)
	{
		/*hex_dump("GTK_Included", Key_Data, data_offset);*/

		if (
#ifdef DOT11R_FT_SUPPORT
			(keyDescVer == KEY_DESC_EXT) ||
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
                        (keyDescVer == KEY_DESC_EXT) ||
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
            (keyDescVer == KEY_DESC_OSEN) ||
#endif /* CONFIG_HOTSPOT_R2 */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
			(keyDescVer == KEY_DESC_NOT_DEFINED) ||
#endif
			(keyDescVer == KEY_DESC_AES))
		{
			UCHAR 	remainder = 0;
			UCHAR	pad_len = 0;
			UINT	wrap_len =0;

			/* Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, */
			/* shall be used to encrypt the Key Data field using the KEK field from */
			/* the derived PTK.*/

			/* If the Key Data field uses the NIST AES key wrap, then the Key Data field */
			/* shall be padded before encrypting if the key data length is less than 16 */
			/* octets or if it is not a multiple of 8. The padding consists of appending*/
			/* a single octet 0xdd followed by zero or more 0x00 octets. */
			if ((remainder = data_offset & 0x07) != 0)
			{
				INT		i;

				pad_len = (8 - remainder);
				Key_Data[data_offset] = 0xDD;
				for (i = 1; i < pad_len; i++)
					Key_Data[data_offset + i] = 0;

				data_offset += pad_len;
			}

			AES_Key_Wrap(Key_Data, (UINT) data_offset,
						 &pEntry->PTK[LEN_PTK_KCK], LEN_PTK_KEK,
						 eGTK, &wrap_len);
			data_offset = wrap_len;

		}
		else
		{
			TKIP_GTK_KEY_WRAP(&pEntry->PTK[LEN_PTK_KCK],
								pMsg->KeyDesc.KeyIv,
								Key_Data,
								data_offset,
								eGTK);
		}

		NdisMoveMemory(pMsg->KeyDesc.KeyData, eGTK, data_offset);
	}
	else
	{
		NdisMoveMemory(pMsg->KeyDesc.KeyData, Key_Data, data_offset);
	}

	/* Update key data length field and total body length*/
	SET_UINT16_TO_ARRARY(pMsg->KeyDesc.KeyDataLen, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);

	os_free_mem(NULL, mpool);

}

/*
	========================================================================

	Routine Description:
		Calcaulate MIC. It is used during 4-ways handsharking.

	Arguments:
		pAd				-	pointer to our pAdapter context
    	PeerWepStatus	-	indicate the encryption type

	Return Value:

	Note:
	 The EAPOL-Key MIC is a MIC of the EAPOL-Key frames,
	 from and including the EAPOL protocol version field
	 to and including the Key Data field, calculated with
	 the Key MIC field set to 0.

	========================================================================
*/
VOID CalculateMIC(
	IN UCHAR KeyDescVer,
	IN UCHAR *PTK,
#ifdef CONFIG_OWE_SUPPORT
	IN NDIS_802_11_AUTHENTICATION_MODE StaAuthMode,
	IN UINT8 key_deri_alg,
#endif
	OUT PEAPOL_PACKET pMsg)
{
    UCHAR   *OutBuffer;
	ULONG	FrameLen = 0;
	UCHAR	mic[LEN_KEY_DESC_MIC];
	UCHAR	digest[80];
#ifdef CONFIG_OWE_SUPPORT
	UINT8 mic_len = LEN_KEY_DESC_MIC;
#endif

	/* allocate memory for MIC calculation*/
	os_alloc_mem(NULL, (PUCHAR *)&OutBuffer, 512);

    if (OutBuffer == NULL)
    {
		DBGPRINT(RT_DEBUG_ERROR, ("!!!CalculateMIC: no memory!!!\n"));
		return;
    }

	/* make a frame for calculating MIC.*/
    MakeOutgoingFrame(OutBuffer,            	&FrameLen,
                      CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4,  	pMsg,
                      END_OF_ARGS);

	NdisZeroMemory(mic, sizeof(mic));

	/* Calculate MIC*/
    if (KeyDescVer == KEY_DESC_AES)
 	{
		RT_HMAC_SHA1(PTK, LEN_PTK_KCK, OutBuffer,  FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
	}
	else if (KeyDescVer == KEY_DESC_TKIP)
	{
		RT_HMAC_MD5(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, MD5_DIGEST_SIZE);
	}
#ifdef CONFIG_OWE_SUPPORT
	else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		(StaAuthMode == Ndis802_11AuthModeOWE) &&
		(key_deri_alg == SEC_KEY_DERI_SHA256)) {
		RT_HMAC_SHA256(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC);
		mic_len = LEN_KEY_DESC_MIC;
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		(StaAuthMode == Ndis802_11AuthModeOWE) &&
		(key_deri_alg == SEC_KEY_DERI_SHA384)) {
		/*TODO: OWE SHA521*/
		RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC_SHA384);
		mic_len = LEN_KEY_DESC_MIC_SHA384;
	}
#endif
	else if ((KeyDescVer == KEY_DESC_EXT) || (KeyDescVer == KEY_DESC_OSEN)
#ifdef DOT11_SAE_SUPPORT
		|| (KeyDescVer == KEY_DESC_NOT_DEFINED)
#endif
		) {
		UINT	mlen = AES_KEY128_LENGTH;
		AES_CMAC(OutBuffer, FrameLen, PTK, LEN_PTK_KCK, mic, &mlen);
	}

	/* store the calculated MIC*/
#ifndef CONFIG_OWE_SUPPORT
	NdisMoveMemory(pMsg->KeyDesc.KeyMic, mic, LEN_KEY_DESC_MIC);
#else
	NdisMoveMemory(pMsg->KeyDesc.KeyMic, mic, mic_len);
#endif

	os_free_mem(NULL, OutBuffer);
}


/* For TKIP frame, calculate the MIC value */
BOOLEAN rtmp_chk_tkip_mic(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	UCHAR UserPriority = pRxBlk->UserPriority;
	PCIPHER_KEY pWpaKey;
	UCHAR *pDA, *pSA;

	if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1)) {
		pDA = pHeader->Addr3;
		pSA = (PUCHAR)pHeader + sizeof(HEADER_802_11);
	}
	else if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 0)) {
		pDA = pHeader->Addr1;
		pSA = pHeader->Addr3;
	} else if ((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 1)) {
		pDA = pHeader->Addr3;
		pSA = pHeader->Addr2;
	} else {
		/* FrDS = 0; ToDS = 0 => IBSS, Non-AP to Non-AP in BSS */
		pDA = pHeader->Addr1;
		pSA = pHeader->Addr2;
	}

	pWpaKey = &pEntry->PairwiseKey;
	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pDA,
								pSA,
								pWpaKey->RxMic,
								UserPriority,
								DataSize) == FALSE)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("Rx MIC Value error 2\n"));


#ifdef CONFIG_AP_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
		{
			RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
		}
#endif /* CONFIG_AP_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}


UCHAR RTMPExtractKeyIdxFromIVHdr(UCHAR *pIV, UINT8 CipherAlg)
{
	UCHAR keyIdx = 0xFF;

	/* extract the key index from IV header */
	switch (CipherAlg)
	{
		case Ndis802_11WEPEnabled:
		case Ndis802_11TKIPEnable:
		case Ndis802_11AESEnable:
		case Ndis802_11GroupWEP40Enabled:
		case Ndis802_11GroupWEP104Enabled:
			keyIdx = (*(pIV + 3) & 0xc0) >> 6;
			break;

#ifdef WAPI_SUPPORT
		case Ndis802_11EncryptionSMS4Enabled:
			keyIdx = *(pIV) & 0xFF;
			break;
#endif /* WAPI_SUPPORT */
	}

	return keyIdx;

}

CIPHER_KEY *RTMPSwCipherKeySelection(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pIV,
	IN RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry)
{
	PCIPHER_KEY pKey = NULL;
	UCHAR keyIdx = 0;
	UINT8 CipherAlg = Ndis802_11EncryptionDisabled;

	if ((pEntry == NULL) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_STA)) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) ||
		(RX_BLK_TEST_FLAG(pRxBlk, fRX_MESH)))
		return NULL;

	if (pRxBlk->pRxInfo->U2M)
	{
		CipherAlg = pEntry->WepStatus;
	}
	else
	{
	}

	if ((keyIdx = RTMPExtractKeyIdxFromIVHdr(pIV, CipherAlg)) > 3)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Invalid key index(%d) !!!\n",
								  __FUNCTION__, keyIdx));
		return NULL;
	}

	if ((CipherAlg == Ndis802_11WEPEnabled)
		|| (CipherAlg == Ndis802_11GroupWEP40Enabled)
		|| (CipherAlg == Ndis802_11GroupWEP104Enabled))
	{
		pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
	}
	else if ((CipherAlg == Ndis802_11TKIPEnable) ||
  			 (CipherAlg == Ndis802_11AESEnable))
	{
		if (pRxBlk->pRxInfo->U2M)
			pKey = &pEntry->PairwiseKey;
		else {
		    	pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
        }
	}
#ifdef WAPI_SUPPORT
	else if (CipherAlg == Ndis802_11EncryptionSMS4Enabled)
	{
		if (pRxBlk->pRxInfo->U2M)
			pKey = &pEntry->PairwiseKey;
		else
			pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
	}
#endif /* WAPI_SUPPORT */

	return pKey;

}

/*
	========================================================================

	Routine Description:
		Some received frames can't decrypt by Asic, so decrypt them by software.

	Arguments:
		pAd				-	pointer to our pAdapter context
    	PeerWepStatus	-	indicate the encryption type

	Return Value:
		NDIS_STATUS_SUCCESS		-	decryption successful
		NDIS_STATUS_FAILURE		-	decryption failure

	========================================================================
*/
NDIS_STATUS	RTMPSoftDecryptionAction(
	IN	PRTMP_ADAPTER		pAd,
	IN 		PUCHAR			pHdr,
	IN 		UCHAR    		UserPriority,
	IN 		PCIPHER_KEY		pKey,
	INOUT 	PUCHAR			pData,
	INOUT 	UINT16			*DataByteCnt)
{
	switch (pKey->CipherAlg)
    {
		case CIPHER_WEP64:
		case CIPHER_WEP128:
			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(pAd, pKey, pData, &(*DataByteCnt)) == FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt WEP data fails.\n"));
				/* give up this frame*/
				return NDIS_STATUS_FAILURE;
			}
			break;

		case CIPHER_TKIP:
			/* handle TKIP decryption */
			if (RTMPSoftDecryptTKIP(pAd, pHdr, UserPriority,
								pKey, pData, &(*DataByteCnt)) == FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt TKIP data fails.\n"));
				/* give up this frame*/
				return NDIS_STATUS_FAILURE;
			}
			break;

		case CIPHER_AES:
			/* handle AES decryption */
			if (RTMPSoftDecryptCCMP(pAd, pHdr, pKey, pData, &(*DataByteCnt)) == FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt AES data fails.\n"));
				/* give up this frame*/
				return NDIS_STATUS_FAILURE;
        	}
			break;
#ifdef WAPI_SUPPORT
#ifdef SOFT_ENCRYPT
		case CIPHER_SMS4:
			{
				INT	ret;
#ifdef RT_BIG_ENDIAN
				RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif
				if ((ret = RTMPSoftDecryptSMS4(pHdr, FALSE, pKey, pData, &(*DataByteCnt))) != STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("ERROR : SW decrypt SMS4 data fails(%d).\n", ret));
					/* give up this frame*/
					return NDIS_STATUS_FAILURE;
				}
#ifdef RT_BIG_ENDIAN
				RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif
			}
			break;
#endif /* SOFT_ENCRYPT */
#endif /* WAPI_SUPPORT */
		default:
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
			break;
	}

	return NDIS_STATUS_SUCCESS;

}

VOID RTMPSoftConstructIVHdr(
	IN	UCHAR			CipherAlg,
	IN	UCHAR			key_id,
	IN	PUCHAR			pTxIv,
	OUT PUCHAR 			pHdrIv,
	OUT	UINT8			*hdr_iv_len)
{
	*hdr_iv_len = 0;

#ifdef WAPI_SUPPORT
	if (CipherAlg == CIPHER_SMS4)
	{
		/* Construct and insert WPI-SMS4 IV header to MPDU header */
		RTMPConstructWPIIVHdr(key_id, pTxIv, pHdrIv);
		*hdr_iv_len = LEN_WPI_IV_HDR;
	}
	else
#endif /* WAPI_SUPPORT */
	if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128))
	{
		/* Construct and insert 4-bytes WEP IV header to MPDU header */
		RTMPConstructWEPIVHdr(key_id, pTxIv, pHdrIv);
		*hdr_iv_len = LEN_WEP_IV_HDR;
	}
	else if (CipherAlg == CIPHER_TKIP)
		;
	else if (CipherAlg == CIPHER_AES)
	{
		/* Construct and insert 8-bytes CCMP header to MPDU header */
		RTMPConstructCCMPHdr(key_id, pTxIv, pHdrIv);
		*hdr_iv_len = LEN_CCMP_HDR;
	}

}

VOID RTMPSoftEncryptionAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			CipherAlg,
	IN	PUCHAR			pHdr,
	IN	PUCHAR			pSrcBufData,
	IN	UINT32			SrcBufLen,
	IN	UCHAR			KeyIdx,
	IN	PCIPHER_KEY		pKey,
	OUT	UINT8			*ext_len)
{
	*ext_len = 0;

#ifdef WAPI_SUPPORT
#ifdef SOFT_ENCRYPT
	if (CipherAlg == CIPHER_SMS4)
	{
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif
		/* Encrypt the MPDU data by software*/
		RTMPSoftEncryptSMS4(pHdr,
							pSrcBufData,
							SrcBufLen,
							KeyIdx,
							pKey->Key,
							pKey->TxTsc);
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, pHdr, DIR_READ, FALSE);
#endif
		*ext_len = LEN_WPI_MIC;
	}
	else
#endif /* SOFT_ENCRYPT */
#endif /* WAPI_SUPPORT */
	if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128))
	{
		/* Encrypt the MPDU data by software*/
		RTMPSoftEncryptWEP(pAd,
						   pKey->TxTsc,
						   pKey,
						   pSrcBufData,
						   SrcBufLen);

		*ext_len = LEN_ICV;
	}
	else if (CipherAlg == CIPHER_TKIP)
		;
	else if (CipherAlg == CIPHER_AES)
	{
		/* Encrypt the MPDU data by software*/
		RTMPSoftEncryptCCMP(pAd,
							pHdr,
							pKey->TxTsc,
							pKey->Key,
							pSrcBufData,
							SrcBufLen);

		*ext_len = LEN_CCMP_MIC;
	}

}

PUINT8	WPA_ExtractSuiteFromRSNIE(
		IN 	PUINT8	rsnie,
		IN 	UINT	rsnie_len,
		IN	UINT8	type,
		OUT	UINT8	*count)
{
	PEID_STRUCT pEid;
	INT			len;
	PUINT8		pBuf;
	INT			offset = 0;

	pEid = (PEID_STRUCT)rsnie;
	len = rsnie_len - 2;	/* exclude IE and length*/
	pBuf = (PUINT8)&pEid->Octet[0];

	/* set default value*/
	*count = 0;

	/* Check length*/
	if ((len <= 0) || (pEid->Len != len))
	{
		DBGPRINT_ERR(("%s : The length is invalid\n", __FUNCTION__));
		goto out;
	}

	/* Check WPA or WPA2*/
	if (pEid->Eid == IE_WPA)
	{
		/* Check the length */
		if (len < sizeof(RSNIE))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s : The length is too short for WPA\n", __FUNCTION__));
			goto out;
		}
		else
		{
			PRSNIE	pRsnie;
			UINT16 	u_cnt;

			pRsnie = (PRSNIE)pBuf;
			u_cnt = cpu2le16(pRsnie->ucount);
			offset = sizeof(RSNIE) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset)
		{
				DBGPRINT(RT_DEBUG_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for WPA-RSN \n",
											__FUNCTION__, offset, len));
				goto out;
		}
			else
			{
		/* Get the group cipher*/
		if (type == GROUP_SUITE)
		{
			*count = 1;
			return pRsnie->mcast;
		}
		/* Get the pairwise cipher suite*/
		else if (type == PAIRWISE_SUITE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of pairwise cipher is %d\n",
												__FUNCTION__, u_cnt));
						*count = u_cnt;
			return pRsnie->ucast[0].oui;
		}
			}
		}
	}
	else if (pEid->Eid == IE_RSN)
	{
		if (len < sizeof(RSNIE2))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s : The length is too short for WPA2\n", __FUNCTION__));
			goto out;
		}
		else
		{
			PRSNIE2	pRsnie2;
			UINT16 	u_cnt;

			pRsnie2 = (PRSNIE2)pBuf;
			u_cnt = cpu2le16(pRsnie2->ucount);
			offset = sizeof(RSNIE2) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset)
		{
				DBGPRINT(RT_DEBUG_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for WPA2-RSN \n",
											__FUNCTION__, offset, len));
				goto out;
		}
			else
			{
		/* Get the group cipher*/
		if (type == GROUP_SUITE)
		{
			*count = 1;
					return pRsnie2->mcast;
		}
		/* Get the pairwise cipher suite*/
		else if (type == PAIRWISE_SUITE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of pairwise cipher is %d\n",
										__FUNCTION__, u_cnt));
					*count = u_cnt;
					return pRsnie2->ucast[0].oui;
				}
			}
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Unknown IE (%d)\n", __FUNCTION__, pEid->Eid));
		goto out;
	}

	/* skip group cipher and pairwise cipher suite	*/
	pBuf += offset;
	len -= offset;

	/* Ready to extract the AKM information and its count */
	if (len < sizeof(RSNIE_AUTH))
	{
		DBGPRINT_ERR(("%s : The length of AKM of RSN is too short\n", __FUNCTION__));
		goto out;
	}
	else
	{
		PRSNIE_AUTH	pAkm;
		UINT16 		a_cnt;

		/* pointer to AKM count */
	pAkm = (PRSNIE_AUTH)pBuf;
		a_cnt = cpu2le16(pAkm->acount);
		offset = sizeof(RSNIE_AUTH) + (LEN_OUI_SUITE * (a_cnt - 1));

		if (len < offset)
	{
			DBGPRINT(RT_DEBUG_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for AKM \n",
										__FUNCTION__, offset, len));
			goto out;
	}
		else
		{
			/* Get the AKM suite */
	if (type == AKM_SUITE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : The count of AKM is %d\n",
											__FUNCTION__, a_cnt));
				*count = a_cnt;
		return pAkm->auth[0].oui;
	}
		}
	}

	/* For WPA1, the remaining shall be ignored. */
	if (pEid->Eid == IE_WPA)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : The remaining shall be ignored in WPA mode\n",
									__FUNCTION__));
		goto out;
	}

	/* skip the AKM capability */
	pBuf += offset;
	len -= offset;

	/* Parse the RSN Capabilities */
	if (len < sizeof(RSN_CAPABILITIES))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : The peer RSNIE doesn't include RSN-Cap\n", __FUNCTION__));
		goto out;
	}
	else
	{
		/* Report the content of the RSN capabilities */
		if (type == RSN_CAP_INFO)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s : Extract RSN Capabilities\n", __FUNCTION__));
			*count = 1;
			return pBuf;
		}

		/* skip RSN capability (2-bytes) */
		offset = sizeof(RSN_CAPABILITIES);
		pBuf += offset;
		len -= offset;
	}

	/* Extract PMKID-list field */
	if (len < sizeof(UINT16))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : The peer RSNIE doesn't include PMKID list Count\n", __FUNCTION__));
		goto out;
	}
	else
	{
		UINT16 	p_count;
		PUINT8	pPmkidList = NULL;

		NdisMoveMemory(&p_count, pBuf, sizeof(UINT16));
		p_count = cpu2le16(p_count);

		/* Get count of the PMKID list */
		if (p_count > 0)
		{
			PRSNIE_PMKID 	pRsnPmkid;

			/* the expected length of PMKID-List field */
			offset = sizeof(RSNIE_PMKID) + (LEN_PMKID * (p_count - 1));

			/* sanity check about the length of PMKID-List field */
			if (len < offset)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) in PMKID-field \n",
											__FUNCTION__, offset, len));
				goto out;
			}

			/* pointer to PMKID field */
			pRsnPmkid = (PRSNIE_PMKID)pBuf;
			pPmkidList = pRsnPmkid->pmkid[0].list;

		}
		else
		{
			/* The PMKID field shall be without PMKID-List */
			offset = sizeof(UINT16);
			pPmkidList = NULL;
		}


		/* Extract PMKID list and its count */
		if (type == PMKID_LIST)
		{
			*count = p_count;
			return pPmkidList;
		}

		/* skip the PMKID field */
		pBuf += offset;
		len -= offset;

	}

#ifdef DOT11W_PMF_SUPPORT
        /* Get group mamagement cipher */
	if (len < LEN_OUI_SUITE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("[PMF]%s : The peer RSNIE doesn't include Group_mgmt_cipher_suite\n", __FUNCTION__));
		goto out;
	}
	else
	{
		offset = LEN_OUI_SUITE;

		/* Get Group-Mgmt-Cipher_Suite information */
		if (type == G_MGMT_SUITE)
		{
			*count = 1;
			return pBuf;
		}
		else
		{
			/* skip the Group-Mgmt-Cipher_Suite field */
			pBuf += offset;
			len -= offset;
		}
	}
#endif /* DOT11W_PMF_SUPPORT */

out:
	*count = 0;
	return NULL;

}

VOID WpaShowAllsuite(
	IN 	PUINT8	rsnie,
	IN 	UINT	rsnie_len)
{
	PUINT8 pSuite = NULL;
	UINT8 count;

	hex_dump("RSNIE", rsnie, rsnie_len);

	/* group cipher*/
	if ((pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, GROUP_SUITE, &count)) != NULL)
	{
		hex_dump("group cipher", pSuite, 4*count);
	}

	/* pairwise cipher*/
	if ((pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PAIRWISE_SUITE, &count)) != NULL)
	{
		hex_dump("pairwise cipher", pSuite, 4*count);
	}

	/* AKM*/
	if ((pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, AKM_SUITE, &count)) != NULL)
	{
		hex_dump("AKM suite", pSuite, 4*count);
	}

	/* PMKID*/
	if ((pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PMKID_LIST, &count)) != NULL)
	{
		hex_dump("PMKID", pSuite, LEN_PMKID);
	}

}

VOID RTMPInsertRSNIE(
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len)
{
	PUCHAR	pTmpBuf;
	ULONG 	TempLen = 0;
	UINT8 	extra_len = 0;
	UINT16 	pmk_count = 0;
	UCHAR	ie_num;
	UINT8 	total_len = 0;
    UCHAR	WPA2_OUI[3]={0x00,0x0F,0xAC};

	pTmpBuf = pFrameBuf;

	/* PMKID-List Must larger than 0 and the multiple of 16. */
	if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0))
	{
		extra_len = sizeof(UINT16) + pmkid_len;

		pmk_count = (pmkid_len >> 4);
		pmk_count = cpu2le16(pmk_count);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : no PMKID-List included(%d).\n", __FUNCTION__, pmkid_len));
	}

	if (rsnie_len != 0)
	{
		ie_num = IE_WPA;
		total_len = rsnie_len;

		if (NdisEqualMemory(rsnie_ptr + 2, WPA2_OUI, sizeof(WPA2_OUI)))
		{
			ie_num = IE_RSN;
			total_len += extra_len;
		}

		/* construct RSNIE body */
		MakeOutgoingFrame(pTmpBuf,			&TempLen,
					  	  1,				&ie_num,
					  	  1,				&total_len,
					  	  rsnie_len,		rsnie_ptr,
					  	  END_OF_ARGS);

		pTmpBuf += TempLen;
		*pFrameLen = *pFrameLen + TempLen;

		if (ie_num == IE_RSN)
		{
			/* Insert PMKID-List field */
			if (extra_len > 0)
			{
				MakeOutgoingFrame(pTmpBuf,					&TempLen,
							  	  2,						&pmk_count,
							  	  pmkid_len,				pmkid_ptr,
							  	  END_OF_ARGS);

				pTmpBuf += TempLen;
				*pFrameLen = *pFrameLen + TempLen;
			}
		}
	}

	return;
}


VOID WPAInstallPairwiseKey(
	PRTMP_ADAPTER		pAd,
	UINT8				BssIdx,
	PMAC_TABLE_ENTRY	pEntry,
	BOOLEAN				bAE)
{
    NdisZeroMemory(&pEntry->PairwiseKey, sizeof(CIPHER_KEY));

	/* Assign the pairwise cipher algorithm	*/
	if (pEntry->WepStatus == Ndis802_11TKIPEnable)
		pEntry->PairwiseKey.CipherAlg = CIPHER_TKIP;
	else if (pEntry->WepStatus == Ndis802_11AESEnable)
		pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
#ifdef CONFIG_OWE_SUPPORT
	else if (pEntry->AuthMode == Ndis802_11AuthModeOWE && pEntry->key_deri_alg == SEC_KEY_DERI_SHA256)
		pEntry->PairwiseKey.CipherAlg = CIPHER_AES;
#endif
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : fails (wcid-%d)\n",
					__FUNCTION__, pEntry->wcid));
		return;
	}

	/* Assign key material and its length */
    pEntry->PairwiseKey.KeyLen = LEN_TK;
    NdisMoveMemory(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], LEN_TK);
	if (pEntry->PairwiseKey.CipherAlg == CIPHER_TKIP)
	{
		if (bAE)
		{
		    NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[OFFSET_OF_AP_TKIP_TX_MIC], LEN_TKIP_MIC);
		    NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[OFFSET_OF_AP_TKIP_RX_MIC], LEN_TKIP_MIC);
		}
		else
		{
		    NdisMoveMemory(pEntry->PairwiseKey.TxMic, &pEntry->PTK[OFFSET_OF_STA_TKIP_TX_MIC], LEN_TKIP_MIC);
		    NdisMoveMemory(pEntry->PairwiseKey.RxMic, &pEntry->PTK[OFFSET_OF_STA_TKIP_RX_MIC], LEN_TKIP_MIC);
		}
	}

#ifdef SOFT_ENCRYPT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("===> SW_ENC ON(wcid=%d) \n", pEntry->wcid));
		NdisZeroMemory(pEntry->PairwiseKey.TxTsc, LEN_WPA_TSC);
		NdisZeroMemory(pEntry->PairwiseKey.RxTsc, LEN_WPA_TSC);
	}
	else
#endif /* SOFT_ENCRYPT */
	{
		/* Add Pair-wise key to Asic */
	    AsicAddPairwiseKeyEntry(
	        pAd,
	        (UCHAR)pEntry->wcid,
	        &pEntry->PairwiseKey);

		RTMPSetWcidSecurityInfo(pAd,
								BssIdx,
								0,
								pEntry->PairwiseKey.CipherAlg,
								(UCHAR)pEntry->wcid,
								PAIRWISEKEYTABLE);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			CmdProcAddRemoveKey(pAd, 0, BssIdx, 0, pEntry->wcid, PAIRWISEKEYTABLE, &pEntry->PairwiseKey, pEntry->Addr);
#endif
	}

}

VOID WPAInstallSharedKey(
	PRTMP_ADAPTER		pAd,
	UINT8				GroupCipher,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				Wcid,
	BOOLEAN				bAE,
	PUINT8				pGtk,
	UINT8				GtkLen)
{
	PCIPHER_KEY 	pSharedKey = NULL;

	if (BssIdx >= MAX_MBSSID_NUM(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The BSS-index(%d) is out of range for MBSSID link. \n",
									__FUNCTION__, BssIdx));
		return;
	}

#if (defined(MT_MAC) && defined(APCLI_SUPPORT))
#ifdef MULTI_APCLI_SUPPORT
    if (Wcid == APCLI_MCAST_WCID(BssIdx))
    {
        PAPCLI_STRUCT pApCliEntry = NULL;
        pApCliEntry = &pAd->ApCfg.ApCliTab[BssIdx];
        if (KeyIdx < SHARE_KEY_NUM )		
        	pSharedKey = &pApCliEntry->SharedKey[KeyIdx];
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():%d Over buffer size!\n", __FUNCTION__,__LINE__));
		}			
    }
#else /* MULTI_APCLI_SUPPORT */
    if (Wcid == APCLI_MCAST_WCID)
    {
        PAPCLI_STRUCT pApCliEntry = NULL;
        pApCliEntry = &pAd->ApCfg.ApCliTab[0];//TODO: Carter, find a better way to know index.
        if (KeyIdx < SHARE_KEY_NUM )
       		pSharedKey = &pApCliEntry->SharedKey[KeyIdx];
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():%d Over buffer size!\n", __FUNCTION__,__LINE__));
		}
    }
#endif /* !MULTI_APCLI_SUPPORT */	
    else
#endif
    {
    	UINT32 		u4MaxSharedKeySize = 0;
		UINT32 		u4MaxSharedKeyLen = 0;
		u4MaxSharedKeySize = sizeof(pAd->SharedKey)/sizeof(pAd->SharedKey[0]);
		u4MaxSharedKeyLen = sizeof(pAd->SharedKey[0])/sizeof(pAd->SharedKey[0][0]);
	  
		if((BssIdx >= u4MaxSharedKeySize) || (KeyIdx >= u4MaxSharedKeyLen))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():%d Over buffer size!\n", __FUNCTION__,__LINE__));
		}
		else
	    	pSharedKey = &pAd->SharedKey[BssIdx][KeyIdx];
    }

	if(pSharedKey)
		NdisZeroMemory(pSharedKey, sizeof(CIPHER_KEY));

	/* Set the group cipher */
	if (!pSharedKey)
		return;
	
	if (GroupCipher == Ndis802_11GroupWEP40Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP64;
	else if (GroupCipher == Ndis802_11GroupWEP104Enabled)
		pSharedKey->CipherAlg = CIPHER_WEP128;
	else if (GroupCipher == Ndis802_11TKIPEnable)
		pSharedKey->CipherAlg = CIPHER_TKIP;
	else if (GroupCipher == Ndis802_11AESEnable)
		pSharedKey->CipherAlg = CIPHER_AES;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : fails (IF/ra%d) \n",
										__FUNCTION__, BssIdx));
		return;
	}

	/* Set the key material and its length */
	if (GroupCipher == Ndis802_11GroupWEP40Enabled ||
		GroupCipher == Ndis802_11GroupWEP104Enabled)
	{
		/* Sanity check the length */
		if ((GtkLen != LEN_WEP64) && (GtkLen != LEN_WEP128))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s : (IF/ra%d) WEP key invlaid(%d) \n",
										__FUNCTION__, BssIdx, GtkLen));
			return;
		}

		pSharedKey->KeyLen = GtkLen;
		NdisMoveMemory(pSharedKey->Key, pGtk, GtkLen);
	}
	else
	{
		/* Sanity check the length */
		if (GtkLen < LEN_TK)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s : (IF/ra%d) WPA key invlaid(%d) \n",
										__FUNCTION__, BssIdx, GtkLen));
			return;
		}

		pSharedKey->KeyLen = LEN_TK;
		NdisMoveMemory(pSharedKey->Key, pGtk, LEN_TK);
		if (pSharedKey->CipherAlg == CIPHER_TKIP)
		{
			if (bAE)
			{
				NdisMoveMemory(pSharedKey->TxMic, pGtk + 16, LEN_TKIP_MIC);
				NdisMoveMemory(pSharedKey->RxMic, pGtk + 24, LEN_TKIP_MIC);
			}
			else
			{
				NdisMoveMemory(pSharedKey->TxMic, pGtk + 24, LEN_TKIP_MIC);
				NdisMoveMemory(pSharedKey->RxMic, pGtk + 16, LEN_TKIP_MIC);
			}
		}
	}

	/* Update group key table(0x6C00) and group key mode(0x7000) */
    AsicAddSharedKeyEntry(
				pAd,
				BssIdx,
				KeyIdx,
				pSharedKey);

	/* When Wcid isn't zero, it means that this is a Authenticator Role.
	   Only Authenticator entity needs to set HW IE/EIV table (0x6000)
	   and WCID attribute table (0x6800) for group key. */
	if (Wcid != 0)
	{
		RTMPSetWcidSecurityInfo(pAd,
								BssIdx,
								KeyIdx,
								pSharedKey->CipherAlg,
								Wcid,
								SHAREDKEYTABLE);
	}
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		CmdProcAddRemoveKey(pAd, 0, BssIdx, KeyIdx, Wcid, SHAREDKEYTABLE, pSharedKey, BROADCAST_ADDR);
#endif
}

VOID RTMPSetWcidSecurityInfo(
	PRTMP_ADAPTER		pAd,
	UINT8				BssIdx,
	UINT8				KeyIdx,
	UINT8				CipherAlg,
	UINT8				Wcid,
	UINT8				KeyTabFlag)
{
	UINT32			IV = 0;
	UINT8			IV_KEYID = 0;

	/* Prepare initial IV value */
	if (CipherAlg == CIPHER_WEP64 || CipherAlg == CIPHER_WEP128)
	{
		INT	i;
		UCHAR	TxTsc[LEN_WEP_TSC];

		/* Generate 3-bytes IV randomly for encryption using */
		for(i = 0; i < LEN_WEP_TSC; i++)
			TxTsc[i] = RandomByte(pAd);

		/* Update HW IVEIV table */
		IV_KEYID = (KeyIdx << 6);
		IV = (IV_KEYID << 24) |
			 (TxTsc[2] << 16) |
			 (TxTsc[1] << 8) |
			 (TxTsc[0]);
	}
	else if (CipherAlg == CIPHER_TKIP || CipherAlg == CIPHER_AES)
	{
		/* Set IVEIV as 1 in Asic -
		In IEEE 802.11-2007 8.3.3.4.3 described :
		The PN shall be implemented as a 48-bit monotonically incrementing
		non-negative integer, initialized to 1 when the corresponding
		temporal key is initialized or refreshed. */
		IV_KEYID = (KeyIdx << 6) | 0x20;
		IV = (IV_KEYID << 24) | 1;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Unsupport cipher Alg (%d) for Wcid-%d \n",
										__FUNCTION__, CipherAlg, Wcid));
		return;
	}
	/* Update WCID IV/EIV table */
	AsicUpdateWCIDIVEIV(pAd, Wcid, IV, 0);

	/* Update WCID attribute entry */
	AsicUpdateWcidAttributeEntry(pAd,
							BssIdx,
							KeyIdx,
							CipherAlg,
							Wcid,
							KeyTabFlag);

}


#ifdef CONFIG_OWE_SUPPORT


VOID HKDF_expand_sha384(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len)
{
	UCHAR T[SHA384_DIGEST_SIZE];
	UCHAR iter = 1;
	const unsigned char *addr[3];
	UINT len[3];
	UINT pos, clen;

	addr[0] = T;
	len[0] = SHA384_DIGEST_SIZE;
	addr[1] = info;
	len[1] = info_len;
	addr[2] = &iter;
	len[2] = 1;

	RT_HMAC_SHA384_VECTOR(secret, secret_len, 2, &addr[1], &len[1], T, SHA384_DIGEST_SIZE);

	pos = 0;
	for (;;) {
		clen = output_Len - pos;
		if (clen > SHA384_DIGEST_SIZE)
			clen = SHA384_DIGEST_SIZE;
		memcpy(output + pos, T, clen);
		pos += clen;

		if (pos == output_Len)
			break;

		if (iter == 255) {
			os_zero_mem(output, output_Len);
			os_zero_mem(T, SHA384_DIGEST_SIZE);
			return;
		}
		iter++;

		RT_HMAC_SHA384_VECTOR(secret, secret_len, 3, addr, len, T, SHA384_DIGEST_SIZE);
	}

	os_zero_mem(T, SHA384_DIGEST_SIZE);
}

VOID HKDF_expand_sha256(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len)
{
	UCHAR T[SHA256_DIGEST_SIZE];
	UCHAR iter = 1;
	const unsigned char *addr[3];
	UINT len[3];
	UINT pos, clen;

	addr[0] = T;
	len[0] = SHA256_DIGEST_SIZE;
	addr[1] = info;
	len[1] = info_len;
	addr[2] = &iter;
	len[2] = 1;

	RT_HMAC_SHA256_VECTOR(secret, secret_len, 2, &addr[1], &len[1], T, SHA256_DIGEST_SIZE);

	pos = 0;
	for (;;) {
		clen = output_Len - pos;
		if (clen > SHA256_DIGEST_SIZE)
			clen = SHA256_DIGEST_SIZE;
		memcpy(output + pos, T, clen);
		pos += clen;

		if (pos == output_Len)
			break;

		if (iter == 255) {
			os_zero_mem(output, output_Len);
			os_zero_mem(T, SHA256_DIGEST_SIZE);
			return;
		}
		iter++;

		RT_HMAC_SHA256_VECTOR(secret, secret_len, 3, addr, len, T, SHA256_DIGEST_SIZE);
	}

	os_zero_mem(T, SHA256_DIGEST_SIZE);
}

#endif
