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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"


#ifdef DOT11_N_SUPPORT

INT ht_support_bw_by_channel_boundary(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, IE_LISTS *peer_ie_list)
{
	struct wifi_dev *wdev = pEntry->wdev;
	UCHAR supported_bw;
	UCHAR ext_cha;

	if (!wdev)
		return HT_BW_20;

	supported_bw = wlan_operate_get_ht_bw(wdev);
	ext_cha = wlan_operate_get_ext_cha(wdev);
	if (!peer_ie_list)
		return supported_bw;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(), Channel = %d, supported_bw = %d, extcha = %d, PeerSupChlLen = %d\n", __func__,
			 wdev->channel,
			 supported_bw,
			 ext_cha,
			 peer_ie_list->SupportedChlLen));

	/* So far, only check 2.4G band */
	if ((wdev->channel <= 14) && (peer_ie_list->SupportedChlLen >= 2)) {
		UCHAR i;
		UCHAR peer_sup_chl_min_2g = 1;
		UCHAR peer_sup_chl_max_2g = 14;

		/* Get Min/Max 2.4G supported channels */
		for (i = 0; i < peer_ie_list->SupportedChlLen; i += 2) {
			if (peer_ie_list->SupportedChl[i] <= 14) {
				peer_sup_chl_min_2g = peer_ie_list->SupportedChl[i];
				peer_sup_chl_max_2g = peer_ie_list->SupportedChl[i] + peer_ie_list->SupportedChl[i + 1] - 1;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("  peer_sup_chl_min_2g = %d, peer_sup_chl_max_2g = %d\n",
						 peer_sup_chl_min_2g, peer_sup_chl_max_2g));
			}
		}

		/* Check Min/Max 2.4G channel boundary in BW40 case */
		if ((supported_bw == BW_40) && (ext_cha != EXTCHA_NONE)) {
			if (((ext_cha == EXTCHA_ABOVE) && ((wdev->channel + 4) > peer_sup_chl_max_2g)) ||
				((ext_cha == EXTCHA_BELOW) && ((wdev->channel - 4) < peer_sup_chl_min_2g)))
				supported_bw = BW_20;
		}
	}

	return supported_bw;
}

INT ht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer_ht_cap)
{
	struct wifi_dev *wdev = pEntry->wdev;
	ADD_HT_INFO_IE *addht;
	HT_CAPABILITY_IE *ht_cap;

	if (!wdev)
		return FALSE;

	addht = wlan_operate_get_addht(wdev);
	ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

	if ((peer_ht_cap->HtCapInfo.GF) && (ht_cap->HtCapInfo.GF))
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
	else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pAd->MacTab.fAnyStationNonGF = TRUE;
	}

	pEntry->MaxHTPhyMode.field.BW = (peer_ht_cap->HtCapInfo.ChannelWidth & wlan_operate_get_ht_bw(wdev));
	if (pEntry->MaxHTPhyMode.field.BW == HT_BW_40) {
		pEntry->MaxHTPhyMode.field.ShortGI =
			((ht_cap->HtCapInfo.ShortGIfor40) & (peer_ht_cap->HtCapInfo.ShortGIfor40));
	} else {
		pEntry->MaxHTPhyMode.field.ShortGI =
			((ht_cap->HtCapInfo.ShortGIfor20) & (peer_ht_cap->HtCapInfo.ShortGIfor20));
		pAd->MacTab.fAnyStation20Only = TRUE;
	}
	pEntry->MaxHTPhyMode.field.STBC = (peer_ht_cap->HtCapInfo.RxSTBC & ht_cap->HtCapInfo.TxSTBC);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(), MODE = %d, BW = %d, SGI = %d, STBC = %d\n", __func__,
			 pEntry->MaxHTPhyMode.field.MODE,
			 pEntry->MaxHTPhyMode.field.BW,
			 pEntry->MaxHTPhyMode.field.ShortGI,
			 pEntry->MaxHTPhyMode.field.STBC));
	return TRUE;
}


INT set_ht_fixed_mcs(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound)
{
	if (fixed_mcs == 32) {
		/* Fix MCS as HT Duplicated Mode */
		pEntry->MaxHTPhyMode.field.BW = 1;
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pEntry->MaxHTPhyMode.field.STBC = 0;
		pEntry->MaxHTPhyMode.field.ShortGI = 0;
		pEntry->MaxHTPhyMode.field.MCS = 32;
	} else if (pEntry->MaxHTPhyMode.field.MCS > mcs_bound) {
		/* STA supports fixed MCS */
		pEntry->MaxHTPhyMode.field.MCS = mcs_bound;
	}

	return TRUE;
}

UINT8 get_max_nss_by_htcap_ie_mcs(UCHAR *cap_mcs)
{
	UINT8 index, nss = 0;
	UCHAR bitmask;

	index = 32;
	do {
		index--;
		nss = index / 8;
		bitmask = (1 << (index - (nss * 8)));

		if (cap_mcs[nss] & bitmask)
			break;
	} while (index);

	return nss;
}

INT get_ht_max_mcs(RTMP_ADAPTER *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs)
{
	INT i, j;
	UCHAR bitmask;

	for (i = 31; i >= 0; i--) {
		j = i / 8;
		bitmask = (1 << (i - (j * 8)));

		if ((desire_mcs[j] & bitmask) && (cap_mcs[j] & bitmask)) {
			/*pEntry->MaxHTPhyMode.field.MCS = i; */
			/* find it !!*/
			break;
		}

		if (i == 0)
			break;
	}

	return i;
}


INT get_ht_cent_ch(RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch, UCHAR Channel)
{
	UCHAR op_ht_bw = HT_BW_20;
	UCHAR op_ext_cha = EXTCHA_NONE;
	UCHAR i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (!wdev)
			continue;

		/*check in the same band*/
		if (Channel == wdev->channel && wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
			op_ht_bw = wlan_operate_get_ht_bw(wdev);
			op_ext_cha = wlan_operate_get_ext_cha(wdev);

			if (op_ht_bw == HT_BW_40)
				break;
		}
	}

	if ((op_ht_bw  == BW_40) &&
		(op_ext_cha == EXTCHA_ABOVE)
	   ) {
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_ABOVE;
	} else if ((Channel > 2) &&
			   (op_ht_bw == BW_40) &&
			   (op_ext_cha == EXTCHA_BELOW)) {
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_BELOW;

	} else
		return FALSE;

	return TRUE;
}

UCHAR cal_ht_cent_ch(UCHAR prim_ch, UCHAR phy_bw, UCHAR ext_cha, UCHAR *cen_ch)
{
	*cen_ch = prim_ch;

	if ((phy_bw  == BW_40) &&
		(ext_cha == EXTCHA_ABOVE)
	   )
		*cen_ch = prim_ch + 2;
	else if ((prim_ch > 2) &&
			 (phy_bw == BW_40) &&
			 (ext_cha == EXTCHA_BELOW)) {
		if (prim_ch == 14)
			*cen_ch = prim_ch - 1;
		else
			*cen_ch = prim_ch - 2;
	} else
		return FALSE;

	return TRUE;
}



UCHAR get_cent_ch_by_htinfo(
	RTMP_ADAPTER *pAd,
	ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap)
{
	UCHAR cent_ch;

	if ((ht_op->ControlChan > 2) &&
		(ht_op->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan - 2;
	else if ((ht_op->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) &&
			 (ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan + 2;
	else
		cent_ch = ht_op->ControlChan;

	return cent_ch;
}


UINT16 ht_max_mpdu_size[2] = {3839u, 7935u};

VOID set_sta_ht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *entry, HT_CAPABILITY_IE *ht_ie)
{
	UCHAR ht_gi = GI_400;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* set HT capabilty flag for entry */
	CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE);

	ht_gi = wlan_config_get_ht_gi(entry->wdev);
	if (ht_gi == GI_400) {
		if (ht_ie->HtCapInfo.ShortGIfor20)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI20_CAPABLE);

		if (ht_ie->HtCapInfo.ShortGIfor40)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_SGI40_CAPABLE);
	}

	if (ht_ie->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_TxSTBC_CAPABLE);

	if (ht_ie->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RxSTBC_CAPABLE);

	if (ht_ie->ExtHtCapInfo.PlusHTC) {
		CLIENT_CAP_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HTC_CAPABLE);
	}

	if (pAd->CommonCfg.bRdg && ht_ie->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_RDG_CAPABLE);

	if (ht_ie->ExtHtCapInfo.MCSFeedback == 0x03)
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

	if (wlan_config_get_ht_ldpc(entry->wdev) && (cap->phy_caps & fPHY_CAP_LDPC)) {
		if (ht_ie->HtCapInfo.ht_rx_ldpc)
			CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE);
	}

	if (wlan_config_get_amsdu_en(entry->wdev) && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
		CLIENT_STATUS_SET_FLAG(entry, fCLIENT_STATUS_AMSDU_INUSED);

	entry->MpduDensity = ht_ie->HtCapParm.MpduDensity;
	entry->MaxRAmpduFactor = ht_ie->HtCapParm.MaxRAmpduFactor;
	entry->MmpsMode = (UCHAR)ht_ie->HtCapInfo.MimoPs;
	entry->AMsduSize = (UCHAR)ht_ie->HtCapInfo.AMsduSize;

	if (entry->AMsduSize < (ARRAY_SIZE(ht_max_mpdu_size)))
		entry->amsdu_limit_len = ht_max_mpdu_size[entry->AMsduSize];
	else
		entry->amsdu_limit_len = 0;
	entry->amsdu_limit_len_adjust = entry->amsdu_limit_len;
}


/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetHT(
	IN RTMP_ADAPTER *pAd,
	IN OID_SET_HT_PHYMODE * pHTPhyMode,
	IN struct wifi_dev *wdev)
{
	INT idx;
#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[CFG_GO_BSSID_IDX];
#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
#ifdef CONFIG_AP_SUPPORT

	/* sanity check for extention channel */
	if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
						   CHANNEL_NO_FAT_BELOW | CHANNEL_NO_FAT_ABOVE) == TRUE) {
		/* only 20MHz is allowed */
		pHTPhyMode->BW = 0;
	} else if (pHTPhyMode->ExtOffset == EXTCHA_BELOW) {
		/* extension channel below this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
							   CHANNEL_NO_FAT_BELOW) == TRUE)
			pHTPhyMode->ExtOffset = EXTCHA_ABOVE;
	} else if (pHTPhyMode->ExtOffset == EXTCHA_ABOVE) {
		/* extension channel above this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pHTPhyMode->Channel,
							   CHANNEL_NO_FAT_ABOVE) == TRUE)
			pHTPhyMode->ExtOffset = EXTCHA_BELOW;
	}
#endif /* CONFIG_AP_SUPPORT */
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
			 pHTPhyMode->HtMode, pHTPhyMode->ExtOffset,
			 pHTPhyMode->MCS, pHTPhyMode->BW,
			 pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));

	/* Decide Rx MCSSet*/
	switch (wlan_config_get_rx_stream(wdev)) {
	case 4:
		ht_cap->MCSSet[3] =  0xff;

	case 3:
		ht_cap->MCSSet[2] =  0xff;

	case 2:
		ht_cap->MCSSet[1] =  0xff;

	case 1:
	default:
		ht_cap->MCSSet[0] =  0xff;
		break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pHTPhyMode->BW == BW_40)) {
		pHTPhyMode->BW = BW_20;
		wlan_config_set_40M_intolerant(wdev, 1);
	}

	/* TODO: shiang-6590, how about the "bw" when channel 14 for JP region??? */
	if (pHTPhyMode->BW == BW_40) {
		UCHAR ext_cha = (pHTPhyMode->ExtOffset == EXTCHA_BELOW) ? (EXTCHA_BELOW) : EXTCHA_ABOVE;

		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/
		wlan_config_set_ht_bw(wdev, HT_BW_40);

		if (pHTPhyMode->Channel <= 14)
			ht_cap->HtCapInfo.CCKmodein40 = 1;

		wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
	} else {
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
	}

#ifdef CONFIG_MULTI_CHANNEL
#if defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT)

	if ((pHTPhyMode->BW == BW_20) && (pHTPhyMode->Channel != 0)) {
		wlan_config_set_ht_bw(wdev, HT_BW_20);
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
	} else if (INFRA_ON(pAd)) {
		struct wifi_dev *p2p_dev = &pAd->StaCfg[0].wdev;
		UCHAR ht_bw = wlan_config_get_ht_bw(p2p_dev);
		UCHAR ext_cha = wlan_config_get_ext_cha(p2p_dev);

		wlan_operate_set_ht_bw(p2p_dev, ht_bw, ext_cha);
	}

#endif /* defined(RT_CFG80211_SUPPORT) && defined(CONFIG_AP_SUPPORT) */
#endif /* CONFIG_MULTI_CHANNEL */

#ifdef TXBF_SUPPORT

	if (cap->FlgHwTxBfCap) {
#ifdef MT_MAC
		/* Set ETxBF */
		mt_WrapSetETxBFCap(pAd, wdev, &ht_cap->TxBFCap);
		/* Check ITxBF */
#else
		/* Set ETxBF */
		setETxBFCap(pAd, &ht_cap->TxBFCap);
		/* Check ITxBF */
		pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn &= rtmp_chk_itxbf_calibration(pAd);
#endif /* MT_MAC */
		/* Apply to ASIC */
	}

#endif /* TXBF_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx);

#ifdef APCLI_SUPPORT

		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
			RTMPSetIndividualHT(pAd, idx + MIN_NET_DEVICE_FOR_APCLI);

#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */
/*	dump_ht_cap(wdev); for Debug purpose */
}

/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetIndividualHT(RTMP_ADAPTER *pAd, UCHAR apidx)
{
	RT_PHY_INFO *pDesired_ht_phy = NULL;
	UCHAR TxStream = 1;
	UCHAR DesiredMcs = MCS_AUTO;
	UINT32 encrypt_mode = 0;
	struct wifi_dev *wdev = NULL;
	UCHAR cfg_ht_bw;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): apidx=%d\n", __func__, apidx));

	do {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

		if (apidx >= MIN_NET_DEVICE_FOR_APCLI) {
			UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;

			if (idx < MAX_APCLI_NUM) {
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				if (!wdev)
					return;
				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
				encrypt_mode = wdev->SecConfig.PairwiseCipher;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			} else {
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid idx(%d)\n", __func__, idx));
				return;
			}
		}

#endif /* APCLI_SUPPORT */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef WDS_SUPPORT

			if (apidx >= MIN_NET_DEVICE_FOR_WDS) {
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;

				if (idx < MAX_WDS_ENTRY) {
					wdev = &pAd->WdsTab.WdsEntry[idx].wdev;
					if (!wdev)
						return;
					pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
					DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
					/*encrypt_mode = pAd->WdsTab.WdsEntry[idx].WepStatus;*/
					wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				} else {
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __func__, apidx));
					return;
				}
			}

#endif /* WDS_SUPPORT */

			if ((apidx < pAd->ApCfg.BssidNum) && (apidx < MAX_MBSSID_NUM(pAd)) && (apidx < HW_BEACON_MAX_NUM)) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				if (!wdev)
					return;
				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
#ifdef WFA_VHT_PF

				/* TODO: Sigma, this code segment used to work around for Sigma Automation! */
				if (WMODE_CAP_AC(wdev->PhyMode) && (DesiredMcs != MCS_AUTO)) {
					DesiredMcs += ((wlan_operate_get_tx_stream(wdev) - 1) << 4);
					pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.FixedTxMode = FIXED_TXMODE_VHT;
					RT_CfgSetAutoFallBack(pAd, "0");
				} else
					RT_CfgSetAutoFallBack(pAd, "1");

#endif /* WFA_VHT_PF */
				encrypt_mode = wdev->SecConfig.PairwiseCipher;
				wdev->bWmmCapable = TRUE;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}

			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __func__, apidx));
			return;
		}
#endif /* CONFIG_AP_SUPPORT */
	} while (FALSE);


	if (pDesired_ht_phy == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: invalid apidx(%d)\n", __func__, apidx));
		return;
	}

	TxStream = wlan_operate_get_tx_stream(wdev);

	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_PHY_INFO));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPSetIndividualHT : Desired MCS = %d\n", DesiredMcs));

	/* Check the validity of MCS */
	if ((TxStream == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15))) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS(%d) is invalid in 1S, reset it as MCS_7\n", __func__, DesiredMcs));
		DesiredMcs = MCS_7;
	}

	cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	if ((cfg_ht_bw == HT_BW_20) && (DesiredMcs == MCS_32)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n", __func__));
		DesiredMcs = MCS_0;
	}


		/*
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode)) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s : Use legacy rate in WEP/TKIP encryption mode (apidx=%d)\n",
					 __func__, apidx));
			return;
		}

	if (pAd->CommonCfg.HT_Disable) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : HT is disabled\n", __func__));
		return;
	}

	pDesired_ht_phy->bHtEnable = TRUE;

	/* Decide desired Tx MCS*/
	switch (TxStream) {
	case 1:
		if (DesiredMcs == MCS_AUTO)
			pDesired_ht_phy->MCSSet[0] = 0xff;
		else if (DesiredMcs <= MCS_7)
			pDesired_ht_phy->MCSSet[0] = 1 << DesiredMcs;

		break;

	case 2:
		if (DesiredMcs == MCS_AUTO) {
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
		} else if (DesiredMcs <= MCS_15) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 2)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;

	case 3:
		if (DesiredMcs == MCS_AUTO) {
			/* MCS0 ~ MCS23, 3 bytes */
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
			pDesired_ht_phy->MCSSet[2] = 0xff;
		} else if (DesiredMcs <= MCS_23) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 3)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;

	case 4:
		if (DesiredMcs == MCS_AUTO) {
			/* MCS0 ~ MCS31, 4 bytes */
			pDesired_ht_phy->MCSSet[0] = 0xff;
			pDesired_ht_phy->MCSSet[1] = 0xff;
			pDesired_ht_phy->MCSSet[2] = 0xff;
			pDesired_ht_phy->MCSSet[3] = 0xff;
		} else if (DesiredMcs <= MCS_31) {
			ULONG mode;

			mode = DesiredMcs / 8;

			if (mode < 4)
				pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
		}

		break;
	}

	if (cfg_ht_bw == HT_BW_40) {
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	/* update HT Rate setting */
	if (pAd->OpMode == OPMODE_STA) {
			MlmeUpdateHtTxRates(pAd, BSS0);
	} else
		MlmeUpdateHtTxRates(pAd, apidx);

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode)) {
		pDesired_ht_phy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, wdev, pDesired_ht_phy);
	}

#endif /* DOT11_VHT_AC */
}

/*
	========================================================================
	Routine Description:
		Clear the desire HT info per interface

	Arguments:

	========================================================================
*/
VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
		RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
}


INT	SetCommonHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	OID_SET_HT_PHYMODE SetHT;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (!WMODE_CAP_N(wdev->PhyMode)) {
		/* Clear previous HT information */
		RTMPDisableDesiredHtInfo(pAd, wdev);
		return FALSE;
	}

	SetHT.PhyMode = (RT_802_11_PHY_MODE)(wdev->PhyMode);
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.MCS = MCS_AUTO;
	SetHT.STBC = wlan_config_get_ht_stbc(wdev);
	SetHT.SHORTGI = (UCHAR)wlan_config_get_ht_gi(wdev);
	SetHT.Channel = wdev->channel;
	SetHT.BandIdx = 0;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_BW_SYNC_FEATURE_ENBL(pAd)) {
		SetHT.BW = wlan_config_get_ht_bw(wdev);
		SetHT.ExtOffset = (SetHT.BW == BW_20) ? (0) : (ext_cha);
	} else {
#endif
	SetHT.BW = (UCHAR)op_ht_bw;
	SetHT.ExtOffset = ext_cha;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	}
#endif

	RTMPSetHT(pAd, &SetHT, wdev);
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.bBssCoexEnable && pAd->CommonCfg.Bss2040NeedFallBack) {
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
	}

#endif /* DOT11N_DRAFT3 */
	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Update HT IE from our capability.

	Arguments:
		Send all HT IE in beacon/probe rsp/assoc rsp/action frame.


	========================================================================
*/
VOID RTMPUpdateHTIE(
	IN UCHAR *pMcsSet,
	IN struct wifi_dev *wdev,
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT ADD_HT_INFO_IE *pAddHtInfo)
{
	UCHAR cfg_ht_bw  = wlan_config_get_ht_bw(wdev);
	UCHAR op_ht_bw = wlan_config_get_ht_bw(wdev);
	HT_CAPABILITY_IE *ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	struct _ADD_HT_INFO_IE *add_ht = wlan_operate_get_addht(wdev);

	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	pHtCapability->HtCapInfo.ChannelWidth = cfg_ht_bw;
	pHtCapability->HtCapInfo.MimoPs = ht_cap->HtCapInfo.MimoPs;
	pHtCapability->HtCapInfo.GF = ht_cap->HtCapInfo.GF;
	pHtCapability->HtCapInfo.ShortGIfor20 = ht_cap->HtCapInfo.ShortGIfor20;
	pHtCapability->HtCapInfo.ShortGIfor40 = ht_cap->HtCapInfo.ShortGIfor40;
	pHtCapability->HtCapInfo.TxSTBC = ht_cap->HtCapInfo.TxSTBC;
	pHtCapability->HtCapInfo.RxSTBC = ht_cap->HtCapInfo.RxSTBC;
	pHtCapability->HtCapInfo.AMsduSize = ht_cap->HtCapInfo.AMsduSize;
	pHtCapability->HtCapParm.MaxRAmpduFactor = ht_cap->HtCapParm.MaxRAmpduFactor;
	pHtCapability->HtCapParm.MpduDensity = ht_cap->HtCapParm.MpduDensity;
	pAddHtInfo->AddHtInfo.ExtChanOffset = add_ht->AddHtInfo.ExtChanOffset;
	pAddHtInfo->AddHtInfo.RecomWidth = op_ht_bw;
	pAddHtInfo->AddHtInfo2.OperaionMode = add_ht->AddHtInfo2.OperaionMode;
	pAddHtInfo->AddHtInfo2.NonGfPresent = add_ht->AddHtInfo2.NonGfPresent;
	RTMPMoveMemory(pAddHtInfo->MCSSet, pMcsSet, 4); /* rt2860 only support MCS max=32, no need to copy all 16 uchar.*/
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPUpdateHTIE <==\n"));
}

/*sta rec ht features decision*/
UINT32 starec_ht_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 features = 0;
	UCHAR amsdu_en = wlan_config_get_amsdu_en(wdev);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if ((cap->SupportAMSDU == TRUE) && amsdu_en) {
		features |= STA_REC_AMSDU_FEATURE;
#ifdef HW_TX_AMSDU_SUPPORT
		/* notify FW the entry is support HW AMSDU and config HW AMSDU parameter (length and number) */
		features |= STA_REC_HW_AMSDU_FEATURE;
#endif
		entry->tx_amsdu_bitmap = 0xffff;
	} else {
		entry->tx_amsdu_bitmap = 0x0;
	}

	if (CLIENT_STATUS_TEST_FLAG(entry, fCLIENT_STATUS_HT_CAPABLE))
		features |= STA_REC_BASIC_HT_INFO_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}

static INT build_bcm_ht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	const UCHAR BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
	UCHAR HtLen;
	INT len = 0;
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	HtLen = SIZE_HT_CAP_IE + 4; /* including BCM IE */
	NdisMoveMemory(&HtCapabilityTmp, curr_ht_cap, SIZE_HT_CAP_IE);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

	if (HcIsBfCapSupport(wdev) == FALSE) {
		UCHAR ucEBfCap;

		ucEBfCap = wlan_config_get_etxbf(wdev);
		wlan_config_set_etxbf(wdev, SUBF_OFF);
		mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
		wlan_config_set_etxbf(wdev, ucEBfCap);
	}

#ifdef RT_BIG_ENDIAN
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#endif /* RT_BIG_ENDIAN */

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &HtLen, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM, 4, len);
	MAKE_IE_TO_BUF(buf, &HtCapabilityTmp, SIZE_HT_CAP_IE, len);

	return len;
}

static INT build_bcm_ht_op_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;

	const UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};
	UCHAR AddHtLen = 0;
	ADD_HT_INFO_IE *addht;
#ifdef RT_BIG_ENDIAN
	ADD_HT_INFO_IE	addHTInfoTmp;
#endif /* RT_BIG_ENDIAN */
	UCHAR epigram_ie_len = 0;

	addht = wlan_operate_get_addht(wdev);
	AddHtLen = sizeof(ADD_HT_INFO_IE);
	epigram_ie_len = AddHtLen + 4;

	/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &epigram_ie_len, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM_AHTINFO, 4, len)
	MAKE_IE_TO_BUF(buf, &addHTInfoTmp, AddHtLen, len);
#else

	MAKE_IE_TO_BUF(buf, &WpaIe, 1, len);
	MAKE_IE_TO_BUF(buf, &epigram_ie_len, 1, len);
	MAKE_IE_TO_BUF(buf, BROADCOM_AHTINFO, 4, len)
	MAKE_IE_TO_BUF(buf, addht, AddHtLen, len);
#endif /* RT_BIG_ENDIAN */

	return len;
}

static INT build_ht_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	UCHAR HtLen = 0;
	INT len = 0;
	HT_CAPABILITY_IE HtCapabilityTmp;
	HT_CAPABILITY_IE *curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	HtLen = sizeof(HT_CAPABILITY_IE);
	NdisMoveMemory(&HtCapabilityTmp, curr_ht_cap, HtLen);
	HtCapabilityTmp.HtCapInfo.ChannelWidth = cfg_ht_bw;

#ifdef TXBF_SUPPORT
	if (HcIsBfCapSupport(wdev) == FALSE) {
		UCHAR ucEBfCap;

		ucEBfCap = wlan_config_get_etxbf(wdev);
		wlan_config_set_etxbf(wdev, SUBF_OFF);
		mt_WrapSetETxBFCap(pAd, wdev, &HtCapabilityTmp.TxBFCap);
		wlan_config_set_etxbf(wdev, ucEBfCap);
	}
#endif /* TXBF_SUPPORT */

#ifdef RT_BIG_ENDIAN
	*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
	{
		EXT_HT_CAP_INFO extHtCapInfo;

		NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
		*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
		NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));
	}
#else
	*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */
#endif /* RT_BIG_ENDIAN */

	MAKE_IE_TO_BUF(buf, &HtCapIe, 1, len);
	MAKE_IE_TO_BUF(buf, &HtLen, 1, len);
	MAKE_IE_TO_BUF(buf, &HtCapabilityTmp, HtLen, len);

	return len;
}

static INT build_ht_op_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR AddHtLen = 0;
	ADD_HT_INFO_IE *addht;
#ifdef RT_BIG_ENDIAN
	ADD_HT_INFO_IE	addHTInfoTmp;
#endif /* RT_BIG_ENDIAN */

	addht = wlan_operate_get_addht(wdev);
	AddHtLen = sizeof(ADD_HT_INFO_IE);

	/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&addHTInfoTmp, addht, AddHtLen);
	*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
	*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

	MAKE_IE_TO_BUF(buf, &AddHtInfoIe, 1, len);
	MAKE_IE_TO_BUF(buf, &AddHtLen, 1, len);
	MAKE_IE_TO_BUF(buf, &addHTInfoTmp, AddHtLen, len);
#else

	MAKE_IE_TO_BUF(buf, &AddHtInfoIe, 1, len);
	MAKE_IE_TO_BUF(buf, &AddHtLen, 1, len);
	MAKE_IE_TO_BUF(buf, addht, AddHtLen, len);
#endif /* RT_BIG_ENDIAN */

	return len;
}

static INT build_overlapping_bss_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel, UCHAR *buf)
{
	INT len = 0;
	UCHAR cfg_ht_bw = HT_BW_20;

#ifdef DOT11N_DRAFT3
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	/* P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE */
	if ((Channel <= 14) && (cfg_ht_bw == HT_BW_40))	{
		OVERLAP_BSS_SCAN_IE  OverlapScanParam;
		UCHAR OverlapScanIE, ScanIELen;

		OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
		ScanIELen = 14;
		OverlapScanParam.ScanPassiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveDwell);
		OverlapScanParam.ScanActiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveDwell);
		OverlapScanParam.TriggerScanInt = cpu2le16(pAd->CommonCfg.Dot11BssWidthTriggerScanInt);
		OverlapScanParam.PassiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel);
		OverlapScanParam.ActiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel);
		OverlapScanParam.DelayFactor = cpu2le16(pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
		OverlapScanParam.ScanActThre = cpu2le16(pAd->CommonCfg.Dot11OBssScanActivityThre);

		MAKE_IE_TO_BUF(buf, &OverlapScanIE, 1, len);
		MAKE_IE_TO_BUF(buf, &ScanIELen, 1, len);
		MAKE_IE_TO_BUF(buf, &OverlapScanParam, ScanIELen, len);
	}
#endif /* DOT11N_DRAFT3 */

	return len;
}

INT build_ht_ies(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->is_draft_n_type == TRUE)
		len += build_bcm_ht_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else
		len += build_ht_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
		(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
		(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
		if (info->is_draft_n_type == TRUE)
			len += build_bcm_ht_op_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
		else
			len += build_ht_op_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	}

	if ((info->frame_subtype == SUBTYPE_BEACON) ||
		(info->frame_subtype == SUBTYPE_PROBE_RSP) ||
		(info->frame_subtype == SUBTYPE_ASSOC_RSP) ||
		(info->frame_subtype == SUBTYPE_REASSOC_RSP)) {
		len += build_overlapping_bss_ie(pAd, info->wdev, info->channel, (UCHAR *)(info->frame_buf + len));
	}

	return len;
}

#endif /* DOT11_N_SUPPORT */

