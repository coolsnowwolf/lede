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

	Module Name: whnat
	woe_ser.c
*/

/*ser detect related file*/
#ifdef ERR_RECOVERY

#include <linux/kthread.h>

#include "woe.h"
#include "wed_def.h"

#define WED_THREAD_NAME "wed_task"
#define WED_THREAD_PERIOD 100
#define WED_MAX_ERR_CNT 3
#define WED_RECYCLE_DIFF(_period_time) (_period_time * 10)
#define WED_SER_RECOVERY_DEFAULT FALSE

/*
*
*/
static int wed_ser_check(struct wed_entry *wed)
{
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;

	if (ser_ctrl->wpdma_idle_cnt > WED_MAX_ERR_CNT &&
		ser_ctrl->tx_dma_err_cnt > WED_MAX_ERR_CNT &&
		ser_ctrl->wdma_err_cnt > WED_MAX_ERR_CNT) {

		ser_ctrl->wpdma_ser_cnt++;
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed_wpdma_tx_drv error !\n", __func__);
		return TRUE;
	}

	if (ser_ctrl->tx_bm_err_cnt > WED_MAX_ERR_CNT) {

		ser_ctrl->tx_bm_ser_cnt++;
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed_tx_bm error !\n", __func__);
		return TRUE;
	}
	return FALSE;
}

#ifdef WED_TX_SUPPORT
/*
*
*/
void wed_ser_err_cnt_wpdma_update(
	struct wed_ser_ctrl *ser_ctrl,
	struct wed_ser_state *new_state)
{
	struct wed_ser_state *state = &ser_ctrl->state;
	/*check WED_WPDMA_TX_DRV*/
	if (state->wpdma_stat == 0 && new_state->wpdma_stat == 0) {
		if ((state->wpdma_tx0_mib == new_state->wpdma_tx0_mib) &&
			(state->wpdma_tx1_mib == new_state->wpdma_tx1_mib)) {
			ser_ctrl->wpdma_idle_cnt++;
		} else {			
			ser_ctrl->wpdma_idle_cnt = 0;
		}
	} else {
		ser_ctrl->wpdma_idle_cnt = 0;
	}
}

/*
*
*/
void wed_ser_err_cnt_txdma_update(
	struct wed_ser_ctrl *ser_ctrl,
	struct wed_ser_state *new_state)
{
	struct wed_ser_state *state = &ser_ctrl->state;
	/*check WED_TX_DMA*/
	if ((state->tx_dma_stat == 0xF && new_state->tx_dma_stat == 0xF) &&
		((state->tx0_mib == new_state->tx0_mib) &&
		(state->tx1_mib == new_state->tx1_mib)))
		ser_ctrl->tx_dma_err_cnt++;
	else if ((state->tx_dma_stat == 0 && new_state->tx_dma_stat == 0) &&
		((state->tx0_mib == new_state->tx0_mib) &&
		(state->tx1_mib == new_state->tx1_mib)) &&
		((new_state->tx0_cidx != new_state->tx0_didx) ||
		(new_state->tx1_cidx != new_state->tx1_didx)))
		ser_ctrl->tx_dma_err_cnt++;
	else
		ser_ctrl->tx_dma_err_cnt = 0;
}

#ifdef WED_HW_TX_SUPPORT
/*
*
*/
void wed_ser_err_cnt_wdma_update(
	struct wed_ser_ctrl *ser_ctrl,
	struct wed_ser_state *new_state)
{
	struct wed_ser_state *state = &ser_ctrl->state;
	/*check WED_WDMA_DRV*/
	if ((state->wdma_stat == 0x8 && new_state->wdma_stat == 0x8) ||
		(state->wdma_stat == 0x5 && new_state->wdma_stat == 0x5)) {
		if ((state->wdma_rx0_mib == new_state->wdma_rx0_mib) &&
			(state->wdma_rx1_mib == new_state->wdma_rx1_mib)) {
			ser_ctrl->wdma_err_cnt++;
		} else {
			ser_ctrl->wdma_err_cnt = 0;
		}
	} else {
		ser_ctrl->wdma_err_cnt = 0;
	}
}

/*
*
*/
void wed_ser_err_cnt_bm_update(
	struct wed_ser_ctrl *ser_ctrl,
	struct wed_ser_state *new_state)
{
	struct wed_ser_state *state = &ser_ctrl->state;
	/*check TX_BM*/
#ifdef WED_WDMA_RECYCLE
	unsigned int recy_diff = WED_RECYCLE_DIFF(ser_ctrl->period_time);
	if (((new_state->wdma_rx0_recycle_mib - state->wdma_rx0_recycle_mib) <= recy_diff) &&
		((new_state->wdma_rx1_recycle_mib - state->wdma_rx1_recycle_mib) <= recy_diff))
		goto normal;
#else
	if (state->wdma_stat != 0x9 || new_state->wdma_stat != 0x9)
		goto normal;

	if ((state->wdma_rx0_mib != new_state->wdma_rx0_mib) ||
			(state->wdma_rx1_mib != new_state->wdma_rx1_mib))
		goto normal;
#endif /*WED_WDMA_RECYCLE*/

	if (new_state->bm_tx_stat != 0)
		goto normal;

	if ((state->txbm_to_wdma_mib != new_state->txbm_to_wdma_mib) ||
		(state->txfree_to_bm_mib != new_state->txfree_to_bm_mib))
		goto normal;

	/*error ocured*/
	ser_ctrl->tx_bm_err_cnt++;
	return;
normal:
	ser_ctrl->tx_bm_err_cnt = 0;
}
#endif /*WED_HW_TX_SUPPORT*/
#endif /*WED_TX_SUPPORT*/
/*
*
*/
static void wed_ser_error_cnt_update(
	struct wed_entry *wed,
	struct wed_ser_state *new_state)
{
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_state *state = &ser_ctrl->state;

	spin_lock(&ser_ctrl->ser_lock);
#ifdef WED_TX_SUPPORT
	wed_ser_err_cnt_wpdma_update(ser_ctrl, new_state);
	wed_ser_err_cnt_txdma_update(ser_ctrl, new_state);
#ifdef WED_HW_TX_SUPPORT
	wed_ser_err_cnt_wdma_update(ser_ctrl, new_state);
	wed_ser_err_cnt_bm_update(ser_ctrl, new_state);
#endif /*WED_HW_TX_SUPPORT*/
#endif /*WED_TX_SUPPORT*/

	/*update new value to ser ctrl*/
	memcpy(state, new_state, sizeof(struct wed_ser_state));
	spin_unlock(&ser_ctrl->ser_lock);
}

/*
*
*/
static void wed_ser_detect(struct wed_entry *wed)
{
	struct wed_ser_state state;

	memset(&state, 0, sizeof(state));
	/*update status*/
	whnat_hal_ser_update(wed, &state);

	/*update error count*/
	wed_ser_error_cnt_update(wed, &state);

	/*check ser should trigger or not*/
	if (wed_ser_check(wed) == TRUE) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed_ser_detect!!!, wed->irq=%d\n", __func__, wed->irq);
		if (wed->ser_ctrl.recovery == TRUE) {
			whnat_hal_ser_trigger((struct whnat_entry *)wed->whnat);
		} else {
			wed_ser_dump(wed);
			wed->ser_ctrl.period_time = 1000;
			WHNAT_DBG(WHNAT_DBG_OFF, "%s(): not recovery ~, delay periodic check to 1 sec for debug!", __func__);
		}
	}
	return ;
}

/*
*
*/
static int wed_ser_task(void *data)
{
	struct wed_entry *wed = data;
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;

	while (!kthread_should_stop()) {
		wed_ser_detect(wed);
		msleep(ser_ctrl->period_time);
	}
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): wed_ser exist, wed->irq=%d!\n", __func__, wed->irq);
	return 0;
}

/*
*
*/
void wed_ser_dump(struct wed_entry *wed)
{
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct wed_ser_state *state = &ser_ctrl->state;

	WHNAT_DBG(WHNAT_DBG_OFF, "======wed ser status========\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "wpdma_tx_drv_ser_cnt\t:%d\n", ser_ctrl->wpdma_ser_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_ser_cnt\t:%d\n", ser_ctrl->wdma_ser_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_ser_cnt\t:%d\n", ser_ctrl->tx_dma_ser_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "bm_tx_ser_cnt\t\t:%d\n", ser_ctrl->tx_bm_ser_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "======current ser indicate========\n");
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_err_cnt\t:%d\n", ser_ctrl->tx_dma_err_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_err_cnt\t:%d\n", ser_ctrl->wdma_err_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "wpdma_tx_drv_idle_cnt\t:%d\n", ser_ctrl->wpdma_idle_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "bm_err_cnt\t\t:%d\n", ser_ctrl->tx_bm_err_cnt);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_state\t:%d\n", state->tx_dma_stat);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx0_mib\t:%d\n", state->tx0_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx0_cidx\t:%d\n", state->tx0_cidx);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx0_didx\t:%d\n", state->tx0_didx);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx1_mib\t:%d\n", state->tx1_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx1_cidx\t:%d\n", state->tx1_cidx);
	WHNAT_DBG(WHNAT_DBG_OFF, "wed_tx_dma_tx1_didx\t:%d\n", state->tx1_didx);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_state\t:%d\n", state->wdma_stat);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_rx0_mib\t:%d\n", state->wdma_rx0_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_rx1_mib\t:%d\n", state->wdma_rx1_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_rx0_rc_mib\t:%d\n", state->wdma_rx0_recycle_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wdma_rx_drv_rx1_rc_mib\t:%d\n", state->wdma_rx1_recycle_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wpdma_tx_drv_state\t:%d\n", state->wpdma_stat);
	WHNAT_DBG(WHNAT_DBG_OFF, "wpdma_tx_drv_tx0_mib\t:%d\n", state->wpdma_tx0_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "wpdma_tx_drv_tx0_mib\t:%d\n", state->wpdma_tx1_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "bm_tx_state\t\t:%d\n", state->bm_tx_stat);
	WHNAT_DBG(WHNAT_DBG_OFF, "txfree_to_bm_mib\t:%d\n", state->txfree_to_bm_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "txbm_to_wdma_mib\t:%d\n", state->txbm_to_wdma_mib);
	WHNAT_DBG(WHNAT_DBG_OFF, "txbm_to_wdma_diff\t:%d\n",
		(state->txbm_to_wdma_mib - state->txfree_to_bm_mib));
}

/*
*
*/
int wed_ser_init(struct wed_entry *wed)
{
	int ret = 0;
	char name[32] = "";
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;
	struct whnat_entry *whnat = wed->whnat;

	snprintf(name, sizeof(name), "%s%d", WED_THREAD_NAME, whnat->idx);
	ser_ctrl->ser_task = kthread_create(wed_ser_task, wed, name);
	if (IS_ERR(ser_ctrl->ser_task)) {
		ser_ctrl->ser_task = NULL;
		goto err;
	}

	ser_ctrl->tx_dma_err_cnt = 0;
	ser_ctrl->tx_dma_ser_cnt = 0;
	ser_ctrl->wdma_err_cnt = 0;
	ser_ctrl->wdma_ser_cnt = 0;
	ser_ctrl->wpdma_idle_cnt = 0;
	ser_ctrl->wpdma_ser_cnt = 0;
	ser_ctrl->tx_bm_err_cnt = 0;
	ser_ctrl->tx_bm_ser_cnt = 0;
	ser_ctrl->recovery = WED_SER_RECOVERY_DEFAULT;
	ser_ctrl->period_time = WED_THREAD_PERIOD;
	memset(&ser_ctrl->state, 0, sizeof(ser_ctrl->state));
	spin_lock_init(&ser_ctrl->ser_lock);
	wake_up_process(ser_ctrl->ser_task);
	return 0;
err:
	return ret;
}

/*
*
*/
void wed_ser_exit(struct wed_entry *wed)
{
	struct wed_ser_ctrl *ser_ctrl = &wed->ser_ctrl;

	kthread_stop(ser_ctrl->ser_task);
	spin_lock_init(&ser_ctrl->ser_lock);
	memset(&ser_ctrl->state, 0, sizeof(ser_ctrl->state));
}

#endif /*ERR_RECOVERY*/
