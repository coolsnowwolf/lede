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
#include <net/cfg80211.h>
#ifdef IAPP_SUPPORT
extern BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);
#endif /*IAPP_SUPPORT*/

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data)
{
	/* UCHAR lock_channel = CFG80211_getCenCh(pAd, Data); */
	UCHAR lock_channel = Data;

    POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
    struct wifi_dev *wdev;

	if (RTMP_CFG80211_HOSTAPD_ON(pAd))
		return;
    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

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

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		wlan_operate_set_prim_ch(p2p_dev, lock_channel);
#else
		wlan_operate_set_prim_ch(wdev, lock_channel);
#endif

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Send Packet: From(%d)-To(%d)\n",
				 pAd->LatchRfRegs.Channel, lock_channel));
	} else
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Off-Channel Channel Equal: %d\n", pAd->LatchRfRegs.Channel));
}

#ifdef CONFIG_AP_SUPPORT

#ifdef DISABLE_HOSTAPD_PROBE_RESP
/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients for hostapd
	Parameters:
		apidx
	==========================================================================
 */
VOID ProbeResponseHandler(
	IN PRTMP_ADAPTER pAd,
	IN PEER_PROBE_REQ_PARAM *ProbeReqParam,
	IN UINT8 apidx)

{
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;

#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif

	CHAR rsne_idx = 0;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ADD_HT_INFO_IE *addht;
	UCHAR cfg_ht_bw;
	UCHAR op_ht_bw;
	struct _build_ie_info ie_info;
	UCHAR Environment = 0x20;
#ifdef CONFIG_HOTSPOT_R2
	extern UCHAR			OSEN_IE[];
	extern UCHAR			OSEN_IELEN;
	BSS_STRUCT *pMbss;
#endif /* CONFIG_HOTSPOT_R2 */
	struct DOT11_H *pDot11h = NULL;

#ifdef WDS_SUPPORT
	/* if in bridge mode, no need to reply probe req. */
	if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
		return;
#endif /* WDS_SUPPORT */

	{
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;
		rate = &wdev->rate;
		addht = wlan_operate_get_addht(wdev);
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			return;
		cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		op_ht_bw = wlan_operate_get_ht_bw(wdev);

		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
			!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev)))) {
			/* the interface is down, so we can not send probe response */
						return;
		}

		PhyMode = wdev->PhyMode;
		ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
		ie_info.channel = wdev->channel;
		ie_info.phy_mode = PhyMode;
		ie_info.wdev = wdev;

					if (((((ProbeReqParam->SsidLen == 0) && (!mbss->bHideSsid)) ||
			   ((ProbeReqParam->SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam->Ssid, mbss->Ssid, (ULONG) ProbeReqParam->SsidLen)))
#ifdef CONFIG_HOTSPOT
			   && ProbeReqforHSAP(pAd, apidx, ProbeReqParam)
#endif
			 )
		) {
			;
		} else {
						return;
		}

		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam->Addr2,
							wdev->if_addr, wdev->bssid);

		{
		SupRateLen = rate->SupRateLen;
		if (PhyMode == WMODE_B)
			SupRateLen = 4;

		MakeOutgoingFrame(pOutBuffer,				  &FrameLen,
						  sizeof(HEADER_802_11),	  &ProbeRspHdr,
						  TIMESTAMP_LEN,			  &FakeTimestamp,
						  2,						  &pAd->CommonCfg.BeaconPeriod,
						  2,						  &mbss->CapabilityInfo,
						  1,						  &SsidIe,
						  1,						  &mbss->SsidLen,
						  mbss->SsidLen,	 mbss->Ssid,
						  1,						  &SupRateIe,
						  1,						  &SupRateLen,
						  SupRateLen,				  rate->SupRate,
						  1,						  &DsIe,
						  1,						  &DsLen,
						  1,						  &wdev->channel,
						  END_OF_ARGS);
		}

		if ((rate->ExtRateLen) && (PhyMode != WMODE_B)) {
			MakeOutgoingFrame(pOutBuffer+FrameLen,		&TmpLen,
							  1,						&ErpIe,
							  1,						&ErpIeLen,
							  1,						&pAd->ApCfg.ErpIeContent,
							  1,						&ExtRateIe,
							  1,						&rate->ExtRateLen,
							  rate->ExtRateLen, 		rate->ExtRate,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable)) {
			ie_info.is_draft_n_type = FALSE;
			ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			FrameLen += build_ht_ies(pAd, &ie_info);
		}
#endif /* DOT11_N_SUPPORT */

		/* Append RSN_IE when  WPA OR WPAPSK, */
		pSecConfig = &wdev->SecConfig;
#ifdef CONFIG_HOTSPOT_R2
		pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
		if ((pMbss->HotSpotCtrl.HotSpotEnable == 0) && (pMbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
			UCHAR RSNIe = IE_WPA;

			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							  1, &RSNIe,
							  1, &OSEN_IELEN,
							  OSEN_IELEN, OSEN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
#ifdef DISABLE_HOSTAPD_PROBE_RESP
				BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				if (mbss->RSNIE_Len[rsne_idx] != 0) {
					MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
						1, &mbss->RSNIE_ID[rsne_idx],
						1, &mbss->RSNIE_Len[rsne_idx],
						mbss->RSNIE_Len[rsne_idx], &mbss->RSN_IE[rsne_idx][0],
						END_OF_ARGS);
					FrameLen += TmpLen;
				}

#else
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &pSecConfig->RSNE_EID[rsne_idx][0],
					1, &pSecConfig->RSNE_Len[rsne_idx],
					pSecConfig->RSNE_Len[rsne_idx], &pSecConfig->RSNE_Content[rsne_idx][0],
					END_OF_ARGS);

				FrameLen += TmpLen;
#endif  /*DISABLE_HOSTAPD_PROBE_RESP */
			}
		}
#ifdef HOSTAPD_OWE_SUPPORT
	if (mbss->TRANSIE_Len) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							 mbss->TRANSIE_Len, mbss->TRANS_IE,
							 END_OF_ARGS);
		FrameLen += TmpLen;
	}

#endif
#if defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT)
		if (pAd->ApCfg.MBSSID[apidx].GASCtrl.b11U_enable) {
			ULONG TmpLen;
			/* Interworking element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].GASCtrl.InterWorkingIELen,
							  pAd->ApCfg.MBSSID[apidx].GASCtrl.InterWorkingIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* Advertisement Protocol element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].GASCtrl.AdvertisementProtoIELen,
							  pAd->ApCfg.MBSSID[apidx].GASCtrl.AdvertisementProtoIE, END_OF_ARGS);

			FrameLen += TmpLen;
		}
#endif /* defined(CONFIG_HOTSPOT) || defined(FTM_SUPPORT) */

#ifdef CONFIG_HOTSPOT
		if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable) {
			ULONG TmpLen;
			/* Hotspot 2.0 Indication */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
					pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIELen,
					pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* Roaming Consortium element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.RoamingConsortiumIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.RoamingConsortiumIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* P2P element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.P2PIELen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.P2PIE, END_OF_ARGS);

			FrameLen += TmpLen;
		}
#endif

		/* Extended Capabilities IE */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_extended_cap_ie(pAd, &ie_info);
#ifdef AP_QLOAD_SUPPORT

		if (pAd->CommonCfg.dbdc_mode == 0)
			pQloadCtrl = HcGetQloadCtrl(pAd);
		else
			pQloadCtrl = (wdev->channel > 14) ? HcGetQloadCtrlByRf(pAd, RFIC_5GHZ) : HcGetQloadCtrlByRf(pAd, RFIC_24GHZ);

		if (pQloadCtrl->FlgQloadEnable != 0) {
#ifdef CONFIG_HOTSPOT_R2
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 1)
				FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pOutBuffer + FrameLen, apidx);
			else if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 0)
#endif
				FrameLen += QBSS_LoadElementAppend(pAd, pOutBuffer + FrameLen, pQloadCtrl);
		}
#endif /* AP_QLOAD_SUPPORT */

		/* add WMM IE here */
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
#ifdef VENDOR_FEATURE7_SUPPORT
		/* add country IE, power constraint IE */
		if (pAd->CommonCfg.bCountryFlag ||
			(wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
			|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
			) {
#ifndef EXT_BUILD_CHANNEL_LIST
			ULONG TmpLen = 0;
#endif /* !EXT_BUILD_CHANNEL_LIST */
			ULONG TmpLen2 = 0;
			UCHAR TmpFrame[256] = { 0 };
#ifndef EXT_BUILD_CHANNEL_LIST
			PCH_DESC pChDesc = NULL;
			UINT i;

			if (WMODE_CAP_2G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc2G != NULL)
					pChDesc = (PCH_DESC) pAd->CommonCfg.pChDesc2G;
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: pChDesc2G is NULL !!!\n", __func__));
			} else if (WMODE_CAP_5G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc5G != NULL)
					pChDesc = (PCH_DESC) pAd->CommonCfg.pChDesc5G;
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: pChDesc5G is NULL !!!\n", __func__));
			}
#endif /* !EXT_BUILD_CHANNEL_LIST */

			os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);
			if (TmpFrame != NULL) {
				NdisZeroMemory(TmpFrame, 256);

				/* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
				BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else
				{
					UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd,
						wdev->PhyMode, wdev->channel, op_ht_bw);

					for (i = 0; pChDesc[i].FirstChannel != 0; i++) {
						MakeOutgoingFrame(TmpFrame+TmpLen2,	&TmpLen,
							1,	&pChDesc[i].FirstChannel,
							1,	&pChDesc[i].NumOfCh,
							1,	&MaxTxPower,
							END_OF_ARGS);
						 TmpLen2 += TmpLen;
					}
				}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef A_BAND_SUPPORT
				/*
				* Only 802.11a APs that comply with 802.11h are required to include
				* a Power Constrint Element(IE=32) in beacons and probe response frames
				*/
				if ((wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
					|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
					) {

					UINT8 PwrConstraintIE = IE_POWER_CONSTRAINT;
					UINT8 PwrConstraintLen = 1;
					UINT8 PwrConstraint = pAd->CommonCfg.PwrConstraint;
					/* prepare power constraint IE */
					MakeOutgoingFrame(pOutBuffer+FrameLen,
								&TmpLen,
								1,	&PwrConstraintIE,
								1,	&PwrConstraintLen,
								1,	&PwrConstraint,
								END_OF_ARGS);
					FrameLen += TmpLen;

					/* prepare TPC Report IE */
					InsertTpcReportIE(pAd,
					pOutBuffer+FrameLen,
					&FrameLen,
					GetMaxTxPwr(pAd),
					0);
#ifdef DOT11_VHT_AC
					if (WMODE_CAP_AC(PhyMode)) {
						ULONG TmpLen;
						UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
						UINT8 ie_len;
						VHT_TXPWR_ENV_IE txpwr_env;

						ie_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
						MakeOutgoingFrame(pOutBuffer + FrameLen,
									&TmpLen,
									1,	&vht_txpwr_env_ie,
									1,	&ie_len,
									ie_len,	&txpwr_env,
									END_OF_ARGS);
						FrameLen += TmpLen;
					}
#endif /* DOT11_VHT_AC */
				}
#endif /* A_BAND_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
				if (IS_RRM_ENABLE(wdev)) {
					UCHAR reg_class = get_regulatory_class(pAd, mbss->wdev.channel,
								mbss->wdev.PhyMode, &mbss->wdev);
					TmpLen2 = 0;
					NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
					RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2, wdev->PhyMode, reg_class);
				}
#endif /* DOT11K_RRM_SUPPORT */

					/* add COUNTRY_IE in probe resp frames */
				{
					UCHAR CountryIe = IE_COUNTRY;
					/* need to do the padding bit check, and concatenate it */
					/* TmpFrame holds Channel list */
					if ((TmpLen2%2) == 0) {
						UCHAR TmpLen3 = TmpLen2+4;
						MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
									1,	&CountryIe,
									1,	&TmpLen3,
									3,	pAd->CommonCfg.CountryCode,
									TmpLen2+1,	TmpFrame,
									 END_OF_ARGS);
					} else {
						UCHAR TmpLen3 = TmpLen2+3;
						MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
									1,	&CountryIe,
									1,	&TmpLen3,
									3,	pAd->CommonCfg.CountryCode,
									TmpLen2, TmpFrame,
									END_OF_ARGS);
					}
				}
				FrameLen += TmpLen;
				os_free_mem(TmpFrame);
			} else
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s: Allocate memory fail!!!\n", __func__));
		}
#endif

#ifdef DOT11K_RRM_SUPPORT

		if (IS_RRM_ENABLE(wdev))
			RRM_InsertRRMEnCapIE(pAd, pOutBuffer + FrameLen, &FrameLen, apidx);

		InsertChannelRepIE(pAd, pOutBuffer + FrameLen, &FrameLen,
						   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
						   get_regulatory_class(pAd, mbss->wdev.channel, mbss->wdev.PhyMode, &mbss->wdev),
						   NULL, PhyMode);
#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pOutBuffer+FrameLen, &FrameLen);

		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pOutBuffer+FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */

#endif /* DOT11K_RRM_SUPPORT */

#ifndef VENDOR_FEATURE7_SUPPORT
		/* add Country IE and power-related IE */
		if (pAd->CommonCfg.bCountryFlag ||
			(wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
			|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
		   ) {
			ULONG TmpLen2 = 0;
			UCHAR TmpFrame[256] = { 0 };
			UCHAR CountryIe = IE_COUNTRY;
			PCH_DESC pChDesc = NULL;

			if (WMODE_CAP_2G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc2G != NULL)
					pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc2G;
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: pChDesc2G is NULL !!!\n", __func__));
			} else if (WMODE_CAP_5G(wdev->PhyMode)) {
				if (pAd->CommonCfg.pChDesc5G != NULL)
					pChDesc = (PCH_DESC)pAd->CommonCfg.pChDesc5G;
				else
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s: pChDesc5G is NULL !!!\n", __func__));
			}

			/*
				Only APs that comply with 802.11h or 802.11k are required to include
				the Power Constraint element (IE=32) and
				the TPC Report element (IE=35) and
				the VHT Transmit Power Envelope element (IE=195)
				in beacon frames and probe response frames
			*/
			if ((wdev->channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
#ifdef DOT11K_RRM_SUPPORT
				|| IS_RRM_ENABLE(wdev)
#endif /* DOT11K_RRM_SUPPORT */
			   ) {
				/* prepare power constraint IE */
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
						3, PowerConstraintIE, END_OF_ARGS);
				FrameLen += TmpLen;
				/* prepare TPC Report IE */
				InsertTpcReportIE(pAd,
								  pOutBuffer + FrameLen,
								  &FrameLen,
								  GetMaxTxPwr(pAd),
								  0);
#ifdef DOT11_VHT_AC
				/* prepare VHT Transmit Power Envelope IE */
				if (WMODE_CAP_AC(PhyMode)) {
					ULONG TmpLen;
					UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
					UINT8 ie_len;
					VHT_TXPWR_ENV_IE txpwr_env;

					ie_len = build_vht_txpwr_envelope(pAd, wdev, (UCHAR *)&txpwr_env);
					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1,							&vht_txpwr_env_ie,
								1,							&ie_len,
								ie_len, 					&txpwr_env,
								END_OF_ARGS);
					FrameLen += TmpLen;
				}
#endif /* DOT11_VHT_AC */
			}

			/* NdisZeroMemory(TmpFrame, sizeof(TmpFrame)); */
#ifdef EXT_BUILD_CHANNEL_LIST
			BuildBeaconChList(pAd, wdev, TmpFrame, &TmpLen2);
#else
			{
				UINT i = 0;
				UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

				if (pChDesc != NULL) {
				for (i = 0; pChDesc[i].FirstChannel != 0; i++) {
					 MakeOutgoingFrame(TmpFrame+TmpLen2,
						&TmpLen,
						1,
						&pChDesc[i].FirstChannel,
						1,
						&pChDesc[i].NumOfCh,
						1,
						&MaxTxPower,
						END_OF_ARGS);
					 TmpLen2 += TmpLen;
				}
			}
			}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11K_RRM_SUPPORT

			if (IS_RRM_ENABLE(wdev)) {
				UCHAR reg_class = get_regulatory_class(pAd, mbss->wdev.channel, mbss->wdev.PhyMode, &mbss->wdev);

				TmpLen2 = 0;
				NdisZeroMemory(TmpFrame, sizeof(TmpFrame));
				RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2, wdev->PhyMode, reg_class);
			}

#endif /* DOT11K_RRM_SUPPORT */
#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(wdev))
				Environment = MBO_AP_USE_GLOBAL_OPERATING_CLASS;
#endif /* MBO_SUPPORT */
			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2%2) == 0) {
				UCHAR TmpLen3 = TmpLen2 + 4;

		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2+1, TmpFrame,
								  END_OF_ARGS);
			} else {
				UCHAR TmpLen3 = TmpLen2 + 3;

				MakeOutgoingFrame(pOutBuffer + FrameLen,
								  &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2, TmpFrame,
								  END_OF_ARGS);
			}

			FrameLen += TmpLen;
		} /* Country IE - */
#endif
#ifdef A_BAND_SUPPORT

		/* add Channel switch announcement IE */
		if ((wdev->channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pDot11h->RDMode == RD_SWITCHING_MODE)) {
			UCHAR CSAIe = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
			UCHAR CSALen = 3;
			UCHAR CSAMode = 1;

			MakeOutgoingFrame(pOutBuffer+FrameLen,		&TmpLen,
							  1,						&CSAIe,
							  1,						&CSALen,
							  1,						&CSAMode,
							  1,						&wdev->channel,
							  1,                        &pDot11h->CSCount,
							  END_OF_ARGS);
	FrameLen += TmpLen;
#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.bExtChannelSwitchAnnouncement) {
				HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

				build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe, wdev->channel, wdev->PhyMode, wdev);
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								  sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE),	&HtExtChannelSwitchIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
#endif /* DOT11_N_SUPPORT */
		}
#endif /* A_BAND_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable)) {
#ifdef DOT11_VHT_AC
			struct _build_ie_info vht_ie_info;
#endif /*DOT11_VHT_AC*/
		if (pAd->bBroadComHT == TRUE) {
				struct _build_ie_info ie_info;

				ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
				ie_info.channel = wdev->channel;
				ie_info.phy_mode = wdev->PhyMode;
				ie_info.wdev = wdev;
				ie_info.is_draft_n_type = TRUE;
				ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
				FrameLen += build_ht_ies(pAd, &ie_info);
			}
#ifdef DOT11_VHT_AC
			vht_ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
			vht_ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
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


#ifdef WSC_AP_SUPPORT
		/* for windows 7 logo test */
		if ((mbss->WscControl.WscConfMode != WSC_DISABLE) &&
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
			ULONG TempLen1 = 0;
			UCHAR PROVISION_SERVICE_IE[7] = {0xDD, 0x05, 0x00, 0x50, 0xF2, 0x05, 0x00};
			MakeOutgoingFrame(pOutBuffer+FrameLen,		  &TempLen1,
								7,							  PROVISION_SERVICE_IE,
								END_OF_ARGS);
			FrameLen += TempLen1;
		}

		/* add Simple Config Information Element */
#ifdef DISABLE_HOSTAPD_PROBE_RESP
		if (mbss->WscIEProbeResp.ValueLen)
#else
		if ((mbss->WscControl.WscConfMode > WSC_DISABLE) && (mbss->WscIEProbeResp.ValueLen))
#endif
		{
			ULONG WscTmpLen = 0;
			MakeOutgoingFrame(pOutBuffer+FrameLen,									&WscTmpLen,
							  mbss->WscIEProbeResp.ValueLen,   mbss->WscIEProbeResp.Value,
							  END_OF_ARGS);
			FrameLen += WscTmpLen;
		}
#endif /* WSC_AP_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
		/* The Mobility Domain information element (MDIE) is present in Probe-
		** Request frame when dot11FastBssTransitionEnable is set to true. */
		if (pAd->ApCfg.MBSSID[apidx].wdev.FtCfg.FtCapFlag.Dot11rFtEnable) {
			PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[apidx].wdev.FtCfg;
			FT_CAP_AND_POLICY FtCap;

			FtCap.field.FtOverDs = pFtCfg->FtCapFlag.FtOverDs;
			FtCap.field.RsrReqCap = pFtCfg->FtCapFlag.RsrReqCap;
			FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen,
							pFtCfg->FtMdId, FtCap);
		}
#endif /* DOT11R_FT_SUPPORT */


	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
										 Byte0.b3=1 for rssi-feedback
	*/

    FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), VIE_PROBE_RESP
								);

	{
		/* Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM */

#ifdef RSSI_FEEDBACK
		UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
		ULONG TmpLen;

		if (ProbeReqParam->bRequestRssi == TRUE) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
										PRINT_MAC(ProbeReqParam->Addr2)));

			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, ProbeReqParam->Addr2);

			if (pEntry != NULL) {
				RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
				RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
				RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
			}
		}
		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							9, RalinkSpecificIe,
							END_OF_ARGS);
		FrameLen += TmpLen;
#endif /* RSSI_FEEDBACK */

	}

	/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	/*MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);*/
	MlmeFreeMemory(pOutBuffer);
	}
}

VOID CFG80211_SyncPacketWpsIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen, UINT8 apidx, UINT8 *da)
{

	const UCHAR *ssid_ie = NULL, *wsc_ie  = NULL;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[apidx];
	EID_STRUCT *eid;
	const UINT WFA_OUI = 0x0050F2;
	PEER_PROBE_REQ_PARAM ProbeReqParam;

	ssid_ie = cfg80211_find_ie(WLAN_EID_SSID, pData, dataLen);
	if (ssid_ie != NULL) {
		eid = (EID_STRUCT *)ssid_ie;
		ProbeReqParam.SsidLen = eid->Len;
		NdisCopyMemory(ProbeReqParam.Ssid, ssid_ie+2, eid->Len);
		NdisCopyMemory(ProbeReqParam.Addr2, da, MAC_ADDR_LEN);

	}
	wsc_ie = (UCHAR *)cfg80211_find_vendor_ie(WFA_OUI, 4, pData, dataLen);
	if (wsc_ie != NULL) {

		eid = (EID_STRUCT *)wsc_ie;

		if (eid->Len + 2 <= 500) {
			NdisCopyMemory(pMbss->WscIEProbeResp.Value, wsc_ie, eid->Len + 2);
			pMbss->WscIEProbeResp.ValueLen = eid->Len + 2;
		}

	}
	ProbeResponseHandler(pAd, &ProbeReqParam, apidx);
}

#endif /*DISABLE_HOSTAPD_PROBE_RESP */

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


#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
VOID CFG80211_AuthRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	AUTH_FRAME_INFO auth_info;	/* auth info from hostapd */
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfo;
#endif /* DOT11R_FT_SUPPORT */
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev;

	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)pData;

	UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AUTH - %s\n", __func__));


	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	ASSERT((wdev->func_idx == apidx));

	pEntry = MacTableLookup(pAd, mgmt->da);
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
#ifdef DOT11W_PMF_SUPPORT
	if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE)
		&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
	goto SendAuth;
#endif /* DOT11W_PMF_SUPPORT */

		if (!RTMPEqualMemory(mgmt->sa, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN)) {
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("AUTH - Bssid does not match\n"));
		} else {
#ifdef DOT11_N_SUPPORT
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s:ENTRY ALREADY EXIST, TERADOWN BLOCKACK SESSION\n", __func__));
			ba_session_tear_down_all(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */
		}
	}

#ifdef DOT11W_PMF_SUPPORT
SendAuth:
#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &wdev->FtCfg;
	if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
		&& (mgmt->u.auth.auth_alg == AUTH_MODE_FT)) {
		/* USHORT result; */

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry != NULL) {
			/* fill auth info from upper layer response */
			COPY_MAC_ADDR(auth_info.addr2, mgmt->da);
			COPY_MAC_ADDR(auth_info.addr1, wdev->if_addr);
			auth_info.auth_alg = mgmt->u.auth.auth_alg;
			auth_info.auth_seq = mgmt->u.auth.auth_transaction;
			auth_info.auth_status = mgmt->u.auth.status_code;

			/* os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO)); */
			pFtInfo = &(pEntry->auth_info_resp.FtInfo);
			{
				PEID_STRUCT eid_ptr;
				UCHAR *Ptr;
				UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
				/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

				NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

				/* Ptr = &Fr->Octet[6]; */
				Ptr = mgmt->u.auth.variable;
				eid_ptr = (PEID_STRUCT) Ptr;

			    /* get variable fields from payload and advance the pointer */
				while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
					switch (eid_ptr->Eid) {
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
							pFtInfo->RicInfo.Len = ((UCHAR *)mgmt + Data)
													- (UCHAR *)eid_ptr + 1;
						}

						if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;


					case IE_FT_RIC_DESCRIPTOR:
						if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;

					case IE_RSN:
						if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
							NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
							pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
						}
						break;

					default:
						break;
					}
				eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
				}
			}

			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				NdisMoveMemory(&pEntry->MdIeInfo, &auth_info.FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));

				pEntry->AuthState = AS_AUTH_OPEN;
				pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: AuthState:%d, Sst:%d\n", __func__, pEntry->AuthState, pEntry->Sst));
			}
#ifdef RADIUS_MAC_AUTH_SUPPORT
			else {
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
			}
#endif
		}
		return;
	} else
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_SAE_SUPPORT
	if (mgmt->u.auth.auth_alg == AUTH_MODE_SAE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SAE Auth Response Sequence %d \n",
					mgmt->u.auth.auth_transaction));
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, mgmt->da, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);


		if (pEntry) {
			if (mgmt->u.auth.status_code == MLME_SUCCESS) {
				pEntry->AuthState = AS_AUTH_OPEN;
				pEntry->Sst = SST_AUTH;
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable)
					pEntry->bAllowTraffic = TRUE;
#endif
			} else {
#ifdef RADIUS_MAC_AUTH_SUPPORT
				if (pEntry->wdev->radius_mac_auth_enable) {
					pEntry->bAllowTraffic = FALSE;
					MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
				}
#endif
			}
		}
	} else
#endif
	if ((auth_info.auth_alg == AUTH_MODE_OPEN) &&
		(!IS_AKM_SHARED(pMbss->wdev.SecConfig.AKMMap))) {
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry) {
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
#ifdef DOT11W_PMF_SUPPORT
		if ((pEntry->SecConfig.PmfCfg.UsePMFConnect == FALSE)
			|| (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED))
#endif /* DOT11W_PMF_SUPPORT */
			{
				pEntry->AuthState = AS_AUTH_OPEN;
				pEntry->Sst = SST_AUTH; /* what if it already in SST_ASSOC ??????? */
			}
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: pEntry created: auth state:%d, Sst:%d", __func__, pEntry->AuthState, pEntry->Sst));
			/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_SUCCESS); */

		} else
			; /* MAC table full, what should we respond ????? */
	} else {
		/* wrong algorithm */
		/* APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_ALG_NOT_SUPPORT); */

		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AUTH - Alg=%d, Seq=%d\n",
				auth_info.auth_alg, auth_info.auth_seq));
	}
}

VOID CFG80211_AssocRespHandler(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	/* BOOLEAN bAssocSkip = FALSE; */
	/* CHAR rssi; */
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocRspHdr;
	/* HEADER_802_11 AssocReqHdr; */
	USHORT CapabilityInfoForAssocResp;
	USHORT StatusCode = MLME_SUCCESS;
	USHORT Aid;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	UCHAR SupRateLen, PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	UCHAR isReassoc;
	UCHAR SubType;
	/* BOOLEAN bACLReject = FALSE; */
#ifdef DOT1X_SUPPORT
	PUINT8 pPmkid = NULL;
	UINT8 pmkid_count = 0;
#endif /* DOT1X_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg = NULL;
	PFT_INFO pFtInfoBuf = NULL; 	/*Wframe-larger-than=1024 warning  removal*/
#endif /* DOT11R_FT_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
		PEID_STRUCT pEcdhIe = NULL;
		PEID_STRUCT pRsnIe = NULL;
#endif
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
	ADD_HT_INFO_IE *addht;
	struct _build_ie_info ie_info;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */

	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)pData;

	UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ASSOC - %s\n", __func__));

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("Wrong Addr1 - %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(mgmt->sa)));
		goto LabelOK;
	}
	ASSERT((wdev->func_idx == apidx));

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &pMbss->WscControl;
#endif /* WSC_AP_SUPPORT */

	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);
	FlgIs11bSta = 1;

#ifdef DOT11R_FT_SUPPORT
	os_alloc_mem(NULL, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));
	if (pFtInfoBuf == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): pFtInfoBuf mem alloc failed\n", __func__));
		return;
	}
	NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));
#endif /* DOT11R_FT_SUPPORT */

	pEntry = MacTableLookup(pAd, mgmt->da);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("NoAuth MAC - %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(mgmt->da)));
		goto LabelOK;
	}

	ie_list = pEntry->ie_list;


	for (i = 0; i < ie_list->SupportedRatesLen; i++) {
		if (((ie_list->SupportedRates[i] & 0x7F) != 2) &&
			((ie_list->SupportedRates[i] & 0x7F) != 4) &&
			((ie_list->SupportedRates[i] & 0x7F) != 11) &&
			((ie_list->SupportedRates[i] & 0x7F) != 22)) {
			FlgIs11bSta = 0;
			break;
		}
	}

	if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():pEntry bounding invalid wdev(apidx=%d)\n",
					__func__, pEntry->func_tb_idx));
		goto LabelOK;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s():pEntry->func_tb_idx=%d\n",
				__func__, pEntry->func_tb_idx));

	tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];

	isReassoc = ieee80211_is_reassoc_resp(mgmt->frame_control);

	/* 2. qualify this STA's auth_asoc status in the MAC table, decide StatusCode */
	StatusCode = APBuildAssociation(pAd, pEntry, pEntry->ie_list, pEntry->MaxSupportedRate, &Aid, isReassoc);
	if (mgmt->u.reassoc_resp.status_code != MLME_SUCCESS)
		StatusCode = mgmt->u.reassoc_resp.status_code;

#ifdef WAPP_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */

	/*is status is success ,update STARec*/
	if (StatusCode == MLME_SUCCESS && (pEntry->Sst == SST_ASSOC)) {
		if (wdev_do_conn_act(pEntry->wdev, pEntry) != TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():connect action fail!!\n", __func__));
		}
	}
	if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
		BOOLEAN  bExtractIe = FALSE;
#ifdef DOT11R_FT_SUPPORT
		pFtCfg = &(wdev->FtCfg);
		if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
			&& (StatusCode == MLME_SUCCESS))
			bExtractIe = TRUE;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && IS_AKM_OWE(wdev->SecConfig.AKMMap))
			bExtractIe = TRUE;
#endif
		if (bExtractIe) {
				PEID_STRUCT eid_ptr;
				UCHAR *Ptr;
				UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
				/* PFT_INFO pFtInfo = &auth_info->FtInfo; */

				NdisZeroMemory(pFtInfoBuf, sizeof(FT_INFO));

				/* Ptr = &Fr->Octet[6]; */
				Ptr = mgmt->u.reassoc_resp.variable;
				eid_ptr = (PEID_STRUCT) Ptr;

				/* get variable fields from payload and advance the pointer */
				while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)mgmt + Data)) {
					switch (eid_ptr->Eid) {
#ifdef DOT11R_FT_SUPPORT
					case IE_FT_MDIE:
						FT_FillMdIeInfo(eid_ptr, &pFtInfoBuf->MdIeInfo);
						break;

					case IE_FT_FTIE:
						FT_FillFtIeInfo(eid_ptr, &pFtInfoBuf->FtIeInfo);
						break;

					case IE_FT_RIC_DATA:
						/* record the pointer of first RDIE. */
						if (pFtInfoBuf->RicInfo.pRicInfo == NULL) {
							pFtInfoBuf->RicInfo.pRicInfo = &eid_ptr->Eid;
							pFtInfoBuf->RicInfo.Len = ((UCHAR *)mgmt + Data)
													- (UCHAR *)eid_ptr + 1;
						}

						if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;

					case IE_FT_RIC_DESCRIPTOR:
						if ((pFtInfoBuf->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
							NdisMoveMemory(&pFtInfoBuf->RicInfo.RicIEs[pFtInfoBuf->RicInfo.RicIEsLen],
											&eid_ptr->Eid, eid_ptr->Len + 2);
							pFtInfoBuf->RicInfo.RicIEsLen += eid_ptr->Len + 2;
						}
						break;
#endif
#if defined(DOT11R_FT_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)

					case IE_RSN:
						if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
#ifdef DOT11R_FT_SUPPORT
							NdisMoveMemory(pFtInfoBuf->RSN_IE, eid_ptr, eid_ptr->Len + 2);
							pFtInfoBuf->RSNIE_Len = eid_ptr->Len + 2;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
							pRsnIe = eid_ptr;
#endif
						}
						break;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
					case IE_WLAN_EXTENSION:
					{
						/*parse EXTENSION EID*/
						UCHAR *extension_id = (UCHAR *)eid_ptr + 2;

						switch (*extension_id) {
						case IE_EXTENSION_ID_ECDH:
							pEcdhIe = eid_ptr;
						}
					}
						break;
#endif

					default:
						break;
					}
					eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
				}
			}

			if (mgmt->u.reassoc_resp.status_code == MLME_SUCCESS) {
/*				NdisMoveMemory(&pEntry->MdIeInfo, &auth_info.FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));
 *
 *				pEntry->AuthState = AS_AUTH_OPEN;
 *				pEntry->Sst = SST_AUTH;
*/
			}
		}
		/* just silencely discard this frame */
		/*if (StatusCode == 0xFFFF)
		*	goto LabelOK;
		*/

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
	/* SendAssocResponse: */
#endif /* DOT11W_PMF_SUPPORT */
	/* 3. send Association Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		goto LabelOK;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Reassoc - Send reassoc response (Status=%d)...\n",
			  StatusCode));
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
	MgtMacHeaderInit(pAd, &AssocRspHdr, SubType, 0, ie_list->Addr2,
					 wdev->if_addr, wdev->bssid);
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(HEADER_802_11), &AssocRspHdr,
					  2,						&CapabilityInfoForAssocResp,
					  2,						&StatusCode,
					  2,						&Aid,
					  1,						&SupRateIe,
					  1,						&SupRateLen,
					  SupRateLen,				rate->SupRate,
					  END_OF_ARGS);

	if ((rate->ExtRateLen) && (PhyMode != WMODE_B) && (FlgIs11bSta == 0)) {
		ULONG TmpLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,	  &TmpLen,
						  1,						&ExtRateIe,
						  1,						&rate->ExtRateLen,
						  rate->ExtRateLen, rate->ExtRate,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

#ifdef CONFIG_MAP_SUPPORT
	if (IS_MAP_ENABLE(pAd)) {
		pEntry->DevPeerRole = ie_list->MAP_AttriValue;
		MAP_InsertMapCapIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen);
	}
#endif /* CONFIG_MAP_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		RRM_InsertRRMEnCapIE(pAd, pOutBuffer + FrameLen, &FrameLen, pEntry->func_tb_idx);

#endif /* DOT11K_RRM_SUPPORT */

	/* add WMM IE here */
	/* printk("%s()=>bWmmCapable=%d,CLINE=%d\n",__FUNCTION__,wdev->bWmmCapable,CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)); */
	ie_info.is_draft_n_type = FALSE;
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wmm_cap_ie(pAd, &ie_info);
	}
	ie_info.channel = wdev->channel;
	ie_info.phy_mode = PhyMode;
	ie_info.wdev = wdev;
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
#ifdef HOSTAPD_OWE_SUPPORT
		if (wdev && IS_AKM_OWE(wdev->SecConfig.AKMMap)) {
			if (pRsnIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pRsnIe->Len + 2,
								  pRsnIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
			if (pEcdhIe) {
				ULONG TmpLen = 0;

				MakeOutgoingFrame(pOutBuffer+FrameLen,
								  &TmpLen,
								  pEcdhIe->Len + 2,
								  pEcdhIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
#endif
#ifdef DOT11R_FT_SUPPORT
	if ((pFtCfg != NULL) && (pFtCfg->FtCapFlag.Dot11rFtEnable)) {
		PUINT8	mdie_ptr;
		UINT8	mdie_len;
		PUINT8	ftie_ptr = NULL;
		UINT8	ftie_len = 0;
		PUINT8  ricie_ptr = NULL;
		UINT8   ricie_len = 0;
		/* struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig; */

		/* Insert RSNIE if necessary */
		if (pFtInfoBuf->RSNIE_Len != 0) {
			ULONG TmpLen;
			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
				pFtInfoBuf->RSNIE_Len, pFtInfoBuf->RSN_IE,
				END_OF_ARGS);
			FrameLen += TmpLen;
		}

		/* Insert MDIE. */
		mdie_ptr = pOutBuffer+FrameLen;
		mdie_len = 5;
		FT_InsertMdIE(pAd, pOutBuffer+FrameLen,
				&FrameLen,
			pFtInfoBuf->MdIeInfo.MdId,
			pFtInfoBuf->MdIeInfo.FtCapPlc);

		/* Insert FTIE. */
		if (pFtInfoBuf->FtIeInfo.Len != 0) {
			ftie_ptr = pOutBuffer+FrameLen;
			ftie_len = (2 + pFtInfoBuf->FtIeInfo.Len);
			FT_InsertFTIE(pAd, pOutBuffer+FrameLen, &FrameLen,
				pFtInfoBuf->FtIeInfo.Len,
				pFtInfoBuf->FtIeInfo.MICCtr,
				pFtInfoBuf->FtIeInfo.MIC,
				pFtInfoBuf->FtIeInfo.ANonce,
				pFtInfoBuf->FtIeInfo.SNonce);
		}
		/* Insert R1KH IE into FTIE. */
		if (pFtInfoBuf->FtIeInfo.R1khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pAd, pOutBuffer+FrameLen,
					&FrameLen,
					FT_R1KH_ID,
					pFtInfoBuf->FtIeInfo.R1khId,
					pFtInfoBuf->FtIeInfo.R1khIdLen);

		/* Insert GTK Key info into FTIE. */
		if (pFtInfoBuf->FtIeInfo.GtkLen != 0)
			FT_FTIE_InsertGTKSubIE(pAd, pOutBuffer+FrameLen,
					&FrameLen,
					pFtInfoBuf->FtIeInfo.GtkSubIE,
					pFtInfoBuf->FtIeInfo.GtkLen);

		/* Insert R0KH IE into FTIE. */
		if (pFtInfoBuf->FtIeInfo.R0khIdLen != 0)
			FT_FTIE_InsertKhIdSubIE(pAd, pOutBuffer+FrameLen,
					&FrameLen,
					FT_R0KH_ID,
					pFtInfoBuf->FtIeInfo.R0khId,
					pFtInfoBuf->FtIeInfo.R0khIdLen);

		/* Insert RIC. */
		if (ie_list->FtInfo.RicInfo.Len) {
			ULONG TempLen;

			FT_RIC_ResourceRequestHandle(pAd, pEntry,
						(PUCHAR)ie_list->FtInfo.RicInfo.pRicInfo,
						ie_list->FtInfo.RicInfo.Len,
						(PUCHAR)pOutBuffer+FrameLen,
						(PUINT32)&TempLen);
			ricie_ptr = (PUCHAR)(pOutBuffer+FrameLen);
			ricie_len = TempLen;
			FrameLen += TempLen;
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
#ifdef RT_CFG80211_SUPPORT
		/*	CFG80211OS_NewSta(pEntry->wdev->if_dev, ie_list->Addr2,
				(PUCHAR)Elem->Msg, Elem->MsgLen, isReassoc); */
#endif

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

		/* hex_dump("ASSOC_REQ", Elem->Msg, Elem->MsgLen); */
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
		 IS_AKM_WPA3_192BIT_Entry(pEntry)) {
		pPmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
						   ie_list->RSNIE_Len,
						   PMKID_LIST,
						   &pmkid_count);

		if (pPmkid != NULL) {
			/* Key cache */
			INT CacheIdx;

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
#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	map_a4_peer_enable(pAd, pEntry, TRUE);
#endif /* CONFIG_MAP_SUPPORT */
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
#ifdef DSCP_QOS_MAP_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)) {
	if (pMbss->DscpQosMapEnable) {
		pEntry->PoolId = pMbss->DscpQosPoolId;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("[DSCP-QOS-MAP] update sta mapping to CR4 for Pool %d wcid %d",
						pEntry->PoolId, pEntry->wcid));
		dscp_qosmap_update_sta_mapping_to_cr4(pAd, pEntry, pEntry->PoolId);
	}
	}
#endif
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
					"	 Station Info (Legacy):\n");
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

#ifdef DOT11R_FT_SUPPORT
	if (pFtInfoBuf != NULL)
		os_free_mem(pFtInfoBuf);
#endif /* DOT11R_FT_SUPPORT */

	return;
}
#endif /* HOSTAPD_11R_SUPPORT */

INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data)
{
	UCHAR	*pBuf = NULL;
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */

	if (pData != NULL) {
#ifdef CONFIG_AP_SUPPORT
		struct ieee80211_mgmt *mgmt;
#endif /* CONFIG_AP_SUPPORT */
#ifdef HOSTAPD_11R_SUPPORT
		UINT8 apidx;
#endif /* HOSTAPD_11R_SUPPORT */
		{
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Data);
	if (pBuf != NULL)
		NdisCopyMemory(pBuf, pData, Data);
	else {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG_TX_STATUS: MEM ALLOC ERROR\n"));
		return NDIS_STATUS_FAILURE;
	}
			CFG80211_CheckActionFrameType(pAd, "TX", pData, Data);
#ifdef CONFIG_AP_SUPPORT
			mgmt = (struct ieee80211_mgmt *)pData;
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
			apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
			if (ieee80211_is_probe_resp(mgmt->frame_control)) {
				INT offset = sizeof(HEADER_802_11) + 12;
#ifdef DISABLE_HOSTAPD_PROBE_RESP
#ifndef HOSTAPD_11R_SUPPORT
			UINT8 apidx = get_apidx_by_addr(pAd, mgmt->sa);
#endif
			CFG80211_SyncPacketWpsIe(pAd, pData + offset, Data - offset, apidx, mgmt->da);
			goto LabelOK;
#endif

			CFG80211_SyncPacketWmmIe(pAd, pData + offset, Data - offset);
			}
			if ((ieee80211_is_auth(mgmt->frame_control)) && (mgmt->u.auth.auth_alg != AUTH_MODE_FT) &&
				(mgmt->u.auth.auth_alg != AUTH_MODE_SAE)) {
#ifdef RADIUS_MAC_AUTH_SUPPORT
				MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, mgmt->da);
				if (pEntry != NULL && pEntry->wdev->radius_mac_auth_enable) {
					if (mgmt->u.auth.status_code == MLME_SUCCESS) {
							pEntry->bAllowTraffic = TRUE;
						} else {
							pEntry->bAllowTraffic = FALSE;
							MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
						}
				}
#endif
				goto LabelOK;
			}

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
			if (ieee80211_is_auth(mgmt->frame_control) &&
				((mgmt->u.auth.auth_alg == AUTH_MODE_FT) || (mgmt->u.auth.auth_alg == AUTH_MODE_SAE))) {
				CFG80211_AuthRespHandler(pAd, pData, Data);

				MiniportMMRequest(pAd, 0, pData, Data);

				if (pBuf) {
					CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, 5678,
										pBuf, Data,
										1);
				}
				goto LabelOK;
			}
				if (ieee80211_is_reassoc_resp(mgmt->frame_control)
				   || ieee80211_is_assoc_resp(mgmt->frame_control)) {
				CFG80211_AssocRespHandler(pAd, pData, Data);

				if (pBuf) {
					CFG80211OS_TxStatus(pAd->ApCfg.MBSSID[apidx].wdev.if_dev, 5678,
										pBuf, Data,
										1);
				}
				goto LabelOK;
			}
#endif

#endif /* CONFIG_AP_SUPPORT */
			MiniportMMRequest(pAd, 0, pData, Data);
		}
	}
LabelOK:
	if (pBuf != NULL)
		os_free_mem(pBuf);

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


#ifdef DISABLE_HOSTAPD_BEACON
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0))
    const UCHAR CFG_WPA_EID = WLAN_EID_VENDOR_SPECIFIC;
#else
    const UCHAR CFG_WPA_EID = WLAN_EID_WPA;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
#endif

	/* Security */
	PairCipher	 = Ndis802_11WEPDisabled;
	PairCipherAux = Ndis802_11WEPDisabled;
	CLEAR_SEC_AKM(wdev->SecConfig.AKMMap);
	CLEAR_PAIRWISE_CIPHER(&wdev->SecConfig);
	CLEAR_GROUP_CIPHER(&wdev->SecConfig);
#ifdef DOT11W_PMF_SUPPORT
	wdev->SecConfig.PmfCfg.MFPC = 0;
	wdev->SecConfig.PmfCfg.MFPR = 0;
	wdev->SecConfig.PmfCfg.igtk_cipher = 0;
#endif

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
#ifdef DISABLE_HOSTAPD_BEACON
			pMbss->RSNIE_ID[0] = CFG_WPA_EID;
#endif
		} else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: wpa open/none case\n", __func__));
			/* wdev->AuthMode = Ndis802_11AuthModeOpen; */
			/* wait until wpa/wpa2 all not exist , then set open/none */
		}
	}

	if (rsn_ie != NULL) {
		PRSN_IE_HEADER_STRUCT			pRsnHeader;
		PCIPHER_SUITE_STRUCT			pCipher;
		UCHAR							Len;

		pEid = (PEID_STRUCT)rsn_ie;
		Len	= pEid->Len + 2;
		pTmp = (PUCHAR)pEid;
		pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;

		/* 0. Version must be 1*/
		if (le2cpu16(pRsnHeader->Version) == 1) {
			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			Len	   -= sizeof(RSN_IE_HEADER_STRUCT);

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
				case 8:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						(" Group Ndis802_11GCMP128Enable\n"));
					SET_CIPHER_GCMP128(wdev->SecConfig.GroupCipher);
					break;
				case 9:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						(" Group Ndis802_11GCMP256Enable\n"));
					SET_CIPHER_GCMP256(wdev->SecConfig.GroupCipher);
					break;
				case 10:
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						(" Group Ndis802_11CCMP256Enable\n"));
					SET_CIPHER_CCMP256(wdev->SecConfig.GroupCipher);
					break;
				default:
					break;
				}

				/* set to correct offset for next parsing*/
				pTmp   += sizeof(CIPHER_SUITE_STRUCT);
				Len    -= sizeof(CIPHER_SUITE_STRUCT);

				/* 2. Get pairwise cipher counts*/
				/*Count = *(PUSHORT) pTmp;*/
				Count = (pTmp[1] << 8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				Len    -= sizeof(USHORT);

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

					case 8:
						TmpCipher = Ndis802_11GCMP128Enable;
						break;
							case 9:
								TmpCipher = Ndis802_11GCMP256Enable;
								break;
							case 10:
								TmpCipher = Ndis802_11CCMP256Enable;
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
					Len  -= sizeof(CIPHER_SUITE_STRUCT);
					Count--;
				}

				/* 4. get AKM suite counts*/
				/*Count	= *(PUSHORT) pTmp;*/
				Count = (pTmp[1] << 8) + pTmp[0];
				pTmp   += sizeof(USHORT);
				Len    -= sizeof(USHORT);

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
#ifdef HOSTAPD_11R_SUPPORT
					case 3:
						/* Set AP support FT WPA-enterprise mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2; */
						SET_AKM_FT_WPA2(wdev->SecConfig.AKMMap);
						break;
					case 4:
						/* Set AP support WPA-PSK mode*/
						/* wdev->AuthMode = Ndis802_11AuthModeWPA2PSK; */
						SET_AKM_FT_WPA2PSK(wdev->SecConfig.AKMMap);
						break;
#endif /* HOSTAPD_11R_SUPPORT */
					case 5:
						/* Set AP support WPA-PSK-EAP256 mode*/
#ifdef DOT11W_PMF_SUPPORT
						SET_AKM_WPA2(wdev->SecConfig.AKMMap);
						wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
#else
						SET_AKM_WPA2_SHA256(wdev->SecConfig.AKMMap);
#endif /*DOT11W_PMF_SUPPORT*/
						break;
					case 6:
						/* Set AP support WPA-PSK-SHA256 mode*/
#ifdef DOT11W_PMF_SUPPORT
						SET_AKM_WPA2PSK(wdev->SecConfig.AKMMap);
						wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
#else
						SET_AKM_WPA2PSK_SHA256(wdev->SecConfig.AKMMap);
#endif /*DOT11W_PMF_SUPPORT*/
						break;

#ifdef HOSTAPD_SAE_SUPPORT
							case 8:
								/*Set AP Support SAE SHA256 */
								SET_AKM_SAE_SHA256(wdev->SecConfig.AKMMap);
								break;
#endif
#ifdef HOSTAPD_SUITEB_SUPPORT
							case 11:
								SET_AKM_SUITEB_SHA256(wdev->SecConfig.AKMMap);
								break;

							case 12:
								SET_AKM_SUITEB_SHA384(wdev->SecConfig.AKMMap);
								break;
#endif
#ifdef HOSTAPD_OWE_SUPPORT
							case 18:
								SET_AKM_OWE(wdev->SecConfig.AKMMap);
								break;

#endif
					default:
						/* wdev->AuthMode = Ndis802_11AuthModeMax; */
						SET_AKM_OPEN(wdev->SecConfig.AKMMap);
						break;
					}

					pTmp   += sizeof(AKM_SUITE_STRUCT);
					Len    -= sizeof(AKM_SUITE_STRUCT);
					Count--;
				}

#ifdef DISABLE_HOSTAPD_BEACON
				/*check for no pairwise, pmf, ptksa, gtksa counters */
				if (Len >= 2)
				{
					memcpy(wdev->SecConfig.RsnCap, pTmp, 2);
#ifdef DOT11W_PMF_SUPPORT
					{
						RSN_CAPABILITIES RsnCap;

						NdisMoveMemory(&RsnCap, pTmp, sizeof(RSN_CAPABILITIES));
						RsnCap.word = cpu2le16(RsnCap.word);
						if (RsnCap.field.MFPC == 1)
							wdev->SecConfig.PmfCfg.Desired_MFPC = 1;
							if (RsnCap.field.MFPR == 1) {
								wdev->SecConfig.PmfCfg.Desired_MFPR = 1;
								wdev->SecConfig.PmfCfg.Desired_PMFSHA256 = 1;
						}
					}
#endif	 /*DOT11W_PMF_SUPPORT*/
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Copied Rsn cap %02x %02x \n", wdev->SecConfig.RsnCap[0], wdev->SecConfig.RsnCap[1]));
				}
					pTmp += sizeof(RSN_CAPABILITIES);
					Len  -= sizeof(RSN_CAPABILITIES);
					/*Extract PMKID list */
					if (Len >= sizeof(UINT16)) {
						INT offset = sizeof(UINT16);

						Count = (pTmp[1] << 8) + pTmp[0];
						if (Count > 0) {
							offset += Count*LEN_PMKID;
						}
						pTmp += offset;
						Len -= offset;
					}
#ifdef DOT11W_PMF_SUPPORT
					if (Len >= LEN_OUI_SUITE) {
						UCHAR OUI_PMF_BIP_CMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x06};
						UCHAR OUI_PMF_BIP_CMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0d};
						UCHAR OUI_PMF_BIP_GMAC_128_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0b};
						UCHAR OUI_PMF_BIP_GMAC_256_CIPHER[4] = {0x00, 0x0F, 0xAC, 0x0c};

						if (RTMPEqualMemory(pTmp, OUI_PMF_BIP_CMAC_128_CIPHER, LEN_OUI_SUITE))
							SET_CIPHER_BIP_CMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
						else if (RTMPEqualMemory(pTmp,
								OUI_PMF_BIP_CMAC_256_CIPHER, LEN_OUI_SUITE))
							SET_CIPHER_BIP_CMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
						else if (RTMPEqualMemory(pTmp,
								OUI_PMF_BIP_GMAC_128_CIPHER, LEN_OUI_SUITE))
							SET_CIPHER_BIP_GMAC128(wdev->SecConfig.PmfCfg.igtk_cipher);
						else if (RTMPEqualMemory(pTmp,
								OUI_PMF_BIP_GMAC_256_CIPHER, LEN_OUI_SUITE))
							SET_CIPHER_BIP_GMAC256(wdev->SecConfig.PmfCfg.igtk_cipher);
						else
							MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
								("Group Mgmt Cipher Not Supported \n"));
					}
#endif

#endif
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AuthMode = 0x%x\n", wdev->SecConfig.AKMMap));

				if ((PairCipher == Ndis802_11WEPDisabled && IS_CIPHER_NONE(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11WEPEnabled && IS_CIPHER_WEP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11TKIPEnable && IS_CIPHER_TKIP(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11AESEnable && IS_CIPHER_CCMP128(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11GCMP128Enable && IS_CIPHER_GCMP128(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11GCMP256Enable && IS_CIPHER_GCMP256(wdev->SecConfig.GroupCipher)) ||
					(PairCipher == Ndis802_11CCMP256Enable && IS_CIPHER_CCMP256(wdev->SecConfig.GroupCipher))
					) {
					wdev->SecConfig.PairwiseCipher = wdev->SecConfig.GroupCipher;
				} else {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WPA2 Mix TKIPAES\n"));
					bMix = TRUE;
				}

				if (bWPA2 && bWPA) {
					pMbss->RSNIE_Len[1] = rsn_ie[1];
					NdisMoveMemory(pMbss->RSN_IE[1], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
#ifdef DISABLE_HOSTAPD_BEACON
					pMbss->RSNIE_ID[1] = WLAN_EID_RSN;
#endif
				} else {
					pMbss->RSNIE_Len[0] = rsn_ie[1];
					os_move_mem(pMbss->RSN_IE[0], rsn_ie + 2, rsn_ie[1]); /* copy rsn ie */
#ifdef DISABLE_HOSTAPD_BEACON
					pMbss->RSNIE_ID[0] = WLAN_EID_RSN;
#endif
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

