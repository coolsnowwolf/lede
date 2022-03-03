/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

	Module Name:
		cut_through.c

	Abstract:

	Note:
		All functions in this file must be PCI-depended, or you should move
		your functions to other files.

	Revision History:
	Who          When          What
	---------    ----------    ----------------------------------------------
*/

#include "rtmp_comm.h"
#include "cut_through.h"
#include "os/rt_linux_cmm.h"
#include "os/rt_drv.h"
#include "rt_os_util.h"
#include "rt_config.h"

VOID dump_ct_token_list(PKT_TOKEN_CB *tokenCb, INT type)
{
	PKT_TOKEN_QUEUE *token_q;
	PKT_TOKEN_LIST *token_list = NULL;
	INT idx;
	BOOLEAN dump;
	os_alloc_mem(NULL, (UCHAR **)&token_list, sizeof(PKT_TOKEN_LIST));

	if (!token_list) {
		printk("%s():alloc memory failed\n", __func__);
		return;
	}

	if ((type & CUT_THROUGH_TYPE_TX) == CUT_THROUGH_TYPE_TX)
		token_q = &tokenCb->tx_id_list;
	else if ((type & CUT_THROUGH_TYPE_RX) == CUT_THROUGH_TYPE_RX)
		token_q = &tokenCb->rx_id_list;
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Unkown type(%d)\n", __func__, type));
		os_free_mem(token_list);
		return;
	}

	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		NdisCopyMemory(token_list, token_q->list, sizeof(PKT_TOKEN_LIST));
		dump = TRUE;
	} else
		dump = FALSE;

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);

	if (dump == TRUE) {
		INT cnt = 0;
		printk("CutThrough Tx Token Queue Status:\n");
		printk("\tFree ID Head/Tail = %d/%d\n", token_list->id_head, token_list->id_tail);
		printk("\tFree ID Pool List:\n");

		for (idx = 0; idx < tokenCb->pkt_tx_tkid_aray; idx++) {
			if (token_list->free_id[idx] != tokenCb->pkt_tkid_invalid) {
				if ((cnt % 8) == 0)
					printk("\t\t");

				printk("ID[%d]=%d ", idx, token_list->free_id[idx]);

				if ((cnt % 8) == 7)
					printk("\n");

				cnt++;
			}
		}

		printk("\tPkt Token Pool List:\n");

		for (idx = 0; idx < tokenCb->pkt_tx_tkid_cnt; idx++) {
			if ((token_list->pkt_token[idx].pkt_buf != NULL) ||
				(token_list->pkt_token[idx].InOrder) ||
				(token_list->pkt_token[idx].rxDone)) {
				printk("\t\tPktToken[%d]=0x%p, InOrder/rxDone=%d/%d\n",
					   idx, token_list->pkt_token[idx].pkt_buf,
					   token_list->pkt_token[idx].InOrder,
					   token_list->pkt_token[idx].rxDone);
			}
		}
	} else {
		if ((type & CUT_THROUGH_TYPE_TX) == CUT_THROUGH_TYPE_TX)
			printk("CutThrough TX Token Queue not init yet!\n");
		else if ((type & CUT_THROUGH_TYPE_RX) == CUT_THROUGH_TYPE_RX)
			printk("CutThrough RX Token Queue not init yet!\n");
	}

	os_free_mem(token_list);
}


static INT cut_through_token_list_destroy(
	PKT_TOKEN_CB *pktTokenCb,
	PKT_TOKEN_QUEUE *token_q,
	INT type)
{
	INT idx;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);

	if (token_q->token_inited == TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): %p,%p\n",
				  __func__, token_q, &token_q->token_inited));
		RTMP_SEM_LOCK(&token_q->token_id_lock);
		token_q->token_inited = FALSE;
		RTMP_SEM_UNLOCK(&token_q->token_id_lock);

		for (idx = 0; idx < pktTokenCb->pkt_tx_tkid_cnt; idx++) {
			PKT_TOKEN_ENTRY *entry = &token_q->list->pkt_token[idx];

			if (entry->pkt_buf) {
				if (type == CUT_THROUGH_TYPE_TX) {
					PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr,
									 entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				}

				RELEASE_NDIS_PACKET(pktTokenCb->pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
			}
		}

		os_free_mem(token_q->list->free_id);
		os_free_mem(token_q->list->pkt_token);
		os_free_mem(token_q->list);
		token_q->list = NULL;
		NdisFreeSpinLock(&token_q->token_id_lock);
	}

	return TRUE;
}


static INT cut_through_token_list_init(
	PKT_TOKEN_CB *pktTokenCb,
	PKT_TOKEN_QUEUE *token_q)
{
	PKT_TOKEN_LIST *token_list;
	INT idx;

	if (token_q->token_inited == FALSE) {
		NdisAllocateSpinLock(pktTokenCb->pAd, &token_q->token_id_lock);
		os_alloc_mem(pktTokenCb->pAd, (UCHAR **)&token_q->list, sizeof(PKT_TOKEN_LIST));

		if (token_q->list == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): AllocMem failed!\n", __func__));
			NdisFreeSpinLock(&token_q->token_id_lock);
			return FALSE;
		}

		NdisZeroMemory(token_q->list, sizeof(PKT_TOKEN_LIST));
		token_list = token_q->list;
		token_list->id_head = 0;
		token_list->id_tail = pktTokenCb->pkt_tx_tkid_cnt;
		/*allocate freeid*/
		os_alloc_mem(pktTokenCb->pAd, (UCHAR **)&token_list->free_id, sizeof(UINT16)*pktTokenCb->pkt_tx_tkid_aray);
		os_zero_mem(token_list->free_id, sizeof(UINT16)*pktTokenCb->pkt_tx_tkid_aray);
		/*allocate pkt_token*/
		os_alloc_mem(pktTokenCb->pAd, (UCHAR **)&token_list->pkt_token, sizeof(PKT_TOKEN_ENTRY)*pktTokenCb->pkt_tx_tkid_cnt);
		os_zero_mem(token_list->pkt_token, sizeof(PKT_TOKEN_ENTRY)*pktTokenCb->pkt_tx_tkid_cnt);

		/*initial freeid*/
		for (idx = 0; idx < pktTokenCb->pkt_tx_tkid_cnt; idx++)
			token_list->free_id[idx] = idx;

		token_list->free_id[pktTokenCb->pkt_tx_tkid_cnt] = pktTokenCb->pkt_tkid_invalid;
		token_list->FreeTokenCnt = pktTokenCb->pkt_tx_tkid_cnt;
		token_list->TotalTxUsedTokenCnt = 0;
		token_list->TotalTxBackTokenCnt = 0;
		token_list->TotalTxTokenEventCnt = 0;
		token_list->TotalTxTokenCnt = 0;
#ifdef CUT_THROUGH_DBG
		token_list->UsedTokenCnt = 0;
		token_list->BackTokenCnt = 0;
		token_list->FreeAgg0_31 = 0;
		token_list->FreeAgg32_63 = 0;
		token_list->FreeAgg64_95 = 0;
		token_list->FreeAgg96_127 = 0;
		token_list->DropPktCnt = 0;

		for (idx = 0; idx < TIME_SLOT_NUMS; idx++) {
			token_list->UsedTokenCntRec[idx] = 0;
			token_list->BackTokenCntRec[idx] = 0;
			token_list->FreeAgg0_31Rec[idx] = 0;
			token_list->FreeAgg32_63Rec[idx] = 0;
			token_list->FreeAgg64_95Rec[idx] = 0;
			token_list->FreeAgg96_127Rec[idx] = 0;
			token_list->DropPktCntRec[idx] = 0;
		}

#endif
		token_q->token_inited = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): TokenList inited done!id_head/tail=%d/%d\n",
				  __func__, token_list->id_head, token_list->id_tail));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): %p,%p\n",
				  __func__, token_q, &token_q->token_inited));
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): TokenList already inited!shall not happened!\n",
				  __func__));

		if (!token_q->list) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): TokenList is NULL!\n", __func__));
			return FALSE;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\tlist.id_head=%d, list.id_tail=%d\n",
				  token_q->list->id_head, token_q->list->id_tail));
	}

	return TRUE;
}

PNDIS_PACKET cut_through_rx_deq(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	UINT8 *Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PNDIS_PACKET pkt_buf = NULL;
#ifdef CUT_THROUGH_DBG
	INT head[2] = { -1, -1}, tail[2] = { -1, -1};
#endif /* CUT_THROUGH_DBG */
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				PKT_TOKEN_ENTRY *entry = &token_list->pkt_token[token];
				pkt_buf = entry->pkt_buf;
				*Type = entry->Type;

				if (pkt_buf == NULL) {
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): buggy here? token ID(%d) without pkt!\n",
							 __func__, token));
					RTMP_SEM_UNLOCK(&token_q->token_id_lock);
					return pkt_buf;
				}

				entry->pkt_buf = NULL;
				entry->InOrder = FALSE;
				entry->rxDone = FALSE;
				entry->Drop = FALSE;
				entry->Type = TOKEN_NONE;
				token_list->free_id[token_list->id_tail] = token;
#ifdef CUT_THROUGH_DBG
				head[0] = token_list->id_head;
				tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
				INC_INDEX(token_list->id_tail, pktTokenCb->pkt_tx_tkid_aray);
				token_list->FreeTokenCnt++;
				token_list->TotalTxBackTokenCnt++;
#ifdef CUT_THROUGH_DBG
				token_list->BackTokenCnt++;
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */
	return pkt_buf;
}


UINT16 cut_through_rx_enq(
	PKT_TOKEN_CB *pktTokenCb,
	PNDIS_PACKET pkt,
	UINT8 Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	UINT16 idx = 0, token = pktTokenCb->pkt_tkid_invalid;
#ifdef CUT_THROUGH_DBG
	INT head[2] = { -1, -1}, tail[2] = { -1, -1};
#endif /* CUT_THROUGH_DBG */
	ASSERT(pkt);
	ASSERT(token_list);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_q->list) {
#ifdef CUT_THROUGH_DBG
			head[0] = token_list->id_head;
			tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
			idx = token_list->id_head;
			token = token_list->free_id[idx];

			if (token <= pktTokenCb->pkt_tx_tkid_max) {
				if (token_list->pkt_token[token].pkt_buf)
					RELEASE_NDIS_PACKET(pktTokenCb->pAd, token_list->pkt_token[token].pkt_buf, NDIS_STATUS_FAILURE);

				token_list->pkt_token[token].pkt_buf = pkt;
				token_list->pkt_token[token].Type = Type;
				token_list->free_id[idx] = pktTokenCb->pkt_tkid_invalid;
				INC_INDEX(token_list->id_head, pktTokenCb->pkt_tx_tkid_aray);
#ifdef CUT_THROUGH_DBG
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
				token_list->UsedTokenCnt++;
#endif /* CUT_THROUGH_DBG */
				token_list->FreeTokenCnt--;
				token_list->TotalTxUsedTokenCnt++;
			} else
				token = pktTokenCb->pkt_tkid_invalid;
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
	return token;
}

VOID cut_through_rx_pkt_assign(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	PNDIS_PACKET pkt)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
#ifdef CUT_THROUGH_DBG
	INT head[2] = { -1, -1}, tail[2] = { -1, -1};
#endif /* CUT_THROUGH_DBG */
	ASSERT(pkt);
	ASSERT(token_list);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_q->list) {
			if (token <= pktTokenCb->pkt_tx_tkid_max)
				token_list->pkt_token[token].pkt_buf = pkt;
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
	return;
}

PNDIS_PACKET cut_through_tx_deq(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	UINT8 *Type)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->tx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PNDIS_PACKET pkt_buf = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
#ifdef CUT_THROUGH_DBG
	INT head[2] = { -1, -1}, tail[2] = { -1, -1};
#endif /* CUT_THROUGH_DBG */
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				PKT_TOKEN_ENTRY *entry = &token_list->pkt_token[token];
				STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[entry->wcid];
				pkt_buf = entry->pkt_buf;
				*Type = entry->Type;

				if (pkt_buf == NULL) {
					MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): buggy here? token ID(%d) without pkt!\n",
							 __func__, token));
					RTMP_SEM_UNLOCK(&token_q->token_id_lock);
					return pkt_buf;
				}

				entry->pkt_buf = NULL;
				PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
				entry->InOrder = FALSE;
				entry->rxDone = FALSE;
				entry->Drop = FALSE;
				entry->Type = TOKEN_NONE;
				token_list->free_id[token_list->id_tail] = token;
#ifdef CUT_THROUGH_DBG
				head[0] = token_list->id_head;
				tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
				INC_INDEX(token_list->id_tail, pktTokenCb->pkt_tx_tkid_aray);
				token_list->FreeTokenCnt++;
				token_list->TotalTxBackTokenCnt++;
#ifdef CUT_THROUGH_DBG
				token_list->BackTokenCnt++;
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
				tr_entry->token_cnt--;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */
	return pkt_buf;
}


UINT16 cut_through_tx_enq(
	PKT_TOKEN_CB *pktTokenCb,
	PNDIS_PACKET pkt,
	UCHAR type,
	UINT8 wcid,
	NDIS_PHYSICAL_ADDRESS pkt_phy_addr,
	size_t pkt_len)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->tx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	UINT16 idx = 0, token = pktTokenCb->pkt_tkid_invalid;
#ifdef CUT_THROUGH_DBG
	INT head[2] = { -1, -1}, tail[2] = { -1, -1};
#endif /* CUT_THROUGH_DBG */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
	PKT_TOKEN_ENTRY *entry = NULL;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	ASSERT(pkt);
	ASSERT(token_list);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_q->list) {
#ifdef CUT_THROUGH_DBG
			head[0] = token_list->id_head;
			tail[0] = token_list->id_tail;
#endif /* CUT_THROUGH_DBG */
			idx = token_list->id_head;
			token = token_list->free_id[idx];

			if (token <= pktTokenCb->pkt_tx_tkid_max) {
				entry = &token_list->pkt_token[token];

				if (entry->pkt_buf) {
					PCI_UNMAP_SINGLE(pAd, entry->pkt_phy_addr, entry->pkt_len, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pktTokenCb->pAd, entry->pkt_buf, NDIS_STATUS_FAILURE);
				}

				entry->pkt_buf = pkt;
				entry->wcid = wcid;
				entry->Type = type;
				entry->pkt_phy_addr = pkt_phy_addr;
				entry->pkt_len = pkt_len;
				token_list->free_id[idx] = pktTokenCb->pkt_tkid_invalid;
				INC_INDEX(token_list->id_head, pktTokenCb->pkt_tx_tkid_aray);
#ifdef CUT_THROUGH_DBG
				head[1] = token_list->id_head;
				tail[1] = token_list->id_tail;
				token_list->UsedTokenCnt++;
#endif /* CUT_THROUGH_DBG */
				token_list->FreeTokenCnt--;
				token_list->TotalTxUsedTokenCnt++;
				tr_entry->token_cnt++;
			} else
				token = pktTokenCb->pkt_tkid_invalid;
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
#endif /* CUT_THROUGH_DBG */

	if (0) {
		printk("%s():Dump latest Free TokenList\n", __func__);

		for  (idx = 0; idx < pktTokenCb->pkt_tx_tkid_aray; idx++) {
			printk("\ttoken_list->free_id[%d]=%d\n", idx, token_list->free_id[idx]);

			if (idx < pktTokenCb->pkt_tx_tkid_cnt)
				printk("\ttoken_list->pkt_token[%d].pkt_buf=0x%p\n", idx, token_list->pkt_token[idx].pkt_buf);
		}
	}

#ifdef CUT_THROUGH_DBG
	NdisGetSystemUpTime(&pktTokenCb->tx_id_list.list->pkt_token[token].startTime);
#endif
	return token;
}


UINT cut_through_rx_in_order(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry;
	UINT inOrder = FALSE;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];
				inOrder = entry->InOrder;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder = %d\n",
			 __func__, token, inOrder));
#endif /* CUT_THROUGH_DBG */
	return inOrder;
}


UINT cut_through_rx_drop(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry;
	UINT Drop = FALSE;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];
				Drop = entry->Drop;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->Drop = %d\n",
			 __func__, token, Drop));
#endif /* CUT_THROUGH_DBG */
	return Drop;
}


INT cut_through_rx_mark_token_info(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token,
	UINT8 Drop)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry = NULL;
	INT32 Ret = FALSE;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];

				if (Drop)
					entry->Drop = TRUE;
				else
					entry->InOrder = TRUE;

				NdisGetSystemUpTime(&entry->endTime);
				Ret = TRUE;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder=%d\n",
			 __func__, token, (entry != NULL ? entry->InOrder : 0xffffffff)));
#endif /* CUT_THROUGH_DBG */
	return Ret;
}


INT cut_through_rx_mark_rxdone(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry = NULL;
	INT32 Ret = FALSE;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];
				entry->rxDone = TRUE;
				NdisGetSystemUpTime(&entry->startTime);
				Ret = TRUE;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->rxDone = %d\n",
			 __func__, token, (entry != NULL ? entry->rxDone : 0xffffffff)));
#endif /* CUT_THROUGH_DBG */
	return Ret;
}


LONG cut_through_inorder_time(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry = NULL;
	LONG timer_interval = 0x7ffffff;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];
				timer_interval = entry->startTime - entry->endTime;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->inOrder=%d\n",
			 __func__, token, (entry != NULL ? entry->InOrder : 0xffffffff)));
#endif /* CUT_THROUGH_DBG */
	return timer_interval;
}


UINT cut_through_rx_rxdone(
	PKT_TOKEN_CB *pktTokenCb,
	UINT16 token)
{
	PKT_TOKEN_QUEUE *token_q = &pktTokenCb->rx_id_list;
	PKT_TOKEN_LIST *token_list = token_q->list;
	PKT_TOKEN_ENTRY *entry;
	UINT rxDone = FALSE;
	ASSERT(token < pktTokenCb->pkt_tx_tkid_cnt);
	RTMP_SEM_LOCK(&token_q->token_id_lock);

	if (token_q->token_inited == TRUE) {
		if (token_list) {
			if (token < pktTokenCb->pkt_tx_tkid_cnt) {
				entry = &token_list->pkt_token[token];
				rxDone = entry->rxDone;
			} else
				MTWF_LOG(DBG_CAT_TOKEN, TOKEN_INFO, DBG_LVL_OFF, ("%s(): Invalid token ID(%d)\n", __func__, token));
		}
	}

	RTMP_SEM_UNLOCK(&token_q->token_id_lock);
#ifdef CUT_THROUGH_DBG
	MTWF_LOG(DBG_CAT_TOKEN, TOKEN_TRACE, DBG_LVL_TRACE, ("%s(): token[%d]->rxDone = %d\n",
			 __func__, token, rxDone));
#endif /* CUT_THROUGH_DBG */
	return rxDone;
}

INT cut_through_deinit(PKT_TOKEN_CB **ppPktTokenCb)
{
	PKT_TOKEN_CB *pktTokenCb;
	RTMP_ADAPTER *pAd;
#ifdef CUT_THROUGH_DBG
	BOOLEAN Cancelled;
#endif

	pktTokenCb = *ppPktTokenCb;

	if (pktTokenCb == NULL)
		return TRUE;

	pAd = (RTMP_ADAPTER *)pktTokenCb->pAd;
	cut_through_token_list_destroy(pktTokenCb, &pktTokenCb->tx_id_list, CUT_THROUGH_TYPE_TX);
	cut_through_token_list_destroy(pktTokenCb, &pktTokenCb->rx_id_list, CUT_THROUGH_TYPE_RX);
#ifdef CUT_THROUGH_DBG
	RTMPReleaseTimer(&pktTokenCb->TokenHistoryTimer, &Cancelled);
#endif
	NdisFreeSpinLock(&pktTokenCb->rx_order_notify_lock);

	os_free_mem((VOID *)pktTokenCb);
	*ppPktTokenCb = NULL;
	return TRUE;
}

#ifdef CUT_THROUGH_DBG
DECLARE_TIMER_FUNCTION(TokenHistoryExec);

VOID TokenHistoryExec(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)FunctionContext;
	PKT_TOKEN_LIST *tx_list = pktTokenCb->tx_id_list.list;
	PKT_TOKEN_LIST *rx_list = pktTokenCb->rx_id_list.list;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)(pktTokenCb->pAd);
	tx_list->UsedTokenCntRec[pktTokenCb->TimeSlot] = tx_list->UsedTokenCnt;
	tx_list->UsedTokenCnt = 0;
	tx_list->BackTokenCntRec[pktTokenCb->TimeSlot] = tx_list->BackTokenCnt;
	tx_list->BackTokenCnt = 0;
	tx_list->FreeAgg0_31Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg0_31;
	tx_list->FreeAgg0_31 = 0;
	tx_list->FreeAgg32_63Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg32_63;
	tx_list->FreeAgg32_63 = 0;
	tx_list->FreeAgg64_95Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg64_95;
	tx_list->FreeAgg64_95 = 0;
	tx_list->FreeAgg96_127Rec[pktTokenCb->TimeSlot] = tx_list->FreeAgg96_127;
	tx_list->FreeAgg96_127 = 0;
	rx_list->UsedTokenCntRec[pktTokenCb->TimeSlot] = rx_list->UsedTokenCnt;
	rx_list->UsedTokenCnt = 0;
	rx_list->BackTokenCntRec[pktTokenCb->TimeSlot] = rx_list->BackTokenCnt;
	rx_list->BackTokenCnt = 0;
	rx_list->FreeAgg0_31Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg0_31;
	rx_list->FreeAgg0_31 = 0;
	rx_list->FreeAgg32_63Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg32_63;
	rx_list->FreeAgg32_63 = 0;
	rx_list->FreeAgg64_95Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg64_95;
	rx_list->FreeAgg64_95 = 0;
	rx_list->FreeAgg96_127Rec[pktTokenCb->TimeSlot] = rx_list->FreeAgg96_127;
	rx_list->FreeAgg96_127 = 0;
	rx_list->DropPktCntRec[pktTokenCb->TimeSlot] = rx_list->DropPktCnt;
	rx_list->DropPktCnt = 0;
}
BUILD_TIMER_FUNCTION(TokenHistoryExec);
#endif


INT cut_through_init(VOID **ppPktTokenCb, VOID *pAd)
{
	PKT_TOKEN_CB *pktTokenCb;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)pAd;

	os_alloc_mem(pAd, (UCHAR **)&pktTokenCb, sizeof(PKT_TOKEN_CB));

	if (pktTokenCb == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s os_alloc_mem fail\n",
				  __func__));
		return FALSE;
	}

	NdisZeroMemory(pktTokenCb, sizeof(PKT_TOKEN_CB));
	pktTokenCb->pAd = pAd;

	NdisAllocateSpinLock(pktTokenCb->pAd, &pktTokenCb->rx_order_notify_lock);

	/* prepare host path can support token id and count */
	pktTokenCb->pkt_tx_tkid_max = DEFAUT_PKT_TX_TOKEN_ID_MAX;

#ifdef WHNAT_SUPPORT
	if (IS_ASIC_CAP(ad, fASIC_CAP_WHNAT) && ad->CommonCfg.whnat_en) {
		pktTokenCb->pkt_tx_tkid_max = DEFAUT_WHNAT_PKT_TX_TOKEN_ID_MAX;
	}
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): ct sw token number = %d\n",
			__func__, pktTokenCb->pkt_tx_tkid_max));

	pktTokenCb->pkt_tkid_invalid = (pktTokenCb->pkt_tx_tkid_max + 1);
	pktTokenCb->pkt_tx_tkid_cnt = (pktTokenCb->pkt_tx_tkid_max + 1);
	pktTokenCb->pkt_tx_tkid_aray = (pktTokenCb->pkt_tx_tkid_cnt + 1);
	cut_through_token_list_init(pktTokenCb, &pktTokenCb->tx_id_list);
	cut_through_token_list_init(pktTokenCb, &pktTokenCb->rx_id_list);
#ifdef RX_CUT_THROUGH
	pktTokenCb->cut_through_type = CUT_THROUGH_TYPE_BOTH;
#else
	pktTokenCb->cut_through_type = CUT_THROUGH_TYPE_TX;
#endif
	*ppPktTokenCb = pktTokenCb;

	if (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD))
		pktTokenCb->TxTokenLowWaterMark = 211;
	else
		pktTokenCb->TxTokenLowWaterMark = 5;

	pktTokenCb->TxTokenHighWaterMark = pktTokenCb->TxTokenLowWaterMark + 10;
	pktTokenCb->tx_token_state = TX_TOKEN_HIGH;
	pktTokenCb->RxTokenLowWaterMark = 5;
	pktTokenCb->RxTokenHighWaterMark = pktTokenCb->RxTokenLowWaterMark * 3;
#ifdef CUT_THROUGH_DBG
	pktTokenCb->TimeSlot = 0;
	RTMPInitTimer(pAd, &pktTokenCb->TokenHistoryTimer, GET_TIMER_FUNCTION(TokenHistoryExec), pktTokenCb, TRUE);
	RTMPSetTimer(&pktTokenCb->TokenHistoryTimer, 1000);
#endif
	return TRUE;
}


INT cut_through_set_mode(
	PKT_TOKEN_CB *pktTokenCb,
	UINT mode)
{
	if (pktTokenCb == NULL)
		return 0;

	if (mode <= CUT_THROUGH_TYPE_BOTH)
		pktTokenCb->cut_through_type = mode;
	else
		pktTokenCb->cut_through_type = 0;

	return 0;
}


INT cut_through_get_mode(
	PKT_TOKEN_CB *pktTokenCb)
{
	return pktTokenCb->cut_through_type;
}


INT Set_CtLowWaterMark_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 val;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	val = os_str_tol(arg, 0, 10);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Set CtLowWaterMark: %d\n", val));
	pktTokenCb->TxTokenLowWaterMark = val;
	pktTokenCb->TxTokenHighWaterMark = pktTokenCb->TxTokenLowWaterMark * 2;
	pktTokenCb->RxTokenLowWaterMark = val;
	pktTokenCb->RxTokenHighWaterMark = pktTokenCb->RxTokenLowWaterMark * 2;
	return TRUE;
}

inline BOOLEAN cut_through_get_token_state(RTMP_ADAPTER *pAd)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;

	return pktTokenCb->tx_token_state;
}

inline INT cut_through_set_token_state(RTMP_ADAPTER *pAd, BOOLEAN state)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;

	pktTokenCb->tx_token_state = state;

	return NDIS_STATUS_SUCCESS;
}

inline VOID cut_through_inc_token_full_cnt(RTMP_ADAPTER *pAd)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;

	pktTokenCb->TxTokenFullCnt++;
}

UINT32 cut_through_check_token_state(RTMP_ADAPTER *pAd)
{
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	BOOLEAN token_state = cut_through_get_token_state(pAd);
	UINT32 free_num = pktTokenCb->tx_id_list.list->FreeTokenCnt;

	if (token_state == TX_TOKEN_HIGH) {
		if (free_num >= pktTokenCb->TxTokenLowWaterMark)
			return TX_TOKEN_HIGH_TO_HIGH;
		else
			return TX_TOKEN_HIGH_TO_LOW;
	} else if (token_state == TX_TOKEN_LOW) {
		if (free_num > pktTokenCb->TxTokenHighWaterMark)
			return TX_TOKEN_LOW_TO_HIGH;
		else
			return TX_TOKEN_LOW_TO_LOW;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: unknow token state %d, free token number = %d",
			__func__, token_state, free_num));
		return TX_TOKEN_UNKNOW_CHANGE;
	}
}
