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

	All related IEEE802.11r Resource Information Container body.

***************************************************************************/

#ifdef DOT11R_FT_SUPPORT


#include "rt_config.h"


#define TYPE_FUNC

#ifdef CONFIG_STA_SUPPORT
/* ----- RIC Linked List ----- */
/* insert a resource request of a AP */
#define FT_RIC_AP_LIST_INSERT_TAIL(__pAd, __pList, __pEntry)		\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		insertTailList((__pList), (LIST_ENTRY *)(__pEntry));		\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* get a resource request from the head of list */
#define FT_RIC_AP_LIST_REMOVE_HEAD(__pAd, __pList, __pEntry)		\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__pEntry = (FT_RIC_AP *)removeHeadList((__pList));			\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* get number of resource request */
#define FT_RIC_AP_LIST_SIZE_GET(__pAd, __pList, __Size)				\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__Size = getListSize((__pList));							\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* get a resource request by AP MAC */
#define FT_RIC_AP_LIST_ENTRY_GET(__pAd, __pList, __pApMac, __FlgIsFound, __pEntry)\
	{																\
		UINT32 __Size, __IdEntry;									\
		FT_RIC_AP *__pRscAp;										\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__Size = getListSize((__pList));							\
		__FlgIsFound = FALSE;										\
		for(__IdEntry=0; __IdEntry<__Size; __IdEntry++)				\
		{															\
			__pRscAp = (FT_RIC_AP *)removeHeadList((__pList));		\
			if (memcmp(__pRscAp->AP_MAC, __pApMac, ETH_ALEN) == 0)	\
			{														\
				memcpy(__pEntry, __pRscAp, sizeof(FT_RIC_AP));		\
				insertTailList((__pList), (LIST_ENTRY *)(__pRscAp));\
				__FlgIsFound = TRUE;								\
				break;												\
			}														\
			insertTailList((__pList), (LIST_ENTRY *)(__pRscAp));	\
		}															\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* set status to a resource request by AP MAC */
#define FT_RIC_AP_LIST_ENTRY_SET(__pAd, __pList, __pApMac, __pEntry)\
	{																\
		UINT32 __Size, __IdEntry;									\
		FT_RIC_AP *__pRscAp;										\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__Size = getListSize((__pList));							\
		for(__IdEntry=0; __IdEntry<__Size; __IdEntry++)				\
		{															\
			__pRscAp = (FT_RIC_AP *)removeHeadList((__pList));		\
			if (memcmp(__pRscAp->AP_MAC, __pApMac, ETH_ALEN) == 0)	\
			{														\
				memcpy(__pRscAp, __pEntry, sizeof(FT_RIC_AP));		\
				insertTailList((__pList), (LIST_ENTRY *)(__pRscAp));\
				break;												\
			}														\
			insertTailList((__pList), (LIST_ENTRY *)(__pRscAp));	\
		}															\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* delete a resource request by AP MAC */
#define FT_RIC_AP_LIST_ENTRY_DEL(__pAd, __pList, __pApMac)			\
	{																\
		UINT32 __Size, __IdEntry;									\
		FT_RIC_AP *__pRscAp;										\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__Size = getListSize((__pList));							\
		for(__IdEntry=0; __IdEntry<__Size; __IdEntry++)				\
		{															\
			__pRscAp = (FT_RIC_AP *)removeHeadList((__pList));		\
			if (memcmp(__pRscAp->AP_MAC, __pApMac, ETH_ALEN) == 0)	\
			{														\
				os_free_mem(NULL, __pRscAp);						\
				break;												\
			}														\
			insertTailList((__pList), (LIST_ENTRY *)(__pRscAp));	\
		}															\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}

/* clear all resource requests */
#define FT_RIC_AP_LIST_EMPTY(__pAd, __pList)						\
	{																\
		FT_RIC_AP *__pEntry;										\
		RTMP_SEM_LOCK(&(__pAd)->FT_RicLock);						\
		__pEntry = (FT_RIC_AP *)removeHeadList((__pList));			\
		while(__pEntry != NULL)										\
		{															\
			os_free_mem(NULL, __pEntry);							\
			__pEntry = (FT_RIC_AP *)removeHeadList((__pList));		\
		}															\
		RTMP_SEM_UNLOCK(&(__pAd)->FT_RicLock);						\
	}
#endif /* CONFIG_STA_SUPPORT */




/*
========================================================================
Routine Description:
	Initialize FT RIC Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID TYPE_FUNC FT_RIC_Init(
	IN	PRTMP_ADAPTER		pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("ap_ftrc> Initialize FT RIC Module...\n"));

#ifdef CONFIG_STA_SUPPORT
	/* allocate control block */
	if (pAd->pFT_RIC_Ctrl_BK == NULL)
	{
		os_alloc_mem(pAd, (UCHAR **)&pAd->pFT_RIC_Ctrl_BK,
						sizeof(FT_RIC_CTRL_BLOCK));

		if (pAd->pFT_RIC_Ctrl_BK == NULL)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ap_ftrc> Allocate control block fail!\n"));
			return;
		} /* End of if */

		/* init spin lock */
		NdisAllocateSpinLock(pAd, &(pAd->FT_RicLock));

		/* init resource list */
		initList(&(FT_RIC_CB->ResourceList));
	} /* End of if */
#endif /* CONFIG_STA_SUPPORT */
} /* End of FT_RIC_Init */


/*
========================================================================
Routine Description:
	Release FT RIC Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID TYPE_FUNC FT_RIC_Release(
	IN	PRTMP_ADAPTER		pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("ap_ftrc> Release FT RIC Module...\n"));

#ifdef CONFIG_STA_SUPPORT
	if (pAd->pFT_RIC_Ctrl_BK != NULL)
	{
		/* free resource list */
		FT_RIC_AP_LIST_EMPTY(pAd, &(FT_RIC_CB->ResourceList));

		/* free spin lock */
		NdisFreeSpinLock(&(pAd->FT_RicLock));

		os_free_mem(pAd, pAd->pFT_RIC_Ctrl_BK);
		pAd->pFT_RIC_Ctrl_BK = NULL;
	} /* End of if */
#endif /* CONFIG_STA_SUPPORT */
} /* End of FT_KDP_Release */


#ifdef CONFIG_STA_SUPPORT
/*
========================================================================
Routine Description:
	Start to do resource request for other APs.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	TRUE			- successfully
	FALSE			- no any resource is needed to request

Note:
	1. After the function is called, no any new TSPEC can be issued from us.
========================================================================
*/
BOOLEAN TYPE_FUNC FT_RIC_ResourceRequestStart(
	IN	PRTMP_ADAPTER			pAd)
{
	BOOLEAN Status = TRUE;


	FT_RIC_AP_LIST_EMPTY(pAd, &FT_RIC_CB->ResourceList);
	return Status;
} /* End of FT_RIC_ResourceRequestStart */


/*
========================================================================
Routine Description:
	End to do resource request for other APs.

Arguments:
	pAd				- WLAN control block pointer
	*pApMac			- the new AP MAC

Return Value:
	None

Note:
	1. After the function is called, no any new TSPEC can be issued from us.
========================================================================
*/
VOID TYPE_FUNC FT_RIC_ResourceRequestEnd(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					*pApMac)
{

	/* empty all backup resource request entries */
	FT_RIC_AP_LIST_EMPTY(pAd, &FT_RIC_CB->ResourceList);
} /* End of FT_RIC_ResourceRequestEnd */


/*
========================================================================
Routine Description:
	Issue resource requests.

Arguments:
	pAd				- WLAN control block pointer
	*pApMac			- the AP MAC
	*pBufReq		- the request packet buffer
	ReqMaxLen		- the maximum remain length of request packet buffer

Return Value:
	filled element length

Note:
	1. Same resource type for all resource descriptors in the RDIE.
	2. Suppose RDIEs are continual.
	3. If pDescpBlk != NULL, skip current TSPEC.

	4. 11A.11.3 Creation and handling of a resource request (11r D9.0)
		(1) In using TSPECs for requesting QoS resources, the TSPECs in the
			request need not belong to only active Traffic Streams;
		(2) The non-AP STA can send TSPECs for any Traffic Stream that it
			intends to use after the transition, and request the same resources
			that would be requested by a later ADDTS exchange.

		==> We do NOT support the function.

	5. Resource request usage for QSTA:
		Case 1:
		(1) Scan or get neighbor report of other QAPs;
		(2) Maybe signal is bad with current AP and good for other QAPs;

		-- FT_RIC_ResourceRequestStart()
			No new TSPEC can be issued.

		-- FT_RIC_ResourceRequest()
			Build current TSPEC to the QAP1.

		(3) FT confirm to QAP1;

		-- FT_RIC_ResourceResponseHandle() for QAP1.

		(4) Handle FT ACK from QAP1;

		(6) a. Signal is still bad so determining to reassociate a QAP;
			b. Maybe signal is good again with current QAP, no roaming is needed;

		-- FT_RIC_ResourceRequestEnd(ptr to TSPEC status)

		Case 2:
		(1) Scan or get neighbor report of other QAPs;
		(2) Maybe signal is bad with current AP and good for other QAPs;

		-- FT_RIC_ResourceRequestStart()
			No new TSPEC can be issued.

		-- FT_RIC_ResourceRequest()
			Build current TSPEC to the QAP1.

		(3) FT confirm to QAP1;

		-- FT_RIC_ResourceRequest()
			Build current TSPEC to the QAP2.

		(4) FT confirm to QAP2;

		-- (get success percentage) FT_RIC_ResourceResponseHandle(ptr to TSPEC status) for QAP1.
		-- (get success percentage) FT_RIC_ResourceResponseHandle(ptr to TSPEC status) for QAP2.

		(5) Handle FT ACK from different QAPs;
		(6) a. Signal is still bad so determining to reassociate a QAP;
			b. Maybe signal is good again with current QAP, no roaming is needed;
			c. Select the best resource allocation;

		-- FT_RIC_ResourceRequestEnd(ptr to TSPEC status)
			New TSPECs can be issued.
========================================================================
*/
UINT32 TYPE_FUNC FT_RIC_ResourceRequest(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					*pApMac,
	IN	UCHAR					*pBufReq,
	IN	UINT32					ReqMaxLen)
{
	FT_RIC_DESCP_BLOCK_ACK DescpBlk, *pDescpBlk;
	FT_RIC_AP *pRscAp;
	UINT32 FilledLen;



	/* sanity check */
	FT_RIC_AP_LIST_ENTRY_DEL(pAd, &FT_RIC_CB->ResourceList, pApMac);

	/* init */
	FilledLen = 0;

	/* allocate a resource record for the AP */
/*	pRscAp = (FT_RIC_AP *)kmallocos(sizeof(FT_RIC_AP), GFP_ATOMIC); */
	os_alloc_mem(NULL, (UCHAR **)&pRscAp, sizeof(FT_RIC_AP));
	if (pRscAp == NULL)
		return 0; /* allocate fail */
	/* End of if */
	memset(pRscAp, 0, sizeof(FT_RIC_AP));
	memcpy(pRscAp->AP_MAC, pApMac, ETH_ALEN);


	/* fill BLOCK ACK resource request */
	pRscAp->FlgHasBaResource = FALSE;
	pDescpBlk = &DescpBlk;

	/* try to get and fill BLOCK ACK resource */
	/* yet implement */


	/* backup the resource record */
	if (FilledLen > 0)
	{
		FT_RIC_AP_LIST_INSERT_TAIL(pAd, &FT_RIC_CB->ResourceList, pRscAp);
	}
	else
	{
	} /* End of if */

	DBGPRINT(RT_DEBUG_TRACE,
				("ft_ric> FilledLen = %d\n", FilledLen));

	return FilledLen;
} /* End of FT_RIC_ResourceRequest */


/*
========================================================================
Routine Description:
	Handle resource responses.

Arguments:
	pAd				- WLAN control block pointer
	*pApMac			- the AP MAC
	*pBufRsp		- the response packet buffer
	RspLen			- the maximum remain length of response packet buffer
	*pRspStatus		- the status report

Return Value:
	handled length

Note:
	1. Same resource type for all resource descriptors in the RDIE.
	2. Suppose RDIEs are continual.
========================================================================
*/
UINT32 TYPE_FUNC FT_RIC_ResourceResponseHandle(
	IN	PRTMP_ADAPTER			pAd,
	IN	UCHAR					*pApMac,
	IN	UCHAR					*pBufRsp,
	IN	UINT32					RspLen,
	OUT	FT_RIC_STATUS			*pRspStatus)
{
	FT_ELM_RIC_DATA_INFO *pElmDataInfoRsp;
	UCHAR *pBufRspElm, *pElmRspDescp;
	FT_RIC_AP RscAp, *pRscAp;
	UINT32 DescpCnt;
	UINT32 HandledSize;
	UCHAR RDIE_Identifier;
	BOOLEAN FlgIsFound;



	/* init */
	if (pRspStatus == NULL)
		return 0;
	/* End of if */

	memset(pRspStatus, 0, sizeof(FT_RIC_STATUS));
	pRscAp = &RscAp;

	/* sanity check resource request */
	FT_RIC_AP_LIST_ENTRY_GET(pAd, &FT_RIC_CB->ResourceList, \
							pApMac, FlgIsFound, pRscAp);

	if (FlgIsFound == FALSE)
	{
		/* no any resource requset is sent to the AP */
		DBGPRINT(RT_DEBUG_TRACE,
					("ft_ric> No any request resource is found!\n"));
		return 0;
	} /* End of if */

	/* init */
	pBufRspElm = pBufRsp;
	HandledSize = 0;

	memcpy(pRspStatus->AP_MAC, pApMac, sizeof(pRspStatus->AP_MAC));

	pRspStatus->FlgHasBaResource = FALSE;
	pRspStatus->FlgIsBaAccepted = FALSE;
	pRspStatus->TspecNumberOfRequested = 0;


	/* handle all RDIEs */
	/*
		The non-AP STA shall construct the RIC with a number of
		Resource Requests, each delineated by an RDIE.
	*/
	while(HandledSize < RspLen)
	{
		/* sanity check the RDIE */
		pElmDataInfoRsp = (FT_ELM_RIC_DATA_INFO *)pBufRspElm;

		if (pElmDataInfoRsp->ElmID != FT_ELM_ID_RIC_DATA_INFO)
			break;
		/* End of if */

		/* parsing the RIC descriptors */
		/*
			The Resource Count shall be set to the number of alternative
			Resource Descriptors that follow.
		*/
		RDIE_Identifier = pElmDataInfoRsp->RDIE_Identifier;
		DescpCnt = pElmDataInfoRsp->RD_Count; /* must be 0 or 1 */

		HandledSize += FT_ELM_HDR_LEN + pElmDataInfoRsp->ElmLen;
		pElmRspDescp = pBufRspElm + FT_ELM_HDR_LEN + pElmDataInfoRsp->ElmLen;

		if ((DescpCnt >= 1) &&
			((*pElmRspDescp) != FT_ELM_ID_RIC_DATA_INFO) &&
			(HandledSize < RspLen))
		{
			/* only one resource descriptor is accepted is enough */
			/*
				If there are multiple Resource Descriptors then these
				are treated as "choices" by the target AP.
			*/
			switch(*pElmRspDescp)
			{
				case FT_ELM_ID_RIC_DESCP_INFO:
					/* BLOCK ACK */
					DBGPRINT(RT_DEBUG_TRACE,
							("ft_ric> A resource BLOCK ACK response is found!\n"));

					HandledSize += (FT_ELM_HDR_LEN + *(pElmRspDescp+1));

					if ((pRscAp->FlgHasBaResource == TRUE) &&
						(RDIE_Identifier == pRscAp->RDIE_IdentifierBA))
					{
						if (pElmDataInfoRsp->StatusCode == 0)
						{
							pRspStatus->FlgHasBaResource = TRUE;
							pRspStatus->FlgIsBaAccepted = TRUE;
						} /* End of if */
					}
					else
						pRspStatus->UnknownRDIE ++;
					/* End of if */
					break;

				default:
					HandledSize += (FT_ELM_HDR_LEN + *(pElmRspDescp+1));

					break;
			} /* End of switch */

			/* check next RDIE */
			/* one RDIE + one Resource response (status == SUCCESS) */
			pElmRspDescp += FT_ELM_HDR_LEN + *(pElmRspDescp+1);
			pBufRspElm = pElmRspDescp;
		}
		else
		{
			/* only one RDIE (status != SUCCESS) */
			BOOLEAN FlgIsHandled = FALSE;

			/* check if the RDIE is for BA resource */
			if ((pRscAp->FlgHasBaResource == TRUE) &&
				(RDIE_Identifier == pRscAp->RDIE_IdentifierBA))
			{
				pRspStatus->FlgHasBaResource = TRUE;
				pRspStatus->FlgIsBaAccepted = FALSE;
				FlgIsHandled = TRUE;
			}

			if (FlgIsHandled == FALSE)
				pRspStatus->UnknownRDIE ++;
			/* End of if */

			/* check next RDIE */
			pBufRspElm += (FT_ELM_HDR_LEN + pElmDataInfoRsp->ElmLen);
		} /* End of if */
	} /* End of while */

	/* save content */
	FT_RIC_AP_LIST_ENTRY_SET(pAd, &FT_RIC_CB->ResourceList, pApMac, pRscAp);
	return HandledSize;
} /* End of FT_RIC_ResourceResponseHandle */
#endif /* CONFIG_STA_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Handle the resource request.

Arguments:
	pAd				- WLAN control block pointer
	*pCdb			- the source QSTA
	*pBufReq		- the request packet buffer
	ReqLen			- the length of request packet buffer
	*pBufRsp		- the response packet buffer
	*pRspLen		- the length of response content

Return Value:
	non-zero		- the frame length we handled
	zero			- no request is found

Note:
	1. The start octet of the buffer is the element ID of RIC request or response.
	2. Same resource type for all resource descriptors in the RDIE.
	3. Suppose RDIEs are continual.
========================================================================
*/
UINT32 TYPE_FUNC FT_RIC_ResourceRequestHandle(
	IN	PRTMP_ADAPTER			pAd,
	IN	MAC_TABLE_ENTRY			*pCdb,
	IN	UCHAR					*pBufReq,
	IN	UINT32					ReqLen,
	OUT	UCHAR					*pBufRsp,
	OUT	UINT32					*pRspLen)
{
	FT_ELM_RIC_DATA_INFO *pElmDataInfoReq, *pElmDataInfoRsp;
	UCHAR *pBufReqElm, *pElmReqDescp;
	UCHAR *pBufRspElm;
	ULONG ElmLen, RspLen;
	UINT32 DescpCnt;
	UINT32 HandledSize;
	BOOLEAN FlgIsTspecAccepted;


	/* init */
	pBufReqElm = pBufReq;
	pBufRspElm = pBufRsp;
	*pRspLen = 0;
	HandledSize = 0;

	/* sanity check */
	if ((pCdb == NULL) ||
		(pBufReq == NULL) ||
		(pBufRsp == NULL) ||
		(pRspLen == NULL))
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ft_ric> (ResourceRequestHandle) Error! NULL pointer!\n"));
		return 0;
	} /* End of if */

	/* handle all RDIEs */
	/*
		The non-AP STA shall construct the RIC with a number of
		Resource Requests, each delineated by an RDIE.
	*/
	while(HandledSize < ReqLen)
	{
		/* sanity check the RDIE */
		pElmDataInfoReq = (FT_ELM_RIC_DATA_INFO *)pBufReqElm;

		if (pElmDataInfoReq->ElmID != FT_ELM_ID_RIC_DATA_INFO)
			break;
		/* End of if */

		/* make up reponse */
		/*
			The RIC Response shall comprise one RDIE for each RDIE in the
			RIC Request.
		*/
		pElmDataInfoRsp = (FT_ELM_RIC_DATA_INFO *)pBufRspElm;
		pElmDataInfoRsp->ElmID = FT_ELM_ID_RIC_DATA_INFO;
		pElmDataInfoRsp->ElmLen = FT_ELM_LEN_RIC_DATA_INFO;

		/*
			The RDIE Identifier in the RDIE shall be an arbitrary value
			chosen by the non-AP STA that uniquely identifies the RDIE
			within the RIC.
		*/
		pElmDataInfoRsp->RDIE_Identifier = pElmDataInfoReq->RDIE_Identifier;

		pBufRspElm += sizeof(FT_ELM_RIC_DATA_INFO);
		*pRspLen += sizeof(FT_ELM_RIC_DATA_INFO);


		/* parsing the RIC descriptors */
		/*
			The Resource Count shall be set to the number of alternative
			Resource Descriptors that follow.
		*/
		DescpCnt = pElmDataInfoReq->RD_Count;
		FlgIsTspecAccepted = FALSE;

		pElmReqDescp = ((UCHAR *)pElmDataInfoReq);
		pElmReqDescp += FT_ELM_HDR_LEN + pElmDataInfoReq->ElmLen;
		HandledSize += FT_ELM_HDR_LEN + pElmDataInfoReq->ElmLen;

		while(DescpCnt > 0)
		{
			if (FlgIsTspecAccepted == FALSE)
			{
				/* only one resource descriptor is accepted is enough */
				/*
					If there are multiple Resource Descriptors then these
					are treated as "choices" by the target AP.
				*/
				switch(*pElmReqDescp)
				{
					case IE_FT_RIC_DESCRIPTOR:
						/* BLOCK ACK */
						/* reject the resource */
						DBGPRINT(RT_DEBUG_TRACE,
								("ft_ric> Handle a resource request BLOCK ACK...\n"));

						if (BA_ResourceAllocate(pAd,
											pCdb,
											pElmReqDescp,
											pBufRspElm,
											&RspLen) == 0)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("ft_ric> Accept BA.\n"));
							FlgIsTspecAccepted = TRUE;
						}
						break;

					default:
						break;
				} /* End of switch */
			} /* End of if */

			/* find the next RD Descriptor element */
			ElmLen = *(pElmReqDescp+1);
			pElmReqDescp += FT_ELM_HDR_LEN + ElmLen;
			HandledSize += FT_ELM_HDR_LEN + ElmLen;

			DescpCnt --;
		} /* End of while */


		/* re-assign the reponse RD Count/status code of RDIE */
		if (FlgIsTspecAccepted == TRUE)
		{
			/*
				A resource request is considered successful by a non-AP STA
				if the Status Code 0 is returned in each RDIE.
			*/
			pElmDataInfoRsp->RD_Count = 1;
			pElmDataInfoRsp->StatusCode = 0;

			pBufRspElm += RspLen;
			*pRspLen += RspLen;

			DBGPRINT(RT_DEBUG_TRACE,
					("ft_ric> Accept a resource request (RspLen: %d).\n", *pRspLen));
		}
		else
		{
			/*
				Status Code non-zero indicates that the resources could not
				be accepted.

				In this case the AP may include a single Resource Descriptor
				following the RDIE indicating a suggested resource that could
				have been accepted.

				The Resource Count field shall be set to '0' or '1' depending
				whether the suggested Resource Descriptor is attached.

				A non-zero Status Code in an RDIE shall not cause a non-zero
				Status Code in the auth/assoc frame containing the RIC.
			*/

			/* we do NOT support suggested Resource Descriptor */

			pElmDataInfoRsp->RD_Count = 0; /* 0 or 1, we only support 0 */
			pElmDataInfoRsp->StatusCode = FT_STATUS_CODE_RESERVED;

			DBGPRINT(RT_DEBUG_TRACE, ("ft_ric> Resource request fails!\n"));
		} /* End of if */


		/* check next RDIE */
		pBufReqElm = pElmReqDescp;
	} /* End of while */

	return HandledSize;
} /* End of FT_RIC_ResourceRequestHandle */


/*
========================================================================
Routine Description:
	Handle the resource request.

Arguments:
	pAd				- WLAN control block pointer
	*pMacEntry		- the source QSTA
	*pBufReq		- the request packet buffer
	ReqLen			- the length of request packet buffer
	*pBufRsp		- the response packet buffer
	*pRspLen		- the length of response content

Return Value:
	non-zero		- the frame length we handled
	zero			- no request is found

Note:
	1. The start octet of the buffer is the element ID of RIC request or response.
	2. Same resource type for all resource descriptors in the RDIE.
	3. Suppose RDIEs are continual.
========================================================================
*/
UINT32 BA_ResourceAllocate(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR *pBufRscReq,
	OUT UCHAR *pBufRscRsp,
	OUT ULONG *pBufRspLen)
{
extern BOOLEAN BARecSessionAdd(
					   IN PRTMP_ADAPTER    pAd, 
					   IN MAC_TABLE_ENTRY  *pEntry,
					   IN PFRAME_ADDBA_REQ pFrame);

	UINT8 IEId = IE_FT_RIC_DESCRIPTOR;
	UINT8 RicType = FT_RIC_TYPE_BA;
	UINT8 Len = 7;
	UINT16 Status = 1;
	BA_PARM BaParm; /*0 - 2 */
	USHORT TimeOutValue;

	FRAME_ADDBA_REQ AddreqFrame;
	FT_ELM_RIC_DESCP_INFO *pRicDesInfo;
	FT_RIC_DESCP_BLOCK_ACK *pRicBaInfo;

	if ((pAd->CommonCfg.bBADecline == FALSE) && IS_HT_STA(pMacEntry))
	{
		UINT16 Value;
		pRicDesInfo = (FT_ELM_RIC_DESCP_INFO *)pBufRscReq;
		pRicBaInfo = (FT_RIC_DESCP_BLOCK_ACK *)pRicDesInfo->Container;
		Value = le2cpu16(*(UINT16 *)(&pRicBaInfo->BaParm));
		NdisMoveMemory((PUCHAR)&AddreqFrame.BaParm, &Value, sizeof(UINT16));
		AddreqFrame.TimeOutValue = le2cpu16(pRicBaInfo->TimeOutValue);
		AddreqFrame.BaStartSeq.word = le2cpu16(pRicBaInfo->BaStartSeq);

		DBGPRINT(RT_DEBUG_OFF, ("Rcv Wcid(%d) AddBAReq\n", pMacEntry->Aid));
		if (BARecSessionAdd(pAd, pMacEntry, &AddreqFrame))
			Status = 0;
		else
			Status = 38; /* more parameters have invalid values */
	}
	else
	{
		Status = 37; /* the request has been declined. */
	}

	if (Status != 0)
		return Status;

	BaParm.BAPolicy = IMMED_BA;
	BaParm.AMSDUSupported = 0;
	BaParm.TID = AddreqFrame.BaParm.TID;
	BaParm.BufSize = min(((UCHAR)AddreqFrame.BaParm.BufSize), (UCHAR)pAd->CommonCfg.BACapability.field.RxBAWinLimit);
	if (BaParm.BufSize == 0)
	{
		BaParm.BufSize = 64; 
	}
	TimeOutValue = 0;

	*(UINT16 *)(&BaParm) = cpu2le16(*(USHORT *)(&BaParm));
	Status = cpu2le16(Status);
	TimeOutValue = cpu2le16(TimeOutValue);

	MakeOutgoingFrame(	pBufRscRsp,	pBufRspLen,
						1,			&IEId,
						1,			&Len,
						1,			&RicType,
						2,			&Status,
						2,			(UCHAR *)&BaParm,
					  	2,  		&TimeOutValue,
						END_OF_ARGS);

	return Status;
}

#endif /* CONFIG_AP_SUPPORT */

#endif /* DOT11R_FT_SUPPORT */

/* End of ap_ftrc.c */
