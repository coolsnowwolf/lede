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

#ifndef __QM_H__
#define __QM_H__

#define MCAST_WCID_TO_REMOVE 0

struct _STA_TR_ENTRY;
struct dequeue_info;

enum {
	TX_QUE_LOW,
	TX_QUE_HIGH,
};

enum {
	TX_QUE_HIGH_TO_HIGH,
	TX_QUE_HIGH_TO_LOW,
	TX_QUE_LOW_TO_LOW,
	TX_QUE_LOW_TO_HIGH,
	TX_QUE_UNKNOW_CHANGE,
};

enum queue_mm {
	GENERIC_QM,
	FAST_PATH_QM,
	GENERIC_FAIR_QM,
	FAST_PATH_FAIR_QM
};

struct qm_ctl {
	UINT32 total_psq_cnt;
} ____cacheline_aligned;

/**
 * @init: qm resource initialization
 * @exit: qm resource exit
 * @enq_mgmt_pkt: en-queue packet to management queue operation
 * @enq_data_pkt: en-queue packet to data queue operation
 * @get_psq_pkt: get packet from power saving queue operation
 * @enq_psq_pkt: en-queue packet to power saving queue operation
 * @schedule_tx_que: schedule job that may use thread, worker, or tasklet to dequeue tx queue and service packet
 * @sta_clean_queue: cleanup resource inside queue per station
 * @sta_dump_queue: dump resource inside queue per station per queue index
 * @dump_all_sw_queue: dump all sw queue information
 * @deq_data_pkt: for fair queue dequeue packet
 */

struct qm_ops {
	/* INIT/EXIT */
	INT (*init)(struct _RTMP_ADAPTER *pAd);
	INT (*exit)(struct _RTMP_ADAPTER *pAd);
	INT (*sta_clean_queue)(struct _RTMP_ADAPTER *pAd, UCHAR wcid);
	VOID (*sta_dump_queue)(struct _RTMP_ADAPTER *pAd, UCHAR wcid, enum PACKET_TYPE pkt_type, UCHAR q_idx);
	INT (*bss_clean_queue)(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev);
	INT (*dump_all_sw_queue)(struct _RTMP_ADAPTER *ad);

	/* TX */
	INT (*enq_mgmtq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt) ____cacheline_aligned;
	INT (*enq_dataq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx);
	NDIS_PACKET *(*get_psq_pkt)(struct _RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry);
	INT (*enq_psq_pkt)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt);
	INT (*schedule_tx_que)(struct _RTMP_ADAPTER *pAd);
	INT32 (*deq_data_pkt)(struct _RTMP_ADAPTER *pAd, struct _TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info);
} ____cacheline_aligned;

#define MGMT_QUE_MAX_NUMS 512
#define HIGH_PRIO_QUE_MAX_NUMS 512
#ifdef CONFIG_RALINK_MT7621
#define DATA_QUE_MAX_NUMS 8192
#else
#define DATA_QUE_MAX_NUMS 4096
#endif

enum pkt_tx_status {
	PKT_SUCCESS = 0,
	INVALID_PKT_LEN = 1,
	INVALID_TR_WCID = 2,
	INVALID_TR_ENTRY = 3,
	INVALID_WDEV = 4,
	INVALID_ETH_TYPE = 5,
	DROP_PORT_SECURE = 6,
	DROP_PSQ_FULL = 7,
	DROP_TXQ_FULL = 8,
	DROP_TX_JAM = 9,
	DROP_TXQ_ENQ_FAIL = 10,
};

struct reason_id_str {
	INT id;
	RTMP_STRING *code_str;
};

#define InitializeQueueHeader(QueueHeader)              \
	{                                                       \
		(QueueHeader)->Head = (QueueHeader)->Tail = NULL;   \
		(QueueHeader)->Number = 0;                          \
	}

#define RemoveHeadQueue(QueueHeader)                \
	(QueueHeader)->Head;                                \
	{                                                   \
		PQUEUE_ENTRY pNext;                             \
		if ((QueueHeader)->Head != NULL) {				\
			pNext = (QueueHeader)->Head->Next;          \
			(QueueHeader)->Head->Next = NULL;		\
			(QueueHeader)->Head = pNext;                \
			if (pNext == NULL)                          \
				(QueueHeader)->Tail = NULL;             \
			(QueueHeader)->Number--;                    \
		}												\
	}

#define RemoveTailQueue(QueueHeader)       \
	(QueueHeader)->Tail;                       \
	{                                          \
		PQUEUE_ENTRY pNext;                     \
		if ((QueueHeader)->Head != NULL) {			 \
			pNext = (QueueHeader)->Head;         \
			if (pNext->Next == NULL) {           \
				(QueueHeader)->Head = NULL;       \
				(QueueHeader)->Tail = NULL;       \
			} else {                             \
				while (pNext->Next != (QueueHeader)->Tail) { \
					pNext = pNext->Next;           \
				}                                 \
				(QueueHeader)->Tail = pNext;      \
				pNext->Next = NULL;               \
			}                                    \
			(QueueHeader)->Number--;              \
		}                                        \
	}


#define InsertHeadQueue(QueueHeader, QueueEntry)            \
	{                                                           \
		((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
		(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		if ((QueueHeader)->Tail == NULL)                        \
			(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
		(QueueHeader)->Number++;                                \
	}

#define InsertTailQueue(QueueHeader, QueueEntry)				\
	{                                                               \
		((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                    \
		if ((QueueHeader)->Tail)                                    \
			(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry); \
		else                                                        \
			(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
		(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);           \
		(QueueHeader)->Number++;                                    \
	}

#define InsertTailQueueAc(pAd, pEntry, QueueHeader, QueueEntry)			\
	{																		\
		((PQUEUE_ENTRY)QueueEntry)->Next = NULL;							\
		if ((QueueHeader)->Tail)											\
			(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);			\
		else																\
			(QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);				\
		(QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);					\
		(QueueHeader)->Number++;											\
	}

#define PickFromQueue(QueueHeader, QueuePrevEntry, QueueEntry)                \
	do {\
		PQUEUE_ENTRY pHead;	\
		if ((QueueHeader)->Head != (PQUEUE_ENTRY)QueueEntry) {	\
			((PQUEUE_ENTRY)QueuePrevEntry)->Next = ((PQUEUE_ENTRY)QueueEntry)->Next;	\
			((PQUEUE_ENTRY)QueueEntry)->Next = NULL;	\
			if (((QueueHeader)->Tail) == (PQUEUE_ENTRY)QueueEntry) {	\
				(QueueHeader)->Tail = (PQUEUE_ENTRY)QueuePrevEntry;	\
				(QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueuePrevEntry);	\
				((PQUEUE_ENTRY)QueuePrevEntry)->Next = NULL;	\
			}	\
			(QueueHeader)->Number--; \
		} else {	\
			pHead = RemoveHeadQueue(QueueHeader); \
			((PQUEUE_ENTRY)QueueEntry)->Next = NULL; \
		}	\
	} while (0);


#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags)				\
	do {													\
		if (bIntContext == FALSE)						\
			RTMP_IRQ_LOCK((lock), IrqFlags);		\
	} while (0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
	do {													\
		if (bIntContext == FALSE)						\
			RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
	} while (0)


VOID ge_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);
VOID RTMPDeQueuePacket(struct _RTMP_ADAPTER *pAd, BOOLEAN bIntContext, UCHAR QueIdx, INT wcid, INT Max_Tx_Packets);
INT ge_enq_req(struct _RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qidx, struct _STA_TR_ENTRY *tr_entry, QUEUE_HEADER *pPktQueue);
VOID ge_tx_swq_dump(struct _RTMP_ADAPTER *pAd, INT qidx);
INT qm_init(struct _RTMP_ADAPTER *pAd);
INT qm_exit(struct _RTMP_ADAPTER *pAd);
VOID qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue, NDIS_SPIN_LOCK *lock);
INT deq_packet_gatter(struct _RTMP_ADAPTER *pAd, struct dequeue_info *deq_info, struct _TX_BLK *pTxBlk);
#endif
