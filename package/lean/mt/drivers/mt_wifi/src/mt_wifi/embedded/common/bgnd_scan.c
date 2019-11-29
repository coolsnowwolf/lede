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

    Abstract:
*/


#include "rt_config.h"
#include "bgnd_scan.h"

/* extern MT_SWITCH_CHANNEL_CFG CurrentSwChCfg[2]; */

BOOLEAN BackgroundScanSkipChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i = 0; i < pAd->BgndScanCtrl.SkipChannelNum; i++) {
		if (Ch == pAd->BgndScanCtrl.SkipChannelList[i]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

static inline INT GetABandChOffset(
	IN INT Channel)
{
#ifdef A_BAND_SUPPORT

	if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60) || (Channel == 100) || (Channel == 108) ||
		(Channel == 116) || (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157))
		return 1;
	else if ((Channel == 40) || (Channel == 48) || (Channel == 56) || (Channel == 64) || (Channel == 104) || (Channel == 112) ||
			 (Channel == 120) || (Channel == 128) || (Channel == 136) || (Channel == 153) || (Channel == 161))
		return -1;

#endif /* A_BAND_SUPPORT */
	return 0;
}

UCHAR BgndSelectBestChannel(RTMP_ADAPTER *pAd)
{
	int i;
	UCHAR BestChannel = 0, BestPercen = 0xff, Percen = 0;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (pAd->BgndScanCtrl.GroupChList[i].SkipGroup == 0) {
			Percen = ((pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time) * 100) / (((pAd->BgndScanCtrl.ScanDuration) * 1000) - (pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ChIdx=%d control-Channle=%d cen-channel=%d\n", i, pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel, pAd->BgndScanCtrl.GroupChList[i].CenChannel));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("		Max_PCCA=%x, Min_PCCA=%x, Band0_Tx_Time=%x, Percentage=%d\n",
					 pAd->BgndScanCtrl.GroupChList[i].Max_PCCA_Time, pAd->BgndScanCtrl.GroupChList[i].Min_PCCA_Time,
					 pAd->BgndScanCtrl.GroupChList[i].Band0_Tx_Time, Percen));

			if (Percen <= BestPercen) {
				BestPercen = Percen;
				BestChannel = pAd->BgndScanCtrl.GroupChList[i].BestCtrlChannel;
			}
		}
	}

	return BestChannel;
}
VOID NextBgndScanChannel(RTMP_ADAPTER *pAd, UCHAR channel)
{
	int i;
	/* UCHAR next_channel = 0; */
	pAd->BgndScanCtrl.ScanChannel = 0;

	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (channel == pAd->BgndScanCtrl.BgndScanChList[i].Channel) {
			/* Record this channel's idx in ChannelList array.*/
			while (i < (pAd->BgndScanCtrl.ChannelListNum - 1)) {
				if (pAd->BgndScanCtrl.BgndScanChList[i + 1].SkipChannel != 1) {
					pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].Channel;
					pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i + 1].CenChannel;
					pAd->BgndScanCtrl.ChannelIdx = i + 1;
					return;
				} else
					i++;
			}

		}
	}
}

VOID FirstBgndScanChannel(RTMP_ADAPTER *pAd)
{
	int i;

	/* Find the first non skiped channel */
	for (i = 0; i < (pAd->BgndScanCtrl.ChannelListNum - 1); i++) {
		if (pAd->BgndScanCtrl.BgndScanChList[i].SkipChannel != 1) {
			/* Record this channel's idx in ChannelList array.*/
			pAd->BgndScanCtrl.ScanChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ScanCenChannel = pAd->BgndScanCtrl.BgndScanChList[i].CenChannel;
			pAd->BgndScanCtrl.FirstChannel = pAd->BgndScanCtrl.BgndScanChList[i].Channel;
			pAd->BgndScanCtrl.ChannelIdx = i;
			break;
		}
	}

}

VOID BuildBgndScanChList(RTMP_ADAPTER *pAd)
{
	INT channel_idx, ChListNum = 0;
	UCHAR ch;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
#endif
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (pAd->BgndScanCtrl.IsABand) {
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
			ch = pChCtrl->ChList[channel_idx].Channel;

			if (cfg_ht_bw == BW_20) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_N_SUPPORT
			else if (((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
					  && (vht_bw == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
					 )
					 &&N_ChannelGroupCheck(pAd, ch, wdev)) {
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;

				if (GetABandChOffset(ch) == 1)
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel + 2;
				else
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel - 2;

				pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
				pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
				ChListNum++;
			}

#ifdef DOT11_VHT_AC
			else if (vht_bw == VHT_BW_80) {
				if (vht80_channel_group(pAd, ch)) {
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pChCtrl->ChList[channel_idx].Channel;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = vht_cent_ch_freq(ch, VHT_BW_80);
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].DfsReq = pChCtrl->ChList[channel_idx].DfsReq;
					pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, ch);
					ChListNum++;
				}
			}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		}
	} else {
		/* 2.4G only support BW20 background scan */
		for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].Channel = pAd->BgndScanCtrl.BgndScanChList[ChListNum].CenChannel = pChCtrl->ChList[channel_idx].Channel;
			pAd->BgndScanCtrl.BgndScanChList[ChListNum].SkipChannel = BackgroundScanSkipChannelCheck(pAd, pChCtrl->ChList[channel_idx].Channel);
			ChListNum++;
		}
	}

	pAd->BgndScanCtrl.ChannelListNum = ChListNum;
	if (pAd->BgndScanCtrl.ChannelListNum == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[BuildBgndScanChList] BandIdx = %d, pAd->BgndScanCtrl.ChannelListNum=%d\n",
			BandIdx, pAd->BgndScanCtrl.ChannelListNum));
	}

	for (channel_idx = 0; channel_idx < pAd->BgndScanCtrl.ChannelListNum; channel_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Support channel: PrimCh=%d, CentCh=%d, DFS=%d\n",
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].Channel, pAd->BgndScanCtrl.BgndScanChList[channel_idx].CenChannel,
				 pAd->BgndScanCtrl.BgndScanChList[channel_idx].DfsReq));
	}
}

UINT8 GroupChListSearch(PRTMP_ADAPTER pAd, UCHAR CenChannel)
{
	UCHAR i;
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = pAd->BgndScanCtrl.GroupChList;

	for (i = 0; i < pAd->BgndScanCtrl.GroupChListNum; i++) {
		if (GroupChList->CenChannel == CenChannel)
			return i;

		GroupChList++;
	}

	return 0xff;
}

VOID GroupChListInsert(PRTMP_ADAPTER pAd, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	UCHAR i = pAd->BgndScanCtrl.GroupChListNum;
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = &pAd->BgndScanCtrl.GroupChList[i];

	GroupChList->BestCtrlChannel = pSource->Channel;
	GroupChList->CenChannel = pSource->CenChannel;
	GroupChList->Max_PCCA_Time = GroupChList->Min_PCCA_Time = pSource->PccaTime;
	GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	GroupChList->SkipGroup = pSource->SkipChannel;
	pAd->BgndScanCtrl.GroupChListNum = i + 1;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Insert new group channel list Number=%d CenChannel=%d BestCtrlChannel=%d Max_PCCA_TIEM=%x, SkipGroup=%d\n",
			 __func__, pAd->BgndScanCtrl.GroupChListNum, GroupChList->CenChannel, GroupChList->BestCtrlChannel,	GroupChList->Max_PCCA_Time, GroupChList->SkipGroup));
}

VOID GroupChListUpdate(PRTMP_ADAPTER pAd, UCHAR index, PBGND_SCAN_SUPP_CH_LIST pSource)
{
	/* UCHAR i; */
	PBGND_SCAN_CH_GROUP_LIST	GroupChList = &pAd->BgndScanCtrl.GroupChList[index];

	if (pSource->PccaTime > GroupChList->Max_PCCA_Time) {
		GroupChList->Max_PCCA_Time = pSource->PccaTime;
		GroupChList->Band0_Tx_Time = pSource->Band0TxTime;
	}

	if (pSource->PccaTime < GroupChList->Min_PCCA_Time) {
		GroupChList->Min_PCCA_Time = pSource->PccaTime;
		GroupChList->BestCtrlChannel = pSource->Channel;
	}

	if (GroupChList->SkipGroup == 0 && pSource->SkipChannel == 1)
		GroupChList->SkipGroup = pSource->SkipChannel;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Update group channel list index=%d CenChannel=%d BestCtrlChannel=%d PCCA_TIEM=%x SkipGroup=%d\n",
			 __func__, pAd->BgndScanCtrl.GroupChListNum, GroupChList->CenChannel, GroupChList->BestCtrlChannel,	GroupChList->Max_PCCA_Time, GroupChList->SkipGroup));
}

VOID GenerateGroupChannelList(PRTMP_ADAPTER pAd)
{
	UCHAR i, ListIndex;
	/* PBGND_SCAN_CH_GROUP_LIST	GroupChList = pAd->BgndScanCtrl.GroupChList; */
	PBGND_SCAN_SUPP_CH_LIST		SuppChList = pAd->BgndScanCtrl.BgndScanChList;
	/* PBACKGROUND_SCAN_CTRL		BgndScanCtrl = &pAd->BgndScanCtrl; */
	/* PBGND_SCAN_SUPP_CH_LIST		SuppChList = BgndScanCtrl->BgndScanChList; */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ChannelListNum=%d\n",
			 __func__, pAd->BgndScanCtrl.ChannelListNum));
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, MAX_NUM_OF_CHANNELS * sizeof(BGND_SCAN_CH_GROUP_LIST));
	pAd->BgndScanCtrl.GroupChListNum = 0; /* reset Group Number. */

	for (i = 0; i < pAd->BgndScanCtrl.ChannelListNum; i++) {
		ListIndex = GroupChListSearch(pAd, SuppChList->CenChannel);

		if (ListIndex == 0xff) /* Not Found */
			GroupChListInsert(pAd, SuppChList);
		else
			GroupChListUpdate(pAd, ListIndex, SuppChList);

		SuppChList++;
	}
}


VOID BackgroundScanStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, BGND_SCAN_MAX_STATE, BGND_SCAN_MAX_MSG, (STATE_MACHINE_FUNC)Drop, BGND_SCAN_IDLE, BGND_SCAN_MACHINE_BASE);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanStartAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_PARTIAL_SCAN, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)BackgroundScanTimeoutAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_LISTEN, BGND_SCAN_DONE, (STATE_MACHINE_FUNC)BackgroundScanWaitAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_REQ, (STATE_MACHINE_FUNC)BackgroundScanPartialAction);
	StateMachineSetAction(Sm, BGND_SCAN_WAIT, BGND_SCAN_CNCL, (STATE_MACHINE_FUNC)BackgroundScanCancelAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_SWITCH_CHANNEL, (STATE_MACHINE_FUNC)BackgroundSwitchChannelAction);
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_RDD_REQ, (STATE_MACHINE_FUNC)DfsZeroWaitStartAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_RDD_TIMEOUT, (STATE_MACHINE_FUNC)DfsZeroWaitStopAction);
#ifdef MT_DFS_SUPPORT
	StateMachineSetAction(Sm, BGND_SCAN_IDLE, BGND_DEDICATE_RDD_REQ, (STATE_MACHINE_FUNC)DedicatedZeroWaitStartAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_RADAR_FOUND, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
	StateMachineSetAction(Sm, BGND_RDD_DETEC, BGND_OUTBAND_SWITCH, (STATE_MACHINE_FUNC)DedicatedZeroWaitRunningAction);
#endif
}

VOID BackgroundScanInit(
	IN PRTMP_ADAPTER pAd)
{
	/* UCHAR channel_idx = 0; */
	/* UINT32 Value; */
	/* RTMP_REG_PAIR Reg[2]; */
	UCHAR PhyMode = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
#endif
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/*
	ToDo: Based on current settings to decide support background scan or not.
	Don't support case: DBDC, 80+80
	*/
	/* Scan BW */
	PhyMode = HcGetRadioPhyMode(pAd);
	pAd->BgndScanCtrl.IsABand = (WMODE_CAP_5G(PhyMode)) ? TRUE : FALSE;

	if (pAd->BgndScanCtrl.IsABand) {
#ifdef DOT11_VHT_AC

		if (vht_bw == VHT_BW_80)
			pAd->BgndScanCtrl.ScanBW = BW_80;
		else
#endif /* DOT11_VHT_AC */
			pAd->BgndScanCtrl.ScanBW = cfg_ht_bw;
	} else /* 2.4G */
		pAd->BgndScanCtrl.ScanBW = BW_20;

	/* Decide RxPath&TxStream for background */
	pAd->BgndScanCtrl.RxPath = 0xc;
	pAd->BgndScanCtrl.TxStream = 0x2;
#ifdef MT_DFS_SUPPORT
	pAd->BgndScanCtrl.DfsZeroWaitDuration = DEFAULT_OFF_CHNL_CAC_TIME;/* 120000; 2 min */
#endif

	if (pAd->CommonCfg.dbdc_mode == TRUE)
		pAd->BgndScanCtrl.BgndScanSupport = 0;

#ifdef DOT11_VHT_AC
	else if (vht_bw == VHT_BW_160 || vht_bw == VHT_BW_8080)
		pAd->BgndScanCtrl.BgndScanSupport = 0;

#endif /* DOT11_VHT_AC */
	else
		pAd->BgndScanCtrl.BgndScanSupport = 1;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IsABand=%d, ScanBW=%d\n", pAd->BgndScanCtrl.IsABand, pAd->BgndScanCtrl.ScanBW));

	if (pAd->BgndScanCtrl.BgndScanSupport) {
		BackgroundScanStateMachineInit(pAd, &pAd->BgndScanCtrl.BgndScanStatMachine, pAd->BgndScanCtrl.BgndScanFunc);
		/* Copy channel list for background. */
		BuildBgndScanChList(pAd);
		RTMPInitTimer(pAd, &pAd->BgndScanCtrl.BgndScanTimer, GET_TIMER_FUNCTION(BackgroundScanTimeout), pAd, FALSE);
		RTMPInitTimer(pAd, &pAd->BgndScanCtrl.DfsZeroWaitTimer, GET_TIMER_FUNCTION(DfsZeroWaitTimeout), wdev, FALSE);

		/* ToDo: Related CR initialization. */
		MAC_IO_WRITE32(pAd, MIB_M1SCR, 0x7ef3ffff); /* 0x820fd200 Enable Band1 PSCCA time */
		MAC_IO_WRITE32(pAd, MIB_M1SCR1, 0xffff); /* 0x820fd208  */
		MAC_IO_WRITE32(pAd, PHY_BAND1_PHY_CCA, 0); /* 0x82070818 Enable Band1 EDCCA  */
		/*
		Reg[0].Register=0x820fd200;
		Reg[1].Register=0x820fd208;
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 2);
		*/
		/* Enabel PSCCA time count & Enable EDCCA time count */
		/* MtCmdMultipleMacRegAccessWrite */
		pAd->BgndScanCtrl.ScanDuration = DefaultScanDuration;
		/* pAd->BgndScanCtrl.BgndScanInterval = DefaultBgndScanInterval; */
		pAd->BgndScanCtrl.BgndScanIntervalCount = 0;
		pAd->BgndScanCtrl.SkipDfsChannel = FALSE;
		pAd->BgndScanCtrl.PartialScanInterval = DefaultBgndScanPerChInterval; /* 10 seconds */
		pAd->BgndScanCtrl.NoisyTH = DefaultNoisyThreshold;
		pAd->BgndScanCtrl.ChBusyTimeTH = DefaultChBusyTimeThreshold;
		pAd->BgndScanCtrl.DriverTrigger = FALSE;
		pAd->BgndScanCtrl.IPIIdleTimeTH = DefaultIdleTimeThreshold;
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan doesn't support in current settings....\n"));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s <===============\n", __func__));
}

VOID BackgroundScanDeInit(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;

	RTMPReleaseTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
	RTMPReleaseTimer(&pAd->BgndScanCtrl.DfsZeroWaitTimer, &Cancelled);
}

VOID BackgroundScanStart(
	IN PRTMP_ADAPTER pAd,
	IN UINT8	BgndscanType)
{
	/* UINT32	Value; */
	/* In-band commad to notify FW(RA) background scan will start. */
	/* Switch channel for Band0 (Include Star Tx/Rx ?) */
	/* Scan channel for Band1 */
	/* Reset Group channel list */
	os_zero_mem(pAd->BgndScanCtrl.GroupChList, sizeof(BGND_SCAN_CH_GROUP_LIST) * MAX_NUM_OF_CHANNELS);
	pAd->BgndScanCtrl.GroupChListNum = 0;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s BgndscanType=%d ===============>\n", __func__, BgndscanType));

	if (BgndscanType && pAd->BgndScanCtrl.ScanType) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Ap Background scan is running ===============>\n", __func__));
		return;
	}

	pAd->BgndScanCtrl.ScanType = BgndscanType; /* 0:Disable 1:partial scan 2:continuous scan */

	if (BgndscanType == TYPE_BGND_PARTIAL_SCAN) {	/* partial scan */
		pAd->BgndScanCtrl.PartialScanIntervalCount = DefaultBgndScanPerChInterval; /* First time hope trigger scan immediately. */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_PARTIAL_SCAN, 0, NULL, BgndscanType);
	} else if (BgndscanType == TYPE_BGND_CONTINUOUS_SCAN) /* continuous scan */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, BgndscanType);
	else if (BgndscanType == TYPE_BGND_CONTINUOUS_SCAN_SWITCH_CH) {/* continuous scan and then switch channel*/
		pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
	} else {
		pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, BgndscanType);
	}

	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanStartAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UINT8 ScanType = (UINT8)(Elem->Priv);
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===> ScanType=%d\n", __func__, ScanType));
	FirstBgndScanChannel(pAd);
	pAd->BgndScanCtrl.ScanType = ScanType;
#ifdef GREENAP_SUPPORT
	greenap_suspend(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	BackgroundScanNextChannel(pAd, ScanType); /* 0:Disable 1:partial scan 2:continuous scan */
}

VOID BackgroundScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	UINT8 ScanType = pAd->BgndScanCtrl.ScanType;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===> ScanType=%d\n", __func__, ScanType));
	MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_TIMEOUT, 0, NULL, ScanType);
	RTMP_MLME_HANDLER(pAd);
}

VOID BackgroundScanTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	/* UINT32	PCCA_TIME; */
	RTMP_REG_PAIR Reg[5];
	MT_BGND_SCAN_CFG BgndScanCfg;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	UCHAR	RxStreamNums = 0;
	UCHAR	RxPath = 0;
	INT	i;
	UINT8 ScanType = (UINT8)(Elem->Priv);
	/* UINT32	pccatime, sccatime, edtime, band0txtime, mdrdy; */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s channel index=%d====ScanTyep=%d ==========>\n", __func__, pAd->BgndScanCtrl.ChannelIdx, ScanType));
	/* Updaet channel info */
	/* RTMP_IO_READ32(pAd, 0x820fd248, &PCCA_TIME); */
	Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
	Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
	Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
	Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
	Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
	MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].PccaTime = Reg[0].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].SccaTime = Reg[1].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].EDCCATime = Reg[2].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].Band0TxTime = Reg[3].Value;
	pAd->BgndScanCtrl.BgndScanChList[pAd->BgndScanCtrl.ChannelIdx].Mdrdy = Reg[4].Value;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ChannelIdx [%d].PCCA_TIME=%x, SCCA_TIEM=%x, EDCCA_TIME=%x, Band0TxTime=%x Mdrdy=%x ===============>\n", pAd->BgndScanCtrl.ChannelIdx, Reg[0].Value, Reg[1].Value, Reg[2].Value, Reg[3].Value, Reg[4].Value));
	NextBgndScanChannel(pAd, pAd->BgndScanCtrl.ScanChannel);

	if (pAd->BgndScanCtrl.ScanChannel == 0 || ScanType == TYPE_BGND_CONTINUOUS_SCAN) /* Ready to stop or continuous scan */
		BackgroundScanNextChannel(pAd, ScanType);
	else {	/* Next time partail scan */
		/*TODO: Switch to 4x4 mode */
		/* RxStream to RxPath */
		/* Restore switch channel configuration */
		CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);
		RxStreamNums = CurrentSwChCfg->RxStream;

		if (RxStreamNums > 4) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():illegal RxStreamNums(%d)\n",
					  __func__, RxStreamNums));
			RxStreamNums = 4;
		}

		for (i = 0; i < RxStreamNums; i++)
			RxPath |= 1 << i;

		BgndScanCfg.BandIdx = 0;
		BgndScanCfg.Bw = CurrentSwChCfg->Bw;
		BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
		BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_STOP;
		BgndScanCfg.RxPath = RxPath; /* return to 4 Rx */
		BgndScanCfg.TxStream = CurrentSwChCfg->TxStream;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Stop Scan Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Notify RA background scan stop */
		BgScNotify.NotifyFunc =  (BgndScanCfg.TxStream << 5 | 0xf);
		BgScNotify.BgndScanStatus = 0;/* stop */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d\n", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);
		/* Enable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_DONE, 0, NULL, 0);
	}
}


VOID BackgroundScanWaitAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/* Change state to Wait State. If all conditions match, will trigger partial scan */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_WAIT;
}
VOID BackgroundScanPartialAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	UINT8 ScanType = (UINT8)(Elem->Priv);
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s =====ScanType=%d===>\n", __func__, ScanType));

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s First time===========>\n", __func__));
		FirstBgndScanChannel(pAd);
#ifdef GREENAP_SUPPORT
		greenap_suspend(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	}

	BackgroundScanNextChannel(pAd, ScanType);
}

VOID BackgroundScanCancelAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ===============>\n", __func__));
	/* Re-init related parameters */
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
	pAd->BgndScanCtrl.ScanChannel = 0;
	pAd->BgndScanCtrl.ScanType = 0;
	pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
	RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanTimer, &Cancelled);
#ifdef GREENAP_SUPPORT
	greenap_resume(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	/* RTMPCancelTimer(&pAd->BgndScanCtrl.BgndScanNextTimer, &Cancelled); */
}
VOID BackgroundScanNextChannel(
	IN PRTMP_ADAPTER pAd,
	IN UINT8		ScanType)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	RTMP_REG_PAIR Reg[5];
	MT_BGND_SCAN_NOTIFY BgScNotify;
	INT	i;
	/* UINT32	pccatime, sccatime, edtime, band0txtime, mdrdy; */
	UCHAR	BestChannel = 0;
	UCHAR	RxStreamNums = 0;
	UCHAR	RxPath = 0;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
	/* Restore switch channel configuration */
	CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Scan Channel=%d===============>\n", __func__, pAd->BgndScanCtrl.ScanChannel));

	if (pAd->BgndScanCtrl.ScanChannel == 0) {
		/* pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;//Scan Stop */
		/* pAd->BgndScanCtrl.ScanType = 0;//Scan Complete. */
		/* RxStream to RxPath */
		RxStreamNums = CurrentSwChCfg->RxStream;

		if (RxStreamNums > 4) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():illegal RxStreamNums(%d)\n",
					  __func__, RxStreamNums));
			RxStreamNums = 4;
		}

		for (i = 0; i < RxStreamNums; i++)
			RxPath |= 1 << i;

		BgndScanCfg.BandIdx = 0;
		BgndScanCfg.Bw = CurrentSwChCfg->Bw;
		BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
		BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_STOP;
		BgndScanCfg.RxPath = RxPath; /* return to 4 Rx */
		BgndScanCfg.TxStream = CurrentSwChCfg->TxStream;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Stop Scan Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Notify RA background scan stop */
		BgScNotify.NotifyFunc =  (BgndScanCfg.TxStream << 5 | 0xf);
		BgScNotify.BgndScanStatus = 0;/* stop */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);
		/* Enable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 1);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */
		/* Dump Channel Info */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Dump Channel Info\n"));
		GenerateGroupChannelList(pAd);
		BestChannel = BgndSelectBestChannel(pAd);
		pAd->BgndScanCtrl.BestChannel = BestChannel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Best Channel=%d, IsSwitchChannel=%d Noisy=%d\n", BestChannel, pAd->BgndScanCtrl.IsSwitchChannel, pAd->BgndScanCtrl.Noisy));
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;/* Scan Stop */
		pAd->BgndScanCtrl.ScanType = 0;/* Scan Complete. */

		if (BestChannel != CurrentSwChCfg->ControlChannel && pAd->BgndScanCtrl.IsSwitchChannel == TRUE) {
			pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SWITCH_CHANNEL, 0, NULL, 0);
			RTMP_MLME_HANDLER(pAd);
		}

		pAd->BgndScanCtrl.IsSwitchChannel = FALSE;
#ifdef GREENAP_SUPPORT
		greenap_resume(pAd, greenap, AP_BACKGROUND_SCAN);
#endif /* GREENAP_SUPPORT */
	} else if (ScanType == TYPE_BGND_PARTIAL_SCAN) { /* Partial Scan */
		BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
		BgScNotify.BgndScanStatus = 1;/* start */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d\n", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);
		/* Disable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 0);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */
		/* Switch Band1 channel to pAd->BgndScanCtrl.ScanChannel */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Fill band1 BgndScanCfg */
		BgndScanCfg.BandIdx = 1;
		BgndScanCfg.Bw = pAd->BgndScanCtrl.ScanBW;
		BgndScanCfg.ControlChannel = pAd->BgndScanCtrl.ScanChannel;
		BgndScanCfg.CentralChannel = pAd->BgndScanCtrl.ScanCenChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
		BgndScanCfg.RxPath = 0x0C; /* Distribute 2 Rx for background scan */
		BgndScanCfg.TxStream = 2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start Scan Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Fill band0 BgndScanCfg */
		BgndScanCfg.BandIdx = 0;
		BgndScanCfg.Bw = CurrentSwChCfg->Bw /*pAd->BgndScanCtrl.ScanBW*/;
		BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
		BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
		BgndScanCfg.RxPath = 0x03; /* Keep 2 Rx for original service */
		BgndScanCfg.TxStream = 2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start Scan Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else if (pAd->BgndScanCtrl.ScanChannel == pAd->BgndScanCtrl.FirstChannel) {
		BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
		BgScNotify.BgndScanStatus = 1;/* start */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d\n", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
		MtCmdBgndScanNotify(pAd, BgScNotify);
		/* Disable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
		BfSwitch(pAd, 0);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
		MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */
		/* Switch Band1 channel to pAd->BgndScanCtrl.ScanChannel */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Fill band1 BgndScanCfg */
		BgndScanCfg.BandIdx = 1;
		BgndScanCfg.Bw = pAd->BgndScanCtrl.ScanBW;
		BgndScanCfg.ControlChannel = pAd->BgndScanCtrl.ScanChannel;
		BgndScanCfg.CentralChannel = pAd->BgndScanCtrl.ScanCenChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
		BgndScanCfg.RxPath = 0x0C; /* Distribute 2 Rx for background scan */
		BgndScanCfg.TxStream = 2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start Scan Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Fill band0 BgndScanCfg */
		BgndScanCfg.BandIdx = 0;
		BgndScanCfg.Bw = CurrentSwChCfg->Bw /*pAd->BgndScanCtrl.ScanBW*/;
		BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
		BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
		BgndScanCfg.RxPath = 0x03; /* Keep 2 Rx for original service */
		BgndScanCfg.TxStream = 2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start Scan Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 200ms timer */
	} else {
		/* RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, 200); //200ms timer */
		/* Switch Band1 channel to pAd->BgndScanCtrl.ScanChannel */
		pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_LISTEN;
		/* Fill band1 BgndScanCfg */
		BgndScanCfg.BandIdx = 1;
		BgndScanCfg.Bw = pAd->BgndScanCtrl.ScanBW;
		BgndScanCfg.ControlChannel = pAd->BgndScanCtrl.ScanChannel;
		BgndScanCfg.CentralChannel = pAd->BgndScanCtrl.ScanCenChannel;
		BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_RUNNING;
		BgndScanCfg.RxPath = 0x0C; /* Distribute 2 Rx for background scan */
		BgndScanCfg.TxStream = 2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Running Scan Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
				 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
				 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
		MtCmdBgndScan(pAd, BgndScanCfg);
		/* Read clear below CR */
		Reg[0].Register = 0x820fd248/*MIB_M1SDR16*/; /* PCCA Time */
		Reg[1].Register = 0x820fd24c/*MIB_M1SDR17*/; /* SCCA Time */
		Reg[2].Register = 0x820fd250/*MIB_M1SDR18*/; /* ED Time */
		Reg[3].Register = 0x820fd094/*MIB_M0SDR35*/; /* Bnad0 TxTime */
		Reg[4].Register = 0x820fd258/*MIB_M1SDR20*/; /* Mdrdy */
		MtCmdMultipleMacRegAccessRead(pAd, Reg, 5);
		RTMPSetTimer(&pAd->BgndScanCtrl.BgndScanTimer, pAd->BgndScanCtrl.ScanDuration); /* 500ms timer */
	}
}

VOID BackgroundSwitchChannelAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev = NULL;
	wdev = pAd->wdev_list[0];
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN;
	printk("Switch to channel to %d\n", pAd->BgndScanCtrl.BestChannel);
	rtmp_set_channel(pAd, wdev, pAd->BgndScanCtrl.BestChannel);
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
}

VOID BackgroundChannelSwitchAnnouncementAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	printk("Trigger Channel Switch Announcemnet IE\n");
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Trigger Channel Switch Announcemnet IE\n"));
	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_CS_ANN; /* Channel switch annoncement. */
	/* HcUpdateCsaCntByChannel(pAd, pAd->BgndScanCtrl.BestChannel); */
	/* pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_SCAN_IDLE; //For temporary */
}

VOID DfsZeroWaitTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	struct wifi_dev *wdev = (struct wifi_dev *)FunctionContext;
	RTMP_ADAPTER *pAd = NULL;

	if (wdev == NULL)
		return;
	pAd = wdev->sys_handle;

#ifdef MT_DFS_SUPPORT
	UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Status=%d",
			 __func__,
			 GET_MT_ZEROWAIT_DFS_STATE(pAd)));
#endif
	MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_TIMEOUT, sizeof(struct wifi_dev), wdev, 0);
	RTMP_MLME_HANDLER(pAd);
}

VOID DfsZeroWaitStart(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN	DfsZeroWaitEnable,
	struct wifi_dev *wdev)
{
	if (wdev == NULL)
		return;

	if (DfsZeroWaitEnable)
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_REQ, sizeof(struct wifi_dev), wdev, 0);
	else
		MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_RDD_CNCL, 0, NULL, 0); /* So far not thing to do. */

	RTMP_MLME_HANDLER(pAd);
}


VOID DfsZeroWaitStartAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	UCHAR		channel_idx;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	struct freq_oper oper;
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Msg;

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev is NULL! \n", __func__));

		return;
	}
	if (hc_radio_query_by_wdev(wdev, &oper)) {
		return;
	}


	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;
	CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);
	BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
	BgScNotify.BgndScanStatus = 1;/* start */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DfsZeroWaitStartAction Notify NotifyFunc=%x, Status=%d", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
	MtCmdBgndScanNotify(pAd, BgScNotify);
	/* Disable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
	BfSwitch(pAd, 0);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */
	BgndScanCfg.CentralChannel = 0;

	for (channel_idx = 0; channel_idx < pAd->BgndScanCtrl.ChannelListNum; channel_idx++) {
		if (pAd->BgndScanCtrl.DfsZeroWaitChannel == pAd->BgndScanCtrl.BgndScanChList[channel_idx].Channel) {
			BgndScanCfg.CentralChannel = pAd->BgndScanCtrl.BgndScanChList[channel_idx].CenChannel;
			break;
		}
	}

	if (BgndScanCfg.CentralChannel == 0)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Illege CentralChannel=%d\n", __func__, BgndScanCfg.CentralChannel));

	BgndScanCfg.ControlChannel = pAd->BgndScanCtrl.DfsZeroWaitChannel;
	BgndScanCfg.BandIdx = 1;
	BgndScanCfg.Bw = pAd->BgndScanCtrl.ScanBW;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
	BgndScanCfg.RxPath = 0x0C; /* Distribute 2 Rx for background scan */
	BgndScanCfg.TxStream = 2;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start DFS Zero Wait Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
	MtCmdBgndScan(pAd, BgndScanCfg);
	/* Fill band0 BgndScanCfg */
	BgndScanCfg.BandIdx = 0;
	BgndScanCfg.Bw = CurrentSwChCfg->Bw /*pAd->BgndScanCtrl.ScanBW*/;
	BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
	BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
	BgndScanCfg.RxPath = 0x03; /* Keep 2 Rx for original service */
	BgndScanCfg.TxStream = 2;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Start DFS Zero Wait Bandidx=%d,	BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
	MtCmdBgndScan(pAd, BgndScanCfg);
	/* Enable band1 DFS */
#ifdef MT_DFS_SUPPORT
	RadarStateCheck(pAd, wdev);
	DfsCacNormalStart(pAd, wdev, RD_NORMAL_MODE);
	WrapDfsRadarDetectStart(pAd, wdev);
#endif /* MT_DFS_SUPPORT */
	RTMPSetTimer(&pAd->BgndScanCtrl.DfsZeroWaitTimer, pAd->BgndScanCtrl.DfsZeroWaitDuration);
}

VOID DfsZeroWaitStopAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	UCHAR	RxStreamNums = 0;
	UCHAR	RxPath = 0;
	UCHAR	i;
	BOOL	Cancelled;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	struct wifi_dev *wdev = (struct wifi_dev *)Elem->Msg;
	struct DOT11_H *pDot11h = NULL;

	/* Please fix me in next phase for neptune DFS zero wait*/
	/*wdev will be NULL in MbssZeroWaitStopValidate and WrapDfsRddReportHandle case*/
	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():illegal Elem\n",
				 __func__));
		return;
	}

	pDot11h = wdev->pDot11_H;

	RTMPCancelTimer(&BgndScanCtrl->DfsZeroWaitTimer, &Cancelled);
	BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;
	CurrentSwChCfg = &(BgndScanCtrl->CurrentSwChCfg[0]);
#ifdef MT_DFS_SUPPORT
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]RadarDetected=%d, state=%d\n",
			 __func__,
			 BgndScanCtrl->RadarDetected,
			 GET_MT_ZEROWAIT_DFS_STATE(pAd)));
	/* Disable Band1 DFS */
	WrapDfsRadarDetectStop(pAd);
#endif /* MT_DFS_SUPPORT */
	/* RxStream to RxPath */
	RxStreamNums = CurrentSwChCfg->RxStream;

	if (RxStreamNums > 4) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():illegal RxStreamNums(%d)\n",
				  __func__, RxStreamNums));
		RxStreamNums = 4;
	}

	for (i = 0; i < RxStreamNums; i++)
		RxPath |= 1 << i;

	BgndScanCfg.BandIdx = 0;
	BgndScanCfg.Bw = CurrentSwChCfg->Bw;
	BgndScanCfg.ControlChannel = CurrentSwChCfg->ControlChannel;
	BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_STOP;
	BgndScanCfg.RxPath = RxPath; /* return to 4 Rx */
	BgndScanCfg.TxStream = CurrentSwChCfg->TxStream;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Stop Scan Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			 __func__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			 BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));
	MtCmdBgndScan(pAd, BgndScanCfg);
	/* Notify RA background scan stop */
	BgScNotify.NotifyFunc =  (BgndScanCfg.TxStream << 5 | 0xf);
	BgScNotify.BgndScanStatus = 0;/* stop */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Background scan Notify NotifyFunc=%x, Status=%d", BgScNotify.NotifyFunc, BgScNotify.BgndScanStatus));
	MtCmdBgndScanNotify(pAd, BgScNotify);
	/* Enable BF, MU */
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
	BfSwitch(pAd, 1);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */
#ifdef MT_DFS_SUPPORT
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]RadarDetected=%d, state=%d\n",
			 __func__,
			 BgndScanCtrl->RadarDetected,
			 GET_MT_ZEROWAIT_DFS_STATE(pAd)));

	/* MBSS Zero Wait Stop flow */
	if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_MBSS_CAC))
		return;

	if ((BgndScanCtrl->RadarDetected != 1)
#ifdef MAC_REPEATER_SUPPORT
		&& (!(CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT) && (pAd->ApCfg.bMACRepeaterEn == TRUE)))
#endif
	   ) {
		UCHAR org_ch;

		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev is NULL!\n",
					 __func__));
			/*pDot11h->RDMode = RD_NORMAL_MODE;*/
			UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
			return;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]ZeroWaitStop,no radar, state=%d\n",
				 __func__,
				 GET_MT_ZEROWAIT_DFS_STATE(pAd)));
		pDot11h->RDMode = RD_SWITCHING_MODE;


		if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_OFF_CHNL_CAC_TIMEOUT)) {
			/* Off-Channel CAC timeout: Band1 DFS channel is safe for using */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Currstate:CAC_TIMEOUT, Switch to CH%d!\n",
					 __func__,
					 pAd->BgndScanCtrl.DfsZeroWaitChannel));
			rtmp_set_channel(pAd, wdev, pAd->BgndScanCtrl.DfsZeroWaitChannel);
		} else if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_CAC)) {
			UCHAR   Channel;

			/* Off-Channel CAC period -> set new channel(nonDFS/DFS) */
			if ((BgndScanCtrl->DfsZeroWaitChannel == 0)
				|| (RadarChannelCheck(pAd, BgndScanCtrl->DfsZeroWaitChannel) == FALSE)
#ifdef MAC_REPEATER_SUPPORT
				|| (pAd->ApCfg.bMACRepeaterEn)
#endif /* MAC_REPEATER_SUPPORT */
			   ) {
				/* 1. set nonDFS Channel case and next step is APStop/APStartUp
				   2. Previous is CAC state and repeater dynamic enable
				*/
				Channel  = BgndScanCtrl->DfsZeroWaitChannel;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]Currstate:DFS_CAC, Switch to CH%d!\n",
						 __func__,
						 Channel));
				UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE);
#ifdef MAC_REPEATER_SUPPORT

				/* Disable DFS zero wait support  for repeater mode dynamic enable */
				if (pAd->ApCfg.bMACRepeaterEn) {
					BgndScanCtrl->DfsZeroWaitSupport = FALSE;
					UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
				}

#endif /* MAC_REPEATER_SUPPORT */
				/* Do ChSwAnn by Set_Channel_Proc */
				rtmp_set_channel(pAd, wdev, Channel);
			} else {
				/* Ori CAC -> set DFS channel case
				   Next Step is ZeroWaitStart and still is CAC state */
				/* Re-select a non-DFS channel. */
				Channel = WrapDfsRandomSelectChannel(pAd, TRUE, 0); /* Skip DFS CH */
				/* Assign DfsZeroWait Band0 CH and update into BgnScan.CurrSwChCfg */
				CurrentSwChCfg->ControlChannel = Channel;
				CurrentSwChCfg->CentralChannel = DfsGetCentCh(pAd, Channel, CurrentSwChCfg->Bw, wdev);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s][CAC Period->Set DFS Ch]New nonDfsCh=%d, DfsCh=%d\n",
						 __func__,
						 Channel,
						 BgndScanCtrl->DfsZeroWaitChannel));
				org_ch = wdev->channel;
				wdev->channel = Channel;
				pDot11h->CSCount = 0;
				pDot11h->new_channel = Channel;
				pDot11h->RDMode = RD_SWITCHING_MODE; /* New add */

				if (HcUpdateCsaCntByChannel(pAd, org_ch) != 0)
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s]Update CsaCnt by Channel%d fail\n", __func__, Channel));
			}
		}
	} else {
		/*1. Radar detected during off-channel CAC*/
		/*2. Repeater enable during off-channel CAC*/
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]ZeroWaitStop,have radar(or repeater En during off-channel CAC), state=%d\n",
				 __func__,
				 GET_MT_ZEROWAIT_DFS_STATE(pAd)));

		if (CHK_MT_ZEROWAIT_DFS_STATE(pAd, DFS_RADAR_DETECT)) {
			DfsSetZeroWaitCacSecond(pAd);
			WrapDfsRddReportHandle(pAd, HW_RDD1);
			/* OffChl CAC period -> detected radar -> band1 Non-Dfs CH is single work CH */
			/* UPDATE_MT_ZEROWAIT_DFS_STATE(pAd, DFS_IDLE); */
			/* Enable Tx Queues and ACK */
			/* MtCmdSetDfsTxStart(pAd, DBDC_BAND0); */
#ifdef MAC_REPEATER_SUPPORT

			/* Disable DFS zero wait support  for repeater mode dynamic enable */
			if (pAd->ApCfg.bMACRepeaterEn) {
				BgndScanCtrl->DfsZeroWaitSupport = FALSE;
				UPDATE_MT_ZEROWAIT_DFS_Support(pAd, FALSE);
			}

#endif /* MAC_REPEATER_SUPPORT */
		}
	}
#endif /* MT_DFS_SUPPORT */
}

#ifdef MT_DFS_SUPPORT
VOID DedicatedZeroWaitStartAction(
		IN RTMP_ADAPTER * pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	CHAR OutBandCh = GET_BGND_PARAM(pAd, OUTBAND_CH);
	CHAR OutBandBw = GET_BGND_PARAM(pAd, OUTBAND_BW);
	CHAR InBandCh = GET_BGND_PARAM(pAd, INBAND_CH);
	CHAR InBandBw = GET_BGND_PARAM(pAd, INBAND_BW);

	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;

	CurrentSwChCfg = &(pAd->BgndScanCtrl.CurrentSwChCfg[0]);

	os_zero_mem(&BgndScanCfg, sizeof(MT_BGND_SCAN_CFG));

	BgScNotify.NotifyFunc =  (0x2 << 5 | 0xf);
	BgScNotify.BgndScanStatus = 1;/*start*/
	MtCmdBgndScanNotify(pAd, BgScNotify);

	/*Disable BF, MU*/
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 0);*/
	DynamicTxBfDisable(pAd, TRUE);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 0);
#endif /* CFG_SUPPORT_MU_MIMO */

	BgndScanCfg.ControlChannel = OutBandCh;
	BgndScanCfg.CentralChannel = DfsPrimToCent(OutBandCh, OutBandBw);
	BgndScanCfg.Bw = OutBandBw;
	BgndScanCfg.TxStream = 2;
	BgndScanCfg.RxPath = 0x0c;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
	BgndScanCfg.BandIdx = 1;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]  Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d \x1b[m \n",
			__FUNCTION__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));

	MtCmdBgndScan(pAd, BgndScanCfg);

	BgndScanCfg.ControlChannel = InBandCh;
	BgndScanCfg.CentralChannel = CurrentSwChCfg->CentralChannel;
	BgndScanCfg.Bw = InBandBw;
	BgndScanCfg.TxStream = 2;
	BgndScanCfg.RxPath = 0x03;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_START;
	BgndScanCfg.BandIdx = 0;


	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s]  Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d \x1b[m \n",
			__FUNCTION__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));

	MtCmdBgndScan(pAd, BgndScanCfg);

	/*Start Band1 radar detection*/
	DfsDedicatedOutBandRDDStart(pAd);
}
VOID DedicatedZeroWaitRunningAction(
		IN RTMP_ADAPTER * pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	CHAR OutBandCh;
	CHAR OutBandBw;

	DfsDedicatedOutBandRDDRunning(pAd);

	OutBandCh = GET_BGND_PARAM(pAd, OUTBAND_CH);
	OutBandBw = GET_BGND_PARAM(pAd, OUTBAND_BW);

	if (OutBandCh == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] No available outband Channel \x1b[m \n",
			__FUNCTION__));
		DedicatedZeroWaitStop(pAd, FALSE);
		return;
	}

	pAd->BgndScanCtrl.BgndScanStatMachine.CurrState = BGND_RDD_DETEC;

	os_zero_mem(&BgndScanCfg, sizeof(MT_BGND_SCAN_CFG));

	BgndScanCfg.ControlChannel = OutBandCh;
	BgndScanCfg.CentralChannel = DfsPrimToCent(OutBandCh, OutBandBw);
	BgndScanCfg.Bw = OutBandBw;
	BgndScanCfg.TxStream = 2;
	BgndScanCfg.RxPath = 0x0c;
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_RUNNING;
	BgndScanCfg.BandIdx = 1;


	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[1;33m [%s] Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d \x1b[m \n",
			__FUNCTION__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));

	MtCmdBgndScan(pAd, BgndScanCfg);

	/*Start Band1 radar detection*/
	DfsDedicatedOutBandRDDStart(pAd);
}

VOID DedicatedZeroWaitStop(
		IN RTMP_ADAPTER * pAd, BOOLEAN bApplyCurrentCh)
{
	MT_BGND_SCAN_CFG BgndScanCfg;
	MT_BGND_SCAN_NOTIFY BgScNotify;
	MT_SWITCH_CHANNEL_CFG *CurrentSwChCfg;
	UCHAR	RxStreamNums = 0;
	UCHAR	RxPath = 0;
	UCHAR	i;
	CHAR InBandCh;
	CHAR InBandBw;
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;

	if (bApplyCurrentCh == TRUE) {
		InBandCh = GET_BGND_PARAM(pAd, INBAND_CH);
		InBandBw = GET_BGND_PARAM(pAd, INBAND_BW);
	} else {
		InBandCh = GET_BGND_PARAM(pAd, ORI_INBAND_CH);
		InBandBw = GET_BGND_PARAM(pAd, ORI_INBAND_BW);
	}

	CurrentSwChCfg = &(BgndScanCtrl->CurrentSwChCfg[0]);

	if (!IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(pAd))
		return;

	if (!GET_BGND_STATE(pAd, BGND_RDD_DETEC))
		return;

	if (InBandCh == 0)
		return;

	BgndScanCtrl->BgndScanStatMachine.CurrState = BGND_SCAN_IDLE;

	DfsDedicatedOutBandRDDStop(pAd);

	/* RxStream to RxPath */
	RxStreamNums = CurrentSwChCfg->RxStream;
	if (RxStreamNums > 4) {
#ifdef DFS_DBG_LOG_0
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("\x1b[1;33m %s():illegal RxStreamNums(%d) \x1b[m \n",
			__FUNCTION__, RxStreamNums));
#endif
		RxStreamNums = 4;
	}

	for (i = 0; i < RxStreamNums; i++)
		RxPath |= 1 << i;

	BgndScanCfg.BandIdx = 0;
	BgndScanCfg.Bw = InBandBw;
	BgndScanCfg.ControlChannel = InBandCh;
	BgndScanCfg.CentralChannel = DfsPrimToCent(InBandCh, InBandBw);
	BgndScanCfg.Reason = CH_SWITCH_BACKGROUND_SCAN_STOP;
	BgndScanCfg.RxPath = RxPath; /* return to 4 Rx */
	BgndScanCfg.TxStream = CurrentSwChCfg->TxStream;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s] Bandidx=%d, BW=%d, CtrlCh=%d, CenCh=%d, Reason=%d, RxPath=%d\n",
			__FUNCTION__, BgndScanCfg.BandIdx, BgndScanCfg.Bw, BgndScanCfg.ControlChannel,
			BgndScanCfg.CentralChannel, BgndScanCfg.Reason, BgndScanCfg.RxPath));

	MtCmdBgndScan(pAd, BgndScanCfg);

	BgScNotify.NotifyFunc =  (BgndScanCfg.TxStream << 5 | 0xf);
	BgScNotify.BgndScanStatus = 0;/*stop*/

	MtCmdBgndScanNotify(pAd, BgScNotify);

	/*Enable BF, MU*/
#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
	/*BfSwitch(pAd, 1);*/
	DynamicTxBfDisable(pAd, FALSE);
#endif
#ifdef CFG_SUPPORT_MU_MIMO
	MuSwitch(pAd, 1);
#endif /* CFG_SUPPORT_MU_MIMO */

}
#endif

VOID BackgroundScanTest(
	IN PRTMP_ADAPTER pAd,
	IN MT_BGND_SCAN_CFG BgndScanCfg)
{
	/* Send Commad to MCU */
	MtCmdBgndScan(pAd, BgndScanCfg);
}

VOID ChannelQualityDetection(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ChBusyTime = 0;
	UINT32 MyTxAirTime = 0;
	UINT32 MyRxAirTime = 0;
	UCHAR BandIdx = 0;
	UINT32      lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, CrValue;
	UCHAR Noisy = 0;
	UINT32 TotalIPI = 0;
	/* RTMP_REG_PAIR Reg[11]; */
	/* Phase 1: No traffic stat */
	/* Check IPI & Channel Busy Time */
	BACKGROUND_SCAN_CTRL *BgndScanCtrl = &pAd->BgndScanCtrl;
	ChBusyTime = pAd->OneSecMibBucket.ChannelBusyTime[BandIdx];
	MyTxAirTime = pAd->OneSecMibBucket.MyTxAirtime[BandIdx];
	MyRxAirTime = pAd->OneSecMibBucket.MyRxAirtime[BandIdx];
	/* 1. Check Open enviroment via IPI (Band0) */
	HW_IO_READ32(pAd, 0x12250, &lv0);
	HW_IO_READ32(pAd, 0x12254, &lv1);
	HW_IO_READ32(pAd, 0x12258, &lv2);
	HW_IO_READ32(pAd, 0x1225c, &lv3);
	HW_IO_READ32(pAd, 0x12260, &lv4);
	HW_IO_READ32(pAd, 0x12264, &lv5);
	HW_IO_READ32(pAd, 0x12268, &lv6);
	HW_IO_READ32(pAd, 0x1226c, &lv7);
	HW_IO_READ32(pAd, 0x12270, &lv8);
	HW_IO_READ32(pAd, 0x12274, &lv9);
	HW_IO_READ32(pAd, 0x12278, &lv10);
	TotalIPI = lv0 + lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 + lv9 + lv10;

	if (TotalIPI != 0)
		Noisy = ((lv9 + lv10) * 100 / (TotalIPI));

	pAd->BgndScanCtrl.Noisy = Noisy;
	pAd->BgndScanCtrl.IPIIdleTime = TotalIPI;
	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, */
	/* ("%s Band0:lv0~5 %d, %d, %d, %d, %d, %d  lv6~10 %d, %d, %d, %d, %d tatol=%d, Noisy=%d, BusyTime=%d, MyTxAir=%d, MyRxAir=%d\n", */
	/* __func__, lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, TotalIPI, Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime)); */
	HW_IO_READ32(pAd, PHY_RXTD_12, &CrValue);
	CrValue |= (1 << B0IrpiSwCtrlOnlyOffset); /*29*/
	HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);/* Cleaer */
	HW_IO_WRITE32(pAd, PHY_RXTD_12, CrValue);/* Trigger again */

	if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_LISTEN)
		return;

	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Noise =%d, ChBusy=%d, MyTxAirTime=%d, MyRxAirTime=%d\n", Noisy, ChBusyTime, MyTxAirTime, MyRxAirTime)); */
	if ((pAd->BgndScanCtrl.DriverTrigger) && ((Noisy > pAd->BgndScanCtrl.NoisyTH) && (TotalIPI > pAd->BgndScanCtrl.IPIIdleTimeTH))) {
		if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_IDLE) {
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
		} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			pAd->BgndScanCtrl.IsSwitchChannel = TRUE;
			pAd->BgndScanCtrl.ScanType = TYPE_BGND_CONTINUOUS_SCAN;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_CNCL, 0, NULL, 0);
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_CONTINUOUS_SCAN);
			RTMP_MLME_HANDLER(pAd);
			/* BackgroundScanStart(pAd, 2); */
		}
	} else if (BgndScanCtrl->BgndScanStatMachine.CurrState == BGND_SCAN_WAIT) {
		pAd->BgndScanCtrl.PartialScanIntervalCount++;

		if  (pAd->BgndScanCtrl.PartialScanIntervalCount >= pAd->BgndScanCtrl.PartialScanInterval
			 && (MyTxAirTime + MyRxAirTime < DefaultMyAirtimeUsageThreshold)) {
			pAd->BgndScanCtrl.PartialScanIntervalCount = 0;
			MlmeEnqueue(pAd, BGND_SCAN_STATE_MACHINE, BGND_SCAN_REQ, 0, NULL, TYPE_BGND_PARTIAL_SCAN);
			RTMP_MLME_HANDLER(pAd);
		}
	}
}

#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
VOID BfSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	INT idx, start_idx, end_idx, wtbl_len;
	UINT32 wtbl_offset, addr;
	UCHAR *wtbl_raw_dw = NULL;
	struct wtbl_entry wtbl_ent;
	struct wtbl_struc *wtbl = &wtbl_ent.wtbl;
	struct wtbl_tx_rx_cap *trx_cap = &wtbl->trx_cap;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	/* Search BF STA and record it. */
	start_idx = 1;
	end_idx = (cap->WtblHwNum - 1);

	if (enabled == 0) { /* Disable */
		wtbl_len = sizeof(WTBL_STRUC);
		os_alloc_mem(pAd, (UCHAR **)&wtbl_raw_dw, wtbl_len);

		if (!wtbl_raw_dw) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():AllocMem fail!\n",
					  __func__));
			return;
		}

		for (idx = start_idx; idx <= end_idx; idx++) {
			wtbl_ent.wtbl_idx = idx;
			wtbl_ent.wtbl_addr = pAd->mac_ctrl.wtbl_base_addr[0] + idx * pAd->mac_ctrl.wtbl_entry_size[0];

			/* Read WTBL Entries */
			for (wtbl_offset = 0; wtbl_offset <= wtbl_len; wtbl_offset += 4) {
				addr = wtbl_ent.wtbl_addr + wtbl_offset;
				HW_IO_READ32(pAd, addr, (UINT32 *)(&wtbl_raw_dw[wtbl_offset]));
			}

			NdisCopyMemory((UCHAR *)wtbl, &wtbl_raw_dw[0], sizeof(struct wtbl_struc));

			if (trx_cap->wtbl_d2.field.tibf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 1; /* iBF */
			else if (trx_cap->wtbl_d2.field.tebf == 1)
				pAd->BgndScanCtrl.BFSTARecord[idx] = 2; /* eBF */
			else
				pAd->BgndScanCtrl.BFSTARecord[idx] = 0; /* No BF */

			if (pAd->BgndScanCtrl.BFSTARecord[idx] != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Disable wcid %d BF!\n",
						  __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 0, 0, 0); /* Disable BF */
			}
		}

		os_free_mem(wtbl_raw_dw);
	} else {/* enable */
		for (idx = start_idx; idx <= end_idx; idx++) {
			if (pAd->BgndScanCtrl.BFSTARecord[idx] == 1) {/* iBF */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Enable wcid %d iBF!\n", __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 0, 1, 0, 0); /* enable iBF */
			} else if (pAd->BgndScanCtrl.BFSTARecord[idx] == 2) { /* eBF */
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Enable wcid %d eBF!\n", __func__, idx));
				CmdTxBfTxApplyCtrl(pAd, idx, 1, 0, 0, 0); /* enable eBF */
			}
		}
	}
}
#endif
#ifdef CFG_SUPPORT_MU_MIMO
VOID MuSwitch(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR enabled)
{
	if (enabled == 0)   /* Disable */
		SetMuEnableProc(pAd, "0");
	else   /* Enable */
		SetMuEnableProc(pAd, "1");
}
#endif /* CFG_SUPPORT_MU_MIMO */
