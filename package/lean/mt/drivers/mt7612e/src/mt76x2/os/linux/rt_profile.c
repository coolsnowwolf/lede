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
	rt_profile.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
 
#include "rt_config.h"

#if defined(CONFIG_RA_HW_NAT) || defined(CONFIG_RA_HW_NAT_MODULE)
#include "../../../../../../../net/nat/hw_nat/ra_nat.h"
#include "../../../../../../../net/nat/hw_nat/frame_engine.h"
#endif


struct dev_type_name_map{
	INT type;
	PSTRING prefix[2];
};


#define SECOND_INF_MAIN_DEV_NAME	"rai"
#define SECOND_INF_MBSSID_DEV_NAME	"rai"
#define SECOND_INF_WDS_DEV_NAME		"wdsi"
#define SECOND_INF_APCLI_DEV_NAME	"apclii"
#define SECOND_INF_MESH_DEV_NAME	"meshi"
#define SECOND_INF_P2P_DEV_NAME		"p2pi"
#define SECOND_INF_MONITOR_DEV_NAME	"moni"

#define xdef_to_str(s)			def_to_str(s)
#define def_to_str(s)			#s

#define FIRST_AP_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860AP.dat"
#define FIRST_STA_PROFILE_PATH		"/etc/Wireless/RT2860/RT2860STA.dat"
#define FIRST_IF_SINGLE_SKU_PATH	"/etc/Wireless/RT2860/SingleSKU.dat"
#define FIRST_CHIP_ID			xdef_to_str(CONFIG_RT_FIRST_CARD)

#define SECOND_AP_PROFILE_PATH		"/etc/wireless/mt7615/mt7615.2.dat"
#define SECOND_STA_PROFILE_PATH		"/etc/Wireless/iNIC/iNIC_sta.dat"
#define SECOND_IF_SINGLE_SKU_PATH	"/etc/Wireless/iNIC/SingleSKU.dat"
#define SECOND_CHIP_ID			xdef_to_str(CONFIG_RT_SECOND_CARD)

static struct dev_type_name_map prefix_map[] =
{
	{INT_MAIN,		{INF_MAIN_DEV_NAME, SECOND_INF_MAIN_DEV_NAME}},
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
	{INT_MBSSID,		{INF_MBSSID_DEV_NAME, SECOND_INF_MBSSID_DEV_NAME}},
#endif /* MBSS_SUPPORT */
#ifdef APCLI_SUPPORT
	{INT_APCLI,		{INF_APCLI_DEV_NAME, SECOND_INF_APCLI_DEV_NAME}},
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	{INT_WDS,		{INF_WDS_DEV_NAME, SECOND_INF_WDS_DEV_NAME}},
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_SNIFFER_SUPPORT
	{INT_MONITOR,		{INF_MONITOR_DEV_NAME, SECOND_INF_MONITOR_DEV_NAME}},
#endif /* CONFIG_SNIFFER_SUPPORT */
	{0},
};

struct dev_id_name_map{
	INT chip_id;
	PSTRING chip_name;
};


VOID get_dev_config_idx(RTMP_ADAPTER *pAd)
{
	INT idx = 0;

#if defined (DRIVER_HAS_MULTI_DEV)
	INT first_card = 0, second_card = 0;
	static int probe_cnt = 1;

	A2Hex(first_card, FIRST_CHIP_ID);
	A2Hex(second_card, SECOND_CHIP_ID);

	if (first_card == second_card) {
		idx = probe_cnt;
		probe_cnt--;
	} else if (IS_MT76x2(pAd)) {
		UINT32 chip_id = (pAd->ChipID >> 16);
		if (chip_id == second_card)
			idx = 1;
	} else {
#ifdef RT6352
		if (IS_RT6352(pAd))
			idx = 0;
		else
#endif
		if (IS_RT8592(pAd))
			idx = 0;
		else if (IS_RT5392(pAd) || IS_MT76x0(pAd))
			idx = 1;
	}

	DBGPRINT(RT_DEBUG_OFF, ("chip_id1=0x%x, chip_id2=0x%x, pAd->MACVersion=0x%x, pAd->ChipID=0x%x, dev_idx=%d\n",
		first_card, second_card, pAd->MACVersion, pAd->ChipID, idx));
#endif

	pAd->dev_idx = idx;
}


UCHAR *get_dev_name_prefix(RTMP_ADAPTER *pAd, INT dev_type)
{
	struct dev_type_name_map *map;
	INT type_idx = 0, dev_idx = pAd->dev_idx;

	do {
		map = &prefix_map[type_idx];
		if (map->type == dev_type) {
			DBGPRINT(RT_DEBUG_OFF, ("%s(): dev_idx = %d, dev_name_prefix=%s\n",
						__FUNCTION__, dev_idx, map->prefix[dev_idx]));
			return map->prefix[dev_idx];
		}
		type_idx++;
	} while (prefix_map[type_idx].type != 0);

	return NULL;
}


static UCHAR *get_dev_profile(RTMP_ADAPTER *pAd)
{
	UCHAR *src = NULL;
#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			src = AP_PROFILE_PATH_RBUS;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			src = STA_PROFILE_PATH_RBUS;
		}
#endif /* CONFIG_STA_SUPPORT */
	}
	else
#endif /* RTMP_RBUS_SUPPORT */
	{	
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#if defined (DRIVER_HAS_MULTI_DEV)
			INT card_idx = pAd->dev_idx;

			if (card_idx == 0)
				src = FIRST_AP_PROFILE_PATH;
			else if (card_idx == 1)
				src = SECOND_AP_PROFILE_PATH;
			else
#endif
				src = AP_PROFILE_PATH;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#if defined (DRIVER_HAS_MULTI_DEV)
			INT card_idx = pAd->dev_idx;

			if (card_idx == 0)
				src = FIRST_STA_PROFILE_PATH;
			else if (card_idx == 1)
				src = SECOND_STA_PROFILE_PATH;
			else
#endif
				src = STA_PROFILE_PATH;
		}
#endif /* CONFIG_STA_SUPPORT */
	}
#ifdef MULTIPLE_CARD_SUPPORT
	src = (PSTRING)pAd->MC_FileName;
#endif /* MULTIPLE_CARD_SUPPORT */

	return src;
}

#ifdef SINGLE_SKU_V2
static CHAR *get_sku_profile(RTMP_ADAPTER *pAd)
{
	CHAR *src = SINGLE_SKU_TABLE_FILE_NAME;

#if defined (DRIVER_HAS_MULTI_DEV)
	INT card_idx = pAd->dev_idx;

	if (card_idx == 0)
		src = FIRST_IF_SINGLE_SKU_PATH;
	else if (card_idx == 1)
		src = SECOND_IF_SINGLE_SKU_PATH;
#endif

	return src;
}
#endif /* SINGLE_SKU_V2 */

NDIS_STATUS	RTMPReadParametersHook(RTMP_ADAPTER *pAd)
{
	PSTRING src = NULL;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	INT retval = NDIS_STATUS_FAILURE;
	ULONG buf_size = MAX_INI_BUFFER_SIZE, fsize;
	PSTRING buffer = NULL;

#ifdef HOSTAPD_SUPPORT
	int i;
#endif /*HOSTAPD_SUPPORT */

	src = get_dev_profile(pAd);
	if (src && *src)
	{
		RtmpOSFSInfoChange(&osFSInfo, TRUE);
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf)) 
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Open file \"%s\" failed!\n", src));
		}
		else 
		{
#ifndef OS_ABL_SUPPORT
			// TODO: need to roll back when convert into OSABL code
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
				fsize = (ULONG)file_inode(srcf)->i_size;
#else
				 fsize = (ULONG)srcf->f_dentry->d_inode->i_size;
#endif
				if (buf_size < (fsize + 1))
					buf_size = fsize + 1;
#endif /* OS_ABL_SUPPORT */
			os_alloc_mem(pAd, (UCHAR **)&buffer, buf_size);
			if (buffer) {
				memset(buffer, 0x00, buf_size);
				retval =RtmpOSFileRead(srcf, buffer, buf_size - 1);
				if (retval > 0)
				{
					RTMPSetProfileParameters(pAd, buffer);
					retval = NDIS_STATUS_SUCCESS;
				}
				else
					DBGPRINT(RT_DEBUG_ERROR, ("Read file \"%s\" failed(errCode=%d)!\n", src, retval));
				os_free_mem(NULL, buffer);
			} else 
				retval = NDIS_STATUS_FAILURE;

			if (RtmpOSFileClose(srcf) != 0)
			{
				retval = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("Close file \"%s\" failed(errCode=%d)!\n", src, retval));
			}
		}
		
		RtmpOSFSInfoChange(&osFSInfo, FALSE);
	}

#ifdef HOSTAPD_SUPPORT
		for (i = 0; i < pAd->ApCfg.BssidNum; i++)
		{
			pAd->ApCfg.MBSSID[i].Hostapd=Hostapd_Diable;
			DBGPRINT(RT_DEBUG_TRACE, ("Reset ra%d hostapd support=FLASE", i));
		}
#endif /*HOSTAPD_SUPPORT */

#ifdef SINGLE_SKU_V2
	RTMPSetSingleSKUParameters(pAd, get_sku_profile(pAd));
#endif /* SINGLE_SKU_V2 */

	return (retval);

}


void RTMP_IndicateMediaState(
	IN	PRTMP_ADAPTER		pAd,
	IN  NDIS_MEDIA_STATE	media_state)
{	
	pAd->IndicateMediaState = media_state;

#ifdef SYSTEM_LOG_SUPPORT
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
		{
			RTMPSendWirelessEvent(pAd, IW_STA_LINKUP_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0);
		}
		else
		{							
			RTMPSendWirelessEvent(pAd, IW_STA_LINKDOWN_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0); 		
		}	
#endif /* SYSTEM_LOG_SUPPORT */
}


void tbtt_tasklet(unsigned long data)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, tbtt_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
		PRTMP_ADAPTER pAd = (RTMP_ADAPTER *)data;
#endif /* WORKQUEUE_BH */

#ifdef RTMP_MAC_PCI
	if (pAd->OpMode == OPMODE_AP)
	{
#ifdef AP_QLOAD_SUPPORT
		/* update channel utilization */
		QBSS_LoadUpdate(pAd, 0);
#endif /* AP_QLOAD_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
		RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	}
#endif /* RTMP_MAC_PCI */

#ifdef RT_CFG80211_P2P_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)	
#else
	if (pAd->OpMode == OPMODE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* step 7 - if DTIM, then move backlogged bcast/mcast frames from PSQ to TXQ whenever DtimCount==0 */
#ifdef RTMP_MAC_PCI
		/* 
			NOTE:
			This updated BEACON frame will be sent at "next" TBTT instead of at cureent TBTT. The reason is
			because ASIC already fetch the BEACON content down to TX FIFO before driver can make any
			modification. To compenstate this effect, the actual time to deilver PSQ frames will be
			at the time that we wrapping around DtimCount from 0 to DtimPeriod-1
		*/
		if (pAd->ApCfg.DtimCount == 0)
#endif /* RTMP_MAC_PCI */
		{
			if (pAd->CommonCfg.bMcastTest == TRUE)
			{
				if (pAd->MacTab.fMcastPsQEnable == TRUE)
				{
					UINT32 macValue = 0, idx = 0, mask = 0;

					for (idx = 0; idx < 40; idx++)
					{
#ifdef RLT_MAC
						if (pAd->chipCap.hif_type == HIF_RLT)
						{
							RTMP_IO_READ32(pAd, TXQ_STA, &macValue);
						}
						else
						{
#endif /* RLT_MAC */
#ifdef RTMP_MAC
							if (pAd->chipCap.hif_type == HIF_RTMP)
								RTMP_IO_READ32(pAd, TXRXQ_STA, &macValue);
#endif /* RTMP_MAC */
#ifdef RLT_MAC
						}
#endif /* RLT_MAC */
						if (macValue & 0x0000F800)
							break;
						else
							RtmpusecDelay(50);
					}

					RTMP_IO_WRITE32(pAd, TX_WCID_DROP_MASK0, 0xFFFFFFFE);
#ifdef RLT_MAC
					if (pAd->chipCap.hif_type == HIF_RLT) 
					{
						RTMP_IO_READ32(pAd, RLT_PBF_CFG, &macValue);
						macValue |= 0x2;
						RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, macValue);

						RTMP_IO_READ32(pAd, RLT_PBF_CFG, &macValue);
						macValue |= 0x4;
						RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, macValue);

						RTMP_IO_READ32(pAd, RLT_PBF_CFG, &macValue);
						macValue |= 0x8;
						RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, macValue);
					}
					else
					{
#endif /* RLT_MAC */
#ifdef RTMP_MAC
						if (pAd->chipCap.hif_type == HIF_RTMP)
						{
							RTMP_IO_READ32(pAd, PBF_CFG, &macValue);
							macValue |= 0x8;
							RTMP_IO_WRITE32(pAd, PBF_CFG, macValue);

							RTMP_IO_READ32(pAd, PBF_CFG, &macValue);
							macValue |= 0x4;
							RTMP_IO_WRITE32(pAd, PBF_CFG, macValue);
						}
#endif /* RTMP_MAC */
#ifdef RLT_MAC
					}
#endif /* RLT_MAC */

					for (idx = 0; idx < 200; idx++)
					{
						RTMP_IO_READ32(pAd, MCU_INT_STA, &macValue);
#ifdef RLT_MAC
						if (pAd->chipCap.hif_type == HIF_RLT) 
							mask = 0x02;
						else
#endif /* RLT_MAC */
							mask = 0x04;

						if (macValue & mask)
							break;
						else
							RtmpusecDelay(50);
					}

					RTMP_IO_WRITE32(pAd, TX_WCID_DROP_MASK0, 0x0);
				}
			}
			else
			{
				PQUEUE_ENTRY pEntry;
			BOOLEAN bPS = FALSE;
			UINT count = 0;
			unsigned long IrqFlags;
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			while (pAd->MacTab.McastPsQueue.Head)
			{
				bPS = TRUE;
				if (pAd->TxSwQueue[QID_AC_BE].Number <= (pAd->TxSwQMaxLen + MAX_PACKETS_IN_MCAST_PS_QUEUE))
				{
					pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
					/*if(pAd->MacTab.McastPsQueue.Number) */
					if (count)
					{
						RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);
					}
					InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
					count++;
				}
				else
				{
					break;
				}
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			

			if (pAd->MacTab.McastPsQueue.Number == 0)
			{			
		                UINT bss_index;

                		/* clear MCAST/BCAST backlog bit for all BSS */
				for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
					WLAN_MR_TIM_BCMC_CLEAR(bss_index);
			}
			pAd->MacTab.PsQIdleCount = 0;

			if (bPS == TRUE) 
			{
				RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			}
		}
	}

		if (pAd->CommonCfg.bMcastTest == TRUE)
		{
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT) 
				RTMP_IO_WRITE32(pAd, MCU_INT_STA, 0x2);
			else
#endif /* RLT_MAC */
				RTMP_IO_WRITE32(pAd, MCU_INT_STA, 0x4);

			pAd->MacTab.fMcastPsQEnable = FALSE;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}


#ifdef WORKQUEUE_BH
void pretbtt_workq(struct work_struct *work)
#else
void pretbtt_tasklet(unsigned long data)
#endif /* WORKQUEUE_BH */
{
#ifdef CONFIG_AP_SUPPORT
	UINT32 mac_414_value = 0;

#ifdef WORKQUEUE_BH
	POS_COOKIE pObj = container_of(work, struct os_cookie, pretbtt_work);
	PRTMP_ADAPTER pAd = pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) 
		RTMP_IO_WRITE32(pAd, MCU_INT_STA, 0x2);
	else
#endif /* RLT_MAC */
		RTMP_IO_WRITE32(pAd, MCU_INT_STA, 0x4);
	pAd->MacTab.fMcastPsQEnable = FALSE;
	
	if ((pAd->ApCfg.DtimCount == 0) && (pAd->MacTab.McastPsQueue.Head))
	{
		UINT32 macValue;
		PQUEUE_ENTRY pEntry;
		BOOLEAN bPS = FALSE;
		UINT count = 0;
		unsigned long IrqFlags;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) 
		{
			RTMP_IO_READ32(pAd, RLT_PBF_CFG, &macValue);
			macValue &= (~0xE);
			RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, macValue);
		}
		else
		{
#endif /* RLT_MAC */
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
			{
				RTMP_IO_READ32(pAd, PBF_CFG, &macValue);
				macValue &= (~0xC);
				RTMP_IO_WRITE32(pAd, PBF_CFG, macValue);
			}
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		}
#endif /* RLT_MAC */
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		
		while (pAd->MacTab.McastPsQueue.Head)
		{
			bPS = TRUE;
			
			if (pAd->TxSwQueue[QID_AC_BE].Number <= (MAX_PACKETS_IN_QUEUE + MAX_PACKETS_IN_MCAST_PS_QUEUE))
			{
				pEntry = RemoveHeadQueue(&pAd->MacTab.McastPsQueue);
				
				if (count)
					RTMP_SET_PACKET_MOREDATA(pEntry, TRUE);

				InsertHeadQueue(&pAd->TxSwQueue[QID_AC_BE], pEntry);
				count++;
			}
			else
				break;
		}
		
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

		if (pAd->MacTab.McastPsQueue.Number == 0)
		{
			UINT bss_index = 0;

			/* clear MCAST/BCAST backlog bit for all BSS */
			for (bss_index = BSS0; bss_index < pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
		pAd->MacTab.PsQIdleCount = 0;

		/* dequeue outgoing frames from TxSwQueue0 queue and process it */
		if (bPS == TRUE)
		{
			RTMPDeQueuePacket(pAd, FALSE, QID_AC_BE, /*MAX_TX_IN_TBTT*/MAX_PACKETS_IN_MCAST_PS_QUEUE);
			pAd->MacTab.fMcastPsQEnable = TRUE; 
		} 	
	}
#endif /* CONFIG_AP_SUPPORT */
}

#ifdef INF_PPA_SUPPORT
static INT process_nbns_packet(
	IN PRTMP_ADAPTER 	pAd, 
	IN struct sk_buff 		*skb)
{
	UCHAR *data;
	USHORT *eth_type;

	data = (UCHAR *)eth_hdr(skb);
	if (data == 0)
	{
		data = (UCHAR *)skb->data;
		if (data == 0)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error\n", __FUNCTION__));
			return 1;
		}
	}
	
	eth_type = (USHORT *)&data[12];
	if (*eth_type == cpu_to_be16(ETH_P_IP))
	{
		INT ip_h_len;
		UCHAR *ip_h;
		UCHAR *udp_h;
		USHORT dport, host_dport;
	    
		ip_h = data + 14;
		ip_h_len = (ip_h[0] & 0x0f)*4;

		if (ip_h[9] == 0x11) /* UDP */
		{
			udp_h = ip_h + ip_h_len;
			memcpy(&dport, udp_h + 2, 2);
			host_dport = ntohs(dport);
			if ((host_dport == 67) || (host_dport == 68)) /* DHCP */
			{
				return 0;          
			}
		}
	}
    	else if ((data[12] == 0x88) && (data[13] == 0x8e)) /* EAPOL */
	{ 
		return 0;
    	}
	return 1;
}
#endif /* INF_PPA_SUPPORT */

#if defined (CONFIG_WIFI_PKT_FWD)
struct net_device *rlt_dev_get_by_name(const char *name)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	return dev_get_by_name(&init_net, name);
#else
	return dev_get_by_name(name);
#endif
}


VOID ApCliLinkCoverRxPolicy(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	OUT BOOLEAN *DropPacket)
{
	void *opp_band_tbl =NULL;
	void *band_tbl =NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pInvalidEntry = NULL;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL, *pOtherBandReptEntry = NULL;
	PNDIS_PACKET pRxPkt = pPacket;
	UCHAR *pPktHdr = NULL , isLinkValid;

	pPktHdr = GET_OS_PKT_DATAPTR(pRxPkt);

	if (wf_fwd_feedback_map_table)
		wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);

	if (opp_band_tbl == NULL)
		return;

	if (IS_GROUP_MAC(pPktHdr)) {
		pInvalidEntry = RepeaterInvaildMacLookup(pAd, pPktHdr+6);
		pOtherBandReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pPktHdr+6, FALSE, &isLinkValid);

		if ((pInvalidEntry != NULL) || (pOtherBandReptEntry != NULL)) {
			DBGPRINT(RT_DEBUG_INFO, ("%s, recv broadcast from InvalidRept Entry, drop this packet\n", __func__));
			*DropPacket = TRUE;
		}
	}
}
#endif /* CONFIG_WIFI_PKT_FWD */


void announce_802_3_packet(
	IN VOID *pAdSrc,
	IN PNDIS_PACKET pNetPkt,
	IN UCHAR OpMode)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	struct sk_buff *pRxPkt;

	ASSERT(pNetPkt);
	MEM_DBG_PKT_FREE_INC(pNetPkt);

	pRxPkt = RTPKT_TO_OSPKT(pNetPkt);

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		if (RTMP_MATPktRxNeedConvert(pAd, pRxPkt->dev))
		{
#if defined (CONFIG_WIFI_PKT_FWD)
			BOOLEAN	 need_drop = FALSE;

			ApCliLinkCoverRxPolicy(pAd, pNetPkt, &need_drop);
			
			if (need_drop == TRUE) {
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
			}
#endif /* CONFIG_WIFI_PKT_FWD */
			RTMP_MATEngineRxHandle(pAd, pNetPkt, 0);
		}
#endif /* MAT_SUPPORT */
	}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef MAT_SUPPORT
			if (RTMP_MATPktRxNeedConvert(pAd, pRxPkt->dev) &&
				(pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE))
				RTMP_MATEngineRxHandle(pAd, pNetPkt, 0);
#endif /* MAT_SUPPORT */
		}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

    /* Push up the protocol stack */
#ifdef CONFIG_AP_SUPPORT
#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)
	{
		extern int (*pToUpperLayerPktSent)(PNDIS_PACKET *pSkb);
		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		pToUpperLayerPktSent(pNetPkt);
		return;
	}
#endif /* defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570) */
#endif /* CONFIG_AP_SUPPORT */

#ifdef IKANOS_VX_1X0
	{
		IKANOS_DataFrameRx(pAd, pNetPkt);
		return;
	}
#endif /* IKANOS_VX_1X0 */

#ifdef INF_PPA_SUPPORT
	{
		if (ppa_hook_directpath_send_fn && (pAd->PPAEnable == TRUE)) 
		{
			INT retVal, ret = 0;
			UINT ppa_flags = 0;
			
			retVal = process_nbns_packet(pAd, pNetPkt);
			
			if (retVal > 0)
			{
				ret = ppa_hook_directpath_send_fn(pAd->g_if_id, pNetPkt, pNetPkt->len, ppa_flags);
				if (ret == 0)
				{
					return;
				}
				netif_rx(pRxPkt);
			}
			else if (retVal == 0)
			{
				pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
				netif_rx(pRxPkt);
			}
			else
			{
				dev_kfree_skb_any(pNetPkt);
				MEM_DBG_PKT_FREE_INC(pAd);
			}
		}	
		else
		{
			pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
			netif_rx(pRxPkt);
		}

		return;
	}
#endif /* INF_PPA_SUPPORT */

#ifdef CONFIG_RT2880_BRIDGING_ONLY
	PACKET_CB_ASSIGN(pNetPkt, 22) = 0xa8;
#endif

#if defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE)
	if(ra_classifier_hook_rx!= NULL)
	{
		unsigned long flags;
		
		RTMP_IRQ_LOCK(&pAd->page_lock, flags);
		ra_classifier_hook_rx(pNetPkt, classifier_cur_cycle);
		RTMP_IRQ_UNLOCK(&pAd->page_lock, flags);
	}
#endif /* CONFIG_RA_CLASSIFIER */

#if defined (CONFIG_RA_HW_NAT)  || defined (CONFIG_RA_HW_NAT_MODULE)
#if !defined(CONFIG_RA_NAT_NONE)
	/* bruce+
		ra_sw_nat_hook_rx return 1 --> continue
		ra_sw_nat_hook_rx return 0 --> FWD & without netif_rx
	*/
	if (ra_sw_nat_hook_rx != NULL)
	{
		pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);
		FOE_MAGIC_TAG(pRxPkt) = FOE_MAGIC_EXTIF;
		if (ra_sw_nat_hook_rx(pRxPkt))
		{
			FOE_MAGIC_TAG(pRxPkt) = 0;
			netif_rx(pRxPkt);
		}
		return;
	}
#endif /* CONFIG_RA_NAT_NONE */
#endif

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	if (BG_FTPH_PacketFromApHandle(pNetPkt) == 0)
		return;
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckUpdateDataCnx(pAd,pRxPkt);
#endif /* REDUCE_TCP_ACK_SUPPORT */

	pRxPkt->protocol = eth_type_trans(pRxPkt, pRxPkt->dev);

#if defined (CONFIG_WIFI_PKT_FWD)
		struct sk_buff *pOsRxPkt = RTPKT_TO_OSPKT(pRxPkt);
		PNET_DEV rx_dev = RtmpOsPktNetDevGet(pRxPkt);

		/* all incoming packets should set CB to mark off which net device received and in which band */
		if ((rx_dev == rlt_dev_get_by_name("rai0")) || (rx_dev == rlt_dev_get_by_name("apclii0")) ||
			(rx_dev == rlt_dev_get_by_name("rai1")) || (rx_dev == rlt_dev_get_by_name("apclii1"))) {
			RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_5G);

			if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
			{
				if (NdisEqualMemory(pOsRxPkt->dev->name, "apclii0", 7))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_GUEST_CLIENT);
			}
			else
			{
				if (NdisEqualMemory(pOsRxPkt->dev->name, "rai0", 4))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_AP);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_5G_GUEST_AP);
			}
		} else {
			RTMP_SET_PACKET_BAND(pOsRxPkt, RTMP_PACKET_SPECIFIC_2G);
		
			if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5))
			{
				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli0", 6))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_CLIENT);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_GUEST_CLIENT);
			}
			else
			{
				if (NdisEqualMemory(pOsRxPkt->dev->name, "ra0", 3))
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_AP);
				else
					RTMP_SET_PACKET_RECV_FROM(pOsRxPkt, RTMP_PACKET_RECV_FROM_2G_GUEST_AP);
			}
		}

		if (wf_fwd_rx_hook != NULL)
		{
			struct ethhdr *mh = eth_hdr(pRxPkt);
			int ret = 0;

			if ((mh->h_dest[0] & 0x1) == 0x1)
			{
				if (NdisEqualMemory(pOsRxPkt->dev->name, "apcli", 5)) 
				{
#ifdef MAC_REPEATER_SUPPORT
					if ((pAd->ApCfg.bMACRepeaterEn == TRUE) &&
						(RTMPQueryLookupRepeaterCliEntry(pAd, mh->h_source) == TRUE)) {
						RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
						return;
					}
					else
#endif /* MAC_REPEATER_SUPPORT */
					{
						VOID *opp_band_tbl = NULL;
						VOID *band_tbl = NULL;

						if (wf_fwd_feedback_map_table)
							wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);

#ifdef APCLI_GUEST_NETWORK_SUPPORT						
						if ((NdisEqualMemory(pOsRxPkt->dev->name, "apcli0", 6)) || 
							(NdisEqualMemory(pOsRxPkt->dev->name, "apclii0", 7))) {
#endif /* APCLI_GUEST_NETWORK_SUPPORT */
						if ((opp_band_tbl != NULL) 
							&& MAC_ADDR_EQUAL(((UCHAR *)((REPEATER_ADAPTER_DATA_TABLE *)opp_band_tbl)->Wdev_ifAddr), mh->h_source)) {
							RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
							return;
						}
#ifdef APCLI_GUEST_NETWORK_SUPPORT
						} else {
							if ((opp_band_pad != NULL) 
								&& MAC_ADDR_EQUAL(opp_band_pad->ApCfg.ApCliTab[1].wdev.if_addr, mh->h_source)) {
								RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
								return;
							}
						}
#endif /* APCLI_GUEST_NETWORK_SUPPORT */
					}
				}
			}
			
			ret = wf_fwd_rx_hook(pRxPkt);

			if (ret == 0) {
				return;
			} else if (ret == 2) {
				RELEASE_NDIS_PACKET(pAd, pRxPkt, NDIS_STATUS_FAILURE);
				return;
			}
		}
#endif /* CONFIG_WIFI_PKT_FWD */

	netif_rx(pRxPkt);
}


#ifdef CONFIG_SNIFFER_SUPPORT

INT Monitor_VirtualIF_Open(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_ERROR, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_OPEN, 0, dev_p, 0);


	//Monitor_Open(pAd,dev_p);

	return 0;
}

INT Monitor_VirtualIF_Close(PNET_DEV dev_p)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(dev_p);
	ASSERT(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: ===> %s\n", __FUNCTION__, RTMP_OS_NETDEV_GET_DEVNAME(dev_p)));

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_CLOSE, 0, dev_p, 0);
	//Monitor_Close(pAd,dev_p);

	VIRTUAL_IF_DOWN(pAd);

	RT_MOD_DEC_USE_COUNT();

	return 0;
}


VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;	

	/* init operation functions */
	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = Monitor_VirtualIF_Open;
	netDevOpHook.stop = Monitor_VirtualIF_Close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	DBGPRINT(RT_DEBUG_OFF, ("%s: %d !!!!####!!!!!!\n",__FUNCTION__, __LINE__)); 
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_INIT,	0, &netDevOpHook, 0);

}
VOID RT28xx_Monitor_Remove(VOID *pAd)
{

	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_SNIFF_REMOVE, 0, NULL, 0);
	
}

void STA_MonPktSend(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	PNET_DEV pNetDev;
	PNDIS_PACKET pRxPacket;
	PHEADER_802_11 pHeader;
	USHORT DataSize;
	UINT32 MaxRssi;
	UINT32 timestamp;
	CHAR RssiForRadiotap;
	UCHAR L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, AMPDU, STBC, RSSI1;
	UCHAR BssMonitorFlag11n, Channel, CentralChannel;
	UCHAR *pData, *pDevName;
	UCHAR sniffer_type = pAd->sniffer_ctl.sniffer_type;
	UCHAR sideband_index = 0;

	ASSERT(pRxBlk->pRxPacket);
	
	if (pRxBlk->DataSize < 10) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too small! (%d)\n", __FUNCTION__, pRxBlk->DataSize));
		goto err_free_sk_buff;
	}

	if (sniffer_type == RADIOTAP_TYPE) {
		if (pRxBlk->DataSize + sizeof(struct mtk_radiotap_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(struct mtk_radiotap_header)));
			goto err_free_sk_buff;
		}
	}

	if (sniffer_type == PRISM_TYPE) {
		if (pRxBlk->DataSize + sizeof(wlan_ng_prism2_header) > RX_BUFFER_AGGRESIZE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s : Size is too large! (%d)\n", __FUNCTION__, pRxBlk->DataSize + sizeof(wlan_ng_prism2_header)));
			goto err_free_sk_buff;
		}
	}

	MaxRssi = RTMPMaxRssi(pAd,
						ConvertToRssi(pAd, pRxBlk->rssi[0], RSSI_0),
						ConvertToRssi(pAd, pRxBlk->rssi[1], RSSI_1),
						ConvertToRssi(pAd, pRxBlk->rssi[2], RSSI_2));

	if (sniffer_type == RADIOTAP_TYPE){
		RssiForRadiotap = RTMPMaxRssi(pAd,
						ConvertToRssi(pAd, pRxBlk->rssi[0], RSSI_0),
						ConvertToRssi(pAd, pRxBlk->rssi[1], RSSI_1),
						ConvertToRssi(pAd, pRxBlk->rssi[2], RSSI_2));
	}

	pNetDev = pAd->monitor_ctrl.monitor_wdev.if_dev;  //send packet to mon0
	//pNetDev = get_netdev_from_bssid(pAd, BSS0);

	pRxPacket = pRxBlk->pRxPacket;
	pHeader = pRxBlk->pHeader;
	pData = pRxBlk->pData;
	DataSize = pRxBlk->DataSize;
	L2PAD = pRxBlk->pRxInfo->L2PAD;
	PHYMODE = pRxBlk->rx_rate.field.MODE;
	BW = pRxBlk->rx_rate.field.BW;

	ShortGI = pRxBlk->rx_rate.field.ShortGI;
	MCS = pRxBlk->rx_rate.field.MCS;
	LDPC = pRxBlk->rx_rate.field.ldpc;
	if (pAd->chipCap.hif_type == HIF_RLT)
		LDPC_EX_SYM = pRxBlk->ldpc_ex_sym;
	else
		LDPC_EX_SYM = 0;
		
	AMPDU = pRxBlk->pRxInfo->AMPDU;
	STBC = pRxBlk->rx_rate.field.STBC;
	RSSI1 = pRxBlk->rssi[1];
	//if(pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12] != 0)
#ifdef RLT_MAC
	NdisCopyMemory(&timestamp,&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[12],4);
#endif

	BssMonitorFlag11n = 0;
#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
	BssMonitorFlag11n = (pAd->StaCfg.BssMonitorFlag & MONITOR_FLAG_11N_SNIFFER);
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */
	pDevName = (UCHAR *)RtmpOsGetNetDevName(pAd->net_dev);
	Channel = pAd->CommonCfg.Channel;

	if (BW == BW_20)
		CentralChannel = Channel;
	else if (BW == BW_40)
		CentralChannel = pAd->CommonCfg.CentralChannel;
#ifdef DOT11_VHT_AC
	else if (BW == BW_80)
		CentralChannel = pAd->CommonCfg.vht_cent_ch; 
#endif /* DOT11_VHT_AC */

#ifdef DOT11_VHT_AC
	if (BW == BW_80) {
		sideband_index = vht_prim_ch_idx(CentralChannel, Channel);
	}
#endif /* DOT11_VHT_AC */

	if (sniffer_type == RADIOTAP_TYPE) {
#ifdef PPI_HEADER
                send_ppi_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
                                                                          L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM,
                                                                          AMPDU, STBC, RSSI1, pDevName, Channel, CentralChannel,
                                                                          sideband_index, RssiForRadiotap,timestamp);

#else
		send_radiotap_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
									  L2PAD, PHYMODE, BW, ShortGI, MCS, LDPC, LDPC_EX_SYM, 
									  AMPDU, STBC, RSSI1, pDevName, Channel, CentralChannel,
							 		  sideband_index, RssiForRadiotap,timestamp);
#endif
	}

	if (sniffer_type == PRISM_TYPE) {
		send_prism_monitor_packets(pNetDev, pRxPacket, (void *)pHeader, pData, DataSize,
								   L2PAD, PHYMODE, BW, ShortGI, MCS, AMPDU, STBC, RSSI1,
								   BssMonitorFlag11n, pDevName, Channel, CentralChannel,
							 	   MaxRssi);
	}

	return;

err_free_sk_buff:
	RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
	return;
}
#endif /* CONFIG_SNIFFER_SUPPORT */


extern NDIS_SPIN_LOCK TimerSemLock;

VOID RTMPFreeAdapter(VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE os_cookie;
	int index;	

	os_cookie=(POS_COOKIE)pAd->OS_Cookie;

	if (pAd->BeaconBuf)
		os_free_mem(NULL, pAd->BeaconBuf);

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
	if (pAd->eebuf && (pAd->eebuf != pAd->EEPROMImage))
	{
		os_free_mem(NULL, pAd->eebuf);
		pAd->eebuf = NULL;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

	NdisFreeSpinLock(&pAd->MgmtRingLock);
	
#ifdef RTMP_MAC_PCI
	for (index = 0; index < NUM_OF_RX_RING; index++)
		NdisFreeSpinLock(&pAd->RxRingLock[index]);

	NdisFreeSpinLock(&pAd->McuCmdLock);
#endif /* RTMP_MAC_PCI */

#if defined(RT3290) || defined(RLT_MAC)
	NdisFreeSpinLock(&pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

	for (index =0 ; index < NUM_OF_TX_RING; index++)
	{
		NdisFreeSpinLock(&pAd->TxSwQueueLock[index]);
		NdisFreeSpinLock(&pAd->DeQueueLock[index]);
		pAd->DeQueueRunning[index] = FALSE;
	}
	
	NdisFreeSpinLock(&pAd->irq_lock);

#ifdef RTMP_MAC_PCI
	NdisFreeSpinLock(&pAd->LockInterrupt);
#ifdef CONFIG_ANDES_SUPPORT
	NdisFreeSpinLock(&pAd->CtrlRingLock);
	NdisFreeSpinLock(&pAd->mcu_atomic);
#endif /* CONFIG_ANDES_SUPPORT */
	NdisFreeSpinLock(&pAd->tssi_lock);
#endif /* RTMP_MAC_PCI */

#ifdef SPECIFIC_BCN_BUF_SUPPORT
	NdisFreeSpinLock(&pAd->ShrMemLock);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

#ifdef UAPSD_SUPPORT
	NdisFreeSpinLock(&pAd->UAPSDEOSPLock); /* OS_ABL_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef DOT11_N_SUPPORT
	NdisFreeSpinLock(&pAd->mpdu_blk_pool.lock);
#endif /* DOT11_N_SUPPORT */

	if (pAd->iw_stats)
	{
		os_free_mem(NULL, pAd->iw_stats);
		pAd->iw_stats = NULL;
	}

	NdisFreeSpinLock(&TimerSemLock);

#ifdef RALINK_ATE
#endif /* RALINK_ATE */

	RTMP_OS_FREE_TIMER(pAd);
	RTMP_OS_FREE_LOCK(pAd);
	RTMP_OS_FREE_TASKLET(pAd);
	RTMP_OS_FREE_TASK(pAd);
	RTMP_OS_FREE_SEM(pAd);
	RTMP_OS_FREE_ATOMIC(pAd);

	RtmpOsVfree(pAd); /* pci_free_consistent(os_cookie->pci_dev,sizeof(RTMP_ADAPTER),pAd,os_cookie->pAd_pa); */
	if (os_cookie)
		os_free_mem(NULL, os_cookie);
}


#ifdef ETH_CONVERT_SUPPORT
static inline BOOLEAN rtmp_clone_mac(
	RTMP_ADAPTER *pAd,
	UCHAR *pkt,
	RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	ETH_CONVERT_STRUCT *eth_convert;
	UCHAR *pData = GET_OS_PKT_DATAPTR(pkt);
	BOOLEAN valid = TRUE;
	
	eth_convert = &pAd->EthConvert;
	/* Don't move this checking into wdev_tx_pkts(), becasue the net_device is OS-depeneded. */
	if ((eth_convert->ECMode & ETH_CONVERT_MODE_CLONE) 
		 && (!eth_convert->CloneMacVaild)
	 	&& (eth_convert->macAutoLearn)
	 	&& (!(pData[6] & 0x1)))
	{
		VOID *pNetDev = Func(pAd->net_dev, pData);

		if (!pNetDev) {
			NdisMoveMemory(&eth_convert->EthCloneMac[0], &pData[6], MAC_ADDR_LEN);
			eth_convert->CloneMacVaild = TRUE;
		}
	}

	/* Drop pkt since we are in pure clone mode and the src is not the cloned mac address. */
	if ((eth_convert->ECMode == ETH_CONVERT_MODE_CLONE)
		&& (NdisEqualMemory(pAd->CurrentAddress, &pData[6], MAC_ADDR_LEN) == FALSE))
		valid = FALSE;

	return valid;
}
#endif /* ETH_CONVERT_SUPPORT */


int RTMPSendPackets(
	IN NDIS_HANDLE dev_hnd,
	IN PPNDIS_PACKET pkt_list,
	IN UINT pkt_cnt,
	IN UINT32 pkt_total_len,
	IN RTMP_NET_ETH_CONVERT_DEV_SEARCH Func)
{
	struct wifi_dev *wdev = (struct wifi_dev *)dev_hnd;
	RTMP_ADAPTER *pAd;
	PNDIS_PACKET pPacket = pkt_list[0];

	ASSERT(wdev->sys_handle);
	pAd = (RTMP_ADAPTER *)wdev->sys_handle;
	INC_COUNTER64(pAd->WlanCounters.TransmitCountFrmOs);
#ifdef RTMP_MAC_PCI
	/* Update timer to flush tx ring buffer */
	RTMPModTimer(&pAd->TxDoneCleanupTimer, 50);
#endif /* RTMP_MAC_PCI */

	if (!pPacket)
		return 0;

#ifdef WSC_NFC_SUPPORT
	{
		struct sk_buff *pRxPkt = RTPKT_TO_OSPKT(pPacket);
		USHORT protocol = 0;
		protocol = ntohs(pRxPkt->protocol);
		if (protocol == 0x6605)
		{
			NfcParseRspCommand(pAd, pRxPkt->data, pRxPkt->len);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			return 0;
		}
	}
#endif /* WSC_NFC_SUPPORT */

	if (pkt_total_len < 14)
	{
		hex_dump("bad packet", GET_OS_PKT_DATAPTR(pPacket), pkt_total_len);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		return 0;
	}

#ifdef RALINK_ATE
	if (ATE_ON(pAd))
	{
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		return 0;
	}
#endif /* RALINK_ATE */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Drop send request since we are in monitor mode */
		if (MONITOR_ON(pAd))
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			return 0;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_5VT_ENHANCE
	RTMP_SET_PACKET_5VT(pPacket, 0);
	if (*(int*)(GET_OS_PKT_CB(pPacket)) == BRIDGE_TAG) {
		RTMP_SET_PACKET_5VT(pPacket, 1);
	}
#endif /* CONFIG_5VT_ENHANCE */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	if (wdev->wdev_type == WDEV_TYPE_STA)
	{
		if (rtmp_clone_mac(pAd, pPacket, Func) == FALSE)
		{
			/* Drop pkt since we are in pure clone mode and the src is not the cloned mac address. */
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			goto done;
		}
	}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_tx_hook != NULL)
	{
		if (wf_fwd_tx_hook(pPacket) == 1)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return 0;
		}
	}
#endif /* CONFIG_WIFI_PKT_FWD */

	wdev_tx_pkts((NDIS_HANDLE)pAd, (PPNDIS_PACKET) &pPacket, 1, wdev);

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
done:
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	return 0;
}


PNET_DEV get_netdev_from_bssid(RTMP_ADAPTER *pAd, UCHAR FromWhichBSSID)
{
	PNET_DEV dev_p = NULL;
#ifdef CONFIG_AP_SUPPORT
	UCHAR infRealIdx;
#endif /* CONFIG_AP_SUPPORT */

	do
	{
#ifdef CONFIG_STA_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_GO)
		{
			dev_p = pAd->ApCfg.MBSSID[BSS0].wdev.if_dev;
			break;		
		}
		else if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_CFG80211_VIF_P2P_CLI)
		{
			//CFG_TODO
			break;
		}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		infRealIdx = FromWhichBSSID & (NET_DEVICE_REAL_IDX_MASK);
#ifdef APCLI_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
		{
			dev_p = (infRealIdx >= MAX_APCLI_NUM ? NULL : pAd->ApCfg.ApCliTab[infRealIdx].wdev.if_dev);
			break;
		} 
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
		if(FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS)
		{
			dev_p = ((infRealIdx >= MAX_WDS_ENTRY) ? NULL : pAd->WdsTab.WdsEntry[infRealIdx].wdev.if_dev);
			break;
		}
#endif /* WDS_SUPPORT */

		if ((FromWhichBSSID > 0) &&
			(FromWhichBSSID < pAd->ApCfg.BssidNum) &&
				(FromWhichBSSID < MAX_MBSSID_NUM(pAd)) &&
				(FromWhichBSSID < HW_BEACON_MAX_NUM))
    		{
    	    		dev_p = pAd->ApCfg.MBSSID[FromWhichBSSID].wdev.if_dev;
    		}
		else
#endif /* CONFIG_AP_SUPPORT */
		{
			dev_p = pAd->net_dev;
		}

	} while (FALSE);

	ASSERT(dev_p);
	return dev_p;
}


#ifdef CONFIG_AP_SUPPORT
/*
========================================================================
Routine Description:
	Driver pre-Ioctl for AP.

Arguments:
	pAdSrc			- WLAN control block pointer
	pCB				- the IOCTL parameters

Return Value:
	NDIS_STATUS_SUCCESS	- IOCTL OK
	Otherwise			- IOCTL fail

Note:
========================================================================
*/
INT RTMP_AP_IoctlPrepare(RTMP_ADAPTER *pAd, VOID *pCB)
{
	RT_CMD_AP_IOCTL_CONFIG *pConfig = (RT_CMD_AP_IOCTL_CONFIG *)pCB;
	POS_COOKIE pObj;
	USHORT index;
	INT	Status = NDIS_STATUS_SUCCESS;
#ifdef CONFIG_APSTA_MIXED_SUPPORT
	INT cmd = 0xff;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */


	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if((pConfig->priv_flags == INT_MAIN) && !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		if (pConfig->pCmdData == NULL)
			return Status;
		
		if (RtPrivIoctlSetVal() == pConfig->CmdId_RTPRIV_IOCTL_SET)
		{
			if (TRUE
#ifdef CONFIG_APSTA_MIXED_SUPPORT
				&& (strstr(pConfig->pCmdData, "OpMode") == NULL)
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef SINGLE_SKU
				&& (strstr(pConfig->pCmdData, "ModuleTxpower") == NULL)
#endif /* SINGLE_SKU */
			)
			{
				return -ENETDOWN;
			}
		}
		else
			return -ENETDOWN;
    }

    /* determine this ioctl command is comming from which interface. */
    if (pConfig->priv_flags == INT_MAIN)
    {
		pObj->ioctl_if_type = INT_MAIN;
		pObj->ioctl_if = MAIN_MBSSID;
    }
    else if (pConfig->priv_flags == INT_MBSSID)
    {
		pObj->ioctl_if_type = INT_MBSSID;
/*    	if (!RTMPEqualMemory(net_dev->name, pAd->net_dev->name, 3))  // for multi-physical card, no MBSSID */
		if (strcmp(pConfig->name, RtmpOsGetNetDevName(pAd->net_dev)) != 0) /* sample */
    	{
	        for (index = 1; index < pAd->ApCfg.BssidNum; index++)
	    	{
	    	    if (pAd->ApCfg.MBSSID[index].wdev.if_dev == pConfig->net_dev)
	    	    {
	    	        pObj->ioctl_if = index;
	    	        break;
	    	    }
	    	}
	        /* Interface not found! */
	        if(index == pAd->ApCfg.BssidNum)
	            return -ENETDOWN;
	    }
	    else    /* ioctl command from I/F(ra0) */
	    {
    	    pObj->ioctl_if = MAIN_MBSSID;
	    }
        MBSS_MR_APIDX_SANITY_CHECK(pAd, pObj->ioctl_if);
    }
#ifdef WDS_SUPPORT
	else if (pConfig->priv_flags == INT_WDS)
	{
		pObj->ioctl_if_type = INT_WDS;
		for(index = 0; index < MAX_WDS_ENTRY; index++)
		{
			if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}
			
			if(index == MAX_WDS_ENTRY)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find wds I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
	}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
	else if (pConfig->priv_flags == INT_APCLI)
	{
		pObj->ioctl_if_type = INT_APCLI;
		for (index = 0; index < MAX_APCLI_NUM; index++)
		{
			if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pConfig->net_dev)
			{
				pObj->ioctl_if = index;
				break;
			}

			if(index == MAX_APCLI_NUM)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find Apcli I/F\n", __FUNCTION__));
				return -ENETDOWN;
			}
		}
		APCLI_MR_APIDX_SANITY_CHECK(pObj->ioctl_if);
	}
#endif /* APCLI_SUPPORT */
    else
    {
	/* DBGPRINT(RT_DEBUG_WARN, ("IOCTL is not supported in WDS interface\n")); */
    	return -EOPNOTSUPP;
    }

	pConfig->apidx = pObj->ioctl_if;
	return Status;
}


VOID AP_E2PROM_IOCTL_PostCtrl(
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	PSTRING					msg)
{
	wrq->u.data.length = strlen(msg);
	if (copy_to_user(wrq->u.data.pointer, msg, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));			
	}
}


VOID IAPP_L2_UpdatePostCtrl(
	IN PRTMP_ADAPTER	pAd,
    IN UINT8 *mac_p,
    IN INT  bssid)
{
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
VOID AP_WDS_KeyNameMakeUp(
	IN	STRING						*pKey,
	IN	UINT32						KeyMaxSize,
	IN	INT							KeyId)
{
	snprintf(pKey, KeyMaxSize, "Wds%dKey", KeyId);
}
#endif /* WDS_SUPPORT */

