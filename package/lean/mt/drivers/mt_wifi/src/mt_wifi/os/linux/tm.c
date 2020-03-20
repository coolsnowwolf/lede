/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2017, MediaTek, Inc.
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

#ifndef CONFIG_PROPRIETARY_DRIVER

static VOID ge_tx_pkt_deq_work(struct work_struct *work)
{
	RTMP_ADAPTER *pAd = container_of(work, struct _RTMP_ADAPTER, tx_deq_work);

	ge_tx_pkt_deq_func(pAd);
}

static VOID fp_tx_pkt_deq_work(struct work_struct *work)
{
	RTMP_ADAPTER *pAd = container_of(work, struct _RTMP_ADAPTER, tx_deq_work);

	fp_tx_pkt_deq_func(pAd);
}

static VOID fp_fair_tx_pkt_deq_work(struct work_struct *work)
{
	RTMP_ADAPTER *pAd = container_of(work, struct _RTMP_ADAPTER, tx_deq_work);

	fp_fair_tx_pkt_deq_func(pAd);
}

static INT tm_wq_qm_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pAd->tx_dequeue_scheduable = TRUE;

#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	pAd->qm_wq = alloc_workqueue("qm_wq", WQ_MEM_RECLAIM | WQ_UNBOUND, 0);
#else
	pAd->qm_wq = create_singlethread_workqueue("qm_wq");
#endif

	if (cap->qm == FAST_PATH_QM) {
		INIT_WORK(&pAd->tx_deq_work, fp_tx_pkt_deq_work);
	} else if (cap->qm == FAST_PATH_FAIR_QM) {
		INIT_WORK(&pAd->tx_deq_work, fp_fair_tx_pkt_deq_work);
	} else if ((cap->qm == GENERIC_QM) || (cap->qm == GENERIC_FAIR_QM)) {
		INIT_WORK(&pAd->tx_deq_work, ge_tx_pkt_deq_work);
	}
	return ret;
}

static VOID tx_dma_done_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, tx_dma_done_work);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	tx_dma_done_func(pAd);
}

static VOID rx_done_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, rx_done_work[HIF_RX_IDX0]);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	rx_done_func(pAd);
}

static VOID rx1_done_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, rx_done_work[HIF_RX_IDX1]);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	rx1_done_func(pAd);
}

#ifdef ERR_RECOVERY
static VOID mt_mac_recovery_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, mac_recovery_work);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	mt_mac_recovery_func(pAd);
}
#endif

#ifdef CONFIG_FWOWN_SUPPORT
static VOID mt_mac_fw_own_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, mac_fw_own_work);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	mt_mac_fw_own_func(pAd);
}
#endif

#ifdef MULTI_LAYER_INTERRUPT
static VOID mt_subsys_int_work(struct work_struct *work)
{
	POS_COOKIE obj = container_of(work, struct os_cookie, subsys_int_work);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)((ULONG)(obj->pAd_va));

	mt_subsys_int_func(pAd);
}
#endif

static INT tm_wq_hif_pci_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
#if (KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE)
	obj->hif_wq = alloc_workqueue("hif_wq", WQ_MEM_RECLAIM | WQ_UNBOUND, 0);
#else
	obj->hif_wq = create_singlethread_workqueue("hif_wq");
#endif

	INIT_WORK(&obj->tx_dma_done_work, tx_dma_done_work);
	INIT_WORK(&obj->rx_done_work[HIF_RX_IDX0], rx_done_work);
	INIT_WORK(&obj->rx_done_work[HIF_RX_IDX1], rx1_done_work);

#ifdef ERR_RECOVERY
	INIT_WORK(&obj->mac_recovery_work, mt_mac_recovery_work);
#endif

#ifdef CONFIG_FWOWN_SUPPORT
	INIT_WORK(&obj->mac_fw_own_work, mt_mac_fw_own_work);
#endif

#ifdef MULTI_LAYER_INTERRUPT
	INIT_WORK(&obj->subsys_int_work, mt_subsys_int_work);
#endif

	return ret;
}

static INT tm_wq_hif_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		ret = tm_wq_hif_pci_init(pAd);
	}

	return ret;
}

static INT tm_wq_hif_pci_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	destroy_workqueue(obj->hif_wq);

	return ret;
}

static INT tm_wq_hif_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		ret = tm_wq_hif_pci_exit(pAd);
	}

	return ret;
}

static INT tm_wq_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	pAd->tx_dequeue_scheduable = FALSE;
	destroy_workqueue(pAd->qm_wq);

	return ret;
}

static INT tm_wq_qm_schedule_task(RTMP_ADAPTER *pAd, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (pAd->tx_dequeue_scheduable) {
			queue_work(pAd->qm_wq, &pAd->tx_deq_work);
		}
		break;

	default:
		break;

	}

	return ret;
}

static INT tm_wq_hif_schedule_task(RTMP_ADAPTER *pAd, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	switch (type) {
	case CMD_MSG_TASK:

		break;

	case TX_DONE_TASK:
		queue_work(obj->hif_wq, &obj->tx_dma_done_work);
		break;

	case RX_DONE_TASK:
		queue_work(obj->hif_wq, &obj->rx_done_work[HIF_RX_IDX0]);
		break;

	case RX1_DONE_TASK:
		queue_work(obj->hif_wq, &obj->rx_done_work[HIF_RX_IDX1]);
		break;

#ifdef ERR_RECOVERY
	case ERROR_RECOVERY_TASK:
		queue_work(obj->hif_wq, &obj->mac_recovery_work);
		break;
#endif

#ifdef CONFIG_FWOWN_SUPPORT
	case FW_OWN_TASK:
		queue_work(obj->hif_wq, &obj->mac_fw_own_work);
		break;
#endif

#ifdef MULTI_LAYER_INTERRUPT
	case SUBSYS_INT_TASK:
		queue_work(obj->hif_wq, &obj->subsys_int_work);
		break;
#endif

	default:

		break;
	}

	return ret;
}


static INT tm_wq_qm_schedule_task_on(RTMP_ADAPTER *pAd, INT cpu, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (pAd->tx_dequeue_scheduable) {
			queue_work_on(cpu, pAd->qm_wq, &pAd->tx_deq_work);
		}
		break;
	default:

		break;
	}

	return ret;
}

static INT tm_wq_hif_schedule_task_on(RTMP_ADAPTER *pAd, INT cpu, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	switch (type) {
	case CMD_MSG_TASK:

		break;

	case TX_DONE_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->tx_dma_done_work);
		break;

	case RX_DONE_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->rx_done_work[HIF_RX_IDX0]);
		break;

	case RX1_DONE_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->rx_done_work[HIF_RX_IDX1]);
		break;

#ifdef ERR_RECOVERY
	case ERROR_RECOVERY_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->mac_recovery_work);
		break;
#endif

#ifdef CONFIG_FWOWN_SUPPORT
	case FW_OWN_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->mac_fw_own_work);
		break;
#endif

#ifdef MULTI_LAYER_INTERRUPT
	case SUBSYS_INT_TASK:
		queue_work_on(cpu, obj->hif_wq, &obj->subsys_int_work);
		break;
#endif

	default:

		break;
	}

	return ret;
}

struct tm_ops tm_wq_qm_ops = {
	.init = tm_wq_qm_init,
	.exit = tm_wq_qm_exit,
	.schedule_task = tm_wq_qm_schedule_task,
	.schedule_task_on = tm_wq_qm_schedule_task_on,
};

struct tm_ops tm_wq_hif_ops = {
	.init = tm_wq_hif_init,
	.exit = tm_wq_hif_exit,
	.schedule_task = tm_wq_hif_schedule_task,
	.schedule_task_on = tm_wq_hif_schedule_task_on,
};
#endif /*CONFIG_PROPRIETARY_DRIVER*/

static VOID ge_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	ge_tx_pkt_deq_func(pAd);
}

static VOID fp_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_tx_pkt_deq_func(pAd);
}

static VOID fp_fair_tx_pkt_deq_tasklet(ULONG param)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)param;

	fp_fair_tx_pkt_deq_func(pAd);
}

static INT tm_tasklet_qm_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->qm == FAST_PATH_QM) {
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet, fp_tx_pkt_deq_tasklet, (unsigned long)pAd);
	} else if (cap->qm == FAST_PATH_FAIR_QM) {
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet, fp_fair_tx_pkt_deq_tasklet, (unsigned long)pAd);
	} else if ((cap->qm == GENERIC_QM) || (cap->qm == GENERIC_FAIR_QM)) {
		RTMP_OS_TASKLET_INIT(pAd, &pAd->tx_deque_tasklet, ge_tx_pkt_deq_tasklet, (unsigned long)pAd);
	}
	pAd->tx_dequeue_scheduable = TRUE;

	return ret;
}

static VOID tx_dma_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	tx_dma_done_func(pAd);
}

static VOID tr_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	tr_done_func(pAd);
}

static VOID rx_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	rx_done_func(pAd);
}

static VOID rx1_done_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	rx1_done_func(pAd);
}

#ifdef ERR_RECOVERY
static VOID mt_mac_recovery_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	mt_mac_recovery_func(pAd);
}
#endif

#ifdef CONFIG_FWOWN_SUPPORT
static VOID mt_mac_fw_own_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	mt_mac_fw_own_func(pAd);
}
#endif

#ifdef MULTI_LAYER_INTERRUPT
static VOID subsys_int_tasklet(unsigned long data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)data;

	mt_subsys_int_func(pAd);
}
#endif

static INT tm_tasklet_hif_pci_init(RTMP_ADAPTER *pAd)
{
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	RTMP_OS_TASKLET_INIT(pAd, &obj->rx_done_task[HIF_RX_IDX0], rx_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &obj->rx_done_task[HIF_RX_IDX1], rx1_done_tasklet, (unsigned long)pAd);

#ifdef ERR_RECOVERY
	RTMP_OS_TASKLET_INIT(pAd, &obj->mac_error_recovey_task, mt_mac_recovery_tasklet, (unsigned long)pAd);
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_OS_TASKLET_INIT(pAd, &obj->mt_mac_fw_own_task, mt_mac_fw_own_tasklet, (unsigned long)pAd);
#endif /* CONFIG_FWOWN_SUPPORT */

#ifdef MULTI_LAYER_INTERRUPT
	RTMP_OS_TASKLET_INIT(pAd, &obj->subsys_int_task, subsys_int_tasklet, (unsigned long)pAd);
#endif

	RTMP_OS_TASKLET_INIT(pAd, &obj->tx_dma_done_task, tx_dma_done_tasklet, (unsigned long)pAd);

	RTMP_OS_TASKLET_INIT(pAd, &obj->tr_done_task, tr_done_tasklet, (unsigned long)pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
	}
#endif /* CONFIG_AP_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

static INT tm_tasklet_hif_init(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		ret = tm_tasklet_hif_pci_init(pAd);
	}

	return ret;
}

static INT tm_tasklet_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	pAd->tx_dequeue_scheduable = FALSE;

	RTMP_OS_TASKLET_KILL(&pAd->tx_deque_tasklet);

	return ret;
}

static INT tm_tasklet_hif_pci_exit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;
	INT ret = NDIS_STATUS_SUCCESS;

	RTMP_OS_TASKLET_KILL(&obj->rx_done_task[HIF_RX_IDX0]);
	RTMP_OS_TASKLET_KILL(&obj->rx_done_task[HIF_RX_IDX1]);

#ifdef ERR_RECOVERY
	RTMP_OS_TASKLET_KILL(&obj->mac_error_recovey_task);
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_OS_TASKLET_KILL(&obj->mt_mac_fw_own_task);
#endif /* CONFIG_FWOWN_SUPPORT */

	RTMP_OS_TASKLET_KILL(&obj->tx_dma_done_task);
	RTMP_OS_TASKLET_KILL(&obj->tr_done_task);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	}
#endif /* CONFIG_AP_SUPPORT */

	return ret;
}

static INT tm_tasklet_hif_exit(RTMP_ADAPTER *pAd)
{
	INT ret = NDIS_STATUS_SUCCESS;

	if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd)) {
		ret = tm_tasklet_hif_pci_exit(pAd);
	}

	return ret;
}

static INT tm_tasklet_qm_schedule_task(RTMP_ADAPTER *pAd, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;

	switch (type) {
	case TX_DEQ_TASK:
		if (pAd->tx_dequeue_scheduable) {
			RTMP_OS_TASKLET_SCHE(&pAd->tx_deque_tasklet);
		}

		break;

	default:

		break;
	}

	return ret;
}

static INT tm_tasklet_hif_schedule_task(RTMP_ADAPTER *pAd, enum task_type type)
{
	INT ret = NDIS_STATUS_SUCCESS;
	POS_COOKIE obj = (POS_COOKIE)pAd->OS_Cookie;

	switch (type) {
	case CMD_MSG_TASK:

		break;

	case TX_DONE_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->tx_dma_done_task);
		break;

	case RX_DONE_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->rx_done_task[HIF_RX_IDX0]);
		break;

	case RX1_DONE_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->rx_done_task[HIF_RX_IDX1]);
		break;

#ifdef ERR_RECOVERY
	case ERROR_RECOVERY_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->mac_error_recovey_task);
		break;
#endif

#ifdef CONFIG_FWOWN_SUPPORT
	case FW_OWN_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->mt_mac_fw_own_task);
		break;
#endif

#ifdef MULTI_LAYER_INTERRUPT
	case SUBSYS_INT_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->subsys_int_task);
		break;
#endif

	case TR_DONE_TASK:
		RTMP_OS_TASKLET_SCHE(&obj->tr_done_task);
		break;

	default:

		break;
	}

	return ret;
}

struct tm_ops tm_tasklet_qm_ops = {
	.init = tm_tasklet_qm_init,
	.exit = tm_tasklet_qm_exit,
	.schedule_task = tm_tasklet_qm_schedule_task,
};

struct tm_ops tm_tasklet_hif_ops = {
	.init = tm_tasklet_hif_init,
	.exit = tm_tasklet_hif_exit,
	.schedule_task = tm_tasklet_hif_schedule_task,
};


INT tm_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct tm_ops **tm_qm_ops = &pAd->tm_qm_ops;
	struct tm_ops **tm_hif_ops = &pAd->tm_hif_ops;
	INT ret = NDIS_STATUS_SUCCESS;

	if (cap->qm_tm == TASKLET_METHOD)
		*tm_qm_ops = &tm_tasklet_qm_ops;
#ifndef CONFIG_PROPRIETARY_DRIVER
	else if (cap->qm_tm == CMWQ_METHOD)
		*tm_qm_ops = &tm_wq_qm_ops;
#endif /*CONFIG_PROPRIETARY_DRIVER*/
	if (cap->hif_tm == TASKLET_METHOD)
		*tm_hif_ops = &tm_tasklet_hif_ops;
#ifndef CONFIG_PROPRIETARY_DRIVER
	else if (cap->hif_tm == CMWQ_METHOD)
		*tm_hif_ops = &tm_wq_hif_ops;
#endif /*CONFIG_PROPRIETARY_DRIVER*/
	ret = (*tm_qm_ops)->init(pAd);
	ret = (*tm_hif_ops)->init(pAd);

	return ret;
}

INT tm_exit(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_qm_ops = pAd->tm_qm_ops;
	struct tm_ops *tm_hif_ops = pAd->tm_hif_ops;
	INT ret = NDIS_STATUS_SUCCESS;

	ret = tm_qm_ops->exit(pAd);
	ret = tm_hif_ops->exit(pAd);

	return ret;
}
