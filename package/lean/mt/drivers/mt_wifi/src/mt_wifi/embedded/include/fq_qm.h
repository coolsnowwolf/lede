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
#include    "rt_config.h"

#ifndef __FQ_QM_H__
#define __FQ_QM_H__

#if defined(MT7615)
#define FQ_PLE_SIZE						4096
#else
#define FQ_PLE_SIZE						2048
#endif
#define FQ_SCH_DBG_SUPPORT					1
#define DEFAULT_THMAX						(16)
#define MAX_VHT_THMAX						(256)
#define MAX_FQ_VHT_AMPDU_NUM                                    (256)
#define MAX_FQ_VHT_AMPDU_LEN                                    (1*1024*1024-1)
#define MAX_HT_THMAX						(42)
#define MIN_HT_THMAX						(4)
#define MAX_FQ_HT_AMPDU_NUM					(64)
#define MAX_FQ_HT_AMPDU_LEN					(65535)
#define MAX_FQ_PPDU_TIME					(5484)	/* 5484 usec */

#define FQ_DBG_DUMP_STA_LOG					(0x1)
#define FQ_DBG_DUMP_PLE_EMPTY					(0x2)
#define FQ_DBG_DUMP_FQLIST					(0x4)
#define FQ_DBG_LOG_ON						(0x8)
#define FQ_DBG_MASK						(0xff)

#define FQ_EN							(0x1)
#define FQ_SKIP_SINGLE_STA_CASE					(0x2)
#define FQ_SKIP_RED						(0x4)
#define FQ_NO_PKT_STA_KEEP_IN_LIST				(0x8)
#define FQ_LONGEST_DROP						(0x10)
#define FQ_ARRAY_SCH						(0x1000)
#define FQ_EN_MASK						(0x01ffffff)
#define FQ_READY						(0x02000000)
#define FQ_NEED_ON						(0x04000000)

#define FQ_PER_AC_LIMIT						4096
#define FQ_EMPTY_STA						0
#define FQ_UN_CLEAN_STA						1
#define FQ_IN_LIST_STA						2

#define FQ_BITMAP_MASK						(0x1F)
#define FQ_BITMAP_SHIFT						(5)
#define FQ_BITMAP_DWORD						(MAX_LEN_OF_MAC_TABLE/(sizeof(UINT32)*8))

#define UMAC_DRR_TABLE_RDATA0           			(PLE_BASE+0x350)
#define UMAC_AIRTIME_QUANTUM_SETTING0          			(PLE_BASE+0x380)
#define UMAC_DRR_TABLE_CTRL0_FQ            			(PLE_BASE+0x388)
#define UMAC_VOW_DBG_SEL					(PLE_BASE+0x3A0)
#define UMAC_AIRTIME_DBG_INFO0					(PLE_BASE+0x3A4)
#define UMAC_AIRTIME_DBG_INFO1					(PLE_BASE+0x3A8)

#define UMAX_AIRTIME_QUANTUM_OFFSET			8
#define UMAX_AIRTIME_QUANTUM_MASK			BITS(0, 7)
#define UMAC_STA_ID_MASK	                        BITS(0, 7)
#define UMAC_STALINK_CUR_STA_ID_OFFSET                  22
#define UMAC_STALINK_HEAD_STA_ID_OFFSET                 2
#define UMAC_STALINK_TAIL_STA_ID_OFFSET                 12
#define UMAC_STALINK_STA_NUM_OFFSET	                16
#define UMAC_STALINK_STA_NUM_MASK	                BITS(0, 10)
#define UMAC_STALINK_STA_LOCK_MASK	                BITS(0, 8)
#define UMAC_QUEUE_ID_MASK      	                BITS(0, 1)
#define UMAC_FORWARD_LINK_STA_ID_OFFSET			2
#define UMAC_DRR_TABLE_CTRL0_EXE			(1<<31)
#define UMAC_DRR_TABLE_CTRL0_MODE_MASK			BITS(16, 23)

struct fq_stainfo_type {
	QUEUE_ENTRY Entry[WMM_NUM_OF_AC];
	UINT16 macInQLen[WMM_NUM_OF_AC];
	UINT16 macOutQLen[WMM_NUM_OF_AC];
	UINT8 wcid;
	UINT8 kickPktCnt[WMM_NUM_OF_AC];
	UINT8 thMax[WMM_NUM_OF_AC];
	UINT16 mpduTime;
	UINT16 KMAX;
	UINT32 drop_cnt[WMM_NUM_OF_AC];
	UINT32 qlen_max_cnt[WMM_NUM_OF_AC];
	INT32 tx_msdu_cnt;
	INT32 macQPktLen[WMM_NUM_OF_AC];
	UINT8 status[WMM_NUM_OF_AC];
	NDIS_SPIN_LOCK	lock[WMM_NUM_OF_AC];
};

struct fq_ctrl_type {
	UINT32 	list_map[WMM_NUM_OF_AC][FQ_BITMAP_DWORD];
	UINT32  no_packet_chk_map[WMM_NUM_OF_AC][FQ_BITMAP_DWORD];
	UINT32  staInUseBitmap[FQ_BITMAP_DWORD];
	QUEUE_HEADER fq[WMM_NUM_OF_AC];
	UINT32 	frm_cnt[WMM_NUM_OF_AC];
	UINT32  drop_cnt[WMM_NUM_OF_AC];
	UINT8	factor;
	UINT32	enable;
	UCHAR	prev_qm;
	UINT8	dbg_en;
	UINT8	nactive;
	UINT8	nbcmc_active;
	UINT8	npow_save;
	QUEUE_ENTRY *pPrevEntry[WMM_NUM_OF_AC];
#ifdef FQ_SCH_DBG_SUPPORT
	INT prev_wcid[WMM_NUM_OF_AC];
	INT prev_qidx;
	INT prev_kick_cnt[WMM_NUM_OF_AC];
	UINT32	sta_in_head[WMM_NUM_OF_AC][MAX_LEN_OF_MAC_TABLE];
	UINT32  frm_max_cnt[WMM_NUM_OF_AC];
#endif
	UINT16	msdu_out_hw;
	UINT16	msdu_in_hw;
	UINT16  msdu_threshold;
	INT16	srch_pos[WMM_NUM_OF_AC];
};

INT set_fq_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT set_fq_debug_enable(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT set_fq_dbg_listmap(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT set_fq_dbg_linklist(PRTMP_ADAPTER pAd, RTMP_STRING *arg);
INT show_fq_info(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
void app_show_fq_dbgmsg(struct _RTMP_ADAPTER *pAd);
void fq_tx_free_per_packet(struct _RTMP_ADAPTER *pAd, UINT8 ucAC, UINT8 ucWlanIdx, NDIS_PACKET *pkt);
void fq_tx_free_event_handler(UINT32 *dataPtr, UINT8 ucMsduIdCnt, UINT8 version, struct _RTMP_ADAPTER *pAd);
void fp_fair_tx_pkt_deq_func(struct _RTMP_ADAPTER *pAd);
INT fq_clean_list(struct _RTMP_ADAPTER *pAd, UCHAR qidx);
UINT16 fq_del_list(struct _RTMP_ADAPTER *pAd, struct dequeue_info *info, CHAR deq_qid, UINT32 *tx_quota);
INT fq_init(struct _RTMP_ADAPTER *pAd);
INT fq_exit(struct _RTMP_ADAPTER *pAd);
INT fq_update_thMax(struct _RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry, UINT8 wcid,
			INT32 mpduTime, UINT32 dwrr_quantum, UINT32 *Value);
INT fq_enq_req(struct _RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, UCHAR qidx,
	struct _STA_TR_ENTRY *tr_entry, struct _QUEUE_HEADER *pPktQueue);
INT fq_del_report(struct _RTMP_ADAPTER *pAd, struct dequeue_info *info);
#endif


