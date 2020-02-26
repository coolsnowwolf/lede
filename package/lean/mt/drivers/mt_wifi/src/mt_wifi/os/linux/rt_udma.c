/****************************************************************************

    Module Name:
    rt_udma.c

    Abstract:
    For PUMA6 UDMA data path implementation.

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Anjan Chanda	05-05-2015    Initial rev.

***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RTMP_UDMA_SUPPORT

#include "rt_config.h"
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_udma.h"

#include <linux/udma_api.h>


/*******************************************************************************************
Routine Description:
    Handle received frames from UDMA layer.

Arguments:
	*skb				received skb
	*net_dev			the raxx interface pointer

Return Value:
    None

Note:
	Frame received from the UDMA layer is check for VLAN tag and handled
	accordingly
******************************************************************************************/
static void udma_net_rx_callback(struct sk_buff *skb, struct net_device *netdev)
{
	UINT16 skb_vlanid = 0, bss_vlanid = 0;
	UINT16 ethtype = 0;
	INT ap_idx = 0;
	PRTMP_ADAPTER pAd = NULL;
	BOOLEAN bDrop = true;
	UCHAR mac_addr[MAC_ADDR_LEN];
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct sk_buff *clone_skb;
	struct net_device *netdev1;


	/* MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Udma Net Rx Callback--->\n")); */
	NdisCopyMemory(&mac_addr[0], &skb->data[0], 6);

	GET_PAD_FROM_NET_DEV(pAd, netdev);

	/* pEntry = MacTableLookup(pAd, mac_addr); */

	ethtype = (skb->data[12] << 8) | (skb->data[13]);
	skb_vlanid = ((skb->data[14] << 8) | skb->data[15]) & 0xFFF;

	if (!IS_BM_MAC_ADDR(mac_addr)) {
		pEntry = MacTableLookup(pAd, mac_addr);

		if (pEntry && (ethtype == ETH_TYPE_VLAN)) {
			/* destination in MacTable and VLAN tagged packet -- need to send */
			bss_vlanid = pEntry->wdev->VLAN_VID;
			if (bss_vlanid == 0 || skb_vlanid == bss_vlanid) {
				if (bss_vlanid == 0) {
					/* vlan not configured in wifi. Strip 4 bytes vlan-tag before send */
					memmove(skb->data + 4, skb->data, 12);
					skb_pull(skb, 4);
				}
				skb->dev = pEntry->wdev->if_dev;
				rt28xx_send_packets(skb, pEntry->wdev->if_dev);
				return;
			} /* else
				printk("%s(): %d: Drop VLAN mismatch Ucast packet: skb_vlanid = %d bss_vlanid = %d\n",
				__func__, __LINE__, skb_vlanid, bss_vlanid); */
		} else {
			if (pEntry) {
				/* destination in MacTable and untagged packet -- need to send */
				/* printk("%s(): %d: Send untagged Ucast packet to associated client --->\n", __func__, __LINE__); */
				skb->dev = pEntry->wdev->if_dev;
				rt28xx_send_packets(skb, netdev);
				return;
			} /* else { */
				/* printk("%s(): %d: Drop VAN tagged (id = %d) but non-matching destined Ucast packets for\n",
				__func__, __LINE__, skb_vlanid); */
				/* printk("%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]); */
			/* } */
		}
	} else {
		if (ethtype == ETH_TYPE_VLAN) {
			for (ap_idx = 0 ; ap_idx < pAd->ApCfg.BssidNum ; ap_idx++) {
				bss_vlanid = pAd->ApCfg.MBSSID[ap_idx].wdev.VLAN_VID;
				netdev1 = pAd->ApCfg.MBSSID[ap_idx].wdev.if_dev;

				if (RTMP_OS_NETDEV_STATE_RUNNING(netdev1) &&
					(bss_vlanid == 0 || skb_vlanid == bss_vlanid)) {
					clone_skb = skb_clone(skb, MEM_ALLOC_FLAG);
					clone_skb->dev = netdev1;

					if (bss_vlanid == 0) {
						/* vlan not configured. Strip 4 bytes vlan-tag before send */
						memmove(clone_skb->data + 4, clone_skb->data, 12);
						skb_pull(clone_skb, 4);
					}

					rt28xx_send_packets(clone_skb, netdev1);
					/* bDrop = false; */
				}
			}
		} else {
			for (ap_idx = 0 ; ap_idx < pAd->ApCfg.BssidNum ; ap_idx++) {
				bss_vlanid = pAd->ApCfg.MBSSID[ap_idx].wdev.VLAN_VID;
				netdev1 = pAd->ApCfg.MBSSID[ap_idx].wdev.if_dev;

				if (RTMP_OS_NETDEV_STATE_RUNNING(netdev1) && bss_vlanid == 0) {
					clone_skb = skb_clone(skb, MEM_ALLOC_FLAG);
					clone_skb->dev = netdev1;
					/* printk("%s(): %d\n", __func__, __LINE__); */
					rt28xx_send_packets(clone_skb, netdev1);
					/* bDrop = false; */
				}
			}
		}
	}

	if (bDrop) {
		/* RELEASE_NDIS_PACKET(NULL,(PNDIS_PACKET)skb,NDIS_STATUS_FAILURE);
		// don't use this funcion to free skbs not allocated by us -- u have been warned! */
		dev_kfree_skb_any(skb);
		skb = NULL;
	}
	/* MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Udma Net Rx Callback\n")); */
}




int mt_udma_pkt_send(VOID *ctx, PNDIS_PACKET pRxPkt)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)ctx;
	BOOLEAN bUdmaSend = TRUE;
	UCHAR port;

	/* if udma not enabled, pass pkts for normal processing through os stack */
	if (unlikely(!pAd->CommonCfg.bUdmaFlag))
		return -1;

	/* udma enabled; check if we can use udma fast path */
	port = pAd->CommonCfg.UdmaPortNum;

#ifdef CONFIG_AP_SUPPORT
	/* check VLAN tagged packet */
	if (((GET_OS_PKT_DATAPTR(pRxPkt))[12] == 0x81) && ((GET_OS_PKT_DATAPTR(pRxPkt))[13] == 0x00)) {
		if (((GET_OS_PKT_DATAPTR(pRxPkt))[16] == 0x88) && ((GET_OS_PKT_DATAPTR(pRxPkt))[17] == 0x8e)) {
			/* bypass UDMA path for EAPOL type frames */
			memmove((GET_OS_PKT_DATAPTR(pRxPkt)) + 4, (GET_OS_PKT_DATAPTR(pRxPkt)), 12);
			skb_pull(pRxPkt, 4);
			/* hex_dump("VLAN EAPOL_DATA" ,GET_OS_PKT_DATAPTR(pRxPkt),GET_OS_PKT_LEN(pRxPkt));		// debug */
			bUdmaSend = FALSE;
		}
	} else {
		bUdmaSend = FALSE;
	}
#endif /* CONFIG_AP_SUPPORT */


	if (bUdmaSend == TRUE) {
		udma_xmit_skb(port, RTPKT_TO_OSPKT(pRxPkt));
		return 0;
	}

	/* pkts reaching here will NOT take udma path */
	return -1;
}


int mt_udma_register(VOID *ctx, PNET_DEV netdev)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)ctx;
	int ret = 0;

	if (pAd->CommonCfg.bUdmaFlag) {
		UCHAR port = pAd->CommonCfg.UdmaPortNum;

		ret = udma_register_handler(port, netdev, udma_net_rx_callback);
	}

	return ret;
}

int mt_udma_unregister(VOID *ctx)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)ctx;

	if (pAd->CommonCfg.bUdmaFlag) {
		UCHAR port = pAd->CommonCfg.UdmaPortNum;

		udma_flush(port);
	}

	return 0;
}

#endif /*RTMP_UDMA_SUPPORT*/
