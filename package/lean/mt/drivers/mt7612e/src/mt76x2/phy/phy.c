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


INT phy_probe(RTMP_ADAPTER *pAd)
{

	return TRUE;
}


NDIS_STATUS NICInitBBP(RTMP_ADAPTER *pAd)
{
	UINT32 Index = 0, val;
	
	/* Before program BBP, we need to wait BBP/RF get wake up.*/
	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;

		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &val);
		if ((val & 0x03) == 0)	/* if BB.RF is stable*/
			break;
		
		DBGPRINT(RT_DEBUG_TRACE, ("Check if MAC_STATUS_CFG is busy(=%x)\n", val));
		RtmpusecDelay(1000);
	} while (Index++ < 100);
	
	if (pAd->phy_op && pAd->phy_op->bbp_init)
		return pAd->phy_op->bbp_init(pAd);
	else
		return NDIS_STATUS_FAILURE;
}


INT bbp_get_temp(struct _RTMP_ADAPTER *pAd, CHAR *temp_val)
{
	if (pAd->phy_op && pAd->phy_op->bbp_get_temp)
		return pAd->phy_op->bbp_get_temp(pAd, temp_val);
	else
		return FALSE;
}


INT bbp_tx_comp_init(RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode)
{
	if (pAd->phy_op && pAd->phy_op->bbp_tx_comp_init)
		return pAd->phy_op->bbp_tx_comp_init(pAd, adc_insel, tssi_mode);
	else
		return FALSE;
}


INT bbp_set_txdac(struct _RTMP_ADAPTER *pAd, INT tx_dac)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_txdac)
		return pAd->phy_op->bbp_set_txdac(pAd, tx_dac);
	else
		return FALSE;
}


INT bbp_set_rxpath(struct _RTMP_ADAPTER *pAd, INT rxpath)
{

//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, rxpath, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): rxpath=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, rxpath, agc));

	if (pAd->phy_op && pAd->phy_op->bbp_set_rxpath)
		return pAd->phy_op->bbp_set_rxpath(pAd, rxpath);
	else
		return FALSE;
}


INT bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 ext_ch)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_ctrlch)
		return pAd->phy_op->bbp_set_ctrlch(pAd, ext_ch);
	else
		return FALSE;
}


INT bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw)
{
	INT result = FALSE;

	if (pAd->phy_op && pAd->phy_op->bbp_set_bw)
		result = pAd->phy_op->bbp_set_bw(pAd, bw);

	if (result == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Set PhyBW as %sHz.l\n",
				__FUNCTION__, get_bw_str(bw)));
	}

	return result;
}


INT bbp_set_mmps(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_mmps)
		return pAd->phy_op->bbp_set_mmps(pAd, ReduceCorePower);
	else
		return FALSE;
}


INT bbp_get_agc(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX chain)
{
	if (pAd->phy_op && pAd->phy_op->bbp_get_agc)
		return pAd->phy_op->bbp_get_agc(pAd, agc, chain);
	else
		return FALSE;
}


INT bbp_set_agc(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX chain)
{
	if (pAd->phy_op && pAd->phy_op->bbp_set_agc)
		return pAd->phy_op->bbp_set_agc(pAd, agc, chain);
	else
		return FALSE;
}


INT filter_coefficient_ctrl(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	if (pAd->phy_op && pAd->phy_op->filter_coefficient_ctrl)
		return pAd->phy_op->filter_coefficient_ctrl(pAd, Channel);
	else
		return FALSE;
}


UCHAR get_random_seed_by_phy(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->get_random_seed_by_phy)
		return pAd->phy_op->get_random_seed_by_phy(pAd);
	else
		return 0;
}


INT bbp_is_ready(struct _RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->bbp_is_ready)
		return pAd->phy_op->bbp_is_ready(pAd);
	else
		return FALSE;
}

