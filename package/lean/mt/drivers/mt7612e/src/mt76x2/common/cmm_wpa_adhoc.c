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
	cmm_wpa_adhoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/

#ifdef ADHOC_WPA2PSK_SUPPORT

#include "rt_config.h"

VOID Adhoc_RTMPGetTxTscFromAsic(
	IN  PRTMP_ADAPTER   pAd,
	IN	UCHAR			apidx,
	OUT	PUCHAR			pTxTsc)
{
	USHORT			Wcid;
	USHORT			offset;
	UCHAR			IvEiv[8];
	UINT32 IV = 0, EIV = 0;
	UINT32 iveiv_tb_base = 0, iveiv_tb_size = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			iveiv_tb_base = RLT_MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = RLT_HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			iveiv_tb_base = MAC_IVEIV_TABLE_BASE;
			iveiv_tb_size = HW_IVEIV_ENTRY_SIZE;
		}
#endif /* RTMP_MAC */

	/* Get apidx for this BSSID */
	GET_GroupKey_WCID(pAd, Wcid, apidx);	

	/* Read IVEIV from Asic */
	offset = iveiv_tb_base + (Wcid * iveiv_tb_size);
	NdisZeroMemory(IvEiv, 8);
	NdisZeroMemory(pTxTsc, 6);
			
	RTMP_IO_READ32(pAd, offset, &IV);
	RTMP_IO_READ32(pAd, offset + 4, &EIV);

	*pTxTsc 	= IV & 0x000000ff;
	*(pTxTsc+1) = IV & 0x0000ff00;
	*(pTxTsc+2) = EIV & 0x000000ff;
	*(pTxTsc+3) = EIV & 0x0000ff00;
	*(pTxTsc+4) = EIV & 0x00ff0000;
	*(pTxTsc+5) = EIV & 0xff000000;

	DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_RTMPGetTxTscFromAsic : WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n", 
									Wcid, *pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));
			

}

/*
    ==========================================================================
    Description:
       Start 4-way HS when rcv EAPOL_START which may create by our driver in assoc.c
    Return:
    ==========================================================================
*/
VOID Adhoc_WpaEAPOLStartAction(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{   
    MAC_TABLE_ENTRY     *pEntry;
    PHEADER_802_11      pHeader;

    DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WpaEAPOLStartAction ===> \n"));
    
    pHeader = (PHEADER_802_11)Elem->Msg;
    
    /*For normaol PSK, we enqueue an EAPOL-Start command to trigger the process. */
    if (Elem->MsgLen == 6)
        pEntry = MacTableLookup(pAd, Elem->Msg);
    else
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);
    }
    
    if (pEntry) 
    {    
        PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = &pEntry->WPA_Authenticator;

		DBGPRINT(RT_DEBUG_TRACE, (" PortSecured(%d), WpaState(%d), AuthMode(%d), PMKID_CacheIdx(%d) \n", pEntry->PortSecured, pAuthenticator->WpaState, pEntry->AuthMode, pEntry->PMKID_CacheIdx));

        if ((pEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
			&& (pAuthenticator->WpaState < AS_PTKSTART)
            && ((pEntry->AuthMode == Ndis802_11AuthModeWPAPSK) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK) || ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx != ENTRY_NOT_FOUND))))
        {
            pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
            pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

            /*Added by Eddy */
            pAuthenticator->WpaState = AS_INITPSK;
            pAuthenticator->MsgRetryCounter = 4;
            Adhoc_WpaStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
        }
    }
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
BOOLEAN Adhoc_PeerWpaMessageSanity (
    IN 	PRTMP_ADAPTER 		pAd, 
    IN 	PEAPOL_PACKET 		pMsg, 
    IN 	ULONG 				MsgLen, 
    IN 	UCHAR				MsgType,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,    
    IN 	MAC_TABLE_ENTRY  	*pEntry)
{
	UCHAR			mic[LEN_KEY_DESC_MIC], digest[80], KEYDATA[MAX_LEN_OF_RSNIE];
	BOOLEAN			bReplayDiff = FALSE;
	BOOLEAN			bWPA2 = FALSE;
	KEY_INFO		EapolKeyInfo;	
	UCHAR			GroupKeyIndex = 0;
	
	
	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, sizeof(KEYDATA));
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));
	
	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	/* Choose WPA2 or not */
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;

	/* 0. Check MsgType */
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("The message type is invalid(%d)! \n", MsgType));
		return FALSE;
	}
				
	/* 1. Replay counter check */
 	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)	/* For supplicant */
    {
    	/* First validate replay counter, only accept message with larger replay counter. */
		/* Let equal pass, some AP start with all zero replay counter */
		UCHAR	ZeroReplay[LEN_KEY_DESC_REPLAY];
		
        NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);
		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
    	{
			bReplayDiff = TRUE;
    	}						
 	}
	else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2)	/* For authenticator */
	{
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard */
    	if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY))
    	{	
			bReplayDiff = TRUE;	        
    	}
	}

	/* Replay Counter different condition */
	if (bReplayDiff)
	{
		/* send wireless event - for replay counter different */
		RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 

		if (MsgType < EAPOL_GROUP_MSG_1)
		{
           	DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));
		}
		
		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY);	
        return FALSE;
	}

	/* 2. Verify MIC except Pairwise Msg1 */
	if (MsgType != EAPOL_PAIR_MSG_1)
	{
		UCHAR			rcvd_mic[LEN_KEY_DESC_MIC];
		UINT			eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;

		/* Record the received MIC for check later */
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
		NdisZeroMemory(pMsg->KeyDesc.KeyMic, LEN_KEY_DESC_MIC);
							
        if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP)	/* TKIP */
        {	
            RT_HMAC_MD5(p4WayProfile->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
        }
        else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)	/* AES */
        {                        
            RT_HMAC_SHA1(p4WayProfile->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
            NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
        }
	
        if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC))
        {
			/* send wireless event - for MIC different */
			RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 

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

			return FALSE;
        }        
	}

	/* 1. Decrypt the Key Data field if GTK is included. */
	/* 2. Extract the context of the Key Data field if it exist. */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear. */
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted. */
	if (CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen) > 0)
	{		
		/* Decrypt this field */
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		{					
			if((EapolKeyInfo.KeyDescVer == KEY_DESC_AES))
			{
				UINT aes_unwrap_len = 0;

   				AES_Key_Unwrap(pMsg->KeyDesc.KeyData, 
                               CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen),
  							   &p4WayProfile->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
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
			
			return TRUE;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2) */
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2 */
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2) */
		if (!RTMPParseEapolKeyData(pAd, KEYDATA, 
								  CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyDataLen), 
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
		{
			return FALSE;
		}
	}

	return TRUE;
	
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
VOID Adhoc_WpaEAPOLKeyAction(
    IN PRTMP_ADAPTER    pAd, 
    IN MLME_QUEUE_ELEM  *Elem) 
{	
    MAC_TABLE_ENTRY     *pEntry;    
    PHEADER_802_11      pHeader;
    PEAPOL_PACKET       pEapol_packet;	
	KEY_INFO			peerKeyInfo;
	UINT				eapol_len;

    DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WpaEAPOLKeyAction ===>\n"));

    pHeader = (PHEADER_802_11)Elem->Msg;
    pEapol_packet = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	eapol_len = CONV_ARRARY_TO_UINT16(pEapol_packet->Body_Len) + LENGTH_EAPOL_H;

	NdisZeroMemory((PUCHAR)&peerKeyInfo, sizeof(peerKeyInfo));
	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));

	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

    do
    {
        pEntry = MacTableLookup(pAd, pHeader->Addr2);

		if (!pEntry || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))		
            break;

		if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
				break;		

		DBGPRINT(RT_DEBUG_TRACE, ("Receive EAPoL-Key frame from STA %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pEntry->Addr)));

		if (eapol_len > Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H)
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

		/* The value 1 shall be used for all EAPOL-Key frames to and from a STA when */
		/* neither the group nor pairwise ciphers are CCMP for Key Descriptor 1. */
		if ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP))
        {
	        DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(TKIP) \n"));
    	    break;
    	}	
		/* The value 2 shall be used for all EAPOL-Key frames to and from a STA when */
		/* either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2. */
    	else if ((pEntry->WepStatus == Ndis802_11Encryption3Enabled) && (peerKeyInfo.KeyDescVer != KEY_DESC_AES))
    	{
        	DBGPRINT(RT_DEBUG_ERROR, ("Key descripter version not match(AES) \n"));
        	break;
    	}

		/* Check if this STA is in class 3 state and the WPA state is started */
        if (pEntry->Sst == SST_ASSOC)
        {			 		
			/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator */
			/* or not. */
			/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL- */
			/* Key frame from the Authenticator must not have the Ack bit set. */
			if ((peerKeyInfo.KeyAck == 1) && (pEntry->WPA_Supplicant.WpaState >= AS_INITPSK))
			{
				/* The frame is snet by Authenticator. */
				/* So the Supplicant side shall handle this. */
				if ((peerKeyInfo.Secure == 0) && (peerKeyInfo.Request == 0) && 
					(peerKeyInfo.Error == 0) && (peerKeyInfo.KeyType == PAIRWISEKEY))
				{
					/* Process 1. the message 1 of 4-way HS in WPA or WPA2 */
					/*			  EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1) */
					/*		   2. the message 3 of 4-way HS in WPA */
					/*			  EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3) */
					if (peerKeyInfo.KeyMic == 0) {
                    	Adhoc_PeerPairMsg1Action(pAd, pEntry, Elem);
	                } else {
    	                Adhoc_PeerPairMsg3Action(pAd, pEntry, Elem);
                    }
				}
				else if ((peerKeyInfo.Secure == 1) && 
						 (peerKeyInfo.KeyMic == 1) &&
						 (peerKeyInfo.Request == 0) && 
						 (peerKeyInfo.Error == 0))
				{
					/* Process 1. the message 3 of 4-way HS in WPA2 */
					/*			  EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3) */
					/*		   2. the message 1 of group KS in WPA or WPA2 */
					/*			  EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N]) */
					if (peerKeyInfo.KeyType == PAIRWISEKEY) {
						Adhoc_PeerPairMsg3Action(pAd, pEntry, Elem);
					} else {
						Adhoc_PeerGroupMsg1Action(pAd, pEntry, Elem);	
                    }
				}
			}
			else if (pEntry->WPA_Authenticator.WpaState >= AS_INITPSK)
			{			
				/* The frame is snet by Supplicant. */
				/* So the Authenticator side shall handle this. */
				if ((peerKeyInfo.KeyMic == 1) && 
					(peerKeyInfo.Request == 1) && 
					(peerKeyInfo.Error == 1))
	            {	                
					/* The Supplicant uses a single Michael MIC Failure Report frame */
					/* to report a MIC failure event to the Authenticator. */
					/* A Michael MIC Failure Report is an EAPOL-Key frame with */
					/* the following Key Information field bits set to 1: */
					/* MIC bit, Error bit, Request bit, Secure bit. */

	                DBGPRINT(RT_DEBUG_ERROR, ("Received an Michael MIC Failure Report, active countermeasure \n"));
	            }
				else if ((peerKeyInfo.Request == 0) && 
					 	 (peerKeyInfo.Error == 0) && 
					 	 (peerKeyInfo.KeyMic == 1))
				{
					if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/* EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data) */
						/* Process 1. message 2 of 4-way HS in WPA or WPA2 */
						/*		   2. message 4 of 4-way HS in WPA */
						if (CONV_ARRARY_TO_UINT16(pEapol_packet->KeyDesc.KeyDataLen) == 0)
						{
							Adhoc_PeerPairMsg4Action(pAd, pEntry, Elem);
    	            	}
						else
						{
							Adhoc_PeerPairMsg2Action(pAd, pEntry, Elem);
						}
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY)
					{
						/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0) */
						/* Process message 4 of 4-way HS in WPA2 */
						Adhoc_PeerPairMsg4Action(pAd, pEntry, Elem);
					}
					else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY)
					{
						/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0) */
						/* Process message 2 of Group key HS in WPA or WPA2 */
/*						Adhoc_PeerGroupMsg2Action(pAd, pEntry, &Elem->Msg[LENGTH_802_11], (Elem->MsgLen - LENGTH_802_11)); */
					}
				}
			}			            
        }
    }while(FALSE);
}



/*
    ==========================================================================
    Description:
        This is a function to initilize 4-way handshake
        
    Return:
         
    ==========================================================================
*/
VOID Adhoc_WpaStart4WayHS(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN ULONG			TimeInterval) 
{
    UCHAR           Header802_3[14];
	UCHAR   		*mpool;
    PEAPOL_PACKET	pEapolFrame;
	PUINT8			pBssid = NULL;
    BOOLEAN         Cancelled;    
	UCHAR			group_cipher = Ndis802_11WEPDisabled;
    PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = NULL;

    DBGPRINT(RT_DEBUG_TRACE, ("===> Adhoc_WpaStart4WayHS\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS | fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : The interface is closed...\n"));
		return;		
	}
    if (!pEntry) {
        DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : The entry doesn't exist.\n"));		
        return;
    }
    pAuthenticator = &pEntry->WPA_Authenticator;
	pBssid = pAd->CommonCfg.Bssid;
	group_cipher = pAd->StaCfg.GroupCipher;	

     /* delete retry timer */
    RTMPCancelTimer(&pAuthenticator->MsgRetryTimer, &Cancelled);

	if (pBssid == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : No corresponding Authenticator.\n"));		
		return;
    }

	/* Check the status */
    if ((pAuthenticator->WpaState > AS_PTKSTART) || (pAuthenticator->WpaState < AS_INITPMK))
    {
        DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]WPAStart4WayHS : Not expect calling=%d\n", pAuthenticator->WpaState));
        return;
    }

	/* Increment replay counter by 1 */
	ADD_ONE_To_64BIT_VAR(pAuthenticator->ReplayCounter);
	
	/* Randomly generate ANonce */
	GenRandom(pAd, (UCHAR *)pBssid, pAuthenticator->ANonce);	

	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	
	/* Construct EAPoL message - Pairwise Msg 1 */
	/* EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1) */
	Adhoc_ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_1,
					  0,					/* Default key index */
					  pAuthenticator->ANonce,
					  NULL,					/* TxRSC */
					  NULL,					/* GTK */
					  NULL,					/* RSNIE */
					  0,					/* RSNIE length */
					  pAuthenticator,
					  pEapolFrame);

    if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
    {
        UCHAR	digest[80], PMK_key[20];
        PKEY_DESCRIPTER  pKeyDesc = &pEapolFrame->KeyDesc;

        pKeyDesc->KeyData[0] = 0xDD;
        pKeyDesc->KeyData[2] = 0x00;
        pKeyDesc->KeyData[3] = 0x0F;
        pKeyDesc->KeyData[4] = 0xAC;
        pKeyDesc->KeyData[5] = 0x04;

        NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
        NdisMoveMemory(&PMK_key[8], pAd->CurrentAddress, MAC_ADDR_LEN);
        NdisMoveMemory(&PMK_key[14], pEntry->Addr, MAC_ADDR_LEN);
        RT_HMAC_SHA1(pAd->StaCfg.PMK, PMK_LEN, PMK_key, 20, digest, LEN_PMKID);

        NdisMoveMemory(&pKeyDesc->KeyData[6], digest, LEN_PMKID);
        pKeyDesc->KeyData[1] = 0x14;/* 4+LEN_PMKID */
        INC_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, 6 + LEN_PMKID);    			
        INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
    }
        
	/* Make outgoing frame */
    MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);            
    RTMPToWirelessSta(pAd, pEntry, Header802_3, 
					  LENGTH_802_3, (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	/* Trigger Retry Timer */
    pAuthenticator->MsgType = EAPOL_PAIR_MSG_1;
    RTMPSetTimer(&pAuthenticator->MsgRetryTimer, TimeInterval);
    
	/* Update State */
    pAuthenticator->WpaState = AS_PTKSTART;

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== Adhoc_WpaStart4WayHS: send Msg1 of 4-way \n"));
        
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
VOID Adhoc_PeerPairMsg1Action(
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
    PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant = NULL;
    	   
	DBGPRINT(RT_DEBUG_TRACE, ("===> Adhoc_PeerPairMsg1Action \n"));

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
		return;

    if (ADHOC_ON(pAd) && (pEntry->WPA_Supplicant.WpaState == AS_PTKINITDONE))
    {
        MlmeDeAuthAction(pAd, pEntry, REASON_DEAUTH_STA_LEAVING, FALSE);
        return;
    }

    pSupplicant = &pEntry->WPA_Supplicant;
    
    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
        return;
	
	pCurrentAddr = pAd->CurrentAddress;
	pmk_ptr = pAd->StaCfg.PMK;
	group_cipher = pAd->StaCfg.GroupCipher;
	rsnie_ptr = pAd->StaCfg.RSN_IE;
	rsnie_len = pAd->StaCfg.RSNIE_Len;

	/* Store the received frame */
	pMsg1 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;
	
	/* Sanity Check peer Pairwise message 1 - Replay Counter */
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg1, MsgLen, EAPOL_PAIR_MSG_1, pSupplicant, pEntry) == FALSE)
		return;

    
	/* Store Replay counter, it will use to verify message 3 and construct message 2 */
	NdisMoveMemory(pSupplicant->ReplayCounter, pMsg1->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);		
        
	/* Store ANonce */
	NdisMoveMemory(pSupplicant->ANonce, pMsg1->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
		
	/* Generate random SNonce */
	GenRandom(pAd, (UCHAR *)pCurrentAddr, pSupplicant->SNonce);

    /* Calculate PTK(ANonce, SNonce) */
    WpaDerivePTK(pAd,
    			pmk_ptr,
		     	pSupplicant->ANonce,
			 	pEntry->Addr, 
			 	pSupplicant->SNonce,
			 	pCurrentAddr, 
			    PTK, 
			    LEN_PTK);

	/* Save key to PTK entry */
	NdisMoveMemory(pSupplicant->PTK, PTK, LEN_PTK);
		
	/* Update WpaState */
	pSupplicant->WpaState = AS_PTKINIT_NEGOTIATING;

	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 2 */
	/*  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2) */
	Adhoc_ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_2,  
					  0,				/* DefaultKeyIdx */
					  pSupplicant->SNonce,
					  NULL,				/* TxRsc */
					  NULL,				/* GTK */
					  (UCHAR *)rsnie_ptr,
					  rsnie_len,
					  pSupplicant,
					  pEapolFrame);

	/* Make outgoing frame */
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);	
	
	RTMPToWirelessSta(pAd, pEntry, 
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame, 
   					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
   					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

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
VOID Adhoc_PeerPairMsg2Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{   
	UCHAR				PTK[80];
    BOOLEAN             Cancelled;
    PHEADER_802_11      pHeader;
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
	PUCHAR				rsnie_ptr = NULL;
	UCHAR				rsnie_len = 0;
    PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = NULL;    

    DBGPRINT(RT_DEBUG_TRACE, ("===> Adhoc_PeerPairMsg2Action \n"));

    if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
        return;

    pAuthenticator = &pEntry->WPA_Authenticator;
        
    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
        return;

    /* check Entry in valid State */
    if (pAuthenticator->WpaState < AS_PTKSTART)
        return;

	pBssid = pAd->CommonCfg.Bssid;

	pmk_ptr = pAd->StaCfg.PMK;
	gtk_ptr = pAd->StaCfg.GTK;
	group_cipher = pAd->StaCfg.GroupCipher;

	default_key = pAd->StaCfg.wdev.DefaultKeyId;
    if (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
    {
        rsnie_len = pAd->StaCfg.RSNIE_Len;
        rsnie_ptr = pAd->StaCfg.RSN_IE;
    }
	

    /* pointer to 802.11 header */
	pHeader = (PHEADER_802_11)Elem->Msg;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pMsg2 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];       
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	/* Store SNonce */
	NdisMoveMemory(pAuthenticator->SNonce, pMsg2->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	/* Derive PTK */
	WpaDerivePTK(pAd, 
				(UCHAR *)pmk_ptr,  
				pAuthenticator->ANonce, 		/* ANONCE */
				pAd->CurrentAddress, 
				pAuthenticator->SNonce, 		/* SNONCE */
				pEntry->Addr, 
				PTK, 
				LEN_PTK); 		

	/* Get Group TxTsc form Asic */
	Adhoc_RTMPGetTxTscFromAsic(pAd, BSS0, TxTsc);

   	NdisMoveMemory(pAuthenticator->PTK, PTK, LEN_PTK);

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE */
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg2, MsgLen, EAPOL_PAIR_MSG_2, pAuthenticator, pEntry) == FALSE)
		return;

    do
    {
		/* Allocate memory for input */
		os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
		if (mpool == NULL)
	    {
	        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
	        return;
	    }

		pEapolFrame = (PEAPOL_PACKET)mpool;
		NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	    
        /* delete retry timer */
		RTMPCancelTimer(&pAuthenticator->MsgRetryTimer, &Cancelled);

		/* Increment replay counter by 1 */
		ADD_ONE_To_64BIT_VAR(pAuthenticator->ReplayCounter);

		/* Construct EAPoL message - Pairwise Msg 3 */
		Adhoc_ConstructEapolMsg(pEntry,
						  group_cipher,
						  EAPOL_PAIR_MSG_3,
						  default_key,
						  pAuthenticator->ANonce,
						  TxTsc,
						  (UCHAR *)gtk_ptr,
						  (UCHAR *)rsnie_ptr,
						  rsnie_len,
						  pAuthenticator,
						  pEapolFrame);
            
        /* Make outgoing frame */
        MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pBssid, EAPOL);            
        RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3, 
						  (PUCHAR)pEapolFrame, 
						  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
						  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

        pAuthenticator->MsgType = EAPOL_PAIR_MSG_3;
		RTMPSetTimer(&pAuthenticator->MsgRetryTimer, PEER_MSG3_RETRY_EXEC_INTV);

		/* Update State */
        pAuthenticator->WpaState = AS_PTKINIT_NEGOTIATING;

		os_free_mem(NULL, mpool);
	
    }while(FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerPairMsg2Action: send Msg3 of 4-way \n"));
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
VOID Adhoc_PeerPairMsg3Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{
	PHEADER_802_11		pHeader;
	UCHAR               Header802_3[14];
	UCHAR				*mpool;
	PEAPOL_PACKET		pEapolFrame;
	PEAPOL_PACKET		pMsg3;
	UINT            	MsgLen;				
	PUINT8				pCurrentAddr = NULL;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
    PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant = NULL;
    
	DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerPairMsg3Action \n"));
	
	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
		return;

    pSupplicant = &pEntry->WPA_Supplicant;
    if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pCurrentAddr = pAd->CurrentAddress;
	group_cipher = pAd->StaCfg.GroupCipher;
		
	/* Record 802.11 header & the received EAPOL packet Msg3 */
	pHeader	= (PHEADER_802_11) Elem->Msg;
	pMsg3 = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE */
	if (Adhoc_PeerWpaMessageSanity(pAd, pMsg3, MsgLen, EAPOL_PAIR_MSG_3, pSupplicant, pEntry) == FALSE)
		return;
	
	/* Save Replay counter, it will use construct message 4 */
	NdisMoveMemory(pSupplicant->ReplayCounter, pMsg3->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Double check ANonce */
	if (!NdisEqualMemory(pSupplicant->ANonce, pMsg3->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
	{
		return;
	}

	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);

	/* Construct EAPoL message - Pairwise Msg 4 */
	Adhoc_ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_PAIR_MSG_4,  
					  0,					/* group key index not used in message 4 */
					  NULL,					/* Nonce not used in message 4 */
					  NULL,					/* TxRSC not used in message 4 */
					  NULL,					/* GTK not used in message 4 */
					  NULL,					/* RSN IE not used in message 4 */
					  0,
					  pSupplicant,
					  pEapolFrame);

	/* open 802.1x port control and privacy filter */
	if (pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK || 
		pEntry->AuthMode == Ndis802_11AuthModeWPA2)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("PeerPairMsg3Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));
	}

	/* Init 802.3 header and send out */
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);	
	RTMPToWirelessSta(pAd, pEntry, 
					  Header802_3, sizeof(Header802_3), 
					  (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, 
   					  (pEntry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);

	/* Update WpaState */
	pSupplicant->WpaState = AS_PTKINITDONE;
    Adhoc_Wpa4WayComplete(pAd, pEntry);
	os_free_mem(NULL, mpool);

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
VOID Adhoc_PeerPairMsg4Action(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem) 
{    
	PEAPOL_PACKET   	pMsg4;    
    PHEADER_802_11      pHeader;
    UINT            	MsgLen;
    BOOLEAN             Cancelled;
	UCHAR				group_cipher = Ndis802_11WEPDisabled;
    PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = NULL; 
    
    DBGPRINT(RT_DEBUG_TRACE, ("===> Adhoc_PeerPairMsg4Action\n"));

    if ((!pEntry) || !IS_ENTRY_CLIENT(pEntry))
        return;

    pAuthenticator = &pEntry->WPA_Authenticator;

    do
    {		
        if (Elem->MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG ) )
            break;

        if (pAuthenticator->WpaState < AS_PTKINIT_NEGOTIATING)
            break;

		group_cipher = pAd->StaCfg.GroupCipher;
 
        /* pointer to 802.11 header */
        pHeader = (PHEADER_802_11)Elem->Msg;

		/* skip 802.11_header(24-byte) and LLC_header(8) */
		pMsg4 = (PEAPOL_PACKET)&Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H]; 
		MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

        /* Sanity Check peer Pairwise message 4 - Replay Counter, MIC */
		if (Adhoc_PeerWpaMessageSanity(pAd, pMsg4, MsgLen, EAPOL_PAIR_MSG_4, pAuthenticator, pEntry) == FALSE)
			break;

        pAuthenticator->WpaState = AS_PTKINITDONE;
		RTMPCancelTimer(&pAuthenticator->MsgRetryTimer, &Cancelled);
        RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);

        Adhoc_Wpa4WayComplete(pAd, pEntry);
        
 		if (pEntry->AuthMode == Ndis802_11AuthModeWPA2 || 
			pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK)
		{
			pEntry->GTKState = REKEY_ESTABLISHED;

			/* send wireless event - for set key done WPA2 */
			RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0); 	 
		}
    }while(FALSE);
    
}


VOID Adhoc_PeerGroupMsg1Action(
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
    PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant = NULL;
    	
	DBGPRINT(RT_DEBUG_ERROR, ("===> Adhoc_PeerGroupMsg1Action \n"));

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)))
        return;

    pSupplicant = &pEntry->WPA_Supplicant;
	pCurrentAddr = pAd->CurrentAddress;
	group_cipher = pAd->StaCfg.GroupCipher;
	default_key = pAd->StaCfg.wdev.DefaultKeyId;
	   
	/* Process Group Message 1 frame. skip 802.11 header(24) & LLC_SNAP header(8) */
	pGroup = (PEAPOL_PACKET) &Elem->Msg[LENGTH_802_11 + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - LENGTH_802_11 - LENGTH_802_1_H;

	/* Sanity Check peer group message 1 - Replay Counter, MIC, RSNIE */
	if (Adhoc_PeerWpaMessageSanity(pAd, pGroup, MsgLen, EAPOL_GROUP_MSG_1, pSupplicant, pEntry) == FALSE)
		return;

	/* Save Replay counter, it will use to construct message 2 */
	NdisMoveMemory(pSupplicant->ReplayCounter, pGroup->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);	

	/* Allocate memory for output */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);
	if (mpool == NULL)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
        return;
    }

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);


	/* Construct EAPoL message - Group Msg 2 */
    Adhoc_ConstructEapolMsg(pEntry,
					  group_cipher,
					  EAPOL_GROUP_MSG_2,  
					  default_key,
					  NULL,					/* Nonce not used */
					  NULL,					/* TxRSC not used */
					  NULL,					/* GTK not used */
					  NULL,					/* RSN IE not used */
					  0,
					  pSupplicant,
					  pEapolFrame);
					
    /* open 802.1x port control and privacy filter */
	pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;

#ifdef CONFIG_STA_SUPPORT
	STA_PORT_SECURED(pAd);
    /* Indicate Connected for GUI */
    pAd->IndicateMediaState = NdisMediaStateConnected;
#endif /* CONFIG_STA_SUPPORT */
	
	DBGPRINT(RT_DEBUG_TRACE, ("PeerGroupMsg1Action: AuthMode(%s) PairwiseCipher(%s) GroupCipher(%s) \n",
									GetAuthMode(pEntry->AuthMode),
									GetEncryptType(pEntry->WepStatus),
									GetEncryptType(group_cipher)));
		
	/* init header and Fill Packet and send Msg 2 to authenticator */
	MAKE_802_3_HEADER(Header802_3, pEntry->Addr, pCurrentAddr, EAPOL);	
	
#ifdef CONFIG_STA_SUPPORT
	if ((pAd->OpMode == OPMODE_STA) && INFRA_ON(pAd) && 
		OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) &&
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
		(pAd->MlmeAux.Channel == pAd->CommonCfg.Channel))
	{
		/* Now stop the scanning and need to send the rekey packet out */
		pAd->MlmeAux.Channel = 0;
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMPToWirelessSta(pAd, pEntry, 
					  Header802_3, sizeof(Header802_3), 
					  (PUCHAR)pEapolFrame, 
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);

	os_free_mem(NULL, mpool);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== PeerGroupMsg1Action: send group message 2\n"));
}


VOID Adhoc_Wpa4WayComplete(
    IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry)
{
    PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = NULL; 
    PFOUR_WAY_HANDSHAKE_PROFILE pSupplicant = NULL; 
	INT compare_address;

    DBGPRINT(RT_DEBUG_TRACE, ("===> Adhoc_Wpa4WayComplete\n"));
    
    if (!pEntry)
        return;

    pAuthenticator = &pEntry->WPA_Authenticator;
    pSupplicant = &pEntry->WPA_Supplicant;

	compare_address = NdisCmpMemory(pAd->CurrentAddress, pEntry->Addr, MAC_ADDR_LEN);
    if ((compare_address > 0) && (pAuthenticator->WpaState == AS_PTKINITDONE)) {
        NdisMoveMemory(pEntry->PTK, pAuthenticator->PTK, 64);
    } else if ((compare_address < 0) && (pSupplicant->WpaState == AS_PTKINITDONE)) {
        NdisMoveMemory(pEntry->PTK, pSupplicant->PTK, 64);
    } else
        return;
    
#ifdef IWSC_SUPPORT
	if (pEntry)
	{
		PWSC_CTRL pWpsCtrl = &pAd->StaCfg.WscControl;
		PWSC_PEER_ENTRY pWscPeerEntry = NULL;

		RTMP_SEM_LOCK(&pWpsCtrl->WscConfiguredPeerListSemLock);
		pWscPeerEntry = WscFindPeerEntry(&pWpsCtrl->WscConfiguredPeerList, pEntry->Addr);
		if (pWscPeerEntry)
		{
			WscDelListEntryByMAC(&pWpsCtrl->WscConfiguredPeerList, pEntry->Addr);
		}
		RTMP_SEM_UNLOCK(&pWpsCtrl->WscConfiguredPeerListSemLock);
	}
#endif /* IWSC_SUPPORT */
    
	WPAInstallPairwiseKey(pAd, 
						  BSS0, 
						  pEntry, 
						  FALSE);

    pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
    pEntry->PortSecured = WPA_802_1X_PORT_SECURED;

    STA_PORT_SECURED(pAd);
    /* Indicate Connected for GUI */
    pAd->IndicateMediaState = NdisMediaStateConnected;

    DBGPRINT(RT_DEBUG_OFF, ("Adhoc_Wpa4WayComplete - WPA2, AuthMode(%d)=%s, WepStatus(%d)=%s, GroupWepStatus(%d)=%s\n\n", 
							pEntry->AuthMode, GetAuthMode(pEntry->AuthMode), 
							pEntry->WepStatus, GetEncryptType(pEntry->WepStatus), 
							pAd->StaCfg.GroupCipher, 
							GetEncryptType(pAd->StaCfg.GroupCipher)));        
}


VOID Adhoc_WpaRetryExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    MAC_TABLE_ENTRY     *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

    if ((pEntry) && IS_ENTRY_CLIENT(pEntry))
    {
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;
        PFOUR_WAY_HANDSHAKE_PROFILE pAuthenticator = &pEntry->WPA_Authenticator;
       
        DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WPARetryExec---> ReTryCounter=%d, WpaState=%d \n", pAuthenticator->MsgRetryCounter, pAuthenticator->WpaState));

        switch (pEntry->AuthMode)
        {
			case Ndis802_11AuthModeWPA:
            case Ndis802_11AuthModeWPAPSK:
			case Ndis802_11AuthModeWPA2:
            case Ndis802_11AuthModeWPA2PSK:
                if (pAuthenticator->MsgRetryCounter == 0)
                {
					/* send wireless event - for pairwise key handshaking timeout */
					RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->apidx, 0);

                    pEntry->WPA_Authenticator.WpaState = AS_NOTUSE;
#ifdef IWSC_SUPPORT
					if (pAd->StaCfg.WscControl.bWscTrigger == FALSE)
#endif // IWSC_SUPPORT //					
					{
                    MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);                    
                    DBGPRINT(RT_DEBUG_ERROR, ("Adhoc_WPARetryExec::MSG1 timeout\n"));
					}
                
                } 
                else if (pAuthenticator->MsgType == EAPOL_PAIR_MSG_1)
                {
                    if ((pAuthenticator->WpaState == AS_PTKSTART) || (pAuthenticator->WpaState == AS_INITPSK) || (pAuthenticator->WpaState == AS_INITPMK))
                    {
                        DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WPARetryExec::ReTry MSG1 of 4-way Handshake\n"));
                        Adhoc_WpaStart4WayHS(pAd, pEntry, PEER_MSG1_RETRY_EXEC_INTV);
                    }                
                }
                else if (pAuthenticator->MsgType == EAPOL_PAIR_MSG_3)
                {
                	pEntry->WPA_Authenticator.WpaState = AS_INITPSK;
#ifdef IWSC_SUPPORT
					if (pAd->StaCfg.WscControl.bWscTrigger == FALSE)
#endif // IWSC_SUPPORT //	
					{
                    MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
                    DBGPRINT(RT_DEBUG_TRACE, ("Adhoc_WPARetryExec::Retry MSG3, TIMEOUT\n"));
                }
                }
                break;
            default:
                break;
        }
        pAuthenticator->MsgRetryCounter--;        
    }
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
VOID    Adhoc_ConstructEapolMsg(
	IN 	PMAC_TABLE_ENTRY	pEntry,
    IN 	UCHAR				GroupKeyWepStatus,
    IN 	UCHAR				MsgType,  
    IN	UCHAR				DefaultKeyIdx,
	IN 	UCHAR				*KeyNonce,
	IN	UCHAR				*TxRSC,
	IN	UCHAR				*GTK,
	IN	UCHAR				*RSNIE,
	IN	UCHAR				RSNIE_Len,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,
    OUT PEAPOL_PACKET       pMsg)
{
	BOOLEAN	bWPA2 = FALSE;
	UCHAR	KeyDescVer;
    PKEY_DESCRIPTER pKeyDesc = &pMsg->KeyDesc;
	PKEY_INFO       pKeyInfo = &pMsg->KeyDesc.KeyInfo;
    
	/* Choose WPA2 or not */
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2 = TRUE;
		
    /* Init Packet and Fill header */
    pMsg->ProVer = EAPOL_VER;
    pMsg->ProType = EAPOLKey;

	/* Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field */
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);

	/* Fill in EAPoL descriptor */
	if (bWPA2)
		pKeyDesc->Type = WPA2_KEY_DESC;
	else
		pKeyDesc->Type = WPA1_KEY_DESC;
			
	/* Key Descriptor Version (bits 0-2) specifies the key descriptor version type */
	/* Fill in Key information, refer to IEEE Std 802.11i-2004 page 78 */
	/* When either the pairwise or the group cipher is AES, the KEY_DESC_AES shall be used. */
	KeyDescVer = (((pEntry->WepStatus == Ndis802_11Encryption3Enabled) || 
	        		(GroupKeyWepStatus == Ndis802_11Encryption3Enabled)) ? (KEY_DESC_AES) : (KEY_DESC_TKIP));

	pKeyInfo->KeyDescVer = KeyDescVer;

	/* Specify Key Type as Group(0) or Pairwise(1) */
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pKeyInfo->KeyType = GROUPKEY;
	else
		pKeyInfo->KeyType = PAIRWISEKEY;

	/* Specify Key Index, only group_msg1_WPA1 */
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pKeyInfo->KeyIndex = DefaultKeyIdx;
	
	if (MsgType == EAPOL_PAIR_MSG_3)
		pKeyInfo->Install = 1;
	
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pKeyInfo->KeyAck = 1;

	if (MsgType != EAPOL_PAIR_MSG_1)	
		pKeyInfo->KeyMic = 1;
 
	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) || (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
       	pKeyInfo->Secure = 1;                   

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1)))
        pKeyInfo->EKD_DL = 1;            

	/* key Information element has done. */
	*(USHORT *)(pKeyInfo) = cpu2le16(*(USHORT *)(pKeyInfo));

	/* Fill in Key Length */
	if (bWPA2)
	{
		/* In WPA2 mode, the field indicates the length of pairwise key cipher, */
		/* so only pairwise_msg_1 and pairwise_msg_3 need to fill. */
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
			pKeyDesc->KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);
	}
	else
	{
		if (MsgType >= EAPOL_GROUP_MSG_1)
		{
			/* the length of group key cipher */
			pKeyDesc->KeyLength[1] = ((GroupKeyWepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_GTK : LEN_AES_GTK);
		}
		else
		{
			/* the length of pairwise key cipher */
			pKeyDesc->KeyLength[1] = ((pEntry->WepStatus == Ndis802_11Encryption2Enabled) ? LEN_TKIP_TK : LEN_AES_TK);			
		}				
	}			
	
 	/* Fill in replay counter */
    NdisMoveMemory(pKeyDesc->ReplayCounter, p4WayProfile->ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Fill Key Nonce field */
	/* ANonce : pairwise_msg1 & pairwise_msg3 */
	/* SNonce : pairwise_msg2 */
	/* GNonce : group_msg1_wpa1 */
	if ((MsgType <= EAPOL_PAIR_MSG_3) || ((!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))))
    	NdisMoveMemory(pKeyDesc->KeyNonce, KeyNonce, LEN_KEY_DESC_NONCE);

	/* Fill key IV - WPA2 as 0, WPA1 as random */
	if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
	{		
		/* Suggest IV be random number plus some number, */
		NdisMoveMemory(pKeyDesc->KeyIv, &KeyNonce[16], LEN_KEY_DESC_IV);		
        pKeyDesc->KeyIv[15] += 2;		
	}
	
    /* Fill Key RSC field */
    /* It contains the RSC for the GTK being installed. */
	if ((TxRSC != NULL) && ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)))
	{		
        NdisMoveMemory(pKeyDesc->KeyRsc, TxRSC, 6);
	}

	/* Clear Key MIC field for MIC calculation later */
    NdisZeroMemory(pKeyDesc->KeyMic, LEN_KEY_DESC_MIC);
	
	Adhoc_ConstructEapolKeyData(pEntry,
						  GroupKeyWepStatus, 
						  KeyDescVer,
						  MsgType, 
						  DefaultKeyIdx, 
						  GTK,
						  RSNIE,
						  RSNIE_Len,
						  p4WayProfile,
						  pMsg);
 
	/* Calculate MIC and fill in KeyMic Field except Pairwise Msg 1. */
	if (MsgType != EAPOL_PAIR_MSG_1)
        CalculateMIC(KeyDescVer, p4WayProfile->PTK, pMsg);

	DBGPRINT(RT_DEBUG_TRACE, ("===> ConstructEapolMsg for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Body length = %d \n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	DBGPRINT(RT_DEBUG_TRACE, ("	     Key length  = %d \n", CONV_ARRARY_TO_UINT16(pKeyDesc->KeyLength)));
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
VOID	Adhoc_ConstructEapolKeyData(
	IN	PMAC_TABLE_ENTRY	pEntry,
	IN	UCHAR			GroupKeyWepStatus,
	IN	UCHAR			keyDescVer,
	IN 	UCHAR			MsgType,
	IN	UCHAR			DefaultKeyIdx,
	IN	UCHAR			*GTK,
	IN	UCHAR			*RSNIE,
	IN	UCHAR			RSNIE_LEN,
	IN  PFOUR_WAY_HANDSHAKE_PROFILE p4WayProfile,	
	OUT PEAPOL_PACKET   pMsg)
{
	UCHAR		*mpool, *Key_Data, *eGTK;  	  
	ULONG		data_offset;
	BOOLEAN		bWPA2Capable = FALSE;
	BOOLEAN		GTK_Included = FALSE;
    PKEY_DESCRIPTER pKeyDesc = &pMsg->KeyDesc;
    
	/* Choose WPA2 or not */
	if ((pEntry->AuthMode == Ndis802_11AuthModeWPA2) || 
		(pEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
		bWPA2Capable = TRUE;

	if (MsgType == EAPOL_PAIR_MSG_1 || 
		MsgType == EAPOL_PAIR_MSG_4 || 
		MsgType == EAPOL_GROUP_MSG_2)
		return;
 
	/* allocate memory pool */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

    if (mpool == NULL)
		return;
        
	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);

	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, 0);
	data_offset = 0;
	
	/* Encapsulate RSNIE in pairwise_msg2 & pairwise_msg3 */
	if (RSNIE_LEN && ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)))
	{
		PUINT8	pmkid_ptr = NULL;
		UINT8 	pmkid_len = 0;

		RTMPInsertRSNIE(&Key_Data[data_offset], 
						&data_offset,
						RSNIE, 
						RSNIE_LEN, 
						pmkid_ptr, 
						pmkid_len);
	}

	/* Encapsulate GTK */
	/* Only for pairwise_msg3_WPA2 and group_msg1 */
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2Capable) || (MsgType == EAPOL_GROUP_MSG_1))
	{
		UINT8	gtk_len;

		/* Decide the GTK length */ 
		if (GroupKeyWepStatus == Ndis802_11Encryption3Enabled)
			gtk_len = LEN_AES_GTK;
		else
			gtk_len = LEN_TKIP_GTK;
		
		/* Insert GTK KDE format in WAP2 mode */
		if (bWPA2Capable)
		{
			/* Construct the common KDE format */
			WPA_ConstructKdeHdr(KDE_GTK, 2 + gtk_len, &Key_Data[data_offset]);
			data_offset += sizeof(KDE_HDR);

			/* GTK KDE format - 802.11i-2004  Figure-43x */
	        Key_Data[data_offset] = (DefaultKeyIdx & 0x03);
	        Key_Data[data_offset + 1] = 0x00;	/* Reserved Byte */
	        data_offset += 2;

		}

		/* Fill in GTK */
		NdisMoveMemory(&Key_Data[data_offset], GTK, gtk_len);
		data_offset += gtk_len;



		GTK_Included = TRUE;
	}

	/* If the Encrypted Key Data subfield (of the Key Information field) 
	   is set, the entire Key Data field shall be encrypted. */
	/* This whole key-data field shall be encrypted if a GTK is included. */
	/* Encrypt the data material in key data field with KEK */
	if (GTK_Included)
	{
		if ((keyDescVer == KEY_DESC_AES))
		{
			UCHAR 	remainder = 0;
			UCHAR	pad_len = 0;			
			UINT	wrap_len =0;

			/* Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, */
			/* shall be used to encrypt the Key Data field using the KEK field from */
			/* the derived PTK. */

			/* If the Key Data field uses the NIST AES key wrap, then the Key Data field */
			/* shall be padded before encrypting if the key data length is less than 16 */
			/* octets or if it is not a multiple of 8. The padding consists of appending */
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
   						 &p4WayProfile->PTK[LEN_PTK_KCK], LEN_PTK_KEK, 
   						 eGTK, &wrap_len);	
   			data_offset = wrap_len;
		}
		else
		{
   			TKIP_GTK_KEY_WRAP(&p4WayProfile->PTK[LEN_PTK_KCK], 
   								pMsg->KeyDesc.KeyIv,									
   								Key_Data, 
   								data_offset,
   								eGTK);
		}

		NdisMoveMemory(pKeyDesc->KeyData, eGTK, data_offset);
	}
	else
	{
		NdisMoveMemory(pKeyDesc->KeyData, Key_Data, data_offset);
	}

	/* Update key data length field and total body length */
	SET_UINT16_TO_ARRARY(pKeyDesc->KeyDataLen, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);

	os_free_mem(NULL, mpool);

}
#endif /* ADHOC_WPA2PSK_SUPPORT */

