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
	hw_init.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "hw_init.tmh"
#endif
#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif


/*Local function*/




#ifdef RTMP_MAC_PCI
static INT mt_hif_sys_pci_init(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef MT_MAC
	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		mt_asic_init_txrx_ring(pAd);
		if (ops->hif_set_pcie_read_params
			#ifdef ERR_RECOVERY
			&& (IsErrRecoveryInIdleStat(pAd) == FALSE)
			#endif /* ERR_RECOVERY*/
			)
			ops->hif_set_pcie_read_params(pAd);
	}
#endif /* MT_MAC */
	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &mac_val);

	/* mac_val |= 0xb0; // bit 7/5~4 => 1 */
	if (IS_MT7637(pAd))
		mac_val = 0x52001055; /* workaround PDMA issue for WHQA_00022606 */
	else if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd))
		mac_val = 0x10001870;
	else
		mac_val = 0x52000850;

	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, mac_val);
	if (ops->dma_shdl_init)
		ops->dma_shdl_init(pAd);
	return NDIS_STATUS_SUCCESS;
}
#endif

static INT mt_hif_sys_init(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{
#ifdef RTMP_MAC_PCI
#ifndef COMPOS_TESTMODE_WIN

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		mt_hif_sys_pci_init(pAd);
	}

#endif
#endif /* RTMP_MAC_PCI */
	return NDIS_STATUS_SUCCESS;
}



/*HW related init*/

INT32 WfHifHwInit(RTMP_ADAPTER *pAd, HIF_INFO_T *pHifInfo)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		ret = mt_hif_sys_init(pAd, pHifInfo);

#endif /* MT_MAC */
	return ret;
}

static INT32 WfTopHwInit(RTMP_ADAPTER *pAd)
{
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		return MtAsicTOPInit(pAd);

#endif
	return FALSE;
}

static INT32 WfMcuHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#ifdef COMPOS_WIN
	/* #elif defined (COMPOS_TESTMODE_WIN) */
#else
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->prepare_fwdl_img)
		ops->prepare_fwdl_img(pAd);

	ret = NICLoadRomPatch(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NICLoadRomPatch failed, Status[=0x%08x]\n", __func__, ret));
		return NDIS_STATUS_FAILURE;
	}

#endif
	{
		UINT32 Value;
#ifdef WIFI_SPECTRUM_SUPPORT
		/* Refer to profile setting to decide the sysram partition format */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Before NICLoadFirmware, check ICapMode = %d\n", __func__, pAd->ICapMode));

		if (pAd->ICapMode == 2) { /* Wifi-spectrum */
			if (IS_MT7615(pAd)) {
				HW_IO_READ32(pAd, CONFG_COM1_REG3, &Value);
				Value = Value | CONFG_COM1_REG3_FWOPMODE;
				HW_IO_WRITE32(pAd, CONFG_COM1_REG3, Value);
			} else if (IS_MT7622(pAd)) {
				HW_IO_READ32(pAd, CONFG_COM2_REG3, &Value);
				Value = Value | CONFG_COM2_REG3_FWOPMODE;
				HW_IO_WRITE32(pAd, CONFG_COM2_REG3, Value);
			}
		} else
#endif /* WIFI_SPECTRUM_SUPPORT */
		{
			if (IS_MT7615(pAd)) {
				HW_IO_READ32(pAd, CONFG_COM1_REG3, &Value);
				Value = Value & (~CONFG_COM1_REG3_FWOPMODE);
				HW_IO_WRITE32(pAd, CONFG_COM1_REG3, Value);
			} else if (IS_MT7622(pAd)) {
				HW_IO_READ32(pAd, CONFG_COM2_REG3, &Value);
				Value = Value & (~CONFG_COM2_REG3_FWOPMODE);
				HW_IO_WRITE32(pAd, CONFG_COM2_REG3, Value);
			}
		}
	}
	ret = NICLoadFirmware(pAd);

	if (ret != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NICLoadFirmware failed, Status[=0x%08x]\n", __func__, ret));
		return NDIS_STATUS_FAILURE;
	}

	/*After fw download should disalbe dma schedule bypass mode*/
#ifdef DMA_SCH_SUPPORT
#if defined(COMPOS_TESTMODE_WIN)
	/* fix build error for testmode, need to check where testmode disable dma bypass mode */
#else

	if (AsicWaitPDMAIdle(pAd, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			ret =  NDIS_STATUS_FAILURE;
			return ret;
		}
	}

	AsicDMASchedulerInit(pAd, DMA_SCH_LMAC);
#endif /* defined (COMPOS_TESTMODE_WIN) */
#endif
	return ret;
}


static INT32 WfEPROMHwInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#if defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
#else
	NICInitAsicFromEEPROM(pAd);
#endif
	return ret;
}


/*Common Part for externl*/

INT32 WfTopInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	if (WfTopHwInit(pAd) != TRUE)
		ret = NDIS_STATUS_FAILURE;

	return ret;
}

INT32 WfHifInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	HIF_INFO_T hifInfo;

	os_zero_mem(&hifInfo, sizeof(HIF_INFO_T));
	ret = WfHifSysInit(pAd, &hifInfo);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	WfHifHwInit(pAd, &hifInfo);
	WLAN_HOOK_CALL(WLAN_HOOK_HIF_INIT, pAd, NULL);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Success!\n", __func__));
	return 0;
err:
	WfHifSysExit(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Err! status=%d\n", __func__, ret));
	return ret;
}

INT32 WfMcuInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = WfMcuSysInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	ret = WfMcuHwInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	HWCtrlOpsReg(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Success!\n", __func__));
	return ret;
err:
	WfMcuSysExit(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(), Err! status=%d\n", __func__, ret));
	return ret;
}

INT32 WfMacInit(RTMP_ADAPTER *pAd)
{
	UINT32 ret = NDIS_STATUS_SUCCESS;
#if defined(COMPOS_TESTMODE_WIN) && defined(RTMP_SDIO_SUPPORT)
	/* todo:  This function cause 7637 FPGA SDIO load FW failed, need to check. */
	return ret;
#endif
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		ret = mt_nic_asic_init(pAd);

#endif /* MT_MAC */
	/* Clear raw counters*/
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
	NicResetRawCounters(pAd);
#endif
	return ret;
}

INT32 WfEPROMInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	/* #if  defined(COMPOS_WIN) || defined (COMPOS_TESTMODE_WIN) */
#if  defined(COMPOS_TESTMODE_WIN)
#else
	ret = WfEPROMSysInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err;

	WfEPROMHwInit(pAd);
	return ret;
err:
	WfEPROMSysExit(pAd);
#endif
	return ret;
}

INT32 WfPhyInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#if  defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
#else
	NICInitBBP(pAd);
#endif
	return ret;
}

INT32 WfInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;

	ret = WfTopInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err0;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Top Init Done!\n"));
	ret = WfHifInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err0;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Hif Init Done!\n"));
	ret = WfMcuInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err1;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCU Init Done!\n"));
#ifdef RLM_CAL_CACHE_SUPPORT
	rlmCalCacheApply(pAd, pAd->rlmCalCache);
#endif /* RLM_CAL_CACHE_SUPPORT */
	/*Adjust eeprom + config => apply to HW*/
	ret = WfEPROMInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err2;

#ifdef SINGLE_SKU_V2
    /* Load SKU table to Host Driver */
    RTMPSetSkuParam(pAd);
#if defined(MT_MAC) && defined(TXBF_SUPPORT)
    /* Load BF Backoff table to Host Driver */
    RTMPSetBackOffParam(pAd);
#endif /* defined(MT_MAC) && defined(TXBF_SUPPORT) */
#endif /* SINGLE_SKU_V2 */
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("EEPROM Init Done!\n"));
	ret = WfMacInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err3;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MAC Init Done!\n"));
	ret = WfPhyInit(pAd);

	if (ret != NDIS_STATUS_SUCCESS)
		goto err3;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PHY Init Done!\n"));
	return ret;
#if  defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
err3:
err2:
err1:
err0:
#else
err3:
	WfEPROMSysExit(pAd);
err2:
	WfMcuSysExit(pAd);
err1:
	WfHifSysExit(pAd);
err0:
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): initial faild!! ret=%d\n", __func__, ret));
#endif
	return ret;
}


/*SW related init*/

INT32 WfSysPreInit(RTMP_ADAPTER *pAd)
{
#ifdef MT7615

	if (IS_MT7615(pAd))
		mt7615_init(pAd);

#endif /* MT7615 */
#ifdef MT7622

	if (IS_MT7622(pAd))
		mt7622_init(pAd);

#endif /* MT7622 */
	return 0;
}


INT32 WfSysPosExit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
#if  defined(COMPOS_WIN)  || defined(COMPOS_TESTMODE_WIN)
#else
	WfEPROMSysExit(pAd);
	WfMcuSysExit(pAd);
	WfHifSysExit(pAd);
#endif
	return ret;
}

INT32 WfSysCfgInit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}


INT32 WfSysCfgExit(RTMP_ADAPTER *pAd)
{
	INT32 ret = NDIS_STATUS_SUCCESS;
	return ret;
}
