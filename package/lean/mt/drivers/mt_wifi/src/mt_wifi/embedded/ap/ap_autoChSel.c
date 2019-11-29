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


extern UCHAR ZeroSsid[32];

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;
#ifdef AP_SCAN_SUPPORT
extern INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev);
#endif/*AP_SCAN_SUPPORT*/

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

	if (pAutoChCtrl->pChannelInfo != NULL) {
		UINT32 BusyTime;

		if (Alg == ChannelAlgCCA) {
			UINT32 cca_cnt = AsicGetCCACnt(pAd);

			pAd->RalinkCounters.OneSecFalseCCACnt += cca_cnt;
			pAutoChCtrl->pChannelInfo->FalseCCA[ch_index] = cca_cnt;
		}

		/*
			do busy time statistics for primary channel
			scan time 200ms, beacon interval 100 ms
		*/
		BusyTime = AsicGetChBusyCnt(pAd, BandIdx);

#ifdef AP_QLOAD_SUPPORT
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / AUTO_CHANNEL_SEL_TIMEOUT;
#else
		pAutoChCtrl->pChannelInfo->chanbusytime[ch_index] = (BusyTime * 100) / 200;
#endif /* AP_QLOAD_SUPPORT */
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

	for (ChannelIdx = StartChannelIdx; ChannelIdx < pAutoChCtrl->AutoChSelCtrl.ChListNum; ChannelIdx++) {
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
		wlan_operate_set_cen_ch_2(wdev, cen_ch_2);
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
		ch = SelectClearChannelBusyTime(pAd, pwdev);
		break;

	default:
		ch = SelectClearChannelBusyTime(pAd, pwdev);
		break;
	}

	RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, ch);
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
	pAutoChCtrl->AutoChSelCtrl.SelCh = ch;
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

	for (i = 0; i < pAutoChCtrl->AutoChSelCtrl.ChListNum; i++) {
		ULONG wait_time = 200; /* Wait for 200 ms at each channel. */

		wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[i].Channel);
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

	if (IsABand) {
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
		if (CheckNonOccupancyChannel(pAd, pwdev)) {
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
			else
				pACSChList[ChIdx].BwCap = FALSE;

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

				if (count == 3)
					pACSChList[ChIdx].BwCap = TRUE;
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

		if (ScanChIdx >= pAutoChCtrl->AutoChSelCtrl.ChListNum)
			ScanChIdx = -1;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s : LastScanChIdx = %d, ScanChIdx = %d, ChannelListNum = %d\n"
			  , __func__, LastScanChIdx, ScanChIdx, pAutoChCtrl->AutoChSelCtrl.ChListNum));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n", __func__));
	return ScanChIdx;
}

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
	ScanTimer = &pAutoChCtrl->AutoChSelCtrl.AutoChScanTimer;
	Idx = pAutoChCtrl->AutoChSelCtrl.ScanChIdx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n", __func__));
#ifdef AP_QLOAD_SUPPORT

	/* QLOAD ALARM, ever alarm from QLOAD module */
	if (QLOAD_DOES_ALARM_OCCUR(pAd))
		wait_time = 400;

#endif /* AP_QLOAD_SUPPORT */

	if (pAutoChCtrl->AutoChSelCtrl.ScanChIdx == -1) {
		NewCh = SelectClearChannelBusyTime(pAd, pwdev);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : Current channel = %d , selected new channel = %d\n", __func__, pwdev->channel, NewCh));

#ifdef AP_SCAN_SUPPORT
		scan_ch_restore(pAd, OPMODE_AP, pwdev); /* Restore original channel */
#endif /* AP_SCAN_SUPPORT */

		if (NewCh != pwdev->channel) {
			ret = rtmp_set_channel(pAd, pwdev, NewCh);
		    if (!ret) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Fail to set channel !! \n", __func__));
		    }
		}

		pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_SELECTED;
		/* Update current state from listen state to idle. */
		pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_IDLE;

		/* Enable MibBucket after ACS done */
		pAd->MsMibBucket.Enabled = TRUE;
		pAd->OneSecMibBucket.Enabled[BandIdx] = TRUE;
    } else {
		/* Update current state from idle state to listen. */
		pAutoChCtrl->AutoChSelCtrl.AutoChScanStatMachine.CurrState = AUTO_CH_SEL_SCAN_LISTEN;

		wlan_operate_scan(pwdev, pAutoChCtrl->AutoChSelCtrl.AutoChSelChList[Idx].Channel);

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

				if (pAd->ApCfg.AutoChannelAlg == ChannelAlgBusyTime)
					AutoChSelScanStart(pAd, pwdev);
				else
					ApSiteSurvey_by_wdev(pAd, NULL, SCAN_PASSIVE, TRUE, pwdev);
				return;
			}
		}
	}
}
#endif /* AP_SCAN_SUPPORT */

