/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf_mt.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "Cmm_txbf_mt.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif
#include "hdev/hdev.h"

#ifdef TXBF_SUPPORT

#define ETXBF_PROBE_TIME (RA_INTERVAL-100)	/* Wait for Sounding Response will time out 100msec before end of RA interval */


#ifdef MFB_SUPPORT

UCHAR mcsToLowerMcs[] = {
	/*   originalMfb, newMfb1s, newMfb2s, newMfb3s*/
	0,	0,	0,	0,
	1,	1,	1,	1,
	2,	2,	2,	2,
	3,	3,	3,	3,
	4,	4,	4,	4,
	5,	5,	5,	5,
	6,	6,	6,	6,
	7,	7,	7,	7,
	8,	0,	8,	8,
	9,	1,	9,	9,
	10,	2,	10,	10,
	11,	3,	11,	11,
	12,	4,	12,	12,
	13,	5,	13,	13,
	14,	6,	14,	14,
	15,	7,	15,	15,
	16,	0,	8,	16,
	17,	1,	9,	17,
	18,	2,	10,	18,
	19,	3,	11,	19,
	20,	4,	12,	20,
	21,	5,	13,	21,
	22,	6,	14,	22,
	23,	7,	15,	23,
	24,	0,	8,	16,
	25,	1,	9,	17,
	26,	2,	10,	18,
	27,	3,	11,	19,
	28,	4,	12,	20,
	29,	5,	13,	21,
	30,	6,	14,	22,
	31,	7,	15,	23,
	32,	0,	0,	0,
	33,	3,	3,	3,
	34,	3,	3,	3,
	35,	3,	11,	11,
	36,	4,	4,	4,
	37,	6,	6,	6,
	38,	6,	12,	12,
	39,	3,	3,	17,
	40,	3,	11,	11,
	41,	3,	3,	17,
	42,	3,	11,	11,
	43,	3,	11,	19,
	44,	3,	11,	11,
	45,	3,	11,	19,
	46,	4,	4,	18,
	47,	4,	12,	12,
	48,	6,	6,	6,
	49,	6,	12,	12,
	50,	6,	12,	20,
	51,	6,	14,	14,
	52,	6,	14,	14,
	53,	3,	3,	17,
	54,	3,	11,	11,
	55,	3,	11,	19,
	56,	3,	3,	17,
	57,	3,	11,	11,
	58,	3,	11,	19,
	59,	3,	11,	19,
	60,	3,	11,	11,
	61,	3,	11,	19,
	62,	3,	11,	19,
	63,	3,	11,	19,
	64,	3,	11,	19,
	65,	4,	4,	18,
	66,	4,	12,	12,
	67,	4,	12,	20,
	68,	6,	6,	6,
	69,	6,	12,	12,
	70,	6,	12,	20,
	71,	6,	12,	20,
	72,	6,	14,	14,
	73,	6,	14,	14,
	74,	6,	14,	14,
	75,	6,	14,	22,
	76,	6,	14,	22
};
#endif /* MFB_SUPPORT */


#ifdef ETXBF_EN_COND3_SUPPORT
UCHAR groupShift[] = {4, 4, 4};
UCHAR groupMethod[] = {0, 0, 0, 0, 0, 0, 0, 0,
					   0, 0, 1, 0, 1, 1, 1, 1,
					   0, 0, 1, 0, 1, 1, 1, 1
					  };
SHORT groupThrd[] = { -8, 4, 20, 32, 52, 68, 80, 88,
					  -16, 8, 12, 64, 40, 60, 80, 88,
					  -24, 12, 12, 96, 40, 60, 80, 88
					};
UINT dataRate[] = {65, 130, 195, 260, 390, 520, 585, 650,
				   130, 260, 390, 520, 780, 1040, 1170, 1300,
				   190, 390, 585, 780, 1170, 1560, 1755, 1950
				  };
#endif /* ETXBF_EN_COND3_SUPPORT */




#if defined(MT_MAC) && (!defined(MT7636))
/*
 * ==========================================================================
 * Description:
 * Enable sounding trigger
 *
 * Return:
 * TRUE if all parameters are OK, FALSE otherwise
 * ==========================================================================
 */
INT mt_Trigger_Sounding_Packet(
	IN    PRTMP_ADAPTER        pAd,
	IN    UCHAR                SndgEn,
	IN    UINT32               u4SNDPeriod,
	IN    UCHAR                ucSu_Mu,
	IN    UCHAR                ucMuNum,
	IN    PUCHAR               pWlanId)
{
	/* Enable sounding trigger in FW */
	return CmdETxBfSoundingPeriodicTriggerCtrl(pAd,
			SndgEn,
			u4SNDPeriod,
			ucSu_Mu,
			ucMuNum,
			pWlanId);
}

#endif /* MT_MAC && !MT7636 */


#ifdef MT_MAC
/*
 * Set_StaETxBfEnCond_Proc - enable/disable ETxBF
 * usage: iwpriv ra0 set StaETxBfEnCond=dd
 * 0=>disable, 1=>enable
 * Note: After use this command, need to re-run StaStartup()/LinkUp() operations to sync all status.
 * If ETxBfIncapable!=0 then we don't need to reassociate.
 */
UCHAR AsicTxBfEnCondProc(
	IN	RTMP_ADAPTER * pAd,
	IN	TXBF_STATUS_INFO * pTxBfInfo)
{
	MAC_TABLE_ENTRY		  *pEntry;
	UCHAR ucETxBfEn;
	UCHAR ucWlanIdx;

	for (ucWlanIdx = 0; VALID_UCAST_ENTRY_WCID(pAd, ucWlanIdx); ucWlanIdx++) {
		pEntry = &pAd->MacTab.Content[ucWlanIdx];

		if (!IS_ENTRY_NONE(pEntry)) {
#ifdef VHT_TXBF_SUPPORT

			if (WMODE_CAP_AC(pTxBfInfo->ucPhyMode) && (pTxBfInfo->u2Channel > 14)) {
				ucETxBfEn = mt_WrapClientSupportsVhtETxBF(pAd, pTxBfInfo->pVhtTxBFCap) ? TRUE : FALSE;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("VHT mode!\n"));
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("STA : Bfee Cap =%d, Bfer Cap =%d!\n",
						 pTxBfInfo->pVhtTxBFCap->bfee_cap_su,  pTxBfInfo->pVhtTxBFCap->bfer_cap_su));
			} else
#endif
			{
				ucETxBfEn = mt_WrapClientSupportsETxBF(pAd, pTxBfInfo->pHtTxBFCap) ? TRUE : FALSE;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("HT mode!\n"));
			}

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Final ETxBF status =%d!\n", ucETxBfEn));

			if (ucETxBfEn) {
				CmdTxBfTxApplyCtrl(pAd,
								   ucWlanIdx,
								   TRUE,
								   FALSE,
								   TRUE,
								   FALSE);
			}
		}
	}

	return TRUE;
}
#endif /* MT_MAC */

/*
 * TxBFInit - Intialize TxBF fields in pEntry
 * supportsETxBF - TRUE if client supports ETxBF
 */
VOID mt_TxBFInit(
	IN PRTMP_ADAPTER	pAd,
	IN TXBF_STATUS_INFO * pTxBfInfo,
	IN TXBF_MAC_TABLE_ENTRY * pEntryTxBf,
	IN HT_BF_CAP * pTxBFCap,
	IN BOOLEAN			supportsETxBF)
{
	pEntryTxBf->bfState = READY_FOR_SNDG0;
	pEntryTxBf->sndgMcs = 0;
	pEntryTxBf->sndg0Snr0 = 0;
	pEntryTxBf->sndg0Snr1 = 0;
	pEntryTxBf->sndg0Snr2 = 0;
	pEntryTxBf->sndg0Mcs  = 0;
	pEntryTxBf->noSndgCnt = 0;
	pEntryTxBf->eTxBfEnCond = supportsETxBF ? pTxBfInfo->ucETxBfTxEn : 0;
	pEntryTxBf->noSndgCntThrd = NO_SNDG_CNT_THRD;
	pEntryTxBf->ndpSndgStreams = pTxBfInfo->ucTxPathNum;
	/* If client supports ETxBf and ITxBF then give ETxBF priority over ITxBF */
	pEntryTxBf->iTxBfEn = pEntryTxBf->eTxBfEnCond > 0 ? 0 : pTxBfInfo->ucITxBfTxEn;

	if (supportsETxBF) {
		UCHAR ndpSndgStreams = pTxBfInfo->ucTxPathNum;

		if ((pTxBFCap->ExpComBF > 0) && (!pTxBfInfo->cmmCfgETxBfNoncompress)) {
			if ((pTxBFCap->ComSteerBFAntSup + 1) < pTxBfInfo->ucTxPathNum)
				ndpSndgStreams = pTxBFCap->ComSteerBFAntSup + 1;
		}

		if ((pTxBFCap->ExpNoComBF > 0) && (pTxBfInfo->cmmCfgETxBfNoncompress)) {
			if ((pTxBFCap->NoComSteerBFAntSup + 1) <  pTxBfInfo->ucTxPathNum)
				ndpSndgStreams = pTxBFCap->NoComSteerBFAntSup + 1;
		}

		pEntryTxBf->ndpSndgStreams = ndpSndgStreams;
	}
}


/* TxBF Fw Init */
VOID mt_TxBFFwInit(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.ETxBfEnCond) {
#if defined(MT_MAC) && (!defined(MT7636))
		/* Enable periodic sounding */
		mt_Trigger_Sounding_Packet(pAd,
								   TRUE,
								   0,
								   4,
								   0,
								   NULL);
#endif
	} else {
#if defined(MT_MAC) && (!defined(MT7636))
		/* Disable periodic sounding */
		mt_Trigger_Sounding_Packet(pAd,
								   FALSE,
								   0,
								   0,
								   0,
								   NULL);
#endif
	}
}


/* clientSupportsETxBF - returns true if client supports compatible Sounding */
BOOLEAN mt_clientSupportsETxBF(
	IN	PRTMP_ADAPTER	 pAd,
	IN	HT_BF_CAP * pTxBFCap,
	IN  BOOLEAN          ETxBfNoncompress)
{
	BOOLEAN compCompat, noncompCompat;

	compCompat    = (pTxBFCap->ExpComBF > 0) &&
					/*(pTxBFCap->ComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath) && */
					(ETxBfNoncompress == 0);
	noncompCompat = (pTxBFCap->ExpNoComBF > 0)
					/* && (pTxBFCap->NoComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath)*/;
	return pTxBFCap->RxNDPCapable == 1 && (compCompat || noncompCompat);
}


#ifdef VHT_TXBF_SUPPORT
/* clientSupportsETxBF - returns true if client supports compatible Sounding */
BOOLEAN mt_clientSupportsVhtETxBF(
	IN	PRTMP_ADAPTER	pAd,
	IN	VHT_CAP_INFO * pTxBFCap)
{
	BOOLEAN fgBfeeCap = FALSE;

#ifdef CFG_SUPPORT_MU_MIMO
	fgBfeeCap = pTxBFCap->bfee_cap_su || pTxBFCap->bfee_cap_mu;
#else
	fgBfeeCap = pTxBFCap->bfee_cap_su;
#endif

	return fgBfeeCap;
}
#endif /* VHT_TXBF_SUPPORT */

/* Nr,Nc */
UINT_8   g_ru2PfmuMemReq[4][4] = {
	{0,  0,  0, 0}, /* None,None,None,None */
	{1,  1,  0, 0}, /* 2//x1,2x2,2x3,2x4 */
	{2,  4,  4, 0}, /* 3//x1,3x2,3x3,3x4 */
	{3,  5,  6, 0}
};/* 4x1,4x2,4x3,4x4 */

UINT_8  g_aPfmuTimeOfMem20M[4] = {0, 1, 2, 2};

INT32 mt_AsicBfStaRecUpdate(
	RTMP_ADAPTER * pAd,
	UCHAR        ucPhyMode,
	UCHAR        ucBssIdx,
	UCHAR        ucWlanIdx)
{
	PMAC_TABLE_ENTRY pEntry;
	UCHAR   ucBFerMaxNr,       ucBFeeMaxNr,          ucBFerCurrNr;
	UCHAR   ucStatus   = TRUE;
	UCHAR   ucPeerRxNumSupport;
	UCHAR   ucTxMCSSetdefined, ucTxRxMCSSetNotEqual, ucTxMaxNumSpatilStream;
	BOOLEAN fgETxBfCap = FALSE;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;
	HT_CAPABILITY_IE *ht_cap;
	VHT_CAP_INFO vht_cap;

	pEntry                                   = &pAd->MacTab.Content[ucWlanIdx];

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(pEntry->wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	switch (pEntry->MaxHTPhyMode.field.MODE) {
#ifdef DOT11_VHT_AC

	case MODE_VHT:
		fgETxBfCap                           = mt_WrapClientSupportsVhtETxBF(pAd, &pEntry->vht_cap_ie.vht_cap);
		fgETxBfCap                           &= wlan_config_get_etxbf(pEntry->wdev);
		pEntry->rStaRecBf.fgETxBfCap         = fgETxBfCap;
		pEntry->rStaRecBf.fgSU_MU            = FALSE;
		pEntry->rStaRecBf.ucTxMode           = 4; /* VHT mode */
		pEntry->rStaRecBf.ucCBW              = pEntry->MaxHTPhyMode.field.BW;

		if (fgETxBfCap) {
			mt_WrapSetVHTETxBFCap(pAd, pEntry->wdev, &vht_cap);
#ifdef CFG_SUPPORT_MU_MIMO

			if (vht_cap.bfer_cap_mu == TRUE)
				pEntry->rStaRecBf.fgSU_MU    = pEntry->vht_cap_ie.vht_cap.bfee_cap_mu;

#endif
			pEntry->rStaRecBf.ucSoundingPhy  = 1; /* OFDM */
			pEntry->rStaRecBf.ucNdpaRate     = 9; /* 9 : OFDM 24M */
			pEntry->rStaRecBf.ucNdpRate      = 0; /* 0 : MCS0 */

			if (pEntry->rStaRecBf.fgSU_MU == TRUE)
				pEntry->rStaRecBf.ucNdpRate  = 7; /* 7 : VHT MCS7 */

			pEntry->rStaRecBf.ucReptPollRate = 9; /* 9 : OFDM 24M */
			ucBFerMaxNr                      = vht_cap.num_snd_dimension;
			ucBFeeMaxNr                      = pEntry->vht_cap_ie.vht_cap.bfee_sts_cap;
			ucBFerCurrNr                     = ucTxPath - 1;
			pEntry->rStaRecBf.ucNr           = (ucBFerMaxNr < ucBFeeMaxNr) ? ucBFerMaxNr : ucBFeeMaxNr;
			pEntry->rStaRecBf.ucNr           = (pEntry->rStaRecBf.ucNr < ucBFerCurrNr) ? pEntry->rStaRecBf.ucNr : ucBFerCurrNr;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != 3) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != 3) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != 3) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNcol      = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		} else {
			pEntry->rStaRecBf.ucNr           = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 != 3) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 != 3) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 != 3) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfNcol      = ucPeerRxNumSupport;

			if ((pEntry->MaxHTPhyMode.field.BW <= BW_40) && (pEntry->rStaRecBf.ucNc == 0))
				pEntry->rStaRecBf.uciBfTimeOut   = 0x48;
			else
				pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		}

		break;
#endif /* DOT11_VHT_AC */

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		fgETxBfCap                           = mt_WrapClientSupportsETxBF(pAd, &pEntry->HTCapability.TxBFCap);
		fgETxBfCap                           &= wlan_config_get_etxbf(pEntry->wdev);
		pEntry->rStaRecBf.fgETxBfCap         = fgETxBfCap;
		pEntry->rStaRecBf.ucTxMode           = 2; /* HT mode */
		pEntry->rStaRecBf.ucCBW              = pEntry->MaxHTPhyMode.field.BW;

		if (fgETxBfCap) {
			pEntry->rStaRecBf.fgSU_MU        = 0;
			ht_cap                           = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(pEntry->wdev);
			ucBFerMaxNr                      = ht_cap->TxBFCap.ChanEstimation;
			ucBFeeMaxNr                      = pEntry->HTCapability.TxBFCap.ComSteerBFAntSup;
			ucBFerCurrNr                     = ucTxPath - 1;
			pEntry->rStaRecBf.ucNr           = (ucBFerMaxNr < ucBFeeMaxNr) ? ucBFerMaxNr : ucBFeeMaxNr;
			pEntry->rStaRecBf.ucNr           = (pEntry->rStaRecBf.ucNr < ucBFerCurrNr) ? pEntry->rStaRecBf.ucNr : ucBFerCurrNr;
			pEntry->rStaRecBf.ucNdpRate      = pEntry->rStaRecBf.ucNr * 8;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[1] > 0) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[2] > 0) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[3] > 0) ? 3 : ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNcol      = pEntry->rStaRecBf.ucNc;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		} else {
			pEntry->rStaRecBf.ucNr           = ucTxPath - 1;
			pEntry->rStaRecBf.uciBfDBW       = pEntry->MaxHTPhyMode.field.BW;
			pEntry->rStaRecBf.uciBfNrow      = ucTxPath - 1;
			/* __________________________________________________________ */
			/* |                |Tx MCS Set |Tx Rx MCS|Tx Max Num Spatial| */
			/* |   Condition    |Defined    |Set N EQL|Streams Supported | */
			/* |________________|___________|_________|__________________| */
			/* |No Tx MCS set is|           |         |                  | */
			/* |defined         |     0     |    0    |        0         | */
			/* |________________|___________|_________|__________________| */
			/* |The Tx MCS set  |           |         |                  | */
			/* |defined to be   |           |         |                  | */
			/* |equal to the Rx |     1     |    0    |        0         | */
			/* |MCS set         |           |         |                  | */
			/* |________________|___________|_________|__________________| */
			/* |The Tx MCS set  |           |         |Set to N for N+1  | */
			/* |may differ from |           |         |spatial stream    | */
			/* |The Rx MCS set  |     1     |    1    |                  | */
			/* |________________|___________|_________|__________________| */
			ucTxMCSSetdefined                = ((pEntry->HTCapability.MCSSet[12] &
												 TX_MCS_SET_DEFINED) >>
												TX_MCS_SET_DEFINED_OFFSET);
			ucTxRxMCSSetNotEqual             = ((pEntry->HTCapability.MCSSet[12] &
												 TX_RX_MCS_SET_N_EQUAL) >>
												TX_RX_MCS_SET_N_EQUAL_OFFSET);
			ucTxMaxNumSpatilStream           = ((pEntry->HTCapability.MCSSet[12] &
												 TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED) >>
												TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED_OFFSET);
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[1] > 0) ? 1 : 0;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[2] > 0) ? 2 : ucPeerRxNumSupport;
			ucPeerRxNumSupport               = (pEntry->HTCapability.MCSSet[3] > 0) ? 3 : ucPeerRxNumSupport;

			if ((ucTxMCSSetdefined == 1) && (ucTxRxMCSSetNotEqual == 1))
				ucPeerRxNumSupport           = ucTxMaxNumSpatilStream;

			pEntry->rStaRecBf.ucNc           = ucPeerRxNumSupport;
			pEntry->rStaRecBf.ucNc           = (pEntry->rStaRecBf.ucNc > pEntry->rStaRecBf.ucNr) ? pEntry->rStaRecBf.ucNr : ucPeerRxNumSupport;
			pEntry->rStaRecBf.uciBfNcol      = ucPeerRxNumSupport;

			if ((pEntry->MaxHTPhyMode.field.BW <= BW_40) && (pEntry->rStaRecBf.ucNc == 0))
				pEntry->rStaRecBf.uciBfTimeOut   = 0x48;
			else
				pEntry->rStaRecBf.uciBfTimeOut   = 0x18;
		}

		break;

	case MODE_OFDM:
	case MODE_CCK:
		pEntry->rStaRecBf.fgETxBfCap         = FALSE;
		pEntry->rStaRecBf.ucTxMode           = 1; /* legacy mode */
		pEntry->rStaRecBf.ucNc               = 0;
		pEntry->rStaRecBf.ucNr               = ucTxPath - 1;
		pEntry->rStaRecBf.ucCBW              = pEntry->MaxHTPhyMode.field.BW;
		pEntry->rStaRecBf.uciBfTimeOut       = 0x18;
		pEntry->rStaRecBf.uciBfDBW           = pEntry->MaxHTPhyMode.field.BW;
		pEntry->rStaRecBf.uciBfNcol          = 0;
		pEntry->rStaRecBf.uciBfNrow          = ucTxPath - 1;
		break;

	default:
		ucStatus = FALSE;
		break;
	}

	/* Once find the phy mode is incorrect, return FALSE alarm */
	if (ucStatus == FALSE)
		return ucStatus;

	if (pEntry->rStaRecBf.fgETxBfCap == TRUE) {
		if (pEntry->rStaRecBf.ucNr == (ucTxPath - 1))
			pEntry->rStaRecBf.ucMemRequire20M = g_ru2PfmuMemReq[pEntry->rStaRecBf.ucNr][pEntry->rStaRecBf.ucNc];
		else
			pEntry->rStaRecBf.ucMemRequire20M = g_ru2PfmuMemReq[ucTxPath - 1][pEntry->rStaRecBf.ucNc];
	} else
		pEntry->rStaRecBf.ucMemRequire20M    = g_ru2PfmuMemReq[pEntry->rStaRecBf.ucNr][pEntry->rStaRecBf.ucNc];

	pEntry->rStaRecBf.ucTotMemRequire        = pEntry->rStaRecBf.ucMemRequire20M * g_aPfmuTimeOfMem20M[pEntry->rStaRecBf.ucCBW];
	pEntry->rStaRecBf.ucMemRow0              = 0;
	pEntry->rStaRecBf.ucMemCol0              = 0;
	pEntry->rStaRecBf.ucMemRow1              = 0;
	pEntry->rStaRecBf.ucMemCol1              = 0;
	pEntry->rStaRecBf.ucMemRow2              = 0;
	pEntry->rStaRecBf.ucMemCol2              = 0;
	pEntry->rStaRecBf.ucMemRow3              = 0;
	pEntry->rStaRecBf.ucMemCol3              = 0;
	pEntry->rStaRecBf.u2SmartAnt             = 0;
	/* pEntry->rStaRecBf.ucSEIdx                = 24; */
#ifdef TXBF_DYNAMIC_DISABLE
	pEntry->rStaRecBf.ucAutoSoundingCtrl     = pAd->CommonCfg.ucAutoSoundingCtrl;
#endif /* TXBF_DYNAMIC_DISABLE */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====================== BF StaRec Info =====================\n"));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("fgSU_MU        =%d\n", pEntry->rStaRecBf.fgSU_MU));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("fgETxBfCap     =%d\n", pEntry->rStaRecBf.fgETxBfCap));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucNdpaRate     =%d\n", pEntry->rStaRecBf.ucNdpaRate));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucNdpRate      =%d\n", pEntry->rStaRecBf.ucNdpRate));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucReptPollRate =%d\n", pEntry->rStaRecBf.ucReptPollRate));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucTxMode       =%d\n", pEntry->rStaRecBf.ucTxMode));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucNc           =%d\n", pEntry->rStaRecBf.ucNc));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucNr           =%d\n", pEntry->rStaRecBf.ucNr));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucCBW          =%d\n", pEntry->rStaRecBf.ucCBW));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucTotMemRequire=%d\n", pEntry->rStaRecBf.ucTotMemRequire));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemRequire20M=%d\n", pEntry->rStaRecBf.ucMemRequire20M));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemRow0      =%d\n", pEntry->rStaRecBf.ucMemRow0));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemCol0      =%d\n", pEntry->rStaRecBf.ucMemCol0));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemRow1      =%d\n", pEntry->rStaRecBf.ucMemRow1));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemCol1      =%d\n", pEntry->rStaRecBf.ucMemCol1));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemRow2      =%d\n", pEntry->rStaRecBf.ucMemRow2));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemCol2      =%d\n", pEntry->rStaRecBf.ucMemCol2));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemRow3      =%d\n", pEntry->rStaRecBf.ucMemRow3));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucMemCol3      =%d\n", pEntry->rStaRecBf.ucMemCol3));
#ifdef TXBF_DYNAMIC_DISABLE
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ucAutoSoundingCtrl=%d\n", pEntry->rStaRecBf.ucAutoSoundingCtrl));
#endif /* TXBF_DYNAMIC_DISABLE */

	if (fgETxBfCap == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("uciBfTimeOut   =0x%x\n", pEntry->rStaRecBf.uciBfTimeOut));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("uciBfDBW       =%d\n",   pEntry->rStaRecBf.uciBfDBW));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("uciBfNcol      =%d\n",   pEntry->rStaRecBf.uciBfNcol));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("uciBfNrow      =%d\n",   pEntry->rStaRecBf.uciBfNrow));
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===========================================================\n"));
	return ucStatus;
}


INT32 mt_AsicBfStaRecRelease(
	RTMP_ADAPTER * pAd,
	UCHAR        ucBssIdx,
	UINT8        ucWlanIdx)
{
	PMAC_TABLE_ENTRY pEntry;
	UCHAR ucStatus = TRUE;

	pEntry = &pAd->MacTab.Content[ucWlanIdx];
	/* Clear BF StaRec */
	os_zero_mem(&pEntry->rStaRecBf, sizeof(TXBF_PFMU_STA_INFO));
	pEntry->rStaRecBf.u2PfmuId = 0xFFFF;
	{
		STA_REC_CFG_T StaCfg;

		os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));

		if (!pEntry->wdev) {
			ASSERT(pEntry->wdev);
			return -1;
		}

		StaCfg.MuarIdx = pEntry->wdev->OmacIdx;
		StaCfg.ConnectionState = TRUE;
		StaCfg.ConnectionType = 0;
		StaCfg.u4EnableFeature = (1 << STA_REC_BF);
		StaCfg.ucBssIndex = ucBssIdx;
		StaCfg.ucWlanIdx = ucWlanIdx;
		StaCfg.pEntry = pEntry;

		if (CmdExtStaRecUpdate(pAd, StaCfg) != STATUS_TRUE) {
			ucStatus = FALSE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Something wrong in the BF STA Rec update!!\n"));
		}
	}
	return ucStatus;
}


VOID mt_AsicClientBfCap(
	RTMP_ADAPTER * pAd,
	PMAC_TABLE_ENTRY pEntry)
{
#ifdef APCLI_SUPPORT

	if (pAd->fgApClientMode == TRUE) {
		/* Force the Pfmu ID of the other repeater cli to be the same with ApCli */
		AsicTxBfApClientCluster(pAd, pEntry->wcid, pAd->ApCli_CmmWlanId);
	}

#endif /* APCLI_SUPPORT */
}


/* MlmeTxBfAllowed - returns true if ETxBF or ITxBF is supported and pTxRate is a valid BF mode */
BOOLEAN MlmeTxBfAllowed(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB * pTxRate)
{
	/* ETxBF */
	if ((pEntry->eTxBfEnCond > 0) &&
		(pTxRate->Mode == MODE_HTMIX || pTxRate->Mode == MODE_HTGREENFIELD)
	   )
		return TRUE;

	/* ITxBF */
	if (pEntry->iTxBfEn && pTxRate->CurrMCS < 16 && pTxRate->Mode != MODE_CCK)
		return TRUE;

	return FALSE;
}





#if defined(MT_MAC) && (!defined(MT7636))
VOID TxBfProfileTag_PfmuIdx(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucProfileID)
{
	prPfmuTag1->rField.ucProfileID = ucProfileID;
}

VOID TxBfProfileTag_TxBfType(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucTxBf)
{
	prPfmuTag1->rField.ucTxBf = ucTxBf;
}

VOID TxBfProfileTag_DBW(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucBw)
{
	prPfmuTag1->rField.ucDBW = ucBw;
}

VOID TxBfProfileTag_SuMu(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucSuMu)
{
	prPfmuTag1->rField.ucSU_MU = ucSuMu;
}

VOID TxBfProfileTag_InValid(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR InvalidFlg)
{
	prPfmuTag1->rField.ucInvalidProf = InvalidFlg;
}

VOID TxBfProfileTag_Mem(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN PUCHAR pMemAddrColIdx,
	IN PUCHAR pMemAddrRowIdx)
{
	prPfmuTag1->rField.ucMemAddr1ColIdx = pMemAddrColIdx[0];
	prPfmuTag1->rField.ucMemAddr1RowIdx = pMemAddrRowIdx[0];
	prPfmuTag1->rField.ucMemAddr2ColIdx = pMemAddrColIdx[1];
	prPfmuTag1->rField.ucMemAddr2RowIdx = pMemAddrRowIdx[1] & 0x1F;
	prPfmuTag1->rField.ucMemAddr2RowIdxMsb = pMemAddrRowIdx[1] >> 5;
	prPfmuTag1->rField.ucMemAddr3ColIdx = pMemAddrColIdx[2];
	prPfmuTag1->rField.ucMemAddr3RowIdx = pMemAddrRowIdx[2];
	prPfmuTag1->rField.ucMemAddr4ColIdx = pMemAddrColIdx[3];
	prPfmuTag1->rField.ucMemAddr4RowIdx = pMemAddrRowIdx[3];
}


VOID TxBfProfileTag_Matrix(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucNrow,
	IN UCHAR ucNcol,
	IN UCHAR ucNgroup,
	IN UCHAR ucLM,
	IN UCHAR ucCodeBook,
	IN UCHAR ucHtcExist)
{
	prPfmuTag1->rField.ucNrow    = ucNrow;
	prPfmuTag1->rField.ucNcol    = ucNcol;
	prPfmuTag1->rField.ucNgroup  = ucNgroup;
	prPfmuTag1->rField.ucLM      = ucLM;
	prPfmuTag1->rField.ucCodeBook = ucCodeBook;
	prPfmuTag1->rField.ucHtcExist = ucHtcExist;
}


VOID TxBfProfileTag_SNR(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucSNR_STS0,
	IN UCHAR ucSNR_STS1,
	IN UCHAR ucSNR_STS2,
	IN UCHAR ucSNR_STS3)
{
	prPfmuTag1->rField.ucSNR_STS0 = ucSNR_STS0;
	prPfmuTag1->rField.ucSNR_STS1 = ucSNR_STS1;
	prPfmuTag1->rField.ucSNR_STS2 = ucSNR_STS2;
	prPfmuTag1->rField.ucSNR_STS3 = ucSNR_STS3;
}


VOID TxBfProfileTag_SmtAnt(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN USHORT u2SmartAnt)
{
	prPfmuTag2->rField.u2SmartAnt = u2SmartAnt;
}



VOID TxBfProfileTag_SeIdx(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR ucSEIdx)
{
	prPfmuTag2->rField.ucSEIdx = ucSEIdx;
}


VOID TxBfProfileTag_RmsdThd(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR ucRMSDThd)
{
	prPfmuTag2->rField.ucRMSDThd = ucRMSDThd;
}


VOID TxBfProfileTag_McsThd(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN PUCHAR pMCSThLSS,
	IN PUCHAR pMCSThSSS)
{
	prPfmuTag2->rField.ucMCSThL1SS = pMCSThLSS[0];
	prPfmuTag2->rField.ucMCSThS1SS = pMCSThSSS[0];
	prPfmuTag2->rField.ucMCSThL2SS = pMCSThLSS[1];
	prPfmuTag2->rField.ucMCSThS2SS = pMCSThSSS[1];
	prPfmuTag2->rField.ucMCSThL3SS = pMCSThLSS[2];
	prPfmuTag2->rField.ucMCSThS3SS = pMCSThSSS[2];
}


VOID TxBfProfileTag_TimeOut(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfTimeOut)
{
	prPfmuTag2->rField.uciBfTimeOut = uciBfTimeOut;
}


VOID TxBfProfileTag_DesiredBW(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfDBW)
{
	prPfmuTag2->rField.uciBfDBW = uciBfDBW;
}


VOID TxBfProfileTag_DesiredNc(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfNcol)
{
	prPfmuTag2->rField.uciBfNcol = uciBfNcol;
}


VOID TxBfProfileTag_DesiredNr(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfNrow)
{
	prPfmuTag2->rField.uciBfNrow = uciBfNrow;
}

INT TxBfProfileTagRead(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR		PfmuIdx,
	IN BOOLEAN              fgBFer)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileTagRead(pAd, PfmuIdx, fgBFer) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfileTagWrite(
	IN PRTMP_ADAPTER	pAd,
	IN P_PFMU_PROFILE_TAG1  prPfmuTag1,
	IN P_PFMU_PROFILE_TAG2  prPfmuTag2,
	IN UCHAR		profileIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileTagWrite(pAd,
									(PUCHAR) (prPfmuTag1),
									(PUCHAR) (prPfmuTag2),
									profileIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				 "============================= TxBf profile Tage1 Info ========================================\n"
				 "Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x, Row data3 = 0x%x\n\n"
				 "PFMU ID = %d        Invalid status = %d\n"
				 "iBf/eBf = %d\n\n"
				 "DBW   = %d\n"
				 "SU/MU = %d\n"
				 "RMSD  = %d\n"
				 "nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d, HtcExist=%d\n\n"
				 "Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
				 "Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n\n"
				 "STS0_SNR =0x%x, STS1_SNR=0x%x, STS2_SNR=0x%x, STS3_SNR=0x%x\n\n"
				 "iBf LNA Idx=0x%x\n"
				 "==============================================================================================\n",
				 prPfmuTag1->au4RawData[0], prPfmuTag1->au4RawData[1], prPfmuTag1->au4RawData[2], prPfmuTag1->au4RawData[3],
				 prPfmuTag1->rField.ucProfileID,      prPfmuTag1->rField.ucInvalidProf,
				 prPfmuTag1->rField.ucTxBf,
				 prPfmuTag1->rField.ucDBW,
				 prPfmuTag1->rField.ucSU_MU,
				 prPfmuTag1->rField.ucRMSD,
				 prPfmuTag1->rField.ucNrow,           prPfmuTag1->rField.ucNcol,          prPfmuTag1->rField.ucNgroup, prPfmuTag1->rField.ucLM,
				 prPfmuTag1->rField.ucCodeBook,       prPfmuTag1->rField.ucHtcExist,
				 prPfmuTag1->rField.ucMemAddr1ColIdx, prPfmuTag1->rField.ucMemAddr1RowIdx,
				 prPfmuTag1->rField.ucMemAddr2ColIdx, (prPfmuTag1->rField.ucMemAddr2RowIdx | (prPfmuTag1->rField.ucMemAddr2RowIdxMsb << 5)),
				 prPfmuTag1->rField.ucMemAddr3ColIdx, prPfmuTag1->rField.ucMemAddr3RowIdx,
				 prPfmuTag1->rField.ucMemAddr4ColIdx, prPfmuTag1->rField.ucMemAddr4RowIdx,
				 prPfmuTag1->rField.ucSNR_STS0,       prPfmuTag1->rField.ucSNR_STS1,
				 prPfmuTag1->rField.ucSNR_STS2,       prPfmuTag1->rField.ucSNR_STS3,
				 prPfmuTag1->rField.ucIBfLnaIdx));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				 "============================= TxBf profile Tage2 Info ========================================\n"
				 "Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x\n\n"
				 "Smart antenna ID = 0x%x,  SE index = %d\n"
				 "RMSD threshold = %d\n"
				 "MCS L1SS thd = %d, S1SS thd = %d, L2SS thd = %d, S2SS thd = %d, L3SS thd = %d, S3SS thd = %d\n"
				 "Time out = 0x%x\n"
				 "Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
				 "==============================================================================================\n",
				 prPfmuTag2->au4RawData[0], prPfmuTag2->au4RawData[1], prPfmuTag2->au4RawData[2],
				 prPfmuTag2->rField.u2SmartAnt,   prPfmuTag2->rField.ucSEIdx,
				 prPfmuTag2->rField.ucRMSDThd,
				 prPfmuTag2->rField.ucMCSThL1SS,  prPfmuTag2->rField.ucMCSThS1SS, prPfmuTag2->rField.ucMCSThL2SS, prPfmuTag2->rField.ucMCSThS2SS,
				 prPfmuTag2->rField.ucMCSThL3SS,  prPfmuTag2->rField.ucMCSThS3SS,
				 prPfmuTag2->rField.uciBfTimeOut, prPfmuTag2->rField.uciBfDBW, prPfmuTag2->rField.uciBfNcol, prPfmuTag2->rField.uciBfNrow));
	return fgStatus;
}


VOID TxBfProfileTagPrint(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN       fgBFer,
	IN PUCHAR        pBuf)
{
	P_PFMU_PROFILE_TAG1 prPfmuTag1;
	P_PFMU_PROFILE_TAG2 prPfmuTag2;

	prPfmuTag1 = (P_PFMU_PROFILE_TAG1) pBuf;
	prPfmuTag2 = (P_PFMU_PROFILE_TAG2) (pBuf + sizeof(PFMU_PROFILE_TAG1));
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((char *)prPfmuTag1, sizeof(PFMU_PROFILE_TAG1));
	RTMPEndianChange((char *)prPfmuTag2, sizeof(PFMU_PROFILE_TAG2));
#endif
	NdisCopyMemory(&pAd->rPfmuTag1, prPfmuTag1, sizeof(PFMU_PROFILE_TAG1));
	NdisCopyMemory(&pAd->rPfmuTag2, prPfmuTag2, sizeof(PFMU_PROFILE_TAG2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				 "============================= TxBf profile Tage1 Info ========================================\n"
				 "Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x, Row data3 = 0x%x\n\n"
				 "PFMU ID = %d        Invalid status = %d\n"
				 "iBf/eBf = %d\n\n"
				 "DBW   = %d\n"
				 "SU/MU = %d\n"
				 "RMSD  = %d\n"
				 "nrow=%d, ncol=%d, ng=%d, LM=%d, CodeBook=%d, HtcExist=%d\n\n"
				 "Mem Col1 = %d, Mem Row1 = %d, Mem Col2 = %d, Mem Row2 = %d\n"
				 "Mem Col3 = %d, Mem Row3 = %d, Mem Col4 = %d, Mem Row4 = %d\n\n"
				 "STS0_SNR =0x%x, STS1_SNR=0x%x, STS2_SNR=0x%x, STS3_SNR=0x%x\n\n"
				 "iBf LNA Idx=0x%x\n"
				 "==============================================================================================\n",
				 prPfmuTag1->au4RawData[0], prPfmuTag1->au4RawData[1], prPfmuTag1->au4RawData[2], prPfmuTag1->au4RawData[3],
				 prPfmuTag1->rField.ucProfileID,      prPfmuTag1->rField.ucInvalidProf,
				 prPfmuTag1->rField.ucTxBf,
				 prPfmuTag1->rField.ucDBW,
				 prPfmuTag1->rField.ucSU_MU,
				 prPfmuTag1->rField.ucRMSD,
				 prPfmuTag1->rField.ucNrow,           prPfmuTag1->rField.ucNcol,          prPfmuTag1->rField.ucNgroup, prPfmuTag1->rField.ucLM,
				 prPfmuTag1->rField.ucCodeBook,       prPfmuTag1->rField.ucHtcExist,
				 prPfmuTag1->rField.ucMemAddr1ColIdx, prPfmuTag1->rField.ucMemAddr1RowIdx,
				 prPfmuTag1->rField.ucMemAddr2ColIdx, (prPfmuTag1->rField.ucMemAddr2RowIdx | (prPfmuTag1->rField.ucMemAddr2RowIdxMsb << 5)),
				 prPfmuTag1->rField.ucMemAddr3ColIdx, prPfmuTag1->rField.ucMemAddr3RowIdx,
				 prPfmuTag1->rField.ucMemAddr4ColIdx, prPfmuTag1->rField.ucMemAddr4RowIdx,
				 prPfmuTag1->rField.ucSNR_STS0,       prPfmuTag1->rField.ucSNR_STS1,
				 prPfmuTag1->rField.ucSNR_STS2,       prPfmuTag1->rField.ucSNR_STS3,
				 prPfmuTag1->rField.ucIBfLnaIdx));

	if (fgBFer == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "============================= TxBf profile Tage2 Info ========================================\n"
					 "Row data0 = 0x%x, Row data1 = 0x%x, Row data2 = 0x%x\n\n"
					 "Smart antenna ID = 0x%x,  SE index = %d\n"
					 "RMSD threshold = %d\n"
					 "MCS L1SS thd = %d, S1SS thd = %d, L2SS thd = %d, S2SS thd = %d, L3SS thd = %d, S3SS thd = %d\n"
					 "Time out = 0x%x\n"
					 "Desired BW = %d, Desired Ncol = %d, Desired Nrow = %d\n"
					 "==============================================================================================\n",
					 prPfmuTag2->au4RawData[0], prPfmuTag2->au4RawData[1], prPfmuTag2->au4RawData[2],
					 prPfmuTag2->rField.u2SmartAnt,   prPfmuTag2->rField.ucSEIdx,
					 prPfmuTag2->rField.ucRMSDThd,
					 prPfmuTag2->rField.ucMCSThL1SS,  prPfmuTag2->rField.ucMCSThS1SS, prPfmuTag2->rField.ucMCSThL2SS, prPfmuTag2->rField.ucMCSThS2SS,
					 prPfmuTag2->rField.ucMCSThL3SS,  prPfmuTag2->rField.ucMCSThS3SS,
					 prPfmuTag2->rField.uciBfTimeOut, prPfmuTag2->rField.uciBfDBW, prPfmuTag2->rField.uciBfNcol, prPfmuTag2->rField.uciBfNrow));
	}
}


INT TxBfProfilePnRead(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfilePnRead(pAd, profileIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfilePnWrite(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         PfmuIdx,
	IN UCHAR         ucBw,
	IN PUCHAR        pProfileData)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfilePnWrite(pAd, PfmuIdx, ucBw, pProfileData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


VOID TxBfProfilePnPrint(
	IN UCHAR  ucBw,
	IN PUCHAR pBuf)
{
	P_PFMU_PN_DBW20M    prPfmuPn20M;
	P_PFMU_PN_DBW40M    prPfmuPn40M;
	P_PFMU_PN_DBW80M    prPfmuPn80M;
	P_PFMU_PN_DBW80_80M prPfmuPn160M;

	switch (ucBw) {
	case P_DBW20M:
		prPfmuPn20M = (P_PFMU_PN_DBW20M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn20M, sizeof(PFMU_PN_DBW20M));
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "============================= TxBf profile PN Info 20M ========================================\n"
					 "1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					 "2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					 "3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					 "===============================================================================================\n",
					 prPfmuPn20M->rField.u2DBW20_1STS_Tx0, prPfmuPn20M->rField.u2DBW20_1STS_Tx1,
					 prPfmuPn20M->rField.u2DBW20_1STS_Tx2 | (prPfmuPn20M->rField.u2DBW20_1STS_Tx2Msb << 11),
					 prPfmuPn20M->rField.u2DBW20_1STS_Tx3,
					 prPfmuPn20M->rField.u2DBW20_2STS_Tx0,
					 prPfmuPn20M->rField.u2DBW20_2STS_Tx1 | (prPfmuPn20M->rField.u2DBW20_2STS_Tx1Msb << 11),
					 prPfmuPn20M->rField.u2DBW20_2STS_Tx2, prPfmuPn20M->rField.u2DBW20_2STS_Tx3,
					 prPfmuPn20M->rField.u2DBW20_3STS_Tx0, prPfmuPn20M->rField.u2DBW20_3STS_Tx1,
					 prPfmuPn20M->rField.u2DBW20_3STS_Tx2,
					 prPfmuPn20M->rField.u2DBW20_3STS_Tx3 | (prPfmuPn20M->rField.u2DBW20_3STS_Tx3Msb << 11)));
		break;

	case P_DBW40M:
		prPfmuPn40M = (P_PFMU_PN_DBW40M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn40M, sizeof(PFMU_PN_DBW40M));
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "============================= TxBf profile PN Info 40M ========================================\n"
					 "1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					 "2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					 "3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					 "===============================================================================================\n",
					 prPfmuPn40M->rField.u2DBW40_1STS_Tx0, prPfmuPn40M->rField.u2DBW40_1STS_Tx1,
					 prPfmuPn40M->rField.u2DBW40_1STS_Tx2 | (prPfmuPn40M->rField.u2DBW40_1STS_Tx2Msb << 11),
					 prPfmuPn40M->rField.u2DBW40_1STS_Tx3,
					 prPfmuPn40M->rField.u2DBW40_2STS_Tx0,
					 prPfmuPn40M->rField.u2DBW40_2STS_Tx1 | (prPfmuPn40M->rField.u2DBW40_2STS_Tx1Msb << 11),
					 prPfmuPn40M->rField.u2DBW40_2STS_Tx2, prPfmuPn40M->rField.u2DBW40_2STS_Tx3,
					 prPfmuPn40M->rField.u2DBW40_3STS_Tx0, prPfmuPn40M->rField.u2DBW40_3STS_Tx1,
					 prPfmuPn40M->rField.u2DBW40_3STS_Tx2,
					 prPfmuPn40M->rField.u2DBW40_3STS_Tx3 | (prPfmuPn40M->rField.u2DBW40_3STS_Tx3Msb << 11)));
		break;

	case P_DBW80M:
		prPfmuPn80M = (P_PFMU_PN_DBW80M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn80M, sizeof(PFMU_PN_DBW80M));
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "============================= TxBf profile PN Info 80M ========================================\n"
					 "1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x, 1STS_TX2 = 0x%x, 1STS_TX3 = 0x%x\n"
					 "2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x, 2STS_TX2 = 0x%x, 2STS_TX3 = 0x%x\n"
					 "3STS_TX0 = 0x%x, 3STS_TX1 = 0x%x, 3STS_TX2 = 0x%x, 3STS_TX3 = 0x%x\n"
					 "===============================================================================================\n",
					 prPfmuPn80M->rField.u2DBW80_1STS_Tx0, prPfmuPn80M->rField.u2DBW80_1STS_Tx1,
					 prPfmuPn80M->rField.u2DBW80_1STS_Tx2 | (prPfmuPn80M->rField.u2DBW80_1STS_Tx2Msb << 11),
					 prPfmuPn80M->rField.u2DBW80_1STS_Tx3,
					 prPfmuPn80M->rField.u2DBW80_2STS_Tx0,
					 prPfmuPn80M->rField.u2DBW80_2STS_Tx1 | (prPfmuPn80M->rField.u2DBW80_2STS_Tx1Msb << 11),
					 prPfmuPn80M->rField.u2DBW80_2STS_Tx2, prPfmuPn80M->rField.u2DBW80_2STS_Tx3,
					 prPfmuPn80M->rField.u2DBW80_3STS_Tx0, prPfmuPn80M->rField.u2DBW80_3STS_Tx1,
					 prPfmuPn80M->rField.u2DBW80_3STS_Tx2,
					 prPfmuPn80M->rField.u2DBW80_3STS_Tx3 | (prPfmuPn80M->rField.u2DBW80_3STS_Tx3Msb << 11)));
		break;

	case P_DBW160M:
		prPfmuPn160M = (P_PFMU_PN_DBW80_80M) pBuf;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)prPfmuPn160M, sizeof(PFMU_PN_DBW80_80M));
#endif
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "============================= TxBf profile PN Info 80M ========================================\n"
					 "1STS_TX0 = 0x%x, 1STS_TX1 = 0x%x\n"
					 "2STS_TX0 = 0x%x, 2STS_TX1 = 0x%x\n"
					 "===============================================================================================\n",
					 prPfmuPn160M->rField.u2DBW160_1STS_Tx0, prPfmuPn160M->rField.u2DBW160_1STS_Tx1,
					 prPfmuPn160M->rField.u2DBW160_2STS_Tx0 | (prPfmuPn160M->rField.u2DBW160_2STS_Tx0Msb << 11),
					 prPfmuPn160M->rField.u2DBW160_2STS_Tx1));
		break;

	default:
		break;
	}
}

/* SC_TABLE_ENTRY g_arSubCarIdxBwTbl[4]={{228, 255, 1, 28}, {198, 254, 2, 58}, {134, 254, 2, 122}, {134, 254, 2, 122}}; */

INT TxBfProfileDataRead(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx,
	IN BOOLEAN       fgBFer,
	IN USHORT        subCarrIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileDataRead(pAd, profileIdx, fgBFer, subCarrIdx) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


INT TxBfProfileDataWrite(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx,
	IN USHORT        subCarrIdx,
	IN PUCHAR        pProfileData
)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfPfmuProfileDataWrite(pAd,
									 profileIdx,
									 subCarrIdx,
									 pProfileData) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}

INT TxBfQdRead(
	IN PRTMP_ADAPTER pAd,
	IN INT8          subCarrIdx)
{
	BOOLEAN  fgStatus = FALSE;

	if (CmdETxBfQdRead(pAd, subCarrIdx) == STATUS_TRUE) {
		fgStatus = TRUE;
	}

	return fgStatus;
}

#ifdef CONFIG_ATE
BOOLEAN TxBfProfileDataFormatTranslate(
	PRTMP_ADAPTER pAd,
	PUCHAR pucDataIn,
	P_PFMU_HALF_DATA pPfmuHalfData)
{
	struct _ATE_CTRL      *ATECtrl = &pAd->ATECtrl;
	UCHAR   control_band_idx = ATECtrl->control_band_idx;
	UINT32  u4SubCarrId, u4AnglePh11, u4AnglePh21, u4AnglePh31, u4AnglePh41;
	INT16   i2Phi11,     i2Phi21,     i2Phi31;
	UINT16  u2InIdx;
	UCHAR	ucLoop;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s :: Band index = %d\n", __func__, control_band_idx));

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		if (control_band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	for (ucLoop = 0; ucLoop < 64; ucLoop++) {
		u2InIdx  = ucLoop * 20;
		/* Subcarrier */
		NdisMoveMemory(&u4SubCarrId, &pucDataIn[u2InIdx], 4);
		u4SubCarrId = PKTL_TRAN_TO_HOST(u4SubCarrId);

		if (u4SubCarrId < 32)
			u4SubCarrId += 224;
		else
			u4SubCarrId -= 32;

		/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
		/* ("%s:: val: %u\n",__FUNCTION__, u4SubCarrId)); */
		/* Angle Ph11 */
		NdisMoveMemory(&u4AnglePh11, &pucDataIn[u2InIdx + 4], 4);
		u4AnglePh11 = PKTL_TRAN_TO_HOST(u4AnglePh11);
		/* Angle Ph21 */
		NdisMoveMemory(&u4AnglePh21, &pucDataIn[u2InIdx + 8], 4);
		u4AnglePh21 = PKTL_TRAN_TO_HOST(u4AnglePh21);
		/* Angle Ph31 */
		NdisMoveMemory(&u4AnglePh31, &pucDataIn[u2InIdx + 12], 4);
		u4AnglePh31 = PKTL_TRAN_TO_HOST(u4AnglePh31);
		/* Angle Ph41 */
		NdisMoveMemory(&u4AnglePh41, &pucDataIn[u2InIdx + 16], 4);
		u4AnglePh41 = PKTL_TRAN_TO_HOST(u4AnglePh41);

		/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, */
		/* ("u4AnglePh11 : 0x%x, u4AnglePh21 : 0x%x, u4AnglePh31 : 0x%x, u4AnglePh41 : 0x%x\n", */
		/* u4AnglePh11, u4AnglePh21, u4AnglePh31, u4AnglePh41)); */

		switch (ucTxPath) {
		case 2:
			i2Phi11    = (INT16)(u4AnglePh21 - u4AnglePh11);
			i2Phi21    = 0;
			i2Phi31    = 0;
			break;
		case 3:
			i2Phi11    = (INT16)(u4AnglePh31 - u4AnglePh11);
			i2Phi21    = (INT16)(u4AnglePh31 - u4AnglePh21);
			i2Phi31    = 0;
			break;

		case 4:
		default:
#ifdef DBDC_MODE
			if (pAd->CommonCfg.dbdc_mode) {

				i2Phi11    = (INT16)(u4AnglePh21 - u4AnglePh11);
				i2Phi21    = 0;
				i2Phi31    = 0;
			} else
#endif
			{
				i2Phi11    = (INT16)(u4AnglePh41 - u4AnglePh11);
				i2Phi21    = (INT16)(u4AnglePh41 - u4AnglePh21);
				i2Phi31    = (INT16)(u4AnglePh41 - u4AnglePh31);
			}

			break;
		}

		pPfmuHalfData[ucLoop].u2SubCarrIdx = (UINT16)u4SubCarrId;
#ifdef RT_BIG_ENDIAN
		pPfmuHalfData[ucLoop].u2SubCarrIdx = cpu2le16(pPfmuHalfData[ucLoop].u2SubCarrIdx);
#endif
		pPfmuHalfData[ucLoop].i2Phi11      = cpu2le16(i2Phi11);
		pPfmuHalfData[ucLoop].i2Phi21      = cpu2le16(i2Phi21);
		pPfmuHalfData[ucLoop].i2Phi31      = cpu2le16(i2Phi31);
	}

	return TRUE;
}
#endif /* CONFIG_ATE */


INT TxBfProfileDataWrite20MAll(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR         profileIdx,
	IN PUCHAR        pData
)
{
	PFMU_HALF_DATA arProfileData[64];
	BOOLEAN  fgStatus = FALSE;
#ifdef CONFIG_ATE

	if (TxBfProfileDataFormatTranslate(pAd, pData, arProfileData) == FALSE)
		return fgStatus;

#endif /* CONFIG_ATE */

	if (CmdETxBfPfmuProfileDataWrite20MAll(pAd,
										   profileIdx,
										   (PUCHAR)&arProfileData[0]) == STATUS_TRUE)
		fgStatus = TRUE;

	return fgStatus;
}


VOID TxBfProfileDataPrint(
	IN PRTMP_ADAPTER pAd,
	IN USHORT        subCarrIdx,
	IN PUCHAR        pBuf)
{
	P_PFMU_DATA prPfmuDataStart;
	PFMU_DATA   rPfmuData;

	prPfmuDataStart = (P_PFMU_DATA) pBuf;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange((UCHAR *)prPfmuDataStart, sizeof(PFMU_DATA));
#endif
	NdisCopyMemory(&rPfmuData, prPfmuDataStart, sizeof(PFMU_DATA));
	NdisCopyMemory(&pAd->prof, prPfmuDataStart, sizeof(PFMU_DATA));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				 "============================= TxBf profile Data - Subcarrier Idx = %d ========================================\n"
				 "Psi41 = 0x%x, Phi31 = 0x%x, Psi31 = 0x%x, Phi21 = 0x%x, Psi21 = 0x%x, Phi11 = 0x%x\n"
				 "Psi43 = 0x%x, Phi33 = 0x%x, Psi42 = 0x%x, Phi32 = 0x%x, Psi32 = 0x%x, Phi22 = 0x%x\n"
				 "SNR00 = 0x%x, SNR01 = 0x%x, SNR02 = 0x%x, SNR03 = 0x%x\n"
				 "==============================================================================================================\n",
				 subCarrIdx,
				 rPfmuData.rField.ucPsi41, rPfmuData.rField.u2Phi31, rPfmuData.rField.ucPsi31, rPfmuData.rField.u2Phi21,
				 rPfmuData.rField.ucPsi21, rPfmuData.rField.u2Phi11,
				 rPfmuData.rField.ucPsi43, rPfmuData.rField.u2Phi33, rPfmuData.rField.ucPsi42, rPfmuData.rField.u2Phi32,
				 rPfmuData.rField.ucPsi32, rPfmuData.rField.u2Phi22,
				 rPfmuData.rField.u2dSNR00, rPfmuData.rField.u2dSNR01, rPfmuData.rField.u2dSNR02, rPfmuData.rField.u2dSNR03));
}


VOID StaRecBfUpdate(
	IN MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BF pCmdStaRecBf)
{
	pCmdStaRecBf->u2Tag      = STA_REC_BF;
	pCmdStaRecBf->u2Length   = sizeof(CMD_STAREC_BF);
#ifdef RT_BIG_ENDIAN
	pCmdStaRecBf->u2Tag      = cpu2le16(pCmdStaRecBf->u2Tag);
	pCmdStaRecBf->u2Length   = cpu2le16(pCmdStaRecBf->u2Length);
#endif
	pCmdStaRecBf->rTxBfPfmuInfo.u2PfmuId      = cpu2le16(pEntry->rStaRecBf.u2PfmuId);
	pCmdStaRecBf->rTxBfPfmuInfo.fgSU_MU       = pEntry->rStaRecBf.fgSU_MU;
	pCmdStaRecBf->rTxBfPfmuInfo.fgETxBfCap    = pEntry->rStaRecBf.fgETxBfCap;
	pCmdStaRecBf->rTxBfPfmuInfo.ucSoundingPhy = pEntry->rStaRecBf.ucSoundingPhy;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpaRate    = pEntry->rStaRecBf.ucNdpaRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNdpRate     = pEntry->rStaRecBf.ucNdpRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucReptPollRate  = pEntry->rStaRecBf.ucReptPollRate;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTxMode      = pEntry->rStaRecBf.ucTxMode;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNc          = pEntry->rStaRecBf.ucNc;
	pCmdStaRecBf->rTxBfPfmuInfo.ucNr          = pEntry->rStaRecBf.ucNr;
	pCmdStaRecBf->rTxBfPfmuInfo.ucCBW         = pEntry->rStaRecBf.ucCBW;
	pCmdStaRecBf->rTxBfPfmuInfo.ucTotMemRequire = pEntry->rStaRecBf.ucTotMemRequire;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRequire20M = pEntry->rStaRecBf.ucMemRequire20M;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow0     = pEntry->rStaRecBf.ucMemRow0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol0     = pEntry->rStaRecBf.ucMemCol0;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow1     = pEntry->rStaRecBf.ucMemRow1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol1     = pEntry->rStaRecBf.ucMemCol1;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow2     = pEntry->rStaRecBf.ucMemRow2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol2     = pEntry->rStaRecBf.ucMemCol2;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemRow3     = pEntry->rStaRecBf.ucMemRow3;
	pCmdStaRecBf->rTxBfPfmuInfo.ucMemCol3     = pEntry->rStaRecBf.ucMemCol3;
	pCmdStaRecBf->rTxBfPfmuInfo.u2SmartAnt    = cpu2le16(pEntry->rStaRecBf.u2SmartAnt);
	pCmdStaRecBf->rTxBfPfmuInfo.ucSEIdx       = pEntry->rStaRecBf.ucSEIdx;
#ifdef TXBF_DYNAMIC_DISABLE
	pCmdStaRecBf->rTxBfPfmuInfo.ucAutoSoundingCtrl = pEntry->rStaRecBf.ucAutoSoundingCtrl;
#endif /* TXBF_DYNAMIC_DISABLE */
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfTimeOut  = pEntry->rStaRecBf.uciBfTimeOut;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfDBW      = pEntry->rStaRecBf.uciBfDBW;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNcol     = pEntry->rStaRecBf.uciBfNcol;
	pCmdStaRecBf->rTxBfPfmuInfo.uciBfNrow     = pEntry->rStaRecBf.uciBfNrow;
}


VOID StaRecBfRead(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBuf)
{
	NdisCopyMemory(&pAd->rStaRecBf, pBuf, sizeof(TXBF_PFMU_STA_INFO));
#ifdef RT_BIG_ENDIAN
	pAd->rStaRecBf.u2PfmuId = le2cpu16(pAd->rStaRecBf.u2PfmuId);
	pAd->rStaRecBf.u2SmartAnt = le2cpu16(pAd->rStaRecBf.u2SmartAnt);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
				 "====================================== BF StaRec ========================================\n"
				 "rStaRecBf.u2PfmuId      = %d\n"
				 "rStaRecBf.fgSU_MU       = %d\n"
				 "rStaRecBf.fgETxBfCap    = %d\n"
				 "rStaRecBf.ucSoundingPhy = %d\n"
				 "rStaRecBf.ucNdpaRate    = %d\n"
				 "rStaRecBf.ucNdpRate     = %d\n"
				 "rStaRecBf.ucReptPollRate= %d\n"
				 "rStaRecBf.ucTxMode      = %d\n"
				 "rStaRecBf.ucNc          = %d\n"
				 "rStaRecBf.ucNr          = %d\n"
				 "rStaRecBf.ucCBW         = %d\n"
				 "rStaRecBf.ucTotMemRequire = %d\n"
				 "rStaRecBf.ucMemRequire20M = %d\n"
				 "rStaRecBf.ucMemRow0     = %d\n"
				 "rStaRecBf.ucMemCol0     = %d\n"
				 "rStaRecBf.ucMemRow1     = %d\n"
				 "rStaRecBf.ucMemCol1     = %d\n"
				 "rStaRecBf.ucMemRow2     = %d\n"
				 "rStaRecBf.ucMemCol2     = %d\n"
				 "rStaRecBf.ucMemRow3     = %d\n"
				 "rStaRecBf.ucMemCol3     = %d\n"
				 "rStaRecBf.u2SmartAnt    = 0x%x\n"
				 "rStaRecBf.ucSEIdx       = %d\n"
#ifdef TXBF_DYNAMIC_DISABLE
				 "rStaRecBf.ucAutoSoundingCtrl = %d\n"
#endif /* TXBF_DYNAMIC_DISABLE */
				 "rStaRecBf.uciBfTimeOut  = 0x%x\n"
				 "rStaRecBf.uciBfDBW      = %d\n"
				 "rStaRecBf.uciBfNcol     = %d\n"
				 "rStaRecBf.uciBfNrow     = %d\n"
				 "=======================================================================================\n",
				 pAd->rStaRecBf.u2PfmuId,
				 pAd->rStaRecBf.fgSU_MU,
				 pAd->rStaRecBf.fgETxBfCap,
				 pAd->rStaRecBf.ucSoundingPhy,
				 pAd->rStaRecBf.ucNdpaRate,
				 pAd->rStaRecBf.ucNdpRate,
				 pAd->rStaRecBf.ucReptPollRate,
				 pAd->rStaRecBf.ucTxMode,
				 pAd->rStaRecBf.ucNc,
				 pAd->rStaRecBf.ucNr,
				 pAd->rStaRecBf.ucCBW,
				 pAd->rStaRecBf.ucTotMemRequire,
				 pAd->rStaRecBf.ucMemRequire20M,
				 pAd->rStaRecBf.ucMemRow0,
				 pAd->rStaRecBf.ucMemCol0,
				 pAd->rStaRecBf.ucMemRow1,
				 pAd->rStaRecBf.ucMemCol1,
				 pAd->rStaRecBf.ucMemRow2,
				 pAd->rStaRecBf.ucMemCol2,
				 pAd->rStaRecBf.ucMemRow3,
				 pAd->rStaRecBf.ucMemCol3,
				 pAd->rStaRecBf.u2SmartAnt,
				 pAd->rStaRecBf.ucSEIdx,
#ifdef TXBF_DYNAMIC_DISABLE
				 pAd->rStaRecBf.ucAutoSoundingCtrl,
#endif /* TXBF_DYNAMIC_DISABLE */
				 pAd->rStaRecBf.uciBfTimeOut,
				 pAd->rStaRecBf.uciBfDBW,
				 pAd->rStaRecBf.uciBfNcol,
				 pAd->rStaRecBf.uciBfNrow));
}


VOID TxBfProfileMemAllocMap(
	IN PUCHAR  pBuf)
{
	UINT_16 au2PfmuMemAllocMap[TXBF_PFMU_ARRAY_SIZE][MAX_PFMU_MEM_LEN_PER_ROW];
	UINT_8  ucLoop, ucBit;

	NdisCopyMemory(au2PfmuMemAllocMap, pBuf, sizeof(au2PfmuMemAllocMap));

	for (ucLoop = 0; ucLoop < TXBF_PFMU_ARRAY_SIZE; ucLoop++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "========= PFMU memory allocation map =========\n"));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "%3d :", ucLoop));

		for (ucBit = 0; ucBit < 6; ucBit++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
						 "%4d |",
						 le2cpu16(au2PfmuMemAllocMap[ucLoop][ucBit])));

			if (ucBit == 5) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
							 "\n"));
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (
					 "==============================================\n"));
	}
}

BOOLEAN TxBfModuleEnCtrl(
	IN PRTMP_ADAPTER pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct _RTMP_CHIP_CAP *cap;
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UINT8 i;
	UINT8 u1BfNum = 0;
	UINT8 u1isBfBfBandNum = 0;
	UINT8 u1BfBitmap = 0;
	UINT8 u1BfSelBand[8];

	if (pHwResource->concurrent_bands < 2) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:It's not DBDC mode\n", __func__));

		return FALSE;
	}
	os_zero_mem(&u1BfSelBand[0], sizeof(u1BfSelBand));
	cap = hc_get_chip_cap(ctrl);

	u1BfNum = (cap->FlgHwTxBfCap & TXBF_HW_2BF) ? 2 : 1;
	if (u1BfNum > 1) {
		/* For 2 BF DBDC mode */
		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			pPhyCtrl =  &pHwResource->PhyCtrl[i];
			pRadioCtrl =  &pPhyCtrl->RadioCtrl;

			if (pRadioCtrl->IsBfBand) {
				u1isBfBfBandNum++;
				u1BfBitmap |= 1 << i;
				u1BfSelBand[i] = 0;
			}
		}
	} else {
		/* For 1 BF DBDC mode */
		for (i = 0; i < pHwResource->concurrent_bands; i++) {
			pPhyCtrl =  &pHwResource->PhyCtrl[i];
			pRadioCtrl =  &pPhyCtrl->RadioCtrl;

			if (pRadioCtrl->IsBfBand) {
				u1isBfBfBandNum++;
				u1BfBitmap |= 1;
				u1BfSelBand[0] = i;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:i = %d, pRadioCtrl->IsBfBand = %d\n", __func__,
				i, pRadioCtrl->IsBfBand));
		}
	}

	if (u1isBfBfBandNum > u1BfNum) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s:Error - isBfBfBandNum > hardware capability\n", __func__));

		return FALSE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s:u1BfNum = %d, u1BfBitmap = %d, u1BfSelBand[0] = %d\n", __func__,
					u1BfNum, u1BfBitmap, u1BfSelBand[0]));

	AsicTxBfModuleEnCtrl(pAd, u1BfNum, u1BfBitmap, &u1BfSelBand[0]);

	return TRUE;
}

#ifdef TXBF_DYNAMIC_DISABLE


/*
	==========================================================================
	Description:
		(1) Dynamically disable BF for current assoc STAs by setting pEntry->rStaRecBf.ucAutoSoundingCtrl[DYNAMIC_BF_DISABLE]
		(2) Set pAd->CommonCfg.ucAutoSoundingCtrl[DYNAMIC_BF_DISABLE] so that STA in the future is also with BF disabled
		(3) Also updates WTBL BF flags

	==========================================================================
 */
INT DynamicTxBfDisable(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN		 fgDisable)
{
	BOOLEAN fgStatus = FALSE, fgETxBf, fgITxBf;
	INT i;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s]\n", __FUNCTION__));

	if (fgDisable) {
		fgETxBf = FALSE;
		fgITxBf = FALSE;
		/* Update ucAutoSoundingCtrol with DYNAMIC_BF_DISABLE
		  * to bypass WTBL BF flag setting and sounding packet Tx
		 */
		pAd->CommonCfg.ucAutoSoundingCtrl |= DYNAMIC_TXBF_DISABLE;
		/* Disable BFee by setting HW */
		AsicTxBfeeHwCtrl(pAd, FALSE);
	} else {
		fgETxBf = pAd->CommonCfg.ETxBfEnCond;
		fgITxBf = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
		pAd->CommonCfg.ucAutoSoundingCtrl = 0;
		/* Enable BFee by setting HW */
		AsicTxBfeeHwCtrl(pAd, TRUE);
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] fgETxBf=%d, fgITxBf=%d, ucAutoSoundingCtrl=%d\n", __FUNCTION__, fgETxBf, fgITxBf, pAd->CommonCfg.ucAutoSoundingCtrl));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {/* For every STA */
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry)) && (pEntry->Sst == SST_ASSOC)) {
			STA_REC_CFG_T StaCfg;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] Enable/Disable BF for wlanid %d \n", __FUNCTION__, i));

			os_zero_mem(&StaCfg, sizeof(STA_REC_CFG_T));
			pEntry->rStaRecBf.ucAutoSoundingCtrl = pAd->CommonCfg.ucAutoSoundingCtrl;

			StaCfg.MuarIdx = 0;
			StaCfg.ConnectionState = TRUE;
			StaCfg.ConnectionType = 0;
			StaCfg.u4EnableFeature = (1 << STA_REC_BF);
			StaCfg.ucBssIndex = pEntry->wdev->bss_info_argument.ucBssIndex;
			StaCfg.ucWlanIdx = i;
			StaCfg.pEntry = pEntry;
			/* update ucAutoSoundingCtrol and WTBL BF flag */
			if (CmdExtStaRecUpdate(pAd, StaCfg) == STATUS_TRUE) {
			    fgStatus = TRUE;
			}

			CmdTxBfTxApplyCtrl(pAd, i, fgETxBf, fgITxBf, FALSE, FALSE); /* Set WTBL IBF EBF */
		}
	}

	return fgStatus;
}
#endif /* TXBF_DYNAMIC_DISABLE */

#endif /* MT_MAC */

/*sta rec txbf feature decision*/
UINT32 starec_txbf_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT32 features = 0;

	if ((ad->CommonCfg.ETxBfEnCond == TRUE)
		|| (ad->CommonCfg.RegTransmitSetting.field.ITxBfEn == TRUE)) {

		if (entry && (!IS_ENTRY_NONE(entry))) {
			if (HcIsBfCapSupport(wdev)) {
				switch (wdev->wdev_type) {
				case WDEV_TYPE_AP:
					if (IS_ENTRY_CLIENT(entry)) {
						ad->fgApClientMode = FALSE;
						features |= STA_REC_BF_FEATURE;
					}
					break;
				case WDEV_TYPE_STA:
					/* we don't check IS_ENTRY_AP(entry) because there are some problems in the setting */
					ad->fgApClientMode = FALSE;
					features |= STA_REC_BF_FEATURE;
					break;
#ifdef APCLI_SUPPORT
				case WDEV_TYPE_APCLI:
					if ((IS_ENTRY_APCLI(entry) || IS_ENTRY_REPEATER(entry))) {
						ad->fgApClientMode = TRUE;

						if (ad->fgApCliBfStaRecRegister == FALSE) {
							ad->fgApCliBfStaRecRegister = TRUE;
							ad->ApCli_CmmWlanId = entry->wcid;
							features |= STA_REC_BF_FEATURE;
						}
					}
					break;
#endif /* APCLI_SUPPORT */

				default:
					break;
				}
			}
		}
	}

	/*return value, must use or operation*/
	*feature |= features;
	return TRUE;
}


#endif /* TXBF_SUPPORT */
