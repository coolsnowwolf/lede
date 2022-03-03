/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"

NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;
	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RtmpCmdQInit(pAd);
	RTMP_OS_TASK_INIT(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTPCICmdThread, (ULONG)pTask);

	if (status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Unable to start RTPCICmdThread!\n"));
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
	/* start the crediential write task first. */
	WscThreadInit(pAd);
#endif /* WSC_INCLUDED */
	return NDIS_STATUS_SUCCESS;
}


VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd)
{
	INT ret;
	/* Terminate cmdQ thread */
	if (RTMP_OS_TASK_LEGALITY(&pAd->cmdQTask)) {
		RtmpCmdQExit(pAd);
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);
		/*RTUSBCMDUp(&pAd->cmdQTask); */
		ret = RtmpOSTaskKill(&pAd->cmdQTask);

		if (ret == NDIS_STATUS_FAILURE)
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Kill command task fail!\n"));

		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}
#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */
}

/*
 * ========================================================================
 * Routine Description:
 *    PCI command kernel thread.
 *
 * Arguments:
 *	*Context			the pAd, driver control block pointer
 *
 * Return Value:
 *   0					close the thread
 * ========================================================================
 */
INT RTPCICmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend) {
		/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;

		while (pAd->CmdQ.size) {
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);

			if (pCmdQElmt) {
				if (pCmdQElmt->CmdFromNdis == TRUE) {
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				} else {
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pCmdQElmt->buffer);

					os_free_mem((PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}

	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<---RTPCICmdThread\n"));
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}



#ifdef MT_MAC
NDIS_STATUS mt_pci_chip_cfg(RTMP_ADAPTER *pAd, USHORT  id)
{
	NDIS_STATUS ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (id == 0 || id == 0xffff)
		ret = NDIS_STATUS_FAILURE;

#ifdef CONFIG_FWOWN_SUPPORT

	if ((id == NIC7615_PCIe_DEVICE_ID)
		 || (id == NIC7616_PCIe_DEVICE_ID)
		 || (id == NIC761A_PCIe_DEVICE_ID)
		 || (id == NIC7611_PCIe_DEVICE_ID)
		 || (id == NIC7637_PCIe_DEVICE_ID))
		DriverOwn(pAd);

#endif
	if ((id == NIC7603_PCIe_DEVICE_ID)
		|| (id == NIC7615_PCIe_DEVICE_ID)
		|| (id == NIC7616_PCIe_DEVICE_ID)
		|| (id == NIC761A_PCIe_DEVICE_ID)
		|| (id == NIC7611_PCIe_DEVICE_ID)
		|| (id == NIC7637_PCIe_DEVICE_ID)
		|| (id == NICP18_PCIe_DEVICE_ID)
		|| (id == NIC7663_PCIe_DEVICE_ID)
	   ) {
		UINT32 Value;

		RTMP_IO_READ32(pAd, TOP_HVR, &Value);
		pAd->HWVersion = Value;

		if (Value == 0)
			ret = NDIS_STATUS_FAILURE;

		RTMP_IO_READ32(pAd, TOP_FVR, &Value);
		pAd->FWVersion = Value;

		if (Value == 0)
			ret = NDIS_STATUS_FAILURE;

		RTMP_IO_READ32(pAd, TOP_HCR, &Value);
		pAd->ChipID = Value;

		if (id == NIC7616_PCIe_DEVICE_ID)
			pAd->RfIcType = RFIC_7616;
		else if (id == NIC761A_PCIe_DEVICE_ID)
			pAd->RfIcType = RFIC_7615A;

		if (Value == 0)
			ret = NDIS_STATUS_FAILURE;

		if (IS_MT7603(pAd)) {
			RTMP_IO_READ32(pAd, STRAP_STA, &Value);
			pAd->AntMode = (Value >> 24) & 0x1;
		}

		cap->hif_type = HIF_MT;
		pAd->infType = RTMP_DEV_INF_PCIE;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
				 ("%s(): HWVer=0x%x, FWVer=0x%x, pAd->ChipID=0x%x\n",
				  __func__, pAd->HWVersion, pAd->FWVersion, pAd->ChipID));
#ifdef MT7615

		if (IS_MT7615(pAd)) {
			RTMP_IO_READ32(pAd, HIF_SYS_REV, &Value);

			if (Value == 0)
				ret = NDIS_STATUS_FAILURE;

			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF,
					 ("%s(): HIF_SYS_REV=0x%x\n", __func__, Value));
		}

#endif /* MT7615 */
	}

	return ret;
}
#endif /* MT_MAC */


/***************************************************************************
 *
 *	PCIe device initialization related procedures.
 *
 ***************************************************************************/
VOID RTMPInitPCIeDevice(RT_CMD_PCIE_INIT *pConfig, VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	VOID *pci_dev = pConfig->pPciDev;
	USHORT  device_id = 0;
	POS_COOKIE pObj;
	NDIS_STATUS ret;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pci_read_config_word(pci_dev, pConfig->ConfigDeviceID, &device_id);
#ifndef RT_BIG_ENDIAN
	device_id = le2cpu16(device_id);
#endif /* RT_BIG_ENDIAN */
	pObj->DeviceID = device_id;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s():device_id=0x%x\n",
			 __func__, device_id));
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
#ifdef MT_MAC
	ret = mt_pci_chip_cfg(pAd, device_id);

	/* check pci configuration CR can be read successfully */
	if (ret != NDIS_STATUS_SUCCESS) {
		pConfig->pci_init_succeed = FALSE;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("%s():pci configuration space can't be read\n",
				 __func__));
		return;
	}

	pConfig->pci_init_succeed = TRUE;

#endif /* */
#ifdef CONFIG_WIFI_MSI_SUPPORT
	pAd->PciHif.is_msi = ((struct pci_dev *)pci_dev)->msi_enabled;
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
	multi_inf_adapt_reg((VOID *) pAd);
#endif /* MULTI_INF_SUPPORT */
#endif
	if (pAd->infType != RTMP_DEV_INF_UNKNOWN)
		RtmpRaDevCtrlInit(pAd, pAd->infType);
}

#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
struct pci_dev *rtmp_get_pci_dev(void *ad)
{
	struct pci_dev *pdev = NULL;
#ifdef RTMP_PCI_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ad;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	pdev = obj->pci_dev;
#endif
	return pdev;
}
#endif
#endif

struct device *rtmp_get_dev(void *ad)
{
	struct device *dev = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ad;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	dev = (struct device *)obj->pDev;
	return dev;
}



