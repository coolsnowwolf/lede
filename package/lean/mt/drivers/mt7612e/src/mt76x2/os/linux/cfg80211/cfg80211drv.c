/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All related CFG80211 function body.

	History:

***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

extern struct notifier_block cfg80211_netdev_notifier;

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER                    *pAd,
	IN      VOID                            *pData,
	IN      ULONG                            Data);

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER                    *pAd,
	IN      VOID                            *pData,
	IN      ULONG                            Data);


INT CFG80211DRV_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;

	switch(cmd)
	{
		case CMD_RTPRIV_IOCTL_80211_START:
		case CMD_RTPRIV_IOCTL_80211_END:
			/* nothing to do */
			break;

		case CMD_RTPRIV_IOCTL_80211_CB_GET:
			*(VOID **)pData = (VOID *)(pAd->pCfg80211_CB);
			break;

		case CMD_RTPRIV_IOCTL_80211_CB_SET:
			pAd->pCfg80211_CB = pData;
			break;

		case CMD_RTPRIV_IOCTL_80211_CHAN_SET:
			if (CFG80211DRV_OpsSetChannel(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_80211_VIF_CHG:
			if (CFG80211DRV_OpsChgVirtualInf(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_80211_SCAN:
			if (CFG80211DRV_OpsScanCheckStatus(pAd, Data) != TRUE) 
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_80211_SCAN_STATUS_LOCK_INIT:
			CFG80211_ScanStatusLockInit(pAd, Data);
			break;
			
		case CMD_RTPRIV_IOCTL_80211_IBSS_JOIN:
			CFG80211DRV_OpsJoinIbss(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_80211_STA_LEAVE:
			CFG80211DRV_OpsLeave(pAd, Data);
			break;

		case CMD_RTPRIV_IOCTL_80211_STA_GET:
			if (CFG80211DRV_StaGet(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;
		case CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD:
			CFG80211DRV_StaKeyAdd(pAd, pData);
			break;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		case CMD_RTPRIV_IOCTL_80211_P2P_CLIENT_KEY_ADD:
			CFG80211DRV_P2pClientKeyAdd(pAd, pData);
			break;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

		case CMD_RTPRIV_IOCTL_80211_POWER_MGMT_SET:
			CFG80211_setPowerMgmt(pAd, Data);
			break;
			
#ifdef CONFIG_STA_SUPPORT			
		case CMD_RTPRIV_IOCTL_80211_STA_KEY_DEFAULT_SET:
			CFG80211_setStaDefaultKey(pAd, Data);
			break;

#ifdef DOT11W_PMF_SUPPORT
		case CMD_RTPRIV_IOCTL_80211_STA_MGMT_KEY_DEFAULT_SET:
			CFG80211_setStaMgmtDefaultKey(pAd, Data);
			break;
#endif /* DOT11W_PMF_SUPPORT*/

#endif /*CONFIG_STA_SUPPORT*/
		case CMD_RTPRIV_IOCTL_80211_CONNECT_TO:
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE		
			if (Data == RT_CMD_80211_IFTYPE_P2P_CLIENT)
				CFG80211DRV_P2pClientConnect(pAd, pData);
			else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */			
				CFG80211DRV_Connect(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO:
			CFG80211DRV_RegNotify(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_80211_UNREGISTER:
			CFG80211_UnRegister(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_80211_BANDINFO_GET:
		{
			CFG80211_BAND *pBandInfo = (CFG80211_BAND *)pData;
			CFG80211_BANDINFO_FILL(pAd, pBandInfo);
		}
			break;

		case CMD_RTPRIV_IOCTL_80211_SURVEY_GET:
			CFG80211DRV_SurveyGet(pAd, pData);
			break;
			
		case CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET:
			CFG80211DRV_OpsScanExtraIesSet(pAd);	
			break;

		case CMD_RTPRIV_IOCTL_80211_REMAIN_ON_CHAN_SET:			
			CFG80211DRV_OpsRemainOnChannel(pAd, pData, Data);			 		
			break;
							
		case CMD_RTPRIV_IOCTL_80211_CANCEL_REMAIN_ON_CHAN_SET:
			CFG80211DRV_OpsCancelRemainOnChannel(pAd, Data);
			break;

		/* CFG_TODO */
		case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_REG:
			CFG80211DRV_OpsMgmtFrameProbeRegister(pAd, pData, Data);
			break;
			
		/* CFG_TODO */
		case CMD_RTPRIV_IOCTL_80211_ACTION_FRAME_REG:
			CFG80211DRV_OpsMgmtFrameActionRegister(pAd, pData, Data);
			break;

		case CMD_RTPRIV_IOCTL_80211_CHANNEL_LOCK:
			CFG80211_SwitchTxChannel(pAd, Data);
			break;
			
		case CMD_RTPRIV_IOCTL_80211_CHANNEL_RESTORE:
			break;

		case CMD_RTPRIV_IOCTL_80211_MGMT_FRAME_SEND:
			CFG80211_SendMgmtFrame(pAd, pData, Data);
			break;

		case CMD_RTPRIV_IOCTL_80211_CHANNEL_LIST_SET:
			return CFG80211DRV_OpsScanSetSpecifyChannel(pAd,pData, Data);			

#ifdef CONFIG_AP_SUPPORT
		case CMD_RTPRIV_IOCTL_80211_BEACON_SET:
			CFG80211DRV_OpsBeaconSet(pAd, pData);			
			break;
		
		case CMD_RTPRIV_IOCTL_80211_BEACON_ADD:
			CFG80211DRV_OpsBeaconAdd(pAd, pData);
			break;
			
		case CMD_RTPRIV_IOCTL_80211_BEACON_DEL:	
		{
			INT i;
			for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
                		pAd->ApCfg.MBSSID[MAIN_MBSSID].TimBitmaps[i] = 0;
			if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL)
			{
				os_free_mem(NULL, pAd->cfg80211_ctrl.beacon_tail_buf);
				pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
			}
			pAd->cfg80211_ctrl.beacon_tail_len = 0;
		}
			break;

                case CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD:
                        CFG80211DRV_ApKeyAdd(pAd, pData);
                        break;

                case CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD:
                        CFG80211DRV_RtsThresholdAdd(pAd, Data);
                        break;

                case CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD:
                        CFG80211DRV_FragThresholdAdd(pAd, Data);
                        break;

                case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL:
                        CFG80211DRV_ApKeyDel(pAd, pData);
                        break;

                case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET:
                        CFG80211_setApDefaultKey(pAd, Data);
                        break;

                case CMD_RTPRIV_IOCTL_80211_PORT_SECURED:
                        CFG80211_StaPortSecured(pAd, pData, Data);
                        break;

                case CMD_RTPRIV_IOCTL_80211_AP_STA_DEL:
                        CFG80211_ApStaDel(pAd, pData);
                        break;
#endif /* CONFIG_AP_SUPPORT */			

		case CMD_RTPRIV_IOCTL_80211_CHANGE_BSS_PARM:
			CFG80211DRV_OpsChangeBssParm(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_80211_AP_PROBE_RSP_EXTRA_IE:
			break;
			
		case CMD_RTPRIV_IOCTL_80211_BITRATE_SET:
			break;
			
        	case CMD_RTPRIV_IOCTL_80211_RESET:
            		CFG80211_reSetToDefault(pAd);
            		break;
							
		case CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT:
		{
		    /*
		    CFG_TODO: For Scan_req per netdevice 
			 struct wireless_dev *pWdev = pAd->pCfg80211_CB->pCfg80211_Wdev;			
			 if (RTMPEqualMemory(pNetDev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			*/
			PNET_DEV pNetDev = (PNET_DEV) pData;
			if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("CFG_SCAN: close the scan cmd in device close phase\n"));
				CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
				pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
			}
		}	
			break;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
                case CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET:
                        CFG80211DRV_SetP2pCliAssocIe(pAd, pData, Data);
                        break;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

		case CMD_RTPRIV_IOCTL_80211_VIF_ADD:
			if (CFG80211DRV_OpsVifAdd(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

	        case CMD_RTPRIV_IOCTL_80211_VIF_DEL:
			RTMP_CFG80211_VirtualIF_Remove(pAd, pData, Data);
            		break;


#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT			
		case CMD_RTPRIV_IOCTL_80211_ANDROID_PRIV_CMD:
			//rt_android_private_command_entry(pAd, );
			break;
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT
		case CMD_RTPRIV_IOCTL_80211_SEND_WIRELESS_EVENT:
			CFG80211_SendWirelessEvent(pAd, pData);
			break;
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */

#ifdef RFKILL_HW_SUPPORT
				case CMD_RTPRIV_IOCTL_80211_RFKILL:
				{
					UINT32 data = 0;
					BOOLEAN active;
		
					/* Read GPIO pin2 as Hardware controlled radio state */
					RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
					active = !!(data & 0x04);
		
					if (!active)
					{
						RTMPSetLED(pAd, LED_RADIO_OFF);
						*(UINT8 *)pData = 0;
					}
					else
						*(UINT8 *)pData = 1;
				}
					break;
#endif /* RFKILL_HW_SUPPORT */

		default:
			return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
        VOID                                            *pAdOrg,
        VOID                                            *pData,
		BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE		
	PNET_DEV pNewNetDev = (PNET_DEV) pData;
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       pDevEntry = NULL;
	PLIST_ENTRY		        pListEntry = NULL;

	/* Search the CFG80211 VIF List First */
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL)
	{
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			break;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
		
	/* Check The Registration is for VIF Device */	
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pDevEntry != NULL))
	{
		if (isReg)
			pDevEntry->Cfg80211ProbeReqCount++;
		else 
			pDevEntry->Cfg80211ProbeReqCount--;	
		
		if (pDevEntry->Cfg80211ProbeReqCount > 0)
			pDevEntry->Cfg80211RegisterProbeReqFrame = TRUE;
		else 
			pDevEntry->Cfg80211RegisterProbeReqFrame = FALSE;			
			
#ifndef RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE			
		return;
#endif /* RT_CFG80211_P2P_STATIC_CONCURRENT_DEVICE */

	}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	
	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount++;
	else 
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount--;	

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = TRUE;
	else 
	{
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	}	
	
	DBGPRINT(RT_DEBUG_INFO, ("[%d] pAd->Cfg80211RegisterProbeReqFrame=%d[%d]\n", 
		isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame, 
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount));
}

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
        VOID                                            *pAdOrg,
        VOID                                            *pData,
		BOOLEAN                                          isReg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE		
	PNET_DEV pNewNetDev = (PNET_DEV) pData;	
	PLIST_HEADER  pCacheList = &pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList;
	PCFG80211_VIF_DEV       pDevEntry = NULL;
	PLIST_ENTRY		        pListEntry = NULL;

	/* Search the CFG80211 VIF List First */
	pListEntry = pCacheList->pHead;
	pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	while (pDevEntry != NULL)
	{
		if (RTMPEqualMemory(pDevEntry->net_dev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
			break;

		pListEntry = pListEntry->pNext;
		pDevEntry = (PCFG80211_VIF_DEV)pListEntry;
	}
		
	/* Check The Registration is for VIF Device */	
	if ((pAd->cfg80211_ctrl.Cfg80211VifDevSet.vifDevList.size > 0) &&
		(pDevEntry != NULL))
	{
		if (isReg)
			pDevEntry->Cfg80211ActionCount++;
		else 
			pDevEntry->Cfg80211ActionCount--;	
		
		if (pDevEntry->Cfg80211ActionCount > 0)
			pDevEntry->Cfg80211RegisterActionFrame = TRUE;
		else 
			pDevEntry->Cfg80211RegisterActionFrame = FALSE;			
		
		DBGPRINT(RT_DEBUG_INFO, ("[%d] TYPE pDevEntry->Cfg80211RegisterActionFrame=%d[%d]\n", 
				isReg, pDevEntry->Cfg80211RegisterActionFrame, pDevEntry->Cfg80211ActionCount));
		
		return;
	}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
	
	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount++;
	else 
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount--;	

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = TRUE;
	else 
	{
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	}	
	
	DBGPRINT(RT_DEBUG_INFO, ("[%d] TYPE pAd->Cfg80211RegisterActionFrame=%d[%d]\n", 
		isReg, pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame, 
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount));
}

VOID CFG80211DRV_OpsChangeBssParm(
        VOID                                            *pAdOrg,
        VOID                                            *pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_BSS_PARM *pBssInfo;
	BOOLEAN TxPreamble;

	CFG80211DBG(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	pBssInfo = (CMD_RTPRIV_IOCTL_80211_BSS_PARM *)pData;	

	/* Short Preamble */
	if (pBssInfo->use_short_preamble != -1)
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("%s: ShortPreamble %d\n", __FUNCTION__, pBssInfo->use_short_preamble));
        	pAd->CommonCfg.TxPreamble = (pBssInfo->use_short_preamble == 0 ? Rt802_11PreambleLong : Rt802_11PreambleShort);	
		TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);
		MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);			
	}
	
	/* CTS Protection */
	if (pBssInfo->use_cts_prot != -1)
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("%s: CTS Protection %d\n", __FUNCTION__, pBssInfo->use_cts_prot));
	}
	
	/* Short Slot */
	if (pBssInfo->use_short_slot_time != -1)
	{
		CFG80211DBG(RT_DEBUG_TRACE, ("%s: Short Slot %d\n", __FUNCTION__, pBssInfo->use_short_slot_time));
	}
}

BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChan;
	UINT8 ChanId, IfType, ChannelType;
	UCHAR lock_channel;
#ifdef DOT11_N_SUPPORT
	BOOLEAN FlgIsChanged;
#endif /* DOT11_N_SUPPORT */

/*
 *  enum nl80211_channel_type {
 *	NL80211_CHAN_NO_HT,
 *	NL80211_CHAN_HT20,
 *	NL80211_CHAN_HT40MINUS,
 *	NL80211_CHAN_HT40PLUS
 *  };
 */
	/* init */
	pChan = (CMD_RTPRIV_IOCTL_80211_CHAN *)pData;
	ChanId = pChan->ChanId;
	IfType = pChan->IfType;
	ChannelType = pChan->ChanType;

	if (IfType != RT_CMD_80211_IFTYPE_MONITOR)
	{
		/* get channel BW */
		FlgIsChanged = TRUE;
	
		/* set to new channel BW */
		if (ChannelType == RT_CMD_80211_CHANTYPE_HT20)
		{
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_NONE;
			pAd->CommonCfg.HT_Disable = 0;
		}
		else if (ChannelType == RT_CMD_80211_CHANTYPE_HT40MINUS)
		{
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_BELOW;
			pAd->CommonCfg.HT_Disable = 0;			
		}
		else if	(ChannelType == RT_CMD_80211_CHANTYPE_HT40PLUS)
		{
			/* not support NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS */
			/* i.e. primary channel = 36, secondary channel must be 40 */
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_ABOVE;
			pAd->CommonCfg.HT_Disable = 0;
		}
		else if  (ChannelType == RT_CMD_80211_CHANTYPE_NOHT)
		{
			pAd->CommonCfg.RegTransmitSetting.field.BW = BW_20;
			pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = EXTCHA_NONE;
			pAd->CommonCfg.HT_Disable = 1;	
		}
		
		CFG80211DBG(RT_DEBUG_TRACE, ("80211> HT Disable = %d\n", pAd->CommonCfg.HT_Disable));	
	}
	else
	{
		/* for monitor mode */
		FlgIsChanged = TRUE;
		pAd->CommonCfg.HT_Disable = 0;
		pAd->CommonCfg.RegTransmitSetting.field.BW = BW_40;
	} 

	if (FlgIsChanged == TRUE)
		SetCommonHT(pAd);

	/* switch to the channel with Common Channel */
	pAd->CommonCfg.Channel = ChanId;
	pAd->MlmeAux.Channel = ChanId;

	/* CFG_TODO: for CentralChannel setting */
	//lock_channel = N_SetCenCh(pAd, pAd->CommonCfg.Channel);
	//pAd->MlmeAux.CentralChannel = lock_channel;

	//if (pAd->LatchRfRegs.Channel != pAd->CommonCfg.Channel)
	//{
	//	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	//	AsicLockChannel(pAd, pAd->CommonCfg.Channel);	
	//}	

        if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW)
              pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
        else if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
              pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
        else
        	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;

	bbp_set_bw(pAd, pAd->CommonCfg.RegTransmitSetting.field.BW);
        AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel,FALSE); 
        AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> New CH = %d, New BW = %d with Ext[%d]\n", 
		pAd->CommonCfg.CentralChannel, pAd->CommonCfg.RegTransmitSetting.field.BW,
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA));
	
	if(IfType == RT_CMD_80211_IFTYPE_AP ||
	   IfType == RT_CMD_80211_IFTYPE_P2P_GO)
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> Set the channel in AP Mode\n"));
		return TRUE;
	}
#ifdef CONFIG_STA_SUPPORT
	if ((IfType == RT_CMD_80211_IFTYPE_STATION) && (FlgIsChanged == TRUE))
	{
		/*
			1. Station mode;
			2. New BW settings is 20MHz but current BW is not 20MHz;
			3. New BW settings is 40MHz but current BW is 20MHz;

			Re-connect to the AP due to BW 20/40 or HT/non-HT change.
		*/
		CFG80211DBG(RT_DEBUG_ERROR, ("80211> Set the channel in STA Mode\n"));
	} 

	if (IfType == RT_CMD_80211_IFTYPE_ADHOC)
	{
		/* update IBSS beacon */
		MlmeUpdateTxRates(pAd, FALSE, 0);
		MakeIbssBeacon(pAd);
		AsicEnableIbssSync(pAd);

		Set_SSID_Proc(pAd, (PSTRING)pAd->CommonCfg.Ssid);
	} 

	if (IfType == RT_CMD_80211_IFTYPE_MONITOR)
	{
		/* reset monitor mode in the new channel */
		Set_NetworkType_Proc(pAd, "Monitor");
		RTMP_IO_WRITE32(pAd, RX_FILTR_CFG, pChan->MonFilterFlag);
	} 
#endif /*CONFIG_STA_SUPPORT*/
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsJoinIbss(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_IBSS *pIbssInfo;


	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_IBSS *)pData;
	pAd->StaCfg.bAutoReconnect = TRUE;

	pAd->CommonCfg.BeaconPeriod = pIbssInfo->BeaconInterval;
	Set_SSID_Proc(pAd, (PSTRING)pIbssInfo->Ssid);
#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsLeave(
	VOID						*pAdOrg,
	UINT8						IfType)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;

    MLME_DEAUTH_REQ_STRUCT   DeAuthReq;
    MLME_QUEUE_ELEM *pMsgElem = NULL;

	pAd->StaCfg.bAutoReconnect = FALSE;
	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;

    pAd->MlmeAux.AutoReconnectSsidLen= 32;
    NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

    os_alloc_mem(pAd, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE	
	if (IfType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		COPY_MAC_ADDR(DeAuthReq.Addr, pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid);
	else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */		
		COPY_MAC_ADDR(DeAuthReq.Addr, pAd->CommonCfg.Bssid);
		
    DeAuthReq.Reason = REASON_DEAUTH_STA_LEAVING;
    pMsgElem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
    NdisMoveMemory(pMsgElem->Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
    MlmeDeauthReqAction(pAd, pMsgElem);
    os_free_mem(NULL, pMsgElem);
	pMsgElem = NULL;

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE	 
	if (IfType == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		ApCliLinkDown(pAd, MAIN_MBSSID /*ifIndex*/);
	else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */		
		LinkDown(pAd, FALSE);

#endif /* CONFIG_STA_SUPPORT */
	return TRUE;
}


BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pIbssInfo;

	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;

#ifdef CONFIG_AP_SUPPORT
{
	MAC_TABLE_ENTRY *pEntry;
	ULONG DataRate = 0;
	UINT32 RSSI;


	pEntry = MacTableLookup(pAd, pIbssInfo->MAC);
	if (pEntry == NULL)
		return FALSE;

	/* fill tx rate */
	RtmpDrvRateGet(pAd, pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.ShortGI,
				 pEntry->HTPhyMode.field.BW,pEntry->HTPhyMode.field.MCS,
				 newRateGetAntenna(pEntry->MaxHTPhyMode.field.MCS, pEntry->HTPhyMode.field.MODE),&DataRate);
	DataRate /= 500000;
	DataRate /= 2;

	if ((pEntry->HTPhyMode.field.MODE == MODE_HTMIX) ||
		(pEntry->HTPhyMode.field.MODE == MODE_HTGREENFIELD))
	{
		if (pEntry->HTPhyMode.field.BW)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_BW_40;
		
		if (pEntry->HTPhyMode.field.ShortGI)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_SHORT_GI;

		pIbssInfo->TxRateMCS = pEntry->HTPhyMode.field.MCS;
	}
	else
	{
		pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
		pIbssInfo->TxRateMCS = DataRate*10; /* unit: 100kbps */
	}

	/* fill signal */
	RSSI = RTMPAvgRssi(pAd, &pEntry->RssiSample);
	pIbssInfo->Signal = RSSI;

	/* fill tx count */
	pIbssInfo->TxPacketCnt = pEntry->OneSecTxNoRetryOkCount + 
						pEntry->OneSecTxRetryOkCount + 
						pEntry->OneSecTxFailCount;

	/* fill inactive time */
	pIbssInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */
	pIbssInfo->InactiveTime *= MLME_TASK_EXEC_MULTIPLE;
	pIbssInfo->InactiveTime /= 20;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
{
	HTTRANSMIT_SETTING PhyInfo;
	ULONG DataRate = 0;
	UINT32 RSSI;


	/* fill tx rate */
    if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) ||
	 (pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.MODE <= MODE_OFDM))
	{
		PhyInfo.word = pAd->StaCfg.wdev.HTPhyMode.word;
	}
    else
		PhyInfo.word = pAd->MacTab.Content[BSSID_WCID].HTPhyMode.word;

	RtmpDrvRateGet(pAd, PhyInfo.field.MODE, PhyInfo.field.ShortGI,
				 PhyInfo.field.BW,PhyInfo.field.MCS,
				 newRateGetAntenna(PhyInfo.field.MCS, PhyInfo.field.MODE),&DataRate);
	DataRate /= 500000;
	DataRate /= 2;

	if ((PhyInfo.field.MODE == MODE_HTMIX) ||
		(PhyInfo.field.MODE == MODE_HTGREENFIELD))
	{
		if (PhyInfo.field.BW)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_BW_40;

		if (PhyInfo.field.ShortGI)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_SHORT_GI;

		pIbssInfo->TxRateMCS = PhyInfo.field.MCS;
	}
	else
	{
		pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
		pIbssInfo->TxRateMCS = DataRate*10; /* unit: 100kbps */
	}

	/* fill signal */
	RSSI = RTMPAvgRssi(pAd, &pAd->StaCfg.RssiSample);
	pIbssInfo->Signal = RSSI;
}
#endif /* CONFIG_STA_SUPPORT */

	return TRUE;
}

BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;


	pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;
#ifdef DOT11W_PMF_SUPPORT
	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_AES_CMAC)
	{
		PPMF_CFG pPmfCfg = pPmfCfg = &pAd->StaCfg.PmfCfg;
		hex_dump("PMF IGTK pKeyInfo->KeyBuf=", (UINT8 *)pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
		DBGPRINT(RT_DEBUG_ERROR, ("PMF IGTK pKeyInfo->KeyId=%d\n",pKeyInfo->KeyId));

		//only 4 or 5, no other case!		
		if (pKeyInfo->KeyId == 4 || pKeyInfo->KeyId == 5)
		{
			// no PN is passed, PN is useless in PMF_CalculateBIPMIC()
			NdisZeroMemory(&pPmfCfg->IPN[pKeyInfo->KeyId -4][0], LEN_WPA_TSC);	
			NdisMoveMemory(&pPmfCfg->IGTK[pKeyInfo->KeyId -4][0], pKeyInfo->KeyBuf, pKeyInfo->KeyLen);
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ERROR !! pKeyInfo->KeyId=%d \n",pKeyInfo->KeyId));	

		}
		
	}
	else
#endif /* DOT11W_PMF_SUPPORT */
	if (pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP40 || pKeyInfo->KeyType == RT_CMD_80211_KEY_WEP104)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT_CMD_80211_KEY_WEP\n"));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Set_WPAPSK_Proc ==> %d, %d, %d...\n", pKeyInfo->KeyId, pKeyInfo->KeyType, strlen(pKeyInfo->KeyBuf)));
		
		RT_CMD_STA_IOCTL_SECURITY IoctlSec;
		
		IoctlSec.KeyIdx = pKeyInfo->KeyId;
		IoctlSec.pData = pKeyInfo->KeyBuf;
		IoctlSec.length = pKeyInfo->KeyLen;
		
		/* YF@20120327: Due to WepStatus will be set in the cfg connect function.*/
		if (pAd->StaCfg.wdev.WepStatus == Ndis802_11Encryption2Enabled)
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
		else if (pAd->StaCfg.wdev.WepStatus == Ndis802_11Encryption3Enabled)
			IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;
		IoctlSec.flags = RT_CMD_STA_IOCTL_SECURITY_ENABLED;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
		if (pKeyInfo->bPairwise == FALSE )
#else
		if (pKeyInfo->KeyId > 0)
#endif	
		{
			if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption2Enabled)
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (pAd->StaCfg.GroupCipher == Ndis802_11Encryption3Enabled)
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;
				
			DBGPRINT(RT_DEBUG_TRACE, ("Install GTK: %d\n", IoctlSec.Alg));
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_GROUP_KEY;
		}	
		else
		{
			if (pAd->StaCfg.PairCipher == Ndis802_11Encryption2Enabled)
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_TKIP;
			else if (pAd->StaCfg.PairCipher == Ndis802_11Encryption3Enabled)
				IoctlSec.Alg = RT_CMD_STA_IOCTL_SECURITY_ALG_CCMP;
				
			DBGPRINT(RT_DEBUG_TRACE, ("Install PTK: %d\n", IoctlSec.Alg));
			IoctlSec.ext_flags = RT_CMD_STA_IOCTL_SECURTIY_EXT_SET_TX_KEY;
		}
		
		/*Set_GroupKey_Proc(pAd, &IoctlSec) */
		RTMP_STA_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_STA_SIOCSIWENCODEEXT, 0,
							  &IoctlSec, 0, INT_MAIN);
	}
#endif /* CONFIG_STA_SUPPORT */

	return TRUE;
}

BOOLEAN CFG80211DRV_Connect(
	VOID						*pAdOrg,
	VOID						*pData)
{
#ifdef CONFIG_STA_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;
	UCHAR SSID[NDIS_802_11_LENGTH_SSID + 1]; /* Add One for SSID_Len == 32 */
	UINT32 SSIDLen;
	RT_CMD_STA_IOCTL_SECURITY_ADV IoctlWpa;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_INFRA_ON) && 
            OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211: Connected, disconnect first !\n"));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("CFG80211: No Connection\n"));
	}

	pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;

	/* change to infrastructure mode if we are in ADHOC mode */
	Set_NetworkType_Proc(pAd, "Infra");

	SSIDLen = pConnInfo->SsidLen;
	if (SSIDLen > NDIS_802_11_LENGTH_SSID)
	{
		SSIDLen = NDIS_802_11_LENGTH_SSID;
	}
	
	memset(&SSID, 0, sizeof(SSID));
	memcpy(SSID, pConnInfo->pSsid, SSIDLen);

	if (pConnInfo->bWpsConnection) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPS Connection onGoing.....\n"));
		/* YF@20120327: Trigger Driver to Enable WPS function. */	
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;  /* Set_Wpa_Support(pAd, "3") */
		Set_AuthMode_Proc(pAd, "OPEN");
		Set_EncrypType_Proc(pAd, "NONE");
		Set_SSID_Proc(pAd, (PSTRING)SSID);
		
		return TRUE;
	}
	else
	{
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE; /* Set_Wpa_Support(pAd, "1")*/
	}	
	
	/* set authentication mode */
	if (pConnInfo->WpaVer == 2)
	{
		if (pConnInfo->FlgIs8021x == TRUE) {
			DBGPRINT(RT_DEBUG_TRACE, ("WPA2\n"));
			Set_AuthMode_Proc(pAd, "WPA2");
		}
		else 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("WPA2PSK\n"));
			Set_AuthMode_Proc(pAd, "WPA2PSK");
		}
	}
	else if (pConnInfo->WpaVer == 1)
	{
		if (pConnInfo->FlgIs8021x == TRUE) {
			DBGPRINT(RT_DEBUG_TRACE, ("WPA\n"));
			Set_AuthMode_Proc(pAd, "WPA");
		}
		else 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("WPAPSK\n"));
			Set_AuthMode_Proc(pAd, "WPAPSK");
		}
	}
	else if (pConnInfo->AuthType == Ndis802_11AuthModeAutoSwitch)
		Set_AuthMode_Proc(pAd, "WEPAUTO");
    else if (pConnInfo->AuthType == Ndis802_11AuthModeShared)
		Set_AuthMode_Proc(pAd, "SHARED");
	else
		Set_AuthMode_Proc(pAd, "OPEN");

	CFG80211DBG(RT_DEBUG_TRACE,
				("80211> AuthMode = %d\n", pAd->StaCfg.wdev.AuthMode));

	/* set encryption mode */
	if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AES\n"));
		Set_EncrypType_Proc(pAd, "AES");
	}
	else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("TKIP\n"));
		Set_EncrypType_Proc(pAd, "TKIP");
	}
	else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_WEP)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WEP\n"));
		Set_EncrypType_Proc(pAd, "WEP");
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("NONE\n"));
		Set_EncrypType_Proc(pAd, "NONE");		
	}
	
	/* Groupwise Key Information Setting */
	IoctlWpa.flags = RT_CMD_STA_IOCTL_WPA_GROUP;    
	if (pConnInfo->GroupwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("GTK AES\n"));
		IoctlWpa.value = RT_CMD_STA_IOCTL_WPA_GROUP_CCMP;
		RtmpIoctl_rt_ioctl_siwauth(pAd, &IoctlWpa, 0);
	}
	else if (pConnInfo->GroupwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("GTK TKIP\n"));
		IoctlWpa.value = RT_CMD_STA_IOCTL_WPA_GROUP_TKIP;
		RtmpIoctl_rt_ioctl_siwauth(pAd, &IoctlWpa, 0);
	} 

	CFG80211DBG(RT_DEBUG_TRACE,
				("80211> EncrypType = %d\n", pAd->StaCfg.wdev.WepStatus));

	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Key = %s\n", pConnInfo->pKey));

	/* set channel: STATION will auto-scan */

	/* set WEP key */
	if (pConnInfo->pKey &&
		((pConnInfo->GroupwiseEncrypType | pConnInfo->PairwiseEncrypType) &
												RT_CMD_80211_CONN_ENCRYPT_WEP))
	{
		UCHAR KeyBuf[50];

		/* reset AuthMode and EncrypType */
		Set_EncrypType_Proc(pAd, "WEP");

		/* reset key */
#ifdef RT_CFG80211_DEBUG
		hex_dump("KeyBuf=", (UINT8 *)pConnInfo->pKey, pConnInfo->KeyLen);
#endif /* RT_CFG80211_DEBUG */

		pAd->StaCfg.wdev.DefaultKeyId = pConnInfo->KeyIdx; /* base 0 */
		if (pConnInfo->KeyLen >= sizeof(KeyBuf))
			return FALSE;

		memcpy(KeyBuf, pConnInfo->pKey, pConnInfo->KeyLen);
		KeyBuf[pConnInfo->KeyLen] = 0x00;

		CFG80211DBG(RT_DEBUG_ERROR,
					("80211> pAd->StaCfg.DefaultKeyId = %d\n",
					pAd->StaCfg.wdev.DefaultKeyId));

		Set_Wep_Key_Proc(pAd, (PSTRING)KeyBuf, (INT)pConnInfo->KeyLen, (INT)pConnInfo->KeyIdx);

	} /* End of if */

	/* TODO: We need to provide a command to set BSSID to associate a AP */
	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;

#ifdef DOT11W_PMF_SUPPORT
	if (pConnInfo->mfp)
	{
			//Set_PMFSHA256_Proc(pAd,"1");
		Set_PMFMFPC_Proc(pAd,"1");
	}
#endif /* DOT11W_PMF_SUPPORT */

	Set_SSID_Proc(pAd, (PSTRING)SSID);
	CFG80211DBG(RT_DEBUG_TRACE, ("80211> Connecting SSID = %s\n", SSID));
#endif /* CONFIG_STA_SUPPORT */

	return TRUE;
}


VOID CFG80211DRV_RegNotify(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *pRegInfo;


	pRegInfo = (CMD_RTPRIV_IOCTL_80211_REG_NOTIFY *)pData;

	/* keep Alpha2 and we can re-call the function when interface is up */
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[0] = pRegInfo->Alpha2[0];
	pAd->cfg80211_ctrl.Cfg80211_Alpha2[1] = pRegInfo->Alpha2[1];

	/* apply the new regulatory rule */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		/* interface is up */
		CFG80211_RegRuleApply(pAd, pRegInfo->pWiphy, (UCHAR *)pRegInfo->Alpha2);
	}
	else
	{
		CFG80211DBG(RT_DEBUG_ERROR, ("crda> interface is down!\n"));
	}
}


VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_SURVEY *pSurveyInfo;


	pSurveyInfo = (CMD_RTPRIV_IOCTL_80211_SURVEY *)pData;

	pSurveyInfo->pCfg80211 = pAd->pCfg80211_CB;

#ifdef AP_QLOAD_SUPPORT
	pSurveyInfo->ChannelTimeBusy = pAd->phy_ctrl.QloadLatestChannelBusyTimePri;
	pSurveyInfo->ChannelTimeExtBusy = pAd->phy_ctrl.QloadLatestChannelBusyTimeSec;
#endif /* AP_QLOAD_SUPPORT */
}


VOID CFG80211_UnRegister(
	IN VOID						*pAdOrg,
	IN VOID						*pNetDev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	/* sanity check */
	if (pAd->pCfg80211_CB == NULL)
		return;


	CFG80211OS_UnRegister(pAd->pCfg80211_CB, pNetDev);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, FALSE);
	unregister_netdevice_notifier(&cfg80211_netdev_notifier);
	
	/* Reset CFG80211 Global Setting Here */
	DBGPRINT(RT_DEBUG_TRACE, ("==========> TYPE Reset CFG80211 Global Setting Here <==========\n"));
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE, 
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;	
	
	pAd->pCfg80211_CB = NULL;
	pAd->CommonCfg.HT_Disable = 0;

    	/* It should be free when ScanEnd, 
      	   But Hit close the device in Scanning */
     	if (pCfg80211_ctrl->pCfg80211ChanList != NULL)
     	{
     		os_free_mem(NULL, pCfg80211_ctrl->pCfg80211ChanList);
        	pCfg80211_ctrl->pCfg80211ChanList = NULL;
     	}

  	pCfg80211_ctrl->Cfg80211ChanListLen = 0;
	pCfg80211_ctrl->Cfg80211CurChanIndex = 0;

	if(pCfg80211_ctrl->pExtraIe)
	{
		os_free_mem(NULL, pCfg80211_ctrl->pExtraIe);
		pCfg80211_ctrl->pExtraIe = NULL;
	}
	pCfg80211_ctrl->ExtraIeLen = 0;

    if(pCfg80211_ctrl->pAssocRspIe)
    {
        os_free_mem(NULL, pCfg80211_ctrl->pAssocRspIe);
        pCfg80211_ctrl->pAssocRspIe = NULL;
    }
    pCfg80211_ctrl->assocRspIeLen = 0;

/*
CFG_TODO
     if (pAd->pTxStatusBuf != NULL)
     {
         os_free_mem(NULL, pAd->pTxStatusBuf);
         pAd->pTxStatusBuf = NULL;
     }
	 pAd->TxStatusBufLen = 0;
*/
#ifdef CONFIG_AP_SUPPORT
    if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL)
    {
        os_free_mem(NULL, pAd->cfg80211_ctrl.beacon_tail_buf);
        pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
    }
	pAd->cfg80211_ctrl.beacon_tail_len = 0;

#endif /* CONFIG_AP_SUPPORT */

}


/*
========================================================================
Routine Description:
	Parse and handle country region in beacon from associated AP.

Arguments:
	pAdCB			- WLAN control block pointer
	pVIE			- Beacon elements
	LenVIE			- Total length of Beacon elements

Return Value:
	NONE

Note:
========================================================================
*/
VOID CFG80211_BeaconCountryRegionParse(
	IN VOID						*pAdCB,
	IN NDIS_802_11_VARIABLE_IEs	*pVIE,
	IN UINT16					LenVIE)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	UCHAR *pElement = (UCHAR *)pVIE;
	UINT32 LenEmt;


	while(LenVIE > 0)
	{
		pVIE = (NDIS_802_11_VARIABLE_IEs *)pElement;

		if (pVIE->ElementID == IE_COUNTRY)
		{
			/* send command to do regulation hint only when associated */
			 RT_CFG80211_CRDA_REG_HINT11D(pAd, pVIE->data, pVIE->Length);
			//RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT_11D,
			//					pVIE->data, pVIE->Length);
			break;
		}

		LenEmt = pVIE->Length + 2;

		if (LenVIE <= LenEmt)
			break; /* length is not enough */

		pElement += LenEmt;
		LenVIE -= LenEmt;
	}
} /* End of CFG80211_BeaconCountryRegionParse */

/*
========================================================================
Routine Description:
	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.

Arguments:
	pAdCB			- WLAN control block pointer

Return Value:
	NONE

Note:
	CFG80211_SupBandInit() is called in xx_probe().
========================================================================
*/
#ifdef CONFIG_STA_SUPPORT
VOID CFG80211_LostApInform(
    IN VOID 					*pAdCB)
{

	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
	
	
	pAd->StaCfg.bAutoReconnect = FALSE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0))
	DBGPRINT(RT_DEBUG_TRACE, ("80211> CFG80211_LostApInform \n"));
	cfg80211_disconnected(pAd->net_dev, 0, NULL, 0, GFP_KERNEL);
#else
	DBGPRINT(RT_DEBUG_TRACE, ("80211> CFG80211_LostApInform ==> %d\n", 
					p80211CB->pCfg80211_Wdev->sme_state));
	if (p80211CB->pCfg80211_Wdev->sme_state == CFG80211_SME_CONNECTING)
	{
		   cfg80211_connect_result(pAd->net_dev, NULL, NULL, 0, NULL, 0,
								   WLAN_STATUS_UNSPECIFIED_FAILURE, GFP_KERNEL);
	}
	else if (p80211CB->pCfg80211_Wdev->sme_state == CFG80211_SME_CONNECTED)
	{
		   cfg80211_disconnected(pAd->net_dev, 0, NULL, 0, GFP_KERNEL);
	}
#endif /* LINUX_VERSION_CODE: 3,11,0 */

}
#endif /*CONFIG_STA_SUPPORT*/



/*
========================================================================
Routine Description:
	Hint to the wireless core a regulatory domain from driver.

Arguments:
	pAd				- WLAN control block pointer
	pCountryIe		- pointer to the country IE
	CountryIeLen	- length of the country IE

Return Value:
	NONE

Note:
	Must call the function in kernel thread.
========================================================================
*/
VOID CFG80211_RegHint(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	CFG80211OS_RegHint(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
========================================================================
Routine Description:
	Hint to the wireless core a regulatory domain from country element.

Arguments:
	pAdCB			- WLAN control block pointer
	pCountryIe		- pointer to the country IE
	CountryIeLen	- length of the country IE

Return Value:
	NONE

Note:
	Must call the function in kernel thread.
========================================================================
*/
VOID CFG80211_RegHint11D(
	IN VOID						*pAdCB,
	IN UCHAR					*pCountryIe,
	IN ULONG					CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	CFG80211OS_RegHint11D(CFG80211CB, pCountryIe, CountryIeLen);
}


/*
========================================================================
Routine Description:
	Apply new regulatory rule.

Arguments:
	pAdCB			- WLAN control block pointer
	pWiphy			- Wireless hardware description
	pAlpha2			- Regulation domain (2B)

Return Value:
	NONE

Note:
	Can only be called when interface is up.

	For general mac80211 device, it will be set to new power by Ops->config()
	In rt2x00/, the settings is done in rt2x00lib_config().
========================================================================
*/
VOID CFG80211_RegRuleApply(
	IN VOID						*pAdCB,
	IN VOID						*pWiphy,
	IN UCHAR					*pAlpha2)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	VOID *pBand24G, *pBand5G;
	UINT32 IdBand, IdChan, IdPwr;
	UINT32 ChanNum, ChanId, Power, RecId, DfsType;
	BOOLEAN FlgIsRadar;
	ULONG IrqFlags;
#ifdef DFS_SUPPORT	
	RADAR_DETECT_STRUCT	*pRadarDetect;
#endif /* DFS_SUPPORT */


	CFG80211DBG(RT_DEBUG_TRACE, ("crda> CFG80211_RegRuleApply ==>\n"));

	/* init */
	pBand24G = NULL;
	pBand5G = NULL;

	if (pAd == NULL)
		return;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	/* zero first */
	NdisZeroMemory(pAd->ChannelList,
					MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	/* 2.4GHZ & 5GHz */
	RecId = 0;
#ifdef DFS_SUPPORT	
	pRadarDetect = &pAd->CommonCfg.RadarDetect;
#endif /* DFS_SUPPORT */

	/* find the DfsType */
	DfsType = CE;

	pBand24G = NULL;
	pBand5G = NULL;

	if (CFG80211OS_BandInfoGet(CFG80211CB, pWiphy, &pBand24G, &pBand5G) == FALSE)
		return;

#ifdef EXT_BUILD_CHANNEL_LIST
	if ((pAlpha2[0] != '0') && (pAlpha2[1] != '0'))
	{
		UINT32 IdReg;

		if (pBand5G != NULL)
		{
			for(IdReg=0; ; IdReg++)
			{
				if (ChRegion[IdReg].CountReg[0] == 0x00)
					break;
	
				if ((pAlpha2[0] == ChRegion[IdReg].CountReg[0]) &&
					(pAlpha2[1] == ChRegion[IdReg].CountReg[1]))
				{
					if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
						DfsType = pAd->CommonCfg.DfsType;
					else
						DfsType = ChRegion[IdReg].DfsType;
	
					CFG80211DBG(RT_DEBUG_TRACE,
								("crda> find region %c%c, DFS Type %d\n",
								pAlpha2[0], pAlpha2[1], DfsType));
					break;
				}
			}
		}
	}
#endif /* EXT_BUILD_CHANNEL_LIST */

	for(IdBand=0; IdBand<2; IdBand++)
	{
		if (((IdBand == 0) && (pBand24G == NULL)) ||
			((IdBand == 1) && (pBand5G == NULL)))
		{
			continue;
		}

		if (IdBand == 0)
		{
			CFG80211DBG(RT_DEBUG_TRACE, ("crda> reset chan/power for 2.4GHz\n"));
		}
		else
		{
			CFG80211DBG(RT_DEBUG_TRACE, ("crda> reset chan/power for 5GHz\n"));
		}

		ChanNum = CFG80211OS_ChanNumGet(CFG80211CB, pWiphy, IdBand);

		for(IdChan=0; IdChan<ChanNum; IdChan++)
		{
			if (CFG80211OS_ChanInfoGet(CFG80211CB, pWiphy, IdBand, IdChan,
									&ChanId, &Power, &FlgIsRadar) == FALSE)
			{
				/* the channel is not allowed in the regulatory domain */
				/* get next channel information */
				continue;
			}

			if (!WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
			{
				/* 5G-only mode */
				if (ChanId <= CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}

			if (!WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
			{
				/* 2.4G-only mode */
				if (ChanId > CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}

			for(IdPwr=0; IdPwr<MAX_NUM_OF_CHANNELS; IdPwr++)
			{
				if (ChanId == pAd->TxPower[IdPwr].Channel)
				{
					/* init the channel info. */
					NdisMoveMemory(&pAd->ChannelList[RecId],
									&pAd->TxPower[IdPwr],
									sizeof(CHANNEL_TX_POWER));

					/* keep channel number */
					pAd->ChannelList[RecId].Channel = ChanId;

					/* keep maximum tranmission power */
					pAd->ChannelList[RecId].MaxTxPwr = Power;

					/* keep DFS flag */
					if (FlgIsRadar == TRUE)
						pAd->ChannelList[RecId].DfsReq = TRUE;
					else
						pAd->ChannelList[RecId].DfsReq = FALSE;

					/* keep DFS type */
					pAd->ChannelList[RecId].RegulatoryDomain = DfsType;

					/* re-set DFS info. */
					pAd->CommonCfg.RDDurRegion = DfsType;

					CFG80211DBG(RT_DEBUG_TRACE,
								("Chan %03d:\tpower %d dBm, "
								"DFS %d, DFS Type %d\n",
								ChanId, Power,
								((FlgIsRadar == TRUE)?1:0),
								DfsType));

					/* change to record next channel info. */
					RecId ++;
					break;
				}
			}
		}
	}

	pAd->ChannelListNum = RecId;
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	CFG80211DBG(RT_DEBUG_TRACE, ("crda> Number of channels = %d\n", RecId));
} /* End of CFG80211_RegRuleApply */

/*
========================================================================
Routine Description:
	Inform CFG80211 about association status.

Arguments:
	pAdCB			- WLAN control block pointer
	pBSSID			- the BSSID of the AP
	pReqIe			- the element list in the association request frame
	ReqIeLen		- the request element length
	pRspIe			- the element list in the association response frame
	RspIeLen		- the response element length
	FlgIsSuccess	- 1: success; otherwise: fail

Return Value:
	None

Note:
========================================================================
*/
VOID CFG80211_ConnectResultInform(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID,
	IN UCHAR					*pReqIe,
	IN UINT32					ReqIeLen,
	IN UCHAR					*pRspIe,
	IN UINT32					RspIeLen,
	IN UCHAR					FlgIsSuccess)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;


	CFG80211DBG(RT_DEBUG_TRACE, ("80211> CFG80211_ConnectResultInform ==>\n"));

	CFG80211OS_ConnectResultInform(CFG80211CB,
								pBSSID,
								pReqIe,
								ReqIeLen,
								pRspIe,
								RspIeLen,
								FlgIsSuccess);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
} /* End of CFG80211_ConnectResultInform */




/*
========================================================================
Routine Description:
	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.

Arguments:
	pAdCB			- WLAN control block pointer

Return Value:
	TRUE			- re-init successfully
	FALSE			- re-init fail

Note:
	CFG80211_SupBandInit() is called in xx_probe().
	But we do not have complete chip information in xx_probe() so we
	need to re-init bands in xx_open().
========================================================================
*/
BOOLEAN CFG80211_SupBandReInit(
	IN VOID						*pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_BAND BandInfo;


	CFG80211DBG(RT_DEBUG_TRACE, ("80211> re-init bands...\n"));

	/* re-init bands */
	NdisZeroMemory(&BandInfo, sizeof(BandInfo));
	CFG80211_BANDINFO_FILL(pAd, &BandInfo);

	return CFG80211OS_SupBandReInit(CFG80211CB, &BandInfo);
} /* End of CFG80211_SupBandReInit */

#ifdef CONFIG_STA_SUPPORT
//CMD_RTPRIV_IOCTL_80211_KEY_DEFAULT_SET:
INT CFG80211_setStaDefaultKey(
	IN VOID                     *pAdCB,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	DBGPRINT(RT_DEBUG_TRACE, ("Set Sta Default Key: %d\n", Data));
    pAd->StaCfg.wdev.DefaultKeyId = Data; /* base 0 */
	return 0;	
}

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setStaMgmtDefaultKey(
	IN VOID                     *pAdCB,
	IN UINT 					Data
)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;

	DBGPRINT(RT_DEBUG_TRACE, ("Set Sta MgmtDefault Key: %d\n", Data));
    	//pAd->StaCfg.wdev.MgmtDefaultKeyId = Data; /* base 0 */
	return 0;	
}
#endif /* DOT11W_PMF_SUPPORT */

#endif /*CONFIG_STA_SUPPORT*/
INT CFG80211_reSetToDefault(
	IN VOID                                         *pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	DBGPRINT(RT_DEBUG_TRACE, (" %s\n", __FUNCTION__));
#ifdef CONFIG_STA_SUPPORT
	/* Driver Internal Parm */
	pAd->StaCfg.bAutoConnectByBssid = FALSE;
#endif /*CONFIG_STA_SUPPORT*/		
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;

	pCfg80211_ctrl->Cfg80211RocTimerInit = FALSE;
	pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	pCfg80211_ctrl->FlgCfg80211Scanning = FALSE;

	return TRUE;
}

/*
initList(&pAd->Cfg80211VifDevSet.vifDevList);
initList(&pAd->cfg80211_ctrl.cfg80211TxPacketList);
*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
BOOLEAN CFG80211_checkScanResInKernelCache(
    IN VOID                                         *pAdCB,
    IN UCHAR                                        *pBSSID,
	IN UCHAR					*pSsid,
	IN INT       					ssidLen)
{
        PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
        CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
        struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
        struct cfg80211_bss *bss;
	
	bss = cfg80211_get_bss(pWiphy, NULL, pBSSID,
                               pSsid, ssidLen,
                               WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
	if (bss)
        {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
			cfg80211_put_bss(pWiphy, bss);
#else
                cfg80211_put_bss(bss);
#endif
                return TRUE;
        }

	return FALSE;
}

BOOLEAN CFG80211_checkScanTable(
        IN VOID                                         *pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	ULONG bss_idx = BSS_NOT_FOUND;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	UINT32 CenFreq;
	UINT64 timestamp;
	struct timeval tv;
	UCHAR *ie, ieLen = 0;
	BOOLEAN isOk = FALSE;	
	BSS_ENTRY *pBssEntry;

	USHORT ifIndex = 0;
	PAPCLI_STRUCT pApCliEntry = NULL;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

					
        if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR))
        {
		CFG80211DBG(RT_DEBUG_ERROR, ("pAd->ApCliMlmeAux.Bssid ==> ZERO_MAC_ADDR\n"));
		//ToDo: pAd->ApCfg.ApCliTab[0].CfgApCliBssid
                return FALSE;
        }

	/* Fake TSF */
	do_gettimeofday(&tv);
	timestamp = ((UINT64)tv.tv_sec * 1000000) + tv.tv_usec;

	bss = cfg80211_get_bss(pWiphy, NULL, pApCliEntry->MlmeAux.Bssid,
			       pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen, 
			       WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
	if (bss)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Found %s in Kernel_ScanTable with CH[%d]\n", pApCliEntry->MlmeAux.Ssid, bss->channel->center_freq));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
		cfg80211_put_bss(pWiphy, bss);
#else
		bss->tsf = timestamp;
		cfg80211_put_bss(bss);
#endif
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can't Found %s in Kernel_ScanTable & Try Fake it\n", pApCliEntry->MlmeAux.Ssid));
	}

	bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);

	if (bss_idx != BSS_NOT_FOUND)
	{
		/* Since the cfg80211 kernel scanTable not exist this Entry,
		 * Build an Entry for this connect inform event.  
         	 */

		pBssEntry = &pAd->ScanTab.BssEntry[bss_idx];

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39))
		if (pAd->ScanTab.BssEntry[bss_idx].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel , IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel , IEEE80211_BAND_2GHZ);
#else
            CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel);
#endif
        	chan = ieee80211_get_channel(pWiphy, CenFreq);		

		ieLen = 2 + pApCliEntry->MlmeAux.SsidLen + pBssEntry->VarIeFromProbeRspLen;

		os_alloc_mem(NULL, (UCHAR **)&ie, ieLen);
		if (!ie)
		{
			CFG80211DBG(RT_DEBUG_ERROR, ("Memory Allocate Fail in CFG80211_checkScanTable\n"));
			return FALSE;
		}

		ie[0] = WLAN_EID_SSID;
		ie[1] = pApCliEntry->MlmeAux.SsidLen;
		NdisCopyMemory(ie + 2, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);
		NdisCopyMemory(ie + 2 + pApCliEntry->MlmeAux.SsidLen, pBssEntry->pVarIeFromProbRsp, 
				pBssEntry->VarIeFromProbeRspLen);
		
		bss = cfg80211_inform_bss(pWiphy, chan,
					  pApCliEntry->MlmeAux.Bssid, timestamp, WLAN_CAPABILITY_ESS, pApCliEntry->MlmeAux.BeaconPeriod,
					  ie, ieLen,
#ifdef CFG80211_SCAN_SIGNAL_AVG
					  (pBssEntry->AvgRssi * 100), 
#else
					  (pBssEntry->Rssi * 100), 
#endif
					  GFP_KERNEL);
		if (bss)
		{
			printk("Fake New %s(%02x:%02x:%02x:%02x:%02x:%02x) in Kernel_ScanTable with CH[%d][%d] BI:%d len:%d\n", 
					pApCliEntry->MlmeAux.Ssid, 
					PRINT_MAC(pApCliEntry->MlmeAux.Bssid),bss->channel->center_freq, pBssEntry->Channel,
					pApCliEntry->MlmeAux.BeaconPeriod, pBssEntry->VarIeFromProbeRspLen);	
			
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
			cfg80211_put_bss(pWiphy, bss);
#else
			cfg80211_put_bss(bss);
#endif
			isOk = TRUE;
		}
		
		if (ie != NULL)
			os_free_mem(NULL, ie);

		if (isOk)
			return TRUE;
	}
	else
		printk("%s Not In Driver Scan Table\n", pApCliEntry->MlmeAux.Ssid);

	return FALSE;
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

//CFG_TODO
UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch)
{
	UCHAR ret_channel;
	
	if (pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40)
	{
		if (pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			ret_channel = prim_ch + 2;
		else
		{
			if (prim_ch == 14)
				ret_channel = prim_ch - 1;
			else
				ret_channel = prim_ch - 2;
		}
	}
	else
		ret_channel = prim_ch;

	return ret_channel;
}

#endif /* RT_CFG80211_SUPPORT */

