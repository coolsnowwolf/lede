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
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if (Ch == pChCtrl->ChList[i].Channel) {
				result = pChCtrl->ChList[i].DfsReq;
				break;
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
	IN struct wifi_dev *wdev)
{
	INT i;
	BOOLEAN InNOP = FALSE;
	UCHAR channel = wdev->channel;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

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

/*
*
*/
NTSTATUS Dot11HCntDownTimeoutAction(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	UCHAR apIdx;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	UCHAR apOper = AP_BSS_OPER_ALL;
	struct DOT11_H *pDot11h = NULL;

	NdisMoveMemory(&apIdx, CMDQelmt->buffer, sizeof(UCHAR));

	/* check apidx valid */
	if (apIdx != 0xff) {
		pMbss = &pAd->ApCfg.MBSSID[apIdx];
		apOper = AP_BSS_OPER_BY_RF;
	}

	if (pMbss == NULL)
		return 0;

	pDot11h = pMbss->wdev.pDot11_H;
	if (pDot11h == NULL)
		return 0;


		/* Normal DFS */
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
		DedicatedZeroWaitStop(pAd, FALSE);
#endif
		pDot11h->RDMode = RD_SILENCE_MODE;
		APStop(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
		if (DfsStopWifiCheck(pAd)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Stop AP Startup\n", __func__));
			return 0;
		}
#endif
		APStartUp(pAd, pMbss, apOper);
#ifdef MT_DFS_SUPPORT
		if (pAd->CommonCfg.dbdc_mode) {
			MtCmdSetDfsTxStart(pAd, HcGetBandByWdev(&pMbss->wdev));
		} else {
			MtCmdSetDfsTxStart(pAd, DBDC_BAND0);
		}
		DfsSetCacRemainingTime(pAd, &pMbss->wdev);
		DfsReportCollision(pAd);
#ifdef BACKGROUND_SCAN_SUPPORT
		DfsDedicatedScanStart(pAd);
#endif
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

