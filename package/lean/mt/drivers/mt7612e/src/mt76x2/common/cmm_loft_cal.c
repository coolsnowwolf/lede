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
	cmm_loft_cal.c

	Abstract:
	Tx LOFT calibration and profile related functions

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------

*/

#include "rt_config.h"

#ifdef RT6352
#define I_PATH	0x0
#define Q_PATH	0x1

#define CHAIN_0	0x0
#define CHAIN_1	0x1

#define RF_ALC_NUM 6 
#define CHAIN_NUM 2
#define _BBP_REG_NUM 168

typedef struct _RF_REG_PAIR
{
	UCHAR Bank;
	UCHAR Register;
	UCHAR Value;
} RF_REG_PAIR, *PRF_REG_PAIR;

static VOID RFConfigStore(
	IN RTMP_ADAPTER *pAd,
	OUT RF_REG_PAIR rf_reg_record[][13],
	IN UCHAR chain)
{
	UCHAR RFValue = 0;

	if (chain == CHAIN_0)
	{
		/* save before config */
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &RFValue);
		rf_reg_record[CHAIN_0][0].Bank = RF_BANK0;
		rf_reg_record[CHAIN_0][0].Register = RF_R01;
		rf_reg_record[CHAIN_0][0].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &RFValue);
		rf_reg_record[CHAIN_0][1].Bank = RF_BANK0;
		rf_reg_record[CHAIN_0][1].Register = RF_R02;
		rf_reg_record[CHAIN_0][1].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R35, &RFValue);
		rf_reg_record[CHAIN_0][2].Bank = RF_BANK0;
		rf_reg_record[CHAIN_0][2].Register = RF_R35;
		rf_reg_record[CHAIN_0][2].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFValue);
		rf_reg_record[CHAIN_0][3].Bank = RF_BANK0;
		rf_reg_record[CHAIN_0][3].Register = RF_R42;
		rf_reg_record[CHAIN_0][3].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK4, RF_R00, &RFValue);
		rf_reg_record[CHAIN_0][4].Bank = RF_BANK4;
		rf_reg_record[CHAIN_0][4].Register = RF_R00;
		rf_reg_record[CHAIN_0][4].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK4, RF_R02, &RFValue);
		rf_reg_record[CHAIN_0][5].Bank = RF_BANK4;
		rf_reg_record[CHAIN_0][5].Register = RF_R02;
		rf_reg_record[CHAIN_0][5].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK4, RF_R34, &RFValue);
		rf_reg_record[CHAIN_0][6].Bank = RF_BANK4;
		rf_reg_record[CHAIN_0][6].Register = RF_R34;
		rf_reg_record[CHAIN_0][6].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R03, &RFValue);
		rf_reg_record[CHAIN_0][7].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][7].Register = RF_R03;
		rf_reg_record[CHAIN_0][7].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R04, &RFValue);
		rf_reg_record[CHAIN_0][8].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][8].Register = RF_R04;
		rf_reg_record[CHAIN_0][8].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R17, &RFValue);
		rf_reg_record[CHAIN_0][9].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][9].Register = RF_R17;
		rf_reg_record[CHAIN_0][9].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R18, &RFValue);
		rf_reg_record[CHAIN_0][10].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][10].Register = RF_R18;
		rf_reg_record[CHAIN_0][10].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R19, &RFValue);
		rf_reg_record[CHAIN_0][11].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][11].Register = RF_R19;
		rf_reg_record[CHAIN_0][11].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK5, RF_R20, &RFValue);
		rf_reg_record[CHAIN_0][12].Bank = RF_BANK5;
		rf_reg_record[CHAIN_0][12].Register = RF_R20;
		rf_reg_record[CHAIN_0][12].Value = RFValue;

	}
	else if (chain == CHAIN_1)
	{
		/* save before config */
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R01, &RFValue);
		rf_reg_record[CHAIN_1][0].Bank = RF_BANK0;
		rf_reg_record[CHAIN_1][0].Register = RF_R01;
		rf_reg_record[CHAIN_1][0].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R02, &RFValue);
		rf_reg_record[CHAIN_1][1].Bank = RF_BANK0;
		rf_reg_record[CHAIN_1][1].Register = RF_R02;
		rf_reg_record[CHAIN_1][1].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R35, &RFValue);
		rf_reg_record[CHAIN_1][2].Bank = RF_BANK0;
		rf_reg_record[CHAIN_1][2].Register = RF_R35;
		rf_reg_record[CHAIN_1][2].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFValue);
		rf_reg_record[CHAIN_1][3].Bank = RF_BANK0;
		rf_reg_record[CHAIN_1][3].Register = RF_R42;
		rf_reg_record[CHAIN_1][3].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK6, RF_R00, &RFValue);
		rf_reg_record[CHAIN_1][4].Bank = RF_BANK6;
		rf_reg_record[CHAIN_1][4].Register = RF_R00;
		rf_reg_record[CHAIN_1][4].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK6, RF_R02, &RFValue);
		rf_reg_record[CHAIN_1][5].Bank = RF_BANK6;
		rf_reg_record[CHAIN_1][5].Register = RF_R02;
		rf_reg_record[CHAIN_1][5].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK6, RF_R34, &RFValue);
		rf_reg_record[CHAIN_1][6].Bank = RF_BANK6;
		rf_reg_record[CHAIN_1][6].Register = RF_R34;
		rf_reg_record[CHAIN_1][6].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R03, &RFValue);
		rf_reg_record[CHAIN_1][8].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][8].Register = RF_R03;
		rf_reg_record[CHAIN_1][8].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R04, &RFValue);
		rf_reg_record[CHAIN_1][7].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][7].Register = RF_R04;
		rf_reg_record[CHAIN_1][7].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R17, &RFValue);
		rf_reg_record[CHAIN_1][9].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][9].Register = RF_R17;
		rf_reg_record[CHAIN_1][9].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R18, &RFValue);
		rf_reg_record[CHAIN_1][10].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][10].Register = RF_R18;
		rf_reg_record[CHAIN_1][10].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R19, &RFValue);
		rf_reg_record[CHAIN_1][11].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][11].Register = RF_R19;
		rf_reg_record[CHAIN_1][11].Value = RFValue;
		RT635xReadRFRegister(pAd, RF_BANK7, RF_R20, &RFValue);
		rf_reg_record[CHAIN_1][12].Bank = RF_BANK7;
		rf_reg_record[CHAIN_1][12].Register = RF_R20;
		rf_reg_record[CHAIN_1][12].Value = RFValue;

	}
	else
	{
		DBGPRINT_ERR(("%s: Unknown chain = %u\n", __FUNCTION__, chain));
		return;
	}

	return;
}


static VOID RFConfigRecover(RTMP_ADAPTER *pAd, RF_REG_PAIR RF_record[][13])
{
	UCHAR chain_index = 0, record_index = 0;
	UCHAR bank = 0, rf_register = 0, value = 0;

	for (chain_index = 0; chain_index < 2; chain_index++)
	{
		for (record_index = 0; record_index < 13; record_index++)
		{
			bank = RF_record[chain_index][record_index].Bank;
			rf_register = RF_record[chain_index][record_index].Register;
			value = RF_record[chain_index][record_index].Value;			
			RT635xWriteRFRegister(pAd, bank, rf_register, value);
			DBGPRINT(RT_DEBUG_TRACE, ("bank: %d, rf_register: %d, value: %x\n",
				bank, rf_register, value));
		}
	}


	return;
}


static VOID SetBbpToneGenerator(
	IN RTMP_ADAPTER *pAd)
{
	UCHAR BBPValue = 0;

	/* choose FFT tone generator index (to 0) */
	BBPValue = 0xaa;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	/* set subcarrier index (subsample ratio to 10 * 0.3125MHz) */
	BBPValue = 0xab;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
	BBPValue = 0x0a;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	/* I/Q amplitude of tone gen0 */
	BBPValue = 0xac;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue); /* I signal */
	BBPValue = 0x3f;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	BBPValue = 0xad;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue); /* Q signal */
	BBPValue = 0x3f;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	/* enable tone gen */
	BBPValue = 0x40;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, BBPValue);

	return;
}


UINT32 Do_FFT_Accumulation(IN RTMP_ADAPTER *pAd, UCHAR tone_idx, UCHAR read_neg)
{
	UINT32 MacValue = 0;
	int fftout_i = 0, fftout_q = 0;
	UINT32 power_tmp=0, power_int = 0;
	UCHAR BBPValue = 0;
	UCHAR tone_idx_int;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x9b);

	BBPValue = 0x9b;

	while(BBPValue == 0x9b)
	{
		RtmpusecDelay(10); 
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue); 
		BBPValue = BBPValue &0xff;
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xba);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);

	RTMP_IO_READ32(pAd, 0x057C, &MacValue);

	fftout_i = (MacValue >> 16);
	fftout_i = (fftout_i & 0x8000) ? (fftout_i - 0x10000) : fftout_i;
	fftout_q = (MacValue & 0xffff);
	fftout_q = (fftout_q & 0x8000) ? (fftout_q - 0x10000) : fftout_q;
	power_tmp = (fftout_i * fftout_i);
	power_tmp = power_tmp + (fftout_q * fftout_q);
	power_int = power_tmp;
	DBGPRINT(RT_DEBUG_TRACE, ("I = %d,  Q = %d, power = %x\n", fftout_i, fftout_q, power_int));
	if(read_neg){
		power_int = power_int >> 1;
		tone_idx_int = 0x40 - tone_idx;
		tone_idx_int = tone_idx_int & 0x3f;

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xba);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx_int);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx_int);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx_int);
	
		RTMP_IO_READ32(pAd, 0x057C, &MacValue);
	
		fftout_i = (MacValue >> 16);
		fftout_i = (fftout_i & 0x8000) ? (fftout_i - 0x10000) : fftout_i;
		fftout_q = (MacValue & 0xffff);
		fftout_q = (fftout_q & 0x8000) ? (fftout_q - 0x10000) : fftout_q;
		power_tmp = (fftout_i * fftout_i);
		power_tmp = power_tmp + (fftout_q * fftout_q);
		power_tmp = power_tmp >> 1;
		power_int = power_int + power_tmp;
		
	}

	return power_int;
}

#ifdef RT6352_EP_SUPPORT
UINT32 Read_FFT_Accumulation(IN RTMP_ADAPTER *pAd, UCHAR tone_idx)
{
	UINT32 MacValue = 0;
	int fftout_i = 0, fftout_q = 0;
	UINT32 power_tmp=0, power_int = 0;
	UCHAR BBPValue = 0;
	UCHAR tone_idx_int;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xba);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, tone_idx);

	RTMP_IO_READ32(pAd, 0x057C, &MacValue);

	fftout_i = (MacValue >> 16);
	fftout_i = (fftout_i & 0x8000) ? (fftout_i - 0x10000) : fftout_i;
	fftout_q = (MacValue & 0xffff);
	fftout_q = (fftout_q & 0x8000) ? (fftout_q - 0x10000) : fftout_q;
	power_tmp = (fftout_i * fftout_i);
	power_tmp = power_tmp + (fftout_q * fftout_q);
	power_int = power_tmp;
	DBGPRINT(RT_DEBUG_TRACE, ("I = %d,  Q = %d, power = %x\n", fftout_i, fftout_q, power_int));

	return power_int;
}
#endif /* RT6352_EP_SUPPORT */

static VOID Write_DC(
	RTMP_ADAPTER *pAd,
	UCHAR chain_idx,
	UCHAR alc,
	UCHAR iorq,
	UCHAR dc)
{
	UCHAR BBPValue = 0;


	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb0);
	BBPValue = alc | 0x80;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	if (chain_idx == 0)
		BBPValue = (iorq == 0) ? 0xb1: 0xb2;
	else
		BBPValue = (iorq == 0) ? 0xb8: 0xb9;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
	BBPValue = dc;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	return;
}


static VOID LOFT_Search(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR chain_idx,
	IN UCHAR alc_idx,
	OUT UCHAR dc_result[][RF_ALC_NUM][2])	
{
	UINT32 pwr0 = 0, pwr1 = 0, pwr_final = 0;
	CHAR   index0 = 0,index1 = 0;
	UCHAR  index_final[] = {0x00, 0x00};
	UCHAR  inverted_bit = 0x20;
	UCHAR  iorq;
	CHAR   bit_index;

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x80);

	for (bit_index = 5; bit_index >= 0; bit_index--)
	{
		for (iorq = 0; iorq <= 1; iorq++)
		{
			DBGPRINT(RT_DEBUG_INFO, ("========================================================\n"));                 

			if (index_final[iorq] == 0x20)
			{
				index0 = 0x20;
				pwr0 = pwr_final;
			}
			else
			{
				index0 = index_final[iorq] - inverted_bit;
				index0 = index0 & 0x3f;
				Write_DC(pAd, chain_idx, 0, iorq, index0);
				pwr0 = Do_FFT_Accumulation(pAd,0xa,0);
			}

			index1 = index_final[iorq] + ((bit_index == 5) ? 0 : inverted_bit);
			index1 = index1 & 0x3f;
			Write_DC(pAd, chain_idx, 0, iorq, index1);
			pwr1 = Do_FFT_Accumulation(pAd, 0xa,0);

			DBGPRINT(RT_DEBUG_INFO, ("alc=%u, IorQ=%u, idx_final=%2x\n", alc_idx, iorq, index_final[iorq])); 
			DBGPRINT(RT_DEBUG_INFO, ("pwr0=%x, pwr1=%x, pwr_final=%x, idx_0=%x, idx_1=%x, inverted_bit=%x !\n"
					,pwr0, pwr1, pwr_final, index0, index1, inverted_bit));

			if ((bit_index != 5) && (pwr_final <= pwr0) && (pwr_final < pwr1))
			{
				pwr_final = pwr_final;
				index_final[iorq] = index_final[iorq];
			}
			else if (pwr0 < pwr1)
			{
				pwr_final = pwr0;
				index_final[iorq] = index0 & 0x3f;
			}
			else
			{
				pwr_final = pwr1;
				index_final[iorq] = index1 & 0x3f;
			}
			DBGPRINT(RT_DEBUG_INFO, ("IorQ=%u, idx_final[%u]:%x, pwr_final:%8x\n"
					, iorq, iorq, index_final[iorq], pwr_final));

			Write_DC(pAd, chain_idx, 0, iorq, index_final[iorq]);

		}
		inverted_bit = inverted_bit >> 1;
	}
	dc_result[chain_idx][alc_idx][0] = index_final[0];
	dc_result[chain_idx][alc_idx][1] = index_final[1];

	return;
}


static VOID IQ_Search(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR chain_idx,
	OUT UCHAR *gain_error_store,
	OUT UCHAR *phase_error_store)	
{
	UINT32 pwr0 = 0, pwr1 = 0, pwr_final = 0;
	CHAR   phase_err = 0, gain_err = 0, iq_err = 0;
	CHAR   phase_err_fine = 0, gain_err_fine = 0;
	CHAR   phase_st,phase_end;
	CHAR   gain_st,gain_end;

	UCHAR inverted_bit = 0x20;
	UCHAR first_search = 0x00, touch_neg_max = 0x00;
	UCHAR index0 = 0, index1 = 0;
	UCHAR gain_or_phase;
	UCHAR BBPValue = 0;
	CHAR bit_index;

	DBGPRINT(RT_DEBUG_ERROR, ("IQCalibration Start!\n"));
	for (bit_index = 5; bit_index >= 1; bit_index--)
	{
		for (gain_or_phase = 0; gain_or_phase < 2; gain_or_phase++)
		{
			DBGPRINT(RT_DEBUG_INFO, ("========================================================\n"));

			if ((gain_or_phase == 1) || (bit_index < 4))
			{
				if (gain_or_phase == 0)
					iq_err = gain_err;
				else
					iq_err = phase_err;

				first_search = (gain_or_phase == 0) ? (bit_index == 3) : (bit_index == 5);
				touch_neg_max = (gain_or_phase) ? ((iq_err & 0x0f) == 0x8) : ((iq_err & 0x3f) == 0x20);

				if (touch_neg_max)
				{
					pwr0 = pwr_final;
					index0 = iq_err;
				}
				else
				{
					index0 = iq_err - inverted_bit;
					BBPValue = (chain_idx == 0) ? 
							((gain_or_phase == 0) ? 0x28 : 0x29):
							((gain_or_phase == 0) ? 0x46 : 0x47);

					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
					RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, index0);

					pwr0 = Do_FFT_Accumulation(pAd,0x14,1);
				}

				index1 = iq_err + (first_search ? 0 : inverted_bit);
				index1 = (gain_or_phase == 0) ? (index1 & 0xf) : (index1 & 0x3f);

				BBPValue = (chain_idx == 0) ? 
						(gain_or_phase == 0) ? 0x28 : 0x29 :
						(gain_or_phase == 0) ? 0x46 : 0x47;

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, index1);

				pwr1 = Do_FFT_Accumulation(pAd,0x14,1);

				DBGPRINT(RT_DEBUG_INFO, ("pwr0=%x, pwr1=%x, pwer_final=%x, index0=%x, index1=%x, iq_err=%x, gain_or_phase=%d, inverted_bit=%x !\n",
                    pwr0, pwr1, pwr_final, index0, index1, iq_err, gain_or_phase, inverted_bit));

				if ((!first_search) && (pwr_final <= pwr0) && (pwr_final < pwr1))
				{
					pwr_final = pwr_final;
				}
				else if (pwr0 < pwr1)
				{
					pwr_final = pwr0;
					iq_err = index0;
				}
				else
				{
					pwr_final = pwr1;
					iq_err = index1;
				}
				
				BBPValue = (chain_idx == 0) ? 
					(gain_or_phase == 0) ? 0x28 : 0x29 :
					(gain_or_phase == 0) ? 0x46 : 0x47;

				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, iq_err);

				if (gain_or_phase == 0)
					gain_err = iq_err;
				else
					phase_err = iq_err;
					
				DBGPRINT(RT_DEBUG_INFO, ("IQCalibration pwr_final=%8x (%2x, %2x) !\n"
						, pwr_final, gain_err & 0xf, phase_err & 0x3f));

			} /* end if */
		} /* end gain_or_phase */

		if(bit_index > 0)
			inverted_bit = (inverted_bit >> 1);
	} 
	gain_err  = (gain_err  & 0x8 ) ? (gain_err  & 0xf ) - 0x10 : (gain_err & 0xf  );
	phase_err = (phase_err & 0x20) ? (phase_err & 0x3f) - 0x40 : (phase_err & 0x3f);
	
	gain_err = (gain_err < -0x7) ?  -0x7 :
		       (gain_err >  0x5) ?   0x5 :
			   	                     gain_err;
	gain_st  = gain_err - 1;
	gain_end = gain_err + 2;
	
	phase_err =(phase_err < -0x1f) ?  -0x1f :
		       (phase_err >  0x1d) ?    0x1d :
			   	                       phase_err;
	phase_st  = phase_err - 1;
	phase_end = phase_err + 2;

	for(gain_err_fine = gain_st; gain_err_fine <= gain_end; gain_err_fine = gain_err_fine + 1)
		for(phase_err_fine = phase_st; phase_err_fine <= phase_end; phase_err_fine = phase_err_fine + 1){
			BBPValue = (chain_idx == 0) ? 0x28 : 0x46;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, gain_err_fine & 0xf);
			
			BBPValue = (chain_idx == 0) ? 0x29 : 0x47;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, phase_err_fine & 0x3f);
			
			pwr1 = Do_FFT_Accumulation(pAd,0x14,1);
			if((gain_err_fine == gain_st) && (phase_err_fine == phase_st)){
				pwr_final = pwr1;
				gain_err  = gain_err_fine;
				phase_err = phase_err_fine;
			}
			else if(pwr_final > pwr1){
				pwr_final = pwr1;
				gain_err  = gain_err_fine;
				phase_err = phase_err_fine;
			}
			DBGPRINT(RT_DEBUG_INFO, ("Fine IQCalibration pwr1=%8x pwr_final=%8x (%2x, %2x) !\n",
						pwr1, pwr_final, gain_err_fine & 0xf, phase_err_fine & 0x3f));
		}
		
	gain_error_store[chain_idx] = gain_err & 0xf;
	phase_error_store[chain_idx] = phase_err & 0x3f;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IQCalibration Done! CH = %u, (gain=%2x, phase=%2x)\n"
			, chain_idx, gain_err & 0xf, phase_err & 0x3f));

	return;
}


static VOID RF_AUX_TX0_LOOPBACK(RTMP_ADAPTER *pAd)
{
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, 0x21);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, 0x10);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x00);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x1b);
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R00, 0x81);
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R02, 0x81);
	RT635xWriteRFRegister(pAd, RF_BANK4, RF_R34, 0xee);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, 0x2d);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x2d);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R17, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R18, 0xd7);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R19, 0xa2);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R20, 0x20);
}

static VOID RF_AUX_TX1_LOOPBACK(RTMP_ADAPTER *pAd)
{
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R01, 0x22);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R02, 0x20);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R35, 0x00);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x4b);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R00, 0x81);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R02, 0x81);
	RT635xWriteRFRegister(pAd, RF_BANK6, RF_R34, 0xee);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, 0x2d);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x2d);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R17, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R18, 0xd7);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R19, 0xa2);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R20, 0x20);
}

VOID LOFT_IQ_Calibration(RTMP_ADAPTER *pAd)
{
	RF_REG_PAIR RF_Store[CHAIN_NUM][13];
	UINT32 MacOrg1 = 0; /* TX_PIN_CFG */
	UINT32 MacOrg2 = 0; /* RF_CONTROL0 */
	UINT32 MacOrg3 = 0; /* RTMP_RF_BYPASS0 */
	UINT32 MacOrg4 = 0; /* RF_CONTROL3 */
	UINT32 MacOrg5 = 0; /* RF_BYPASS3 */
	UINT32 ORIG528 = 0;
	UINT32 ORIG52C = 0;

	UINT32 saveMacSysCtrl = 0,MTxCycle = 0;
	UINT32 MacValue = 0;
	UINT32 Mac13b8  = 0;
//	UINT32 table_dc_i0 = 0;
//	UINT32 table_dc_q0 = 0;
//	UINT32 table_dc_i1 = 0;
//	UINT32 table_dc_q1 = 0;
	UINT32 pwr0 = 0, pwr1 = 0;
#ifdef RT6352_EP_SUPPORT
	UINT32 pwr0_idx10= 0, pwr1_idx10 = 0;
#endif /* RT6352_EP_SUPPORT */

	UCHAR RFValue;
	UCHAR LOFT_DC_Search_Result[CHAIN_NUM][RF_ALC_NUM][2]; /* 0: I_PATH; 1: Q_PATH */
	UCHAR gain_error_result[CHAIN_NUM], phase_error_result[CHAIN_NUM];
	UCHAR RF_gain[] = {0x0, 0x1, 0x2, 0x4,0x8,0xc};
	UCHAR RFVGA_gain_table[]=
	  { 0x24, 0x25, 0x26, 0x27,   //[0:2:6]
            0x28, 0x2c, 0x2d, 0x2e,   //[8:2:14]
	    0x2f, 0x30, 0x31, 0x38,   //[16:2:22]
	    0x39, 0x3a, 0x3b, 0x3c,   //[24:2:30]
	    0x3d, 0x3e, 0x3f       }; //[32:2:36]
	
//    UCHAR VGA_gain[]     = {0x3b, 0x3b, 0x3b, 0x3b, 0x3b, 0x3bi;
	CHAR VGA_gain[]     = {14, 14};   
	UCHAR BBP_2324gain[] = {0x16, 0x14, 0x12, 0x10, 0xc, 0x8};
	UCHAR BBPValue = 0, chain_idx = 0, rf_alc_idx = 0, idx = 0;
	UCHAR BBPR30Value, RFB0_R39, RFB0_R42;
#ifdef RT6352_EP_SUPPORT
	UCHAR BBP_R1_Value;
	UCHAR BBP_R4_Value;
	UCHAR BBPR241, BBPR242;
#endif /* RT6352_EP_SUPPORT */
	UCHAR count_step;

	//RF self Tx DC calibration
	//RF_SELF_TXDC_CAL(pAd); 
	/* backup before MAC RF Interface config */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacOrg1);
	RTMP_IO_READ32(pAd, RF_CONTROL0, &MacOrg2);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &MacOrg3);
	RTMP_IO_READ32(pAd, RF_CONTROL3, &MacOrg4);
	RTMP_IO_READ32(pAd, RF_BYPASS3, &MacOrg5);
	RTMP_IO_READ32(pAd, 0x13b8 , &Mac13b8);
	RTMP_IO_READ32(pAd, RF_CONTROL2, &ORIG528);
	RTMP_IO_READ32(pAd, RF_BYPASS2,  &ORIG52C);

	/* MAC Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x04);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);

	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x1)
			RtmpusecDelay(50);
		else
			break;
	}

	/* MAC Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x08);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);

	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x2)
			RtmpusecDelay(50);
		else
			break;
	} 
	
	/* 
		step 1: manually turn on ADC8, ADC6, PA, Tx, Rx, 
		and bypass ALC control
	*/

	/* backup RF registers before config */
	for (chain_idx = 0; chain_idx < 2; chain_idx++)
	{
		RFConfigStore(pAd, RF_Store, chain_idx);
	}

	/* Backup ADC clcok selection */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPR30Value);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R39, &RFB0_R39);
	RT635xReadRFRegister(pAd, RF_BANK0, RF_R42, &RFB0_R42);

	/* change to Shielding clock */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, 0x1F);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x80);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x5B);

	/* step 7: set BBP tone generator */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);

	SetBbpToneGenerator(pAd);

	for( chain_idx = 0; chain_idx < 2; chain_idx ++) 
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x0000000f);
		RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000004);
		RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x00003306);
		RTMP_IO_WRITE32(pAd, 0x13b8 , 0x10);
		RtmpusecDelay(1);

		if (chain_idx == 0)
		{
			/* step 2: set RF loopback for ANT0 */
			RF_AUX_TX0_LOOPBACK(pAd);
		}
		else
		{
			/* step 4: set RF loopback for ANT1 */
			RF_AUX_TX1_LOOPBACK(pAd);
		}
		RtmpusecDelay(1);
		if(chain_idx == 0)
		{
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00001004);
		}
		else
		{
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00002004);
		}

		/* step 8: set accumulation length */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x05);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);
	
		/* step 9: set chain index */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x01);
		if(chain_idx == 0)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00); /* for pair 0 */
		else
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x01); /* for pair 0 */
	
		/* step 10: for loop of chain 0 with rf_alc_idx */
		VGA_gain[chain_idx] = 18;
		for (rf_alc_idx = 0; rf_alc_idx < 3; rf_alc_idx++)
		{
			/* step 13: manually set BBP digital power control */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, BBP_2324gain[rf_alc_idx]);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, BBP_2324gain[rf_alc_idx]);

			/* step 11: manually set RF ALC code */
			RTMP_IO_READ32(pAd, RF_CONTROL3, &MacValue); /* fix RF ALC */
			MacValue &= (~0x0000f1f1);
			MacValue |= (RF_gain[rf_alc_idx] << 4);
			MacValue |= (RF_gain[rf_alc_idx] << 12);
			RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacValue);
			MacValue = (0x0000f1f1);
			RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacValue);

			/* step 12: manually set RF VGA gain */
 			if(rf_alc_idx == 0) {				
				Write_DC(pAd, chain_idx, 0, 1, 0x21);
				for(;VGA_gain[chain_idx] > 0;VGA_gain[chain_idx] = VGA_gain[chain_idx] -2){ 
					RFValue = RFVGA_gain_table[VGA_gain[chain_idx]];
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
					RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, RFValue);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
					RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, RFValue);
					Write_DC(pAd, chain_idx, 0, 1, 0x0);
					Write_DC(pAd, chain_idx, 0, 0, 0x0);
					pwr0 = Do_FFT_Accumulation(pAd, 0xa,0);
					Write_DC(pAd, chain_idx, 0, 0, 0x21);
					pwr1 = Do_FFT_Accumulation(pAd, 0xa,0);
					DBGPRINT(RT_DEBUG_TRACE, ("LOFT AGC %d %d\n",pwr0,pwr1)); 
					if((pwr0 < 7000*7000) && (pwr1 < (7000*7000))){						
						break;
					}
				}
				/*  for alc = 0 calibration, clean the dc value */ 
				Write_DC(pAd, chain_idx, 0, 0, 0x0);
				Write_DC(pAd, chain_idx, 0, 1, 0x0);
				
				DBGPRINT(RT_DEBUG_TRACE, ("Used VGA %d %x\n",VGA_gain[chain_idx], RFVGA_gain_table[VGA_gain[chain_idx]])); 

				if(VGA_gain[chain_idx] < 0)
					VGA_gain[chain_idx] = 0;

		 	}			

			RFValue = RFVGA_gain_table[VGA_gain[chain_idx]];
			
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, RFValue);
	
			/* step 14, step 15, and step 16: search */
			LOFT_Search(pAd, chain_idx, rf_alc_idx, LOFT_DC_Search_Result);
		}
	
	}

	/* step 19: write back compensate value */
	for (rf_alc_idx = 0; rf_alc_idx < 3; rf_alc_idx++)
	{
//		DBGPRINT(RT_DEBUG_OFF, ("dc_result[0][%u][0]=%x, dc_result[0][%u][1]=%x\n"
//			, rf_alc_idx, LOFT_DC_Search_Result[0][rf_alc_idx][0], rf_alc_idx
//			, LOFT_DC_Search_Result[0][rf_alc_idx][1])); 
//		DBGPRINT(RT_DEBUG_OFF, ("dc_result[1][%u][0]=%x, dc_result[1][%u][1]=%x\n"
//			, rf_alc_idx, LOFT_DC_Search_Result[1][rf_alc_idx][0], rf_alc_idx
//			, LOFT_DC_Search_Result[1][rf_alc_idx][1])); 


		for (idx = 0; idx < 4; idx++)
		{
//			table_dc_i0 = LOFT_DC_Search_Result[0][(idx == 0) ? 0 : (idx+2)][0] - LOFT_DC_Search_Result[0][0][0];
//			table_dc_q0 = LOFT_DC_Search_Result[0][(idx == 0) ? 0 : (idx+2)][1] - LOFT_DC_Search_Result[0][0][1];
//			table_dc_i1 = LOFT_DC_Search_Result[1][(idx == 0) ? 0 : (idx+2)][0] - LOFT_DC_Search_Result[1][0][0];
//			table_dc_q1 = LOFT_DC_Search_Result[1][(idx == 0) ? 0 : (idx+2)][1] - LOFT_DC_Search_Result[1][0][1];
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb0);
			BBPValue = (idx<<2) + rf_alc_idx;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, (" ALC %2x,", BBPValue));

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb1);
			BBPValue = LOFT_DC_Search_Result[CHAIN_0][rf_alc_idx][I_PATH]; 
			BBPValue = BBPValue & 0x3f; 
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, (" I0 %2x,", BBPValue));

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb2);
			BBPValue = LOFT_DC_Search_Result[CHAIN_0][rf_alc_idx][Q_PATH]; 
			BBPValue = BBPValue & 0x3f; 
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, (" Q0 %2x,", BBPValue));

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb8);
			BBPValue = LOFT_DC_Search_Result[CHAIN_1][rf_alc_idx][I_PATH]; 
			BBPValue = BBPValue & 0x3f; 
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, (" I1 %2x,", BBPValue));

			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb9);
			BBPValue = LOFT_DC_Search_Result[CHAIN_1][rf_alc_idx][Q_PATH]; 
			BBPValue = BBPValue & 0x3f; 
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
			DBGPRINT(RT_DEBUG_TRACE, (" Q1 %2x\n", BBPValue));

		} 
	}

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);

	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x04);
	/* change BBP Tx to normal state */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);

	BBPValue = 0x00;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x00);
	/* BBP soft reset */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x01);
	RtmpusecDelay(1); /* wait 1 usec */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x00);

      /* recover RF registers */
	RFConfigRecover(pAd, RF_Store);

      /* recover MAC registers */
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacOrg1);
  	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x04);
  	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacOrg2);
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MacOrg3);
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacOrg4);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacOrg5);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
	RTMP_IO_WRITE32(pAd, RF_CONTROL2, ORIG528);
	RTMP_IO_WRITE32(pAd, RF_BYPASS2,  ORIG52C);
	RTMP_IO_WRITE32(pAd, 0x13b8 , Mac13b8);

	DBGPRINT(RT_DEBUG_ERROR, ("LOFT Calibration Done!\n"));

	/*************************************************************************/	
	/* start I/Q calibration */	

	/* backup before MAC RF Interface config */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);
	RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacOrg1);
	RTMP_IO_READ32(pAd, RF_CONTROL0, &MacOrg2);
	RTMP_IO_READ32(pAd, RTMP_RF_BYPASS0, &MacOrg3);
	RTMP_IO_READ32(pAd, RF_CONTROL3, &MacOrg4);
	RTMP_IO_READ32(pAd, RF_BYPASS3, &MacOrg5);

#ifdef RT6352_EP_SUPPORT
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBP_R1_Value);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBP_R4_Value);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R241, &BBPR241);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R242, &BBPR242);
#endif /* RT6352_EP_SUPPORT */
	RTMP_IO_READ32(pAd, 0x13b8 , &Mac13b8);

	/* MAC Tx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x04);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);
	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x1)
			RtmpusecDelay(50);
		else
			break;
	}

	/* MAC Rx */
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacValue);
	MacValue &= (~0x08);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacValue);
	for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacValue);
		if (MacValue & 0x2)
			RtmpusecDelay(50);
		else
			break;
	}

	/* 
		step 1: manually turn on ADC8, ADC6, PA, Tx, Rx, 
		and bypass ALC control
	*/
#ifdef RT6352_EP_SUPPORT
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, 0x00000101);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3 , 0x0000f1f1);
#endif /* RT6352_EP_SUPPORT */

	/* manually set RF ALC code */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
#ifdef RT6352_EP_SUPPORT
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBP_R4_Value & (~0x18));
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x1);
		RtmpusecDelay(1); /* wait 1 usec */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x0);
	
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, 0x14);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, 0x80);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x31);
#else
	SetBbpToneGenerator(pAd);
#endif /* RT6352_EP_SUPPORT */

	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00000004);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, 0x00003306);
	RtmpusecDelay(1);
	
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG	, 0x0000000f);
#ifndef RT6352_EP_SUPPORT
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, 0x00000000);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3	, 0x0000f1f1);
#endif /* RT6352_EP_SUPPORT */
	RTMP_IO_WRITE32(pAd, 0x13b8, 0x00000010);

	/* backup before RF registers before config */
	for (chain_idx = 0; chain_idx < 2; chain_idx++)
	{
		RFConfigStore(pAd, RF_Store, chain_idx);
	}

	/* step 3: set BBP tone generator */

	/* step 6: manually set RF VGA gain */
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, 0x3b);
	RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, 0x3b);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, 0x3b);
	RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, 0x3b);


    /* step 9: enable Amp/Phase IQ compensation & LOFT compensation */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x03);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x60);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x80);

	for (chain_idx = 0; chain_idx < 2; chain_idx ++)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);

		if (chain_idx == 0)
		{
			/* step 2: set RF loopback for ANT0 */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x01);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00); /* for pair 0 */
#ifdef RT6352_EP_SUPPORT
			// BBP only Tx0
			BBPValue = BBP_R1_Value & (~ 0x18);
			BBPValue = BBPValue | 0x00;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);
#endif /* RT6352_EP_SUPPORT */
			RF_AUX_TX0_LOOPBACK(pAd);
		}
		else
		{
			/* step 2: set RF loopback for ANT1 */
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x01);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x01); /* for pair 0 */
#ifdef RT6352_EP_SUPPORT
			// BBP only Tx1
			BBPValue = BBP_R1_Value & (~ 0x18);
			BBPValue = BBPValue | 0x08;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBPValue);
#endif /* RT6352_EP_SUPPORT */
			RF_AUX_TX1_LOOPBACK(pAd);
		}

		if(chain_idx == 0)
		{
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00001004);
		}
		else
		{
			RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x00002004);
		}

		/* step 8: set calibration length */
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x05);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x04);

		/* step 10: search for chain 0 */
		// clear gain value
		BBPValue = (chain_idx == 0) ? 0x28 : 0x46;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x0);

#ifdef RT6352_EP_SUPPORT
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x6);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x6);
#else
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x1F);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x1F);
#endif /* RT6352_EP_SUPPORT */

#ifdef RT6352_EP_SUPPORT
		count_step = 1;
#else
		count_step = 2;
#endif /* RT6352_EP_SUPPORT */

		for(;VGA_gain[chain_idx] < 19; VGA_gain[chain_idx]=(VGA_gain[chain_idx]+ count_step)){
			RFValue = RFVGA_gain_table[VGA_gain[chain_idx]];
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R03, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK5, RF_R04, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R03, RFValue);
			RT635xWriteRFRegister(pAd, RF_BANK7, RF_R04, RFValue);
			
			BBPValue = (chain_idx == 0) ? 0x29 : 0x47;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x0);
			pwr0 = Do_FFT_Accumulation(pAd,0x14,0);
#ifdef RT6352_EP_SUPPORT
			pwr0_idx10 = Read_FFT_Accumulation(pAd,0xa);
#endif /* RT6352_EP_SUPPORT */
			BBPValue = (chain_idx == 0) ? 0x29 : 0x47;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x21);
			pwr1 = Do_FFT_Accumulation(pAd,0x14,0);
#ifdef RT6352_EP_SUPPORT
			pwr1_idx10 = Read_FFT_Accumulation(pAd,0xa);
#endif /* RT6352_EP_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("IQ AGC %d %d\n",pwr0,pwr1));
#ifdef RT6352_EP_SUPPORT
			DBGPRINT(RT_DEBUG_TRACE, ("IQ AGC IDX 10 %d %d\n",pwr0_idx10,pwr1_idx10));
			if((pwr0_idx10 > 7000*7000) ||(pwr1_idx10 > 7000*7000)){
				if(VGA_gain[chain_idx]!=0)
					VGA_gain[chain_idx] = VGA_gain[chain_idx]-1;
				break;
			}
#endif /* RT6352_EP_SUPPORT */

			if((pwr0 >2500*2500) || (pwr1 > 2500*2500)){				
				break;
			}
		}	

		if (VGA_gain[chain_idx] > 18)
			VGA_gain[chain_idx] = 18;
		DBGPRINT(RT_DEBUG_TRACE, ("Used VGA %d %x\n",VGA_gain[chain_idx], RFVGA_gain_table[VGA_gain[chain_idx]])); 

		BBPValue = (chain_idx == 0) ? 0x29 : 0x47;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x0);


		IQ_Search(pAd, chain_idx, gain_error_result, phase_error_result);
	}	

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R23, 0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R24, 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x04);

	/* step 19: write back compensate value */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x28);
	BBPValue = gain_error_result[CHAIN_0] & 0x0f;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x29);
	BBPValue = phase_error_result[CHAIN_0] & 0x3f; 
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x46);
	BBPValue = gain_error_result[CHAIN_1] & 0x0f; 
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x47);
	BBPValue = phase_error_result[CHAIN_1] & 0x3f; 
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue); 
 
#ifdef RT6352_EP_SUPPORT
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BBP_R1_Value);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R241, BBPR241);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R242, BBPR242);
#endif /* RT6352_EP_SUPPORT */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R244, 0x00);

	/* change BBP Tx to normal state */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, 0xb0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, 0x00);

	/* restore ADC clcok selection */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPR30Value);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, RFB0_R39);
	RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, RFB0_R42);

#ifdef RT6352_EP_SUPPORT
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBP_R4_Value);
#endif /* RT6352_EP_SUPPORT */

	/* BBP soft reset */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x01);
	RtmpusecDelay(1); /* wait 1 usec */
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, 0x00);

      /* recover RF registers */
	RFConfigRecover(pAd, RF_Store);

      /* recover MAC registers */
	RTMP_IO_WRITE32(pAd, TX_PIN_CFG, MacOrg1);
  	RTMP_IO_WRITE32(pAd, RF_CONTROL0, 0x0);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0 , 0x0);
	RTMP_IO_WRITE32(pAd, RF_CONTROL0, MacOrg2);
	RtmpusecDelay(1);
	RTMP_IO_WRITE32(pAd, RTMP_RF_BYPASS0, MacOrg3);
	RTMP_IO_WRITE32(pAd, RF_CONTROL3, MacOrg4);
	RTMP_IO_WRITE32(pAd, RF_BYPASS3, MacOrg5);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
	RTMP_IO_WRITE32(pAd, 0x13b8 , Mac13b8);

	DBGPRINT(RT_DEBUG_ERROR, ("TX IQ Calibration Done!\n"));

	return;
}
#endif /* RT6352 */

