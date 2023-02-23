/*===========================================================================
FILE:
   GobiUSBNet.c

DESCRIPTION:
   Qualcomm USB Network device for Gobi 3000
   
FUNCTIONS:
   GobiNetSuspend
   GobiNetResume
   GobiNetDriverBind
   GobiNetDriverUnbind
   GobiUSBNetURBCallback
   GobiUSBNetTXTimeout
   GobiUSBNetAutoPMThread
   GobiUSBNetStartXmit
   GobiUSBNetOpen
   GobiUSBNetStop
   GobiUSBNetProbe
   GobiUSBNetModInit
   GobiUSBNetModExit

Copyright (c) 2011, Code Aurora Forum. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Code Aurora Forum nor
      the names of its contributors may be used to endorse or promote
      products derived from this software without specific prior written
      permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
===========================================================================*/

//---------------------------------------------------------------------------
// Include Files
//---------------------------------------------------------------------------

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/ethtool.h>
#include <linux/version.h>

#include <net/arp.h>
#include <net/ip.h>
#include <net/ipv6.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,16,0) //8b094cd03b4a3793220d8d8d86a173bfea8c285b
#include <linux/timekeeping.h>
#else
#define timespec64  timespec
#define ktime_get_ts64 ktime_get_ts
#define timespec64_sub timespec_sub
#endif

#include "Structs.h"
#include "QMIDevice.h"
#include "QMI.h"

#ifndef ETH_P_MAP
#define ETH_P_MAP 0xDA1A
#endif

#if (ETH_P_MAP == 0x00F9)
#undef ETH_P_MAP
#define ETH_P_MAP 0xDA1A
#endif

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

// Version Information
//add new module or new feature, increase major version. fix bug, increase minor version
#define VERSION_NUMBER "V1.6.3"
#define DRIVER_VERSION "Quectel_Linux&Android_GobiNet_Driver_"VERSION_NUMBER
#define DRIVER_AUTHOR "Qualcomm Innovation Center"
#define DRIVER_DESC "GobiNet"
static const char driver_name[] = "GobiNet";

// Debug flag
int quec_debug = 0;

// Allow user interrupts
//int interruptible = 1;

// Number of IP packets which may be queued up for transmit
static int txQueueLength = 100;

// Class should be created during module init, so needs to be global
static struct class * gpClass;

static const unsigned char ec20_mac[ETH_ALEN] = {0x02, 0x50, 0xf3, 0x00, 0x00, 0x00};
static const unsigned char default_modem_addr[ETH_ALEN] = {0x02, 0x50, 0xf3, 0x00, 0x00, 0x00};
static const unsigned char node_id[ETH_ALEN] = {0x02, 0x50, 0xf4, 0x00, 0x00, 0x00};
//static const u8 broadcast_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

//setup data call by "AT$QCRMCALL=1,1"
static uint __read_mostly qcrmcall_mode = 0;
module_param( qcrmcall_mode, uint, S_IRUGO | S_IWUSR );

static struct sk_buff * ether_to_ip_fixup(struct net_device *dev, struct sk_buff *skb) {
	const struct ethhdr *ehdr;
	
	skb_reset_mac_header(skb);
	ehdr = eth_hdr(skb);
	
	if (ehdr->h_proto == htons(ETH_P_IP)) {
		if (unlikely(skb->len <= (sizeof(struct ethhdr) + sizeof(struct iphdr)))) {
			goto drop_skb;
		}
	}
	else if (ehdr->h_proto == htons(ETH_P_IPV6)) {
		if (unlikely(skb->len <= (sizeof(struct ethhdr) + sizeof(struct ipv6hdr)))) {
			goto drop_skb;
		}
	}
	else {
		DBG("%s skb h_proto is %04x\n", dev->name, ntohs(ehdr->h_proto));
		goto drop_skb;
	}

	if (unlikely(skb_pull(skb, ETH_HLEN)))
		return skb;

drop_skb:
	return NULL;
}

//#define QUECTEL_REMOVE_TX_ZLP
#define USB_CDC_SET_REMOVE_TX_ZLP_COMMAND 0x5D

//#define QUECTEL_WWAN_MULTI_PACKAGES

#ifdef QUECTEL_WWAN_MULTI_PACKAGES
static uint __read_mostly rx_packets = 10;
module_param( rx_packets, uint, S_IRUGO | S_IWUSR );

#define USB_CDC_SET_MULTI_PACKAGE_COMMAND (0x5C)
#define QUEC_NET_MSG_SPEC		(0x80)
#define QUEC_NET_MSG_ID_IP_DATA		(0x00)

struct multi_package_config {
	__le32 enable;
	__le32 package_max_len;
	__le32 package_max_count_in_queue;
	__le32 timeout;
} __packed;

struct quec_net_package_header {
	unsigned char msg_spec;
	unsigned char msg_id;
	unsigned short payload_len;
	unsigned char reserve[16];
} __packed;
#endif

#ifdef QUECTEL_WWAN_QMAP
/*
    Quectel_WCDMA&LTE_Linux_USB_Driver_User_Guide_V1.9.pdf
    5.6.	Test QMAP on GobiNet or QMI WWAN
    0 - no QMAP
    1 - QMAP (Aggregation protocol)
    X - QMAP (Multiplexing and Aggregation protocol)
*/
static uint __read_mostly qmap_mode = 0;
module_param( qmap_mode, uint, S_IRUGO | S_IWUSR );

struct qmap_hdr {
    u8 cd_rsvd_pad;
    u8 mux_id;
    u16 pkt_len;
} __packed;

enum rmnet_map_v5_header_type {
	RMNET_MAP_HEADER_TYPE_UNKNOWN,
	RMNET_MAP_HEADER_TYPE_COALESCING = 0x1,
	RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD = 0x2,
	RMNET_MAP_HEADER_TYPE_ENUM_LENGTH
};

/* Main QMAP header */
struct rmnet_map_header {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u8  pad_len:6;
	u8  next_hdr:1;
	u8  cd_bit:1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u8  cd_bit:1;
	u8  next_hdr:1;
	u8  pad_len:6;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	u8  mux_id;
	__be16 pkt_len;
}  __aligned(1);

/* QMAP v5 headers */
struct rmnet_map_v5_csum_header {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	u8  next_hdr:1;
	u8  header_type:7;
	u8  hw_reserved:7;
	u8  csum_valid_required:1;
#elif defined (__BIG_ENDIAN_BITFIELD)
	u8  header_type:7;
	u8  next_hdr:1;
	u8  csum_valid_required:1;
	u8  hw_reserved:7;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__be16 reserved;
} __aligned(1);

struct qmap_priv {
	struct net_device *real_dev;
	struct net_device *self_dev;
	uint qmap_version;
	uint offset_id;
	uint mux_id;
	uint link_state;

#if defined(QUECTEL_UL_DATA_AGG)
	/* QMIWDS_ADMIN_SET_DATA_FORMAT_RESP TLV_0x17 and TLV_0x18 */
	uint ul_data_aggregation_max_datagrams; //UplinkDataAggregationMaxDatagramsTlv
	uint ul_data_aggregation_max_size; //UplinkDataAggregationMaxSizeTlv
	uint dl_minimum_padding; //0x1A

	spinlock_t agg_lock;
	struct sk_buff *agg_skb;
	unsigned agg_count;
	struct timespec64 agg_time;
	struct hrtimer agg_hrtimer;
	struct work_struct agg_wq;
#endif

#ifdef QUECTEL_BRIDGE_MODE
	int m_bridge_mode;
	uint m_bridge_ipv4;
	unsigned char mHostMAC[6];
#endif
};

#ifdef QUECTEL_BRIDGE_MODE
static int is_qmap_netdev(const struct net_device *netdev);
#endif

#endif

#ifdef QUECTEL_BRIDGE_MODE
static int __read_mostly bridge_mode = 0/*|BIT(1)*/;
module_param( bridge_mode, int, S_IRUGO | S_IWUSR );

static int bridge_arp_reply(struct net_device *net, struct sk_buff *skb, uint bridge_ipv4) {
    struct arphdr *parp;
    u8 *arpptr, *sha;
    u8  sip[4], tip[4], ipv4[4];
    struct sk_buff *reply = NULL;

    ipv4[0]  = (bridge_ipv4 >> 24) & 0xFF;
    ipv4[1]  = (bridge_ipv4 >> 16) & 0xFF;
    ipv4[2]  = (bridge_ipv4 >> 8) & 0xFF;
    ipv4[3]  = (bridge_ipv4 >> 0) & 0xFF;

    parp = arp_hdr(skb);

    if (parp->ar_hrd == htons(ARPHRD_ETHER)  && parp->ar_pro == htons(ETH_P_IP)
        && parp->ar_op == htons(ARPOP_REQUEST) && parp->ar_hln == 6 && parp->ar_pln == 4) {
        arpptr = (u8 *)parp + sizeof(struct arphdr);
        sha = arpptr;
        arpptr += net->addr_len;	/* sha */
        memcpy(sip, arpptr, sizeof(sip));
        arpptr += sizeof(sip);
        arpptr += net->addr_len;	/* tha */
        memcpy(tip, arpptr, sizeof(tip));

        pr_info("%s sip = %d.%d.%d.%d, tip=%d.%d.%d.%d, ipv4=%d.%d.%d.%d\n", netdev_name(net),
            sip[0], sip[1], sip[2], sip[3], tip[0], tip[1], tip[2], tip[3], ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
	//wwan0 sip = 10.151.137.255, tip=10.151.138.0, ipv4=10.151.137.255
        if (tip[0] == ipv4[0] && tip[1] == ipv4[1] && (tip[2]&0xFC) == (ipv4[2]&0xFC) && tip[3] != ipv4[3])
            reply = arp_create(ARPOP_REPLY, ETH_P_ARP, *((__be32 *)sip), net, *((__be32 *)tip), sha, ec20_mac, sha);

        if (reply) {
            skb_reset_mac_header(reply);
            __skb_pull(reply, skb_network_offset(reply));
            reply->ip_summed = CHECKSUM_UNNECESSARY;
            reply->pkt_type = PACKET_HOST;

            netif_rx_ni(reply);
        }
        return 1;
    }

    return 0;
}

static struct sk_buff *bridge_mode_tx_fixup(struct net_device *net, struct sk_buff *skb, uint bridge_ipv4, unsigned char *bridge_mac) {
	struct ethhdr *ehdr;
	const struct iphdr *iph;

	skb_reset_mac_header(skb);
	ehdr = eth_hdr(skb);

	if (ehdr->h_proto == htons(ETH_P_ARP)) {
		if (bridge_ipv4)
			bridge_arp_reply(net, skb, bridge_ipv4);
		return NULL;
	}

	iph = ip_hdr(skb);
	//DBG("iphdr: ");
	//PrintHex((void *)iph, sizeof(struct iphdr));

// 1	0.000000000	0.0.0.0	255.255.255.255	DHCP	362	DHCP Request  - Transaction ID 0xe7643ad7
	if (ehdr->h_proto == htons(ETH_P_IP) && iph->protocol == IPPROTO_UDP && iph->saddr == 0x00000000 && iph->daddr == 0xFFFFFFFF) {
		//if (udp_hdr(skb)->dest == htons(67)) //DHCP Request
		{
			memcpy(bridge_mac, ehdr->h_source, ETH_ALEN);
			pr_info("%s PC Mac Address: %02x:%02x:%02x:%02x:%02x:%02x\n", netdev_name(net),
				bridge_mac[0], bridge_mac[1], bridge_mac[2], bridge_mac[3], bridge_mac[4], bridge_mac[5]);
		}
	}

	if (memcmp(ehdr->h_source, bridge_mac, ETH_ALEN)) {
		return NULL;
	}

	return skb;
}

static void bridge_mode_rx_fixup(sGobiUSBNet *pQmapDev, struct net_device *net, struct sk_buff *skb) {
	uint bridge_mode = 0;
	unsigned char *bridge_mac;

	if (pQmapDev->qmap_mode > 1) {
		struct qmap_priv *priv = netdev_priv(net);
		bridge_mode = priv->m_bridge_mode;
		bridge_mac = priv->mHostMAC;
	}
	else {
		bridge_mode = pQmapDev->m_bridge_mode;
		bridge_mac = pQmapDev->mHostMAC;
	}

	if (bridge_mode)
		memcpy(eth_hdr(skb)->h_dest, bridge_mac, ETH_ALEN);
	else
		memcpy(eth_hdr(skb)->h_dest, net->dev_addr, ETH_ALEN);
}

static ssize_t bridge_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct net_device *pNet = to_net_dev(dev);
    uint bridge_mode = 0;

	if (is_qmap_netdev(pNet)) {
		struct qmap_priv *priv = netdev_priv(pNet);
		bridge_mode = priv->m_bridge_mode;
	}
	else {
        struct usbnet * pDev = netdev_priv( pNet );
		sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];
        bridge_mode = pGobiDev->m_bridge_mode;
	}
  
    return snprintf(buf, PAGE_SIZE, "%d\n", bridge_mode);
}

static ssize_t bridge_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *pNet = to_net_dev(dev);
	uint old_mode = 0;
	uint bridge_mode = simple_strtoul(buf, NULL, 0);

	if (pNet->type != ARPHRD_ETHER) {
		return count;
	}

	if (is_qmap_netdev(pNet)) {
		struct qmap_priv *priv = netdev_priv(pNet);

		old_mode = priv->m_bridge_mode;
		priv->m_bridge_mode = bridge_mode;
	}
	else {
		struct usbnet * pDev = netdev_priv( pNet );
		sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];    

		old_mode = pGobiDev->m_bridge_mode;
		pGobiDev->m_bridge_mode = bridge_mode;
	}

	if (old_mode != bridge_mode)
		dev_info(dev, "bridge_mode change to 0x%x\n", bridge_mode);

	return count;
}

static ssize_t bridge_ipv4_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct net_device *pNet = to_net_dev(dev);
    unsigned int bridge_ipv4 = 0;
    unsigned char ipv4[4];

	if (is_qmap_netdev(pNet)) {
		struct qmap_priv *priv = netdev_priv(pNet);
		bridge_ipv4 = priv->m_bridge_ipv4;
	}
	else {
		struct usbnet * pDev = netdev_priv( pNet );
        sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];
        bridge_ipv4 = pGobiDev->m_bridge_ipv4;
	}

	ipv4[0]  = (bridge_ipv4 >> 24) & 0xFF;
	ipv4[1]  = (bridge_ipv4 >> 16) & 0xFF;
	ipv4[2]  = (bridge_ipv4 >> 8) & 0xFF;
	ipv4[3]  = (bridge_ipv4 >> 0) & 0xFF;

	return snprintf(buf, PAGE_SIZE, "%d.%d.%d.%d\n",  ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
}

static ssize_t bridge_ipv4_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    struct net_device *pNet = to_net_dev(dev);

	if (is_qmap_netdev(pNet)) {
		struct qmap_priv *priv = netdev_priv(pNet);
		priv->m_bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}
	else {
        struct usbnet * pDev = netdev_priv( pNet );
        sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];
		pGobiDev->m_bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}

    return count;
}

static DEVICE_ATTR(bridge_mode, S_IWUSR | S_IRUGO, bridge_mode_show, bridge_mode_store);
static DEVICE_ATTR(bridge_ipv4, S_IWUSR | S_IRUGO, bridge_ipv4_show, bridge_ipv4_store);

static struct attribute *qmi_qmap_sysfs_attrs[] = {
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
	NULL,
};

static struct attribute_group qmi_qmap_sysfs_attr_group = {
	.attrs = qmi_qmap_sysfs_attrs,
};
#endif

#ifdef QUECTEL_WWAN_QMAP
static sGobiUSBNet * net_to_qmap(struct net_device *dev) {
	struct usbnet *usbnet = netdev_priv(dev);
	sGobiUSBNet * pGobiDev = (sGobiUSBNet *)usbnet->data[0];

	return pGobiDev;
}

static struct sk_buff * add_qhdr(struct sk_buff *skb, u8 mux_id) {
	struct qmap_hdr *qhdr;
	int pad = 0;

	pad = skb->len%4;
	if (pad) {
		pad = 4 - pad;
		if (skb_tailroom(skb) < pad) {
			printk("skb_tailroom small!\n");
			pad = 0;
		}
		if (pad)
			__skb_put(skb, pad);
	}
					
	qhdr = (struct qmap_hdr *)skb_push(skb, sizeof(struct qmap_hdr));
	qhdr->cd_rsvd_pad = pad;
	qhdr->mux_id = mux_id;
	qhdr->pkt_len = cpu_to_be16(skb->len - sizeof(struct qmap_hdr));

	return skb;
}

static struct sk_buff * add_qhdr_v5(struct sk_buff *skb, u8 mux_id) {
	struct rmnet_map_header *map_header;
	struct rmnet_map_v5_csum_header *ul_header;
	u32 padding, map_datalen;

	map_datalen = skb->len;
	padding = map_datalen%4;
	if (padding) {
		padding = 4 - padding;
		if (skb_tailroom(skb) < padding) {
			printk("skb_tailroom small!\n");
			padding = 0;
		}
		if (padding)
			__skb_put(skb, padding);
	}
					
	map_header = (struct rmnet_map_header *)skb_push(skb, (sizeof(struct rmnet_map_header) + sizeof(struct rmnet_map_v5_csum_header)));
	map_header->cd_bit = 0;
	map_header->next_hdr = 1;
	map_header->pad_len = padding;
	map_header->mux_id = mux_id;
	map_header->pkt_len = htons(map_datalen + padding);

	ul_header = (struct rmnet_map_v5_csum_header *)(map_header + 1);
	memset(ul_header, 0, sizeof(*ul_header));
	ul_header->header_type = RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD;
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
#if 0 //TODO
		skb->ip_summed = CHECKSUM_NONE;
		/* Ask for checksum offloading */
		ul_header->csum_valid_required = 1;
#endif
	}

	return skb;
}

static void rmnet_usb_tx_wake_queue(unsigned long data) {
	sGobiUSBNet *pQmapDev = (void *)data;
	int i;

	for (i = 0; i < pQmapDev->qmap_mode; i++) {
		struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];
		if (qmap_net) {
			if (netif_queue_stopped(qmap_net) && !netif_queue_stopped(pQmapDev->mpNetDev->net)) {
				netif_wake_queue(qmap_net);
			}
		}
	}
}

static void rmnet_usb_tx_skb_destructor(struct sk_buff *skb) {
	sGobiUSBNet *pQmapDev = net_to_qmap(skb->dev);
	int i;
	
	for (i = 0; i < pQmapDev->qmap_mode; i++) {
		struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];

		if (qmap_net) {
			if (netif_queue_stopped(qmap_net)) {
				tasklet_schedule(&pQmapDev->txq);
				break;
			}
		}
	}
}

static void rmnet_vnd_update_rx_stats(struct net_device *net,
			unsigned rx_packets, unsigned rx_bytes) {
	net->stats.rx_packets += rx_packets;
	net->stats.rx_bytes += rx_bytes;
}

static void rmnet_vnd_update_tx_stats(struct net_device *net,
			unsigned tx_packets, unsigned tx_bytes) {	
	net->stats.tx_packets += tx_packets;
	net->stats.tx_bytes += tx_bytes;
}

#if defined(QUECTEL_UL_DATA_AGG)
static long agg_time_limit __read_mostly = 1000000L; //reduce this time, can get better TPUT performance, but will increase USB interrupts
module_param(agg_time_limit, long, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(agg_time_limit, "Maximum time packets sit in the agg buf");

static long agg_bypass_time __read_mostly = 10000000L;
module_param(agg_bypass_time, long, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(agg_bypass_time, "Skip agg when apart spaced more than this");

static int rmnet_usb_tx_agg_skip(struct sk_buff *skb, int offset)
{
	u8 *packet_start = skb->data + offset;
	int ready2send = 0;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *ip4h = (struct iphdr *)(packet_start);

		if (ip4h->protocol == IPPROTO_TCP) {
			const struct tcphdr *th = (const struct tcphdr *)(packet_start + sizeof(struct iphdr));
			if (th->psh) {
				ready2send = 1;
			}
		}
		else if (ip4h->protocol == IPPROTO_ICMP)
			ready2send = 1;

	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)(packet_start);

		if (ip6h->nexthdr == NEXTHDR_TCP) {
			const struct tcphdr *th = (const struct tcphdr *)(packet_start + sizeof(struct ipv6hdr));
			if (th->psh) {
				ready2send = 1;
			}
		} else if (ip6h->nexthdr == NEXTHDR_ICMP) {
			ready2send = 1;
		} else if (ip6h->nexthdr == NEXTHDR_FRAGMENT) {
			struct frag_hdr *frag;

			frag = (struct frag_hdr *)(packet_start
						   + sizeof(struct ipv6hdr));
			if (frag->nexthdr == IPPROTO_ICMPV6)
				ready2send = 1;
		}
	}

	return ready2send;
}

static void rmnet_usb_tx_agg_work(struct work_struct *work)
{
	struct qmap_priv *priv =
			container_of(work, struct qmap_priv, agg_wq);
	struct sk_buff *skb = NULL;
	unsigned long flags;

	spin_lock_irqsave(&priv->agg_lock, flags);
	if (likely(priv->agg_skb)) {
		skb = priv->agg_skb;
		priv->agg_skb = NULL;
		priv->agg_count = 0;
		skb->protocol = htons(ETH_P_MAP);
		skb->dev = priv->real_dev;
		ktime_get_ts64(&priv->agg_time);
	}
	spin_unlock_irqrestore(&priv->agg_lock, flags);
	
	if (skb) {
		int err = dev_queue_xmit(skb);
		if (err != NET_XMIT_SUCCESS) {
			priv->self_dev->stats.tx_errors++;
		}
	}
}

static enum hrtimer_restart  rmnet_usb_tx_agg_timer_cb(struct hrtimer *timer)
{
	struct qmap_priv *priv =
			container_of(timer, struct qmap_priv, agg_hrtimer);

	schedule_work(&priv->agg_wq);
	return HRTIMER_NORESTART;
}

static int rmnet_usb_tx_agg(struct sk_buff *skb, struct qmap_priv *priv) {
	int ready2send = 0;
	int xmit_more = 0;
	struct timespec64 diff, now;
	struct sk_buff *agg_skb = NULL;
	unsigned long flags;
	int err;
	struct net_device *pNet = priv->self_dev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0) //6b16f9ee89b8d5709f24bc3ac89ae8b5452c0d7c
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,16,0)
	xmit_more = skb->xmit_more;
#endif
#else
	xmit_more = netdev_xmit_more();
#endif

	rmnet_vnd_update_tx_stats(pNet, 1, skb->len);

	if (priv->ul_data_aggregation_max_datagrams == 1) {
		skb->protocol = htons(ETH_P_MAP);
		skb->dev = priv->real_dev;
		if (!skb->destructor)
			skb->destructor = rmnet_usb_tx_skb_destructor;
		err = dev_queue_xmit(skb);
		if (err != NET_XMIT_SUCCESS)		
			pNet->stats.tx_errors++;
		return NET_XMIT_SUCCESS;
	}

new_packet:
	spin_lock_irqsave(&priv->agg_lock, flags);
	agg_skb = NULL;
	ready2send = 0;
	ktime_get_ts64(&now);
	diff = timespec64_sub(now, priv->agg_time);

	if (priv->agg_skb) {
		if ((priv->agg_skb->len + skb->len) < priv->ul_data_aggregation_max_size) {
			memcpy(skb_put(priv->agg_skb, skb->len), skb->data, skb->len);
			priv->agg_count++;

			if (diff.tv_sec > 0 || diff.tv_nsec > agg_time_limit) {
				ready2send = 1;
			}
			else if (priv->agg_count == priv->ul_data_aggregation_max_datagrams) {
				ready2send = 1;
			}
			else if (xmit_more == 0) {
				struct rmnet_map_header *map_header = (struct rmnet_map_header *)skb->data;
				size_t offset = sizeof(struct rmnet_map_header);
				if (map_header->next_hdr)
					offset += sizeof(struct rmnet_map_v5_csum_header);

				ready2send = rmnet_usb_tx_agg_skip(skb, offset);
			}
			
			dev_kfree_skb_any(skb);
			skb = NULL;
		}
		else {
			ready2send = 1;
		}

		if (ready2send) {
			agg_skb = priv->agg_skb;
			priv->agg_skb = NULL;
			priv->agg_count = 0;
		}
	}
	else if (skb) {
		if (diff.tv_sec > 0 || diff.tv_nsec > agg_bypass_time) {
			ready2send = 1;
		}
		else if (xmit_more == 0) {
			struct rmnet_map_header *map_header = (struct rmnet_map_header *)skb->data;
			size_t offset = sizeof(struct rmnet_map_header);
			if (map_header->next_hdr)
				offset += sizeof(struct rmnet_map_v5_csum_header);

			ready2send = rmnet_usb_tx_agg_skip(skb, offset);
		}

		if (ready2send == 0) {
			priv->agg_skb = alloc_skb(priv->ul_data_aggregation_max_size, GFP_ATOMIC);
			if (priv->agg_skb) {
				memcpy(skb_put(priv->agg_skb, skb->len), skb->data, skb->len);
				priv->agg_count++;
				dev_kfree_skb_any(skb);
				skb = NULL;
			}
			else {
				ready2send = 1;
			}
		}

		if (ready2send) {
			agg_skb = skb;
			skb = NULL;
		}
	}

	if (ready2send) {
		priv->agg_time = now;
	}
	spin_unlock_irqrestore(&priv->agg_lock, flags);

	if (agg_skb) {
		agg_skb->protocol = htons(ETH_P_MAP);
		agg_skb->dev = priv->real_dev;
		if (!agg_skb->destructor)
			agg_skb->destructor = rmnet_usb_tx_skb_destructor;
		err = dev_queue_xmit(agg_skb);
		if (err != NET_XMIT_SUCCESS) {
			pNet->stats.tx_errors++;
		}
	}

	if (skb) {
		goto new_packet;
	}

	if (priv->agg_skb) {
		if (!hrtimer_is_queued(&priv->agg_hrtimer))
			hrtimer_start(&priv->agg_hrtimer, ns_to_ktime(NSEC_PER_MSEC * 2), HRTIMER_MODE_REL);
	}

	return NET_XMIT_SUCCESS;
}
#endif

static int qmap_open(struct net_device *dev)
{
	struct qmap_priv *priv = netdev_priv(dev);
	sGobiUSBNet * pGobiDev = net_to_qmap(priv->real_dev);

	if (!(priv->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	if (!pGobiDev->mbQMIReady)
		return -ENETDOWN;

#if defined(QUECTEL_UL_DATA_AGG)
	if (priv->ul_data_aggregation_max_datagrams == 1 && pGobiDev->agg_ctx.ul_data_aggregation_max_datagrams > 1) {
		priv->ul_data_aggregation_max_datagrams = pGobiDev->agg_ctx.ul_data_aggregation_max_datagrams;
		priv->ul_data_aggregation_max_size = pGobiDev->agg_ctx.ul_data_aggregation_max_size;
		priv->dl_minimum_padding = pGobiDev->agg_ctx.dl_minimum_padding;
	}
#endif

	if (netif_carrier_ok(priv->real_dev) && priv->link_state)
		netif_carrier_on(dev);

	if (netif_carrier_ok(dev)) {
		if (netif_queue_stopped(dev) && !netif_queue_stopped(priv->real_dev))
			netif_wake_queue(dev);
	}

	return 0;
}

static int qmap_stop(struct net_device *pNet)
{
	netif_carrier_off(pNet);
	return 0;
}

static int qmap_start_xmit(struct sk_buff *skb, struct net_device *pNet)
{
	int err;
	struct qmap_priv *priv = netdev_priv(pNet);

	if (netif_queue_stopped(priv->real_dev)) {
		//printk(KERN_DEBUG "s\n");
		netif_stop_queue(pNet);
		return NETDEV_TX_BUSY;
	}

	if (pNet->type == ARPHRD_ETHER) {
#ifdef QUECTEL_BRIDGE_MODE
		if (priv->m_bridge_mode && bridge_mode_tx_fixup(pNet, skb, priv->m_bridge_ipv4, priv->mHostMAC) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
#endif

		if (ether_to_ip_fixup(pNet, skb) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}

	if (priv->qmap_version == 5) {
		add_qhdr(skb, priv->mux_id);
	}
	else if (priv->qmap_version == 9) {
		add_qhdr_v5(skb, priv->mux_id);
	}
	else {
		dev_kfree_skb_any (skb);
		return NETDEV_TX_OK;
	}

#if defined(QUECTEL_UL_DATA_AGG)
	err = rmnet_usb_tx_agg(skb, priv);
#else
	skb->protocol = htons(ETH_P_MAP);
	skb->dev = priv->real_dev;
	if (!skb->destructor)
		skb->destructor = rmnet_usb_tx_skb_destructor;
	err = dev_queue_xmit(skb);
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,14 ))
	if (err == NET_XMIT_SUCCESS) {
		rmnet_vnd_update_tx_stats(pNet, 1, skb->len);
	} else {
		pNet->stats.tx_errors++;
	}
#endif
#endif

	return err;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
#else
static const struct net_device_ops qmap_netdev_ops = {
	.ndo_open       = qmap_open,
	.ndo_stop       = qmap_stop,
	.ndo_start_xmit = qmap_start_xmit,
};
#endif

#ifdef QUECTEL_BRIDGE_MODE
static int is_qmap_netdev(const struct net_device *netdev) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
    return netdev->open == qmap_open;
#else
    return netdev->netdev_ops == &qmap_netdev_ops;
#endif
}
#endif

static int qmap_register_device(sGobiUSBNet * pDev, u8 offset_id)
{
	struct net_device *real_dev = pDev->mpNetDev->net;
	struct net_device *qmap_net;
	struct qmap_priv *priv;
	int err;

	qmap_net = alloc_etherdev(sizeof(*priv));
	if (!qmap_net)
		return -ENOBUFS;

	SET_NETDEV_DEV(qmap_net, &real_dev->dev);
	priv = netdev_priv(qmap_net);
	priv->offset_id = offset_id;
	priv->mux_id = QUECTEL_QMAP_MUX_ID + offset_id;
	priv->qmap_version = pDev->qmap_version;
	priv->real_dev = real_dev;
	priv->self_dev = qmap_net;

#if defined(QUECTEL_UL_DATA_AGG)
	priv->ul_data_aggregation_max_datagrams = 1;
	priv->ul_data_aggregation_max_size = 2048;
	priv->dl_minimum_padding = 0;
	priv->agg_skb = NULL;
	priv->agg_count = 0;
	hrtimer_init(&priv->agg_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->agg_hrtimer.function = rmnet_usb_tx_agg_timer_cb;
	INIT_WORK(&priv->agg_wq, rmnet_usb_tx_agg_work);
	ktime_get_ts64(&priv->agg_time);
	spin_lock_init(&priv->agg_lock);
#endif

    sprintf(qmap_net->name, "%s.%d", real_dev->name, offset_id + 1);
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
    qmap_net->open = qmap_open;
    qmap_net->stop = qmap_stop;
    qmap_net->hard_start_xmit = qmap_start_xmit;
#else
    qmap_net->netdev_ops = &qmap_netdev_ops;
#endif
    memcpy (qmap_net->dev_addr, real_dev->dev_addr, ETH_ALEN);

#ifdef QUECTEL_BRIDGE_MODE
	priv->m_bridge_mode = !!(pDev->m_bridge_mode & BIT(offset_id));
	qmap_net->sysfs_groups[0] = &qmi_qmap_sysfs_attr_group;
#endif

    err = register_netdev(qmap_net);
    if (err < 0) {
        INFO("register_netdev(%s), err=%d\n", qmap_net->name, err);
        goto out_free_newdev;
    }
    netif_device_attach (qmap_net);

    pDev->mpQmapNetDev[offset_id] = qmap_net;
    qmap_net->flags |= IFF_NOARP;
    qmap_net->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

    INFO("%s\n", qmap_net->name);

    return 0;

out_free_newdev:
    free_netdev(qmap_net);
    return err;
}

static void qmap_unregister_device(sGobiUSBNet * pDev, u8 offset_id) {
	struct net_device *qmap_net;
#if defined(QUECTEL_UL_DATA_AGG)
	struct qmap_priv *priv;
	unsigned long flags;
#endif

	qmap_net = pDev->mpQmapNetDev[offset_id];
	if (qmap_net == NULL)
		return;

	netif_carrier_off(qmap_net);
	netif_stop_queue(qmap_net);

#if defined(QUECTEL_UL_DATA_AGG)
	priv = netdev_priv(qmap_net);
	hrtimer_cancel(&priv->agg_hrtimer);
	cancel_work_sync(&priv->agg_wq);
	spin_lock_irqsave(&priv->agg_lock, flags);
	if (priv->agg_skb) {
		kfree_skb(priv->agg_skb);
	}
	spin_unlock_irqrestore(&priv->agg_lock, flags);
#endif

	unregister_netdev(qmap_net);
	free_netdev(qmap_net);
}

static ssize_t qmap_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct net_device *pNet = to_net_dev(dev);
    struct usbnet * pDev = netdev_priv( pNet );
    sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];

    return snprintf(buf, PAGE_SIZE, "%d\n", pGobiDev->qmap_mode);
}

static DEVICE_ATTR(qmap_mode, S_IRUGO, qmap_mode_show, NULL);

static ssize_t qmap_size_show(struct device *dev, struct device_attribute *attr, char *buf) {
    struct net_device *pNet = to_net_dev(dev);
    struct usbnet * pDev = netdev_priv( pNet );
    sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];

    return snprintf(buf, PAGE_SIZE, "%d\n", pGobiDev->qmap_size);
}

static DEVICE_ATTR(qmap_size, S_IRUGO, qmap_size_show, NULL);

static ssize_t link_state_show(struct device *dev, struct device_attribute *attr, char *buf) {
	sGobiUSBNet *pQmapDev = net_to_qmap(to_net_dev(dev));

	return snprintf(buf, PAGE_SIZE, "0x%x\n",  pQmapDev->link_state);
}

static ssize_t link_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *netdev = to_net_dev(dev);
	sGobiUSBNet *pQmapDev = net_to_qmap(to_net_dev(dev));
	unsigned qmap_mode = pQmapDev->qmap_mode;
	unsigned link_state = 0;
	unsigned old_link = pQmapDev->link_state;
	uint offset_id = 0;

	link_state = simple_strtoul(buf, NULL, 0);
	if (qmap_mode == 1) {
		pQmapDev->link_state = !!link_state;
	}
	else if (qmap_mode > 1) {
		offset_id = ((link_state&0x7F) - 1);

		if (offset_id >= qmap_mode) {
			dev_info(dev, "%s offset_id is %d. but qmap_mode is %d\n", __func__, offset_id, pQmapDev->qmap_mode);
			return count;
		}

		if (link_state&0x80)
			pQmapDev->link_state &= ~(1 << offset_id);
		else
			pQmapDev->link_state |= (1 << offset_id);
	}

	if (old_link != pQmapDev->link_state) {
		struct net_device *qmap_net = pQmapDev->mpQmapNetDev[offset_id];

		if (pQmapDev->link_state) {
			netif_carrier_on(netdev);
		} else {
			netif_carrier_off(netdev);
		}

		if (qmap_net && qmap_net != netdev) {
			struct qmap_priv *priv = netdev_priv(qmap_net);

			priv->link_state = !!(pQmapDev->link_state & (1 << offset_id));
			if (priv->link_state) {
				netif_carrier_on(qmap_net);
				if (netif_queue_stopped(qmap_net) && !netif_queue_stopped(priv->real_dev))
					netif_wake_queue(qmap_net);
			}
			else
				netif_carrier_off(qmap_net);
		}
	}

	if (old_link != pQmapDev->link_state)
		dev_info(dev, "link_state 0x%x -> 0x%x\n", old_link, pQmapDev->link_state);

	return count;
}

static DEVICE_ATTR(link_state, S_IWUSR | S_IRUGO, link_state_show, link_state_store);
#endif

static struct attribute *gobinet_sysfs_attrs[] = {
#ifdef QUECTEL_BRIDGE_MODE
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
#endif
#ifdef QUECTEL_WWAN_QMAP
	&dev_attr_qmap_mode.attr,
	&dev_attr_qmap_size.attr,
	&dev_attr_link_state.attr,
#endif
	NULL,
};

static struct attribute_group gobinet_sysfs_attr_group = {
	.attrs = gobinet_sysfs_attrs,
};

#if defined(QUECTEL_WWAN_QMAP)
typedef struct {
    unsigned int size;
    unsigned int rx_urb_size;
    unsigned int ep_type;
    unsigned int iface_id;
    unsigned int qmap_mode;
    unsigned int qmap_version;
    unsigned int dl_minimum_padding;
    char ifname[8][16];
    unsigned char mux_id[8];
} RMNET_INFO;

static void rmnet_info_set(struct sGobiUSBNet *pQmapDev, RMNET_INFO *rmnet_info)
{	
	int i;

	memset(rmnet_info, 0, sizeof(*rmnet_info));
	rmnet_info->size = sizeof(RMNET_INFO);
	rmnet_info->rx_urb_size = pQmapDev->qmap_size;
	rmnet_info->ep_type = 2; //DATA_EP_TYPE_HSUSB
	rmnet_info->iface_id = 4;
	rmnet_info->qmap_mode = pQmapDev->qmap_mode;
	rmnet_info->qmap_version = pQmapDev->qmap_version;
	rmnet_info->dl_minimum_padding = 0;

	for (i = 0; i < pQmapDev->qmap_mode; i++) {
		struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];

		if (!qmap_net)
			break;

		strcpy(rmnet_info->ifname[i], qmap_net->name);
		rmnet_info->mux_id[i] = QUECTEL_QMAP_MUX_ID;
		if (pQmapDev->qmap_mode > 1) {
			struct qmap_priv *priv = netdev_priv(qmap_net);

			rmnet_info->mux_id[i] = priv->mux_id;
		}
	}
}

static int qmap_ndo_do_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd) {
	int rc = -EOPNOTSUPP;
	uint link_state = 0;
	sGobiUSBNet *pQmapDev = net_to_qmap(dev);

	atomic_inc(&pQmapDev->refcount);
	if (!pQmapDev->mbQMIReady) {
		if (wait_for_completion_interruptible_timeout(&pQmapDev->mQMIReadyCompletion, 15*HZ) <= 0) {
			if (atomic_dec_and_test(&pQmapDev->refcount)) {
				kfree( pQmapDev );
			}
			return -ETIMEDOUT;
		}
	}
	atomic_dec(&pQmapDev->refcount);

	switch (cmd) {
	case 0x89F1: //SIOCDEVPRIVATE
		rc = copy_from_user(&link_state, ifr->ifr_ifru.ifru_data, sizeof(link_state));
		if (!rc) {
			char buf[32];
			snprintf(buf, sizeof(buf), "%u", link_state);
			link_state_store(&dev->dev, NULL, buf, strlen(buf));
		}
	break;

	case 0x89F2: //SIOCDEVPRIVATE
		rc = 0;
	break;

	case 0x89F3: //SIOCDEVPRIVATE
		if (pQmapDev->qmap_mode) {
			RMNET_INFO rmnet_info;

			rmnet_info_set(pQmapDev, &rmnet_info);
			rc = copy_to_user(ifr->ifr_ifru.ifru_data, &rmnet_info, sizeof(rmnet_info));
		}
	break;

	default:
	break;
	}

	return rc;
}
#endif

#ifdef CONFIG_PM
/*===========================================================================
METHOD:
   GobiNetSuspend (Public Method)

DESCRIPTION:
   Stops QMI traffic while device is suspended

PARAMETERS
   pIntf          [ I ] - Pointer to interface
   powerEvent     [ I ] - Power management event

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
static int GobiNetSuspend(
   struct usb_interface *     pIntf,
   pm_message_t               powerEvent )
{
   struct usbnet * pDev;
   sGobiUSBNet * pGobiDev;
   
   if (pIntf == 0)
   {
      return -ENOMEM;
   }
   
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,23 ))
   pDev = usb_get_intfdata( pIntf );
#else
   pDev = (struct usbnet *)pIntf->dev.platform_data;
#endif

   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get netdevice\n" );
      return -ENXIO;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return -ENXIO;
   }

   if (pGobiDev->mbQMISyncIng)
   {
      DBG( "QMI sync ing\n" );
      return -EBUSY;
   }

   // Is this autosuspend or system suspend?
   //    do we allow remote wakeup?
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,33 ))
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
   if (pDev->udev->auto_pm == 0)
#else
   if (1)
#endif
#else
   if ((powerEvent.event & PM_EVENT_AUTO) == 0)
#endif
   {
      DBG( "device suspended to power level %d\n", 
           powerEvent.event );
      GobiSetDownReason( pGobiDev, DRIVER_SUSPENDED );
   }
   else
   {
      DBG( "device autosuspend\n" );
   }
     
   if (powerEvent.event & PM_EVENT_SUSPEND)
   {
      // Stop QMI read callbacks
   if (pGobiDev->m_qcrmcall_mode) {
   } else {
      KillRead( pGobiDev );
   }
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,22 ))
      pDev->udev->reset_resume = 0;
#endif
      
      // Store power state to avoid duplicate resumes
      pIntf->dev.power.power_state.event = powerEvent.event;
   }
   else
   {
      // Other power modes cause QMI connection to be lost
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,22 ))
      pDev->udev->reset_resume = 1;
#endif
   }
   
   // Run usbnet's suspend function
   return usbnet_suspend( pIntf, powerEvent );
}
int QuecGobiNetSuspend(struct usb_interface *pIntf, pm_message_t powerEvent ) {
	return GobiNetSuspend(pIntf, powerEvent);
}
  
/*===========================================================================
METHOD:
   GobiNetResume (Public Method)

DESCRIPTION:
   Resume QMI traffic or recreate QMI device

PARAMETERS
   pIntf          [ I ] - Pointer to interface

RETURN VALUE:
   int - 0 for success
         negative errno for failure
===========================================================================*/
static int GobiNetResume( struct usb_interface * pIntf )
{
   struct usbnet * pDev;
   sGobiUSBNet * pGobiDev;
   int nRet;
   int oldPowerState;
   
   if (pIntf == 0)
   {
      return -ENOMEM;
   }
   
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,23 ))
   pDev = usb_get_intfdata( pIntf );
#else
   pDev = (struct usbnet *)pIntf->dev.platform_data;
#endif

   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get netdevice\n" );
      return -ENXIO;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return -ENXIO;
   }

   oldPowerState = pIntf->dev.power.power_state.event;
   pIntf->dev.power.power_state.event = PM_EVENT_ON;
   DBG( "resuming from power mode %d\n", oldPowerState );

   if (oldPowerState & PM_EVENT_SUSPEND)
   {
      // It doesn't matter if this is autoresume or system resume
      GobiClearDownReason( pGobiDev, DRIVER_SUSPENDED );
   
      nRet = usbnet_resume( pIntf );
      if (nRet != 0)
      {
         DBG( "usbnet_resume error %d\n", nRet );
         return nRet;
      }

      // Restart QMI read callbacks
      if (pGobiDev->m_qcrmcall_mode) {
         nRet = 0;
      } else {
         nRet = StartRead( pGobiDev );
      }
      if (nRet != 0)
      {
         DBG( "StartRead error %d\n", nRet );
         return nRet;
      }

#ifdef CONFIG_PM
   #if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))   
      // Kick Auto PM thread to process any queued URBs
      complete( &pGobiDev->mAutoPM.mThreadDoWork );
    #endif
#endif /* CONFIG_PM */

#if defined(QUECTEL_WWAN_QMAP)
      if ((!netif_queue_stopped(pDev->net)) && (pGobiDev->qmap_mode > 1)) {
            rmnet_usb_tx_wake_queue((unsigned long )pGobiDev);
      }
#endif
   }
   else
   {
      DBG( "nothing to resume\n" );
      return 0;
   }
   
   return nRet;
}
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,27 ))   
static int GobiNetResetResume( struct usb_interface * pIntf )
{
   INFO( "device do not support reset_resume\n" );
   pIntf->needs_binding = 1;
   
   return -EOPNOTSUPP;
}
#endif
#endif /* CONFIG_PM */

static void ql_net_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *info)
{
	usbnet_get_drvinfo(net, info);
	/* Inherit standard device info */
	strlcpy(info->driver, driver_name, sizeof(info->driver));
	strlcpy(info->version, VERSION_NUMBER, sizeof(info->version));
}

static struct ethtool_ops ql_net_ethtool_ops;

/*===========================================================================
METHOD:
   GobiNetDriverBind (Public Method)

DESCRIPTION:
   Setup in and out pipes

PARAMETERS
   pDev           [ I ] - Pointer to usbnet device
   pIntf          [ I ] - Pointer to interface

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int GobiNetDriverBind( 
   struct usbnet *         pDev, 
   struct usb_interface *  pIntf )
{
   int numEndpoints;
   int endpointIndex;
   struct usb_host_endpoint * pEndpoint = NULL;
   struct usb_host_endpoint * pIn = NULL;
   struct usb_host_endpoint * pOut = NULL;
  
   // Verify one altsetting
   if (pIntf->num_altsetting != 1)
   {
      DBG( "invalid num_altsetting %u\n", pIntf->num_altsetting );
      return -ENODEV;
   }

   // Verify correct interface (4 for UC20)
   if ( !test_bit(pIntf->cur_altsetting->desc.bInterfaceNumber, &pDev->driver_info->data))
   {
      DBG( "invalid interface %d\n", 
           pIntf->cur_altsetting->desc.bInterfaceNumber );
      return -ENODEV;
   }
   
   if ( pIntf->cur_altsetting->desc.bInterfaceClass != 0xff)
   {
      struct usb_interface_descriptor *desc = &pIntf->cur_altsetting->desc;
      const char *qcfg_usbnet = "UNKNOW";
      
      if (desc->bInterfaceClass == 2 && desc->bInterfaceSubClass == 0x0e) {
         qcfg_usbnet = "MBIM";
      } else if (desc->bInterfaceClass == 2 && desc->bInterfaceSubClass == 0x06) {
         qcfg_usbnet = "ECM";
      } else if (desc->bInterfaceClass == 0xe0 && desc->bInterfaceSubClass == 1 && desc->bInterfaceProtocol == 3) {
         qcfg_usbnet = "RNDIS";
      }

      INFO( "usbnet is %s not NDIS/RMNET!\n", qcfg_usbnet);

      return -ENODEV;
   }
   
   // Collect In and Out endpoints
   numEndpoints = pIntf->cur_altsetting->desc.bNumEndpoints;
   for (endpointIndex = 0; endpointIndex < numEndpoints; endpointIndex++)
   {
      pEndpoint = pIntf->cur_altsetting->endpoint + endpointIndex;
      if (pEndpoint == NULL)
      {
         DBG( "invalid endpoint %u\n", endpointIndex );
         return -ENODEV;
      }

      if (usb_endpoint_dir_in( &pEndpoint->desc ) == true
      &&  usb_endpoint_xfer_int( &pEndpoint->desc ) == false)
      {
         pIn = pEndpoint;
      }
      else if (usb_endpoint_dir_out( &pEndpoint->desc ) == true)
      {
         pOut = pEndpoint;
      }
   }
   
   if (pIn == NULL || pOut == NULL)
   {
      DBG( "invalid endpoints\n" );
      return -ENODEV;
   }

   if (usb_set_interface( pDev->udev, 
                          pIntf->cur_altsetting->desc.bInterfaceNumber,
                          0 ) != 0)
   {
      DBG( "unable to set interface\n" );
      return -ENODEV;
   }

   pDev->in = usb_rcvbulkpipe( pDev->udev,
                   pIn->desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK );
   pDev->out = usb_sndbulkpipe( pDev->udev,
                   pOut->desc.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK );

#if defined(QUECTEL_WWAN_MULTI_PACKAGES)
    if (rx_packets && pDev->udev->descriptor.idVendor == cpu_to_le16(0x2C7C)) {
        struct multi_package_config rx_config = {
            .enable = cpu_to_le32(1),
            .package_max_len = cpu_to_le32((1500 + sizeof(struct quec_net_package_header)) * rx_packets),
            .package_max_count_in_queue = cpu_to_le32(rx_packets), 
            .timeout = cpu_to_le32(10*1000), //10ms
        };
        int ret = 0;
        
    	ret = usb_control_msg(
    		interface_to_usbdev(pIntf),
    		usb_sndctrlpipe(interface_to_usbdev(pIntf), 0),
    		USB_CDC_SET_MULTI_PACKAGE_COMMAND,
    		0x21, //USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE
    		1,
    		pIntf->cur_altsetting->desc.bInterfaceNumber,
    		&rx_config, sizeof(rx_config), 100);

        DBG( "Quectel EC21&EC25 rx_packets=%d, ret=%d\n", rx_packets, ret);        
        if (ret == sizeof(rx_config)) {
           pDev->rx_urb_size = le32_to_cpu(rx_config.package_max_len);
        } else {
            rx_packets = 0;
        }
    }
#endif

#if 1 //def DATA_MODE_RP
    /* make MAC addr easily distinguishable from an IP header */
    if ((pDev->net->dev_addr[0] & 0xd0) == 0x40) {
        /*clear this bit wil make usbnet apdater named as usbX(instead if ethX)*/
        pDev->net->dev_addr[0] |= 0x02;	/* set local assignment bit */
        pDev->net->dev_addr[0] &= 0xbf;	/* clear "IP" bit */
    }
    memcpy (pDev->net->dev_addr, node_id, sizeof node_id);
    pDev->net->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
    pDev->net->features |= (NETIF_F_VLAN_CHALLENGED);
#endif

	ql_net_ethtool_ops = *pDev->net->ethtool_ops;
	ql_net_ethtool_ops.get_drvinfo = ql_net_get_drvinfo;
	pDev->net->ethtool_ops = &ql_net_ethtool_ops;
                   
   DBG( "in %x, out %x\n", 
        pIn->desc.bEndpointAddress, 
        pOut->desc.bEndpointAddress );

   // In later versions of the kernel, usbnet helps with this
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,23 ))
   pIntf->dev.platform_data = (void *)pDev;
#endif

    if (qcrmcall_mode == 0 && pDev->net->sysfs_groups[0] == NULL && gobinet_sysfs_attr_group.attrs[0] != NULL) {
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,32)) //see commit 0c509a6c9393b27a8c5a01acd4a72616206cfc24
        pDev->net->sysfs_groups[1] = &gobinet_sysfs_attr_group; //see netdev_register_sysfs()
#else
        pDev->net->sysfs_groups[0] = &gobinet_sysfs_attr_group;
#endif
    }

    if (!pDev->rx_urb_size) {
//to advoid module report mtu 1460, but rx 1500 bytes IP packets, and cause the customer's system crash
//next setting can make usbnet.c:usbnet_change_mtu() do not modify rx_urb_size according to mtu
        pDev->rx_urb_size = ETH_DATA_LEN + ETH_HLEN + 6;
    }

   return 0;
}

/*===========================================================================
METHOD:
   GobiNetDriverUnbind (Public Method)

DESCRIPTION:
   Deregisters QMI device (Registration happened in the probe function)

PARAMETERS
   pDev           [ I ] - Pointer to usbnet device
   pIntfUnused    [ I ] - Pointer to interface

RETURN VALUE:
   None
===========================================================================*/
static void GobiNetDriverUnbind( 
   struct usbnet *         pDev, 
   struct usb_interface *  pIntf)
{
   sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];

   // Should already be down, but just in case...
   netif_carrier_off( pDev->net );

   if (pGobiDev->m_qcrmcall_mode) {
   } else {
      DeregisterQMIDevice( pGobiDev );
   }
   
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,29 ))
   kfree( pDev->net->netdev_ops );
   pDev->net->netdev_ops = NULL;
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,23 ))
   pIntf->dev.platform_data = NULL;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,19 ))
   pIntf->needs_remote_wakeup = 0;
#endif

   if (atomic_dec_and_test(&pGobiDev->refcount))
      kfree( pGobiDev );
   else
      INFO("memory leak!\n");
}

#if 1 //def DATA_MODE_RP

#if defined(QUECTEL_WWAN_QMAP)
static void _rmnet_usb_rx_handler(struct usbnet *dev, struct sk_buff *skb_in)
{
	sGobiUSBNet * pQmapDev = (sGobiUSBNet *)dev->data[0];
	struct sk_buff *qmap_skb;
	struct sk_buff_head skb_chain;
	uint dl_minimum_padding = 0;

#if defined(QUECTEL_UL_DATA_AGG)
	if (pQmapDev->qmap_version == 9)
		dl_minimum_padding = pQmapDev->agg_ctx.dl_minimum_padding;
#endif

	__skb_queue_head_init(&skb_chain);

	while (skb_in->len > sizeof(struct qmap_hdr)) {
		struct rmnet_map_header *map_header = (struct rmnet_map_header *)skb_in->data;
		struct rmnet_map_v5_csum_header *ul_header = NULL;
		size_t hdr_size = sizeof(struct rmnet_map_header);	
		struct net_device *qmap_net;
		int pkt_len = ntohs(map_header->pkt_len);
		int skb_len;
		__be16 protocol;
		int mux_id;

		if (map_header->next_hdr) {
			ul_header = (struct rmnet_map_v5_csum_header *)(map_header + 1);
			hdr_size += sizeof(struct rmnet_map_v5_csum_header);
		}
			
		skb_len = pkt_len - (map_header->pad_len&0x3F);
		skb_len -= dl_minimum_padding;
		if (skb_len > 1500) {
			dev_info(&dev->net->dev, "drop skb_len=%x larger than 1500\n", skb_len);
			goto error_pkt;
		}

		if (skb_in->len < (pkt_len + hdr_size)) {
			dev_info(&dev->net->dev, "drop qmap unknow pkt, len=%d, pkt_len=%d\n", skb_in->len, pkt_len);
			goto error_pkt;
		}

		if (map_header->cd_bit) {
			dev_info(&dev->net->dev, "skip qmap command packet\n");
			goto skip_pkt;
		}

		switch (skb_in->data[hdr_size] & 0xf0) {
			case 0x40:
				protocol = htons(ETH_P_IP);
			break;
			case 0x60:
				protocol = htons(ETH_P_IPV6);
			break;
			default:
				dev_info(&dev->net->dev, "unknow skb->protocol %02x\n", skb_in->data[hdr_size]);
				goto error_pkt;
		}
		
		mux_id = map_header->mux_id - QUECTEL_QMAP_MUX_ID;
		if (mux_id >= pQmapDev->qmap_mode) {
			dev_info(&dev->net->dev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto error_pkt;
		}

		qmap_net = pQmapDev->mpQmapNetDev[mux_id];

		if (qmap_net == NULL) {
			dev_info(&dev->net->dev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto skip_pkt;
		}

		qmap_skb = netdev_alloc_skb(qmap_net, skb_len);
		if (qmap_skb) {
			skb_put(qmap_skb, skb_len);
			memcpy(qmap_skb->data, skb_in->data + hdr_size, skb_len);
		}

		if (qmap_skb == NULL) {
			dev_info(&dev->net->dev, "fail to alloc skb, pkt_len = %d\n", skb_len);
			goto error_pkt;
		}

		skb_reset_transport_header(qmap_skb);
		skb_reset_network_header(qmap_skb);
		qmap_skb->pkt_type = PACKET_HOST;
		skb_set_mac_header(qmap_skb, 0);
		qmap_skb->protocol = protocol;

		if (ul_header && ul_header->header_type == RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD
			&& ul_header->csum_valid_required) {
#if 0 //TODO
			qmap_skb->ip_summed = CHECKSUM_UNNECESSARY;
#endif
		}

		if (qmap_skb->dev->type == ARPHRD_ETHER) {
			skb_push(qmap_skb, ETH_HLEN);
			skb_reset_mac_header(qmap_skb);
			memcpy(eth_hdr(qmap_skb)->h_source, default_modem_addr, ETH_ALEN);
			memcpy(eth_hdr(qmap_skb)->h_dest, qmap_net->dev_addr, ETH_ALEN);
			eth_hdr(qmap_skb)->h_proto = protocol;
#ifdef QUECTEL_BRIDGE_MODE
			bridge_mode_rx_fixup(pQmapDev, qmap_net, qmap_skb);
#endif
		}

		__skb_queue_tail(&skb_chain, qmap_skb);

skip_pkt:
		skb_pull(skb_in, pkt_len + hdr_size);
	}

error_pkt:
	while ((qmap_skb = __skb_dequeue (&skb_chain))) {
		if (qmap_skb->dev != dev->net) {
			if (qmap_skb->dev->type == ARPHRD_ETHER)
				__skb_pull(qmap_skb, ETH_HLEN);
			rmnet_vnd_update_rx_stats(qmap_skb->dev, 1, qmap_skb->len);
			netif_rx(qmap_skb);
		}
		else {
			qmap_skb->protocol = 0;
			usbnet_skb_return(dev, qmap_skb);
		}
	}
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,35 )) //ab95bfe01f9872459c8678572ccadbf646badad0
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,39 )) //8a4eb5734e8d1dc60a8c28576bbbdfdcc643626d
static struct sk_buff* rmnet_usb_rx_handler(struct sk_buff *skb)
{
	struct usbnet *dev;

	if (!skb)
		goto done;

	//printk("%s skb=%p, protocol=%x, len=%d\n", __func__, skb, skb->protocol, skb->len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return skb;

	if (skb->protocol != htons(ETH_P_MAP)) {
		WARN_ON(1);
		return skb;
	}

	dev = netdev_priv(skb->dev);

	if (dev == NULL) {
		WARN_ON(1);
		return skb;
	}

	_rmnet_usb_rx_handler(dev, skb);
	consume_skb(skb);

done:
	return NULL;
}
#else
static rx_handler_result_t rmnet_usb_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct usbnet *dev;

	if (!skb)
		goto done;

	//printk("%s skb=%p, protocol=%x, len=%d\n", __func__, skb, skb->protocol, skb->len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	if (skb->protocol != htons(ETH_P_MAP)) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}

	dev = netdev_priv(skb->dev);

	if (dev == NULL) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}

	_rmnet_usb_rx_handler(dev, skb);
	consume_skb(skb);

done:
	return RX_HANDLER_CONSUMED;
}
#endif
#endif
#endif
/*===========================================================================
METHOD:
   GobiNetDriverTxFixup (Public Method)

DESCRIPTION:
   Handling data format mode on transmit path

PARAMETERS
   pDev           [ I ] - Pointer to usbnet device
   pSKB           [ I ] - Pointer to transmit packet buffer
   flags          [ I ] - os flags

RETURN VALUE:
   None
===========================================================================*/
static struct sk_buff *GobiNetDriverTxFixup(struct usbnet *dev, struct sk_buff *skb, gfp_t flags)
{
	sGobiUSBNet * pGobiDev = (sGobiUSBNet *)dev->data[0];

	if (!pGobiDev) {
		DBG( "failed to get QMIDevice\n" );
		dev_kfree_skb_any(skb);
		return NULL;       
	}

	if (unlikely(!skb)) {
		return NULL;
	}

	if (!pGobiDev->mbRawIPMode)
		return skb;

#ifdef QUECTEL_WWAN_QMAP
	if (pGobiDev->qmap_mode > 1) {
		if (skb->protocol == htons(ETH_P_MAP))
			return skb;

		goto drop_skb;
	}
	else if (pGobiDev->qmap_mode == 1) {
		if (unlikely(!pGobiDev->link_state)) {
			dev_info(&dev->net->dev, "link_state 0x%x, drop skb, len = %u\n", pGobiDev->link_state, skb->len);
			goto drop_skb;
		}

		if (dev->net->type == ARPHRD_ETHER) {
#ifdef QUECTEL_BRIDGE_MODE
			if (pGobiDev->m_bridge_mode && bridge_mode_tx_fixup(dev->net, skb, pGobiDev->m_bridge_ipv4, pGobiDev->mHostMAC) == NULL) {
				goto drop_skb;
			}
#endif

			if (ether_to_ip_fixup(dev->net, skb) == NULL)
				goto drop_skb;
		}

		if (pGobiDev->qmap_version == 5) {
			add_qhdr(skb, QUECTEL_QMAP_MUX_ID);
		}
		else if (pGobiDev->qmap_version == 9) {
			add_qhdr_v5(skb, QUECTEL_QMAP_MUX_ID);
		}
		else {
	               goto drop_skb;
		}

		return skb;
	}
#endif

#ifdef QUECTEL_BRIDGE_MODE
	if (pGobiDev->m_bridge_mode && bridge_mode_tx_fixup(dev->net, skb, pGobiDev->m_bridge_ipv4, pGobiDev->mHostMAC) == NULL) {
		goto drop_skb;
	}
#endif

    // Skip Ethernet header from message
	if (likely(ether_to_ip_fixup(dev->net, skb))) {
		return skb;
	}
	else {
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,22 ))
		dev_err(&dev->intf->dev,  "Packet Dropped ");
#elif (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
		dev_err(dev->net->dev.parent,  "Packet Dropped ");
#else
		INFO("Packet Dropped ");
#endif
	}

#if defined(QUECTEL_WWAN_QMAP)    
drop_skb:
#endif
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 2,6,24 )) && defined(CONFIG_X86_32)
	INFO("dev_kfree_skb_any() will make kernel panic on CentOS!\n");
	quec_debug=1;PrintHex(skb->data, 32);quec_debug=0;
#else
	// Filter the packet out, release it
	dev_kfree_skb_any(skb);
#endif

	return NULL;
}

#if defined(QUECTEL_WWAN_MULTI_PACKAGES)
static int GobiNetDriverRxPktsFixup(struct usbnet *dev, struct sk_buff *skb)
{
    sGobiUSBNet * pGobiDev = (sGobiUSBNet *)dev->data[0];

    if (!pGobiDev->mbRawIPMode)
        return 1;

    /* This check is no longer done by usbnet */
    if (skb->len < dev->net->hard_header_len)
        return 0;

    if (!rx_packets) {
        return GobiNetDriverRxFixup(dev, skb);
    }

    while (likely(skb->len)) {
        struct sk_buff* new_skb;
        struct quec_net_package_header package_header;

        if (skb->len < sizeof(package_header))
            return 0;

        memcpy(&package_header, skb->data, sizeof(package_header));
        package_header.payload_len = be16_to_cpu(package_header.payload_len);

        if (package_header.msg_spec != QUEC_NET_MSG_SPEC || package_header.msg_id != QUEC_NET_MSG_ID_IP_DATA)
            return 0;

        if (skb->len < (package_header.payload_len + sizeof(package_header)))
            return 0;

        skb_pull(skb, sizeof(package_header));

        if (skb->len == package_header.payload_len)
            return GobiNetDriverRxFixup(dev, skb);

        new_skb = skb_clone(skb, GFP_ATOMIC);
        if (new_skb) {
            skb_trim(new_skb, package_header.payload_len);
            if (GobiNetDriverRxFixup(dev, new_skb))
                usbnet_skb_return(dev, new_skb);
            else
                return 0;
        }

        skb_pull(skb, package_header.payload_len);
    }

    return 0;
}
#endif

#ifdef QUECTEL_WWAN_QMAP
static int GobiNetDriverRxQmapFixup(struct usbnet *dev, struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,35 )) //ab95bfe01f9872459c8678572ccadbf646badad0
	rx_handler_func_t *rx_handler;

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,3,1 )) //7bdd402706cf26bfef9050dfee3f229b7f33ee4f
	if (skb->dev == NULL) {
		skb->dev = dev->net;
	}
#endif
	rx_handler = rcu_dereference(skb->dev->rx_handler);

	if (rx_handler == rmnet_usb_rx_handler) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,3,1 )) //7bdd402706cf26bfef9050dfee3f229b7f33ee4f
		unsigned headroom = skb_headroom(skb);
		if (headroom < ETH_HLEN) {
			unsigned tailroom = skb_tailroom(skb);
			if ((tailroom + headroom) >= ETH_HLEN) {
				unsigned moveroom = ETH_HLEN - headroom;
				memmove(skb->data + moveroom ,skb->data, skb->len);
				skb->data += moveroom;
				skb->tail += moveroom;
				#ifdef WARN_ONCE
				WARN_ONCE(1, "It is better reserve headroom in usbnet.c:rx_submit()!\n");
				#endif
			}
		}
#endif
		
		if (dev->net->type == ARPHRD_ETHER && skb_headroom(skb) >= ETH_HLEN) {
			//usbnet.c rx_process() usbnet_skb_return() eth_type_trans()
			skb_push(skb, ETH_HLEN);
			skb_reset_mac_header(skb);
			memcpy(eth_hdr(skb)->h_source, default_modem_addr, ETH_ALEN);
			memcpy(eth_hdr(skb)->h_dest, dev->net->dev_addr, ETH_ALEN);
			eth_hdr(skb)->h_proto = htons(ETH_P_MAP);

			return 1;
		}

#ifdef WARN_ONCE
		WARN_ONCE(1, "skb_headroom < ETH_HLEN\n");
#endif
		return 0;
	}
#endif

	_rmnet_usb_rx_handler(dev, skb);	
	return 0;
}
#endif
/*===========================================================================
METHOD:
   GobiNetDriverRxFixup (Public Method)

DESCRIPTION:
   Handling data format mode on receive path

PARAMETERS
   pDev           [ I ] - Pointer to usbnet device
   pSKB           [ I ] - Pointer to received packet buffer

RETURN VALUE:
   None
===========================================================================*/
static int GobiNetDriverRxFixup(struct usbnet *dev, struct sk_buff *skb)
{
    __be16 proto;
    sGobiUSBNet * pGobiDev = (sGobiUSBNet *)dev->data[0];

    if (!pGobiDev->mbRawIPMode)
        return 1;

    /* This check is no longer done by usbnet */
    if (skb->len < dev->net->hard_header_len)
        return 0;

#ifdef QUECTEL_WWAN_QMAP
    if (pGobiDev->qmap_mode) {
        return GobiNetDriverRxQmapFixup(dev, skb);
    }
#endif

    switch (skb->data[0] & 0xf0) {
    case 0x40:
    	proto = htons(ETH_P_IP);
    	break;
    case 0x60:
    	proto = htons(ETH_P_IPV6);
    	break;
    case 0x00:
    	if (is_multicast_ether_addr(skb->data))
    		return 1;
    	/* possibly bogus destination - rewrite just in case */
    	skb_reset_mac_header(skb);
    	goto fix_dest;
    default:
    	/* pass along other packets without modifications */
    	return 1;
    }
    if (skb_headroom(skb) < ETH_HLEN && pskb_expand_head(skb, ETH_HLEN, 0, GFP_ATOMIC)) {
        DBG("%s: couldn't pskb_expand_head\n", __func__);
        return 0;
    }
    skb_push(skb, ETH_HLEN);
    skb_reset_mac_header(skb);
    eth_hdr(skb)->h_proto = proto;
    memcpy(eth_hdr(skb)->h_source, ec20_mac, ETH_ALEN);
fix_dest:
#ifdef QUECTEL_BRIDGE_MODE
	bridge_mode_rx_fixup(pGobiDev, dev->net, skb);
#else
	memcpy(eth_hdr(skb)->h_dest, dev->net->dev_addr, ETH_ALEN);
#endif

#ifdef QUECTEL_BRIDGE_MODE
#if 0
    if (pGobiDev->m_bridge_mode) {
        struct ethhdr *ehdr = eth_hdr(skb);
quec_debug = 1;
        DBG(": ");
        PrintHex(ehdr, sizeof(struct ethhdr));
quec_debug = 0;
    }
#endif
#endif
       
    return 1;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
#ifdef CONFIG_PM
/*===========================================================================
METHOD:
   GobiUSBNetURBCallback (Public Method)

DESCRIPTION:
   Write is complete, cleanup and signal that we're ready for next packet

PARAMETERS
   pURB     [ I ] - Pointer to sAutoPM struct

RETURN VALUE:
   None
===========================================================================*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
void GobiUSBNetURBCallback( struct urb * pURB )
#else
void GobiUSBNetURBCallback(struct urb *pURB, struct pt_regs *regs)
#endif
{
   unsigned long activeURBflags;
   sAutoPM * pAutoPM = (sAutoPM *)pURB->context;
   if (pAutoPM == NULL)
   {
      // Should never happen
      DBG( "bad context\n" );
      return;
   }

   if (pURB->status != 0)
   {
      // Note that in case of an error, the behaviour is no different
      DBG( "urb finished with error %d\n", pURB->status );
   }

   // Remove activeURB (memory to be freed later)
   spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );

   // EAGAIN used to signify callback is done
   pAutoPM->mpActiveURB = ERR_PTR( -EAGAIN );

   spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );

   complete( &pAutoPM->mThreadDoWork );
   
#ifdef URB_FREE_BUFFER_BY_SELF
    if (pURB->transfer_flags & URB_FREE_BUFFER)
        kfree(pURB->transfer_buffer);
#endif
   usb_free_urb( pURB );
}

/*===========================================================================
METHOD:
   GobiUSBNetTXTimeout (Public Method)

DESCRIPTION:
   Timeout declared by the net driver.  Stop all transfers

PARAMETERS
   pNet     [ I ] - Pointer to net device

RETURN VALUE:
   None
===========================================================================*/
void GobiUSBNetTXTimeout( struct net_device * pNet )
{
   struct sGobiUSBNet * pGobiDev;
   sAutoPM * pAutoPM;
   sURBList * pURBListEntry;
   unsigned long activeURBflags, URBListFlags;
   struct usbnet * pDev = netdev_priv( pNet );
   struct urb * pURB;

   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get usbnet device\n" );
      return;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return;
   }
   pAutoPM = &pGobiDev->mAutoPM;

   DBG( "\n" );

   // Grab a pointer to active URB
   spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );
   pURB = pAutoPM->mpActiveURB;
   spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
   // Stop active URB
   if (pURB != NULL)
   {
      usb_kill_urb( pURB );
   }

   // Cleanup URB List
   spin_lock_irqsave( &pAutoPM->mURBListLock, URBListFlags );

   pURBListEntry = pAutoPM->mpURBList;
   while (pURBListEntry != NULL)
   {
      pAutoPM->mpURBList = pAutoPM->mpURBList->mpNext;
      atomic_dec( &pAutoPM->mURBListLen );
      usb_free_urb( pURBListEntry->mpURB );
      kfree( pURBListEntry );
      pURBListEntry = pAutoPM->mpURBList;
   }

   spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );

   complete( &pAutoPM->mThreadDoWork );

   return;
}

/*===========================================================================
METHOD:
   GobiUSBNetAutoPMThread (Public Method)

DESCRIPTION:
   Handle device Auto PM state asynchronously
   Handle network packet transmission asynchronously

PARAMETERS
   pData     [ I ] - Pointer to sAutoPM struct

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int GobiUSBNetAutoPMThread( void * pData )
{
   unsigned long activeURBflags, URBListFlags;
   sURBList * pURBListEntry;
   int status;
   struct usb_device * pUdev;
   sAutoPM * pAutoPM = (sAutoPM *)pData;
   struct urb * pURB;

   if (pAutoPM == NULL)
   {
      DBG( "passed null pointer\n" );
      return -EINVAL;
   }
   
   pUdev = interface_to_usbdev( pAutoPM->mpIntf );

   DBG( "traffic thread started\n" );

   while (pAutoPM->mbExit == false)
   {
      // Wait for someone to poke us
      wait_for_completion_interruptible( &pAutoPM->mThreadDoWork );

      // Time to exit?
      if (pAutoPM->mbExit == true)
      {
         // Stop activeURB
         spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );
         pURB = pAutoPM->mpActiveURB;
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );

         // EAGAIN used to signify callback is done
         if (IS_ERR( pAutoPM->mpActiveURB )
                 &&  PTR_ERR( pAutoPM->mpActiveURB ) == -EAGAIN )
         {
             pURB = NULL;
         }

         if (pURB != NULL)
         {
            usb_kill_urb( pURB );
         }
         // Will be freed in callback function

         // Cleanup URB List
         spin_lock_irqsave( &pAutoPM->mURBListLock, URBListFlags );

         pURBListEntry = pAutoPM->mpURBList;
         while (pURBListEntry != NULL)
         {
            pAutoPM->mpURBList = pAutoPM->mpURBList->mpNext;
            atomic_dec( &pAutoPM->mURBListLen );
            usb_free_urb( pURBListEntry->mpURB );
            kfree( pURBListEntry );
            pURBListEntry = pAutoPM->mpURBList;
         }

         spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );

         break;
      }
      
      // Is our URB active?
      spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );

      // EAGAIN used to signify callback is done
      if (IS_ERR( pAutoPM->mpActiveURB ) 
      &&  PTR_ERR( pAutoPM->mpActiveURB ) == -EAGAIN )
      {
         pAutoPM->mpActiveURB = NULL;

         // Restore IRQs so task can sleep
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
         
         // URB is done, decrement the Auto PM usage count
         usb_autopm_put_interface( pAutoPM->mpIntf );

         // Lock ActiveURB again
         spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );
      }

      if (pAutoPM->mpActiveURB != NULL)
      {
         // There is already a URB active, go back to sleep
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
         continue;
      }
      
      // Is there a URB waiting to be submitted?
      spin_lock_irqsave( &pAutoPM->mURBListLock, URBListFlags );
      if (pAutoPM->mpURBList == NULL)
      {
         // No more URBs to submit, go back to sleep
         spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
         continue;
      }

      // Pop an element
      pURBListEntry = pAutoPM->mpURBList;
      pAutoPM->mpURBList = pAutoPM->mpURBList->mpNext;
      atomic_dec( &pAutoPM->mURBListLen );
      spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );

      // Set ActiveURB
      pAutoPM->mpActiveURB = pURBListEntry->mpURB;
      spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );

      // Tell autopm core we need device woken up
      status = usb_autopm_get_interface( pAutoPM->mpIntf );
      if (status < 0)
      {
         DBG( "unable to autoresume interface: %d\n", status );

         // likely caused by device going from autosuspend -> full suspend
         if (status == -EPERM)
         {
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,33 ))
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))
            pUdev->auto_pm = 0;
#else
             pUdev = pUdev;
#endif
#endif
            GobiNetSuspend( pAutoPM->mpIntf, PMSG_SUSPEND );
         }

         // Add pURBListEntry back onto pAutoPM->mpURBList
         spin_lock_irqsave( &pAutoPM->mURBListLock, URBListFlags );
         pURBListEntry->mpNext = pAutoPM->mpURBList;
         pAutoPM->mpURBList = pURBListEntry;
         atomic_inc( &pAutoPM->mURBListLen );
         spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );
         
         spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );
         pAutoPM->mpActiveURB = NULL;
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
         
         // Go back to sleep
         continue;
      }

      // Submit URB
      status = usb_submit_urb( pAutoPM->mpActiveURB, GFP_KERNEL );
      if (status < 0)
      {
         // Could happen for a number of reasons
         DBG( "Failed to submit URB: %d.  Packet dropped\n", status );
         spin_lock_irqsave( &pAutoPM->mActiveURBLock, activeURBflags );
         usb_free_urb( pAutoPM->mpActiveURB );
         pAutoPM->mpActiveURB = NULL;
         spin_unlock_irqrestore( &pAutoPM->mActiveURBLock, activeURBflags );
         usb_autopm_put_interface( pAutoPM->mpIntf );

         // Loop again
         complete( &pAutoPM->mThreadDoWork );
      }
      
      kfree( pURBListEntry );
   }   
   
   DBG( "traffic thread exiting\n" );
   pAutoPM->mpThread = NULL;
   return 0;
}      

/*===========================================================================
METHOD:
   GobiUSBNetStartXmit (Public Method)

DESCRIPTION:
   Convert sk_buff to usb URB and queue for transmit

PARAMETERS
   pNet     [ I ] - Pointer to net device

RETURN VALUE:
   NETDEV_TX_OK on success
   NETDEV_TX_BUSY on error
===========================================================================*/
int GobiUSBNetStartXmit( 
   struct sk_buff *     pSKB,
   struct net_device *  pNet )
{
   unsigned long URBListFlags;
   struct sGobiUSBNet * pGobiDev;
   sAutoPM * pAutoPM;
   sURBList * pURBListEntry, ** ppURBListEnd;
   void * pURBData;
   struct usbnet * pDev = netdev_priv( pNet );
   
   //DBG( "\n" );
   
   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get usbnet device\n" );
      return NETDEV_TX_BUSY;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return NETDEV_TX_BUSY;
   }
   pAutoPM = &pGobiDev->mAutoPM;
   
   if( NULL == pSKB )
   {
       DBG( "Buffer is NULL \n" );
       return NETDEV_TX_BUSY;
   }

   if (GobiTestDownReason( pGobiDev, DRIVER_SUSPENDED ))
   {
      // Should not happen
      DBG( "device is suspended\n" );
      dump_stack();
      return NETDEV_TX_BUSY;
   }
   
   if (GobiTestDownReason( pGobiDev, NO_NDIS_CONNECTION ))
   {
      //netif_carrier_off( pGobiDev->mpNetDev->net );
      //DBG( "device is disconnected\n" );
      //dump_stack();
      return NETDEV_TX_BUSY;
   }
   
   // Convert the sk_buff into a URB

   // Check if buffer is full
   if ( atomic_read( &pAutoPM->mURBListLen ) >= txQueueLength)
   {
      DBG( "not scheduling request, buffer is full\n" );
      return NETDEV_TX_BUSY;
   }

   // Allocate URBListEntry
   pURBListEntry = kmalloc( sizeof( sURBList ), GFP_ATOMIC );
   if (pURBListEntry == NULL)
   {
      DBG( "unable to allocate URBList memory\n" );
      return NETDEV_TX_BUSY;
   }
   pURBListEntry->mpNext = NULL;

   // Allocate URB
   pURBListEntry->mpURB = usb_alloc_urb( 0, GFP_ATOMIC );
   if (pURBListEntry->mpURB == NULL)
   {
      DBG( "unable to allocate URB\n" );
      // release all memory allocated by now 
      if (pURBListEntry)
         kfree( pURBListEntry );
      return NETDEV_TX_BUSY;
   }

#if 1 //def DATA_MODE_RP
   GobiNetDriverTxFixup(pDev, pSKB, GFP_ATOMIC);	
#endif

   // Allocate URB transfer_buffer
   pURBData = kmalloc( pSKB->len, GFP_ATOMIC );
   if (pURBData == NULL)
   {
      DBG( "unable to allocate URB data\n" );
      // release all memory allocated by now
      if (pURBListEntry)
      {
         usb_free_urb( pURBListEntry->mpURB );
         kfree( pURBListEntry );
      }
      return NETDEV_TX_BUSY;
   }
   // Fill with SKB's data
   memcpy( pURBData, pSKB->data, pSKB->len );

   usb_fill_bulk_urb( pURBListEntry->mpURB,
                      pGobiDev->mpNetDev->udev,
                      pGobiDev->mpNetDev->out,
                      pURBData,
                      pSKB->len,
                      GobiUSBNetURBCallback,
                      pAutoPM );

   /* Handle the need to send a zero length packet and release the
    * transfer buffer
    */
    pURBListEntry->mpURB->transfer_flags |= (URB_ZERO_PACKET | URB_FREE_BUFFER);

   // Aquire lock on URBList
   spin_lock_irqsave( &pAutoPM->mURBListLock, URBListFlags );
   
   // Add URB to end of list
   ppURBListEnd = &pAutoPM->mpURBList;
   while ((*ppURBListEnd) != NULL)
   {
      ppURBListEnd = &(*ppURBListEnd)->mpNext;
   }
   *ppURBListEnd = pURBListEntry;
   atomic_inc( &pAutoPM->mURBListLen );

   spin_unlock_irqrestore( &pAutoPM->mURBListLock, URBListFlags );

   complete( &pAutoPM->mThreadDoWork );

   // Start transfer timer
   pNet->trans_start = jiffies;
   // Free SKB
   if (pSKB)
      dev_kfree_skb_any( pSKB );

   return NETDEV_TX_OK;
}
#endif
static int (*local_usbnet_start_xmit) (struct sk_buff *skb, struct net_device *net);
#endif

static int GobiUSBNetStartXmit2( struct sk_buff *pSKB, struct net_device *pNet ){
   struct sGobiUSBNet * pGobiDev;
   struct usbnet * pDev = netdev_priv( pNet );
   
   //DBG( "\n" );
   
   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get usbnet device\n" );
      return NETDEV_TX_BUSY;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return NETDEV_TX_BUSY;
   }
   
   if( NULL == pSKB )
   {
       DBG( "Buffer is NULL \n" );
       return NETDEV_TX_BUSY;
   }

   if (GobiTestDownReason( pGobiDev, DRIVER_SUSPENDED ))
   {
      // Should not happen
      DBG( "device is suspended\n" );
      dump_stack();
      return NETDEV_TX_BUSY;
   }
   
   if (GobiTestDownReason( pGobiDev, NO_NDIS_CONNECTION ))
   {
      //netif_carrier_off( pGobiDev->mpNetDev->net );
      //DBG( "device is disconnected\n" );
      //dump_stack();
      return NETDEV_TX_BUSY;
   }

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
   return local_usbnet_start_xmit(pSKB, pNet);
#else
   return usbnet_start_xmit(pSKB, pNet);
#endif
}

/*===========================================================================
METHOD:
   GobiUSBNetOpen (Public Method)

DESCRIPTION:
   Wrapper to usbnet_open, correctly handling autosuspend
   Start AutoPM thread (if CONFIG_PM is defined)

PARAMETERS
   pNet     [ I ] - Pointer to net device

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int GobiUSBNetOpen( struct net_device * pNet )
{
   int status = 0;
   struct sGobiUSBNet * pGobiDev;
   struct usbnet * pDev = netdev_priv( pNet );
 
   if (pDev == NULL)
   {
      DBG( "failed to get usbnet device\n" );
      return -ENXIO;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return -ENXIO;
   }

   DBG( "\n" );

#ifdef CONFIG_PM
   #if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
   // Start the AutoPM thread
   pGobiDev->mAutoPM.mpIntf = pGobiDev->mpIntf;
   pGobiDev->mAutoPM.mbExit = false;
   pGobiDev->mAutoPM.mpURBList = NULL;
   pGobiDev->mAutoPM.mpActiveURB = NULL;
   spin_lock_init( &pGobiDev->mAutoPM.mURBListLock );
   spin_lock_init( &pGobiDev->mAutoPM.mActiveURBLock );
   atomic_set( &pGobiDev->mAutoPM.mURBListLen, 0 );
   init_completion( &pGobiDev->mAutoPM.mThreadDoWork );
   
   pGobiDev->mAutoPM.mpThread = kthread_run( GobiUSBNetAutoPMThread, 
                                               &pGobiDev->mAutoPM, 
                                               "GobiUSBNetAutoPMThread" );
   if (IS_ERR( pGobiDev->mAutoPM.mpThread ))
   {
      DBG( "AutoPM thread creation error\n" );
      return PTR_ERR( pGobiDev->mAutoPM.mpThread );
   }
   #endif
#endif /* CONFIG_PM */

   // Allow traffic
   GobiClearDownReason( pGobiDev, NET_IFACE_STOPPED );

   // Pass to usbnet_open if defined
   if (pGobiDev->mpUSBNetOpen != NULL)
   {
      status = pGobiDev->mpUSBNetOpen( pNet );
#ifdef CONFIG_PM
      // If usbnet_open was successful enable Auto PM
      if (status == 0)
      {
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,33 ))
         usb_autopm_enable( pGobiDev->mpIntf );
#else
         usb_autopm_put_interface( pGobiDev->mpIntf );
#endif
      }
#endif /* CONFIG_PM */
   }
   else
   {
      DBG( "no USBNetOpen defined\n" );
   }
   
   return status;
}

/*===========================================================================
METHOD:
   GobiUSBNetStop (Public Method)

DESCRIPTION:
   Wrapper to usbnet_stop, correctly handling autosuspend
   Stop AutoPM thread (if CONFIG_PM is defined)

PARAMETERS
   pNet     [ I ] - Pointer to net device

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int GobiUSBNetStop( struct net_device * pNet )
{
   struct sGobiUSBNet * pGobiDev;
   struct usbnet * pDev = netdev_priv( pNet );

   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get netdevice\n" );
      return -ENXIO;
   }
   
   pGobiDev = (sGobiUSBNet *)pDev->data[0];
   if (pGobiDev == NULL)
   {
      DBG( "failed to get QMIDevice\n" );
      return -ENXIO;
   }

   // Stop traffic
   GobiSetDownReason( pGobiDev, NET_IFACE_STOPPED );

#ifdef CONFIG_PM
   #if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
   // Tell traffic thread to exit
   pGobiDev->mAutoPM.mbExit = true;
   complete( &pGobiDev->mAutoPM.mThreadDoWork );
   
   // Wait for it to exit
   while( pGobiDev->mAutoPM.mpThread != NULL )
   {
      msleep( 100 );
   }
   DBG( "thread stopped\n" );
   #endif
#endif /* CONFIG_PM */

   // Pass to usbnet_stop, if defined
   if (pGobiDev->mpUSBNetStop != NULL)
   {
      return pGobiDev->mpUSBNetStop( pNet );
   }
   else
   {
      return 0;
   }
}

static int GobiNetDriver_check_connect(struct usbnet *pDev) {
   int status = 0;
   struct sGobiUSBNet * pGobiDev = NULL;

   while (status++ < 10) {
      pGobiDev = (sGobiUSBNet *)pDev->data[0];
      if (pGobiDev && pGobiDev->mbProbeDone)
         break;
      msleep(1);
   }

   return 0;
}

/*=========================================================================*/
// Struct driver_info
/*=========================================================================*/
static struct driver_info GobiNetInfo = 
{
   .description   = "GobiNet Ethernet Device",
#if 1//def CONFIG_ANDROID
#if defined(QUECTEL_WWAN_QMAP) && defined(FLAG_RX_ASSEMBLE)
	.flags         = FLAG_RX_ASSEMBLE, //usb0
#endif
#else
#if defined(QUECTEL_WWAN_QMAP) && defined(FLAG_RX_ASSEMBLE)
	.flags         = FLAG_ETHER | FLAG_RX_ASSEMBLE,
#else
	.flags         = FLAG_ETHER,
#endif
#endif
   .bind          = GobiNetDriverBind,
   .unbind        = GobiNetDriverUnbind,
#if 1 //def DATA_MODE_RP
#if defined(QUECTEL_WWAN_MULTI_PACKAGES)
   .rx_fixup      = GobiNetDriverRxPktsFixup,
#else
   .rx_fixup      = GobiNetDriverRxFixup,
#endif
   .tx_fixup      = GobiNetDriverTxFixup,
#endif
   .check_connect = GobiNetDriver_check_connect,
   .data          = (1 << 4),
};

/*=========================================================================*/
// Qualcomm Gobi 3000 VID/PIDs
/*=========================================================================*/
#define GOBI_FIXED_INTF(vend, prod) \
    { \
          USB_DEVICE( vend, prod ), \
          .driver_info = (unsigned long)&GobiNetInfo, \
    }
static const struct usb_device_id QuecGobiVIDPIDTable [] =
{
    GOBI_FIXED_INTF( 0x05c6, 0x9003 ), // Quectel UC20
    GOBI_FIXED_INTF( 0x05c6, 0x9215 ), // Quectel EC20 (MDM9215)
    GOBI_FIXED_INTF( 0x2c7c, 0x0125 ), // Quectel EC20 (MDM9X07)/EC25/EG25
    GOBI_FIXED_INTF( 0x2c7c, 0x0121 ), // Quectel EC21
    GOBI_FIXED_INTF( 0x2c7c, 0x0306 ), // Quectel EP06
    GOBI_FIXED_INTF( 0x2c7c, 0x030B ), // Quectel EG065K,SDX12
    GOBI_FIXED_INTF( 0x2c7c, 0x0435 ), // Quectel AG35
    GOBI_FIXED_INTF( 0x2c7c, 0x0296 ), // Quectel BG96
    GOBI_FIXED_INTF( 0x2c7c, 0x0191 ), // Quectel EG91
    GOBI_FIXED_INTF( 0x2c7c, 0x0195 ), // Quectel EG95	
    GOBI_FIXED_INTF( 0x2c7c, 0x0512 ), // Quectel EG12/EP12/EM12/EG16/EG18,SDx20
    GOBI_FIXED_INTF( 0x2c7c, 0x0620 ), // Quectel EG20,SDx24
    GOBI_FIXED_INTF( 0x2c7c, 0x0800 ), // Quectel RG500Q,RM500Q,RM510Q,SDX55
    GOBI_FIXED_INTF( 0x2c7c, 0x0801 ), // Quectel RG520Q,RM520Q,SG520Q,SDX6X
   //Terminating entry
   { }
};

MODULE_DEVICE_TABLE( usb, QuecGobiVIDPIDTable );

/*===========================================================================
METHOD:
   GobiUSBNetProbe (Public Method)

DESCRIPTION:
   Run usbnet_probe
   Setup QMI device

PARAMETERS
   pIntf        [ I ] - Pointer to interface
   pVIDPIDs     [ I ] - Pointer to VID/PID table

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int GobiUSBNetProbe( 
   struct usb_interface *        pIntf, 
   const struct usb_device_id *  pVIDPIDs )
{
   int status;
   struct usbnet * pDev;
   sGobiUSBNet * pGobiDev;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,29 ))
   struct net_device_ops * pNetDevOps;
#endif   

   status = usbnet_probe( pIntf, pVIDPIDs );
   if (status < 0)
   {
      DBG( "usbnet_probe failed %d\n", status );
	  return status; 
   }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 2,6,19 ))
   pIntf->needs_remote_wakeup = 1;
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,23 ))
   pDev = usb_get_intfdata( pIntf );
#else
   pDev = (struct usbnet *)pIntf->dev.platform_data;
#endif

   if (pDev == NULL || pDev->net == NULL)
   {
      DBG( "failed to get netdevice\n" );
      usbnet_disconnect( pIntf );
      return -ENXIO;
   }

   pGobiDev = kzalloc( sizeof( sGobiUSBNet ), GFP_KERNEL );
   if (pGobiDev == NULL)
   {
      DBG( "fail to allocate device buffers" );
      usbnet_disconnect( pIntf );
      return -ENOMEM;
   }
   
   atomic_set(&pGobiDev->refcount, 1);

   pDev->data[0] = (unsigned long)pGobiDev;
   
   pGobiDev->mpNetDev = pDev;

   // Clearing endpoint halt is a magic handshake that brings 
   // the device out of low power (airplane) mode
   usb_clear_halt( pGobiDev->mpNetDev->udev, pDev->out );

   // Overload PM related network functions
#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
   pGobiDev->mpUSBNetOpen = pDev->net->open;
   pDev->net->open = GobiUSBNetOpen;
   pGobiDev->mpUSBNetStop = pDev->net->stop;
   pDev->net->stop = GobiUSBNetStop;
#if defined(CONFIG_PM) && (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,14 ))
   pDev->net->hard_start_xmit = GobiUSBNetStartXmit;
   pDev->net->tx_timeout = GobiUSBNetTXTimeout;
#else  //quectel donot send dhcp request before ndis connect for uc20
    local_usbnet_start_xmit = pDev->net->hard_start_xmit;
    pDev->net->hard_start_xmit = GobiUSBNetStartXmit2;
#endif
#else
   pNetDevOps = kmalloc( sizeof( struct net_device_ops ), GFP_KERNEL );
   if (pNetDevOps == NULL)
   {
      DBG( "falied to allocate net device ops" );
      usbnet_disconnect( pIntf );
      return -ENOMEM;
   }
   memcpy( pNetDevOps, pDev->net->netdev_ops, sizeof( struct net_device_ops ) );
   
   pGobiDev->mpUSBNetOpen = pNetDevOps->ndo_open;
   pNetDevOps->ndo_open = GobiUSBNetOpen;
   pGobiDev->mpUSBNetStop = pNetDevOps->ndo_stop;
   pNetDevOps->ndo_stop = GobiUSBNetStop;
#if 1 //quectel donot send dhcp request before ndis connect for uc20
   pNetDevOps->ndo_start_xmit = GobiUSBNetStartXmit2;
#else
   pNetDevOps->ndo_start_xmit = usbnet_start_xmit;
#endif
   pNetDevOps->ndo_tx_timeout = usbnet_tx_timeout;

#if defined(QUECTEL_WWAN_QMAP)
   pNetDevOps->ndo_do_ioctl = qmap_ndo_do_ioctl;
#endif

   pDev->net->netdev_ops = pNetDevOps;
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,31 ))
   memset( &(pGobiDev->mpNetDev->stats), 0, sizeof( struct net_device_stats ) );
#else
   memset( &(pGobiDev->mpNetDev->net->stats), 0, sizeof( struct net_device_stats ) );
#endif

   pGobiDev->mpIntf = pIntf;
   memset( &(pGobiDev->mMEID), '0', 14 );
   
   DBG( "Mac Address:\n" );
   PrintHex( &pGobiDev->mpNetDev->net->dev_addr[0], 6 );

   pGobiDev->mbQMIValid = false;
   memset( &pGobiDev->mQMIDev, 0, sizeof( sQMIDev ) );
   pGobiDev->mQMIDev.mbCdevIsInitialized = false;

   pGobiDev->mQMIDev.mpDevClass = gpClass;
   
#ifdef CONFIG_PM
   #if (LINUX_VERSION_CODE < KERNEL_VERSION( 2,6,29 ))
   init_completion( &pGobiDev->mAutoPM.mThreadDoWork );
   #endif
#endif /* CONFIG_PM */
   spin_lock_init( &pGobiDev->mQMIDev.mClientMemLock );

   // Default to device down
   pGobiDev->mDownReason = 0;

//#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,11,0 ))
   GobiSetDownReason( pGobiDev, NO_NDIS_CONNECTION );
   GobiSetDownReason( pGobiDev, NET_IFACE_STOPPED );
//#endif

   // Register QMI
   pGobiDev->mbMdm9x07 |= (pDev->udev->descriptor.idVendor == cpu_to_le16(0x2c7c));
   pGobiDev->mbMdm9x06 |= (pDev->udev->descriptor.idVendor == cpu_to_le16(0x2c7c) && pDev->udev->descriptor.idProduct == cpu_to_le16(0x0296));
   pGobiDev->mbRawIPMode = pGobiDev->mbMdm9x07;
   if ( pGobiDev->mbRawIPMode)
      pGobiDev->mpNetDev->net->flags |= IFF_NOARP;
#ifdef QUECTEL_BRIDGE_MODE
   memcpy(pGobiDev->mHostMAC, pDev->net->dev_addr, 6);
   pGobiDev->m_bridge_mode = bridge_mode;
#endif

#ifdef QUECTEL_REMOVE_TX_ZLP
	{
		struct remove_tx_zlp_config {
			__le32 enable;
		} __packed;

		struct remove_tx_zlp_config cfg;
		cfg.enable = cpu_to_le32(1);  //1-enable  0-disable

		usb_control_msg(
			interface_to_usbdev(pIntf),
			usb_sndctrlpipe(interface_to_usbdev(pIntf), 0),
			USB_CDC_SET_REMOVE_TX_ZLP_COMMAND,
			0x21, //USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE
			0,
			pIntf->cur_altsetting->desc.bInterfaceNumber,
			&cfg, sizeof(cfg), 100);
	}
#endif

	pGobiDev->m_qcrmcall_mode = qcrmcall_mode;
    
	if (pGobiDev->m_qcrmcall_mode) {
		INFO("AT$QCRMCALL MODE!");

		GobiClearDownReason( pGobiDev, NO_NDIS_CONNECTION );
		usb_control_msg(
			interface_to_usbdev(pIntf),
			usb_sndctrlpipe(interface_to_usbdev(pIntf), 0),
			0x22, //USB_CDC_REQ_SET_CONTROL_LINE_STATE
			0x21, //USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE
			1, //active CDC DTR
			pIntf->cur_altsetting->desc.bInterfaceNumber,
			NULL, 0, 100);
		status = 0;
	}
	else {
#if defined(QUECTEL_WWAN_QMAP)
		if (pGobiDev->mbRawIPMode) {
			unsigned idProduct = le16_to_cpu(pDev->udev->descriptor.idProduct);

			pGobiDev->qmap_mode = qmap_mode;
			if (pGobiDev->qmap_mode == 0) {
				if (idProduct == 0x0800 || idProduct == 0x0801) {
					pGobiDev->qmap_mode = 1;
				}
			}

			pGobiDev->qmap_version = 5;
			if (idProduct == 0x0800 || idProduct == 0x0801) {
				pGobiDev->qmap_version = 9;
			}
	      }

		if (pGobiDev->qmap_mode) {
			netif_carrier_off(pDev->net);
		}

		if (pGobiDev->qmap_mode > 1) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,35 )) //ab95bfe01f9872459c8678572ccadbf646badad0
			rtnl_lock();
			netdev_rx_handler_register(pDev->net, rmnet_usb_rx_handler, NULL);
			rtnl_unlock();
#endif
		}

#if defined(QUECTEL_UL_DATA_AGG)
		if (pGobiDev->qmap_mode) {
			struct ul_agg_ctx *agg_ctx = &pGobiDev->agg_ctx;

			agg_ctx->ul_data_aggregation_max_datagrams = 1;
			agg_ctx->ul_data_aggregation_max_size = 2048;
			agg_ctx->dl_minimum_padding = 0;
		}
#endif
#endif
		status = RegisterQMIDevice( pGobiDev );
	}
    
   if (status != 0)
   {
      // usbnet_disconnect() will call GobiNetDriverUnbind() which will call
      // DeregisterQMIDevice() to clean up any partially created QMI device
      usbnet_disconnect( pIntf );
      return status;
   }
   
#if defined(QUECTEL_WWAN_QMAP)
	tasklet_init(&pGobiDev->txq, rmnet_usb_tx_wake_queue, (unsigned long)pGobiDev);

	if (pGobiDev->qmap_mode > 1) {
		unsigned i;

		for (i = 0; i < pGobiDev->qmap_mode; i++) {
			qmap_register_device(pGobiDev, i);
		}
	} else {
		pGobiDev->mpQmapNetDev[0] = pDev->net;
	}
#endif

   pGobiDev->mbProbeDone = 1;
   // Success
   return 0;
}

static void GobiUSBNetDisconnect (struct usb_interface *intf) {
#if defined(QUECTEL_WWAN_QMAP)
	struct usbnet *pDev = usb_get_intfdata(intf);
	sGobiUSBNet * pGobiDev = (sGobiUSBNet *)pDev->data[0];
	unsigned i;

	if (pGobiDev->qmap_mode > 1) {
		for (i = 0; i < pGobiDev->qmap_mode; i++) {
			qmap_unregister_device(pGobiDev, i);
		}

	}

	tasklet_kill(&pGobiDev->txq);
#endif

	usbnet_disconnect(intf);
}

static struct usb_driver GobiNet =
{
   .name       = "GobiNet",
   .id_table   = QuecGobiVIDPIDTable,
   .probe      = GobiUSBNetProbe,
   .disconnect = GobiUSBNetDisconnect,
#ifdef CONFIG_PM
   .suspend    = GobiNetSuspend,
   .resume     = GobiNetResume,
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,27 ))   
   .reset_resume = GobiNetResetResume,
#endif   
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 2,6,18 ))   
   .supports_autosuspend = true,
#endif   
#endif /* CONFIG_PM */
};

/*===========================================================================
METHOD:
   GobiUSBNetModInit (Public Method)

DESCRIPTION:
   Initialize module
   Create device class
   Register out usb_driver struct

RETURN VALUE:
   int - 0 for success
         Negative errno for error
===========================================================================*/
static int __init GobiUSBNetModInit( void )
{
   gpClass = class_create( THIS_MODULE, "GobiQMI" );
   if (IS_ERR( gpClass ) == true)
   {
      DBG( "error at class_create %ld\n", PTR_ERR( gpClass ) );
      return -ENOMEM;
   }

   // This will be shown whenever driver is loaded
   printk( KERN_INFO "%s: %s\n", DRIVER_DESC, DRIVER_VERSION );

   return usb_register( &GobiNet );
}
module_init( GobiUSBNetModInit );

/*===========================================================================
METHOD:
   GobiUSBNetModExit (Public Method)

DESCRIPTION:
   Deregister module
   Destroy device class

RETURN VALUE:
   void
===========================================================================*/
static void __exit GobiUSBNetModExit( void )
{
   usb_deregister( &GobiNet );

   class_destroy( gpClass );
}
module_exit( GobiUSBNetModExit );

MODULE_VERSION( DRIVER_VERSION );
MODULE_AUTHOR( DRIVER_AUTHOR );
MODULE_DESCRIPTION( DRIVER_DESC );
MODULE_LICENSE("Dual BSD/GPL");
 
#ifdef bool
#undef bool
#endif

module_param_named( debug, quec_debug, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC( debug, "Debuging enabled or not" );

//module_param_named( interruptible, Quecinterruptible, int, S_IRUGO | S_IWUSR );
//MODULE_PARM_DESC( interruptible, "Listen for and return on user interrupt" );
module_param( txQueueLength, int, S_IRUGO | S_IWUSR );
MODULE_PARM_DESC( txQueueLength, 
                  "Number of IP packets which may be queued up for transmit" );

