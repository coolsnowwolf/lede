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
 *
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT

#include "rt_config.h"
#include <net/cfg80211.h>
#ifdef MT_MAC
VOID write_tmac_info_beacon(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *tmac_buf, HTTRANSMIT_SETTING *BeaconTransmit, ULONG frmLen);

#endif /* MT_MAC */

INT CFG80211_FindMbssApIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev)
{
	USHORT index = 0;
	BOOLEAN found = FALSE;

	for (index = 0; index < MAX_MBSSID_NUM(pAd); index++) {
		if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pNetDev) {
			found = TRUE;
			break;
		}
	}

	return (found) ? index : WDEV_NOT_FOUND;
}


INT CfgAsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetPreTbtt(pAd, enable, 1);

#endif
	return FALSE;
}
static INT CFG80211DRV_UpdateTimIE(PRTMP_ADAPTER pAd, UINT mbss_idx, PUCHAR pBeaconFrame, UINT32 tim_ie_pos)
{
	UCHAR  ID_1B, TimFirst, TimLast, *pTim, *ptr, New_Tim_Len;
	UINT  i;
	struct wifi_dev *wdev = NULL;
	BCN_BUF_STRUC *bcn_buf = NULL;

	ptr = pBeaconFrame + tim_ie_pos; /* TIM LOCATION */
	*ptr = IE_TIM;
	*(ptr + 2) = pAd->ApCfg.DtimCount;
	*(ptr + 3) = pAd->ApCfg.DtimPeriod;
	TimFirst = 0; /* record first TIM byte != 0x00 */
	TimLast = 0;  /* record last  TIM byte != 0x00 */
	wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
	bcn_buf = &wdev->bcn_buf;
	pTim = bcn_buf->TimBitmaps;

	for (ID_1B = 0; ID_1B < WLAN_MAX_NUM_OF_TIM; ID_1B++) {
		/* get the TIM indicating PS packets for 8 stations */
		UCHAR tim_1B = pTim[ID_1B];

		if (ID_1B == 0)
			tim_1B &= 0xfe; /* skip bit0 bc/mc */

		if (tim_1B == 0)
			continue; /* find next 1B */

		if (TimFirst == 0)
			TimFirst = ID_1B;

		TimLast = ID_1B;
	}

	/* fill TIM content to beacon buffer */
	if (TimFirst & 0x01)
		TimFirst--; /* find the even offset byte */

	*(ptr + 1) = 3 + (TimLast - TimFirst + 1); /* TIM IE length */
	*(ptr + 4) = TimFirst;

	for (i = TimFirst; i <= TimLast; i++)
		*(ptr + 5 + i - TimFirst) = pTim[i];

	/* bit0 means backlogged mcast/bcast */
	if (pAd->ApCfg.DtimCount == 0)
		*(ptr + 4) |= (bcn_buf->TimBitmaps[WLAN_CT_TIM_BCMC_OFFSET] & 0x01);

	/* adjust BEACON length according to the new TIM */
	New_Tim_Len = (2 + *(ptr + 1));
	return New_Tim_Len;
}

static INT CFG80211DRV_UpdateApSettingFromBeacon(PRTMP_ADAPTER pAd, UINT mbss_idx, CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[mbss_idx];
	struct wifi_dev *wdev = &pMbss->wdev;
	const UCHAR *ssid_ie = NULL, *wpa_ie = NULL, *rsn_ie = NULL;
	const UCHAR *supp_rates_ie = NULL;
	const UCHAR *ext_supp_rates_ie = NULL, *ht_cap = NULL, *ht_info = NULL;

#ifdef CONFIG_AP_SUPPORT
    INT idx;
#endif

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	const UCHAR *dsparam_ie = NULL, *ht_operation = NULL, *vht_operation = NULL;
	PADD_HT_INFO_IE phtinfo;
	VHT_OP_IE	*vhtinfo;
	UCHAR channel = 0;
	PEID_STRUCT pEid;
#endif
#endif

#ifdef DISABLE_HOSTAPD_BEACON
	const UCHAR *wsc_ie = NULL;
	const UINT WFA_OUI = 0x0050F2;
#endif
#ifdef HOSTAPD_11R_SUPPORT
	const UCHAR *md_ie = NULL;
#endif /* HOSTAPD_11R_SUPPORT */

#ifdef HOSTAPD_OWE_SUPPORT
	const UCHAR *trans_ie = NULL;

	const UINT OUI_WFA = 0x506f9a;

	UINT8 OWE_OUI_TYPE = 28;
#endif

#ifdef HOSTAPD_11K_SUPPORT
    const UCHAR *rrm_caps = NULL;
#endif




#if (KERNEL_VERSION(3, 5, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_OPERATION;
#else
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_INFORMATION;
#endif /* LINUX_VERSION_CODE: 3.5.0 */
#ifdef HOSTAPD_11K_SUPPORT
    const UCHAR CFG_RRM_OP_EID = WLAN_EID_RRM_ENABLED_CAPABILITIES;
#endif

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
	const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	if (WMODE_CAP_2G(wdev->PhyMode))
		channel = HcGetChannelByRf(pAd, RFIC_24GHZ);
	else
		channel = HcGetChannelByRf(pAd, RFIC_5GHZ);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Channel from Auto selection is :%d\n", channel));
#endif
#endif

	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	supp_rates_ie = cfg80211_find_ie(WLAN_EID_SUPP_RATES, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	/* if it doesn't find WPA_IE in tail first 30 bytes. treat it as is not found */
	wpa_ie = cfg80211_find_ie(CFG_WPA_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	rsn_ie = cfg80211_find_ie(WLAN_EID_RSN, pBeacon->beacon_tail, pBeacon->beacon_tail_len);/* wpa2 case. */
	ext_supp_rates_ie = cfg80211_find_ie(WLAN_EID_EXT_SUPP_RATES, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_cap = cfg80211_find_ie(WLAN_EID_HT_CAPABILITY, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_info = cfg80211_find_ie(CFG_HT_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);

#ifdef HOSTAPD_11K_SUPPORT
	rrm_caps = cfg80211_find_ie(CFG_RRM_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#endif
#ifdef HOSTAPD_11R_SUPPORT
	md_ie = cfg80211_find_ie(WLAN_EID_MOBILITY_DOMAIN, pBeacon->beacon_tail, pBeacon->beacon_tail_len); /* WLAN_EID_MOBILITY_DOMAIN=54 */
#endif
#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	dsparam_ie = cfg80211_find_ie(WLAN_EID_DS_PARAMS, pBeacon->beacon_head+36, pBeacon->beacon_head_len-36);
	ht_operation = cfg80211_find_ie(WLAN_EID_HT_OPERATION, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	vht_operation = cfg80211_find_ie(WLAN_EID_VHT_OPERATION, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
#endif
#endif

#ifdef HOSTAPD_11K_SUPPORT
	if (rrm_caps != NULL) {

		EID_STRUCT *eid;
		INT loop;
		UINT8 bit_nr, bit_lci;
		PRRM_CONFIG pRrmCfg;

		eid = (EID_STRUCT *)rrm_caps;

		printk("RRM : len %d eid %d octet %d\n", eid->Len, eid->Eid, eid->Octet[0]);
		pRrmCfg = &pAd->ApCfg.MBSSID[loop].RrmCfg;

		pMbss->RrmCfg.bDot11kRRMEnable = 1;
		pMbss->RrmCfg.bDot11kRRMEnableSet = 1;

		bit_nr = (eid->Octet[0] >> 1) & 1; /*checking bit position 1:neighbor report */
		bit_lci = (eid->Octet[1] >> 4) & 1; /*checking LCI */

		printk("bit_nr bit_lci: %d %d\n", bit_nr, bit_lci);

		if (bit_nr)
			pRrmCfg->hstapd_nei_rep = TRUE;

		if (bit_lci)
			pRrmCfg->hstapd_lci = TRUE;

		for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
			init_rrm_capabilities(pRrmCfg, &pAd->ApCfg.MBSSID[loop]);
		}

		if (pRrmCfg->hstapd_nei_rep)
			pRrmCfg->hstapd_nei_rep = FALSE;

		if (pRrmCfg->hstapd_lci)
			pRrmCfg->hstapd_lci = FALSE;
		} else
			printk("No RRM capabilities enabled from hostapd\n");
#endif

#ifdef HOSTAPD_OWE_SUPPORT
		/*owe trans oui */
	trans_ie = (UCHAR *)cfg80211_find_vendor_ie(OUI_WFA, OWE_OUI_TYPE,
			pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	if (trans_ie != NULL) {
		EID_STRUCT *eid;

		eid = (EID_STRUCT *)trans_ie;
		if (eid->Len + 2 <= MAX_LEN_OF_TRANS_IE) {
			NdisCopyMemory(pMbss->TRANS_IE, trans_ie, eid->Len+2);
			pMbss->TRANSIE_Len = eid->Len + 2;
		}
	}
#endif

	/* SSID */

	if (ssid_ie == NULL) {
		os_move_mem(pMbss->Ssid, "CFG_Linux_GO", 12);
		pMbss->SsidLen = 12;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: SSID Not Found In Packet\n"));
	} else if (pBeacon->ssid_len != 0) {
		os_zero_mem(pMbss->Ssid, MAX_LEN_OF_SSID + 1);
		NdisZeroMemory(pMbss->Ssid, MAX_LEN_OF_SSID + 1);
		pMbss->SsidLen = pBeacon->ssid_len;
		NdisCopyMemory(pMbss->Ssid, ssid_ie + 2, pMbss->SsidLen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nCFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen));
	}

#ifdef DISABLE_HOSTAPD_BEACON
		else if (*(ssid_ie+1) != 0) {
		os_zero_mem(pMbss->Ssid, pMbss->SsidLen);
		NdisZeroMemory(pMbss->Ssid, pMbss->SsidLen);
		pMbss->SsidLen = *(ssid_ie+1);
		NdisCopyMemory(pMbss->Ssid, ssid_ie+2, pMbss->SsidLen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nCFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen));
	}
		wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
		if (pMbss->WscIEBeacon.Value) {
			pAd->ApCfg.MBSSID[mbss_idx].WscIEBeacon.ValueLen = 0;
			pAd->ApCfg.MBSSID[mbss_idx].WscIEProbeResp.ValueLen = 0;
		}
		if (wsc_ie != NULL) {
#ifdef HOSTAPD_MAP_SUPPORT
			if (IS_MAP_ENABLE(pAd) && wdev &&
				(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_BACKHAUL_BSS)) &&
				(!(wdev->MAPCfg.DevOwnRole & BIT(MAP_ROLE_FRONTHAUL_BSS)))) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Strictly BH BSS: %s, donot BC WPS cap\n", pMbss->Ssid));
			} else
#endif
			{
				EID_STRUCT *eid;

				eid = (EID_STRUCT *)wsc_ie;

				if (eid->Len + 2 <= 500) {
					NdisCopyMemory(pMbss->WscIEBeacon.Value, wsc_ie, eid->Len+2);
					pMbss->WscIEBeacon.ValueLen = eid->Len + 2;
				}
			}
		}

#ifdef HOSTAPD_11R_SUPPORT
		if (md_ie != NULL) {
			PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[mbss_idx].wdev.FtCfg;
			NdisCopyMemory(pFtCfg->FtMdId, md_ie+2, FT_MDID_LEN);
			pFtCfg->FtCapFlag.FtOverDs = (0x01)&(*(md_ie + 4));
			pFtCfg->FtCapFlag.RsrReqCap = (0x02)&(*(md_ie + 4));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("vikas %s::MDID::%x %x FToverDS:%d RsrCap:%d\n", __func__,
				pFtCfg->FtMdId[0], pFtCfg->FtMdId[1],
				pFtCfg->FtCapFlag.FtOverDs, pFtCfg->FtCapFlag.RsrReqCap));
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:MDIE is NULL\n", __func__));

#endif /* HOSTAPD_11R_SUPPORT */
#endif

#ifndef DISABLE_HOSTAPD_BEACON
#ifdef HOSTAPD_AUTO_CH_SUPPORT
	if (dsparam_ie != NULL) {
		pEid = (PEID_STRUCT)dsparam_ie;
		*pEid->Octet = channel;
	}

	if (ht_operation != NULL) {
		pEid = (PEID_STRUCT)ht_operation;
		phtinfo = (PADD_HT_INFO_IE)pEid->Octet;
		phtinfo->ControlChan = channel;
		phtinfo->AddHtInfo.RecomWidth = wlan_operate_get_ht_bw(&pMbss->wdev);
		/* phtinfo->AddHtInfo.ExtChanOffset = 3; */
		phtinfo->AddHtInfo.ExtChanOffset = HcGetExtCha(pAd, channel);
	}
	if (vht_operation != NULL) {
		UCHAR bw = pAd->CommonCfg.vht_bw;
		UCHAR cent_ch = vht_cent_ch_freq(channel, bw);
		pEid = (PEID_STRUCT)vht_operation;
		vhtinfo = (VHT_OP_IE *)pEid->Octet;

		switch (bw) {
		case  VHT_BW_2040:
			vhtinfo->vht_op_info.ch_width = 0;
			vhtinfo->vht_op_info.center_freq_1 = 0;
			vhtinfo->vht_op_info.center_freq_2 = 0;
			break;

		case VHT_BW_80:
			vhtinfo->vht_op_info.ch_width = 1;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = 0;
			break;

		case VHT_BW_160:
			vhtinfo->vht_op_info.ch_width = 2;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
			break;

		case VHT_BW_8080:

			vhtinfo->vht_op_info.ch_width = 3;
			vhtinfo->vht_op_info.center_freq_1 = cent_ch;
			vhtinfo->vht_op_info.center_freq_2 = pAd->CommonCfg.vht_cent_ch2;
			break;
		}
	}

#endif
#endif


#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)

	if ((pBeacon->hidden_ssid > 0 && pBeacon->hidden_ssid < 3) || (pMbss->bHideSsid)) {
		pMbss->bHideSsid = TRUE;

		if ((pBeacon->ssid_len != 0)
			&& (pBeacon->ssid_len <= MAX_LEN_OF_SSID)) {
			pMbss->SsidLen = pBeacon->ssid_len;
			NdisCopyMemory(pMbss->Ssid, pBeacon->ssid, pMbss->SsidLen);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> [Hidden] SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen));
		}
	} else
		pMbss->bHideSsid = FALSE;

#endif /* LINUX_VERSION_CODE 3.4.0 */
	/* WMM EDCA Paramter */
	CFG80211_SyncPacketWmmIe(pAd, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	pMbss->RSNIE_Len[0] = 0;
	pMbss->RSNIE_Len[1] = 0;
	NdisZeroMemory(pMbss->RSN_IE[0], MAX_LEN_OF_RSNIE);
	NdisZeroMemory(pMbss->RSN_IE[1], MAX_LEN_OF_RSNIE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("80211> pBeacon->privacy = %d\n", pBeacon->privacy));

	if (pBeacon->privacy) {
		/* Security */
		if (pBeacon->auth_type == NL80211_AUTHTYPE_SHARED_KEY) {
			/* Shared WEP */
			/* wdev->WepStatus = Ndis802_11WEPEnabled; */
			/* wdev->AuthMode = Ndis802_11AuthModeShared; */
			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_CIPHER(wdev->SecConfig.PairwiseCipher);
			CLEAR_CIPHER(wdev->SecConfig.GroupCipher);
			SET_AKM_SHARED(wdev->SecConfig.AKMMap);
			SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\nCFG80211 BEACON => AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
					  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher)));
		} else
			CFG80211_ParseBeaconIE(pAd, pMbss, wdev, (UCHAR *)wpa_ie, (UCHAR *)rsn_ie);


		if ((IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)) &&
			(IS_AKM_OPEN(wdev->SecConfig.AKMMap))) {
			/* WEP Auto */
			/* wdev->WepStatus = Ndis802_11WEPEnabled; */
			/* wdev->AuthMode = Ndis802_11AuthModeAutoSwitch; */
			CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
			CLEAR_CIPHER(wdev->SecConfig.PairwiseCipher);
			CLEAR_CIPHER(wdev->SecConfig.GroupCipher);
			SET_AKM_OPEN(wdev->SecConfig.AKMMap);
			SET_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap);
			SET_CIPHER_WEP(wdev->SecConfig.PairwiseCipher);
			SET_CIPHER_WEP(wdev->SecConfig.GroupCipher);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\nCFG80211 BEACON => AuthMode = %s ,wdev->PairwiseCipher = %s wdev->SecConfig.GroupCipher = %s\n"
					  , GetAuthModeStr(wdev->SecConfig.AKMMap), GetEncryModeStr(wdev->SecConfig.PairwiseCipher), GetEncryModeStr(wdev->SecConfig.GroupCipher)));
		}

	} else {
		/* wdev->WepStatus = Ndis802_11EncryptionDisabled; */
		/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		CFG80211_ParseBeaconIE(pAd, pMbss, wdev, (UCHAR *)wpa_ie, (UCHAR *)rsn_ie);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
			RTMPSetIndividualHT(pAd, idx);
	}
#endif /* CONFIG_AP_SUPPORT */

	pMbss->CapabilityInfo =	CAP_GENERATE(1, 0, (!IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)),
										 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1), pAd->CommonCfg.bUseShortSlotTime, /*SpectrumMgmt*/FALSE);

#ifdef DOT11K_RRM_SUPPORT
	if (IS_RRM_ENABLE(wdev))
		pMbss->CapabilityInfo |= RRM_CAP_BIT;
#endif /* DOT11K_RRM_SUPPORT */


	/* Disable Driver-Internal Rekey */
	pMbss->WPAREKEY.ReKeyInterval = 0;
	pMbss->WPAREKEY.ReKeyMethod = DISABLE_REKEY;

#ifndef DISABLE_HOSTAPD_BEACON
    if (pBeacon->interval != 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG_TIM New BI %d\n", pBeacon->interval));
		pAd->CommonCfg.BeaconPeriod = pBeacon->interval;
	}

	if (pBeacon->dtim_period != 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG_TIM New DP %d\n", pBeacon->dtim_period));
		pAd->ApCfg.DtimPeriod = pBeacon->dtim_period;
	}
#endif
	return TRUE;
}

VOID CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd)
{
	UINT apidx = MAIN_MBSSID;
	/*CFG_TODO: IT Should be set fRTMP_ADAPTER_HALT_IN_PROGRESS */
	struct wifi_dev *pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.bBcnSntReq = FALSE;

	/* For AP - STA switch */
	if (wlan_operate_get_vht_bw(pWdev) != BW_40) {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> %s, switch to BW_20\n", __func__));
		wlan_operate_set_ht_bw(pWdev, HT_BW_20, EXTCHA_NONE);
	}

	/* Disable pre-TBTT interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);

	if (1) { /* !INFRA_ON(pAd)) */
		/* Disable piggyback */
		AsicSetPiggyBack(pAd, FALSE);
		AsicUpdateProtect(pAd);
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		AsicDisableSync(pAd, HW_BSSID_0);

	/* beacon exit for USB/SDIO */
	asic_bss_beacon_exit(pAd);

	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
}

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
PCHAR rtstrstr2(PCHAR s1, const PCHAR s2, INT s1_len, INT s2_len)
{
	INT offset = 0;

	while (s1_len >= s2_len) {
		s1_len--;

		if (!memcmp(s1, s2, s2_len))
			return offset;

		s1++;
		offset++;
	}

	return NULL;
}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

VOID CFG80211_UpdateBeacon(
	VOID                                            *pAdOrg,
	UCHAR										    *beacon_head_buf,
	UINT32											beacon_head_len,
	UCHAR										    *beacon_tail_buf,
	UINT32											beacon_tail_len,
	BOOLEAN											isAllUpdate,
	UINT32											apidx)

{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	HTTRANSMIT_SETTING BeaconTransmit;   /* MGMT frame PHY rate setting when operatin at Ht rate. */
	PUCHAR pBeaconFrame;
	UCHAR *tmac_info, New_Tim_Len = 0;
	UINT32 beacon_len = 0;
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;
	COMMON_CONFIG *pComCfg;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	ULONG	Value;
	ULONG	TimeTillTbtt;
	ULONG	temp;
	INT		bufferoffset = 0;
	USHORT		bufferoffset2 = 0;
	CHAR	temp_buf[512] = {0};
	CHAR	P2POUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x9};
	INT	temp_len;
	INT P2P_IE = 4;
	USHORT p2p_ie_len;
	UCHAR Count;
	ULONG StartTime;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UCHAR tx_hw_hdr_len = cap->tx_hw_hdr_len;
	UINT8 TXWISize = cap->TXWISize;
	BCN_BUF_STRUC *pbcn_buf = NULL;

	pComCfg = &pAd->CommonCfg;
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	pbcn_buf = &wdev->bcn_buf;

	if (!pMbss || !pMbss->wdev.bcn_buf.BeaconPkt)
		return;

	RTMP_SEM_LOCK(&pbcn_buf->BcnContentLock);
	tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->wdev.bcn_buf.BeaconPkt);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		pBeaconFrame = (UCHAR *)(tmac_info + tx_hw_hdr_len);
	else
#endif /* MT_MAC */
	{
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);
	}

	if (isAllUpdate) { /* Invoke From CFG80211 OPS For setting Beacon buffer */
		/* 1. Update the Buf before TIM IE */
		NdisCopyMemory(pBeaconFrame, beacon_head_buf, beacon_head_len);
		/* 2. Update the Location of TIM IE */
		pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon = beacon_head_len;

		/* 3. Store the Tail Part For appending later */
		if (pCfg80211_ctrl->beacon_tail_buf != NULL)
			os_free_mem(pCfg80211_ctrl->beacon_tail_buf);

		os_alloc_mem(NULL, (UCHAR **)&pCfg80211_ctrl->beacon_tail_buf, beacon_tail_len);

		if (pCfg80211_ctrl->beacon_tail_buf != NULL) {
			NdisCopyMemory(pCfg80211_ctrl->beacon_tail_buf, beacon_tail_buf, beacon_tail_len);
			pCfg80211_ctrl->beacon_tail_len = beacon_tail_len;
		} else {
			pCfg80211_ctrl->beacon_tail_len = 0;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211 Beacon: MEM ALLOC ERROR\n"));
		}

		/* return; */
	} else { /* Invoke From Beacon Timer */
		if (pAd->ApCfg.DtimCount == 0)
			pAd->ApCfg.DtimCount = pAd->ApCfg.DtimPeriod - 1;
		else
			pAd->ApCfg.DtimCount -= 1;

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		/*
		 *	3 mode:
		 *		1. infra scan  7 channel  ( Duration(30+3) *7   interval (+120)  *   count  1 ),
		 *		2. p2p find    3 channel   (Duration (65 ) *3     interval (+130))  * count 2   > 120 sec
		 *		3. mcc  tw channel switch (Duration )  (Infra time )  interval (+ GO time )  count 3  mcc enabel always;
		 */
		if (pAd->cfg80211_ctrl.GONoASchedule.Count > 0) {
			if (pAd->cfg80211_ctrl.GONoASchedule.Count != 200)
				pAd->cfg80211_ctrl.GONoASchedule.Count--;

			os_move_mem(temp_buf, pCfg80211_ctrl->beacon_tail_buf, pCfg80211_ctrl->beacon_tail_len);
			bufferoffset = rtstrstr2(temp_buf, P2POUIBYTE, pCfg80211_ctrl->beacon_tail_len, P2P_IE);

			while (bufferoffset2 <= (pCfg80211_ctrl->beacon_tail_len - bufferoffset - 4 - bufferoffset2 - 3)) {
				if ((pCfg80211_ctrl->beacon_tail_buf)[bufferoffset + 4 + bufferoffset2] == 12)
					break;

				bufferoffset2 = pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + 1 + bufferoffset2] + bufferoffset2;
				bufferoffset2 = bufferoffset2 + 3;
			}

			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 5], &pAd->cfg80211_ctrl.GONoASchedule.Count, 1);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 6], &pAd->cfg80211_ctrl.GONoASchedule.Duration, 4);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 10], &pAd->cfg80211_ctrl.GONoASchedule.Interval, 4);
			NdisCopyMemory(&pCfg80211_ctrl->beacon_tail_buf[bufferoffset + 4 + bufferoffset2 + 14], &pAd->cfg80211_ctrl.GONoASchedule.StartTime, 4);
		}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	}

#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd)) {
			BOOLEAN is_pretbtt_int = FALSE;
			UCHAR resource_idx = 0;
			USHORT FreeNum;

			if (pAd->archOps.get_resource_idx)
				resource_idx = pAd->archOps.get_resource_idx(pAd, wdev, TX_DATA, 0);

			FreeNum = GET_BCNRING_FREENO(pAd, resource_idx);

			if (FreeNum < 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()=>BSS0:BcnRing FreeNum is not enough!\n",
						 __func__));
				return;
			}

			if (pMbss->wdev.bcn_buf.bcn_state != BCN_TX_IDLE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s()=>BSS0:BcnPkt not idle(%d)!\n",
						 __func__, pMbss->wdev.bcn_buf.bcn_state));
				APCheckBcnQHandler(pAd, apidx, &is_pretbtt_int);

				if (is_pretbtt_int == FALSE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("==============> pretbtt_int not init\n"));
					return;
				}
			}
		}

#endif /* RTMP_PCI_SUPPORT */
	}

#endif /* MT_MAC */
	/* 4. Update the TIM IE */
	New_Tim_Len = CFG80211DRV_UpdateTimIE(pAd, apidx, pBeaconFrame,
										  pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon);

	/* 5. Update the Buffer AFTER TIM IE */
	if (pCfg80211_ctrl->beacon_tail_buf != NULL) {
		NdisCopyMemory(pBeaconFrame + pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon + New_Tim_Len,
					   pCfg80211_ctrl->beacon_tail_buf, pCfg80211_ctrl->beacon_tail_len);
		beacon_len = pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon + pCfg80211_ctrl->beacon_tail_len
					 + New_Tim_Len;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BEACON ====> CFG80211_UpdateBeacon OOPS\n"));
		return;
	}

	BeaconTransmit.word = 0;
	/* Should be Find the P2P IE Then Set Basic Rate to 6M */
		BeaconTransmit.field.MODE = MODE_CCK;

	BeaconTransmit.field.MCS = MCS_RATE_6;
	write_tmac_info_beacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, tmac_info, &BeaconTransmit, beacon_len);
	RTMP_SEM_UNLOCK(&pbcn_buf->BcnContentLock);
	/* CFG_TODO */
#ifdef BCN_OFFLOAD_SUPPORT

	if (cap->fgBcnOffloadSupport == TRUE) {
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
#ifndef RT_CFG80211_SUPPORT
	RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
		pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_V2_BCN);
#else
	pAd->archOps.archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, PKT_BCN);
#endif

#else
#ifdef RT_CFG80211_SUPPORT
	pAd->archOps.archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, PKT_BCN);
#else
	RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_BCN);
#endif /* RT_CFG80211_SUPPORT */
#endif
	}
	else
#endif /* BCN_OFFLOAD_SUPPORT */
	{
#ifdef BCN_V2_SUPPORT	/* add bcn v2 support , 1.5k beacon support */
#ifndef RT_CFG80211_SUPPORT
	RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
	 pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon,
	 PKT_V2_BCN);
#else
	pAd->archOps.archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, PKT_BCN);
#endif


#else
#ifdef RT_CFG80211_SUPPORT
	pAd->archOps.archUpdateBeacon(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, PKT_BCN);
#else
	RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
		pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon,
		PKT_BCN);
#endif /* RT_CFG80211_SUPPORT */
#endif
	}
}

BOOLEAN CFG80211DRV_OpsBeaconSet(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;

#ifdef DISABLE_HOSTAPD_BEACON
    BSS_STRUCT *pMbss;
    struct wifi_dev *wdev;
    UINT16 Frame_Len = 0;
#endif
	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
#ifdef DISABLE_HOSTAPD_BEACON
	pMbss = &pAd->ApCfg.MBSSID[pBeacon->apidx];
	wdev = &pMbss->wdev;
#endif
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, pBeacon->apidx, pBeacon);
#ifdef DISABLE_HOSTAPD_BEACON
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("############MakeBeacon for apidx %d OpsBeaconSet \n",
		pBeacon->apidx));
	Frame_Len = MakeBeacon(pAd, wdev, FALSE);
	AsicUpdateBeacon(pAd, wdev, Frame_Len, PKT_BCN);
#else
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
		pBeacon->beacon_tail, pBeacon->beacon_tail_len,
		TRUE, pBeacon->apidx);
#endif
	return TRUE;
}

extern struct wifi_dev_ops ap_wdev_ops;

BOOLEAN CFG80211DRV_OpsBeaconAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	/* BOOLEAN Cancelled; */
	UINT i = 0;
	INT32 Ret = 0;
	EDCA_PARM *pEdca, *pBssEdca = NULL;
	STA_REC_CTRL_T *sta_rec;

#ifdef DISABLE_HOSTAPD_BEACON
	UINT16 FrameLen = 0;
#endif
	CHAR tr_tb_idx;
	PNET_DEV pNetDev;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon =  (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	UCHAR ext_cha;
	UCHAR ht_bw;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	UINT apidx = pBeacon->apidx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	struct wifi_dev *wdev = &pMbss->wdev;
	BCN_BUF_STRUC *bcn_buf = &wdev->bcn_buf;
	tr_tb_idx = MAX_LEN_OF_MAC_TABLE + apidx;
	pNetDev = pBeacon->pNetDev;
#ifdef RT_CFG80211_SUPPORT
		wdev->Hostapd = Hostapd_CFG;

#endif
	CFG80211DBG(DBG_LVL_OFF, ("80211> %s ==>\n", __func__));
#ifdef UAPSD_SUPPORT
	wdev->UapsdInfo.bAPSDCapable = TRUE;
	pMbss->CapabilityInfo |= 0x0800;
#endif /* UAPSD_SUPPORT */
#ifndef DISABLE_HOSTAPD_BEACON
    pAd->cfg80211_ctrl.beaconIsSetFromHostapd = TRUE; /* set here to prevent MakeBeacon do further modifications about BCN */
#endif
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, apidx, pBeacon);
#define MCAST_WCID_TO_REMOVE 0
	MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);
	APSecInit(pAd, wdev);
	sta_rec = &pAd->MacTab.tr_entry[tr_tb_idx].StaRec;
	APKeyTableInit(pAd, wdev, sta_rec);
	AsicSetRxFilter(pAd);
	/* Start from 0 & MT_MAC using HW_BSSID 1, TODO */
	pAd->MacTab.MsduLifeTime = 20; /* pEntry's UAPSD Q Idle Threshold */
	/* CFG_TODO */
	bcn_buf->BcnBufIdx = 0;

	for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
		bcn_buf->TimBitmaps[i] = 0;

	bcn_buf->bBcnSntReq = TRUE;
	/* For GO Timeout */
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	pAd->ApCfg.StaIdleTimeout = 300;
	pMbss->StationKeepAliveTime = 60;
#else
	pAd->ApCfg.StaIdleTimeout = 300;
	pMbss->StationKeepAliveTime = 0;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	AsicDisableSync(pAd, HW_BSSID_0);

#ifndef DISABLE_HOSTAPD_BEACON
	if (wdev->channel > 14)
		wdev->PhyMode = (WMODE_A | WMODE_AN);
	else
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN);
#endif /* DISABLE_HOSTAPD_BEACON */

	{
#if defined(MT7615) || defined(MT7622)


		if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
			;  /* don't reinit wdev, for tr_tbl was acquired in previous flow */
		} else
#endif
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, pAd->net_dev, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (VOID *)pAd);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): register wdev fail\n", __func__));
		}

	wdev_attr_update(pAd, wdev);
	COPY_MAC_ADDR(wdev->bssid, pNetDev->dev_addr);
	COPY_MAC_ADDR(wdev->if_addr, pNetDev->dev_addr);
	os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
	}

	/* cfg_todo */
	wdev->bWmmCapable = TRUE;
	os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
	/* BC/MC Handling */
#if defined(MT7615) || defined(MT7622)

	if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
		if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher))
			CFG80211DRV_ApKeyAdd(pAdOrg, &pAd->cfg80211_ctrl.WepKeyInfoBackup);
	} else
#endif
		TRTableInsertMcastEntry(pAd, tr_tb_idx, wdev);

	MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
	WDEV_BSS_STATE(wdev) = BSS_ACTIVE;
	wdev->bss_info_argument.CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
	wdev->bss_info_argument.u4BssInfoFeature = (BSS_INFO_OWN_MAC_FEATURE |
			BSS_INFO_BASIC_FEATURE |
			BSS_INFO_RF_CH_FEATURE |
			BSS_INFO_SYNC_MODE_FEATURE);
	/* AsicBssInfoUpdate(pAd, wdev->bss_info_argument); */
	os_msec_delay(200);
	HW_UPDATE_BSSINFO(pAd, &wdev->bss_info_argument);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("New AP BSSID %02x:%02x:%02x:%02x:%02x:%02x (%d)\n",
			 PRINT_MAC(wdev->bssid), wdev->PhyMode));
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) && (pAd->Antenna.field.TxPath == 2))
		bbp_set_txdac(pAd, 2);
	else
#endif /* DOT11_N_SUPPORT */
		bbp_set_txdac(pAd, 0);

	/* Receiver Antenna selection */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED)) {
		if (WMODE_CAP_N(wdev->PhyMode) || wdev->bWmmCapable) {
			pEdca = &pAd->CommonCfg.APEdcaParm[wdev->EdcaIdx];

			/* EDCA parameters used for AP's own transmission */
			if (pEdca->bValid == FALSE)
				set_default_ap_edca_param(pEdca);

			pBssEdca = wlan_config_get_ht_edca(wdev);
			if (pBssEdca) {
				/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
				if (pBssEdca->bValid == FALSE)
					set_default_sta_edca_param(pBssEdca);
			}

			HcAcquiredEdca(pAd, wdev, pEdca);
			HcSetEdca(wdev);
		} else
			HcReleaseEdca(pAd, wdev);
	}

#ifdef DOT11_N_SUPPORT
	if (pAd->CommonCfg.bRdg)
		AsicSetRDG(pAd, WCID_ALL, 0, 0, 0);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset WCID Table\n", __func__));
#if defined(MT7615) || defined(MT7622)
    if (IS_MT7615(pAd) || IS_MT7622(pAd))  {
		;  /* don't reset WCID table , for 7615 has set in previous flow */
	} else
#endif
	HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);
	pAd->MacTab.Content[0].Addr[0] = 0x01;

	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
#ifdef DOT11_N_SUPPORT
	SetCommonHtVht(pAd, wdev);
#endif /* DOT11_N_SUPPORT */
	wlan_operate_set_prim_ch(wdev, wdev->channel);
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble); */
	/* MlmeUpdateTxRates(pAd, FALSE, MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO + apidx); */
	MlmeUpdateTxRates(pAd, FALSE, apidx);
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, MIN_NET_DEVICE_FOR_MBSSID);

#endif /* DOT11_N_SUPPORT */

	/* Disable Protection first. */
	if (1)/* !INFRA_ON(pAd)) */
		AsicUpdateProtect(pAd);

	ApUpdateCapabilityAndErpIe(pAd, pMbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */

#ifdef DISABLE_HOSTAPD_BEACON
	FrameLen =  MakeBeacon(pAd, wdev, FALSE);
	AsicUpdateBeacon(pAd, wdev, FrameLen, PKT_BCN);
#else
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
	pBeacon->beacon_tail, pBeacon->beacon_tail_len, TRUE, pBeacon->apidx);
#endif /*DISABLE_HOSTAPD_BEACON */


#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	if (INFRA_ON(pAd)) {
		ULONG BPtoJiffies;
		LONG timeDiff;
		INT starttime = pAd->Mlme.channel_1st_staytime;

		NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
		timeDiff = (pAd->Mlme.BeaconNow32 - pAd->StaCfg[0].LastBeaconRxTime) % (pAd->CommonCfg.BeaconPeriod);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("#####pAd->Mlme.Now32 %d pAd->StaCfg[0].LastBeaconRxTime %d\n", pAd->Mlme.BeaconNow32, pAd->StaCfg[0].LastBeaconRxTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("####    timeDiff %d\n", timeDiff));

		if (starttime > timeDiff)
			OS_WAIT((starttime - timeDiff));
		else
			OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod - timeDiff)));
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* Enable BSS Sync*/
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	if (INFRA_ON(pAd)) {
		ULONG BPtoJiffies;
		LONG timeDiff;
		INT starttime = pAd->Mlme.channel_1st_staytime;

		NdisGetSystemUpTime(&pAd->Mlme.BeaconNow32);
		timeDiff = (pAd->Mlme.BeaconNow32 - pAd->StaCfg[0].LastBeaconRxTime) % (pAd->CommonCfg.BeaconPeriod);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("#####pAd->Mlme.Now32 %d pAd->StaCfg[0].LastBeaconRxTime %d\n", pAd->Mlme.BeaconNow32, pAd->StaCfg[0].LastBeaconRxTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("####    timeDiff %d\n", timeDiff));

		if (starttime > timeDiff)
			OS_WAIT((starttime - timeDiff));
		else
			OS_WAIT((starttime + (pAd->CommonCfg.BeaconPeriod - timeDiff)));
	}

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	/* Enable AP BSS Sync */
	/* AsicEnableApBssSync(pAd, pAd->CommonCfg.BeaconPeriod); */
	/* AsicEnableBcnSntReq(pAd); */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
#if defined(RT_CFG80211_SUPPORT) || defined(MT7622)
	WDEV_BSS_STATE(wdev) = BSS_READY;
#endif
	return TRUE;
}

BOOLEAN CFG80211DRV_ApKeyDel(
	VOID                                            *pAdOrg,
	VOID                                            *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	MAC_TABLE_ENTRY *pEntry;
	ASIC_SEC_INFO Info = {0};

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)

	if (pKeyInfo->bPairwise == FALSE)
#else
	if (pKeyInfo->KeyId > 0)
#endif
	{
		UINT Wcid = 0;
		UINT apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pKeyInfo->pNetDev);
		BSS_STRUCT *pMbss;
		struct wifi_dev *pWdev;

		if (apidx == WDEV_NOT_FOUND) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s failed - [ERROR]can't find wdev in driver MBSS. \n", __FUNCTION__));
			return FALSE;
		}
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		pWdev  = &pMbss->wdev;
		GET_GroupKey_WCID(pWdev, Wcid);
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_REMOVE_GROUP_KEY;
		Info.Wcid = Wcid;
		/* Set key material to Asic */
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
	} else {
		pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

		if (pEntry && (pEntry->Aid != 0)) {
			/* Set key material to Asic */
			os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
			Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
			Info.Wcid = pEntry->wcid;
			/* Set key material to Asic */
			HW_ADDREMOVE_KEYTABLE(pAd, &Info);
		}
	}

	return TRUE;
}

VOID CFG80211DRV_RtsThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev *wdev,
	UINT                                            threshold)
{
	UINT32 len_thld = MAX_RTS_THRESHOLD;

	if ((threshold > 0) && (threshold <= MAX_RTS_THRESHOLD))
		len_thld = (UINT32)threshold;
	wlan_operate_set_rts_len_thld(wdev, len_thld);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s =====>threshold %d\n", __func__, len_thld));
}


VOID CFG80211DRV_FragThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev *wdev,
	UINT                                            threshold)
{
	if (threshold > MAX_FRAG_THRESHOLD || threshold < MIN_FRAG_THRESHOLD)
		threshold =  MAX_FRAG_THRESHOLD;
	else if (threshold % 2 == 1)
		threshold -= 1;
	wlan_operate_set_frag_thld(wdev, threshold);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s =====>operate: frag_thld=%d\n", __func__, threshold));
}

BOOLEAN CFG80211DRV_ApKeyAdd(
	VOID                                            *pAdOrg,
	VOID                                            *pData)
{
#ifdef CONFIG_AP_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UINT apidx;
	BSS_STRUCT *pMbss;
	struct wifi_dev *pWdev;

	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
	/* UINT Wcid = 0; */
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pKeyInfo->pNetDev);
	pMbss = &pAd->ApCfg.MBSSID[apidx];
	pWdev = &pMbss->wdev;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s =====>\n", __func__));
	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104) {
		SET_CIPHER_WEP(pWdev->SecConfig.PairwiseCipher);
		SET_CIPHER_WEP(pWdev->SecConfig.GroupCipher);
		{
			CIPHER_KEY	*pSharedKey;
			POS_COOKIE pObj;

			pObj = (POS_COOKIE) pAd->OS_Cookie;
			pSharedKey = &pAd->SharedKey[apidx][pKeyInfo->KeyId];
			pSharedKey->KeyLen = pKeyInfo->KeyLen;
			os_move_mem(pSharedKey->Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);

			if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40)
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP64;
			else
				pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_WEP128;

			AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId, pSharedKey);
#if defined(MT7615) || defined(MT7622)

			if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
				if (pKeyInfo->bPairwise == FALSE) {
					ASIC_SEC_INFO Info = {0};
					UINT Wcid = 0;

					NdisCopyMemory(&pAd->cfg80211_ctrl.WepKeyInfoBackup, pKeyInfo, sizeof(CMD_RTPRIV_IOCTL_80211_KEY));
					pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen = pKeyInfo->KeyLen;
					os_move_mem(pWdev->SecConfig.WepKey[pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					pWdev->SecConfig.GroupKeyId = pKeyInfo->KeyId;
					os_move_mem(pWdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					/* Get a specific WCID to record this MBSS key attribute */
					GET_GroupKey_WCID(pWdev, Wcid);
					/* Set key material to Asic */
					os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
					Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
					Info.Direction = SEC_ASIC_KEY_TX;
					Info.Wcid = Wcid;
					Info.BssIndex = apidx;
					Info.Cipher = pWdev->SecConfig.GroupCipher;
					Info.KeyIdx = pKeyInfo->KeyId;
					os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
					/* Install Shared key */
					os_move_mem(Info.Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
					Info.Key.KeyLen = pKeyInfo->KeyLen;
					HW_ADDREMOVE_KEYTABLE(pAd, &Info);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							 ("%s, %u B/MC KEY pKeyInfo->KeyId %d pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen %d\n"
							  , __func__, __LINE__, pKeyInfo->KeyId, pWdev->SecConfig.WepKey[pKeyInfo->KeyId].KeyLen));
				} else {
					if (pKeyInfo->MAC)
						pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

					if (pEntry) {
						ASIC_SEC_INFO Info = {0};

						pEntry->SecConfig.PairwiseKeyId = pKeyInfo->KeyId;
						SET_CIPHER_WEP(pEntry->SecConfig.PairwiseCipher);
						/* Set key material to Asic */
						os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
						Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
						Info.Direction = SEC_ASIC_KEY_BOTH;
						Info.Wcid = pEntry->wcid;
						Info.BssIndex = pEntry->func_tb_idx;
						Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						Info.Cipher = pEntry->SecConfig.PairwiseCipher;
						Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
						os_move_mem(Info.Key.Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
						os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
						Info.Key.KeyLen = pKeyInfo->KeyLen;
						HW_ADDREMOVE_KEYTABLE(pAd, &Info);
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								 ("%s, %u UNICAST Info.Key.KeyLen %d pKeyInfo->KeyId %d Info.Key.KeyLen %d\n"
								  , __func__, __LINE__, Info.Key.KeyLen, pKeyInfo->KeyId, Info.Key.KeyLen));
					}
				}
			}

#endif
		}
	} else if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WPA) {
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
		if (pKeyInfo->bPairwise == FALSE)
#else
		if (pKeyInfo->KeyId > 0)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
		{
			USHORT Wcid;

			/* Get a specific WCID to record this MBSS key attribute */
			GET_GroupKey_WCID(pWdev, Wcid);
			pAd->SharedKey[apidx][pKeyInfo->KeyId].KeyLen = LEN_TK;

			switch (pKeyInfo->cipher)
			{
				case Ndis802_11GCMP256Enable:
					if (!IS_CIPHER_GCMP256(pWdev->SecConfig.GroupCipher)) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher));
						return FALSE;
					}
					SET_CIPHER_GCMP256(pWdev->SecConfig.GroupCipher);
					break;

				case Ndis802_11AESEnable:
					if (!IS_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher)) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher));
						return FALSE;
					}
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_AES;
					SET_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher);
					break;

				case Ndis802_11TKIPEnable:
					if (!IS_CIPHER_TKIP(pWdev->SecConfig.GroupCipher)) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher));
						return FALSE;
					}
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_TKIP;
					SET_CIPHER_TKIP(pWdev->SecConfig.GroupCipher);
					break;

				case Ndis802_11GCMP128Enable:
					if (!IS_CIPHER_GCMP128(pWdev->SecConfig.GroupCipher)) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher));
						return FALSE;
					}
					SET_CIPHER_GCMP128(pWdev->SecConfig.GroupCipher);

					break;

				case Ndis802_11CCMP256Enable:
					if (!IS_CIPHER_CCMP256(pWdev->SecConfig.GroupCipher)) {
						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("CFG: Wrong Group Cipher %d\n", pWdev->SecConfig.GroupCipher));
						return FALSE;
					}
					SET_CIPHER_CCMP256(pWdev->SecConfig.GroupCipher);

					break;
			}
#if defined(MT7615) || defined(MT7622)
			if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
				ASIC_SEC_INFO Info = {0};
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_TX;
				Info.Wcid = Wcid;
				Info.BssIndex = apidx;
				Info.Cipher = pWdev->SecConfig.GroupCipher;
				Info.KeyIdx = pKeyInfo->KeyId;
				pWdev->SecConfig.GroupKeyId = Info.KeyIdx;
				os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				/* Install Shared key */
				os_move_mem(pWdev->SecConfig.GTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				os_move_mem(Info.Key.Key, pWdev->SecConfig.GTK, LEN_MAX_GTK);
				WPAInstallKey(pAd, &Info, TRUE, TRUE);
				pWdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;
			}
#else
			os_move_mem(pAd->SharedKey[apidx][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId,
						&pAd->SharedKey[apidx][pKeyInfo->KeyId]);
			GET_GroupKey_WCID(pWdev, Wcid);
#endif
			} else {
				if (pKeyInfo->MAC)
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
					switch (pKeyInfo->cipher) {
					case Ndis802_11GCMP256Enable:
						SET_CIPHER_GCMP256(pEntry->SecConfig.PairwiseCipher);
						break;

					case Ndis802_11AESEnable:
						SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
						break;

					case Ndis802_11TKIPEnable:
						SET_CIPHER_TKIP(pEntry->SecConfig.PairwiseCipher);
						break;

					case Ndis802_11GCMP128Enable:
						SET_CIPHER_GCMP128(pEntry->SecConfig.PairwiseCipher);
						break;

					case Ndis802_11CCMP256Enable:
						SET_CIPHER_CCMP256(pEntry->SecConfig.PairwiseCipher);
						break;

					}
					NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK],
								pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
#if defined(MT7615) || defined(MT7622)
					if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;

						/* Set key material to Asic */
						os_alloc_mem(NULL, (UCHAR **)&info, sizeof(ASIC_SEC_INFO));
						if (info) {
							os_zero_mem(info, sizeof(ASIC_SEC_INFO));
							NdisCopyMemory(&pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], pKeyInfo->KeyBuf, LEN_MAX_PTK);
							info->Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
							info->Direction = SEC_ASIC_KEY_BOTH;
							info->Wcid = pEntry->wcid;
							info->BssIndex = pEntry->func_tb_idx;
							info->Cipher = pEntry->SecConfig.PairwiseCipher;
							info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);
							os_move_mem(&info->PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
							os_move_mem(info->Key.Key, &pEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK], (LEN_TK + LEN_TK2));
							WPAInstallKey(pAd, info, TRUE, TRUE);
							os_free_mem(info);
						} else {
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
									 DBG_LVL_ERROR, ("%s: struct alloc fail\n",
										__func__));
						}
					}
#else
					pEntry->PairwiseKey.KeyLen = LEN_TK;
					NdisCopyMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					os_move_mem(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);
					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);

#endif
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
					if (pKeyInfo->cipher == Ndis802_11AESEnable)
					{
						UCHAR op_ht_bw1 = wlan_operate_get_ht_bw(pWdev);
						UCHAR op_ht_bw2 = wlan_operate_get_ht_bw(&pAd->StaCfg[0].wdev);

						MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: InfraCh=%d, pWdev->channel=%d\n", __func__, pAd->MlmeAux.InfraChannel, pWdev->channel));

						if (INFRA_ON(pAd) &&
							(((op_ht_bw2 == op_ht_bw1) && (pAd->StaCfg[0].wdev.channel != pWdev->channel))
							 || !((op_ht_bw2 == op_ht_bw1) && ((pAd->StaCfg[0].wdev.channel == pWdev->channel))))) {
							/*wait 1 s  DHCP  for P2P CLI */
							OS_WAIT(1000);
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("OS WAIT 1000 FOR DHCP\n"));
							/* pAd->MCC_GOConnect_Protect = FALSE; */
							/* pAd->MCC_GOConnect_Count = 0; */
							Start_MCC(pAd);
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("infra => GO test\n"));
						} else if ((op_ht_bw2 != op_ht_bw1) && ((pAd->StaCfg[0].wdev.channel == pWdev->channel))) {
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("start bw !=  && SCC\n"));
							pAd->Mlme.bStartScc = TRUE;
						}

						/*after p2p cli connect , neet to change to default configure*/
						if (op_ht_bw1 == HT_BW_20) {
							wlan_operate_set_ht_bw(pWdev, HT_BW_40, EXTCHA_BELOW);
							pAd->CommonCfg.HT_Disable = 0;
							SetCommonHtVht(pAd, pWdev);
						}
					}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

			} else
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("CFG: Security Set. (PAIRWISE) But pEntry NULL\n"));
		}
	}
#ifdef DOT11W_PMF_SUPPORT
	else if(pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC)
	{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		if ((pKeyInfo->bPairwise == FALSE ) && (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5))
#else
		if (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
		{
			hex_dump("PMF IGTK pKeyInfo->KeyBuf=", (UINT8 *)pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("PMF IGTK pKeyInfo->KeyId=%d\n", pKeyInfo->KeyId));

#ifdef MT7615
			if (IS_MT7615(pAd))
			{
				PPMF_CFG pPmfCfg = &pWdev->SecConfig.PmfCfg;
				ASIC_SEC_INFO Info = {0};
				USHORT Wcid;
				if (pKeyInfo->cipher == Ndis802_11AESEnable)
					SET_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher);
				else if (pKeyInfo->cipher == Ndis802_11TKIPEnable)
					SET_CIPHER_TKIP(pWdev->SecConfig.GroupCipher);
				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(pWdev, Wcid);

				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_TX;
				Info.Wcid = Wcid;
				Info.BssIndex = apidx;
				Info.Cipher = pWdev->SecConfig.GroupCipher;
				Info.KeyIdx = pWdev->SecConfig.GroupKeyId;
				Info.IGTKKeyLen = LEN_TK;
				os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_zero_mem(&pPmfCfg->IPN[pKeyInfo->KeyId - 4][0], LEN_WPA_TSC);
				os_move_mem(&pPmfCfg->IGTK[pKeyInfo->KeyId - 4][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				os_move_mem(Info.Key.Key,pWdev->SecConfig.GTK,LEN_MAX_GTK);
				os_move_mem(Info.IGTK, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
				WPAInstallKey(pAd, &Info, TRUE, TRUE);
				pWdev->SecConfig.Handshake.GTKState = REKEY_ESTABLISHED;

			}

#else
			os_move_mem(pAd->SharedKey[apidx][pKeyInfo->KeyId].Key, pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
			AsicAddSharedKeyEntry(pAd, apidx, pKeyInfo->KeyId,
				&pAd->SharedKey[apidx][pKeyInfo->KeyId]);

			GET_GroupKey_WCID(pWdev, Wcid);
			RTMPSetWcidSecurityInfo(pAd, apidx, (UINT8)(pKeyInfo->KeyId),
			pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg, Wcid, SHAREDKEYTABLE);

#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT)
				CmdProcAddRemoveKey(pAd, 0, apidx, pKeyInfo->KeyId, Wcid, SHAREDKEYTABLE,
									&pAd->SharedKey[apidx][pKeyInfo->KeyId], BROADCAST_ADDR);
#endif /* MT_MAC */
#endif	 /* MT7615 */
		}
		else
		{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ERROR !! pKeyInfo->bPairwise = %d pKeyInfo->KeyId=%d \n", pKeyInfo->bPairwise,pKeyInfo->KeyId));
#else
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("ERROR !! pKeyInfo->KeyId=%d \n", pKeyInfo->KeyId));
#endif /* LINUX_VERSION_CODE 2.6.37 */
		}

	}

#endif /* DOT11W_PMF_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT CFG80211_StaPortSecured(
	IN VOID                                         *pAdCB,
	IN UCHAR					*pMac,
	IN UINT						flag)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;

	pEntry = MacTableLookup(pAd, pMac);

	if (!pEntry)
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can't find pEntry in CFG80211_StaPortSecured\n"));
	else {
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

		if (flag) {
			/* Update status and set Port as Secured */
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#if defined(MT7615) || defined(MT7622)

			if (IS_MT7615(pAd) || IS_MT7622(pAd)) {
				pEntry->SecConfig.Handshake.WpaState = AS_PTKINITDONE;
				WifiSysUpdatePortSecur(pAd, pEntry, NULL);
			}

#else
			pEntry->WpaState = AS_PTKINITDONE;
#endif
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AID:%d, PortSecured\n", pEntry->Aid));
		} else {
			pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AID:%d, PortNotSecured\n", pEntry->Aid));
		}
	}

	return 0;
}


#ifdef HOSTAPD_MAP_SUPPORT
INT CFG80211_ApStaDel(
	IN VOID                                         *pAdCB,
	IN VOID                                         *pData,
	IN UINT						reason)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;
	CMD_RTPRIV_IOCTL_AP_STA_DEL *pApStaDelInfo = NULL;
	PUCHAR pMac = NULL;

	pApStaDelInfo = (CMD_RTPRIV_IOCTL_AP_STA_DEL *)pData;

	if (pApStaDelInfo->pSta_MAC != NULL)
		pMac = (PUCHAR)pApStaDelInfo->pSta_MAC;

	if (pMac == NULL) {
		{
			if (pApStaDelInfo->pWdev != NULL)
				MacTableResetWdev(pAd, pApStaDelInfo->pWdev);
			else
				MacTableReset(pAd);
			NdisZeroMemory(pAd->radius_tbl, MAX_LEN_OF_MAC_TABLE * sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	} else {
		int i;

		pEntry = MacTableLookup(pAd, pMac);
		if (pEntry) {
			USHORT reason_code = REASON_NO_LONGER_VALID;
#ifdef HOSTAPD_OWE_SUPPORT
			if (pEntry->wdev && (pEntry->Sst == SST_AUTH)
				&& IS_AKM_OWE(pEntry->wdev->SecConfig.AKMMap))
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\n OWE mode Ignore Deauth from Hostapd"));
			else

#endif
{
			if (reason)
				reason_code = reason;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
				DBG_LVL_OFF,
				("%s, Deauth : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x reason code %d\n", __func__,
				 pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5], reason_code));

				MlmeDeAuthAction(pAd, pEntry, reason_code, FALSE);
}
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Can't find pEntry in ApStaDel\n"));

		/* Find entry in radius tbl and delete it if found */

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pMac))
				NdisZeroMemory(pRadiusEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	}
	return 0;
}

#else
INT CFG80211_ApStaDel(
	IN VOID                                         *pAdCB,
	IN UCHAR                                        *pMac,
	IN UINT						reason)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;

	if (pMac == NULL) {
		{
			MacTableReset(pAd);
			NdisZeroMemory(pAd->radius_tbl, MAX_LEN_OF_MAC_TABLE * sizeof(RADIUS_ACCOUNT_ENTRY));
		}

	} else {
		int i;

		pEntry = MacTableLookup(pAd, pMac);
		if (pEntry) {
			USHORT reason_code = REASON_NO_LONGER_VALID;
#ifdef HOSTAPD_OWE_SUPPORT
			if (pEntry->wdev && (pEntry->Sst == SST_AUTH)
				&& IS_AKM_OWE(pEntry->wdev->SecConfig.AKMMap))
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("\n OWE mode Ignore Deauth from Hostapd"));
			else

#endif
{
			if (reason)
				reason_code = reason;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL,
				DBG_LVL_ERROR,
				("%s, Deauth : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x reason code %d\n", __func__,
				 pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5], reason_code));

				MlmeDeAuthAction(pAd, pEntry, reason_code, FALSE);
}
		} else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Can't find pEntry in ApStaDel\n"));

		/* Find entry in radius tbl and delete it if found */

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pMac))
				NdisZeroMemory(pRadiusEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
		}
	}
	return 0;
}
#endif


INT CFG80211_setApDefaultKey(
	IN VOID                    *pAdCB,
	IN struct net_device		*pNetdev,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	INT32 apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetdev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s failed - [ERROR]can't find wdev in driver MBSS. \n",
			__func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set Ap Default Key: %d\n", Data));
#if defined(MT7615) || defined(MT7622)

	pAd->ApCfg.MBSSID[apidx].wdev.SecConfig.PairwiseKeyId = Data;
#else
	pAd->ApCfg.MBSSID[apidx].wdev.DefaultKeyId = Data;
#endif

	return 0;
}

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setApDefaultMgmtKey(
	IN VOID                    *pAdCB,
	IN struct net_device		*pNetdev,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	INT32 apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, pNetdev);
	struct wifi_dev *pWdev = NULL;

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s failed - [ERROR]can't find wdev in driver MBSS. \n", __FUNCTION__));
		return FALSE;
	}
	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;

#ifdef MT7615
	if((Data == 4)||(Data == 5))
	{
		pWdev->SecConfig.PmfCfg.IGTK_KeyIdx = (UINT8)Data;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set Ap Default Mgmt KeyId: %d\n", Data));
	}
	else
	{
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s fail - [ERROR]Invalid Mgmt KeyId: %d\n", __FUNCTION__,Data));
	}
#endif

	return 0;
}
#endif /*DOT11W_PMF_SUPPORT*/


INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr, IN PNET_DEV pNetDevIn)
{
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SINGLE_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n"));
		CFG80211OS_DelSta(pNetDevIn, mac_addr);
	}

	return 0;
}

#ifdef APCLI_CFG80211_SUPPORT
VOID CFG80211_ApClientConnectResultInform(
	VOID *pAdCB, UCHAR *pBSSID, UCHAR *pReqIe, UINT32 ReqIeLen,
	UCHAR *pRspIe, UINT32 RspIeLen, UCHAR FlgIsSuccess)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	CFG80211OS_P2pClientConnectResultInform(pAd->ApCfg.ApCliTab[MAIN_MBSSID].wdev.if_dev, pBSSID,
					pReqIe, ReqIeLen, pRspIe, RspIeLen, FlgIsSuccess);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
}
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
