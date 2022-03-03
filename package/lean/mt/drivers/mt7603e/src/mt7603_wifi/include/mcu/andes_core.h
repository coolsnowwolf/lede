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
	andes_core.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __ANDES_CORE_H__
#define __ANDES_CORE_H__

#include "mcu.h"

#ifdef LINUX
#ifndef WORKQUEUE_BH
#include <linux/interrupt.h>
#endif
#endif /* LINUX */

#ifdef RLT_MAC
#include "mcu/andes_rlt.h"
#endif

#ifdef MT_MAC
#include "mcu/andes_mt.h"
#endif

#include "link_list.h"

struct _RTMP_ADAPTER;
struct _RXFCE_INFO;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;

enum cmd_msg_state {
	illegal,
	tx_start,
	tx_kickout_fail,
	tx_timeout_fail,
	tx_retransmit,
	tx_done,
	wait_cmd_out,
	wait_cmd_out_and_ack,
	wait_ack,
	rx_start,
	rx_receive_fail,
	rx_done,
#ifdef RTMP_SDIO_SUPPORT
	tx_sdio_ok,
	tx_sdio_fail,
#endif
};

enum {
	MCU_INIT,
	MCU_TX_HALT,
	MCU_RX_HALT,
};

enum {
	CMD_ACK,
};

enum cmd_msg_error_type {
	error_tx_kickout_fail,
	error_tx_timeout_fail,
	error_rx_receive_fail,
};

/*
 * Command response RX Ring selection
 */
enum RX_RING_ID {
	RX_RING0,
	RX_RING1,
};

struct MCU_CTRL {
	UINT8 cmd_seq;
	ULONG flags;
#ifdef LINUX
#ifndef WORKQUEUE_BH
	RTMP_NET_TASK_STRUCT cmd_msg_task;
#else
	struct tasklet_struct cmd_msg_task;
#endif
#endif /* LINUX */
	NDIS_SPIN_LOCK txq_lock;
	DL_LIST txq;
	NDIS_SPIN_LOCK rxq_lock;
	DL_LIST rxq;
	NDIS_SPIN_LOCK ackq_lock;
	DL_LIST ackq;
	NDIS_SPIN_LOCK kickq_lock;
	DL_LIST kickq;
	NDIS_SPIN_LOCK tx_doneq_lock;
	DL_LIST tx_doneq;
	NDIS_SPIN_LOCK rx_doneq_lock;
	DL_LIST rx_doneq;
#ifdef RTMP_SDIO_SUPPORT
	NDIS_SPIN_LOCK txq_sdio_lock;
	DL_LIST txq_sdio;
	UINT32 SDIOPadSize;
#endif
	ULONG tx_kickout_fail_count;
	ULONG tx_timeout_fail_count;
	ULONG rx_receive_fail_count;
	ULONG alloc_cmd_msg;
	ULONG free_cmd_msg;
	BOOLEAN power_on;
	BOOLEAN dpd_on;
	UINT8 RxStream0, RxStream1;
#ifdef MT_MAC
#define FW_NO_INIT 0
#define FW_DOWNLOAD 1
#define FW_RUN_TIME 2
#define ROM_PATCH_DOWNLOAD 3
	UINT16 Stage;
	UINT8 SemStatus;
#endif
	struct _RTMP_ADAPTER *ad;
};


struct cmd_msg;
typedef VOID (*MSG_RSP_HANDLER)(struct cmd_msg *msg, char *payload, UINT16 payload_len);
typedef VOID (*MSG_EVENT_HANDLER)(struct _RTMP_ADAPTER *ad, char *payload, UINT16 payload_len);

struct cmd_msg_cb {
	struct cmd_msg *msg;
};

#define CMD_MSG_CB(pkt) ((struct cmd_msg_cb *)(GET_OS_PKT_CB(pkt)))
#define CMD_MSG_RETRANSMIT_TIMES 3
#define CMD_MSG_TIMEOUT 3000

struct cmd_msg {
	DL_LIST list;
	UINT16 pq_id;
	UINT8 cmd_type;
	UINT8 set_query;
	UINT8 ext_cmd_type;
	UINT8 seq;
	UINT16 timeout;
	UINT16 rsp_payload_len;
	BOOLEAN need_wait;
	BOOLEAN need_rsp;
	BOOLEAN need_retransmit;
	RTMP_OS_COMPLETION ack_done;
	char *rsp_payload;
	MSG_RSP_HANDLER rsp_handler;
	enum cmd_msg_state state;
	VOID *priv;
	PNDIS_PACKET net_pkt;
	INT retransmit_times;
#ifdef RTMP_SDIO_SUPPORT
	RTMP_OS_COMPLETION tx_sdio_done;
	UINT16 tx_sdio_timeout;
#endif
	UINT32 wcid; /* Index of MacTableEntry */
};

enum BW_SETTING {
	BW20 = 1,
	BW40 = 2,
	BW10 = 4,
	BW80 = 8,
};

enum mcu_skb_state {
	ILLEAGAL = 0,
	MCU_CMD_START,
	MCU_CMD_DONE,
	MCU_RSP_START,
	MCU_RSP_DONE,
	MCU_RSP_CLEANUP,
	UNLINK_START,
};

#define CMD_QUERY 0
#define CMD_SET 1
#define CMD_NA 3

#define EXT_CMD_NA 0

#define P1_Q0 0x8000

#define USB_END_PADDING 4
#define SDIO_END_PADDING 4
#define PATCH_INFO_SIZE 30

struct mcu_skb_data {
	enum mcu_skb_state state;
};

#ifdef LED_CONTROL_SUPPORT
#ifdef RT_BIG_ENDIAN
typedef union _LED_ENHANCE {
	struct {
		UINT32 rsv:8;
		UINT32 on_time:8;
		UINT32 off_time:8;
		UINT32 idx:8;
	} field;
	UINT32 word;
} LED_ENHANCE;
#else
typedef union _LED_ENHANCE {
	struct {
		UINT32 idx:8;
		UINT32 off_time:8;
		UINT32 on_time:8;
		UINT32 rsv:8;
	} field;
	UINT32 word;
} LED_ENHANCE;
#endif /* RT_BIG_ENDIAN */
#endif /*LED_CONTROL_SUPPORT*/


VOID AndesCtrlInit(struct _RTMP_ADAPTER *pAd);
VOID AndesCtrlExit(struct _RTMP_ADAPTER *pAd);
INT32 AndesSendCmdMsg(struct _RTMP_ADAPTER *pAd, struct cmd_msg *msg);
BOOLEAN IsInbandCmdProcessing(struct _RTMP_ADAPTER *pAd);
VOID AndesCmdMsgBh(unsigned long param);
INT32 UsbRxCmdMsgSubmit(struct _RTMP_ADAPTER *pAd);
INT32 UsbRxCmdMsgsReceive(struct _RTMP_ADAPTER *pAd);
VOID AndesBhSchedule(struct _RTMP_ADAPTER *pAd);

struct cmd_msg *AndesAllocCmdMsg(struct _RTMP_ADAPTER *pAd, unsigned int length);

VOID AndesInitCmdMsg(struct cmd_msg *msg, UINT16 pq_id, UINT8 cmd_type,
			UINT8 set_query, UINT8 ExtCmdType, BOOLEAN need_wait,
			UINT16 timeout, BOOLEAN need_retransmit, BOOLEAN need_rsp,
			UINT16 rsp_payload_len, char *rsp_payload,
			MSG_RSP_HANDLER rsp_handler);

VOID AndesAppendCmdMsg(struct cmd_msg *msg, char *data, unsigned int len);
VOID AndesFreeCmdMsg(struct cmd_msg *msg);
VOID AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state);
VOID AndesIncErrorCount(struct MCU_CTRL *ctl, enum cmd_msg_error_type type);
VOID _AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list);
VOID AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list);
struct cmd_msg *AndesDequeueCmdMsg(struct MCU_CTRL *ctl, DL_LIST *list);
VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state);
VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state);
UINT32 AndesQueueLen(struct MCU_CTRL *ctl, DL_LIST *list);

#ifdef RTMP_PCI_SUPPORT
VOID PciKickOutCmdMsgComplete(PNDIS_PACKET net_pkt);
#endif


#ifdef RTMP_SDIO_SUPPORT
INT32 AndesWaitForSdioCompleteTimeout(struct cmd_msg *msg, long timeout);
#endif
#endif /* __ANDES_CORE_H__ */

