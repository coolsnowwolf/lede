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
 ***************************************************************************/

/****************************************************************************
	Abstract:
        IEEE P802.11w

***************************************************************************/

#ifdef DOT11W_PMF_SUPPORT

#include "rt_config.h"

/* The definition in IEEE 802.11w - Table 7-32 Cipher suite selectors */
UCHAR OUI_PMF_BIP_CIPHER[4]= {0x00, 0x0F, 0xAC, 0x06};

/* The definition in IEEE 802.11w - Table 7-34 AKM suite selectors */
UCHAR OUI_PMF_8021X_AKM[4]= {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_PMF_PSK_AKM[4]= {0x00, 0x0F, 0xAC, 0x06};

UCHAR PMF_MMIE_BUFFER[18]= {0x4C, 0x10, 
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#define SAQ_IDLE	0
#define SAQ_RETRY	1
#define SAQ_SENDING	2


VOID PMF_PeerAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
        UCHAR Action = Elem->Msg[LENGTH_802_11+1];

        DBGPRINT(RT_DEBUG_WARN, ("[PMF]%s : PMF_PeerAction Action=%d\n", __FUNCTION__, Action));
        switch (Action) {
        case ACTION_SAQ_REQUEST:
                PMF_PeerSAQueryReqAction(pAd, Elem);
                break;
        case ACTION_SAQ_RESPONSE:           
                PMF_PeerSAQueryRspAction(pAd, Elem);
                break;
        }
}


VOID PMF_MlmeSAQueryReq(
        IN PRTMP_ADAPTER pAd, 
        IN MAC_TABLE_ENTRY *pEntry)
{
        PUCHAR pOutBuffer = NULL;
        HEADER_802_11 SAQReqHdr;
        UINT32 FrameLen = 0;
        UCHAR SACategoryType, SAActionType;
        PPMF_CFG pPmfCfg = NULL;

        if (!pEntry)
        {
                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is NULL\n", __FUNCTION__));
                return;
        }
    
        if (!(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
        {
                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pEntry->Addr)));
                return;
        }

	if (pEntry->SAQueryStatus == SAQ_SENDING)
		return;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{									
                pPmfCfg = &pAd->ApCfg.MBSSID[pEntry->apidx].PmfCfg;
        }
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{									
                pPmfCfg = &pAd->StaCfg.PmfCfg;
	}
#endif /* CONFIG_STA_SUPPORT */

        if (pPmfCfg)
        {
                /* Send the SA Query Request */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
                if(pOutBuffer == NULL)
                        return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{		
                MgtMacHeaderInit(pAd, &SAQReqHdr, SUBTYPE_ACTION, 0, pEntry->Addr,pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_addr,
                                pAd->ApCfg.MBSSID[pEntry->apidx].wdev.bssid);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{	
		MgtMacHeaderInit(pAd, &SAQReqHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
							pAd->CurrentAddress,
							pEntry->Addr);
		}
#endif /* CONFIG_STA_SUPPORT */

                pEntry->TransactionID++;

        	SACategoryType = CATEGORY_SA;
        	SAActionType = ACTION_SAQ_REQUEST;
                MakeOutgoingFrame(pOutBuffer, (ULONG *) &FrameLen,
                                sizeof(HEADER_802_11), &SAQReqHdr,
                                1, &SACategoryType,
                                1, &SAActionType,
                                2, &pEntry->TransactionID,
                                END_OF_ARGS);

                if (pEntry->SAQueryStatus == SAQ_IDLE) {
                        RTMPSetTimer(&pEntry->SAQueryTimer, 1000); /* 1000ms */
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s -- SAQueryTimer\n", __FUNCTION__));
                }

                pEntry->SAQueryStatus = SAQ_SENDING;
                RTMPSetTimer(&pEntry->SAQueryConfirmTimer, 200); /* 200ms */

                /* transmit the frame */
                MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
                os_free_mem(NULL, pOutBuffer);

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - Send SA Query Request to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
								__FUNCTION__, PRINT_MAC(pEntry->Addr)));                
        }        
}


VOID PMF_PeerSAQueryReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
        UCHAR Action = Elem->Msg[LENGTH_802_11+1];

        if (Action == ACTION_SAQ_REQUEST)
        {
                PMAC_TABLE_ENTRY pEntry;
                PFRAME_802_11 pHeader;
                USHORT TransactionID;
                PUCHAR pOutBuffer = NULL;
                HEADER_802_11 SAQRspHdr;
                UINT32 FrameLen = 0;
                UCHAR SACategoryType, SAActionType;

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Receive SA Query Request\n", __FUNCTION__));
                pHeader = (PFRAME_802_11) Elem->Msg;

                pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);

                if (!pEntry)
                {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is not found, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));
                        return;
                }

                if (!(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
                {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));
                        return;
                }
        
                NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));

                /* Response the SA Query */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
                if(pOutBuffer == NULL)
                        return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{		
		MgtMacHeaderInit(pAd, &SAQRspHdr, SUBTYPE_ACTION, 0, pHeader->Hdr.Addr2,pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[pEntry->apidx].wdev.bssid);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{		
			MgtMacHeaderInit(pAd, &SAQRspHdr, SUBTYPE_ACTION, 0, pHeader->Hdr.Addr2,				
						pAd->CurrentAddress,
						pHeader->Hdr.Addr2);
		}
#endif /* CONFIG_STA_SUPPORT */

		SACategoryType = CATEGORY_SA;
		SAActionType = ACTION_SAQ_RESPONSE;
                MakeOutgoingFrame(pOutBuffer, (ULONG *) &FrameLen,
                                sizeof(HEADER_802_11), &SAQRspHdr,
                                1, &SACategoryType,
                                1, &SAActionType,
                                2, &TransactionID,
                                END_OF_ARGS);

                /* transmit the frame */
                MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
                os_free_mem(NULL, pOutBuffer);

		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - Send SA Query Response to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(SAQRspHdr.Addr1)));        
	}
}


VOID PMF_PeerSAQueryRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

        if (Action == ACTION_SAQ_RESPONSE)
        {
                PMAC_TABLE_ENTRY pEntry;
                PFRAME_802_11 pHeader;
                USHORT TransactionID;
                BOOLEAN Cancelled;

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Receive SA Query Response\n", __FUNCTION__));

                pHeader = (PFRAME_802_11) Elem->Msg;
				
                pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);

                if (!pEntry)
                {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is not found, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));
                        return;
                }

                if (!(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)))
                {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));
                        return;
                }
        
                NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));

                if (pEntry->TransactionID == TransactionID)
                {
                        pEntry->SAQueryStatus = SAQ_IDLE;
                        RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
                        RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - Compare TransactionID correctly, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));        
                } 
                else 
                {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - Compare TransactionID wrong, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__, PRINT_MAC(pHeader->Hdr.Addr2)));
                }
	}
}


VOID PMF_SAQueryTimeOut(
        IN PVOID SystemSpecific1, 
        IN PVOID FunctionContext, 
        IN PVOID SystemSpecific2, 
        IN PVOID SystemSpecific3) 
{
        MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

        if (pEntry)
        {
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

   		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
   					__FUNCTION__, PRINT_MAC(pEntry->Addr)));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{		
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			BOOLEAN Cancelled;
			MLME_DISASSOC_REQ_STRUCT DisassocReq;

			RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
			DisassocParmFill(pAd, &DisassocReq,
			pAd->CommonCfg.Bssid, REASON_DISASSOC_STA_LEAVING);
			MlmeEnqueue(pAd, ASSOC_STATE_MACHINE,
						MT2_MLME_DISASSOC_REQ,
						sizeof (MLME_DISASSOC_REQ_STRUCT),
						&DisassocReq, 0);
			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_DISASSOC;
		}
#endif /* CONFIG_STA_SUPPORT */
	}
}


VOID PMF_SAQueryConfirmTimeOut(
        IN PVOID SystemSpecific1, 
        IN PVOID FunctionContext, 
        IN PVOID SystemSpecific2, 
        IN PVOID SystemSpecific3) 
{
        MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

        if (pEntry)
        {
			PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s - STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
					__FUNCTION__, PRINT_MAC(pEntry->Addr)));
			pEntry->SAQueryStatus = SAQ_RETRY;
			PMF_MlmeSAQueryReq(pAd, pEntry);
        }
}


VOID PMF_ConstructBIPAad(
        IN PUCHAR pHdr,
	OUT UCHAR *aad_hdr)
{
	UINT8 aad_len = 0;

	/* Frame control -		
                Retry bit (bit 11) masked to 0
		PwrMgt bit (bit 12) masked to 0
		MoreData bit (bit 13) masked to 0 */
	aad_hdr[0] = (*pHdr);
	aad_hdr[1] = (*(pHdr + 1)) & 0xc7;
	aad_len = 2;
	
	/* Append Addr 1, 2 & 3 */
	NdisMoveMemory(&aad_hdr[aad_len], pHdr + 4, 3 * MAC_ADDR_LEN);
	aad_len += (3 * MAC_ADDR_LEN);
}


BOOLEAN PMF_CalculateBIPMIC(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAadHdr,
	IN PUCHAR pFrameBuf,
	IN UINT32 FrameLen,
	IN PUCHAR pKey,
	OUT PUCHAR pBipMic) 
{
	UCHAR *m_buf;
	UINT32 total_len;
	UCHAR cmac_output[16];
	UINT mlen = AES_KEY128_LENGTH;

	/* Allocate memory for MIC calculation */
	os_alloc_mem(NULL, (PUCHAR *)&m_buf, MGMT_DMA_BUFFER_SIZE);	
        if (m_buf == NULL)
        {
                DBGPRINT(RT_DEBUG_ERROR, ("%s : out of resource.\n", __FUNCTION__));
                return FALSE;
        }

	/* Initialize the buffer */
	NdisZeroMemory(m_buf, MGMT_DMA_BUFFER_SIZE);

	/* Construct the concatenation */
	NdisMoveMemory(m_buf, pAadHdr, LEN_PMF_BIP_AAD_HDR);
 	total_len = LEN_PMF_BIP_AAD_HDR;

	/* Append the Mgmt frame into the concatenation */
	NdisMoveMemory(&m_buf[total_len], pFrameBuf, FrameLen);
 	total_len += FrameLen;

	/* Compute AES-128-CMAC over the concatenation */	
        AES_CMAC(m_buf, total_len, pKey, 16, cmac_output, &mlen);

	/* Truncate the first 64-bits */
	NdisMoveMemory(pBipMic, cmac_output, LEN_PMF_BIP_MIC);

	os_free_mem(NULL, m_buf);

	return TRUE;

}


VOID PMF_DerivePTK(
	IN PRTMP_ADAPTER pAd, 
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT	len)
{	
	UCHAR concatenation[76];
	UINT CurrPos = 0;
	UCHAR temp[32];
	UCHAR Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ', 
			'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	/* initiate the concatenation input */
	NdisZeroMemory(temp, sizeof(temp));
	NdisZeroMemory(concatenation, 76);

	/* Get smaller address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(concatenation, AA, 6);
	else
		NdisMoveMemory(concatenation, SA, 6);
	CurrPos += 6;

	/* Get larger address */
	if (RTMPCompareMemory(SA, AA, 6) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SA, 6);
	else
		NdisMoveMemory(&concatenation[CurrPos], AA, 6);
		
	/* store the larger mac address for backward compatible of 
	   ralink proprietary STA-key issue */
	NdisMoveMemory(temp, &concatenation[CurrPos], MAC_ADDR_LEN);		
	CurrPos += 6;

	/* Get smaller Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	CurrPos += 32;

	/* Get larger Nonce */
	if (RTMPCompareMemory(ANonce, SNonce, 32) == 0)
		NdisMoveMemory(&concatenation[CurrPos], temp, 32);
	else if (RTMPCompareMemory(ANonce, SNonce, 32) == 1)
		NdisMoveMemory(&concatenation[CurrPos], ANonce, 32);
	else
		NdisMoveMemory(&concatenation[CurrPos], SNonce, 32);
	CurrPos += 32;

	hex_dump("[PMF]PMK", PMK, LEN_PMK);
	hex_dump("[PMF]concatenation=", concatenation, 76);

	/* Calculate a key material through FT-KDF */
	KDF(PMK, LEN_PMK, Prefix, 22, concatenation, 76, output, len);	
}


/*
        ========================================================================
	
	Routine Description:
		Derive IGTK randomly
		IGTK, a hierarchy consisting of a single key to provide integrity 
		protection for broadcast and multicast Robust Management frames

	Arguments:
		
	Return Value:

	Note:
		It's defined in IEEE 802.11w 8.5.1.3a
		
	========================================================================
*/
VOID PMF_DeriveIGTK(
        IN PRTMP_ADAPTER pAd,
        OUT UCHAR *output)
{
	INT i;

	for(i = 0; i < LEN_TK; i++)
                output[i] = RandomByte(pAd);   
}


/*
	========================================================================
	
	Routine Description:
		Insert IGTK KDE. The field shall be included in pair-Msg3-WPA2 and
		group-Msg1-WPA2. 

	Arguments:
		
	Return Value:

	Note:		
		
	========================================================================
*/
VOID PMF_InsertIGTKKDE(
	IN PRTMP_ADAPTER pAd,
	IN INT apidx,
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	PPMF_IGTK_KDE igtk_kde_ptr;
	UINT8 idx = 0;
	PPMF_CFG pPmfCfg = NULL;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pPmfCfg = &pAd->ApCfg.MBSSID[apidx].PmfCfg;
#endif /* CONFIG_AP_SUPPORT */

	if (!pPmfCfg)
		return;

	/* Construct the common KDE format */
	WPA_ConstructKdeHdr(KDE_IGTK, LEN_PMF_IGTK_KDE, pFrameBuf);

	/* Prepare the IGTK KDE */
	igtk_kde_ptr = (PPMF_IGTK_KDE)(pFrameBuf + LEN_KDE_HDR);	
	NdisZeroMemory(igtk_kde_ptr, LEN_PMF_IGTK_KDE);

	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception. 
	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	igtk_kde_ptr->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	idx = (pPmfCfg->IGTK_KeyIdx == 5) ? 1 : 0;
			
	/* Fill in the IPN field */
	NdisMoveMemory(igtk_kde_ptr->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);

	/* Fill uin the IGTK field */
	NdisMoveMemory(igtk_kde_ptr->IGTK, &pPmfCfg->IGTK[idx][0], LEN_AES_GTK);

	/* Update the total output length */
	*pFrameLen = *pFrameLen + LEN_KDE_HDR + LEN_PMF_IGTK_KDE;
	
	return; 
}


/*
	========================================================================
	
	Routine Description:
		Extract IGTK KDE.

	Arguments:
		
	Return Value:

	Note:		
		
	========================================================================
*/
BOOLEAN PMF_ExtractIGTKKDE(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBuf,
	IN INT buf_len)
{
	PPMF_IGTK_KDE igtk_kde_ptr;
	UINT8 idx = 0;
	UINT8 offset = 0;
	PPMF_CFG pPmfCfg = NULL;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pPmfCfg = &pAd->StaCfg.PmfCfg;
#endif /* CONFIG_STA_SUPPORT */

	if (pPmfCfg == NULL)
		return FALSE;
	
	igtk_kde_ptr = (PPMF_IGTK_KDE) pBuf;
	pPmfCfg->IGTK_KeyIdx = igtk_kde_ptr->KeyID[0];
	if (pPmfCfg->IGTK_KeyIdx == 5)
		idx = 1;
	offset += 2;

	NdisMoveMemory(&pPmfCfg->IPN[idx][0], igtk_kde_ptr->IPN, LEN_WPA_TSC);	
	offset += LEN_WPA_TSC;

	if ((buf_len - offset) == LEN_AES_GTK)
	{
		NdisMoveMemory(&pPmfCfg->IGTK[idx][0], igtk_kde_ptr->IGTK, LEN_AES_GTK);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : the IGTK length(%d) is invalid\n", 
					 __FUNCTION__, (buf_len - offset)));	
		return FALSE;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s : IGTK_Key_ID=%d\n", 
				__FUNCTION__, pPmfCfg->IGTK_KeyIdx));	
	return TRUE;
}


/*
	========================================================================
	
	Routine Description:
		Build Group Management Cipher in RSN-IE. 
		It only shall be called by RTMPMakeRSNIE. 

	Arguments:
		pAd - pointer to our pAdapter context	
    	        ElementID - indicate the WPA1 or WPA2    	
		apidx - indicate the interface index
		
	Return Value:
		
	Note:
		
	========================================================================
*/
VOID PMF_MakeRsnIeGMgmtCipher(
	IN  PRTMP_ADAPTER pAd,	
	IN UCHAR ElementID,
	IN UCHAR apidx,
	OUT PUCHAR pRsnIe,
	OUT UCHAR *rsn_len)
{
	PUINT8 pBuf;
	BOOLEAN MFP_Enabled = FALSE;

	/* it could be ignored in WPA1 mode */
	if (ElementID == WpaIe)
		return;
	
	pBuf = (pRsnIe + (*rsn_len));
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (apidx < pAd->ApCfg.BssidNum)		
		{
			PMULTISSID_STRUCT pMbss = &pAd->ApCfg.MBSSID[apidx];
		
			MFP_Enabled = pMbss->PmfCfg.MFPC;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{		
		MFP_Enabled = pAd->StaCfg.PmfCfg.MFPC;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* default group management cipher suite in an RSNA with 
	   Management Frame Protection enabled. */
	if (MFP_Enabled)
	{
		NdisMoveMemory(pBuf, OUI_PMF_BIP_CIPHER, LEN_OUI_SUITE);
		(*rsn_len) += sizeof(LEN_OUI_SUITE);
                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: Insert BIP to the group management cipher of RSNIE\n", __FUNCTION__));		
	}
}


/*	
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
NTSTATUS PMF_RsnCapableValidation(
        IN PRTMP_ADAPTER pAd,
        IN PUINT8 pRsnie,
        IN UINT rsnie_len,
        IN BOOLEAN self_MFPC,
	IN BOOLEAN self_MFPR,
        IN PMAC_TABLE_ENTRY pEntry)
{
	UINT8 count;
	PUINT8 pBuf = NULL;
	BOOLEAN	peer_MFPC = FALSE, peer_MFPR = FALSE;

	/* Check the peer's MPFC and MPFR -
	   Refer to Table 8-1a, IEEE 802.11W to check the PMF policy */
	if ((pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, RSN_CAP_INFO, &count)) == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : Peer's MPFC isn't used.\n", __FUNCTION__));
		if (self_MFPR)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : PMF policy violation.\n", __FUNCTION__));
			return PMF_POLICY_VIOLATION;			
		}			
	}
	else
	{
		RSN_CAPABILITIES RsnCap;

		NdisMoveMemory(&RsnCap, pBuf, sizeof(RSN_CAPABILITIES));				
		RsnCap.word = cpu2le16(RsnCap.word);

		peer_MFPC = RsnCap.field.MFPC;
		peer_MFPR = RsnCap.field.MFPR;

                if ((self_MFPC == TRUE) && (peer_MFPC == FALSE ))
		{
		        if ((self_MFPR == TRUE) && (peer_MFPR == FALSE))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : PMF policy violation for case 4\n", __FUNCTION__));
				return PMF_POLICY_VIOLATION;			
			}
                        
		        if (peer_MFPR == TRUE)
		        {
				DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : PMF policy violation for case 7\n", __FUNCTION__));
			        return PMF_POLICY_VIOLATION;						
		        }
	        }

		if ((self_MFPC == TRUE) && (peer_MFPC == TRUE))
		{
		        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: PMF Connection \n", __FUNCTION__));
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE);
		}
	}

        /* SHA1 or SHA256 */
        if ((self_MFPC == TRUE) 
                && (pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, AKM_SUITE, &count)) != NULL)
        {
                UCHAR OUI_WPA2_1X_SHA256[4] = {0x00, 0x0F, 0xAC, 0x05};
                UCHAR OUI_WPA2_PSK_SHA256[4] = {0x00, 0x0F, 0xAC, 0x06};
            
                while (count > 0)
                {
                        if(RTMPEqualMemory(pBuf,OUI_WPA2_1X_SHA256,4) || RTMPEqualMemory(pBuf,OUI_WPA2_PSK_SHA256,4) )
                        {
                                CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_USE_SHA256);
                                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : SHA256 Support\n", __FUNCTION__));
			}
                        pBuf += 4;
                        count--;
		}
	}
	
	return PMF_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
	Decide if the frame is PMF Robust frame

Arguments:
	pHdr		:	pointer to the 802.11 header
	pFrame		:	point to frame body. It exclude the 802.11 header
	frame_len	:	the frame length without 802.11 header

Return Value:
	NOT_ROBUST_FRAME
	UNICAST_ROBUST_FRAME
	GROUP_ROBUST_FRAME
	
Note:

========================================================================
*/
INT PMF_RobustFrameClassify(
	IN PHEADER_802_11 pHdr,
	IN PUCHAR pFrame,
	IN UINT	frame_len,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN IsRx)
{
	if ((pHdr->FC.Type != FC_TYPE_MGMT) || (frame_len <= 0))
		return NORMAL_FRAME;

	/* Classify the frame */
	switch (pHdr->FC.SubType)
        {
		case SUBTYPE_DISASSOC:
		case SUBTYPE_DEAUTH:
			break;
		case SUBTYPE_ACTION:
                {
                        if  ((IsRx == FALSE) 
                                || (IsRx && (pHdr->FC.Wep == 0)))
                        {
                                UCHAR Category = (UCHAR) (pHdr->Octet[0]);
                                
        			switch (Category)
        			{
        				/* Refer to IEEE 802.11w Table7-24 */
        				case CATEGORY_SPECTRUM:
        				case CATEGORY_QOS:
        				case CATEGORY_DLS:
        				case CATEGORY_BA:
        				case CATEGORY_RM:
        				case CATEGORY_FT:
        				case CATEGORY_SA:
        				case CATEGORY_PD:
        				case CATEGORY_VSP:
        					break;
        				default:
        					return NORMAL_FRAME;
        			}
                        }
			break;
		}
		default:
			return NORMAL_FRAME;
	}
	
	if (pHdr->Addr1[0] & 0x01) /* Broadcast frame */
	{
		UINT8 offset_mmie;

		if (frame_len <= (LEN_PMF_MMIE + 2))
                        return NOT_ROBUST_GROUP_FRAME;

		/* The offset of MMIE */
		offset_mmie = frame_len - (LEN_PMF_MMIE + 2);

		/* check if this is a group Robust frame */
		if (((*(pFrame + offset_mmie)) == IE_PMF_MMIE) && 
			((*(pFrame + offset_mmie + 1)) == LEN_PMF_MMIE))
			return GROUP_ROBUST_FRAME;
                else
                        return NOT_ROBUST_GROUP_FRAME;
        }
	
        /* Unicast frame */
        else if (pEntry == NULL)
		return NORMAL_FRAME;
        else if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE))
		return NORMAL_FRAME;
        else if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE) && (pHdr->FC.Wep == 0) && IsRx)
		return NOT_ROBUST_UNICAST_FRAME;
        else if (((IsRx == TRUE) && (pHdr->FC.Wep == 1)) || (IsRx == FALSE))
		return UNICAST_ROBUST_FRAME;

        return ERROR_FRAME;
}


INT PMF_EncryptUniRobustFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT mgmt_len)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	INT data_len;
	PUCHAR pBuf;
	INT Status;

	/* Check if the length is valid */
	data_len = mgmt_len - (LENGTH_802_11 + LEN_CCMP_HDR + LEN_CCMP_MIC);
	if (data_len <= 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The payload length(%d) is invalid\n", 
					__FUNCTION__, data_len));	
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Look up the entry through Address 1 of 802.11 header */
        pEntry = MacTableLookup(pAd, pHdr->Addr1);

	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The entry doesn't exist\n", __FUNCTION__));	
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* check the PMF capable for this entry */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE) == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : the entry no PMF capable !\n", __FUNCTION__));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}	

	/* Allocate a buffer for building PMF packet */
	Status = MlmeAllocateMemory(pAd, &pBuf);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : allocate PMF buffer fail!\n", __FUNCTION__));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	} 

	/* Construct and insert 8-bytes CCMP header to MPDU header */
	RTMPConstructCCMPHdr(0, pEntry->PmfTxTsc, pBuf);	

	NdisMoveMemory(pBuf + LEN_CCMP_HDR, 
				   &pHdr->Octet[0], 
				   data_len);

	// Encrypt the MPDU data by software
	RTMPSoftEncryptCCMP(pAd,
			(PUCHAR)pHdr,
			pEntry->PmfTxTsc,
			pEntry->PairwiseKey.Key,
			pBuf + LEN_CCMP_HDR,
			data_len);

	data_len += (LEN_CCMP_HDR + LEN_CCMP_MIC);		
	NdisMoveMemory(&pHdr->Octet[0], pBuf, data_len);

	/* TSC increment for next transmittion */
	INC_TX_TSC(pEntry->PmfTxTsc, LEN_WPA_TSC);	

	MlmeFreeMemory(pAd, pBuf);		

	return PMF_STATUS_SUCCESS;

}

INT PMF_DecryptUniRobustFrameAction(
	IN PRTMP_ADAPTER pAd,
	INOUT PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHeader = (PHEADER_802_11)pMgmtFrame;
	PUCHAR pDate = pMgmtFrame + LENGTH_802_11;
	UINT16 data_len = mgmt_len - LENGTH_802_11;


	/* Check if the length is valid */
	if (data_len <= LEN_CCMP_HDR + LEN_CCMP_MIC)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : The payload length(%d) is invalid\n", 
					__FUNCTION__, data_len));	
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : the entry doesn't exist !\n", __FUNCTION__));
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE) == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : the entry no PMF capable !\n", __FUNCTION__));
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

#ifdef RT_BIG_ENDIAN
      	if ((pHeader->FC.SubType == SUBTYPE_DISASSOC) || (pHeader->FC.SubType == SUBTYPE_DEAUTH)) {
		*(USHORT *)pDate = SWAP16(*(USHORT *)pDate); /* swap reason code */
   	}
#endif /* RT_BIG_ENDIAN */		
	
	if (RTMPSoftDecryptCCMP(pAd, 
				pMgmtFrame, 
				&pEntry->PairwiseKey, 
				pDate, 
				&data_len) == FALSE)
	{
		return PMF_UNICAST_DECRYPT_FAILURE;
	}
	
	return PMF_STATUS_SUCCESS;
}


INT PMF_EncapBIPAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{	
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	PPMF_CFG pPmfCfg = NULL;
	PPMF_MMIE pMMIE;
	INT idx = 0;
	PUCHAR pKey = NULL;
	UCHAR aad_hdr[LEN_PMF_BIP_AAD_HDR];
	UCHAR BIP_MIC[LEN_PMF_BIP_MIC];
	PUCHAR pFrameBody = &pHdr->Octet[0];
	UINT32 body_len = mgmt_len - LENGTH_802_11;				
#ifdef RT_BIG_ENDIAN
	PUCHAR pMacHdr = NULL;
	BOOLEAN bSwaped = FALSE;
#endif
	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : the total length(%d) is too short\n", 
									__FUNCTION__, body_len));	
		return PMF_ENCAP_BIP_FAILURE;	
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
                pPmfCfg = &pAd->ApCfg.MBSSID[MAIN_MBSSID].PmfCfg;
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Sanity check */
	if (pPmfCfg == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : No related PMF configuation\n", __FUNCTION__));	
		return PMF_ENCAP_BIP_FAILURE;
	}

	if (pPmfCfg && pPmfCfg->MFPC == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s : PMF is disabled \n", __FUNCTION__));	
		return PMF_ENCAP_BIP_FAILURE;
	}

	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));
	
	/*  Select the IGTK currently active for transmission of frames to 
	    the intended group of recipients and construct the MMIE (see 7.3.2.55) 
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pPmfCfg->IGTK_KeyIdx == 5)
		idx = 1;	
	pKey = &pPmfCfg->IGTK[idx][0];
	
	NdisZeroMemory(pMMIE, LEN_PMF_MMIE);

	/* Bits 0-11 define a value in the range 0-4095.
   	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception. 
   	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	pMMIE->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	NdisMoveMemory(pMMIE->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);

	/* The transmitter shall insert a monotonically increasing non-neg-
	   ative integer into the MMIE IPN field. */
	INC_TX_TSC(pPmfCfg->IPN[idx], LEN_WPA_TSC);	

	/* Compute AAD  */
#ifdef RT_BIG_ENDIAN
	if (pHdr->FC.SubType ==SUBTYPE_DISASSOC || pHdr->FC.SubType == SUBTYPE_DEAUTH)
	{
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr); //swap frame-control
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);	//swap reason code
		bSwaped= TRUE;
	}
#endif
	PMF_ConstructBIPAad((PUCHAR)pHdr, aad_hdr);

	/* Calculate BIP MIC */
	PMF_CalculateBIPMIC(pAd, aad_hdr, pFrameBody, body_len, pKey, BIP_MIC);

	/* Fill into the MMIE MIC field */
	NdisMoveMemory(pMMIE->MIC, BIP_MIC, LEN_PMF_BIP_MIC);

#ifdef RT_BIG_ENDIAN
	if (bSwaped)
	{
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = SWAP16(*(USHORT *)pMacHdr);	
		bSwaped= TRUE;
	}
#endif
	/* BIP doesn't need encrypt frame */
	pHdr->FC.Wep = 0;

	return PMF_STATUS_SUCCESS;	
}

INT PMF_ExtractBIPAction(
	IN PRTMP_ADAPTER pAd,
	INOUT PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PPMF_CFG pPmfCfg = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PHEADER_802_11 pHeader = (PHEADER_802_11)pMgmtFrame;
	PPMF_MMIE pMMIE;
	INT idx = 0;
	PUCHAR pKey = NULL;
	UCHAR aad_hdr[LEN_PMF_BIP_AAD_HDR];
	UCHAR rcvd_mic[LEN_PMF_BIP_MIC];
	UCHAR cal_mic[LEN_PMF_BIP_MIC];
	UINT32 body_len = mgmt_len - LENGTH_802_11;	


	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : the total length(%d) is too short\n", 
					__FUNCTION__, body_len));	
		return PMF_EXTRACT_BIP_FAILURE;	
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : the entry doesn't exist !\n", __FUNCTION__));
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE) == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s : the entry no PMF capable !\n", __FUNCTION__));
		return PMF_EXTRACT_BIP_FAILURE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
                pPmfCfg = &pAd->ApCfg.MBSSID[pEntry->apidx].PmfCfg;
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{		
        	pPmfCfg = &pAd->StaCfg.PmfCfg;	
        }                
#endif // CONFIG_STA_SUPPORT //

	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));

	/*  Select the IGTK currently active for transmission of frames to 
	    the intended group of recipients and construct the MMIE (see 7.3.2.55) 
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pMMIE->KeyID[0] == 5)
		idx = 1;	
	pKey = &pPmfCfg->IGTK[idx][0];

	/* store the MIC value of the received frame */
	NdisMoveMemory(rcvd_mic, pMMIE->MIC, LEN_PMF_BIP_MIC);
	NdisZeroMemory(pMMIE->MIC, LEN_PMF_BIP_MIC);

	/* Compute AAD  */
	PMF_ConstructBIPAad((PUCHAR)pMgmtFrame, aad_hdr);

	/* Calculate BIP MIC */
	PMF_CalculateBIPMIC(pAd, aad_hdr, 
                        pMgmtFrame + LENGTH_802_11, 
                        body_len, pKey, cal_mic);

	if (!NdisEqualMemory(rcvd_mic, cal_mic, LEN_PMF_BIP_MIC))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : MIC Different !\n", __FUNCTION__));
		return PMF_EXTRACT_BIP_FAILURE;	
	}

	return PMF_STATUS_SUCCESS;
}


BOOLEAN	PMF_PerformTxFrameAction(
	IN PRTMP_ADAPTER pAd,
	OUT PNDIS_PACKET pPacket)
{
	PHEADER_802_11 pHeader_802_11;
	PACKET_INFO PacketInfo;
	PUCHAR pSrcBufVA;
	UINT SrcBufLen;
        UINT8 TXWISize = pAd->chipCap.TXWISize;
	INT FrameType;
	INT ret = 0;
	PMAC_TABLE_ENTRY pEntry = NULL;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NORMAL_FRAME;
        
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TXWISize);

	pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);

	FrameType = PMF_RobustFrameClassify(
				(PHEADER_802_11)pHeader_802_11,
				(PUCHAR)( ((PUCHAR) pHeader_802_11) + LENGTH_802_11),
				(SrcBufLen - LENGTH_802_11 - TXINFO_SIZE - TXWISize),
				pEntry,
				FALSE);
				
	switch (FrameType)
	{
		case ERROR_FRAME:
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: ERROR FRAME\n", __FUNCTION__));
			return FALSE;
		case NORMAL_FRAME:
                case NOT_ROBUST_GROUP_FRAME:   
                case NOT_ROBUST_UNICAST_FRAME:
			break;
		case UNICAST_ROBUST_FRAME:
		{
			int tailroom =0 ;

			tailroom = GET_OS_PKT_END(pPacket) - GET_OS_PKT_DATATAIL(pPacket);
                        pHeader_802_11->FC.Wep = 1;

                        /* Format: Mac header|CCMP header|Data|MIC|FCS */
			if (tailroom < (LEN_CCMP_HDR + LEN_CCMP_MIC))
			{
				int headroom =GET_OS_PKT_DATAPTR(pPacket) - GET_OS_PKT_HEAD(pPacket) ;
				pPacket = OS_PKT_COPY_EXPAND(pPacket, headroom, (LEN_CCMP_HDR + LEN_CCMP_MIC));
			}

			OS_PKT_TAIL_BUF_EXTEND(pPacket, (LEN_CCMP_HDR + LEN_CCMP_MIC));

                	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
                	if (pSrcBufVA == NULL)
                		return NORMAL_FRAME;
                        
                	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TXWISize);

			ret = PMF_EncryptUniRobustFrameAction(pAd, 
			        			(PUCHAR) pHeader_802_11, 
				        		(SrcBufLen - TXINFO_SIZE - TXWISize));
		        break;
		}
		case GROUP_ROBUST_FRAME:	
		{
			ret = PMF_EncapBIPAction(pAd, 
						(PUCHAR) pHeader_802_11, 
						(SrcBufLen - TXINFO_SIZE - TXWISize));
		        break;
	        }
	}

        if (ret == PMF_STATUS_SUCCESS)
                return TRUE;
        else
                return FALSE;                
}


/*
========================================================================
Routine Description:

Arguments:
	
Return Value:

Note:

========================================================================
*/
BOOLEAN	PMF_PerformRxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK *pRxBlk)
{
	INT FrameType;
	PUCHAR pMgmtFrame;
	UINT mgmt_len;
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	PMAC_TABLE_ENTRY pEntry = NULL;

	pMgmtFrame = (PUCHAR)pHeader;
	mgmt_len = pRxBlk->MPDUtotalByteCnt;

	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if(pEntry == NULL)
		return TRUE;

	if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE))
		return TRUE;

	FrameType = PMF_RobustFrameClassify(pHeader,
					(PUCHAR)(pMgmtFrame + LENGTH_802_11),
                                        (mgmt_len - LENGTH_802_11),
                                        pEntry,
                                        TRUE);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	switch (FrameType)
	{
		case ERROR_FRAME:
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: ERROR FRAME\n", __FUNCTION__));
			return FALSE;
		case NORMAL_FRAME:
                case NOT_ROBUST_GROUP_FRAME:   
			break;
                case NOT_ROBUST_UNICAST_FRAME:
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: ERROR FRAME\n", __FUNCTION__));
			return FALSE;
			case UNICAST_ROBUST_FRAME:
			{
				if (PMF_DecryptUniRobustFrameAction(pAd, 
								pMgmtFrame, 
								mgmt_len) != PMF_STATUS_SUCCESS)
	        			return FALSE;

			/* update the total length */
			pRxBlk->MPDUtotalByteCnt -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
	        	break;
		}
		case GROUP_ROBUST_FRAME:
		{
			if (PMF_ExtractBIPAction(pAd, 
				pMgmtFrame, 
				mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

			/* update the total length */
			pRxBlk->MPDUtotalByteCnt -= (2 + LEN_PMF_MMIE);
			break;
	        }
	}	
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		RXWI_STRUC *pRxWI = pRxBlk->pRxWI;

		switch (FrameType)
		{
			case ERROR_FRAME:
	                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: ERROR FRAME\n", __FUNCTION__));
				return FALSE;
		case NORMAL_FRAME:
			break;
                case NOT_ROBUST_UNICAST_FRAME:
                        if (((pHeader->FC.SubType == SUBTYPE_DISASSOC) || (pHeader->FC.SubType == SUBTYPE_DEAUTH))
                                && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE))
                        {
                                PMF_MlmeSAQueryReq(pAd, pEntry);
                                return FALSE;
                        }
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s: ERROR FRAME\n", __FUNCTION__));
			return FALSE;
                case NOT_ROBUST_GROUP_FRAME:   
                        if ((pEntry) && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE))
                                return FALSE;
                        else
                                break;
		case UNICAST_ROBUST_FRAME:
		{
			if (PMF_DecryptUniRobustFrameAction(pAd, 
							pMgmtFrame, 
							mgmt_len) != PMF_STATUS_SUCCESS)
        			return FALSE;

			/* update the total length */
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT)
			{
				pRxWI->RXWI_N.MPDUtotalByteCnt -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
			}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP) {
					pRxWI->MPDUtotalByteCount -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
			}
#endif /* RTMP_MAC */			
	        	break;
		}
		case GROUP_ROBUST_FRAME:
		{
			if (PMF_ExtractBIPAction(pAd, 
						pMgmtFrame, 
						mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

			/* update the total length */
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT)
			{
				pRxWI->RXWI_N.MPDUtotalByteCnt -= (2 + LEN_PMF_MMIE);
			}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP) {
				pRxWI->MPDUtotalByteCount -= (2 + LEN_PMF_MMIE);
			}
#endif /* RTMP_MAC */
		        break;
	        }
	}
	}	
#endif /* CONFIG_STA_SUPPORT */												
	return TRUE;
}



/*	
========================================================================
Routine Description:
    Protection Management Frame Capable
    Protection Management Frame Required

Arguments:

Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
void rtmp_read_pmf_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING tmpbuf,
	IN PSTRING pBuffer)
{
	PSTRING macptr;

#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {
                INT apidx;
                POS_COOKIE pObj;
            
                pObj = (POS_COOKIE) pAd->OS_Cookie;
                for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
                {
                        pAd->ApCfg.MBSSID[apidx].PmfCfg.Desired_MFPC = FALSE;
                        pAd->ApCfg.MBSSID[apidx].PmfCfg.Desired_MFPR = FALSE;
                        pAd->ApCfg.MBSSID[apidx].PmfCfg.Desired_PMFSHA256 = FALSE;
                }

                /* Protection Management Frame Capable */
                if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, 32, pBuffer, TRUE))
                {
                        for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
                        {
                                pObj->ioctl_if = apidx;
                                Set_PMFMFPC_Proc(pAd, macptr);
                        }
		}

        	/* Protection Management Frame Required */
	        if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, 32, pBuffer, TRUE))
	        {
                        for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
                        {
                                pObj->ioctl_if = apidx;
                                Set_PMFMFPR_Proc(pAd, macptr);
                        }	        
                }

        	if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, 32, pBuffer, TRUE))
        	{
                        for (apidx = 0, macptr = rstrtok(tmpbuf,";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL,";"), apidx++)
                        {
                                pObj->ioctl_if = apidx;
                                Set_PMFSHA256_Proc(pAd, macptr);
                        }        	
                }
        }
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        {
                pAd->StaCfg.PmfCfg.Desired_MFPC = FALSE; 
                pAd->StaCfg.PmfCfg.Desired_MFPR = FALSE; 
                pAd->StaCfg.PmfCfg.Desired_PMFSHA256 = FALSE; 
                
                /* Protection Management Frame Capable */
                if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, 32, pBuffer, TRUE))
                        Set_PMFMFPC_Proc(pAd, tmpbuf);

        	/* Protection Management Frame Required */
	        if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, 32, pBuffer, TRUE))
	                Set_PMFMFPR_Proc(pAd, tmpbuf);

        	if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, 32, pBuffer, TRUE))
        	        Set_PMFSHA256_Proc(pAd, tmpbuf);
        }
#endif /* CONFIG_STA_SUPPORT */
}


/*
========================================================================
Routine Description: Protection Management Frame Capable

Arguments:
	
Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
/* chane the cmd depend on security mode first, and update to run time flag */
INT Set_PMFMFPC_Proc (
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
 	if(strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {
                POS_COOKIE pObj;

                pObj = (POS_COOKIE) pAd->OS_Cookie;
                if (simple_strtol(arg, 0, 10))			
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPC = TRUE; 
                else
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPC = FALSE; 

    		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: apidx=%d, Desired MFPC=%d\n", __FUNCTION__
                , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPC));
        }
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{									
		if (simple_strtol(arg, 0, 10))			
			pAd->StaCfg.PmfCfg.Desired_MFPC = TRUE; 
		else
		{
			pAd->StaCfg.PmfCfg.Desired_MFPC = FALSE; 
			pAd->StaCfg.PmfCfg.MFPC = FALSE;
			pAd->StaCfg.PmfCfg.MFPR = FALSE;
			//dont need to clear the SHA256
		}

        	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired MFPC=%d\n", __FUNCTION__
                , pAd->StaCfg.PmfCfg.Desired_MFPC));


		{
			if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2 || pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
				&& (pAd->StaCfg.wdev.WepStatus == Ndis802_11AESEnable))
			{
				pAd->StaCfg.PmfCfg.PMFSHA256 = pAd->StaCfg.PmfCfg.Desired_PMFSHA256;
				if (pAd->StaCfg.PmfCfg.Desired_MFPC)
				{
					pAd->StaCfg.PmfCfg.MFPC = TRUE;
					pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.Desired_MFPR;

					if (pAd->StaCfg.PmfCfg.MFPR)
						pAd->StaCfg.PmfCfg.PMFSHA256 = TRUE;
				}
			} else if (pAd->StaCfg.PmfCfg.Desired_MFPC) {
				DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
			}

			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
						__FUNCTION__, pAd->StaCfg.PmfCfg.MFPC, pAd->StaCfg.PmfCfg.MFPR,
						pAd->StaCfg.PmfCfg.PMFSHA256));
		}


	}
#endif /* CONFIG_STA_SUPPORT */
    	return TRUE;
}


/*
========================================================================
Routine Description: Protection Management Frame Required

Arguments:
	
Return Value:

Note:
RSNA policy selection in a ESS: IEEE P802.11w Table 8-1a
RSNA policy selection in an IBSS: IEEE P802.11w Table 8-1b
========================================================================
*/
/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_PMFMFPR_Proc (
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
 	if(strlen(arg) == 0)
		return FALSE;
	
#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {					
                POS_COOKIE pObj;

                pObj = (POS_COOKIE) pAd->OS_Cookie;            
                if (simple_strtol(arg, 0, 10))			
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPR = TRUE; 
                else
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPR = FALSE; 

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: apidx=%d, Desired MFPR=%d\n", __FUNCTION__
                , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_MFPR));
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{									
                if (simple_strtol(arg, 0, 10))			
                        pAd->StaCfg.PmfCfg.Desired_MFPR = TRUE; 
                else
                {
                        pAd->StaCfg.PmfCfg.Desired_MFPR = FALSE;
                        //only close the MFPR
                        pAd->StaCfg.PmfCfg.MFPR = FALSE;
                }

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired MFPR=%d\n", __FUNCTION__
                , pAd->StaCfg.PmfCfg.Desired_MFPR));

		{
			if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2 || pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
				&& (pAd->StaCfg.wdev.WepStatus == Ndis802_11AESEnable))
			{
				pAd->StaCfg.PmfCfg.PMFSHA256 = pAd->StaCfg.PmfCfg.Desired_PMFSHA256;
				if (pAd->StaCfg.PmfCfg.Desired_MFPC)
				{
					pAd->StaCfg.PmfCfg.MFPC = TRUE;
					pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.Desired_MFPR;

					if (pAd->StaCfg.PmfCfg.MFPR)
						pAd->StaCfg.PmfCfg.PMFSHA256 = TRUE;
				}
			} else if (pAd->StaCfg.PmfCfg.Desired_MFPC) {
				DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
			}

			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
						__FUNCTION__, pAd->StaCfg.PmfCfg.MFPC, pAd->StaCfg.PmfCfg.MFPR,
						pAd->StaCfg.PmfCfg.PMFSHA256));
		}

				
        }
#endif /* CONFIG_STA_SUPPORT */
    	return TRUE;
}


INT Set_PMFSHA256_Proc (
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
 	if(strlen(arg) == 0)
		return FALSE;
	
#ifdef CONFIG_AP_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
        {					
                POS_COOKIE pObj;

                pObj = (POS_COOKIE) pAd->OS_Cookie;            
                if (simple_strtol(arg, 0, 10))			
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256 = TRUE; 
                else
                        pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256 = FALSE; 

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: apidx=%d, Desired PMFSHA256=%d\n", __FUNCTION__
                , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256));
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
        IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{									
                if (simple_strtol(arg, 0, 10))			
                        pAd->StaCfg.PmfCfg.Desired_PMFSHA256 = TRUE; 
                else
                        pAd->StaCfg.PmfCfg.Desired_PMFSHA256 = FALSE; 

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired PMFSHA256=%d\n", __FUNCTION__
                , pAd->StaCfg.PmfCfg.Desired_PMFSHA256));
        }
#endif /* CONFIG_STA_SUPPORT */
    	return TRUE;
}

#endif /* DOT11W_PMF_SUPPORT */

