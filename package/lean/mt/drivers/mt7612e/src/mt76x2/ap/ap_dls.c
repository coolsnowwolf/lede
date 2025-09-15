/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 
    Abstract:
 
    Handle WMM-DLS state machine.

***************************************************************************/

#include "rt_config.h"


#ifdef QOS_DLS_SUPPORT

/*
========================================================================
Routine Description:
    DLS state machine init, including state transition and timer init.

Arguments:
    pAd				points to our adapter
    S				pointer to the DLS state machine
	Trans[]			

Return Value:
    None

    Note:
        The state machine looks like the following 

                                    DLS_IDLE             
        MT2_PEER_DLS_REQ			PeerDlsReqAction 
        MT2_PEER_DLS_RSP			PeerDlsRspAction     
        MT2_PEER_DLS_TEAR_DOWN		PeerDlsTearDownAction
========================================================================
 */
VOID APDLSStateMachineInit(
    IN	PRTMP_ADAPTER		pAd, 
    IN  STATE_MACHINE		*S, 
    OUT STATE_MACHINE_FUNC	Trans[]) 
{
    StateMachineInit(S, (STATE_MACHINE_FUNC*)Trans, MAX_DLS_STATE, MAX_DLS_MSG,
					(STATE_MACHINE_FUNC)Drop, DLS_IDLE, DLS_MACHINE_BASE);

    StateMachineSetAction(S, DLS_IDLE, MT2_PEER_DLS_REQ,
							(STATE_MACHINE_FUNC)APPeerDlsReqAction);
    StateMachineSetAction(S, DLS_IDLE, MT2_PEER_DLS_RSP,
							(STATE_MACHINE_FUNC)APPeerDlsRspAction);
    StateMachineSetAction(S, DLS_IDLE, MT2_PEER_DLS_TEAR_DOWN,
							(STATE_MACHINE_FUNC)APPeerDlsTearDownAction);
}


/*
========================================================================
Routine Description:
	Handle peer DLS Request action frame.

Arguments:
    pAd				points to our adapter
    *pElem			action frame

Return Value:
    None

Note:
========================================================================
*/
VOID APPeerDlsReqAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*pElem)
{
	PMAC_TABLE_ENTRY	pDAEntry, pSAEntry;
	UCHAR				DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
	UINT16				CapabilityInfo;
	UINT16				DLSTimeout;
	PUCHAR				pOutBuffer = NULL;
	PFRAME_802_11		Fr;
	UINT16				Status;
	UINT32				FrameLen = 0;
	HEADER_802_11		DlsRspHdr;
	UCHAR				Category = CATEGORY_DLS;
	UCHAR				Action = ACTION_DLS_RESPONSE;
    UCHAR				SupportedRatesLen = 0;
    UCHAR				SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
	HT_CAPABILITY_IE	HtCapability;
	UCHAR				HtCapabilityLen;


	/* frame sanity check */
    if (!PeerDlsReqSanity(pAd, pElem->Msg, pElem->MsgLen, DA, SA,
							&CapabilityInfo, &DLSTimeout,
							&SupportedRatesLen, &SupportedRates[0],
							&HtCapabilityLen, &HtCapability))
	{
        return;
	}

	/* check whether the source station is legal */
	pSAEntry = MacTableLookup(pAd, SA);
	if (!pSAEntry)
		return;

	pSAEntry->bDlsInit	= FALSE;

	/* check whether the destination station exists in our associated table */
	pDAEntry = MacTableLookup(pAd, DA);
	if (!pDAEntry)
		Status = MLME_DEST_STA_NOT_IN_QBSS;
	else if (pDAEntry && (pDAEntry->apidx != pSAEntry->apidx))
		Status = MLME_DEST_STA_NOT_IN_QBSS;
	else if (pDAEntry && !CLIENT_STATUS_TEST_FLAG(pDAEntry, fCLIENT_STATUS_WMM_CAPABLE))
		Status = MLME_DEST_STA_IS_NOT_A_QSTA;
	else if (pDAEntry->WepStatus != pSAEntry->WepStatus)
		Status = MLME_QOS_UNSPECIFY; /* different security algorithm */
	else if (!pAd->ApCfg.MBSSID[pSAEntry->apidx].bDLSCapable)
		Status = MLME_DLS_NOT_ALLOW_IN_QBSS;
	else
		Status = MLME_SUCCESS;

	if (pDAEntry)
		pDAEntry->bDlsInit	= FALSE;

	/* forward DLS-Request to real destination */
	Fr = (PFRAME_802_11)pElem->Msg;

/*	pOutBuffer = kmalloc(MAX_LEN_OF_MLME_BUFFER, MEM_ALLOC_FLAG); */
	os_alloc_mem(pAd, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
	if(pOutBuffer == NULL)
		return;

#ifdef CONFIG_HOTSPOT
	if (pAd->ApCfg.MBSSID[pSAEntry->apidx].HotSpotCtrl.L2Filter)
		Status = MLME_DLS_NOT_ALLOW_IN_QBSS;
#endif

	/*
		If status is successful, forward DLS-Request frame to destination
		otherwise send DLS-Response with reason code to originator.
	*/
	if (Status == MLME_SUCCESS)
	{
		NdisMoveMemory(Fr->Hdr.Addr1, DA, MAC_ADDR_LEN);
		NdisMoveMemory(Fr->Hdr.Addr2, pAd->ApCfg.MBSSID[pSAEntry->apidx].wdev.bssid, MAC_ADDR_LEN);
		NdisMoveMemory(Fr->Hdr.Addr3, SA, MAC_ADDR_LEN);
		NdisMoveMemory(pOutBuffer, pElem->Msg, pElem->MsgLen);
		FrameLen = pElem->MsgLen;
	}
	else
	{
		/* response error to source station */
		MgtMacHeaderInit(pAd, &DlsRspHdr, SUBTYPE_ACTION, 0, SA,
						pAd->ApCfg.MBSSID[pSAEntry->apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[pSAEntry->apidx].wdev.bssid);

		/*
			Capability information and supported rate field are present
			only when status code is zero.
		*/
        MakeOutgoingFrame(pOutBuffer,				(ULONG *) &FrameLen,
                          sizeof(HEADER_802_11),	&DlsRspHdr,
                          1,						&Category,
                          1,						&Action,
                          2,						&Status,
                          6,						SA,
                          6,						DA,
                          END_OF_ARGS);
	}

	/* transmit the frame */
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
/*	kfree(pOutBuffer); */
	os_free_mem(NULL, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE,
			("DLS - APPeerDlsReqAction() from %02x:%02x:%02x:%02x:%02x:%02x "
			"with Status=%d\n",
			SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], Status));
}


/*
========================================================================
Routine Description:
	Handle peer DLS Response action frame.

Arguments:
    pAd				points to our adapter
    *pElem			action frame

Return Value:
    None

Note:
========================================================================
*/
VOID APPeerDlsRspAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*pElem)
{
	PMAC_TABLE_ENTRY	pDAEntry, pSAEntry;
	UCHAR				DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
	UINT16				CapabilityInfo;
	UINT16				StatusCode;
	PUCHAR				pOutBuffer = NULL;
	PFRAME_802_11		Fr;
	UINT32				FrameLen = 0;
    UCHAR				SupportedRatesLen = 0;
    UCHAR				SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR				HtCapabilityLen;
	HT_CAPABILITY_IE	HtCapability;


	/* frame sanity check */
	if (! PeerDlsRspSanity(pAd, pElem->Msg, pElem->MsgLen, DA, SA,
							&CapabilityInfo, &StatusCode,
							&SupportedRatesLen, &SupportedRates[0],
							&HtCapabilityLen, &HtCapability))
	{
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE,
			("DLS - PeerDlsRspAction() from %02x:%02x:%02x:%02x:%02x:%02x "
			"with StatusCode=%d\n",
			SA[0], SA[1], SA[2], SA[3], SA[4], SA[5], StatusCode));

	/* check whether the source station is legal */
	pSAEntry = MacTableLookup(pAd, SA);
	if (!pSAEntry)
    		return;

	pDAEntry = MacTableLookup(pAd, DA);
	if (!pDAEntry)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Destination station does not exist!\n"));
		return;
	}

	pSAEntry->bDlsInit = FALSE;

	/* forward DLS-Request to real destination */
	Fr = (PFRAME_802_11)pElem->Msg;

/*	pOutBuffer = kmalloc(MAX_LEN_OF_MLME_BUFFER, MEM_ALLOC_FLAG); */
	os_alloc_mem(pAd, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
	if (pOutBuffer == NULL)
		return; /* fatal error, no available memory */

	NdisMoveMemory(Fr->Hdr.Addr1, DA, MAC_ADDR_LEN);
	NdisMoveMemory(Fr->Hdr.Addr2, pAd->ApCfg.MBSSID[pSAEntry->apidx].wdev.bssid, MAC_ADDR_LEN);
	NdisMoveMemory(Fr->Hdr.Addr3, SA, MAC_ADDR_LEN);

	NdisMoveMemory(pOutBuffer, pElem->Msg, pElem->MsgLen);
	FrameLen = pElem->MsgLen;

	/* transmit the response frame */
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
/*	kfree(pOutBuffer); */
	os_free_mem(NULL, pOutBuffer);
}


/*
========================================================================
Routine Description:
	Handle peer DLS Tear down action frame.

Arguments:
    pAd				points to our adapter
    *pElem			action frame

Return Value:
    None

Note:
========================================================================
*/
VOID APPeerDlsTearDownAction(
    IN PRTMP_ADAPTER	pAd,
    IN MLME_QUEUE_ELEM	*pElem)
{
	PMAC_TABLE_ENTRY	pDAEntry, pSAEntry;
	UCHAR				DA[MAC_ADDR_LEN], SA[MAC_ADDR_LEN];
	UINT16				ReasonCode;
	PUCHAR				pOutBuffer = NULL;
	PFRAME_802_11		Fr;
	UINT32				FrameLen = 0;


	/* frame sanity check */
    if (! PeerDlsTearDownSanity(pAd, pElem->Msg, pElem->MsgLen,
									DA, SA, &ReasonCode))
	{
        return;
	}

    DBGPRINT(RT_DEBUG_TRACE,
			("DLS - PeerDlsTearDownAction() from %02x:%02x:%02x:%02x:%02x:%02x\n",
			SA[0], SA[1], SA[2], SA[3], SA[4], SA[5]));

	pSAEntry = MacTableLookup(pAd, SA);
    if (!pSAEntry)
    	return;

	pDAEntry = MacTableLookup(pAd, DA);
	if (!pDAEntry)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Destination station does not exist!\n"));
		return;
	}

	pSAEntry->bDlsInit = FALSE;

    /* forward Tear-down to real destination */
    Fr = (PFRAME_802_11)pElem->Msg;

/*	pOutBuffer = kmalloc(MAX_LEN_OF_MLME_BUFFER, MEM_ALLOC_FLAG); */
	os_alloc_mem(pAd, (UCHAR **)&pOutBuffer, MAX_LEN_OF_MLME_BUFFER);
	if (pOutBuffer == NULL)
		return; /* fatal error, no available memory */

	NdisMoveMemory(Fr->Hdr.Addr1, DA, MAC_ADDR_LEN);
	NdisMoveMemory(Fr->Hdr.Addr2, pAd->ApCfg.MBSSID[pSAEntry->apidx].wdev.bssid, MAC_ADDR_LEN);
	NdisMoveMemory(Fr->Hdr.Addr3, SA, MAC_ADDR_LEN);

	NdisMoveMemory(pOutBuffer, pElem->Msg, pElem->MsgLen);
	FrameLen = pElem->MsgLen;

	/* transmit the tear down frame */
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
/*	kfree(pOutBuffer); */
	os_free_mem(NULL, pOutBuffer);
}

#endif /* QOS_DLS_SUPPORT */

/* End of ap_dls.c */
