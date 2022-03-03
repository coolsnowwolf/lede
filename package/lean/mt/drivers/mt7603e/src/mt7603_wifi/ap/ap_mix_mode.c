#include "rt_config.h"
#include "mix_mode.h"


static INT scan_ch_restore_for_MixMode(RTMP_ADAPTER *pAd, UCHAR OpMode)
{
	INT bw, ch;
		

        if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
                bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);

#ifdef DOT11_VHT_AC
	if (pAd->hw_cfg.bbp_bw == BW_80)
		ch = pAd->CommonCfg.vht_cent_ch;
	else 
#endif /* DOT11_VHT_AC */ 
	if (pAd->hw_cfg.bbp_bw == BW_40)
		ch = pAd->CommonCfg.CentralChannel;
        else
		ch = pAd->CommonCfg.Channel;

	switch(pAd->CommonCfg.BBPCurrentBW)
	{
		case BW_80:
			bw = 80;
			break;
		case BW_40:
			bw = 40;
			break;
		case BW_10:
			bw = 10;
			break;
		case BW_20:
		default:
			bw =20;
			break;
	}
		

	ASSERT((ch != 0));
       AsicSwitchChannel(pAd, ch, FALSE); 
       AsicLockChannel(pAd, ch);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to %dMHz channel %d, Total BSS[%02d]\n",
				bw, ch, pAd->ScanTab.BssNr));
		

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
	{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if (pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
			{
				if (!ApCliAutoConnectExec(pAd))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Error in  %s\n", __FUNCTION__));
				}
			}			
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
		RTMPResumeMsduTransmission(pAd);

		/* iwpriv set auto channel selection*/
		/* scanned all channels*/
		if (pAd->ApCfg.bAutoChannelAtBootup==TRUE)
		{
			pAd->CommonCfg.Channel = SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
			APStop(pAd);
			APStartUp(pAd);
		}

		if (!((pAd->CommonCfg.Channel > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE) && (pAd->Dot11_H.RDMode != RD_NORMAL_MODE)))
			AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);
	}
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}


static BOOLEAN MixModeChannelCheck(IN PRTMP_ADAPTER pAd, UCHAR channel, struct wifi_dev *wdev)
{

	if(channel == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s pass a invalid channel\n", __FUNCTION__));
		return FALSE;
	}

	/*check channel in channel list*/
	if(ChannelSanity(pAd, channel) == 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("the channel parameter is out of channel list\n"));
		return FALSE;
	}

	/*check channel & phy mode*/
	if((channel < 14 && WMODE_CAP_5G(wdev->PhyMode)) ||
		(channel > 14 && WMODE_CAP_2G(wdev->PhyMode))) {
		DBGPRINT(RT_DEBUG_ERROR,
			("the channel parameter and phy mode is not matched\n"));
		return FALSE;
	}

	return TRUE;
}


static INT MixModeSwitchCH(IN PRTMP_ADAPTER pAd)
{
	UCHAR channel;
	struct wifi_dev *wdev;
	struct peer_info *mon_sta_info = &pAd->MixModeCtrl.sta_info[0];

	if(pAd == NULL)
		return -1;

	channel = pAd->MixModeCtrl.sta_info[0].channel;
	wdev = pAd->MixModeCtrl.pMixModewdev;

	/*check channel with current band channel*/
	if(channel == wdev->channel) {
		DBGPRINT(RT_DEBUG_TRACE,
			("%s:: current channel is the sniffer set channel %d\n", __FUNCTION__, channel));
		return 0;
	}

	/* Disable BCN */
	AsicDisableSync(pAd);

	/*need to poll tx Q unitl empty*/

	/* Suspend MSDU transmission here */
	RTMPSuspendMsduTransmission(pAd);

	bbp_set_bw(pAd, mon_sta_info->bw);

	/*Switch Channel*/
	AsicSwitchChannel(pAd, mon_sta_info->center_channel, FALSE);
	AsicLockChannel(pAd, mon_sta_info->center_channel);

	DBGPRINT(RT_DEBUG_OFF,
		("%s:: switch to channel %d\n", __FUNCTION__, channel));


	return 0;
}

static CHAR MixModeCalcAvgRssi(RTMP_ADAPTER *pAd, UCHAR idx)
{
	UCHAR mnt_idx = 0;
	LONG avgRssi = -127;

	struct peer_info *sta =  &pAd->MixModeCtrl.sta_info[mnt_idx];
	LONG mgmt_rssi = sta->mnt_sta.frm[FC_TYPE_MGMT].rssi;
	LONG data_rssi = sta->mnt_sta.frm[FC_TYPE_DATA].rssi;
	LONG cntl_rssi = sta->mnt_sta.frm[FC_TYPE_CNTL].rssi;
	LONG total = sta->mnt_sta.Count;

	/*
	* weight * rssi
	* But need to check vaule
	* it cannot overflow
	*/
	if (total)
		avgRssi = (mgmt_rssi + data_rssi + cntl_rssi) / total;

	DBGPRINT(RT_DEBUG_OFF,
		("%s:: %02x:%02x:%02x:%02x:%02x:%02x, avgRssi=%ld\n", __FUNCTION__, PRINT_MAC(sta->mac_addr), avgRssi));

	return avgRssi;
}

static INT MixModeSniffer(RTMP_ADAPTER *pAd, UINT8 enable)
{
	if (enable == TRUE)
		pAd->MixModeCtrl.current_monitor_mode = MIX_MODE_FULL;
	else if (enable == FALSE)
		pAd->MixModeCtrl.current_monitor_mode = MIX_MODE_OFF;

	DBGPRINT(RT_DEBUG_OFF,
		("%s(): set Current Monitor Mode = %d\n",
		__FUNCTION__, pAd->MixModeCtrl.current_monitor_mode));

	switch(pAd->MixModeCtrl.current_monitor_mode)
	{
	/*reset to normal */
	case MIX_MODE_OFF:
		pAd->ApCfg.BssType = BSS_INFRA;
		AsicSetRxFilter(pAd);
		break;

	/*fully report, Enable Rx with promiscuous reception*/
	case MIX_MODE_FULL:
		pAd->ApCfg.BssType = BSS_MONITOR;
		AsicSetRxFilter(pAd);
		break;
	default:
		DBGPRINT(RT_DEBUG_OFF,
			("MIX MODE index ONLY be set to 0 or 2\n"));
		break;
	}

	return 0;
}

//extern INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode);

static VOID MixModeRestoreCH(IN PRTMP_ADAPTER pAd)
{
	if(pAd->MixModeCtrl.pMixModewdev != NULL &&
		(pAd->MixModeCtrl.sta_info[0].channel != (pAd->MixModeCtrl.pMixModewdev)->channel))
		/*scan ch restore for MixMode will do enable BCN*/
		scan_ch_restore_for_MixMode(pAd, OPMODE_AP);
}

static VOID MixModeReset(IN PRTMP_ADAPTER pAd)
{
	pAd->MixModeCtrl.MixModeOn = FALSE;
	pAd->MixModeCtrl.pMixModewdev = NULL;
	pAd->MixModeCtrl.MixModeStatMachine.CurrState =  MIX_MODE_STATE_INIT;
	pAd->MixModeCtrl.ioctl_if = -1;
}

static VOID MixModeListenExit(RTMP_ADAPTER *pAd)
{
	BOOLEAN disbleSniffer = FALSE;
	BOOLEAN mixmode_done = TRUE;

	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	if(MixModeSniffer(pAd,disbleSniffer) !=0)
		DBGPRINT(RT_DEBUG_ERROR, ("%s fail by sniffer mode\n",__FUNCTION__));

	MixModeRestoreCH(pAd);
	MixModeReset(pAd);

	DBGPRINT(RT_DEBUG_OFF, ("%s::Exit mix mode\n", __FUNCTION__));
	/* send event to userspace */
	RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				RT_MIX_MODE_EVENT_FLAG,
				NULL,
				(char *)&mixmode_done,
				sizeof(mixmode_done));
}

static VOID MixModeListenCancel(RTMP_ADAPTER *pAd,MLME_QUEUE_ELEM *pElem)
{
	if(pAd == NULL)
		return;

	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	MixModeListenExit(pAd);
}


static VOID MixModeListenTimeout(RTMP_ADAPTER *pAd,MLME_QUEUE_ELEM *pElem)
{
	if(pAd == NULL)
		return;

	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	MixModeListenExit(pAd);

}


static VOID MixModeEnterListen(RTMP_ADAPTER *pAd,MLME_QUEUE_ELEM *pElem)
{
	INT32 ret;
	BOOLEAN enbleSniffer = TRUE;

	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	if(pAd == NULL)
		return;

	/*switch channel*/
	ret = MixModeSwitchCH(pAd);
	if(ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s fail by switch channel\n",__FUNCTION__));
		MixModeReset(pAd);
		return;
	}

	/*enter sniffer mode*/
	ret = MixModeSniffer(pAd,enbleSniffer);
	if(ret != 0) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s fail by sniffer mode\n",__FUNCTION__));
		MixModeRestoreCH(pAd);
		MixModeReset(pAd);
		return;
	}
	pAd->MixModeCtrl.MixModeOn = TRUE;
	pAd->MixModeCtrl.MixModeStatMachine.CurrState = MIX_MODE_STATE_RUNNING;
	DBGPRINT(RT_DEBUG_OFF, ("pAd->MixModeCtrl.sta_info[0].duration = %d\n", pAd->MixModeCtrl.sta_info[0].duration));
	RTMPSetTimer(&pAd->MixModeCtrl.MixModeTimer, pAd->MixModeCtrl.sta_info[0].duration);
}

static VOID MixModeStateMachineInit(IN RTMP_ADAPTER *pAd, IN STATE_MACHINE *Sm, OUT STATE_MACHINE_FUNC Trans[])
{
	/*Init*/
	StateMachineInit(Sm,(STATE_MACHINE_FUNC *)Trans,MIX_MODE_STATE_MAX, MIX_MODE_MSG_MAX, (STATE_MACHINE_FUNC)Drop, MIX_MODE_STATE_INIT, MIX_MODE_STATE_BASE);

	/*Idle*/
	StateMachineSetAction(Sm, MIX_MODE_STATE_INIT, MIX_MODE_MSG_LISTEN, (STATE_MACHINE_FUNC)MixModeEnterListen);
	/*Running*/
	StateMachineSetAction(Sm, MIX_MODE_STATE_RUNNING, MIX_MODE_MSG_CANCLE, (STATE_MACHINE_FUNC)MixModeListenCancel);
	StateMachineSetAction(Sm, MIX_MODE_STATE_RUNNING, MIX_MODE_MSG_TIMEOUT, (STATE_MACHINE_FUNC)MixModeListenTimeout);
}


VOID MixModeTimeout(IN PVOID SystemSpecific1, IN PVOID FunctionContext,
			IN PVOID SystemSpecific2, IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	if(pAd == NULL)
		return;

	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	MlmeEnqueue(pAd, MIX_MODE_STATE_MACHINE, MIX_MODE_MSG_TIMEOUT, 0, NULL, 0);
    	RTMP_MLME_HANDLER(pAd);
}

VOID MixModeDebugInfo(IN PRTMP_ADAPTER pAd, IN int idx)
{
	MIX_MODE_CTRL *MixModeCtrl = &pAd->MixModeCtrl;
	DBGPRINT(RT_DEBUG_OFF,
		("MixMode status = %d\nMixMode CurrState = %lu\nMixMode CH = %d\n"
		"targetMac = %02x:%02x:%02x:%02x:%02x:%02x\nMixMode Duration = %d\n",
		MixModeCtrl->MixModeOn, MixModeCtrl->MixModeStatMachine.CurrState,
		MixModeCtrl->sta_info[idx].channel,
		PRINT_MAC(MixModeCtrl->sta_info[idx].mac_addr),
		MixModeCtrl->sta_info[idx].duration));

}

VOID MixModeProcessData(RTMP_ADAPTER *pAd, void *rxBlk)
{
	RX_BLK *pRxBlk = (RX_BLK*)rxBlk;
	PHEADER_802_11 pHeader;
	CHAR avg_rssi;
	struct peer_info *mon_sta_info = &pAd->MixModeCtrl.sta_info[0];
	FRAME_CONTROL *fc;
	struct MNT_STA_INFO *mnt_sta;

//	DBGPRINT(RT_DEBUG_OFF, ("%s()\n", __FUNCTION__));

	ASSERT(pRxBlk->pRxPacket);

	pHeader = pRxBlk->pHeader;
	fc = &pHeader->FC;
	mnt_sta = &mon_sta_info->mnt_sta;

//	DBGPRINT(RT_DEBUG_OFF, ("mon_sta_info->mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(mon_sta_info->mac_addr)));

	if (NdisEqualMemory(pHeader->Addr2, mon_sta_info->mac_addr, MAC_ADDR_LEN)) {
		if (fc->Type == FC_TYPE_DATA)
			Update_Rssi_Sample(pAd, &mon_sta_info->RssiSample, &pRxBlk->rx_signal,
				    pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
		else
			Update_Rssi_Sample(pAd, &mon_sta_info->RssiSample, &pRxBlk->rx_signal,
				    0, pRxBlk->rx_rate.field.BW);

		avg_rssi = RTMPAvgRssi(pAd, &mon_sta_info->RssiSample);

		if (fc->Type < FC_TYPE_RSVED) {
			mnt_sta->frm[fc->Type].cnt++;
			mnt_sta->frm[fc->Type].rssi += avg_rssi;
			mnt_sta->Count++;
		}
		DBGPRINT(RT_DEBUG_OFF,
			("fctype = %d ,mgmt = %ld , data = %ld , cnt = %ld "
			"frame FROM %02x:%02x:%02x:%02x:%02x:%02x TO "
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			fc->Type,
			mnt_sta->frm[FC_TYPE_MGMT].cnt,
			mnt_sta->frm[FC_TYPE_DATA].cnt,
			mnt_sta->frm[FC_TYPE_CNTL].cnt,
			PRINT_MAC(pHeader->Addr2),
			PRINT_MAC(pHeader->Addr1)));
	}

	return;
}

/*
 * 1. check parameters
 * 2. check other SMs
 * 3. run SM
*/
INT MixModeSet(IN PRTMP_ADAPTER pAd, struct mix_peer_parameter *sta_info, IN UCHAR idx)
{
	POS_COOKIE pObj;
	struct wifi_dev *wdev;
	struct peer_info *mon_sta_info = &pAd->MixModeCtrl.sta_info[0];

	if (pAd == NULL)
		return -EFAULT;

	pObj = (POS_COOKIE)pAd->OS_Cookie;
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;

	if (wdev == NULL)
		return -EFAULT;

	pAd->MixModeCtrl.pMixModewdev = wdev;
	pAd->MixModeCtrl.ioctl_if = pObj->ioctl_if;

	if (sta_info->mac_addr == NULL)
		return -EFAULT;

	if (NdisEqualMemory(sta_info->mac_addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN) ||
		NdisEqualMemory(sta_info->mac_addr, &ZERO_MAC_ADDR[0], MAC_ADDR_LEN)) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s pass a invalid mac %02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(sta_info->mac_addr)));
		return -EFAULT;
	}

	NdisZeroMemory(mon_sta_info, sizeof(struct peer_info));
	NdisCopyMemory(mon_sta_info->mac_addr, sta_info->mac_addr, MAC_ADDR_LEN);

	if (MixModeChannelCheck(pAd, sta_info->channel, wdev))
		mon_sta_info->channel = sta_info->channel;
	else
		return -EFAULT;

	if (sta_info->duration == 0)
		mon_sta_info->duration = MixModeDefualtListenTime;
	else
		mon_sta_info->duration = sta_info->duration;

#ifdef DOT11_VHT_AC
	if (sta_info->channel > 14) {
		mon_sta_info->bw = BW_80;
		mon_sta_info->center_channel = vht_cent_ch_freq(pAd, sta_info->channel);
	} else
#endif
	{
		mon_sta_info->bw = sta_info->bw;
		mon_sta_info->ch_offset = sta_info->ch_offset;

		if(sta_info->bw == BW_40) {
			/*check bw & channel*/
			if ((sta_info->channel < 5 && sta_info->ch_offset == EXTCHA_BELOW)
				|| (sta_info->channel > 9 && sta_info->ch_offset == EXTCHA_ABOVE)) {
				DBGPRINT(RT_DEBUG_ERROR,
					("%s pass a invalid primary channel & second channel\n", __FUNCTION__));
				return -EFAULT;
			}

			if(sta_info->ch_offset == EXTCHA_ABOVE)
				mon_sta_info->center_channel = sta_info->channel + 2;
			else if(sta_info->ch_offset == EXTCHA_BELOW)
				mon_sta_info->center_channel = sta_info->channel - 2;
		} else
			mon_sta_info->center_channel = sta_info->channel;
	}

	/*check other SMs like autsel or bgscan etc*/
	MlmeEnqueue(pAd, MIX_MODE_STATE_MACHINE, MIX_MODE_MSG_LISTEN, 0, NULL, 0);
    	RTMP_MLME_HANDLER(pAd);

	return 0;
}
INT RTMPIoctlQueryMixModeRssi(IN PRTMP_ADAPTER pAd, IN RTMP_IOCTL_INPUT_STRUCT *wrq)
{
	INT Status;
	CHAR avg_rssi;

	avg_rssi = MixModeCalcAvgRssi(pAd, 0);

	wrq->u.data.length = sizeof(avg_rssi);

	Status = copy_to_user(wrq->u.data.pointer, &avg_rssi, wrq->u.data.length);

	if(Status)
		DBGPRINT(RT_DEBUG_ERROR,
		("%s: copy_to_user() fail\n", __FUNCTION__));

	return Status;
}

VOID MixModeCancel(IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;
	if(pAd == NULL)
		return;

	RTMPCancelTimer(&pAd->MixModeCtrl.MixModeTimer,&Cancelled);

	MlmeEnqueue(pAd, MIX_MODE_STATE_MACHINE, MIX_MODE_MSG_CANCLE, 0, NULL, 0);
    	RTMP_MLME_HANDLER(pAd);
}

VOID MixModeInit(IN PRTMP_ADAPTER pAd)
{
	if(pAd == NULL)
		return;

	MixModeReset(pAd);
	MixModeStateMachineInit(pAd, &pAd->MixModeCtrl.MixModeStatMachine, pAd->MixModeCtrl.MixModeStateFunc);
	RTMPInitTimer(pAd, &pAd->MixModeCtrl.MixModeTimer, GET_TIMER_FUNCTION(MixModeTimeout), pAd, FALSE);
	DBGPRINT(RT_DEBUG_TRACE, ("Enter %s\n", __FUNCTION__));
}

VOID MixModeDel(IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;
	if(pAd == NULL)
		return;

	RTMPReleaseTimer(&pAd->MixModeCtrl.MixModeTimer, &Cancelled);
	NdisZeroMemory(&pAd->MixModeCtrl.sta_info, sizeof(pAd->MixModeCtrl.sta_info));

	DBGPRINT(RT_DEBUG_TRACE, ("Exit %s\n", __FUNCTION__));
}

