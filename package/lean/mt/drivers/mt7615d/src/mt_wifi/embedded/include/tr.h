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

#ifndef __TR_H__
#define __TR_H__

struct tx_delay_control {
	RALINK_TIMER_STRUCT que_agg_timer;
	BOOLEAN que_agg_timer_running;
	BOOLEAN que_agg_en;
	BOOLEAN force_deq;
#define TX_BATCH_CNT 4
	UINT32 tx_process_batch_cnt;
#define MIN_AGG_PKT_LEN 58
#define MAX_AGG_PKT_LEN 135
#define MAX_AGG_EN_TP 700
#define MIN_AGG_EN_TP 50
	UINT32 min_pkt_len;
	UINT32 max_pkt_len;
#define QUE_AGG_TIMEOUT 4000
	UINT32 que_agg_timeout_value;
};

struct dly_ctl_cfg {
	UINT32 avg_tp;
	UINT32 dly_cfg;
};

struct tr_delay_control {
	RALINK_TIMER_STRUCT rx_delay_timer;
	BOOLEAN rx_delay_timer_running;
	BOOLEAN rx_delay_en;
	struct dly_ctl_cfg *dl_rx_dly_ctl_tbl;
	UINT32 dl_rx_dly_ctl_tbl_size;
	struct dly_ctl_cfg *ul_rx_dly_ctl_tbl;
	UINT32 ul_rx_dly_ctl_tbl_size;
	BOOLEAN tx_delay_en;
};

#define TP_DBG_TIME_SLOT_NUMS 10
struct tp_debug {
	UINT32 IsrTxDlyCnt;
	UINT32 IsrTxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrTxCnt;
	UINT32 IsrTxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxDlyCnt;
	UINT32 IsrRxDlyCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRxCnt;
	UINT32 IsrRxCntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IsrRx1Cnt;
	UINT32 IsrRx1CntRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadTx;
	UINT32 IoReadTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteTx;
	UINT32 IoWriteTxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx;
	UINT32 IoReadRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoReadRx1;
	UINT32 IoReadRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx;
	UINT32 IoWriteRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 IoWriteRx1;
	UINT32 IoWriteRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx;
	UINT32 MaxProcessCntRxRec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 MaxProcessCntRx1;
	UINT32 MaxProcessCntRx1Rec[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntA;
	UINT32 MaxProcessCntRxRecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntB;
	UINT32 MaxProcessCntRxRecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntC;
	UINT32 MaxProcessCntRxRecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxMaxProcessCntD;
	UINT32 MaxProcessCntRxRecD[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntA;
	UINT32 MaxProcessCntRx1RecA[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntB;
	UINT32 MaxProcessCntRx1RecB[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntC;
	UINT32 MaxProcessCntRx1RecC[TP_DBG_TIME_SLOT_NUMS];
	UINT32 Rx1MaxProcessCntD;
	UINT32 MaxProcessCntRx1RecD[TP_DBG_TIME_SLOT_NUMS];
	UINT32 RxDropPacket;
	UINT16 time_slot;
	RALINK_TIMER_STRUCT tp_dbg_history_timer;
};

struct tr_flow_control {
	UINT8 *TxFlowBlockState;
	NDIS_SPIN_LOCK *TxBlockLock;
	DL_LIST *TxBlockDevList;
	BOOLEAN IsTxBlocked;
	UINT8 RxFlowBlockState;
};

struct tx_rx_ctl {
	struct tr_flow_control tr_flow_ctl;
	struct tx_delay_control tx_delay_ctl;
	UINT32 tx_sw_q_drop;
	UINT32 net_if_stop_cnt;
#ifdef CONFIG_TP_DBG
	struct tp_debug tp_dbg;
#endif
	struct tr_delay_control tr_delay_ctl;
	UINT32 rx_icv_err_cnt;
#ifdef CONFIG_RECOVERY_ON_INTERRUPT_MISS
	ULONG total_int_count;
	ULONG rx_data_int_count;
	ULONG rx_cmd_int_count;
#endif
};


struct _RTMP_ADAPTER;

enum {
	NO_ENOUGH_SWQ_SPACE = (1 << 0),
};

INT32 tr_ctl_init(struct _RTMP_ADAPTER *pAd);
INT32 tr_ctl_exit(struct _RTMP_ADAPTER *pAd);
BOOLEAN tx_flow_check_state(struct _RTMP_ADAPTER *pAd, UINT8 State, UINT8 RingIdx);
INT32 tx_flow_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
INT32 tx_flow_set_state_block(struct _RTMP_ADAPTER *pAd, PNET_DEV NetDev, UINT8 State, BOOLEAN Block, UINT8 RingIdx);
#endif
