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
        Who             When                    What
        --------        ----------              ----------------------------------------------
*/


#include "rt_config.h"


#ifdef RLT_BBP

static INT rlt_bbp_is_ready(struct _RTMP_ADAPTER *pAd)
{
	INT idx = 0;
	UINT32 val;
	
	do 
	{
		RTMP_BBP_IO_READ32(pAd, CORE_R0, &val);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return FALSE;
	} while ((++idx < 20) && ((val == 0xffffffff) || (val == 0x0)));

	if (!((val == 0xffffffff) || (val == 0x0)))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", val));
	}

	return (((val == 0xffffffff) || (val == 0x0)) ? FALSE : TRUE);
}


static INT rlt_bbp_init(RTMP_ADAPTER *pAd)
{
	INT idx;
	
	/* Read BBP register, make sure BBP is up and running before write new data*/
	if (rlt_bbp_is_ready(pAd) == FALSE)
		return NDIS_STATUS_FAILURE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Init BBP Registers\n", __FUNCTION__));

	/* re-config specific BBP registers for individual chip */
	if (pAd->chipCap.pBBPRegTable)
	{
		RTMP_REG_PAIR *reg = (RTMP_REG_PAIR *)pAd->chipCap.pBBPRegTable;
		
		for (idx = 0; idx < pAd->chipCap.bbpRegTbSize; idx++)
		{
			RTMP_BBP_IO_WRITE32(pAd, reg[idx].Register, reg[idx].Value);
			DBGPRINT(RT_DEBUG_INFO, ("BBP[%x]=0x%x\n", 
					reg[idx].Register, reg[idx].Value));
		}
	}

	if (pAd->chipOps.AsicBbpInit != NULL)
		pAd->chipOps.AsicBbpInit(pAd);

	// TODO: shiang-6590, check these bbp registers if need to remap to new BBP_Registers

	return NDIS_STATUS_SUCCESS;
	
}


static INT rlt_bbp_get_temp(struct _RTMP_ADAPTER *pAd, CHAR *temp_val)
{
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
	UINT32 bbp_val;

	RTMP_BBP_IO_READ32(pAd, CORE_R35, &bbp_val);
	*temp_val = (CHAR)(bbp_val & 0xff);

	pAd->curr_temp = (bbp_val & 0xff);
#endif
	return TRUE;
}


static INT rlt_bbp_tx_comp_init(RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode)
{
	UINT32 bbp_val;
	UCHAR rf_val;

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) 
	RTMP_BBP_IO_READ32(pAd, CORE_R34, &bbp_val);
	bbp_val = (bbp_val & 0xe7);
	bbp_val = (bbp_val | 0x80);
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, bbp_val);
	
	RT30xxReadRFRegister(pAd, RF_R27, &rf_val);
	rf_val = ((rf_val & 0x3f) | 0x40);
	RT30xxWriteRFRegister(pAd, RF_R27, rf_val);

	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Set RF_R27 to 0x%x\n", rf_val));
#endif
	return 0;
}


static INT rlt_bbp_set_txdac(struct _RTMP_ADAPTER *pAd, INT tx_dac)
{
	UINT32 txbe, txbe_r5 = 0;
	
	RTMP_BBP_IO_READ32(pAd, TXBE_R5, &txbe_r5);
	txbe = txbe_r5 & (~0x3);
	switch (tx_dac)
	{
		case 2:
			txbe |= 0x3;
			break;
		case 1:
		case 0:
		default:		
			txbe &= (~0x3);
			break;
	}

	if (txbe != txbe_r5)
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R5, txbe);

	return TRUE;
}


static INT rlt_bbp_set_rxpath(struct _RTMP_ADAPTER *pAd, INT rxpath)
{
	UINT32 agc, agc_r0 = 0;

	RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
	agc = agc_r0 & (~0x18);
	if(rxpath == 2)
		agc |= (0x8);
	else if(rxpath == 1)
		agc |= (0x0);


	if (agc != agc_r0)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);

//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, rxpath, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, rxpath, agc));

	return TRUE;
}

	
static INT rlt_bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 ext_ch)
{
	UINT32 agc, agc_r0 = 0;
	UINT32 be, be_r0 = 0;

	RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
	agc = agc_r0 & (~0x300);
	RTMP_BBP_IO_READ32(pAd, TXBE_R0, &be_r0);
	be = (be_r0 & (~0x03));
#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80 && 
		pAd->CommonCfg.Channel >= 36 &&
		pAd->CommonCfg.vht_cent_ch)
	{
		if (pAd->CommonCfg.Channel < pAd->CommonCfg.vht_cent_ch)
		{
			switch (pAd->CommonCfg.vht_cent_ch - pAd->CommonCfg.Channel)
			{
				case 6:
					be |= 0;
					agc |=0x000;
					break;
				case 2:
					be |= 1;
					agc |=0x100;
					break;

			}
		}
		else if (pAd->CommonCfg.Channel > pAd->CommonCfg.vht_cent_ch)
		{
			switch (pAd->CommonCfg.Channel - pAd->CommonCfg.vht_cent_ch)
			{
				case 6:
					be |= 0x3;
					agc |=0x300;
					break;
				case 2:
					be |= 0x2;
					agc |=0x200;
					break;
			}
		}
	}
	else
#endif /* DOT11_VHT_AC */
	{
		switch (ext_ch)
		{
			case EXTCHA_BELOW:
				agc |= 0x100;
				be |= 0x01;
				break;
			case EXTCHA_ABOVE:
				agc &= (~0x300);
				be &= (~0x03);
				break;
			case EXTCHA_NONE:
			default:
				agc &= (~0x300);
				be &= (~0x03);
				break;
		}
	}
	if (agc != agc_r0)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);

	if (be != be_r0)
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R0, be);


//DBGPRINT(RT_DEBUG_OFF, ("%s(): ext_ch=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, ext_ch, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): ext_ch=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, ext_ch, agc));

	return TRUE;
}


/*
	<<Gamma2.1 Control Registers Rev1.3.pdf>>
	BBP bandwidth (CORE_R1[4:3]) change procedure:
	1. Hold BBP in reset by setting CORE_R4[0] to '1'
	2. Wait 0.5 us to ensure BBP is in the idle State
	3. Change BBP bandwidth with CORE_R1[4:3]
		CORE_R1 (Bit4:3)
		0: 20MHz
		1: 10MHz (11J)
		2: 40MHz
		3: 80MHz
	4. Wait 0.5 us for BBP clocks to settle
	5. Release BBP from reset by clearing CORE_R4[0]
*/
static INT rlt_bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw)
{
	UINT32 core, core_r1 = 0;
	UINT32 agc, agc_r0 = 0;



	RTMP_BBP_IO_READ32(pAd, CORE_R1, &core_r1);
		core = (core_r1 & (~0x18));
	RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
	agc = agc_r0 & (~0x7000);
	switch (bw)
	{
		case BW_80:
			core |= 0x18;
			agc |= 0x7000;
			break;
		case BW_40:
			core |= 0x10;
			agc |= 0x3000;
			break;
		case BW_20:
			core &= (~0x18);
			agc |= 0x1000;
			break;
		case BW_10:
			core |= 0x08;
			agc |= 0x1000;
			break;
	}


	if (core != core_r1) {
		RTMP_BBP_IO_WRITE32(pAd, CORE_R1, core);
	}

	if (agc != agc_r0) {
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, bw, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, bw, agc));
	}

	pAd->CommonCfg.BBPCurrentBW = bw;


	return TRUE;
}


static INT rlt_bbp_set_mmps(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower)
{
	UINT32 bbp_val, org_val;

	RTMP_BBP_IO_READ32(pAd, AGC1_R0, &org_val);
	bbp_val = org_val;
	if (ReduceCorePower)
		bbp_val |= 0x04;
	else
		bbp_val &= ~0x04;

	if (bbp_val != org_val)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, bbp_val);

	return TRUE;
}


static INT rlt_bbp_get_agc(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX chain)
{
	UCHAR idx, val;
	UINT32 bbp_val, bbp_reg = AGC1_R8;


	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		chain = RX_CHAIN_0;
	}

	idx = val = 0;
	while(chain != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (chain & 0x01)
		{
			RTMP_BBP_IO_READ32(pAd, bbp_reg, &bbp_val);
			val = ((bbp_val & (0x0000ff00)) >> 8) & 0xff;
			break;
		}
		chain >>= 1;
		bbp_reg += 4;
		idx++;
	}

	*agc = val;

	return NDIS_STATUS_SUCCESS;
}


static INT rlt_bbp_set_agc(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX chain)
{
	UCHAR idx = 0;
	UINT32 bbp_val, bbp_reg = AGC1_R8;
	
	if (((pAd->MACVersion & 0xf0000000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		chain = RX_CHAIN_0;
	}

	while (chain != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;
		
		if (idx & 0x01)
		{
			RTMP_BBP_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val = (bbp_val & 0xffff00ff) | (agc << 8);
			RTMP_BBP_IO_WRITE32(pAd, bbp_reg, bbp_val);

			DBGPRINT(RT_DEBUG_INFO, 
					("%s(Idx):Write(R%d,val:0x%x) to Chain(0x%x, idx:%d)\n",
						__FUNCTION__, bbp_reg, bbp_val, chain, idx));
		}
		chain >>= 1;
		bbp_reg += 4;
		idx++;
	}

	return TRUE;
}


static INT rlt_bbp_set_filter_coefficient_ctrl(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	UINT32 bbp_val = 0, org_val = 0;

	if (Channel == 14)
	{
		/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
		RTMP_BBP_IO_READ32(pAd, CORE_R1, &org_val);
		bbp_val = org_val;
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
			bbp_val |= 0x20;
		else
			bbp_val &= (~0x20);

		if (bbp_val != org_val)
			RTMP_BBP_IO_WRITE32(pAd, CORE_R1, bbp_val);
	}

	return TRUE;
}


static UCHAR rlt_bbp_get_random_seed(RTMP_ADAPTER *pAd)
{
	UINT32 value, value2;
	UCHAR seed;

	RTMP_BBP_IO_READ32(pAd, AGC1_R16, &value);
	seed = (UCHAR)((value & 0xff) ^ ((value & 0xff00) >> 8)^ 
					((value & 0xff0000) >> 16));
	RTMP_BBP_IO_READ32(pAd, RXO_R9, &value2);

	return (UCHAR)(seed ^ (value2 & 0xff)^ ((value2 & 0xff00) >> 8));
}


#ifdef DYNAMIC_VGA_SUPPORT
INT rlt_dynamic_vga_enable(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT rlt_dynamic_vga_disable(RTMP_ADAPTER *pAd)
{
	UINT32 bbp_val;
	/* Just restore InitGain */
	RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val);
	bbp_val = (bbp_val & 0xffff80ff) | (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 << 8);
	RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val);

	if (pAd->CommonCfg.RxStream >= 2) {
		RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val);
		bbp_val = (bbp_val & 0xffff80ff) | (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1 << 8);
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val);
	}
	
	return TRUE;
}

INT rlt_dynamic_vga_adjust(RTMP_ADAPTER *pAd)
{
	if ((pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable) && (pAd->chipCap.dynamic_vga_support) &&
		OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		UCHAR val1, val2;
		UINT32 bbp_val1, bbp_val2;

		RTMP_BBP_IO_READ32(pAd, AGC1_R8, &bbp_val1);
		val1 = ((bbp_val1 & (0x00007f00)) >> 8) & 0x7f;
		RTMP_BBP_IO_READ32(pAd, AGC1_R9, &bbp_val2);
		val2 = ((bbp_val2 & (0x00007f00)) >> 8) & 0x7f;

		DBGPRINT(RT_DEBUG_INFO, ("vga_init_0 = %x, vga_init_1 = %x\n",  pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0, pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1));
		DBGPRINT(RT_DEBUG_INFO,
			("one second False CCA=%d, fixed agc_vga_0:0%x, fixed agc_vga_1:0%x\n", pAd->RalinkCounters.OneSecFalseCCACnt, val1, val2));

		if (pAd->RalinkCounters.OneSecFalseCCACnt > pAd->CommonCfg.lna_vga_ctl.nFalseCCATh) {
			if (val1 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0 - 0x10)) {
				val1 -= 0x04;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
#ifdef DFS_SUPPORT
       			pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->CommonCfg.RxStream >= 2) {
				if (val2 > (pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1 - 0x10)) {
					val2 -= 0x04;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		} else if (pAd->RalinkCounters.OneSecFalseCCACnt < 
					pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh) {
			if (val1 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_0) {
				val1 += 0x04;
				bbp_val1 = (bbp_val1 & 0xffff80ff) | (val1 << 8);
				RTMP_BBP_IO_WRITE32(pAd, AGC1_R8, bbp_val1);
#ifdef DFS_SUPPORT
				pAd->CommonCfg.RadarDetect.bAdjustDfsAgc = TRUE;
#endif
			}

			if (pAd->CommonCfg.RxStream >= 2) {
				if (val2 < pAd->CommonCfg.lna_vga_ctl.agc_vga_init_1) {
					val2 += 0x04;
					bbp_val2 = (bbp_val2 & 0xffff80ff) | (val2 << 8);
					RTMP_BBP_IO_WRITE32(pAd, AGC1_R9, bbp_val2);
				}
			}
		}
	}

	return TRUE;
}
#endif /* DYNAMIC_VGA_SUPPORT */


static struct phy_ops rlt_phy_ops = {
	.bbp_init = rlt_bbp_init,
	.bbp_is_ready = rlt_bbp_is_ready,
	.get_random_seed_by_phy = rlt_bbp_get_random_seed,
	.filter_coefficient_ctrl = rlt_bbp_set_filter_coefficient_ctrl,
	.bbp_set_agc = rlt_bbp_set_agc,
	.bbp_get_agc = rlt_bbp_get_agc,
	.bbp_set_bw = rlt_bbp_set_bw,
	.bbp_set_ctrlch = rlt_bbp_set_ctrlch,
	.bbp_set_rxpath = rlt_bbp_set_rxpath,
	.bbp_set_txdac = rlt_bbp_set_txdac,
	.bbp_set_mmps = rlt_bbp_set_mmps,
	.bbp_tx_comp_init = rlt_bbp_tx_comp_init,
	.bbp_get_temp = rlt_bbp_get_temp,
#ifdef DYNAMIC_VGA_SUPPORT
	.dynamic_vga_enable = rlt_dynamic_vga_enable,
	.dynamic_vga_disable = rlt_dynamic_vga_disable,
	.dynamic_vga_adjust = rlt_dynamic_vga_adjust,
#endif /* DYNAMIC_VGA_SUPPORT */
};


INT rlt_phy_probe(RTMP_ADAPTER *pAd)
{
	pAd->phy_op = &rlt_phy_ops;

	return TRUE;
}

#endif /* RLT_BBP */

