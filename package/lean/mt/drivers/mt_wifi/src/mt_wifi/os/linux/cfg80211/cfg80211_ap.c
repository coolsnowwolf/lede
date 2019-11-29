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
#if (KERNEL_VERSION(3, 5, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_OPERATION;
#else
	const UCHAR CFG_HT_OP_EID = WLAN_EID_HT_INFORMATION;
#endif /* LINUX_VERSION_CODE: 3.5.0 */
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
	const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	supp_rates_ie = cfg80211_find_ie(WLAN_EID_SUPP_RATES, pBeacon->beacon_head + 36, pBeacon->beacon_head_len - 36);
	/* if it doesn't find WPA_IE in tail first 30 bytes. treat it as is not found */
	wpa_ie = cfg80211_find_ie(CFG_WPA_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	rsn_ie = cfg80211_find_ie(WLAN_EID_RSN, pBeacon->beacon_tail, pBeacon->beacon_tail_len);/* wpa2 case. */
	ext_supp_rates_ie = cfg80211_find_ie(WLAN_EID_EXT_SUPP_RATES, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_cap = cfg80211_find_ie(WLAN_EID_HT_CAPABILITY, pBeacon->beacon_tail, pBeacon->beacon_tail_len);
	ht_info = cfg80211_find_ie(CFG_HT_OP_EID, pBeacon->beacon_tail, pBeacon->beacon_tail_len);

	/* SSID */

	if (ssid_ie == NULL) {
		os_move_mem(pMbss->Ssid, "CFG_Linux_GO", 12);
		pMbss->SsidLen = 12;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: SSID Not Found In Packet\n"));
	} else if (pBeacon->ssid_len != 0) {
		os_zero_mem(pMbss->Ssid, pMbss->SsidLen);
		NdisZeroMemory(pMbss->Ssid, pMbss->SsidLen);
		pMbss->SsidLen = pBeacon->ssid_len;
		NdisCopyMemory(pMbss->Ssid, ssid_ie + 2, pMbss->SsidLen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nCFG : SSID: %s, %d\n", pMbss->Ssid, pMbss->SsidLen));
	}

#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)

	if (pBeacon->hidden_ssid > 0 && pBeacon->hidden_ssid < 3) {
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

	pMbss->CapabilityInfo =	CAP_GENERATE(1, 0, (!IS_CIPHER_NONE(wdev->SecConfig.PairwiseCipher)),
										 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1), pAd->CommonCfg.bUseShortSlotTime, /*SpectrumMgmt*/FALSE);
	/* Disable Driver-Internal Rekey */
	pMbss->WPAREKEY.ReKeyInterval = 0;
	pMbss->WPAREKEY.ReKeyMethod = DISABLE_REKEY;

	if (pBeacon->interval != 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG_TIM New BI %d\n", pBeacon->interval));
		pAd->CommonCfg.BeaconPeriod = pBeacon->interval;
	}

	if (pBeacon->dtim_period != 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG_TIM New DP %d\n", pBeacon->dtim_period));
		pAd->ApCfg.DtimPeriod = pBeacon->dtim_period;
	}

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
	BOOLEAN											isAllUpdate)
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
	UINT apidx = MAIN_MBSSID;
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

	if (cap->fgBcnOffloadSupport == TRUE)
		RT28xx_UpdateBcnAndTimToMcu(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len, pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_BCN);
	else
#endif /* BCN_OFFLOAD_SUPPORT */
		RT28xx_UpdateBeaconToAsic(pAd, &pAd->ApCfg.MBSSID[apidx].wdev, beacon_len,
								  pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimIELocationInBeacon, PKT_BCN);
}

BOOLEAN CFG80211DRV_OpsBeaconSet(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;
	UINT apidx = MAIN_MBSSID;
	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, apidx, pBeacon);
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
						  pBeacon->beacon_tail, pBeacon->beacon_tail_len,
						  TRUE);
	return TRUE;
}

extern struct wifi_dev_ops ap_wdev_ops;

BOOLEAN CFG80211DRV_OpsBeaconAdd(VOID *pAdOrg, VOID *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BEACON *pBeacon;
	/* BOOLEAN Cancelled; */
	UINT i = 0;
	INT32 Ret = 0;
	EDCA_PARM *pEdca, *pBssEdca = NULL;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
	UCHAR ext_cha;
	UCHAR ht_bw;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
	UINT apidx = MAIN_MBSSID;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	struct wifi_dev *wdev = &pMbss->wdev;
	BCN_BUF_STRUC *bcn_buf = &wdev->bcn_buf;
	CHAR tr_tb_idx = MAX_LEN_OF_MAC_TABLE + apidx;
#ifdef RT_CFG80211_SUPPORT
		wdev->Hostapd = Hostapd_CFG;

#endif
	CFG80211DBG(DBG_LVL_OFF, ("80211> %s ==>\n", __func__));
	pBeacon = (CMD_RTPRIV_IOCTL_80211_BEACON *)pData;
#ifdef UAPSD_SUPPORT
	wdev->UapsdInfo.bAPSDCapable = TRUE;
	pMbss->CapabilityInfo |= 0x0800;
#endif /* UAPSD_SUPPORT */
	pAd->cfg80211_ctrl.beaconIsSetFromHostapd = TRUE; /* set here to prevent MakeBeacon do further modifications about BCN */
	CFG80211DRV_UpdateApSettingFromBeacon(pAd, apidx, pBeacon);
#define MCAST_WCID_TO_REMOVE 0
	MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);
	APSecInit(pAd, wdev);
	APKeyTableInit(pAd, wdev);
	AsicSetRxFilter(pAd);
	/* Start from 0 & MT_MAC using HW_BSSID 1, TODO */
	pAd->ApCfg.BssidNum = (MAIN_MBSSID + 1);
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

	if (wdev->channel > 14)
		wdev->PhyMode = (WMODE_A | WMODE_AN);
	else
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN);

	{
#ifdef MT7615

		if (IS_MT7615(pAd)) {
			;  /* don't reinit wdev, for tr_tbl was acquired in previous flow */
		} else
#endif
			Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, pAd->net_dev, apidx, (VOID *)&pAd->ApCfg.MBSSID[apidx], (VOID *)pAd);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): register wdev fail\n", __func__));
			RtmpOSNetDevFree(pAd->net_dev);
			return FALSE;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
			RtmpOSNetDevFree(pAd->net_dev);
			return FALSE;
		}

		wdev_attr_update(pAd, wdev);
		COPY_MAC_ADDR(wdev->bssid, pAd->CurrentAddress);
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
		os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
		/* assoc to MBSSID's wdev */
		RTMP_OS_NETDEV_SET_WDEV(pAd->net_dev, wdev);
		RTMP_OS_NETDEV_SET_PRIV(pAd->net_dev, pAd);
	}

	/* cfg_todo */
	wdev->bWmmCapable = TRUE;
	os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
	/* BC/MC Handling */
#ifdef MT7615

	if (IS_MT7615(pAd)) {
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
#ifndef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		wlan_config_set_ht_bw(wdev, HT_BW_20);

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	if (pAd->CommonCfg.bRdg)
		AsicSetRDG(pAd, WCID_ALL, 0, 0, 0);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset WCID Table\n", __func__));
#ifdef MT7615

	if (IS_MT7615(pAd)) {
		;  /* don't reset WCID table , for 7615 has set in previous flow */
	} else
#endif
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
	CFG80211_UpdateBeacon(pAd, pBeacon->beacon_head, pBeacon->beacon_head_len,
						  pBeacon->beacon_tail, pBeacon->beacon_tail_len, TRUE);

	/* beacon init for USB/SDIO */
	asic_bss_beacon_init(pAd);
	/* beacon start for USB/SDIO */
	asic_bss_beacon_start(pAd);

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
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_REMOVE_GROUP_KEY;
		Info.Wcid = BSS0;
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
	/* UINT Wcid = 0; */
	UINT apidx = MAIN_MBSSID;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	struct wifi_dev *pWdev = &pMbss->wdev;

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
#ifdef MT7615

			if (IS_MT7615(pAd)) {
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
		if (pKeyInfo->cipher == Ndis802_11AESEnable) {
			/* AES */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
			if (pKeyInfo->bPairwise == FALSE)
#else
			if (pKeyInfo->KeyId > 0)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
			{
				if (IS_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher)) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: Set AES Security Set. (GROUP) %d\n", pKeyInfo->KeyLen));
					pAd->SharedKey[apidx][pKeyInfo->KeyId].KeyLen = LEN_TK;
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_AES;
#ifdef MT7615

					if (IS_MT7615(pAd)) {
						ASIC_SEC_INFO Info = {0};
						USHORT Wcid;

						SET_CIPHER_CCMP128(pWdev->SecConfig.GroupCipher);
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
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))
						CmdProcAddRemoveKey(pAd, 0, apidx, pKeyInfo->KeyId, Wcid, SHAREDKEYTABLE,
											&pAd->SharedKey[apidx][pKeyInfo->KeyId], BROADCAST_ADDR);

#endif /* MT_MAC */
#endif
				}
			} else {
				if (pKeyInfo->MAC)
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: Set AES Security Set. (PAIRWISE) %d\n", pKeyInfo->KeyLen));
#ifdef MT7615

					if (IS_MT7615(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;

						NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
						SET_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher);
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
							info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);/* pEntry->SecConfig.PairwiseKeyId; */
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
					PairwiseKey.KeyLen = LEN_TK;
					NdisCopyMemory(&pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
					os_move_mem(pEntry->PairwiseKey.Key, &pEntry->PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyLen);
					AsicAddPairwiseKeyEntry(pAd, (UCHAR)pEntry->Aid, &pEntry->PairwiseKey);
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))
						CmdProcAddRemoveKey(pAd, 0, apidx, pKeyInfo->KeyId, pEntry->wcid, PAIRWISEKEYTABLE,
											&pEntry->PairwiseKey, pEntry->Addr);

#endif /* MT_MAC */
#endif
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
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
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: Set AES Security Set. (PAIRWISE) But pEntry NULL\n"));
			}
		} else if (pKeyInfo->cipher == Ndis802_11TKIPEnable) {
			/* TKIP */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
			if (pKeyInfo->bPairwise == FALSE)
#else
			if (pKeyInfo->KeyId > 0)
#endif	/* LINUX_VERSION_CODE 2.6.37 */
			{
				if (IS_CIPHER_TKIP(pWdev->SecConfig.GroupCipher)) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG: Set TKIP Security Set. (GROUP) %d\n", pKeyInfo->KeyLen));
					pAd->SharedKey[apidx][pKeyInfo->KeyId].KeyLen = LEN_TK;
					pAd->SharedKey[apidx][pKeyInfo->KeyId].CipherAlg = CIPHER_TKIP;
#ifdef MT7615

					if (IS_MT7615(pAd)) {
						ASIC_SEC_INFO Info = {0};
						USHORT Wcid;

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
						Info.KeyIdx = pKeyInfo->KeyId;
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
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))
						RTMP_ADDREMOVE_KEY(pAd, 0, apidx, pKeyInfo->KeyId, Wcid, SHAREDKEYTABLE,
										   &pAd->SharedKey[apidx][pKeyInfo->KeyId], BROADCAST_ADDR);

#endif /* MT_MAC */
#endif /* MT7615 */
				}
			} else {
				if (pKeyInfo->MAC)
					pEntry = MacTableLookup(pAd, pKeyInfo->MAC);

				if (pEntry) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CFG: Set TKIP Security Set. (PAIRWISE) %d\n", pKeyInfo->KeyLen));
					NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
#ifdef MT7615

					if (IS_MT7615(pAd)) {
						struct _ASIC_SEC_INFO *info = NULL;

						NdisCopyMemory(&pEntry->SecConfig.PTK[OFFSET_OF_PTK_TK], pKeyInfo->KeyBuf, OFFSET_OF_PTK_TK);
						SET_CIPHER_TKIP(pEntry->SecConfig.PairwiseCipher);
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
							info->KeyIdx = (UINT8)(pKeyInfo->KeyId & 0x0fff);/* pEntry->SecConfig.PairwiseKeyId; */
							os_move_mem(&info->PeerAddr[0],
										pEntry->Addr, MAC_ADDR_LEN);
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
#ifdef MT_MAC

					if (IS_HIF_TYPE(pAd, HIF_MT))
						RTMP_ADDREMOVE_KEY(pAd, 0, apidx, pKeyInfo->KeyId, pEntry->wcid, PAIRWISEKEYTABLE,
										   &pEntry->PairwiseKey, pEntry->Addr);

#endif /* MT_MAC */
#endif /* MT7615 */
				} else
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG: Set TKIP Security Set. (PAIRWISE) But pEntry NULL\n"));
			}
		}
	}

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
#ifdef MT7615

			if (IS_MT7615(pAd)) {
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

INT CFG80211_ApStaDel(
	IN VOID                                         *pAdCB,
	IN UCHAR                                        *pMac)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	MAC_TABLE_ENTRY *pEntry;

	if (pMac == NULL) {
			MacTableReset(pAd);
	} else {
		pEntry = MacTableLookup(pAd, pMac);

		if (pEntry)
			MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can't find pEntry in ApStaDel\n"));
	}

	return 0;
}

INT CFG80211_setApDefaultKey(
	IN VOID                    *pAdCB,
	IN UINT					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Set Ap Default Key: %d\n", Data));
#ifdef MT7615
	pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.SecConfig.PairwiseKeyId = Data;
#else
	pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.DefaultKeyId = Data;
#endif
	return 0;
}

INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr, IN PNET_DEV pNetDevIn)
{
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SINGLE_DEVICE CFG : GO NOITFY THE CLIENT Disconnected\n"));
		CFG80211OS_DelSta(pNetDevIn, mac_addr);
	}

	return 0;
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
