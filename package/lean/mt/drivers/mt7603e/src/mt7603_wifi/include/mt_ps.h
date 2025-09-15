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
	mt_io.h
*/

#ifndef __MT_PS_H__
#define __MT_PS_H__

VOID MtHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive);
BOOLEAN MtPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm);

#ifdef MT_PS
VOID MtPsRedirectDisableCheck(RTMP_ADAPTER *pAd, UCHAR wcid);
VOID MtPsSendToken(RTMP_ADAPTER *pAd, UINT32 WlanIdx);
VOID MtSetIgnorePsm(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR value);
VOID CheckSkipTX(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
void MtEnqTxSwqFromPsQueue(RTMP_ADAPTER *pAd, UCHAR qidx, STA_TR_ENTRY *tr_entry);

#ifdef RTMP_PCI_SUPPORT
#define MT_SET_IGNORE_PSM(__pAd, __pEntry, __value) \
	MtSetIgnorePsm(__pAd, __pEntry, __value)
#endif /* RTMP_PCI_SUPPORT */
	
#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
#define MT_SET_IGNORE_PSM(__pAd, __pEntry, __value) \
{ \
	UCHAR *__pData = NULL; \
	UCHAR __ps_en = __value; \
	os_alloc_mem(NULL, (UCHAR **)&__pData, sizeof(UCHAR)+sizeof(MAC_TABLE_ENTRY)); \
	if (__pData) \
	{ \
		NdisMoveMemory(__pData , &__ps_en, sizeof(UCHAR)); \
		NdisMoveMemory(__pData+sizeof(UCHAR) , __pEntry, sizeof(MAC_TABLE_ENTRY)); \
		RTEnqueueInternalCmd(__pAd, CMDTHREAD_PS_IGNORE_PSM_SET, __pData, sizeof(UCHAR)+sizeof(MAC_TABLE_ENTRY)); \
		os_free_mem(NULL, __pData); \
	} \
	else \
	{ \
		DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("%s(%d): os_alloc_mem fail.\n", __FUNCTION__, __LINE__)); \
	} \
}
#endif /* defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT) */
#endif /* MT_PS */

#endif

