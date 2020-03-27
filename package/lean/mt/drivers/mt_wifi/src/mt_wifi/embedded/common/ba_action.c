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
	ba_action.c
*/

#include "rt_config.h"

#define BA_ORI_INIT_SEQ		(tr_entry->TxSeq[TID]) /* 1 : inital sequence number of BA session*/
#define ORI_SESSION_MAX_RETRY	8
#if defined(MT7637_FPGA) || defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA)
#define ORI_BA_SESSION_TIMEOUT	(10000)	/* ms */
#else
#define ORI_BA_SESSION_TIMEOUT	(2000)	/* ms */
#endif
#define REC_BA_SESSION_IDLE_TIMEOUT	(1000)	/* ms */
#define REORDERING_PACKET_TIMEOUT		((100 * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define MAX_REORDERING_PACKET_TIMEOUT	((1500 * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define INVALID_RCV_SEQ (0xFFFF)

static inline void ba_enqueue_head(struct reordering_list *list, struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = list->next;
	list->next = mpdu_blk;
}

static inline void ba_enqueue_tail(struct reordering_list *list, struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = NULL;

	if (list->tail)
		list->tail->next = mpdu_blk;
	else
		list->next = mpdu_blk;

	list->tail = mpdu_blk;
}

static inline struct reordering_mpdu *ba_dequeue_head(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	ASSERT(list);

	if (list->next) {
		list->qlen--;
		mpdu_blk = list->next;
		list->next = mpdu_blk->next;

		if (!list->next)
			if (!list->next)
				mpdu_blk->next = NULL;
	}

	return mpdu_blk;
}

static inline struct reordering_mpdu  *ba_reordering_mpdu_dequeue(struct reordering_list *list)
{
	struct reordering_mpdu *ret = (ba_dequeue_head(list));
	return ret;
}


inline struct reordering_mpdu *ba_reordering_mpdu_probe(struct reordering_list *list)
{
	struct reordering_mpdu *ret;

	ASSERT(list);
	ret = list->next;
	return ret;
}

void dump_ba_list(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	if (list->next) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("\n ba sn list:"));
		mpdu_blk = list->next;

		while (mpdu_blk) {
			printk("%x ", mpdu_blk->Sequence);
			mpdu_blk = mpdu_blk->next;
		}
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("\n\n"));
}

static VOID ba_free_ori_entry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return;

	pBAEntry = &pAd->BATable.BAOriEntry[Idx];
	NdisAcquireSpinLock(&pAd->BATabLock);

	if (pBAEntry->ORI_BA_Status != Originator_NONE) {
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BAOriWcidArray[pBAEntry->TID] = 0;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s: Wcid = %d, TID = %d\n", __func__, pBAEntry->Wcid, pBAEntry->TID));

		if (pBAEntry->ORI_BA_Status == Originator_Done) {
			pAd->BATable.numDoneOriginator -= 1;
			pEntry->TXBAbitmap &= (~(1 << (pBAEntry->TID)));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("ba_free_ori_entry numAsOriginator= %ld\n", pAd->BATable.numDoneOriginator));
			/* Erase Bitmap flag.*/
		}

		ASSERT(pAd->BATable.numAsOriginator != 0);
		pAd->BATable.numAsOriginator -= 1;
		pBAEntry->ORI_BA_Status = Originator_NONE;
		pBAEntry->Token = 0;
	}

	NdisReleaseSpinLock(&pAd->BATabLock);
}

static UINT announce_non_hw_damsdu_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR OpMode)
{
	PUCHAR pData;
	USHORT DataSize;
	UINT nMSDU = 0;

	pData = (PUCHAR)GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT)GET_OS_PKT_LEN(pPacket);
	nMSDU = deaggregate_amsdu_announce(pAd, pPacket, pData, DataSize, OpMode);
	return nMSDU;
}

static void announce_ba_reorder_pkt(RTMP_ADAPTER *pAd, struct reordering_mpdu *mpdu)
{
	PNDIS_PACKET pPacket;
	BOOLEAN opmode = pAd->OpMode;

	pPacket = mpdu->pPacket;

	if (mpdu->bAMSDU && (!IS_ASIC_CAP(pAd, fASIC_CAP_HW_DAMSDU)))
		announce_non_hw_damsdu_pkt(pAd, pPacket, mpdu->OpMode);
	else {
		/* pass this 802.3 packet to upper layer or forward this packet to WM directly */
		announce_or_forward_802_3_pkt(pAd, pPacket,
									  wdev_search_by_idx(pAd, RTMP_GET_PACKET_WDEV(pPacket)), opmode);
	}
}

static void ba_mpdu_blk_free(PRTMP_ADAPTER pAd, struct reordering_mpdu *mpdu_blk)
{
	ASSERT(mpdu_blk);
	NdisAcquireSpinLock(&pAd->mpdu_blk_pool.lock);
	ba_enqueue_head(&pAd->mpdu_blk_pool.freelist, mpdu_blk);
	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
}


static void ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd, BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	/* dequeue in-order frame from reodering list */
	while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
		announce_ba_reorder_pkt(pAd, mpdu_blk);

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(pAd, msdu_blk);
		}

		pBAEntry->LastIndSeq = mpdu_blk->Sequence;
		ba_mpdu_blk_free(pAd, mpdu_blk);
		/* update last indicated sequence */
	}

	ASSERT(pBAEntry->list.qlen == 0);
	pBAEntry->CurMpdu = NULL;
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

static VOID ba_ori_session_setup_timeout(
	PVOID SystemSpecific1,
	PVOID FunctionContext,
	PVOID SystemSpecific2,
	PVOID SystemSpecific3)
{
	BA_ORI_ENTRY *pBAEntry = (BA_ORI_ENTRY *)FunctionContext;
	MAC_TABLE_ENTRY *pEntry;
	RTMP_ADAPTER *pAd;

	if (pBAEntry == NULL)
		return;

	pAd = pBAEntry->pAdapter;
#ifdef CONFIG_ATE

	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;

#endif /* CONFIG_ATE */
	pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];

	if ((pBAEntry->ORI_BA_Status == Originator_WaitRes) && (pBAEntry->Token < ORI_SESSION_MAX_RETRY)) {
		MLME_ADDBA_REQ_STRUCT AddbaReq;
		NdisZeroMemory(&AddbaReq, sizeof(AddbaReq));
		COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
		AddbaReq.Wcid = pEntry->wcid;
		AddbaReq.TID = pBAEntry->TID;
#ifdef COEX_SUPPORT
		AddbaReq.BaBufSize = pAd->CommonCfg.BACapability.field.TxBAWinLimit;
#else
		AddbaReq.BaBufSize = pAd->CommonCfg.BACapability.field.RxBAWinLimit;
#endif /* COEX_SUPPORT */
		AddbaReq.TimeOutValue = 0;
		AddbaReq.Token = pBAEntry->Token;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq, 0);
		}
#endif
		RTMP_MLME_HANDLER(pAd);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BA Ori Session Timeout(%d) : Send ADD BA again\n", pBAEntry->Token));
		pBAEntry->Token++;
		RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
	} else
		ba_free_ori_entry(pAd, pEntry->BAOriWcidArray[pBAEntry->TID]);
}
BUILD_TIMER_FUNCTION(ba_ori_session_setup_timeout);

static VOID ba_rec_session_idle_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	BA_REC_ENTRY    *pBAEntry = (BA_REC_ENTRY *)FunctionContext;
	PRTMP_ADAPTER   pAd;
	ULONG           Now32;

	if (pBAEntry == NULL)
		return;

	if ((pBAEntry->REC_BA_Status == Recipient_Established)) {
		NdisGetSystemUpTime(&Now32);

		if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer + REC_BA_SESSION_IDLE_TIMEOUT))) {
			pAd = pBAEntry->pAdapter;
			ba_refresh_reordering_mpdus(pAd, pBAEntry);
			pBAEntry->REC_BA_Status = Recipient_Initialization;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("%ld: REC BA session Timeout\n", Now32));
		}
	}
}
BUILD_TIMER_FUNCTION(ba_rec_session_idle_timeout);

VOID ba_max_winsize_reassign(
	PRTMP_ADAPTER pAd,
	MAC_TABLE_ENTRY *pEntryPeer,
	UCHAR *pWinSize)
{
	UCHAR MaxSize;
	UCHAR MaxPeerRxSize;

	if (CLIENT_STATUS_TEST_FLAG(pEntryPeer, fCLIENT_STATUS_RALINK_CHIPSET))
		MaxPeerRxSize = (1 << (pEntryPeer->MaxRAmpduFactor + 3));  /* (2^(13 + exp)) / 2048 bytes */
	else
		MaxPeerRxSize = (((1 << (pEntryPeer->MaxRAmpduFactor + 3)) * 10) / 16) - 1;

	if (pAd->Antenna.field.TxPath == 3 &&
		(pEntryPeer->HTCapability.MCSSet[2] != 0))
		MaxSize = 31;		/* for 3x3, MaxSize use ((48KB/1.5KB) -1) = 31 */
	else
		MaxSize = 20;			/* for not 3x3, MaxSize use ((32KB/1.5KB) -1) ~= 20 */

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("ba>WinSize=%d, MaxSize=%d, MaxPeerRxSize=%d\n",
			 *pWinSize, MaxSize, MaxPeerRxSize));
	MaxSize = min(MaxPeerRxSize, MaxSize);

	if ((*pWinSize) > MaxSize) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("ba> reassign max win size from %d to %d\n",
				 *pWinSize, MaxSize));
		*pWinSize = MaxSize;
	}
}

static BOOLEAN ba_reordering_mpdu_insertsorted(struct reordering_list *list, struct reordering_mpdu *mpdu)
{
	struct reordering_mpdu **ppScan = &list->next;

	while (*ppScan != NULL) {
		if (SEQ_SMALLER((*ppScan)->Sequence, mpdu->Sequence, MAXSEQ))
			ppScan = &(*ppScan)->next;
		else if ((*ppScan)->Sequence == mpdu->Sequence) {
			/* give up this duplicated frame */
			return FALSE;
		} else {
			/* find position */
			break;
		}
	}

	mpdu->next = *ppScan;
	*ppScan = mpdu;
	list->qlen++;
	return TRUE;
}

VOID ba_resource_dump_all(RTMP_ADAPTER *pAd)
{
	INT i, j;
	BA_ORI_ENTRY *pOriBAEntry;
	BA_REC_ENTRY *pRecBAEntry;
	RTMP_STRING tmpBuf[10];
	struct reordering_mpdu *mpdu_blk = NULL, *msdu_blk = NULL;

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_APCLI(pEntry) || IS_ENTRY_REPEATER(pEntry))
			&& (pEntry->Sst != SST_ASSOC))
			continue;

		if (IS_ENTRY_APCLI(pEntry))
			strncpy(tmpBuf, "ApCli", 10);

		if (IS_ENTRY_REPEATER(pEntry))
			strncpy(tmpBuf, "Repeater", 10);
		else if (IS_ENTRY_WDS(pEntry))
			strncpy(tmpBuf, "WDS", 10);
		else if (IS_ENTRY_MESH(pEntry))
			strncpy(tmpBuf, "Mesh", 10);
		else if (IS_ENTRY_AP(pEntry))
			strncpy(tmpBuf, "AP", 10);
		else
			strncpy(tmpBuf, "STA", 10);

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X (Aid = %d) (%s) -\n",
				 PRINT_MAC(pEntry->Addr), pEntry->Aid, tmpBuf));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Originator]\n"));

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BAOriWcidArray[j] != 0) {
				pOriBAEntry = &pAd->BATable.BAOriEntry[pEntry->BAOriWcidArray[j]];

				if (pOriBAEntry->ORI_BA_Status == Originator_Done)
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TID=%d, BAWinSize=%d, StartSeq=%d, CurTxSeq=%d\n",
							 j, pOriBAEntry->BAWinSize, pOriBAEntry->Sequence,
							 pAd->MacTab.tr_entry[pEntry->wcid].TxSeq[j]));
			}
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[Recipient]\n"));

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BARecWcidArray[j] != 0) {
				pRecBAEntry = &pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];

				if ((pRecBAEntry->REC_BA_Status == Recipient_Established) || (pRecBAEntry->REC_BA_Status == Recipient_Initialization))
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("TID=%d, BAWinSize=%d, LastIndSeq=%d, ReorderingPkts=%d, FreeMpduBls=%d\n", j, pRecBAEntry->BAWinSize,
							  pRecBAEntry->LastIndSeq, pRecBAEntry->list.qlen, pAd->mpdu_blk_pool.freelist.qlen));
			}
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[RX ReorderBuffer]\n"));

		for (j = 0; j < NUM_OF_TID; j++) {
			if (pEntry->BARecWcidArray[j] != 0) {
				pRecBAEntry = &pAd->BATable.BARecEntry[pEntry->BARecWcidArray[j]];
				mpdu_blk = ba_reordering_mpdu_probe(&pRecBAEntry->list);

				if (mpdu_blk) {
					MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence,
							 mpdu_blk->bAMSDU));
					msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

					if (msdu_blk) {
						MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence,
									 msdu_blk->bAMSDU));

						while (msdu_blk->next) {
							msdu_blk = msdu_blk->next;
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence,
									 msdu_blk->bAMSDU));
						}
					}

					while (mpdu_blk->next) {
						mpdu_blk = mpdu_blk->next;
						printk("mpdu:SN = %d, AMSDU = %d\n", mpdu_blk->Sequence, mpdu_blk->bAMSDU);
						msdu_blk = ba_reordering_mpdu_probe(&mpdu_blk->AmsduList);

						if (msdu_blk) {
							MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence,
								 msdu_blk->bAMSDU));

							while (msdu_blk->next) {
								msdu_blk = msdu_blk->next;
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("msdu:SN = %d, AMSDU = %d\n", msdu_blk->Sequence,
									 msdu_blk->bAMSDU));
							}
						}
					}
				}
			}
		}
	}

	return;
}

VOID ba_reordering_resource_dump_all(RTMP_ADAPTER *pAd)
{
	BA_TABLE *Tab;
	BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	int i;
	UINT32 total_pkt_cnt = 0;

	Tab = &pAd->BATable;
	NdisAcquireSpinLock(&pAd->BATabLock);

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &Tab->BARecEntry[i];
		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("total %d msdu packt in ba list\n", total_pkt_cnt));
	NdisReleaseSpinLock(&pAd->BATabLock);
}

VOID ba_reodering_resource_dump(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	int i, j;
	BA_REC_ENTRY *pBAEntry;
	UINT32 total_pkt_cnt = 0;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return;

	for (i = 0; i < NUM_OF_TID; i++) {

		j = pAd->MacTab.Content[wcid].BARecWcidArray[i];

		if (j == 0)
			continue;

		pBAEntry = &pAd->BATable.BARecEntry[j];

		if (pBAEntry->list.next) {
			mpdu_blk = pBAEntry->list.next;
			while (mpdu_blk) {
				if (mpdu_blk->AmsduList.next) {
					msdu_blk = mpdu_blk->AmsduList.next;
					while (msdu_blk) {
						msdu_blk = msdu_blk->next;
						total_pkt_cnt++;
					}
				}
				total_pkt_cnt++;
				mpdu_blk = mpdu_blk->next;
			}
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("total %d msdu packt in wcid (%d) ba list\n", total_pkt_cnt, wcid));
}

/* free all resource for reordering mechanism */
void ba_reordering_resource_release(RTMP_ADAPTER *pAd)
{
	BA_TABLE *Tab;
	BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	int i;

	Tab = &pAd->BATable;
	/* I.  release all pending reordering packet */
	NdisAcquireSpinLock(&pAd->BATabLock);

	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &Tab->BARecEntry[i];

		if (pBAEntry->REC_BA_Status != Recipient_NONE) {
			while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list))) {
				while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
					RELEASE_NDIS_PACKET(pAd, msdu_blk->pPacket, NDIS_STATUS_FAILURE);
					ba_mpdu_blk_free(pAd, msdu_blk);
				}

				ASSERT(mpdu_blk->pPacket);
				RELEASE_NDIS_PACKET(pAd, mpdu_blk->pPacket, NDIS_STATUS_FAILURE);
				ba_mpdu_blk_free(pAd, mpdu_blk);
			}
		}
	}

	NdisReleaseSpinLock(&pAd->BATabLock);
	ASSERT(pBAEntry->list.qlen == 0);
	/* II. free memory of reordering mpdu table */
	NdisAcquireSpinLock(&pAd->mpdu_blk_pool.lock);
	os_free_mem(pAd->mpdu_blk_pool.mem);
	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
}

BOOLEAN ba_reordering_resource_init(RTMP_ADAPTER *pAd, int num)
{
	int     i;
	PUCHAR  mem;
	struct reordering_mpdu *mpdu_blk;
	struct reordering_list *freelist;
	/* allocate spinlock */
	NdisAllocateSpinLock(pAd, &pAd->mpdu_blk_pool.lock);
	/* initialize freelist */
	freelist = &pAd->mpdu_blk_pool.freelist;
	freelist->next = NULL;
	freelist->qlen = 0;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("Allocate %d memory for BA reordering\n", (UINT32)(num * sizeof(struct reordering_mpdu))));
	/* allocate number of mpdu_blk memory */
	os_alloc_mem(pAd, (PUCHAR *)&mem, (num * sizeof(struct reordering_mpdu)));
	pAd->mpdu_blk_pool.mem = mem;

	if (mem == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("Can't Allocate Memory for BA Reordering\n"));
		return FALSE;
	}

	/* build mpdu_blk free list */
	for (i = 0; i < num; i++) {
		/* get mpdu_blk */
		mpdu_blk = (struct reordering_mpdu *)mem;
		/* initial mpdu_blk */
		NdisZeroMemory(mpdu_blk, sizeof(struct reordering_mpdu));
		/* next mpdu_blk */
		mem += sizeof(struct reordering_mpdu);
		/* insert mpdu_blk into freelist */
		ba_enqueue_head(freelist, mpdu_blk);
	}

	return TRUE;
}

static struct reordering_mpdu *ba_mpdu_blk_alloc(RTMP_ADAPTER *pAd)
{
	struct reordering_mpdu *mpdu_blk;

	NdisAcquireSpinLock(&pAd->mpdu_blk_pool.lock);
	mpdu_blk = ba_dequeue_head(&pAd->mpdu_blk_pool.freelist);

	if (mpdu_blk)
		NdisZeroMemory(mpdu_blk, sizeof(*mpdu_blk));

	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
	return mpdu_blk;
}

static USHORT ba_indicate_reordering_mpdus_in_order(PRTMP_ADAPTER pAd,
		PBA_REC_ENTRY pBAEntry,
		USHORT StartSeq)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk;
	USHORT  LastIndSeq = INVALID_RCV_SEQ;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if (!SEQ_STEPONE(mpdu_blk->Sequence, StartSeq, MAXSEQ))
			break;

		/* dequeue in-order frame from reodering list */
		mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
		/* pass this frame up */
		announce_ba_reorder_pkt(pAd, mpdu_blk);
		/* move to next sequence */
		StartSeq = mpdu_blk->Sequence;
		LastIndSeq = StartSeq;

		while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
			announce_ba_reorder_pkt(pAd, msdu_blk);
			ba_mpdu_blk_free(pAd, msdu_blk);
		}

		/* free mpdu_blk */
		ba_mpdu_blk_free(pAd, mpdu_blk);
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	/* update last indicated sequence */
	return LastIndSeq;
}

static void ba_indicate_reordering_mpdus_le_seq(PRTMP_ADAPTER pAd,
		PBA_REC_ENTRY pBAEntry,
		USHORT Sequence)
{
	struct reordering_mpdu *mpdu_blk, *msdu_blk = NULL;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list))) {
		/* find in-order frame */
		if ((mpdu_blk->Sequence == Sequence) || SEQ_SMALLER(mpdu_blk->Sequence, Sequence, MAXSEQ)) {
			/* dequeue in-order frame from reodering list */
			mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
			/* pass this frame up */
			announce_ba_reorder_pkt(pAd, mpdu_blk);

			while ((msdu_blk = ba_reordering_mpdu_dequeue(&mpdu_blk->AmsduList))) {
				announce_ba_reorder_pkt(pAd, msdu_blk);
				ba_mpdu_blk_free(pAd, msdu_blk);
			}

			/* free mpdu_blk */
			ba_mpdu_blk_free(pAd, mpdu_blk);
		} else
			break;
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
}

void ba_timeout_flush(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0;
	UINT32 idx1 = 0;
	PBA_REC_ENTRY pBAEntry = NULL;

	NdisGetSystemUpTime(&now);

	for (idx0 = 0; idx0 < BA_TIMEOUT_BITMAP_LEN; idx0++) {
		idx1 = 0;
		while ((pAd->BATable.ba_timeout_bitmap[idx0] != 0) && (idx1 < 32)) {
			if (pAd->BATable.ba_timeout_bitmap[idx0] & 0x1) {
				pBAEntry = &pAd->BATable.BARecEntry[(idx0 << 5) + idx1];
				ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, now);
			}

			pAd->BATable.ba_timeout_bitmap[idx0] >>= 1;
			idx1++;
		}
	}

	pAd->BATable.ba_timeout_check = FALSE;
}

void ba_timeout_monitor(PRTMP_ADAPTER pAd)
{
	UINT32 idx = 0;
	PBA_REC_ENTRY pBAEntry = NULL;
	ULONG now;
	BOOLEAN need_check = FALSE;
	struct tm_ops *tm_ops = pAd->tm_hif_ops;

	if (!pAd->BATable.ba_timeout_check) {
		NdisGetSystemUpTime(&now);
		for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
			pBAEntry = &pAd->BATable.BARecEntry[idx];
			if ((pBAEntry->REC_BA_Status == Recipient_Established)
					&& (pBAEntry->list.qlen > 0)) {

				if (RTMP_TIME_AFTER((unsigned long)now,
					(unsigned long)(pBAEntry->LastIndSeqAtTimer + REORDERING_PACKET_TIMEOUT))) {
					need_check = TRUE;
					pAd->BATable.ba_timeout_bitmap[(idx >> 5)] |= (1 << (idx % 32));
				}

			}
		}

		if (need_check) {
			pAd->BATable.ba_timeout_check = need_check;

			if (IS_ASIC_CAP(pAd, fASIC_CAP_RX_DLY)) {
				tm_ops->schedule_task(pAd, TR_DONE_TASK);
			} else {
				tm_ops->schedule_task(pAd, RX_DONE_TASK);
			}
		}

	}
}


void ba_flush_reordering_timeout_mpdus(PRTMP_ADAPTER pAd,
									   PBA_REC_ENTRY pBAEntry,
									   ULONG Now32)

{
	USHORT Sequence;

	if ((pBAEntry == NULL) || (pBAEntry->list.qlen <= 0))
		return;

	/*	if ((RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+REORDERING_PACKET_TIMEOUT)) &&*/
	/*		 (pBAEntry->list.qlen > ((pBAEntry->BAWinSize*7)/8))) ||*/
	/*		(RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(10*REORDERING_PACKET_TIMEOUT))) &&*/
	/*		 (pBAEntry->list.qlen > (pBAEntry->BAWinSize/8)))*/
	if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer + (MAX_REORDERING_PACKET_TIMEOUT / 6)))
		&& (pBAEntry->list.qlen > 0)
	   ) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("timeout[%d] (%08lx-%08lx = %d > %d): %x, flush all!\n ", pBAEntry->list.qlen, Now32, (pBAEntry->LastIndSeqAtTimer),
				 (int)((long) Now32 - (long)(pBAEntry->LastIndSeqAtTimer)), MAX_REORDERING_PACKET_TIMEOUT,
				 pBAEntry->LastIndSeq));
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		pBAEntry->LastIndSeqAtTimer = Now32;
	} else if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer + (REORDERING_PACKET_TIMEOUT)))
			   && (pBAEntry->list.qlen > 0)
			  ) {
		/*
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("timeout[%d] (%lx-%lx = %d > %d): %x, ", pBAEntry->list.qlen, Now32, (pBAEntry->LastIndSeqAtTimer),
					   (int)((long) Now32 - (long)(pBAEntry->LastIndSeqAtTimer)), REORDERING_PACKET_TIMEOUT,
					   pBAEntry->LastIndSeq));
		*/
		/* force LastIndSeq to shift to LastIndSeq+1*/
		Sequence = (pBAEntry->LastIndSeq + 1) & MAXSEQ;
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, Sequence);
		pBAEntry->LastIndSeqAtTimer = Now32;
		pBAEntry->LastIndSeq = Sequence;
		/* indicate in-order mpdus*/
		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, Sequence);

		if (Sequence != INVALID_RCV_SEQ)
			pBAEntry->LastIndSeq = Sequence;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("%x, flush one!\n", pBAEntry->LastIndSeq));
	}

}

static BA_ORI_ENTRY *ba_alloc_ori_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_ORI_ENTRY *pBAEntry = NULL;

	NdisAcquireSpinLock(&pAd->BATabLock);

	if (pAd->BATable.numAsOriginator >= (MAX_LEN_OF_BA_ORI_TABLE - 1))
		goto done;

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_ORI_TABLE; i++) {
		pBAEntry = &pAd->BATable.BAOriEntry[i];

		if ((pBAEntry->ORI_BA_Status == Originator_NONE)) {
			/* get one */
			pAd->BATable.numAsOriginator++;
			pBAEntry->ORI_BA_Status = Originator_USED;
			pBAEntry->pAdapter = pAd;
			*Idx = i;
			break;
		}
	}

done:
	NdisReleaseSpinLock(&pAd->BATabLock);
	return pBAEntry;
}

VOID ba_ori_session_setup(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	UCHAR TID,
	USHORT TimeOut,
	ULONG DelayTime,
	BOOLEAN isForced)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	USHORT Idx;
	BOOLEAN Cancelled;
	STA_TR_ENTRY *tr_entry;
	UCHAR BAWinSize = 0;

	ASSERT(TID < NUM_OF_TID);

	if (TID >= NUM_OF_TID) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("Wrong TID %d!\n", TID));
		return;
	}

	if ((pAd->CommonCfg.BACapability.field.AutoBA != TRUE)  &&  (isForced == FALSE))
		return;

	/* if this entry is limited to use legacy tx mode, it doesn't generate BA.  */
	if (RTMPStaFixedTxMode(pAd, pEntry) != FIXED_TXMODE_HT)
		return;

	if ((pEntry->BADeclineBitmap & (1 << TID)) && (isForced == FALSE)) {
		DelayTime = 3000; /* try again after 3 secs*/
		/*		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("DeCline BA from Peer\n"));*/
		/*		return;*/
	}

	Idx = pEntry->BAOriWcidArray[TID];

	if (Idx == 0) {
		pBAEntry = ba_alloc_ori_entry(pAd, &Idx);

		if (!pBAEntry) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s(): alloc BA session failed\n",
					 __func__));
			return;
		}
	} else
		pBAEntry = &pAd->BATable.BAOriEntry[Idx];

	if (pBAEntry->ORI_BA_Status >= Originator_WaitRes)
		return;

	pEntry->BAOriWcidArray[TID] = Idx;
	BAWinSize = pAd->CommonCfg.BACapability.field.TxBAWinLimit;
	/* Initialize BA session */
	pBAEntry->ORI_BA_Status = Originator_WaitRes;
	pBAEntry->Wcid = pEntry->wcid;
	pBAEntry->BAWinSize = BAWinSize;
	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	pBAEntry->Sequence = BA_ORI_INIT_SEQ;
	pBAEntry->Token = 1;	/* (2008-01-21) Jan Lee recommends it - this token can't be 0*/
	pBAEntry->TID = TID;
	pBAEntry->TimeOutValue = TimeOut;
	pBAEntry->pAdapter = pAd;

	if (!(pEntry->TXBAbitmap & (1 << TID)))
		RTMPInitTimer(pAd, &pBAEntry->ORIBATimer, GET_TIMER_FUNCTION(ba_ori_session_setup_timeout), pBAEntry, FALSE);
	else
		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);

	/* set timer to send ADDBA request */
	RTMPSetTimer(&pBAEntry->ORIBATimer, DelayTime);
}


static VOID ba_ori_session_add(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN FRAME_ADDBA_RSP * pFrame)
{
	BA_ORI_ENTRY  *pBAEntry = NULL;
	BOOLEAN Cancelled;
	UCHAR TID;
	USHORT Idx;
	UCHAR MaxPeerBufSize;
	STA_TR_ENTRY *tr_entry;
	UCHAR amsdu_en = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	TID = pFrame->BaParm.TID;
	Idx = pEntry->BAOriWcidArray[TID];
	pBAEntry = &pAd->BATable.BAOriEntry[Idx];
	MaxPeerBufSize = 0;

	/* Start fill in parameters.*/
	if ((Idx != 0) && (pBAEntry->TID == TID) && (pBAEntry->ORI_BA_Status == Originator_WaitRes)) {
		MaxPeerBufSize = (UCHAR)pFrame->BaParm.BufSize;
		{
			if (IS_HIF_TYPE(pAd, HIF_MT))
				pBAEntry->BAWinSize = min(pBAEntry->BAWinSize, MaxPeerBufSize);
			else {
				if (MaxPeerBufSize > 0)
					MaxPeerBufSize -= 1;
				else
					MaxPeerBufSize = 0;

				pBAEntry->BAWinSize = min(pBAEntry->BAWinSize, MaxPeerBufSize);
				ba_max_winsize_reassign(pAd, pEntry, &pBAEntry->BAWinSize);
			}
		}

		pBAEntry->TimeOutValue = pFrame->TimeOutValue;
		if (pEntry->wdev)
			amsdu_en = wlan_config_get_amsdu_en(pEntry->wdev);
		pBAEntry->amsdu_cap = pFrame->BaParm.AMSDUSupported && amsdu_en && cap->SupportAMSDU;
		pBAEntry->ORI_BA_Status = Originator_Done;
		pAd->BATable.numDoneOriginator++;
		/* reset sequence number */
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		pBAEntry->Sequence = BA_ORI_INIT_SEQ;
		/* Set Bitmap flag.*/
		pEntry->TXBAbitmap |= (1 << TID);

		pEntry->tx_amsdu_bitmap &= ~(1 << TID);

		if (pBAEntry->amsdu_cap)
			pEntry->tx_amsdu_bitmap |= (1 << TID);

		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);
		pBAEntry->ORIBATimer.TimerValue = 0;	/*pFrame->TimeOutValue;*/
		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->Sequence, pBAEntry->BAWinSize, BA_SESSION_ORI);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s():TXBAbitmap=%x, AMSDUCap=%d, BAWinSize=%d, TimeOut=%ld\n",
				 __func__, pEntry->TXBAbitmap, pBAEntry->amsdu_cap,
				 pBAEntry->BAWinSize, pBAEntry->ORIBATimer.TimerValue));
		/* Send BAR after BA session is build up */
		if (pEntry->wdev)
			SendRefreshBAR(pAd, pEntry);

		if (pBAEntry->ORIBATimer.TimerValue)
			RTMPSetTimer(&pBAEntry->ORIBATimer, pBAEntry->ORIBATimer.TimerValue); /* in mSec */
	}
}

static BA_REC_ENTRY *ba_alloc_rec_entry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_REC_ENTRY *pBAEntry = NULL;

	NdisAcquireSpinLock(&pAd->BATabLock);

	if (pAd->BATable.numAsRecipient >= (MAX_LEN_OF_BA_REC_TABLE - 1)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("BA Recipeint Session (%ld) > %d\n",
				 pAd->BATable.numAsRecipient, (MAX_LEN_OF_BA_REC_TABLE - 1)));
		goto done;
	}

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i = 1; i < MAX_LEN_OF_BA_REC_TABLE; i++) {
		pBAEntry = &pAd->BATable.BARecEntry[i];

		if ((pBAEntry->REC_BA_Status == Recipient_NONE)) {
			/* get one */
			pAd->BATable.numAsRecipient++;
			pBAEntry->REC_BA_Status = Recipient_USED;
			*Idx = i;
			break;
		}
	}

done:
	NdisReleaseSpinLock(&pAd->BATabLock);
	return pBAEntry;
}

BOOLEAN ba_rec_session_add(
	RTMP_ADAPTER *pAd,
	MAC_TABLE_ENTRY *pEntry,
	FRAME_ADDBA_REQ *pFrame)
{
	BA_REC_ENTRY *pBAEntry = NULL;
	BOOLEAN Status = TRUE, Cancelled;
	USHORT Idx;
	UCHAR TID, BAWinSize;

	ASSERT(pEntry);
	TID = pFrame->BaParm.TID;
	BAWinSize = min(((UCHAR)pFrame->BaParm.BufSize), (UCHAR)pAd->CommonCfg.BACapability.field.RxBAWinLimit);

	if (BAWinSize == 0)
		BAWinSize = pAd->CommonCfg.BACapability.field.RxBAWinLimit;

	/* get software BA rec array index, Idx*/
	Idx = pEntry->BARecWcidArray[TID];

	if (Idx == 0)
		pBAEntry = ba_alloc_rec_entry(pAd, &Idx);
	else {
		pBAEntry = &pAd->BATable.BARecEntry[Idx];
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s(%ld): Idx = %d, BAWinSize(req %d) = %d\n",
			 __func__, pAd->BATable.numAsRecipient, Idx,
			 pFrame->BaParm.BufSize, BAWinSize));

	/* Start fill in parameters.*/
	if (pBAEntry != NULL) {
		NdisAcquireSpinLock(&pAd->BATabLock);
		pBAEntry->REC_BA_Status = Recipient_HandleRes;
		pBAEntry->BAWinSize = BAWinSize;
		pBAEntry->Wcid = pEntry->wcid;
		pBAEntry->TID = TID;
		pBAEntry->TimeOutValue = pFrame->TimeOutValue;
		pBAEntry->REC_BA_Status = Recipient_Initialization;
		pBAEntry->check_amsdu_miss = TRUE;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("Start Seq = %08x\n",  pFrame->BaStartSeq.field.StartSeq));
		NdisReleaseSpinLock(&pAd->BATabLock);

		if (pEntry->RXBAbitmap & (1 << TID))
			RTMPCancelTimer(&pBAEntry->RECBATimer, &Cancelled);
		else
			RTMPInitTimer(pAd, &pBAEntry->RECBATimer, GET_TIMER_FUNCTION(ba_rec_session_idle_timeout), pBAEntry, TRUE);

		/* Set Bitmap flag.*/
		pEntry->RXBAbitmap |= (1 << TID);
		pEntry->BARecWcidArray[TID] = Idx;
		pEntry->BADeclineBitmap &= ~(1 << TID);
		/* Set BA session mask in WCID table.*/
		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->LastIndSeq,
									pBAEntry->BAWinSize, BA_SESSION_RECP);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("MACEntry[%d]RXBAbitmap = 0x%x. BARecWcidArray=%d\n",
				 pEntry->wcid, pEntry->RXBAbitmap, pEntry->BARecWcidArray[TID]));
	} else {
		Status = FALSE;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("Can't Accept ADDBA for %02x:%02x:%02x:%02x:%02x:%02x TID = %d\n",
				 PRINT_MAC(pEntry->Addr), TID));
	}

	return Status;
}

static VOID ba_free_rec_entry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_REC_ENTRY    *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_REC_TABLE))
		return;

	pBAEntry = &pAd->BATable.BARecEntry[Idx];
	NdisAcquireSpinLock(&pAd->BATabLock);

	if (pBAEntry->REC_BA_Status != Recipient_NONE) {
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BARecWcidArray[pBAEntry->TID] = 0;
		ASSERT(pAd->BATable.numAsRecipient != 0);

		if (pAd->BATable.numAsRecipient > 0)
			pAd->BATable.numAsRecipient -= 1;
		else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("Idx = %lu, REC_BA_Status = %d, Wcid(pBAEntry) = %d,\
				Wcid(pEntry) = %d, Tid = %d\n", Idx, pBAEntry->REC_BA_Status, pBAEntry->Wcid, pEntry->wcid, pBAEntry->TID));
		}

		pBAEntry->REC_BA_Status = Recipient_NONE;
	}

	NdisReleaseSpinLock(&pAd->BATabLock);
}


VOID ba_ori_session_tear_down(
	INOUT RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR TID,
	IN BOOLEAN bPassive,
	IN BOOLEAN bForceSend)
{
	UINT Idx = 0;
	BA_ORI_ENTRY *pBAEntry;
	BOOLEAN Cancelled;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

	/* Locate corresponding BA Originator Entry in BA Table with the (pAddr,TID).*/
	Idx = pAd->MacTab.Content[Wcid].BAOriWcidArray[TID];

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE)) {
		if (bForceSend == TRUE) {
			/* force send specified TID DelBA*/
			MLME_DELBA_REQ_STRUCT   DelbaReq;
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem != NULL) {
				NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
				NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
				COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
				DelbaReq.Wcid = Wcid;
				DelbaReq.TID = TID;
				DelbaReq.Initiator = ORIGINATOR;
				Elem->MsgLen  = sizeof(DelbaReq);
				NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
				MlmeDELBAAction(pAd, Elem);
				os_free_mem(Elem);
			} else
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("%s(bForceSend):alloc memory failed!\n", __func__));
		}

		return;
	}

	RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, Wcid, TID, BA_SESSION_ORI);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s===>Wcid=%d.TID=%d\n", __func__, Wcid, TID));
	pBAEntry = &pAd->BATable.BAOriEntry[Idx];
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("\t===>Idx = %d, Wcid=%d.TID=%d, ORI_BA_Status = %d\n", Idx, Wcid, TID, pBAEntry->ORI_BA_Status));

	/* Prepare DelBA action frame and send to the peer.*/
	if ((bPassive == FALSE) && (pBAEntry->TID == TID) && (pBAEntry->ORI_BA_Status == Originator_Done)) {
		MLME_DELBA_REQ_STRUCT   DelbaReq;
		MLME_QUEUE_ELEM *Elem;

		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

		if (Elem != NULL) {
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = pBAEntry->TID;
			DelbaReq.Initiator = ORIGINATOR;
			Elem->MsgLen  = sizeof(DelbaReq);
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(Elem);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("%s():alloc memory failed!\n", __func__));
			return;
		}
	}

	RTMPReleaseTimer(&pBAEntry->ORIBATimer, &Cancelled);
	ba_free_ori_entry(pAd, Idx);
}

VOID ba_rec_session_tear_down(PRTMP_ADAPTER pAd,
							  UCHAR Wcid,
							  UCHAR TID,
							  BOOLEAN bPassive)
{
	ULONG Idx = 0;
	BA_REC_ENTRY *pBAEntry;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
		return;

	/*  Locate corresponding BA Originator Entry in BA Table with the (pAddr,TID).*/
	Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];

	if (Idx == 0)
		return;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s===>Wcid=%d.TID=%d\n", __func__, Wcid, TID));
	pBAEntry = &pAd->BATable.BARecEntry[Idx];
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("\t===>Idx = %ld, Wcid=%d.TID=%d, REC_BA_Status = %d\n", Idx, Wcid, TID, pBAEntry->REC_BA_Status));

	/* Prepare DelBA action frame and send to the peer.*/
	if (pBAEntry->TID == TID) {
		MLME_DELBA_REQ_STRUCT DelbaReq;
		BOOLEAN Cancelled;

		RTMPReleaseTimer(&pBAEntry->RECBATimer, &Cancelled);

		/* 1. Send DELBA Action Frame*/
		if (bPassive == FALSE) {
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));

			if (Elem != NULL) {
				NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
				NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
				COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
				DelbaReq.Wcid = Wcid;
				DelbaReq.TID = TID;
				DelbaReq.Initiator = RECIPIENT;
				Elem->MsgLen  = sizeof(DelbaReq);
				NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
				MlmeDELBAAction(pAd, Elem);
				os_free_mem(Elem);
			} else {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("%s():alloc memory failed!\n", __func__));
				return;
			}
		}

		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		NdisAcquireSpinLock(&pAd->BATabLock);
		/* Erase Bitmap flag at software mactable*/
		pAd->MacTab.Content[Wcid].BARecWcidArray[TID] = 0;
		pAd->MacTab.Content[Wcid].RXBAbitmap &= (~(1 << (pBAEntry->TID)));
		NdisReleaseSpinLock(&pAd->BATabLock);
		RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, Wcid, TID, BA_SESSION_RECP);
	}

	ba_free_rec_entry(pAd, Idx);
}


VOID ba_session_tear_down_all(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	int i;

	for (i = 0; i < NUM_OF_TID; i++) {
		ba_ori_session_tear_down(pAd, Wcid, i, FALSE, FALSE);
		ba_rec_session_tear_down(pAd, Wcid, i, FALSE);
	}
}

VOID peer_addba_req_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Status = 1;
	UCHAR pAddr[6];
	FRAME_ADDBA_RSP ADDframe;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	PFRAME_ADDBA_REQ pAddreqFrame =  (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);
	ULONG FrameLen, *ptemp;
	MAC_TABLE_ENTRY *pMacEntry;
#ifdef DOT11W_PMF_SUPPORT
	STA_TR_ENTRY *tr_entry;
	PFRAME_802_11 pFrame = NULL;
#endif /* DOT11W_PMF_SUPPORT */
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	RETURN_IF_PAD_NULL(pAd);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s ==> (Wcid = %d)\n", __func__, Elem->Wcid));

	/*ADDBA Request from unknown peer, ignore this.*/
	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	pMacEntry = &pAd->MacTab.Content[Elem->Wcid];
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BA - peer_addba_req_action---->\n"));
	ptemp = (PULONG)Elem->Msg;

	if (pMacEntry) {
		if (IS_ENTRY_CLIENT(pMacEntry)) {
			if (pMacEntry->Sst != SST_ASSOC) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("peer entry is not in association state\n"));
				return;
			}

#ifdef DOT11W_PMF_SUPPORT
			tr_entry = &pAd->MacTab.tr_entry[Elem->Wcid];

			if ((pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
						 ("%s: PMF Connection IGNORE THIS PKT DUE TO NOT IN PORTSECURED\n", __func__));
				return;
			}

			pFrame = (PFRAME_802_11)Elem->Msg;

			if ((pMacEntry->SecConfig.PmfCfg.UsePMFConnect == TRUE) &&
				(pFrame->Hdr.FC.Wep == 0)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,
						 ("%s: PMF CONNECTION BUT RECV WEP=0 ACTION, ACTIVE THE SA QUERY\n", __func__));
				PMF_MlmeSAQueryReq(pAd, pMacEntry);
				return;
			}

#endif /* DOT11W_PMF_SUPPORT */
		}
	} else
		return;

	if (PeerAddBAReqActionSanity(pAd, Elem->Msg, Elem->MsgLen, pAddr)) {
		if ((pAd->CommonCfg.bBADecline == FALSE) && IS_HT_STA(pMacEntry)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("Rcv Wcid(%d) AddBAReq\n", Elem->Wcid));

			if (ba_rec_session_add(pAd, &pAd->MacTab.Content[Elem->Wcid], pAddreqFrame)) {
#ifdef PEER_DELBA_TX_ADAPT
				peer_delba_tx_adapt_disable(pAd, &pAd->MacTab.Content[Elem->Wcid]);
#endif /* PEER_DELBA_TX_ADAPT */
				Status = 0;
			} else
				Status = 38; /* more parameters have invalid values*/
		} else {
			Status = 37; /* the request has been declined.*/
		}
	}

	/* 2. Always send back ADDBA Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("ACTION - PeerBAAction() allocate memory failed\n"));
		return;
	}

	NdisZeroMemory(&ADDframe, sizeof(FRAME_ADDBA_RSP));
	/* 2-1. Prepare ADDBA Response frame.*/
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		INT apidx;
#ifdef APCLI_SUPPORT

		if (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) {
			apidx = pMacEntry->func_tb_idx;
#ifdef MAC_REPEATER_SUPPORT

			if (pMacEntry && pMacEntry->bReptCli)
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress, pAddr);
			else
#endif /* MAC_REPEATER_SUPPORT */
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, pAddr);
		} else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (IS_ENTRY_WDS(pMacEntry)) {
				apidx = pMacEntry->func_tb_idx;
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr, pAddr);
			} else
#endif /* WDS_SUPPORT */
			{
				apidx = pMacEntry->func_tb_idx;
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			}
	}

#endif /* CONFIG_AP_SUPPORT */
	ADDframe.Category = CATEGORY_BA;
	ADDframe.Action = ADDBA_RESP;
	ADDframe.Token = pAddreqFrame->Token;
	/* What is the Status code??  need to check.*/
	ADDframe.StatusCode = Status;
	ADDframe.BaParm.BAPolicy = IMMED_BA;

	if (pMacEntry && (Status == 0) && cap->SupportAMSDU)
		ADDframe.BaParm.AMSDUSupported = pAddreqFrame->BaParm.AMSDUSupported;
	else
		ADDframe.BaParm.AMSDUSupported = 0;

	ADDframe.BaParm.TID = pAddreqFrame->BaParm.TID;
	ADDframe.BaParm.BufSize = min(((UCHAR)pAddreqFrame->BaParm.BufSize), (UCHAR)pAd->CommonCfg.BACapability.field.RxBAWinLimit);

	if (ADDframe.BaParm.BufSize == 0) {
		/* ADDframe.BaParm.BufSize = 64; */
		ADDframe.BaParm.BufSize = pAd->CommonCfg.BACapability.field.RxBAWinLimit;
	}

	ADDframe.TimeOutValue = 0; /* pAddreqFrame->TimeOutValue; */
#ifdef UNALIGNMENT_SUPPORT
	{
		BA_PARM tmpBaParm;

		NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&ADDframe.BaParm), sizeof(BA_PARM));
		*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
		NdisMoveMemory((PUCHAR)(&ADDframe.BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
	}
#else
	*(USHORT *)(&ADDframe.BaParm) = cpu2le16(*(USHORT *)(&ADDframe.BaParm));
#endif /* UNALIGNMENT_SUPPORT */
	ADDframe.StatusCode = cpu2le16(ADDframe.StatusCode);
	ADDframe.TimeOutValue = cpu2le16(ADDframe.TimeOutValue);
	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  sizeof(FRAME_ADDBA_RSP),  &ADDframe,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s(%d): TID(%d), BufSize(%d) <==\n", __func__, Elem->Wcid, ADDframe.BaParm.TID,
			 ADDframe.BaParm.BufSize));
}

VOID peer_addba_rsp_action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_ADDBA_RSP pFrame = NULL;

	if (!VALID_UCAST_ENTRY_WCID(pAd, Elem->Wcid))
		return;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s ==> Wcid(%d)\n", __func__, Elem->Wcid));

	if (PeerAddBARspActionSanity(pAd, Elem->Msg, Elem->MsgLen)) {
		pFrame = (PFRAME_ADDBA_RSP)(&Elem->Msg[0]);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("\t\t StatusCode = %d\n", pFrame->StatusCode));

		switch (pFrame->StatusCode) {
		case 0:
			/* I want a BAsession with this peer as an originator. */
			ba_ori_session_add(pAd, &pAd->MacTab.Content[Elem->Wcid], pFrame);
#ifdef PEER_DELBA_TX_ADAPT
			peer_delba_tx_adapt_enable(pAd, &pAd->MacTab.Content[Elem->Wcid]);
#endif /* PEER_DELBA_TX_ADAPT */
			break;

		default:
			/* check status == USED ??? */
			ba_ori_session_tear_down(pAd, Elem->Wcid, pFrame->BaParm.TID, TRUE, FALSE);
			break;
		}

		/* Rcv Decline StatusCode*/
		if ((pFrame->StatusCode == 37)
		   )
			pAd->MacTab.Content[Elem->Wcid].BADeclineBitmap |= 1 << pFrame->BaParm.TID;
	}
}

VOID peer_delba_action(
	PRTMP_ADAPTER pAd,
	MLME_QUEUE_ELEM *Elem)
{
	PFRAME_DELBA_REQ    pDelFrame = NULL;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s ==>\n", __func__));

	/*DELBA Request from unknown peer, ignore this.*/
	if (PeerDelBAActionSanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen)) {
		pDelFrame = (PFRAME_DELBA_REQ)(&Elem->Msg[0]);
#ifdef PEER_DELBA_TX_ADAPT

		if (pDelFrame->DelbaParm.TID == 0)
			peer_delba_tx_adapt_enable(pAd, &pAd->MacTab.Content[Elem->Wcid]);

#endif /* PEER_DELBA_TX_ADAPT */

		if (pDelFrame->DelbaParm.Initiator == ORIGINATOR) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BA - peer_delba_action----> ORIGINATOR\n"));
			ba_rec_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BA - peer_delba_action----> RECIPIENT, Reason = %d\n",  pDelFrame->ReasonCode));
			ba_ori_session_tear_down(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE, FALSE);
		}
	}
}

static BOOLEAN amsdu_sanity(RTMP_ADAPTER *pAd, UINT16 CurSN, UINT8 cur_amsdu_state, PBA_REC_ENTRY pBAEntry, ULONG Now32)
{
	BOOLEAN PreviosAmsduMiss = FALSE;
	USHORT  LastIndSeq;

	if (CurSN != pBAEntry->PreviousSN) {
		if ((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
				(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}
	} else {
		if (((pBAEntry->PreviousAmsduState == FIRST_AMSDU_FORMAT) ||
			(pBAEntry->PreviousAmsduState == MIDDLE_AMSDU_FORMAT)) &&
				(cur_amsdu_state == FIRST_AMSDU_FORMAT)) {
			PreviosAmsduMiss = TRUE;
		}


	}

	if (PreviosAmsduMiss) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("PreviosAmsduMiss or only one MSDU in AMPDU"));
		pBAEntry->CurMpdu = NULL;

		switch (pBAEntry->PreviousReorderCase) {
		case STEP_ONE:
			pBAEntry->LastIndSeq = pBAEntry->PreviousSN;
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				pBAEntry->LastIndSeq = LastIndSeq;

			pBAEntry->LastIndSeqAtTimer = Now32;
			break;

		case REPEAT:
		case OLDPKT:
		case WITHIN:
		case SURPASS:
			break;
		}
	}

	return PreviosAmsduMiss;
}

BOOLEAN bar_process(RTMP_ADAPTER *pAd, ULONG Wcid, ULONG MsgLen, PFRAME_BA_REQ pMsg)
{
	PFRAME_BA_REQ pFrame = pMsg;
	PBA_REC_ENTRY pBAEntry;
	ULONG Idx;
	UCHAR TID;
	ULONG Now32;

	TID = (UCHAR)pFrame->BARControl.TID;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("%s(): BAR-Wcid(%ld), Tid (%d)\n", __func__, Wcid, TID));

	/*hex_dump("BAR", (PCHAR) pFrame, MsgLen);*/
	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MAX_MGMT_PKT_LEN) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("frame too large, size = %ld\n", MsgLen));
		return FALSE;
	} else if (MsgLen != sizeof(FRAME_BA_REQ)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("BlockAck Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	} else if (MsgLen != sizeof(FRAME_BA_REQ)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("BlockAck Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}

	if ((VALID_UCAST_ENTRY_WCID(pAd, Wcid)) && (TID < 8)) {
		/* if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.*/
		Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
		pBAEntry = &pAd->BATable.BARecEntry[Idx];
	} else
		return FALSE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BAR(%ld) : Tid (%d) - %04x:%04x\n", Wcid, TID, pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq));

	NdisGetSystemUpTime(&Now32);
	amsdu_sanity(pAd, pFrame->BAStartingSeq.field.StartSeq, FIRST_AMSDU_FORMAT, pBAEntry, Now32);

	if (SEQ_SMALLER(pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq, MAXSEQ)) {
		LONG TmpSeq, seq;
		/*MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BAR Seq = %x, LastIndSeq = %x\n", pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq));*/

		seq = (pFrame->BAStartingSeq.field.StartSeq == 0) ? MAXSEQ : (pFrame->BAStartingSeq.field.StartSeq - 1);
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, seq);
		pBAEntry->LastIndSeq = seq;
		pBAEntry->LastIndSeqAtTimer = Now32;

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ) {
			pBAEntry->LastIndSeq = TmpSeq;
		}
	}

	return TRUE;
}

void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	RTMP_ADAPTER *pAd,
	RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPkt;
	UCHAR Header802_3[LENGTH_802_3];
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);

	if (wdev_idx >= WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("%s(): invalid wdev_idx(%d)\n", __func__, wdev_idx));
		return;
	}

	wdev = pAd->wdev_list[wdev_idx];
	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	ASSERT(pRxBlk->pRxPacket);

	if (pRxBlk->pRxPacket == NULL)
		return;

	pRxPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);
	RTMP_OS_PKT_INIT(pRxBlk->pRxPacket,
					 get_netdev_from_bssid(pAd, wdev_idx),
					 pRxBlk->pData, pRxBlk->DataSize);

	/* copy 802.3 header, if necessary */
	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU)) {
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;
		USHORT VLAN_VID = 0, VLAN_Priority = 0;
		/* TODO: shiang-usw, fix me!! */
#ifdef CONFIG_AP_SUPPORT

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA) || RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
			/* Check if need to insert VLAN tag to the received packet */
			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

			if (VLAN_VID)
				VLAN_Size = LENGTH_802_1Q;
		}

#endif /* CONFIG_AP_SUPPORT */
		{
			data_p = OS_PKT_HEAD_BUF_EXTEND(pRxPkt, LENGTH_802_3 + VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									 Header802_3, LENGTH_802_3,
									 data_p, TPID);
		}
	}
}

static VOID ba_enqueue_reordering_packet(
	RTMP_ADAPTER *pAd,
	BA_REC_ENTRY *pBAEntry,
	RX_BLK *pRxBlk,
	UCHAR wdev_idx)
{
	struct reordering_mpdu *msdu_blk;
	UINT16 Sequence = pRxBlk->SN;

	msdu_blk = ba_mpdu_blk_alloc(pAd);

	if ((msdu_blk != NULL) &&
		(!RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP))) {
		/* Write RxD buffer address & allocated buffer length */
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
		msdu_blk->Sequence = Sequence;
		msdu_blk->OpMode = pRxBlk->OpMode;
		msdu_blk->bAMSDU = RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU);

		if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			convert_reordering_packet_to_preAMSDU_or_802_3_packet(pAd, pRxBlk, wdev_idx);
		else {
			struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

			pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);

			SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		}

		/* it is necessary for reordering packet to record
			which BSS it come from
		*/
		RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
		STATS_INC_RX_PACKETS(pAd, wdev_idx);
		msdu_blk->pPacket = pRxBlk->pRxPacket;

		if (!pBAEntry->CurMpdu) {
			if (ba_reordering_mpdu_insertsorted(&pBAEntry->list, msdu_blk) == FALSE) {
#ifdef CUT_THROUGH_DBG
				pAd->RxDropPacket++;
#endif
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_SUCCESS);
				ba_mpdu_blk_free(pAd, msdu_blk);
				pBAEntry->CurMpdu = NULL;
			} else
				pBAEntry->CurMpdu = msdu_blk;
		} else
			ba_enqueue_tail(&pBAEntry->CurMpdu->AmsduList, msdu_blk);

		ASSERT((pBAEntry->list.qlen >= 0)  && (pBAEntry->list.qlen <= pBAEntry->BAWinSize));
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	} else {
		ULONG Now32;

		if (msdu_blk)
			ba_mpdu_blk_free(pAd, msdu_blk);
		else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR,  ("!!! (used:%d/free:%d) Can't allocate reordering mpdu blk\n",
					 pBAEntry->list.qlen, pAd->mpdu_blk_pool.freelist.qlen));
		}

		/*
		 * flush all pending reordering mpdus
		 * and receving mpdu to upper layer
		 * make tcp/ip to take care reordering mechanism
		 */
		/*ba_refresh_reordering_mpdus(pAd, pBAEntry);*/

		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, Sequence);
		pBAEntry->LastIndSeq = Sequence;

		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);

		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

		if (Sequence != INVALID_RCV_SEQ) {
			pBAEntry->LastIndSeq = Sequence;
		}

		NdisGetSystemUpTime(&Now32);
		pBAEntry->LastIndSeqAtTimer = Now32;
		pBAEntry->CurMpdu = NULL;
	}
}

VOID ba_reorder(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	ULONG Now32;
	UINT16 Sequence = pRxBlk->SN;
	USHORT Idx;
	PBA_REC_ENTRY pBAEntry = NULL;
	BOOLEAN amsdu_miss = FALSE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s: sn = %d, amsdu state = %d, wcid = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState, pRxBlk->wcid));

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		Idx = pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID];

		if (Idx == 0) {
			/* Rec BA Session had been torn down */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s: sn = %d, amsdu state = %d, Rec BA Session had been torn down\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
			indicate_rx_pkt(pAd, pRxBlk, wdev_idx);
			return;
		}

		pBAEntry = &pAd->BATable.BARecEntry[Idx];
	} else {
		ASSERT(0);
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ASSERT(pBAEntry);
	NdisGetSystemUpTime(&Now32);

	if (pBAEntry->check_amsdu_miss) {
		amsdu_miss = amsdu_sanity(pAd, Sequence, pRxBlk->AmsduState, pBAEntry, Now32);
	}

	pBAEntry->check_amsdu_miss = TRUE;

	if ((pRxBlk->AmsduState == FINAL_AMSDU_FORMAT)
			|| (pRxBlk->AmsduState == MSDU_FORMAT) || amsdu_miss)
			ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);

	switch (pBAEntry->REC_BA_Status) {
	case Recipient_NONE:
	case Recipient_USED:
	case Recipient_HandleRes:
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		return;

	case Recipient_Initialization:
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		ASSERT((pBAEntry->list.qlen == 0) && (pBAEntry->list.next == NULL));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Reset Last Indicate Sequence(%d): amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);
		pBAEntry->LastIndSeq = Sequence;
		pBAEntry->LastIndSeqAtTimer = Now32;
		pBAEntry->PreviousAmsduState = pRxBlk->AmsduState;
		pBAEntry->PreviousSN = Sequence;
		pBAEntry->REC_BA_Status = Recipient_Established;
		return;

	case Recipient_Established:
		break;

	default:
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_ERROR, ("%s: unknow receipt state = %d\n", __func__, pBAEntry->REC_BA_Status));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

ba_reorder_check:
	/* I. Check if in order. */
	if (SEQ_STEPONE(Sequence, pBAEntry->LastIndSeq, MAXSEQ)) {
		USHORT  LastIndSeq;

		indicate_rx_pkt(pAd, pRxBlk, wdev_idx);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Check if in order(%d) : amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));

		if ((pRxBlk->AmsduState == FINAL_AMSDU_FORMAT) || (pRxBlk->AmsduState == MSDU_FORMAT)) {
			pBAEntry->LastIndSeq = Sequence;
			LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

			if (LastIndSeq != INVALID_RCV_SEQ)
				pBAEntry->LastIndSeq = LastIndSeq;

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Check if in order 2(%d) : amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
			pBAEntry->LastIndSeqAtTimer = Now32;
		}

		pBAEntry->PreviousReorderCase = STEP_ONE;
	}
	/* II. Drop Duplicated Packet*/
	else if (Sequence == pBAEntry->LastIndSeq) {
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) &&
			(!IS_ASIC_CAP(pAd, fASIC_CAP_HW_DAMSDU))) {
			pBAEntry->LastIndSeqAtTimer = Now32;
			indicate_rx_pkt(pAd, pRxBlk, wdev_idx);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%sDrop Duplicated Packet(%d) : amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
		} else {
			pBAEntry->nDropPacket++;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Drop Duplicated Packet 2(%d) : amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
#ifdef CUT_THROUGH_DBG
			pAd->RxDropPacket++;
#endif
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		}

		pBAEntry->PreviousReorderCase = REPEAT;
	}
	/* III. Drop Old Received Packet*/
	else if (SEQ_SMALLER(Sequence, pBAEntry->LastIndSeq, MAXSEQ)) {
		pBAEntry->nDropPacket++;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Drop Old Received Packet(%d) : amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		pBAEntry->PreviousReorderCase = OLDPKT;
	}
	/* IV. Receive Sequence within Window Size*/
	else if (SEQ_SMALLER(Sequence, (((pBAEntry->LastIndSeq + pBAEntry->BAWinSize + 1)) & MAXSEQ), MAXSEQ)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:Receive Sequence within Window Size(%d) :  amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
		ba_enqueue_reordering_packet(pAd, pBAEntry, pRxBlk, wdev_idx);
		pBAEntry->PreviousReorderCase = WITHIN;
	}
	/* V. Receive seq surpasses Win(lastseq + nMSDU). So refresh all reorder buffer*/
	else {
		LONG WinStartSeq, TmpSeq;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%lu: Refresh. Seq=0x%x, #RxPkt=%d. LastIndSeq=0x%x.\n",
				 Now32, Sequence, pBAEntry->list.qlen, pBAEntry->LastIndSeq));

		TmpSeq = Sequence - (pBAEntry->BAWinSize) + 1;

		if (TmpSeq < 0)
			TmpSeq = (MAXSEQ + 1) + TmpSeq;

		WinStartSeq = TmpSeq;
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, (WinStartSeq - 1) & MAXSEQ);

		pBAEntry->LastIndSeq = (WinStartSeq - 1) & MAXSEQ;
		pBAEntry->LastIndSeqAtTimer = Now32;

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

		if (TmpSeq != INVALID_RCV_SEQ)
			pBAEntry->LastIndSeq = TmpSeq;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_INFO, ("%s:others(%d) :  amsdu state = %d\n", __func__, pRxBlk->SN, pRxBlk->AmsduState));
		pBAEntry->PreviousReorderCase = SURPASS;

		goto ba_reorder_check;
	}

	pBAEntry->PreviousAmsduState = pRxBlk->AmsduState;
	pBAEntry->PreviousSN = Sequence;

	if ((pRxBlk->AmsduState == MSDU_FORMAT) || (pRxBlk->AmsduState == FINAL_AMSDU_FORMAT))
		pBAEntry->CurMpdu = NULL;
}

VOID ba_reorder_buf_maintain(RTMP_ADAPTER *pAd)
{
	ULONG Now32;
	UCHAR Wcid;
	USHORT Idx;
	UCHAR TID;
	PBA_REC_ENTRY pBAEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	/* update last rx time*/
	NdisGetSystemUpTime(&Now32);

	/*TODO:Carter, check why Wcid start from 1.*/
	for (Wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, Wcid); Wcid++) {
		pEntry = &pAd->MacTab.Content[Wcid];

		if (IS_ENTRY_NONE(pEntry))
			continue;

		for (TID = 0; TID < NUM_OF_TID; TID++) {
			Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
			pBAEntry = &pAd->BATable.BARecEntry[Idx];
			ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);
		}
	}
}

VOID ba_ori_session_start(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UINT8 UPriority)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[tr_entry->wcid];

	if (pAd->CommonCfg.BACapability.field.AutoBA == FALSE)
		return;

	/* TODO: shiang-usw, fix me for pEntry, we should replace this paramter as tr_entry! */
	if ((tr_entry && tr_entry->EntryType != ENTRY_CAT_MCAST && VALID_UCAST_ENTRY_WCID(pAd, tr_entry->wcid)) &&
		(pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry)) {
		BOOLEAN isRalink = CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);

		if (((pEntry->TXBAbitmap & (1 << UPriority)) == 0)
			/* && ((pEntry->BADeclineBitmap & (1 << UPriority)) == 0) */
			&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)
			&& (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			&& ((isRalink || IS_ENTRY_MESH(pEntry) || IS_ENTRY_WDS(pEntry))
				|| (IS_NO_SECURITY(&pEntry->SecConfig)
					|| IS_CIPHER_CCMP128(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_CCMP256(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_GCMP128(pEntry->SecConfig.PairwiseCipher)
					|| IS_CIPHER_GCMP256(pEntry->SecConfig.PairwiseCipher)
				   ))
		   )
			ba_ori_session_setup(pAd, pEntry, UPriority, 0, 10, FALSE);
	}
}



#ifdef PEER_DELBA_TX_ADAPT
static VOID peer_delba_tx_adapt_enable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
#ifdef MCS_LUT_SUPPORT
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("%s(): No need this for HIF_MT!\n", __func__));
		return;
	}

#endif /* MT_MAC */

	if ((pAd->CommonCfg.bBADecline == TRUE) ||
		(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))) {
		/* we should not do this if bBADecline is TRUE or RGD is ON */
		return;
	}

	if (!RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) || !(pEntry->wcid < 128)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
				 ("%s(): Warning! This chip does not support HW Tx rate lookup.\n",
				  __func__));
		return;
	}

	if (pEntry) {
		USHORT RegId = 0;
		UINT32 MacReg = 0, BitLUT;

		pEntry->bPeerDelBaTxAdaptEn = 1;
		/* Enable Tx Mac look up table */
		RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
		BitLUT = (MacReg & ((1 << 18)));

		if (BitLUT) {
			/* Keep original register setting in this flag */
			pEntry->bPeerDelBaTxAdaptEn |= BitLUT;
		} else {
			MacReg |= (1 << 18);
			RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);
		}

		RegId = 0x1C00 + (pEntry->Aid << 3);
		RTMP_IO_WRITE32(pAd, RegId, 0x4007); /* Legacy OFDM / no STBC / LGI / BW20 / MCS 7 */
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE,
				 ("%s():MacReg = 0x%08x, bPeerDelBaTxAdaptEn = 0x%x\n",
				  __func__, MacReg, pEntry->bPeerDelBaTxAdaptEn));
	}

#endif /* MCS_LUT_SUPPORT */
}


static VOID peer_delba_tx_adapt_enable(
	PRTMP_ADAPTER pAd,
	PMAC_TABLE_ENTRY pEntry)
{
#ifdef MCS_LUT_SUPPORT
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_OFF, ("%s(): No need this for HIF_MT!\n", __func__));
		return;
	}

#endif /* MT_MAC */

	if (!RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) || !(pEntry->wcid < 128)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_WARN,
				 ("%s(): Warning! This chip does not support HW Tx rate lookup.\n",
				  __func__));
		return;
	}

	if (pEntry && pEntry->bPeerDelBaTxAdaptEn) {
		UINT32 BitLUT;

		BitLUT = (pEntry->bPeerDelBaTxAdaptEn & (1 << 18));

		if (!BitLUT) {
			UINT32 MacReg = 0;
			/* Disable Tx Mac look up table (Ressume original setting) */
			RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
			MacReg &= ~(1 << 18);
			RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE,
					 ("%s():TX_FBK_LIMIT = 0x%08x\n",
					  __func__, MacReg));
		}

		/* TODO: ressume MSC rate of the MAC look up table? */
		pEntry->bPeerDelBaTxAdaptEn = 0;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE,
				 ("%s():bPeerDelBaTxAdaptEn = 0x%x\n",
				  __func__, pEntry->bPeerDelBaTxAdaptEn));
	}

#endif /* MCS_LUT_SUPPORT */
}
#endif /* PEER_DELBA_TX_ADAPT */
