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


#include "rt_config.h"
#include "wfd_cmm.h"

#ifdef WFD_SUPPORT

UCHAR WIFIDISPLAY_OUI[] = {0x50, 0x6f, 0x9a, 0x0a};

INT Set_WfdEnable_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	BOOLEAN bEnable;

	bEnable = simple_strtol(arg, 0, 10);

	if (bEnable == TRUE)
	{
		pAd->StaCfg.WfdCfg.bWfdEnable= TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("%s:: Enable WFD Support!\n", __FUNCTION__));
	}
	else
	{
		pAd->StaCfg.WfdCfg.bWfdEnable= FALSE;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Disable WFD Support!\n", __FUNCTION__));
	}

	return TRUE;
}

INT Set_WfdDeviceType_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	UCHAR DeviceType;

	DeviceType = simple_strtol(arg, 0, 10);

	if (DeviceType <= WFD_SOURCE_PRIMARY_SINK)
	{
		pAd->StaCfg.WfdCfg.DeviceType = DeviceType;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Device Type Not Support!!\n", __FUNCTION__));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Device Type = %d\n", __FUNCTION__, pAd->StaCfg.WfdCfg.DeviceType));

	return TRUE;
}


INT Set_WfdCouple_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	UCHAR coupled;

	if (simple_strtol(arg, 0, 10) == 0)
		coupled = WFD_COUPLED_NOT_SUPPORT;
	else if (simple_strtol(arg, 0, 10) == 1)
		coupled = WFD_COUPLED_SUPPORT;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Coupled out of range!!\n", __FUNCTION__));
		return FALSE;
	}

	switch (pAd->StaCfg.WfdCfg.DeviceType)
	{
		case WFD_SOURCE:
			pAd->StaCfg.WfdCfg.SourceCoupled = coupled;
			break;
		case WFD_PRIMARY_SINK:
		case WFD_SECONDARY_SINK:
			pAd->StaCfg.WfdCfg.SinkCoupled = coupled;
			break;
		case WFD_SOURCE_PRIMARY_SINK:
			pAd->StaCfg.WfdCfg.SourceCoupled = coupled;
			pAd->StaCfg.WfdCfg.SinkCoupled = coupled;
			break;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Device Type = %d, Source Coupled = %d, Sink Coupled = %d\n", __FUNCTION__, 
		pAd->StaCfg.WfdCfg.DeviceType, pAd->StaCfg.WfdCfg.SourceCoupled, pAd->StaCfg.WfdCfg.SinkCoupled));

	return TRUE;
}

INT Set_WfdSessionAvailable_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
		pAd->StaCfg.WfdCfg.SessionAvail= WFD_SESSION_NOT_AVAILABLE;
	else if (simple_strtol(arg, 0, 10) == 1)
		pAd->StaCfg.WfdCfg.SessionAvail = WFD_SESSION_AVAILABLE;
	else
	{
		pAd->StaCfg.WfdCfg.SessionAvail = WFD_SESSION_NOT_AVAILABLE;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Session Available out of range, using default\n", __FUNCTION__, pAd->StaCfg.WfdCfg.SessionAvail));
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Session Available = %d\n", __FUNCTION__, pAd->StaCfg.WfdCfg.SessionAvail));
	
	return TRUE;
}

INT Set_WfdCP_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	if (simple_strtol(arg, 0, 10) == 0)
		pAd->StaCfg.WfdCfg.CP = WFD_CP_NOT_SUPPORT;
	else if (simple_strtol(arg, 0, 10) == 1)
		pAd->StaCfg.WfdCfg.CP = WFD_CP_HDCP20;
	else
	{
		pAd->StaCfg.WfdCfg.CP = WFD_CP_NOT_SUPPORT;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Content Protection out of range, using default\n", __FUNCTION__, pAd->StaCfg.WfdCfg.CP));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Content Protection = %d\n", __FUNCTION__, pAd->StaCfg.WfdCfg.CP));

	return TRUE;
}


INT	Set_WfdRtspPort_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	INT32 RtspPort;
	
	RtspPort = simple_strtol(arg, 0, 10);
	
	if ((RtspPort < 0) || (65535 < RtspPort))
	{
		pAd->StaCfg.WfdCfg.RtspPort = WFD_RTSP_DEFAULT_PORT;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: RTSP Port out of range, using default\n", __FUNCTION__, pAd->StaCfg.WfdCfg.RtspPort));
	}
	else
		pAd->StaCfg.WfdCfg.RtspPort = RtspPort;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: RTSP Port = %d\n", __FUNCTION__, pAd->StaCfg.WfdCfg.RtspPort));

	return TRUE;
}


INT	Set_WfdMaxThroughput_Proc(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PSTRING			arg)
{
	INT32 Throughput;
	
	Throughput = simple_strtol(arg, 0, 10);

	if ((Throughput <= 0)|| (65535 < Throughput))
	{
		pAd->StaCfg.WfdCfg.MaxThroughput = WFD_MAX_THROUGHPUT_DEFAULT;
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: Max Throughput out of range, using default\n", __FUNCTION__, pAd->StaCfg.WfdCfg.MaxThroughput));
	}
	else
		pAd->StaCfg.WfdCfg.MaxThroughput = Throughput;

	DBGPRINT(RT_DEBUG_TRACE, ("%s:: Max Throughput = %d\n", __FUNCTION__, pAd->StaCfg.WfdCfg.MaxThroughput));

	return TRUE;
}

INT Set_WfdLocalIp_Proc(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PSTRING 		arg)
{
	PRT_WFD_CONFIG pWFDCtrl = &pAd->StaCfg.WfdCfg;
	UINT32 ip_addr;

	rtinet_aton(arg, &ip_addr);
	printk("IP = %04x\n", ip_addr);
	pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[0] = WFD_LOCAL_IP_ADDR_VERSION_IPV4;
	RTMPMoveMemory(&pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[1], &ip_addr, sizeof(UINT32));
	DBGPRINT(RT_DEBUG_TRACE, ("%s:: local IP Address = %d.%d.%d.%d\n", __FUNCTION__,
			pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[1], 
			pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[2],
			pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[3],
			pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie[4]));

	return TRUE;
}

INT Set_PeerRtspPort_Proc(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PSTRING 		arg)
{
	PRT_WFD_CONFIG pWFDCtrl = &pAd->StaCfg.WfdCfg;
	UINT32 ip_addr;

	MAC_TABLE_ENTRY *pEntry;
	USHORT RtspPort = WFD_RTSP_DEFAULT_PORT;
	UCHAR P2pIdx = P2P_NOT_FOUND;
	PRT_P2P_CONFIG pP2PCtrl = &pAd->P2pCfg;
	INT i;

#ifdef DOT11Z_TDLS_SUPPORT
	i = -1;
	if (pAd->StaCfg.TdlsInfo.bTDLSCapable && pAd->StaCfg.WfdCfg.PC == WFD_PC_TDLS)
	{
		PRT_802_11_TDLS pTDLS = NULL;
		DBGPRINT(RT_DEBUG_TRACE, ("%s - TDLS peer rtsp port get...\n", __FUNCTION__));
		for (i = MAX_NUM_OF_TDLS_ENTRY - 1; i >= 0; i--)
		{
			if ((pAd->StaCfg.TdlsInfo.TDLSEntry[i].Valid) && (pAd->StaCfg.TdlsInfo.TDLSEntry[i].Status == TDLS_MODE_CONNECTED))
			{
				pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[i];
				RtspPort = pTDLS->WfdEntryInfo.rtsp_port;
				DBGPRINT(RT_DEBUG_TRACE, ("TDLS Entry[%d][%02x:%02x:%02x:%02x:%02x:%02x]\n", i, PRINT_MAC(pTDLS->MacAddr)));
				DBGPRINT(RT_DEBUG_TRACE, ("RTSP_PORT = %d.\n", pTDLS->WfdEntryInfo.rtsp_port));
				break;
			}
		}

		if ((RtspPort == 0) && (pTDLS != NULL))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("TDLS peer rtsp port is zero, search P2P Entry!\n", RtspPort));

			P2pIdx = P2pGroupTabSearch(pAd, pTDLS->MacAddr);
			if (P2pIdx != P2P_NOT_FOUND)
			{
				RtspPort = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.rtsp_port; 
				DBGPRINT(RT_DEBUG_TRACE, ("P2P Entry[%d][%02x:%02x:%02x:%02x:%02x:%02x]\n", P2pIdx, PRINT_MAC(pTDLS->MacAddr)));
				DBGPRINT(RT_DEBUG_TRACE, ("RTSP_PORT = %d.\n", pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.rtsp_port));
				if (RtspPort == 0)
					RtspPort = WFD_RTSP_DEFAULT_PORT;
			}
			else
			{
				RtspPort = WFD_RTSP_DEFAULT_PORT;
				DBGPRINT(RT_DEBUG_ERROR, ("OID_802_11_P2P_PEER_RTSP_PORT::P2P not found, use default RTSP port\n"));
			}
		}
	}
	if (i < 0)
#endif /* DOT11Z_TDLS_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s - P2P peer rtsp port get...\n", __FUNCTION__));
		if (P2P_GO_ON(pAd) || P2P_CLI_ON(pAd))
		{
			for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
			{
				pEntry = &pAd->MacTab.Content[i];
				if (IS_P2P_GO_ENTRY(pEntry) || IS_P2P_CLI_ENTRY(pEntry))
				{
					P2pIdx = P2pGroupTabSearch(pAd, pEntry->Addr);
					DBGPRINT(RT_DEBUG_TRACE, ("P2P Entry[%d][%02x:%02x:%02x:%02x:%02x:%02x]\n", pEntry->P2pInfo.p2pIndex, PRINT_MAC(pEntry->Addr)));
					DBGPRINT(RT_DEBUG_TRACE, ("RTSP_PORT = %d.\n", pAd->P2pTable.Client[pEntry->P2pInfo.p2pIndex].WfdEntryInfo.rtsp_port));
					if (P2pIdx != P2P_NOT_FOUND)
						RtspPort = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.rtsp_port;	
					else
					{
						RtspPort = WFD_RTSP_DEFAULT_PORT;
						DBGPRINT(RT_DEBUG_ERROR, ("OID_802_11_P2P_PEER_RTSP_PORT::P2P not found, use default RTSP port\n"));
					}
					if (pEntry->P2pInfo.p2pIndex < MAX_P2P_GROUP_SIZE)
						P2PPrintP2PEntry(pAd, pEntry->P2pInfo.p2pIndex);
					break;
				}
			}
			DBGPRINT(RT_DEBUG_TRACE, ("OID_802_11_P2P_PEER_RTSP_PORT bssid: %02x:%02x:%02x:%02x:%02x:%02x.\n", PRINT_MAC(pP2PCtrl->CurrentAddress)));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Query::OID_802_11_P2P_PEER_RTSP_PORT (=%d)\n", RtspPort));

	return TRUE;
}


VOID WfdMakeWfdIE(
	IN	PRTMP_ADAPTER	pAd,
	IN 	UCHAR			PacketType,
	OUT	PUCHAR			pOutBuf,
	OUT	PULONG			pIeLen)
{
	PRT_WFD_CONFIG	pWFDCtrl = &pAd->StaCfg.WfdCfg;
	UCHAR			WfdIEFixed[6] = {0xdd, 0x0c, 0x50, 0x6f, 0x9a, 0x0a};	 /* Length will be modified later */
	PUCHAR			pData, pBuf;
	ULONG			TempLen;
	ULONG			Len = 0;
	INT 				i = 0;

	pData = pOutBuf;
	*pIeLen = 0;

	if (!pWFDCtrl->bWfdEnable)
		return;

	RTMPMoveMemory(pData, &WfdIEFixed[0], 6);
	pData += 6;
	Len += 6;

	/* To append to WFD Device Information Subelement */
	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_DEVICE_INFO, NULL, pData, ACTION_WIFI_DIRECT);
	DBGPRINT(RT_DEBUG_INFO, ("%s(%d) ---->\n", __FUNCTION__, TempLen));
	for (i=0; i<TempLen; i++)
		DBGPRINT(RT_DEBUG_INFO, ("%02x ", *(pData+i)));
	DBGPRINT(RT_DEBUG_INFO, ("\n"));
	
	Len += TempLen;
	pData += TempLen;

	/* To append to WFD Associated Bssid Subelement */
	TempLen = 0;
	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_ASSOCIATED_BSSID, NULL, pData, ACTION_WIFI_DIRECT);
	DBGPRINT(RT_DEBUG_INFO, ("%s(%d) ---->\n", __FUNCTION__, TempLen));
	for (i=0; i<TempLen; i++)
		DBGPRINT(RT_DEBUG_INFO, ("%02x ", *(pData+i)));
	DBGPRINT(RT_DEBUG_INFO, ("\n"));
	
	Len += TempLen;
	pData += TempLen;	

	/* To append to WFD Coupled Sink Information Subelement */
	TempLen = 0;
	TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_COUPLED_SINK_INFO, NULL, pData, ACTION_WIFI_DIRECT);
	DBGPRINT(RT_DEBUG_INFO, ("%s(%d) ---->\n", __FUNCTION__, TempLen));
	for (i=0; i<TempLen; i++)
		DBGPRINT(RT_DEBUG_INFO, ("%02x ", *(pData+i)));
	DBGPRINT(RT_DEBUG_INFO, ("\n"));
	
	Len += TempLen;
	pData += TempLen;

#ifdef DOT11Z_TDLS_SUPPORT
	/* TDLS : TO DO */
	if ((PacketType == TDLS_ACTION_CODE_SETUP_REQUEST) ||
		(PacketType == TDLS_ACTION_CODE_SETUP_RESPONSE) ||
		(PacketType == TDLS_ACTION_CODE_SETUP_CONFIRM))
	{
		TempLen = 0;
		TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_LOCAL_IP_ADDR, NULL, pData, ACTION_WIFI_DIRECT);
		Len += TempLen;
		pData += TempLen;
	}
#endif /* DOT11Z_TDLS_SUPPORT */
	if (P2P_GO_ON(pAd) && 
		((PacketType == SUBTYPE_BEACON) ||
		(PacketType == SUBTYPE_PROBE_RSP) ||
		(PacketType == SUBTYPE_ASSOC_RSP)))
	{
		/* To append to WFD Session Information Subelement */
		TempLen = 0;
		TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_SESSION_INFO, NULL, pData, ACTION_WIFI_DIRECT);
		DBGPRINT(RT_DEBUG_INFO, ("%s(%d) ---->\n", __FUNCTION__, TempLen));
		for (i=0; i<TempLen; i++)
			DBGPRINT(RT_DEBUG_INFO, ("%02x ", *(pData+i)));
		DBGPRINT(RT_DEBUG_INFO, ("\n"));
	
		Len += TempLen;
		pData += TempLen;

		if (PacketType == SUBTYPE_PROBE_RSP)
		{
			TempLen = 0;
			TempLen = InsertWfdSubelmtTlv(pAd, SUBID_WFD_ALTERNATE_MAC_ADDR, NULL, pData, ACTION_WIFI_DIRECT);
			Len += TempLen;
			pData += TempLen;
		}
	}

	*(pOutBuf+1) = (Len-2);
	*pIeLen = Len;
	
	return;
}


ULONG InsertWfdSubelmtTlv(
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR			SubId,
	IN PUCHAR			pInBuffer,
	IN PUCHAR			pOutBuffer,
	IN UINT				Action)
{
	PRT_WFD_CONFIG	pWFDCtrl = &pAd->StaCfg.WfdCfg;
	PUCHAR	pDest;
	ULONG	Length, tmpValue = 0;
	USHORT	EidLen = 0;

	pDest = pOutBuffer;
	RTMPZeroMemory(pDest, 255);
	*pDest = SubId;
	pDest += 1;
	Length = 0;

	switch (SubId)
	{
		case SUBID_WFD_DEVICE_INFO:
		{
			WFD_DEVICE_INFO DevInfo;
			PUSHORT pDevInfo = &DevInfo;

			RTMPZeroMemory(&DevInfo, sizeof(WFD_DEVICE_INFO));

			EidLen = SUBID_WFD_DEVICE_INFO_LEN;
			tmpValue = cpu2be16(EidLen);
			RTMPMoveMemory(pDest, &tmpValue, 2);
			DevInfo.DeviceType = pWFDCtrl->DeviceType;
			DevInfo.SourceCoupled = pWFDCtrl->SourceCoupled;
			DevInfo.SinkCoupled = pWFDCtrl->SinkCoupled;
			DevInfo.SessionAvail = pWFDCtrl->SessionAvail;
			DevInfo.WSD = pWFDCtrl->WSD;
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
				DevInfo.PC = pWFDCtrl->PC;
			else
				DevInfo.PC = WFD_PC_P2P;
			DevInfo.CP = pWFDCtrl->CP;
			DevInfo.TimeSync = pWFDCtrl->TimeSync;
			/* RTMPMoveMemory(pDest + 1, &DevInfo, sizeof(WFD_DEVICE_INFO)); */
			tmpValue = cpu2be16(*pDevInfo);
			RTMPMoveMemory((pDest + 2), &tmpValue, 2);
			tmpValue = cpu2be16(pWFDCtrl->RtspPort);
			RTMPMoveMemory((pDest + 4), &tmpValue, 2);
			tmpValue = cpu2be16(pWFDCtrl->MaxThroughput);
			RTMPMoveMemory((pDest + 6), &tmpValue, 2);
			Length = 9;
			break;
		}
		case SUBID_WFD_ASSOCIATED_BSSID:
		{
			UCHAR AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_ASSOCIATED_BSSID_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
				if (!NdisEqualMemory(AllZero, pAd->CommonCfg.Bssid, MAC_ADDR_LEN) &&
					(Action == ACTION_GAS_INITIAL_RSP))
				{
					RTMPMoveMemory(pDest + 2, pAd->CommonCfg.Bssid, MAC_ADDR_LEN);
				}
			}
			else
			{
				if (!NdisEqualMemory(AllZero, pAd->CommonCfg.Bssid, MAC_ADDR_LEN))
				{
					EidLen = SUBID_WFD_ASSOCIATED_BSSID_LEN;
					tmpValue = cpu2be16(EidLen);
					RTMPMoveMemory(pDest, &tmpValue, 2);
					RTMPMoveMemory(pDest + 2, pAd->CommonCfg.Bssid, MAC_ADDR_LEN);
					Length = EidLen + 3;
				}
			}
			break;
		}
		case SUBID_WFD_AUDIO_FORMATS:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_AUDIO_FORMATS_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_VIDEO_FORMATS:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_VIDEO_FORMATS_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_3D_VIDEO_FORMATS:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_3D_VIDEO_FORMATS_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_CONTENT_PROTECTION:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_CONTENT_PROTECTION_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_COUPLED_SINK_INFO:
		{
//			if ((pWFDCtrl->DeviceType != WFD_SOURCE ) && (pWFDCtrl->SinkCoupled == WFD_COUPLED_SUPPORT))
			{
				WFD_COUPLED_SINK_INFO SinkInfo;

				RTMPZeroMemory(&SinkInfo, sizeof(WFD_COUPLED_SINK_INFO));
				EidLen = SUBID_WFD_COUPLED_SINK_INFO_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				SinkInfo.CoupledStat = pWFDCtrl->CoupledSinkStatus.CoupledStat;
				RTMPMoveMemory(pDest + 2, &SinkInfo, sizeof(WFD_COUPLED_SINK_INFO));
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_EXTENDED_CAP:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_EXTENDED_CAP_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_LOCAL_IP_ADDR:
		{
			if ((Action == ACTION_GAS_INITIAL_REQ) || (Action == ACTION_GAS_INITIAL_RSP))
			{
				EidLen = SUBID_WFD_LOCAL_IP_ADDR_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				Length = EidLen + 3;
			}
			else
			{
				EidLen = SUBID_WFD_LOCAL_IP_ADDR_LEN;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				RTMPMoveMemory(pDest + 2, &pWFDCtrl->wfd_serv_disc_query_info.wfd_local_ip_ie, SUBID_WFD_LOCAL_IP_ADDR_LEN);
				Length = EidLen + 3;
			}
			break;
		}
		case SUBID_WFD_SESSION_INFO:
		{
			INT i = 0, NumOfDev = 0; 
			UCHAR P2pIdx = P2P_NOT_FOUND;
			PRT_P2P_TABLE Tab = &pAd->P2pTable;

			if (Action == ACTION_WIFI_DIRECT)
			{
				for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
				{
					MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
					P2pIdx = P2pGroupTabSearch(pAd, pEntry->Addr);

					if ((P2pIdx < MAX_P2P_GROUP_SIZE) && (Tab->Client[P2pIdx].WfdEntryInfo.bWfdClient == TRUE))
						NumOfDev++;
				}

				EidLen = 24*NumOfDev;
				tmpValue = cpu2be16(EidLen);
				RTMPMoveMemory(pDest, &tmpValue, 2);
				DBGPRINT(RT_DEBUG_INFO, ("%s:: NumOfDev = %d, Len = %d\n", __FUNCTION__, NumOfDev, *pDest));

				pDest+=2;
				for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
				{
					MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
					P2pIdx = P2pGroupTabSearch(pAd, pEntry->Addr);

					if ((P2pIdx < MAX_P2P_GROUP_SIZE) && (Tab->Client[P2pIdx].WfdEntryInfo.bWfdClient == TRUE))
					{
						INT j = 0;
						WFD_SESSION_INFO SessionInfo;

						RTMPZeroMemory(&SessionInfo, sizeof(WFD_SESSION_INFO));

						SessionInfo.Length = 23;
						RTMPMoveMemory(&SessionInfo.DeviceAddr[0], &pAd->P2pTable.Client[P2pIdx].addr[0], MAC_ADDR_LEN);
						RTMPMoveMemory(&SessionInfo.Bssid[0], &pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.assoc_addr[0], MAC_ADDR_LEN);
						/*  Below is the WFD Device Information */
						SessionInfo.WfdDevInfo.DeviceType = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.wfd_devive_type;
						SessionInfo.WfdDevInfo.SourceCoupled = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.source_coupled;
						SessionInfo.WfdDevInfo.SinkCoupled = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.sink_coupled;
						SessionInfo.WfdDevInfo.SessionAvail = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.session_avail;
						SessionInfo.WfdDevInfo.WSD = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.wfd_service_discovery;
						SessionInfo.WfdDevInfo.PC = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.wfd_PC;
						SessionInfo.WfdDevInfo.TimeSync = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.wfd_time_sync;
						SessionInfo.MaxThroughput = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.max_throughput;
						SessionInfo.CoupledSinkInfo = pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.coupled_sink_status;

						/* 
							So far we cannot know the address of coupled devices, 
						   	the coupled address will be filled "0" until WiFi Display spec. is ready for this part. 
						*/
						RTMPMoveMemory(&SessionInfo.CoupledPeerAddr[0], &pAd->P2pTable.Client[P2pIdx].WfdEntryInfo.coupled_peer_addr[0], MAC_ADDR_LEN);
						RTMPMoveMemory(pDest, &SessionInfo, sizeof(WFD_SESSION_INFO));

						for (j = 0; j < 24; j++)
							DBGPRINT(RT_DEBUG_INFO, ("%02x ", *(pDest+j)));
						DBGPRINT(RT_DEBUG_INFO, ("\n"));

						pDest += 24;
					}
				}			

				Length = 24*NumOfDev + 3;
			}
			break;
		}
		case SUBID_WFD_ALTERNATE_MAC_ADDR:
		{
			UCHAR AllZero[MAC_ADDR_LEN] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

			if ((Action == ACTION_GAS_INITIAL_REQ) ||
				(Action == ACTION_GAS_INITIAL_RSP) ||
				(Action == ACTION_WIFI_DIRECT))
			{
				EidLen = SUBID_WFD_ALTERNATE_MAC_ADDR_LEN;
				*((PUSHORT) (pDest)) = cpu2be16(EidLen);
				Length = EidLen + 3;
				if (!NdisEqualMemory(AllZero, pAd->CurrentAddress, MAC_ADDR_LEN) &&
					(Action == ACTION_GAS_INITIAL_RSP))
				{
					RTMPMoveMemory(pDest + 2, pAd->CurrentAddress, MAC_ADDR_LEN);
				}
			}
			else
			{
				if (!NdisEqualMemory(AllZero, pAd->CurrentAddress, MAC_ADDR_LEN))
				{
					EidLen = SUBID_WFD_ALTERNATE_MAC_ADDR_LEN;
					*((PUSHORT) (pDest)) = cpu2be16(EidLen);
					RTMPMoveMemory(pDest + 2, pAd->CurrentAddress, MAC_ADDR_LEN);
					Length = EidLen + 3;
				}
			}
			break;
		}
		default:
			*pDest = 0;
			Length = 0;
			break;
	}

	return Length;
}

VOID WfdParseSubElmt(
	IN PRTMP_ADAPTER 	pAd, 
	IN PWFD_ENTRY_INFO	pWfdEntryInfo,
	IN VOID 			*Msg, 
	IN ULONG 			MsgLen)
{
	PWFD_COUPLED_SINK_INFO pSinkInfo;
	PWFD_DEVICE_INFO pWfd_info;
	WFD_DEVICE_INFO DevInfo;
	PP2PEID_STRUCT pWfdEid;
	PEID_STRUCT	pEid;
	PUCHAR 		pWfdIe = NULL;
	ULONG		AccuWfdIELen;
	ULONG		AccuIeLen = 0;
	ULONG		Length = 0;
	ULONG		AttriLen;
	UCHAR		offset;
	BOOLEAN		bTdlsEntry = FALSE;

	DBGPRINT(RT_DEBUG_INFO, ("%s ----->\n", __FUNCTION__));

//QQ TBD, p2p widi need to parse rtsp port!
	{
		if ((!pAd->StaCfg.WfdCfg.bWfdEnable) || (MsgLen == 0))
			return;
	}

//	hex_dump("WfdParseSubElmt::", Msg, MsgLen);
	pEid = (PEID_STRUCT)Msg;
	AccuIeLen = pEid->Len + 2;
//	printk("MsgLen = %d. AccuIeLen = %d.\n", MsgLen, AccuIeLen);
	while ((ULONG)(AccuIeLen) <= MsgLen)
	{
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDISPLAY_OUI, 4))
		{
			/* Get Request content capability */
			pWfdIe = pWfdEid = (PP2PEID_STRUCT) &pEid->Octet[4];
			AccuWfdIELen = pEid->Len;
//			printk("AccuWfdIeLen = %d. EidLen = %04x\n", AccuWfdIELen, pEid->Len);
			/* The value of AccuP2PIELen shall reduce the length of OUI (4) */
			AccuWfdIELen -= 4;
			
			AttriLen = pWfdEid->Len[1] + (pWfdEid->Len[0] << 8);
			Length = 0;
//			printk("AttriLen = %d.  WfdEid = %d.  WfdEidLen = %x %x\n", AttriLen, pWfdEid->Eid, pWfdEid->Len[1], pWfdEid->Len[0]);
			pWfdEntryInfo->bWfdClient = TRUE; /* Set the P2P client as the WFD device */

//			while (Length <=(Length + 3 + AttriLen) <= AccuWfdIELen)
			while (Length <= AccuWfdIELen)
			{
//				printk(">> Eid = %d.\n", pWfdEid->Eid);
				switch (pWfdEid->Eid)
				{						
					case SUBID_WFD_DEVICE_INFO:
					{
						pWfd_info = &(pWfdEid->Octet[0]);
						RTMPMoveMemory(&DevInfo, pWfdIe, sizeof(WFD_DEVICE_INFO));
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_device_info_ie, pWfdEid->Octet, SUBID_WFD_DEVICE_INFO_LEN);
						cpu2le16(&DevInfo);

						pWfdEntryInfo->wfd_devive_type = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 0) & 0x3);
						pWfdEntryInfo->source_coupled = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 2) & 0x1);
						pWfdEntryInfo->sink_coupled = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 3) & 0x1);
						pWfdEntryInfo->session_avail = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 4) & 0x3);
						pWfdEntryInfo->wfd_service_discovery = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 6) & 0x1);
						pWfdEntryInfo->wfd_PC = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 7) & 0x1);
						pWfdEntryInfo->wfd_CP = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 8) & 0x1);
						pWfdEntryInfo->wfd_time_sync = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 9) & 0x1);
						pWfdEntryInfo->sink_audio_unsupport = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 10) & 0x1);
						pWfdEntryInfo->source_audio_only= ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 11) & 0x1);
						pWfdEntryInfo->tdls_persistent_group = ((be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[0]))) >> 12) & 0x1);
						pWfdEntryInfo->rtsp_port = be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[2])));
						pWfdEntryInfo->max_throughput = be2cpu16(get_unaligned((PUSHORT)(&pWfdEid->Octet[4])));

						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_DEVICE_INFO\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_ASSOCIATED_BSSID:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_associate_bssid_ie, pWfdEid->Octet, SUBID_WFD_ASSOCIATED_BSSID_LEN);
						RTMPMoveMemory(&pWfdEntryInfo->assoc_addr, pWfdEid->Octet, MAC_ADDR_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_ASSOCIATED_BSSID\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_AUDIO_FORMATS:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_audio_format_ie, pWfdEid->Octet, SUBID_WFD_AUDIO_FORMATS_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_AUDIO_FORMATS\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_VIDEO_FORMATS:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_video_format_ie, pWfdEid->Octet, SUBID_WFD_VIDEO_FORMATS_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_VIDEO_FORMATS\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_3D_VIDEO_FORMATS:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_3d_video_format_ie, pWfdEid->Octet, SUBID_WFD_3D_VIDEO_FORMATS_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_3D_VIDEO_FORMATS\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_CONTENT_PROTECTION:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_content_proctection, pWfdEid->Octet, SUBID_WFD_CONTENT_PROTECTION_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_CONTENT_PROTECTION\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_COUPLED_SINK_INFO:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_couple_sink_info_ie, pWfdEid->Octet, SUBID_WFD_COUPLED_SINK_INFO_LEN);
						RTMPMoveMemory(&pWfdEntryInfo->coupled_sink_status, pWfdEid->Octet, SUBID_WFD_COUPLED_SINK_INFO_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_COUPLED_SINK_INFO\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_EXTENDED_CAP:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_extent_capability_ie, &pWfdEid->Octet, SUBID_WFD_EXTENDED_CAP_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_EXTENDED_CAP\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_LOCAL_IP_ADDR:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_local_ip_ie, &pWfdEid->Octet, SUBID_WFD_LOCAL_IP_ADDR_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_LOCAL_IP_ADDR\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_SESSION_INFO:
					{
						/* TODO : allocate memory to store the parsed WFD device tables */
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_session_info_ie, &pWfdEid->Octet, SUBID_WFD_DEVICE_INFO_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_SESSION_INFO\n", __FUNCTION__));
						break;
					}
					case SUBID_WFD_ALTERNATE_MAC_ADDR:
					{
						RTMPMoveMemory(&pWfdEntryInfo->wfd_serv_disc_query_info.wfd_alternate_mac_addr_ie, &pWfdEid->Octet, SUBID_WFD_ALTERNATE_MAC_ADDR_LEN);
						DBGPRINT(RT_DEBUG_INFO, ("%s::SUBID_WFD_ALTERNATE_MAC_ADDR\n", __FUNCTION__));
						break;
					}
					default:
						DBGPRINT(RT_DEBUG_ERROR, (" SUBID_WFD_ unknown  Eid = %x \n", pWfdEid->Eid));
						hex_dump("WfdParseSubElement::", Msg, MsgLen);
						break;						
				}
//				printk("<< Length = %d. AttriLen = %d. AccuWfdIELen = %d.\n", Length, AttriLen, AccuWfdIELen);
				Length = Length + 3 + AttriLen;  /* Eid[1] + Len[2] + content[Len] */
//				printk(">> Length = %d. AttriLen = %d. AccuWfdIELen = %d.\n", Length, AttriLen, AccuWfdIELen);
				if (Length >= AccuWfdIELen)
					break;

				pWfdEid = (PP2PEID_STRUCT)((UCHAR*)pWfdEid + 3 + AttriLen);
				AttriLen = pWfdEid->Len[1] + (pWfdEid->Len[0] << 8);
			}

		}
		/* Already reach the final IE and stop finding next Eid. */
		if (AccuIeLen >= MsgLen)
			break;

		/* Forward buffer to next pEid */
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDISPLAY_OUI, 4))
		{
			pEid = (PEID_STRUCT)((UCHAR*)pEid + pEid->Len + 2);    
		}
		
		/* 
			Since we get the next pEid, 
		   	Predict the accumulated IeLen after adding the next pEid's length.
		   	The accumulated IeLen is for checking length.
		*/
		if (RTMPEqualMemory(&pEid->Octet[0], WIFIDISPLAY_OUI, 4))
		{
			AccuIeLen += (pEid->Len + 2);
		}
	}
	return;
}

VOID	WfdCfgInit(

	IN PRTMP_ADAPTER pAd) 
{
	PRT_WFD_CONFIG	pWfdcfg = &pAd->StaCfg.WfdCfg;

	RTMPZeroMemory(&pAd->StaCfg.WfdCfg, sizeof(RT_WFD_CONFIG));	
	pWfdcfg->bWfdEnable = FALSE;
	pWfdcfg->DeviceType = WFD_PRIMARY_SINK;
	pWfdcfg->SessionAvail = WFD_SESSION_AVAILABLE;
	pWfdcfg->PeerSessionAvail = WFD_SESSION_AVAILABLE;
	pWfdcfg->PeerPC = WFD_PC_TDLS;
	pWfdcfg->TdlsSecurity = WFD_TDLS_STRONG_SECURITY;
	pWfdcfg->RtspPort = WFD_RTSP_DEFAULT_PORT;
}
#endif /* WFD_SUPPORT */
