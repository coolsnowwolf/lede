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
     sync.c

     Abstract:
     Synchronization state machine related services

     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
     John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"

#define OBSS_BEACON_RSSI_THRESHOLD		(-85)


/*
	==========================================================================
	Description:
		Process the received ProbeRequest from clients
	Parameters:
		Elem - msg containing the ProbeReq frame
	==========================================================================
 */
VOID APPeerProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	PEER_PROBE_REQ_PARAM ProbeReqParam = { {0} };
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR apidx = 0, PhyMode, SupRateLen;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
	struct freq_oper oper;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif
	CHAR rsne_idx = 0;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	UCHAR cfg_ht_bw;
	UCHAR op_ht_bw;
	struct _build_ie_info ie_info;
	UCHAR Environment = 0x20;
#ifdef CONFIG_HOTSPOT_R2
	extern UCHAR			OSEN_IE[];
	extern UCHAR			OSEN_IELEN;
#endif /* CONFIG_HOTSPOT_R2 */
	struct DOT11_H *pDot11h = NULL;
#ifdef WSC_AP_SUPPORT
	UCHAR Addr3[MAC_ADDR_LEN];
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;

	COPY_MAC_ADDR(Addr3, pFrame->Hdr.Addr3);
#endif /* WSC_AP_SUPPORT */
#ifdef WDS_SUPPORT

	/* if in bridge mode, no need to reply probe req. */
	if (pAd->WdsTab.Mode == WDS_BRIDGE_MODE)
		return;

#endif /* WDS_SUPPORT */

	if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, &ProbeReqParam) == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():shiang! PeerProbeReqSanity failed!\n", __func__));
		return;
	}


	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;
		rate = &wdev->rate;
		pDot11h = wdev->pDot11_H;
		if (pDot11h == NULL)
			continue;
		cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		op_ht_bw = wlan_operate_get_ht_bw(wdev);
		hc_radio_query_by_channel(pAd, wdev->channel, &oper);
		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
									   !(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev)))) {
			/* the interface is down, so we can not send probe response */
			continue;
		}

		if ((Elem->Channel != oper.prim_ch) && (Elem->Channel != oper.cen_ch_1))
			continue;

		PhyMode = wdev->PhyMode;

		ie_info.frame_subtype = SUBTYPE_PROBE_RSP;
		ie_info.channel = wdev->channel;
		ie_info.phy_mode = PhyMode;
		ie_info.wdev = wdev;

		if (((((ProbeReqParam.SsidLen == 0) && (!mbss->bHideSsid)) ||
			((ProbeReqParam.SsidLen == mbss->SsidLen) &&
			NdisEqualMemory(ProbeReqParam.Ssid, mbss->Ssid, (ULONG) ProbeReqParam.SsidLen)))
#ifdef CONFIG_HOTSPOT
			&& ProbeReqforHSAP(pAd, apidx, &ProbeReqParam)
#endif
			)
#ifdef WSC_AP_SUPPORT
			/* buffalo WPS testbed STA send ProbrRequest ssid length = 32 and ssid are not AP , but DA are AP. for WPS test send ProbeResponse */
			|| ((ProbeReqParam.SsidLen == 32) && MAC_ADDR_EQUAL(Addr3, wdev->bssid) && (mbss->bHideSsid == 0))
#endif /* WSC_AP_SUPPORT */
		   )
			;
		else {
			continue; /* check next BSS */
		}

#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering
	) {
		BOOLEAN bBndStrgCheck = TRUE;

		bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, ProbeReqParam.Addr2, Elem, &ProbeReqParam);
		if (bBndStrgCheck == FALSE)
			return;
	}
#endif /* BAND_STEERING */
#ifdef WAPP_SUPPORT
		wapp_send_cli_probe_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), ProbeReqParam.Addr2, Elem);
#endif
		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam.Addr2,
						 wdev->if_addr, wdev->bssid);
		{
			SupRateLen = rate->SupRateLen;

			if (PhyMode == WMODE_B)
				SupRateLen = 4;

			MakeOutgoingFrame(pOutBuffer,                 &FrameLen,
							  sizeof(HEADER_802_11),      &ProbeRspHdr,
							  TIMESTAMP_LEN,              &FakeTimestamp,
							  2,                          &pAd->CommonCfg.BeaconPeriod,
							  2,                          &mbss->CapabilityInfo,
							  1,                          &SsidIe,
							  1,                          &mbss->SsidLen,
							  mbss->SsidLen,     mbss->Ssid,
							  1,                          &SupRateIe,
							  1,                          &SupRateLen,
							  SupRateLen,                 rate->SupRate,
							  1,                          &DsIe,
							  1,                          &DsLen,
							  1,                          &wdev->channel,
							  END_OF_ARGS);
		}

		if ((rate->ExtRateLen) && (PhyMode != WMODE_B)) {
			MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
							  1,                        &ErpIe,
							  1,                        &ErpIeLen,
							  1,                        &pAd->ApCfg.ErpIeContent,
							  1,                        &ExtRateIe,
							  1,                        &rate->ExtRateLen,
							  rate->ExtRateLen, rate->ExtRate,
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

		if ((mbss->HotSpotCtrl.HotSpotEnable == 0) && (mbss->HotSpotCtrl.bASANEnable == 1) && (IS_AKM_WPA2_Entry(wdev))) {
			/* replace RSN IE with OSEN IE if it's OSEN wdev */
			UCHAR RSNIe = IE_WPA;

			MakeOutgoingFrame(pOutBuffer + FrameLen,			&TmpLen,
							  1,							&RSNIe,
							  1,							&OSEN_IELEN,
							  OSEN_IELEN,					OSEN_IE,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		} else
#endif /* CONFIG_HOTSPOT_R2 */
		{
			for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
				if (pSecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
					continue;

				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								  1, &pSecConfig->RSNE_EID[rsne_idx][0],
								  1, &pSecConfig->RSNE_Len[rsne_idx],
								  pSecConfig->RSNE_Len[rsne_idx], &pSecConfig->RSNE_Content[rsne_idx][0],
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}

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
		RRM_InsertBssACDelayIE(pAd, pOutBuffer + FrameLen, &FrameLen);
		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pOutBuffer + FrameLen, &FrameLen);
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
									  ie_len,					&txpwr_env,
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
						MakeOutgoingFrame(TmpFrame + TmpLen2,
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
			if ((TmpLen2 % 2) == 0) {
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

			MakeOutgoingFrame(pOutBuffer + FrameLen,      &TmpLen,
							  1,                        &CSAIe,
							  1,                        &CSALen,
							  1,                        &CSAMode,
							  1,                        &wdev->channel,
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
#ifdef WSC_INCLUDED
		ie_info.frame_buf = (UCHAR *)(pOutBuffer + FrameLen);
		FrameLen += build_wsc_ie(pAd, &ie_info);
#endif /* WSC_INCLUDED */
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
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
		MakeMboOceIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_PROBE_RSP);
	}
#endif /* MBO_SUPPORT */
		{
			/* Question to Rorscha: bit4 in old chip is used? but currently is using for 2.4G 256QAM */
#ifdef RSSI_FEEDBACK
			UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
			ULONG TmpLen;

			if (ProbeReqParam.bRequestRssi == TRUE) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
						 PRINT_MAC(ProbeReqParam.Addr2)));
				RalinkSpecificIe[5] |= 0x8;
				pEntry = MacTableLookup(pAd, ProbeReqParam.Addr2);

				if (pEntry != NULL) {
					RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
					RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
					RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
				}
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  9, RalinkSpecificIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
#endif /* RSSI_FEEDBACK */
		}
		/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		/* MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen); */
		MlmeFreeMemory(pOutBuffer);
	}
}


/*
	==========================================================================
	Description:
		parse the received BEACON

	NOTE:
		The only thing AP cares about received BEACON frames is to decide
		if there's any overlapped legacy BSS condition (OLBC).
		If OLBC happened, this AP should set the ERP->Use_Protection bit in its
		outgoing BEACON. The result is to tell all its clients to use RTS/CTS
		or CTS-to-self protection to protect B/G mixed traffic
	==========================================================================
 */


struct {
	ULONG	count;
	UCHAR	bssid[MAC_ADDR_LEN];
} BSSIDENTRY;


VOID APPeerBeaconAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem)
{
	UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES], *pRates = NULL, RatesLen;
	BOOLEAN LegacyBssExist;
	CHAR RealRssi;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	UCHAR MaxSupportedRate = 0;
	BCN_IE_LIST *ie_list = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	UCHAR Channel = 0;
	struct freq_oper oper;

	RETURN_IF_PAD_NULL(pAd);
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate ie_list fail!!!\n", __func__));
		goto LabelErr;
	}

	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));
	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);

	if (VarIE == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate VarIE fail!!!\n", __func__));
		goto LabelErr;
	}

	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	pRates = (PUCHAR)Rates;
	/* Init the DUT's working channel from RX'D param first, actually we need to get the accurate Channel from wdev */
	Channel = Elem->Channel;
	/* PeerBeaconAndProbeRspSanity() may overwrite ie_list->Channel if beacon or  probe resp contain IE_DS_PARM */
	ie_list->Channel = Elem->Channel;
	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

	if (PeerBeaconAndProbeRspSanity(pAd,
									Elem->Msg,
									Elem->MsgLen,
									Elem->Channel,
									ie_list,
									&LenVIE,
									pVIE,
									FALSE,
									FALSE)) {
		if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
			pEntry = MacTableLookup(pAd, ie_list->Addr2);/* Found the pEntry from Peer Bcn Content */

			if (!pEntry || !pEntry->wdev)
				goto __End_Of_APPeerBeaconAction;

			Channel = pEntry->wdev->channel;
		}

		hc_radio_query_by_channel(pAd, Channel, &oper);
#ifdef APCLI_SUPPORT
		ApCliCheckPeerExistence(pAd, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel);
#endif /* APCLI_SUPPORT */

		/* ignore BEACON not in this channel */
		if (ie_list->Channel != Channel
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef CFG80211_MULTI_STA
			&& (!RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev))
#endif /* CFG80211_MULTI_STA */
		   )
			goto __End_Of_APPeerBeaconAction;

#ifdef IDS_SUPPORT
		/* Conflict SSID detection */
		RTMPConflictSsidDetection(pAd, (PUCHAR)ie_list->Ssid, ie_list->SsidLen,
								  (CHAR)Elem->rssi_info.raw_rssi[0],
								  (CHAR)Elem->rssi_info.raw_rssi[1],
								  (CHAR)Elem->rssi_info.raw_rssi[2]);
#endif /* IDS_SUPPORT */
#ifdef DOT11_N_SUPPORT

		/* 40Mhz BSS Width Trigger events Intolerant devices */
		if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) && (ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) /* || (HtCapabilityLen == 0))) */
			Handle_BSS_Width_Trigger_Events(pAd, Channel);

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_N_SUPPORT

		if (Channel <= 14
			&& (oper.ht_bw == HT_BW_40)
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
		) {
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

			if (OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED) &&
				RTMP_CFG80211_MULTI_STA_ON(pAd, pAd->cfg80211_ctrl.multi_sta_net_dev)
				   ) {
					if (ie_list->Channel != Channel) {
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
							("Channel=%d is not equal as  band Channel = %d.\n",
							 ie_list->Channel, Channel));
					}
				} else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_CONCURRENT_DEVICE */
					if (((oper.cen_ch_1 + 2) != ie_list->Channel) &&
						((oper.cen_ch_1 - 2) != ie_list->Channel)) {
						/*
											MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x is a legacy BSS (%d)\n",
														Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], Channel));
						*/
					/* EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("APPeerBeaconAction 4\n")); */
					/* EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Central channel = %d\n", HcGetCentralChByRf(pAd,RFIC_24GHZ))); */
						goto __End_Of_APPeerBeaconAction;
					}
			} else {
				if (ie_list->Channel != Channel)
					goto __End_Of_APPeerBeaconAction;
		}

#endif /* DOT11_N_SUPPORT */
		SupportRate(ie_list->SupRate, ie_list->SupRateLen, ie_list->ExtRate, ie_list->ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

		if ((ie_list->Erp & 0x01) || (RatesLen <= 4))
			LegacyBssExist = TRUE;
		else
			LegacyBssExist = FALSE;

		if (LegacyBssExist && pAd->CommonCfg.DisableOLBCDetect == 0) {
			pAd->ApCfg.LastOLBCDetectTime = pAd->Mlme.Now32;
		}

#ifdef DOT11_N_SUPPORT

		if ((ie_list->HtCapabilityLen == 0) && (RealRssi > OBSS_BEACON_RSSI_THRESHOLD)) {
			pAd->ApCfg.LastNoneHTOLBCDetectTime = pAd->Mlme.Now32;
		}

#endif /* DOT11_N_SUPPORT */
#ifdef APCLI_SUPPORT

		if (VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid)) {
			pEntry = &pAd->MacTab.Content[Elem->Wcid];

			if (pEntry &&
				(IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) &&
				(pEntry->func_tb_idx < MAX_APCLI_NUM)) {
				PAPCLI_STRUCT pApCliEntry = NULL;

				pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
				pApCliEntry->ApCliRcvBeaconTime = pAd->Mlme.Now32;
				AdjustBwToSyncAp(pAd, ie_list, pApCliEntry->MacTabWCID, pEntry->func_tb_idx);

#ifdef APCLI_CERT_SUPPORT
				if (pAd->bApCliCertTest == TRUE) {
					UCHAR RegClass;
					OVERLAP_BSS_SCAN_IE BssScan;
					BOOLEAN brc;
#ifdef DOT11_N_SUPPORT
					ADD_HT_INFO_IE *aux_add_ht = &pApCliEntry->MlmeAux.AddHtInfo;
					ADD_HT_INFO_IE *addht;
					BOOLEAN bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;
					UINT16 OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
#endif /* DOT11_N_SUPPORT */

					brc = PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);

					if (brc == TRUE) {
						pAd->CommonCfg.Dot11BssWidthTriggerScanInt = le2cpu16(BssScan.TriggerScanInt); /*APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];*/

						/*DBGPRINT(RT_DEBUG_ERROR,("Update Dot11BssWidthTriggerScanInt=%d\n", pAd->CommonCfg.Dot11BssWidthTriggerScanInt)); */
						/* out of range defined in MIB... So fall back to default value.*/
						if ((pAd->CommonCfg.Dot11BssWidthTriggerScanInt < 10) || (pAd->CommonCfg.Dot11BssWidthTriggerScanInt > 900))
							pAd->CommonCfg.Dot11BssWidthTriggerScanInt = 900;
					}
#ifdef DOT11_N_SUPPORT
					/* check Ht protection mode. and adhere to the Non-GF device indication by AP. */
					if (ie_list->AddHtInfoLen != 0) {
						if ((ie_list->AddHtInfo.AddHtInfo2.OperaionMode != OperationMode)
							|| (ie_list->AddHtInfo.AddHtInfo2.NonGfPresent != bNonGFExist)) {
							aux_add_ht->AddHtInfo2.OperaionMode = ie_list->AddHtInfo.AddHtInfo2.OperaionMode;
							aux_add_ht->AddHtInfo2.NonGfPresent = ie_list->AddHtInfo.AddHtInfo2.NonGfPresent;

							pApCliEntry->wdev.protection = SET_PROTECT(ie_list->AddHtInfo.AddHtInfo2.OperaionMode);

							OperationMode = aux_add_ht->AddHtInfo2.OperaionMode;
							bNonGFExist = (aux_add_ht->AddHtInfo2.NonGfPresent) ? TRUE : FALSE;

							if (bNonGFExist) {
								pApCliEntry->wdev.protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
							} else {
								pApCliEntry->wdev.protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));
							}

							if (pApCliEntry->wdev.channel > 14) {
								/* always no BG protection in A-band.
								* falsely happened when switching A/G band to a dual-band AP */
								pApCliEntry->wdev.protection &= ~(SET_PROTECT(ERP));
							}
							addht = wlan_operate_get_addht(&pApCliEntry->wdev);
							if (addht) { /* sync addht information into wlan operation addht */
								*addht = pApCliEntry->MlmeAux.AddHtInfo;
							}

							AsicUpdateProtect(pAd);

							MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
								("SYNC - AP changed N OperaionMode to %d, my protection to %d\n",
								OperationMode, pApCliEntry->wdev.protection));
						}
					}
#endif /* DOT11_N_SUPPORT */
				}
#endif /* APCLI_CERT_SUPPORT */
			}

			if (pEntry && ie_list->NewChannel != 0)
				ApCliPeerCsaAction(pAd, pEntry->wdev, ie_list);
		}

#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

		if (pAd->WdsTab.Mode != WDS_DISABLE_MODE) {
			if (pAd->WdsTab.flg_wds_init) {
				MAC_TABLE_ENTRY *pEntry;
				BOOLEAN bWmmCapable;
				/* check BEACON does in WDS TABLE. */
				pEntry = WdsTableLookup(pAd, ie_list->Addr2, FALSE);
				bWmmCapable = ie_list->EdcaParm.bValid ? TRUE : FALSE;

				if (pEntry && (pEntry->bAutoTxRateSwitch == TRUE))
					WdsPeerBeaconProc(pAd, pEntry, MaxSupportedRate, RatesLen, bWmmCapable,	ie_list);
			} else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(), ERROR!! Beacon comes before wds_init\n", __func__));
		}

#endif /* WDS_SUPPORT */
	if (Elem->wdev != NULL) {
		bss_coex_insert_effected_ch_list(pAd, Channel, ie_list, Elem->wdev);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), Elem->wdev = NULL\n", __func__));
	}

	}

	/* sanity check fail, ignore this frame */
__End_Of_APPeerBeaconAction:
	/*#ifdef AUTO_CH_SELECT_ENHANCE */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		UCHAR BandIdx = BAND0;
		AUTO_CH_CTRL *pAutoChCtrl = NULL;
		if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel) {
			if (Elem->wdev != NULL)
				BandIdx = HcGetBandByWdev(Elem->wdev);
			else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(), Elem->wdev = NULL\n", __func__));

			pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

			if (pAutoChCtrl->pChannelInfo && AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, Elem->wdev) == BSS_NOT_FOUND)
				pAutoChCtrl->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;

			AutoChBssInsertEntry(pAd, ie_list->Bssid, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi, Elem->wdev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	/*#endif // AUTO_CH_SELECT_ENHANCE */
LabelErr:

	if (VarIE != NULL)
		os_free_mem(VarIE);

	if (ie_list != NULL)
		os_free_mem(ie_list);
}

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID APInvalidStateWhenScan(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.ApSyncMachine.CurrState));
}

/*
    ==========================================================================
    Description:
	Scan timeout handler, executed in timer thread
    ==========================================================================
 */
VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP SYNC - Scan Timeout\n"));
	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_SCAN_TIMEOUT, 0, NULL, (ULONG)pAd->ApCfg.ScanReqwdev);
	RTMP_MLME_HANDLER(pAd);
}

/*
    ==========================================================================
    Description:
	Scan timeout procedure. basically add channel index by 1 and rescan
    ==========================================================================
 */
VOID APScanTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	struct wifi_dev *wdev = NULL;
	if (pAd->ApCfg.ScanReqwdev)
		wdev = pAd->ApCfg.ScanReqwdev;
	else
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;

	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel, wdev);

	/* only scan the channel which binding band supported */
	if (pAd->ApCfg.ScanReqwdev != NULL && (pAd->ScanCtrl.Channel != 0)) {
		while ((WMODE_CAP_2G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel > 14) ||
			(WMODE_CAP_5G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel <= 14)) {
			pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel, wdev);

			if (pAd->ScanCtrl.Channel == 0)
				break;
		}
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/*
			iwpriv set auto channel selection
			update the current index of the channel
		*/
		if (pAd->ApCfg.bAutoChannelAtBootup == TRUE) {
			/* update current channel info */
			UpdateChannelInfo(pAd, pAd->ApCfg.current_channel_index, pAd->ApCfg.AutoChannelAlg, wdev);
			/* move to next channel */
			pAd->ApCfg.current_channel_index++;

			if (pAd->ApCfg.current_channel_index < pChCtrl->ChListNum)
				pAd->ApCfg.AutoChannel_Channel = pChCtrl->ChList[pAd->ApCfg.current_channel_index].Channel;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	ScanNextChannel(pAd, OPMODE_AP, pAd->ApCfg.ScanReqwdev);
}

#ifdef CON_WPS
VOID APMlmeScanCompleteAction(PRTMP_ADAPTER pAd, MLME_QUEUE_ELEM *Elem)
{
	PWSC_CTRL   pWscControl;
	PWSC_CTRL   pApCliWscControl;
	UCHAR       apidx;
	INT         IsAPConfigured;
	struct wifi_dev *wdev;
	BOOLEAN     bNeedSetPBCTimer = TRUE;
#if defined(CON_WPS)
	INT currIfaceIdx = 0;
	UCHAR ifIdx;
	UCHAR oppifIdx;
	struct wifi_dev *ConWpsdev = NULL;
	PWSC_CTRL   pTriggerApCliWscControl;
	PWSC_CTRL   pOpposApCliWscControl;
	PRTMP_ADAPTER pOpposAd;
	BOOLEAN     bTwoCardConWPS = FALSE;
	UCHAR apcli_idx;
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
	UINT opposIfaceIdx = !multi_inf_get_idx(pAd);
#endif /* MULTI_INF_SUPPORT */
#endif /*CON_WPS*/
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP SYNC - APMlmeScanCompleteAction\n"));
#if defined(CON_WPS)
	pOpposAd = NULL;
	pOpposApCliWscControl = NULL;
	pTriggerApCliWscControl = NULL;
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */
	pOpposAd = (PRTMP_ADAPTER)adapt_list[opposIfaceIdx];
#endif /* MULTI_INF_SUPPORT */
#endif /*CON_WPS*/

	/* If We catch the SR=TRUE in last scan_res, stop the AP Wsc SM */
	if (Elem) {
		ifIdx = (USHORT)(Elem->Priv);

		if (ifIdx < pAd->ApCfg.ApCliNum)
			ConWpsdev =  &(pAd->ApCfg.ApCliTab[ifIdx].wdev);

		if (ConWpsdev == NULL)
			return;
	} else
		return;

	if (ifIdx == BSS0)
		oppifIdx = BSS1;
	else if (ifIdx == BSS1)
		oppifIdx = BSS0;
	else
		return;

	if (ConWpsdev) {
		pApCliWscControl = &pAd->ApCfg.ApCliTab[ifIdx].wdev.WscControl;
		pAd->ApCfg.ApCliTab[ifIdx].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_FINISH;
	}

	if (pOpposAd) {
		for (apcli_idx = 0; apcli_idx < pOpposAd->ApCfg.ApCliNum; apcli_idx++) {
			if (pOpposAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl.conWscStatus == CON_WPS_STATUS_APCLI_RUNNING) {
				pOpposApCliWscControl = &pOpposAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl;
				bTwoCardConWPS = TRUE;
				break;
			}
		}

		if (apcli_idx == pOpposAd->ApCfg.ApCliNum) {
			pOpposApCliWscControl = NULL;
			bTwoCardConWPS = FALSE;
		}
	}

	if (bTwoCardConWPS == FALSE) {
		for (apcli_idx = 0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
			if (apcli_idx == ifIdx)
				continue;
			else if (pAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl.conWscStatus == CON_WPS_STATUS_APCLI_RUNNING) {
				pOpposApCliWscControl = &pAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl;
				break;
			}
		}
	}

	if (pOpposAd && pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) { /* 2.2G and 5G must trigger scan */
		if (pOpposAd && bTwoCardConWPS) {
			for (apcli_idx = 0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
				if (pOpposAd->ApCfg.ApCliTab[apcli_idx].ConWpsApCliModeScanDoneStatus == CON_WPS_APCLI_SCANDONE_STATUS_ONGOING) {
					pApCliWscControl->ConWscApcliScanDoneCheckTimerRunning = TRUE;
					RTMPSetTimer(&pApCliWscControl->ConWscApcliScanDoneCheckTimer, 1000);
					return;
				}
			}
		}
	} else {
		for (apcli_idx = 0; apcli_idx < pAd->ApCfg.ApCliNum; apcli_idx++) {
			if (pAd->ApCfg.ApCliTab[apcli_idx].ConWpsApCliModeScanDoneStatus == CON_WPS_APCLI_SCANDONE_STATUS_ONGOING) {
				pApCliWscControl->ConWscApcliScanDoneCheckTimerRunning = TRUE;

				if (pAd->Mlme.ApSyncMachine.CurrState == 0)
					WscScanExec(pAd, &(pAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl));

				RTMPSetTimer(&pApCliWscControl->ConWscApcliScanDoneCheckTimer, 1000);
				return;
			}
		}
	}

	if ((pOpposApCliWscControl) == NULL && pOpposAd) {
			pOpposApCliWscControl = &pOpposAd->ApCfg.ApCliTab[BSS0].WscControl;
			bTwoCardConWPS = TRUE;
	}


	if (pOpposApCliWscControl == NULL) {
			pOpposApCliWscControl = &pAd->ApCfg.ApCliTab[oppifIdx].WscControl;
			bTwoCardConWPS = FALSE;
	}

	WscPBCBssTableSort(pAd, pApCliWscControl);
#if defined(CON_WPS)
#ifdef MULTI_INF_SUPPORT /* Index 0 for 2.4G, 1 for 5Ghz Card */

	if (pOpposAd && bTwoCardConWPS) {
		if (pOpposApCliWscControl)
			WscPBCBssTableSort(pOpposAd, pOpposApCliWscControl);
	} else
#endif /* MULTI_INF_SUPPORT */
	{
		if (pOpposApCliWscControl)
			WscPBCBssTableSort(pAd, pOpposApCliWscControl);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[Iface_Idx: %d] Scan_Completed!!! In APMlmeScanCompleteAction\n", currIfaceIdx));
#endif /*CON_WPS*/
#ifdef MULTI_INF_SUPPORT
	currIfaceIdx = multi_inf_get_idx(pAd);
#else
	currIfaceIdx = (pApCliWscControl->EntryIfIdx & 0x0F);
#endif /* MULTI_INF_SUPPORT */

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
		pWscControl = &wdev->WscControl;
		IsAPConfigured = pWscControl->WscConfStatus;

		if ((pWscControl->WscConfMode != WSC_DISABLE) &&
			(pApCliWscControl->WscPBCBssCount > 0)) {
			if (pWscControl->bWscTrigger == TRUE) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s CON_WPS[%d]: Stop the AP Wsc Machine\n", __func__, apidx));
				WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
				WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, apidx, NULL, 0, AP_MODE);
				UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
				WscStop(pAd, FALSE, pWscControl);
			}

			WscConWpsStop(pAd, FALSE, pWscControl);
		}

		continue;
	}

	if (bTwoCardConWPS) {
		if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 1) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] AutoPreferIface = %d\n"
					 , currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));

			if (currIfaceIdx == 0) {
				if (pAd->ApCfg.ConWpsApcliAutoPreferIface == CON_WPS_APCLI_AUTO_PREFER_IFACE1) {
					bNeedSetPBCTimer = FALSE;
					WscStop(pAd, TRUE, pApCliWscControl);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", currIfaceIdx));
				} else {
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", !currIfaceIdx));
				}
			} else if (currIfaceIdx == 1) {
				if (pAd->ApCfg.ConWpsApcliAutoPreferIface == CON_WPS_APCLI_AUTO_PREFER_IFACE0) {
					bNeedSetPBCTimer = FALSE;
					WscStop(pAd, TRUE, pApCliWscControl);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!!STOP APCLI = %d !!\n", currIfaceIdx));
				} else {
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! STOP APCLI = %d !!\n", !currIfaceIdx));
				}
			}
		}

		if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 0) {
			WscConWpsStop(pAd, TRUE, pApCliWscControl);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (5)STOP APCLI = %d !!\n", !currIfaceIdx));
		}
	} else {
		currIfaceIdx = (pApCliWscControl->EntryIfIdx & 0x0F);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] Registrar_Found,  APCLI_Auto_Mode PreferIface = %d\n",
				 currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] WscPBCBssCount = %d, opposWscPBCBssCount = %d\n",
				 currIfaceIdx,
				 pApCliWscControl->WscPBCBssCount,
				 pOpposApCliWscControl->WscPBCBssCount));

		if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 1) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Iface_Idx: %d] AutoPreferIface = %d\n"
					 , currIfaceIdx, pAd->ApCfg.ConWpsApcliAutoPreferIface));

			if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) {
				if (currIfaceIdx != pAd->ApCfg.ConWpsApcliAutoPreferIface) {
					bNeedSetPBCTimer = FALSE;
					WscStop(pAd, TRUE, pApCliWscControl);
					WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
					pTriggerApCliWscControl = pOpposApCliWscControl;
				} else {
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
					pTriggerApCliWscControl = pApCliWscControl;
				}
			} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G) {
				WscConWpsStop(pAd, TRUE, &(pAd->ApCfg.ApCliTab[BSS0].wdev.WscControl));
				pTriggerApCliWscControl =  &(pAd->ApCfg.ApCliTab[BSS0].wdev.WscControl);
			} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G) {
				WscConWpsStop(pAd, TRUE, &(pAd->ApCfg.ApCliTab[BSS1].wdev.WscControl));
				pTriggerApCliWscControl =  &(pAd->ApCfg.ApCliTab[BSS1].wdev.WscControl);
			}
		}

		/*Only Found 1 Registrar at one interface*/
		if (pApCliWscControl->WscPBCBssCount == 1 && pOpposApCliWscControl->WscPBCBssCount == 0) {
			if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) {
				WscConWpsStop(pAd, TRUE, pApCliWscControl);
				pTriggerApCliWscControl = pApCliWscControl;
			} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G) {
				if (currIfaceIdx == 0) {
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
					pTriggerApCliWscControl = pApCliWscControl;
				}
			} else if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G) {
				if (currIfaceIdx == 1) {
					WscConWpsStop(pAd, TRUE, pApCliWscControl);
					pTriggerApCliWscControl = pApCliWscControl;
				}
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (5)STOP APCLI = %d !!\n", !currIfaceIdx));
		} else if (pApCliWscControl->WscPBCBssCount == 0 && pOpposApCliWscControl->WscPBCBssCount == 1) {
			if (pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_AUTO) {
				WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
				pTriggerApCliWscControl = pOpposApCliWscControl;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("!! (6)STOP APCLI = %d !!\n", !currIfaceIdx));
			} else {
				if ((pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_2G) && (pOpposApCliWscControl->EntryIfIdx & 0x0F) == 0) {
					WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
					pTriggerApCliWscControl = pOpposApCliWscControl;
				} else if ((pAd->ApCfg.ConWpsApCliMode == CON_WPS_APCLI_BAND_5G) && (pOpposApCliWscControl->EntryIfIdx & 0x0F) == 1) {
					WscConWpsStop(pAd, TRUE, pOpposApCliWscControl);
					pTriggerApCliWscControl = pOpposApCliWscControl;
				}
			}
		}
	}

	if (bTwoCardConWPS) {
		if (bNeedSetPBCTimer && pApCliWscControl->WscPBCTimerRunning == FALSE) {
			if (pApCliWscControl->bWscTrigger) {
				pApCliWscControl->WscPBCTimerRunning = TRUE;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!! TwoCardConWPS Trigger %s WPS!!\n", (pApCliWscControl->IfName)));
				RTMPSetTimer(&pApCliWscControl->WscPBCTimer, 1000);
			}
		}
	} else {
		if (pTriggerApCliWscControl != NULL &&
			(pTriggerApCliWscControl->WscPBCTimerRunning == FALSE) &&
			(pTriggerApCliWscControl->bWscTrigger == TRUE)) {
			pTriggerApCliWscControl->WscPBCTimerRunning = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!! One Card DBDC Trigger %s WPS!!\n", (pTriggerApCliWscControl->IfName)));
			RTMPSetTimer(&pTriggerApCliWscControl->WscPBCTimer, 1000);
		} else {
			if (pApCliWscControl && pApCliWscControl->WscPBCTimerRunning == FALSE &&
				(pApCliWscControl->bWscTrigger == TRUE)) {
				pAd->ApCfg.ApCliTab[(pApCliWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
				pApCliWscControl->WscPBCTimerRunning = TRUE;
				RTMPSetTimer(&pApCliWscControl->WscPBCTimer, 1000);
			}

			if (pOpposApCliWscControl && pOpposApCliWscControl->WscPBCTimerRunning == FALSE &&
				(pOpposApCliWscControl->bWscTrigger == TRUE)) {
				pAd->ApCfg.ApCliTab[(pOpposApCliWscControl->EntryIfIdx & 0x0F)].ConWpsApCliModeScanDoneStatus = CON_WPS_APCLI_SCANDONE_STATUS_ONGOING;
				pOpposApCliWscControl->WscPBCTimerRunning = TRUE;
				RTMPSetTimer(&pOpposApCliWscControl->WscPBCTimer, 1000);
			}
		}
	}
}
#endif /* CON_WPS*/

/*
    ==========================================================================
    Description:
	MLME SCAN req state machine procedure
    ==========================================================================
 */
VOID APMlmeScanReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	UCHAR Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType;
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Priv;
#ifdef CONFIG_AP_SUPPORT
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#endif

	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, Elem->Msg, Elem->MsgLen, &BssType, (PCHAR)Ssid, &SsidLen, &ScanType)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP SYNC - MlmeScanReqAction\n"));
		NdisGetSystemUpTime(&pAd->ApCfg.LastScanTime);
		RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
		/* record desired BSS parameters */
		pAd->ScanCtrl.BssType = BssType;
		pAd->ScanCtrl.ScanType = ScanType;
		pAd->ScanCtrl.SsidLen = SsidLen;
		NdisMoveMemory(pAd->ScanCtrl.Ssid, Ssid, SsidLen);

		/* start from the first channel */
		if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
			/* only scan the channel which binding band supported */
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;
find_next_channel:
			pAd->ScanCtrl.Channel = FindScanChannel(pAd, 0, wdev);

			if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
				if ((WMODE_CAP_2G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel > 14) ||
					(WMODE_CAP_5G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel <= 14)) {
					pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel, wdev);

					if (pAd->ScanCtrl.Channel == 0) {
						if (pAd->ScanCtrl.PartialScan.bScanning == TRUE) {
							pAd->ScanCtrl.PartialScan.NumOfChannels = DEFLAUT_PARTIAL_SCAN_CH_NUM;
							goto find_next_channel;
						}
					}
				}
			}
		} else {
			pAd->ScanCtrl.Channel = FindScanChannel(pAd, 0, wdev);
			/* only scan the channel which binding band supported */
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;

			if (pAd->ApCfg.ScanReqwdev != NULL) {
				while ((WMODE_CAP_2G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel > 14) ||
					(WMODE_CAP_5G(pAd->ApCfg.ScanReqwdev->PhyMode) && pAd->ScanCtrl.Channel <= 14)) {
					pAd->ScanCtrl.Channel = FindScanChannel(pAd, pAd->ScanCtrl.Channel, wdev);

					if (pAd->ScanCtrl.Channel == 0)
						break;
				}
			}
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->ApCfg.bAutoChannelAtBootup == TRUE) { /* iwpriv set auto channel selection */
				APAutoChannelInit(pAd, wdev);
				pAd->ApCfg.AutoChannel_Channel = pChCtrl->ChList[0].Channel;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		ScanNextChannel(pAd, OPMODE_AP, (struct wifi_dev *)Elem->Priv);
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP SYNC - MlmeScanReqAction() sanity check fail. BUG!!!\n"));
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	}
}


/*
    ==========================================================================
    Description:
	peer sends beacon back when scanning
    ==========================================================================
 */
VOID APPeerBeaconAtScanAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	CHAR RealRssi = -127;
	BCN_IE_LIST *ie_list = NULL;

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));

	if (!ie_list) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Alloc memory for ie_list fail!!!\n", __func__));
		return;
	}

	NdisZeroMemory((UCHAR *)ie_list, sizeof(BCN_IE_LIST));
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);

	if (VarIE == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate memory fail!!!\n", __func__));
		goto LabelErr;
	}

	pFrame = (PFRAME_802_11) Elem->Msg;
	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;

	if (PeerBeaconAndProbeRspSanity(pAd,
									Elem->Msg, Elem->MsgLen, Elem->Channel,
									ie_list, &LenVIE, pVIE, FALSE, FALSE)) {
		ULONG Idx;
		CHAR  Rssi = -127;
		MAC_TABLE_ENTRY *pEntry = NULL;
		UCHAR Channel;

		pEntry = MacTableLookup(pAd, ie_list->Addr2);/* Found the pEntry from Peer Bcn Content */
		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
							   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

		if (ie_list->Channel > 14)
			Channel = HcGetChannelByRf(pAd, RFIC_5GHZ);
		else
			Channel = HcGetChannelByRf(pAd, RFIC_24GHZ);

		{
			/* ignore BEACON not in this channel */
			if (ie_list->Channel != pAd->ScanCtrl.Channel
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
				&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			   )
				goto __End_Of_APPeerBeaconAtScanAction;
		}

#ifdef DOT11_N_SUPPORT

		if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) && (ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) { /* || (HtCapabilityLen == 0))) */
			if ((pAd->ScanCtrl.ScanType == SCAN_2040_BSS_COEXIST) &&
				(pAd->ApCfg.ScanReqwdev->wdev_type == WDEV_TYPE_APCLI))
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:Ignore BW 40->20\n", __func__));
			else
				Handle_BSS_Width_Trigger_Events(pAd, Channel);
		}

#endif /* DOT11_N_SUPPORT */
#ifdef IDS_SUPPORT

		/* Conflict SSID detection */
		if (ie_list->Channel == Channel)
			RTMPConflictSsidDetection(pAd, ie_list->Ssid, ie_list->SsidLen,
									  Elem->rssi_info.raw_rssi[0],
									  Elem->rssi_info.raw_rssi[1],
									  Elem->rssi_info.raw_rssi[2]);

#endif /* IDS_SUPPORT */
		/*
			This correct im-proper RSSI indication during SITE SURVEY issue.
			Always report bigger RSSI during SCANNING when receiving multiple BEACONs from the same AP.
			This case happens because BEACONs come from adjacent channels, so RSSI become weaker as we
			switch to more far away channels.
		*/
		Idx = BssTableSearch(&pAd->ScanTab, ie_list->Bssid, ie_list->Channel);

		if (Idx != BSS_NOT_FOUND && Idx < MAX_LEN_OF_BSS_TABLE)
			Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;

		/* TODO: 2005-03-04 dirty patch. we should change all RSSI related variables to SIGNED SHORT for easy/efficient reading and calaulation */
		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
							   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

		if ((RealRssi + pAd->BbpRssiToDbmDelta) > Rssi)
			Rssi = RealRssi + pAd->BbpRssiToDbmDelta;

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE);
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		{
			struct wifi_dev *wdev = (struct wifi_dev *)Elem->wdev;
			UCHAR BandIdx = HcGetBandByWdev(wdev);
			CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			UCHAR apcli2Gidx = 0;
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
				apcli2Gidx = 1;
#endif
			/* Check if this scan channel is the effeced channel */
			if (APCLI_IF_UP_CHECK(pAd, apcli2Gidx) &&
				(pAd->bApCliCertTest == TRUE) &&
				(pAd->CommonCfg.bBssCoexEnable == TRUE)
				&& ((ie_list->Channel > 0) && (ie_list->Channel <= 14))) {
				int chListIdx;

				/*
				First we find the channel list idx by the channel number
				*/
				for (chListIdx = 0; chListIdx < pChCtrl->ChListNum; chListIdx++) {
					if (ie_list->Channel == pChCtrl->ChList[chListIdx].Channel)
						break;
				}

				if (chListIdx < pChCtrl->ChListNum) {

					/*
						If this channel is effected channel for the 20/40 coex operation. Check the related IEs.
					*/
					if (pChCtrl->ChList[chListIdx].bEffectedChannel == TRUE) {
						UCHAR RegClass;
						OVERLAP_BSS_SCAN_IE BssScan;
						/* Read Beacon's Reg Class IE if any. */
						PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
						/* printk("\x1b[31m TriEventTableSetEntry \x1b[m\n"); */
						TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, ie_list->Bssid, &ie_list->HtCapability, ie_list->HtCapabilityLen, RegClass, ie_list->Channel);
					}
				}
			}
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */

		if (Idx != BSS_NOT_FOUND) {
			NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
		}

#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

		if (RTMPEqualMemory(ie_list->Ssid, "DIRECT-", 7))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s P2P_SCANNING: %s [%lu], channel =%d\n"
					 , __func__, ie_list->Ssid, Idx, Elem->Channel));

		/* Determine primary channel by IE's DSPS rather than channel of received frame */
		if (ie_list->Channel != 0)
			Elem->Channel = ie_list->Channel;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APPeerBeaconAtScanAction : Update the SSID %s in Kernel Table, Elem->Channel=%u\n", ie_list->Ssid, Elem->Channel));
		RT_CFG80211_SCANNING_INFORM(pAd, Idx, /*ie_list->Channel*/Elem->Channel, (UCHAR *)Elem->Msg, Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */
	}

	/* sanity check fail, ignored */
__End_Of_APPeerBeaconAtScanAction:
	/*scan beacon in pastive */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel) {
			UCHAR BandIdx = HcGetBandByWdev(Elem->wdev);
			AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

			if (AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, Elem->wdev) == BSS_NOT_FOUND)
				pAutoChCtrl->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;

			AutoChBssInsertEntry(pAd, ie_list->Bssid, (CHAR *)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi, Elem->wdev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
LabelErr:

	if (VarIE != NULL)
		os_free_mem(VarIE);

	if (ie_list != NULL)
		os_free_mem(ie_list);
}

/*
    ==========================================================================
    Description:
	MLME Cancel the SCAN req state machine procedure
    ==========================================================================
 */
VOID APScanCnclAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;

	RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	pAd->ScanCtrl.Channel = 0;
	ScanNextChannel(pAd, OPMODE_AP, pAd->ApCfg.ScanReqwdev);
}

/*
    ==========================================================================
    Description:
	if ChannelSel is false,
		AP scans channels and lists the information of channels.
	if ChannelSel is true,
		AP scans channels and selects an optimal channel.

    NOTE:
    ==========================================================================
*/
VOID ApSiteSurvey(
		PRTMP_ADAPTER	pAd,
		PNDIS_802_11_SSID	pSsid,
		UCHAR				ScanType,
		BOOLEAN				ChannelSel)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	struct wifi_dev *wdev = NULL;
	INT BssIdx;
	INT MaxNumBss = pAd->ApCfg.BssidNum;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		/*
		     Still scanning, ignore this scan.
		*/
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Scanning now!\n", __func__));
		return;
	}

	AsicDisableSync(pAd, HW_BSSID_0);

	/* Disable beacon tx for all BSS */
	for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

		if (wdev->bAllowBeaconing)
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_DISABLE_TX);
	}

	/* Don't clear the scan table if we are doing partial scan */
	if ((pAd->ScanCtrl.PartialScan.bScanning == TRUE && pAd->ScanCtrl.PartialScan.LastScanChannel == 0) ||
		pAd->ScanCtrl.PartialScan.bScanning == FALSE)
		BssTableInit(&pAd->ScanTab);

	pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;
	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
	RTMP_MLME_HANDLER(pAd);
}

VOID ApSiteSurvey_by_wdev(
		PRTMP_ADAPTER	pAd,
		PNDIS_802_11_SSID	pSsid,
		UCHAR				ScanType,
		BOOLEAN				ChannelSel,
		struct wifi_dev		*wdev)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
#ifdef CON_WPS
	UCHAR ifIdx;
#endif /*ifIdx*/
	struct wifi_dev *wdev_temp = NULL;
	INT BssIdx;
	INT MaxNumBss = pAd->ApCfg.BssidNum;
	INT BandIdx = HcGetBandByWdev(wdev);

	AsicDisableSync(pAd, HW_BSSID_0);

	/* Disable beacon tx for BSS with same band */
	for (BssIdx = 0; BssIdx < MaxNumBss; BssIdx++) {
		wdev_temp = &pAd->ApCfg.MBSSID[BssIdx].wdev;

		if (BandIdx != HcGetBandByWdev(wdev_temp))
			continue;

		if (wdev_temp->bAllowBeaconing)
			UpdateBeaconHandler(pAd, wdev_temp, BCN_UPDATE_DISABLE_TX);
	}

	/* Don't clear the scan table if we are doing partial scan */
#ifdef CON_WPS
	ifIdx = wdev->func_idx;

	if ((ifIdx < MAX_APCLI_NUM)
			&& (pAd->ApCfg.ConWpsApCliDisabled == FALSE)
			&& (pAd->ApCfg.ApCliTab[ifIdx].wdev.WscControl.conWscStatus & CON_WPS_STATUS_APCLI_RUNNING))
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\033[1;32m ApSiteSurvey_by_wdev don't need Init BSS table\033[0m\n"));
	else
#endif /*CON_WPS*/
		if ((pAd->ScanCtrl.PartialScan.bScanning == TRUE && pAd->ScanCtrl.PartialScan.LastScanChannel == 0) ||
			pAd->ScanCtrl.PartialScan.bScanning == FALSE) {
			#if defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT)
			if (pAd->CommonCfg.dbdc_mode == TRUE) {
				UCHAR Band = 0;
				if (WMODE_CAP_2G(wdev->PhyMode))
					Band = 0;
				if (WMODE_CAP_5G(wdev->PhyMode))
					Band = 1;

				/* backup the other band's scan result. */
				BssTableInitByBand(&pAd->ScanTab, Band);
			} else
			#endif /* defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT) */
				BssTableInit(&pAd->ScanTab);
		}

	pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;

	if ((pSsid) && (pSsid->SsidLength > 0) && (pSsid->SsidLength <= (NDIS_802_11_LENGTH_SSID))) {
		ScanReq.SsidLen = pSsid->SsidLength;
		NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}

	ScanReq.BssType = BSS_ANY;
	ScanReq.ScanType = ScanType;
	pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;
	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, (ULONG)wdev);
	RTMP_MLME_HANDLER(pAd);
}

BOOLEAN ApScanRunning(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	BOOLEAN	isScanOn = FALSE;
	UCHAR BandIdx = 0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	if (pwdev) {
		BandIdx = HcGetBandByWdev(pwdev);
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	if ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)
			|| (pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN))
			isScanOn = TRUE;
	else
			isScanOn = FALSE;
	} else {
		/* wdev null means check all of band */
		BandIdx = 0;
		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
			pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
			if ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)
				|| (pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState == AUTO_CH_SEL_SCAN_LISTEN))
				isScanOn |= TRUE;
		}
	}

	return isScanOn;
}
#endif /* AP_SCAN_SUPPORT */

/*
	==========================================================================
	Description:
		The sync state machine,
	Parameters:
		Sm - pointer to the state machine
	Note:
		the state machine looks like the following

							AP_SYNC_IDLE
	APMT2_PEER_PROBE_REQ	peer_probe_req_action
	==========================================================================
 */
VOID APSyncStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AP_MAX_SYNC_STATE, AP_MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, AP_SYNC_IDLE, AP_SYNC_MACHINE_BASE);
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)APPeerProbeReqAction);
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_BEACON, (STATE_MACHINE_FUNC)APPeerBeaconAction);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT || CFG80211_MULTI_STA */
#ifdef AP_SCAN_SUPPORT
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)APMlmeScanReqAction);
#ifdef CON_WPS
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_MLME_SCAN_COMPLETE, (STATE_MACHINE_FUNC)APMlmeScanCompleteAction);
#endif /* CON_WPS */
	/* scan_listen state */
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)APInvalidStateWhenScan);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_PEER_BEACON, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)APPeerBeaconAtScanAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)APScanTimeoutAction);
	StateMachineSetAction(Sm, AP_SCAN_LISTEN, APMT2_MLME_SCAN_CNCL, (STATE_MACHINE_FUNC)APScanCnclAction);
	RTMPInitTimer(pAd, &pAd->ScanCtrl.APScanTimer, GET_TIMER_FUNCTION(APScanTimeout), pAd, FALSE);
#endif /* AP_SCAN_SUPPORT */
}


VOID SupportRate(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN PUCHAR ExtRate,
	IN UCHAR ExtRateLen,
	OUT PUCHAR *ppRates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate)
{
	INT i;
	*pMaxSupportRate = 0;

	if ((SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES) && (SupRateLen > 0)) {
		NdisMoveMemory(*ppRates, SupRate, SupRateLen);
		*RatesLen = SupRateLen;
	} else {
		/* HT rate not ready yet. return true temporarily. rt2860c */
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\n")); */
		*RatesLen = 8;
		*(*ppRates + 0) = 0x82;
		*(*ppRates + 1) = 0x84;
		*(*ppRates + 2) = 0x8b;
		*(*ppRates + 3) = 0x96;
		*(*ppRates + 4) = 0x12;
		*(*ppRates + 5) = 0x24;
		*(*ppRates + 6) = 0x48;
		*(*ppRates + 7) = 0x6c;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SUPP_RATES., Len=%d\n", SupRateLen));
	}

	if (ExtRateLen + *RatesLen <= MAX_LEN_OF_SUPPORTED_RATES) {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, ExtRateLen);
		*RatesLen = (*RatesLen) + ExtRateLen;
	} else {
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, MAX_LEN_OF_SUPPORTED_RATES - (*RatesLen));
		*RatesLen = MAX_LEN_OF_SUPPORTED_RATES;
	}


	for (i = 0; i < *RatesLen; i++) {
		if (*pMaxSupportRate < (*(*ppRates + i) & 0x7f))
			*pMaxSupportRate = (*(*ppRates + i) & 0x7f);
	}
}

#ifdef DOT11_N_SUPPORT
void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE,
	IN UCHAR Channel,
	IN UCHAR PhyMode,
	IN struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	pIE->ID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	pIE->Length = 4;
	pIE->ChannelSwitchMode = 1;	/*no further frames */
	pIE->NewRegClass = get_regulatory_class(pAd, Channel, PhyMode, wdev);
	pIE->NewChannelNum = Channel;
	pIE->ChannelSwitchCount = (pDot11h->CSPeriod - pDot11h->CSCount - 1);
}
#endif /* DOT11_N_SUPPORT */

