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
	Fonchi Wu    2008		   created for 802.11h
 */

#include "rt_config.h"
#include "action.h"

UINT8 GetRegulatoryMaxTxPwr(RTMP_ADAPTER *pAd, UINT8 channel)
{
	ULONG ChIdx;
	PCH_REGION pChRegion = NULL;
	UCHAR FirstChannelIdx, NumCh;
	UCHAR increment = 0, index = 0, ChannelIdx = 0;
	/* Get Channel Region (CountryCode)*/
	pChRegion = GetChRegion(pAd->CommonCfg.CountryCode);

	if (!pChRegion || !pChRegion->pChDesp) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): pChRegion is NULL\n", __func__));
		return 0xff;
	}

	FirstChannelIdx = pChRegion->pChDesp->FirstChannel;
	NumCh = pChRegion->pChDesp->NumOfCh;

	while (FirstChannelIdx != 0) {
		ChannelIdx = FirstChannelIdx;

		for (ChIdx = 0; ChIdx < NumCh; ChIdx++) {
			if (FirstChannelIdx > channel)
				break;

			/* check increment */
			if (ChannelIdx > 14)
				increment = 4;
			else
				increment = 1;

			if (channel == ChannelIdx)
				return pChRegion->pChDesp[index].MaxTxPwr;

			ChannelIdx += increment;
		}

		index++;
		FirstChannelIdx = pChRegion->pChDesp[index].FirstChannel;
		NumCh = pChRegion->pChDesp[index].NumOfCh;
	}

	return 0xff;
}

typedef struct __TX_PWR_CFG {
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
	, {MODE_HTMIX, 0, 1, 20, 0x00f00000},
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


CHAR RTMP_GetTxPwr(RTMP_ADAPTER *pAd, HTTRANSMIT_SETTING HTTxMode, UCHAR Channel, struct wifi_dev *wdev)
{
	UINT32 Value;
	INT Idx;
	UINT8 PhyMode;
	CHAR CurTxPwr;
	UINT8 TxPwrRef = 0;
	CHAR DaltaPwr;
	ULONG TxPwr[5];
	UCHAR cen_ch = wlan_operate_get_cen_ch_1(wdev);
	UCHAR bw = wlan_operate_get_bw(wdev);
	CurTxPwr = 19;

	/* check Tx Power setting from UI. */
	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90)
		;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60)  /* reduce Pwr for 1 dB. */
		CurTxPwr -= 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30)  /* reduce Pwr for 3 dB. */
		CurTxPwr -= 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15)  /* reduce Pwr for 6 dB. */
		CurTxPwr -= 6;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 9)   /* reduce Pwr for 9 dB. */
		CurTxPwr -= 9;
	else                                           /* reduce Pwr for 12 dB. */
		CurTxPwr -= 12;

	if (bw == BW_40) {
		if (cen_ch > 14) {
			TxPwr[0] = pAd->Tx40MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgABand[4];
		} else {
			TxPwr[0] = pAd->Tx40MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx40MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx40MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx40MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx40MPwrCfgGBand[4];
		}
	} else {
		if (Channel > 14) {
			TxPwr[0] = pAd->Tx20MPwrCfgABand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgABand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgABand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgABand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgABand[4];
		} else {
			TxPwr[0] = pAd->Tx20MPwrCfgGBand[0];
			TxPwr[1] = pAd->Tx20MPwrCfgGBand[1];
			TxPwr[2] = pAd->Tx20MPwrCfgGBand[2];
			TxPwr[3] = pAd->Tx20MPwrCfgGBand[3];
			TxPwr[4] = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	switch (HTTxMode.field.MODE) {
	case MODE_CCK:
	case MODE_OFDM:
		Value = TxPwr[1];
		TxPwrRef = (Value & 0x00000f00) >> 8;
		break;
#ifdef DOT11_N_SUPPORT

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		if (pAd->Antenna.field.TxPath == 1) {
			Value = TxPwr[2];
			TxPwrRef = (Value & 0x00000f00) >> 8;
		} else if (pAd->Antenna.field.TxPath == 2) {
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

	for (Idx = 0; Idx < MAX_TXPWR_TAB_SIZE; Idx++) {
		if ((TxPwrCfg[Idx].Mode == PhyMode)
			&& (TxPwrCfg[Idx].MCS == HTTxMode.field.MCS)) {
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

	os_alloc_mem(pAd, (UCHAR **) &(pAd->CommonCfg.pMeasureReqTab), sizeof(MEASURE_REQ_TAB));

	if (pAd->CommonCfg.pMeasureReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pMeasureReqTab, sizeof(MEASURE_REQ_TAB));
		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.MeasureReqTabLock);
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Fail to alloc memory for pAd->CommonCfg.pMeasureReqTab.\n",
				 __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID MeasureReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pMeasureReqTab) {
		os_free_mem(pAd->CommonCfg.pMeasureReqTab);
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
	PMEASURE_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __func__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);
	HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry) {
		if (pEntry->DialogToken == DialogToken)
			break;
		else {
			pPrevEntry = pEntry;
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

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __func__));
		return NULL;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken);

	if (pEntry == NULL) {
		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);

		for (i = 0; i < MAX_MEASURE_REQ_TAB_SIZE; i++) {
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];

			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now,
				(unsigned long)(pEntry->lastTime + MQ_REQ_AGE_OUT))
#ifdef CONFIG_11KV_API_SUPPORT
				&& pEntry->skip_time_check == FALSE
#endif /* CONFIG_11KV_API_SUPPORT */
				) {
				PMEASURE_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

				/* update Hash list*/
				do {
					if (pProbeEntry == pEntry) {
						if (pPrevEntry == NULL)
							pTab->Hash[HashIdx] = pEntry->pNext;
						else
							pPrevEntry->pNext = pEntry->pNext;

						break;
					}

					pPrevEntry = pProbeEntry;
					pProbeEntry = pProbeEntry->pNext;
				} while (pProbeEntry);

				NdisZeroMemory(pEntry, sizeof(MEASURE_REQ_ENTRY));
				pTab->Size--;
				break;
			}

			if (pEntry->Valid == FALSE)
				break;
		}

		if (i < MAX_MEASURE_REQ_TAB_SIZE) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
#ifdef CONFIG_11KV_API_SUPPORT
			pEntry->skip_time_check = FALSE;
#endif /* CONFIG_11KV_API_SUPPORT */
			pTab->Size++;
		} else {
			pEntry = NULL;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMeasureReqTab tab full.\n", __func__));
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry) {
			HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(DialogToken);

			if (pTab->Hash[HashIdx] == NULL)
				pTab->Hash[HashIdx] = pEntry;
			else {
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

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pMeasureReqTab doesn't exist.\n", __func__));
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pMeasureReqTab empty.\n"));
		return;
	}

	pEntry = MeasureReqLookUp(pAd, DialogToken);

	if (pEntry != NULL) {
		PMEASURE_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = MQ_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PMEASURE_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.MeasureReqTabLock);

		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pTab->Hash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

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

	os_alloc_mem(pAd, (UCHAR **) &(pAd->CommonCfg.pTpcReqTab), sizeof(TPC_REQ_TAB));

	if (pAd->CommonCfg.pTpcReqTab) {
		NdisZeroMemory(pAd->CommonCfg.pTpcReqTab, sizeof(TPC_REQ_TAB));
		NdisAllocateSpinLock(pAd, &pAd->CommonCfg.TpcReqTabLock);
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Fail to alloc memory for pAd->CommonCfg.pTpcReqTab.\n",
				 __func__));
		Status = NDIS_STATUS_FAILURE;
	}

	return Status;
}


VOID TpcReqTabExit(RTMP_ADAPTER *pAd)
{
	if (pAd->CommonCfg.pTpcReqTab) {
		os_free_mem(pAd->CommonCfg.pTpcReqTab);
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
	PTPC_REQ_ENTRY pPrevEntry = NULL;

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __func__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);
	HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);
	pEntry = pTab->Hash[HashIdx];

	while (pEntry) {
		if (pEntry->DialogToken == DialogToken)
			break;
		else {
			pPrevEntry = pEntry;
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

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __func__));
		return NULL;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);

	if (pEntry == NULL) {
		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);

		for (i = 0; i < MAX_TPC_REQ_TAB_SIZE; i++) {
			NdisGetSystemUpTime(&Now);
			pEntry = &pTab->Content[i];

			if ((pEntry->Valid == TRUE)
				&& RTMP_TIME_AFTER((unsigned long)Now, (unsigned long)(pEntry->lastTime + TPC_REQ_AGE_OUT))) {
				PTPC_REQ_ENTRY pPrevEntry = NULL;
				ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
				PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

				/* update Hash list*/
				do {
					if (pProbeEntry == pEntry) {
						if (pPrevEntry == NULL)
							pTab->Hash[HashIdx] = pEntry->pNext;
						else
							pPrevEntry->pNext = pEntry->pNext;

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

		if (i < MAX_TPC_REQ_TAB_SIZE) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			pEntry->Valid = TRUE;
			pEntry->DialogToken = DialogToken;
			pTab->Size++;
		} else {
			pEntry = NULL;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pTpcReqTab tab full.\n", __func__));
		}

		/* add this Neighbor entry into HASH table*/
		if (pEntry) {
			HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(DialogToken);

			if (pTab->Hash[HashIdx] == NULL)
				pTab->Hash[HashIdx] = pEntry;
			else {
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

#ifdef TPC_SUPPORT
static VOID TpcReqDelete(RTMP_ADAPTER *pAd, UINT8 DialogToken)
{
	PTPC_REQ_TAB pTab = pAd->CommonCfg.pTpcReqTab;
	PTPC_REQ_ENTRY pEntry = NULL;

	if (pTab == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pTpcReqTab doesn't exist.\n", __func__));
		return;
	}

	/* if empty, return*/
	if (pTab->Size == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pTpcReqTab empty.\n"));
		return;
	}

	pEntry = TpcReqLookUp(pAd, DialogToken);

	if (pEntry != NULL) {
		PTPC_REQ_ENTRY pPrevEntry = NULL;
		ULONG HashIdx = TPC_DIALOGTOKEN_HASH_INDEX(pEntry->DialogToken);
		PTPC_REQ_ENTRY pProbeEntry = pTab->Hash[HashIdx];

		RTMP_SEM_LOCK(&pAd->CommonCfg.TpcReqTabLock);

		/* update Hash list*/
		do {
			if (pProbeEntry == pEntry) {
				if (pPrevEntry == NULL)
					pTab->Hash[HashIdx] = pEntry->pNext;
				else
					pPrevEntry->pNext = pEntry->pNext;

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
#endif /* TPC_SUPPORT */


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
		Get Maximum Transmit Power.

	Parametrs:

	Return	: Current Transmit Power.
	==========================================================================
 */
UINT8 GetMaxTxPwr(RTMP_ADAPTER *pAd)
{
#ifdef SINGLE_SKU_V2
	UINT8 max = 0;
	UINT8 i;

	for (i = 0; i < SKU_SIZE; i++) {
		if (pAd->TxPowerSKU[i] > max)
			max = pAd->TxPowerSKU[i];
	}

	return max;
#else
	return 0x3f;	/* dBm */
#endif
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
	IN UINT8 *ChReptList,
	IN UCHAR PhyMode
)
{
	ULONG TempLen;
	UINT8 Len;
	UINT8 IEId = IE_AP_CHANNEL_REPORT;
	PUCHAR pChListPtr = NULL;
	UINT8 i, j;
	UCHAR ChannelList[16] = {0};
	UINT8 NumberOfChannels = 0;
	UINT8 *pChannelList = NULL;
	PUCHAR channel_set = NULL;
	UCHAR channel_set_num;
	UCHAR ch_list_num = 0;

	if (RegulatoryClass == 0)
		return;

	Len = 1;
	channel_set = get_channelset_by_reg_class(pAd, RegulatoryClass, PhyMode);
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

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: Requlatory class (%d), NumberOfChannels=%d, channel_set_num=%d\n",
			  __func__, RegulatoryClass, NumberOfChannels, channel_set_num));
	Len += NumberOfChannels;
	pChListPtr = pChannelList;

	if (Len > 1) {
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
						  1,				&IEId,
						  1,				&Len,
						  1,				&RegulatoryClass,
						  Len - 1,			pChListPtr,
						  END_OF_ARGS);
		*pFrameLen = *pFrameLen + TempLen;
	}

	return;
}


/*
	==========================================================================
	Description:
		Add last beacon report indication request into beacon request

	Parametrs:

	Return	: NAN
	==========================================================================
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

	return;
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

	if ((ReportLnfoLen > 0) && (pReportInfo != NULL)) {
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
	UINT16 leRepetitions = cpu2le16(NumOfRepetitions);

	/*printk("NumOfRepetitions=0x%x, leRepetitions=0x%x,", NumOfRepetitions, leRepetitions);*/
	InsertActField(pAd, (pOutBuffer + *pFrameLen), pFrameLen, Category, Action);
	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + *pFrameLen), pFrameLen, MeasureToken);
#ifdef RT_BIG_ENDIAN
	NumOfRepetitions = cpu2le16(NumOfRepetitions);
#endif

	/* fill Number of repetitions. */
	if (Category == CATEGORY_RM) {
		MakeOutgoingFrame((pOutBuffer + *pFrameLen),	&TempLen,
						  2,							&leRepetitions,
						  END_OF_ARGS);
		*pFrameLen += TempLen;
	}

	/* prepare Measurement IE.*/
	NdisZeroMemory(&MeasureReqIE, sizeof(MEASURE_REQ_INFO));
	MeasureReqIE.Token = MeasureToken;
	MeasureReqIE.ReqMode.word = MeasureReqMode;
	MeasureReqIE.ReqType = MeasureReqType;
	InsertMeasureReqIE(pAd, (pOutBuffer + *pFrameLen), pFrameLen,
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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
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
	MlmeFreeMemory(pOutBuffer);
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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
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
	MlmeFreeMemory(pOutBuffer);
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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
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
	MlmeFreeMemory(pOutBuffer);
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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return;
	}

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	InsertChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, 0);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return;
}
#endif /* WDS_SUPPORT */


static BOOLEAN DfsRequirementCheck(RTMP_ADAPTER *pAd, UINT8 Channel)
{
	BOOLEAN Result = FALSE;
	INT i;
	UCHAR BandIdx = HcGetBandByChannel(pAd, Channel);
	CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

	do {
		/* check DFS procedure is running.*/
		/* make sure DFS procedure won't start twice.*/
		if (pAd->Dot11_H[0].RDMode != RD_NORMAL_MODE) {
			Result = FALSE;
			break;
		}

		/* check the new channel carried from Channel Switch Announcemnet is valid.*/
		for (i = 0; i < pChCtrl->ChListNum; i++) {
			if ((Channel == pChCtrl->ChList[i].Channel)
				&& (pChCtrl->ChList[i].RemainingTimeForUse == 0)) {
				/* found radar signal in the channel. the channel can't use at least for 30 minutes.*/
				pChCtrl->ChList[i].RemainingTimeForUse = 1800;/*30 min = 1800 sec*/
				Result = TRUE;
				break;
			}
		}
	} while (FALSE);

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

	if (!((pRA[0] & 0xff) == 0xff)) { /* is pRA a broadcase address.*/
		INT i;

		/* info neighbor APs that Radar signal found throgh WDS link.*/
		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			if (ValidWdsEntry(pAd, i)) {
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
	pAd->Dot11_H[0].RDMode = RD_SWITCHING_MODE;
	pAd->Dot11_H[0].CSCount = 0;
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

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			NdisMoveMemory(&pChSwAnnInfo->ChSwMode, eid_ptr->Octet, 1);
			NdisMoveMemory(&pChSwAnnInfo->Channel, eid_ptr->Octet + 1, 1);
			NdisMoveMemory(&pChSwAnnInfo->ChSwCnt, eid_ptr->Octet + 2, 1);
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
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

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
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

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
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

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_MEASUREMENT_REPORT:
			NdisMoveMemory(&pMeasureReportInfo->Token, eid_ptr->Octet, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportMode, eid_ptr->Octet + 1, 1);
			NdisMoveMemory(&pMeasureReportInfo->ReportType, eid_ptr->Octet + 2, 1);

			if (pMeasureReportInfo->ReportType == RM_BASIC) {
				PMEASURE_BASIC_REPORT pReport = (PMEASURE_BASIC_REPORT)pReportBuf;

				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pReport->ChNum, ptr, 1);
				NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
				NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
				NdisMoveMemory(&pReport->Map, ptr + 11, 1);
			} else if (pMeasureReportInfo->ReportType == RM_CCA) {
				PMEASURE_CCA_REPORT pReport = (PMEASURE_CCA_REPORT)pReportBuf;

				ptr = (PUCHAR)(eid_ptr->Octet + 3);
				NdisMoveMemory(&pReport->ChNum, ptr, 1);
				NdisMoveMemory(&pReport->MeasureStartTime, ptr + 1, 8);
				NdisMoveMemory(&pReport->MeasureDuration, ptr + 9, 2);
				NdisMoveMemory(&pReport->CCA_Busy_Fraction, ptr + 11, 1);
			} else if (pMeasureReportInfo->ReportType == RM_RPI_HISTOGRAM) {
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

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}


#ifdef TPC_SUPPORT
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

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_TPC_REQUEST:
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
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

	while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((PUCHAR)pFramePtr + MsgLen)) {
		switch (eid_ptr->Eid) {
		case IE_TPC_REPORT:
			NdisMoveMemory(&pTpcRepInfo->TxPwr, eid_ptr->Octet, 1);
			NdisMoveMemory(&pTpcRepInfo->LinkMargin, eid_ptr->Octet + 1, 1);
			result = TRUE;
			break;

		default:
			break;
		}

		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

	return result;
}
#endif /* TPC_SUPPORT */


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

	if (!PeerChSwAnnSanity(pAd, Elem->Msg, Elem->MsgLen, &ChSwAnnInfo)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Invalid Channel Switch Action Frame.\n"));
		return;
	}

#ifdef CONFIG_AP_SUPPORT

	/* ChSwAnn need check.*/
	if ((pAd->OpMode == OPMODE_AP) &&
		(DfsRequirementCheck(pAd, ChSwAnnInfo.Channel) == TRUE)) {
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

	if (PeerMeasureReqSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReqInfo, &MeasureReq)) {
		ReportMode.word = 0;
		ReportMode.field.Incapable = 1;
		EnqueueMeasurementRep(pAd, pFr->Hdr.Addr2, DialogToken, MeasureReqInfo.Token, ReportMode.word, MeasureReqInfo.ReqType,
							  0, NULL);
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

	if (pMeasureReportInfo == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s unable to alloc memory for measure report buffer (size=%zu).\n", __func__, sizeof(MEASURE_RPI_REPORT)));
		return;
	}

	NdisZeroMemory(&MeasureReportInfo, sizeof(MEASURE_REPORT_INFO));
	NdisZeroMemory(pMeasureReportInfo, sizeof(MEASURE_RPI_REPORT));

	if (PeerMeasureReportSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &MeasureReportInfo, pMeasureReportInfo)) {
		do {
			PMEASURE_REQ_ENTRY pEntry = NULL;

			pEntry = MeasureReqLookUp(pAd, DialogToken);

			/* Not a autonomous measure report.*/
			/* check the dialog token field. drop it if the dialog token doesn't match.*/
			if ((DialogToken != 0)
				&& (pEntry == NULL))
				break;

			if (pEntry != NULL)
				MeasureReqDelete(pAd, pEntry->DialogToken);

			if (MeasureReportInfo.ReportType == RM_BASIC) {
				PMEASURE_BASIC_REPORT pBasicReport = (PMEASURE_BASIC_REPORT)pMeasureReportInfo;

				if ((pBasicReport->Map.field.Radar)
					&& (DfsRequirementCheck(pAd, pBasicReport->ChNum) == TRUE)) {
					NotifyChSwAnnToPeerAPs(pAd, pFr->Hdr.Addr1, pFr->Hdr.Addr2, 1, pBasicReport->ChNum);
					StartDFSProcedure(pAd, pBasicReport->ChNum, 1);
				}
			}
		} while (FALSE);
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Invalid Measurement Report Frame.\n"));

	os_free_mem(pMeasureReportInfo);
	return;
}


#ifdef TPC_SUPPORT
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
	UINT8 TxPwr = GetMaxTxPwr(pAd);
	UINT8 LinkMargin = 0;
	CHAR RealRssi;

	if (!pAd->CommonCfg.b80211TPC) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): (X) b80211TPC=%d\n",
				 __func__, pAd->CommonCfg.b80211TPC));
		return;
	}

	/* link margin: Ratio of the received signal power to the minimum desired by the station (STA). The*/
	/*				STA may incorporate rate information and channel conditions, including interference, into its computation*/
	/*				of link margin.*/
	RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
						   ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2)
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
							, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_3)
#endif
						   );
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
	const UINT8 MAX_PWR_LIMIT_DBM = (0x3F >> 1);
	/* Register: signed, only 7 bits for positive integer. Change the unit from 0.5 dBm to 1 dBm */
	UINT8 DialogToken;
	TPC_REPORT_INFO TpcRepInfo;
	PTPC_REQ_ENTRY pEntry = NULL;
	BOOLEAN bUpdated = TRUE;
	PFRAME_802_11 pFr = (PFRAME_802_11)Elem->Msg;
	INT MaxTxPower = 0;

	if (!pAd->CommonCfg.b80211TPC) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): (X) b80211TPC=%d\n",
				 __func__, pAd->CommonCfg.b80211TPC));
		return;
	}

	NdisZeroMemory(&TpcRepInfo, sizeof(TPC_REPORT_INFO));

	if (PeerTpcRepSanity(pAd, Elem->Msg, Elem->MsgLen, &DialogToken, &TpcRepInfo)) {
		pEntry = TpcReqLookUp(pAd, DialogToken);

		if (pEntry != NULL) {
			TpcReqDelete(pAd, pEntry->DialogToken);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: DialogToken=%x, TxPwr=%d, LinkMargin=%d\n",
					 __func__, DialogToken, TpcRepInfo.TxPwr, TpcRepInfo.LinkMargin));
		}
	}

	MaxTxPower = TpcRepInfo.TxPwr - TpcRepInfo.LinkMargin;

	if (MaxTxPower > MAX_PWR_LIMIT_DBM)
		MaxTxPower = MAX_PWR_LIMIT_DBM;

	MaxTxPower <<= 1;
	/* unit: 0.5 dBm in hardware register */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: MaxTxPower = %d (unit: 0.5 dBm)\n",
			 __func__, MaxTxPower));

	if (bUpdated) {
		struct wifi_dev *wdev = NULL;
		MAC_TABLE_ENTRY *mac_entry = MacTableLookup(pAd, pFr->Hdr.Addr2);

		if (mac_entry)
			wdev = mac_entry->wdev;
		else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;

		TxPowerTpcFeatureCtrl(pAd, wdev, (INT8)MaxTxPower);
	}

	return;
}
#endif /* TPC_SUPPORT */


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
	UCHAR	Action = Elem->Msg[LENGTH_802_11 + 1];

	if (pAd->CommonCfg.bIEEE80211H != TRUE)
		return;

	switch (Action) {
	case SPEC_MRQ:
		/* current rt2860 unable do such measure specified in Measurement Request.*/
		/* reject all measurement request.*/
		PeerMeasureReqAction(pAd, Elem);
		break;

	case SPEC_MRP:
		PeerMeasureReportAction(pAd, Elem);
		break;
#ifdef TPC_SUPPORT

	case SPEC_TPCRQ:
		PeerTpcReqAction(pAd, Elem);
		break;

	case SPEC_TPCRP:
		PeerTpcRepAction(pAd, Elem);
		break;
#endif /* TPC_SUPPORT */

	case SPEC_CHANNEL_SWITCH:
#ifdef CONFIG_RCSA_SUPPORT
		if (pAd->CommonCfg.DfsParameter.bRCSAEn) {
			CSA_IE_INFO CsaInfo = {0};
			struct wifi_dev *wdev = pAd->MacTab.Content[Elem->Wcid].wdev;
			struct DOT11_H *pDot11h = wdev->pDot11_H;

			if (ApCliPeerCsaSanity(Elem, &CsaInfo) == FALSE)
				return;

			CsaInfo.wcid = Elem->Wcid;
			if (pAd->CommonCfg.DfsParameter.fUseCsaCfg == TRUE) {
				if (CsaInfo.ChSwAnnIE.ChSwCnt)
					pDot11h->CSPeriod = CsaInfo.ChSwAnnIE.ChSwCnt + 1;
				else if (CsaInfo.ExtChSwAnnIE.ChSwCnt)
					pDot11h->CSPeriod = CsaInfo.ExtChSwAnnIE.ChSwCnt + 1;
			}
			pAd->CommonCfg.DfsParameter.fSendRCSA = TRUE;
			ChannelSwitchAction_1(pAd, &CsaInfo);
		} else
#endif
		{
#ifdef DOT11N_DRAFT3
		{
			SEC_CHA_OFFSET_IE	Secondary;
			CHA_SWITCH_ANNOUNCE_IE	ChannelSwitch;
			/* 802.11h only has Channel Switch Announcement IE. */
			RTMPMoveMemory(&ChannelSwitch, &Elem->Msg[LENGTH_802_11 + 4], sizeof(CHA_SWITCH_ANNOUNCE_IE));

			/* 802.11n D3.03 adds secondary channel offset element in the end.*/
			if (Elem->MsgLen ==  (LENGTH_802_11 + 2 + sizeof(CHA_SWITCH_ANNOUNCE_IE) + sizeof(SEC_CHA_OFFSET_IE)))
				RTMPMoveMemory(&Secondary, &Elem->Msg[LENGTH_802_11 + 9], sizeof(SEC_CHA_OFFSET_IE));
			else
				Secondary.SecondaryChannelOffset = 0;

			if ((Elem->Msg[LENGTH_802_11 + 2] == IE_CHANNEL_SWITCH_ANNOUNCEMENT) && (Elem->Msg[LENGTH_802_11 + 3] == 3))
				ChannelSwitchAction(pAd, Elem->Wcid, ChannelSwitch.NewChannel, Secondary.SecondaryChannelOffset);
		}

#endif /* DOT11N_DRAFT3 */
		PeerChSwAnnAction(pAd, Elem);
		}
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
	UINT8 TotalLen;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		goto END_OF_MEASURE_REQ;
	}

	ArgIdx = 1;

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		switch (ArgIdx) {
		case 1:	/* Aid.*/
			wcid = (UINT8) os_str_tol(thisChar, 0, 16);
			break;

		case 2: /* Measurement Request Type.*/
			MeasureReqType = os_str_tol(thisChar, 0, 16);

			if (MeasureReqType > 3) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknow MeasureReqType(%d)\n", __func__, MeasureReqType));
				goto END_OF_MEASURE_REQ;
			}

			break;

		case 3: /* Measurement channel.*/
			MeasureCh = (UINT8) os_str_tol(thisChar, 0, 16);
			break;
		}

		ArgIdx++;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::Aid = %d, MeasureReqType=%d MeasureCh=%d\n",
			 __func__, wcid, MeasureReqType, MeasureCh));

	if (!VALID_WCID(wcid)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknow sta of Aid(%d)\n", __func__, wcid));
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
	TotalLen = sizeof(MEASURE_REQ_INFO) + sizeof(MEASURE_REQ);
	MakeMeasurementReqFrame(pAd, pOutBuffer, &FrameLen,
							sizeof(MEASURE_REQ_INFO), CATEGORY_RM, RM_BASIC,
							MeasureReqToken, MeasureReqMode.word,
							MeasureReqType, 1);
	MeasureReq.ChNum = MeasureCh;
	MeasureReq.MeasureStartTime = cpu2le64(MeasureStartTime);
	MeasureReq.MeasureDuration = cpu2le16(2000);
	{
		ULONG TempLen;

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  sizeof(MEASURE_REQ),	&MeasureReq,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, (UINT)FrameLen);
END_OF_MEASURE_REQ:
	MlmeFreeMemory(pOutBuffer);
	return TRUE;
}

INT Set_TpcReq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT wcid;
	UINT8 TpcReqToken = RandomByte(pAd);

	wcid = (UINT) os_str_tol(arg, 0, 16);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::wcid = %d\n", __func__, wcid));

	if (!VALID_WCID(wcid)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknow sta of Aid(%d)\n", __func__, wcid));
		return TRUE;
	}

	TpcReqInsert(pAd, TpcReqToken);
	EnqueueTPCReq(pAd, pAd->MacTab.Content[wcid].Addr, TpcReqToken);
	return TRUE;
}

INT Set_TpcReqByAddr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char *value;
	UINT8 macAddr[MAC_ADDR_LEN] = {0};
	INT i;
	UINT8 TpcReqToken = RandomByte(pAd);

	if (strlen(arg) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: TpcReqToken = %d (0x%02X)\n",
			  __func__, TpcReqToken, TpcReqToken));

	for (i = 0, value = rstrtok(arg, ":"); value; value = rstrtok(NULL, ":")) {
		if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
			return FALSE;  /*Invalid */

		AtoH(value, (UINT8 *)&macAddr[i++], 1);
	}

	TpcReqInsert(pAd, TpcReqToken);
	EnqueueTPCReq(pAd, macAddr, TpcReqToken);
	return TRUE;
}

#ifdef TPC_SUPPORT
INT Set_TpcCtrl_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	const UINT CMD_NUM = 3;
	UINT arg_len, i, j, cmd_pos[CMD_NUM];
	UINT BandIdx, Power, CentCh;

	arg_len = strlen(arg);
	cmd_pos[0] = 0;
	j = 1;

	for (i = 0; i  < arg_len; i++) {
		if (arg[i] == ':') {
			cmd_pos[j++] = i + 1;
			arg[i] = 0;
		}
	}

	if (j != CMD_NUM) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("usage format is [band:power:channel], power unit is 0.5 dBm\n\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("e.g.\n\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set TpcCtrl=0:62:6\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("means (band 0), (31 dBm), (channel 6)\n\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("iwpriv ra0 set TpcCtrl=1:10:100\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("means (band 1), (5 dBm), (channel 100)\n\n"));
		return TRUE;
	}

	BandIdx = os_str_tol(arg + cmd_pos[0], 0, 10);
	Power = os_str_tol(arg + cmd_pos[1], 0, 10);
	CentCh = os_str_tol(arg + cmd_pos[2], 0, 10);

	if (Power > 63)
		Power = 63;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("BandIdx=%d, Power=%d, CentCh=%d\n", BandIdx, Power, CentCh));
	TxPowerTpcFeatureForceCtrl(pAd, Power, BandIdx, CentCh);
	return TRUE;
}

INT Set_TpcEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable;

	enable = os_str_tol(arg, 0, 10);

	if (enable != FALSE)
		enable = TRUE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s(): %d -> %d\n", __func__, pAd->CommonCfg.b80211TPC, enable));
	pAd->CommonCfg.b80211TPC = enable;
	return TRUE;
}

#endif /* TPC_SUPPORT */

#ifdef CUSTOMER_DCC_FEATURE
#ifdef DOT11_N_SUPPORT
static VOID InsertSecondaryChOffsetIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Offset)
{
	ULONG TempLen;
	ULONG Len = sizeof(SEC_CHA_OFFSET_IE);
	UINT8 ElementID = IE_SECONDARY_CH_OFFSET;
	SEC_CHA_OFFSET_IE SChOffIE;

	SChOffIE.SecondaryChannelOffset = Offset;

	MakeOutgoingFrame(pFrameBuf,      &TempLen,
				1,      &ElementID,
				1,      &Len,
				Len,    &SChOffIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

#endif

VOID InsertChSwAnnIENew(
	IN PRTMP_ADAPTER pAd,
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

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
				1,	&ElementID,
				1,	&Len,
				Len,	&ChSwAnnIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	printk("%s \n", __func__);
	return;
}


INT NotifyChSwAnnToConnectedSTAs(
	IN PRTMP_ADAPTER pAd,
	IN UINT8		ChSwMode,
	IN UINT8		Channel,
	struct wifi_dev *wdev)
{
	UINT32 i;
	MAC_TABLE_ENTRY *pEntry;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	pAd->CommonCfg.channelSwitch.CHSWMode = ChSwMode;
	pAd->CommonCfg.channelSwitch.CHSWCount = 0;
	pDot11h->CSPeriod = pAd->CommonCfg.channelSwitch.CHSWPeriod;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC)) {
			EnqueueChSwAnnNew(pAd, pEntry->Addr, ChSwMode, Channel, pEntry->bssid, wdev);

		}
	}

	if (HcUpdateCsaCntByChannel(pAd, Channel) != 0) {
		return FALSE;
	}
	return TRUE;
}

VOID EnqueueChSwAnnNew(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDA,
	IN UINT8 ChSwMode,
	IN UINT8 NewCh,
	IN PUCHAR pSA,
	struct wifi_dev *wdev)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	HEADER_802_11 ActHdr;

	/* build action frame header.*/
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA, pAd->CurrentAddress, pSA);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);

	InsertChSwAnnIENew(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, pAd->CommonCfg.channelSwitch.CHSWPeriod);

#ifdef DOT11_N_SUPPORT
	InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, HcGetExtCha(pAd, wdev->channel));
#endif

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	return;
}

#endif

#ifdef CONFIG_AP_SUPPORT
INT Set_PwrConstraint(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	typedef struct __PWR_CONSTRAIN_CFG {
		CHAR Attenuation;
		UINT8 ucTxPowerPercentage;
	} PWR_CONSTRAIN_CFG;
	PWR_CONSTRAIN_CFG PwrConstrainTab[] = {
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
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[0];

	Value = (UINT) os_str_tol(arg, 0, 10);
	MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, pEntry->wdev->channel) - (CHAR)Value;
	CurTxPwr = RTMP_GetTxPwr(pAd, pEntry->HTPhyMode, pEntry->wdev->channel, pEntry->wdev);
	DaltaPwr = CurTxPwr - MaxTxPwr;

	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90)
		DaltaPwr += 0;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60)  /* reduce Pwr for 1 dB. */
		DaltaPwr += 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30)  /* reduce Pwr for 3 dB. */
		DaltaPwr += 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15)  /* reduce Pwr for 6 dB. */
		DaltaPwr += 6;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 9)   /* reduce Pwr for 9 dB. */
		DaltaPwr += 9;
	else                                            /* reduce Pwr for 12 dB. */
		DaltaPwr += 12;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MaxTxPwr=%d, CurTxPwr=%d, DaltaPwr=%d\n",
			 MaxTxPwr, CurTxPwr, DaltaPwr));

	for (Idx = 0; Idx < PWR_CONSTRAION_TAB_SIZE; Idx++) {
		if (DaltaPwr < PwrConstrainTab[Idx].Attenuation) {
			pAd->CommonCfg.PwrConstraint = Value;
			pAd->CommonCfg.ucTxPowerPercentage[BAND0] = PwrConstrainTab[Idx].ucTxPowerPercentage;
			break;
		}
	}

	if (Idx == PWR_CONSTRAION_TAB_SIZE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, \
				 ("Power constraint value be in range from 0 to 13dB\n"));
	}

	return TRUE;
}

#ifdef DOT11K_RRM_SUPPORT
#endif /* DOT11K_RRM_SUPPORT */

VOID RguClass_BuildBcnChList(RTMP_ADAPTER *pAd, UCHAR *pBuf, ULONG *pBufLen, UCHAR PhyMode, UCHAR RegClass)
{
	/* INT loop; */
	ULONG TmpLen;
	PUCHAR channel_set = NULL;
	UCHAR channel_set_num, MaxTxPwr;
	UINT8 i, ChSetMinLimPwr;

	ChSetMinLimPwr = 0xff;

	if (RegClass == 0)
		return;

	channel_set = get_channelset_by_reg_class(pAd, RegClass, PhyMode);
	channel_set_num = get_channel_set_num(channel_set);

	/* no match channel set. */
	if (channel_set == NULL)
		return;

	/* empty channel set. */
	if (channel_set_num == 0)
		return;

	/*
		There is many channel which have different limit tx power
		we choose the minimum
	*/
	for (i = 0; i < channel_set_num; i++) {
		MaxTxPwr = GetRegulatoryMaxTxPwr(pAd, channel_set[i]);

		if (MaxTxPwr < ChSetMinLimPwr)
			ChSetMinLimPwr = MaxTxPwr;
	}

	MakeOutgoingFrame(pBuf + *pBufLen,		&TmpLen,
					  1,		&channel_set[0],
					  1,		&channel_set_num,
					  1,		&ChSetMinLimPwr,
					  END_OF_ARGS);
	*pBufLen += TmpLen;
	return;
}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_RCSA_SUPPORT

static VOID InsertExtChSwAnnIE(
	IN RTMP_ADAPTER * pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ChSwMode,
	IN UINT8 RegClass,
	IN UINT8 NewChannel,
	IN UINT8 ChSwCnt)
{
	ULONG TempLen;
	ULONG Len = sizeof(EXT_CH_SW_ANN_INFO);
	UINT8 ElementID = IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT;
	EXT_CH_SW_ANN_INFO ExtChSwAnnIE;

	ExtChSwAnnIE.ChSwMode = ChSwMode;
	ExtChSwAnnIE.RegClass = RegClass;
	ExtChSwAnnIE.Channel = NewChannel;
	ExtChSwAnnIE.ChSwCnt = ChSwCnt;
	MakeOutgoingFrame(pFrameBuf,				&TempLen,
					  1,						&ElementID,
					  1,						&Len,
					  Len,						&ExtChSwAnnIE,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

#ifdef DOT11_VHT_AC
static VOID InsertWideBWChSwitchIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 NewCh,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	ULONG TempLen;
	ULONG Len = sizeof(WIDE_BW_CH_SWITCH_ELEMENT);
	UINT8 ElementID = IE_WIDE_BW_CH_SWITCH;
	WIDE_BW_CH_SWITCH_ELEMENT wb_info = {0};
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
	UCHAR vht_bw = wlan_config_get_vht_bw(wdev);

	if (op_ht_bw == BW_40) {
		switch (vht_bw) {
		case VHT_BW_2040:
			wb_info.new_ch_width = 0;
		break;
		case VHT_BW_80:
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = vht_cent_ch_freq(NewCh, vht_bw);
			wb_info.center_freq_2 = 0;
		break;
		case VHT_BW_160:
#ifdef DOT11_VHT_R2
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = (vht_cent_ch_freq(wdev->channel, vht_bw) - 8);
			wb_info.center_freq_2 = vht_cent_ch_freq(wdev->channel, vht_bw);
#else
			wb_info.new_ch_width = 2;
			wb_info.center_freq_1 = vht_cent_ch_freq(wdev->channel, vht_bw);
#endif /* DOT11_VHT_R2 */
		break;
		case VHT_BW_8080:
#ifdef DOT11_VHT_R2
			wb_info.new_ch_width = 1;
			wb_info.center_freq_1 = vht_cent_ch_freq(wdev->channel, vht_bw);
			wb_info.center_freq_2 = wlan_operate_get_cen_ch_2(wdev);
#else
			wb_info.new_ch_width = 3;
			wb_info.center_freq_1 = vht_cent_ch_freq(wdev->channel, vht_bw);
			wb_info.center_freq_2 = wlan_operate_get_cen_ch_2(wdev);
#endif /* DOT11_VHT_R2 */
		break;
		}
		MakeOutgoingFrame(pFrameBuf,	&TempLen,
					1,	&ElementID,
					1,      &Len,
					Len,    &wb_info,
					END_OF_ARGS);

		*pFrameLen = *pFrameLen + TempLen;
	}
}
#endif

#ifdef DOT11_N_SUPPORT
static VOID InsertSecondaryChOffsetIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Offset)
{
	ULONG TempLen;
	ULONG Len = sizeof(SEC_CHA_OFFSET_IE);
	UINT8 ElementID = IE_SECONDARY_CH_OFFSET;
	SEC_CHA_OFFSET_IE SChOffIE;

	SChOffIE.SecondaryChannelOffset = Offset;

	MakeOutgoingFrame(pFrameBuf,	&TempLen,
				1,	&ElementID,
				1,	&Len,
				Len,	&SChOffIE,
				END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}
#endif

#ifdef APCLI_SUPPORT
VOID EnqueueChSwAnnApCli(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN UINT8 ifIndex,
	IN UINT8 NewCh,
	IN UINT8 ChSwMode)
{
	PUCHAR pOutBuffer = NULL;
	UCHAR ChSwCnt = 0, RegClass;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	HEADER_802_11 ActHdr;
	struct DOT11_H *pDot11h = wdev->pDot11_H;

	/* build action frame header.*/
	ApCliMgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid,
					 pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, ifIndex);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return;
	}

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() NULL wdev\n", __func__));
		return;
	}

	ChSwCnt = pDot11h->CSPeriod - pDot11h->CSCount - 1;

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, CATEGORY_SPECTRUM, SPEC_CHANNEL_SWITCH);
	InsertChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, NewCh, ChSwCnt);

#ifdef DOT11_N_SUPPORT
	InsertSecondaryChOffsetIE(pAd, (pOutBuffer + FrameLen), &FrameLen, wlan_config_get_ext_cha(wdev));

	RegClass = get_regulatory_class(pAd, NewCh, wdev->PhyMode, wdev);
	InsertExtChSwAnnIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ChSwMode, RegClass, NewCh, ChSwCnt);
#endif

#ifdef DOT11_VHT_AC
	InsertWideBWChSwitchIE(pAd, wdev, NewCh, (pOutBuffer + FrameLen), &FrameLen);
#endif

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) %s::MiniportMMRequest\n",
		ifIndex, __func__));
	MiniportMMRequest(pAd, AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: <--Exit\n", __func__));
}
#endif

INT NotifyChSwAnnToBackhaulAP(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UINT8 Channel,
	IN UINT8 ChSwMode)
{
	INT8 inf_idx;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:Channel = %d, ChSwMode = %d\n", __func__, Channel, ChSwMode));

	for (inf_idx = 0; inf_idx < MAX_APCLI_NUM; inf_idx++) {
		if (pAd->ApCfg.ApCliTab[inf_idx].wdev.channel == wdev->channel) {
			EnqueueChSwAnnApCli(pAd, wdev, inf_idx, Channel, ChSwMode);
			return TRUE;
		}
	}

	return FALSE;
}

INT ApCliPeerCsaSanity(
	MLME_QUEUE_ELEM * Elem,
	CSA_IE_INFO *CsaInfo)
{
	UCHAR action, IE_ID, Length = 0, status = FALSE;

	action = Elem->Msg[LENGTH_802_11 + 1];

	if (action != SPEC_CHANNEL_SWITCH)
		return FALSE;

	Length = LENGTH_802_11 + 2;

	while (Length < Elem->MsgLen) {
		IE_ID = Elem->Msg[Length];

		switch (IE_ID) {
		case IE_CHANNEL_SWITCH_ANNOUNCEMENT:
			RTMPMoveMemory(&CsaInfo->ChSwAnnIE, &Elem->Msg[Length+2], sizeof(CH_SW_ANN_INFO));
			status = TRUE;
		break;

		case IE_SECONDARY_CH_OFFSET:
			CsaInfo->SChOffIE.SecondaryChannelOffset = Elem->Msg[Length+2];
		break;

		case IE_EXT_CHANNEL_SWITCH_ANNOUNCEMENT:
			RTMPMoveMemory(&CsaInfo->ExtChSwAnnIE, &Elem->Msg[Length+2], sizeof(EXT_CH_SW_ANN_INFO));
			status = TRUE;
		break;

		case IE_WIDE_BW_CH_SWITCH:
			RTMPMoveMemory(&CsaInfo->wb_info, &Elem->Msg[Length+2], sizeof(WIDE_BW_CH_SWITCH_ELEMENT));
		break;

		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Unknown IE=%d\n", __func__, IE_ID));
		break;
		}
		Length += Elem->Msg[Length+1] + 2;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Dump parsed CSA action frame --->\n", __func__));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CSA: Channel:%d ChSwMode:%d CsaCnt:%d\n",
		CsaInfo->ChSwAnnIE.Channel, CsaInfo->ChSwAnnIE.ChSwMode, CsaInfo->ChSwAnnIE.ChSwCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("SecChOffSet:%d\n", CsaInfo->SChOffIE.SecondaryChannelOffset));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ExtCSA: Channel:%d RegClass:%d ChSwMode:%d CsaCnt:%d\n",
		CsaInfo->ExtChSwAnnIE.Channel, CsaInfo->ExtChSwAnnIE.RegClass, CsaInfo->ExtChSwAnnIE.ChSwMode, CsaInfo->ExtChSwAnnIE.ChSwCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WB IE: ChWidth:%d  CentFreq:%d CentFreq:%d\n",
		CsaInfo->wb_info.new_ch_width, CsaInfo->wb_info.center_freq_1, CsaInfo->wb_info.center_freq_2));

	return status;
}

VOID ChannelSwitchAction_1(
	IN	RTMP_ADAPTER * pAd,
	IN	CSA_IE_INFO *CsaInfo)
{
	UINT8 BandIdx;
	struct DOT11_H *pDot11h = NULL;
	struct wifi_dev *wdev = pAd->MacTab.Content[CsaInfo->wcid].wdev;

	if (ChannelSwitchSanityCheck(pAd, CsaInfo->wcid, CsaInfo->ChSwAnnIE.Channel, CsaInfo->SChOffIE.SecondaryChannelOffset) == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): Channel Sanity check:%d\n", __func__, __LINE__));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): NewChannel=%d, Secondary=%d -->\n",
				 __func__, CsaInfo->ChSwAnnIE.Channel, CsaInfo->SChOffIE.SecondaryChannelOffset));

	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Return:%d\n", __func__, __LINE__));
		return;
	}

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		CsaInfo->ChSwAnnIE.Channel != 0 &&
		wdev->channel != CsaInfo->ChSwAnnIE.Channel &&
		pDot11h->RDMode != RD_SWITCHING_MODE) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			("[APCLI] Following root AP to switch channel to ch%u\n",
					CsaInfo->ChSwAnnIE.Channel));

		if ((pAd->CommonCfg.DfsParameter.fUseCsaCfg == FALSE) ||
			(CsaInfo->ChSwAnnIE.ChSwMode == 1)) {
			BandIdx = HcGetBandByWdev(wdev);
			/* Inform FW(N9) about RDD on mesh network */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("[%s] inform N9 about RDD detect BandIdx:%d\n", __func__, BandIdx));
			mtRddControl(pAd, RDD_DETECT_INFO, BandIdx, 0, 0);
		}

		/* Sync wdev settings as per CSA*/
		if (pAd->CommonCfg.DfsParameter.fUseCsaCfg == TRUE) {
#ifdef DOT11_N_SUPPORT
			wlan_config_set_ext_cha(wdev, CsaInfo->SChOffIE.SecondaryChannelOffset);
#endif

#ifdef DOT11_VHT_AC
			wlan_config_set_vht_bw(wdev, CsaInfo->wb_info.new_ch_width);
			wlan_config_set_cen_ch_2(wdev, CsaInfo->wb_info.center_freq_2);
#endif
		}

		pAd->CommonCfg.DfsParameter.ChSwMode = CsaInfo->ChSwAnnIE.ChSwMode;
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), CsaInfo->ChSwAnnIE.Channel);
#endif
		rtmp_set_channel(pAd, wdev, CsaInfo->ChSwAnnIE.Channel);
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Exit:%d <---\n", __func__, __LINE__));
}

void RcsaRecovery(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct DOT11_H *pDot11h = NULL;
	UCHAR BandIdx;

	if ((wdev == NULL) || (pAd->CommonCfg.DfsParameter.bRCSAEn == FALSE))
		return;

	pDot11h = wdev->pDot11_H;
	BandIdx = HcGetBandByWdev(wdev);

	if (pDot11h && (pDot11h->RDMode == RD_SILENCE_MODE)) {
		if (pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone == TRUE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::Got TxDone PAUSE ALTX0\n", __func__));
			mtRddControl(pAd, RDD_ALTX_CTRL, BandIdx, 0, 2);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() RESUME BF RDD_MODE:%d!!!\n", __func__, pDot11h->RDMode));
			mtRddControl(pAd, RDD_RESUME_BF, BandIdx, 0, 0);
			pAd->CommonCfg.DfsParameter.fCheckRcsaTxDone = FALSE;
		}
	}
}
#endif

