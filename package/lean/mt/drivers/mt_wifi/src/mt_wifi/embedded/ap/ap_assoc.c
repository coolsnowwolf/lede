/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    assoc.c

    Abstract:
    Handle association related requests either from WSTA or from local MLME

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"

#ifdef VENDOR_FEATURE7_SUPPORT
#include "arris_wps_gpio_handler.h"
#endif
extern UCHAR	CISCO_OUI[];
extern UCHAR	WPA_OUI[];
extern UCHAR	RSN_OUI[];
extern UCHAR	WME_INFO_ELEM[];
extern UCHAR	WME_PARM_ELEM[];
extern UCHAR	RALINK_OUI[];
extern UCHAR BROADCOM_OUI[];





static USHORT update_associated_mac_entry(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRate,
	IN BOOLEAN isReassoc)
{
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
#ifdef TXBF_SUPPORT
	BOOLEAN	 supportsETxBF = FALSE;
#endif /* TXBF_SUPPORT // */
	STA_TR_ENTRY *tr_entry;
	UCHAR PhyMode;
	UCHAR Channel;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	BOOLEAN need_clr_set_wtbl = FALSE;

	ASSERT((pEntry->func_tb_idx < pAd->ApCfg.BssidNum));
	mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	wdev = &mbss->wdev;
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	PhyMode = wdev->PhyMode;
	Channel = wdev->channel;

	/* use this to decide reassociation rsp need to clear set wtbl or not */
	if (pEntry->Sst == SST_ASSOC && isReassoc == TRUE
#ifdef DOT11R_FT_SUPPORT
			&& (IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
								)
	need_clr_set_wtbl = TRUE;

	/* Update auth, wep, legacy transmit rate setting . */
	pEntry->Sst = SST_ASSOC;
	pEntry->MaxSupportedRate = min(wdev->rate.MaxTxRate, MaxSupportedRate);
	MacTableSetEntryPhyCfg(pAd, pEntry);
	pEntry->CapabilityInfo = ie_list->CapabilityInfo;

	if (IS_AKM_PSK_Entry(pEntry)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
	}

#ifdef DOT1X_SUPPORT
	else if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev)) {
		pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
	}

#endif /* DOT1X_SUPPORT */
	/* Ralink proprietary Piggyback and Aggregation support for legacy RT61 chip */
	MacTableSetEntryRaCap(pAd, pEntry, &ie_list->vendor_ie);

	/* In WPA or 802.1x mode, the port is not secured, otherwise is secued. */
	if (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
		|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
		|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
#ifdef WSC_INCLUDED
		|| (pEntry->bWscCapable && IS_AKM_WPA_CAPABILITY_Entry(wdev))
#endif /* WSC_INCLUDED */
	   )
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	else
		tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef SOFT_ENCRYPT
	/* There are some situation to need to encryption by software
	   1. The Client support PMF. It shall ony support AES cipher.
	   2. The Client support WAPI.
	   If use RT3883 or later, HW can handle the above.
	   */
#ifdef DOT11W_PMF_SUPPORT

	if ((cap->FlgPMFEncrtptMode == PMF_ENCRYPT_MODE_0)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE))
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT);

#endif /* DOT11W_PMF_SUPPORT */
#endif /* SOFT_ENCRYPT */

	if (wdev->bWmmCapable && ie_list->bWmmCapable)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	else
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

#ifdef DOT11_N_SUPPORT

	/*
		WFA recommend to restrict the encryption type in 11n-HT mode.
		So, the WEP and TKIP are not allowed in HT rate.
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP &&
		IS_INVALID_HT_SECURITY(pEntry->SecConfig.PairwiseCipher)) {
		/* Force to None-HT mode due to WiFi 11n policy */
		ie_list->ht_cap_len = 0;
#ifdef DOT11_VHT_AC
		ie_list->vht_cap_len = 0;
#endif /* DOT11_VHT_AC */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s : Force the STA as Non-HT mode\n", __func__));
	}

#ifdef CONFIG_HOTSPOT_R2

		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;

#endif
#ifdef CONFIG_DOT11V_WNM

		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance. */
		if (ie_list->ExtCapInfo.BssTransitionManmt)
			pEntry->BssTransitionManmtSupport = 1;

		/* #endif CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if ((ie_list->ht_cap_len != 0) &&
		(wdev->DesiredHtPhyInfo.bHtEnable) &&
		WMODE_CAP_N(PhyMode)) {
		ht_mode_adjust(pAd, pEntry, &ie_list->HTCapability);

#ifdef DOT11N_DRAFT3
		if (ie_list->ExtCapInfo.BssCoexistMgmtSupport)
			pEntry->BSS2040CoexistenceMgmtSupport = 1;
#endif /* DOT11N_DRAFT3 */


		/* 40Mhz BSS Width Trigger events */
		if (ie_list->HTCapability.HtCapInfo.Forty_Mhz_Intolerant) {
#ifdef DOT11N_DRAFT3
			UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
			UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
			UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

			pEntry->bForty_Mhz_Intolerant = TRUE;
			pAd->MacTab.fAnyStaFortyIntolerant = TRUE;

			if (((cfg_ht_bw == HT_BW_40) &&
				 (Channel <= 14)) &&
				((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
				 (op_ht_bw != HT_BW_20) &&
				 (op_ext_cha != 0))
			   ) {
				pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
				wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
				pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
				MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("%s, Update Beacon for mbss_idx:%d\n", __func__, pEntry->func_tb_idx));
				UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("pEntry set 40MHz Intolerant as 1\n"));
#endif /* DOT11N_DRAFT3 */
			Handle_BSS_Width_Trigger_Events(pAd, Channel);
		}

#ifdef TXBF_SUPPORT
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(PhyMode) &&
			(Channel > 14) &&
			(ie_list->vht_cap_len))
			supportsETxBF = mt_WrapClientSupportsVhtETxBF(pAd, &ie_list->vht_cap.vht_cap);
		else
#endif /* DOT11_VHT_AC */
			supportsETxBF = mt_WrapClientSupportsETxBF(pAd, &ie_list->HTCapability.TxBFCap);

#endif /* TXBF_SUPPORT */
		/* find max fixed rate */
		pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
										 &ie_list->HTCapability.MCSSet[0]);

		if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("@@@ IF-ra%d DesiredTransmitSetting.field.MCS = %d\n",
					  pEntry->func_tb_idx,
					  wdev->DesiredTransmitSetting.field.MCS));
			set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
		}

		set_sta_ht_cap(pAd, pEntry, &ie_list->HTCapability);
		/* Record the received capability from association request */
		NdisMoveMemory(&pEntry->HTCapability, &ie_list->HTCapability, sizeof(HT_CAPABILITY_IE));
#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(PhyMode) &&
			(Channel > 14) &&
			ie_list->vht_cap_len) {
			vht_mode_adjust(pAd, pEntry, &ie_list->vht_cap,
					(ie_list->vht_op_len == 0) ? NULL :  &ie_list->vht_op);
			dot11_vht_mcs_to_internal_mcs(pAd, wdev, &ie_list->vht_cap, &pEntry->MaxHTPhyMode);

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s(): Peer's PhyCap=>Mode:%s, BW:%s, NSS:%d, MCS:%d\n",
					  __func__,
					  get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
					  get_bw_str(pEntry->MaxHTPhyMode.field.BW),
					  ((pEntry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1,
					  (pEntry->MaxHTPhyMode.field.MCS & 0xf)));

			set_vht_cap(pAd, pEntry, &ie_list->vht_cap);
			NdisMoveMemory(&pEntry->vht_cap_ie, &ie_list->vht_cap, sizeof(VHT_CAP_IE));
		}

		if (ie_list->operating_mode_len == sizeof(OPERATING_MODE) &&
			ie_list->operating_mode.rx_nss_type == 0) {
			pEntry->operating_mode = ie_list->operating_mode;
			pEntry->force_op_mode = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s(): Peer's OperatingMode=>RxNssType: %d, RxNss: %d, ChBW: %d\n",
					  __func__, pEntry->operating_mode.rx_nss_type,
					  pEntry->operating_mode.rx_nss,
					  pEntry->operating_mode.ch_width));
		} else
			pEntry->force_op_mode = FALSE;

#endif /* DOT11_VHT_AC */
	} else {
#ifdef CONFIG_HOTSPOT_R2

		if (ie_list->ExtCapInfo.qosmap)
			pEntry->QosMapSupport = 1;

#endif
		pAd->MacTab.fAnyStationIsLegacy = TRUE;
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		pEntry->SupportHTMCS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_HT_MODE);
#ifdef DOT11_VHT_AC
		/* TODO: shiang-usw, it's ugly and need to revise it */
		NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));
		pEntry->SupportVHTMCS1SS = 0;
		pEntry->SupportVHTMCS2SS = 0;
		pEntry->SupportVHTMCS3SS = 0;
		pEntry->SupportVHTMCS4SS = 0;
		pEntry->SupportRateMode &= (~SUPPORT_VHT_MODE);
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
		pEntry->CurrTxRate = pEntry->MaxSupportedRate;

#ifdef MFB_SUPPORT
	pEntry->lastLegalMfb = 0;
	pEntry->isMfbChanged = FALSE;
	pEntry->fLastChangeAccordingMfb = FALSE;
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = 0;/*has to increment whenever a mrq is sent */
	pEntry->mrqCnt = 0;
	pEntry->pendingMfsi = 0;
	pEntry->toTxMfb = FALSE;
	pEntry->mfbToTx = 0;
	pEntry->mfb0 = 0;
	pEntry->mfb1 = 0;
#endif	/* MFB_SUPPORT */
	pEntry->freqOffsetValid = FALSE;
#ifdef TXBF_SUPPORT

	if (cap->FlgHwTxBfCap)
		chip_tx_bf_init(pAd, pEntry, ie_list, supportsETxBF);

#endif /* TXBF_SUPPORT // */
#ifdef MT_MAC

	if (cap->hif_type == HIF_MT) {
		if (wdev->bAutoTxRateSwitch == TRUE)
			pEntry->bAutoTxRateSwitch = TRUE;
		else {
			pEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
			pEntry->bAutoTxRateSwitch = FALSE;
#ifdef WFA_VHT_PF

			if (WMODE_CAP_AC(PhyMode)) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

#endif /* WFA_VHT_PF */

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS +
											  ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
			}

			/* If the legacy mode is set, overwrite the transmit setting of this entry. */
			RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
		}

#if !defined(MT7615) && !defined(MT7637) && !defined(MT7622) && !defined(P18) && !defined(MT7663)

		if (!IS_MT7615(pAd) && !IS_MT7637(pAd) && !IS_MT7622(pAd) && !IS_P18(pAd) && !IS_MT7663(pAd))
			RAInit(pAd, pEntry);

#endif /* !defined(MT7615) && !defined(MT7637) && !defined(MT7622) && !defined(P18) && !defined(MT7663) */
	}

#endif /* MT_MAC */

	if (IS_NO_SECURITY(&pEntry->SecConfig) || IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher))
		ApLogEvent(pAd, pEntry->Addr, EVENT_ASSOCIATED);


	if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) ||
		(pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
		pAd->MacTab.fAnyStationIsLegacy = TRUE;

	NdisAcquireSpinLock(&pAd->MacTabLock);
	nonerp_sta_num(pEntry, PEER_JOIN);
	NdisReleaseSpinLock(&pAd->MacTabLock);

	ApUpdateCapabilityAndErpIe(pAd, mbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
#ifdef HOSTAPD_SUPPORT

	if ((wdev->Hostapd == Hostapd_EXT) &&
		((wdev->AuthMode >= Ndis802_11AuthModeWPA) || wdev->IEEE8021X)) {
		RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED,
								   -1, pEntry->Addr, NULL, 0,
								   ((pEntry->CapabilityInfo & 0x0010) == 0 ? 0xFFFD : 0xFFFC));
	}

#endif /*HOSTAPD_SUPPORT*/
	return MLME_SUCCESS;
}

/*
    ==========================================================================
    Description:
       assign a new AID to the newly associated/re-associated STA and
       decide its MaxSupportedRate and CurrTxRate. Both rates should not
       exceed AP's capapbility
    Return:
       MLME_SUCCESS - association successfully built
       others - association failed due to resource issue
    ==========================================================================
 */
static USHORT APBuildAssociation(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRateIn500Kbps,
	OUT USHORT * pAid,
	IN BOOLEAN isReassoc)
{
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate = RATE_11;
	struct wifi_dev *wdev;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	STA_TR_ENTRY *tr_entry;
#ifdef CONFIG_OWE_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /*CONFIG_OWE_SUPPORT*/

	if (!pEntry)
		return MLME_UNSPECIFY_FAIL;

	wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);

	if ((WMODE_EQUAL(wdev->PhyMode, WMODE_G)
#ifdef DOT11_N_SUPPORT
		 || WMODE_EQUAL(wdev->PhyMode, (WMODE_G | WMODE_GN))
#endif /* DOT11_N_SUPPORT */
		)
		&& (MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	   )
		return MLME_ASSOC_REJ_DATA_RATE;


#ifdef DOT11_N_SUPPORT

	/* 11n only */
	if (WMODE_HT_ONLY(wdev->PhyMode) && (ie_list->ht_cap_len == 0))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode) &&
		(wdev->channel > 14) &&
		(ie_list->vht_cap_len == 0) &&
		(pAd->CommonCfg.bNonVhtDisallow))
		return MLME_ASSOC_REJ_DATA_RATE;

#endif /* DOT11_VHT_AC */

	if ((pEntry->Sst == SST_AUTH) || (pEntry->Sst == SST_ASSOC)) {
		/* TODO:
			should qualify other parameters, for example -
			capablity, supported rates, listen interval, etc., to
			decide the Status Code
		*/
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		*pAid = pEntry->Aid;
		pEntry->NoDataIdleCount = 0;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
		pEntry->StaConnectTime = 0;
#ifdef CONFIG_HOTSPOT_R2

		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE))
#endif
		{
#ifdef WSC_AP_SUPPORT

			if (pEntry->bWscCapable == FALSE)
#endif /* WSC_AP_SUPPORT */
			{
				/* check the validity of the received RSNIE */
				StatusCode = WPAValidateRSNIE(&wdev->SecConfig,
							      &pEntry->SecConfig,
							      &ie_list->RSN_IE[0],
							      ie_list->RSNIE_Len);
#ifdef CONFIG_OWE_SUPPORT
				StatusCode = owe_pmkid_ecdh_process(pAd,
								    pEntry,
								    StatusCode,
								    ie_list->RSN_IE,
								    ie_list->RSNIE_Len,
								    &ie_list->ecdh_ie,
								    ie_list->ecdh_ie.length,
								    pPmkid,
								    &pmkid_count,
								    SUBTYPE_ASSOC_REQ);
#endif /*CONFIG_OWE_SUPPORT*/

				if (StatusCode != MLME_SUCCESS) {
					/* send wireless event - for RSN IE sanity check fail */
					RTMPSendWirelessEvent(pAd, IW_RSNIE_SANITY_FAIL_EVENT_FLAG, pEntry->Addr, 0, 0);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : invalid status code(%d) !!!\n", __func__, StatusCode));
					return StatusCode;
				}

				if (pEntry->SecConfig.AKMMap == 0x0) {
					SET_AKM_OPEN(pEntry->SecConfig.AKMMap);
					SET_CIPHER_NONE(pEntry->SecConfig.PairwiseCipher);
					SET_CIPHER_NONE(pEntry->SecConfig.GroupCipher);
				}

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : (AID#%d AKM=0x%x, PairwiseCipher=0x%x)\n", __func__,
						 pEntry->Aid, pEntry->SecConfig.AKMMap, pEntry->SecConfig.PairwiseCipher));
			}
		}

		NdisMoveMemory(pEntry->RSN_IE, &ie_list->RSN_IE[0], ie_list->RSNIE_Len);
		pEntry->RSNIE_Len = ie_list->RSNIE_Len;

		if (*pAid == 0)
			StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		else if ((pEntry->RSNIE_Len == 0) &&
				 (IS_AKM_WPA_CAPABILITY_Entry(pEntry))
#ifdef HOSTAPD_SUPPORT
				 && (wdev->Hostapd == Hostapd_EXT)
#endif /* HOSTAPD_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				 && (wdev->Hostapd == Hostapd_CFG)
#endif /*RT_CFG80211_SUPPORT*/
				) {
#ifdef WSC_AP_SUPPORT
			wsc_ctrl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl;

			if (((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				 pEntry->bWscCapable
#ifdef WSC_V2_SUPPORT
				 && (wsc_ctrl->WscV2Info.bWpsEnable ||
					 (wsc_ctrl->WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
				)
#ifdef HOSTAPD_SUPPORT
				|| (wdev->Hostapd == Hostapd_EXT)
#endif /*HOSTAPD_SUPPORT*/
#ifdef RT_CFG80211_SUPPORT
				|| wdev->Hostapd == Hostapd_CFG
#endif /*RT_CFG80211_SUPPORT*/
			   ) {
				pEntry->Sst = SST_ASSOC;
				StatusCode = MLME_SUCCESS;

				/* In WPA or 802.1x mode, the port is not secured. */
				if (IS_AKM_WPA_CAPABILITY(pEntry->SecConfig.AKMMap)
#ifdef DOT1X_SUPPORT
					|| IS_IEEE8021X_Entry(wdev)
#endif /* DOT1X_SUPPORT */
				   )
					tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

				if (IS_AKM_PSK(pEntry->SecConfig.AKMMap)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
				}

#ifdef DOT1X_SUPPORT
				else if (IS_AKM_1X(pEntry->SecConfig.AKMMap) || IS_IEEE8021X_Entry(wdev)) {
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
					pEntry->SecConfig.Handshake.WpaState = AS_AUTHENTICATION;
				}

#endif /* DOT1X_SUPPORT */
			} else {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("ASSOC - WSC_STATE_MACHINE is OFF.<WscConfMode = %d, apidx =%d>\n",
						  wsc_ctrl->WscConfMode, pEntry->func_tb_idx));
				StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
			}

#else  /* WSC_AP_SUPPORT */
			StatusCode = MLME_ASSOC_DENY_OUT_SCOPE;
#endif /* WSC_AP_SUPPORT */
#ifdef HOSTAPD_SUPPORT

			if (wdev->Hostapd == Hostapd_EXT
				&& (wdev->AuthMode >= Ndis802_11AuthModeWPA
					|| wdev->IEEE8021X)) {
				RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED,
										   -1, pEntry->Addr, NULL, 0,
										   ((pEntry->CapabilityInfo & 0x0010) == 0 ? 0xFFFD : 0xFFFC));
			}

#endif /*HOSTAPD_SUPPORT*/
		} else
			StatusCode = update_associated_mac_entry(pAd, pEntry, ie_list, MaxSupportedRate, isReassoc);
	} else /* CLASS 3 error should have been handled beforehand; here should be MAC table full */
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;


	if (StatusCode == MLME_SUCCESS) {

	}

	return StatusCode;
}


#ifdef IAPP_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Update Frame to update forwarding table in Layer 2 devices.

 Arguments:
    *mac_p - the STATION MAC address pointer

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx)
{
	NDIS_PACKET	 *pNetBuf;
	struct wifi_dev *wdev;

	wdev = pAd->wdev_list[wdev_idx];
	pNetBuf = RtmpOsPktIappMakeUp(get_netdev_from_bssid(pAd, wdev_idx), mac);

	if (pNetBuf == NULL)
		return FALSE;

	/*Which band is this packet transfer*/
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_needed_hook != NULL && wf_drv_tbl.wf_fwd_needed_hook() == TRUE)
		set_wf_fwd_cb(pAd, pNetBuf, wdev);

#endif /* CONFIG_WIFI_PKT_FWD */
	/* UCOS: update the built-in bridge, too (don't use gmac.xmit()) */
	announce_802_3_packet(pAd, pNetBuf, OPMODE_AP);
	IAPP_L2_UpdatePostCtrl(pAd, mac, wdev_idx);
	return TRUE;
} /* End of IAPP_L2_Update_Frame_Send */
#endif /* IAPP_SUPPORT */


/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
BOOLEAN PeerAssocReqCmmSanity(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS *ie_lists)
{
	CHAR *Ptr;
	PFRAME_802_11	Fr = (PFRAME_802_11)Msg;
	PEID_STRUCT eid_ptr;
	UCHAR Sanity = 0;
	UCHAR WPA1_OUI[4] = { 0x00, 0x50, 0xF2, 0x01 };
#ifdef CONFIG_HOTSPOT_R2
	UCHAR HS2_OSEN_OUI[4] = { 0x50, 0x6f, 0x9a, 0x12 };
	UCHAR HS2OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x10};
#endif
#ifdef MBO_SUPPORT
	UCHAR MBO_OCE_OUI_BYTE[4] = {0x50, 0x6f, 0x9a, 0x16};
#endif /* MBO_SUPPORT */

	MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)NULL;
#ifdef DOT11R_FT_SUPPORT
	FT_INFO *pFtInfo = &ie_lists->FtInfo;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE *pRrmEnCap = &ie_lists->RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */
	HT_CAPABILITY_IE *pHtCapability = &ie_lists->HTCapability;
	UCHAR i;

	pEntry = MacTableLookup(pAd, &Fr->Hdr.Addr2[0]);

	if (pEntry == NULL)
		return FALSE;

	COPY_MAC_ADDR(&ie_lists->Addr1[0], &Fr->Hdr.Addr1[0]);
	COPY_MAC_ADDR(&ie_lists->Addr2[0], &Fr->Hdr.Addr2[0]);
	Ptr = (PCHAR)Fr->Octet;
	NdisMoveMemory(&ie_lists->CapabilityInfo, &Fr->Octet[0], 2);
	NdisMoveMemory(&ie_lists->ListenInterval, &Fr->Octet[2], 2);

	if (isReassoc) {
		NdisMoveMemory(&ie_lists->ApAddr[0], &Fr->Octet[4], 6);
		eid_ptr = (PEID_STRUCT) &Fr->Octet[10];
	} else
		eid_ptr = (PEID_STRUCT) &Fr->Octet[4];

	/* get variable fields from payload and advance the pointer */
	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_SSID:
			if (((Sanity & 0x1) == 1))
				break;

			if (eid_ptr->Len <= MAX_LEN_OF_SSID) {
				Sanity |= 0x01;
				NdisMoveMemory(&ie_lists->Ssid[0], eid_ptr->Octet, eid_ptr->Len);
				ie_lists->SsidLen = eid_ptr->Len;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("PeerAssocReqSanity - SsidLen = %d\n", ie_lists->SsidLen));
			} else {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAssocReqSanity - wrong IE_SSID\n"));
				return FALSE;
			}

			break;

		case IE_SUPP_RATES:
			if ((eid_ptr->Len <= MAX_LEN_OF_SUPPORTED_RATES) &&
				(eid_ptr->Len > 0)) {
				Sanity |= 0x02;
				NdisMoveMemory(&ie_lists->SupportedRates[0], eid_ptr->Octet, eid_ptr->Len);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("PeerAssocReqSanity - IE_SUPP_RATES., Len=%d. Rates[0]=%x\n"
						  , eid_ptr->Len, ie_lists->SupportedRates[0]));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("Rates[1]=%x %x %x %x %x %x %x\n",
						  ie_lists->SupportedRates[1], ie_lists->SupportedRates[2],
						  ie_lists->SupportedRates[3], ie_lists->SupportedRates[4],
						  ie_lists->SupportedRates[5], ie_lists->SupportedRates[6],
						  ie_lists->SupportedRates[7]));
				ie_lists->SupportedRatesLen = eid_ptr->Len;
			} else {
				UCHAR RateDefault[8] = { 0x82, 0x84, 0x8b, 0x96, 0x12, 0x24, 0x48, 0x6c };
				/* HT rate not ready yet. return true temporarily. rt2860c */
				/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\n")); */
				Sanity |= 0x02;
				ie_lists->SupportedRatesLen = 8;
				NdisMoveMemory(&ie_lists->SupportedRates[0], RateDefault, 8);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("PeerAssocReqSanity - wrong IE_SUPP_RATES., Len=%d\n",
						  eid_ptr->Len));
			}

			break;

		case IE_EXT_SUPP_RATES:
			if (eid_ptr->Len + ie_lists->SupportedRatesLen <= MAX_LEN_OF_SUPPORTED_RATES) {
				UINT32 _RateIdx = ie_lists->SupportedRatesLen %
								  MAX_LEN_OF_SUPPORTED_RATES;
				NdisMoveMemory(&ie_lists->SupportedRates[_RateIdx], eid_ptr->Octet,
							   eid_ptr->Len);
				ie_lists->SupportedRatesLen += eid_ptr->Len;
			} else {
				NdisMoveMemory(&ie_lists->SupportedRates[ie_lists->SupportedRatesLen], eid_ptr->Octet,
							   MAX_LEN_OF_SUPPORTED_RATES - (ie_lists->SupportedRatesLen));
				ie_lists->SupportedRatesLen = MAX_LEN_OF_SUPPORTED_RATES;
			}

			break;

		case IE_HT_CAP:
			if (eid_ptr->Len >= sizeof(HT_CAPABILITY_IE)) {
				NdisMoveMemory(pHtCapability, eid_ptr->Octet, SIZE_HT_CAP_IE);
				*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
				{
					EXT_HT_CAP_INFO extHtCapInfo;

					NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
					*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
					NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				}
#else
				*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#ifdef RT_BIG_ENDIAN
				*(USHORT *)(&pHtCapability->TxBFCap) = le2cpu32(*(USHORT *)(&pHtCapability->TxBFCap));
#endif
				ie_lists->ht_cap_len = SIZE_HT_CAP_IE;
				Sanity |= 0x10;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("PeerAssocReqSanity - IE_HT_CAP\n"));
			} else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("PeerAssocReqSanity - wrong IE_HT_CAP.eid_ptr->Len = %d\n",
						 eid_ptr->Len));

			break;

		case IE_EXT_CAPABILITY:
			if (eid_ptr->Len) {
				INT ext_len = eid_ptr->Len;
#ifdef RT_BIG_ENDIAN
				UCHAR *pextCapInfo = NULL;
#endif
				ext_len = ext_len > sizeof(EXT_CAP_INFO_ELEMENT) ? sizeof(EXT_CAP_INFO_ELEMENT) : ext_len;
				NdisMoveMemory(&ie_lists->ExtCapInfo, eid_ptr->Octet, ext_len);
#ifdef RT_BIG_ENDIAN
				pextCapInfo = (UCHAR *)&ie_lists->ExtCapInfo;
				*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
				*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("PeerAssocReqSanity - IE_EXT_CAPABILITY!\n"));
			}

			break;

		case IE_WPA:    /* same as IE_VENDOR_SPECIFIC */
#ifdef MBO_SUPPORT
			if (NdisEqualMemory(eid_ptr->Octet, MBO_OCE_OUI_BYTE, sizeof(MBO_OCE_OUI_BYTE)) && (eid_ptr->Len >= 5)) {
				MboParseStaMboIE(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev, pEntry, eid_ptr->Octet, eid_ptr->Len, MBO_FRAME_TYPE_ASSOC_REQ);
				break;
			}
#endif /* MBO_SUPPORT */


		case IE_WPA2:
#ifdef DOT11R_FT_SUPPORT
#endif /* DOT11R_FT_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
			if (NdisEqualMemory(eid_ptr->Octet, HS2OUIBYTE, sizeof(HS2OUIBYTE)) && (eid_ptr->Len >= 5)) {
				/* UCHAR tmp2 = 0x12; */
				UCHAR *hs2_config = (UCHAR *)&eid_ptr->Octet[4];
				UCHAR ppomo_exist = ((*hs2_config) >> 1) & 0x01;
				UCHAR hs2_version = ((*hs2_config) >> 4) & 0x0f;
				/* UCHAR *tmp3 = (UCHAR *)&pEntry->hs_info.ppsmo_id; */
				/* UCHAR tmp[2] = {0x12,0x34}; */
				pEntry->hs_info.version = hs2_version;
				pEntry->hs_info.ppsmo_exist = ppomo_exist;

				if (pEntry->hs_info.ppsmo_exist) {
					NdisMoveMemory(&pEntry->hs_info.ppsmo_id, &eid_ptr->Octet[5], 2);
					/* NdisMoveMemory(tmp3, tmp, 2); */
				}

				break;
			}

#endif /* CONFIG_HOTSPOT_R2 */


			if (NdisEqualMemory(eid_ptr->Octet, WPS_OUI, 4)) {
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

				if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bWpsEnable) ||
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.WscV2Info.bEnableWpsV2 == FALSE))
#endif /* WSC_V2_SUPPORT */
					ie_lists->bWscCapable = TRUE;

#ifdef RT_CFG80211_SUPPORT
				ie_lists->bWscCapable = TRUE;
#endif
#endif /* WSC_AP_SUPPORT */
				break;
			}

			/* Handle Atheros and Broadcom draft 11n STAs */
			if (NdisEqualMemory(eid_ptr->Octet, BROADCOM_OUI, 3)) {
				switch (eid_ptr->Octet[3]) {
				case 0x33:
					if ((eid_ptr->Len - 4) == sizeof(HT_CAPABILITY_IE)) {
						NdisMoveMemory(pHtCapability, &eid_ptr->Octet[4], SIZE_HT_CAP_IE);
						*(USHORT *)(&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
						{
							EXT_HT_CAP_INFO extHtCapInfo;

							NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&pHtCapability->ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
							*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
							NdisMoveMemory((PUCHAR)(&pHtCapability->ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
						}
#else
						*(USHORT *)(&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
						ie_lists->ht_cap_len = SIZE_HT_CAP_IE;
					}

					break;

				default:
					/* ignore other cases */
					break;
				}
			}

			check_vendor_ie(pAd, (UCHAR *)eid_ptr, &(ie_lists->vendor_ie));

			/* WMM_IE */
			if (NdisEqualMemory(eid_ptr->Octet, WME_INFO_ELEM, 6) && (eid_ptr->Len == 7)) {
				ie_lists->bWmmCapable = TRUE;
#ifdef UAPSD_SUPPORT

				if (pEntry) {
					UAPSD_AssocParse(pAd,
									 pEntry, (UINT8 *)&eid_ptr->Octet[6],
									 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.UapsdInfo.bAPSDCapable);
				}

#endif /* UAPSD_SUPPORT */
				break;
			}

			if (IS_NO_SECURITY(&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig)
				|| IS_CIPHER_WEP(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseCipher))
				break;

			/*	If this IE did not begins with 00:0x50:0xf2:0x01,
				it would be proprietary. So we ignore it. */
			if (!NdisEqualMemory(eid_ptr->Octet, WPA1_OUI, sizeof(WPA1_OUI))
				&& !(eid_ptr->Eid == IE_WPA2)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Not RSN IE, maybe WMM IE!!!\n"));
#ifdef CONFIG_HOTSPOT_R2

				if (!NdisEqualMemory(eid_ptr->Octet, HS2_OSEN_OUI, sizeof(HS2_OSEN_OUI))) {
					unsigned char *tmp = (unsigned char *)eid_ptr->Octet;

					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!!!!!not found OSEN IE,%x:%x:%x:%x\n", *tmp, *(tmp + 1),
							 *(tmp + 2), *(tmp + 3)));
					pEntry->OSEN_IE_Len = 0;
					CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
					break;
				}

				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_OSEN_CAPABLE);
				NdisMoveMemory(pEntry->OSEN_IE, eid_ptr, eid_ptr->Len + 2);
				pEntry->OSEN_IE_Len = eid_ptr->Len + 2;
				SET_AKM_WPA2(pEntry->SecConfig.AKMMap);
				SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
				SET_CIPHER_CCMP128(pEntry->SecConfig.GroupCipher);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!!!!!found OSEN IE Entry AuthMode %s, EncryptType %s\n",
						 GetAuthModeStr(pEntry->SecConfig.AKMMap), GetEncryModeStr(pEntry->SecConfig.PairwiseCipher)));
#else
				break;
#endif
			}

			/* Copy whole RSNIE context */
			NdisMoveMemory(&ie_lists->RSN_IE[0], eid_ptr, eid_ptr->Len + 2);
			ie_lists->RSNIE_Len = eid_ptr->Len + 2;
#ifdef DOT11R_FT_SUPPORT
			NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
			pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
#endif /* DOT11R_FT_SUPPORT */

			break;
#ifdef DOT11R_FT_SUPPORT

		case IE_FT_MDIE:
			FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
			break;

		case IE_FT_FTIE:
			FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
			break;

		case IE_FT_RIC_DATA:

			/* record the pointer of first RDIE. */
			if (pFtInfo->RicInfo.pRicInfo == NULL) {
				pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
				pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen) - (UCHAR *)eid_ptr + 1;
			}
			break;

		case IE_FT_RIC_DESCRIPTOR:
			if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
				NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
							   &eid_ptr->Eid, eid_ptr->Len + 2);
				pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
			}

			break;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

		case IE_RRM_EN_CAP: {
			UINT64 value;

			NdisMoveMemory(&value, eid_ptr->Octet, sizeof(UINT64));
			pRrmEnCap->word = le2cpu64(value);
		}
		break;
#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (eid_ptr->Len == sizeof(VHT_CAP_IE)) {
#ifdef RT_BIG_ENDIAN
				UINT32 tmp_1;
				UINT64 tmp_2;
#endif
				NdisMoveMemory(&ie_lists->vht_cap, eid_ptr->Octet, sizeof(VHT_CAP_IE));
				ie_lists->vht_cap_len = eid_ptr->Len;
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp_1, &ie_lists->vht_cap.vht_cap, 4);
				tmp_1 = le2cpu32(tmp_1);
				NdisCopyMemory(&ie_lists->vht_cap.vht_cap, &tmp_1, 4);

				NdisCopyMemory(&tmp_2, &(ie_lists->vht_cap.mcs_set), 8);
				tmp_2 = le2cpu64(tmp_2);
				NdisCopyMemory(&(ie_lists->vht_cap.mcs_set), &tmp_2, 8);
#endif

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():IE_VHT_CAP\n", __func__));
			} else {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():wrong IE_VHT_CAP, eid->Len = %d\n",
						 __func__, eid_ptr->Len));
			}
			break;

		case IE_VHT_OP:
			if (eid_ptr->Len == sizeof(VHT_OP_IE)) {
#ifdef RT_BIG_ENDIAN
				UINT32 tmp;
#endif
				NdisMoveMemory(&ie_lists->vht_op, eid_ptr->Octet, sizeof(VHT_OP_IE));
				ie_lists->vht_op_len = eid_ptr->Len;
#ifdef RT_BIG_ENDIAN
				NdisCopyMemory(&tmp, &ie_lists->vht_op.basic_mcs_set, sizeof(VHT_MCS_MAP));
				tmp = le2cpu16(tmp);
				NdisCopyMemory(&ie_lists->vht_op.basic_mcs_set, &tmp, sizeof(VHT_MCS_MAP));
#endif

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():IE_VHT_OP\n", __func__));
			}
			break;

		case IE_OPERATING_MODE_NOTIFY:
			if (eid_ptr->Len == sizeof(OPERATING_MODE)) {
				ie_lists->operating_mode_len = sizeof(OPERATING_MODE);
				NdisMoveMemory(&ie_lists->operating_mode, &eid_ptr->Octet[0], sizeof(OPERATING_MODE));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():IE_OPERATING_MODE_NOTIFY!\n", __func__));
			}

			break;
#endif /* DOT11_VHT_AC */

		case IE_SUPP_CHANNELS:
			if (eid_ptr->Len > MAX_LEN_OF_SUPPORTED_CHL || (eid_ptr->Len % 2)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s():wrong IE_SUPP_CHANNELS, eid->Len = %d\n",
						 __func__, eid_ptr->Len));
			} else if (eid_ptr->Len + ie_lists->SupportedChlLen <= MAX_LEN_OF_SUPPORTED_CHL) {
				UINT32 _ChlIdx = ie_lists->SupportedChlLen %
								 MAX_LEN_OF_SUPPORTED_CHL;
				NdisMoveMemory(&ie_lists->SupportedChl[_ChlIdx], eid_ptr->Octet,
							   eid_ptr->Len);
				ie_lists->SupportedChlLen += eid_ptr->Len;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():IE_SUPP_CHANNELS, eid->Len = %d\n",
						 __func__, eid_ptr->Len));
			} else {
				NdisMoveMemory(&ie_lists->SupportedChl[ie_lists->SupportedChlLen], eid_ptr->Octet,
							   MAX_LEN_OF_SUPPORTED_CHL - (ie_lists->SupportedChlLen));
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():IE_SUPP_CHANNELS, eid->Len = %d (Exceeded)\n",
						 __func__, eid_ptr->Len));
			}

			if (ie_lists->SupportedChlLen > MAX_LEN_OF_SUPPORTED_CHL)
				ie_lists->SupportedChlLen = MAX_LEN_OF_SUPPORTED_CHL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("  Supported Channels: [ FirstCh : NumOfCh ]\n"));

			for (i = 0; i < ie_lists->SupportedChlLen; i += 2)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("  [ %4d : %4d ]\n", ie_lists->SupportedChl[i],
						 ie_lists->SupportedChl[i + 1]));

			break;
		case IE_WLAN_EXTENSION:
		{
			/*parse EXTENSION EID*/
			UCHAR *extension_id = (UCHAR *)eid_ptr + 2;

			switch (*extension_id) {
			case IE_EXTENSION_ID_ECDH:
#ifdef CONFIG_OWE_SUPPORT
			{
				UCHAR *ext_ie_length = (UCHAR *)eid_ptr + 1;
				os_zero_mem(ie_lists->ecdh_ie.public_key, *ext_ie_length-3);
				ie_lists->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
				ie_lists->ecdh_ie.length = eid_ptr->Len;
				NdisMoveMemory(&ie_lists->ecdh_ie.ext_id_ecdh, eid_ptr->Octet, eid_ptr->Len);
			}
#endif /*CONFIG_OWE_SUPPORT*/
				break;
			default:
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id));
				break;
			}
		}
			break;
		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}


	if ((Sanity & 0x3) != 0x03) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): - missing mandatory field\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s() - success\n", __func__));
	return TRUE;
}


VOID ap_cmm_peer_assoc_req_action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN BOOLEAN isReassoc)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	BOOLEAN bAssocSkip = FALSE;
	CHAR rssi;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid = 0;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR MaxSupportedRate = 0;
	UCHAR SupRateLen, PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
#ifdef DBG
	UCHAR *sAssoc = isReassoc ? (PUCHAR)"ReASSOC" : (PUCHAR)"ASSOC";
#endif /* DBG */
	UCHAR SubType;
	BOOLEAN bACLReject = FALSE;
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	FT_INFO FtInfoBuf;
#endif /* DOT11R_FT_SUPPORT */
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/
	ADD_HT_INFO_IE *addht;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	struct _build_ie_info ie_info;
#ifdef MBO_SUPPORT
	BOOLEAN bMboReject = FALSE;
#endif /* MBO_SUPPORT */


	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Disallow new Association\n"));
		return;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): mem alloc failed\n", __func__));
		return;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	if (!PeerAssocReqCmmSanity(pAd, isReassoc, Elem->Msg, Elem->MsgLen, ie_list))
		goto LabelOK;

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("NoAuth MAC - %02x:%02x:%02x:%02x:%02x:%02x\n",
				  PRINT_MAC(ie_list->Addr2)));
		goto LabelOK;
	}

	if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():pEntry bounding invalid wdev(apidx=%d)\n",
				  __func__, pEntry->func_tb_idx));
		goto LabelOK;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s():pEntry->func_tb_idx=%d\n",
			  __func__, pEntry->func_tb_idx));
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Wrong Addr1 - %02x:%02x:%02x:%02x:%02x:%02x\n",
				  PRINT_MAC(ie_list->Addr1)));
		goto LabelOK;
	}

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): AP is not ready, disallow new Association\n", __func__));
		goto LabelOK;
	}


	ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;

#ifdef HTC_DECRYPT_IOT

	if ((pEntry->HTC_ICVErrCnt)
		|| (pEntry->HTC_AAD_OM_Force)
		|| (pEntry->HTC_AAD_OM_CountDown)
		|| (pEntry->HTC_AAD_OM_Freeze)
	   ) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("@@@ %s(): (wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u), HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
				  __func__, pEntry->wcid, pEntry->HTC_ICVErrCnt, pEntry->HTC_AAD_OM_Force, pEntry->HTC_AAD_OM_CountDown,
				  pEntry->HTC_AAD_OM_Freeze));
		/* Force clean. */
		pEntry->HTC_ICVErrCnt = 0;
		pEntry->HTC_AAD_OM_Force = 0;
		pEntry->HTC_AAD_OM_CountDown = 0;
		pEntry->HTC_AAD_OM_Freeze = 0;
	}

#endif /* HTC_DECRYPT_IOT */
	FlgIs11bSta = 1;

	for (i = 0; i < ie_list->SupportedRatesLen; i++) {
		if (((ie_list->SupportedRates[i] & 0x7F) != 2) &&
			((ie_list->SupportedRates[i] & 0x7F) != 4) &&
			((ie_list->SupportedRates[i] & 0x7F) != 11) &&
			((ie_list->SupportedRates[i] & 0x7F) != 22)) {
			FlgIs11bSta = 0;
			break;
		}
	}


#ifdef MBO_SUPPORT
	if (!MBO_AP_ALLOW_ASSOC(wdev)) {
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		bMboReject = TRUE;
		goto SendAssocResponse;
	}
#endif /* MBO_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT

	if ((tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		&& (pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
#ifdef DOT11R_FT_SUPPORT
		&& (!IS_FT_RSN_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
		) {
		StatusCode = MLME_ASSOC_REJ_TEMPORARILY;
		goto SendAssocResponse;
	}

#endif /* DOT11W_PMF_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
		   )) {
		ASIC_SEC_INFO Info = {0};
		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
		Info.Wcid = pEntry->wcid;
		/* Set key material to Asic */
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
			|| IS_IEEE8021X(&pEntry->SecConfig))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
	}

#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0) &&
			(IS_AKM_WPA_CAPABILITY_Entry(wdev)) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}

#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0))
			goto LabelOK;

#ifdef WSC_V2_SUPPORT

	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	if ((pEntry->bWscCapable) &&
		(wsc_ctrl->WscConfMode != WSC_DISABLE) &&
		(wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable) &&
		(MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)))
		;
	else
#endif /* WSC_V2_SUPPORT */

			/* set a flag for sending Assoc-Fail response to unwanted STA later. */
			if (!ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->func_tb_idx))
				bACLReject = TRUE;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s - MBSS(%d), receive %s request from %02x:%02x:%02x:%02x:%02x:%02x\n",
			  sAssoc, pEntry->func_tb_idx, sAssoc, PRINT_MAC(ie_list->Addr2)));

	/* supported rates array may not be sorted. sort it and find the maximum rate */
	for (i = 0; i < ie_list->SupportedRatesLen; i++) {
		if (MaxSupportedRate < (ie_list->SupportedRates[i] & 0x7f))
			MaxSupportedRate = ie_list->SupportedRates[i] & 0x7f;
	}

	/*
		Assign RateLen here or we will select wrong rate table in
		APBuildAssociation() when 11N compile option is disabled.
	*/
	pEntry->RateLen = ie_list->SupportedRatesLen;

	RTMPSetSupportMCS(pAd,
					  OPMODE_AP,
					  pEntry,
					  ie_list->SupportedRates,
					  ie_list->SupportedRatesLen,
					  NULL,
					  0,
#ifdef DOT11_VHT_AC
					  ie_list->vht_cap_len,
					  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
					  &ie_list->HTCapability,
					  ie_list->ht_cap_len);

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, ie_list, MaxSupportedRate, &Aid, isReassoc);

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if (wdev_do_conn_act(pEntry->wdev, pEntry) != TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():connect action fail!!\n", __func__));
		}
	}

#ifdef DOT11R_FT_SUPPORT

	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
		pFtCfg = &(wdev->FtCfg);

		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			StatusCode = FT_AssocReqHandler(pAd, isReassoc, pFtCfg, pEntry,
											&ie_list->FtInfo, &FtInfoBuf);

		/* just silencely discard this frame */
		if (StatusCode == 0xFFFF)
			goto LabelOK;
	}

#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT

	if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_ENABLE(wdev))
		pEntry->RrmEnCap.word = ie_list->RrmEnCap.word;

#endif /* DOT11K_RRM_SUPPORT */
#ifdef DOT11_VHT_AC

	if (ie_list->vht_cap_len) {
		/* +++Add by shiang for debug */
		if (WMODE_CAP_AC(wdev->PhyMode)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s():Peer is VHT capable device!\n", __func__));
			NdisMoveMemory(&pEntry->ext_cap, &ie_list->ExtCapInfo, sizeof(ie_list->ExtCapInfo));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("\tOperatingModeNotification=%d\n",
					  pEntry->ext_cap.operating_mode_notification));
			/* dump_vht_cap(pAd, &ie_list->vht_cap); */
		}

		/* ---Add by shiang for debug */
	}

#endif /* DOT11_VHT_AC */

	if (StatusCode == MLME_ASSOC_REJ_DATA_RATE)
		RTMPSendWirelessEvent(pAd, IW_STA_MODE_EVENT_FLAG, pEntry->Addr, wdev->wdev_idx, 0);

#ifdef DOT11W_PMF_SUPPORT
SendAssocResponse:
#endif /* DOT11W_PMF_SUPPORT */
	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s - Send %s response (Status=%d)...\n",
			  sAssoc, sAssoc, StatusCode));
	Aid |= 0xc000; /* 2 most significant bits should be ON */
	SubType = isReassoc ? SUBTYPE_REASSOC_RSP : SUBTYPE_ASSOC_RSP;
	CapabilityInfoForAssocResp = pMbss->CapabilityInfo; /*use AP's cability */
#ifdef WSC_AP_SUPPORT
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if ((wsc_ctrl->WscConfMode != WSC_DISABLE) &&
			(ie_list->CapabilityInfo & 0x0010))
			CapabilityInfoForAssocResp |= 0x0010;
	}

#endif /* WSC_AP_SUPPORT */
	/* fail in ACL checking => send an Assoc-Fail resp. */
	SupRateLen = rate->SupRateLen;

	/* TODO: need to check rate in support rate element, not number */
	if (FlgIs11bSta == 1)
		SupRateLen = 4;

	/* YF@20120419: Refuse the weak signal of AssocReq */
	rssi = RTMPMaxRssi(pAd,
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("ra[%d] ASSOC_REQ Threshold = %d, PktMaxRssi=%d\n",
			  pEntry->func_tb_idx, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold,
			  rssi));

	if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold != 0) &&
		(rssi < pAd->ApCfg.MBSSID[pEntry->func_tb_idx].AssocReqRssiThreshold)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Reject this ASSOC_REQ due to Weak Signal.\n"));
		bAssocSkip = TRUE;
	}

	if (bACLReject == TRUE || bAssocSkip) {
		MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
						 wdev->if_addr, wdev->bssid);
		StatusCode = MLME_UNSPECIFY_FAIL;
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &AssocRspHdr,
						  2,                        &CapabilityInfoForAssocResp,
						  2,                        &StatusCode,
						  2,                        &Aid,
						  1,                        &SupRateIe,
						  1,                        &SupRateLen,
						  SupRateLen, rate->SupRate,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory((PVOID) pOutBuffer);
		RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, ie_list->Addr2, wdev->wdev_idx, 0);
#ifdef WSC_V2_SUPPORT

		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

#endif /* WSC_V2_SUPPORT */

		if (bAssocSkip == TRUE) {
			pEntry = MacTableLookup(pAd, ie_list->Addr2);

			if (pEntry)
				MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}

		goto LabelOK;
	}

	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,                        &CapabilityInfoForAssocResp,
					  2,                        &StatusCode,
					  2,                        &Aid,
					  1,                        &SupRateIe,
					  1,                        &SupRateLen,
					  SupRateLen,				rate->SupRate,
					  END_OF_ARGS);

	if ((rate->ExtRateLen) && (PhyMode != WMODE_B) && (FlgIs11bSta == 0)) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
						  1,                        &ExtRateIe,
						  1,                        &rate->ExtRateLen,
						  rate->ExtRateLen, rate->ExtRate,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}


#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

	/* add WMM IE here */
	/* printk("%s()=>bWmmCapable=%d,CLINE=%d\n",__FUNCTION__,wdev->bWmmCapable,CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)); */

	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
	}

#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY) {
		ULONG TmpLen;
		UCHAR IEType = IE_TIMEOUT_INTERVAL; /* IE:0x15 */
		UCHAR IELen = 5;
		UCHAR TIType = 3;
		UINT32 units = 1 << 10; /* 1 seconds, should be 0x3E8 */

		MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						  1, &IEType,
						  1, &IELen,
						  1, &TIType,
						  4, &units,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif /* DOT11W_PMF_SUPPORT */
#ifdef DOT11_N_SUPPORT

	/* HT capability in AssocRsp frame. */
	if ((ie_list->ht_cap_len > 0) && WMODE_CAP_N(wdev->PhyMode) &&
		(wdev->DesiredHtPhyInfo.bHtEnable)) {
#ifdef DOT11_VHT_AC
		struct _build_ie_info vht_ie_info;
#endif /* DOT11_VHT_AC */

		ie_info.is_draft_n_type = FALSE;
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_ht_ies(pAd, &ie_info);

		if ((ie_list->vendor_ie.ra_cap) == 0 || (pAd->bBroadComHT == TRUE)) {
			ie_info.is_draft_n_type = TRUE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);

		}
#ifdef DOT11_VHT_AC
		vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		vht_ie_info.frame_subtype = SUBTYPE_ASSOC_RSP;
		vht_ie_info.channel = wdev->channel;
		vht_ie_info.phy_mode = wdev->PhyMode;
		vht_ie_info.wdev = wdev;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		ucETxBfCap = wlan_config_get_etxbf(wdev);

		if (HcIsBfCapSupport(wdev) == FALSE)
			wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
		FrameLen += build_vht_ies(pAd, &vht_ie_info);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
		wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#endif /* DOT11_VHT_AC */
	}

#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2
	/* qosmap IE */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("entry wcid %d QosMapSupport=%d\n",
			 pEntry->wcid, pEntry->QosMapSupport));

	if (pEntry->QosMapSupport) {
		ULONG	TmpLen;
		UCHAR	QosMapIE, ielen = 0, explen = 0;
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

		if (pHSCtrl->QosMapEnable) {
			QosMapIE = IE_QOS_MAP_SET;

			/* Fixed field Dscp range:16, len:1 IE_ID:1*/
			if (pHSCtrl->QosMapSetIELen > 18)
				explen = pHSCtrl->QosMapSetIELen - 18;

			pEntry->DscpExceptionCount = explen;
			memcpy((UCHAR *)pEntry->DscpRange, (UCHAR *)pHSCtrl->DscpRange, 16);
			memcpy((UCHAR *)pEntry->DscpException, (UCHAR *)pHSCtrl->DscpException, 42);
			ielen = explen + 16;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1,			&QosMapIE,
							  1,			&ielen,
							  explen,		pEntry->DscpException,
							  16,			pEntry->DscpRange,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

#endif /* CONFIG_HOTSPOT_R2 */

	if (IS_AKM_OWE_Entry(pEntry)) {
		CHAR rsne_idx;
		ULONG	TmpLen;
		struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;
		/*struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;*/

		WPAMakeRSNIE(wdev->wdev_type, pSecConfig, pEntry);

		for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
			if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
				continue;

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1, &pSecConfig->RSNE_EID[rsne_idx][0],
								1, &pSecConfig->RSNE_Len[rsne_idx],
								pSecConfig->RSNE_Len[rsne_idx],
								&pSecConfig->RSNE_Content[rsne_idx][0],
								END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}

	/* 7.3.2.27 Extended Capabilities IE */
	{
		ULONG TmpLen, infoPos;
		PUCHAR pInfo;
		UCHAR extInfoLen;
		BOOLEAN bNeedAppendExtIE = FALSE;
		EXT_CAP_INFO_ELEMENT extCapInfo;
#ifdef RT_BIG_ENDIAN
		UCHAR *pextCapInfo;
#endif

		extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
		NdisZeroMemory(&extCapInfo, extInfoLen);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		/* P802.11n_D1.10, HT Information Exchange Support */
		if (WMODE_CAP_N(wdev->PhyMode)
			&& (wdev->channel <= 14)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_DOT11V_WNM
		/* #ifdef CONFIG_HOTSPOT_R2 Remove for WNM independance */

		if (ie_list->ExtCapInfo.BssTransitionManmt == 1) {
			pEntry->bBSSMantSTASupport = TRUE;
			if (pMbss->WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
		}
		/* #endif CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */

#ifdef CONFIG_DOT11U_INTERWORKING
	    if (pMbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */

#ifdef DOT11_VHT_AC

		if (WMODE_CAP_AC(wdev->PhyMode) &&
			(wdev->channel > 14))
			extCapInfo.operating_mode_notification = 1;

#endif /* DOT11_VHT_AC */
#ifdef FTM_SUPPORT

		/* add IE_EXT_CAPABILITY IE here */
		if (pAd->pFtmCtrl->bSetCivicRpt)
			extCapInfo.civic_location = 1;

		if (pAd->pFtmCtrl->bSetLciRpt)
			extCapInfo.geospatial_location = 1;

		/* 802.11mc D3.0: 10.24.6.2 (p.1717):
			"A STA in which dot11FineTimingMsmtRespActivated is true shall set the Fine Timing Measurement
			Responder field of the Extended Capabilities element to 1."
		*/
		/* 8.4.2.26 Extended Capabilities element (p.817):
			Capabilities field= 70: Fine Timing Measurement Responder (p.823)
		*/
		extCapInfo.ftm_resp = 1;
#endif /* FTM_SUPPORT */
#ifdef RT_BIG_ENDIAN
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)pextCapInfo) = cpu2le32(*((UINT32 *)pextCapInfo));
		pextCapInfo = (UCHAR *)&extCapInfo;
		*((UINT32 *)(pextCapInfo + 4)) = cpu2le32(*((UINT32 *)(pextCapInfo + 4)));
#endif

		pInfo = (UCHAR *)(&extCapInfo);

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
							  1,			&ExtCapIe,
							  1,			&extInfoLen,
							  extInfoLen,	&extCapInfo,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
	}
	/* add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back */
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_ASSOC_RESP
							   );

#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev))
		MakeMboOceIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_ASSOC_RSP);
#endif /* MBO_SUPPORT */
#ifdef WSC_AP_SUPPORT

	if (pEntry->bWscCapable) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;
		ULONG WscTmpLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocRespIE(pAd, pEntry->func_tb_idx, 0, pWscBuf, &WscIeLen);
			MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
							  WscIeLen, pWscBuf,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
			os_free_mem(pWscBuf);
		}
	}

#endif /* WSC_AP_SUPPORT */
#ifdef RT_CFG80211_SUPPORT

	/* Append extra IEs provided by wpa_supplicant */
	if (pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIeLen) {
		ULONG TmpLen = 0;
		INT32 IesLen = pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIeLen;
		UCHAR *Ies = pAd->ApCfg.MBSSID[pEntry->apidx].AssocRespExtraIe;

		if (RTMPIsValidIEs(Ies, IesLen)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("AssocRespExtraIE Added (Len=%d)\n", IesLen));
			MakeOutgoingFrame(pOutBuffer + FrameLen,
							  &TmpLen,
							  IesLen,
							  Ies,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("AssocRespExtraIE len incorrect!\n"));
	}

#endif /* RT_CFG80211_SUPPORT */
#ifdef DOT11R_FT_SUPPORT

	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		PUINT8	ftie_ptr = NULL;
		UINT8	ftie_len = 0;
		PUINT8  ricie_ptr = NULL;
		UINT8   ricie_len = 0;
		struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

		/* Insert RSNIE if necessary */
		if (FtInfoBuf.RSNIE_Len != 0) {
			ULONG TmpLen;

			MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
							  FtInfoBuf.RSNIE_Len,		FtInfoBuf.RSN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer + FrameLen;
		mdie_len = 5;
		FT_InsertMdIE(pAd, pOutBuffer + FrameLen,
					  &FrameLen,
					  FtInfoBuf.MdIeInfo.MdId,
					  FtInfoBuf.MdIeInfo.FtCapPlc);

		/* Insert FTIE. */
		if (FtInfoBuf.FtIeInfo.Len != 0) {
			ftie_ptr = pOutBuffer + FrameLen;
			ftie_len = (2 + FtInfoBuf.FtIeInfo.Len);
			FT_InsertFTIE(pAd, pOutBuffer + FrameLen, &FrameLen,
						  FtInfoBuf.FtIeInfo.Len,
						  FtInfoBuf.FtIeInfo.MICCtr,
						  FtInfoBuf.FtIeInfo.MIC,
						  FtInfoBuf.FtIeInfo.ANonce,
						  FtInfoBuf.FtIeInfo.SNonce);
		}

		/* Insert R1KH IE into FTIE. */
		if (FtInfoBuf.FtIeInfo.R1khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pAd, pOutBuffer + FrameLen,
									&FrameLen,
									FT_R1KH_ID,
									FtInfoBuf.FtIeInfo.R1khId,
									FtInfoBuf.FtIeInfo.R1khIdLen);

		/* Insert GTK Key info into FTIE. */
		if (FtInfoBuf.FtIeInfo.GtkLen != 0)
			FT_FTIE_InsertGTKSubIE(pAd, pOutBuffer + FrameLen,
								   &FrameLen,
								   FtInfoBuf.FtIeInfo.GtkSubIE,
								   FtInfoBuf.FtIeInfo.GtkLen);

		/* Insert R0KH IE into FTIE. */
		if (FtInfoBuf.FtIeInfo.R0khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pAd, pOutBuffer + FrameLen,
									&FrameLen,
									FT_R0KH_ID,
									FtInfoBuf.FtIeInfo.R0khId,
									FtInfoBuf.FtIeInfo.R0khIdLen);

		/* Insert IGTK Key info into FTIE. */

		if (FtInfoBuf.FtIeInfo.IGtkLen != 0) {
			FT_FTIE_InsertIGTKSubIE(pAd, pOutBuffer+FrameLen,
					&FrameLen,
					FtInfoBuf.FtIeInfo.IGtkSubIE,
					FtInfoBuf.FtIeInfo.IGtkLen);
		}

		/* Insert RIC. */
		if (ie_list->FtInfo.RicInfo.Len) {
			ULONG TempLen;

			FT_RIC_ResourceRequestHandle(pAd, pEntry,
										 (PUCHAR)ie_list->FtInfo.RicInfo.pRicInfo,
										 ie_list->FtInfo.RicInfo.Len,
										 (PUCHAR)pOutBuffer + FrameLen,
										 (PUINT32)&TempLen);
			ricie_ptr = (PUCHAR)(pOutBuffer + FrameLen);
			ricie_len = TempLen;
			FrameLen += TempLen;
		}

		/* Calculate the FT MIC for FT procedure */
		if (FtInfoBuf.FtIeInfo.MICCtr.field.IECnt) {
			UINT8	ft_mic[FT_MIC_LEN];
			PFT_FTIE	pFtIe;

			FT_CalculateMIC(pEntry->Addr,
							wdev->bssid,
							pEntry->FT_PTK,
							6,
							FtInfoBuf.RSN_IE,
							FtInfoBuf.RSNIE_Len,
							mdie_ptr,
							mdie_len,
							ftie_ptr,
							ftie_len,
							ricie_ptr,
							ricie_len,
							ft_mic);
			/* Update the MIC field of FTIE */
			pFtIe = (PFT_FTIE)(ftie_ptr + 2);
			NdisMoveMemory(pFtIe->MIC, ft_mic, FT_MIC_LEN);
			/* Install pairwise key */
#ifdef MBO_SUPPORT
				/* YF_FT */
				if (IS_MBO_ENABLE(wdev))
					/* update STA bssid & security info to daemon */
					MboIndicateStaBssidInfo(pAd, wdev, pEntry->Addr);
#endif /* MBO_SUPPORT */
				pSecConfig->Handshake.GTKState = REKEY_ESTABLISHED;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
				WifiSysUpdatePortSecur(pAd, pEntry, NULL);
		}

		/*	Record the MDIE & FTIE of (re)association response of
			Initial Mobility Domain Association. It's used in
			FT 4-Way handshaking */
		if ((IS_AKM_WPA2_Entry(pEntry) || IS_AKM_WPA2PSK_Entry(pEntry))
			&& ie_list->FtInfo.FtIeInfo.Len == 0) {
			NdisMoveMemory(&pEntry->InitialMDIE,
						   mdie_ptr, mdie_len);
			pEntry->InitialFTIE_Len = ftie_len;
			NdisMoveMemory(pEntry->InitialFTIE, ftie_ptr, ftie_len);
		}
	}

#endif /* DOT11R_FT_SUPPORT */

#ifdef CONFIG_OWE_SUPPORT
	if (IS_AKM_OWE_Entry(pEntry) && (StatusCode == MLME_SUCCESS)) {
		BOOLEAN need_ecdh_ie = FALSE;
		INT CacheIdx;/* Key cache */

		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE, ie_list->RSNIE_Len, PMKID_LIST, &pmkid_count);
		if (pPmkid != NULL) {
			CacheIdx = RTMPSearchPMKIDCache(&pAd->ApCfg.PMKIDCache, pEntry->func_tb_idx, pEntry->Addr);
			if ((CacheIdx == -1) ||
			    ((RTMPEqualMemory(pPmkid,
					      &pAd->ApCfg.PMKIDCache.BSSIDInfo[CacheIdx].PMKID,
					      LEN_PMKID)) == 0)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s: AKM_OWE_Entry PMKID not found, do normal ECDH procedure\n",
						__func__));
				need_ecdh_ie = TRUE;
			}
		} else
			need_ecdh_ie = TRUE;

		if (need_ecdh_ie == TRUE) {
			FrameLen +=  build_owe_dh_ie(pAd,
						     pEntry,
						     (UCHAR *)pOutBuffer + FrameLen,
						     pEntry->SecConfig.owe.last_try_group);
		}
	}
#endif /*CONFIG_OWE_SUPPORT*/

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory((PVOID) pOutBuffer);
#ifdef DOT11W_PMF_SUPPORT

	if (StatusCode == MLME_ASSOC_REJ_TEMPORARILY)
		PMF_MlmeSAQueryReq(pAd, pEntry);

#endif /* DOT11W_PMF_SUPPORT */

	/* set up BA session */
	if (StatusCode == MLME_SUCCESS) {
		pEntry->PsMode = PWR_ACTIVE;
		/* TODO: shiang-usw, we need to rmove upper setting and migrate to tr_entry->PsMode */
		pAd->MacTab.tr_entry[pEntry->wcid].PsMode = PWR_ACTIVE;
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
#ifdef IAPP_SUPPORT
		/*PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg; */
		/*		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie; */
		{
			/* send association ok message to IAPPD */
			IAPP_L2_Update_Frame_Send(pAd, pEntry->Addr, pEntry->wdev->wdev_idx);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("####### Send L2 Frame Mac=%02x:%02x:%02x:%02x:%02x:%02x\n",
					  PRINT_MAC(pEntry->Addr)));
		}

		/*		SendSingalToDaemon(SIGUSR2, pObj->IappPid); */
#ifdef DOT11R_FT_SUPPORT
		{
			/*
				Do not do any check here.
				We need to send MOVE-Req frame to AP1 even open mode.
			*/
			/*		if (IS_FT_RSN_STA(pEntry) && (FtInfo.FtIeInfo.Len != 0)) */
			if (isReassoc == 1) {
				/* only for reassociation frame */
				FT_KDP_EVT_REASSOC EvtReAssoc;

				EvtReAssoc.SeqNum = 0;
				NdisMoveMemory(EvtReAssoc.MacAddr, pEntry->Addr, MAC_ADDR_LEN);
				NdisMoveMemory(EvtReAssoc.OldApMacAddr, ie_list->ApAddr, MAC_ADDR_LEN);
				FT_KDP_EVENT_INFORM(pAd, pEntry->func_tb_idx, FT_KDP_SIG_FT_REASSOCIATION,
									&EvtReAssoc, sizeof(EvtReAssoc), NULL);
			}
		}

#endif /* DOT11R_FT_SUPPORT */
#endif /* IAPP_SUPPORT */
		/* ap_assoc_info_debugshow(pAd, isReassoc, pEntry, ie_list); */
		/* send wireless event - for association */
#ifdef VENDOR_FEATURE7_SUPPORT
		 /* Passed in the pEntry->apindx argument */
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, pEntry->func_tb_idx, 0);
#else
		RTMPSendWirelessEvent(pAd, IW_ASSOC_EVENT_FLAG, pEntry->Addr, 0, 0);
#endif
		/* This is a reassociation procedure */
		pEntry->IsReassocSta = isReassoc;
		/* clear txBA bitmap */
		pEntry->TXBAbitmap = 0;

		if (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX) {
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

			if ((wdev->channel <= 14) &&
				addht->AddHtInfo.ExtChanOffset &&
				(ie_list->HTCapability.HtCapInfo.ChannelWidth == BW_40))
				SendBeaconRequest(pAd, pEntry->wcid);

			ba_ori_session_setup(pAd, pEntry, 5, 0, 10, FALSE);
		}

#ifdef DOT11R_FT_SUPPORT

		/*	If the length of FTIE field of the (re)association-request frame
			is larger than zero, it shall indicate the Fast-BSS transition is in progress. */
		if (ie_list->FtInfo.FtIeInfo.Len > 0)
			;
		else
#endif /* DOT11R_FT_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
			if (TRUE) { /*CFG_TODO*/
				/* need to update pEntry to  inform later flow to keep ConnectionState in connected */
				pEntry->bWscCapable = ie_list->bWscCapable;
				{
					MTWF_LOG(DBG_CAT_AP,
						 DBG_SUBCAT_ALL,
						 DBG_LVL_TRACE,
						 ("SINGLE CFG: NOITFY ASSOCIATED, pEntry->bWscCapable:%d\n",
						  pEntry->bWscCapable));
					CFG80211OS_NewSta(pEntry->wdev->if_dev,
							  ie_list->Addr2,
							  (PUCHAR)Elem->Msg,
							  Elem->MsgLen);

					if (IS_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher)) {
						ASIC_SEC_INFO Info = {0};
						/* Set key material to Asic */
						os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
						Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						Info.Direction = SEC_ASIC_KEY_BOTH;
						Info.Wcid = pEntry->wcid;
						Info.BssIndex = pEntry->func_tb_idx;
						Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						Info.Cipher = pEntry->SecConfig.PairwiseCipher;
						Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						os_move_mem(&Info.Key,
							    &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId],
							    sizeof(SEC_KEY_INFO));
						os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
						HW_ADDREMOVE_KEYTABLE(pAd, &Info);
					}
				}

				hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen);
			} else
#endif
			/* enqueue a EAPOL_START message to trigger EAP state machine doing the authentication */
			if (IS_AKM_PSK_Entry(pEntry)) {
				pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
								   ie_list->RSNIE_Len,
								   PMKID_LIST,
								   &pmkid_count);

				if (pPmkid != NULL) {
					INT CacheIdx;

					CacheIdx = RTMPValidatePMKIDCache(&pAd->ApCfg.PMKIDCache,
									  pEntry->func_tb_idx,
									  pEntry->Addr,
									  pPmkid);

					store_pmkid_cache_in_sec_config(pAd, pEntry, CacheIdx);

					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("ASSOC - CacheIdx = %d\n",
						  CacheIdx));

					if (IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
					   !is_pmkid_cache_in_sec_config(&pEntry->SecConfig)) {
						MTWF_LOG(DBG_CAT_SEC,
							 CATSEC_SAE,
							 DBG_LVL_ERROR,
							 ("ASSOC - SAE - verify pmkid fail\n"));
						MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
						goto LabelOK;
					}
				}
#ifdef WSC_AP_SUPPORT
				/*
				 * In WPA-PSK mode,
				 * If Association Request of station has RSN/SSN,
				 * WPS AP Must Not send EAP-Request/Identity to station
				 * no matter WPS AP does receive EAPoL-Start from STA or not.
				 * Marvell WPS test bed(v2.1.1.5) will send AssocReq with WPS IE and RSN/SSN IE.
				 */
				if (pEntry->bWscCapable || (ie_list->RSNIE_Len == 0)) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("ASSOC - IF(ra%d) This is a WPS Client.\n\n",
							  pEntry->func_tb_idx));
					goto LabelOK;
				} else {
					pEntry->bWscCapable = FALSE;
					pEntry->Receive_EapolStart_EapRspId = (WSC_ENTRY_GET_EAPOL_START |
									       WSC_ENTRY_GET_EAP_RSP_ID);
					/* This STA is not a WPS STA */
					NdisZeroMemory(wsc_ctrl->EntryAddr, 6);
				}

#endif /* WSC_AP_SUPPORT */
				/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
				if (1
#ifdef HOSTAPD_SUPPORT
				    && wdev->Hostapd == Hostapd_Disable
#endif/*HOSTAPD_SUPPORT*/
#ifdef WSC_AP_SUPPORT
				    && !pEntry->bWscCapable
#endif /* WSC_AP_SUPPORT */
				   ) {
					/* Enqueue a EAPOL-start message with the pEntry */
					os_move_mem(&pEntry->SecConfig.Handshake.AAddr, wdev->bssid, MAC_ADDR_LEN);
					os_move_mem(&pEntry->SecConfig.Handshake.SAddr, pEntry->Addr, MAC_ADDR_LEN);

					if (!IS_AKM_WPA3PSK(pEntry->SecConfig.AKMMap) &&
					    !(IS_AKM_OWE(pEntry->SecConfig.AKMMap)))
						os_move_mem(&pEntry->SecConfig.PMK, &wdev->SecConfig.PMK, LEN_PMK);

					RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
				}
			}

#ifdef DOT1X_SUPPORT
			else if (IS_AKM_WPA2_Entry(pEntry) ||
				 IS_AKM_WPA3_Entry(pEntry)) {
				pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
								   ie_list->RSNIE_Len,
								   PMKID_LIST,
								   &pmkid_count);

				if (pPmkid != NULL) {
					/* Key cache */
					INT	CacheIdx;

					CacheIdx = RTMPValidatePMKIDCache(&pAd->ApCfg.PMKIDCache,
									  pEntry->func_tb_idx,
									  pEntry->Addr,
									  pPmkid);

					process_pmkid(pAd, wdev, pEntry, CacheIdx);
				}
			} else if (IS_AKM_1X_Entry(pEntry) ||
				   (IS_IEEE8021X(&pEntry->SecConfig)
#ifdef WSC_AP_SUPPORT
				   && (!pEntry->bWscCapable)
#endif /* WSC_AP_SUPPORT */
				   )) {
				/* Enqueue a EAPOL-start message to trigger EAP SM */
				if (pEntry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT
					&& wdev->Hostapd == Hostapd_Disable
#endif/*HOSTAPD_SUPPORT*/
				) {
					pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
					RTMPSetTimer(&pEntry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
				}
			}

#endif /* DOT1X_SUPPORT */

#if defined(MWDS) || defined(CONFIG_MAP_SUPPORT) || defined(WAPP_SUPPORT)
		if (tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
#ifdef WAPP_SUPPORT
			wapp_send_cli_join_event(pAd, pEntry);
#endif
		}
#endif
#ifdef SMART_ANTENNA
		{
			unsigned long irqflags;
			/* Check if need to reset the sa training procedures to init stage! */
			RTMP_IRQ_LOCK(&pAd->smartAntLock, irqflags);

			if (RTMP_SA_WORK_ON(pAd)) {
				/* sa_add_train_entry(pAd, &pEntry->Addr[0], FALSE); */
				pAd->pSAParam->bStaChange = TRUE;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("%s():sta(%02x:%02x:%02x:%02x:%02x:%02x) add!\n",
						  __func__, PRINT_MAC(pEntry->Addr)));
			}

			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqflags);
		}
#endif /* SMART_ANTENNA // */
#ifdef GREENAP_SUPPORT

		if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap)) {
			if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC))
				greenap_check_peer_connection_at_link_up_down(pAd, wdev, greenap);
		}

#endif /* GREENAP_SUPPORT */
#ifdef CONFIG_HOTSPOT_R2

		/* add to cr4 pool */
		if (pEntry->QosMapSupport) {
			PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[pEntry->apidx].HotSpotCtrl;

			if (pHSCtrl->QosMapEnable) {
				if (!pHSCtrl->QosMapAddToPool) {
					pHSCtrl->QosMapAddToPool = TRUE;
					pHSCtrl->QosMapPoolID = hotspot_qosmap_add_pool(pAd, pEntry);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("add current MBSS qosmap to CR4\n"));
				}

				hotspot_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pHSCtrl->QosMapPoolID);
			}
		}

#endif /* CONFIG_HOTSPOT_R2 */
	}

#ifdef FAST_EAPOL_WAR
/*
*	Moved from WifiSysApPeerLinkUp() in open security mode.
*	to make sure the STATE_PORT_SECURE flag can be polled by MU N9 module.
*	then MU Action Frame sent out after Asso Resp.
*/
		if (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) {
			if (!(IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#ifdef DOT1X_SUPPORT
				|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
#ifdef RT_CFG80211_SUPPORT
				|| wdev->IsCFG1xWdev
#endif /* RT_CFG80211_SUPPORT */
				|| pEntry->bWscCapable)) {
				WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			}
		}
#endif /* FAST_EAPOL_WAR */
#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering)
		) {
			BndStrg_UpdateEntry(pAd, pEntry, ie_list, TRUE);
		}
#endif

#ifdef VENDOR_FEATURE7_SUPPORT
		if (arris_event_send_hook && pEntry && (StatusCode == MLME_SUCCESS)) {
			UCHAR assoc_event_msg[32] = {0};
			UINT32 count = 0;
			UCHAR *assoc_sta_info = NULL;
			HT_CAP_INFO  *pHTCap = &(ie_list->HTCapability).HtCapInfo;
			HT_CAP_PARM  *pHTCapParm = &(ie_list->HTCapability).HtCapParm;
			/* Send a WLAN_EVENT to ATOM which in turns sends an RPC
			*	to update our client table on the ARM.
			*/
			NdisZeroMemory(assoc_event_msg, sizeof(assoc_event_msg));
			if (WMODE_CAP_5G(PhyMode))
				count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
						"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
						PRINT_MAC(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_50_RADIO);
			else
				count = snprintf(assoc_event_msg, sizeof(assoc_event_msg),
						"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
						PRINT_MAC(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_24_RADIO);

			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_ASSOC,
				assoc_event_msg, count);

			/* Log this cleint's capabilities in our nvram */
			/* assoc_sta_info = kmalloc(1300, GFP_ATOMIC); */
			os_alloc_mem(NULL, (UCHAR **)&assoc_sta_info, 1300);
			if (assoc_sta_info) {
				NdisZeroMemory(assoc_sta_info, 1300);
				count = 0;
				count += snprintf((assoc_sta_info+count), (1300-count),
				"Association: (%02x:%02x:%02x:%02x:%02x:%02x) --> %s%d (%s)\n",
				PRINT_MAC(pEntry->Addr), INF_MAIN_DEV_NAME, pEntry->func_tb_idx,
				pEntry->pMbss->Ssid);
				if (pHTCap && pHTCapParm && ie_list->ht_cap_len && WMODE_CAP_N(wdev->PhyMode)) {
					count += snprintf((assoc_sta_info+count), (1300-count), "  Station Info:\n");
					count += snprintf((assoc_sta_info+count), (1300-count),
					"\tRSSI0(%d), RSSI1(%d), Mode(%s), BW(%s), MCS(%d), SGI(%d)\n",
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
					ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
					get_phymode_str(pEntry->HTPhyMode.field.MODE),
					get_bw_str(pEntry->HTPhyMode.field.BW),
					pEntry->HTPhyMode.field.MCS,
					pEntry->HTPhyMode.field.ShortGI);
				} else {
					count += snprintf((assoc_sta_info+count), (1300-count),
						"    Station Info (Legacy):\n");
					count += snprintf((assoc_sta_info+count), (1300-count),
						"\tRSSI0(%d), RSSI1(%d), Mode(%s), MCS(%d)\n",
						ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
						get_phymode_str(pEntry->HTPhyMode.field.MODE),
						pEntry->HTPhyMode.field.MCS);
				}
				ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST,
				WLAN_LOG_SAVE, 0, assoc_sta_info, count);
				os_free_mem(assoc_sta_info);
			}
		}
#endif

LabelOK:
#ifdef RT_CFG80211_SUPPORT

	if (StatusCode != MLME_SUCCESS)
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);

#endif /* RT_CFG80211_SUPPORT */

	if (ie_list != NULL)
		os_free_mem(ie_list);
}



/*
    ==========================================================================
    Description:
	peer assoc req handling procedure
    Parameters:
	Adapter - Adapter pointer
	Elem - MLME Queue Element
    Pre:
	the station has been authenticated and the following information is stored
    Post  :
	-# An association response frame is generated and sent to the air
    ==========================================================================
 */
VOID APPeerAssocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 0);
}

/*
    ==========================================================================
    Description:
	mlme reassoc req handling procedure
    Parameters:
	Elem -
    Pre:
	-# SSID  (Adapter->ApCfg.ssid[])
	-# BSSID (AP address, Adapter->ApCfg.bssid)
	-# Supported rates (Adapter->ApCfg.supported_rates[])
	-# Supported rates length (Adapter->ApCfg.supported_rates_len)
	-# Tx power (Adapter->ApCfg.tx_power)
    ==========================================================================
 */
VOID APPeerReassocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	ap_cmm_peer_assoc_req_action(pAd, Elem, 1);
}


/*
    ==========================================================================
    Description:
	MLME message sanity check
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
static BOOLEAN PeerDisassocReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pDA,
	OUT PUCHAR pAddr2,
	OUT	UINT16 * SeqNum,
	OUT USHORT *Reason)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(pDA, &Fr->Hdr.Addr1);
	COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
	NdisMoveMemory(Reason, &Fr->Octet[0], 2);
	return TRUE;
}


/*
    ==========================================================================
    Description:
	left part of IEEE 802.11/1999 p.374
    Parameters:
	Elem - MLME message containing the received frame
    ==========================================================================
 */
VOID APPeerDisassocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	USHORT Reason;
	UINT16 SeqNum;
	MAC_TABLE_ENTRY *pEntry;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ASSOC - 1 receive DIS-ASSOC request\n"));

	if (!PeerDisassocReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, &SeqNum, &Reason))
		return;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ASSOC - receive DIS-ASSOC(seq-%d) request from %02x:%02x:%02x:%02x:%02x:%02x, reason=%d\n",
			  SeqNum, PRINT_MAC(Addr2), Reason));
	pEntry = MacTableLookup(pAd, Addr2);

	if (pEntry == NULL)
		return;

	if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
		wdev = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev;

		/*
			iPhone sometimes sends disassoc frame which DA is old AP and BSSID is new AP.
			@2016/1/26
		*/
		if (!MAC_ADDR_EQUAL(wdev->if_addr, Addr1)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("ASSOC - The DA of this DIS-ASSOC request is %02x:%02x:%02x:%02x:%02x:%02x, ignore.\n",
					  PRINT_MAC(Addr1)));
			return;
		}

#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(wdev))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
		/* send wireless event - for disassociation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, Addr2, 0, 0);
#ifdef VENDOR_FEATURE7_SUPPORT
		{
			UCHAR disassoc_event_msg[32] = {0};

			if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.PhyMode))
				snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(Addr2), (pEntry->func_tb_idx) + WIFI_50_RADIO);
			else
				snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(Addr2), (pEntry->func_tb_idx) + WIFI_24_RADIO);

			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_DISSOC,
				disassoc_event_msg, strlen(disassoc_event_msg));
		}
#endif
		ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);

		MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);
#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn == TRUE) {
			UCHAR apCliIdx, CliIdx;
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, Addr2, TRUE);

			if (pReptEntry && (pReptEntry->CliConnectState != 0)) {
				apCliIdx = pReptEntry->MatchApCliIdx;
				CliIdx = pReptEntry->MatchLinkIdx;
				pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_AP_PEER_DISASSOC_REQ;
				MlmeEnqueue(pAd,
							APCLI_CTRL_STATE_MACHINE,
							APCLI_CTRL_DISCONNECT_REQ,
							0,
							NULL,
							(REPT_MLME_START_IDX + CliIdx));
				RTMP_MLME_HANDLER(pAd);
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
	}
}


/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID MbssKickOutStas(RTMP_ADAPTER *pAd, INT apidx, USHORT Reason)
{
	INT i;
	PMAC_TABLE_ENTRY pEntry;

#ifdef MBO_SUPPORT
	MboIndicateStaDisassocToDaemon(pAd, NULL, MBO_MSG_AP_TERMINATION);
#endif /* MBO_SUPPORT */

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->func_tb_idx == apidx)
			APMlmeKickOutSta(pAd, pEntry->Addr, pEntry->wcid, Reason);
	}
	/* mbo wait for max 500ms and check all IsKeep is zero */
#ifdef MBO_SUPPORT
	MboWaitAllStaGone(pAd, apidx);
#endif /* MBO_SUPPORT */
}


/*
    ==========================================================================
    Description:
	delete it from STA and disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *pStaAddr, UCHAR Wcid, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR Aid;
	UCHAR ApIdx;

	pEntry = MacTableLookup(pAd, pStaAddr);

	if (pEntry == NULL)
		return;

	Aid = pEntry->Aid;
	ApIdx = pEntry->func_tb_idx;

	if (ApIdx >= pAd->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Apidx=%d\n",
				 __func__, ApIdx));
		return;
	}

	if (VALID_UCAST_ENTRY_WCID(pAd, Wcid)) {
		/* send wireless event - for disassocation */
		RTMPSendWirelessEvent(pAd, IW_DISASSOC_EVENT_FLAG, pStaAddr, 0, 0);
#ifdef VENDOR_FEATURE7_SUPPORT
		{
			UCHAR disassoc_event_msg[32] = {0};

			if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[ApIdx].wdev.PhyMode))
				snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(pStaAddr), (ApIdx) + WIFI_50_RADIO);
			else
				snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					"%02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(pStaAddr), (ApIdx) + WIFI_24_RADIO);

			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_DISSOC,
				disassoc_event_msg, strlen(disassoc_event_msg));
		}
#endif
		ApLogEvent(pAd, pStaAddr, EVENT_DISASSOCIATED);
		/* 2. send out a DISASSOC request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("ASSOC - MLME disassociates %02x:%02x:%02x:%02x:%02x:%02x; Send DISASSOC request\n",
				  PRINT_MAC(pStaAddr)));
		MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pStaAddr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.if_addr,
						 pAd->ApCfg.MBSSID[ApIdx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer,            &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2,                     &Reason,
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
		MacTableDeleteEntry(pAd, Wcid, pStaAddr);
	}
}


#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason)
{
	HEADER_802_11 DisassocHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS     NStatus;
	UCHAR           BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	PPMF_CFG        pPmfCfg = NULL;

	pPmfCfg = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PmfCfg;

	if ((apidx < pAd->ApCfg.BssidNum) && (pPmfCfg)) {
		/* Send out a Deauthentication request frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Send DISASSOC Broadcast frame(%d) with ra%d\n", Reason, apidx));
		/* 802.11 Header */
		NdisZeroMemory(&DisassocHdr, sizeof(HEADER_802_11));
		DisassocHdr.FC.Type = FC_TYPE_MGMT;
		DisassocHdr.FC.SubType = SUBTYPE_DISASSOC;
		DisassocHdr.FC.ToDs = 0;
		DisassocHdr.FC.Wep = 0;
		COPY_MAC_ADDR(DisassocHdr.Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		COPY_MAC_ADDR(DisassocHdr.Addr3, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
						  sizeof(HEADER_802_11), &DisassocHdr,
						  2, &Reason,
						  END_OF_ARGS);

		if (pPmfCfg->MFPC == TRUE) {
			ULONG TmpLen;
			UCHAR res_buf[LEN_PMF_MMIE];
			UCHAR EID, ELen;

			EID = IE_PMF_MMIE;
			ELen = LEN_PMF_MMIE;
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  1, &EID,
							  1, &ELen,
							  LEN_PMF_MMIE, res_buf,
							  END_OF_ARGS);
			FrameLen += TmpLen;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("[PMF]: This is a Broadcast Robust management frame, Add 0x4C(76) EID\n"));
		}

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pOutBuffer);
	}
}
#endif /* DOT11W_PMF_PLUGFEST */


/*
    ==========================================================================
    Description:
	Upper layer orders to disassoc s STA
    Parameters:
	Elem -
    ==========================================================================
 */
VOID APMlmeDisassocReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MLME_DISASSOC_REQ_STRUCT *DisassocReq = (MLME_DISASSOC_REQ_STRUCT *)(Elem->Msg);
	APMlmeKickOutSta(pAd, DisassocReq->Addr, Elem->Wcid, DisassocReq->Reason);
}


/*
    ==========================================================================
    Description:
	right part of IEEE 802.11/1999 page 374
    Note:
	This event should never cause ASSOC state machine perform state
	transition, and has no relationship with CNTL machine. So we separate
	this routine as a service outside of ASSOC state transition table.
    ==========================================================================
 */
VOID APCls3errAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11         DisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	USHORT                Reason = REASON_CLS3ERR;
	MAC_TABLE_ENTRY       *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	if (pEntry) {
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
		mac_entry_delete(pAd, pEntry);
	}

	/* 2. send out a DISASSOC request frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ASSOC - Class 3 Error, Send DISASSOC frame to %02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(pRxBlk->Addr2)));
	MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pRxBlk->Addr2,
					 pRxBlk->Addr1,
					 pRxBlk->Addr1);
	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2,                     &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}


/*
    ==========================================================================
    Description:
	association state machine init, including state transition and timer init
    Parameters:
	S - pointer to the association state machine
    Note:
	The state machine looks like the following

				    AP_ASSOC_IDLE
	APMT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action
	APMT2_PEER_DISASSOC_REQ    peer_disassoc_action
	APMT2_PEER_ASSOC_REQ       drop
	APMT2_PEER_REASSOC_REQ     drop
	APMT2_CLS3ERR              cls3err_action
    ==========================================================================
 */
VOID APAssocStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, AP_MAX_ASSOC_STATE, AP_MAX_ASSOC_MSG, (STATE_MACHINE_FUNC)Drop,
					 AP_ASSOC_IDLE, AP_ASSOC_MACHINE_BASE);
	StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)APMlmeDisassocReqAction);
	StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)APPeerDisassocReqAction);
	StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_ASSOC_REQ,    (STATE_MACHINE_FUNC)APPeerAssocReqAction);
	StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_PEER_REASSOC_REQ,  (STATE_MACHINE_FUNC)APPeerReassocReqAction);
	/*  StateMachineSetAction(S, AP_ASSOC_IDLE, APMT2_CLS3ERR,           APCls3errAction); */
}

