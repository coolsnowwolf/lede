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
   soft_ap.c

   Abstract:
   Access Point specific routines and MAC table maintenance routines

   Revision History:
   Who         When          What
   --------    ----------    ----------------------------------------------
   John Chang  08-04-2003    created for 11g soft-AP

*/

#include "rt_config.h"
#ifdef ONDEMAND_DFS
#include "wlan_config/config_internal.h"
#endif
#define VLAN_HDR_LEN	4
#ifdef VENDOR_FEATURE7_SUPPORT
#include "arris_wps_gpio_handler.h"
#endif
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif

#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */
#ifdef SPECIAL_11B_OBW_FEATURE
#define MAX_TXTD_CCK_CR 20
#endif /* SPECIAL_11B_OBW_FEATURE */
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
extern VOID vow_avg_pkt_len_reset(struct _RTMP_ADAPTER *ad);
extern VOID vow_avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry);
#endif
#ifdef IAPP_SUPPORT
BOOLEAN IAPP_L2_Update_Frame_Send(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);
#endif /*IAPP_SUPPORT*/
static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad);
char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated",
	"active countermeasures",
	"has disassociated with invalid PSK password"
};
UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}

static INT ap_mlme_set_capability(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN SpectrumMgmt = FALSE;

#ifdef A_BAND_SUPPORT

	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if ((wdev->channel > 14) && pAd->CommonCfg.bIEEE80211H == TRUE)
		SpectrumMgmt = TRUE;

#endif /* A_BAND_SUPPORT */

	pMbss->CapabilityInfo = CAP_GENERATE(1,
										 0,
										 IS_SECURITY_Entry(wdev),
										 (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1),
										 pAd->CommonCfg.bUseShortSlotTime,
										 SpectrumMgmt);
#ifdef DOT11K_RRM_SUPPORT

	if (IS_RRM_ENABLE(wdev))
		pMbss->CapabilityInfo |= RRM_CAP_BIT;

#endif /* DOT11K_RRM_SUPPORT */

	if (pMbss->wdev.bWmmCapable == TRUE) {
		/*
		    In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
		    bit in the capability field of association, beacon and probe
		    management frames.
		*/
		/*          pMbss->CapabilityInfo |= 0x0200; */
	}

#ifdef UAPSD_SUPPORT

	if (pMbss->wdev.UapsdInfo.bAPSDCapable == TRUE) {
		/*
		    QAPs set the APSD subfield to 1 within the Capability
		    Information field when the MIB attribute
		    dot11APSDOptionImplemented is true and set it to 0 otherwise.
		    STAs always set this subfield to 0.
		*/
		pMbss->CapabilityInfo |= 0x0800;
	}

#endif /* UAPSD_SUPPORT */
	return TRUE;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
BOOLEAN ApAutoChannelSkipListBuild(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	UCHAR chGrp = 0;
	BOOLEAN status = FALSE;

	SET_V10_OFF_CHNL_TIME(pAd, V10_NORMAL_SCAN_TIME);

	if (pAd->ApCfg.bAutoChannelAtBootup) {
		/* ACS Enable */
		if (wdev->channel != 0) {
			/* Non-Zero Channel in ACS */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] Non-Zero Chnl in ACS\n", __func__));
			return status;
		} else {
			AutoChannelSkipListClear(pAd);
			if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
				AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else
				return FALSE;
		}
	} else {
		/* ACS Disable */
		if (wdev->channel == 0) {
			/* No Channel in Non-ACS */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] Zero Channel in Non ACS %d\n", __func__, wdev->channel));
			return status;
		} else {
			/* Background ACS Algorithm = 3 */
			pAd->ApCfg.AutoChannelAlg = ChannelAlgBusyTime;

			if (IS_V10_W56_GRP_VALID(pAd))
				chGrp = W56_UA;
			else if (IS_V10_W56_VHT80_SWITCHED(pAd))
				chGrp = W56_UC;
			else
				chGrp = DfsV10CheckChnlGrp(wdev->channel);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("[%s] CHgrp %d Channel %d\n", __func__, chGrp, wdev->channel));

			/* Clean Skip List */
			AutoChannelSkipListClear(pAd);

			if (chGrp == NA_GRP) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] Illegal Group Number \n", __func__));
				return status;
			} else if (chGrp == W52) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] W52\n", __func__));
				AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
				status = TRUE;
			} else if (chGrp == W53) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] W53\n", __func__));

				if (DfsV10CheckGrpChnlLeft(pAd, W53, V10_W53_SIZE))
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
				else {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] W53 Channel Finish \n", __func__));
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
				}

				AutoChannelSkipChannels(pAd, V10_W56_SIZE, GROUP3_LOWER);
				AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);

				status = TRUE;
			} else if (chGrp == W56_UA || chGrp == W56_UB || chGrp == W56_UC) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] W56\n", __func__));
				SET_V10_W56_GRP_VALID(pAd, FALSE);

				if (wlan_config_get_vht_bw(wdev) == VHT_BW_2040) {
					if (IS_V10_W56_VHT80_SWITCHED(pAd) && chGrp == W56_UC)
						AutoChannelSkipChannels(pAd, V10_W56_VHT80_SIZE, GROUP3_LOWER);

					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				} else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
					if (chGrp == W56_UC) {
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("[%s]Incorrect Channel W56 C\n", __func__));
						return status;
					}

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("[%s] W56_80\n", __func__));
					AutoChannelSkipChannels(pAd, V10_W52_SIZE, GROUP1_LOWER);
					AutoChannelSkipChannels(pAd, V10_W53_SIZE, GROUP2_LOWER);
					AutoChannelSkipChannels(pAd, V10_W56_VHT20_SIZE, GROUP5_LOWER);
					AutoChannelSkipChannels(pAd, V10_LAST_SIZE, GROUP6_LOWER);
					status = TRUE;
				}
			}
		}
	}

	return status;
}
#endif

UCHAR ApAutoChannelAtBootUp(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#define SINGLE_BAND 0
#define DUAL_BAND   1

	UCHAR NewChannel;
	BOOLEAN IsAband;
	UCHAR BandIdx = BAND0;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;
#ifdef TR181_SUPPORT
	struct hdev_obj *hdev = (struct hdev_obj *)wdev->pHObj;
#endif
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s----------------->\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: AutoChannelBootup = %d\n",
			 __func__, pAd->ApCfg.bAutoChannelAtBootup));

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE) && (IS_V10_APINTF_DOWN(pAd) == FALSE)) {
		if (ApAutoChannelSkipListBuild (pAd, wdev) == FALSE) {
			return FALSE;
		}
	} else if (!pAd->ApCfg.bAutoChannelAtBootup) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __func__));
		return FALSE;
	}
#else
	if (!pAd->ApCfg.bAutoChannelAtBootup) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __func__));
		return FALSE;
	}
#endif

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\x1b[41m %s(): wdev is NULL !!\x1b[m\n", __func__));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __func__));
		return FALSE;
	}

	BandIdx = HcGetBandByWdev(wdev);
	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	/* Now Enable RxTx*/
	RTMPEnableRxTx(pAd);

	if ((wdev->channel == 0)
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		|| (IS_SUPPORT_V10_DFS(pAd) && (IS_V10_BOOTACS_INVALID(pAd) == FALSE))
#endif
#ifdef ONDEMAND_DFS
		|| (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) && (RadarChannelCheck(pAd, wdev->channel)))
		/*Invoke ACS IF ONDEMAND DFS & USER Selected DFS CHANNEL*/
#endif
	) {
#ifdef ONDEMAND_DFS
RetriggerACS:
#endif
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: PhyMode: %d\n", __func__, wdev->PhyMode));

		if (WMODE_CAP_5G(wdev->PhyMode))
				IsAband = TRUE;
		else
			IsAband = FALSE;

				/* Now we can receive the beacon and do ACS */
		if (pAutoChCtrl->AutoChSelCtrl.ACSChStat != ACS_CH_STATE_SELECTED) {
#ifdef ONDEMAND_DFS
			if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) &&
				RadarChannelCheck(pAd, wdev->channel)) {
				/* Pre-Selected Channel/ACS Channel is DFS: Skip DFS ACS next time */
				pAd->BgndScanCtrl.SkipDfsChannel = TRUE;
				/* Store Current DFS Channel as Outband Channel */
				pAd->CommonCfg.DfsParameter.OutBandCh = wdev->channel;
			}
#endif
			/* Disable MibBucket during doing ACS */
			pAd->MsMibBucket.Enabled = FALSE;
			pAd->OneSecMibBucket.Enabled[BandIdx] = FALSE;

			NewChannel = APAutoSelectChannel(pAd, wdev, pAd->ApCfg.AutoChannelAlg, IsAband);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s : Auto channel selection: Selected channel = %d, IsAband = %d\n", __func__, NewChannel, IsAband));

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
			if (IS_SUPPORT_V10_DFS(pAd) && NewChannel) {
				SET_V10_BOOTACS_INVALID(pAd, TRUE);
				DfsV10ACSMarkChnlConsumed(pAd, NewChannel);
			}
#endif
		} else {
#ifdef ONDEMAND_DFS
			/* Pick InBand Channel for OnDemand DFS */
			if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
				NewChannel = pAd->CommonCfg.DfsParameter.Band0Ch;
			else
#endif
			NewChannel = pAutoChCtrl->AutoChSelCtrl.SelCh;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("[ApAutoChannelAtBootUp] ACS of BandIdx = %d is already DONE, Channel = %d\n", BandIdx, NewChannel));
		}

#ifdef ONDEMAND_DFS
		if ((IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
			&& (RadarChannelCheck(pAd, NewChannel))
			&& (!GET_BGND_STATE(pAd, BGND_ONDMND_CNLSWITCH_ON))) {
			/* ACS returned DFS Channel as Output */
			struct wlan_config *cfg = NULL;
			UCHAR phy_bw = 0;

			cfg = (struct wlan_config *)wdev->wpf_cfg;
			if (cfg == NULL)
				return FALSE;

			if (cfg->ht_conf.ht_bw == HT_BW_20)
				phy_bw = BW_20;
			else if (cfg->ht_conf.ht_bw == HT_BW_40) {
				if (cfg->vht_conf.vht_bw == VHT_BW_2040)
					phy_bw = BW_40;
				else if (cfg->vht_conf.vht_bw == VHT_BW_80)
					phy_bw = BW_80;
				else if (cfg->vht_conf.vht_bw == VHT_BW_160)
					phy_bw = BW_160;
				else if (cfg->vht_conf.vht_bw == VHT_BW_8080)
					phy_bw = BW_8080;
			}

			/* Record Original DFS Channel as Outband Channel */
			pAd->CommonCfg.DfsParameter.OutBandCh = NewChannel;
			pAd->CommonCfg.DfsParameter.OutBandBw = phy_bw;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s : Outband channel = %d,BW = %d\n", __func__, NewChannel, phy_bw));
			/* Retrigger ACS as DFS Channel is selected at Bootup */
			pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
			pAd->BgndScanCtrl.SkipDfsChannel = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Retrigger ACS\n", __func__));
			/* Retrigger ACS to find Non-DFS Channel */
			goto RetriggerACS;
		}

		/* Clear Non-DFS Channel Selection Flag */
		if ((IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
			&& (pAd->BgndScanCtrl.SkipDfsChannel)) {
			pAd->BgndScanCtrl.SkipDfsChannel = FALSE;
		}
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("[%s] Chnl Restore Enbl %d\n", __func__, IS_V10_OLD_CHNL_VALID(wdev)));
		if (!IS_V10_OLD_CHNL_VALID(wdev))
#endif
			/* Update channel of wdev as new channel */
			AutoChSelUpdateChannel(pAd, NewChannel, IsAband, wdev);

#ifdef MT_DFS_SUPPORT
		/* Check new channel is DFS channel or not */
		RadarStateCheck(pAd, wdev);
#endif
		if ((pAd->ApCfg.bAutoChannelAtBootup == TRUE) && (vht_bw == VHT_BW_8080)) {
			wlan_config_set_cen_ch_2(wdev, wdev->auto_channel_cen_ch_2);
			wlan_operate_set_cen_ch_2(wdev, wdev->auto_channel_cen_ch_2);
		}

		/* Update primay channel */
		wlan_operate_set_prim_ch(wdev, wdev->channel);
#ifdef TR181_SUPPORT
		pAd->ApBootACSChannelChangePerBandCount[BandIdx]++;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Boot:%d Total:%d\n",
				__func__, pAd->ApBootACSChannelChangePerBandCount[BandIdx],
				(hdev->rdev->pRadioCtrl->TotalChannelChangeCount +
					pAd->ApBootACSChannelChangePerBandCount[BandIdx])));
#endif

		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled[BandIdx] = TRUE;
#ifdef MT_DFS_SUPPORT
		DfsBuildChannelList(pAd, wdev);
#endif
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s<-----------------\n", __func__));
	return TRUE;
}


/*
	==========================================================================
	Description:
	Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
static BOOLEAN ApCheckLongPreambleSTA(RTMP_ADAPTER *pAd)
{
	UCHAR   i;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo)) {
			return TRUE;
		}
	}

	return FALSE;
}

/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */

static VOID ap_run_at_boot(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_INIT_RADIO_ONOFF
	if(pAd->ApCfg.bRadioOn)
#endif
		if (ApAutoChannelAtBootUp(pAd, wdev) != TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("\x1b[41m %s() : ACS is disable !!\x1b[m\n", __func__));
	}
#ifdef ONDEMAND_DFS
	else {
		if ((IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
			&& (RadarChannelCheck(pAd, pAd->CommonCfg.DfsParameter.OutBandCh))
			&& (!RadarChannelCheck(pAd, pAd->CommonCfg.DfsParameter.Band0Ch))) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[41m %s() : OutBand Chnl %d Bw %d\x1b[m\n",
				__func__, pAd->CommonCfg.DfsParameter.OutBandCh,
				pAd->CommonCfg.DfsParameter.OutBandBw));
			/* Restore to Default Bgnd State */
			pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
			/* Initiate OnDemand DFS Zero-Wait State Machine */
			DfsDedicatedOutBandSetChannel(pAd, pAd->CommonCfg.DfsParameter.OutBandCh,
				pAd->CommonCfg.DfsParameter.Bw);
		}
	}
#endif

#ifdef BACKGROUND_SCAN_SUPPORT
	{
#ifdef DOT11_VHT_AC
		UCHAR vht_bw = wlan_operate_get_vht_bw(wdev);
#endif

		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			pAd->BgndScanCtrl.BgndScanSupport = 0;
			pAd->BgndScanCtrl.DfsZeroWaitSupport = 0;
		}

#ifdef DOT11_VHT_AC
		else if (vht_bw == VHT_BW_160 || vht_bw == VHT_BW_8080) {
			pAd->BgndScanCtrl.BgndScanSupport = 0;
			pAd->BgndScanCtrl.DfsZeroWaitSupport = 0;
		}

#endif /* DOT11_VHT_AC */
		else
			pAd->BgndScanCtrl.BgndScanSupport = 1;

#ifdef MT_DFS_SUPPORT
		/* Update bInitMbssZeroWait for MBSS Zero Wait */
		UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);
#endif /* MT_DFS_SUPPORT */
	}
#endif /* BACKGROUND_SCAN_SUPPORT */
}



NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> APOneShotSettingInitialize\n"));
	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
	MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
	AutoChBssTableInit(pAd);
	ChannelInfoInit(pAd);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerInit(pAd);
#endif /* DOT11N_DRAFT3 */
#endif /*DOT11_N_SUPPORT*/

	/* beacon init for USB/SDIO */
	asic_bss_beacon_init(pAd);

#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	/*
	    Some modules init must be called before APStartUp().
	    Or APStartUp() will make up beacon content and call
	    other modules API to get some information to fill.
	*/
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
#ifdef VIDEO_TURBINE_SUPPORT
		VideoTurbineDynamicTune(pAd);
#endif /* VIDEO_TURBINE_SUPPORT */
	}

#endif /* RTMP_RBUS_SUPPORT */
#ifdef MAT_SUPPORT
	MATEngineInit(pAd);
#endif /* MAT_SUPPORT */
#ifdef CLIENT_WDS
	CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- APOneShotSettingInitialize\n"));
	return Status;
}


/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("---> APShutdown\n"));
#ifdef MT_MAC
	/*	Disable RX */
	/* MtAsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE,0); */
#ifdef RTMP_MAC_PCI
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* RTMP_MAC_PCI */
	/* MlmeRadioOff(pAd); */
#else
	MlmeRadioOff(pAd);
#ifdef RTMP_MAC_PCI
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* RTMP_MAC_PCI */
#endif
#ifdef INTELP6_SUPPORT
	pAd->pre_n9_heart_beat_cnt = 0;
	pAd->pre_cr4_heart_beat_cnt = 0;
#endif
	/*remove sw related timer and table*/
	rtmp_ap_exit(pAd);
#ifdef FTM_SUPPORT
	FtmMgmtExit(pAd);
#endif /* FTM_SUPPORT */
	NdisFreeSpinLock(&pAd->MacTabLock);
#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTab.WdsTabLock);
#endif /* WDS_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--- APShutdown\n"));
}



/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */

VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd, struct _BSS_STRUCT *mbss)
{
	UCHAR  i, ErpIeContent = 0;
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;
	BOOLEAN bUseBGProtection;
	BOOLEAN LegacyBssExist;
	MAC_TABLE_ENTRY *pEntry = NULL;
	USHORT *pCapInfo = NULL;
	struct wifi_dev *wdev = &mbss->wdev;
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	UCHAR Channel = wdev->channel;
	UCHAR PhyMode = wdev->PhyMode;

	if (WMODE_EQUAL(PhyMode, WMODE_B))
		return;

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		if (wdev != pEntry->wdev)
			continue;

		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
			ShortSlotCapable = FALSE;
			ErpIeContent |= 0x01;
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = FALSE;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32)
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;

	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
	   AUTO (0): UseProtection = 1 if any 11b STA associated
	   ON (1): always USE protection
	   OFF (2): always NOT USE protection
	   */
	if (pAd->CommonCfg.UseBGProtection == 0) {
		ErpIeContent = (ErpIeContent) ? 0x03 : 0x00;

		/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
		if (LegacyBssExist) {
			ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
		}
	} else if (pAd->CommonCfg.UseBGProtection == 1)
		ErpIeContent |= 0x02;

	bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
					   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));
#ifdef A_BAND_SUPPORT

	/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
	if (Channel > 14)
		bUseBGProtection = FALSE;

#endif /* A_BAND_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("-- bUseBGProtection: %s, BG_PROTECT_INUSED: %s, ERP IE Content: 0x%x\n",
			  (bUseBGProtection) ? "Yes" : "No",
			  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) ? "Yes" : "No",
			  ErpIeContent));

	if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED)) {
		USHORT OperationMode = 0;
		BOOLEAN	bNonGFExist = 0;
#ifdef DOT11_N_SUPPORT
		OperationMode = addht->AddHtInfo2.OperaionMode;
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;
#endif /* DOT11_N_SUPPORT */

		if (bUseBGProtection)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
		else
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);

	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
		pAd->ApCfg.ErpIeContent = (ErpIeContent | 0x04);
	else
		pAd->ApCfg.ErpIeContent = ErpIeContent;

#ifdef A_BAND_SUPPORT

	/* Force to use ShortSlotTime at A-band */
	if (Channel > 14)
		ShortSlotCapable = TRUE;

#endif /* A_BAND_SUPPORT */
	pCapInfo = &(mbss->CapabilityInfo);

	/* In A-band, the ShortSlotTime bit should be ignored. */
	if (ShortSlotCapable
#ifdef A_BAND_SUPPORT
		&& (Channel <= 14)
#endif /* A_BAND_SUPPORT */
	   )
		(*pCapInfo) |= 0x0400;
	else
		(*pCapInfo) &= 0xfbff;

	if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
		(*pCapInfo) &= (~0x020);
	else
		(*pCapInfo) |= 0x020;


	/*update slot time only when value is difference*/
	if (pAd->CommonCfg.bUseShortSlotTime != ShortSlotCapable) {
		HW_SET_SLOTTIME(pAd, ShortSlotCapable, Channel, NULL);
		pAd->CommonCfg.bUseShortSlotTime = ShortSlotCapable;
	}
}



static INT ap_hw_tb_init(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset WCID Table\n", __func__));
	HW_SET_DEL_ASIC_WCID(pAd, WCID_ALL);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Reset Sec Table\n", __func__));
	return TRUE;
}


/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */

VOID APStartUpForMbss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev = &pMbss->wdev;
	BOOLEAN bWmmCapable = FALSE;
	EDCA_PARM *pEdca = NULL, *pBssEdca = NULL;
	UCHAR phy_mode = pAd->CommonCfg.cfg_wmode;
	UCHAR ucBandIdx = 0;
#ifdef SPECIAL_11B_OBW_FEATURE
	UINT32 phy_val_read = 0;
	UINT32 phy_val_write = 0;
	UINT32 reg_addr = 0;
	UCHAR i = 0;
	/* specific values to be set for OBW 15.75 MHZ on ch 1-14 */
	UINT32 phy_reg_value_table[MAX_TXTD_CCK_CR] = {
	0x3FEFF7FD, 0x3FDFF3FC, 0x3FBFEBFA, 0x3F9FE3F8, 0x3F7FDBF6, 0x3F5FD7F5, 0x3F5FD7F6, 0x3F6FE3F9,
	0x3FBFF801, 0x0050240E, 0x0130641F, 0x0260B836, 0x03F12051, 0x05B1946F,
	0x07A2108F, 0x09928CAD, 0x0B62FCC8, 0x0D035CDE, 0x0E33A0EC, 0x0EF3C4F2
};
#endif /* SPECIAL_11B_OBW_FEATURE */
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
	UCHAR amount_of_band = HcGetAmountOfBand(pAd);
#endif /* GREENAP_SUPPORT */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	BOOLEAN bZeroWaitStop;
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	bZeroWaitStop = MbssZeroWaitStopValidate(pAd, pMbss->wdev.channel, pMbss->mbss_idx);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s(), CfgMode:%d\n", __func__, phy_mode));

	/*Ssid length sanity check.*/
	if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID)) {
		NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
		pMbss->Ssid[5] = '0' + pMbss->mbss_idx;
		pMbss->SsidLen = 6;
	}

	if (wdev->func_idx == 0)
		MgmtTableSetMcastEntry(pAd, MCAST_WCID_TO_REMOVE);

	APSecInit(pAd, wdev);


#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, pMbss->mbss_idx, TRUE);
#endif
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	/* Deduce IPv6 Link local address for this MBSS & IPv6 format checksum for use in MLD query message*/
	calc_mldv2_gen_query_chksum(pAd, pMbss);
#endif


#if defined(WAPP_SUPPORT)
	wapp_init(pAd, pMbss);
#endif

	ap_mlme_set_capability(pAd, pMbss);
#ifdef WSC_V2_SUPPORT

	if (wdev->WscControl.WscV2Info.bEnableWpsV2) {
		/*
		    WPS V2 doesn't support Chiper WEP and TKIP.
		*/
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		if (IS_CIPHER_WEP_TKIP_ONLY(pSecConfig->PairwiseCipher)
			|| (pMbss->bHideSsid))
			WscOnOff(pAd, wdev->func_idx, TRUE);
		else
			WscOnOff(pAd, wdev->func_idx, FALSE);
	}

#endif /* WSC_V2_SUPPORT */

	/* If any BSS is WMM Capable, we need to config HW CRs */
	if (wdev->bWmmCapable)
		bWmmCapable = TRUE;

	if (WMODE_CAP_N(wdev->PhyMode) || bWmmCapable) {
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
	}

#ifdef DOT11_N_SUPPORT
	BuildChannelList(pAd, wdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[APStartUpForMbss] BuildChannelList, #711\n"));
	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
	/*update rate info for wdev*/
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);

	if (!WMODE_CAP_N(wdev->PhyMode))
		wlan_config_set_ht_bw(wdev, HT_BW_20);

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_INIT_RADIO_ONOFF
	if (pAd->ApCfg.bRadioOn) {
#endif
    if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				  pAd->CommonCfg.bBssCoexEnable,
				  pAd->CommonCfg.bForty_Mhz_Intolerant));
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
				 pAd->CommonCfg.bBssCoexEnable));
		APOverlappingBSSScan(pAd, wdev);
	}
#ifdef CONFIG_INIT_RADIO_ONOFF
	}
#endif
#endif /*RT_CFG80211_SUPPORT*/
#endif /*DOT11_N_SUPPORT*/
	MlmeUpdateTxRates(pAd, FALSE, wdev->func_idx);
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode))
		MlmeUpdateHtTxRates(pAd, wdev->func_idx);

#endif /* DOT11_N_SUPPORT */
#ifdef CONFIG_INIT_RADIO_ONOFF
	if(pAd->ApCfg.bRadioOn)
#endif
	if (WDEV_WITH_BCN_ABILITY(wdev) && wdev->bAllowBeaconing) {
		if (wdev_do_linkup(wdev, NULL) != TRUE)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: link up fail!!\n", __func__));
	}

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	ZeroWaitUpdateForMbss(pAd, bZeroWaitStop, pMbss->wdev.channel, pMbss->mbss_idx);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
	ucBandIdx = HcGetBandByWdev(wdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BssIndex = %d channel = %d\n", __func__, ucBandIdx, pMbss->wdev.channel));
#ifdef SINGLE_SKU_V2
#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		pAd->CommonCfg.SKUenable[ucBandIdx] = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! SKUenable = 1!!\n", __func__));
	}

#endif /* RF_LOCKDOWN */

#if defined(MT7615) || defined(MT7622)
	/* enable/disable SKU via profile */
	TxPowerSKUCtrl(pAd, pAd->CommonCfg.SKUenable[ucBandIdx], ucBandIdx);

	/* enable/disable BF Backoff via profile */
	TxPowerBfBackoffCtrl(pAd, pAd->CommonCfg.BFBACKOFFenable[ucBandIdx], ucBandIdx);
#else
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* SINGLE_SKU_V2*/
	/* enable/disable Power Percentage via profile */
	TxPowerPercentCtrl(pAd, pAd->CommonCfg.PERCENTAGEenable[ucBandIdx], ucBandIdx);

	/* Tx Power Percentage value via profile */
	TxPowerDropCtrl(pAd, pAd->CommonCfg.ucTxPowerPercentage[ucBandIdx], ucBandIdx);

/* Config Tx CCK Stream */
	TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream[ucBandIdx], ucBandIdx);

#ifdef RF_LOCKDOWN

	/* Check RF lock Status */
	if (chip_check_rf_lock_down(pAd)) {
		pAd->CommonCfg.BFBACKOFFenable[ucBandIdx] = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! BFBACKOFFenable = 1!!\n", __func__));
	}

#endif /* RF_LOCKDOWN */
#ifdef TX_POWER_CONTROL_SUPPORT
	/* config Power boost table via profile */
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_CCK_OFDM,
			pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_HT20,
			pAd->CommonCfg.cPowerUpHt20[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_HT40,
			pAd->CommonCfg.cPowerUpHt40[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT20,
			pAd->CommonCfg.cPowerUpVht20[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT40,
			pAd->CommonCfg.cPowerUpVht40[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT80,
			pAd->CommonCfg.cPowerUpVht80[ucBandIdx]);
	TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT160,
			pAd->CommonCfg.cPowerUpVht160[ucBandIdx]);
#endif /* TX_POWER_CONTROL_SUPPORT */
	/*init tx/rx stream */
	hc_set_rrm_init(wdev);
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_check_device_hook)
		wf_drv_tbl.wf_fwd_check_device_hook(wdev->if_dev, INT_MBSSID, pMbss->mbss_idx, wdev->channel, 1);

#endif /* CONFIG_WIFI_PKT_FWD */

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	/* Update bInitMbssZeroWait for MBSS Zero Wait */
	UPDATE_MT_INIT_ZEROWAIT_MBSS(pAd, FALSE);
#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
#ifdef VOW_SUPPORT
	vow_mbss_init(pAd, wdev);
#endif /* VOW_SUPPORT */
#ifdef GREENAP_SUPPORT
	greenap_check_allow_status(pAd, greenap);

	if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap)) {
		for (ucBandIdx = 0; ucBandIdx < amount_of_band; ucBandIdx++) {
			greenap_check_peer_connection_status(
				pAd,
				ucBandIdx,
				FALSE,
				greenap_get_allow_status(greenap));
		}
	}

#endif /* GREENAP_SUPPORT */
#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
    if (pAd->ApCfg.BandSteering) {
	PBND_STRG_CLI_TABLE table;

	table = Get_BndStrgTable(pAd, pMbss->mbss_idx);

	if (!table) {
		/* Init time table may not init as wdev not populated */
		BndStrg_TableInit(pAd, pMbss->mbss_idx);
		table = Get_BndStrgTable(pAd, pMbss->mbss_idx);
	}

	if (table) {
	    /* Inform daemon interface ready */
	    BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, TRUE);
	}
    }
#ifdef SPECIAL_11B_OBW_FEATURE
	if (wdev->channel >= 1 && wdev->channel <= 14) {
		if (!pAd->CommonCfg.dbdc_mode) {
			reg_addr = PHY_BAND0_PHY_CTRL_1;
			phy_val_read = 0;
			PHY_IO_READ32(pAd, reg_addr, &phy_val_read);
			phy_val_write = phy_val_read & (~(1 << 30));
			printk("[%s]phy_val_write = %x for register = %x\n", __func__, phy_val_write, reg_addr);
			PHY_IO_WRITE32(pAd, reg_addr, phy_val_write);
		}
		PHY_IO_READ32(pAd, reg_addr, &phy_val_read);
		if (phy_val_write != phy_val_read)
			printk("[%s] reg write not successful for reg = %x\n", __func__, reg_addr);
		for (i = 0; i < MAX_TXTD_CCK_CR ; i++) {
			reg_addr = (CR_TXTD_CCK_0 + (i*4));
			phy_val_read = 0;
			phy_val_write = 0;
			phy_val_write = phy_reg_value_table[i];
			PHY_IO_WRITE32(pAd, reg_addr, phy_val_write);
			PHY_IO_READ32(pAd, reg_addr, &phy_val_read);
			if (phy_val_write != phy_val_read)
				 printk("[%s] reg write not successful for reg = %x\n", __func__, reg_addr);
		}
	}
#endif /* SPECIAL_11B_OBW_FEATURE */
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */

#ifdef CONFIG_DOT11U_INTERWORKING
	pMbss->GASCtrl.b11U_enable = 1;
#endif /* CONFIG_DOT11U_INTERWORKING */

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev, WAPP_BSS_START);
#endif /*WAPP_SUPPORT*/

#ifdef CONFIG_AP_SUPPORT
#ifdef MIN_PHY_RATE_SUPPORT
	/*Send In-Band Command to N9 to Config Min PHy Rate*/
	MtCmdSetMinPhyRate(pAd, &pMbss->wdev);
#endif /* MIN_PHY_RATE_SUPPORT */

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
	DfsThresholdParamInit(pAd);
	Set_DfsDefaultRDDThresholdParam(pAd);
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

}

VOID APStartUpByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;
#ifdef CUSTOMER_DCC_FEATURE
	UCHAR bandIdx = 0;
#endif
	if (pMbss == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Invalid Mbss\n",
				 __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s() mbss_idx %d\n", __func__, pMbss->mbss_idx));
	wdev = &pMbss->wdev;


	/* Workaround start: Let Rx packet can be dequeued from PSE or Tx CMD will fail */
	/* Workaround end */

	/* setup tx preamble */
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	/* default NO_PROTECTION */
	AsicUpdateProtect(pAd);

	/* Update devinfo for any phymode change */
	if (wdev_do_open(wdev) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));

	/* Update ERP */
	ApUpdateCapabilityAndErpIe(pAd, pMbss);
#ifndef CONFIG_INIT_RADIO_ONOFF
	APStartUpForMbss(pAd, pMbss);
#endif
#ifdef DOT11_N_SUPPORT
	/* Update HT & GF Protect */
	APUpdateOperationMode(pAd, wdev);
#endif /* DOT11_N_SUPPORT */

	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	/*pAd->Dot11_H.RDMode = RD_NORMAL_MODE;*/
#ifdef LED_CONTROL_SUPPORT
#ifndef CONFIG_LED_ACTIVITY_ON_MAIN_MBSS
	RTMPSetLED(pAd, LED_LINK_UP);
#endif
#endif /* LED_CONTROL_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */
	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);

	/* beacon init for USB/SDIO */
	asic_bss_beacon_init(pAd);

	/* start sending BEACON out */
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pMbss && (pMbss->APStartPseduState == AP_STATE_ALWAYS_START_AP_DEFAULT))
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
	UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IF_STATE_CHG);
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, wdev);

#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);
#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n"));

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStart(pAd);
#endif /* SMART_ANTENNA */
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

#ifdef CUSTOMER_DCC_FEATURE
	bandIdx = HcGetBandByWdev(wdev);

	pAd->CommonCfg.channelSwitch.CHSWMode = NORMAL_MODE;
	pAd->Dot11_H[bandIdx].CSPeriod = pAd->CommonCfg.channelSwitch.Dot11_H_CSPeriod;
	pAd->CommonCfg.NewExtChanOffset.NewExtChanOffset = HcGetExtCha(pAd, wdev->channel);
#endif

#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
if (pAd->ApEnableBeaconTable)
	BssTableInit(&pAd->AvailableBSS);
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
			 PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));
#if defined(NF_SUPPORT) || defined(OFFCHANNEL_SCAN_FEATURE)
	if (IS_MT7615(pAd))
		enable_nf_support(pAd);
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __func__));
#ifdef VENDOR_FEATURE7_SUPPORT
	{
		UCHAR event_msg[48] = {0};

		if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode))
			sprintf(event_msg, "%sGHz AP start/restart successful\n", "5");
		else
			sprintf(event_msg, "%sGHz AP start/restart successful\n", "2.4");

		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_LOG_SAVE, 0, event_msg, strlen(event_msg));
	}
#endif
}

VOID ap_over_lapping_scan(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	struct wifi_dev *wdev;
	if (pMbss == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Invalid Mbss\n",
				 __func__));
		return;
	}
	wdev = &pMbss->wdev;

	if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n",
				  pAd->CommonCfg.bBssCoexEnable,
				  pAd->CommonCfg.bForty_Mhz_Intolerant));
	} else if (pAd->CommonCfg.bBssCoexEnable == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n",
				 pAd->CommonCfg.bBssCoexEnable));
		APOverlappingBSSScan(pAd, wdev);
	}
}

VOID APStartUp(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Invalid oper(%d)\n",
				 __func__, oper));
		return;
	}

	if (pMbss)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), oper(%d) bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, oper, pMbss->mbss_idx, PRINT_MAC(pMbss->wdev.bssid)));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), oper(%d)\n",
				 __func__, oper));

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			APStartUpByBss(pAd, pCurMbss);
			if (pCurMbss->wdev.channel && (pCurMbss->wdev.channel < 14))
				ch = pCurMbss->wdev.channel;
		}
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			if (ch < 14) {
				/* check MBSS work on the same RF(channel) */
				if (pCurMbss->wdev.channel == ch) {
					ap_over_lapping_scan(pAd, pCurMbss);
					break;
				}
			}
		}
		break;
	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != ch)
				continue;

			APStartUpByBss(pAd, pCurMbss);
		}
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {

			/* No need for obss scan if RF up is on 5Ghz */
			if (ch > 13)
				break;
			pCurMbss = &pAd->ApCfg.MBSSID[idx];

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state)
				continue;

			/* check MBSS work on the same RF(channel) */
			if ((pCurMbss->wdev.channel != ch) && (pCurMbss->wdev.channel > 13))
				continue;

			ap_over_lapping_scan(pAd, pCurMbss);
			break;
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStartUpByBss(pAd, pMbss);
		if (pMbss->wdev.if_up_down_state && pMbss->wdev.channel < 14)
			ap_over_lapping_scan(pAd, pMbss);
		break;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __func__));
}

/*Only first time will run it*/
VOID APInitForMain(RTMP_ADAPTER *pAd)
{
	BSS_STRUCT *pMbss = NULL;
	UINT32 apidx = 0;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s()\n", __func__));
	AsicDisableSync(pAd, HW_BSSID_0);/* don't gen beacon, reset tsf timer, don't gen interrupt. */
	/*reset hw wtbl*/
	ap_hw_tb_init(pAd);
	pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	pMbss->mbss_idx = MAIN_MBSSID;
	/*update main runtime attribute*/

#ifdef DOT11_N_SUPPORT
	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#ifdef PIGGYBACK_SUPPORT
	AsicSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */
#endif /* DOT11_N_SUPPORT */
	/* Workaround start: Let Rx packet can be dequeued from PSE or Tx CMD will fail */
	/* Workaround end */
	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	/* pAd->Dot11_H.RDMode = RD_NORMAL_MODE; */
	/* Disable Protection first. */
	AsicUpdateProtect(pAd);
	ApUpdateCapabilityAndErpIe(pAd, pMbss);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd, &pMbss->wdev);
#endif /* DOT11_N_SUPPORT */
#ifdef LED_CONTROL_SUPPORT
#ifndef CONFIG_LED_ACTIVITY_ON_MAIN_MBSS
	RTMPSetLED(pAd, LED_LINK_UP);
#endif
#endif /* LED_CONTROL_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	WifiFwdSet(pAd->CommonCfg.WfFwdDisabled);
#endif /* CONFIG_WIFI_PKT_FWD */
	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;
	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	/*
		Set group re-key timer if necessary.
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	APStartRekeyTimer(pAd, &pMbss->wdev);
	/* RadarStateCheck(pAd); */

#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);

#endif /* CARRIER_DETECTION_SUPPORT */

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbtt(pAd, TRUE, HW_BSSID_0);

	for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
		pMbss = &pAd->ApCfg.MBSSID[apidx];
		OPSTATUS_SET_FLAG_WDEV(&pMbss->wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	}

#ifdef IDS_SUPPORT

	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable) {
#ifdef SYSTEM_LOG_SUPPORT

		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled\n"));

#endif /* SYSTEM_LOG_SUPPORT */
		RTMPIdsStart(pAd);
	}

#endif /* IDS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStart(pAd);
#endif /* SMART_ANTENNA */
#ifdef BAND_STEERING

	if (pAd->ApCfg.BandSteering)
		BndStrg_Init(pAd);

#endif /* BAND_STEERING */
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
			 PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));
#ifdef VENDOR_FEATURE7_SUPPORT
	{
		UCHAR event_msg[48] = {0};

		if (WMODE_CAP_5G(pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode))
			sprintf(event_msg, "%sGHz AP start/restart successful\n", "5");
		else
			sprintf(event_msg, "%sGHz AP start/restart successful\n", "2.4");

		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_LOG_SAVE,
			0 /*dummy*/, event_msg, strlen(event_msg));
	}
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<=== %s()\n", __func__));
}

VOID APStopByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss)
{
	BOOLEAN Cancelled;
	struct wifi_dev *wdev_bss;
#if defined(MESH_SUPPORT) || defined(APCLI_SUPPORT) || defined(BAND_STEERING)
	INT idx;
	struct wifi_dev *wdev;
	PWSC_CTRL pWscControl;
#endif
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */

	if (pMbss == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Invalid Mbss\n",
				 __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===> %s() mbss_idx %d\n", __func__, pMbss->mbss_idx));
	wdev_bss = &pMbss->wdev;

	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);
#ifdef CARRIER_DETECTION_SUPPORT

	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE) {
		/* make sure CarrierDetect wont send CTS */
		CarrierDetectionStop(pAd);
	}

#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd) == FALSE) {
#endif
		for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
			wdev = &pAd->ApCfg.ApCliTab[idx].wdev;

			/* WPS cli will disconnect and connect again */
			pWscControl = &pAd->ApCfg.ApCliTab[idx].wdev.WscControl;
			if (pWscControl->bWscTrigger == TRUE)
				continue;

			if (wdev->channel == wdev_bss->channel) {
				UINT8 enable = pAd->ApCfg.ApCliTab[idx].Enable;

				if (enable) {
					pAd->ApCfg.ApCliTab[idx].Enable = FALSE;
					ApCliIfDown(pAd);
#ifdef CONFIG_MAP_SUPPORT
					if (!IS_MAP_TURNKEY_ENABLE(pAd))
#endif
					pAd->ApCfg.ApCliTab[idx].Enable = enable;
				}
			}
		}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	}
#endif
#endif /* APCLI_SUPPORT */

	/*AP mode*/
	MacTableResetWdev(pAd, wdev_bss);
	OPSTATUS_CLEAR_FLAG_WDEV(wdev_bss, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
#ifdef GREENAP_SUPPORT
	if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap))
		greenap_exit(pAd, wdev_bss, greenap);
#endif /* GREENAP_SUPPORT */

	/* Disable pre-tbtt interrupt */
	AsicSetPreTbtt(pAd, FALSE, HW_BSSID_0);
	/* Disable piggyback */
	AsicSetPiggyBack(pAd, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		AsicDisableSync(pAd, HW_BSSID_0);
#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}

	/* beacon exit for USB/SDIO */
	asic_bss_beacon_exit(pAd);



#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_deinit(pAd, pMbss->mbss_idx, TRUE);
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if (wf_drv_tbl.wf_fwd_entry_delete_hook)
		wf_drv_tbl.wf_fwd_entry_delete_hook(pAd->net_dev, wdev_bss->if_dev, 0);

	if (wf_drv_tbl.wf_fwd_check_device_hook)
		wf_drv_tbl.wf_fwd_check_device_hook(wdev_bss->if_dev,
			INT_MBSSID, pMbss->mbss_idx, wdev_bss->channel, 0);

#endif /* CONFIG_WIFI_PKT_FWD */
	/* clear protection to default */
	wdev_bss->protection = 0;

#ifdef CONFIG_RCSA_SUPPORT
	/* When RCSA is send, ALTX is en and BF suspended, restore state*/
	RcsaRecovery(pAd, wdev_bss);
#endif

	if (wdev_do_linkdown(wdev_bss) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() linkdown fail!!!\n", __func__));

	if (WMODE_CAP_5G(wdev_bss->PhyMode)) {
#ifdef MT_DFS_SUPPORT /* Jelly20150217 */
		WrapDfsRadarDetectStop(pAd);
		/* Zero wait hand off recovery for CAC period + interface down case */
		DfsZeroHandOffRecovery(pAd, wdev_bss);
#endif
	}

	/* Update devinfo for any phymode change */
	if (wdev_do_close(wdev_bss) != TRUE)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));

	APReleaseRekeyTimer(pAd, wdev_bss);
#ifdef BAND_STEERING
	if (pAd->ApCfg.BandSteering) {
		PBND_STRG_CLI_TABLE table;

		table = Get_BndStrgTable(pAd, idx);
		if (table) {
			/* Inform daemon interface ready */
			BndStrg_SetInfFlags(pAd, &pMbss->wdev, table, FALSE);
		}
	}
#endif /* BAND_STEERING */

	if (pAd->ApCfg.CMTimerRunning == TRUE) {
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
		pAd->ApCfg.BANClass3Data = FALSE;
	}

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */
#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */
#ifdef SMART_ANTENNA
	RtmpSAStop(pAd);
#endif /* SMART_ANTENNA */
#ifdef VOW_SUPPORT
	vow_reset(pAd);
#endif /* VOW_SUPPORT */
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	wapp_send_bss_state_change(pAd, wdev_bss, WAPP_BSS_STOP);
#endif /*WAPP_SUPPORT*/
}

VOID APStop(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;
	UCHAR ch = 0;

	/* Don't care pMbss if operation is for all */
	if ((pMbss == NULL) && (oper != AP_BSS_OPER_ALL)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() Invalid oper(%d)\n",
				 __func__, oper));
		return;
	}

	if (pMbss)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), oper(%d) bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, oper, pMbss->mbss_idx, PRINT_MAC(pMbss->wdev.bssid)));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(), oper(%d)\n",
				 __func__, oper));

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);

	switch (oper) {
	case AP_BSS_OPER_ALL:
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			APStopByBss(pAd, pCurMbss);
		}
		break;
	case AP_BSS_OPER_BY_RF:
		ch = pMbss->wdev.channel;
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			pCurMbss = &pAd->ApCfg.MBSSID[idx];
			pCurMbss->mbss_idx = idx;

			/* check MBSS status is up */
			if (!pCurMbss->wdev.if_up_down_state && !pCurMbss->wdev.bcn_buf.bBcnSntReq)
				continue;

			/* check MBSS work on the same RF(channel) */
			if (pCurMbss->wdev.channel != ch)
				continue;

			APStopByBss(pAd, pCurMbss);
		}
		break;

	case AP_BSS_OPER_SINGLE:
	default:
		APStopByBss(pAd, pMbss);
		break;
	}

	if (!RTCMDRunning(&pAd->cmdQTask))
		RTCMDUp(&pAd->cmdQTask);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<=== %s()\n", __func__));
}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	PQUEUE_ENTRY pEntry;
	PNDIS_PACKET pPacket;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): (0x%08lx)...\n", __func__, (ULONG)pQueue));

	while (pQueue->Head) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():%u...\n", __func__, pQueue->Number));
		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}

/*
*
*/
static VOID avg_pkt_len_reset(struct _RTMP_ADAPTER *ad)
{
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_reset(ad);
#endif
	ad->mcli_ctl.pkt_avg_len = 0;
	ad->mcli_ctl.sta_nums = 0;
}

/*
*
*/
static VOID avg_pkt_len_calculate(struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = entry->wdev->sys_handle;
	UINT32 avg_pkt_len = 0;
	struct multi_cli_ctl *mctrl = &ad->mcli_ctl;

#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	vow_avg_pkt_len_calculate(entry);
#endif
	if (!ad->CommonCfg.dbdc_mode)
		return;

	if (entry->avg_tx_pkts > 0)
		avg_pkt_len = (UINT32)(entry->AvgTxBytes / entry->avg_tx_pkts);

	if ((avg_pkt_len > VERIWAVE_INVALID_PKT_LEN_HIGH) ||
			(avg_pkt_len < VERIWAVE_INVALID_PKT_LEN_LOW))
		return;
	/*moving average for pkt avg length*/
	mctrl->pkt_avg_len =
		((mctrl->pkt_avg_len * mctrl->sta_nums) + avg_pkt_len) / (mctrl->sta_nums + 1);
	mctrl->sta_nums++;
}

#define FAR_CLIENT_RSSI	(-70)
#define FAR_CLIENT_DELTA_RSSI	(10)
static VOID CalFarClientNum(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
	UINT8 idx;
	CHAR avg_rssi[4];

	for (idx = 0; idx < 4; idx++)
		avg_rssi[idx] = ad->ApCfg.RssiSample.LastRssi[idx] - ad->BbpRssiToDbmDelta;

	for (idx = 0; idx < 4; idx++) {
		if ((entry->RssiSample.AvgRssi[idx] < FAR_CLIENT_RSSI) &&
			(entry->RssiSample.AvgRssi[idx] != -127)) {
			if ((avg_rssi[idx] - entry->RssiSample.AvgRssi[idx]) >= FAR_CLIENT_DELTA_RSSI) {
				ad->nearfar_far_client_num++;
				break;
			}
		}
	}
}

/*
*
*/

#ifndef DISABLE_MULTICLIENT_DYNAMIC_TXOP
static VOID dynamic_amsdu_protect_adjust(struct _RTMP_ADAPTER *ad)
{
	UCHAR amsdu_cnt = 4, band_idx = 0;
	ULONG per = 0;
	ULONG tx_cnt;
	ULONG tx_fail_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	UINT16 multi_client_num_th = 0, veriwave_tp_amsdu_dis_th = 0;
	BOOLEAN mc_flg = FALSE;

	/*concurrent case, not adjust*/
	if (ad->txop_ctl[DBDC_BAND0].multi_client_nums > 0) {
#if DBDC_BAND_NUM == 2
		if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
			return;
#endif
		band_idx = DBDC_BAND0;
	}
#if DBDC_BAND_NUM == 2
	 else if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
		band_idx = DBDC_BAND1;
#endif
	else
		return;

	wlan_ct = &ad->WlanCounters[band_idx];
	wdev = ad->txop_ctl[band_idx].cur_wdev;

	if (WMODE_CAP_2G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_2G_TP_AMSDU_DIS_TH;
	} else if (WMODE_CAP_5G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_TP_AMSDU_DIS_TH;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s[%d]\n", __func__, __LINE__));
		return;
	}

	/*adjust amsdu*/
	if (ad->txop_ctl[band_idx].multi_client_nums == multi_client_num_th) {
		mc_flg = TRUE;
		if (ad->mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
			amsdu_cnt = 2;

	} else {
		if (ad->txop_ctl[band_idx].multi_client_nums > 1) {
			if (ad->mcli_ctl.pkt_avg_len > veriwave_tp_amsdu_dis_th)
				amsdu_cnt = 1;
			else if (ad->mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
				amsdu_cnt = 2;
		}
	}

	if (ad->mcli_ctl.amsdu_cnt != amsdu_cnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): change amsdu %d to %d\n",
			__func__, ad->mcli_ctl.amsdu_cnt, amsdu_cnt));
		ad->mcli_ctl.amsdu_cnt = amsdu_cnt;
		amsdu_cnt = 4;
		if (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))
			MtCmdCr4Set(ad, CR4_SET_ID_CONFIG_STA_AMSDU_MAX_NUM, 255, amsdu_cnt);
		else
			ad->amsdu_max_num = amsdu_cnt;
	}
	/* reset */
	ad->nearfar_far_client_num = 0;

	/*only change protection scenario when sta more than MULTI_CLIENT_NUMS_TH*/
	if (!mc_flg) {
		ad->mcli_ctl.last_tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
		ad->mcli_ctl.last_tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart;
		if (ad->mcli_ctl.c2s_only == TRUE) {
			asic_rts_on_off(wdev, TRUE);
			ad->mcli_ctl.c2s_only = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): enable RTS\n", __func__));
		}
		return;
	}

	tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart - ad->mcli_ctl.last_tx_cnt;
	tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart - ad->mcli_ctl.last_tx_fail_cnt;

	ad->mcli_ctl.last_tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;
	ad->mcli_ctl.last_tx_fail_cnt = wlan_ct->AmpduFailCount.u.LowPart;

	if (tx_cnt > 0) {
		per = (100 * tx_fail_cnt) / (tx_fail_cnt + tx_cnt);
		if ((ad->mcli_ctl.c2s_only == FALSE) && (per < VERIWAVE_PER_RTS_DIS_TH_LOW_MARK)) {
			asic_rts_on_off(wdev, FALSE);
			ad->mcli_ctl.c2s_only = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): disable RTS, per=%lu\n",
			__func__, per));
		} else if ((ad->mcli_ctl.c2s_only == TRUE) && (per >= VERIWAVE_PER_RTS_DIS_TH_HIGH_MARK)) {
			asic_rts_on_off(wdev, TRUE);
			ad->mcli_ctl.c2s_only = FALSE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(): enable RTS, per=%lu\n",
			__func__, per));
		}
	}

}
#endif

#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
static VOID dynamic_airtime_fairness_adjust(struct _RTMP_ADAPTER *ad)
{
	BOOLEAN shrink_flag = FALSE;
	BOOLEAN mc_flg = FALSE, fband_2g = FALSE;
	UCHAR band_idx = 0;
	UINT16 multi_client_num_th = 0, veriwave_tp_amsdu_dis_th = 0;
	struct wifi_dev *wdev = NULL;

	if (!ad->vow_cfg.en_airtime_fairness)
		return;

	/*concurrent case, not adjust*/
	if (ad->txop_ctl[DBDC_BAND0].multi_client_nums > 0) {
#if DBDC_BAND_NUM == 2
		if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
			return;
#endif
		band_idx = DBDC_BAND0;
	}
#if DBDC_BAND_NUM == 2
	 else if (ad->txop_ctl[DBDC_BAND1].multi_client_nums > 0)
		band_idx = DBDC_BAND1;
#endif
	else
		return;

	wdev = ad->txop_ctl[band_idx].cur_wdev;

	if (WMODE_CAP_2G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_2G_TP_AMSDU_DIS_TH;
		fband_2g = TRUE;
	} else if (WMODE_CAP_5G(wdev->PhyMode)) {
		multi_client_num_th = MULTI_CLIENT_NUMS_TH;
		veriwave_tp_amsdu_dis_th = VERIWAVE_TP_AMSDU_DIS_TH;
	} else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s[%d]\n", __func__, __LINE__));
		return;
	}

	/*adjust amsdu*/
	if (ad->txop_ctl[band_idx].multi_client_nums == multi_client_num_th) {
		if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
			shrink_flag = TRUE;

	} else {
		if (ad->txop_ctl[band_idx].multi_client_nums > 1) {
			if (ad->vow_mcli_ctl.pkt_avg_len > veriwave_tp_amsdu_dis_th)
				shrink_flag = TRUE;
			else if (ad->vow_mcli_ctl.pkt_avg_len > VERIWAVE_PKT_LEN_LOW)
				shrink_flag = TRUE;
		}
	}

	if (ad->txop_ctl[band_idx].multi_client_nums == multi_client_num_th)
		mc_flg = TRUE;

	if (ad->vow_sta_frr_flag != shrink_flag) {
		/* adj airtime quantum only when WATF is not enabled */
		ad->vow_sta_frr_flag = shrink_flag;
		if (ad->vow_cfg.en_airtime_fairness && ad->vow_sta_frr_quantum && !vow_watf_is_enabled(ad)) {
			if ((shrink_flag == TRUE) & (ad->nearfar_far_client_num <= 1) && mc_flg)
				RTMP_SET_STA_DWRR_QUANTUM(ad, FALSE, ad->vow_sta_frr_quantum);/* fast round robin */
			else
				RTMP_SET_STA_DWRR_QUANTUM(ad, TRUE, 0);/* restore */
		}

	}
	return;
}
#endif /* defined (VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT) */

#ifdef WHNAT_SUPPORT
static VOID tx_offload_counter_update(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry)
{
#ifdef CONFIG_AP_SUPPORT
		if (IS_ENTRY_CLIENT(entry) && entry->pMbss) {
			entry->pMbss->TxCount += entry->one_sec_tx_pkts;
			entry->pMbss->TransmittedByteCount += entry->OneSecTxBytes;
			/* Update per-entry cumulative stats */
			entry->TxBytes += entry->OneSecTxBytes;
			entry->TxPackets.QuadPart += entry->one_sec_tx_pkts;
		}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef APCLI_SUPPORT
		if ((IS_ENTRY_APCLI(entry)
#ifdef MAC_REPEATER_SUPPORT
			|| IS_ENTRY_REPEATER(entry)
#endif /*MAC_REPEATER_SUPPORT*/
			) && entry->pApCli) {
			struct _APCLI_STRUCT *apcli_entry = entry->pApCli;
			apcli_entry->TxCount += entry->one_sec_tx_pkts;
			apcli_entry->TransmittedByteCount += entry->OneSecTxBytes;
		}
#endif /*APCLI_SUPPORT*/

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry)) {
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount.QuadPart +=  entry->one_sec_tx_pkts;
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += entry->OneSecTxBytes;
	}
#endif /* WDS_SUPPORT */
}
#endif /*WHNAT_SUPPORT*/

static VOID do_sta_keep_action(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *entry, BOOLEAN is_no_rx)
{
#ifdef IAPP_SUPPORT
	if (entry->wdev &&
		is_no_rx &&
		(ad->Mlme.PeriodicRound % STA_KEEP_ALIVE_NOTIFY_L2) == 0) {
		IAPP_L2_Update_Frame_Send(ad, entry->Addr, entry->wdev->wdev_idx);
	}
#endif /*IAPP_SUPPORT*/
}

/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/

VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int wcid, startWcid;
#ifdef DOT11_N_SUPPORT
	BOOLEAN	bRdgActive = FALSE;
	BOOLEAN bRalinkBurstMode;
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	MAC_TABLE *pMacTable;
	CHAR avgRssi;
	BSS_STRUCT *pMbss;
#ifdef WFA_VHT_PF
	RSSI_SAMPLE *worst_rssi = NULL;
	int worst_rssi_sta_idx = 0;
#endif /* WFA_VHT_PF */
#ifdef MT_MAC
	BOOLEAN bPreAnyStationInPsm = FALSE;
#endif /* MT_MAC */
	UINT     BandIdx = 0;
#ifdef SMART_CARRIER_SENSE_SUPPORT
	CHAR    tmpRssi = 0;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef APCLI_SUPPORT
	ULONG	apcli_avg_tx = 0;
	ULONG	apcli_avg_rx = 0;
	struct wifi_dev *apcli_wdev = NULL;
#endif /* APCLI_SUPPORT */
	struct wifi_dev *sta_wdev = NULL;
	struct wifi_dev *txop_wdev = NULL;
	UCHAR    sta_hit_2g_infra_case_number = 0;
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
	BOOLEAN IgmpQuerySendTickChanged = FALSE;
	BOOLEAN MldQuerySendTickChanged = FALSE;
#endif
	struct _RTMP_CHIP_CAP *cap;
	BOOLEAN is_no_rx_cnt = FALSE;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	pMacTable = &pAd->MacTab;
#ifdef MT_MAC
	bPreAnyStationInPsm = pMacTable->fAnyStationInPsm;
#endif /* MT_MAC */
	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink[0] = TRUE;
	pMacTable->fAllStationAsRalink[1] = TRUE;
	pMacTable->fCurrentStaBw40 = FALSE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStationNonGF = FALSE;
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */
	startWcid = 1;
#ifdef SMART_CARRIER_SENSE_SUPPORT

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAd->SCSCtrl.SCSMinRssi[BandIdx] =  0; /* (Reset)The minimum RSSI of STA */
		pAd->SCSCtrl.OneSecRxByteCount[BandIdx] = 0;
		pAd->SCSCtrl.OneSecTxByteCount[BandIdx] = 0;
	}

#endif /* SMART_CARRIER_SENSE_SUPPORT */

	avg_pkt_len_reset(pAd);
	/*TODO: Carter, modification start Wcid, Aid shall not simply equal to WCID.*/
	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(pAd, wcid); wcid++) {
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[wcid];
		STA_TR_ENTRY *tr_entry = &pMacTable->tr_entry[wcid];
		BOOLEAN bDisconnectSta = FALSE;

		if (IS_ENTRY_NONE(pEntry))
			continue;

#ifdef CUSTOMER_DCC_FEATURE
		GetTxRxActivityTime(pAd, wcid);
#endif

#ifdef HTC_DECRYPT_IOT

		if (pEntry && (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))) {
			if (pEntry->HTC_AAD_OM_CountDown > 0) /* count down to start all new pEntry->HTC_ICVErrCnt */
				pEntry->HTC_AAD_OM_CountDown--;
		}

#endif /* HTC_DECRYPT_IOT */
#ifdef SMART_CARRIER_SENSE_SUPPORT

		if (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) {
			BandIdx = HcGetBandByWdev(pEntry->wdev);
			pAd->SCSCtrl.OneSecRxByteCount[BandIdx] += pEntry->OneSecRxBytes;
			pAd->SCSCtrl.OneSecTxByteCount[BandIdx] += pEntry->OneSecTxBytes;

			if (pAd->SCSCtrl.SCSEnable[BandIdx] == SCS_ENABLE) {
				tmpRssi = RTMPMinRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1],
									  pEntry->RssiSample.AvgRssi[2], pEntry->RssiSample.AvgRssi[3]);

				if (tmpRssi < pAd->SCSCtrl.SCSMinRssi[BandIdx])
					pAd->SCSCtrl.SCSMinRssi[BandIdx] = tmpRssi;
			}
		}

#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef RACTRL_LIMIT_MAX_PHY_RATE
#ifdef DOT11_VHT_AC

		if (pAd->MacTab.Size == 0)
			pAd->fgRaLimitPhyRate = FALSE;
		else if ((pAd->fgRaLimitPhyRate == FALSE) && !IS_ENTRY_NONE(pEntry)) {
			UINT16 u2TxTP;
			BOOLEAN fgPhyModeCheck = FALSE;

			u2TxTP = pEntry->OneSecTxBytes >> BYTES_PER_SEC_TO_MBPS;

			if (pEntry->SupportRateMode & SUPPORT_VHT_MODE) {
				if (pEntry->MaxHTPhyMode.field.BW == BW_160)
					fgPhyModeCheck = TRUE;

				if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_3SS) {
					if (pEntry->SupportVHTMCS4SS)
						fgPhyModeCheck = TRUE;
				} else if (RACTRL_LIMIT_MAX_PHY_RATE >= MAX_PHY_RATE_2SS) {
					if (pEntry->SupportVHTMCS3SS)
						fgPhyModeCheck = TRUE;
				} else
					fgPhyModeCheck = TRUE;
			}

			if ((u2TxTP > LIMIT_MAX_PHY_RATE_THRESHOLD) && fgPhyModeCheck) {
				MtCmdSetMaxPhyRate(pAd, RACTRL_LIMIT_MAX_PHY_RATE);
				pAd->fgRaLimitPhyRate = TRUE;
			}
		}

#endif /* DOT11_VHT_AC */
#endif /* RACTRL_LIMIT_MAX_PHY_RATE */
#ifdef APCLI_SUPPORT

		if (IS_ENTRY_APCLI(pEntry) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
			PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];

			pApCliEntry->OneSecTxBytes = pEntry->OneSecTxBytes;
			pApCliEntry->OneSecRxBytes = pEntry->OneSecRxBytes;
#ifdef APCLI_CERT_SUPPORT

			if (pApCliEntry->NeedFallback == TRUE) {
				pApCliEntry->NeedFallback = FALSE;

				if (pAd->bApCliCertTest == TRUE) {
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
					if (cap->fgRateAdaptFWOffload == TRUE) {
						CMD_STAREC_AUTO_RATE_UPDATE_T rRaParam;

						NdisZeroMemory(&rRaParam, sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T));
						rRaParam.u4Field = RA_PARAM_HT_2040_COEX;
						RAParamUpdate(pAd, pEntry, &rRaParam);
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("FallBack APClient BW to 20MHz\n"));
					}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
				}
			}

#endif /* APCLI_CERT_SUPPORT */
		}

#endif /* APCLI_SUPPORT */

#ifdef WHNAT_SUPPORT
		if ((pAd->CommonCfg.whnat_en) && (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))) {
			MtCmdCr4Query(pAd, CR4_QUERY_OPTION_GET_TX_STATISTICS, wcid, 0);
			tx_offload_counter_update(pAd, pEntry);
		}
		else
#endif
		{
			pEntry->AvgTxBytes = (pEntry->AvgTxBytes == 0) ?
								 pEntry->OneSecTxBytes :
								 ((pEntry->AvgTxBytes + pEntry->OneSecTxBytes) >> 1);
			pEntry->OneSecTxBytes = 0;
			pEntry->avg_tx_pkts = (pEntry->avg_tx_pkts == 0) ? \
								  pEntry->one_sec_tx_pkts : \
								  ((pEntry->avg_tx_pkts + pEntry->one_sec_tx_pkts) >> 1);
			pEntry->one_sec_tx_pkts = 0;
		}

		pEntry->AvgRxBytes = (pEntry->AvgRxBytes == 0) ?
							 pEntry->OneSecRxBytes :
							 ((pEntry->AvgRxBytes + pEntry->OneSecRxBytes) >> 1);
		pEntry->OneSecRxBytes = 0;

		HW_GET_TX_STATISTIC(pAd, GET_TX_STAT_ENTRY_TX_CNT, wcid);
#ifdef APCLI_SUPPORT

		if ((IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
		   ) {
#ifdef MAC_REPEATER_SUPPORT

			if (pEntry->bReptCli) {
				pEntry->ReptCliIdleCount++;

				if ((pEntry->bReptEthCli)
					&& (pEntry->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME)
					&& (pEntry->bReptEthBridgeCli == FALSE)) { /* Do NOT ageout br0 link. @2016/1/27 */
					REPEATER_CLIENT_ENTRY *pReptCliEntry = NULL;

					pReptCliEntry = &pAd->ApCfg.pRepeaterCliPool[pEntry->MatchReptCliIdx];

					if (pReptCliEntry)
						pReptCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA;

					MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
								APCLI_CTRL_DISCONNECT_REQ,
								0,
								NULL,
								(REPT_MLME_START_IDX + pEntry->MatchReptCliIdx));
					RTMP_MLME_HANDLER(pAd);
					continue;
				}
			}

#endif /* MAC_REPEATER_SUPPORT */

			if (IS_ENTRY_APCLI(pEntry))
				apcli_wdev = pEntry->wdev;

			apcli_avg_tx += pEntry->AvgTxBytes;
			apcli_avg_rx += pEntry->AvgRxBytes;

			if (((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
#ifdef CONFIG_MULTI_CHANNEL
				&& (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			   ) {
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
					ApclibQosNull = TRUE;

					ApCliRTMPSendNullFrame(pAd, pEntry->CurrTxRate, ApclibQosNull, pEntry, PWR_ACTIVE);
				continue;
			}
		}

#endif /* APCLI_SUPPORT */

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->fgGband256QAMSupport && (pEntry->RXBAbitmap != 0) && (pEntry->TXBAbitmap != 0) &&
			sta_hit_2g_infra_case_number <= STA_NUMBER_FOR_TRIGGER) {
			sta_wdev = pEntry->wdev;

			if (WMODE_CAP_2G(sta_wdev->PhyMode)) {
				UINT tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
				UINT rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);

				if (tx_tp > INFRA_TP_PEEK_BOUND_THRESHOLD && ((tx_tp + rx_tp) > 0) &&
					(tx_tp * 100) / (tx_tp + rx_tp) > TX_MODE_RATIO_THRESHOLD) {
					if (sta_hit_2g_infra_case_number < STA_NUMBER_FOR_TRIGGER) {
						txop_wdev = sta_wdev;
						sta_hit_2g_infra_case_number++;
					} else
						sta_hit_2g_infra_case_number++;
				}
			}
		}

		if (pEntry->wdev) {
			UINT32 tx_tp = (pEntry->AvgTxBytes >> BYTES_PER_SEC_TO_MBPS);
			UINT32 rx_tp = (pEntry->AvgRxBytes >> BYTES_PER_SEC_TO_MBPS);
			ULONG avg_tx_b = pEntry->AvgTxBytes;
			ULONG avg_rx_b = pEntry->AvgRxBytes;
			UINT8 traffc_mode = 0;
#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			ULONG data_rate = 0;
			UINT32 tp_ratio = 0;
			UINT8 bidir_traffc_mode = 0;
#endif

#ifdef CONFIG_TX_DELAY
			struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
			RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if ((rx_tp >= MIN_AGG_EN_TP) && (rx_tp <= MAX_AGG_EN_TP) && cap->tx_delay_support) {
				if (!tx_delay_ctl->que_agg_en) {
					tx_delay_ctl->que_agg_en = TRUE;
					if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
						MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
								TX_DELAY_MODE_ARG1_TX_BATCH_CNT, tx_delay_ctl->tx_process_batch_cnt);
					}
				}
			} else {
				if (tx_delay_ctl->que_agg_en) {
					tx_delay_ctl->que_agg_en = FALSE;
					if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
						MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
								TX_DELAY_MODE_ARG1_TX_BATCH_CNT, 1);
					}
				}
			}
#endif

			if ((tx_tp + rx_tp) == 0)
				traffc_mode = TRAFFIC_0;
			else if (((tx_tp * 100) / (tx_tp + rx_tp)) > TX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_DL_MODE;
			else if (((rx_tp * 100) / (tx_tp + rx_tp)) > RX_MODE_RATIO_THRESHOLD)
				traffc_mode = TRAFFIC_UL_MODE;

			/* get associated band with wdev*/
			BandIdx = HcGetBandByWdev(pEntry->wdev);
			if (traffc_mode) {
				struct tr_delay_control *tr_delay_ctl = &pAd->tr_ctl.tr_delay_ctl;
				UINT32 i;
				struct dly_ctl_cfg *cfg;
				UINT32 reg_val;
				UINT32 rx_reg_val = 0x0;

				HIF_IO_READ32(pAd, MT_DELAY_INT_CFG, &reg_val);

				rx_reg_val = reg_val & 0xffff;

				if (tr_delay_ctl->rx_delay_en) {
					if (traffc_mode == TRAFFIC_DL_MODE) {
						for (i = 0; i < tr_delay_ctl->dl_rx_dly_ctl_tbl_size; i++) {
							cfg = tr_delay_ctl->dl_rx_dly_ctl_tbl + i;
							if (tx_tp > cfg->avg_tp) {
								rx_reg_val = cfg->dly_cfg;
								continue;
							} else {
								break;
							}
						}
					}

					if (traffc_mode == TRAFFIC_UL_MODE) {
						for (i = 0; i < tr_delay_ctl->ul_rx_dly_ctl_tbl_size; i++) {
							cfg = tr_delay_ctl->ul_rx_dly_ctl_tbl + i;
							if (rx_tp > cfg->avg_tp) {
								rx_reg_val = cfg->dly_cfg;
								continue;
							} else {
								break;
							}
						}
					}
				}

				HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, rx_reg_val);
			}

			if (WMODE_CAP_5G(pEntry->wdev->PhyMode) && (pAd->txop_ctl[BandIdx].multi_client_nums < MULTI_CLIENT_NUMS_TH)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;

				if ((pEntry->avg_tx_pkts > VERIWAVE_5G_PKT_CNT_TH) && ((avg_tx_b + avg_rx_b) > 0) &&
					(((avg_tx_b * 100) / (avg_tx_b + avg_rx_b)) > TX_MODE_RATIO_THRESHOLD))
					pAd->txop_ctl[BandIdx].multi_client_nums++;
			}

			if (WMODE_CAP_2G(pEntry->wdev->PhyMode) && (pAd->txop_ctl[BandIdx].multi_client_nums < MULTI_CLIENT_2G_NUMS_TH)) {
				pAd->txop_ctl[BandIdx].cur_wdev = pEntry->wdev;

				if ((pEntry->avg_tx_pkts > VERIWAVE_2G_PKT_CNT_TH) && ((avg_tx_b + avg_rx_b) > 0) &&
					(((avg_tx_b * 100) / (avg_tx_b + avg_rx_b)) > TX_MODE_RATIO_THRESHOLD))
					pAd->txop_ctl[BandIdx].multi_client_nums++;
			}

			avg_pkt_len_calculate(pEntry);
			CalFarClientNum(pAd, pEntry);

			if (WMODE_CAP_5G(pEntry->wdev->PhyMode)) {
				if (pAd->peak_tp_ctl[BandIdx].cur_wdev == NULL)
					pAd->peak_tp_ctl[BandIdx].cur_wdev = pEntry->wdev;
				pAd->peak_tp_ctl[BandIdx].client_nums++;
				if (traffc_mode == TRAFFIC_DL_MODE) {
					/* the entry is in tx mode */
					if (tx_tp > pAd->peak_tp_ctl[BandIdx].max_tx_tp) {
						pAd->peak_tp_ctl[BandIdx].max_tx_tp = tx_tp;
						pAd->peak_tp_ctl[BandIdx].cur_wdev = pEntry->wdev;
					}
				}
			}

			if (WMODE_CAP_2G(pEntry->wdev->PhyMode)) {
				if (pAd->peak_tp_ctl[BandIdx].cur_wdev == NULL)
					pAd->peak_tp_ctl[BandIdx].cur_wdev = pEntry->wdev;

				pAd->peak_tp_ctl[BandIdx].client_nums++;

				if (traffc_mode == TRAFFIC_DL_MODE) {
					/* the entry is in tx mode */
					if (tx_tp > pAd->peak_tp_ctl[BandIdx].max_tx_tp) {
						pAd->peak_tp_ctl[BandIdx].max_tx_tp = tx_tp;
						pAd->peak_tp_ctl[BandIdx].cur_wdev = pEntry->wdev;
					}
				}
			}

			if (pEntry->one_sec_tx_succ_pkts > INFRA_KEEP_STA_PKT_TH && pEntry->NoDataIdleCount != 0) {
				pEntry->NoDataIdleCount = 0;
				is_no_rx_cnt = TRUE;
			}

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
			if (IS_MAP_ENABLE(pAd)) {
				getRate(pEntry->HTPhyMode, &data_rate);
				tp_ratio = ((tx_tp + rx_tp) * 100) / data_rate;

				if (tp_ratio > STA_TP_IDLE_THRESHOLD)
					bidir_traffc_mode = TRAFFIC_BIDIR_ACTIVE_MODE;
				else
					bidir_traffc_mode = TRAFFIC_BIDIR_IDLE_MODE;

				if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_ACTIVE_MODE) &&
					(tp_ratio <= STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, INACTIVE);
				else if ((pEntry->pre_traffic_mode == TRAFFIC_BIDIR_IDLE_MODE) &&
					(tp_ratio > STA_TP_IDLE_THRESHOLD))
					wapp_send_cli_active_change(pAd, pEntry, ACTIVE);

				pEntry->pre_traffic_mode = bidir_traffc_mode;
			}
#endif
		}


		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		pEntry->LastRxTimeCount++;
#endif
		pEntry->NoDataIdleCount++;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
		pEntry->StaConnectTime++;
		pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %lu sec\n",
					  PRINT_MAC(pEntry->Addr), pEntry->AssocDeadLine));
#ifdef WSC_AP_SUPPORT

			if (NdisEqualMemory(pEntry->Addr, pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
				mac_entry_delete(pAd, pEntry);
			continue;
		}

		/*
			1. check if there's any associated STA in power-save mode. this affects outgoing
				MCAST/BCAST frames should be stored in PSQ till DtimCount=0
		*/
		if (pEntry->PsMode == PWR_SAVE) {
			pMacTable->fAnyStationInPsm = TRUE;

			if (pEntry->wdev &&
				(pEntry->wdev->wdev_type == WDEV_TYPE_AP || pEntry->wdev->wdev_type == WDEV_TYPE_GO)) {
				/* TODO: it looks like pEntry->wdev->tr_tb_idx is not assigned? */
				pAd->MacTab.tr_entry[pEntry->wdev->tr_tb_idx].PsMode = PWR_SAVE;

				if (tr_entry->PsDeQWaitCnt) {
					tr_entry->PsDeQWaitCnt++;

					if (tr_entry->PsDeQWaitCnt > 2)
						tr_entry->PsDeQWaitCnt = 0;
				}
			}
		}

#ifdef DOT11_N_SUPPORT

		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD)
			pMacTable->fAnyStationNonGF = TRUE;

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = TRUE;

#ifdef DOT11N_DRAFT3

		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;

#endif /* DOT11N_DRAFT3 */

#endif /* DOT11_N_SUPPORT */
		/* detect the station alive status */

		/* detect the station alive status */
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime)) {
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

			    ......
			    sam> tx ok!
			    sam> count = 1!	 ==> 1 second after the Null Frame is acked
			    sam> count = 2!	 ==> 2 second after the Null Frame is acked
			    sam> count = 3!
			    sam> count = 4!
			    sam> count = 5!
			    sam> count = 6!
			    sam> count = 7!
			    sam> count = 8!
			    sam> count = 9!
			    sam> count = 10!
			    sam> count = 11!
			    sam> count = 12!
			    sam> count = 13!
			    sam> count = 14!
			    sam> count = 15! ==> 15 second after the Null Frame is acked
			    sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
			    sam> count = 1!  ==> 1 second after the Null Frame is acked
			    sam> count = 2!
			    sam> count = 3!
			    sam> count = 4!
			    ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/
			if (pEntry->StationKeepAliveCount++ == 0) {

					if (pEntry->PsMode == PWR_SAVE) {
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->func_tb_idx, pEntry->Aid);
					} else {
						/* use Null or QoS Null to detect the ACTIVE station */
						BOOLEAN bQosNull = FALSE;

						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = TRUE;

						RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
											 pEntry->Aid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
					}

			} else {
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */

		if (pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout) {
			bDisconnectSta = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					 PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_FRAME_LOST, REASON_AGING_TIME_OUT);
#endif
		} else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck) {
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE) {
				bDisconnectSta = TRUE;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left (%d %lu)\n",
						 PRINT_MAC(pEntry->Addr),
						 pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck));
#ifdef WIFI_DIAG
				if (pEntry && IS_ENTRY_CLIENT(pEntry))
					DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_FRAME_LOST, REASON_CONTINUE_TX_FAIL);
#endif
			}
		}

		avgRssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

		if (pMbss->RssiLowForStaKickOut &&
			(avgRssi < pMbss->RssiLowForStaKickOut)) {
			bDisconnectSta = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("Disassoc STA %02x:%02x:%02x:%02x:%02x:%02x , RSSI Kickout Thres[%d]-[%d]\n",
					  PRINT_MAC(pEntry->Addr), pMbss->RssiLowForStaKickOut,	avgRssi));

#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_DEAUTH, REASON_RSSI_TOO_LOW);
#endif
		}

		if (bDisconnectSta) {
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0);
#ifdef VENDOR_FEATURE7_SUPPORT
			{
				UCHAR disassoc_event_msg[32] = {0};

				if (WMODE_CAP_5G(pMbss->wdev.PhyMode))
					snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					" %02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_50_RADIO);
				else
					snprintf(disassoc_event_msg, sizeof(disassoc_event_msg),
					" %02x:%02x:%02x:%02x:%02x:%02x BSS(%d)",
					PRINT_MAC(pEntry->Addr), (pEntry->func_tb_idx) + WIFI_24_RADIO);
				ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, STA_DISSOC,
					disassoc_event_msg, strlen(disassoc_event_msg));
			}
#endif
			if (pEntry->Sst == SST_ASSOC) {
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;
				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}

				Reason = REASON_DEAUTH_STA_LEAVING;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x\n",
						 Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen,
								  sizeof(HEADER_802_11), &DeAuthHdr,
								  2, &Reason,
								  END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);
#ifdef MAC_REPEATER_SUPPORT

				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
					&& (IS_ENTRY_A4(pEntry) == FALSE)
#endif /* A4_CONN */
				) {
					UCHAR apCliIdx, CliIdx;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(
									 pAd,
									 TRUE,
									 pEntry->Addr,
									 TRUE);

					if (pReptEntry &&
						(pReptEntry->CliConnectState != REPT_ENTRY_DISCONNT)) {
						pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MTM_REMOVE_STA;
						apCliIdx = pReptEntry->MatchApCliIdx;
						CliIdx = pReptEntry->MatchLinkIdx;
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

				mac_entry_delete(pAd, pEntry);
			continue;
		} else {
			do_sta_keep_action(pAd, pEntry, is_no_rx_cnt);
		}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)

		if (pEntry->BTMDisassocCount == 1) {
			PUCHAR      pOutBuffer = NULL;
			NDIS_STATUS NStatus;
			ULONG       FrameLen = 0;
			HEADER_802_11 DisassocHdr;
			USHORT      Reason;
			/*  send out a DISASSOC request frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

			if (NStatus != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
				/*NdisReleaseSpinLock(&pAd->MacTabLock); */
				continue;
			}

			Reason = REASON_DISASSOC_INACTIVE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n", Reason, pEntry->Addr[0], pEntry->Addr[1],
					  pEntry->Addr[2], pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
			MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
			MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
			MiniportMMRequest(pAd, MGMT_USE_PS_FLAG, pOutBuffer, FrameLen);
			MlmeFreeMemory(pOutBuffer);

#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_DEAUTH, Reason);
#endif

			/* JERRY */
			if (!pEntry->IsKeep) {
					mac_entry_delete(pAd, pEntry);
			}

			continue;
		}

		if (pEntry->BTMDisassocCount != 0)
			pEntry->BTMDisassocCount--;

#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the ps_queue if the STA has being idle for a while */
		if ((pEntry->PsMode == PWR_SAVE) && (tr_entry->ps_state == APPS_RETRIEVE_DONE ||
											 tr_entry->ps_state == APPS_RETRIEVE_IDLE)) {
			if (tr_entry->enqCount > 0) {
				tr_entry->PsQIdleCount++;

				if (tr_entry->PsQIdleCount > 5) {
					struct qm_ops *ops = pAd->qm_ops;

					if (ops->sta_clean_queue)
						ops->sta_clean_queue(pAd, pEntry->wcid);

					tr_entry->PsQIdleCount = 0;
					WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Clear WCID[%d] packets\n", __func__, pEntry->wcid));
				}
			}
		} else
			tr_entry->PsQIdleCount = 0;

#ifdef UAPSD_SUPPORT
		UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) {
			UCHAR band_idx;

			band_idx = HcGetBandByWdev(pEntry->wdev);
			pMacTable->fAllStationAsRalink[band_idx] = FALSE;
		}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

		if ((pEntry->BSS2040CoexistenceMgmtSupport)
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		   )
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->func_tb_idx);

#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) &&
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
			pMacTable->fAllStationGainGoodMCS = FALSE;

		/* Check Current STA's Operation Mode is BW20 or BW40 */
		pMacTable->fCurrentStaBw40 = (pEntry->HTPhyMode.field.BW == BW_40) ? TRUE : FALSE;
#ifdef WFA_VHT_PF

		if (worst_rssi == NULL) {
			worst_rssi = &pEntry->RssiSample;
			worst_rssi_sta_idx = wcid;
		} else {
			if (worst_rssi->AvgRssi[0] > pEntry->RssiSample.AvgRssi[0]) {
				worst_rssi = &pEntry->RssiSample;
				worst_rssi_sta_idx = wcid;
			}
		}

#endif /* WFA_VHT_PF */
#if (defined(WH_EVENT_NOTIFIER) || (defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)))
		if (pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
			&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			) {
#if defined(A4_CONN) && defined(IGMP_SNOOP_SUPPORT)
				if (pMbss->a4_init && pMbss->wdev.IgmpSnoopEnable) {
											/* If Snooping & MWDS enabled*/
					if ((pAd->Mlme.OneSecPeriodicRound % 10) == 0)	{
												/* Per 10 sec check */
						/* Logic implemented to do Periodic Multicast membership queries to NON-MWDS STAs
						 for update of entries in IGMP table, to avoid aging of interested members.
						 Also, in case another Membership querier detected in network, logic implemented to
						 hold internal query transmission to avoid flooding on network.
						 Decrement time tick counters for Query Sent & Query hold as applicable
						 Send the query once on each MBSS, for which both these counters are 0.
						 (Ensured that even with multiple STA on a MBSS, only one multicast query transmitted)*/

						if (pAd->bIGMPperiodicQuery == TRUE) {
							/* If Periodic IGMP query feature enabled */

							if ((pMbss->IgmpQueryHoldTick > 0) && (pMbss->IgmpQueryHoldTickChanged == FALSE)) {
								/*Decrement IgmpQueryHoldTick, only once for each MBSS*/
								pMbss->IgmpQueryHoldTick--;
								pMbss->IgmpQueryHoldTickChanged = TRUE;
							}

							if ((pAd->IgmpQuerySendTick > 0) && (IgmpQuerySendTickChanged == FALSE)) {
								/*Decrement IgmpQuerySendTick, only once for each MBSS*/
								pAd->IgmpQuerySendTick--;
								IgmpQuerySendTickChanged = TRUE;
							}

							if ((pMbss->IGMPPeriodicQuerySent == FALSE)
							   && ((pMbss->IgmpQueryHoldTick == 0) && (pAd->IgmpQuerySendTick == 0))) {
								/*transmit IGMP query on this MBSS, only once*/
								send_igmpv3_gen_query_pkt(pAd, pEntry);

								pMbss->IGMPPeriodicQuerySent = TRUE;
							}
						}

						if (pAd->bMLDperiodicQuery == TRUE) { /*If Periodic MLD query feature enabled*/

							if ((pMbss->MldQueryHoldTick > 0) && (pMbss->MldQueryHoldTickChanged == FALSE)) {
								/*Decrement MldQueryHoldTick, only once for each MBSS*/
								pMbss->MldQueryHoldTick--;
								pMbss->MldQueryHoldTickChanged = TRUE;
							}

							if ((pAd->MldQuerySendTick > 0) && (MldQuerySendTickChanged == FALSE)) {
								/*Decrement MldQuerySendTick, only once for each MBSS*/
								pAd->MldQuerySendTick--;
								MldQuerySendTickChanged = TRUE;
							}
							if ((pMbss->MLDPeriodicQuerySent == FALSE)
							   && ((pMbss->MldQueryHoldTick == 0) && (pAd->MldQuerySendTick == 0)))	{
								/*transmit MLD query on this MBSS, only once*/
								send_mldv2_gen_query_pkt(pAd, pEntry);

								pMbss->MLDPeriodicQuerySent = TRUE;
							}

						}

					} else if ((pAd->Mlme.OneSecPeriodicRound % 10) == 1) { /* Per 11 sec check */

						if (pAd->IgmpQuerySendTick == 0) /* Set the period for IGMP query again */
							pAd->IgmpQuerySendTick = QUERY_SEND_PERIOD;

						if (pAd->MldQuerySendTick == 0) /* Set the period for MLD query again */
							pAd->MldQuerySendTick = QUERY_SEND_PERIOD;

						if (pMbss->IGMPPeriodicQuerySent == TRUE)
							pMbss->IGMPPeriodicQuerySent = FALSE; /* Reset flag for next period query */

						if (pMbss->MLDPeriodicQuerySent == TRUE)
							pMbss->MLDPeriodicQuerySent = FALSE; /* Reset flag for next period query */

						pMbss->IgmpQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */
						pMbss->MldQueryHoldTickChanged = FALSE; /* Reset flag for next 10th second counter edit */

					}
				}
#endif

			}
#endif

	}

#ifdef WFA_VHT_PF

	if (worst_rssi != NULL &&
		((pAd->Mlme.OneSecPeriodicRound % 10) == 5) &&
		(worst_rssi_sta_idx >= 1)) {
		CHAR gain = 2;

		if (worst_rssi->AvgRssi[0] >= -40)
			gain = 1;
		else if (worst_rssi->AvgRssi[0] <= -50)
			gain = 2;

		rt85592_lna_gain_adjust(pAd, gain);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s():WorstRSSI for STA(%02x:%02x:%02x:%02x:%02x:%02x):%d,%d,%d, Set Gain as %s\n",
				  __func__,
				  PRINT_MAC(pMacTable->Content[worst_rssi_sta_idx].Addr),
				  worst_rssi->AvgRssi[0], worst_rssi->AvgRssi[1], worst_rssi->AvgRssi[2],
				  (gain == 2 ? "Mid" : "Low")));
	}

#endif /* WFA_VHT_PF */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);

#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg) {
		if (pMacTable->fAllStationAsRalink[0])
			bRdgActive = TRUE;
		else
			bRdgActive = FALSE;

		if (pAd->CommonCfg.dbdc_mode) {
			if (pMacTable->fAllStationAsRalink[1])
				; /* Not support yet... */
		}
	}

#ifdef APCLI_SUPPORT

	if (apcli_wdev) {
		UINT tx_tp = (apcli_avg_tx >> BYTES_PER_SEC_TO_MBPS);
		UINT rx_tp = (apcli_avg_rx >> BYTES_PER_SEC_TO_MBPS);

		apcli_dync_txop_alg(pAd, apcli_wdev, tx_tp, rx_tp);
	}

#endif /* APCLI_SUPPORT */

	if (sta_hit_2g_infra_case_number == STA_NUMBER_FOR_TRIGGER) {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == FALSE) {
			pAd->g_mode_txop_wdev = txop_wdev;
			pAd->G_MODE_INFRA_TXOP_RUNNING = TRUE;
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
		} else if (pAd->g_mode_txop_wdev != txop_wdev) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			enable_tx_burst(pAd, txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_FE);
			pAd->g_mode_txop_wdev = txop_wdev;
		}
	} else {
		if (pAd->G_MODE_INFRA_TXOP_RUNNING == TRUE) {
			disable_tx_burst(pAd, pAd->g_mode_txop_wdev, AC_BE, PRIO_2G_INFRA, TXOP_0);
			pAd->G_MODE_INFRA_TXOP_RUNNING = FALSE;
			pAd->g_mode_txop_wdev = NULL;
		}
	}

#ifndef DISABLE_MULTICLIENT_DYNAMIC_TXOP
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		if (pAd->txop_ctl[BandIdx].multi_client_nums == MULTI_CLIENT_NUMS_TH) {
			if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == FALSE) {
				pAd->txop_ctl[BandIdx].multi_cli_txop_running = TRUE;
				enable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
		} else {
			if (pAd->txop_ctl[BandIdx].multi_cli_txop_running == TRUE) {
				pAd->txop_ctl[BandIdx].multi_cli_txop_running = FALSE;
				disable_tx_burst(pAd, pAd->txop_ctl[BandIdx].cur_wdev, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
			}
		}
	}

/*
	if (pAd->txop_ctl.multi_client_nums_2g == MULTI_CLIENT_2G_NUMS_TH) {
		if (pAd->txop_ctl.multi_cli_txop_2g_running == FALSE) {
			pAd->txop_ctl.multi_cli_txop_2g_running = TRUE;
			enable_tx_burst(pAd, pAd->txop_ctl.cur_wdev_2g, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
	} else {
		if (pAd->txop_ctl.multi_cli_txop_2g_running == TRUE) {
			pAd->txop_ctl.multi_cli_txop_2g_running = FALSE;
			disable_tx_burst(pAd, pAd->txop_ctl.cur_wdev_2g, AC_BE, PRIO_MULTI_CLIENT, TXOP_0);
		}
	}
*/
	/*dynamic adjust amsdu & protection mode*/
	dynamic_amsdu_protect_adjust(pAd);
#endif
#if defined(VOW_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	dynamic_airtime_fairness_adjust(pAd);
#endif
	dynamic_ampdu_efficiency_adjust_all(pAd);

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = TRUE;
	else
		bRalinkBurstMode = FALSE;

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE)) {
		/* DBDC not support yet, only using BAND_0 */
	}

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);

#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

	/*
	   4.
	   garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
	   stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	/* TODO: shiang-usw. revise this becasue now we have per-BSS McastPsQueue! */
#ifdef RT_CFG80211_SUPPORT
#else
	if (pMacTable->McastPsQueue.Head) {
		UINT bss_index;

		pMacTable->PsQIdleCount++;

		if (pMacTable->PsQIdleCount > 1) {
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);

			/* clear MCAST/BCAST backlog bit for all BSS */
			for (bss_index = BSS0; bss_index < pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	} else
		pMacTable->PsQIdleCount = 0;
#endif

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
#ifndef DISABLE_MULTICLIENT_DYNAMIC_TXOP
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++)
		pAd->txop_ctl[BandIdx].multi_client_nums = 0;
#endif
}


UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd)
{
	UINT32 num = 0;
	UINT32 i;

	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num++;
	}

	return num;
}


/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	OUT SST * Sst,
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) { /* mcast & broadcast address */
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID_TO_REMOVE;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate;
	} else { /* unicast address */
		pEntry = MacTableLookup(pAd, pAddr);

		if (pEntry) {
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;

			if (IS_AKM_WPA_CAPABILITY_Entry(pEntry)
				&& (pEntry->SecConfig.Handshake.GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		} else {
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID_TO_REMOVE;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate;
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT) {
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];

		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("LOG#%ld %02x:%02x:%02x:%02x:%02x:%02x %s\n",
				 pAd->EventTab.Num, pAddr[0], pAddr[1], pAddr[2],
				 pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates.
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN bNonGFExist = FALSE;
	ADD_HT_INFO_IE *addht = NULL;
	UINT32 new_protection = 0;
	UCHAR op_mode = NON_PROTECT;

	if (wdev == NULL)
		return;

	addht = wlan_operate_get_addht(wdev);

	/* non HT BSS exist within 5 sec */
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32 &&
		pAd->Mlme.Now32 != 0) {
		op_mode = NONMEMBER_PROTECT;
		bNonGFExist = TRUE; /* non-HT means nonGF support */
		new_protection = SET_PROTECT(NON_MEMBER_PROTECT);
	}

	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((op_mode == NON_PROTECT) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(wlan_config_get_ht_bw(wdev) == HT_BW_40)) {
		op_mode = BW20_PROTECT;
		new_protection = SET_PROTECT(HT20_PROTECT);
	}

	if (pAd->MacTab.fAnyStationIsLegacy) {
		op_mode = NONHT_MM_PROTECT;
		new_protection = SET_PROTECT(NON_HT_MIXMODE_PROTECT);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 (" --%s:\n OperationMode: %d, bNonGFExist: %d\n",
			  __func__, addht->AddHtInfo2.OperaionMode, bNonGFExist));

	if ((op_mode != addht->AddHtInfo2.OperaionMode)
			|| (pAd->MacTab.fAnyStationNonGF != addht->AddHtInfo2.NonGfPresent)) {
		addht->AddHtInfo2.OperaionMode = op_mode;
		addht->AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF;

		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
	}

	if (bNonGFExist == FALSE)
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;

	if (bNonGFExist)
		new_protection |= SET_PROTECT(GREEN_FIELD_PROTECT);

	if (nonerp_protection(&pAd->ApCfg.MBSSID[wdev->func_idx]))
		new_protection |= SET_PROTECT(ERP);

	/*if no protect should enable for CTS-2-Self, WHQA_00025629*/
	if (MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3))
		if (pAd->CommonCfg.dbdc_mode && op_mode == NON_PROTECT)	{
			op_mode = BW20_PROTECT;
			new_protection |= SET_PROTECT(HT20_PROTECT);
		}

#if defined(MT_MAC)
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((new_protection & wdev->protection) != new_protection) {
			wdev->protection = new_protection;

			AsicUpdateProtect(pAd);
		}

	}
#endif
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR Apidx)
{
	BOOLEAN Result = TRUE;
#ifdef ACL_BLK_COUNT_SUPPORT
		ULONG idx;
#endif
	if (Apidx >= HW_BEACON_MAX_NUM)
		return FALSE;

#ifdef CUSTOMER_DCC_FEATURE
	if (pAd->ApDisableSTAConnectFlag == TRUE) {
		INT	count, i;
		UINT32	time;

		Result = FALSE;
		time = jiffies_to_msecs(jiffies);
		count = pAd->AllowedStaList.StaCount;
		for (i = 0; i < count; i++) {
			if (NdisEqualMemory(&(pAd->AllowedStaList.AllowedSta[i].MacAddr[0]), pAddr, MAC_ADDR_LEN)) {
				if ((time - pAd->AllowedStaList.AllowedSta[i].DissocTime) < 30000) {
					Result = TRUE;
				}
				break;
			}
		}

		if (!Result)
			return Result;
	}
#endif

	if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
		Result = TRUE;
	else {
		ULONG i;

		if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
			Result = FALSE;
		else                                              /* ACL is a negative list */
			Result = TRUE;

		for (i = 0; i < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++) {
			if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr)) {
				Result = !Result;
				break;
			}
		}
	}
#ifdef ACL_BLK_COUNT_SUPPORT
			if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy != 2) {
				for (idx = 0; idx < pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; idx++)
					(pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[idx].Reject_Count) = 0;
			}
#endif

	if (Result == FALSE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x failed in ACL checking\n",
				 PRINT_MAC(pAddr)));
	}

	return Result;
}


/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;
	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	USHORT      Reason;
	MAC_TABLE_ENTRY *pEntry;
	BSS_STRUCT *pMbss;
	BOOLEAN drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));

	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApUpdateAccessControlList : Apidx = %d\n", Apidx));
	/* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];

	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, MacIdx); MacIdx++) {
		pEntry = &pAd->MacTab.Content[MacIdx];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->func_tb_idx != Apidx)
			continue;

		drop = FALSE;
		Matched = FALSE;

		for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++) {
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr)) {
				Matched = TRUE;
				break;
			}
		}

		if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1)) {
			drop = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA not on positive ACL. remove it...\n"));
		} else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2)) {
			drop = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA on negative ACL. remove it...\n"));
		}

		if (drop == TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Apidx = %d\n", Apidx));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
					 pMbss->AccessControlList.Policy));

			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC) {
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

				if (NStatus != NDIS_STATUS_SUCCESS) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					return;
				}

				Reason = REASON_DECLINED;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x\n",
						 Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
								 pEntry->Addr,
								 pMbss->wdev.if_addr,
								 pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pOutBuffer);
				RtmpusecDelay(5000);
			}
#ifdef WIFI_DIAG
			if (pEntry && IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr, DIAG_CONN_ACL_BLK, 0);
#endif
			mac_entry_delete(pAd, pEntry);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel
				range given by following equation:
											 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
											      2                          2
					Where
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting
				 Boolean value.
				=>When the value of OPi is the empty set, then the expression (P == OPi for all values of i)
					is defined to be TRUE
				=>When the value of OTi is the empty set, then the expression (P == OTi for all values of i)
					is defined to be TRUE
				=>When the value of OSi is the empty set, then the expression (S == OSi for all values of i)
					is defined to be TRUE
*/
INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE * pCoexChRange,
	IN UCHAR Channel)
{
	INT index, cntrCh = 0;
	UCHAR op_ext_cha = wlan_operate_get_ext_cha(wdev);

	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));
	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT

	if (Channel > 14) {
		/* For 5GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = pChCtrl->ChList[index].Channel;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			} else {
				pCoexChRange->effectChStart = pCoexChRange->primaryCh - 4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
					  index,
					  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
					  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
					  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
					  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
					  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel));
			return TRUE;
		}

		/* It should not happened! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n",
				 Channel));
	} else
#endif /* A_BAND_SUPPORT */
	{	/* For 2.4GHz band */
		for (index = 0; index < pChCtrl->ChListNum; index++) {
			if (pChCtrl->ChList[index].Channel == Channel)
				break;
		}

		if (index < pChCtrl->ChListNum) {
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;

			/* Now check about the secondary and central channel */
			if (op_ext_cha == EXTCHA_ABOVE) {
				if ((index + 4) < pChCtrl->ChListNum) {
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			} else {
				if ((index - 4) >= 0) {
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh) {
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd = (cntrCh + 5) < pChCtrl->ChListNum ?
					(cntrCh + 5) : (pChCtrl->ChListNum - 1);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n",
						  index,
						  ((op_ext_cha == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"),
						  pCoexChRange->primaryCh, pChCtrl->ChList[pCoexChRange->primaryCh].Channel,
						  pCoexChRange->secondaryCh, pChCtrl->ChList[pCoexChRange->secondaryCh].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChStart].Channel,
						  pChCtrl->ChList[pCoexChRange->effectChEnd].Channel));
			}

			return TRUE;
		}

		/* It should not happened! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n",
				  index, pChCtrl->ChListNum, Channel));
	}

	return FALSE;
}

#ifndef RT_CFG80211_SUPPORT
static INT32 wlan_update_rf_ht_bw(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR ht_bw, UCHAR ext_cha)
{
	UINT32 idx;
	BSS_STRUCT *pCurMbss = NULL;

	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		pCurMbss = &pAd->ApCfg.MBSSID[idx];

		/* MBSS is up */
		if (!pCurMbss->wdev.if_up_down_state)
			continue;

		/* MBSS works on the same channel as wdev */
		if (pCurMbss->wdev.channel == wdev->channel) {
			wlan_operate_set_prim_ch(&pCurMbss->wdev, pCurMbss->wdev.channel);
			wlan_operate_set_ht_bw(&pCurMbss->wdev, ht_bw, ext_cha);
		}
	}

	return 0;
}
#endif

VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	BOOLEAN needFallBack = FALSE;
	INT chStartIdx, chEndIdx, index, curPriChIdx, curSecChIdx;
	BSS_COEX_CH_RANGE  coexChRange;
	UCHAR PhyMode = wdev->PhyMode;
	UCHAR Channel = wdev->channel;
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (!WMODE_CAP_2G(PhyMode))
		return;

	/* We just care BSS who operating in 40MHz N Mode. */
	if ((!WMODE_CAP_N(PhyMode)) ||
		(ht_bw == BW_20)
	   ) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The wdev->PhyMode=%d, BW=%d, didn't need channel adjustment!\n", PhyMode, ht_bw));
		return;
	}

	/* Build the effected channel list, if something wrong, return directly. */
	/* For 2.4GHz band */
	for (index = 0; index < pChCtrl->ChListNum; index++) {
		if (pChCtrl->ChList[index].Channel == Channel)
			break;
	}

	if (index < pChCtrl->ChListNum) {
		if (ext_cha == EXTCHA_ABOVE) {
			curPriChIdx = index;
			curSecChIdx = ((index + 4) < pChCtrl->ChListNum) ? (index + 4) : (pChCtrl->ChListNum - 1);
			chStartIdx = (curPriChIdx >= 3) ? (curPriChIdx - 3) : 0;
			chEndIdx = ((curSecChIdx + 3) < pChCtrl->ChListNum) ? (curSecChIdx + 3) :
					   (pChCtrl->ChListNum - 1);
		} else {
			curPriChIdx = index;
			curSecChIdx = ((index - 4) >= 0) ? (index - 4) : 0;
			chStartIdx = (curSecChIdx >= 3) ? (curSecChIdx - 3) : 0;
			chEndIdx =  ((curPriChIdx + 3) < pChCtrl->ChListNum) ? (curPriChIdx + 3) :
						(pChCtrl->ChListNum - 1);
		}
	} else {
		/* It should not happened! */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n", Channel));
		return;
	}

	GetBssCoexEffectedChRange(pAd, wdev, &coexChRange, Channel);

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pChCtrl->ChListNum; index++)
		pChCtrl->ChList[index].bEffectedChannel = 0;

	pAd->CommonCfg.BssCoexApCnt = 0;

	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Card still not enable Tx/Rx, enable it now!\n"));
		RTMP_IRQ_ENABLE(pAd);
		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

	RTMPEnableRxTx(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Ready to do passive scanning for Channel[%d] to Channel[%d]!\n",
			 pChCtrl->ChList[chStartIdx].Channel, pChCtrl->ChList[chEndIdx].Channel));
	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;

	for (index = chStartIdx; index <= chEndIdx; index++) {
		Channel = pChCtrl->ChList[index].Channel;
		wlan_operate_scan(wdev, Channel);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AP OBSS SYNC - BBP R4 to 20MHz.l\n"));
		/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		OS_WAIT(300); /* wait for 200 ms at each channel. */
	}

	pAd->CommonCfg.bOverlapScanning = FALSE;

	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n",
				 index, pChCtrl->ChList[index].Channel, pChCtrl->ChList[index].bEffectedChannel));

		if ((pChCtrl->ChList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  &&
			(index != curPriChIdx)) {
			needFallBack = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("needFallBack=TRUE due to OP/OT!\n"));
		}

		if ((pChCtrl->ChList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx)) {
			needFallBack = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("needFallBack=TRUE due to OS!\n"));
		}
	}

	/* If need fallback, now do it. */
	if ((needFallBack == TRUE)
		&& (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr)
	   ) {
#ifdef RT_CFG80211_SUPPORT
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
#else
		wlan_update_rf_ht_bw(pAd, wdev, HT_BW_20, EXTCHA_NONE);
#endif
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
	} else {
		/*restore to original channel*/
		wlan_operate_set_prim_ch(wdev, wdev->channel);
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
	}
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#define DBDC_AP_5G_PEAK_TP	610
#define DBDC_AP_2G_PEAK_TP	270
#define TP_100M_LOWER_BOUND	60
#define TP_100M_UPPER_BOUND	120
#define DBDC_PEAK_TP_PER_THRESHOLD 5

static VOID dynamic_ampdu_efficiency_adjust_all(struct _RTMP_ADAPTER *ad)
{
	EDCA_PARM *edcaparam;
	ULONG per = 0;
	ULONG tx_cnt;
	COUNTER_802_11 *wlan_ct = NULL;
	struct wifi_dev *wdev = NULL;
	UINT16 level = TXOP_80, multi_client_num_th = 0, dbdc_peak_tp = 0;
	UCHAR band_idx = 0;

	for (band_idx = 0; band_idx < DBDC_BAND_NUM; band_idx++) {

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] BandIdx:%d\n", __func__, band_idx));

		if (ad->peak_tp_ctl[band_idx].client_nums == 0)
			goto ignore_ampdu_efficiency_check;

		wlan_ct = &ad->WlanCounters[band_idx];
		wdev = ad->peak_tp_ctl[band_idx].cur_wdev;
		tx_cnt = wlan_ct->AmpduSuccessCount.u.LowPart;

		if (WMODE_CAP_2G(wdev->PhyMode)) {
			multi_client_num_th = MULTI_CLIENT_2G_NUMS_TH;
			dbdc_peak_tp = DBDC_AP_2G_PEAK_TP;
		} else if (WMODE_CAP_5G(wdev->PhyMode)) {
			multi_client_num_th = MULTI_CLIENT_NUMS_TH;
			dbdc_peak_tp = DBDC_AP_5G_PEAK_TP;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s[%d]\n", __func__, __LINE__));
			continue;
		}

		if (ad->txop_ctl[band_idx].multi_client_nums == multi_client_num_th) {
			/* do no apply patch, it is in veriwave multi-client case */
			goto ignore_ampdu_efficiency_check;
		}

		if (tx_cnt > 0)
			per = 100 * (wlan_ct->AmpduFailCount.u.LowPart) / (wlan_ct->AmpduFailCount.u.LowPart + tx_cnt);

		if (per >= DBDC_PEAK_TP_PER_THRESHOLD) {
			/* do no apply patch, it is in noise environment */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s[%d]per=%lu\n", __func__, __LINE__, per));
			goto ignore_ampdu_efficiency_check;
		}

		/* scenario detection */
		if ((ad->CommonCfg.dbdc_mode) &&
			(ad->peak_tp_ctl[band_idx].max_tx_tp > dbdc_peak_tp)) {
			ad->peak_tp_ctl[band_idx].cli_peak_tp_running = 1;
			level = TXOP_FE;

			if (level != ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_level)
				ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable = FALSE;
		} else if ((ad->peak_tp_ctl[band_idx].max_tx_tp > TP_100M_LOWER_BOUND) &&
				   (ad->peak_tp_ctl[band_idx].max_tx_tp < TP_100M_UPPER_BOUND)) {
			ad->peak_tp_ctl[band_idx].cli_peak_tp_running = 1;
			level = TXOP_60;

			if (level != ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_level)
				ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable = FALSE;
		} else
			ad->peak_tp_ctl[band_idx].cli_peak_tp_running = 0;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s[%d]per=%lu, tx=%d M, (%d, %d, %d)\n\r",
				 __func__, __LINE__, per, ad->peak_tp_ctl[band_idx].max_tx_tp,
				 ad->peak_tp_ctl[band_idx].cli_peak_tp_running,
				 ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running,
				 ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable));

		/* increase ampdu efficiency if running peak T.P */
		if (ad->peak_tp_ctl[band_idx].cli_peak_tp_running) {
			if (!ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running) {
				if (query_tx_burst_prio(ad, wdev) <= PRIO_PEAK_TP) {
					AsicAmpduEfficiencyAdjust(wdev, 0xf);
					ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running = TRUE;
				}
			}

			if (!ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable) {
				enable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
				ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_level = level;
				ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable = TRUE;
			}
		} else {
			/* restore to original */
			if (ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running) {
				edcaparam = HcGetEdca(ad, wdev);

				if (edcaparam)
					AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

				ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running = FALSE;
			}

			if (ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable) {
				disable_tx_burst(ad, wdev, AC_BE, PRIO_PEAK_TP, level);
				ad->peak_tp_ctl[band_idx].cli_peak_tp_txop_enable = FALSE;
			}
		}

ignore_ampdu_efficiency_check:

		/* restore aifs adjust since dynamic txop owner is not peak throughput */
		if (ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running) {
			if (query_tx_burst_prio(ad, wdev) > PRIO_PEAK_TP) {
				edcaparam = HcGetEdca(ad, wdev);

				if (edcaparam)
					AsicAmpduEfficiencyAdjust(wdev, edcaparam->Aifsn[0]);

				ad->peak_tp_ctl[band_idx].cli_ampdu_efficiency_running = FALSE;
			}
		}

		/* clear some record */
		ad->peak_tp_ctl[band_idx].client_nums = 0;
		ad->peak_tp_ctl[band_idx].max_tx_tp = 0;
	}
}

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
	IN  PRTMP_ADAPTER	pAd,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN UINT8 cmd)
{
	INT				apidx = MAIN_MBSSID;
	UCHAR			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
#ifdef RADIUS_ACCOUNTING_SUPPORT
	DOT1X_QUERY_STA_DATA	data;
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN + sizeof(DOT1X_QUERY_STA_DATA))
#else
#define FRAME_BUF_LEN	(LENGTH_802_3 + sizeof(RalinkIe) + VLAN_HDR_LEN)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
	UINT8			frame_len = 0;
	UCHAR			FrameBuf[FRAME_BUF_LEN];
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;
	/* Init the frame buffer */
	NdisZeroMemory(FrameBuf, frame_len);
#ifdef RADIUS_ACCOUNTING_SUPPORT
	NdisMoveMemory(data.StaAddr, pEntry->Addr, MAC_ADDR_LEN);
	data.rx_bytes = pEntry->RxBytes;
	data.tx_bytes = pEntry->TxBytes;
	data.rx_packets = pEntry->RxPackets.u.LowPart;
	data.tx_packets = pEntry->TxPackets.u.LowPart;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::rx_byte:%lu  tx_byte:%lu rx_pkt:%lu tx_pkt:%lu\n",
			 __func__, data.rx_bytes, data.tx_bytes, data.rx_packets, data.rx_packets));
#endif /* RADIUS_ACCOUNTING_SUPPORT */

	if (pEntry) {
		apidx = pEntry->func_tb_idx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	} else {
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
		bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
		bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
		frame_len = FRAME_BUF_LEN;
		MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, s_addr, EAPOL_IE);
		offset += LENGTH_802_3 - 2;
		NdisMoveMemory((FrameBuf + offset), TPID, 2);
		offset += 2;
		TCI = (bss_Vlan & 0x0FFF) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef RT_BIG_ENDIAN
		TCI = SWAP16(TCI);
#endif
		*(USHORT *)(FrameBuf + offset) = TCI;
		offset += 2;
		NdisMoveMemory((FrameBuf + offset), EAPOL_IE, 2);
		offset += 2;
	} else {
		frame_len = FRAME_BUF_LEN - VLAN_HDR_LEN;
		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(FrameBuf,
						  pAd->ApCfg.MBSSID[apidx].wdev.bssid,
						  s_addr,
						  EAPOL_IE);
		offset += LENGTH_802_3;
	}

	/* Prepare the specific header of internal command */
	NdisMoveMemory(&FrameBuf[offset], RalinkIe, sizeof(RalinkIe));
#ifdef RADIUS_ACCOUNTING_SUPPORT
	offset += sizeof(RalinkIe);
	/*add accounting info*/
	NdisMoveMemory(&FrameBuf[offset], (unsigned char *)&data, sizeof(DOT1X_QUERY_STA_DATA));
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

	/* Report to upper layer */
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
		return FALSE;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s done. (cmd=%d)\n", __func__, cmd));
	return TRUE;
}


/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	/* TODO: shiang-usw, fix me for pEntry->apidx to func_tb_idx */
	INT				apidx = MAIN_MBSSID;
	UCHAR			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			FrameBuf[frame_len + 32];
	UINT8			offset = 0;
	USHORT			bss_Vlan_Priority = 0;
	USHORT			bss_Vlan;
	USHORT			TCI;

	if (IS_AKM_1X_Entry(pEntry) || IS_IEEE8021X_Entry(&pAd->ApCfg.MBSSID[apidx].wdev)) {
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);
		/* Assign apidx */
		apidx = pEntry->func_tb_idx;

		if (pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID) {
			/*Prepare 802.3 header including VLAN tag*/
			bss_Vlan = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_VID;
			bss_Vlan_Priority = pAd->ApCfg.MBSSID[apidx].wdev.VLAN_Priority;
			frame_len += VLAN_HDR_LEN;
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL)
			offset += LENGTH_802_3 - 2;
			NdisMoveMemory((FrameBuf + offset), TPID, 2);
			offset += 2;
			TCI = (bss_Vlan & 0x0fff) | ((bss_Vlan_Priority & 0x7) << 13);
#ifndef RT_BIG_ENDIAN
			TCI = SWAP16(TCI);
#endif
			*(USHORT *)(FrameBuf + offset) = TCI;
			offset += 2;
			NdisMoveMemory((FrameBuf + offset), EAPOL, 2);
			offset += 2;
		} else {
			/* Prepare the 802.3 header */
			MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
			offset += LENGTH_802_3;
		}

		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));
#ifdef CONFIG_HOTSPOT_R2

		if (pEntry) {
			BSS_STRUCT *pMbss = pEntry->pMbss;

			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (IS_AKM_WPA2_Entry(&pMbss->wdev)) &&
				(pEntry->hs_info.ppsmo_exist == 1)) {
				UCHAR HS2_Header[4] = {0x50, 0x6f, 0x9a, 0x12};

				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4 + sizeof(struct _sta_hs_info);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("DOT1X_EapTriggerAction: event eapol start, %x:%x:%x:%x\n",
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 4], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 5],
						 FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 6], FrameBuf[offset + sizeof(eapol_start_1x_hdr) + 7]));
			}
		}

#endif

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Notify 8021.x daemon to trigger EAP-SM for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));
	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */


INT rtmp_ap_init(RTMP_ADAPTER *pAd)
{
#ifdef WSC_AP_SUPPORT
	UCHAR j;
	BSS_STRUCT *mbss = NULL;
	struct wifi_dev *wdev = NULL;
	PWSC_CTRL pWscControl;

	for (j = BSS0; j < pAd->ApCfg.BssidNum; j++) {
		mbss = &pAd->ApCfg.MBSSID[j];
		wdev = &pAd->ApCfg.MBSSID[j].wdev;
		{
			pWscControl = &wdev->WscControl;
			pWscControl->WscRxBufLen = 0;
			pWscControl->pWscRxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscRxBuf)
				NdisZeroMemory(pWscControl->pWscRxBuf, MAX_MGMT_PKT_LEN);

			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscTxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);

			if (pWscControl->pWscTxBuf)
				NdisZeroMemory(pWscControl->pWscTxBuf, MAX_MGMT_PKT_LEN);
		}
	}

#endif /* WSC_AP_SUPPORT */
	APOneShotSettingInitialize(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("apstart up %02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(pAd->CurrentAddress)));
	APInitForMain(pAd);

	/* Set up the Mac address*/
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
	/* ap_func_init(pAd); */

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		/* Now Enable MacRxTx*/
		RTMP_IRQ_ENABLE(pAd);
		RTMPEnableRxTx(pAd);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}

#ifdef CONFIG_INIT_RADIO_ONOFF
	if(!pAd->ApCfg.bRadioOn)
	{
#ifdef FT_R1KH_KEEP
		pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif /* FT_R1KH_KEEP */
		APStop(pAd, &pAd->ApCfg.MBSSID[BSS0], AP_BSS_OPER_BY_RF);
		MlmeRadioOff(pAd, &pAd->ApCfg.MBSSID[BSS0].wdev);
	}
#endif

#ifdef CFG_SUPPORT_MU_MIMO_RA
	/*Send In-Band Command to N9 about Platform Type 7621/7623*/
	SetMuraPlatformTypeProc(pAd);
	SetMuraEnableHwSwPatch(pAd);
#endif

#ifdef FAST_UP_RATE_SUPPORT
	MtCmdSetFastUpRate(pAd, TRUE);
#endif

#ifdef MT_FDB
	fdb_enable(pAd);
#endif /* MT_FDB */
	return NDIS_STATUS_SUCCESS;
}


VOID rtmp_ap_exit(RTMP_ADAPTER *pAd)
{
	BOOLEAN Cancelled;

	RTMPReleaseTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPReleaseTimer(&pAd->ApCfg.IDSTimer, &Cancelled);
#endif /* IDS_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	RTMP_11N_D3_TimerRelease(pAd);
#endif /*DOT11N_DRAFT3*/
#endif /*DOT11_N_SUPPORT*/
	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);
#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */
}

/*
* system security decision for ap mode
*/
UINT32 starec_ap_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *) wdev->sys_handle;
	UINT32 features = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);

	if (cap->APPSMode == APPS_MODE2)
		features |= STA_REC_AP_PS_FEATURE;

	/*temport used for security will integrate to CMD*/
	if (IS_CIPHER_WEP(entry->SecConfig.PairwiseCipher))
		features |= STA_REC_INSTALL_KEY_FEATURE;

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}


/*system for ap mode*/
/*
*
*/
INT ap_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		wifi_sys_linkup(wdev, NULL);
		APStartRekeyTimer(wdev->sys_handle, wdev);
	}

	OPSTATUS_SET_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	return TRUE;
}

/*
*
*/
INT ap_link_down(struct wifi_dev *wdev)
{
	APStopRekeyTimer(wdev->sys_handle, wdev);

	if (WDEV_BSS_STATE(wdev) >= BSS_INITED) {
		/* kick out all stas behind the Bss */
		MbssKickOutStas(wdev->sys_handle, wdev->func_idx, REASON_DISASSOC_INACTIVE);
		UpdateBeaconHandler(
			wdev->sys_handle,
			wdev,
			BCN_UPDATE_DISABLE_TX);

		/*linkdown bssinfo*/
		if (wifi_sys_linkdown(wdev) != TRUE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): linkdown fail!\n", __func__));
			return FALSE;
		}
	}

	return TRUE;
}

/*
*
*/
INT ap_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	/*generic connection action*/
	if (wifi_sys_conn_act(wdev, entry) != TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): connect action fail!\n", __func__));
	}

#if defined(MT7615) || defined(MT7637) || defined(MT7622) || defined(P18) || defined(MT7663)

	if (IS_MT7615(ad) || IS_MT7637(ad) || IS_MT7622(ad) || IS_P18(ad) || IS_MT7663(ad))
		RAInit(ad, entry);

#endif /* defined(MT7615) || defined(MT7637) || defined(MT7622) */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) && defined(TXBF_SUPPORT) && defined(MT7615)

	/* Disable TXBF apply for DFS zero wait start */
	if ((ad->BgndScanCtrl.BgndScanStatMachine.CurrState == BGND_RDD_DETEC)
		&& IS_SUPPORT_MT_ZEROWAIT_DFS(ad)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("[%s][ZeroWait]In CAC period, Disable TXBF Apply\n", __func__));
		HW_AP_TXBF_TX_APPLY(ad, FALSE);
	}

#endif
	return TRUE;
}

/*
*
*/
INT ap_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef BACKGROUND_SCAN_SUPPORT
	UINT32	CrValue;
#endif /* BACKGROUND_SCAN_SUPPORT */


#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */
#ifdef CONFIG_INIT_RADIO_ONOFF
	if (pAd->ApCfg.bRadioOn) {
#endif

	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AP inf up for ra_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx));

	MlmeRadioOn(pAd, wdev);
#ifdef CONFIG_INIT_RADIO_ONOFF
	}
#endif

	/* action for ap interface up */
	pAd->ApCfg.MBSSID[wdev->func_idx].mbss_idx = wdev->func_idx;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pAd->ApCfg.MBSSID[wdev->func_idx].APStartPseduState != AP_STATE_ALWAYS_START_AP_DEFAULT) {
		wdev->bAllowBeaconing = FALSE;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Disable Beaconing\n", __func__));
	} else {
		wdev->bAllowBeaconing = TRUE;
	}
#else
	wdev->bAllowBeaconing = TRUE;
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */
	ap_run_at_boot(pAd, wdev);
#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanInit(pAd);
	/* Enable band0 IPI SW control */
	HW_IO_READ32(pAd, PHY_RXTD_12, &CrValue);
	CrValue |= (1 << B0IrpiSwCtrlResetOffset);
	CrValue |= (1 << B0IrpiSwCtrlOnlyOffset);
	HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);
	HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);
	/* Enable badn0 IPI control */
	HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5, &CrValue);
	CrValue |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);
	HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5, CrValue);
#endif /* BACKGROUND_SCAN_SUPPORT */

	APStartUpForMbss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
	/* Logic to perform OBSS scan for 2.4G only and
	 * one time for all  MBSS configured to same channel.
	 */
	{
	BSS_STRUCT *pMbss = NULL;

	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	/*
	* GBandChanBitMap is used to store the 2.4Ghz channel for which
	* BSS overlap scan is done.
	* Purpose: In case of MBSS, to avoid repeated
	* overlapped scan for the same channel.
	* No need for obss scan if RF up is on 5Ghz
	*/
	if (pMbss && (pMbss->wdev.channel < 14)) {
		if (!(pAd->ApCfg.ObssGBandChanBitMap & (1 << pMbss->wdev.channel))) {
			pAd->ApCfg.ObssGBandChanBitMap |= (1 << pMbss->wdev.channel);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s] wdev->func_idx:%d channel:%d ApCfg.BssidNum:%d \
			 wdev.if_up_down_state:%d GBandChanBitMap:0x%x\n",
			__func__, wdev->func_idx, wdev->channel,
			pAd->ApCfg.BssidNum, pMbss->wdev.if_up_down_state,
			pAd->ApCfg.ObssGBandChanBitMap));

			if (pMbss->wdev.if_up_down_state) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("[%s] calling ap_over_lapping_scan\n", __func__));
				ap_over_lapping_scan(pAd, pMbss);
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s] 2.4G Band CHAN already Set:: GBandChanBitMap:0x%x pMbss->wdev.channel:%d\n",
				__func__, pAd->ApCfg.ObssGBandChanBitMap, pMbss->wdev.channel));
		}
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s] CHAN is of 5G Band: pMbss->wdev.channel:%d\n",
			 __func__, pMbss->wdev.channel));
	}
	}

#ifdef WSC_INCLUDED
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("apidx %d for WscUUIDInit\n", wdev->func_idx));
	WscUUIDInit(pAd, wdev->func_idx, FALSE);
#endif /* WSC_INCLUDED */


	return TRUE;
}

/*
*
*/
INT ap_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;


	if (pAd == NULL)
		return FALSE;

#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */

	/* Move orig RTMPInfClose here */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef APCLI_CFG80211_SUPPORT
		int j;
#endif /* APCLI_CFG80211_SUPPORT */
		wdev->bAllowBeaconing = FALSE;

		if (wdev_do_linkdown(wdev) != TRUE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() linkdown fail!!!\n", __func__));

		if (wifi_sys_close(wdev) != TRUE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));
			return FALSE;
		}
#ifdef APCLI_CFG80211_SUPPORT
		for (j = 0; j < MAX_APCLI_NUM; j++) {
			APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[j];

			if (apcli_entry->wpa_supplicant_info.pWpsProbeReqIe) {
				os_free_mem(apcli_entry->wpa_supplicant_info.pWpsProbeReqIe);
				apcli_entry->wpa_supplicant_info.pWpsProbeReqIe = NULL;
				apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen = 0;
			}

			if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
				os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
				apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
				apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			}
		}
#endif /* APCLI_CFG80211_SUPPORT */

#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering) {
			PBND_STRG_CLI_TABLE table;

			table = Get_BndStrgTable(pAd, BSS0);
			if (table) {
				/* Inform daemon interface down */
				BndStrg_SetInfFlags(pAd, wdev, table, FALSE);
			}
		}
#endif /* BAND_STEERING */
	}


	return TRUE;
}

BOOLEAN media_state_connected(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad;

	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	return (OPSTATUS_TEST_FLAG(ad, fOP_STATUS_MEDIA_STATE_CONNECTED)
			&& OPSTATUS_TEST_FLAG(ad, fOP_AP_STATUS_MEDIA_STATE_CONNECTED));
}

