/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
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

	All related Dynamic Rate Switch (AP/STA) function body.

	History:

***************************************************************************/

#ifdef NEW_RATE_ADAPT_SUPPORT
#include "rt_config.h"

VOID MlmeCaliRSSITable(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, CHAR Rssi, PUCHAR pTable)
{
    CHAR    ii;
    UCHAR   EstiTxRateIdx = 0;
    UCHAR   WiFiMode, CurrMCSRate;
    CHAR    mcs[24];
    RTMP_RA_GRP_TB *pEstiTxRate, *pCurrTxRate;

    pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, pEntry->CurrTxRateIndex);

    WiFiMode = pCurrTxRate->Mode;
    CurrMCSRate = pCurrTxRate->CurrMCS;

    DBGPRINT(RT_DEBUG_TRACE ,("RSSI %d\n", Rssi));

    if((WiFiMode == MODE_CCK) || (WiFiMode == MODE_OFDM))
    {
        pEntry->RX_RSSI_MCS = 0;
        return;
    }

    // Cover 16dB, about 4~5 MCS Steps
    for(ii = 4; ii > -12; ii--)
    {
        UCHAR   EstiMCSRate;
        /* Check existence and get index of each MCS */
        MlmeGetSupportedMcsAdapt(pAd, pEntry, GI_400, mcs);
        EstiTxRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, ii);
        pEstiTxRate = PTX_RA_GRP_ENTRY(pTable, EstiTxRateIdx);
        EstiMCSRate = pEstiTxRate->CurrMCS;

        pEntry->RX_RSSI_MCS = ii;

        if((WiFiMode == MODE_HTMIX) || (WiFiMode == MODE_HTGREENFIELD))
        {
            DBGPRINT(RT_DEBUG_TRACE ,("HT Mode %d | %d\n", (EstiMCSRate & 0x07), (CurrMCSRate & 0x07)));
            if((EstiMCSRate  & 0x07) == (CurrMCSRate & 0x07))
            {
                break;
            }
        }
        else
        {
            DBGPRINT(RT_DEBUG_TRACE ,("VHT Mode %d | %d\n", EstiMCSRate, CurrMCSRate));
            if(EstiMCSRate == CurrMCSRate)
            {
                break;
            }
        }
    }

    return;
}


/*
	MlmeSetMcsGroup - set initial mcsGroup based on supported MCSs
		On exit pEntry->mcsGroup is set to the mcsGroup
*/
VOID MlmeSetMcsGroup(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
#ifdef DOT11_VHT_AC
	// TODO: shiang-6590, fix me!!
	if (pEntry->SupportRateMode & SUPPORT_VHT_MODE)
	{
		if ((pAd->CommonCfg.TxStream == 2) && (pEntry->SupportVHTMCS[10] == 0x1))
			pEntry->mcsGroup = 2;
		else
			pEntry->mcsGroup = 1;
	}
	else
#endif /* DOT11_VHT_AC */
#ifdef DOT11N_SS3_SUPPORT
	if ((pEntry->HTCapability.MCSSet[2] == 0xff) && (pAd->CommonCfg.TxStream == 3))
		pEntry->mcsGroup = 3;
	 else
#endif /* DOT11N_SS3_SUPPORT */
	 if ((pEntry->HTCapability.MCSSet[0] == 0xff) &&
		(pEntry->HTCapability.MCSSet[1] == 0xff) &&
		(pAd->CommonCfg.TxStream > 1) &&
		((pAd->CommonCfg.TxStream == 2) || (pEntry->HTCapability.MCSSet[2] == 0x0)))
		pEntry->mcsGroup = 2;
	else
		pEntry->mcsGroup = 1;

#ifdef THERMAL_PROTECT_SUPPORT
	if (pAd->force_one_tx_stream == TRUE)
	{
		pEntry->mcsGroup = 1;
	}
#endif /* THERMAL_PROTECT_SUPPORT */
}


/*
	MlmeSelectUpRate - select UpRate based on MCS group
	returns the UpRate index and updates the MCS group
*/
UCHAR MlmeSelectUpRate(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate)
{
	UCHAR UpRateIdx = 0;
	UCHAR grp_cnt;

#ifdef DOT11_VHT_AC
	if ((pEntry->pTable == RateTableVht2S) || (pEntry->pTable == RateTableVht2S_BW20)
					|| (pEntry->pTable == RateTableVht2S_BW40) || (pEntry->pTable == RateTableVht2S_MCS7)
					|| (pEntry->pTable == RateTableVht2S_2G_BW20) || (pEntry->pTable == RateTableVht2S_2G_BW40))
		grp_cnt = 2;
	else if ((pEntry->pTable == RateTableVht1S) || (pEntry->pTable == RateTableVht1S_MCS9)
					|| (pEntry->pTable == RateTableVht1S_2G_BW20) || (pEntry->pTable == RateTableVht1S_2G_BW40))
		grp_cnt = 1;
	else
#endif /* DOT11_VHT_AC */
	if ((pEntry->HTCapability.MCSSet[2] == 0xff) && (pAd->CommonCfg.TxStream == 3))
		grp_cnt =3;
	else if ((pEntry->HTCapability.MCSSet[0] == 0xff) &&
			(pEntry->HTCapability.MCSSet[1] == 0xff) &&
			(pAd->CommonCfg.TxStream > 1) &&
			((pAd->CommonCfg.TxStream == 2) || (pEntry->HTCapability.MCSSet[2] == 0x0)))
		grp_cnt = 2;
	else
		grp_cnt = 1;

#ifdef THERMAL_PROTECT_SUPPORT
	if (pAd->force_one_tx_stream == TRUE)
	{
		grp_cnt = 1;
	}
#endif /* THERMAL_PROTECT_SUPPORT */


	while (1)
	{
		if (grp_cnt == 3)
		{
			switch (pEntry->mcsGroup)
			{
				case 0:/* improvement: use round robin mcs when group == 0 */
					UpRateIdx = pCurrTxRate->upMcs3;
					if (UpRateIdx == pCurrTxRate->ItemNo)
					{
						UpRateIdx = pCurrTxRate->upMcs2;
						if (UpRateIdx == pCurrTxRate->ItemNo)
							UpRateIdx = pCurrTxRate->upMcs1;
					}

					if (MlmeGetTxQuality(pEntry, UpRateIdx) > MlmeGetTxQuality(pEntry, pCurrTxRate->upMcs2) &&
						pCurrTxRate->upMcs2 != pCurrTxRate->ItemNo)
						UpRateIdx = pCurrTxRate->upMcs2;

					if (MlmeGetTxQuality(pEntry, UpRateIdx) > MlmeGetTxQuality(pEntry, pCurrTxRate->upMcs1) &&
						pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)
						UpRateIdx = pCurrTxRate->upMcs1;
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
					DBGPRINT_RAW(RT_DEBUG_ERROR, ("3ss:wrong mcsGroup value\n"));
					break;
			}
		}
		else if (grp_cnt == 2)
		{
			switch (pEntry->mcsGroup)
			{
				case 0:
					UpRateIdx = pCurrTxRate->upMcs2;
					if (UpRateIdx == pCurrTxRate->ItemNo)
						UpRateIdx = pCurrTxRate->upMcs1;

					if (MlmeGetTxQuality(pEntry, UpRateIdx) > MlmeGetTxQuality(pEntry, pCurrTxRate->upMcs1) &&
						pCurrTxRate->upMcs1 != pCurrTxRate->ItemNo)
						UpRateIdx = pCurrTxRate->upMcs1;
					break;
				case 2:
					UpRateIdx = pCurrTxRate->upMcs2;
					break;
				case 1:
					UpRateIdx = pCurrTxRate->upMcs1;
					break;
				default:
					DBGPRINT_RAW(RT_DEBUG_TRACE, ("2ss:wrong mcsGroup value %d\n", pEntry->mcsGroup));
					break;
			}
		}
		else if (grp_cnt == 1)
		{
			switch (pEntry->mcsGroup)
			{
				case 1:
				case 0:
					UpRateIdx = pCurrTxRate->upMcs1;
					break;
				default:
					DBGPRINT_RAW(RT_DEBUG_TRACE, ("1ss:wrong mcsGroup value %d\n", pEntry->mcsGroup));
					break;
			}
		} else {
			DBGPRINT_RAW(RT_DEBUG_TRACE, ("wrong mcsGroup cnt %d\n", grp_cnt));
		}

		/*  If going up from CCK to MCS32 make sure it's allowed */
		if (PTX_RA_GRP_ENTRY(pEntry->pTable, UpRateIdx)->CurrMCS == 32)
		{
			/*  If not allowed then skip over it */
			BOOLEAN mcs32Supported = 0;
			BOOLEAN mcs0Fallback = 0;

			if ((pEntry->HTCapability.MCSSet[4] & 0x1)
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_HT_DUP)
#endif /* DBG_CTRL_SUPPORT */
			)
				mcs32Supported = 1;

#ifdef DBG_CTRL_SUPPORT
			if ((pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0)==0)
				mcs0Fallback = 1;
#endif /* DBG_CTRL_SUPPORT */

			if (pEntry->MaxHTPhyMode.field.BW != BW_40 ||
				pAd->CommonCfg.BBPCurrentBW != BW_40 ||
				(!mcs32Supported && !mcs0Fallback))
			{
				UpRateIdx = PTX_RA_GRP_ENTRY(pEntry->pTable, UpRateIdx)->upMcs1;
				pEntry->mcsGroup = 1;
				break;
			}
		}

		/*  If ShortGI and not allowed then mark it as bad. We'll try another group below */
		if (PTX_RA_GRP_ENTRY(pEntry->pTable, UpRateIdx)->ShortGI &&
			!pEntry->MaxHTPhyMode.field.ShortGI)
		{
			MlmeSetTxQuality(pEntry, UpRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
		}

		/*  If we reached the end of the group then select the best next time */
		if (UpRateIdx == pEntry->CurrTxRateIndex)
		{
			pEntry->mcsGroup = 0;
			break;
		}

		/*  If the current group has bad TxQuality then try another group */
		if ((MlmeGetTxQuality(pEntry, UpRateIdx) > 0) && (pEntry->mcsGroup > 0))
			pEntry->mcsGroup--;
		else
			break;
	}

	return UpRateIdx;
}

/*
	MlmeSelectDownRate - select DownRate.
		pEntry->pTable is assumed to be a pointer to an adaptive rate table with mcsGroup values
		CurrRateIdx - current rate index
		returns the DownRate index. Down Rate = CurrRateIdx if there is no valid Down Rate
*/
UCHAR MlmeSelectDownRate(
	IN RTMP_ADAPTER *pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN UCHAR CurrRateIdx)
{
	UCHAR DownRateIdx = PTX_RA_GRP_ENTRY(pEntry->pTable, CurrRateIdx)->downMcs;
	RTMP_RA_GRP_TB *pDownRate;

	/*  Loop until a valid down rate is found */
	while (1) {
		pDownRate = PTX_RA_GRP_ENTRY(pEntry->pTable, DownRateIdx);

		/*  Break out of loop if rate is valid */
		if (pDownRate->Mode==MODE_CCK)
		{
			/*  CCK is valid only if in G band and if not disabled */
			if ((pAd->LatchRfRegs.Channel<=14
#ifdef DBG_CTRL_SUPPORT
				|| (pAd->CommonCfg.DebugFlags & DBF_ENABLE_CCK_5G)
#endif /* DBG_CTRL_SUPPORT */
			     )
#ifdef DBG_CTRL_SUPPORT
				&& ((pAd->CommonCfg.DebugFlags & DBF_DISABLE_CCK)==0)
#endif /* DBG_CTRL_SUPPORT */
			)
				break;
		}
		else if (pDownRate->CurrMCS == MCS_32)
		{
			BOOLEAN valid_mcs32 = FALSE;

			if ((pEntry->MaxHTPhyMode.field.BW == BW_40 && pAd->CommonCfg.BBPCurrentBW == BW_40)
#ifdef DOT11_VHT_AC
				|| (pEntry->MaxHTPhyMode.field.BW == BW_80 && pAd->CommonCfg.BBPCurrentBW == BW_80)
#endif /* DOT11_VHT_AC */
			)
				valid_mcs32 = TRUE;

			/*  If 20MHz MCS0 fallback enabled and in 40MHz then MCS32 is valid and will be mapped to 20MHz MCS0 */
			if (valid_mcs32
#ifdef DBG_CTRL_SUPPORT
				&& ((pAd->CommonCfg.DebugFlags & DBF_DISABLE_20MHZ_MCS0)==0)
#endif /* DBG_CTRL_SUPPORT */
			)
				break;

			/*  MCS32 is valid if enabled and client supports it */
			if (valid_mcs32 && (pEntry->HTCapability.MCSSet[4] & 0x1)
#ifdef DBG_CTRL_SUPPORT
				&& (pAd->CommonCfg.DebugFlags & DBF_ENABLE_HT_DUP)
#endif /* DBG_CTRL_SUPPORT */
			)
				break;
		}
		else
			break;	/*  All other rates are valid */

		/*  Return original rate if we reached the end without finding a valid rate */
		if (DownRateIdx == pDownRate->downMcs)
			return CurrRateIdx;

		/*  Otherwise try the next lower rate */
		DownRateIdx = pDownRate->downMcs;
	}

	return DownRateIdx;
}


/*
	MlmeSupportMaxRate - Limit Max Rate in the table of supported MCSs
		pAd - pointer to adapter
		pEntry - MAC Table entry. pEntry->pTable is a rate table with mcsGroup values
*/
VOID MlmeLimitMaxRate(
	IN struct _RTMP_ADAPTER     *pAd,
	IN struct _MAC_TABLE_ENTRY  *pEntry,
	IN UCHAR			        *pRateIdx,
	IN UCHAR                    ucDirection)
{
	PUCHAR 			pTable;
	RTMP_RA_GRP_TB 	*pCurrTxRate, *pUpTxRate;
	UCHAR 			CurrRateIdx, UpRateIdx;
	UCHAR			MAXMCSRate;

   	pTable = pEntry->pTable;

    // Check Current Max Limit MCS Rate
    if(ucDirection == RATE_NO_CHANGE)
    {
        CurrRateIdx = *pRateIdx;
	    pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

       	DBGPRINT(RT_DEBUG_INFO, ("Max MCS %X\n", pEntry->MaxHTPhyMode.field.MCS));

        if(pCurrTxRate->CurrMCS == MCS_32)
  	    	return;

        //CCK / OFDM without Limit
        if(pCurrTxRate->Mode < MODE_HTMIX)
   		    return;

	    if(pEntry->SupportRateMode & SUPPORT_VHT_MODE) {
	        MAXMCSRate = pEntry->MaxHTPhyMode.field.MCS & 0x0F;
    	}
	    else if(pEntry->SupportRateMode & SUPPORT_HT_MODE) {
		    if(pCurrTxRate->CurrMCS > MCS_8) { // 7612 Support Only 2SS
			    MAXMCSRate = pEntry->MaxHTPhyMode.field.MCS | 0x08;
    		}
	    	else {
		    	MAXMCSRate = pEntry->MaxHTPhyMode.field.MCS;
            }
	    }
    	else {
		    return;
    	}

	    while(pCurrTxRate->CurrMCS > MAXMCSRate) {
		    if(pCurrTxRate->CurrMCS == MCS_0)
			    break;

    		CurrRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);
	    	pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);
    	}

    	*pRateIdx = CurrRateIdx;

	    return;
    }
    // Check Max Limit MCS Rate when happen up rate process
    else if(ucDirection == RATE_UP)
    {
        UpRateIdx = *pRateIdx;
	    pUpTxRate = PTX_RA_GRP_ENTRY(pTable, UpRateIdx);

        if(pUpTxRate->CurrMCS == MCS_32)
  	    	return;

        //CCK / OFDM without Limit
        if(pUpTxRate->Mode < MODE_HTMIX)
   		    return;

	    if(pEntry->SupportRateMode & SUPPORT_VHT_MODE) {
		    if(pUpTxRate->CurrMCS > (pEntry->MaxHTPhyMode.field.MCS & 0x0F))
			    *pRateIdx = pEntry->CurrTxRateIndex;
    	}
     	else if(pEntry->SupportRateMode & SUPPORT_HT_MODE) {
        	if((pUpTxRate->CurrMCS & 0x07) > (pEntry->MaxHTPhyMode.field.MCS & 0x07))
	        	*pRateIdx = pEntry->CurrTxRateIndex;
       	}
	    return;
    }
    else
   		return;
}

/*
	MlmeGetSupportedMcsAdapt - fills in the table of supported MCSs
		pAd - pointer to adapter
		pEntry - MAC Table entry. pEntry->pTable is a rate table with mcsGroup values
		mcs23GI - the MCS23 entry will have this guard interval
		mcs - table of MCS index into the Rate Table. -1 => not supported
*/
VOID MlmeGetSupportedMcsAdapt(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR mcs23GI,
	OUT CHAR mcs[])
{
	CHAR idx;
	RTMP_RA_GRP_TB *pCurrTxRate;
	UCHAR *pTable = pEntry->pTable;

	for (idx=0; idx<24; idx++)
		mcs[idx] = -1;

#ifdef DOT11_VHT_AC
	if ((pEntry->pTable == RateTableVht1S) || (pEntry->pTable == RateTableVht2S)
				|| (pEntry->pTable == RateTableVht2S_BW20)
				|| (pEntry->pTable == RateTableVht2S_BW40)
				|| (pEntry->pTable == RateTableVht2S_MCS7)
				|| (pEntry->pTable == RateTableVht1S_2G_BW20) || (pEntry->pTable == RateTableVht1S_2G_BW40)
				|| (pEntry->pTable == RateTableVht2S_2G_BW20) || (pEntry->pTable == RateTableVht2S_2G_BW40)
				)
	{
		for (idx = 0; idx < RATE_TABLE_SIZE(pTable); idx++)
		{
			pCurrTxRate = PTX_RA_GRP_ENTRY(pEntry->pTable, idx);
			if (pCurrTxRate->CurrMCS == MCS_RATE_6 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[20] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_9 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[21] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_12 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[22] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_18 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[23] = idx;
			else if ((pCurrTxRate->CurrMCS == MCS_0) && (pCurrTxRate->dataRate == 1) && (mcs[0] == -1))
				mcs[0] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_1 && pCurrTxRate->dataRate == 1)
				mcs[1] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_2 && pCurrTxRate->dataRate == 1)
				mcs[2] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_3 && pCurrTxRate->dataRate == 1)
				mcs[3] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_4 && pCurrTxRate->dataRate == 1)
				mcs[4] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_5 && pCurrTxRate->dataRate == 1)
				mcs[5] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_6 && pCurrTxRate->dataRate == 1)
				mcs[6] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_7 && pCurrTxRate->dataRate == 1)
				mcs[7] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_8 && pCurrTxRate->dataRate == 1)
				mcs[8] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_9 && pCurrTxRate->dataRate == 1)
				mcs[9] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_0 && pCurrTxRate->dataRate == 2)
				mcs[10] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_1 && pCurrTxRate->dataRate == 2)
				mcs[11] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_2 && pCurrTxRate->dataRate == 2)
				mcs[12] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_3 && pCurrTxRate->dataRate == 2)
				mcs[13] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_4 && pCurrTxRate->dataRate == 2)
				mcs[14] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_5 && pCurrTxRate->dataRate == 2)
				mcs[15] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_6 && pCurrTxRate->dataRate == 2)
				mcs[16] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_7 && pCurrTxRate->dataRate == 2)
				mcs[17] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_8 && pCurrTxRate->dataRate == 2)
				mcs[18] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_9 && pCurrTxRate->dataRate == 2)
				mcs[19] = idx;
		}

		return;
	}

	if (pEntry->pTable == RateTableVht1S_MCS9)
	{
		for (idx = 0; idx < RATE_TABLE_SIZE(pTable); idx++)
		{
			pCurrTxRate = PTX_RA_GRP_ENTRY(pEntry->pTable, idx);
			if (pCurrTxRate->CurrMCS == MCS_RATE_6 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[20] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_9 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[21] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_12 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[22] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_RATE_18 && pCurrTxRate->dataRate == 1 && pCurrTxRate->Mode == MODE_OFDM)
				mcs[23] = idx;
			else if ((pCurrTxRate->CurrMCS == MCS_0) && (pCurrTxRate->dataRate == 1) && (mcs[0] == -1))
				mcs[0] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_1 && pCurrTxRate->dataRate == 1)
				mcs[1] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_2 && pCurrTxRate->dataRate == 1)
				mcs[2] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_3 && pCurrTxRate->dataRate == 1)
				mcs[3] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_4 && pCurrTxRate->dataRate == 1)
				mcs[4] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_5 && pCurrTxRate->dataRate == 1)
				mcs[5] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_6 && pCurrTxRate->dataRate == 1)
				mcs[6] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_7 && pCurrTxRate->dataRate == 1)
				mcs[7] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_8 && pCurrTxRate->dataRate == 1)
				mcs[8] = idx;
			else if (pCurrTxRate->CurrMCS == MCS_9 && pCurrTxRate->dataRate == 1)
				mcs[9] = idx;
		}

		return;
	}
#endif /* DOT11_VHT_AC */

	/*  check the existence and index of each needed MCS */
	for (idx = 0; idx < RATE_TABLE_SIZE(pTable); idx++)
	{
		pCurrTxRate = PTX_RA_GRP_ENTRY(pEntry->pTable, idx);

		if ((pCurrTxRate->CurrMCS >= 8 && pAd->CommonCfg.TxStream < 2) ||
			(pCurrTxRate->CurrMCS >= 16 && pAd->CommonCfg.TxStream < 3))
			continue;

		/*  Rate Table may contain CCK and MCS rates. Give HT/Legacy priority over CCK */
		if (pCurrTxRate->CurrMCS==MCS_0 && (mcs[0]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[0] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_1 && (mcs[1]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[1] = idx;
		else if (pCurrTxRate->CurrMCS==MCS_2 && (mcs[2]==-1 || pCurrTxRate->Mode!=MODE_CCK))
			mcs[2] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_3)
			mcs[3] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_4)
			mcs[4] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_5)
			mcs[5] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_6)
			mcs[6] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_7) && (pCurrTxRate->ShortGI == GI_800))
			mcs[7] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_8)
			mcs[8] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_9)
			mcs[9] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_10)
			mcs[10] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_11)
			mcs[11] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_12)
			mcs[12] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_13)
			mcs[13] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_14) && (pCurrTxRate->ShortGI == GI_800))
			mcs[14] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_15) && (pCurrTxRate->ShortGI == GI_800))
			mcs[15] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_16)
			mcs[16] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_17)
			mcs[17] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_18)
			mcs[18] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_19)
			mcs[19] = idx;
		else if (pCurrTxRate->CurrMCS == MCS_20)
			mcs[20] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_21) && (pCurrTxRate->ShortGI == GI_800))
			mcs[21] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_22) && (pCurrTxRate->ShortGI == GI_800))
			mcs[22] = idx;
		else if ((pCurrTxRate->CurrMCS == MCS_23) && (pCurrTxRate->ShortGI == mcs23GI))
			mcs[23] = idx;
	}

#ifdef DBG_CTRL_SUPPORT
	/*  Debug Option: Disable highest MCSs when picking initial MCS based on RSSI */
	if (pAd->CommonCfg.DebugFlags & DBF_INIT_MCS_DIS1)
		mcs[23] = mcs[15] = mcs[7] = mcs[22] = mcs[14] = mcs[6] = 0;
#endif /* DBG_CTRL_SUPPORT */

}


UCHAR get_rate_idx_by_rate(RTMP_ADAPTER *pAd, UCHAR *rate_tb,  USHORT rate)
{
	UCHAR tb_idx = 0;

#ifdef DOT11_N_SUPPORT
	if (ADAPT_RATE_TABLE(rate_tb))
	{

	}
#endif /* DOT11_N_SUPPORT */

	return tb_idx;
}

#ifdef NEW_RATE_ADAPT_QUICK_DOWN
/* QuickInitMCSRate now has bad algorithm and periodic drop PhyMode to CCK/0 */
VOID TriggerQuickInitMCSRate(
    	IN PRTMP_ADAPTER 	pAd,
	    IN PMAC_TABLE_ENTRY	pEntry,
        IN UINT32 CheckInterval)
{
   	PUCHAR pTable;
   	pTable = pEntry->pTable;
    NdisZeroMemory(pEntry->DownTxMCSRate, sizeof(pEntry->DownTxMCSRate));

#ifdef DOT11_VHT_AC
#ifdef NEW_RATE_ADAPT_SUPPORT
	if (pTable == RateTableVht1S || pTable == RateTableVht2S || pTable == RateTableVht1S_MCS9
					|| pTable == RateTableVht2S_BW20
					|| pTable == RateTableVht2S_BW40
					|| pTable == RateTableVht2S_MCS7
					|| pTable == RateSwitchTableAdapt11N1S
					|| pTable == RateSwitchTableAdapt11N2S)
	{
    	pEntry->LowPacket = TRUE;
    	pEntry->LastSecTxRateChangeAction = RATE_DOWN;

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (!pAd->ApCfg.ApQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, CheckInterval);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
			}
		}
#endif /*  CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, CheckInterval);
				pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
			}
	}
#endif /*  CONFIG_STA_SUPPORT */
	}
#endif /* NEW_RATE_ADAPT_SUPPORT */
#endif /* DOT11_VHT_AC */
}

BOOLEAN QuickInitMCSRate(
	    IN PRTMP_ADAPTER 	pAd,
	    IN PMAC_TABLE_ENTRY	pEntry,
	    IN UINT16 PacketSucc,
	    IN UINT16 PacketErro)
{
	PUCHAR					pTable;
	UCHAR					CurrRateIdx;
	RTMP_RA_GRP_TB          *pCurrTxRate;

    if(pEntry->LowPacket == TRUE)
    {
        CHAR DownIdx, MaxIdx = 0;

        pTable = pEntry->pTable;
        CurrRateIdx = pEntry->CurrTxRateIndex;
        if(PacketSucc)
            MaxIdx = PacketErro/PacketSucc;
        else
            MaxIdx = 0;

        DBGPRINT(RT_DEBUG_TRACE ,("CR : Quick Fast Down MCS %d/%d=%d\n",
            PacketErro, PacketSucc, MaxIdx));

        if(MaxIdx >= (NUM_OF_SWFB-1))
            MaxIdx = (NUM_OF_SWFB-1);

        for(DownIdx=0; DownIdx < MaxIdx; DownIdx++)
        {
	        pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

	        CurrRateIdx = pCurrTxRate->downMcs;

	        if(pCurrTxRate->TrainDown > 100)
	            break;
        }

        DBGPRINT(RT_DEBUG_TRACE ,("Quick Fast Down MCS %d %d %d\n",
            CurrRateIdx, pEntry->CurrTxRateIndex, MaxIdx));

        pEntry->CurrTxRateIndex = CurrRateIdx;

        pEntry->LastSaveRateIdx = CurrRateIdx;

        NdisZeroMemory(pEntry->DownTxMCSRate, sizeof(pEntry->DownTxMCSRate));

        MlmeClearAllTxQuality(pEntry);	/* clear all history */

		MlmeNewTxRate(pAd, pEntry);

        pEntry->LowPacket = FALSE;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif

/*
	MlmeSelectTxRateAdapt - select the MCS based on the RSSI and the available MCSs
		pAd - pointer to adapter
		pEntry - pointer to MAC table entry
		mcs - table of MCS index into the Rate Table. -1 => not supported
		Rssi - the Rssi value
		RssiOffset - offset to apply to the Rssi
*/
UCHAR MlmeSelectTxRateAdapt(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset)
{
	UCHAR TxRateIdx = 0;
	UCHAR *pTable = pEntry->pTable;

#ifdef DBG_CTRL_SUPPORT
	/*  Debug option: Add 6 dB of margin */
	if (pAd->CommonCfg.DebugFlags & DBF_INIT_MCS_MARGIN)
		RssiOffset += 6;
#endif /* DBG_CTRL_SUPPORT */

#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
	if (pTable == RateTableVht1S || pTable == RateTableVht2S || pTable == RateTableVht1S_MCS9
					|| pTable == RateTableVht2S_BW20
					|| pTable == RateTableVht2S_BW40
					|| pTable == RateTableVht2S_MCS7
					|| pTable == RateTableVht1S_2G_BW20
					|| pTable == RateTableVht1S_2G_BW40
					|| pTable == RateTableVht2S_2G_BW20
					|| pTable == RateTableVht2S_2G_BW40
					)
	{
		if (pTable == RateTableVht2S || pTable == RateTableVht2S_BW40
			|| (pTable == RateTableVht2S_2G_BW40))
		{
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: GRP: 2*2, RssiOffset=%d\n", __FUNCTION__, RssiOffset));

			/* 2x2 peer device (Adhoc, DLS or AP) */
			if (mcs[19] && (Rssi > (-65 + RssiOffset))) {
				TxRateIdx = mcs[19];
			}
			else if (mcs[18] && (Rssi > (-67 + RssiOffset))) {
				TxRateIdx = mcs[18];
			}
			else if (mcs[17] && (Rssi > (-69 + RssiOffset))) {
				TxRateIdx = mcs[17];
			}
			else if (mcs[16] && (Rssi > (-71 + RssiOffset))) {
				TxRateIdx = mcs[16];
			}
			else if (mcs[15] && (Rssi > (-74 + RssiOffset))) {
				TxRateIdx = mcs[15];
			}
			else if (mcs[14] && (Rssi > (-76 + RssiOffset))) {
				TxRateIdx = mcs[14];
			}
			else if (mcs[3] && (Rssi > (-80 + RssiOffset))) {
				TxRateIdx = mcs[3];
			}
			else if (mcs[2] && (Rssi > (-82 + RssiOffset))) {
				TxRateIdx = mcs[2];
			}
			else if (mcs[23] && (Rssi > (-85 + RssiOffset))) {
				TxRateIdx = mcs[23]; // OFDM 1x1 MCS3 BW20
			}
			else if (mcs[22] && (Rssi > (-87 + RssiOffset))) {
				TxRateIdx = mcs[22]; // OFDM 1x1 MCS2 BW20
			}
			else {
				TxRateIdx = mcs[20]; // OFDM 1x1 MCS0 BW20
			}

			pEntry->mcsGroup = 2;
		} else if (pTable == RateTableVht2S_MCS7) {
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: GRP: 2*2, RssiOffset=%d\n", __FUNCTION__, RssiOffset));

			/* 2x2 peer device (Adhoc, DLS or AP) */
			if (mcs[17] && (Rssi > (-69 + RssiOffset))) {
				TxRateIdx = mcs[17];
			}
			else if (mcs[16] && (Rssi > (-71 + RssiOffset))) {
				TxRateIdx = mcs[16];
			}
			else if (mcs[15] && (Rssi > (-74 + RssiOffset))) {
				TxRateIdx = mcs[15];
			}
			else if (mcs[14] && (Rssi > (-76 + RssiOffset))) {
				TxRateIdx = mcs[14];
			}
			else if (mcs[3] && (Rssi > (-80 + RssiOffset))) {
				TxRateIdx = mcs[3];
			}
			else if (mcs[2] && (Rssi > (-82 + RssiOffset))) {
				TxRateIdx = mcs[2];
			}
			else if (mcs[23] && (Rssi > (-85 + RssiOffset))) {  // add new condition
				TxRateIdx = mcs[23]; // OFDM 1x1 MCS3 BW20
			}
			else if (mcs[22] && (Rssi > (-87 + RssiOffset))) {
				TxRateIdx = mcs[22]; // OFDM 1x1 MCS2 BW20
			}
			else {
				TxRateIdx = mcs[20]; // OFDM 1x1 MCS0 BW20
			}

			pEntry->mcsGroup = 2;
		}
		else if ((pTable == RateTableVht2S_BW20)
			|| (pTable == RateTableVht2S_2G_BW20))
		{

			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: GRP: 2*2, RssiOffset=%d\n", __FUNCTION__, RssiOffset));

			/* 2x2 peer device (Adhoc, DLS or AP) */
			if (mcs[18] && (Rssi > (-67 + RssiOffset))) {
				TxRateIdx = mcs[18];
			}
			else if (mcs[17] && (Rssi > (-69 + RssiOffset))) {
				TxRateIdx = mcs[17];
			}
			else if (mcs[16] && (Rssi > (-71 + RssiOffset))) {
				TxRateIdx = mcs[16];
			}
			else if (mcs[15] && (Rssi > (-74 + RssiOffset))) {
				TxRateIdx = mcs[15];
			}
			else if (mcs[14] && (Rssi > (-76 + RssiOffset))) {
				TxRateIdx = mcs[14];
			}
			else if (mcs[3] && (Rssi > (-80 + RssiOffset))) {
				TxRateIdx = mcs[3];
			}
			else if (mcs[23] && (Rssi > (-85 + RssiOffset))) {
				TxRateIdx = mcs[23];
			}
			else if (mcs[22] && (Rssi > (-92 + RssiOffset))) {
				TxRateIdx = mcs[22];
			}
			else if (mcs[20] && (Rssi > (-94 + RssiOffset))) {
				TxRateIdx = mcs[21];
			}
			else {
				TxRateIdx = mcs[20];
			}

			pEntry->mcsGroup = 2;

		}
		else if ((pTable == RateTableVht1S_MCS9)
			|| (pTable == RateTableVht1S_2G_BW20) || (pTable == RateTableVht1S_2G_BW40))
		{
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: GRP: 1*1, RssiOffset=%d\n", __FUNCTION__, RssiOffset));

			/* 1x1 peer device (Adhoc, DLS or AP) */
			if (mcs[9] && (Rssi > (-67 + RssiOffset)))
				TxRateIdx = mcs[9];
			else if (mcs[8] && (Rssi > (-69 + RssiOffset)))
				TxRateIdx = mcs[8];
			else if (mcs[7] && (Rssi > (-71 + RssiOffset)))
				TxRateIdx = mcs[7];
			else if (mcs[6] && (Rssi > (-73 + RssiOffset)))
				TxRateIdx = mcs[6];
			else if (mcs[5] && (Rssi > (-76 + RssiOffset)))
				TxRateIdx = mcs[5];
			else if (mcs[3] && (Rssi > (-78 + RssiOffset)))
				TxRateIdx = mcs[3];
			else if (mcs[2] && (Rssi > (-80 + RssiOffset))) // new
				TxRateIdx = mcs[2];
			else if (mcs[23] && (Rssi > (-82 + RssiOffset)))
				TxRateIdx = mcs[23]; // MCS_RATE_18
			else if (mcs[22] && (Rssi > (-84 + RssiOffset)))
				TxRateIdx = mcs[22]; // MCS_RATE_12
			else if (mcs[21] && (Rssi > (-87 + RssiOffset))) // new
				TxRateIdx = mcs[21]; // MCS_RATE_9
			//else if (mcs[20] && (Rssi > (-89 + RssiOffset)))
			//	TxRateIdx = mcs[20]; // MCS_RATE_9
			else
				TxRateIdx = mcs[20]; // MCS_RATE_6
			
			pEntry->mcsGroup = 1;
		}
		else
		{
			DBGPRINT_RAW(RT_DEBUG_INFO | DBG_FUNC_RA, ("%s: GRP: 1*1, RssiOffset=%d\n", __FUNCTION__, RssiOffset));

			/* 1x1 peer device (Adhoc, DLS or AP) */
			if (mcs[7] && (Rssi > (-71 + RssiOffset)))
				TxRateIdx = mcs[7];
			else if (mcs[6] && (Rssi > (-73 + RssiOffset)))
				TxRateIdx = mcs[6];
			else if (mcs[5] && (Rssi > (-76 + RssiOffset)))
				TxRateIdx = mcs[5];
			else if (mcs[4] && (Rssi > (-78 + RssiOffset)))
				TxRateIdx = mcs[4];
			else if (mcs[23] && (Rssi > (-82 + RssiOffset)))
				TxRateIdx = mcs[23]; // MCS_RATE_18
			else if (mcs[22] && (Rssi > (-84 + RssiOffset)))
				TxRateIdx = mcs[22]; // MCS_RATE_12
			else if (mcs[21] && (Rssi > (-89 + RssiOffset)))
				TxRateIdx = mcs[21]; // MCS_RATE_9
			else
				TxRateIdx = mcs[0];

			pEntry->mcsGroup = 1;
		}
	}
	else
#endif /* DOT11_VHT_AC */
	 if (ADAPT_RATE_TABLE(pTable) ||
		 (pTable == RateSwitchTable11BGN3S) ||
		 (pTable == RateSwitchTable11BGN3SForABand))
	{/*  N mode with 3 stream */
		if ((pEntry->HTCapability.MCSSet[2] == 0xff) &&
#ifdef THERMAL_PROTECT_SUPPORT
				(pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
				(pAd->CommonCfg.TxStream == 3)
				)
		{
			if (mcs[23]>=0 && (Rssi > (-72+RssiOffset)))
				TxRateIdx = mcs[23];
			else if (mcs[22]>=0 && (Rssi > (-74+RssiOffset)))
				TxRateIdx = mcs[22];
			else if (mcs[21]>=0 && (Rssi > (-77+RssiOffset)))
				TxRateIdx = mcs[21];
			else if (mcs[20]>=0 && (Rssi > (-79+RssiOffset)))
				TxRateIdx = mcs[20];
			else if (mcs[11]>=0 && (Rssi > (-81+RssiOffset)))
				TxRateIdx = mcs[11];
			else if (mcs[10]>=0 && (Rssi > (-83+RssiOffset)))
				TxRateIdx = mcs[10];
			else if (mcs[2]>=0 && (Rssi > (-86+RssiOffset)))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-88+RssiOffset)))
				TxRateIdx = mcs[1];
			else
				TxRateIdx = mcs[0];

			pEntry->mcsGroup = 3;
		}
		else if ((pEntry->HTCapability.MCSSet[0] == 0xff) &&
				(pEntry->HTCapability.MCSSet[1] == 0xff) &&
				(pAd->CommonCfg.TxStream > 1) &&
#ifdef THERMAL_PROTECT_SUPPORT
				(pAd->force_one_tx_stream == FALSE) &&
#endif /* THERMAL_PROTECT_SUPPORT */
				((pAd->CommonCfg.TxStream == 2) || (pEntry->HTCapability.MCSSet[2] == 0x0)))
		{
			if (mcs[15]>=0 && (Rssi > (-72+RssiOffset)))
				TxRateIdx = mcs[15];
			else if (mcs[14]>=0 && (Rssi > (-74+RssiOffset)))
				TxRateIdx = mcs[14];
			else if (mcs[13]>=0 && (Rssi > (-77+RssiOffset)))
				TxRateIdx = mcs[13];
			else if (mcs[12]>=0 && (Rssi > (-79+RssiOffset)))
				TxRateIdx = mcs[12];
			else if (mcs[11]>=0 && (Rssi > (-81+RssiOffset)))
				TxRateIdx = mcs[11];
			else if (mcs[10]>=0 && (Rssi > (-83+RssiOffset)))
				TxRateIdx = mcs[10];
			else if (mcs[2]>=0 && (Rssi > (-86+RssiOffset)))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-88+RssiOffset)))
				TxRateIdx = mcs[1];
			else
				TxRateIdx = mcs[0];

			pEntry->mcsGroup = 2;
		}
		else
		{
			if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)))
				TxRateIdx = mcs[7];
			else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)))
				TxRateIdx = mcs[6];
			else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)))
				TxRateIdx = mcs[5];
			else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)))
				TxRateIdx = mcs[4];
			else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)))
				TxRateIdx = mcs[3];
			else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)))
				TxRateIdx = mcs[2];
			else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)))
				TxRateIdx = mcs[1];
			else
				TxRateIdx = mcs[0];

			pEntry->mcsGroup = 1;
		}
	}
	else if ((pTable == RateSwitchTable11BGN2S) ||
		(pTable == RateSwitchTable11BGN2SForABand) ||
		(pTable == RateSwitchTable11N2S) ||
		(pTable == RateSwitchTable11N2SForABand))
	{/*  N mode with 2 stream */
		if (mcs[15]>=0 && (Rssi >= (-70+RssiOffset)))
			TxRateIdx = mcs[15];
		else if (mcs[14]>=0 && (Rssi >= (-72+RssiOffset)))
			TxRateIdx = mcs[14];
		else if (mcs[13]>=0 && (Rssi >= (-76+RssiOffset)))
			TxRateIdx = mcs[13];
		else if (mcs[12]>=0 && (Rssi >= (-78+RssiOffset)))
			TxRateIdx = mcs[12];
		else if (mcs[4]>=0 && (Rssi >= (-82+RssiOffset)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi >= (-84+RssiOffset)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi >= (-86+RssiOffset)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi >= (-88+RssiOffset)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else if ((pTable == RateSwitchTable11BGN1S) ||
			 (pTable == RateSwitchTable11N1S) ||
			 (pTable == RateSwitchTable11N1SForABand))
	{/*  N mode with 1 stream */
		if (mcs[7]>=0 && (Rssi > (-72+RssiOffset)))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > (-74+RssiOffset)))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > (-77+RssiOffset)))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > (-79+RssiOffset)))
			TxRateIdx = mcs[4];
		else if (mcs[3]>=0 && (Rssi > (-81+RssiOffset)))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > (-83+RssiOffset)))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > (-86+RssiOffset)))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{/*  Legacy mode */
		if (mcs[7]>=0 && (Rssi > -70))
			TxRateIdx = mcs[7];
		else if (mcs[6]>=0 && (Rssi > -74))
			TxRateIdx = mcs[6];
		else if (mcs[5]>=0 && (Rssi > -78))
			TxRateIdx = mcs[5];
		else if (mcs[4]>=0 && (Rssi > -82))
			TxRateIdx = mcs[4];
		else if (mcs[4] == -1)	/*  for B-only mode */
			TxRateIdx = mcs[3];
		else if (mcs[3]>=0 && (Rssi > -85))
			TxRateIdx = mcs[3];
		else if (mcs[2]>=0 && (Rssi > -87))
			TxRateIdx = mcs[2];
		else if (mcs[1]>=0 && (Rssi > -90))
			TxRateIdx = mcs[1];
		else
			TxRateIdx = mcs[0];
	}

	return TxRateIdx;
}


/*
	MlmeRAHybridRule - decide whether to keep the new rate or use old rate
		pEntry - the MAC table entry for this STA
		pCurrTxRate - pointer to Rate table entry for new up rate
		NewTxOkCount - normalized count of Tx packets for new up rate
		TxErrorRatio - the PER
	returns
		TRUE if old rate should be used
*/
BOOLEAN MlmeRAHybridRule(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate,
	IN ULONG			NewTxOkCount,
	IN ULONG			TxErrorRatio)
{


    DBGPRINT(RT_DEBUG_TRACE, ("RAA : Tx OK Counter %lu %lu\n", NewTxOkCount , pEntry->LastTxOkCount));

	if ((120*NewTxOkCount > pAd->CommonCfg.TrainUpHighThrd*pEntry->LastTxOkCount) ||
        (TxErrorRatio < 10))
		return FALSE;

    return TRUE;
}

/*
	MlmeNewRateAdapt - perform Rate Adaptation based on PER using New RA algorithm
		pEntry - the MAC table entry for this STA
		UpRateIdx, DownRateIdx - UpRate and DownRate index
		TrainUp, TrainDown - TrainUp and Train Down threhsolds
		TxErrorRatio - the PER

		On exit:
			pEntry->LastSecTxRateChangeAction = RATE_UP or RATE_DOWN if there was a change
			pEntry->CurrTxRateIndex = new rate index
			pEntry->TxQuality is updated
*/
VOID MlmeNewRateAdapt(
	IN PRTMP_ADAPTER 	pAd,
	IN PMAC_TABLE_ENTRY	pEntry,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio)
{
	USHORT		phyRateLimit20 = 0;
	BOOLEAN		bTrainUp = FALSE;
#ifdef TXBF_SUPPORT
	BOOLEAN 	invertTxBf = FALSE;
#endif /*  TXBF_SUPPORT */
	UCHAR *pTable = pEntry->pTable;
	UCHAR CurrRateIdx = pEntry->CurrTxRateIndex;
	RTMP_RA_GRP_TB *pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

	pEntry->CurrTxRateStableTime++;

	pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;

    pEntry->LastSaveRateIdx = pEntry->CurrTxRateIndex;


	if (TxErrorRatio >= TrainDown)
	{
#ifdef TXBF_SUPPORT
		RTMP_RA_GRP_TB *pDownRate, *pLastNonBfRate;
#endif /* TXBF_SUPPORT */

		/*  Downgrade TX quality if PER >= Rate-Down threshold */
		MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
		/*
			Need to train down. If BF and last Non-BF is no worse than the down rate then
			go to last Non-BF rate. Otherwise just go to the down rate
		*/

		pDownRate = PTX_RA_GRP_ENTRY(pTable, DownRateIdx);
		pLastNonBfRate = PTX_RA_GRP_ENTRY(pTable, pEntry->lastNonBfRate);

		if ((pEntry->phyETxBf || pEntry->phyITxBf) &&
			(pLastNonBfRate->dataRate >= pDownRate->dataRate)
#ifdef DBG_CTRL_SUPPORT
			&& ((pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)==0)
#endif /* DBG_CTRL_SUPPORT */
		)
		{
			invertTxBf = TRUE;
			pEntry->CurrTxRateIndex = pEntry->lastNonBfRate;
			pEntry->LastSecTxRateChangeAction = RATE_DOWN;
		}
		else
#endif /*  TXBF_AWARE */
#endif /*  TXBF_SUPPORT */
		if (CurrRateIdx != DownRateIdx)
		{
			pEntry->CurrTxRateIndex = DownRateIdx;
			pEntry->LastSecTxRateChangeAction = RATE_DOWN;
		}
	}
	else
	{
		RTMP_RA_GRP_TB *pUpRate = PTX_RA_GRP_ENTRY(pTable, UpRateIdx);

		/*  Upgrade TX quality if PER <= Rate-Up threshold */
		if (TxErrorRatio <= TrainUp)
		{
			bTrainUp = TRUE;
			MlmeDecTxQuality(pEntry, CurrRateIdx);  /*  quality very good in CurrRate */

			if (pEntry->TxRateUpPenalty) /* always == 0, always go to else */
				pEntry->TxRateUpPenalty --;
			else
			{
				/*
					Decrement the TxQuality of the UpRate and all of the MCS groups.
					Note that UpRate may mot equal one of the MCS groups if MlmeSelectUpRate
					skipped over a rate that is not valid for this configuration.
				*/
				MlmeDecTxQuality(pEntry, UpRateIdx);

				if (pCurrTxRate->upMcs3!=CurrRateIdx &&
					pCurrTxRate->upMcs3!=UpRateIdx)
					MlmeDecTxQuality(pEntry, pCurrTxRate->upMcs3);

				if (pCurrTxRate->upMcs2!=CurrRateIdx &&
						pCurrTxRate->upMcs2!=UpRateIdx &&
						pCurrTxRate->upMcs2!=pCurrTxRate->upMcs3)
					MlmeDecTxQuality(pEntry, pCurrTxRate->upMcs2);

				if (pCurrTxRate->upMcs1!=CurrRateIdx &&
						pCurrTxRate->upMcs1!=UpRateIdx &&
						pCurrTxRate->upMcs1!=pCurrTxRate->upMcs3 &&
						pCurrTxRate->upMcs1!=pCurrTxRate->upMcs2)
					MlmeDecTxQuality(pEntry, pCurrTxRate->upMcs1);
			}
		}
		else if (pEntry->mcsGroup > 0) /* even if TxErrorRatio > TrainUp */
		{
			/*  Moderate PER but some groups are not tried */
			bTrainUp = TRUE;

			/* TxQuality[CurrRateIdx] must be decremented so that mcs won't decrease wrongly */
			MlmeDecTxQuality(pEntry, CurrRateIdx);  /*  quality very good in CurrRate */
			MlmeDecTxQuality(pEntry, UpRateIdx);    /*  may improve next UP rate's quality */
		}

		/*  Don't try up rate if it's greater than the limit */
		if ((phyRateLimit20 != 0) && (pUpRate->dataRate >= phyRateLimit20))
			return;

		/*  If UpRate is good then train up in current BF state */
		if ((CurrRateIdx != UpRateIdx) && (MlmeGetTxQuality(pEntry, UpRateIdx) <= 0) && bTrainUp)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RAA : Up Rate %d\n", UpRateIdx));
			pEntry->CurrTxRateIndex = UpRateIdx;
			pEntry->LastSecTxRateChangeAction = RATE_UP;
		}
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
		else
#ifdef DBG_CTRL_SUPPORT
		if ((pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)==0)
#endif /* DBG_CTRL_SUPPORT */
		{
			/*  If not at the highest rate then try inverting BF state */
			if (pEntry->phyETxBf || pEntry->phyITxBf)
			{
				/*  If BF then try the same MCS non-BF unless PER is good */
				if (TxErrorRatio > TrainUp)
				{
					if (pEntry->TxQuality[CurrRateIdx])
						pEntry->TxQuality[CurrRateIdx]--;

					if (pEntry->TxQuality[CurrRateIdx]==0)
					{
						invertTxBf = TRUE;
						pEntry->CurrTxRateIndex = CurrRateIdx;
						pEntry->LastSecTxRateChangeAction = RATE_UP;
					}
				}
			}
			else if (pEntry->eTxBfEnCond>0 || pEntry->iTxBfEn)
			{
				/*  First try Up Rate with BF */
				if ((CurrRateIdx != UpRateIdx) &&
					 MlmeTxBfAllowed(pAd, pEntry, (RTMP_RA_LEGACY_TB *)pUpRate))
				{
					if (pEntry->BfTxQuality[UpRateIdx])
						pEntry->BfTxQuality[UpRateIdx]--;

					if (pEntry->BfTxQuality[UpRateIdx]==0)
					{
						invertTxBf = TRUE;
						pEntry->CurrTxRateIndex = UpRateIdx;
						pEntry->LastSecTxRateChangeAction = RATE_UP;
					}
				}

				/*  Try Same Rate if Up Rate failed */
				if (pEntry->LastSecTxRateChangeAction==RATE_NO_CHANGE &&
					MlmeTxBfAllowed(pAd, pEntry, (RTMP_RA_LEGACY_TB *)pCurrTxRate))
				{
					if (pEntry->BfTxQuality[CurrRateIdx])
						pEntry->BfTxQuality[CurrRateIdx]--;

					if (pEntry->BfTxQuality[CurrRateIdx]==0)
					{
						invertTxBf = TRUE;
						pEntry->CurrTxRateIndex = CurrRateIdx;
						pEntry->LastSecTxRateChangeAction = RATE_UP;
					}
				}
			}
		}
#endif /*  TXBF_AWARE*/
#endif /*  TXBF_SUPPORT */
	}

	/*  Handle the rate change */
	if ((pEntry->LastSecTxRateChangeAction != RATE_NO_CHANGE)
#ifdef DBG_CTRL_SUPPORT
		|| (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)
#endif /* DBG_CTRL_SUPPORT */
	)
	{
		if (pEntry->LastSecTxRateChangeAction!=RATE_NO_CHANGE)
		{
			DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: %sTX rate from %d to %d \n",
				pEntry->LastSecTxRateChangeAction==RATE_UP? "++": "--", CurrRateIdx, pEntry->CurrTxRateIndex));
		}

		pEntry->CurrTxRateStableTime = 0;
		pEntry->TxRateUpPenalty = 0;

		/*  Save last rate information */
		pEntry->lastRateIdx = CurrRateIdx;
#ifdef TXBF_SUPPORT
#ifdef TXBF_AWARE
		if (pEntry->eTxBfEnCond > 0)
		{
			pEntry->lastRatePhyTxBf = pEntry->phyETxBf;
			pEntry->phyETxBf ^= invertTxBf;
		}
		else
		{
			pEntry->lastRatePhyTxBf = pEntry->phyITxBf;
			pEntry->phyITxBf ^= invertTxBf;
		}
#else
#ifdef MT76x2
    pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);
    //For VHT Mode
    if((pCurrTxRate->Mode == 4) && (pCurrTxRate->dataRate == 1))
    {
		    if (pEntry->eTxBfEnCond > 0)
		    {
		        pEntry->phyETxBf = TRUE;
		        pEntry->lastRatePhyTxBf = pEntry->phyETxBf;
	      }
		    else
		    {
			      pEntry->phyITxBf = TRUE;
	          pEntry->lastRatePhyTxBf = pEntry->phyITxBf;
			  }
    }
    //For HT Mode
    else if((pCurrTxRate->Mode == 3 || pCurrTxRate->Mode == 2)
        && (pCurrTxRate->CurrMCS < 8))
    {
		    if (pEntry->eTxBfEnCond > 0)
		    {
		        pEntry->phyETxBf = TRUE;
		        pEntry->lastRatePhyTxBf = pEntry->phyETxBf;
	      }
		    else
		    {
			      pEntry->phyITxBf = TRUE;
	          pEntry->lastRatePhyTxBf = pEntry->phyITxBf;
			  }
    }
    // Other OFDM and CCK
    else
    {
		    if (pEntry->eTxBfEnCond > 0)
		    {
		        pEntry->phyETxBf = FALSE;
		        pEntry->lastRatePhyTxBf = pEntry->phyETxBf;
	      }
		    else
		    {
			      pEntry->phyITxBf = FALSE;
	          pEntry->lastRatePhyTxBf = pEntry->phyITxBf;
			  }
    }
#endif	/* MT76x2 */
#endif /* TXBF_AWARE */
#endif /*  TXBF_SUPPORT */

		/*  Update TxQuality */
		if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
		{
			MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
			pEntry->PER[pEntry->CurrTxRateIndex] = 0;
		}

		/*  Set timer for check in 100 msec */
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (!pAd->ApCfg.ApQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->ApCfg.ApQuickResponeForRateUpTimer, pAd->ra_fast_interval);
				pAd->ApCfg.ApQuickResponeForRateUpTimerRunning = TRUE;
			}
		}
#endif /*  CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (!pAd->StaCfg.StaQuickResponeForRateUpTimerRunning)
			{
				RTMPSetTimer(&pAd->StaCfg.StaQuickResponeForRateUpTimer, pAd->ra_fast_interval);
				pAd->StaCfg.StaQuickResponeForRateUpTimerRunning = TRUE;
			}
		}
#endif /*  CONFIG_STA_SUPPORT */

		/*  Update PHY rate */
		MlmeNewTxRate(pAd, pEntry);
	}
}


#ifdef CONFIG_AP_SUPPORT
/*
    ========================================================================
    Routine Description:
        AP side, Auto TxRate faster train up timer call back function.

    Arguments:
        SystemSpecific1         - Not used.
        FunctionContext         - Pointer to our Adapter context.
        SystemSpecific2         - Not used.
        SystemSpecific3         - Not used.

    Return Value:
        None

    ========================================================================
*/
VOID APQuickResponeForRateUpExecAdapt(/* actually for both up and down */
    IN PRTMP_ADAPTER pAd,
    IN UINT idx)
{
	PUCHAR					pTable;
	UCHAR					CurrRateIdx;
	ULONG					AccuTxTotalCnt, TxTotalCnt, TxCnt;
	ULONG					TxErrorRatio = 0;
	MAC_TABLE_ENTRY			*pEntry;
	RTMP_RA_GRP_TB *pCurrTxRate;
	UCHAR					TrainUp, TrainDown;
	CHAR					Rssi, ratio;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
	ULONG					OneSecTxNoRetryOKRationCount;
	BOOLEAN					rateChanged;
#ifdef TXBF_SUPPORT
	BOOLEAN					CurrPhyETxBf, CurrPhyITxBf;
#endif /*  TXBF_SUPPORT */


	pEntry = &pAd->MacTab.Content[idx];

    //if(QuickInitMCSRate(pAd,pEntry) == TRUE)
    //    return;


	pTable = pEntry->pTable;

	Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

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
#ifdef CUSTOMER_DCC_FEATURE
		pAd->RadioStatsCounter.TotalBeaconSentCount += TxStaCnt0.field.TxBeaconCount;
		pAd->RadioStatsCounter.TotalTxCount += (StaTx1.field.TxSuccess + TxStaCnt0.field.TxFailCount + StaTx1.field.TxRetransmit + TxStaCnt0.field.TxBeaconCount);
		pAd->RadioStatsCounter.TxRetryCount += StaTx1.field.TxRetransmit;
#endif
		AccuTxTotalCnt = pAd->RalinkCounters.OneSecTxNoRetryOkCount +
							pAd->RalinkCounters.OneSecTxRetryOkCount +
							pAd->RalinkCounters.OneSecTxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

		/* Rssi is calculated again with new formula?In rory's code, the average instead of max is used. */
		if (pAd->Antenna.field.TxPath > 1)
			Rssi = (pEntry->RssiSample.AvgRssi0 + pEntry->RssiSample.AvgRssi1) >> 1;
		else
			Rssi = pEntry->RssiSample.AvgRssi0;

		TxCnt = AccuTxTotalCnt;
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;
		TxCnt = TxTotalCnt;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
	}

#ifdef FIFO_EXT_SUPPORT
		if (NicGetMacFifoTxCnt(pAd, pEntry))
		{
			ULONG 	HwTxCnt, HwErrRatio;

			HwTxCnt = pEntry->fifoTxSucCnt + pEntry->fifoTxRtyCnt;
			if (HwTxCnt)
				HwErrRatio = (pEntry->fifoTxRtyCnt * 100) / HwTxCnt;
			else
				HwErrRatio = 0;

			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("%s()=>Wcid:%d, MCS:%d, TxErrRation(Hw:0x%lx-0x%lx, Sw:0x%lx-%lx)\n",
					__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS,
					HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

			TxSuccess = pEntry->fifoTxSucCnt;
			TxRetransmit = pEntry->fifoTxRtyCnt;
			TxErrorRatio = HwErrRatio;
			TxTotalCnt = HwTxCnt;
			TxCnt = HwTxCnt;

			ApTxFailCntUpdate(pAd, pEntry, TxSuccess, TxRetransmit);
		}
#endif /*  FIFO_EXT_SUPPORT */

#ifdef NEW_RATE_ADAPT_QUICK_DOWN
/* QuickInitMCSRate now has bad algorithm and periodic drop PhyMode to CCK/0 */
    if(QuickInitMCSRate(pAd,pEntry, TxSuccess, TxRetransmit) == TRUE)
        return;
#endif

	DBGPRINT(RT_DEBUG_INFO, ("Quick PER %lu, Total Cnt %lu\n", TxErrorRatio, TxTotalCnt));

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));
		/*  reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);
		return;
	}
#endif	/*  MFB_SUPPORT */

	/*  Remember the current rate */
	CurrRateIdx = pEntry->CurrTxRateIndex;
#ifdef TXBF_SUPPORT
	CurrPhyETxBf = pEntry->phyETxBf;
	CurrPhyITxBf = pEntry->phyITxBf;
#endif /*  TXBF_SUPPORT */
	pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX) && pEntry->perThrdAdj == 1)
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


#ifdef DBG_CTRL_SUPPORT
	/*  Debug option: Concise RA log */
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_QUICK_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */

	/*  Handle the low traffic case */
	if ((TxCnt <= 15) && 
		(pEntry->HTPhyMode.field.MODE == MODE_HTMIX) &&
		(pEntry->HTPhyMode.field.MCS > 1))
	{
		/*  Go back to the original rate */
		MlmeRestoreLastRate(pEntry);
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA, ("   QuickDRS: TxTotalCnt <= 15, back to original rate \n"));

		MlmeNewTxRate(pAd, pEntry);


		// TODO: should we reset all OneSecTx counters?
		/* RESET_ONE_SEC_TX_CNT(pEntry); */

		return;
	}

	/*
		Compare throughput.
		LastTxCount is based on a time interval of 500 msec or "500 - pAd->ra_fast_interval" ms.
	*/
	if ((pEntry->LastTimeTxRateChangeAction == RATE_NO_CHANGE)
#ifdef DBG_CTRL_SUPPORT
		&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
	)
		ratio = RA_INTERVAL / pAd->ra_fast_interval;
	else
		ratio = (RA_INTERVAL - pAd->ra_fast_interval) / pAd->ra_fast_interval;

	if (pAd->MacTab.Size == 1)
		OneSecTxNoRetryOKRationCount = (TxSuccess * ratio);
	else
		//OneSecTxNoRetryOKRationCount = pEntry->OneSecTxNoRetryOkCount * ratio + (pEntry->OneSecTxNoRetryOkCount >> 1);
        OneSecTxNoRetryOKRationCount = (TxSuccess * ratio) + ((TxSuccess * ratio) >> 1);

	/* Downgrade TX quality if PER >= Rate-Down threshold */
	/* the only situation when pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND but no rate change */
	if (TxErrorRatio >= TrainDown)
		MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;


	/*  Perform DRS - consider TxRate Down first, then rate up. */
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
	{
		BOOLEAN useOldRate;

		// TODO: gaa - Finalize the decision criterion
		/*
			0=>Throughput. Use New Rate if New TP is better than Old TP
			1=>PER. Use New Rate if New PER is less than the TrainDown PER threshold
			2=>Hybrid. Use rate with best TP if difference > 10%. Otherwise use rate with Best Estimated TP
			3=>Hybrid with check that PER<TrainDown Threshold
		*/
		if (pAd->CommonCfg.TrainUpRule == 0)
		{
			useOldRate = (pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount;
		}
		else if (pAd->CommonCfg.TrainUpRule==2 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else if (pAd->CommonCfg.TrainUpRule==3 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = (TxErrorRatio >= TrainDown) ||
						 MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else
			useOldRate = TxErrorRatio >= TrainDown;
		if (useOldRate)
		{
			/*  If PER>50% or TP<lastTP/2 then double the TxQuality delay */
			if ((TxErrorRatio > 50) || (OneSecTxNoRetryOKRationCount < pEntry->LastTxOkCount/2))
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
			else
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

			MlmeRestoreLastRate(pEntry);
		}
		else
		{
			RTMP_RA_GRP_TB *pLastTxRate = PTX_RA_GRP_ENTRY(pTable, pEntry->lastRateIdx);

			/*  Clear the history if we changed the MCS and PHY Rate */
			if ((pCurrTxRate->CurrMCS != pLastTxRate->CurrMCS) &&
				(pCurrTxRate->dataRate != pLastTxRate->dataRate))
				MlmeClearTxQuality(pEntry);

			if (pEntry->mcsGroup == 0)
				MlmeSetMcsGroup(pAd, pEntry);

			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,
						("   QuickDRS: (Up) keep rate-up (L:%ld, C:%ld)\n",
						pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
	}
	else if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
	{
		if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown)) /* there will be train down again */
		{
			MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) direct train down (TxErrorRatio >= TrainDown)\n"));
		}
		else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
		{
		    if(TxErrorRatio >= TrainUp)
            {
			    MlmeRestoreLastRate(pEntry);
			    DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) bad tx ok count (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		    }
		    else
		    {
                DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) direct train down (TxErrorRatio >= TrainUp)\n"));
            }
		}
		else
		{
			MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) keep rate-down (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
	}

	/*  See if we reverted to the old rate */
#ifdef TXBF_SUPPORT
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx) ||
				  (pEntry->phyETxBf!=CurrPhyETxBf) || (pEntry->phyITxBf!=CurrPhyITxBf);

	/*  Remember last good non-BF rate */
	if (!pEntry->phyETxBf && !pEntry->phyITxBf)
		pEntry->lastNonBfRate = pEntry->CurrTxRateIndex;
#else
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx);
#endif /*  TXBF_SUPPORT */


	/*  Update mcsGroup */
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
	{
		UCHAR UpRateIdx;

		/*  If RATE_UP failed look for the next group with valid mcs */
		if (pEntry->CurrTxRateIndex != CurrRateIdx && pEntry->mcsGroup > 0)
		{
			pEntry->mcsGroup--;
			pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, pEntry->lastRateIdx);
		}

		switch (pEntry->mcsGroup)
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
			default:
				UpRateIdx = CurrRateIdx;
				break;
		}

		if (UpRateIdx == pEntry->CurrTxRateIndex)
			pEntry->mcsGroup = 0;
	}


	/*  Handle change back to old rate */
	if (rateChanged)
	{
		/*  Clear Old Rate's TxQuality */
		MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);

		pEntry->TxRateUpPenalty = 0;	/* redundant */
		pEntry->PER[pEntry->CurrTxRateIndex] = 0;	/* redundant */

		/*  Set new Tx rate */
		MlmeNewTxRate(pAd, pEntry);
	}

	// TODO: should we reset all OneSecTx counters?
	/* RESET_ONE_SEC_TX_CNT(pEntry); */
}



/*
    ==========================================================================
    Description:
        This routine walks through the MAC table, see if TX rate change is
        required for each associated client.
    Output:
        pEntry->CurrTxRate -
    NOTE:
        call this routine every second
    ==========================================================================
 */
VOID APMlmeDynamicTxRateSwitchingAdapt(RTMP_ADAPTER *pAd, UINT i)
{
	PUCHAR pTable;
	UCHAR UpRateIdx, DownRateIdx, CurrRateIdx, TrainUp, TrainDown;
	ULONG TxTotalCnt, TxSuccess, TxRetransmit, TxFailCount, TxErrorRatio;
	MAC_TABLE_ENTRY *pEntry;
	RTMP_RA_GRP_TB *pCurrTxRate;
	CHAR Rssi;

	pEntry = &pAd->MacTab.Content[i]; /* point to information of the individual station */
	pTable = pEntry->pTable;
	TxTotalCnt = TxSuccess = TxRetransmit = TxFailCount = TxErrorRatio = 0;

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
#ifdef CUSTOMER_DCC_FEATURE
		pAd->RadioStatsCounter.TotalBeaconSentCount += TxStaCnt0.field.TxBeaconCount;
		pAd->RadioStatsCounter.TotalTxCount += (StaTx1.field.TxSuccess + TxStaCnt0.field.TxFailCount + StaTx1.field.TxRetransmit + TxStaCnt0.field.TxBeaconCount);
		pAd->RadioStatsCounter.TxRetryCount += StaTx1.field.TxRetransmit;
#endif
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
	}

#ifdef FIFO_EXT_SUPPORT
		if (NicGetMacFifoTxCnt(pAd, pEntry))
		{
			ULONG HwTxCnt, HwErrRatio;

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

			ApTxFailCntUpdate(pAd, pEntry, TxSuccess, TxRetransmit);
		}
#endif /*  FIFO_EXT_SUPPORT */

	/*  Save LastTxOkCount, LastTxPER and last MCS action for APQuickResponeForRateUpExec */
	pEntry->LastTxOkCount = TxSuccess;
	pEntry->LastTxPER = (UCHAR)TxErrorRatio;
	pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

	/* different calculation in APQuickResponeForRateUpExec() */
	Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

	if (pEntry->CurrTxRateIndex >= RATE_TABLE_SIZE(pTable))
		pEntry->CurrTxRateIndex = RATE_TABLE_SIZE(pTable) - 1;		

	/*  decide the next upgrade rate and downgrade rate, if any */
	CurrRateIdx = pEntry->CurrTxRateIndex;

	//Current Rate and Check Max MCS Rate
	MlmeLimitMaxRate(pAd, pEntry, &CurrRateIdx, RATE_NO_CHANGE);

	//Up Rate and Check Max MCS Rate
	pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);
	pEntry->CurrTxRateIndex = CurrRateIdx;
	UpRateIdx = MlmeSelectUpRate(pAd, pEntry, pCurrTxRate);
	MlmeLimitMaxRate(pAd, pEntry, &UpRateIdx, RATE_UP);

	//Down Rate
	DownRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);

	DBGPRINT(RT_DEBUG_TRACE, ("Average PER %lu, Cur %d, Up %d, Dn %d\n", TxErrorRatio,
								CurrRateIdx, UpRateIdx, DownRateIdx));

	DBGPRINT(RT_DEBUG_TRACE, ("RAA:Tx Quality 1SS %d, 2SS %d\n",
								pEntry->TxQuality[pCurrTxRate->upMcs1],
								pEntry->TxQuality[pCurrTxRate->upMcs2]));


#ifdef DOT11_N_SUPPORT
	/*
		when Rssi > -65, there is a lot of interference usually. therefore,
		the algorithm tends to choose the mcs lower than the optimal one.
		By increasing the thresholds, the chosen mcs will be closer to the
		optimal mcs
	*/
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX) && pEntry->perThrdAdj == 1)
	{
		TrainUp = (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown = (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /*  DOT11_N_SUPPORT */
	{
		TrainUp = pCurrTxRate->TrainUp;
		TrainDown = pCurrTxRate->TrainDown;
	}


#ifdef DBG_CTRL_SUPPORT
	/*  Debug option: Concise RA log */
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_NEW_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		RTMP_RA_LEGACY_TB *pNextTxRate;

		/* with this method mfb result can be applied every 500msec, instead of immediately */
		NdisAcquireSpinLock(&pEntry->fLastChangeAccordingMfbLock);
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		NdisReleaseSpinLock(&pEntry->fLastChangeAccordingMfbLock);
		APMlmeSetTxRate(pAd, pEntry, pEntry->LegalMfbRS);
		DBGPRINT(RT_DEBUG_INFO,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));
		MlmeClearAllTxQuality(pEntry); /* clear all history, same as train up, purpose??? */
		/*  reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

		pEntry->CurrTxRateIndex = (pEntry->LegalMfbRS)->ItemNo;
		pNextTxRate = (RTMP_RA_LEGACY_TB *) &pTable[(pEntry->CurrTxRateIndex+1)*10]; /* actually = pEntry->LegalMfbRS */
		return;
	}
#endif	/* MFB_SUPPORT */


	/* Handle low traffic case */
	if (TxTotalCnt <= 15)
	{
		pEntry->lowTrafficCount++;
		if (pEntry->lowTrafficCount >= pAd->CommonCfg.lowTrafficThrd)
		{
			UCHAR TxRateIdx;
			CHAR mcs[24];
			CHAR RssiOffset = 0;

            //pEntry->lowTrafficCount = 0;

			/* Check existence and get index of each MCS */
			MlmeGetSupportedMcsAdapt(pAd, pEntry, GI_400, mcs);

			if ((pTable == RateSwitchTable11BGN2S) || (pTable == RateSwitchTable11BGN2SForABand) ||
				(pTable == RateSwitchTable11N2S) || (pTable == RateSwitchTable11N2SForABand))
			{
				RssiOffset = -12;
			}
			else if (ADAPT_RATE_TABLE(pTable))
			{
				RssiOffset = -12;
			}
			else
			{
				RssiOffset = 5;
			}

            TxRateIdx = pEntry->CurrTxRateIndex;

               /* Select the Tx rate based on the RSSI */
	        if((pEntry->lowTrafficCount == pAd->CommonCfg.lowTrafficThrd) &&
	           (pEntry->LastSaveRateIdx != 0xFF)) {
                // Get Latest Rate Index to calibration once
                MlmeCaliRSSITable(pAd, pEntry, Rssi, pTable);
                TxRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, pEntry->RX_RSSI_MCS);
                DBGPRINT(RT_DEBUG_TRACE,("Update New MCS with Cali RSSI:%d\n", pEntry->RX_RSSI_MCS));
	        }
	        else if((pEntry->lowTrafficCount > 60) || (pEntry->LastSaveRateIdx == 0xFF)) {
                // 30 second and First Time
                // Low Packet with holding safe MCS Rate
                TxRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, RssiOffset);
       			pEntry->lowTrafficCount = pAd->CommonCfg.lowTrafficThrd;
       			DBGPRINT(RT_DEBUG_TRACE,("Update New MCS with RSSI\n"));
	        }

			MlmeLimitMaxRate(pAd, pEntry, &TxRateIdx, RATE_NO_CHANGE);
			pEntry->lastRateIdx = pEntry->CurrTxRateIndex;
			MlmeSetMcsGroup(pAd, pEntry);
			pEntry->CurrTxRateIndex = TxRateIdx;

#ifdef TXBF_SUPPORT
			//pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
#endif /* TXBF_SUPPORT */
			MlmeNewTxRate(pAd, pEntry);
			if (!pEntry->fLastSecAccordingRSSI)
			{
				DBGPRINT(RT_DEBUG_INFO,("DRS: TxTotalCnt <= 15, switch to MCS%d according to RSSI (%d), RssiOffset=%d\n", pEntry->HTPhyMode.field.MCS, Rssi, RssiOffset));
			}

			MlmeClearAllTxQuality(pEntry);	/* clear all history */
			pEntry->fLastSecAccordingRSSI = TRUE;
		}

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
#ifdef DBG_CTRL_SUPPORT
		/* In Unaware mode always try to send sounding */
		if (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
			eTxBFProbing(pAd, pEntry);
#endif /* DBG_CTRL_SUPPORT */
#endif /* TXBF_SUPPORT */

        pEntry->fLastSecAccordingRSSI = TRUE;

#ifdef NEW_RATE_ADAPT_QUICK_DOWN
/* QuickInitMCSRate now has bad algorithm and periodic drop PhyMode to CCK/0 */
        if(TxTotalCnt) {
    		TriggerQuickInitMCSRate(pAd, pEntry, pAd->ra_fast_interval >> 1);
    		DBGPRINT(RT_DEBUG_TRACE ,("Trigger @ RSSI Mapping (Light Packet)\n"));
    		MlmeClearAllTxQuality(pEntry);	/* clear all history */
        }
#endif
		return;
	}

    if(TxTotalCnt > 100)
	pEntry->lowTrafficCount = 0;

	/*
		After pEntry->fLastSecAccordingRSSI = TRUE; the for loop
		continue. this condition is true when RateSwitching() is run
		next time.
		so the next rate adaptation is skipped. This mechanism is
		deliberately designed by rory.
	*/
	if (pEntry->fLastSecAccordingRSSI == TRUE)
	{
		pEntry->fLastSecAccordingRSSI = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		/* DBGPRINT(RT_DEBUG_INFO,("DRS: MCS is according to RSSI, and ignore tuning this sec \n")); */

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef NEW_RATE_ADAPT_QUICK_DOWN
/* QuickInitMCSRate now has bad algorithm and periodic drop PhyMode to CCK/0 */
        if(TxTotalCnt) {
    		TriggerQuickInitMCSRate(pAd, pEntry, pAd->ra_fast_interval >> 1);
	    	DBGPRINT(RT_DEBUG_TRACE ,("Trigger @ RSSI Mapping (Light to Heavy Packet)\n"));
		    MlmeClearAllTxQuality(pEntry);	/* clear all history */
        }
#endif
		return;
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	/* Select rate based on PER */
	MlmeNewRateAdapt(pAd, pEntry, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);

#ifdef DOT11N_SS3_SUPPORT
	/* Turn off RDG when 3s and rx count > tx count*5 */
	MlmeCheckRDG(pAd, pEntry);
#endif /* DOT11N_SS3_SUPPORT */

	/* reset all OneSecTx counters */
	RESET_ONE_SEC_TX_CNT(pEntry);
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID StaQuickResponeForRateUpExecAdapt(
	IN PRTMP_ADAPTER	pAd,
	IN ULONG i,
	IN CHAR  Rssi)
{
	PUCHAR					pTable;
	UCHAR					CurrRateIdx;
	ULONG					TxTotalCnt;
	ULONG					TxErrorRatio = 0;
	PMAC_TABLE_ENTRY		pEntry;
	RTMP_RA_GRP_TB *pCurrTxRate;
	UCHAR					TrainUp, TrainDown;
	CHAR					ratio;
	ULONG					TxSuccess, TxRetransmit, TxFailCount;
	ULONG					OneSecTxNoRetryOKRationCount;
	BOOLEAN					rateChanged;
#ifdef TXBF_SUPPORT
	BOOLEAN					CurrPhyETxBf, CurrPhyITxBf;
#endif /* TXBF_SUPPORT */


	pEntry = &pAd->MacTab.Content[i];
	pTable = pEntry->pTable;

	if (pAd->MacTab.Size == 1)
	{
		TX_STA_CNT1_STRUC		StaTx1;
		TX_STA_CNT0_STRUC		TxStaCnt0;

		/* Update statistic counter */
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);

		TxRetransmit = StaTx1.field.TxRetransmit;
		TxSuccess = StaTx1.field.TxSuccess;
		TxFailCount = TxStaCnt0.field.TxFailCount;
	}
	else
	{
		TxRetransmit = pEntry->OneSecTxRetryOkCount;
		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxFailCount = pEntry->OneSecTxFailCount;
	}

	TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;
	if (TxTotalCnt)
		TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);
		return;
	}
#endif	/* MFB_SUPPORT */

    if(QuickInitMCSRate(pAd,pEntry, TxSuccess, TxRetransmit) == TRUE)
        return;

	/* Remember the current rate */
	CurrRateIdx = pEntry->CurrTxRateIndex;
#ifdef TXBF_SUPPORT
	CurrPhyETxBf = pEntry->phyETxBf;
	CurrPhyITxBf = pEntry->phyITxBf;
#endif /* TXBF_SUPPORT */
	pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX) && pEntry->perThrdAdj == 1)
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Concise RA log */
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_QUICK_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */

	/*
		CASE 1. when TX samples are fewer than 15, then decide TX rate solely on RSSI
		     (criteria copied from RT2500 for Netopia case)
	*/
	if (TxTotalCnt <= 12)
	{
		/* Go back to the original rate */
		MlmeRestoreLastRate(pEntry);
		DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: TxTotalCnt <= 12, back to original rate \n"));

		MlmeNewTxRate(pAd, pEntry);

		// TODO: should we reset all OneSecTx counters?
		/* RESET_ONE_SEC_TX_CNT(pEntry); */
		return;
	}

	/*
		Compare throughput.
		LastTxCount is based on a time interval of 500 msec or "500-pAd->ra_fast_interval" ms.
	*/
	if ((pEntry->LastTimeTxRateChangeAction == RATE_NO_CHANGE)
#ifdef DBG_CTRL_SUPPORT
		&& (pAd->CommonCfg.DebugFlags & DBF_FORCE_QUICK_DRS)==0
#endif /* DBG_CTRL_SUPPORT */
	)
		ratio = RA_INTERVAL/pAd->ra_fast_interval;
	else
		ratio = (RA_INTERVAL-pAd->ra_fast_interval)/pAd->ra_fast_interval;

	OneSecTxNoRetryOKRationCount = (TxSuccess * ratio);

	/* Downgrade TX quality if PER >= Rate-Down threshold */
	if (TxErrorRatio >= TrainDown)
	{
		MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND); /* the only situation when pEntry->TxQuality[CurrRateIdx] = DRS_TX_QUALITY_WORST_BOUND but no rate change */
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	/* Perform DRS - consider TxRate Down first, then rate up. */
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
	{
		BOOLEAN useOldRate;

		// TODO: gaa - Finalize the decision criterion
		/*
			0=>Throughput. Use New Rate if New TP is better than Old TP
			1=>PER. Use New Rate if New PER is less than the TrainDown PER threshold
			2=>Hybrid. Use rate with best TP if difference > 10%. Otherwise use rate with Best Estimated TP
			3=>Hybrid with check that PER<TrainDown Threshold
		*/
		if (pAd->CommonCfg.TrainUpRule == 0)
		{
			useOldRate = (pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount;
		}
		else if (pAd->CommonCfg.TrainUpRule==2 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else if (pAd->CommonCfg.TrainUpRule==3 && Rssi<=pAd->CommonCfg.TrainUpRuleRSSI)
		{
			useOldRate = (TxErrorRatio >= TrainDown) ||
						 MlmeRAHybridRule(pAd, pEntry, pCurrTxRate, OneSecTxNoRetryOKRationCount, TxErrorRatio);
		}
		else
			useOldRate = TxErrorRatio >= TrainDown;
		if (useOldRate)
		{
			/* If PER>50% or TP<lastTP/2 then double the TxQuality delay */
			if ((TxErrorRatio > 50) || (OneSecTxNoRetryOKRationCount < pEntry->LastTxOkCount/2))
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND*2);
			else
				MlmeSetTxQuality(pEntry, CurrRateIdx, DRS_TX_QUALITY_WORST_BOUND);

			MlmeRestoreLastRate(pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Up) bad tx ok count (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
		else
		{
			RTMP_RA_GRP_TB *pLastTxRate = PTX_RA_GRP_ENTRY(pTable, pEntry->lastRateIdx);

			/* Clear the history if we changed the MCS and PHY Rate */
			if ((pCurrTxRate->CurrMCS != pLastTxRate->CurrMCS) &&
				(pCurrTxRate->dataRate != pLastTxRate->dataRate))
				MlmeClearTxQuality(pEntry);

			if (pEntry->mcsGroup == 0)
				MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Up) keep rate-up (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
	}
	else if (pEntry->LastSecTxRateChangeAction == RATE_DOWN)
	{
		if ((TxErrorRatio >= 50) || (TxErrorRatio >= TrainDown))
		{
			MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) direct train down (TxErrorRatio >= TrainDown)\n"));
		}
		else if ((pEntry->LastTxOkCount + 2) >= OneSecTxNoRetryOKRationCount)
		{
			MlmeRestoreLastRate(pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) bad tx ok count (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
		else
		{
			MlmeSetMcsGroup(pAd, pEntry);
			DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_RA,("   QuickDRS: (Down) keep rate-down (L:%ld, C:%ld)\n", pEntry->LastTxOkCount, OneSecTxNoRetryOKRationCount));
		}
	}

	/* See if we reverted to the old rate */
#ifdef TXBF_SUPPORT
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx) ||
				  (pEntry->phyETxBf!=CurrPhyETxBf) || (pEntry->phyITxBf!=CurrPhyITxBf);

	/* Remember last good non-BF rate */
	if (!pEntry->phyETxBf && !pEntry->phyITxBf)
		pEntry->lastNonBfRate = pEntry->CurrTxRateIndex;
#else
	rateChanged = (pEntry->CurrTxRateIndex != CurrRateIdx);
#endif /* TXBF_SUPPORT */

	/* Update mcsGroup */
	if (pEntry->LastSecTxRateChangeAction == RATE_UP)
	{
		UCHAR UpRateIdx;

		/* If RATE_UP failed look for the next group with valid mcs */
		if (pEntry->CurrTxRateIndex != CurrRateIdx && pEntry->mcsGroup > 0)
		{
			pEntry->mcsGroup--;
			pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, pEntry->lastRateIdx);
		}

		switch (pEntry->mcsGroup)
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
			default:
				UpRateIdx = CurrRateIdx;
				break;
		}

		if (UpRateIdx == pEntry->CurrTxRateIndex)
			pEntry->mcsGroup = 0;
	}

	/* Handle change back to old rate */
	if (rateChanged)
	{
		/* Clear Old Rate's history */
		MlmeSetTxQuality(pEntry, pEntry->CurrTxRateIndex, 0);
		pEntry->TxRateUpPenalty = 0;/*redundant */
		pEntry->PER[pEntry->CurrTxRateIndex] = 0;/*redundant */

		/* Set new Tx rate */
		MlmeNewTxRate(pAd, pEntry);
	}

	// TODO: should we reset all OneSecTx counters?
	/* RESET_ONE_SEC_TX_CNT(pEntry); */
}


VOID MlmeDynamicTxRateSwitchingAdapt(
    IN PRTMP_ADAPTER pAd,
	IN ULONG i,
	IN ULONG TxSuccess,
	IN ULONG TxRetransmit,
	IN ULONG TxFailCount)
{
	PUCHAR			  pTable;
	UCHAR			  UpRateIdx, DownRateIdx, CurrRateIdx;
	ULONG			  TxTotalCnt;
	ULONG			  TxErrorRatio = 0;
	MAC_TABLE_ENTRY	  *pEntry;
	RTMP_RA_GRP_TB *pCurrTxRate;
	UCHAR			  TrainUp, TrainDown;
	CHAR			  Rssi;

	pEntry = &pAd->MacTab.Content[i];
	pTable = pEntry->pTable;

	if ((pAd->MacTab.Size == 1) || (IS_ENTRY_DLS(pEntry)))
	{
		Rssi = RTMPAvgRssi(pAd, &pAd->StaCfg.RssiSample);

		/* Update statistic counter */
		TxTotalCnt = TxRetransmit + TxSuccess + TxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((TxRetransmit + TxFailCount) * 100) / TxTotalCnt;
	}
	else
	{
		Rssi = RTMPAvgRssi(pAd, &pEntry->RssiSample);

		TxSuccess = pEntry->OneSecTxNoRetryOkCount;
		TxTotalCnt = pEntry->OneSecTxNoRetryOkCount +
					 pEntry->OneSecTxRetryOkCount +
					 pEntry->OneSecTxFailCount;

		if (TxTotalCnt)
			TxErrorRatio = ((pEntry->OneSecTxRetryOkCount + pEntry->OneSecTxFailCount) * 100) / TxTotalCnt;

#ifdef FIFO_EXT_SUPPORT
		if (pEntry->bUseHwFifoExt)
		{
			WCID_TX_CNT_STRUC wcidTxCnt;
			ULONG HwTxCnt, HwErrRatio = 0;
			
			RTMP_IO_READ32(pAd, pAd->FifoExtTbl[pEntry->hwFifoExtIdx].hwTxCntCROffset, &wcidTxCnt.word);

			HwTxCnt = wcidTxCnt.field.succCnt + wcidTxCnt.field.reTryCnt;
			if (HwTxCnt)
				HwErrRatio = (wcidTxCnt.field.reTryCnt * 100) / HwTxCnt;

			DBGPRINT(RT_DEBUG_TRACE ,("%s():TxErrRatio(Wcid:%d, MCS:%d, Hw:0x%x-0x%x, Sw:0x%x-%x)\n",
					__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.field.MCS,
					HwTxCnt, HwErrRatio, TxTotalCnt, TxErrorRatio));

			TxSuccess = wcidTxCnt.field.succCnt;
			TxRetransmit = wcidTxCnt.field.reTryCnt;
			TxErrorRatio = HwErrRatio;
			TxTotalCnt = HwTxCnt;
		}
#endif /* FIFO_EXT_SUPPORT */
	}

	/* Save LastTxOkCount, LastTxPER and last MCS action for StaQuickResponeForRateUpExec */
	pEntry->LastTxOkCount = TxSuccess;
	pEntry->LastTxPER = (UCHAR)TxErrorRatio;
	pEntry->LastTimeTxRateChangeAction = pEntry->LastSecTxRateChangeAction;

	if (pEntry->CurrTxRateIndex >= RATE_TABLE_SIZE(pTable))
		pEntry->CurrTxRateIndex = RATE_TABLE_SIZE(pTable) - 1;	

	/* decide the next upgrade rate and downgrade rate, if any */
	CurrRateIdx = pEntry->CurrTxRateIndex;
	pCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);
	UpRateIdx = MlmeSelectUpRate(pAd, pEntry, pCurrTxRate);
	DownRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);

#ifdef DOT11_N_SUPPORT
	/*
		when Rssi > -65, there is a lot of interference usually. therefore, the algorithm tends to choose the mcs lower than the optimal one.
		by increasing the thresholds, the chosen mcs will be closer to the optimal mcs
	*/
	if ((Rssi > -65) && (pCurrTxRate->Mode >= MODE_HTMIX) && pEntry->perThrdAdj == 1)
	{
		TrainUp		= (pCurrTxRate->TrainUp + (pCurrTxRate->TrainUp >> 1));
		TrainDown	= (pCurrTxRate->TrainDown + (pCurrTxRate->TrainDown >> 1));
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		TrainUp		= pCurrTxRate->TrainUp;
		TrainDown	= pCurrTxRate->TrainDown;
	}


#ifdef DBG_CTRL_SUPPORT
	/* Debug option: Concise RA log */
	if ((pAd->CommonCfg.DebugFlags & DBF_SHOW_RA_LOG) || (pAd->CommonCfg.DebugFlags & DBF_DBQ_RA_LOG))
		MlmeRALog(pAd, pEntry, RAL_NEW_DRS, TxErrorRatio, TxTotalCnt);
#endif /* DBG_CTRL_SUPPORT */

#ifdef MFB_SUPPORT
	if (pEntry->fLastChangeAccordingMfb == TRUE)
	{
		pEntry->fLastChangeAccordingMfb = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: MCS is according to MFB, and ignore tuning this sec \n"));

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

		return;
	}
#endif	/* MFB_SUPPORT */


	/*
		CASE 1. when TX samples are fewer than 15, then decide TX rate solely on RSSI
		     (criteria copied from RT2500 for Netopia case)
	*/
	if (TxTotalCnt <= 15)
	{
		pEntry->lowTrafficCount++;

		if (pEntry->lowTrafficCount >= pAd->CommonCfg.lowTrafficThrd)
		{
			UCHAR	TxRateIdx;
			CHAR	mcs[24];
			CHAR	RssiOffset = 0;

			pEntry->lowTrafficCount = 0;

			/* Check existence and get index of each MCS */
			MlmeGetSupportedMcsAdapt(pAd, pEntry, GI_800, mcs);

			if (pAd->LatchRfRegs.Channel <= 14)
			{
				if (pAd->NicConfig2.field.ExternalLNAForG)
				{
					RssiOffset = 2;
				}
				else if (ADAPT_RATE_TABLE(pTable))
				{
					RssiOffset = -12;
				}
				else
				{
					RssiOffset = 5;
				}
			}
			else
			{
				if (pAd->NicConfig2.field.ExternalLNAForA)
				{
					RssiOffset = 5;
				}
				else if (ADAPT_RATE_TABLE(pTable))
				{
					RssiOffset = -12;
				}
				else
				{
					RssiOffset = 8;
				}
			}

			//Low Packet with safe MCS Rate
			if(pEntry->LastSaveRateIdx > 0)
			{
				/* Select the Tx rate based on the RSSI */
				if(pEntry->lowTrafficCount > 120) // 60 second
			{
				pEntry->LastSaveRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, RssiOffset);
				pEntry->lowTrafficCount = pAd->CommonCfg.lowTrafficThrd;
			}

				TxRateIdx = pEntry->LastSaveRateIdx;
				DBGPRINT(RT_DEBUG_INFO,("Save Rate %d %d\n", TxRateIdx, pEntry->lowTrafficCount));
			}
			else
			{
			/* Select the Tx rate based on the RSSI */
			TxRateIdx = MlmeSelectTxRateAdapt(pAd, pEntry, mcs, Rssi, RssiOffset);
			}

			/* if (TxRateIdx != pEntry->CurrTxRateIndex) */
			{
				pEntry->lastRateIdx = pEntry->CurrTxRateIndex;
				MlmeSetMcsGroup(pAd, pEntry);

				pEntry->CurrTxRateIndex = TxRateIdx;
#ifdef TXBF_SUPPORT
				//pEntry->phyETxBf = pEntry->phyITxBf = FALSE;
#endif /* TXBF_SUPPORT */
				MlmeNewTxRate(pAd, pEntry);
				if (!pEntry->fLastSecAccordingRSSI)
				{
					DBGPRINT_RAW(RT_DEBUG_INFO,("DRS: TxTotalCnt <= 15, switch to MCS%d according to RSSI (%d), RssiOffset=%d\n", pEntry->HTPhyMode.field.MCS, Rssi, RssiOffset));
				}
			}

			MlmeClearAllTxQuality(pEntry);	/* clear all history */
			pEntry->fLastSecAccordingRSSI = TRUE;
		}

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);

#ifdef TXBF_SUPPORT
#ifdef DBG_CTRL_SUPPORT
		/* In Unaware mode always try to send sounding */
		if (pAd->CommonCfg.DebugFlags & DBF_NO_BF_AWARE_RA)
			eTxBFProbing(pAd, pEntry);
#endif /* DBG_CTRL_SUPPORT */
#endif /* TXBF_SUPPORT */

		if (TxTotalCnt > 0)
		{
			TriggerQuickInitMCSRate(pAd, pEntry, pAd->ra_fast_interval >> 1);
		}

		return;
	}

	pEntry->lowTrafficCount = 0;

	if (pEntry->fLastSecAccordingRSSI == TRUE)
	{
		pEntry->fLastSecAccordingRSSI = FALSE;
		pEntry->LastSecTxRateChangeAction = RATE_NO_CHANGE;
		/* DBGPRINT_RAW(RT_DEBUG_TRACE,("DRS: MCS is according to RSSI, and ignore tuning this sec \n")); */

		TriggerQuickInitMCSRate(pAd, pEntry, pAd->ra_fast_interval >> 1);
		DBGPRINT(RT_DEBUG_TRACE ,("Trigger @ RSSI Mapping\n"));

		/* reset all OneSecTx counters */
		RESET_ONE_SEC_TX_CNT(pEntry);
		return;
	}

	pEntry->PER[CurrRateIdx] = (UCHAR)TxErrorRatio;

	/* Select rate based on PER */
	MlmeNewRateAdapt(pAd, pEntry, UpRateIdx, DownRateIdx, TrainUp, TrainDown, TxErrorRatio);

#ifdef DOT11_VHT_AC
    {
        RTMP_RA_GRP_TB *pAGSCurrTxRate;

        pAGSCurrTxRate = PTX_RA_GRP_ENTRY(pTable, CurrRateIdx);

        DBGPRINT(RT_DEBUG_TRACE, ("Current Rate is %d\n",CurrRateIdx));

        if((pEntry->pTable == RateTableVht1S) || (pEntry->pTable == RateTableVht2S)
            || (pEntry->pTable == RateTableVht2S_BW20) || (pEntry->pTable == RateTableVht2S_BW40)
            || (pEntry->pTable == RateTableVht2S_MCS7) || (pEntry->pTable == RateTableVht1S_2G_BW20)
            || (pEntry->pTable == RateTableVht1S_2G_BW40) || (pEntry->pTable == RateTableVht2S_2G_BW20)
            || (pEntry->pTable == RateTableVht2S_2G_BW40))
        {
            if(pAGSCurrTxRate->dataRate == 2)
            {
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, 0xBBA98800);
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xEDCBA980);
            }
            else
            {
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, 0x65432100);
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xEDC74320);
            }

        }
        else if ((pEntry->pTable == RateSwitchTableAdapt11N1S) || (pEntry->pTable == RateSwitchTableAdapt11N2S))
        {
            if(pAGSCurrTxRate->CurrMCS > 7)
            {
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, 0xBBA98800);
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xEDCBA980);
            }
            else
            {
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, 0x65432100);
                RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xEDC74320);
            }
        }
        else
        {
            RTMP_IO_WRITE32(pAd, HT_FBK_CFG0, 0x65432100);
            RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xEDCBA980);
        }
    }
#endif


#ifdef DOT11N_SS3_SUPPORT
	/* Turn off RDG when 3SS and rx count > tx count*5 */
	MlmeCheckRDG(pAd, pEntry);
#endif /* DOT11N_SS3_SUPPORT */

	/* reset all OneSecTx counters */
	RESET_ONE_SEC_TX_CNT(pEntry);
}
#endif /* CONFIG_STA_SUPPORT */


/*
	Set_RateTable_Proc - Display or replace byte for item in RateSwitchTableAdapt11N3S
		usage: iwpriv ra0 set RateTable=<item>[:<offset>:<value>]
*/
INT Set_RateTable_Proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UCHAR *pTable, TableSize, InitTxRateIdx;
	int i;
	MAC_TABLE_ENTRY *pEntry;
	int itemNo, rtIndex, value;
	UCHAR *pRateEntry;

	/* Find first Associated STA in MAC table */
	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pEntry) && pEntry->Sst==SST_ASSOC)
			break;
	}

	if (i==MAX_LEN_OF_MAC_TABLE)
	{
	    DBGPRINT(RT_DEBUG_ERROR, ("Set_RateTable_Proc: Empty MAC Table\n"));
		return FALSE;
	}

	/* Get peer's rate table */
	MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &InitTxRateIdx);

	/* Get rate index */
	itemNo = simple_strtol(arg, &arg, 10);
	if (itemNo<0 || itemNo>=RATE_TABLE_SIZE(pTable))
		return FALSE;

#ifdef NEW_RATE_ADAPT_SUPPORT
	if (ADAPT_RATE_TABLE(pTable))
		pRateEntry = (UCHAR *)PTX_RA_GRP_ENTRY(pTable, itemNo);
	else
#endif /* NEW_RATE_ADAPT_SUPPORT */
		pRateEntry = (UCHAR *)PTX_RA_LEGACY_ENTRY(pTable, itemNo);

	/* If no addtional parameters then print the entry */
	if (*arg != ':') {
		DBGPRINT(RT_DEBUG_OFF, ("Set_RateTable_Proc::%d\n", itemNo));
	}
	else {
		/* Otherwise get the offset and the replace byte */
		while (*arg<'0' || *arg>'9')
			arg++;
		rtIndex = simple_strtol(arg, &arg, 10);
		if (rtIndex<0 || rtIndex>9)
			return FALSE;

		if (*arg!=':')
			return FALSE;
		while (*arg<'0' || *arg>'9')
			arg++;
		value = simple_strtol(arg, &arg, 10);
		pRateEntry[rtIndex] = value;
		DBGPRINT(RT_DEBUG_OFF, ("Set_RateTable_Proc::%d:%d:%d\n", itemNo, rtIndex, value));
	}

    DBGPRINT(RT_DEBUG_OFF, ("%d, 0x%02x, %d, %d, %d, %d, %d, %d, %d, %d\n",
		pRateEntry[0], pRateEntry[1], pRateEntry[2], pRateEntry[3], pRateEntry[4],
		pRateEntry[5], pRateEntry[6], pRateEntry[7], pRateEntry[8], pRateEntry[9]));

	return TRUE;
}


INT	Set_PerThrdAdj_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	UCHAR i;
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++){
		pAd->MacTab.Content[i].perThrdAdj = simple_strtol(arg, 0, 10);
	}
	return TRUE;
}

/* Set_LowTrafficThrd_Proc - set threshold for reverting to default MCS based on RSSI */
INT	Set_LowTrafficThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.lowTrafficThrd = simple_strtol(arg, 0, 10);

	return TRUE;
}

/* Set_TrainUpRule_Proc - set rule for Quick DRS train up */
INT	Set_TrainUpRule_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.TrainUpRule = simple_strtol(arg, 0, 10);

	return TRUE;
}

/* Set_TrainUpRuleRSSI_Proc - set RSSI threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpRuleRSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.TrainUpRuleRSSI = simple_strtol(arg, 0, 10);

	return TRUE;
}

/* Set_TrainUpLowThrd_Proc - set low threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpLowThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.TrainUpLowThrd = simple_strtol(arg, 0, 10);

	return TRUE;
}

/* Set_TrainUpHighThrd_Proc - set high threshold for Quick DRS Hybrid train up */
INT	Set_TrainUpHighThrd_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{
	pAd->CommonCfg.TrainUpHighThrd = simple_strtol(arg, 0, 10);

	return TRUE;
}

#endif /* NEW_RATE_ADAPT_SUPPORT */

