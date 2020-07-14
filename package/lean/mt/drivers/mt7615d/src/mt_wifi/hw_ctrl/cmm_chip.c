/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_chip.c
*/

#include "rt_config.h"
#include "hdev/hdev.h"

#ifdef TXBF_SUPPORT
VOID chip_tx_bf_init(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->TxBFInit)
		ops->TxBFInit(ad, pEntry, ie_list, supportsETxBF);
}
#endif /*TXBF_SUPPORT*/

#ifdef RF_LOCKDOWN
BOOLEAN chip_check_rf_lock_down(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->check_RF_lock_down)
		return ops->check_RF_lock_down(ad);
	return FALSE;
}
#endif /*RF_LOCKDOWN*/

INT32 chip_cmd_tx(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->MtCmdTx)
		return ops->MtCmdTx(ad, msg);
	return 0;
}

#ifdef WIFI_SPECTRUM_SUPPORT
VOID chip_spectrum_start(struct _RTMP_ADAPTER *ad, UINT8 *info)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->SpectrumStart != NULL)
		ops->SpectrumStart(ad, info);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s : The function is not hooked !!\n", __func__));
	}
}
#endif /*WIFI_SPECTRUM_SUPPORT*/

BOOLEAN chip_eeprom_read16(struct _RTMP_ADAPTER *ad, USHORT offset, USHORT *value)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread)
		return ops->eeread(ad, offset, value);
	else
		return FALSE;
}

BOOLEAN chip_eeprom_read_with_range(struct _RTMP_ADAPTER *ad, USHORT start, USHORT length, UCHAR *pbuf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->eeread_range)
		return ops->eeread_range(ad, start, length, pbuf);
	else
		return FALSE;
}

VOID chip_fw_init(struct _RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->FwInit && (IS_HIF_TYPE(ad, HIF_MT)))
		ops->FwInit(ad);
}

VOID chip_parse_rxv_packet(struct _RTMP_ADAPTER *ad, UINT32 Type, struct _RX_BLK *RxBlk, UCHAR *Data)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->parse_RXV_packet)
		ops->parse_RXV_packet(ad, Type, RxBlk, Data);
}

INT32 chip_txs_handler(RTMP_ADAPTER *ad, RX_BLK *rx_blk, VOID *rx_packet)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->txs_handler)
		ops->txs_handler(ad, rx_blk, rx_packet);

	return FALSE;
}

VOID chip_rx_event_handler(struct _RTMP_ADAPTER *ad, VOID *rx_packet)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->rx_event_handler != NULL) {
#ifdef UNIFY_FW_CMD
		ops->rx_event_handler(ad, rx_packet + sizeof(RXD_BASE_STRUCT));
#else
		ops->rx_event_handler(ad, rx_packet);
#endif /* UNIFY_FW_CMD */
	}
}

INT chip_show_pwr_info(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->show_pwr_info)
		ops->show_pwr_info(pAd);

	return 0;
}

VOID chip_arch_set_aid(struct _RTMP_ADAPTER *ad, USHORT aid)
{
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
		struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);
		if (ops->archSetAid)
			ops->archSetAid(ad, aid);
#endif
}

VOID AsicSetRxAnt(RTMP_ADAPTER *ad, UCHAR Ant)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->SetRxAnt)
		ops->SetRxAnt(ad, Ant);
}

#ifdef MICROWAVE_OVEN_SUPPORT
VOID AsicMeasureFalseCCA(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMeasureFalseCCA)
		ops->AsicMeasureFalseCCAad;
}

VOID AsicMitigateMicrowave(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->AsicMitigateMicrowave)
		ops->AsicMitigateMicrowave(ad);
}
#endif /* MICROWAVE_OVEN_SUPPORT */


VOID AsicBbpInitFromEEPROM(RTMP_ADAPTER *ad)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BbpInitFromEEPROM)
		ops->BbpInitFromEEPROM(ad);
}


#if defined(MT_MAC) && defined(TXBF_SUPPORT)
INT32 AsicBfStaRecUpdate(
	RTMP_ADAPTER *ad,
	UCHAR        ucPhyMode,
	UCHAR        ucBssIdx,
	UCHAR        ucWlanIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfStaRecUpdate) {
		return ops->BfStaRecUpdate(
				   ad,
				   ucPhyMode,
				   ucBssIdx,
				   ucWlanIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfStaRecRelease(
	RTMP_ADAPTER *ad,
	UCHAR        ucBssIdx,
	UCHAR        ucWlanIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfStaRecRelease) {
		return ops->BfStaRecRelease(
				   ad,
				   ucBssIdx,
				   ucWlanIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfPfmuMemAlloc(
	RTMP_ADAPTER *ad,
	UCHAR ucSu_Mu,
	UCHAR ucWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfPfmuMemAlloc) {
		return ops->BfPfmuMemAlloc(
				   ad,
				   ucSu_Mu,
				   ucWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicBfPfmuMemRelease(
	RTMP_ADAPTER *ad,
	UCHAR ucWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfPfmuMemRelease) {
		return ops->BfPfmuMemRelease(
				   ad,
				   ucWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfTxApplyCtrl(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	BOOLEAN fgETxBf,
	BOOLEAN fgITxBf,
	BOOLEAN fgMuTxBf,
	BOOLEAN fgPhaseCali)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->TxBfTxApplyCtrl) {
		return ops->TxBfTxApplyCtrl(
				   ad,
				   ucWlanId,
				   fgETxBf,
				   fgITxBf,
				   fgMuTxBf,
				   fgPhaseCali);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfeeHwCtrl(
	RTMP_ADAPTER *ad,
	BOOLEAN   fgBfeeHwCtrl)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfeeHwCtrl) {
		return ops->BfeeHwCtrl(
				   ad,
				   fgBfeeHwCtrl);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfApClientCluster(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	UCHAR   ucCmmWlanId)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfApClientCluster) {
		return ops->BfApClientCluster(
				   ad,
				   ucWlanId,
				   ucCmmWlanId);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfReptClonedStaToNormalSta(
	RTMP_ADAPTER *ad,
	UCHAR   ucWlanId,
	UCHAR   ucCliIdx)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfReptClonedStaToNormalSta) {
		return ops->BfReptClonedStaToNormalSta(
				   ad,
				   ucWlanId,
				   ucCliIdx);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfHwEnStatusUpdate(
	RTMP_ADAPTER *ad,
	BOOLEAN   fgETxBf,
	BOOLEAN   fgITxBf)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->BfHwEnStatusUpdate) {
		return ops->BfHwEnStatusUpdate(
				   ad,
				   fgETxBf,
				   fgITxBf);
	} else {
		AsicNotSupportFunc(ad, __func__);
		return FALSE;
	}
}

INT32 AsicTxBfModuleEnCtrl(
	struct _RTMP_ADAPTER *pAd,
	UINT8 u1BfNum,
	UINT8 u1BfBitmap,
	UINT8 u1BfSelBand[])
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->BfModuleEnCtrl) {
		return ops->BfModuleEnCtrl(
					pAd,
					u1BfNum,
					u1BfBitmap,
					u1BfSelBand);
	} else {
		AsicNotSupportFunc(pAd, __func__);
		return FALSE;
	}
}

#endif /* MT_MAC && TXBF_SUPPORT */


INT32 chip_tssi_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->tssi_set)
		return ops->tssi_set(ad, efuse);
	return 0;
}

INT32 chip_pa_lna_set(struct _RTMP_ADAPTER *ad, char *efuse)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(ad->hdev_ctrl);

	if (ops->pa_lna_set)
		return ops->pa_lna_set(ad, efuse);

	return 0;
}

