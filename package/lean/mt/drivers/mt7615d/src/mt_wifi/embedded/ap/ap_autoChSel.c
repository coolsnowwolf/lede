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
#include "ap_autoChSel.h"
#ifdef TR181_SUPPORT
#include "hdev/hdev_basic.h"
#endif

extern UCHAR ZeroSsid[32];

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;
#ifdef AP_SCAN_SUPPORT
extern INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
#endif/*AP_SCAN_SUPPORT*/

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
extern VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);
#endif


#ifdef DOT11_VHT_AC
struct vht_ch_layout {
	UCHAR ch_low_bnd;
	UCHAR ch_up_bnd;
	UCHAR cent_freq_idx;
};
#endif/* DOT11_VHT_AC */

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

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN struct wifi_dev *pwdev)
{
	UCHAR i;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return (ULONG)BSS_NOT_FOUND;
	}

	for (i = 0; i < pBssInfoTab->BssNr; i++) {
		if ((((pBssInfoTab->BssEntry[i].Channel <= 14) && (Channel <= 14)) ||
			 ((pBssInfoTab->BssEntry[i].Channel > 14) && (Channel > 14))) &&
			MAC_ADDR_EQUAL(&(pBssInfoTab->BssEntry[i].Bssid), Bssid) &&
			(SSID_EQUAL(pSsid, SsidLen, pBssInfoTab->BssEntry[i].Ssid, pBssInfoTab->BssEntry[i].SsidLen) ||
			 (NdisEqualMemory(pSsid, ZeroSsid, SsidLen)) ||
			 (NdisEqualMemory(pBssInfoTab->BssEntry[i].Ssid, ZeroSsid, pBssInfoTab->BssEntry[i].SsidLen))))
			return i;
	}

	return (ULONG)BSS_NOT_FOUND;
}

static inline VOID AutoChBssEntrySet(
	OUT BSSENTRY * pBss,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi)
{
	COPY_MAC_ADDR(pBss->Bssid, pBssid);

	if (SsidLen > 0 && SsidLen <= MAX_LEN_OF_SSID) {
		/*
			For hidden SSID AP, it might send beacon with SSID len equal to 0,
			Or send beacon /probe response with SSID len matching real SSID length,
			but SSID is all zero. such as "00-00-00-00" with length 4.
			We have to prevent this case overwrite correct table
		*/
		if (NdisEqualMemory(Ssid, ZeroSsid, SsidLen) == 0) {
			NdisMoveMemory(pBss->Ssid, Ssid, SsidLen);
			pBss->SsidLen = SsidLen;
		}
	}

	pBss->Channel = Channel;
	pBss->ExtChOffset = ExtChOffset;
	pBss->Rssi = Rssi;
}


VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch_index,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev)
{
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
#ifdef ACS_CTCC_SUPPORT
	CHANNEL_CTRL *ch_ctrl = NULL;
	INT score = 0;

	ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
#endif
	if (pAutoChCtrl->pChannelInfo != NULL) {
		UINT32 BusyTime;
		UINT32 cca_cnt = AsicGetCCACnt(pAd);

		if (Alg == ChannelAlgCCA) {
			pAd->RalinkCounters.OneSecFalseCCACnt += cca_cnt;
			pAutoChCtrl->pChannelInfo->FalseCCA[ch_index] = cca_cnt;
		}

		/*
			do busy time statistics for primary channel
			scan time 200ms, beacon interval 100 ms
		*/
		BusyTime = AsicGetChBusyCnt(pAd, BandIdx);
#if (defined(CUSTOMER_DCC_FEATURE) || defined(OFFCHANNEL_SCAN_FEATURE))
		if ((pAd->ScanCtrl.ScanTime[pAd->ScanCtrl.CurrentGivenChan_Index]) != 0)
			pAd->ScanCtrl.ScanTimeActualEnd = ktime_get();
#endif
#ifdef AP_QLOAD_SUPPORT
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
#else
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / 200;
#endif /* AP_QLOAD_SUPPORT */
#ifdef OFFCHANNEL_SCAN_FEATURE
		pAd->ChannelInfo.chanbusytime[ch_index] = BusyTime;
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("[%s] channel busy time[%d] = %d\n", __func__, ch_index, BusyTime));
		if ((pAd->ScanCtrl.ScanTime[pAd->ScanCtrl.CurrentGivenChan_Index]) != 0) {
			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("[%s] calling Calculate_NF with bandidx = %d\n", __func__, pAd->ChannelInfo.bandidx));
			Calculate_NF(pAd, pAd->ChannelInfo.bandidx);
		}
#endif
#ifdef CUSTOMER_DCC_FEATURE
		pAd->ChannelInfo.FalseCCA[ch_index] = cca_cnt;
		if (!pAd->ChannelInfo.GetChannelInfo)
			pAd->ChannelInfo.GetChannelInfo = TRUE;
#endif
#if (defined(CUSTOMER_DCC_FEATURE) || defined(OFFCHANNEL_SCAN_FEATURE))
		if ((pAd->ScanCtrl.ScanTime[pAd->ScanCtrl.CurrentGivenChan_Index]) != 0) {
			/* Calculate the channel busy value precision by using actual scan time */
			pAd->ScanCtrl.ScanTimeActualDiff = ktime_to_ms(ktime_sub(pAd->ScanCtrl.ScanTimeActualEnd, pAd->ScanCtrl.ScanTimeActualStart)) + 1;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("time_diff: %d Busytime: %d\n",
				pAd->ScanCtrl.ScanTimeActualDiff, pAd->ChannelInfo.chanbusytime[ch_index]));
		}
#endif
#ifdef ACS_CTCC_SUPPORT
#ifdef AP_QLOAD_SUPPORT
		score = 100 - BusyTime/(AUTO_CHANNEL_SEL_TIMEOUT * 10);
		pAutoChCtrl->pChannelInfo->supp_ch_list[ch_index].busy_time =
			(BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
#else
		score = 100 - BusyTime/(200 * 10);
		pAutoChCtrl->pChannelInfo->supp_ch_list[ch_index].busy_time = (BusyTime * 100) / 200;
#endif
		if (score < 0)
			score = 0;
		pAutoChCtrl->pChannelInfo->channel_score[ch_index].score = score;
		pAutoChCtrl->pChannelInfo->channel_score[ch_index].channel = ch_ctrl->ChList[ch_index].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("channel %d busytime %d\n",
			pAutoChCtrl->pChannelInfo->supp_ch_list[ch_index].channel,
			pAutoChCtrl->pChannelInfo->chanbusytime[ch_index]));
#endif
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAutoChCtrl->pChannelInfo equal NULL.\n"));
}

static inline INT GetChIdx(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN UCHAR BandIdx)
{
	INT Idx;

	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	for (Idx = 0; Idx < pChCtrl->ChListNum; Idx++) {
		if (Channel == pChCtrl->ChList[Idx].Channel)
			break;
	}

	return Idx;
}

static inline VOID AutoChannelSkipListSetDirty(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR i;
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		UCHAR channel_idx = GetChIdx(pAd, pAd->ApCfg.AutoChannelSkipList[i], BandIdx);

		if (channel_idx != pChCtrl->ChListNum)
			pAutoChCtrl->pChannelInfo->SkipList[channel_idx] = TRUE;
	}
}

static inline BOOLEAN AutoChannelSkipListCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	UCHAR i;
	BOOLEAN result = FALSE;

	for (i = 0; i < pAd->ApCfg.AutoChannelSkipListNum; i++) {
		if (Ch == pAd->ApCfg.AutoChannelSkipList[i]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

static inline BOOLEAN BW40_ChannelCheck(
	IN UCHAR ch)
{
	INT i;
	BOOLEAN result = TRUE;
	UCHAR NorBW40_CH[] = {140, 165};
	UCHAR NorBW40ChNum = sizeof(NorBW40_CH) / sizeof(UCHAR);

	for (i = 0; i < NorBW40ChNum; i++) {
		if (ch == NorBW40_CH[i]) {
			result = FALSE;
			break;
		}
	}

	return result;
}

static inline UCHAR SelectClearChannelRandom(RTMP_ADAPTER *pAd)
{
	UCHAR cnt, ch = 0, i, RadomIdx;
	/*BOOLEAN bFindIt = FALSE;*/
	UINT8 TempChList[MAX_NUM_OF_CHANNELS] = {0};
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (pAd->CommonCfg.bIEEE80211H) {
		cnt = 0;

		/* Filter out an available channel list */
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			/* Check DFS channel RemainingTimeForUse */
			if (pChCtrl->ChList[i].RemainingTimeForUse)
				continue;

			/* Check skip channel list */
			if (AutoChannelSkipListCheck(pAd, pChCtrl->ChList[i].Channel) == TRUE)
				continue;

#ifdef DOT11_N_SUPPORT

			/* Check N-group of BW40 */
			if (cfg_ht_bw == BW_40 &&
				!(pChCtrl->ChList[i].Flags & CHANNEL_40M_CAP))
				continue;

#endif /* DOT11_N_SUPPORT */
			/* Store available channel to temp list */
			TempChList[cnt++] = pChCtrl->ChList[i].Channel;
		}

		/* Randomly select a channel from temp list */
		if (cnt) {
			RadomIdx = RandomByte2(pAd)%cnt;
			ch = TempChList[RadomIdx];
		} else
			ch = get_channel_by_reference(pAd, 1, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
	} else {
		ch = pChCtrl->ChList[RandomByte2(pAd)%pChCtrl->ChListNum].Channel;

		if (ch == 0)
			ch = FirstChannel(pAd, wdev);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Select Channel %d\n", __func__, ch));
	return ch;
}

/*
	==========================================================================
	Description:
	This routine calaulates the dirtyness of all channels by the
	CCA value  and Rssi. Store dirtyness to pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */
static inline UCHAR SelectClearChannelCCA(RTMP_ADAPTER *pAd)
{
#define CCA_THRESHOLD (100)
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	INT ch = 1, channel_idx, BssTab_idx;
	BSSENTRY *pBss;
	UINT32 min_dirty, min_falsecca;
	int candidate_ch;
	UCHAR  ExChannel[2] = {0}, candidate_ExChannel[2] = {0};
	UCHAR base;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(&pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);

	if (pBssInfoTab == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return FirstChannel(pAd, wdev);
	}

	if (pChannelInfo == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return FirstChannel(pAd, wdev);
	}

	for (BssTab_idx = 0; BssTab_idx < pBssInfoTab->BssNr; BssTab_idx++) {
		pBss = &(pBssInfoTab->BssEntry[BssTab_idx]);
		channel_idx = GetChIdx(pAd, pBss->Channel, BandIdx);

		if (channel_idx < 0 || channel_idx >= MAX_NUM_OF_CHANNELS+1)
			continue;


		if (pBss->Rssi >= RSSI_TO_DBM_OFFSET-50) {
			/* high signal >= -50 dbm */
			pChannelInfo->dirtyness[channel_idx] += 50;
		} else if (pBss->Rssi <= RSSI_TO_DBM_OFFSET-80) {
			/* low signal <= -80 dbm */
			pChannelInfo->dirtyness[channel_idx] += 30;
		} else {
			/* mid signal -50 ~ -80 dbm */
			pChannelInfo->dirtyness[channel_idx] += 40;
		}

		pChannelInfo->dirtyness[channel_idx] += 40;
		{
			INT BelowBound;
			INT AboveBound;
			INT loop;

			switch (pBss->ExtChOffset) {
			case EXTCHA_ABOVE:
				BelowBound = pChannelInfo->IsABand ? 1 : 4;
				AboveBound = pChannelInfo->IsABand ? 2 : 8;
				break;

			case EXTCHA_BELOW:
				BelowBound = pChannelInfo->IsABand ? 2 : 8;
				AboveBound = pChannelInfo->IsABand ? 1 : 4;
				break;

			default:
				BelowBound = pChannelInfo->IsABand ? 1 : 4;
				AboveBound = pChannelInfo->IsABand ? 1 : 4;
				break;
			}

			/* check neighbor channel */
			for (loop = (channel_idx+1); loop <= (channel_idx+AboveBound); loop++) {
				if (loop >= MAX_NUM_OF_CHANNELS)
					break;

				if (pChCtrl->ChList[loop].Channel - pChCtrl->ChList[loop-1].Channel > 4)
					break;

				pChannelInfo->dirtyness[loop] += ((9 - (loop - channel_idx)) * 4);
			}

			/* check neighbor channel */
			for (loop = (channel_idx-1); loop >= (channel_idx-BelowBound); loop--) {
				if (loop < 0 || loop >= MAX_NUM_OF_CHANNELS)
					break;

				if (pChCtrl->ChList[(loop+1) % MAX_NUM_OF_CHANNELS].Channel - pChCtrl->ChList[loop % MAX_NUM_OF_CHANNELS].Channel > 4)
					continue;

				pChannelInfo->dirtyness[loop] +=
					((9 - (channel_idx - loop)) * 4);
			}
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" ch%d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
				 pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));
	}

	AutoChannelSkipListSetDirty(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));

	for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel %d : Dirty = %ld, False CCA = %u, Busy Time = %u, Skip Channel = %s\n",
				 pChCtrl->ChList[channel_idx].Channel,
				 pChannelInfo->dirtyness[channel_idx],
				 pChannelInfo->FalseCCA[channel_idx],
#ifdef AP_QLOAD_SUPPORT
				 pChannelInfo->chanbusytime[channel_idx],
#else
				 0,
#endif /* AP_QLOAD_SUPPORT */
				 (pChannelInfo->SkipList[channel_idx] == TRUE) ? "TRUE" : "FALSE"));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));
	min_dirty = min_falsecca = 0xFFFFFFFF;
	/*
	 * Rule 1. Pick up a good channel that False_CCA =< CCA_THRESHOLD
	 *		   by dirtyness
	 */
	candidate_ch = -1;

	for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
		if (pChannelInfo->SkipList[channel_idx] == TRUE)
			continue;

		if (pChannelInfo->FalseCCA[channel_idx] <= CCA_THRESHOLD) {
			UINT32 dirtyness = pChannelInfo->dirtyness[channel_idx];

			ch = pChCtrl->ChList[channel_idx].Channel;
#ifdef AP_QLOAD_SUPPORT

			/* QLOAD ALARM */
			/* when busy time of a channel > threshold, skip it */
			/* TODO: Use weight for different references to do channel selection */
			if (QBSS_LoadIsBusyTimeAccepted(pAd,
											pChannelInfo->chanbusytime[channel_idx]) == FALSE) {
				/* check next one */
				continue;
			}

#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11_N_SUPPORT

			/*
				User require 40MHz Bandwidth.
				In the case, ignor all channel
				doesn't support 40MHz Bandwidth.
			*/
			if ((cfg_ht_bw == BW_40)
				&& (pChannelInfo->IsABand && (GetABandChOffset(ch) == 0)))
				continue;

			/*
				Need to Consider the dirtyness of extending channel
				in 40 MHz bandwidth channel.
			*/
			if (cfg_ht_bw == BW_40) {
				if (pAutoChCtrl->pChannelInfo->IsABand) {
					if (((channel_idx + GetABandChOffset(ch)) >= 0)
						&& ((channel_idx + GetABandChOffset(ch)) < pChCtrl->ChListNum)) {
						INT ChOffsetIdx = channel_idx + GetABandChOffset(ch);

						dirtyness += pChannelInfo->dirtyness[ChOffsetIdx];
					}
				} else {
					UCHAR ExChannel_idx = 0;

					if (pChCtrl->ChList[channel_idx].Channel == 14) {
						dirtyness = 0xFFFFFFFF;
						break;
					}
					NdisZeroMemory(ExChannel, sizeof(ExChannel));

					if (((channel_idx - 4) >= 0) && ((channel_idx - 4) < pChCtrl->ChListNum)) {
						dirtyness += pChannelInfo->dirtyness[channel_idx - 4];
						ExChannel[ExChannel_idx++] = pChCtrl->ChList[channel_idx - 4].Channel;
					}

					if (((channel_idx + 4) >= 0) && ((channel_idx + 4) < pChCtrl->ChListNum)) {
						dirtyness += pChannelInfo->dirtyness[channel_idx + 4];
						ExChannel[ExChannel_idx++] = pChCtrl->ChList[channel_idx + 4].Channel;
					}
				}
			}

#endif /* DOT11_N_SUPPORT */

			if (min_dirty > dirtyness) {
				min_dirty = dirtyness;
				candidate_ch = channel_idx;
				NdisMoveMemory(candidate_ExChannel, ExChannel, 2);
			}
		}
	}

	if (candidate_ch >= 0) {
		ch = pChCtrl->ChList[candidate_ch].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 1 CCA value : Min Dirtiness (Include extension channel) ==> Select Channel %d\n", ch));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Min Dirty = %u\n", min_dirty));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ExChannel = %d , %d\n", candidate_ExChannel[0], candidate_ExChannel[1]));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("BW        = %s\n", (cfg_ht_bw == BW_40) ? "40" : "20"));
		return ch;
	}

	/*
	 * Rule 2. Pick up a good channel that False_CCA > CCA_THRESHOLD
	 *		   by FalseCCA (FalseCCA + Dirtyness)
	 */
	candidate_ch = -1;

	for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
		if (pChannelInfo->SkipList[channel_idx] == TRUE)
			continue;

		if (pChannelInfo->FalseCCA[channel_idx] > CCA_THRESHOLD) {
			UINT32 falsecca = pChannelInfo->FalseCCA[channel_idx] + pChannelInfo->dirtyness[channel_idx];

			ch = pChCtrl->ChList[channel_idx].Channel;
#ifdef DOT11_N_SUPPORT

			if ((cfg_ht_bw == BW_40)
				&& (pChannelInfo->IsABand && (GetABandChOffset(ch) == 0)))
				continue;

#endif /* DOT11_N_SUPPORT */

			if ((GetABandChOffset(ch) != 0)
				&& ((channel_idx + GetABandChOffset(ch)) >= 0)
				&& ((channel_idx + GetABandChOffset(ch)) < pChCtrl->ChListNum)) {
				INT ChOffsetIdx = channel_idx + GetABandChOffset(ch);

				falsecca += (pChannelInfo->FalseCCA[ChOffsetIdx] +
							 pChannelInfo->dirtyness[ChOffsetIdx]);
			}

#ifdef AP_QLOAD_SUPPORT

			/* QLOAD ALARM */
			/* when busy time of a channel > threshold, skip it */
			/* TODO: Use weight for different references to do channel selection */
			if (QBSS_LoadIsBusyTimeAccepted(pAd,
											pChannelInfo->chanbusytime[channel_idx]) == FALSE) {
				/* check next one */
				continue;
			}

#endif /* AP_QLOAD_SUPPORT */

			if (min_falsecca > falsecca) {
				min_falsecca = falsecca;
				candidate_ch = channel_idx;
			}
		}
	}

	if (candidate_ch >= 0) {
		ch = pChCtrl->ChList[candidate_ch].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 2 CCA value : Min False CCA value ==> Select Channel %d, min falsecca = %d\n", ch, min_falsecca));
		return	ch;
	}

	base = RandomByte2(pAd);

	for (channel_idx = 0; channel_idx < pChCtrl->ChListNum; channel_idx++) {
		ch = pChCtrl->ChList[(base + channel_idx) % pChCtrl->ChListNum].Channel;

		if (AutoChannelSkipListCheck(pAd, ch))
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&& (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, ch))
			continue;

		break;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 3 CCA value : Randomly Select ==> Select Channel %d\n", ch));
	return ch;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
VOID AutoChannelSkipChannels(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			size,
	IN UINT16			grpStart)
{
	UCHAR i = 0;

	for (i = 0; i < size; i++)
		AutoChannelSkipListAppend(pAd, (grpStart + (i*4)));
}

VOID AutoChannelSkipListClear(
	IN PRTMP_ADAPTER	pAd)
{
	UCHAR ChIdx = 0;

	os_zero_mem(pAd->ApCfg.AutoChannelSkipList, 20);
	pAd->ApCfg.AutoChannelSkipListNum = 0;

	for (ChIdx = 0; ChIdx < 20; ChIdx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s] Ch = %3d\n", __func__, pAd->ApCfg.AutoChannelSkipList[ChIdx]));
	}
}


VOID AutoChannelSkipListAppend(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch)
{
	pAd->ApCfg.AutoChannelSkipList[pAd->ApCfg.AutoChannelSkipListNum] = Ch;

	pAd->ApCfg.AutoChannelSkipListNum++;
}

BOOLEAN DfsV10ACSMarkChnlConsumed(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR i = 0;
	BOOLEAN status = FALSE;

	if (IS_DFS_V10_ACS_VALID(pAd) == FALSE)
		return FALSE;

	for (i = 0; i < V10_TOTAL_CHANNEL_COUNT; i++) {
		if (channel == pDfsParam->DfsV10SortedACSList[i].Channel) {
			pDfsParam->DfsV10SortedACSList[i].isConsumed = TRUE;
			status = TRUE;
			break;
		} else
			continue;
	}

	if (status == FALSE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("[%s] Channel %d not found \n", __func__, channel));

	return status;
}

BOOLEAN DfsV10ACSListSortFunction(
	IN PRTMP_ADAPTER pAd)
{
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UINT_32 temp_busy = 0;
	UCHAR i = 0, j = 0, temp_chnl = 0;

	if ((!pAd->ApCfg.bAutoChannelAtBootup) && IS_DFS_V10_ACS_VALID(pAd)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] False Entry \n", __func__));
		return FALSE;
	}

	for (i = 0; i < V10_TOTAL_CHANNEL_COUNT; i++) {
		for (j = i+1; j < V10_TOTAL_CHANNEL_COUNT; j++) {
			if (pDfsParam->DfsV10SortedACSList[i].BusyTime > pDfsParam->DfsV10SortedACSList[j].BusyTime) {
				temp_busy = pDfsParam->DfsV10SortedACSList[i].BusyTime;
				temp_chnl = pDfsParam->DfsV10SortedACSList[i].Channel;

				pDfsParam->DfsV10SortedACSList[i].BusyTime = pDfsParam->DfsV10SortedACSList[j].BusyTime;
				pDfsParam->DfsV10SortedACSList[i].Channel  = pDfsParam->DfsV10SortedACSList[j].Channel;

				pDfsParam->DfsV10SortedACSList[j].BusyTime = temp_busy;
				pDfsParam->DfsV10SortedACSList[j].Channel  = temp_chnl;
			}
		}
	}

	for (i = 0; i < V10_TOTAL_CHANNEL_COUNT; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel %d\t", pDfsParam->DfsV10SortedACSList[i].Channel));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Busy Time %d\t", pDfsParam->DfsV10SortedACSList[i].BusyTime));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Used %d\n", pDfsParam->DfsV10SortedACSList[i].isConsumed));
	}

	pDfsParam->bV10ChannelListValid = TRUE;

	return TRUE;
}
#endif

/*
	==========================================================================
	Description:
	This routine calaulates the dirtyness of all channels by the dirtiness value and
	number of AP in each channel and stores in pChannelInfo strcut.
		This routine is called at iwpriv cmmand or initialization. It chooses and returns
		a good channel whith less interference.
	Return:
		ch -  channel number that
	NOTE:
	==========================================================================
 */
static inline UCHAR SelectClearChannelApCnt(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	/*PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab; */
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	/*BSSENTRY *pBss; */
	UCHAR channel_index = 0, dirty, base = 0;
	UCHAR final_channel = 0;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(pwdev);
	UCHAR ext_cha = wlan_operate_get_ext_cha(pwdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	if (pChannelInfo == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return FirstChannel(pAd, pwdev);
	}

	/* Calculate Dirtiness */

	for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
		if (pChannelInfo->ApCnt[channel_index] > 0) {
			INT ll;

			pChannelInfo->dirtyness[channel_index] += 30;

			/*5G */
			if (pChannelInfo->IsABand) {
				int Channel = pChCtrl->ChList[channel_index].Channel;

				/*Make secondary channel dirty */
				if (op_ht_bw == BW_40) {
					if (Channel > 14) {
						if ((Channel == 36) || (Channel == 44) || (Channel == 52) || (Channel == 60)
							|| (Channel == 100) || (Channel == 108) || (Channel == 116)
							|| (Channel == 124) || (Channel == 132) || (Channel == 149) || (Channel == 157)) {
							if (channel_index + 1 < MAX_NUM_OF_CHANNELS)
								if (pChCtrl->ChList[channel_index+1].Channel - pChCtrl->ChList[channel_index].Channel == 4)
									pChannelInfo->dirtyness[channel_index+1] += 1;
						} else if ((Channel == 40) || (Channel == 48) || (Channel == 56) ||
								   (Channel == 64) || (Channel == 104) || (Channel == 112) ||
								   (Channel == 120) || (Channel == 128) || (Channel == 136) ||
								   (Channel == 153) || (Channel == 161)) {
							if (channel_index - 1 >= 0)
								if (pChCtrl->ChList[channel_index].Channel - pChCtrl->ChList[channel_index-1].Channel == 4)
									pChannelInfo->dirtyness[channel_index-1] += 1;
						}
					}
				}
			}

			/*2.4G */
			if (!pChannelInfo->IsABand) {
				int ChanOffset = 0;

				if ((op_ht_bw == BW_40) &&
						(ext_cha == EXTCHA_BELOW)) {
					/*
						BW is 40Mhz
						the distance between two channel to prevent interference
						is 4 channel width plus 4 channel width (secondary channel)
					*/
					ChanOffset = 8;
				} else {
					/*
						BW is 20Mhz
						The channel width of 2.4G band is 5Mhz.
						The distance between two channel to prevent interference is 4 channel width
					*/
					ChanOffset = 4;
				}

				for (ll = channel_index + 1; ll < (channel_index + ChanOffset + 1); ll++) {
					if (ll < MAX_NUM_OF_CHANNELS)
						pChannelInfo->dirtyness[ll]++;
				}

				if ((op_ht_bw == BW_40) &&
						(ext_cha == EXTCHA_ABOVE)) {
					/* BW is 40Mhz */
					ChanOffset = 8;
				} else {
					/* BW is 20Mhz */
					ChanOffset = 4;
				}

				for (ll = channel_index - 1; ll > (channel_index - ChanOffset - 1); ll--) {
					if (ll >= 0 && ll < MAX_NUM_OF_CHANNELS+1)
						pChannelInfo->dirtyness[ll]++;
				}
			}
		}
	} /* Calculate Dirtiness */

	AutoChannelSkipListSetDirty(pAd);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));

	for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++)
		/* debug messages */
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Channel %d : Dirty = %ld, ApCnt=%ld, Busy Time = %d, Skip Channel = %s\n",
				 pChCtrl->ChList[channel_index].Channel,
				 pChannelInfo->dirtyness[channel_index],
				 pChannelInfo->ApCnt[channel_index],
#ifdef AP_QLOAD_SUPPORT
				 pChannelInfo->chanbusytime[channel_index],
#else
				 0,
#endif /* AP_QLOAD_SUPPORT */
				 (pChannelInfo->SkipList[channel_index] == TRUE) ? "TRUE" : "FALSE"));

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("=====================================================\n"));
	pAd->ApCfg.AutoChannel_Channel = 0;

	/* RULE 1. pick up a good channel that no one used */

	for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
		if (pChannelInfo->SkipList[channel_index] == TRUE)
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, pChCtrl->ChList[channel_index].Channel))
			continue;

#ifdef AP_QLOAD_SUPPORT

		/* QLOAD ALARM */
		if (QBSS_LoadIsBusyTimeAccepted(pAd,
										pChannelInfo->chanbusytime[channel_index]) == FALSE)
			continue;

#endif /* AP_QLOAD_SUPPORT */

		if (pChannelInfo->dirtyness[channel_index] == 0)
			break;
	}

	if (channel_index < pChCtrl->ChListNum) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 1 APCnt : dirtiness == 0 (no one used and no interference) ==> Select Channel %d\n", pChCtrl->ChList[channel_index].Channel));
		return pChCtrl->ChList[channel_index].Channel;
	}

	/* RULE 2. if not available, then co-use a channel that's no interference (dirtyness=30) */
	/* RULE 3. if not available, then co-use a channel that has minimum interference (dirtyness=31,32) */
	for (dirty = 30; dirty <= 32; dirty++) {
		BOOLEAN candidate[MAX_NUM_OF_CHANNELS+1], candidate_num = 0;
		UCHAR min_ApCnt = 255;

		final_channel = 0;
		NdisZeroMemory(candidate, MAX_NUM_OF_CHANNELS+1);

		for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
			if (pChannelInfo->SkipList[channel_index] == TRUE)
				continue;

			if (pChannelInfo->dirtyness[channel_index] == dirty) {
				candidate[channel_index] = TRUE;
				candidate_num++;
			}
		}

		/* if there's more than 1 candidate, pick up the channel with minimum RSSI */
		if (candidate_num) {
			for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
#ifdef AP_QLOAD_SUPPORT

				/* QLOAD ALARM */
				/* when busy time of a channel > threshold, skip it */
				/* TODO: Use weight for different references to do channel selection */
				if (QBSS_LoadIsBusyTimeAccepted(pAd,
												pChannelInfo->chanbusytime[channel_index]) == FALSE) {
					/* check next one */
					continue;
				}

#endif /* AP_QLOAD_SUPPORT */

				if (candidate[channel_index] && (pChannelInfo->ApCnt[channel_index] < min_ApCnt)) {
					if ((op_ht_bw == BW_40)
						&& (BW40_ChannelCheck(pChCtrl->ChList[channel_index].Channel) == FALSE))
						continue;

					if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
						&& RadarChannelCheck(pAd, pChCtrl->ChList[channel_index].Channel))
						continue;

					final_channel = pChCtrl->ChList[channel_index].Channel;
					min_ApCnt = pChannelInfo->ApCnt[channel_index];
				}
			}

			if (final_channel != 0) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("Rule 2 APCnt : minimum APCnt with  minimum interference(dirtiness: 30~32) ==> Select Channel %d\n", final_channel));
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Dirtiness = %d ,  Min ApCnt = %d\n", dirty, min_ApCnt));
				return final_channel;
			}
		}
	}

	/* RULE 3. still not available, pick up the random channel */
	base = RandomByte2(pAd);

	for (channel_index = 0; channel_index < pChCtrl->ChListNum; channel_index++) {
		final_channel = pChCtrl->ChList[(base + channel_index) % pChCtrl->ChListNum].Channel;

		if (AutoChannelSkipListCheck(pAd, final_channel))
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE) && (pChannelInfo->IsABand == TRUE)
			&& RadarChannelCheck(pAd, final_channel))
			continue;

		break;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rule 3 APCnt : Randomly Select  ==> Select Channel %d\n", final_channel));
	return final_channel;
}

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi,
	IN struct wifi_dev *pwdev)
{
	ULONG	Idx;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pBssInfoTab equal NULL.\n"));
		return BSS_NOT_FOUND;
	}

	Idx = AutoChBssSearchWithSSID(pAd, pBssid, (PUCHAR)Ssid, SsidLen, ChannelNo, pwdev);

	if (Idx == BSS_NOT_FOUND) {
		Idx = pBssInfoTab->BssNr;
		if (Idx >= MAX_LEN_OF_BSS_TABLE)
			return BSS_NOT_FOUND;

		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
						  ChannelNo, ExtChOffset, Rssi);
		pBssInfoTab->BssNr++;
	} else {
		AutoChBssEntrySet(&pBssInfoTab->BssEntry[Idx % MAX_LEN_OF_BSS_TABLE], pBssid, Ssid, SsidLen,
						  ChannelNo, ExtChOffset, Rssi);
	}

	return Idx;
}

void AutoChBssTableDestroy(RTMP_ADAPTER *pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl;
	UCHAR BandIdx;

	pAutoChCtrl = HcGetAutoChCtrl(pAd);

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

		if (!pAutoChCtrl)
			continue;

		if (pAutoChCtrl->pBssInfoTab) {
			os_free_mem(pAutoChCtrl->pBssInfoTab);
			pAutoChCtrl->pBssInfoTab = NULL;
		}
	}

}


static VOID AutoChBssTableReset(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PBSSINFO pBssInfoTab = pAutoChCtrl->pBssInfoTab;

	if (pBssInfoTab)
		NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
	else {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("pAutoChCtrl->pBssInfoTab equal NULL.\n"));
	}
}


void AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd)
{
	BSSINFO *pBssInfoTab = NULL;
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl = NULL;

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

		os_alloc_mem(pAd, (UCHAR **)&pBssInfoTab, sizeof(BSSINFO));

		if (pBssInfoTab) {
			NdisZeroMemory(pBssInfoTab, sizeof(BSSINFO));
			pAutoChCtrl->pBssInfoTab = pBssInfoTab;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s Fail to alloc memory for pAutoChCtrl->pBssInfoTab",
					  __func__));
		}
	}

}


void ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BandIdx;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		if (pAutoChCtrl->pChannelInfo) {
			os_free_mem(pAutoChCtrl->pChannelInfo);
			pAutoChCtrl->pChannelInfo = NULL;
		}
	}
}


static VOID ChannelInfoReset(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	CHANNELINFO *ch_info = pAutoChCtrl->pChannelInfo;

	if (ch_info)
		NdisZeroMemory(ch_info, sizeof(CHANNELINFO));
	else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("pChannelInfo equal NULL, band:%d\n", BandIdx));
}


void ChannelInfoInit(
	IN PRTMP_ADAPTER pAd)
{
	CHANNELINFO *ch_info = NULL;
	UCHAR BandIdx;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		ch_info = NULL;
		os_alloc_mem(pAd, (UCHAR **)&ch_info, sizeof(CHANNELINFO));

		if (ch_info) {
			os_zero_mem(ch_info, sizeof(CHANNELINFO));
			pAutoChCtrl->pChannelInfo = ch_info;
		} else {
			pAutoChCtrl->pChannelInfo = NULL;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s Fail to alloc memory for pAd->pChannelInfo", __func__));
		}
	}
}

#ifdef ACS_CTCC_SUPPORT
VOID build_acs_scan_ch_list(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR channel_idx = 0;
	UCHAR ch_list_num = 0;
	UCHAR ch = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR op_ext_cha = wlan_config_get_ext_cha(wdev);
	UCHAR band_idx = HcGetBandByWdev(wdev);
	CHANNEL_CTRL *ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
	AUTO_CH_CTRL *auto_ch_ctrl = NULL;

	auto_ch_ctrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[%s] ------------>\n", __func__));
	AutoChannelSkipListSetDirty(pAd);
	if (auto_ch_ctrl->pChannelInfo->IsABand) {
		for (channel_idx = 0; channel_idx < ch_ctrl->ChListNum; channel_idx++) {
			ch = ch_ctrl->ChList[channel_idx].Channel;
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].ap_cnt =
				auto_ch_ctrl->pChannelInfo->ApCnt[ch_list_num];
			if (cfg_ht_bw == BW_20) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel =
					ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req =
					ch_ctrl->ChList[channel_idx].DfsReq;
		if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#ifdef DOT11_N_SUPPORT
			else if ((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
				&& (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
				) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel =
					ch_ctrl->ChList[channel_idx].Channel;

				if (N_ChannelGroupCheck(pAd, ch, wdev)) {
					if (GetABandChOffset(ch) == 1)
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel + 2;
					else
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel - 2;
				} else
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req =
					ch_ctrl->ChList[channel_idx].DfsReq;
				if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#ifdef DOT11_VHT_AC
			else if (wlan_config_get_vht_bw(wdev) == VHT_BW_80) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel =
					ch_ctrl->ChList[channel_idx].Channel;
				if (vht80_channel_group(pAd, ch))
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					vht_cent_ch_freq(ch, VHT_BW_80);
				else
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					ch_ctrl->ChList[channel_idx].Channel;
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].dfs_req =
					ch_ctrl->ChList[channel_idx].DfsReq;
				if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
				ch_list_num++;
			}
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		}
	} else {
		for (channel_idx = 0; channel_idx < ch_ctrl->ChListNum; channel_idx++) {
			if (cfg_ht_bw == BW_40) {
				if (op_ext_cha == EXTCHA_ABOVE)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
					ch_ctrl->ChList[channel_idx].Channel + 2;
				else {
					if (auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].channel == 14)
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel - 1;
					else
						auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
						ch_ctrl->ChList[channel_idx].Channel - 2;
				}
			} else
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].cen_channel =
				ch_ctrl->ChList[channel_idx].Channel;
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].channel =
				ch_ctrl->ChList[channel_idx].Channel;
			if (auto_ch_ctrl->pChannelInfo->SkipList[channel_idx] == TRUE)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel = TRUE;
			ch_list_num++;
		}
	}
	auto_ch_ctrl->pChannelInfo->channel_list_num = ch_list_num;
	for (channel_idx = 0; channel_idx < auto_ch_ctrl->pChannelInfo->channel_list_num; channel_idx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("[%s] Support channel: PrimCh=%d, CentCh=%d, DFS=%d, skip %d\n",
			__func__, auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].channel,
			auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].cen_channel,
			auto_ch_ctrl->pChannelInfo->supp_ch_list[channel_idx].dfs_req,
			auto_ch_ctrl->pChannelInfo->supp_ch_list[ch_list_num].skip_channel));
	}
}

UINT8 acs_group_ch_list_search(
	AUTO_CH_CTRL *auto_ch_ctrl,
	UCHAR cen_channel)
{
	UCHAR i;
	struct acs_scan_ch_group_list *group_ch_list = auto_ch_ctrl->pChannelInfo->group_ch_list;

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++)	{
		if (group_ch_list->cen_channel == cen_channel)
			return i;
		group_ch_list++;
	}

	return 0xff;
}

VOID acs_group_ch_list_insert(
	AUTO_CH_CTRL *auto_ch_ctrl,
	struct acs_scan_supp_ch_list *source,
	IN struct wifi_dev *wdev)
{
	UCHAR i = auto_ch_ctrl->pChannelInfo->group_ch_list_num;
	UCHAR j = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
	struct acs_scan_ch_group_list *group_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[i];

	group_ch_list->best_ctrl_channel = source->channel;
	group_ch_list->cen_channel = source->cen_channel;
	group_ch_list->max_busy_time = source->busy_time;
	group_ch_list->min_busy_time = source->busy_time;
	group_ch_list->skip_group = source->skip_channel;
	if (cfg_vht_bw == VHT_BW_80) {
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw80_grp_ch_member_idx++;
	} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw40_grp_ch_member_idx++;
	} else {
	}

	for (j = 0; j < auto_ch_ctrl->pChannelInfo->channel_list_num; j++) {
		if (source->channel == auto_ch_ctrl->pChannelInfo->channel_score[j].channel)
			group_ch_list->grp_score = auto_ch_ctrl->pChannelInfo->channel_score[j].score;
	}

	auto_ch_ctrl->pChannelInfo->group_ch_list_num = i + 1;
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s Insert new group channel list Number=%d,cen_channel=%d\n",
		__func__, auto_ch_ctrl->pChannelInfo->group_ch_list_num, group_ch_list->cen_channel));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s best_ctrl_channel=%d,BUSY_TIEM=%d,skip_group=%d,grp_score=%d\n",
		__func__, group_ch_list->best_ctrl_channel, group_ch_list->max_busy_time,
		group_ch_list->skip_group, group_ch_list->grp_score));
}

VOID acs_group_ch_list_update(
	AUTO_CH_CTRL *auto_ch_ctrl,
	UCHAR index,
	struct acs_scan_supp_ch_list *source,
	IN struct wifi_dev *wdev)
{
	UCHAR i = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
	struct acs_scan_ch_group_list *group_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[index];

	if (source->busy_time > group_ch_list->max_busy_time) {
		group_ch_list->max_busy_time = source->busy_time;
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (source->channel == auto_ch_ctrl->pChannelInfo->channel_score[i].channel)
				group_ch_list->grp_score = auto_ch_ctrl->pChannelInfo->channel_score[i].score;
		}
	}

	if (source->busy_time < group_ch_list->min_busy_time) {
		group_ch_list->min_busy_time = source->busy_time;
		group_ch_list->best_ctrl_channel = source->channel;
	}

	if (group_ch_list->skip_group == 0 && source->skip_channel == 1)
		group_ch_list->skip_group = source->skip_channel;

	if (cfg_vht_bw == VHT_BW_80) {
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw80_grp_ch_member[group_ch_list->bw80_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw80_grp_ch_member_idx++;
	} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].channel = source->channel;
		group_ch_list->bw40_grp_ch_member[group_ch_list->bw40_grp_ch_member_idx].busy_time = source->busy_time;
		group_ch_list->bw40_grp_ch_member_idx++;
	} else {
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s Update group channel list index=%d,cen_channel=%d\n",
		__func__, auto_ch_ctrl->pChannelInfo->group_ch_list_num, group_ch_list->cen_channel));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s best_ctrl_channel=%d,BUSY_TIEM=%d,skip_group=%d,grp_score=%d\n",
		__func__, group_ch_list->best_ctrl_channel, group_ch_list->max_busy_time,
		group_ch_list->skip_group, group_ch_list->grp_score));
}

VOID acs_generate_group_channel_list(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR i = 0;
	UCHAR list_index = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	AUTO_CH_CTRL *auto_ch_ctrl = NULL;
	struct acs_scan_supp_ch_list *supp_ch_list = NULL;

	auto_ch_ctrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);
	supp_ch_list = &auto_ch_ctrl->pChannelInfo->supp_ch_list[0];
	memset(auto_ch_ctrl->pChannelInfo->group_ch_list, 0,
		(MAX_NUM_OF_CHANNELS+1) * sizeof(struct acs_scan_ch_group_list));
	auto_ch_ctrl->pChannelInfo->group_ch_list_num = 0;

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		list_index = acs_group_ch_list_search(auto_ch_ctrl, supp_ch_list->cen_channel);
		if (list_index == 0xff)
			acs_group_ch_list_insert(auto_ch_ctrl, supp_ch_list, wdev);
		else
			acs_group_ch_list_update(auto_ch_ctrl, list_index, supp_ch_list, wdev);

		supp_ch_list++;
	}
}

UCHAR find_best_channel_of_all_grp(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	UINT32 j = 0;
	UINT32 k = 0;
	INT l = 0;
	UINT32 min_busy = 0xffffffff;
	UINT32 max_busy = 0x0;
	UINT32 min_score = 0x0;
	UINT32 busy = 0;
	UCHAR best_channel = 0;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	AUTO_CH_CTRL *auto_ch_ctrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);
	CHANNEL_CTRL *ch_ctrl = hc_get_channel_ctrl(pAd->hdev_ctrl, band_idx);
	struct auto_ch_sel_grp_member tmp;
	struct acs_scan_ch_group_list *acs_grp_ch_list = NULL;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++) {
		acs_grp_ch_list = &auto_ch_ctrl->pChannelInfo->group_ch_list[i];

		if (cfg_vht_bw == VHT_BW_80) {
			for (k = 0; k < 3; k++) {
				for (j = 0; j < 3 - k; j++) {
					if (acs_grp_ch_list->bw80_grp_ch_member[j].busy_time >
						acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time) {
						tmp.busy_time = acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time;
						tmp.channel =  acs_grp_ch_list->bw80_grp_ch_member[j+1].channel;
						acs_grp_ch_list->bw80_grp_ch_member[j+1].busy_time =
							acs_grp_ch_list->bw80_grp_ch_member[j].busy_time;
						acs_grp_ch_list->bw80_grp_ch_member[j+1].channel =
							acs_grp_ch_list->bw80_grp_ch_member[j].channel;
						acs_grp_ch_list->bw80_grp_ch_member[j].busy_time = tmp.busy_time;
						acs_grp_ch_list->bw80_grp_ch_member[j].channel = tmp.channel;
					}
				}
			}

			if (vht80_channel_group(pAd, auto_ch_ctrl->pChannelInfo->group_ch_list[i].cen_channel) == FALSE)
				acs_grp_ch_list->bw80_not_allowed = TRUE;
		} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
			if (acs_grp_ch_list->bw40_grp_ch_member[0].busy_time >
				acs_grp_ch_list->bw40_grp_ch_member[1].busy_time) {
				tmp.busy_time = acs_grp_ch_list->bw40_grp_ch_member[1].busy_time;
				tmp.channel =  acs_grp_ch_list->bw40_grp_ch_member[1].channel;
				acs_grp_ch_list->bw40_grp_ch_member[1].busy_time =
					acs_grp_ch_list->bw40_grp_ch_member[0].busy_time;
				acs_grp_ch_list->bw40_grp_ch_member[1].channel =
					acs_grp_ch_list->bw40_grp_ch_member[0].channel;
				acs_grp_ch_list->bw40_grp_ch_member[0].busy_time = tmp.busy_time;
				acs_grp_ch_list->bw40_grp_ch_member[0].channel = tmp.channel;
			}

			if (vht40_channel_group(pAd, auto_ch_ctrl->pChannelInfo->group_ch_list[i].cen_channel) == FALSE)
				acs_grp_ch_list->bw40_not_allowed = TRUE;
		} else {
		}

		if (auto_ch_ctrl->pChannelInfo->group_ch_list[i].skip_group == FALSE) {
			busy = auto_ch_ctrl->pChannelInfo->group_ch_list[i].max_busy_time;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("ChIdx=%d control-Channel=%d cen-channel=%d Max_BUSY_TIME=%d\n",
				i, auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel,
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].cen_channel,
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].max_busy_time));

			if ((busy <= min_busy) &&
				(acs_grp_ch_list->bw80_not_allowed == FALSE) &&
				(acs_grp_ch_list->bw40_not_allowed == FALSE)) {
				min_busy = busy;
				best_channel = auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel;
			}

			if (busy > max_busy) {
				max_busy = busy;
				min_score = 100 - max_busy/1000;
				if (min_score < 0)
					min_score = 0;
			}
	    }
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->group_ch_list_num; i++) {
	for (j = 0; j < auto_ch_ctrl->pChannelInfo->channel_list_num; j++) {
	if (auto_ch_ctrl->pChannelInfo->group_ch_list[i].best_ctrl_channel ==
		auto_ch_ctrl->pChannelInfo->supp_ch_list[j].channel) {
	if (cfg_vht_bw == VHT_BW_80) {
		for (k = 0; k < 4; k++) {
		for (l = -3; l < 4; l++) {
			if ((j+l < 0) || (j+l >= auto_ch_ctrl->pChannelInfo->channel_list_num))
				continue;
			if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel ==
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_grp_ch_member[k].channel) {
				auto_ch_ctrl->pChannelInfo->channel_score[j+l].score =
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].grp_score + 3 - k;
				if ((auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw80_not_allowed == TRUE) ||
(vht80_channel_group(pAd, auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel) == FALSE)) {
					auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = min_score;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Channel %d don't support BW80, force to assign min score(%d)!!!!!!\n",
						auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel,
						auto_ch_ctrl->pChannelInfo->channel_score[j+l].score));
				}
				if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].score > 100)
					auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = 100;
				break;
			}
		}
		}
	} else if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040)) {
		for (k = 0; k < 2; k++) {
		for (l = -1; l < 2; l++) {
			if ((j+l < 0) || (j+l >= auto_ch_ctrl->pChannelInfo->channel_list_num))
				continue;
			if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel ==
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw40_grp_ch_member[k].channel) {
				auto_ch_ctrl->pChannelInfo->channel_score[j+l].score =
				auto_ch_ctrl->pChannelInfo->group_ch_list[i].grp_score + 1 - k;
				if ((auto_ch_ctrl->pChannelInfo->group_ch_list[i].bw40_not_allowed == TRUE) ||
(vht40_channel_group(pAd, auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel) == FALSE)) {
					auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = min_score;
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Channel %d don't support BW40, force to assign min score(%d)!!!!!!\n",
						auto_ch_ctrl->pChannelInfo->channel_score[j+l].channel,
						auto_ch_ctrl->pChannelInfo->channel_score[j+l].score));
				}
				if (auto_ch_ctrl->pChannelInfo->channel_score[j+l].score > 100)
					auto_ch_ctrl->pChannelInfo->channel_score[j+l].score = 100;
				break;
			}
		}
		}
		} else {
				}
	}
	}
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("=================ACS score board===================\n"));
	for (j = 0; j < ch_ctrl->ChListNum; j++) {
		if (auto_ch_ctrl->pChannelInfo->channel_score[j].channel == 0)
			continue;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("CH%d final score is %d\n", auto_ch_ctrl->pChannelInfo->channel_score[j].channel,
			 auto_ch_ctrl->pChannelInfo->channel_score[j].score));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("Min Busy Time=%d,select best channel %d\n", min_busy, best_channel));
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("====================================================\n"));

	return best_channel;
}

static inline UCHAR select_clear_channel_busy_time(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UINT32 i = 0;
	UINT32 score = 0;
	UINT32 ch1_busy_time = 0xffffffff;
	UINT32 ch6_busy_time = 0xffffffff;
	UINT32 ch11_busy_time = 0xffffffff;
	UINT32 min_busy = 0xffffffff;
	UINT32 max_busy = 0;
	UCHAR best_channel = 0;
	UINT8 bit_map = 0;/*ch1:bit0;ch6:bit1;ch11:bit2*/
	UCHAR band_idx = HcGetBandByWdev(wdev);
	AUTO_CH_CTRL *auto_ch_ctrl = HcGetAutoChCtrlbyBandIdx(pAd, band_idx);

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1) ||
			(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6) ||
			(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11))) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy) {
				min_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time > max_busy)
				max_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if ((auto_ch_ctrl->pChannelInfo->ApCnt[i] != 0)) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1)
				bit_map |= (1<<0);
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6)
				bit_map |= (1<<1);
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11)
				bit_map |= (1<<2);
		}
	}
	/*AP @ Ch 1,6,11*/
	switch (bit_map) {
	case 7:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1) ||
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6) ||
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11))) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time < min_busy) {
					min_busy = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
				}
			}

		}

		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (((auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1) &&
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6) &&
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11))) {
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			}
		}
	break;
	case 6:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 5:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 4:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1)
				ch1_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6)
				ch6_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch1_busy_time <= ch6_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 3:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;
	case 2:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 1)
				ch1_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11)
				ch11_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch1_busy_time <= ch11_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 1)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 1:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 6)
				ch6_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
			else if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel == 11)
				ch11_busy_time = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time;
		}

		if (ch6_busy_time <= ch11_busy_time) {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 6)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		} else {
			for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
				if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel != 11)
					auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
				else
					best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
			}
		}
	break;
	case 0:
		for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
			if (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time != min_busy)
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time += max_busy;
			else
				best_channel = auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;
		}
	break;

	default:
	break;
	}

	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		score = 100 - (auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time / 1000);
		if (score < 0)
			score = 0;
		auto_ch_ctrl->pChannelInfo->channel_score[i].score = score;
		auto_ch_ctrl->pChannelInfo->channel_score[i].channel =
			auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel;

	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("=====================================================\n"));
	for (i = 0; i < auto_ch_ctrl->pChannelInfo->channel_list_num; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Channel %d : Busy Time = %u, Score %d,Skip Channel = %s\n",
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].channel,
				auto_ch_ctrl->pChannelInfo->supp_ch_list[i].busy_time,
				auto_ch_ctrl->pChannelInfo->channel_score[i].score,
				(auto_ch_ctrl->pChannelInfo->supp_ch_list[i].skip_channel == TRUE) ? "TRUE" : "FALSE"));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("=====================================================\n"));
	}

	if (WMODE_CAP_5G(wdev->PhyMode)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("=============== select best ch for 5G ===============\n"));
		acs_generate_group_channel_list(pAd, wdev);
		best_channel = find_best_channel_of_all_grp(pAd, wdev);
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("=============== select best ch for 2.4G =============\n"));

	return best_channel;
}
#else
static inline UCHAR SelectClearChannelBusyTime(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR BandIdx = HcGetBandByWdev(wdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	PCHANNELINFO pChannelInfo = pAutoChCtrl->pChannelInfo;
	UINT32 SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, MinBusyTime;
	UINT32 SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx, ChannelIdx, StartChannelIdx, Temp1, Temp2;
	INT	i, j, GroupNum, CandidateCh1 = 0, CandidateChIdx1, base;
#ifdef DOT11_VHT_AC
	UINT32 MinorMinBusyTime;
	INT CandidateCh2, CandidateChIdx2;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);
	UCHAR cen_ch_2;
#endif/* DOT11_VHT_AC */
#ifndef DOT11_VHT_AC
#endif/* DOT11_VHT_AC */
	PUINT32 pSubGroupMaxBusyTimeTable = NULL;
	PUINT32 pSubGroupMaxBusyTimeChIdxTable = NULL;
	PUINT32 pSubGroupMinBusyTimeTable = NULL;
	PUINT32 pSubGroupMinBusyTimeChIdxTable = NULL;
#if defined(ONDEMAND_DFS) || defined(DFS_VENDOR10_CUSTOM_FEATURE)
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[SelectClearChannelBusyTime] - band%d START\n", BandIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[SelectClearChannelBusyTime] - cfg_ht_bw = %d vht_bw = %d\n", cfg_ht_bw, vht_bw));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	if (pChannelInfo == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pAd->pChannelInfo equal NULL.\n"));
		return FirstChannel(pAd, wdev);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("====================================================================\n"));

	for (ChannelIdx = 0; ChannelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChannelIdx++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel %3d : Busy Time = %6u, Skip Channel = %s, BwCap = %s\n",
				 pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Channel, pChannelInfo->chanbusytime[ChannelIdx],
				 (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].SkipChannel == TRUE) ? "TRUE" : "FALSE",
				 (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].BwCap == TRUE)?"TRUE" : "FALSE"));
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("====================================================================\n"));
	/*Initialization*/
	SubGroupMaxBusyTimeChIdx = 0;
	SubGroupMaxBusyTime = pChannelInfo->chanbusytime[SubGroupMaxBusyTimeChIdx];
	SubGroupMinBusyTimeChIdx = 0;
	SubGroupMinBusyTime = pChannelInfo->chanbusytime[SubGroupMinBusyTimeChIdx];
	StartChannelIdx = SubGroupMaxBusyTimeChIdx + 1;
	GroupNum = 0;
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	os_alloc_mem(pAd, (UCHAR **)&pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMaxBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMaxBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));
	NdisZeroMemory(pSubGroupMinBusyTimeChIdxTable, (MAX_NUM_OF_CHANNELS+1)*sizeof(UINT32));

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
		&& (IS_DFS_V10_ACS_VALID(pAd) == FALSE) && (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)) {
		UCHAR listSize = 0;
		ChannelIdx = 0;
		listSize = V10_TOTAL_CHANNEL_COUNT;

		NdisZeroMemory(pDfsParam->DfsV10SortedACSList, (V10_TOTAL_CHANNEL_COUNT)*sizeof(V10_CHANNEL_LIST));
		pDfsParam->DfsV10SortedACSList[ChannelIdx].BusyTime = pChannelInfo->chanbusytime[ChannelIdx];
		pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Channel;
	}
#endif

	for (ChannelIdx = StartChannelIdx; ChannelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChannelIdx++) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
			&& (IS_DFS_V10_ACS_VALID(pAd) == FALSE) && (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)) {
			pDfsParam->DfsV10SortedACSList[ChannelIdx].BusyTime = pChannelInfo->chanbusytime[ChannelIdx];
			pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Channel;
		}
#endif
		/*Compare the busytime with each other in the same group*/
		if (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].CentralChannel == pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx-1].CentralChannel) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("pChannelInfo->chanbusytime[%d] = %d, SubGroupMaxBusyTime = %d, SubGroupMinBusyTime = %d\n",
					  ChannelIdx, pChannelInfo->chanbusytime[ChannelIdx], SubGroupMaxBusyTime, SubGroupMinBusyTime));

			if (pChannelInfo->chanbusytime[ChannelIdx] > SubGroupMaxBusyTime) {
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;
			} else if (pChannelInfo->chanbusytime[ChannelIdx] < SubGroupMinBusyTime) {
				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("SubGroupMaxBusyTime = %d, SubGroupMaxBusyTimeChIdx = %d,SubGroupMinBusyTime = %d SubGroupMinBusyTimeChIdx = %d\n",
					  SubGroupMaxBusyTime, SubGroupMaxBusyTimeChIdx, SubGroupMinBusyTime, SubGroupMinBusyTimeChIdx));

			/*Fill in the group table in order for the last group*/
			if (ChannelIdx == (pAutoChCtrl->AutoChSelCtrl.ChListNum - 1)) {
				pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;
				pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
						  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
						  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));
				GroupNum++;
			}
		} else {
			/*Fill in the group table*/
			pSubGroupMaxBusyTimeTable[GroupNum] = SubGroupMaxBusyTime;
			pSubGroupMaxBusyTimeChIdxTable[GroupNum] = SubGroupMaxBusyTimeChIdx;
			pSubGroupMinBusyTimeTable[GroupNum] = SubGroupMinBusyTime;
			pSubGroupMinBusyTimeChIdxTable[GroupNum] = SubGroupMinBusyTimeChIdx;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
					  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
					  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));
			GroupNum++;

			/*Fill in the group table in order for the last group in case of BW20*/
			if ((ChannelIdx == (pAutoChCtrl->AutoChSelCtrl.ChListNum - 1))
				&& (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChannelIdx].Bw == BW_20)) {
				pSubGroupMaxBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];
				pSubGroupMaxBusyTimeChIdxTable[GroupNum] = ChannelIdx;
				pSubGroupMinBusyTimeTable[GroupNum] = pChannelInfo->chanbusytime[ChannelIdx];
				pSubGroupMinBusyTimeChIdxTable[GroupNum] = ChannelIdx;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("SubGroupMaxBusyTimeTable[%d] = %d, SubGroupMaxBusyTimeChIdxTable[%d] = %d, SubGroupMinBusyTimeTable[%d] = %d, SubGroupMinBusyTimeChIdxTable[%d] = %d\n",
						  GroupNum, pSubGroupMaxBusyTimeTable[GroupNum], GroupNum, pSubGroupMaxBusyTimeChIdxTable[GroupNum],
						  GroupNum, pSubGroupMinBusyTimeTable[GroupNum], GroupNum, pSubGroupMinBusyTimeChIdxTable[GroupNum]));
				GroupNum++;
			} else {
				/*Reset indices in order to start checking next group*/
				SubGroupMaxBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMaxBusyTimeChIdx = ChannelIdx;
				SubGroupMinBusyTime = pChannelInfo->chanbusytime[ChannelIdx];
				SubGroupMinBusyTimeChIdx = ChannelIdx;
			}
		}
	}

	for (i = 0; i < GroupNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d,\nSubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d\n",
				  i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
				  i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i]));
	}

	/*Sort max_busy_time group table from the smallest to the biggest one  */
	for (i = 0; i < GroupNum; i++) {
		for (j = 1; j < (GroupNum-i); j++) {
			if (pSubGroupMaxBusyTimeTable[i] > pSubGroupMaxBusyTimeTable[i+j]) {
				/*Swap pSubGroupMaxBusyTimeTable[i] for pSubGroupMaxBusyTimeTable[i+j]*/
				Temp1 = pSubGroupMaxBusyTimeTable[i+j];
				pSubGroupMaxBusyTimeTable[i+j] = pSubGroupMaxBusyTimeTable[i];
				pSubGroupMaxBusyTimeTable[i] = Temp1;
				/*Swap pSubGroupMaxBusyTimeChIdxTable[i] for pSubGroupMaxBusyTimeChIdxTable[i+j]*/
				Temp2 = pSubGroupMaxBusyTimeChIdxTable[i+j];
				pSubGroupMaxBusyTimeChIdxTable[i+j] = pSubGroupMaxBusyTimeChIdxTable[i];
				pSubGroupMaxBusyTimeChIdxTable[i] = Temp2;
				/*Swap pSubGroupMinBusyTimeTable[i] for pSubGroupMinBusyTimeTable[i+j]*/
				Temp1 = pSubGroupMinBusyTimeTable[i+j];
				pSubGroupMinBusyTimeTable[i+j] = pSubGroupMinBusyTimeTable[i];
				pSubGroupMinBusyTimeTable[i] = Temp1;
				/*Swap pSubGroupMinBusyTimeChIdxTable[i] for pSubGroupMinBusyTimeChIdxTable[i+j]*/
				Temp2 = pSubGroupMinBusyTimeChIdxTable[i+j];
				pSubGroupMinBusyTimeChIdxTable[i+j] = pSubGroupMinBusyTimeChIdxTable[i];
				pSubGroupMinBusyTimeChIdxTable[i] = Temp2;
			}
		}
	}

	for (i = 0; i < GroupNum; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("SubGroupMaxBusyTimeTable[%d] = %d, pSubGroupMaxBusyTimeChIdxTable[%d] = %d,\nSubGroupMinBusyTimeTable[%d] = %d, pSubGroupMinBusyTimeChIdxTable[%d] = %d\n",
				  i, pSubGroupMaxBusyTimeTable[i], i, pSubGroupMaxBusyTimeChIdxTable[i],
				  i, pSubGroupMinBusyTimeTable[i], i, pSubGroupMinBusyTimeChIdxTable[i]));
	}

#ifdef DOT11_VHT_AC

	/*Return channel in case of VHT BW80+80*/
	if ((vht_bw == VHT_BW_8080)
		&& (cfg_ht_bw == BW_40)
		&& (GroupNum > 2)
		&& (WMODE_CAP_AC(wdev->PhyMode) == TRUE)) {
		MinBusyTime = pSubGroupMaxBusyTimeTable[0];
		MinorMinBusyTime = pSubGroupMaxBusyTimeTable[1];
		/*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Channel;
		/*Select secondary VHT80 central channel*/
		CandidateChIdx2 = pSubGroupMaxBusyTimeChIdxTable[1];
		CandidateCh2 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx2].Channel;
		cen_ch_2 = vht_cent_ch_freq((UCHAR)CandidateCh2, VHT_BW_80);
		/*Since primary channel is not updated yet ,cannot update sec Ch here*/
		wdev->auto_channel_cen_ch_2 = cen_ch_2;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : Select Primary Channel %d\n", CandidateCh1));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : Select Secondary Central Channel %d\n", cen_ch_2));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : MinorMin Channel Busy = %u\n", MinorMinBusyTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : BW = %s\n", "80+80"));
		os_free_mem(pSubGroupMaxBusyTimeTable);
		os_free_mem(pSubGroupMaxBusyTimeChIdxTable);
		os_free_mem(pSubGroupMinBusyTimeTable);
		os_free_mem(pSubGroupMinBusyTimeChIdxTable);
		goto ReturnCh;
	}

#endif/*DOT11_VHT_AC*/

	if (GroupNum > 0) {
#ifdef ONDEMAND_DFS
	if (IS_SUPPORT_ONDEMAND_ZEROWAIT_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
		&& (IS_ONDEMAND_ACS_LIST_VALID(pAd) == FALSE)) {
		os_alloc_mem(pAd, (UCHAR **)&pDfsParam->OnDemandChannelList, (GroupNum)*sizeof(OD_CHANNEL_LIST));
		if (pDfsParam->OnDemandChannelList) {
			/* Record Best Channels from each group */
			os_zero_mem(pDfsParam->OnDemandChannelList, (GroupNum)*sizeof(OD_CHANNEL_LIST));
			for (ChannelIdx = 0; ChannelIdx < GroupNum; ChannelIdx++) {
				CandidateCh1 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[ChannelIdx]].Channel;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Channel %d BusyTime %d Idx %d\n",
					CandidateCh1, pSubGroupMinBusyTimeTable[ChannelIdx],
					 pSubGroupMinBusyTimeChIdxTable[ChannelIdx]));
				pDfsParam->OnDemandChannelList[ChannelIdx].Channel = CandidateCh1;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Channel %d \n", pDfsParam->OnDemandChannelList[ChannelIdx].Channel));
			}
			/* Enable ACS List */
			pDfsParam->bOnDemandChannelListValid = TRUE;
			pDfsParam->MaxGroupCount = GroupNum;
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[%s] Null Pointer returned for ACS List \n", __func__));
		}
	}
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	/* V10 VHT80 ACS Enable */
	if ((pAd->ApCfg.bAutoChannelAtBootup) && IS_SUPPORT_V10_DFS(pAd) && (WMODE_CAP_5G(wdev->PhyMode))
		&& (IS_DFS_V10_ACS_VALID(pAd) == FALSE) && (wlan_config_get_vht_bw(wdev) == VHT_BW_80)) {
			/* Record Best Channels from each group */
			os_zero_mem(pDfsParam->DfsV10SortedACSList, (GroupNum)*sizeof(V10_CHANNEL_LIST));
			for (ChannelIdx = 0; ChannelIdx < GroupNum; ChannelIdx++) {
				CandidateCh1 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[pSubGroupMinBusyTimeChIdxTable[ChannelIdx]].Channel;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Channel %d BusyTime %d Idx %d\n",
					CandidateCh1, pSubGroupMinBusyTimeTable[ChannelIdx],
					 pSubGroupMinBusyTimeChIdxTable[ChannelIdx]));
				pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel = CandidateCh1;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Channel %d \n", pDfsParam->DfsV10SortedACSList[ChannelIdx].Channel));
			}
			/* Enable V10 VHT80 ACS List */
			pDfsParam->bV10ChannelListValid = TRUE;
			pDfsParam->GroupCount = GroupNum;
		}

		if ((pAd->ApCfg.bAutoChannelAtBootup) && (wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
			&& DfsV10ACSListSortFunction(pAd) == FALSE)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("[%s] Invalid V10 ACS List BW %d \n", __func__, wlan_config_get_vht_bw(wdev)));
#endif

		MinBusyTime = pSubGroupMaxBusyTimeTable[0];
		/*Select primary channel, whose busy time is minimum in the group*/
		CandidateChIdx1 = pSubGroupMinBusyTimeChIdxTable[0];
		CandidateCh1 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Channel;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : Select Primary Channel %d\n", CandidateCh1));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : Min Channel Busy = %u\n", MinBusyTime));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("Rule 3 Channel Busy time value : BW = %s\n",
				 (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw == BW_160) ? "160"
				 : (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw == BW_80) ? "80"
				 : (pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[CandidateChIdx1].Bw == BW_40) ? "40":"20"));
		os_free_mem(pSubGroupMaxBusyTimeTable);
		os_free_mem(pSubGroupMaxBusyTimeChIdxTable);
		os_free_mem(pSubGroupMinBusyTimeTable);
		os_free_mem(pSubGroupMinBusyTimeChIdxTable);
		goto ReturnCh;
	}

	base = RandomByte2(pAd);

	for (ChannelIdx = 0; ChannelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChannelIdx++) {
		CandidateCh1 = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[(base + ChannelIdx) % pAutoChCtrl->AutoChSelCtrl.ChListNum].Channel;

		if (AutoChannelSkipListCheck(pAd, CandidateCh1))
			continue;

		if ((pAd->ApCfg.bAvoidDfsChannel == TRUE)
			&& (pAutoChCtrl->AutoChSelCtrl.IsABand == TRUE)
			&& RadarChannelCheck(pAd, CandidateCh1))
			continue;

		break;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("Randomly Select : Select Channel %d\n", CandidateCh1));
ReturnCh:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[SelectClearChannelBusyTime] - band%d END\n", BandIdx));
	return CandidateCh1;
}
#endif
/*
	==========================================================================
	Description:
		This routine sets the current PhyMode for calculating
		the dirtyness.
	Return:
		none
	NOTE:
	==========================================================================
 */
void CheckPhyModeIsABand(RTMP_ADAPTER *pAd, UINT8 BandIdx)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	UCHAR PhyMode = HcGetRadioPhyModeByBandIdx(pAd, BandIdx);

	pAutoChCtrl->pChannelInfo->IsABand = (WMODE_CAP_5G(PhyMode)) ? TRUE : FALSE;
}


UCHAR SelectBestChannel(RTMP_ADAPTER *pAd, ChannelSel_Alg Alg, struct wifi_dev *pwdev)
{
	UCHAR ch = 0;
	/* init RadioCtrl.pChannelInfo->IsABand */
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	CheckPhyModeIsABand(pAd, BandIdx);
#ifdef MICROWAVE_OVEN_SUPPORT

	if (Alg == ChannelAlgCCA)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;

#endif /* MICROWAVE_OVEN_SUPPORT */

	switch (Alg) {
	case ChannelAlgRandom:
	case ChannelAlgApCnt:
		ch = SelectClearChannelApCnt(pAd, pwdev);
		break;

	case ChannelAlgCCA:
		ch = SelectClearChannelCCA(pAd);
		break;

	case ChannelAlgBusyTime:
#ifdef ACS_CTCC_SUPPORT
		ch = select_clear_channel_busy_time(pAd, pwdev);
#else
		ch = SelectClearChannelBusyTime(pAd, pwdev);
#endif
		break;

	default:
#ifdef ACS_CTCC_SUPPORT
		ch = select_clear_channel_busy_time(pAd, pwdev);
#else
		ch = SelectClearChannelBusyTime(pAd, pwdev);
#endif
		break;
	}

	RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, ch);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
	pAutoChCtrl->AutoChSelCtrl.SelCh = ch;
#ifdef CONFIG_INIT_RADIO_ONOFF
	if(pAd->ApCfg.bRadioOn == TRUE)
	{
		int i = 0;
		UCHAR line[256] = {0};
		UCHAR *event_msg = NULL;
		//PCHANNELINFO chinfo = pAd->pChannelInfo;
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
		PCHANNELINFO chinfo = pAutoChCtrl->pChannelInfo;
		//event_msg = kmalloc(1300, GFP_ATOMIC);
		os_alloc_mem(NULL, (UCHAR **)&event_msg, 5120);
		if(chinfo && event_msg)
		{
			NdisZeroMemory(event_msg, 5120);
			if (chinfo->IsABand)
				strcat(event_msg, "******** 5GHz ACS report ********\n");
			else
				strcat(event_msg, "******** 2.4GHz ACS report ********\n");
			strcat(event_msg, "+-------+-----------+----------+----------+----------+----+\n");
			snprintf(line, sizeof(line), "|%-7s|%-11s|%-10s|%-10s|%-10s|%-4s|\n","Channel","AP Detected","Dirty","False CCA","Busy Time","Skip");
			strcat(event_msg, line);
			strcat(event_msg, "+-------+-----------+----------+----------+----------+----+\n");
			for(; i < pAd->ChannelListNum; i++)
			{
				if(ch == pAd->ChannelList[i].Channel)
				{
					//strcat(event_msg, "|\033[40m\033[30;47m%-7d|%-11lu|%-10lu|%-10d|%-10d|%-4c\033[0m|\n",
					snprintf(line, sizeof(line), "|%-7d|%-11lu|%-10lu|%-10d|%-10d|%-4c|<--\n",
							pAd->ChannelList[i].Channel, chinfo->ApCnt[i], chinfo->dirtyness[i],
							chinfo->FalseCCA[i], chinfo->chanbusytime[i], (((chinfo->SkipList[i])? 'Y':'N')));
				}
				else
				{
					snprintf(line, sizeof(line), "|%-7d|%-11lu|%-10lu|%-10d|%-10d|%-4c|\n",
							pAd->ChannelList[i].Channel, chinfo->ApCnt[i], chinfo->dirtyness[i],
							chinfo->FalseCCA[i], chinfo->chanbusytime[i], (((chinfo->SkipList[i])? 'Y':'N')));
				}
				strcat(event_msg, line);
			}
			strcat(event_msg, "+-------+-----------+----------+----------+----------+----+\n");

			event_msg[5120] = '\0';
			ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_LOG_SAVE, 0 /*dummy*/, event_msg, strlen(event_msg));
			os_free_mem(event_msg);
		}
	}
#endif
	return ch;
}

VOID APAutoChannelInit(RTMP_ADAPTER *pAd, struct wifi_dev *pwdev)
{
	UINT32 BusyTime;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);

	/* reset bss table */
	AutoChBssTableReset(pAd, BandIdx);
	/* clear Channel Info */
	ChannelInfoReset(pAd, BandIdx);
	/* init RadioCtrl.pChannelInfo->IsABand */
	CheckPhyModeIsABand(pAd, BandIdx);
#ifdef ACS_CTCC_SUPPORT
	build_acs_scan_ch_list(pAd, pwdev);
#endif
	pAd->ApCfg.current_channel_index = 0;
	/* read clear for primary channel */
	BusyTime = AsicGetChBusyCnt(pAd, BandIdx);
}

/*
	==========================================================================
	Description:
		This routine is called at initialization. It returns a channel number
		that complies to regulation domain and less interference with current
		enviornment.
	Return:
		ch -  channel number that
	NOTE:
		The retruned channel number is guaranteed to comply to current regulation
		domain that recorded in pAd->CommonCfg.CountryRegion
	==========================================================================
 */
UCHAR APAutoSelectChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand)
{
	UCHAR ch = 0;

	if (pAd->phy_op && pAd->phy_op->AutoCh)
		ch = pAd->phy_op->AutoCh(pAd, pwdev, Alg, IsABand);

	return ch;
}

UCHAR MTAPAutoSelectChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand)
{
	UCHAR ch = 0, i = 0;
	UINT32 BusyTime;

	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	if (!BandIdx && WMODE_CAP_5G(pwdev->PhyMode) && pAd->CommonCfg.dbdc_mode)
		printk("[%s] Incorrect Bandidx for 5G Phy mode\n", __func__);
	if (BandIdx && WMODE_CAP_2G(pwdev->PhyMode) && pAd->CommonCfg.dbdc_mode)
		printk("[%s] Incorrect Bandidx for 2G Phy mode\n", __func__);
	if (pAutoChCtrl->AutoChSelCtrl.ACSChStat == ACS_CH_STATE_SELECTED) {
		ch = pAutoChCtrl->AutoChSelCtrl.SelCh;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[MTAPAutoSelectChannel] ACS channel is selected, selected ch = %d\n", ch));
		return ch;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	/* Init some structures before doing AutoChannelSelect() */
	APAutoChannelInit(pAd, pwdev);

#ifdef MICROWAVE_OVEN_SUPPORT
	pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
	AsicMeasureFalseCCA(pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

	/* Re-arrange channel list and fill in channel properties for auto-channel selection*/
	AutoChSelBuildChannelList(pAd, IsABand, pwdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: IsABand = %d, ChannelListNum = %d\n", __func__, IsABand, pAutoChCtrl->AutoChSelCtrl.ChListNum));

#ifdef ACS_CTCC_SUPPORT
	for (i = 0; i < pAutoChCtrl->pChannelInfo->channel_list_num; i++)
#else
	for (i = 0; i < pAutoChCtrl->AutoChSelCtrl.ChListNum; i++)
#endif
	{
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
		ULONG wait_time = GET_V10_OFF_CHNL_TIME(pAd);
#else
		ULONG wait_time = 200; /* Wait for 200 ms at each channel. */
#endif
#ifdef ACS_CTCC_SUPPORT
		wlan_operate_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[i].channel);
#else
		wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel);
#endif
		pAd->ApCfg.current_channel_index = i;
		pAd->ApCfg.AutoChannel_Channel = pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel;
		/* Read-Clear reset Channel busy time counter */
		BusyTime = AsicGetChBusyCnt(pAd, BandIdx);
#ifdef AP_QLOAD_SUPPORT
		/* QLOAD ALARM, ever alarm from QLOAD module */
		if (QLOAD_DOES_ALARM_OCCUR(pAd))
			wait_time = 400;
#endif /* AP_QLOAD_SUPPORT */
		OS_WAIT(wait_time);
		UpdateChannelInfo(pAd, i, Alg, pwdev);
	}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	if (IS_SUPPORT_V10_DFS(pAd) && pAutoChCtrl->pChannelInfo)
		/* Weighting Factor for ACS Enable Case */
		DfsV10AddWeighingFactor(pAd, pwdev);
#endif

	ch = SelectBestChannel(pAd, Alg, pwdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return ch;
}

/*
   ==========================================================================
   Description:
       Update channel to wdev which is supported for A-band or G-band.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelUpdateChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev)
{
	UINT8 ExtChaDir;
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	UCHAR i = 0;
	struct wifi_dev *wdev = NULL;
#endif

	if (IsABand) {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#ifdef CONFIG_AP_SUPPORT
		if (IS_SUPPORT_V10_DFS(pAd)) {
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					wdev = &pAd->ApCfg.MBSSID[i].wdev;
					wdev->channel = Channel;
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("BSS%d Channel=%d\n", i, wdev->channel));
				}
			}
		} else
#endif /* CONFIG_AP_SUPPORT */
#endif
		/*5G Channel*/
		pwdev->channel = Channel;
	} else {
		/*2G Channel*/
		/* Update primary channel in wdev */
		pwdev->channel = Channel;

		/* Query ext_cha in wdev */
		ExtChaDir = wlan_config_get_ext_cha(pwdev);

		/* Check current extension channel */
		if (!ExtChCheck(pAd, Channel, ExtChaDir, pwdev)) {
			if (ExtChaDir == EXTCHA_BELOW)
				ExtChaDir = EXTCHA_ABOVE;
			else
				ExtChaDir = EXTCHA_BELOW;

			/* Update ext_cha in wdev */
			wlan_config_set_ext_cha(pwdev, ExtChaDir);
		}

	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	("\x1b[41m %s(): Update channel for wdev for this band PhyMode = %d, Channel = %d \x1b[m\n",
	__func__, pwdev->PhyMode, pwdev->channel));
}

/*
   ==========================================================================
   Description:
       Build channel list for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelList(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev)
{

	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	/* Initialize channel list*/
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));

	if (IsABand) {
		pAutoChCtrl->AutoChSelCtrl.IsABand = TRUE;

		/* Build 5G channel list used by ACS */
		AutoChSelBuildChannelListFor5G(pAd, pwdev);
	} else if (!IsABand) {
		pAutoChCtrl->AutoChSelCtrl.IsABand = FALSE;

		/* Build 2G channel list used by ACS */
		AutoChSelBuildChannelListFor2G(pAd, pwdev);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}
/*
   ==========================================================================
   Description:
       Build channel list for 2.4G according to 1) Country Region 2) RF IC type for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor2G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	INT ChIdx, ChListNum = 0;
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
	AUTOCH_SEL_CH_LIST *pACSChList;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	pAutoChCtrl->AutoChSelCtrl.ChListNum = pChCtrl->ChListNum;

	/* Initialize local ACS channel list*/
	os_alloc_mem(NULL, (UCHAR **)&pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));
	os_zero_mem(pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++)
		pACSChList[ChIdx].Channel = pChCtrl->ChList[ChIdx].Channel;

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s : Ch = %3d\n", __func__, pACSChList[ChIdx].Channel));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));

	/* Check for skip-channel list */
	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++)
		pACSChList[ChIdx].SkipChannel = AutoChannelSkipListCheck(pAd, pACSChList[ChIdx].Channel);

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		/* 2.4G only support for BW20 auto-channel selection */
		pACSChList[ChIdx].Bw = BW_20;
		pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel;

		if (cfg_ht_bw == BW_20)
			pACSChList[ChIdx].BwCap = TRUE;
#ifdef DOT11_N_SUPPORT
		else if ((cfg_ht_bw == BW_40)
			&& N_ChannelGroupCheck(pAd, pACSChList[ChIdx].Channel, pwdev))
			pACSChList[ChIdx].BwCap = TRUE;
#endif /* DOT11_N_SUPPORT */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[AutoChSelBuildChannelListFor2G] - ChIdx = %d,  ChListNum = %d\n", ChIdx, pAutoChCtrl->AutoChSelCtrl.ChListNum));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s:	PrimChannel =  %3d, CenChannel = %3d, BW= %d, BwCap= %d, SkipChannel= %d\n", __func__,
				  pACSChList[ChIdx].Channel, pACSChList[ChIdx].CentralChannel,
				  pACSChList[ChIdx].Bw, pACSChList[ChIdx].BwCap,
				  pACSChList[ChIdx].SkipChannel));
	}

	for (ChIdx = 0; ChIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChIdx++) {
		if ((pACSChList[ChIdx].SkipChannel == TRUE) || (pACSChList[ChIdx].BwCap == FALSE))
			continue;
		else {
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel = pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel = pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel = pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
			ChListNum++;
		}
	}
	pAutoChCtrl->AutoChSelCtrl.ChListNum = ChListNum;
	os_free_mem(pACSChList);
}
/*
   ==========================================================================
   Description:
       Build channel list for 5G according to 1) Country Region 2) RF IC type for auto-channel selection.

    Return:
	None.
   ==========================================================================
 */
VOID AutoChSelBuildChannelListFor5G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
#define EXT_ABOVE     1
#define EXT_BELOW    -1
	INT ChIdx;
#ifdef DOT11_VHT_AC
	INT k, count, idx;
	struct vht_ch_layout *vht_ch_80M = get_ch_array(80);
	struct vht_ch_layout *vht_ch_160M = get_ch_array(160);
	UCHAR cfg_vht_bw = wlan_config_get_vht_bw(pwdev);
#endif/* DOT11_VHT_AC */
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(pwdev);
	AUTOCH_SEL_CH_LIST *pACSChList;
	INT ChListNum5G = 0;
	INT ChListNum = 0;

	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[AutoChSelBuildChannelListFor5G] cfg_ht_bw = %d, cfg_vht_bw = %d\n", cfg_ht_bw, cfg_vht_bw));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	/* Initialize local ACS channel list*/
	os_alloc_mem(NULL, (UCHAR **)&pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));
	os_zero_mem(pACSChList, (MAX_NUM_OF_CHANNELS+1) * sizeof(AUTOCH_SEL_CH_LIST));

	/*Skip Non occupancy channel*/
	for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
		if (CheckNonOccupancyChannel(pAd, pwdev, pChCtrl->ChList[ChIdx].Channel)) {
			pACSChList[ChListNum5G++].Channel = pChCtrl->ChList[ChIdx].Channel;
		}
	}

	for (ChIdx = 0; ChIdx < ChListNum5G; ChIdx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : Ch = %3d\n", __func__, pACSChList[ChIdx].Channel));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[AutoChSelBuildChannelListFor5G] ChListNum5G = %d\n", ChListNum5G));
	/* Check for skip-channel list */
	for (ChIdx = 0; ChIdx < ChListNum5G; ChIdx++) {
		pACSChList[ChIdx].SkipChannel = AutoChannelSkipListCheck(pAd, pACSChList[ChIdx].Channel);
#ifdef BACKGROUND_SCAN_SUPPORT
		if (pAd->BgndScanCtrl.SkipDfsChannel)
			pACSChList[ChIdx].SkipChannel = RadarChannelCheck(pAd, pACSChList[ChIdx].Channel);
#endif /* BACKGROUND_SCAN_SUPPORT */
	}

	/* Set parameters (BW/BWCap/CentralChannel/..)of ACS channel list*/
	for (ChIdx = 0; ChIdx < ChListNum5G; ChIdx++) {

		if (cfg_ht_bw == BW_20) {
			pACSChList[ChIdx].Bw = BW_20;
			pACSChList[ChIdx].BwCap = TRUE;
			pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel;
		}

#ifdef DOT11_N_SUPPORT
		else if (((cfg_ht_bw == BW_40)
#ifdef DOT11_VHT_AC
			&& (cfg_vht_bw == VHT_BW_2040)
#endif /* DOT11_VHT_AC */
			 ) && N_ChannelGroupCheck(pAd, pACSChList[ChIdx].Channel, pwdev)) {
			pACSChList[ChIdx].Bw = BW_40;

			/* Check that if there is a secondary channel in current BW40-channel group for BW40 capacity. */
			if ((GetABandChOffset(pACSChList[ChIdx].Channel) == EXT_ABOVE)
				&& (pACSChList[ChIdx + 1].Channel == (pACSChList[ChIdx].Channel + 4)))
				pACSChList[ChIdx].BwCap = TRUE;
			else if ((GetABandChOffset(pACSChList[ChIdx].Channel) == EXT_BELOW)
				&& (pACSChList[ChIdx - 1].Channel == (pACSChList[ChIdx].Channel - 4)))
				pACSChList[ChIdx].BwCap = TRUE;
			else {
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
				if (IS_SUPPORT_V10_DFS(pAd) && pACSChList[ChIdx].Channel == 140)
					pACSChList[ChIdx].BwCap = TRUE;
				else
#endif
				pACSChList[ChIdx].BwCap = FALSE;
			}

			/* Check that whether there is a skip-channel in current BW40-channel group */
			/* If there is a skip-channel in BW40-channel group, just also skip secondary channel */
			if (pACSChList[ChIdx].SkipChannel == TRUE) {
				if ((GetABandChOffset(pACSChList[ChIdx].Channel) == EXT_ABOVE)
					&& (pACSChList[ChIdx + 1].Channel == (pACSChList[ChIdx].Channel + 4)))
					pACSChList[ChIdx + 1].SkipChannel = TRUE;
				else if ((GetABandChOffset(pACSChList[ChIdx].Channel) == EXT_BELOW)
					&& (pACSChList[ChIdx - 1].Channel == (pACSChList[ChIdx].Channel - 4)))
					pACSChList[ChIdx - 1].SkipChannel = TRUE;
			}

			/* Fill in central-channel parameter */
			if (GetABandChOffset(pACSChList[ChIdx].Channel) == EXT_ABOVE)
				pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel + 2;
			else
				pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel - 2;
		}
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
		else if (((cfg_vht_bw == VHT_BW_80) || (cfg_vht_bw == VHT_BW_8080))
				 && vht80_channel_group(pAd, pACSChList[ChIdx].Channel)) {
			pACSChList[ChIdx].Bw = BW_80;
			idx = 0;
			count = 0;

			/* Find out VHT BW80 channel group for current channel */
			while (vht_ch_80M[idx].ch_up_bnd != 0) {
				if ((pACSChList[ChIdx].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
					(pACSChList[ChIdx].Channel <= vht_ch_80M[idx].ch_up_bnd))
					break;
				idx++;
			}

			if (vht_ch_80M[idx].ch_up_bnd != 0) {
				/* Count for secondary channels in current VHT BW80 channel group */
				for (k = 1; k < 4; k++) {
					if ((pACSChList[ChIdx + k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx + k].Channel <= vht_ch_80M[idx].ch_up_bnd))
						count++;

					if ((pACSChList[ChIdx - k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx - k].Channel <= vht_ch_80M[idx].ch_up_bnd))
						count++;
				}

#ifndef ACS_CTCC_SUPPORT
				if (count == 3)
#endif
				{
					pACSChList[ChIdx].BwCap = TRUE;
			}
			}
			/* Check that whether there is a skip-channel in BW80-channel group */
			/* If there is a skip-channel in BW80-channel group, just also skip secondary channels */
			if (pACSChList[ChIdx].SkipChannel == TRUE) {
				for (k = 1; k < 4; k++) {
					if ((pACSChList[ChIdx + k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx + k].Channel <= vht_ch_80M[idx].ch_up_bnd))
						pACSChList[ChIdx + k].SkipChannel = TRUE;

					if ((pACSChList[ChIdx - k].Channel >= vht_ch_80M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx - k].Channel <= vht_ch_80M[idx].ch_up_bnd))
						pACSChList[ChIdx - k].SkipChannel = TRUE;
				}
			}

			pACSChList[ChIdx].CentralChannel = vht_cent_ch_freq(pACSChList[ChIdx].Channel, VHT_BW_80);
		} else if ((cfg_vht_bw == VHT_BW_160)
				   && vht80_channel_group(pAd, pACSChList[ChIdx].Channel)) {
			pACSChList[ChIdx].Bw = BW_160;
			idx = 0;
			count = 0;

			/* Find out VHT BW160 channel group for current channel */
			while (vht_ch_160M[idx].ch_up_bnd != 0) {
				if ((pACSChList[ChIdx].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
					(pACSChList[ChIdx].Channel <= vht_ch_160M[idx].ch_up_bnd))
					break;

				idx++;
			}

			if (vht_ch_160M[idx].ch_up_bnd != 0) {
				/* Count for secondary channels in current VHT BW160 channel group */
				for (k = 1; k < 8; k++) {
					if ((pACSChList[ChIdx + k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx + k].Channel <= vht_ch_160M[idx].ch_up_bnd))
						count++;

					if ((pACSChList[ChIdx - k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx - k].Channel <= vht_ch_160M[idx].ch_up_bnd))
						count++;
				}

				if (count == 7)
					pACSChList[ChIdx].BwCap = TRUE;
			}

			/* Check that whether there is a skip-channel in BW160-channel group */
			/* If there is a skip-channel in BW160-channel group, just also skip secondary channels */
			if (pACSChList[ChIdx].SkipChannel == TRUE) {
				for (k = 1; k < 8; k++) {
					if ((pACSChList[ChIdx + k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx + k].Channel <= vht_ch_160M[idx].ch_up_bnd))
						pACSChList[ChIdx + k].SkipChannel = TRUE;

					if ((pACSChList[ChIdx - k].Channel >= vht_ch_160M[idx].ch_low_bnd) &&
						(pACSChList[ChIdx - k].Channel <= vht_ch_160M[idx].ch_up_bnd))
						pACSChList[ChIdx - k].SkipChannel = TRUE;
				}
			}

			pACSChList[ChIdx].CentralChannel = vht_cent_ch_freq(pACSChList[ChIdx].Channel, VHT_BW_160);
		} else {
			/*The channel is undefined*/
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[AutoChSelBuildChannelListFor5G] The channel:%d is undefined\n", pACSChList[ChIdx].Channel));
			if ((cfg_ht_bw == BW_40) && (cfg_vht_bw == VHT_BW_2040))
				pACSChList[ChIdx].Bw = BW_40;
			else if ((cfg_vht_bw == VHT_BW_80) || (cfg_vht_bw == VHT_BW_8080))
				pACSChList[ChIdx].Bw = BW_80;
			else if (cfg_vht_bw == VHT_BW_160)
				pACSChList[ChIdx].Bw = BW_160;

			pACSChList[ChIdx].BwCap = FALSE;
			pACSChList[ChIdx].CentralChannel = pACSChList[ChIdx].Channel;
		}
#endif /* DOT11_VHT_AC */
	}

	/*Show ACS channel list*/
	for (ChIdx = 0; ChIdx < ChListNum5G; ChIdx++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s:	PrimChannel =  %3d, CenChannel = %3d, BW= %d, BwCap= %d, SkipChannel= %d\n", __func__,
				  pACSChList[ChIdx].Channel, pACSChList[ChIdx].CentralChannel,
				  pACSChList[ChIdx].Bw, pACSChList[ChIdx].BwCap,
				  pACSChList[ChIdx].SkipChannel));
	}

	/*Set channel list of auto channel selection*/
	for (ChIdx = 0; ChIdx < ChListNum5G; ChIdx++) {
		if ((pACSChList[ChIdx].SkipChannel == TRUE) || (pACSChList[ChIdx].BwCap == FALSE))
			continue;
		else {
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Channel = pACSChList[ChIdx].Channel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Bw = pACSChList[ChIdx].Bw;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].BwCap = pACSChList[ChIdx].BwCap;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].CentralChannel = pACSChList[ChIdx].CentralChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].SkipChannel = pACSChList[ChIdx].SkipChannel;
			pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[ChListNum].Flags = pACSChList[ChIdx].Flags;
			ChListNum++;
		}
	}
	pAutoChCtrl->AutoChSelCtrl.ChListNum = ChListNum;

	os_free_mem(pACSChList);
}

/*
	==========================================================================
	Description:

	Return:
		ScanChIdx - Channel index which is mapping to related channel to be scanned.
	Note:
		return -1 if no more next channel
	==========================================================================
 */
CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev,
	IN CHAR LastScanChIdx)
{
	CHAR ScanChIdx = -1;
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	if (LastScanChIdx == -1)
		ScanChIdx = 0;
	else {
		ScanChIdx = LastScanChIdx + 1;

#ifdef ACS_CTCC_SUPPORT
		if (ScanChIdx >= pAutoChCtrl->pChannelInfo->channel_list_num)
#else
		if (ScanChIdx >= pAutoChCtrl->AutoChSelCtrl.ChListNum)
#endif
			ScanChIdx = -1;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s : LastScanChIdx = %d, ScanChIdx = %d, ChannelListNum = %d\n"
			  , __func__, LastScanChIdx, ScanChIdx, pAutoChCtrl->AutoChSelCtrl.ChListNum));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return ScanChIdx;
}

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT8 SelectBestV10Chnl_From_List(
	IN RTMP_ADAPTER *pAd)
{
	UCHAR Best_Channel = 0;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;
	UCHAR i = 0;
	POS_COOKIE	pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT32 ifIndex = pObj->ioctl_if;
	struct wifi_dev *wdev;

	if (pObj->ioctl_if == INT_MBSSID)
		wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
	else
		wdev = &pAd->ApCfg.MBSSID[0].wdev;

	if (IS_DFS_V10_ACS_VALID(pAd)) {
		/* Fetch Channel from ACS Channel List */
		for (i = 0; i < ((wlan_config_get_vht_bw(wdev) == VHT_BW_2040)
			? (V10_TOTAL_CHANNEL_COUNT) : (pDfsParam->GroupCount)); i++) {
			if (pDfsParam->DfsV10SortedACSList[i].isConsumed) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d Consumed\n",
					 __func__, pDfsParam->DfsV10SortedACSList[i].Channel));
				continue;
			}

			if (!CheckNonOccupancyChannel(pAd, wdev, pDfsParam->DfsV10SortedACSList[i].Channel)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]:Channel %d NOP\n",
					 __func__, pDfsParam->DfsV10SortedACSList[i].Channel));
				continue;
			}

			Best_Channel = pDfsParam->DfsV10SortedACSList[i].Channel;
			pDfsParam->DfsV10SortedACSList[i].isConsumed = TRUE;
			break;
		}
	}
	return Best_Channel;
}
#endif


#if defined(OFFCHANNEL_SCAN_FEATURE) || defined (ONDEMAND_DFS)
VOID ChannelInfoResetNew(
	IN PRTMP_ADAPTER	pAd)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrl(pAd);
	CHANNELINFO *ch_info = pAutoChCtrl->pChannelInfo;

	if (ch_info) {
		NdisZeroMemory(ch_info, sizeof(CHANNELINFO));
		pAd->ApCfg.current_channel_index = 0;
	}
}
#endif
/*
	==========================================================================
	Description:Select best channel from Sorted Ranking List of channels

	Return:
		Best_Channel - Channel which is having least channel busy .
	Note:
		return 0 if no channel
	==========================================================================
 */
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined (ONDEMAND_DFS)
UINT8 SelectBestChannel_From_List(RTMP_ADAPTER *pAd, BOOLEAN IsABand, BOOLEAN SkipDFS)
{
	INT i = 0;
	UINT8 Best_Channel = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	if ((!pAd) || (!pAd->sorted_list.size)) {
		if (pAd != NULL)
			MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_ERROR, ("%s return error list size = %d\n",
				__func__, pAd->sorted_list.size));
		return 0;
	}
	for (i = 0; i < pAd->sorted_list.size; i++) {
		if (!SkipDFS) {
			if (RadarChannelCheck(pAd, pAd->sorted_list.SortedMinChannelBusyTimeList[i])) {
				if (pAd->last_selected_channel != pAd->sorted_list.SortedMinChannelBusyTimeList[i]) {
					if (CheckNonOccupancyOnDemandChannel(pAd, pAd->sorted_list.SortedMinChannelBusyTimeList[i])) {
						pAd->last_selected_channel = pAd->sorted_list.SortedMinChannelBusyTimeList[i];
						MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_TRACE,
							("%s Return Best Radar channel = %d \n", __func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
						break;
					} else {
						MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_INFO, ("%s skipping occupied Radar channel = %d\n",
							__func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
						continue;
					}
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_INFO,
						("%s skipping Radar channel = %d because same as last occupied\n",
						 __func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
					continue;
				}
			} else {
				if (pAd->last_selected_channel != pAd->sorted_list.SortedMinChannelBusyTimeList[i]) {
						pAd->last_selected_channel = pAd->sorted_list.SortedMinChannelBusyTimeList[i];
						MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_TRACE,
								("%s Return Best non Radar channel = %d (skip DFS = FALSE)\n",
								 __func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
						break;
					} else {
						MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_INFO,
								("%s skipping  channel = %d because its already last selected\n",
								 __func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
						continue;
					}
			}
		} else {
			if (!RadarChannelCheck(pAd, pAd->sorted_list.SortedMinChannelBusyTimeList[i])) {
				if (pAd->last_selected_channel != pAd->sorted_list.SortedMinChannelBusyTimeList[i]) {
					pAd->last_selected_channel = pAd->sorted_list.SortedMinChannelBusyTimeList[i];
					MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_TRACE, ("%s Return Best non Radar channel = %d \n",
								__func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
					break;
				} else {
					MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_INFO,
							("%s skipping  LAST SELECTED non Radar channel = %d in skip DFS = TRUE mode\n",
							 __func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
					continue;
				}
			} else {
				MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_INFO, ("%s skipping  Radar channel = %d because skip DFS = TRUE\n",
							__func__, pAd->sorted_list.SortedMinChannelBusyTimeList[i]));
				continue;
			}
		}
	}
	if (i < pAd->sorted_list.size) {
		MTWF_LOG(DBG_CAT_ALL, DBG_CAT_ALL, DBG_LVL_TRACE, ("%s return  selected channel = %d skip dfs = %d\n",
					__func__, pAd->last_selected_channel, SkipDFS));
		Best_Channel = pAd->last_selected_channel;
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s failure no channel selected\n", __func__));
		Best_Channel = 0;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return Best_Channel;
}

#endif


/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID AutoChSelScanNextChannel(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	RALINK_TIMER_STRUCT *ScanTimer;
	CHAR Idx;
	UINT32 BusyTime;
	ULONG wait_time = 200; /* Wait for 200 ms at each channel. */
	UCHAR NewCh, BandIdx = HcGetBandByWdev(pwdev);
	INT ret;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
#ifdef TR181_SUPPORT
	struct hdev_obj *hdev = (struct hdev_obj *)pwdev->pHObj;
#endif

	ScanTimer = &pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer;
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
#ifdef AP_QLOAD_SUPPORT

	/* QLOAD ALARM, ever alarm from QLOAD module */
	if (QLOAD_DOES_ALARM_OCCUR(pAd))
		wait_time = 400;

#endif /* AP_QLOAD_SUPPORT */

	if (pAutoChCtrl->AutoChSelCtrl.ScanChIdx == -1) {
#ifdef ACS_CTCC_SUPPORT
		NewCh = select_clear_channel_busy_time(pAd, pwdev);
#else
		NewCh = SelectClearChannelBusyTime(pAd, pwdev);
#endif

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : Current channel = %d , selected new channel = %d\n", __func__, pwdev->channel, NewCh));

#ifdef AP_SCAN_SUPPORT
		scan_ch_restore(pAd, OPMODE_AP, pwdev); /* Restore original channel */
#endif /* AP_SCAN_SUPPORT */

		if (NewCh != pwdev->channel) {
#ifdef ACS_CTCC_SUPPORT
			if (pAd->ApCfg.auto_ch_score_flag == FALSE)
#endif
			{
			ret = rtmp_set_channel(pAd, pwdev, NewCh);
		    if (!ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Fail to set channel !!\n", __func__));
				}
#ifdef TR181_SUPPORT
			else {
				if (hdev->rdev->pRadioCtrl->ACSTriggerFlag == 2) {
					/*ACS triggered by manual command*/
					hdev->rdev->pRadioCtrl->ForceACSChannelChangeCount++;
					hdev->rdev->pRadioCtrl->TotalChannelChangeCount++;
					hdev->rdev->pRadioCtrl->ACSTriggerFlag = 0;
				} else
				if (hdev->rdev->pRadioCtrl->ACSTriggerFlag == 1) {
					/*ACS triggered by periodic refresh*/
					hdev->rdev->pRadioCtrl->RefreshACSChannelChangeCount++;
					hdev->rdev->pRadioCtrl->TotalChannelChangeCount++;
					hdev->rdev->pRadioCtrl->ACSTriggerFlag = 0;
				}
			}
#endif
			 }
		}
		RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, 0);

		pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
		/* Update current state from listen state to idle. */
		pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;
#ifdef ACS_CTCC_SUPPORT
		pAd->ApCfg.auto_ch_score_flag = FALSE;
#endif
		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled[BandIdx] = TRUE;
    } else {
		/* Update current state from idle state to listen. */
		pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_LISTEN;
#ifdef ACS_CTCC_SUPPORT
		wlan_operate_scan(pwdev, pAutoChCtrl->pChannelInfo->supp_ch_list[Idx].channel);
#else
		wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[Idx].Channel);
#endif
		/* Read-Clear reset Channel busy time counter */
		BusyTime = AsicGetChBusyCnt(pAd, BandIdx);

		RTMPSetTimer(ScanTimer, wait_time);
    }

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
    ==========================================================================
    Description:
	Auto-channel selection SCAN req state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM * pElem)
{
	BOOLEAN	Cancelled;
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	struct wifi_dev *pwdev = (struct wifi_dev *)pElem->Priv;
	if (!pwdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AutoChSelScanReqAction - pwdev == NULL \n"));
		return;
	}
	BandIdx = HcGetBandByWdev(pwdev);
	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	RTMPCancelTimer(&pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, &Cancelled);
#ifdef ACS_CTCC_SUPPORT
	APAutoChannelInit(pAd, pwdev);
#endif
	AutoChSelScanNextChannel(pAd, pwdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
    ==========================================================================
    Description:
	Auto-channel selection SCAN timeout state machine procedure
    ==========================================================================
 */
VOID AutoChSelScanTimeoutAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM * pElem)
{
	CHAR Idx;
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	struct wifi_dev *pwdev = (struct wifi_dev *)pElem->Priv;
	if (!pwdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("AutoChSelScanTimeoutAction - pwdev == NULL \n"));
		return;
	}
	BandIdx = HcGetBandByWdev(pwdev);
	pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	UpdateChannelInfo(pAd, Idx, ChannelAlgBusyTime, pwdev);
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = AutoChSelFindScanChIdx(pAd, pwdev, Idx);
	AutoChSelScanNextChannel(pAd, pwdev);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
    ==========================================================================
    Description:
	Scan start handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanStart(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev)
{
	UCHAR BandIdx = HcGetBandByWdev(pwdev);
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	/* Reset ACS ChCtrl*/
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));

	/* Disable MibBucket during doing ACS */
	pAd->MsMibBucket.Enabled = FALSE;
	pAd->OneSecMibBucket.Enabled[BandIdx] = FALSE;

	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0; /* Start from first channel */
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = pwdev;

	if (WMODE_CAP_5G(pwdev->PhyMode))
		pAutoChCtrl->AutoChSelCtrl.IsABand = TRUE;
	else
		pAutoChCtrl->AutoChSelCtrl.IsABand = FALSE;

	AutoChSelBuildChannelList(pAd, pAutoChCtrl->AutoChSelCtrl.IsABand, pwdev);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: IsABand = %d, ChannelListNum = %d\n", __func__, pAutoChCtrl->AutoChSelCtrl.IsABand, pAutoChCtrl->AutoChSelCtrl.ChListNum));
	MlmeEnqueue(pAd, AUTO_CH_SEL_STATE_MACHINE, AUTO_CH_SEL_SCAN_REQ, 0, NULL, (ULONG)pwdev);
	RTMP_MLME_HANDLER(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
    ==========================================================================
    Description:
	Scan timeout handler, executed in timer thread
    ==========================================================================
 */
VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PTIMER_FUNC_CONTEXT pContext = (PTIMER_FUNC_CONTEXT)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pContext->pAd;
	UCHAR BandIdx = pContext->BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	struct wifi_dev *pwdev = pAutoChCtrl->AutoChSelCtrl.pScanReqwdev;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[AutoChSelScanTimeout] - BandIdx = %d\n", BandIdx));
	MlmeEnqueue(pAd, AUTO_CH_SEL_STATE_MACHINE, AUTO_CH_SEL_SCAN_TIMEOUT, 0, NULL, (ULONG)pwdev);
	RTMP_MLME_HANDLER(pAd);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
   ==========================================================================
   Description:
	Auto-channel selection state machine.
   Parameters:
		Sm - pointer to the state machine
   NOTE:
	The state machine is classified as follows:
	a. AUTO_CH_SEL_SCAN_IDLE
	b. AUTO_CH_SEL_SCAN_LISTEN
   ==========================================================================
 */
VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR BandIdx,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{

	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex.pAd = pAd;
	pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex.BandIdx = BandIdx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AUTO_CH_SEL_SCAN_MAX_STATE, AUTO_CH_SEL_SCAN_MAX_MSG,
					 (STATE_MACHINE_FUNC)Drop, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_MACHINE_BASE);
	/* Scan idle state */
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_IDLE, AUTO_CH_SEL_SCAN_REQ, (STATE_MACHINE_FUNC)AutoChSelScanReqAction);
	/* Scan listen state */
	StateMachineSetAction(Sm, AUTO_CH_SEL_SCAN_LISTEN, AUTO_CH_SEL_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)AutoChSelScanTimeoutAction);
	RTMPInitTimer(pAd, &pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, GET_TIMER_FUNCTION(AutoChSelScanTimeout), &pAutoChCtrl->AutoChSelCtrl.ACSTimerFuncContex, FALSE);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
}

/*
   ==========================================================================
   Description:
	Init for auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */
VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BandIdx;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		AutoChSelStateMachineInit(pAd, BandIdx, &pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine, pAutoChCtrl->AutoChSelCtrl.AutoChScanFunc);
	}
}

/*
   ==========================================================================
   Description:
	Release auto-channel selection scan-timer.
   NOTE:
   ==========================================================================
 */
VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN Cancelled;

	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		RTMPReleaseTimer(&pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer, &Cancelled);
	}
}

#ifdef AP_SCAN_SUPPORT
/*
   ==========================================================================
   Description:
       trigger Auto Channel Selection every period of ACSCheckTime.

   NOTE:
		This function is called in a 1-sec mlme periodic check.
		Do ACS only on one HW band at a time.
		Do ACS only when no clients is associated.
   ==========================================================================
 */
VOID AutoChannelSelCheck(RTMP_ADAPTER *pAd)
{
	UCHAR i, WdevBandIdx = 0, BandIdx, HWBandNum;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *pwdev = NULL;

	/* Query current cofigured HW band number */
	HWBandNum = HcGetAmountOfBand(pAd);

	for (BandIdx = 0; BandIdx < HWBandNum; BandIdx++) {
		/* Do nothing if ACSCheckTime is not configured */
		if (pAd->ApCfg.ACSCheckTime[BandIdx] == 0)
			continue;
		pAd->ApCfg.ACSCheckCount[BandIdx]++;

		if (pAd->ApCfg.ACSCheckCount[BandIdx] > pAd->ApCfg.ACSCheckTime[BandIdx]) {

			/* Find wdev, BandIdx of wdev is the same as BandIdx */
			for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
				pwdev = &pAd->ApCfg.MBSSID[i].wdev;
				WdevBandIdx = HcGetBandByWdev(pwdev);
				if (BandIdx == WdevBandIdx)
					break;
			}

			/* Do nothing if AP is doing channel scanning */
			if (ApScanRunning(pAd, pwdev))
				continue;

			/* Reset Counter */
			pAd->ApCfg.ACSCheckCount[BandIdx] = 0;

			/* Do Auto Channel Selection only when no client is associated in current band */
			if (pAd->MacTab.Size != 0) {
				for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if ((pEntry->wdev != NULL) && (IS_ENTRY_CLIENT(pEntry))) {
						WdevBandIdx = HcGetBandByWdev(pEntry->wdev);

						if (BandIdx == WdevBandIdx) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\x1b[41m%s(): Ignore ACS checking because has associated clients in current band%d\x1b[m\n",
							__func__, BandIdx));

							return;
						}
					}
				}
			}
			/* Start for ACS checking */
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("%s(): Scanning channels for channel selection.\n", __func__));

				if (pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime) {
#ifdef TR181_SUPPORT
					{
						struct hdev_obj *hdev = (struct hdev_obj *)pwdev->pHObj;

						/*set ACS trigger flag to periodic refresh trigger*/
						hdev->rdev->pRadioCtrl->ACSTriggerFlag = 1;
					}
#endif
					AutoChSelScanStart(pAd, pwdev);
				}
				else
					ApSiteSurvey_by_wdev(pAd, NULL, SCAN_PASSIVE, TRUE, pwdev);
				return;
			}
		}
	}
}
#endif /* AP_SCAN_SUPPORT */

