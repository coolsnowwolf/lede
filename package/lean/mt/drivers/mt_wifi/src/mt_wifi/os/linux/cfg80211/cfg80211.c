/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2009, Ralink Technology, Inc.
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
 *		1. 2009/09/17	Sample Lin
 *			(1) Init version.
 *		2. 2009/10/27	Sample Lin
 *			(1) Do not use ieee80211_register_hw() to create virtual interface.
 *				Use wiphy_register() to register nl80211 command handlers.
 *			(2) Support iw utility.
 *		3. 2009/11/03	Sample Lin
 *			(1) Change name MAC80211 to CFG80211.
 *			(2) Modify CFG80211_OpsChannelSet().
 *			(3) Move CFG80211_Register()/CFG80211_UnRegister() to open/close.
 *		4. 2009/12/16	Sample Lin
 *			(1) Patch for Linux 2.6.32.
 *			(2) Add more supported functions in CFG80211_Ops.
 *		5. 2010/12/10	Sample Lin
 *			(1) Modify for OS_ABL.
 *		6. 2011/04/19	Sample Lin
 *			(1) Add more supported functions in CFG80211_Ops v33 ~ 38.
 *
 *	Note:
 *		The feature is supported only in "LINUX" 2.6.28 ~ 2.6.38.
 *
 ***************************************************************************/


#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"


#if (KERNEL_VERSION(2, 6, 28) <= LINUX_VERSION_CODE)
#ifdef RT_CFG80211_SUPPORT

/* 36 ~ 64, 100 ~ 136, 140 ~ 161 */
#define CFG80211_NUM_OF_CHAN_5GHZ	(sizeof(Cfg80211_Chan)-CFG80211_NUM_OF_CHAN_2GHZ)

#ifdef OS_ABL_FUNC_SUPPORT
/*
 *	Array of bitrates the hardware can operate with
 *	in this band. Must be sorted to give a valid "supported
 *	rates" IE, i.e. CCK rates first, then OFDM.
 *
 *	For HT, assign MCS in another structure, ieee80211_sta_ht_cap.
 */
const struct ieee80211_rate Cfg80211_SupRate[12] = {
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 10,    /* bitrate in units of 100 Kbps */
		.hw_value = 0,
		.hw_value_short = 0,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 20,
		.hw_value = 1,
		.hw_value_short = 1,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 55,
		.hw_value = 2,
		.hw_value_short = 2,
	},
	{
		.flags = IEEE80211_RATE_SHORT_PREAMBLE,
		.bitrate = 110,
		.hw_value = 3,
		.hw_value_short = 3,
	},
	{
		.flags = 0,
		.bitrate = 60,
		.hw_value = 4,
		.hw_value_short = 4,
	},
	{
		.flags = 0,
		.bitrate = 90,
		.hw_value = 5,
		.hw_value_short = 5,
	},
	{
		.flags = 0,
		.bitrate = 120,
		.hw_value = 6,
		.hw_value_short = 6,
	},
	{
		.flags = 0,
		.bitrate = 180,
		.hw_value = 7,
		.hw_value_short = 7,
	},
	{
		.flags = 0,
		.bitrate = 240,
		.hw_value = 8,
		.hw_value_short = 8,
	},
	{
		.flags = 0,
		.bitrate = 360,
		.hw_value = 9,
		.hw_value_short = 9,
	},
	{
		.flags = 0,
		.bitrate = 480,
		.hw_value = 10,
		.hw_value_short = 10,
	},
	{
		.flags = 0,
		.bitrate = 540,
		.hw_value = 11,
		.hw_value_short = 11,
	},
};
#endif /* OS_ABL_FUNC_SUPPORT */

/* all available channels */
static const UCHAR Cfg80211_Chan[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	/* 802.11 UNI / HyperLan 2 */
	36, 38, 40, 44, 46, 48, 52, 54, 56, 60, 62, 64,

	/* 802.11 HyperLan 2 */
	100, 104, 108, 112, 116, 118, 120, 124, 126, 128, 132, 134, 136,

	/* 802.11 UNII */
	140, 149, 151, 153, 157, 159, 161, 165, 167, 169, 171, 173,

	/* Japan */
	184, 188, 192, 196, 208, 212, 216,
};


static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC
#endif /*DOT11W_PMF_SUPPORT*/
};


/* get RALINK pAd control block in 80211 Ops */
#define MAC80211_PAD_GET(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 ("80211> %s but pAd = NULL!", __func__));	\
			return -EINVAL;											\
		}															\
	}

#define MAC80211_PAD_GET_NO_RV(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 ("80211> %s but pAd = NULL!", __func__));	\
			return;											\
		}															\
	}

#define MAC80211_PAD_GET_RETURN_NULL(__pAd, __pWiphy)							\
	{																\
		ULONG *__pPriv;												\
		__pPriv = (ULONG *)(wiphy_priv(__pWiphy));					\
		__pAd = (VOID *)(*__pPriv);									\
		if (__pAd == NULL) {											\
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,								\
					 ("80211> %s but pAd = NULL!", __func__));	\
			return NULL;											\
		}															\
	}


/*
 * ========================================================================
 * Routine Description:
 *	Set channel.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pChan			- Channel information
 *	ChannelType		- Channel type
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: set channel, set freq
 *
 *	enum nl80211_channel_type {
 *		NL80211_CHAN_NO_HT,
 *		NL80211_CHAN_HT20,
 *		NL80211_CHAN_HT40MINUS,
 *		NL80211_CHAN_HT40PLUS
 *	};
 * ========================================================================
 */
#if (KERNEL_VERSION(3, 6, 0) > LINUX_VERSION_CODE)
#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
static int CFG80211_OpsChannelSet(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pDev,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)

#else
static int CFG80211_OpsChannelSet(
	IN struct wiphy					*pWiphy,
	IN struct ieee80211_channel		*pChan,
	IN enum nl80211_channel_type	ChannelType)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CFG80211_CB *p80211CB;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	UINT32 ChanId;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(DBG_LVL_TRACE, ("80211> Channel = %d, Type = %d\n", ChanId, ChannelType));
	/* init */
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	p80211CB = NULL;
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	if (p80211CB == NULL) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
		return 0;
	}

	ChanInfo.IfType = pDev->ieee80211_ptr->iftype;

	if (ChannelType == NL80211_CHAN_NO_HT)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
	else if (ChannelType == NL80211_CHAN_HT20)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
	else if (ChannelType == NL80211_CHAN_HT40MINUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;
	else if (ChannelType == NL80211_CHAN_HT40PLUS)
		ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;

	ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;
	/* set channel */
	RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	return 0;
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0) */

/*
 * ========================================================================
 * Routine Description:
 *	Change type/configuration of virtual interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	IfIndex			- Interface index
 *	Type			- Interface type, managed/adhoc/ap/station, etc.
 *	pFlags			- Monitor flags
 *	pParams			- Mesh parameters
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: set type, set monitor
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNetDevIn,
	IN enum nl80211_iftype			Type,
	IN UINT32							*pFlags,
	struct vif_params				*pParams)
#else
static int CFG80211_OpsVirtualInfChg(
	IN struct wiphy					*pWiphy,
	IN int							IfIndex,
	IN enum nl80211_iftype			Type,
	IN UINT32							*pFlags,
	struct vif_params				*pParams)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CFG80211_CB *pCfg80211_CB;
	struct net_device *pNetDev;
	CMD_RTPRIV_IOCTL_80211_VIF_PARM VifInfo;
	UINT oldType = pNetDevIn->ieee80211_ptr->iftype;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	CFG80211DBG(DBG_LVL_TRACE, ("80211> IfTypeChange %d ==> %d\n", oldType, Type));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* sanity check */
	{
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Wrong interface type %d!\n", Type));
		return -EINVAL;
	} /* End of if */

	/* update interface type */
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
	pNetDev = pNetDevIn;
#else
	pNetDev = __dev_get_by_index(&init_net, IfIndex);
#endif /* LINUX_VERSION_CODE */

	if (pNetDev == NULL)
		return -ENODEV;

	pNetDev->ieee80211_ptr->iftype = Type;
	VifInfo.net_dev = pNetDev;
	VifInfo.newIfType = Type;
	VifInfo.oldIfType = oldType;

	if (pFlags != NULL) {
		VifInfo.MonFilterFlag = 0;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_FCSFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_FCSFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_FCSFAIL) == NL80211_MNTR_FLAG_PLCPFAIL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_PLCPFAIL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_CONTROL)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_CONTROL;

		if (((*pFlags) & NL80211_MNTR_FLAG_CONTROL) == NL80211_MNTR_FLAG_OTHER_BSS)
			VifInfo.MonFilterFlag |= RT_CMD_80211_FILTER_OTHER_BSS;
	}

	/* Type transer from linux to driver defined */
	if (Type == NL80211_IFTYPE_STATION)
		Type = RT_CMD_80211_IFTYPE_STATION;
	else if (Type == NL80211_IFTYPE_ADHOC)
		Type = RT_CMD_80211_IFTYPE_ADHOC;
	else if (Type == NL80211_IFTYPE_MONITOR)
		Type = RT_CMD_80211_IFTYPE_MONITOR;

#ifdef CONFIG_AP_SUPPORT
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	else if (Type == NL80211_IFTYPE_P2P_CLIENT)
		Type = RT_CMD_80211_IFTYPE_P2P_CLIENT;
	else if (Type == NL80211_IFTYPE_P2P_GO)
		Type = RT_CMD_80211_IFTYPE_P2P_GO;

#endif /* LINUX_VERSION_CODE 2.6.37 */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_DRIVER_80211_VIF_CHG(pAd, &VifInfo);
	/*CFG_TODO*/
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pCfg80211_CB->MonFilterFlag = VifInfo.MonFilterFlag;
	return 0;
}

#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
#if defined(SIOCGIWSCAN) || defined(RT_CFG80211_SUPPORT)
extern int rt_ioctl_siwscan(struct net_device *dev,
							struct iw_request_info *info,
							union iwreq_data *wreq, char *extra);
#endif /* LINUX_VERSION_CODE: 2.6.30 */
/*
 * ========================================================================
 * Routine Description:
 *	Request to do a scan. If returning zero, the scan request is given
 *	the driver, and will be valid until passed to cfg80211_scan_done().
 *	For scan results, call cfg80211_inform_bss(); you can call this outside
 *	the scan/scan_done bracket too.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pRequest		- Scan request
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: scan
 *
 *	struct cfg80211_scan_request {
 *		struct cfg80211_ssid *ssids;
 *		int n_ssids;
 *		struct ieee80211_channel **channels;
 *		UINT32 n_channels;
 *		const u8 *ie;
 *		size_t ie_len;
 *
 *	 * @ssids: SSIDs to scan for (active scan only)
 *	 * @n_ssids: number of SSIDs
 *	 * @channels: channels to scan on.
 *	 * @n_channels: number of channels for each band
 *	 * @ie: optional information element(s) to add into Probe Request or %NULL
 *	 * @ie_len: length of ie in octets
 * ========================================================================
 */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct cfg80211_scan_request *pRequest)
#else
static int CFG80211_OpsScan(
	IN struct wiphy					*pWiphy,
	IN struct net_device			*pNdev,
	IN struct cfg80211_scan_request *pRequest)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
	VOID *pAd;
	CFG80211_CB *pCfg80211_CB;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *pNdev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	MAC80211_PAD_GET(pAd, pWiphy);
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	RTMP_DRIVER_NET_DEV_GET(pAd, &pNdev);
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_TRACE, ("========================================================================\n"));
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==> %s(%d)\n", __func__, pNdev->name, pNdev->ieee80211_ptr->iftype));
	/* YF_TODO: record the scan_req per netdevice */
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pCfg80211_CB->pCfg80211_ScanReq = pRequest; /* used in scan end */
	CFG80211OS_ScanEnd(pCfg80211_CB, TRUE);
	return 0;
	/* return -EOPNOTSUPP; */
}
#endif /* LINUX_VERSION_CODE */


#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
#endif /* LINUX_VERSION_CODE */


#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
/*
 * ========================================================================
 * Routine Description:
 *	Set the transmit power according to the parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	Type			-
 *	dBm				- dBm
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	Type -
 *	enum nl80211_tx_power_setting - TX power adjustment
 *	 @NL80211_TX_POWER_AUTOMATIC: automatically determine transmit power
 *	 @NL80211_TX_POWER_LIMITED: limit TX power by the mBm parameter
 *	 @NL80211_TX_POWER_FIXED: fix TX power to the mBm parameter
 * ========================================================================
 */
static int CFG80211_OpsTxPwrSet(
	IN struct wiphy						*pWiphy,
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#endif
#if (KERNEL_VERSION(2, 6, 36) <= LINUX_VERSION_CODE)
	IN enum nl80211_tx_power_setting	Type,
#else
	IN enum tx_power_setting			Type,
#endif /* LINUX_VERSION_CODE */
	IN int								dBm)
{
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	return -EOPNOTSUPP;
}


/*
 * ========================================================================
 * Routine Description:
 *	Store the current TX power into the dbm variable.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pdBm			- dBm
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 *Note:
 * ========================================================================
 */
static int CFG80211_OpsTxPwrGet(
	IN struct wiphy						*pWiphy,
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#endif
	IN int								*pdBm)
{
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsTxPwrGet */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0) */

/*
 * ========================================================================
 * Routine Description:
 *	Power management.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	FlgIsEnabled	-
 *	Timeout			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPwrMgmt(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN bool							 enabled,
	IN INT32							 timeout)
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==> power save %s\n", __func__, (enabled ? "enable" : "disable")));
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_POWER_MGMT_SET(pAd, enabled);
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Get information for a specific station.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	pMac			- STA MAC
 *	pSinfo			- STA INFO
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8 * pMac,
	IN struct station_info				*pSinfo)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_STA StaInfo;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* init */
	memset(pSinfo, 0, sizeof(*pSinfo));
	memset(&StaInfo, 0, sizeof(StaInfo));
	memcpy(StaInfo.MAC, pMac, 6);

	/* get sta information */
	if (RTMP_DRIVER_80211_STA_GET(pAd, &StaInfo) != NDIS_STATUS_SUCCESS)
		return -ENOENT;

	if (StaInfo.TxRateFlags != RT_CMD_80211_TXRATE_LEGACY) {
		pSinfo->txrate.flags = RATE_INFO_FLAGS_MCS;

		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_BW_40)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;

		if (StaInfo.TxRateFlags & RT_CMD_80211_TXRATE_SHORT_GI)
			pSinfo->txrate.flags |= RATE_INFO_FLAGS_SHORT_GI;

		pSinfo->txrate.mcs = StaInfo.TxRateMCS;
	} else
		pSinfo->txrate.legacy = StaInfo.TxRateMCS;

	pSinfo->filled |= STATION_INFO_TX_BITRATE;
	/* fill signal */
	pSinfo->signal = StaInfo.Signal;
	pSinfo->filled |= STATION_INFO_SIGNAL;
#ifdef CONFIG_AP_SUPPORT
	/* fill tx count */
	pSinfo->tx_packets = StaInfo.TxPacketCnt;
	pSinfo->filled |= STATION_INFO_TX_PACKETS;
	/* fill inactive time */
	pSinfo->inactive_time = StaInfo.InactiveTime;
	pSinfo->filled |= STATION_INFO_INACTIVE_TIME;
#endif /* CONFIG_AP_SUPPORT */
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	List all stations known, e.g. the AP on managed interfaces.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	Idx				-
 *	pMac			-
 *	pSinfo			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsStaDump(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN int								Idx,
	IN UINT8 * pMac,
	IN struct station_info				*pSinfo)
{
	VOID *pAd;

	if (Idx != 0)
		return -ENOENT;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	return -EOPNOTSUPP;
} /* End of CFG80211_OpsStaDump */


/*
 * ========================================================================
 * Routine Description:
 *	Notify that wiphy parameters have changed.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	Changed			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsWiphyParamsSet(
	IN struct wiphy						*pWiphy,
	IN UINT32							Changed)
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);

	if (Changed & WIPHY_PARAM_RTS_THRESHOLD) {
		RTMP_DRIVER_80211_RTS_THRESHOLD_ADD(pAd, (void *)pWiphy->rts_threshold);
		CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==> rts_threshold(%d)\n", __func__, pWiphy->rts_threshold));
		return 0;
	} else if (Changed & WIPHY_PARAM_FRAG_THRESHOLD) {
		RTMP_DRIVER_80211_FRAG_THRESHOLD_ADD(pAd, (void *)pWiphy->frag_threshold);
		CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==> frag_threshold(%d)\n", __func__, pWiphy->frag_threshold));
		return 0;
	}

	return -EOPNOTSUPP;
} /* End of CFG80211_OpsWiphyParamsSet */


/*
 * ========================================================================
 * Routine Description:
 *	Add a key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pParams			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	pMacAddr will be NULL when adding a group key.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8 * pMacAddr,
	IN struct key_params				*pParams)
#else

static int CFG80211_OpsKeyAdd(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8 * pMacAddr,
	IN struct key_params				*pParams)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;

	p80211CB = NULL;
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
#ifdef RT_CFG80211_DEBUG
	hex_dump("KeyBuf=", (UINT8 *)pParams->key, pParams->key_len);
#endif /* RT_CFG80211_DEBUG */
	CFG80211DBG(DBG_LVL_TRACE, ("80211> KeyIdx = %d\n", KeyIdx));

	if (pParams->key_len >= sizeof(KeyInfo.KeyBuf))
		return -EINVAL;

	/* End of if */
	/* init */
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	memcpy(KeyInfo.KeyBuf, pParams->key, pParams->key_len);
	KeyInfo.KeyBuf[pParams->key_len] = 0x00;
	KeyInfo.KeyId = KeyIdx;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	KeyInfo.bPairwise = Pairwise;
#endif /* LINUX_VERSION_CODE: 2,6,37 */
	KeyInfo.KeyLen = pParams->key_len;
#ifdef DOT11W_PMF_SUPPORT
#endif	/* DOT11W_PMF_SUPPORT */
		if (pParams->cipher == WLAN_CIPHER_SUITE_WEP40)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP40;
		else if (pParams->cipher == WLAN_CIPHER_SUITE_WEP104)
			KeyInfo.KeyType = RT_CMD_80211_KEY_WEP104;
		else if ((pParams->cipher == WLAN_CIPHER_SUITE_TKIP) ||
				 (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)) {
			KeyInfo.KeyType = RT_CMD_80211_KEY_WPA;

			if (pParams->cipher == WLAN_CIPHER_SUITE_TKIP)
				KeyInfo.cipher = Ndis802_11TKIPEnable;
			else if (pParams->cipher == WLAN_CIPHER_SUITE_CCMP)
				KeyInfo.cipher = Ndis802_11AESEnable;
		} else
			return -ENOTSUPP;

	KeyInfo.pNetDev = pNdev;
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		if (pMacAddr) {
			CFG80211DBG(DBG_LVL_OFF, ("80211> KeyAdd STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
									  PRINT_MAC(pMacAddr)));
			NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
		}

		CFG80211DBG(DBG_LVL_OFF, ("80211> AP Key Add\n"));
		RTMP_DRIVER_80211_AP_KEY_ADD(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
	}

#ifdef RT_P2P_SPECIFIC_WIRELESS_EVENT

	if (pMacAddr) {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> P2pSendWirelessEvent(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
									PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_SEND_WIRELESS_EVENT(pAd, pMacAddr);
	}

#endif /* RT_P2P_SPECIFIC_WIRELESS_EVENT */
	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Get information about the key with the given parameters.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	Pairwise		-
 *	pMacAddr		-
 *	pCookie			-
 *	pCallback		-
 *
 * Return Value:
 *	0			- success
 *	-x			- fail
 *
 * Note:
 *	pMacAddr will be NULL when requesting information for a group key.
 *
 *	All pointers given to the pCallback function need not be valid after
 *	it returns.
 *
 *	This function should return an error if it is not possible to
 *	retrieve the key, -ENOENT if it doesn't exist.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
			struct key_params *))
#else

static int CFG80211_OpsKeyGet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr,
	IN void								*pCookie,
	IN void								(*pCallback)(void *cookie,
			struct key_params *))
#endif /* LINUX_VERSION_CODE */
{
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	return -ENOTSUPP;
}


/*
 * ========================================================================
 * Routine Description:
 *	Remove a key given the pMacAddr (NULL for a group key) and KeyIdx.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *	pMacAddr		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	return -ENOENT if the key doesn't exist.
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Pairwise,
	IN const UINT8						*pMacAddr)
#else

static int CFG80211_OpsKeyDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN const UINT8						*pMacAddr)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_KEY KeyInfo;
	CFG80211_CB *p80211CB;

	p80211CB = NULL;
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));

	if (pMacAddr) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> KeyDel STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n", PRINT_MAC(pMacAddr)));
		NdisCopyMemory(KeyInfo.MAC, pMacAddr, MAC_ADDR_LEN);
	}

	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);
	memset(&KeyInfo, 0, sizeof(KeyInfo));
	KeyInfo.KeyId = KeyIdx;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_TRACE, ("80211> KeyDel isPairwise %d\n", Pairwise));
	KeyInfo.bPairwise = Pairwise;
#endif /* LINUX_VERSION_CODE 2.6.37 */
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO)) {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> AP Key Del\n"));
		RTMP_DRIVER_80211_AP_KEY_DEL(pAd, &KeyInfo);
	} else
#endif /* CONFIG_AP_SUPPORT */
	{
		CFG80211DBG(DBG_LVL_TRACE, ("80211> STA Key Del\n"));

		if (pMacAddr) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> STA Key Del -- DISCONNECT\n"));
			RTMP_DRIVER_80211_STA_LEAVE(pAd, pNdev);
		}
	}

	return 0;
}


/*
 * ========================================================================
 * Routine Description:
 *	Set the default key on an interface.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			-
 *	KeyIdx			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx,
	IN bool								Unicast,
	IN bool								Multicast)
#else

static int CFG80211_OpsKeyDefaultSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT8							KeyIdx)
#endif /* LINUX_VERSION_CODE */
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_TRACE, ("80211> Default KeyIdx = %d\n", KeyIdx));
#ifdef CONFIG_AP_SUPPORT

	if ((pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_AP) ||
		(pNdev->ieee80211_ptr->iftype == RT_CMD_80211_IFTYPE_P2P_GO))
		RTMP_DRIVER_80211_AP_KEY_DEFAULT_SET(pAd, KeyIdx);
	else
#endif /* CONFIG_AP_SUPPORT */
		RTMP_DRIVER_80211_STA_KEY_DEFAULT_SET(pAd, KeyIdx);

	return 0;
} /* End of CFG80211_OpsKeyDefaultSet */


/*
 * ========================================================================
 * Routine Description:
 *	Connect to the ESS with the specified parameters. When connected,
 *	call cfg80211_connect_result() with status code %WLAN_STATUS_SUCCESS.
 *	If the connection fails for some reason, call cfg80211_connect_result()
 *	with the status from the AP.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pSme			-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *
 *	You must use "iw ra0 connect xxx", then "iw ra0 disconnect";
 *	You can not use "iw ra0 connect xxx" twice without disconnect;
 *	Or you will suffer "command failed: Operation already in progress (-114)".
 *
 *	You must support add_key and set_default_key function;
 *	Or kernel will crash without any error message in linux 2.6.32.
 *
 *
 * struct cfg80211_connect_params - Connection parameters
 *
 * This structure provides information needed to complete IEEE 802.11
 * authentication and association.
 *
 *  @channel: The channel to use or %NULL if not specified (auto-select based
 *	on scan results)
 *  @bssid: The AP BSSID or %NULL if not specified (auto-select based on scan
 *	results)
 * @ssid: SSID
 *  @ssid_len: Length of ssid in octets
 *  @auth_type: Authentication type (algorithm)
 *
 * @ie: IEs for association request
 * @ie_len: Length of assoc_ie in octets
 *
 * @privacy: indicates whether privacy-enabled APs should be used
 * @crypto: crypto settings
 * @key_len: length of WEP key for shared key authentication
 * @key_idx: index of WEP key for shared key authentication
 * @key: WEP key for shared key authentication
 * ========================================================================
 */
static int CFG80211_OpsConnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_connect_params	*pSme)
{
	return 0;
} /* End of CFG80211_OpsConnect */


/*
 * ========================================================================
 * Routine Description:
 *	Disconnect from the BSS/ESS.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	ReasonCode		-
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	For iw utility: connect
 *========================================================================
 */
static int CFG80211_OpsDisconnect(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN UINT16								ReasonCode)
{
	return 0;
}
#endif /* LINUX_VERSION_CODE */


#ifdef RFKILL_HW_SUPPORT
static int CFG80211_OpsRFKill(
	IN struct wiphy						*pWiphy)
{
	VOID		*pAd;
	BOOLEAN		active;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	RTMP_DRIVER_80211_RFKILL(pAd, &active);
	wiphy_rfkill_set_hw_state(pWiphy, !active);
	return active;
}


VOID CFG80211_RFKillStatusUpdate(
	IN PVOID							pAd,
	IN BOOLEAN							active)
{
	struct wiphy *pWiphy;
	CFG80211_CB *pCfg80211_CB;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	RTMP_DRIVER_80211_CB_GET(pAd, &pCfg80211_CB);
	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	wiphy_rfkill_set_hw_state(pWiphy, !active);
}
#endif /* RFKILL_HW_SUPPORT */

#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)

/*
 * ========================================================================
 * Routine Description:
 *	Cache a PMKID for a BSSID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 *	This is mostly useful for fullmac devices running firmwares capable of
 *	generating the (re) association RSN IE.
 *	It allows for faster roaming between WPA2 BSSIDs.
 * ========================================================================
 */
static int CFG80211_OpsPmksaSet(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
	return 0;
} /* End of CFG80211_OpsPmksaSet */


/*
 * ========================================================================
 * Routine Description:
 *	Delete a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *	pPmksa			- PMKID information
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPmksaDel(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev,
	IN struct cfg80211_pmksa			*pPmksa)
{
	return 0;
} /* End of CFG80211_OpsPmksaDel */


/*
 * ========================================================================
 * Routine Description:
 *	Flush a cached PMKID.
 *
 * Arguments:
 *	pWiphy			- Wireless hardware description
 *	pNdev			- Network device interface
 *
 * Return Value:
 *	0				- success
 *	-x				- fail
 *
 * Note:
 * ========================================================================
 */
static int CFG80211_OpsPmksaFlush(
	IN struct wiphy						*pWiphy,
	IN struct net_device				*pNdev)
{
	return 0;
} /* End of CFG80211_OpsPmksaFlush */
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsRemainOnChannel(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *pWdev,
	IN struct ieee80211_channel *pChan,
	IN unsigned int duration,
	OUT u64 *cookie)
#else /* LINUX_VERSION_CODE >= 3.8.0 */
static int CFG80211_OpsRemainOnChannel(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *pWdev,
#else
	IN struct net_device *dev,
#endif
	IN struct ieee80211_channel *pChan,
	IN enum nl80211_channel_type ChannelType,
	IN unsigned int duration,
	OUT u64 *cookie)
#endif /* LINUX_VERSION_CODE < 3.6.0 */
{
	VOID *pAd;
	UINT32 ChanId;
	CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
	u32 rndCookie;
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	INT ChannelType = RT_CMD_80211_CHANTYPE_HT20;
#endif /* LINUX_VERSION_CODE: 3.8.0 */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;

	dev = pWdev->netdev;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	rndCookie = MtRandom32() | 1;
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/*CFG_TODO: Shall check channel type*/
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(DBG_LVL_TRACE, ("%s: CH = %d, Type = %d, duration = %d, cookie=%d\n", __func__,
								ChanId, ChannelType, duration, rndCookie));
	/* init */
	*cookie = rndCookie;
	memset(&ChanInfo, 0, sizeof(ChanInfo));
	ChanInfo.ChanId = ChanId;
	ChanInfo.IfType = dev->ieee80211_ptr->iftype;
	ChanInfo.ChanType = ChannelType;
	ChanInfo.chan = pChan;
	ChanInfo.cookie = rndCookie;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	ChanInfo.pWdev = pWdev;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	/* set channel */
	RTMP_DRIVER_80211_REMAIN_ON_CHAN_SET(pAd, &ChanInfo, duration);
	return 0;
}

static void CFG80211_OpsMgmtFrameRegister(
	struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct wireless_dev *wdev,
#else
	struct net_device *dev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	UINT16 frame_type, bool reg)
{
	VOID *pAd;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	MAC80211_PAD_GET_NO_RV(pAd, pWiphy);
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	CFG80211DBG(DBG_LVL_INFO, ("frame_type = %x, req = %d , (%d)\n", frame_type, reg,  dev->ieee80211_ptr->iftype));

	if (frame_type == IEEE80211_STYPE_PROBE_REQ)
		RTMP_DRIVER_80211_MGMT_FRAME_REG(pAd, dev, reg);
	else if (frame_type == IEEE80211_STYPE_ACTION)
		RTMP_DRIVER_80211_ACTION_FRAME_REG(pAd, dev, reg);
	else
		CFG80211DBG(DBG_LVL_ERROR, ("Unkown frame_type = %x, req = %d\n", frame_type, reg));
}

/* Supplicant_NEW_TDLS */
#ifdef CFG_TDLS_SUPPORT
static int CFG80211_OpsTdlsMgmt
(
	IN struct wiphy *pWiphy,
	IN struct net_device *pDev,
	IN u8 *peer,
	IN u8 action_code,
	IN u8 dialog_token,
	IN u16 status_code,
	IN const u8 *extra_ies,
	IN size_t extra_ies_len
)
{
	int ret = 0;
	VOID *pAd;

	CFG80211DBG(DBG_LVL_ERROR, ("80211> extra_ies_len : %d ==>\n", extra_ies_len));
	MAC80211_PAD_GET(pAd, pWiphy);

	if (action_code == WLAN_TDLS_SETUP_REQUEST || action_code == WLAN_TDLS_SETUP_RESPONSE)
		RTMP_DRIVER_80211_STA_TDLS_SET_KEY_COPY_FLAG(pAd);

	switch (action_code) {
	case WLAN_TDLS_SETUP_REQUEST:
	case WLAN_TDLS_DISCOVERY_REQUEST:
	case WLAN_TDLS_SETUP_CONFIRM:
	case WLAN_TDLS_TEARDOWN:
	case WLAN_TDLS_SETUP_RESPONSE:
	case WLAN_PUB_ACTION_TDLS_DISCOVER_RES:
		cfg_tdls_build_frame(pAd, peer, dialog_token, action_code, status_code, extra_ies, extra_ies_len, FALSE, 0, 0);
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ:
		cfg_tdls_TunneledProbeRequest(pAd, peer, extra_ies, extra_ies_len);
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP:
		cfg_tdls_TunneledProbeResponse(pAd, peer, extra_ies, extra_ies_len);
		break;

	default:
		ret = -1;
		break;
	}

	return ret;
}

static int CFG80211_OpsTdlsOper(
	IN struct wiphy *pWiphy,
	IN struct net_device *pDev,
	IN u8 *peer,
	IN enum nl80211_tdls_operation oper)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);

	switch (oper) {
	case NL80211_TDLS_ENABLE_LINK:
		RTMP_DRIVER_80211_STA_TDLS_INSERT_DELETE_PENTRY(pAd, peer, tdls_insert_entry);
		break;

	case NL80211_TDLS_DISABLE_LINK:
		RTMP_DRIVER_80211_STA_TDLS_INSERT_DELETE_PENTRY(pAd, peer, tdls_delete_entry);
		break;

	default:
		CFG80211DBG(DBG_LVL_ERROR, ("%s Unhandled TdlsOper : %d ==>\n", __func__, oper));
	}

	return 0;
}
#endif /*CFG_TDLS_SUPPORT*/

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
static int CFG80211_OpsMgmtTx(
	IN struct wiphy *pWiphy,
	IN struct wireless_dev *wdev,
	IN struct ieee80211_channel *pChan,
	IN bool Offchan,
	IN unsigned int Wait,
	IN const u8 *pBuf,
	IN size_t Len,
	IN bool no_cck,
	IN bool done_wait_for_ack,
	IN u64 *pCookie)
#else
static int CFG80211_OpsMgmtTx(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#else
	IN struct net_device *pDev,
#endif
	IN struct ieee80211_channel *pChan,
	IN bool Offchan,
	IN enum nl80211_channel_type ChannelType,
	IN bool ChannelTypeValid,
	IN unsigned int Wait,
	IN const u8 *pBuf,
	IN size_t Len,
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	IN bool no_cck,
#endif
#if (KERNEL_VERSION(3, 3, 0) <= LINUX_VERSION_CODE)
	IN bool done_wait_for_ack,
#endif
	IN u64 *pCookie)
#endif /* LINUX_VERSION_CODE: 3.6.0 */
{
	VOID *pAd;
	UINT32 ChanId;
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct net_device *dev = NULL;
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	RTMP_DRIVER_NET_DEV_GET(pAd, &dev);
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	/* get channel number */
	ChanId = ieee80211_frequency_to_channel(pChan->center_freq);
	CFG80211DBG(DBG_LVL_INFO, ("80211> Mgmt Channel = %d\n", ChanId));
	/* Send the Frame with basic rate 6 */
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)

	if (no_cck)
		; /* pAd->isCfgDeviceInP2p = TRUE; */

#else
#endif
	*pCookie = 5678;
	RTMP_DRIVER_80211_CHANNEL_LOCK(pAd, ChanId);
	RTMP_DRIVER_80211_MGMT_FRAME_SEND(pAd, (VOID *)pBuf, Len);
	/* Mark it for using Supplicant-Based off-channel wait
	 *	if (Offchan)
	 *		RTMP_DRIVER_80211_CHANNEL_RESTORE(pAd);
	 */
	return 0;
}

static int CFG80211_OpsTxCancelWait(
	IN struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	IN struct wireless_dev *wdev,
#else
	IN struct net_device *pDev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	u64 cookie)
{
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	return 0;
}

static int CFG80211_OpsCancelRemainOnChannel(
	struct wiphy *pWiphy,
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	struct wireless_dev *wdev,
#else
	struct net_device *dev,
#endif /* LINUX_VERSION_CODE: 3.6.0 */
	u64 cookie)
{
	VOID *pAd;

	CFG80211DBG(DBG_LVL_INFO, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	/* It cause the Supplicant-based OffChannel Hang */
	RTMP_DRIVER_80211_CANCEL_REMAIN_ON_CHAN_SET(pAd, cookie);
	return 0;
}

#ifdef CONFIG_AP_SUPPORT
#if (KERNEL_VERSION(3, 4, 0) > LINUX_VERSION_CODE)
static int CFG80211_OpsSetBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct beacon_parameters *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	hex_dump("Beacon head", info->head, info->head_len);
	hex_dump("Beacon tail", info->tail, info->tail_len);
	CFG80211DBG(DBG_LVL_TRACE, ("80211>dtim_period = %d\n", info->dtim_period));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>interval = %d\n", info->interval));
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_TRACE, ("80211>ssid = %s\n", info->ssid));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>ssid_len = %d\n", info->ssid_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>beacon_ies_len = %d\n", info->beacon_ies_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>proberesp_ies_len = %d\n", info->proberesp_ies_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>assocresp_ies_len = %d\n", info->assocresp_ies_len));

	if (info->proberesp_ies_len > 0 && info->proberesp_ies)
		RTMP_DRIVER_80211_AP_PROBE_RSP(pAd, (VOID *)info->proberesp_ies, info->proberesp_ies_len);

	if (info->assocresp_ies_len > 0 && info->assocresp_ies)
		RTMP_DRIVER_80211_AP_ASSOC_RSP(pAd, (VOID *)info->assocresp_ies, info->assocresp_ies_len);

#endif
	os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
	NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
	NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = info->dtim_period;
	bcn.interval = info->interval;
	RTMP_DRIVER_80211_BEACON_SET(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsAddBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct beacon_parameters *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	hex_dump("Beacon head", info->head, info->head_len);
	hex_dump("Beacon tail", info->tail, info->tail_len);
	CFG80211DBG(DBG_LVL_TRACE, ("80211>dtim_period = %d\n", info->dtim_period));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>interval = %d\n", info->interval));
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	CFG80211DBG(DBG_LVL_TRACE, ("80211>ssid = %s\n", info->ssid));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>ssid_len = %d\n", info->ssid_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>beacon_ies_len = %d\n", info->beacon_ies_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>proberesp_ies_len = %d\n", info->proberesp_ies_len));
	CFG80211DBG(DBG_LVL_TRACE, ("80211>assocresp_ies_len = %d\n", info->assocresp_ies_len));

	if (info->proberesp_ies_len > 0 && info->proberesp_ies)
		RTMP_DRIVER_80211_AP_PROBE_RSP(pAd, (VOID *)info->proberesp_ies, info->proberesp_ies_len);

	if (info->assocresp_ies_len > 0 && info->assocresp_ies)
		RTMP_DRIVER_80211_AP_ASSOC_RSP(pAd, (VOID *)info->assocresp_ies, info->assocresp_ies_len);

#endif
	os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
	NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
	NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = info->dtim_period;
	bcn.interval = info->interval;
	RTMP_DRIVER_80211_BEACON_ADD(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsDelBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	RTMP_DRIVER_80211_BEACON_DEL(pAd);
	return 0;
}
#else /* ! LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) */
static int CFG80211_OpsStartAp(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_ap_settings *settings)
{
	VOID *pAdOrg;
	PRTMP_ADAPTER pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;
	INT apidx;
	struct wifi_dev *pWdev = NULL;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAdOrg, pWiphy);
	pAd = (PRTMP_ADAPTER)pAdOrg;
	apidx = CFG80211_FindMbssApIdxByNetDevice(pAd, netdev);

	if (apidx == WDEV_NOT_FOUND) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s failed - [ERROR]can't find wdev in driver MBSS.\n", __func__));
		return FALSE;
	}

	pWdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	NdisZeroMemory(&bcn, sizeof(CMD_RTPRIV_IOCTL_80211_BEACON));

	if (settings->beacon.head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, settings->beacon.head_len);
		NdisCopyMemory(beacon_head_buf, settings->beacon.head, settings->beacon.head_len);
	}

	if (settings->beacon.tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, settings->beacon.tail_len);
		NdisCopyMemory(beacon_tail_buf, settings->beacon.tail, settings->beacon.tail_len);
	}

	bcn.beacon_head_len = settings->beacon.head_len;
	bcn.beacon_tail_len = settings->beacon.tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;
	bcn.dtim_period = settings->dtim_period;
	bcn.interval = settings->beacon_interval;
	bcn.ssid_len = settings->ssid_len;
	bcn.privacy = settings->privacy;

	if (settings->crypto.akm_suites[0] == WLAN_AKM_SUITE_8021X) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> This is a 1X wdev\n"));
		pWdev->IsCFG1xWdev = TRUE;
	} else
		pWdev->IsCFG1xWdev = FALSE;

	NdisZeroMemory(&bcn.ssid[0], MAX_LEN_OF_SSID);

	if (settings->ssid && (settings->ssid_len <= 32))
		NdisCopyMemory(&bcn.ssid[0], settings->ssid, settings->ssid_len);

	bcn.auth_type = settings->auth_type;
#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)
	bcn.hidden_ssid = settings->hidden_ssid;
#endif /*LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0)*/
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)

	/* set channel callback has been replaced by using chandef of cfg80211_ap_settings */
	if (settings->chandef.chan) {
		CFG80211_CB *p80211CB;
		CMD_RTPRIV_IOCTL_80211_CHAN ChanInfo;
		/* UINT32 ChanId; */
		/* init */
		memset(&ChanInfo, 0, sizeof(ChanInfo));
		p80211CB = NULL;
		RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

		if (p80211CB == NULL) {
			CFG80211DBG(DBG_LVL_ERROR, ("80211> p80211CB == NULL!\n"));
			return 0;
		}

		/* get channel number */
		ChanInfo.ChanId = ieee80211_frequency_to_channel(settings->chandef.chan->center_freq);
		ChanInfo.CenterChanId = ieee80211_frequency_to_channel(settings->chandef.center_freq1);
		CFG80211DBG(DBG_LVL_ERROR, ("80211> Channel = %d, CenterChanId = %d\n", ChanInfo.ChanId, ChanInfo.CenterChanId));
		ChanInfo.IfType = RT_CMD_80211_IFTYPE_P2P_GO;
		CFG80211DBG(DBG_LVL_ERROR, ("80211> ChanInfo.IfType == %d!\n", ChanInfo.IfType));

		switch (settings->chandef.width) {
		case NL80211_CHAN_WIDTH_20_NOHT:
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;

		case NL80211_CHAN_WIDTH_20:
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT20;
			break;

		case NL80211_CHAN_WIDTH_40:
			if (settings->chandef.center_freq1 > settings->chandef.chan->center_freq)
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40PLUS;
			else
				ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_HT40MINUS;

			break;
#ifdef DOT11_VHT_AC

		case NL80211_CHAN_WIDTH_80:
			CFG80211DBG(DBG_LVL_ERROR, ("80211> NL80211_CHAN_WIDTH_80 CtrlCh: %d, CentCh: %d\n", ChanInfo.ChanId, ChanInfo.CenterChanId));
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_VHT80;
			break;

		/* Separated BW 80 and BW 160 is not supported yet */
		case NL80211_CHAN_WIDTH_80P80:
		case NL80211_CHAN_WIDTH_160:
#endif /* DOT11_VHT_AC */
		default:
			CFG80211DBG(DBG_LVL_ERROR, ("80211> Unsupported Chan Width: %d\n", settings->chandef.width));
			ChanInfo.ChanType = RT_CMD_80211_CHANTYPE_NOHT;
			break;
		}

		CFG80211DBG(DBG_LVL_ERROR, ("80211> ChanInfo.ChanType == %d!\n", ChanInfo.ChanType));
		ChanInfo.MonFilterFlag = p80211CB->MonFilterFlag;
		/* set channel */
		RTMP_DRIVER_80211_CHAN_SET(pAd, &ChanInfo);
	}

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) */
	RTMP_DRIVER_80211_BEACON_ADD(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

VOID CFG80211_UpdateAssocRespExtraIe(
	VOID *pAdOrg,
	UCHAR *assocresp_ies,
	UINT32 assocresp_ies_len)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	PUCHAR pAssocRespBuf = (PUCHAR)pAd->ApCfg.MBSSID[MAIN_MBSSID].AssocRespExtraIe;

	CFG80211DBG(DBG_LVL_TRACE, ("%s: IE len = %d\n", __func__, assocresp_ies_len));

	if (assocresp_ies_len > sizeof(pAd->ApCfg.MBSSID[MAIN_MBSSID].AssocRespExtraIe)) {
		CFG80211DBG(DBG_LVL_TRACE, ("%s: AssocResp buf size not enough\n", __func__));
		return;
	}

	NdisCopyMemory(pAssocRespBuf, assocresp_ies, assocresp_ies_len);
	pAd->ApCfg.MBSSID[MAIN_MBSSID].AssocRespExtraIeLen = assocresp_ies_len;
}

static int CFG80211_OpsChangeBeacon(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct cfg80211_beacon_data *info)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BEACON bcn;
	UCHAR *beacon_head_buf, *beacon_tail_buf;

	memset(&bcn, 0, sizeof(CMD_RTPRIV_IOCTL_80211_BEACON));
	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));

	if (info->head_len > 0) {
		os_alloc_mem(NULL, &beacon_head_buf, info->head_len);
		NdisCopyMemory(beacon_head_buf, info->head, info->head_len);
	}

	if (info->tail_len > 0) {
		os_alloc_mem(NULL, &beacon_tail_buf, info->tail_len);
		NdisCopyMemory(beacon_tail_buf, info->tail, info->tail_len);
	}

	bcn.beacon_head_len = info->head_len;
	bcn.beacon_tail_len = info->tail_len;
	bcn.beacon_head = beacon_head_buf;
	bcn.beacon_tail = beacon_tail_buf;

	/* Update assoc resp extra ie */
	if (info->assocresp_ies_len && info->assocresp_ies)
		CFG80211_UpdateAssocRespExtraIe(pAd, (UCHAR *)info->assocresp_ies, info->assocresp_ies_len);

	RTMP_DRIVER_80211_BEACON_SET(pAd, &bcn);

	if (beacon_head_buf)
		os_free_mem(beacon_head_buf);

	if (beacon_tail_buf)
		os_free_mem(beacon_tail_buf);

	return 0;
}

static int CFG80211_OpsStopAp(
	struct wiphy *pWiphy,
	struct net_device *netdev)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_ERROR, ("80211> %s ==>\n", __func__));
	RTMP_DRIVER_80211_BEACON_DEL(pAd);
	return 0;
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(3,4,0) */
#endif /* CONFIG_AP_SUPPORT */

static int CFG80211_OpsChangeBss(
	struct wiphy *pWiphy,
	struct net_device *netdev,
	struct bss_parameters *params)
{
	VOID *pAd;
	CMD_RTPRIV_IOCTL_80211_BSS_PARM bssInfo;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	MAC80211_PAD_GET(pAd, pWiphy);
	bssInfo.use_short_preamble = params->use_short_preamble;
	bssInfo.use_short_slot_time = params->use_short_slot_time;
	bssInfo.use_cts_prot = params->use_cts_prot;
	RTMP_DRIVER_80211_CHANGE_BSS_PARM(pAd, &bssInfo);
	return 0;
}

static int CFG80211_OpsStaDel(
	struct wiphy *pWiphy,
	struct net_device *dev,
	UINT8 *pMacAddr)
{
	VOID *pAd;

	MAC80211_PAD_GET(pAd, pWiphy);
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));

	if (pMacAddr ==  NULL)
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, NULL);
	else {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> Delete STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n",
									PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_STA_DEL(pAd, pMacAddr);
	}

	return 0;
}

static int CFG80211_OpsStaAdd(
	struct wiphy *wiphy,
	struct net_device *dev,
	UINT8 *mac,
	struct station_parameters *params)
{
	CFG80211DBG(DBG_LVL_TRACE, ("80211> %s ==>\n", __func__));
	return 0;
}

static int CFG80211_OpsStaChg(
	struct wiphy *pWiphy,
	struct net_device *dev,
	UINT8 *pMacAddr,
	struct station_parameters *params)
{
	void *pAd;
	CFG80211_CB *p80211CB;

	CFG80211DBG(DBG_LVL_TRACE, ("80211> Change STA(%02X:%02X:%02X:%02X:%02X:%02X) ==>\n", PRINT_MAC(pMacAddr)));
	MAC80211_PAD_GET(pAd, pWiphy);
	p80211CB = NULL;
	RTMP_DRIVER_80211_CB_GET(pAd, &p80211CB);

	if ((dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_AP) &&
		(dev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_P2P_GO))
		return -EOPNOTSUPP;

	if (!(params->sta_flags_mask & BIT(NL80211_STA_FLAG_AUTHORIZED))) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %x ==>\n", params->sta_flags_mask));
		return -EOPNOTSUPP;
	}

	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED)) {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> STA(%02X:%02X:%02X:%02X:%02X:%02X) ==> PortSecured\n",
									PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, pMacAddr, 1);
	} else {
		CFG80211DBG(DBG_LVL_TRACE, ("80211> STA(%02X:%02X:%02X:%02X:%02X:%02X) ==> PortNotSecured\n",
									PRINT_MAC(pMacAddr)));
		RTMP_DRIVER_80211_AP_MLME_PORT_SECURED(pAd, pMacAddr, 0);
	}

	return 0;
}




#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
static const struct ieee80211_txrx_stypes
	ralink_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_STATION] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_RESP >> 4),
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4),
	},

};
#endif

#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
static const struct ieee80211_iface_limit ra_p2p_sta_go_limits[] = {
	{
		.max = 3,
		.types = BIT(NL80211_IFTYPE_STATION) |
		BIT(NL80211_IFTYPE_AP),
	},
};

static const struct ieee80211_iface_combination
	ra_iface_combinations_p2p[] = {
	{
#ifdef RT_CFG80211_P2P_MULTI_CHAN_SUPPORT
		.num_different_channels = 2,
#else
		.num_different_channels = 1,
#endif /* RT_CFG80211_P2P_MULTI_CHAN_SUPPORT */
		.max_interfaces = 3,
		/* CFG TODO*/
		/* .beacon_int_infra_match = true, */
		.limits = ra_p2p_sta_go_limits,
		.n_limits = ARRAY_SIZE(ra_p2p_sta_go_limits),
	},
};
#endif /* LINUX_VERSION_CODE: 3.8.0 */

struct cfg80211_ops CFG80211_Ops = {
#ifdef CFG_TDLS_SUPPORT
#if (KERNEL_VERSION(3, 4, 0) < LINUX_VERSION_CODE)
	.tdls_mgmt = CFG80211_OpsTdlsMgmt,
	.tdls_oper = CFG80211_OpsTdlsOper,
#endif
#endif /* CFG_TDLS_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#if (KERNEL_VERSION(3, 4, 0) > LINUX_VERSION_CODE)
	.set_beacon	= CFG80211_OpsSetBeacon,
	.add_beacon	= CFG80211_OpsAddBeacon,
	.del_beacon	= CFG80211_OpsDelBeacon,
#else
	.start_ap	    = CFG80211_OpsStartAp,
	.change_beacon	= CFG80211_OpsChangeBeacon,
	.stop_ap	    = CFG80211_OpsStopAp,
#endif	/* LINUX_VERSION_CODE 3.4 */
#endif /* CONFIG_AP_SUPPORT */
	/* set channel for a given wireless interface */
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	/* CFG_TODO */
	/* .set_monitor_channel = CFG80211_OpsMonitorChannelSet, */
#else
	.set_channel	     = CFG80211_OpsChannelSet,
#endif /* LINUX_VERSION_CODE: 3.6.0 */

	/* change type/configuration of virtual interface */
	.change_virtual_intf		= CFG80211_OpsVirtualInfChg,


#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* request to do a scan */
	/*
	 *	Note: must exist whatever AP or STA mode; Or your kernel will crash
	 *	in v2.6.38.
	 */
	.scan						= CFG80211_OpsScan,
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 31) <= LINUX_VERSION_CODE)
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
#if (KERNEL_VERSION(3, 8, 0) > LINUX_VERSION_CODE)
	/* set the transmit power according to the parameters */
	.set_tx_power				= CFG80211_OpsTxPwrSet,
	/* store the current TX power into the dbm variable */
	.get_tx_power				= CFG80211_OpsTxPwrGet,
#endif /* LINUX_VERSION_CODE: 3.8.0 */
	/* configure WLAN power management */
	.set_power_mgmt				= CFG80211_OpsPwrMgmt,
	/* get station information for the station identified by @mac */
	.get_station				= CFG80211_OpsStaGet,
	/* dump station callback */
	.dump_station				= CFG80211_OpsStaDump,
	/* notify that wiphy parameters have changed */
	.set_wiphy_params			= CFG80211_OpsWiphyParamsSet,
	/* add a key with the given parameters */
	.add_key					= CFG80211_OpsKeyAdd,
	/* get information about the key with the given parameters */
	.get_key					= CFG80211_OpsKeyGet,
	/* remove a key given the @mac_addr */
	.del_key					= CFG80211_OpsKeyDel,
	/* set the default key on an interface */
	.set_default_key			= CFG80211_OpsKeyDefaultSet,
	/* connect to the ESS with the specified parameters */
	.connect					= CFG80211_OpsConnect,
	/* disconnect from the BSS/ESS */
	.disconnect					= CFG80211_OpsDisconnect,
#endif /* LINUX_VERSION_CODE */

#ifdef RFKILL_HW_SUPPORT
	/* polls the hw rfkill line */
	.rfkill_poll				= CFG80211_OpsRFKill,
#endif /* RFKILL_HW_SUPPORT */

#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)
	/* get site survey information */
	/* .dump_survey				= CFG80211_OpsSurveyGet, */
	/* cache a PMKID for a BSSID */
	.set_pmksa					= CFG80211_OpsPmksaSet,
	/* delete a cached PMKID */
	.del_pmksa					= CFG80211_OpsPmksaDel,
	/* flush all cached PMKIDs */
	.flush_pmksa				= CFG80211_OpsPmksaFlush,
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE)
	/*
	 *	Request the driver to remain awake on the specified
	 *	channel for the specified duration to complete an off-channel
	 *	operation (e.g., public action frame exchange).
	 */
	.remain_on_channel			= CFG80211_OpsRemainOnChannel,
	/* cancel an on-going remain-on-channel operation */
	.cancel_remain_on_channel	=  CFG80211_OpsCancelRemainOnChannel,
#if (KERNEL_VERSION(2, 6, 37) > LINUX_VERSION_CODE)
	/* transmit an action frame */
	.action						= NULL,
#else
	.mgmt_tx                    = CFG80211_OpsMgmtTx,
#endif /* LINUX_VERSION_CODE */
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
	.mgmt_tx_cancel_wait       = CFG80211_OpsTxCancelWait,
#endif /* LINUX_VERSION_CODE */


#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
	/* configure connection quality monitor RSSI threshold */
	.set_cqm_rssi_config		= NULL,
#endif /* LINUX_VERSION_CODE */

#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	/* notify driver that a management frame type was registered */
	.mgmt_frame_register		= CFG80211_OpsMgmtFrameRegister,
#endif /* LINUX_VERSION_CODE : 2.6.37 */

#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
	/* set antenna configuration (tx_ant, rx_ant) on the device */
	.set_antenna				= NULL,
	/* get current antenna configuration from device (tx_ant, rx_ant) */
	.get_antenna				= NULL,
#endif /* LINUX_VERSION_CODE */
	.change_bss                             = CFG80211_OpsChangeBss,
	.del_station                            = CFG80211_OpsStaDel,
	.add_station                            = CFG80211_OpsStaAdd,
	.change_station                         = CFG80211_OpsStaChg,
	/* .set_bitrate_mask                       = CFG80211_OpsBitrateSet, */
};

/* =========================== Global Function ============================== */

static INT CFG80211NetdevNotifierEvent(
	struct notifier_block *nb, ULONG state, VOID *ndev)
{
	VOID *pAd;
	struct net_device *pNev = ndev;
	struct wireless_dev *pWdev = pNev->ieee80211_ptr;

	if (!ndev || !pWdev || !pWdev->wiphy)
		return NOTIFY_DONE;

	MAC80211_PAD_GET(pAd, pWdev->wiphy);

	if (!pAd)
		return NOTIFY_DONE;

	switch (state) {
	case NETDEV_UNREGISTER:
		break;

	case NETDEV_GOING_DOWN:
		RTMP_DRIVER_80211_NETDEV_EVENT(pAd, pNev, state);
		break;
	}

	return NOTIFY_DONE;
}

struct notifier_block cfg80211_netdev_notifier = {
	.notifier_call = CFG80211NetdevNotifierEvent,
};

/*
 * ========================================================================
 * Routine Description:
 *	Allocate a wireless device.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pDev			- Generic device interface
 *
 * Return Value:
 *	wireless device
 *
 * Note:
 * ========================================================================
 */
static struct wireless_dev *CFG80211_WdevAlloc(
	IN CFG80211_CB					*pCfg80211_CB,
	IN CFG80211_BAND * pBandInfo,
	IN VOID						*pAd,
	IN struct device				*pDev)
{
	struct wireless_dev *pWdev;
	ULONG *pPriv;
	/*
	 * We're trying to have the following memory layout:
	 *
	 * +------------------------+
	 * | struct wiphy			|
	 * +------------------------+
	 * | pAd pointer			|
	 * +------------------------+
	 */
	pWdev = kzalloc(sizeof(struct wireless_dev), GFP_KERNEL);

	if (pWdev == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Wireless device allocation fail!\n"));
		return NULL;
	} /* End of if */
	
#if defined(PLATFORM_M_STB)	
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	pWdev->use_4addr =true;
#endif /* LINUX_VERSION_CODE 3.0.0 */
#endif

	pWdev->wiphy = wiphy_new(&CFG80211_Ops, sizeof(ULONG *));

	if (pWdev->wiphy == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Wiphy device allocation fail!\n"));
		goto LabelErrWiphyNew;
	} /* End of if */

	/* keep pAd pointer */
	pPriv = (ULONG *)(wiphy_priv(pWdev->wiphy));
	*pPriv = (ULONG)pAd;
	set_wiphy_dev(pWdev->wiphy, pDev);
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* max_scan_ssids means in each scan request, how many ssids can driver handle to send probe-req.
	 *  In current design, we only support 1 ssid at a time. So we should set to 1.
	*/
	/* pWdev->wiphy->max_scan_ssids = pBandInfo->MaxBssTable; */
	pWdev->wiphy->max_scan_ssids = 1;
#endif /* KERNEL_VERSION */
#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)
	/* @NL80211_FEATURE_INACTIVITY_TIMER:
	 * This driver takes care of freeingup
	 * the connected inactive stations in AP mode.
	 */
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* http://www.permalink.gmane.org/gmane.linux.kernel.wireless.general/86454 */
	pWdev->wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;
#endif
	pWdev->wiphy->interface_modes = BIT(NL80211_IFTYPE_AP) | BIT(NL80211_IFTYPE_STATION);
	/* pWdev->wiphy->reg_notifier = CFG80211_RegNotifier; */
	/* init channel information */
	CFG80211_SupBandInit(pCfg80211_CB, pBandInfo, pWdev->wiphy, NULL, NULL);
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
	pWdev->wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
	pWdev->wiphy->max_scan_ie_len = IEEE80211_MAX_DATA_LEN;
#endif
#if (KERNEL_VERSION(2, 6, 33) <= LINUX_VERSION_CODE)
	pWdev->wiphy->max_num_pmkids = 4;
#endif
#if (KERNEL_VERSION(2, 6, 38) <= LINUX_VERSION_CODE)
	pWdev->wiphy->max_remain_on_channel_duration = 5000;
#endif /* KERNEL_VERSION */
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	pWdev->wiphy->mgmt_stypes = ralink_mgmt_stypes;
#endif
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
	pWdev->wiphy->cipher_suites = CipherSuites;
	pWdev->wiphy->n_cipher_suites = ARRAY_SIZE(CipherSuites);
#endif /* LINUX_VERSION_CODE */
#if (KERNEL_VERSION(3, 2, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
#endif /* LINUX_VERSION_CODE: 3.2.0 */
#if (KERNEL_VERSION(3, 3, 0) <= LINUX_VERSION_CODE)
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 018-cfg80211-internal-ap-mlme.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
	/*what if you get compile error for below flag, please add the patch into your kernel*/
	/* 008-cfg80211-offchan-flags.patch */
	pWdev->wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_STRICT_REGULATORY; */
#endif /* LINUX_VERSION_CODE: 3.3.0 */

#if defined(PLATFORM_M_STB)	
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
#endif /* LINUX_VERSION_CODE 3.0.0 */
#endif

	/* Driver Report Support TDLS to supplicant */
#ifdef CFG_TDLS_SUPPORT
	pWdev->wiphy->flags |= WIPHY_FLAG_SUPPORTS_TDLS;
	pWdev->wiphy->flags |= WIPHY_FLAG_TDLS_EXTERNAL_SETUP;
#endif /* CFG_TDLS_SUPPORT */
	/* CFG_TODO */
	/* pWdev->wiphy->flags |= WIPHY_FLAG_IBSS_RSN; */
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	pWdev->wiphy->iface_combinations = ra_iface_combinations_p2p;
	pWdev->wiphy->n_iface_combinations = ARRAY_SIZE(ra_iface_combinations_p2p);
#endif

	if (wiphy_register(pWdev->wiphy) < 0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Register wiphy device fail!\n"));
		goto LabelErrReg;
	}

	return pWdev;
LabelErrReg:
	wiphy_free(pWdev->wiphy);
LabelErrWiphyNew:
	os_free_mem(pWdev);
	return NULL;
} /* End of CFG80211_WdevAlloc */


/*
 * ========================================================================
 * Routine Description:
 *	Register MAC80211 Module.
 *
 * Arguments:
 *	pAdCB			- WLAN control block pointer
 *	pDev			- Generic device interface
 *	pNetDev			- Network device
 *
 * Return Value:
 *	NONE
 *
 * Note:
 *	pDev != pNetDev
 *	#define SET_NETDEV_DEV(net, pdev)	((net)->dev.parent = (pdev))
 *
 *	Can not use pNetDev to replace pDev; Or kernel panic.
 * ========================================================================
 */
BOOLEAN CFG80211_Register(
	IN VOID						*pAd,
	IN struct device			*pDev,
	IN struct net_device		*pNetDev)
{
	CFG80211_CB *pCfg80211_CB = NULL;
	CFG80211_BAND BandInfo;
	INT err = 0;
	/* allocate Main Device Info structure */
	os_alloc_mem(NULL, (UCHAR **)&pCfg80211_CB, sizeof(CFG80211_CB));

	if (pCfg80211_CB == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Allocate MAC80211 CB fail!\n"));
		return FALSE;
	}

	/* allocate wireless device */
	RTMP_DRIVER_80211_BANDINFO_GET(pAd, &BandInfo);
	pCfg80211_CB->pCfg80211_Wdev = CFG80211_WdevAlloc(pCfg80211_CB, &BandInfo, pAd, pDev);

	if (pCfg80211_CB->pCfg80211_Wdev == NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Allocate Wdev fail!\n"));
		os_free_mem(pCfg80211_CB);
		return FALSE;
	}

	/* bind wireless device with net device */
#ifdef CONFIG_AP_SUPPORT
	/* default we are AP mode */
	pCfg80211_CB->pCfg80211_Wdev->iftype = NL80211_IFTYPE_AP;
#endif /* CONFIG_AP_SUPPORT */
	pNetDev->ieee80211_ptr = pCfg80211_CB->pCfg80211_Wdev;
	SET_NETDEV_DEV(pNetDev, wiphy_dev(pCfg80211_CB->pCfg80211_Wdev->wiphy));
	pCfg80211_CB->pCfg80211_Wdev->netdev = pNetDev;
#ifdef RFKILL_HW_SUPPORT
	wiphy_rfkill_start_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */
	RTMP_DRIVER_80211_CB_SET(pAd, pCfg80211_CB);
	RTMP_DRIVER_80211_RESET(pAd);
	RTMP_DRIVER_80211_SCAN_STATUS_LOCK_INIT(pAd, TRUE);

	/* TODO */
	/* err = register_netdevice_notifier(&cfg80211_netdev_notifier);	//CFG TODO */
	if (err)
		CFG80211DBG(DBG_LVL_ERROR, ("80211> Failed to register notifierl %d\n", err));

	CFG80211DBG(DBG_LVL_ERROR, ("80211> CFG80211_Register\n"));
	return TRUE;
} /* End of CFG80211_Register */




/* =========================== Local Function =============================== */


#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX_VERSION_CODE */

/* End of crda.c */
