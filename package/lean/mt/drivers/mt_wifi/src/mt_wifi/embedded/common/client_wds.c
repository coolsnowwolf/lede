/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	client_wds.c

	Abstract:
*/


#ifdef CLIENT_WDS

#include "rt_config.h"

VOID CliWds_ProxyTabInit(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	ULONG i;
	NdisAllocateSpinLock(pAd, &pAd->ApCfg.CliWdsTabLock);
	os_alloc_mem(pAd, (UCHAR **)&(pAd->ApCfg.pCliWdsEntryPool), sizeof(CLIWDS_PROXY_ENTRY) * CLIWDS_POOL_SIZE);

	if (pAd->ApCfg.pCliWdsEntryPool) {
		NdisZeroMemory(pAd->ApCfg.pCliWdsEntryPool, sizeof(CLIWDS_PROXY_ENTRY) * CLIWDS_POOL_SIZE);
		initList(&pAd->ApCfg.CliWdsEntryFreeList);

		for (i = 0; i < CLIWDS_POOL_SIZE; i++)
			insertTailList(&pAd->ApCfg.CliWdsEntryFreeList, (RT_LIST_ENTRY *)(pAd->ApCfg.pCliWdsEntryPool + (ULONG)i));
	} else
		MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s Fail to alloc memory for pAd->CommonCfg.pCliWdsEntryPool", __func__));

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++)
		initList(&pAd->ApCfg.CliWdsProxyTb[idx]);

	return;
}


VOID CliWds_ProxyTabDestory(
	IN PRTMP_ADAPTER pAd)
{
	INT idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	NdisFreeSpinLock(&pAd->ApCfg.CliWdsTabLock);

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry =
			(PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[idx].pHead;

		while (pCliWdsEntry) {
			PCLIWDS_PROXY_ENTRY pCliWdsEntryNext = pCliWdsEntry->pNext;
			CliWdsEntyFree(pAd, pCliWdsEntry);
			pCliWdsEntry = pCliWdsEntryNext;
		}
	}

	if (pAd->ApCfg.pCliWdsEntryPool)
		os_free_mem(pAd->ApCfg.pCliWdsEntryPool);

	pAd->ApCfg.pCliWdsEntryPool = NULL;
	return;
}


PCLIWDS_PROXY_ENTRY CliWdsEntyAlloc(
	IN PRTMP_ADAPTER pAd)
{
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	RTMP_SEM_LOCK(&pAd->ApCfg.CliWdsTabLock);
	pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)removeHeadList(&pAd->ApCfg.CliWdsEntryFreeList);
	RTMP_SEM_UNLOCK(&pAd->ApCfg.CliWdsTabLock);
	return pCliWdsEntry;
}


VOID CliWdsEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PCLIWDS_PROXY_ENTRY pCliWdsEntry)
{
	RTMP_SEM_LOCK(&pAd->ApCfg.CliWdsTabLock);
	insertTailList(&pAd->ApCfg.CliWdsEntryFreeList, (RT_LIST_ENTRY *)pCliWdsEntry);
	RTMP_SEM_UNLOCK(&pAd->ApCfg.CliWdsTabLock);
	return;
}

VOID CliWdsEnryFreeAid(
	 IN PRTMP_ADAPTER pAd,
	 IN SHORT Aid)
{

	INT idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry =
			(PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[idx].pHead;
		while (pCliWdsEntry) {
			if (pCliWdsEntry->Aid == Aid) {
				delEntryList(&pAd->ApCfg.CliWdsProxyTb[idx], (RT_LIST_ENTRY *)pCliWdsEntry);
				CliWdsEntyFree(pAd, pCliWdsEntry);
			}
			pCliWdsEntry = pCliWdsEntry->pNext;
		}

	}
	return;
}


UCHAR *CliWds_ProxyLookup(RTMP_ADAPTER *pAd, UCHAR *pMac)
{
	UINT8 HashId = (*(pMac + 5) & (CLIWDS_HASH_TAB_SIZE - 1));
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)pAd->ApCfg.CliWdsProxyTb[HashId].pHead;

	while (pCliWdsEntry) {
		if (MAC_ADDR_EQUAL(pMac, pCliWdsEntry->Addr)) {
			ULONG Now;
			NdisGetSystemUpTime(&Now);
			pCliWdsEntry->LastRefTime = Now;

			/*FIXME: take aid as wcid shall be refined, let aid is aid, wcid is wcid.*/
			if (VALID_UCAST_ENTRY_WCID(pAd, pCliWdsEntry->Aid))
				return pAd->MacTab.Content[pCliWdsEntry->Aid].Addr;
			else
				return NULL;
		}

		pCliWdsEntry = pCliWdsEntry->pNext;
	}

	return NULL;
}


VOID CliWds_ProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN SHORT Aid,
	IN PUCHAR pMac)
{
	UINT8 HashId = (*(pMac + 5) & (CLIWDS_HASH_TAB_SIZE - 1));
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;

	if (CliWds_ProxyLookup(pAd, pMac) != NULL)
		return;

	pCliWdsEntry = CliWdsEntyAlloc(pAd);

	if (pCliWdsEntry) {
		ULONG Now;
		NdisGetSystemUpTime(&Now);
		pCliWdsEntry->Aid = Aid;
		COPY_MAC_ADDR(&pCliWdsEntry->Addr, pMac);
		pCliWdsEntry->LastRefTime = Now;
		pCliWdsEntry->pNext = NULL;
		insertTailList(&pAd->ApCfg.CliWdsProxyTb[HashId], (RT_LIST_ENTRY *)pCliWdsEntry);
	}

	return;
}


VOID CliWds_ProxyTabMaintain(
	IN PRTMP_ADAPTER pAd)
{
	ULONG idx;
	PCLIWDS_PROXY_ENTRY pCliWdsEntry;
	ULONG Now;
	NdisGetSystemUpTime(&Now);

	for (idx = 0; idx < CLIWDS_HASH_TAB_SIZE; idx++) {
		pCliWdsEntry = (PCLIWDS_PROXY_ENTRY)(pAd->ApCfg.CliWdsProxyTb[idx].pHead);

		while (pCliWdsEntry) {
			PCLIWDS_PROXY_ENTRY pCliWdsEntryNext = pCliWdsEntry->pNext;
		if (RTMP_TIME_AFTER(Now, pCliWdsEntry->LastRefTime + (ULONG)((ULONG)(CLI_WDS_ENTRY_AGEOUT) * OS_HZ / 1000))) {

				delEntryList(&pAd->ApCfg.CliWdsProxyTb[idx], (RT_LIST_ENTRY *)pCliWdsEntry);
				CliWdsEntyFree(pAd, pCliWdsEntry);
			}

			pCliWdsEntry = pCliWdsEntryNext;
		}
	}

	return;
}

#ifndef WDS_SUPPORT
MAC_TABLE_ENTRY *FindWdsEntry(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK * pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Wcid = %d, PhyMode = 0x%x\n", __func__,
	 pRxBlk->wcid, pRxBlk->rx_rate.field.MODE));
	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, pRxBlk->wcid, pRxBlk->Addr2, TRUE);
	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pRxBlk->Addr2, TRUE);

	/* Report to MLME, add WDS entry */
	if ((pEntry == NULL) && (pAd->WdsTab.Mode >= WDS_LAZY_MODE)) {
		UCHAR *pTmpBuf = pRxBlk->pData - LENGTH_802_11;

		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		NdisMoveMemory(pTmpBuf, pRxBlk->FC, LENGTH_802_11);
		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
					  pTmpBuf,
					  pRxBlk->DataSize + LENGTH_802_11,
					  pRxBlk->rx_signal.raw_rssi[0],
					  pRxBlk->rx_signal.raw_rssi[1],
					  pRxBlk->rx_signal.raw_rssi[2],
					  pRxBlk->rx_signal.raw_rssi[3],
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
					  pRxBlk->rx_signal.raw_snr[0],
					  pRxBlk->rx_signal.raw_snr[1],
					  pRxBlk->rx_signal.raw_snr[2],
					  pRxBlk->rx_signal.raw_snr[3],
#endif
					  (rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
					  0,
					  OPMODE_AP,
					  &pAd->ApCfg.MBSSID[pRxBlk->bss_idx].wdev,
					  pRxBlk->rx_rate.field.MODE);

		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		 ("!!! report WDS UC DATA (from %02x-%02x-%02x-%02x-%02x-%02x) to MLME (len=%d) !!!\n",
		  PRINT_MAC(pRxBlk->Addr2), pRxBlk->DataSize));

	}
	return pEntry;
}

MAC_TABLE_ENTRY *WdsTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount)
{
	ULONG WdsIndex;
	MAC_TABLE_ENTRY *pCurEntry = NULL, *pEntry = NULL;

	RETURN_ZERO_IF_PAD_NULL(pAd);
	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);

	do {
		pCurEntry = &pAd->MacTab.Content[wcid];
		WdsIndex = 0xff;
		if ((pCurEntry) && IS_ENTRY_WDS(pCurEntry))
			WdsIndex = pCurEntry->func_tb_idx;

		if (WdsIndex == 0xff)
			break;

		if (pAd->WdsTab.WdsEntry[WdsIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddr)) {
			if (bResetIdelCount) {
				pCurEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting because we need to migrate to tr_entry! */
				pAd->MacTab.tr_entry[pCurEntry->tr_tb_idx].NoDataIdleCount = 0;
			}
			pEntry = pCurEntry;
			break;
		}
	} while (FALSE);
	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}


MAC_TABLE_ENTRY *WdsTableLookup(RTMP_ADAPTER *pAd, UCHAR *addr, BOOLEAN bResetIdelCount)
{

	USHORT HashIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);
	HashIdx = MAC_ADDR_HASH_INDEX(addr);
	pEntry = pAd->MacTab.Hash[HashIdx];
	while (pEntry) {
		if (IS_ENTRY_WDS(pEntry) && MAC_ADDR_EQUAL(pEntry->Addr, addr)) {
			if (bResetIdelCount) {
				pEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting because we need to migrate to tr_entry! */
				pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
			}
			break;
		}
		pEntry = pEntry->pNext;
	}
	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}
#endif

#endif /* CLIENT_WDS */

