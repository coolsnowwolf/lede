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
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/
#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"
#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

extern struct notifier_block cfg80211_netdev_notifier;

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);

extern INT RtmpIoctl_rt_ioctl_siwauth(
	IN      RTMP_ADAPTER * pAd,
	IN      VOID * pData,
	IN      ULONG                            Data);


INT CFG80211DRV_IoctlHandle(
	IN	VOID * pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT * wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID * pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = get_wdev_by_ioctl_idx_and_iftype(pAd, pObj->ioctl_if, pObj->ioctl_if_type);
#ifdef CONFIG_MULTI_CHANNEL
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif /* CONFIG_MULTI_CHANNEL */

	switch (cmd) {
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
		CFG80211DRV_OpsLeave(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_GET:
		if (CFG80211DRV_StaGet(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;
#ifdef CFG_TDLS_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_STA_TDLS_INSERT_PENTRY:
		CFG80211DRV_StaTdlsInsertDeletepEntry(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_STA_TDLS_SET_KEY_COPY_FLAG:
		CFG80211DRV_StaTdlsSetKeyCopyFlag(pAd);
		break;
#endif /* CFG_TDLS_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_STA_KEY_ADD: {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CMD_RTPRIV_IOCTL_80211_KEY *pKeyInfo;

		pKeyInfo = (CMD_RTPRIV_IOCTL_80211_KEY *)pData;

		if (
#ifdef CFG80211_MULTI_STA
			RTMP_CFG80211_MULTI_STA_ON(pAd, pKeyInfo->pNetDev) ||
#endif /* CFG80211_MULTI_STA */
			(pKeyInfo->pNetDev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		)
			CFG80211DRV_P2pClientKeyAdd(pAd, pData);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA*/
#ifdef APCLI_CFG80211_SUPPORT
			CFG80211DRV_ApClientKeyAdd(pAd, pData);
#else
			CFG80211DRV_StaKeyAdd(pAd, pData);
#endif /* APCLI_CFG80211_SUPPORT */
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_CONNECT_TO: {
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
		CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;

		pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;

		if (
#ifdef CFG80211_MULTI_STA
			(RTMP_CFG80211_MULTI_STA_ON(pAd, pConnInfo->pNetDev)) ||
#endif /* CFG80211_MULTI_STA */
			(Data == RT_CMD_80211_IFTYPE_P2P_CLIENT))
			CFG80211DRV_P2pClientConnect(pAd, pData);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
			CFG80211DRV_Connect(pAd, pData);
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_REG_NOTIFY_TO:
		CFG80211DRV_RegNotify(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_UNREGISTER:

		/* Only main net_dev needs to do CFG80211_UnRegister. */
		if (pAd->net_dev == pData)
			CFG80211_UnRegister(pAd, pData);

		break;

	case CMD_RTPRIV_IOCTL_80211_BANDINFO_GET: {
		CFG80211_BAND *pBandInfo = (CFG80211_BAND *)pData;

		CFG80211_BANDINFO_FILL(pAd, wdev, pBandInfo);
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode)
				pBandInfo->RFICType = RFIC_DUAL_BAND;
#endif
	}
	    break;

	case CMD_RTPRIV_IOCTL_80211_SURVEY_GET:
		CFG80211DRV_SurveyGet(pAd, pData);
		break;

#ifdef APCLI_CFG80211_SUPPORT
	case CMD_RTPRIV_IOCTL_APCLI_SITE_SURVEY:
		CFG80211DRV_ApcliSiteSurvey(pAd, pData);
		break;
#endif /* APCLI_CFG80211_SUPPORT */
	case CMD_RTPRIV_IOCTL_80211_EXTRA_IES_SET:
		CFG80211DRV_OpsScanExtraIesSet(pAd);
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
		return CFG80211DRV_OpsScanSetSpecifyChannel(pAd, pData, Data);
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT

	case CMD_RTPRIV_IOCTL_MCC_DHCP_PROTECT_STATUS:
		pApCliEntry = &pAd->ApCfg.ApCliTab[0];
		*(UCHAR *)pData = pApCliEntry->Valid;
		break;

	case CMD_RTPRIV_IOCTL_80211_SET_NOA:
		CFG80211DRV_Set_NOA(pAd, Data);
		break;
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_BEACON_SET:
		CFG80211DRV_OpsBeaconSet(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_BEACON_ADD:
		CFG80211DRV_OpsBeaconAdd(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_BEACON_DEL: {

		INT i, apidx = Data;

			for (i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
				pAd->ApCfg.MBSSID[apidx].wdev.bcn_buf.TimBitmaps[i] = 0;
			if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
				os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
				pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
			}
			pAd->cfg80211_ctrl.beacon_tail_len = 0;
	}
	break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_ADD:
		CFG80211DRV_ApKeyAdd(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_RTS_THRESHOLD_ADD:
		CFG80211DRV_RtsThresholdAdd(pAd, wdev, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_FRAG_THRESHOLD_ADD:
		CFG80211DRV_FragThresholdAdd(pAd, wdev, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEL:
		CFG80211DRV_ApKeyDel(pAd, pData);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_SET:
		CFG80211_setApDefaultKey(pAd, pData, Data);
		break;

#ifdef DOT11W_PMF_SUPPORT
	case CMD_RTPRIV_IOCTL_80211_AP_KEY_DEFAULT_MGMT_SET:
		CFG80211_setApDefaultMgmtKey(pAd, pData, Data);
		break;
#endif /*DOT11W_PMF_SUPPORT*/


	case CMD_RTPRIV_IOCTL_80211_PORT_SECURED:
		CFG80211_StaPortSecured(pAd, pData, Data);
		break;

	case CMD_RTPRIV_IOCTL_80211_AP_STA_DEL:
		CFG80211_ApStaDel(pAd, pData, Data);
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

	case CMD_RTPRIV_IOCTL_80211_NETDEV_EVENT: {
		/*
		 * CFG_TODO: For Scan_req per netdevice
		 * PNET_DEV pNetDev = (PNET_DEV) pData;
		 * struct wireless_dev *pWdev = pAd->pCfg80211_CB->pCfg80211_Wdev;
		 * if (RTMPEqualMemory(pNetDev->dev_addr, pNewNetDev->dev_addr, MAC_ADDR_LEN))
		 */
		if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG_SCAN: close the scan cmd in device close phase\n"));
			CFG80211OS_ScanEnd(pAd->pCfg80211_CB, TRUE);
			pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		}
	}
	break;
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)

	case CMD_RTPRIV_IOCTL_80211_P2PCLI_ASSSOC_IE_SET: {
		CMD_RTPRIV_IOCTL_80211_ASSOC_IE *pAssocIe;

		pAssocIe = (CMD_RTPRIV_IOCTL_80211_ASSOC_IE *)pData;
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

		if (
#ifdef CFG80211_MULTI_STA
			RTMP_CFG80211_MULTI_STA_ON(pAd, pAssocIe->pNetDev) ||
#endif /* CFG80211_MULTI_STA */
			(Data == RT_CMD_80211_IFTYPE_P2P_CLIENT)
		)
			CFG80211DRV_SetP2pCliAssocIe(pAd, pAssocIe->ie, pAssocIe->ie_len);
		else
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef APCLI_CFG80211_SUPPORT
			CFG80211DRV_SetApCliAssocIe(pAd, pAssocIe->ie, pAssocIe->ie_len);
#endif
	}
	break;
#endif /*CONFIG_STA_SUPPORT*/
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)

	case CMD_RTPRIV_IOCTL_80211_VIF_ADD:
		if (CFG80211DRV_OpsVifAdd(pAd, pData) != TRUE)
			return NDIS_STATUS_FAILURE;

		break;

	case CMD_RTPRIV_IOCTL_80211_VIF_DEL:
		RTMP_CFG80211_VirtualIF_Remove(pAd, pData, Data);
		break;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */
#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_ANDROID_PRIV_CMD:
		/* rt_android_private_command_entry(pAd, ); */
		break;
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */
#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT

	case CMD_RTPRIV_IOCTL_80211_SEND_WIRELESS_EVENT:
		CFG80211_SendWirelessEvent(pAd, pData);
		break;
#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
#ifdef RFKILL_HW_SUPPORT

	case CMD_RTPRIV_IOCTL_80211_RFKILL: {
		UINT32 data = 0;
		BOOLEAN active;
		/* Read GPIO pin2 as Hardware controlled radio state */
		RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
		active = !!(data & 0x04);

		if (!active) {
			RTMPSetLED(pAd, LED_RADIO_OFF);
			*(UINT8 *)pData = 0;
		} else
			*(UINT8 *)pData = 1;
	}
	break;
#endif /* RFKILL_HW_SUPPORT */

	case CMD_RTPRIV_IOCTL_80211_REGISTER:

		/* Only main net_dev needs to do CFG80211_Register. */
		if (pAd->net_dev == pData)
			CFG80211_Register(pAd, pObj->pDev, pAd->net_dev);

		break;

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

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d] pAd->Cfg80211RegisterProbeReqFrame=%d[%d]\n",
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

	/* IF Not Exist on VIF List, the device must be MAIN_DEV */
	if (isReg) {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount < 255)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount++;
	} else {
		if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
			pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount--;
	}

	if (pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount > 0)
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = TRUE;
	else {
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
		pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%d] TYPE pAd->Cfg80211RegisterActionFrame=%d[%d]\n",
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

	CFG80211DBG(DBG_LVL_TRACE, ("%s\n", __func__));
	pBssInfo = (CMD_RTPRIV_IOCTL_80211_BSS_PARM *)pData;

	/* Short Preamble */
	if (pBssInfo->use_short_preamble != -1) {
		CFG80211DBG(DBG_LVL_TRACE, ("%s: ShortPreamble %d\n", __func__, pBssInfo->use_short_preamble));
		pAd->CommonCfg.TxPreamble = (pBssInfo->use_short_preamble == 0 ? Rt802_11PreambleLong : Rt802_11PreambleShort);
		TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);
		MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);
	}

	/* CTS Protection */
	if (pBssInfo->use_cts_prot != -1)
		CFG80211DBG(DBG_LVL_TRACE, ("%s: CTS Protection %d\n", __func__, pBssInfo->use_cts_prot));

	/* Short Slot */
	if (pBssInfo->use_short_slot_time != -1)
		CFG80211DBG(DBG_LVL_TRACE, ("%s: Short Slot %d\n", __func__, pBssInfo->use_short_slot_time));
}

BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData)
{
	CMD_RTPRIV_IOCTL_80211_CHAN *pChan;
	UINT8 ChanId, IfType, ChannelType;
#ifdef DOT11_N_SUPPORT
	BOOLEAN FlgIsChanged;
#endif /* DOT11_N_SUPPORT */
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
	struct wifi_dev *wdev = &pMbss->wdev;
	UCHAR RfIC = 0;
	UCHAR newBW = BW_20;
	UCHAR ext_cha;
	CHANNEL_CTRL *pChCtrl;
	UCHAR BandIdx;
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

#ifdef HOSTAPD_AUTO_CH_SUPPORT
	printk("HOSTAPD AUTO_CH_SUPPORT Ignore Channel %d from HostAPD \n", pChan->ChanId);
	return TRUE;
#endif

	/* set phymode by channel number */
	if (ChanId > 14) {
		wdev->PhyMode = (WMODE_A | WMODE_AN | WMODE_AC); /*5G phymode*/
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
		RfIC = RFIC_5GHZ;
	} else {
		wdev->PhyMode = (WMODE_B | WMODE_G | WMODE_GN);  /*2G phymode*/
		/* Change channel state to NONE */
		BandIdx = HcGetBandByWdev(wdev);
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		hc_set_ChCtrlChListStat(pChCtrl, CH_LIST_STATE_NONE);
		BuildChannelList(pAd, wdev);
		RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
		RfIC = RFIC_24GHZ;
	}

	if (IfType != RT_CMD_80211_IFTYPE_MONITOR) {
		/* get channel BW */
		FlgIsChanged = TRUE;

		/* set to new channel BW */
		if (ChannelType == RT_CMD_80211_CHANTYPE_HT20) {
			newBW = BW_20;
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;
				wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if (ChannelType == RT_CMD_80211_CHANTYPE_HT40MINUS) {
			newBW = BW_40;
			wlan_operate_set_ht_bw(wdev, HT_BW_40, EXTCHA_BELOW);
			ext_cha = wlan_operate_get_ext_cha(wdev);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

				wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if	(ChannelType == RT_CMD_80211_CHANTYPE_HT40PLUS) {
			/* not support NL80211_CHAN_HT40MINUS or NL80211_CHAN_HT40PLUS */
			/* i.e. primary channel = 36, secondary channel must be 40 */
			newBW = BW_40;
			wlan_operate_set_ht_bw(wdev, HT_BW_40, EXTCHA_ABOVE);
			pAd->CommonCfg.HT_Disable = 0;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if  (ChannelType == RT_CMD_80211_CHANTYPE_NOHT) {
			newBW = BW_20;
			wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
			ext_cha = wlan_operate_get_ext_cha(wdev);
			pAd->CommonCfg.HT_Disable = 1;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_prim_ch(wdev, wdev->channel);
		} else if  (ChannelType == RT_CMD_80211_CHANTYPE_VHT80) {
			newBW = BW_80;

			if (pChan->CenterChanId > pChan->ChanId)
				ext_cha = EXTCHA_ABOVE;
			else
				ext_cha = EXTCHA_BELOW;

			if (IfType == RT_CMD_80211_IFTYPE_AP ||
				IfType == RT_CMD_80211_IFTYPE_P2P_GO)
				wdev->channel = ChanId;

			wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
			wlan_operate_set_vht_bw(wdev, VHT_BW_80);
			wlan_operate_set_prim_ch(wdev, wdev->channel);
		}

		CFG80211DBG(DBG_LVL_TRACE, ("80211> HT Disable = %d\n", pAd->CommonCfg.HT_Disable));
	} else {
		/* for monitor mode */
		FlgIsChanged = TRUE;
		pAd->CommonCfg.HT_Disable = 0;
		wlan_operate_set_ht_bw(wdev, HT_BW_40, wlan_operate_get_ext_cha(wdev));
	}

	ext_cha = wlan_operate_get_ext_cha(wdev);
	/* switch to the channel with Common Channel */
	wdev->channel = ChanId;
	CFG80211DBG(DBG_LVL_ERROR, ("80211> CentralChannel = %d, New BW = %d with Ext[%d]\n",
								wlan_operate_get_cen_ch_1(wdev), newBW, ext_cha));
#ifdef CONFIG_AP_SUPPORT
	os_msec_delay(1000);
#ifndef RT_CFG80211_SUPPORT
	APStopByRf(pAd, RfIC);
#else
    APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
#endif
	os_msec_delay(1000);
#ifndef RT_CFG80211_SUPPORT
	APStartUpByRf(pAd, RfIC);
#else
    APStartUp(pAd, pMbss, AP_BSS_OPER_BY_RF);
#endif
#endif /* CONFIG_AP_SUPPORT */

	if (IfType == RT_CMD_80211_IFTYPE_AP ||
		IfType == RT_CMD_80211_IFTYPE_P2P_GO) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> Set the channel in AP Mode\n"));
		return TRUE;
	}

	return TRUE;
}

BOOLEAN CFG80211DRV_OpsJoinIbss(
	VOID						*pAdOrg,
	VOID						*pData)
{
	return TRUE;
}

BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, PNET_DEV pNetDev)
{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	MLME_DEAUTH_REQ_STRUCT   DeAuthReq;
	MLME_QUEUE_ELEM *pMsgElem = NULL;
#ifdef APCLI_CFG80211_SUPPORT
		/*check if net dev corresponding to Apcli entry */
	if (pAd->ApCfg.ApCliTab[0].wdev.if_dev == pNetDev) {
		pAd->cfg80211_ctrl.FlgCfg80211Connecting = FALSE;
		os_alloc_mem(pAd, (UCHAR **)&pMsgElem, sizeof(MLME_QUEUE_ELEM));
		COPY_MAC_ADDR(DeAuthReq.Addr, pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid);
		DeAuthReq.Reason = REASON_DEAUTH_STA_LEAVING;
		pMsgElem->MsgLen = sizeof(MLME_DEAUTH_REQ_STRUCT);
		pMsgElem->Priv = 0;
		os_move_mem(pMsgElem->Msg, &DeAuthReq, sizeof(MLME_DEAUTH_REQ_STRUCT));
		ApCliMlmeDeauthReqAction(pAd, pMsgElem);
		os_free_mem(pMsgElem);
		pMsgElem = NULL;
		ApCliLinkDown(pAd, 0);
	}
#endif /* APCLI_CFG80211_SUPPORT */
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT) */
	return TRUE;
}


BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_STA *pIbssInfo;

    HTTRANSMIT_SETTING *HtPhyMode = NULL;
	struct wifi_dev *wdev = NULL;
	RSSI_SAMPLE *RssiSample = NULL;
	RADIUS_ACCOUNT_ENTRY *pFoundEntry = NULL;

	pIbssInfo = (CMD_RTPRIV_IOCTL_80211_STA *)pData;
#ifdef CONFIG_AP_SUPPORT
	{
		MAC_TABLE_ENTRY *pEntry;
		ULONG DataRate = 0;
		UINT32 RSSI;

		pEntry = MacTableLookup(pAd, pIbssInfo->MAC);

		if (pEntry == NULL) {
		    UCHAR i;
		    BOOLEAN found = FALSE;
		    /*search MAC Address in Radius Table */
		    for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			    RADIUS_ACCOUNT_ENTRY *pRadiusEntry = &pAd->radius_tbl[i];

			    if (MAC_ADDR_EQUAL(pRadiusEntry->Addr, pIbssInfo->MAC)) {
				    /*Found Radius Entry */
				    found = TRUE;
				    pFoundEntry = pRadiusEntry;
				    HtPhyMode = &pRadiusEntry->HTPhyMode;
				    wdev = pRadiusEntry->wdev;
				    RssiSample = &pRadiusEntry->RssiSample;

				    getRate(pRadiusEntry->HTPhyMode, &DataRate);
				    pIbssInfo->rx_bytes = pRadiusEntry->RxBytes;

				    /* fill tx_bytes count */
				    pIbssInfo->tx_bytes = pRadiusEntry->TxBytes;

				    /* fill rx_packets count */
				    pIbssInfo->rx_packets = pRadiusEntry->RxPackets.u.LowPart;

				    /* fill tx_packets count */
				    pIbssInfo->tx_packets = pRadiusEntry->TxPackets.u.LowPart;

				    /* fill inactive time */
				    pIbssInfo->InactiveTime = pRadiusEntry->NoDataIdleCount * 1000; /* unit: ms */
				    break;
			    }
		    }
		    if (!found)
			    return FALSE;
	    } 	else {

		HtPhyMode = &pEntry->HTPhyMode;
		    wdev = pEntry->wdev;
		    RssiSample = &pEntry->RssiSample;
		    getRate(pEntry->HTPhyMode, &DataRate);

		    pIbssInfo->rx_bytes = pEntry->RxBytes;

		    /* fill tx_bytes count */
		    pIbssInfo->tx_bytes = pEntry->TxBytes;

		    /* fill rx_packets count */
		    pIbssInfo->rx_packets = pEntry->RxPackets.u.LowPart;

		    /* fill tx_packets count */
		    pIbssInfo->tx_packets = pEntry->TxPackets.u.LowPart;

		    /* fill inactive time */
		    pIbssInfo->InactiveTime = pEntry->NoDataIdleCount * 1000; /* unit: ms */

	    }

		/* fill tx rate */
	if ((HtPhyMode->field.MODE == MODE_HTMIX) ||
		(HtPhyMode->field.MODE == MODE_HTGREENFIELD)) {
		if (HtPhyMode->field.BW)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_BW_40;

		if (HtPhyMode->field.ShortGI)
			pIbssInfo->TxRateFlags |= RT_CMD_80211_TXRATE_SHORT_GI;

			pIbssInfo->TxRateMCS = HtPhyMode->field.MCS;
		} else {
			pIbssInfo->TxRateFlags = RT_CMD_80211_TXRATE_LEGACY;
			pIbssInfo->TxRateMCS = DataRate * 1000; /* unit: 100kbps */
		}

		/* fill signal */
		RSSI = RTMPAvgRssi(pAd, RssiSample);
		pIbssInfo->Signal = RSSI;

		pIbssInfo->InactiveTime *= MLME_TASK_EXEC_MULTIPLE;
		pIbssInfo->InactiveTime /= 20;

		if (pFoundEntry)
			NdisZeroMemory(pFoundEntry, sizeof(RADIUS_ACCOUNT_ENTRY));
	}
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData)
{
	return TRUE;
}

BOOLEAN CFG80211DRV_Connect(
	VOID						*pAdOrg,
	VOID						*pData)
{
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
		PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
		CMD_RTPRIV_IOCTL_80211_CONNECT *pConnInfo;
		UCHAR SSID[NDIS_802_11_LENGTH_SSID + 1]; /* Add One for SSID_Len == 32 */
		UINT32 SSIDLen;
#ifdef APCLI_CFG80211_SUPPORT
		APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[0];
#endif /* APCLI_CFG80211_SUPPORT */
		pConnInfo = (CMD_RTPRIV_IOCTL_80211_CONNECT *)pData;
		SSIDLen = pConnInfo->SsidLen;
		if (SSIDLen > NDIS_802_11_LENGTH_SSID)
			SSIDLen = NDIS_802_11_LENGTH_SSID;
		memset(&SSID, 0, sizeof(SSID));
		memcpy(SSID, pConnInfo->pSsid, SSIDLen);
#ifdef APCLI_CFG80211_SUPPORT
		apcli_entry->wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
#endif

#ifdef APCLI_CFG80211_SUPPORT
	/* Check the connection is WPS or not */
	if (pConnInfo->bWpsConnection) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AP_CLI WPS Connection onGoing.....\n"));
		apcli_entry->wpa_supplicant_info.WpaSupplicantUP |= WPA_SUPPLICANT_ENABLE_WPS;
	}

	/* Set authentication mode */
	if (pConnInfo->WpaVer == 2) {
		if (!pConnInfo->FlgIs8021x) {
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCLI WPA2PSK\n"));
			Set_ApCli_AuthMode_Proc(pAd, "WPA2PSK");
		}
	} else if (pConnInfo->WpaVer == 1) {
		if (!pConnInfo->FlgIs8021x) {
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("APCLI WPAPSK\n"));
			Set_ApCli_AuthMode_Proc(pAd, "WPAPSK");
		}
	} else if (pConnInfo->AuthType == Ndis802_11AuthModeShared)
		Set_ApCli_AuthMode_Proc(pAd, "SHARED");
	else if (pConnInfo->AuthType == Ndis802_11AuthModeOpen)
		Set_ApCli_AuthMode_Proc(pAd, "OPEN");
	else
		Set_ApCli_AuthMode_Proc(pAd, "WEPAUTO");
	/* Set PTK Encryption Mode */
	if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_CCMP) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AES\n"));
		Set_ApCli_EncrypType_Proc(pAd, "AES");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_TKIP) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TKIP\n"));
		Set_ApCli_EncrypType_Proc(pAd, "TKIP");
	} else if (pConnInfo->PairwiseEncrypType & RT_CMD_80211_CONN_ENCRYPT_WEP) {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WEP\n"));
		Set_ApCli_EncrypType_Proc(pAd, "WEP");
	} else {
		MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NONE\n"));
		Set_ApCli_EncrypType_Proc(pAd, "NONE");
	}
/* #endif */

	if (pConnInfo->pBssid != NULL) {
		os_zero_mem(apcli_entry->CfgApCliBssid, MAC_ADDR_LEN);
		NdisCopyMemory(apcli_entry->CfgApCliBssid, pConnInfo->pBssid, MAC_ADDR_LEN);
	}

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	pAd->cfg80211_ctrl.FlgCfg80211Connecting = TRUE;
	Set_ApCli_Ssid_Proc(pAd, (RTMP_STRING *)SSID);
	Set_ApCli_Enable_Proc(pAd, "1");
	CFG80211DBG(DBG_LVL_OFF, ("80211> APCLI CONNECTING SSID = %s\n", SSID));
#endif /* APCLI_CFG80211_SUPPORT */
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
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
		/* interface is up */
		CFG80211_RegRuleApply(pAd, pRegInfo->pWiphy, (UCHAR *)pRegInfo->Alpha2);
	} else
		CFG80211DBG(DBG_LVL_ERROR, ("crda> interface is down!\n"));
}


VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	CMD_RTPRIV_IOCTL_80211_SURVEY *pSurveyInfo;
#ifdef AP_QLOAD_SUPPORT
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);
#endif
	pSurveyInfo = (CMD_RTPRIV_IOCTL_80211_SURVEY *)pData;
	pSurveyInfo->pCfg80211 = pAd->pCfg80211_CB;
#ifdef AP_QLOAD_SUPPORT
	pSurveyInfo->ChannelTimeBusy = pQloadCtrl->QloadLatestChannelBusyTimePri;
	pSurveyInfo->ChannelTimeExtBusy = pQloadCtrl->QloadLatestChannelBusyTimeSec;
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
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==========> TYPE Reset CFG80211 Global Setting Here <==========\n"));
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE,
									pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pAd->pCfg80211_CB = NULL;
	pAd->CommonCfg.HT_Disable = 0;

	/* It should be free when ScanEnd, */
	/*  But Hit close the device in Scanning */
	if (pCfg80211_ctrl->pCfg80211ChanList != NULL) {
		os_free_mem(pCfg80211_ctrl->pCfg80211ChanList);
		pCfg80211_ctrl->pCfg80211ChanList = NULL;
	}

	pCfg80211_ctrl->Cfg80211ChanListLen = 0;
	pCfg80211_ctrl->Cfg80211CurChanIndex = 0;

	if (pCfg80211_ctrl->pExtraIe) {
		os_free_mem(pCfg80211_ctrl->pExtraIe);
		pCfg80211_ctrl->pExtraIe = NULL;
	}

	pCfg80211_ctrl->ExtraIeLen = 0;
	/*
	 * CFG_TODO
	 *    if (pAd->pTxStatusBuf != NULL)
	 *    {
	 *	 os_free_mem(pAd->pTxStatusBuf);
	 *	 pAd->pTxStatusBuf = NULL;
	 *   }
	 *	 pAd->TxStatusBufLen = 0;
	 */
#ifdef CONFIG_AP_SUPPORT

	if (pAd->cfg80211_ctrl.beacon_tail_buf != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.beacon_tail_buf);
		pAd->cfg80211_ctrl.beacon_tail_buf = NULL;
	}

	pAd->cfg80211_ctrl.beacon_tail_len = 0;
#endif /* CONFIG_AP_SUPPORT */

	if (pAd->cfg80211_ctrl.BeaconExtraIe != NULL) {
		os_free_mem(pAd->cfg80211_ctrl.BeaconExtraIe);
		pAd->cfg80211_ctrl.BeaconExtraIe = NULL;
	}

	pAd->cfg80211_ctrl.BeaconExtraIeLen = 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Parse and handle country region in beacon from associated AP.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pVIE			- Beacon elements
 *	LenVIE			- Total length of Beacon elements
 *
 * Return Value:
 *	NONE
 * ========================================================================
 */
VOID CFG80211_BeaconCountryRegionParse(
	IN VOID						*pAdCB,
	IN NDIS_802_11_VARIABLE_IEs * pVIE,
	IN UINT16					LenVIE)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	UCHAR *pElement = (UCHAR *)pVIE;
	UINT32 LenEmt;

	while (LenVIE > 0) {
		pVIE = (NDIS_802_11_VARIABLE_IEs *)pElement;

		if (pVIE->ElementID == IE_COUNTRY) {
			/* send command to do regulation hint only when associated */
			/* RT_CFG80211_CRDA_REG_HINT11D(pAd, pVIE->data, pVIE->Length); */
			RTEnqueueInternalCmd(pAd, CMDTHREAD_REG_HINT_11D,
								 pVIE->data, pVIE->Length);
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
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 * ========================================================================
 */



/*
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from driver.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
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
 * ========================================================================
 * Routine Description:
 *	Hint to the wireless core a regulatory domain from country element.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pCountryIe		- pointer to the country IE
 *	CountryIeLen	- length of the country IE
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Must call the function in kernel thread.
 * ========================================================================
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
 * ========================================================================
 * Routine Description:
 *	Apply new regulatory rule.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pWiphy			- Wireless hardware description
 *	pAlpha2			- Regulation domain (2B)
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	Can only be called when interface is up.
 *
 *	For general mac80211 device, it will be set to new power by Ops->config()
 *	In rt2x00/, the settings is done in rt2x00lib_config().
 * ========================================================================
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
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	CFG80211DBG(DBG_LVL_TRACE, ("crda> CFG80211_RegRuleApply ==>\n"));
	/* init */
	pBand24G = NULL;
	pBand5G = NULL;

	if (pAd == NULL)
		return;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
	/* zero first */
	/* 2.4GHZ & 5GHz */
	RecId = 0;
	/* find the DfsType */
	DfsType = CE;
	pBand24G = NULL;
	pBand5G = NULL;

	if (CFG80211OS_BandInfoGet(CFG80211CB, pWiphy, &pBand24G, &pBand5G) == FALSE)
		return;

#ifdef AUTO_CH_SELECT_ENHANCE
#ifdef EXT_BUILD_CHANNEL_LIST

	if ((pAlpha2[0] != '0') && (pAlpha2[1] != '0')) {
		UINT32 IdReg;

		if (pBand5G != NULL) {
			for (IdReg = 0;; IdReg++) {
				if (ChRegion[IdReg].CountReg[0] == 0x00)
					break;

				if ((pAlpha2[0] == ChRegion[IdReg].CountReg[0]) &&
					(pAlpha2[1] == ChRegion[IdReg].CountReg[1])) {
					if (pAd->CommonCfg.DfsType != MAX_RD_REGION)
						DfsType = pAd->CommonCfg.DfsType;
					else
						DfsType = ChRegion[IdReg].op_class_region;

					CFG80211DBG(DBG_LVL_TRACE,
								("crda> find region %c%c, DFS Type %d\n",
								 pAlpha2[0], pAlpha2[1], DfsType));
					break;
				}
			}
		}
	}

#endif /* EXT_BUILD_CHANNEL_LIST */
#endif /* AUTO_CH_SELECT_ENHANCE */

	for (IdBand = 0; IdBand < IEEE80211_NUM_BANDS; IdBand++) {
		if (((IdBand == IEEE80211_BAND_2GHZ) && (pBand24G == NULL)) ||
			((IdBand == IEEE80211_BAND_5GHZ) && (pBand5G == NULL)))
			continue;

		if (IdBand == IEEE80211_BAND_2GHZ)
			CFG80211DBG(DBG_LVL_TRACE, ("crda> reset chan/power for 2.4GHz\n"));
		else
			CFG80211DBG(DBG_LVL_TRACE, ("crda> reset chan/power for 5GHz\n"));

		ChanNum = CFG80211OS_ChanNumGet(CFG80211CB, pWiphy, IdBand);

		for (IdChan = 0; IdChan < ChanNum; IdChan++) {
			if (CFG80211OS_ChanInfoGet(CFG80211CB, pWiphy, IdBand, IdChan,
									   &ChanId, &Power, &FlgIsRadar) == FALSE) {
				/* the channel is not allowed in the regulatory domain */
				/* get next channel information */
				continue;
			}

			if (!WMODE_CAP_2G(pAd->CommonCfg.cfg_wmode)) {
				/* 5G-only mode */
				if (ChanId <= CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}

			if (!WMODE_CAP_5G(pAd->CommonCfg.cfg_wmode)) {
				/* 2.4G-only mode */
				if (ChanId > CFG80211_NUM_OF_CHAN_2GHZ)
					continue;
			}
			BandIdx = HcGetBandByChannel(pAd, ChanId);
			pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

			/* zero first */
			os_zero_mem(pChCtrl->ChList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));
			for (IdPwr = 0; IdPwr < MAX_NUM_OF_CHANNELS; IdPwr++) {
				/* sachin - TODO */
				/* if (ChanId == pAd->TxPower[IdPwr].Channel) */
				{
					/* sachin - TODO */
					/* init the channel info. */
					/* os_move_mem(&pAd->ChannelList[RecId],&pAd->TxPower[IdPwr],sizeof(CHANNEL_TX_POWER)); */
					/* keep channel number */
					pChCtrl->ChList[RecId].Channel = ChanId;
					/* keep maximum tranmission power */
					pChCtrl->ChList[RecId].MaxTxPwr = Power;

					/* keep DFS flag */
					if (FlgIsRadar == TRUE)
						pChCtrl->ChList[RecId].DfsReq = TRUE;
					else
						pChCtrl->ChList[RecId].DfsReq = FALSE;

					/* keep DFS type */
					pChCtrl->ChList[RecId].RegulatoryDomain = DfsType;
					/* re-set DFS info. */
					pAd->CommonCfg.RDDurRegion = DfsType;
					CFG80211DBG(DBG_LVL_TRACE,
								("Chan %03d:\tpower %d dBm, DFS %d, DFS Type %d\n",
								 ChanId, Power,
								 ((FlgIsRadar == TRUE) ? 1 : 0),
								 DfsType));
					/* change to record next channel info. */
					RecId++;
					break;
				}
			}
		}
	}

	pChCtrl->ChListNum = RecId;
	CFG80211DBG(DBG_LVL_TRACE, ("[CFG80211_RegRuleApply] - pChCtrl->ChListNum = %d\n", pChCtrl->ChListNum));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
	CFG80211DBG(DBG_LVL_TRACE, ("crda> Number of channels = %d\n", RecId));
} /* End of CFG80211_RegRuleApply */

/*
 * ========================================================================
 * Routine Description:
 *	Inform CFG80211 about association status.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pBSSID			- the BSSID of the AP
 *	pReqIe			- the element list in the association request frame
 *	ReqIeLen		- the request element length
 *	pRspIe			- the element list in the association response frame
 *	RspIeLen		- the response element length
 *	FlgIsSuccess	- 1: success; otherwise: fail
 *
 * Return Value:
 *	None
 * ========================================================================
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

	CFG80211DBG(DBG_LVL_TRACE, ("80211> CFG80211_ConnectResultInform ==>\n"));

	if (pAd->cfg80211_ctrl.FlgCfg80211Scanning == TRUE ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) {
		CFG80211DBG(DBG_LVL_ERROR, ("Abort running scan\n"));
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
		pAd->cfg80211_ctrl.FlgCfg80211Scanning = FALSE;
		CFG80211OS_ScanEnd(CFG80211CB, TRUE);
	}

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
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *
 * Return Value:
 *	TRUE			- re-init successfully
 *	FALSE			- re-init fail
 *
 * Note:
 *	CFG80211_SupBandInit() is called in xx_probe().
 *	But we do not have complete chip information in xx_probe() so we
 *	need to re-init bands in xx_open().
 * ========================================================================
 */
BOOLEAN CFG80211_SupBandReInit(VOID *pAdCB, VOID *wdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	struct wifi_dev *curr_wdev = (struct wifi_dev *)wdev;
	CFG80211_BAND BandInfo;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> re-init bands...\n"));
	/* re-init bands */
	os_zero_mem(&BandInfo, sizeof(BandInfo));
    CFG80211_BANDINFO_FILL(pAd, curr_wdev, &BandInfo);

	return CFG80211OS_SupBandReInit(CFG80211CB, &BandInfo);
} /* End of CFG80211_SupBandReInit */


INT CFG80211_reSetToDefault(
	IN VOID                                         *pAdCB)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s\n", __func__));
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterActionFrame = FALSE;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ProbeReqCount = 0;
	pCfg80211_ctrl->cfg80211MainDev.Cfg80211ActionCount = 0;
	pCfg80211_ctrl->Cfg80211RocTimerInit = FALSE;
	pCfg80211_ctrl->Cfg80211RocTimerRunning = FALSE;
	pCfg80211_ctrl->FlgCfg80211Scanning = FALSE;
	/* pCfg80211_ctrl->isMccOn = FALSE; */
	return TRUE;
}

/* initList(&pAd->Cfg80211VifDevSet.vifDevList); */
/* initList(&pAd->cfg80211_ctrl.cfg80211TxPacketList); */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA) || defined(APCLI_CFG80211_SUPPORT)
BOOLEAN CFG80211_checkScanResInKernelCache(
	IN VOID *pAdCB,
	IN UCHAR *pBSSID,
	IN UCHAR *pSsid,
	IN INT ssidLen)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct cfg80211_bss *bss;

	bss = cfg80211_get_bss(pWiphy, NULL, pBSSID,
						   pSsid, ssidLen,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
				IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);
#else
				WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
#endif

	if (bss) {
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}

	return FALSE;
}

BOOLEAN CFG80211_checkScanTable(
	IN VOID *pAdCB)
{
#ifndef APCLI_CFG80211_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdCB;
	CFG80211_CB *pCfg80211_CB  = (CFG80211_CB *)pAd->pCfg80211_CB;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	ULONG bss_idx = BSS_NOT_FOUND;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	UINT32 CenFreq;
	UINT64 timestamp;
	struct timespec64 tv;
	UCHAR *ie, ieLen = 0;
	BOOLEAN isOk = FALSE;
	BSS_ENTRY *pBssEntry;
	USHORT ifIndex = 0;
	PAPCLI_STRUCT pApCliEntry = NULL;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR)) {
		CFG80211DBG(DBG_LVL_ERROR, ("pAd->ApCliMlmeAux.Bssid ==> ZERO_MAC_ADDR\n"));
		/* ToDo: pAd->ApCfg.ApCliTab[0].CfgApCliBssid */
		return FALSE;
	}

	/* Fake TSF */
	ktime_get_real_ts64(&tv);
	mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_nsec;
	bss = cfg80211_get_bss(pWiphy, NULL, pApCliEntry->MlmeAux.Bssid,
						   pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
						IEEE80211_BSS_TYPE_ESS, IEEE80211_PRIVACY_ANY);
#else
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);
#endif

	if (bss) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Found %s in Kernel_ScanTable with CH[%d]\n", pApCliEntry->MlmeAux.Ssid, bss->channel->center_freq));
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
		bss->tsf = timestamp;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0) */
		CFG80211OS_PutBss(pWiphy, bss);
		return TRUE;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Can't Found %s in Kernel_ScanTable & Try Fake it\n", pApCliEntry->MlmeAux.Ssid));

	bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, pApCliEntry->MlmeAux.Ssid, pApCliEntry->MlmeAux.SsidLen);

	if (bss_idx != BSS_NOT_FOUND) {
		/* Since the cfg80211 kernel scanTable not exist this Entry,
		 * Build an Entry for this connect inform event.
			 */
		pBssEntry = &pAd->ScanTab.BssEntry[bss_idx];
#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)

		if (pAd->ScanTab.BssEntry[bss_idx].Channel > 14)
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_5GHZ);
		else
			CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel, IEEE80211_BAND_2GHZ);

#else
		CenFreq = ieee80211_channel_to_frequency(pBssEntry->Channel);
#endif
		chan = ieee80211_get_channel(pWiphy, CenFreq);
		ieLen = 2 + pApCliEntry->MlmeAux.SsidLen + pBssEntry->VarIeFromProbeRspLen;
		os_alloc_mem(NULL, (UCHAR **)&ie, ieLen);

		if (!ie) {
			CFG80211DBG(DBG_LVL_ERROR, ("Memory Allocate Fail in CFG80211_checkScanTable\n"));
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

		if (bss) {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("Fake New %s(%02x:%02x:%02x:%02x:%02x:%02x) in Kernel_ScanTable with CH[%d][%d] BI:%d len:%d\n",
					  pApCliEntry->MlmeAux.Ssid,
					  PRINT_MAC(pApCliEntry->MlmeAux.Bssid), bss->channel->center_freq, pBssEntry->Channel,
					  pApCliEntry->MlmeAux.BeaconPeriod, pBssEntry->VarIeFromProbeRspLen));
			CFG80211OS_PutBss(pWiphy, bss);
			isOk = TRUE;
		}

		if (ie != NULL)
			os_free_mem(ie);

		if (isOk)
			return TRUE;
	} else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s Not In Driver Scan Table\n", pApCliEntry->MlmeAux.Ssid));

	return FALSE;
#else
	return TRUE;
#endif /* APCLI_CFG80211_SUPPORT */
}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

/* CFG_TODO */
UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch)
{
	UCHAR ret_channel;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	UCHAR ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);

	if (ht_bw == BW_40) {
		if (ext_cha == EXTCHA_ABOVE)
			ret_channel = prim_ch + 2;
		else {
			if (prim_ch == 14)
				ret_channel = prim_ch - 1;
			else
				ret_channel = prim_ch - 2;
		}
	} else
		ret_channel = prim_ch;

	return ret_channel;
}

VOID CFG80211_JoinIBSS(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID)
{
}

#ifdef MT_MAC
VOID CFG80211_InitTxSCallBack(RTMP_ADAPTER *pAd)
{
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %s:: Only MT_MAC support this feature.\n", __func__));
		return;
	}

#ifdef CFG_TDLS_SUPPORT
	AddTxSTypePerPkt(pAd, PID_TDLS, TXS_FORMAT0, TdlsTxSHandler);
	TxSTypeCtlPerPkt(pAd, PID_TDLS, TXS_FORMAT0, TRUE, TRUE, FALSE, 0);
#endif /* CFG_TDLS_SUPPORT */
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()\n", __func__));
}
#endif /* MT_MAC */

#endif /* RT_CFG80211_SUPPORT */

