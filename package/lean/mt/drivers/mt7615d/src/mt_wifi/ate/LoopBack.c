#if defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#if defined(RTMP_USB_SUPPORT) || defined(RTMP_PCI_SUPPORT) || defined(MTK_UART_SUPPORT)
INT32 ATECheckFWROMWiFiSysOn(RTMP_ADAPTER *pAd)
{
	INT32 ntStatus = STATUS_SUCCESS;
	UINT32 loop = 0;
	UINT32 value = 0;
	/* Step 1 polling 0x81021250 to 0 */
	/* poll SW_SYN0 == 0 */
	loop = 0;

	do {
		value = MtAsicGetFwSyncValue(pAd);

		if (value == 0x0)
			break;

		os_msec_delay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: 2. SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __func__, value));
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	/* power on WiFi SYS*/
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: 2. power on WiFi SYS\n", __func__));
	ntStatus = MtCmdPowerOnWiFiSys(pAd);

	if (ntStatus)
		ntStatus = STATUS_UNSUCCESSFUL;

	/* poll SW_SYN0 == 1*/
	loop = 0;

	do {
		value = MtAsicGetFwSyncValue(pAd);

		if (value == 0x1)
			break;

		os_msec_delay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: SW_SYN0 is not at init. state (SW_SYN0 = %d)\n", __func__, value));
		ntStatus = STATUS_UNSUCCESSFUL;
	}

	return ntStatus;
}
#endif

INT32 CheckFWROMWiFiSysOn(IN  RTMP_ADAPTER *pAd)
{
	INT32 ntStatus = STATUS_SUCCESS;
#if defined(RTMP_USB_SUPPORT) || defined(RTMP_PCI_SUPPORT) || defined(MTK_UART_SUPPORT)
	ntStatus = ATECheckFWROMWiFiSysOn(pAd);
#endif
	return ntStatus;
}
INT32 FWSwitchToROM(
	IN  RTMP_ADAPTER *pAd
)
{
	INT32		ntStatus = STATUS_SUCCESS;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
#ifdef COMPOS_TESTMODE_WIN		/* Linux ready in FWRAM */

	if (pAd->FWMode == FWROM)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: FWMode is already ROM\n", __func__));
	else if (pAd->FWMode == FWRAM)
#endif
	{
		/* Step 1 set firmware to ROM mode */
		/* ntStatus = FirmwareCommnadRestartDownloadFW(pAd); */
		/* if(ntStatus != STATUS_SUCCESS) */
		/* { */
		/* MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s, RestartDownloadFW cmd failed\n",__FUNCTION__)); */
		/* } */
		NICRestartFirmware(pAd);
		ntStatus = CheckFWROMWiFiSysOn(pAd);

		if (ntStatus != STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, CheckFWROMWiFiSysOn  failed\n", __func__));
	}

	return ntStatus;
}
void LoopBack_Start(RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	/* UINT16 BreakCount = 0; */
#ifdef COMPOS_TESTMODE_WIN		/* TODO::Fix the strcture */
#endif
	/* UINT32				Length = 0; */
	/* UINT32				RepeatIdx = 0; */
	UINT32				RxQ = 1;
	/* RtlCopyMemory(&pAd->LoopBackSetting, pSetting,sizeof(struct _LOOPBACK_SETTING)); */
	os_move_mem(&pAd->LbCtrl.LoopBackSetting, pSetting, sizeof(struct _LOOPBACK_SETTING));
	/* RtlZeroMemory(&pAd->LoopBackTxRaw, LOOPBACK_SIZE); */
	os_zero_mem(&pAd->LbCtrl.LoopBackTxRaw, LOOPBACK_SIZE);
	/* RtlZeroMemory(&pAd->LoopBackRxRaw, LOOPBACK_SIZE); */
	os_zero_mem(&pAd->LbCtrl.LoopBackRxRaw, LOOPBACK_SIZE);
	/* RtlZeroMemory(&pAd->LoopBackResult, sizeof(struct _LOOPBACK_RESULT)); */
	os_zero_mem(&pAd->LbCtrl.LoopBackResult, sizeof(struct _LOOPBACK_RESULT));

	if (pSetting->StartLen < sizeof(TMAC_TXD_L)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s startLength(0x%x) is shorter than sizeof(TMAC_TXD_L) (0x%lx)\n", __func__, pSetting->StartLen, (ULONG)sizeof(TMAC_TXD_L)));
		return;
	}

	OS_SPIN_LOCK(&pAd->LbCtrl.LoopBackLock);
	pAd->LbCtrl.LoopBackWaitRx = FALSE;
	OS_SPIN_UNLOCK(&pAd->LbCtrl.LoopBackLock);
	pAd->LbCtrl.LoopBackRunning = TRUE;
	pAd->LbCtrl.LoopBackResult.Status = RUNNING;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IsDefaultPattern	%d\n", pSetting->IsDefaultPattern));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RepeatTimes		%u, %u\n", pSetting->RepeatTimes, pAd->LbCtrl.LoopBackSetting.RepeatTimes));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("StartLen			%d\n", pSetting->StartLen));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("StopLen			%d\n", pSetting->StopLen));
	pAd->LbCtrl.LoopBackDefaultPattern = (BOOLEAN)pSetting->IsDefaultPattern;

	if (pAd->LbCtrl.DebugMode) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("In LoopBack Debug Mode\n"));
		goto ATE_LPBK_DEBUG;
	}

	if (!pAd->LbCtrl.LoopBackUDMA) {
		ntStatus = FWSwitchToROM(pAd);

		if (ntStatus != STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, Switch to ROM failed (0x%x)\n", __func__, ntStatus));

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s FWSwitchToROM\n", __func__));
	}


	if (!pAd->LbCtrl.LoopBackUDMA)
		MtCmdHIFLoopBackTest(pAd, 1, (UINT8)RxQ);

ATE_LPBK_DEBUG:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("***************************************%s @#@# Rx[%d]***************************************\n", __func__, RxQ));
	/* pAd->LbCtrl.LoopBackTxThread = NULL; */
	/*
	 * ntStatus =  PsCreateSystemThread(&pAd->LbCtrl.LoopBackTxTask.hThread,
	 *								(ACCESS_MASK) 0L,
	 *								NULL,
	 *								NULL,
	 *								NULL,
	 *								LoopBack_TxThread,
	 *								pAd);
	*/
	ntStatus =  RtmpOSTaskAttach(&pAd->LbCtrl.LoopBackTxTask, LoopBack_TxThread, (ULONG)&pAd->LbCtrl.LoopBackTxTask);

	if (ntStatus != STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, thread create fail\n", __func__));
}
void LoopBack_Stop(RTMP_ADAPTER *pAd)
{
	UINT32 BreakCount = 0;

	while (pAd->LbCtrl.LoopBackWaitRx == TRUE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, @#@#BreakCount = %d\n", __func__, BreakCount));

		if (BreakCount > 100)
			break;

#ifdef COMPOS_TESTMODE_WIN
		RTMPusecDelay(300);
#else
		RtmpusecDelay(300);
#endif
		BreakCount++;
	}

	if (RtmpOSTaskKill(&pAd->LbCtrl.LoopBackTxTask) == NDIS_STATUS_FAILURE)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("kill LoopBackTxTask task failed!\n"));

	if (pAd->LbCtrl.LoopBackResult.Status == RUNNING) {
		pAd->LbCtrl.LoopBackResult.Status = PASS;
		pAd->LbCtrl.LoopBackResult.FailReason = NO_ERROR;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("***************************************%s @#@#***************************************\n", __func__));
	pAd->LbCtrl.LoopBackRunning = FALSE;

	if (pAd->LbCtrl.DebugMode)
		goto ATE_LB_DEBUG_STOP;


	if (!pAd->LbCtrl.LoopBackUDMA)
		MtCmdHIFLoopBackTest(pAd, 0, 0);

ATE_LB_DEBUG_STOP:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s,\n", __func__));
}
void LoopBack_Status(RTMP_ADAPTER *pAd, struct _LOOPBACK_RESULT *pResult)
{
	os_move_mem(pResult, &pAd->LbCtrl.LoopBackResult, sizeof(struct _LOOPBACK_RESULT));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Status: %d\n", __func__, pAd->LbCtrl.LoopBackResult.Status));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, FailReason: %d\n", __func__, pAd->LbCtrl.LoopBackResult.FailReason));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RxByteCount: %d\n", __func__, pAd->LbCtrl.LoopBackResult.RxByteCount));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RxPktCount: %d\n", __func__, pAd->LbCtrl.LoopBackResult.RxPktCount));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, TxByteCount: %d\n", __func__, pAd->LbCtrl.LoopBackResult.TxByteCount));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, TxPktCount: %d\n", __func__, pAd->LbCtrl.LoopBackResult.TxPktCount));
}
void LoopBack_RawData(RTMP_ADAPTER *pAd, UINT32  *pLength, BOOLEAN IsTx, UCHAR *RawData)
{
	if (*pLength > LOOPBACK_SIZE) {
		*pLength = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, max length is %d\n", __func__, LOOPBACK_SIZE));
	}

	if (IsTx) {
		*pLength = pAd->LbCtrl.LoopBackTxRawLen;
		/* RtlCopyMemory(RawData, &pAd->LoopBackTxRaw,*pLength); */
		os_move_mem(RawData, &pAd->LbCtrl.LoopBackTxRaw, *pLength);
	} else {
		*pLength = pAd->LbCtrl.LoopBackRxRawLen;
		/* RtlCopyMemory(RawData, &pAd->LoopBackRxRaw,*pLength); */
		os_move_mem(RawData, &pAd->LbCtrl.LoopBackRxRaw, *pLength);
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Length = 0x%x\n", __func__, *pLength));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, LoopBackRxRawLen = 0x%x\n", __func__, pAd->LbCtrl.LoopBackRxRawLen));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, LoopBackTxRawLen = 0x%x\n", __func__, pAd->LbCtrl.LoopBackTxRawLen));
}
void LoopBack_ExpectRx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData)
{
	PULONG ptr;
	UINT8	i = 0;

	if (Length > LOOPBACK_SIZE) {
		Length = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, max length is %d\n", __func__, LOOPBACK_SIZE));
	}

	/* (&pAd->LbCtrl.LoopBackExpectRx, pRawData, Length); */
	os_move_mem(&pAd->LbCtrl.LoopBackExpectRx, pRawData, Length);
	ptr = (PULONG)(&pAd->LbCtrl.LoopBackExpectRx);
	/* Length = ptr[0] & 0xffff; */
	pAd->LbCtrl.LoopBackExpectRxLen = Length;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Length = %d\n", __func__, Length));

	for (i = 0; i < 20; i++)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TXD(TXWI) %d 0x%08lX\n", i, *(ptr + i)));

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s,\n", __func__));
}

void LoopBack_ExpectTx(RTMP_ADAPTER *pAd, UINT32 Length, UINT8 *pRawData)
{
	PULONG ptr;
	UINT8	i = 0;

	if (Length > LOOPBACK_SIZE) {
		Length = LOOPBACK_SIZE;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, max length is %d\n", __func__, LOOPBACK_SIZE));
	}

	/* RtlCopyMemory(&pAd->LoopBackExpectTx, pRawData, Length); */
	os_move_mem(&pAd->LbCtrl.LoopBackExpectTx, pRawData, Length);
	ptr = (PULONG)(&pAd->LbCtrl.LoopBackExpectTx);
	/* Length = ptr[0] & 0xffff; */
	pAd->LbCtrl.LoopBackExpectTxLen = Length;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Length = %d\n", __func__, Length));

	for (i = 0; i < 20; i++)
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TXD(TXWI) %d 0x%08lX\n", i, *(ptr + i)));

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s,\n", __func__));
}

void LoopBack_Run(RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting, UINT32 Length)
{
	if (Length > LOOPBACK_SIZE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, LOOPBACK length too long\n", __func__));
		return;
	}

#ifdef _RTMP_PCI_SUPPORT
	PCILoopBack_Run(pAd, pSetting, Length);
#endif
}
void LoopBack_BitTrueCheck(RTMP_ADAPTER *pAd)
{
	if (!pAd->LbCtrl.LoopBackDefaultPattern) { /* Rx compare expect Rx */
		if (pAd->LbCtrl.LoopBackExpectRxLen != pAd->LbCtrl.LoopBackRxRawLen) {
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### TX/RX Length not equal ####################\n", __func__));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ExpectRxLen = %d, RxRawLen = %d\n", __func__, pAd->LbCtrl.LoopBackExpectRxLen, pAd->LbCtrl.LoopBackRxRawLen));
		} else if (RTMPEqualMemory((PVOID)&pAd->LbCtrl.LoopBackExpectRx, (PVOID)&pAd->LbCtrl.LoopBackRxRaw, pAd->LbCtrl.LoopBackRxRawLen) == 0) {
			if (pAd->LbCtrl.DebugMode) {
				UINT32 j = 0;

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxExpect Dump(%u): ", pAd->LbCtrl.LoopBackRxRawLen));

				for (j = 0; j < pAd->LbCtrl.LoopBackRxRawLen; j++)
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", pAd->LbCtrl.LoopBackExpectRx[j]));

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxBackRaw Dump: "));

				for (j = 0; j < pAd->LbCtrl.LoopBackRxRawLen; j++)
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02x", pAd->LbCtrl.LoopBackRxRaw[j]));

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
			}

			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### BIT_TRUE_FAIL ####################\n", __func__));
		} else
			pAd->LbCtrl.LoopBackResult.FailReason = NO_ERROR;
	} else { /* Rx compare Tx */
		if (pAd->LbCtrl.LoopBackTxRawLen != pAd->LbCtrl.LoopBackRxRawLen) {
			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### TX/RX Length not equal ####################\n", __func__));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, TxRawLen = %d, RxRawLen = %d\n", __func__, pAd->LbCtrl.LoopBackTxRawLen, pAd->LbCtrl.LoopBackRxRawLen));
		} else if (RTMPEqualMemory((PVOID)&pAd->LbCtrl.LoopBackTxRaw, (PVOID)&pAd->LbCtrl.LoopBackRxRaw, pAd->LbCtrl.LoopBackTxRawLen) == 0) {
			if (pAd->LbCtrl.DebugMode) {
				UINT32 j = 0;

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("TxExpect Dump(%u): ", pAd->LbCtrl.LoopBackTxRawLen));

				for (j = 0; j < pAd->LbCtrl.LoopBackRxRawLen; j++)
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x", pAd->LbCtrl.LoopBackTxRaw[j]));

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RxBackRaw Dump: "));

				for (j = 0; j < pAd->LbCtrl.LoopBackRxRawLen; j++)
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x", pAd->LbCtrl.LoopBackRxRaw[j]));

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
			}

			LoopBack_Fail(pAd, BIT_TRUE_FAIL);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### BIT_TRUE_FAIL ####################\n", __func__));
		} else {
			pAd->LbCtrl.LoopBackResult.FailReason = NO_ERROR;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### BIT_TRUE OK ####################\n", __func__));
		}
	}
}

void LoopBack_Fail(RTMP_ADAPTER *pAd, enum _LOOPBACK_FAIL FailNum)
{
	if (pAd->LbCtrl.LoopBackResult.Status == RUNNING) {
		pAd->LbCtrl.LoopBackResult.FailReason = FailNum;
		pAd->LbCtrl.LoopBackResult.Status = FAIL;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, FailReason = %d\n", __func__, FailNum));
	}
}

#ifdef COMPOS_TESTMODE_WIN
INT LoopBack_TxThread(IN OUT PVOID Context)
#else
INT LoopBack_TxThread(ULONG Context)
#endif
{
#ifdef COMPOS_TESTMODE_WIN
	RTMP_ADAPTER	*pAd = (RTMP_ADAPTER *)Context;
#else
	RTMP_OS_TASK *pTask = (RTMP_OS_TASK *)Context;
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
#endif
	struct _LOOPBACK_SETTING *pSetting = &pAd->LbCtrl.LoopBackSetting;
	UINT32 RepeatIdx = 0;
	UINT32 Length = 0;
	UINT32 BreakCount = 0;
	UINT32 DbgCount = 0;
	/* KIRQL Irql; */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Repeat:%u\n", __func__, pSetting->RepeatTimes));

	if (pSetting->RepeatTimes == 0)
		pSetting->RepeatTimes = 0xffffffff;

	for (RepeatIdx = 0; RepeatIdx < pSetting->RepeatTimes ; RepeatIdx++) {
		if (pSetting->RepeatTimes == 0xffffffff)
			RepeatIdx = 0;

		for (Length = pSetting->StartLen; Length <= pSetting->StopLen; Length++) {
			while (pAd->LbCtrl.LoopBackWaitRx) {
				BreakCount++;
#ifdef COMPOS_TESTMODE_WIN
				RTMPusecDelay(50);
#else
				RtmpusecDelay(50);
#endif

				if (BreakCount > 2000) {
					MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, no Rx come back  Stop1!!!\n", __func__));
					break;
				}

				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, no Rx BreakCount = %d\n", __func__, BreakCount));
			}

			if (BreakCount > 2000) {
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, no Rx come back  Stop2!!!\n", __func__));
				LoopBack_Fail(pAd, RX_TIMEOUT);
				break;
			}

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, Length =  %d\n", __func__, Length));
			BreakCount = 0;

			if (pAd->LbCtrl.LoopBackRunning == FALSE || pAd->LbCtrl.LoopBackResult.Status != RUNNING)
				break;

			if (!pSetting->IsDefaultPattern)
				Length = pAd->LbCtrl.LoopBackExpectTxLen;

			if (pAd->LbCtrl.LoopBackResult.Status == RUNNING) {
				/* KeAcquireSpinLock(&pAd->LoopBackLock, &Irql); */
				OS_SPIN_LOCK(&pAd->LbCtrl.LoopBackLock);
				pAd->LbCtrl.LoopBackWaitRx = TRUE;
				/* KeReleaseSpinLock(&pAd->LoopBackLock, Irql); */
				OS_SPIN_UNLOCK(&pAd->LbCtrl.LoopBackLock);
				LoopBack_Run(pAd, pSetting, Length);

				if (pAd->LbCtrl.LoopBackRunning == FALSE || pAd->LbCtrl.LoopBackResult.Status != RUNNING)
					break;
			}

#ifdef COMPOS_TESTMODE_WIN
			RTMPusecDelay(200);
#else
			RtmpusecDelay(200);
#endif
			DbgCount++;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, DbgCount =  %d\n", __func__, DbgCount));

			if (!pSetting->IsDefaultPattern) {
				/* use script file does not need to auto increase length */
				break;
			}
		}

		if (pAd->LbCtrl.LoopBackRunning == FALSE || pAd->LbCtrl.LoopBackResult.Status != RUNNING)
			break;
	}

	if (pAd->LbCtrl.LoopBackRunning)
		LoopBack_Stop(pAd);

	/* pAd->LbCtrl.LoopBackTxThread = NULL; */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("+ + + + Control Thread Terminated + + + +\n"));
	/* DecrementIoCount(pAd); */
	/* PsTerminateSystemThread(STATUS_SUCCESS); */
	RtmpOSTaskNotifyToExit(&pAd->LbCtrl.LoopBackTxTask);
	return 0;
}

VOID LoopBack_Rx(RTMP_ADAPTER *pAd, UINT32 pktlen, UINT8 *pData)
{
	if (!pAd->LbCtrl.LoopBackRunning && !pAd->LbCtrl.LoopBackWaitRx)
		return;

	{
		UINT32 LPLength = 0;
		INT32 TotalLength = (INT32)pktlen;
		UINT8  *ptr = pData;
		EVENT_RXD EvnRxD;

		/* FwCMDRspTxD_STRUC FwCMDRspTxD; */
		if (pAd->LbCtrl.DebugMode) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Debug Mode, Total Len:%d\n", __func__, TotalLength));
			LPLength = pktlen;
		} else {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Driver Rx LoopBackRunning\n"));
			/* os_move_mem(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD)); */
			os_move_mem(&EvnRxD, ptr, sizeof(EvnRxD));
			/* LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount; */
			LPLength = EvnRxD.fw_rxd_0.field.length;

			if (LPLength > LOOPBACK_SIZE) {
				LPLength = LOOPBACK_SIZE;
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, max length is %d\n", __func__, LOOPBACK_SIZE));
			}
		}

		{
#ifdef COMPOS_TESTMODE_WIN	/* Windows First -4 for padding bits and then add back here,Linux(?) */

			if (!pAd->LbCtrl.LoopBackSetting.IsDefaultPattern)
				LPLength = TotalLength + 4;

#endif
			/* RtlCopyMemory(&FwCMDRspTxD,ptr,sizeof(FwCMDRspTxD)); */
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Total Length = %d, LPLength is %d, bytecount = %d\n",
					 __func__, TotalLength, LPLength, EvnRxD.fw_rxd_0.field.length));
			/* LPLength = FwCMDRspTxD.FwEventTxD.u2RxByteCount; */
			pAd->LbCtrl.LoopBackResult.RxByteCount += LPLength;
			pAd->LbCtrl.LoopBackResult.RxPktCount++;
			os_move_mem(&pAd->LbCtrl.LoopBackRxRaw, ptr, LPLength);
			pAd->LbCtrl.LoopBackRxRawLen = LPLength;
		}

		if (pAd->LbCtrl.LoopBackUDMA == FALSE)
			LoopBack_BitTrueCheck(pAd);

		/* KeAcquireSpinLock(&pAdapter->LoopBackLock, &Irql); */
		OS_SPIN_LOCK(&pAd->LbCtrl.LoopBackLock);
		pAd->LbCtrl.LoopBackWaitRx = FALSE;
		/* KeReleaseSpinLock(&pAdapter->LoopBackLock, Irql); */
		OS_SPIN_UNLOCK(&pAd->LbCtrl.LoopBackLock);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RxPktCount = %d\n", __func__, pAd->LbCtrl.LoopBackResult.RxPktCount));
	}
}


/* currently not implement PCIE loopback */
#ifdef _RTMP_PCI_SUPPORT
void PCILoopBack_Run(RTMP_ADAPTER *pAd, struct _LOOPBACK_SETTING *pSetting, UINT32 length)
{
	UINT32 count = 0;
	TMAC_TXD_L TxD;
	PUCHAR ptr;
	UINT8 alignment = 0;
	/* UINT32 TimeOut1Second = 100; */
	UCHAR *TxDataBuffer = NULL;
	UINT32 *TxDataBufferLength;
	UINT32 BUF_SIZE = 0;
#ifdef COMPOS_TESTMODE_WIN
	TxDataBuffer = pAd->TxDataBuffer;
	BUF_SIZE = BUFFER_SIZE;
	TxDataBufferLength = &pAd->TxDataBufferLength;
#else
	struct _ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	TxDataBuffer = pAd->ATECtrl.pate_pkt;
	BUF_SIZE = ATE_TESTPKT_LEN;
	TxDataBufferLength = &pAd->ATECtrl.TxLength;
#endif

	/* length = 1024; */
	if (pSetting->IsDefaultPattern) {
		os_zero_mem(TxDataBuffer, BUF_SIZE);
		os_zero_mem(&TxD, sizeof(TxD));
		ptr = TxDataBuffer + sizeof(TxD);

		for (count = 0; count < length - sizeof(TxD); count++)
			ptr[count] = count % 16;

		/* set pkt content */
		TxD.TxD0.TxByteCount = length;
#ifndef MT7615
		TxD.TxD0.PIdx = 1;
		TxD.TxD0.QIdx = 0;
		TxD.TxD1.TxDFmt = 1;
		TxD.TxD1.HdrFmt = 1;
#endif
		os_move_mem(TxDataBuffer, &TxD, sizeof(TxD));
		os_move_mem(&pAd->LbCtrl.LoopBackTxRaw, TxDataBuffer, LOOPBACK_SIZE);
		pAd->LbCtrl.LoopBackTxRawLen = length;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, length = %d\n", __func__, length));

		if (length > 32) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("===buffer===\n"));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[3], TxDataBuffer[2], TxDataBuffer[1], TxDataBuffer[0]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[7], TxDataBuffer[6], TxDataBuffer[5], TxDataBuffer[4]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[11], TxDataBuffer[10], TxDataBuffer[9], TxDataBuffer[8]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[15], TxDataBuffer[14], TxDataBuffer[13], TxDataBuffer[12]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[19], TxDataBuffer[18], TxDataBuffer[17], TxDataBuffer[16]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[23], TxDataBuffer[22], TxDataBuffer[21], TxDataBuffer[20]));
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("0x%x 0x%x 0x%x 0x%x\n", TxDataBuffer[27], TxDataBuffer[26], TxDataBuffer[25], TxDataBuffer[24]));
		}

		if (length % 4 != 0)
			alignment = 4 - (length % 4);

		if (pAd->LbCtrl.DebugMode) {
			int j = 0;

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Tx Dump: "));

			for (j = 0; j < length; j++)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%02x", TxDataBuffer[j]));

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));
		}

		/* pAd->TxDataBufferLength = length + alignment + 4 ; */
		*TxDataBufferLength = length + alignment + 4;
	} else {
		os_move_mem(TxDataBuffer, &pAd->LbCtrl.LoopBackExpectTx, pAd->LbCtrl.LoopBackExpectTxLen);
		os_zero_mem(TxDataBuffer + length, pAd->LbCtrl.LoopBackExpectTxLen - length);
		os_move_mem(&pAd->LbCtrl.LoopBackTxRaw, TxDataBuffer, length);
		pAd->LbCtrl.LoopBackTxRawLen = pAd->LbCtrl.LoopBackExpectTxLen;
		/* pAd->LoopBackExpectRxLen = length; */
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, LP length = %d, alignment = %d\n", __func__, length, alignment));
	pAd->LbCtrl.LoopBackResult.TxByteCount += length;
	pAd->LbCtrl.LoopBackResult.TxPktCount++;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, RxPktCount = %d\n", __func__, pAd->LbCtrl.LoopBackResult.RxPktCount));
#ifdef COMPOS_TESTMODE_WIN
	StartTx(pAd, 1, 0);
#else
	ate_ctrl->TxLength = length;
	MT_ATETxPkt(pAd, TESTMODE_BAND0);
#endif

	/* cannot pass UINT32 TimeOut1Second = 100; to wait_event, it will not wait.. workaround use 0 (1second instead of TimeOut1Second) */
	if (!pAd->LbCtrl.DebugMode)
		if (RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pAd->LbCtrl.LoopBackPCITxEvent, 0) != STATUS_SUCCESS) {
			LoopBack_Fail(pAd, TX_TIMEOUT);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, ####################### TX_TIMEOUT ####################3\n", __func__));
		}
}
#endif

