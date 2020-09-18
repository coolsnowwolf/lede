
#ifdef SNIFFER_MIB_CMD

#include "rt_config.h"

VOID sniffer_timeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

BUILD_TIMER_FUNCTION(sniffer_timeout);

INT sniffer_channel_restore(IN RTMP_ADAPTER *pAd)
{
	INT bw, ch;
	pAd->hw_cfg.bbp_bw = pAd->ApCfg.sniffer_mib_ctrl.bbp_bw;
#ifdef DOT11_VHT_AC	 
	pAd->CommonCfg.vht_cent_ch = pAd->ApCfg.sniffer_mib_ctrl.vht_cent_ch;
#endif /* DOT11_VHT_AC */
	pAd->CommonCfg.CentralChannel = pAd->ApCfg.sniffer_mib_ctrl.CentralChannel;
	pAd->CommonCfg.Channel = pAd->ApCfg.sniffer_mib_ctrl.Channel;

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

	DBGPRINT(RT_DEBUG_TRACE, ("ch_restore - End of Monitor, restore to %dMHz channel %d\n",
		bw, ch));

	ASSERT((ch != 0));
	AsicSwitchChannel(pAd, ch, FALSE); 
	AsicLockChannel(pAd, ch);

	return TRUE;
	
}

void sniffer_timeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl = (SNIFFER_MIB_CTRL *)FunctionContext;
	PRTMP_ADAPTER  pAd  = psniffer_mib_ctrl->pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return;
	}
	
	//set back the sniffer mode to off
#ifdef CONFIG_SNIFFER_SUPPORT		
	Set_MonitorMode_Proc(pAd,"0");
#endif /* CONFIG_SNIFFER_SUPPORT */

	//if (psniffer_mib_ctrl->scan_channel != 0) // restore to original channel
	{
		//sniffer_channel_restore(pAd);
		RTEnqueueInternalCmd(pAd, CMDTHREAD_CHAN_RESTORE, NULL, 0);
	}
	
}


#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
VOID vSnifferMacTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
	INT32 i4AvgRssi = 0;
	SNIFFER_MAC_CTRL *pMACEntry = (SNIFFER_MAC_CTRL *)FunctionContext;
	PSNIFFER_MAC_NOTIFY_T pMACEntryNotify = NULL;
	UCHAR bssid[MAC_ADDR_LEN] = {0}; //report zero bssid, to prevent too many check

	if(pMACEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():%d: pMACEntry is NULL!\n", __FUNCTION__,__LINE__));
		return;
	}

	pMACEntryNotify = &pMACEntry->rNotify;
	if(pMACEntryNotify->i4RxPacketConut > 0)
	{
		i4AvgRssi = pMACEntryNotify->i4RssiAccum/pMACEntryNotify->i4RxPacketConut;
		wext_send_event(pMACEntryNotify->pNetDev,pMACEntry->MACAddr,bssid,pMACEntryNotify->u4Channel,i4AvgRssi,FBT_LINK_STA_FOUND_NOTIFY);

		// Only clear data for next collection.
		NdisZeroMemory(pMACEntryNotify,sizeof(*pMACEntryNotify));
	}
}
BUILD_TIMER_FUNCTION(vSnifferMacTimeout);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */

VOID sniffer_mib_ctrlInit(IN PRTMP_ADAPTER pAd)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	NdisZeroMemory(psniffer_mib_ctrl, sizeof(SNIFFER_MIB_CTRL));
	psniffer_mib_ctrl->AgeOutTime = 100 ; //set default 100 ms
	psniffer_mib_ctrl->pAd = pAd;

	RTMPInitTimer(pAd, &psniffer_mib_ctrl->AgeOutTimer, 
					GET_TIMER_FUNCTION(sniffer_timeout), psniffer_mib_ctrl, FALSE);
	psniffer_mib_ctrl->AgeOutTimer_Running = FALSE;
	
	NdisAllocateSpinLock(pAd, &psniffer_mib_ctrl->MAC_ListLock);
	
	DlListInit(&psniffer_mib_ctrl->MAC_List);

	psniffer_mib_ctrl->MAC_ListNum = 0 ; // if we don't set MAC to listen, we expect to sniffer all
}


VOID sniffer_mib_ctrlExit(IN PRTMP_ADAPTER pAd)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	BOOLEAN Cancelled;
	SNIFFER_MAC_CTRL *pMACEntry, *pMACEntryTmp;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;

	if(psniffer_mib_ctrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&psniffer_mib_ctrl->AgeOutTimer, &Cancelled);			
		psniffer_mib_ctrl->AgeOutTimer_Running = FALSE;
	}
	RTMPReleaseTimer(&psniffer_mib_ctrl->AgeOutTimer, &Cancelled);
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
	DlListForEachSafe(pMACEntry, pMACEntryTmp, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
		RTMPReleaseTimer(&pMACEntry->rNotifyTimer, &Cancelled);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
		DlListDel(&pMACEntry->List);
		os_free_mem(NULL, pMACEntry);
	}
	DlListInit(&psniffer_mib_ctrl->MAC_List);
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);

	NdisFreeSpinLock(&psniffer_mib_ctrl->MAC_ListLock);	
}

INT exsta_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	BOOLEAN find_list = FALSE;
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	SNIFFER_MAC_CTRL *pMACEntry;
	UCHAR	macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT	i;

	BOOLEAN Cancelled;
	
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	if (psniffer_mib_ctrl->MAC_ListNum >= (MAX_NUM_OF_SNIFFER_MAC - 1))
    	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : sniffer mac table is full, and no more entry can join the list!\n",__FUNCTION__));
		return FALSE;
	}
	
	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	//need to cancel timer ?
	if(psniffer_mib_ctrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&psniffer_mib_ctrl->AgeOutTimer, &Cancelled);
		psniffer_mib_ctrl->AgeOutTimer_Running = FALSE;
	}
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
	DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
		if (MAC_ADDR_EQUAL(pMACEntry->MACAddr, macAddr))
		{
			find_list = TRUE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s : duplicate mac =%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,PRINT_MAC(macAddr)));			
			break;
		}
	}
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);

	if (find_list == FALSE)
		os_alloc_mem(NULL, (UCHAR **)&pMACEntry, sizeof(*pMACEntry));
	
	if (!pMACEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	if (find_list == FALSE) 
	{
		RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
		NdisZeroMemory(pMACEntry,sizeof(*pMACEntry));
		NdisMoveMemory(pMACEntry->MACAddr, macAddr, MAC_ADDR_LEN);
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
		RTMPInitTimer(pAd, &pMACEntry->rNotifyTimer, GET_TIMER_FUNCTION(vSnifferMacTimeout), pMACEntry, TRUE);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
		DlListAddTail(&psniffer_mib_ctrl->MAC_List, &pMACEntry->List);
		psniffer_mib_ctrl->MAC_ListNum ++;
		RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
	}

	return TRUE;
}

INT exsta_list_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	SNIFFER_MAC_CTRL *pMACEntry;
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	INT i=0;
	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
	DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC [%d]=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,i++, PRINT_MAC(pMACEntry->MACAddr)));
	}
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC count=%d\n", __FUNCTION__,i));	

	return TRUE;
}

INT exsta_clear_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	SNIFFER_MAC_CTRL *pMACEntry, *pMACEntryTmp;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;

	if(psniffer_mib_ctrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&psniffer_mib_ctrl->AgeOutTimer, &Cancelled);			
		psniffer_mib_ctrl->AgeOutTimer_Running = FALSE;
	}
	
	RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
	DlListForEachSafe(pMACEntry, pMACEntryTmp, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
	{
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
		RTMPReleaseTimer(&pMACEntry->rNotifyTimer, &Cancelled);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
		DlListDel(&pMACEntry->List);
		os_free_mem(NULL, pMACEntry);
	}
	DlListInit(&psniffer_mib_ctrl->MAC_List);	

	psniffer_mib_ctrl->MAC_ListNum = 0;
	RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);

	return TRUE;
}


INT set_monitor_channel(IN RTMP_ADAPTER *ad, IN INT bw, IN INT channel, IN INT pri_idx)
{
	UINT8 ext_ch;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): bw = %d, channel = %d, pri_idx = %d\n", __FUNCTION__, bw, channel, pri_idx));
	
	ad->CommonCfg.Channel = channel;
	if (bw == BW_40) {
		if (pri_idx == 0) {
			ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel + 2;
			ext_ch = EXTCHA_ABOVE;
		} else if (pri_idx == 1) {
			ext_ch = EXTCHA_BELOW;
			if (channel == 14) {
				ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel - 1;
			} else {
				ad->CommonCfg.CentralChannel = ad->CommonCfg.Channel - 2;
			}
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("pri_idx(%d) is invalid\n"));
			return FALSE;
		}
	}
	else if (bw == BW_20) {
		ad->CommonCfg.CentralChannel = channel;
	} else if ((bw == BW_80) && channel > 14) {
#ifdef DOT11_VHT_AC
		ad->CommonCfg.vht_bw = VHT_BW_80;
		ad->CommonCfg.vht_cent_ch = vht_cent_ch_freq(ad, ad->CommonCfg.Channel);
#endif
	} else {
		return FALSE;
	}

	bbp_set_bw(ad, bw);

	/* TX/Rx : control channel setting */
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (ad->chipCap.hif_type == HIF_RTMP || ad->chipCap.hif_type == HIF_RLT)
		rtmp_mac_set_ctrlch(ad, ext_ch);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (ad->chipCap.hif_type == HIF_MT)
		mt_mac_set_ctrlch(ad, ext_ch);
#endif /* MT_MAC */

	bbp_set_ctrlch(ad, ext_ch);

#ifdef DOT11_VHT_AC
	if (ad->CommonCfg.BBPCurrentBW == BW_80)
		ad->hw_cfg.cent_ch = ad->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		ad->hw_cfg.cent_ch = ad->CommonCfg.CentralChannel;

	AsicSwitchChannel(ad, ad->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(ad, ad->hw_cfg.cent_ch);

	return TRUE;
}


/* trigger sniffer timmer*/
INT exsta_scan_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	BOOLEAN Cancelled;
	ULONG start = 0;
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
	SNIFFER_MAC_CTRL *pMACEntry = NULL;
	PSNIFFER_MAC_NOTIFY_T pMACEntryNotify = NULL;
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	start = simple_strtol(arg, 0, 10);

	if(psniffer_mib_ctrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&psniffer_mib_ctrl->AgeOutTimer, &Cancelled);
		psniffer_mib_ctrl->AgeOutTimer_Running = FALSE;
#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
		RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
		DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
		{
			if(pMACEntry)
			{
				RTMPCancelTimer(&pMACEntry->rNotifyTimer, &Cancelled);
			}
		}
		RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
	}

	if(start)
	{
		if (psniffer_mib_ctrl->AgeOutTime)
		{
			RTMPSetTimer(&psniffer_mib_ctrl->AgeOutTimer, psniffer_mib_ctrl->AgeOutTime);
			psniffer_mib_ctrl->AgeOutTimer_Running = TRUE;

			psniffer_mib_ctrl->bbp_bw = pAd->CommonCfg.BBPCurrentBW;//pAd->hw_cfg.bbp_bw;
#ifdef DOT11_VHT_AC
			psniffer_mib_ctrl->vht_cent_ch = pAd->CommonCfg.vht_cent_ch;
#endif /* DOT11_VHT_AC */
			psniffer_mib_ctrl->CentralChannel = pAd->CommonCfg.CentralChannel;
			psniffer_mib_ctrl->Channel = pAd->CommonCfg.Channel;

#ifdef DOT11_VHT_AC
			DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->bbp_bw=%d,psniffer_mib_ctrl->CentralChannel=%d,psniffer_mib_ctrl->Channel\n",__FUNCTION__,
			psniffer_mib_ctrl->bbp_bw,psniffer_mib_ctrl->CentralChannel,psniffer_mib_ctrl->Channel));
#else /* DOT11_VHT_AC */
			DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->bbp_bw=%d, psniffer_mib_ctrl->vht_cent_ch=%d,psniffer_mib_ctrl->CentralChannel=%d,psniffer_mib_ctrl->Channel\n",__FUNCTION__,
			psniffer_mib_ctrl->bbp_bw,psniffer_mib_ctrl->vht_cent_ch,psniffer_mib_ctrl->CentralChannel,psniffer_mib_ctrl->Channel));
#endif /* !DOT11_VHT_AC */

			// no cmd to set bw, keep the original bw
			if (psniffer_mib_ctrl->scan_channel == 0)
			{
				;// do nothing, don't switch channel
			}
			else
			{
				if (pAd->hw_cfg.bbp_bw !=BW_20 )
				{
					if (psniffer_mib_ctrl->scan_channel <=7 ) //EXTCHA_ABOVE , ch 1~7 use EXTCHA_ABOVE
					{
						set_monitor_channel(pAd,psniffer_mib_ctrl->bbp_bw, psniffer_mib_ctrl->scan_channel, 0);
					}
					else // others use EXTCHA_BELOW
					{
						set_monitor_channel(pAd,psniffer_mib_ctrl->bbp_bw, psniffer_mib_ctrl->scan_channel, 1);
					}
				}
				else //BW_20 case
				{
					set_monitor_channel(pAd,psniffer_mib_ctrl->bbp_bw, psniffer_mib_ctrl->scan_channel, 0);				
				}
			}

#ifdef CONFIG_SNIFFER_SUPPORT
#ifdef ALL_NET_EVENT
			RTMP_SEM_LOCK(&psniffer_mib_ctrl->MAC_ListLock);
			DlListForEach(pMACEntry, &psniffer_mib_ctrl->MAC_List, SNIFFER_MAC_CTRL, List)
			{
				if(pMACEntry)
				{
					pMACEntryNotify = &pMACEntry->rNotify;
					NdisZeroMemory(pMACEntryNotify,sizeof(*pMACEntryNotify));
					RTMPSetTimer(&pMACEntry->rNotifyTimer, SNIFFER_MAC_TIMEOUT);
				}
			}
			RTMP_SEM_UNLOCK(&psniffer_mib_ctrl->MAC_ListLock);
#endif /* ALL_NET_EVENT */
#endif /* CONFIG_SNIFFER_SUPPORT */
			Set_MonitorMode_Proc(pAd,"2");
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->AgeOutTime=%lu, not start timer!\n",__FUNCTION__,psniffer_mib_ctrl->AgeOutTime));
		}
	}
	return TRUE;
}

INT set_mib_passive_scan_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;

	ULONG scan_type = (ULONG) simple_strtol(arg, 0, 10);	

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	if (scan_type == PASSIVE_TYPE)
	{
		psniffer_mib_ctrl->scan_type = PASSIVE_TYPE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: not implement yet !, force set to PASSIVE_TYPE\n",__FUNCTION__));
		psniffer_mib_ctrl->scan_type = PASSIVE_TYPE;	
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->scan_type=%u\n",__FUNCTION__,psniffer_mib_ctrl->scan_type));

	return TRUE;
}

INT set_mib_scan_interval_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	ULONG interval = (ULONG) simple_strtol(arg, 0, 10);	
	
	psniffer_mib_ctrl->AgeOutTime = interval;

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==> psniffer_mib_ctrl->AgeOutTime=%u\n",__FUNCTION__,psniffer_mib_ctrl->AgeOutTime));
	
	return TRUE;
}

INT set_mib_scan_scope_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	ULONG scope = (ULONG) simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	if (scope == SCAN_ONE)
	{
		psniffer_mib_ctrl->scan_scope = SCAN_ONE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: not implement yet !, force set to SCAN_ONE\n",__FUNCTION__));
		psniffer_mib_ctrl->scan_scope = SCAN_ONE;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->scan_scope=%u\n",__FUNCTION__,psniffer_mib_ctrl->scan_scope));
	
	return TRUE;
}

INT set_mib_scan_channel_proc(IN RTMP_ADAPTER *pAd, IN RTMP_STRING *arg)
{
	PSNIFFER_MIB_CTRL psniffer_mib_ctrl;
	psniffer_mib_ctrl = &pAd->ApCfg.sniffer_mib_ctrl;
	ULONG channel;

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ==>\n",__FUNCTION__));

	channel = (ULONG) simple_strtol(arg, 0, 10);
	if ((channel>0) && (channel <=14))
	{
		psniffer_mib_ctrl->scan_channel = channel;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: ==> error channel=%u, force set to channel 0\n",__FUNCTION__,channel));
		psniffer_mib_ctrl->scan_channel = 0; // 0 means current channel
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s: psniffer_mib_ctrl->scan_channel = %u\n",__FUNCTION__,psniffer_mib_ctrl->scan_channel));
	
	return TRUE;
}

#endif /* SNIFFER_MIB_CMD */


