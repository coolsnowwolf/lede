/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/


/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#if defined(COMPOS_WIN)
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Ra_ctrl_mt.tmh"
#endif
#else
#include "rt_config.h"
#endif

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/
/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/
/*!
* \brief     Initialize Rate Adaptation for this entry
*
* \param[in] pAd
* \param[in] pEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
RAInit(
	PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry
)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	RA_COMMON_INFO_T RaCfg;
	P_RA_INTERNAL_INFO_T pRaInternal;
	P_RA_COMMON_INFO_T pRaCfg;
	struct _RTMP_CHIP_CAP *cap;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	pRaEntry = &pEntry->RaEntry;
	pRaInternal = &pEntry->RaInternal;
	pRaCfg = &RaCfg;
	os_zero_mem(pRaEntry, sizeof(RA_ENTRY_INFO_T));
	os_zero_mem(pRaInternal, sizeof(RA_INTERNAL_INFO_T));
	os_zero_mem(&RaCfg, sizeof(RaCfg));
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
	raWrapperConfigSet(pAd, pEntry->wdev, pRaCfg);
	pRaInternal->ucLastRateIdx = 0xFF;
	pRaInternal->ucLowTrafficCount = 0;
	pRaInternal->fgLastSecAccordingRSSI = FALSE;
	pRaInternal->ucInitialRateMode =  RA_INIT_RATE_BY_RSSI;
	pRaInternal->ucLastSecTxRateChangeAction = RATE_NO_CHANGE;
	pRaInternal->ucCurrTxRateIndex = 0;
	pRaInternal->ucTxRateUpPenalty = 0;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE) {
		pRaEntry->fgRaValid = TRUE;
		WifiSysRaInit(pAd, pEntry);
	}

#else

	if (pRaEntry->fgAutoTxRateSwitch == TRUE) {
		UCHAR TableSize = 0;
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_GRP) {
			raSetMcsGroup(pRaEntry, pRaCfg, pRaInternal);
			raClearTxQuality(pRaInternal);
			raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
			NewTxRateMtCore(pAd, pRaEntry, pRaCfg, pRaInternal);
		}

#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS) {
			raSetMcsGroupAGBS(pRaEntry, pRaCfg, pRaInternal);
			raClearTxQualityAGBS(pRaInternal);
			raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
			SetTxRateMtCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);
		}

#endif /* RATE_ADAPT_AGBS_SUPPORT */
	} else {
#ifdef MCS_LUT_SUPPORT
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_GRP)
			MtAsicMcsLutUpdateCore(pAd, pRaEntry, pRaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS)
			MtAsicMcsLutUpdateCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MCS_LUT_SUPPORT */
	}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#ifndef COMPOS_WIN
	RA_SAVE_LAST_TX_CFG(pRaEntry);
#endif /* !COMPOS_WIN */
	raWrapperEntryRestore(pAd, pEntry, pRaEntry);
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     Update RA paramater
*
* \param[in] pAd
* \param[in] pEntry
*
* \return    None
*/
/*----------------------------------------------------------------------------*/
VOID
RAParamUpdate(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry,
	P_CMD_STAREC_AUTO_RATE_UPDATE_T prParam
)
{
	P_RA_ENTRY_INFO_T pRaEntry;
	struct _RTMP_CHIP_CAP *cap;
	/* pRaEntry is used in AsicStaRecUpdate() */
	pRaEntry = &pEntry->RaEntry;

	if (pRaEntry->fgRaValid != TRUE)
		return;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Field=%d\n", __func__, prParam->u4Field));
	raWrapperEntrySet(pAd, pEntry, pRaEntry);
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	if (cap->fgRateAdaptFWOffload == TRUE)
		WifiSysUpdateRa(pAd, pEntry, prParam);

#else
	{
		RA_COMMON_INFO_T RaCfg;
		P_RA_INTERNAL_INFO_T pRaInternal;
		P_RA_COMMON_INFO_T pRaCfg;

		pRaInternal = &pEntry->RaInternal;
		pRaCfg = &RaCfg;
		os_zero_mem(pRaInternal, sizeof(RA_INTERNAL_INFO_T));
		os_zero_mem(&RaCfg, sizeof(RaCfg));
		raWrapperConfigSet(pAd, pEntry->wdev, pRaCfg);
#ifdef MT_MAC
#ifdef RATE_ADAPT_AGBS_SUPPORT

		if (RaCfg.ucRateAlg == RATE_ALG_AGBS) {
			if (prParam->u4Field == RA_PARAM_VHT_OPERATING_MODE) {
				UCHAR TableSize;

				raSetMcsGroupAGBS(pRaEntry, pRaCfg, pRaInternal);
				raSelectTxRateTable(pRaEntry, pRaCfg, pRaInternal, &pRaInternal->pucTable, &TableSize, &pRaInternal->ucCurrTxRateIndex);
				SetTxRateMtCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);
			}
		}

#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MT_MAC */
	}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
}


#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
/*----------------------------------------------------------------------------*/
/*!
* \brief     set RA Common config to FW ( Driver API)
*
* \param[in] pRaCfg
* \param[out] pCmdStaRecAutoRateCfg
*
* \return    NDIS_STATUS_SUCCESS
*
*/
/*----------------------------------------------------------------------------*/
INT32
BssInfoRACommCfgSet(
	IN P_RA_COMMON_INFO_T pRaCfg,
	OUT P_CMD_BSSINFO_AUTO_RATE_CFG_T pCmdBssInfoAutoRateCfg
)
{
	/* Fill TLV format */
	pCmdBssInfoAutoRateCfg->u2Tag = BSS_INFO_RA;
	pCmdBssInfoAutoRateCfg->u2Length = sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T);
#ifdef RT_BIG_ENDIAN
	pCmdBssInfoAutoRateCfg->u2Tag = cpu2le16(pCmdBssInfoAutoRateCfg->u2Tag);
	pCmdBssInfoAutoRateCfg->u2Length = cpu2le16(pCmdBssInfoAutoRateCfg->u2Length);
#endif
	pCmdBssInfoAutoRateCfg->OpMode = pRaCfg->OpMode;
	pCmdBssInfoAutoRateCfg->fgAdHocOn = pRaCfg->fgAdHocOn;
	pCmdBssInfoAutoRateCfg->fgShortPreamble = pRaCfg->fgShortPreamble;
	pCmdBssInfoAutoRateCfg->TxStream = pRaCfg->TxStream;
	pCmdBssInfoAutoRateCfg->RxStream = pRaCfg->RxStream;
	pCmdBssInfoAutoRateCfg->ucRateAlg = pRaCfg->ucRateAlg;
	pCmdBssInfoAutoRateCfg->TestbedForceShortGI = pRaCfg->TestbedForceShortGI;
	pCmdBssInfoAutoRateCfg->TestbedForceGreenField = pRaCfg->TestbedForceGreenField;
#ifdef DOT11_N_SUPPORT
	pCmdBssInfoAutoRateCfg->HtMode = pRaCfg->HtMode;
	pCmdBssInfoAutoRateCfg->fAnyStation20Only = pRaCfg->fAnyStation20Only;
	pCmdBssInfoAutoRateCfg->bRcvBSSWidthTriggerEvents = pRaCfg->bRcvBSSWidthTriggerEvents;
#ifdef DOT11_VHT_AC
	pCmdBssInfoAutoRateCfg->vht_nss_cap = pRaCfg->vht_nss_cap;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pCmdBssInfoAutoRateCfg->fgSeOff = pRaCfg->fgSeOff;
	pCmdBssInfoAutoRateCfg->ucAntennaIndex = pRaCfg->ucAntennaIndex;
	pCmdBssInfoAutoRateCfg->TrainUpRule = pRaCfg->TrainUpRule;
	pCmdBssInfoAutoRateCfg->TrainUpHighThrd = cpu2le16(pRaCfg->TrainUpHighThrd);
	pCmdBssInfoAutoRateCfg->TrainUpRuleRSSI = cpu2le16(pRaCfg->TrainUpRuleRSSI);
	pCmdBssInfoAutoRateCfg->lowTrafficThrd = cpu2le16(pRaCfg->lowTrafficThrd);
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	pCmdBssInfoAutoRateCfg->u2MaxPhyRate = cpu2le16(pRaCfg->u2MaxPhyRate);
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
	pCmdBssInfoAutoRateCfg->PhyCaps = cpu2le32(pRaCfg->PhyCaps);
	pCmdBssInfoAutoRateCfg->u4RaInterval = cpu2le32(pRaCfg->u4RaInterval);
	pCmdBssInfoAutoRateCfg->u4RaFastInterval = cpu2le32(pRaCfg->u4RaFastInterval);

#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: sizeof CMD_BSSINFO_AUTO_RATE_CFG_T = %d\n", __func__, (UINT_32)sizeof(CMD_BSSINFO_AUTO_RATE_CFG_T)));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("OpMode=%d\n", pCmdBssInfoAutoRateCfg->OpMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgAdHocOn=%d\n", pCmdBssInfoAutoRateCfg->fgAdHocOn));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgShortPreamble=%d\n", pCmdBssInfoAutoRateCfg->fgShortPreamble));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxStream=%d\n", pCmdBssInfoAutoRateCfg->TxStream));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxStream=%d\n", pCmdBssInfoAutoRateCfg->RxStream));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucRateAlg=%d\n", pCmdBssInfoAutoRateCfg->ucRateAlg));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TestbedForceShortGI=%d\n", pCmdBssInfoAutoRateCfg->TestbedForceShortGI));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TestbedForceGreenField=%d\n", pCmdBssInfoAutoRateCfg->TestbedForceGreenField));
#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HtMode=%d\n", pCmdBssInfoAutoRateCfg->HtMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fAnyStation20Only=%d\n", pCmdBssInfoAutoRateCfg->fAnyStation20Only));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bRcvBSSWidthTriggerEvents=%d\n", pCmdBssInfoAutoRateCfg->bRcvBSSWidthTriggerEvents));
#ifdef DOT11_VHT_AC
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vht_nss_cap=%d\n", pCmdBssInfoAutoRateCfg->vht_nss_cap));
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgSeOff=%d\n", pCmdBssInfoAutoRateCfg->fgSeOff));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucAntennaIndex=%d\n", pCmdBssInfoAutoRateCfg->ucAntennaIndex));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpRule=%d\n", pCmdBssInfoAutoRateCfg->TrainUpRule));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpHighThrd=%d\n", pCmdBssInfoAutoRateCfg->TrainUpHighThrd));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TrainUpRuleRSSI=%d\n", pCmdBssInfoAutoRateCfg->TrainUpRuleRSSI));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("lowTrafficThrd=%d\n", pCmdBssInfoAutoRateCfg->lowTrafficThrd));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u2MaxPhyRate=%d\n", pCmdBssInfoAutoRateCfg->u2MaxPhyRate));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PhyCaps=%d\n", pCmdBssInfoAutoRateCfg->PhyCaps));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4RaInterval=%d\n", pCmdBssInfoAutoRateCfg->u4RaInterval));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4RaFastInterval=%d\n", pCmdBssInfoAutoRateCfg->u4RaFastInterval));
#endif

	return NDIS_STATUS_SUCCESS;
}
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */


#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) || defined(P18) || defined(MT7663)
/*----------------------------------------------------------------------------*/
/*!
* \brief     Set rate entry info to FW ( Driver API)
*
* \param[in] pRaEntry
* \param[out] pCmdStaRecAutoRate
*
* \return    NDIS_STATUS_SUCCESS
*/
/*----------------------------------------------------------------------------*/
INT32
StaRecAutoRateParamSet(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	OUT P_CMD_STAREC_AUTO_RATE_T pCmdStaRecAutoRate
)
{
#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	UINT_8  ucIdx;
#endif

	/* Fill TLV format */
	pCmdStaRecAutoRate->u2Tag = STA_REC_RA;
	pCmdStaRecAutoRate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_T);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecAutoRate->u2Tag = cpu2le16(pCmdStaRecAutoRate->u2Tag);
	pCmdStaRecAutoRate->u2Length = cpu2le16(pCmdStaRecAutoRate->u2Length);
#endif
	pCmdStaRecAutoRate->fgRaValid = pRaEntry->fgRaValid;
	pCmdStaRecAutoRate->fgAutoTxRateSwitch = pRaEntry->fgAutoTxRateSwitch;

	if (pCmdStaRecAutoRate->fgRaValid == FALSE)
		return NDIS_STATUS_SUCCESS;

	pCmdStaRecAutoRate->ucPhyMode = pRaEntry->ucPhyMode;
	pCmdStaRecAutoRate->ucChannel = pRaEntry->ucChannel;
	pCmdStaRecAutoRate->ucBBPCurrentBW = pRaEntry->ucBBPCurrentBW;
	pCmdStaRecAutoRate->fgDisableCCK = pRaEntry->fgDisableCCK;
	pCmdStaRecAutoRate->fgHtCapMcs32 = pRaEntry->fgHtCapMcs32;
	pCmdStaRecAutoRate->fgHtCapInfoGF = pRaEntry->fgHtCapInfoGF;
	os_move_mem(pCmdStaRecAutoRate->aucHtCapMCSSet, pRaEntry->aucHtCapMCSSet, sizeof(pCmdStaRecAutoRate->aucHtCapMCSSet));
	pCmdStaRecAutoRate->ucMmpsMode = pRaEntry->ucMmpsMode;
	pCmdStaRecAutoRate->ucGband256QAMSupport = pRaEntry->ucGband256QAMSupport;
	pCmdStaRecAutoRate->ucMaxAmpduFactor = pRaEntry->ucMaxAmpduFactor;
	pCmdStaRecAutoRate->fgAuthWapiMode = pRaEntry->fgAuthWapiMode;
	pCmdStaRecAutoRate->RateLen = pRaEntry->RateLen;
	pCmdStaRecAutoRate->ucSupportRateMode = pRaEntry->ucSupportRateMode;
	pCmdStaRecAutoRate->ucSupportCCKMCS = pRaEntry->ucSupportCCKMCS;
	pCmdStaRecAutoRate->ucSupportOFDMMCS = pRaEntry->ucSupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	pCmdStaRecAutoRate->u4SupportHTMCS = cpu2le32(pRaEntry->u4SupportHTMCS);
#ifdef DOT11_VHT_AC
	pCmdStaRecAutoRate->u2SupportVHTMCS1SS = cpu2le16(pRaEntry->u2SupportVHTMCS1SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS2SS = cpu2le16(pRaEntry->u2SupportVHTMCS2SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS3SS = cpu2le16(pRaEntry->u2SupportVHTMCS3SS);
	pCmdStaRecAutoRate->u2SupportVHTMCS4SS = cpu2le16(pRaEntry->u2SupportVHTMCS4SS);
	pCmdStaRecAutoRate->force_op_mode = pRaEntry->force_op_mode;
	pCmdStaRecAutoRate->vhtOpModeChWidth = pRaEntry->vhtOpModeChWidth;
	pCmdStaRecAutoRate->vhtOpModeRxNss = pRaEntry->vhtOpModeRxNss;
	pCmdStaRecAutoRate->vhtOpModeRxNssType = pRaEntry->vhtOpModeRxNssType;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	pCmdStaRecAutoRate->ClientStatusFlags = cpu2le32(pRaEntry->ClientStatusFlags);
	os_move_mem(&pCmdStaRecAutoRate->MaxPhyCfg, &pRaEntry->MaxPhyCfg, sizeof(RA_PHY_CFG_T));

	if (pCmdStaRecAutoRate->fgAutoTxRateSwitch == FALSE) {
		if (pRaEntry->MaxPhyCfg.ShortGI)
			pCmdStaRecAutoRate->MaxPhyCfg.ShortGI = SGI_20 + SGI_40 + SGI_80 + SGI_160;

		if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.ldpc |= VHT_LDPC;
		if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.ldpc |= HT_LDPC;

		if ((pRaEntry->MaxPhyCfg.MODE == MODE_VHT) &&
				CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE))
			pCmdStaRecAutoRate->MaxPhyCfg.STBC = STBC_USE;
	}

#ifdef CFG_RATE_ADJUST_PARAM_DEBUG
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: sizeof CMD_STAREC_AUTO_RATE_T = %d\n", __func__, (UINT_32)sizeof(CMD_STAREC_AUTO_RATE_T)));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgRaValid=%d\n", pCmdStaRecAutoRate->fgRaValid));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgAutoTxRateSwitch=%d\n", pCmdStaRecAutoRate->fgAutoTxRateSwitch));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucPhyMode=%d\n", pCmdStaRecAutoRate->ucPhyMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucChannel=%d\n", pCmdStaRecAutoRate->ucChannel));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucBBPCurrentBW=%d\n", pCmdStaRecAutoRate->ucBBPCurrentBW));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgDisableCCK=%d\n", pCmdStaRecAutoRate->fgDisableCCK));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgHtCapMcs32=%d\n", pCmdStaRecAutoRate->fgHtCapMcs32));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgHtCapInfoGF=%d\n", pCmdStaRecAutoRate->fgHtCapInfoGF));

	for (ucIdx = 0; ucIdx < 4; ucIdx++)
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("aucHtCapMCSSet[%d]=%d\n", ucIdx, pCmdStaRecAutoRate->aucHtCapMCSSet[ucIdx]));

	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucMmpsMode=%d\n", pCmdStaRecAutoRate->ucMmpsMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucGband256QAMSupport=%d\n", pCmdStaRecAutoRate->ucGband256QAMSupport));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucMaxAmpduFactor=%d\n", pCmdStaRecAutoRate->ucMaxAmpduFactor));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgAuthWapiMode=%d\n", pCmdStaRecAutoRate->fgAuthWapiMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RateLen=%d\n", pCmdStaRecAutoRate->RateLen));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucSupportRateMode=%x\n", pCmdStaRecAutoRate->ucSupportRateMode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucSupportCCKMCS=%x\n", pCmdStaRecAutoRate->ucSupportCCKMCS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ucSupportOFDMMCS=%x\n", pCmdStaRecAutoRate->ucSupportOFDMMCS));
#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u4SupportHTMCS=%x\n", pCmdStaRecAutoRate->u4SupportHTMCS));
#ifdef DOT11_VHT_AC
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u2SupportVHTMCS1SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS1SS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u2SupportVHTMCS2SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS2SS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u2SupportVHTMCS3SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS3SS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("u2SupportVHTMCS4SS=%x\n", pCmdStaRecAutoRate->u2SupportVHTMCS4SS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("force_op_mode=%d\n", pCmdStaRecAutoRate->force_op_mode));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vhtOpModeChWidth=%d\n", pCmdStaRecAutoRate->vhtOpModeChWidth));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vhtOpModeRxNss=%d\n", pCmdStaRecAutoRate->vhtOpModeRxNss));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("vhtOpModeRxNssType=%x\n", pCmdStaRecAutoRate->vhtOpModeRxNssType));
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ClientStatusFlags=%x\n", pCmdStaRecAutoRate->ClientStatusFlags));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.MODE=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.MODE));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.Flags=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.Flags));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.STBC=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.STBC));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.ShortGI=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.ShortGI));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.BW=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.BW));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.ldpc=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.ldpc));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.MCS=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.MCS));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxPhyCfg.VhtNss=%d\n", pCmdStaRecAutoRate->MaxPhyCfg.VhtNss));
#endif
	return NDIS_STATUS_SUCCESS;
}


/*----------------------------------------------------------------------------*/
/*!
* \brief     set RA data partial update to FW ( Driver API)
*
* \param[in] pRaEntry
* \param[in] pRaInternal
* \param[in] u4Field
* \param[out] pCmdStaRecAutoRateUpdate
*
* \return    NDIS_STATUS_SUCCESS
*
*/
/*----------------------------------------------------------------------------*/
INT32
StaRecAutoRateUpdate(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_INTERNAL_INFO_T pRaInternal,
	IN P_CMD_STAREC_AUTO_RATE_UPDATE_T pRaParam,
	OUT P_CMD_STAREC_AUTO_RATE_UPDATE_T pCmdStaRecAutoRateUpdate
)
{
	/* Fill TLV format */
	pCmdStaRecAutoRateUpdate->u2Tag = STA_REC_RA_UPDATE;
	pCmdStaRecAutoRateUpdate->u2Length = sizeof(CMD_STAREC_AUTO_RATE_UPDATE_T);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (pRaParam->u4Field == RA_PARAM_VHT_OPERATING_MODE) {
		pCmdStaRecAutoRateUpdate->force_op_mode = pRaEntry->force_op_mode;
		pCmdStaRecAutoRateUpdate->vhtOpModeChWidth = pRaEntry->vhtOpModeChWidth;
		pCmdStaRecAutoRateUpdate->vhtOpModeRxNss = pRaEntry->vhtOpModeRxNss;
		pCmdStaRecAutoRateUpdate->vhtOpModeRxNssType = pRaEntry->vhtOpModeRxNssType;
		pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_VHT_OPERATING_MODE;
	} else
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
		if (pRaParam->u4Field == RA_PARAM_HT_2040_COEX)
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_HT_2040_COEX;
		else if (pRaParam->u4Field == RA_PARAM_HT_2040_BACK)
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_HT_2040_BACK;
		else if (pRaParam->u4Field == RA_PARAM_MMPS_UPDATE) {
			pCmdStaRecAutoRateUpdate->ucMmpsMode = pRaEntry->ucMmpsMode;
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_MMPS_UPDATE;
		} else if (pRaParam->u4Field == RA_PARAM_FIXED_RATE) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_FIXED_RATE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MODE = pRaParam->FixedRateCfg.MODE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.BW = pRaParam->FixedRateCfg.BW;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
			pCmdStaRecAutoRateUpdate->ucShortPreamble = pRaParam->ucShortPreamble;
			pCmdStaRecAutoRateUpdate->ucSpeEn = pRaParam->ucSpeEn;
		} else if (pRaParam->u4Field == RA_PARAM_FIXED_RATE_FALLBACK) {
			pCmdStaRecAutoRateUpdate->u4Field = RA_PARAM_FIXED_RATE_FALLBACK;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MODE = pRaParam->FixedRateCfg.MODE;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.STBC = pRaParam->FixedRateCfg.STBC;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ShortGI = pRaParam->FixedRateCfg.ShortGI;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.BW = pRaParam->FixedRateCfg.BW;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.ldpc = pRaParam->FixedRateCfg.ldpc;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.MCS = pRaParam->FixedRateCfg.MCS;
			pCmdStaRecAutoRateUpdate->FixedRateCfg.VhtNss = pRaParam->FixedRateCfg.VhtNss;
			pCmdStaRecAutoRateUpdate->ucShortPreamble = pRaParam->ucShortPreamble;
			pCmdStaRecAutoRateUpdate->ucSpeEn = pRaParam->ucSpeEn;
			pCmdStaRecAutoRateUpdate->fgIs5G = pRaParam->fgIs5G;
		} else if (pRaParam->u4Field > RA_PARAM_MAX) {
			pCmdStaRecAutoRateUpdate->u4Field = pRaParam->u4Field;
		}

#ifdef RT_BIG_ENDIAN
	pCmdStaRecAutoRateUpdate->u2Tag = cpu2le16(pCmdStaRecAutoRateUpdate->u2Tag);
	pCmdStaRecAutoRateUpdate->u2Length = cpu2le16(pCmdStaRecAutoRateUpdate->u2Length);
	pCmdStaRecAutoRateUpdate->u4Field = cpu2le32(pCmdStaRecAutoRateUpdate->u4Field);
#endif
	return NDIS_STATUS_SUCCESS;
}
#endif /* defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) || defined(P18) || defined(MT7663) */



#endif /* MT_MAC */

