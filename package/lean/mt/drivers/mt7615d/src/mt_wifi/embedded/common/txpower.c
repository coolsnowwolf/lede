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
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"

INT tx_temp_dbg;
#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm						0x01
#define MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK		0x03


VOID AsicGetAutoAgcOffsetForExternalTxAlc(
	IN PRTMP_ADAPTER			pAd,
	IN PCHAR					pDeltaPwr,
	IN PCHAR					pTotalDeltaPwr,
	IN PCHAR					pAgcCompensate,
	IN PCHAR					pDeltaPowerByBbpR1)
{
	BBP_R49_STRUC	BbpR49;
	BOOLEAN			bAutoTxAgc = FALSE;
	UCHAR			TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep, idx;
	PCHAR			pTxAgcCompensate = NULL;
	CHAR			DeltaPwr = 0;
	UCHAR Channel = HcGetRadioChannel(pAd);
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s\n", __func__));
	BbpR49.byte = 0;

	/* TX power compensation for temperature variation based on TSSI. Try every 4 second */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0) {
		if (Channel <= 14) {
			/* bg channel */
			bAutoTxAgc = pAd->bAutoTxAgcG;
			TssiRef = pAd->TssiRefG;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
			pTssiPlusBoundary = &pAd->TssiPlusBoundaryG[0];
			TxAgcStep = pAd->TxAgcStepG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		} else {
			/* a channel */
			bAutoTxAgc = pAd->bAutoTxAgcA;
			TssiRef = pAd->TssiRefA;
			pTssiMinusBoundary = &pAd->TssiMinusBoundaryA[0][0];
			pTssiPlusBoundary = &pAd->TssiPlusBoundaryA[0][0];
			TxAgcStep = pAd->TxAgcStepA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc) {
			bbp_get_temp(pAd, &BbpR49.byte);

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

			if (BbpR49.byte > pTssiMinusBoundary[1]) {
				/* Reading is larger than the reference value */
				/* Check for how large we need to decrease the Tx power */
				for (idx = 1; idx < 5; idx++) {
					if (BbpR49.byte <= pTssiMinusBoundary[idx])  /* Found the range */
						break;
				}

				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = -(TxAgcStep * (idx - 1));
				DeltaPwr += (*pTxAgcCompensate);
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
						  BbpR49.byte, TssiRef, TxAgcStep, idx - 1));
			} else if (BbpR49.byte < pTssiPlusBoundary[1]) {
				/* Reading is smaller than the reference value */
				/* Check for how large we need to increase the Tx power */
				for (idx = 1; idx < 5; idx++) {
					if (BbpR49.byte >= pTssiPlusBoundary[idx])   /* Found the range*/
						break;
				}

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = TxAgcStep * (idx - 1);
				DeltaPwr += (*pTxAgcCompensate);
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						  BbpR49.byte, TssiRef, TxAgcStep, idx - 1));
			} else {
				*pTxAgcCompensate = 0;
				MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("   Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						  BbpR49.byte, TssiRef, TxAgcStep, 0));
			}
		}
	} else {
		if (Channel <= 14) {
			bAutoTxAgc = pAd->bAutoTxAgcG;
			pTxAgcCompensate = &pAd->TxAgcCompensateG;
		} else {
			bAutoTxAgc = pAd->bAutoTxAgcA;
			pTxAgcCompensate = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	*pDeltaPwr = DeltaPwr;
	*pAgcCompensate = *pTxAgcCompensate;
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s\n", __func__));
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
	UCHAR Channel = HcGetRadioChannel(pAd);
	/* Get Tx rate offset table which from EEPROM 0xDEh ~ 0xEFh */
	RTMP_CHIP_ASIC_TX_POWER_OFFSET_GET(pAd, (PULONG)&CfgOfTxPwrCtrlOverMAC);
	/* Get temperature compensation delta power value */
	RTMP_CHIP_ASIC_AUTO_AGC_OFFSET_GET(
		pAd, &DeltaPwr, &TotalDeltaPower, &TxAgcCompensate, &DeltaPowerByBbpR1, Channel);
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): DeltaPwr=%d, TotalDeltaPower=%d, TxAgcCompensate=%d, DeltaPowerByBbpR1=%d\n",
			  __func__,
			  DeltaPwr,
			  TotalDeltaPower,
			  TxAgcCompensate,
			  DeltaPowerByBbpR1));
	/* Get delta power based on the percentage specified from UI */
	AsicPercentageDeltaPower(pAd, Rssi, &DeltaPwr,  &DeltaPowerByBbpR1);
	/* The transmit power controlled by the BBP */
	TotalDeltaPower += DeltaPowerByBbpR1;
	/* The transmit power controlled by the MAC */
	TotalDeltaPower += DeltaPwr;
	{
		AsicCompensatePowerViaBBP(pAd, &TotalDeltaPower);
	}

	/* Power will be updated each 4 sec. */
	if (pAd->Mlme.OneSecPeriodicRound % 4 == 0) {
		/* Set new Tx power for different Tx rates */
		for (i = 0; i < CfgOfTxPwrCtrlOverMAC.NumOfEntries; i++) {
			TX_POWER_CONTROL_OVER_MAC_ENTRY *pTxPwrEntry;
			ULONG reg_val;
			pTxPwrEntry = &CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[i];
			reg_val = pTxPwrEntry->RegisterValue;

			if (reg_val != 0xffffffff) {
				for (j = 0; j < 8; j++) {
					CHAR _upbound, _lowbound, t_pwr;
					BOOLEAN _bValid;
					_lowbound = 0;
					_bValid = TRUE;
					Value = (CHAR)((reg_val >> j * 4) & 0x0F);
					_upbound = 0xc;

					if (_bValid) {
						t_pwr = Value + TotalDeltaPower;

						if (t_pwr < _lowbound)
							Value = _lowbound;
						else if (t_pwr > _upbound)
							Value = _upbound;
						else
							Value = t_pwr;
					}

					/* Fill new value into the corresponding MAC offset */
#ifdef E3_DBG_FALLBACK
					pTxPwrEntry->RegisterValue = (reg_val & ~(0x0000000F << j * 4)) | (Value << j * 4);
#else
					reg_val = (reg_val & ~(0x0000000F << j * 4)) | (Value << j * 4);
#endif /* E3_DBG_FALLBACK */
				}

#ifndef E3_DBG_FALLBACK
				pTxPwrEntry->RegisterValue = reg_val;
#endif /* E3_DBG_FALLBACK */
				RTMP_IO_WRITE32(pAd, pTxPwrEntry->MACRegisterOffset, pTxPwrEntry->RegisterValue);
			}
		}

		/* Extra set MAC registers to compensate Tx power if any */
		RTMP_CHIP_ASIC_EXTRA_POWER_OVER_MAC(pAd);
	}

}




VOID AsicPercentageDeltaPower(
	IN		PRTMP_ADAPTER		pAd,
	IN		CHAR				Rssi,
	INOUT	PCHAR				pDeltaPwr,
	INOUT	PCHAR				pDeltaPowerByBbpR1)
{
	/*
		Calculate delta power based on the percentage specified from UI.
		E2PROM setting is calibrated for maximum TX power (i.e. 100%).
		We lower TX power here according to the percentage specified from UI.
	*/
	if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] >= 100) { /* AUTO TX POWER control */
	} else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 90) /* 91 ~ 100% & AUTO, treat as 100% in terms of mW */
		*pDeltaPwr -= 0;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 60) /* 61 ~ 90%, treat as 75% in terms of mW		 DeltaPwr -= 1; */
		*pDeltaPwr -= 1;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 30) /* 31 ~ 60%, treat as 50% in terms of mW		 DeltaPwr -= 3; */
		*pDeltaPwr -= 3;
	else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] > 15) { /* 16 ~ 30%, treat as 25% in terms of mW		 DeltaPwr -= 6; */
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
	} else if (pAd->CommonCfg.ucTxPowerPercentage[BAND0] >
			   9) { /* 10 ~ 15%, treat as 12.5% in terms of mW		 DeltaPwr -= 9; */
		*pDeltaPowerByBbpR1 -= 6; /* -6 dBm */
		*pDeltaPwr -= 3;
	} else { /* 0 ~ 9 %, treat as MIN(~3%) in terms of mW		 DeltaPwr -= 12; */
		*pDeltaPowerByBbpR1 -= 12; /* -12 dBm */
	}
}

VOID AsicCompensatePowerViaBBP(RTMP_ADAPTER *pAd, CHAR *pTotalDeltaPower)
{
	UCHAR mdsm_drop_pwr;
	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: <Before> TotalDeltaPower = %d dBm\n", __func__,
			 *pTotalDeltaPower));

	if (*pTotalDeltaPower <= -12) {
		*pTotalDeltaPower += 12;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_12dBm;
	} else if ((*pTotalDeltaPower <= -6) && (*pTotalDeltaPower > -12)) {
		*pTotalDeltaPower += 6;
		mdsm_drop_pwr = MDSM_DROP_TX_POWER_BY_6dBm;
	} else {
		/* Control the the transmit power by using the MAC only */
		mdsm_drop_pwr = MDSM_NORMAL_TX_POWER;
	}

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: Drop the BBP transmit power by %d dBm!\n",
			 __func__,
			 (mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_12dBm ? 12 : \
			  (mdsm_drop_pwr == MDSM_DROP_TX_POWER_BY_6dBm ? 6 : 0))));
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
	/* Don't Support this now! */
	return;
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
	for (i = 0; i < 7; i++) {
		/* Default routine. RT3070 and RT3370 run here. */
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX_PWR_OFFSET + i * 2, Power.word);
		RT28xx_EEPROM_READ16(pAd, EEPROM_G_TX2_PWR_OFFSET + i * 2, Power2.word);
		pAd->TxPower[i * 2].Channel = i * 2 + 1;
		pAd->TxPower[i * 2 + 1].Channel = i * 2 + 2;
		pAd->TxPower[i * 2].Power = Power.field.Byte0;

		if ((Power.field.Byte0 > 31) || (Power.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power = DEFAULT_RF_TX_POWER;

		pAd->TxPower[i * 2 + 1].Power = Power.field.Byte1;

		if ((Power.field.Byte1 > 31) || (Power.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power = DEFAULT_RF_TX_POWER;

		if ((Power2.field.Byte0 > 31) || (Power2.field.Byte0 < 0))
			pAd->TxPower[i * 2].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2].Power2 = Power2.field.Byte0;

		if ((Power2.field.Byte1 > 31) || (Power2.field.Byte1 < 0))
			pAd->TxPower[i * 2 + 1].Power2 = DEFAULT_RF_TX_POWER;
		else
			pAd->TxPower[i * 2 + 1].Power2 = Power2.field.Byte1;
	}

	{
		/* 1. U-NII lower/middle band: 36, 38, 40; 44, 46, 48; 52, 54, 56; 60, 62, 64 (including central frequency in BW 40MHz)*/
		/* 1.1 Fill up channel*/
		choffset = 14;

		for (i = 0; i < 4; i++) {
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
		for (i = 0; i < 6; i++) {
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

		for (i = 0; i < 5; i++) {
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
		for (i = 0; i < 8; i++) {
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
		for (i = 0; i < 3; i++) {
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
		for (i = 0; i < 6; i++) {
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


VOID LoadTssiInfoFromEEPROM(RTMP_ADAPTER *pAd)
{
	EEPROM_TX_PWR_STRUC Power;
	Power.word = 0;
	/* Read TSSI reference and TSSI boundary for temperature compensation. This is ugly */
	/* 0. 11b/g*/
	/* these are tempature reference value (0x00 ~ 0xFE)
	   ex: 0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
	   TssiPlusBoundaryG [4] [3] [2] [1] [0] (smaller) +
	   TssiMinusBoundaryG[0] [1] [2] [3] [4] (larger) */
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND1, Power.word);
	pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
	pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND2, Power.word);
	pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
	pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND3, Power.word);
	pAd->TssiRefG   = Power.field.Byte0; /* reference value [0] */
	pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND4, Power.word);
	pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
	pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND5, Power.word);
	pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;
	pAd->TxAgcStepG = Power.field.Byte1;
	pAd->TxAgcCompensateG = 0;
	pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
	pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

	/* Disable TxAgc if the based value is not right*/
	if (pAd->TssiRefG == 0xff)
		pAd->bAutoTxAgcG = FALSE;

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("E2PROM: G Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			  pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
			  pAd->TssiRefG,
			  pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
			  pAd->TxAgcStepG, pAd->bAutoTxAgcG));
	/* 1. 11a*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND1, Power.word);
	pAd->TssiMinusBoundaryA[0][4] = Power.field.Byte0;
	pAd->TssiMinusBoundaryA[0][3] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND2, Power.word);
	pAd->TssiMinusBoundaryA[0][2] = Power.field.Byte0;
	pAd->TssiMinusBoundaryA[0][1] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND3, Power.word);
	pAd->TssiRefA = Power.field.Byte0;
	pAd->TssiPlusBoundaryA[0][1] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND4, Power.word);
	pAd->TssiPlusBoundaryA[0][2] = Power.field.Byte0;
	pAd->TssiPlusBoundaryA[0][3] = Power.field.Byte1;
	RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND5, Power.word);
	pAd->TssiPlusBoundaryA[0][4] = Power.field.Byte0;
	pAd->TxAgcStepA = Power.field.Byte1;
	pAd->TxAgcCompensateA = 0;
	pAd->TssiMinusBoundaryA[0][0] = pAd->TssiRefA;
	pAd->TssiPlusBoundaryA[0][0]  = pAd->TssiRefA;

	/* Disable TxAgc if the based value is not right*/
	if (pAd->TssiRefA == 0xff)
		pAd->bAutoTxAgcA = FALSE;

	MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("E2PROM: A Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			  pAd->TssiMinusBoundaryA[0][4], pAd->TssiMinusBoundaryA[0][3], pAd->TssiMinusBoundaryA[0][2],
			  pAd->TssiMinusBoundaryA[0][1],
			  pAd->TssiRefA,
			  pAd->TssiPlusBoundaryA[0][1], pAd->TssiPlusBoundaryA[0][2], pAd->TssiPlusBoundaryA[0][3], pAd->TssiPlusBoundaryA[0][4],
			  pAd->TxAgcStepA, pAd->bAutoTxAgcA));
}


INT tx_pwr_comp_init(RTMP_ADAPTER *pAd)
{
	return FALSE;

}

INT32 get_low_mid_hi_index(UINT8 channel)
{
	INT32 index = G_BAND_LOW;

	if (channel <= 14) {
		if (channel >= 1 && channel <= 5)
			index = G_BAND_LOW;
		else if (channel >= 6 && channel <= 10)
			index = G_BAND_MID;
		else if (channel >= 11 && channel <= 14)
			index = G_BAND_HI;
		else
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal channel(%d)\n", channel));
	} else {
		if (channel >= 184 && channel <= 188)
			index = A_BAND_LOW;
		else if (channel >= 192 && channel <= 196)
			index = A_BAND_HI;
		else if (channel >= 36 && channel <= 42)
			index = A_BAND_LOW;
		else if (channel >= 44 && channel <= 48)
			index = A_BAND_HI;
		else if (channel >= 52 && channel <= 56)
			index = A_BAND_LOW;
		else if (channel >= 58 && channel <= 64)
			index = A_BAND_HI;
		else if (channel >= 98 && channel <= 104)
			index = A_BAND_LOW;
		else if (channel >= 106 && channel <= 114)
			index = A_BAND_HI;
		else if (channel >= 116 && channel <= 128)
			index = A_BAND_LOW;
		else if (channel >= 130 && channel <= 144)
			index = A_BAND_HI;
		else if (channel >= 149 && channel <= 156)
			index = A_BAND_LOW;
		else if (channel >= 157 && channel <= 165)
			index = A_BAND_HI;
		else
			MTWF_LOG(DBG_CAT_POWER, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal channel(%d)\n", channel));
	}

	return index;
}

