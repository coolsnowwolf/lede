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
	hw_ctrl_basic.c
*/
#include "rt_config.h"
#include "hw_ctrl_basic.h"

extern HW_CMD_TABLE_T *HwCmdTable[];
extern HW_FLAG_TABLE_T HwFlagTable[];

/*==========================================================/
 //	Basic Command API implement															/
/==========================================================*/
static inline HwCmdHdlr HwCtrlValidCmd(HwCmdQElmt *CmdQelmt)
{
	UINT32 CmdType =  CmdQelmt->type;
	UINT32 CmdIndex = CmdQelmt->command;
	SHORT CurIndex = 0;
	HwCmdHdlr Handler = NULL;
	HW_CMD_TABLE_T  *pHwTargetTable = NULL;

	if (CmdType >= HWCMD_TYPE_END) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("CMD TPYE(%u) OOB error! HWCMD_TYPE_END %u\n",
				  CmdType, HWCMD_TYPE_END));
		return NULL;
	}

	if (CmdIndex >= HWCMD_ID_END) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("CMD ID(%u) OOB error! HWCMD_ID_END %u\n",
				  CmdIndex, HWCMD_ID_END));
		return NULL;
	}

	pHwTargetTable = HwCmdTable[CmdType];

	if (!pHwTargetTable) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("No HwCmdTable entry for this CMD %u Type %u\n",
				  CmdIndex, CmdType));
		return NULL;
	}

	CurIndex = 0;

	do {
		if (pHwTargetTable[CurIndex].CmdID == CmdIndex) {
			Handler = pHwTargetTable[CurIndex].CmdHdlr;
			pHwTargetTable[CurIndex].RfCnt++;
			break;
		}

		CurIndex++;
	} while (pHwTargetTable[CurIndex].CmdHdlr != NULL);

	if (Handler == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("No corresponding CMDHdlr for this CMD %u Type %u\n",
				  CmdIndex, CmdType));
	}

	return Handler;
}


static inline HwFlagHdlr HwCtrlValidFlag(PHwFlagCtrl pHwCtrlFlag)
{
	SHORT CurIndex = 0;
	HwFlagHdlr Handler = NULL;

	if (pHwCtrlFlag->FlagId > (1 << HWFLAG_ID_END)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("FLAG ID(%x) is out of boundary\n", pHwCtrlFlag->FlagId));
		pHwCtrlFlag->FlagId = 0;
		return NULL;
	}

	CurIndex = 0;

	do {
		if (HwFlagTable[CurIndex].FlagId & pHwCtrlFlag->FlagId) {
			Handler = HwFlagTable[CurIndex].FlagHdlr;
			/*Unmask flag*/
			pHwCtrlFlag->FlagId &=  ~(HwFlagTable[CurIndex].FlagId);
			HwFlagTable[CurIndex].RfCnt++;
			break;
		}

		CurIndex++;
	} while (HwFlagTable[CurIndex].FlagHdlr != NULL);

	if (Handler == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No corresponding FlagHdlr for this FlagID(%x)\n",  pHwCtrlFlag->FlagId));
		pHwCtrlFlag->FlagId = 0;
	}

	return Handler;
}

static VOID HwCtrlDequeueCmd(HwCmdQ *cmdq, HwCmdQElmt **pcmdqelmt)
{
	*pcmdqelmt = cmdq->head;

	if (*pcmdqelmt != NULL) {
		cmdq->head = cmdq->head->next;
		cmdq->size--;

		if (cmdq->size == 0)
			cmdq->tail = NULL;
	}
}

static VOID free_hwcmd(os_kref *ref)
{
	struct _HwCmdQElmt *cmd = container_of(ref, struct _HwCmdQElmt, refcnt);

	if (cmd->NeedWait)
		RTMP_OS_EXIT_COMPLETION(&cmd->ack_done);

	if (cmd->buffer != NULL) {
		os_free_mem(cmd->buffer);
		cmd->buffer = NULL;
	}
	os_free_mem(cmd);
}

static VOID HwCtrlCmdHandler(RTMP_ADAPTER *pAd)
{
	PHwCmdQElmt	cmdqelmt;
	NDIS_STATUS	NdisStatus = NDIS_STATUS_SUCCESS;
	NTSTATUS		ntStatus;
	HwCmdHdlr		Handler = NULL;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	while (pAd && pHwCtrl->HwCtrlQ.size > 0) {
		NdisStatus = NDIS_STATUS_SUCCESS;
		NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &cmdqelmt);
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

		if (cmdqelmt == NULL)
			break;


		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) && RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)) {
			Handler = HwCtrlValidCmd(cmdqelmt);

			if (Handler) {
				ntStatus = Handler(pAd, cmdqelmt);

				if (cmdqelmt->CallbackFun)
					cmdqelmt->CallbackFun(pAd, cmdqelmt->CallbackArgs);
			}
		}
#ifdef DBG_STARVATION
		starv_dbg_put(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/

		/*complete*/
		if (cmdqelmt->NeedWait)
			RTMP_OS_COMPLETE(&cmdqelmt->ack_done);

		os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	}	/* end of while */
}


static VOID HwCtrlFlagHandler(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T		*pHwCtrl = &pAd->HwCtrl;
	PHwFlagCtrl		pHwCtrlFlag = &pHwCtrl->HwCtrlFlag;
	NTSTATUS		ntStatus;
	HwFlagHdlr		Handler = NULL;
	HwFlagCtrl      HwCtrlFlag;

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	HwCtrlFlag.FlagId = pHwCtrlFlag->FlagId;
	HwCtrlFlag.IsFlagSet = pHwCtrlFlag->IsFlagSet;
	pHwCtrlFlag->FlagId = 0;
	pHwCtrlFlag->IsFlagSet = 0;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		while (HwCtrlFlag.FlagId) {
			Handler = HwCtrlValidFlag(&HwCtrlFlag);

			if (Handler)
				ntStatus = Handler(pAd);
		}
	}
}


static INT HwCtrlThread(ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	HwCmdQElmt	*pCmdQElmt = NULL;
	HW_CTRL_T *pHwCtrl;
	int status;

	status = 0;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
		return 0;

	pHwCtrl = &pAd->HwCtrl;
	RtmpOSTaskCustomize(pTask);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);

	while (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_RUNNING) {
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pHwCtrl->HwCtrlQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		/*every time check command formate event*/
		HwCtrlCmdHandler(pAd);

		/*support flag type*/
		if (pHwCtrl->HwCtrlFlag.IsFlagSet)
			HwCtrlFlagHandler(pAd);

		pHwCtrl->TotalCnt++;
	}

	/* Clear the CmdQElements. */
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlQ.CmdQState = RTMP_TASK_STAT_STOPED;

	while (pHwCtrl->HwCtrlQ.size) {
		HwCtrlDequeueCmd(&pHwCtrl->HwCtrlQ, &pCmdQElmt);

		if (pCmdQElmt) {
#ifdef DBG_STARVATION
			starv_dbg_put(&pCmdQElmt->starv);
#endif /*DBG_STARVATION*/
			/*complete*/
			if (pCmdQElmt->NeedWait)
				RTMP_OS_COMPLETE(&pCmdQElmt->ack_done);

			os_kref_put(&pCmdQElmt->refcnt, free_hwcmd);
		}
	}

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("<---%s\n", __func__));
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}


#ifdef ERR_RECOVERY
static INT ser_ctrl_task(ULONG context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *task;
	HW_CTRL_T *hw_ctrl;
	int status = 0;

	task = (RTMP_OS_TASK *)context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(task);

	if (pAd == NULL)
		return 0;

	hw_ctrl = &pAd->HwCtrl;
	RtmpOSTaskCustomize(task);
	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);

	while (task && !RTMP_OS_TASK_IS_KILLED(task)) {
		if (RtmpOSTaskWait(pAd, task, &status) == FALSE)
			break;

		HwRecoveryFromError(pAd);
	}

	NdisAcquireSpinLock(&hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_UNKNOWN;
	NdisReleaseSpinLock(&hw_ctrl->ser_lock);
	status = RtmpOSTaskNotifyToExit(task);
	return status;
}


INT ser_init(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	RTMP_OS_TASK *task = &hw_ctrl->ser_task;

	NdisAllocateSpinLock(pAd, &hw_ctrl->ser_lock);
	hw_ctrl->ser_func_state = RTMP_TASK_STAT_INITED;
	RTMP_OS_TASK_INIT(task, "ser_task", pAd);
	Status = RtmpOSTaskAttach(task, ser_ctrl_task, (ULONG)task);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: unable to start %s\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), __func__));
		return NDIS_STATUS_FAILURE;
	}

	return TRUE;
}


INT ser_exit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *hw_ctrl = &pAd->HwCtrl;
	/*kill task*/
	ret = RtmpOSTaskKill(&hw_ctrl->ser_task);
	NdisFreeSpinLock(&hw_ctrl->ser_lock);
	return ret;
}
#endif /* ERR_RECOVERY */


#ifdef DBG_STARVATION
static void hwctrl_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct _HwCmdQElmt *cmd = container_of(starv, struct _HwCmdQElmt, starv);
	struct _HW_CTRL_T *hw_ctrl = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (log) {
		log->qsize = hw_ctrl->HwCtrlQ.size;
		log->id = cmd->command;
		entry->log = log;
	}
}

static void hwctrl_starv_block_init(struct starv_log *ctrl, struct _HW_CTRL_T *hw_ctrl)
{
	struct starv_dbg_block *block = &hw_ctrl->block;

	strncpy(block->name, "hwctrl", sizeof(block->name));
	block->priv = hw_ctrl;
	block->ctrl = ctrl;
	block->timeout = 100;
	block->timeout_fn = hwctrl_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}

#endif /*DBG_STARVATION*/

/*
	========================================================================

	Routine Description:

	Arguments:

	Return Value:

	IRQL =

	Note:

	========================================================================
*/
UINT32 HwCtrlInit(RTMP_ADAPTER *pAd)
{
	INT Status = 0;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HwCmdQ *cmdq = &pHwCtrl->HwCtrlQ;
	RTMP_OS_TASK *pTask = &pHwCtrl->HwCtrlTask;

#ifdef DBG_STARVATION
	hwctrl_starv_block_init(&pAd->starv_log_ctrl, pHwCtrl);
#endif /*DBG_STARVATION*/
	NdisAllocateSpinLock(pAd, &pHwCtrl->HwCtrlQLock);
	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	cmdq->head = NULL;
	cmdq->tail = NULL;
	cmdq->size = 0;
	cmdq->CmdQState = RTMP_TASK_STAT_INITED;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->TotalCnt = 0;
	pTask = &pHwCtrl->HwCtrlTask;
	RTMP_OS_TASK_INIT(pTask, "HwCtrlTask", pAd);
	Status = RtmpOSTaskAttach(pTask, HwCtrlThread, (ULONG)pTask);

	if (Status == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unable to start %s\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev), __func__));
		return NDIS_STATUS_FAILURE;
	}

#ifdef ERR_RECOVERY
	Status = ser_init(pAd);
#endif /* ERR_RECOVERY */
	return NDIS_STATUS_SUCCESS;
}


VOID HwCtrlExit(RTMP_ADAPTER *pAd)
{
	INT32 ret;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;
	/*flush all queued command*/
	HwCtrlCmdHandler(pAd);
	/*kill task*/
	ret = RtmpOSTaskKill(&pHwCtrl->HwCtrlTask);
	NdisFreeSpinLock(&pHwCtrl->HwCtrlQLock);
	hwctrl_ops->wifi_sys_open = NULL;
	hwctrl_ops->wifi_sys_close = NULL;
	hwctrl_ops->wifi_sys_link_up = NULL;
	hwctrl_ops->wifi_sys_link_down = NULL;
	hwctrl_ops->wifi_sys_connt_act = NULL;
	hwctrl_ops->wifi_sys_disconnt_act = NULL;
	hwctrl_ops->wifi_sys_peer_update = NULL;
#ifdef ERR_RECOVERY
	ret = ser_exit(pAd);
#endif /* ERR_RECOVERY */
#ifdef DBG_STARVATION
	unregister_starv_block(&pHwCtrl->block);
#endif /*DBG_STARVATION*/
}

NDIS_STATUS HwCtrlEnqueueCmd(
	RTMP_ADAPTER *pAd,
	HW_CTRL_TXD HwCtrlTxd)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	PHwCmdQElmt	cmdqelmt = NULL;
	PHwCmdQ	cmdq = NULL;
	UINT32 wait_time = 0;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s - NIC is not exist!!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt, sizeof(HwCmdQElmt));

	if (cmdqelmt == NULL)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(cmdqelmt, sizeof(HwCmdQElmt));
	/*initial lock*/
	NdisAllocateSpinLock(NULL, &cmdqelmt->lock);
	/*creat wait */
	cmdqelmt->NeedWait = HwCtrlTxd.NeedWait;
	/*initial stravation dbg*/
#ifdef DBG_STARVATION
	starv_dbg_init(&pHwCtrl->block, &cmdqelmt->starv);
#endif /*DBG_STARVATION*/

	if (HwCtrlTxd.NeedWait)
		RTMP_OS_INIT_COMPLETION(&cmdqelmt->ack_done);

	if (HwCtrlTxd.InformationBufferLength > 0) {
		status = os_alloc_mem(pAd, (PUCHAR *)&cmdqelmt->buffer, HwCtrlTxd.InformationBufferLength);
		if (cmdqelmt->buffer == NULL) {
			status =  NDIS_STATUS_RESOURCES;
			goto end;
		}
		/*initial buffer*/
		os_move_mem(cmdqelmt->buffer, HwCtrlTxd.pInformationBuffer, HwCtrlTxd.InformationBufferLength);
		cmdqelmt->bufferlength = HwCtrlTxd.InformationBufferLength;
	}
	/*initial cmd element*/
	cmdqelmt->command = HwCtrlTxd.CmdId;
	cmdqelmt->type = HwCtrlTxd.CmdType;
	cmdqelmt->RspBuffer = HwCtrlTxd.pRespBuffer;
	cmdqelmt->RspBufferLen = HwCtrlTxd.RespBufferLength;
	cmdqelmt->CallbackFun = HwCtrlTxd.CallbackFun;
	cmdqelmt->CallbackArgs = HwCtrlTxd.CallbackArgs;

	/*create reference count*/
	os_kref_init(&cmdqelmt->refcnt);

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	/*check queue status*/
	if (!(pHwCtrl->HwCtrlQ.CmdQState & RTMP_TASK_CAN_DO_INSERT)) {
		NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
		status = NDIS_STATUS_FAILURE;
		goto end;
	}
	/*add reference count for cmd due to send to thread*/
	os_kref_get(&cmdqelmt->refcnt);

	/*enqueue to cmdq*/
	cmdq = &pHwCtrl->HwCtrlQ;

	if (cmdq->size == 0)
		cmdq->head = cmdqelmt;
	else
		cmdq->tail->next = cmdqelmt;

	cmdq->tail = cmdqelmt;
	cmdqelmt->next = NULL;
	cmdq->size++;

	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	/*get stravation */
#ifdef DBG_STARVATION
	starv_dbg_get(&cmdqelmt->starv);
#endif /*DBG_STARVATION*/
	RTCMDUp(&pHwCtrl->HwCtrlTask);

	/*not need wait, goto end directly*/
	if (!HwCtrlTxd.NeedWait)
		goto end;

	/*wait handle*/
	wait_time = HwCtrlTxd.wait_time ? HwCtrlTxd.wait_time : HWCTRL_CMD_TIMEOUT;
	if (!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&cmdqelmt->ack_done, RTMPMsecsToJiffies(wait_time))) {
		status = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): HwCtrl CmdTimeout, TYPE:%d,ID:%d!!\n",
			__func__, cmdqelmt->type, cmdqelmt->command));
	}
end:
	os_kref_put(&cmdqelmt->refcnt, free_hwcmd);
	return status;
}

NDIS_STATUS HwCtrlSetFlag(
	RTMP_ADAPTER	*pAd,
	INT32			FlagId)
{
	NDIS_STATUS	status = NDIS_STATUS_SUCCESS;
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s - NIC is not exist!!\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	pHwCtrl->HwCtrlFlag.IsFlagSet = TRUE;
	pHwCtrl->HwCtrlFlag.FlagId |= FlagId;
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	/*Send command*/
	RTCMDUp(&pHwCtrl->HwCtrlTask);
	return status;
}



/*
*
*/
INT Show_HwCtrlStatistic_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HW_CMD_TABLE_T *pHwCmdTable = NULL;
	UCHAR i = 0, j = 0;
	PHwCmdQElmt	cmdqelmt = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask Totaol Ref. Cnt: %d\n", pHwCtrl->TotalCnt));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask CMD Statistic:\n"));
	pHwCmdTable = HwCmdTable[i];

	while (pHwCmdTable != NULL) {
		j = 0;

		while (pHwCmdTable[j].CmdID != HWCMD_ID_END) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCMDID: %d, Handler: %p, RfCnt: %d\n",
					 pHwCmdTable[j].CmdID, pHwCmdTable[j].CmdHdlr, pHwCmdTable[j].RfCnt));
			j++;
		}

		pHwCmdTable = HwCmdTable[++i];
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tHwCtrlTask Flag Statistic:\n"));
	i = 0;

	while (HwFlagTable[i].FlagId != HWFLAG_ID_END) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFLAGID: %d, Handler: %p, RfCnt: %d\n",
				 HwFlagTable[i].FlagId, HwFlagTable[i].FlagHdlr, HwFlagTable[i].RfCnt));
		i++;
	}

	NdisAcquireSpinLock(&pHwCtrl->HwCtrlQLock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tQueSize: %d\n",
		pHwCtrl->HwCtrlQ.size));
	cmdqelmt = pHwCtrl->HwCtrlQ.head;
	while (cmdqelmt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTYPE:%d, CID:%d\n",
			cmdqelmt->type, cmdqelmt->command));
		cmdqelmt = cmdqelmt->next;
	}
	NdisReleaseSpinLock(&pHwCtrl->HwCtrlQLock);
	return TRUE;
}

UINT32 HWCtrlOpsReg(RTMP_ADAPTER *pAd)
{
	HW_CTRL_T *pHwCtrl = &pAd->HwCtrl;
	HWCTRL_OP *hwctrl_ops = &pHwCtrl->hwctrl_ops;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/*hook funcion*/
	switch (cap->hw_ops_ver) {
#ifdef WIFI_SYS_FW_V2

	case HWCTRL_OP_TYPE_V2:
		hw_ctrl_ops_v2_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V2*/
#ifdef WIFI_SYS_FW_V1

	case HWCTRL_OP_TYPE_V1:
	default:
		hw_ctrl_ops_v1_register(hwctrl_ops);
		break;
#endif /*WIFI_SYS_FW_V1*/
	}

	return NDIS_STATUS_SUCCESS;
}


