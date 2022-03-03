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
	who		When			What
	--------	----------		----------------------------------------------
*/
#ifdef COMPOS_WIN
#include "MTConfig.h"
#if defined(EVENT_TRACING)
#include "phy.tmh"
#endif
#else
#include "rt_config.h"
#endif

INT phy_probe(RTMP_ADAPTER *pAd)
{

	return TRUE;
}


NDIS_STATUS NICInitBBP(RTMP_ADAPTER *pAd)
{
	/* Before program BBP, we need to wait BBP/RF get wake up.*/
#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (MtAsicWaitMacTxRxIdle(pAd) == FALSE)
			return NDIS_STATUS_FAILURE;
	}
#endif

	if (pAd->phy_op && pAd->phy_op->bbp_init)
		return pAd->phy_op->bbp_init(pAd);
	else
		return NDIS_STATUS_FAILURE;
}


VOID AsicBBPAdjust(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* TODO: shiang-6590, now this function only used for AP mode, why we need this differentation? */
	if (ops->ChipBBPAdjust != NULL)
		ops->ChipBBPAdjust(pAd, Channel);
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


INT bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw, UCHAR BandIdx)
{
	INT result = FALSE;

	if (pAd->phy_op && pAd->phy_op->bbp_set_bw)
		result = pAd->phy_op->bbp_set_bw(pAd, bw);

	if (result == TRUE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set PhyBW as %sHz.l\n",
				__func__, get_bw_str(bw)));
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


INT phy_rrm_adjust(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


INT phy_rrm_request(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


INT phy_rrm_init(struct _RTMP_ADAPTER *pAd)
{

	return TRUE;
}


#ifdef DYNAMIC_VGA_SUPPORT
INT dynamic_vga_enable(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_enable)
		return pAd->phy_op->dynamic_vga_enable(pAd);
	else
		return FALSE;
}


INT dynamic_vga_disable(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_disable)
		return pAd->phy_op->dynamic_vga_disable(pAd);
	else
		return FALSE;
}


INT dynamic_vga_adjust(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->dynamic_vga_adjust)
		return pAd->phy_op->dynamic_vga_adjust(pAd);
	else
		return FALSE;
}
#endif /* DYNAMIC_VGA_SUPPORT */


INT32 ShowPartailBBP(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	if (pAd->phy_op && pAd->phy_op->ShowPartialBBP)
		return pAd->phy_op->ShowPartialBBP(pAd, Start, End);
	else
		return FALSE;
}

INT32 ShowAllBBP(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->ShowAllBBP)
		return pAd->phy_op->ShowAllBBP(pAd);
	else
		return FALSE;
}

INT32 ShowAllRF(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->ShowAllRF)
		return pAd->phy_op->ShowAllRF(pAd);
	else
		return FALSE;
}


INT32 ShowPartialRF(RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End)
{
	if (pAd->phy_op && pAd->phy_op->ShowPartialRF)
		return pAd->phy_op->ShowPartialRF(pAd, Start, End);
	else
		return FALSE;
}

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Smart_Carrier_Sense(RTMP_ADAPTER *pAd)
{
	if (pAd->phy_op && pAd->phy_op->Smart_Carrier_Sense)
		return pAd->phy_op->Smart_Carrier_Sense(pAd);
	else
		return FALSE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

