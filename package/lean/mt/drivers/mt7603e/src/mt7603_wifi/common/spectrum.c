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
	action.c
 
    Abstract:
    Handle association related requests either from WSTA or from local MLME
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
	Fonchi Wu    2008	  	   created for 802.11h
 */

#include "rt_config.h"
#include "action.h"


/* The regulatory information in the USA (US) */
DOT11_REGULATORY_INFO USARegulatoryInfo[] = 
{
/*  "regulatory class"  "number of channels"  "Max Tx Pwr"  "channel list" */
    {0,	                {0,                   0,           {0}}}, /* Invlid entry*/
    {1,                 {4,                   16,           {36, 40, 44, 48}}}, 
    {2,                 {4,                   23,           {52, 56, 60, 64}}}, 
    {3,                 {4,                   29,           {149, 153, 157, 161}}}, 
    {4,                 {11,                  23,           {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140}}}, 
    {5,                 {5,                   30,           {149, 153, 157, 161, 165}}}, 
    {6,                 {10,                  14,           {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}}}, 
    {7,                 {10,                  27,           {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}}}, 
    {8,                 {5,                   17,           {11, 13, 15, 17, 19}}}, 
    {9,                 {5,                   30,           {11, 13, 15, 17, 19}}}, 
    {10,                {2,                   20,           {21, 25}}}, 
    {11,                {2,                   33,            {21, 25}}}, 
    {12,                {11,                  30,            {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}}}
};
#define USA_REGULATORY_INFO_SIZE (sizeof(USARegulatoryInfo) / sizeof(DOT11_REGULATORY_INFO))


/* The regulatory information in Europe */
DOT11_REGULATORY_INFO EuropeRegulatoryInfo[] = 
{
/*  "regulatory class"  "number of channels"  "Max Tx Pwr"  "channel list" */
    {0,                 {0,                   0,           {0}}}, /* Invalid entry*/
    {1,                 {4,                   20,           {36, 40, 44, 48}}}, 
    {2,                 {4,                   20,           {52, 56, 60, 64}}}, 
    {3,                 {11,                  30,           {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140}}}, 
    {4,                 {13,                  20,           {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}}}
};
#define EU_REGULATORY_INFO_SIZE (sizeof(EuropeRegulatoryInfo) / sizeof(DOT11_REGULATORY_INFO))


/* The regulatory information in Japan */
DOT11_REGULATORY_INFO JapanRegulatoryInfo[] = 
{
/*  "regulatory class"  "number of channels"  "Max Tx Pwr"  "channel list" */
    {0,                 {0,                   0,           {0}}}, /* Invalid entry*/
    {1,                 {4,                   22,           {34, 38, 42, 46}}}, 
    {2,                 {3,                   24,           {8, 12, 16}}}, 
    {3,                 {3,                   24,           {8, 12, 16}}}, 
    {4,                 {3,                   24,           {8, 12, 16}}}, 
    {5,                 {3,                   24,           {8, 12, 16}}}, 
    {6,                 {3,                   22,           {8, 12, 16}}}, 
    {7,                 {4,                   24,           {184, 188, 192, 196}}}, 
    {8,                 {4,                   24,           {184, 188, 192, 196}}}, 
    {9,                 {4,                   24,           {184, 188, 192, 196}}}, 
    {10,                {4,                   24,           {184, 188, 192, 196}}}, 
    {11,                {4,                   22,           {184, 188, 192, 196}}}, 
    {12,                {4,                   24,           {7, 8, 9, 11}}}, 
    {13,                {4,                   24,           {7, 8, 9, 11}}}, 
    {14,                {4,                   24,           {7, 8, 9, 11}}}, 
    {15,                {4,                   24,           {7, 8, 9, 11}}}, 
    {16,                {6,                   24,           {183, 184, 185, 187, 188, 189}}}, 
    {17,                {6,                   24,           {183, 184, 185, 187, 188, 189}}}, 
    {18,                {6,                   24,           {183, 184, 185, 187, 188, 189}}}, 
    {19,                {6,                   24,           {183, 184, 185, 187, 188, 189}}}, 
    {20,                {6,                   17,           {183, 184, 185, 187, 188, 189}}}, 
    {21,                {6,                   24,           {6, 7, 8, 9, 10, 11}}}, 
    {22,                {6,                   24,           {6, 7, 8, 9, 10, 11}}}, 
    {23,                {6,                   24,           {6, 7, 8, 9, 10, 11}}}, 
    {24,                {6,                   24,           {6, 7, 8, 9, 10, 11}}}, 
    {25,                {8,                   24,           {182, 183, 184, 185, 186, 187, 188, 189}}}, 
    {26,                {8,                   24,           {182, 183, 184, 185, 186, 187, 188, 189}}}, 
    {27,                {8,                   24,           {182, 183, 184, 185, 186, 187, 188, 189}}}, 
    {28,                {8,                   24,           {182, 183, 184, 185, 186, 187, 188, 189}}}, 
    {29,                {8,                   17,           {182, 183, 184, 185, 186, 187, 188, 189}}}, 
    {30,                {13,                  23,           {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}}}, 
    {31,                {1,                   23,           {14}}}, 
    {32,                {4,                   22,           {52, 56, 60, 64}}}
};
#define JP_REGULATORY_INFO_SIZE (sizeof(JapanRegulatoryInfo) / sizeof(DOT11_REGULATORY_INFO))


UINT8 GetRegulatoryMaxTxPwr(RTMP_ADAPTER *pAd, UINT8 channel)
{
	ULONG RegulatoryClassLoop, ChIdx;
	UINT8 RegulatoryClass;
	UINT8 MaxRegulatoryClassNum;
	DOT11_REGULATORY_INFO *pRegulatoryClass;
	RTMP_STRING *pCountry = (RTMP_STRING *)(pAd->CommonCfg.CountryCode);


	if (strncmp(pCountry, "US", 2) == 0)
	{
		MaxRegulatoryClassNum = USA_REGULATORY_INFO_SIZE;
		pRegulatoryClass = &USARegulatoryInfo[0];
	}
	else if (strncmp(pCountry, "JP", 2) == 0)
	{
		MaxRegulatoryClassNum = JP_REGULATORY_INFO_SIZE;
		pRegulatoryClass = &JapanRegulatoryInfo[0];
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknow Country (%s)\n",
					__FUNCTION__, pCountry));
		return 0xff;
	}

	for (RegulatoryClassLoop = 0;
			RegulatoryClassLoop<MAX_NUM_OF_REGULATORY_CLASS;
			RegulatoryClassLoop++)
	{
		DOT11_CHANNEL_SET *pChannelSet;

		RegulatoryClass = pAd->CommonCfg.RegulatoryClass[RegulatoryClassLoop];
		if (RegulatoryClass >= MaxRegulatoryClassNum)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: %c%c Unknow Requlatory class (%d)\n",
						__FUNCTION__, pCountry[0], pCountry[1], RegulatoryClass));
			return 0xff;
		}
		pChannelSet = &pRegulatoryClass[RegulatoryClass].ChannelSet;
		for (ChIdx=0; ChIdx<pChannelSet->NumberOfChannels; ChIdx++)
		{
			if (channel == pChannelSet->ChannelList[ChIdx])
				return pChannelSet->MaxTxPwr;
		
		}
		if (ChIdx == pChannelSet->NumberOfChannels)
			return 0xff;
	}

	return 0xff;
}

typedef struct __TX_PWR_CFG
{
	UINT8 Mode;
	UINT8 MCS;
	UINT16 req;
	UINT8 shift;
	UINT32 BitMask;
} TX_PWR_CFG;

/* Note: the size of TxPwrCfg is too large, do not put it to function */
TX_PWR_CFG TxPwrCfg[] = {
	{MODE_CCK, 0, 0, 4, 0x000000f0},
	{MODE_CCK, 1, 0, 0, 0x0000000f},
	{MODE_CCK, 2, 0, 12, 0x0000f000},
	{MODE_CCK, 3, 0, 8, 0x00000f00},

	{MODE_OFDM, 0, 0, 20, 0x00f00000},
	{MODE_OFDM, 1, 0, 16, 0x000f0000},
	{MODE_OFDM, 2, 0, 28, 0xf0000000},
	{MODE_OFDM, 3, 0, 24, 0x0f000000},
	{MODE_OFDM, 4, 1, 4, 0x000000f0},
	{MODE_OFDM, 5, 1, 0, 0x0000000f},
	{MODE_OFDM, 6, 1, 12, 0x0000f000},
	{MODE_OFDM, 7, 1, 8, 0x00000f00}
#ifdef DOT11_N_SUPPORT
	,{MODE_HTMIX, 0, 1, 20, 0x00f00000},
	{MODE_HTMIX, 1, 1, 16, 0x000f0000},
	{MODE_HTMIX, 2, 1, 28, 0xf0000000},
	{MODE_HTMIX, 3, 1, 24, 0x0f000000},
	{MODE_HTMIX, 4, 2, 4, 0x000000f0},
	{MODE_HTMIX, 5, 2, 0, 0x0000000f},
	{MODE_HTMIX, 6, 2, 12, 0x0000f000},
	{MODE_HTMIX, 7, 2, 8, 0x00000f00},
	{MODE_HTMIX, 8, 2, 20, 0x00f00000},
	{MODE_HTMIX, 9, 2, 16, 0x000f0000},
	{MODE_HTMIX, 10, 2, 28, 0xf0000000},
	{MODE_HTMIX, 11, 2, 24, 0x0f000000},
	{MODE_HTMIX, 12, 3, 4, 0x000000f0},
	{MODE_HTMIX, 13, 3, 0, 0x0000000f},
	{MODE_HTMIX, 14, 3, 12, 0x0000f000},
	{MODE_HTMIX, 15, 3, 8, 0x00000f00}
#endif /* DOT11_N_SUPPORT */
};
#define MAX_TXPWR_TAB_SIZE (sizeof(TxPwrCfg) / sizeof(TX_PWR_CFG))


CHAR RTMP_GetTxPwr(RTMP_ADAPTER *pAd, HTTRANSMIT_SETTING HTTxMode)
{
	UINT32 Value;
	INT Idx;
	UINT8 PhyMode;
	CHAR CurTxPwr;
	UINT8 TxPwrRef = 0;
	CHAR DaltaPwr;
	ULONG TxPwr[5];


#ifdef SINGLE_SKU
	CurTxPwr = pAd->CommonCfg.DefineMaxTxPwr;
#else
	CurTxPwr = 19;
#endif /* SINGLE_SKU */

	/* check Tx Power setting from UI. */
	if (pAd->CommonCfg.TxPowerPercentage > 90)
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)  /* reduce Pwr for 1 dB. */
		CurTxPwr -= 1;
	else if (pAd->CommonCfg.TxPowerPercentage > 30)  /* reduce Pwr for 3 dB. */
		CurTxPwr -= 3;
	else if (pAd->CommonCfg.TxPowerPercentage > 15)  /* reduce Pwr for 6 dB. */
		CurTxPwr -= 6;
	else if (pAd->CommonCfg.TxPowerPercentage > 9)   /* reduce Pwr for 9 dB. */
		CurTxPwr -= 9;
	else                                           /* reduce Pwr for 12 dB. */
		CurTxPwr -= 12;

	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			TxPwr[0] = pAd->Tx40MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel > 14)
		{
			TxPwr[0] = pAd->Tx20MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgABand[4];
		}
		else
		{
			TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];
		}
	}


	switch(HTTxMode.field.MODE)
	{
		case MODE_CCK:
		case MODE_OFDM:
			Value = TxPwr[1];
			TxPwrRef = (Value & 0x00000f00) >> 8;
			
			break;

#ifdef DOT11_N_SUPPORT
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			if (pAd->CommonCfg.TxStream == 1)
			{
				Value = TxPwr[2];
				TxPwrRef = (Value & 0x00000f00) >> 8;
			}
			else if (pAd->CommonCfg.TxStream == 2)
			{
				Value = TxPwr[3];
				TxPwrRef = (Value & 0x00000f00) >> 8;
			}
			break;
#endif /* DOT11_N_SUPPORT */
	}

	PhyMode =
#ifdef DOT11_N_SUPPORT
				(HTTxMode.field.MODE == MODE_HTGREENFIELD)
				? MODE_HTMIX :
#endif /* DOT11_N_SUPPORT */
				HTTxMode.field.MODE;

	for (Idx = 0; Idx < MAX_TXPWR_TAB_SIZE; Idx++)
	{
		if ((TxPwrCfg[Idx].Mode == PhyMode)
			&& (TxPwrCfg[Idx].MCS == HTTxMode.field.MCS))
		{
			Value = TxPwr[TxPwrCfg[Idx].req];
			DaltaPwr = TxPwrRef - (CHAR)((Value & TxPwrCfg[Idx].BitMask)
											>> TxPwrCfg[Idx].shift);
			CurTxPwr -= DaltaPwr;
			break;
		}
	}

	return CurTxPwr;
}


NDIS_STATUS	MeasureReqTabInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

	os_alloc_mem(pAd, (UCHAR **)&(pAd->CommonCfg.pMeasureReqTab), sizeof(MEASURE_REQ_TAB));
	if (pAd->CommonCfg.pMeasureReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pMeasureReqTab, sizeof(MEASURE_REQ_TAB));

		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.MeasureReqTabLock);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for pAd->CommonCfg.pMeasureReqTab.\n", __FUNCTION__));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID MeasureReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pMeasureReqTab) {
		os_free_mem(NULL, pAd->CommonCfg.pMeasureReqTab);
		pAd->CommonCfg.pMeasureReqTab = NULL;

		NdisFreeSpinLock(&pAd->CommonCfg.MeasureReqTabLock);
	}
	return;
}


PMEASURE_REQ_ENTRY MeasureReqLookUp(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	UINT HashIdx;
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	//PMEASURE_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __FUNCTION__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);

	HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry)
	{
		if (pEntry->DialogToken == DialogToken) 
			break;
		else
		{
			//pPrevEntry = pEntry;
			pEntry = pEntry->pNext;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	
	return pEntry;
}


PMEASURE_REQ_ENTRY MeasureReqInsert(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	INT i;
	ULONG HashIdx;
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL, pCurrEntry;
	ULONG Now;

	if(pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __FUNCTION__));
		return NULL;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken);
	if (pEntry == NULL)
	{
		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);
		for (i = 0; i < MAX_MEASURE_REQ_TAB_SIZE; i++)
		{
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];
 
			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now, (unsigned long)(pEntry->lastTime + MQ_REQ_AGE_OUT)))
			{
				PMEASURE_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];
				BOOLEAN Cancelled;

				/* update Hash list*/
				do
				{
					if (pProbeEntry == pEntry)
					{
						if (pPrevEntry == NULL)
						{
							pTab->Hash[HashIdx] = pEntry->pNext;
						}
						else
						{
							pPrevEntry->pNext = pEntry->pNext;
						}
						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);

				RTMPCancelTimer(&pEntry->WaitBCNRepTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->WaitBCNRepTimer, &Cancelled);
				RTMPCancelTimer(&pEntry->WaitNRRspTimer, &Cancelled);
				RTMPReleaseTimer(&pEntry->WaitNRRspTimer, &Cancelled);
				NdisZeroMemory(pEntry, sizeof(MEASURE_REQ_ENTRY));
				pTab->Size--;

				break;
			}
 
			if (pEntry->Valid == FALSE)
				break;
		}

		if (i < MAX_MEASURE_REQ_TAB_SIZE)
		{
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
			pTab->Size++;
		}
		else
		{
			pEntry = NULL;
			DBGPRINT(RT_DEBUG_ERROR, ("%s: pMeasureReqTab tab full.\n", __FUNCTION__));
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry)
		{
			HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);
			if (pTab->Hash[HashIdx] == NULL)
			{
				pTab->Hash[HashIdx] = pEntry;
			}
			else
			{
				pCurrEntry = pTab->Hash[HashIdx];
				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;
				pCurrEntry->pNext = pEntry;
			}
		}

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	} 

	return pEntry;
}


VOID MeasureReqDelete(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	PMEASURE_REQ_TAB pTab = pAd->CommonCfg.pMeasureReqTab;
	PMEASURE_REQ_ENTRY pEntry = NULL;

	if(pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __FUNCTION__));
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pMeasureReqTab empty.\n"));
		return;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken);
	if (pEntry != NULL)
	{
		PMEASURE_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);
		/* update Hash list*/
		do
		{
			if (pProbeEntry == pEntry)
			{
				if (pPrevEntry == NULL)
				{
					pTab->Hash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);

		NdisZeroMemory(pEntry, sizeof(MEASURE_REQ_ENTRY));
		pTab->Size--;

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.MeasureReqTabLock);
	}

	return;
}


NDIS_STATUS TpcReqTabInit(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;

	os_alloc_mem(pAd, (UCHAR **)&(pAd->CommonCfg.pTpcReqTab), sizeof(TPC_REQ_TAB));
	if (pAd->CommonCfg.pTpcReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pTpcReqTab, sizeof(TPC_REQ_TAB));

		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.TpcReqTabLock);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for pAd->CommonCfg.pTpcReqTab.\n", __FUNCTION__));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID TpcReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pTpcReqTab) {
		os_free_mem(NULL, pAd->CommonCfg.pTpcReqTab);
		pAd->CommonCfg.pTpcReqTab = NULL;

		NdisFreeSpinLock(&pAd->CommonCfg.TpcReqTabLock);
	}

	return;
}


static PTPC_REQ_ENTRY TpcReqLookUp(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	UINT HashIdx;
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL;
	//PTPC_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __FUNCTION__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);

	HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry)
	{
		if (pEntry->DialogToken == DialogToken) 
			break;
		else
		{
			//pPrevEntry = pEntry;
			pEntry = pEntry->pNext;
		}
	}

	RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	
	return pEntry;
}


static PTPC_REQ_ENTRY TpcReqInsert(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	INT i;
	ULONG HashIdx;
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL, pCurrEntry;
	ULONG Now;

	if(pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __FUNCTION__));
		return NULL;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);
	if (pEntry == NULL)
	{
		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);
		for (i = 0; i < MAX_TPC_REQ_TAB_SIZE; i++)
		{
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];
 
			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now, (unsigned long)(pEntry->lastTime + TPC_REQ_AGE_OUT)))
			{
				PTPC_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

				/* update Hash list*/
				do
				{
					if (pProbeEntry == pEntry)
					{
						if (pPrevEntry == NULL)
						{
							pTab->Hash[HashIdx] = pEntry->pNext;
						}
						else
						{
							pPrevEntry->pNext = pEntry->pNext;
						}
						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);

				NdisZeroMemory(pEntry, sizeof(TPC_REQ_ENTRY));
				pTab->Size--;

				break;
			}
 
			if (pEntry->Valid == FALSE)
				break;
		}

		if (i < MAX_TPC_REQ_TAB_SIZE)
		{
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
			pTab->Size++;
		}
		else
		{
			pEntry = NULL;
			DBGPRINT(RT_DEBUG_ERROR, ("%s: pTpcReqTab tab full.\n", __FUNCTION__));
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry)
		{
			HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);
			if (pTab->Hash[HashIdx] == NULL)
			{
				pTab->Hash[HashIdx] = pEntry;
			}
			else
			{
				pCurrEntry = pTab->Hash[HashIdx];
				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;
				pCurrEntry->pNext = pEntry;
			}
		}

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	} 

	return pEntry;
}


static VOID TpcReqDelete(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL;

	if(pTab == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __FUNCTION__));
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pTpcReqTab empty.\n"));
		return;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);
	if (pEntry != NULL)
	{
		PTPC_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);
		/* update Hash list*/
		do
		{
			if (pProbeEntry == pEntry)
			{
				if (pPrevEntry == NULL)
				{
					pTab->Hash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);

		NdisZeroMemory(pEntry, sizeof(TPC_REQ_ENTRY));
		pTab->Size--;

		RTMP_SEM_UNLOCK(&pAd->CommonCfg.TpcReqTabLock);
	}

	return;
}


/*
	==========================================================================
	Description:
		Get Current TimeS tamp.
		
	Parametrs:
	
	Return	: Current Time Stamp.
	==========================================================================
 */
static UINT64 GetCurrentTimeStamp(RTMP_ADAPTER *pAd)
{
	/* get current time stamp.*/
	return 0;
}


/*
	==========================================================================
	Description:
		Get Current Transmit Power.
		
	Parametrs:
	
	Return	: Current Time Stamp.
	==========================================================================
 */
static UINT8 GetCurTxPwr(RTMP_ADAPTER *pAd, UINT8 Wcid)
{
	return 16; /* 16 dBm */
}


/*
	==========================================================================
	Description:
		Get Current Transmit Power.
		
	Parametrs:
	
	Return	: Current Time Stamp.
	==========================================================================
 */
VOID InsertChannelRepIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN RTMP_STRING *pCountry,
	IN UINT8 RegulatoryClass,
	IN UINT8 *ChReptList)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 IEId = IE_AP_CHANNEL_REPORT;
	PUCHAR pChListPtr = NULL;
	UINT8 i,j;
	PUCHAR channel_set = NULL;
	UCHAR ChannelList[16] ={0};
	UINT8 NumberOfChannels = 0;
	UINT8 *pChannelList = NULL;
	UCHAR channel_set_num = 0;
	UCHAR ch_list_num = 0;

	Len = 1;

	channel_set = get_channelset_by_reg_class(pAd, RegulatoryClass);
	channel_set_num = get_channel_set_num(channel_set);

	ch_list_num = get_channel_set_num(ChReptList);

	/* no match channel set. */
	if (channel_set == NULL)
		return;

	/* empty channel set. */
	if (channel_set_num == 0)
		return;

	if (ch_list_num) { /* assign partial channel list */
		for (i = 0; i < channel_set_num; i++) {
			for (j = 0; j < ch_list_num; j++) {
				if (ChReptList[j] == channel_set[i])
					ChannelList[NumberOfChannels++] = channel_set[i];
			}
		}

		pChannelList = &ChannelList[0];
	} else {
		NumberOfChannels = channel_set_num;
		pChannelList = channel_set;
	}

	Len += NumberOfChannels;
	pChListPtr = pChannelList;

	if (Len > 1)
	{
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
						1,				&IEId,
						1,				&Len,
						1,				&RegulatoryClass,
						Len -1,			pChListPtr,
						END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}
	return;
}

/*
*	==========================================================================
*	Description:
*		Add last beacon report indication request into beacon request
*
*	Parametrs:
*
*	Return	: NAN
*	==========================================================================
 */
VOID InsertBcnReportIndicationReqIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Data
)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 IEId = IE_LAST_BCN_REPORT_INDICATION_REQUEST;

	Len = 1;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
					  1,				&IEId,
					  1,				&Len,
					  1,				&Data,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

/*
	==========================================================================
	Description:
		Insert Dialog Token into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Dialog token.
	
	Return	: None.
	==========================================================================
 */
VOID InsertDialogToken(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 DialogToken)
{
	ULONG TempLen;
	MakeOutgoingFrame(pFrameBuf,	&TempLen,
					1,				&DialogToken,
					END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


/*
	==========================================================================
	Description:
		Insert TPC Request IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
	
	Return	: None.
	==========================================================================
 */
 static VOID InsertTpcReqIE(RTMP_ADAPTER *pAd, UCHAR *frm_buf, ULONG *frm_len)
{
	ULONG TempLen;
	UINT8 Len = 0;
	UINT8 ElementID = IE_TPC_REQUEST;

	MakeOutgoingFrame(frm_buf,					&TempLen,
						1,							&ElementID,
						1,							&Len,
						END_OF_ARGS);

	*frm_len = *frm_len + TempLen;

	return;
}


/*
	==========================================================================
	Description:
		Insert TPC Report IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Transmit Power.
		4. Link Margin.
	
	Return	: None.
	==========================================================================
 */
VOID InsertTpcReportIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin)
{
	ULONG TempLen;
	UINT8 Len = sizeof(TPC_REPORT_INFO);
	UINT8 ElementID = IE_TPC_REPORT;
	TPC_REPORT_INFO TpcReportIE;

	TpcReportIE.TxPwr = TxPwr;
	TpcReportIE.LinkMargin = LinkMargin;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&ElementID,
						1,							&Len,
						Len,						&TpcReportIE,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;


	return;
}

/*
	==========================================================================
	Description:
		Insert Measure Request IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Measure Token.
		4. Measure Request Mode.
		5. Measure Request Type.
		6. Measure Channel.
		7. Measure Start time.
		8. Measure Duration.

	
	Return	: None.
	==========================================================================
 */
static VOID InsertMeasureReqIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN PMEASURE_REQ_INFO pMeasureReqIE)
{
	ULONG TempLen;
	UINT8 ElementID = IE_MEASUREMENT_REQUEST;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&ElementID,
						1,							&Len,
						sizeof(MEASURE_REQ_INFO),	pMeasureReqIE,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


/*
	==========================================================================
	Description:
		Insert Measure Report IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. Measure Token.
		4. Measure Request Mode.
		5. Measure Request Type.
		6. Length of Report Infomation 
		7. Pointer of Report Infomation Buffer.
	
	Return	: None.
	==========================================================================
 */
static VOID InsertMeasureReportIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PMEASURE_REPORT_INFO pMeasureReportIE,
	IN UINT8 ReportLnfoLen,
	IN PUINT8 pReportInfo)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 ElementID = IE_MEASUREMENT_REPORT;

	Len = sizeof(MEASURE_REPORT_INFO) + ReportLnfoLen;
		
	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&ElementID,
						1,							&Len,
						Len,						pMeasureReportIE,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	if ((ReportLnfoLen > 0) && (pReportInfo != NULL))
	{
		MakeOutgoingFrame(pFrameBuf + *pFrameLen,		&TempLen,
							ReportLnfoLen,				pReportInfo,
							END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}
	return;
}


/*
	==========================================================================
	Description:
		Prepare Measurement request action frame and enqueue it into
		management queue waiting for transmition.
		
	Parametrs:
		1. the destination mac address of the frame.
	
	Return	: None.
	==========================================================================
 */
VOID MakeMeasurementReqFrame(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pOutBuffer,
	OUT PULONG pFrameLen,
	IN UINT8 TotalLen,
	IN UINT8 Category,
	IN UINT8 Action,
	IN UINT8 MeasureToken, 
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT16 NumOfRepetitions)
{
	ULONG TempLen;
	MEASURE_REQ_INFO MeasureReqIE;
	UINT16 leRepetitions;

	InsertActField(pAd, (pOutBuffer + *pFrameLen), pFrameLen, Category, Action);

	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + *pFrameLen), pFrameLen, MeasureToken);
	leRepetitions = cpu2le16(NumOfRepetitions);


	/* fill Number of repetitions. */
	if (Category == CATEGORY_RM)
	{
		MakeOutgoingFrame((pOutBuffer+*pFrameLen),	&TempLen,
						2,							&leRepetitions,
						END_OF_ARGS);

		*pFrameLen += TempLen;
	}

	/* prepare Measurement IE.*/
	NdisZeroMemory(&MeasureReqIE, sizeof(MEASURE_REQ_INFO));
	MeasureReqIE.Token = MeasureToken;
	MeasureReqIE.ReqMode.word = MeasureReqMode;
	MeasureReqIE.ReqType = MeasureReqType;
	InsertMeasureReqIE(pAd, (pOutBuffer+*pFrameLen), pFrameLen,
		TotalLen, &MeasureReqIE);

	return;
}


/*
	==========================================================================
	Description:
		Prepare Measurement report action frame and enqueue it into
		management queue waiting for transmition.
		
	Parametrs:
		1. the destination mac address of the frame.
	
	Return	: None.
	==========================================================================
 */
VOID EnqueueMeasurementRep(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 MeasureToken, 
	IN UINT8 MeasureReqMode,
	IN UINT8 MeasureReqType,
	IN UINT8 ReportInfoLen,
	IN PUINT8 pReportInfo)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	MEASURE_REPORT_INFO MeasureRepIE;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
						pAd->CurrentAddress,
						pAd->CurrentAddress);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_MRP);

	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

	/* prepare Measurement IE.*/
	NdisZeroMemory(&MeasureRepIE, sizeof(MEASURE_REPORT_INFO));
	MeasureRepIE.Token = MeasureToken;
	MeasureRepIE.ReportMode = MeasureReqMode;
	MeasureRepIE.ReportType = MeasureReqType;
	InsertMeasureReportIE(pAd, (pOutBuffer + FrameLen), &FrameLen, &MeasureRepIE, ReportInfoLen, pReportInfo);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;	
}


/*
	==========================================================================
	Description:
		Prepare TPC Request action frame and enqueue it into
		management queue waiting for transmition.
		
	Parametrs:
		1. the destination mac address of the frame.
	
	Return	: None.
	==========================================================================
 */
VOID EnqueueTPCReq(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN UCHAR DialogToken)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
						pAd->CurrentAddress,
						pAd->CurrentAddress);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_TPCRQ);

	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

	/* Insert TPC Request IE.*/
	InsertTpcReqIE(pAd, (pOutBuffer + FrameLen), &FrameLen);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;
}


/*
	==========================================================================
	Description:
		Prepare TPC Report action frame and enqueue it into
		management queue waiting for transmition.
		
	Parametrs:
		1. the destination mac address of the frame.
	
	Return	: None.
	==========================================================================
 */
VOID EnqueueTPCRep(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA,
	IN UINT8 DialogToken,
	IN UINT8 TxPwr,
	IN UINT8 LinkMargin)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
						pAd->CurrentAddress,
						pAd->CurrentAddress);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_TPCRP);

	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

	/* Insert TPC Request IE.*/
	InsertTpcReportIE(pAd, (pOutBuffer + FrameLen), &FrameLen, TxPwr, LinkMargin);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;
}


#ifdef WDS_SUPPORT
/*
	==========================================================================
	Description:
		Insert Channel Switch Announcement IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. channel switch announcement mode.
		4. new selected channel.
		5. channel switch announcement count.
	
	Return	: None.
	==========================================================================
 */
static VOID InsertChSwAnnIE(
	IN RTMP_ADAPTER *pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(CH_SW_ANN_INFO);
	UINT8 ElementID = IE_CHANNEL_SWITCH_ANNOUNCEMENT;
	CH_SW_ANN_INFO ChSwAnnIE;

	ChSwAnnIE.ChSwMode = ChSwMode;
	ChSwAnnIE.Channel = NewChannel;
	ChSwAnnIE.ChSwCnt = ChSwCnt;

	MakeOutgoingFrame(pFrameBuf,				&TempLen,
						1,						&ElementID,
						1,						&Len,
						Len,					&ChSwAnnIE,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;


	return;
}


/*
	==========================================================================
	Description:
		Prepare Channel Switch Announcement action frame and enqueue it into
		management queue waiting for transmition.
		
	Parametrs:
		1. the destination mac address of the frame.
		2. Channel switch announcement mode.
		2. a New selected channel.
	
	Return	: None.
	==========================================================================
 */
VOID EnqueueChSwAnn(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pDA, 
	IN UINT8 ChSwMode,
	IN UINT8 NewCh)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
						pAd->CurrentAddress,
						pAd->CurrentAddress);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);

	InsertChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, 0);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;
}
#endif /* WDS_SUPPORT */


static BOOLEAN DfsRequirementCheck(RTMP_ADAPTER *pAd, UINT8 Channel)
{
	BOOLEAN Result = FALSE;
	INT i;

	do
	{
		/* check DFS procedure is running.*/
		/* make sure DFS procedure won't start twice.*/
		if (pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
		{
			Result = FALSE;
			break;
		}

		/* check the new channel carried from Channel Switch Announcemnet is valid.*/
		for (i=0; i<pAd->ChannelListNum; i++)
		{
			if ((Channel == pAd->ChannelList[i].Channel)
				&&(pAd->ChannelList[i].RemainingTimeForUse == 0))
			{
				/* found radar signal in the channel. the channel can't use at least for 30 minutes.*/
				pAd->ChannelList[i].RemainingTimeForUse = 1800;/*30 min = 1800 sec*/
				Result = TRUE;
				break;
			}
		}
	} while(FALSE);

	return Result;
}


VOID NotifyChSwAnnToPeerAPs(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pRA,
	IN PUCHAR pTA,
	IN UINT8 ChSwMode,
	IN UINT8 Channel)
{
#ifdef WDS_SUPPORT
	if (!((pRA[0] & 0xff) == 0xff)) /* is pRA a broadcase address.*/
	{
		INT i;
		/* info neighbor APs that Radar signal found throgh WDS link.*/
		for (i = 0; i < MAX_WDS_ENTRY; i++)
		{
			if (ValidWdsEntry(pAd, i))
			{
				PUCHAR pDA = pAd->WdsTab.WdsEntry[i].PeerWdsAddr;

				/* DA equal to SA. have no necessary orignal AP which found Radar signal.*/
				if (MAC_ADDR_EQUAL(pTA, pDA))
					continue;

				/* send Channel Switch Action frame to info Neighbro APs.*/
				EnqueueChSwAnn(pAd, pDA, ChSwMode, Channel);
			}
		}
	}
#endif /* WDS_SUPPORT */
}


static VOID StartDFSProcedure(RTMP_ADAPTER *pAd, UCHAR Channel, UINT8 ChSwMode)
{
	/* start DFS procedure*/
	pAd->CommonCfg.Channel = Channel;
#ifdef DOT11_N_SUPPORT
	N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
	pAd->Dot11_H.RDMode = RD_SWITCHING_MODE;
	pAd->Dot11_H.CSCount = 0;
}


/*
	==========================================================================
	Description:
		Channel Switch Announcement action frame sanity check.
		
	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Channel switch announcement infomation buffer.
		
	
	Return	: None.
	==========================================================================
 */

/*
  Channel Switch Announcement IE.
  +----+-----+-----------+------------+-----------+
  | ID | Len |Ch Sw Mode | New Ch Num | Ch Sw Cnt |
  +----+-----+-----------+------------+-----------+
    1    1        1           1            1      
*/
static BOOLEAN PeerChSwAnnSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PCH_SW_ANN_INFO pChSwAnnInfo)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;

	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pChSwAnnInfo == NULL)
		return result;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
				NdisMoveMemory(&pChSwAnnInfo->ChSwMode, eid_ptr->Octet, 1);
				NdisMoveMemory(&pChSwAnnInfo->Channel, eid_ptr->Octet + 1, 1);
				NdisMoveMemory(&pChSwAnnInfo->ChSwCnt, eid_ptr->Octet + 2, 1);
				
				result = TRUE;
                break;
            
			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
	}

	return result;
}


/*
	==========================================================================
	Description:
		Measurement request action frame sanity check.
		
	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Measurement request infomation buffer.
	
	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerMeasureReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REQ_INFO pMeasureReqInfo,
	OUT PMEASURE_REQ pMeasureReq)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PUCHAR ptr;
	UINT64 MeasureStartTime;
	UINT16 MeasureDuration;

	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pMeasureReqInfo == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_MEASUREMENT_REQUEST:
				NdisMoveMemory(&pMeasureReqInfo->Token, eid_ptr->Octet, 1);
				NdisMoveMemory(&pMeasureReqInfo->ReqMode.word, eid_ptr->Octet + 1, 1);
				NdisMoveMemory(&pMeasureReqInfo->ReqType, eid_ptr->Octet + 2, 1);
				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pMeasureReq->ChNum, ptr, 1);
				NdisMoveMemory(&MeasureStartTime, ptr + 1, 8);
				pMeasureReq->MeasureStartTime = SWAP64(MeasureStartTime);
				NdisMoveMemory(&MeasureDuration, ptr + 9, 2);
				pMeasureReq->MeasureDuration = SWAP16(MeasureDuration);
				
				result = TRUE;
				break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


/*
	==========================================================================
	Description:
		Measurement report action frame sanity check.
		
	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Measurement report infomation buffer.
		4. basic report infomation buffer.
	
	Return	: None.
	==========================================================================
 */

/*
  Measurement Report IE.
  +----+-----+-------+-------------+--------------+----------------+
  | ID | Len | Token | Report Mode | Measure Type | Measure Report |
  +----+-----+-------+-------------+--------------+----------------+
    1     1      1          1             1            variable

  Basic Report.
  +--------+------------+----------+-----+
  | Ch Num | Start Time | Duration | Map |
  +--------+------------+----------+-----+
      1          8           2        1

  Map Field Bit Format.
  +-----+---------------+---------------------+-------+------------+----------+
  | Bss | OFDM Preamble | Unidentified signal | Radar | Unmeasured | Reserved |
  +-----+---------------+---------------------+-------+------------+----------+
     0          1                  2              3         4          5-7
*/
static BOOLEAN PeerMeasureReportSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PMEASURE_REPORT_INFO pMeasureReportInfo,
	OUT PUINT8 pReportBuf)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;
	PUCHAR ptr;

	/* skip 802.11 header.*/
	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pMeasureReportInfo == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_MEASUREMENT_REPORT:
				NdisMoveMemory(&pMeasureReportInfo->Token, eid_ptr->Octet, 1);
				NdisMoveMemory(&pMeasureReportInfo->ReportMode, eid_ptr->Octet + 1, 1);
				NdisMoveMemory(&pMeasureReportInfo->ReportType, eid_ptr->Octet + 2, 1);
				if (pMeasureReportInfo->ReportType == RM_BASIC)
				{
					PMEASURE_BASIC_REPORT pReport = (PMEASURE_BASIC_REPORT)pReportBuf;
					ptr = (PUCHAR)(eid_ptr->Octet + 3);
					NdisMoveMemory(&pReport->ChNum, ptr, 1);
					NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
					NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
					NdisMoveMemory(&pReport->Map, ptr + 11, 1);
					
				}
				else if (pMeasureReportInfo->ReportType == RM_CCA)
				{
					PMEASURE_CCA_REPORT pReport = (PMEASURE_CCA_REPORT)pReportBuf;
					ptr = (PUCHAR)(eid_ptr->Octet + 3);
					NdisMoveMemory(&pReport->ChNum, ptr, 1);
					NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
					NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
					NdisMoveMemory(&pReport->CCA_Busy_Fraction, ptr + 11, 1);
					
				}
				else if (pMeasureReportInfo->ReportType == RM_RPI_HISTOGRAM)
				{
					PMEASURE_RPI_REPORT pReport = (PMEASURE_RPI_REPORT)pReportBuf;
					ptr = (PUCHAR)(eid_ptr->Octet + 3);
					NdisMoveMemory(&pReport->ChNum, ptr, 1);
					NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
					NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
					NdisMoveMemory(&pReport->RPI_Density, ptr + 11, 8);
				}
				result = TRUE;
                break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


/*
	==========================================================================
	Description:
		TPC Request action frame sanity check.
		
	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Dialog Token.
	
	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerTpcReqSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;

	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pDialogToken == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_TPC_REQUEST:
				result = TRUE;
                break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;	
}


/*
	==========================================================================
	Description:
		TPC Report action frame sanity check.
		
	Parametrs:
		1. MLME message containing the received frame
		2. message length.
		3. Dialog Token.
		4. TPC Report IE.
	
	Return	: None.
	==========================================================================
 */
static BOOLEAN PeerTpcRepSanity(
	IN RTMP_ADAPTER *pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUINT8 pDialogToken,
	OUT PTPC_REPORT_INFO pTpcRepInfo)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pMsg;
	PUCHAR pFramePtr = Fr->Octet;
	BOOLEAN result = FALSE;
	PEID_STRUCT eid_ptr;

	MsgLen -= sizeof(HEADER_802_11);

	/* skip category and action code.*/
	pFramePtr += 2;
	MsgLen -= 2;

	if (pDialogToken == NULL)
		return result;

	NdisMoveMemory(pDialogToken, pFramePtr, 1);
	pFramePtr += 1;
	MsgLen -= 1;

	eid_ptr = (PEID_STRUCT)pFramePtr;
	while (((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen))
	{
		switch(eid_ptr->Eid)
		{
			case IE_TPC_REPORT:
				NdisMoveMemory(&pTpcRepInfo->TxPwr, eid_ptr->Octet, 1);
				NdisMoveMemory(&pTpcRepInfo->LinkMargin, eid_ptr->Octet + 1, 1);
				result = TRUE;
                break;

			default:
				break;
		}
		eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;	
}


/*
	==========================================================================
	Description:
		Channel Switch Announcement action frame handler.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
static VOID PeerChSwAnnAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	CH_SW_ANN_INFO ChSwAnnInfo;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;

	NdisZeroMemory(&ChSwAnnInfo, sizeof(CH_SW_ANN_INFO));
	if (! PeerChSwAnnSanity(pAd, Elem->Msg, Elem->MsgLen, &ChSwAnnInfo))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Invalid Channel Switch Action Frame.\n"));
		return;
	}

#ifdef CONFIG_AP_SUPPORT
	/* ChSwAnn need check.*/
	if ((pAd->OpMode == OPMODE_AP) &&
		(DfsRequirementCheck(pAd, ChSwAnnInfo.Channel) == TRUE))
	{
		NotifyChSwAnnToPeerAPs(pAd, pFr->Hdr.Addr1, pFr->Hdr.Addr2, ChSwAnnInfo.ChSwMode, ChSwAnnInfo.Channel);
		StartDFSProcedure(pAd, ChSwAnnInfo.Channel, ChSwAnnInfo.ChSwMode);
	}
#endif /* CONFIG_AP_SUPPORT */


	return;
}


/*
	==========================================================================
	Description:
		Measurement Request action frame handler.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
static VOID PeerMeasureReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	UINT8 DialogToken;
	MEASURE_REQ_INFO MeasureReqInfo;
	MEASURE_REQ	MeasureReq;
	MEASURE_REPORT_MODE ReportMode;

	if(PeerMeasureReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReqInfo, &MeasureReq))
	{
		ReportMode.word = 0;
		ReportMode.field.Incapable = 1;
		EnqueueMeasurementRep(pAd, pFr->Hdr.Addr2, DialogToken, MeasureReqInfo.Token, ReportMode.word, MeasureReqInfo.ReqType, 0, NULL);
	}

	return;
}


/*
	==========================================================================
	Description:
		Measurement Report action frame handler.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
static VOID PeerMeasureReportAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	MEASURE_REPORT_INFO MeasureReportInfo;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	UINT8 DialogToken;
	PUINT8 pMeasureReportInfo;

	os_alloc_mem(pAd, (UCHAR **)&pMeasureReportInfo, sizeof(MEASURE_RPI_REPORT));
	if (pMeasureReportInfo == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s unable to alloc memory for measure report buffer (size=%d).\n", __FUNCTION__, sizeof(MEASURE_RPI_REPORT)));
		return;
	}

	NdisZeroMemory(&MeasureReportInfo, sizeof(MEASURE_REPORT_INFO));
	NdisZeroMemory(pMeasureReportInfo, sizeof(MEASURE_RPI_REPORT));
	if (PeerMeasureReportSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReportInfo, pMeasureReportInfo))
	{
		do {
			PMEASURE_REQ_ENTRY pEntry = NULL;

			/* Not a autonomous measure report.*/
			/* check the dialog token field. drop it if the dialog token doesn't match.*/
			if ((DialogToken != 0)
				&& ((pEntry = MeasureReqLookUp(pAd, DialogToken)) == NULL))
				break;

			if (pEntry != NULL)
				MeasureReqDelete(pAd, pEntry->DialogToken);

			if (MeasureReportInfo.ReportType == RM_BASIC)
			{
				PMEASURE_BASIC_REPORT pBasicReport = (PMEASURE_BASIC_REPORT)pMeasureReportInfo;
				if ((pBasicReport->Map.field.Radar)
					&& (DfsRequirementCheck(pAd, pBasicReport->ChNum) == TRUE))
				{
					NotifyChSwAnnToPeerAPs(pAd, pFr->Hdr.Addr1, pFr->Hdr.Addr2, 1, pBasicReport->ChNum);
					StartDFSProcedure(pAd, pBasicReport->ChNum, 1);
				}
			}
		} while (FALSE);
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("Invalid Measurement Report Frame.\n"));

	os_free_mem(NULL, pMeasureReportInfo);

	return;
}


/*
	==========================================================================
	Description:
		TPC Request action frame handler.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
static VOID PeerTpcReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	PUCHAR pFramePtr = pFr->Octet;
	UINT8 DialogToken;
	UINT8 TxPwr = GetCurTxPwr(pAd, Elem->Wcid);
	UINT8 LinkMargin = 0;
	CHAR RealRssi;

	/* link margin: Ratio of the received signal power to the minimum desired by the station (STA). The*/
	/*				STA may incorporate rate information and channel conditions, including interference, into its computation*/
	/*				of link margin.*/

	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
				ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));

	/* skip Category and action code.*/
	pFramePtr += 2;

	/* Dialog token.*/
	NdisMoveMemory(&DialogToken, pFramePtr, 1);

	LinkMargin = (RealRssi / MIN_RCV_PWR);
	if (PeerTpcReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken))
		EnqueueTPCRep(pAd, pFr->Hdr.Addr2, DialogToken, TxPwr, LinkMargin);

	return;
}


/*
	==========================================================================
	Description:
		TPC Report action frame handler.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
static VOID PeerTpcRepAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UINT8 DialogToken;
	TPC_REPORT_INFO TpcRepInfo;
	PTPC_REQ_ENTRY pEntry = NULL;

	NdisZeroMemory(&TpcRepInfo, sizeof(TPC_REPORT_INFO));
	if (PeerTpcRepSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &TpcRepInfo))
	{
		if ((pEntry = TpcReqLookUp(pAd, DialogToken)) != NULL)
		{
			TpcReqDelete(pAd, pEntry->DialogToken);
			DBGPRINT(RT_DEBUG_TRACE, ("%s: DialogToken=%x, TxPwr=%d, LinkMargin=%d\n",
				__FUNCTION__, DialogToken, TpcRepInfo.TxPwr, TpcRepInfo.LinkMargin));
		}
	}

	return;
}


/*
	==========================================================================
	Description:
		Spectrun action frames Handler such as channel switch annoucement,
		measurement report, measurement request actions frames.
		
	Parametrs:
		Elme - MLME message containing the received frame
	
	Return	: None.
	==========================================================================
 */
VOID PeerSpectrumAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{

	UCHAR	Action = Elem->Msg[LENGTH_802_11+1];

	if (pAd->CommonCfg.bIEEE80211H != TRUE)
		return;

	switch(Action)
	{
		case SPEC_MRQ:
			/* current rt2860 unable do such measure specified in Measurement Request.*/
			/* reject all measurement request.*/
			PeerMeasureReqAction(pAd, Elem);
			break;

		case SPEC_MRP:
			PeerMeasureReportAction(pAd, Elem);
			break;

		case SPEC_TPCRQ:
			PeerTpcReqAction(pAd, Elem);
			break;

		case SPEC_TPCRP:
			PeerTpcRepAction(pAd, Elem);
			break;

		case SPEC_CHANNEL_SWITCH:

#ifdef DOT11N_DRAFT3
			{
				SEC_CHA_OFFSET_IE	Secondary;
				CHA_SWITCH_ANNOUNCE_IE	ChannelSwitch;

				/* 802.11h only has Channel Switch Announcement IE. */
				RTMPMoveMemory(&ChannelSwitch, &Elem->Msg[LENGTH_802_11+4], sizeof (CHA_SWITCH_ANNOUNCE_IE));
					
				/* 802.11n D3.03 adds secondary channel offset element in the end.*/
				if (Elem->MsgLen ==  (LENGTH_802_11 + 2 + sizeof (CHA_SWITCH_ANNOUNCE_IE) + sizeof (SEC_CHA_OFFSET_IE)))
				{
					RTMPMoveMemory(&Secondary, &Elem->Msg[LENGTH_802_11+9], sizeof (SEC_CHA_OFFSET_IE));
				}
				else
				{
					Secondary.SecondaryChannelOffset = 0;
				}

				if ((Elem->Msg[LENGTH_802_11+2] == IE_CHANNEL_SWITCH_ANNOUNCEMENT) && (Elem->Msg[LENGTH_802_11+3] == 3))
				{
					ChannelSwitchAction(pAd, Elem->Wcid, ChannelSwitch.NewChannel, Secondary.SecondaryChannelOffset);
				}
			}
#endif /* DOT11N_DRAFT3 */

			PeerChSwAnnAction(pAd, Elem);
			break;
	}

	return;
}

/*
	==========================================================================
	Description:
		
	Parametrs:
	
	Return	: None.
	==========================================================================
 */
INT Set_MeasureReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT wcid = 1;
	UINT ArgIdx;
	RTMP_STRING *thisChar;

	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	UINT8 MeasureReqType = RM_BASIC;
	UINT8 MeasureCh = 1;
	UINT64 MeasureStartTime = GetCurrentTimeStamp(pAd);
	MEASURE_REQ MeasureReq;
	//UINT8 TotalLen;

	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		goto END_OF_MEASURE_REQ;
	}

	ArgIdx = 1;
	while ((thisChar = strsep((char **)&arg, "-")) != NULL)
	{
		switch(ArgIdx)
		{
			case 1:	/* Aid.*/
				wcid = (UINT8) simple_strtol(thisChar, 0, 16);
				break;

			case 2: /* Measurement Request Type.*/
				MeasureReqType = simple_strtol(thisChar, 0, 16);
				if (MeasureReqType > 3)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s: unknow MeasureReqType(%d)\n", __FUNCTION__, MeasureReqType));
					goto END_OF_MEASURE_REQ;
				}
				break;

			case 3: /* Measurement channel.*/
				MeasureCh = (UINT8) simple_strtol(thisChar, 0, 16);
				break;
		}
		ArgIdx++;
	}	

	DBGPRINT(RT_DEBUG_TRACE, ("%s::Aid = %d, MeasureReqType=%d MeasureCh=%d\n",
				__FUNCTION__, wcid, MeasureReqType, MeasureCh));
	if (!VALID_WCID(wcid))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, wcid));
		goto END_OF_MEASURE_REQ;
	}

	MeasureReqMode.word = 0;
	MeasureReqMode.field.Enable = 1;

	MeasureReqInsert(pAd, MeasureReqToken);

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[wcid].Addr,
						pAd->CurrentAddress,
						pAd->CurrentAddress);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	//TotalLen = sizeof(MEASURE_REQ_INFO) + sizeof(MEASURE_REQ);

	MakeMeasurementReqFrame(pAd, pOutBuffer, &FrameLen,
		sizeof(MEASURE_REQ_INFO), CATEGORY_RM, RM_BASIC,
		MeasureReqToken, MeasureReqMode.word,
		MeasureReqType, 1);

	MeasureReq.ChNum = MeasureCh;
	MeasureReq.MeasureStartTime = cpu2le64(MeasureStartTime);
	MeasureReq.MeasureDuration = cpu2le16(2000);

	{
		ULONG TempLen;
		MakeOutgoingFrame(	pOutBuffer+FrameLen,	&TempLen,
							sizeof(MEASURE_REQ),	&MeasureReq,
							END_OF_ARGS);
		FrameLen += TempLen;
	}

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, (UINT)FrameLen);

END_OF_MEASURE_REQ:
	MlmeFreeMemory(pAd, pOutBuffer);

	return TRUE;
}

INT Set_TpcReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT wcid;

	UINT8 TpcReqToken = RandomByte(pAd);

	wcid = (UINT) simple_strtol(arg, 0, 16);

	DBGPRINT(RT_DEBUG_TRACE, ("%s::wcid = %d\n", __FUNCTION__, wcid));
	if (!VALID_WCID(wcid))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: unknow sta of Aid(%d)\n", __FUNCTION__, wcid));
		return TRUE;
	}

	TpcReqInsert(pAd, TpcReqToken);

	EnqueueTPCReq(pAd, pAd->MacTab.Content[wcid].Addr, TpcReqToken);

	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
INT Set_PwrConstraint(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

typedef struct __PWR_CONSTRAIN_CFG
{
	CHAR Attenuation;
	ULONG TxPowerPercentage;	
} PWR_CONSTRAIN_CFG;

	PWR_CONSTRAIN_CFG PwrConstrainTab[] =
	{
		{0, 100},
		{1, 70},
		{4, 50},
		{6, 20},
		{10, 10},
		{13, 5}
	};
#define PWR_CONSTRAION_TAB_SIZE \
	(sizeof(PwrConstrainTab)/sizeof(PWR_CONSTRAIN_CFG))

	INT Idx;
	LONG Value;
	CHAR MaxTxPwr;
	CHAR CurTxPwr;
	CHAR DaltaPwr;

	Value = (UINT) simple_strtol(arg, 0, 10);
	MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, pAd->CommonCfg.Channel) - (CHAR)Value;
	CurTxPwr = RTMP_GetTxPwr(pAd, pAd->MacTab.Content[0].HTPhyMode);
	DaltaPwr = CurTxPwr - MaxTxPwr;

	if (pAd->CommonCfg.TxPowerPercentage > 90)
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60)	/* reduce Pwr for 1 dB. */
		DaltaPwr += 1;
	else if (pAd->CommonCfg.TxPowerPercentage > 30)	/* reduce Pwr for 3 dB. */
		DaltaPwr += 3;
	else if (pAd->CommonCfg.TxPowerPercentage > 15)	/* reduce Pwr for 6 dB. */
		DaltaPwr += 6;
	else if (pAd->CommonCfg.TxPowerPercentage > 9)	/* reduce Pwr for 9 dB. */
		DaltaPwr += 9;
	else											/* reduce Pwr for 12 dB. */
		DaltaPwr += 12;

	DBGPRINT(RT_DEBUG_OFF, ("MaxTxPwr=%d, CurTxPwr=%d, DaltaPwr=%d\n",
				MaxTxPwr, CurTxPwr, DaltaPwr));

	for (Idx = 0; Idx < PWR_CONSTRAION_TAB_SIZE; Idx++)
	{
		if (DaltaPwr < PwrConstrainTab[Idx].Attenuation)
		{
			pAd->CommonCfg.PwrConstraint = Value;
			pAd->CommonCfg.TxPowerPercentage =
					PwrConstrainTab[Idx].TxPowerPercentage;

			break;
		}
	}

	if (Idx == PWR_CONSTRAION_TAB_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, \
			("Power constraint value be in range from 0 to 13dB\n"));
	}


	return TRUE;
}

#ifdef DOT11K_RRM_SUPPORT
#endif /* DOT11K_RRM_SUPPORT */


static DOT11_REGULATORY_INFO *GetRugClassRegion(RTMP_STRING *pCountryCode, UINT8 RugClass)
{
	DOT11_REGULATORY_INFO *pRugClass;

	pRugClass = NULL;
	do
	{
		if (strncmp(pCountryCode, "US", 2) == 0)
		{
			if (RugClass >= USA_REGULATORY_INFO_SIZE)
				break;
			pRugClass = &USARegulatoryInfo[RugClass];
		}

		if (strncmp(pCountryCode, "JP", 2) == 0)
		{
			if (RugClass >= JP_REGULATORY_INFO_SIZE)
				break;
			pRugClass = &JapanRegulatoryInfo[RugClass];

		}
	} while (FALSE);

	return pRugClass;
}


VOID RguClass_BuildBcnChList(RTMP_ADAPTER *pAd, UCHAR *pBuf, ULONG *pBufLen)
{
	INT loop;
	ULONG TmpLen;
	DOT11_REGULATORY_INFO *pRguClassRegion;
	DOT11_CHANNEL_SET *pChList;

	for (loop = 0 ;loop < MAX_NUM_OF_REGULATORY_CLASS; loop++)
	{
		if (pAd->CommonCfg.RegulatoryClass[loop] == 0)
			break;

		pRguClassRegion = GetRugClassRegion(
							(RTMP_STRING *)pAd->CommonCfg.CountryCode,
							pAd->CommonCfg.RegulatoryClass[loop]);

		pChList = &pRguClassRegion->ChannelSet;

		if (pRguClassRegion == NULL)
			return;

		MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
							1,                 	&pChList->ChannelList[0],
							1,                 	&pChList->NumberOfChannels,
							1,                 	&pChList->MaxTxPwr,
							END_OF_ARGS);

		*pBufLen += TmpLen;
	}

	return;
}
#endif /* CONFIG_AP_SUPPORT */

