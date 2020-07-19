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

/* --------------------EddySEC Start---------------- */

UCHAR SES_OUI[] = {0x00, 0x90, 0x4c};

/* WPA OUI*/
UCHAR OUI_WPA[3]                           = {0x00, 0x50, 0xF2};
UCHAR OUI_WPA_NONE_AKM[4]     = {0x00, 0x50, 0xF2, 0x00};
UCHAR OUI_WPA_VERSION[4]          = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_WEP40[4]             = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_TKIP[4]                  = {0x00, 0x50, 0xF2, 0x02};
UCHAR OUI_WPA_CCMP[4]               = {0x00, 0x50, 0xF2, 0x04};
UCHAR OUI_WPA_WEP104[4]           = {0x00, 0x50, 0xF2, 0x05};
UCHAR OUI_WPA_8021X_AKM[4]     = {0x00, 0x50, 0xF2, 0x01};
UCHAR OUI_WPA_PSK_AKM[4]         = {0x00, 0x50, 0xF2, 0x02};

/* WPA2 OUI*/
UCHAR OUI_WPA2[3]					   = {0x00, 0x0F, 0xAC};
UCHAR OUI_WPA2_CIPHER[3]                        = {0x00, 0x0F, 0xAC};
UCHAR OUI_WPA2_CIPHER_WEP40[4]          = {0x00, 0x0F, 0xAC, 0x01};
UCHAR OUI_WPA2_CIPHER_TKIP[4]               = {0x00, 0x0F, 0xAC, 0x02};
UCHAR OUI_WPA2_CIPHER_CCMP128[4]       = {0x00, 0x0F, 0xAC, 0x04};
UCHAR OUI_WPA2_CIPHER_WEP104[4]         = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_WPA2_CIPHER_BIPCMAC128[4]  = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_WPA2_CIPHER_GCMP128[4]       = {0x00, 0x0F, 0xAC, 0x08};
UCHAR OUI_WPA2_CIPHER_GCMP256[4]       = {0x00, 0x0F, 0xAC, 0x09};
UCHAR OUI_WPA2_CIPHER_CCMP256[4]       = {0x00, 0x0F, 0xAC, 0x0A};
UCHAR OUI_WPA2_CIPHER_BIPGMAC128[4] = {0x00, 0x0F, 0xAC, 0x0B};
UCHAR OUI_WPA2_CIPHER_BIPGMAC256[4] = {0x00, 0x0F, 0xAC, 0x0C};
UCHAR OUI_WPA2_CIPHER_BIPCMAC256[4]  = {0x00, 0x0F, 0xAC, 0x0D};

UCHAR OUI_WPA2_AKM_8021X[4]                      = {0x00, 0x0F, 0xAC, 0x01};
UCHAR OUI_WPA2_AKM_PSK[4]                          = {0x00, 0x0F, 0xAC, 0x02};
UCHAR OUI_WPA2_AKM_FT_8021X[4]                = {0x00, 0x0F, 0xAC, 0x03};
UCHAR OUI_WPA2_AKM_FT_PSK[4]                    = {0x00, 0x0F, 0xAC, 0x04};
UCHAR OUI_WPA2_AKM_8021X_SHA256[4]       = {0x00, 0x0F, 0xAC, 0x05};
UCHAR OUI_WPA2_AKM_PSK_SHA256[4]           = {0x00, 0x0F, 0xAC, 0x06};
UCHAR OUI_WPA2_AKM_TDLS[4]                        = {0x00, 0x0F, 0xAC, 0x07};
UCHAR OUI_WPA2_AKM_SAE_SHA256[4]           = {0x00, 0x0F, 0xAC, 0x08};
UCHAR OUI_WPA2_AKM_FT_SAE_SHA256[4]     = {0x00, 0x0F, 0xAC, 0x09};
UCHAR OUI_WPA2_AKM_SUITEB_SHA256[4]      = {0x00, 0x0F, 0xAC, 0x0B};
UCHAR OUI_WPA2_AKM_SUITEB_SHA384[4]      = {0x00, 0x0F, 0xAC, 0x0C};
UCHAR OUI_WPA2_AKM_FT_8021X_SHA384[4]  = {0x00, 0x0F, 0xAC, 0x0D};
UCHAR OUI_WPA2_AKM_OWE[4]  = {0x00, 0x0F, 0xAC, 0x12/*d'18*/};


BUILD_TIMER_FUNCTION(WPAStartFor4WayExec);
BUILD_TIMER_FUNCTION(WPAStartFor2WayExec);
BUILD_TIMER_FUNCTION(WPAHandshakeMsgRetryExec);
/* --------------------EddySEC END---------------- */



#ifdef DOT11R_FT_SUPPORT
UCHAR	OUI_FT_8021X_AKM[4]	= {0x00, 0x0F, 0xAC, 0x03};
UCHAR	OUI_FT_PSK_AKM[4]		= {0x00, 0x0F, 0xAC, 0x04};
#endif /* DOT11R_FT_SUPPORT */

#ifdef CONFIG_HOTSPOT_R2
UCHAR	OSEN_IE[] = {0x50, 0x6f, 0x9a, 0x12, 0x00, 0x0f, 0xac, 0x07, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x04, 0x01, 0x00, 0x50, 0x6f, 0x9a, 0x01, 0x00, 0x00};
UCHAR	OSEN_IELEN = sizeof(OSEN_IE);
#endif

static VOID WpaEAPPacketAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLASFAlertAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLLogoffAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLStartAction(
	IN PRTMP_ADAPTER    pAd,
	IN MLME_QUEUE_ELEM * Elem);

static VOID WpaEAPOLKeyAction(
	IN PRTMP_ADAPTER    pAd,
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
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_WPA_PTK_STATE, MAX_WPA_MSG, (STATE_MACHINE_FUNC)Drop, WPA_PTK, WPA_MACHINE_BASE);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPPacket, (STATE_MACHINE_FUNC)WpaEAPPacketAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLStart, (STATE_MACHINE_FUNC)WpaEAPOLStartAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLLogoff, (STATE_MACHINE_FUNC)WpaEAPOLLogoffAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLKey, (STATE_MACHINE_FUNC)WpaEAPOLKeyAction);
	StateMachineSetAction(S, WPA_PTK, MT2_EAPOLASFAlert, (STATE_MACHINE_FUNC)WpaEAPOLASFAlertAction);
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
	IN MLME_QUEUE_ELEM * Elem)
{
}

VOID WpaEAPOLASFAlertAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
}

VOID WpaEAPOLLogoffAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
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
	OUT	INT * MsgType)
{
	switch (EAPType) {
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
	int	pos = 0;
	int	carry = 0;
	UCHAR	pre_iv;

	while (pos < len) {
		pre_iv = iv[pos];

		if (carry == 1)
			iv[pos]++;
		else
			iv[pos] += cnt;

		if (iv[pos] > pre_iv)
			break;

		carry = 1;
		pos++;
	}

	if (pos >= len)
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("!!! inc_iv_byte overflow !!!\n"));

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
	if (msg == EAPOL_PAIR_MSG_1)
		return "Pairwise Message 1";
	else if (msg == EAPOL_PAIR_MSG_2)
		return "Pairwise Message 2";
	else if (msg == EAPOL_PAIR_MSG_3)
		return "Pairwise Message 3";
	else if (msg == EAPOL_PAIR_MSG_4)
		return "Pairwise Message 4";
	else if (msg == EAPOL_GROUP_MSG_1)
		return "Group Message 1";
	else if (msg == EAPOL_GROUP_MSG_2)
		return "Group Message 2";
	else
		return "Invalid Message";
}


UCHAR RTMPExtractKeyIdxFromIVHdr(UCHAR *pIV, UINT8 CipherAlg)
{
	UCHAR keyIdx = 0xFF;

	/* extract the key index from IV header */
	switch (CipherAlg) {
	case Ndis802_11WEPEnabled:
	case Ndis802_11TKIPEnable:
	case Ndis802_11AESEnable:
	case Ndis802_11GroupWEP40Enabled:
	case Ndis802_11GroupWEP104Enabled:
		keyIdx = (*(pIV + 3) & 0xc0) >> 6;
		break;
	}

	return keyIdx;
}


#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
CIPHER_KEY *RTMPSwCipherKeySelection(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *pIV,
	IN RX_BLK * pRxBlk,
	IN MAC_TABLE_ENTRY * pEntry)
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
		CipherAlg = pEntry->WepStatus;
	else {
	}

	keyIdx = RTMPExtractKeyIdxFromIVHdr(pIV, CipherAlg);

	if (keyIdx > 3) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : Invalid key index(%d) !!!\n",
				 __func__, keyIdx));
		return NULL;
	}

	if ((CipherAlg == Ndis802_11WEPEnabled)
		|| (CipherAlg == Ndis802_11GroupWEP40Enabled)
		|| (CipherAlg == Ndis802_11GroupWEP104Enabled))
		pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
	else if ((CipherAlg == Ndis802_11TKIPEnable) ||
			 (CipherAlg == Ndis802_11AESEnable)) {
		if (pRxBlk->pRxInfo->U2M)
			pKey = &pEntry->PairwiseKey;
		else {
				pKey = &pAd->SharedKey[pEntry->func_tb_idx][keyIdx];
		}
	}

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
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 * DataByteCnt)
{
	switch (pKey->CipherAlg) {
	case CIPHER_WEP64:
	case CIPHER_WEP128:

		/* handle WEP decryption */
		if (RTMPSoftDecryptWEP((PSEC_KEY_INFO) pKey, pData, &(*DataByteCnt)) == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR : SW decrypt WEP data fails.\n"));
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

	case CIPHER_TKIP:

		/* handle TKIP decryption */
		if (RTMPSoftDecryptTKIP(pAd, pHdr, UserPriority,
								pKey, pData, &(*DataByteCnt)) == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR : SW decrypt TKIP data fails.\n"));
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

	case CIPHER_AES:

		/* handle AES decryption */
		if (RTMPSoftDecryptCCMP(pAd, pHdr, pKey, pData, &(*DataByteCnt)) == FALSE) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR : SW decrypt AES data fails.\n"));
			/* give up this frame*/
			return NDIS_STATUS_FAILURE;
		}

		break;

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
	OUT PUCHAR			pHdrIv,
	OUT	UINT8			*hdr_iv_len)
{
	*hdr_iv_len = 0;
		if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128)) {
			/* Construct and insert 4-bytes WEP IV header to MPDU header */
			RTMPConstructWEPIVHdr(key_id, pTxIv, pHdrIv);
			*hdr_iv_len = LEN_WEP_IV_HDR;
		} else if (CipherAlg == CIPHER_TKIP)
			;
		else if (CipherAlg == CIPHER_AES) {
			/* Construct and insert 8-bytes CCMP header to MPDU header */
			RTMPConstructCCMPHdr(key_id, pTxIv, pHdrIv);
			*hdr_iv_len = LEN_CCMP_HDR;
		}
}

VOID RTMPSoftEncryptionAction(
	IN	PRTMP_ADAPTER pAd,
	IN	UCHAR			CipherAlg,
	IN	PUCHAR			pHdr,
	IN	PUCHAR			pSrcBufData,
	IN	UINT32			SrcBufLen,
	IN	UCHAR			KeyIdx,
	IN	PCIPHER_KEY		pKey,
	OUT	UINT8			*ext_len)
{
	*ext_len = 0;
		if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128)) {
			/* Encrypt the MPDU data by software*/
			RTMPSoftEncryptWEP(pKey->TxTsc,
							   pKey,
							   pSrcBufData,
							   SrcBufLen);
			*ext_len = LEN_ICV;
		} else if (CipherAlg == CIPHER_TKIP)
			;
		else if (CipherAlg == CIPHER_AES) {
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
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */

PUINT8	WPA_ExtractSuiteFromRSNIE(
	IN	PUINT8	rsnie,
	IN	UINT	rsnie_len,
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
	if ((len <= 0) || (pEid->Len != len)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The length is invalid\n", __func__));
		goto out;
	}

	/* Check WPA or WPA2*/
	if (pEid->Eid == IE_WPA) {
		/* Check the length */
		if (len < sizeof(RSNIE)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The length is too short for WPA\n", __func__));
			goto out;
		} else {
			PRSNIE	pRsnie;
			UINT16	u_cnt;

			pRsnie = (PRSNIE)pBuf;
			u_cnt = cpu2le16(pRsnie->ucount);
			offset = sizeof(RSNIE) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for WPA-RSN\n",
						 __func__, offset, len));
				goto out;
			} else {
				/* Get the group cipher*/
				if (type == GROUP_SUITE) {
					*count = 1;
					return pRsnie->mcast;
				}
				/* Get the pairwise cipher suite*/
				else if (type == PAIRWISE_SUITE) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The count of pairwise cipher is %d\n",
							 __func__, u_cnt));
					*count = u_cnt;
					return pRsnie->ucast[0].oui;
				}
			}
		}
	} else if (pEid->Eid == IE_RSN) {
		if (len < sizeof(RSNIE2)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The length is too short for WPA2\n", __func__));
			goto out;
		} else {
			PRSNIE2	pRsnie2;
			UINT16	u_cnt;

			pRsnie2 = (PRSNIE2)pBuf;
			u_cnt = cpu2le16(pRsnie2->ucount);
			offset = sizeof(RSNIE2) + (LEN_OUI_SUITE * (u_cnt - 1));

			if (len < offset) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for WPA2-RSN\n",
						 __func__, offset, len));
				goto out;
			} else {
				/* Get the group cipher*/
				if (type == GROUP_SUITE) {
					*count = 1;
					return pRsnie2->mcast;
				}
				/* Get the pairwise cipher suite*/
				else if (type == PAIRWISE_SUITE) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The count of pairwise cipher is %d\n",
							 __func__, u_cnt));
					*count = u_cnt;
					return pRsnie2->ucast[0].oui;
				}
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : Unknown IE (%d)\n", __func__, pEid->Eid));
		goto out;
	}

	/* skip group cipher and pairwise cipher suite	*/
	pBuf += offset;
	len -= offset;

	/* Ready to extract the AKM information and its count */
	if (len < sizeof(RSNIE_AUTH)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The length of AKM of RSN is too short\n", __func__));
		goto out;
	} else {
		PRSNIE_AUTH	pAkm;
		UINT16		a_cnt;
		/* pointer to AKM count */
		pAkm = (PRSNIE_AUTH)pBuf;
		a_cnt = cpu2le16(pAkm->acount);
		offset = sizeof(RSNIE_AUTH) + (LEN_OUI_SUITE * (a_cnt - 1));

		if (len < offset) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) for AKM\n",
					 __func__, offset, len));
			goto out;
		} else {
			/* Get the AKM suite */
			if (type == AKM_SUITE) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The count of AKM is %d\n",
						 __func__, a_cnt));
				*count = a_cnt;
				return pAkm->auth[0].oui;
			}
		}
	}

	/* For WPA1, the remaining shall be ignored. */
	if (pEid->Eid == IE_WPA) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The remaining shall be ignored in WPA mode\n",
				 __func__));
		goto out;
	}

	/* skip the AKM capability */
	pBuf += offset;
	len -= offset;

	/* Parse the RSN Capabilities */
	if (len < sizeof(RSN_CAPABILITIES)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The peer RSNIE doesn't include RSN-Cap\n", __func__));
		goto out;
	} else {
		/* Report the content of the RSN capabilities */
		if (type == RSN_CAP_INFO) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : Extract RSN Capabilities\n", __func__));
			*count = 1;
			return pBuf;
		}

		/* skip RSN capability (2-bytes) */
		offset = sizeof(RSN_CAPABILITIES);
		pBuf += offset;
		len -= offset;
	}

	/* Extract PMKID-list field */
	if (len < sizeof(UINT16)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : The peer RSNIE doesn't include PMKID list Count\n", __func__));
		goto out;
	} else {
		UINT16	p_count;
		PUINT8	pPmkidList = NULL;

		NdisMoveMemory(&p_count, pBuf, sizeof(UINT16));
		p_count = cpu2le16(p_count);

		/* Get count of the PMKID list */
		if (p_count > 0) {
			PRSNIE_PMKID	pRsnPmkid;
			/* the expected length of PMKID-List field */
			offset = sizeof(RSNIE_PMKID) + (LEN_PMKID * (p_count - 1));

			/* sanity check about the length of PMKID-List field */
			if (len < offset) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : The expected lenght(%d) exceed the remaining length(%d) in PMKID-field\n",
						 __func__, offset, len));
				goto out;
			}

			/* pointer to PMKID field */
			pRsnPmkid = (PRSNIE_PMKID)pBuf;
			pPmkidList = pRsnPmkid->pmkid[0].list;
		} else {
			/* The PMKID field shall be without PMKID-List */
			offset = sizeof(UINT16);
			pPmkidList = NULL;
		}

		/* Extract PMKID list and its count */
		if (type == PMKID_LIST) {
			*count = p_count;
			return pPmkidList;
		}

		/* skip the PMKID field */
		pBuf += offset;
		len -= offset;
	}

#ifdef DOT11W_PMF_SUPPORT

	/* Get group mamagement cipher */
	if (len < LEN_OUI_SUITE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s : The peer RSNIE doesn't include Group_mgmt_cipher_suite\n", __func__));
		goto out;
	} else {
		offset = LEN_OUI_SUITE;

		/* Get Group-Mgmt-Cipher_Suite information */
		if (type == G_MGMT_SUITE) {
			*count = 1;
			return pBuf;
		} else {
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
	IN	PUINT8	rsnie,
	IN	UINT	rsnie_len)
{
	PUINT8 pSuite = NULL;
	UINT8 count;

	hex_dump("RSNIE", rsnie, rsnie_len);
	/* group cipher*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, GROUP_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("group cipher", pSuite, 4 * count);

	/* pairwise cipher*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PAIRWISE_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("pairwise cipher", pSuite, 4 * count);

	/* AKM*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, AKM_SUITE, &count);

	if (pSuite != NULL)
		hex_dump("AKM suite", pSuite, 4 * count);

	/* PMKID*/
	pSuite = WPA_ExtractSuiteFromRSNIE(rsnie, rsnie_len, PMKID_LIST, &count);

	if (pSuite != NULL)
		hex_dump("PMKID", pSuite, LEN_PMKID);
}


#ifdef RT_CFG80211_SUPPORT
BOOLEAN RTMPIsValidIEs(UCHAR *Ies, INT32 Len)
{
	/* Validate if the IE is in correct format. */
	INT32 Pos = 0;
	INT32 IeLen = 0;

	while (Pos < Len) {
		IeLen = (INT32)(Ies[Pos + 1]) + 2;

		if (IeLen < 0)
			return FALSE;

		Pos += IeLen;
	}

	if (Pos == Len)
		return TRUE;
	else
		return FALSE;
}

#endif /* RT_CFG80211_SUPPORT */


/* --------------------Eddy---------------- */
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
VOID PRF(
	IN UCHAR *key,
	IN INT key_len,
	IN UCHAR *prefix,
	IN INT prefix_len,
	IN UCHAR *data,
	IN INT data_len,
	OUT UCHAR *output,
	IN INT len)
{
	INT i;
	UCHAR *input;
	INT currentindex = 0;
	INT total_len;
	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!PRF: no memory!!!\n"));
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
	for	(i = 0;	i <	(len + 19) / 20; i++) {
		RT_HMAC_SHA1(key, key_len, input, total_len, &output[currentindex], SHA1_DIGEST_SIZE);
		currentindex +=	20;
		/* update the last octet */
		input[total_len - 1]++;
	}

	os_free_mem(input);
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
VOID KDF(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	USHORT	i;
	UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;
	UINT	len_in_bits = (len << 3);

	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!KDF: no memory!!!\n"));
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

	for	(i = 1;	i <= ((len_in_bits + 255) / 256); i++) {
		/* HMAC-SHA256 derives output */
		RT_HMAC_SHA256((UCHAR *)key, key_len, input, total_len, (UCHAR *)&output[currentindex], 32);
		currentindex +=	32; /* next concatenation location*/
		input[0]++;			/* increment octet count*/
	}

	os_free_mem(input);
}

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

VOID KDF_384(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len)
{
	USHORT	i;
	UCHAR   *input;
	INT		currentindex = 0;
	INT		total_len;
	UINT	len_in_bits = (len << 3);

	os_alloc_mem(NULL, (PUCHAR *)&input, 1024);

	if (input == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!KDF: no memory!!!\n"));
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

	for	(i = 1;	i <= ((len_in_bits + 383) / 384); i++) {
		/* HMAC-SHA384 derives output */
		RT_HMAC_SHA384((UCHAR *)key,
				key_len,
				input,
				total_len,
				(UCHAR *)&output[currentindex],
				SHA384_DIGEST_SIZE);
		currentindex +=	SHA384_DIGEST_SIZE; /* next concatenation location*/
		input[0]++;			/* increment octet count*/
	}

	os_free_mem(input);
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

	for	(i = 0;	i <	32;	i++) {
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
		PRF(KeyCounter, 32, prefix, 12, local, curr, result, 32);
	}

	NdisMoveMemory(random, result,	32);
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
	/* UINT8 debug_level_bkp = DebugLevel; */

	if ((!pEntry) || (!IS_ENTRY_CLIENT(pEntry) && !IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
					  && (!IS_ENTRY_REPEATER(pEntry))
#endif
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
					  && (!IS_ENTRY_TDLS(pEntry))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
					 ))
		return;

	Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);

	if (Status != NDIS_STATUS_SUCCESS)
		return;

	RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, (bClearFrame ? 1 : 0));
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

	if (IS_ENTRY_APCLI(pEntry)) {
	} else
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{
	}

	RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->wcid);

	/* TODO: shiang-usw, fix this! */
	if (!pEntry->wdev) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}
	RTMP_SET_PACKET_WDEV(pPacket, pEntry->wdev->wdev_idx);
	RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);


	wdev = pEntry->wdev;

	if (!wdev) {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}

	send_data_pkt(pAd, wdev, pPacket);
}


/* Eddy: should be move to auth file */
VOID MlmeDeAuthAction(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN USHORT           Reason,
	IN BOOLEAN          bDataFrameFirst)
{
	PUCHAR          pOutBuffer = NULL;
	ULONG           FrameLen = 0;
	HEADER_802_11   DeAuthHdr;
	NDIS_STATUS     NStatus;

	if (pEntry) {
		/* Send out a Deauthentication request frame*/
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		/* send wireless event - for send disassication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Send DEAUTH frame with ReasonCode(%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
				  Reason, PRINT_MAC(pEntry->Addr)));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
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

		if (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE
			&& pAd->MacTab.tr_entry[pEntry->wcid].PortSecured == WPA_802_1X_PORT_SECURED) {
			ULONG	TmpLen;
			UINT	res_len = LEN_CCMP_HDR + LEN_CCMP_MIC;
			UCHAR	res_buf[res_len];
			/* reserve a buffer for PMF CCMP calculation later */
			MakeOutgoingFrame(pOutBuffer + FrameLen,	&TmpLen,
							  res_len,				res_buf,
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

		MlmeFreeMemory(pOutBuffer);
		/*Sta entry is deleted before sending EAP-Failure & Deauth frame
		  Using OS_WAIT to rescheduling the frame TX before entry delete*/
		OS_WAIT(1);
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH_COM, Reason);
#endif
		/* ApLogEvent(pAd, pEntry->Addr, EVENT_DISASSOCIATED);*/
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
		if (!pEntry->IsKeep)
#endif /* CONFIG_HOTSPOT_R2 */
			/*usd dispatch level due to MlmeDeAuthAction will be called in timer task*/
			mac_entry_delete(pAd, pEntry);
	}
}


/*
    ==========================================================================
    Description:
		Check whether the received frame is EAP frame.

	Arguments:
		pAd				-	pointer to our pAdapter context
		pEntry			-	pointer to active entry
		pData			-	the received frame
		DataByteCount	-	the received frame's length
		wdev_idx	-	indicate the interface index

    Return:
	 TRUE			-	This frame is EAP frame
	 FALSE			-	otherwise
    ==========================================================================
*/
BOOLEAN RTMPCheckWPAframe(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN UCHAR *pData,
	IN ULONG DataByteCount,
	IN UCHAR wdev_idx,
	IN BOOLEAN eth_frame)
{
	ULONG Body_len, min_len = (LENGTH_802_1_H + LENGTH_EAPOL_H);
	BOOLEAN Cancelled;
	struct wifi_dev *wdev;
	ASSERT(wdev_idx <= WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX)
		return FALSE;

	wdev = pAd->wdev_list[wdev_idx];

	do {
		if (eth_frame) {
			min_len = (LENGTH_802_3 + LENGTH_EAPOL_H);
			break;
		}

	} while (FALSE);

	if (DataByteCount < min_len)
		return FALSE;

	/* Skip LLC or ETH header */
	if (eth_frame == TRUE)
		pData += LENGTH_802_3;
	else {
		/* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
		if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
			pData += 6;
	}

	/* Skip 2-bytes EAPoL type */
	if (NdisEqualMemory(EAPOL, pData, 2))
		pData += 2;
	else
		return FALSE;

	switch (*(pData + 1)) {
	case EAPPacket:
		Body_len = (*(pData + 2) << 8) | (*(pData + 3));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef IDS_SUPPORT

			if ((*(pData + 4)) == EAP_CODE_REQUEST)
				pAd->ApCfg.RcvdEapReqCount++;

#endif /* IDS_SUPPORT */
		}
#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive EAP-Packet frame, TYPE = 0, Length = %ld\n", Body_len));
		break;

	case EAPOLStart:
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive EAPOL-Start frame, TYPE = 1\n"));

		if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Cancel the EnqueueEapolStartTimerRunning\n"));
			RTMPCancelTimer(&pEntry->SecConfig.StartFor4WayTimer, &Cancelled);
			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		}

		break;

	case EAPOLLogoff:
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive EAPOLLogoff frame, TYPE = 2\n"));
		break;

	case EAPOLKey:
		Body_len = (*(pData + 2) << 8) | (*(pData + 3));
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive EAPOL-Key frame, TYPE = 3, Length = %ld\n", Body_len));
		break;

	case EAPOLASFAlert:
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive EAPOLASFAlert frame, TYPE = 4\n"));
		break;

	default:
		return FALSE;
	}

	return TRUE;
}


/*
* F(P, S, c, i) = U1 xor U2 xor ... Uc
* U1 = PRF(P, S || Int(i))
* U2 = PRF(P, U1)
* Uc = PRF(P, Uc-1)
*/
static void WPA_F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output)
{
	unsigned char digest[36], digest1[SHA1_DIGEST_SIZE];
	int i, j, len;

	len = strlen(password);
	/* U1 = PRF(P, S || int(i)) */
	memcpy(digest, ssid, ssidlength);
	digest[ssidlength] = (unsigned char)((count >> 24) & 0xff);
	digest[ssidlength + 1] = (unsigned char)((count >> 16) & 0xff);
	digest[ssidlength + 2] = (unsigned char)((count >> 8) & 0xff);
	digest[ssidlength + 3] = (unsigned char)(count & 0xff);
	RT_HMAC_SHA1((unsigned char *) password, len, digest, ssidlength + 4, digest1, SHA1_DIGEST_SIZE); /* for WPA update*/
	/* output = U1 */
	NdisCopyMemory(output, digest1, SHA1_DIGEST_SIZE);

	for (i = 1; i < iterations; i++) {
		/* Un = PRF(P, Un-1) */
		RT_HMAC_SHA1((unsigned char *) password, len, digest1, SHA1_DIGEST_SIZE, digest, SHA1_DIGEST_SIZE); /* for WPA update*/
		NdisCopyMemory(digest1, digest, SHA1_DIGEST_SIZE);

		/* output = output xor Un */
		for (j = 0; j < SHA1_DIGEST_SIZE; j++)
			output[j] ^= digest[j];
	}
}


/*
* password - ascii string up to 63 characters in length
* ssid - octet string up to 32 octets
* ssidlength - length of ssid in octets
* output must be 40 octets in length and outputs 256 bits of key
*/
INT WPAPasswordHash(
	IN CHAR *password,
	IN UCHAR *ssid,
	IN INT ssidlength,
	OUT UCHAR *output)
{
	if ((strlen(password) > 63) || (ssidlength > 32))
		return 0;

	WPA_F(password, ssid, ssidlength, 4096, 1, output);
	WPA_F(password, ssid, ssidlength, 4096, 2, &output[SHA1_DIGEST_SIZE]);
	return 1;
}

INT SetWPAPSKKey(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *keyString,
	IN INT keyStringLen,
	IN UCHAR *pHashStr,
	IN INT hashStrLen,
	OUT PUCHAR pPMKBuf)
{
	UCHAR keyMaterial[40];

	if ((keyStringLen < 8) || (keyStringLen > 64)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n",
				 keyStringLen, keyString));
		return FALSE;
	}

	NdisZeroMemory(pPMKBuf, 32);

	if (keyStringLen == 64)
		AtoH(keyString, pPMKBuf, 32);
	else {
		WPAPasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
		NdisMoveMemory(pPMKBuf, keyMaterial, 32);
	}

	return TRUE;
}


/* For TKIP frame, calculate the MIC value */
BOOLEAN rtmp_chk_tkip_mic(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN RX_BLK * pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	UCHAR UserPriority = pRxBlk->UserPriority;
	UCHAR *pDA, *pSA;
	CHAR *pKey = NULL;

	if ((FC->FrDs == 1) && (FC->ToDs == 1)) {
		pDA = pRxBlk->Addr3;
		pSA = pRxBlk->Addr4;
	} else if ((FC->FrDs == 1) && (FC->ToDs == 0)) {
		pDA = pRxBlk->Addr1;
		pSA = pRxBlk->Addr3;
	} else if ((FC->FrDs == 0) && (FC->ToDs == 1)) {
		pDA = pRxBlk->Addr3;
		pSA = pRxBlk->Addr2;
	} else {
		/* FrDS = 0; ToDS = 0 => IBSS, Non-AP to Non-AP in BSS */
		pDA = pRxBlk->Addr1;
		pSA = pRxBlk->Addr2;
	}

#ifdef HDR_TRANS_RX_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
		pData = (pData + LENGTH_802_3);
		DataSize = (DataSize - LENGTH_802_3);
	}

#endif /* HDR_TRANS_RX_SUPPORT */

	pKey = &pEntry->SecConfig.PTK[OFFSET_OF_TKIP_RX_MIC];

	if (RTMPTkipCompareMICValue(pAd,
								pData,
								pDA,
								pSA,
								pKey,
								UserPriority,
								DataSize) == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Rx MIC Value error 2\n"));
#ifdef CONFIG_AP_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
			RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);

#endif /* CONFIG_AP_SUPPORT */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return FALSE;
	}

	return TRUE;
}


VOID ReadWPAParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
	RTMP_STRING tok_str[16];
#endif /* CONFIG_AP_SUPPORT */
	RTMP_STRING *macptr;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/*
			In WPA-WPA2 mix mode, it provides a more flexible cipher combination.
			- WPA-AES and WPA2-TKIP
			- WPA-AES and WPA2-TKIPAES
			- WPA-TKIP and WPA2-AES
			- WPA-TKIP and WPA2-TKIPAES
			- WPA-TKIPAES and WPA2-AES
			- WPA-TKIPAES and WPA2-TKIP
			- WPA-TKIPAES and WPA2-TKIPAES (default)
		*/
		if (RTMPGetKeyParameter("WpaMixPairCipher", tmpbuf, 128, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				SetWdevAuthMode(pSecConfig, macptr);
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}

		/* GroupKey Rekey Method*/
		if (RTMPGetKeyParameter("RekeyMethod", tmpbuf, 128, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_DISABLE;

				if (rtstrcasecmp(macptr, "TIME") == TRUE)
					pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_TIME;
				else if (rtstrcasecmp(macptr, "PKT") == TRUE)
					pSecConfig->GroupReKeyMethod = SEC_GROUP_REKEY_PACKET;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) GroupKey ReKeyMethod=%x\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod));
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->GroupReKeyMethod = pAd->ApCfg.MBSSID[0].wdev.SecConfig.GroupReKeyMethod;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) GroupKey ReKeyMethod=%x\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyMethod));
				}
			}
		}

		/* GroupKey RekeyInterval */
		if (RTMPGetKeyParameter("RekeyInterval", tmpbuf, 255, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				ULONG value_interval;

				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				value_interval = os_str_tol(macptr, 0, 10);

				if ((value_interval >= 10) && (value_interval < MAX_GROUP_REKEY_INTERVAL))
					pSecConfig->GroupReKeyInterval = value_interval;
				else /*Default*/
					pSecConfig->GroupReKeyInterval = DEFAULT_GROUP_REKEY_INTERVAL;

				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) GroupKey ReKeyInterval=%ld seconds\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval));
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->GroupReKeyInterval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.GroupReKeyInterval;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ReKeyInterval=%ld\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->GroupReKeyInterval));
				}
			}
		}

		/*PMKCachePeriod*/
		if (RTMPGetKeyParameter("PMKCachePeriod", tmpbuf, 255, pBuffer, TRUE)) {
			struct _SECURITY_CONFIG *pSecConfig = NULL;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				ULONG value_interval;

				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				value_interval = os_str_tol(macptr, 0, 10) * 60 * OS_HZ;
				pSecConfig->PMKCachePeriod = value_interval;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) PMKCachePeriod=%ld\n",
						 INF_MBSSID_DEV_NAME, apidx, pAd->ApCfg.MBSSID[apidx].PMKCachePeriod));
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->PMKCachePeriod = pAd->ApCfg.MBSSID[0].wdev.SecConfig.PMKCachePeriod;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) PMKCachePeriod=%ld\n",
							 INF_MBSSID_DEV_NAME, apidx, pAd->ApCfg.MBSSID[apidx].PMKCachePeriod));
				}
			}
		}

		/*WPAPSK_KEY*/
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

			snprintf(tok_str, sizeof(tok_str), "WPAPSK%d", apidx + 1);

			if (RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE)) {
				ULONG len = strlen(tmpbuf);

				if (len < 65) {
					os_move_mem(pSecConfig->PSK, tmpbuf, len);
					pSecConfig->PSK[len] = '\0';
				} else
					pSecConfig->PSK[0] = '\0';
#ifdef WSC_AP_SUPPORT
					NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk, 64);
					pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen = 0;
					if ((len >= 8) && (len <= 64)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk,
									tmpbuf, len);
						pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen = len;
					}
#endif /* WSC_AP_SUPPORT */

			}
		}

		/* If not match, search WPAPSK */
		if (RTMPGetKeyParameter("WPAPSK", tmpbuf, 512, pBuffer, FALSE)) {
			ULONG len;

			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;

				len = strlen(macptr);

				if (len < 65) {
					os_move_mem(pSecConfig->PSK, macptr, len);
					pSecConfig->PSK[len] = '\0';
				} else
					pSecConfig->PSK[0] = '\0';
#ifdef WSC_AP_SUPPORT
					NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk, 64);
					pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen = 0;
					if ((len >= 8) && (len <= 64)) {
						NdisMoveMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPsk,
									tmpbuf, len);
						pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WpaPskLen = len;
					}
#endif /* WSC_AP_SUPPORT */

			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}

static BOOLEAN WPAMakeRsnIeCipher(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	PUCHAR pRsnIe = &pSecConfig->RSNE_Content[ie_idx][0];
	UCHAR	PairwiseCnt = 0;
	*rsn_len = 0;

	if (ElememtId == SEC_RSNIE_WPA1_IE) {
		RSNIE *pRsnie_cipher = (RSNIE *) pRsnIe;
		/* Assign OUI and version*/
		NdisMoveMemory(pRsnie_cipher->oui, OUI_WPA_VERSION, 4);
		pRsnie_cipher->version = 1;

		/* Group cipher */
		if (IS_CIPHER_WEP40(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP40, 4);
		else if (IS_CIPHER_WEP104(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_WEP104, 4);
		else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_TKIP, 4);
		else if (IS_CIPHER_CCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA_CCMP, 4);
		else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==> %s not support group cipher on WPA1 (GroupCipher=0x%x)\n", __func__, pSecConfig->GroupCipher));
			return FALSE;
		}

		/* Pairwise cipher */
		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA_TKIP, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA_CCMP, 4);
			PairwiseCnt++;
		}

		pRsnie_cipher->ucount = PairwiseCnt;
		*rsn_len = sizeof(RSNIE) + (4 * (PairwiseCnt - 1));
		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
		pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	} else if (ElememtId == SEC_RSNIE_WPA2_IE) {
		RSNIE2 *pRsnie_cipher = (RSNIE2 *) pRsnIe;
		/* Assign the verson as 1*/
		pRsnie_cipher->version = 1;

		/* Group cipher */
		if (IS_CIPHER_WEP40(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_WEP40, 4);
		else if (IS_CIPHER_WEP104(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_WEP104, 4);
		else if (IS_CIPHER_TKIP(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_TKIP, 4);
		else if (IS_CIPHER_CCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP128, 4);
		else if (IS_CIPHER_CCMP256(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_CCMP256, 4);
		else if (IS_CIPHER_GCMP128(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_GCMP128, 4);
		else if (IS_CIPHER_GCMP256(pSecConfig->GroupCipher))
			NdisMoveMemory(pRsnie_cipher->mcast, OUI_WPA2_CIPHER_GCMP256, 4);
		else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==> %s not support group cipher on WPA2 (GroupCipher=0x%x)\n", __func__, pSecConfig->GroupCipher));
			return FALSE;
		}

		/* Pairwise cipher */
		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("==> %s not support group cipher on SAE\n", __func__));
				return FALSE;
			}
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_TKIP, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_CCMP128, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_CCMP256, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_GCMP128, 4);
			PairwiseCnt++;
		}

		if (IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
			NdisMoveMemory(pRsnie_cipher->ucast[0].oui + PairwiseCnt * 4, OUI_WPA2_CIPHER_GCMP256, 4);
			PairwiseCnt++;
		}

		pRsnie_cipher->ucount = PairwiseCnt;
		*rsn_len = sizeof(RSNIE2) + (4 * (PairwiseCnt - 1));
		/* swap for big-endian platform*/
		pRsnie_cipher->version = cpu2le16(pRsnie_cipher->version);
		pRsnie_cipher->ucount = cpu2le16(pRsnie_cipher->ucount);
	}

	return TRUE;
}


static BOOLEAN WPAMakeRsnIeAKM(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UINT32 wdev_type,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	PRSNIE_AUTH pRsnie_auth = (RSNIE_AUTH *) (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));
	UCHAR	AkmCnt = 0;

	if (ElememtId == SEC_RSNIE_WPA1_IE) {
		if (IS_AKM_WPA1(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_8021X_AKM, 4);
			AkmCnt++;
		} else if (IS_AKM_WPA1PSK(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_PSK_AKM, 4);
			AkmCnt++;
		} else if (IS_AKM_WPANONE(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui, OUI_WPA_NONE_AKM, 4);
			AkmCnt++;
		} else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==> %s not support AKM on WPA1\n", __func__));
			return FALSE;
		}
	} else if (ElememtId == SEC_RSNIE_WPA2_IE) {
		if (IS_AKM_WPA2(pSecConfig->AKMMap)) {
#ifdef DOT11W_PMF_SUPPORT

			if (pSecConfig->PmfCfg.UsePMFConnect
				&& (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)
				&& ((wdev_type == WDEV_TYPE_STA) || (wdev_type == WDEV_TYPE_APCLI))) {
				NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_8021X_SHA256, 4);
				AkmCnt++;
			} else if (pSecConfig->PmfCfg.PMFSHA256 && (wdev_type == WDEV_TYPE_AP)) {
				CHAR offset = 0;

				if (pSecConfig->PmfCfg.MFPR == FALSE) {
					NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_8021X, 4);
					AkmCnt++;
					offset += 4;
				}

				NdisMoveMemory(pRsnie_auth->auth[0].oui + offset, OUI_WPA2_AKM_8021X_SHA256, 4);
				AkmCnt++;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2(pSecConfig->AKMMap)) {
					offset += 4;
					NdisMoveMemory(pRsnie_auth->auth[0].oui + offset,
								   OUI_WPA2_AKM_FT_8021X, 4);
					AkmCnt++;
				}

#endif /* DOT11R_FT_SUPPORT */
			} else
#endif /* DOT11W_PMF_SUPPORT */
			{
				NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_8021X, 4);
				AkmCnt++;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2(pSecConfig->AKMMap)) {
					CHAR offset = 4;

					NdisMoveMemory(pRsnie_auth->auth[0].oui + offset,
								   OUI_WPA2_AKM_FT_8021X, 4);
					AkmCnt++;
				}

#endif /* DOT11R_FT_SUPPORT */
			}
		}

		if (IS_AKM_WPA2PSK(pSecConfig->AKMMap)) {
#ifdef DOT11W_PMF_SUPPORT

			if (pSecConfig->PmfCfg.UsePMFConnect
				&& (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)
				&& ((wdev_type == WDEV_TYPE_STA) || (wdev_type == WDEV_TYPE_APCLI))) {
				NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_PSK_SHA256, 4);
				AkmCnt++;
			} else if (pSecConfig->PmfCfg.PMFSHA256 && (wdev_type == WDEV_TYPE_AP)) {
				CHAR offset = 0;

				if (pSecConfig->PmfCfg.MFPR == FALSE) {
					NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_PSK, 4);
					AkmCnt++;
					offset += 4;
				}

				NdisMoveMemory(pRsnie_auth->auth[0].oui + offset, OUI_WPA2_AKM_PSK_SHA256, 4);
				AkmCnt++;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap)) {
					offset += 4;
					NdisMoveMemory(pRsnie_auth->auth[0].oui + offset,
								   OUI_WPA2_AKM_FT_PSK, 4);
					AkmCnt++;
				}

#endif /* DOT11R_FT_SUPPORT */
			} else
#endif /* DOT11W_PMF_SUPPORT */
			{
				NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_PSK, 4);
				AkmCnt++;
#ifdef DOT11R_FT_SUPPORT

				if (IS_AKM_FT_WPA2PSK(pSecConfig->AKMMap)) {
					CHAR offset = 4;

					NdisMoveMemory(pRsnie_auth->auth[0].oui + offset,
								   OUI_WPA2_AKM_FT_PSK, 4);
					AkmCnt++;
				}

#endif /* DOT11R_FT_SUPPORT */
			}
		}

		if (IS_AKM_WPA2_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_8021X_SHA256, 4);
			AkmCnt++;
		}

		if (IS_AKM_WPA2PSK_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_PSK_SHA256, 4);
			AkmCnt++;
		}

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

		if (IS_AKM_TDLS(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_TDLS, 4);
			AkmCnt++;
		}

#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#ifdef DOT11_SAE_SUPPORT

		if (IS_AKM_SAE_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_SAE_SHA256, 4);
			AkmCnt++;
		}

		if (IS_AKM_FT_SAE_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_FT_SAE_SHA256, 4);
			AkmCnt++;
		}

#endif /* DOT11_SAE_SUPPORT */
#if defined(DOT11_SUITEB_SUPPORT) || defined(HOSTAPD_SUITEB_SUPPORT)

		if (IS_AKM_SUITEB_SHA256(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_SUITEB_SHA256, 4);
			AkmCnt++;
		}

		if (IS_AKM_SUITEB_SHA384(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_SUITEB_SHA384, 4);
			AkmCnt++;
		}

#endif /* DOT11_SUITEB_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

		if (IS_AKM_FT_WPA2_SHA384(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_FT_8021X_SHA384, 4);
			AkmCnt++;
		}

#endif /* DOT11R_FT_SUPPORT */
		if (IS_AKM_OWE(pSecConfig->AKMMap)) {
			NdisMoveMemory(pRsnie_auth->auth[0].oui + 4 * AkmCnt, OUI_WPA2_AKM_OWE, 4);
			AkmCnt++;
		}

	}

	pRsnie_auth->acount = AkmCnt;
	pRsnie_auth->acount = cpu2le16(pRsnie_auth->acount);
	/* update current RSNIE length*/
	(*rsn_len) += (sizeof(RSNIE_AUTH) + (4 * (AkmCnt - 1)));
	return TRUE;
}


static BOOLEAN WPAMakeRsnIeCap(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{
	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
#ifndef DISABLE_HOSTAPD_BEACON
	RSN_CAPABILITIES *pRSN_Cap = (RSN_CAPABILITIES *) (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));
#endif

	if (ElememtId == SEC_RSNIE_WPA2_IE) {
#ifdef DISABLE_HOSTAPD_BEACON
	memcpy((&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len)), pSecConfig->RsnCap, 2);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("[RSN IE CAP] %s RSN CAP %02x %02x \n", __func__,
		pSecConfig->RsnCap[0], pSecConfig->RsnCap[1]));
#else

#ifdef DOT1X_SUPPORT
		pRSN_Cap->field.PreAuth = (pSecConfig->PreAuth == TRUE) ? 1 : 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
		pRSN_Cap->field.MFPC = (pSecConfig->PmfCfg.MFPC) ? 1 : 0;
		pRSN_Cap->field.MFPR = (pSecConfig->PmfCfg.MFPR) ? 1 : 0;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_TRACE, ("[PMF]%s: RSNIE Capability MFPC=%d, MFPR=%d\n", __func__, pRSN_Cap->field.MFPC, pRSN_Cap->field.MFPR));
#endif /* DOT11W_PMF_SUPPORT */
		pRSN_Cap->word = cpu2le16(pRSN_Cap->word);
#endif /*DISABLE_HOSTAPD_BEACON*/
		(*rsn_len) += sizeof(RSN_CAPABILITIES); /* update current RSNIE length*/
	}

	return TRUE;
}


static BOOLEAN WPAInsertRsnIePMKID(
	IN PMAC_TABLE_ENTRY pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN UCHAR ie_idx,
	OUT UCHAR *rsn_len)
{

	SEC_RSNIE_TYPE ElememtId = pSecConfig->RSNE_Type[ie_idx];
	UINT8 *pBuf = (&pSecConfig->RSNE_Content[ie_idx][0] + (*rsn_len));

	if (ElememtId == SEC_RSNIE_WPA2_IE) {
#ifdef DOT11R_FT_SUPPORT
		if (pEntry && IS_ENTRY_CLIENT(pEntry) &&  pEntry->FT_Status != 0) {
			PUINT8	pmkid_ptr = NULL;
			UINT8	pmkid_len = 0;
			UINT8	extra_len = 0;
			UINT16	pmk_count = 0;

			if (pEntry->FT_Status == TX_AUTH_RSP)
				pmkid_ptr = pEntry->FT_PMK_R0_NAME;
			else
				pmkid_ptr = pEntry->FT_PMK_R1_NAME;

			pmkid_len = LEN_PMK_NAME;

			if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0)) {
				extra_len = sizeof(UINT16) + pmkid_len;

				pmk_count = (pmkid_len >> 4);
				pmk_count = cpu2le16(pmk_count);
			} else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : no FT PMKID-List included(%d).\n", __func__, pmkid_len));
			}

		    /* Insert PMKID-List field */
			if (extra_len > 0) {
				hex_dump("FT_PMKID", pmkid_ptr, pmkid_len);

				NdisMoveMemory(pBuf, &pmk_count, 2);
				NdisMoveMemory((pBuf+2), pmkid_ptr, pmkid_len);
				(*rsn_len) += extra_len;
			}
		} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
		if (IS_AKM_SAE(pSecConfig->AKMMap)
			&& pEntry && (IS_ENTRY_CLIENT(pEntry)
#ifdef APCLI_SAE_SUPPORT
						|| IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)
#endif
						) && is_pmkid_cache_in_sec_config(pSecConfig)) {

			UINT16	pmk_count = 1;

			pmk_count = cpu2le16(pmk_count);
			NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
			NdisMoveMemory(pBuf+sizeof(pmk_count), pSecConfig->pmkid, LEN_PMKID);
			(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: (SAE) including the PMKID.\n", __func__));
		} else
#endif
		if (IS_AKM_OWE(pSecConfig->AKMMap)
			&& pEntry && (IS_ENTRY_CLIENT(pEntry)
#ifdef APCLI_OWE_SUPPORT
						|| IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)
#endif
						) && is_pmkid_cache_in_sec_config(pSecConfig)) {
			UINT16	pmk_count = 1;

			pmk_count = cpu2le16(pmk_count);
			NdisMoveMemory(pBuf, &pmk_count, sizeof(pmk_count));
			NdisMoveMemory(pBuf+sizeof(pmk_count), pSecConfig->pmkid, LEN_PMKID);
			(*rsn_len) += sizeof(pmk_count) + LEN_PMKID;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: (OWE) including the PMKID.\n", __func__));
		}
#ifdef DOT11W_PMF_SUPPORT
		else if (pSecConfig->PmfCfg.MFPC) {
			UCHAR ZeroPmkID[2] = {0x00, 0x00};

			NdisMoveMemory(pBuf, ZeroPmkID, 2);
			(*rsn_len) += 2;
		} else
#endif /* DOT11W_PMF_SUPPORT */
		if (0)
			; /* for build pass */
	}

	return TRUE;
}


VOID WPAMakeRSNIE (
	IN UINT32 wdev_type,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN PMAC_TABLE_ENTRY pEntry)
{
	UCHAR ie_idx = 0;

	/* Initiate some related information */
	for (ie_idx = 0; ie_idx < SEC_RSNIE_NUM; ie_idx++) {
		pSecConfig->RSNE_Type[ie_idx] = SEC_RSNIE_NONE;
		pSecConfig->RSNE_Len[ie_idx] = 0;
		NdisZeroMemory(pSecConfig->RSNE_Content[ie_idx], MAX_LEN_OF_RSNIE);
	}

	/* Check AKM support per wdev type  */
	switch (wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
		if ((pSecConfig->AKMMap & AKM_AP_MASK) == 0)
			return;

		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> %s(AP)\n", __func__));
		break;
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT

	case WDEV_TYPE_APCLI:
		/* Only support WPAPSK or WPA2PSK for AP-Client mode */
		{
			if ((pSecConfig->AKMMap & AKM_APCLI_MASK) == 0)
				return;

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==> %s(ApCli)\n", __func__));
		}

		break;
#endif /* APCLI_SUPPORT */
	}

	if (IS_AKM_WPA1(pSecConfig->AKMMap)
		|| IS_AKM_WPA1PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPANONE(pSecConfig->AKMMap)) {
		pSecConfig->RSNE_Type[0] = SEC_RSNIE_WPA1_IE;
		pSecConfig->RSNE_EID[0][0] = IE_WPA;
	}


	if (IS_AKM_WPA2(pSecConfig->AKMMap) ||
	    IS_AKM_WPA2PSK(pSecConfig->AKMMap) ||
	    IS_AKM_WPA3PSK(pSecConfig->AKMMap) ||
	    IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) ||
	    IS_AKM_OWE(pSecConfig->AKMMap)) {
		pSecConfig->RSNE_Type[1] = SEC_RSNIE_WPA2_IE;
		pSecConfig->RSNE_EID[1][0] = IE_WPA2;
	}

	for (ie_idx = 0; ie_idx < SEC_RSNIE_NUM; ie_idx++) {
		UCHAR p_offset = 0;

		if (pSecConfig->RSNE_Type[ie_idx] == SEC_RSNIE_NONE)
			continue;

		else {
			/* Build the primary RSNIE*/
			/* 1. insert cipher suite*/
			if (WPAMakeRsnIeCipher(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 2. insert AKM*/
			if (WPAMakeRsnIeAKM(pSecConfig, wdev_type, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 3. insert capability*/
			if (WPAMakeRsnIeCap(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

			/* 4. Insert PMKID */
			if (WPAInsertRsnIePMKID(pEntry, pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

#ifdef DOT11W_PMF_SUPPORT

			/* 5. Insert Group Management Cipher*/
			if (PMF_MakeRsnIeGMgmtCipher(pSecConfig, ie_idx, &p_offset) == FALSE)
				continue;

			ASSERT(p_offset < 255);

			if (p_offset >= 255)
				continue;

#endif /* DOT11W_PMF_SUPPORT */
			pSecConfig->RSNE_Len[ie_idx] = p_offset;
			hex_dump("The RSNE", &pSecConfig->RSNE_Content[ie_idx][0], pSecConfig->RSNE_Len[ie_idx]);
		}
	}
}

BOOLEAN WPACheckGroupCipher(
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	CHAR *pCipher = NULL;

	if (eid_ptr->Len >= 6) {
		/* WPA and WPA2 format not the same in RSN_IE */
		if (eid_ptr->Eid == IE_WPA) {
			if (IS_CIPHER_TKIP(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA_TKIP;
				SET_CIPHER_TKIP(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA_CCMP;
				SET_CIPHER_CCMP128(pSecConfigEntry->GroupCipher);
			}

			if (pCipher && NdisEqualMemory(&eid_ptr->Octet[6], pCipher, 4))
				return TRUE;
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (IS_CIPHER_WEP40(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_WEP40;
				SET_CIPHER_WEP40(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_WEP104(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_WEP104;
				SET_CIPHER_WEP104(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_TKIP(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_TKIP;
				SET_CIPHER_TKIP(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_CCMP128;
				SET_CIPHER_CCMP128(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_CCMP256(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_CCMP256;
				SET_CIPHER_CCMP256(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_GCMP128(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_GCMP128;
				SET_CIPHER_GCMP128(pSecConfigEntry->GroupCipher);
			} else if (IS_CIPHER_GCMP256(pSecConfigSelf->GroupCipher)) {
				pCipher = OUI_WPA2_CIPHER_GCMP256;
				SET_CIPHER_GCMP256(pSecConfigEntry->GroupCipher);
			}

			if (pCipher && NdisEqualMemory(&eid_ptr->Octet[2], pCipher, 4))
				return TRUE;
		}
	}

	CLEAR_GROUP_CIPHER(pSecConfigEntry);
	return FALSE;
}


BOOLEAN WPACheckUcast(
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	PUCHAR pStaTmp;
	USHORT Count, i;

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	if (eid_ptr->Eid == IE_WPA2) {
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
		pStaTmp += 6;
	} else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
		pStaTmp += 10;
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : invalid IE=%d\n", __func__, eid_ptr->Eid));
		return FALSE;
	}

	/* Store unicast cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/* pointer to unicast cipher */
	pStaTmp += sizeof(USHORT);

	for (i = 0; i < Count; i++) {
		if (eid_ptr->Eid == IE_WPA) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA_TKIP, 4))
				SET_CIPHER_TKIP(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA_CCMP, 4))
				SET_CIPHER_CCMP128(pSecConfigEntry->PairwiseCipher);
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_TKIP, 4))
				SET_CIPHER_TKIP(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_CCMP128, 4))
				SET_CIPHER_CCMP128(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_CCMP256, 4))
				SET_CIPHER_CCMP256(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_GCMP128, 4))
				SET_CIPHER_GCMP128(pSecConfigEntry->PairwiseCipher);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_CIPHER_GCMP256, 4))
				SET_CIPHER_GCMP256(pSecConfigEntry->PairwiseCipher);
		}

		pStaTmp += 4;
		Count--;
	}

	if ((pSecConfigSelf->PairwiseCipher & pSecConfigEntry->PairwiseCipher) > 0)
		return TRUE;

	CLEAR_PAIRWISE_CIPHER(pSecConfigEntry);
	return FALSE;
}


BOOLEAN WPACheckAKM(
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PEID_STRUCT eid_ptr)
{
	PUCHAR pStaTmp;
	USHORT Count, i;

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	if (eid_ptr->Eid == IE_WPA2) {
		/* skip Version(2),Multicast cipter(4) 2+4==6 */
		/* point to number of unicast */
		pStaTmp += 6;
	} else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4),Vesrion(2),Multicast cipher(4) 4+2+4==10 */
		/* point to number of unicast */
		pStaTmp += 10;
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s ==> Unknown WPAIE, WPAIE=%d\n", __func__, eid_ptr->Eid));
		return FALSE;
	}

	/* Store unicast cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/* pointer to unicast cipher */
	pStaTmp += sizeof(USHORT);
	/* Skip all unicast cipher suite */
	pStaTmp += 4 * Count;
	/* Store AKM count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);
	/*pointer to AKM cipher */
	pStaTmp += sizeof(USHORT);

	/* Ellis todo: if akm count > 1, key_deri_alg will be overwriten, pmf akm workaround need to be fixed first */
	for (i = 0; i < Count; i++) {
		if (eid_ptr->Eid == IE_WPA) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA_8021X_AKM, 4))
				SET_AKM_WPA1(pSecConfigEntry->AKMMap);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA_PSK_AKM, 4))
				SET_AKM_WPA1PSK(pSecConfigEntry->AKMMap);
		} else if (eid_ptr->Eid == IE_WPA2) {
			if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_8021X, 4)) {
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA1;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_PSK, 4)) {
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA1;
			}

#ifdef DOT11R_FT_SUPPORT
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_8021X, 4)) {
				SET_AKM_FT_WPA2(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_PSK, 4)) {
				SET_AKM_FT_WPA2PSK(pSecConfigEntry->AKMMap);
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
			}

#endif /* DOT11R_FT_SUPPORT */
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_8021X_SHA256, 4)) {
#ifdef DOT11W_PMF_SUPPORT
				SET_AKM_WPA2(pSecConfigEntry->AKMMap);
#else
				SET_AKM_WPA2_SHA256(pSecConfigEntry->AKMMap);
#endif /* DOT11W_PMF_SUPPORT */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_PSK_SHA256, 4)) {
#ifdef DOT11W_PMF_SUPPORT
				SET_AKM_WPA2PSK(pSecConfigEntry->AKMMap);
#else
				SET_AKM_WPA2PSK_SHA256(pSecConfigEntry->AKMMap);
#endif /* DOT11W_PMF_SUPPORT */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_TDLS, 4))
				SET_AKM_TDLS(pSecConfigEntry->AKMMap);
			else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SAE_SHA256, 4)) {
				SET_AKM_SAE_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_SAE_SHA256, 4)) {
				SET_AKM_FT_SAE_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SUITEB_SHA256, 4)) {
				SET_AKM_SUITEB_SHA256(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_SUITEB_SHA384, 4)) {
				SET_AKM_SUITEB_SHA384(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_FT_8021X_SHA384, 4)) {
				SET_AKM_FT_WPA2_SHA384(pSecConfigEntry->AKMMap);
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA384;
			} else if (NdisEqualMemory(pStaTmp, &OUI_WPA2_AKM_OWE, 4)) {
				SET_AKM_OWE(pSecConfigEntry->AKMMap);
				/* OWE cannot derive key_deri_alg by OWE akm.
				 * it shall parse ECDH parameter to determine it.
				 * set a temporary alg here.
				 */
				pSecConfigEntry->key_deri_alg = SEC_KEY_DERI_SHA256;
			}
		}

		pStaTmp += 4;
	}

	if ((pSecConfigSelf->AKMMap & pSecConfigEntry->AKMMap) > 0)
		return TRUE;

	CLEAR_SEC_AKM(pSecConfigEntry->AKMMap);
	return FALSE;
}

static BOOLEAN wpa_check_pmkid(
	IN PUINT8 rsnie_ptr,
	IN UINT rsnie_len,
	IN UINT32 akm)
{
	UINT8 count = 0;
	PUINT8 pBuf = NULL;

	if (IS_AKM_WPA2PSK(akm)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_AKM_FT_WPA2PSK(akm))
#endif
	) {
		pBuf = WPA_ExtractSuiteFromRSNIE(rsnie_ptr, rsnie_len, PMKID_LIST, &count);

		if (count > 0)
			return FALSE;
	}

	return TRUE;
}

/* Ellis: not ready for WAPI */
BOOLEAN wpa_rsne_sanity(
	IN PUCHAR rsnie_ptr,
	IN UCHAR rsnie_len,
	OUT UCHAR *end_field)
{
	EID_STRUCT  *eid_ptr;
	PUCHAR pStaTmp;
	USHORT ver;
	USHORT Count;
	UCHAR len = 0;

	eid_ptr = (EID_STRUCT *)rsnie_ptr;

	if ((eid_ptr->Len + 2) != rsnie_len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : the len is invalid !!!\n", __func__));
		return FALSE;
	}

	if (eid_ptr->Len < MIN_LEN_OF_RSNIE) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : len is too short(len = %d) !!!\n", __func__, eid_ptr->Len));
		return FALSE;
	}

	/* Store STA RSN_IE capability */
	pStaTmp = (PUCHAR)&eid_ptr->Octet[0];

	/* check Element ID */
	if (eid_ptr->Eid == IE_WPA2)
		;
	else if (eid_ptr->Eid == IE_WPA) {
		/* skip OUI(4) */
		pStaTmp += 4;
		len += 4;
	} else if (eid_ptr->Eid == IE_WAPI)
		return TRUE;
	else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid IE=%d\n", __func__, eid_ptr->Eid));
		return FALSE;
	}

	/* check version */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : no version(len = %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	}
	NdisMoveMemory(&ver, pStaTmp, sizeof(ver));
	ver = cpu2le16(ver);

	if (ver != 1) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : unknown version(%d)\n", __func__, ver));
		return FALSE;
	}

	if (eid_ptr->Len == len) {
		/* None of the optional fields are included in the RSNE */
		*end_field = RSN_FIELD_NONE;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT);

	/* check group cipher suite */
	len += LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : group cipher is truncated(len = %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* Group Data Cipher Suite are included in the RSNE */
		*end_field = RSN_FIELD_GROUP_CIPHER;
		return TRUE;
	}

	pStaTmp += LEN_OUI_SUITE;

	/* check pairwise cipher suite */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s :  pairwise cipher suite cnt is truncated(%d)\n", __func__, eid_ptr->Len));
		return FALSE;
	}

	/* Store pairwise cipher count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * LEN_OUI_SUITE;

	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : Pairwise Cipher Suite is truncated(len %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* Pairwise Cipher Suite are included in the RSNE */
		*end_field = RSN_FIELD_PAIRWISE_CIPHER;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT) + Count * LEN_OUI_SUITE;

	/* check akm suite */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s :  akm suite cnt is truncated(%d)\n", __func__, eid_ptr->Len));
		return FALSE;
	}

	/* Store akm count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : Akm Suite is truncated(len %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* akm Suite are included in the RSNE */
		*end_field = RSN_FIELD_AKM;
		return TRUE;
	}

	pStaTmp += sizeof(USHORT) + Count * LEN_OUI_SUITE;

	/* check rsn capabilities */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : RSN capabilities is truncated(len %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* rsn capabilities are included in the RSNE */
		*end_field = RSN_FIELD_RSN_CAP;
		return TRUE;
	}

	pStaTmp += 2;

	/* check PMKID */
	len += 2;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s :  pmkid cnt is truncated(%d)\n", __func__, eid_ptr->Len));
		return FALSE;
	}

	/* Store pmkid count */
	NdisMoveMemory(&Count, pStaTmp, sizeof(USHORT));
	Count = cpu2le16(Count);

	len += Count * 16;

	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : PMKID is truncated(len %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* PMKID are included in the RSNE */
		*end_field = RSN_FIELD_PMKID;
		return TRUE;
	}

	pStaTmp += 2 + Count * 16;


#ifdef DOT11W_PMF_SUPPORT
	/* check Group Management Cipher Suite*/
	len += LEN_OUI_SUITE;
	if (eid_ptr->Len < len) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : Group Management Cipher Suite is truncated(len %d)\n", __func__, eid_ptr->Len));
		return FALSE;
	} else if (eid_ptr->Len == len) {
		/* PMKID are included in the RSNE */
		*end_field = RSN_FIELD_GROUP_MGMT_CIPHER;
		return TRUE;
	}

	pStaTmp += LEN_OUI_SUITE;
#endif

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s : extensible element len %d\n", __func__, eid_ptr->Len - len));
	*end_field = RSN_FIELD_EXTENSIBLE_ELE;
	return TRUE;
}



UINT WPAValidateRSNIE (
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PUCHAR pRsnIe,
	IN UCHAR rsnie_len)
{
	PEID_STRUCT  eid_ptr;
	UCHAR end_field = 0;
	UINT res = MLME_SUCCESS;
#ifdef DOT1X_SUPPORT
	pSecConfigEntry->IEEE8021X = pSecConfigSelf->IEEE8021X;
#endif /* DOT1X_SUPPORT */

	if (rsnie_len == 0)
		return MLME_SUCCESS;

	eid_ptr = (PEID_STRUCT)pRsnIe;

	if (wpa_rsne_sanity(pRsnIe, rsnie_len, &end_field) == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s : wpa_rsne_sanity fail\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s : peer RSNE end field is %d\n", __func__, end_field));


	/* Check group cipher */
	if ((end_field >= RSN_FIELD_GROUP_CIPHER)
		&& !WPACheckGroupCipher(pSecConfigSelf, pSecConfigEntry, eid_ptr)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid group cipher !!!\n", __func__));
		return MLME_INVALID_GROUP_CIPHER;
	} else if (end_field < RSN_FIELD_GROUP_CIPHER
			&& IS_CIPHER_CCMP128(pSecConfigSelf->GroupCipher))
		pSecConfigEntry->GroupCipher = pSecConfigSelf->GroupCipher;
	else if (end_field < RSN_FIELD_GROUP_CIPHER) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid group cipher(peer use default cipher) !!!\n", __func__));
		return MLME_INVALID_GROUP_CIPHER;
	}

	/* Check pairwise cipher */
	if ((end_field >= RSN_FIELD_PAIRWISE_CIPHER)
		&& !WPACheckUcast(pSecConfigSelf, pSecConfigEntry, eid_ptr)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid pairwise cipher !!!\n", __func__));
		return MLME_INVALID_PAIRWISE_CIPHER;
	} else if (end_field < RSN_FIELD_PAIRWISE_CIPHER
			&& IS_CIPHER_CCMP128(pSecConfigSelf->PairwiseCipher))
		pSecConfigEntry->PairwiseCipher = pSecConfigSelf->PairwiseCipher; /* Ellis todo mixed mode */
	else if (end_field < RSN_FIELD_PAIRWISE_CIPHER) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid pairwise cipher(peer use default cipher) !!!\n", __func__));
		return MLME_INVALID_PAIRWISE_CIPHER;
	}

	/* Check AKM */
	if ((end_field >= RSN_FIELD_AKM)
		&& !WPACheckAKM(pSecConfigSelf, pSecConfigEntry, eid_ptr)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid AKM !!!\n", __func__));
		return MLME_INVALID_AKMP;
	} else if (end_field < RSN_FIELD_AKM
			&& IS_AKM_WPA2(pSecConfigSelf->AKMMap))
		pSecConfigEntry->AKMMap = pSecConfigSelf->AKMMap; /* Ellis todo mixed mode */
	else if (end_field < RSN_FIELD_AKM) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[ERROR]%s : invalid AKM(peer use default akm) !!!\n", __func__));
		return MLME_INVALID_AKMP;
	}
	/* Check PMKID */
	if ((end_field >= RSN_FIELD_PMKID)
		&& !wpa_check_pmkid(pRsnIe, rsnie_len, pSecConfigEntry->AKMMap)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : invalid PMKID !!!\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}
#ifdef DOT11W_PMF_SUPPORT
	else {
		res = PMF_RsnCapableValidation(pRsnIe, rsnie_len, pSecConfigSelf->PmfCfg.MFPC, pSecConfigSelf->PmfCfg.MFPR,
					pSecConfigSelf->PmfCfg.igtk_cipher, end_field, pSecConfigEntry);
		if (res != MLME_SUCCESS)
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("[PMF]%s : Invalid PMF Capability !!!\n", __func__));
	}

#endif /* DOT11W_PMF_SUPPORT */
	return res;
}


VOID WpaDerivePTK(
	IN	UCHAR *PMK,
	IN	UCHAR *ANonce,
	IN	UCHAR *AA,
	IN	UCHAR *SNonce,
	IN	UCHAR *SA,
	OUT UCHAR *output,
	IN	UINT len)
{
	UCHAR concatenation[76];
	UINT CurrPos = 0;
	UCHAR temp[32];
	UCHAR Prefix[] = {'P', 'a', 'i', 'r', 'w', 'i', 's', 'e', ' ', 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};
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


/*
	========================================================================

	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.
	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2

	========================================================================
*/

VOID WpaDerivePTK_KDF_256(
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
					  'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'
					 };
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

	/*TODO: temporary mofification here, will move the length of pmk at outside.*/
	if (len > LEN_AES_PTK)
		KDF(PMK, LEN_PMK_SHA384, Prefix, 22, concatenation, 76, output, len);
	else
		KDF(PMK, LEN_PMK, Prefix, 22, concatenation, 76, output, len);
}


/*
	========================================================================

	Routine Description:
		It utilizes PRF-384 or PRF-512 to derive session-specific keys from a PMK.
		It shall be called by 4-way handshake processing.
	Note:
		Refer to IEEE 802.11i-2004 8.5.1.2

	========================================================================
*/

VOID WpaDerivePTK_KDF_384(
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
					  'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'
					 };
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
	hex_dump("[PMF]PMK", PMK, LEN_PMK_SHA384);
	hex_dump("[PMF]concatenation=", concatenation, 76);
	/* Calculate a key material through FT-KDF */
	KDF_384(PMK, LEN_PMK_SHA384, Prefix, 22, concatenation, 76, output, len);
}



VOID WpaDeriveGTK(
	IN  UCHAR *GMK,
	IN  UCHAR *GNonce,
	IN  UCHAR *AA,
	OUT UCHAR *output,
	IN  UINT len)
{
	UCHAR   concatenation[76];
	UINT    CurrPos = 0;
	UCHAR   temp[80];
	UCHAR Prefix[] = {'G', 'r', 'o', 'u', 'p', ' ', 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n'};

	NdisMoveMemory(&concatenation[CurrPos], AA, 6);
	CurrPos += 6;
	NdisMoveMemory(&concatenation[CurrPos], GNonce, 32);
	CurrPos += 32;
	PRF(GMK, LEN_PMK, Prefix,  19, concatenation, 38, temp, len);
	NdisMoveMemory(output, temp, len);
}

VOID WPA_ConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf)
{
	PKDE_HDR pHdr;

	pHdr = (PKDE_HDR)pBuf;
	NdisZeroMemory(pHdr, sizeof(KDE_HDR));
	pHdr->Type = WPA_KDE_TYPE;
	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */
	pHdr->Len = 4 + data_len;
	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;
}

VOID WPAInstallKey(
	IN PRTMP_ADAPTER pAd,
	IN struct _ASIC_SEC_INFO *pInfo,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install)
{
	struct _SEC_KEY_INFO *pKey = &pInfo->Key;

	if (IS_CIPHER_TKIP(pInfo->Cipher))
		pKey->KeyLen = LEN_TKIP_TK;
	else if (IS_CIPHER_CCMP128(pInfo->Cipher))
		pKey->KeyLen = LEN_CCMP128_TK;
	else if (IS_CIPHER_CCMP256(pInfo->Cipher))
		pKey->KeyLen = LEN_CCMP256_TK;
	else if (IS_CIPHER_GCMP128(pInfo->Cipher))
		pKey->KeyLen = LEN_GCMP128_TK;
	else if (IS_CIPHER_GCMP256(pInfo->Cipher))
		pKey->KeyLen = LEN_GCMP256_TK;

	if (pInfo->IGTKKeyLen != 0) {
		if (IS_CIPHER_BIP_GMAC128(pInfo->Cipher)
			|| IS_CIPHER_BIP_GMAC256(pInfo->Cipher)
			|| IS_CIPHER_BIP_CMAC256(pInfo->Cipher))
			; /* do nothing due to hw not support*/
		else if (pKey->KeyLen + pInfo->IGTKKeyLen > sizeof(pKey->Key))
			; /* do nothing due to fw not support*/
		else if (IS_CIPHER_CCMP128(pInfo->Cipher) ||
			IS_CIPHER_CCMP256(pInfo->Cipher) ||
			IS_CIPHER_GCMP128(pInfo->Cipher) ||
			IS_CIPHER_GCMP256(pInfo->Cipher)) {
			os_move_mem(&pKey->Key[pKey->KeyLen], pInfo->IGTK, pInfo->IGTKKeyLen);
			pKey->KeyLen += pInfo->IGTKKeyLen;
		}
	}

	if (IS_CIPHER_TKIP(pInfo->Cipher)) {
		if (bAE) {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
		} else {
			os_move_mem(pKey->TxMic, &pKey->Key[LEN_TK + 8], LEN_TKIP_MIC);
			os_move_mem(pKey->RxMic, &pKey->Key[LEN_TK], LEN_TKIP_MIC);
		}
	}

	if (is_install)
		HW_ADDREMOVE_KEYTABLE(pAd, pInfo);
}

VOID WPACalculateMIC(
	IN UCHAR KeyDescVer,
	IN UINT32 AKMMap,
	IN UCHAR *PTK,
	IN UINT8 key_deri_alg,
	OUT PEAPOL_PACKET pMsg)
{
	UCHAR *OutBuffer;
	ULONG FrameLen = 0;
	UCHAR	mic[LEN_KEY_DESC_MIC_MAX];
	UCHAR	digest[80];
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	/* allocate memory for MIC calculation*/
	os_alloc_mem(NULL, (PUCHAR *)&OutBuffer, 512);

	if (OutBuffer == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!CalculateMIC: no memory!!!\n"));
		return;
	}

	/* make a frame for calculating MIC.*/
	MakeOutgoingFrame(OutBuffer, &FrameLen,
					  CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4, pMsg,
					  END_OF_ARGS);
	NdisZeroMemory(mic, sizeof(mic));

	/* Calculate MIC*/
	if (KeyDescVer == KEY_DESC_AES) {
		RT_HMAC_SHA1(PTK, LEN_PTK_KCK, OutBuffer,  FrameLen, digest, SHA1_DIGEST_SIZE);
		NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
	} else if (KeyDescVer == KEY_DESC_TKIP)
		RT_HMAC_MD5(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, MD5_DIGEST_SIZE);
	else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) && IS_AKM_SHA384(AKMMap)) {
		RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC_SHA384);
		mic_len = LEN_KEY_DESC_MIC_SHA384;
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		   IS_AKM_OWE(AKMMap) &&
		   (key_deri_alg == SEC_KEY_DERI_SHA256)) {
		RT_HMAC_SHA256(PTK, LEN_PTK_KCK, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC);
		mic_len = LEN_KEY_DESC_MIC;
	} else if ((KeyDescVer == KEY_DESC_NOT_DEFINED) &&
		   IS_AKM_OWE(AKMMap) &&
		   (key_deri_alg == SEC_KEY_DERI_SHA384)) {
		/*TODO: OWE SHA521*/
		RT_HMAC_SHA384(PTK, LEN_PTK_KCK_SHA384, OutBuffer, FrameLen, mic, LEN_KEY_DESC_MIC_SHA384);
		mic_len = LEN_KEY_DESC_MIC_SHA384;
	} else if ((KeyDescVer == KEY_DESC_EXT)
		|| (KeyDescVer == KEY_DESC_NOT_DEFINED)) {
		UINT mlen = AES_KEY128_LENGTH;

		AES_CMAC(OutBuffer, FrameLen, PTK, LEN_PTK_KCK, mic, &mlen);
	}

	/* store the calculated MIC*/
	NdisMoveMemory(pMsg->KeyDesc.KeyMicAndData, mic, mic_len);
	os_free_mem(OutBuffer);
}


VOID WPAInsertRSNIE (
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len)
{
	PUCHAR	pTmpBuf;
	ULONG	TempLen = 0;
	UINT8	extra_len = 0;
	UINT16	pmk_count = 0;
	UCHAR	ie_num;
	UINT8	total_len = 0;

	pTmpBuf = pFrameBuf;

	/* PMKID-List Must larger than 0 and the multiple of 16. */
	if (pmkid_len > 0 && ((pmkid_len & 0x0f) == 0)) {
		extra_len = sizeof(UINT16) + pmkid_len;
		pmk_count = (pmkid_len >> 4);
		pmk_count = cpu2le16(pmk_count);
	} else
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : no PMKID-List included(%d).\n", __func__, pmkid_len));

	if (rsnie_len != 0) {
		ie_num = IE_WPA;
		total_len = rsnie_len;

		if (NdisEqualMemory(rsnie_ptr + 2, OUI_WPA2_CIPHER, sizeof(OUI_WPA2_CIPHER))) {
			ie_num = IE_RSN;
			total_len += extra_len;
		}

		/* construct RSNIE body */
		MakeOutgoingFrame(pTmpBuf, &TempLen,
						  1, &ie_num,
						  1, &total_len,
						  rsnie_len, rsnie_ptr,
						  END_OF_ARGS);
		pTmpBuf += TempLen;
		*pFrameLen = *pFrameLen + TempLen;

		if (ie_num == IE_RSN) {
			/* Insert PMKID-List field */
			if (extra_len > 0) {
				MakeOutgoingFrame(pTmpBuf, &TempLen,
								  2, &pmk_count,
								  pmkid_len, pmkid_ptr,
								  END_OF_ARGS);
				pTmpBuf += TempLen;
				*pFrameLen = *pFrameLen + TempLen;
			}
		}
	}

	return;
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
	Note:
		It's defined in IEEE 802.11-2007 Figure 8-25.

	========================================================================
*/
VOID WPAConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf)
{
	PKDE_HDR pHdr;

	pHdr = (PKDE_HDR)pBuf;
	NdisZeroMemory(pHdr, sizeof(KDE_HDR));
	pHdr->Type = WPA_KDE_TYPE;
	/* The Length field specifies the number of octets in the OUI, Data
	   Type, and Data fields. */
	pHdr->Len = 4 + data_len;
	NdisMoveMemory(pHdr->OUI, OUI_WPA2, 3);
	pHdr->DataType = data_type;
}


VOID WPAConstructEapolKeyData(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN	UCHAR keyDescVer,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg)
{
	UCHAR *mpool, *Key_Data, *eGTK;
	ULONG data_offset;
	BOOLEAN bWPA2 = TRUE;
	BOOLEAN GTK_Included = FALSE;
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	UINT8 *key_data_len_ptr = NULL;
	UINT8 *key_data_ptr = NULL;

	if (MsgType == EAPOL_PAIR_MSG_1
		|| MsgType == EAPOL_PAIR_MSG_4
		|| MsgType == EAPOL_GROUP_MSG_2)
		return;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecPairwise->AKMMap) || IS_AKM_WPA1PSK(pSecPairwise->AKMMap))
		bWPA2 = FALSE;

	/* allocate memory pool*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, 1500);

	if (mpool == NULL)
		return;

	if (IS_AKM_SHA384(pSecPairwise->AKMMap) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		mic_len = LEN_KEY_DESC_MIC_SHA384;

	key_data_len_ptr = pMsg->KeyDesc.KeyMicAndData + mic_len;
	key_data_ptr = key_data_len_ptr + 2;

	/* eGTK Len = 512 */
	eGTK = (UCHAR *) ROUND_UP(mpool, 4);
	/* Key_Data Len = 512 */
	Key_Data = (UCHAR *) ROUND_UP(eGTK + 512, 4);
	NdisZeroMemory(Key_Data, 512);
	SET_UINT16_TO_ARRARY(key_data_len_ptr, 0);
	data_offset = 0;

	/* Encapsulate RSNIE in pairwise_msg2 & pairwise_msg3 */
	if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3)) {
		UCHAR *RSNIE = NULL;
		UCHAR RSNIE_LEN = 0;
		CHAR rsne_idx = 0;
		SEC_RSNIE_TYPE RSNType = SEC_RSNIE_WPA1_IE;
		PUINT8	pmkid_ptr = NULL;
		UINT8	pmkid_len = 0;

#ifdef DOT11R_FT_SUPPORT
		if (IS_FT_RSN_STA(pEntry)) {
			/* YF_FT */
			pEntry->FT_Status = TX_EAPOL_3;
			WPAMakeRSNIE(pEntry->wdev->wdev_type, pSecGroup, pEntry);
		}
#endif /* DOT11R_FT_SUPPORT */

		if (bWPA2)
			RSNType = SEC_RSNIE_WPA2_IE;

		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (pSecGroup->RSNE_Type[rsne_idx] == RSNType) {
				RSNIE = &pSecGroup->RSNE_Content[rsne_idx][0];
				RSNIE_LEN = pSecGroup->RSNE_Len[rsne_idx];
				break;
			}
		}

		WPAInsertRSNIE(&Key_Data[data_offset],
					   &data_offset,
					   RSNIE,
					   RSNIE_LEN,
					   pmkid_ptr,
					   pmkid_len);
	}

#ifdef DOT11R_FT_SUPPORT

	/* Encapsulate MDIE if FT is enabled*/
	if (IS_FT_RSN_STA(pEntry) && bWPA2 &&
		((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3))) {
		/*	The MDIE shall be the same as those provided in
		    the AP's (Re)association Response frame. */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			NdisMoveMemory(&Key_Data[data_offset], pEntry->InitialMDIE, 5);
			data_offset += 5;
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#endif /* DOT11R_FT_SUPPORT */

	/* Encapsulate GTK */
	/* Only for pairwise_msg3_WPA2 and group_msg1*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {
		UINT8 gtk_len = 0;

		/* Decide the GTK length */
		if (IS_CIPHER_TKIP(pSecGroup->GroupCipher))
			gtk_len = LEN_TKIP_TK;
		else if (IS_CIPHER_CCMP128(pSecGroup->GroupCipher))
			gtk_len = LEN_CCMP128_TK;
		else if (IS_CIPHER_CCMP256(pSecGroup->GroupCipher))
			gtk_len = LEN_CCMP256_TK;
		else if (IS_CIPHER_GCMP128(pSecGroup->GroupCipher))
			gtk_len = LEN_GCMP128_TK;
		else if (IS_CIPHER_GCMP256(pSecGroup->GroupCipher))
			gtk_len = LEN_GCMP256_TK;

		/* Insert GTK KDE format in WAP2 mode */
		if (bWPA2) {
			/* Construct the common KDE format */
			WPAConstructKdeHdr(KDE_GTK, 2 + gtk_len, &Key_Data[data_offset]);
			data_offset += sizeof(KDE_HDR);
			/* GTK KDE format - 802.11i-2004  Figure-43x*/
			Key_Data[data_offset] = (pSecGroup->GroupKeyId & 0x03);
			Key_Data[data_offset + 1] = 0x00;	/* Reserved Byte*/
			data_offset += 2;
		}

		/* Fill in GTK */
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)

		if (pEntry->pMbss->HotSpotCtrl.HotSpotEnable
			&& pEntry->pMbss->HotSpotCtrl.DGAFDisable) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[HOTSPOT]:%s - Unique GTK for each STA\n", __func__));
			NdisMoveMemory(&Key_Data[data_offset], pSecPairwise->HsUniGTK, gtk_len);
		} else
#endif /* defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT) */
		{
			NdisMoveMemory(&Key_Data[data_offset], pSecGroup->GTK, gtk_len);
		}

		data_offset += gtk_len;
#ifdef DOT11W_PMF_SUPPORT

		/* Insert IGTK KDE to Key_Data field */
		if ((bWPA2)
			&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_WARN, ("[PMF]:%s - Insert IGTK\n", __func__));
			PMF_InsertIGTKKDE(pEntry->pAd, pEntry->func_tb_idx, &Key_Data[data_offset], &data_offset);
		}

#endif /* DOT11W_PMF_SUPPORT */
		GTK_Included = TRUE;
	}

#ifdef DOT11R_FT_SUPPORT

	/* Encapsulate the related IE of FT when FT is enabled */
	if (IS_FT_RSN_STA(pEntry) && bWPA2 &&
		((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3))) {
		/*	Encapsulate FTIE. The MDIE shall be the same
			as those provided in the AP's (Re)association Response frame. */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			NdisMoveMemory(&Key_Data[data_offset], pEntry->InitialFTIE, pEntry->InitialFTIE_Len);
			data_offset += pEntry->InitialFTIE_Len;

			if (MsgType == EAPOL_PAIR_MSG_3) {
				/* INSERT TIE with PKT keylifetime, reassoc deadline
				   And reassoc deadline <= PTK key lifetime
				 */
				/* YF_TIE */
				FT_InsertTimeoutIntervalIE(&Key_Data[data_offset], &data_offset,
					REASSOC_DEADLINE_INTERVAL, MAC_TABLE_ASSOC_TIMEOUT);

				FT_InsertTimeoutIntervalIE(&Key_Data[data_offset], &data_offset,
					KEY_LIFETIME_INTERVAL, 600); /* Sync PMKID Cache Period */

			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#endif /* DOT11R_FT_SUPPORT */

	/* If the Encrypted Key Data subfield (of the Key Information field)
	   is set, the entire Key Data field shall be encrypted. */
	/* This whole key-data field shall be encrypted if a GTK is included.*/
	/* Encrypt the data material in key data field with KEK*/
	if (GTK_Included) {
		if ((keyDescVer == KEY_DESC_EXT)
			|| (keyDescVer == KEY_DESC_NOT_DEFINED)
			|| (keyDescVer == KEY_DESC_AES)) {
			UCHAR remainder = 0;
			UCHAR pad_len = 0;
			UINT wrap_len = 0;
			UINT8 kck_len = LEN_PTK_KCK;
			UINT8 kek_len = LEN_PTK_KEK;

			if (IS_AKM_SHA384(pSecPairwise->AKMMap) ||
			   (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384)) {
				kck_len = LEN_PTK_KCK_SHA384;
				kek_len = LEN_PTK_KEK_SHA384;
			}

			/* Key Descriptor Version 2 or 3: AES key wrap, defined in IETF RFC 3394, */
			/* shall be used to encrypt the Key Data field using the KEK field from */
			/* the derived PTK.*/
			/* If the Key Data field uses the NIST AES key wrap, then the Key Data field */
			/* shall be padded before encrypting if the key data length is less than 16 */
			/* octets or if it is not a multiple of 8. The padding consists of appending*/
			/* a single octet 0xdd followed by zero or more 0x00 octets. */
			remainder = data_offset & 0x07;

			if (remainder != 0) {
				INT i;

				pad_len = (8 - remainder);
				Key_Data[data_offset] = 0xDD;

				for (i = 1; i < pad_len; i++)
					Key_Data[data_offset + i] = 0;

				data_offset += pad_len;
			}

			AES_Key_Wrap(Key_Data, (UINT) data_offset,
						 &pSecPairwise->PTK[kck_len], kek_len,
						 eGTK, &wrap_len);
			data_offset = wrap_len;
		} else {
			TKIP_GTK_KEY_WRAP(&pSecPairwise->PTK[LEN_PTK_KCK],
							  pMsg->KeyDesc.KeyIv,
							  Key_Data,
							  data_offset,
							  eGTK);
		}

		NdisMoveMemory(key_data_ptr, eGTK, data_offset);
	} else
		NdisMoveMemory(key_data_ptr, Key_Data, data_offset);

	/* Update key data length field and total body length*/
	SET_UINT16_TO_ARRARY(key_data_len_ptr, data_offset);
	INC_UINT16_TO_ARRARY(pMsg->Body_Len, data_offset);
	os_free_mem(mpool);
}


VOID WPAConstructEapolMsg(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg)
{
	BOOLEAN bWPA2 = TRUE;
	UCHAR KeyDescVer = 0;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecPairwise->AKMMap) || IS_AKM_WPA1PSK(pSecPairwise->AKMMap))
		bWPA2 = FALSE;

	/* Init Packet and Fill header */
	pMsg->ProVer = EAPOL_VER;
	pMsg->ProType = EAPOLKey;
	/* Default 95 bytes, the EAPoL-Key descriptor exclude Key-data field*/
	SET_UINT16_TO_ARRARY(pMsg->Body_Len, MIN_LEN_OF_EAPOL_KEY_MSG);

	if (IS_AKM_SHA384(pSecPairwise->AKMMap) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		INC_UINT16_TO_ARRARY(pMsg->Body_Len, LEN_KEY_DESC_MIC_SHA384 - LEN_KEY_DESC_MIC);

	/* Fill in EAPoL descriptor*/
	if (bWPA2)
		pMsg->KeyDesc.Type = WPA2_KEY_DESC;
	else
		pMsg->KeyDesc.Type = WPA1_KEY_DESC;

	if (IS_AKM_WPA3_192BIT(pSecPairwise->AKMMap) ||
	    IS_AKM_WPA3PSK(pSecPairwise->AKMMap) ||
	    IS_AKM_OWE(pSecPairwise->AKMMap) ||
#ifdef CONFIG_HOTSPOT_R2
	    CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE) ||
#endif /* CONFIG_HOTSPOT_R2 */
	    (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		KeyDescVer = KEY_DESC_NOT_DEFINED;
	/* Key Descriptor Version (bits 0-2) specifies the key descriptor version type*/
	/* AKM is 00-0F-AC:3, 00-0F-AC:4, , 00-0F-AC:5, , 00-0F-AC:6 */
	else if (IS_AKM_SHA256(pSecPairwise->AKMMap)
#ifdef DOT11W_PMF_SUPPORT
		|| (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA256)
#endif /* DOT11W_PMF_SUPPORT */
	   )
		KeyDescVer = KEY_DESC_EXT;
	else if (((MsgType <= EAPOL_PAIR_MSG_4) && IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher))
			 || ((MsgType >= EAPOL_GROUP_MSG_1) && IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher)))   /* Must see pairwise cipher*/
		KeyDescVer = KEY_DESC_TKIP;
	else
		KeyDescVer = KEY_DESC_AES;

	pMsg->KeyDesc.KeyInfo.KeyDescVer = KeyDescVer;

	/* Specify Key Type as Group(0) or Pairwise(1)*/
	if (MsgType >= EAPOL_GROUP_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyType = GROUPKEY;
	else
		pMsg->KeyDesc.KeyInfo.KeyType = PAIRWISEKEY;

	/* Specify Key Index, only group_msg1_WPA1*/
	if (!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyIndex = pSecGroup->GroupKeyId;

	if (MsgType == EAPOL_PAIR_MSG_3)
		pMsg->KeyDesc.KeyInfo.Install = 1;

	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3) || (MsgType == EAPOL_GROUP_MSG_1))
		pMsg->KeyDesc.KeyInfo.KeyAck = 1;

	if (MsgType != EAPOL_PAIR_MSG_1)
		pMsg->KeyDesc.KeyInfo.KeyMic = 1;

	if ((bWPA2 && (MsgType >= EAPOL_PAIR_MSG_3)) ||
		(!bWPA2 && (MsgType >= EAPOL_GROUP_MSG_1)))
		pMsg->KeyDesc.KeyInfo.Secure = 1;

	/* This subfield shall be set, and the Key Data field shall be encrypted, if
	   any key material (e.g., GTK or SMK) is included in the frame. */
	if (bWPA2 && ((MsgType == EAPOL_PAIR_MSG_3)
				  || (MsgType == EAPOL_GROUP_MSG_1)))
		pMsg->KeyDesc.KeyInfo.EKD_DL = 1;

	/* key Information element has done. */
	*(USHORT *)(&pMsg->KeyDesc.KeyInfo) = cpu2le16(*(USHORT *)(&pMsg->KeyDesc.KeyInfo));

	/* Fill in Key Length*/
	if (bWPA2) {
		/* In WPA2 mode, the field indicates the length of pairwise key cipher, */
		/* so only pairwise_msg_1 and pairwise_msg_3 need to fill. */
		if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3)) {
			if (IS_CIPHER_TKIP(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_TKIP_TK;
			else if (IS_CIPHER_CCMP128(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_CCMP128_TK;
			else if (IS_CIPHER_CCMP256(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_CCMP256_TK;
			else if (IS_CIPHER_GCMP128(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_GCMP128_TK;
			else if (IS_CIPHER_GCMP256(pSecPairwise->PairwiseCipher))
				pMsg->KeyDesc.KeyLength[1] = LEN_GCMP256_TK;
		}
	} else {
		UINT32 Cipher = 0;

		if (MsgType >= EAPOL_GROUP_MSG_1)
			Cipher = pSecGroup->GroupCipher;
		else
			Cipher = pSecPairwise->PairwiseCipher;

		if (IS_CIPHER_TKIP(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_TKIP_TK;
		else if (IS_CIPHER_CCMP128(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_CCMP128_TK;
		else if (IS_CIPHER_CCMP256(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_CCMP256_TK;
		else if (IS_CIPHER_GCMP128(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_GCMP128_TK;
		else if (IS_CIPHER_GCMP256(Cipher))
			pMsg->KeyDesc.KeyLength[1] = LEN_GCMP256_TK;
	}

	/* Fill in replay counter */
	NdisMoveMemory(pMsg->KeyDesc.ReplayCounter, pSecPairwise->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Fill Key Nonce field	  */
	/* ANonce : pairwise_msg1 & pairwise_msg3*/
	/* SNonce : pairwise_msg2*/
	/* GNonce : group_msg1_wpa1 */
	if ((MsgType == EAPOL_PAIR_MSG_1) || (MsgType == EAPOL_PAIR_MSG_3))
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecPairwise->Handshake.ANonce, LEN_KEY_DESC_NONCE);
	else if (MsgType == EAPOL_PAIR_MSG_2)
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecPairwise->Handshake.SNonce, LEN_KEY_DESC_NONCE);
	else if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1)) {
		NdisMoveMemory(pMsg->KeyDesc.KeyNonce, pSecGroup->Handshake.GNonce, LEN_KEY_DESC_NONCE);
		/* Fill key IV - WPA2 as 0, WPA1 as random*/
		/* Suggest IV be random number plus some number,*/
		NdisMoveMemory(pMsg->KeyDesc.KeyIv, &pSecGroup->Handshake.GNonce[16], LEN_KEY_DESC_IV);
		pMsg->KeyDesc.KeyIv[15] += 2;
	}

	/* Fill Key RSC field		 */
	/* It contains the RSC for the GTK being installed.*/
	if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1))
		NdisMoveMemory(pMsg->KeyDesc.KeyRsc, pSecGroup->Handshake.RSC, 6);

	/* Clear Key MIC field for MIC calculation later   */
	if (IS_AKM_SHA384(pSecPairwise->AKMMap) || (pSecPairwise->key_deri_alg == SEC_KEY_DERI_SHA384))
		NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, LEN_KEY_DESC_MIC_SHA384);
	else
		NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, LEN_KEY_DESC_MIC);
	WPAConstructEapolKeyData(pEntry,
							 MsgType,
							 KeyDescVer,
							 pSecPairwise,
							 pSecGroup,
							 pMsg);

	/* Calculate MIC and fill in KeyMic Field except Pairwise Msg 1.*/
	if (MsgType != EAPOL_PAIR_MSG_1)
		WPACalculateMIC(KeyDescVer, pSecPairwise->AKMMap, pSecPairwise->PTK, pSecPairwise->key_deri_alg, pMsg);

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("===> %s for %s %s\n", ((bWPA2) ? "WPA2" : "WPA"), __func__, GetEapolMsgType(MsgType)));
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	     Body length = %d\n", CONV_ARRARY_TO_UINT16(pMsg->Body_Len)));
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	     Key length  = %d\n", CONV_ARRARY_TO_UINT16(pMsg->KeyDesc.KeyLength)));
}

BOOLEAN WPAParseEapolKeyData(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pKeyData,
	IN UCHAR KeyDataLen,
	IN	UCHAR GroupKeyIndex,
	IN	UCHAR MsgType,
	IN	BOOLEAN bWPA2,
	IN MAC_TABLE_ENTRY * pEntry)
{
	PUCHAR pMyKeyData = pKeyData;
	UCHAR KeyDataLength = KeyDataLen;
	UCHAR GTK[MAX_LEN_GTK];
	UCHAR GTKLEN = 0;
	UCHAR DefaultIdx = 0;
	UCHAR	 skip_offset = 0;
#ifdef DOT11W_PMF_SUPPORT
	UCHAR IGTK[LEN_MAX_IGTK];
	UCHAR IGTKLEN = 0;
	UCHAR IPN[LEN_WPA_TSC];
	UINT8 IGTK_KeyIdx;/* It shall be 4 or 5 */
#endif /*DOT11W_PMF_SUPPORT */
	NdisZeroMemory(GTK, MAX_LEN_GTK);

	/* Verify The RSN IE contained in pairewise_msg_2 && pairewise_msg_3 and skip it*/
	if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_3) {
		{
			if (bWPA2 && MsgType == EAPOL_PAIR_MSG_3) {
				/* skip RSN IE*/
				pMyKeyData += skip_offset;
				KeyDataLength -= skip_offset;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPParseEapolKeyData ==> WPA2/WPA2PSK RSN IE matched in Msg 3, Length(%d)\n", skip_offset));
			} else
				return TRUE;
		}
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPParseEapolKeyData ==> KeyDataLength %d without RSN_IE\n", KeyDataLength));
#ifdef DOT11R_FT_SUPPORT

	if (IS_FT_RSN_STA(pEntry)) {
		PEID_STRUCT pEid;

		pEid = (PEID_STRUCT)pMyKeyData;

		if (pEid->Eid == IE_FT_MDIE) {
			/* Skip MDIE of FT*/
			pMyKeyData += (pEid->Len + 2);
			KeyDataLength -= (pEid->Len + 2);
		}
	}

#endif /* DOT11R_FT_SUPPORT */

	/* Parse KDE format in pairwise_msg_3_WPA2 && group_msg_1_WPA2*/
	if (bWPA2 && (MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1)) {
		PEID_STRUCT pEid;

		pEid = (PEID_STRUCT) pMyKeyData;
		skip_offset = 0;

		while ((skip_offset + 2 + pEid->Len) <= KeyDataLength) {
			switch (pEid->Eid) {
			case WPA_KDE_TYPE: {
				PKDE_HDR pKDE;

				pKDE = (PKDE_HDR)pEid;

				if (NdisEqualMemory(pKDE->OUI, OUI_WPA2_CIPHER, 3)) {
					if (pKDE->DataType == KDE_GTK) {
						PGTK_KDE pKdeGtk;

						pKdeGtk = (PGTK_KDE) &pKDE->octet[0];
						DefaultIdx = pKdeGtk->Kid;
						/* Get GTK length - refer to IEEE 802.11i-2004 p.82 */
						GTKLEN = pKDE->Len - 6;

						if (GTKLEN < LEN_WEP64) {
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: GTK Key length is too short (%d)\n", GTKLEN));
							return FALSE;
						}

						NdisMoveMemory(GTK, pKdeGtk->GTK, GTKLEN);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GTK in KDE format ,DefaultKeyID=%d, KeyLen=%d\n", DefaultIdx, GTKLEN));
					}

#ifdef DOT11W_PMF_SUPPORT
					else if (pKDE->DataType == KDE_IGTK) {
						if (PMF_ExtractIGTKKDE(&pKDE->octet[0], pKDE->Len - 4, IGTK, &IGTKLEN, IPN, &IGTK_KeyIdx) == FALSE) {
							MTWF_LOG(DBG_CAT_SEC, CATSEC_PMF, DBG_LVL_ERROR, ("PMF_ExtractIGTKKDE: %s FAIL\n", __func__));
							return FALSE;
					}
					}

#endif /* DOT11W_PMF_SUPPORT */
				}
			}
			break;
			}

			skip_offset = skip_offset + 2 + pEid->Len;
			pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
		}

		/* skip KDE Info*/
		pMyKeyData += skip_offset;
		KeyDataLength -= skip_offset;
	} else if (!bWPA2 && MsgType == EAPOL_GROUP_MSG_1) {
		DefaultIdx = GroupKeyIndex;
		GTKLEN = KeyDataLength;
		NdisMoveMemory(GTK, pMyKeyData, KeyDataLength);
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GTK without KDE, DefaultKeyID=%d, KeyLen=%d\n", DefaultIdx, GTKLEN));
	}

	/* Sanity check - shared key index must be 0 ~ 3*/
	if (DefaultIdx > 3) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: GTK Key index(%d) is invalid in %s %s\n", DefaultIdx, ((bWPA2) ? "WPA2" : "WPA"), GetEapolMsgType(MsgType)));
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ASIC_SEC_INFO Info = {0};
		/* set key material, TxMic and RxMic		*/
		NdisMoveMemory(pEntry->SecConfig.GTK, GTK, GTKLEN);
		pEntry->SecConfig.GroupKeyId = DefaultIdx;
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
		Info.Direction = SEC_ASIC_KEY_RX;
		Info.Wcid = pEntry->wdev->bss_info_argument.ucBcMcWlanIdx;
		Info.BssIndex = BSS0;
		Info.Cipher = pEntry->SecConfig.GroupCipher;
		Info.KeyIdx = pEntry->SecConfig.GroupKeyId;
		os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
		os_move_mem(Info.Key.Key, pEntry->SecConfig.GTK, LEN_MAX_GTK);
#ifdef DOT11W_PMF_SUPPORT

		if (IGTKLEN != 0) {
#ifdef SOFT_ENCRYPT
			UINT8 idx = 0;
			PPMF_CFG pPmfCfg = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.SecConfig.PmfCfg;

			pPmfCfg->IGTK_KeyIdx = IGTK_KeyIdx;

			if (pPmfCfg->IGTK_KeyIdx == 5)
				idx = 1;

			NdisMoveMemory(&pPmfCfg->IPN[idx][0], IPN, LEN_WPA_TSC);
			NdisMoveMemory(&pPmfCfg->IGTK[idx][0], IGTK, IGTKLEN);
			/* hex_dump("IGTK===>",IGTK,IGTKLEN); */
#endif /* SOFT_ENCRYPT */
			Info.Cipher |= pEntry->SecConfig.PmfCfg.igtk_cipher;
			os_move_mem(Info.IGTK, IGTK, IGTKLEN);
			Info.IGTKKeyLen = IGTKLEN;
		}

#endif /* DOT11W_PMF_SUPPORT */

		/* Prevent the GTK reinstall key attack */
		if ((pEntry->SecConfig.LastGroupKeyId != pEntry->SecConfig.GroupKeyId) ||
			!NdisEqualMemory(pEntry->SecConfig.LastGTK, pEntry->SecConfig.GTK, LEN_MAX_GTK)) {
			WPAInstallKey(pAd, &Info, FALSE, TRUE);
			pEntry->SecConfig.LastGroupKeyId = pEntry->SecConfig.GroupKeyId;
			os_move_mem(pEntry->SecConfig.LastGTK, pEntry->SecConfig.GTK, LEN_MAX_GTK);
			pEntry->AllowUpdateRSC = TRUE;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("ooo%s : the Group install  (%d)\n", __func__, pEntry->wcid));

		} else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("!!!%s : the Group reinstall attack, skip install key (%d)\n", __func__, pEntry->wcid));
		}
	}

#endif /* MT_MAC */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}


BOOLEAN WpaMessageSanity(
	IN PRTMP_ADAPTER pAd,
	IN PEAPOL_PACKET pMsg,
	IN ULONG MsgLen,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR mic[LEN_KEY_DESC_MIC_MAX], digest[80]; /*, KEYDATA[MAX_LEN_OF_RSNIE];*/
	UCHAR *KEYDATA = NULL;
	BOOLEAN bWPA2 = TRUE;
	KEY_INFO EapolKeyInfo;
	BOOLEAN bReplayDiff = FALSE;
	PHANDSHAKE_PROFILE pHandshake4Way  = &pSecConfig->Handshake;
	UINT8 mic_len = LEN_KEY_DESC_MIC;
	UINT8 *key_data_len_ptr = NULL;
	UINT8 *key_data_ptr = NULL;

	/* 0. Check MsgType*/
	if ((MsgType > EAPOL_GROUP_MSG_2) || (MsgType < EAPOL_PAIR_MSG_1)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The message type is invalid(%d)!\n", MsgType));
		return FALSE;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&KEYDATA, MAX_LEN_OF_RSNIE);

	if (KEYDATA == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return FALSE;
	}

	NdisZeroMemory(mic, sizeof(mic));
	NdisZeroMemory(digest, sizeof(digest));
	NdisZeroMemory(KEYDATA, MAX_LEN_OF_RSNIE);
	NdisZeroMemory((PUCHAR)&EapolKeyInfo, sizeof(EapolKeyInfo));
	NdisMoveMemory((PUCHAR)&EapolKeyInfo, (PUCHAR)&pMsg->KeyDesc.KeyInfo, sizeof(KEY_INFO));
	*((USHORT *)&EapolKeyInfo) = cpu2le16(*((USHORT *)&EapolKeyInfo));

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecConfig->AKMMap) || IS_AKM_WPA1PSK(pSecConfig->AKMMap))
		bWPA2 = FALSE;

	/* 1. Replay counter check */
	if (MsgType == EAPOL_PAIR_MSG_1 || MsgType == EAPOL_PAIR_MSG_3 || MsgType == EAPOL_GROUP_MSG_1) { /* For supplicant*/
		/* First validate replay counter, only accept message with larger replay counter.*/
		/* Let equal pass, some AP start with all zero replay counter*/
		UCHAR ZeroReplay[LEN_KEY_DESC_REPLAY];

		NdisZeroMemory(ZeroReplay, LEN_KEY_DESC_REPLAY);

		if ((RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY) != 1) &&
			(RTMPCompareMemory(pMsg->KeyDesc.ReplayCounter, ZeroReplay, LEN_KEY_DESC_REPLAY) != 0))
			bReplayDiff = TRUE;
	} else if (MsgType == EAPOL_PAIR_MSG_2 || MsgType == EAPOL_PAIR_MSG_4 || MsgType == EAPOL_GROUP_MSG_2) {	/* For authenticator*/
		/* check Replay Counter coresponds to MSG from authenticator, otherwise discard*/
		if (!NdisEqualMemory(pMsg->KeyDesc.ReplayCounter, pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY))
			bReplayDiff = TRUE;
	}

	/* Replay Counter different condition*/
	if (bReplayDiff) {
		/* send wireless event - for replay counter different*/
		RTMPSendWirelessEvent(pAd, IW_REPLAY_COUNTER_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

		if (MsgType < EAPOL_GROUP_MSG_1)
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Replay Counter Different in pairwise msg %d of 4-way handshake!\n", MsgType));
		else
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Replay Counter Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));

		hex_dump("Receive replay counter ", pMsg->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
		hex_dump("Current replay counter ", pHandshake4Way->ReplayCounter, LEN_KEY_DESC_REPLAY);
		goto LabelErr;
	}

	if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		mic_len  = LEN_KEY_DESC_MIC_SHA384;

	key_data_len_ptr = pMsg->KeyDesc.KeyMicAndData + mic_len;
	key_data_ptr = key_data_len_ptr + 2;

	/* 2. Verify MIC except Pairwise Msg1*/
	if (MsgType != EAPOL_PAIR_MSG_1) {
		UCHAR rcvd_mic[LEN_KEY_DESC_MIC_MAX];
		UINT eapol_len = CONV_ARRARY_TO_UINT16(pMsg->Body_Len) + 4;
		/* Record the received MIC for check later*/
		NdisMoveMemory(rcvd_mic, pMsg->KeyDesc.KeyMicAndData, mic_len);
		NdisZeroMemory(pMsg->KeyDesc.KeyMicAndData, mic_len);

		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): peer KeyDescVer = %d\n", __func__, EapolKeyInfo.KeyDescVer));

		if (EapolKeyInfo.KeyDescVer == KEY_DESC_TKIP) /* TKIP*/
			RT_HMAC_MD5(pSecConfig->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, mic, MD5_DIGEST_SIZE);
		else if (EapolKeyInfo.KeyDescVer == KEY_DESC_AES) {	/* AES        */
			RT_HMAC_SHA1(pSecConfig->PTK, LEN_PTK_KCK, (PUCHAR)pMsg, eapol_len, digest, SHA1_DIGEST_SIZE);
			NdisMoveMemory(mic, digest, LEN_KEY_DESC_MIC);
		} else if (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED && IS_AKM_SHA384(pSecConfig->AKMMap))
			RT_HMAC_SHA384(pSecConfig->PTK, LEN_PTK_KCK_SHA384, (PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC_SHA384);
		else if (IS_AKM_WPA3PSK(pSecConfig->AKMMap)) {
			UINT mlen = AES_KEY128_LENGTH;

			AES_CMAC((PUCHAR)pMsg, eapol_len, pSecConfig->PTK, LEN_PTK_KCK, mic, &mlen);
		} else if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256))
			/*FIXME: OWE SHA521 support*/
			RT_HMAC_SHA256(pSecConfig->PTK, LEN_PTK_KEK, (PUCHAR)pMsg, eapol_len, mic, LEN_KEY_DESC_MIC);
		else if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
			RT_HMAC_SHA384(pSecConfig->PTK,
				       LEN_PTK_KCK_SHA384,
				       (PUCHAR)pMsg,
				       eapol_len,
				       mic,
				       LEN_KEY_DESC_MIC_SHA384);
		else if ((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)
			|| (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED)) { /* AES-128 */
			UINT mlen = AES_KEY128_LENGTH;

			AES_CMAC((PUCHAR)pMsg, eapol_len, pSecConfig->PTK, LEN_PTK_KCK, mic, &mlen);
		}

		if (!NdisEqualMemory(rcvd_mic, mic, LEN_KEY_DESC_MIC)) {
			/* send wireless event - for MIC different*/
			RTMPSendWirelessEvent(pAd, IW_MIC_DIFF_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);

			if (MsgType < EAPOL_GROUP_MSG_1)
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MIC Different in pairwise msg %d of 4-way handshake!\n", MsgType));
			else
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MIC Different in group msg %d of 2-way handshake!\n", (MsgType - EAPOL_PAIR_MSG_4)));

#ifdef VENDOR_FEATURE7_SUPPORT
			{
				int k = 0;
				UCHAR mic_hex[2];
				UCHAR *event_msg = NULL;

				os_alloc_mem(NULL, (UCHAR **)&event_msg, 768);
				if (event_msg) {
					NdisZeroMemory(event_msg, 768);
					sprintf(event_msg, "(%02x:%02x:%02x:%02x:%02x:%02x) --> %s%d (%s) failed\n",
							PRINT_MAC(pEntry->Addr), INF_MAIN_DEV_NAME, pEntry->wdev->wdev_idx, pEntry->pMbss->Ssid);
					strcat(event_msg, "Desired  MIC: ");
					for (k = 0; k < LEN_KEY_DESC_MIC; k++) {
						sprintf(mic_hex, "%02x ", mic[k]);
						strcat(event_msg, mic_hex);
					}
					strcat(event_msg, "\nReceived MIC: ");
					for (k = 0; k < LEN_KEY_DESC_MIC; k++) {
						sprintf(mic_hex, "%02x ", rcvd_mic[k]);
						strcat(event_msg, mic_hex);
					}
					strcat(event_msg, "\n");
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", event_msg));
					os_free_mem(event_msg);
				}
			}
#endif
			hex_dump("Received MIC", rcvd_mic, LEN_KEY_DESC_MIC);
			hex_dump("Desired  MIC", mic, LEN_KEY_DESC_MIC);
			goto LabelErr;
		}
	}

	/* 1. Decrypt the Key Data field if GTK is included.*/
	/* 2. Extract the context of the Key Data field if it exist.	 */
	/* The field in pairwise_msg_2_WPA1(WPA2) & pairwise_msg_3_WPA1 is clear.*/
	/* The field in group_msg_1_WPA1(WPA2) & pairwise_msg_3_WPA2 is encrypted.*/
	if (CONV_ARRARY_TO_UINT16(key_data_len_ptr) > 0) {
		UCHAR GroupKeyIndex = 0;

		/* Decrypt this field */
		if ((MsgType == EAPOL_PAIR_MSG_3 && bWPA2) || (MsgType == EAPOL_GROUP_MSG_1)) {
			if ((EapolKeyInfo.KeyDescVer == KEY_DESC_EXT)
				|| (EapolKeyInfo.KeyDescVer == KEY_DESC_AES)
				|| (EapolKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED)) {
				UINT aes_unwrap_len = 0;
				UINT8 kck_len = LEN_PTK_KCK;
				UINT8 kek_len = LEN_PTK_KEK;

				if (IS_AKM_SHA384(pSecConfig->AKMMap)
							|| (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
					kck_len = LEN_PTK_KCK_SHA384;
					kek_len = LEN_PTK_KEK_SHA384;
				}

				/* AES */
				AES_Key_Unwrap(key_data_ptr,
							   CONV_ARRARY_TO_UINT16(key_data_len_ptr),
							   &pSecConfig->PTK[kck_len], kek_len,
							   KEYDATA, &aes_unwrap_len);
				SET_UINT16_TO_ARRARY(key_data_len_ptr, aes_unwrap_len);
			} else {
				TKIP_GTK_KEY_UNWRAP(&pSecConfig->PTK[LEN_PTK_KCK],
									pMsg->KeyDesc.KeyIv,
									key_data_ptr,
									CONV_ARRARY_TO_UINT16(key_data_len_ptr),
									KEYDATA);
			}

			if (!bWPA2 && (MsgType == EAPOL_GROUP_MSG_1))
				GroupKeyIndex = EapolKeyInfo.KeyIndex;
		} else if ((MsgType == EAPOL_PAIR_MSG_2) || (MsgType == EAPOL_PAIR_MSG_3 && !bWPA2))
			NdisMoveMemory(KEYDATA, key_data_ptr, CONV_ARRARY_TO_UINT16(key_data_len_ptr));
		else { /* it may have PMKID to check in msg1 */
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("The Key Data Length should be zero !!!\n"));
			goto LabelOK;
		}

		/* Parse Key Data field to */
		/* 1. verify RSN IE for pairwise_msg_2_WPA1(WPA2) ,pairwise_msg_3_WPA1(WPA2)*/
		/* 2. verify KDE format for pairwise_msg_3_WPA2, group_msg_1_WPA2*/
		/* 3. update shared key for pairwise_msg_3_WPA2, group_msg_1_WPA1(WPA2)*/
		if (!WPAParseEapolKeyData(pAd, KEYDATA,
								  CONV_ARRARY_TO_UINT16(key_data_len_ptr),
								  GroupKeyIndex, MsgType, bWPA2, pEntry))
			goto LabelErr;
	}

LabelOK:

	if (KEYDATA != NULL)
		os_free_mem(KEYDATA);

	return TRUE;
LabelErr:

	if (KEYDATA != NULL)
		os_free_mem(KEYDATA);

	return FALSE;
}


VOID WPABuildPairMsg1(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET	pEapolFrame;
	STA_TR_ENTRY *tr_entry;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : The interface is closed...\n", __func__));
		return;
	}

	if ((!pEntry) || IS_ENTRY_NONE(pEntry)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : The entry doesn't exist.\n", __func__));
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BSS_STRUCT *pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;

		if (pEntry->wdev != &pMbss->wdev) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : cannot get binding wdev(%p).\n", __func__, pEntry->wdev));
			return;
		}
	}

#endif /* CONFIG_AP_SUPPORT */
	pHandshake4Way = &pSecConfig->Handshake;

	/* Check the status*/
	if ((pHandshake4Way->WpaState > AS_PTKSTART) || (pHandshake4Way->WpaState < AS_INITPMK)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[ERROR]%s : Not expect calling\n", __func__));
		return;
	}

#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)pEntry->wdev->func_dev;

		if (MAC_ADDR_EQUAL(pEntry->Addr, pMbss->wdev.WscControl.EntryAddr) &&
			pMbss->wdev.WscControl.EapMsgRunning) {
			pHandshake4Way->WpaState = AS_NOTUSE;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("This is a WSC-Enrollee. Not expect calling WPAStart4WayHS here\n"));
			return;
		}
	}
#endif /* WSC_AP_SUPPORT */
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pHandshake4Way->ReplayCounter);
	/* Randomly generate ANonce */
	GenRandom(pAd, (UCHAR *)pHandshake4Way->AAddr, pHandshake4Way->ANonce);
	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 1*/
	/* EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1) */
	pHandshake4Way->MsgType = EAPOL_PAIR_MSG_1;
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_1,
						 pSecConfig, /* Pairwise */
						 &pEntry->wdev->SecConfig, /* Group */
						 pEapolFrame);
#ifdef CONFIG_AP_SUPPORT

	/* If PMKID match in WPA2-enterprise mode, fill PMKID into Key data field and update PMK here	*/
	if ((IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) || IS_AKM_OWE(pSecConfig->AKMMap))
		&& (is_pmkid_cache_in_sec_config(pSecConfig))) {
		UINT8 mic_len = LEN_KEY_DESC_MIC;
		UINT8 *key_data_len_ptr = NULL;
		UINT8 *key_data_ptr = NULL;
		UINT8 pmk_len = LEN_PMK;

		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s      Add PMKID\n", __func__));

		if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
			mic_len = LEN_KEY_DESC_MIC_SHA384;
			pmk_len = LEN_PMK_SHA384;
		}

		hex_dump("PMKID", pSecConfig->pmkid, LEN_PMKID);
		hex_dump("PMK", pSecConfig->pmk_cache, pmk_len);

		key_data_len_ptr = pEapolFrame->KeyDesc.KeyMicAndData + mic_len;
		key_data_ptr = key_data_len_ptr + 2;
		/* Fill in value for KDE */
		key_data_ptr[0] = 0xDD;
		key_data_ptr[2] = 0x00;
		key_data_ptr[3] = 0x0F;
		key_data_ptr[4] = 0xAC;
		key_data_ptr[5] = 0x04;
		NdisMoveMemory(key_data_ptr + 6, pSecConfig->pmkid, LEN_PMKID);
		NdisMoveMemory(&pSecConfig->PMK, pSecConfig->pmk_cache, pmk_len);
		key_data_ptr[1] = 0x14;/* 4+LEN_PMKID*/
		INC_UINT16_TO_ARRARY(key_data_len_ptr, 6 + LEN_PMKID);
		INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}

#ifdef DOT11W_PMF_SUPPORT
	else if ((IS_AKM_WPA3PSK(pSecConfig->AKMMap) ||
		  IS_AKM_OWE(pSecConfig->AKMMap)) &&
		  (pSecConfig->PmfCfg.UsePMFConnect == TRUE)) {
		UCHAR digest[80], PMK_key[20];
		UINT8 mic_len = LEN_KEY_DESC_MIC;
		UINT8 *key_data_len_ptr = NULL;
		UINT8 *key_data_ptr = NULL;
		UINT8 pmk_len = LEN_PMK;

		if (IS_AKM_SHA384(pSecConfig->AKMMap) || pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
			mic_len = LEN_KEY_DESC_MIC_SHA384;
			pmk_len = LEN_PMK_SHA384;
		}

		key_data_len_ptr = pEapolFrame->KeyDesc.KeyMicAndData + mic_len;
		key_data_ptr = key_data_len_ptr + 2;

		key_data_ptr[0] = 0xDD;
		key_data_ptr[2] = 0x00;
		key_data_ptr[3] = 0x0F;
		key_data_ptr[4] = 0xAC;
		key_data_ptr[5] = 0x04;

		if (is_pmkid_cache_in_sec_config(pSecConfig) == FALSE) {
			NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
			NdisMoveMemory(&PMK_key[8], pHandshake4Way->AAddr, MAC_ADDR_LEN);
			NdisMoveMemory(&PMK_key[14], pHandshake4Way->SAddr, MAC_ADDR_LEN);

#ifdef DOT11_SAE_SUPPORT
			if (IS_AKM_SAE(pSecConfig->AKMMap)) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE, ("%s:[SAE]pmkid not found\n", __func__));
				if (!sae_get_pmk_cache(&pAd->SaeCfg, pHandshake4Way->AAddr, pHandshake4Way->SAddr, digest, NULL)) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, ("%s: derive pmkid fail\n", __func__));
					return;
				}
			}
#endif
#ifdef CONFIG_OWE_SUPPORT
			if (IS_AKM_OWE(pSecConfig->AKMMap)) {
				OWE_INFO *owe = &pSecConfig->owe;

				NdisMoveMemory(key_data_ptr + 6, owe->pmkid, LEN_PMKID);
			} else
#endif /*CONFIG_OWE_SUPPORT*/
				NdisMoveMemory(key_data_ptr + 6, digest, LEN_PMKID);
		} else {
			NdisMoveMemory(key_data_ptr + 6, pSecConfig->pmkid, LEN_PMKID);
			NdisMoveMemory(&pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s: PMKID found for WPA2PSK/WPA3PSK\n", __func__));
		}

		key_data_ptr[1] = 0x14;/* 4+LEN_PMKID */
		INC_UINT16_TO_ARRARY(key_data_len_ptr, 6 + LEN_PMKID);
		INC_UINT16_TO_ARRARY(pEapolFrame->Body_Len, 6 + LEN_PMKID);
	}

#endif /* DOT11W_PMF_SUPPORT */
#endif
	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	RTMPToWirelessSta(pAd, pEntry, Header802_3,
					  LENGTH_802_3, (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4,
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);
	/* Trigger Retry Timer*/
	RTMPModTimer(&pHandshake4Way->MsgRetryTimer, PEER_MSG1_RETRY_EXEC_INTV);
	/* Update State*/
	pHandshake4Way->WpaState = AS_PTKSTART;
	pHandshake4Way->GTKState = REKEY_NEGOTIATING;
	os_free_mem(mpool);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s: send Msg1 of 4-way\n", __func__));
}




VOID WPABuildPairMsg2(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET	pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	BOOLEAN is_unencrypted = TRUE;
	STA_TR_ENTRY *tr_entry;

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 2*/
	/*  EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,DataKD_M2)*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_2,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);
	/* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	if (pSecConfig->is_eapol_encrypted &&
	    tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		is_unencrypted = FALSE;

	RTMPToWirelessSta(pAd, pEntry,
			  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
			  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, is_unencrypted);
	os_free_mem(mpool);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s: send Msg2 of 4-way\n", __func__));
}


VOID WPABuildPairMsg3(
	IN PRTMP_ADAPTER    pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	STA_TR_ENTRY *tr_entry;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	ASIC_SEC_INFO Info = {0};
#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	UCHAR HSClientGTK[32];
	/* UCHAR *gtk_ptr = NULL; */
#endif
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));
	/* Allocate memory for input*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pHandshake4Way->ReplayCounter);

	/* Get Group TxTsc form Asic*/
	if (pEntry->wdev)
		AsicGetTxTsc(pAd, pEntry->wdev, pHandshake4Way->RSC);

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)

	if (pAd->ApCfg.MBSSID[pEntry->wdev->func_idx].HotSpotCtrl.HotSpotEnable
		&& pAd->ApCfg.MBSSID[pEntry->wdev->func_idx].HotSpotCtrl.DGAFDisable) {
		/* Radom GTK for hotspot sation client */
		GenRandom(pAd, pEntry->Addr, HSClientGTK);
		/* gtk_ptr = HSClientGTK; */
		os_move_mem(pSecConfig->HsUniGTK, HSClientGTK, LEN_MAX_GTK);
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:Random unique GTK for each mobile device when dgaf disable\n", __func__));
		hex_dump("GTK", pSecConfig->HsUniGTK, 32);
	}

#endif
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_AES_TK);

	else if (IS_AKM_SHA384(pSecConfig->AKMMap) || (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_TK_SHA384);
	else
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
	WPAInstallKey(pAd, &Info, TRUE, TRUE);
	/* Construct EAPoL message - Pairwise Msg 3*/
	pHandshake4Way->MsgType = EAPOL_PAIR_MSG_3;
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_3,
						 pSecConfig, /* Pairwise */
						 &pEntry->wdev->SecConfig, /* Group */
						 pEapolFrame);
	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	RTMPToWirelessSta(pAd, pEntry, Header802_3, LENGTH_802_3,
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4,
					  (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ? FALSE : TRUE);
	RTMPSetTimer(&pHandshake4Way->MsgRetryTimer, PEER_MSG3_RETRY_EXEC_INTV);
	os_free_mem(mpool);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s: send Msg3 of 4-way\n", __func__));
}


VOID WPABuildPairMsg4(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	STA_TR_ENTRY *tr_entry;
	PEAPOL_PACKET	pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	ASIC_SEC_INFO Info = {0};
	BOOLEAN is_unencrypted = TRUE;
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	PAPCLI_STRUCT pApCliEntry;
#endif

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Pairwise Msg 4*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_PAIR_MSG_4,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);
	/* Update WpaState*/
	pSecConfig->Handshake.WpaState = AS_PTKINITDONE;
	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = pEntry->wcid;
	Info.BssIndex = pEntry->func_tb_idx;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_AES_TK);

	else if (IS_AKM_SHA384(pSecConfig->AKMMap))
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384], LEN_TK_SHA384);
	else
		os_move_mem(Info.Key.Key, &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
	/* open 802.1x port control and privacy filter*/
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;

	/* move port secured before EAPOL msg sent out to prevent drop on LMAC AC reset, related to FAST_EAPOL_WAR */
	WifiSysUpdatePortSecur(pAd, pEntry, &Info);

	/* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);
	if (pSecConfig->is_eapol_encrypted &&
	    tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		is_unencrypted = FALSE;

	RTMPToWirelessSta(pAd, pEntry,
			  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
			  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, is_unencrypted);

	if (pHandshake4Way->AllowInsPTK == TRUE) {
		WPAInstallKey(pAd, &Info, FALSE, TRUE);
		pHandshake4Way->AllowInsPTK = FALSE;
		pEntry->AllowUpdateRSC = TRUE;
	} else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("!!!%s : the M3 reinstall attack, skip install key\n", __func__));
	}

	os_free_mem(mpool);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (IS_ENTRY_APCLI(pEntry)) {
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			UCHAR	ifIdx = 0;

			if (pEntry->wdev)
				ifIdx = pEntry->wdev->func_idx;

			if (pAd->ApCfg.ApCliAutoConnectRunning[ifIdx] == TRUE) {
				pAd->ApCfg.ApCliAutoConnectRunning[ifIdx] = FALSE;
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Apcli auto connected:WPABuildPairMsg4(),pAd->ApCfg.ApCliAutoConnectRunning[%d]=%d\n",
						 ifIdx, pAd->ApCfg.ApCliAutoConnectRunning[ifIdx]));
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT*/

#ifdef APCLI_SUPPORT
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	pApCliEntry = &pAd->ApCfg.ApCliTab[tr_entry->func_tb_idx];
	if (IS_CIPHER_AES_Entry(pEntry) &&
		(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))/*For security AES case*/
		wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT)
			if (pEntry && (pEntry->func_tb_idx < MAX_APCLI_NUM) && tr_entry) {
				if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
#ifdef CONFIG_MAP_SUPPORT
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							("APCLIENT MAP_ENABLE\n"));
#ifdef A4_CONN
					map_a4_peer_enable(pAd, pEntry, FALSE);
#endif
					map_send_bh_sta_wps_done_event(pAd, pEntry, FALSE);
#endif
				}
			}
#endif /* defined(MWDS) || defined(CONFIG_MAP_SUPPORT) */
#endif /* APCLI_SUPPORT */
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	/* move port secured before EAPOL msg sent out to prevent drop on LMAC AC reset, related to FAST_EAPOL_WAR */
	/* WifiSysUpdatePortSecur(pAd, pEntry, &Info); */
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("===> WifiSysUpdatePortSecur called by (%s), wcid=%d, PortSecured=%d\n",
			  __func__, pEntry->wcid, STATE_PORT_SECURE));



#ifdef APCLI_SUPPORT
#ifdef APCLI_OWE_SUPPORT
	if ((IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) && (IS_AKM_OWE(pSecConfig->AKMMap))) {
		OWE_INFO *owe = &pSecConfig->owe;
		UINT pmk_len = LEN_PMK;
		UCHAR intf_idx = pEntry->func_tb_idx;
		UCHAR CliIdx = 0xff;

		if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)
			pmk_len = LEN_PMK_SHA384;


#ifdef MAC_REPEATER_SUPPORT
		if (pEntry->bReptCli == TRUE) {
			intf_idx = pEntry->MatchReptCliIdx;
			CliIdx = pEntry->MatchReptCliIdx;
		}
#endif

		if (owe->pmkid) {
			apcli_add_pmkid_cache(pAd, pEntry->Addr, owe->pmkid, pSecConfig->PMK, pmk_len, intf_idx, CliIdx);
			MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("Store PMKID for MAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pEntry->Addr)));
			hex_dump("PMK cache ID", owe->pmkid, LEN_PMKID);
			hex_dump("PMK key", pSecConfig->PMK, pmk_len);
		}

	}
#endif
#endif


#ifdef DOT11_SAE_SUPPORT
	if (IS_AKM_WPA3PSK(pSecConfig->AKMMap))
		set_sae_instance_removable(&pAd->SaeCfg, pSecConfig->Handshake.SAddr, pSecConfig->Handshake.AAddr);
#endif

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s: send Msg4 of 4-way\n", __func__));
}


VOID WPABuildGroupMsg1(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));

	if (pEntry->wdev == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : pEntry->wdev is NULL!!!\n", __func__));
		return;
	}

	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Increment replay counter by 1*/
	ADD_ONE_To_64BIT_VAR(pSecConfig->Handshake.ReplayCounter);

	/* Get Group TxTsc form Asic*/
	AsicGetTxTsc(pAd, pEntry->wdev, pHandshake4Way->RSC);

	/* Construct EAPoL message - Group Msg 1*/
	pHandshake4Way->MsgType = EAPOL_GROUP_MSG_1;
	WPAConstructEapolMsg(pEntry,
						 EAPOL_GROUP_MSG_1,
						 pSecConfig, /* Pairwise */
						 &pEntry->wdev->SecConfig, /* Group */
						 pEapolFrame);
	/* Make outgoing frame: Authenticator send to Supplicant */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->SAddr, pHandshake4Way->AAddr, EAPOL);
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, LENGTH_802_3,
					  (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);
	os_free_mem(mpool);

	/* Trigger Retry Timer*/
	/* When group retry counter > limit, extend rekey time interval */
	if (pHandshake4Way->MsgRetryCounter > GROUP_MSG1_RETRY_LIMIT)
		RTMPModTimer(&pHandshake4Way->MsgRetryTimer, GROUP_MSG1_RETRY_EXEC_EXTEND);
	else
		RTMPModTimer(&pHandshake4Way->MsgRetryTimer, GROUP_MSG1_RETRY_EXEC_INTV);

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s : send out Group Message 1\n", __func__));
}


VOID WPABuildGroupMsg2(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR Header802_3[14];
	UCHAR *mpool;
	PEAPOL_PACKET pEapolFrame;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));
	/* Allocate memory for output*/
	os_alloc_mem(NULL, (PUCHAR *)&mpool, TX_EAPOL_BUFFER);

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s : no memory!!!\n", __func__));
		return;
	}

	pHandshake4Way = &pSecConfig->Handshake;
	pEapolFrame = (PEAPOL_PACKET)mpool;
	NdisZeroMemory(pEapolFrame, TX_EAPOL_BUFFER);
	/* Construct EAPoL message - Group Msg 2*/
	WPAConstructEapolMsg(pEntry,
						 EAPOL_GROUP_MSG_2,
						 pSecConfig, /* Pairwise */
						 pSecConfig, /* Group */
						 pEapolFrame);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
#ifdef A4_CONN
		if (pEntry && IS_ENTRY_APCLI(pEntry) && (pEntry->func_tb_idx < MAX_APCLI_NUM)) {
			STA_TR_ENTRY *tr_entry = NULL;

			if (VALID_TR_WCID(pEntry->wcid))
				tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

			if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
				PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
				/*For security TKIP case*/
				wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
#endif /*WAPP_SUPPORT*/
#ifdef CONFIG_MAP_SUPPORT
				map_a4_peer_enable(pAd, pEntry, FALSE);
#endif
			}
		}
#endif /* A4_CONN */
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Make outgoing frame: Supplicant send to Authenticator */
	MAKE_802_3_HEADER(Header802_3, pHandshake4Way->AAddr, pHandshake4Way->SAddr, EAPOL);
	RTMPToWirelessSta(pAd, pEntry,
					  Header802_3, sizeof(Header802_3), (PUCHAR)pEapolFrame,
					  CONV_ARRARY_TO_UINT16(pEapolFrame->Body_Len) + 4, FALSE);
	os_free_mem(mpool);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s : send out Group Message 2\n", __func__));
}

VOID PeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR PTK[120];
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UINT8 len_ptk = LEN_AES_PTK;
#ifdef A4_CONN
	PHEADER_802_11 pHeader;

#endif

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));
#ifdef A4_CONN
	pHeader = (PHEADER_802_11)Elem->Msg;
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pHandshake4Way = &pSecConfig->Handshake;
	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_1, pSecConfig, pEntry) == FALSE)
		return;

	/* Store Replay counter, it will use to verify message 3 and construct message 2*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
	/* Store ANonce*/
	NdisMoveMemory(pHandshake4Way->ANonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);
	/* Generate random SNonce*/
	GenRandom(pAd, (UCHAR *)pHandshake4Way->SAddr, pHandshake4Way->SNonce);
	pHandshake4Way->AllowInsPTK = TRUE;
	pEntry->AllowUpdateRSC = FALSE;
	pEntry->SecConfig.LastGroupKeyId = 0;
	NdisZeroMemory(pEntry->SecConfig.LastGTK, LEN_MAX_GTK);

	if (is_pmkid_cache_in_sec_config(pSecConfig)) {
#ifdef CONFIG_OWE_SUPPORT
#ifdef APCLI_OWE_SUPPORT
		if (IS_AKM_OWE(pSecConfig->AKMMap)
			&& (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256)) {
			if (pSecConfig->owe.last_try_group == pSecConfig->owe.curr_group)
				NdisMoveMemory(pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
			else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("===> %s:PMK Cache not valid as owe group changed from %d to %d\n",
													__func__, pSecConfig->owe.last_try_group, pSecConfig->owe.curr_group));
			 return;
			}
		} else if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
			if (pSecConfig->owe.last_try_group == pSecConfig->owe.curr_group)
				NdisMoveMemory(pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK_SHA384);
			else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("===> %s:PMK Cache not valid as owe group changed from %d to %d\n",
													__func__, pSecConfig->owe.last_try_group, pSecConfig->owe.curr_group));
			 return;
			}
		} else
#endif
#endif
		{
			NdisMoveMemory(pSecConfig->PMK, pSecConfig->pmk_cache, LEN_PMK);
		}



	}
#ifdef CONFIG_OWE_SUPPORT
#ifdef APCLI_OWE_SUPPORT
	else if (pEntry->need_process_ecdh_ie == TRUE) {
		process_ecdh_element(pAd,
					pEntry,
					(EXT_ECDH_PARAMETER_IE *)&pEntry->ecdh_ie,
					pEntry->ecdh_ie.length,
					SUBTYPE_ASSOC_RSP, FALSE);
	}
#endif
#endif

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		len_ptk = LEN_OWE_PTK_SHA384;
	else if (IS_AKM_SHA384(pSecConfig->AKMMap))
		len_ptk = LEN_PTK_SHA384;

	if (IS_AKM_OWE(pSecConfig->AKMMap)) {
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  len_ptk);
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
		hex_dump("PTK", PTK, len_ptk);
	} else if (IS_AKM_SHA384(pSecConfig->AKMMap)
		|| pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
		WpaDerivePTK_KDF_384(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  len_ptk);
		NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
		hex_dump("PTK SHA384", PTK, len_ptk);
	} else if (IS_AKM_SAE(pSecConfig->AKMMap)) {
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					 pHandshake4Way->ANonce,		/* ANONCE*/
					 pHandshake4Way->AAddr,
					 pHandshake4Way->SNonce,		/* SNONCE*/
					 pHandshake4Way->SAddr,
					 PTK,
					 LEN_AES_PTK);
		NdisMoveMemory(pSecConfig->PTK, PTK, LEN_AES_PTK);
	} else
#ifdef DOT11W_PMF_SUPPORT
	if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) {
		WpaDerivePTK_KDF_256(pSecConfig->PMK,
					  pHandshake4Way->ANonce,		/* ANONCE*/
					  pHandshake4Way->AAddr,
					  pHandshake4Way->SNonce,		/* SNONCE*/
					  pHandshake4Way->SAddr,
					  PTK,
					  LEN_AES_PTK);   /* Must is 48 bytes */
		NdisMoveMemory(pSecConfig->PTK, PTK, LEN_AES_PTK);
		hex_dump("PTK", PTK, LEN_AES_PTK);
	} else
#endif /* DOT11W_PMF_SUPPORT */
	{
		/* Derive PTK*/
		WpaDerivePTK(pSecConfig->PMK,
					 pHandshake4Way->ANonce,		/* ANONCE*/
					 pHandshake4Way->AAddr,
					 pHandshake4Way->SNonce,		/* SNONCE*/
					 pHandshake4Way->SAddr,
					 PTK,
					 LEN_PTK);
		NdisMoveMemory(pSecConfig->PTK, PTK, LEN_PTK);
	}

	/* Update WpaState*/
	pSecConfig->Handshake.WpaState = AS_PTKINIT_NEGOTIATING;
	WPABuildPairMsg2(pAd, pSecConfig, pEntry);
}


VOID PeerPairMsg2Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR PTK[120];
	BOOLEAN Cancelled;
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UINT8 len_ptk = LEN_AES_PTK;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));

	pHandshake4Way = &pSecConfig->Handshake;
	/* check Entry in valid State*/
	if (pHandshake4Way->WpaState < AS_PTKSTART)
		return;
	/* Prevent the Replayed Msg2 Attack */
	if (pHandshake4Way->WpaState == AS_PTKINITDONE) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: reject the Replayed Msg2\n", __func__));
		return;
	}

	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;
	/* Store SNonce*/
	NdisMoveMemory(pHandshake4Way->SNonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE);

	if (IS_AKM_OWE(pSecConfig->AKMMap) && (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384))
		len_ptk = LEN_OWE_PTK_SHA384;
	else if (IS_AKM_SHA384(pSecConfig->AKMMap))
		len_ptk = LEN_PTK_SHA384;

#ifdef DOT11R_FT_SUPPORT

	if (IS_FT_RSN_STA(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			UINT8 ptk_len;
			UCHAR apidx = 0;
			struct wifi_dev *wdev;
			PFT_CFG pFtCfg;

			if (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: func_tb_idx=%d, return here.\n", __func__, pEntry->func_tb_idx));
				return;
			} else
				apidx = pEntry->func_tb_idx;

			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pFtCfg = &wdev->FtCfg;
			FT_DerivePMKR0(pSecConfig->PMK, LEN_PMK,
						   (PUINT8)pAd->ApCfg.MBSSID[apidx].Ssid,
						   pAd->ApCfg.MBSSID[apidx].SsidLen,
						   pFtCfg->FtMdId,
						   pFtCfg->FtR0khId,
						   pFtCfg->FtR0khIdLen,
						   pEntry->Addr,
						   pEntry->FT_PMK_R0,
						   pEntry->FT_PMK_R0_NAME);
			FT_DerivePMKR1(pEntry->FT_PMK_R0,
						   pEntry->FT_PMK_R0_NAME,
						   wdev->bssid, /* R1KHID*/
						   pEntry->Addr,					/* S1KHID*/
						   pEntry->FT_PMK_R1,
						   pEntry->FT_PMK_R1_NAME);

			if (IS_CIPHER_TKIP_Entry(pEntry))
				ptk_len = 32 + 32;
			else
				ptk_len = 32 + 16;

			FT_DerivePTK(pEntry->FT_PMK_R1,
						 pEntry->FT_PMK_R1_NAME,
						 pHandshake4Way->ANonce,
						 pHandshake4Way->SNonce,
						 wdev->bssid, /* Bssid*/
						 pEntry->Addr,								/* sta mac*/
						 ptk_len,
						 pEntry->FT_PTK,
						 pEntry->PTK_NAME);
			NdisCopyMemory(pSecConfig->PTK, pEntry->FT_PTK, LEN_MAX_PTK);
		}
#endif /* CONFIG_AP_SUPPORT */
	} else
#endif /* DOT11R_FT_SUPPORT */
		if (IS_AKM_OWE(pSecConfig->AKMMap)) {
			WpaDerivePTK_KDF_256(pSecConfig->PMK,
				pHandshake4Way->ANonce,		/* ANONCE*/
				pHandshake4Way->AAddr,
				pHandshake4Way->SNonce,		/* SNONCE*/
				pHandshake4Way->SAddr,
				PTK,
				len_ptk);
			NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
			hex_dump("OWE PTK", PTK, len_ptk);
		} else if (IS_AKM_SHA384(pSecConfig->AKMMap)
			|| pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384) {
			WpaDerivePTK_KDF_384(pSecConfig->PMK,
						  pHandshake4Way->ANonce,		/* ANONCE*/
						  pHandshake4Way->AAddr,
						  pHandshake4Way->SNonce,		/* SNONCE*/
						  pHandshake4Way->SAddr,
						  PTK,
						  len_ptk);
			NdisMoveMemory(pSecConfig->PTK, PTK, len_ptk);
			hex_dump("PTK SHA384", PTK, len_ptk);
		} else
		if (IS_AKM_SAE(pSecConfig->AKMMap)) {
			WpaDerivePTK_KDF_256(pSecConfig->PMK,
						  pHandshake4Way->ANonce,		/* ANONCE*/
						  pHandshake4Way->AAddr,
						  pHandshake4Way->SNonce,		/* SNONCE*/
						  pHandshake4Way->SAddr,
						  PTK,
						  LEN_AES_PTK);   /* Must is 48 bytes */
			NdisMoveMemory(pSecConfig->PTK, PTK, LEN_AES_PTK);
			hex_dump("PTK", PTK, LEN_AES_PTK);
		} else
#ifdef DOT11W_PMF_SUPPORT
		if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) {
			WpaDerivePTK_KDF_256(pSecConfig->PMK,
						  pHandshake4Way->ANonce,		/* ANONCE*/
						  pHandshake4Way->AAddr,
						  pHandshake4Way->SNonce,		/* SNONCE*/
						  pHandshake4Way->SAddr,
						  PTK,
						  LEN_AES_PTK);   /* Must is 48 bytes */
			NdisMoveMemory(pSecConfig->PTK, PTK, LEN_AES_PTK);
			hex_dump("PTK", PTK, LEN_AES_PTK);
		} else
#endif /* DOT11W_PMF_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("got msg2 derivePTK\n"));
			WpaDerivePTK_KDF_256(pSecConfig->PMK,
						  pHandshake4Way->ANonce,		/* ANONCE*/
						  pHandshake4Way->AAddr,
						  pHandshake4Way->SNonce,		/* SNONCE*/
						  pHandshake4Way->SAddr,
						  PTK,
						  LEN_AES_PTK);   /* Must is 48 bytes */
			NdisMoveMemory(pSecConfig->PTK, PTK, LEN_AES_PTK);
			hex_dump("PTK", PTK, LEN_AES_PTK);
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			/* Derive PTK*/
			WpaDerivePTK(pSecConfig->PMK,
						 pHandshake4Way->ANonce,		/* ANONCE*/
						 pHandshake4Way->AAddr,
						 pHandshake4Way->SNonce,		/* SNONCE*/
						 pHandshake4Way->SAddr,
						 PTK,
						 LEN_PTK);
			NdisMoveMemory(pSecConfig->PTK, PTK, LEN_PTK);
		}

	/* Sanity Check peer Pairwise message 2 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_2, pSecConfig, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG2_FAIL);
#endif
		return;
	}

	/* delete retry timer*/
	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	pHandshake4Way->MsgRetryCounter = 0;
	/* Change state*/
	pHandshake4Way->WpaState = AS_PTKINIT_NEGOTIATING;
	WPABuildPairMsg3(pAd, pSecConfig, pEntry);
}


VOID PeerPairMsg3Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT MsgLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UCHAR idx = 0;
	BOOLEAN bWPA2 = TRUE;

	/* Choose WPA2 or not*/
	if (IS_AKM_WPA1(pSecConfig->AKMMap) || IS_AKM_WPA1PSK(pSecConfig->AKMMap))
		bWPA2 = FALSE;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));

	pHandshake4Way = &pSecConfig->Handshake;
	/* Record 802.11 header & the received EAPOL packet Msg3*/
	pHeader = (PHEADER_802_11) Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	pReceiveEapol = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	MsgLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 3 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, MsgLen, EAPOL_PAIR_MSG_3, pSecConfig, pEntry) == FALSE)
		return;

	if ((pHandshake4Way->AllowInsPTK == TRUE) && bWPA2) {
		UCHAR kid = pEntry->SecConfig.LastGroupKeyId;
		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC-2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pReceiveEapol->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		pEntry->AllowUpdateRSC = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): update CCMP_BC_PN to %llu\n",
			__FUNCTION__, pEntry->wcid, pEntry->CCMP_BC_PN[kid]));
#ifdef MAC_REPEATER_SUPPORT
		/* sync PN of ApCli entry as the time rept rekey */
		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			MAC_TABLE_ENTRY *pEntry2 = MacTableLookup(pAd, pHeader->Addr2);

			if (pEntry2 != NULL) {
				pEntry2->CCMP_BC_PN[kid] = pEntry->CCMP_BC_PN[kid];
				pEntry2->Init_CCMP_BC_PN_Passed[kid] = pEntry->Init_CCMP_BC_PN_Passed[kid];
				pEntry2->AllowUpdateRSC = pEntry->AllowUpdateRSC;
			}
		}
#endif
	}

	/* Save Replay counter, it will use construct message 4*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);

	/* Double check ANonce*/
	if (!NdisEqualMemory(pHandshake4Way->ANonce, pReceiveEapol->KeyDesc.KeyNonce, LEN_KEY_DESC_NONCE))
		return;

	pSecConfig->is_eapol_encrypted = pHeader->FC.Wep;
	WPABuildPairMsg4(pAd, pSecConfig, pEntry);
}


VOID PeerPairMsg4Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapolM4;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
	unsigned char hdr_len = LENGTH_802_11;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));

	pHandshake4Way = &pSecConfig->Handshake;

	if (pHandshake4Way->WpaState < AS_PTKINIT_NEGOTIATING)
		return;

	/* Prevent the Replayed Msg4 Attack */
	if (pHandshake4Way->WpaState == AS_PTKINITDONE) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s: reject the Replayed Msg4\n", __func__));
		return;
	}

	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	if (Elem->MsgLen < (hdr_len + LENGTH_802_1_H + LENGTH_EAPOL_H + MIN_LEN_OF_EAPOL_KEY_MSG))
		return;

	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapolM4 = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer Pairwise message 4 - Replay Counter, MIC*/
	if (WpaMessageSanity(pAd, pReceiveEapolM4, EapolLen, EAPOL_PAIR_MSG_4, pSecConfig, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_4WAY_HS_MSG4_FAIL);
#endif
		return;
	}
	/* Sanity Check pEntry->func_tb_idx to avoid out of bound with pAd->ApCfg.MBSSID*/
	if (pEntry->func_tb_idx >= HW_BEACON_MAX_NUM)
		return;

	/* 4. upgrade state */
	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
	pHandshake4Way->WpaState = AS_PTKINITDONE;
	pHandshake4Way->GTKState = REKEY_ESTABLISHED;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#if defined(DOT1X_SUPPORT) && defined(RADIUS_ACCOUNTING_SUPPORT)

	/* Notify 802.1x daemon to add this sta for accounting*/
	if (IS_AKM_WPA1PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA3PSK(pSecConfig->AKMMap))
		DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_LOCAL_AUTH_ENTRY);

#endif /* DOT1X_SUPPORT && RADIUS_ACCOUNTING_SUPPORT */
	WifiSysUpdatePortSecur(pAd, pEntry, NULL);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("===> WifiSysUpdatePortSecur called by (%s), wcid=%d, PortSecured=%d\n",
			  __func__, pEntry->wcid, STATE_PORT_SECURE));
#ifdef WSC_AP_SUPPORT

	if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE)
		WscInformFromWPA(pEntry);

#endif /* WSC_AP_SUPPORT */

	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)
		|| IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)
		|| IS_AKM_WPA3PSK(pSecConfig->AKMMap)
		|| IS_AKM_OWE(pSecConfig->AKMMap)) {
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT

		if (IS_FT_RSN_STA(pEntry)) {
			PFT_R1HK_ENTRY pR1khEntry;
			PUINT8 pUCipher = NULL;
			PUINT8 pAkm = NULL;
			UINT8 count;

			pUCipher = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, PAIRWISE_SUITE, &count);
			pAkm = WPA_ExtractSuiteFromRSNIE(pEntry->RSN_IE, pEntry->RSNIE_Len, AKM_SUITE, &count);
			/* Record the PMK-R0 related information */
			RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
							  pEntry->func_tb_idx,
							  pEntry->Addr,
							  pEntry->FT_PMK_R0_NAME,
							  pEntry->FT_PMK_R0,
							  LEN_PMK);
			/* Delete previous entry */
			pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);

			if (pR1khEntry != NULL)
				FT_R1khEntryDelete(pAd, pR1khEntry);

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
							   (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKCachePeriod / OS_HZ),
							   FT_REASSOC_DEADLINE, /* YF_TIE */
							   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.FtCfg.FtR0khId,
							   pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.FtCfg.FtR0khIdLen,
							   pEntry->Addr);
#ifdef IAPP_SUPPORT
			{
				FT_KDP_EVT_ASSOC EvtAssoc;

				EvtAssoc.SeqNum = 0;
				NdisMoveMemory(EvtAssoc.MacAddr, pEntry->Addr, MAC_ADDR_LEN);

				{
					FT_KDP_EVENT_INFORM(pAd,
										pEntry->func_tb_idx,
										FT_KDP_SIG_FT_ASSOCIATION,
										&EvtAssoc,
										sizeof(EvtAssoc),
										NULL);
				}
	    }
#endif /* IAPP_SUPPORT				*/
			pR1khEntry = FT_R1khEntryTabLookup(pAd, pEntry->FT_PMK_R1_NAME);

			if (pR1khEntry != NULL) {
				pR1khEntry->AKMMap = pSecConfig->AKMMap;
				hex_dump("R1KHTab-R0KHID", pR1khEntry->R0khId, pR1khEntry->R0khIdLen);
				hex_dump("R1KHTab-PairwiseCipher", pR1khEntry->PairwisChipher, 4);
				hex_dump("R1KHTab-AKM", pR1khEntry->AkmSuite, 4);
				hex_dump("R1KHTab-PMKR0Name", pR1khEntry->PmkR0Name, 16);
				hex_dump("R1KHTab-PMKR1Name", pR1khEntry->PmkR1Name, 16);
			} else
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("The entry in R1KH table doesn't exist\n"));
		} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT1X_SUPPORT
			if (IS_AKM_WPA2(pSecConfig->AKMMap) || IS_AKM_WPA3_192BIT(pSecConfig->AKMMap)) {
				UCHAR  PMK_key[20];
				UCHAR  digest[80];
				UINT8 pmk_len = LEN_PMK;
				/* Calculate PMKID, refer to IEEE 802.11i-2004 8.5.1.2*/
				NdisMoveMemory(&PMK_key[0], "PMK Name", 8);
				NdisMoveMemory(&PMK_key[8], pSecConfig->Handshake.AAddr, MAC_ADDR_LEN);
				NdisMoveMemory(&PMK_key[14], pSecConfig->Handshake.SAddr, MAC_ADDR_LEN);
				if (IS_AKM_SHA384(pSecConfig->AKMMap) ||
				   (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)) {
					pmk_len = LEN_PMK_SHA384;
					RT_HMAC_SHA384(pSecConfig->PMK, pmk_len, PMK_key, 20, digest, LEN_PMKID);
				} else
					RT_HMAC_SHA1(pSecConfig->PMK, pmk_len, PMK_key, 20, digest, SHA1_DIGEST_SIZE);
				RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
						  pEntry->func_tb_idx,
						  pSecConfig->Handshake.SAddr,
						  digest,
						  pSecConfig->PMK,
						  pmk_len);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n", digest[0], digest[1], digest[2], digest[3], digest[4], digest[5]));
			}
#endif /* DOT1X_SUPPORT */
#ifdef DOT11_SAE_SUPPORT
			else if (IS_AKM_WPA3PSK(pSecConfig->AKMMap))
				set_sae_instance_removable(&pAd->SaeCfg, pSecConfig->Handshake.AAddr, pSecConfig->Handshake.SAddr);
#endif
#ifdef CONFIG_OWE_SUPPORT
			else if (IS_AKM_OWE(pSecConfig->AKMMap)) {
				INT CacheIdx;/* Key cache */

				if (pEntry->SecConfig.owe.pmkid) {
					UINT8 pmk_len = LEN_PMK;

					if (pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA384)
						pmk_len = LEN_PMK_SHA384;
					CacheIdx = RTMPValidatePMKIDCache(&pAd->ApCfg.PMKIDCache,
									pEntry->func_tb_idx,
									pEntry->Addr,
									pEntry->SecConfig.owe.pmkid);

					hex_dump("store pmkid:", pEntry->SecConfig.owe.pmkid, LEN_PMKID);
					RTMPAddPMKIDCache(&pAd->ApCfg.PMKIDCache,
							pEntry->func_tb_idx,
							pEntry->Addr,
							pEntry->SecConfig.owe.pmkid,
							pEntry->SecConfig.PMK,
							pmk_len);
					store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx);
				}
			}
#endif /*CONFIG_OWE_SUPPORT*/
#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
		if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
#if defined(CONFIG_MAP_SUPPORT)
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MAP_ENABLE\n"));
#if defined(A4_CONN)
			map_a4_peer_enable(pAd, pEntry, TRUE);
#endif
			map_send_bh_sta_wps_done_event(pAd, pEntry, TRUE);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		}
#endif
#endif /* CONFIG_AP_SUPPORT */


		/* send wireless event - for set key done WPA2*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry->IsWNMReqValid == TRUE) {
			struct wnm_req_data *req_data = pEntry->ReqData;

			Send_WNM_Notify_Req(pAd,
								req_data->peer_mac_addr,
								req_data->wnm_req,
								req_data->wnm_req_len,
								req_data->type);
			pEntry->IsWNMReqValid = FALSE;
			os_free_mem(req_data);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!!!!msg 4 send wnm req\n"));
		}

#endif
#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
		if (pEntry->IsBTMReqValid == TRUE) {
			struct btm_req_data *req_data = pEntry->ReqbtmData;

			Send_BTM_Req(pAd,
						 req_data->peer_mac_addr,
						 req_data->btm_req,
						 req_data->btm_req_len);
			pEntry->IsBTMReqValid = FALSE;
			os_free_mem(req_data);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!!!!msg 4 send btm req\n"));
		}

#endif

#ifdef MBO_SUPPORT
		/* update STA bssid & security info to daemon */
		MboIndicateStaBssidInfo(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry->Addr);
#endif /* MBO_SUPPORT */

		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("AP SETKEYS DONE - AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s, wcid=%d from %02X:%02X:%02X:%02X:%02X:%02X\n\n",
				  GetAuthModeStr(pSecConfig->AKMMap),
				  GetEncryModeStr(pSecConfig->PairwiseCipher),
				  GetEncryModeStr(pSecConfig->GroupCipher),
				  pEntry->wcid,
				  PRINT_MAC(pEntry->Addr)));

		if ((!IS_AKM_OPEN(GET_SEC_AKM(pSecConfig))) && tr_entry->PortSecured == 1) {
			INT32 i;

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PTK:"));

			for (i = 0; i < 64; i++)
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", pSecConfig->PTK[i]));

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		}
	} else {
		/* 5. init Group 2-way handshake if necessary.*/
		RTMPSetTimer(&pEntry->SecConfig.StartFor2WayTimer, ENQUEUE_EAPOL_2WAY_START_TIMER);
	}
}

VOID PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapol;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	UCHAR idx = 0;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));
	pHandshake4Way = &pSecConfig->Handshake;
	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapol = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;

	/* Sanity Check peer group message 1 - Replay Counter, MIC, RSNIE*/
	if (WpaMessageSanity(pAd, pReceiveEapol, EapolLen, EAPOL_GROUP_MSG_1, pSecConfig, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG1_FAIL);
#endif
		return;
	}

	if (pEntry->AllowUpdateRSC == TRUE) {
		UCHAR kid = pEntry->SecConfig.LastGroupKeyId;
		pEntry->CCMP_BC_PN[kid] = 0;
		for (idx = 0; idx < (LEN_KEY_DESC_RSC-2); idx++)
			pEntry->CCMP_BC_PN[kid] += ((UINT64)pReceiveEapol->KeyDesc.KeyRsc[idx] << (idx*8));
		pEntry->Init_CCMP_BC_PN_Passed[kid] = FALSE;
		pEntry->AllowUpdateRSC = FALSE;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): update CCMP_BC_PN to %llu\n",
			__FUNCTION__, pEntry->wcid, pEntry->CCMP_BC_PN[kid]));
#ifdef MAC_REPEATER_SUPPORT
		/* Sync PN of ApCli entry as the time rept rekey */
		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			MAC_TABLE_ENTRY *pEntry2 = MacTableLookup(pAd, pHeader->Addr2);

			if (pEntry2 != NULL) {
				pEntry2->CCMP_BC_PN[kid] = pEntry->CCMP_BC_PN[kid];
				pEntry2->Init_CCMP_BC_PN_Passed[kid] = pEntry->Init_CCMP_BC_PN_Passed[kid];
				pEntry2->AllowUpdateRSC = pEntry->AllowUpdateRSC;
			}
		}
#endif
	}

	/* Save Replay counter, it will use to construct message 2*/
	NdisMoveMemory(pHandshake4Way->ReplayCounter, pReceiveEapol->KeyDesc.ReplayCounter, LEN_KEY_DESC_REPLAY);
	WPABuildGroupMsg2(pAd, pSecConfig, pEntry);
}

VOID PeerGroupMsg2Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem)
{
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pReceiveEapolM2;
	UINT EapolLen;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
	BOOLEAN Cancelled;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s\n", __func__));
	/* pointer to 802.11 header*/
	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
		if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
			hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif
	/* skip 802.11_header(24-byte) and LLC_header(8) */
	pReceiveEapolM2 = (PEAPOL_PACKET) &Elem->Msg[hdr_len + LENGTH_802_1_H];
	EapolLen = Elem->MsgLen - hdr_len - LENGTH_802_1_H;
	pHandshake4Way = &pSecConfig->Handshake;

	if (pHandshake4Way->WpaState != AS_PTKINITDONE)
		return;

	/* Sanity Check peer Group message 2 - Replay Counter, MIC*/
	if (WpaMessageSanity(pAd, pReceiveEapolM2, EapolLen, EAPOL_GROUP_MSG_2, pSecConfig, pEntry) == FALSE) {
#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_AUTH_FAIL, REASON_2WAY_HS_MSG2_FAIL);
#endif
		return;
	}

	RTMPCancelTimer(&pHandshake4Way->MsgRetryTimer, &Cancelled);
	pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;

#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
	{
		STA_TR_ENTRY *tr_entry = NULL;

		if (VALID_TR_WCID(pEntry->wcid))
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
			map_a4_peer_enable(pAd, pEntry, TRUE);
#endif /* CONFIG_MAP_SUPPORT */

#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		}
	}
#endif


	if (IS_AKM_WPA2(pSecConfig->AKMMap)
		|| IS_AKM_WPA2PSK(pSecConfig->AKMMap)) {
		/* send wireless event - for set key done WPA2*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA2_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
	} else {
		/* send wireless event - for set key done WPA*/
		RTMPSendWirelessEvent(pAd, IW_SET_KEY_DONE_WPA1_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP SETKEYS DONE - AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s from %02X:%02X:%02X:%02X:%02X:%02X\n\n",
			 GetAuthModeStr(pSecConfig->AKMMap),
			 GetEncryModeStr(pSecConfig->PairwiseCipher),
			 GetEncryModeStr(pSecConfig->GroupCipher),
			 PRINT_MAC(pEntry->Addr)));
}


static VOID WpaEAPOLStartAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry  = NULL;
	STA_TR_ENTRY *tr_entry  = NULL;
	PSECURITY_CONFIG pSecConfig  = NULL;
	PHEADER_802_11 pHeader;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WpaEAPOLStartAction ===>\n"));
	pHeader = (PHEADER_802_11)Elem->Msg;

	/*For normaol PSK, we enqueue an EAPOL-Start command to trigger the process.*/
	if (Elem->MsgLen == MAC_ADDR_LEN) {
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, Elem->Msg);
#endif
	} else {
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader->Addr2);
#endif
#ifdef WSC_AP_SUPPORT

		/*
		    a WSC enabled AP must ignore EAPOL-Start frames received from clients that associated to
		    the AP with an RSN IE or SSN IE indicating a WPA2-PSK/WPA-PSK authentication method in
		    the assication request.  <<from page52 in Wi-Fi Simple Config Specification version 1.0g>>
		*/
		if (pEntry &&
			(pEntry->func_tb_idx == MAIN_MBSSID) &&
			(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscConfMode != WSC_DISABLE) &&
			(IS_AKM_PSK_Entry(pEntry)) && pEntry->bWscCapable) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WPS enabled AP: Ignore EAPOL-Start frames received from clients.\n"));
			return;
		}

#endif /* WSC_AP_SUPPORT */
	}

	/*TODO: find the root cause.*/
	if (pEntry == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s, cannot find entry:%02x:%02x:%02x:%02x:%02x:%02x\n",
				__func__, PRINT_MAC(Elem->Msg)));
		return;
	}

	pSecConfig = &pEntry->SecConfig;

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" PortSecured(%d), WpaState(%d), AKM(0x%x), is_cache(%d)\n",
			 tr_entry->PortSecured, pSecConfig->Handshake.WpaState, pSecConfig->AKMMap, is_pmkid_cache_in_sec_config(pSecConfig)));

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		&& (pSecConfig->Handshake.WpaState < AS_PTKSTART)
		&& (IS_AKM_PSK(pSecConfig->AKMMap)
			|| (IS_AKM_WPA2(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
			|| (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig)))
			|| (IS_AKM_OWE(pSecConfig->AKMMap) && (is_pmkid_cache_in_sec_config(pSecConfig))))) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		NdisZeroMemory(pSecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);
		pSecConfig->Handshake.MsgRetryCounter = 0;
		WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);
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
static VOID WpaEAPOLKeyAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	PHEADER_802_11 pHeader;
	PEAPOL_PACKET pEapol_packet;
	KEY_INFO peerKeyInfo;
	UINT eapol_len;
	PSECURITY_CONFIG pSecConfig  = NULL;
	PHANDSHAKE_PROFILE pHandshake4Way  = NULL;
	unsigned char hdr_len = LENGTH_802_11;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	pHeader = (PHEADER_802_11)Elem->Msg;
#ifdef A4_CONN
	if (pHeader->FC.FrDs == 1 && pHeader->FC.ToDs == 1)
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	pEapol_packet = (PEAPOL_PACKET)&Elem->Msg[hdr_len + LENGTH_802_1_H];
	eapol_len = CONV_ARRARY_TO_UINT16(pEapol_packet->Body_Len) + LENGTH_EAPOL_H;
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:Receive EAPoL-Key frame from %02X-%02X-%02X-%02X-%02X-%02X\n", __func__, PRINT_MAC(pHeader->Addr2)));

	if (eapol_len > Elem->MsgLen - hdr_len - LENGTH_802_1_H) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("The length of EAPoL packet is invalid from %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pHeader->Addr2)));
		return;
	}

	NdisMoveMemory((PUCHAR)&peerKeyInfo, (PUCHAR)&pEapol_packet->KeyDesc.KeyInfo, sizeof(KEY_INFO));
	*((USHORT *)&peerKeyInfo) = cpu2le16(*((USHORT *)&peerKeyInfo));

	if (((pEapol_packet->ProVer != EAPOL_VER) && (pEapol_packet->ProVer != EAPOL_VER2)) ||
		((pEapol_packet->KeyDesc.Type != WPA1_KEY_DESC) && (pEapol_packet->KeyDesc.Type != WPA2_KEY_DESC) && (pEapol_packet->KeyDesc.Type != KEY_DESC_EXT))) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Key descripter does not match with WPA rule from %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pHeader->Addr2)));
		return;
	}

#ifdef MAC_REPEATER_SUPPORT

	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ifIndex - REPT_MLME_START_IDX;
		pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
	}

#endif /* MAC_REPEATER_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF) {
		if (pReptEntry->MacTabWCID >= MAX_LEN_OF_MAC_TABLE) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: MacTabWCID = %d >= MAX_LEN_OF_MAC_TABLE(128)", __func__, pReptEntry->MacTabWCID));
			return;
		}

		pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];

		if (!IS_ENTRY_REPEATER(pEntry)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s:pEntry is not repeater ,wcid = %d, A2=%02X-%02X-%02X-%02X-%02X-%02X\n",
					  __func__, pEntry->wcid, PRINT_MAC(pHeader->Addr2)));
			return;
		}

		pEntry->SecConfig.STARec_Bssid = pReptEntry->wdev->bss_info_argument.ucBssIndex;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
#ifdef CONFIG_AP_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader->Addr2);
#endif

		if (!pEntry) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MacTableLookup FAILE with A2 %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pHeader->Addr2)));
			return;
		}

		pEntry->SecConfig.STARec_Bssid = pEntry->wdev->bss_info_argument.ucBssIndex;
	}

	if (IS_ENTRY_NONE(pEntry)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:pEntry is none ,wcid = %d, A2=%02X-%02X-%02X-%02X-%02X-%02X\n",
				  __func__, pEntry->wcid, PRINT_MAC(pHeader->Addr2)));
		return;
	}

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	pSecConfig = &pEntry->SecConfig;
	pHandshake4Way = &pSecConfig->Handshake;

	/* The value 1 shall be used for all EAPOL-Key frames to and from a STA when */
	/* neither the group nor pairwise ciphers are CCMP for Key Descriptor 1.*/
	if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher) && (peerKeyInfo.KeyDescVer != KEY_DESC_TKIP)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Key descripter version not match(TKIP) from %02X-%02X-%02X-%02X-%02X-%02X\n", PRINT_MAC(pHeader->Addr2)));
		return;
	}

#ifdef DOT11R_FT_SUPPORT
	/* The value 3 shall be used for all EAPOL-Key frames to and from a STA when the negotiated */
	/* AKM is 00-0F-AC:3 or 00-0F-AC:4. It is a FT STA.	*/
	else if (IS_FT_RSN_STA(pEntry) && (peerKeyInfo.KeyDescVer != KEY_DESC_EXT)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[FT] Key descripter version(%d) not match(FT)\n", peerKeyInfo.KeyDescVer));
		return;
	}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	else if ((pSecConfig->key_deri_alg == SEC_KEY_DERI_SHA256) &&
		!(peerKeyInfo.KeyDescVer == KEY_DESC_EXT || peerKeyInfo.KeyDescVer == KEY_DESC_NOT_DEFINED))
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF] Key descripter version not match(AES-128/NOT_DEFINED)\n"));

#endif  /* DOT11W_PMF_SUPPORT */
	/* The value 2 shall be used for all EAPOL-Key frames to and from a STA when */
	/* either the pairwise or the group cipher is AES-CCMP for Key Descriptor 2 or 3.*/
	else if ((peerKeyInfo.KeyDescVer == KEY_DESC_AES)
			 && (!(IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
				   || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)))) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Key descripter version not match peerKeyInfo.KeyDescVer=%d, PairwiseCipher=0x%x  from %02X-%02X-%02X-%02X-%02X-%02X\n"
				 , peerKeyInfo.KeyDescVer, pSecConfig->PairwiseCipher, PRINT_MAC(pHeader->Addr2)));
		return;
	}

	/* Check if this STA is in class 3 state and the WPA state is started						*/
	if ((pEntry->Sst == SST_ASSOC) && (pSecConfig->Handshake.WpaState >= AS_INITPSK)) {
		/* Check the Key Ack (bit 7) of the Key Information to determine the Authenticator or not.*/
		/* An EAPOL-Key frame that is sent by the Supplicant in response to an EAPOL-*/
		/* Key frame from the Authenticator must not have the Ack bit set.*/
		if (peerKeyInfo.KeyAck == 1) {
			/* The frame is snet by Authenticator. So the Supplicant side shall handle this.*/
			/*
				IOT with 3COM 3CRWE454G72, Mode : WL-525 : According to 802.11i spec, actually we don't need to check the peerKeyInfo.Secure == 0 form peer RootAP
				We found the AP will set peerKeyInfo.Secure to 1 in EAPOL-MSG1 after we reconnect the AP again, then driver drop the MSG1, DUT can't connect success anymore.

			*/
			if ((peerKeyInfo.Request == 0)
#ifndef FAST_EAPOL_WAR
				&& (peerKeyInfo.Secure == 0)
#endif /* !FAST_EAPOL_WAR */
				&& (peerKeyInfo.Error == 0)
				&& (peerKeyInfo.KeyType == PAIRWISEKEY)) {

				/* Process
				    1. the message 1 of 4-way HS in WPA or WPA2
					EAPOL-Key(0,0,1,0,P,0,0,ANonce,0,DataKD_M1)
				    2. the message 3 of 4-way HS in WPA
					EAPOL-Key(0,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
				 */
				if (peerKeyInfo.KeyMic == 0) {
					PeerPairMsg1Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else
					PeerPairMsg3Action(pAd, pEntry, &pEntry->SecConfig, Elem);
			} else if ((peerKeyInfo.Secure == 1) && (peerKeyInfo.KeyMic == 1)
					   && (peerKeyInfo.Request == 0) && (peerKeyInfo.Error == 0)) {
				/* Process
				    1. the message 3 of 4-way HS in WPA2
					EAPOL-Key(1,1,1,1,P,0,KeyRSC,ANonce,MIC,DataKD_M3)
				    2. the message 1 of group KS in WPA or WPA2
					EAPOL-Key(1,1,1,0,G,0,Key RSC,0, MIC,GTK[N])
				*/
				if (peerKeyInfo.KeyType == PAIRWISEKEY)
					PeerPairMsg3Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				else
					PeerGroupMsg1Action(pAd, pEntry, &pEntry->SecConfig, Elem);
			}
		} else {
			/* The frame is snet by Supplicant.So the Authenticator side shall handle this. */
#ifdef CONFIG_AP_SUPPORT
			if ((peerKeyInfo.KeyMic == 1) && (peerKeyInfo.Request == 1)
				&& (peerKeyInfo.Error == 1)) {
				/* The Supplicant uses a single Michael MIC Failure Report frame */
				/* to report a MIC failure event to the Authenticator. */
				/* A Michael MIC Failure Report is an EAPOL-Key frame with */
				/* the following Key Information field bits set to 1: */
				/* MIC bit, Error bit, Request bit, Secure bit.*/
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Received an Michael MIC Failure Report, active countermeasure\n"));
				RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
			} else
#endif /* CONFIG_AP_SUPPORT */
			if ((peerKeyInfo.Request == 0) && (peerKeyInfo.Error == 0)
				&& (peerKeyInfo.KeyMic == 1)) {
				if (peerKeyInfo.Secure == 0 && peerKeyInfo.KeyType == PAIRWISEKEY) {
					/*
					EAPOL-Key(0,1,0,0,P,0,0,SNonce,MIC,Data) Process:
					1. message 2 of 4-way HS in WPA or WPA2
					2. message 4 of 4-way HS in WPA
					*/
					UINT8 mic_len = LEN_KEY_DESC_MIC;
					UINT8 *key_data_len_ptr = NULL;

					if (IS_AKM_SHA384(pEntry->SecConfig.AKMMap))
						mic_len = LEN_KEY_DESC_MIC_SHA384;

					key_data_len_ptr = pEapol_packet->KeyDesc.KeyMicAndData + mic_len;

					if (CONV_ARRARY_TO_UINT16(key_data_len_ptr) == 0)
						PeerPairMsg4Action(pAd, pEntry, &pEntry->SecConfig, Elem);
					else
						PeerPairMsg2Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == PAIRWISEKEY) {
					/* EAPOL-Key(1,1,0,0,P,0,0,0,MIC,0) */
					/* Process message 4 of 4-way HS in WPA2*/
					PeerPairMsg4Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				} else if (peerKeyInfo.Secure == 1 && peerKeyInfo.KeyType == GROUPKEY) {
					/* EAPOL-Key(1,1,0,0,G,0,0,0,MIC,0)*/
					/* Process message 2 of Group key HS in WPA or WPA2 */
					PeerGroupMsg2Action(pAd, pEntry, &pEntry->SecConfig, Elem);
				}
			}
		}
	}
}


VOID WPAStartFor4WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
	struct _SECURITY_CONFIG *pSecConfig  = NULL;
	PRTMP_ADAPTER pAd = NULL;

	if (!pEntry)
		return;

	pAd = (PRTMP_ADAPTER) pEntry->pAd;
	pSecConfig = &pEntry->SecConfig;

	if (pSecConfig->Handshake.WpaState >= AS_PTKSTART)
		return;

	if (IS_AKM_PSK(pSecConfig->AKMMap)
		|| (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_PSK /*For PMKIDCache */)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Enqueue EAPoL-Start-PSK for sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			MlmeEnqueue(pAd, WPA_STATE_MACHINE, MT2_EAPOLStart, 6, &pEntry->Addr, 0);
		}
#endif
		RTMP_MLME_HANDLER(pAd);
	}

#ifdef DOT1X_SUPPORT
	else if  (IS_AKM_1X(pSecConfig->AKMMap)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Enqueue EAPoL-Start-1X for sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));
		DOT1X_EapTriggerAction(pAd, pEntry);
	}

#endif /* DOT1X_SUPPORT */
	pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
}


VOID WPAStartFor2WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
	struct _SECURITY_CONFIG *pSecConfig  = NULL;
	PRTMP_ADAPTER pAd = NULL;
	STA_TR_ENTRY *tr_entry;

	if (!pEntry)
		return;

	pAd = (PRTMP_ADAPTER) pEntry->pAd;
	pSecConfig = &pEntry->SecConfig;
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

	if (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
		return;

	if (pSecConfig->Handshake.GTKState == REKEY_NEGOTIATING)
		return;

	pSecConfig->Handshake.MsgRetryCounter = 0;
	pSecConfig->Handshake.GTKState = REKEY_NEGOTIATING;
	WPABuildGroupMsg1(pAd, pSecConfig, pEntry);
}

VOID WPAHandshakeMsgRetryExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)FunctionContext;

	if ((pEntry) && IS_ENTRY_CLIENT(pEntry)) {
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;
		struct _SECURITY_CONFIG *pSecConfig  = &pEntry->SecConfig;
		PHANDSHAKE_PROFILE pHandshake = &pSecConfig->Handshake;

		pHandshake->MsgRetryCounter++;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s---> WpaState=%d,  MsgType=%d\n", __func__, pHandshake->WpaState, pHandshake->MsgType));

		if (IS_AKM_PSK(pSecConfig->AKMMap) || IS_AKM_1X(pSecConfig->AKMMap)) {
			if (pHandshake->MsgType == EAPOL_PAIR_MSG_1) {
#ifdef CONFIG_MAP_SUPPORT
				/* Retry counter limitation */
				/* MAP Certification[MT7621 + 7615D] Data Passing Test TC 4.10.4_ETH_FH24G*/
				/* Broadcom Agent it taking more than 3 seconds in sending Reply for EAP Message 1.*/
				/* Increase the counter to retry EAP message 1 from 2 to  4 times.*/
				if (IS_MAP_ENABLE(pAd)) {
					if (pHandshake->MsgRetryCounter > PEER_MSG1_RETRY_LIMIT + 2) {
						pHandshake->WpaState = AS_INITIALIZE;
						pHandshake->GTKState = REKEY_FAILURE;
						/* send wireless event - for pairwise key handshaking timeout */
						RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::4Way-MSG1 timeout with %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, PRINT_MAC(pHandshake->SAddr)));
						MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
					} else {
						WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::ReTry MSG1 of 4-way Handshake, Counter=%d\n", __func__, pHandshake->MsgRetryCounter));
					}
				} else
#endif
					if (pHandshake->MsgRetryCounter > PEER_MSG1_RETRY_LIMIT) {
						pHandshake->WpaState = AS_INITIALIZE;
						pHandshake->GTKState = REKEY_FAILURE;
						/* send wireless event - for pairwise key handshaking timeout */
						RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::4Way-MSG1 timeout with %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, PRINT_MAC(pHandshake->SAddr)));
						MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
					} else {
						WPABuildPairMsg1(pAd, &pEntry->SecConfig, pEntry);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::ReTry MSG1 of 4-way Handshake, Counter=%d\n", __func__, pHandshake->MsgRetryCounter));
					}
			} else if (pHandshake->MsgType == EAPOL_PAIR_MSG_3) {
				if (pHandshake->MsgRetryCounter > PEER_MSG3_RETRY_LIMIT) {
					pHandshake->WpaState = AS_INITIALIZE;
					pHandshake->GTKState = REKEY_FAILURE;
					/* send wireless event - for pairwise key handshaking timeout */
					RTMPSendWirelessEvent(pAd, IW_PAIRWISE_HS_TIMEOUT_EVENT_FLAG, pEntry->Addr, pEntry->wdev->wdev_idx, 0);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::4Way-MSG3 timeout with %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, PRINT_MAC(pHandshake->SAddr)));
					MlmeDeAuthAction(pAd, pEntry, REASON_4_WAY_TIMEOUT, FALSE);
				} else {
					WPABuildPairMsg3(pAd, &pEntry->SecConfig, pEntry);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::ReTry MSG3 of 4-way Handshake, Counter = %d\n", __func__, pHandshake->MsgRetryCounter));
				}
			} else if (pHandshake->MsgType == EAPOL_GROUP_MSG_1) {
				if (pHandshake->MsgRetryCounter > GROUP_MSG1_RETRY_LIMIT) {
					pHandshake->GTKState = REKEY_FAILURE;
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Group rekey timeout from %02X:%02X:%02X:%02X:%02X:%02X\n", __func__, PRINT_MAC(pHandshake->SAddr)));
				} else {
					WPABuildGroupMsg1(pAd, &pEntry->SecConfig, pEntry);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::ReTry MSG1 of 2-way Handshake, Counter = %d\n", __func__, pHandshake->MsgRetryCounter));
				}
			}
		}
	}

}

