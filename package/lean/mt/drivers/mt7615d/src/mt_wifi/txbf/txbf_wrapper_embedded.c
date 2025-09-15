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
	txbf_wrapper_embedded.c
*/

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "rt_config.h"

/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

/*******************************************************************************
*                            P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*                           P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

/*******************************************************************************
*                              F U N C T I O N S
********************************************************************************
*/
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
/*----------------------------------------------------------------------------*/

/* Wrap function for TxBFInit */
VOID mt_WrapTxBFInit(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN BOOLEAN			supportsETxBF)
{
	TXBF_MAC_TABLE_ENTRY TxBfMacEntry;
	TXBF_STATUS_INFO  TxBfInfo;
	HT_BF_CAP *pTxBFCap = &ie_list->HTCapability.TxBFCap;

	TxBfInfo.ucTxPathNum = pAd->Antenna.field.TxPath;
	TxBfInfo.ucETxBfTxEn = (UCHAR) pAd->CommonCfg.ETxBfEnCond;
	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	TxBfInfo.ucITxBfTxEn = pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn;
	mt_TxBFInit(pAd, &TxBfInfo, &TxBfMacEntry, pTxBFCap, supportsETxBF);
	pEntry->bfState        = TxBfMacEntry.bfState;
	pEntry->sndgMcs        = TxBfMacEntry.sndgMcs;
	pEntry->sndg0Snr0      = TxBfMacEntry.sndg0Snr0;
	pEntry->sndg0Snr1      = TxBfMacEntry.sndg0Snr1;
	pEntry->sndg0Snr2      = TxBfMacEntry.sndg0Snr2;
	pEntry->sndg0Mcs       = TxBfMacEntry.sndg0Mcs;
	pEntry->noSndgCnt      = TxBfMacEntry.noSndgCnt;
	pEntry->eTxBfEnCond    = TxBfMacEntry.eTxBfEnCond;
	pEntry->noSndgCntThrd  = TxBfMacEntry.noSndgCntThrd;
	pEntry->ndpSndgStreams = TxBfMacEntry.ndpSndgStreams;
	pEntry->iTxBfEn        = TxBfMacEntry.iTxBfEn;
}

/* Wrap function for clientSupportsETxBF */
BOOLEAN mt_WrapClientSupportsETxBF(
	IN  PRTMP_ADAPTER    pAd,
	IN  HT_BF_CAP       *pTxBFCap)
{
	TXBF_STATUS_INFO  TxBfInfo;

	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	return mt_clientSupportsETxBF(pAd, pTxBFCap, TxBfInfo.cmmCfgETxBfNoncompress);
}


#ifdef VHT_TXBF_SUPPORT
/* Wrap function for clientSupportsVHTETxBF */
BOOLEAN mt_WrapClientSupportsVhtETxBF(
	IN  PRTMP_ADAPTER    pAd,
	IN  VHT_CAP_INFO * pTxBFCap)
{
	return mt_clientSupportsVhtETxBF(pAd, pTxBFCap);
}
#endif /* VHT_TXBF_SUPPORT */


/* wrapper for mt_chk_itxbf_calibration */
BOOLEAN mt_Wrap_chk_itxbf_calibration(
	IN RTMP_ADAPTER * pAd, struct wifi_dev *wdev)
{
	TXBF_STATUS_INFO TxBfInfo;

	TxBfInfo.u2Channel = wdev->channel;
	/* return mt_chk_itxbf_calibration(pAd,&TxBfInfo); */
	return TRUE;
}

/* wrapper for setETxBFCap */
void mt_WrapSetETxBFCap(
	IN  RTMP_ADAPTER * pAd,
	IN  struct wifi_dev *wdev,
	IN  HT_BF_CAP         *pTxBFCap)
{
	TXBF_STATUS_INFO   TxBfInfo;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	TxBfInfo.pHtTxBFCap = pTxBFCap;
	TxBfInfo.cmmCfgETxBfEnCond = wlan_config_get_etxbf(wdev);
	TxBfInfo.cmmCfgETxBfNoncompress = pAd->CommonCfg.ETxBfNoncompress;
	TxBfInfo.ucRxPathNum = pAd->Antenna.field.RxPath;
	if (ops->setETxBFCap)
		ops->setETxBFCap(pAd, &TxBfInfo);
}

#ifdef VHT_TXBF_SUPPORT
/* Wrapper for mt_setVHTETxBFCap */
void mt_WrapSetVHTETxBFCap(
	IN  RTMP_ADAPTER * pAd,
	IN  struct wifi_dev *wdev,
	IN  VHT_CAP_INFO * pTxBFCap)
{
	TXBF_STATUS_INFO   TxBfInfo;
	UINT8   ucTxPath = pAd->Antenna.field.TxPath;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0)
			ucTxPath = pAd->dbdc_band0_tx_path;
		else
			ucTxPath = pAd->dbdc_band1_tx_path;
	}
#endif

	TxBfInfo.pVhtTxBFCap = pTxBFCap;
	TxBfInfo.ucTxPathNum = ucTxPath;
	TxBfInfo.cmmCfgETxBfEnCond = wlan_config_get_etxbf(wdev);
	if (ops->setVHTETxBFCap)
		ops->setVHTETxBFCap(pAd, &TxBfInfo);
}
#endif /* VHT_TXBF_SUPPORT */

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
