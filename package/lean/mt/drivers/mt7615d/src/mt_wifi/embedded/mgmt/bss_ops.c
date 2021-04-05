/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    bss.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-25      AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"

UCHAR CISCO_OUI[]       = {0x00, 0x40, 0x96};
UCHAR RALINK_OUI[]      = {0x00, 0x0c, 0x43};
UCHAR WPA_OUI[]         = {0x00, 0x50, 0xf2, 0x01};
UCHAR RSN_OUI[]         = {0x00, 0x0f, 0xac};
UCHAR WAPI_OUI[]        = {0x00, 0x14, 0x72};
UCHAR WME_INFO_ELEM[]   = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
UCHAR WME_PARM_ELEM[]   = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};
UCHAR BROADCOM_OUI[]    = {0x00, 0x90, 0x4c};
UCHAR MARVELL_OUI[]     = {0x00, 0x50, 0x43};
UCHAR ATHEROS_OUI[] = {0x00, 0x03, 0x7F};
UCHAR WPS_OUI[]         = {0x00, 0x50, 0xf2, 0x04};
#if defined(WH_EZ_SETUP) || defined(MWDS) || defined(WAPP_SUPPORT)
UCHAR MTK_OUI[]         = {0x00, 0x0c, 0xe7};
#endif /* WH_EZ_SETUP || MWDS */


#ifdef IGMP_TVM_SUPPORT
UCHAR IGMP_TVM_OUI[] = {0x00, 0x0D, 0x02, 0x03};
#endif /* IGMP_TVM_SUPPORT */

extern UCHAR WPA_OUI[];
extern UCHAR SES_OUI[];

UCHAR ZeroSsid[MAX_LEN_OF_SSID] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static VOID BssCipherParse(BSS_ENTRY *pBss)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	PRSN_IE_HEADER_STRUCT			pRsnHeader;
	PCIPHER_SUITE_STRUCT			pCipher;
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	SHORT								Length;
	UCHAR end_field = 0;
	UCHAR res = TRUE;
	/* WepStatus will be reset later, if AP announce TKIP or AES on the beacon frame.*/
	CLEAR_SEC_AKM(pBss->AKMMap);
	CLEAR_CIPHER(pBss->PairwiseCipher);
	CLEAR_CIPHER(pBss->GroupCipher);
	Length = (SHORT) pBss->VarIELen;

	while (Length > 0) {
		/* Parse cipher suite base on WPA1 & WPA2, they should be parsed differently*/
		pTmp = ((PUCHAR) pBss->VarIEs) + pBss->VarIELen - ((USHORT)Length);
		pEid = (PEID_STRUCT) pTmp;

		switch (pEid->Eid) {
		case IE_WPA:
			if (NdisEqualMemory(pEid->Octet, SES_OUI, 3) && (pEid->Len == 7)) {
				pBss->bSES = TRUE;
				break;
			} else if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1) {
				/* if unsupported vendor specific IE*/
				break;
			}

			/*
				Skip OUI, version, and multicast suite
				This part should be improved in the future when AP supported multiple cipher suite.
				For now, it's OK since almost all APs have fixed cipher suite supported.
			*/
			/* pTmp = (PUCHAR) pEid->Octet;*/
			pTmp   += 11;

			/*
				Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
				Value	   Meaning
				0			None
				1			WEP-40
				2			Tkip
				3			WRAP
				4			AES
				5			WEP-104
			*/
			/* Parse group cipher*/
			switch (*pTmp) {
			case 1:
				SET_CIPHER_WEP40(pBss->GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pBss->GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pBss->GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pBss->GroupCipher);
				break;

			default:
				break;
			}

			/* number of unicast suite*/
			pTmp   += 1;
			/* skip all unicast cipher suites*/
			/*Count = *(PUSHORT) pTmp;				*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pTmp += 3;

				switch (*pTmp) {
				case 1:
					SET_CIPHER_WEP40(pBss->PairwiseCipher);
					break;

				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					SET_CIPHER_WEP104(pBss->PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pBss->PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pBss->PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp++;
				Count--;
			}

			/* 4. get AKM suite counts*/
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			pTmp   += 3;

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				SET_AKM_WPA1(pBss->AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				SET_AKM_WPA1PSK(pBss->AKMMap);
				break;

			default:
				break;
			}

			pTmp   += 1;

			/* Fixed for WPA-None*/
			if (pBss->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(pBss->AKMMap);

			break;

		case IE_RSN:
			pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
			res = wpa_rsne_sanity(pTmp, le2cpu16(pRsnHeader->Length) + 2, &end_field);

			if (res == FALSE)
				break;

			if (end_field < RSN_FIELD_GROUP_CIPHER)
				SET_CIPHER_CCMP128(pBss->GroupCipher);
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				SET_CIPHER_CCMP128(pBss->PairwiseCipher);
			if (end_field < RSN_FIELD_AKM)
				SET_AKM_WPA2(pBss->AKMMap);

			/* 0. Version must be 1*/
			if (le2cpu16(pRsnHeader->Version) != 1)
				break;

			/* 1. Check group cipher*/
			if (end_field < RSN_FIELD_GROUP_CIPHER)
				break;

			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (!RTMPEqualMemory(&pCipher->Oui, RSN_OUI, 3))
				break;

			/* Parse group cipher*/
			switch (pCipher->Type) {
			case 1:
				SET_CIPHER_WEP40(pBss->GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pBss->GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pBss->GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pBss->GroupCipher);
				break;

			case 8:
				SET_CIPHER_GCMP128(pBss->GroupCipher);
				break;

			case 9:
				SET_CIPHER_GCMP256(pBss->GroupCipher);
				break;

			case 10:
				SET_CIPHER_CCMP256(pBss->GroupCipher);
				break;

			default:
				break;
			}

			/* set to correct offset for next parsing*/
			pTmp   += sizeof(CIPHER_SUITE_STRUCT);
			/* 2. Get pairwise cipher counts*/
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				break;
			/*Count = *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 3. Get pairwise cipher*/
			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

				switch (pCipher->Type) {
				case 1:
					SET_CIPHER_WEP40(pBss->PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pBss->PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pBss->PairwiseCipher);
					break;

				case 5:
					SET_CIPHER_WEP104(pBss->PairwiseCipher);
					break;

				case 8:
					SET_CIPHER_GCMP128(pBss->PairwiseCipher);
					break;

				case 9:
					SET_CIPHER_GCMP256(pBss->PairwiseCipher);
					break;

				case 10:
					SET_CIPHER_CCMP256(pBss->PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp += sizeof(CIPHER_SUITE_STRUCT);
				Count--;
			}

			/* 4. get AKM suite counts*/
			if (end_field < RSN_FIELD_AKM)
				break;
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 5. Get AKM ciphers*/
			/* Parsing all AKM ciphers*/
			while (Count > 0) {
				pAKM = (PAKM_SUITE_STRUCT) pTmp;

				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				switch (pAKM->Type) {
				case 0:
					SET_AKM_WPANONE(pBss->AKMMap);
					break;

				case 1:
					SET_AKM_WPA2(pBss->AKMMap);
					break;

				case 2:
					SET_AKM_WPA2PSK(pBss->AKMMap);
					break;

				case 3:
					SET_AKM_FT_WPA2(pBss->AKMMap);
					break;

				case 4:
					SET_AKM_FT_WPA2PSK(pBss->AKMMap);
					break;
#ifdef DOT11W_PMF_SUPPORT

				case 5:
					/* SET_AKM_WPA2_SHA256(pBss->AKMMap); */
					SET_AKM_WPA2(pBss->AKMMap);
					pBss->IsSupportSHA256KeyDerivation = TRUE;
					break;

				case 6:
					/* SET_AKM_WPA2PSK_SHA256(pBss->AKMMap); */
					SET_AKM_WPA2PSK(pBss->AKMMap);
					pBss->IsSupportSHA256KeyDerivation = TRUE;
					break;
#endif /* DOT11W_PMF_SUPPORT */

				case 7:
					SET_AKM_TDLS(pBss->AKMMap);
					break;

				case 8:
					SET_AKM_SAE_SHA256(pBss->AKMMap);
					break;

				case 9:
					SET_AKM_FT_SAE_SHA256(pBss->AKMMap);
					break;

				case 11:
					SET_AKM_SUITEB_SHA256(pBss->AKMMap);
					break;

				case 12:
					SET_AKM_SUITEB_SHA384(pBss->AKMMap);
					break;

				case 13:
					SET_AKM_FT_WPA2_SHA384(pBss->AKMMap);
					break;

				case 18:
					SET_AKM_OWE(pBss->AKMMap);
					break;

				default:
					break;
				}

				pTmp   += sizeof(AKM_SUITE_STRUCT);
				Count--;
			}

			/* Fixed for WPA-None*/
			if (pBss->BssType == BSS_ADHOC)
				SET_AKM_WPANONE(pBss->AKMMap);

			/* 6. Get RSN capability*/
			if (end_field < RSN_FIELD_RSN_CAP)
				break;
			/*pBss->WPA2.RsnCapability = *(PUSHORT) pTmp;*/
			pBss->RsnCapability = (pTmp[1] << 8) + pTmp[0];
			pTmp += sizeof(USHORT);
			break;

		default:
			break;
		}

		Length -= (pEid->Len + 2);
	}

	if (pBss->AKMMap == 0x0) {
		SET_AKM_OPEN(pBss->AKMMap);

		if (pBss->Privacy) {
			SET_AKM_SHARED(pBss->AKMMap);
			SET_CIPHER_WEP(pBss->PairwiseCipher);
			SET_CIPHER_WEP(pBss->GroupCipher);
		} else {
			SET_CIPHER_NONE(pBss->PairwiseCipher);
			SET_CIPHER_NONE(pBss->GroupCipher);
		}
	}
}


/*! \brief initialize BSS table
 *	\param p_tab pointer to the table
 *	\return none
 *	\pre
 *	\post

 IRQL = PASSIVE_LEVEL
 IRQL = DISPATCH_LEVEL

 */
VOID BssTableInit(BSS_TABLE *Tab)
{
	int i;

	Tab->BssNr = 0;
	Tab->BssOverlapNr = 0;

	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++) {
		UCHAR *pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;

		NdisZeroMemory(&Tab->BssEntry[i], sizeof(BSS_ENTRY));
		Tab->BssEntry[i].Rssi = -127;	/* initial the rssi as a minimum value */

		if (pOldAddr) {
			RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
			Tab->BssEntry[i].pVarIeFromProbRsp = pOldAddr;
		}
	}
}

#if defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT)
/*
	backup the other band's scan result, and init the Band from input.

	Band 0 : init 2.4G
	Band 1 : init 5G
*/
VOID BssTableInitByBand(BSS_TABLE *Tab, UCHAR Band)
{
	int i;
	INT bss_2G_cnt = 0;
	INT bss_5G_cnt = 0;
	INT bss_backup_cnt = 0;
	UCHAR *pOldAddr = NULL;
	BSS_TABLE *tab_buf = NULL;

	/* check the total 2.4G/5G BSS */
	if (Tab->BssNr > 0) {
		for (i = 0; i < Tab->BssNr; i++) {
			if (Tab->BssEntry[i].Channel > 14)
				bss_5G_cnt++;
			else
				bss_2G_cnt++;
		}
	}


	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==> Band=%u, bss_2G_cnt=%d, bss_5G_cnt=%d\n", __FUNCTION__, Band, bss_2G_cnt, bss_5G_cnt));

	if (((Band == 0) && bss_5G_cnt) ||
		((Band == 1) && bss_2G_cnt)) {
		os_alloc_mem(NULL, (UCHAR **)&tab_buf, sizeof(BSS_TABLE));

		if (tab_buf == NULL) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): alloc tab_buf fail!!\n", __FUNCTION__));
			return;
		}


		/* backup the dedicated band */
		for (i = 0; i < Tab->BssNr; i++) {
			if (((Tab->BssEntry[i].Channel > 14) && (Band == 0))  /* init 2.4G, backup 5G */
				|| ((Tab->BssEntry[i].Channel > 0) && (Tab->BssEntry[i].Channel <= 14) && (Band == 1))) /* init 5G, backup 2.4G */ {
				pOldAddr = Tab->BssEntry[i].pVarIeFromProbRsp;
				os_move_mem(&tab_buf->BssEntry[bss_backup_cnt], &Tab->BssEntry[i], sizeof(BSS_ENTRY));
				if (Tab->BssEntry[i].VarIeFromProbeRspLen && pOldAddr)
					os_move_mem(tab_buf->BssEntry[bss_backup_cnt].pVarIeFromProbRsp, pOldAddr, Tab->BssEntry[i].VarIeFromProbeRspLen);
				bss_backup_cnt++;
			}
		}

		tab_buf->BssNr = bss_backup_cnt;
		tab_buf->BssOverlapNr = 0;
		/* MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ==> Band=%u, bss_2G_cnt=%d, bss_5G_cnt=%d, bss_backup_cnt=%d\n",__FUNCTION__,Band, bss_2G_cnt, bss_5G_cnt, bss_backup_cnt)); */
		/* reset the global table */
		BssTableInit(Tab);
		/* restore the backup band */
		for (i = 0; i < bss_backup_cnt; i++) {
			pOldAddr = tab_buf->BssEntry[i].pVarIeFromProbRsp;
			os_move_mem(&Tab->BssEntry[i], &tab_buf->BssEntry[i], sizeof(BSS_ENTRY));

			if (tab_buf->BssEntry[i].VarIeFromProbeRspLen && pOldAddr)
				os_move_mem(Tab->BssEntry[i].pVarIeFromProbRsp, pOldAddr, Tab->BssEntry[i].VarIeFromProbeRspLen);
		}

		Tab->BssNr = tab_buf->BssNr;
		Tab->BssOverlapNr = tab_buf->BssOverlapNr;

		/* MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): <== Band=%u,  Tab->BssNr=%d\n",__FUNCTION__,Band, Tab->BssNr)); */
		if (tab_buf != NULL)
			os_free_mem(tab_buf);
	} else {	/* no need to backup, reset the global table */
		BssTableInit(Tab);
	}
}
#endif /* defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT) */

/*! \brief search the BSS table by SSID
 *	\param p_tab pointer to the bss table
 *	\param ssid SSID string
 *	\return index of the table, BSS_NOT_FOUND if not in the table
 *	\pre
 *	\post
 *	\note search by sequential search

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSearch(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
		/*
			Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.
			We should distinguish this case.
		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearch(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 pBssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
		/* Some AP that support A/B/G mode that may used the same BSSID on 11A and 11B/G.*/
		/* We should distinguish this case.*/
		/*		*/
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid) &&
			SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssTableSearchWithSSID(
	IN BSS_TABLE *Tab,
	IN PUCHAR	 Bssid,
	IN PUCHAR	 pSsid,
	IN UCHAR	 SsidLen,
	IN UCHAR	 Channel)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
		if ((((Tab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((Tab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(Tab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen) ||
			 (NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			 (NdisEqualMemory(Tab->BssEntry[i].Ssid, ZeroSsid, Tab->BssEntry[i].SsidLen))))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


ULONG BssSsidTableSearchBySSID(BSS_TABLE *Tab, UCHAR *pSsid, UCHAR SsidLen)
{
	UCHAR i;

	for (i = 0; i < Tab->BssNr  && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
		if (SSID_EQUAL(pSsid, SsidLen, Tab->BssEntry[i].Ssid, Tab->BssEntry[i].SsidLen))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}


VOID BssTableDeleteEntry(BSS_TABLE *Tab, UCHAR *pBssid, UCHAR Channel)
{
	UCHAR i, j;

	for (i = 0; i < Tab->BssNr && Tab->BssNr < MAX_LEN_OF_BSS_TABLE; i++) {
		if ((Tab->BssEntry[i].Channel == Channel) &&
			(MAC_ADDR_EQUAL(Tab->BssEntry[i].Bssid, pBssid))) {
			UCHAR *pOldAddr = NULL;

			for (j = i; j < Tab->BssNr - 1; j++) {
				pOldAddr = Tab->BssEntry[j].pVarIeFromProbRsp;
				NdisMoveMemory(&(Tab->BssEntry[j]), &(Tab->BssEntry[j + 1]), sizeof(BSS_ENTRY));

				if (pOldAddr) {
					RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
					NdisMoveMemory(pOldAddr,
								   Tab->BssEntry[j + 1].pVarIeFromProbRsp,
								   Tab->BssEntry[j + 1].VarIeFromProbeRspLen);
					Tab->BssEntry[j].pVarIeFromProbRsp = pOldAddr;
				}
			}

			pOldAddr = Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp;
			NdisZeroMemory(&(Tab->BssEntry[Tab->BssNr - 1]), sizeof(BSS_ENTRY));

			if (pOldAddr) {
				RTMPZeroMemory(pOldAddr, MAX_VIE_LEN);
				Tab->BssEntry[Tab->BssNr - 1].pVarIeFromProbRsp = pOldAddr;
			}

			Tab->BssNr -= 1;
			return;
		}
	}
}

#ifdef CONFIG_OWE_SUPPORT
static VOID update_bss_by_owe_trans(struct _RTMP_ADAPTER *ad,
				    ULONG bss_idx,
				    UCHAR *pair_bssid,
				    UCHAR *pair_ssid,
				    UCHAR pair_ssid_len)
{
	BSS_ENTRY *extracted_trans_bss = &ad->ScanTab.BssEntry[bss_idx];

	if (MAC_ADDR_EQUAL(extracted_trans_bss->Bssid, pair_bssid)) {
		if (extracted_trans_bss->Hidden == 1) {
			/*double confirm the hidden bss is OWE AKM*/
			if (!IS_AKM_OWE(extracted_trans_bss->AKMMap))
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : %02x-%02x-%02x-%02x-%02x-%02x, hidden SSID but not OWE_AKM:0x%x!?\n",
					__func__,
					PRINT_MAC(extracted_trans_bss->Bssid),
					extracted_trans_bss->AKMMap));

				extracted_trans_bss->hide_owe_bss = TRUE;

				extracted_trans_bss->bhas_owe_trans_ie = TRUE;


			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : %02x-%02x-%02x-%02x-%02x-%02x, update hidden SSID:%s\n",
					__func__,
					PRINT_MAC(extracted_trans_bss->Bssid),
					extracted_trans_bss->Ssid));
		}
	}
}
#endif


/*! \brief
 *	\param
 *	\return
 *	\pre
 *	\post
 */
VOID BssEntrySet(
	IN RTMP_ADAPTER *pAd,
	OUT BSS_ENTRY *pBss,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	,
	IN UCHAR	*Snr,
	IN CHAR 	*rssi
#endif

	)

{
	COPY_MAC_ADDR(pBss->Bssid, ie_list->Bssid);
	/* Default Hidden SSID to be TRUE, it will be turned to FALSE after coping SSID*/
	pBss->Hidden = 1;
	pBss->FromBcnReport = ie_list->FromBcnReport;

	if (ie_list->SsidLen > 0) {
		/* For hidden SSID AP, it might send beacon with SSID len equal to 0*/
		/* Or send beacon /probe response with SSID len matching real SSID length,*/
		/* but SSID is all zero. such as "00-00-00-00" with length 4.*/
		/* We have to prevent this case overwrite correct table*/
		if (NdisEqualMemory(ie_list->Ssid, ZeroSsid, ie_list->SsidLen) == 0) {
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			NdisMoveMemory(pBss->Ssid, ie_list->Ssid, ie_list->SsidLen);
			pBss->SsidLen = ie_list->SsidLen;
			pBss->Hidden = 0;
		}
	} else {
		/* avoid  Hidden SSID form beacon to overwirite correct SSID from probe response */
		if (NdisEqualMemory(pBss->Ssid, ZeroSsid, pBss->SsidLen)) {
			NdisZeroMemory(pBss->Ssid, MAX_LEN_OF_SSID);
			pBss->SsidLen = 0;
		}
	}

	pBss->BssType = ie_list->BssType;
	pBss->BeaconPeriod = ie_list->BeaconPeriod;

	if (ie_list->BssType == BSS_INFRA) {
		if (ie_list->CfParm.bValid) {
			pBss->CfpCount = ie_list->CfParm.CfpCount;
			pBss->CfpPeriod = ie_list->CfParm.CfpPeriod;
			pBss->CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pBss->CfpDurRemaining = ie_list->CfParm.CfpDurRemaining;
		}
	} else
		pBss->AtimWin = ie_list->AtimWin;

	NdisGetSystemUpTime(&pBss->LastBeaconRxTime);
	pBss->CapabilityInfo = ie_list->CapabilityInfo;
	/* The privacy bit indicate security is ON, it maight be WEP, TKIP or AES*/
	/* Combine with AuthMode, they will decide the connection methods.*/
	pBss->Privacy = CAP_IS_PRIVACY_ON(pBss->CapabilityInfo);
	ASSERT(ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES);

	if (ie_list->SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES)
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, ie_list->SupRateLen);
	else
		NdisMoveMemory(pBss->SupRate, ie_list->SupRate, MAX_LEN_OF_SUPPORTED_RATES);

	pBss->SupRateLen = ie_list->SupRateLen;
	ASSERT(ie_list->ExtRateLen <= MAX_LEN_OF_SUPPORTED_RATES);

	if (ie_list->ExtRateLen > MAX_LEN_OF_SUPPORTED_RATES)
		ie_list->ExtRateLen = MAX_LEN_OF_SUPPORTED_RATES;

	NdisMoveMemory(pBss->ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
	pBss->NewExtChanOffset = ie_list->NewExtChannelOffset;
	pBss->ExtRateLen = ie_list->ExtRateLen;
	pBss->Erp  = ie_list->Erp;
	pBss->Channel = ie_list->Channel;
	pBss->CentralChannel = ie_list->Channel;
	pBss->Rssi = Rssi;
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	pBss->Snr[0] = Snr[0];
	pBss->Snr[1] = Snr[1];
	pBss->Snr[2] = Snr[2];
	pBss->Snr[3] = Snr[3];

	pBss->rssi[0] = rssi[0];
	pBss->rssi[1] = rssi[1];
	pBss->rssi[2] = rssi[2];
	pBss->rssi[3] = rssi[3];

	NdisMoveMemory(pBss->vendorOUI0, ie_list->VendorID0, 3);
	NdisMoveMemory(pBss->vendorOUI1, ie_list->VendorID1, 3);
#endif

	/* Update CkipFlag. if not exists, the value is 0x0*/
	pBss->CkipFlag = ie_list->CkipFlag;
	/* New for microsoft Fixed IEs*/
	NdisMoveMemory(pBss->FixIEs.Timestamp, &ie_list->TimeStamp, 8);
	pBss->FixIEs.BeaconInterval = ie_list->BeaconPeriod;
	pBss->FixIEs.Capabilities = ie_list->CapabilityInfo;
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	pBss->LastBeaconRxTimeT = jiffies_to_msecs(jiffies);
#endif

#ifdef CONFIG_OWE_SUPPORT
	NdisZeroMemory(pBss->owe_trans_ie, MAX_VIE_LEN);
	pBss->owe_trans_ie_len = 0;
	pBss->bhas_owe_trans_ie = FALSE;
#endif

	/* New for microsoft Variable IEs*/
	if (LengthVIE != 0) {
		pBss->VarIELen = LengthVIE;
		NdisMoveMemory(pBss->VarIEs, pVIE, pBss->VarIELen);
	} else
		pBss->VarIELen = 0;

#ifdef CONFIG_MAP_SUPPORT
	pBss->map_vendor_ie_found = ie_list->vendor_ie.map_vendor_ie_found;
	if (pBss->map_vendor_ie_found)
		NdisMoveMemory(&pBss->map_info, &ie_list->vendor_ie.map_info, sizeof(struct map_vendor_ie));
#endif

	pBss->AddHtInfoLen = 0;
	pBss->HtCapabilityLen = 0;
#ifdef DOT11_N_SUPPORT

	if (ie_list->HtCapabilityLen > 0) {
		pBss->HtCapabilityLen = ie_list->HtCapabilityLen;
		NdisMoveMemory(&pBss->HtCapability, &ie_list->HtCapability, ie_list->HtCapabilityLen);

		if (ie_list->AddHtInfoLen > 0) {
			pBss->AddHtInfoLen = ie_list->AddHtInfoLen;
			NdisMoveMemory(&pBss->AddHtInfo, &ie_list->AddHtInfo, ie_list->AddHtInfoLen);
			pBss->CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
								   &ie_list->HtCapability);
		}

#ifdef DOT11_VHT_AC

		if (ie_list->vht_cap_len) {
			NdisMoveMemory(&pBss->vht_cap_ie, &ie_list->vht_cap_ie, ie_list->vht_cap_len);
			pBss->vht_cap_len = ie_list->vht_cap_len;
		}

		if (ie_list->vht_op_len) {
			VHT_OP_IE *vht_op;

			NdisMoveMemory(&pBss->vht_op_ie, &ie_list->vht_op_ie, ie_list->vht_op_len);
			pBss->vht_op_len = ie_list->vht_op_len;
			vht_op = &ie_list->vht_op_ie;

			if ((vht_op->vht_op_info.ch_width > 0) &&
				(ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
				(ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
				(pBss->CentralChannel != ie_list->AddHtInfo.ControlChan)) {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():VHT vht_op_info->center_freq_1=%d, Bss->CentCh=%d, change from CentralChannel to cent_ch!\n",
						  __func__, vht_op->vht_op_info.center_freq_1, pBss->CentralChannel));
				pBss->CentralChannel = vht_op->vht_op_info.center_freq_1;
			}
		}

#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
	BssCipherParse(pBss);

	/* new for QOS*/
	if (ie_list->EdcaParm.bValid)
		NdisMoveMemory(&pBss->EdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
	else
		pBss->EdcaParm.bValid = FALSE;

	if (ie_list->QosCapability.bValid)
		NdisMoveMemory(&pBss->QosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
	else
		pBss->QosCapability.bValid = FALSE;

	if (ie_list->QbssLoad.bValid)
		NdisMoveMemory(&pBss->QbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
	else
		pBss->QbssLoad.bValid = FALSE;

	{
		PEID_STRUCT pEid;
		USHORT Length = 0;
#ifdef WSC_INCLUDED
		pBss->WpsAP = 0x00;
		pBss->WscDPIDFromWpsAP = 0xFFFF;
#endif /* WSC_INCLUDED */
		pEid = (PEID_STRUCT) pVIE;

		while ((Length + 2 + (USHORT)pEid->Len) <= LengthVIE) {
#define WPS_AP		0x01

			switch (pEid->Eid) {
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4)
				   ) {
#ifdef WSC_INCLUDED
					pBss->WpsAP |= WPS_AP;
					WscCheckWpsIeFromWpsAP(pAd,
										   pEid,
										   &pBss->WscDPIDFromWpsAP);
#endif /* WSC_INCLUDED */
					break;
				}

#ifdef CONFIG_OWE_SUPPORT
				if (NdisEqualMemory(pEid->Octet, OWE_TRANS_OUI, 4)) {
					ULONG bss_idx = BSS_NOT_FOUND;
					UCHAR pair_ch = 0;
					UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
					UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
					UCHAR pair_band = 0;
					UCHAR pair_ssid_len = 0;


					NdisZeroMemory(pBss->owe_trans_ie, MAX_VIE_LEN);
					NdisMoveMemory(pBss->owe_trans_ie, pEid->Octet + 4, pEid->Len - 4);
					pBss->owe_trans_ie_len = pEid->Len - 4;


					pBss->bhas_owe_trans_ie = TRUE;

					extract_pair_owe_bss_info(pEid->Octet + 4,
								  pEid->Len - 4,
								  pair_bssid,
								  pair_ssid,
								  &pair_ssid_len,
								  &pair_band,
								  &pair_ch);
					if (pair_ch != 0)
						bss_idx = BssTableSearch(&pAd->ScanTab, pair_bssid, pair_ch);
					else
						bss_idx = BssTableSearch(&pAd->ScanTab, pair_bssid, ie_list->Channel);

					if (bss_idx != BSS_NOT_FOUND)
						update_bss_by_owe_trans(pAd,
									bss_idx,
									pair_bssid,
									pair_ssid,
									pair_ssid_len);
				}
#endif
				break;
			}

			Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len]*/
			pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
		}
	}
}

/*!
 *	\brief insert an entry into the bss table
 *	\param p_tab The BSS table
 *	\param Bssid BSSID
 *	\param ssid SSID
 *	\param ssid_len Length of SSID
 *	\param bss_type
 *	\param beacon_period
 *	\param timestamp
 *	\param p_cf
 *	\param atim_win
 *	\param cap
 *	\param rates
 *	\param rates_len
 *	\param channel_idx
 *	\return none
 *	\pre
 *	\post
 *	\note If SSID is identical, the old entry will be replaced by the new one

 IRQL = DISPATCH_LEVEL

 */
ULONG BssTableSetEntry(
	IN PRTMP_ADAPTER pAd,
	OUT BSS_TABLE *Tab,
	IN BCN_IE_LIST * ie_list,
	IN CHAR Rssi,
	IN USHORT LengthVIE,
	IN PNDIS_802_11_VARIABLE_IEs pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	,
	IN UCHAR	*Snr,
	IN CHAR 	*rssi
#endif

	)
{
	ULONG	Idx;
#ifdef APCLI_SUPPORT
	BOOLEAN bInsert = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	UCHAR i;
#endif /* APCLI_SUPPORT */
	Idx = BssTableSearch(Tab, ie_list->Bssid, ie_list->Channel);

	if (Idx == BSS_NOT_FOUND) {
		if (Tab->BssNr >= MAX_LEN_OF_BSS_TABLE) {
			/*
				It may happen when BSS Table was full.
				The desired AP will not be added into BSS Table
				In this case, if we found the desired AP then overwrite BSS Table.
			*/
#ifdef APCLI_SUPPORT
			for (i = 0; i < pAd->ApCfg.ApCliNum; i++) {
				pApCliEntry = &pAd->ApCfg.ApCliTab[i];

				if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid)
					|| SSID_EQUAL(pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen, ie_list->Ssid, ie_list->SsidLen)) {
					bInsert = TRUE;
					break;
				}
				if (MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid)
					|| SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, ie_list->Ssid, ie_list->SsidLen)) {
					bInsert = TRUE;
					break;
				}

			}

#endif /* APCLI_SUPPORT */

			if (
#ifdef CONFIG_AP_SUPPORT
				!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) ||
#endif
				!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
				if (MAC_ADDR_EQUAL(pAd->ScanCtrl.Bssid, ie_list->Bssid) ||
					SSID_EQUAL(pAd->ScanCtrl.Ssid, pAd->ScanCtrl.SsidLen, ie_list->Ssid, ie_list->SsidLen)
#ifdef APCLI_SUPPORT
					|| bInsert
#endif /* APCLI_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
					/* YF: Driver ScanTable full but supplicant the SSID exist on supplicant */
					|| SSID_EQUAL(pAd->cfg80211_ctrl.Cfg_pending_Ssid, pAd->cfg80211_ctrl.Cfg_pending_SsidLen, ie_list->Ssid,
								  ie_list->SsidLen)
#endif /* RT_CFG80211_SUPPORT */
				   ) {
					Idx = Tab->BssOverlapNr;
					NdisZeroMemory(&(Tab->BssEntry[Idx]), sizeof(BSS_ENTRY));
					BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
					, Snr, rssi
#endif
					);
					Tab->BssOverlapNr += 1;
					Tab->BssOverlapNr = Tab->BssOverlapNr % MAX_LEN_OF_BSS_TABLE;
#ifdef RT_CFG80211_SUPPORT
					pAd->cfg80211_ctrl.Cfg_pending_SsidLen = 0;
					NdisZeroMemory(pAd->cfg80211_ctrl.Cfg_pending_Ssid, MAX_LEN_OF_SSID + 1);
#endif /* RT_CFG80211_SUPPORT */
				}

				return Idx;
			} else
				return BSS_NOT_FOUND;
		}

		Idx = Tab->BssNr;
		BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
		, Snr, rssi
#endif
		);
		Tab->BssNr++;
	} else if (Idx < MAX_LEN_OF_BSS_TABLE)
		BssEntrySet(pAd, &Tab->BssEntry[Idx], ie_list, Rssi, LengthVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
		, Snr, rssi
#endif
		);
	else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(error):Idx is larger than MAX_LEN_OF_BSS_TABLE", __func__));

	return Idx;
}


VOID BssTableSortByRssi(
	IN OUT BSS_TABLE *OutTab,
	IN BOOLEAN isInverseOrder)
{
	INT i, j;
	BSS_ENTRY *pTmpBss = NULL;
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&pTmpBss, sizeof(BSS_ENTRY));

	if (pTmpBss == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		return;
	}

	if (OutTab->BssNr == 0) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: BssNr=%d!!!\n", __func__, OutTab->BssNr));
		os_free_mem(pTmpBss);
		return;
	}

	for (i = 0; i < OutTab->BssNr - 1; i++) {
		for (j = i + 1; j < OutTab->BssNr; j++) {
			if (OutTab->BssEntry[j].Rssi > OutTab->BssEntry[i].Rssi ?
				!isInverseOrder : isInverseOrder) {
				if (OutTab->BssEntry[j].Rssi != OutTab->BssEntry[i].Rssi) {
					NdisMoveMemory(pTmpBss, &OutTab->BssEntry[j], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[j], &OutTab->BssEntry[i], sizeof(BSS_ENTRY));
					NdisMoveMemory(&OutTab->BssEntry[i], pTmpBss, sizeof(BSS_ENTRY));
				}
			}
		}
	}

	if (pTmpBss != NULL)
		os_free_mem(pTmpBss);
}



BOOLEAN bss_coex_insert_effected_ch_list(
	RTMP_ADAPTER *pAd,
	UCHAR Channel,
	BCN_IE_LIST *ie_list,
	struct wifi_dev *pwdev)
{
	BOOLEAN Inserted = FALSE;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.bOverlapScanning == TRUE) {
		INT index, secChIdx;
		ADD_HTINFO *pAdd_HtInfo;
		struct freq_oper oper;
		UCHAR BandIdx;
		CHANNEL_CTRL *pChCtrl;

		hc_radio_query_by_channel(pAd, Channel, &oper);
		BandIdx = HcGetBandByChannel(pAd, Channel);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		for (index = 0; index < pChCtrl->ChListNum; index++) {
			/* found the effected channel, mark that. */
			if (pChCtrl->ChList[index].Channel == ie_list->Channel) {
				secChIdx = -1;

				if (ie_list->HtCapabilityLen > 0 && ie_list->AddHtInfoLen > 0) {
					/* This is a 11n AP. */
					pChCtrl->ChList[index].bEffectedChannel |= EFFECTED_CH_PRIMARY; /* 2;	// 2 for 11N 20/40MHz AP with primary channel set as this channel. */
					pAdd_HtInfo = &ie_list->AddHtInfo.AddHtInfo;

					if (pAdd_HtInfo->ExtChanOffset == EXTCHA_BELOW) {
						if (ie_list->Channel > 14)
							secChIdx = ((index > 0) ? (index - 1) : -1);
						else
							secChIdx = ((index >= 4) ? (index - 4) : -1);
					} else if (pAdd_HtInfo->ExtChanOffset == EXTCHA_ABOVE) {
						if (ie_list->Channel > 14)
							secChIdx = (((index + 1) < pChCtrl->ChListNum) ? (index + 1) : -1);
						else
							secChIdx = (((index + 4) < pChCtrl->ChListNum) ? (index + 4) : -1);
					}

					if (secChIdx >= 0)
						pChCtrl->ChList[secChIdx].bEffectedChannel |= EFFECTED_CH_SECONDARY; /* 1; */

					if ((Channel != ie_list->Channel) ||
						(pAdd_HtInfo->ExtChanOffset != oper.ext_cha)) {
						pAd->CommonCfg.BssCoexApCnt++;
						Inserted = TRUE;
					}
				} else {
					/* This is a legacy AP. */
					pChCtrl->ChList[index].bEffectedChannel |=  EFFECTED_CH_LEGACY; /* 4; 1 for legacy AP. */
					pAd->CommonCfg.BssCoexApCnt++;
					Inserted = TRUE;
				}
			}
		}
	}

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	return Inserted;
}



