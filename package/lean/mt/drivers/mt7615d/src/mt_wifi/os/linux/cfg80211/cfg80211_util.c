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
 *	Abstract:
 *
 *	All related CFG80211 function body.
 *
 *	History:
 *
 ***************************************************************************/
#ifdef RT_CFG80211_SUPPORT
#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"

#include "rt_config.h"
#include "chlist.h"

/* all available channels */

static const UCHAR Cfg80211_Chan[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	/* 802.11 UNI / HyperLan 2 */
	36, 40, 44, 48, 52, 56, 60, 64,

	/* 802.11 HyperLan 2 */
	100, 104, 108, 112, 116, 120, 124, 128, 132, 136,

	/* 802.11 UNII */
	140, 144, 149, 153, 157, 161, 165, 169, 173,

	/* Japan */
	184, 188, 192, 196, 208, 212, 216,
};

UCHAR Cfg80211_RadarChan[] = {
	52, 54, 56, 60, 62, 64, 100, 104, 144,
};

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
		.bitrate = 10,
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

static const UINT32 CipherSuites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef DOT11W_PMF_SUPPORT
	WLAN_CIPHER_SUITE_AES_CMAC,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
#endif
#endif /*DOT11W_PMF_SUPPORT*/
	WLAN_CIPHER_SUITE_GCMP,
#if (KERNEL_VERSION(4, 0, 0) <= LINUX_VERSION_CODE)
	WLAN_CIPHER_SUITE_CCMP_256,
#ifdef HOSTAPD_SUITEB_SUPPORT
	WLAN_CIPHER_SUITE_GCMP_256,
#endif
#endif
};

static BOOLEAN IsRadarChannel(UCHAR ch)
{
	UINT idx = 0;

	for (idx = 0; idx < sizeof(Cfg80211_RadarChan); idx++) {
		if (Cfg80211_RadarChan[idx] == ch)
			return TRUE;
	}

	return FALSE;
}

VOID CFG80211OS_PutBss(
	IN VOID *pWiphyOrg,
	IN VOID *pCfg80211Bss)
{
	struct cfg80211_bss *bss = (struct cfg80211_bss *)pCfg80211Bss;
#if (KERNEL_VERSION(3, 9, 0) <= LINUX_VERSION_CODE)
	struct wiphy *pWiphy = (struct wiphy *) pWiphyOrg;

	cfg80211_put_bss(pWiphy, bss);
#else
	cfg80211_put_bss(bss);
#endif /* LINUX_VERSION_CODE: 3.9.0 */
}

/*
 * ========================================================================
 * Routine Description:
 *	UnRegister MAC80211 Module.
 *
 * Arguments:
 *	pCB				- CFG80211 control block pointer
 *	pNetDev			- Network device
 *
 * Return Value:
 *	NONE
 *
 * ========================================================================
 */
VOID CFG80211OS_UnRegister(VOID *pCB, VOID *pNetDevOrg)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct net_device *pNetDev = (struct net_device *)pNetDevOrg;

	/* unregister */
	if (pCfg80211_CB->pCfg80211_Wdev != NULL) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> unregister/free wireless device\n"));
		/*
		 *	Must unregister, or you will suffer problem when you change
		 *	regulatory domain by using iw.
		 */
#ifdef RFKILL_HW_SUPPORT
		wiphy_rfkill_stop_polling(pCfg80211_CB->pCfg80211_Wdev->wiphy);
#endif /* RFKILL_HW_SUPPORT */
		wiphy_unregister(pCfg80211_CB->pCfg80211_Wdev->wiphy);
		wiphy_free(pCfg80211_CB->pCfg80211_Wdev->wiphy);
		kfree(pCfg80211_CB->pCfg80211_Wdev);

		if (pCfg80211_CB->pCfg80211_Channels != NULL)
			kfree(pCfg80211_CB->pCfg80211_Channels);

		if (pCfg80211_CB->pCfg80211_Rates != NULL)
			kfree(pCfg80211_CB->pCfg80211_Rates);

		pCfg80211_CB->pCfg80211_Wdev = NULL;
		pCfg80211_CB->pCfg80211_Channels = NULL;
		pCfg80211_CB->pCfg80211_Rates = NULL;
		/* must reset to NULL; or kernel will panic in unregister_netdev */
		pNetDev->ieee80211_ptr = NULL;
		SET_NETDEV_DEV(pNetDev, NULL);
	}

	os_free_mem(pCfg80211_CB);
}


/*
 * ========================================================================
 * Routine Description:
 *	Initialize wireless channel in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	pWiphy			- WLAN PHY interface
 *	pChannels		- Current channel info
 *	pRates			- Current rate info
 *
 * Return Value:
 *	TRUE			- init successfully
 *	FALSE			- init fail
 *
 * Note:
 *	TX Power related:
 *
 *	1. Suppose we can send power to 15dBm in the board.
 *	2. A value 0x0 ~ 0x1F for a channel. We will adjust it based on 15dBm/
 *		54Mbps. So if value == 0x07, the TX power of 54Mbps is 15dBm and
 *		the value is 0x07 in the EEPROM.
 *	3. Based on TX power value of 54Mbps/channel, adjust another value
 *		0x0 ~ 0xF for other data rate. (-6dBm ~ +6dBm)
 *
 *	Other related factors:
 *	1. TX power percentage from UI/users;
 *	2. Maximum TX power limitation in the regulatory domain.
 * ========================================================================
 */
BOOLEAN CFG80211_SupBandInit(
	IN VOID *pCB,
	IN CFG80211_BAND * pDriverBandInfo,
	IN VOID *pWiphyOrg,
	IN VOID *pChannelsOrg,
	IN VOID *pRatesOrg)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;
	struct ieee80211_channel *pChannels = (struct ieee80211_channel *)pChannelsOrg;
	struct ieee80211_rate *pRates = (struct ieee80211_rate *)pRatesOrg;
	struct ieee80211_supported_band *pBand;
	UINT32 NumOfChan, NumOfRate;
	UINT32 IdLoop;
	UINT32 CurTxPower;

	/* sanity check */
	if (pDriverBandInfo->RFICType == 0)
		pDriverBandInfo->RFICType = RFIC_24GHZ | RFIC_5GHZ;

	/* 1. Calcute the Channel Number */
	if (pDriverBandInfo->RFICType & RFIC_5GHZ)
		NumOfChan = CFG80211_NUM_OF_CHAN_2GHZ + CFG80211_NUM_OF_CHAN_5GHZ;
	else
		NumOfChan = CFG80211_NUM_OF_CHAN_2GHZ;

	/* 2. Calcute the Rate Number */
		NumOfRate = 4 + 8;
#ifdef DBDC_MODE
	NumOfRate = 4 + 8;
#endif

	CFG80211DBG(DBG_LVL_ERROR, ("80211> RFICType= %d, NumOfChan= %d\n", pDriverBandInfo->RFICType, NumOfChan));
	CFG80211DBG(DBG_LVL_ERROR, ("80211> Number of rate = %d\n", NumOfRate));

	/* 3. Allocate the Channel instance */
	if (pChannels == NULL && NumOfChan) {
		pChannels = kcalloc(NumOfChan, sizeof(*pChannels), GFP_KERNEL);

		if (!pChannels) {
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> ieee80211_channel allocation fail!\n"));
			return FALSE;
		}
	}

	/* 4. Allocate the Rate instance */
	if (pRates == NULL && NumOfRate) {
		pRates = kcalloc(NumOfRate, sizeof(*pRates), GFP_KERNEL);

		if (!pRates) {
			os_free_mem(pChannels);
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> ieee80211_rate allocation fail!\n"));
			return FALSE;
		}
	}

	/* get TX power */
	CurTxPower = 0; /* unknown */
	CFG80211DBG(DBG_LVL_ERROR, ("80211> CurTxPower = %d dBm\n", CurTxPower));

	/* 5. init channel */
	for (IdLoop = 0; IdLoop < NumOfChan; IdLoop++) {
#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)

		if (IdLoop >= 14) {
			pChannels[IdLoop].band = IEEE80211_BAND_5GHZ;
			pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop], IEEE80211_BAND_5GHZ);
		} else {
			pChannels[IdLoop].band = IEEE80211_BAND_2GHZ;
			pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop], IEEE80211_BAND_2GHZ);
		}

#else
		pChannels[IdLoop].center_freq = ieee80211_channel_to_frequency(Cfg80211_Chan[IdLoop]);
#endif
		pChannels[IdLoop].hw_value = IdLoop;

		if (IdLoop < CFG80211_NUM_OF_CHAN_2GHZ)
			pChannels[IdLoop].max_power = CurTxPower;
		else
			pChannels[IdLoop].max_power = CurTxPower;

		pChannels[IdLoop].max_antenna_gain = 0xff;
		pChannels[IdLoop].flags = 0;

		/* if (RadarChannelCheck(pAd, Cfg80211_Chan[IdLoop])) */
		if (IsRadarChannel(Cfg80211_Chan[IdLoop])) {
			CFG80211DBG(DBG_LVL_TRACE, ("====> Radar Channel %d\n", Cfg80211_Chan[IdLoop]));
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
			pChannels[IdLoop].flags |= (IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_IR);
#else
			pChannels[IdLoop].flags |= (IEEE80211_CHAN_RADAR | IEEE80211_CHAN_PASSIVE_SCAN);
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
			pChannels[IdLoop].dfs_state = NL80211_DFS_AVAILABLE;
#endif
		}

		/*		CFG_TODO:
		 *		pChannels[IdLoop].flags
		 *		enum ieee80211_channel_flags {
		 *			IEEE80211_CHAN_DISABLED		= 1<<0,
		 *			IEEE80211_CHAN_PASSIVE_SCAN	= 1<<1,
		 *			IEEE80211_CHAN_NO_IBSS		= 1<<2,
		 *			IEEE80211_CHAN_RADAR		= 1<<3,
		 *			IEEE80211_CHAN_NO_HT40PLUS	= 1<<4,
		 *			IEEE80211_CHAN_NO_HT40MINUS	= 1<<5,
		 *		};
		 */
	}

	/* 6. init rate */
	for (IdLoop = 0; IdLoop < NumOfRate; IdLoop++)
		memcpy(&pRates[IdLoop], &Cfg80211_SupRate[IdLoop], sizeof(*pRates));

	/*		CFG_TODO:
	 *		enum ieee80211_rate_flags {
	 *			IEEE80211_RATE_SHORT_PREAMBLE	= 1<<0,
	 *			IEEE80211_RATE_MANDATORY_A	= 1<<1,
	 *			IEEE80211_RATE_MANDATORY_B	= 1<<2,
	 *			IEEE80211_RATE_MANDATORY_G	= 1<<3,
	 *			IEEE80211_RATE_ERP_G		= 1<<4,
	 *		};
	 */
	/* 7. Fill the Band 2.4GHz */
	pBand = &pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_2GHZ];

	if (pDriverBandInfo->RFICType & RFIC_24GHZ) {
		pBand->n_channels = CFG80211_NUM_OF_CHAN_2GHZ;
#ifdef DBDC_MODE
#endif
				pBand->n_bitrates = NumOfRate;
				pBand->channels = pChannels;
				pBand->bitrates = pRates;
#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
							IEEE80211_HT_CAP_SM_PS |
							IEEE80211_HT_CAP_SGI_40 |
							IEEE80211_HT_CAP_SGI_20 |
							IEEE80211_HT_CAP_TX_STBC |
						    IEEE80211_HT_CAP_RX_STBC |
							IEEE80211_HT_CAP_DSSSCCK40;
		pBand->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = pDriverBandInfo->MpduDensity; /* YF_TODO */
		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));
		CFG80211DBG(DBG_LVL_ERROR, ("80211> TxStream = %d\n", pDriverBandInfo->TxStream));

		switch (pDriverBandInfo->TxStream) {
		case 1:
		default:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			break;

		case 2:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(300);
			break;

		case 3:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			break;

		case 4:
		    pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_mask[3] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(600);
			break;
		}

		pBand->ht_cap.mcs.rx_mask[4] = 0x01; /* 40MHz*/
		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
#endif /* DOT11_N_SUPPORT */
		pWiphy->bands[IEEE80211_BAND_2GHZ] = pBand;
	} else {
		pWiphy->bands[IEEE80211_BAND_2GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
	}

	/* 8. Fill the Band 5GHz */
	pBand = &pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_5GHZ];

	if (pDriverBandInfo->RFICType & RFIC_5GHZ) {
		pBand->n_channels = CFG80211_NUM_OF_CHAN_5GHZ;
		pBand->n_bitrates = NumOfRate - 4;	/*Disable 11B rate*/
		pBand->channels = &pChannels[CFG80211_NUM_OF_CHAN_2GHZ];
		pBand->bitrates = &pRates[4];
#ifdef DOT11_N_SUPPORT
		/* for HT, assign pBand->ht_cap */
		pBand->ht_cap.ht_supported = true;
		pBand->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 |
							IEEE80211_HT_CAP_SM_PS |
							IEEE80211_HT_CAP_SGI_40 |
							IEEE80211_HT_CAP_SGI_20 |
							IEEE80211_HT_CAP_TX_STBC |
						    IEEE80211_HT_CAP_RX_STBC |
							IEEE80211_HT_CAP_DSSSCCK40;
		pBand->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K; /* 2 ^ 16 */
		pBand->ht_cap.ampdu_density = pDriverBandInfo->MpduDensity; /* YF_TODO */
		memset(&pBand->ht_cap.mcs, 0, sizeof(pBand->ht_cap.mcs));

		switch (pDriverBandInfo->RxStream) {
		case 1:
		default:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			break;

		case 2:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(300);
			break;

		case 3:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			break;

		case 4:
			pBand->ht_cap.mcs.rx_mask[0] = 0xff;
			pBand->ht_cap.mcs.rx_mask[1] = 0xff;
			pBand->ht_cap.mcs.rx_mask[2] = 0xff;
			pBand->ht_cap.mcs.rx_mask[3] = 0xff;
			pBand->ht_cap.mcs.rx_highest = cpu_to_le16(600);
			break;
		}

		pBand->ht_cap.mcs.rx_mask[4] = 0x01; /* 40MHz*/
		pBand->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		pBand->vht_cap.vht_supported = true;
		pBand->vht_cap.cap = IEEE80211_VHT_CAP_RXLDPC  |
					IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK |
					IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
							 IEEE80211_VHT_CAP_SHORT_GI_80  |
							 IEEE80211_VHT_CAP_TXSTBC |
							 IEEE80211_STA_RX_BW_80 |
							 IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK |
					IEEE80211_VHT_CAP_RXSTBC_MASK;
#endif /* DOT11_VHT_AC */
		pWiphy->bands[IEEE80211_BAND_5GHZ] = pBand;
	} else {
		pWiphy->bands[IEEE80211_BAND_5GHZ] = NULL;
		pBand->channels = NULL;
		pBand->bitrates = NULL;
	}

	/* 9. re-assign to mainDevice info */
	pCfg80211_CB->pCfg80211_Channels = pChannels;
	pCfg80211_CB->pCfg80211_Rates = pRates;
	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Re-Initialize wireless channel/PHY in 2.4GHZ and 5GHZ.
 *
 * Arguments:
 *	pCB				- CFG80211 control block pointer
 *	pBandInfo		- Band information
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
BOOLEAN CFG80211OS_SupBandReInit(
	IN VOID *pCB,
	IN CFG80211_BAND * pBandInfo)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy;

	if ((pCfg80211_CB == NULL) || (pCfg80211_CB->pCfg80211_Wdev == NULL))
		return FALSE;

	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;

	if (pWiphy != NULL) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> re-init bands...\n"));
		/* re-init bands */
		CFG80211_SupBandInit(pCfg80211_CB, pBandInfo, pWiphy,
							 pCfg80211_CB->pCfg80211_Channels,
							 pCfg80211_CB->pCfg80211_Rates);
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
		/* re-init PHY */
		pWiphy->rts_threshold = pBandInfo->RtsThreshold;
		pWiphy->frag_threshold = pBandInfo->FragmentThreshold;
		pWiphy->retry_short = pBandInfo->RetryMaxCnt & 0xff;
		pWiphy->retry_long = (pBandInfo->RetryMaxCnt & 0xff00) >> 8;
#endif /* LINUX_VERSION_CODE */
		return TRUE;
	}

	return FALSE;
}


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
VOID CFG80211OS_RegHint(
	IN VOID *pCB,
	IN UCHAR *pCountryIe,
	IN ULONG CountryIeLen)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;

	CFG80211DBG(DBG_LVL_ERROR,
				("crda> regulatory domain hint: %c%c\n",
				 pCountryIe[0], pCountryIe[1]));

	if ((pCfg80211_CB->pCfg80211_Wdev == NULL) || (pCountryIe == NULL)) {
		CFG80211DBG(DBG_LVL_ERROR, ("crda> regulatory domain hint not support!\n"));
		return;
	}

	/* hints a country IE as a regulatory domain "without" channel/power info. */
	regulatory_hint(pCfg80211_CB->pCfg80211_Wdev->wiphy, (const char *)pCountryIe);
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
VOID CFG80211OS_RegHint11D(
	IN VOID *pCB,
	IN UCHAR *pCountryIe,
	IN ULONG CountryIeLen)
{
	/* no regulatory_hint_11d() in 2.6.32 */
#if (KERNEL_VERSION(2, 6, 32) > LINUX_VERSION_CODE)
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;

	if ((pCfg80211_CB->pCfg80211_Wdev == NULL) || (pCountryIe == NULL)) {
		CFG80211DBG(DBG_LVL_ERROR, ("crda> regulatory domain hint not support!\n"));
		return;
	}

	CFG80211DBG(DBG_LVL_ERROR,
				("crda> regulatory domain hint: %c%c\n",
				 pCountryIe[0], pCountryIe[1]));
	/*
	 *	hints a country IE as a regulatory domain "with" channel/power info.
	 *	but if you use regulatory_hint(), it only hint "regulatory domain".
	 */
	regulatory_hint_11d(pCfg80211_CB->pCfg80211_Wdev->wiphy, pCountryIe, CountryIeLen);
#endif /* LINUX_VERSION_CODE */
}


BOOLEAN CFG80211OS_BandInfoGet(
	IN VOID *pCB,
	IN VOID *pWiphyOrg,
	OUT VOID **ppBand24,
	OUT VOID **ppBand5)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL)
		return FALSE;

	*ppBand24 = pWiphy->bands[IEEE80211_BAND_2GHZ];
	*ppBand5 = pWiphy->bands[IEEE80211_BAND_5GHZ];
	return TRUE;
}


UINT32 CFG80211OS_ChanNumGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN UINT32					IdBand)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL)
		return 0;

	if (pWiphy->bands[IdBand] != NULL)
		return pWiphy->bands[IdBand]->n_channels;

	return 0;
}


BOOLEAN CFG80211OS_ChanInfoGet(
	IN VOID						*pCB,
	IN VOID						*pWiphyOrg,
	IN UINT32					IdBand,
	IN UINT32					IdChan,
	OUT UINT32					*pChanId,
	OUT UINT32					*pPower,
	OUT BOOLEAN					*pFlgIsRadar)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct wiphy *pWiphy = (struct wiphy *)pWiphyOrg;
	struct ieee80211_supported_band *pSband;
	struct ieee80211_channel *pChan;

	if (pWiphy == NULL) {
		if ((pCfg80211_CB != NULL) && (pCfg80211_CB->pCfg80211_Wdev != NULL))
			pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	}

	if (pWiphy == NULL)
		return FALSE;

	pSband = pWiphy->bands[IdBand];
	pChan = &pSband->channels[IdChan];
	*pChanId = ieee80211_frequency_to_channel(pChan->center_freq);

	if (pChan->flags & IEEE80211_CHAN_DISABLED) {
		CFG80211DBG(DBG_LVL_ERROR, ("Chan %03d (frq %d):\tnot allowed!\n",
									(*pChanId), pChan->center_freq));
		return FALSE;
	}

	*pPower = pChan->max_power;

	if (pChan->flags & IEEE80211_CHAN_RADAR)
		*pFlgIsRadar = TRUE;
	else
		*pFlgIsRadar = FALSE;

	return TRUE;
}


/*
 * ========================================================================
 * Routine Description:
 *	Initialize a channel information used in scan inform.
 *
 * Arguments:
 *
 * Return Value:
 *	TRUE		- Successful
 *	FALSE		- Fail
 *
 * ========================================================================
 */
BOOLEAN CFG80211OS_ChanInfoInit(
	IN VOID						*pCB,
	IN UINT32					InfoIndex,
	IN UCHAR					ChanId,
	IN UCHAR					MaxTxPwr,
	IN BOOLEAN					FlgIsNMode,
	IN BOOLEAN					FlgIsBW20M)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct ieee80211_channel *pChan;

	if (InfoIndex >= MAX_NUM_OF_CHANNELS)
		return FALSE;

	pChan = (struct ieee80211_channel *) &(pCfg80211_CB->ChanInfo[InfoIndex]);
	memset(pChan, 0, sizeof(*pChan));

	if (ChanId > 14)
		pChan->band = IEEE80211_BAND_5GHZ;
	else
		pChan->band = IEEE80211_BAND_2GHZ;

#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)
	pChan->center_freq = ieee80211_channel_to_frequency(ChanId, pChan->band);
#else
	pChan->center_freq = ieee80211_channel_to_frequency(ChanId);
#endif
#if (KERNEL_VERSION(2, 6, 32) > LINUX_VERSION_CODE)

	if (FlgIsNMode == TRUE) {
		if (FlgIsBW20M == TRUE)
			pChan->max_bandwidth = 20; /* 20MHz */
		else
			pChan->max_bandwidth = 40; /* 40MHz */
	} else
		pChan->max_bandwidth = 5; /* 5MHz for non-HT device */

#endif /* LINUX_VERSION_CODE */
	/* no use currently in 2.6.30 */
	/*	if (ieee80211_is_beacon(((struct ieee80211_mgmt *)pFrame)->frame_control)) */
	/*		pChan->beacon_found = 1; */
	return TRUE;
}


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
VOID CFG80211OS_Scaning(
	IN VOID						*pCB,
	IN UINT32					ChanId,
	IN UCHAR					*pFrame,
	IN UINT32					FrameLen,
	IN INT32					RSSI,
	IN BOOLEAN					FlgIsNMode,
	IN UINT8					BW)
{
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;
	struct ieee80211_supported_band *pBand;
	UINT32 IdChan;
	UINT32 CenFreq;
	UINT CurBand;
	struct wiphy *pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	struct cfg80211_bss *bss = NULL;
	struct ieee80211_mgmt *mgmt;

	mgmt = (struct ieee80211_mgmt *) pFrame;

	if (ChanId == 0)
		ChanId = 1;

	/* get channel information */
#if (KERNEL_VERSION(2, 6, 39) <= LINUX_VERSION_CODE)

	if (ChanId > 14)
		CenFreq = ieee80211_channel_to_frequency(ChanId, IEEE80211_BAND_5GHZ);
	else
		CenFreq = ieee80211_channel_to_frequency(ChanId, IEEE80211_BAND_2GHZ);

#else
	CenFreq = ieee80211_channel_to_frequency(ChanId);
#endif

	if (ChanId > 14)
		CurBand = IEEE80211_BAND_5GHZ;
	else
		CurBand = IEEE80211_BAND_2GHZ;

	pBand = &pCfg80211_CB->Cfg80211_bands[CurBand];

	for (IdChan = 0; IdChan < pBand->n_channels; IdChan++) {
		if (pBand->channels[IdChan].center_freq == CenFreq)
			break;
	}

	if (IdChan >= pBand->n_channels) {
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("80211> Can not find any chan info! ==> %d[%d],[%d]\n",
				 ChanId, CenFreq, pBand->n_channels));
		return;
	}

	if (pWiphy->signal_type == CFG80211_SIGNAL_TYPE_MBM) {
		/* CFG80211_SIGNAL_TYPE_MBM: signal strength in mBm (100*dBm) */
		RSSI = RSSI * 100;
	}

	if (!mgmt->u.probe_resp.timestamp) {
		struct timespec64 tv;

		ktime_get_real_ts64(&tv);
		mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_nsec;
	}

	/* inform 80211 a scan is got */
	/* we can use cfg80211_inform_bss in 2.6.31, it is easy more than the one */
	/* in cfg80211_inform_bss_frame(), it will memcpy pFrame but pChan */
	bss = cfg80211_inform_bss_frame(pWiphy, &pBand->channels[IdChan],
									mgmt,	FrameLen,
									RSSI,	GFP_ATOMIC);

	if (unlikely(!bss)) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> bss inform fail ==> %d\n", IdChan));
		return;
	}

	CFG80211OS_PutBss(pWiphy, bss);
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_CFG80211_SUPPORT) */
#endif /* LINUX_VERSION_CODE */
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
VOID CFG80211OS_ScanEnd(
	IN VOID *pCB,
	IN BOOLEAN FlgIsAborted)
{
#if (KERNEL_VERSION(2, 6, 30) <= LINUX_VERSION_CODE)
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;

	NdisAcquireSpinLock(&pCfg80211_CB->scan_notify_lock);

	if (pCfg80211_CB->pCfg80211_ScanReq) {
		CFG80211DBG(DBG_LVL_ERROR, ("80211> cfg80211_scan_done\n"));
		cfg80211_scan_done(pCfg80211_CB->pCfg80211_ScanReq, FlgIsAborted);
		pCfg80211_CB->pCfg80211_ScanReq = NULL;
	} else
		CFG80211DBG(DBG_LVL_ERROR, ("80211> cfg80211_scan_done ==> NULL\n"));

	NdisReleaseSpinLock(&pCfg80211_CB->scan_notify_lock);
#endif /* LINUX_VERSION_CODE */
}


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
void CFG80211OS_ConnectResultInform(
	IN VOID *pCB,
	IN UCHAR *pBSSID,
	IN UCHAR *pReqIe,
	IN UINT32 ReqIeLen,
	IN UCHAR *pRspIe,
	IN UINT32 RspIeLen,
	IN UCHAR FlgIsSuccess)
{
#if (KERNEL_VERSION(2, 6, 32) <= LINUX_VERSION_CODE)
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *)pCB;

	if ((pCfg80211_CB->pCfg80211_Wdev->netdev == NULL) || (pBSSID == NULL))
		return;

	if (FlgIsSuccess) {
		cfg80211_connect_result(pCfg80211_CB->pCfg80211_Wdev->netdev,
								pBSSID,
								pReqIe,
								ReqIeLen,
								pRspIe,
								RspIeLen,
								WLAN_STATUS_SUCCESS,
								GFP_KERNEL);
	} else {
		cfg80211_connect_result(pCfg80211_CB->pCfg80211_Wdev->netdev,
								pBSSID,
								NULL, 0, NULL, 0,
								WLAN_STATUS_UNSPECIFIED_FAILURE,
								GFP_KERNEL);
	}

#endif /* LINUX_VERSION_CODE */
}

/* CFG_TODO: should be merge totoger */
void CFG80211OS_P2pClientConnectResultInform(
	IN PNET_DEV				pNetDev,
	IN UCHAR					*pBSSID,
	IN UCHAR					*pReqIe,
	IN UINT32					ReqIeLen,
	IN UCHAR					*pRspIe,
	IN UINT32					RspIeLen,
	IN UCHAR					FlgIsSuccess)
{
	if ((pNetDev == NULL) || (pBSSID == NULL))
		return;

	if (FlgIsSuccess) {
		CFG80211DBG(DBG_LVL_ERROR, ("APCLI: ReqIeLen %d, RspIeLen, %d\n", ReqIeLen, RspIeLen));
		hex_dump("APCLI Req:", pReqIe, ReqIeLen);
		hex_dump("APCLI Rsp:", pRspIe, RspIeLen);
		cfg80211_connect_result(pNetDev,
								pBSSID,
								pReqIe,
								ReqIeLen,
								pRspIe,
								RspIeLen,
								WLAN_STATUS_SUCCESS,
								GFP_KERNEL);
	} else {
		cfg80211_connect_result(pNetDev,
								pBSSID,
								NULL, 0, NULL, 0,
								WLAN_STATUS_UNSPECIFIED_FAILURE,
								GFP_KERNEL);
	}
}

BOOLEAN CFG80211OS_RxMgmt(IN PNET_DEV pNetDev, IN INT32 freq, IN PUCHAR frame, IN UINT32 len)
{
	/* Sanity Check */
	if (pNetDev == NULL) {
		CFG80211DBG(DBG_LVL_ERROR, ("%s: pNetDev == NULL\n", __func__));
		return FALSE;
	}

#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	return cfg80211_rx_mgmt(pNetDev->ieee80211_ptr,
							freq,
							0,       /* CFG_TODO return 0 in dbm */
							frame,
							len,
							GFP_ATOMIC);
#else
#if (KERNEL_VERSION(3, 4, 0) <= LINUX_VERSION_CODE)
	return cfg80211_rx_mgmt(pNetDev,
							freq,
							0,       /* CFG_TODO return 0 in dbm */
							frame,
							len,
							GFP_ATOMIC);
#else
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	return cfg80211_rx_mgmt(pNetDev, freq, frame, len, GFP_ATOMIC);
#else
#if (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE)
	return cfg80211_rx_action(pNetDev, freq, frame, len, GFP_ATOMIC);
#else
	return FALSE;
#endif /* LINUX_VERSION_CODE: 2.6.34*/
#endif /* LINUX_VERSION_CODE: 2.6.37*/
#endif /* LINUX_VERSION_CODE: 3.4.0*/
#endif /* LINUX_VERSION_CODE: 3.6.0 */
}

VOID CFG80211OS_TxStatus(IN PNET_DEV pNetDev, IN INT32 cookie, IN PUCHAR frame, IN UINT32 len, IN BOOLEAN ack)
{
#if (KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE)
	return cfg80211_mgmt_tx_status(pNetDev->ieee80211_ptr, cookie, frame, len, ack, GFP_ATOMIC);
#else
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	return cfg80211_mgmt_tx_status(pNetDev, cookie, frame, len, ack, GFP_ATOMIC);
#else
#if (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE)
	return cfg80211_action_tx_status(pNetDev, cookie, frame, len, ack, GFP_ATOMIC);
#else
	return FALSE;
#endif /* LINUX_VERSION_CODE: 2.6.34*/
#endif /* LINUX_VERSION_CODE: 2.6.37*/
#endif /* LINUX_VERSION_CODE: 3.6.0 */
}

VOID CFG80211OS_NewSta(IN PNET_DEV pNetDev, IN const PUCHAR mac_addr, IN const PUCHAR assoc_frame, IN UINT32 assoc_len, IN BOOLEAN isReassoc)
{
	struct station_info sinfo;
	struct ieee80211_mgmt *mgmt;

	os_zero_mem(&sinfo, sizeof(sinfo));


/* If get error here, be sure patch the cfg80211_new_sta.patch into kernel. */
	if (pNetDev->ieee80211_ptr->iftype != RT_CMD_80211_IFTYPE_ADHOC) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		/*no option available in latest implementation, to fill this var*/
#else
		sinfo.filled = STATION_INFO_ASSOC_REQ_IES;
#endif
		mgmt = (struct ieee80211_mgmt *) assoc_frame;
		if (isReassoc) {
			sinfo.assoc_req_ies_len = assoc_len - 24 - 10;
			sinfo.assoc_req_ies = mgmt->u.reassoc_req.variable;
		} else {
			sinfo.assoc_req_ies_len = assoc_len - 24 - 4;
			sinfo.assoc_req_ies = mgmt->u.assoc_req.variable;
		}
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
	return cfg80211_new_sta(pNetDev, mac_addr, &sinfo, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE: 2.6.34 */

}



VOID CFG80211OS_DelSta(IN PNET_DEV pNetDev, IN const PUCHAR mac_addr)
{
#if (KERNEL_VERSION(3, 0, 0) <= LINUX_VERSION_CODE)
	return cfg80211_del_sta(pNetDev, mac_addr, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE: 3.0.0 */
}


VOID CFG80211OS_MICFailReport(PNET_DEV pNetDev, const PUCHAR src_addr, BOOLEAN unicast, INT key_id, const PUCHAR tsc)
{
	cfg80211_michael_mic_failure(pNetDev, src_addr,
								 (unicast ? NL80211_KEYTYPE_PAIRWISE : NL80211_KEYTYPE_GROUP),
								 key_id, tsc, GFP_ATOMIC);
}





VOID CFG80211OS_EnableChanFlagsByBand(IN struct ieee80211_channel *pChannels,
				      IN UINT32 n_channels,
				      IN UINT32 freq_start_mhz,
				      IN UINT32 freq_end_mhz, IN UINT32 flags)
{
	INT32 idx = 0;

	if (!pChannels)
		return;

	for (idx = 0; idx < n_channels; idx++) {
		if ((pChannels[idx].center_freq >= (UINT16) freq_start_mhz) &&
		    (pChannels[idx].center_freq <= (UINT16) freq_end_mhz)) {
			/* If this is not disabled channel, we clear the flag of IEEE80211_CHAN_DISABLED */
				pChannels[idx].flags &= ~IEEE80211_CHAN_DISABLED;
		}
	}

}

VOID CFG80211OS_ForceUpdateChanFlagsByBand(IN struct ieee80211_supported_band *pBand,
					   IN struct ieee80211_channel *pChannelUpdate)
{
	struct ieee80211_channel *pChannels;
	INT32 idx = 0;

	if (!pBand || !pChannelUpdate)
		return;

	pChannels = pBand->channels;

	for (idx = 0; idx < pBand->n_channels; idx++)
		pChannels[idx].flags = pChannelUpdate[idx].flags;
}

INT32 CFG80211OS_UpdateRegRuleByRegionIdx(IN VOID *pCB, IN VOID *pChDesc2G, IN VOID *pChDesc5G)
{
	CFG80211_CB *pCfg80211_CB = (CFG80211_CB *) pCB;
	struct wiphy *pWiphy = NULL;
	UINT32 freq_start_mhz = 0, freq_end_mhz = 0;
	PCH_DESC pChDesc = NULL;
	INT32 n_channels = 0;
	INT32 ii = 0;
	struct ieee80211_supported_band *pSband;
	if (!pCB || (!pChDesc2G && !pChDesc5G))
		return -EINVAL;
	pWiphy = pCfg80211_CB->pCfg80211_Wdev->wiphy;
	if (!pWiphy)
		CFG80211DBG(DBG_LVL_ERROR, ("80211> %s: invalid pWiphy!!\n", __FUNCTION__));
	/* 2GHz rules */
	pChDesc = (PCH_DESC) pChDesc2G;
	n_channels = pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_2GHZ].n_channels;
	if (pChDesc && n_channels > 0) {
		struct ieee80211_channel pTmpCh[n_channels];
		memset(pTmpCh, 0, sizeof(pTmpCh));
		/* init all channels to be disabled */
		for (ii = 0; ii < n_channels; ii++) {
			pTmpCh[ii].flags |= IEEE80211_CHAN_DISABLED;
			pTmpCh[ii].center_freq =
			    pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_2GHZ].channels[ii].
			    center_freq;
		}
		while (pChDesc && pChDesc->FirstChannel) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
			freq_start_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel,
							   IEEE80211_BAND_2GHZ);
			freq_end_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel +
							   (pChDesc->NumOfCh - 1),
							   IEEE80211_BAND_2GHZ);
#else
			freq_start_mhz = ieee80211_channel_to_frequency(
				pChDesc->FirstChannel);
			freq_end_mhz = ieee80211_channel_to_frequency(
				pChDesc->FirstChannel + (pChDesc->NumOfCh - 1));
#endif /* LINUX_VERSION_CODE */
			CFG80211OS_EnableChanFlagsByBand(pTmpCh, n_channels, freq_start_mhz,
							 freq_end_mhz,
							 (UINT32) pChDesc->ChannelProp);
			pChDesc++;
		}
		pSband = pWiphy->bands[IEEE80211_BAND_2GHZ];		
		CFG80211OS_ForceUpdateChanFlagsByBand(pSband, pTmpCh);
	}
	/* 5GHz rules */
	pChDesc = (PCH_DESC) pChDesc5G;
	n_channels = pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_5GHZ].n_channels;
	if (pChDesc && n_channels > 0) {
		struct ieee80211_channel pTmpCh2[n_channels];
		memset(pTmpCh2, 0, sizeof(pTmpCh2));
		/* init all channels to be disabled */
		for (ii = 0; ii < n_channels; ii++) {
			pTmpCh2[ii].flags |= IEEE80211_CHAN_DISABLED;
			pTmpCh2[ii].center_freq =
			    pCfg80211_CB->Cfg80211_bands[IEEE80211_BAND_5GHZ].channels[ii].
			    center_freq;
		}
		while (pChDesc && pChDesc->FirstChannel) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
			freq_start_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel,
							   IEEE80211_BAND_5GHZ);
			freq_end_mhz =
			    ieee80211_channel_to_frequency(pChDesc->FirstChannel +
							   ((pChDesc->NumOfCh - 1) * 4),
							   IEEE80211_BAND_5GHZ);
#else
			freq_start_mhz = ieee80211_channel_to_frequency(
				pChDesc->FirstChannel);
			freq_end_mhz = ieee80211_channel_to_frequency(
				pChDesc->FirstChannel +
					((pChDesc->NumOfCh - 1) * 4));
#endif
			CFG80211OS_EnableChanFlagsByBand(pTmpCh2, n_channels, freq_start_mhz,
							 freq_end_mhz,
							 (UINT32) pChDesc->ChannelProp);
			pChDesc++;
		}
		pSband = pWiphy->bands[IEEE80211_BAND_5GHZ];
		CFG80211OS_ForceUpdateChanFlagsByBand(pSband, pTmpCh2);
	}
	return 0;
}

#endif /* RT_CFG80211_SUPPORT */
