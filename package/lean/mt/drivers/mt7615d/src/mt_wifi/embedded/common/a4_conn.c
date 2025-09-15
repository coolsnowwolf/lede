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
    a4_conn.c

    Abstract:
    This is A4 connection function used to process those 4-addr of connected APClient or STA.
    Used by MWDS and MAP feature

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef A4_CONN
#include "rt_config.h"

BOOLEAN a4_interface_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
)
{
	BSS_STRUCT *mbss = NULL;
	BOOLEAN add_inf = FALSE;

	if (is_ap) {
		if (if_index >= HW_BEACON_MAX_NUM)
			return FALSE;

		mbss = &adapter->ApCfg.MBSSID[if_index];
		if (mbss->a4_init == 0) {
			NdisAllocateSpinLock(adapter, &mbss->a4_entry_lock);
			DlListInit(&mbss->a4_entry_list);
			RoutingTabInit(adapter, if_index, ROUTING_ENTRY_A4);
			add_inf = TRUE;
		}
		mbss->a4_init |= (1 << a4_type);
	}
#ifdef APCLI_SUPPORT
	else {
		PAPCLI_STRUCT apcli_entry;

		if (if_index >= MAX_APCLI_NUM)
			return FALSE;
		apcli_entry = &adapter->ApCfg.ApCliTab[if_index];
		if (apcli_entry->a4_init == 0)
			add_inf = TRUE;
		apcli_entry->a4_init |= (1 << a4_type);
	}
#else
	else
		return FALSE;
#endif

	if (add_inf) {
		if (adapter->a4_interface_count == 0)
			MtCmdSetA4Enable(adapter, HOST2CR4, TRUE);
		adapter->a4_interface_count++;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("a4_interface_init a4_interface_count: %d\n",
			adapter->a4_interface_count));
	return TRUE;
}


BOOLEAN a4_interface_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap,
	IN UCHAR a4_type
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL, pConnEntryTmp = NULL;
	PDL_LIST a4_entry_list = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN delete_inf = FALSE;

	if (is_ap) {
		if (if_index >= HW_BEACON_MAX_NUM)
			return FALSE;

		mbss = &adapter->ApCfg.MBSSID[if_index];
		if (mbss->a4_init) {
			a4_entry_list = &mbss->a4_entry_list;
			DlListForEachSafe(a4_entry, pConnEntryTmp, a4_entry_list, A4_CONNECT_ENTRY, List) {
				if (a4_entry) {
					entry = &adapter->MacTab.Content[a4_entry->wcid];
					a4_ap_peer_disable(adapter, entry, a4_type);
				}
			}

			mbss->a4_init &= ~(1 << a4_type);
			if (!mbss->a4_init) {
				NdisFreeSpinLock(&mbss->a4_entry_lock);
				RoutingTabDestory(adapter, if_index, ROUTING_ENTRY_A4);
				delete_inf = TRUE;
			}
		}
	}
#ifdef APCLI_SUPPORT
	else {
		PAPCLI_STRUCT apcli_entry;

		if (if_index >= MAX_APCLI_NUM)
			return FALSE;
		apcli_entry = &adapter->ApCfg.ApCliTab[if_index];
		if (apcli_entry->a4_init) {
			apcli_entry->a4_init &= ~(1 << a4_type);
			if (!apcli_entry->a4_init)
				delete_inf = TRUE;
		}
	}
#else
	else
		return FALSE;
#endif

	if (delete_inf) {
		adapter->a4_interface_count--;
		if (adapter->a4_interface_count == 0)
			MtCmdSetA4Enable(adapter, HOST2CR4, FALSE);

	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("a4_interface_init a4_interface_count: %d\n",
			adapter->a4_interface_count));
	return TRUE;
}


INT a4_get_entry_count(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
)
{
	int count = 0;
	BSS_STRUCT *mbss = NULL;

	if (if_index >= HW_BEACON_MAX_NUM)
		return 0;

	mbss = &adapter->ApCfg.MBSSID[if_index];

	if (!mbss->a4_init)
		return 0;

	count = DlListLen(&mbss->a4_entry_list);
	return count;
}


BOOLEAN a4_lookup_entry_by_wcid(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UCHAR wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	BOOLEAN found = FALSE;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry &&
			a4_entry->valid &&
			(a4_entry->wcid == wcid)) {
			found = TRUE;
			break;
		}
	}
	return found;
}


BOOLEAN a4_lookup_entry_by_addr(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	BSS_STRUCT *mbss = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN found = FALSE;

	if ((a4_get_entry_count(adapter, if_index) == 0) ||
		(mac_addr == NULL))
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry && a4_entry->valid && VALID_WCID(a4_entry->wcid)) {
			entry = &adapter->MacTab.Content[a4_entry->wcid];

			if (MAC_ADDR_EQUAL(mac_addr, entry->Addr)) {
				found = TRUE;
				break;
			}
		}
	}
	return found;
}


VOID a4_add_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UCHAR wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;

	if (a4_lookup_entry_by_wcid(adapter, if_index, wcid))
		return;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	os_alloc_mem(adapter, (UCHAR **)&a4_entry, sizeof(A4_CONNECT_ENTRY));

	if (a4_entry) {
		NdisZeroMemory(a4_entry, sizeof(A4_CONNECT_ENTRY));
		a4_entry->valid = 1;
		a4_entry->wcid = wcid;
		RTMP_SEM_LOCK(&mbss->a4_entry_lock);
		DlListAddTail(&mbss->a4_entry_list, &a4_entry->List);
		RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: Fail to alloc memory for pNewConnEntry\n", __func__));
}


VOID a4_delete_entry(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UCHAR wcid
)
{
	BSS_STRUCT *mbss = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PDL_LIST a4_entry_list = NULL;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	mbss = &adapter->ApCfg.MBSSID[if_index];
	a4_entry_list = &mbss->a4_entry_list;
	RTMP_SEM_LOCK(&mbss->a4_entry_lock);
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry &&
			a4_entry->valid &&
			(a4_entry->wcid == wcid)) {
			DlListDel(&a4_entry->List);
			os_free_mem(a4_entry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&mbss->a4_entry_lock);
}


VOID a4_proxy_delete(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr
)
{
	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	RoutingTabSetOneFree(adapter, if_index, mac_addr, ROUTING_ENTRY_A4);
}

BOOLEAN a4_get_dst_ip(void *pkt, unsigned char *ip)
{
	UCHAR *pSrcBuf;
	UINT16 TypeLen;
	UINT32 type_len_idx = 12;
	UCHAR *ip_header = NULL;

	pSrcBuf = pkt;
	TypeLen = (pSrcBuf[type_len_idx] << 8) | pSrcBuf[type_len_idx + 1];
	while (TypeLen == ETH_TYPE_VLAN) {
		type_len_idx += 2;
		TypeLen = (pSrcBuf[type_len_idx] << 8) | pSrcBuf[type_len_idx + 1];
	}
	if (TypeLen == ETH_TYPE_IPv4) {
		ip_header = &pSrcBuf[type_len_idx + 2];
		NdisCopyMemory(ip, ip_header + 16, 4);
		return TRUE;
	}
	return FALSE;
}
BOOLEAN a4_proxy_lookup(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PUCHAR mac_addr,
	IN BOOLEAN update_alive_time,
	IN BOOLEAN is_rx,
	OUT UCHAR *wcid
)
{

	*wcid = 0;
	if (a4_get_entry_count(adapter, if_index) == 0) {
		return FALSE;
	}
	if (RoutingTabLookup(adapter, if_index, mac_addr, update_alive_time, wcid) != NULL)
		return TRUE;
	else {
		return FALSE;
	}
}


VOID a4_proxy_update(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN UCHAR wcid,
	IN PUCHAR mac_addr,
	IN UINT32 ip /* ARP Sender IP*/
)
{
	UCHAR proxy_wcid = 0;
	BOOLEAN found = FALSE;
	PROUTING_ENTRY routing_entry = NULL;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	if (!VALID_WCID(wcid) || !mac_addr)
		return;

	routing_entry = RoutingTabLookup(adapter, if_index, mac_addr, TRUE, &proxy_wcid);
	found = (routing_entry != NULL) ? TRUE : FALSE;

	if (found) {
		if (ROUTING_ENTRY_TEST_FLAG(routing_entry, ROUTING_ENTRY_A4)) {
			/* Mean the target change to other ProxyAP */
			if (proxy_wcid != wcid) {
				RoutingTabSetOneFree(adapter, if_index, mac_addr, ROUTING_ENTRY_A4);
				routing_entry = NULL;
				found = FALSE;
			}
		} else {
			/* Assign A4 falg to this one if found. */
			SET_ROUTING_ENTRY(routing_entry, ROUTING_ENTRY_A4);
		}
	}

	if (!found) {
		/* Allocate a new one if not found. */
		routing_entry = RoutingTabGetFree(adapter, if_index);

		if (routing_entry) {
			SET_ROUTING_ENTRY(routing_entry, ROUTING_ENTRY_A4);
			RoutingEntrySet(adapter, if_index, wcid, mac_addr, routing_entry);
		}
	}

	if (routing_entry != NULL && (ip != 0)) /* ARP Sender IP Update*/
		RoutingTabARPLookupUpdate(adapter, if_index, routing_entry, ip);
}


VOID a4_proxy_maintain(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index
)
{
	if (a4_get_entry_count(adapter, if_index) == 0)
		return;

	RoutingTabMaintain(adapter, if_index);
}


INT a4_hard_transmit(
	IN PRTMP_ADAPTER adapter,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt
)
{
	UINT pkt_len;
	INT ret = NDIS_STATUS_SUCCESS;
	UCHAR wcid = RESERVED_WCID;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	/* Precautionary measure */
	if (RTMP_GET_PACKET_WCID(pkt) == 0) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("invalid wcid=0 in a4_hard_transmit\n"));
		RELEASE_NDIS_PACKET(adapter, pkt, NDIS_STATUS_FAILURE);
		return ret;
	}

	if (IS_ASIC_CAP(adapter, fASIC_CAP_MCU_OFFLOAD))
		ret = ops->fp_send_data_pkt(adapter, wdev, pkt);
	else {
		wcid = RTMP_GET_PACKET_WCID(pkt);
		pkt_len = GET_OS_PKT_LEN(pkt);
		if ((pkt_len <= 14)
			|| (!RTMPCheckEtherType(adapter, pkt, &adapter->MacTab.tr_entry[wcid], wdev))) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Pkt Len/Ethernet Check Fail\n"));
			RELEASE_NDIS_PACKET(adapter, pkt, NDIS_STATUS_FAILURE);
			return ret;
		}
		ret = ops->send_data_pkt(adapter, wdev, pkt);
	}
	return ret;
}


PNDIS_PACKET a4_clone_packet(
	IN PRTMP_ADAPTER adapter,
	IN PNET_DEV if_dev,
	IN PNDIS_PACKET pkt
)
{
	struct sk_buff *skb = NULL;
	PNDIS_PACKET pkt_clone = NULL;

	skb = skb_copy(RTPKT_TO_OSPKT(pkt), GFP_ATOMIC);

	if (skb) {
		skb->dev = if_dev;
		pkt_clone = OSPKT_TO_RTPKT(skb);
	}

	return pkt_clone;
}

void a4_send_clone_pkt(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN PNDIS_PACKET pkt,
	IN PUCHAR exclude_mac_addr
)
{
	struct wifi_dev *wdev = NULL;
	PNDIS_PACKET pkt_clone = NULL;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PROUTING_ENTRY routing_entry = NULL;
	PMAC_TABLE_ENTRY entry = NULL;
	BOOLEAN found = FALSE;
	UCHAR wcid = 0;
	struct wifi_dev *ap_wdev = NULL;

	if (!pkt)
		return;

	if ((a4_get_entry_count(adapter, if_index) > 0)) {
		ap_wdev = &adapter->ApCfg.MBSSID[if_index].wdev;

#if (defined(IGMP_SNOOP_SUPPORT) && defined(IGMP_TX_QUERY_HOLD))
	if (ap_wdev->IgmpSnoopEnable) {	/* If snooping enabled*/
		UCHAR *pDestAddr = NULL;

		pDestAddr = GET_OS_PKT_DATAPTR(pPacket);

		if (IS_MULTICAST_MAC_ADDR(pDestAddr)) {
			PUCHAR pData = pDestAddr + 12;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			/* Check whether membership query sent by some other device.*/
			/* If detected, set internal query Hold duration to avoid flooding in network*/

			if (protoType == ETH_P_IP) {
				if (isIGMPquery(adapter, pDestAddr, pData)) {
					adapter->ApCfg.MBSSID[if_index].IgmpQueryHoldTick = QUERY_HOLD_PERIOD;
				}
			} else if (protoType == ETH_P_IPV6) {
				if (isMLDquery(adapter, pDestAddr, pData)) {
					adapter->ApCfg.MBSSID[if_index].MldQueryHoldTick = QUERY_HOLD_PERIOD;
				}
			}
		}
	}
#endif



		if (exclude_mac_addr) {
			routing_entry = RoutingTabLookup(adapter, if_index, exclude_mac_addr, FALSE, &wcid);

			if (routing_entry && ROUTING_ENTRY_TEST_FLAG(routing_entry, ROUTING_ENTRY_A4))
				found = TRUE;
			else
				found = FALSE;
		}

		a4_entry_list = &adapter->ApCfg.MBSSID[if_index].a4_entry_list;
		DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
			if (a4_entry && a4_entry->valid && VALID_WCID(a4_entry->wcid)) {
				if (found && (wcid == a4_entry->wcid))
					continue;

				entry = &adapter->MacTab.Content[a4_entry->wcid];
				wdev = entry->wdev;

				pkt_clone = a4_clone_packet(adapter, wdev->if_dev, pkt);
				if (pkt_clone == NULL) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("%s: Fail to alloc memory for pPacketClone\n", __func__));
					return;
				}

				RTMP_SET_PACKET_WCID(pkt_clone, entry->wcid);
				RTMP_SET_PACKET_WDEV(pkt_clone, wdev->wdev_idx);
				RTMP_SET_PACKET_MOREDATA(pkt_clone, FALSE);
				/*RTMP_SET_PACKET_QUEIDX(pPacketClone, QID_AC_BE);*/
				a4_hard_transmit(adapter, wdev, pkt_clone);
			}
		}
	}
}


BOOLEAN a4_ap_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{
	BSS_STRUCT *mbss = NULL;
	UCHAR if_index;
	CMD_WTBL_HDR_TRANS_T wtbl_hdr_trans;

	if (!entry || !IS_ENTRY_CLIENT(entry))
		return FALSE;

	if_index = entry->func_tb_idx;
	if (if_index >= HW_BEACON_MAX_NUM)
		return FALSE;

	mbss = &adapter->ApCfg.MBSSID[if_index];
#ifdef WSC_AP_SUPPORT
	if (mbss &&
		(mbss->WscControl.WscConfMode != WSC_DISABLE) &&
		(mbss->WscControl.bWscTrigger == TRUE))
		return FALSE;
#endif /* WSC_AP_SUPPORT */

	if (IS_ENTRY_A4(entry) == FALSE) {
		/* To check and remove entry which is created from another side. */
		a4_proxy_delete(adapter, if_index, entry->Addr);

		os_zero_mem(&wtbl_hdr_trans, sizeof(CMD_WTBL_HDR_TRANS_T));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("Enabled A4 for entry:%02x-%02x-%02x-%02x-%02x-%02x\n",
					PRINT_MAC(entry->Addr)));
		wtbl_hdr_trans.u2Tag = WTBL_HDR_TRANS;
		wtbl_hdr_trans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);
		wtbl_hdr_trans.ucTd = 1;
		wtbl_hdr_trans.ucFd = 1;
		wtbl_hdr_trans.ucDisRhtr = 0;
		if (NDIS_STATUS_SUCCESS != CmdExtWtblUpdate(adapter,
													entry->wcid,
													SET_WTBL,
													&wtbl_hdr_trans,
													sizeof(CMD_WTBL_HDR_TRANS_T)))
			return FALSE;

		a4_add_entry(adapter, if_index, entry->wcid);
	}

	if (type > GET_ENTRY_A4(entry)) {
		SET_ENTRY_A4(entry, type);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SET_A4_ENTRY type:%d OK!\n", type));
	}

	return TRUE;
}


BOOLEAN a4_ap_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{
	UCHAR if_index;

	if (!entry || !IS_ENTRY_CLIENT(entry))
		return FALSE;

	if_index = entry->func_tb_idx;
	if (if_index >= HW_BEACON_MAX_NUM)
		return FALSE;

	if (type == GET_ENTRY_A4(entry)) {
		SET_ENTRY_A4(entry, A4_TYPE_NONE);
		a4_delete_entry(adapter, if_index, entry->wcid);
		RoutingTabSetAllFree(adapter, if_index, entry->wcid, ROUTING_ENTRY_A4);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("a4_ap_peer_disable: Disable A4 for entry : %02x-%02x-%02x-%02x-%02x-%02x\n",
				PRINT_MAC(entry->Addr)));
	}

	return TRUE;
}


#ifdef APCLI_SUPPORT
BOOLEAN a4_apcli_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PAPCLI_STRUCT apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{

	CMD_WTBL_HDR_TRANS_T wtbl_hdr_trans;

	if (!apcli_entry || !entry || !IS_ENTRY_APCLI(entry))
		return FALSE;

#ifdef WSC_AP_SUPPORT
	if (((apcli_entry->WscControl.WscConfMode != WSC_DISABLE) &&
			(apcli_entry->WscControl.bWscTrigger == TRUE)))
		return FALSE;
#endif /* WSC_AP_SUPPORT */

	if (IS_ENTRY_A4(entry) == FALSE) {

		os_zero_mem(&wtbl_hdr_trans, sizeof(CMD_WTBL_HDR_TRANS_T));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("a4_apcli_peer_enable enabled A4 for entry : %02x-%02x-%02x-%02x-%02x-%02x\n",
				PRINT_MAC(entry->Addr)));
		wtbl_hdr_trans.u2Tag = WTBL_HDR_TRANS;
		wtbl_hdr_trans.u2Length = sizeof(CMD_WTBL_HDR_TRANS_T);
		wtbl_hdr_trans.ucTd = 1;
		wtbl_hdr_trans.ucFd = 1;
		wtbl_hdr_trans.ucDisRhtr = 0;
		if (CmdExtWtblUpdate(adapter,
							entry->wcid,
							SET_WTBL,
							&wtbl_hdr_trans,
							sizeof(CMD_WTBL_HDR_TRANS_T)) == NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SET_A4_ENTRY OK!\n"));
		else
			return FALSE;
	}

	if (type > GET_ENTRY_A4(entry)) {
		SET_APCLI_A4(apcli_entry, type);
		SET_ENTRY_A4(entry, type);
	}
	return TRUE;
}


BOOLEAN a4_apcli_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PAPCLI_STRUCT apcli_entry,
	IN PMAC_TABLE_ENTRY entry,
	IN UCHAR type
)
{

	if (!apcli_entry || !entry)
		return FALSE;

	if (type == GET_ENTRY_A4(entry)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("a4_apcli_peer_disable, Disable A4 for entry:%02x-%02x-%02x-%02x-%02x-%02x\n",
				PRINT_MAC(entry->Addr)));
		SET_APCLI_A4(apcli_entry, A4_TYPE_NONE);
		SET_ENTRY_A4(entry, A4_TYPE_NONE);
	}

	return TRUE;
}
#endif /* APCLI_SUPPORT */


INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER adapter,
	IN  RTMP_STRING * arg)
{
	POS_COOKIE obj;
	UCHAR if_index;
	INT32 i, count = 0;
	UINT32 ip_addr = 0;
	ULONG now = 0, AliveTime = 0;
	PDL_LIST a4_entry_list = NULL;
	PA4_CONNECT_ENTRY a4_entry = NULL;
	PROUTING_ENTRY routing_entry = NULL, *routing_entry_list[ROUTING_POOL_SIZE];
	UCHAR *proxy_mac_addr = NULL, proxy_ip[64];

	obj = (POS_COOKIE) adapter->OS_Cookie;
	if_index = obj->ioctl_if;

	if ((obj->ioctl_if_type != INT_MBSSID) && (obj->ioctl_if_type != INT_MAIN))
		return FALSE;

	if (a4_get_entry_count(adapter, if_index) == 0)
		return TRUE;

	a4_entry_list = &adapter->ApCfg.MBSSID[if_index].a4_entry_list;
	NdisGetSystemUpTime(&now);
	DlListForEach(a4_entry, a4_entry_list, A4_CONNECT_ENTRY, List) {
		if (a4_entry && a4_entry->valid && VALID_WCID(a4_entry->wcid)) {
			count = 0;
			proxy_mac_addr = adapter->MacTab.Content[a4_entry->wcid].Addr;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("Proxy Mac: %02X:%02X:%02X:%02X:%02X:%02X\n",
					PRINT_MAC(proxy_mac_addr)));

			if (GetRoutingEntryAll(adapter, if_index, a4_entry->wcid, ROUTING_ENTRY_A4,
					ROUTING_POOL_SIZE, (ROUTING_ENTRY **)&routing_entry_list, &count)) {
				for (i = 0; i < count; i++) {
					routing_entry = (PROUTING_ENTRY)routing_entry_list[i];

					if (!routing_entry)
						continue;

					if (routing_entry->KeepAliveTime >= now)
						AliveTime = ((routing_entry->KeepAliveTime - now) / OS_HZ);
					else
						AliveTime = 0;

					if (routing_entry->IPAddr != 0) {
						ip_addr = routing_entry->IPAddr;
						sprintf(proxy_ip, "%d.%d.%d.%d",
								(ip_addr & 0xff),
								((ip_addr & (0xff << 8)) >> 8),
								((ip_addr & (0xff << 16)) >> 16),
								((ip_addr & (0xff << 24)) >> 24));
					} else
						strcpy(proxy_ip, "0.0.0.0");

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("MAC:%02X:%02X:%02X:%02X:%02X:%02X, IP:%s, AgeOut:%lus, Retry:(%d,%d)\n",
							PRINT_MAC(routing_entry->Mac), proxy_ip, AliveTime,
							routing_entry->Retry, ROUTING_ENTRY_MAX_RETRY));
				}

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Count = %d\n\n", count));
			}
		}
	}
	return TRUE;
}


INT Set_APProxy_Refresh_Proc(
	IN	PRTMP_ADAPTER adapter,
	IN	RTMP_STRING * arg)
{
	adapter->a4_need_refresh = TRUE;
	return TRUE;
}

#endif /* A4_CONN */
