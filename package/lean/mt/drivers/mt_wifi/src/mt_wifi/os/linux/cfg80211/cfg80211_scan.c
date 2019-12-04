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
 *	All related CFG80211 Scan function body.
 *
 *	History:
 *
 ***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"


/* Refine on 2013/04/30 for two functin into one */
INT CFG80211DRV_OpsScanGetNextChannel(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;

	if (cfg80211_ctrl->pCfg80211ChanList != NULL) {
		if (cfg80211_ctrl->Cfg80211CurChanIndex < cfg80211_ctrl->Cfg80211ChanListLen)
			return cfg80211_ctrl->pCfg80211ChanList[cfg80211_ctrl->Cfg80211CurChanIndex++];

		os_free_mem(cfg80211_ctrl->pCfg80211ChanList);
		cfg80211_ctrl->pCfg80211ChanList = NULL;
		cfg80211_ctrl->Cfg80211ChanListLen = 0;
		cfg80211_ctrl->Cfg80211CurChanIndex = 0;
		return 0;
	}

	return 0;
}

BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT8						 dataLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL cfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT32 *pChanList = (UINT32 *) pData;

	if (pChanList != NULL) {
		if (cfg80211_ctrl->pCfg80211ChanList != NULL)
			os_free_mem(cfg80211_ctrl->pCfg80211ChanList);

		os_alloc_mem(NULL, (UCHAR **)&cfg80211_ctrl->pCfg80211ChanList, sizeof(UINT32 *) * dataLen);

		if (cfg80211_ctrl->pCfg80211ChanList != NULL) {
			NdisCopyMemory(cfg80211_ctrl->pCfg80211ChanList, pChanList, sizeof(UINT32 *) * dataLen);
			cfg80211_ctrl->Cfg80211ChanListLen = dataLen;
			cfg80211_ctrl->Cfg80211CurChanIndex = 0; /* Start from index 0 */
			return NDIS_STATUS_SUCCESS;
		} else
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_FAILURE;
}

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID						*pAdOrg,
	UINT8						 IfType)
{
	return TRUE;
}

#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
void CFG80211DRV_Set_NOA(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	/* iversonmcc */
	UINT32   Lowpart, Highpart, NextTbtt, temp;

	if (pAd->Mlme.bStartMcc == TRUE) {
		/* scan , send null fram to infra , and switch to GO */
		Stop_MCC(pAd, 1);
	}

	/*
	 * pNdev->name = wlan0
	 * [ 2434.071449] 80211> CFG80211_OpsScan ==> wlan0(2)
	 * 10ms  start scan
	 * one channel 85 ms ~  7 channel
	 * 600ms
	 * 600 + wait 120 =720
	 *
	 * 898.328054] mt76x2_switch_channel(): Switch to Ch#9(2T2R), BBP_BW=0, bbp_ch_idx=0)
	 * interval = 400+120 ms
	 * Count =4;
	 *
	 * pNdev->name = p2p0
	 * 10ms  start scan
	 *
	 * 85ms & 3
	 * 260ms
	 * count =2
	 */
	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd)) {
		if (rtstrcasecmp(pData, "p2p0") == TRUE) {
			AsicGetTsfTime(pAd, &Highpart, &Lowpart);
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!Current Tsf LSB = = %ld\n",  Lowpart));
			RTMP_IO_READ32(pAd, LPON_T1STR, &temp);
			temp = temp & 0x0000FFFF;
			NextTbtt	= temp % pAd->CommonCfg.BeaconPeriod;
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!NextTbtt =  %ld\n", NextTbtt));
			temp = NextTbtt * 1024 + Lowpart;
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!Tsf LSB + TimeTillTbtt= %ld\n", temp));
			pAd->cfg80211_ctrl.GONoASchedule.StartTime = Lowpart + NextTbtt * 1024 + 409600 + 3200;
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" pAd->GONoASchedule.StartTime = %ld\n", pAd->cfg80211_ctrl.GONoASchedule.StartTime));
			pAd->cfg80211_ctrl.GONoASchedule.Count = 9;
			pAd->cfg80211_ctrl.GONoASchedule.Duration = 260000;
			pAd->cfg80211_ctrl.GONoASchedule.Interval =  384800;
		}

		if (rtstrcasecmp(pData, "wlan0") == TRUE) {
			AsicGetTsfTime(pAd, &Highpart, &Lowpart);
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!Current Tsf LSB = = %ld\n",  Lowpart));
			RTMP_IO_READ32(pAd, LPON_T1STR, &temp);
			temp = temp & 0x0000FFFF;
			NextTbtt	= temp % pAd->CommonCfg.BeaconPeriod;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!NextTbtt =  %ld\n", NextTbtt));
			temp = NextTbtt * 1024 + Lowpart;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!!Tsf LSB + TimeTillTbtt= %ld\n", temp));
			pAd->cfg80211_ctrl.GONoASchedule.StartTime = Lowpart + NextTbtt * 1024 + 512000 + 3200;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" pAd->GONoASchedule.StartTime = %ld\n", pAd->cfg80211_ctrl.GONoASchedule.StartTime));
			pAd->cfg80211_ctrl.GONoASchedule.Count = (4 + (11 * 5) + 2); /*wait 4 beacon + (interval * 4)*/
			pAd->cfg80211_ctrl.GONoASchedule.Duration = 737280; /*720*1024*/
			pAd->cfg80211_ctrl.GONoASchedule.Interval =  737280 + (400 * 1024);
		}
	}

	return TRUE;
}
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */


BOOLEAN CFG80211DRV_OpsScanExtraIesSet(
	VOID						*pAdOrg)
{
	return TRUE;
}

#ifdef CFG80211_SCAN_SIGNAL_AVG
static void CFG80211_CalBssAvgRssi(
	IN      BSS_ENTRY * pBssEntry)
{
	BOOLEAN bInitial = FALSE;

	if (!(pBssEntry->AvgRssiX8 | pBssEntry->AvgRssi))
		bInitial = TRUE;

	if (bInitial) {
		pBssEntry->AvgRssiX8 = pBssEntry->Rssi << 3;
		pBssEntry->AvgRssi  = pBssEntry->Rssi;
	} else {
		/* For smooth purpose, oldRssi for 7/8, newRssi for 1/8 */
		pBssEntry->AvgRssiX8 =
			(pBssEntry->AvgRssiX8 - pBssEntry->AvgRssi) + pBssEntry->Rssi;
	}

	pBssEntry->AvgRssi = pBssEntry->AvgRssiX8 >> 3;
}

static void CFG80211_UpdateBssTableRssi(
	IN VOID							*pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct ieee80211_channel *chan;
	struct cfg80211_bss *bss;
	BSS_ENTRY *pBssEntry;
	UINT index;
	UINT32 CenFreq;

	for (index = 0; index < pAd->ScanTab.BssNr; index++) {
		pBssEntry = &pAd->ScanTab.BssEntry[index];
#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)

		if (pAd->ScanTab.BssEntry[index].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel, IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel, IEEE80211_BAND_2GHZ);

#else
		CenFreq = ieee80211_channel_to_frequency(pAd->ScanTab.BssEntry[index].Channel);
#endif
		chan = ieee80211_get_channel(pWiphy, CenFreq);
		bss = cfg80211_get_bss(pWiphy, chan, pBssEntry->Bssid, pBssEntry->Ssid, pBssEntry->SsidLen,
							   WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

		if (bss == NULL) {
			/* ScanTable Entry not exist in kernel buffer */
		} else {
			/* HIT */
			CFG80211_CalBssAvgRssi(pBssEntry);
			bss->signal = pBssEntry->AvgRssi * 100; /* UNIT: MdBm */
			CFG80211OS_PutBss(pWiphy, bss);
		}
	}
}
#endif /* CFG80211_SCAN_SIGNAL_AVG */

/*
 * ========================================================================
 * Routine Description:
 *	Inform us that a scan is got.
 *
 * Arguments:
 *	pAdCB				- WLAN control block pointer
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Call RT_CFG80211_SCANNING_INFORM, not CFG80211_Scaning
 * ========================================================================
 */
VOID CFG80211_Scaning(
	IN VOID							*pAdCB,
	IN UINT32						BssIdx,
	IN UINT32						ChanId,
	IN UCHAR						*pFrame,
	IN UINT32						FrameLen,
	IN INT32						RSSI)
{
}


/*
 * ========================================================================
 * Routine Description:
 *	Inform us that scan ends.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	FlgIsAborted	- 1: scan is aborted
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211_ScanEnd(
	IN VOID						*pAdCB,
	IN BOOLEAN					FlgIsAborted)
{
}

VOID CFG80211_ScanStatusLockInit(
	IN VOID						*pAdCB,
	IN UINT                      init)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;

	if (init)
		NdisAllocateSpinLock(pAd, &pCfg80211_CB->scan_notify_lock);
	else
		NdisFreeSpinLock(&pCfg80211_CB->scan_notify_lock);
}

#endif /* RT_CFG80211_SUPPORT */

