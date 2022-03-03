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

} /* End of FT_KDP_Release */




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
	if (pRspLen == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ft_ric> (ResourceRequestHandle) Error! NULL pointer!\n"));
		return 0;
	}
	*pRspLen = 0;
	HandledSize = 0;

	/* sanity check */
	if ((pCdb == NULL) ||
		(pBufReq == NULL) ||
		(pBufRsp == NULL)
		)
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
