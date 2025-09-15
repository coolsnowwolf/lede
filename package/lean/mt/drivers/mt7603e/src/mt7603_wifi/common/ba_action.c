#ifdef DOT11_N_SUPPORT

#include "rt_config.h"



#define BA_ORI_INIT_SEQ		(tr_entry->TxSeq[TID]) /* 1 : inital sequence number of BA session*/

#define ORI_SESSION_MAX_RETRY	8
#define ORI_BA_SESSION_TIMEOUT	(2000)	/* ms */
#define REC_BA_SESSION_IDLE_TIMEOUT	(1000)	/* ms */

#define REORDERING_PACKET_TIMEOUT		((100 * OS_HZ)/1000)	/* system ticks -- 100 ms*/
#define MULTI_CLIENT_REORDERING_PACKET_TIMEOUT		((200 * OS_HZ)/1000)	/* system ticks -- 200 ms*/
#define MAX_REORDERING_PACKET_TIMEOUT	((3000 * OS_HZ)/1000)	/* system ticks -- 100 ms*/

#if defined(MAX_CONTINUOUS_TX_CNT) || defined(NEW_IXIA_METHOD)
#undef MULTI_CLIENT_REORDERING_PACKET_TIMEOUT
#define MULTI_CLIENT_REORDERING_PACKET_TIMEOUT		((500 * OS_HZ)/1000)	/* system ticks -- 200 ms*/
#endif
#define RESET_RCV_SEQ		(0xFFFF)

static void ba_mpdu_blk_free(PRTMP_ADAPTER pAd, struct reordering_mpdu *mpdu_blk);

#ifdef PEER_DELBA_TX_ADAPT
static VOID Peer_DelBA_Tx_Adapt_Enable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);

static VOID Peer_DelBA_Tx_Adapt_Disable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry);
#endif /* PEER_DELBA_TX_ADAPT */

BA_ORI_ENTRY *BATableAllocOriEntry(RTMP_ADAPTER *pAd, USHORT *Idx);
BA_REC_ENTRY *BATableAllocRecEntry(RTMP_ADAPTER *pAd, USHORT *Idx);

VOID BAOriSessionSetupTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID BARecSessionIdleTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);


BUILD_TIMER_FUNCTION(BAOriSessionSetupTimeout);
BUILD_TIMER_FUNCTION(BARecSessionIdleTimeout);

#define ANNOUNCE_REORDERING_PACKET(_pAd, _mpdu_blk)	\
			Announce_Reordering_Packet(_pAd, _mpdu_blk);

VOID BA_MaxWinSizeReasign(
	IN PRTMP_ADAPTER	pAd,
	IN MAC_TABLE_ENTRY  *pEntryPeer,
	OUT UCHAR			*pWinSize)
{
	UCHAR MaxSize;
	UCHAR MaxPeerRxSize;


	if (CLIENT_STATUS_TEST_FLAG(pEntryPeer, fCLIENT_STATUS_RALINK_CHIPSET))
		MaxPeerRxSize = (1 << (pEntryPeer->MaxRAmpduFactor + 3));  /* (2^(13 + exp)) / 2048 bytes */
	else
		MaxPeerRxSize = (((1 << (pEntryPeer->MaxRAmpduFactor + 3)) * 10) / 16) -1;

	if (pAd->Antenna.field.TxPath == 3 &&
		(pEntryPeer->HTCapability.MCSSet[2] != 0))
		MaxSize = 31; 		/* for 3x3, MaxSize use ((48KB/1.5KB) -1) = 31 */
	else
		MaxSize = 20;			/* for not 3x3, MaxSize use ((32KB/1.5KB) -1) ~= 20 */

	DBGPRINT(RT_DEBUG_TRACE, ("ba>WinSize=%d, MaxSize=%d, MaxPeerRxSize=%d\n", 
			*pWinSize, MaxSize, MaxPeerRxSize));

	MaxSize = min(MaxPeerRxSize, MaxSize);
	if ((*pWinSize) > MaxSize)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ba> reassign max win size from %d to %d\n", 
				*pWinSize, MaxSize));

		*pWinSize = MaxSize;
	}
}


UINT BA_Reorder_AMSDU_Annnounce(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket, UCHAR OpMode)
{
	PUCHAR			pData;
	USHORT			DataSize;
	UINT			nMSDU = 0;

	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);

	nMSDU = deaggregate_AMSDU_announce(pAd, pPacket, pData, DataSize, OpMode);

	return nMSDU;
}


void Announce_Reordering_Packet(RTMP_ADAPTER *pAd, struct reordering_mpdu *mpdu)
{
	PNDIS_PACKET pPacket;
	BOOLEAN opmode = pAd->OpMode;

	pPacket = mpdu->pPacket;
	if (mpdu->bAMSDU)
	{
		BA_Reorder_AMSDU_Annnounce(pAd, pPacket, mpdu->OpMode);
	}
	else
	{
		/* pass this 802.3 packet to upper layer or forward this packet to WM directly */
		Announce_or_Forward_802_3_Packet(pAd, pPacket, RTMP_GET_PACKET_WDEV(pPacket), opmode);
	}
}


/* Insert a reordering mpdu into sorted linked list by sequence no. */
BOOLEAN ba_reordering_mpdu_insertsorted(struct reordering_list *list, struct reordering_mpdu *mpdu)
{

	struct reordering_mpdu **ppScan = &list->next;

	while (*ppScan != NULL)
	{
		if (SEQ_SMALLER((*ppScan)->Sequence, mpdu->Sequence, MAXSEQ))
		{
			ppScan = &(*ppScan)->next;          
		}
		else if ((*ppScan)->Sequence == mpdu->Sequence)
		{
			/* give up this duplicated frame */
			return(FALSE);         
		}
		else
		{
			/* find position */
			break;
		}
	}

	mpdu->next = *ppScan;
	*ppScan = mpdu;
	list->qlen++;
	return TRUE;
}


/* Caller lock critical section if necessary */
static inline void ba_enqueue(struct reordering_list *list, struct reordering_mpdu *mpdu_blk)
{
	list->qlen++;
	mpdu_blk->next = list->next;
	list->next = mpdu_blk;
}


/* caller lock critical section if necessary */
static inline struct reordering_mpdu * ba_dequeue(struct reordering_list *list)
{
	struct reordering_mpdu *mpdu_blk = NULL;

	ASSERT(list);

		if (list->qlen)
		{
			list->qlen--;
			mpdu_blk = list->next;
			if (mpdu_blk)
			{
				list->next = mpdu_blk->next;
				mpdu_blk->next = NULL;
			}
		}
	return mpdu_blk;
}


static inline struct reordering_mpdu  *ba_reordering_mpdu_dequeue(struct reordering_list *list)
{
	return(ba_dequeue(list));
}


static inline struct reordering_mpdu  *ba_reordering_mpdu_probe(struct reordering_list *list)
{
	ASSERT(list);

	return(list->next);
}


/* free all resource for reordering mechanism */
void ba_reordering_resource_release(RTMP_ADAPTER *pAd)
{
	BA_TABLE *Tab;
	BA_REC_ENTRY *pBAEntry;
	struct reordering_mpdu *mpdu_blk;
	int i;

	Tab = &pAd->BATable;

	/* I.  release all pending reordering packet */
	NdisAcquireSpinLock(&pAd->BATabLock);
	for (i = 0; i < MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		pBAEntry = &Tab->BARecEntry[i];
		if (pBAEntry->REC_BA_Status != Recipient_NONE)
		{
			while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list)))
			{
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
	os_free_mem(pAd, pAd->mpdu_blk_pool.mem);
	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
}


/* 
 * Allocate all resource for reordering mechanism 
 */
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

	DBGPRINT(RT_DEBUG_TRACE, ("Allocate %d memory for BA reordering\n", (UINT32)(num*sizeof(struct reordering_mpdu))));

	/* allocate number of mpdu_blk memory */
	os_alloc_mem(pAd, (PUCHAR *)&mem, (num*sizeof(struct reordering_mpdu)));

	pAd->mpdu_blk_pool.mem = mem;

	if (mem == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can't Allocate Memory for BA Reordering\n"));
		return(FALSE);
	}

	/* build mpdu_blk free list */
	for (i=0; i<num; i++)
	{
		/* get mpdu_blk */
		mpdu_blk = (struct reordering_mpdu *) mem;
		/* initial mpdu_blk */
		NdisZeroMemory(mpdu_blk, sizeof(struct reordering_mpdu));
		/* next mpdu_blk */
		mem += sizeof(struct reordering_mpdu);
		/* insert mpdu_blk into freelist */
		ba_enqueue(freelist, mpdu_blk);
	}

	return(TRUE);
}

/* static int blk_count=0;  sample take off, no use */

static struct reordering_mpdu *ba_mpdu_blk_alloc(RTMP_ADAPTER *pAd)
{
	struct reordering_mpdu *mpdu_blk;

	NdisAcquireSpinLock(&pAd->mpdu_blk_pool.lock);
	mpdu_blk = ba_dequeue(&pAd->mpdu_blk_pool.freelist);
	if (mpdu_blk)
	{
		/* reset mpdu_blk */
		NdisZeroMemory(mpdu_blk, sizeof(struct reordering_mpdu));
	}
	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
	return mpdu_blk;
}

static void ba_mpdu_blk_free(PRTMP_ADAPTER pAd, struct reordering_mpdu *mpdu_blk)
{
	ASSERT(mpdu_blk);

	NdisAcquireSpinLock(&pAd->mpdu_blk_pool.lock);
	ba_enqueue(&pAd->mpdu_blk_pool.freelist, mpdu_blk);
	NdisReleaseSpinLock(&pAd->mpdu_blk_pool.lock);
}


static USHORT ba_indicate_reordering_mpdus_in_order(
												   IN PRTMP_ADAPTER    pAd, 
												   IN PBA_REC_ENTRY    pBAEntry,
												   IN USHORT           StartSeq)
{
	struct reordering_mpdu *mpdu_blk;
	USHORT  LastIndSeq = RESET_RCV_SEQ;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list)))
	{
			/* find in-order frame */
		if (!SEQ_STEPONE(mpdu_blk->Sequence, StartSeq, MAXSEQ))
			break;

		/* dequeue in-order frame from reodering list */
		mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
		if(mpdu_blk == NULL)
		{
			break;
		}
		/* pass this frame up */
		ANNOUNCE_REORDERING_PACKET(pAd, mpdu_blk);
		/* move to next sequence */
		StartSeq = mpdu_blk->Sequence;
		LastIndSeq = StartSeq;
		/* free mpdu_blk */
		ba_mpdu_blk_free(pAd, mpdu_blk);
	}

	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);

	/* update last indicated sequence */
	return LastIndSeq;
}

static void ba_indicate_reordering_mpdus_le_seq(
											   IN PRTMP_ADAPTER    pAd, 
											   IN PBA_REC_ENTRY    pBAEntry,
											   IN USHORT           Sequence)
{
	struct reordering_mpdu *mpdu_blk;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);
	while ((mpdu_blk = ba_reordering_mpdu_probe(&pBAEntry->list)))
		{
			/* find in-order frame */
		if ((mpdu_blk->Sequence == Sequence) || SEQ_SMALLER(mpdu_blk->Sequence, Sequence, MAXSEQ))
		{
			/* dequeue in-order frame from reodering list */
			mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list);
			if(mpdu_blk == NULL)
			{
				break;
			}
			/* pass this frame up */
			ANNOUNCE_REORDERING_PACKET(pAd, mpdu_blk);
			/* free mpdu_blk */
			ba_mpdu_blk_free(pAd, mpdu_blk);            
		}
		else
			{
				break;
			}
	}
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);   
}


static void ba_refresh_reordering_mpdus(RTMP_ADAPTER *pAd, BA_REC_ENTRY *pBAEntry)
{
	struct reordering_mpdu *mpdu_blk;

	NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

			/* dequeue in-order frame from reodering list */
	while ((mpdu_blk = ba_reordering_mpdu_dequeue(&pBAEntry->list)))
	{
			/* pass this frame up */
		ANNOUNCE_REORDERING_PACKET(pAd, mpdu_blk);

		pBAEntry->LastIndSeq = mpdu_blk->Sequence;
			ba_mpdu_blk_free(pAd, mpdu_blk);
			
		/* update last indicated sequence */                    
	}
	ASSERT(pBAEntry->list.qlen == 0);
	pBAEntry->LastIndSeq = RESET_RCV_SEQ;
	NdisReleaseSpinLock(&pBAEntry->RxReRingLock);   
}


/* static */
void ba_flush_reordering_timeout_mpdus(
									IN PRTMP_ADAPTER    pAd, 
									IN PBA_REC_ENTRY    pBAEntry,
									IN ULONG            Now32)

{
	USHORT Sequence;

    if ((pBAEntry == NULL) || (pBAEntry->list.qlen <= 0))
        return;

/*	if ((RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+REORDERING_PACKET_TIMEOUT)) &&*/
/*		 (pBAEntry->list.qlen > ((pBAEntry->BAWinSize*7)/8))) ||*/
/*		(RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(10*REORDERING_PACKET_TIMEOUT))) &&*/
/*		 (pBAEntry->list.qlen > (pBAEntry->BAWinSize/8)))*/
	if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(MAX_REORDERING_PACKET_TIMEOUT/6))) 
		 &&(pBAEntry->list.qlen > 0/*1*/)
		)
	{
		DBGPRINT(RT_DEBUG_TRACE,("timeout[%d] (%08lx-%08lx = %d > %d): %x, flush all!\n ", pBAEntry->list.qlen, Now32, (pBAEntry->LastIndSeqAtTimer), 
			   (int)((long) Now32 - (long)(pBAEntry->LastIndSeqAtTimer)), MAX_REORDERING_PACKET_TIMEOUT,
			   pBAEntry->LastIndSeq));
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		pBAEntry->LastIndSeqAtTimer = Now32;
	}
	else
#ifdef CONFIG_BA_REORDER_MONITOR		
	if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(pAd->BATable.ba_reordering_packet_timeout)))
		&& (pBAEntry->list.qlen > 0)
	   )
#else
	if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer+(REORDERING_PACKET_TIMEOUT)))
		&& (pBAEntry->list.qlen > 0)
	   )
#endif
		{
/*
		DBGPRINT(RT_DEBUG_OFF, ("timeout[%d] (%lx-%lx = %d > %d): %x, ", pBAEntry->list.qlen, Now32, (pBAEntry->LastIndSeqAtTimer),
			   (int)((long) Now32 - (long)(pBAEntry->LastIndSeqAtTimer)), REORDERING_PACKET_TIMEOUT,
			   pBAEntry->LastIndSeq));
*/
    		
		/* force LastIndSeq to shift to LastIndSeq+1*/
    		Sequence = (pBAEntry->LastIndSeq+1) & MAXSEQ;
    		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, Sequence);
    		pBAEntry->LastIndSeqAtTimer = Now32;
			pBAEntry->LastIndSeq = Sequence;
    		
    		/* indicate in-order mpdus*/
    		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, Sequence);
    		if (Sequence != RESET_RCV_SEQ)
    		{
    			pBAEntry->LastIndSeq = Sequence;
    		}

		DBGPRINT(RT_DEBUG_TRACE, ("%x, flush one!\n", pBAEntry->LastIndSeq));

	}
}


/* 
 * generate ADDBA request to 
 * set up BA agreement
 */
VOID BAOriSessionSetUp(
	IN RTMP_ADAPTER *pAd, 
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR TID,
	IN USHORT TimeOut,
	IN ULONG DelayTime,
	IN BOOLEAN isForced)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	USHORT Idx;
	BOOLEAN Cancelled;
	STA_TR_ENTRY *tr_entry;
	UCHAR BAWinSize = 0;

	ASSERT(TID < NUM_OF_TID);
	if (TID >= NUM_OF_TID)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Wrong TID %d!\n", TID));
		return;
	}
    
	if ((pAd->CommonCfg.BACapability.field.AutoBA != TRUE)  &&  (isForced == FALSE))
		return;

	/* if this entry is limited to use legacy tx mode, it doesn't generate BA.  */
	if (RTMPStaFixedTxMode(pAd, pEntry) != FIXED_TXMODE_HT)
		return;

	if ((pEntry->BADeclineBitmap & (1<<TID)) && (isForced == FALSE))
	{
		DelayTime = 3000; /* try again after 3 secs*/
/*		DBGPRINT(RT_DEBUG_TRACE, ("DeCline BA from Peer\n"));*/
/*		return;*/
	}

	Idx = pEntry->BAOriWcidArray[TID];
	if (Idx == 0)
	{
		/* allocate a BA session*/
		pBAEntry = BATableAllocOriEntry(pAd, &Idx);     
		if (pBAEntry == NULL)
		{
			DBGPRINT(RT_DEBUG_TRACE,("%s(): alloc BA session failed\n",
						__FUNCTION__));
			return;
		}
	}
	else
	{
		pBAEntry =&pAd->BATable.BAOriEntry[Idx];
	}

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
	
	if (!(pEntry->TXBAbitmap & (1<<TID)))
	{
		RTMPInitTimer(pAd, &pBAEntry->ORIBATimer, GET_TIMER_FUNCTION(BAOriSessionSetupTimeout), pBAEntry, FALSE);
	}
	else
		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);

	/* set timer to send ADDBA request */
	RTMPSetTimer(&pBAEntry->ORIBATimer, DelayTime);
}


VOID BAOriSessionAdd(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN FRAME_ADDBA_RSP *pFrame)
{
	BA_ORI_ENTRY  *pBAEntry = NULL;
	BOOLEAN Cancelled;
	UCHAR TID;
	USHORT Idx;

	UCHAR MaxPeerBufSize;
	STA_TR_ENTRY *tr_entry;

	TID = pFrame->BaParm.TID;
	Idx = pEntry->BAOriWcidArray[TID];  
	pBAEntry =&pAd->BATable.BAOriEntry[Idx];

	MaxPeerBufSize = 0;

	/* Start fill in parameters.*/
	if ((Idx !=0) && (pBAEntry->TID == TID) && (pBAEntry->ORI_BA_Status == Originator_WaitRes))
	{
		MaxPeerBufSize = (UCHAR)pFrame->BaParm.BufSize;
	
		{
			if (pAd->chipCap.hif_type == HIF_MT)
			{
				pBAEntry->BAWinSize = min(pBAEntry->BAWinSize, MaxPeerBufSize);
			}
			else
			{
				if (MaxPeerBufSize > 0)
					MaxPeerBufSize -= 1;
				else
					MaxPeerBufSize = 0;
				pBAEntry->BAWinSize = min(pBAEntry->BAWinSize, MaxPeerBufSize);
				BA_MaxWinSizeReasign(pAd, pEntry, &pBAEntry->BAWinSize);
			}
		}

		pBAEntry->TimeOutValue = pFrame->TimeOutValue;
		pBAEntry->amsdu_cap = pFrame->BaParm.AMSDUSupported;
		pBAEntry->ORI_BA_Status = Originator_Done;
		pAd->BATable.numDoneOriginator ++;
		
		/* reset sequence number */
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		pBAEntry->Sequence = BA_ORI_INIT_SEQ;
		
		/* Set Bitmap flag.*/
		pEntry->TXBAbitmap |= (1<<TID);
		RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);

		pBAEntry->ORIBATimer.TimerValue = 0;	/*pFrame->TimeOutValue;*/

		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->Sequence, 
												pBAEntry->BAWinSize, BA_SESSION_ORI);

		DBGPRINT(RT_DEBUG_TRACE, ("%s():TXBAbitmap=%x, AMSDUCap=%d, BAWinSize=%d, TimeOut=%ld\n",
					__FUNCTION__, pEntry->TXBAbitmap, pBAEntry->amsdu_cap,
					pBAEntry->BAWinSize, pBAEntry->ORIBATimer.TimerValue));

        /* Send BAR after BA session is build up */
        SendRefreshBAR(pAd, pEntry);

		if (pBAEntry->ORIBATimer.TimerValue)
			RTMPSetTimer(&pBAEntry->ORIBATimer, pBAEntry->ORIBATimer.TimerValue); /* in mSec */
	}

}


BOOLEAN BARecSessionAdd(
	IN RTMP_ADAPTER *pAd, 
	IN MAC_TABLE_ENTRY *pEntry,
	IN FRAME_ADDBA_REQ *pFrame)
{
	BA_REC_ENTRY *pBAEntry = NULL;
	BOOLEAN Status = TRUE, Cancelled;
	USHORT Idx;
	UCHAR TID, BAWinSize;


	ASSERT(pEntry);

	/* find TID*/
	TID = pFrame->BaParm.TID;

	BAWinSize = min(((UCHAR)pFrame->BaParm.BufSize), (UCHAR)pAd->CommonCfg.BACapability.field.RxBAWinLimit);

	if (BAWinSize == 0) {
		BAWinSize = pAd->CommonCfg.BACapability.field.RxBAWinLimit;
	}
	
	NdisAcquireSpinLock(&pAd->BATabLock);
	/* get software BA rec array index, Idx*/
	Idx = pEntry->BARecWcidArray[TID];


	if (Idx == 0)
	{
		/* allocate new array entry for the new session*/
		pBAEntry = BATableAllocRecEntry(pAd, &Idx);  
	}
	else
	{
		pBAEntry = &pAd->BATable.BARecEntry[Idx];
		/* flush all pending reordering mpdus*/
		NdisReleaseSpinLock(&pAd->BATabLock);
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		NdisAcquireSpinLock(&pAd->BATabLock);
	}

	DBGPRINT(RT_DEBUG_TRACE,("%s(%ld): Idx = %d, BAWinSize(req %d) = %d\n",
				__FUNCTION__, pAd->BATable.numAsRecipient, Idx, 
							 pFrame->BaParm.BufSize, BAWinSize));

	/* Start fill in parameters.*/
	if (pBAEntry != NULL)
	{
		ASSERT(pBAEntry->list.qlen == 0);

		pBAEntry->REC_BA_Status = Recipient_HandleRes;
		pBAEntry->BAWinSize = BAWinSize;
		pBAEntry->Wcid = pEntry->wcid;
		pBAEntry->TID = TID;
		pBAEntry->TimeOutValue = pFrame->TimeOutValue;
		pBAEntry->REC_BA_Status = Recipient_Accept;
		/* initial sequence number */
				/* initial LastIndSeq = SSN - 1*/
		pBAEntry->LastIndSeq = (pFrame->BaStartSeq.field.StartSeq - 1) & MAXSEQ;
		
		DBGPRINT(RT_DEBUG_TRACE, ("Start Seq = %08x\n",  pFrame->BaStartSeq.field.StartSeq));

		if (pEntry->RXBAbitmap & (1<<TID))
			RTMPCancelTimer(&pBAEntry->RECBATimer, &Cancelled);
		else
			RTMPInitTimer(pAd, &pBAEntry->RECBATimer, GET_TIMER_FUNCTION(BARecSessionIdleTimeout), pBAEntry, TRUE);

		/* Set Bitmap flag.*/
		pEntry->RXBAbitmap |= (1<<TID);
		pEntry->BARecWcidArray[TID] = Idx;
		pEntry->BADeclineBitmap &= ~(1<<TID);

		/* Set BA session mask in WCID table.*/
		RTMP_ADD_BA_SESSION_TO_ASIC(pAd, pEntry->wcid, TID, pBAEntry->LastIndSeq, 
												pBAEntry->BAWinSize, BA_SESSION_RECP);

		DBGPRINT(RT_DEBUG_TRACE, ("MACEntry[%d]RXBAbitmap = 0x%x. BARecWcidArray=%d\n",
				pEntry->wcid, pEntry->RXBAbitmap, pEntry->BARecWcidArray[TID]));
	}
	else
	{
		Status = FALSE;
		DBGPRINT(RT_DEBUG_TRACE,("Can't Accept ADDBA for %02x:%02x:%02x:%02x:%02x:%02x TID = %d\n", 
				PRINT_MAC(pEntry->Addr), TID));
	}
	NdisReleaseSpinLock(&pAd->BATabLock);
	return(Status);
}


BA_REC_ENTRY *BATableAllocRecEntry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_REC_ENTRY *pBAEntry = NULL;


	//NdisAcquireSpinLock(&pAd->BATabLock);

	if (pAd->BATable.numAsRecipient >= (MAX_LEN_OF_BA_REC_TABLE - 1))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("BA Recipeint Session (%ld) > %d\n", 
							pAd->BATable.numAsRecipient, (MAX_LEN_OF_BA_REC_TABLE - 1)));
		goto done;
	}

	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i=1; i < MAX_LEN_OF_BA_REC_TABLE; i++)
	{
		pBAEntry =&pAd->BATable.BARecEntry[i];
		if ((pBAEntry->REC_BA_Status == Recipient_NONE))
		{
			/* get one */
			pAd->BATable.numAsRecipient++;
			pBAEntry->REC_BA_Status = Recipient_USED;
			*Idx = i;
			break;
		}
	}

done:
	//NdisReleaseSpinLock(&pAd->BATabLock);
	return pBAEntry;
}


BA_ORI_ENTRY *BATableAllocOriEntry(RTMP_ADAPTER *pAd, USHORT *Idx)
{
	int i;
	BA_ORI_ENTRY *pBAEntry = NULL;

	NdisAcquireSpinLock(&pAd->BATabLock);
	if (pAd->BATable.numAsOriginator >= (MAX_LEN_OF_BA_ORI_TABLE - 1))
		goto done;
		
	/* reserve idx 0 to identify BAWcidArray[TID] as empty*/
	for (i=1; i<MAX_LEN_OF_BA_ORI_TABLE; i++)
	{
		pBAEntry =&pAd->BATable.BAOriEntry[i];
		if ((pBAEntry->ORI_BA_Status == Originator_NONE))
		{
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


VOID BATableFreeOriEntry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_ORI_ENTRY *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;

	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
		return;

	pBAEntry =&pAd->BATable.BAOriEntry[Idx];

	NdisAcquireSpinLock(&pAd->BATabLock);
	if (pBAEntry->ORI_BA_Status != Originator_NONE)
	{
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BAOriWcidArray[pBAEntry->TID] = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("%s: Wcid = %d, TID = %d\n", __FUNCTION__, pBAEntry->Wcid, pBAEntry->TID));

		if (pBAEntry->ORI_BA_Status == Originator_Done)
		{
			pAd->BATable.numDoneOriginator -= 1;
		 	pEntry->TXBAbitmap &= (~(1<<(pBAEntry->TID) ));
			DBGPRINT(RT_DEBUG_TRACE, ("BATableFreeOriEntry numAsOriginator= %ld\n", pAd->BATable.numAsRecipient));
			/* Erase Bitmap flag.*/
		}
	
		ASSERT(pAd->BATable.numAsOriginator != 0);

		pAd->BATable.numAsOriginator -= 1;
		
		pBAEntry->ORI_BA_Status = Originator_NONE;
		pBAEntry->Token = 0;
	}
	NdisReleaseSpinLock(&pAd->BATabLock);
	
}


VOID BATableFreeRecEntry(RTMP_ADAPTER *pAd, ULONG Idx)
{
	BA_REC_ENTRY    *pBAEntry = NULL;
	MAC_TABLE_ENTRY *pEntry;


	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_REC_TABLE))
		return;

	pBAEntry =&pAd->BATable.BARecEntry[Idx];

	NdisAcquireSpinLock(&pAd->BATabLock);	
	if (pBAEntry->REC_BA_Status != Recipient_NONE)
	{
		pEntry = &pAd->MacTab.Content[pBAEntry->Wcid];
		pEntry->BARecWcidArray[pBAEntry->TID] = 0;

		ASSERT(pAd->BATable.numAsRecipient != 0);

		pAd->BATable.numAsRecipient -= 1;

		pBAEntry->REC_BA_Status = Recipient_NONE;
	}
	NdisReleaseSpinLock(&pAd->BATabLock);
}


VOID BAOriSessionTearDown(
	INOUT RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR TID,
	IN BOOLEAN bPassive,
	IN BOOLEAN bForceSend)
{
	UINT Idx = 0;
	BA_ORI_ENTRY *pBAEntry;
	BOOLEAN Cancelled;

	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

    /* Clear WTBL2.dw15 when BA is deleted */
    RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, Wcid, TID, BA_SESSION_ORI);  
	
	/* Locate corresponding BA Originator Entry in BA Table with the (pAddr,TID).*/
	Idx = pAd->MacTab.Content[Wcid].BAOriWcidArray[TID];
	if ((Idx == 0) || (Idx >= MAX_LEN_OF_BA_ORI_TABLE))
	{
		if (bForceSend == TRUE)
		{
			/* force send specified TID DelBA*/
			MLME_DELBA_REQ_STRUCT   DelbaReq;   
			MLME_QUEUE_ELEM *Elem;
			os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
			if (Elem != NULL)
			{
				NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
				NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			
				COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
				DelbaReq.Wcid = Wcid;
				DelbaReq.TID = TID;
				DelbaReq.Initiator = ORIGINATOR;
				Elem->MsgLen  = sizeof(DelbaReq);			
				NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
				MlmeDELBAAction(pAd, Elem);
				os_free_mem(NULL, Elem);
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(bForceSend):alloc memory failed!\n", __FUNCTION__));
			}
		}

		return;
	}

	DBGPRINT(RT_DEBUG_TRACE,("%s===>Wcid=%d.TID=%d \n", __FUNCTION__, Wcid, TID));

	pBAEntry = &pAd->BATable.BAOriEntry[Idx];
	DBGPRINT(RT_DEBUG_TRACE,("\t===>Idx = %d, Wcid=%d.TID=%d, ORI_BA_Status = %d \n", Idx, Wcid, TID, pBAEntry->ORI_BA_Status));
	
	/* Prepare DelBA action frame and send to the peer.*/
	if ((bPassive == FALSE) && (TID == pBAEntry->TID) && (pBAEntry->ORI_BA_Status == Originator_Done))
	{
		MLME_DELBA_REQ_STRUCT   DelbaReq;   
		MLME_QUEUE_ELEM *Elem;
		os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
		if (Elem != NULL)
		{
			NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
			NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
		
			COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
			DelbaReq.Wcid = Wcid;
			DelbaReq.TID = pBAEntry->TID;
			DelbaReq.Initiator = ORIGINATOR;
			Elem->MsgLen  = sizeof(DelbaReq);			
			NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
			MlmeDELBAAction(pAd, Elem);
			os_free_mem(NULL, Elem);
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():alloc memory failed!\n", __FUNCTION__));
			return;
		}
	}
	RTMPCancelTimer(&pBAEntry->ORIBATimer, &Cancelled);
	BATableFreeOriEntry(pAd, Idx);

	if (bPassive)
	{
		/*BAOriSessionSetUp(pAd, &pAd->MacTab.Content[Wcid], TID, 0, 10000, TRUE);*/
	}
}

VOID BARecSessionTearDown(
						 IN OUT  PRTMP_ADAPTER   pAd, 
						 IN      UCHAR           Wcid,
						 IN      UCHAR           TID,
						 IN      BOOLEAN         bPassive)
{
	ULONG Idx = 0;
	BA_REC_ENTRY *pBAEntry;
			
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	NdisAcquireSpinLock(&pAd->BATabLock);
	/*  Locate corresponding BA Originator Entry in BA Table with the (pAddr,TID).*/
	Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
	if (Idx == 0) {
		NdisReleaseSpinLock(&pAd->BATabLock);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE,("%s===>Wcid=%d.TID=%d \n", __FUNCTION__, Wcid, TID));
	

	pBAEntry = &pAd->BATable.BARecEntry[Idx];
	DBGPRINT(RT_DEBUG_TRACE,("\t===>Idx = %ld, Wcid=%d.TID=%d, REC_BA_Status = %d \n", Idx, Wcid, TID, pBAEntry->REC_BA_Status));
	
	/* Prepare DelBA action frame and send to the peer.*/
	if ((TID == pBAEntry->TID) && (pBAEntry->REC_BA_Status == Recipient_Accept))
	{
		MLME_DELBA_REQ_STRUCT DelbaReq;
		BOOLEAN Cancelled;
				
		RTMPCancelTimer(&pBAEntry->RECBATimer, &Cancelled);         

		
		/* 1. Send DELBA Action Frame*/
		if (bPassive == FALSE)
		{
			MLME_QUEUE_ELEM *Elem;
			os_alloc_mem(NULL, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
			if (Elem != NULL)
			{
				NdisZeroMemory(&DelbaReq, sizeof(DelbaReq));
				NdisZeroMemory(Elem, sizeof(MLME_QUEUE_ELEM));
			
				COPY_MAC_ADDR(DelbaReq.Addr, pAd->MacTab.Content[Wcid].Addr);
				DelbaReq.Wcid = Wcid;
				DelbaReq.TID = TID;
				DelbaReq.Initiator = RECIPIENT;
				Elem->MsgLen  = sizeof(DelbaReq);			
				NdisMoveMemory(Elem->Msg, &DelbaReq, sizeof(DelbaReq));
				MlmeDELBAAction(pAd, Elem);
				os_free_mem(NULL, Elem);
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s():alloc memory failed!\n", __FUNCTION__));
				NdisReleaseSpinLock(&pAd->BATabLock);
				return;
			}
		}


		
		/* 2. Free resource of BA session*/
		/* flush all pending reordering mpdus */
		NdisReleaseSpinLock(&pAd->BATabLock);
		ba_refresh_reordering_mpdus(pAd, pBAEntry);
		NdisAcquireSpinLock(&pAd->BATabLock);

		//NdisAcquireSpinLock(&pAd->BATabLock);

		/* Erase Bitmap flag.*/
		pBAEntry->LastIndSeq = RESET_RCV_SEQ;
		pBAEntry->BAWinSize = 0;
		/* Erase Bitmap flag at software mactable*/
		pAd->MacTab.Content[Wcid].RXBAbitmap &= (~(1<<(pBAEntry->TID)));
		pAd->MacTab.Content[Wcid].BARecWcidArray[TID] = 0;

		RTMP_DEL_BA_SESSION_FROM_ASIC(pAd, Wcid, TID, BA_SESSION_RECP);

		//NdisReleaseSpinLock(&pAd->BATabLock);

	}
	NdisReleaseSpinLock(&pAd->BATabLock);
	BATableFreeRecEntry(pAd, Idx);
}


VOID BASessionTearDownALL(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	int i;

	for (i=0; i<NUM_OF_TID; i++)
	{
		BAOriSessionTearDown(pAd, Wcid, i, FALSE, FALSE);
		BARecSessionTearDown(pAd, Wcid, i, FALSE);
	}
}


/*
	==========================================================================
	Description:
		Retry sending ADDBA Reqest.
		
	IRQL = DISPATCH_LEVEL
	
	Parametrs:
	p8023Header: if this is already 802.3 format, p8023Header is NULL
	
	Return	: TRUE if put into rx reordering buffer, shouldn't indicaterxhere.
				FALSE , then continue indicaterx at this moment.
	==========================================================================
 */
VOID BAOriSessionSetupTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
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

	if ((pBAEntry->ORI_BA_Status == Originator_WaitRes) && (pBAEntry->Token < ORI_SESSION_MAX_RETRY))
	{
		MLME_ADDBA_REQ_STRUCT AddbaReq;  


		NdisZeroMemory(&AddbaReq, sizeof(AddbaReq));
		COPY_MAC_ADDR(AddbaReq.pAddr, pEntry->Addr);
		AddbaReq.Wcid = pEntry->wcid;
		AddbaReq.TID = pBAEntry->TID;
		AddbaReq.BaBufSize = pAd->CommonCfg.BACapability.field.RxBAWinLimit;
		AddbaReq.TimeOutValue = 0;
		AddbaReq.Token = pBAEntry->Token;
		MlmeEnqueue(pAd, ACTION_STATE_MACHINE, MT2_MLME_ADD_BA_CATE, sizeof(MLME_ADDBA_REQ_STRUCT), (PVOID)&AddbaReq, 0);
		RTMP_MLME_HANDLER(pAd);
		DBGPRINT(RT_DEBUG_TRACE,("BA Ori Session Timeout(%d) : Send ADD BA again\n", pBAEntry->Token));

		pBAEntry->Token++;
		RTMPSetTimer(&pBAEntry->ORIBATimer, ORI_BA_SESSION_TIMEOUT);
	}
	else
	{
		BATableFreeOriEntry(pAd, pEntry->BAOriWcidArray[pBAEntry->TID]);
	}
}


/*
	==========================================================================
	Description:
		Retry sending ADDBA Reqest.
		
	IRQL = DISPATCH_LEVEL
	
	Parametrs:
	p8023Header: if this is already 802.3 format, p8023Header is NULL
	
	Return	: TRUE if put into rx reordering buffer, shouldn't indicaterxhere.
				FALSE , then continue indicaterx at this moment.
	==========================================================================
 */
VOID BARecSessionIdleTimeout(
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

	if ((pBAEntry->REC_BA_Status == Recipient_Accept))
	{
		NdisGetSystemUpTime(&Now32);

		if (RTMP_TIME_AFTER((unsigned long)Now32, (unsigned long)(pBAEntry->LastIndSeqAtTimer + REC_BA_SESSION_IDLE_TIMEOUT)))
		{
			pAd = pBAEntry->pAdapter;
			/* flush all pending reordering mpdus */
			ba_refresh_reordering_mpdus(pAd, pBAEntry); 
			DBGPRINT(RT_DEBUG_TRACE, ("%ld: REC BA session Timeout\n", Now32));
		}
	}
}


VOID PeerAddBAReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Status = 1;
	UCHAR pAddr[6];
	FRAME_ADDBA_RSP ADDframe;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	PFRAME_ADDBA_REQ pAddreqFrame =  (PFRAME_ADDBA_REQ)(&Elem->Msg[0]);
	ULONG FrameLen/*, *ptemp*/;
	MAC_TABLE_ENTRY *pMacEntry;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("%s ==> (Wcid = %d)\n", __FUNCTION__, Elem->Wcid));

	/*ADDBA Request from unknown peer, ignore this.*/
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;
	
	pMacEntry = &pAd->MacTab.Content[Elem->Wcid];
	DBGPRINT(RT_DEBUG_TRACE,("BA - PeerAddBAReqAction----> \n"));
    if ((pMacEntry == NULL) || (pMacEntry->wdev == NULL))
    {
        DBGPRINT(RT_DEBUG_ERROR,(" %s: HIT the pMacEntry or pMacEntry->wdev NULL check\n", __FUNCTION__));
        return;
    }
	
	//ptemp = (PULONG)Elem->Msg;

	if (PeerAddBAReqActionSanity(pAd, Elem->Msg, Elem->MsgLen, pAddr))
	{
		if ((pAd->CommonCfg.bBADecline == FALSE) && IS_HT_STA(pMacEntry))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Rcv Wcid(%d) AddBAReq\n", Elem->Wcid));
			if (BARecSessionAdd(pAd, &pAd->MacTab.Content[Elem->Wcid], pAddreqFrame))
			{
#ifdef PEER_DELBA_TX_ADAPT
				Peer_DelBA_Tx_Adapt_Disable(pAd, &pAd->MacTab.Content[Elem->Wcid]);
#endif /* PEER_DELBA_TX_ADAPT */
				Status = 0;
			}
			else
				Status = 38; /* more parameters have invalid values*/
		}
		else
		{
			Status = 37; /* the request has been declined.*/
		}
	}

	if (IS_ENTRY_CLIENT(pMacEntry))
		ASSERT(pMacEntry->Sst == SST_ASSOC);

	/* 2. Always send back ADDBA Response */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE,("ACTION - PeerBAAction() allocate memory failed \n"));
		return;
	}

	NdisZeroMemory(&ADDframe, sizeof(FRAME_ADDBA_RSP));
	/* 2-1. Prepare ADDBA Response frame.*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pMacEntry))
		{
			apidx = pMacEntry->func_tb_idx;
#ifdef MAC_REPEATER_SUPPORT
			if (pMacEntry && pMacEntry->bReptCli)
				ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.ApCliTab[apidx].RepeaterCli[pMacEntry->MatchReptCliIdx].CurrentAddress, pAddr);
			else
#endif /* MAC_REPEATER_SUPPORT */
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, pAddr);
		}
		else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if (IS_ENTRY_WDS(pMacEntry))
		{
			apidx = pMacEntry->func_tb_idx;
			ActHeaderInit(pAd, &ADDframe.Hdr, pAddr, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_addr, pAddr);
		}
		else
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
#ifdef DOT11_VHT_AC
	if (pMacEntry && IS_VHT_STA(pMacEntry) && (Status == 0))
		ADDframe.BaParm.AMSDUSupported = pAddreqFrame->BaParm.AMSDUSupported;
	else
#endif /* DOT11_VHT_AC */
		ADDframe.BaParm.AMSDUSupported = 0;

#ifdef WFA_VHT_PF
	if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable)
		ADDframe.BaParm.AMSDUSupported = 1;
#endif /* WFA_VHT_PF */
	ADDframe.BaParm.TID = pAddreqFrame->BaParm.TID;
	ADDframe.BaParm.BufSize = min(((UCHAR)pAddreqFrame->BaParm.BufSize), (UCHAR)pAd->CommonCfg.BACapability.field.RxBAWinLimit);
	if (ADDframe.BaParm.BufSize == 0) {
		//ADDframe.BaParm.BufSize = 64;
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
	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("%s(%d): TID(%d), BufSize(%d) <== \n", __FUNCTION__, Elem->Wcid, ADDframe.BaParm.TID, 
							  ADDframe.BaParm.BufSize));
}


VOID PeerAddBARspAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	PFRAME_ADDBA_RSP pFrame = NULL;

	/*ADDBA Response from unknown peer, ignore this.*/
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s ==> Wcid(%d)\n", __FUNCTION__, Elem->Wcid));
	
	/*hex_dump("PeerAddBARspAction()", Elem->Msg, Elem->MsgLen);*/

	if (PeerAddBARspActionSanity(pAd, Elem->Msg, Elem->MsgLen))
	{
		pFrame = (PFRAME_ADDBA_RSP)(&Elem->Msg[0]);

		DBGPRINT(RT_DEBUG_TRACE, ("\t\t StatusCode = %d\n", pFrame->StatusCode));
		switch (pFrame->StatusCode)
		{
			case 0:
				/* I want a BAsession with this peer as an originator. */
				BAOriSessionAdd(pAd, &pAd->MacTab.Content[Elem->Wcid], pFrame);
#ifdef PEER_DELBA_TX_ADAPT
				Peer_DelBA_Tx_Adapt_Disable(pAd, &pAd->MacTab.Content[Elem->Wcid]);
#endif /* PEER_DELBA_TX_ADAPT */
				break;
			default:
				/* check status == USED ??? */
				BAOriSessionTearDown(pAd, Elem->Wcid, pFrame->BaParm.TID, TRUE, FALSE);
				break;
		}
		/* Rcv Decline StatusCode*/
		if ((pFrame->StatusCode == 37) 
            ) 
		{
			pAd->MacTab.Content[Elem->Wcid].BADeclineBitmap |= 1<<pFrame->BaParm.TID;
		}
	}
}

VOID PeerDelBAAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 

{
	PFRAME_DELBA_REQ    pDelFrame = NULL;
	
	DBGPRINT(RT_DEBUG_TRACE,("%s ==>\n", __FUNCTION__));
	/*DELBA Request from unknown peer, ignore this.*/
	if (PeerDelBAActionSanity(pAd, Elem->Wcid, Elem->Msg, Elem->MsgLen))
	{
		pDelFrame = (PFRAME_DELBA_REQ)(&Elem->Msg[0]);

#ifdef PEER_DELBA_TX_ADAPT
		if (pDelFrame->DelbaParm.TID == 0)
			Peer_DelBA_Tx_Adapt_Enable(pAd, &pAd->MacTab.Content[Elem->Wcid]);
#endif /* PEER_DELBA_TX_ADAPT */

		if (pDelFrame->DelbaParm.Initiator == ORIGINATOR)
		{
			DBGPRINT(RT_DEBUG_TRACE,("BA - PeerDelBAAction----> ORIGINATOR\n"));
			BARecSessionTearDown(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE);
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE,("BA - PeerDelBAAction----> RECIPIENT, Reason = %d\n",  pDelFrame->ReasonCode));
			/*hex_dump("DelBA Frame", pDelFrame, Elem->MsgLen);*/
			BAOriSessionTearDown(pAd, Elem->Wcid, pDelFrame->DelbaParm.TID, TRUE, FALSE);
		}
	}
}


BOOLEAN CntlEnqueueForRecv(
						  IN RTMP_ADAPTER *pAd, 
						  IN ULONG Wcid, 
						  IN ULONG MsgLen, 
						  IN PFRAME_BA_REQ pMsg) 
{
	PFRAME_BA_REQ pFrame = pMsg;
	PBA_REC_ENTRY pBAEntry;
	ULONG Idx;
	UCHAR TID;
	ULONG Now32;

	TID = (UCHAR)pFrame->BARControl.TID;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): BAR-Wcid(%ld), Tid (%d)\n", __FUNCTION__, Wcid, TID));
	/*hex_dump("BAR", (PCHAR) pFrame, MsgLen);*/
	/* Do nothing if the driver is starting halt state.*/
	/* This might happen when timer already been fired before cancel timer with mlmehalt*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return FALSE;

	/* First check the size, it MUST not exceed the mlme queue size*/
	if (MsgLen > MGMT_DMA_BUFFER_SIZE) /* 1600B */
	{
		DBGPRINT_ERR(("CntlEnqueueForRecv: frame too large, size = %ld \n", MsgLen));
		return FALSE;
	}
	else if (MsgLen != sizeof(FRAME_BA_REQ))
	{
		DBGPRINT_ERR(("CntlEnqueueForRecv: BlockAck Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}
	else if (MsgLen != sizeof(FRAME_BA_REQ))
	{
		DBGPRINT_ERR(("CntlEnqueueForRecv: BlockAck Request frame length size = %ld incorrect\n", MsgLen));
		return FALSE;
	}
		
	if ((Wcid < MAX_LEN_OF_MAC_TABLE) && (TID < 8))
	{
		/* if this receiving packet is from SA that is in our OriEntry. Since WCID <9 has direct mapping. no need search.*/
		Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
		pBAEntry = &pAd->BATable.BARecEntry[Idx];
	}
	else
	{
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("BAR(%ld) : Tid (%d) - %04x:%04x\n", Wcid, TID, pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq ));
	NdisGetSystemUpTime(&Now32);
	if (SEQ_SMALLER(pBAEntry->LastIndSeq, pFrame->BAStartingSeq.field.StartSeq, MAXSEQ))
	{
		LONG TmpSeq, seq;
		/*MTWF_LOG(DBG_CAT_PROTO, CATPROTO_BA, DBG_LVL_TRACE, ("BAR Seq = %x, LastIndSeq = %x\n", pFrame->BAStartingSeq.field.StartSeq, pBAEntry->LastIndSeq));*/

		seq = (pFrame->BAStartingSeq.field.StartSeq == 0) ? MAXSEQ :(pFrame->BAStartingSeq.field.StartSeq -1); 
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, seq);
		pBAEntry->LastIndSeq = seq;
		pBAEntry->LastIndSeqAtTimer = Now32;

			TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);
			
		if (TmpSeq != RESET_RCV_SEQ) {
				pBAEntry->LastIndSeq = TmpSeq;
		}
	}

	return TRUE;
}


/* Description : Send SMPS Action frame If SMPS mode switches. */
VOID SendSMPSAction(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR smps)
{
	struct wifi_dev *wdev;
	MAC_TABLE_ENTRY *pEntry;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_SMPS_ACTION Frame;
	ULONG FrameLen;


	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeADDBAAction() allocate memory failed \n"));
		return;
	}

	if (!VALID_WCID(Wcid))
	{
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_ERROR,("BA - Invalid WCID(%d)\n",  Wcid));
		return;
	}

	pEntry = &pAd->MacTab.Content[Wcid];
	wdev = pEntry->wdev;
	if (!wdev)
	{
		MlmeFreeMemory(pAd, pOutBuffer);
		DBGPRINT(RT_DEBUG_ERROR, ("BA - wdev is null\n"));
		return;
	}

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_APCLI(pEntry))
	{
#ifdef MAC_REPEATER_SUPPORT
		if (pEntry->bReptCli)
			ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].RepeaterCli[pEntry->MatchReptCliIdx].CurrentAddress, wdev->bssid);
		else
#endif /* MAC_REPEATER_SUPPORT */
		ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	}
	else
#endif /* APCLI_SUPPORT */
	ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, wdev->if_addr, wdev->bssid);
	Frame.Category = CATEGORY_HT;
	Frame.Action = SMPS_ACTION;
	switch (smps)
	{
		case MMPS_DISABLE:
			Frame.smps = 0;
			break;
		case MMPS_DYNAMIC:
			Frame.smps = 3;
			break;
		case MMPS_STATIC:
			Frame.smps = 1;
			break;
	}


	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					  sizeof(FRAME_SMPS_ACTION), &Frame,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
	DBGPRINT(RT_DEBUG_ERROR,("HT - %s( %d )  \n", __FUNCTION__, Frame.smps));
}


#define RADIO_MEASUREMENT_REQUEST_ACTION	0

typedef struct GNU_PACKED _BEACON_REQUEST {
	UCHAR	RegulatoryClass;
	UCHAR	ChannelNumber;
	USHORT	RandomInterval;
	USHORT	MeasurementDuration;
	UCHAR	MeasurementMode;
	UCHAR   BSSID[MAC_ADDR_LEN];
	UCHAR	ReportingCondition;
	UCHAR	Threshold;
	UCHAR   SSIDIE[2];			/* 2 byte*/
} BEACON_REQUEST;

typedef struct GNU_PACKED _MEASUREMENT_REQ
{
	UCHAR	ID;
	UCHAR	Length;
	UCHAR	Token;
	UCHAR	RequestMode;
	UCHAR	Type;
} MEASUREMENT_REQ;


#ifdef CONFIG_AP_SUPPORT
VOID SendBeaconRequest(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_RM_REQ_ACTION Frame;
	ULONG FrameLen;
	BEACON_REQUEST BeaconReq;
	MEASUREMENT_REQ MeasureReg;
	UCHAR apidx;

	if (IS_ENTRY_APCLI(&pAd->MacTab.Content[Wcid]))
		return;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	 /*Get an unused nonpaged memory*/
	if (NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("Radio - SendBeaconRequest() allocate memory failed \n"));
		return;
	}
	apidx = pAd->MacTab.Content[Wcid].func_tb_idx;
	ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, pAd->ApCfg.MBSSID[apidx].wdev.if_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid);

	Frame.Category = CATEGORY_RM;
	Frame.Action = RADIO_MEASUREMENT_REQUEST_ACTION;
	Frame.Token = 1;
	Frame.Repetition = 0;	/* executed once*/

	BeaconReq.RegulatoryClass = 32;		/* ?????*/
	BeaconReq.ChannelNumber = 255;		/* all channels*/
	BeaconReq.RandomInterval = 0;
	BeaconReq.MeasurementDuration = 10;	/* 10 TU*/
	BeaconReq.MeasurementMode = 1; 		/* Active mode */
	COPY_MAC_ADDR(BeaconReq.BSSID, 	BROADCAST_ADDR);
	BeaconReq.ReportingCondition = 254;	/* report not necesssary*/
	BeaconReq.Threshold = 0;			/* minimum RCPI*/
	BeaconReq.SSIDIE[0] = 0;
	BeaconReq.SSIDIE[1] = 0; 			/* wildcard SSID zero length */


	MeasureReg.ID = IE_MEASUREMENT_REQUEST;
	MeasureReg.Token = 0;
	MeasureReg.RequestMode = 0;
	MeasureReg.Type = 5;				/* Beacon Request*/
	MeasureReg.Length = sizeof(MEASUREMENT_REQ)+sizeof(BEACON_REQUEST)-2;

	MakeOutgoingFrame(pOutBuffer,               &FrameLen,
					  sizeof(FRAME_RM_REQ_ACTION),      &Frame,
					  sizeof(MEASUREMENT_REQ),			&MeasureReg,
					  sizeof(BEACON_REQUEST),			&BeaconReq,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
	DBGPRINT(RT_DEBUG_TRACE,("Radio - SendBeaconRequest\n"));
}
#endif /* CONFIG_AP_SUPPORT */


void convert_reordering_packet_to_preAMSDU_or_802_3_packet(
	IN RTMP_ADAPTER *pAd, 
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPkt;
	UCHAR Header802_3[LENGTH_802_3];
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
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

	pRxPkt = RTPKT_TO_OSPKT(pRxBlk->pRxPacket);

	RTMP_OS_PKT_INIT(pRxBlk->pRxPacket,
					get_netdev_from_bssid(pAd, wdev_idx),
					pRxBlk->pData, pRxBlk->DataSize);
	
	/* copy 802.3 header, if necessary */
	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
	{
		UCHAR VLAN_Size = 0;
		UCHAR *data_p;
		USHORT VLAN_VID = 0, VLAN_Priority = 0;

		// TODO: shiang-usw, fix me!!
#ifdef CONFIG_AP_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA) || RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS)) {
			/* Check if need to insert VLAN tag to the received packet */
			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
			if (VLAN_VID)
				VLAN_Size = LENGTH_802_1Q;
		}
#endif /* CONFIG_AP_SUPPORT */

		{
			data_p = OS_PKT_HEAD_BUF_EXTEND(pRxPkt, LENGTH_802_3+VLAN_Size);
			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
								Header802_3, LENGTH_802_3,
								data_p, TPID);
		}
	}
}


#define INDICATE_LEGACY_OR_AMSDU(_pAd, _pRxBlk, _wdev_idx)		\
	do																	\
	{																	\
    	if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_AMSDU))						\
    	{																\
    		Indicate_AMSDU_Packet(_pAd, _pRxBlk, _wdev_idx);		\
    	}																\
		else if (RX_BLK_TEST_FLAG(_pRxBlk, fRX_EAP))					\
		{																\
			Indicate_EAPOL_Packet(_pAd, _pRxBlk, _wdev_idx);		\
		}																\
    	else															\
    	{																\
    		Indicate_Legacy_Packet(_pAd, _pRxBlk, _wdev_idx);		\
    	}																\
	} while (0);


static VOID ba_enqueue_reordering_packet(
	IN RTMP_ADAPTER *pAd,
	IN BA_REC_ENTRY *pBAEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	struct reordering_mpdu *mpdu_blk;
	UINT16	Sequence = (UINT16) pRxBlk->pHeader->Sequence;

	mpdu_blk = ba_mpdu_blk_alloc(pAd);
	if ((mpdu_blk != NULL) &&
		(!RX_BLK_TEST_FLAG(pRxBlk, fRX_EAP)))
	{
		/* Write RxD buffer address & allocated buffer length */
		NdisAcquireSpinLock(&pBAEntry->RxReRingLock);

		mpdu_blk->Sequence = Sequence;
		mpdu_blk->OpMode = pRxBlk->OpMode;

		mpdu_blk->bAMSDU = RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU);

		convert_reordering_packet_to_preAMSDU_or_802_3_packet(pAd, pRxBlk, wdev_idx);

#ifdef HDR_TRANS_SUPPORT
		if (pRxBlk->bHdrRxTrans) {
			RTMP_OS_PKT_INIT(pRxBlk->pRxPacket,
					get_netdev_from_bssid(pAd, wdev_idx),
					pRxBlk->pTransData, pRxBlk->TransDataSize);
	}
	else
#endif
	{
		/* it is necessary for reordering packet to record 
			which BSS it come from
		*/
			RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
		}

		STATS_INC_RX_PACKETS(pAd, wdev_idx);        

		mpdu_blk->pPacket = pRxBlk->pRxPacket;

		if (ba_reordering_mpdu_insertsorted(&pBAEntry->list, mpdu_blk) == FALSE)
		{
			/* had been already within reordering list don't indicate */
			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_SUCCESS);                     
			ba_mpdu_blk_free(pAd, mpdu_blk);
	#ifdef NEW_IXIA_METHOD
			pAd->tr_ststic.rx[ALREADY_IN_ORDER]++;
	#endif
		}

		ASSERT((0<= pBAEntry->list.qlen)  && (pBAEntry->list.qlen <= pBAEntry->BAWinSize));
		NdisReleaseSpinLock(&pBAEntry->RxReRingLock);
	}
	else
	{
		ULONG Now32;

		if (mpdu_blk)
			ba_mpdu_blk_free(pAd, mpdu_blk);
		else {
		DBGPRINT(RT_DEBUG_ERROR,  ("!!! (%d) Can't allocate reordering mpdu blk\n",
								   pBAEntry->list.qlen));
		}
		/* 
		 * flush all pending reordering mpdus 
		 * and receving mpdu to upper layer
		 * make tcp/ip to take care reordering mechanism
		 */
		/*ba_refresh_reordering_mpdus(pAd, pBAEntry);*/
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, Sequence);

		pBAEntry->LastIndSeq = Sequence;
		INDICATE_LEGACY_OR_AMSDU(pAd, pRxBlk, wdev_idx);

		Sequence = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);

		if (Sequence != RESET_RCV_SEQ) {
			pBAEntry->LastIndSeq = Sequence;
		}

		NdisGetSystemUpTime(&Now32);
		pBAEntry->LastIndSeqAtTimer = Now32;
	}
}


#ifdef MAC_REPEATER_SUPPORT
VOID Rtmp_Chk_Reset_Ba_Sequence(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;
	UCHAR wcid = pRxBlk->wcid;
	MAC_TABLE_ENTRY *pEntry = NULL;

	UINT32  value;

	if (!pAd->ApCfg.bMACRepeaterEn)
		return;
	
	pEntry = &pAd->MacTab.Content[wcid];

	if (!pEntry)
		return;

	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0))
	{
		if (IS_ENTRY_APCLI(pEntry) && (pEntry->bReptCli == TRUE))
		{
			value = (pEntry->Addr[0] | (pEntry->Addr[1] << 8) |
		                (pEntry->Addr[2] << 16) | (pEntry->Addr[3] << 24));
		        	RTMP_IO_WRITE32(pAd, BSCR0, value);
					
		        	RTMP_IO_READ32(pAd, BSCR1, &value);
		        	value &= ~(BA_MAC_ADDR_47_32_MASK | RST_BA_TID_MASK | RST_BA_SEL_MASK);
		        	value |= BA_MAC_ADDR_47_32((pEntry->Addr[4] | (pEntry->Addr[5] << 8)));
		        	value |= (RST_BA_SEL(RST_BA_MAC_TID_MATCH) | RST_BA_TID(pRxBlk->TID) | START_RST_BA_SB);
		        	RTMP_IO_WRITE32(pAd, BSCR1, value);				
			pAd->RalinkCounters.BaSnResetCnt++;
		}
	}

}
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_BA_REORDER_MONITOR
void ba_timeout_flush(PRTMP_ADAPTER pAd)
{
	ULONG now;
	UINT32 idx0 = 0; 
	UINT32 idx1 = 0;
	PBA_REC_ENTRY pBAEntry = NULL;

	NdisGetSystemUpTime(&now);
	
	for (idx0 = 0; idx0 < 16; idx0++) {
		idx1 = 0;
		while ((pAd->BATable.ba_timeout_bitmap[idx0] != 0) && (idx1 < 32)) {

			if (pAd->BATable.ba_timeout_bitmap[idx0] & 0x1) {
				pBAEntry = &pAd->BATable.BARecEntry[(idx0 << 5) + idx1];
				ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, now);
				//pBAEntry->check_amsdu_miss = FALSE;
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
	POS_COOKIE obj = (POS_COOKIE) pAd->OS_Cookie;
	BOOLEAN need_check = FALSE;
	static int NeedFallBack=0;

	if ( pAd->MacTab.Size >= 20 )
	{
		pAd->BATable.ba_reordering_packet_timeout = MULTI_CLIENT_REORDERING_PACKET_TIMEOUT;
		NeedFallBack = 0;
	}
	else 
	{
		NeedFallBack ++;
		if (NeedFallBack >= 5) 
			pAd->BATable.ba_reordering_packet_timeout = REORDERING_PACKET_TIMEOUT;
	}

	if (!pAd->BATable.ba_timeout_check) {
		NdisGetSystemUpTime(&now);
		for (idx = 0; idx < MAX_LEN_OF_BA_REC_TABLE; idx++) {
			pBAEntry = &pAd->BATable.BARecEntry[idx];
		
			if ((pBAEntry->REC_BA_Status == Recipient_Accept) 
					&& (pBAEntry->list.qlen > 0)) {

				if (RTMP_TIME_AFTER((unsigned long)now, 
					(unsigned long)(pBAEntry->LastIndSeqAtTimer + pAd->BATable.ba_reordering_packet_timeout ))) {
					need_check = TRUE;	
					pAd->BATable.ba_timeout_bitmap[(idx >> 5)] |= (1 << (idx % 32));
				}
			}
		}

		if (need_check) {
			pAd->BATable.ba_timeout_check = need_check;
			RTMP_OS_TASKLET_SCHE(&obj->rx_done_task);
		}
	}	
}
#endif



/*
	==========================================================================
	Description:
		Indicate this packet to upper layer or put it into reordering buffer	
	
	Parametrs:
		pRxBlk         : carry necessary packet info 802.11 format
		wdev_idx : the packet received from which wdev

	Return	: 
			  none

	Note    :
	          the packet queued into reordering buffer need to cover to 802.3 format 
			  or pre_AMSDU format
	==========================================================================
*/
VOID Indicate_AMPDU_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	USHORT Idx;
	PBA_REC_ENTRY pBAEntry = NULL;
	UINT16 Sequence = pRxBlk->pHeader->Sequence;
	ULONG Now32;
	INT max_pkt_len, data_len;

#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans) {
		max_pkt_len = 1514;
		data_len = pRxBlk->TransDataSize;
	}
	else
#endif /* HDR_TRANS_SUPPORT */
	{
		max_pkt_len = MAX_RX_PKT_LEN;
		data_len = pRxBlk->DataSize;
	}

	if (!data_len) {
		/* release packet*/
		/* avoid processing with null paiload packets - QCA61X4A bug */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}


	if (!RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU) &&  (data_len > max_pkt_len))
	{
		static int err_size;

		err_size++;
		if (err_size > 20) {
			 DBGPRINT(RT_DEBUG_TRACE, ("AMPDU DataSize = %d\n", pRxBlk->DataSize));
			 hex_dump("802.11 Header", (UCHAR *)pRxBlk->pHeader, 24);
			 hex_dump("Payload", pRxBlk->pData, 64);
			 err_size = 0;
		}

		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
	{
		Idx = pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID];
		if (Idx == 0)
		{
			/* Rec BA Session had been torn down */
			INDICATE_LEGACY_OR_AMSDU(pAd, pRxBlk, wdev_idx); 
			return;
		}
		pBAEntry = &pAd->BATable.BARecEntry[Idx];
	}
	else
	{
		/* impossible !!! release packet*/
		ASSERT(0);
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	ASSERT(pBAEntry);

	/* update last rx time*/
	NdisGetSystemUpTime(&Now32);

	pBAEntry->rcvSeq = Sequence;

	ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);
	//pBAEntry->LastIndSeqAtTimer = Now32;

	
	/* Reset Last Indicate Sequence*/
	if (pBAEntry->LastIndSeq == RESET_RCV_SEQ)
	{
		ASSERT((pBAEntry->list.qlen == 0) && (pBAEntry->list.next == NULL));

		/* reset rcv sequence of BA session */
		pBAEntry->LastIndSeq = Sequence;
		pBAEntry->LastIndSeqAtTimer = Now32;
		INDICATE_LEGACY_OR_AMSDU(pAd, pRxBlk, wdev_idx);
		return;
	}

	
	/* I. Check if in order. */
	if (SEQ_STEPONE(Sequence, pBAEntry->LastIndSeq, MAXSEQ))
	{
		USHORT  LastIndSeq;

		pBAEntry->LastIndSeq = Sequence;
		INDICATE_LEGACY_OR_AMSDU(pAd, pRxBlk, wdev_idx);
 		LastIndSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);
		if (LastIndSeq != RESET_RCV_SEQ)
			pBAEntry->LastIndSeq = LastIndSeq;

		pBAEntry->LastIndSeqAtTimer = Now32;
	}
	/* II. Drop Duplicated Packet*/
	else if (Sequence == pBAEntry->LastIndSeq)
	{
#ifdef NEW_IXIA_METHOD
		pAd->tr_ststic.rx[DUP_SEQ_PKT]++;
#endif
		pBAEntry->nDropPacket++;
#ifdef MAC_REPEATER_SUPPORT
		Rtmp_Chk_Reset_Ba_Sequence(pAd, pRxBlk);
#endif /* MAC_REPEATER_SUPPORT */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	}
	/* III. Drop Old Received Packet*/
	else if (SEQ_SMALLER(Sequence, pBAEntry->LastIndSeq, MAXSEQ))
	{
#ifdef NEW_IXIA_METHOD
		pAd->tr_ststic.rx[DUP_SEQ_PKT]++;
#endif
		pBAEntry->nDropPacket++;
#ifdef MAC_REPEATER_SUPPORT
		Rtmp_Chk_Reset_Ba_Sequence(pAd, pRxBlk);
#endif /* MAC_REPEATER_SUPPORT */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	}
	/* IV. Receive Sequence within Window Size*/
	else if (SEQ_SMALLER(Sequence, (((pBAEntry->LastIndSeq+pBAEntry->BAWinSize+1)) & MAXSEQ), MAXSEQ))
	{
		ba_enqueue_reordering_packet(pAd, pBAEntry, pRxBlk, wdev_idx);
	}
	/* V. Receive seq surpasses Win(lastseq + nMSDU). So refresh all reorder buffer*/
	else
	{
		LONG WinStartSeq, TmpSeq;



		TmpSeq = Sequence - (pBAEntry->BAWinSize) +1;
		if (TmpSeq < 0)
			TmpSeq = (MAXSEQ+1) + TmpSeq;

		WinStartSeq = TmpSeq;
		ba_indicate_reordering_mpdus_le_seq(pAd, pBAEntry, (WinStartSeq - 1) & MAXSEQ);
		pBAEntry->LastIndSeq = (WinStartSeq - 1) & MAXSEQ; /* TmpSeq; */

		pBAEntry->LastIndSeqAtTimer = Now32;

		ba_enqueue_reordering_packet(pAd, pBAEntry, pRxBlk, wdev_idx);

		TmpSeq = ba_indicate_reordering_mpdus_in_order(pAd, pBAEntry, pBAEntry->LastIndSeq);
		if (TmpSeq != RESET_RCV_SEQ)
			pBAEntry->LastIndSeq = TmpSeq;
	}
}


VOID BaReOrderingBufferMaintain(RTMP_ADAPTER *pAd)
{
    ULONG Now32;
    UCHAR Wcid;
    USHORT Idx;
    UCHAR TID;
    PBA_REC_ENTRY pBAEntry = NULL;
    PMAC_TABLE_ENTRY pEntry = NULL;
 
    /* update last rx time*/
    NdisGetSystemUpTime(&Now32);
 
    for (Wcid = 1; Wcid < MAX_LEN_OF_MAC_TABLE; Wcid++)
    {
        pEntry = &pAd->MacTab.Content[Wcid];
        if (IS_ENTRY_NONE(pEntry))
            continue;
 
        for (TID= 0; TID < NUM_OF_TID; TID++)
        {
            Idx = pAd->MacTab.Content[Wcid].BARecWcidArray[TID];
            pBAEntry = &pAd->BATable.BARecEntry[Idx];
            ba_flush_reordering_timeout_mpdus(pAd, pBAEntry, Now32);
        }
    }
}


#ifdef PEER_DELBA_TX_ADAPT
static VOID Peer_DelBA_Tx_Adapt_Enable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
#ifdef MCS_LUT_SUPPORT

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): No need this for HIF_MT!\n", __FUNCTION__));
		return;
	}
#endif /* MT_MAC */

	if ((pAd->CommonCfg.bBADecline == TRUE) ||
		(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)))
	{
		/* we should not do this if bBADecline is TRUE or RGD is ON */
		return;
	}

	if(!RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) || !(pEntry->wcid < 128))
	{
		DBGPRINT(RT_DEBUG_WARN, 
				("%s(): Warning! This chip does not support HW Tx rate lookup.\n",
				__FUNCTION__));
		return;
	}
	
	if (pEntry)
	{	
		USHORT RegId = 0;
		UINT32 MacReg = 0, BitLUT;
		pEntry->bPeerDelBaTxAdaptEn = 1;
		/* Enable Tx Mac look up table */
		RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
		BitLUT = (MacReg & ((1 << 18)));
		if (BitLUT)
		{
			/* Keep original register setting in this flag */
			pEntry->bPeerDelBaTxAdaptEn |= BitLUT;
		}
		else
		{
			MacReg |= (1 << 18);
			RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);
		}
		RegId = 0x1C00 + (pEntry->Aid << 3);
		RTMP_IO_WRITE32(pAd, RegId, 0x4007); /* Legacy OFDM / no STBC / LGI / BW20 / MCS 7 */
		DBGPRINT(RT_DEBUG_TRACE,
				("%s():MacReg = 0x%08x, bPeerDelBaTxAdaptEn = 0x%x\n",
				__FUNCTION__, MacReg, pEntry->bPeerDelBaTxAdaptEn));
	}
#endif /* MCS_LUT_SUPPORT */
}


static VOID Peer_DelBA_Tx_Adapt_Disable(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
#ifdef MCS_LUT_SUPPORT
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): No need this for HIF_MT!\n", __FUNCTION__));
		return;
	}
#endif /* MT_MAC */

	if(!RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) || !(pEntry->wcid < 128))
	{
		DBGPRINT(RT_DEBUG_WARN, 
				("%s(): Warning! This chip does not support HW Tx rate lookup.\n",
				__FUNCTION__));
		return;
	}
	
	if (pEntry && pEntry->bPeerDelBaTxAdaptEn)
	{
		UINT32 BitLUT;
		BitLUT = (pEntry->bPeerDelBaTxAdaptEn & (1 << 18));
		if (!BitLUT)
		{
			UINT32 MacReg = 0;
			/* Disable Tx Mac look up table (Ressume original setting) */
			RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
			MacReg &= ~(1 << 18);
			RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);
			DBGPRINT(RT_DEBUG_TRACE,
					("%s():TX_FBK_LIMIT = 0x%08x\n",
					__FUNCTION__, MacReg));
		}
		/* TODO: ressume MSC rate of the MAC look up table? */
		pEntry->bPeerDelBaTxAdaptEn = 0;
		DBGPRINT(RT_DEBUG_TRACE,
				("%s():bPeerDelBaTxAdaptEn = 0x%x\n",
				__FUNCTION__, pEntry->bPeerDelBaTxAdaptEn));
	}
#endif /* MCS_LUT_SUPPORT */
}
#endif /* PEER_DELBA_TX_ADAPT */


#ifdef MAC_REPEATER_SUPPORT
VOID RxTrackingInit(struct wifi_dev *wdev)
{
    UCHAR j;
    RX_TRACKING_T *pTracking = NULL;
    RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;

    pTracking = &wdev->rx_tracking;
    pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;

    pTracking->TriggerNum = 0;

    pTaTidSeqMapEntry->RxDWlanIdx = 0xff;
    pTaTidSeqMapEntry->MuarIdx = 0xff;
    for (j = 0; j < 8; j++)
    {
        pTaTidSeqMapEntry->TID_SEQ[j] = 0xffff;
    }
    pTaTidSeqMapEntry->LatestTID = 0xff;
	NdisZeroMemory(pTaTidSeqMapEntry->LatestTA, MAC_ADDR_LEN);

    return;
}

VOID TaTidRecAndCmp(
	struct _RTMP_ADAPTER	*pAd,
	struct rxd_base_struct	*rx_base,
	UINT16			SN,
	BOOLEAN			isBAR,
	HEADER_802_11		*pHeader)
{
    struct wifi_dev *wdev = NULL;
    RX_TRACKING_T *pTracking = NULL;
    RX_TA_TID_SEQ_MAPPING *pTaTidSeqMapEntry = NULL;
    UCHAR Widx = rx_base->rxd_2.wlan_idx;
    UCHAR Tid = rx_base->rxd_2.tid;
    UCHAR MuarIdx = rx_base->rxd_1.bssid;
    UINT32 value = 0;
    MAC_TABLE_ENTRY *pEntry = NULL;
    struct _STA_TR_ENTRY *tr_entry = NULL;
	UINT32 i;
	UCHAR Zero[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if ( pAd->ApCfg.bBaSnResetDis)
		return;
	if (!pAd->ApCfg.bMACRepeaterEn)
		return;
	if (isBAR) {
		PFRAME_BA_REQ pFrame = (PFRAME_BA_REQ)(&(pHeader->FC));

		Tid = pFrame->BARControl.TID;

		for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
			if (!IS_ENTRY_NONE(&pAd->MacTab.Content[i])) {
				pEntry = &pAd->MacTab.Content[i];
				tr_entry = &pAd->MacTab.tr_entry[i];
				if (tr_entry)
					wdev = tr_entry->wdev;

				if (NdisEqualMemory(pFrame->Addr1, pEntry->ReptCliAddr, MAC_ADDR_LEN) &&
					NdisEqualMemory(pFrame->Addr2, pEntry->Addr, MAC_ADDR_LEN)) {
					Widx = pEntry->wcid;
					break;
				} else if (wdev && NdisEqualMemory(pEntry->ReptCliAddr, Zero, MAC_ADDR_LEN) &&
					NdisEqualMemory(pFrame->Addr2, pEntry->Addr, MAC_ADDR_LEN) &&
					NdisEqualMemory(pFrame->Addr1, wdev->if_addr, MAC_ADDR_LEN)) {
					Widx = pEntry->wcid;
					break;
				}
			}
		}
	}

	if (Widx >= MAX_LEN_OF_MAC_TABLE)
        return;

	tr_entry = &pAd->MacTab.tr_entry[Widx];
	
	if (tr_entry) {
		wdev = tr_entry->wdev;
	}

    if (wdev == NULL)
        return;

    if ((wdev->wdev_type != WDEV_TYPE_STA))
    {
        return;
    }

    pTracking = &wdev->rx_tracking;
    pTaTidSeqMapEntry = &pTracking->LastRxWlanIdx;
	if (isBAR) {
		if (NdisEqualMemory(pTaTidSeqMapEntry->LatestTA, pHeader->Addr2, MAC_ADDR_LEN))
			MuarIdx = pTaTidSeqMapEntry->MuarIdx;
	}
    if (pTaTidSeqMapEntry->RxDWlanIdx == 0xff)
    {/*first Rx pkt, just record it.*/
        pTaTidSeqMapEntry->RxDWlanIdx = Widx;
        pTaTidSeqMapEntry->MuarIdx = MuarIdx;
        pTaTidSeqMapEntry->TID_SEQ[Tid] = SN;
        pTaTidSeqMapEntry->LatestTID = Tid;
		NdisCopyMemory(pTaTidSeqMapEntry->LatestTA, pHeader->Addr2, MAC_ADDR_LEN);
    }
    else
    {
        /* compare*/
        if ((pTaTidSeqMapEntry->MuarIdx == MuarIdx) &&
            (pTaTidSeqMapEntry->RxDWlanIdx != Widx) &&
            pTaTidSeqMapEntry->LatestTID == Tid)
        {
            /*condition match, trigger scoreboard update*/

		DBGPRINT(RT_DEBUG_INFO,
				("last Widx = %d, muar_idx = %x, last TID = %d\n",
                            pTaTidSeqMapEntry->RxDWlanIdx,
                            pTaTidSeqMapEntry->MuarIdx,
                            pTaTidSeqMapEntry->LatestTID));

		DBGPRINT(RT_DEBUG_INFO,
				("new Widx = %d, muar_idx = %x, last TID = %d\n",
                                        Widx,
                                        MuarIdx,
                                        Tid));



			pEntry=&pAd->MacTab.Content[Widx];

			if (!pEntry)
				return;
			else {
				value = (pEntry->Addr[0] | (pEntry->Addr[1] << 8) |
			                (pEntry->Addr[2] << 16) | (pEntry->Addr[3] << 24));
			        	RTMP_IO_WRITE32(pAd, BSCR0, value);
						
			        	RTMP_IO_READ32(pAd, BSCR1, &value);
			        	value &= ~(BA_MAC_ADDR_47_32_MASK | RST_BA_TID_MASK | RST_BA_SEL_MASK);
			        	value |= BA_MAC_ADDR_47_32((pEntry->Addr[4] | (pEntry->Addr[5] << 8)));
			        	value |= (RST_BA_SEL(RST_BA_MAC_TID_MATCH) | RST_BA_TID(Tid) | START_RST_BA_SB);
			        	RTMP_IO_WRITE32(pAd, BSCR1, value);				
				pAd->RalinkCounters.BaSnResetCnt++;
	            		pTracking->TriggerNum++;
			}
        }

        /*update lastest rx information.*/
        pTaTidSeqMapEntry->RxDWlanIdx = Widx;
        pTaTidSeqMapEntry->MuarIdx = MuarIdx;
        pTaTidSeqMapEntry->TID_SEQ[Tid] = SN;
        pTaTidSeqMapEntry->LatestTID = Tid;
		NdisCopyMemory(pTaTidSeqMapEntry->LatestTA, pHeader->Addr2, MAC_ADDR_LEN);
    }
}
#endif /* MAC_REPEATER_SUPPORT */
#endif /* DOT11_N_SUPPORT */

