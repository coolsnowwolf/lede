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
#elif defined(WIFI_BUILD_RAM)
#include "precomp.h"
#else
#include "rt_config.h"
#endif

#if !defined(WIFI_BUILD_RAM) || (CFG_WIFI_DRIVER_OFFLOAD_RATE_CTRL == 1)
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
#ifdef WIFI_BUILD_RAM
UCHAR tmi_rate_map_cck_lp[] = {
	TMI_TX_RATE_CCK_1M_LP,
	TMI_TX_RATE_CCK_2M_LP,
	TMI_TX_RATE_CCK_5M_LP,
	TMI_TX_RATE_CCK_11M_LP,
};

UCHAR tmi_rate_map_cck_sp[] = {
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_2M_SP,
	TMI_TX_RATE_CCK_5M_SP,
	TMI_TX_RATE_CCK_11M_SP,
};

UCHAR tmi_rate_map_ofdm[] = {
	TMI_TX_RATE_OFDM_6M,
	TMI_TX_RATE_OFDM_9M,
	TMI_TX_RATE_OFDM_12M,
	TMI_TX_RATE_OFDM_18M,
	TMI_TX_RATE_OFDM_24M,
	TMI_TX_RATE_OFDM_36M,
	TMI_TX_RATE_OFDM_48M,
	TMI_TX_RATE_OFDM_54M,
};
#endif /* WIFI_BUILD_RAM */
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
 * \brief     Get max Rssi by RxStream
 *
 * \param[in] prRaCfg
 * \param[in] cRssi0
 * \param[in] cRssi1
 * \param[in] cRssi2
 *
 * \return    MaxRssi
 */
/*----------------------------------------------------------------------------*/
CHAR
raMaxRssi(
	IN P_RA_COMMON_INFO_T prRaCfg,
	IN CHAR cRssi0,
	IN CHAR cRssi1,
	IN CHAR cRssi2
)
{
	CHAR cLarger = -100;

	if (prRaCfg->RxStream == 1)
		cLarger = cRssi0;

	if (prRaCfg->RxStream >= 2)
		cLarger = max(cRssi0, cRssi1);

	if (prRaCfg->RxStream >= 3)
		cLarger = max(cLarger, cRssi2);

	/*
	 * if (prRaCfg->RxStream == 4)
	 * {
	 * cLarger = max(cLarger, cRssi3);
	 * }
	 */
	return cLarger;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Get min Rssi by RxStream
 *
 * \param[in] prRaCfg
 * \param[in] cRssi0
 * \param[in] cRssi1
 * \param[in] cRssi2
 *
 * \return    MinRssi
 */
/*----------------------------------------------------------------------------*/

CHAR
raMinRssi(
	IN P_RA_COMMON_INFO_T prRaCfg,
	IN CHAR cRssi0,
	IN CHAR cRssi1,
	IN CHAR cRssi2
)
{
	CHAR cMin = -100;

	if (prRaCfg->RxStream == 1)
		cMin = cRssi0;

	if (prRaCfg->RxStream >= 2)
		cMin = min(cRssi0, cRssi1);

	if (prRaCfg->RxStream >= 3)
		cMin = min(cMin, cRssi2);

	return cMin;
}


#ifdef NEW_RATE_ADAPT_SUPPORT


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Set TxPhy according to MaxPhy, rate table and config
 *
 * \param[in] pAd
 * \param[in] pRaEntry
 * \param[in] pRaCfg
 * \param[in] pRaInternal
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
SetTxRateMtCore(
	IN PRTMP_ADAPTER pAd,
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal,
	IN RTMP_RA_GRP_TB *pTxRate
)
{
	UCHAR tx_mode = pTxRate->Mode;
	UCHAR *pTable;
#ifdef DOT11_VHT_AC
	UCHAR tx_bw = pTxRate->BW;
#endif /* DOT11_VHT_AC */
	pTable = pRaInternal->pucTable;
#ifdef DOT11_VHT_AC

	if ((pRaCfg->PhyCaps & fPHY_CAP_VHT) &&
		((pTable == RateTableVht2S) || (pTable == RateTableVht1S) ||
		 (pTable == RateTableVht1S_MCS9) ||
		 (pTable == RateTableVht2S_BW20) ||
		 (pTable == RateTableVht2S_BW40) ||
		 (pTable == RateTableVht2S_MCS7))) {
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;
		UCHAR bw_cap = BW_20;

		if (pAdaptTbEntry->BW != pRaEntry->MaxPhyCfg.BW) {
			switch (pRaEntry->MaxPhyCfg.BW) {
			case BW_80:
				bw_cap = pAdaptTbEntry->BW;
				break;

			case BW_40:
				if (pAdaptTbEntry->BW == BW_80)
					bw_cap = BW_40;
				else
					bw_cap = pAdaptTbEntry->BW;

				break;

			case BW_10:
				bw_cap = BW_10;
				break;

			case BW_20:
			default:
				if (pAdaptTbEntry->BW == BW_80 || pAdaptTbEntry->BW == BW_40)
					bw_cap = BW_20;
				else
					bw_cap = pAdaptTbEntry->BW;

				break;
			}

			tx_bw = bw_cap;
		} else
			tx_bw = pAdaptTbEntry->BW;

		if (pRaEntry->force_op_mode == TRUE) {
			switch (pRaEntry->vhtOpModeChWidth) {
			case 1:
				bw_cap = BW_40;
				break;

			case 2:
				bw_cap = BW_80;
				break;

			case 0:
			default:
				bw_cap = BW_20;
				break;
			}

			if ((tx_bw != BW_10) && (tx_bw >= bw_cap))
				tx_bw = bw_cap;
		}

		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): txbw=%d, txmode=%d\n", __func__, tx_bw, tx_mode));
	}

#endif /* DOT11_VHT_AC */
#ifdef DOT11_N_SUPPORT

	if (tx_mode == MODE_HTMIX || tx_mode == MODE_HTGREENFIELD) {
		if ((pTxRate->STBC) && (pRaEntry->MaxPhyCfg.STBC))
			pRaEntry->TxPhyCfg.STBC = STBC_USE;
		else
			pRaEntry->TxPhyCfg.STBC = STBC_NONE;

		if ((pTxRate->ShortGI || pRaCfg->TestbedForceShortGI) && (pRaEntry->MaxPhyCfg.ShortGI))
			pRaEntry->TxPhyCfg.ShortGI = GI_400;
		else
			pRaEntry->TxPhyCfg.ShortGI = GI_800;
	} else
		pRaEntry->TxPhyCfg.STBC = STBC_NONE;

#ifndef COMPOS_WIN
	/* TODO: Lens, fix it! */
	pRaEntry->TxPhyCfg.ldpc = 0;

	if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE))
		pRaEntry->TxPhyCfg.ldpc |= VHT_LDPC;

	if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE))
		pRaEntry->TxPhyCfg.ldpc |= HT_LDPC;

#endif
#ifdef DOT11_VHT_AC

	if (tx_mode == MODE_VHT) {
		if ((CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI80_CAPABLE)) &&
			(pTxRate->ShortGI))
			pRaEntry->TxPhyCfg.ShortGI = GI_400;
		else
			pRaEntry->TxPhyCfg.ShortGI = GI_800;

		if (pTxRate->STBC && (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE)))
			pRaEntry->TxPhyCfg.STBC = STBC_USE;
		else
			pRaEntry->TxPhyCfg.STBC = STBC_NONE;
	}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	if (pTxRate->CurrMCS < MCS_AUTO)
		pRaEntry->TxPhyCfg.MCS = pTxRate->CurrMCS;

#ifdef DOT11_N_SUPPORT

	if ((pRaCfg->HtMode == HTMODE_GF) &&
		(pRaEntry->fgHtCapInfoGF == HTMODE_GF))
		pRaEntry->TxPhyCfg.MODE = MODE_HTGREENFIELD;
	else
		pRaEntry->TxPhyCfg.MODE = tx_mode;

	if ((pRaCfg->TestbedForceGreenField & pRaEntry->fgHtCapInfoGF) &&
		(pRaEntry->TxPhyCfg.MODE == MODE_HTMIX)) {
		/* force Tx GreenField */
		pRaEntry->TxPhyCfg.MODE = MODE_HTGREENFIELD;
	}

	/* BW depends on BSSWidthTrigger and Negotiated BW */
	if (pRaCfg->bRcvBSSWidthTriggerEvents ||
		(pRaEntry->MaxPhyCfg.BW == BW_20) ||
		(pRaEntry->ucBBPCurrentBW == BW_20))
		pRaEntry->TxPhyCfg.BW = BW_20;
	else
		pRaEntry->TxPhyCfg.BW = BW_40;

#ifdef DOT11_VHT_AC

	if ((pRaEntry->ucBBPCurrentBW == BW_80) &&
		(pRaEntry->MaxPhyCfg.BW == BW_80) &&
		pRaEntry->MaxPhyCfg.MODE == MODE_VHT)
		pRaEntry->TxPhyCfg.BW = BW_80;

#ifdef NEW_RATE_ADAPT_SUPPORT

	if ((pTable == RateTableVht2S) ||
		(pTable == RateTableVht2S_BW20) ||
		(pTable == RateTableVht2S_BW40) ||
		(pTable == RateTableVht1S) ||
		(pTable == RateTableVht1S_MCS9) ||
		(pTable == RateTableVht2S_MCS7)) {
		RTMP_RA_GRP_TB *pAdaptTbEntry = (RTMP_RA_GRP_TB *)pTxRate;

		pRaEntry->TxPhyCfg.MCS = pAdaptTbEntry->CurrMCS;
		pRaEntry->TxPhyCfg.VhtNss = pAdaptTbEntry->dataRate;
		pRaEntry->TxPhyCfg.BW = tx_bw;
	} else if (pRaEntry->MaxPhyCfg.MODE == MODE_VHT) {
		UCHAR bw_max = pRaEntry->MaxPhyCfg.BW;

		if (pRaEntry->force_op_mode == TRUE) {
			switch (pRaEntry->vhtOpModeChWidth) {
			case 1:
				bw_max = BW_40;
				break;

			case 2: /* not support for BW_80 for other rate table */
			case 0:
			default:
				bw_max = BW_20;
				break;
			}
		}

		if ((bw_max != BW_10) &&
			(bw_max > pRaEntry->ucBBPCurrentBW))
			bw_max = pRaEntry->ucBBPCurrentBW;

		pRaEntry->TxPhyCfg.BW = bw_max;
	}

#endif /* NEW_RATE_ADAPT_SUPPORT */
#endif /* DOT11_VHT_AC */

	/* Reexam each bandwidth's SGI support. */
	if (((pRaEntry->TxPhyCfg.BW == BW_20) && !CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI20_CAPABLE)) ||
		((pRaEntry->TxPhyCfg.BW == BW_40) && !CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI40_CAPABLE)))
		pRaEntry->TxPhyCfg.ShortGI = GI_800;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_N_SUPPORT

	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pRaEntry->TxPhyCfg.MCS == MCS_32) {
		if ((pRaEntry->TxPhyCfg.BW != BW_40) && (pRaEntry->TxPhyCfg.BW != BW_80))
			pRaEntry->TxPhyCfg.MCS = 0;
	}

#endif /*  DOT11_N_SUPPORT */
#ifdef MCS_LUT_SUPPORT
	MtAsicMcsLutUpdateCore(pAd, pRaEntry, pRaCfg, pRaInternal);
#endif /* MCS_LUT_SUPPORT */
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     The entrypoint of set new tx rate
 *
 * \param[in] pAd
 * \param[in] pRaEntry
 * \param[in] pRaCfg
 * \param[in] pRaInternal
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
NewTxRateMtCore(
	IN PRTMP_ADAPTER pAd,
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal
)
{
	RTMP_RA_GRP_TB *pNextTxRate;
	UCHAR *pTable;

	if ((pRaEntry == NULL) || (pRaInternal->pucTable == NULL))
		return;
	pTable = pRaInternal->pucTable;

	/*  Get pointer to CurrTxRate entry */
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (ADAPT_RATE_TABLE(pTable))
		pNextTxRate = PTX_RA_GRP_ENTRY(pTable, pRaInternal->ucCurrTxRateIndex);
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Not GRP table!\n", __func__));
		return;
	}

#ifdef NEW_RATE_ADAPT_SUPPORT

	if (pRaEntry->fgAuthWapiMode) {
		if (pTable == RateSwitchTableAdapt11N2S) {
			if ((pRaInternal->ucCurrTxRateIndex >= 14) && (pRaInternal->ucCurrTxRateIndex <= 16))
				pNextTxRate = PTX_RA_GRP_ENTRY(pTable, 13);
		}
	}

#endif /* NEW_RATE_ADAPT_SUPPORT */
	/*  Set new rate */
	SetTxRateMtCore(pAd, pRaEntry, pRaCfg, pRaInternal, pNextTxRate);
#ifdef DOT11_N_SUPPORT

	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pRaEntry->TxPhyCfg.MCS == MCS_32) {
		if ((pRaEntry->TxPhyCfg.BW != BW_40) && (pRaEntry->TxPhyCfg.BW != BW_80))
			pRaEntry->TxPhyCfg.MCS = 0;
		else
			pRaEntry->TxPhyCfg.STBC = 0;
	}

#endif /*  DOT11_N_SUPPORT */
#ifndef COMPOS_WIN
	RA_SAVE_LAST_TX_CFG(pRaEntry);
#endif /* !COMPOS_WIN */
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))
/*----------------------------------------------------------------------------*/
/*!
 * \brief     Set TxPhy according to MaxPhy, rate table and config
 *
 * \param[in] pAd
 * \param[in] pRaEntry
 * \param[in] pRaCfg
 * \param[in] pRaInternal
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
SetTxRateMtCoreAGBS(
	IN PRTMP_ADAPTER pAd,
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal
)
{
	UINT_8 ucTxMode;
	RA_AGBS_TABLE_ENTRY *pTxRate;
	UCHAR *pTable;
	UINT_8 nsts = 1;
	UINT_16 u2PhyRate;

	if ((pRaEntry == NULL) || (pRaInternal->pucTable == NULL))
		return;
	pTable = pRaInternal->pucTable;

	/*  Get pointer to CurrTxRate entry */
	if (RATE_TABLE_AGBS(pTable))
		pTxRate = RA_AGBS_ENTRY(pTable, pRaInternal->ucCurrTxRateIndex);
	else {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Not AGBS table!\n", __func__));
		return;
	}

	/* TODO: check 7615 need it? */
	ucTxMode = pTxRate->Mode;
	/* STBC and GI */
	pRaEntry->TxPhyCfg.STBC = STBC_NONE;
	pRaEntry->TxPhyCfg.ShortGI = GI_800;

	if (ucTxMode == MODE_HTMIX || ucTxMode == MODE_HTGREENFIELD) {
		if ((pTxRate->STBC) && (pRaEntry->MaxPhyCfg.STBC))
			pRaEntry->TxPhyCfg.STBC = STBC_USE;

		/* if (pTxRate->ShortGI || pRaCfg->TestbedForceShortGI) */
		if (((pRaEntry->ucCERMSD > RA_RMDS_THRD) || (pTxRate->ShortGI) || (pRaInternal->ucDynamicSGIState == RA_DYNAMIC_SGI_TRY_SUCCESS_STATE)) ||
			pRaCfg->TestbedForceShortGI) {
			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI20_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_20;

			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI40_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_40;
		}
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	if (ucTxMode == MODE_VHT) {
		if (pTxRate->STBC && (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE)))
			pRaEntry->TxPhyCfg.STBC = STBC_USE;

		/* if (pTxRate->ShortGI */
		if (((pRaEntry->ucCERMSD > RA_RMDS_THRD) || (pTxRate->ShortGI) || (pRaInternal->ucDynamicSGIState == RA_DYNAMIC_SGI_TRY_SUCCESS_STATE))) {
			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI20_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_20;

			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI40_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_40;

			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI80_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_80;

			if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_SGI160_CAPABLE))
				pRaEntry->TxPhyCfg.ShortGI |= SGI_160;
		}
	}

#endif /* DOT11_VHT_AC */
	/* LDPC */
#ifndef COMPOS_WIN
	/* TODO: Lens, fix it! */
	pRaEntry->TxPhyCfg.ldpc = 0;

	if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_HT_RX_LDPC_CAPABLE))
		pRaEntry->TxPhyCfg.ldpc |= HT_LDPC;

#ifdef DOT11_VHT_AC

	if (CLIENT_STATUS_TEST_FLAG(pRaEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE))
		pRaEntry->TxPhyCfg.ldpc |= VHT_LDPC;

#endif /* DOT11_VHT_AC */
#endif
#endif /* DOT11_N_SUPPORT */

	/* MCS */
	if (pTxRate->CurrMCS < MCS_AUTO)
		pRaEntry->TxPhyCfg.MCS = pTxRate->CurrMCS;

	/* PHY Mode */
	pRaEntry->TxPhyCfg.MODE = ucTxMode;

	if ((pRaCfg->HtMode == HTMODE_GF) &&
		(pRaEntry->fgHtCapInfoGF == HTMODE_GF))
		pRaEntry->TxPhyCfg.MODE = MODE_HTGREENFIELD;

	if ((pRaCfg->TestbedForceGreenField & pRaEntry->fgHtCapInfoGF) &&
		(pRaEntry->TxPhyCfg.MODE == MODE_HTMIX)) {
		/* force Tx GreenField */
		pRaEntry->TxPhyCfg.MODE = MODE_HTGREENFIELD;
	}

	/* BW */
	/* TODO: fix it at auto BW */
	/* BW depends on BSSWidthTrigger and Negotiated BW */
	if (pRaCfg->bRcvBSSWidthTriggerEvents ||
		(pRaEntry->MaxPhyCfg.BW == BW_20) ||
		(pRaEntry->ucBBPCurrentBW == BW_20))
		pRaEntry->TxPhyCfg.BW = BW_20;
	else
		pRaEntry->TxPhyCfg.BW = BW_40;

	if (((pRaEntry->ucBBPCurrentBW == BW_80) || (pRaEntry->ucBBPCurrentBW == BW_160) || (pRaEntry->ucBBPCurrentBW == BW_8080)) &&
		(pRaEntry->MaxPhyCfg.BW == BW_80) &&
		(pRaEntry->MaxPhyCfg.MODE == MODE_VHT))
		pRaEntry->TxPhyCfg.BW = BW_80;

	if (((pRaEntry->ucBBPCurrentBW == BW_160) || (pRaEntry->ucBBPCurrentBW == BW_8080)) &&
		(pRaEntry->MaxPhyCfg.BW == BW_160) &&
		(pRaEntry->MaxPhyCfg.MODE == MODE_VHT))
		pRaEntry->TxPhyCfg.BW = BW_160;

	if (pRaInternal->ucDynamicBWState != RA_DYNAMIC_BW_UNCHANGED_STATE)
		pRaEntry->TxPhyCfg.BW = pRaInternal->ucDynamicBW;

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(pRaEntry->ucPhyMode) &&
		(pRaEntry->ucSupportRateMode & SUPPORT_VHT_MODE) &&
		AGBS_VHT_TABLE(pRaInternal->pucTable)) {
		UINT_8 ucBwCap = BW_20;

		pRaEntry->TxPhyCfg.VhtNss = pRaInternal->ucMcsGroup;

		if (pRaEntry->force_op_mode == TRUE) {
			switch (pRaEntry->vhtOpModeChWidth) {
			case 1:
				ucBwCap = BW_40;
				break;

			case 2:
				ucBwCap = BW_80;
				break;

			case 3:
				ucBwCap = BW_160;
				break;

			case 0:
			default:
				ucBwCap = BW_20;
				break;
			}

			if (ucBwCap <= pRaEntry->MaxPhyCfg.BW)
				pRaEntry->TxPhyCfg.BW = ucBwCap;

			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): TxPhyCfg.BW=%d, ucBwCap=%d\n", __func__, pRaEntry->TxPhyCfg.BW, ucBwCap));
		}
	}

#endif /* DOT11_VHT_AC */

	if ((pRaEntry->ucGband256QAMSupport) && (ucTxMode == MODE_VHT))
		pRaEntry->TxPhyCfg.VhtNss = pRaInternal->ucMcsGroup;

#ifdef DOT11_N_SUPPORT

	/*  Disable invalid HT Duplicate modes to prevent PHY error */
	if (pRaEntry->TxPhyCfg.MCS == MCS_32) {
		if ((pRaEntry->TxPhyCfg.BW != BW_40) && (pRaEntry->TxPhyCfg.BW != BW_80) &&
			(pRaEntry->TxPhyCfg.BW != BW_160) && (pRaEntry->TxPhyCfg.BW != BW_8080))
			pRaEntry->TxPhyCfg.MCS = 0;
	}

	if (pRaCfg->u2MaxPhyRate != 0) {
		u2PhyRate = raGetPhyRate(pRaEntry->TxPhyCfg.MODE, pRaEntry->TxPhyCfg.MCS,
								 pRaEntry->TxPhyCfg.VhtNss, pRaEntry->TxPhyCfg.BW, pRaEntry->TxPhyCfg.ShortGI);

		if ((pRaCfg->u2MaxPhyRate != 0) && (u2PhyRate > pRaCfg->u2MaxPhyRate)
			&& (pRaCfg->TestbedForceShortGI == FALSE))
			pRaEntry->TxPhyCfg.ShortGI = GI_800;
	}

#endif /*  DOT11_N_SUPPORT */

	if (pRaCfg->ucForceTxStream != 0) {
		if ((pRaEntry->TxPhyCfg.MODE == MODE_HTMIX) || (pRaEntry->TxPhyCfg.MODE == MODE_HTMIX)) {
			if (pRaEntry->TxPhyCfg.MCS != MCS_32) {
				nsts += (pRaEntry->TxPhyCfg.MCS >> 3);

				if ((pRaEntry->TxPhyCfg.STBC == STBC_USE) && (nsts == 1))
					nsts++;
			}

			if (nsts > pRaCfg->ucForceTxStream) {
				if ((pRaEntry->TxPhyCfg.STBC == STBC_USE) && (pRaCfg->ucForceTxStream == 1))
					pRaEntry->TxPhyCfg.STBC = STBC_NONE;
				else
					pRaEntry->TxPhyCfg.MCS = (pRaEntry->TxPhyCfg.MCS & 0x7) + ((pRaCfg->ucForceTxStream - 1) << 3);
			}
		} else if (pRaEntry->TxPhyCfg.MODE == MODE_VHT) {
			if ((pRaEntry->TxPhyCfg.STBC == STBC_USE) && (pRaEntry->TxPhyCfg.VhtNss == 1))
				nsts++;
			else
				nsts = pRaEntry->TxPhyCfg.VhtNss;

			if (nsts > pRaCfg->ucForceTxStream) {
				if ((pRaEntry->TxPhyCfg.STBC == STBC_USE) && (pRaCfg->ucForceTxStream == 1))
					pRaEntry->TxPhyCfg.STBC = STBC_NONE;
				else
					pRaEntry->TxPhyCfg.VhtNss = pRaCfg->ucForceTxStream;
			}
		}
	}

#ifdef MCS_LUT_SUPPORT
	MtAsicMcsLutUpdateCoreAGBS(pAd, pRaEntry, pRaCfg, pRaInternal);
#endif /* MCS_LUT_SUPPORT */
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     Notify max AMSDU length to CR4
 *
 * \param[in] pRaEntry
 * \param[in] pRaInternal
 * \param[OUT]
 *
 * \return    none
 */
/*----------------------------------------------------------------------------*/
VOID
raMaxAmsduLenNotifyAGBS(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_INTERNAL_INFO_T pRaInternal
)
{
#ifdef WIFI_BUILD_RAM
#if (PRODUCT_VERSION == 7615)
	UINT_8 ucTxMode;
	RA_AGBS_TABLE_ENTRY *pTxRate;
	UCHAR *pTable;
	UINT_8 ucMaxAmsduLength = 0;

	if ((pRaEntry == NULL) || (pRaInternal->pucTable == NULL))
		return;
	pTable = pRaInternal->pucTable;

	/*  Get pointer to CurrTxRate entry */
	if (RATE_TABLE_AGBS(pTable))
		pTxRate = RA_AGBS_ENTRY(pTable, pRaInternal->ucCurrTxRateIndex);
	else {
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:Not AGBS table!\n", __func__));
		return;
	}

	switch (pRaEntry->TxPhyCfg.BW) {
	case BW_20:
		ucMaxAmsduLength = pTxRate->AmsduLenBw20;
		break;

	case BW_40:
		ucMaxAmsduLength = pTxRate->AmsduLenBw40;
		break;

	case BW_80:
		ucMaxAmsduLength = pTxRate->AmsduLenBw80;
		break;

	case BW_160:
		ucMaxAmsduLength = pTxRate->AmsduLenBw160;
		break;

	default:
		ucMaxAmsduLength = 0;
		break;
	}

	hemExtEventMaxAMSDULengthUpdate(pRaEntry->ucWcid, ucMaxAmsduLength);
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMaxAmsduLength=%d\n", ucMaxAmsduLength));
#endif
#endif /* WIFI_BUILD_RAM */
}
#endif /* RATE_ADAPT_AGBS_SUPPORT */


/*----------------------------------------------------------------------------*/
/*!
 * \brief     The entrypoint of select tx rate table.
 *
 * \param[in] pRaEntry
 * \param[in] pRaCfg
 * \param[in] pRaInternal
 * \param[OUT] ppTable
 * \param[OUT] pTableSize
 * \param[OUT] pInitTxRateIdx
 *
 * \return    none
 */
/*----------------------------------------------------------------------------*/
VOID
raSelectTxRateTable(
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal,
	OUT UCHAR **ppTable,
	OUT UCHAR *pTableSize,
	OUT UCHAR *pInitTxRateIdx)
{
	*ppTable = NULL;

	do {
#ifdef DOT11_VHT_AC
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (pRaCfg->ucRateAlg == RATE_ALG_GRP)
			*ppTable = raSelectVHTTxRateTableGRP(pRaEntry, pRaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))

		if (pRaCfg->ucRateAlg == RATE_ALG_AGBS)
			*ppTable = raSelectVHTTxRateTableAGBS(pRaEntry, pRaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */

		if (*ppTable)
			break;

#endif /* DOT11_VHT_AC */
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (pRaCfg->ucRateAlg == RATE_ALG_GRP)
			*ppTable = raSelectTxRateTableGRP(pRaEntry, pRaCfg, pRaInternal);

#endif /* NEW_RATE_ADAPT_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))

		if (pRaCfg->ucRateAlg == RATE_ALG_AGBS)
			*ppTable = raSelectTxRateTableAGBS(pRaEntry, pRaCfg, pRaInternal);

#endif /* RATE_ADAPT_AGBS_SUPPORT */
	} while (FALSE);

#ifdef NEW_RATE_ADAPT_SUPPORT

	if (pRaCfg->ucRateAlg == RATE_ALG_GRP) {
		if (ADAPT_RATE_TABLE(*ppTable) == FALSE) {
			*ppTable = RateSwitchTableAdapt11B;
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Rate Table, Set to RateSwitchTableAdapt11B.\n", __func__));
		}
	}

#endif /* NEW_RATE_ADAPT_SUPPORT */
#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))

	if (pRaCfg->ucRateAlg == RATE_ALG_AGBS) {
		if (RATE_TABLE_AGBS(*ppTable) == FALSE) {
			*ppTable = RateSwitchTableAGBS11B;
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Rate Table, Set to RateSwitchTableAGBS11B\n", __func__));
		}
	}

#endif /* RATE_ADAPT_AGBS_SUPPORT */

	if (*ppTable) {
		*pTableSize = RATE_TABLE_SIZE(*ppTable);
		*pInitTxRateIdx = RATE_TABLE_INIT_INDEX(*ppTable);
	} else
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:TX rate table is Null!\n", __func__));
}


#ifdef MCS_LUT_SUPPORT
#ifdef WIFI_BUILD_RAM
/*----------------------------------------------------------------------------*/
/*!
 * \brief     convert TX rate format to TMI format (copy from mt_mac.c)
 *
 * \param[in] mode
 * \param[in] mcs
 * \param[in] nss
 * \param[in] stbc
 * \param[in] preamble
 *
 * \return    TMI rate
 */
/*----------------------------------------------------------------------------*/
UINT_16
tx_rate_to_tmi_rate(
	IN UINT_8 mode,
	IN UINT_8 mcs,
	IN UINT_8 nss,
	IN BOOL stbc,
	IN UINT_8 preamble
)
{
	UINT_16 tmi_rate = 0, mcs_id = 0;

	stbc = (stbc == TRUE) ? 1 : 0;

	switch (mode) {
	case MODE_CCK:
		if (preamble)
			mcs_id = tmi_rate_map_cck_lp[mcs];
		else
			mcs_id = tmi_rate_map_cck_sp[mcs];

		tmi_rate = (TMI_TX_RATE_MODE_CCK << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_OFDM:
		mcs_id = tmi_rate_map_ofdm[mcs];
		tmi_rate = (TMI_TX_RATE_MODE_OFDM << TMI_TX_RATE_BIT_MODE) | (mcs_id);
		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		tmi_rate = ((USHORT)(stbc << TMI_TX_RATE_BIT_STBC)) |
				   (((nss - 1) & TMI_TX_RATE_MASK_NSS) << TMI_TX_RATE_BIT_NSS) |
				   ((USHORT)(mode << TMI_TX_RATE_BIT_MODE)) |
				   ((USHORT)(mcs));
		/* MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): mode=%d, mcs=%d, stbc=%d converted tmi_rate=0x%x\n", */
		/* __FUNCTION__, mode, mcs, stbc, tmi_rate)); */
		break;

	case MODE_VHT:
		tmi_rate = TMI_TX_RATE_VHT_VAL(nss, mcs, stbc);
		break;

	default:
		MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid mode(mode=%d)\n",
				 __func__, mode));
		break;
	}

	return tmi_rate;
}


/*----------------------------------------------------------------------------*/
/*!
 * \brief     get nss by mcs setting (copy from mt_mac.c)
 *
 * \param[in] phy_mode
 * \param[in] mcs
 * \param[in] stbc
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
UINT_8
get_nsts_by_mcs(
	UINT_8 phy_mode,
	UINT_8 mcs,
	BOOL stbc,
	UINT_8 vht_nss
)
{
	UINT_8 nsts = 1;

	switch (phy_mode) {
	case MODE_VHT:
		if (stbc && (vht_nss == 1))
			nsts++;
		else
			nsts = vht_nss;

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (mcs != 32) {
			nsts += (mcs >> 3);

			if (stbc && (nsts == 1))
				nsts++;
		}

		break;

	case MODE_CCK:
	case MODE_OFDM:
	default:
		break;
	}

	return nsts;
}
#endif /* WIFI_BUILD_RAM */


/*----------------------------------------------------------------------------*/
/*!
 * \brief     check if STBC setting violate 802.11 spec and HW limit
 *
 * \param[in] ucOrigStbc
 * \param[in] ucMode
 * \param[in] ucMcs
 * \param[in] ucVhtNss
 * \param[in] fgForceOneTx
 *
 * \return    STBC
 */
/*----------------------------------------------------------------------------*/
UINT_8
raStbcSettingCheck(
	UINT_8 ucOrigStbc,
	UINT_8 ucMode,
	UINT_8 ucMcs,
	UINT_8 ucVhtNss,
	BOOL fgBFOn,
	BOOL fgForceOneTx
)
{
	UINT_8 ucStbc = 0;

	if (fgForceOneTx == TRUE)
		return ucStbc;

	if (fgBFOn == TRUE)
		return ucStbc;

	switch (ucMode) {
	case MODE_VHT:
		if (ucVhtNss == 1)
			ucStbc = ucOrigStbc;
		else
			ucStbc = 0;

		break;

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (ucMcs < MCS_8)
			ucStbc = ucOrigStbc;
		else
			ucStbc = 0;

		break;

	case MODE_CCK:
	case MODE_OFDM:
	default:
		ucStbc = 0;
		break;
	}

	return ucStbc;
}


#ifdef NEW_RATE_ADAPT_SUPPORT
/*----------------------------------------------------------------------------*/
/*!
 * \brief     set tx rate table in wtbl2 by TxPhyCfg
 *
 * \param[in] phy_mode
 * \param[in] mcs
 * \param[in] stbc
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
MtAsicMcsLutUpdateCore(
	IN PRTMP_ADAPTER pAd,
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal
)
{
	UINT_32 rate[8];
	UINT_8 stbc, nsts, preamble, ucVhtNss;
	BOOL fgSpeEn = FALSE;
	UINT_8 ucIndex;
	BOOL fgBFOn = FALSE;
	/* CHAR rssi; */

	if (pRaCfg->fgShortPreamble == TRUE)
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	stbc = raStbcSettingCheck(pRaEntry->TxPhyCfg.STBC,
							  pRaEntry->TxPhyCfg.MODE,
							  pRaEntry->TxPhyCfg.MCS,
							  pRaEntry->TxPhyCfg.VhtNss,
							  fgBFOn,
							  pRaCfg->force_one_tx_stream);
	nsts = get_nsts_by_mcs(pRaEntry->TxPhyCfg.MODE, pRaEntry->TxPhyCfg.MCS, stbc, pRaEntry->TxPhyCfg.VhtNss);

	if (((pRaEntry->ucMmpsMode != MMPS_STATIC) || (pRaEntry->TxPhyCfg.MODE < MODE_HTMIX))
		&& (pRaCfg->force_one_tx_stream == FALSE)
	   ) {
		/* rssi = raMaxRssi(pRaCfg, pRaEntry->AvgRssiSample[0], pRaEntry->AvgRssiSample[1], pRaEntry->AvgRssiSample[2]); */
		/* if (rssi < -50 ) */
		{
			fgSpeEn = TRUE;
		}
	}

	rate[0] = tx_rate_to_tmi_rate(pRaEntry->TxPhyCfg.MODE,
								  pRaEntry->TxPhyCfg.MCS,
								  nsts,
								  stbc,
								  preamble);
	rate[0] &= 0xfff;

	if (pRaEntry->fgAutoTxRateSwitch == TRUE) {
		BOOL fgLowestRate = FALSE;
		UCHAR DownRateIdx, CurrRateIdx;
		UCHAR mode, mcs;

		CurrRateIdx = pRaInternal->ucCurrTxRateIndex;
		DownRateIdx = CurrRateIdx;

		for (ucIndex = 1; ucIndex < 8 ; ucIndex++) {
			if (ADAPT_RATE_TABLE(pRaInternal->pucTable)) {
				RTMP_RA_GRP_TB *pCurrTxRate;

				if (ucIndex == 7) {
					if (fgLowestRate == FALSE) {
						do {
							CurrRateIdx = DownRateIdx;
							DownRateIdx = raSelectDownRate(pRaEntry, pRaCfg, pRaInternal, CurrRateIdx);
						} while (CurrRateIdx != DownRateIdx);
					}
				} else {
					if (fgLowestRate == FALSE)
						DownRateIdx = raSelectDownRate(pRaEntry, pRaCfg, pRaInternal, CurrRateIdx);
				}

				if (pRaEntry->TxPhyCfg.ShortGI) {
					pCurrTxRate = PTX_RA_GRP_ENTRY(pRaInternal->pucTable, DownRateIdx);

					if (pCurrTxRate->CurrMCS == pRaEntry->TxPhyCfg.MCS) {
						CurrRateIdx = DownRateIdx;
						DownRateIdx = raSelectDownRate(pRaEntry, pRaCfg, pRaInternal, CurrRateIdx);
					}
				}

				pCurrTxRate = PTX_RA_GRP_ENTRY(pRaInternal->pucTable, DownRateIdx);
				mode = pCurrTxRate->Mode;
				mcs = pCurrTxRate->CurrMCS;
				ucVhtNss = (pRaEntry->TxPhyCfg.MODE == MODE_VHT) ? pCurrTxRate->dataRate : 0;
			} else {
				mode = MODE_CCK;
				mcs = 0;
				DownRateIdx = 0;
				ucVhtNss = 0;
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Not support legacy table.\n", __func__));
			}

			stbc = raStbcSettingCheck(pRaEntry->TxPhyCfg.STBC,
									  mode,
									  mcs,
									  ucVhtNss,
									  fgBFOn,
									  pRaCfg->force_one_tx_stream);
			nsts = get_nsts_by_mcs(mode, mcs, stbc, ucVhtNss);
			rate[ucIndex] = tx_rate_to_tmi_rate(mode,
												mcs,
												nsts,
												stbc,
												preamble);
			rate[ucIndex] &= 0xfff;

			if (CurrRateIdx == DownRateIdx)
				fgLowestRate = TRUE;
			else
				CurrRateIdx = DownRateIdx;
		}
	} else
		rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];

	MtAsicTxCapAndRateTableUpdate(pAd, pRaEntry->ucWcid, &pRaEntry->TxPhyCfg, rate, fgSpeEn);
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():WCID=%d\n",
			 __func__, pRaEntry->ucWcid));
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\tCurTxRateIdx=%d, Mode/BW/MCS/STBC/LDPC/SGI=%d/%d/%d/%d/%d/%d\n\n",
			  pRaInternal->ucCurrTxRateIndex,
			  pRaEntry->TxPhyCfg.MODE,
			  pRaEntry->TxPhyCfg.BW,
			  pRaEntry->TxPhyCfg.MCS,
			  pRaEntry->TxPhyCfg.STBC,
			  pRaEntry->TxPhyCfg.ldpc,
			  pRaEntry->TxPhyCfg.ShortGI));
}
#endif /* NEW_RATE_ADAPT_SUPPORT */


#if defined(RATE_ADAPT_AGBS_SUPPORT) && (!defined(RACTRL_FW_OFFLOAD_SUPPORT) || defined(WIFI_BUILD_RAM))
/*----------------------------------------------------------------------------*/
/*!
 * \brief     set tx rate table in wtbl2 by TxPhyCfg
 *
 * \param[in] phy_mode
 * \param[in] mcs
 * \param[in] stbc
 *
 * \return    None
 */
/*----------------------------------------------------------------------------*/
VOID
MtAsicMcsLutUpdateCoreAGBS(
	IN PRTMP_ADAPTER pAd,
	IN P_RA_ENTRY_INFO_T pRaEntry,
	IN P_RA_COMMON_INFO_T pRaCfg,
	IN P_RA_INTERNAL_INFO_T pRaInternal
)
{
	RA_AGBS_TABLE_ENTRY *pTxRate;
	UINT_32 rate[8];
	UINT_8 stbc, nsts, preamble;
	UINT_8 uc_cbrn = 7;
	BOOL fgSpeEn = FALSE;
	BOOL fgBFOn = FALSE;

	if (pRaCfg->fgShortPreamble == TRUE)
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	stbc = raStbcSettingCheck(pRaEntry->TxPhyCfg.STBC,
							  pRaEntry->TxPhyCfg.MODE,
							  pRaEntry->TxPhyCfg.MCS,
							  pRaEntry->TxPhyCfg.VhtNss,
							  fgBFOn,
							  pRaCfg->force_one_tx_stream);
	nsts = get_nsts_by_mcs(pRaEntry->TxPhyCfg.MODE, pRaEntry->TxPhyCfg.MCS, stbc, pRaEntry->TxPhyCfg.VhtNss);

	if (((pRaEntry->ucMmpsMode != MMPS_STATIC) || (pRaEntry->TxPhyCfg.MODE < MODE_HTMIX))
		&& (pRaCfg->force_one_tx_stream == FALSE)
		&& (pRaCfg->ucForceTxStream == 0)
	   ) {
		if (fgBFOn) {
		} else {
			UINT_8 ucMcs = MCS_7;

			if (pRaEntry->TxPhyCfg.MODE < MODE_HTMIX)
				fgSpeEn = TRUE;
			else {
				if ((pRaEntry->TxPhyCfg.MODE == MODE_HTMIX) || (pRaEntry->TxPhyCfg.MODE == MODE_HTGREENFIELD))
					ucMcs = pRaEntry->TxPhyCfg.MCS & 0x7;
				else if (pRaEntry->TxPhyCfg.MODE == MODE_VHT)
					ucMcs = pRaEntry->TxPhyCfg.MCS;

				if ((pRaEntry->TxPhyCfg.BW == BW_160) && (pRaEntry->TxPhyCfg.MODE == MODE_VHT)) {
					if ((nsts == 1) && (ucMcs <= MCS_6))
						fgSpeEn = TRUE;
				} else {
					if ((nsts == 1) || (nsts == 2)) {
						if (ucMcs <= MCS_6)
							fgSpeEn = TRUE;
					} else if (nsts == 3) {
						if (ucMcs <= MCS_3)
							fgSpeEn = TRUE;
					}
				}
			}
		}
	}

	rate[0] = tx_rate_to_tmi_rate(pRaEntry->TxPhyCfg.MODE,
								  pRaEntry->TxPhyCfg.MCS,
								  nsts,
								  stbc,
								  preamble);
	rate[0] &= 0xfff;

	if (pRaEntry->fgAutoTxRateSwitch == TRUE) {
		UINT_32 u4TableSize;
		UINT_16 *pu2FallbackTable = NULL;
		UINT_8 ucIndex;
		BOOL fgFound = FALSE;

		if (pRaEntry->TxPhyCfg.MODE == MODE_CCK) {
			pu2FallbackTable = HwFallbackTable11B;
			u4TableSize = sizeof(HwFallbackTable11B) / 2;
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11B\n"));
		} else if (pRaInternal->pucTable == RateSwitchTableAGBS11BG) {
			pu2FallbackTable = HwFallbackTable11BG;
			u4TableSize = sizeof(HwFallbackTable11BG) / 2;
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11BG\n"));
		} else if (pRaEntry->TxPhyCfg.MODE == MODE_OFDM) {
			pu2FallbackTable = HwFallbackTable11G;
			u4TableSize = sizeof(HwFallbackTable11G) / 2;
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11G\n"));
		}

#ifdef DOT11_N_SUPPORT
		else if (AGBS_HT_TABLE(pRaInternal->pucTable)) {
			UINT_8 ucHtNss = 1;

			if (pRaEntry->TxPhyCfg.MODE == MODE_VHT)
				ucHtNss = pRaEntry->TxPhyCfg.VhtNss;
			else
				ucHtNss += (pRaEntry->TxPhyCfg.MCS >> 3);

			if ((pRaEntry->ucChannel <= 14) && (pRaEntry->ucSupportRateMode & (SUPPORT_CCK_MODE))) {
				switch (ucHtNss) {
				case 1:
					pu2FallbackTable = HwFallbackTableBGN1SS;
					u4TableSize = sizeof(HwFallbackTableBGN1SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableBGN1SS\n"));
					break;

				case 2:
					pu2FallbackTable = HwFallbackTableBGN2SS;
					u4TableSize = sizeof(HwFallbackTableBGN2SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableBGN2SS\n"));
					break;

				case 3:
					pu2FallbackTable = HwFallbackTableBGN3SS;
					u4TableSize = sizeof(HwFallbackTableBGN3SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableBGN3SS\n"));
					break;

				case 4:
					pu2FallbackTable = HwFallbackTableBGN4SS;
					u4TableSize = sizeof(HwFallbackTableBGN4SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableBGN4SS\n"));
					break;

				default:
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
					break;
				}
			} else {
				switch (ucHtNss) {
				case 1:
					pu2FallbackTable = HwFallbackTable11N1SS;
					u4TableSize = sizeof(HwFallbackTable11N1SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11N1SS\n"));
					break;

				case 2:
					pu2FallbackTable = HwFallbackTable11N2SS;
					u4TableSize = sizeof(HwFallbackTable11N2SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11N2SS\n"));
					break;

				case 3:
					pu2FallbackTable = HwFallbackTable11N3SS;
					u4TableSize = sizeof(HwFallbackTable11N3SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11N3SS\n"));
					break;

				case 4:
					pu2FallbackTable = HwFallbackTable11N4SS;
					u4TableSize = sizeof(HwFallbackTable11N4SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTable11N4SS\n"));
					break;

				default:
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", ucHtNss));
					break;
				}
			}
		}

#ifdef DOT11_VHT_AC
		else if (AGBS_VHT_TABLE(pRaInternal->pucTable)) {
			switch (pRaEntry->TxPhyCfg.VhtNss) {
			case 1:
				pu2FallbackTable = HwFallbackTableVht1SS;
				u4TableSize = sizeof(HwFallbackTableVht1SS) / 2;
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableVht1SS\n"));
				break;

			case 2:
				pu2FallbackTable = HwFallbackTableVht2SS;
				u4TableSize = sizeof(HwFallbackTableVht2SS) / 2;
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableVht2SS\n"));
				break;

			case 3:
				{
					pu2FallbackTable = HwFallbackTableVht3SS;
					u4TableSize = sizeof(HwFallbackTableVht3SS) / 2;
					MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableVht3SS\n"));
				}

				break;

			case 4:
				pu2FallbackTable = HwFallbackTableVht4SS;
				u4TableSize = sizeof(HwFallbackTableVht4SS) / 2;
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HwFallbackTableVht4SS\n"));
				break;

			default:
				MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Nss%d!\n", pRaEntry->TxPhyCfg.VhtNss));
				break;
			}
		}

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

		if (pu2FallbackTable != NULL) {
			for (ucIndex = 0; ucIndex < u4TableSize; ucIndex += 8) {
				union RA_RATE_CODE rInitialRate;

				rInitialRate.word = *(pu2FallbackTable + ucIndex);

				if ((rInitialRate.field.mcs == (rate[0] & TMI_TX_RATE_MASK_MCS)) &&
					(rInitialRate.field.mode == ((rate[0] >> TMI_TX_RATE_BIT_MODE) & TMI_TX_RATE_MASK_MODE))) {
					fgFound = TRUE;
					break;
				}
			}

			if (fgFound) {
				UINT_8 ucIdx;
				union RA_RATE_CODE rRateCode;

				for (ucIdx = 1; ucIdx < 8; ucIdx++) {
					rRateCode.word = *(pu2FallbackTable + ucIndex + ucIdx);

					if (((pRaEntry->TxPhyCfg.MODE == MODE_HTMIX) || (pRaEntry->TxPhyCfg.MODE == MODE_VHT))
						&& stbc && (rRateCode.field.nsts == 0)) {
						rRateCode.field.nsts = 1;
						rRateCode.field.stbc = 1;
					}

					rate[ucIdx] = rRateCode.word;
				}
			}
		}

		if (!fgFound) {
			rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
			MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Cannot find fallback table!\n"));
		}

		/* Set CBRN */
		pTxRate = RA_AGBS_ENTRY(pRaInternal->pucTable, pRaInternal->ucCurrTxRateIndex);
		uc_cbrn = pTxRate->CBRN;
	} else
		rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];

	MtAsicTxCapAndRateTableUpdate(pAd, pRaEntry->ucWcid, &pRaEntry->TxPhyCfg, rate, fgSpeEn);
	MTWF_LOG(DBG_CAT_RA, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DRS: WCID=%d, %s - CurTxRateIdx=%d, Mode/BW/MCS/VHT_NSS/STBC/LDPC/SGI=%d/%d/%d/%d/%d/%d/%d\n",
			 pRaEntry->ucWcid, __func__,
			 pRaInternal->ucCurrTxRateIndex,
			 pRaEntry->TxPhyCfg.MODE,
			 pRaEntry->TxPhyCfg.BW,
			 pRaEntry->TxPhyCfg.MCS,
			 pRaEntry->TxPhyCfg.VhtNss,
			 pRaEntry->TxPhyCfg.STBC,
			 pRaEntry->TxPhyCfg.ldpc,
			 pRaEntry->TxPhyCfg.ShortGI));
}
#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MCS_LUT_SUPPORT */

#endif /* MT_MAC */
#endif
