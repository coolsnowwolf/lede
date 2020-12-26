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
    cmm_info_element.c

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      --------------------------------------------
				2016-08-18		AP/APCLI/STA SYNC FSM Integration
*/

#include "rt_config.h"

static INT build_wsc_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_INCLUDED
	BOOLEAN bHasWscIe = FALSE;

	if (wdev->wdev_type == WDEV_TYPE_APCLI) {
#ifdef APCLI_SUPPORT
			/*
				Append WSC information in probe request if WSC state is running
			*/
			if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
				(wdev->WscControl.bWscTrigger))
				bHasWscIe = TRUE;

#if defined(WSC_V2_SUPPORT) && !defined(CONFIG_MAP_SUPPORT) && !defined(CON_WPS)
			/* need to check if !defined(CONFIG_MAP_SUPPORT) is necessary */
			else if (wdev->WscControl.WscV2Info.bEnableWpsV2)
				bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef CON_WPS
		if (bHasWscIe) {
			PWSC_CTRL pWscControl = NULL;
			/* Do not include wsc ie in case concurrent WPS is running */
			bHasWscIe = FALSE;
			pWscControl = &wdev->WscControl;

			if ((pWscControl->conWscStatus == CON_WPS_STATUS_DISABLED) ||
			    (pAd->ApCfg.ConWpsApCliMode != CON_WPS_APCLI_BAND_AUTO))
				bHasWscIe = TRUE;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("[scan_active: %d] ConWpsApCliMode=%d conWscStatus=%d bHasWscIe=%d\n",
				  __LINE__, pAd->ApCfg.ConWpsApCliMode,
				  pWscControl->conWscStatus, bHasWscIe));

		}
#endif /*CON_WPS*/
	} else if (wdev->wdev_type == WDEV_TYPE_STA) {
	}

	if (bHasWscIe) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildProbeReqIE(pAd, wdev, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:: WscBuf Allocate failed!\n", __func__));
	}

#endif /* WSC_INCLUDED */
	return len;
}

static INT build_wsc_probe_rsp_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;

	switch (wdev->wdev_type) {
#ifdef WSC_AP_SUPPORT

	case WDEV_TYPE_AP: {
		/* for windows 7 logo test */
		if ((wdev->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
			(!IS_IEEE8021X_Entry(wdev)) &&
#endif /* DOT1X_SUPPORT */
			(IS_CIPHER_WEP(wdev->SecConfig.PairwiseCipher))) {
			/*
				Non-WPS Windows XP and Vista PCs are unable to determine if a WEP enalbed network is static key based
				or 802.1X based. If the legacy station gets an EAP-Rquest/Identity from the AP, it assume the WEP
				network is 802.1X enabled & will prompt the user for 802.1X credentials. If the legacy station doesn't
				receive anything after sending an EAPOL-Start, it will assume the WEP network is static key based and
				prompt user for the WEP key. <<from "WPS and Static Key WEP Networks">>
				A WPS enabled AP should include this IE in the beacon when the AP is hosting a static WEP key network.
				The IE would be 7 bytes long with the Extended Capability field set to 0 (all bits zero)
				http://msdn.microsoft.com/library/default.asp?url=/library/en-us/randz/protocol/securing_public_wi-fi_hotspots.asp
			*/
			const UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};

			MAKE_IE_TO_BUF(buf, PROVISION_SERVICE_IE, 7, len);
		}

		/* add Simple Config Information Element */
		if ((wdev->WscControl.WscConfMode > WSC_DISABLE) && (wdev->WscIEProbeResp.ValueLen))
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#endif /* WSC_AP_SUPPORT */
#ifdef WSC_STA_SUPPORT

	case WDEV_TYPE_ADHOC: {
		/* PSTA_ADMIN_CONFIG pStaCfg = GetStaCfgByWdev(pAd,wdev); */
		/* add Simple Config Information Element */
		if (wdev->WscIEProbeResp.ValueLen != 0)
			MAKE_IE_TO_BUF(buf, wdev->WscIEProbeResp.Value, wdev->WscIEProbeResp.ValueLen, len);

		break;
	}

#endif /* WSC_STA_SUPPORT */
	}

	return len;
}

static INT build_wsc_assoc_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
#ifdef WSC_STA_SUPPORT

	/* Add WSC IE if we are connecting to WSC AP */
	if ((wdev->WscControl.WscEnAssociateIE) &&
		(wdev->WscControl.WscConfMode != WSC_DISABLE) &&
		(wdev->WscControl.bWscTrigger)) {
		UCHAR *pWscBuf = NULL, WscIeLen = 0;

		os_alloc_mem(pAd, (UCHAR **) &pWscBuf, 512);

		if (pWscBuf != NULL) {
			NdisZeroMemory(pWscBuf, 512);
			WscBuildAssocReqIE(&wdev->WscControl, pWscBuf, &WscIeLen);
			MAKE_IE_TO_BUF(buf, pWscBuf, WscIeLen, len);
			os_free_mem(pWscBuf);
		} else
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("%s:: WscBuf Allocate failed!\n",
					  __func__));
	}

#endif /* WSC_STA_SUPPORT */
	return len;
}

INT build_wsc_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_wsc_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_PROBE_RSP)
		len += build_wsc_probe_rsp_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_ASSOC_REQ)
		len += build_wsc_assoc_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

INT build_extra_probe_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	SCAN_INFO *ScanInfo = &wdev->ScanInfo;
#ifdef RT_CFG80211_SUPPORT
#ifdef APCLI_CFG80211_SUPPORT
	if ((pAd->ApCfg.ApCliTab[wdev->func_idx].wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
							  (pAd->cfg80211_ctrl.ExtraIeLen > 0)) {
		MAKE_IE_TO_BUF(buf, pAd->cfg80211_ctrl.pExtraIe,
					   pAd->cfg80211_ctrl.ExtraIeLen, len);
	}
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

	if (ScanInfo->ExtraIeLen && ScanInfo->ExtraIe) {
		MAKE_IE_TO_BUF(buf, ScanInfo->ExtraIe,
					   ScanInfo->ExtraIeLen, len);
	}

	return len;
}

INT build_extra_assoc_req_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	return len;
}

INT build_extra_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;

	if (info->frame_subtype == SUBTYPE_PROBE_REQ)
		len += build_extra_probe_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
	else if (info->frame_subtype == SUBTYPE_ASSOC_REQ)
		len += build_extra_assoc_req_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));

	return len;
}

#ifdef CONFIG_AP_SUPPORT
/* Extended Capabilities IE */
INT build_ap_extended_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	ULONG infoPos;
	PUCHAR pInfo;
	BOOLEAN bNeedAppendExtIE = FALSE;
	UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
	BSS_STRUCT *mbss;
	EXT_CAP_INFO_ELEMENT extCapInfo = { 0 };

	/* NdisZeroMemory(&extCapInfo, extInfoLen); */
	mbss = wdev->func_dev;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	/* P802.11n_D1.10, HT Information Exchange Support */
	if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
		WMODE_CAP_N(wdev->PhyMode) && (wdev->channel <= 14) &&
		(wdev->DesiredHtPhyInfo.bHtEnable))
		extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_DOT11V_WNM

	if (mbss->WNMCtrl.ProxyARPEnable)
		extCapInfo.proxy_arp = 1;

	if (mbss->WNMCtrl.WNMBTMEnable)
		extCapInfo.BssTransitionManmt = 1;

#ifdef CONFIG_HOTSPOT_R2

	if (mbss->WNMCtrl.WNMNotifyEnable)
		extCapInfo.wnm_notification = 1;

	if (mbss->HotSpotCtrl.QosMapEnable)
		extCapInfo.qosmap = 1;

#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_DOT11V_WNM */
		/* interworking ie without hotspot enabled. */
#ifdef DOT11U_INTERWORKING_IE_SUPPORT
	if (mbss->bEnableInterworkingIe == TRUE)
		extCapInfo.interworking = 1;
#endif

#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)

	if (mbss->GASCtrl.b11U_enable)
		extCapInfo.interworking = 1;

#endif
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
	pInfo = (PUCHAR)(&extCapInfo);

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

		MAKE_IE_TO_BUF(buf, &ExtCapIe,   1, len);
		MAKE_IE_TO_BUF(buf, &extInfoLen, 1, len);
		MAKE_IE_TO_BUF(buf, &extCapInfo, extInfoLen, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */

INT build_extended_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	switch (wdev->wdev_type) {
	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_extended_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}

#ifdef CONFIG_AP_SUPPORT
static INT build_ap_wmm_cap_ie(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *buf)
{
	INT len = 0;
	UCHAR i;
	UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
	UINT8 AIFSN[4];
#ifdef UAPSD_SUPPORT
	BSS_STRUCT *pMbss = wdev->func_dev;
#endif
	struct _EDCA_PARM *pBssEdca = wlan_config_get_ht_edca(wdev);

	if (pBssEdca) {
		WmeParmIe[8] = pBssEdca->EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
		UAPSD_MR_IE_FILL(WmeParmIe[8], &pMbss->wdev.UapsdInfo);
#endif /* UAPSD_SUPPORT */
		NdisMoveMemory(AIFSN, pBssEdca->Aifsn, sizeof(AIFSN));

		for (i = QID_AC_BK; i <= QID_AC_VO; i++) {
			WmeParmIe[10 + (i * 4)] = (i << 5)							  +	/* b5-6 is ACI */
									  ((UCHAR)pBssEdca->bACM[i] << 4)	  +	/* b4 is ACM */
									  (AIFSN[i] & 0x0f);						/* b0-3 is AIFSN */
			WmeParmIe[11 + (i * 4)] = (pBssEdca->Cwmax[i] << 4)		  +	/* b5-8 is CWMAX */
									  (pBssEdca->Cwmin[i] & 0x0f);				/* b0-3 is CWMIN */
			WmeParmIe[12 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] & 0xff);		/* low byte of TXOP */
			WmeParmIe[13 + (i * 4)] = (UCHAR)(pBssEdca->Txop[i] >> 8);			/* high byte of TXOP */
		}

		MAKE_IE_TO_BUF(buf, WmeParmIe, 26, len);
	}

	return len;
}
#endif /* CONFIG_AP_SUPPORT */


INT build_wmm_cap_ie(RTMP_ADAPTER *pAd, struct _build_ie_info *info)
{
	INT len = 0;
	struct wifi_dev *wdev = info->wdev;

	if (!wdev->bWmmCapable)
		return len;

	switch (wdev->wdev_type) {

	default:
#ifdef CONFIG_AP_SUPPORT
		len += build_ap_wmm_cap_ie(pAd, info->wdev, (UCHAR *)(info->frame_buf + len));
#endif /* CONFIG_AP_SUPPORT */
		break;
	}

	return len;
}

