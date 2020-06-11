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

UCHAR OUI_PMF_BIP_CMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_PMF_BIP_CMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0d};
UCHAR OUI_PMF_BIP_GMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0b};
UCHAR OUI_PMF_BIP_GMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0c};

/* The definition in IEEE 802.11w - Table 7-34 AKM suite selectors */
UCHAR OUI_PMF_8021X_AKM[4] = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_PMF_PSK_AKM[4] = {0x00, 0x0F, 0xAC, 0x06};

UCHAR PMF_MMIE_BUFFER[18] = {0x4C, 0x10,
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
							};

#define SAQ_IDLE	0
#define SAQ_RETRY	1
#define SAQ_SENDING	2


VOID PMF_PeerAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_WARN, ("[PMF]%s : PMF_PeerAction Action=%d\n", __func__, Action));

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
	IN MAC_TABLE_ENTRY * pEntry)
{
	PUCHAR pOutBuffer = NULL;
	HEADER_802_11 SAQReqHdr;
	ULONG FrameLen = 0;
	UCHAR SACategoryType, SAActionType;
	PPMF_CFG pPmfCfg = NULL;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is NULL\n", __func__));
		return;
	}

	pPmfCfg = &pEntry->SecConfig.PmfCfg;

	if (pPmfCfg) {
		if ((pPmfCfg->UsePMFConnect == FALSE)) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pEntry->Addr)));
			return;
		}

		if (pPmfCfg->SAQueryStatus == SAQ_SENDING)
			return;

		/* Send the SA Query Request */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

		if (pOutBuffer == NULL)
			return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if ((pEntry) && IS_ENTRY_APCLI(pEntry)) {
				ApCliMgtMacHeaderInit(pAd,
									  &SAQReqHdr,
									  SUBTYPE_ACTION, 0,
									  pEntry->Addr,
									  pEntry->Addr,
									  pEntry->func_tb_idx);
			} else
#endif /* APCLI_SUPPORT */
			{
				MgtMacHeaderInit(pAd, &SAQReqHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		pPmfCfg->TransactionID++;
		SACategoryType = CATEGORY_SA;
		SAActionType = ACTION_SAQ_REQUEST;
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &SAQReqHdr,
						  1, &SACategoryType,
						  1, &SAActionType,
						  2, &pPmfCfg->TransactionID,
						  END_OF_ARGS);

		if (pPmfCfg->SAQueryStatus == SAQ_IDLE) {
			RTMPSetTimer(&pPmfCfg->SAQueryTimer, 1000); /* 1000ms */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s -- SAQueryTimer\n", __func__));
		}

		pPmfCfg->SAQueryStatus = SAQ_SENDING;
		RTMPSetTimer(&pPmfCfg->SAQueryConfirmTimer, 200); /* 200ms */
		/* transmit the frame */
		MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
		os_free_mem(pOutBuffer);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - Send SA Query Request to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
				 __func__, PRINT_MAC(pEntry->Addr)));
	}
}


VOID PMF_PeerSAQueryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	if (Action == ACTION_SAQ_REQUEST) {
		PMAC_TABLE_ENTRY pEntry;
		PFRAME_802_11 pHeader;
		USHORT TransactionID;
		PUCHAR pOutBuffer = NULL;
		HEADER_802_11 SAQRspHdr;
		ULONG FrameLen = 0;
		UCHAR SACategoryType, SAActionType;
		PPMF_CFG pPmfCfg = NULL;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Receive SA Query Request\n", __func__));
		pHeader = (PFRAME_802_11) Elem->Msg;
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);
#endif

		if (!pEntry) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is not found, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
			return;
		}

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
			return;
		}

		/* Fix PMF 5.3.3.4 un-protect SA Query Req. Need to ignore. */
		if (pHeader->Hdr.FC.Wep == 0) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : un-Protected SA Query Req.!!! Drop it!!, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
			return;
		}

		NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));
		/* Response the SA Query */
		os_alloc_mem(NULL, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);

		if (pOutBuffer == NULL)
			return;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if (pEntry && IS_ENTRY_APCLI(pEntry)) {
				ApCliMgtMacHeaderInit(pAd, &SAQRspHdr,
									  SUBTYPE_ACTION, 0,
									  pHeader->Hdr.Addr2,
									  pHeader->Hdr.Addr2,
									  pEntry->func_tb_idx);
			} else
#endif /* APCLI_SUPPORT */
			{
				MgtMacHeaderInit(pAd, &SAQRspHdr, SUBTYPE_ACTION, 0, pHeader->Hdr.Addr2,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid,
								 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		SACategoryType = CATEGORY_SA;
		SAActionType = ACTION_SAQ_RESPONSE;
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &SAQRspHdr,
						  1, &SACategoryType,
						  1, &SAActionType,
						  2, &TransactionID,
						  END_OF_ARGS);
		/* transmit the frame */
		MiniportMMRequest(pAd, QID_MGMT, pOutBuffer, FrameLen);
		os_free_mem(pOutBuffer);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - Send SA Query Response to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(SAQRspHdr.Addr1)));
	}
}


VOID PMF_PeerSAQueryRspAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	if (Action == ACTION_SAQ_RESPONSE) {
		PMAC_TABLE_ENTRY pEntry;
		PFRAME_802_11 pHeader;
		USHORT TransactionID;
		BOOLEAN Cancelled;
		PPMF_CFG pPmfCfg = NULL;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Receive SA Query Response\n", __func__));
		pHeader = (PFRAME_802_11) Elem->Msg;
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader->Hdr.Addr2);
#endif

		if (!pEntry) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is not found, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
			return;
		}

		pPmfCfg = &pEntry->SecConfig.PmfCfg;

		if (pPmfCfg->UsePMFConnect == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Entry is not PMF capable, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
			return;
		}

		NdisMoveMemory(&TransactionID, &Elem->Msg[LENGTH_802_11+2], sizeof(USHORT));

		if (pPmfCfg->TransactionID == TransactionID) {
			pPmfCfg->SAQueryStatus = SAQ_IDLE;
			RTMPCancelTimer(&pPmfCfg->SAQueryTimer, &Cancelled);
			RTMPCancelTimer(&pPmfCfg->SAQueryConfirmTimer, &Cancelled);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - Compare TransactionID correctly, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
		} else
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - Compare TransactionID wrong, STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pHeader->Hdr.Addr2)));
	}
}


VOID PMF_SAQueryTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;
#ifdef APCLI_SUPPORT
	USHORT ifIndex = 0;
#endif /* APCLI_SUPPORT */

	if (pEntry) {
		RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pEntry->pAd;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
				 __func__, PRINT_MAC(pEntry->Addr)));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_SUPPORT

			if (IS_ENTRY_APCLI(pEntry)) {
				BOOLEAN Cancelled;
				MLME_DISASSOC_REQ_STRUCT DisassocReq;
				PULONG pCurrState = NULL;

				RTMPCancelTimer(&pEntry->SecConfig.PmfCfg.SAQueryTimer, &Cancelled);
				RTMPCancelTimer(&pEntry->SecConfig.PmfCfg.SAQueryConfirmTimer, &Cancelled);
				DisassocParmFill(pAd, &DisassocReq,
								 pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].MlmeAux.Bssid,
								 REASON_DISASSOC_STA_LEAVING);
				pCurrState = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].CtrlCurrState;
				ifIndex = pEntry->func_tb_idx;
				*pCurrState = APCLI_CTRL_DEASSOC;
				MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE,
							APCLI_MT2_MLME_DISASSOC_REQ,
							sizeof(MLME_DISASSOC_REQ_STRUCT), &DisassocReq, ifIndex);
				RTMP_MLME_HANDLER(pAd);
			} else
#endif /* APCLI_SUPPORT */
				mac_entry_delete(pAd, pEntry);
		}
#endif /* CONFIG_AP_SUPPORT */
	}
}


VOID PMF_SAQueryConfirmTimeOut(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

	if (pEntry) {
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s - STA(%02x:%02x:%02x:%02x:%02x:%02x)\n", __func__, PRINT_MAC(pEntry->Addr)));
		pEntry->SecConfig.PmfCfg.SAQueryStatus = SAQ_RETRY;
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
	os_alloc_mem(NULL, (PUCHAR *)&m_buf, MAX_MGMT_PKT_LEN);

	if (m_buf == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : out of resource.\n", __func__));
		return FALSE;
	}

	/* Initialize the buffer */
	NdisZeroMemory(m_buf, MAX_MGMT_PKT_LEN);
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
	os_free_mem(m_buf);
	return TRUE;
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

	for (i = 0; i < LEN_MAX_IGTK; i++)
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
	UINT8 igtk_len = 0;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	pPmfCfg = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg;
#endif /* CONFIG_AP_SUPPORT */

	if (!pPmfCfg)
		return;

	/* Decide the IGTK length */
	if (IS_CIPHER_BIP_CMAC128(pPmfCfg->igtk_cipher)
		|| IS_CIPHER_BIP_GMAC128(pPmfCfg->igtk_cipher))
		igtk_len = LEN_BIP128_IGTK;
	else if (IS_CIPHER_BIP_CMAC256(pPmfCfg->igtk_cipher)
		|| IS_CIPHER_BIP_GMAC256(pPmfCfg->igtk_cipher))
		igtk_len = LEN_BIP256_IGTK;
	else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
			("%s : unknown igtk cipher %x.\n", __func__, pPmfCfg->igtk_cipher));
		return;
	}

	/* Construct the common KDE format */
	WPA_ConstructKdeHdr(KDE_IGTK, 8 + igtk_len, pFrameBuf);
	/* Prepare the IGTK KDE */
	igtk_kde_ptr = (PPMF_IGTK_KDE)(pFrameBuf + LEN_KDE_HDR);
	NdisZeroMemory(igtk_kde_ptr, 8 + igtk_len);
	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception.
	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	igtk_kde_ptr->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	idx = (pPmfCfg->IGTK_KeyIdx == 5) ? 1 : 0;
	/* Fill in the IPN field */
	NdisMoveMemory(igtk_kde_ptr->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);
	/* Fill uin the IGTK field */
	NdisMoveMemory(igtk_kde_ptr->IGTK, &pPmfCfg->IGTK[idx][0], igtk_len);
	/* Update the total output length */
	*pFrameLen = *pFrameLen + LEN_KDE_HDR + 8 + igtk_len;
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
	IN PUCHAR pBuf,
	IN INT buf_len,
	OUT PUCHAR IGTK,
	OUT UCHAR *IGTKLEN,
	OUT PUCHAR IPN,
	OUT UINT8 *IGTK_KeyIdx)
{
	PPMF_IGTK_KDE igtk_kde_ptr;
	UINT8 offset = 0;

	igtk_kde_ptr = (PPMF_IGTK_KDE) pBuf;
	*IGTK_KeyIdx = igtk_kde_ptr->KeyID[0];
	offset += 2;
	NdisMoveMemory(IPN, igtk_kde_ptr->IPN, LEN_WPA_TSC);
	offset += LEN_WPA_TSC;

	if ((buf_len - offset) == LEN_BIP128_IGTK) {
		NdisMoveMemory(IGTK, igtk_kde_ptr->IGTK, LEN_BIP128_IGTK);
		*IGTKLEN = LEN_BIP128_IGTK;
	} else if ((buf_len - offset) == LEN_BIP256_IGTK) {
		NdisMoveMemory(IGTK, igtk_kde_ptr->IGTK, LEN_BIP256_IGTK);
		*IGTKLEN = LEN_BIP256_IGTK;
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the IGTK length(%d) is invalid\n",
				 __func__, (buf_len - offset)));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s : IGTK_Key_ID=%d, IGTK length=%d\n",
			 __func__, *IGTK_KeyIdx, *IGTKLEN));
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
BOOLEAN PMF_MakeRsnIeGMgmtCipher(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	UINT8 *pBuf = (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));

	if (pSecConfig->RSNE_Type[ie_idx] == SEC_RSNIE_WPA2_IE) {
		/* default group management cipher suite in an RSNA with
		      Management Frame Protection enabled. */
		if (pSecConfig->PmfCfg.MFPC == TRUE) {
			if (IS_CIPHER_BIP_CMAC128(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_CMAC256(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_GMAC128(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE);
			else if (IS_CIPHER_BIP_GMAC256(pSecConfig->PmfCfg.igtk_cipher))
				NdisMoveMemory(pBuf, OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE);
			else {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
					("[PMF]%s: insert fail, IGTK cipher is wrong\n", __func__));
				return FALSE;
			}
			(*rsn_len) += sizeof(LEN_OUI_SUITE);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: Insert BIP to the group management cipher of RSNIE\n", __func__));
		}
	}

	return TRUE;
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
	IN PUINT8 pRsnie,
	IN UINT rsnie_len,
	IN BOOLEAN self_MFPC,
	IN BOOLEAN self_MFPR,
	IN UINT32 self_igtk_cipher,
	IN UCHAR end_field,
	IN struct _SECURITY_CONFIG *pSecConfigEntry)
{
	UINT8 count;
	PUINT8 pBuf = NULL;
	BOOLEAN	peer_MFPC = FALSE, peer_MFPR = FALSE;
	/* Check the peer's MPFC and MPFR -
	   Refer to Table 8-1a, IEEE 802.11W to check the PMF policy */
	pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, RSN_CAP_INFO, &count);

	if (pBuf == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Peer's MPFC isn't used.\n", __func__));

		if (self_MFPR) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : PMF policy violation.\n", __func__));
			return PMF_POLICY_VIOLATION;
		}
	} else {
		RSN_CAPABILITIES RsnCap;

		NdisMoveMemory(&RsnCap, pBuf, sizeof(RSN_CAPABILITIES));
		RsnCap.word = cpu2le16(RsnCap.word);
		peer_MFPC = RsnCap.field.MFPC;
		peer_MFPR = RsnCap.field.MFPR;



		if ((self_MFPC == TRUE) && (peer_MFPC == FALSE)) {
			if ((self_MFPR == TRUE) && (peer_MFPR == FALSE)) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : PMF policy violation for case 4\n", __func__));
				return PMF_POLICY_VIOLATION;
			}

			if (peer_MFPR == TRUE) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : PMF policy violation for case 7\n", __func__));
				return PMF_POLICY_VIOLATION;
			}
		}

		if ((self_MFPC == TRUE) && (peer_MFPC == TRUE)) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: PMF Connection\n", __func__));
			pSecConfigEntry->PmfCfg.UsePMFConnect = TRUE;
		}

		if (IS_AKM_OWE(pSecConfigEntry->AKMMap)) {
			pSecConfigEntry->PmfCfg.MFPC = 1;
			pSecConfigEntry->PmfCfg.MFPR = 1;
		}

		if (IS_AKM_SAE(pSecConfigEntry->AKMMap) && (peer_MFPC == FALSE)) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				("[PMF]%s: SAE connection fail due to not PMF connection(peer MFPR = %d, MFPC = %d)\n", __func__, peer_MFPR, peer_MFPC));
			return PMF_POLICY_VIOLATION;
		}
	}

	/* SHA1 or SHA256 */
	pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, AKM_SUITE, &count);

	if ((self_MFPC == TRUE)
		&& (pBuf != NULL)) {
		UCHAR OUI_WPA2_1X_SHA256[4] = {0x00, 0x0F, 0xAC, 0x05};
		UCHAR OUI_WPA2_PSK_SHA256[4] = {0x00, 0x0F, 0xAC, 0x06};
		UCHAR OUI_WPA2_SAE_SHA256[4] = {0x00, 0x0F, 0xAC, 0x08};

		while (count > 0) {
			if (RTMPEqualMemory(pBuf, OUI_WPA2_1X_SHA256, 4)
				|| RTMPEqualMemory(pBuf, OUI_WPA2_PSK_SHA256, 4)
				|| RTMPEqualMemory(pBuf, OUI_WPA2_SAE_SHA256, 4)) {
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
				MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : SHA256 Support\n", __func__));
			}

			pBuf += 4;
			count--;
		}
	}

	/* Group Management Cipher Suite */
	if (pSecConfigEntry->PmfCfg.UsePMFConnect == TRUE) {
		if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER
			&& IS_CIPHER_BIP_CMAC128(self_igtk_cipher)) {
			pSecConfigEntry->PmfCfg.igtk_cipher = self_igtk_cipher;
			return PMF_STATUS_SUCCESS;
		} else if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				("[PMF]%s : The peer Group_mgmt_cipher_suite(default) is mismatch\n", __func__));
			return PMF_POLICY_VIOLATION;
		}

		pBuf = WPA_ExtractSuiteFromRSNIE(pRsnie, rsnie_len, G_MGMT_SUITE, &count);

		if (pBuf == NULL) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				("[PMF]%s : The peer RSNIE doesn't include Group_mgmt_cipher_suite\n", __func__));
			return PMF_POLICY_VIOLATION;
		}

		if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_CMAC128(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_CMAC256(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_GMAC128(pSecConfigEntry->PmfCfg.igtk_cipher);
		else if (RTMPEqualMemory(pBuf, OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE))
			SET_CIPHER_BIP_GMAC256(pSecConfigEntry->PmfCfg.igtk_cipher);
		else {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				("[PMF]%s : unknown peer Group_mgmt_cipher_suite\n", __func__));
			hex_dump("peer Group_mgmt_cipher_suite", pBuf, LEN_OUI_SUITE);
			return PMF_POLICY_VIOLATION;
		}

		if ((pSecConfigEntry->PmfCfg.igtk_cipher & self_igtk_cipher) == 0) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				("[PMF]%s : peer Group_mgmt_cipher_suite(%s) is mismatch\n",
				__func__, GetEncryModeStr(pSecConfigEntry->PmfCfg.igtk_cipher)));
			CLEAR_CIPHER(pSecConfigEntry->PmfCfg.igtk_cipher);
			return PMF_POLICY_VIOLATION;
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
	IN PUCHAR pData,
	IN BOOLEAN IsRx)
{
	PMAC_TABLE_ENTRY pEntry = (PMAC_TABLE_ENTRY) pData;

	if ((pHdr->FC.Type != FC_TYPE_MGMT) || (frame_len <= 0))
		return NORMAL_FRAME;

	/* Classify the frame */
	switch (pHdr->FC.SubType) {
	case SUBTYPE_DISASSOC:
	case SUBTYPE_DEAUTH:
		break;

	case SUBTYPE_ACTION: {
		if  ((IsRx == FALSE)
			 || (IsRx && (pHdr->FC.Wep == 0))) {
			UCHAR Category = (UCHAR) (pHdr->Octet[0]);

			switch (Category) {
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
			case CATEGORY_WNM:
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

	if (pHdr->Addr1[0] & 0x01) { /* Broadcast frame */
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
	else if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
		return NORMAL_FRAME;
	else if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) && (pHdr->FC.Wep == 0) && (IsRx == TRUE))
		return NOT_ROBUST_UNICAST_FRAME;
	else if (((IsRx == TRUE) && (pHdr->FC.Wep == 1)) || (IsRx == FALSE))
		return UNICAST_ROBUST_FRAME;

	return ERROR_FRAME;
}

#ifdef SOFT_ENCRYPT
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

	if (data_len <= 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : The payload length(%d) is invalid\n",
				 __func__, data_len));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Look up the entry through Address 1 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHdr->Addr1);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : The entry doesn't exist\n", __func__));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the entry no PMF capable !\n", __func__));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Allocate a buffer for building PMF packet */
	Status = MlmeAllocateMemory(pAd, &pBuf);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : allocate PMF buffer fail!\n", __func__));
		return PMF_UNICAST_ENCRYPT_FAILURE;
	}

	/* Construct and insert 8-bytes CCMP header to MPDU header */
	RTMPConstructCCMPHdr(0, pEntry->PmfTxTsc, pBuf);
	NdisMoveMemory(pBuf + LEN_CCMP_HDR,
				   &pHdr->Octet[0],
				   data_len);
	/* Encrypt the MPDU data by software */
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
	MlmeFreeMemory(pBuf);
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
	if (data_len <= LEN_CCMP_HDR + LEN_CCMP_MIC) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : The payload length(%d) is invalid\n",
				 __func__, data_len));
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the entry doesn't exist !\n", __func__));
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the entry no PMF capable !\n", __func__));
		return PMF_UNICAST_DECRYPT_FAILURE;
	}

#ifdef RT_BIG_ENDIAN

	if ((pHeader->FC.SubType == SUBTYPE_DISASSOC) || (pHeader->FC.SubType == SUBTYPE_DEAUTH))
		*(USHORT *)pDate = cpu2le16(*(USHORT *)pDate); /* swap reason code */

#endif /* RT_BIG_ENDIAN */

	if (RTMPSoftDecryptCCMP(pAd,
							pMgmtFrame,
							&pEntry->PairwiseKey,
							pDate,
							&data_len) == FALSE)
		return PMF_UNICAST_DECRYPT_FAILURE;

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
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : the total length(%d) is too short\n",
				 __func__, body_len));
		return PMF_ENCAP_BIP_FAILURE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pPmfCfg = &pAd->ApCfg.MBSSID[MAIN_MBSSID].PmfCfg;
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Sanity check */
	if (pPmfCfg == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : No related PMF configuation\n", __func__));
		return PMF_ENCAP_BIP_FAILURE;
	}

	if (pPmfCfg && pPmfCfg->MFPC == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : PMF is disabled\n", __func__));
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

	if (pHdr->FC.SubType == SUBTYPE_DISASSOC || pHdr->FC.SubType == SUBTYPE_DEAUTH) {
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr); /* swap frame-control */
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);	/* swap reason code */
		bSwaped = TRUE;
	}

#endif
	PMF_ConstructBIPAad((PUCHAR)pHdr, aad_hdr);
	/* Calculate BIP MIC */
	PMF_CalculateBIPMIC(pAd, aad_hdr, pFrameBody, body_len, pKey, BIP_MIC);
	/* Fill into the MMIE MIC field */
	NdisMoveMemory(pMMIE->MIC, BIP_MIC, LEN_PMF_BIP_MIC);
#ifdef RT_BIG_ENDIAN

	if (bSwaped) {
		pMacHdr = (PUCHAR) pHdr;
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);
		pMacHdr += sizeof(HEADER_802_11);
		*(USHORT *)pMacHdr = cpu2le16(*(USHORT *)pMacHdr);
		bSwaped = TRUE;
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
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the total length(%d) is too short\n",
				 __func__, body_len));
		return PMF_EXTRACT_BIP_FAILURE;
	}

	/* Look up the entry through Address 2 of 802.11 header */
	pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the entry doesn't exist !\n", __func__));
		return PMF_STATUS_SUCCESS;
	}

	/* check the PMF capable for this entry */
	if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : the entry no PMF capable !\n", __func__));
		return PMF_EXTRACT_BIP_FAILURE;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* MT7615 only need H/W  Decrypt, SOFT_ENCRYPT of BIP is not need */
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
			pPmfCfg = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;
		else
#endif /* APCLI_SUPPORT */
			pPmfCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;
	}
#endif /* CONFIG_AP_SUPPORT // */
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

	if (!NdisEqualMemory(rcvd_mic, cal_mic, LEN_PMF_BIP_MIC)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("%s : MIC Different !\n", __func__));
		return PMF_EXTRACT_BIP_FAILURE;
	}

	return PMF_STATUS_SUCCESS;
}
#endif /* SOFT_ENCRYPT */


VOID PMF_AddMMIE(
	IN PMF_CFG * pPmfCfg,
	IN PUCHAR pMgmtFrame,
	IN UINT	mgmt_len)
{
	PHEADER_802_11 pHdr = (PHEADER_802_11)pMgmtFrame;
	PPMF_MMIE pMMIE;
	INT idx = 0;
	UINT32 body_len = mgmt_len - LENGTH_802_11;

	/* Sanity check the total frame body length */
	if (body_len <= (2 + LEN_PMF_MMIE)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : the total length(%d) is too short\n",
				 __func__, body_len));
		return;
	}

	/* Sanity check */
	if (pPmfCfg == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : No related PMF configuation\n", __func__));
		return;
	}

	if (pPmfCfg && pPmfCfg->MFPC == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : PMF is disabled\n", __func__));
		return;
	}

	/* Pointer to the position of MMIE */
	pMMIE = (PPMF_MMIE)(pMgmtFrame + (mgmt_len - LEN_PMF_MMIE));

	/*  Select the IGTK currently active for transmission of frames to
	    the intended group of recipients and construct the MMIE (see 7.3.2.55)
	    with the MIC field masked to zero and the KeyID field set to the
	    corresponding IGTK KeyID value. */
	if (pPmfCfg->IGTK_KeyIdx == 5)
		idx = 1;

	os_zero_mem((char *) pMMIE, LEN_PMF_MMIE);
	/* Bits 0-11 define a value in the range 0-4095.
	   Bits 12 - 15 are reserved and set to 0 on transmission and ignored on reception.
	   The IGTK Key ID is either 4 or 5. The remaining Key IDs are reserved. */
	pMMIE->KeyID[0] = pPmfCfg->IGTK_KeyIdx;
	os_move_mem(pMMIE->IPN, &pPmfCfg->IPN[idx][0], LEN_WPA_TSC);
	/* The transmitter shall insert a monotonically increasing non-neg-
	   ative integer into the MMIE IPN field. */
	INC_TX_TSC(pPmfCfg->IPN[idx], LEN_WPA_TSC);
	/* BIP doesn't need encrypt frame */
	pHdr->FC.Wep = 0;
	return;
}


BOOLEAN PMF_PerformTxFrameAction(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pHeader_802_11,
	IN UINT SrcBufLen,
	IN UINT8 tx_hw_hdr_len,
	OUT UCHAR *prot)
{
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		UINT32 ret = 0;
		MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
#endif
		ret = PMF_RobustFrameClassify(
				  (PHEADER_802_11)pHeader_802_11,
				  (PUCHAR)(((PUCHAR)pHeader_802_11)+LENGTH_802_11),
				  (SrcBufLen - LENGTH_802_11 - tx_hw_hdr_len),
				  (PUCHAR) pEntry,
				  FALSE);

		if (ret == UNICAST_ROBUST_FRAME) {
			*prot = 1;
			pHeader_802_11->FC.Wep = 1;
		} else if (ret == GROUP_ROBUST_FRAME) {
#ifdef SOFT_ENCRYPT
			ret = PMF_EncapBIPAction(pAd,
									 (UCHAR *) pHeader_802_11,
									 (SrcBufLen - tx_hw_hdr_len));

			if (ret == PMF_STATUS_SUCCESS)
				*prot = 3;
			else
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, PMF GROUP ROBUST Encap fail, ret=%d\n",
						 __func__, ret));

#else
			PMF_CFG *pPmfCfg = NULL;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				struct wifi_dev *wdev = NULL;

				if (pHeader_802_11->Addr1[0] & 0x01) /* Broadcast frame */
				  wdev = wdev_search_by_address(pAd, pHeader_802_11->Addr2);
				else if (pEntry != NULL)
				  wdev = &pAd->ApCfg.MBSSID[pEntry->apidx].wdev;

				if (wdev)
				  pPmfCfg = &wdev->SecConfig.PmfCfg;
				else
				  pPmfCfg = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.SecConfig.PmfCfg;
			}
#endif /* CONFIG_AP_SUPPORT */
			PMF_AddMMIE(pPmfCfg,
						(UCHAR *) pHeader_802_11,
						(SrcBufLen - tx_hw_hdr_len));
			if (IS_CIPHER_BIP_CMAC128(pPmfCfg->igtk_cipher))
				*prot = 2;
			else
				*prot = 3;
#endif /* SOFT_ENCRYPT */
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s, PMF GROUP ROBUST\n", __func__));
		}
	}

	return TRUE;
}


/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
BOOLEAN PMF_PerformRxFrameAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	INT FrameType = NORMAL_FRAME;
	PUCHAR pMgmtFrame;
	UINT mgmt_len;
	HEADER_802_11 Header;
	PMAC_TABLE_ENTRY pEntry = NULL;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;

	pMgmtFrame = (PUCHAR)FC;
	mgmt_len = pRxBlk->MPDUtotalByteCnt;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

	if ((pRxBlk->Addr1[0] & 0x01) &&
		(FC->Type == FC_TYPE_MGMT) &&
		((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH))) {
#ifdef CONFIG_AP_SUPPORT
		/* MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: Bcast  FRAME, FC->SubType=%d, pRxBlk->pRxInfo->U2M=%d, pRxBlk->pRxInfo->disasso=%d, pRxBlk->pRxInfo->Decrypted=%d, pRxBlk->wcid=%d\n", */
		/* __FUNCTION__, FC->SubType, pRxBlk->pRxInfo->U2M, pRxBlk->pRxInfo->disasso, pRxBlk->pRxInfo->Decrypted,pRxBlk->wcid)); */
		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);
#endif

		if (!pEntry)
			return TRUE;
		else {
			if (pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
				return TRUE;
		}
	}

#ifdef MT_MAC

	if ((IS_HIF_TYPE(pAd, HIF_MT))
		&& (pEntry != NULL)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
		&& (FC->Type == FC_TYPE_MGMT)
		&& (!(pRxBlk->Addr1[0] & 0x01))
		&& ((FC->SubType == SUBTYPE_DISASSOC)  || (FC->SubType == SUBTYPE_DEAUTH) || (FC->SubType == SUBTYPE_ACTION))) {
		if (pRxBlk->CipherMis) {
			NdisZeroMemory(&Header, sizeof(HEADER_802_11));
			NdisMoveMemory(&Header.Addr1, &pRxBlk->Addr1[0], 6);
			NdisMoveMemory(&Header.FC, pRxBlk->FC, sizeof(FRAME_CONTROL));
			FrameType = PMF_RobustFrameClassify(&Header,
												(PUCHAR)(pMgmtFrame + LENGTH_802_11),
												(mgmt_len - LENGTH_802_11),
												(PUCHAR) pEntry,
												TRUE);
		}
	} else
#endif
	{
		NdisZeroMemory(&Header, sizeof(HEADER_802_11));
		NdisMoveMemory(&Header.Addr1, &pRxBlk->Addr1[0], 6);
		NdisMoveMemory(&Header.FC, pRxBlk->FC, sizeof(FRAME_CONTROL));
		FrameType = PMF_RobustFrameClassify(&Header,
											(PUCHAR)(pMgmtFrame + LENGTH_802_11),
											(mgmt_len - LENGTH_802_11),
											(PUCHAR) pEntry,
											TRUE);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		switch (FrameType) {
		case ERROR_FRAME:
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: ERROR FRAME, FC->SubType=%d\n", __func__, FC->SubType));
			return FALSE;

		case NORMAL_FRAME:
			break;

		case NOT_ROBUST_GROUP_FRAME:
#ifdef APCLI_SUPPORT

			/* H/W Decrypt case won't fall into below case */
			if ((pEntry) && IS_ENTRY_APCLI(pEntry)) {
				if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
					return FALSE;
			}

#endif /* APCLI_SUPPORT */
			break;

		case NOT_ROBUST_UNICAST_FRAME:
#ifdef APCLI_SUPPORT
			if ((pEntry) && IS_ENTRY_APCLI(pEntry)) {
				if (((Header.FC.SubType == SUBTYPE_DISASSOC) || (Header.FC.SubType == SUBTYPE_DEAUTH))
					&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)) {
					PMF_MlmeSAQueryReq(pAd, pEntry);
					return FALSE;
				}
			}

#endif /* APCLI_SUPPORT */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: ERROR FRAME, FC->SubType=%d\n", __func__, FC->SubType));
			return FALSE;

		case UNICAST_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (PMF_DecryptUniRobustFrameAction(pAd,
												pMgmtFrame,
												mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

			pRxBlk->MPDUtotalByteCnt -= (LEN_CCMP_HDR + LEN_CCMP_MIC);
#endif /* SOFT_ENCRYPT */
			break;
		}

		case GROUP_ROBUST_FRAME: {
#ifdef SOFT_ENCRYPT

			if (PMF_ExtractBIPAction(pAd,
									 pMgmtFrame,
									 mgmt_len) != PMF_STATUS_SUCCESS)
				return FALSE;

#endif /* SOFT_ENCRYPT */
			/* MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s: GROUP_ROBUST_FRAME, FC->SubType=%d\n", __FUNCTION__, FC->SubType)); */
			pRxBlk->MPDUtotalByteCnt -= (2 + LEN_PMF_MMIE);
			break;
		}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
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
	IN RTMP_STRING * tmpbuf,
	IN RTMP_STRING * pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT apidx;
		POS_COOKIE pObj;
		RTMP_STRING *macptr;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;
			pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;
		}

		/* Protection Management Frame Capable */
		if (RTMPGetKeyParameter("PMFMFPC", tmpbuf, 32, pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPC_Proc(pAd, macptr);
			}
		}

		/* Protection Management Frame Required */
		if (RTMPGetKeyParameter("PMFMFPR", tmpbuf, 32, pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFMFPR_Proc(pAd, macptr);
			}
		}

		if (RTMPGetKeyParameter("PMFSHA256", tmpbuf, 32, pBuffer, TRUE)) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				pObj->ioctl_if = apidx;
				Set_PMFSHA256_Proc(pAd, macptr);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
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
INT Set_PMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPC = TRUE;
		else
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPC = FALSE;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s:: apidx=%d, Desired MFPC=%d\n", __func__
				 , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPC));
	}
#endif /* CONFIG_AP_SUPPORT */
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
INT Set_PMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPR = TRUE;
		else
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPR = FALSE;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s:: apidx=%d, Desired MFPR=%d\n", __func__
				 , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_MFPR));
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}


INT Set_PMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	if (strlen(arg) == 0)
		return FALSE;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		POS_COOKIE pObj;

		pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (os_str_tol(arg, 0, 10))
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = TRUE;
		else
			pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_PMFSHA256 = FALSE;

		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s:: apidx=%d, Desired PMFSHA256=%d\n", __func__
				 , pObj->ioctl_if, pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig.PmfCfg.Desired_PMFSHA256));
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Set_PMFSA_Q_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
#ifdef CONFIG_AP_SUPPORT
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	CHAR Wcid = os_str_tol(arg, 0, 10);

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	pMacEntry = &pAd->MacTab.Content[Wcid];
	tr_entry = &pAd->MacTab.tr_entry[Wcid];
	if ((pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
		(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				 ("%s: PMF Connection IGNORE THIS PKT DUE TO NOT IN PORTSECURED\n", __func__));
		return FALSE;
	}

	if (pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR,
				 ("%s: PMF CONNECTION BUT RECV WEP=0 ACTION, ACTIVE THE SA QUERY\n", __func__));
		PMF_MlmeSAQueryReq(pAd, pMacEntry);
		return TRUE;
	}
#endif /* CONFIG_AP_SUPPORT */
	return FALSE;

}


#endif /* DOT11W_PMF_SUPPORT */

