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
	cmm_rf_cal.c

	Abstract:
	RF calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Arvin Tai     2012/05/02
*/

#include "rt_config.h"

REG_PAIR RT6352_VGA_TABLE[] = {
	/* Gain(dB), BBP_R66 */
	{0, 0x10},
	{2, 0x14},
	{4, 0x18},
	{6, 0x1C},
	{8, 0x20},
	{10, 0x30},
	{12, 0x34},
	{14, 0x38},
	{16, 0x3C},
	{18, 0x40},
	{20, 0x44},
	{22, 0x60},
	{24, 0x64},
	{26, 0x68},
	{28, 0x6C},
	{30, 0x70},
	{32, 0x74},
	{34, 0x78},
	{36, 0x7C},
};

REG_PAIR RT6352_RFR3R4_TABLE[] = {
	/* Gain(dB), RF_R3R4 */
	{ 0, 0x4 },
	{ 2, 0x5 },
	{ 4, 0x6 },
	{ 6, 0x7 },
	{ 8, 0x8 },
	{10, 0xc },
	{12, 0xd },
	{14, 0xe },
	{16, 0xf },
	{18, 0x10},
	{20, 0x11},
	{22, 0x18},
	{24, 0x19},
	{26, 0x1a},
	{28, 0x1b},
	{30, 0x1c},
	{32, 0x1d},
	{34, 0x1e},
	{36, 0x1f},
};


UCHAR RT6352_VGA_TABLE_PARMS = (sizeof(RT6352_VGA_TABLE) / sizeof(REG_PAIR));

INT32 CalcRCalibrationCode(
	IN PRTMP_ADAPTER pAd,
	IN INT32 D1,
	IN INT32 D2)
{
	INT32 CalCode;

	CalCode = ((D2 - D1) * 1000) / 43;

	if ((CalCode % 10) >= 5)
		CalCode += 10;

	CalCode = (CalCode / 10);
	return CalCode;
}

/****************************************************************************
 *
 *   VOID R Calibraton
 *
 ****************************************************************************/

VOID R_Calibration(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 saveMacSysCtrl;
	UCHAR  saveRfB0R1, saveRfB0R34, saveRfB0R35;
	UCHAR  saveRfB5R4, saveRfB5R17, saveRfB5R18;
	UCHAR  saveRfB5R19, saveRfB5R20;
	UCHAR  saveBBPR22, saveBBPR47, saveBBPR49;
	UCHAR byteValue = 0;
	INT32 RCalCode;
	UCHAR R_Cal_Code = 0;
	CHAR D1 = 0, D2 = 0;
	UCHAR RFValue;
	UINT32 MAC_RF_BYPASS0, MAC_RF_CONTROL0, MAC_PWR_PIN_CFG;
	/* Save RF Register */
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &saveRfB0R1);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R34, &saveRfB0R34);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R35, &saveRfB0R35);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &saveRfB5R4);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &saveRfB5R17);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &saveRfB5R18);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &saveRfB5R19);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &saveRfB5R20);
	/* save BBP registers */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R22, &saveBBPR22);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &saveBBPR47);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &saveBBPR49);
	/* Save MAC registers */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &MAC_RF_BYPASS0);
	RTMP_IO_READ32(pAd, RF_CONTROL0, &MAC_RF_CONTROL0);
	RTMP_IO_READ32(pAd, PWR_PIN_CFG, &MAC_PWR_PIN_CFG);
	{
		UINT32 macCfg, macStatus;
		UINT32 MTxCycle, MRxCycle;
		ULONG stTime, mt_time, mr_time;
		/* Disable MAC Tx and MAC Rx and wait MAC Tx/Rx status in idle state */
		/* MAC Tx */
		NdisGetSystemUpTime(&stTime);
		AsicSetMacTxRx(pAd, ASIC_MAC_TX, FALSE);

		for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++) {
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

			if (macStatus & 0x1)
				RtmpusecDelay(50);
			else
				break;
		}

		NdisGetSystemUpTime(&mt_time);
		mt_time -= stTime;

		if (MTxCycle == 10000) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(cnt=%d,time=0x%lx):stop MTx,macStatus=0x%x!\n",
					 __func__, MTxCycle, mt_time, macStatus));
		}

		/* MAC Rx */
		NdisGetSystemUpTime(&stTime);
		AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);

		for (MRxCycle = 0; MRxCycle < 10000; MRxCycle++) {
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

			if (macStatus & 0x2)
				RtmpusecDelay(50);
			else
				break;
		}

		NdisGetSystemUpTime(&mr_time);
		mr_time -= stTime;

		if (MRxCycle == 10000) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s(cnt=%d,time=0x%lx):stop MRx, macStatus=%d!\n",
					 __func__, MRxCycle, mr_time, macStatus));
		}
	}
	/* RF bypass MAC */
	RFValue = (MAC_RF_BYPASS0 | 0x3004);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, RFValue);
	RFValue = (MAC_RF_CONTROL0 & (~0x3002));
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, RFValue);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x27);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0x83);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0x00);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x20);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, 0x00);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R34, 0x13);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x00);
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x1);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, 0x04);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x80);
	RtmpusecDelay(100);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &byteValue);

	if (byteValue > 128)
		D1 = byteValue - 256;
	else
		D1 = (CHAR)byteValue;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x0);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x01);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x80);
	RtmpusecDelay(100);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &byteValue);

	if (byteValue > 128)
		D2 = byteValue - 256;
	else
		D2 = (CHAR)byteValue;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x0);
	RCalCode = CalcRCalibrationCode(pAd, D1, D2);

	if (RCalCode < 0)
		R_Cal_Code = 256 + RCalCode;
	else
		R_Cal_Code = (UCHAR)RCalCode;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("D1 = %d, D2 = %d, CalCode = %d !!!\n", D1, D2, RCalCode));
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R07, R_Cal_Code);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, 0x00);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &byteValue);
	byteValue |= 0x1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, byteValue);
	RtmpOsMsDelay(1);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &byteValue);
	byteValue &= (~0x1);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, byteValue);
	/* Restore RF Register */
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, saveRfB0R1);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R34, saveRfB0R34);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, saveRfB0R35);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, saveRfB5R4);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, saveRfB5R17);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, saveRfB5R18);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, saveRfB5R19);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, saveRfB5R20);
	/* Restore BBP registers */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R22, saveBBPR22);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, saveBBPR47);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, saveBBPR49);
	/* Restore registers */
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MAC_RF_BYPASS0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MAC_RF_CONTROL0);
	/*	Return to normal mode */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, MAC_PWR_PIN_CFG);
}

INT Set_TestRxIQCalibration_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TestRxIQCalibration !!!\n"));
	RXIQ_Calibration(pAd);
	return TRUE;
}

INT Set_TestLoftTxIQCalibration_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	UINT32 Value;

	Value = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TestLOFTTxIQCalibration !!!\n"));
	pAd->bCalibrationDone = FALSE;
	/* LOFT_IQ_Calibration_v2(pAd, TRUE, Value); */
	LOFT_IQ_Calibration(pAd);
	pAd->bCalibrationDone = TRUE;
	return TRUE;
}

VOID RtmpKickOutHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bPrepareContent,
	IN BOOLEAN bTransmit)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	TXWI_STRUC NullTxWI, *pTxWI = NULL;
	PUCHAR pNullFrame;
	NDIS_STATUS NState;
	PHEADER_802_11 pNullFr;
	ULONG Length;
	UCHAR *ptr;
	UINT i;
	UINT32 longValue, macStatus;
	USHORT k_count = 0;
	HTTRANSMIT_SETTING MlmeTransmit;
	MAC_TX_INFO mac_info;
	struct wifi_dev *wdev;

	if (bPrepareContent) {
		NState = MlmeAllocateMemory(pAd, (PUCHAR *)&pNullFrame);
		NdisZeroMemory(pNullFrame, 48);

		if (NState == NDIS_STATUS_SUCCESS) {
			pTxWI = &NullTxWI;
			NdisZeroMemory(pTxWI, TXWISize);
			pNullFr = (PHEADER_802_11) pNullFrame;
			Length = sizeof(HEADER_802_11);
			pNullFr->FC.Type = FC_TYPE_DATA;
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;
			pNullFr->FC.ToDs = 0;
			pNullFr->FC.FrDs = 1;
			COPY_MAC_ADDR(pNullFr->Addr1, BROADCAST_ADDR);
			COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pNullFr->Addr3, pAd->CurrentAddress);
			wdev = wdev_search_by_address(pAd, pNullFr->Addr2);

			if (wdev == NULL) {
				MlmeFreeMemory(pNullFrame);
				return;
			}

			pNullFr->FC.PwrMgmt = 0;
			pNullFr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, pAd->CommonCfg.TxRate, 14);
			/* sequence is increased in MlmeHardTx */
			pNullFr->Sequence = pAd->Sequence;
			pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence  */
			MlmeTransmit.word = 0;
			MlmeTransmit.field.MCS = 15;
			MlmeTransmit.field.MODE = MODE_HTMIX;
			MlmeTransmit.field.BW = 0;
			NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
			mac_info.FRAG = FALSE;
			mac_info.CFACK = FALSE;
			mac_info.InsTimestamp = FALSE;
			mac_info.AMPDU = FALSE;
			mac_info.BM = IS_BM_MAC_ADDR(pNullFr->Addr1);
			mac_info.Ack = FALSE;
			mac_info.NSeq = TRUE;
			mac_info.BASize = 0;
			mac_info.WCID = 1;
			mac_info.Length = Length;
			mac_info.PID = 15;
			mac_info.TID = 0;
			mac_info.TxRate = 15;
			mac_info.Txopmode = IFS_HTTXOP;
			mac_info.Preamble = LONG_PREAMBLE;
			mac_info.IsAutoRate = FALSE;
			mac_info.SpeEn = 1;
#ifdef MT_MAC
			mac_info.OmacIdx = wdev->OmacIdx;
#endif
			pAd->archOps.write_tmac_info_fixed_rate(pAd, (UCHAR *)pTxWI, &mac_info, &MlmeTransmit);
			pTxWI->TXWI_O.MCS = 15;
			pTxWI->TXWI_O.PHYMODE = MODE_HTMIX;
			pTxWI->TXWI_O.BW = 0;
			ptr = (PUCHAR)&NullTxWI;
#ifdef RT_BIG_ENDIAN
			RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif

			for (i = 0; i < TXWISize; i += 4) {
				longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
				RTMP_IO_WRITE32(pAd, HW_CS_CTS_BASE + i, longValue);
				ptr += 4;
			}

			ptr = pNullFrame;
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif

			for (i = 0; i < Length; i += 4) {
				longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
				RTMP_IO_WRITE32(pAd, HW_CS_CTS_BASE + TXWISize + i, longValue);
				ptr += 4;
			}
		}

		if (pNullFrame)
			MlmeFreeMemory(pNullFrame);
	}

	if (bTransmit) {
		/* kick NULL frame #0 */
		RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x80);

		/* Check MAC Tx/Rx idle */
		for (k_count = 0; k_count < 200; k_count++) {
			RTMP_IO_READ32(pAd, PBF_CTRL, &macStatus);

			if (macStatus & 0x80)
				RtmpusecDelay(100);
			else
				break;
		}

		if (k_count == 200)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Wait Null Frame SendOut to MAX  !!!\n"));
	}

	return;
}

/****************************************************************************
 *
 *   VOID DPD Calibraton
 *
 ****************************************************************************/

/* (Workaround for IQ swap introduced AM-PM inversion)
	Multiply the AM-PM in LUTs by -1
*/
VOID DPD_IQ_Swap_AM_PM_Inversion(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx)
{
	USHORT k_count = 0, PM_inv_upper = 0, PM_inv_lower = 0;
	SHORT PM = 0, PM_inv = 0;
	UCHAR PM_upper = 0, PM_lower = 0;

	if (AntIdx == 0)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x02);
	else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x04);

	/* Read out AM-PM one by one, multiply by (-1) and write it back */
	for (k_count = 0; k_count < 64; k_count++) {
		/* Read out the AM-PM value through BBP registers */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 1));
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &PM_upper);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 2));
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &PM_lower);
		PM = (PM_upper * 256) + PM_lower;

		/* unsigned to signed conversion */
		if (PM > 512)
			PM -= 1024;

		/* Invert the AM-PM value */
		PM_inv = PM * (-1);

		/* Signed to unsigned conversion */
		if (PM_inv < 0)
			PM_inv += 1024;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("PM (%d), PM_inv (%d)\n", PM, PM_inv));
		/* Write back the AM-PM value through BBP registers */
		PM_inv_upper = (PM_inv / 256);
		PM_inv_lower = (PM_inv % 256);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 1));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, PM_inv_upper);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 2));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, PM_inv_lower);
	}

	return;
}

VOID DPD_AM_AM_LUT_Scaling(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx)
{
	USHORT k_count = 0;
	UCHAR AM_temp = 0, AM_scaling = 0;
	SHORT AM_Dealta = 0;
	UINT32 check_loop = 0;
	UCHAR scaling_vector[] = {0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  0, 0, 0, 0, 0, 0, 0, 0,
							  5, 5, 5, 5, 5, 5, 5, 5,
							  10, 10, 10, 10, 10, 10, 10, 10,
							  10, 10, 10, 10, 10, 10, 10, 10,
							  10, 10, 10, 10, 10, 10, 10, 10,
							  10, 10, 10, 10, 10, 10, 10, 10
							 };

	for (check_loop = 0; check_loop < 2; check_loop++) {
		if (AntIdx == 0) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("DPD Scaling for TX0 !!!\n"));

			if (check_loop == 0)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x02);
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x03);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("DPD Scaling for TX1 !!!\n"));

			if (check_loop == 0)
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x04);
			else
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x05);
		}

		/* Read out AM-PM one by one, multiply by (-1) and write it back */
		for (k_count = 0; k_count < 64; k_count++) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 3));
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &AM_temp);
			AM_Dealta = AM_temp - 128;

			if (scaling_vector[k_count] > 0) {
				if (AM_Dealta > 0)
					AM_scaling = 128 + ((AM_Dealta * scaling_vector[k_count]) / 10);
				else
					AM_scaling = 128;
			} else
				AM_scaling = 128;

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, AM_scaling);
			RtmpusecDelay(100);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(%d). AM_temp (%d), AM_Dealta (%d), AM_scaling (%d)\n", k_count, AM_temp, AM_Dealta, AM_scaling));
		}
	}
}

UCHAR DPD_Calibration(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR AntIdx)
{
	UCHAR index, Max_Retry = 0, Pass_Thres = 0, byteValue = 0;
	UINT32 macStatus, macValue;
	UINT32 saveMacSysCtrl, saveTxPinCfg, saveTxAlgCfg0;
	USHORT AM_SUM = 0, AM_10 = 0, k_count = 0;
	BOOLEAN DPD_Cal_success = FALSE, bNeedDoDPD = TRUE;
	UCHAR saveBbpR27, saveBbpR65;
	UCHAR saveBbpR241, saveBbpR242, saveBbpR244;
	UCHAR saveRfB0R1, saveRfB5R1, saveRfB7R1;
	UCHAR saveRfB4R11, saveRfB4R13;
	UCHAR saveRfB6R11, saveRfB6R13;
	UCHAR saveRfB4R19, saveRfB4R21, saveRfB4R22;
	UCHAR saveRfB5R17, saveRfB5R18, saveRfB5R19, saveRfB5R20;
	UCHAR saveRfB6R19, saveRfB6R21, saveRfB6R22;
	UCHAR saveRfB7R17, saveRfB7R18, saveRfB7R19, saveRfB7R20;
	UCHAR saveRfB5R3, saveRfB5R4;
	UCHAR saveRfB7R3, saveRfB7R4;
	UCHAR VGA_Upper_Bound, VGA_Lower_Bound, AM_63 = 0, VGA_code = 0;
	UCHAR saveBbpR159;/* , BBPValue; */
	CHAR VGA_code_idx = 0, target_power, delta_power;
	INT32 txALC_init = 0, txALC_limit = 0;
	INT32 temp_comp, tx_alc_txwi = 0;
	INT32 txvga2, txvga3, mac_gain_atten;
	INT32 gain_atten_bb = 0, gain_atten_rf = 0, txALC_req = 0;
	INT32 txALC_req_at, txALC_req_skip2, txALC_req_skip3;
	INT32 txALC_req_sat = 0;
	UINT32 macValue_Tx_Cfg0 = 0, macValue_2nd, check_loop = 0;
	BOOLEAN bInternalTxALC = FALSE;
	struct wifi_dev *wdev = get_default_wdev(pAd);
	/* init base power by e2p target power */
	target_power = ((pAd->E2p_D0_Value & 0xFF) == 0xFF) ? 0x1E : (pAd->E2p_D0_Value & 0x3F);

	switch (AntIdx) {
	case 0:
		/* Compute the corresponding signal values
		    Do exactly what TXALC does:
		*/
		RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &macValue);
		txALC_init = (macValue & 0x0000003F);
		txALC_limit = (macValue & 0x003F0000) >> 16;
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
		temp_comp = (macValue & 0x0000003F);

		if ((temp_comp & 0x20) == 0x20)
			temp_comp -= 64;

		tx_alc_txwi = 0;
		RTMP_IO_READ32(pAd, TX_ALC_VGA3, &macValue);
		txvga2 = (macValue & 0x001F0000) >> 16;
		txvga3 = (macValue & 0x0000001F);
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
		mac_gain_atten = (macValue & 0x00300000) >> 20;

		if (mac_gain_atten == 1) {
			RTMP_IO_READ32(pAd, TX0_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x00001F00) >> 8;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX0_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x00007F00) >> 8;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		} else if (mac_gain_atten == 2) {
			RTMP_IO_READ32(pAd, TX0_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x001F0000) >> 16;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX0_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x007F0000) >> 16;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		} else if (mac_gain_atten == 3) {
			RTMP_IO_READ32(pAd, TX0_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x1F000000) >> 24;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX0_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x7F000000) >> 24;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		}

		txALC_req = txALC_init + temp_comp + tx_alc_txwi;

		if (txALC_req >= txALC_limit)
			txALC_req = txALC_limit;

		if (txALC_req < 0)
			txALC_req_at = txALC_req - gain_atten_bb -  gain_atten_rf;
		else
			txALC_req_at = txALC_req;

		if (txALC_req_at > 23)
			txALC_req_skip2 = txALC_req_at + txvga2;
		else
			txALC_req_skip2 = txALC_req_at;

		if (txALC_req_skip2 > 35)
			txALC_req_skip3 = txALC_req_skip2 + txvga3;
		else
			txALC_req_skip3 = txALC_req_skip2;

		if (txALC_req_skip3 < 0)
			txALC_req_sat = 0;
		else if (txALC_req_skip3 > 47)
			txALC_req_sat = 47;
		else
			txALC_req_sat = txALC_req_skip3;

		/* no dpd when power is less 15 dBm or greater 20 dBm */
		bInternalTxALC = FALSE;
		{
#ifdef CONFIG_ATE

			if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
			{
				RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
				delta_power = (macValue & 0x3F);
				delta_power = (delta_power & 0x20) ? delta_power - 0x40 : delta_power;
				target_power += delta_power;
#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT

				if ((target_power < 12) || (target_power > 30))
#else
				if ((target_power < 30) || (target_power > 40))
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */
				{
					bNeedDoDPD = FALSE;
					break;
				}
			}
		}
		break;

	case 1:
		/* Compute the corresponding signal values
		    Do exactly what TXALC does:
		*/
		RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &macValue);
		txALC_init = (macValue & 0x00003F00) >> 8;
		txALC_limit = (macValue & 0x3F000000) >> 24;
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
		temp_comp = (macValue & 0x0000003F);

		if ((temp_comp & 0x20) == 0x20)
			temp_comp -= 64;

		tx_alc_txwi = 0;
		RTMP_IO_READ32(pAd, TX_ALC_VGA3, &macValue);
		txvga2 = (macValue & 0x1F000000) >> 24;
		txvga3 = (macValue & 0x00001F00) >> 8;
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
		mac_gain_atten = (macValue & 0x00C00000) >> 22;

		if (mac_gain_atten == 1) {
			RTMP_IO_READ32(pAd, TX1_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x00001F00) >> 8;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX1_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x00007F00) >> 8;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		} else if (mac_gain_atten == 2) {
			RTMP_IO_READ32(pAd, TX1_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x001F0000) >> 16;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX1_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x007F0000) >> 16;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		} else if (mac_gain_atten == 3) {
			RTMP_IO_READ32(pAd, TX1_BB_GAIN_ATTEN, &macValue);
			gain_atten_bb = (macValue & 0x1F000000) >> 24;

			if ((gain_atten_bb & 0x10) == 0x10)
				gain_atten_bb -= 32;

			RTMP_IO_READ32(pAd, TX1_RF_GAIN_ATTEN, &macValue);
			gain_atten_rf = (macValue & 0x7F000000) >> 24;

			if ((gain_atten_rf & 0x40) == 0x40)
				gain_atten_rf -= 128;
		}

		txALC_req = txALC_init + temp_comp + tx_alc_txwi;

		if (txALC_req >= txALC_limit)
			txALC_req = txALC_limit;

		if (txALC_req < 0)
			txALC_req_at = txALC_req - gain_atten_bb -  gain_atten_rf;
		else
			txALC_req_at = txALC_req;

		if (txALC_req_at > 23)
			txALC_req_skip2 = txALC_req_at + txvga2;
		else
			txALC_req_skip2 = txALC_req_at;

		if (txALC_req_skip2 > 35)
			txALC_req_skip3 = txALC_req_skip2 + txvga3;
		else
			txALC_req_skip3 = txALC_req_skip2;

		if (txALC_req_skip3 < 0)
			txALC_req_sat = 0;
		else if (txALC_req_skip3 > 47)
			txALC_req_sat = 47;
		else
			txALC_req_sat = txALC_req_skip3;

		/* no dpd when power is less 15 dBm or greater 20 dBm */
		bInternalTxALC = FALSE;
		{
#ifdef CONFIG_ATE

			if (!ATE_ON(pAd))
#endif /* CONFIG_ATE */
			{
				/* for single sku */
				RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &macValue);
				delta_power = (macValue & 0x3F);
				delta_power = (delta_power & 0x20) ? delta_power - 0x40 : delta_power;
				target_power += delta_power;
#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT

				if ((target_power < 12) || (target_power > 30))
#else
				if ((target_power < 30) || (target_power > 40))
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */
				{
					bNeedDoDPD = FALSE;
					break;
				}
			}
		}
		break;

	default:
		break;
	}

	if (bNeedDoDPD == FALSE) {
		DPD_Cal_success = FALSE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Don't need do DPD Calibration  !!!\n"));
		return 0;
	}


	if (AntIdx == 0) {
		if ((pAd->Tx0_DPD_ALC_tag0 == 0) && (pAd->Tx0_DPD_ALC_tag1 == 0))
			pAd->Tx0_DPD_ALC_tag1 = pAd->Tx0_DPD_ALC_tag1;
		else if ((txALC_req_sat <= (pAd->Tx0_DPD_ALC_tag1 | 0x3)) && (txALC_req_sat >= (pAd->Tx0_DPD_ALC_tag0 & (~0x3)))) {
			DPD_Cal_success = FALSE;
			return 1;
		}
	} else {
		if ((pAd->Tx1_DPD_ALC_tag0 == 0) && (pAd->Tx1_DPD_ALC_tag1 == 0))
			pAd->Tx1_DPD_ALC_tag1 = pAd->Tx1_DPD_ALC_tag1;
		else if ((txALC_req_sat <= (pAd->Tx1_DPD_ALC_tag1 | 0x3)) && (txALC_req_sat >= (pAd->Tx1_DPD_ALC_tag0 & (~0x3)))) {
			DPD_Cal_success = FALSE;
			return 1;
		}
	}

	/* Save MAC SYS CTRL registers */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	/* Save TX PIN CFG registers */
	RTMP_IO_READ32(pAd, TX_PIN_CFG, &saveTxPinCfg);
	/* Save TX_ALG_CFG_0 registers */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &saveTxAlgCfg0);
	/* Save BBP registers */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &saveBbpR27);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R65, &saveBbpR65);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBP_R140);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &saveBbpR159);

	if (pAd->CommonCfg.Chip_VerID > 1) {
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R241, &saveBbpR241);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R242, &saveBbpR242);
	} else
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R242, &saveBbpR242);

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R244, &saveBbpR244);
	/* Save RF registers */
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &saveRfB0R1);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R11, &saveRfB4R11);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R13, &saveRfB4R13);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R19, &saveRfB4R19);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R21, &saveRfB4R21);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R22, &saveRfB4R22);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &saveRfB5R1);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &saveRfB5R3);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &saveRfB5R4);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &saveRfB5R17);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &saveRfB5R18);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &saveRfB5R19);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &saveRfB5R20);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R11, &saveRfB6R11);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R13, &saveRfB6R13);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R19, &saveRfB6R19);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R21, &saveRfB6R21);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R22, &saveRfB6R22);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R01, &saveRfB7R1);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &saveRfB7R3);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R04, &saveRfB7R4);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R17, &saveRfB7R17);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R18, &saveRfB7R18);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R19, &saveRfB7R19);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R20, &saveRfB7R20);

	if (AntIdx == 0) {
		/* Setup the MAC to Transmit-Idle Mode through MAC registers */
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x001C0020);
		/* Connect RF loopback through MAC registers  */
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, 0x41);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R11, 0x51);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R13, 0x3C);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x80);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0xF1);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0xA1);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x01);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, 0xA0);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R21, 0x12);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R22, 0xA1);
	} else {
		/* Setup the MAC to Transmit-Idle Mode through MAC registers */
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x001C0080);
		/* Connect RF loopback through MAC registers  */
		RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, 0x42);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R11, 0x51);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R13, 0x3C);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, 0x80);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R18, 0xF1);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R19, 0xA1);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R20, 0x01);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, 0xA0);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R21, 0x12);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R22, 0xA1);
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &macValue_Tx_Cfg0);

	if (AntIdx == 0) {
		macValue_2nd = macValue_Tx_Cfg0 & 0x3f;

		if ((txALC_req_sat & 0x3) > 1) {
			pAd->Tx0_DPD_ALC_tag1 = txALC_req_sat + 4;
			pAd->Tx0_DPD_ALC_tag0 = txALC_req_sat;
			macValue_2nd = macValue_2nd + 4;
		} else {
			pAd->Tx0_DPD_ALC_tag1 = txALC_req_sat;
			pAd->Tx0_DPD_ALC_tag0 = txALC_req_sat - 4;
			macValue_2nd = macValue_2nd - 4;
		}

		macValue_2nd = macValue_2nd | (macValue_Tx_Cfg0 & (~0x3f));
	} else {
		macValue_2nd = (macValue_Tx_Cfg0 >> 8) & 0x3f;

		if ((txALC_req_sat & 0x3) > 1) {
			pAd->Tx1_DPD_ALC_tag1 = txALC_req_sat + 4;
			pAd->Tx1_DPD_ALC_tag0 = txALC_req_sat;
			macValue_2nd = macValue_2nd + 4;
		} else {
			pAd->Tx1_DPD_ALC_tag1 = txALC_req_sat;
			pAd->Tx1_DPD_ALC_tag0 = txALC_req_sat - 4;
			macValue_2nd = macValue_2nd - 4;
		}

		macValue_2nd = (macValue_2nd << 8) | (macValue_Tx_Cfg0 & (~0x3f00));
	}

	if (AntIdx == 0) {
		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
	} else {
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;
	}

	for (check_loop = 0; check_loop < 2; check_loop++) {
		if (check_loop == 0)
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, macValue_Tx_Cfg0);
		else
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, macValue_2nd);

		/* Disable Tx/Rx */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

		/* Check MAC Tx/Rx idle */
		for (k_count = 0; k_count < 1000; k_count++) {
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

			if (macStatus & 0x3)
				RtmpusecDelay(50);
			else
				break;
		}

		if (k_count == 1000)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nWait MAC Status to MAX  !!!\n"));

		/* Transmit packet */
		/* ====================================== */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x04);
		RtmpKickOutHwNullFrame(pAd, FALSE, TRUE);
		/* Disable Tx/Rx */
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

		/* Check MAC Tx/Rx idle */
		for (k_count = 0; k_count < 500; k_count++) {
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

			if (macStatus & 0x3)
				RtmpusecDelay(100);
			else
				break;
		}

		if (k_count == 500)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\nWait MAC Status to MAX  !!!\n"));

		/* Set BBP DPD parameters through MAC registers  */
		if (AntIdx == 0)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x10);
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x10);

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R109, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R110, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, 0x39);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x02);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x03);

		if (AntIdx == 0)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x41);
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x21);

		if (pAd->CommonCfg.Chip_VerID > 1) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, 0x00);
		} else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, 0x10);

		Max_Retry = DPD_CAL_MAX_RETRY;
		Pass_Thres = DPD_CAL_PASS_THRES;
		/* Init VGA Gain */
		VGA_code_idx = 5;
		VGA_Upper_Bound = 245;
		VGA_Lower_Bound = 180;
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		for (index = 0; index < Max_Retry; index++) {
			while ((VGA_code_idx >= 0) && (VGA_code_idx <= 18)) {
				VGA_code = RT6352_RFR3R4_TABLE[VGA_code_idx].Value;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RF Gain(%d), RF R3/4(%x)\n", RT6352_RFR3R4_TABLE[VGA_code_idx].Register, VGA_code));

				if (AntIdx == 0) {
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, (0x20 | VGA_code));
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, (0x20 | VGA_code));
				} else {
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, (0x20 | VGA_code));
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, (0x20 | VGA_code));
				}

				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x08);
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

				/* Check MAC Tx/Rx idle */
				for (k_count = 0; k_count < 10000; k_count++) {
					RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

					if (macStatus & 0x3)
						RtmpusecDelay(50);
					else
						break;
				}

				if (k_count == 10000)
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("2. Wait MAC Status to MAX  !!!\n"));

				/* Turn on debug tone and start DPD calibration through MAC registers */
				if (pAd->CommonCfg.Chip_VerID > 1)
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x11);
				else
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x2A);

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x01);

				if (AntIdx == 0) {
					if (check_loop == 0)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x80);
					else
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x81);
				} else {
					if (check_loop == 0)
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x82);
					else
						RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x83);
				}

				/* Wait up to 1ms for capture buffer to fill */
				for (k_count = 0; k_count < 1000; k_count++) {
					RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &byteValue);

					if ((byteValue & 0x80) == 0)
						break;

					RtmpusecDelay(50);
				}

				if (k_count == 1000)
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wait capture buffer to MAX  !!!\n"));

				/* Turn off debug tone */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x00);
				/* Check if VGA is appropriate (signal has large swing but not clipped in ADC) */
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x01);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0xFF);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &byteValue);
				AM_63 = byteValue;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("AM_63 (%d)\n", AM_63));

				if (VGA_Lower_Bound > AM_63)
					VGA_code_idx++;
				else if (VGA_Upper_Bound < AM_63)
					VGA_code_idx--;
				else {
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Appropriate VGA Gain(%d), R66(%x)\n", RT6352_VGA_TABLE[VGA_code_idx].Register, VGA_code));
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("AM_63 (%d)\n", AM_63));
					break;
				}
			}

			/* VGA_code cannot be found, report error and stop DPD calibration */
			if ((VGA_code_idx < 0) || (VGA_code_idx > 18)) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" VGA Code idx overflow(%d), AM_63(%d) !!!\n", VGA_code_idx, AM_63));
				DPD_Cal_success = FALSE;
				break;
			}

			AM_SUM = 0;
			AM_10 = 0;
			/* DPD calibration protection mechanism */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x01);

			for (k_count = 0; k_count < 11; k_count++) {
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, ((4 * k_count) + 3));
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &byteValue);
				AM_SUM += byteValue;
			}

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x2B);
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R188, &byteValue);
			AM_10 = byteValue;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("AM_SUM = %d, AM_10 = %d !!!\n", AM_SUM, AM_10));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("(AM_SUM - 5.5*AM_10) = (%d) @@\n", (AM_SUM - ((11 * AM_10) / 2))));

			if (AntIdx == 0) {
				if (((AM_SUM - ((11 * AM_10) / 2)) >= -3) && ((AM_SUM - ((11 * AM_10) / 2)) <= Pass_Thres)) {
					MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DPD Calibration Pass for TX0 !!!\n"));

					if (check_loop == 0)
						pAd->Tx0_DPD_ALC_tag0_flag = 0x1;
					else if (check_loop == 1)
						pAd->Tx0_DPD_ALC_tag1_flag = 0x1;

					DPD_Cal_success = TRUE;
					break;
				}

				if (index == 3)
					Pass_Thres += 3;
			} else {
				if (index == 4) {
					if (((AM_SUM - ((11 * AM_10) / 2)) >= -9) && ((AM_SUM - ((11 * AM_10) / 2)) <= 12)) {
						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DPD Calibration Pass for TX1 !!!\n"));

						if (check_loop == 0)
							pAd->Tx1_DPD_ALC_tag0_flag = 0x1;
						else if (check_loop == 1)
							pAd->Tx1_DPD_ALC_tag1_flag = 0x1;

						DPD_Cal_success = TRUE;
						break;
					}
				} else {
					if (((AM_SUM - ((11 * AM_10) / 2)) >= -5) && ((AM_SUM - ((11 * AM_10) / 2)) <= 5)) {
						MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DPD Calibration Pass for TX1 !!!\n"));

						if (check_loop == 0)
							pAd->Tx1_DPD_ALC_tag0_flag = 0x1;
						else if (check_loop == 1)
							pAd->Tx1_DPD_ALC_tag1_flag = 0x1;

						DPD_Cal_success = TRUE;
						break;
					}
				}
			}
		}

	}

	/* Restore RF registers */
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, saveRfB0R1);

	if (AntIdx == 0) {
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R01, saveRfB5R1);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, saveRfB5R17);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, saveRfB5R18);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, saveRfB5R19);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, saveRfB5R20);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R11, saveRfB4R11);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R13, saveRfB4R13);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, saveRfB4R19);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R21, saveRfB4R21);
		RT635xWriteRFRegister(pAd, RF_BANK4, RF_R22, saveRfB4R22);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, saveRfB5R3);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, saveRfB5R4);
	} else {
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R01, saveRfB7R1);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, saveRfB7R17);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R18, saveRfB7R18);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R19, saveRfB7R19);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R20, saveRfB7R20);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R11, saveRfB6R11);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R13, saveRfB6R13);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, saveRfB6R19);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R21, saveRfB6R21);
		RT635xWriteRFRegister(pAd, RF_BANK6, RF_R22, saveRfB6R22);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, saveRfB7R3);
		RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, saveRfB7R4);
	}

	/* Restore BBP registers */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
	byteValue &= (~0x18);

	if (wlan_operate_get_bw(wdev) == BW_40)
		byteValue |= 0x10;

#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		PATE_INFO pATEInfo = &(pAd->ate);

		byteValue &= (~0x18);

		if (pATEInfo->TxWI.TXWI_O.BW == BW_40)
			byteValue |= 0x10;
	}

#endif /* CONFIG_ATE */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x1);
	RtmpusecDelay(2);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, saveBbpR27);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R65, saveBbpR65);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBP_R140);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, saveBbpR159);

	if (pAd->CommonCfg.Chip_VerID > 1) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, saveBbpR241);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, saveBbpR242);
	} else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, saveBbpR242);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, saveBbpR244);
	/* Restore MAC registers */
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, saveTxAlgCfg0);
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, saveTxPinCfg);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
	return 2;
}

VOID DoDPDCalibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR Ant0 = 0, Ant1 = 0;
	UCHAR byteValue = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);
#ifdef CONFIG_ATE
	UCHAR saveBbpR1 = 0, saveRfB0R2 = 0, BBPValue = 0, RFValue = 0;
#endif /* CONFIG_ATE */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" Do DPD Calibration !!!\n"));
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &saveBbpR1);
		BBPValue = saveBbpR1;
		BBPValue &= (~0x18);
		BBPValue |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &saveRfB0R2);
		RFValue = saveRfB0R2;
		RFValue |= 0x33;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);
	}

#endif /* CONFIG_ATE */
	Ant0 = DPD_Calibration(pAd, 0);

	if (pAd->Antenna.field.TxPath > 1)
		Ant1 = DPD_Calibration(pAd, 1);

#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, saveBbpR1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, saveRfB0R2);
	}

#endif /* CONFIG_ATE */

	if ((Ant0 == 2) || (Ant1 == 2)) {
		/* Disable DPD Compensation for Pa_mode 1 and 3 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if (pAd->Tx0_DPD_ALC_tag0_flag == 0) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x02);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x02);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		} else if (pAd->Tx0_DPD_ALC_tag1_flag == 0) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x03);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x03);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		} else if (pAd->Tx1_DPD_ALC_tag0_flag == 0) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		} else if (pAd->Tx1_DPD_ALC_tag1_flag == 0) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x05);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x05);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);

		if (pAd->CommonCfg.Chip_VerID == 1) {
			DPD_IQ_Swap_AM_PM_Inversion(pAd, 0);

			if (pAd->Antenna.field.TxPath > 1)
				DPD_IQ_Swap_AM_PM_Inversion(pAd, 1);
		}

		if (Ant0 == 2)
			DPD_AM_AM_LUT_Scaling(pAd, 0);

		if (pAd->Antenna.field.TxPath > 1) {
			if (Ant1 == 2)
				DPD_AM_AM_LUT_Scaling(pAd, 1);
		}

		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);

		if (wlan_operate_get_bw(wdev) == BW_40)
			byteValue |= 0x10;

#ifdef CONFIG_ATE

		if (ATE_ON(pAd)) {
			PATE_INFO pATEInfo = &(pAd->ate);

			byteValue &= (~0x18);

			if (pATEInfo->TxWI.TXWI_O.BW == BW_40)
				byteValue |= 0x10;
		}

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if ((pAd->Tx0_DPD_ALC_tag0_flag == 0) &&
			(pAd->Tx0_DPD_ALC_tag1_flag == 0) &&
			(pAd->Tx1_DPD_ALC_tag0_flag == 0) &&
			(pAd->Tx1_DPD_ALC_tag1_flag == 0)) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" Enable DPD Compensation !!!\n"));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" DPD Calibration Ant00 = %d, Ant01 = %d, Ant10 = %d,  Ant11 = %d!!!\n",
					 pAd->Tx0_DPD_ALC_tag0_flag, pAd->Tx0_DPD_ALC_tag1_flag,
					 pAd->Tx1_DPD_ALC_tag0_flag, pAd->Tx1_DPD_ALC_tag1_flag));
			/* Enable DPD Compensation */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x04);
#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x10);
#else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x1C);
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x05);
#ifdef ADJUST_POWER_CONSUMPTION_SUPPORT
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x10);
#else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x12);
#endif /* ADJUST_POWER_CONSUMPTION_SUPPORT */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0xC0);
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x0);
	} else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" DPD Calibration Ant0 = %d, Ant1 = %d !!!\n", Ant0, Ant1));

	return;
}

INT Set_TestDPDCalibration_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
#ifdef CONFIG_ATE
	PATE_INFO pATEInfo = &(pAd->ate);
#endif /* CONFIG_ATE */
	INT bDPDCalibrationEnable = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" TestDPDCalibration !!!\n"));
	pAd->bCalibrationDone = FALSE;
	bDPDCalibrationEnable = os_str_tol(arg, 0, 10);

	if (bDPDCalibrationEnable == 0) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = FALSE;

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		pAd->bCalibrationDone = TRUE;
		return TRUE;
	}

	if (bDPDCalibrationEnable == 1) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = TRUE;

#endif /* CONFIG_ATE */
		pAd->Tx0_DPD_ALC_tag0 = 0;
		pAd->Tx0_DPD_ALC_tag1 = 0;
		pAd->Tx1_DPD_ALC_tag0 = 0;
		pAd->Tx1_DPD_ALC_tag1 = 0;
		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;
	}

	DoDPDCalibration(pAd);
	pAd->bCalibrationDone = TRUE;
	return TRUE;
}

INT Set_TestDPDCalibrationTX0_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	UCHAR Ant0 = 0;
	UCHAR byteValue = 0;
#ifdef CONFIG_ATE
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR saveBbpR1 = 0, saveRfB0R2 = 0, BBPValue = 0, RFValue = 0;
#endif /* CONFIG_ATE */
	INT bDPDCalibrationEnable = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" TestDPDCalibrationTX0 !!!\n"));
	pAd->bCalibrationDone = FALSE;
	bDPDCalibrationEnable = os_str_tol(arg, 0, 10);

	if (bDPDCalibrationEnable == 0) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = FALSE;

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		pAd->bCalibrationDone = TRUE;
		return TRUE;
	}

#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &saveBbpR1);
		BBPValue = saveBbpR1;
		BBPValue &= (~0x18);
		BBPValue |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &saveRfB0R2);
		RFValue = saveRfB0R2;
		RFValue |= 0x33;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);
	}

#endif /* CONFIG_ATE */

	if (bDPDCalibrationEnable == 1) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = TRUE;

#endif /* CONFIG_ATE */
		pAd->Tx0_DPD_ALC_tag0 = 0;
		pAd->Tx0_DPD_ALC_tag1 = 0;
		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
	}

	Ant0 = DPD_Calibration(pAd, 0);
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, saveBbpR1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, saveRfB0R2);
	}

#endif /* CONFIG_ATE */

	if (Ant0 == 2) {
		/* Disable DPD Compensation for Pa_mode 1 and 3 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);

		if (pAd->CommonCfg.Chip_VerID == 1)
			DPD_IQ_Swap_AM_PM_Inversion(pAd, 0);

		DPD_AM_AM_LUT_Scaling(pAd, 0);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);

		if (wlan_operate_get_bw(wdev) == BW_40)
			byteValue |= 0x10;

#ifdef CONFIG_ATE

		if (ATE_ON(pAd)) {
			byteValue &= (~0x18);

			if (pATEInfo->TxWI.TXWI_O.BW == BW_40)
				byteValue |= 0x10;
		}

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if ((pAd->Tx0_DPD_ALC_tag0_flag == 0) &&
			(pAd->Tx0_DPD_ALC_tag1_flag == 0)) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" Enable DPD Compensation !!!\n"));
			/* Enable DPD Compensation */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x1C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x05);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x1C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0xC0);
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x0);
	}

	pAd->bCalibrationDone = TRUE;
	return TRUE;
}

INT Set_TestDPDCalibrationTX1_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	UCHAR Ant1 = 0;
	UCHAR byteValue = 0;
#ifdef CONFIG_ATE
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR saveBbpR1 = 0, saveRfB0R2 = 0, BBPValue = 0, RFValue = 0;
#endif /* CONFIG_ATE */
	INT bDPDCalibrationEnable = 0;
	struct wifi_dev *wdev = get_default_wdev(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" TestDPDCalibrationTX1 !!!\n"));
	pAd->bCalibrationDone = FALSE;
	bDPDCalibrationEnable = os_str_tol(arg, 0, 10);

	if (bDPDCalibrationEnable == 0) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = FALSE;

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		pAd->bCalibrationDone = TRUE;
		return TRUE;
	}

#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &saveBbpR1);
		BBPValue = saveBbpR1;
		BBPValue &= (~0x18);
		BBPValue |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &saveRfB0R2);
		RFValue = saveRfB0R2;
		RFValue |= 0x33;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);
	}

#endif /* CONFIG_ATE */

	if (bDPDCalibrationEnable == 1) {
#ifdef CONFIG_ATE

		if (ATE_ON(pAd))
			pATEInfo->bDPDEnable = TRUE;

#endif /* CONFIG_ATE */
		pAd->Tx1_DPD_ALC_tag0 = 0;
		pAd->Tx1_DPD_ALC_tag1 = 0;
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;
	}

	Ant1 = DPD_Calibration(pAd, 1);
#ifdef CONFIG_ATE

	if (ATE_ON(pAd)) {
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, saveBbpR1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, saveRfB0R2);
	}

#endif /* CONFIG_ATE */

	if (Ant1 == 2) {
		/* Disable DPD Compensation for Pa_mode 1 and 3 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);
		byteValue |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x06);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x07);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x08);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x09);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0A);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0B);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0C);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x0D);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R189, 0x0F);

		if (pAd->CommonCfg.Chip_VerID == 1)
			DPD_IQ_Swap_AM_PM_Inversion(pAd, 1);

		DPD_AM_AM_LUT_Scaling(pAd, 1);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &byteValue);
		byteValue &= (~0x18);

		if (wlan_operate_get_bw(wdev) == BW_40)
			byteValue |= 0x10;

#ifdef CONFIG_ATE

		if (ATE_ON(pAd)) {
			byteValue &= (~0x18);

			if (pATEInfo->TxWI.TXWI_O.BW == BW_40)
				byteValue |= 0x10;
		}

#endif /* CONFIG_ATE */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, byteValue);

		if ((pAd->Tx1_DPD_ALC_tag0_flag == 0) &&
			(pAd->Tx1_DPD_ALC_tag1_flag == 0)) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x00);
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" Enable DPD Compensation !!!\n"));
			/* Enable DPD Compensation */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R186, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x04);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x1C);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x05);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0x12);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R187, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R188, 0xC0);
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x0);
	}

	pAd->bCalibrationDone = TRUE;
	return TRUE;
}

INT Set_DPDCalPassThres_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	cap->DPDCalPassThres = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("DPD Pass Threshold = %d\n", cap->DPDCalPassThres));
	return TRUE;
}

/****************************************************************************
 *
 *   VOID TX Filter BandWidth Calibraton
 *
 ****************************************************************************/
static INT BBP_Core_Soft_Reset(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN set_bw,
	IN INT bw)
{
	UINT8 bbp_val;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &bbp_val);
	bbp_val |= 0x1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbp_val);
	RtmpusecDelay(100);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &bbp_val);

	if (set_bw == TRUE) {
		bbp_val &= (~0x18);

		switch (bw) {
		case BW_40:
			bbp_val |= 0x10;
			break;

		case BW_20:
		default:
			break;
		}

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbp_val);
		RtmpusecDelay(100);
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &bbp_val);
	bbp_val &= (~0x1);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbp_val);
	RtmpusecDelay(100);
	return 0;
}

static INT RF_lp_Config(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bTxCal)
{
	UCHAR rf_val;

	if (bTxCal) {
		RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x04);
		RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x06);
	} else {
		RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x02);
		RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x06);
	}

	RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &rf_val);
	rf_val |= 0x80;
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, rf_val);

	if (bTxCal) {
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0xC1);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0x20);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x02);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &rf_val);
		rf_val &= (~0x3F);
		rf_val |= 0x3F;
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, rf_val);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &rf_val);
		rf_val &= (~0x3F);
		rf_val |= 0x3F;
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, rf_val);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R05, 0x31);
	} else {
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0xF1);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0x18);
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x02);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &rf_val);
		rf_val &= (~0x3F);
		rf_val |= 0x34;
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, rf_val);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &rf_val);
		rf_val &= (~0x3F);
		rf_val |= 0x34;
		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, rf_val);
	}

	return 0;
}

static CHAR lp_Tx_Filter_BW_Cal(
	IN PRTMP_ADAPTER pAd)
{
	INT cnt;
	UINT8 bbp_val;
	CHAR cal_val;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x82);
	cnt = 0;

	do {
		RtmpusecDelay(500);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbp_val);

		if (bbp_val == 0x02 || cnt == 20)
			break;

		cnt++;
	} while (cnt < 20);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x39);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbp_val);
	cal_val = bbp_val & 0x7F;

	if (cal_val >= 0x40)
		cal_val -= 128;

	return cal_val;
}


/* RF Bandwidth calibration */
BOOLEAN BW_Filter_Calibration(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bTxCal)
{
	UINT8 tx_agc_fc = 0, rx_agc_fc = 0, cmm_agc_fc;
	UINT8 filter_target;
	UINT8 tx_filter_target_20m = 0x09, tx_filter_target_40m = 0x02;
	UINT8 rx_filter_target_20m = 0x27, rx_filter_target_40m = 0x31;
	INT loop = 0, bw, cnt;
	UINT8 bbp_val, rf_val;
	CHAR cal_r32_init, cal_r32_val, cal_diff;
	UINT8 saveRfB5R00, saveRfB5R01, saveRfB5R03, saveRfB5R04, saveRfB5R05;
	UINT8 saveRfB5R06, saveRfB5R07;
	UINT8 saveRfB5R08, saveRfB5R17, saveRfB5R18, saveRfB5R19, saveRfB5R20;
	UINT8 saveRfB5R37, saveRfB5R38, saveRfB5R39, saveRfB5R40, saveRfB5R41;
	UINT8 saveRfB5R42, saveRfB5R43, saveRfB5R44, saveRfB5R45, saveRfB5R46;
	UINT8 saveRfB5R58, saveRfB5R59;
	UINT8 saveBBP159R0, saveBBP159R2, saveBBPR23;
	UINT32 MAC_RF_CONTROL0, MAC_RF_BYPASS0;
	struct wifi_dev *wdev = get_default_wdev(pAd);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" %s BW Filter Calibration !!!\n", (bTxCal == TRUE ? "TX" : "RX")));
	/* Save MAC registers */
	RTMP_IO_READ32(pAd, RF_CONTROL0, &MAC_RF_CONTROL0);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &MAC_RF_BYPASS0);
	/* save BBP registers */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R23, &saveBBPR23);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x0);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &saveBBP159R0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x02);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &saveBBP159R2);
	/* Save RF registers */
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R00, &saveRfB5R00);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &saveRfB5R01);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &saveRfB5R03);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &saveRfB5R04);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R05, &saveRfB5R05);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &saveRfB5R06);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &saveRfB5R07);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R08, &saveRfB5R08);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &saveRfB5R17);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &saveRfB5R18);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &saveRfB5R19);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &saveRfB5R20);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R37, &saveRfB5R37);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R38, &saveRfB5R38);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R39, &saveRfB5R39);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R40, &saveRfB5R40);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R41, &saveRfB5R41);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R42, &saveRfB5R42);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R43, &saveRfB5R43);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R44, &saveRfB5R44);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R45, &saveRfB5R45);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R46, &saveRfB5R46);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &saveRfB5R58);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &saveRfB5R59);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R00, &rf_val);
	rf_val |= 0x3;
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R00, rf_val);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &rf_val);
	rf_val |= 0x1;
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R01, rf_val);
	cnt = 0;

	do {
		RtmpusecDelay(500);
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &rf_val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("ABB.R1 = %d, Cnt = %d\n", rf_val, cnt));

		if (((rf_val & 0x1) == 0x00) || (cnt == 40))
			break;

		cnt++;
	} while (cnt < 40);

	RT635xReadRFRegister(pAd, RF_BANK5, RF_R00, &rf_val);
	rf_val &= (~0x3);
	rf_val |= 0x1;
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R00, rf_val);
	/* I-3 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R23, &bbp_val);
	bbp_val &= (~0x1F);
	bbp_val |= 0x10;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, bbp_val);

	do {
		/* I-4,5,6,7,8,9 */
		if (loop == 0) {
			bw = BW_20;

			if (bTxCal)
				filter_target = tx_filter_target_20m;
			else
				filter_target = rx_filter_target_20m;
		} else {
			bw = BW_40;

			if (bTxCal)
				filter_target = tx_filter_target_40m;
			else
				filter_target = rx_filter_target_40m;
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Start BW Cal for %dMHz\n",
				 __func__, (bw == BW_20 ? 20 : 40)));
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R08, &rf_val);
		rf_val &= (~0x04);

		if (loop == 1)
			rf_val |= 0x4;

		RT635xWriteRFRegister(pAd, RF_BANK5, RF_R08, rf_val);
		BBP_Core_Soft_Reset(pAd, TRUE, bw);
		RF_lp_Config(pAd, bTxCal);

		if (bTxCal) {
			tx_agc_fc = 0;
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &rf_val);
			rf_val &= (~0x7F);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, rf_val);
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &rf_val);
			rf_val &= (~0x7F);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, rf_val);
		} else {
			rx_agc_fc = 0;
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &rf_val);
			rf_val &= (~0x7F);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, rf_val);
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &rf_val);
			rf_val &= (~0x7F);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, rf_val);
		}

		RtmpusecDelay(1000);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbp_val);
		bbp_val &= (~0x6);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, bbp_val);
		BBP_Core_Soft_Reset(pAd, FALSE, bw);
		cal_r32_init = lp_Tx_Filter_BW_Cal(pAd);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbp_val);
		bbp_val |= 0x6;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, bbp_val);
do_cal:

		if (bTxCal) {
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &rf_val);
			rf_val &= (~0x7F);
			rf_val |= tx_agc_fc;
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, rf_val);
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &rf_val);
			rf_val &= (~0x7F);
			rf_val |= tx_agc_fc;
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, rf_val);
		} else {
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &rf_val);
			rf_val &= (~0x7F);
			rf_val |= rx_agc_fc;
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, rf_val);
			RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &rf_val);
			rf_val &= (~0x7F);
			rf_val |= rx_agc_fc;
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, rf_val);
		}

		RtmpusecDelay(500);
		BBP_Core_Soft_Reset(pAd, FALSE, bw);
		cal_r32_val = lp_Tx_Filter_BW_Cal(pAd);
		cal_diff = cal_r32_init - cal_r32_val;

		if (bTxCal)
			cmm_agc_fc = tx_agc_fc;
		else
			cmm_agc_fc = rx_agc_fc;

		if (((cal_diff > filter_target) && (cmm_agc_fc == 0)) ||
			((cal_diff < filter_target) && (cmm_agc_fc == 0x3f))) {

			if (bTxCal)
				tx_agc_fc = 0;
			else
				rx_agc_fc = 0;
		} else if ((cal_diff <= filter_target) && (cmm_agc_fc < 0x3f)) {
			if (bTxCal)
				tx_agc_fc++;
			else
				rx_agc_fc++;

			goto do_cal;
		} else {
			/* do nothing */
		}


		if (bTxCal) {
			if (loop == 0)
				pAd->tx_bw_cal[0] = tx_agc_fc;
			else
				pAd->tx_bw_cal[1] = tx_agc_fc;
		} else {
			if (loop == 0)
				pAd->rx_bw_cal[0] = rx_agc_fc;
			else
				pAd->rx_bw_cal[1] = rx_agc_fc;
		}

		loop++;
	} while (loop <= 1);

	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R00, saveRfB5R00);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R01, saveRfB5R01);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, saveRfB5R03);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, saveRfB5R04);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R05, saveRfB5R05);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, saveRfB5R06);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, saveRfB5R07);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R08, saveRfB5R08);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, saveRfB5R17);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, saveRfB5R18);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, saveRfB5R19);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, saveRfB5R20);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R37, saveRfB5R37);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R38, saveRfB5R38);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R39, saveRfB5R39);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R40, saveRfB5R40);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R41, saveRfB5R41);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R42, saveRfB5R42);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R43, saveRfB5R43);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R44, saveRfB5R44);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R45, saveRfB5R45);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R46, saveRfB5R46);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, saveRfB5R58);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, saveRfB5R59);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, saveBBPR23);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, saveBBP159R0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x2);
	/* RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbp_val); */
	/* bbp_val &= (~0x6); */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, saveBBP159R2);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &bbp_val);
	bbp_val &= (~0x18);

	if (wlan_operate_get_bw(wdev) == BW_40)
		bbp_val |= 0x10;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbp_val);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MAC_RF_CONTROL0);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MAC_RF_BYPASS0);
	return TRUE;
}

/* Rx DCOC Calibration */
VOID RxDCOC_Calibration(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR BbpReg = 0;
	UINT32 MacValue = 0, MacValue1 = 0;
	USHORT k_count = 0;
	UINT8 saveRfB0R2, saveRfB5R4, saveRfB7R4, RFValue;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" RxDCOC_Calibration !!!\n"));
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &saveRfB0R2);
	RFValue = saveRfB0R2;
	RFValue |= 0x03;
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFValue);
	/* Write BBP CAL R141 bit[4] = 1. (Enable calibrate MID-GAIN) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBP_R141);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BbpReg);
	BbpReg |= 0x10;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BbpReg);
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x8);

	for (k_count = 0; k_count < 10000; k_count++) {
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue1);

		if (MacValue1 & 0x1)
			RtmpusecDelay(50);
		else
			break;
	}

	RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &saveRfB5R4);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R04, &saveRfB7R4);
	saveRfB5R4 = saveRfB5R4 & (~0x40);
	saveRfB7R4 = saveRfB7R4 & (~0x40);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x64);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x64);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, saveRfB5R4);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, saveRfB7R4);
	/* Write BBP CAL R140 bit [6],[3] to 1 (Start Full,Gainfreeze calibration) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBP_R140);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BbpReg);
	BbpReg = BbpReg & (~0x40);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BbpReg);
	BbpReg |= 0x48;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BbpReg);

	/* Polling CAL R140 bit[6] = 0 (Cal done) */
	for (k_count = 0; k_count < 10000; k_count++) {
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BbpReg);

		if ((BbpReg & 0x40) == 0)
			break;

		RtmpusecDelay(50);
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BbpReg);
	BbpReg = BbpReg & (~0x40);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BbpReg);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);
	/* Write CAL R141 bit[4] = 0 (Disable calibrate MID-GAIN) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBP_R141);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BbpReg);
	BbpReg &= (~0x10);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BbpReg);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, saveRfB0R2);
}

/****************************************************************************
 *
 *   VOID RXIQCalibraton(VOID)
 *
 ****************************************************************************/
static UINT32 do_sqrt_accumulation(UINT32 si)
{
	UINT32 root, root_pre, bit;
	CHAR   i;

	bit = 1 << 15;
	root = 0;

	for (i = 15; i >= 0; i = i - 1) {
		root_pre = root + bit;

		if ((root_pre * root_pre) <= si)
			root = root_pre;

		bit = bit >> 1;
	}

	return root;
}

VOID RXIQ_Calibration(
	IN PRTMP_ADAPTER pAd)
{
	UINT8 RFB0R1, RFB0R2, RFB0R42;
	UINT8 RFB4R0, RFB4R19;
	UINT8 RFB5R3, RFB5R4, RFB5R17, RFB5R18, RFB5R19, RFB5R20;
	UINT8 RFB6R0, RFB6R19;
	UINT8 RFB7R3, RFB7R4, RFB7R17, RFB7R18, RFB7R19, RFB7R20;
	UINT8 BBP1, BBP4;
	UINT8 BBPR241, BBPR242;
	UINT32 i;
	UINT8 ch_idx;
	UINT8 bbpval;
	UINT8 rfval, vga_idx = 0;
	INT32 mi = 0, mq = 0, si = 0, sq = 0, riq = 0;
	INT32 Sigma_i, Sigma_q, R_iq, G_rx;
	INT32 G_imb;
	INT32 Ph_rx;
	UINT32 saveMacSysCtrl = 0;
	UINT32 orig_RF_CONTROL0 = 0; /* 0x0518 */
	UINT32 orig_RF_BYPASS0  = 0; /* 0x051c */
	UINT32 orig_RF_CONTROL1 = 0; /* 0x0520 */
	UINT32 orig_RF_BYPASS1  = 0; /* 0x0524 */
	UINT32 orig_RF_CONTROL3 = 0; /* 0x0530 */
	UINT32 orig_RF_BYPASS3  = 0; /* 0x0534 */
	UINT32 macStatus, k_count, bbpval1 = 0;
	UCHAR rf_vga_table[] = {0x20, 0x21, 0x22,
							0x38, 0x39, 0x3a,
							0x3b, 0x3c, 0x3d,
							0x3e, 0x3f
						   };
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" RxIQ Calibration !!!\n"));
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, RF_CONTROL0, &orig_RF_CONTROL0);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &orig_RF_BYPASS0);
	RTMP_IO_READ32(pAd, RF_CONTROL1, &orig_RF_CONTROL1);
	RTMP_IO_READ32(pAd, RF_BYPASS1, &orig_RF_BYPASS1);
	RTMP_IO_READ32(pAd, RF_CONTROL3, &orig_RF_CONTROL3);
	RTMP_IO_READ32(pAd, RF_BYPASS3, &orig_RF_BYPASS3);
	/* BBP store */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBP1);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBP4);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	/* Check MAC Tx/Rx idle */
	for (k_count = 0; k_count < 10000; k_count++) {
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);

		if (macStatus & 0x3)
			RtmpusecDelay(50);
		else
			break;
	}

	if (k_count == 10000)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wait MAC Status to MAX  !!!\n"));

	bbpval = BBP4 & (~0x18);
	bbpval = BBP4 | 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbpval);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &bbpval);
	bbpval = bbpval | 1;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);
	bbpval = bbpval & 0xFE;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);
	RTMP_IO_WRITE32(pAd, RF_CONTROL1, 0x00000202); /* 0x0520 */
	RTMP_IO_WRITE32(pAd, RF_BYPASS1, 0x00000303); /* 0x0524 */
#ifdef RT6352_EP_SUPPORT
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, 0x0101); /* 0x0530 */
#else
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, 0x0000); /* 0x0530 */
#endif /* RT6352_EP_SUPPORT */
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, 0xF1F1); /* 0x0534 */
	/* B)   Store RF Original Setting */
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &RFB0R1);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &RFB0R2);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFB0R42);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R00,  &RFB4R0);
	RT635xReadRFRegister(pAd, RF_BANK4, RF_R19, &RFB4R19);
	/* RF TRx0 VGA gain store */
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFB5R3);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &RFB5R4);
	/* RF TRx0 Loopback path force */
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &RFB5R17);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &RFB5R18);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &RFB5R19);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &RFB5R20);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R00,  &RFB6R0);
	RT635xReadRFRegister(pAd, RF_BANK6, RF_R19, &RFB6R19);
	/* RF TRx1 VGA gain store */
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFB7R3);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R04, &RFB7R4);
	/* RF TRx1 Loopback path force */
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R17, &RFB7R17);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R18, &RFB7R18);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R19, &RFB7R19);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R20, &RFB7R20);
	/* ) RF Loopback Setting*/
	/* RF common setting */
	/* TRx 0 loopback setting */
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R00, 0x87); /* force TR switch */
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, 0x27); /* force TR switch */
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, 0x38);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x38);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0xC1);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0x60);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x00);
	/* TRx 1 loopback setting */
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R00, 0x87); /* force TR switch */
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, 0x27); /* force TR switch */
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, 0x38);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x38);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R18, 0xC1);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R19, 0x60);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R20, 0x00);
	/* BBP Tx power control */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x5);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x0);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R241, &BBPR241);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R242, &BBPR242);
	/* C) Set Debug Tone (need to be disable after calibration) */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, 0x10);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, 0x84);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x31);
	/* Disable rx iq compensation */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x3);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbpval);
	bbpval = bbpval & (~0x7);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, bbpval);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000004); /* 0x0518 */
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000006); /* 0x0518 */
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x00003376); /* 0x051c */
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00001006); /* 0x0518 */
	RtmpusecDelay(1);
#ifdef RT6352_EP_SUPPORT
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x06);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x06);
#else
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x02);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x02);
#endif /* RT6352_EP_SUPPORT */

	/* BBP Rf Rx gain control */
	for (ch_idx = 0; ch_idx < 2; ch_idx = ch_idx + 1) {
		/* Path control */
		if (ch_idx == 0) {
			/* only on Tx0 */
			/* RF only Tx0 */
			rfval = RFB0R1 & (~0x3);
			rfval = RFB0R1 | 0x1;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, rfval);
			rfval = RFB0R2 & (~0x33);
			rfval = RFB0R2 | 0x11;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, rfval);
			rfval = RFB0R42 & (~0x50);
			rfval = RFB0R42 | 0x10;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, rfval);
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00001006); /* 0x0518 */
			RtmpusecDelay(1);
			/* BBP only Tx0 */
			bbpval = BBP1 & (~0x18);
			bbpval = bbpval | 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, bbpval);
			/* BBP only Rx0 */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x01);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);
		} else {
			/* RF only Tx1 */
			rfval = RFB0R1 & (~0x3);
			rfval = RFB0R1 | 0x2;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, rfval);
			rfval = RFB0R2 & (~0x33);
			rfval = RFB0R2 | 0x22;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, rfval);
			rfval = RFB0R42 & (~0x50);
			rfval = RFB0R42 | 0x40;
			RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, rfval);
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00002006); /* 0x0518 */
			RtmpusecDelay(1);
			/* BBP only Tx1 */
			bbpval = BBP1 & (~0x18);
			bbpval = bbpval | 0x08;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, bbpval);
			/* BBP only Rx1 */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x01);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x01);
		}

		RtmpusecDelay(500);
		/* e)   Read mi, mq, si, sq, riq */
		vga_idx = 0;

		while (vga_idx < 11) {
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, rf_vga_table[vga_idx]);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, rf_vga_table[vga_idx]);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, rf_vga_table[vga_idx]);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, rf_vga_table[vga_idx]);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x00);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x93);

			for (i = 0; i < 10000; i++) {
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbpval);

				if ((bbpval & 0xff) == 0x93)
					RtmpusecDelay(50);
				else
					break;
			}

			if ((bbpval & 0xff) == 0x93) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fatal Error: Calibration doesn't finish"));
				goto Restore_Value;
			}

			for (i = 0; i < 5; i++) { /* 0 mi, 1 mq, 2 si, 3 sq, 4 riq */
				UINT32 BBPtemp = 0;
				UINT8 value = 0;
				INT32 result = 0;

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x1E);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, i);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x22);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &value);
				BBPtemp = BBPtemp + (value << 24);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x21);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &value);
				BBPtemp = BBPtemp + (value << 16);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x20);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &value);
				BBPtemp = BBPtemp + (value << 8);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x1F);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &value);
				BBPtemp = BBPtemp + value;

				if ((i < 2) && (BBPtemp & 0x800000)) /* 23:0 */
					result = (BBPtemp & 0xFFFFFF) - 0x1000000;
				else if (i == 4)
					result = BBPtemp;
				else
					result = BBPtemp;

				if (i == 0)
					mi = result / 4096;
				else if (i == 1)
					mq = result / 4096;
				else if (i == 2)
					si = BBPtemp / 4096;
				else if (i == 3)
					sq = BBPtemp / 4096;
				else
					riq = result / 4096;
			}

			/* Software AGC */
			bbpval1 = si - mi * mi;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RXIQ si=%d, sq=%d, riq=%d, bbpval %d, vga_idx %d\n",
					 si, sq, riq, bbpval1, vga_idx));

			if (bbpval1 >= (100 * 100))
				break;

			if (bbpval1 <= 100)
				vga_idx = vga_idx + 9;
			else if (bbpval1 <= 158)
				vga_idx = vga_idx + 8;
			else if (bbpval1 <= 251)
				vga_idx = vga_idx + 7;
			else if (bbpval1 <= 398)
				vga_idx = vga_idx + 6;
			else if (bbpval1 <= 630)
				vga_idx = vga_idx + 5;
			else if (bbpval1 <= 1000)
				vga_idx = vga_idx + 4;
			else if (bbpval1 <= 1584)
				vga_idx = vga_idx + 3;
			else if (bbpval1 <= 2511)
				vga_idx = vga_idx + 2;
			else
				vga_idx = vga_idx + 1;
		}

		/* F)   Calculate Sigma_i, Sigma_q, R_iq */
		Sigma_i = do_sqrt_accumulation(100 * (si - mi * mi));
		Sigma_q = do_sqrt_accumulation(100 * (sq - mq * mq));
		R_iq = 10 * (riq - (mi * mq));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Sigma_i=%d, Sigma_q=%d, R_iq=%d\n", Sigma_i, Sigma_q, R_iq));

		if (((Sigma_i <= 1400) && (Sigma_i >= 1000))
			&& ((Sigma_i - Sigma_q) <= 112)
			&& ((Sigma_i - Sigma_q) >= -112)
			&& ((mi <= 32) && (mi >= -32))
			&& ((mq <= 32) && (mq >= -32))) {
			R_iq = 10 * (riq - (mi * mq));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RXIQ Sigma_i=%d, Sigma_q=%d, R_iq=%d\n", Sigma_i, Sigma_q, R_iq));
			/* G)   Calculate Gain/ Phase imbalance */
			G_rx = (1000 * Sigma_q) / Sigma_i;
			G_imb = ((-2) * 128 * (1000 - G_rx)) / (1000 + G_rx);
			Ph_rx = (R_iq * 2292) / (Sigma_i * Sigma_q);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RXIQ G_imb=%d, Ph_rx=%d\n", G_imb, Ph_rx));

			if ((Ph_rx > 20) || (Ph_rx < -20)) {
				Ph_rx = 0;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RXIQ calibration FAIL\n"));
			}

			if ((G_imb > 12) || (G_imb < -12)) {
				G_imb = 0;
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RXIQ calibration FAIL\n"));
			}
		} else {
			G_imb = 0;
			Ph_rx = 0;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RXIQ Sigma_i=%d, Sigma_q=%d, R_iq=%d\n", Sigma_i, Sigma_q, R_iq));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RXIQ calibration FAIL\n"));
		}

		/* H)   Write back compensation value and disable HW calculation */
		if (ch_idx == 0) {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x37);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, G_imb & 0x3F);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x35);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, Ph_rx & 0x3F);
		} else {
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x55);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, G_imb & 0x3F);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x53);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, Ph_rx & 0x3F);
		}
	}

Restore_Value:
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x3);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &bbpval);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, (bbpval | 0x07));
	/*BBP  Restore */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBP1);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBP4);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, BBPR241);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, BBPR242);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x00);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &bbpval);
	bbpval = (bbpval | 0x1);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);
	RtmpusecDelay(10);
	bbpval = (bbpval & 0xFE);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, bbpval);
	/* Bank 0 recover */
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, RFB0R1);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, RFB0R2);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFB0R42);
	/* Bank TRx0 recover */
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R00, RFB4R0);
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R19, RFB4R19);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFB5R3);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, RFB5R4);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, RFB5R17);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, RFB5R18);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, RFB5R19);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, RFB5R20);
	/* Bank TRx1 recover */
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R00, RFB6R0);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R19, RFB6R19);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFB7R3);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, RFB7R4);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, RFB7R17);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R18, RFB7R18);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R19, RFB7R19);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R20, RFB7R20);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000006); /* 0x0518 */
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000004); /* 0x0518 */
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, orig_RF_CONTROL0);
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, orig_RF_BYPASS0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL1, orig_RF_CONTROL1);
	RTMP_IO_WRITE32(pAd, RF_BYPASS1, orig_RF_BYPASS1);
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, orig_RF_CONTROL3);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, orig_RF_BYPASS3);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
}

VOID RF_SELF_TXDC_CAL(
	IN PRTMP_ADAPTER pAd)
{
	/* RF_CONTROL0: 0x0518 */
	/* RTMP_RF_BYPASS0 : 0x051c */
	/* RF_CONTROL2: 0x0528 */
	/* RF_BYPASS2 : 0x052c */
	UCHAR RfB5R1_Org, RfB7R1_Org, RFValue;
	UINT32 mac0518, mac051c, mac0528, mac052c;
	CHAR  i;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RF Tx_0 self calibration start\n"));
	RTMP_IO_READ32(pAd, RF_CONTROL0, &mac0518);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &mac051c);
	RTMP_IO_READ32(pAd, RF_CONTROL2, &mac0528);
	RTMP_IO_READ32(pAd, RF_BYPASS2, &mac052c);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x0);
	RTMP_IO_WRITE32(pAd, RF_BYPASS2, 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0xC);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x3306);
	RTMP_IO_WRITE32(pAd, RF_CONTROL2, 0x3330);
	RTMP_IO_WRITE32(pAd, RF_BYPASS2, 0xfffff);
	RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &RfB5R1_Org);
	RT635xReadRFRegister(pAd, RF_BANK7, RF_R01, &RfB7R1_Org);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R01, 0x4);

	for (i = 0; i < 100; i = i + 1) {
		RtmpusecDelay(50); /* wait 1 usec */
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R01, &RFValue);

		if ((RFValue & 0x04) != 0x4)
			break;
	}

	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R01, RfB5R1_Org);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R01, 0x4);

	for (i = 0; i < 100; i = i + 1) {
		RtmpusecDelay(50); /* wait 1 usec */
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R01, &RFValue);

		if ((RFValue & 0x04) != 0x4)
			break;
	}

	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R01, RfB7R1_Org);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x0);
	RTMP_IO_WRITE32(pAd, RF_BYPASS2, 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, mac0518);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, mac051c);
	RTMP_IO_WRITE32(pAd, RF_CONTROL2, mac0528);
	RTMP_IO_WRITE32(pAd, RF_BYPASS2, mac052c);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RF Tx self calibration end\n"));
}

