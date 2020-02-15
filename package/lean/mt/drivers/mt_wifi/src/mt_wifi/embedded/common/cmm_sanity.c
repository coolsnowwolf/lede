/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sanity.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang  2004-09-01      add WMM support
*/
#include "rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

extern UCHAR	CISCO_OUI[];

extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
extern UCHAR	BROADCOM_OUI[];
extern UCHAR	MARVELL_OUI[];
extern UCHAR	ATHEROS_OUI[];



typedef struct wsc_ie_probreq_data {
	UCHAR	ssid[32];
	UCHAR	macAddr[6];
	UCHAR	data[2];
} WSC_IE_PROBREQ_DATA;

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAddBAReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2)
{
	PMLME_ADDBA_REQ_STRUCT   pInfo;

	pInfo = (MLME_ADDBA_REQ_STRUCT *)Msg;

	if ((MsgLen != sizeof(MLME_ADDBA_REQ_STRUCT))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeAddBAReqSanity fail - message lenght not correct.\n"));
		return FALSE;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pInfo->Wcid)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeAddBAReqSanity fail - The peer Mac is not associated yet.\n"));
		return FALSE;
	}

	/*
	if ((pInfo->BaBufSize > MAX_RX_REORDERBUF) || (pInfo->BaBufSize < 2))
	{
	    MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeAddBAReqSanity fail - Rx Reordering buffer too big or too small\n"));
	    return FALSE;
	}
	*/

	if ((pInfo->pAddr[0] & 0x01) == 0x01) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeAddBAReqSanity fail - broadcast address not support BA\n"));
		return FALSE;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeDelBAReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen)
{
	MLME_DELBA_REQ_STRUCT *pInfo;

	pInfo = (MLME_DELBA_REQ_STRUCT *)Msg;

	if (pInfo->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	if ((MsgLen != sizeof(MLME_DELBA_REQ_STRUCT))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeDelBAReqSanity fail - message lenght not correct.\n"));
		return FALSE;
	}

	if (!VALID_UCAST_ENTRY_WCID(pAd, pInfo->Wcid)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeDelBAReqSanity fail - The peer Mac is not associated yet.\n"));
		return FALSE;
	}

	if ((pInfo->TID & 0xf0)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeDelBAReqSanity fail - The peer TID is incorrect.\n"));
		return FALSE;
	}

	if (NdisEqualMemory(pAd->MacTab.Content[pInfo->Wcid].Addr, pInfo->Addr, MAC_ADDR_LEN) == 0) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeDelBAReqSanity fail - the peer addr dosen't exist.\n"));
		return FALSE;
	}

	return TRUE;
}


BOOLEAN PeerAddBAReqActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PFRAME_ADDBA_REQ pAddFrame;

	pAddFrame = (PFRAME_ADDBA_REQ)(pMsg);

	if (MsgLen < (sizeof(FRAME_ADDBA_REQ))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("PeerAddBAReqActionSanity: ADDBA Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}

	/* we support immediate BA.*/
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);
	pAddFrame->BaStartSeq.word = cpu2le16(pAddFrame->BaStartSeq.word);
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("PeerAddBAReqActionSanity: ADDBA Request Ba Policy[%d] not support\n", pAddFrame->BaParm.BAPolicy));
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ADDBA Request. tid=%x, Bufsize=%x, AMSDUSupported=%x\n", pAddFrame->BaParm.TID, pAddFrame->BaParm.BufSize, pAddFrame->BaParm.AMSDUSupported));
		return FALSE;
	}

	return TRUE;
}

BOOLEAN PeerAddBARspActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * pMsg,
	IN ULONG MsgLen)
{
	PFRAME_ADDBA_RSP pAddFrame;

	pAddFrame = (PFRAME_ADDBA_RSP)(pMsg);

	if (MsgLen < (sizeof(FRAME_ADDBA_RSP))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): ADDBA Resp frame length incorrect(len=%ld)\n", __func__, MsgLen));
		return FALSE;
	}

	/* we support immediate BA.*/
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM		tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&pAddFrame->BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&pAddFrame->BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&pAddFrame->BaParm) = cpu2le16(*(USHORT *)(&pAddFrame->BaParm));
#endif
	pAddFrame->StatusCode = cpu2le16(pAddFrame->StatusCode);
	pAddFrame->TimeOutValue = cpu2le16(pAddFrame->TimeOutValue);

	if (pAddFrame->BaParm.BAPolicy != IMMED_BA) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): ADDBA Resp Ba Policy[%d] not support\n", __func__, pAddFrame->BaParm.BAPolicy));
		return FALSE;
	}

	return TRUE;
}

BOOLEAN PeerDelBAActionSanity(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN VOID * pMsg,
	IN ULONG MsgLen)
{
	PFRAME_DELBA_REQ  pDelFrame;

	if (MsgLen != (sizeof(FRAME_DELBA_REQ)))
		return FALSE;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return FALSE;

	pDelFrame = (PFRAME_DELBA_REQ)(pMsg);
	*(USHORT *)(&pDelFrame->DelbaParm) = cpu2le16(*(USHORT *)(&pDelFrame->DelbaParm));
	pDelFrame->ReasonCode = cpu2le16(pDelFrame->ReasonCode);
	return TRUE;
}


/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerBeaconAndProbeRspSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	IN UCHAR  MsgChannel,
	OUT BCN_IE_LIST * ie_list,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pVIE,
	IN BOOLEAN bGetDtim,
	IN BOOLEAN bFromBeaconReport)
{
	UCHAR *Ptr;
	PFRAME_802_11 pFrame;
	PEID_STRUCT pEid;
	UCHAR SubType = SUBTYPE_ASSOC_REQ;
	UCHAR Sanity;
	ULONG Length = 0;
	UCHAR *pPeerWscIe = NULL;
	INT PeerWscIeLen = 0;
	BOOLEAN bWscCheck = TRUE;
	UCHAR LatchRfChannel = 0;
	UCHAR *ptr_eid = NULL;
	/*
		For some 11a AP which didn't have DS_IE, we use two conditions to decide the channel
		1. If the AP is 11n enabled, then check the control channel.
		2. If the AP didn't have any info about channel, use the channel we received this
			frame as the channel. (May inaccuracy!!)
	*/
	UCHAR CtrlChannel = 0;
	os_alloc_mem(NULL, &pPeerWscIe, 512);
	Sanity = 0;		/* Add for 3 necessary EID field check*/
	ie_list->AironetCellPowerLimit = 0xFF;  /* Default of AironetCellPowerLimit is 0xFF*/
	ie_list->NewExtChannelOffset = 0xff;	/*Default 0xff means no such IE*/
	*LengthVIE = 0; /* Set the length of VIE to init value 0*/

	if (bFromBeaconReport == FALSE) {
		pFrame = (PFRAME_802_11)Msg;
		/* get subtype from header*/
		SubType = (UCHAR)pFrame->Hdr.FC.SubType;
		/* get Addr2 and BSSID from header*/
		COPY_MAC_ADDR(&ie_list->Addr2[0], pFrame->Hdr.Addr2);
		COPY_MAC_ADDR(&ie_list->Bssid[0], pFrame->Hdr.Addr3);
		Ptr = pFrame->Octet;
		Length += LENGTH_802_11;
	} else {
		/* beacon report response's body have no 802.11 header part! */
		SubType = 255; /* beacon report can't get SubType init 255 */
		Ptr = (UINT8 *)Msg;
		pFrame = NULL; /* init. */
	}

	/* get timestamp from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->TimeStamp, Ptr, TIMESTAMP_LEN);
	ie_list->TimeStamp.u.LowPart = cpu2le32(ie_list->TimeStamp.u.LowPart);
	ie_list->TimeStamp.u.HighPart = cpu2le32(ie_list->TimeStamp.u.HighPart);
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;
	/* get beacon interval from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->BeaconPeriod, Ptr, 2);
	Ptr += 2;
	Length += 2;
	/* get capability info from payload and advance the pointer*/
	NdisMoveMemory(&ie_list->CapabilityInfo, Ptr, 2);
	Ptr += 2;
	Length += 2;

	if (CAP_IS_ESS_ON(ie_list->CapabilityInfo))
		ie_list->BssType = BSS_INFRA;
	else
		ie_list->BssType = BSS_ADHOC;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer*/
	while ((Length + 2 + pEid->Len) <= MsgLen) {
		/* Secure copy VIE to VarIE[MAX_VIE_LEN] didn't overflow.*/
		if ((*LengthVIE + pEid->Len + 2) >= MAX_VIE_LEN) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - Variable IEs out of resource [len(=%d) > MAX_VIE_LEN(=%d)]\n",
					 __func__, (*LengthVIE + pEid->Len + 2), MAX_VIE_LEN));
			break;
		}

		ptr_eid = (UCHAR *)pEid;

		switch (pEid->Eid) {
		case IE_SSID:

			/* Already has one SSID EID in this beacon, ignore the second one*/
			if (Sanity & 0x1)
				break;

			if (pEid->Len <= MAX_LEN_OF_SSID) {
				NdisMoveMemory(&ie_list->Ssid[0], pEid->Octet, pEid->Len);
				ie_list->SsidLen = pEid->Len;
				Sanity |= 0x1;
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() - wrong IE_SSID (len=%d)\n", __func__, pEid->Len));
				goto SanityCheck;
			}

			break;

		case IE_SUPP_RATES:
			if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES) {
				Sanity |= 0x2;
				NdisMoveMemory(&ie_list->SupRate[0], pEid->Octet, pEid->Len);
				ie_list->SupRateLen = pEid->Len;
				/*
				TODO: 2004-09-14 not a good design here, cause it exclude extra
				rates from ScanTab. We should report as is. And filter out
				unsupported rates in MlmeAux
				*/
				/* Check against the supported rates*/
				/* RTMPCheckRates(pAd, SupRate, pSupRateLen,wdev->PhyMode);*/
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() - wrong IE_SUPP_RATES (len=%d)\n", __func__, pEid->Len));
				goto SanityCheck;
			}

			break;

		case IE_HT_CAP:
			if (pEid->Len >= SIZE_HT_CAP_IE) { /*Note: allow extension.!!*/
				NdisMoveMemory(&ie_list->HtCapability, pEid->Octet, sizeof(HT_CAPABILITY_IE));
				ie_list->HtCapabilityLen = SIZE_HT_CAP_IE;	/* Nnow we only support 26 bytes.*/
				*(USHORT *)(&ie_list->HtCapability.HtCapInfo) = cpu2le16(*(USHORT *)(&ie_list->HtCapability.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&ie_list->HtCapability.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&ie_list->HtCapability.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&ie_list->HtCapability.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&ie_list->HtCapability.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#ifdef RT_BIG_ENDIAN
				*(USHORT *)(&ie_list->HtCapability.TxBFCap) =
						le2cpu32(*(USHORT *)(&ie_list->HtCapability.TxBFCap));
#endif

			} else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - wrong IE_HT_CAP. pEid->Len = %d\n", __func__, pEid->Len));

			break;

		case IE_ADD_HT:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE)) {
				/*
				This IE allows extension, but we can ignore extra bytes beyond our
				knowledge , so only copy first sizeof(ADD_HT_INFO_IE)
				*/
				NdisMoveMemory(&ie_list->AddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				ie_list->AddHtInfoLen = SIZE_ADD_HT_INFO_IE;
				CtrlChannel = ie_list->AddHtInfo.ControlChan;
				*(USHORT *)(&ie_list->AddHtInfo.AddHtInfo2) = cpu2le16(*(USHORT *)(&ie_list->AddHtInfo.AddHtInfo2));
				*(USHORT *)(&ie_list->AddHtInfo.AddHtInfo3) = cpu2le16(*(USHORT *)(&ie_list->AddHtInfo.AddHtInfo3));
			} else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - wrong IE_ADD_HT.\n", __func__));

			break;

		case IE_SECONDARY_CH_OFFSET:
			if (pEid->Len == 1)
				ie_list->NewExtChannelOffset = pEid->Octet[0];
			else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - wrong IE_SECONDARY_CH_OFFSET.\n", __func__));

			break;

		case IE_FH_PARM:
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(IE_FH_PARM)\n", __func__));
			break;

		case IE_DS_PARM:
			if (pEid->Len == 1) {
				ie_list->Channel = *pEid->Octet;
				Sanity |= 0x4;
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() - wrong IE_DS_PARM (len=%d)\n", __func__, pEid->Len));
				goto SanityCheck;
			}

			break;

		case IE_CF_PARM:
			if (pEid->Len == 6) {
				ie_list->CfParm.bValid = TRUE;
				ie_list->CfParm.CfpCount = pEid->Octet[0];
				ie_list->CfParm.CfpPeriod = pEid->Octet[1];
				ie_list->CfParm.CfpMaxDuration = pEid->Octet[2] + 256 * pEid->Octet[3];
				ie_list->CfParm.CfpDurRemaining = pEid->Octet[4] + 256 * pEid->Octet[5];
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() - wrong IE_CF_PARM\n", __func__));

				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);

				return FALSE;
			}

			break;

		case IE_IBSS_PARM:
			if (pEid->Len == 2)
				NdisMoveMemory(&ie_list->AtimWin, pEid->Octet, pEid->Len);
			else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() - wrong IE_IBSS_PARM\n", __func__));

				if (pPeerWscIe)
					os_free_mem(pPeerWscIe);

				return FALSE;
			}

			break;

		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			if (pEid->Len == 3)
				ie_list->NewChannel = pEid->Octet[1];	/*extract new channel number*/

			break;

		/*
		New for WPA
		CCX v2 has the same IE, we need to parse that too
		Wifi WMM use the same IE vale, need to parse that too
		*/
		/* case IE_WPA:*/
		case IE_VENDOR_SPECIFIC:
			if (NdisEqualMemory(pEid->Octet, MARVELL_OUI, 3))
				ie_list->is_marvell_ap = TRUE;

			if (NdisEqualMemory(pEid->Octet, ATHEROS_OUI, 3))
				ie_list->is_atheros_ap = TRUE;


			/* Check the OUI version, filter out non-standard usage*/
			check_vendor_ie(pAd, (UCHAR *)pEid, &(ie_list->vendor_ie));

#ifdef CONFIG_OWE_SUPPORT
			if (NdisEqualMemory(pEid->Octet, OWE_TRANS_OUI, 4)) {
				/* Copy to pVIE which will report to bssid list.*/
				Ptr = (PUCHAR) pVIE;
				NdisMoveMemory(Ptr + *LengthVIE, ptr_eid, pEid->Len + 2);
				*LengthVIE += (pEid->Len + 2);
			} else
#endif /*CONFIG_OWE_SUPPORT*/
			if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) {
				/* Copy to pVIE which will report to bssid list.*/
				Ptr = (PUCHAR) pVIE;
				NdisMoveMemory(Ptr + *LengthVIE, ptr_eid, pEid->Len + 2);
				*LengthVIE += (pEid->Len + 2);
			} else if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24)) {
				PUCHAR ptr;
				int i;
				/* parsing EDCA parameters*/
				ie_list->EdcaParm.bValid          = TRUE;
				ie_list->EdcaParm.bQAck           = FALSE; /* pEid->Octet[0] & 0x10;*/
				ie_list->EdcaParm.bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20;*/
				ie_list->EdcaParm.bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40;*/
				ie_list->EdcaParm.EdcaUpdateCount = pEid->Octet[6] & 0x0f;
				ie_list->EdcaParm.bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
				ptr = &pEid->Octet[8];

				for (i = 0; i < 4; i++) {
					UCHAR aci = (*ptr & 0x60) >> 5; /* b5~6 is AC INDEX*/

					ie_list->EdcaParm.bACM[aci]  = (((*ptr) & 0x10) == 0x10);   /* b5 is ACM*/
					ie_list->EdcaParm.Aifsn[aci] = (*ptr) & 0x0f;               /* b0~3 is AIFSN*/
					ie_list->EdcaParm.Cwmin[aci] = *(ptr + 1) & 0x0f;           /* b0~4 is Cwmin*/
					ie_list->EdcaParm.Cwmax[aci] = *(ptr + 1) >> 4;             /* b5~8 is Cwmax*/
					ie_list->EdcaParm.Txop[aci]  = *(ptr + 2) + 256 * (*(ptr + 3)); /* in unit of 32-us*/
					ptr += 4; /* point to next AC*/
				}
			} else if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7)) {
				/* parsing EDCA parameters*/
				ie_list->EdcaParm.bValid          = TRUE;
				ie_list->EdcaParm.bQAck           = FALSE; /* pEid->Octet[0] & 0x10;*/
				ie_list->EdcaParm.bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20;*/
				ie_list->EdcaParm.bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40;*/
				ie_list->EdcaParm.EdcaUpdateCount = pEid->Octet[6] & 0x0f;
				ie_list->EdcaParm.bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
				/* use default EDCA parameter*/
				ie_list->EdcaParm.bACM[QID_AC_BE]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_BE] = 3;
				ie_list->EdcaParm.Cwmin[QID_AC_BE] = pAd->wmm_cw_min;
				ie_list->EdcaParm.Cwmax[QID_AC_BE] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_BE]  = 0;
				ie_list->EdcaParm.bACM[QID_AC_BK]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_BK] = 7;
				ie_list->EdcaParm.Cwmin[QID_AC_BK] = pAd->wmm_cw_min;
				ie_list->EdcaParm.Cwmax[QID_AC_BK] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_BK]  = 0;
				ie_list->EdcaParm.bACM[QID_AC_VI]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_VI] = 2;
				ie_list->EdcaParm.Cwmin[QID_AC_VI] = pAd->wmm_cw_min - 1;
				ie_list->EdcaParm.Cwmax[QID_AC_VI] = pAd->wmm_cw_max;
				ie_list->EdcaParm.Txop[QID_AC_VI]  = 96;   /* AC_VI: 96*32us ~= 3ms*/
				ie_list->EdcaParm.bACM[QID_AC_VO]  = 0;
				ie_list->EdcaParm.Aifsn[QID_AC_VO] = 2;
				ie_list->EdcaParm.Cwmin[QID_AC_VO] = pAd->wmm_cw_min - 2;
				ie_list->EdcaParm.Cwmax[QID_AC_VO] = pAd->wmm_cw_max - 1;
				ie_list->EdcaParm.Txop[QID_AC_VO]  = 48;   /* AC_VO: 48*32us ~= 1.5ms*/
			} else if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
					  ) {

				/*
					1. WSC 2.0 IE also has 0x104a000110. (WSC 1.0 version)

					2. Some developing devices would broadcast incorrect IE content.
					   To prevent system crashed by those developing devices, we shall check length.

					@20140123
				*/
				if (pPeerWscIe && (pEid->Len > 4)) {
					if ((PeerWscIeLen + (pEid->Len - 4)) <= 512) {
						NdisMoveMemory(pPeerWscIe + PeerWscIeLen, pEid->Octet + 4, pEid->Len - 4);
						PeerWscIeLen += (pEid->Len - 4);
					} else { /* ((PeerWscIeLen +(pEid->Len - 4)) > 512) */
						bWscCheck = FALSE;
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error!!! Sum of All PeerWscIeLen = %d (> 512)\n", __func__, (PeerWscIeLen + (pEid->Len - 4))));
					}
				} else {
					bWscCheck = FALSE;

					if (pEid->Len <= 4)
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Error!!! Incorrect WPS IE!\n", __func__));

					if (pPeerWscIe == NULL)
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Error!!! pPeerWscIe is null!\n", __func__));
				}

			}
			break;

		case IE_EXT_SUPP_RATES:
			if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES) {
				NdisMoveMemory(&ie_list->ExtRate[0], pEid->Octet, pEid->Len);
				ie_list->ExtRateLen = pEid->Len;
				/*
				TODO: 2004-09-14 not a good design here, cause it exclude extra rates
				from ScanTab. We should report as is. And filter out unsupported
				rates in MlmeAux
				*/
				/* Check against the supported rates*/
				/* RTMPCheckRates(pAd, ExtRate, pExtRateLen,wdev->PhyMode);*/
			}

			break;

		case IE_ERP:
			if (pEid->Len == 1)
				ie_list->Erp = (UCHAR)pEid->Octet[0];

			break;

		case IE_AIRONET_CKIP:

			/*
			0. Check Aironet IE length, it must be larger or equal to 28
			Cisco AP350 used length as 28
			Cisco AP12XX used length as 30
			*/
			if (pEid->Len < (CKIP_NEGOTIATION_LENGTH - 2))
				break;

			/* 1. Copy CKIP flag byte to buffer for process*/
			ie_list->CkipFlag = *(pEid->Octet + 8);
			break;

		case IE_AP_TX_POWER:

			/* AP Control of Client Transmit Power*/
			/*0. Check Aironet IE length, it must be 6*/
			if (pEid->Len != 0x06)
				break;

			/* Get cell power limit in dBm*/
			if (NdisEqualMemory(pEid->Octet, CISCO_OUI, 3) == 1)
				ie_list->AironetCellPowerLimit = *(pEid->Octet + 4);

			break;

		/* WPA2 & 802.11i RSN*/
		case IE_RSN:

			/* There is no OUI for version anymore, check the group cipher OUI before copying*/
			if (RTMPEqualMemory(pEid->Octet + 2, RSN_OUI, 3)) {
				/* Copy to pVIE which will report to microsoft bssid list.*/
				Ptr = (PUCHAR) pVIE;
				NdisMoveMemory(Ptr + *LengthVIE, ptr_eid, pEid->Len + 2);
				*LengthVIE += (pEid->Len + 2);
			}

			break;

		case IE_QBSS_LOAD:
			if (pEid->Len == 5) {
				ie_list->QbssLoad.bValid = TRUE;
				ie_list->QbssLoad.StaNum = pEid->Octet[0] + pEid->Octet[1] * 256;
				ie_list->QbssLoad.ChannelUtilization = pEid->Octet[2];
				ie_list->QbssLoad.RemainingAdmissionControl = pEid->Octet[3] + pEid->Octet[4] * 256;
				/* Copy to pVIE*/
				Ptr = (PUCHAR) pVIE;
				NdisMoveMemory(Ptr + *LengthVIE, ptr_eid, pEid->Len + 2);
				*LengthVIE += (pEid->Len + 2);
			}

			break;
#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			Ptr = (PUCHAR) pVIE;
			NdisMoveMemory(Ptr + *LengthVIE, ptr_eid, pEid->Len + 2);
			*LengthVIE += (pEid->Len + 2);
			break;
#endif /* DOT11R_FT_SUPPORT */

		case IE_EXT_CAPABILITY:
			if (pEid->Len >= 1) {
				UCHAR cp_len, buf_space = sizeof(EXT_CAP_INFO_ELEMENT);

				cp_len = min(pEid->Len, buf_space);
				NdisMoveMemory(&ie_list->ExtCapInfo, &pEid->Octet[0], cp_len);
#ifdef RT_BIG_ENDIAN
				(*(UINT32 *)(&(ie_list->ExtCapInfo))) =
					le2cpu32(*(UINT32 *)(&(ie_list->ExtCapInfo)));
				(*(UINT32 *)(&(ie_list->ExtCapInfo)+4)) =
					le2cpu32(*(UINT32 *)(&(ie_list->ExtCapInfo)+4));
#endif

			}

			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (pEid->Len == sizeof(VHT_CAP_IE)) {
#ifdef RT_BIG_ENDIAN
				UINT32 tmp_1;
				UINT64 tmp_2;
#endif

				NdisMoveMemory(&ie_list->vht_cap_ie, &pEid->Octet[0], sizeof(VHT_CAP_IE));
				ie_list->vht_cap_len = pEid->Len;
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &ie_list->vht_cap_ie.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&ie_list->vht_cap_ie.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(ie_list->vht_cap_ie.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(ie_list->vht_cap_ie.mcs_set), &tmp_2, 8);
#endif
			}

			break;

		case IE_VHT_OP:
			if (pEid->Len == sizeof(VHT_OP_IE)) {
#ifdef RT_BIG_ENDIAN
				UINT16 tmp;
#endif
				NdisMoveMemory(&ie_list->vht_op_ie, &pEid->Octet[0], sizeof(VHT_OP_IE));
				ie_list->vht_op_len = pEid->Len;
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp, &ie_list->vht_op_ie.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&ie_list->vht_op_ie.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif
			}

			break;

		case IE_OPERATING_MODE_NOTIFY:
			if (pEid->Len == sizeof(OPERATING_MODE) && (bFromBeaconReport == FALSE)) {
#ifdef CONFIG_AP_SUPPORT
				MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
#endif
				OPERATING_MODE op_mode;

				NdisMoveMemory(&op_mode, &pEid->Octet[0], sizeof(OPERATING_MODE));

				if ((pEntry) && (op_mode.rx_nss_type == 0)) {
					pEntry->force_op_mode = TRUE;
					NdisMoveMemory(&pEntry->operating_mode, &op_mode, 1);
				}

				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s() - IE_OPERATING_MODE_NOTIFY(=%d)\n", __func__, pEid->Eid));
			}

			break;

		case IE_CH_SWITCH_WRAPPER: {
			INT8 ch_sw_wrp_len = pEid->Len;
			UCHAR *subelement = &pEid->Octet[0];
			INT8		len_subelement = 0;

			while (ch_sw_wrp_len > 0) {
				len_subelement = *(subelement + 1);

				if (*subelement == IE_WIDE_BW_CH_SWITCH) {
					NdisMoveMemory(&ie_list->wb_info, subelement, len_subelement);
					break;
				} else {
					subelement  += ch_sw_wrp_len;
					ch_sw_wrp_len -= ch_sw_wrp_len;
				}
			}
		}
		break;
#endif /* DOT11_VHT_AC */

		default:
			break;
		}

		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
		pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
	}

	LatchRfChannel = MsgChannel;
	/* this will lead Infra can't get beacon, when Infra on 5G then p2p in 2G. */
#ifdef CONFIG_MULTI_CHANNEL

	if (((Sanity & 0x4) == 0))
#else /* CONFIG_MULTI_CHANNEL */
	if ((LatchRfChannel > 14) && ((Sanity & 0x4) == 0))
#endif /* !CONFIG_MULTI_CHANNEL */
	{
		struct freq_oper oper;
		UCHAR bw = BW_20;
		if (hc_radio_query_by_rf(pAd, RFIC_5GHZ, &oper) == HC_STATUS_OK)
			bw = oper.bw;

		if (CtrlChannel != 0)
			ie_list->Channel = CtrlChannel;
		else {
			if (bw == BW_40
#ifdef DOT11_VHT_AC
				|| bw == BW_80
#endif /* DOT11_VHT_AC */
			   ) {
				{
					ie_list->Channel = LatchRfChannel;
				}
			} else
				ie_list->Channel = LatchRfChannel;
		}

		Sanity |= 0x4;
	}

	if (pPeerWscIe && (PeerWscIeLen > 0) && (PeerWscIeLen <= 512) && (bWscCheck == TRUE)) {
		UCHAR WscIe[] = {0xdd, 0x00, 0x00, 0x50, 0xF2, 0x04};

		Ptr = (PUCHAR) pVIE;
		WscIe[1] = PeerWscIeLen + 4;
		NdisMoveMemory(Ptr + *LengthVIE, WscIe, 6);
		NdisMoveMemory(Ptr + *LengthVIE + 6, pPeerWscIe, PeerWscIeLen);
		*LengthVIE += (PeerWscIeLen + 6);
	}

SanityCheck:

	if (pPeerWscIe)
		os_free_mem(pPeerWscIe);

	if ((Sanity != 0x7) || (bWscCheck == FALSE)) {
		if (((bFromBeaconReport == FALSE) && (Sanity != 0x7)) /* case 1: */
			|| ((bFromBeaconReport == TRUE) && (!(Sanity & 0x1))) /* case 2: */
			|| (bWscCheck == FALSE) /* case 3 */
		   ) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s() - missing field, Sanity=0x%02x, bWscCheck=%d\n", __func__, Sanity, bWscCheck));
			return FALSE;
		}
	} else {
	}

	return TRUE;
}

#ifdef DOT11N_DRAFT3
/*
	==========================================================================
	Description:
		MLME message sanity check for some IE addressed  in 802.11n d3.03.
	Return:
		TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
BOOLEAN PeerBeaconAndProbeRspSanity2(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	IN OVERLAP_BSS_SCAN_IE * BssScan,
	OUT UCHAR	*RegClass)
{
	CHAR				*Ptr;
	PFRAME_802_11		pFrame;
	PEID_STRUCT			pEid;
	ULONG				Length = 0;
	BOOLEAN				brc;

	pFrame = (PFRAME_802_11)Msg;
	*RegClass = 0;
	Ptr = pFrame->Octet;
	Length += LENGTH_802_11;
	/* get timestamp from payload and advance the pointer*/
	Ptr += TIMESTAMP_LEN;
	Length += TIMESTAMP_LEN;
	/* get beacon interval from payload and advance the pointer*/
	Ptr += 2;
	Length += 2;
	/* get capability info from payload and advance the pointer*/
	Ptr += 2;
	Length += 2;
	pEid = (PEID_STRUCT) Ptr;
	brc = FALSE;
	RTMPZeroMemory(BssScan, sizeof(OVERLAP_BSS_SCAN_IE));

	/* get variable fields from payload and advance the pointer*/
	while ((Length + 2 + pEid->Len) <= MsgLen) {
		switch (pEid->Eid) {
		case IE_SUPP_REG_CLASS:
			if (pEid->Len > 0)
				*RegClass = *pEid->Octet;
			else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_SUPP_REG_CLASS (len=%d)\n", pEid->Len));

			break;

		case IE_OVERLAPBSS_SCAN_PARM:
			if (pEid->Len == sizeof(OVERLAP_BSS_SCAN_IE)) {
				brc = TRUE;
				RTMPMoveMemory(BssScan, pEid->Octet, sizeof(OVERLAP_BSS_SCAN_IE));
			} else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerBeaconAndProbeRspSanity - wrong IE_OVERLAPBSS_SCAN_PARM (len=%d)\n", pEid->Len));

			break;

		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerBeaconAndProbeRspSanity - IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT\n"));
			break;
		}

		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len]	*/
		pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
	}

	return brc;
}
#endif /* DOT11N_DRAFT3 */

#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeScanReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT UCHAR *pBssType,
	OUT CHAR Ssid[],
	OUT UCHAR *pSsidLen,
	OUT UCHAR *pScanType)
{
	MLME_SCAN_REQ_STRUCT *Info;

	Info = (MLME_SCAN_REQ_STRUCT *)(Msg);
	*pBssType = Info->BssType;
	*pSsidLen = Info->SsidLen;
	NdisMoveMemory(Ssid, Info->Ssid, *pSsidLen);
	*pScanType = Info->ScanType;

	if ((*pBssType == BSS_INFRA || *pBssType == BSS_ADHOC || *pBssType == BSS_ANY)
		&& (SCAN_MODE_VALID(*pScanType))
	   )
		return TRUE;
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeScanReqSanity fail - wrong BssType or ScanType\n"));
		return FALSE;
	}
}
#endif

/* IRQL = DISPATCH_LEVEL*/
UCHAR ChannelSanity(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	int i;
	UCHAR BandIdx = HcGetBandByChannel(pAd, channel);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (channel == pChCtrl->ChList[i].Channel)
			return 1;
	}

	return 0;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef DBDC_MODE
UCHAR CheckWPSTriggeredPerBand(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR bss_index = 0;
	UCHAR current_band;

	current_band = HcGetBandByChannel(pAd, Elem->Channel);

	for (bss_index = 0; bss_index < pAd->ApCfg.BssidNum; bss_index++) {
		/* For the current band , check whether any bss has triggered wps, if yes,
		*   do WscCheckPeerDPID to add WscPBCStaProbeCount which is ued to
		*   check if any wps pbc overlap existed; if not, do nothing to avoid false
		*   alarm of wps pbc overlap
		*/
		if ((current_band == HcGetBandByWdev(&pAd->ApCfg.MBSSID[bss_index].wdev)) &&
				pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.WscConfMode != WSC_DISABLE &&
				pAd->ApCfg.MBSSID[bss_index].wdev.WscControl.bWscTrigger == TRUE)
			break;
	}

	return bss_index;
}
#endif /*DBDC_MODE*/
#endif /*CONFIG_AP_SUPPORT*/

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDeauthSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr1,
	OUT PUCHAR pAddr2,
	OUT PUCHAR pAddr3,
	OUT USHORT *pReason)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr1, pFrame->Hdr.Addr1);
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	COPY_MAC_ADDR(pAddr3, pFrame->Hdr.Addr3);
	NdisMoveMemory(pReason, &pFrame->Octet[0], 2);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerAuthSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT USHORT *pAlg,
	OUT USHORT *pSeq,
	OUT USHORT *pStatus,
	CHAR *pChlgText)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr,   pFrame->Hdr.Addr2);
	NdisMoveMemory(pAlg,    &pFrame->Octet[0], 2);
	NdisMoveMemory(pSeq,    &pFrame->Octet[2], 2);
	NdisMoveMemory(pStatus, &pFrame->Octet[4], 2);

	if (*pAlg == AUTH_MODE_OPEN) {
		if (*pSeq == 1 || *pSeq == 2)
			return TRUE;
		else {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
			return FALSE;
		}
	} else if (*pAlg == AUTH_MODE_KEY) {
		if (*pSeq == 1 || *pSeq == 4)
			return TRUE;
		else if (*pSeq == 2 || *pSeq == 3) {
			NdisMoveMemory(pChlgText, &pFrame->Octet[8], CIPHER_TEXT_LEN);
			return TRUE;
		} else {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAuthSanity fail - wrong Seg#\n"));
			return FALSE;
		}
	}

#ifdef DOT11R_FT_SUPPORT
	else if (*pAlg == AUTH_MODE_FT)
		return TRUE;

#endif /* DOT11R_FT_SUPPORT */
	else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAuthSanity fail - wrong algorithm\n"));
		return FALSE;
	}
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN MlmeAuthReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT ULONG *pTimeout,
	OUT USHORT *pAlg)
{
	MLME_AUTH_REQ_STRUCT *pInfo;
	pInfo  = (MLME_AUTH_REQ_STRUCT *)Msg;
	COPY_MAC_ADDR(pAddr, pInfo->Addr);
	*pTimeout = pInfo->Timeout;
	*pAlg = pInfo->Alg;

	if (((*pAlg == AUTH_MODE_KEY) || (*pAlg == AUTH_MODE_OPEN)
#ifdef DOT11R_FT_SUPPORT
		 || (*pAlg == AUTH_MODE_FT)
#endif /* DOT11R_FT_SUPPORT */
		) &&
		((*pAddr & 0x01) == 0)) {
		return TRUE;
	} else {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MlmeAuthReqSanity fail - wrong algorithm\n"));
		return FALSE;
	}
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN MlmeAssocReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pApAddr,
	OUT USHORT *pCapabilityInfo,
	OUT ULONG *pTimeout,
	OUT USHORT *pListenIntv)
{
	MLME_ASSOC_REQ_STRUCT *pInfo;

	pInfo = (MLME_ASSOC_REQ_STRUCT *)Msg;
	*pTimeout = pInfo->Timeout;                             /* timeout*/
	COPY_MAC_ADDR(pApAddr, pInfo->Addr);                   /* AP address*/
	*pCapabilityInfo = pInfo->CapabilityInfo;               /* capability info*/
	*pListenIntv = pInfo->ListenIntv;
	return TRUE;
}

/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise

	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN PeerDisassocSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pReason)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	NdisMoveMemory(pReason, &pFrame->Octet[0], 2);
	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Sanity check NetworkType (11b, 11g or 11a)

	Arguments:
		pBss - Pointer to BSS table.

	Return Value:
	Ndis802_11DS .......(11b)
	Ndis802_11OFDM24....(11g)
	Ndis802_11OFDM5.....(11a)

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
NDIS_802_11_NETWORK_TYPE NetworkTypeInUseSanity(BSS_ENTRY *pBss)
{
	NDIS_802_11_NETWORK_TYPE	NetWorkType;
	UCHAR						rate, i;

	NetWorkType = Ndis802_11DS;

	if (pBss->Channel <= 14) {
		/* First check support Rate.*/
		for (i = 0; i < pBss->SupRateLen; i++) {
			rate = pBss->SupRate[i] & 0x7f; /* Mask out basic rate set bit*/

			if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
				continue;
			else {
				/* Otherwise (even rate > 108) means Ndis802_11OFDM24*/
				NetWorkType = Ndis802_11OFDM24;
				break;
			}
		}

		/* Second check Extend Rate.*/
		if (NetWorkType != Ndis802_11OFDM24) {
			for (i = 0; i < pBss->ExtRateLen; i++) {
				rate = pBss->SupRate[i] & 0x7f; /* Mask out basic rate set bit*/

				if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
					continue;
				else {
					/* Otherwise (even rate > 108) means Ndis802_11OFDM24*/
					NetWorkType = Ndis802_11OFDM24;
					break;
				}
			}
		}
	} else
		NetWorkType = Ndis802_11OFDM5;

	if (pBss->HtCapabilityLen != 0) {
		if (NetWorkType == Ndis802_11OFDM5) {
#ifdef DOT11_VHT_AC

			if (pBss->vht_cap_len != 0)
				NetWorkType = Ndis802_11OFDM5_AC;
			else
#endif /* DOT11_VHT_AC */
				NetWorkType = Ndis802_11OFDM5_N;
		} else
			NetWorkType = Ndis802_11OFDM24_N;
	}

	return NetWorkType;
}


/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerProbeReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID * Msg,
	IN ULONG MsgLen,
	OUT PEER_PROBE_REQ_PARAM * ProbeReqParam)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;
	UCHAR		*Ptr;
	UCHAR		eid = 0, eid_len = 0, *eid_data;
#ifdef CONFIG_AP_SUPPORT
#if defined(WSC_INCLUDED) || defined(EASY_CONFIG_SETUP) || defined(WAPP_SUPPORT)
	UCHAR       apidx = MAIN_MBSSID;
#endif /* defined(WSC_INCLUDED) || defined(EASY_CONFIG_SETUP) */
	UCHAR       Addr1[MAC_ADDR_LEN];
#ifdef WSC_INCLUDED
	UCHAR		*pPeerWscIe = NULL;
	UINT		PeerWscIeLen = 0;
	BOOLEAN		bWscCheck = TRUE;
#endif /* WSC_INCLUDED */
#endif /* CONFIG_AP_SUPPORT */
	UINT		total_ie_len = 0;

	MLME_QUEUE_ELEM *Elem = NULL;
	UCHAR current_band = 0;

	/* NdisZeroMemory(ProbeReqParam, sizeof(*ProbeReqParam)); */
	COPY_MAC_ADDR(ProbeReqParam->Addr2, &Fr->Hdr.Addr2);

	if (Fr->Octet[0] != IE_SSID || Fr->Octet[1] > MAX_LEN_OF_SSID) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): sanity fail - wrong SSID IE\n", __func__));
		return FALSE;
	}

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	if (!ApCheckAccessControlList(pAd, ProbeReqParam->Addr2, apidx)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCheckAccessControlList(): Keep silent\n"));
		return FALSE;
	}
#endif

	ProbeReqParam->SsidLen = Fr->Octet[1];
	NdisMoveMemory(ProbeReqParam->Ssid, &Fr->Octet[2], ProbeReqParam->SsidLen);
#ifdef CONFIG_AP_SUPPORT
	COPY_MAC_ADDR(Addr1, &Fr->Hdr.Addr1);
#ifdef WSC_AP_SUPPORT
	os_alloc_mem(NULL, &pPeerWscIe, 512);
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	Ptr = Fr->Octet;
	eid = Ptr[0];
	eid_len = Ptr[1];
	total_ie_len = eid_len + 2;
	eid_data = Ptr + 2;

	/* get variable fields from payload and advance the pointer*/
	while ((eid_data + eid_len) <= ((UCHAR *)Fr + MsgLen)) {
		switch (eid) {
		case IE_VENDOR_SPECIFIC:
			if (eid_len <= 4)
				break;

#ifdef RSSI_FEEDBACK

			if (ProbeReqParam->bRssiRequested &&
				NdisEqualMemory(eid_data, RALINK_OUI, 3) && (eid_len == 7)) {
				if (*(eid_data + 3/* skip RALINK_OUI */) & 0x8)
					ProbeReqParam->bRssiRequested = TRUE;

				break;
			}

#endif /* RSSI_FEEDBACK */


			if (NdisEqualMemory(eid_data, WPS_OUI, 4)
			   ) {
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
		Elem = CONTAINER_OF(Msg, MLME_QUEUE_ELEM, Msg[0]);
		current_band = HcGetBandByChannel(pAd, Elem->Channel);

					WscCheckPeerDPID(pAd, Fr, eid_data, eid_len, current_band);

#ifdef CONFIG_AP_SUPPORT

				if (pPeerWscIe) {
					/* Ignore old WPS IE fragments, if we get the version 0x10 */
					if (*(eid_data + 4) == 0x10) { /* First WPS IE will have version 0x10 */
						NdisMoveMemory(pPeerWscIe, eid_data + 4, eid_len - 4);
						PeerWscIeLen = (eid_len - 4);
					} else { /* reassembly remanning, other IE fragmentations will not have version 0x10 */
						if ((PeerWscIeLen + (eid_len - 4)) <= 512) {
							NdisMoveMemory(pPeerWscIe + PeerWscIeLen, eid_data + 4, eid_len - 4);
							PeerWscIeLen += (eid_len - 4);
						} else { /* ((PeerWscIeLen +(eid_len-4)) > 512) */
							bWscCheck = FALSE;
							MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error!Sum of All PeerWscIeLen = %d (> 512)\n",
									 __func__, (PeerWscIeLen + (eid_len - 4))));
						}
					}
				} else {
					bWscCheck = FALSE;
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error!!! pPeerWscIe is empty!\n", __func__));
				}

#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */
			}

			break;
#ifdef CONFIG_HOTSPOT

		case IE_INTERWORKING:
			ProbeReqParam->AccessNetWorkType = (*eid_data) & 0x0F;

			if (eid_len > 3) {
				if (eid_len == 7)
					NdisMoveMemory(ProbeReqParam->Hessid, eid_data + 1, MAC_ADDR_LEN);
				else
					NdisMoveMemory(ProbeReqParam->Hessid, eid_data + 3, MAC_ADDR_LEN);

				ProbeReqParam->IsHessid = TRUE;
			}

			ProbeReqParam->IsIWIE = TRUE;
			break;
#endif

		case IE_EXT_CAPABILITY:
#ifdef CONFIG_HOTSPOT
			if (eid_len >= 4) {
				if (((*(eid_data + 3)) & 0x80) == 0x80)
					ProbeReqParam->IsIWCapability = TRUE;
			}

#endif
			break;
#if (defined(BAND_STEERING) || defined(WH_EVENT_NOTIFIER))
			case IE_HT_CAP:
				if (eid_len >= SIZE_HT_CAP_IE) {
#ifdef BAND_STEERING
					if (pAd->ApCfg.BandSteering) {
						ProbeReqParam->IsHtSupport = TRUE;
						ProbeReqParam->RxMCSBitmask = *(UINT32 *)(eid_data + 3);
					}
#endif
				} else
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - wrong IE_HT_CAP. eid_len = %d\n", __func__, eid_len));
				break;

#ifdef DOT11_VHT_AC
			case IE_VHT_CAP:
				if (eid_len >= SIZE_OF_VHT_CAP_IE)
				{
#ifdef BAND_STEERING
					if (pAd->ApCfg.BandSteering)
						ProbeReqParam->IsVhtSupport = TRUE;
#endif
		}
				else
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s() - wrong IE_VHT_CAP. eid_len = %d\n", __func__, eid_len));
				break;
#endif /* DOT11_VHT_AC */
#endif
		default:
			break;
		}

		eid = Ptr[total_ie_len];
		eid_len = Ptr[total_ie_len + 1];
		eid_data = Ptr + total_ie_len + 2;
		total_ie_len += (eid_len + 2);
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_INCLUDED

	if (pPeerWscIe && (PeerWscIeLen > 0) && (bWscCheck == TRUE)) {
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
			if (NdisEqualMemory(Addr1, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
				break;
		}

		/*
			Due to Addr1 in Probe Request may be FF:FF:FF:FF:FF:FF
			and we need to send out this information to external registrar.
			Therefore we choose ra0 to send this probe req when we couldn't find apidx by Addr1.
		*/
		if (apidx >= pAd->ApCfg.BssidNum)
			apidx = MAIN_MBSSID;

		if ((pAd->ApCfg.MBSSID[apidx].wdev.WscControl.WscConfMode & WSC_PROXY) != WSC_DISABLE) {
			int bufLen = 0;
			PUCHAR pBuf = NULL;
			WSC_IE_PROBREQ_DATA	*pprobreq = NULL;
			/*
				PeerWscIeLen: Len of WSC IE without WSC OUI
			*/
			bufLen = sizeof(WSC_IE_PROBREQ_DATA) + PeerWscIeLen;
			os_alloc_mem(NULL, &pBuf, bufLen);

			if (pBuf) {
				/*Send WSC probe req to UPnP*/
				NdisZeroMemory(pBuf, bufLen);
				pprobreq = (WSC_IE_PROBREQ_DATA *)pBuf;

				if (ProbeReqParam->SsidLen <= 32) {	/*Well, I think that it must be TRUE!*/
					NdisMoveMemory(pprobreq->ssid, ProbeReqParam->Ssid, ProbeReqParam->SsidLen);			/* SSID*/
					NdisMoveMemory(pprobreq->macAddr, Fr->Hdr.Addr2, 6);	/* Mac address*/
					pprobreq->data[0] = PeerWscIeLen >> 8;									/* element ID*/
					pprobreq->data[1] = PeerWscIeLen & 0xff;							/* element Length					*/
					NdisMoveMemory((pBuf + sizeof(WSC_IE_PROBREQ_DATA)), pPeerWscIe, PeerWscIeLen);	/* (WscProbeReqData)*/
					WscSendUPnPMessage(pAd, apidx,
									   WSC_OPCODE_UPNP_MGMT, WSC_UPNP_MGMT_SUB_PROBE_REQ,
									   pBuf, bufLen, 0, 0, &Fr->Hdr.Addr2[0], AP_MODE);
				}

				os_free_mem(pBuf);
			}
		}
	}

	if (pPeerWscIe)
		os_free_mem(pPeerWscIe);

#endif /* WSC_INCLUDED */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

/*
========================================================================
Routine Description:
	Check a packet is Action frame or not

Arguments:
	pAd			- WLAN control block pointer
	pbuf			- packet buffer

Return Value:
	TRUE		- yes
	FALSE		- no

========================================================================
*/
BOOLEAN
IsPublicActionFrame(
	IN PRTMP_ADAPTER	pAd,
	IN VOID * pbuf
)
{
	HEADER_802_11 *pHeader = pbuf;
	UINT8 *ptr = pbuf;

	if (pHeader->FC.Type != FC_TYPE_MGMT)
		return FALSE;

	if (pHeader->FC.SubType != SUBTYPE_ACTION)
		return FALSE;

	ptr += sizeof(HEADER_802_11);

	if (*ptr == CATEGORY_PUBLIC)
		return TRUE;
	else
		return FALSE;
}

