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
	rtmp_mcu.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/


#include	"rt_config.h"

INT MCUBurstWrite(PRTMP_ADAPTER pAd, UINT32 Offset, UINT32 *Data, UINT32 Cnt)
{
	return 0;
}


INT MCURandomWrite(PRTMP_ADAPTER pAd, RTMP_REG_PAIR *RegPair, UINT32 Num)
{
	UINT32 Index;

	for (Index = 0; Index < Num; Index++)
		RTMP_IO_WRITE32(pAd, RegPair->Register, RegPair->Value);

	return 0;
}


INT32 MCUSysPrepare(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);

	return Ret;
}

INT32 MCUSysInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MCU_CTRL_INIT(pAd);
	chip_fw_init(pAd);
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
	Ret = NICLoadFirmware(pAd);

	if (Ret != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: NICLoadFirmware failed, Status[=0x%08x]\n", __func__, Ret));
		return -1;
	}

	return Ret;
}


INT32 MCUSysExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->FwExit)
		ops->FwExit(pAd);

	MCU_CTRL_EXIT(pAd);
	return Ret;
}


VOID ChipOpsMCUHook(PRTMP_ADAPTER pAd, enum MCU_TYPE MCUType)
{
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef CONFIG_ANDES_SUPPORT

	if ((MCUType & ANDES) == ANDES) {
#ifdef MT_MAC
		if (cap->hif_type == HIF_MT) {
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
			if (IS_PCIE_INF(pAd) || IS_RBUS_INF(pAd)) {
				pChipOps->FwInit = AndesMTPciFwInit;
				pChipOps->FwExit = AndesMTPciFwExit;
				pChipOps->pci_kick_out_cmd_msg = AndesMTPciKickOutCmdMsg;
			}
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */
			pChipOps->andes_fill_cmd_header = AndesMTFillCmdHeaderWithTXD;
			pChipOps->rx_event_handler = AndesMTRxEventHandler;
		}
#endif /* MT_MAC */

		pChipOps->MtCmdTx = AndesSendCmdMsg;
		pChipOps->MCUCtrlInit = AndesCtrlInit;
		pChipOps->MCUCtrlExit = AndesCtrlExit;

		FwdlHookInit(pAd);
	}

#endif /* CONFIG_ANDES_SUPPORT */
}

