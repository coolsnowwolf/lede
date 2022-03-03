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
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

 #ifdef DOT11Z_TDLS_SUPPORT
#include <linux/timer.h>
#include "rt_config.h"
#include "dot11z_tdls.h"

UCHAR	TDLS_LLC_SNAP_WITH_CATEGORY[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d, PROTO_NAME_TDLS, CATEGORY_TDLS};
#ifdef WFD_SUPPORT
UCHAR	TDLS_LLC_SNAP_WITH_WFD_CATEGORY[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d, PROTO_NAME_TDLS, CATEGORY_VENDOR_SPECIFIC_WFD};
extern UCHAR	WIFIDISPLAY_OUI[];
#endif /* WFD_SUPPORT */
UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
extern UCHAR WPS_OUI[];
extern UCHAR	P2POUIBYTE[];

	
typedef struct
{
	UCHAR	regclass;		// regulatory class
	UCHAR	spacing;		// 0: 20Mhz, 1: 40Mhz
	UCHAR	channelset[16];	// max 15 channels, use 0 as terminator
} TDLS_REG_CLASS;

TDLS_REG_CLASS tdls_reg_class[] =
{
	{  1,  0, {36, 40, 44, 48, 0}},
	{  2,  0, {52, 56, 60, 64, 0}},
	{  3,  0, {149, 153, 157, 161, 0}},
	{  4,  0, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0}},
	{  5,  0, {165, 0}},
	{ 12, 0, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0}},
	{ 22, 1, {36, 44, 0}},
	{ 23, 1, {52, 60, 0}},
	{ 24, 1, {100, 108, 116, 124, 132, 0}},
	{ 25, 1, {149, 157, 0}},
	{ 26, 1, {149, 157, 0}},
	{ 27, 1, {40, 48, 0}},
	{ 28, 1, {56, 64, 0}},
	{ 29, 1, {104, 112, 120, 128, 136, 0}},
	{ 30, 1, {153, 161, 0}},
	{ 31, 1, {153, 161, 0}},
	{ 32, 1, {1, 2, 3, 4, 5, 6, 7, 0}},
	{ 33, 1, {5, 6, 7, 8, 9, 10, 11, 0}},
	{ 0,   0, {0}}			// end
};

VOID
TDLS_Table_Init(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	/* initialize TDLS allocate spin lock */
	NdisAllocateSpinLock(pAd, &pAd->StaCfg.TdlsInfo.TDLSEntryLock);
	NdisAllocateSpinLock(pAd, &pAd->StaCfg.TdlsInfo.TdlsChSwLock);
	NdisAllocateSpinLock(pAd, &pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

		pTDLS->Token = 0;
		pTDLS->Valid = FALSE;
		pTDLS->Status = TDLS_MODE_NONE;
#ifdef WFD_SUPPORT
		RTMPZeroMemory(&pTDLS->WfdEntryInfo, sizeof(WFD_ENTRY_INFO));
		pTDLS->WfdEntryInfo.wfd_PC = WFD_PC_TDLS;
#endif /* WFD_SUPPORT */
	}
}

VOID
TDLS_Table_Destory(
	IN	PRTMP_ADAPTER	pAd)
{
	UCHAR idx;
	BOOLEAN TimerCancelled;
	PRT_802_11_TDLS	pTDLS = NULL;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

#ifdef UAPSD_SUPPORT
	TDLS_UAPSDP_Release(pAd);
#endif /* UAPSD_SUPPORT */

	NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TDLSEntryLock);
	NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TdlsChSwLock);
	NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);


	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

		pTDLS->Token = 0;
		pTDLS->Valid = FALSE;
		pTDLS->Status = TDLS_MODE_NONE;

		RTMPCancelTimer(&pTDLS->Timer, &TimerCancelled);
		RTMPCancelTimer(&pTDLS->ChannelSwitchTimer, &TimerCancelled);
		RTMPCancelTimer(&pTDLS->ChannelSwitchTimeoutTimer, &TimerCancelled);
	}

	RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
	RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoOffChTimer, &TimerCancelled);
	RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, &TimerCancelled);
}

UCHAR TDLS_GetRegulatoryClass(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	ChannelWidth,
	IN UCHAR	TargetChannel)
{
	int i=0;
	UCHAR regclass = 0;

	do
	{
		if (tdls_reg_class[i].spacing == ChannelWidth)
		{
			int j=0;

			do
			{
				if (tdls_reg_class[i].channelset[j] == TargetChannel)
				{
					regclass = tdls_reg_class[i].regclass;
					break;
				}
				j++;
			} while (tdls_reg_class[i].channelset[j] != 0);
		}
		i++;
	} while (tdls_reg_class[i].regclass != 0);

	return regclass;
}

BOOLEAN
TDLS_IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel)

{
	INT i;

	for (i = 0; i < pAd->ChannelListNum; i++)
	{
		if (pAd->ChannelList[i].Channel == channel)
			break;
	}

	if (i == pAd->ChannelListNum)
		return FALSE;
	else
		return TRUE;
}


UCHAR
TDLS_GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction)
{
	CHAR ExtCh;

	if (Direction == EXTCHA_ABOVE)
		ExtCh = Channel + 4;
	else
		ExtCh = (Channel - 4) > 0 ? (Channel - 4) : 0;

	return ExtCh;
}

#ifdef TDLS_AUTOLINK_SUPPORT
VOID
TDLS_ClearEntryList(
	IN  PLIST_HEADER	pTdlsEnList)
{
	PLIST_ENTRY		pEntry = NULL;

	pEntry = pTdlsEnList->pHead;

	while (pEntry != NULL)
	{		
		removeHeadList(pTdlsEnList);
		os_free_mem(NULL, pEntry);
		pEntry = pTdlsEnList->pHead;
	}
	
	return;
}

PTDLS_DISCOVERY_ENTRY
TDLS_FindDiscoveryEntry(
	IN	PLIST_HEADER	pTdlsEnList,
	IN	PUCHAR			pMacAddr)
{
	PTDLS_DISCOVERY_ENTRY	pPeerEntry = NULL;
	PLIST_ENTRY				pListEntry = NULL;

	pListEntry = pTdlsEnList->pHead;
	pPeerEntry = (PTDLS_DISCOVERY_ENTRY)pListEntry;
	while (pPeerEntry != NULL)
	{
		if (NdisEqualMemory(pPeerEntry->Responder, pMacAddr, MAC_ADDR_LEN))
			return pPeerEntry;
		pListEntry = pListEntry->pNext;
		pPeerEntry = (PTDLS_DISCOVERY_ENTRY)pListEntry;
	}
	
	return NULL;
}

BOOLEAN
TDLS_InsertDiscoveryPeerEntryByMAC(
	IN	PLIST_HEADER pTdlsEnList,
	IN	PUCHAR pMacAddr,
	IN	BOOLEAN bConnected)
{
	PTDLS_DISCOVERY_ENTRY pTdlsPeer = NULL;

	pTdlsPeer = TDLS_FindDiscoveryEntry(pTdlsEnList, pMacAddr);
	if (pTdlsPeer)
	{
		NdisGetSystemUpTime(&pTdlsPeer->InitRefTime);
		pTdlsPeer->bFirstTime = FALSE;
		pTdlsPeer->bConnectedFirstTime = FALSE;
	}
	else
	{
		os_alloc_mem(NULL, (PUCHAR *)&pTdlsPeer, sizeof(TDLS_DISCOVERY_ENTRY));

		if (pTdlsPeer)
		{
			DBGPRINT(RT_DEBUG_ERROR,("\n!!! Add %02x:%02x:%02x:%02x:%02x:%02x to discovery table !!!\n",
									pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]));

			NdisZeroMemory(pTdlsPeer, sizeof(TDLS_DISCOVERY_ENTRY));
			NdisMoveMemory(pTdlsPeer->Responder, pMacAddr, MAC_ADDR_LEN);
			NdisGetSystemUpTime(&pTdlsPeer->InitRefTime);
			pTdlsPeer->CurrentState = TDLS_DISCOVERY_IDLE;
			pTdlsPeer->RetryCount = 0;
			pTdlsPeer->bFirstTime = TRUE;
			pTdlsPeer->bConnected = bConnected;
			insertTailList(pTdlsEnList, (PLIST_ENTRY)pTdlsPeer);
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!!%s : no memory!!!\n", __FUNCTION__));
			return FALSE;
		}
		
	}

	return TRUE;
}

VOID TDLS_DelDiscoveryEntryByMAC(
	IN	PLIST_HEADER	pTdlsEnList,
	IN  PUCHAR	pMacAddr)
{
	PLIST_ENTRY	pListEntry = NULL;
	PTDLS_DISCOVERY_ENTRY 	pPeerEntry = NULL;

	pPeerEntry = TDLS_FindDiscoveryEntry(pTdlsEnList, pMacAddr);
	pListEntry = (PLIST_ENTRY)pPeerEntry;

	if (pPeerEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR,("\n !!! Delete %02x:%02x:%02x:%02x:%02x:%02x from discovery table !!!\n",
							pPeerEntry->Responder[0],
							pPeerEntry->Responder[1],
							pPeerEntry->Responder[2],
							pPeerEntry->Responder[3],
							pPeerEntry->Responder[4],
							pPeerEntry->Responder[5]));
		delEntryList(pTdlsEnList, pListEntry);
		os_free_mem(NULL, pPeerEntry);
	}
}

VOID
TDLS_MaintainDiscoveryEntryList(
	IN PRTMP_ADAPTER 	pAd)
{
	ULONG now_time = 0;
	PTDLS_DISCOVERY_ENTRY pPeerEntry = NULL;
	PLIST_ENTRY	pListEntry = NULL, pTempListEntry = NULL;
	PLIST_HEADER pTdlsDiscoveryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;

	NdisGetSystemUpTime(&now_time);
	pListEntry = pTdlsDiscoveryEnList->pHead;
	pPeerEntry = (PTDLS_DISCOVERY_ENTRY)pListEntry;

	while (pPeerEntry != NULL)
	{
		if (pPeerEntry->bConnected)
		{
			if (RTMP_TIME_AFTER(now_time, pPeerEntry->InitRefTime + (pAd->StaCfg.TdlsInfo.TdlsRssiMeasurementPeriod * ((1000 * OS_HZ)/1000))))
			{
				BOOLEAN rv;
				UCHAR PeerMAC[MAC_ADDR_LEN];

				NdisMoveMemory(PeerMAC, pPeerEntry->Responder, MAC_ADDR_LEN);

				if (pPeerEntry->RetryCount >= 2)
				{
					/* tear down */
					if (INFRA_ON(pAd))
					{
						MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
						USHORT		Reason = REASON_UNSPECIFY;
						INT			idx;

						DBGPRINT(RT_DEBUG_ERROR,("\n !!! Will tear down %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
												pPeerEntry->Responder[0],
												pPeerEntry->Responder[1],
												pPeerEntry->Responder[2],
												pPeerEntry->Responder[3],
												pPeerEntry->Responder[4],
												pPeerEntry->Responder[5]));

						idx = TDLS_SearchLinkId(pAd, pPeerEntry->Responder);

						if (idx == -1 || idx == MAX_NUM_OF_TDLS_ENTRY)
						{
							DBGPRINT(RT_DEBUG_ERROR,("TDLS - can not find or full the LinkId!\n"));
						}
						else if (idx >= 0)
						{
							Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
							pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Valid	= FALSE;
							pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Status	= TDLS_MODE_NONE;
							TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[idx], Reason, FALSE);

							MlmeEnqueue(pAd,
										TDLS_STATE_MACHINE,
										MT2_MLME_TDLS_TEAR_DOWN,
										sizeof(MLME_TDLS_REQ_STRUCT),
										&MlmeTdlsReq,
										0);

						}
					}

					RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
					pTempListEntry = pListEntry->pNext;
					DBGPRINT(RT_DEBUG_ERROR,("\n !!! peer connect and retrycount > 2 Delete %02x:%02x:%02x:%02x:%02x:%02x from discovery table !!!\n",
												pPeerEntry->Responder[0],
												pPeerEntry->Responder[1],
												pPeerEntry->Responder[2],
												pPeerEntry->Responder[3],
												pPeerEntry->Responder[4],
												pPeerEntry->Responder[5]));
					delEntryList(pTdlsDiscoveryEnList, pListEntry);
					os_free_mem(NULL, pPeerEntry);
					pListEntry = pTempListEntry;
					RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
				}
				else
				{
					RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
					if (pPeerEntry->bConnectedFirstTime)
					{
						NdisGetSystemUpTime(&pPeerEntry->InitRefTime);
						pPeerEntry->RetryCount = 0;
						pPeerEntry->bConnected = TRUE;
						rv = TRUE;
					}
					else
					{
						rv = TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscoveryEnList,
																pPeerEntry->Responder,
																TRUE);
					}
					pPeerEntry->RetryCount++;
					RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);

					if (rv)
					{
						MlmeEnqueue(pAd, 
									TDLS_STATE_MACHINE, 
									MT2_MLME_TDLS_DISCOVER_REQ, 
									MAC_ADDR_LEN, 
									pPeerEntry->Responder,
									0);
					}
					pListEntry = pListEntry->pNext;
				}
			}
			else
			{
				pListEntry = pListEntry->pNext;
			}
		}
		else
		{
			if ((RTMP_TIME_AFTER(now_time, pPeerEntry->InitRefTime + TDLS_AUTO_DISCOVERY_INTERVAL)) &&
				(pPeerEntry->bFirstTime))
			{
				BOOLEAN rv;

				RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
				rv = TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscoveryEnList,
														pPeerEntry->Responder,
														FALSE);
				RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
				if (rv)
				{
					MlmeEnqueue(pAd, 
								TDLS_STATE_MACHINE, 
								MT2_MLME_TDLS_DISCOVER_REQ, 
								MAC_ADDR_LEN, 
								pPeerEntry->Responder,
								0);
				}

				pListEntry = pListEntry->pNext;
			}	
			else
			{
				if (RTMP_TIME_AFTER(now_time, pPeerEntry->InitRefTime + (5 * ((1000 * OS_HZ)/1000))))
				{
					if (pPeerEntry->CurrentState != TDLS_DISCOVERY_TO_SETUP_DONE)
					{
						PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;

						RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
						TDLS_InsertBlackEntryByMAC(pTdlsBlackEnList, pPeerEntry->Responder, TDLS_BLACK_AUTO_DISCOVERY);
						RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);

						RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
						pTempListEntry = pListEntry->pNext;
						DBGPRINT(RT_DEBUG_ERROR,("\n !!! peer disconnect  and over 5 sec Delete %02x:%02x:%02x:%02x:%02x:%02x from discovery table !!!\n",
													pPeerEntry->Responder[0],
													pPeerEntry->Responder[1],
													pPeerEntry->Responder[2],
													pPeerEntry->Responder[3],
													pPeerEntry->Responder[4],
													pPeerEntry->Responder[5]));
						delEntryList(pTdlsDiscoveryEnList, pListEntry);
						os_free_mem(NULL, pPeerEntry);
						pListEntry = pTempListEntry;
						RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
					}
					else
					{
						pListEntry = pListEntry->pNext;
					}
				}
				else
				{
					pListEntry = pListEntry->pNext;
				}
			}
		}
		pPeerEntry = (PTDLS_DISCOVERY_ENTRY)pListEntry;
	}
}

PTDLS_BLACK_ENTRY
TDLS_FindBlackEntry(
	IN	PLIST_HEADER	pTdlsEnList,
	IN	PUCHAR			pMacAddr)
{
	PTDLS_BLACK_ENTRY	pPeerEntry = NULL;
	PLIST_ENTRY			pListEntry = NULL;

	pListEntry = pTdlsEnList->pHead;
	pPeerEntry = (PTDLS_BLACK_ENTRY)pListEntry;

	while (pPeerEntry != NULL)
	{
		if (NdisEqualMemory(pPeerEntry->MacAddr, pMacAddr, MAC_ADDR_LEN))
			return pPeerEntry;

		pListEntry = pListEntry->pNext;
		pPeerEntry = (PTDLS_BLACK_ENTRY)pListEntry;
	}
	
	return NULL;
}

VOID
TDLS_InsertBlackEntryByMAC(
	IN	PLIST_HEADER	pTdlsEnList,
	IN	PUCHAR		pMacAddr,
	IN	UCHAR	CurrentState)
{
	PTDLS_BLACK_ENTRY		pTdlsBlack = NULL;
	
	pTdlsBlack = TDLS_FindBlackEntry(pTdlsEnList, pMacAddr);
	if (pTdlsBlack)
	{
		NdisGetSystemUpTime(&pTdlsBlack->InitRefTime);
		pTdlsBlack->CurrentState = CurrentState;
	}
	else
	{
		os_alloc_mem(NULL, (PUCHAR *)&pTdlsBlack, sizeof(TDLS_BLACK_ENTRY));

		if (pTdlsBlack)
		{
			DBGPRINT(RT_DEBUG_ERROR,("\n Add %02x:%02x:%02x:%02x:%02x:%02x to black table!!!\n",
								pMacAddr[0], pMacAddr[1], pMacAddr[2], pMacAddr[3], pMacAddr[4], pMacAddr[5]));
			NdisZeroMemory(pTdlsBlack, sizeof(TDLS_BLACK_ENTRY));
			NdisMoveMemory(pTdlsBlack->MacAddr, pMacAddr, MAC_ADDR_LEN);
			NdisGetSystemUpTime(&pTdlsBlack->InitRefTime);
			pTdlsBlack->CurrentState = CurrentState;
			insertTailList(pTdlsEnList, (PLIST_ENTRY)pTdlsBlack);
		}
		ASSERT(pTdlsBlack != NULL);
	}
}

VOID
TDLS_DelBlackEntryByMAC(
	IN	PLIST_HEADER		pTdlsEnList,
	IN  PUCHAR			pMacAddr)
{
	PLIST_ENTRY	pListEntry = NULL;
	PTDLS_BLACK_ENTRY 	pBlackEntry = NULL;

	pBlackEntry = TDLS_FindBlackEntry(pTdlsEnList, pMacAddr);
	pListEntry = (PLIST_ENTRY)pBlackEntry;
	
	if (pBlackEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR,("\n Delete %02x:%02x:%02x:%02x:%02x:%02x from black table!!!\n",
							pBlackEntry->MacAddr[0],
							pBlackEntry->MacAddr[1],
							pBlackEntry->MacAddr[2],
							pBlackEntry->MacAddr[3],
							pBlackEntry->MacAddr[4],
							pBlackEntry->MacAddr[5]));

		delEntryList(pTdlsEnList, pListEntry);
		os_free_mem(NULL, pBlackEntry);
	}
}

VOID
TDLS_MaintainBlackList(
	IN PRTMP_ADAPTER	pAd,
	IN PLIST_HEADER	pTdlsBlackenList)
{
	PTDLS_BLACK_ENTRY 	pBlackEntry = NULL;
	PLIST_ENTRY			pListEntry = NULL, pTempListEntry = NULL;
	ULONG				now_time = 0;

	NdisGetSystemUpTime(&now_time);
	pListEntry = pTdlsBlackenList->pHead;
	pBlackEntry = (PTDLS_BLACK_ENTRY)pListEntry;

	while (pBlackEntry != NULL)
	{
		if (pBlackEntry->CurrentState == TDLS_BLACK_AUTO_DISCOVERY)
		{
			if (RTMP_TIME_AFTER(now_time, pBlackEntry->InitRefTime + (pAd->StaCfg.TdlsInfo.TdlsAutoDiscoveryPeriod * ((1000 * OS_HZ)/1000))))
			{
				RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
				pTempListEntry = pListEntry->pNext;
				DBGPRINT(RT_DEBUG_ERROR,("\n balck auto discovery after %d secs Delete %02x:%02x:%02x:%02x:%02x:%02x from black table !!!\n",
										pAd->StaCfg.TdlsInfo.TdlsAutoDiscoveryPeriod,
										pBlackEntry->MacAddr[0],
										pBlackEntry->MacAddr[1],
										pBlackEntry->MacAddr[2],
										pBlackEntry->MacAddr[3],
										pBlackEntry->MacAddr[4],
										pBlackEntry->MacAddr[5]));
				delEntryList(pTdlsBlackenList, pListEntry);
				os_free_mem(NULL, pBlackEntry);
				pListEntry = pTempListEntry;
				RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
			}
			else
			{
				pListEntry = pListEntry->pNext;
			}
		}
		else if (pBlackEntry->CurrentState == TDLS_BLACK_TDLS_BY_TEARDOWN)
		{
			if (RTMP_TIME_AFTER(now_time, pBlackEntry->InitRefTime + (pAd->StaCfg.TdlsInfo.TdlsDisabledPeriodByTeardown * ((1000 * OS_HZ)/1000))))
			{
				RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
				pTempListEntry = pListEntry->pNext;
				DBGPRINT(RT_DEBUG_ERROR,("\n black tdls bt teardown after %d secs Delete %02x:%02x:%02x:%02x:%02x:%02x from black table!!!\n",
										pAd->StaCfg.TdlsInfo.TdlsDisabledPeriodByTeardown,
										pBlackEntry->MacAddr[0],
										pBlackEntry->MacAddr[1],
										pBlackEntry->MacAddr[2],
										pBlackEntry->MacAddr[3],
										pBlackEntry->MacAddr[4],
										pBlackEntry->MacAddr[5]));
				delEntryList(pTdlsBlackenList, pListEntry);
				os_free_mem(NULL, pBlackEntry);
				pListEntry = pTempListEntry;
				RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
			}
			else
			{
				pListEntry = pListEntry->pNext;
			}
		}
		else
		{
			pListEntry = pListEntry->pNext;
		}
		pBlackEntry = (PTDLS_BLACK_ENTRY)pListEntry;
	}
}

UCHAR
TDLS_ValidIdLookup(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr)
{
	INT	idIdx = MAX_NUM_OF_TDLS_ENTRY;
	PRT_802_11_TDLS	pTDLS = NULL;
	
	for (idIdx = 0; idIdx < MAX_NUM_OF_TDLS_ENTRY; idIdx++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idIdx];

		if (pTDLS->Valid && MAC_ADDR_EQUAL(pAddr, pTDLS->MacAddr))
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_ValidIdLookup - Find Link ID with Peer Address %02x:%02x:%02x:%02x:%02x:%02x(%d) \n", 
            	pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], idIdx));
			break;
		}
	}

	if (idIdx == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_INFO, ("!!! TDLS_ValidIdLookup -  not found !!!\n"));
		return MAX_NUM_OF_TDLS_ENTRY;
	}

	return idIdx;
}

VOID TDLS_AutoSetupByRcvFrame(RTMP_ADAPTER *pAd, HEADER_802_11 *pHeader)
{
	PTDLS_BLACK_ENTRY pTdlsBlackEntry = NULL;
	PTDLS_DISCOVERY_ENTRY pTdlsDiscoveryEntry = NULL;
	PLIST_HEADER pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;
	PLIST_HEADER pTdlsDiscoveryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;

	DBGPRINT(RT_DEBUG_TRACE, ("TDLS ===> TDLS_AutoSetupByRcvFrame\n"));

	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
	pTdlsBlackEntry = TDLS_FindBlackEntry(pTdlsBlackEnList, pHeader->Addr3);
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);

	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
	pTdlsDiscoveryEntry = TDLS_FindDiscoveryEntry(pTdlsDiscoveryEnList, pHeader->Addr3);
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);

	if ((pTdlsBlackEntry == NULL) && (pTdlsDiscoveryEntry == NULL))
	{
		BOOLEAN rv;
		UCHAR LinkId = 0xff;

		DBGPRINT(RT_DEBUG_ERROR, ("AutoSetupByRcvFrame trigger discovery request !!!\n"));

		RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
		rv = TDLS_InsertDiscoveryPeerEntryByMAC(pTdlsDiscoveryEnList,
												pHeader->Addr3,
												FALSE);
		RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
		LinkId = TDLS_ValidIdLookup(pAd, pHeader->Addr3);

		if (LinkId < MAX_NUM_OF_TDLS_ENTRY)
		{
			PTDLS_DISCOVERY_ENTRY pTdlsPeer = NULL;

			pTdlsPeer = TDLS_FindDiscoveryEntry(pTdlsDiscoveryEnList, pHeader->Addr3);
			if (pTdlsPeer)
			{
				pTdlsPeer->bConnectedFirstTime = TRUE;
				pTdlsPeer->bConnected = TRUE;
				pTdlsPeer->RetryCount = 0;
				pTdlsPeer->CurrentState = TDLS_DISCOVERY_TO_SETUP_DONE;
				DBGPRINT(RT_DEBUG_ERROR, ("TDLS peer entry already on table !!!\n"));
			}
		}
		else if (rv)
		{
			MlmeEnqueue(pAd, 
						TDLS_STATE_MACHINE, 
						MT2_MLME_TDLS_DISCOVER_REQ, 
						MAC_ADDR_LEN, 
						pHeader->Addr3,
						0);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("TDLS <=== TDLS_AutoSetupByRcvFrame\n"));
}
#endif /* TDLS_AUTOLINK_SUPPORT */

#ifdef WFD_SUPPORT
/*
==========================================================================
	Description: WFD
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_TunneledProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR	Category = 0x7F;
	UCHAR	OUI[3] = {0x50, 0x6F, 0x9A};
	UCHAR	FrameBodyType = 4; /* 4: Request. 5: Response. */
	PUCHAR	pWfdIeLen = NULL;
	UCHAR	WfdIEFixed[6] = {0xdd, 0x0c, 0x50, 0x6f, 0x9a, 0x0a};
	UCHAR	WfdIeLen = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("TDLS ===> TDLS_TunneledProbeRequest\n"));

	MAKE_802_3_HEADER(Header802_3, pMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						1,				&Category,
						3,				&OUI,
						1,				&FrameBodyType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;


	pWfdIeLen = pOutBuffer + FrameLen + 1;
	MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						6,							&WfdIEFixed,
						END_OF_ARGS);

	WfdIeLen += TempLen;
	FrameLen = FrameLen + TempLen;


	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_DEVICE_INFO, NULL, pOutBuffer + FrameLen, ACTION_WIFI_DIRECT);
	FrameLen = FrameLen + TempLen;
	WfdIeLen += TempLen; 

	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_ASSOCIATED_BSSID, NULL, pOutBuffer + FrameLen, ACTION_WIFI_DIRECT);
	FrameLen = FrameLen + TempLen;
	WfdIeLen += TempLen; 
	*pWfdIeLen = (WfdIeLen - 2);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS tunneled request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("TDLS <=== TDLS_TunneledProbeRequest\n"));

	return NStatus;
}

/*
==========================================================================
	Description: WFD
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_TunneledProbeResponse(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR	Category = 0x7F;
	UCHAR	OUI[3] = {0x50, 0x6F, 0x9A};
	UCHAR	FrameBodyType = 5; /* 4: Request. 5: Response. */
	PUCHAR	pWfdIeLen = NULL;
	UCHAR	WfdIEFixed[6] = {0xdd, 0x0c, 0x50, 0x6f, 0x9a, 0x0a};
	UCHAR	WfdIeLen = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("TDLS ===> TDLS_TunneledProbeResponse\n"));

	MAKE_802_3_HEADER(Header802_3, pMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	// Allocate buffer for transmitting message
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						1,				&Category,
						3,				&OUI,
						1,				&FrameBodyType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	pWfdIeLen = pOutBuffer + FrameLen + 1;
	MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						6,							&WfdIEFixed,
						END_OF_ARGS);

	WfdIeLen += TempLen;
	FrameLen = FrameLen + TempLen;

	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_DEVICE_INFO, NULL, pOutBuffer + FrameLen, ACTION_WIFI_DIRECT);
	FrameLen = FrameLen + TempLen;
	WfdIeLen += TempLen; 

	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_ASSOCIATED_BSSID, NULL, pOutBuffer + FrameLen, ACTION_WIFI_DIRECT);
	FrameLen = FrameLen + TempLen;
	WfdIeLen += TempLen; 

	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_ALTERNATE_MAC_ADDR, NULL, pOutBuffer + FrameLen, ACTION_WIFI_DIRECT);
	FrameLen = FrameLen + TempLen;
	WfdIeLen += TempLen;

	*pWfdIeLen = (WfdIeLen - 2);

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);

	hex_dump("TDLS tunneled request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("TDLS <=== TDLS_TunneledProbeResponse\n"));

	return NStatus;
}
#endif /* WFD_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsCapable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bTdlsCapable;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bTdlsCapable = simple_strtol(arg, 0, 10);

	if (!bTdlsCapable && pAd->StaCfg.TdlsInfo.bTDLSCapable)
	{
		/* tear	down local dls table entry */
		TDLS_LinkTearDown(pAd, TRUE);
	}

	pAd->StaCfg.TdlsInfo.bTDLSCapable = bTdlsCapable;
#ifdef WFD_SUPPORT
	if (pAd->StaCfg.TdlsInfo.bTDLSCapable)
			pAd->StaCfg.WfdCfg.PC = WFD_PC_TDLS;
	else
			pAd->StaCfg.WfdCfg.PC = WFD_PC_P2P;
#endif /* WFD_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_TdlsCapableProc::(bTdlsCapable=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.bTDLSCapable));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsSetup_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			macAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;
	RT_802_11_TDLS	Tdls;

	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid */

		AtoH(value, &macAddr[value_offset++], 1);
	}

	/* TDLS will not be supported when Adhoc mode */
	if (INFRA_ON(pAd))
	{
		if (pAd->StaActive.ExtCapInfo.TDLSProhibited == TRUE)
		{
			DBGPRINT(RT_DEBUG_OFF,("TDLS - Set_TdlsSetup_Proc() AP Prohibited TDLS !!!\n"));
			return FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("\n%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2],
									macAddr[3], macAddr[4], macAddr[5]));

		NdisZeroMemory(&Tdls, sizeof(RT_802_11_TDLS));
		Tdls.TimeOut = 0;
		COPY_MAC_ADDR(Tdls.MacAddr, macAddr);
		Tdls.Valid = 1;

		MlmeEnqueue(pAd, 
					MLME_CNTL_STATE_MACHINE, 
					RT_OID_802_11_SET_TDLS_PARAM, 
					sizeof(RT_802_11_TDLS), 
					&Tdls,
					0);

		RTMP_MLME_HANDLER(pAd);

		return TRUE;
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			macAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;
	CHAR			idx;

	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid */

		AtoH(value, &macAddr[value_offset++], 1);
	}

	/* TDLS will not be supported when Adhoc mode */
	if (INFRA_ON(pAd))
	{
		MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
		USHORT		Reason = REASON_UNSPECIFY;

		DBGPRINT(RT_DEBUG_TRACE,("\n%02x:%02x:%02x:%02x:%02x:%02x\n", macAddr[0], macAddr[1], macAddr[2],
									macAddr[3], macAddr[4], macAddr[5]));

		idx = TDLS_SearchLinkId(pAd, macAddr);

		if (idx == -1 || idx == MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_ERROR,("TDLS - Set_TdlsTearDown_Proc() can not find or full the LinkId!\n"));
			return FALSE;
		}

		if (idx >= 0)
		{
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Valid	= FALSE;
			pAd->StaCfg.TdlsInfo.TDLSEntry[idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);

			return TRUE;
		}
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsDiscoveryReq_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			PeerMacAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;

	if(strlen(arg) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  //Invalid

		AtoH(value, &PeerMacAddr[value_offset++], 1);
	}

	/* TDLS will not be supported when Adhoc mode */
	if (INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR,("\n Discovery Peer %02x:%02x:%02x:%02x:%02x:%02x\n",
								PeerMacAddr[0], PeerMacAddr[1], PeerMacAddr[2],
								PeerMacAddr[3], PeerMacAddr[4], PeerMacAddr[5]));


		MlmeEnqueue(pAd, 
					TDLS_STATE_MACHINE, 
					MT2_MLME_TDLS_DISCOVER_REQ, 
					MAC_ADDR_LEN, 
					PeerMacAddr,
					0);

		return TRUE;
	}

	return FALSE;
}

#ifdef WFD_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsTunneledReqProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UCHAR			PeerMacAddr[MAC_ADDR_LEN];
	PSTRING			value;
	INT				value_offset;

	if(strlen(arg) != 17)  //Mac address acceptable format 01:02:03:04:05:06 length 17
		return FALSE;

	for (value_offset=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  //Invalid

		AtoH(value, &PeerMacAddr[value_offset++], 1);
	}

	/* TDLS will not be supported when Adhoc mode */
	if (INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR,("\n Discovery Peer %02x:%02x:%02x:%02x:%02x:%02x\n",
								PeerMacAddr[0], PeerMacAddr[1], PeerMacAddr[2],
								PeerMacAddr[3], PeerMacAddr[4], PeerMacAddr[5]));

		MlmeEnqueue(pAd, 
					TDLS_STATE_MACHINE, 
					MT2_MLME_TDLS_TUNNELED_REQ, 
					MAC_ADDR_LEN, 
					PeerMacAddr,
					0);

		return TRUE;
	}

	return FALSE;
}
#endif /* WFD_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT Set_TdlsAcceptWeakSecurityProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING 		arg)
{
	BOOLEAN	bAcceptWeakSecurity;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bAcceptWeakSecurity = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.bAcceptWeakSecurity = bAcceptWeakSecurity;
	
	DBGPRINT(RT_DEBUG_TRACE, ("IF(ra%d) Set_TdlsAcceptWeakSecurityProc::(bAcceptWeakSecurity=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.bAcceptWeakSecurity));

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsTPKLifeTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT32	keyLifeTime;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	keyLifeTime = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime = keyLifeTime;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsTPKLifeTimeProc::(TdlsKeyLifeTime=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime));

	return TRUE;
}

#ifdef TDLS_AUTOLINK_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsAutoLinkProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	BOOLEAN	bTdlsAutoLink;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bTdlsAutoLink = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsAutoLink = (bTdlsAutoLink != 0 ? TRUE : FALSE);
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsAutoLinkProc::(TdlsAutoLink=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsAutoLink));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT
Set_TdlsRssiMeasurementPeriodProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT RssiMeasurementPeriod;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	RssiMeasurementPeriod = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsRssiMeasurementPeriod = RssiMeasurementPeriod;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsRssiMeasurementPeriodProc::(RssiMeasurementPeriod = %d secs)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsRssiMeasurementPeriod));

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT
Set_TdlsAutoDiscoveryPeriodProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT AutoDiscoveryPeriod;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	AutoDiscoveryPeriod = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsAutoDiscoveryPeriod = AutoDiscoveryPeriod;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsAutoDiscoveryPeriodProc::(AutoDiscoveryPeriod = %d secs)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsAutoDiscoveryPeriod));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT
Set_TdlsAutoSetupRssiThresholdProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	CHAR AutoSetupRssiThreshold;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	AutoSetupRssiThreshold = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsAutoSetupRssiThreshold = AutoSetupRssiThreshold;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsAutoSetupRssiThresholdProc::(AutoSetupRssiThreshold = %d dbm)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsAutoSetupRssiThreshold));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT
Set_TdlsDisabledPeriodByTeardownProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	USHORT DisabledPeriodByTeardown;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	DisabledPeriodByTeardown = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsDisabledPeriodByTeardown = DisabledPeriodByTeardown;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsDisabledPeriodByTeardownProc::(DisabledPeriodByTeardown = %d secs)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsDisabledPeriodByTeardown));

	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT
Set_TdlsAutoTeardownRssiThresholdProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	CHAR AutoTeardownRssiThreshold;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	AutoTeardownRssiThreshold = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold = AutoTeardownRssiThreshold;
	
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsAutoTeardownRssiThresholdProc::(AutoTeardownRssiThreshold = %d dbm)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold));

	return TRUE;
}
#endif /* TDLS_AUTOLINK_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsChannelSwitch_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg)
{
	UCHAR PeerMAC[6], TargetChannel;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	INT i;

/*
	The Set_TdlsChannelSwitchProc inupt string format should be xx:xx:xx:xx:xx:xx-d, 
		=>The six 2 digit hex-decimal number previous are the Mac address, 
		=>The seventh decimal number is the channel value.
*/

	if(strlen(arg) < 19)  //Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and channel value in decimal format.
		return FALSE;

	token = strchr(arg, DASH);

	if ((token != NULL) && (strlen(token)>1))
	{
		TargetChannel = (UCHAR) simple_strtol((token+1), 0, 10);
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&PeerMAC[i]), 1);
		}

		if(i != 6)
			return FALSE;

		DBGPRINT(RT_DEBUG_OFF, ("\nChannel Switch Peer %02x:%02x:%02x:%02x:%02x:%02x-%02x\n", 
								PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5], TargetChannel));

		/* TDLS will not be supported when Adhoc mode */
		if (INFRA_ON(pAd))
		{
			INT LinkId;
			PRT_802_11_TDLS	pTDLS = NULL;
			PTDLS_STRUCT pTdlsControl = NULL;

			LinkId = TDLS_SearchLinkId(pAd, PeerMAC);

			if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s(%d):  can not find from %02x:%02x:%02x:%02x:%02x:%02x on TDLS entry !!!\n",
									__FUNCTION__,__LINE__, PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5]));
				return FALSE;
			}

			pTdlsControl = &pAd->StaCfg.TdlsInfo;
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

			RtmpPrepareHwNullFrame(pAd,
								&pAd->MacTab.Content[pTDLS->MacTabMatchWCID],
								TRUE,
								FALSE,
								0,
								OPMODE_STA,
								PWR_ACTIVE,
								TRUE,
								1);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);

			if (pTDLS->Valid && (pTDLS->Status == TDLS_MODE_CONNECTED))
			{
				BOOLEAN TimerCancelled;

				RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoOffChTimer, &TimerCancelled);
				RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, &TimerCancelled);
				RTMPCancelTimer(&pTDLS->ChannelSwitchTimer, &TimerCancelled);
				RTMPCancelTimer(&pTDLS->ChannelSwitchTimeoutTimer, &TimerCancelled);

				if (TargetChannel != pAd->CommonCfg.Channel)
				{
					RTMP_SEM_LOCK(&pTdlsControl->TdlsChSwLock);
					pTdlsControl->bChannelSwitchInitiator = TRUE;
					pTDLS->bDoingPeriodChannelSwitch = TRUE;
					pTdlsControl->bDoingPeriodChannelSwitch = TRUE;
					pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;
					pTdlsControl->TdlsGoBackStartTime = 0;
					pTdlsControl->TdlsChannelSwitchRetryCount = 10;
					pTdlsControl->TdlsDesireChannel = TargetChannel;
					COPY_MAC_ADDR(pTdlsControl->TdlsDesireChSwMacAddr, PeerMAC);
					RTMP_SEM_UNLOCK(&pTdlsControl->TdlsChSwLock);

					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsChannelSwitchBW_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg)
{
	UCHAR	ChannelBW;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	ChannelBW = simple_strtol(arg, 0, 10);

	pAd->StaCfg.TdlsInfo.TdlsDesireChannelBW = ChannelBW;
	DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsChannelSwitchBW_Proc::(TdlsChannelSwitchBW=%d)\n", 
		pObj->ioctl_if, pAd->StaCfg.TdlsInfo.TdlsDesireChannelBW));

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT	Set_TdlsChannelSwitchDisable_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg)
{
	UCHAR idx = 0;
	BOOLEAN bChannelSwitchDisable;
	PRT_802_11_TDLS	pTDLS = NULL;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

	POS_COOKIE	pObj= (POS_COOKIE)pAd->OS_Cookie;

	bChannelSwitchDisable = simple_strtol(arg, 0, 10);

	if (INFRA_ON(pAd))
	{
		for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
		{
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

			if ((pTDLS->Valid) && (pTDLS->bDoingPeriodChannelSwitch) &&
				(pTDLS->Status == TDLS_MODE_CONNECTED))
			{
				RTMP_SEM_LOCK(&pTdlsControl->TdlsChSwLock);
				pTDLS->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->bChannelSwitchInitiator = FALSE;
				RTMP_SEM_UNLOCK(&pTdlsControl->TdlsChSwLock);
				break;
			}
		}

		if (idx < MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d) Set_TdlsChannelSwitchDisable_Proc\n", pObj->ioctl_if));

			return TRUE;
		}
	}

	return FALSE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_InitPeerEntryRateCapability(
	IN	PRTMP_ADAPTER pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN USHORT *pCapabilityInfo,
	IN UCHAR SupportRateLens,
	IN UCHAR *pSupportRates,
	IN UCHAR HtCapabilityLen,
	IN HT_CAPABILITY_IE *pHtCapability)
{
	UCHAR MaxSupportedRate = RATE_11;
	UCHAR MaxSupportedRateIn500Kbps = 0;
	UCHAR idx;

    for (idx = 0; idx < SupportRateLens; idx++)
    {
        if (MaxSupportedRateIn500Kbps < (pSupportRates[idx] & 0x7f)) 
            MaxSupportedRateIn500Kbps = pSupportRates[idx] & 0x7f;
    }

	switch (MaxSupportedRateIn500Kbps)
	{
		case 108: MaxSupportedRate = RATE_54;   break;
		case 96:  MaxSupportedRate = RATE_48;   break;
		case 72:  MaxSupportedRate = RATE_36;   break;
		case 48:  MaxSupportedRate = RATE_24;   break;
		case 36:  MaxSupportedRate = RATE_18;   break;
		case 24:  MaxSupportedRate = RATE_12;   break;
		case 18:  MaxSupportedRate = RATE_9;    break;
		case 12:  MaxSupportedRate = RATE_6;    break;
		case 22:  MaxSupportedRate = RATE_11;   break;
		case 11:  MaxSupportedRate = RATE_5_5;  break;
		case 4:   MaxSupportedRate = RATE_2;    break;
		case 2:   MaxSupportedRate = RATE_1;    break;
		default:  MaxSupportedRate = RATE_11;   break;
	}

	pEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxDesiredRate, MaxSupportedRate);

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	}
	else
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}

	pEntry->MaxHTPhyMode.field.BW = BW_20;
	pEntry->MinHTPhyMode.field.BW = BW_20;
				
#ifdef DOT11_N_SUPPORT
	pEntry->HTCapability.MCSSet[0] = 0;
	pEntry->HTCapability.MCSSet[1] = 0;
	pEntry->HTCapability.MCSSet[2] = 0;

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if ((HtCapabilityLen != 0) && (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED))
	{
		UCHAR	j, bitmask; /*k,bitmask; */
		CHAR    ii;

		DBGPRINT(RT_DEBUG_TRACE,
				("TDLS - Receive Peer HT Capable STA from %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(pEntry->Addr)));

		if ((pHtCapability->HtCapInfo.GF) &&
			(pAd->CommonCfg.DesiredHtPhy.GF) &&
			(pAd->StaActive.SupportedHtPhy.GF))
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
		}
		else
		{	
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
			pAd->MacTab.fAnyStationNonGF = TRUE;
			pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
		}

		if ((pHtCapability->HtCapInfo.ChannelWidth) &&
			(pAd->CommonCfg.DesiredHtPhy.ChannelWidth))
		{
			pEntry->MaxHTPhyMode.field.BW= BW_40;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor40)&(pHtCapability->HtCapInfo.ShortGIfor40));
		}
		else
		{	
			pEntry->MaxHTPhyMode.field.BW = BW_20;
			pEntry->MaxHTPhyMode.field.ShortGI = ((pAd->CommonCfg.DesiredHtPhy.ShortGIfor20)&(pHtCapability->HtCapInfo.ShortGIfor20));
			pAd->MacTab.fAnyStation20Only = TRUE;
		}

		/* find max fixed rate */
		for (ii = 23; ii >= 0; ii--) /* 3*3 */
		{
			j = ii/8;
			bitmask = (1<<(ii-(j*8)));
			if ( (pAd->StaCfg.wdev.DesiredHtPhyInfo.MCSSet[j]&bitmask) && (pHtCapability->MCSSet[j]&bitmask))
			{
				pEntry->MaxHTPhyMode.field.MCS = ii;
				break;
			}

			if (ii==0)
				break;
		}

		if (pAd->StaCfg.wdev.DesiredTransmitSetting.field.MCS != MCS_AUTO)
		{
			if (pAd->StaCfg.wdev.DesiredTransmitSetting.field.MCS == 32)
			{
				/* Fix MCS as HT Duplicated Mode */
				pEntry->MaxHTPhyMode.field.BW = 1;
				pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
				pEntry->MaxHTPhyMode.field.STBC = 0;
				pEntry->MaxHTPhyMode.field.ShortGI = 0;
				pEntry->MaxHTPhyMode.field.MCS = 32;
			}
			else if (pEntry->MaxHTPhyMode.field.MCS > pAd->StaCfg.wdev.HTPhyMode.field.MCS)
			{
				/* STA supports fixed MCS */
				pEntry->MaxHTPhyMode.field.MCS = pAd->StaCfg.wdev.HTPhyMode.field.MCS;
			}
		}

		pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
		pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;				
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (pHtCapability->HtCapInfo.ShortGIfor20)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);
		if (pHtCapability->HtCapInfo.ShortGIfor40)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		if (pHtCapability->HtCapInfo.TxSTBC)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
		if (pHtCapability->HtCapInfo.RxSTBC)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
		if (pHtCapability->ExtHtCapInfo.PlusHTC)				
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);
		if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)				
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);	
		if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);		

		NdisMoveMemory(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		DBGPRINT(RT_DEBUG_OFF, ("TDLS - Receive Peer Legacy STA \n"));
	}

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

	if ((pHtCapability->HtCapInfo.ChannelWidth) &&
		(pAd->CommonCfg.DesiredHtPhy.ChannelWidth) &&
		(pAd->StaActive.SupportedHtPhy.ChannelWidth))
	{
		pEntry->HTPhyMode.field.BW= BW_40;
	}
	else
	{
		pEntry->HTPhyMode.field.BW= BW_20;
	}


	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

	if (pAd->StaCfg.wdev.bAutoTxRateSwitch == TRUE)
	{
		PUCHAR pTable;
		UCHAR TableSize = 0;

		MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &pEntry->CurrTxRateIndex);
		pEntry->bAutoTxRateSwitch = TRUE;
	}
	else
	{
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg.wdev.HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS	= pAd->StaCfg.wdev.HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = FALSE;

		RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg.wdev.DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}

	pEntry->RateLen = SupportRateLens;
}

/*
	========================================================================
	
	Routine Description:
		Classify TDLS message type

	Arguments:
		TDLSActionType		Value of TDLS message type
		MsgType				Internal Message definition for MLME state machine
		
	Return Value:
		TRUE		Found appropriate message type
		FALSE		No appropriate message type
			
	========================================================================
*/
BOOLEAN
TDLS_MsgTypeSubst(
	IN	UCHAR	TDLSActionType,
	OUT	INT		*MsgType)
{
	switch (TDLSActionType)
	{
		case TDLS_ACTION_CODE_SETUP_REQUEST:
			*MsgType = MT2_PEER_TDLS_SETUP_REQ;
			break;
        case TDLS_ACTION_CODE_SETUP_RESPONSE:
            *MsgType = MT2_PEER_TDLS_SETUP_RSP;
			break;
        case TDLS_ACTION_CODE_SETUP_CONFIRM:
            *MsgType = MT2_PEER_TDLS_SETUP_CONF;
			break;
        case TDLS_ACTION_CODE_TEARDOWN:
            *MsgType = MT2_PEER_TDLS_TEAR_DOWN;
			break;
		case TDLS_ACTION_CODE_DISCOVERY_REQUEST:
			*MsgType = MT2_PEER_TDLS_DISCOVER_REQ;
			break;
		case TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION: /* for TDLS UAPSD */
			*MsgType = MT2_PEER_TDLS_TRAFFIC_IND;
			break;
		case TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE: /* for TDLS UAPSD */
			*MsgType = MT2_PEER_TDLS_TRAFFIC_RSP;
			break;
		case TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST:
			*MsgType = MT2_PEER_TDLS_CH_SWITCH_REQ;
			break;
		case TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE:
			*MsgType = MT2_PEER_TDLS_CH_SWITCH_RSP;
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("TDLS_MsgTypeSubst : unsupported TDLS Action Type(%d); \n", TDLSActionType));
			return FALSE;		
	}	

	return TRUE;
}

/*
    ==========================================================================
    Description:
		Check whether the received frame is TDLS frame.

	Arguments:
		pAd				-	pointer to our pAdapter context			
		pData			-	the received frame
		DataByteCount 	-	the received frame's length				
       
    Return:
         TRUE 			-	This frame is TDLS frame
         FALSE 			-	otherwise
    ==========================================================================
*/
BOOLEAN TDLS_CheckTDLSframe(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount)
{
    if(DataByteCount < (LENGTH_802_1_H + LENGTH_TDLS_H))
        return FALSE;


	/* LLC Header(6) + TDLS Ethernet Type(2) + Protocol(1) + Category(1) */
	if (!NdisEqualMemory(TDLS_LLC_SNAP_WITH_CATEGORY, pData, LENGTH_802_1_H + 2)
#ifdef WFD_SUPPORT
		&& !NdisEqualMemory(TDLS_LLC_SNAP_WITH_WFD_CATEGORY, pData, LENGTH_802_1_H + 2)
#endif /* WFD_SUPPORT */
		)
	{
		//hex_dump("TDLS_CheckTDLSframe Fail", pData, DataByteCount);
		return FALSE;
	}
	else
		hex_dump("TDLS_CheckTDLSframe", pData, DataByteCount);
	
    return TRUE;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID TDLS_CntlOidTDLSRequestProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS			pTDLS = (PRT_802_11_TDLS)Elem->Msg;
	MLME_TDLS_REQ_STRUCT	MlmeTdlsReq;
	USHORT		Reason = REASON_UNSPECIFY;
	BOOLEAN		TimerCancelled;
	INT			Idx, i;
	
	DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS_CntlOidTDLSRequestProc set %02x:%02x:%02x:%02x:%02x:%02x with Valid=%d, Status=%d\n",
		pTDLS->MacAddr[0], pTDLS->MacAddr[1], pTDLS->MacAddr[2], pTDLS->MacAddr[3], pTDLS->MacAddr[4], pTDLS->MacAddr[5],
		pTDLS->Valid, pTDLS->Status));

	if (!IS_TDLS_SUPPORT(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR,("CNTL - TDLS Capable disable !!!\n"));
		return;
	}

	if (!INFRA_ON(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR,("CNTL - STA do not connect to AP !!!\n"));
		return;
	}

#ifdef WFD_SUPPORT
	if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
		(!pAd->StaCfg.TdlsInfo.bAcceptWeakSecurity) &&
		((pAd->StaCfg.wdev.AuthMode != Ndis802_11AuthModeWPA2PSK) ||
			(pAd->StaCfg.wdev.WepStatus != Ndis802_11AESEnable)))
	{
		pAd->StaCfg.WfdCfg.TdlsSecurity = WFD_TDLS_WEAK_SECURITY;
		return;
	}

	/* Search P2P table for this MAC address before connecting TDLS */
	Idx = P2pGroupTabSearch(pAd, pTDLS->MacAddr);
	if (Idx < MAX_P2P_GROUP_SIZE)
	{
		UCHAR AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
		PWFD_SERV_DISC_QUERY_INFO pInfo = &pAd->P2pTable.Client[Idx].WfdEntryInfo.wfd_serv_disc_query_info;
		
		DBGPRINT(RT_DEBUG_TRACE, ("%s::find entry in p2p table\n", __FUNCTION__));

		/* If we can find the entry in the P2P table, check the alternate MAC address in the WFD IE */
		if (!MAC_ADDR_EQUAL(AllZero, pInfo->wfd_alternate_mac_addr_ie) &&
			!MAC_ADDR_EQUAL(pTDLS->MacAddr, pInfo->wfd_alternate_mac_addr_ie))
		{
			/* 
			* If the alternate MAC address is not all zero and different with MAC address of this entry *
			* It mean the interface for TDLS is different with the one of P2P entry *
			* Copy the alternate MAC address to the entry for TDLS connection *
			*/
			COPY_MAC_ADDR(pTDLS->MacAddr, pInfo->wfd_alternate_mac_addr_ie);
			DBGPRINT(RT_DEBUG_TRACE, ("WFD alternate mac address is:%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pTDLS->MacAddr)));
		}
	}
#endif /* WFD_SUPPORT */

	Idx = TDLS_SearchLinkId(pAd, pTDLS->MacAddr);
	
	if (Idx == -1) /* not found and the entry is not full */
	{
		if (pTDLS->Valid) 
		{
			/* 1. Enable case, start TDLS setup procedure */
			for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
			{
				if (!pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid)
				{
					NdisMoveMemory(&pAd->StaCfg.TdlsInfo.TDLSEntry[i], pTDLS, sizeof(RT_802_11_TDLS_UI));
					TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[i], Reason, FALSE);
					MlmeEnqueue(pAd,
								TDLS_STATE_MACHINE,
								MT2_MLME_TDLS_SETUP_REQ,
								sizeof(MLME_TDLS_REQ_STRUCT),
								&MlmeTdlsReq,
								0);
					DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS setup case\n"));
					break;
				}
				DBGPRINT(RT_DEBUG_ERROR,("CNTL - TDLS  do not find vaild entry !!!!\n"));
			}	
		}
		else
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = -1 (Valid=%d)\n", pTDLS->Valid));

	}
	else  if (Idx == MAX_NUM_OF_TDLS_ENTRY)		/* not found and full */
	{
		if (pTDLS->Valid) 
		{
			/* 2. table full, cancel the non-finished entry and restart a new one */
			for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
			{
				if ((pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid) &&(pAd->StaCfg.TdlsInfo.TDLSEntry[i].Status < TDLS_MODE_CONNECTED))
				{
					/* update mac case */
					RTMPCancelTimer(&pAd->StaCfg.TdlsInfo.TDLSEntry[i].Timer, &TimerCancelled);
					NdisMoveMemory(&pAd->StaCfg.TdlsInfo.TDLSEntry[i], pTDLS, sizeof(RT_802_11_TDLS_UI));
					TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[i], Reason, FALSE);
					MlmeEnqueue(pAd,
								TDLS_STATE_MACHINE,
								MT2_MLME_TDLS_SETUP_REQ,
								sizeof(MLME_TDLS_REQ_STRUCT),
								&MlmeTdlsReq,
								0);
					DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS restart case\n"));
					break;
				}
			}	
		}
		else
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in Idx = MAX_NUM_OF_TDLS_ENTRY (Valid=%d)\n", pTDLS->Valid));
	}
	else	/* found one in entry */
	{
#ifdef WFD_SUPPORT
		if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
			pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].WfdEntryInfo.wfd_PC == WFD_PC_P2P)
		{
			pAd->StaCfg.WfdCfg.PeerPC = WFD_PC_P2P;

			DBGPRINT(RT_DEBUG_TRACE,("CNTL - Peer TDLS PC is P2P\n"));
		}
		else
#endif /* WFD_SUPPORT */
		if ((!pTDLS->Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Status >= TDLS_MODE_CONNECTED))
		{
			/* 3. Disable TDLS link case, just tear down TDLS link */
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Valid	= FALSE;
			pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
			DBGPRINT(RT_DEBUG_TRACE,("CNTL - start tear down procedure\n"));
		}
		else if ((pTDLS->Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Status >= TDLS_MODE_CONNECTED)) 
		{
			/* 4. re-setup case, tear down old link and re-start TDLS setup procedure */
			Reason = TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON;
			pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Valid	= FALSE;
			pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Status	= TDLS_MODE_NONE;
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_TEAR_DOWN,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
		
			RTMPCancelTimer(&pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Timer, &TimerCancelled);
			NdisMoveMemory(&pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], pTDLS, sizeof(RT_802_11_TDLS_UI));
			TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], Reason, FALSE);
			MlmeEnqueue(pAd,
						TDLS_STATE_MACHINE,
						MT2_MLME_TDLS_SETUP_REQ,
						sizeof(MLME_TDLS_REQ_STRUCT),
						&MlmeTdlsReq,
						0);
			DBGPRINT(RT_DEBUG_TRACE,("CNTL - TDLS retry setup procedure\n"));
		}
		else
		{
#ifdef WFD_SUPPORT
			if (pAd->StaCfg.WfdCfg.bWfdEnable)
			{
				/* WFD will send TDLS tunneled request and receive reponse frame,
				 * this will add TDLS entry to TDLS table */
				RTMPCancelTimer(&pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Timer, &TimerCancelled);
				NdisMoveMemory(&pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], pTDLS, sizeof(RT_802_11_TDLS_UI));
				TDLS_MlmeParmFill(pAd, &MlmeTdlsReq, &pAd->StaCfg.TdlsInfo.TDLSEntry[Idx], Reason, FALSE);
				MlmeEnqueue(pAd,
							TDLS_STATE_MACHINE,
							MT2_MLME_TDLS_SETUP_REQ,
							sizeof(MLME_TDLS_REQ_STRUCT),
							&MlmeTdlsReq,
							0);
			}
#endif /* WFD_SUPPORT */
			DBGPRINT(RT_DEBUG_WARN,("CNTL - TDLS not changed in entry - %d - Valid=%d, Status=%d\n",
				Idx, pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Valid, pAd->StaCfg.TdlsInfo.TDLSEntry[Idx].Status));
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/

/* Not found and full : return MAX_NUM_OF_TDLS_ENTRY
 *  not found and the entry is not full : return -1
 */
INT TDLS_SearchLinkId(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pAddr)
{
	INT		i = 0;
	UCHAR	empty = 0;
	PRT_802_11_TDLS	pTDLS = NULL;
	
	for (i = 0; i < MAX_NUM_OF_TDLS_ENTRY; i++)
	{
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[i];
		if (!pTDLS->Valid)
			empty |= 1;
		
		if (pTDLS->Valid && MAC_ADDR_EQUAL(pAddr, pTDLS->MacAddr))
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId - Find Link ID with Peer Address %02x:%02x:%02x:%02x:%02x:%02x(%d) \n", 
            	pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5], i));
			break;
		}
	}

	if (i == MAX_NUM_OF_TDLS_ENTRY)
	{
		if (empty == 0)
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId -  not found and full\n"));
			return MAX_NUM_OF_TDLS_ENTRY;
		}
		else
		{
			DBGPRINT(RT_DEBUG_INFO, ("TDLS_SearchLinkId -  not found\n"));
			return -1;
		}	
	}

	return i;
}

/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID TDLS_MlmeParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
	IN PRT_802_11_TDLS pTdls,
	IN USHORT Reason,
	IN BOOLEAN IsViaAP) 
{
	pTdlsReq->pTDLS = pTdls;
	pTdlsReq->Reason = Reason;
	pTdlsReq->IsViaAP = IsViaAP;
}

/*
	========================================================================
	
	Routine Description:
		It is used to derive the TDLS Peer Key and its identifier TPK-Name.
		(IEEE 802.11z/D4.0, 8.5.9.1)
		
	Arguments:

	Return Value:

	Note:
		TPK = KDF-256(0, "TDLS PMK", min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || N_KEY)
		TPK-Name = Truncate-128(SHA-256(min(MAC_I, MAC_R) || max(MAC_I, MAC_R) || min(SNonce, ANonce) || max(SNonce, ANonce) || BSSID || 256))
	========================================================================
*/
VOID TDLS_FTDeriveTPK(
	IN	PUCHAR 	mac_i,
	IN	PUCHAR 	mac_r,
	IN	PUCHAR 	a_nonce,
	IN	PUCHAR 	s_nonce,
	IN	PUCHAR 	bssid,
	IN	UINT	key_len,
 	OUT	PUCHAR	tpk,
	OUT	PUCHAR	tpk_name)
{	
	UCHAR	temp_result[64];
	UCHAR   context[128];
	UINT    c_len=0; 
	UCHAR	temp_var[32];
	/*UINT	key_len = LEN_PMK; */
	//USHORT	len_in_bits = (key_len << 3) + 128;
	UCHAR	TPK_KEY_INPUT[LEN_PMK];

	/* ================================ */
	/* 		TPK-Key-Input derivation 	*/
	/* ================================ */
	/*
		Refer to IEEE 802.11z-8.5.9.1
		TPK-Key-Input = 
			SHA-256(min (SNonce, ANonce) || max (SNonce, ANonce)) 
	 */

	/* Zero the context firstly */
	NdisZeroMemory(context, 128);
	c_len = 0;

	/* concatenate min(SNonce, ANonce) with 32-octets */
	if (RTMPCompareMemory(s_nonce, a_nonce, 32) == 1)
		NdisMoveMemory(&context[c_len], a_nonce, 32);
	else
		NdisMoveMemory(&context[c_len], s_nonce, 32);
	c_len += 32;
	
	/* concatenate max(SNonce, ANonce) with 32-octets */
	if (RTMPCompareMemory(s_nonce, a_nonce, 32) == 1)
		NdisMoveMemory(&context[c_len], s_nonce, 32);
	else
		NdisMoveMemory(&context[c_len], a_nonce, 32);
	c_len += 32;

	/* Zero key material */
	NdisZeroMemory(TPK_KEY_INPUT, LEN_PMK);	
	
	RT_SHA256(context, c_len, TPK_KEY_INPUT);

	/* =============================== */
	/* 		TPK derivation */
	/* =============================== */

	/* construct the concatenated context for TPK */
	/* min(MAC_I, MAC_R)	(6 bytes) */
	/* max(MAC_I, MAC_R)	(6 bytes) */
	/* BSSID				(6 bytes) */
	/* Number of Key in bits(2 bytes) */

	/* Initial the related parameter */
	NdisZeroMemory(temp_result, 64);
	NdisZeroMemory(context, 128);
	NdisZeroMemory(temp_var, 32);
	c_len = 0;

	/* concatenate min(MAC_I, MAC_R) with 6-octets */
	if (RTMPCompareMemory(mac_i, mac_r, 6) == 1)
		NdisMoveMemory(temp_var, mac_r, 6);
	else
		NdisMoveMemory(temp_var, mac_i, 6);
    NdisMoveMemory(&context[c_len], temp_var, 6);
	c_len += 6;

	/* concatenate max(MAC_I, MAC_R) with 6-octets */
	if (RTMPCompareMemory(mac_i, mac_r, 6) == 1)
		NdisMoveMemory(temp_var, mac_i, 6);
	else
		NdisMoveMemory(temp_var, mac_r, 6);
    NdisMoveMemory(&context[c_len], temp_var, 6);
	c_len += 6;

	/* concatenate the BSSID with 6-octets */
	NdisMoveMemory(&context[c_len], bssid, MAC_ADDR_LEN);
	c_len += MAC_ADDR_LEN;

	/* concatenate the N_KEY with 2-octets */
	//NdisMoveMemory(&context[c_len], &len_in_bits, 2);
	//c_len += 2;
	
	/*hex_dump("TDLS_FTDeriveTPK", context, 128); */

	/* Calculate a key material through FT-KDF */
	KDF(TPK_KEY_INPUT, 
			LEN_PMK, 
			(PUCHAR)"TDLS PMK", 
			8, 
			context, 
			c_len, 
			temp_result, 
			(key_len + 16));
	NdisMoveMemory(tpk, temp_result, (key_len + 16));

	hex_dump("TPK ", tpk , (key_len + 16));


	/* =============================== */
	/* 		TPK-Name derivation */
	/* =============================== */

	/* construct the concatenated context for TPK-Name */
	/* min(MAC_I, MAC_R)	(6 bytes) */
	/* max(MAC_I, MAC_R)	(6 bytes) */
	/* min(SNonce, ANonce)	(32 bytes) */
	/* max(SNonce, ANonce)	(32 bytes) */
	/* BSSID				(6 bytes) */
	/* Number of Key in bits(2 bytes) */

	/* The context is the same as the contxex of TPK. */
	
	/* Initial the related parameter */
	NdisZeroMemory(temp_result, 64);

	/* derive TPK-Name */
	RT_SHA256(context, c_len, temp_result);
	NdisMoveMemory(tpk_name, temp_result, LEN_PMK_NAME);

	hex_dump("TPK-Name ", tpk_name, LEN_PMK_NAME);

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN MlmeTdlsReqSanity(
	IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUINT16 pReason,
    OUT BOOLEAN *pIsViaAP)
{
	MLME_TDLS_REQ_STRUCT *pInfo;

    pInfo = (MLME_TDLS_REQ_STRUCT *)Msg;
    
	*pTDLS = pInfo->pTDLS;
	*pReason = pInfo->Reason;
	*pIsViaAP = pInfo->IsViaAP;/* default = FALSE, not pass through AP */

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupReqSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
#ifdef WFD_SUPPORT
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement,
#endif /* WFD_SUPPORT */
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;
	UCHAR LinkIdLen = 0;
	UCHAR LinkIdBSSID[MAC_ADDR_LEN];
	UCHAR LinkIdInitiatorAddr[MAC_ADDR_LEN];
	UCHAR LinkIdResponderAddr[MAC_ADDR_LEN];

	// Init output parameters
	*pSupRateLen = 0;
	*pExtRateLen = 0;
	*pCapabilityInfo = 0;
	*pHtCapLen = 0;
	*pTdlsExtCapLen = 0;
	*pbWmmCapable = FALSE;
	*pQosCapability = 0; // default: no IE_QOS_CAPABILITY found
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	
	/* Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes),
	    TDLS Action header(3 bytes) and Payload (variable)
	  */
	if (MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H)) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return FALSE;	
	}

	// Offset to Dialog Token
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return FALSE;
	}	
	*pToken = *Ptr;

	// Offset to Capability
	Ptr += 1;
	RemainLen -= 1;
	//Length += 1;

	// Get capability info from payload and advance the pointer
	if (RemainLen < 2) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (capability) \n", __FUNCTION__));
		return FALSE;
	}	
	NdisMoveMemory((PUCHAR)pCapabilityInfo, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;

	// Add for 2 necessary EID field check
	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{				
			case IE_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(SupRate, pEid->Octet, pEid->Len);
					*pSupRateLen = pEid->Len;
				}
				else
					return FALSE;

				break;

			case IE_COUNTRY:
				break;

			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;

			case IE_SUPP_CHANNELS:
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_EXT_CAPABILITY:
				if (pEid->Len >= sizeof(EXT_CAP_INFO_ELEMENT))
				{
					NdisMoveMemory(pTdlsExtCap, &pEid->Octet[0], sizeof(EXT_CAP_INFO_ELEMENT));
					*pTdlsExtCapLen = pEid->Len;
				}
				break;

			case IE_QOS_CAPABILITY:
				if (pEid->Len == 1)
				{
					*pbWmmCapable = TRUE;
					*pQosCapability = *(pEid->Octet);
				}
				break;

			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;

			case IE_SUPP_REG_CLASS:
				break;

			case IE_HT_CAP:
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
				{
					if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
					{
						NdisMoveMemory(pHtCap, pEid->Octet, sizeof(HT_CAPABILITY_IE));
						*pHtCapLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
					}
				}
				break;
				
			case IE_2040_BSS_COEXIST:
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len >= TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(LinkIdBSSID, &pEid->Octet[0], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdInitiatorAddr, &pEid->Octet[6], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdResponderAddr, &pEid->Octet[12], MAC_ADDR_LEN);
					LinkIdLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
					NdisMoveMemory(pSA, &pEid->Octet[6], MAC_ADDR_LEN);
				}
				break;
		
			case IE_VENDOR_SPECIFIC:
				// handle WME PARAMTER ELEMENT
				if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
				{
					*pQosCapability = pEid->Octet[6];
					*pbWmmCapable = TRUE;
				}
#ifdef WFD_SUPPORT
				else if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
					NdisEqualMemory(pEid->Octet, WIFIDISPLAY_OUI, 4) && (pEid->Len >= 4))
				{
					hex_dump("WFD_IE", &pEid->Eid, pEid->Len +2);
					RTMPMoveMemory(pWfdSubelement, &pEid->Eid, pEid->Len +2);
					*pWfdSubelementLen = pEid->Len + 2;
				}
#endif /* WFD_SUPPORT */
				break;

			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupReqSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}

	if (LinkIdLen == 0)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		if (!MAC_ADDR_EQUAL(LinkIdBSSID, pAd->CommonCfg.Bssid))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
			return FALSE;
		}	
		else if (!MAC_ADDR_EQUAL(LinkIdResponderAddr, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
			return FALSE;
		}
	}

	/* Process in succeed */
	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
#ifdef WFD_SUPPORT
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement,
#endif /* WFD_SUPPORT */
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;
	UCHAR LinkIdLen = 0;
	UCHAR LinkIdBSSID[MAC_ADDR_LEN];
	UCHAR LinkIdInitiatorAddr[MAC_ADDR_LEN];
	UCHAR LinkIdResponderAddr[MAC_ADDR_LEN];

	// Init output parameters
	*pSupRateLen = 0;
	*pExtRateLen = 0;
	*pCapabilityInfo = 0;
	*pHtCapLen = 0;
	*pTdlsExtCapLen = 0;
	*pbWmmCapable = FALSE;
	*pQosCapability= 0; // default: no IE_QOS_CAPABILITY found
	*pStatusCode = MLME_SUCCESS;
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	
	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return FALSE;	
	}

	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (status code) \n", __FUNCTION__));
		return FALSE;
	}	
	NdisMoveMemory(pStatusCode, Ptr, 2);

	if (*pStatusCode != MLME_SUCCESS)
		return TRUE;	// in the end of Setup Response frame

	// Offset to Dialog Token
	Ptr	+= 2;
	RemainLen -= 2;

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return FALSE;
	}	
	*pToken = *Ptr;

	// Offset to Capability
	Ptr += 1;
	RemainLen -= 1;

	// Get capability info from payload and advance the pointer
	if (RemainLen < 2) 
		return FALSE;
	NdisMoveMemory(pCapabilityInfo, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;


	// Add for 2 necessary EID field check
	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(SupRate, &pEid->Octet[0], pEid->Len);
					*pSupRateLen = pEid->Len;
				}
				else
					return FALSE;

				break;

			case IE_COUNTRY:
				break;

			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, &pEid->Octet[0], pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;

			case IE_SUPP_CHANNELS:
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_EXT_CAPABILITY:
				if (pEid->Len >= sizeof(EXT_CAP_INFO_ELEMENT))
				{
					NdisMoveMemory(pTdlsExtCap, &pEid->Octet[0], sizeof(EXT_CAP_INFO_ELEMENT));
					*pTdlsExtCapLen = pEid->Len;
				}
				break;

			case IE_QOS_CAPABILITY:
				if (pEid->Len ==  1)
				{
					*pQosCapability = *(pEid->Octet);
					*pbWmmCapable = TRUE;
				}
				break;

			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;

			case IE_SUPP_REG_CLASS:
				break;

			case IE_HT_CAP:
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
				{
					if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
					{
						NdisMoveMemory(pHtCap, &pEid->Octet[0], sizeof(HT_CAPABILITY_IE));
						*pHtCapLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
					}
				}
				
				break;
				
			case IE_2040_BSS_COEXIST:
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len >= TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(LinkIdBSSID, &pEid->Octet[0], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdInitiatorAddr, &pEid->Octet[6], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdResponderAddr, &pEid->Octet[12], MAC_ADDR_LEN);
					LinkIdLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
					NdisMoveMemory(pSA, &pEid->Octet[12], MAC_ADDR_LEN);
				}
				break;

			case IE_VENDOR_SPECIFIC:
				// handle WME PARAMTER ELEMENT
				if (NdisEqualMemory(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
				{
					*pQosCapability = pEid->Octet[6];
					*pbWmmCapable = TRUE;
				}
#ifdef WFD_SUPPORT
				else if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
					NdisEqualMemory(pEid->Octet, WIFIDISPLAY_OUI, 4) && (pEid->Len >= 4))
				{
					ULONG	WfdSubelementLen = 0;
					PUCHAR	WfdSubelement = NULL;

					hex_dump("WFD_IE", &pEid->Eid, pEid->Len +2);
					RTMPMoveMemory(pWfdSubelement, &pEid->Eid, pEid->Len +2);
					*pWfdSubelementLen = pEid->Len + 2;
				}
#endif /* WFD_SUPPORT */
				break;

			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupRspSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}

	if (LinkIdLen == 0)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		if (!MAC_ADDR_EQUAL(LinkIdBSSID, pAd->CommonCfg.Bssid))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
			return FALSE;
		}	
		else if (!MAC_ADDR_EQUAL(LinkIdInitiatorAddr, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
			return FALSE;
		}	
	}

	/* Process in succeed */
	*pStatusCode = MLME_SUCCESS;
	return TRUE;

}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsSetupConfSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT EDCA_PARM	*pEdcaParm,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;	
	UCHAR LinkIdLen = 0;
	UCHAR LinkIdBSSID[MAC_ADDR_LEN];
	UCHAR LinkIdInitiatorAddr[MAC_ADDR_LEN];
	UCHAR LinkIdResponderAddr[MAC_ADDR_LEN];

	// Init output parameters
	*pCapabilityInfo = 0;
	*pStatusCode = MLME_REQUEST_DECLINED;
	 //pEdcaParm = 0;      // default: no IE_EDCA_PARAMETER found
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	
	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return FALSE;	
	}
	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (status code) \n", __FUNCTION__));
		return FALSE;
	}	
	NdisMoveMemory(pStatusCode, Ptr, 2);

	if (*pStatusCode != MLME_SUCCESS)
		return TRUE;	// in the end of Setup Response frame

	// Offset to Dialog Token
	Ptr	+= 2;
	RemainLen -= 2;

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return FALSE;
	}

	*pToken = *Ptr;

	// Offset to other elements
	Ptr += 1;
	RemainLen -= 1;

	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_EDCA_PARAMETER:
				break;

			case IE_VENDOR_SPECIFIC:
				// handle WME PARAMTER ELEMENT
				if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
				{
					PUCHAR ptr;
					int i;

					// parsing EDCA parameters
					pEdcaParm->bValid		   = TRUE;
					pEdcaParm->bQAck		   = FALSE; // pEid->Octet[0] & 0x10;
					pEdcaParm->bQueueRequest   = FALSE; // pEid->Octet[0] & 0x20;
					pEdcaParm->bTxopRequest    = FALSE; // pEid->Octet[0] & 0x40;
					//pEdcaParm->bMoreDataAck	 = FALSE; // pEid->Octet[0] & 0x80;
					pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0xff;
					//pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
					ptr = &pEid->Octet[8];
					for (i=0; i<4; i++)
					{
						UCHAR aci = (*ptr & 0x60) >> 5; // b5~6 is AC INDEX
						pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);	 // b5 is ACM
						pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;				 // b0~3 is AIFSN
						pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;			 // b0~4 is Cwmin
						pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;				 // b5~8 is Cwmax
						pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); // in unit of 32-us
						ptr += 4; // point to next AC
					}
				}
				break;

			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;

			case IE_ADD_HT:
			case IE_ADD_HT2:
				if (pEid->Len >= sizeof(ADD_HT_INFO_IE))				
				{
					// This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only
					// copy first sizeof(ADD_HT_INFO_IE)
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity - wrong IE_ADD_HT. \n"));
				}
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len >= TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(LinkIdBSSID, &pEid->Octet[0], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdInitiatorAddr, &pEid->Octet[6], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdResponderAddr, &pEid->Octet[12], MAC_ADDR_LEN);
					LinkIdLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
					NdisMoveMemory(pSA, &pEid->Octet[6], MAC_ADDR_LEN);
				}
				break;

			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsSetupConfSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}


	if (LinkIdLen == 0)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		if (!MAC_ADDR_EQUAL(LinkIdBSSID, pAd->CommonCfg.Bssid))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
			return FALSE;
		}	
		else if (!MAC_ADDR_EQUAL(LinkIdResponderAddr, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
			return FALSE;
		}
	}

	/* Process in succeed */
	*pStatusCode = MLME_SUCCESS;

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsTearDownSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen,
	OUT UCHAR	*pSA,
	OUT	BOOLEAN *pIsInitator,
	OUT USHORT *pReasonCode,
	OUT UCHAR	*pFTLen,    
	OUT UCHAR	FTIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;
	UCHAR LinkIdLen = 0;
	UCHAR LinkIdBSSID[MAC_ADDR_LEN];
	UCHAR LinkIdInitiatorAddr[MAC_ADDR_LEN];
	UCHAR LinkIdResponderAddr[MAC_ADDR_LEN];

	// Init output parameters
	*pReasonCode = 0;
	*pFTLen = 0 ;

	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (cation header) \n", __FUNCTION__));
		return FALSE;	
	}
	// Offset to Reason Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	
	// Get the value of Reason Code from payload and advance the pointer
	if (RemainLen < 2) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (reason code) \n", __FUNCTION__));
		return FALSE;	
	}
	NdisMoveMemory(pReasonCode, Ptr, 2);


	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;

	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len == TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(LinkIdBSSID, &pEid->Octet[0], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdInitiatorAddr, &pEid->Octet[6], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdResponderAddr, &pEid->Octet[12], MAC_ADDR_LEN);
					LinkIdLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
					NdisMoveMemory(pSA, &pEid->Octet[12], MAC_ADDR_LEN);
				}
				break;

			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsTearDownSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 
	}

	if (LinkIdLen == 0)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		if (!MAC_ADDR_EQUAL(LinkIdBSSID, pAd->CommonCfg.Bssid))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
			return FALSE;
		}

		// Check if my MAC address and then find out SA
		if (!MAC_ADDR_EQUAL(pAd->CurrentAddress, LinkIdInitiatorAddr))
		{
			if (!MAC_ADDR_EQUAL(pAd->CurrentAddress, LinkIdResponderAddr))
			{
				DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my Address\n", __FUNCTION__));
				return FALSE;
			}
			else
			{
				*pIsInitator = TRUE;	// peer are Initator.
				NdisMoveMemory(pSA, LinkIdInitiatorAddr, MAC_ADDR_LEN);
			}
		}
		else
		{
			*pIsInitator = FALSE;	// peer are not Initator.
			NdisMoveMemory(pSA, LinkIdResponderAddr, MAC_ADDR_LEN);
		}
	}

	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsDiscovReqSanity(
	IN	PRTMP_ADAPTER	pAd, 
	IN	VOID	*Msg, 
	IN	ULONG	MsgLen,
	OUT UCHAR	*pSA,
	OUT UCHAR	*pToken)
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;

	// Init output parameters
	*pToken = 0;
	
	/*	Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes)
		TDLS Action header(payload type + category + action)(3 bytes) and Payload (variable)
	*/
	if (MsgLen < (LENGTH_802_11 + LENGTH_802_1_H + 3)) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return FALSE;	
	}

	/* Offset to Dialog Token */
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);

	/* Get the value of token from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return FALSE;
	}

	*pToken = *Ptr;
	
	/* Offset to Link Identifier */
	Ptr += 1;
	RemainLen -= 1;

	/* Get BSSID, SA and DA from payload and advance the pointer */
	if ((RemainLen < 20) || (Ptr[0] != IE_TDLS_LINK_IDENTIFIER) || (Ptr[1] != 18))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
		return FALSE;
	}	
	else if (!MAC_ADDR_EQUAL(Ptr+14, pAd->CurrentAddress))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
		return FALSE;
	}	

	NdisMoveMemory(pSA, Ptr+8, MAC_ADDR_LEN);

	// Process in succeed
	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsDiscovRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR TIIe[])
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;
	PEID_STRUCT		pEid;
	ULONG			Length = 0;
	UCHAR LinkIdLen = 0;
	UCHAR LinkIdBSSID[MAC_ADDR_LEN];
	UCHAR LinkIdInitiatorAddr[MAC_ADDR_LEN];
	UCHAR LinkIdResponderAddr[MAC_ADDR_LEN];

	/* Init output parameters */
	*pSupRateLen = 0;
	*pExtRateLen = 0;
	*pCapabilityInfo = 0;
	*pHtCapLen = 0;
	*pTdlsExtCapLen = 0;
	*pRsnLen = 0;
	*pFTLen = 0;
	*pTILen = 0;
	LinkIdLen = 0;

	/* Message contains 802.11 header (24 bytes), 
	    public action(2 bytes), TDLS Action header(2 bytes) and Payload (variable)
	 */
	if (RemainLen < (LENGTH_802_11 + 2 + 2))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (action header) \n", __FUNCTION__));
		return FALSE;	
	}

	/* Offset to Dialog Token */
	Ptr	+= (LENGTH_802_11 + 2 + 2);
	RemainLen -= (LENGTH_802_11 + 2 + 2);

	// Get the value of token from payload and advance the pointer
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (dialog token) \n", __FUNCTION__));
		return FALSE;
	}	
	*pToken = *Ptr;

	/* Offset to Capability */
	Ptr += 1;
	RemainLen -= 1;

	/* Get capability info from payload and advance the pointer */
	if (RemainLen < 2) 
		return FALSE;

	NdisMoveMemory(pCapabilityInfo, Ptr, 2);

	/* Offset to other elements */
	Ptr += 2;
	RemainLen -= 2;

	/* Add for 2 necessary EID field check */
	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(SupRate, &pEid->Octet[0], pEid->Len);
					*pSupRateLen = pEid->Len;
				}
				else
					return FALSE;

				break;

			case IE_COUNTRY:
				break;

			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, &pEid->Octet[0], pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;

			case IE_SUPP_CHANNELS:
				break;

			case IE_RSN:
				if ((pEid->Len + 2) < 64)
				{
					NdisMoveMemory(RsnIe, &pEid->Eid, pEid->Len + 2);
					*pRsnLen = pEid->Len + 2;
				}
				break;

			case IE_EXT_CAPABILITY:
				if (pEid->Len >= sizeof(EXT_CAP_INFO_ELEMENT))
				{
					NdisMoveMemory(pTdlsExtCap, &pEid->Octet[0], sizeof(EXT_CAP_INFO_ELEMENT));
					*pTdlsExtCapLen = pEid->Len;
				}
				break;



			case IE_FT_FTIE:
				if ((pEid->Len) == sizeof(FT_FTIE))
				{
					NdisMoveMemory(FTIe, &pEid->Eid, pEid->Len + 2);
					*pFTLen = pEid->Len + 2;
				}	
				break;
				
			case IE_FT_TIMEOUT_INTERVAL:
				if ((pEid->Len + 2) == 7)
				{
					NdisMoveMemory(TIIe, &pEid->Eid, pEid->Len + 2);
					*pTILen = pEid->Len + 2;
				}
				break;

			case IE_SUPP_REG_CLASS:
				break;

			case IE_HT_CAP:
				if (pAd->CommonCfg.PhyMode >= PHY_11ABGN_MIXED)
				{
					if (pEid->Len >= SIZE_HT_CAP_IE)  //Note: allow extension.!!
					{
						NdisMoveMemory(pHtCap, &pEid->Octet[0], sizeof(HT_CAPABILITY_IE));
						*pHtCapLen = SIZE_HT_CAP_IE;	// Nnow we only support 26 bytes.
					}
				}
				
				break;
				
			case IE_2040_BSS_COEXIST:
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len >= TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					NdisMoveMemory(LinkIdBSSID, &pEid->Octet[0], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdInitiatorAddr, &pEid->Octet[6], MAC_ADDR_LEN);
					NdisMoveMemory(LinkIdResponderAddr, &pEid->Octet[12], MAC_ADDR_LEN);
					LinkIdLen = TDLS_ELM_LEN_LINK_IDENTIFIER;
					NdisMoveMemory(pSA, &pEid->Octet[12], MAC_ADDR_LEN);
				}
				break;

			default:
				/* Unknown IE, we have to pass it as variable IEs */
				DBGPRINT(RT_DEBUG_WARN, ("PeerTdlsDiscovRspSanity - unrecognized EID = %d\n", pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 	   
	}

	if (LinkIdLen == 0)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet - (link identifier) \n", __FUNCTION__));
		return FALSE;
	}
	else
	{
		if (!MAC_ADDR_EQUAL(LinkIdBSSID, pAd->CommonCfg.Bssid))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my BSSID\n", __FUNCTION__));
			return FALSE;
		}	
		else if (!MAC_ADDR_EQUAL(LinkIdInitiatorAddr, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - It's not my MAC address\n", __FUNCTION__));
			return FALSE;
		}	
	}

	/* Process in succeed */
	return TRUE;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsChannelSwitchReqSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen,
	OUT UCHAR *pPeerAddr,
	OUT	BOOLEAN *pIsInitator,
	OUT UCHAR *pTargetChannel,    
	OUT UCHAR *pRegulatoryClass,
	OUT UCHAR *pNewExtChannelOffset,
	OUT USHORT *pChSwitchTime,
	OUT USHORT *pChSwitchTimeOut)
{
	ULONG RemainLen = MsgLen;
	CHAR *Ptr =(CHAR *)Msg;
	PEID_STRUCT pEid;
	ULONG Length = 0;
	PHEADER_802_11 pHeader;
	BOOLEAN rv = TRUE;

	/* init value */
	*pNewExtChannelOffset = 0;

	/* Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable) */
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (ation header) \n", __FUNCTION__));
		return FALSE;	
	}

	pHeader = (PHEADER_802_11)Ptr;
	COPY_MAC_ADDR(pPeerAddr,  &pHeader->Addr2);	

	/* Offset to Target Channel */
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);

	/* Get the value of target channel from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (target channel) \n", __FUNCTION__));
		return FALSE;
	}	

	*pTargetChannel = *Ptr;

	/* Offset to Regulatory Class */
	Ptr += 1;
	RemainLen -= 1;

	/* Get the value of regulatory class from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (regulatory class) \n", __FUNCTION__));
		return FALSE;
	}
	
	*pRegulatoryClass = *Ptr;
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s() - Regulatory class = %d \n", __FUNCTION__, *pRegulatoryClass));

	/* Offset to other elements */
	Ptr += 1;
	RemainLen -= 1;

	pEid = (PEID_STRUCT) Ptr;

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - wrong IE_SECONDARY_CH_OFFSET. \n", __FUNCTION__));
				}
				break;

			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len != TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - wrong IE_TDLS_LINK_IDENTIFIER. \n", __FUNCTION__));
				}
				break;

			case IE_TDLS_CHANNEL_SWITCH_TIMING:
				if (pEid->Len == 4)
				{
					NdisMoveMemory(pChSwitchTime, &pEid->Octet[0], 2);
					NdisMoveMemory(pChSwitchTimeOut, &pEid->Octet[2], 2);
				}
				else
				{
					rv = FALSE;
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - wrong IE_TDLS_CHANNEL_SWITCH_TIMING. \n", __FUNCTION__));
				}
				break;

			default:
				/* Unknown IE, we have to pass it as variable IEs */
				DBGPRINT(RT_DEBUG_WARN, ("%s() - unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 
	}

    return rv;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN PeerTdlsChannelSwitchRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pPeerAddr,
	OUT USHORT	*pStatusCode,
	OUT USHORT	*pChSwitchTime,
	OUT USHORT	*pChSwitchTimeOut)
{
	ULONG RemainLen = MsgLen;
	CHAR *Ptr =(CHAR *)Msg;
	PEID_STRUCT pEid;
	ULONG Length = 0;
	PHEADER_802_11 pHeader;
	BOOLEAN rv = TRUE;

	// Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes), TDLS Action header(3 bytes) and Payload (variable)
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (ation header) \n", __FUNCTION__));
		return FALSE;	
	}

	pHeader = (PHEADER_802_11)Ptr;
	COPY_MAC_ADDR(pPeerAddr,  &pHeader->Addr2);	

	// Offset to Status Code
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H);
	
	// Get the value of Status Code from payload and advance the pointer
	if (RemainLen < 2)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - Invaild packet length - (status code) \n", __FUNCTION__));
		return FALSE;
	}	
	NdisMoveMemory(pStatusCode, Ptr, 2);

	// Offset to other elements
	Ptr += 2;
	RemainLen -= 2;

	pEid = (PEID_STRUCT) Ptr;

	// get variable fields from payload and advance the pointer
	while ((Length + 2 + pEid->Len) <= RemainLen)	  
	{	
		switch(pEid->Eid)
		{
			case IE_TDLS_LINK_IDENTIFIER:
				if (pEid->Len != TDLS_ELM_LEN_LINK_IDENTIFIER)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - wrong IE_TDLS_LINK_IDENTIFIER. \n", __FUNCTION__));
					rv = FALSE;
				}
				break;

			case IE_TDLS_CHANNEL_SWITCH_TIMING:
				if (pEid->Len == 4)
				{
					NdisMoveMemory(pChSwitchTime, &pEid->Octet[0], 2);
					NdisMoveMemory(pChSwitchTimeOut, &pEid->Octet[2], 2);

				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s() - wrong IE_TDLS_CHANNEL_SWITCH_TIMING. \n", __FUNCTION__));
					rv = FALSE;
				}
				break;
			default:
				// Unknown IE, we have to pass it as variable IEs
				DBGPRINT(RT_DEBUG_WARN, ("%s() - unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
				break;
		}

		Length = Length + 2 + pEid->Len; 	
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len); 
	}

    return rv;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
ULONG PeerTdlsBasicSanity(
	IN	PRTMP_ADAPTER				pAd, 
	IN	VOID						*Msg, 
	IN	ULONG						MsgLen,
	IN	BOOLEAN						bInitiator,
	OUT UCHAR						*pToken,
	OUT UCHAR						*pSA)
{
	ULONG			RemainLen = MsgLen;
	CHAR			*Ptr =(CHAR *)Msg;

	/*
		Message contains 802.11 header (24 bytes), LLC_SNAP (8 bytes),
		TDLS Action header(3 bytes) and Payload (variable)
	*/
	if (RemainLen < (LENGTH_802_11 + LENGTH_802_1_H + 3)) 
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
					"Invaild packet length - (action header) \n",
					__FUNCTION__));
		return 0;	
	}

	/* Offset to Dialog Token */
	Ptr	+= (LENGTH_802_11 + LENGTH_802_1_H + 3);
	RemainLen -= (LENGTH_802_11 + LENGTH_802_1_H + 3);

	/* Get the value of token from payload and advance the pointer */
	if (RemainLen < 1)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
					"Invaild packet length - (dialog token) \n",
					__FUNCTION__));
		return 0;
	}	
	*pToken = *Ptr;
	
	/* Offset to Link Identifier */
	Ptr += 1;
	RemainLen -= 1;

	/* Get BSSID, SA and DA from payload and advance the pointer */
	if (RemainLen < 20 || Ptr[0] != IE_TDLS_LINK_IDENTIFIER || Ptr[1] != 18)
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
					"Invaild packet length - (link identifier) \n",
					__FUNCTION__));
		return 0;
	}

	if (!MAC_ADDR_EQUAL(Ptr+2, pAd->CommonCfg.Bssid))
	{
		DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
					"It's not my BSSID\n",
					__FUNCTION__));
		return 0;
	}

	if (bInitiator)
	{
		if (!MAC_ADDR_EQUAL(Ptr+14, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
						"It's not my MAC address\n",
						__FUNCTION__));
			hex_dump("Dst Mac=", Ptr+14, 6);
			return 0;
		}
		NdisMoveMemory(pSA, Ptr+8, MAC_ADDR_LEN);
	}
	else
	{
		if (!MAC_ADDR_EQUAL(Ptr+8, pAd->CurrentAddress))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR, ("%s() - "
						"It's not my MAC address\n",
						__FUNCTION__));
			hex_dump("Dst Mac=", Ptr+8, 6);
			return 0;
		}
		NdisMoveMemory(pSA, Ptr+14, MAC_ADDR_LEN);
	}

	/* Offset to PU Buffer Status */
	Ptr += 20;
	RemainLen -= 20;
	return (MsgLen - RemainLen);
}

#ifdef WFD_SUPPORT
/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
BOOLEAN TDLS_PeerTunneledProbeReqRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT UCHAR *pChannel,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement,
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement)
{
	PFRAME_802_11		pFrame;
	PEID_STRUCT         pEid;
	ULONG				Length = 0;
	BOOLEAN				brc = FALSE;
	PUCHAR				Ptr;
	BOOLEAN				bFirstP2pOUI = TRUE;
	BOOLEAN				bLastIsP2pOUI = FALSE;
	PUCHAR				pP2PIeConLen = NULL;	/* pointer to 2 bytes to indicate Contenated length of all P2P IE */
	ULONG				P2PIeConLen = 0; 	/*  Contenated length of all P2P IE */
	ULONG			idx;

	pFrame = (PFRAME_802_11)Msg;
	Length = LENGTH_802_11;

	*P2PSubelementLen = 0;
#ifdef WFD_SUPPORT
	*pWfdSubelementLen = 0;
#endif /* WFD_SUPPORT */
	*pSsidLen = 0;
	*Peerip = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	
	Ptr = pFrame->Octet;

	hex_dump("TDLS_PeerTunneledProbeReqRspSanity", Ptr, MsgLen - LENGTH_802_11);
	/* LLC+SNAP */
	Ptr += LENGTH_802_1_H;
	Length += LENGTH_802_1_H;

	/* Payload Type(1) + Category(1) + OU(3) + Frame Body Type(1) */
	Ptr += 6;
	Length += 6;

	pEid = (PEID_STRUCT) Ptr;
	printk("starting parsing form:: %02x %02x %02x %02x\n", *Ptr, *(Ptr+1), *(Ptr+2), *(Ptr+3));

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)    
	{
		switch(pEid->Eid)
		{			
			case IE_SSID:
				bLastIsP2pOUI = FALSE;
				if(pEid->Len <= MAX_LEN_OF_SSID)
				{
					RTMPMoveMemory(Ssid, pEid->Octet, pEid->Len);
					*pSsidLen = pEid->Len;
				}
				break;
			case IE_VENDOR_SPECIFIC:
				bLastIsP2pOUI = FALSE;
				/* Check the OUI version, filter out non-standard usage */
				if (NdisEqualMemory(pEid->Octet, WPS_OUI, 4) && (pEid->Len >= 4))
				{
					if (*P2PSubelementLen == 0)
					{
						RTMPMoveMemory(pP2pSubelement, &pEid->Eid, pEid->Len +2);
						*P2PSubelementLen = pEid->Len +2;
					}
					else if (*P2PSubelementLen > 0)
					{
						if (((*P2PSubelementLen) + (pEid->Len+2)) <= MAX_VIE_LEN)
						{
							RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, pEid->Len+2);
							*P2PSubelementLen += (pEid->Len+2);
						}
						else
						{
							DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 111 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__ ,((*P2PSubelementLen) + (pEid->Len+2))));
							return FALSE;
						}
					}
				}
				else if (NdisEqualMemory(pEid->Octet, P2POUIBYTE, 4) && (pEid->Len >= 4))
				{
					brc = TRUE;
					bLastIsP2pOUI = TRUE;
					/* If this is the first P2P OUI. Then also append P2P OUI. */
					if (bFirstP2pOUI == TRUE)
					{
						/* Althought this is first P2P IE. */
						/* still need to Check *P2PSubelementLen, because *P2PSubelementLen also includes WPS IE. */
						if (*P2PSubelementLen == 0)
						{
							RTMPMoveMemory(pP2pSubelement, &pEid->Eid, 2);
							*(pP2pSubelement + 2) = 0;
							/* Make one more byte for P2P accumulated length. */
							RTMPMoveMemory(pP2pSubelement + 3, &pEid->Octet[0], pEid->Len);
							pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
							*P2PSubelementLen = (pEid->Len + 3);
							P2PIeConLen = pEid->Len;	/* Real P2P IE length is Len. */
							DBGPRINT(RT_DEBUG_INFO, ("SYNC -1-1 P2PIeConLen  = %ld\n", P2PIeConLen));
						}
						else if (*P2PSubelementLen > 0)
						{
							if (((*P2PSubelementLen) + (pEid->Len+3)) <= MAX_VIE_LEN)
							{
								RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Eid, 2);
								*(pP2pSubelement + *P2PSubelementLen + 2) = 0;
								/* Make one more byte for P2P accumulated length. */
								RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen + 3, &pEid->Octet[0], pEid->Len);
								pP2PIeConLen = pP2pSubelement + *P2PSubelementLen + 1;
								*P2PSubelementLen += (pEid->Len+3);
								/* bFirstP2pOUI is TURE. So use =   */
								P2PIeConLen = pEid->Len;
								DBGPRINT(RT_DEBUG_INFO, (" -1-2 P2PIeConLen  = %ld\n", P2PIeConLen));
							}
							else
							{
								DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 222 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__ ,((*P2PSubelementLen) + (pEid->Len+3))));
								return FALSE;
							}
						}
						bFirstP2pOUI = FALSE;
					}
					else if (bLastIsP2pOUI == TRUE)
					{
						/* If this is not the first P2P OUI. Then don't append P2P OUI. */
						/* because our parse function doesn't need so many P2P OUI. */
						if ((*P2PSubelementLen > 0) && (pEid->Len > 4))
						{
							if (((*P2PSubelementLen) + (pEid->Len-4)) <= MAX_VIE_LEN)
							{
								RTMPMoveMemory(pP2pSubelement + *P2PSubelementLen, &pEid->Octet[4], pEid->Len-4);
								*P2PSubelementLen += (pEid->Len-4);
								P2PIeConLen += (pEid->Len - 4);
							}
							else
							{
								DBGPRINT(RT_DEBUG_ERROR, ("%s: ERROR!! 333 Sum of P2PSubelementLen= %lu, > MAX_VIE_LEN !!\n", __FUNCTION__ ,((*P2PSubelementLen) + (pEid->Len-4))));
								return FALSE;
							}
						}
					}
				}
				else if ((pAd->StaCfg.WfdCfg.bWfdEnable) &&
					NdisEqualMemory(pEid->Octet, WIFIDISPLAY_OUI, 4) && (pEid->Len >= 4))
				{
					hex_dump("WFD_IE", &pEid->Eid, pEid->Len +2);
					RTMPMoveMemory(pWfdSubelement, &pEid->Eid, pEid->Len +2);
					*pWfdSubelementLen = pEid->Len + 2;
				}
				break;
			default : 
				bLastIsP2pOUI = FALSE;
				break;

		}
		Length = Length + 2 + pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	
	}

	if ((P2PIeConLen != 0) && (pP2PIeConLen != NULL))
	{
		*pP2PIeConLen = (UCHAR)(P2PIeConLen%256);
		*(pP2PIeConLen+1) = (UCHAR)(P2PIeConLen/256);
		DBGPRINT(RT_DEBUG_INFO, ("  - 3 P2PIeConLen  = %ld. /256 = %ld. *P2PSubelementLen = %ld \n", P2PIeConLen, (P2PIeConLen/256), *P2PSubelementLen));
		DBGPRINT(RT_DEBUG_INFO, ("  -  %x %x \n", *pP2PIeConLen,  *(pP2PIeConLen+1) ));
		for (idx = 0; idx < (*P2PSubelementLen);)
		{
			DBGPRINT_RAW(RT_DEBUG_INFO, ("%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-%x-", 
				*(pP2pSubelement+idx), *(pP2pSubelement+idx+1), *(pP2pSubelement+idx+2), *(pP2pSubelement+idx+3)
				,*(pP2pSubelement+idx+4) ,*(pP2pSubelement+idx+5) ,*(pP2pSubelement+idx+6),*(pP2pSubelement+idx+7)
				,*(pP2pSubelement+idx+8),*(pP2pSubelement+idx+9),*(pP2pSubelement+idx+10),*(pP2pSubelement+idx+11)));
			
			 idx = idx + 12;
		}
		
	}
	return brc;
}
#endif /* WFD_SUPPORT */

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_SendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull)
{
	UCHAR	NullFrame[48];
	ULONG	Length;
	PHEADER_802_11	pHeader_802_11;
	UCHAR	idx;
	PRT_802_11_TDLS	pTDLS = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

    /* WPA 802.1x secured port control */
    if (((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA) || 
         (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPAPSK) ||
         (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2) || 
         (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef WPA_SUPPLICANT_SUPPORT
			  || (pAd->StaCfg.wdev.IEEE8021X == TRUE)		
#endif 		
#ifdef WAPI_SUPPORT
		  || (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWAICERT)
		  || (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
        ) &&
       (pAd->StaCfg.wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)) 
	{
		return;
	}

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

		if ((pTDLS->Valid) && (pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			NdisZeroMemory(NullFrame, 48);
			Length = sizeof(HEADER_802_11);

			pHeader_802_11 = (PHEADER_802_11) NullFrame;

			pHeader_802_11->FC.Type = FC_TYPE_DATA;
			pHeader_802_11->FC.SubType = SUBTYPE_DATA_NULL;
			pHeader_802_11->FC.ToDs = 0;
			pHeader_802_11->FC.FrDs = 0;
			COPY_MAC_ADDR(pHeader_802_11->Addr1, pTDLS->MacAddr);
			COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);

			if (pAd->CommonCfg.bAPSDForcePowerSave)
			{
				pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
			}
			else
			{
				pHeader_802_11->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd) == TRUE) ? 1: 0;
			}

			pHeader_802_11->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, TxRate, 14);

			pAd->Sequence++;
			pHeader_802_11->Sequence = pAd->Sequence;

			/* Prepare QosNull function frame */
			if (bQosNull)
			{
				pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;

				/* copy QOS control bytes */
				NullFrame[Length]	=  0;
				NullFrame[Length+1] =  0;
				Length += 2;/* if pad with 2 bytes for alignment, APSD will fail */
			}

			HAL_KickOutNullFrameTx(pAd, 0, NullFrame, Length);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID TDLS_LinkMaintenance(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR idx;
	PRT_802_11_TDLS	pTDLS = NULL;
#ifdef TDLS_AUTOLINK_SUPPORT
	PLIST_HEADER	pTdlsBlackEnList = &pAd->StaCfg.TdlsInfo.TdlsBlackList;
#endif // TDLS_AUTOLINK_SUPPORT //

	for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
	{
		pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[idx];

		if ((pTDLS->Valid == TRUE) && (pTDLS->Status == TDLS_MODE_CONNECTED))
		{
			UCHAR wcid = pTDLS->MacTabMatchWCID;
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[wcid];
		
			if(!IS_ENTRY_TDLS(pEntry))
				continue;

			if (pAd->StaCfg.wdev.WepStatus != Ndis802_11EncryptionDisabled)
			{
				NdisAcquireSpinLock(&pAd->MacTabLock);
				pEntry->TdlsKeyLifeTimeCount++;
				NdisReleaseSpinLock(&pAd->MacTabLock);

				if (pEntry->TdlsKeyLifeTimeCount >= pTDLS->KeyLifetime)
				{
#ifdef TDLS_AUTOLINK_SUPPORT
					PLIST_HEADER	pTdlsDiscovryEnList = &pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList;
#endif // TDLS_AUTOLINK_SUPPORT //
					DBGPRINT(RT_DEBUG_WARN, 
								("ageout %02x:%02x:%02x:%02x:%02x:%02x from TDLS #%d after %d-sec silence\n",
								PRINT_MAC(pEntry->Addr), idx, TDLS_ENTRY_AGEOUT_TIME));

					NdisAcquireSpinLock(&pAd->StaCfg.TdlsInfo.TDLSEntryLock);
					pTDLS->Token = 0;
					pTDLS->Valid = FALSE;
					pTDLS->Status = TDLS_MODE_NONE;
					NdisReleaseSpinLock(&pAd->StaCfg.TdlsInfo.TDLSEntryLock);

					TDLS_TearDownAction(pAd, pTDLS, TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON, FALSE);
#ifdef TDLS_AUTOLINK_SUPPORT
					TDLS_DelDiscoveryEntryByMAC(pTdlsDiscovryEnList, pTDLS->MacAddr);
#endif // TDLS_AUTOLINK_SUPPORT //

					MacTableDeleteEntry(pAd, pTDLS->MacTabMatchWCID, pTDLS->MacAddr);
				}
			}

			/* UAPSD also use the variable to do some check */
			NdisAcquireSpinLock(&pAd->MacTabLock);
			pEntry->NoDataIdleCount++;
			NdisReleaseSpinLock(&pAd->MacTabLock);

#ifdef TDLS_UAPSD_DEBUG
			/* virtual timeout handle */
			RTMP_PS_VIRTUAL_TIMEOUT_HANDLE(pEntry);
#else /* TDLS_UAPSD_DEBUG */
#ifdef UAPSD_SUPPORT
			/* one second timer */
			UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */
#endif /* TDLS_UAPSD_DEBUG */
		}
	}

#ifdef TDLS_AUTOLINK_SUPPORT
	TDLS_MaintainBlackList(pAd, pTdlsBlackEnList);
#endif // TDLS_AUTOLINK_SUPPORT //
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
INT Set_TdlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR arg)
{
	INT i;

	DBGPRINT(RT_DEBUG_OFF, ("\n%-19s\n", "MAC\n"));
	for (i=0; i<MAX_NUM_OF_TDLS_ENTRY; i++)
	{
		if ((pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
		{
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacTabMatchWCID];

			DBGPRINT(RT_DEBUG_OFF, ("%02x:%02x:%02x:%02x:%02x:%02x  \n",
				pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[0], pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[1], pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[2],
				pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[3], pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[4], pAd->StaCfg.TdlsInfo.TDLSEntry[i].MacAddr[5]));
			/*DBGPRINT(RT_DEBUG_OFF, ("%-8d\n", pAd->StaCfg.DLSEntry[i].TimeOut)); */

			DBGPRINT(RT_DEBUG_OFF, ("\n"));
			DBGPRINT(RT_DEBUG_OFF, ("\n%-19s%-4s%-4s%-4s%-4s%-7s%-7s%-7s","MAC", "AID", "BSS", "PSM", "WMM", "RSSI0", "RSSI1", "RSSI2"));
#ifdef DOT11_N_SUPPORT			
			DBGPRINT(RT_DEBUG_OFF, ("%-8s%-10s%-6s%-6s%-6s%-6s", "MIMOPS", "PhMd", "BW", "MCS", "SGI", "STBC"));
#endif /* DOT11_N_SUPPORT */
			DBGPRINT(RT_DEBUG_OFF, ("\n%02X:%02X:%02X:%02X:%02X:%02X  ",
				pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
				pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->Aid));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->apidx));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->PsMode));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi0));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi1));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi2));
#ifdef DOT11_N_SUPPORT
			DBGPRINT(RT_DEBUG_OFF, ("%-8d", (int)pEntry->MmpsMode));
			DBGPRINT(RT_DEBUG_OFF, ("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.STBC));
#endif /* DOT11_N_SUPPORT */
			DBGPRINT(RT_DEBUG_OFF, ("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0));
			DBGPRINT(RT_DEBUG_OFF, ("\n"));

		}
	}

	return TRUE;
}

#define PKT_CHANNEL_MAX_WAIT_CYCLE 100

VOID
TDLS_DisableMacTxRx(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 Data = 0;
	UINT32 macStatus;
	UINT32 MTxCycle;

	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> \n", __FUNCTION__));

	//Disable MAC Tx/Rx
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
	Data &= (~0xC);
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);

	// Check MAC Tx/Rx idle
	for (MTxCycle = 0; MTxCycle < PKT_CHANNEL_MAX_WAIT_CYCLE; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
		if (macStatus & 0x3)
			RtmpusecDelay(1000);
		else
			break;
	}

	if (MTxCycle == PKT_CHANNEL_MAX_WAIT_CYCLE)
	{
		printk("Polling MAC idle to max !!\n");
	}
}

VOID
TDLS_EnableMacTx(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 Data = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> \n", __FUNCTION__));

	//Enable MAC Tx
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
	Data |= 0x4;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
}

VOID
TDLS_EnableMacRx(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 Data = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> \n", __FUNCTION__));

	//Enable MAC Rx
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
	Data |= 0x8;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
}

VOID
TDLS_EnableMacTxRx(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 Data = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> \n", __FUNCTION__));

	//Enable MAC Tx/Rx
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
	Data |= 0xC;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
}


// TODO: shiang-usw, need to revise for new arch
VOID TDLS_DisablePktChannel(RTMP_ADAPTER *pAd, UCHAR QSel)
{
	UINT32 Data = 0;
	UINT32 MTxCycle = 0, MaxCycle = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): FixME!!\n", __FUNCTION__));
		return;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	/* Set HCCA/EDCA Q mode to manual-mode  */
	RTMP_IO_READ32(pAd, PBF_CFG, &Data);
	Data |= ((1 << 10) | (1 << 11));
	RTMP_IO_WRITE32(pAd, PBF_CFG, Data);


#ifdef RTMP_MAC_PCI
	if (QSel == FIFO_HCCA)
	{
		/* Polling HCCA Out-Q until empty  */
		for (MTxCycle = 0; MTxCycle < 10; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, TXRXQ_STA, &Data);

			if (((Data >> 11) & 0x1f) == 0)
				break;
			else
				RtmpusecDelay(1000);
		}
	}
	else if (QSel == FIFO_EDCA)
	{
		/* Polling EDCA Out-Q until empty  */
		for (MTxCycle = 0; MTxCycle < 10; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, TXRXQ_STA, &Data);

			if (((Data >> 19) & 0x1f) == 0)	
				break;
			else
				RtmpusecDelay(1000);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("unknow Out-Q\n"));
		return;
	}

	if (MTxCycle >= 10)
		DBGPRINT(RT_DEBUG_ERROR, ("Polling %s Out-Q max\n", (QSel == FIFO_EDCA ? "EDCA" : "HCCA")));

	/* Disable PBF HCCA/EDCA */
	RTMP_IO_READ32(pAd, PBF_CFG, &Data);
	Data &= (~0xC);
	RTMP_IO_WRITE32(pAd, PBF_CFG, Data);
#endif /* RTMP_MAC_PCI */
#endif /* RTMP_MAC */
}


VOID TDLS_EnablePktChannel(RTMP_ADAPTER *pAd, UCHAR QSel)
{
	UINT32 Data = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): FixME!!\n", __FUNCTION__));
		return;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	RTMP_IO_READ32(pAd, PBF_CFG, &Data);

	if (QSel == FIFO_HCCA)	
	{
		Data |= 0x8;
		Data &= ~(1 << 11);
	}
	else if (QSel == FIFO_EDCA)
	{
		Data |=0x4;
		Data &= ~(1 << 10);
	}
	else
	{
		Data |= 0xC;
		Data &= ~(1 << 10);
		Data &= ~(1 << 11);
	}

	RTMP_IO_WRITE32(pAd, PBF_CFG, Data);
#endif /* RTMP_MAC */
}

VOID
TDLS_InitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bEnableMACTxRx,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	UCHAR	Value = 0;
	UINT32	Data = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s() ===> \n", __FUNCTION__));

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */


	if (pAd->StaCfg.TdlsInfo.TdlsCurrentTargetChannel == TargetChannel)
	{
#ifdef RTMP_MAC_PCI
		RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */


		return;
	}

	pAd->StaCfg.TdlsInfo.TdlsCurrentTargetChannel = TargetChannel;

#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */


	TDLS_DisableMacTxRx(pAd);

#ifdef DOT11_N_SUPPORT
	if (TargetChannelBW == EXTCHA_ABOVE)
	{	
		/* Must using 40MHz. */
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, (TargetChannel + 2), FALSE);
		AsicLockChannel(pAd, (TargetChannel + 2));
			
		/* TX/RX Bandwidth control */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		
		/*  TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data &= 0xfffffffe;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

		/*  RX : control channel at lower */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value &= (~0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
		
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
			DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}	

		DBGPRINT(RT_DEBUG_WARN, ("A. %d. %ld @!\n", TargetChannel, (jiffies * 1000) / OS_HZ));
	}
	else if (TargetChannelBW == EXTCHA_BELOW)
	{	
	    /* Must using 40MHz. */
		pAd->CommonCfg.BBPCurrentBW = BW_40;
		AsicSwitchChannel(pAd, (TargetChannel - 2), FALSE);
		AsicLockChannel(pAd, (TargetChannel - 2));
		
		/* TX/RX Bandwidth control */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		Value |= 0x10;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);
		
		/*  TX : control channel at lower */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
		Data |= 0x1;
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);
		
		/* RX : control channel at upper */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
		Value |= (0x20);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);
	
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
			DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}

		DBGPRINT(RT_DEBUG_WARN, ("B. %d. %ld @!\n", TargetChannel, (jiffies * 1000) / OS_HZ));
    }
    else
#endif /* DOT11_N_SUPPORT */
    {
		/* Must using 20MHz. */
		pAd->CommonCfg.BBPCurrentBW = BW_20;
		AsicSwitchChannel(pAd, TargetChannel, FALSE);
		AsicLockChannel(pAd, TargetChannel);
		
		/* TX/RX Bandwidth control */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
		Value &= (~0x18);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

#ifdef RTMP_MAC_PCI
        pAd->StaCfg.BBPR3 = Value;
#endif /* RTMP_MAC_PCI */
		
		if (pAd->MACVersion == 0x28600100)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
			DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
		}
		
		DBGPRINT(RT_DEBUG_WARN, ("%d... %ld @!\n", TargetChannel, (jiffies * 1000) / OS_HZ));
	}

	if (bEnableMACTxRx)
	{
		/* Enable MAC Tx/Rx to receive packet */
		TDLS_EnableMacTxRx(pAd);
	}
	else
	{
		/* Enable MAC Rx to receive packet */
		TDLS_EnableMacRx(pAd);
	}

	pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel = TargetChannel;
	pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannelBW = (TargetChannelBW == EXTCHA_NONE) ? (BW_20) : (BW_40);

	//RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

	//AsicBBPReadWithRxChain(pAd, BBP_R66, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

	DBGPRINT(RT_DEBUG_TRACE, ("%s() <=== \n", __FUNCTION__));

}

VOID TDLS_UpdateHwNullFramePwr(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR PwrMgmt,
	IN CHAR Index)
{
	UINT32	longValue;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	RTMP_IO_READ32(pAd,  pAd->NullBufOffset[Index] + TXWISize, &longValue);
	if (PwrMgmt == PWR_ACTIVE)
		longValue &= (~0x00001000);
	else
		longValue |= 0x00001000;
	RTMP_IO_WRITE32(pAd, pAd->NullBufOffset[Index] + TXWISize, longValue);
}

VOID TDLS_KickOutHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR PwrMgmt,
	IN CHAR Index)
{
	TDLS_UpdateHwNullFramePwr(pAd, PwrMgmt, Index);

	/* kick NULL frame #0 */
	RTMP_IO_WRITE32(pAd, PBF_CTRL, 0x80);
}

#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
			do{													\
				if (bIntContext == FALSE)						\
				RTMP_IRQ_LOCK((lock), IrqFlags);		\
			}while(0)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
			do{													\
				if (bIntContext == FALSE)						\
					RTMP_IRQ_UNLOCK((lock), IrqFlags);	\
			}while(0)

NDIS_STATUS
TDLS_SendOutActionFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN UINT DataLen,
	IN UCHAR FrameType)
{
	PACKET_INFO PacketInfo;
	PUCHAR pSrcBufVA;
	UINT SrcBufLen;
	UINT AllowFragSize;
	UCHAR NumberOfFrag;
	UCHAR RTSRequired;
	UCHAR QueIdx, UserPriority;
	unsigned int IrqFlags;
	UCHAR Rate;
	TX_BLK TxBlk;
	TX_BLK *pTxBlk;
	BOOLEAN hasTxDesc = FALSE;
	PNDIS_PACKET    pPacket;
	NDIS_STATUS     Status;
#ifdef RTMP_MAC_PCI
	ULONG FreeNumber[NUM_OF_TX_RING];
#endif /* RTMP_MAC_PCI */

	if ((!pEntry) || (!IS_ENTRY_TDLS(pEntry)))
	{
		DBGPRINT(RT_DEBUG_ERROR,  ("%s() --> unknow Entry !!!\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}

	/* build a NDIS packet*/
	Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s (error:: can't allocate NDIS PACKET)\n", __FUNCTION__));
		return NDIS_STATUS_FAILURE;
	}


	RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, MAIN_MBSSID);	/* set a default value*/
	if(pEntry->apidx != 0)
		RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, pEntry->apidx);

	RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->Aid);
	RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

	if (FrameType != 0)
		RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, FrameType);
	else
		RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, 0);

	/* Prepare packet information structure for buffer descriptor */
	/* chained within a single NDIS packet. */
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s() - pSrcBufVA == NULL !!!SrcBufLen=%x\n", __FUNCTION__, SrcBufLen));
		/* Resourece is low, system did not allocate virtual address */
		/* return NDIS_STATUS_FAILURE directly to upper layer */
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (SrcBufLen < 14) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s() - Ndis Packet buffer error !!!\n", __FUNCTION__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return (NDIS_STATUS_FAILURE);
	}

	/* In HT rate adhoc mode, A-MPDU is often used. So need to lookup BA Table and MAC Entry. */
	/* Note multicast packets in adhoc also use BSSID_WCID index. */
	
	if (pAd->StaCfg.BssType == BSS_INFRA)
	{
		if (VALID_WCID(pEntry->Aid) &&
			(IS_ENTRY_TDLS(&pAd->MacTab.Content[pEntry->Aid])))
		{
			pEntry = &pAd->MacTab.Content[pEntry->Aid];
			Rate = pAd->MacTab.Content[pEntry->Aid].CurrTxRate;
		}		
		else
		{
			pEntry = &pAd->MacTab.Content[BSSID_WCID];
			RTMP_SET_PACKET_WCID(pPacket, BSSID_WCID);
			Rate = pAd->CommonCfg.TxRate;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s() - TDLS can't work in ADHOC !!!\n", __FUNCTION__));

		/* Resourece is low, system did not allocate virtual address */
		/* return NDIS_STATUS_FAILURE directly to upper layer */
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return (NDIS_STATUS_FAILURE);
	}

	if (!pEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR,
			 ("%s() -Cannot find pEntry(%2x:%2x:%2x:%2x:%2x:%2x) in MacTab!\n",
			  __FUNCTION__, PRINT_MAC(pSrcBufVA)));
		/* Resourece is low, system did not allocate virtual address */
		/* return NDIS_STATUS_FAILURE directly to upper layer */
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	/* Check the Ethernet Frame type of this packet, and set the RTMP_SET_PACKET_SPECIFIC flags. */
	/*              Here we set the PACKET_SPECIFIC flags(LLC, VLAN, DHCP/ARP, EAPOL). */
	UserPriority = 5;
	QueIdx = QID_AC_VI;
	RTMP_SET_PACKET_TDLS(pPacket, 1);

#ifdef UAPSD_SUPPORT
	/* the code must behind RTMPCheckEtherType() to get QueIdx */
	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(pEntry->FlgPsModeIsWakeForAWhile == FALSE) &&
		(pEntry->bAPSDDeliverEnabledPerAC[QueIdx] == 0))
	{
		/* the packet will be sent to AP, not the peer directly */
		pEntry = &pAd->MacTab.Content[BSSID_WCID];

		/* for AP entry, pEntry->bAPSDDeliverEnabledPerAC[QueIdx] always is 0 */
		RTMP_SET_PACKET_WCID(pPacket, BSSID_WCID);
		Rate = pAd->CommonCfg.TxRate;
	}
#endif /* UAPSD_SUPPORT */

	/*
	   STEP 1. Decide number of fragments required to deliver this MSDU. 
	   The estimation here is not very accurate because difficult to 
	   take encryption overhead into consideration here. The result 
	   "NumberOfFrag" is then just used to pre-check if enough free 
	   TXD are available to hold this MSDU.
	 */
	if (*pSrcBufVA & 0x01)	/* fragmentation not allowed on multicast & broadcast */
	NumberOfFrag = 1;
	else if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
		NumberOfFrag = 1;	/* Aggregation overwhelms fragmentation */
	else if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED))
		NumberOfFrag = 1;	/* Aggregation overwhelms fragmentation */
#ifdef DOT11_N_SUPPORT
	else if ((pEntry->HTPhyMode.field.MODE == MODE_HTMIX)
		 || (pEntry->HTPhyMode.field.MODE == MODE_HTGREENFIELD))
		NumberOfFrag = 1;	/* MIMO RATE overwhelms fragmentation */
#endif /* DOT11_N_SUPPORT */
	else {
		/*
		   The calculated "NumberOfFrag" is a rough estimation because of various 
		   encryption/encapsulation overhead not taken into consideration. This number is just
		   used to make sure enough free TXD are available before fragmentation takes place.
		   In case the actual required number of fragments of an NDIS packet 
		   excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
		   last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of 
		   resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should 
		   rarely happen and the penalty is just like a TX RETRY fail. Affordable.
		 */

		AllowFragSize =
		    (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 -
		    LENGTH_CRC;
		NumberOfFrag =
		    ((PacketInfo.TotalPacketLength - LENGTH_802_3 +
		      LENGTH_802_1_H) / AllowFragSize) + 1;
		/* To get accurate number of fragmentation, Minus 1 if the size just match to allowable fragment size */
		if (((PacketInfo.TotalPacketLength - LENGTH_802_3 +
		      LENGTH_802_1_H) % AllowFragSize) == 0) {
			NumberOfFrag--;
		}
	}

	/* Save fragment number to Ndis packet reserved field */
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);

	/*
	   STEP 2. Check the requirement of RTS:
	   If multiple fragment required, RTS is required only for the first fragment
	   if the fragment size large than RTS threshold
	   For RT28xx, Let ASIC send RTS/CTS
	 */
	if (NumberOfFrag > 1)
		RTSRequired =
		    (pAd->CommonCfg.FragmentThreshold >
		     pAd->CommonCfg.RtsThreshold) ? 1 : 0;
	else
		RTSRequired =
		    (PacketInfo.TotalPacketLength >
		     pAd->CommonCfg.RtsThreshold) ? 1 : 0;

	/* Save RTS requirement to Ndis packet reserved field */
	RTMP_SET_PACKET_RTS(pPacket, RTSRequired);

	RTMP_SET_PACKET_TXRATE(pPacket, Rate);

	RTMP_SET_PACKET_UP(pPacket, UserPriority);

#ifdef UAPSD_SUPPORT
	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(pEntry->FlgPsModeIsWakeForAWhile == FALSE) &&
		(pEntry->bAPSDDeliverEnabledPerAC[QueIdx] != 0))
	{
		/*
			only UAPSD of the peer for the queue is set;
			for non-UAPSD queue, we will send it to the AP.
		*/
		if (RtmpInsertPsQueue(pAd, pPacket, pEntry, QueIdx) != NDIS_STATUS_SUCCESS)
			return NDIS_STATUS_FAILURE;
	}
#endif /* UAPSD_SUPPORT */

	//RTMP_START_DEQUEUE(pAd, QueIdx, IrqFlags);

	DEQUEUE_LOCK(&pAd->irq_lock, FALSE, IrqFlags);
#ifdef RTMP_MAC_PCI
	FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);

	if (FreeNumber[QueIdx] <= 5)
	{
		/* free Tx(QueIdx) resources*/
		RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
		FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
	}
#endif /* RTMP_MAC_PCI */

	pTxBlk = &TxBlk;
	NdisZeroMemory((PUCHAR)pTxBlk, sizeof(TX_BLK));
	/*InitializeQueueHeader(&pTxBlk->TxPacketList);		 Didn't need it because we already memzero it.*/
	pTxBlk->QueIdx = QueIdx;

#ifdef VENDOR_FEATURE1_SUPPORT
	pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */

	/* Early check to make sure we have enoguh Tx Resource.*/
	hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
	if (!hasTxDesc)
	{
		pAd->PrivateInfo.TxRingFullCnt++;

		/* Resourece is low, system did not allocate virtual address */
		/* return NDIS_STATUS_FAILURE directly to upper layer */
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		DEQUEUE_UNLOCK(&pAd->irq_lock, FALSE, IrqFlags);
		return NDIS_STATUS_FAILURE;
	}

	pTxBlk->TxFrameType = TX_LEGACY_FRAME;
	pTxBlk->TotalFrameNum++;
	pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
	pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
	pTxBlk->pPacket = pPacket;

	InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));


	STAHardTransmit(pAd, pTxBlk, QueIdx);

#ifdef RTMP_MAC_PCI
	DEQUEUE_UNLOCK(&pAd->irq_lock, FALSE, IrqFlags);
#endif /* RTMP_MAC_PCI */

#ifdef VENDOR_FEATURE1_SUPPORT
	if (++pAd->FifoUpdateDone >= 4)
	{
		NICUpdateFifoStaCounters(pAd);
		pAd->FifoUpdateDone = 0;
	}
#else
	NICUpdateFifoStaCounters(pAd);
#endif /* VENDOR_FEATURE1_SUPPORT */

	//RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);


	return NDIS_STATUS_SUCCESS;
}

VOID TDLS_SendOutNullFrame(
	IN	PRTMP_ADAPTER pAd,
	IN	PMAC_TABLE_ENTRY pEntry,
	IN	BOOLEAN bQosNull,
	IN	BOOLEAN bWaitACK)
{
	UCHAR	NullFrame[48];
	ULONG	Length;
	PHEADER_802_11	pHeader_802_11;
#ifdef RTMP_MAC_PCI
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR rtmpHwHdr[TXINFO_SIZE +  pAd->chipCap.TXWISize];
	ULONG FreeNum;
	NDIS_STATUS  	Status = NDIS_STATUS_SUCCESS;
	PNDIS_PACKET	pPacket;
	PACKET_INFO 	PacketInfo;
	PUCHAR pSrcBufVA;
	UINT	SrcBufLen;
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	/* WPA 802.1x secured port control */
	if (((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA) || 
		(pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPAPSK) ||
		(pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2) || 
		(pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK) 		
#ifdef WAPI_SUPPORT
		|| (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWAICERT)
		|| (pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWAIPSK)
#endif /* WAPI_SUPPORT */
		) &&
		(pAd->StaCfg.wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("===> %s() - Port not Secured !!!\n", __FUNCTION__));
		return;
	}
	
	NdisZeroMemory(NullFrame, 48);
	Length = sizeof(HEADER_802_11);

	pHeader_802_11 = (PHEADER_802_11) NullFrame;

	pHeader_802_11->FC.Type = FC_TYPE_DATA;
	pHeader_802_11->FC.SubType = SUBTYPE_DATA_NULL;
	pHeader_802_11->FC.ToDs = 0;
	pHeader_802_11->FC.FrDs = 0;
	COPY_MAC_ADDR(pHeader_802_11->Addr1, pEntry->Addr);
	COPY_MAC_ADDR(pHeader_802_11->Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pHeader_802_11->Addr3, pAd->CommonCfg.Bssid);

	if (pAd->CommonCfg.bAPSDForcePowerSave)
	{
		pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
	}
	else
	{
		pHeader_802_11->FC.PwrMgmt = (RtmpPktPmBitCheck(pAd) == TRUE) ? 1: 0;
	}

	pAd->Sequence++;
	pHeader_802_11->Sequence = pAd->Sequence;

	/* Prepare QosNull function frame */
	if (bQosNull)
	{
		pHeader_802_11->FC.SubType = SUBTYPE_QOS_NULL;

		/* copy QOS control bytes */
		NullFrame[Length]	=  0;
		NullFrame[Length+1] =  0;
		Length += 2;/* if pad with 2 bytes for alignment, APSD will fail */
	}


#ifdef RTMP_MAC_PCI
	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	/* Reset is in progress, stop immediately*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST)||
		!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		return;
	}

	FreeNum = GET_MGMTRING_FREENO(pAd);

	if ((FreeNum > 0))
	{
		/* We need to reserve space for rtmp hardware header. i.e., TxWI for RT2860 and TxInfo+TxWI for RT2870*/
		NdisZeroMemory(&rtmpHwHdr, (TXINFO_SIZE + TXWISize));
		Status = RTMPAllocateNdisPacket(pAd, &pPacket, (PUCHAR)&rtmpHwHdr, (TXINFO_SIZE + TXWISize), NullFrame, Length);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_WARN, ("%s() (error:: can't allocate NDIS PACKET)\n", __FUNCTION__));
			return;
		}
	
#ifdef UAPSD_SUPPORT
		UAPSD_MR_QOS_NULL_HANDLE(pAd, NullFrame, pPacket);
#endif /* UAPSD_SUPPORT */

		RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

		if (pSrcBufVA == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR,
				 ("%s() - pSrcBufVA == NULL !!!SrcBufLen=%x\n", __FUNCTION__, SrcBufLen));

			/* Resourece is low, system did not allocate virtual address */
			/* return NDIS_STATUS_FAILURE directly to upper layer */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return;
		}

		if (bWaitACK)
			RTMP_SET_PACKET_TDLS_WAIT_ACK(pPacket, 1);
		else
			RTMP_SET_PACKET_TDLS_WAIT_ACK(pPacket, 0);

		MlmeHardTransmitMgmtRing(pAd, QID_AC_BE, pPacket);
	}
	else
	{
		pAd->RalinkCounters.MgmtRingFullCount++;

		DBGPRINT(RT_DEBUG_ERROR, ("not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
									pAd->RalinkCounters.MgmtRingFullCount));
	}
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));
}

VOID TdlsMacHeaderInit(
	IN PRTMP_ADAPTER	pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
	IN PUCHAR pBssid) 
{
	NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	
	pHdr80211->FC.Type = FC_TYPE_DATA;
	pHdr80211->FC.SubType = SubType;
	pHdr80211->FC.ToDs = ToDs;
	pHdr80211->FC.FrDs = 0;

	if (ToDs == 1)
	{
		COPY_MAC_ADDR(pHdr80211->Addr1, pBssid);
		COPY_MAC_ADDR(pHdr80211->Addr3, pDA);
	}
	else
	{
		COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
		COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
	}

	COPY_MAC_ADDR(pHdr80211->Addr2, pAd->CurrentAddress);
}

VOID TDLS_SoftwareRecovery(
	IN PRTMP_ADAPTER pAd)
{
	ULONG Now;
	ULONG BPtoJiffies;
	UINT32 Data;

	NdisGetSystemUpTime(&Now);

	BPtoJiffies = (((pAd->CommonCfg.BeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);

	if (RTMP_TIME_AFTER(Now, pAd->StaCfg.LastBeaconRxTime + (3 * BPtoJiffies * pAd->StaCfg.DtimPeriod)))
	{
		UCHAR	Value = 0;
		UINT32	Data = 0;
		UCHAR idx;
		PRT_802_11_TDLS	pTDLS = NULL;
		PMAC_TABLE_ENTRY pMacEntry;

		RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
		RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);

		for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
		{
			pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];
			if (pTDLS->Valid && (pTDLS->Status >= TDLS_MODE_CONNECTED))
		{
				break;
			}
		}

		if (idx >= MAX_NUM_OF_TDLS_ENTRY)
		{
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
				return;
		}

		pAd->StaCfg.TdlsInfo.TdlsCurrentTargetChannel = pAd->CommonCfg.Channel;

		TDLS_DisableMacTxRx(pAd);

#ifdef DOT11_N_SUPPORT
		if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
		{	
			/* Must using 40MHz. */
			pAd->CommonCfg.BBPCurrentBW = BW_40;
			AsicSwitchChannel(pAd, (pAd->CommonCfg.Channel + 2), FALSE);
			AsicLockChannel(pAd, (pAd->CommonCfg.Channel + 2));

			/* TX/RX Bandwidth control */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			Value |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			/*  TX : control channel at lower */
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data &= 0xfffffffe;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			/*  RX : control channel at lower */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}	

			DBGPRINT(RT_DEBUG_WARN, ("A. %d. %ld @!\n", pAd->CommonCfg.Channel, (jiffies * 1000) / OS_HZ));
		}
		else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
		{	
			/* Must using 40MHz. */
			pAd->CommonCfg.BBPCurrentBW = BW_40;
			AsicSwitchChannel(pAd, (pAd->CommonCfg.Channel - 2), FALSE);
			AsicLockChannel(pAd, (pAd->CommonCfg.Channel - 2));

			/* TX/RX Bandwidth control */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			Value |= 0x10;
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			/*  TX : control channel at lower */
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data |= 0x1;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			/* RX : control channel at upper */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value |= (0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x1A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x0A);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x16);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}

			DBGPRINT(RT_DEBUG_WARN, ("B. %d. %ld @!\n", pAd->CommonCfg.Channel, (jiffies * 1000) / OS_HZ));
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			/* Must using 20MHz. */
			pAd->CommonCfg.BBPCurrentBW = BW_20;
			AsicSwitchChannel(pAd, pAd->CommonCfg.Channel , FALSE);
			AsicLockChannel(pAd, pAd->CommonCfg.Channel );

			/* TX/RX Bandwidth control */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &Value);
			Value &= (~0x18);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, Value);

			/*  TX : control channel at lower */
			RTMP_IO_READ32(pAd, TX_BAND_CFG, &Data);
			Data &= 0xfffffffe;
			RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Data);

			/* RX : control channel at upper */
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &Value);
			Value &= (~0x20);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, Value);

#ifdef RTMP_MAC_PCI
			pAd->StaCfg.BBPR3 = Value;
#endif /* RTMP_MAC_PCI */

			if (pAd->MACVersion == 0x28600100)
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, 0x08);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x11);
				DBGPRINT(RT_DEBUG_TRACE, ("!!!rt2860C !!! \n" ));
			}

			DBGPRINT(RT_DEBUG_WARN, ("%d... %ld @!\n", pAd->CommonCfg.Channel, (jiffies * 1000) / OS_HZ));
		}

		/* Enable MAC Tx/Rx to receive packet */
		TDLS_EnableMacTxRx(pAd);

		pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel = pAd->CommonCfg.Channel;
		pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannelBW = (pAd->CommonCfg.Channel == EXTCHA_NONE) ? (BW_20) : (BW_40);

		//RTMPSetAGCInitValue(pAd, pAd->CommonCfg.BBPCurrentBW);

		//AsicBBPReadWithRxChain(pAd, BBP_R66, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

		TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);

		RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
		RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		printk("1000.\n");
	}
}
#endif /* DOT11Z_TDLS_SUPPORT */
 
