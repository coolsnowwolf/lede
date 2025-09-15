/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wsc_v2.c
*/

#include    "rt_config.h"

#ifdef WSC_V2_SUPPORT
#include    "wsc_tlv.h"

BOOLEAN	WscAppendV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	UCHAR			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen);

#ifdef CONFIG_AP_SUPPORT
VOID 	WscOnOff(
	IN  PRTMP_ADAPTER	pAd,
	IN  INT				ApIdx,
	IN  BOOLEAN			bOff)
{
	PWSC_V2_INFO	pWpsV2Info = &pAd->ApCfg.MBSSID[ApIdx & 0x0F].WscControl.WscV2Info;
	if (bOff)
	{
		/*
			AP must not support WEP in WPS V2
		*/
		pWpsV2Info->bWpsEnable = FALSE;
		pAd->ApCfg.MBSSID[ApIdx & 0x0F].WscIEBeacon.ValueLen = 0;
		pAd->ApCfg.MBSSID[ApIdx & 0x0F].WscIEProbeResp.ValueLen = 0;
		DBGPRINT(RT_DEBUG_TRACE, ("WscOnOff - OFF.\n"));
	}
	else
	{
		pWpsV2Info->bWpsEnable = TRUE;
		if (pAd->ApCfg.MBSSID[ApIdx & 0x0F].WscControl.WscConfMode != WSC_DISABLE)
		{
			INT IsAPConfigured;
			IsAPConfigured = pAd->ApCfg.MBSSID[ApIdx & 0x0F].WscControl.WscConfStatus;
			WscBuildBeaconIE(pAd, IsAPConfigured, FALSE, 0, 0, (ApIdx & 0x0F), NULL, 0, AP_MODE);
			WscBuildProbeRespIE(pAd, WSC_MSGTYPE_AP_WLAN_MGR, IsAPConfigured, FALSE, 0, 0, ApIdx, NULL, 0, AP_MODE);
			DBGPRINT(RT_DEBUG_TRACE, ("WscOnOff - ON.\n"));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("WscOnOff - bWpsEnable = %d\n", pWpsV2Info->bWpsEnable));
}

VOID	WscAddEntryToAclList(
	IN  PRTMP_ADAPTER	pAd,
	IN	INT				ApIdx,
	IN  PUCHAR			pMacAddr)
{
	PRT_802_11_ACL	pACL = NULL;
	INT				i;
	BOOLEAN			bFound = FALSE;
	
	pACL = &pAd->ApCfg.MBSSID[ApIdx].AccessControlList;

	if ((pACL->Policy == 0) ||
		(pACL->Policy == 2))
		return;
	
	if (pACL->Num >= (MAX_NUM_OF_ACL_LIST - 1))
    {
		DBGPRINT(RT_DEBUG_WARN, ("The AccessControlList is full, and no more entry can join the list!\n"));		
		return;
	}

	for (i=0; i<pACL->Num; i++)
	{
		if (NdisEqualMemory(pACL->Entry[i].Addr, pMacAddr, MAC_ADDR_LEN))
			bFound = TRUE;
	}
	if (bFound == FALSE)
	{
		NdisMoveMemory(pACL->Entry[i].Addr, pMacAddr, MAC_ADDR_LEN);
		pACL->Num++;
	}
}
#endif /* CONFIG_AP_SUPPORT */

/*
	Vendor ID: 0x00372A
	Subelements ID (one-octet)
		Version2				0x00
		AuthorizedMACs			0x01
		Network Key Shareable	0x02
		Request to Enroll		0x03
		Settings Delay Time		0x04
		Reserved for future use	0x05 to 0xFF
	Length (one-octet) - number of octets in the payload of this subelment
		Version2				1B
		AuthorizedMACs			<=30B
		Network Key Shareable	Bool
		Request to Enroll		Bool
		Settings Delay Time		1B
*/
BOOLEAN	WscGenV2Msg(
	IN  PWSC_CTRL		pWpsCtrl,
	IN  BOOLEAN			bSelRegistrar,
	IN	PUCHAR			pAuthorizedMACs,
	IN  INT   			AuthorizedMACsLen,
	OUT	UCHAR			**pOutBuf,
	OUT	INT				*pOutBufLen)
{
	PUCHAR			pWscV2Msg = NULL;
	USHORT			WscV2MsgLen = 0;
	PWSC_REG_DATA	pReg = &pWpsCtrl->RegData;
	INT				templen = 0;
		
	os_alloc_mem(NULL, (UCHAR **)&pWscV2Msg, 128);
	if (pWscV2Msg)
	{
		UCHAR TmpLen = 0;
		pWscV2Msg[0] = 0x00;
		pWscV2Msg[1] = 0x37;
		pWscV2Msg[2] = 0x2A;

		/* Version2 */
		WscAppendV2SubItem(WFA_EXT_ID_VERSION2, &pReg->SelfInfo.Version2, 1, pWscV2Msg+3, &TmpLen);
		WscV2MsgLen += (3 + (USHORT)TmpLen);

		if (bSelRegistrar)
		{
			/* 
				Authorized MACs 
				Registrars (both internal and external) shall add the AuthorizedMACs subelement, 
				this applies to all Configuration Methods, including PIN, PBC and NFC.
			*/
			if (pAuthorizedMACs)
			{
				WscAppendV2SubItem(	WFA_EXT_ID_AUTHORIZEDMACS, 
									pAuthorizedMACs, 
									AuthorizedMACsLen, 
									pWscV2Msg+WscV2MsgLen, 
									&TmpLen	);
				WscV2MsgLen += (USHORT)TmpLen;
			}
			else
			{
				WscAppendV2SubItem(	WFA_EXT_ID_AUTHORIZEDMACS, 
									BROADCAST_ADDR, 
									MAC_ADDR_LEN, 
									pWscV2Msg+WscV2MsgLen, 
									&TmpLen	);
				WscV2MsgLen += (USHORT)TmpLen;
			}
		}

		templen = AppendWSCTLV(WSC_ID_VENDOR_EXT, (*pOutBuf), (UINT8 *)pWscV2Msg, WscV2MsgLen);
		(*pOutBuf) += templen;
		(*pOutBufLen)   += templen;

		os_free_mem(NULL, pWscV2Msg);
		
		return TRUE;
	}
	return FALSE;
}

BOOLEAN	WscAppendV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	UCHAR			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen)
{
	PUCHAR	pBuf = NULL;
	os_alloc_mem(NULL, &pBuf, DataLen + 10);
	if (pBuf)
	{
		pBuf[0] = SubID;
		pBuf[1] = DataLen;
		NdisMoveMemory(pBuf+2, pData, DataLen);
		*pOutBufLen = (DataLen + 2);
		NdisMoveMemory(pOutBuf, pBuf, *pOutBufLen);
		os_free_mem(NULL, pBuf);
		return TRUE;
	}
	return FALSE;
}

BOOLEAN	WscParseV2SubItem(
	IN	UCHAR			SubID,
	IN	PUCHAR			pData,
	IN	USHORT			DataLen,
	OUT	PUCHAR			pOutBuf,
	OUT	PUCHAR			pOutBufLen)
{
	PEID_STRUCT   pEid;
	USHORT		  Length = 0;
	pEid = (PEID_STRUCT) (pData + 3);
	hex_dump("WscParseV2SubItem - pData", (pData+3), DataLen-3);
	while ((Length + 2 + pEid->Len) <= (DataLen-3))
    {
    	if (pEid->Eid == SubID)
        {
			*pOutBufLen = pEid->Len;
			NdisMoveMemory(pOutBuf, &pEid->Octet[0], pEid->Len);
			return TRUE;
    	}
    	Length = Length + 2 + pEid->Len; 
        pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);
	}
	return FALSE;
}

VOID	WscSendEapFragAck(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry)
{
	if (pEntry == NULL)
	{
		ASSERT(pEntry!=NULL);
		return;
	}
	if (IS_ENTRY_CLIENT(pEntry))
	{
		pWscControl->bWscLastOne = TRUE;
		if (pAdapter->OpMode == OPMODE_AP)
			WscSendMessage(pAdapter, WSC_OPCODE_FRAG_ACK, NULL, 0, pWscControl, AP_MODE, EAP_CODE_REQ);
		else
		{						
			if (ADHOC_ON(pAdapter) && (pWscControl->WscConfMode == WSC_REGISTRAR))
				WscSendMessage(pAdapter, WSC_OPCODE_FRAG_ACK, NULL, 0, pWscControl, STA_MODE, EAP_CODE_REQ);
			else
				WscSendMessage(pAdapter, WSC_OPCODE_FRAG_ACK, NULL, 0, pWscControl, STA_MODE, EAP_CODE_RSP);
		}
	}
	else if (IS_ENTRY_APCLI(pEntry))
		WscSendMessage(pAdapter, WSC_OPCODE_FRAG_ACK, NULL, 0, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
}

VOID	WscSendEapFragData(
	IN	PRTMP_ADAPTER		pAdapter,
	IN  PWSC_CTRL			pWscControl,
	IN	PMAC_TABLE_ENTRY	pEntry)
{
	INT 	DataLen = 0;
	PUCHAR	pData = NULL;
	
	if (pEntry == NULL)
	{
		ASSERT(pEntry!=NULL);
		return;
	}

	pData = pWscControl->pWscCurBufIdx;
	pWscControl->bWscLastOne = TRUE;
	if (pWscControl->WscTxBufLen > pWscControl->WscFragSize)
	{
		pWscControl->bWscLastOne = FALSE;
		DataLen = pWscControl->WscFragSize;
		pWscControl->WscTxBufLen -= pWscControl->WscFragSize;
		pWscControl->pWscCurBufIdx = (pWscControl->pWscCurBufIdx + pWscControl->WscFragSize);
	}
	else
	{
		DataLen = pWscControl->WscTxBufLen;
		pWscControl->pWscCurBufIdx = NULL;
		pWscControl->WscTxBufLen = 0;
	}
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAdapter)
	{
		if (IS_ENTRY_APCLI(pEntry))
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, pData, DataLen, pWscControl, AP_CLIENT_MODE, EAP_CODE_RSP);
		else
			WscSendMessage(pAdapter, WSC_OPCODE_MSG, pData, DataLen, pWscControl, AP_MODE, EAP_CODE_REQ);
	}
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAdapter)
		WscSendMessage(pAdapter, WSC_OPCODE_MSG, pData, DataLen, pWscControl, STA_MODE, EAP_CODE_RSP);
#endif // CONFIG_STA_SUPPORT //
}

#endif /* WSC_V2_SUPPORT */

