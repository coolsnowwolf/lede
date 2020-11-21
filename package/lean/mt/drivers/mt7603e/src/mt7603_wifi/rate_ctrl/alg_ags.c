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

	Abstract:

	All related AGS (Adaptive Group Switching) function body.

***************************************************************************/ 

#ifdef AGS_SUPPORT

#include "rt_config.h"


/*
	HT Rate Table format
	[Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]

	[Mode]*: 
		bit0: STBC -STBC_XXX
		bit1: Short GI - GI_XXX
		bit2~3: BW - BW_XXX
		bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
		bit7: Reserved

	[Mode]**
		bit0~1: Nss - NSS_XXX (VHT only)
		bit2~7: Reserved
*/

/* AGS: 1x1 HT-capable rate table */
UCHAR AGS1x1HTRateTable[] = {
	0x09, 0x08, 0, 0, 0, 0, 0, 0, 0, 0,	/* Initial used item after association: the number of rate indexes, the initial mcs */
	0x00, 0x21, 0, 0, 30, 101, 0, 16, 8, 1,	/* MCS 0 */
	0x01, 0x21, 0, 1, 20, 50, 0, 16, 9, 2,	/* MCS 1 */
	0x02, 0x21, 0, 2, 20, 50, 1, 17, 9, 3,	/* MCS 2 */
	0x03, 0x21, 0, 3, 15, 50, 2, 17, 10, 4,	/* MCS 3 */
	0x04, 0x21, 0, 4, 15, 30, 3, 18, 11, 5,	/* MCS 4 */
	0x05, 0x21, 0, 5, 10, 25, 4, 18, 12, 6,	/* MCS 5 */
	0x06, 0x21, 0, 6, 8, 14, 5, 19, 12, 7,	/* MCS 6 */
	0x07, 0x21, 0, 7, 8, 14, 6, 19, 12, 8,	/* MCS 7 */
	0x08, 0x23, 0, 7, 8, 14, 7, 19, 12, 8,	/* MCS 7 + Short GI */
};


/* AGS: 2x2 HT-capable rate table */
UCHAR AGS2x2HTRateTable[] = {
	0x11, 0x10, 0, 0, 0, 0, 0, 0, 0, 0,	/* Initial used item after association: the number of rate indexes, the initial mcs */
	0x00, 0x21, 0, 0, 30, 101, 0, 16, 8, 1,	/* MCS 0 */
	0x01, 0x21, 0, 1, 20, 50, 0, 16, 9, 2,	/* MCS 1 */
	0x02, 0x21, 0, 2, 20, 50, 1, 17, 9, 3,	/* MCS 2 */
	0x03, 0x21, 0, 3, 15, 50, 2, 17, 10, 4,	/* MCS 3 */
	0x04, 0x21, 0, 4, 15, 30, 3, 18, 11, 5,	/* MCS 4 */
	0x05, 0x21, 0, 5, 10, 25, 4, 18, 12, 6,	/* MCS 5 */
	0x06, 0x21, 0, 6, 8, 14, 5, 19, 12, 7,	/* MCS 6 */
	0x07, 0x21, 0, 7, 8, 14, 6, 19, 12, 7,	/* MCS 7 */
	0x08, 0x20, 0, 8, 30, 50, 0, 16, 9, 2,	/* MCS 8 */
	0x09, 0x20, 0, 9, 20, 50, 8, 17, 10, 4,	/* MCS 9 */
	0x0A, 0x20, 0, 10, 20, 50, 9, 18, 11, 5,	/* MCS 10 */
	0x0B, 0x20, 0, 11, 15, 30, 10, 18, 12, 6,	/* MCS 11 */
	0x0C, 0x20, 0, 12, 15, 30, 11, 20, 13, 12,	/* MCS 12 */
	0x0D, 0x20, 0, 13, 8, 20, 12, 20, 14, 13,	/* MCS 13 */
	0x0E, 0x20, 0, 14, 8, 18, 13, 21, 15, 14,	/* MCS 14 */
	0x0F, 0x20, 0, 15, 8, 25, 14, 21, 16, 15,	/* MCS 15 */
	0x10, 0x22, 0, 15, 8, 25, 15, 21, 16, 16,	/* MCS 15 + Short GI */
};


/* AGS: 3x3 HT-capable rate table */
UCHAR AGS3x3HTRateTable[] = {
	0x19, 0x18, 0, 0, 0, 0, 0, 0, 0, 0,	/* Initial used item after association: the number of rate indexes, the initial mcs */
	0x00, 0x21, 0, 0, 30, 101, 0, 16, 8, 1,	/* MCS 0 */
	0x01, 0x21, 0, 1, 20, 50, 0, 16, 9, 2,	/* MCS 1 */
	0x02, 0x21, 0, 2, 20, 50, 1, 17, 9, 3,	/* MCS 2 */
	0x03, 0x21, 0, 3, 15, 50, 2, 17, 10, 4,	/* MCS 3 */
	0x04, 0x21, 0, 4, 15, 30, 3, 18, 11, 5,	/* MCS 4 */
	0x05, 0x21, 0, 5, 10, 25, 4, 18, 12, 6,	/* MCS 5 */
	0x06, 0x21, 0, 6, 8, 14, 5, 19, 12, 7,	/* MCS 6 */
	0x07, 0x21, 0, 7, 8, 14, 6, 19, 12, 7,	/* MCS 7 */
	0x08, 0x20, 0, 8, 30, 50, 0, 16, 9, 2,	/* MCS 8 */
	0x09, 0x20, 0, 9, 20, 50, 8, 17, 10, 4,	/* MCS 9 */
	0x0A, 0x20, 0, 10, 20, 50, 9, 18, 11, 5,	/* MCS 10 */
	0x0B, 0x20, 0, 11, 15, 30, 10, 18, 12, 6,	/* MCS 11 */
	0x0C, 0x20, 0, 12, 15, 30, 11, 20, 13, 12,	/* MCS 12 */
	0x0D, 0x20, 0, 13, 8, 20, 12, 20, 14, 13,	/* MCS 13 */
	0x0E, 0x20, 0, 14, 8, 18, 13, 21, 15, 14,	/* MCS 14 */
	0x0F, 0x20, 0, 15, 8, 14, 14, 21, 15, 15,	/* MCS 15 */
	0x10, 0x20, 0, 16, 30, 50, 8, 17, 9, 3,	/* MCS 16 */
	0x11, 0x20, 0, 17, 20, 50, 16, 18, 11, 5,	/* MCS 17 */
	0x12, 0x20, 0, 18, 20, 50, 17, 19, 12, 7,	/* MCS 18 */
	0x13, 0x20, 0, 19, 15, 30, 18, 20, 13, 19,	/* MCS 19 */
	0x14, 0x20, 0, 20, 15, 30, 19, 21, 15, 20,	/* MCS 20 */
	0x15, 0x20, 0, 21, 8, 20, 20, 22, 21, 21,	/* MCS 21 */
	0x16, 0x20, 0, 22, 8, 20, 21, 23, 22, 22,	/* MCS 22 */
	0x17, 0x20, 0, 23, 6, 18, 22, 24, 23, 23,	/* MCS 23 */
	0x18, 0x22, 0, 23, 6, 14, 23, 24, 24, 24,	/* MCS 23 + Short GI */
};


#ifdef DOT11_VHT_AC

#define NSS_1 0
#define NSS_2 1
/* RSSI Offset table for Ags rate tuning */
UCHAR AgsRssiOffsetTable[3][4] = 
{
	// [i][] MAX System spatial stream capability: 1*1, 2*2, 3*3
	// [i][] MAX System Bandwidth: 20, 40, 80, 160
	// ----------------------
	{0,    2,    4,    6},    // For VHT 1*1: BW20, BW40, BW80, BW160
	{2,    4,    6,    8},    // For VHT 2*2: BW20, BW40, BW80, BW160
	{4,    6,    8,    10}    // For VHT 3*3: BW20, BW40, BW80, BW160             	
};


/*
	[Item no.] [Mode]* [Mode]** [CurrMCS] [TrainUp] [TrainDown] [downMCS] [upMCS3] [upMCS2] [upMCS1]

	Note: downMCS, upMCS3, upMCS2 and upMCS1 are zero-based array index.

	[Mode]*: 
		bit0: STBC -STBC_XXX
		bit1: Short GI - GI_XXX
		bit2~3: BW - BW_XXX
		bit4~bit6: Mode (0:CCK, 1:OFDM, 2:HT Mix, 3:HT GF, 4: VHT) - MODE_XXX
		bit7: Reserved

	[Mode]**
		bit0~1: Nss - NSS_XXX (VHT only)
		bit2~7: Reserved
*/

/* AGS: 1x1 VHT-capable rate table */
UCHAR Ags1x1VhtRateTable[] = 
{
	// Initial used item after association: the number of rate indexes, the initial MCS (index)
	9,	0x08,	0x00, 0,							0,	0,		0,	0,	0,	0,
	0,	0x41,	0x00, 0,/* VHT 1x1 MCS 0 */		30,	101,	0,	0,	0,	1, 
	1,	0x41,	0x00, 1,/* VHT 1x1 MCS 1 */		20,	50,		0,	0,	0,	2, 
	2,	0x41,	0x00, 2,/* VHT 1x1 MCS 2 */		20,	50,		1,	0,	0,	3, 
	3,	0x41,	0x00, 3,/* VHT 1x1 MCS 3 */		15,	50,		2,	0,	0,	4, 
	4,	0x41,	0x00, 4,/* VHT 1x1 MCS 4 */		15,	30,		3,	0,	0,	5, 
	5,	0x41,	0x00, 5,/* VHT 1x1 MCS 5 */		10,	25,		4,	0,	0,	6, 
	6,	0x41,	0x00, 6,/* VHT 1x1 MCS 6 */		8,	14,		5,	0,	0,	7, 
	7,	0x41,	0x00, 7,/* VHT 1x1 MCS 7 */		8,	14,		6,	0,	0,	8, 
	8,	0x43,	0x00, 7,/* VHT 1x1 MCS 7 + SGI */	8,	14,		7,	0,	0,	8, 
};


/* AGS: 2x2 VHT-capable rate table */
UCHAR Ags2x2VhtRateTable[] = {
	// row #1 is initial used item after association: the number of rate indexes, the initial MCS (index)
	17,	0x10,	0x00, 0,							0,	0,		0,	0,	0,	0,
	0,	0x41,	0x00, 0, /* VHT 1x1 MCS 0 */		30,	101,	0,	0,	8,	1, 
	1,	0x41,	0x00, 1, /* VHT 1x1 MCS 1 */		20,	50,		0,	0,	9,	2, 
	2,	0x41,	0x00, 2, /* VHT 1x1 MCS 2 */		20,	50,		1,	0,	9,	3, 
	3,	0x41,	0x00, 3, /* VHT 1x1 MCS 3 */		15,	50,		2,	0,	10,	4, 
	4,	0x41,	0x00, 4, /* VHT 1x1 MCS 4 */		15,	30,		3,	0,	11,	5, 
	5,	0x41,	0x00, 5, /* VHT 1x1 MCS 5 */		10,	25,		4,	0,	12,	6, 
	6,	0x41,	0x00, 6, /* VHT 1x1 MCS 6 */		8,	14,		5,	0,	12,	7, 
	7,	0x41,	0x00, 7, /* VHT 1x1 MCS 7 */		8,	14,		6,	0,	12,	7, 
	8,	0x40,	0x01, 0, /* VHT 2x2 MCS 0 */		30,	50,		0,	0,	9,	2, 
	9,	0x40,	0x01, 1, /* VHT 2x2 MCS 1 */		20,	50,		8,	0,	10,	4, 
	10,	0x40,	0x01, 2, /* VHT 2x2 MCS 2 */		20,	50,		9,	0,	11,	5, 
	11,	0x40,	0x01, 3, /* VHT 2x2 MCS 3 */		15,	30,		10,	0,	12,	6, 
	12,	0x40,	0x01, 4, /* VHT 2x2 MCS 4 */		15,	30,		11,	0,	13,	12, 
	13,	0x40,	0x01, 5, /* VHT 2x2 MCS 5 */		8,	20,		12,	0,	14,	13, 
	14,	0x40,	0x01, 6, /* VHT 2x2 MCS 6 */		8,	18,		13,	0,	15,	14, 
	15,	0x40,	0x01, 7, /* VHT 2x2 MCS 7 */		8,	25,		14,	0,	16,	15, 
	16,	0x42,	0x01, 7, /* VHT 2x2 MCS 7 + SGI */	8,	25,		15,	0,	16,	16, 
};
#endif /* DOT11_VHT_AC */


INT Show_AGS_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  RTMP_STRING *arg)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[1];
	UINT32 IdQuality;


	DBGPRINT(RT_DEBUG_OFF, ("MCS Group\t\tMCS Index\n"));
	DBGPRINT(RT_DEBUG_OFF, ("%d\t\t\t%d\n\n", pEntry->AGSCtrl.MCSGroup, pEntry->CurrTxRateIndex));

	DBGPRINT(RT_DEBUG_OFF, ("MCS Quality:\n"));
	for(IdQuality=0; IdQuality<=23; IdQuality++)
		DBGPRINT(RT_DEBUG_OFF, ("%02d\t\t%d\n", IdQuality, pEntry->TxQuality[IdQuality]));

	return TRUE;
}




#ifdef CONFIG_AP_SUPPORT
/*
	The dynamic Tx rate switching for AGS in VHT(Adaptive Group Switching)

	Parameters
		pAd: The adapter data structure

	Return Value:
	None
*/
VOID ApMlmeDynamicTxRateSwitchingAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx)
{
	UCHAR *pTable, TableSize = 0;
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[idx];
	UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
	RTMP_RA_AGS_TB *pCurrTxRate = NULL;
	RTMP_RA_LEGACY_TB *pNextTxRate = NULL;
	BOOLEAN bTxRateChanged = TRUE, bUpgradeQuality = FALSE;
	UCHAR TrainUp = 0, TrainDown = 0, next_grp;
	CHAR RssiOffset = 0;
	ULONG TxTotalCnt, TxErrorRatio = 0;
	ULONG TxSuccess, TxRetransmit, TxFailCount;
	AGS_STATISTICS_INFO AGSStatisticsInfo = {0};


	DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: ---> %s\n", __FUNCTION__));
	
	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC	StaTx1;
		TX_STA_CNT0_STRUC	TxStaCnt0;

		/*  Update statistic counter */
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

		TxRetransmit = StaTx1.field.TxRetransmit;
		TxSuccess = StaTx1.field.TxSuccess;
		TxFailCount = TxStaCnt0.field.TxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)) {
			if (pEntry->wcid >= 1 && pEntry->wcid <= 8)
			{
				ULONG 	HwTxCnt, HwErrRatio;

				NicGetMacFifoTxCnt(pAd, pEntry);
				HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
				if (HwTxCnt)
					HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
				else
					HwErrRatio = 0;
				DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("%s()=>Wcid:%d, MCS:%d, TxErrRatio(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
						__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS, 
						HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

				TxSuccess = pEntry->fifoTxSucCnt;
				TxRetransmit = pEntry->fifoTxRtyCnt;
				TxTotalCnt = HwTxCnt;
				TxErrorRatio = HwErrRatio;
			}
		}
#endif /*  FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}

	AGSStatisticsInfo.RSSI = RTMPAvgRssi(pAd, &pEntry->RssiSample);;
	AGSStatisticsInfo.TxErrorRatio = TxErrorRatio;
	AGSStatisticsInfo.AccuTxTotalCnt = TxTotalCnt;
	AGSStatisticsInfo.TxTotalCnt = TxTotalCnt;
	AGSStatisticsInfo.TxSuccess = TxSuccess;
	AGSStatisticsInfo.TxRetransmit = TxRetransmit;
	AGSStatisticsInfo.TxFailCount = TxFailCount;

	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: AccuTxTotalCnt = %ld, TxSuccess = %ld, TxRetransmit = %ld, TxFailCount = %ld, TxErrorRatio = %ld\n",
		__FUNCTION__, 
		AGSStatisticsInfo.AccuTxTotalCnt, 
		AGSStatisticsInfo.TxSuccess, 
		AGSStatisticsInfo.TxRetransmit, 
		AGSStatisticsInfo.TxFailCount, 
		AGSStatisticsInfo.TxErrorRatio));

	pTable = pEntry->pTable;
	TableSize = pTable[0];
	
	CurrRateIdx = pEntry->CurrTxRateIndex;	

	if (CurrRateIdx >= TableSize)
		CurrRateIdx = TableSize - 1;


	pCurrTxRate = (RTMP_RA_AGS_TB *)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

	/* Select the next upgrade rate and the next downgrade rate, if any */
	do 
	{
		if (pTable == AGS3x3HTRateTable)
		{
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  "
				"TxQuality1[%d] = %d, TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, "
				"pCurrTxRate->ItemNo = %d\n",
				__FUNCTION__, 
				pEntry->AGSCtrl.MCSGroup, 
				pCurrTxRate->upMcs3, 
				pEntry->TxQuality[pCurrTxRate->upMcs3], 
				pCurrTxRate->upMcs2, 
				pEntry->TxQuality[pCurrTxRate->upMcs2], 
				pCurrTxRate->upMcs1, 
				pEntry->TxQuality[pCurrTxRate->upMcs1], 
				pCurrTxRate->upMcs1, 
				pCurrTxRate->ItemNo));

			/* 3x3 */
			/* for 3*3, pEntry->AGSCtrl.MCSGroup = 0, 3, 3, 3, ... */
			next_grp = pEntry->AGSCtrl.MCSGroup;
			switch (pEntry->AGSCtrl.MCSGroup)
			{
				case 0: /* MCS selection in round robin policy (different MCS group) */
				{
					UpRateIdx = pCurrTxRate->upMcs3;
					next_grp = 3;

					/* MCS group #2 has better Tx quality */
					if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && 
					     (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo))
					{
						UpRateIdx = pCurrTxRate->upMcs2;
						next_grp = 2;
					}

					/* MCS group #1 has better Tx quality */
					if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
					     (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
					{
						UpRateIdx = pCurrTxRate->upMcs1;
						next_grp = 1;
					}
				}
				break;
				
				case 3:
					UpRateIdx = pCurrTxRate->upMcs3;
				break;
				
				case 2:
					UpRateIdx = pCurrTxRate->upMcs2;
				break;
				
				case 1:
					UpRateIdx = pCurrTxRate->upMcs1;
				break;
				
				default:
				{
					DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s: AGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n",
						__FUNCTION__, pEntry->AGSCtrl.MCSGroup));
				}
				break;
			}			
			
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
							("AGS> Current Group:%d, Select UpRateIdx=%d in group %d\n",
							pEntry->AGSCtrl.MCSGroup, UpRateIdx, next_grp));


			if ((pEntry->AGSCtrl.MCSGroup == 0) && 
			     (((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs2]) && (pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)) || 
			     ((pEntry->TxQuality[pCurrTxRate->upMcs3] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))))
			{
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: ##############\n", __FUNCTION__));

				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("%s: AGS: [3x3 Before - "
					"pEntry->AGSCtrl.MCSGroup = %d, TxQuality2[%d] = %d,  "
					"TxQuality1[%d] = %d, TxQuality0[%d] = %d\n",
					__FUNCTION__, 
					pEntry->AGSCtrl.MCSGroup, 
					pCurrTxRate->upMcs3, 
					pEntry->TxQuality[pCurrTxRate->upMcs3], 
					pCurrTxRate->upMcs2, 
					pEntry->TxQuality[pCurrTxRate->upMcs2], 
					pCurrTxRate->upMcs1, 
					pEntry->TxQuality[pCurrTxRate->upMcs1]));
			}
		}
		else if ((pTable == AGS2x2HTRateTable)
#ifdef DOT11_VHT_AC
				|| (pTable == Ags2x2VhtRateTable)
#endif /* DOT11_VHT_AC*/
			)
		{
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("%s: AGS: pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, "
				"TxQuality0[%d] = %d, pCurrTxRate->upMcs1 = %d, "
				"pCurrTxRate->ItemNo = %d\n",
				__FUNCTION__, 
				pEntry->AGSCtrl.MCSGroup, 
				pCurrTxRate->upMcs2, pEntry->TxQuality[pCurrTxRate->upMcs2], 
				pCurrTxRate->upMcs1, pEntry->TxQuality[pCurrTxRate->upMcs1], 
				pCurrTxRate->upMcs1, pCurrTxRate->ItemNo));
			
			/* 2x2 peer device (Adhoc, DLS or AP) */
			switch (pEntry->AGSCtrl.MCSGroup)
			{
				case 0: /* MCS selection in round robin policy */
				{
					UpRateIdx = pCurrTxRate->upMcs2;
						/* MCS group #1 has better Tx quality */
					if ((pEntry->TxQuality[UpRateIdx] > pEntry->TxQuality[pCurrTxRate->upMcs1]) && 
					     (pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo))
						UpRateIdx = pCurrTxRate->upMcs1;
				}
				break;
				
				case 2:
					UpRateIdx = pCurrTxRate->upMcs2;
				break;
				
				case 1:
					UpRateIdx = pCurrTxRate->upMcs1;
				break;
				
				default:
				{
					DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], "
						"Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
						__FUNCTION__, 
						pEntry->AGSCtrl.MCSGroup));
				}
				break;
			}	

			if ((pEntry->AGSCtrl.MCSGroup == 0) && 
			     ((pEntry->TxQuality[pCurrTxRate->upMcs2] > pEntry->TxQuality[pCurrTxRate->upMcs1]) &&
					(pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)))
			{
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: ###########\n", __FUNCTION__));

				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("%s: AGS: [2x2 peer device (Adhoc, DLS or AP)], Before - "
					"pEntry->AGSCtrl.MCSGroup = %d, TxQuality1[%d] = %d, "
					"TxQuality0[%d] = %d\n",
					__FUNCTION__, 
					pEntry->AGSCtrl.MCSGroup, 
					pCurrTxRate->upMcs2, 
					pEntry->TxQuality[pCurrTxRate->upMcs2], 
					pCurrTxRate->upMcs1, 
					pEntry->TxQuality[pCurrTxRate->upMcs1]));
			}
		} 
		else 
		{
			/* 1x1 peer device (Adhoc, DLS or AP) */
			switch (pEntry->AGSCtrl.MCSGroup)
			{
				case 1:
				case 0:
					UpRateIdx = pCurrTxRate->upMcs1;
				break;
				
				default:
				{
					DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s: AGS: [1x1 peer device (Adhoc, DLS or AP)], "
						"Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
						__FUNCTION__, 
						pEntry->AGSCtrl.MCSGroup));
				}
				break;
			}	
		}


		/* The STA uses the best Tx rate at this moment. */
		if (UpRateIdx == pEntry->CurrTxRateIndex)
		{
			/* current rate is the best one */
			pEntry->AGSCtrl.MCSGroup = 0; /* Try to escape the local optima */

			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> Current rate is the best one!\n"));
			break;
		}
		
		if ((pEntry->TxQuality[UpRateIdx] > 0) && (pEntry->AGSCtrl.MCSGroup > 0))
		{
			/*
				Quality of up rate is bad try to use lower group.
				So continue to get the up rate index.
			*/
			pEntry->AGSCtrl.MCSGroup--; /* Try to use the MCS of the lower MCS group */
		}
		else
		{
			break;
		}
	} while (1);

	DownRateIdx = pCurrTxRate->downMcs;

	DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("ags> UpRateIdx = %d, DownRateIdx = %d\n",
				UpRateIdx, DownRateIdx));

#ifdef DOT11_N_SUPPORT
	if ((AGSStatisticsInfo.RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
	{
		TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		TrainUp = pCurrTxRate->TrainUp;
		TrainDown = pCurrTxRate->TrainDown;
	}

	/* Keep the TxRateChangeAction status */
	pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;


	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: Rssi = %d, TxSuccess = %lu, TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n", 
		__FUNCTION__, 
		AGSStatisticsInfo.RSSI, 
		AGSStatisticsInfo.TxSuccess, 
		AGSStatisticsInfo.TxRetransmit, 
		AGSStatisticsInfo.TxFailCount, 
		AGSStatisticsInfo.TxErrorRatio));

	DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, 
		("%s: AGS: Before - CurrTxRateIdx = %d, MCS = %d, STBC = %d, ShortGI = %d, Mode = %d, "
		"TrainUp = %d, TrainDown = %d, NextUp = %d, NextDown = %d, "
		"CurrMCS = %d, pEntry->AGSCtrl.MCSGroup = %d, PER = %lu%%, Retry = %lu, NoRetry = %lu\n", 
		__FUNCTION__, 
		CurrRateIdx, 
		pCurrTxRate->CurrMCS, 
		pCurrTxRate->STBC, 
		pCurrTxRate->ShortGI, 
		pCurrTxRate->Mode, 
		TrainUp, 
		TrainDown, 
		UpRateIdx, 
		DownRateIdx, 
		pEntry->HTPhyMode.field.MCS, 
		pEntry->AGSCtrl.MCSGroup, 
		AGSStatisticsInfo.TxErrorRatio, 
		AGSStatisticsInfo.TxRetransmit, 
		AGSStatisticsInfo.TxSuccess));

	/* MCS selection based on the RSSI information when the Tx samples are fewer than 15. */
	if (AGSStatisticsInfo.AccuTxTotalCnt <= 15)
	{
		CHAR idx = 0;
		UCHAR TxRateIdx;
		UCHAR MCS[24] = {0};
		/* Check the existence and index of each needed MCS */

#ifdef DOT11_VHT_AC
		if ((pTable == Ags2x2VhtRateTable) || (pTable == Ags1x1VhtRateTable))
		{
			INT mcs_idx_offset;
			while (idx < pTable[0])
			{
				pCurrTxRate = (RTMP_RA_AGS_TB *)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
				if (pCurrTxRate->Mode == MODE_VHT)
				{
					if (pCurrTxRate->Nss == NSS_1)
						mcs_idx_offset = 0;
					else if ((pCurrTxRate->Nss == NSS_2) && (pTable == Ags2x2VhtRateTable))
						mcs_idx_offset = 8;
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid Nss(%d)\n", __FUNCTION__, pCurrTxRate->Nss));
						mcs_idx_offset = 0;
					}

					if ((pCurrTxRate->CurrMCS <= MCS_7) /* && (pCurrTxRate->CurrMCS >= MCS_0) */)
						MCS[pCurrTxRate->CurrMCS + mcs_idx_offset] = idx;
				}
				idx++;
			}
		}
		else
#endif /* DOT11_VHT_AC */
		{
			while (idx < pTable[0])
			{
				pCurrTxRate = (RTMP_RA_AGS_TB *)(&pTable[(idx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

				if (pCurrTxRate->CurrMCS <= MCS_23 /* && pCurrTxRate->CurrMCS >= MCS_0 */)
					MCS[pCurrTxRate->CurrMCS] = idx;				
				idx++;
			}
		}

		/* peer device (Adhoc, DLS or AP) */
		RssiOffset = 0;
		if (pTable == AGS3x3HTRateTable)
		{
			if (MCS[23] && (AGSStatisticsInfo.RSSI > (-67 + RssiOffset)))
				TxRateIdx = MCS[23];
			else if (MCS[22] && (AGSStatisticsInfo.RSSI > (-69 + RssiOffset)))
				TxRateIdx = MCS[22];
			else if (MCS[21] && (AGSStatisticsInfo.RSSI > (-72 + RssiOffset)))
				TxRateIdx = MCS[21];
			else if (MCS[20] && (AGSStatisticsInfo.RSSI > (-74 + RssiOffset)))
				TxRateIdx = MCS[20];
			else if (MCS[19] && (AGSStatisticsInfo.RSSI > (-78 + RssiOffset)))
				TxRateIdx = MCS[19];
			else if (MCS[18] && (AGSStatisticsInfo.RSSI > (-80 + RssiOffset)))
				TxRateIdx = MCS[18];
			else if (MCS[17] && (AGSStatisticsInfo.RSSI > (-85 + RssiOffset)))
				TxRateIdx = MCS[17];
			else
				TxRateIdx = MCS[16];

			pEntry->AGSCtrl.MCSGroup = 3;

			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("ags> Group3 RSSI = %d, TxRateIdx = %d\n",
					AGSStatisticsInfo.RSSI, TxRateIdx));
		}
		else if ((pTable == AGS2x2HTRateTable)
#ifdef DOT11_VHT_AC
				|| (pTable == Ags2x2VhtRateTable)
#endif /* DOT11_VHT_AC */
		)
		{
#ifdef DOT11_VHT_AC
			if (pTable == Ags2x2VhtRateTable)
				RssiOffset = AgsRssiOffsetTable[1][pAd->CommonCfg.BBPCurrentBW];
#endif /* DOT11_VHT_AC */

			DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: AGS: 2*2, RssiOffsetForAgs=%d\n", __FUNCTION__, RssiOffset));
			
			if (MCS[15] && (AGSStatisticsInfo.RSSI > (-69 + RssiOffset)))
				TxRateIdx = MCS[15];
			else if (MCS[14] && (AGSStatisticsInfo.RSSI > (-71 + RssiOffset)))
				TxRateIdx = MCS[14];
			else if (MCS[13] && (AGSStatisticsInfo.RSSI > (-74 + RssiOffset)))
				TxRateIdx = MCS[13];
			else if (MCS[12] && (AGSStatisticsInfo.RSSI > (-76 + RssiOffset)))
				TxRateIdx = MCS[12];
			else if (MCS[11] && (AGSStatisticsInfo.RSSI > (-80 + RssiOffset)))
				TxRateIdx = MCS[11];
			else if (MCS[10] && (AGSStatisticsInfo.RSSI > (-82 + RssiOffset)))
				TxRateIdx = MCS[10];
			else if (MCS[9] && (AGSStatisticsInfo.RSSI > (-87 + RssiOffset)))
				TxRateIdx = MCS[9];
			else
				TxRateIdx = MCS[8];
			
			pEntry->AGSCtrl.MCSGroup = 2;
		} 
		else
		{
#ifdef DOT11_VHT_AC
			if (pTable == Ags1x1VhtRateTable)
				RssiOffset = AgsRssiOffsetTable[0][pAd->CommonCfg.BBPCurrentBW];
#endif /* DOT11_VHT_AC */
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: 1*1, RssiOffset=%d\n", __FUNCTION__, RssiOffset));
			
			/* 1x1 peer device (Adhoc, DLS or AP) */
			if (MCS[7] && (AGSStatisticsInfo.RSSI > (-71 + RssiOffset)))
				TxRateIdx = MCS[7];
			else if (MCS[6] && (AGSStatisticsInfo.RSSI > (-73 + RssiOffset)))
				TxRateIdx = MCS[6];
			else if (MCS[5] && (AGSStatisticsInfo.RSSI > (-76 + RssiOffset)))
				TxRateIdx = MCS[5];
			else if (MCS[4] && (AGSStatisticsInfo.RSSI > (-78 + RssiOffset)))
				TxRateIdx = MCS[4];
			else if (MCS[3] && (AGSStatisticsInfo.RSSI > (-82 + RssiOffset)))
				TxRateIdx = MCS[3];
			else if (MCS[2] && (AGSStatisticsInfo.RSSI > (-84 + RssiOffset)))
				TxRateIdx = MCS[2];
			else if (MCS[1] && (AGSStatisticsInfo.RSSI > (-89 + RssiOffset)))
				TxRateIdx = MCS[1];
			else
				TxRateIdx = MCS[0];
			
			pEntry->AGSCtrl.MCSGroup = 1;
		}

		pEntry->AGSCtrl.lastRateIdx = pEntry->CurrTxRateIndex;
		pEntry->CurrTxRateIndex = TxRateIdx;

		pNextTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);

		RTMPZeroMemory(pEntry->TxQuality, (sizeof(USHORT) * (MAX_TX_RATE_INDEX+1)));
		RTMPZeroMemory(pEntry->PER, (sizeof(UCHAR) * (MAX_TX_RATE_INDEX+1)));
		
		pEntry->fLastSecAccordingRSSI = TRUE;			
		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);
		return;
	}

	/* The MCS selection is based on the RSSI and skips the rate tuning this time. */
	if (pEntry->fLastSecAccordingRSSI == TRUE)
	{
		pEntry->fLastSecAccordingRSSI = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		
		RESET_ONE_SEC_TX_CNT(pEntry);
		
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: The MCS selection base on RSSI\n", __FUNCTION__));

		return;
	}

	DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: TrainUp:%d, TrainDown:%d\n", __FUNCTION__, TrainUp, TrainDown));

	do
	{
		BOOLEAN	bTrainUpDown = FALSE;
		
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("%s: AGS: TxQuality[CurrRateIdx(%d)] = %d, UpPenalty:%d\n",
					__FUNCTION__, CurrRateIdx,
					pEntry->TxQuality[CurrRateIdx], pEntry->TxRateUpPenalty));
			
		if (AGSStatisticsInfo.TxErrorRatio >= TrainDown) /* Poor quality */
		{
			/* error ratio too high, do rate down */
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
						("%s: AGS: (DOWN) TxErrorRatio >= TrainDown\n",__FUNCTION__));
			bTrainUpDown = TRUE;
			pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
		}
		else if (AGSStatisticsInfo.TxErrorRatio <= TrainUp) /* Good quality */
		{
			bTrainUpDown = TRUE;
			bUpgradeQuality = TRUE;
			
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("%s: AGS: (UP) pEntry->TxQuality[CurrRateIdx] = %d, pEntry->TxRateUpPenalty = %d\n",
				__FUNCTION__, 
				pEntry->TxQuality[CurrRateIdx], 
				pEntry->TxRateUpPenalty));
			
			if (pEntry->TxQuality[CurrRateIdx])
				pEntry->TxQuality[CurrRateIdx]--;	/* Good quality in the current Tx rate */

			if (pEntry->TxRateUpPenalty)
				pEntry->TxRateUpPenalty--;	/* no use for the parameter */
			else
			{
				if (pEntry->TxQuality[pCurrTxRate->upMcs3] && (pCurrTxRate->upMcs3 != CurrRateIdx))
					pEntry->TxQuality[pCurrTxRate->upMcs3]--;
				
				if (pEntry->TxQuality[pCurrTxRate->upMcs2] && (pCurrTxRate->upMcs2 != CurrRateIdx))
					pEntry->TxQuality[pCurrTxRate->upMcs2]--;
				
				if (pEntry->TxQuality[pCurrTxRate->upMcs1] && (pCurrTxRate->upMcs1 != CurrRateIdx))
					pEntry->TxQuality[pCurrTxRate->upMcs1]--;
			}
		}
		else if (pEntry->AGSCtrl.MCSGroup > 0) /* even if TxErrorRatio > TrainUp */
		{
			/* not bad and not good */
			if (UpRateIdx != 0)
			{
				bTrainUpDown = FALSE;
				
				/* Good quality in the current Tx rate */
				if (pEntry->TxQuality[CurrRateIdx])
					pEntry->TxQuality[CurrRateIdx]--;

				/* It may improve next train-up Tx rate's quality */
				 if (pEntry->TxQuality[UpRateIdx])
					pEntry->TxQuality[UpRateIdx]--;

				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> not bad and not good\n"));
			}
		}

		/* update error ratio for current MCS */
		pEntry->PER[CurrRateIdx] = (UCHAR)(AGSStatisticsInfo.TxErrorRatio);

		/* Update the current Tx rate */
		if (bTrainUpDown)
		{
			/* need to rate up or down */
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("%s: AGS: bTrainUpDown = %d, CurrRateIdx = %d, DownRateIdx = %d, UpRateIdx = %d, pEntry->TxQuality[CurrRateIdx] = %d, pEntry->TxQuality[UpRateIdx] = %d\n",
				__FUNCTION__, 
				bTrainUpDown, CurrRateIdx, DownRateIdx, UpRateIdx, 
				pEntry->TxQuality[CurrRateIdx], pEntry->TxQuality[UpRateIdx]));
			
			/* Downgrade Tx rate */
			if ((CurrRateIdx != DownRateIdx) && 
			     (pEntry->TxQuality[CurrRateIdx] >= AGS_TX_QUALITY_WORST_BOUND))
			{
				pEntry->CurrTxRateIndex = DownRateIdx;
				pEntry->LastSecTxRateChangeAction = RATE_DOWN;
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> rate down!\n"));
			}
			else if ((CurrRateIdx != UpRateIdx) && 
					(pEntry->TxQuality[UpRateIdx] <= 0)) /* Upgrade Tx rate */
			{
				pEntry->CurrTxRateIndex = UpRateIdx;
				pEntry->LastSecTxRateChangeAction = RATE_UP;
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> rate up!\n"));
			}
		}
	} while (FALSE);

	DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: pEntry->CurrTxRateIndex = %d, CurrRateIdx = %d, pEntry->LastSecTxRateChangeAction = %d\n", 
		__FUNCTION__, 
		pEntry->CurrTxRateIndex, CurrRateIdx, 
		pEntry->LastSecTxRateChangeAction));
	
	/* rate up/down post handle */
	if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
	     (pEntry->LastSecTxRateChangeAction == RATE_UP))
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: ++TX rate from %d to %d\n", 
			__FUNCTION__, CurrRateIdx, pEntry->CurrTxRateIndex));
		
		pEntry->LastSecTxRateChangeAction = RATE_UP;
		pEntry->TxRateUpPenalty = 0;
		RTMPZeroMemory(pEntry->PER, sizeof(UCHAR) * (MAX_TX_RATE_INDEX+1));
		pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

		bTxRateChanged = TRUE;
	}
	else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
	             (pEntry->LastSecTxRateChangeAction == RATE_DOWN))
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: AGS: --TX rate from %d to %d\n", 
			__FUNCTION__, CurrRateIdx, pEntry->CurrTxRateIndex));
		
		pEntry->LastSecTxRateChangeAction = RATE_DOWN;
		pEntry->TxRateUpPenalty = 0; /* No penalty */
		pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
		pEntry->PER[pEntry->CurrTxRateIndex] = 0;
		pEntry->AGSCtrl.lastRateIdx = CurrRateIdx;

		bTxRateChanged = TRUE;
	}
	else /* Tx rate remains unchanged. */
	{
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE; /* Tx rate remains unchanged. */
		bTxRateChanged = FALSE;
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> no rate up/down!\n"));
	}

	/* Tx rate fast train up/down */
	if ((bTxRateChanged == TRUE) &&
		(!pAd->ApCfg.ApQuickResponeForRateUpTimerRunning))
	{
		RTMPSetTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, pAd->ra_fast_interval);
		pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
	}
	pEntry->LastTxOkCount = AGSStatisticsInfo.TxSuccess;

	/* set new tx rate */
	pNextTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

	if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("ags> set new rate MCS = %d!\n", pEntry->CurrTxRateIndex));
		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}
	
	/* reset all OneSecTx counters */
	RESET_ONE_SEC_TX_CNT(pEntry);
	
    DBGPRINT_RAW(RT_DEBUG_TRACE, ("AGS: <--- %s\n", __FUNCTION__));
}


/*
	Auto Tx rate faster train up/down for AGS (Adaptive Group Switching)

	Parameters
		pAd: The adapter data structure
		pEntry: Pointer to a caller-supplied variable in which points to a MAC table entry
		pTable: Pointer to a caller-supplied variable in wich points to a Tx rate switching table
		TableSize: The size, in bytes, of the specified Tx rate switching table
		pAGSStatisticsInfo: Pointer to a caller-supplied variable in which points to the statistics information

	Return Value:
		None
*/
VOID ApQuickResponeForRateUpExecAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx)
{
	UCHAR *pTable, TableSize = 0;
	UCHAR UpRateIdx = 0, DownRateIdx = 0, CurrRateIdx = 0;
	RTMP_RA_AGS_TB *pCurrTxRate = NULL;
	RTMP_RA_LEGACY_TB *pNextTxRate = NULL;
	BOOLEAN bTxRateChanged = TRUE;
	UCHAR TrainUp = 0, TrainDown = 0;
	CHAR ratio = 0;
	ULONG OneSecTxNoRetryOKRationCount = 0;
	MAC_TABLE_ENTRY *pEntry;
	AGS_STATISTICS_INFO AGSStatisticsInfo = {0};
	ULONG TxTotalCnt, TxErrorRatio = 0;
	ULONG TxSuccess, TxRetransmit, TxFailCount;


	DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: ---> %s\n", __FUNCTION__));
	pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = FALSE;
	
	pEntry = &pAd->MacTab.Content[idx]; /* point to information of the individual station */
	pTable = pEntry->pTable;
	TableSize = pTable[0];

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC StaTx1;
		TX_STA_CNT0_STRUC TxStaCnt0;

		/*  Update statistic counter */
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

		TxRetransmit = StaTx1.field.TxRetransmit;
		TxSuccess = StaTx1.field.TxSuccess;
		TxFailCount = TxStaCnt0.field.TxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#if defined(RTMP_MAC) || defined(RLT_MAC)
#ifdef FIFO_EXT_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT)) {
			if (pEntry->wcid >= 1 && pEntry->wcid <= 8)
			{
				ULONG 	HwTxCnt, HwErrRatio;

				NicGetMacFifoTxCnt(pAd, pEntry);
				HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
				if (HwTxCnt)
					HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
				else
					HwErrRatio = 0;
				DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("%s()=>Wcid:%d, MCS:%d, TxErrRatio(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n", 
						__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS, 
						HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

				TxSuccess = pEntry->fifoTxSucCnt;
				TxRetransmit = pEntry->fifoTxRtyCnt;
				TxTotalCnt = HwTxCnt;
				TxErrorRatio = HwErrRatio;
			}
		}
#endif /*  FIFO_EXT_SUPPORT */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}

	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,
		("%s: QuickAGS: AccuTxTotalCnt = %lu, TxSuccess = %lu, "
		"TxRetransmit = %lu, TxFailCount = %lu, TxErrorRatio = %lu\n",
		__FUNCTION__, 
		AGSStatisticsInfo.AccuTxTotalCnt, 
		AGSStatisticsInfo.TxSuccess, 
		AGSStatisticsInfo.TxRetransmit, 
		AGSStatisticsInfo.TxFailCount, 
		AGSStatisticsInfo.TxErrorRatio));
	
	CurrRateIdx = pEntry->CurrTxRateIndex;	

	if (CurrRateIdx >= TableSize)
		CurrRateIdx = TableSize - 1;

	UpRateIdx = DownRateIdx = pEntry->AGSCtrl.lastRateIdx;

	pCurrTxRate = (RTMP_RA_AGS_TB *)(&pTable[(CurrRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);

	if ((AGSStatisticsInfo.RSSI > -65) && (pCurrTxRate->Mode >= MODE_HTMIX))
	{
		TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
	{
		TrainUp = pCurrTxRate->TrainUp;
		TrainDown = pCurrTxRate->TrainDown;
	}

	/* MCS selection based on the RSSI information when the Tx samples are fewer than 15. */
	if (AGSStatisticsInfo.AccuTxTotalCnt <= 15)
	{
		RTMPZeroMemory(pEntry->TxQuality, sizeof(USHORT) * (MAX_TX_RATE_INDEX+1));
		RTMPZeroMemory(pEntry->PER, sizeof(UCHAR) * (MAX_TX_RATE_INDEX+1));

		if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
		{
			pEntry->CurrTxRateIndex = DownRateIdx;
			pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;
		}
		else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx))
		{
			pEntry->CurrTxRateIndex = UpRateIdx;
		}

		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
			("%s: QuickAGS: AccuTxTotalCnt <= 15, train back to original rate\n", 
			__FUNCTION__));
		
		return;
	}

	do
	{
		if (pEntry->LastTimeTxRateChangeAction == 0)
			ratio = 5;
		else
			ratio = 4;

		if (AGSStatisticsInfo.TxErrorRatio >= TrainDown) /* Poor quality */
			pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

		pEntry->PER[CurrRateIdx] = (UCHAR)(AGSStatisticsInfo.TxErrorRatio);

		OneSecTxNoRetryOKRationCount = (AGSStatisticsInfo.TxSuccess * ratio);
		
		/* Tx rate down */
		if ((pEntry->LastSecTxRateChangeAction == 1) && (CurrRateIdx != DownRateIdx))
		{
			// Change Auto Rate tuning rule 1: Change from tx_ok to PER
			if (AGSStatisticsInfo.TxErrorRatio > TrainDown)  /* Poor quality */
			{
				pEntry->CurrTxRateIndex = DownRateIdx;
				pEntry->TxQuality[CurrRateIdx] = AGS_TX_QUALITY_WORST_BOUND;

				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("%s: QuickAGS: (UP) bad Tx ok count (Current PER:%ld, NewMcs's TrainDown:%d)\n",
					__FUNCTION__, AGSStatisticsInfo.TxErrorRatio, TrainDown));
			}
			else /* Good quality */
			{
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
					("%s: QuickAGS: (UP) keep rate-up (Current PER:%ld, NewMcs's TrainDown:%d)\n", 
					__FUNCTION__, AGSStatisticsInfo.TxErrorRatio, TrainDown));

				RTMPZeroMemory(pEntry->TxQuality, sizeof(USHORT) * (MAX_TX_RATE_INDEX+1));

				if (pEntry->AGSCtrl.MCSGroup == 0)
				{
					if (pTable == AGS3x3HTRateTable)
						pEntry->AGSCtrl.MCSGroup = 3;
					else if ((pTable == AGS2x2HTRateTable) || 
					            (pTable == Ags2x2VhtRateTable))
						pEntry->AGSCtrl.MCSGroup = 2;
					else
						pEntry->AGSCtrl.MCSGroup = 1;
				}
			}
		}
		else if ((pEntry->LastSecTxRateChangeAction == 2) && (CurrRateIdx != UpRateIdx)) // Tx rate up
		{

// Don't quick check within train down case
		}
	}while (FALSE);

	DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA,
				("ags> new group = %d\n", pEntry->AGSCtrl.MCSGroup));

	/* Last action is rate-up */
	if (pEntry->LastSecTxRateChangeAction == 1) 
	{
		/* looking for the next group with valid MCS */
		if ((pEntry->CurrTxRateIndex != CurrRateIdx) && (pEntry->AGSCtrl.MCSGroup > 0))
		{
			pEntry->AGSCtrl.MCSGroup--; /* Try to use the MCS of the lower MCS group */
			pCurrTxRate = (RTMP_RA_AGS_TB *)(&pTable[(DownRateIdx + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
		}
		
		/* UpRateIdx is for temp use in this section */
		switch (pEntry->AGSCtrl.MCSGroup)
		{
			case 3: 
				UpRateIdx = pCurrTxRate->upMcs3;
				break;
			
			case 2: 
				UpRateIdx = pCurrTxRate->upMcs2;
				break;
			
			case 1: 
				UpRateIdx = pCurrTxRate->upMcs1;
				break;
			
			case 0: 
				UpRateIdx = CurrRateIdx;
				break;
			
			default: 
			{
				DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: Incorrect MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
					__FUNCTION__, 
					pEntry->AGSCtrl.MCSGroup));
			}
			break;
		}

		/* Try to escape the local optima */
		if (UpRateIdx == pEntry->CurrTxRateIndex)
			pEntry->AGSCtrl.MCSGroup = 0;
		
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: next MCS group, pEntry->AGSCtrl.MCSGroup = %d\n", 
			__FUNCTION__, pEntry->AGSCtrl.MCSGroup));
	}

	if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
	     (pEntry->LastSecTxRateChangeAction == 2)) /* Tx rate up */
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: ++TX rate from %d to %d\n", 
			__FUNCTION__, CurrRateIdx, pEntry->CurrTxRateIndex));	
		
		pEntry->TxRateUpPenalty = 0;
		pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0; /*restore the TxQuality from max to 0 */
		RTMPZeroMemory(pEntry->PER, sizeof(UCHAR) * (MAX_TX_RATE_INDEX+1));
	}
	else if ((pEntry->CurrTxRateIndex != CurrRateIdx) && 
	            (pEntry->LastSecTxRateChangeAction == 1)) /* Tx rate down */
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: --TX rate from %d to %d\n", 
			__FUNCTION__, CurrRateIdx, pEntry->CurrTxRateIndex));
		
		pEntry->TxRateUpPenalty = 0; /* No penalty */
		pEntry->TxQuality[pEntry->CurrTxRateIndex] = 0;
		pEntry->PER[pEntry->CurrTxRateIndex] = 0;
	}
	else
	{
		bTxRateChanged = FALSE;
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: QuickAGS: rate is not changed\n", __FUNCTION__));
	}

	pNextTxRate = (RTMP_RA_LEGACY_TB *)(&pTable[(pEntry->CurrTxRateIndex + 1) * SIZE_OF_AGS_RATE_TABLE_ENTRY]);
	if ((bTxRateChanged == TRUE) && (pNextTxRate != NULL))
	{
		DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("ags> confirm current rate MCS = %d!\n", pEntry->CurrTxRateIndex));

		APMlmeSetTxRate(pAd, pEntry, pNextTxRate);
	}

	/* reset all OneSecTx counters */
	RESET_ONE_SEC_TX_CNT(pEntry);

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("QuickAGS: <--- %s\n", __FUNCTION__));
}
#endif /* CONFIG_AP_SUPPORT */

#endif	/* AGS_SUPPORT */

