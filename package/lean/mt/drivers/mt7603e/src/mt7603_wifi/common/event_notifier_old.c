/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	event_notifier.c
 
    Abstract:
    This is event notify feature used to send wireless event to upper layer.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifdef CONFIG_PUSH_SUPPORT
#include "rt_config.h"
//#include "hdev/hdev.h"

static NTSTATUS WHCStaJoin(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    MAC_TABLE_ENTRY *pEntry = NULL;
    UCHAR Channel;    
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_join *pStaJoin = &drvevnt.data.join;
    
    NdisZeroMemory(pStaJoin, sizeof(*pStaJoin));

    va_start(Args, pAd);
    pEntry = va_arg(Args, MAC_TABLE_ENTRY *);
    Channel = va_arg(Args, UINT32);
    va_end(Args);
    
    wdev = pEntry->wdev;
    pStaJoin->type = WHC_DRVEVNT_STA_JOIN;
    pStaJoin->channel = Channel;    
    COPY_MAC_ADDR(pStaJoin->sta_mac, pEntry->Addr);
    pStaJoin->aid = pEntry->Aid;

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_JOIN,
						    NULL, (PUCHAR)pStaJoin, sizeof(*pStaJoin));

	DBGPRINT(RT_DEBUG_TRACE, ("%s WHC_DRVEVNT_STA_JOIN send ....\n", __FUNCTION__));

    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaLeave(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    UCHAR *pStaMac = NULL;
    UCHAR Channel;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_leave *pStaLeave = &drvevnt.data.leave;

    NdisZeroMemory(pStaLeave, sizeof(*pStaLeave));
    
    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    pStaMac = va_arg(Args, UCHAR *);
    Channel = (UCHAR)va_arg(Args, UINT32);
    va_end(Args);

    pStaLeave->type = WHC_DRVEVNT_STA_LEAVE;
    COPY_MAC_ADDR(pStaLeave->sta_mac, pStaMac);
    pStaLeave->channel = Channel;

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_LEAVE,
						    NULL, (PUCHAR)pStaLeave, sizeof(*pStaLeave));

	DBGPRINT(RT_DEBUG_TRACE, ("%s WHC_DRVEVNT_STA_LEAVE send ....\n", __FUNCTION__));
    return NDIS_STATUS_SUCCESS;
}

static EVENT_TABLE_T EventHdlrTable[] = {
    {WHC_DRVEVNT_STA_JOIN,          WHCStaJoin},
    {WHC_DRVEVNT_STA_LEAVE,         WHCStaLeave},
    {DRVEVNT_END_ID, NULL}
};

EventHdlr GetEventNotiferHook(IN UINT32 EventID)
{
	UINT32 CurIndex = 0;
	UINT32 EventHdlrTableLength= sizeof(EventHdlrTable) / sizeof(EVENT_TABLE_T);
	EventHdlr Handler = NULL;

	if(EventID > DRVEVNT_END_ID)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Event ID(%d) is out of boundary.\n", EventID));
		return NULL;
	}

	for(CurIndex=0; CurIndex < EventHdlrTableLength; CurIndex++)
	{
		if (EventHdlrTable[CurIndex].EventID == EventID)
		{
			Handler = EventHdlrTable[CurIndex].EventHandler;
			break;
		}
	}

	if(Handler == NULL)
		DBGPRINT(RT_DEBUG_ERROR, ("No corresponding EventHdlr for this Event ID(%d).\n",  EventID));
    
	return Handler;
}

#endif /* CONFIG_PUSH_SUPPORT */
