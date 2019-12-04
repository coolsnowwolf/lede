/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 *	Abstract:
 *
 *	All related CFG80211 P2P function body.
 *
 *	History:
 *
 ****************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	/* UCHAR lock_channel = CFG80211_getCenCh(pAd, Data); */
	UCHAR lock_channel = Data;

	if (RTMP_CFG80211_HOSTAPD_ON(pAd))
		return;

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
	struct wifi_dev *wdev = &pMbss->wdev;

	if (pAd->Mlme.bStartMcc == TRUE)
		return;

	if (pAd->Mlme.bStartScc == TRUE) {
		/* MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SCC Enabled, Do not switch channel for Tx  %d\n",lock_channel)); */
		return;
	}

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && (wdev->channel == lock_channel) && (wlan_operate_get_ht_bw(wdev) == HT_BW_40)) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("40 BW Enabled || GO enable , wait for CLI connect, Do not switch channel for Tx\n"));
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("GO wdev->channel  %d  lock_channel %d\n", wdev->channel, lock_channel));
		return;
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
	UCHAR cen_ch = wlan_operate_get_cen_ch_1(p2p_dev);

	if (INFRA_ON(pAd) &&
		(((pAd->LatchRfRegs.Channel != cen_ch) && (cen_ch != 0)))
		|| (pAd->LatchRfRegs.Channel != lock_channel))
#else
	if (pAd->LatchRfRegs.Channel != lock_channel)
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	{
		wlan_operate_set_prim_ch(p2p_dev, lock_channel);
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Send Packet: From(%d)-To(%d)\n",
				 pAd->LatchRfRegs.Channel, lock_channel));
	} else
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel));
}

#ifdef CONFIG_AP_SUPPORT
BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen)
{
	const UINT WFA_OUI = 0x0050F2;
	const UCHAR WMM_OUI_TYPE = 0x2;
	UCHAR *wmm_ie = NULL;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;
	EDCA_PARM *pBssEdca = NULL;
	/* hex_dump("probe_rsp_in:", pData, dataLen); */
	wmm_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, WMM_OUI_TYPE, pData, dataLen);

	if (wmm_ie != NULL) {
		UINT i = QID_AC_BE;
#ifdef UAPSD_SUPPORT
#endif /* UAPSD_SUPPORT */

		pBssEdca = wlan_config_get_ht_edca(wdev);
		if (pBssEdca) {
			/* WMM: sync from driver's EDCA paramter */
			for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
				wmm_ie[10 + (i * 4)] = (i << 5) +                                  /* b5-6 is ACI */
									   ((UCHAR)pBssEdca->bACM[i] << 4) +           /* b4 is ACM */
									   (pBssEdca->Aifsn[i] & 0x0f);                /* b0-3 is AIFSN */
				wmm_ie[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4) +                 /* b5-8 is CWMAX */
									   (pBssEdca->Cwmin[i] & 0x0f);                /* b0-3 is CWMIN */
				wmm_ie[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);          /* low byte of TXOP */
				wmm_ie[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);            /* high byte of TXOP */
			}
		}
		return TRUE;
	}
	else
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can't find the wmm ie\n", __func__));

	return FALSE;
}
#endif /* CONFIG_AP_SUPPORT */


INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	if (pData != NULL) {
#ifdef CONFIG_AP_SUPPORT
		struct ieee80211_mgmt *mgmt;
#endif /* CONFIG_AP_SUPPORT */
		{
			PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

			pCfg80211_ctrl->TxStatusInUsed = TRUE;
			pCfg80211_ctrl->TxStatusSeq = pAd->Sequence;

			if (pCfg80211_ctrl->pTxStatusBuf != NULL) {
				os_free_mem(pCfg80211_ctrl->pTxStatusBuf);
				pCfg80211_ctrl->pTxStatusBuf = NULL;
			}

			os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->pTxStatusBuf, Data);

			if (pCfg80211_ctrl->pTxStatusBuf != NULL) {
				NdisCopyMemory(pCfg80211_ctrl->pTxStatusBuf, pData, Data);
				pCfg80211_ctrl->TxStatusBufLen = Data;
			} else {
				pCfg80211_ctrl->TxStatusBufLen = 0;
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG_TX_STATUS: MEM ALLOC ERROR\n"));
				return NDIS_STATUS_FAILURE;
			}

			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);
#ifdef CONFIG_AP_SUPPORT
			mgmt = (struct ieee80211_mgmt *)pData;

			if (ieee80211_is_probe_resp(mgmt->frame_control)) {
				INT offset = sizeof(HEADER_802_11) + 12;

				CFG80211_SyncPacketWmmIe(pAd, pData + offset, Data - offset);
			}

#endif /* CONFIG_AP_SUPPORT */
			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}

	return 0;
}

VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence, BOOLEAN ack)
{
	/* RTMP_USB_SUPPORT/RTMP_PCI_SUPPORT */
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (pCfg80211_ctrl->TxStatusInUsed && pCfg80211_ctrl->pTxStatusBuf
		/*&& (pAd->TxStatusSeq == pHeader->Sequence)*/) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG_TX_STATUS: REAL send %d\n", Sequence));
		CFG80211OS_TxStatus(CFG80211_GetEventDevice(pAd), 5678,
							pCfg80211_ctrl->pTxStatusBuf, pCfg80211_ctrl->TxStatusBufLen,
							ack);
		pCfg80211_ctrl->TxStatusSeq = 0;
		pCfg80211_ctrl->TxStatusInUsed = FALSE;
	}

}
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, struct wifi_dev *wdev, UCHAR *wpa_ie, UCHAR *rsn_ie)
{
	PEID_STRUCT		 pEid;
	PUCHAR				pTmp;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	PAKM_SUITE_STRUCT				pAKM;
	USHORT							Count;
	BOOLEAN bWPA = FALSE;
	BOOLEAN bWPA2 = FALSE;
	BOOLEAN bMix = FALSE;
	/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;
	CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
	CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
	CLEAR_GROUP_CIPHER(&wdev->SecConfig);

	if ((wpa_ie == NULL) && (rsn_ie == NULL)) { /* open case */
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: Open/None case\n", __func__));
		/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
		/* wdev->WepStatus = Ndis802_11WEPDisabled; */
		/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (wpa_ie != NULL) { /* wpapsk/tkipaes case */
		pEid = (PEID_STRUCT)wpa_ie;
		pTmp = (PUCHAR)pEid;

		if (os_equal_mem(pEid->Octet, WPA_OUI, 4)) {
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* SET_AKM_OPEN(wdev->SecConfig.AKMMap); */
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: WPA case\n", __func__));
			bWPA = TRUE;
			pTmp   += 11;

			switch (*pTmp) {
			case 1:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11GroupWEP40Enabled\n"));
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled; */
				SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
				break;

			case 5:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11GroupWEP104Enabled\n"));
				/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled; */
				SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
				break;

			case 2:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11TKIPEnable\n"));
				/* wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable; */
				SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
				break;

			case 4:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Group Ndis802_11AESEnable\n"));
				/* wdev->GroupKeyWepStatus  = Ndis802_11AESEnable; */
				SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
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
				TmpCipher = Ndis802_11WEPDisabled;

				switch (*pTmp) {
				case 1:
				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					TmpCipher = Ndis802_11WEPEnabled;
					break;

				case 2:
					TmpCipher = Ndis802_11TKIPEnable;
					break;

				case 4:
					TmpCipher = Ndis802_11AESEnable;
					break;

				default:
					break;
				}

				if (TmpCipher > PairCipher) {
					/* Move the lower cipher suite to PairCipherAux*/
					PairCipherAux = PairCipher;
					PairCipher	= TmpCipher;
				} else
					PairCipherAux = TmpCipher;

				pTmp++;
				Count--;
			}

			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Auth Count in WPA = %d ,we only parse the first for AKM\n", Count));
			pTmp   += 3; /* parse first AuthOUI for AKM */

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPA; */
				SET_AKM_WPA1(wdev->SecConfig.AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				/* wdev->AuthMode = Ndis802_11AuthModeWPAPSK; */
				SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
				break;

			default:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("UNKNOWN AKM 0x%x IN WPA,please check!\n", *pTmp));
				break;
			}

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AuthMode = 0x%x\n", wdev->SecConfig.AKMMap));

			/* if (wdev->GroupKeyWepStatus == PairCipher) */
			if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
				(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
			   ) {
				/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
				/* pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus; */
				wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
			} else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WPA Mix TKIPAES\n"));
				bMix = TRUE;
			}

			pMbss->RSNIE_Len[0] = wpa_ie[1];
			os_move_mem(pMbss->RSN_IE[0], wpa_ie + 2, wpa_ie[1]); /* copy rsn ie */
		} else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: wpa open/none case\n", __func__));
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* wait until wpa/wpa2 all not exist , then set open/none */
		}
	}

	if (rsn_ie != NULL) {
		PRSN_IE_HEADER_STRUCT			pRsnHeader;
		PCIPHER_SUITE_STRUCT			pCipher;

		pEid = (PEID_STRUCT)rsn_ie;
		pTmp = (PUCHAR)pEid;
		pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

		/* 0. Version must be 1*/
		if (le2cpu16(pRsnHeader->Version) == 1) {
			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			/* 1. Check group cipher*/
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (os_equal_mem(pTmp, RSN_OUI, 3)) {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: WPA2 case\n", __func__));
				bWPA2 = TRUE;

				/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
				/* SET_AKM_OPEN(wdev->SecConfig.AKMMap); */
				switch (pCipher->Type) {
				case 1:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11GroupWEP40Enabled\n"));
					/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP40Enabled; */
					SET_CIPHER_WEP40(wdev->SecConfig.GroupCipher);
					break;

				case 5:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11GroupWEP104Enabled\n"));
					/* wdev->GroupKeyWepStatus  = Ndis802_11GroupWEP104Enabled; */
					SET_CIPHER_WEP104(wdev->SecConfig.GroupCipher);
					break;

				case 2:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Group Ndis802_11TKIPEnable\n"));
					/* wdev->GroupKeyWepStatus  = Ndis802_11TKIPEnable; */
					SET_CIPHER_TKIP(wdev->SecConfig.GroupCipher);
					break;

				case 4:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Group Ndis802_11AESEnable\n"));
					/* wdev->GroupKeyWepStatus  = Ndis802_11AESEnable; */
					SET_CIPHER_CCMP128(wdev->SecConfig.GroupCipher);
					break;

				default:
					break;
				}

				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);
				/* 2. Get pairwise cipher counts*/
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1] << 8) + pTmp[0];
				pTmp   += sizeof(USHORT);

				/* 3. Get pairwise cipher*/
				/* Parsing all unicast cipher suite*/
				while (Count > 0) {
					/* Skip OUI*/
					pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
					TmpCipher = Ndis802_11WEPDisabled;

					switch (pCipher->Type) {
					case 1:
					case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
						TmpCipher = Ndis802_11WEPEnabled;
						break;

					case 2:
						TmpCipher = Ndis802_11TKIPEnable;
						break;

					case 4:
						TmpCipher = Ndis802_11AESEnable;
						break;

					default:
						break;
					}

					/* pMbss->wdev.WepStatus = TmpCipher; */
					if (TmpCipher > PairCipher) {
						/* Move the lower cipher suite to PairCipherAux*/
						PairCipherAux = PairCipher;
						PairCipher	 = TmpCipher;
					} else
						PairCipherAux = TmpCipher;

					pTmp += sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				/* 4. get AKM suite counts*/
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
						/* wdev->AuthMode = Ndis802_11AuthModeWPANone; */
						SET_AKM_OPEN(wdev->SecConfig.AKMMap);
						break;

					case 1:
						/* Set AP support WPA-enterprise mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2; */
						SET_AKM_WPA2(wdev->SecConfig.AKMMap);
						break;

					case 2:
						/* Set AP support WPA-PSK mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2PSK; */
						SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
						break;

					default:
						/* wdev->AuthMode = Ndis802_11AuthModeMax; */
						SET_AKM_OPEN(wdev->SecConfig.AKMMap);
						break;
					}

					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AuthMode = 0x%x\n", wdev->SecConfig.AKMMap));

				if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher))
				   ) {
					/* wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE; */
					/* pMbss->wdev.WepStatus=wdev->GroupKeyWepStatus; */
					wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
				} else {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WPA2 Mix TKIPAES\n"));
					bMix = TRUE;
				}

				if (bWPA2 && bWPA) {
					pMbss->RSNIE_Len[1] = rsn_ie[1];
					NdisMoveMemory(pMbss->RSN_IE[1], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
				} else {
					pMbss->RSNIE_Len[0] = rsn_ie[1];
					os_move_mem(pMbss->RSN_IE[0], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
				}
			} else {
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: wpa2 Open/None case\n", __func__));
				/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
				/* wait until wpa/wpa2 all not exist , then set open/none */
			}
		}
	}

	if (bWPA2 && bWPA) {
		/* wdev->AuthMode = Ndis802_11AuthModeWPA1PSKWPA2PSK; */
		SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
		SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);

		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else if (bWPA2) {
		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else if (bWPA) {
		if (bMix) {
			/* wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE; */
			/* wdev->WepStatus = Ndis802_11TKIPAESMix; */
			SET_AKM_WPA1PSK(wdev->SecConfig.AKMMap);
			SET_CIPHER_TKIP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher);
		}
	} else {
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (IS_AKM_WPA1(wdev->SecConfig.AKMMap) || IS_AKM_WPA2(wdev->SecConfig.AKMMap))
		wdev->SecConfig.IEEE8021X = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\nCFG80211 BEACON => bwpa2 %d, bwpa %d, bmix %d,AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
			  , bWPA2, bWPA, bMix
			  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher)));
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

