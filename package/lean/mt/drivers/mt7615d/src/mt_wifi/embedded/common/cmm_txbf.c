/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf.c

	Abstract:
	Tx Beamforming related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2009/11/04
*/

#include	"rt_config.h"

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


/*
	TxBFInit - Intialize TxBF fields in pEntry
		supportsETxBF - TRUE if client supports ETxBF
*/
VOID TxBFInit(
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
	pEntryTxBf->sndg0Mcs = 0;
#ifdef ETXBF_EN_COND3_SUPPORT
	pEntryTxBf->sndgRateIdx = 0;
	pEntryTxBf->sndg0RateIdx = 0;
	pEntryTxBf->sndg1Mcs = 0;
	pEntryTxBf->sndg1RateIdx = 0;
	pEntryTxBf->sndg1Snr0 = 0;
	pEntryTxBf->sndg1Snr1 = 0;
	pEntryTxBf->sndg1Snr2 = 0;
	pEntryTxBf->bf0Mcs = 0;
	pEntryTxBf->bf0RateIdx = 0;
	pEntryTxBf->bf1Mcs = 0;
	pEntryTxBf->bf1RateIdx = 0;
#endif /* EXTBF_EN_COND3_SUPPORT */
	pEntryTxBf->noSndgCnt = 0;
	pEntryTxBf->eTxBfEnCond = supportsETxBF ? pAd->CommonCfg.ETxBfEnCond : 0;
	pEntryTxBf->noSndgCntThrd = NO_SNDG_CNT_THRD;
	pEntryTxBf->ndpSndgStreams = pAd->Antenna.field.TxPath;
	/* If client supports ETxBf and ITxBF then give ETxBF priority over ITxBF */
	pEntryTxBf->iTxBfEn = pEntryTxBf->eTxBfEnCond > 0 ? 0 : pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;

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

/*
	Wrap function for TxBFInit
*/
VOID WrapTxBFInit(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN BOOLEAN			supportsETxBF)
{
	TXBF_MAC_TABLE_ENTRY TxBfMacEntry;
	TXBF_STATUS_INFO  TxBfInfo;
	HT_BF_CAP *pTxBFCap = &ie_list->HTCapability.TxBFCap;
	TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
	TxBfInfo.ucETxBfTxEn = (UCHAR) pAd->CommonCfg.ETxBfEnCond;
	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	TxBfInfo.ucITxBfTxEn = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
	mt_TxBFInit(pAd, &TxBfInfo, &TxBfMacEntry, pTxBFCap, supportsETxBF);
	pEntry->bfState        = TxBfMacEntry.bfState;
	pEntry->sndgMcs        = TxBfMacEntry.sndgMcs;
	pEntry->sndg0Snr0      = TxBfMacEntry.sndg0Snr0;
	pEntry->sndg0Snr1      = TxBfMacEntry.sndg0Snr1;
	pEntry->sndg0Snr2      = TxBfMacEntry.sndg0Snr2;
	pEntry->sndg0Mcs       = TxBfMacEntry.sndg0Mcs;
	pEntry->noSndgCnt      = TxBfMacEntry.noSndgCnt;
	pEntry->eTxBfEnCond    = TxBfMacEntry.eTxBfEnCond;
	pEntry->noSndgCntThrd  = TxBfMacEntry.noSndgCntThrd;
	pEntry->ndpSndgStreams = TxBfMacEntry.ndpSndgStreams;
	pEntry->iTxBfEn        = TxBfMacEntry.iTxBfEn;
}

BOOLEAN rtmp_chk_itxbf_calibration(
	IN RTMP_ADAPTER * pAd)
{
	INT calIdx, calCnt;
	USHORT offset, eeVal, *calptr;
	USHORT g_caladdr[] = {0x1a0, 0x1a2, 0x1b0, 0x1b2, 0x1b6, 0x1b8};
	USHORT a_caladdr[] = {0x1a4, 0x1a6, 0x1a8, 0x1aa, 0x1ac, 0x1ae, 0x1b4, 0x1ba, 0x1bc, 0x1be, 0x1c0, 0x1c2, 0x1c4, 0x1c6, 0x1c8};
	UINT32 ee_sum;
	BOOLEAN bCalibrated = TRUE;
	UCHAR Channel = HcGetChannelByBf(pAd);

	if (Channel <= 14) {
		calCnt = sizeof(g_caladdr) / sizeof(USHORT);
		calptr = &g_caladdr[0];
	} else {
		calCnt = sizeof(a_caladdr) / sizeof(USHORT);
		calptr = &a_caladdr[0];
	}

	ee_sum = 0;

	for (calIdx = 0; calIdx < calCnt; calIdx++) {
		offset = *(calptr + calIdx);
		RT28xx_EEPROM_READ16(pAd, offset, eeVal);
		ee_sum += eeVal;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Check EEPROM(offset=0x%x, eeVal=0x%x, ee_sum=0x%x)!\n",
				 offset, eeVal, ee_sum));

		if (eeVal != 0xffff && eeVal != 0)
			return TRUE;
	}

	if ((ee_sum == (0xffff * calCnt)) || (ee_sum == 0x0)) {
		bCalibrated = FALSE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EEPROM all 0xffff(cnt =%d, sum=0x%x), not valid calibration value!\n",
				 calCnt, ee_sum));
	}

	return bCalibrated;
}


VOID Trigger_Sounding_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			SndgType,
	IN	UCHAR			SndgBW,
	IN	UCHAR			SndgMcs,
	IN  MAC_TABLE_ENTRY * pEntry)
{
	/*
		SngType
			0: disable
			1 : sounding
			2: NDP sounding
	*/
	NdisAcquireSpinLock(&pEntry->TxSndgLock);
	pEntry->TxSndgType = SndgType;
	NdisReleaseSpinLock(&pEntry->TxSndgLock);
	RTMPSetTimer(&pEntry->eTxBfProbeTimer, ETXBF_PROBE_TIME);
	/*MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in Trigger_Sounding_Packet(): sndgType=%d, bw=%d, mcs=%d\n", SndgType, SndgBW, SndgMcs)); */
}


/*
	eTxBFProbing - called by Rate Adaptation routine each interval.
		Initiates a sounding packet if enabled.
*/
VOID eTxBFProbing(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry)
{
	if (pEntry->eTxBfEnCond == 0)
		pEntry->bfState = READY_FOR_SNDG0;
	else if (pEntry->bfState == READY_FOR_SNDG0 && pEntry->noSndgCnt >= pEntry->noSndgCntThrd) {
		/* Select NDP sounding, maximum streams */
		pEntry->sndgMcs = (pEntry->ndpSndgStreams == 3) ? 16 : 8;
		Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);
		pEntry->bfState = WAIT_SNDG_FB0;
		pEntry->noSndgCnt = 0;
	} else if (pEntry->bfState == READY_FOR_SNDG0)
		pEntry->noSndgCnt++;
	else
		pEntry->noSndgCnt = 0;
}

/*
	clientSupportsETxBF - returns true if client supports compatible Sounding
*/
BOOLEAN clientSupportsETxBF(
	IN	PRTMP_ADAPTER	 pAd,
	IN	HT_BF_CAP *pTxBFCap)
{
	BOOLEAN compCompat, noncompCompat;
	compCompat = (pTxBFCap->ExpComBF > 0) &&
				 /*(pTxBFCap->ComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath) && */
				 (pAd->CommonCfg.ETxBfNoncompress == 0);
	noncompCompat = (pTxBFCap->ExpNoComBF > 0)
					/* && (pTxBFCap->NoComSteerBFAntSup+1 >= pAd->Antenna.field.TxPath)*/;
	return pTxBFCap->RxNDPCapable == 1 && (compCompat || noncompCompat);
}

#ifndef MT_MAC
#ifdef VHT_TXBF_SUPPORT
/*
	clientSupportsETxBF - returns true if client supports compatible Sounding
*/
BOOLEAN clientSupportsVHTETxBF(
	IN	PRTMP_ADAPTER	 pAd,
	IN	VHT_CAP_INFO * pTxBFCap)
{
	return pTxBFCap->bfee_cap_su;
}
#endif
#endif


/*
	setETxBFCap - sets our ETxBF capabilities
*/
void setETxBFCap(RTMP_ADAPTER *pAd, HT_BF_CAP *pTxBFCap)
{
	if (pAd->CommonCfg.ETxBfIncapable)
		memset(pTxBFCap, 0, sizeof(*pTxBFCap));
	else {
		pTxBFCap->RxNDPCapable =  TRUE;
		pTxBFCap->TxNDPCapable =  TRUE;
		pTxBFCap->ExpNoComSteerCapable =  TRUE;
		pTxBFCap->ExpComSteerCapable = !pAd->CommonCfg.ETxBfNoncompress;
		pTxBFCap->ExpNoComBF = HT_ExBF_FB_CAP_IMMEDIATE;
		pTxBFCap->ExpComBF = pAd->CommonCfg.ETxBfNoncompress ? HT_ExBF_FB_CAP_NONE : HT_ExBF_FB_CAP_IMMEDIATE;
		pTxBFCap->MinGrouping = 3;
		pTxBFCap->NoComSteerBFAntSup = 2;
		pTxBFCap->ComSteerBFAntSup = 2;
		pTxBFCap->ChanEstimation = pAd->Antenna.field.RxPath - 1;
	}
}

#ifdef VHT_TXBF_SUPPORT
void setVHTETxBFCap(RTMP_ADAPTER *pAd, VHT_CAP_INFO *pTxBFCap)
{
	if (pAd->CommonCfg.ETxBfIncapable) {
		pTxBFCap->num_snd_dimension = 0;
		pTxBFCap->bfee_cap_mu = 0;
		pTxBFCap->bfee_cap_su = 0;
		pTxBFCap->bfer_cap_mu = 0;
		pTxBFCap->bfer_cap_su = 0;
		pTxBFCap->bfee_sts_cap = 0;
	} else {
		pTxBFCap->bfee_cap_su = 1;
		pTxBFCap->bfer_cap_su = 1;
		pTxBFCap->num_snd_dimension = 1;
		pTxBFCap->bfee_sts_cap = 1;
	}
}
#endif


#ifdef ETXBF_EN_COND3_SUPPORT
/*
	4. determine the best method among  mfb0, mfb1, snrComb0, snrComb1
	5. use the best method. if necessary, sndg with the mcs which resulting in the best snrComb.
*/
/*if mcs is not in group 1 */
VOID txSndgSameMcs(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY * pEntry,
	IN	UCHAR smoothMfb)/*smoothMfb should be the current mcs */
{
	PUCHAR		pTable;
	UCHAR		TableSize = 0;
	UCHAR		InitTxRateIdx, i, step;
	BOOLEAN		bWriteEnable;
	UCHAR		SndgType = SNDG_TYPE_SOUNDING;

	if (pEntry->eTxBfEnCond == 0) {
		pEntry->bfState = READY_FOR_SNDG0;
		return;
	}

	/*0. write down the current mfb0 */
	pEntry->mfb0 = smoothMfb;

	/* 1. sndg with current mcs, get snrComb0 */
	if (smoothMfb >> 3 > 0)
		pEntry->sndgMcs = smoothMfb;
	else {
		pEntry->sndgMcs = 8;
		SndgType = SNDG_TYPE_NDP;
	}

	/* if ndp sndg is forced by iwpriv command */
	if (pEntry->ndpSndgStreams == 2 || pEntry->ndpSndgStreams == 3) {
		SndgType = SNDG_TYPE_NDP;

		if (pEntry->ndpSndgStreams == 3)
			pEntry->sndgMcs = 16;
		else
			pEntry->sndgMcs = 8;
	}

	/*
		smoothMfb is guaranteed included in the current pTable because
		it is converted from received MFB in handleHtcField()
	*/
	/* TODO: MlmeSelectTxRateTable() is not supported on MT_MAC */
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;

	for (i = 1; i <= TableSize; i++) {
		if (pTable[i * step + 2] >= pEntry->sndgMcs)
			break;
	}

	if (i > TableSize)
		i = TableSize - 1;

	/*
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("txSndgSameMcs i = %x, step = %x, sndgMcs = %x CurrentMCS = %x\n",
					i, step, pEntry->sndgMcs, pTable[i*step+2]));
	*/
	pEntry->sndgRateIdx = pTable[i * step];

	if (pEntry->sndgMcs != pTable[i * step + 2]) {
		/*pEntry->sndgMcs = pTable[i*step+2];*/
		if (pTable[i * step + 2] > 16)
			pEntry->sndgMcs = 16;
		else if (pTable[i * step + 2] > 8)
			pEntry->sndgMcs = 8;
		else
			pEntry->sndgMcs = 0;

		SndgType = SNDG_TYPE_NDP;
	}

	/* Enable/disable BF matrix writing */
	if  (pEntry->eTxBfEnCond == 1 || pEntry->eTxBfEnCond == 2) {
		bWriteEnable = TRUE;
		pEntry->HTPhyMode.field.eTxBF = 1;
	} else
		bWriteEnable = FALSE;

	/* send a sounding packet*/
	Trigger_Sounding_Packet(pAd, SndgType, 0, pEntry->sndgMcs, pEntry);
	pEntry->bfState = WAIT_SNDG_FB0;
	pEntry->noSndgCnt = 0;
}


/*
	txSndgOtherGroup - NOTE: currently unused.
		Only called when ETxBfEnCond==3
*/
VOID txSndgOtherGroup(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY * pEntry)
{
	PUCHAR		pTable;
	UCHAR		TableSize = 0;
	UCHAR		InitTxRateIdx, i, step;
	UCHAR		byteValue = 0;
	UCHAR		SndgType = SNDG_TYPE_SOUNDING;

	/* tx sndg with mcs in the other group */
	if ((pEntry->sndgMcs) >> 3 == 2) {
		pEntry->sndgMcs = 8;
		SndgType = SNDG_TYPE_NDP;
	} else {
		pEntry->sndgMcs = 16;
		SndgType = SNDG_TYPE_NDP;
	}

	/* copied from txSndgSameMcs() */
	/* TODO: MlmeSelectTxRateTable() is not supported on MT_MAC */
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;

	for (i = 1; i <= TableSize; i++) {
		if (pTable[i * step + 2] >= pEntry->sndgMcs)
			break;
	}

	if (i > TableSize)
		i = TableSize - 1;

	pEntry->sndgRateIdx = pTable[i * step];

	if (pEntry->sndgMcs != pTable[i * step + 2]) {
		pEntry->sndgMcs = pTable[i * step + 2];
		SndgType = SNDG_TYPE_NDP;
	}

	/*---copied from txSndgSameMcs() end */
	/* disable BF matrix writing */
	Trigger_Sounding_Packet(pAd, SndgType, 0, pEntry->sndgMcs, pEntry);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in txSndgOtherGroup(): tx the second SNDG, enter state WAIT_SNDG_FB1\n"));
	pEntry->bfState = WAIT_SNDG_FB1;
}

VOID txMrqInvTxBF(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY * pEntry)

{
	pEntry->toTxMrq = TRUE;
	pEntry->msiToTx = MSI_TOGGLE_BF;
	/*	pEntry->HTPhyMode.field.TxBF = ~pEntry->HTPhyMode.field.TxBF;done in another function call*/
	RTMPSetTimer(&pEntry->eTxBfProbeTimer, ETXBF_PROBE_TIME);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in txMrqInvTxBF(): tx the second MRQ, enter state WAIT_MFB\n"));
	pEntry->bfState = WAIT_MFB;
}

UINT convertSnrToThroughput(
	IN UCHAR streamsIn,
	IN INT snr0,
	IN INT snr1,
	IN INT snr2,
	IN PUCHAR pTable,
	OUT UCHAR *bestMcsPtr,
	OUT UCHAR *bestRateIdxPtr

)
{
	UCHAR streams;
	INT	snrTemp;
	UCHAR	i, j;
	SHORT idx;
	SHORT group;
	INT	snrTemp1[3];
	INT snr[] = {snr0, snr1, snr2};
	INT snrSum, tpTemp, bestTp = 0;
	SHORT thrdTemp;
	BOOLEAN isMcsValid[24];
	UCHAR	rateIdx[24], step, tableSize;
	UCHAR mcs;
#ifdef NEW_RATE_ADAPT_SUPPORT

	if (ADAPT_RATE_TABLE(pTable))
		step = 10;
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		step = 5;

	tableSize = RATE_TABLE_SIZE(pTable);

	for (i = 0; i < 24; i++) {
		isMcsValid[i] = FALSE;
		rateIdx[i] = 0;
	}

	for (i = 1; i <= tableSize; i++) {
		isMcsValid[pTable[i * step + 2]] = TRUE;
		rateIdx[pTable[i * step + 2]] = pTable[i * step];
	}

	if (streamsIn > 3) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("convertSnrToThroughput(): %d streams are not supported!!!", streamsIn));
		streams = 3;
	} else
		streams = streamsIn;

	for (i = 0; i < streams; i++) {
		for (j = i + 1; j < streams; j++) {
			if (snr[i] < snr[j]) {
				snrTemp = snr[i];
				snr[i] = snr[j];
				snr[j] = snrTemp;
			}
		}
	}

	(*bestMcsPtr) = 0;
	(*bestRateIdxPtr) = rateIdx[0];

	for (group = streams - 1; group >= 0; group--) {
		snrTemp1[0] = snr[0];
		snrTemp1[1] = snr[1];
		snrTemp1[2] = snr[2];

		/*SNR processing for each group according to the baseband implementation, for example MRC*/
		switch (group) {
		case 0:
			snrTemp1[1] = 0;
			snrTemp1[2] = 0;
			break;

		case 1:
			snrTemp1[2] = 0;
			break;

		case 2:
			break;

		default:
			break;
		}

		snrSum = snr[0] + snr[1] + snr[2];

		for (idx = 7; idx >= 0; idx--) {
			mcs = group * 8 + idx;
			thrdTemp = groupThrd[mcs];
			tpTemp = 0;

			if (groupMethod[mcs] == 0) {
				if (snrSum > thrdTemp)
					tpTemp =  ((snrSum - thrdTemp) * dataRate[mcs]) >> groupShift[group];
			} else {
				if (group == 1)
					snrTemp1[2] = thrdTemp + 1;

				if (snrTemp1[0] > thrdTemp && snrTemp1[1] > thrdTemp && snrTemp1[2] > thrdTemp)
					tpTemp = ((snrTemp1[0] - thrdTemp) * (snrTemp1[1] - thrdTemp) * (snrTemp1[2] - thrdTemp) * dataRate[mcs]) >> groupShift[group]; /* have to be revised!!!*/
			}

			if (tpTemp > dataRate[mcs])
				tpTemp = dataRate[mcs];

			if (tpTemp > bestTp && isMcsValid[mcs] == TRUE) {
				bestTp = tpTemp;
				(*bestMcsPtr) = mcs;
				(*bestRateIdxPtr) = rateIdx[mcs];
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("convertSnrToThroughput(): new candidate snr0=%d, snr1=%d, snr2=%d, tp=%d, best MCS=%d\n", snrTemp1[0], snrTemp1[1], snrTemp1[2], tpTemp, *bestMcsPtr));
			}
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("convertSnrToThroughput(): snr0=%d, snr1=%d, snr2=%d, tp=%d, best MCS=%d\n", snr0, snr1, snr2, bestTp, *bestMcsPtr));
	return bestTp;
}


/*
	NOTE: currently unused. Only called when ETxBfEnCond==3
*/
VOID chooseBestMethod(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY * pEntry,
	IN	UCHAR			mfb)
{
	/*	UCHAR bestMethod;0:original, 1:inverted TxBF, 2:first sndg, 3:second sndg*/
	UINT tp[4], bestTp;
	UCHAR streams, i;
	PUCHAR		pTable;
	UCHAR		TableSize = 0;
	UCHAR		InitTxRateIdx;
	UCHAR		byteValue = 0;
	pEntry->mfb1 = mfb;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): received the second MFB %d, noted as mfb1\n", pEntry->mfb1));

	if ((pEntry->HTCapability.MCSSet[2] == 0xff) && (wlan_operate_get_tx_stream(pEntry->wdev) == 3))
		streams = 3;
	else if (pEntry->HTCapability.MCSSet[0] == 0xff && pEntry->HTCapability.MCSSet[1] == 0xff && wlan_operate_get_tx_stream(pEntry->wdev) > 1
			 && (wlan_operate_get_tx_stream(pEntry->wdev) == 2 || pEntry->HTCapability.MCSSet[2] == 0x0))
		streams = 2;
	else
		streams = 1;

	/* TODO: MlmeSelectTxRateTable() is not supported on MT_MAC */
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
	tp[2] = convertSnrToThroughput(streams, pEntry->sndg0Snr0,  pEntry->sndg0Snr1, pEntry->sndg0Snr2, pTable, &(pEntry->bf0Mcs), &(pEntry->bf0RateIdx));
	tp[3] = convertSnrToThroughput(streams, pEntry->sndg1Snr0,  pEntry->sndg1Snr1, pEntry->sndg1Snr2, pTable, &(pEntry->bf1Mcs), &(pEntry->bf1RateIdx));
	tp[0] = dataRate[pEntry->mfb0];
	tp[1] = dataRate[pEntry->mfb1];
	bestTp = 0;

	for (i = 0; i < 4; i++) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): predicted throughput of method %d = %d\n", i, tp[i]));

		if (tp[i] > bestTp) {
			bestTp = tp[i];
			pEntry->bestMethod = i;
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): method %d is chosen\n", pEntry->bestMethod));

	switch (pEntry->bestMethod) {
	case 0:/*do nothing*/
		pEntry->bfState = READY_FOR_SNDG0;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): do nothing, and enter state READY_FOR_SNDG0\n"));
		break;

	case 1:
		pEntry->HTPhyMode.field.eTxBF = ~pEntry->HTPhyMode.field.eTxBF;
		pEntry->bfState = READY_FOR_SNDG0;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): invert the ETxBF status, and enter state READY_FOR_SNDG0\n"));
		break;

	case 2:
		pEntry->sndgMcs = pEntry->sndg0Mcs;
		pEntry->sndgRateIdx = pEntry->sndg0RateIdx;

		/* enable BF matrix writing */
		if (pEntry->sndgRateIdx == pEntry->CurrTxRateIndex)
			Trigger_Sounding_Packet(pAd, SNDG_TYPE_SOUNDING, 0, pEntry->sndgMcs, pEntry);
		else
			Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): tx the SNDG of the best method, enter state WAIT_BEST_SNDG\n"));
		pEntry->bfState = WAIT_BEST_SNDG;
		break;

	case 3:
		/* tx sndg with mcs in the other group */
		pEntry->sndgMcs = pEntry->sndg1Mcs;
		pEntry->sndgRateIdx = pEntry->sndg1RateIdx;

		/* enable BF matrix writing */
		if (pEntry->sndgRateIdx == pEntry->CurrTxRateIndex)
			Trigger_Sounding_Packet(pAd, SNDG_TYPE_SOUNDING, 0, pEntry->sndgMcs, pEntry);
		else
			Trigger_Sounding_Packet(pAd, SNDG_TYPE_NDP, 0, pEntry->sndgMcs, pEntry);

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in chooseBestMethod(): tx the SNDG of the best method, enter state WAIT_BEST_SNDG\n"));
		pEntry->bfState = WAIT_BEST_SNDG;
		break;
	}
}


/*
	NOTE: currently unused. Only called when ETxBfEnCond==3
*/
VOID rxBestSndg(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY * pEntry)
{
	/*set the best mcs of this BF matrix*/
	if (pEntry->bestMethod == 2) {
		pEntry->CurrTxRate = pEntry->bf0Mcs;
		pEntry->CurrTxRateIndex = pEntry->bf0RateIdx;
	} else if (pEntry->bestMethod == 3) {
		pEntry->CurrTxRate = pEntry->bf1Mcs;
		pEntry->CurrTxRateIndex = pEntry->bf1RateIdx;
	}

	pEntry->HTPhyMode.field.eTxBF = 1;
	/*must sync the timing of using new BF matrix and its bfRateIdx!!!*/
	/*need to reset counter for rateAdapt and may have to skip one adaptation when the new BF matrix is applied!!!*/
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in rxBestSndg(): received the feedback of the best SNDG, and enter state READY_FOR_SNDG0\n"));
	pEntry->bfState = READY_FOR_SNDG0;
}
#endif	/* ETXBF_EN_COND3_SUPPORT */


VOID handleBfFb(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->Wcid))
		return;

	pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	/*
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF :(%02x:%02x:%02x:%02x:%02x:%02x)\n",
							PRINT_MAC(pEntry->Addr)));
	*/

	if (pEntry->bfState == WAIT_SNDG_FB0) {
		int Nc = ((pRxBlk->pData)[2] & 0x3) + 1;
		/*record the snr comb*/
		pEntry->sndg0Snr0 = 88 + (CHAR)(pRxBlk->pData[8]);
		pEntry->sndg0Snr1 = (Nc < 2) ? 0 : 88 + (CHAR)(pRxBlk->pData[9]);
		pEntry->sndg0Snr2 = (Nc < 3) ? 0 : 88 + (CHAR)(pRxBlk->pData[10]);
		pEntry->sndg0Mcs = pEntry->sndgMcs;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("   ETxBF: aid=%d  snr %d.%02d %d.%02d %d.%02d\n",
				 pRxBlk->wcid,
				 pEntry->sndg0Snr0 / 4,  25 * (pEntry->sndg0Snr0 & 0x3),
				 pEntry->sndg0Snr1 / 4,  25 * (pEntry->sndg0Snr1 & 0x3),
				 pEntry->sndg0Snr2 / 4,  25 * (pEntry->sndg0Snr2 & 0x3)));
#ifdef ETXBF_EN_COND3_SUPPORT

		if (pEntry->eTxBfEnCond == 1 || pEntry->eTxBfEnCond == 2)
			pEntry->bfState = READY_FOR_SNDG0;
		/* 2. sndg with current mcs+8 or -8, get snrComb1*/
		else if (pEntry->eTxBfEnCond == 3)
			txSndgOtherGroup(pAd, pEntry);

#else
		pEntry->bfState = READY_FOR_SNDG0;
#endif
	}

#ifdef ETXBF_EN_COND3_SUPPORT
	else if (pEntry->bfState == WAIT_SNDG_FB1) {
		/* 3. mrq with inverted TxBF status, get mfb1*/
		if (TRUE) {
			int Nc  = ((pRxBlk->pData)[2] & 0x3) + 1;
			/* record the snr comb */
			pEntry->sndg1Snr0 = 88 + (CHAR)(pRxBlk->pData[8]);
			pEntry->sndg1Snr1 = (Nc < 2) ? 0  : 88 + (CHAR)(pRxBlk->pData[9]);
			pEntry->sndg1Snr2 = (Nc < 3) ? 0  : 88 + (CHAR)(pRxBlk->pData[10]);
			pEntry->sndg1Mcs = pEntry->sndgMcs;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("   ETxBF: mcs%d, snr %d  %d %d\n",  pEntry->sndg1Mcs, pEntry->sndg1Snr0, pEntry->sndg1Snr1, pEntry->sndg1Snr2));
			txMrqInvTxBF(pAd,  pEntry);
		} else
			chooseBestMethod(pAd, pEntry, 0);
	} else if (pEntry->bfState == WAIT_USELESS_RSP) {
		int Nc  = ((pRxBlk->pData)[2] & 0x3) + 1;
		pEntry->sndg0Snr0  = 88 + (CHAR)(pRxBlk->pData[8]);
		pEntry->sndg0Snr1 = (Nc < 2) ? 0  : 88 + (CHAR)(pRxBlk->pData[9]);
		pEntry->sndg0Snr2 = (Nc < 3) ? 0  : 88 + (CHAR)(pRxBlk->pData[10]);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("   ETxBF: mcs%d, snr %d  %d %d\n",  pEntry->sndg1Mcs, pEntry->sndg1Snr0, pEntry->sndg1Snr1, pEntry->sndg1Snr2));
		txSndgSameMcs(pAd, pEntry, /*pRxBlk,*/ pEntry->lastLegalMfb);
	} else if (pEntry->bfState == WAIT_BEST_SNDG)
		rxBestSndg(pAd, pEntry);

#endif	/* ETXBF_EN_COND3_SUPPORT */
}


VOID handleHtcField(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
#ifdef MFB_SUPPORT
	UCHAR mfb = ((PHT_CONTROL)(pRxBlk->pData))->MFBorASC;
	UCHAR mfsi = ((PHT_CONTROL)(pRxBlk->pData))->MFSI;
	UCHAR legalMfb = 0, legalMfbIdx = 0, smoothMfb = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR i, j;
	UCHAR *pTable;
	UCHAR TableSize = 0;
	UCHAR InitTxRateIdx;
	UCHAR snr[] = {pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1], pRxBlk->rx_signal.raw_snr[2]};
	UCHAR snrTemp1[3];
	UCHAR snrTemp;
	UCHAR streams;
	UINT tpTemp, bestTp = 0, snrSum;
	SHORT thrdTemp;
	SHORT group, idx;
	UCHAR mcs;
	RTMP_RA_GRP_TB *pLegalMfbRS3S = NULL;
	RTMP_RA_LEGACY_TB *pLegalMfbRS = NULL;

	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->Wcid))
		return;

	pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);

	/* if MFB is received, have to rule out the case when mai==14 */
	if (!(((PHT_CONTROL)(pRxBlk->pData))->MRQ == 0 && ((PHT_CONTROL)(pRxBlk->pData))->MSI == 7)
		&& mfb != 127) {
		/* need a timer in case there is no mfb with this mfsi */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	MFB in handleHtcField(): MFB %d is received\n", mfb));

		/* check if the mfb is valid. if not, convert to a valid mcs */
		if (mfb > 76)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Error in handleHtcField: received MFB > 76\n"));

		if (pEntry->HTCapability.MCSSet[0] == 0xff && pEntry->HTCapability.MCSSet[1] == 0xff
			&& pEntry->HTCapability.MCSSet[2] == 0xff && wlan_operate_get_tx_stream(pEntry->wdev) == 3)
			legalMfb = mcsToLowerMcs[4 * mfb + 3];
		else if (pEntry->HTCapability.MCSSet[0] == 0xff && pEntry->HTCapability.MCSSet[1] == 0xff && wlan_operate_get_tx_stream(pEntry->wdev) > 1
				 && (wlan_operate_get_tx_stream(pEntry->wdev) == 2 || pEntry->HTCapability.MCSSet[2] == 0x0))
			legalMfb = mcsToLowerMcs[4 * mfb + 2];
		else if (pEntry->HTCapability.MCSSet[0] == 0xff && (wlan_operate_get_tx_stream(pEntry->wdev) == 1 || pEntry->HTCapability.MCSSet[1] == 0x0))
			legalMfb = mcsToLowerMcs[4 * mfb + 1];
		else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("no available MFB mapping for the received MFB\n"));

		/*
			have to rule out the mfb that the Rx shouldn't be able to suggest???
			for example, mrq was sent with 2 streams but the Rx suggests MCS
			with 3 streams
		*/
#ifdef ETXBF_EN_COND3_SUPPORT

		if (mfsi == MSI_TOGGLE_BF) {
			if (pEntry->bfState == WAIT_MFB)
				chooseBestMethod(pAd,  pEntry, legalMfb);
			else if (pEntry->bfState == WAIT_USELESS_RSP)
				txSndgSameMcs(pAd, pEntry,/* pRxBlk,*/ legalMfb);
		}

#endif
	}

	if (!(((PHT_CONTROL)(pRxBlk->pData))->MRQ == 0 && ((PHT_CONTROL)(pRxBlk->pData))->MSI == 7 && mfsi != MSI_TOGGLE_BF)
		&& mfb != 127) {
		/* the main body of algorithm */
		/* legalMfb smoothing */
		/* TODO: MlmeSelectTxRateTable() is not supported on MT_MAC */
		MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);
#ifdef NEW_RATE_ADAPT_SUPPORT

		if (ADAPT_RATE_TABLE(pTable)) {
			for (i = 1; i <= RATE_TABLE_SIZE(pTable); i++) {
				if (legalMfb == pTable[i * 10 + 2]) {
					legalMfbIdx = pTable[i * 10];
					pLegalMfbRS3S = (RTMP_RA_GRP_TB *) &pTable[i * 10];
					pLegalMfbRS = (RTMP_RA_LEGACY_TB *) &pTable[i * 10];
					break;
				}
			}

			/* pLegalMfbRS3S may be null if pLegalMfbRS3S is not found!!! */
			if (pEntry->lastLegalMfb == pTable[(pLegalMfbRS3S->downMcs + 1) * 10 + 2] || pEntry->lastLegalMfb == pTable[(pLegalMfbRS3S->upMcs1 + 1) * 10 + 2]
				|| pEntry->lastLegalMfb == pTable[(pLegalMfbRS3S->upMcs2 + 1) * 10 + 2] || pEntry->lastLegalMfb == pTable[(pLegalMfbRS3S->upMcs3 + 1) * 10 + 2])
				smoothMfb = pEntry->lastLegalMfb;
			else
				smoothMfb = legalMfb;
		} else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		{
			for (i = 1; i <= RATE_TABLE_SIZE(pTable); i++) {
				if (legalMfb == pTable[i * 5 + 2]) {
					legalMfbIdx = pTable[i * 5];
					pLegalMfbRS = (RTMP_RA_LEGACY_TB *) &pTable[i * 5];
					break;
				}
			}

			if ((pEntry->lastLegalMfb <= legalMfb + 1 || pEntry->lastLegalMfb + 1 >= legalMfb) && ((legalMfb >> 3) == (pEntry->lastLegalMfb >> 3)))
				smoothMfb = pEntry->lastLegalMfb;
			else
				smoothMfb = legalMfb;
		}

		if (smoothMfb != pEntry->lastLegalMfb && smoothMfb != pTable[(pEntry->CurrTxRateIndex + 1) * 10 + 2]) {
			/* if mfb changes and mfb is different from current mcs: means channel change */
#ifdef NEW_RATE_ADAPT_SUPPORT
			/* TODO: MlmeSetMcsGroup() is not supported on MT_MAC */
			if ((ADAPT_RATE_TABLE(pTable)))
				MlmeSetMcsGroup(pAd, pEntry);

#endif /* NEW_RATE_ADAPT_SUPPORT */
			pEntry->CurrTxRateIndex = legalMfbIdx;
			/* TODO: MlmeClearTxQuality() is not supported on MT_MAC */
			MlmeClearTxQuality(pEntry);/* clear all history, same as train up, purpose??? */
			NdisAcquireSpinLock(&pEntry->fLastChangeAccordingMfbLock);
			NdisMoveMemory(pEntry->LegalMfbRS, pLegalMfbRS, sizeof(RTMP_RA_LEGACY_TB));
			pEntry->fLastChangeAccordingMfb = TRUE;
			/* reset all OneSecTx counters */
			/* RESET_ONE_SEC_TX_CNT(pEntry); */
			NdisReleaseSpinLock(&pEntry->fLastChangeAccordingMfbLock);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	MFB in handleHtcField(): MFB changes and use the new mfb=%d, mfbIdx=%d\n", legalMfb, legalMfbIdx));
			/*			pEntry->isMfbChanged = TRUE; */

			if ((pEntry->HTCapability.TxBFCap.ExpNoComBF && pAd->CommonCfg.HtCapability.TxBFCap.TxSoundCapable
				 && pAd->CommonCfg.HtCapability.TxBFCap.ExpNoComSteerCapable)) {
				/* support ETxBF. what's the correct criterion???? */
				/* the process is triggered by channel change here. have to add the mechanism where the process is triggered by timer expiration!!! */
#ifdef ETXBF_EN_COND3_SUPPORT
				if (pEntry->eTxBfEnCond == 3) {
					if (pEntry->bfState == READY_FOR_SNDG0) {
						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETxBF in handleHtcField(): detect MFB change, set pEntry->mfb0=%d\n", pEntry->mfb0));
						txSndgSameMcs(pAd, pEntry, /*pRxBlk,*/ legalMfb);
					} else {
						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ETxBF in handleHtcField(): detect channel change before enter the ETxBF probe process is complete, enter state WAIT_USELESS_RSP\n"));
						pEntry->bfState = WAIT_USELESS_RSP;
					}
				}

#endif
				/* write down the current MFB and ixTxBF. currentMFB is recorded in lastLegalMfb */
				/* send one packet with reverse TxBF, 2 stream sounding */
			}
		}

		/* post-prossessing */
		if (pEntry->fLastChangeAccordingMfb)
			pEntry->lastLegalMfb = legalMfb;
	}

	/*if mrq is received*/
	if (((PHT_CONTROL)(pRxBlk->pData))->MRQ == 1) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	MFB in handleHtcField(): MRQ is received\n"));

		/*
			Assumption:
				snr are not sorted, wait for John or Julian's answer as to
				the SNR values of unused streams
		*/
		if ((pEntry->HTCapability.MCSSet[2] == 0xff && wlan_operate_get_tx_stream(pEntry->wdev) == 3))
			streams = 3;
		else if (pEntry->HTCapability.MCSSet[0] == 0xff && pEntry->HTCapability.MCSSet[1] == 0xff && wlan_operate_get_tx_stream(pEntry->wdev) > 1
				 && (wlan_operate_get_tx_stream(pEntry->wdev) == 2 || pEntry->HTCapability.MCSSet[2] == 0x0))
			streams = 2;
		else
			streams = 1;

		/*sort such that snr[0]>=snr[1]>=snr[2]. sorting is required for group 2 so that the best 2 streams are used.*/
		for (i = 0; i < streams; i++) {
			for (j = i + 1; j < streams; j++) {
				if (snr[i] < snr[j]) {
					snrTemp = snr[i];
					snr[i] = snr[j];
					snr[j] = snrTemp;
				}
			}
		}

		for (group = streams - 1; group >= 0; group--) {
			snrTemp1[0] = snr[0];
			snrTemp1[1] = snr[1];
			snrTemp1[2] = snr[2];

			/*SNR processing for each group according to the baseband implementation, for example MRC*/
			switch (group) {
			case 0:
				snrTemp1[1] = 0;
				snrTemp1[2] = 0;
				break;

			case 1:
				snrTemp1[2] = 0;
				break;

			case 2:
				break;

			default:
				break;
			}

			snrSum = snr[0] + snr[1] + snr[2];

			for (idx = 8; idx >= 1; idx--) {
				mcs = group * 8 + idx - 1;
				thrdTemp = groupThrd[mcs];
				tpTemp = 0;

				if (groupMethod[mcs] == 0) {
					if (snrSum > thrdTemp)
						tpTemp =  ((snrSum - thrdTemp) * dataRate[mcs]) >> groupShift[group];
				} else {
					if (group == 1)
						snrTemp1[2] = thrdTemp + 1;

					if (snrTemp1[0] > thrdTemp && snrTemp1[1] > thrdTemp && snrTemp1[2] > thrdTemp)
						tpTemp = ((snrTemp1[0] - thrdTemp) * (snrTemp1[1] - thrdTemp) * (snrTemp1[2] - thrdTemp) * dataRate[mcs]) >> groupShift[group]; /* have to be revised!!!			*/
				}

				if (tpTemp > dataRate[mcs])
					tpTemp = dataRate[mcs];

				if (tpTemp > bestTp) {
					bestTp = tpTemp;
					pEntry->mfbToTx = mcs;
				}
			}
		}

		pEntry->toTxMfb = 1;/*should be reset to 0 when mfb is actually sent out!!!*/
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("	MFB in handleHtcField(): MFB %d is going to be sent\n", pEntry->mfbToTx));
	}

#endif	/* MFB_SUPPORT */
}


void eTxBfProbeTimerExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	MAC_TABLE_ENTRY     *pEntry = (PMAC_TABLE_ENTRY) FunctionContext;
#ifdef ETXBF_EN_COND3_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pEntry->pAd;
#endif

	if (pEntry->bfState == WAIT_SNDG_FB0) {
		/*record the snr comb*/
		pEntry->sndg0Snr0 = -128;
		pEntry->sndg0Snr1 = -128;
		pEntry->sndg0Snr2 = -128;
		pEntry->sndg0Mcs = pEntry->sndgMcs;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("   ETxBF: timer of WAIT_SNDG_FB0 expires\n"));
#ifdef ETXBF_EN_COND3_SUPPORT

		if (pEntry->eTxBfEnCond == 1 || pEntry->eTxBfEnCond == 2)
			pEntry->bfState = READY_FOR_SNDG0;
		else if (pEntry->eTxBfEnCond == 3)
			txSndgOtherGroup(pAd, pEntry);

#else
		pEntry->bfState = READY_FOR_SNDG0;
#endif
	}

#ifdef ETXBF_EN_COND3_SUPPORT
	else if (pEntry->bfState == WAIT_SNDG_FB1) {
		/*record the snr comb*/
		pEntry->sndg1Snr0 = -128;
		pEntry->sndg1Snr1 = -128;
		pEntry->sndg1Snr2 = -128;
		pEntry->sndg1Mcs = pEntry->sndgMcs;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("   ETxBF: timer of WAIT_SNDG_FB1 expires, run txMrqInvTxBF()\n"));
		txMrqInvTxBF(pAd, pEntry);
	} else if (pEntry->bfState == WAIT_MFB) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("   ETxBF: timer of WAIT_MFB expires, run chooseBestMethod()\n"));
		chooseBestMethod(pAd, pEntry, 0);
	} else if (pEntry->bfState == WAIT_BEST_SNDG) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("   ETxBF: timer of WAIT_BEST_SNDG expires, run rxBestSndg()\n"));
		rxBestSndg(pAd, pEntry);
	}

#endif /* ETXBF_EN_COND3_SUPPORT */
}


#ifdef MFB_SUPPORT
VOID MFB_PerPareMRQ(
	IN	PRTMP_ADAPTER	pAd,
	OUT	VOID *pBuf,
	IN	PMAC_TABLE_ENTRY pEntry)
{
	PHT_CONTROL pHT_Control;

	/*	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----> MFB_PerPareMRQ\n"));*/
	if (pEntry->HTCapability.ExtHtCapInfo.MCSFeedback >= MCSFBK_MRQ) {
		pHT_Control = (HT_CONTROL *)pBuf;
		pHT_Control->MRQ = 1;

		if (pEntry->msiToTx == MSI_TOGGLE_BF) {
			if (pEntry->mrqCnt == 0)
				pEntry->mrqCnt = TOGGLE_BF_PKTS;
			else {
				(pEntry->mrqCnt)--;

				if (pEntry->mrqCnt == 0)
					pEntry->msiToTx = 0;
			}
		}

		pHT_Control->MSI = pEntry->msiToTx;

		/*update region*/
		if (pEntry->msiToTx == MSI_TOGGLE_BF - 1) /*MSI_TOGGLE_BF==6 is used to indicate the TxBF status is inverted for this packet*/
			pEntry->msiToTx = 0;
		else if (pEntry->msiToTx !=  MSI_TOGGLE_BF)
			pEntry->msiToTx++;
	}

	/*	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- MFB_PerPareMRQ\n"));*/
}


/*
	Need to be completed!!!!!!!!!!!!!!!!!
*/
VOID MFB_PerPareMFB(
	IN	PRTMP_ADAPTER	pAd,
	OUT	VOID *pBuf,
	IN	PMAC_TABLE_ENTRY pEntry)
{
	/*	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("-----> MFB_PerPareMRQ\n")); */
	/*	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<----- MFB_PerPareMRQ\n"));*/
}
#endif /* MFB_SUPPORT */


/* MlmeTxBfAllowed - returns true if ETxBF or ITxBF is supported and pTxRate is a valid BF mode */
BOOLEAN MlmeTxBfAllowed(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate)
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

#endif /* TXBF_SUPPORT */

