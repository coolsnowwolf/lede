/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_radar.c

    Abstract:
    CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#include "rt_config.h"
#include "hdev/hdev.h"
#include "wlan_config/config_internal.h"

/*----- 802.11H -----*/
/*
	========================================================================

	Routine Description:
		Radar channel check routine

	Arguments:
		pAd	Pointer to our adapter

	Return Value:
		TRUE	need to do radar detect
		FALSE	need not to do radar detect

	========================================================================
*/
BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	INT	i;
	BOOLEAN result = FALSE;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;

	/* Non-Zero Channel & Null Pointer Check */
	if (Ch && pAd) {
		for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
			for (i = 0; i < pChCtrl->ChListNum; i++) {
				if (Ch == pChCtrl->ChList[i].Channel) {
					result = pChCtrl->ChList[i].DfsReq;
					break;
				}
			}
		}
	}
	return result;
}


/*
	========================================================================

	Routine Description:
		Determine the current radar state

	Arguments:
		pAd	Pointer to our adapter

	Return Value:

	========================================================================
*/
VOID RadarStateCheck(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
	struct wlan_config *cfg = NULL;
	UCHAR phy_bw = 0;
	UCHAR vht_cent2 = 0;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;

	cfg = (struct wlan_config *)wdev->wpf_cfg;

	if (cfg == NULL)
		return;

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
		else
			;
	}
	vht_cent2 = cfg->phy_conf.cen_ch_2;

	if (wdev->csa_count != 0)
		return;

#ifdef MT_DFS_SUPPORT
	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		DfsRadarChannelCheck(pAd, wdev, vht_cent2, phy_bw)
#ifdef BACKGROUND_SCAN_SUPPORT
		&& ((IS_SUPPORT_MT_ZEROWAIT_DFS(pAd) == FALSE)	|| (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT)))
#endif /* BACKGROUND_SCAN_SUPPORT */
	) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_SILENCE_MODE \x1b[m \n", __func__));

		pDot11h->RDMode = RD_SILENCE_MODE;
		pDot11h->RDCount = 0;
		pDot11h->InServiceMonitorCount = 0;
		if (DfsIsOutBandAvailable(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] OutBand Available. Set into RD_NORMAL_MODE \x1b[m \n", __func__));
			pDot11h->RDMode = RD_NORMAL_MODE;
		} else if (DfsIsTargetChAvailable(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Target Channel Bypass CAC. Set into RD_NORMAL_MODE \x1b[m \n", __FUNCTION__));
			pDot11h->RDMode = RD_NORMAL_MODE;

		} else
			;
	} else
#endif
	{
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] RD_NORMAL_MODE \x1b[m \n", __func__));
		/* DFS Zero wait case, OP CH always is normal mode */
		pDot11h->RDMode = RD_NORMAL_MODE;
	}
}

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR ch)
{
	INT i;
	BOOLEAN InNOP = FALSE;
	UCHAR channel = 0;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (ch == RDD_CHECK_NOP_BY_WDEV)
		channel = wdev->channel;
	else
		channel = ch;

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	for (i = 0; i < pChCtrl->ChListNum; i++) {
		if (pChCtrl->ChList[i].Channel == channel) {
			if (pChCtrl->ChList[i].RemainingTimeForUse > 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("ERROR: previous detection of a radar on this channel(Channel=%d).\n",
						  pChCtrl->ChList[i].Channel));
				InNOP = TRUE;
				break;
			}
		}
	}

	if ((InNOP == FALSE)
#ifdef MT_DFS_SUPPORT
		|| DfsStopWifiCheck(pAd)
#endif
	)
		return TRUE;
	else
		return FALSE;
}

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd)
{
	ULONG		i;
	const UCHAR	Channel[15] = {52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
	BOOLEAN IsSupport5G = HcIsRfSupport(pAd, RFIC_5GHZ);
	UCHAR Channel5G = HcGetChannelByRf(pAd, RFIC_5GHZ);

	if (pAd->CommonCfg.RDDurRegion != JAP)
		return pAd->CommonCfg.RDDurRegion;

	for (i = 0; i < 15; i++) {
		if (IsSupport5G && Channel5G ==  Channel[i])
			break;
	}

	if (i < 4)
		return JAP_W53;
	else if (i < 15)
		return JAP_W56;
	else
		return JAP; /* W52*/
}


UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode,
	IN struct wifi_dev *wdev)
{
	UCHAR ch = 0;
	INT ch_idx;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

#ifdef MT_DFS_SUPPORT
	DfsNonOccupancyUpdate(pAd);
#endif

	switch (mode) {
	case 1: {
		USHORT min_time = 0xFFFF;

		/* select channel with least RemainingTimeForUse */
		for (ch_idx = 0; ch_idx <  pChCtrl->ChListNum; ch_idx++) {
			if (pChCtrl->ChList[ch_idx].RemainingTimeForUse < min_time) {
				min_time = pChCtrl->ChList[ch_idx].RemainingTimeForUse;
				ch = pChCtrl->ChList[ch_idx].Channel;
			}
		}

		break;
	}

	default: {
		ch = FirstChannel(pAd, wdev);
		break;
	}
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): mode = %u, ch = %u\n",
			 __func__, mode, ch));
	return ch;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Channel switching count down process upon radar detection

	Arguments:
		pAd	Pointer to our adapter

	========================================================================
*/
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev)
{
	UCHAR apIdx = 0xff;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s(): Wdev(%d) Channel Switching...(%d/%d)\n",
			 __func__, wdev->wdev_idx, pDot11h->CSCount, pDot11h->CSPeriod));
	pDot11h->CSCount++;

	if (pDot11h->CSCount >= pDot11h->CSPeriod) {
		if (wdev && (wdev->wdev_type == WDEV_TYPE_AP))
			apIdx = wdev->func_idx;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("  Type = %d, func_idx = %d\n",
				 wdev->wdev_type, wdev->func_idx));

		RTEnqueueInternalCmd(pAd, CMDTHRED_DOT11H_SWITCH_CHANNEL, &apIdx, sizeof(UCHAR));
	}
}

#ifdef CUSTOMER_DCC_FEATURE
VOID ChannelSwitchingCountDownProcNew(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev)
{
	UCHAR apIdx = 0xff;

	if (wdev == NULL)
		return;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Channel Switching...(%d/%d)\n", __func__,
			pAd->CommonCfg.channelSwitch.CHSWCount, pAd->CommonCfg.channelSwitch.CHSWPeriod));

	pAd->CommonCfg.channelSwitch.CHSWCount++;
	if (pAd->CommonCfg.channelSwitch.CHSWCount >= pAd->CommonCfg.channelSwitch.CHSWPeriod) {

		if (wdev && (wdev->wdev_type == WDEV_TYPE_AP))
			apIdx = wdev->func_idx;

		pAd->CommonCfg.channelSwitch.CHSWMode = NORMAL_MODE;
		MlmeEnqueue(pAd, AP_SYNC_STATE_MACHINE, APMT2_CHANNEL_SWITCH, sizeof(apIdx), &apIdx, 0);
		RTMP_MLME_HANDLER(pAd);
	}
}
#endif
void update_ch_by_wdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR apIdx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR apOper = AP_BSS_OPER_ALL;
	struct DOT11_H *pDot11h = NULL;
#ifdef OFFCHANNEL_SCAN_FEATURE
	OFFCHANNEL_SCAN_MSG Rsp;
	UCHAR RfIC = 0;

	Rsp.Action = DRIVER_CHANNEL_SWITCH_SUCCESSFUL;
	memcpy(Rsp.ifrn_name, pAd->ScanCtrl.if_name, IFNAMSIZ);
#endif
	NdisMoveMemory(&apIdx, CMDQelmt->buffer, sizeof(UCHAR));

	/* check apidx valid */
	if (apIdx != 0xff) {
		pMbss = &pAd->ApCfg.MBSSID[apIdx];
		/* This event comes on every bss of the RF */
		apOper = AP_BSS_OPER_SINGLE;
	}

	if (pMbss == NULL)
		return 0;
	pDot11h = pMbss->wdev.pDot11_H;
	if (pDot11h == NULL)
		return 0;
		/* Normal DFS */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] 2 \x1b[m \n", __func__));
#ifdef ONDEMAND_DFS
#ifdef OFFCHANNEL_SCAN_FEATURE
		if (pAd->radar_hit == TRUE) {
			printk("[%s] Radar detected  \n", __func__);
			Rsp.Action = DFS_DRIVER_CHANNEL_SWITCH;
			pAd->radar_hit = FALSE;
		}
#endif
		if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd))
			DedicatedZeroWaitStop(pAd, TRUE);
		else
#endif
			DedicatedZeroWaitStop(pAd, FALSE);
#endif
		pDot11h->RDMode = RD_SILENCE_MODE;
#ifdef CONFIG_MAP_SUPPORT
		if (pMbss->wdev.quick_ch_change == TRUE && !RadarChannelCheck(pAd, pMbss->wdev.channel)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s %d\n",
							(char *)pMbss->wdev.if_dev->name,
							pMbss->wdev.quick_ch_change));
			update_ch_by_wdev(pAd, &pMbss->wdev);
		} else {
#endif
		APStop(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
		if (DfsStopWifiCheck(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n", __func__));
			return 0;
		}
#endif
		APStartUp(pAd, pMbss, apOper);
#ifdef CONFIG_MAP_SUPPORT
		}
#endif
#ifdef MT_DFS_SUPPORT
		if (pAd->CommonCfg.dbdc_mode)
			MtCmdSetDfsTxStart(pAd, HcGetBandByWdev(&pMbss->wdev));
		else
			MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
		DfsSetCacRemainingTime(pAd, &pMbss->wdev);
		DfsReportCollision(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT
		DfsDedicatedScanStart(pAd);
#endif
#endif
#ifdef CONFIG_MAP_SUPPORT
		if (pMbss->wdev.cac_not_required) {
			pMbss->wdev.cac_not_required = FALSE;
			pDot11h->cac_not_required = TRUE;
		}
		if (pDot11h->cac_not_required) {
			int i = 0;
			struct wifi_dev *wdev_temp = NULL;
			BOOLEAN can_reset_cac = TRUE;

			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				wdev_temp = &pAd->ApCfg.MBSSID[i].wdev;
				if ((wdev_temp->pDot11_H == pDot11h) &&
					wdev_temp->cac_not_required) {
					can_reset_cac = FALSE;
					break;
				}
			}
			if (can_reset_cac) {
				pDot11h->RDCount = pDot11h->ChMovingTime;
				pDot11h->cac_not_required = FALSE;
				if (IS_MAP_TURNKEY_ENABLE(pAd)) {
					int j;
					for (j = 0; j < MAX_APCLI_NUM; j++) {
						wdev_temp = &pAd->ApCfg.ApCliTab[j].wdev;
						if (wdev_temp->pDot11_H == pDot11h) {
							pAd->ApCfg.ApCliTab[j].Enable = TRUE;
							break;
						}
					}
				}
			}
		}
#endif
#ifdef OFFCHANNEL_SCAN_FEATURE
		RfIC = (WMODE_CAP_5G(pMbss->wdev.PhyMode)) ? RFIC_5GHZ : RFIC_24GHZ;
		Rsp.data.operating_ch_info.channel = HcGetChannelByRf(pAd, RfIC);
		Rsp.data.operating_ch_info.cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.cfg_vht_bw = wlan_config_get_vht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		Rsp.data.operating_ch_info.RDDurRegion = pAd->CommonCfg.RDDurRegion;
		Rsp.data.operating_ch_info.region = GetCountryRegionFromCountryCode(pAd->CommonCfg.CountryCode);
#ifdef ONDEMAND_DFS
		Rsp.data.operating_ch_info.is4x4Mode = IS_ONDEMAND_DFS_MODE_4x4(pAd);
#else
		Rsp.data.operating_ch_info.is4x4Mode = 1;
#endif
		RtmpOSWrielessEventSend(
				pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_OFFCHANNEL_INFO,
				NULL,
				(UCHAR *) &Rsp,
				sizeof(OFFCHANNEL_SCAN_MSG));
#endif
	return 0;
}

#endif /* CONFIG_AP_SUPPORT */




/*
    ==========================================================================
    Description:
	Set channel switch Period
    Return:
	TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	Set_CSPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING * arg)
{
	pAd->Dot11_H[0].CSPeriod = (USHORT) os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("Set_CSPeriod_Proc::(CSPeriod=%d)\n", pAd->Dot11_H[0].CSPeriod));
	return TRUE;
}

/*
    ==========================================================================
    Description:
		change channel moving time for DFS testing.

	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	USHORT Value;
	Value = (USHORT) os_str_tol(arg, 0, 10);
	pAd->Dot11_H[0].ChMovingTime = Value;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: %d\n", __func__,
			 pAd->Dot11_H[0].ChMovingTime));
	return TRUE;
}


/*
    ==========================================================================
    Description:
		Reset channel block status.
	Arguments:
	    pAd				Pointer to our adapter
	    arg				Not used

    Return Value:
	None

    Note:
	Usage:
	       1.) iwpriv ra0 set ChMovTime=[value]
    ==========================================================================
*/
INT Set_BlockChReset_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	INT i;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_TRACE, ("%s: Reset channel block status.\n", __func__));

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++)
			pChCtrl->ChList[i].RemainingTimeForUse = 0;
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
	Initialize the pDot11H of wdev

    Parameters:

    return:
    ==========================================================================
 */
VOID UpdateDot11hForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN attach)
{
	UCHAR bandIdx = 0;

	if (attach) {
		if (wdev) {
			bandIdx = HcGetBandByWdev(wdev);
			wdev->pDot11_H = &pAd->Dot11_H[bandIdx];
		} else {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): no wdev!\n", __func__));
		}
	} else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Detach wdev=%d_Dot11_H!\n", __func__, wdev->wdev_idx));
		wdev->pDot11_H = NULL;
	}
}

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
INT Set_RadarMinLPN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT16 u2MinLpnUpdate = simple_strtol(arg, 0, 10);

	if (u2MinLpnUpdate <= PB_SIZE) {
		pAd->CommonCfg.DfsParameter.u2FCC_LPN_MIN = u2MinLpnUpdate;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():LPN Update %d \n", __func__, pAd->CommonCfg.DfsParameter.u2FCC_LPN_MIN));
		MtCmdSetFcc5MinLPN(pAd, u2MinLpnUpdate);
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():Invalid LPN value %d, please set in range 0 to %d\n", __func__, u2MinLpnUpdate, PB_SIZE));
	}
	return TRUE;
}

INT Set_RadarThresholdParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	CMD_RDM_RADAR_THRESHOLD_UPDATE_T RadarThreshold = {0};
	PSW_RADAR_TYPE_T prRadarType = NULL;
	INT32 i4Recv = 0;
	UINT32 u2RadarType = 0;
	UINT32 ucRT_ENB = 0, ucRT_STGR = 0;
	UINT32 ucRT_CRPN_MIN = 0, ucRT_CRPN_MAX = 0, ucRT_CRPR_MIN = 0;
	UINT32 ucRT_PW_MIN = 0, ucRT_PW_MAX = 0;
	UINT32 ucRT_CRBN_MIN = 0, ucRT_CRBN_MAX = 0;
	UINT32 ucRT_STGPN_MIN = 0, ucRT_STGPN_MAX = 0, ucRT_STGPR_MIN = 0;
	UINT32 u4RT_PRI_MIN = 0, u4RT_PRI_MAX = 0;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;

	prRadarThresholdParam = &pAd->CommonCfg.DfsParameter.rRadarThresholdParam;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
						&u2RadarType, &ucRT_ENB, &ucRT_STGR, &ucRT_CRPN_MIN,
						&ucRT_CRPN_MAX, &ucRT_CRPR_MIN, &ucRT_PW_MIN, &ucRT_PW_MAX,
						&u4RT_PRI_MIN, &u4RT_PRI_MAX, &ucRT_CRBN_MIN, &ucRT_CRBN_MAX,
						&ucRT_STGPN_MIN, &ucRT_STGPN_MAX, &ucRT_STGPR_MIN);

		if (i4Recv != 15) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
						"RadarType-RT_ENB-RT_STGR-RT_CRPN_MIN-RT_CRPN_MAX-RT_CRPR_MIN-RT_PW_MIN-RT_PW_MAX-"
						"RT_PRI_MIN-RT_PRI_MAX-RT_CRBN_MIN-RT_CRBN_MAX-RT_STGPN_MIN-RT_STGPN_MAX-RT_STGPR_MIN\n"));
			return TRUE;
		}
		if (u2RadarType > RT_NUM) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Radar Type invalid!! Only 0 to %d supported\n", RT_NUM));
			return TRUE;
		}

		memset(&RadarThreshold, 0, sizeof(CMD_RDM_RADAR_THRESHOLD_UPDATE_T));
		RadarThreshold.u2RadarType = u2RadarType;
		RadarThreshold.ucRT_ENB  = ucRT_ENB;
		RadarThreshold.ucRT_STGR = ucRT_STGR;
		RadarThreshold.ucRT_CRPN_MIN =  ucRT_CRPN_MIN;
		RadarThreshold.ucRT_CRPN_MAX = ucRT_CRPN_MAX;
		RadarThreshold.ucRT_CRPR_MIN = ucRT_CRPR_MIN;
		RadarThreshold.ucRT_PW_MIN = ucRT_PW_MIN;
		RadarThreshold.ucRT_PW_MAX = ucRT_PW_MAX;
		RadarThreshold.u4RT_PRI_MIN = u4RT_PRI_MIN;
		RadarThreshold.u4RT_PRI_MAX = u4RT_PRI_MAX;
		RadarThreshold.ucRT_CRBN_MIN = ucRT_CRBN_MIN;
		RadarThreshold.ucRT_CRBN_MAX = ucRT_CRBN_MAX;
		RadarThreshold.ucRT_STGPN_MIN = ucRT_STGPN_MIN;
		RadarThreshold.ucRT_STGPN_MAX = ucRT_STGPN_MAX;
		RadarThreshold.ucRT_STGPR_MIN = ucRT_STGPR_MIN;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():RadarType = %d\n RT_ENB = %d\n RT_STGR = %d\n "
			"RT_CRPN_MIN = %d\n RT_CRPN_MAX = %d\n RT_CRPR_MIN = %d\n "
			"RT_PW_MIN = %d\n RT_PW_MAX =%d\n RT_PRI_MIN = %d\n "
			"RT_PRI_MAX = %d\n RT_CRBN_MIN = %d\n RT_CRBN_MAX = %d\n"
			"RT_STGPN_MIN = %d\n RT_STGPN_MAX = %d\n RT_STGPR_MIN = %d\n ",
			__func__, RadarThreshold.u2RadarType, RadarThreshold.ucRT_ENB, RadarThreshold.ucRT_STGR,
			RadarThreshold.ucRT_CRPN_MIN, RadarThreshold.ucRT_CRPN_MAX, RadarThreshold.ucRT_CRPR_MIN,
			RadarThreshold.ucRT_PW_MIN, RadarThreshold.ucRT_PW_MAX, RadarThreshold.u4RT_PRI_MIN,
			RadarThreshold.u4RT_PRI_MAX, RadarThreshold.ucRT_CRBN_MIN, RadarThreshold.ucRT_CRBN_MAX,
			RadarThreshold.ucRT_STGPN_MIN, RadarThreshold.ucRT_STGPN_MAX, RadarThreshold.ucRT_STGPR_MIN));

		prRadarType = &prRadarThresholdParam->arRadarType[u2RadarType];
		prRadarType->ucRT_ENB = RadarThreshold.ucRT_ENB;
		prRadarType->ucRT_STGR = RadarThreshold.ucRT_STGR;
		prRadarType->ucRT_CRPN_MIN = RadarThreshold.ucRT_CRPN_MIN;
		prRadarType->ucRT_CRPN_MAX = RadarThreshold.ucRT_CRPN_MAX;
		prRadarType->ucRT_CRPR_MIN = RadarThreshold.ucRT_CRPR_MIN;
		prRadarType->ucRT_PW_MIN = RadarThreshold.ucRT_PW_MIN;
		prRadarType->ucRT_PW_MAX = RadarThreshold.ucRT_PW_MAX;
		prRadarType->u4RT_PRI_MIN = RadarThreshold.u4RT_PRI_MIN;
		prRadarType->u4RT_PRI_MAX = RadarThreshold.u4RT_PRI_MAX;
		prRadarType->ucRT_CRBN_MIN = RadarThreshold.ucRT_CRBN_MIN;
		prRadarType->ucRT_CRBN_MAX = RadarThreshold.ucRT_CRBN_MAX;
		prRadarType->ucRT_STGPN_MIN = RadarThreshold.ucRT_STGPN_MIN;
		prRadarType->ucRT_STGPN_MAX = RadarThreshold.ucRT_STGPN_MAX;
		prRadarType->ucRT_STGPR_MIN = RadarThreshold.ucRT_STGPR_MIN;

		MtCmdSetRadarThresholdParam(pAd, &RadarThreshold);
	}

	return TRUE;

}
INT Set_RadarPulseThresholdParam_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0, i4PulsePwrMax = 0, i4PulsePwrMin = 0;
	UINT32 u4PulseWidthMax = 0, u4PRIMinSTGR = 0, u4PRIMaxSTGR = 0;
	UINT32 u4PRIMinCR = 0, u4PRIMaxCR = 0;
	PDFS_PULSE_THRESHOLD_PARAM prPulseThresholdParam = NULL;
	CMD_RDM_PULSE_THRESHOLD_UPDATE_T PulseThresholdUpdate = {0};

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d-%d",
							&u4PulseWidthMax, &i4PulsePwrMax, &i4PulsePwrMin,
							&u4PRIMinSTGR, &u4PRIMaxSTGR, &u4PRIMinCR, &u4PRIMaxCR);

		if (i4Recv != 7) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"MaxPulseWidth-MaxPulsePower-MinPulsePower-"
					"MinPRISTGR-MaxPRISTGR-MinPRICR-MaxPRICR\n"));
			return TRUE;
		}

		PulseThresholdUpdate.u4PP_PulseWidthMAX = u4PulseWidthMax;
		PulseThresholdUpdate.i4PulsePowerMAX = i4PulsePwrMax;
		PulseThresholdUpdate.i4PulsePowerMIN = i4PulsePwrMin;
		PulseThresholdUpdate.u4PRI_MIN_STGR = u4PRIMinSTGR;
		PulseThresholdUpdate.u4PRI_MAX_STGR = u4PRIMaxSTGR;
		PulseThresholdUpdate.u4PRI_MIN_CR = u4PRIMinCR;
		PulseThresholdUpdate.u4PRI_MAX_CR = u4PRIMaxCR;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():MaxPulseWidth = %d\nMaxPulsePower = %d\nMinPulsePower = %d\n"
					"MinPRISTGR = %d\nMaxPRISTGR = %d\nMinPRICR = %d\nMaxPRICR = %d\n",
					__func__, PulseThresholdUpdate.u4PP_PulseWidthMAX, PulseThresholdUpdate.i4PulsePowerMAX,
					PulseThresholdUpdate.i4PulsePowerMIN, PulseThresholdUpdate.u4PRI_MIN_STGR,
					PulseThresholdUpdate.u4PRI_MAX_STGR, PulseThresholdUpdate.u4PRI_MAX_STGR,
					PulseThresholdUpdate.u4PRI_MAX_CR));

		prPulseThresholdParam = &pAd->CommonCfg.DfsParameter.rRadarThresholdParam.rPulseThresholdParam;

		prPulseThresholdParam->u4PulseWidthMax = u4PulseWidthMax;
		prPulseThresholdParam->i4PulsePwrMax = i4PulsePwrMax;
		prPulseThresholdParam->i4PulsePwrMin = i4PulsePwrMin;
		prPulseThresholdParam->u4PRI_MIN_STGR = u4PRIMinSTGR;
		prPulseThresholdParam->u4PRI_MAX_STGR = u4PRIMaxSTGR;
		prPulseThresholdParam->u4PRI_MIN_CR = u4PRIMinCR;
		prPulseThresholdParam->u4PRI_MAX_CR = u4PRIMaxCR;

		MtCmdSetPulseThresholdParam(pAd, &PulseThresholdUpdate);
	}

	return TRUE;

}

INT Set_RadarDbgLogConfig_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	UINT32 ucHwRDDLogEnable = 0;
	UINT32 ucSwRDDLogEnable = 0;
	UINT32 ucSwRDDLogCond = 1;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	if (arg) {
		i4Recv = sscanf(arg, "%d-%d-%d", &ucHwRDDLogEnable, &ucSwRDDLogEnable, &ucSwRDDLogCond);

		if (i4Recv != 3) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("Format Error! Please enter in the following format\n"
					"HWRDD_LOG_ENB-SWRDD_LOG_ENB-SWRDD_LOG_COND\n"));
			return TRUE;
		}

		if (ucHwRDDLogEnable != 0)
			pDfsParam->fgHwRDDLogEnable = TRUE;
		else
			pDfsParam->fgHwRDDLogEnable = FALSE;
		if (ucSwRDDLogEnable != 0)
			pDfsParam->fgSwRDDLogEnable = TRUE;
		else
			pDfsParam->fgSwRDDLogEnable = FALSE;
		if (ucSwRDDLogCond == 0)
			pDfsParam->fgSwRDDLogCond = FALSE;
		else
			pDfsParam->fgSwRDDLogCond = TRUE;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():HWRDD_LOG_ENB = %d, SWRDD_LOG_ENB = %d SWRDD_LOG_COND = %d \n",
				__func__, pDfsParam->fgHwRDDLogEnable, pDfsParam->fgSwRDDLogEnable, pDfsParam->fgSwRDDLogCond));

		MtCmdSetRddLogConfigUpdate(pAd, ucHwRDDLogEnable, ucSwRDDLogEnable, ucSwRDDLogCond);
	}

	return TRUE;

}

INT Show_Radar_Threshold_Param_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg)
{
	UINT8 ucRadarTypeIdx = 0;
	PDFS_RADAR_THRESHOLD_PARAM prRadarThresholdParam = NULL;
	PSW_RADAR_TYPE_T prRadarType = NULL;

	prRadarThresholdParam = &pAd->CommonCfg.DfsParameter.rRadarThresholdParam;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------Debug Log Conditions---------------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("HWRDD_LOG_ENB = %d\nSWRDD_LOG_ENB = %d\nSWRDD_LOG_COND = %d\n",
					pAd->CommonCfg.DfsParameter.fgHwRDDLogEnable,
					pAd->CommonCfg.DfsParameter.fgSwRDDLogEnable,
					pAd->CommonCfg.DfsParameter.fgSwRDDLogCond));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("-------------------------------Pulse Threshold Parameters-----------------------------------\n"));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("FCC5_LPN = %d\n",
					pAd->CommonCfg.DfsParameter.u2FCC_LPN_MIN));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MIN = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMin));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PLS_POWER_MAX = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.i4PulsePwrMax));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("SP_PW_MAX = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PulseWidthMax));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_STGR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_STGR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_STGR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_STGR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MIN_CR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MIN_CR));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF, ("PRI_MAX_CR = %d\n",
														prRadarThresholdParam->rPulseThresholdParam.u4PRI_MAX_CR));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("---------------------------------RADAR Threshold Info---------------------------------------\n"));

	for (ucRadarTypeIdx = 0; ucRadarTypeIdx < RT_NUM; ucRadarTypeIdx++) {
		prRadarType = &prRadarThresholdParam->arRadarType[ucRadarTypeIdx];

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_DFS, DBG_LVL_OFF,
				("RT - %d: ENB = %d, STGR = %d, CRPN_MIN = %d, CRPN_MAX = %d, CRPR_MIN = %d, PW_MIN = %d, PW_MAX = %d,"
					"PRI_MIN = %d, PRI_MAX = %d, CRBN_MIN = %d, CRBN_MAX = %d\n\t"
					"STGPN_MIN = %d, STGPN_MAX = %d, STGPR_MIN = %d\n",
							ucRadarTypeIdx,
							prRadarType->ucRT_ENB,
							prRadarType->ucRT_STGR,
							prRadarType->ucRT_CRPN_MIN,
							prRadarType->ucRT_CRPN_MAX,
							prRadarType->ucRT_CRPR_MIN,
							prRadarType->ucRT_PW_MIN,
							prRadarType->ucRT_PW_MAX,
							prRadarType->u4RT_PRI_MIN,
							prRadarType->u4RT_PRI_MAX,
							prRadarType->ucRT_CRBN_MIN,
							prRadarType->ucRT_CRBN_MAX,
							prRadarType->ucRT_STGPN_MIN,
							prRadarType->ucRT_STGPN_MAX,
							prRadarType->ucRT_STGPR_MIN
							));
	}
	MTWF_LOG(DBG_CAT_AP, CATPROTO_DFS, DBG_LVL_OFF,
		("---------------------------------------------------------------------------------------------\n"));

	return TRUE;
}

#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
INT Set_RadarTestPulsePattern_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 i4Recv = 0;
	CHAR    *pcPulseParams = 0;
	UINT32 u4PulseNum = 0;

	CMD_RDM_TEST_RADAR_PATTERN_T PulsePattern = {0};
	PPERIODIC_PULSE_BUFFER_T prPulseBuffer = NULL;
	/*
	 Ex: 29151901-28-748;29153127-29-760;29154352-29-748;29155577-28-760;29156652-29-751
	*/
	if (arg) {
		for (u4PulseNum = 0, pcPulseParams = rstrtok(arg, ";"); (pcPulseParams != NULL) && (u4PulseNum < PB_SIZE); pcPulseParams = rstrtok(NULL, ";"), u4PulseNum++) {
			prPulseBuffer = &PulsePattern.arPulseBuffer[u4PulseNum];

			i4Recv = sscanf(pcPulseParams, "%d-%hu-%hi", &(prPulseBuffer->u4PeriodicStartTime), &(prPulseBuffer->u2PeriodicPulseWidth), &(prPulseBuffer->i2PeriodicPulsePower));

			if (i4Recv != 3) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Format Error! Please enter in the following format\n"
							"StartTime0-PulseWidth0-PulsePower0;StartTime1-PulseWidth1-PulsePower1;...\n"));
				return TRUE;
			}
		}

		PulsePattern.ucPulseNum = u4PulseNum;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:No of pulses = %d\n", __func__, PulsePattern.ucPulseNum));
		MtCmdSetTestRadarPattern(pAd, &PulsePattern);
	}

	return TRUE;
}
#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */
