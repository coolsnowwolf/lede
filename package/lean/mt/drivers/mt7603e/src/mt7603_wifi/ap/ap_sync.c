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

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_conn_perm_lock;
#endif
#endif
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
	IN MLME_QUEUE_ELEM *Elem)
{
	PEER_PROBE_REQ_PARAM ProbeReqParam;
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS NStatus;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0, TmpLen;
	LARGE_INTEGER FakeTimestamp;
	UCHAR DsLen = 1;
	UCHAR ErpIeLen = 1;
	UCHAR apidx = 0, PhyMode, SupRateLen;
	UCHAR RSNIe=IE_WPA, RSNIe2=IE_WPA2;
	BSS_STRUCT *mbss;
	struct wifi_dev *wdev;
	UCHAR idx = 0;
	UCHAR Environment = 0x20;
#ifdef WH_EZ_SETUP
	BOOLEAN ez_peer = FALSE;
#endif /* WH_EZ_SETUP */

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
DBGPRINT(RT_DEBUG_OFF, ("%s():shiang! PeerProbeReqSanity failed!\n", __FUNCTION__));
		return;
	}

	for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{
		mbss = &pAd->ApCfg.MBSSID[apidx];
		wdev = &mbss->wdev;
		RSNIe = IE_WPA;
	
		if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
			!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))))
		{
			/* the interface is down, so we can not send probe response */
			continue;
		}
#ifdef AIRPLAY_SUPPORT
		if (pAd->ApCfg.MBSSID[apidx].bcn_buf.bBcnSntReq == FALSE)
			continue;
#endif /* AIRPLAY_SUPPORT */
#ifdef WAPP_SUPPORT
		wapp_send_cli_probe_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), ProbeReqParam.Addr2, Elem);
#endif

		PhyMode = wdev->PhyMode;

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev)) {
			unsigned ez_probe_req_action = 0;
#ifdef EZ_ROAM_SUPPORT
			if (ez_is_roam_blocked_mac(wdev, ProbeReqParam.Addr2)){
			//if(MAC_ADDR_EQUAL(wdev->ez_security.ez_ap_roam_blocked_mac,ProbeReqParam.Addr2)){
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Connection not allowed as roaming ongoing.\n"));
				continue;
			}
#endif
			ez_probe_req_action = ez_process_probe_request(pAd, wdev, ProbeReqParam.Addr2, Elem->Msg, Elem->MsgLen);
			if (ez_probe_req_action == 1) {
				ez_peer = TRUE;
				EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("Easy Setup Peer - %02x:%02x:%02x:%02x:%02x:%02x\n", 
					ProbeReqParam.Addr2[0],
					ProbeReqParam.Addr2[1],
					ProbeReqParam.Addr2[2],
					ProbeReqParam.Addr2[3],
					ProbeReqParam.Addr2[4],
					ProbeReqParam.Addr2[5]));
			}
			else if (ez_probe_req_action == 2)
			{
					EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,(" Connection not allowd\n"));
					continue;
			} else
			{			
#ifdef WH_EZ_SETUP	
				//! if ez_setup is enabled on this wdev and and it is a triband repeater than this AP should accept only EZ connections		
				if(ez_is_triband() && IS_EZ_SETUP_ENABLED(wdev)) {
					if (!ez_check_for_ez_enable(wdev, Elem->Msg, Elem->MsgLen))
					{
					} else {
						continue;
					}
				}
#endif	
			//	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_process_probe_request returned false\n"));
			}
		}
		else{
		//	EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ez_process_probe_request not entered: %d %d\n", wdev->wdev_idx, wdev->enable_easy_setup));
		}
#endif /* WH_EZ_SETUP */

		if ( ((((ProbeReqParam.SsidLen == 0) && (!mbss->bHideSsid)) ||
			   ((ProbeReqParam.SsidLen == mbss->SsidLen) && NdisEqualMemory(ProbeReqParam.Ssid, mbss->Ssid, (ULONG) ProbeReqParam.SsidLen)))
#ifdef CONFIG_HOTSPOT
			   && ProbeReqforHSAP(pAd, apidx, &ProbeReqParam)
#endif
			 )
#ifdef WSC_AP_SUPPORT
            /* buffalo WPS testbed STA send ProbrRequest ssid length = 32 and ssid are not AP , but DA are AP. for WPS test send ProbeResponse */
			|| ((ProbeReqParam.SsidLen == 32) && MAC_ADDR_EQUAL(Addr3, wdev->bssid) && (mbss->bHideSsid == 0))
#endif /* WSC_AP_SUPPORT */
		)
		{
			;
		}
		else {
#ifdef WH_EZ_SETUP
			if (!ez_peer)
#endif /* WH_EZ_SETUP */			
				continue; /* check next BSS */
		}
#ifdef STA_FORCE_ROAM_SUPPORT
					// Enhancement: Block probe Response to a peer on acl list to avoid conenct attempts by peer.
					if (pAd->en_force_roam_supp &&
						! ApCheckFroamAccessControlList(pAd, ProbeReqParam.Addr2, apidx)) {
						DBGPRINT(RT_DEBUG_OFF,
									("ACL reject Peer - %02x:%02x:%02x:%02x:%02x:%02x, block Probe Response.\n",
									PRINT_MAC(ProbeReqParam.Addr2)));
						return;
					}
#endif

#ifdef WH_EVENT_NOTIFIER
        {
            EventHdlr pEventHdlrHook = NULL;
            pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_PROBE_REQ);
            if(pEventHdlrHook && wdev)
                pEventHdlrHook(pAd, wdev, &ProbeReqParam, Elem);
        }
#endif /* WH_EVENT_NOTIFIER */


#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering)
#ifdef WH_EZ_SETUP
			&& !((wdev != NULL) && (IS_EZ_SETUP_ENABLED(wdev)) && (ez_peer == TRUE))
#endif
		) {
			BOOLEAN bBndStrgCheck = TRUE;
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, ProbeReqParam.Addr2, Elem, &ProbeReqParam);
			if (bBndStrgCheck == FALSE)
				return;
		}
#endif /* BAND_STEERING */

		/* allocate and send out ProbeRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus != NDIS_STATUS_SUCCESS)
			return;

		MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP, 0, ProbeReqParam.Addr2, 
							wdev->if_addr, wdev->bssid);

		 if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
			RSNIe = IE_WPA;
		else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2)
			|| (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef DOT11_SAE_SUPPORT
			|| (wdev->AuthMode == Ndis802_11AuthModeWPA3PSK)
			|| (wdev->AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK)
#endif
#ifdef CONFIG_OWE_SUPPORT
			|| (wdev->AuthMode == Ndis802_11AuthModeOWE)
#endif
			)
			RSNIe = IE_WPA2;
#ifdef WAPI_SUPPORT
		else if ((wdev->AuthMode == Ndis802_11AuthModeWAICERT) || (wdev->AuthMode == Ndis802_11AuthModeWAIPSK))
			RSNIe = IE_WAPI;
#endif /* WAPI_SUPPORT */

		{
#ifdef DYNAMIC_RX_RATE_ADJ
		SupRateLen = mbss->SupRateLen;
#else
		SupRateLen = pAd->CommonCfg.SupRateLen;
#endif /* DYNAMIC_RX_RATE_ADJ */
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
#ifdef DYNAMIC_RX_RATE_ADJ
						SupRateLen, 				mbss->SupRate,
#else
						  SupRateLen,                 pAd->CommonCfg.SupRate,
#endif /* DYNAMIC_RX_RATE_ADJ */
						  1,                          &DsIe,
						  1,                          &DsLen,
						  1,                          &pAd->CommonCfg.Channel,
						  END_OF_ARGS);
		}

#ifdef DYNAMIC_RX_RATE_ADJ
		if ((mbss->ExtRateLen) && (PhyMode != WMODE_B))
#else
		if ((pAd->CommonCfg.ExtRateLen) && (PhyMode != WMODE_B))
#endif /* DYNAMIC_RX_RATE_ADJ */
		{
			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &ErpIe,
							  1,                        &ErpIeLen,
							  1,                        &pAd->ApCfg.ErpIeContent,
							  1,                        &ExtRateIe,
#ifdef DYNAMIC_RX_RATE_ADJ
								1,						&mbss->ExtRateLen,
								mbss->ExtRateLen,		mbss->ExtRate,
#else
							  1,                        &pAd->CommonCfg.ExtRateLen,
							  pAd->CommonCfg.ExtRateLen,    pAd->CommonCfg.ExtRate,
#endif /* DYNAMIC_RX_RATE_ADJ */
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

#ifdef A_BAND_SUPPORT
		/* add Channel switch announcement IE */
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			UCHAR CSAIe=IE_CHANNEL_SWITCH_ANNOUNCEMENT;
			UCHAR CSALen=3;
			UCHAR CSAMode=1;

			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &CSAIe,
							  1,                        &CSALen,
							  1,                        &CSAMode,
							  1,                        &pAd->CommonCfg.Channel,
							  1,                        &pAd->Dot11_H.CSCount,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif /* A_BAND_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable))
		{
			ULONG TmpLen;
			UCHAR	HtLen, AddHtLen/*, NewExtLen*/;
			HT_CAPABILITY_IE HtCapabilityTmp;
#ifdef RT_BIG_ENDIAN
			ADD_HT_INFO_IE	addHTInfoTmp;
#endif

/* YF@20120419: Fix IOT Issue with Atheros STA on Windows 7 When IEEE80211H flag turn on. */

			HtLen = sizeof(pAd->CommonCfg.HtCapability);
			AddHtLen = sizeof(pAd->CommonCfg.AddHTInfo);
			//NewExtLen = 1;
			/*New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
#ifndef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
#ifdef DYNAMIC_RX_RATE_ADJ
			NdisMoveMemory(HtCapabilityTmp.MCSSet, mbss->ExpectedSuppHTMCSSet, 4);
#endif /* DYNAMIC_RX_RATE_ADJ */
			HtCapabilityTmp.HtCapInfo.ChannelWidth = pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;

			MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
							  1,                                &HtCapIe,
							  1,                                &HtLen,
							 sizeof(HT_CAPABILITY_IE),          &HtCapabilityTmp,
							  1,                                &AddHtInfoIe,
							  1,                                &AddHtLen,
							 sizeof(ADD_HT_INFO_IE),          &pAd->CommonCfg.AddHTInfo,
							  END_OF_ARGS);
#else
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
#ifdef DYNAMIC_RX_RATE_ADJ
			NdisMoveMemory(HtCapabilityTmp.MCSSet, mbss->ExpectedSuppHTMCSSet, 4);
#endif /* DYNAMIC_RX_RATE_ADJ */
			HtCapabilityTmp.HtCapInfo.ChannelWidth = pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;
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

			NdisMoveMemory(&addHTInfoTmp, &pAd->CommonCfg.AddHTInfo, AddHtLen);
			*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
			*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

			MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                           &HtCapIe,
								1,                           &HtLen,
								HtLen,                       &HtCapabilityTmp,
								1,                           &AddHtInfoIe,
								1,                           &AddHtLen,
								AddHtLen,                    &addHTInfoTmp,
								END_OF_ARGS);

#endif
			FrameLen += TmpLen;
		}
#endif /* DOT11_N_SUPPORT */

		/* Append RSN_IE when  WPA OR WPAPSK, */
		if (wdev->AuthMode < Ndis802_11AuthModeWPA)
			; /* enough information */
		else if ((wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) ||
			(wdev->AuthMode == Ndis802_11AuthModeWPA1PSKWPA2PSK))
		{
			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &RSNIe,
							  1,                        &mbss->RSNIE_Len[0],
							  mbss->RSNIE_Len[0],  mbss->RSN_IE[0],
							  1,                        &RSNIe2,
							  1,                        &mbss->RSNIE_Len[1],
							  mbss->RSNIE_Len[1],  mbss->RSN_IE[1],
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
		else
		{
#ifdef CONFIG_HOTSPOT_R2
			PHOTSPOT_CTRL pHSCtrl =  &mbss->HotSpotCtrl;
			extern UCHAR		OSEN_IE[];
			extern UCHAR		OSEN_IELEN;

			if ((pHSCtrl->HotSpotEnable == 0) && (pHSCtrl->bASANEnable == 1) && (wdev->AuthMode == Ndis802_11AuthModeWPA2))
			{
				RSNIe = IE_WPA;
				MakeOutgoingFrame(pOutBuffer+FrameLen,        &TmpLen,
						  1,                            &RSNIe,
						  1,                            &OSEN_IELEN,
						  OSEN_IELEN,					OSEN_IE,
						  END_OF_ARGS);
			}
			else
#endif
			{						
				MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &RSNIe,
							  1,                        &mbss->RSNIE_Len[0],
							  mbss->RSNIE_Len[0],  mbss->RSN_IE[0],
							  END_OF_ARGS);
			}
			FrameLen += TmpLen;
		}

#ifdef CONFIG_HOTSPOT
		if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable)
		{
			ULONG TmpLen;
				
			/* Hotspot 2.0 Indication */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIELen, 
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HSIndicationIE, END_OF_ARGS);

			FrameLen += TmpLen;

			/* Interworking element */
			/*MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.InterWorkingIELen, 
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.InterWorkingIE, END_OF_ARGS); 

			FrameLen += TmpLen;
*/
			/* Advertisement Protocol element */
			/*MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.AdvertisementProtoIELen, 
							  pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.AdvertisementProtoIE, END_OF_ARGS); 
			FrameLen += TmpLen;
*/
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
		{
			ULONG TmpLen;
			EXT_CAP_INFO_ELEMENT extCapInfo;
			UCHAR extInfoLen = sizeof(EXT_CAP_INFO_ELEMENT);
#ifdef RT_BIG_ENDIAN
			PUCHAR pInfo;
#endif
			NdisZeroMemory(&extCapInfo, extInfoLen);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			/* P802.11n_D1.10, HT Information Exchange Support */
			if ((pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED) && (pAd->CommonCfg.Channel <= 14) &&
				(pAd->ApCfg.MBSSID[apidx].wdev.DesiredHtPhyInfo.bHtEnable) && 
				(pAd->CommonCfg.bBssCoexEnable == TRUE))
			{
				extCapInfo.BssCoexistMgmtSupport = 1;
			}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_DOT11V_WNM
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.ProxyARPEnable)
				extCapInfo.proxy_arp = 1;
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMBTMEnable)
				extCapInfo.BssTransitionManmt = 1;
#ifdef CONFIG_HOTSPOT_R2		
			if (pAd->ApCfg.MBSSID[apidx].WNMCtrl.WNMNotifyEnable)
				extCapInfo.wnm_notification= 1;
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QosMapEnable)
				extCapInfo.qosmap= 1;
#endif		
#endif

#ifdef CONFIG_HOTSPOT
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.HotSpotEnable)
				extCapInfo.interworking = 1;
#endif

#ifdef RT_BIG_ENDIAN
			pInfo = (PUCHAR)(&extCapInfo);
			*((UINT32*)(pInfo)) = cpu2le32(*((UINT32*)(pInfo)));
			*((UINT32*)(pInfo+4)) = cpu2le32(*((UINT32*)(pInfo+4)));
#endif
			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
								1, 			&ExtCapIe,
								1, 			&extInfoLen,
								extInfoLen, 	&extCapInfo,
								END_OF_ARGS);
				
			FrameLen += TmpLen;
		}

#ifdef AP_QLOAD_SUPPORT
		if (pAd->phy_ctrl.FlgQloadEnable != 0)
		{
#ifdef CONFIG_HOTSPOT_R2		
			if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 1)
				FrameLen += QBSS_LoadElementAppend_HSTEST(pAd, pOutBuffer+FrameLen, apidx);
			else if (pAd->ApCfg.MBSSID[apidx].HotSpotCtrl.QLoadTestEnable == 0)	
#endif		
			FrameLen += QBSS_LoadElementAppend(pAd, pOutBuffer+FrameLen);
		}
#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11U_INTERWORKING
		if (mbss->GASCtrl.b11U_enable) {
			ULONG TmpLen;
			/* Interworking element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  mbss->GASCtrl.InterWorkingIELen,
							  mbss->GASCtrl.InterWorkingIE, END_OF_ARGS);
			FrameLen += TmpLen;
			/* Advertisement Protocol element */
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
							  mbss->GASCtrl.AdvertisementProtoIELen,
							  mbss->GASCtrl.AdvertisementProtoIE, END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif/* DOT11U_INTERWORKING */

		/* add WMM IE here */
		if (mbss->wdev.bWmmCapable)
		{
			UCHAR i;
			UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};
			WmeParmIe[8] = pAd->ApCfg.BssEdcaParm.EdcaUpdateCount & 0x0f;
#ifdef UAPSD_SUPPORT
			UAPSD_MR_IE_FILL(WmeParmIe[8], &mbss->wdev.UapsdInfo);
#endif /* UAPSD_SUPPORT */
			for (i=QID_AC_BE; i<=QID_AC_VO; i++)
			{
				WmeParmIe[10+ (i*4)] = (i << 5) + /* b5-6 is ACI */
									   ((UCHAR)pAd->ApCfg.BssEdcaParm.bACM[i] << 4) +     /* b4 is ACM */
									   (pAd->ApCfg.BssEdcaParm.Aifsn[i] & 0x0f);		/* b0-3 is AIFSN */
				WmeParmIe[11+ (i*4)] = (pAd->ApCfg.BssEdcaParm.Cwmax[i] << 4) +	/* b5-8 is CWMAX */
									   (pAd->ApCfg.BssEdcaParm.Cwmin[i] & 0x0f);	/* b0-3 is CWMIN */
				WmeParmIe[12+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] & 0xff);        /* low byte of TXOP */
				WmeParmIe[13+ (i*4)] = (UCHAR)(pAd->ApCfg.BssEdcaParm.Txop[i] >> 8);          /* high byte of TXOP */
			}

			MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							  26,                       WmeParmIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}

	    /* add country IE, power constraint IE */
		if (pAd->CommonCfg.bCountryFlag)
		{
#ifndef EXT_BUILD_CHANNEL_LIST
			ULONG TmpLen; 
#endif  /* !EXT_BUILD_CHANNEL_LIST */
			ULONG TmpLen2=0;
			UCHAR *TmpFrame = NULL;

			os_alloc_mem(NULL, (UCHAR **)&TmpFrame, 256);
			if (TmpFrame != NULL)
			{
				NdisZeroMemory(TmpFrame, 256);

				/* prepare channel information */
#ifdef EXT_BUILD_CHANNEL_LIST
				BuildBeaconChList(pAd, TmpFrame, &TmpLen2);
#else
				{
					UCHAR MaxTxPower = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel);
					MakeOutgoingFrame(TmpFrame+TmpLen2,     &TmpLen,
										1,                 	&pAd->ChannelList[0].Channel,
										1,                 	&pAd->ChannelListNum,
										1,                 	&MaxTxPower,
										END_OF_ARGS);
					TmpLen2 += TmpLen;
				}
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef DOT11K_RRM_SUPPORT
				if (IS_RRM_ENABLE(pAd, apidx)
					&& (pAd->CommonCfg.RegulatoryClass[0] != 0))
				{
					TmpLen2 = 0;
					NdisZeroMemory(TmpFrame, 256);
					RguClass_BuildBcnChList(pAd, TmpFrame, &TmpLen2);
				}		
#endif /* DOT11K_RRM_SUPPORT */

				os_free_mem(NULL, TmpFrame);
			}
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		}
			
#ifdef DOT11K_RRM_SUPPORT
		if (IS_RRM_ENABLE(pAd, apidx))
		{
			InsertTpcReportIE(pAd, pOutBuffer+FrameLen, &FrameLen,
			RTMP_GetTxPwr(pAd, pAd->CommonCfg.MlmeTransmit), 0);
			RRM_InsertRRMEnCapIE(pAd, pOutBuffer+FrameLen, &FrameLen, apidx);
		}


		{
			INT loop;
			for (loop=0; loop<MAX_NUM_OF_REGULATORY_CLASS; loop++)
			{
				if (pAd->CommonCfg.RegulatoryClass[loop] == 0)
					break;
				InsertChannelRepIE(pAd, pOutBuffer+FrameLen, &FrameLen,
									(RTMP_STRING *)pAd->CommonCfg.CountryCode,
									pAd->CommonCfg.RegulatoryClass[loop], NULL);
			}
		}

#ifndef APPLE_11K_IOT
		/* Insert BSS AC Access Delay IE. */
		RRM_InsertBssACDelayIE(pAd, pOutBuffer+FrameLen, &FrameLen);

		/* Insert BSS Available Access Capacity IE. */
		RRM_InsertBssAvailableACIE(pAd, pOutBuffer+FrameLen, &FrameLen);
#endif /* !APPLE_11K_IOT */

#endif /* DOT11K_RRM_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	 	/* P802.11n_D3.03, 7.3.2.60 Overlapping BSS Scan Parameters IE */
	 	if (WMODE_CAP_N(PhyMode) &&
			(pAd->CommonCfg.Channel <= 14) &&
			(wdev->DesiredHtPhyInfo.bHtEnable) &&
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == 1))
	 	{
			OVERLAP_BSS_SCAN_IE  OverlapScanParam;
			ULONG	TmpLen;
			UCHAR	OverlapScanIE, ScanIELen;

			OverlapScanIE = IE_OVERLAPBSS_SCAN_PARM;
			ScanIELen = 14;
			OverlapScanParam.ScanPassiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveDwell);
			OverlapScanParam.ScanActiveDwell = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveDwell);
			OverlapScanParam.TriggerScanInt = cpu2le16(pAd->CommonCfg.Dot11BssWidthTriggerScanInt);
			OverlapScanParam.PassiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel);
			OverlapScanParam.ActiveTalPerChannel = cpu2le16(pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel);
			OverlapScanParam.DelayFactor = cpu2le16(pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
			OverlapScanParam.ScanActThre = cpu2le16(pAd->CommonCfg.Dot11OBssScanActivityThre);
			
			MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1,			&OverlapScanIE,
								1,			&ScanIELen,
								ScanIELen,	&OverlapScanParam,
								END_OF_ARGS);
			
			FrameLen += TmpLen;
	 	}
		
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	    /* add country IE, power constraint IE */
		if (pAd->CommonCfg.bCountryFlag)
		{
			ULONG TmpLen2=0;
			UCHAR TmpFrame[256] = {0};
			UCHAR CountryIe = IE_COUNTRY;
			UCHAR MaxTxPower=16;

#ifdef A_BAND_SUPPORT
			/* 
			Only 802.11a APs that comply with 802.11h are required to include
			a Power Constrint Element(IE=32) in beacons and probe response frames
			*/
			if (pAd->CommonCfg.Channel > 14 && pAd->CommonCfg.bIEEE80211H == TRUE)
			{
				/* prepare power constraint IE */
				MakeOutgoingFrame(pOutBuffer+FrameLen,    &TmpLen,
						3,                 	PowerConstraintIE,
						END_OF_ARGS);
						FrameLen += TmpLen;

#ifdef DOT11_VHT_AC
				if (WMODE_CAP_AC(PhyMode)) {
					ULONG TmpLen;
					UINT8 vht_txpwr_env_ie = IE_VHT_TXPWR_ENV;
					UINT8 ie_len;
					VHT_TXPWR_ENV_IE txpwr_env;

					ie_len = build_vht_txpwr_envelope(pAd, (UCHAR *)&txpwr_env);
					MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								1,							&vht_txpwr_env_ie,
								1,							&ie_len,
								ie_len,						&txpwr_env,
								END_OF_ARGS);
					FrameLen += TmpLen;
				}
#endif /* DOT11_VHT_AC */
			}
#endif /* A_BAND_SUPPORT */

			//NdisZeroMemory(TmpFrame, sizeof(TmpFrame));

			/* prepare channel information */
			MakeOutgoingFrame(TmpFrame+TmpLen2,     &TmpLen,
					1,                 	&pAd->ChannelList[0].Channel,
					1,                 	&pAd->ChannelListNum,
					1,                 	&MaxTxPower,
					END_OF_ARGS);
			TmpLen2 += TmpLen;
#ifdef MBO_SUPPORT
			if (IS_MBO_ENABLE(wdev))
				Environment = MBO_AP_USE_GLOBAL_OPERATING_CLASS;
#endif/* MBO_SUPPORT */
			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2%2) == 0)
			{
				UCHAR	TmpLen3 = TmpLen2+4;
				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2+1, TmpFrame,
					END_OF_ARGS);
			}
			else
			{
				UCHAR	TmpLen3 = TmpLen2+3;
				MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
					1, &CountryIe,
					1, &TmpLen3,
					1, &pAd->CommonCfg.CountryCode[0],
					1, &pAd->CommonCfg.CountryCode[1],
					1, &Environment,
					TmpLen2, TmpFrame,
					END_OF_ARGS);
			}
			FrameLen += TmpLen;
		}/* Country IE - */

#ifdef A_BAND_SUPPORT
		/* add Channel switch announcement IE */
		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			UCHAR CSAIe=IE_CHANNEL_SWITCH_ANNOUNCEMENT;
			UCHAR CSALen=3;
			UCHAR CSAMode=1;

			MakeOutgoingFrame(pOutBuffer+FrameLen,      &TmpLen,
							  1,                        &CSAIe,
							  1,                        &CSALen,
							  1,                        &CSAMode,
							  1,                        &pAd->CommonCfg.Channel,
							  1,                        &pAd->Dot11_H.CSCount,
							  END_OF_ARGS);
			FrameLen += TmpLen;
#ifdef DOT11_N_SUPPORT
   			if (pAd->CommonCfg.bExtChannelSwitchAnnouncement)
			{
				HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE HtExtChannelSwitchIe;

				build_ext_channel_switch_ie(pAd, &HtExtChannelSwitchIe);
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TmpLen,
								  sizeof(HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE),	&HtExtChannelSwitchIe,
								  END_OF_ARGS);
			}
#endif /* DOT11_N_SUPPORT */
			FrameLen += TmpLen;
		}
#endif /* A_BAND_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(PhyMode) &&
			(wdev->DesiredHtPhyInfo.bHtEnable))
		{
			ULONG TmpLen;
			UCHAR	HtLen, AddHtLen;/*, NewExtLen; */
#ifdef RT_BIG_ENDIAN
			HT_CAPABILITY_IE HtCapabilityTmp;
			ADD_HT_INFO_IE	addHTInfoTmp;
#endif
			HtLen = sizeof(pAd->CommonCfg.HtCapability);
			AddHtLen = sizeof(pAd->CommonCfg.AddHTInfo);

		if (pAd->bBroadComHT == TRUE)
		{
			UCHAR epigram_ie_len;
			UCHAR BROADCOM_HTC[4] = {0x0, 0x90, 0x4c, 0x33};
			UCHAR BROADCOM_AHTINFO[4] = {0x0, 0x90, 0x4c, 0x34};


			epigram_ie_len = HtLen + 4;
#ifndef RT_BIG_ENDIAN
			MakeOutgoingFrame(pOutBuffer + FrameLen,        &TmpLen,
						  1,                                &WpaIe,
							  1,                                &epigram_ie_len,
							  4,                                &BROADCOM_HTC[0],
							  HtLen,          					&pAd->CommonCfg.HtCapability,
							  END_OF_ARGS);
#else
				NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
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

				MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                               &WpaIe,
								1,                               &epigram_ie_len,
								4,                               &BROADCOM_HTC[0],
								HtLen,                           &HtCapabilityTmp,
								END_OF_ARGS);
#endif

				FrameLen += TmpLen;

				epigram_ie_len = AddHtLen + 4;
#ifndef RT_BIG_ENDIAN
				MakeOutgoingFrame(pOutBuffer + FrameLen,          &TmpLen,
								  1,                              &WpaIe,
								  1,                              &epigram_ie_len,
								  4,                              &BROADCOM_AHTINFO[0],
								  AddHtLen, 					  &pAd->CommonCfg.AddHTInfo,
								  END_OF_ARGS);
#else
				NdisMoveMemory(&addHTInfoTmp, &pAd->CommonCfg.AddHTInfo, AddHtLen);
				*(USHORT *)(&addHTInfoTmp.AddHtInfo2) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo2));
				*(USHORT *)(&addHTInfoTmp.AddHtInfo3) = SWAP16(*(USHORT *)(&addHTInfoTmp.AddHtInfo3));

				MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
								1,                               &WpaIe,
								1,                               &epigram_ie_len,
								4,                               &BROADCOM_AHTINFO[0],
								AddHtLen,                        &addHTInfoTmp,
							  END_OF_ARGS);
#endif

				FrameLen += TmpLen;
			}

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(PhyMode) &&
				(pAd->CommonCfg.Channel > 14)) {
				FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer+FrameLen), SUBTYPE_PROBE_RSP);
			}
#endif /* DOT11_VHT_AC */

		}
#endif /* DOT11_N_SUPPORT */


#ifdef WSC_AP_SUPPORT
		/* for windows 7 logo test */
		if ((mbss->WscControl.WscConfMode != WSC_DISABLE) &&
#ifdef DOT1X_SUPPORT
				(wdev->IEEE8021X == FALSE) && 
#endif /* DOT1X_SUPPORT */
				(wdev->WepStatus == Ndis802_11WEPEnabled))
		{
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
			MakeOutgoingFrame(pOutBuffer+FrameLen,        &TempLen1,
								7,                            PROVISION_SERVICE_IE,
								END_OF_ARGS);
			FrameLen += TempLen1;
	    }

        /* add Simple Config Information Element */
        if ((mbss->WscControl.WscConfMode > WSC_DISABLE) && (mbss->WscIEProbeResp.ValueLen))
        {
    		ULONG WscTmpLen = 0;
    		MakeOutgoingFrame(pOutBuffer+FrameLen,                                  &WscTmpLen,
    						  mbss->WscIEProbeResp.ValueLen,   mbss->WscIEProbeResp.Value,
                              END_OF_ARGS);
    		FrameLen += WscTmpLen;
        }
#endif /* WSC_AP_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
		/* The Mobility Domain information element (MDIE) is present in Probe-
		** Request frame when dot11FastBssTransitionEnable is set to true. */
		if (pAd->ApCfg.MBSSID[apidx].FtCfg.FtCapFlag.Dot11rFtEnable)
		{
			PFT_CFG pFtCfg = &pAd->ApCfg.MBSSID[apidx].FtCfg;
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
#ifdef WH_EZ_SETUP
        /*
                To prevent old device has trouble to parse MTK vendor IE,
                insert easy setup IE first.
        */
        if (IS_EZ_SETUP_ENABLED(wdev) && ez_peer) {
                FrameLen += ez_build_probe_response_ie(wdev, pOutBuffer+FrameLen);

#ifdef NEW_CONNECTION_ALGO

        if (!ez_update_connection_permission(pAd, wdev, EZ_DISALLOW_ALL_ALLOW_ME)) {
                        return;
                }
#endif
        }
        /*
                add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
                                                 Byte0.b3=1 for rssi-feedback
        */

//    FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen), SUBTYPE_PROBE_RSP);
#endif /* WH_EZ_SETUP */
#if defined(MWDS) || defined(CONFIG_OWE_SUPPORT)
	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen)
#ifdef CONFIG_OWE_SUPPORT
	, VIE_PROBE_RESP
#endif
#ifdef WH_EZ_SETUP
	,SUBTYPE_PROBE_RSP
#endif
	);
#else
	{
		ULONG TmpLen;
		UCHAR RalinkSpecificIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};

		if (pAd->CommonCfg.bAggregationCapable)
			RalinkSpecificIe[5] |= 0x1;
		if (pAd->CommonCfg.bPiggyBackCapable)
			RalinkSpecificIe[5] |= 0x2;
#ifdef DOT11_N_SUPPORT
		if (pAd->CommonCfg.bRdg)
			RalinkSpecificIe[5] |= 0x4;
#endif /* DOT11_N_SUPPORT */

#ifdef WH_EZ_SETUP
	/*
		To prevent old device has trouble to parse MTK vendor IE,
		insert easy setup IE first.
	*/
	if (IS_EZ_SETUP_ENABLED(wdev) && ez_peer) {
		FrameLen += ez_build_probe_response_ie(wdev, pOutBuffer+FrameLen);

#ifdef NEW_CONNECTION_ALGO

	if (!ez_update_connection_permission(pAd, wdev, EZ_DISALLOW_ALL_ALLOW_ME)) {
			return;
		}
#endif
	}

	/*
		add Ralink-specific IE here - Byte0.b0=1 for aggregation, Byte0.b1=1 for piggy-back
		                                 Byte0.b3=1 for rssi-feedback
	*/

	FrameLen += build_vendor_ie(pAd, wdev, (pOutBuffer + FrameLen)
#ifdef CONFIG_OWE_SUPPORT
	, VIE_PROBE_RESP
#endif
	, SUBTYPE_PROBE_RSP);
#endif /* WH_EZ_SETUP */

#ifdef RSSI_FEEDBACK
		if (ProbeReqParam.bRequestRssi == TRUE)
		{
		    MAC_TABLE_ENTRY *pEntry=NULL;

			DBGPRINT(RT_DEBUG_ERROR, ("SYNC - Send PROBE_RSP to %02x:%02x:%02x:%02x:%02x:%02x...\n",
										PRINT_MAC(Addr2)));
	    
			RalinkSpecificIe[5] |= 0x8;
			pEntry = MacTableLookup(pAd, Addr2);

			if (pEntry != NULL)
			{
				RalinkSpecificIe[6] = (UCHAR)pEntry->RssiSample.AvgRssi[0];
				RalinkSpecificIe[7] = (UCHAR)pEntry->RssiSample.AvgRssi[1];
				RalinkSpecificIe[8] = (UCHAR)pEntry->RssiSample.AvgRssi[2];
			}
		}
#endif /* RSSI_FEEDBACK */
		MakeOutgoingFrame(pOutBuffer+FrameLen, &TmpLen,
							9, RalinkSpecificIe,
							END_OF_ARGS);
		FrameLen += TmpLen;

	}
#endif /* MWDS */
#ifdef MBO_SUPPORT
	if (IS_MBO_ENABLE(wdev)) {
		MakeMboOceIE(pAd, wdev, pOutBuffer+FrameLen, &FrameLen, MBO_FRAME_TYPE_PROBE_RSP);
	}
#endif/* MBO_SUPPORT */
#ifdef AIRPLAY_SUPPORT
		if (AIRPLAY_ON(pAd))
		{ 
			ULONG	AirplayTmpLen = 0;		

			/*User user setting IE*/
			if (pAd->pAirplayIe && (pAd->AirplayIeLen != 0))
			{	
				//printk("AIRPLAY IE setting : MakeOutgoingFrame IeLen=%d\n",pAd->AirplayIeLen);
				//hex_dump("APPLE IE:", pAd->pAirplayIe , pAd->AirplayIeLen);
				MakeOutgoingFrame(pOutBuffer+FrameLen, &AirplayTmpLen,
									 pAd->AirplayIeLen, pAd->pAirplayIe,	 
										END_OF_ARGS);
				FrameLen += AirplayTmpLen;
			}
		}
#endif /* AIRPLAY_SUPPORT*/

	/* 802.11n 11.1.3.2.2 active scanning. sending probe response with MCS rate is */
		for (idx = 0; idx < mbss->ProbeRspTimes; idx++)
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

#ifdef WH_EZ_SETUP
	if(IS_EZ_SETUP_ENABLED(wdev) && (ez_peer)) {
		ez_prepare_security_key(wdev, ProbeReqParam.Addr2, TRUE);
	}
#endif /* WH_EZ_SETUP */	
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

#ifdef EZ_REGROUP_SUPPORT
regrp_ap_info_struct regrp_ap_info[2][EZ_MAX_DEVICE_SUPPORT];
#endif

typedef struct
{
	ULONG	count;
	UCHAR	bssid[MAC_ADDR_LEN];
} BSSIDENTRY;


VOID APPeerBeaconAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES], *pRates = NULL, RatesLen;
	BOOLEAN LegacyBssExist;
	CHAR RealRssi;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	UCHAR MaxSupportedRate = 0;
#ifdef APCLI_SUPPORT
	UINT ApCliIndex = 0;
#ifdef MT_MAC
        BOOLEAN ApCliWcid = FALSE;
#endif /* MT_MAC */
#endif /* APCLI_SUPPORT */
	BCN_IE_LIST *ie_list = NULL;
#ifdef MWDS
	ULONG BssIdx;
#endif


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate ie_list fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate VarIE fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;



	pRates = (PUCHAR)Rates;

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
								FALSE))
	{
#ifdef WH_EZ_SETUP
	int j;
	for (j = 0; j <  pAd->ApCfg.BssidNum; j++)
	{
		struct wifi_dev * ez_wdev = &pAd->ApCfg.MBSSID[j].wdev;
		if (IS_EZ_SETUP_ENABLED(ez_wdev)) {			
			ez_ap_peer_beacon_action(ez_wdev, ie_list->Addr2, ie_list->vendor_ie.ez_capability, &RealRssi, ie_list);
			//ez_ap_peer_beacon_action(ez_wdev, ie_list->Addr2);
#ifdef EZ_REGROUP_SUPPORT
			if(IS_REGRP_SUPP(ez_wdev) && pAd->CommonCfg.Channel == ie_list->Channel)
			{
				UINT8 band;
				//UINT8 mac[6]= {0x20,0xaa,0x4b,0x6a,0x36,0x63};
				if(pAd->CommonCfg.Channel <= 14)
					band =0;
				else
					band = 1;
				if((ie_list->vendor_ie.support_easy_setup == 1 && 
				ie_list->vendor_ie.open_group_id_len == ez_wdev->ez_driver_params.open_group_id_len
				&& NdisEqualMemory(ie_list->vendor_ie.open_group_id,ez_wdev->ez_driver_params.open_group_id,ie_list->vendor_ie.open_group_id_len))
				||
				(ie_list->vendor_ie.support_easy_setup == 0 &&
				SSID_EQUAL(pAd->ApCfg.MBSSID[j].Ssid,pAd->ApCfg.MBSSID[j].SsidLen, ie_list->Ssid, ie_list->SsidLen))
				)
				{
					p_regrp_ap_info_struct ap_info = NULL;

				//	printk("Beacon: Ch=%d,EZ: %d,Bssid: %02x:%02x:%02x:%02x:%02x:%02x\n ",ie_list->Channel,
				//						ie_list->vendor_ie.support_easy_setup, PRINT_MAC(ie_list->Bssid));

					ap_info = ez_find_regrp_ap_by_bssid(regrp_ap_info[band],ie_list->Bssid);
					if(ap_info == NULL)
					{
						ap_info = ez_add_regrp_ap(regrp_ap_info[band]);
						printk("Added Entry: %02x:%02x:%02x:%02x:%02x:%02x, RSSI: %d\n",PRINT_MAC(ie_list->Bssid),RealRssi);
					}
					if(ap_info != NULL)
					{
						ap_info->Non_MAN = !ie_list->vendor_ie.support_easy_setup;
						COPY_MAC_ADDR(ap_info->bssid,ie_list->Bssid);
						if(ap_info->Non_MAN == 0)
						{
							NdisCopyMemory(&ap_info->node_number,
								&ie_list->vendor_ie.beacon_info.node_number,
								sizeof(EZ_NODE_NUMBER));
							NdisCopyMemory(ap_info->nw_wt,
								ie_list->vendor_ie.beacon_info.network_weight,
								NETWORK_WEIGHT_LEN);
						}
						else
						{
							NdisZeroMemory(&ap_info->node_number,
								sizeof(EZ_NODE_NUMBER));
							NdisZeroMemory(ap_info->nw_wt,
								NETWORK_WEIGHT_LEN);
						}
						ap_info->last_rssi = RealRssi;
						ap_info->rx_cnt++;
						ap_info->rssi_sum += RealRssi;
						if(ap_info->rx_cnt <= 0)
						{
						   ap_info->rssi_sum = RealRssi;
						   ap_info->rx_cnt = 1;
						}
						ap_info->avg_rssi = (CHAR)( (ap_info->rssi_sum)/(INT32)(ap_info->rx_cnt));
												NdisGetSystemUpTime(&ap_info->last_rx_time);
						
					
					}
				}
			}
#endif

		}
	}

#endif
	

		/* ignore BEACON not in this channel */
		if (ie_list->Channel != pAd->CommonCfg.Channel
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			)
		{
			goto __End_Of_APPeerBeaconAction;
		}

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
		{
			Handle_BSS_Width_Trigger_Events(pAd);
		}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
		   )
		{
			if (pAd->CommonCfg.Channel<=14)
			{
				if (((pAd->CommonCfg.CentralChannel+2) != ie_list->Channel) &&
					((pAd->CommonCfg.CentralChannel-2) != ie_list->Channel))
				{
/*
					DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x is a legacy BSS (%d) \n",
								Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5], Channel));
*/
					goto __End_Of_APPeerBeaconAction;
				}
			}
			else
			{
				if (ie_list->Channel != pAd->CommonCfg.Channel)
				{
					//goto __End_Of_APPeerBeaconAction;
				}
			}
		}
#endif /* DOT11_N_SUPPORT */

                SupportRate(ie_list->SupRate, ie_list->SupRateLen, ie_list->ExtRate, ie_list->ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);
		
                if ((ie_list->Erp & 0x01) || (RatesLen <= 4))
			LegacyBssExist = TRUE;
		else
			LegacyBssExist = FALSE;

		if (LegacyBssExist && pAd->CommonCfg.DisableOLBCDetect == 0)
		{
			pAd->ApCfg.LastOLBCDetectTime = pAd->Mlme.Now32;
			
		}

#ifdef DOT11_N_SUPPORT
		if ((pAd->CommonCfg.bHTProtect)
			&& (ie_list->HtCapabilityLen == 0) && (RealRssi > OBSS_BEACON_RSSI_THRESHOLD))
		{
			
			pAd->ApCfg.LastNoneHTOLBCDetectTime = pAd->Mlme.Now32;
		}
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef MT_MAC
#ifdef MULTI_APCLI_SUPPORT
        if (Elem->Wcid == APCLI_MCAST_WCID(0) || Elem->Wcid == APCLI_MCAST_WCID(1))
        {
        	ApCliWcid = TRUE;
        }
#else /* MULTI_APCLI_SUPPORT */
        if (Elem->Wcid == APCLI_MCAST_WCID)
		    ApCliWcid = TRUE;
#endif /* !MULTI_APCLI_SUPPORT */
#endif
		if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE
#ifdef MT_MAC
			|| ApCliWcid
#endif
		)
		{
			PMAC_TABLE_ENTRY pEntry = NULL;
#ifdef MT_MAC
            if (ApCliWcid == FALSE)
#endif
			pEntry = &pAd->MacTab.Content[Elem->Wcid];
#ifdef MT_MAC
            else
                pEntry = MacTableLookup(pAd, ie_list->Addr2);//Found the pEntry from Peer Bcn Content
#endif /* MT_MAC */
			
			
			if (pEntry && IS_ENTRY_APCLI(pEntry) && (pEntry->func_tb_idx < MAX_APCLI_NUM))
			{
				//printk("%s: ===> beacon FINAL WCID[%d]\n", __FUNCTION__, Elem->Wcid);
				ApCliIndex = pEntry->func_tb_idx;	
				pAd->ApCfg.ApCliTab[ApCliIndex].ApCliRcvBeaconTime = pAd->Mlme.Now32;
#ifdef WH_EZ_SETUP
				if (IS_EZ_SETUP_ENABLED(pEntry->wdev)) {
#ifdef IF_UP_DOWN
					UINT8 bcn_config_same =
							ez_apcli_check_beacon_params(pEntry->wdev,
								ie_list->vendor_ie.beacon_info);

					if (pAd->ApCfg.ApCliTab[ApCliIndex].Valid == 1
						&& pEntry->easy_setup_enabled == 1
						&& bcn_config_same == FALSE) {
						pEntry->wdev->ez_driver_params.ez_apcli_peer_ap_config_cnt++;
						DBGPRINT(RT_DEBUG_ERROR,
							("APCLI:Beacon params of Peer not matching %d\n",
							pEntry->wdev->ez_driver_params.ez_apcli_peer_ap_config_cnt));

						if (pEntry->wdev->ez_driver_params.ez_apcli_peer_ap_config_cnt > 100)
							ez_apcli_disconnect_both_intf(pEntry->wdev);
						goto __End_Of_APPeerBeaconAction;
					}
					if (bcn_config_same == TRUE)
						pEntry->wdev->ez_driver_params.ez_apcli_peer_ap_config_cnt = 0;
#endif
				}
#endif

				if (pAd->CommonCfg.BBPCurrentBW == BW_40)
				{
					/* Check if root-ap change BW to 20 */
					if ((ie_list->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_NONE) &&
						(ie_list->AddHtInfo.AddHtInfo.RecomWidth == 0))
					{
#ifdef MAC_REPEATER_SUPPORT
						UINT ifIndex;
						UCHAR CliIdx;
						REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
						pEntry->HTPhyMode.field.BW = 0;
#ifdef MAC_REPEATER_SUPPORT
						ifIndex = pEntry->func_tb_idx;

						if (pAd->ApCfg.bMACRepeaterEn)
						{
							for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
							{
								pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];

								if ((pReptEntry->CliEnable) && (pReptEntry->CliValid))
								{
									pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
									if (pEntry)
										pEntry->HTPhyMode.field.BW = 0;
								}
							}
						}
#endif /* MAC_REPEATER_SUPPORT */
						DBGPRINT(RT_DEBUG_INFO, ("FallBack APClient BW to 20MHz\n"));
					}

					/* Check if root-ap change BW to 40 */
					if ((ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
						(ie_list->HtCapabilityLen > 0) &&
						(ie_list->HtCapability.HtCapInfo.ChannelWidth == 1))
					{
#ifdef MAC_REPEATER_SUPPORT
						UINT ifIndex;
						UCHAR CliIdx;
						REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
						pEntry->HTPhyMode.field.BW = 1;
#ifdef MAC_REPEATER_SUPPORT
						ifIndex = pEntry->func_tb_idx;

						if (pAd->ApCfg.bMACRepeaterEn)
						{
							for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
							{
								pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];

								if ((pReptEntry->CliEnable) && (pReptEntry->CliValid))
								{
									pEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
									if (pEntry)
										pEntry->HTPhyMode.field.BW = 1;
								}
							}
						}
#endif /* MAC_REPEATER_SUPPORT */
						DBGPRINT(RT_DEBUG_INFO, ("FallBack APClient BW to 40MHz\n"));
					}
				}
			}

			//ApCliWaitProbRsp(pAd, ApCliIndex);
			if ( /*ApCliWaitProbRsp(pAd, ApCliIndex) && */
		   	    (NdisEqualMemory(pAd->ApCfg.ApCliTab[ApCliIndex].CfgApCliBssid, ie_list->Bssid, MAC_ADDR_LEN)))
			{
				MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PEER_BEACON, Elem->MsgLen, Elem->Msg, ApCliIndex);
			}
		}
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
		do
		{
			MAC_TABLE_ENTRY *pEntry;
			BOOLEAN bWmmCapable;

			/* check BEACON does in WDS TABLE. */
			pEntry = WdsTableLookup(pAd, ie_list->Addr2, FALSE);
			bWmmCapable = ie_list->EdcaParm.bValid ? TRUE : FALSE;

			if (pEntry)
			{
				WdsPeerBeaconProc(pAd, pEntry, ie_list->CapabilityInfo,
								MaxSupportedRate, RatesLen, bWmmCapable,
								ie_list->RalinkIe, &ie_list->HtCapability,
								ie_list->HtCapabilityLen);
			}
		} while(FALSE);
#endif /* WDS_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if (pAd->CommonCfg.bOverlapScanning == TRUE)
		{
			INT		index,secChIdx;
			//BOOLEAN		found = FALSE;
			ADD_HTINFO *pAdd_HtInfo;
			
			for (index = 0; index < pAd->ChannelListNum; index++)
			{
				/* found the effected channel, mark that. */
				if(pAd->ChannelList[index].Channel == ie_list->Channel)
				{
					secChIdx = -1;
					if (ie_list->HtCapabilityLen > 0 && ie_list->AddHtInfoLen > 0)
					{	/* This is a 11n AP. */
						pAd->ChannelList[index].bEffectedChannel |= EFFECTED_CH_PRIMARY; /* 2; 	// 2 for 11N 20/40MHz AP with primary channel set as this channel. */
						pAdd_HtInfo = &ie_list->AddHtInfo.AddHtInfo;
						if (pAdd_HtInfo->ExtChanOffset == EXTCHA_BELOW)
						{
#ifdef A_BAND_SUPPORT						
							if (ie_list->Channel > 14)
								secChIdx = ((index > 0) ? (index - 1) : -1);
							else
#endif /* A_BAND_SUPPORT */								
								secChIdx = ((index >= 4) ? (index - 4) : -1);
						}
						else if (pAdd_HtInfo->ExtChanOffset == EXTCHA_ABOVE)
						{
#ifdef A_BAND_SUPPORT						
							if (ie_list->Channel > 14)
								secChIdx = (((index+1) < pAd->ChannelListNum) ? (index + 1) : -1);
							else
#endif /* A_BAND_SUPPORT */								
								secChIdx = (((index+4) < pAd->ChannelListNum) ? (index + 4) : -1);
						}

						if (secChIdx >=0)
							pAd->ChannelList[secChIdx].bEffectedChannel |= EFFECTED_CH_SECONDARY; /* 1; */

						if ((pAd->CommonCfg.Channel != ie_list->Channel) || 
							(pAdd_HtInfo->ExtChanOffset  != pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset)
						)
							pAd->CommonCfg.BssCoexApCnt++;
					}
					else
					{
						/* This is a legacy AP. */
						pAd->ChannelList[index].bEffectedChannel |=  EFFECTED_CH_LEGACY; /* 4; 1 for legacy AP. */
						pAd->CommonCfg.BssCoexApCnt++;
					}

					//found = TRUE;
				}
			}
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef MWDS
	BssIdx = BssTableSearch(&pAd->ScanTab, ie_list->Bssid, ie_list->Channel);
	if (BssIdx != BSS_NOT_FOUND)
	{
		pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS = FALSE;
		if(ie_list->vendor_ie.mtk_cap_found)
		{
			BOOLEAN bSupportMWDS = FALSE;
			if(ie_list->vendor_ie.support_mwds)
				bSupportMWDS = TRUE;
			if(pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS != bSupportMWDS)
				pAd->ScanTab.BssEntry[BssIdx].bSupportMWDS = bSupportMWDS;
		}
	}
#endif /* MWDS */
	}
	/* sanity check fail, ignore this frame */

__End_Of_APPeerBeaconAction:
/*#ifdef AUTO_CH_SELECT_ENHANCE */
#ifdef CONFIG_AP_SUPPORT
IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
{
	if (pAd->pChannelInfo != NULL)
	{
		if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel)
		{
			if (AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel) == BSS_NOT_FOUND)
				pAd->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;
			AutoChBssInsertEntry(pAd, ie_list->Bssid, ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi);
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */
/*#endif // AUTO_CH_SELECT_ENHANCE */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;
}

#ifdef AP_SCAN_SUPPORT
/*
    ==========================================================================
    Description:
    ==========================================================================
 */
VOID APInvalidStateWhenScan(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	DBGPRINT(RT_DEBUG_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.ApSyncMachine.CurrState));
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

	DBGPRINT(RT_DEBUG_TRACE, ("AP SYNC - Scan Timeout \n"));
	MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_SCAN_TIMEOUT, 0, NULL, 0);
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
#ifdef WH_EZ_SETUP
	ez_dev_t *ezdev = NULL;
	BOOLEAN b_internal_force_connect_bssid = FALSE;
#endif

#ifdef CONFIG_AP_SUPPORT
#endif
#ifdef AP_PARTIAL_SCAN_SUPPORT
	pAd->ScanCtrl.Channel = RTMPFindScanChannel(pAd, pAd->ScanCtrl.Channel);
#else
	pAd->ScanCtrl.Channel = NextChannel(pAd, pAd->ScanCtrl.Channel);
#endif /* AP_PARTIAL_SCAN_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
		/*
			iwpriv set auto channel selection
			update the current index of the channel
		*/
		if (pAd->ApCfg.bAutoChannelAtBootup == TRUE)
		{
			/* update current channel info */
			UpdateChannelInfo(pAd, pAd->ApCfg.current_channel_index, pAd->ApCfg.AutoChannelAlg);

			/* move to next channel */
			pAd->ApCfg.current_channel_index++;
			if (pAd->ApCfg.current_channel_index < pAd->ChannelListNum)
			{
				pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[pAd->ApCfg.current_channel_index].Channel;
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EZ_SETUP
	if (pAd->ApCfg.ScanReqwdev) {
		ezdev = (ez_dev_t *)pAd->ApCfg.ScanReqwdev->ez_driver_params.ezdev;
		if (ezdev)
			b_internal_force_connect_bssid = ezdev->ez_security.internal_force_connect_bssid;
	}

	if (IS_EZ_SETUP_ENABLED(pAd->ApCfg.ScanReqwdev) &&
	(pAd->ApCfg.ScanReqwdev->ez_driver_params.scan_one_channel ||
	b_internal_force_connect_bssid)) {
		pAd->ScanCtrl.Channel = 0;
		/*pAd->ScanCtrl.PartialScan.bScanning = FALSE;*/
	}
#endif /* WH_EZ_SETUP */

	ScanNextChannel(pAd, OPMODE_AP, pAd->ApCfg.ScanReqwdev);
}

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
#ifdef CONFIG_AP_SUPPORT
#endif
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Priv;
#ifdef WH_EZ_SETUP
	CHAR apcli_idx;
	MAC_TABLE_ENTRY *pMacEntry;
	struct wifi_dev *wdev;
#endif

	/* Suspend MSDU transmission here */
	RTMPSuspendMsduTransmission(pAd);

	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, Elem->Msg, Elem->MsgLen, &BssType, (PCHAR)Ssid, &SsidLen, &ScanType
						))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AP SYNC - MlmeScanReqAction\n"));
		NdisGetSystemUpTime(&pAd->ApCfg.LastScanTime);

		RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);

		/* record desired BSS parameters */
		pAd->ScanCtrl.BssType = BssType;
		pAd->ScanCtrl.ScanType = ScanType;
		pAd->ScanCtrl.SsidLen = SsidLen;
		NdisMoveMemory(pAd->ScanCtrl.Ssid, Ssid, SsidLen);
		/* only scan the channel which binding band supported */
		pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;


#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED((struct wifi_dev *)Elem->Priv)) 
		{
			pAd->ApCfg.ScanReqwdev = (struct wifi_dev *)Elem->Priv;
			apcli_idx = pAd->ApCfg.ScanReqwdev->func_idx;
#ifdef WSC_AP_SUPPORT
			if(!((pAd->ApCfg.ApCliTab[apcli_idx].WscControl.WscConfMode != WSC_DISABLE) 
				&& (pAd->ApCfg.ApCliTab[apcli_idx].WscControl.bWscTrigger == TRUE)))
#endif	
			{
				//ez_apcli_check_partial_scan(pAd, apcli_idx);
			}
		} else {
			pAd->ApCfg.ScanReqwdev = NULL;
			DBGPRINT(RT_DEBUG_TRACE, ("%s, ScanReqwdev is NULL!!!!\n", __func__));
		}
#endif
		
		{
		/* start from the first channel */
#ifdef AP_PARTIAL_SCAN_SUPPORT
			pAd->ScanCtrl.Channel = RTMPFindScanChannel(pAd, 0);
#else		
			pAd->ScanCtrl.Channel = FirstChannel(pAd);
#endif /* AP_PARTIAL_SCAN_SUPPORT */
		}

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED((struct wifi_dev *)Elem->Priv)) 
		{
			wdev = (struct wifi_dev *)Elem->Priv;
			apcli_idx = wdev->func_idx;
#ifdef WSC_AP_SUPPORT
			if(!((pAd->ApCfg.ApCliTab[apcli_idx].WscControl.WscConfMode != WSC_DISABLE) 
				&& (pAd->ApCfg.ApCliTab[apcli_idx].WscControl.bWscTrigger == TRUE)))
#endif	
			{
				if (IS_SINGLE_CHIP_DBDC(pAd))
				{
					if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
					{
					}
				}
			}
		}
		if (IS_EZ_SETUP_ENABLED(pAd->ApCfg.ScanReqwdev) 
			 /*&& (pAd->ApCfg.ScanReqwdev->ez_security.scan_one_channel || pAd->ApCfg.ScanReqwdev->ez_security.internal_force_connect_bssid)*/
			&& ez_ap_scan_complete_handle(pAd->ApCfg.ScanReqwdev)
			) {
			//pAd->ScanCtrl.Channel = pAd->ApCfg.ScanReqwdev->channel;
			if(pAd->CommonCfg.Channel != 0)
				pAd->ScanCtrl.Channel = pAd->CommonCfg.Channel;
			
			EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("AP SYNC - Only scan ch.%d and keep original BW setting.\n", pAd->ScanCtrl.Channel));
		}
		else
#endif /* WH_EZ_SETUP */
		{
			/* Let BBP register at 20MHz to do scan */
			bbp_set_bw(pAd, BW_20);
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup == TRUE)/* iwpriv set auto channel selection */
			{
				APAutoChannelInit(pAd);	
				pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[0].Channel;
			}
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef WH_EZ_SETUP		
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if (pAd->ApCfg.ScanReqwdev) {
			apcli_idx = pAd->ApCfg.ScanReqwdev->func_idx;
			pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[apcli_idx].wdev.bssid);
			if (pMacEntry && pAd->ApCfg.ApCliTab[apcli_idx].Valid
				&& (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev))) {
				pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
				ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, FALSE, pMacEntry, PWR_SAVE);
			}
		}
#endif	//APCLI_SUPPORT
#endif	//CONFIG_AP_SUPPORT
#endif	//WH_EZ_SETUP
		ScanNextChannel(pAd, OPMODE_AP, wdev);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AP SYNC - MlmeScanReqAction() sanity check fail. BUG!!!\n"));
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
	//PFRAME_802_11 pFrame;
	UCHAR *VarIE = NULL;
	USHORT LenVIE;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	CHAR RealRssi = -127;
#ifdef APCLI_SUPPORT
#ifdef WH_EVENT_NOTIFIER
	PFRAME_802_11 pFrame;
#endif
#endif

	BCN_IE_LIST *ie_list = NULL;
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
	UCHAR		Snr0 = Elem->Snr0;
	UCHAR		Snr1 = Elem->Snr1;
		
	Snr0 = ConvertToSnr(pAd, Snr0);
	Snr1 = ConvertToSnr(pAd, Snr1);
#endif

	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (!ie_list) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Alloc memory for ie_list fail!!!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory((UCHAR *)ie_list, sizeof(BCN_IE_LIST));

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}

#ifdef APCLI_SUPPORT
#ifdef WH_EVENT_NOTIFIER
	pFrame = (PFRAME_802_11) Elem->Msg;
#endif
#endif

	//pFrame = (PFRAME_802_11) Elem->Msg;
	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;


	if (PeerBeaconAndProbeRspSanity(pAd,
					Elem->Msg, Elem->MsgLen, Elem->Channel,
					ie_list, &LenVIE, pVIE, FALSE, FALSE))
    {
		ULONG Idx;
		CHAR  Rssi = -127;

		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));



#ifdef WH_EZ_SETUP
	if( !IS_ADPTR_EZ_SETUP_ENABLED(pAd) )
#endif
	{
		/* ignore BEACON not in this channel */
		if (ie_list->Channel != pAd->ScanCtrl.Channel
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			&& (pAd->CommonCfg.bOverlapScanning == FALSE)
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
		   )
		{
			goto __End_Of_APPeerBeaconAtScanAction;
		}
	}

#ifdef DOT11_N_SUPPORT
   		if ((RealRssi > OBSS_BEACON_RSSI_THRESHOLD) && (ie_list->HtCapability.HtCapInfo.Forty_Mhz_Intolerant)) /* || (HtCapabilityLen == 0))) */
		{
			Handle_BSS_Width_Trigger_Events(pAd);
		}
#endif /* DOT11_N_SUPPORT */

#ifdef IDS_SUPPORT
		/* Conflict SSID detection */
		if (ie_list->Channel == pAd->CommonCfg.Channel)
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
		if (Idx != BSS_NOT_FOUND)
        {
			UINT32 u4MaxBssEntrySize = sizeof(pAd->ScanTab.BssEntry)/sizeof(pAd->ScanTab.BssEntry[0]);
			if(Idx < u4MaxBssEntrySize)
            	Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): Over buffer size!\n", __FUNCTION__));
			}
		}

		

        /* TODO: 2005-03-04 dirty patch. we should change all RSSI related variables to SIGNED SHORT for easy/efficient reading and calaulation */
		RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0), 
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
								ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
        if ((RealRssi + pAd->BbpRssiToDbmDelta) > Rssi)
            Rssi = RealRssi + pAd->BbpRssiToDbmDelta;

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
							,
							Snr0,
							Snr1
#endif
							);
		if (Idx != BSS_NOT_FOUND)
		{
			NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
		}

#ifdef MWDS
		if (Idx != BSS_NOT_FOUND)
		{
			pAd->ScanTab.BssEntry[Idx].bSupportMWDS = FALSE;
			if(ie_list->vendor_ie.mtk_cap_found)
			{
				BOOLEAN bSupportMWDS = FALSE;
				if(ie_list->vendor_ie.support_mwds)
					bSupportMWDS = TRUE;
				if(pAd->ScanTab.BssEntry[Idx].bSupportMWDS != bSupportMWDS)
					pAd->ScanTab.BssEntry[Idx].bSupportMWDS = bSupportMWDS;
			}
		}
#endif /* MWDS */
#ifdef APCLI_SUPPORT
#ifdef WH_EVENT_NOTIFIER
		if(pFrame && (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP))
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_AP_PROBE_RSP);
			if(pEventHdlrHook && pAd->ApCfg.ScanReqwdev)
				pEventHdlrHook(pAd, pAd->ApCfg.ScanReqwdev, ie_list, Elem);
		}
#endif /* WH_EVENT_NOTIFIER */
#endif /* APCLI_SUPPORT */
		
	}

	/* sanity check fail, ignored */
__End_Of_APPeerBeaconAtScanAction:
	/*scan beacon in pastive */
#ifdef CONFIG_AP_SUPPORT
IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
{
	if (pAd->pChannelInfo != NULL)
	{
		if (ie_list->Channel == pAd->ApCfg.AutoChannel_Channel)
		{
			if (AutoChBssSearchWithSSID(pAd, ie_list->Bssid, (PUCHAR)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel) == BSS_NOT_FOUND)
				pAd->pChannelInfo->ApCnt[pAd->ApCfg.current_channel_index]++;
			AutoChBssInsertEntry(pAd, ie_list->Bssid, (CHAR *)ie_list->Ssid, ie_list->SsidLen, ie_list->Channel, ie_list->NewExtChannelOffset, RealRssi);   
		}
	}
}
#endif /* CONFIG_AP_SUPPORT */

LabelErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

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

	return;
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
	IN	PRTMP_ADAPTER  		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel,
	IN  struct wifi_dev		*wdev
)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;
	BOOLEAN bResetBssTable = TRUE;
#ifdef WH_EZ_SETUP
	struct wifi_dev* other_band_wdev;
	struct wifi_dev* other_band_wdev_ap;
#endif

#ifdef WH_EZ_SETUP
	
		if ((wdev != NULL) && (IS_EZ_SETUP_ENABLED(wdev)) && RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		{
			/*	
					 Still scanning, ignore this scan.
				*/
				EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Scanning now!\n", __FUNCTION__));
				return;
		}
		
		if ((wdev != NULL) && (IS_EZ_SETUP_ENABLED(wdev)) && (ScanRunning(pAd) == TRUE))
		{
			EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Scan Already Running! \n", __FUNCTION__));
			return;
		}
	
		if ((wdev != NULL) && IS_SINGLE_CHIP_DBDC(pAd) && (IS_EZ_SETUP_ENABLED(wdev)))
		{
			if (pAd->ApCfg.BssidNum == 2 && pAd->CommonCfg.dbdc_mode == 1)
			{
				other_band_wdev = &pAd->ApCfg.ApCliTab[wdev->func_idx ^ 1].wdev;
				other_band_wdev_ap = &pAd->ApCfg.MBSSID[wdev->func_idx ^ 1].wdev;
				if ((other_band_wdev->ez_driver_params.bPartialScanRunning == TRUE) || (other_band_wdev_ap->ez_driver_params.bPartialScanRunning == TRUE))
				{
					EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Partial Scan Already Running on other Band! \n", __FUNCTION__));
					return;
				}
			}
		}
			
		if((wdev != NULL) && IS_EZ_SETUP_ENABLED(wdev))
			increment_best_ap_rssi_threshold(&pAd->ApCfg.ApCliTab[wdev->func_idx].wdev);
#endif

    AsicDisableSync(pAd);
#ifdef AP_PARTIAL_SCAN_SUPPORT
	if (((pAd->ApCfg.bPartialScanning == TRUE) && (pAd->ApCfg.LastPartialScanChannel != 0)))
		bResetBssTable = FALSE;	
#endif /* AP_PARTIAL_SCAN_SUPPORT */
	if (bResetBssTable) {
    	BssTableInit(&pAd->ScanTab);
#ifdef NEIGHBORING_AP_STAT
		pAd->ScanTab.ScanResult.cnt = 0;
		RTMPZeroMemory(pAd->ScanTab.ScanResult.items, MAX_COUNT_OF_BSS_ENTRIES * sizeof(SCAN_RPT_ITEM));
#endif
	}
    pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;

	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;
	if (pSsid)
	{
	    ScanReq.SsidLen = pSsid->SsidLength;
	    NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}
    ScanReq.BssType = BSS_ANY;
    ScanReq.ScanType = ScanType;
    pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;
    
#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev))
		MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ,
			sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, (ULONG)wdev);
	else
#endif
		MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_MLME_SCAN_REQ,
			sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, (ULONG)wdev);
    RTMP_MLME_HANDLER(pAd);
}


BOOLEAN ApScanRunning(RTMP_ADAPTER *pAd)
{
	return (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) ? TRUE : FALSE;
}

#ifdef AP_PARTIAL_SCAN_SUPPORT
/* 
	==========================================================================
	Description:

	Return:
		scan_channel - channel to scan.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR FindPartialScanChannel(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR scan_channel = 0;
	if (pAd->ApCfg.PartialScanChannelNum > 0)
	{
		pAd->ApCfg.PartialScanChannelNum--;
		
		if (pAd->ApCfg.LastPartialScanChannel == 0)
			scan_channel = FirstChannel(pAd);
		else
			scan_channel = NextChannel(pAd, pAd->ApCfg.LastPartialScanChannel);
		
		/* update last scanned channel */
		pAd->ApCfg.LastPartialScanChannel = scan_channel;
		if (scan_channel == 0)
		{
			pAd->ApCfg.bPartialScanning = FALSE;
			pAd->ApCfg.PartialScanChannelNum = DEFLAUT_PARTIAL_SCAN_CH_NUM;
		}
	}
	else
	{
		/* Pending for next partial scan */
		scan_channel = 0;
		pAd->ApCfg.PartialScanChannelNum = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("%s, %u, scan_channel = %u, PartialScanChannelNum = %u, LastPartialScanChannel = %u, bPartialScanning = %u\n",
			__FUNCTION__, __LINE__,
			scan_channel,
			pAd->ApCfg.PartialScanChannelNum,
			pAd->ApCfg.LastPartialScanChannel,
			pAd->ApCfg.bPartialScanning));
	return scan_channel;
}
#endif /* AP_PARTIAL_SCAN_SUPPORT */
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
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AP_MAX_SYNC_STATE, AP_MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, AP_SYNC_IDLE, AP_SYNC_MACHINE_BASE);

	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)APPeerProbeReqAction);
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_PEER_BEACON, (STATE_MACHINE_FUNC)APPeerBeaconAction);
#ifdef AP_SCAN_SUPPORT
	StateMachineSetAction(Sm, AP_SYNC_IDLE, APMT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)APMlmeScanReqAction);

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

	if ((SupRateLen <= MAX_LEN_OF_SUPPORTED_RATES) && (SupRateLen > 0))
	{
		NdisMoveMemory(*ppRates, SupRate, SupRateLen);
		*RatesLen = SupRateLen;
	}
	else
	{
		/* HT rate not ready yet. return true temporarily. rt2860c */
		/*DBGPRINT(RT_DEBUG_TRACE, ("PeerAssocReqSanity - wrong IE_SUPP_RATES\n")); */
		*RatesLen = 8;
		*(*ppRates + 0) = 0x82;
		*(*ppRates + 1) = 0x84;
		*(*ppRates + 2) = 0x8b;
		*(*ppRates + 3) = 0x96;
		*(*ppRates + 4) = 0x12;
		*(*ppRates + 5) = 0x24;
		*(*ppRates + 6) = 0x48;
		*(*ppRates + 7) = 0x6c;
		DBGPRINT(RT_DEBUG_TRACE, ("SUPP_RATES., Len=%d\n", SupRateLen));
	}

	if (ExtRateLen + *RatesLen <= MAX_LEN_OF_SUPPORTED_RATES)
	{
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, ExtRateLen);
		*RatesLen = (*RatesLen) + ExtRateLen;
	}
	else
	{
		NdisMoveMemory((*ppRates + (ULONG)*RatesLen), ExtRate, MAX_LEN_OF_SUPPORTED_RATES - (*RatesLen));
		*RatesLen = MAX_LEN_OF_SUPPORTED_RATES;
	}

	

	for (i = 0; i < *RatesLen; i++)
	{
		if(*pMaxSupportRate < (*(*ppRates + i) & 0x7f))
			*pMaxSupportRate = (*(*ppRates + i) & 0x7f);
	}

	return;
}

#ifdef DOT11_N_SUPPORT
/* Regulatory classes in the USA */

void build_ext_channel_switch_ie(
	IN PRTMP_ADAPTER pAd,
	IN HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE *pIE)
{

	pIE->ID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	pIE->Length = 4;
	pIE->ChannelSwitchMode = 1;	/*no further frames */
	pIE->NewRegClass = get_regulatory_class(pAd, 0);
	pIE->NewChannelNum = pAd->CommonCfg.Channel;
    pIE->ChannelSwitchCount = (pAd->Dot11_H.CSPeriod - pAd->Dot11_H.CSCount - 1);
}
#endif /* DOT11_N_SUPPORT */
INT Set_Reg_Domain_Proc(
	IN	PRTMP_ADAPTER pAd,
	IN  RTMP_STRING *arg)
{
	pAd->reg_domain = simple_strtol(arg, 0, 10);
	/* 0: REG_LOCAL  1:REG_GLOBAL */
	MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	     ("reg_domain = %u\n", pAd->reg_domain));
	return TRUE;
}

