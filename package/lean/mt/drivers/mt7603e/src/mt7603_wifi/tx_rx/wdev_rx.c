/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"

#ifdef CONFIG_HOTSPOT
extern BOOLEAN hotspot_rx_handler(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, RX_BLK *pRxBlk);
#endif /* CONFIG_HOTSPOT */
#ifdef AIR_MONITOR
extern VOID Air_Monitor_Pkt_Report_Action(PRTMP_ADAPTER pAd, UCHAR wcid, RX_BLK *pRxBlk);
extern BOOLEAN IsValidUnicastToMe(IN PRTMP_ADAPTER pAd,
                          IN UCHAR WCID,
                          IN PUCHAR pDA);
#endif /* AIR_MONITOR */


#ifdef PREVENT_ARP_SPOOFING
INT check_arp_spoofing( PNET_DEV dev, PNDIS_PACKET pskb)
{
	struct net_bridge_port *br_port=NULL;
	struct in_device *in_dev;
	struct in_ifaddr *if_info;
	struct arphdr *arp;
	UCHAR *br_ip, *arp_ptr;
	UCHAR *pPkt;
	USHORT protocol;

	br_port = GET_BR_PORT(dev);	
	
	if (!br_port)
		return 0;

	//DBGPRINT(RT_DEBUG_TRACE, ("%s(): ==> \n", __FUNCTION__));
	
	in_dev = (struct in_device *)(br_port->br->dev->ip_ptr);
	if_info = (in_dev->ifa_list);
	br_ip = (UCHAR *)&if_info->ifa_local;

	pPkt = (GET_OS_PKT_DATAPTR(pskb) + (2 * ETH_ALEN));
	protocol = *(USHORT *)pPkt;

	if(protocol == OS_HTONS(ETH_P_ARP)) {
		pPkt = (GET_OS_PKT_DATAPTR(pskb) + ETH_HLEN);  // ETH_HLEN is 14
		arp = (struct arphdr *)pPkt;	
		
		arp_ptr = (UCHAR *)(arp + 1);

		if(arp->ar_pro == OS_HTONS(ETH_P_IP)) {
			arp_ptr += arp->ar_hln;
			if (!NdisCmpMemory(br_ip, arp_ptr, arp->ar_pln))
				return -1;
		}
	}

	return 0;
}
#endif /* PREVENT_ARP_SPOOFING */


// TODO: shiang-usw, temporary put this function here, should remove to other place or re-write!
VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN struct rx_signal_info *signal,
	IN UCHAR phy_mode,
	IN UCHAR bw)
{
	BOOLEAN bInitial = FALSE;
	INT ant_idx, ant_max = 3;

	if (!(pRssi->AvgRssi[0] | pRssi->AvgRssiX8[0] | pRssi->LastRssi[0]))
		bInitial = TRUE;

	// TODO: shiang-usw, shall we check this here to reduce the for loop count?
	if (ant_max > pAd->Antenna.field.RxPath)
		ant_max = pAd->Antenna.field.RxPath;

	for (ant_idx = 0; ant_idx < 3; ant_idx++)
	{
		if (signal->raw_snr[ant_idx] != 0 && phy_mode != MODE_CCK)
		{
			pRssi->LastSnr[ant_idx] = ConvertToSnr(pAd, signal->raw_snr[ant_idx]);
			if (bInitial)
			{
				pRssi->AvgSnrX8[ant_idx] = pRssi->LastSnr[ant_idx] << 3;
				pRssi->AvgSnr[ant_idx] = pRssi->LastSnr[ant_idx];
			}
			else
				pRssi->AvgSnrX8[ant_idx] = (pRssi->AvgSnrX8[ant_idx] - pRssi->AvgSnr[ant_idx]) + pRssi->LastSnr[ant_idx];

			pRssi->AvgSnr[ant_idx] = pRssi->AvgSnrX8[ant_idx] >> 3;
		}

		if (signal->raw_rssi[ant_idx] != 0)
		{
			pRssi->LastRssi[ant_idx] = ConvertToRssi(pAd, (struct raw_rssi_info *)(&signal->raw_rssi[0]), ant_idx);

			if (bInitial)
			{
				pRssi->AvgRssiX8[ant_idx] = pRssi->LastRssi[ant_idx] << 3;
				pRssi->AvgRssi[ant_idx] = pRssi->LastRssi[ant_idx];
			}
			else
				pRssi->AvgRssiX8[ant_idx] = (pRssi->AvgRssiX8[ant_idx] - pRssi->AvgRssi[ant_idx]) + pRssi->LastRssi[ant_idx];

			pRssi->AvgRssi[ant_idx] = pRssi->AvgRssiX8[ant_idx] >> 3;
		}
	}
}

/* this function ONLY if not allow pn replay attack and drop packet */
static BOOLEAN check_rx_pkt_pn_allowed(RTMP_ADAPTER *pAd, RX_BLK *rx_blk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN isAllow = TRUE;
	HEADER_802_11 *pHeader = rx_blk->pHeader;
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;

	if (pFmeCtrl->Wep == 0)
		return TRUE;

	if (rx_blk->wcid >= MAX_LEN_OF_MAC_TABLE)
		return TRUE;

	pEntry = &pAd->MacTab.Content[rx_blk->wcid];

	if ((!pEntry) || (!pEntry->wdev) || (!IS_ENTRY_APCLI(pEntry)))
		return TRUE;

	if (rx_blk->pRxInfo->Mcast || rx_blk->pRxInfo->Bcast) {
		NDIS_802_11_ENCRYPTION_STATUS GroupCipher;
		UCHAR kid = rx_blk->key_idx;

		WPA_GET_GROUP_CIPHER(pAd, pEntry, GroupCipher);

		if (GroupCipher == Ndis802_11AESEnable) {
			if (unlikely(kid >= ARRAY_SIZE(pEntry->CCMP_BC_PN))) {
				DBGPRINT(RT_DEBUG_TRACE, ("BC, %s invalid key id %u\n", __func__, kid));
				return TRUE;
			}

			if (unlikely(pEntry->Init_CCMP_BC_PN_Passed[kid] == FALSE)) {
				if (rx_blk->CCMP_PN >= pEntry->CCMP_BC_PN[kid]) {
					DBGPRINT(RT_DEBUG_TRACE,
						("BC, %s (%d)-%d OK: come-in the %llu and now is %llu\n",
						__func__, pEntry->wcid, kid, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[kid]));
					pEntry->CCMP_BC_PN[kid] = rx_blk->CCMP_PN;
					pEntry->Init_CCMP_BC_PN_Passed[kid] = TRUE;
				} else {
					DBGPRINT(RT_DEBUG_TRACE,
						("BC, %s (%d)-%d Reject: come-in the %llu and now is %llu\n",
						__func__, pEntry->wcid, kid, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[kid]));
					isAllow = FALSE;
				}
			} else {
				if (rx_blk->CCMP_PN > pEntry->CCMP_BC_PN[kid]) {
					DBGPRINT(RT_DEBUG_TRACE,
						("BC, %s (%d)-%d OK: come-in the %llu and now is %llu\n",
						__func__, pEntry->wcid, kid, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[kid]));
					pEntry->CCMP_BC_PN[kid] = rx_blk->CCMP_PN;
				} else {
					DBGPRINT(RT_DEBUG_TRACE,
						("BC, %s (%d)-%d Reject: come-in the %llu and now is %llu\n",
						__func__, pEntry->wcid, kid, rx_blk->CCMP_PN, pEntry->CCMP_BC_PN[kid]));
					isAllow = FALSE;
				}
			}
		}
	}

	return isAllow;
}

#ifdef DOT11_N_SUPPORT
UINT deaggregate_AMSDU_announce(
	IN RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	IN UCHAR *pData,
	IN ULONG DataSize,
	IN UCHAR OpMode)
{
	USHORT PayloadSize;
	USHORT SubFrameSize;
	HEADER_802_3 *pAMSDUsubheader;
	UINT nMSDU;
	UCHAR Header802_3[14];
	UCHAR *pPayload, *pDA, *pSA, *pRemovedLLCSNAP;
	PNDIS_PACKET pClonePacket;
	struct wifi_dev *wdev;
	UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	UCHAR VLAN_Size;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():invalud wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}

	wdev = pAd->wdev_list[wdev_idx];

	/* only MBssid support VLAN.*/
	VLAN_Size = (wdev->VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	nMSDU = 0;

	while (DataSize > LENGTH_802_3)
	{
		nMSDU++;

		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0]<<8);
		SubFrameSize = PayloadSize + LENGTH_802_3;

		if ((DataSize < SubFrameSize) || (PayloadSize > 1518 ))
			break;

		/*DBGPRINT(RT_DEBUG_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/
		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		/* convert to 802.3 header*/
		CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pRemovedLLCSNAP)
			{
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
				/*NdisMoveMemory(pPayload, &Header802_3, LENGTH_802_3);*/
			}
			else
			{
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);

			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									Header802_3, LENGTH_802_3, pPayload,
									TPID);
		}
#endif /* CONFIG_AP_SUPPORT */

		pClonePacket = ClonePacket(wdev->if_dev, pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
			UCHAR opmode = pAd->OpMode;

			Announce_or_Forward_802_3_Packet(pAd, pClonePacket, RTMP_GET_PACKET_WDEV(pPacket), opmode);
		}


		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

		if (DataSize > SubFrameSize)
		{
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		}
		else
		{
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}

	/* finally release original rx packet*/
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

	return nMSDU;
}


VOID Indicate_AMSDU_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	//UINT nMSDU;

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_AMSDU_Packet:drop packet by PN mismatch!\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, wdev_idx);
	RTMP_SET_PACKET_WDEV(pRxBlk->pRxPacket, wdev_idx);
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		struct rxd_base_struct *rx_base;

		rx_base = (struct rxd_base_struct *)pRxBlk->rmac_info;
		
		if ((rx_base->rxd_1.hdr_offset == 1) && (rx_base->rxd_1.payload_format != 0) && (rx_base->rxd_1.hdr_trans == 0)) {
			pRxBlk->pData += 2;
			pRxBlk->DataSize -= 2;
		}
	}
#endif	
	/*nMSDU =*/ deaggregate_AMSDU_announce(pAd, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}
#endif /* DOT11_N_SUPPORT */


VOID Announce_or_Forward_802_3_Packet(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN UCHAR wdev_idx,
	IN UCHAR op_mode)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	UCHAR *pPktHdr = NULL;
	UCHAR DestAddr[6];
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif/*ROAMING_ENHANCE_SUPPORT*/
#endif/*APCLI_SUPPORT*/
#endif/*CONFIG_AP_SUPPORT*/

	BOOLEAN to_os = FALSE;
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():invalid wdev_idx(%d)!\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev))
	{
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		if (wf_fwd_needed_hook != NULL && wf_fwd_needed_hook() == TRUE)
			set_wf_fwd_cb(pAd, pPacket, wdev);
#endif
#endif /* CONFIG_WIFI_PKT_FWD */	

		if((wdev->wdev_type == WDEV_TYPE_STA)
#ifdef EZ_API_SUPPORT	
			 && (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#endif	 
		){
			if(ez_apcli_rx_grp_pkt_drop(wdev,pPacket)){
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				//EZ_DEBUG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Drop Pkt for NonEz Duplicate link check!\n", __FUNCTION__));
				return;
			}
		}
	}
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef ROAMING_ENHANCE_SUPPORT
	if (pAd->ApCfg.bRoamingEnhance) {
		if ((pAd->ApCfg.ApCliInfRunned > 0)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			|| ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE))
#endif/* CONFIG_WIFI_PKT_FWD */
		) {
			pPktHdr = GET_OS_PKT_DATAPTR(pPacket);/*get 802.3 pkt*/
			pEntry = MacTableLookup(pAd, pPktHdr+6);/*Find the source mac entry*/
			if (pEntry) {
				NdisCopyMemory(DestAddr, pPktHdr, MAC_ADDR_LEN);
				if ((pEntry->bRoamingRefreshDone == FALSE) && IS_ENTRY_CLIENT(pEntry))
					ApCliDoRoamingRefresh(pAd, pEntry, pPacket, wdev, DestAddr);
			}
		}
	}
#endif/*ROAMING_ENHANCE_SUPPORT*/
#endif/*APCLIS_SUPPORT*/
#endif/*CONFIG_AP_SUPPORT*/

	if (wdev->rx_pkt_foward)
		to_os = wdev->rx_pkt_foward(pAd, wdev, pPacket);

	if (to_os == TRUE)
	{
#ifdef WH_EZ_SETUP
		if (!IS_EZ_SETUP_ENABLED(wdev))
#endif
		{
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			if (wf_fwd_needed_hook != NULL && wf_fwd_needed_hook() == TRUE)
				set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */	
#endif
		}
#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev))
			pAd->CurWdevIdx = wdev_idx;
#endif	
		announce_802_3_packet(pAd, pPacket,op_mode);
	}
	else {
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}


/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR wdev_idx)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UINT max_pkt_len = MAX_RX_PKT_LEN;
	UCHAR *pData = pRxBlk->pData;
	INT data_len = pRxBlk->DataSize;
	struct wifi_dev *wdev;
	UCHAR opmode = pAd->OpMode;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():invalid wdev_idx(%d)!\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_Legacy_Packet:drop packet by PN mismatch!\n"));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

//+++Add by shiang for debug
if (1) {
#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans) {
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
#endif /* HDR_TRANS_SUPPORT */
//	hex_dump("Indicate_Legacy_Packet", pData, data_len);
//	hex_dump("802_11_hdr", (UCHAR *)pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans) {
		max_pkt_len = 1514;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
	else
#endif /* HDR_TRANS_SUPPORT */

	if (!data_len) {
		/* release packet*/
		/* avoid processing with null paiload packets - QCA61X4A bug */
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}


	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
	//hex_dump("802_3_hdr", (UCHAR *)Header802_3, LENGTH_802_3);

	pData = pRxBlk->pData;
	data_len = pRxBlk->DataSize;


	if (data_len > max_pkt_len)
	{
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
DBGPRINT(RT_DEBUG_ERROR, ("%s():data_len(%d) > max_pkt_len(%d)!\n",
			__FUNCTION__, data_len, max_pkt_len));
		return;
	}

	STATS_INC_RX_PACKETS(pAd, wdev_idx);


#ifdef CONFIG_AP_SUPPORT
	WDEV_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, wdev);
#endif /* CONFIG_AP_SUPPORT */

//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pData, data_len);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug

#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans) {
		struct sk_buff *pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		pOSPkt->dev = get_netdev_from_bssid(pAd, wdev_idx);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		SET_OS_PKT_DATATAIL(pOSPkt, pOSPkt->len);
		//printk("%s: rx trans ...%d\n", __FUNCTION__, __LINE__);
	}
	else
#endif /* HDR_TRANS_SUPPORT */
	{
		RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);
	}

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
#ifdef A4_CONN
		MAC_TABLE_ENTRY *pEntry = NULL;
		pEntry = MacTableLookup(pAd, pRxBlk->pHeader->Addr2);
#endif
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != 1)
#ifdef A4_CONN
		&& (pEntry && (!IS_ENTRY_A4(pEntry)))	/*add a4 disable code here*/
#endif
		)
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11)
			{
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;

				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));

				if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
				{
					PUCHAR bootpHdr/*, dhcpHdr*/, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
					UCHAR isLinkValid;
#ifdef DATA_QUEUE_RESERVE
					PUCHAR dhcp_msg_type;
					/*
						1 = DHCP Discover message (DHCPDiscover).
						2 = DHCP Offer message (DHCPOffer).
						3 = DHCP Request message (DHCPRequest).
						4 = DHCP Decline message (DHCPDecline).
						5 = DHCP Acknowledgment message (DHCPAck).
						6 = DHCP Negative Acknowledgment message (DHCPNak).
						7 = DHCP Release message (DHCPRelease).
						8 = DHCP Informational message (DHCPInform).
					*/
#endif /* DATA_QUEUE_RESERVE */

					bootpHdr = pUdpHdr + 8;
					//dhcpHdr = bootpHdr + 236;
					pCliHwAddr = (bootpHdr+28);

#ifdef DATA_QUEUE_RESERVE
					if (pAd->bDump)
					{
						dhcp_msg_type = (bootpHdr+ (28 + 6 + 10 + 64+ 128 + 4));
						dhcp_msg_type += 2;

						if (*(dhcp_msg_type) == 2)
	 						DBGPRINT(RT_DEBUG_ERROR, ("### %s() DHCP OFFER to rept mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PRINT_MAC(pCliHwAddr)));

						if (*(dhcp_msg_type) == 5)
	 						DBGPRINT(RT_DEBUG_ERROR, ("### %s() DHCP ACK to rept mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PRINT_MAC(pCliHwAddr)));

						if (*(dhcp_msg_type) == 6)
	 						DBGPRINT(RT_DEBUG_ERROR, ("### %s() DHCP NACK to rept mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PRINT_MAC(pCliHwAddr)));

						if (*(dhcp_msg_type) == 8)
	 						DBGPRINT(RT_DEBUG_ERROR, ("### %s() DHCP INFORM to rept mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, PRINT_MAC(pCliHwAddr)));
					}
#endif /* DATA_QUEUE_RESERVE */

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE, &isLinkValid);
					if (pReptEntry)
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
					else
					{	
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
						VOID *other_band_tbl = NULL;
#endif

						if (wf_fwd_feedback_map_table)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
							wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl,&other_band_tbl);
#else
							wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
#endif

						if (opp_band_tbl != NULL) {
							/* 
								check the ReptTable of the opposite band due to dhcp packet (BC)
									may come-in 2/5G band when STA send dhcp broadcast to Root AP 
							*/
							pReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pCliHwAddr, FALSE, &isLinkValid);
							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						}
						else
								DBGPRINT(RT_DEBUG_INFO, ("cannot find the adapter of the oppsite band\n")); 
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
					if (other_band_tbl != NULL) {
						pReptEntry = RTMPLookupRepeaterCliEntry(other_band_tbl, FALSE, pCliHwAddr, FALSE, &isLinkValid);
						if (pReptEntry)
							NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
					}
					else
						DBGPRINT(RT_DEBUG_INFO, ("cannot find the adapter of the othersite band\n"));

#endif
					}
#endif
					bHdrChanged = TRUE;
				}

				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxPacket), GET_OS_PKT_LEN(pRxPacket));
}
//---Add by shiang for debug
	Announce_or_Forward_802_3_Packet(pAd, pRxPacket, wdev->wdev_idx, opmode);
}


/* Ralink Aggregation frame */
VOID Indicate_ARalink_Packet(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR Header802_3[LENGTH_802_3];
	UINT16 Msdu2Size;
	UINT16 Payload1Size, Payload2Size;
	PUCHAR pData2;
	PNDIS_PACKET pPacket2 = NULL;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;
	UCHAR opmode = pAd->OpMode;
	struct wifi_dev *wdev;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
	wdev = pAd->wdev_list[wdev_idx];

	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);
	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
	RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);

	ASSERT(pRxBlk->pRxPacket);

	pAd->RalinkCounters.OneSecRxARalinkCnt++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
	pPacket2 = duplicate_pkt_vlan(wdev->if_dev,
							wdev->VLAN_VID, wdev->VLAN_Priority,
							(pData2-LENGTH_802_3), LENGTH_802_3,
							pData2, Payload2Size, TPID);

	if (!pPacket2)
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, wdev_idx, TPID);

	Announce_or_Forward_802_3_Packet(pAd, pRxBlk->pRxPacket, wdev_idx, opmode);
	if (pPacket2)
		Announce_or_Forward_802_3_Packet(pAd, pPacket2, wdev_idx, opmode);
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


PNDIS_PACKET RTMPDeFragmentDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		RXWISize = 0;
#endif /* MT_MAC */	

	ASSERT(pHeader);

	HeaderRoom = pData - (UCHAR *)pHeader;

	/* Re-assemble the fragmented packets*/
	if (pHeader->Frag == 0)
	{	/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (pHeader->FC.MoreFrag)
		{
			ASSERT(pAd->FragFrame.pFragPacket);
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items.
			    Copy RxWI content to pFragBuffer.
			*/
			//pAd->FragFrame.RxSize = DataSize + HeaderRoom;
			//NdisMoveMemory(pFragBuffer, pHeader, pAd->FragFrame.RxSize);
			pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
			NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
			NdisMoveMemory(pFragBuffer + RXWISize,	 pHeader, pAd->FragFrame.RxSize - RXWISize);
			pAd->FragFrame.Sequence = pHeader->Sequence;
			pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Should be 0*/
			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		}
	}
	else
	{	/*Middle & End of fragment*/
		if ((pHeader->Sequence != pAd->FragFrame.Sequence) ||
			(pHeader->Frag != (pAd->FragFrame.LastFrag + 1)))
		{
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		//else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize)
		{
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done;
		}


		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pHeader->Sequence, pHeader->Frag));
			goto done;
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);

		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Update fragment number*/

		/* Last fragment*/
		if (pHeader->FC.MoreFrag == FALSE)
			bReassDone = TRUE;
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone)
	{
		PNDIS_PACKET pNewFragPacket;

		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
		if (pNewFragPacket)
		{
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			pAd->FragFrame.pFragPacket = pNewFragPacket;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			//pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket);
			//pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			//pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			//pRxBlk->pRxPacket = pRetPacket;
			pRxBlk->pRxWI = (RXWI_STRUC *) GET_OS_PKT_DATAPTR(pRetPacket);
			pRxBlk->pHeader = (PHEADER_802_11) ((UCHAR *)pRxBlk->pRxWI + RXWISize);
			pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			pRxBlk->pRxPacket = pRetPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}

	return pRetPacket;
}


VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	UCHAR *pTmpBuf;
	BOOLEAN to_mlme = TRUE, to_daemon = FALSE;
	struct wifi_dev *wdev;
	unsigned char hdr_len = LENGTH_802_11;

#if defined(WPA_SUPPLICANT_SUPPORT) && defined(CONFIG_AP_SUPPORT)
	STA_TR_ENTRY *tr_entry;
#endif

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX)
		goto done;

	wdev = pAd->wdev_list[wdev_idx];

	if(pRxBlk->DataSize < (LENGTH_802_1_H + LENGTH_EAPOL_H))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("pkts size too small\n"));
		goto done;
	}
	else if (!RTMPEqualMemory(SNAP_802_1H, pRxBlk->pData, 6))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("no SNAP_802_1H parameter\n"));
		goto done;
	}
	else if (!RTMPEqualMemory(EAPOL, pRxBlk->pData+6, 2))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("no EAPOL parameter\n"));
		goto done;
	}
	else if(*(pRxBlk->pData+9) > EAPOLASFAlert)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Unknown EAP type(%d)\n", *(pRxBlk->pData+9)));
		goto done;
	}

#ifdef A4_CONN
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

#ifdef CONFIG_AP_SUPPORT
	if (pEntry && IS_ENTRY_CLIENT(pEntry))
	{

#ifdef HOSTAPD_SUPPORT
		if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Indicate_Legacy_Packet\n"));
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
			return;
		}
#endif/*HOSTAPD_SUPPORT*/
	}
#endif /* CONFIG_AP_SUPPORT */



	if (IS_ENTRY_AP(pEntry))
	{
		{
			to_mlme = TRUE;
			to_daemon = FALSE;
		}
	}

#ifdef CONFIG_AP_SUPPORT
	if (IS_ENTRY_CLIENT(pEntry))
	{
#ifdef DOT1X_SUPPORT
		/* sent this frame to upper layer TCPIP */
		if ((pEntry->WpaState < AS_INITPMK) &&
			((pEntry->AuthMode == Ndis802_11AuthModeWPA) ||
			((pEntry->AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->PMKID_CacheIdx == ENTRY_NOT_FOUND)) ||
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.IEEE8021X == TRUE))
		{
			to_daemon = TRUE;
			to_mlme = FALSE;
			
#ifdef WSC_AP_SUPPORT
			/* report EAP packets to MLME to check this packet is WPS packet or not */
			if ((pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.WscConfMode != WSC_DISABLE) &&
				(!MAC_ADDR_EQUAL(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr, ZERO_MAC_ADDR)))
			{
				to_mlme = TRUE;
				pTmpBuf = pRxBlk->pData - hdr_len;
				// TODO: shiang-usw, why we need to change pHeader here??
				pRxBlk->pHeader = (PHEADER_802_11)pTmpBuf;
			}
#endif /* WSC_AP_SUPPORT */

		}
		else
#endif /* DOT1X_SUPPORT */
		{
			/* sent this frame to WPA state machine */

			/*
				Check Addr3 (DA) is AP or not.
				If Addr3 is AP, forward this EAP packets to MLME
				If Addr3 is NOT AP, forward this EAP packets to upper layer or STA.
			*/
			if (wdev->wdev_type == WDEV_TYPE_AP) {
				ASSERT(wdev->func_idx < HW_BEACON_MAX_NUM);
				if (wdev->func_idx < HW_BEACON_MAX_NUM) {
					ASSERT(wdev == (&pAd->ApCfg.MBSSID[wdev->func_idx].wdev));
				}
			}

			// TODO: shiang-usw, why we check this here??
			if ((wdev->wdev_type == WDEV_TYPE_AP) &&
				(NdisEqualMemory(pRxBlk->pHeader->Addr3, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid, MAC_ADDR_LEN) == FALSE))
				to_daemon = TRUE;
			else
				to_mlme = TRUE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	/*
	   Special DATA frame that has to pass to MLME
	   1. Cisco Aironet frames for CCX2. We need pass it to MLME for special process
	   2. EAPOL handshaking frames when driver supplicant enabled, pass to MLME for special process
	 */
	if (to_mlme)
	{
		pTmpBuf = pRxBlk->pData - hdr_len;
		NdisMoveMemory(pTmpBuf, pRxBlk->pHeader, hdr_len);
		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
							pTmpBuf,
							pRxBlk->DataSize + hdr_len,
							pRxBlk->rx_signal.raw_rssi[0],
							pRxBlk->rx_signal.raw_rssi[1],
							pRxBlk->rx_signal.raw_rssi[2],
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
							pRxBlk->rx_signal.raw_snr[0],
							pRxBlk->rx_signal.raw_snr[1],
#endif
							0,
							pRxBlk->OpMode);

		DBGPRINT_RAW(RT_DEBUG_TRACE,
			     ("!!! report EAPOL DATA to MLME (len=%d) !!!\n",
			      pRxBlk->DataSize));
	}

	if (to_daemon == TRUE)
	{
		Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
		return;
	}

done:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}


VOID Indicate_EAPOL_Packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: invalid wcid.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet:drop packet by PN mismatch!\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
	return;
}


bool check_duplicated_mgmt_frame(HEADER_802_11 *pHeader)
{
	static DUPLICATED_FRAME duplicated_frame[15]={
		{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}},
		{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}},
		{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}},{-1,{0}}};
	INT current_sn = pHeader->Sequence;
	UINT16 retry = pHeader->FC.Retry;
	UINT16 mgmt_type = pHeader->FC.SubType;
	
 	if (mgmt_type >= 15) {
		DBGPRINT(RT_DEBUG_OFF, ("%s:: check duplicated mgmt frame fail(invalid mgmt subtype(%d)) \n",__FUNCTION__, mgmt_type));
		return FALSE;
	}
	
	if (MAC_ADDR_EQUAL(duplicated_frame[mgmt_type].prev_mgmt_src_addr, pHeader->Addr2) && retry == 1 
		&& duplicated_frame[mgmt_type].prev_mgmt_frame_sn == current_sn) {
		DBGPRINT(RT_DEBUG_INFO, ("%s:: Drop duplicated mgmt frame(subtype=%d, current_sn=%d, prev_sn=%d, retry=%d) \n",__FUNCTION__, 
			mgmt_type, current_sn, duplicated_frame[mgmt_type].prev_mgmt_frame_sn, retry));
		return TRUE;
	} else {
		duplicated_frame[mgmt_type].prev_mgmt_frame_sn = current_sn;
		COPY_MAC_ADDR(duplicated_frame[mgmt_type].prev_mgmt_src_addr, pHeader->Addr2);
		return FALSE;
	}
}



// TODO: shiang-usw, modify the op_mode assignment for this function!!!
VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT op_mode = pRxBlk->OpMode;

DBGPRINT(RT_DEBUG_FPGA, ("-->%s()\n", __FUNCTION__));

#ifdef CFG_TDLS_SUPPORT
	if (CFG80211_HandleTdlsDiscoverRespFrame(pAd, pRxBlk, op_mode))
		goto done;
#endif /* CFG_TDLS_SUPPORT */



#ifdef DOT11W_PMF_SUPPORT
	if (PMF_PerformRxFrameAction(pAd, pRxBlk) == FALSE)
		goto done;
#endif /* DOT11W_PMF_SUPPORT */

	if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#ifdef APCLI_DOT11W_PMF_SUPPORT
	else {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
		pEntry = MacTableLookup(pAd, pHeader->Addr2);
#endif /* APCLI_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
		if (pEntry)
			pRxBlk->wcid = pEntry->wcid;
	}
#endif /* APCLI_DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

		op_mode = OPMODE_AP;
#ifdef IDS_SUPPORT
		/*
			Check if a rogue AP impersonats our mgmt frame to spoof clients
			drop it if it's a spoofed frame
		*/
		if (RTMPSpoofedMgmtDetection(pAd, pHeader, pRxBlk))
			goto done;

		/* update sta statistics for traffic flooding detection later */
		RTMPUpdateStaMgmtCounter(pAd, pHeader->FC.SubType);
#endif /* IDS_SUPPORT */

		if (!pRxInfo->U2M)
		{
			if ((pHeader->FC.SubType != SUBTYPE_BEACON) && (pHeader->FC.SubType != SUBTYPE_PROBE_REQ))
			{
				BOOLEAN bDrop = TRUE;
#if defined(WAPP_SUPPORT)
				if (IsPublicActionFrame(pAd, (VOID *)pHeader))
					bDrop = FALSE;
#endif /* WAPP_SUPPORT */

				if (bDrop)
					goto done;
			}
		}

		/* Software decrypt WEP data during shared WEP negotiation */
		if ((pHeader->FC.SubType == SUBTYPE_AUTH) &&
			(pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0))
		{
			UCHAR *pMgmt = (PUCHAR)pHeader;
			UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;

			if (!pEntry)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails - the Entry is empty.\n"));
				goto done;
			}

			/* Skip 802.11 header */
			pMgmt += LENGTH_802_11;
			mgmt_len -= LENGTH_802_11;

			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(pAd,
								   &pAd->SharedKey[pEntry->func_tb_idx][pRxBlk->key_idx],
								   pMgmt,
								   &mgmt_len) == FALSE)
			{
#ifdef WIFI_DIAG
				if (IS_ENTRY_CLIENT(pEntry))
					DiagConnError(pAd, pEntry->func_tb_idx, pHeader->Addr2,
						DIAG_CONN_AUTH_FAIL, REASON_DECRYPTION_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
				if (IS_ENTRY_CLIENT(pEntry))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pHeader->Addr2,
						REASON_MIC_FAILURE);
#endif
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails.\n"));
				goto done;
			}

#ifdef RT_BIG_ENDIAN
			/* swap 16 bit fields - Auth Alg No. field */
			*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);

			/* swap 16 bit fields - Auth Seq No. field */
			*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));

			/* swap 16 bit fields - Status Code field */
			*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */

			DBGPRINT(RT_DEBUG_TRACE, ("Decrypt AUTH seq#3 successfully\n"));

			/* Update the total length */
			pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
		}
	}
#endif /* CONFIG_AP_SUPPORT */



	if (pRxBlk->DataSize > MAX_RX_PKT_LEN) {
		DBGPRINT(RT_DEBUG_TRACE, ("DataSize=%d\n", pRxBlk->DataSize));
		hex_dump("MGMT ???", (UCHAR *)pHeader, pRxBlk->pData - (UCHAR *) pHeader);
		goto done;
	}

#if defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	if (pEntry && (pHeader->FC.SubType == SUBTYPE_ACTION))
	{
		/* only PM bit of ACTION frame can be set */
		if (((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) ||
			((op_mode == OPMODE_STA) && (IS_ENTRY_TDLS(pEntry))))
		   	RtmpPsIndicate(pAd, pHeader->Addr2, pRxBlk->wcid, pHeader->FC.PwrMgmt);

		/*
			In IEEE802.11, 11.2.1.1 STA Power Management modes,
			The Power Managment bit shall not be set in any management
			frame, except an Action frame.

			In IEEE802.11e, 11.2.1.4 Power management with APSD,
			If there is no unscheduled SP in progress, the unscheduled SP
			begins when the QAP receives a trigger frame from a non-AP QSTA,
			which is a QoS data or QoS Null frame associated with an AC the
			STA has configured to be trigger-enabled.
			So a management action frame is not trigger frame.
		*/
	}
#endif /* defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	//if(pHeader->FC.SubType == SUBTYPE_BEACON)
		//printk("%02x:%02x:%02x:%02x:%02x:%02x=================================> pRxBlk->wcid: %d\n", PRINT_MAC(pHeader->Addr2), pRxBlk->wcid);
	
	/* check duplicated mgmt frame */
	if(check_duplicated_mgmt_frame(pHeader) == TRUE)
	{
		goto done;
	}

	REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
						pHeader,
						pRxBlk->DataSize,
						pRxBlk->rx_signal.raw_rssi[0],
						pRxBlk->rx_signal.raw_rssi[1],
						pRxBlk->rx_signal.raw_rssi[2],
#if defined(CUSTOMER_DCC_FEATURE) || defined(NEIGHBORING_AP_STAT)
						pRxBlk->rx_signal.raw_snr[0],
						pRxBlk->rx_signal.raw_snr[1],
#endif
						min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
						op_mode);


#ifdef WIFI_DIAG
	DiagDevRxMgmtFrm(pAd, pRxBlk);
#endif

done:

DBGPRINT(RT_DEBUG_FPGA, ("<--%s()\n", __FUNCTION__));

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN OwFlag = TRUE;

#ifdef WIFI_DIAG
#ifdef CONFIG_SNIFFER_SUPPORT
	DiagDevRxCntlFrm(pAd, pRxBlk);
#endif
#endif

	switch (pHeader->FC.SubType)
	{
#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK_REQ:
			{
				FRAME_BA_REQ *bar = (FRAME_BA_REQ *)pHeader;

#ifdef AIR_MONITOR
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					/*
					To avoid control frames captured by sniffer confusing due to lack of Addr 3
					*/
					if(!IsValidUnicastToMe(pAd, pRxBlk->wcid, bar->Addr1))
					{
						RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
						return;
					}
				}
#endif /* CONFIG_AP_SUPPORT */
#endif /* AIR_MONITOR */

#ifdef MT_MAC
				if ((pAd->chipCap.hif_type == HIF_MT) &&
				    (pRxBlk->wcid == RESERVED_WCID)) {

					MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, &pHeader->Addr2[0]);
					if (pEntry) {

						pRxBlk->wcid = pEntry->wcid;
#ifdef APCLI_SUPPORT
						/* resolve macRepeater issue */
						if (IS_ENTRY_APCLI(pEntry)) {

							PAPCLI_STRUCT pApCliEntry = NULL;

							if (pEntry->wdev->func_idx < MAX_APCLI_NUM)
							pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx];

							if (pApCliEntry) {

								if ((NdisCmpMemory(pApCliEntry->wdev.if_addr,
									pHeader->Addr1, MAC_ADDR_LEN))
#ifdef MAC_REPEATER_SUPPORT
								&& ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
								NdisCmpMemory(pAd->ApCfg.ApCliTab[pEntry->func_tb_idx]
								.RepeaterCli[pEntry->MatchReptCliIdx]
								.CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN))
#endif
								) {

									OwFlag = FALSE;
									MTWF_LOG(DBG_CAT_ALL,
										DBG_SUBCAT_ALL,
										DBG_LVL_WARN,
										("APCLI PRINT VALID wcid=%d %pM\n",
										pRxBlk->wcid,
										pApCliEntry->wdev.if_addr));
									MTWF_LOG(DBG_CAT_ALL,
										DBG_SUBCAT_ALL,
										DBG_LVL_WARN,
										("%pM, %pM\n", pHeader->Addr1,
										pHeader->Addr2));
								}
							}
						}
#endif
					}
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Cannot found WCID of BAR packet!\n",
									__FUNCTION__));
					}
				}
#endif /* MT_MAC */

				if (OwFlag) {
				CntlEnqueueForRecv(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt),
									(PFRAME_BA_REQ)pHeader);

				if (bar->BARControl.Compressed == 0) {
					UCHAR tid = bar->BARControl.TID;
					BARecSessionTearDown(pAd, pRxBlk->wcid, tid, FALSE);
				}
			}
		}
			break;
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		case SUBTYPE_PS_POLL:
			/*CFG_TODO*/
			//IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				USHORT Aid = pHeader->Duration & 0x3fff;
				PUCHAR pAddr = pHeader->Addr2;
				MAC_TABLE_ENTRY *pEntry;

#ifdef MT_MAC
				if ((pAd->chipCap.hif_type == HIF_MT) &&
				    (pRxBlk->wcid == RESERVED_WCID)) 
				{
					pEntry = MacTableLookup(pAd, &pHeader->Addr2[0]);
					if (pEntry)
						pRxBlk->wcid = pEntry->wcid;
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Cannot found WCID of PS-Poll packet!\n",
									__FUNCTION__));
					}
				}
#endif /* MT_MAC */


               //printk("dev_rx_ctrl_frm0 SUBTYPE_PS_POLL pRxBlk->wcid: %x pEntry->wcid:%x\n",pRxBlk->wcid,pEntry->wcid);
				if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
                                 //printk("dev_rx_ctrl_frm1 SUBTYPE_PS_POLL\n");
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
					if (pEntry->Aid == Aid)
						RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, FALSE);
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
									__FUNCTION__, Aid, pEntry->Aid));
					}
				}
			}
			break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WFA_VHT_PF
		case SUBTYPE_RTS:
			if (pAd->CommonCfg.vht_bw_signal && pRxBlk->wcid <= MAX_LEN_OF_MAC_TABLE)
			{
				PLCP_SERVICE_FIELD *srv_field;
				RTS_FRAME *rts = (RTS_FRAME *)pRxBlk->pHeader;

				if ((rts->Addr1[0] & 0x1) == 0x1) {
					srv_field = (PLCP_SERVICE_FIELD *)&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[15];
					if (srv_field->dyn_bw == 1) {
						DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x, WCID:%d, DYN,BW=%d\n",
									PRINT_MAC(rts->Addr1), pRxBlk->wcid, srv_field->cbw_in_non_ht));
					}
				}
			}
			break;

		case SUBTYPE_CTS:
			break;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK:
//+++Add by shiang for debug
// TODO: shiang-MT7603, remove this!
			{
				UCHAR *ptr, *ra, *ta;
				BA_CONTROL *ba_ctrl;
				DBGPRINT(RT_DEBUG_OFF, ("%s():BlockAck From WCID:%d\n", __FUNCTION__, pRxBlk->wcid));

				ptr = (PUCHAR)pRxBlk->pHeader;
				ptr += 4;
				ra = ptr;
				ptr += 6;
				ta = ptr;
				ptr += 6;
				ba_ctrl = (BA_CONTROL *)ptr;
				ptr += sizeof(BA_CONTROL);
				DBGPRINT(RT_DEBUG_OFF, ("\tRA=%02x:%02x:%02x:%02x:%02x:%02x, TA=%02x:%02x:%02x:%02x:%02x:%02x\n",
							PRINT_MAC(ra), PRINT_MAC(ta)));
				DBGPRINT(RT_DEBUG_OFF, ("\tBA Control: AckPolicy=%d, MTID=%d, Compressed=%d, TID_INFO=0x%x\n",
							ba_ctrl->ACKPolicy, ba_ctrl->MTID, ba_ctrl->Compressed, ba_ctrl->TID));
				if (ba_ctrl->ACKPolicy == 0 && ba_ctrl->Compressed == 1) {
					BASEQ_CONTROL *ba_seq;
					ba_seq = (BASEQ_CONTROL *)ptr;
					DBGPRINT(RT_DEBUG_OFF, ("\tBA StartingSeqCtrl:StartSeq=%d, FragNum=%d\n",
									ba_seq->field.StartSeq, ba_seq->field.FragNum));
					ptr += sizeof(BASEQ_CONTROL);
					DBGPRINT(RT_DEBUG_OFF, ("\tBA Bitmap:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
								*ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7)));
				}
			}
//---Add by shiang for debug
#endif /* DOT11_N_SUPPORT */
		case SUBTYPE_ACK:
		default:
			break;
	}

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


/*
	========================================================================
	Routine Description:
		Check Rx descriptor, return NDIS_STATUS_FAILURE if any error found
	========================================================================
*/
static INT rtmp_chk_rx_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, HEADER_802_11 *pHdr)
{
	RXINFO_STRUC *pRxInfo;
	if((pRxBlk == NULL) || (pRxBlk->pRxInfo == NULL))
		return NDIS_STATUS_FAILURE;

	pRxInfo = pRxBlk->pRxInfo;
	
#ifdef MT_MAC
	// TODO: shiang-MT7603
	if (pAd->chipCap.hif_type == HIF_MT) {
//+++Add by shiang for work-around, should remove it once we correctly configure the BSSID!
		// TODO: shiang-MT7603 work around!!
		struct rxd_base_struct *rxd_base = (struct rxd_base_struct *)pRxBlk->rmac_info;

		if (rxd_base->rxd_2.icv_err) {
#ifdef A4_CONN
			if (IS_BM_MAC_ADDR(pRxBlk->pHeader->Addr1)) {
				MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAd, pRxBlk->pHeader->Addr2);

				if (pEntry && IS_ENTRY_APCLI(pEntry) && IS_ENTRY_A4(pEntry))
					return NDIS_STATUS_FAILURE;
			}
#endif
			DBGPRINT(RT_DEBUG_OFF, ("ICV Error\n"));
			dump_rxblk(pAd, pRxBlk);
#ifdef WIFI_DIAG
			/* WEP + open, wrong passowrd can association success, but rx data error */
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
				if (IS_ENTRY_CLIENT(pEntry) && (pEntry->WepStatus == Ndis802_11WEPEnabled))
					DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
						DIAG_CONN_AUTH_FAIL, REASON_DECRYPTION_FAIL);
			}
#endif

#ifdef CONN_FAIL_EVENT
			/* WEP + open, wrong passowrd can association success, but rx data error */
			if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) && (pRxBlk->pRxInfo->U2M)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (IS_ENTRY_CLIENT(pEntry) && (pEntry->WepStatus == Ndis802_11WEPEnabled))
					ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
						pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
						pEntry->Addr,
						REASON_MIC_FAILURE);
			}
#endif
			return NDIS_STATUS_FAILURE;
		}
		if (rxd_base->rxd_2.cm && !rxd_base->rxd_2.null_frm && !rxd_base->rxd_2.ndata) {
			DBGPRINT(RT_DEBUG_INFO, ("CM\n"));
			//dump_rxblk(pAd, pRxBlk);
			//hex_dump("CMPkt",  (UCHAR *)rxd_base, rxd_base->rxd_0.rx_byte_cnt);
			return NDIS_STATUS_SUCCESS;
		}
		if (rxd_base->rxd_2.clm) {
			DBGPRINT(RT_DEBUG_OFF, ("CM Length Error\n"));
			return NDIS_STATUS_FAILURE;
		}
		if (rxd_base->rxd_2.tkip_mic_err) {
			DBGPRINT(RT_DEBUG_OFF, ("TKIP MIC Error\n"));
		}
	}
#endif /* MT_MAC */

	

	/* Phy errors & CRC errors*/
	if (pRxInfo->Crc) {

		return NDIS_STATUS_FAILURE;
	}


	/* drop decyption fail frame*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pRxInfo->CipherErr)
		{
			/*
				WCID equal to 255 mean MAC couldn't find any matched entry in Asic-MAC table.
				The incoming packet mays come from WDS or AP-Client link.
				We need them for further process. Can't drop the packet here.
			*/
			if ((pRxInfo->U2M) && (pRxBlk->wcid == 255)
#ifdef WDS_SUPPORT
				&& (pAd->WdsTab.Mode == WDS_LAZY_MODE)
#endif /* WDS_SUPPORT */
			)
				return NDIS_STATUS_SUCCESS;

			APRxErrorHandle(pAd, pRxBlk);

			/* Increase received error packet counter per BSS */
			if (pHdr->FC.FrDs == 0 &&
				pRxInfo->U2M &&
				pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
			{
				BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];
				pMbss->RxDropCount ++;
				pMbss->RxErrorCount ++;
			}

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
			if ((pHdr->FC.FrDs == 1) && (pHdr->FC.ToDs == 1) &&
				(pRxBlk->wcid <MAX_LEN_OF_MAC_TABLE))
			{
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if (IS_ENTRY_WDS(pEntry) && (pEntry->func_tb_idx < MAX_WDS_ENTRY))
					pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].WdsCounter.RxErrorCount++;
			}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */

			DBGPRINT(RT_DEBUG_INFO, ("%s(): pRxInfo:Crc=%d, CipherErr=%d, U2M=%d, Wcid=%d\n",
						__FUNCTION__, pRxInfo->Crc, pRxInfo->CipherErr, pRxInfo->U2M, pRxBlk->wcid));
			return NDIS_STATUS_FAILURE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */


	return NDIS_STATUS_SUCCESS;
}


BOOLEAN dev_rx_no_foward(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	return TRUE;
}


#ifdef CONFIG_ATE
INT ate_rx_done_handle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{

#if defined(CONFIG_QA) || defined(HUAWEI_ATE)
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
#endif
#ifdef CONFIG_QA
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
#endif /* CONFIG_QA */
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	
#if defined(CONFIG_QA) || defined(HUAWEI_ATE)	
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (UCHAR *)pHeader, DIR_READ, TRUE);
#endif /* RT_BIG_ENDIAN */
#endif

		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);

	ATECtrl->RxTotalCnt++;
	ATEOp->SampleRssi(pAd, pRxBlk);

#ifdef CONFIG_QA
	if ((ATECtrl->bQARxStart == TRUE) || (ATECtrl->Mode & ATE_RXFRAME))
	{
		/* GetPacketFromRxRing() has copy the endian-changed RxD if it is necessary. */
		ATE_QA_Statistics(pAd, pRxWI, pRxInfo, pHeader);
	}

#endif /* CONFIG_QA */

	return TRUE;
}
#endif /* CONFIG_ATE */




#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)
INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For STA, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE; /* need annouce to upper layer */
}


INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = FALSE;
	struct wifi_dev *wdev;
#ifdef APCLI_SUPPORT
#ifdef A4_CONN
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif
#endif


DBGPRINT(RT_DEBUG_INFO, ("-->%s():pRxBlk->wcid=%d\n", __FUNCTION__, pRxBlk->wcid));

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	wdev = pEntry->wdev;

	if (wdev == NULL)
		DBGPRINT(RT_DEBUG_TRACE, ("wdev is NULL.\n"));

#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(wdev)) 
			hdr_len = LENGTH_802_11;
#endif
	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
	{
#ifdef CLIENT_WDS
		if ((pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			&& IS_ENTRY_CLIENT(pEntry))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
		}
#endif /* CLIENT_WDS */
#ifdef APCLI_SUPPORT
#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry)) {
			if(pEntry->func_tb_idx < MAX_APCLI_NUM)
				pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
			if(pApCliEntry)
			{
				NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
				if(MAC_ADDR_EQUAL(pHeader->Octet, pApCliEntry->wdev.if_addr))
				{
				   MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ApCli receive a looping packet!\n"));
				   return FALSE;
				}
			}
		}
#endif /* A4_CONN */
#endif /* APCLI_SUPPORT */

		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;
	}


	ASSERT((pEntry != NULL));

	/* Drop not my BSS frames */
	if (pRxInfo->MyBss == 0) {
/* CFG_TODO: NEED CHECK for MT_MAC */	
#ifdef A4_CONN
		if (IS_ENTRY_A4(pEntry))
			pRxInfo->MyBss = 1;
		else
#endif /* A4_CONN */
		{
DBGPRINT(RT_DEBUG_OFF, ("%s():  Not my bss! pRxInfo->MyBss=%d\n", __FUNCTION__, pRxInfo->MyBss));
			return FALSE;
		}
	}


	pAd->RalinkCounters.RxCountSinceLastNULL++;
#ifdef UAPSD_SUPPORT

	if (!wdev) {
		DBGPRINT(RT_DEBUG_TRACE, ("wdev is NULL.\n"));
		return FALSE;
	}

	if (wdev->UapsdInfo.bAPSDCapable
	    && pAd->CommonCfg.APEdcaParm.bAPSDCapable
	    && (pHeader->FC.SubType & 0x08))
	{
		UCHAR *pData;
		DBGPRINT(RT_DEBUG_INFO, ("bAPSDCapable\n"));

		/* Qos bit 4 */
		pData = (PUCHAR) pHeader + LENGTH_802_11;
		if ((*pData >> 4) & 0x01)
		{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)

			/* ccv EOSP frame so the peer can sleep */
			if (pEntry != NULL)
			{
				RTMP_PS_VIRTUAL_SLEEP(pEntry);
			}

			if (pAd->StaCfg.FlgPsmCanNotSleep == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("tdls uapsd> Rcv EOSP frame but we can not sleep!\n"));
			}
			else
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
				{
				DBGPRINT(RT_DEBUG_INFO,
					("RxDone- Rcv EOSP frame, driver may fall into sleep\n"));
				pAd->CommonCfg.bInServicePeriod = FALSE;

			}
		}

		if ((pHeader->FC.MoreData) && (pAd->CommonCfg.bInServicePeriod)) {
			DBGPRINT(RT_DEBUG_TRACE, ("MoreData bit=1, Sending trigger frm again\n"));
		}
	}
#endif /* UAPSD_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	if (pEntry != NULL)
	{
		UCHAR OldPwrMgmt;

		OldPwrMgmt = RtmpPsIndicate(pAd, pHeader->Addr2, pEntry->wcid, pFmeCtrl->PwrMgmt);
#ifdef UAPSD_SUPPORT
		RTMP_PS_VIRTUAL_TIMEOUT_RESET(pEntry);

		if (pFmeCtrl->PwrMgmt)
		{
			if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
				(pFmeCtrl->SubType & 0x08))
			{
				/*
					In IEEE802.11e, 11.2.1.4 Power management with APSD,
					If there is no unscheduled SP in progress, the unscheduled SP begins
					when the QAP receives a trigger frame from a non-AP QSTA, which is a
					QoS data or QoS Null frame associated with an AC the STA has
					configured to be trigger-enabled.

					In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
					to/from Power Save Mode is not considered to be a Trigger Frame and
					the AP shall not respond with a QoS Null frame.
				*/
				/* Trigger frame must be QoS data or QoS Null frame */
				UCHAR  OldUP;

				if ((*(pRxBlk->pData+LENGTH_802_11) & 0x10) == 0)
				{
					/* this is not a EOSP frame */
					OldUP = (*(pRxBlk->pData+LENGTH_802_11) & 0x07);
					if (OldPwrMgmt == PWR_SAVE)
					{
						//hex_dump("trigger frame", pRxBlk->pData, 26);
						UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
					}
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("This is a EOSP frame, not a trigger frame!\n"));
				}
			}
		}
#endif /* UAPSD_SUPPORT */
	}
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if ((pFmeCtrl->SubType & 0x04)) /* bit 2 : no DATA */ {
		DBGPRINT(RT_DEBUG_OFF, ("%s():  No DATA!\n", __FUNCTION__));
		return FALSE;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	rept_entry = lookup_rept_entry(pAd, pHeader->Addr1);
#endif
	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0))
		RX_BLK_SET_FLAG(pRxBlk, fRX_AP);

	if (pEntry && (IS_ENTRY_AP(pEntry)
#ifdef MAC_REPEATER_SUPPORT
		|| (rept_entry && (rept_entry->CliEnable))
#endif
	)) {
#if(defined (WH_EZ_SETUP) && defined(EZ_DUAL_BAND_SUPPORT))
		if(IS_EZ_SETUP_ENABLED(wdev) 
#ifdef EZ_API_SUPPORT	
			&& (wdev->ez_driver_params.ez_api_mode != CONNECTION_OFFLOAD) 
#endif		
		){ 
			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Rx Pkt on => wdev_type=0x%x, func_idx=0x%x\n",wdev->wdev_type,wdev->func_idx));
			//hex_dump("sta_rx_pkt_allow: Eth Hdr: ",pRxBlk->pData,14)
			//EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
			//	  ((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
			//	  ((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
			//	  ((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
			/*if( ( (((PUCHAR)pRxBlk->pData)[4])& 0x1 )== 0x1){
				if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)){ 
					EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nMWDS PKT Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
						wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
						((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],
						((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13],((PUCHAR)pRxBlk->pData)[14],((PUCHAR)pRxBlk->pData)[15],
						((PUCHAR)pRxBlk->pData)[16],((PUCHAR)pRxBlk->pData)[17]));
				}
				else{ 
					EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("sta_rx_pkt_allow: wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x \nNON MWDS PKT Eth Hdr: Dest[%02x-%02x-%02x-%02x-%02x-%02x] Source[%02x-%02x-%02x-%02x-%02x-%02x] Type[%02x-%02x]\n",
						wdev->wdev_idx,wdev->wdev_type,wdev->func_idx,
						((PUCHAR)pRxBlk->pData)[0],((PUCHAR)pRxBlk->pData)[1],((PUCHAR)pRxBlk->pData)[2],((PUCHAR)pRxBlk->pData)[3],((PUCHAR)pRxBlk->pData)[4],((PUCHAR)pRxBlk->pData)[5],
						((PUCHAR)pRxBlk->pData)[6],((PUCHAR)pRxBlk->pData)[7],((PUCHAR)pRxBlk->pData)[8],((PUCHAR)pRxBlk->pData)[9],((PUCHAR)pRxBlk->pData)[10],((PUCHAR)pRxBlk->pData)[11],
						((PUCHAR)pRxBlk->pData)[12],((PUCHAR)pRxBlk->pData)[13]));
			
				}
			}*/
				
			if (MAC_ADDR_IS_GROUP(pRxBlk->pData) 
				&& ez_sta_rx_pkt_handle(wdev, pRxBlk))
			{
				return FALSE;
			}

		}
#endif

		if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 0)) {
#ifdef MWDS
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
					/* MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, */
					/* ("wdev_idx=0x%x, wdev_type=0x%x, func_idx=0x%x : Non MWDS Pkt not allowed\n", */
						/* wdev->wdev_idx,wdev->wdev_type,wdev->func_idx)); */
						return FALSE;
			}
#endif /* MWDS */

#if defined(MAP_SUPPORT) && defined(A4_CONN)
			/* do not receive 3-address broadcast/multicast packet, */
			/* because the broadcast/multicast packet woulld be send using 4-address, */
			/* 1905 message is an exception, need to receive 3-address 1905 multicast, */
			/* because some vendor send only one 3-address 1905 multicast packet */
			/* 1905 daemon would filter and drop duplicate packet */
			if (GET_ENTRY_A4(pEntry) == A4_TYPE_MAP &&
				(pRxInfo->Mcast || pRxInfo->Bcast) &&
				(memcmp(pHeader->Addr1, multicast_mac_1905, MAC_ADDR_LEN) != 0))
				return FALSE;
#endif
		}

		RX_BLK_SET_FLAG(pRxBlk, fRX_AP);
		goto ret;
    }
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	if (pEntry) {
	}



#ifndef WFA_VHT_PF
	// TODO: shiang@PF#2, is this atheros protection still necessary here???
	/* check Atheros Client */
	if ((pEntry->bIAmBadAtheros == FALSE) && (pRxInfo->AMPDU == 1)
	    && (pHeader->FC.Retry)) {
		pEntry->bIAmBadAtheros = TRUE;
	}
#endif /* WFA_VHT_PF */

ret: 

#ifdef A4_CONN
	if (!IS_ENTRY_A4(pEntry))
#endif
	hdr_len = LENGTH_802_11;

	return hdr_len;
}
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */


VOID rx_data_frm_announce(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN struct wifi_dev *wdev)
{
	BOOLEAN eth_frame = FALSE;
	UCHAR *pData = pRxBlk->pData;
	UINT data_len = pRxBlk->DataSize;
	UCHAR wdev_idx = wdev->wdev_idx;

	ASSERT(wdev_idx < WDEV_NUM_MAX);
	if (wdev_idx >= WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid wdev_idx(%d)\n", __FUNCTION__, wdev_idx));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans) {
		eth_frame = TRUE;
		pData = pRxBlk->pTransData;
		data_len = pRxBlk->TransDataSize;
	}
#endif /* HDR_TRANS_SUPPORT */

	/* non-EAP frame */
	if (!RTMPCheckWPAframe(pAd, pEntry, pData, data_len, wdev_idx, eth_frame))
	{
#ifdef MWDS
		if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS)
		{
			if (!((pRxBlk->pHeader->FC.FrDs == 1) && (pRxBlk->pHeader->FC.ToDs == 1)))
			{
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif


#ifdef WAPI_SUPPORT
		/* report to upper layer if the received frame is WAI frame */
		if (RTMPCheckWAIframe(pRxBlk->pData, pRxBlk->DataSize)) {
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
			return;
		}
#endif /* WAPI_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		/* drop all non-EAP DATA frame before peer's Port-Access-Control is secured */
		if ((pEntry->wdev->wdev_type == WDEV_TYPE_AP) &&
		    IS_ENTRY_CLIENT(pEntry) && (pEntry->PrivacyFilter == Ndis802_11PrivFilter8021xWEP))
		{
			/*
				If	1) no any EAP frame is received within 5 sec and
					2) an encrypted non-EAP frame from peer associated STA is received,
				AP would send de-authentication to this STA.
			 */
			if (pRxBlk->pHeader->FC.Wep &&
				pEntry->StaConnectTime > 5 && pEntry->WpaState < AS_AUTHENTICATION2)
			{
				DBGPRINT(RT_DEBUG_WARN, ("==> De-Auth this STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
							PRINT_MAC(pEntry->Addr)));
				MlmeDeAuthAction(pAd, pEntry, REASON_NO_LONGER_VALID, FALSE);
			}

			RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
#endif /* CONFIG_AP_SUPPORT */


		
#ifdef IGMP_SNOOP_SUPPORT
		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
			&& (pAd->ApCfg.IgmpSnoopEnable)
			&& IS_MULTICAST_MAC_ADDR(pRxBlk->pHeader->Addr3))
		{
			PUCHAR pDA = pRxBlk->pHeader->Addr3;
			PUCHAR pSA = pRxBlk->pHeader->Addr2;
			PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			if (protoType == ETH_P_IP)
				IGMPSnooping(pAd, pDA, pSA, pData, wdev->if_dev);
			else if (protoType == ETH_P_IPV6)
				MLDSnooping(pAd, pDA, pSA,  pData, wdev->if_dev);
		}
#endif /* IGMP_SNOOP_SUPPORT */

		if(pAd->bPingLog)
		{
			CheckICMPPacket(pAd, pRxBlk->pData, 1);
		}
#ifdef CONFIG_HOTSPOT
		if (pEntry->pMbss->HotSpotCtrl.HotSpotEnable) {
			if (hotspot_rx_handler(pAd, pEntry, pRxBlk) == TRUE)
				return;
		}
#endif /* CONFIG_HOTSPOT */

#ifdef CONFIG_AP_SUPPORT
#ifdef STATS_COUNT_SUPPORT
		if ((IS_ENTRY_CLIENT(pEntry)) && (pEntry->pMbss))
		{
			BSS_STRUCT *pMbss = pEntry->pMbss;
			UCHAR *pDA = pRxBlk->pHeader->Addr3;
			if (((*pDA) & 0x1) == 0x01) {
				if(IS_BROADCAST_MAC_ADDR(pDA))
				{
					pMbss->bcPktsRx++;

				}
				else
					pMbss->mcPktsRx++;
			} else
				pMbss->ucPktsRx++;
		}
#endif /* STATS_COUNT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) /*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
			Indicate_AMPDU_Packet(pAd, pRxBlk, wdev_idx);
		else if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			Indicate_AMSDU_Packet(pAd, pRxBlk, wdev_idx);
		else
#endif /* DOT11_N_SUPPORT */
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_ARALINK))
			Indicate_ARalink_Packet(pAd, pEntry, pRxBlk, wdev->wdev_idx);
		else
			Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);
	}
	else
	{
		RX_BLK_SET_FLAG(pRxBlk, fRX_EAP);

#ifdef CONFIG_AP_SUPPORT
		/* Update the WPA STATE to indicate the EAP handshaking is started */
		if (IS_ENTRY_CLIENT(pEntry)) {
			if (pEntry->WpaState == AS_AUTHENTICATION)
			pEntry->WpaState = AS_AUTHENTICATION2;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)
			/*&& (pAd->CommonCfg.bDisableReordering == 0)*/)
		{
			Indicate_AMPDU_Packet(pAd, pRxBlk, wdev_idx);
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
#ifdef CONFIG_HOTSPOT_R2
			UCHAR *pData = (UCHAR *)pRxBlk->pData;

			if (pEntry)
			{
				BSS_STRUCT *pMbss = pEntry->pMbss;
				if (NdisEqualMemory(SNAP_802_1H, pData, 6) ||
			        /* Cisco 1200 AP may send packet with SNAP_BRIDGE_TUNNEL*/
        			NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pData, 6))
			    {
			        pData += 6;
			    }

				if (NdisEqualMemory(EAPOL, pData, 2))
    	    		pData += 2;

			    if ((*(pData+1) == EAPOLStart) && (pMbss->HotSpotCtrl.HotSpotEnable == 1) && (pMbss->wdev.AuthMode == Ndis802_11AuthModeWPA2) && (pEntry->hs_info.ppsmo_exist == 1))
				{
					UCHAR HS2_Header[4] = {0x50,0x6f,0x9a,0x12};
					memcpy(&pRxBlk->pData[pRxBlk->DataSize], HS2_Header, 4);
					memcpy(&pRxBlk->pData[pRxBlk->DataSize+4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
					printk("rcv eapol start, %x:%x:%x:%x\n",pRxBlk->pData[pRxBlk->DataSize+4], pRxBlk->pData[pRxBlk->DataSize+5],pRxBlk->pData[pRxBlk->DataSize+6], pRxBlk->pData[pRxBlk->DataSize+7]);
					pRxBlk->DataSize += 8;
				}
			}
#endif
			/* Determin the destination of the EAP frame */
			/*  to WPA state machine or upper layer */
			rx_eapol_frm_handle(pAd, pEntry, pRxBlk, wdev_idx);
		}
	}
}


#define SN_NQOS_INDEX 8
static INT rx_chk_duplicate_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;
	UCHAR up = pRxBlk->UserPriority;
	UCHAR wcid = pRxBlk->wcid;
	STA_TR_ENTRY *trEntry= NULL;
	INT sn = pHeader->Sequence;

	/*check if AMPDU frame ignore it, since AMPDU wil handle reorder problem and AMSDU is the set of AMPDU*/
	if(RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) || RX_BLK_TEST_FLAG(pRxBlk,fRX_AMSDU))
	{
		return NDIS_STATUS_SUCCESS;
	}

	/*check is vaild sta entry*/
	if(wcid >= MAX_LEN_OF_TR_TABLE)
	{
		return NDIS_STATUS_SUCCESS;
	}

	/*check sta tr entry is exist*/
	trEntry = &pAd->MacTab.tr_entry[wcid];
	if(!trEntry)
	{
		return NDIS_STATUS_SUCCESS;
	}
	/*check frame is QoS or Non-QoS frame*/
	if(!(pFmeCtrl->SubType & 0x08))
	{
		up = SN_NQOS_INDEX;
	}

	/*check is not retry frame or check sn is duplicate or not, update sn only*/
	if(!pFmeCtrl->Retry || trEntry->cacheSn[up] != sn)
	{
		/*update cache*/
		trEntry->cacheSn[up] = sn;
		return NDIS_STATUS_SUCCESS;
	}

	/* Middle/End of fragment */
	if (pHeader->Frag && pHeader->Frag != pAd->FragFrame.LastFrag)
	{
		return NDIS_STATUS_SUCCESS;
	}

	/*is duplicate frame, should return failed*/
	return NDIS_STATUS_FAILURE;
}



/*
 All Rx routines use RX_BLK structure to hande rx events
 It is very important to build pRxBlk attributes
  1. pHeader pointer to 802.11 Header
  2. pData pointer to payload including LLC (just skip Header)
  3. set payload size including LLC to DataSize
  4. set some flags with RX_BLK_SET_FLAG()
*/
// TODO: shiang-usw, FromWhichBSSID is replaced by "pRxBlk->wdev_idx"
// TODO:
// TODO: FromWhichBSSID = pEntry->apidx // For AP
// TODO: FromWhichBSSID = BSS0; // For STA
// TODO: FromWhichBSSID = pEntry->MatchMeshTabIdx + MIN_NET_DEVICE_FOR_MESH; // For Mesh
// TODO: FromWhichBSSID = pEntry->MatchWDSTabIdx + MIN_NET_DEVICE_FOR_WDS; // For WDS
// TODO: FromWhichBSSID = pEntry->MatchAPCLITabIdx + MIN_NET_DEVICE_FOR_APCLI; // For APCLI
// TODO: FromWhichBSSID = pEntry->apidx + MIN_NET_DEVICE_FOR_P2P_GO;  // For P2P
VOID dev_rx_data_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	BOOLEAN bFragment = FALSE;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR wdev_idx = BSS0;
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;
	UCHAR UserPriority = 0;
	INT hdr_len = LENGTH_802_11;
	COUNTER_RALINK *pCounter = &pAd->RalinkCounters;
	UCHAR *pData;
	struct wifi_dev *wdev;
	BOOLEAN drop_err = TRUE;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) */
#ifdef NEW_IXIA_METHOD
	UCHAR rdrop_reson = 0;
#endif
    DBGPRINT(RT_DEBUG_INFO, ("-->%s():pRxBlk->wcid=%d, pRxBlk->DataSize=%d\n",
                __FUNCTION__, pRxBlk->wcid, pRxBlk->DataSize));

//dump_rxblk(pAd, pRxBlk);


//+++Add by shiang for debug

//	hex_dump("DataFrameHeader", (UCHAR *)pHeader, sizeof(HEADER_802_11));
//	hex_dump("DataFramePayload", pRxBlk->pData , (pRxBlk->DataSize > 128 ? 128 :pRxBlk->DataSize));
//---Add by shiangf for debug

	// TODO: shiang-usw, check wcid if we are repeater mode! when in Repeater mode, wcid is get by "A2" + "A1"
	if (VALID_WCID(pRxBlk->wcid))
	{
		pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
#ifdef MT7603		
		if (MTK_REV_GTE(pAd, MT7603, MT7603E1)) 
		{
			if (NdisCmpMemory(pEntry->Addr, pHeader->Addr2, MAC_ADDR_LEN))
			{
				pEntry = MacTableLookup(pAd, pHeader->Addr2);
				if (pEntry)
					pRxBlk->wcid = pEntry->wcid;
			}
		}
#endif	/* MT7603 */

		/* resolve macRepeater issue */
		if (pEntry) {

			pRxBlk->wcid = pEntry->wcid;
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pEntry)) {

				PAPCLI_STRUCT pApCliEntry = NULL;

				if (pEntry->wdev->func_idx < MAX_APCLI_NUM)
					pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx];

				if (pApCliEntry) {
					if ((NdisCmpMemory(pApCliEntry->wdev.if_addr,
						pHeader->Addr1, MAC_ADDR_LEN))
#ifdef MAC_REPEATER_SUPPORT
					&& ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
					NdisCmpMemory(pAd->ApCfg.ApCliTab[pEntry->func_tb_idx]
					.RepeaterCli[pEntry->MatchReptCliIdx]
					.CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN))
#endif /* MAC_REPEATER_SUPPORT */
				) {

					if (!(pRxInfo->Mcast || pRxInfo->Bcast))
						pEntry = NULL;
					MTWF_LOG(DBG_CAT_ALL,
						DBG_SUBCAT_ALL,
						DBG_LVL_WARN,
						("APCLI PRINT VALID wcid=%d %pM\n",
						pRxBlk->wcid,
						pApCliEntry->wdev.if_addr));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						("%pM %pM\n", pHeader->Addr1,
						pHeader->Addr2));
					}
				}
			}
#endif
		}
	}
	else {
		/* IOT issue with Marvell test bed AP
		    Marvell AP ResetToOOB and do wps.
		    Because of AP send EAP Request too fast and without retransmit.
		    STA not yet add BSSID to WCID search table.
		    So, the EAP Request is dropped.
		    The patch lookup pEntry from MacTable.
		*/
		pEntry = MacTableLookup(pAd, pHeader->Addr2);
		if (pEntry) {

			pRxBlk->wcid = pEntry->wcid;
#ifdef APCLI_SUPPORT
			/* resolve macRepeater issue */
			if (IS_ENTRY_APCLI(pEntry)) {

				PAPCLI_STRUCT pApCliEntry = NULL;

				if (pEntry->wdev->func_idx < MAX_APCLI_NUM)
					pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev->func_idx];

				if (pApCliEntry) {
					if ((NdisCmpMemory(pApCliEntry->wdev.if_addr,
						pHeader->Addr1, MAC_ADDR_LEN))
#ifdef MAC_REPEATER_SUPPORT
					&& ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
					NdisCmpMemory(pAd->ApCfg.ApCliTab[pEntry->func_tb_idx]
					.RepeaterCli[pEntry->MatchReptCliIdx]
					.CurrentAddress, pHeader->Addr1, MAC_ADDR_LEN))
#endif /* MAC_REPEATER_SUPPORT */
				) {

					if (!(pRxInfo->Mcast || pRxInfo->Bcast))
						pEntry = NULL;
					MTWF_LOG(DBG_CAT_ALL,
						DBG_SUBCAT_ALL,
						DBG_LVL_WARN,
						("APCLI PRINT VALID wcid=%d %pM\n",
						pRxBlk->wcid,
						pApCliEntry->wdev.if_addr));
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
						("%pM %pM\n", pHeader->Addr1,
						pHeader->Addr2));
					}
				}
			}
#endif
		}
	}



	/*
		if FrameCtrl.type == DATA
			FromDS = 1, ToDS = 1 : send from WDS/MESH
			FromDS = 1, ToDS = 0 : send from STA
			FromDS = 0, ToDS = 1 : send from AP
			FromDS = 0, ToDS = 0 : AdHoc / TDLS

			if pRxBlk->wcid == VALID,
				directly assign to the pEntry[WCID]->wdev->rx

			if pRxBlk->wcid == INVALD,
				FromDS = 1, ToDS = 1 : WDS/MESH Rx
				FromDS = 1, ToDS = 0 : drop
				FromDS = 0, ToDS = 1 : drop
				FromDS = 0, ToDS = 0 : AdHoc/TDLS Rx
	*/
	if (pEntry && pEntry->wdev && pEntry->wdev->rx_pkt_allowed)
		hdr_len = pEntry->wdev->rx_pkt_allowed(pAd, pRxBlk);
	else {
		if (pEntry) {
			DBGPRINT(RT_DEBUG_INFO, ("invalid hdr_len, wdev=%p! ", pEntry->wdev));
			if (pEntry->wdev) {
				DBGPRINT(RT_DEBUG_INFO, ("rx_pkt_allowed=%p!", pEntry->wdev->rx_pkt_allowed));
			}
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		}
		else
		{
#ifdef CONFIG_AP_SUPPORT
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
			{
				if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
					pEntry = FindWdsEntry(pAd, pRxBlk->wcid, pHeader->Addr2, pRxBlk->rx_rate.field.MODE);
			}
#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

			/* check if Class2 or 3 error */
			if ((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))
			{
				APChkCls2Cls3Err(pAd, pRxBlk->wcid, pHeader);
			}
#endif /* CONFIG_AP_SUPPORT */
		}
#ifdef NEW_IXIA_METHOD
		rdrop_reson = DROP_NOT_ALLOW;
#endif
		goto drop;
	}

	wdev = pEntry->wdev;
	wdev_idx = wdev->wdev_idx;
	DBGPRINT(RT_DEBUG_INFO, ("%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
                __FUNCTION__, pEntry->wcid, wdev->wdev_idx,
                pRxBlk->Flags,
                RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
                RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
                pHeader->FC.Type, pHeader->FC.SubType,
                pHeader->FC.FrDs, pHeader->FC.ToDs));

   	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
   	/* must be here, before no DATA check */
	pData = (UCHAR *)pHeader;

	if (wdev->rx_ps_handle)
		wdev->rx_ps_handle(pAd, pRxBlk);

#ifdef A4_CONN
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_WDS))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	/*
		update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	*/
	pData = (UCHAR *)pHeader;

	/* 1. skip 802.11 HEADER */
	pData += hdr_len;
	pRxBlk->DataSize -= hdr_len;

	/* 2. QOS */
	if (pFmeCtrl->SubType & 0x08)
	{
		UserPriority = *(pData) & 0x0f;

#ifdef CONFIG_AP_SUPPORT

		/* count packets priroity more than BE */
		detect_wmm_traffic(pAd, UserPriority, 0);
#endif /* CONFIG_AP_SUPPORT */

		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
			pCounter->RxAMSDUCount.u.LowPart++;
		}

#ifdef DOT11_N_SUPPORT
		if (pRxInfo->BA)
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);

			/* incremented by the number of MPDUs */
			/* received in the A-MPDU when an A-MPDU is received. */
			pCounter->MPDUInReceivedAMPDUCount.u.LowPart ++;
		}
		else
		{
			if (pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
				RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
		}
#endif /* DOT11_N_SUPPORT */

		/* skip QOS contorl field */
		pData += 2;
		pRxBlk->DataSize -= 2;
	}
	pRxBlk->UserPriority = UserPriority;

	/*check if duplicate frame, ignore it and then drop*/
	if(rx_chk_duplicate_frame(pAd,pRxBlk) == NDIS_STATUS_FAILURE)
	{
		DBGPRINT(RT_DEBUG_INFO, ("%s(): duplication frame, drop it!\n", __FUNCTION__));
#ifdef NEW_IXIA_METHOD
		rdrop_reson = DROP_DUP_FRAME;
#endif
		goto drop;
	}


	/* 3. Order bit: A-Ralink or HTC+ */
	if (pFmeCtrl->Order)
	{
#ifdef AGGREGATION_SUPPORT
		// TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
		{
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		}
		else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef DOT11_N_SUPPORT

			/* skip HTC control field */
			pData += 4;
			pRxBlk->DataSize -= 4;
#endif /* DOT11_N_SUPPORT */
		}
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (pFmeCtrl->SubType & 0x04) /* bit 2 : no DATA */
	{
		DBGPRINT(RT_DEBUG_INFO, ("%s(): Null/QosNull frame!\n", __FUNCTION__));

		drop_err = FALSE;
		goto drop;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD)
	{
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}


	pRxBlk->pData = pData;

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1)
	   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	*/
	if (pAd->chipCap.hif_type != HIF_MT)
	{
	if ((pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0))
	{
#ifdef HDR_TRANS_SUPPORT
		if ( pRxBlk->bHdrRxTrans) {
			status = RTMPSoftDecryptionAction(pAd,
								 	(PUCHAR)pHeader,
									 UserPriority,
									 &pEntry->PairwiseKey,
								 	 pRxBlk->pTransData + 14,
									 &(pRxBlk->TransDataSize));
		}
		else
#endif /* HDR_TRANS_SUPPORT */
		{
			CIPHER_KEY *pSwKey = &pEntry->PairwiseKey;


			status = RTMPSoftDecryptionAction(pAd,
								 	(PUCHAR)pHeader,
									 UserPriority,
									 pSwKey,
								 	 pRxBlk->pData,
									 &(pRxBlk->DataSize));
		}

		if ( status != NDIS_STATUS_SUCCESS)
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			return;
		}
		/* Record the Decrypted bit as 1 */
		pRxInfo->Decrypted = 1;
	}
	}
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */



#ifdef DOT11_N_SUPPORT
#ifndef DOT11_VHT_AC
#ifndef WFA_VHT_PF
// TODO: shiang@PF#2, is this atheros protection still necessary here????
	/* check Atheros Client */
	if (!pEntry->bIAmBadAtheros && (pFmeCtrl->Retry) &&
		(pRxBlk->rx_rate.field.MODE < MODE_VHT) &&
		(pRxInfo->AMPDU == 1) && (pAd->CommonCfg.bHTProtect == TRUE)
	)
	{
		if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
			RTMP_UPDATE_PROTECT(pAd, 8 , ALLN_SETPROTECT, FALSE, FALSE);
		pEntry->bIAmBadAtheros = TRUE;

		if (pEntry->WepStatus != Ndis802_11WEPDisabled)
			pEntry->MpduDensity = 6;
	}
#endif /* WFA_VHT_PF */
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
   	DBGPRINT(RT_DEBUG_INFO, ("Rcv packet to IF(ra%d)\n", wdev_idx));

	/* update rssi sample */
	Update_Rssi_Sample(pAd, &pEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
	pEntry->NoDataIdleCount = 0;
	// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;


	if (pRxInfo->U2M)
	{
#ifdef CONFIG_AP_SUPPORT
		Update_Rssi_Sample(pAd, &pAd->ApCfg.RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
		pAd->ApCfg.NumOfAvgRssiSample ++;
#endif /* CONFIG_AP_SUPPORT */

		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);


#ifdef DBG_DIAGNOSE
		if (pAd->DiagStruct.inited) {
			struct dbg_diag_info *diag_info;
			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
#ifdef DBG_RX_MCS
			if (pRxBlk->rx_rate.field.MODE == MODE_HTMIX ||
				pRxBlk->rx_rate.field.MODE == MODE_HTGREENFIELD) {
				if (pRxBlk->rx_rate.field.MCS < MAX_MCS_SET)
					diag_info->RxMcsCnt_HT[pRxBlk->rx_rate.field.MCS]++;
			}
#ifdef DOT11_VHT_AC
			if (pRxBlk->rx_rate.field.MODE == MODE_VHT) {
				INT mcs_idx = ((pRxBlk->rx_rate.field.MCS >> 4) * 10) +
								(pRxBlk->rx_rate.field.MCS & 0xf);
				if (mcs_idx < MAX_VHT_MCS_SET)
					diag_info->RxMcsCnt_VHT[mcs_idx]++;
			}
#endif /* DOT11_VHT_AC */
#endif /* DBG_RX_MCS */
		}
#endif /* DBG_DIAGNOSE */
	}

	wdev->LastSNR0 = (UCHAR)(pRxBlk->rx_signal.raw_snr[0]);
	wdev->LastSNR1 = (UCHAR)(pRxBlk->rx_signal.raw_snr[1]);
#ifdef DOT11N_SS3_SUPPORT
	wdev->LastSNR2 = (UCHAR)(pRxBlk->rx_signal.raw_snr[2]);
#endif /* DOT11N_SS3_SUPPORT */
	pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
	pEntry->freqOffsetValid = TRUE;


	if (!((pHeader->Frag == 0) && (pFmeCtrl->MoreFrag == 0)))
	{
		/*
			re-assemble the fragmented packets, return complete
			frame (pRxPacket) or NULL
		*/
		bFragment = TRUE;
		pRxPacket = RTMPDeFragmentDataFrame(pAd, pRxBlk);
	}

	if (pRxPacket)
	{
		/*
			process complete frame which encrypted by TKIP,
			Minus MIC length and calculate the MIC value
		*/
		if (bFragment && (pFmeCtrl->Wep) && (pEntry->WepStatus == Ndis802_11TKIPEnable))
		{
			pRxBlk->DataSize -= 8;
			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return;
		}

#ifdef CONFIG_AP_SUPPORT
		pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
		pEntry->OneSecRxBytes += pRxBlk->MPDUtotalByteCnt;
		INC_COUNTER64(pEntry->RxPackets);
#endif /* CONFIG_AP_SUPPORT */
        pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

#ifdef IKANOS_VX_1X0
		RTMP_SET_PACKET_WDEV(pRxPacket, wdev_idx);
#endif /* IKANOS_VX_1X0 */

#ifdef MAC_REPEATER_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
			RTMP_SET_PACKET_WCID(pRxPacket, pRxBlk->wcid);
#endif /* MAC_REPEATER_SUPPORT */

		rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
	}
	else
	{
		/*
			just return because RTMPDeFragmentDataFrame() will release rx
			packet, if packet is fragmented
		*/
	}

    DBGPRINT(RT_DEBUG_INFO, ("<--%s(): Success!\n", __FUNCTION__));

	return;

drop:
#ifdef CONFIG_AP_SUPPORT
	/* Increase received error packet counter per BSS */
	if (pFmeCtrl->FrDs == 0 &&
		pRxInfo->U2M &&
		pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
	{
		pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxDropCount ++;
		if (drop_err == TRUE)
			pAd->ApCfg.MBSSID[pRxBlk->bss_idx].RxErrorCount ++;
	}
#endif /* CONFIG_AP_SUPPORT */
	//DBGPRINT(RT_DEBUG_OFF, ("%s():release packet!\n", __FUNCTION__));

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
#ifdef NEW_IXIA_METHOD
	/*RX Drop*/
	pAd->tr_ststic.rx[rdrop_reson]++;
#endif
    DBGPRINT(RT_DEBUG_INFO, ("<--%s(): Drop!\n", __FUNCTION__));

	return;
}


/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/
#undef MAX_RX_PROCESS_CNT
#define MAX_RX_PROCESS_CNT	(256)

BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI
	UINT32 RxProcessed = 0; 
#endif
    UINT32 RxPending = 0;
	BOOLEAN bReschedule = FALSE;
	PNDIS_PACKET pRxPacket;
	HEADER_802_11 *pHeader;
	RX_BLK rxblk, *pRxBlk;
#ifdef AIR_MONITOR
	
		RXINFO_STRUC *pRxInfo=NULL;
		MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* AIR_MONITOR */
	INT status = 0;

	DBGPRINT(RT_DEBUG_FPGA, ("-->%s():\n", __FUNCTION__));

#ifdef LINUX
#endif /* LINUX */

	/* process whole rx ring */
	while (1)
	{
		if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
				(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
			)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
#ifdef UAPSD_SUPPORT
		UAPSD_TIMING_RECORD_INDEX(RxProcessed);
#endif /* UAPSD_SUPPORT */

		if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		{
			bReschedule = TRUE;
			break;
		}

#ifdef UAPSD_SUPPORT
		/* static rate also need NICUpdateFifoStaCounters() function. */
		/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)) */
		UAPSD_MR_SP_SUSPEND(pAd);
#endif /* UAPSD_SUPPORT */

		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}
#endif /* RTMP_MAC_PCI */

		/*
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/

		NdisZeroMemory(&rxblk,sizeof(RX_BLK));

		pRxBlk = &rxblk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 0);

#ifdef RTMP_SDIO_SUPPORT
		/* 
			PCI case FIX Memory Leakage:
			Actually almost F/W RAM code cmd Response is from Ring1, But cmd resp from F/W ROM code, it's From Ring0, so we need to free this packet here for PCI case.
		*/
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_CMD_RSP)) {
			RX_BLK_CLEAR_FLAG(pRxBlk, fRX_CMD_RSP);
			continue;
		}
#endif /* RTMP_SDIO_SUPPORT */

      	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_RETRIEVE)) {
         	RX_BLK_CLEAR_FLAG(pRxBlk, fRX_RETRIEVE);
         	continue;
      	}
		
		if (pRxPacket == NULL)
			break;

		/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		{
			if (pRxPacket)
			{
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
#ifdef NEW_IXIA_METHOD
				/*RX Drop*/
				pAd->tr_ststic.rx[DROP_RING_FULL]++;
#endif
				continue;
			}
		}

		/* get rx descriptor and data buffer */
		pHeader = rxblk.pHeader;
#ifdef AIR_MONITOR
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
					pRxInfo = rxblk.pRxInfo;
					if(pRxInfo != NULL)
					{
					   if (pAd->MntEnable && !pRxInfo->CipherErr && !pRxInfo->Crc)
					   {
						if (VALID_WCID(pRxBlk->wcid))
							pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
							else
									pEntry = NULL;
						if (pEntry != NULL)
						{
							if(IS_ENTRY_MONITOR(pEntry))		
							{
							   Air_Monitor_Pkt_Report_Action(pAd, pRxBlk->wcid, pRxBlk);
								   {
								   if (!((pHeader->FC.Type == FC_TYPE_MGMT) && (pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
									{
										   RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
									   continue;
									}
							   }
							}
						}
						}
					}
				}
#endif /* CONFIG_AP_SUPPORT */
#endif /* AIR_MONITOR */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
                   if ((rxblk.DataSize == 0) && (pRxPacket)) {
                      RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
                      DBGPRINT(RT_DEBUG_INFO, ("%s():Packet Length is zero!\n", __FUNCTION__));
#ifdef NEW_IXIA_METHOD
					  /*RX Drop*/
					  pAd->tr_ststic.rx[DROP_DATA_SIZE]++;
#endif
	              continue;
		   }

		}
#endif /* MT_MAC */

#ifdef RLT_MAC
#ifdef CONFIG_ANDES_SUPPORT
#ifdef RTMP_PCI_SUPPORT
		if ((pAd->chipCap.hif_type == HIF_RLT) &&
			(pRxBlk->pRxFceInfo->info_type == CMD_PACKET))
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s: Receive command packet.\n", __FUNCTION__));
			continue;
		}
#endif /* RTMP_PCI_SUPPORT */
#endif /* CONFIG_ANDES_SUPPORT */

		// TODO: shiang-6590, handle packet from other ports
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			RXINFO_STRUC *pRxInfo = rxblk.pRxInfo;
			RXFCE_INFO *pFceInfo = rxblk.pRxFceInfo;

#ifdef HDR_TRANS_SUPPORT
			if ((pFceInfo->info_type == 0) && (pFceInfo->pkt_80211 == 0) &&
				(pRxInfo->hdr_trans_ip_sum_err == 1))
			{
				pRxBlk->bHdrRxTrans = pRxBlk->pRxInfo->hdr_trans_ip_sum_err;
				pRxBlk->bHdrVlanTaged = pRxBlk->pRxInfo->vlan_taged_tcp_sum_err;
				if (IS_MT7601(pAd))
					pRxBlk->pTransData = (UCHAR *)pHeader +  38; /* 36 byte + 802.3 padding */
				else
					pRxBlk->pTransData = (UCHAR *)pHeader +  36; /* 36 byte RX Wifi Info */
				pRxBlk->TransDataSize = pRxBlk->MPDUtotalByteCnt;
			} else
#endif /* HDR_TRANS_SUPPORT */
			if ((pFceInfo->info_type != 0) || (pFceInfo->pkt_80211 != 1))
			{
				RXD_STRUC *pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];

				DBGPRINT(RT_DEBUG_OFF, ("==>%s(): GetFrameFromOtherPorts!\n", __FUNCTION__));
				hex_dump("hw_rx_info", &rxblk.hw_rx_info[0], sizeof(rxblk.hw_rx_info));
				DBGPRINT(RT_DEBUG_TRACE, ("Dump the RxD, RxFCEInfo and RxInfo:\n"));
				hex_dump("RxD", (UCHAR *)pRxD, sizeof(RXD_STRUC));
#ifdef RTMP_MAC_PCI
				dump_rxd(pAd, pRxD);
#endif /* RTMP_MAC_PCI */
				dumpRxFCEInfo(pAd, pFceInfo);
				dump_rxinfo(pAd, pRxInfo);
				hex_dump("RxFrame", (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket), (pFceInfo->pkt_len));
				DBGPRINT(RT_DEBUG_OFF, ("<==\n"));
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				continue;
			}
		}
#endif /* RLT_MAC */

//+++Add by shiang for debug
		if (!pRxBlk->pRxInfo) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): pRxBlk->pRxInfo is NULL!\n", __FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
#ifdef NEW_IXIA_METHOD
			/*RX Drop*/
			pAd->tr_ststic.rx[DROP_INFO_NULL]++;
#endif
			continue;
		}
//---Add by shiang for debug

#ifdef CONFIG_ATE
		if (ATE_ON(pAd)) {
			ate_rx_done_handle(pAd, pRxBlk);

			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif /* CONFIG_ATE */

#ifdef MIXMODE_SUPPORT
		if (MONITOR_ON(pAd) && pAd->MixModeCtrl.current_monitor_mode != MIX_MODE_OFF) {
			if (pAd->MixModeCtrl.current_monitor_mode == MIX_MODE_FULL) {
				MixModeProcessData(pAd, pRxBlk);
			}
		}
#endif	/* MIXMODE_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
		if (MONITOR_ON(pAd) && pAd->monitor_ctrl.current_monitor_mode != MONITOR_MODE_OFF)
		{
			PNDIS_PACKET	pCopyPacket;
			PNDIS_PACKET    pTmpRxPacket;
			struct wifi_dev *wdev;
			
			UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(rxblk.pRxPacket);
			
			ASSERT(wdev_idx < WDEV_NUM_MAX);
			if (wdev_idx >= WDEV_NUM_MAX) {
				DBGPRINT(RT_DEBUG_ERROR, ("%s():invalid wdev_idx(%d)!\n", __FUNCTION__, wdev_idx));
				RELEASE_NDIS_PACKET(pAd, rxblk.pRxPacket, NDIS_STATUS_FAILURE);
				return 0;
			}
			
			wdev = pAd->wdev_list[wdev_idx];

			if(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_REGULAR_RX)
			{	
				USHORT Data;
				PFRAME_CONTROL FC;
				NdisMoveMemory((PUCHAR)(&Data),(PUCHAR)pHeader,2);
#ifdef RT_BIG_ENDIAN
				Data = SWAP16(Data);
#endif					
				FC = (PFRAME_CONTROL)(&Data);

				/* only report Probe_Req */
				if((FC->Type == FC_TYPE_MGMT) && (FC->SubType == SUBTYPE_PROBE_REQ))	
		    	{					
					pTmpRxPacket = rxblk.pRxPacket;
					pCopyPacket = CopyPacket(wdev->if_dev, rxblk.pRxPacket, 0);
					rxblk.pRxPacket = pCopyPacket;
					STA_MonPktSend(pAd, &rxblk);
					
					rxblk.pRxPacket = pTmpRxPacket;	
				}
			}
			else if(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_FULL)
			{
					pTmpRxPacket = rxblk.pRxPacket;
					pCopyPacket = CopyPacket(wdev->if_dev, rxblk.pRxPacket, 0);
					rxblk.pRxPacket = pCopyPacket;
					STA_MonPktSend(pAd, &rxblk);
					rxblk.pRxPacket = pTmpRxPacket;	
			}

		}		
#endif /* CONFIG_SNIFFER_SUPPORT */

#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (UCHAR *)pHeader, DIR_READ, TRUE);
		// TODO: shiang-usw, following endian swap move the GetPacketFromRxRing()
#endif /* RT_BIG_ENDIAN */

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI)
			dbQueueEnqueueRxFrame(GET_OS_PKT_DATAPTR(pRxPacket),
										(UCHAR *)pHeader,
										pAd->CommonCfg.DebugFlags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */




		/* Increase Total receive byte counter after real data received no mater any error or not */
		pAd->RalinkCounters.ReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.OneSecReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.RxCount++;
		pAd->RalinkCounters.OneSecRxCount++;
#ifdef NEW_IXIA_METHOD
		if (IS_EXPECTED_LENGTH(GET_OS_PKT_LEN(pRxPacket) - 16)) {
			rx_pkt_from_hw++;
			rx_pkt_len = GET_OS_PKT_LEN(pRxPacket) - 16;
			rxpktdetect2s++;
		}
#endif
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);
#endif /* STATS_COUNT_SUPPORT */



		/* Check for all RxD errors */
		status = rtmp_chk_rx_err(pAd, pRxBlk, pHeader);
		if (status != NDIS_STATUS_SUCCESS) {
#ifdef NEW_IXIA_METHOD
			/*RX Drop*/
			pAd->tr_ststic.rx[DROP_RXD_ERROR]++;
#endif
			if (status == NDIS_STATUS_INVALID_DATA) {
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				continue;
			} else {
				pAd->Counters8023.RxErrors++;
#ifdef APCLI_SUPPORT
				/* When root AP is Open WEP,
				 * it will cause a fake connection state if user keys in wrong password.
				 */
				if (pHeader->FC.Wep == 1)
					ApCliRxOpenWEPCheck(pAd, pRxBlk, FALSE);
#endif /* APCLI_SUPPORT */
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				DBGPRINT(RT_DEBUG_TRACE, ("%s(): CheckRxError!\n", __func__));
				continue;
			}
		}
#ifdef APCLI_SUPPORT
		/* When root AP is Open WEP, it will cause a fake connection state if user keys in wrong password. */
		if(pHeader->FC.Wep == 1)
			ApCliRxOpenWEPCheck(pAd,pRxBlk,TRUE);
#endif /* APCLI_SUPPORT */


		// TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!!

		switch (pHeader->FC.Type)
		{
			case FC_TYPE_DATA:
				dev_rx_data_frm(pAd, &rxblk);
				break;

			case FC_TYPE_MGMT:
				dev_rx_mgmt_frm(pAd, &rxblk);
				break;

			case FC_TYPE_CNTL:
				dev_rx_ctrl_frm(pAd, &rxblk);
				break;

			default:
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				break;
		}
	}

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* dont remove the function or UAPSD will fail */
		UAPSD_MR_SP_RESUME(pAd);
		UAPSD_SP_CloseInRVDone(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

	return bReschedule;
}

