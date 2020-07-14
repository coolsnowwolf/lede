/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


INT tx_temp_dbg = 0;

#ifdef RTMP_TEMPERATURE_COMPENSATION

/*
	Lookup table format:

ArrayIdx:
			0	   1		2		10	11	12	13	14	......			46

PwrCompen:	-12    -11   -10  ......   -2	-1	0	1	2	.......	31	32
TempRange:								-4	3	10	.......	114	116
										 |
										\/
									    Temperature reference base

	PwrCompensation in unit of 0.5dB
	TempRange in temperature sensor value
*/

#define LUT_POSITIVE(_band, _idx)		pAd->TxPowerCtrl.LookupTable[(_band)][(_idx) + Offset]
#define LUT_NEGATIVE(_band, _idx)		pAd->TxPowerCtrl.LookupTable[(_band)][Offset - (_idx)]
#define LUT_BASE(_band)					pAd->TxPowerCtrl.LookupTable[(_band)][Offset]
#define LUT_INDEX(_band, _idx)			pAd->TxPowerCtrl.LookupTable[(_band)][(_idx)]


VOID InitLookupTable(
	IN PRTMP_ADAPTER pAd)
{
	int lut_idx, IdxTmp, pos_idx, pos_bound, neg_idx, neg_bound;
	int step_cnt, comp_val;
	enum IEEE80211_BAND band;
	int band_nums = 1, Offset;
	EEPROM_WORD_STRUC WordStruct;
	UCHAR PlusStepNum[IEEE80211_BAND_NUMS][8];
	UCHAR MinusStepNum[IEEE80211_BAND_NUMS][8];
	UCHAR Step[IEEE80211_BAND_NUMS];


	DBGPRINT(RT_DEBUG_TRACE, ("==> InitLookupTable\n"));
	
	/* Read from EEPROM, as parameters for lookup table for G band */
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 2.4G] EEPROM\n"));
	RT28xx_EEPROM_READ16(pAd, 0x6e, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("\t6e = 0x%x\n", WordStruct.word));
	PlusStepNum[IEEE80211_BAND_2G][0] = (WordStruct.field.Byte0 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][2] = (WordStruct.field.Byte1 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x70, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("\t70 = 0x%x\n", WordStruct.word));
	PlusStepNum[IEEE80211_BAND_2G][4] = (WordStruct.field.Byte0 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][6] = (WordStruct.field.Byte1 & 0x0F);
	PlusStepNum[IEEE80211_BAND_2G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x72, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("\t72 = 0x%x\n", WordStruct.word));
	MinusStepNum[IEEE80211_BAND_2G][0] = (WordStruct.field.Byte0 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][2] = (WordStruct.field.Byte1 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x74, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("\t74 = 0x%x\n", WordStruct.word));
	MinusStepNum[IEEE80211_BAND_2G][4] = (WordStruct.field.Byte0 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][6] = (WordStruct.field.Byte1 & 0x0F);
	MinusStepNum[IEEE80211_BAND_2G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

	RT28xx_EEPROM_READ16(pAd, 0x76, WordStruct.word);
	DBGPRINT(RT_DEBUG_TRACE, ("\t76 = 0x%x\n", WordStruct.word));
	pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_2G] = (WordStruct.field.Byte0 & 0x0F);
	Step[IEEE80211_BAND_2G] = (WordStruct.field.Byte0 >> 4);
	pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G] = (CHAR)WordStruct.field.Byte1;

	DBGPRINT(RT_DEBUG_TRACE, ("\tPlus = %u %u %u %u %u %u %u %u\n",
		PlusStepNum[IEEE80211_BAND_2G][0],
		PlusStepNum[IEEE80211_BAND_2G][1],
		PlusStepNum[IEEE80211_BAND_2G][2],
		PlusStepNum[IEEE80211_BAND_2G][3],
		PlusStepNum[IEEE80211_BAND_2G][4],
		PlusStepNum[IEEE80211_BAND_2G][5],
		PlusStepNum[IEEE80211_BAND_2G][6],
		PlusStepNum[IEEE80211_BAND_2G][7]
		));
	DBGPRINT(RT_DEBUG_TRACE, ("\tMinus = %u %u %u %u %u %u %u %u\n",
		MinusStepNum[IEEE80211_BAND_2G][0],
		MinusStepNum[IEEE80211_BAND_2G][1],
		MinusStepNum[IEEE80211_BAND_2G][2],
		MinusStepNum[IEEE80211_BAND_2G][3],
		MinusStepNum[IEEE80211_BAND_2G][4],
		MinusStepNum[IEEE80211_BAND_2G][5],
		MinusStepNum[IEEE80211_BAND_2G][6],
		MinusStepNum[IEEE80211_BAND_2G][7]
		));
	DBGPRINT(RT_DEBUG_TRACE, ("\ttssi gain/step = %u\n", pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_2G]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tStep = %u\n", Step[IEEE80211_BAND_2G]));
	DBGPRINT(RT_DEBUG_TRACE, ("\tRefTemp = %d\n", pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G]));

#ifdef A_BAND_SUPPORT
	if (RFIC_IS_5G_BAND(pAd))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation 5G] EEPROM\n"));
		/* Read from EEPROM, as parameters for lookup table for A band */
		RT28xx_EEPROM_READ16(pAd, 0xd4, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("\td4 = 0x%x\n", WordStruct.word));
		PlusStepNum[IEEE80211_BAND_5G][0] = (WordStruct.field.Byte0 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][2] = (WordStruct.field.Byte1 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xd6, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("\td6 = 0x%x\n", WordStruct.word));
		PlusStepNum[IEEE80211_BAND_5G][4] = (WordStruct.field.Byte0 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][6] = (WordStruct.field.Byte1 & 0x0F);
		PlusStepNum[IEEE80211_BAND_5G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xd8, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("\td8 = 0x%x\n", WordStruct.word));
		MinusStepNum[IEEE80211_BAND_5G][0] = (WordStruct.field.Byte0 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][1] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][2] = (WordStruct.field.Byte1 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][3] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xda, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("\tda = 0x%x\n", WordStruct.word));
		MinusStepNum[IEEE80211_BAND_5G][4] = (WordStruct.field.Byte0 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][5] = (((WordStruct.field.Byte0 & 0xF0) >> 4) & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][6] = (WordStruct.field.Byte1 & 0x0F);
		MinusStepNum[IEEE80211_BAND_5G][7] = (((WordStruct.field.Byte1 & 0xF0) >> 4) & 0x0F);

		RT28xx_EEPROM_READ16(pAd, 0xdc, WordStruct.word);
		DBGPRINT(RT_DEBUG_TRACE, ("\tdc = 0x%x\n", WordStruct.word));
		pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_5G] = (WordStruct.field.Byte0 & 0x0F);
		Step[IEEE80211_BAND_5G] = (WordStruct.field.Byte0 >> 4);
		pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G] = (CHAR)WordStruct.field.Byte1;

		DBGPRINT(RT_DEBUG_TRACE, ("\tPlus = %u %u %u %u %u %u %u %u\n",
			PlusStepNum[IEEE80211_BAND_5G][0],
			PlusStepNum[IEEE80211_BAND_5G][1],
			PlusStepNum[IEEE80211_BAND_5G][2],
			PlusStepNum[IEEE80211_BAND_5G][3],
			PlusStepNum[IEEE80211_BAND_5G][4],
			PlusStepNum[IEEE80211_BAND_5G][5],
			PlusStepNum[IEEE80211_BAND_5G][6],
			PlusStepNum[IEEE80211_BAND_5G][7]
			));
		DBGPRINT(RT_DEBUG_TRACE, ("\tMinus = %u %u %u %u %u %u %u %u\n",
			MinusStepNum[IEEE80211_BAND_5G][0],
			MinusStepNum[IEEE80211_BAND_5G][1],
			MinusStepNum[IEEE80211_BAND_5G][2],
			MinusStepNum[IEEE80211_BAND_5G][3],
			MinusStepNum[IEEE80211_BAND_5G][4],
			MinusStepNum[IEEE80211_BAND_5G][5],
			MinusStepNum[IEEE80211_BAND_5G][6],
			MinusStepNum[IEEE80211_BAND_5G][7]
			));
		DBGPRINT(RT_DEBUG_TRACE, ("\ttssi gain/step = %u\n", pAd->TxPowerCtrl.TssiGain[IEEE80211_BAND_5G]));
		DBGPRINT(RT_DEBUG_TRACE, ("\tStep = %u\n", Step[IEEE80211_BAND_5G]));
		DBGPRINT(RT_DEBUG_TRACE, ("\tRefTemp= %d\n", pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G]));

		band_nums = IEEE80211_BAND_NUMS;
	}
#endif /* A_BAND_SUPPORT */

#ifdef RT8592
	if (IS_RT8592(pAd)) {
		pos_bound = 32;
		neg_bound = 12;
		Offset = 12;
	}
	else
#endif /* RT8592 */
	{
		pos_bound = 26;
		neg_bound = 8;
		Offset = 7;
	}

	for (band = IEEE80211_BAND_2G; band < band_nums; band++)
	{
		/*
			For positive table, in sequence 0, 1, 2 ...., 31, 32
				F(x) = F(x-1) + Step(x)
			with following exception:
				F(0) = F(1) - Compensation(x);
				F(1) = Step(band) / 2;

			After each step, Compensation value will decrease base on step value and 
				Compensation(x) = Step(band) - step_cnt;
		*/

		LUT_POSITIVE(band,1) = Step[band] / 2;
		LUT_BASE(band) = LUT_POSITIVE(band, 1) - Step[band];

		step_cnt = 0;
		IdxTmp = 1;
		for (pos_idx = 2; pos_idx <= pos_bound;)
			{
			comp_val = Step[band] - step_cnt;
			LUT_POSITIVE(band, pos_idx) = LUT_POSITIVE(band, pos_idx - 1) + comp_val /* (Step[band] - (step_cnt+1) + 1)*/;
			if (PlusStepNum[band][step_cnt] != 0 || step_cnt >= 8)
				{
				if (pos_idx >= IdxTmp + PlusStepNum[band][step_cnt] && step_cnt < 8)
				{
					IdxTmp = IdxTmp + PlusStepNum[band][step_cnt];
					step_cnt += 1;
				}
				pos_idx++;
			}
			else
			{
				step_cnt += 1;
			}
		}

		/* negative */
		step_cnt = 0;
		IdxTmp = 1;
		for (neg_idx = 1; neg_idx <= neg_bound;)
		{
			comp_val = Step[band] + step_cnt;
			LUT_NEGATIVE(band, neg_idx) = LUT_NEGATIVE(band, (neg_idx - 1)) - comp_val;
			if (MinusStepNum[band][step_cnt] != 0 || step_cnt >= 8)
			{
				if ((neg_idx + 1) >= IdxTmp + MinusStepNum[band][step_cnt] && step_cnt < 8)
				{
					IdxTmp = IdxTmp + MinusStepNum[band][step_cnt];
					step_cnt += 1;
				}
				neg_idx++;
			}
			else
			{
				step_cnt += 1;
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation %sG band(%d)] Lookup table:\n",
						(band == 0 ? "2.4" : "5"), band));
		for (lut_idx = 0; lut_idx < LOOKUP_TB_SIZE; lut_idx++)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("\t%d, %d\n", lut_idx - Offset, LUT_INDEX(band, lut_idx)));
		}
	}

	bbp_tx_comp_init(pAd, 1, 0);
}


VOID AsicGetAutoAgcOffsetForTemperatureSensor(
	IN RTMP_ADAPTER *pAd,
	IN CHAR *pDeltaPwr,
	IN CHAR *pTotalDeltaPwr,
	IN CHAR *pAgcCompensate,
	IN CHAR *pDeltaPowerByBbpR1)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable;
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry0 = NULL; /* Ant0 */
	TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTableEntry1 = NULL; /* Ant1 */
	BOOLEAN bAutoTxAgc = FALSE;
	PCHAR pTxAgcCompensate = NULL;
	UCHAR RFValue = 0;
	CHAR TuningTableUpperBound = 0, TuningTableIndex0 = 0, TuningTableIndex1 = 0;
	CHAR temper_val, rf_txpwr_bnd;
	INT CurrentTemp = 0, RefTemp, lut_base;
	INT *LookupTable, LookupTableIndex;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));
	
#ifdef RT8592
	if (IS_RT8592(pAd))
		lut_base = 12;
	else
#endif /* RT8592 */
		lut_base = TEMPERATURE_COMPENSATION_LOOKUP_TABLE_OFFSET;
	LookupTableIndex = pAd->TxPowerCtrl.LookupTableIndex + lut_base;

	*pTotalDeltaPwr = 0;

#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{
		/* a band channel */
		bAutoTxAgc = pAd->bAutoTxAgcA;
		pTxAgcCompensate = &pAd->TxAgcCompensateA;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_5G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_5G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_5G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_5G;
		rf_txpwr_bnd = 0x2b;
	}
	else
#endif /* A_BAND_SUPPORT */
	{
		/* bg band channel */
		bAutoTxAgc = pAd->bAutoTxAgcG;
		pTxAgcCompensate = &pAd->TxAgcCompensateG;
		TxPowerTuningTable = pChipCap->TxPowerTuningTable_2G;
		RefTemp = pAd->TxPowerCtrl.RefTemp[IEEE80211_BAND_2G];
		LookupTable = &pAd->TxPowerCtrl.LookupTable[IEEE80211_BAND_2G][0];
		TuningTableUpperBound = pChipCap->TxAlcTxPowerUpperBound_2G;
		rf_txpwr_bnd = 0x27;
	}

	/* AutoTxAgc in EEPROM means temperature compensation enabled/diablded. */
	if (bAutoTxAgc)
	{ 
		/* Current temperature */
		bbp_get_temp(pAd, &temper_val);
		CurrentTemp = temper_val;

		if (tx_temp_dbg) {
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] current temp = %d(0x%x), pAd->curr_temp=%d, temper_val=%d\n",
									CurrentTemp, CurrentTemp, pAd->curr_temp, temper_val));
		}
		CurrentTemp  = pAd->curr_temp;
		if (tx_temp_dbg) {
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] RefTemp = %d\n", RefTemp));
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] index = %d\n", pAd->TxPowerCtrl.LookupTableIndex));
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex - 1, LookupTable[LookupTableIndex - 1]));
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex, LookupTable[LookupTableIndex]));
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] f(%d)= %d\n", pAd->TxPowerCtrl.LookupTableIndex + 1, LookupTable[LookupTableIndex + 1]));
		}
		if (CurrentTemp > RefTemp + LookupTable[LookupTableIndex + 1] + ((LookupTable[LookupTableIndex + 1] - LookupTable[LookupTableIndex]) >> 2) &&
			LookupTableIndex < (LOOKUP_TB_SIZE - 1))
		{
			if (tx_temp_dbg) {
				DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ++\n"));
			}
			LookupTableIndex++;
			pAd->TxPowerCtrl.LookupTableIndex++;
		}
		else if (CurrentTemp < RefTemp + LookupTable[LookupTableIndex] - ((LookupTable[LookupTableIndex] - LookupTable[LookupTableIndex - 1]) >> 2) &&
			LookupTableIndex > 0)
		{
			if (tx_temp_dbg) {
				DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] --\n"));
			}
			LookupTableIndex--;
			pAd->TxPowerCtrl.LookupTableIndex--;
		}
		else
		{
			if (tx_temp_dbg) {
				DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] ==\n"));
			}
		}

		if (tx_temp_dbg) {
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] idxTxPowerTable=%d, idxTxPowerTable2=%d, TuningTableUpperBound=%d\n",
			pAd->TxPowerCtrl.idxTxPowerTable + pAd->TxPowerCtrl.LookupTableIndex,
			pAd->TxPowerCtrl.idxTxPowerTable2 + pAd->TxPowerCtrl.LookupTableIndex,
			TuningTableUpperBound));
		}

#ifdef E3_DBG_FALLBACK
		TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable 
									+ pAd->TxPowerCtrl.LookupTableIndex 
#ifdef DOT11_N_SUPPORT
									+ pAd->TxPower[pAd->CommonCfg.CentralChannel-1].Power;
#else
									+ pAd->TxPower[pAd->CommonCfg.Channel-1].Power;
#endif /* DOT11_N_SUPPORT */
#else
		TuningTableIndex0 = pAd->TxPowerCtrl.idxTxPowerTable 
									+ pAd->TxPowerCtrl.LookupTableIndex 
									+ pAd->hw_cfg.cur_ch_pwr[0];
#endif /* E3_DBG_FALLBACK */
		/* The boundary verification */ 
		TuningTableIndex0 = (TuningTableIndex0 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex0;
		TuningTableIndex0 = (TuningTableIndex0 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex0;
		TxPowerTuningTableEntry0 = &TxPowerTuningTable[TuningTableIndex0 + TX_POWER_TUNING_ENTRY_OFFSET];
		
#ifdef E3_DBG_FALLBACK
		TuningTableIndex1 = pAd->TxPowerCtrl.idxTxPowerTable2 
									+ pAd->TxPowerCtrl.LookupTableIndex 
#ifdef DOT11_N_SUPPORT				
									+ pAd->TxPower[pAd->CommonCfg.CentralChannel-1].Power2;
#else
									+ pAd->TxPower[pAd->CommonCfg.Channel-1].Power2;
#endif /* DOT11_N_SUPPORT */
#else
		TuningTableIndex1 = pAd->TxPowerCtrl.idxTxPowerTable2 
									+ pAd->TxPowerCtrl.LookupTableIndex
									+ pAd->hw_cfg.cur_ch_pwr[1];
#endif /* E3_DBG_FALLBACK */
		/* The boundary verification */
		TuningTableIndex1 = (TuningTableIndex1 > TuningTableUpperBound) ? TuningTableUpperBound : TuningTableIndex1;
		TuningTableIndex1 = (TuningTableIndex1 < LOWERBOUND_TX_POWER_TUNING_ENTRY) ? 
							LOWERBOUND_TX_POWER_TUNING_ENTRY : TuningTableIndex1;
		TxPowerTuningTableEntry1 = &TxPowerTuningTable[TuningTableIndex1 + TX_POWER_TUNING_ENTRY_OFFSET];

		/*
			Update RF_R49 [0:5], RF_R50[0:5]
				The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27
				The valid range of the RF R50 (<5:0>tx0_alc<5:0>) is 0x00~0x27
		*/
		RT30xxReadRFRegister(pAd, RF_R49, &RFValue);
		RFValue &= (~0x3F);
		// TODO: Shiang-6590, for a band, the upper bound of RF_R49/RF_R50 shall be 0x2b instead of 0x27!!!!	
		RFValue |= ((TxPowerTuningTableEntry0->RF_TX_ALC > rf_txpwr_bnd) ? rf_txpwr_bnd : TxPowerTuningTableEntry0->RF_TX_ALC);
		RT30xxWriteRFRegister(pAd, RF_R49, RFValue);
		if (tx_temp_dbg) {
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx0)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry0->RF_TX_ALC, TxPowerTuningTableEntry0->MAC_PowerDelta, TuningTableIndex0));
			DBGPRINT(RT_DEBUG_TRACE, ("\tUpdate RF_R49[0:5] to 0x%x\n", (RFValue & 0x3f)));
		}

		RT30xxReadRFRegister(pAd, RF_R50, &RFValue);
		RFValue &= (~0x3F);
		RFValue |= ((TxPowerTuningTableEntry1->RF_TX_ALC > rf_txpwr_bnd) ? rf_txpwr_bnd : TxPowerTuningTableEntry1->RF_TX_ALC);
		RT30xxWriteRFRegister(pAd, RF_R50, RFValue);
		if (tx_temp_dbg) {
			DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] (tx1)RF_TX_ALC = %x, MAC_PowerDelta = %d, TuningTableIndex = %d\n",
			TxPowerTuningTableEntry1->RF_TX_ALC, TxPowerTuningTableEntry1->MAC_PowerDelta, TuningTableIndex1));
			DBGPRINT(RT_DEBUG_TRACE, ("\tUpdate RF_R50[0:5] to 0x%x\n", (RFValue & 0x3f)));
		}
		
		*pTotalDeltaPwr = TxPowerTuningTableEntry0->MAC_PowerDelta;
	}

	*pAgcCompensate = *pTxAgcCompensate;

	if (tx_temp_dbg) {
		DBGPRINT(RT_DEBUG_TRACE, ("<--%s(): pDeltaPwr=%d, TotalDeltaPwr=%d, AgcCompensate=%d, DeltaPowerByBbpR1=%d\n",
				__FUNCTION__, *pDeltaPwr, *pTotalDeltaPwr, *pAgcCompensate, *pDeltaPowerByBbpR1));
}
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */


#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm						0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK		0x03


#ifdef RT6352
VOID InitRfPaModeTable(
	IN PRTMP_ADAPTER	pAd)
{
	UINT32 mac_value;
	UCHAR bit, pa_value;

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &mac_value);
	
	for (bit = 0; bit < 8; bit += 2) /* CCK */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_cck[bit/2] = pa_value;
	}

	for (bit = 8; bit < 24; bit += 2) /* OFDM */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_ofdm[(bit - 8)/2] = pa_value;
	}
		
	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &mac_value);
	
	for (bit = 0; bit < 32; bit += 2) /* HT */
	{
		pa_value = (UCHAR)((mac_value >> bit) & (0x03));
		pAd->rf_pa_mode_over_ht[bit/2] = pa_value;
	}
}
#endif /* RT6352 */


VOID AsicGetTxPowerOffset(RTMP_ADAPTER *pAd, ULONG *TxPwr)
{
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;
	DBGPRINT(RT_DEBUG_INFO, ("-->AsicGetTxPowerOffset\n"));

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	CfgOfTxPwrCtrlOverMAC.NumOfEntries = 5; /* MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324 */

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80 &&
		pAd->CommonCfg.Channel > 14)
	{
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx80MPwrCfgABand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx80MPwrCfgABand[1];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx80MPwrCfgABand[2];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx80MPwrCfgABand[3];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx80MPwrCfgABand[4];
	}
	else
#endif /* DOT11_VHT_AC */
	if (pAd->CommonCfg.BBPCurrentBW == BW_40)
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgABand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgABand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgABand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgABand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgABand[4];
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
		}
	}
	else
	{
		if (pAd->CommonCfg.CentralChannel > 14)
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgABand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgABand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgABand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgABand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgABand[4];
		}
		else
		{
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
			CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
		}
	}

	NdisCopyMemory(TxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	DBGPRINT(RT_DEBUG_INFO, ("<--AsicGetTxPowerOffset\n"));
}


VOID AsicGetAutoAgcOffsetForExternalTxAlc(
	IN PRTMP_ADAPTER 			pAd,
	IN PCHAR 					pDeltaPwr,
	IN PCHAR 					pTotalDeltaPwr,
	IN PCHAR 					pAgcCompensate,
	IN PCHAR 					pDeltaPowerByBbpR1)
{
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	UCHAR			TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep, idx;
	PCHAR			pTxAgcCompensate = NULL;
	CHAR    			DeltaPwr = 0;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));

	BbpR49.byte = 0;

	/* TX power compensation for temperature variation based on TSSI. Try every 4 second */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0)
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			/* bg channel */
			bAutoTxAgc = pAd->bAutoTxAgcG;
			TssiRef = pAd->TssiRefG;
			pTssiMinusBoundary = (UCHAR *)&pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary = (UCHAR *)&pAd->TssiPlusBoundaryG[0];
			TxAgcStep = pAd->TxAgcStepG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		}
		else
		{
			/* a channel */
			bAutoTxAgc = pAd->bAutoTxAgcA;
			TssiRef = pAd->TssiRefA;
			pTssiMinusBoundary = (UCHAR *)&pAd->TssiMinusBoundaryA[0];
			pTssiPlusBoundary = (UCHAR *)&pAd->TssiPlusBoundaryA[0];
			TxAgcStep = pAd->TxAgcStepA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
		{
			bbp_get_temp(pAd, &BbpR49.byte);

			/* TSSI representation */
			if (IS_RT3071(pAd) || IS_RT3390(pAd) || IS_RT3090A(pAd) || IS_RT3572(pAd)) /* 5-bits */
			{
				BbpR49.byte = (BbpR49.byte & 0x1F);
			}
				
			/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
			/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
			/* step value is defined in pAd->TxAgcStepG for tx power value */

			/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
			/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
			    above value are examined in mass factory production */
			/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

			/* plus (+) is 0x00 ~ 0x45, minus (-) is 0xa0 ~ 0xf0 */
			/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
			/* if value is 0xa5, tx power will be -= TxAgcStep*(2-1) */

			if (BbpR49.byte > pTssiMinusBoundary[1])
			{
				/* Reading is larger than the reference value */
				/* Check for how large we need to decrease the Tx power */
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte <= pTssiMinusBoundary[idx])  /* Found the range */
						break;
				}
				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */

				*pTxAgcCompensate = -(TxAgcStep * (idx-1));			
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					                BbpR49.byte, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49.byte < pTssiPlusBoundary[1])
			{
				/* Reading is smaller than the reference value */
				/* Check for how large we need to increase the Tx power */
				for (idx = 1; idx < 5; idx++)
				{
					if (BbpR49.byte >= pTssiPlusBoundary[idx])   /* Found the range*/
						break;
				}

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                	BbpR49.byte, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
				                	BbpR49.byte, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pAd->CommonCfg.Channel <= 14)
		{
			bAutoTxAgc = pAd->bAutoTxAgcG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc = pAd->bAutoTxAgcA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}


	*pDeltaPwr = DeltaPwr;
	*pAgcCompensate = *pTxAgcCompensate;

	DBGPRINT(RT_DEBUG_INFO, ("<--%s\n", __FUNCTION__));
}


VOID AsicExtraPowerOverMAC(RTMP_ADAPTER *pAd)
{
	UINT32 txpwr = 0;
	UINT32 txpwr7 = 0, txpwr8 = 0, txpwr9 = 0;

	/* For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 */
	RTMP_IO_READ32(pAd, 0x1318, &txpwr);
	txpwr7 |= (txpwr & 0x0000FF00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, 0x131C, &txpwr);
	txpwr7 |= (txpwr & 0x0000FF00) << 8; /* Get Tx power for HT MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, txpwr7);

	/* For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1324, &txpwr);
	txpwr9 |= (txpwr & 0x0000FF00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, txpwr9);

	if (pAd->CommonCfg.TxStream == 2)
	{
		/* For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
		RTMP_IO_READ32(pAd, 0x1320, &txpwr);
		txpwr8 |= (txpwr & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
		RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, txpwr8);
	}

}


/*
	==========================================================================
	Description:
		Gives CCK TX rate 2 more dB TX power.
		This routine works only in LINK UP in INFRASTRUCTURE mode.

		calculate desired Tx power in RF R3.Tx0~5,	should consider -
		0. if current radio is a noisy environment (pAd->DrsCounters.fNoisyEnvironment)
		1. TxPowerPercentage
		2. auto calibration based on TSSI feedback
		3. extra 2 db for CCK
		4. -10 db upon very-short distance (AvgRSSI >= -40db) to AP

	NOTE: Since this routine requires the value of (pAd->DrsCounters.fNoisyEnvironment),
		it should be called AFTER MlmeDynamicTxRatSwitching()
	==========================================================================
 */
VOID AsicAdjustTxPower(RTMP_ADAPTER *pAd) 
{
	INT i, j;
	CHAR Value;
	CHAR Rssi = -127;
	CHAR DeltaPwr = 0;
	CHAR TxAgcCompensate = 0;
	CHAR DeltaPowerByBbpR1 = 0; 
	CHAR TotalDeltaPower = 0; /* (non-positive number) including the transmit power controlled by the MAC and the BBP R1 */
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};	
#ifdef SINGLE_SKU
	CHAR TotalDeltaPowerOri = 0;
	UCHAR SingleSKUBbpR1Offset = 0;
	ULONG SingleSKUTotalDeltaPwr[MAX_TXPOWER_ARRAY_SIZE] = {0};
#endif /* SINGLE_SKU */


#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF))
		return;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) || 
#ifdef RTMP_MAC_PCI
		(pAd->bPCIclkOff == TRUE) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF) ||
#endif /* RTMP_MAC_PCI */
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		return;

	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if(INFRA_ON(pAd))
		{
			Rssi = RTMPMaxRssi(pAd, 
						   pAd->StaCfg.RssiSample.AvgRssi0, 
						   pAd->StaCfg.RssiSample.AvgRssi1, 
						   pAd->StaCfg.RssiSample.AvgRssi2);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Get Tx rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
	/* Get temperature compensation delta power value */
	RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(
		pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &DeltaPowerByBbpR1);

	DBGPRINT(RT_DEBUG_INFO, ("%s(): DeltaPwr=%d, TotalDeltaPower=%d, TxAgcCompensate=%d, DeltaPowerByBbpR1=%d\n",
			__FUNCTION__,
			DeltaPwr,
			TotalDeltaPower,
			TxAgcCompensate,
			DeltaPowerByBbpR1));

#ifdef RT8592
	// TODO: shiang-6590, fix me for move this ugly function to other place!!!
	if (IS_RT8592(pAd) && (pAd->chipCap.rx_temp_comp == TRUE)) {
		rx_temp_compensation(pAd);
	}
#endif /* RT8592 */


	/* Get delta power based on the percentage specified from UI */
	AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,&DeltaPowerByBbpR1);

	/* The transmit power controlled by the BBP */
	TotalDeltaPower += DeltaPowerByBbpR1; 
	/* The transmit power controlled by the MAC */
	TotalDeltaPower += DeltaPwr; 	

#ifdef SINGLE_SKU
	if (pAd->CommonCfg.bSKUMode == TRUE)
	{
		/* Re calculate delta power while enabling Single SKU */
		GetSingleSkuDeltaPower(pAd, &TotalDeltaPower, (PULONG)&SingleSKUTotalDeltaPwr, &SingleSKUBbpR1Offset);
	
		TotalDeltaPowerOri = TotalDeltaPower;
	}
	else
#endif /* SINGLE_SKU */
	{
		AsicCompensatePowerViaBBP(pAd, &TotalDeltaPower);
	}			

#if defined(MT7601) || defined(MT76x2)
	if (IS_MT7601(pAd) || IS_MT76x2(pAd))
		return;
#endif /* MT7601 */


}


#ifdef SINGLE_SKU
VOID GetSingleSkuDeltaPower(
	IN RTMP_ADAPTER *pAd,
	IN CHAR *pTotalDeltaPower,
	INOUT ULONG *pSingleSKUTotalDeltaPwr,
	INOUT UCHAR *pSingleSKUBbpR1Offset) 
{
	INT		i, j;
	CHAR	Value;
	CHAR 	MinValue = 127;
	UCHAR	BbpR1 = 0;
	UCHAR  	TxPwrInEEPROM = 0xFF, CountryTxPwr = 0xFF, criterion;
	UCHAR   	AdjustMaxTxPwr[(MAX_TX_PWR_CONTROL_OVER_MAC_REGISTERS * 8)]; 
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC = {0};
	
	/* Get TX rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
		
	/* Handle regulatory max. TX power constraint */
	if (pAd->CommonCfg.Channel > 14) 
	{
		TxPwrInEEPROM = ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF00) >> 8); /* 5G band */
	}
	else 
	{
		TxPwrInEEPROM = (pAd->CommonCfg.DefineMaxTxPwr & 0x00FF); /* 2.4G band */
	}

	CountryTxPwr = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel); 

	/* Use OFDM 6M as the criterion */
	criterion = (UCHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue & 0x000F0000) >> 16);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: criterion=%d, TxPwrInEEPROM=%d, CountryTxPwr=%d\n", 
		__FUNCTION__, criterion, TxPwrInEEPROM, CountryTxPwr));

	/* Adjust max. TX power according to the relationship of TX power in EEPROM */
	for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (i == 0)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); 

				if (j < 4)
				{
					AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion) + 4; /* CCK has 4dBm larger than OFDM */
				}
				else
				{
					AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);
				}

				DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n", 
					__FUNCTION__,
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset,
					i, 
					j, 
					Value, 
					AdjustMaxTxPwr[i*8+j]));
			}
		}
		else
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

				AdjustMaxTxPwr[i*8+j] = TxPwrInEEPROM + (Value - criterion);

				DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Default)Value=%d, %d\n", 
					__FUNCTION__,
					CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
					i, 
					j, 
					Value, 
					AdjustMaxTxPwr[i*8+j]));
			}
		}
	}

	/* Adjust TX power according to the relationship */
	for (i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F);

				/* The TX power is larger than the regulatory, the power should be restrained */
				if (AdjustMaxTxPwr[i*8+j] > CountryTxPwr)
				{
					Value = (AdjustMaxTxPwr[i*8+j] - CountryTxPwr);
					
					if (Value > 0xF)
					{
						/* The output power is larger than Country Regulatory over 15dBm, the origianl design has overflow case */
						DBGPRINT(RT_DEBUG_ERROR,("%s: Value overflow - %d\n", __FUNCTION__, Value));
					}
					
					*(pSingleSKUTotalDeltaPwr+i) = (*(pSingleSKUTotalDeltaPwr+i) & ~(0x0000000F << j*4)) | (Value << j*4);

					DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, (Exceed)Value=%d, %d\n", 
						__FUNCTION__,
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j]));
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%s: offset = 0x%04X, i/j=%d/%d, Value=%d, %d, no change\n",
						__FUNCTION__,
						CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].MACRegisterOffset, 
						i, 
						j, 
						Value, 
						AdjustMaxTxPwr[i*8+j]));
				}
			}
		}
	}

	/* Calculate the min. TX power */
	for(i=0; i<CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++)
	{
		if (CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				CHAR PwrChange;
				/* 
				   After Single SKU, each data rate offset power value is saved in TotalDeltaPwr[].
				   PwrChange will add SingleSKUDeltaPwr and TotalDeltaPwr[] for each data rate to calculate
				   the final adjust output power value which is saved in MAC Reg. and BBP_R1.
				*/
				
				/*   
				   Value / TxPwr[] is get from eeprom 0xDEh ~ 0xEFh and increase or decrease the  
				   20/40 Bandwidth Delta Value in eeprom 0x50h. 
				*/
				Value = (CHAR)((CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i].RegisterValue >> j*4) & 0x0F); /* 0 ~ 15 */

				/* Fix the corner case of Single SKU read eeprom offset 0xF0h ~ 0xFEh which for BBP Instruction configuration */
				if (Value == 0xF)
					continue;

				/* Value_offset is current Pwr comapre with Country Regulation and need adjust delta value */
				PwrChange = (CHAR)((*(pSingleSKUTotalDeltaPwr+i) >> j*4) & 0x0F); /* 0 ~ 15 */
				PwrChange -= *pTotalDeltaPower;

				Value -= PwrChange;
				
				if (MinValue > Value)
					MinValue = Value;				
			}
		}
	}

	// TODO: shiang, handle this for RLT_BBP!!
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	/* Depend on the min. TX power to adjust and prevent the value of MAC_TX_PWR_CFG less than 0 */
	if ((MinValue < 0) && (MinValue >= -6))
	{
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;
		*pSingleSKUBbpR1Offset = 6;
	}
	else if ((MinValue < -6)&&(MinValue >= -12))
	{
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	}
	else if (MinValue < -12)
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s: ASIC limit..\n", __FUNCTION__));
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;
		*pSingleSKUBbpR1Offset = 12;
	}
#ifndef E3_DBG_FALLBACK 
	else {
                BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;
                *pSingleSKUBbpR1Offset = 0;
	}
#endif /* E3_DBG_FALLBACK */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: <After BBP R1> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X \n", __FUNCTION__, *pTotalDeltaPower, BbpR1));
}
#endif /* SINGLE_SKU */


VOID AsicPercentageDeltaPower(
	IN 		PRTMP_ADAPTER 		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1) 
{
	/* 
		Calculate delta power based on the percentage specified from UI.
		E2PROM setting is calibrated for maximum TX power (i.e. 100%).
		We lower TX power here according to the percentage specified from UI.
	*/
	
	if (pAd->CommonCfg.TxPowerPercentage >= 100) /* AUTO TX POWER control */
	{
#ifdef CONFIG_STA_SUPPORT
		if ((pAd->OpMode == OPMODE_STA)
		)
		{
			/* To patch high power issue with some APs, like Belkin N1.*/
			if (Rssi > -35)
			{
				*pDeltaPwr -= 12;
			}
			else if (Rssi > -40)
			{
				*pDeltaPwr -= 6;
			}
			else
				;
		}
#endif /* CONFIG_STA_SUPPORT */
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 90) /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		;
	else if (pAd->CommonCfg.TxPowerPercentage > 60) /* 61 ~ 90%, treat as 75% in terms of mW		 DeltaPwr -= 1; */
	{
		*pDeltaPwr -= 1;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 30) /* 31 ~ 60%, treat as 50% in terms of mW		 DeltaPwr -= 3; */
	{
		*pDeltaPwr -= 3;
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 15) /* 16 ~ 30%, treat as 25% in terms of mW		 DeltaPwr -= 6; */
	{
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
	}
	else if (pAd->CommonCfg.TxPowerPercentage > 9) /* 10 ~ 15%, treat as 12.5% in terms of mW		 DeltaPwr -= 9; */
	{
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
		*pDeltaPwr -= 3;
	}
	else /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW		 DeltaPwr -= 12; */
	{
		*pDeltaPowerByBbpR1 -= 12; /* -12 dBm */
	}
}


VOID AsicCompensatePowerViaBBP(RTMP_ADAPTER *pAd, CHAR *pTotalDeltaPower)
{
	UCHAR mdsm_drop_pwr;

#if defined(MT7601) || defined(MT76x2)
	if (IS_MT7601(pAd) || IS_MT76x2(pAd))
	{
		return;
	}
#endif /* MT7601 */

	DBGPRINT(RT_DEBUG_INFO, ("%s: <Before> TotalDeltaPower = %d dBm\n", __FUNCTION__, *pTotalDeltaPower));

	if (*pTotalDeltaPower <= -12)
	{
		*pTotalDeltaPower += 12;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_12dBm;
	}
	else if ((*pTotalDeltaPower <= -6) && (*pTotalDeltaPower > -12))
	{
		*pTotalDeltaPower += 6;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_6dBm;
	}
	else
	{
		/* Control the the transmit power by using the MAC only */
		mdsm_drop_pwr = MDSM_NORMAL_TX_POWER;
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: Drop the BBP transmit power by %d dBm!\n",
				__FUNCTION__, 
				(mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_12dBm ? 12 : \
				(mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_6dBm ? 6 : 0))));

#ifdef RT65xx
	if (IS_RT65XX(pAd))
	{
		UINT32 bbp_val = 0;

		RTMP_BBP_IO_READ32(pAd, TXBE_R4, &bbp_val);
		bbp_val &= (~0x3);
		bbp_val |= mdsm_drop_pwr;
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R4, bbp_val);
		DBGPRINT(RT_DEBUG_INFO, ("%s: <After> TotalDeltaPower = %d dBm, TXBE_R4 = 0x%0x\n", __FUNCTION__, *pTotalDeltaPower, bbp_val));
	}
#endif /* RT65xx */

#ifdef RTMP_BBP
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		UCHAR	BbpR1 = 0;
	
		/* The BBP R1 controls the transmit power for all rates */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
		BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;	
		BbpR1 |= mdsm_drop_pwr;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
		
		DBGPRINT(RT_DEBUG_INFO, ("%s: <After> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X \n", __FUNCTION__, *pTotalDeltaPower, BbpR1));
	}
#endif /* RTMP_BBP */
}


/*
	========================================================================
	
	Routine Description:
		Read initial Tx power per MCS and BW from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID RTMPReadTxPwrPerRate(RTMP_ADAPTER *pAd)
{
	ULONG		data, Adata, Gdata;
	USHORT		i, value, value2;
	USHORT		value_1, value_2, value_3, value_4;
	INT			Apwrdelta, Gpwrdelta;
	UCHAR		t1,t2,t3,t4;
	BOOLEAN		bApwrdeltaMinus = TRUE, bGpwrdeltaMinus = TRUE;




#ifdef RT8592
	if (IS_RT8592(pAd)) {
		RT85592ReadTxPwrPerRate(pAd);
		return;
	}
#endif /* RT8592 */


#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		mt76x2_get_tx_pwr_per_rate(pAd);	
		return;
	}
#endif


#ifdef RT6352
	if (IS_RT6352(pAd)) {
		RT6352_RTMPReadTxPwrPerRate(pAd);
		return;
	}
#endif /* RT6352 */


	/* For default one, go here!! */
	{	
		
		/* Get power delta for 20MHz and 40MHz.*/
		DBGPRINT(RT_DEBUG_TRACE, ("Txpower per Rate\n"));
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value2);
		Apwrdelta = 0;
		Gpwrdelta = 0;

		if ((value2 & 0xff) != 0xff)
		{
			if ((value2 & 0x80))
				Gpwrdelta = (value2&0xf);
			
			if ((value2 & 0x40))
				bGpwrdeltaMinus = FALSE;
			else
				bGpwrdeltaMinus = TRUE;
		}
		if ((value2 & 0xff00) != 0xff00)
		{
			if ((value2 & 0x8000))
				Apwrdelta = ((value2&0xf00)>>8);

			if ((value2 & 0x4000))
				bApwrdeltaMinus = FALSE;
			else
				bApwrdeltaMinus = TRUE;
		}	
		DBGPRINT(RT_DEBUG_TRACE, ("Gpwrdelta = %x, Apwrdelta = %x .\n", Gpwrdelta, Apwrdelta));

		
		/* Get Txpower per MCS for 20MHz in 2.4G.*/
		
		for (i=0; i<5; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4, value);
			data = value;

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata = t1 + (t2<<4) + (t3<<8) + (t4<<12);
			
			RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + i*4 + 2, value);

			/* use value_1 ~ value_4 for code size reduce */
			value_1 = value&0xf;
			value_2 = (value&0xf0)>>4;
			value_3 = (value&0xf00)>>8;
			value_4 = (value&0xf000)>>12;

			if (bApwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Apwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Apwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Apwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Apwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Apwrdelta)
					t1 = value_1-(Apwrdelta);
				else
					t1 = 0;
				if (value_2 > Apwrdelta)
					t2 = value_2-(Apwrdelta);
				else
					t2 = 0;
				if (value_3 > Apwrdelta)
					t3 = value_3-(Apwrdelta);
				else
					t3 = 0;
				if (value_4 > Apwrdelta)
					t4 = value_4-(Apwrdelta);
				else
					t4 = 0;
			}				
			Adata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			if (bGpwrdeltaMinus == FALSE)
			{
				t1 = value_1+(Gpwrdelta);
				if (t1 > 0xf)
					t1 = 0xf;
				t2 = value_2+(Gpwrdelta);
				if (t2 > 0xf)
					t2 = 0xf;
				t3 = value_3+(Gpwrdelta);
				if (t3 > 0xf)
					t3 = 0xf;
				t4 = value_4+(Gpwrdelta);
				if (t4 > 0xf)
					t4 = 0xf;
			}
			else
			{
				if (value_1 > Gpwrdelta)
					t1 = value_1-(Gpwrdelta);
				else
					t1 = 0;
				if (value_2 > Gpwrdelta)
					t2 = value_2-(Gpwrdelta);
				else
					t2 = 0;
				if (value_3 > Gpwrdelta)
					t3 = value_3-(Gpwrdelta);
				else
					t3 = 0;
				if (value_4 > Gpwrdelta)
					t4 = value_4-(Gpwrdelta);
				else
					t4 = 0;
			}				
			Gdata |= ((t1<<16) + (t2<<20) + (t3<<24) + (t4<<28));
			data |= (value<<16);

			/* For 20M/40M Power Delta issue */		
			pAd->Tx20MPwrCfgABand[i] = data;
			pAd->Tx20MPwrCfgGBand[i] = data;
			pAd->Tx40MPwrCfgABand[i] = Adata;
			pAd->Tx40MPwrCfgGBand[i] = Gdata;
			
			if (data != 0xffffffff)
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + i*4, data);
			DBGPRINT_RAW(RT_DEBUG_TRACE, ("20MHz BW, 2.4G band-%lx,  Adata = %lx,  Gdata = %lx \n", data, Adata, Gdata));
		}
	}
}


/*
	========================================================================
	
	Routine Description:
		Read initial channel power parameters from EEPROM
		
	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		
	========================================================================
*/
VOID RTMPReadChannelPwr(RTMP_ADAPTER *pAd)
{
	UINT32					i, choffset;
	EEPROM_TX_PWR_STRUC	    Power;
	EEPROM_TX_PWR_STRUC	    Power2;
#if (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593)
	UCHAR Tx0ALC = 0, Tx1ALC = 0, Tx0FinePowerCtrl = 0, Tx1FinePowerCtrl = 0;
#endif /* (defined(RT30xx) && defined(RTMP_MAC_PCI)) || defined(RT3593) */

	/* Read Tx power value for all channels*/
	/* Value from 1 - 0x7f. Default value is 24.*/
	/* Power value : 2.4G 0x00 (0) ~ 0x1F (31)*/
	/*             : 5.5G 0xF9 (-7) ~ 0x0F (15)*/

	/* 0. 11b/g, ch1 - ch 14*/
	for (i = 0; i < 7; i++)
	{
#if defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) || defined(RT5592) || defined(RT3290) || defined(RT65xx)
		if (IS_RT5390(pAd) || IS_RT5392(pAd) || IS_RT5592(pAd) || IS_RT3290(pAd) || IS_RT65XX(pAd))
		{
			 RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2,Power.word);
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2,Power2.word);
			}
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;
	
			if ((Power.field.Byte0 > 0x27) || (Power.field.Byte0 < 0))
			{
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2].Power = Power.field.Byte0;
			}
	
			if ((Power.field.Byte1 > 0x27) || (Power.field.Byte1 < 0))
			{
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}
			else
			{
				pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			}
	
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				if ((Power2.field.Byte0 > 0x27) || (Power2.field.Byte0 < 0))
				{
					pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;
				}
		
				if ((Power2.field.Byte1 > 0x27) || (Power2.field.Byte1 < 0))
				{
					pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
				}
				else
				{
					pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
				}
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPower[%d].Power = 0x%02X, TxPower[%d].Power = 0x%02X\n", 
				__FUNCTION__, 
				i * 2, 
				pAd->TxPower[i * 2].Power, 
				i * 2 + 1, 
				pAd->TxPower[i * 2 + 1].Power));
			
			if (IS_RT5392(pAd) || IS_RT5592(pAd))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("%s: TxPower[%d].Power2 = 0x%02X, TxPower[%d].Power2 = 0x%02X\n", 
					__FUNCTION__, 
					i * 2, 
					pAd->TxPower[i * 2].Power2, 
					i * 2 + 1, 
					pAd->TxPower[i * 2 + 1].Power2));
			}
		}
		else
#endif /* defined(RT5370) || defined(RT5372) || defined(RT5390) || defined(RT5392) || defined(RT5592) || defined(RT65xx) */
		{ /* Default routine. RT3070 and RT3370 run here. */
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
			pAd->TxPower[i * 2].Channel = i * 2 + 1;
			pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;

			pAd->TxPower[i * 2].Power = Power.field.Byte0;
			if(!IS_RT3390(pAd))  // 3370 has different Tx power range
			{
			if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;
			}				

			pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;
			if(!IS_RT3390(pAd)) // 3370 has different Tx power range
			{
			if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;
			}				

			if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
				pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
				pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
			else
				pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
		}
	}
	

	{
		if (IS_RT5592(pAd))
			return;
		
		/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
		/* 1.1 Fill up channel*/
		choffset = 14;
		for (i = 0; i < 4; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 36 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 36 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 36 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}

		/* 1.2 Fill up power*/
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}
		
		/* 2. HipperLAN 2 100, 102 ,104; 108, 110, 112; 116, 118, 120; 124, 126, 128; 132, 134, 136; 140 (including central frequency in BW 40MHz)*/
		/* 2.1 Fill up channel*/
		choffset = 14 + 12;
		for (i = 0; i < 5; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 100 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 100 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 100 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}
		pAd->TxPower[3 * 5 + choffset + 0].Channel		= 140;
		pAd->TxPower[3 * 5 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 5 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

		/* 2.2 Fill up power*/
		for (i = 0; i < 8; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}

		/* 3. U-NII upper band: 149, 151, 153; 157, 159, 161; 165, 167, 169; 171, 173 (including central frequency in BW 40MHz)*/
		/* 3.1 Fill up channel*/
		choffset = 14 + 12 + 16;
		/*for (i = 0; i < 2; i++)*/
		for (i = 0; i < 3; i++)
		{
			pAd->TxPower[3 * i + choffset + 0].Channel	= 149 + i * 8 + 0;
			pAd->TxPower[3 * i + choffset + 0].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 0].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 1].Channel	= 149 + i * 8 + 2;
			pAd->TxPower[3 * i + choffset + 1].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 1].Power2	= DEFAULT_RF_TX_POWER;

			pAd->TxPower[3 * i + choffset + 2].Channel	= 149 + i * 8 + 4;
			pAd->TxPower[3 * i + choffset + 2].Power	= DEFAULT_RF_TX_POWER;
			pAd->TxPower[3 * i + choffset + 2].Power2	= DEFAULT_RF_TX_POWER;
		}
		pAd->TxPower[3 * 3 + choffset + 0].Channel		= 171;
		pAd->TxPower[3 * 3 + choffset + 0].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 0].Power2		= DEFAULT_RF_TX_POWER;

		pAd->TxPower[3 * 3 + choffset + 1].Channel		= 173;
		pAd->TxPower[3 * 3 + choffset + 1].Power		= DEFAULT_RF_TX_POWER;
		pAd->TxPower[3 * 3 + choffset + 1].Power2		= DEFAULT_RF_TX_POWER;

		/* 3.2 Fill up power*/
		/*for (i = 0; i < 4; i++)*/
		for (i = 0; i < 6; i++)
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX_PWR_OFFSET + (choffset - 14) + i * 2, Power.word);
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TX2_PWR_OFFSET + (choffset - 14) + i * 2, Power2.word);

			if ((Power.field.Byte0 < 16) && (Power.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power = Power.field.Byte0;

			if ((Power.field.Byte1 < 16) && (Power.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power = Power.field.Byte1;			

			if ((Power2.field.Byte0 < 16) && (Power2.field.Byte0 >= -7))
				pAd->TxPower[i * 2 + choffset + 0].Power2 = Power2.field.Byte0;

			if ((Power2.field.Byte1 < 16) && (Power2.field.Byte1 >= -7))
				pAd->TxPower[i * 2 + choffset + 1].Power2 = Power2.field.Byte1;			
		}
	}


	/* 4. Print and Debug*/
	/*choffset = 14 + 12 + 16 + 7;*/
	choffset = 14 + 12 + 16 + 11;
	

}

#ifdef RTMP_TEMPERATURE_COMPENSATION
/* 
	Note: This function use E2P_OFFSET_START and E2P_OFFSET_END to
	determine the table type is minus or plus.
*/
BOOLEAN LoadTempCompTableFromEEPROM(
		IN	RTMP_ADAPTER	*pAd,
		IN	const USHORT		E2P_OFFSET_START,
		IN	const USHORT		E2P_OFFSET_END,
		OUT	PUCHAR			TssiTable,
		IN	const INT			StartIndex,
		IN	const UINT32		TABLE_SIZE)
{
	USHORT	e2p_value;
	INT	e2p_index, table_index;
	CHAR	table_sign; /* +1 for plus table, -1 for minus table */

	table_sign = (E2P_OFFSET_START < E2P_OFFSET_END) ? 1 : (-1);

	if (StartIndex < TABLE_SIZE)
		table_index = StartIndex;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, 
					("%s(): Error! Wrong starting index (%d).\n",
					__FUNCTION__, StartIndex));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE,
				("%s(): Load %s %s table from E2P 0x%x to 0x%x. StartIndex = %d\n",
				__FUNCTION__, 
				pAd->CommonCfg.Channel > 14 ? "5G" : "2.4G",
				(table_sign == 1) ? "plus" : "minus",
				E2P_OFFSET_START, E2P_OFFSET_END, StartIndex));
	// TODO: Load two bytes
	for (e2p_index = E2P_OFFSET_START;
		 e2p_index != (E2P_OFFSET_END + (1*table_sign));
		 e2p_index = e2p_index + (1*table_sign))
	{
		RT28xx_EEPROM_READ16(pAd, e2p_index, e2p_value);
		TssiTable[table_index] = (UCHAR)(e2p_value & 0xFF);
		if ((++table_index) >= TABLE_SIZE)
			break; /* table full */
	}

	if (e2p_index != E2P_OFFSET_END)
	{
		/* Table is full before e2p_offset_end */
		DBGPRINT(RT_DEBUG_WARN, 
					("%s(): Warning! EEPROM table may not be completely loaded.\n",
					__FUNCTION__));
		return FALSE;
	}
	else
	{	
		for (table_index = 0; table_index < TABLE_SIZE; table_index++)
			DBGPRINT(RT_DEBUG_TRACE, ("\tTable[%d] = %3d (0x%02X)\n", 
						table_index, (CHAR)TssiTable[table_index], TssiTable[table_index]));
		return TRUE;
	}

	return TRUE;
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */

