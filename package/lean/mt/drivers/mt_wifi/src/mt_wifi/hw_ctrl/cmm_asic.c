/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_asic.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "mcu/mt_cmd.h"
#endif
#include "hdev/hdev.h"
#define MCAST_WCID_TO_REMOVE 0 /* Pat: TODO */


static char *hif_2_str[] = {"HIF_RTMP", "HIF_RLT", "HIF_MT", "Unknown"};
VOID AsicNotSupportFunc(RTMP_ADAPTER *pAd, const RTMP_STRING *caller)
{
	RTMP_STRING *str;
	INT32 hif_type = GET_HIF_TYPE(pAd);

	if (hif_type <= HIF_MAX)
		str = hif_2_str[hif_type];
	else
		str = hif_2_str[HIF_MAX];

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NotSupportedFunc for this arch(%s)!\n",
			 caller, str));
}

#ifndef	COMPOS_TESTMODE_WIN
UINT32 AsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetCrcErrCnt(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetCCACnt(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetCCACnt(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


UINT32 AsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetChBusyCnt(pAd, BandIdx);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}




INT AsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetAutoFallBack(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicAutoFallbackInit(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicUpdateRtsThld(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	UINT32 pkt_num,
	UINT32 length,
	UINT32 retry_limit)
{

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			return;

#endif /* CONFIG_ATE */

		if (pAd->archOps.archUpdateRtsThld)
			return pAd->archOps.archUpdateRtsThld(pAd, wdev, pkt_num, length,
								retry_limit);
	}

	AsicNotSupportFunc(pAd, __func__);
}


/*
 * ========================================================================
 *
 * Routine Description:
 * Set MAC register value according operation mode.
 * OperationMode AND bNonGFExist are for MM and GF Proteciton.
 * If MM or GF mask is not set, those passing argument doesn't not take effect.
 *
 * Operation mode meaning:
 * = 0 : Pure HT, no preotection.
 * = 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
 * = 0x10: No Transmission in 40M is protected.
 * = 0x11: Transmission in both 40M and 20M shall be protected
 * if (bNonGFExist)
 * we should choose not to use GF. But still set correct ASIC registers.
 * ========================================================================
 */
VOID AsicUpdateProtect(
	IN PRTMP_ADAPTER pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			return;

#endif /* CONFIG_ATE */
		HwCtrlSetFlag(pAd, HWFLAG_ID_UPDATE_PROTECT);
		return;
	}

#endif
}

/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR band_idx, struct freq_oper *oper, BOOLEAN bScan)
{
#ifdef CUSTOMER_DCC_FEATURE
	if (!(ApScanRunning(pAd, NULL)) && pAd->ApEnableBeaconTable)
		BssTableInit(&pAd->AvailableBSS);
#endif
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_SWITCH_CHANNEL_CFG SwChCfg;
		os_zero_mem(&SwChCfg, sizeof(MT_SWITCH_CHANNEL_CFG));
		SwChCfg.bScan = bScan;
		SwChCfg.CentralChannel = oper->cen_ch_1;
		SwChCfg.BandIdx = band_idx;

		SwChCfg.RxStream = pAd->Antenna.field.RxPath;
		SwChCfg.TxStream = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode) {
			if (SwChCfg.BandIdx == DBDC_BAND0) {
				SwChCfg.RxStream = pAd->dbdc_band0_rx_path;
				SwChCfg.TxStream = pAd->dbdc_band0_tx_path;
			} else {
				SwChCfg.RxStream = pAd->dbdc_band1_rx_path;
				SwChCfg.TxStream = pAd->dbdc_band1_tx_path;
			}
		}
#endif

		SwChCfg.Bw = oper->bw;
		SwChCfg.ControlChannel = oper->prim_ch;
		SwChCfg.OutBandFreq = 0;
#ifdef DOT11_VHT_AC
		SwChCfg.ControlChannel2 = oper->cen_ch_2;
#endif /* DOT11_VHT_AC */
#ifdef MT_DFS_SUPPORT
		SwChCfg.bDfsCheck = DfsSwitchCheck(pAd, SwChCfg.ControlChannel, band_idx);
#endif

		/*update radio info to band*/
		if (!bScan) {
		}

		MtAsicSwitchChannel(pAd, SwChCfg);
	}

#endif
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */


VOID AsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicResetBBPAgent(pAd);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}





#endif/*COMPOS_TESTMODE_WIN*/

/* Replaced by AsicDevInfoUpdate() */
INT AsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
			 __func__, PRINT_MAC(addr)));
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetDevMac(pAd, omac_idx, addr, 0, TRUE, DEVINFO_ACTIVE_FEATURE);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifndef	COMPOS_TESTMODE_WIN
#ifdef CONFIG_AP_SUPPORT
VOID AsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetMbssMode(pAd, NumOfBcns);
		return;
	}

#endif
}
#endif /* CONFIG_AP_SUPPORT */


BOOLEAN AsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev)
{
	if (pAd->archOps.archDisableBeacon)
		return pAd->archOps.archDisableBeacon(pAd, wdev);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev)
{
	if (pAd->archOps.archEnableBeacon)
		return pAd->archOps.archEnableBeacon(pAd, wdev);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicUpdateBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev, UINT16 FrameLen, UCHAR UpdatePktType)
{
	if (pAd->archOps.archUpdateBeacon)
		return pAd->archOps.archUpdateBeacon(pAd, wdev, FrameLen, UpdatePktType);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT AsicSetReptFuncEnable(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	if (pAd->archOps.archSetReptFuncEnable) {
		if (enable)
			RepeaterCtrlInit(pAd);
		else
			RepeaterCtrlExit(pAd);

		return pAd->archOps.archSetReptFuncEnable(pAd, enable);
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicInsertRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr)
{
	if (pAd->archOps.archInsertRepeaterEntry)
		pAd->archOps.archInsertRepeaterEntry(pAd, CliIdx, pAddr);
	else
		AsicNotSupportFunc(pAd, __func__);
}


VOID AsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
	if (pAd->archOps.archRemoveRepeaterEntry)
		pAd->archOps.archRemoveRepeaterEntry(pAd, CliIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}


VOID AsicInsertRepeaterRootEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	if (pAd->archOps.archInsertRepeaterRootEntry)
		pAd->archOps.archInsertRepeaterRootEntry(pAd, Wcid, pAddr, ReptCliIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT AsicSetRxFilter(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_RX_FILTER_CTRL_T RxFilter;
		os_zero_mem(&RxFilter, sizeof(MT_RX_FILTER_CTRL_T));
		if (FALSE)
		{
			RxFilter.bPromiscuous = TRUE;
		} else {
			RxFilter.bPromiscuous = FALSE;
			RxFilter.bFrameReport = FALSE;
			RxFilter.filterMask = RX_NDPA | RX_NOT_OWN_BTIM | RX_NOT_OWN_UCAST |
								  RX_RTS | RX_CTS | RX_CTRL_RSV | RX_BC_MC_DIFF_BSSID_A2 |
								  RX_BC_MC_DIFF_BSSID_A3 | RX_BC_MC_OWN_MAC_A3 | RX_PROTOCOL_VERSION |
								  RX_FCS_ERROR;
		} /*Endof Monitor ON*/

		if (pAd->archOps.archSetRxFilter) {
			INT ret = 0;
			ret = pAd->archOps.archSetRxFilter(pAd, RxFilter);
			return ret;
		}

		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef DOT11_N_SUPPORT
INT AsicSetRDG(RTMP_ADAPTER *pAd,
			   UCHAR wlan_idx,
			   UCHAR band_idx,
			   UCHAR init,
			   UCHAR resp)
{
	INT ret = FALSE;
	INT bSupport = FALSE;
	BOOLEAN is_en;
	is_en = (init && resp) ? TRUE : FALSE;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_RDG_CTRL_T   rdg;
		RTMP_ARCH_OP    *arch_op = &pAd->archOps;

		if (arch_op->archSetRDG) {
			bSupport = TRUE;
			rdg.WlanIdx = wlan_idx;
			rdg.BandIdx = band_idx;
			rdg.Init = init;
			rdg.Resp = resp;
			rdg.Txop = (is_en) ? (0x80) : (0x60);
			rdg.LongNav = (is_en) ? (1) : (0);
			ret = arch_op->archSetRDG(pAd, &rdg);
		}
	}

#endif

	if (ret == TRUE) {
		if (is_en)
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
		else
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE);
	}

	if (!bSupport)
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}
#endif /* DOT11_N_SUPPORT */


/*
 * ========================================================================
 * Routine Description:
 * Set/reset MAC registers according to bPiggyBack parameter
 *
 * Arguments:
 * pAd         - Adapter pointer
 * bPiggyBack  - Enable / Disable Piggy-Back
 *
 * Return Value:
 * None
 *
 * ========================================================================
 */
VOID AsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetPiggyBack(pAd, bPiggyBack);

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT AsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx)
{
	if (pAd->archOps.archSetPreTbtt) {
		pAd->archOps.archSetPreTbtt(pAd, enable, HwBssidIdx);
		return TRUE;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetGPTimer(pAd, enable, timeout);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetChBusyStat(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef LINUX
#ifdef RTMP_WLAN_HOOK_SUPPORT
EXPORT_SYMBOL(AsicGetTsfTime);
#endif /* RTMP_WLAN_HOOK_SUPPORT */
#endif /* LINUX */
INT AsicGetTsfTime(
	RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx)
{
	if (pAd->archOps.archGetTsfTime)
		return pAd->archOps.archGetTsfTime(pAd, high_part, low_part, HwBssidIdx);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicDisableSync(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	if (pAd->archOps.archDisableSync)
		pAd->archOps.archDisableSync(pAd, HWBssidIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetSyncModeAndEnable(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode)
{
	if (pAd->archOps.archSetSyncModeAndEnable)
		pAd->archOps.archSetSyncModeAndEnable(pAd, BeaconPeriod, HWBssidIdx, OPMode);
	else
		AsicNotSupportFunc(pAd, __func__);
}




INT AsicSetWmmParam(RTMP_ADAPTER *pAd, UCHAR idx, UINT32 ac, UINT32 type, UINT32 val)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetWmmParam(pAd, idx, ac, type, val);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return NDIS_STATUS_FAILURE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm, struct wifi_dev *wdev)
{
	INT i;
	UCHAR EdcaIdx = wdev->EdcaIdx;
	EDCA_PARM *pEdca = NULL;

	if (EdcaIdx >= WMM_NUM_OF_AC) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): EdcaIdx >= 4\n", __func__));
		return;
	}

	pEdca = &pAd->CommonCfg.APEdcaParm[EdcaIdx];

	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NoEDCAParam\n", __func__));
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WMM_INUSED);

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			if (IS_ENTRY_CLIENT(&pAd->MacTab.Content[i]) ||
				IS_ENTRY_APCLI(&pAd->MacTab.Content[i]) ||
				IS_ENTRY_REPEATER(&pAd->MacTab.Content[i]))
				CLIENT_STATUS_CLEAR_FLAG(&pAd->MacTab.Content[i], fCLIENT_STATUS_WMM_CAPABLE);
		}

		os_zero_mem(pEdca, sizeof(EDCA_PARM));
	} else {
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WMM_INUSED);
		os_move_mem(pEdca, pEdcaParm, sizeof(EDCA_PARM));

		if (!ADHOC_ON(pAd)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("EDCA [#%d]: AIFSN CWmin CWmax  TXOP(us)  ACM, WMM Set: %d, BandIdx: %d\n",
					  pEdcaParm->EdcaUpdateCount,
					  pEdcaParm->WmmSet,
					  pEdcaParm->BandIdx));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_BE      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[0],
					 pEdcaParm->Cwmin[0],
					 pEdcaParm->Cwmax[0],
					 pEdcaParm->Txop[0] << 5,
					 pEdcaParm->bACM[0]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_BK      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[1],
					 pEdcaParm->Cwmin[1],
					 pEdcaParm->Cwmax[1],
					 pEdcaParm->Txop[1] << 5,
					 pEdcaParm->bACM[1]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_VI      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[2],
					 pEdcaParm->Cwmin[2],
					 pEdcaParm->Cwmax[2],
					 pEdcaParm->Txop[2] << 5,
					 pEdcaParm->bACM[2]));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("     AC_VO      %2d     %2d     %2d      %4d     %d\n",
					 pEdcaParm->Aifsn[3],
					 pEdcaParm->Cwmin[3],
					 pEdcaParm->Cwmax[3],
					 pEdcaParm->Txop[3] << 5,
					 pEdcaParm->bACM[3]));
		}
#ifdef APCLI_CERT_SUPPORT
		ApCliCertEDCAAdjust(pAd, wdev, pEdcaParm);
#endif

	}

#ifdef VOW_SUPPORT
	vow_update_om_wmm(pAd, wdev, pEdcaParm);
#endif /* VOW_SUPPORT */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetEdcaParm(pAd, pEdcaParm);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		UINT32 TxSuccess = 0;
		MtAsicTxCntUpdate(pAd, Wcid, pTxInfo);
		TxSuccess = pTxInfo->TxCount - pTxInfo->TxFailCount;

		if (pTxInfo->TxFailCount == 0)
			pAd->RalinkCounters.OneSecTxNoRetryOkCount += pTxInfo->TxCount;
		else
			pAd->RalinkCounters.OneSecTxRetryOkCount += pTxInfo->TxCount;

		pAd->RalinkCounters.OneSecTxFailCount += pTxInfo->TxFailCount;
#ifdef STATS_COUNT_SUPPORT
		pAd->WlanCounters[0].TransmittedFragmentCount.u.LowPart += TxSuccess;
		pAd->WlanCounters[0].FailedCount.u.LowPart += pTxInfo->TxFailCount;
#endif /* STATS_COUNT_SUPPORT */
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicRssiUpdate(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC
	MAC_TABLE_ENTRY *pEntry;
	CHAR RssiSet[3];
	INT i = 0;
	NdisZeroMemory(RssiSet, sizeof(RssiSet));

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->MacTab.Size == 0) {
				pEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
				MtAsicRssiGet(pAd, pEntry->wcid, &RssiSet[0]);

				for (i = 0; i < 3; i++) {
					pEntry->RssiSample.AvgRssi[i] = RssiSet[i];
					pEntry->RssiSample.LastRssi[i] = RssiSet[i];
					pAd->ApCfg.RssiSample.AvgRssi[i] = RssiSet[i];
					pAd->ApCfg.RssiSample.LastRssi[i] = RssiSet[i];
				}
			} else {
				INT32 TotalRssi[3];
				INT j;
				NdisZeroMemory(TotalRssi, sizeof(TotalRssi));

				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if (IS_VALID_ENTRY(pEntry)) {
						MtAsicRssiGet(pAd, pEntry->wcid, &RssiSet[0]);

						for (j = 0; j < 3; j++) {
							pEntry->RssiSample.AvgRssi[j] = RssiSet[j];
							pEntry->RssiSample.LastRssi[j] = RssiSet[j];
							TotalRssi[j] += RssiSet[j];
						}
					}
				}

				for (i = 0; i < 3; i++)
					pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	return;
#endif
}

INT AsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetRetryLimit(pAd, type, limit);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


UINT32 AsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicGetRetryLimit(pAd, type);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ==========================================================================
 * Description:
 *
 * IRQL = PASSIVE_LEVEL
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetSlotTime(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bUseShortSlotTime,
	IN UCHAR channel,
	IN struct wifi_dev *wdev)
{
	UINT32 SlotTime = 0;
	UINT32 SifsTime = SIFS_TIME_24G;
	UCHAR BandIdx;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (bUseShortSlotTime && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
			return;
		else if ((!bUseShortSlotTime) && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED)))
			return;

		if (bUseShortSlotTime)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);
		else
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED);

		SlotTime = (bUseShortSlotTime) ? 9 : 20;
	}
#endif
	BandIdx = HcGetBandByChannel(pAd, channel);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetSlotTime(pAd, SlotTime, SifsTime, BandIdx);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetMacMaxLen) {
			INT ret = 0;
			ret = pAd->archOps.archSetMacMaxLen(pAd);
			return ret;
		}

		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


VOID AsicGetTxTsc(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archGetTxTsc)
			pAd->archOps.archGetTxTsc(pAd, wdev, pTxTsc);

		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetSMPS)
			pAd->archOps.archSetSMPS(pAd, Wcid, smps);
		else
			AsicNotSupportFunc(pAd, __func__);

		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}



/*
 * ========================================================================
 * Description:
 * Add Shared key information into ASIC.
 * Update shared key, TxMic and RxMic to Asic Shared key table
 * Update its cipherAlg to Asic Shared key Mode.
 *
 * Return:
 * ========================================================================
 */
VOID AsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BssIndex,
	IN UCHAR			KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicAddSharedKeyEntry(pAd, BssIndex, KeyIdx, pCipherKey);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


/*	IRQL = DISPATCH_LEVEL*/
VOID AsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicRemoveSharedKeyEntry(pAd, BssIndex, KeyIdx);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}



#ifdef MCS_LUT_SUPPORT
VOID AsicMcsLutUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	AsicNotSupportFunc(pAd, __func__);
}
#endif /* MCS_LUT_SUPPORT */


UINT16 AsicGetTidSn(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archGetTidSn)
			return pAd->archOps.archGetTidSn(pAd, wcid, tid);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0xffff;
}


VOID AsicUpdateBASession(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
#ifdef MT_MAC
	RETURN_IF_PAD_NULL(pAd);

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MAC_TABLE_ENTRY *mac_entry;
		MT_BA_CTRL_T BaCtrl;
		STA_REC_BA_CFG_T StaRecBaCfg;
		VOID *pBaEntry;
		UINT32 i;
		os_zero_mem(&BaCtrl, sizeof(MT_BA_CTRL_T));
		mac_entry = &pAd->MacTab.Content[wcid];
		BaCtrl.BaSessionType = ses_type;
		BaCtrl.BaWinSize = basize;
		BaCtrl.isAdd = isAdd;
		BaCtrl.Sn = sn;
		BaCtrl.Wcid = wcid;
		BaCtrl.Tid = tid;

		if (mac_entry && mac_entry->wdev) {
			BaCtrl.band_idx = HcGetBandByWdev(mac_entry->wdev);
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s(): mac_entry=%p!mac_entry->wdev=%p, Set BaCtrl.band_idx=%d\n",
					  __func__, mac_entry, mac_entry->wdev, BaCtrl.band_idx));
		} else {
			BaCtrl.band_idx = 0;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s(): mac_entry=%p!Set BaCtrl.band_idx=%d\n",
					  __func__, mac_entry, BaCtrl.band_idx));
		}

		if (ses_type == BA_SESSION_RECP) {
			/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
			if (isAdd)
				os_move_mem(&BaCtrl.PeerAddr[0], &mac_entry->Addr[0], MAC_ADDR_LEN);
		}

		if (pAd->archOps.archUpdateBASession) {
			pAd->archOps.archUpdateBASession(pAd, BaCtrl);

			if (pAd->archOps.archUpdateStaRecBa) {
				if (!mac_entry  || !mac_entry->wdev)
					return;

				StaRecBaCfg.baDirection = ses_type;

				if (ses_type == ORI_BA) {
					i = mac_entry->BAOriWcidArray[tid];

					if (pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
						pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;

					pBaEntry = &pAd->BATable.BAOriEntry[i];
				} else {
					i = mac_entry->BARecWcidArray[tid];

					if (pAd->CommonCfg.dbdc_mode && !WMODE_CAP_AC(mac_entry->wdev->PhyMode))
						pAd->BATable.BAOriEntry[i].amsdu_cap = FALSE;

					pBaEntry = &pAd->BATable.BARecEntry[i];
				}

				StaRecBaCfg.BaEntry = pBaEntry;
				StaRecBaCfg.BssIdx = mac_entry->wdev->bss_info_argument.ucBssIndex;

				if (IS_ENTRY_REPEATER(mac_entry))
					StaRecBaCfg.MuarIdx = pAd->MacTab.tr_entry[mac_entry->wcid].OmacIdx;
				else
					StaRecBaCfg.MuarIdx = mac_entry->wdev->OmacIdx;

				StaRecBaCfg.tid = tid;
				StaRecBaCfg.BaEnable = (isAdd << tid);
				StaRecBaCfg.WlanIdx = wcid;
				pAd->archOps.archUpdateStaRecBa(pAd, StaRecBaCfg);
			}

			return;
		}

		AsicNotSupportFunc(pAd, __func__);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

VOID AsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr, BOOLEAN IsBCMCWCID, BOOLEAN IsReset)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_WCID_TABLE_INFO_T WtblInfo;
		MAC_TABLE_ENTRY *mac_entry = NULL;
		struct _STA_TR_ENTRY *tr_entry = NULL;
		os_zero_mem(&WtblInfo, sizeof(MT_WCID_TABLE_INFO_T));
		WtblInfo.Wcid = WCID;
		WtblInfo.IsReset = IsReset;
		os_move_mem(&WtblInfo.Addr[0], &pAddr[0], 6);

		if (VALID_UCAST_ENTRY_WCID(pAd, WCID))
			mac_entry = &pAd->MacTab.Content[WCID];

		if ((IsBCMCWCID == TRUE) || WCID == MAX_LEN_OF_MAC_TABLE) {
			/* BC Mgmt or BC/MC data */
			WtblInfo.MacAddrIdx = 0xe;
			WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
			WtblInfo.CipherSuit = WTBL_CIPHER_NONE;

			if (HcGetWcidLinkType(pAd, WCID) == WDEV_TYPE_APCLI)
				WtblInfo.WcidType = MT_WCID_TYPE_APCLI_MCAST;
		} else if (mac_entry) {
			if (IS_ENTRY_CLIENT(mac_entry)) {
				/* FIXME: will fix this when set entry fix for sta mode */
				if (mac_entry->wdev->wdev_type == WDEV_TYPE_AP)
					WtblInfo.WcidType = MT_WCID_TYPE_CLI;
				else if (mac_entry->wdev->wdev_type == WDEV_TYPE_STA)
					WtblInfo.WcidType = MT_WCID_TYPE_AP;
			} else if (IS_ENTRY_APCLI(mac_entry))
				WtblInfo.WcidType = MT_WCID_TYPE_APCLI;
			else if (IS_ENTRY_REPEATER(mac_entry))
				WtblInfo.WcidType = MT_WCID_TYPE_REPEATER;
			else if (IS_ENTRY_WDS(mac_entry))
				WtblInfo.WcidType = MT_WCID_TYPE_WDS;
			else
				WtblInfo.WcidType = MT_WCID_TYPE_CLI;

			if (IS_ENTRY_REPEATER(mac_entry)) {
				tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
				WtblInfo.MacAddrIdx = tr_entry->OmacIdx;
			} else
				WtblInfo.MacAddrIdx = mac_entry->wdev->OmacIdx;

			WtblInfo.Aid = mac_entry->Aid;
#ifdef TXBF_SUPPORT
			WtblInfo.PfmuId    = pAd->rStaRecBf.u2PfmuId;

			if (IS_HT_STA(mac_entry)) {
				WtblInfo.fgTiBf    = (mac_entry->rStaRecBf.fgETxBfCap > 0) ? FALSE : TRUE;
				WtblInfo.fgTiBf    = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTiBf : FALSE;
				WtblInfo.fgTeBf    = mac_entry->rStaRecBf.fgETxBfCap;
				WtblInfo.fgTeBf    = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTeBf : FALSE;
			}

			if (IS_VHT_STA(mac_entry)) {
				WtblInfo.fgTibfVht = (mac_entry->rStaRecBf.fgETxBfCap > 0) ? FALSE : TRUE;
				WtblInfo.fgTibfVht = (pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE) ? WtblInfo.fgTibfVht : FALSE;
				WtblInfo.fgTebfVht = mac_entry->rStaRecBf.fgETxBfCap;
				WtblInfo.fgTebfVht = (pAd->CommonCfg.ETxBfEnCond == TRUE) ? WtblInfo.fgTebfVht : FALSE;
			}

#endif

			if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)
				&& CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RALINK_CHIPSET))
				WtblInfo.aad_om = 1;

			if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE))
				WtblInfo.SupportQoS = TRUE;

			if (IS_HT_STA(mac_entry)) {
				WtblInfo.SupportHT = TRUE;

				if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE))
					WtblInfo.SupportRDG = TRUE;

				WtblInfo.SmpsMode = mac_entry->MmpsMode;
				WtblInfo.MpduDensity = mac_entry->MpduDensity;
				WtblInfo.MaxRAmpduFactor = mac_entry->MaxRAmpduFactor;

				if (IS_VHT_STA(mac_entry)) {
					WtblInfo.SupportVHT = TRUE;
					WtblInfo.dyn_bw = wlan_config_get_vht_bw_sig(mac_entry->wdev);
#ifdef TXBF_SUPPORT
					WtblInfo.gid = 63;
#endif
				}
			}

			if (IS_CIPHER_TKIP_Entry(mac_entry)) {
				WtblInfo.DisRHTR = 1;
#ifdef A4_CONN
				if (IS_ENTRY_A4(mac_entry))
					WtblInfo.DisRHTR = 0;
#endif
			}

#ifdef A4_CONN
			WtblInfo.a4_enable = IS_ENTRY_A4(mac_entry);

			if (IS_ENTRY_A4(mac_entry))
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("AsicUpdateRxWCIDTable: Enable A4 in WTBLinfo\n"));

#endif
#ifdef DOT11R_FT_SUPPORT
			if (IS_FT_STA(mac_entry)) {
				WtblInfo.SkipClearPrevSecKey = TRUE;
			}
#endif /* DOT11R_FT_SUPPORT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
	if (mac_entry->wdev->wds_enable)
		WtblInfo.fg4AddrEnable = mac_entry->bEnable4Addr;
#endif

		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():mac_entry is NULL!\n", __func__));

		if (pAd->archOps.archUpdateRxWCIDTable)
			return pAd->archOps.archUpdateRxWCIDTable(pAd, WtblInfo);

		AsicNotSupportFunc(pAd, __func__);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


#ifdef TXBF_SUPPORT
VOID AsicUpdateClientBfCap(RTMP_ADAPTER *pAd, PMAC_TABLE_ENTRY pMacEntry)
{
#ifdef MT_MAC
	if (pAd->archOps.archUpdateClientBfCap)
		return pAd->archOps.archUpdateClientBfCap(pAd, pMacEntry);
#endif
	AsicNotSupportFunc(pAd, __func__);
}
#endif /* TXBF_SUPPORT */



/*
 * ==========================================================================
 * Description:
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archDelWcidTab)
			return pAd->archOps.archDelWcidTab(pAd, wcid_idx);

		AsicNotSupportFunc(pAd, __func__);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


#ifdef HTC_DECRYPT_IOT
VOID AsicSetWcidAAD_OM(RTMP_ADAPTER *pAd, UCHAR wcid_idx, CHAR value)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetWcidAAD_OM)
			return pAd->archOps.archSetWcidAAD_OM(pAd, wcid_idx, value);

		AsicNotSupportFunc(pAd, __func__);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}
#endif /* HTC_DECRYPT_IOT */

#ifdef MBSS_AS_WDS_AP_SUPPORT
VOID AsicSetWcid4Addr_HdrTrans(RTMP_ADAPTER *pAd, UCHAR wcid_idx, UCHAR IsEnable)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[wcid_idx];
	UCHAR IsApcliEntry = 0;
	if (IS_ENTRY_APCLI(pEntry))
		IsApcliEntry = 1;
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetWcid4Addr_HdrTrans) {
			
			return pAd->archOps.archSetWcid4Addr_HdrTrans(pAd, wcid_idx, IsEnable, IsApcliEntry);
		}
		else {
			AsicNotSupportFunc(pAd, __FUNCTION__);
			return;
		}
	}
#endif

	AsicNotSupportFunc(pAd, __FUNCTION__);
	return;
}
#endif


VOID AsicAddRemoveKeyTab(
	IN PRTMP_ADAPTER pAd,
	IN ASIC_SEC_INFO *pInfo)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)
		&& (pAd->archOps.archAddRemoveKeyTab))
		return pAd->archOps.archAddRemoveKeyTab(pAd, pInfo);

#endif
	AsicNotSupportFunc(pAd, __func__);
}


INT AsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSendCmdToMcuAndWait(pAd, Command, Token, Arg0, Arg1, in_atomic);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSendCommandToMcuBBP(pAd, Command, Token, Arg0, Arg1, FlgIsNeedLocked);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ========================================================================
 * Description:
 * For 1x1 chipset : 2070 / 3070 / 3090 / 3370 / 3390 / 5370 / 5390
 * Usage :      1. Set Default Antenna as initialize
 * 2. Antenna Diversity switching used
 * 3. iwpriv command switch Antenna
 *
 * Return:
 * ========================================================================
 */






#ifdef STREAM_MODE_SUPPORT
/* StreamModeRegVal - return MAC reg value for StreamMode setting */
UINT32 StreamModeRegVal(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtStreamModeRegVal(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


/*
 * ========================================================================
 * Description:
 * configure the stream mode of specific MAC or all MAC and set to ASIC.
 *
 * Prameters:
 * pAd           ---
 * pMacAddr ---
 * bClear        --- disable the stream mode for specific macAddr when
 * (pMacAddr!=NULL)
 *
 * Return:
 * ========================================================================
 */
VOID AsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicSetStreamMode(pAd, pMacAddr, chainIdx, bEnabled);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}


VOID AsicStreamModeInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MtAsicStreamModeInit(pAd);
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}
#endif /* STREAM_MODE_SUPPORT // */


#ifdef DOT11_N_SUPPORT
INT AsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicReadAggCnt(pAd, aggCnt, cnt_len);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetRalinkBurstMode(pAd, enable);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicUpdateTxOP(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 TxOpVal)
{
	UINT32 last_txop_val;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->CommonCfg.ManualTxop)
			return TRUE;

		last_txop_val = MtAsicGetWmmParam(pAd, AcNum, WMM_PARAM_TXOP);

		if (last_txop_val == TxOpVal) {
			/* No need to Update TxOP CR */
			return TRUE;
		} else if (last_txop_val == 0xdeadbeef) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error CR value for TxOP = 0x%08x\n", __func__,
					 last_txop_val));
			return FALSE;
		}

		MtAsicSetWmmParam(pAd, 0, AcNum, WMM_PARAM_TXOP, TxOpVal);
		return TRUE;
	}

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#endif /* DOT11_N_SUPPORT */


INT AsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel)
{
	/* TODO: Shiang-MT7615, remove this from here?? Do we still need it?? */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicWaitMacTxRxIdle(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetMacTxRx(RTMP_ADAPTER *pAd, INT txrx, BOOLEAN enable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT ret = 0;

		if (pAd->archOps.archSetMacTxRx) {
			ret = pAd->archOps.archSetMacTxRx(pAd, txrx, enable, BAND0);

			if (ret != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s(): SetMacTxRx failed!\n", __func__));
				return ret;
			}

#ifdef DBDC_MODE

			if (pAd->CommonCfg.dbdc_mode) {
				ret = pAd->archOps.archSetMacTxRx(pAd, txrx, enable, BAND1);

				if (ret != 0) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							 ("%s(): SetMacTxRx failed!\n", __func__));
					return ret;
				}
			}

#endif /*DBDC_MODE*/
			return ret;
		}
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR ucBandIdx)
{

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT ret = 0;

		if (pAd->archOps.archSetRxvFilter) {
			ret = pAd->archOps.archSetRxvFilter(pAd, enable, ucBandIdx);

			if (ret != 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s(): SetRxvTxRx  failed!\n", __func__));
				return ret;
			}

			return ret;
		}
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
#ifdef MT_MAC
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetWPDMA(pAd, TxRx, enable, cap->WPDMABurstSIZE);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


BOOLEAN AsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicWaitPDMAIdle(pAd, round, wait_us);
#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN AsicResetWPDMA(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		MtAsicResetWPDMA(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetMacWD(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetMacWD(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicHIFInit(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT AsicTOPInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicTOPInit(pAd);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT AsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR opmode, BOOLEAN up, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetTxStream) {
			INT Ret;
			Ret = pAd->archOps.archSetTxStream(pAd, pAd->Antenna.field.TxPath, 0);
			return Ret;
		}

		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 rx_path, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetRxStream) {
			INT Ret;
			Ret = pAd->archOps.archSetRxStream(pAd, rx_path, BandIdx);
			return Ret;
		}

		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicSetBW(pAd, bw, BandIdx);

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


INT AsicSetCtrlCh(RTMP_ADAPTER *pAd, UINT8 extch)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return mt_mac_set_ctrlch(pAd, extch);

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

VOID AsicSetTmrCR(RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx)
{
	if (pAd->archOps.archSetTmrCR)
		pAd->archOps.archSetTmrCR(pAd, enable, BandIdx);
	else
		AsicNotSupportFunc(pAd, __func__);
}

#ifdef MAC_APCLI_SUPPORT
/*
 * ==========================================================================
 * Description:
 * Set BSSID of Root AP
 *
 * IRQL = DISPATCH_LEVEL
 *
 * ==========================================================================
 */
VOID AsicSetApCliBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR index)
{
	if (pAd->archOps.archSetApCliBssid)
		pAd->archOps.archSetApCliBssid(pAd, pBssid, index);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* MAC_APCLI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssWdevIfAddr(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode)
{
	if (pAd->archOps.archSetMbssWdevIfAddr)
		pAd->archOps.archSetMbssWdevIfAddr(pAd, idx, if_addr, opmode);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	if (pAd->archOps.archSetMbssHwCRSetting)
		pAd->archOps.archSetMbssHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

/* set Wdev Mac Address, some chip arch need to set CR .*/
VOID AsicSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	if (pAd->archOps.archSetExtMbssEnableCR)
		pAd->archOps.archSetExtMbssEnableCR(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}

VOID AsicSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
	if (pAd->archOps.archSetExtTTTTHwCRSetting)
		pAd->archOps.archSetExtTTTTHwCRSetting(pAd, mbss_idx, enable);
	else
		AsicNotSupportFunc(pAd, __func__);
}
#endif /* CONFIG_AP_SUPPORT */


VOID AsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_DMASCH_CTRL_T DmaSchCtrl;

		if (MTK_REV_GTE(pAd, MT7603, MT7603E1) && MTK_REV_LT(pAd, MT7603, MT7603E2))
			DmaSchCtrl.bBeaconSpecificGroup = FALSE;
		else
			DmaSchCtrl.bBeaconSpecificGroup = TRUE;

		DmaSchCtrl.mode = mode;
#ifdef DMA_SCH_SUPPORT
		MtAsicDMASchedulerInit(pAd, DmaSchCtrl);
#endif
		return;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
}

INT32 AsicDevInfoUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Set OwnMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, PRINT_MAC(OwnMacAddr)));
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (pAd->archOps.archSetDevMac)
			return pAd->archOps.archSetDevMac(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, EnableFeature);

		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}

#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicBssInfoUpdate(
	RTMP_ADAPTER *pAd,
	BSS_INFO_ARGUMENT_T bss_info_argument)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Set Bssid=%02x:%02x:%02x:%02x:%02x:%02x, BssIndex(%d)\n",
			  __func__,
			  PRINT_MAC(bss_info_argument.Bssid),
			  bss_info_argument.ucBssIndex));

	if (pAd->archOps.archSetBssid)
		return pAd->archOps.archSetBssid(pAd, bss_info_argument);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

INT32 AsicExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit)
{
	MT_PWR_MGT_BIT_WIFI_T rPwtMgtBitWiFi = {0};
	rPwtMgtBitWiFi.ucWlanIdx = ucWlanIdx;
	rPwtMgtBitWiFi.ucPwrMgtBit = ucPwrMgtBit;
	return MtCmdExtPwrMgtBitWifi(pAd, rPwtMgtBitWiFi);
}

INT32 AsicStaRecUpdate(
	RTMP_ADAPTER *pAd,
	STA_REC_CTRL_T *sta_rec_ctrl)
{
	UINT8 WlanIdx = sta_rec_ctrl->WlanIdx;

	if (pAd->archOps.archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;
		INT32 ret = 0;
		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= &pAd->MacTab.Content[WlanIdx];

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s::Wcid(%d), u4EnableFeature(%d)\n",
				  __func__, sta_rec_ctrl->WlanIdx, sta_rec_ctrl->EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			if (IS_ENTRY_REPEATER(pEntry))
				StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
			else
				StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

#ifdef TXBF_SUPPORT
		if (pEntry && !IS_ENTRY_NONE(pEntry)
			&& (IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry) || IS_ENTRY_AP(pEntry))) {
			UINT8 ucTxPath = pAd->Antenna.field.TxPath;

#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {
				UINT8 band_idx = HcGetBandByWdev(pEntry->wdev);

				if (band_idx == DBDC_BAND0)
					ucTxPath = pAd->dbdc_band0_tx_path;
				else
					ucTxPath = pAd->dbdc_band1_tx_path;
			}
#endif

			if (HcIsBfCapSupport(pEntry->wdev) == TRUE) {
				if (sta_rec_ctrl->EnableFeature & STA_REC_BF_FEATURE)
					if (ucTxPath > 1)
						AsicBfStaRecUpdate(pAd, pEntry->wdev->PhyMode, sta_rec_ctrl->BssIndex, WlanIdx);
			}
		}
#endif /* TXBF_SUPPORT */
		StaCfg.ConnectionState = sta_rec_ctrl->ConnectionState;
		StaCfg.ConnectionType = sta_rec_ctrl->ConnectionType;
		StaCfg.u4EnableFeature = sta_rec_ctrl->EnableFeature;
		StaCfg.ucBssIndex = sta_rec_ctrl->BssIndex;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.IsNewSTARec = sta_rec_ctrl->IsNewSTARec;
		os_move_mem(&StaCfg.asic_sec_info, &sta_rec_ctrl->asic_sec_info, sizeof(ASIC_SEC_INFO));
		ret = pAd->archOps.archSetStaRec(pAd, StaCfg);
		return ret;
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}


#ifdef MT_MAC
INT32 AsicRaParamStaRecUpdate(
	RTMP_ADAPTER *pAd,
	UINT8 WlanIdx,
	P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam,
	UINT32 EnableFeature)
{
	if (pAd->archOps.archSetStaRec) {
		STA_REC_CFG_T StaCfg;
		PMAC_TABLE_ENTRY pEntry = NULL;
		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		/* Need to provide H/W BC/MC WLAN index to CR4 */
		if (!VALID_UCAST_ENTRY_WCID(pAd, WlanIdx))
			pEntry = NULL;
		else
			pEntry	= &pAd->MacTab.Content[WlanIdx];

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s::Wcid(%d), u4EnableFeature(%d)\n",
				  __func__, WlanIdx, EnableFeature));

		if (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (!pEntry->wdev) {
				ASSERT(pEntry->wdev);
				return -1;
			}

			if (IS_ENTRY_REPEATER(pEntry))
				StaCfg.MuarIdx = pAd->MacTab.tr_entry[pEntry->wcid].OmacIdx;
			else
				StaCfg.MuarIdx = pEntry->wdev->OmacIdx;

			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
		} else {
			StaCfg.MuarIdx = 0xe;/* TODO: Carter, check this on TX_HDR_TRANS */
		}

		StaCfg.ConnectionState = STATE_CONNECTED;
		StaCfg.u4EnableFeature = EnableFeature;
		StaCfg.ucWlanIdx = WlanIdx;
		StaCfg.pEntry = pEntry;
		StaCfg.pRaParam = prParam;
		/*tracking the starec input history*/
		return pAd->archOps.archSetStaRec(pAd, StaCfg);
	}

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}
#endif /* MT_MAC */

INT32 AsicRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 ucRadio)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};
		PmStatCtrl.PmNumber = PM5;
		PmStatCtrl.DbdcIdx = ucDbdcIdx;

		if (ucRadio == WIFI_RADIO_ON) {
			PmStatCtrl.PmState = EXIT_PM_STATE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOn\n",
					 __func__, ucDbdcIdx));
		} else {
			PmStatCtrl.PmState = ENTER_PM_STATE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DbdcIdx=%d RadioOff\n",
					 __func__, ucDbdcIdx));
		}

		return  MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef GREENAP_SUPPORT
INT32 AsicGreenAPOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN ucGreenAPOn)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_GREENAP_CTRL_T GreenAPCtrl = {0};
		GreenAPCtrl.ucDbdcIdx = ucDbdcIdx;
		GreenAPCtrl.ucGreenAPOn = ucGreenAPOn;
		return  MtCmdExtGreenAPOnOffCtrl(pAd, GreenAPCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* GREENAP_SUPPORT */

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
INT32 asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PCIE_ASPM_DYM_CTRL_T mt_pcie_aspm_dym_ctrl = {0};

		mt_pcie_aspm_dym_ctrl.ucDbdcIdx = ucDbdcIdx;
		mt_pcie_aspm_dym_ctrl.fgL1Enable = fgL1Enable;
		mt_pcie_aspm_dym_ctrl.fgL0sEnable = fgL0sEnable;
		return  mt_cmd_ext_pcie_aspm_dym_ctrl(pAd, mt_pcie_aspm_dym_ctrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* PCIE_ASPM_DYM_CTRL_SUPPORT */

INT32 AsicExtPmStateCtrl(
	RTMP_ADAPTER *pAd,
	PSTA_ADMIN_CONFIG pStaCfg,
	UINT8 ucPmNumber,
	UINT8 ucPmState)
{
	struct wifi_dev *wdev = NULL;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
	}
#endif
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_PMSTAT_CTRL_T PmStatCtrl = {0};
		PmStatCtrl.PmNumber = ucPmNumber;
		PmStatCtrl.PmState = ucPmState;

		if (ucPmNumber == PM4) {
		}

		return  MtCmdExtPmStateCtrl(pAd, PmStatCtrl);
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicExtWifiHifCtrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, UINT8 PmStatCtrl, VOID *pReslt)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return  MtCmdWifiHifCtrl(pAd, ucDbdcIdx, PmStatCtrl, pReslt);

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef CONFIG_MULTI_CHANNEL

INT32 AsicMccStart(struct _RTMP_ADAPTER *ad,
				   UCHAR channel_1st,
				   UCHAR channel_2nd,
				   UINT32 bw_1st,
				   UINT32 bw_2nd,
				   UCHAR central_1st_seg0,
				   UCHAR central_1st_seg1,
				   UCHAR central_2nd_seg0,
				   UCHAR central_2nd_seg1,
				   UCHAR role_1st,
				   UCHAR role_2nd,
				   USHORT stay_time_1st,
				   USHORT stay_time_2nd,
				   USHORT idle_time,
				   USHORT null_repeat_cnt,
				   UINT32 start_tsf)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MT_MCC_ENTRT_T entries[2];
		entries[0].BssIdx = 0;
		entries[0].WlanIdx = 1;
		entries[0].WmmIdx = 0;
		entries[0].OwnMACAddressIdx = 0;
		entries[0].Bw = bw_1st;
		entries[0].CentralSeg0 =  central_1st_seg0;
		entries[0].CentralSeg1 =  central_1st_seg1;
		entries[0].Channel = channel_1st;
		entries[0].Role = role_1st;
		entries[0].StayTime = stay_time_1st;
		entries[1].BssIdx = 1;
		entries[1].WlanIdx = 2;
		entries[1].WmmIdx = 1;
		entries[1].OwnMACAddressIdx = 1;
		entries[1].Bw = bw_2nd;
		entries[1].CentralSeg0 =  central_2nd_seg0;
		entries[1].CentralSeg1 =  central_2nd_seg1;
		entries[1].Channel = channel_2nd;
		entries[1].Role = role_2nd;
		entries[1].StayTime = stay_time_2nd;
		return MtCmdMccStart(ad, 2, entries, idle_time, null_repeat_cnt, start_tsf);
	}

#endif
	AsicNotSupportFunc(ad, __func__);
	return 0;
}

#endif




#ifdef THERMAL_PROTECT_SUPPORT
INT32
AsicThermalProtect(
	RTMP_ADAPTER *pAd,
	UINT8 HighEn,
	CHAR HighTempTh,
	UINT8 LowEn,
	CHAR LowTempTh,
	UINT32 RechkTimer,
	UINT8 RFOffEn,
	CHAR RFOffTh,
	UINT8 ucType)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;
		ret = MtCmdThermalProtect(pAd, HighEn, HighTempTh, LowEn, LowTempTh, RechkTimer, RFOffEn, RFOffTh, ucType);

		if (ret == NDIS_STATUS_SUCCESS) {
			pAd->thermal_pro_high_criteria = HighTempTh;
			pAd->thermal_pro_high_en = HighEn;
			pAd->thermal_pro_low_criteria = LowTempTh;
			pAd->thermal_pro_low_en = LowEn;
			pAd->recheck_timer = RechkTimer;
			pAd->thermal_pro_RFOff_criteria = RFOffTh;
			pAd->thermal_pro_RFOff_en = RFOffEn;
		}

		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}


INT32
AsicThermalProtectAdmitDuty(
	RTMP_ADAPTER *pAd,
	UINT32 u4Lv0Duty,
	UINT32 u4Lv1Duty,
	UINT32 u4Lv2Duty,
	UINT32 u4Lv3Duty
)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		INT32 ret = 0;
		ret = MtCmdThermalProtectAdmitDuty(pAd, u4Lv0Duty, u4Lv1Duty, u4Lv2Duty, u4Lv3Duty);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif /* THERMAL_PROTECT_SUPPORT */


INT32 AsicGetMacInfo(RTMP_ADAPTER *pAd, UINT32 *ChipId, UINT32 *HwVer, UINT32 *FwVer)
{
	INT32 ret;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicGetMacInfo(pAd, ChipId, HwVer, FwVer);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}
#endif/*COMPOS_TESTMODE_WIN*/
INT32 AsicGetAntMode(RTMP_ADAPTER *pAd, UCHAR *AntMode)
{
	INT32 ret;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicGetAntMode(pAd, AntMode);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

INT32 AsicSetDmaByPassMode(RTMP_ADAPTER *pAd, BOOLEAN isByPass)
{
	INT32 ret;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		ret = MtAsicSetDmaByPassMode(pAd, isByPass);
		return ret;
	}

#endif
	AsicNotSupportFunc(pAd, __func__);
	return 0;
}

#ifdef DBDC_MODE
INT32 AsicGetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret = 0;

	if (pAd->archOps.archGetDbdcCtrl)
		ret = pAd->archOps.archGetDbdcCtrl(pAd, pBctrlInfo);
	else
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}

INT32 AsicSetDbdcCtrl(RTMP_ADAPTER *pAd, BCTRL_INFO_T *pBctrlInfo)
{
	INT32 ret = 0;

	if (pAd->archOps.archSetDbdcCtrl)
		ret = pAd->archOps.archSetDbdcCtrl(pAd, pBctrlInfo);
	else
		AsicNotSupportFunc(pAd, __func__);

	return ret;
}

#endif /*DBDC_MODE*/

INT32 AsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan,
						   BOOLEAN SwPcP)
{
	INT32 ret = 0;

	if (pAd->archOps.archRxHeaderTransCtl)
		ret = pAd->archOps.archRxHeaderTransCtl(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);

	return ret;
}

INT32 AsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{
	INT32 ret = 0;

	if (pAd->archOps.archRxHeaderTaranBLCtl)
		ret = pAd->archOps.archRxHeaderTaranBLCtl(pAd, Index, En, EthType);

	return ret;
}

#ifdef IGMP_SNOOP_SUPPORT
BOOLEAN AsicMcastEntryInsert(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, UINT8 Type, PUCHAR MemberAddr,
							 PNET_DEV dev, UINT8 WlanIndex)
{
	INT32 Ret = 0;

	if (pAd->archOps.archMcastEntryInsert)
		Ret = pAd->archOps.archMcastEntryInsert(pAd, GrpAddr, BssIdx, Type, MemberAddr, dev, WlanIndex);

	return Ret;
}


BOOLEAN AsicMcastEntryDelete(RTMP_ADAPTER *pAd, PUCHAR GrpAddr, UINT8 BssIdx, PUCHAR MemberAddr, PNET_DEV dev,
							 UINT8 WlanIndex)
{
	INT32 Ret = 0;

	if (pAd->archOps.archMcastEntryDelete)
		Ret = pAd->archOps.archMcastEntryDelete(pAd, GrpAddr, BssIdx, MemberAddr, dev, WlanIndex);

	return Ret;
}

#ifdef IGMP_TVM_SUPPORT
BOOLEAN AsicMcastConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 omac_idx)
{
	INT32 Ret = 0;

	if (pAd->archOps.archMcastConfigAgeout)
		Ret = pAd->archOps.archMcastConfigAgeout(pAd, AgeOutTime, omac_idx);

	return Ret;
}

BOOLEAN AsicMcastGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev)
{
	INT32 Ret = 0;

	if (pAd->archOps.archMcastGetMcastTable)
		Ret = pAd->archOps.archMcastGetMcastTable(pAd, ucOwnMacIdx, wdev);

	return Ret;
}

#endif /* IGMP_TVM_SUPPORT*/

#endif

#ifdef DOT11_VHT_AC
INT AsicSetRtsSignalTA(RTMP_ADAPTER *pAd, UCHAR bw_sig)
{
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
#ifdef DBDC_MODE
		if (pAd->CommonCfg.dbdc_mode)
			pAd->archOps.archSetRtsSignalTA(pAd, 1, bw_sig);
#endif /*  DBDC_MODE */
		pAd->archOps.archSetRtsSignalTA(pAd, 0, bw_sig);
		return TRUE;
	}
#endif /* MT_MAC */
	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}
#endif /*DOT11_VHT_AC*/

VOID RssiUpdate(RTMP_ADAPTER *pAd)
{
	CHAR RSSI[4];
	MAC_TABLE_ENTRY *pEntry;
	INT i = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			if (pAd->MacTab.Size == 0) {
				pEntry = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE];
				MtRssiGet(pAd, pEntry->wcid, &RSSI[0]);

				for (i = 0; i < TX_STREAM_PATH; i++) {
					pEntry->RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pEntry->RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.AvgRssi[i] = MINIMUM_POWER_VALUE;
					pAd->ApCfg.RssiSample.LastRssi[i] = MINIMUM_POWER_VALUE;
				}
			} else {
				INT32 TotalRssi[4];
				INT j;
				NdisZeroMemory(TotalRssi, sizeof(TotalRssi));

				for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
					pEntry = &pAd->MacTab.Content[i];

					if (IS_VALID_ENTRY(pEntry)) {
						MtRssiGet(pAd, pEntry->wcid, &RSSI[0]);

						for (j = 0; j < TX_STREAM_PATH; j++) {
							pEntry->RssiSample.AvgRssi[j] = RSSI[j];
							pEntry->RssiSample.LastRssi[j] = RSSI[j];
							TotalRssi[j] += RSSI[j];
						}
					}
				}

				for (i = 0; i < 4; i++) {
					if (pAd->MacTab.Size != 0)
						pAd->ApCfg.RssiSample.AvgRssi[i] = pAd->ApCfg.RssiSample.LastRssi[i] = TotalRssi[i] / pAd->MacTab.Size;
					else
						break;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
}

/* end Trace for every 100 ms */
#ifdef ETSI_RX_BLOCKER_SUPPORT
VOID CheckRssi(RTMP_ADAPTER *pAd)
{
	UINT8   u1MaxWRssiIdx;
	UINT8   u1WFBitMap	   = BITMAP_WF_ALL;
	CHAR	c1MaxWbRssi	= MINIMUM_POWER_VALUE;
	UINT32	u4WbRssi	   = 0;
	UINT8	u1CheckIdx;
	UINT32  u4DcrfCr = 0;
	UCHAR   u1BandIdx = 0;

	switch (pAd->u1RxBlockerState) {
	case ETSI_RXBLOCKER4R:

		/* Enable DCRF tracking */
		PHY_IO_READ32(pAd, DCRF_TRACK, &u4DcrfCr);
		u4DcrfCr &= ~(BITS(28, 29));
		u4DcrfCr |= ((0x3 << 28) & BITS(28, 29)); /*Enable DCRF*/
		PHY_IO_WRITE32(pAd, DCRF_TRACK, u4DcrfCr);


		/* confidence count check for 1R transition */
		for (u1CheckIdx = 0; u1CheckIdx < pAd->u1To1RCheckCnt; u1CheckIdx++) {
			/* update Max WBRSSI index */
			u1MaxWRssiIdx = ETSIWbRssiCheck(pAd);

			/* log check Max Rssi Index or not found */
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" i1MaxWRssiIdxPrev: %x \n", pAd->i1MaxWRssiIdxPrev));
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u1MaxWRssiIdx: %x \n", u1MaxWRssiIdx));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
			/* ---------------- */

			/* not found Max WBRSSI Index */
			if (0xFF == u1MaxWRssiIdx) {
				pAd->u1ValidCnt = 0;
				pAd->i1MaxWRssiIdxPrev = 0xFF;
			}
			/* confidence count increment to 1R state */
			else if (pAd->i1MaxWRssiIdxPrev == u1MaxWRssiIdx) {
				pAd->u1ValidCnt++;
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Same index: u1ValidCnt: %d \n", pAd->u1ValidCnt));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
			}
			/* Max WBRSSI index changed */
			else {
				pAd->u1ValidCnt = 1;
				pAd->i1MaxWRssiIdxPrev = u1MaxWRssiIdx;
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Different index: u1ValidCnt: %d \n", pAd->u1ValidCnt));
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
			}

			/* confidence count check */
			if (pAd->u1ValidCnt >= pAd->u2To1RvaildCntTH) {
				/* config Rx index according to bitmap */
				switch (u1MaxWRssiIdx) {
				case 0:
					u1WFBitMap = BITMAP_WF0;
					break;
				case 1:
					u1WFBitMap = BITMAP_WF1;
					break;
				case 2:
					u1WFBitMap = BITMAP_WF2;
					break;
				case 3:
					u1WFBitMap = BITMAP_WF3;
					break;
				default:
					break;
				}

				/* config Rx */
				MtCmdLinkTestRxCtrl(pAd, u1WFBitMap, u1BandIdx);

				/* reset confidence count */
				pAd->u1ValidCnt = 0;
				/* update state */
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
				/* break out for loop */
				break;
			}
		}
		break;

	case ETSI_RXBLOCKER1R:

		/* Disable DCRF tracking */
		PHY_IO_READ32(pAd, DCRF_TRACK, &u4DcrfCr);
		u4DcrfCr &= ~(BITS(28, 29));
		u4DcrfCr |= ((0x0 << 28) & BITS(28, 29));/*disable DCRF*/
		PHY_IO_WRITE32(pAd, DCRF_TRACK, u4DcrfCr);


#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (IS_MT7622(pAd)) { /* for 7622 */
			if (pAd->i1MaxWRssiIdxPrev == WF0 || pAd->i1MaxWRssiIdxPrev == WF1) {
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
			} else {
			/* Read WBRSSI (WF2) */
			PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
			}
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			if (pAd->i1MaxWRssiIdxPrev == WF0 || pAd->i1MaxWRssiIdxPrev == WF1) {
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			} else {
			/* Read WBRSSI (WF2) */
			PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			}
		}
	} else {
		if (IS_MT7622(pAd)) { /* for 7622 */
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = (u4WbRssi & BITS(0, 7));/* [7:0] */
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			/* Read WBRSSI (WF0) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
		}
	}
#else
	if (IS_MT7622(pAd)) { /* for 7622 */
		/* Read WBRSSI (WF0) */
		PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
		c1MaxWbRssi = (u4WbRssi & BITS(0, 7)); /* [7:0] */
	}
	if (IS_MT7615(pAd)) { /* for 7615 */
		/* Read WBRSSI (WF0) */
		PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
		c1MaxWbRssi = ((u4WbRssi >> 16) & BITS(0, 7)); /* [23:16] */
	}
#endif /* DBDC_MODE */

	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(1R State)----------------------------------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %x \n", c1MaxWbRssi&0xFF));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	/* ---------------- */


		/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
		if ((c1MaxWbRssi&0xFF) == 0x80) {

			pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;

		}
		/* No CR risk */
		else {

			/* check whether back to 4R mode */
			if (c1MaxWbRssi < pAd->c1WBRssiTh4R) {
				/* CR risk - Protect unexpected value */
				if (pAd->u14RValidCnt >= pAd->u2To4RvaildCntTH) {

					MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL, u1BandIdx);
					/* update state */
					pAd->u1RxBlockerState = ETSI_RXBLOCKER4R;
					pAd->u14RValidCnt = 1;

					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(TO 4R State)------------------------------- \n"));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u14RValidCnt: %d \n", pAd->u14RValidCnt));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
					/* ---------------- */
				} else {
					pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
					pAd->u14RValidCnt++;
					/* log for check Rssi Read (WBRSSI/IBRSSI) */
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------(Keep 1R State)------------------------------- \n"));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %d, c1WBRssiTh4R: %d \n", c1MaxWbRssi, pAd->c1WBRssiTh4R));
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" CR risk!! u14RValidCnt: %d \n", pAd->u14RValidCnt));
					MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
					/* ---------------- */
				}

			} else
				pAd->u1RxBlockerState = ETSI_RXBLOCKER1R;
		}

		break;
	default:
		break;
	}
}
#endif /* end of ETSI_RX_BLOCKER_SUPPORT */

#define RTS_NUM_DIS_VALUE 0xff
#define RTS_LEN_DIS_VALUE 0xffffff
INT asic_rts_on_off(struct wifi_dev *wdev, BOOLEAN rts_en)
{
	struct _RTMP_ADAPTER *ad;
	UCHAR band_idx;
	UINT32 rts_num;
	UINT32 rts_len;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	band_idx = HcGetBandByWdev(wdev);

#ifdef MT_MAC
	if (ad->archOps.asic_rts_on_off) {
		if (rts_en) {
			rts_num = wlan_operate_get_rts_pkt_thld(wdev);
			rts_len = wlan_operate_get_rts_len_thld(wdev);
		} else {
			rts_num = RTS_NUM_DIS_VALUE;
			rts_len = RTS_LEN_DIS_VALUE;
		}
		return ad->archOps.asic_rts_on_off(ad, band_idx, rts_num, rts_len, rts_en);
	}
#endif

	AsicNotSupportFunc(ad, __func__);
	return 0;
}

INT AsicAmpduEfficiencyAdjust(struct wifi_dev *wdev, UCHAR	aifs_adjust)
{
	struct _RTMP_ADAPTER *ad;
	UINT32	wmm_idx;

	if (!wdev)
		return 0;

	ad = wdev->sys_handle;
	wmm_idx = HcGetWmmIdx(ad, wdev);
#ifdef MT_MAC

	if (ad->archOps.asic_ampdu_efficiency_on_off)
		return ad->archOps.asic_ampdu_efficiency_on_off(ad, wmm_idx, aifs_adjust);

#endif
	AsicNotSupportFunc(ad, __func__);
	return 0;
}


BOOLEAN asic_bss_beacon_exit(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	/* beacon init for USB/SDIO */
	if (arch_ops->arch_bss_beacon_exit)
		arch_ops->arch_bss_beacon_exit(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_stop(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	/* beacon stop for USB/SDIO */
	if (arch_ops->arch_bss_beacon_stop)
		arch_ops->arch_bss_beacon_stop(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_start(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	/* beacon start for USB/SDIO */
	if (arch_ops->arch_bss_beacon_start)
		arch_ops->arch_bss_beacon_start(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

BOOLEAN asic_bss_beacon_init(struct _RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	/* beacon init for USB/SDIO */
	if (arch_ops->arch_bss_beacon_init)
		arch_ops->arch_bss_beacon_init(pAd);

	AsicNotSupportFunc(pAd, __func__);
	return FALSE;
}

/*----------------------------------------------------------------------------*/
/*! Key word: "RXBLOCKER", "WBRSSI", "IBRSSI"
* \Concept:  Switch 4RX to 1RX by detect WBRSSI
*1
* \Input: 	 None
*
* \return:   WBRSSI[MAX] or -1
*/
/*----------------------------------------------------------------------------*/
#ifdef	ETSI_RX_BLOCKER_SUPPORT
UINT8 ETSIWbRssiCheck(
	RTMP_ADAPTER *pAd
)
{
	BOOLEAN	fg1RVaild		   = TRUE;
	UINT8	u1WfIdx;
	UINT8	u1MaxWbRssiIdx	  = 0;
	CHAR	c1MaxWbRssi		 = MINIMUM_POWER_VALUE;
	CHAR	c1WbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};
	CHAR	c1IbRssi[WF_NUM]	= {MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE, MINIMUM_POWER_VALUE};

	/* buffer to read CR */
	UINT32	u4WbRssi			= 0;

	/* Read CR (manual command) */
	if (pAd->fgFixWbIBRssiEn) {
		/* WBRSSI */
		c1WbRssi[WF0] = pAd->c1WbRssiWF0;
		c1WbRssi[WF1] = pAd->c1WbRssiWF1;
		c1WbRssi[WF2] = pAd->c1WbRssiWF2;
		c1WbRssi[WF3] = pAd->c1WbRssiWF3;
		/* IBRSSI */
		c1IbRssi[WF0] = pAd->c1IbRssiWF0;
		c1IbRssi[WF1] = pAd->c1IbRssiWF1;
		c1IbRssi[WF2] = pAd->c1IbRssiWF2;
		c1IbRssi[WF3] = pAd->c1IbRssiWF3;
	}
	/* Read CR (HW CR) */
	else {
		if (IS_MT7622(pAd)) { /* for 7622 */
			/* Read WBRSSI/IBRSSI (WF0, WF1) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF1] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF0] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF1] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF0] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */

			/* Read WBRSSI/IBRSSI (WF2, WF3) */
			PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF3] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF2] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF3] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF2] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */
		}
		if (IS_MT7615(pAd)) { /* for 7615 */
			/* Read WBRSSI/IBRSSI (WF0, WF1) */
			PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF0] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF1] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF0] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF1] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */

			/* Read WBRSSI/IBRSSI (WF2, WF3) */
			PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_2, &u4WbRssi);
			c1WbRssi[WF2] = ((u4WbRssi >> 16) & BITS(0, 7));/* [23:16] */
			c1WbRssi[WF3] = (u4WbRssi & BITS(0, 7));/* [7:0] */
			c1IbRssi[WF2] = ((u4WbRssi >> 24) & BITS(0, 7));/* [31:24] */
			c1IbRssi[WF3] = ((u4WbRssi >> 8) & BITS(0, 7));/* [15:8] */
		}
	}

	/* log for check Rssi Read (WBRSSI/IBRSSI) */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1WbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x \n", c1WbRssi[WF0]&0xFF, c1WbRssi[WF1]&0xFF, c1WbRssi[WF2]&0xFF, c1WbRssi[WF3]&0xFF));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1IbRssi: WF0: %x, WF1: %x, WF2: %x, WF3: %x \n", c1IbRssi[WF0]&0xFF, c1IbRssi[WF1]&0xFF, c1IbRssi[WF2]&0xFF, c1IbRssi[WF3]&0xFF));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	/* ---------------- */


	/* CR risk - no expected 0x80 value on WF0/WF1 ; WF2/WF3  */
	if (((c1WbRssi[WF0]&0xFF) == 0x80) || ((c1WbRssi[WF2]&0xFF) == 0x80)) {

		fg1RVaild = TRUE;
		/* log for check Rssi Read (WBRSSI/IBRSSI) */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" CR risk !! \n"));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
		/* ---------------- */
	}
	/* No CR risk */
	else {

	/* Find Max Rssi */
	for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
		if (c1WbRssi[u1WfIdx] > c1MaxWbRssi) {
			/* update Max WBRSSI value */
			c1MaxWbRssi = c1WbRssi[u1WfIdx];
			/* update Max WBRSSI index */
			u1MaxWbRssiIdx = u1WfIdx;
		}
	}


	/* log Max Rssi Value and Max Rssi Index */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----------------------------------(4R State)------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" u1WfIdx: %x \n", u1WfIdx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" c1MaxWbRssi: %x \n", c1MaxWbRssi));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	/* ---------------- */


	/* check state transition status (4R->1R) */
	if (c1MaxWbRssi >= pAd->c1RWbRssiHTh) {
		for (u1WfIdx = WF0; u1WfIdx < WF_NUM; u1WfIdx++) {
			if ((u1WfIdx != u1MaxWbRssiIdx) && \
				((c1WbRssi[u1WfIdx] > pAd->c1RWbRssiLTh) || (c1IbRssi[u1WfIdx] > pAd->c1RIbRssiLTh))) {
				fg1RVaild = FALSE;
			} else
				fg1RVaild = TRUE;
		}
	} else
		fg1RVaild = FALSE;

	}

	/* log check flag to 1R */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" fg1RVaild: %x \n", fg1RVaild));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("----------------------------------------------------------------------------- \n"));
	/* ---------------- */

	/* check 1R transition flag */
	if (fg1RVaild)
		return u1MaxWbRssiIdx;
	else
		return 0xFF;
}
#endif /* end ETSI_RX_BLOCKER_SUPPORT */
