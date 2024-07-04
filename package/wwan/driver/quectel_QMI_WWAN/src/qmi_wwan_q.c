
/*
 * Copyright (c) 2012  Bjørn Mork <bjorn@mork.no>
 *
 * The probing code is heavily inspired by cdc_ether, which is:
 * Copyright (C) 2003-2005 by David Brownell
 * Copyright (C) 2006 by Ole Andre Vadla Ravnas (ActiveSync)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/time.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,16,0) //8b094cd03b4a3793220d8d8d86a173bfea8c285b
#include <linux/timekeeping.h>
#else
#define timespec64  timespec
#define ktime_get_ts64 ktime_get_ts
#define timespec64_sub timespec_sub
#endif
#include <net/arp.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <linux/usb/usbnet.h>
#include <linux/usb/cdc-wdm.h>

#ifndef ETH_P_MAP
#define ETH_P_MAP 0xDA1A
#endif

#if (ETH_P_MAP == 0x00F9)
#undef ETH_P_MAP
#define ETH_P_MAP 0xDA1A
#endif

#ifndef ARPHRD_RAWIP
#define ARPHRD_RAWIP ARPHRD_NONE
#endif

#ifdef CONFIG_PINCTRL_IPQ807x
#define CONFIG_QCA_NSS_DRV
//#define CONFIG_QCA_NSS_PACKET_FILTER
#endif

#define _RMNET_NSS_H_
#define _RMENT_NSS_H_
struct rmnet_nss_cb {
        int (*nss_create)(struct net_device *dev);
        int (*nss_free)(struct net_device *dev);
        int (*nss_tx)(struct sk_buff *skb);
};
static struct rmnet_nss_cb __read_mostly *nss_cb = NULL;
#if defined(CONFIG_PINCTRL_IPQ807x) || defined(CONFIG_PINCTRL_IPQ5018)
#ifdef CONFIG_RMNET_DATA
#define CONFIG_QCA_NSS_DRV
/* define at qsdk/qca/src/linux-4.4/net/rmnet_data/rmnet_data_main.c */
/* set at qsdk/qca/src/data-kernel/drivers/rmnet-nss/rmnet_nss.c */
extern struct rmnet_nss_cb *rmnet_nss_callbacks __rcu __read_mostly;
#endif
#endif

/* This driver supports wwan (3G/LTE/?) devices using a vendor
 * specific management protocol called Qualcomm MSM Interface (QMI) -
 * in addition to the more common AT commands over serial interface
 * management
 *
 * QMI is wrapped in CDC, using CDC encapsulated commands on the
 * control ("master") interface of a two-interface CDC Union
 * resembling standard CDC ECM.  The devices do not use the control
 * interface for any other CDC messages.  Most likely because the
 * management protocol is used in place of the standard CDC
 * notifications NOTIFY_NETWORK_CONNECTION and NOTIFY_SPEED_CHANGE
 *
 * Alternatively, control and data functions can be combined in a
 * single USB interface.
 *
 * Handling a protocol like QMI is out of the scope for any driver.
 * It is exported as a character device using the cdc-wdm driver as
 * a subdriver, enabling userspace applications ("modem managers") to
 * handle it.
 *
 * These devices may alternatively/additionally be configured using AT
 * commands on a serial interface
 */
#define VERSION_NUMBER "V1.2.2"
#define QUECTEL_WWAN_VERSION "Quectel_Linux&Android_QMI_WWAN_Driver_"VERSION_NUMBER
static const char driver_name[] = "qmi_wwan_q";

/* driver specific data */
struct qmi_wwan_state {
	struct usb_driver *subdriver;
	atomic_t pmcount;
	unsigned long unused;
	struct usb_interface *control;
	struct usb_interface *data;
};

/* default ethernet address used by the modem */
static const u8 default_modem_addr[ETH_ALEN] = {0x02, 0x50, 0xf3};

#if 1 //Added by Quectel
/*
    Quectel_WCDMA&LTE_Linux_USB_Driver_User_Guide_V1.9.pdf
    5.6.	Test QMAP on GobiNet or QMI WWAN
    0 - no QMAP
    1 - QMAP (Aggregation protocol)
    X - QMAP (Multiplexing and Aggregation protocol)
*/
#define QUECTEL_WWAN_QMAP 4 //MAX is 7

#if defined(QUECTEL_WWAN_QMAP)
#define QUECTEL_QMAP_MUX_ID 0x81

static uint __read_mostly qmap_mode = 0;
module_param( qmap_mode, uint, S_IRUGO);
module_param_named( rx_qmap, qmap_mode, uint, S_IRUGO );
#endif

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE) || defined(CONFIG_BRIDGE_LAN)
#define QUECTEL_BRIDGE_MODE
#endif

#ifdef QUECTEL_BRIDGE_MODE
static uint __read_mostly bridge_mode = 0/*|BIT(1)*/;
module_param( bridge_mode, uint, S_IRUGO );
#endif

#ifdef CONFIG_BRIDGE_LAN
static const u8 broadcast_mac_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif

//#define QMI_NETDEV_ONE_CARD_MODE
static uint __read_mostly one_card_mode = 0;

#if defined(QUECTEL_WWAN_QMAP)
#define QUECTEL_UL_DATA_AGG 1

#if defined(QUECTEL_UL_DATA_AGG)
struct tx_agg_ctx {
	/* QMIWDS_ADMIN_SET_DATA_FORMAT_RESP TLV_0x17 and TLV_0x18 */
	uint ul_data_aggregation_max_datagrams; //UplinkDataAggregationMaxDatagramsTlv
	uint ul_data_aggregation_max_size; //UplinkDataAggregationMaxSizeTlv
	uint dl_minimum_padding; //0x1A
};
#endif

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

typedef struct sQmiWwanQmap
{
	struct usbnet *mpNetDev;
	struct driver_info driver_info;
	atomic_t refcount;
	struct net_device *mpQmapNetDev[QUECTEL_WWAN_QMAP];
	uint link_state;
	uint qmap_mode;
	uint qmap_size;
	uint qmap_version;

#if defined(QUECTEL_UL_DATA_AGG)
	struct tx_agg_ctx tx_ctx;
	struct tasklet_struct	txq;
	struct tasklet_struct usbnet_bh;
#endif

#ifdef QUECTEL_BRIDGE_MODE
	uint bridge_mode;
	uint bridge_ipv4;
	unsigned char bridge_mac[ETH_ALEN];
#ifdef CONFIG_BRIDGE_LAN
    unsigned char bridge_self_mac[ETH_ALEN];
#endif
#endif
	uint use_rmnet_usb;
	RMNET_INFO rmnet_info;
} sQmiWwanQmap;

#if LINUX_VERSION_CODE > KERNEL_VERSION(3,13,0) //8f84985fec10de64a6b4cdfea45f2b0ab8f07c78
#define MHI_NETDEV_STATUS64
#endif
struct qmap_priv {
	struct usbnet *dev;
	struct net_device *real_dev;
	struct net_device *self_dev;
	u8 offset_id;
	u8 mux_id;
	u8 qmap_version; // 5~v1, 9~v5
	u8 link_state;

#if defined(MHI_NETDEV_STATUS64)
	struct pcpu_sw_netstats __percpu *stats64;
#endif

	spinlock_t agg_lock;
	struct sk_buff *agg_skb;
	unsigned agg_count;
	struct timespec64 agg_time;
	struct hrtimer agg_hrtimer;
	struct work_struct agg_wq;
	
#ifdef QUECTEL_BRIDGE_MODE
	uint bridge_mode;
	uint bridge_ipv4;
	unsigned char bridge_mac[ETH_ALEN];
#ifdef CONFIG_BRIDGE_LAN
    unsigned char bridge_self_mac[ETH_ALEN];
#endif
#endif
	uint use_qca_nss;	
};

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

#ifdef QUECTEL_BRIDGE_MODE
static int is_qmap_netdev(const struct net_device *netdev);
#endif
#endif

static const struct driver_info rmnet_usb_info;

#ifdef QUECTEL_BRIDGE_MODE
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
            reply = arp_create(ARPOP_REPLY, ETH_P_ARP, *((__be32 *)sip), net, *((__be32 *)tip), sha, default_modem_addr, sha);

        if (reply) {
            skb_reset_mac_header(reply);
            __skb_pull(reply, skb_network_offset(reply));
            reply->ip_summed = CHECKSUM_UNNECESSARY;
            reply->pkt_type = PACKET_HOST;

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
            netif_rx_ni(reply);
#else
            netif_rx(reply);
#endif
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
	
#ifdef CONFIG_BRIDGE_LAN
	//bridge Lan IP 192.168.0.0
	if (ehdr->h_proto == htons(ETH_P_IP) && (iph->daddr & 0xFFFF) == 0xA8C0)
	{
	    struct sk_buff *reply = skb_copy(skb, GFP_ATOMIC);
	    ehdr = eth_hdr(reply);
		
		memcpy(ehdr->h_source, default_modem_addr, ETH_ALEN);
		if(is_qmap_netdev(net))
	    {
			struct qmap_priv *priv = netdev_priv(net);
	        memcpy(ehdr->h_dest, priv->bridge_self_mac, ETH_ALEN);
	    }
	    else
	    {
			struct usbnet * usbnetdev = netdev_priv(net);
			struct qmi_wwan_state *info = (void *)&usbnetdev->data;
			sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	        memcpy(ehdr->h_dest, pQmapDev->bridge_self_mac, ETH_ALEN);
	    }
		
		//pr_info("%s br rx pkt addr: %02x:%02x:%02x:%02x:%02x:%02x -> %02x:%02x:%02x:%02x:%02x:%02x\n", netdev_name(net),
        //    ehdr->h_source[0], ehdr->h_source[1], ehdr->h_source[2], ehdr->h_source[3], ehdr->h_source[4], ehdr->h_source[5],
        //    ehdr->h_dest[0], ehdr->h_dest[1], ehdr->h_dest[2], ehdr->h_dest[3], ehdr->h_dest[4], ehdr->h_dest[5]);
		
		skb_reset_mac_header(reply);
        __skb_pull(reply, skb_network_offset(reply));
        reply->ip_summed = CHECKSUM_UNNECESSARY;
        reply->pkt_type = PACKET_HOST;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
        netif_rx_ni(reply);
#else
        netif_rx(reply);
#endif
		return NULL;
	}
#endif

	if (memcmp(ehdr->h_source, bridge_mac, ETH_ALEN)) {
		return NULL;
	}

	return skb;
}

static void bridge_mode_rx_fixup(sQmiWwanQmap *pQmapDev, struct net_device *net, struct sk_buff *skb) {
	uint bridge_mode = 0;
	unsigned char *bridge_mac;

	if (pQmapDev->qmap_mode > 1 || pQmapDev->use_rmnet_usb == 1) {
		struct qmap_priv *priv = netdev_priv(net);
		bridge_mode = priv->bridge_mode;
		bridge_mac = priv->bridge_mac;
	}
	else {
		bridge_mode = pQmapDev->bridge_mode;
		bridge_mac = pQmapDev->bridge_mac;
	}

	if (bridge_mode)
		memcpy(eth_hdr(skb)->h_dest, bridge_mac, ETH_ALEN);
	else
		memcpy(eth_hdr(skb)->h_dest, net->dev_addr, ETH_ALEN);
}
#endif

#if defined(QUECTEL_WWAN_QMAP)
static ssize_t qmap_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *netdev = to_net_dev(dev);
	struct usbnet * usbnetdev = netdev_priv( netdev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	return snprintf(buf, PAGE_SIZE, "%d\n",  pQmapDev->qmap_mode);
}

static DEVICE_ATTR(qmap_mode, S_IRUGO, qmap_mode_show, NULL);

static ssize_t qmap_size_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *netdev = to_net_dev(dev);
	struct usbnet * usbnetdev = netdev_priv( netdev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	return snprintf(buf, PAGE_SIZE, "%u\n",  pQmapDev->qmap_size);
}

static DEVICE_ATTR(qmap_size, S_IRUGO, qmap_size_show, NULL);

static ssize_t link_state_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *netdev = to_net_dev(dev);
	struct usbnet * usbnetdev = netdev_priv( netdev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	return snprintf(buf, PAGE_SIZE, "0x%x\n",  pQmapDev->link_state);
}

static ssize_t link_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *netdev = to_net_dev(dev);
	struct usbnet * usbnetdev = netdev_priv( netdev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	unsigned link_state = 0;
	unsigned old_link = pQmapDev->link_state;
	uint offset_id = 0;

	link_state = simple_strtoul(buf, NULL, 0);

	if (pQmapDev->qmap_mode == 1) {
		pQmapDev->link_state = !!link_state;
	}
	else if (pQmapDev->qmap_mode > 1) {
		offset_id = ((link_state&0x7F) - 1);

		if (offset_id >= pQmapDev->qmap_mode) {
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

		if (usbnetdev->net->flags & IFF_UP) {
			if (pQmapDev->link_state) {
				netif_carrier_on(usbnetdev->net);
			}
		}

		if (qmap_net && qmap_net != netdev) {
			struct qmap_priv *priv = netdev_priv(qmap_net);

			priv->link_state = !!(pQmapDev->link_state & (1 << offset_id));

			if (qmap_net->flags & IFF_UP) {
				if (priv->link_state) {
					netif_carrier_on(qmap_net);
					if (netif_queue_stopped(qmap_net) && !netif_queue_stopped(usbnetdev->net))
						netif_wake_queue(qmap_net);
				}
				else {
					netif_carrier_off(qmap_net);
				}
			}
		}

		if (usbnetdev->net->flags & IFF_UP) { 
			if (!pQmapDev->link_state) {
				netif_carrier_off(usbnetdev->net);
			}
		}

		dev_info(dev, "link_state 0x%x -> 0x%x\n", old_link, pQmapDev->link_state);
	}

	return count;
}

#ifdef QUECTEL_BRIDGE_MODE
static ssize_t bridge_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *netdev = to_net_dev(dev);
	uint old_mode = 0;
	uint bridge_mode = simple_strtoul(buf, NULL, 0);

	if (netdev->type != ARPHRD_ETHER) {
		return count;
	}

	if (is_qmap_netdev(netdev)) {
		struct qmap_priv *priv = netdev_priv(netdev);
		old_mode = priv->bridge_mode;
		priv->bridge_mode = bridge_mode;
	}
	else {
		struct usbnet * usbnetdev = netdev_priv( netdev );
		struct qmi_wwan_state *info = (void *)&usbnetdev->data;
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
		old_mode = pQmapDev->bridge_mode;
		pQmapDev->bridge_mode = bridge_mode;
	}

	if (old_mode != bridge_mode) {
		dev_info(dev, "bridge_mode change to 0x%x\n", bridge_mode);
	}

	return count;
}

static ssize_t bridge_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *netdev = to_net_dev(dev);
	uint bridge_mode = 0;

	if (is_qmap_netdev(netdev)) {
		struct qmap_priv *priv = netdev_priv(netdev);
		bridge_mode = priv->bridge_mode;
	}
	else {
		struct usbnet * usbnetdev = netdev_priv( netdev );
		struct qmi_wwan_state *info = (void *)&usbnetdev->data;
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
		bridge_mode = pQmapDev->bridge_mode;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", bridge_mode);
}

static ssize_t bridge_ipv4_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *netdev = to_net_dev(dev);
	unsigned int bridge_ipv4 = 0;
	unsigned char ipv4[4];

	if (is_qmap_netdev(netdev)) {
		struct qmap_priv *priv = netdev_priv(netdev);
		bridge_ipv4 = priv->bridge_ipv4;
	}
	else {
		struct usbnet * usbnetdev = netdev_priv( netdev );
		struct qmi_wwan_state *info = (void *)&usbnetdev->data;
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
		bridge_ipv4 = pQmapDev->bridge_ipv4;
	}

	ipv4[0]  = (bridge_ipv4 >> 24) & 0xFF;
	ipv4[1]  = (bridge_ipv4 >> 16) & 0xFF;
	ipv4[2]  = (bridge_ipv4 >> 8) & 0xFF;
	ipv4[3]  = (bridge_ipv4 >> 0) & 0xFF;

	return snprintf(buf, PAGE_SIZE, "%d.%d.%d.%d\n",  ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
}

static ssize_t bridge_ipv4_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *netdev = to_net_dev(dev);

	if (is_qmap_netdev(netdev)) {
		struct qmap_priv *priv = netdev_priv(netdev);
		priv->bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}
	else {
		struct usbnet * usbnetdev = netdev_priv( netdev );
		struct qmi_wwan_state *info = (void *)&usbnetdev->data;
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
		pQmapDev->bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}

	return count;
}
#endif

static DEVICE_ATTR(link_state, S_IWUSR | S_IRUGO, link_state_show, link_state_store);
#ifdef QUECTEL_BRIDGE_MODE
static DEVICE_ATTR(bridge_mode, S_IWUSR | S_IRUGO, bridge_mode_show, bridge_mode_store);
static DEVICE_ATTR(bridge_ipv4, S_IWUSR | S_IRUGO, bridge_ipv4_show, bridge_ipv4_store);
#endif

static struct attribute *qmi_wwan_sysfs_attrs[] = {
	&dev_attr_link_state.attr,
	&dev_attr_qmap_mode.attr,
	&dev_attr_qmap_size.attr,
#ifdef QUECTEL_BRIDGE_MODE
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
#endif
	NULL,
};

static struct attribute_group qmi_wwan_sysfs_attr_group = {
	.attrs = qmi_wwan_sysfs_attrs,
};

#ifdef QUECTEL_BRIDGE_MODE
static struct attribute *qmi_qmap_sysfs_attrs[] = {
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
	NULL,
};

static struct attribute_group qmi_qmap_sysfs_attr_group = {
	.attrs = qmi_qmap_sysfs_attrs,
};
#endif

static int qmap_open(struct net_device *qmap_net)
{
	struct qmap_priv *priv = netdev_priv(qmap_net);
	struct net_device *real_dev = priv->real_dev;

	//printk("%s %s real_dev %d %d %d %d+++\n", __func__, dev->name,
	//    netif_carrier_ok(real_dev), netif_queue_stopped(real_dev), netif_carrier_ok(dev), netif_queue_stopped(dev));

	if (!(priv->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	if (priv->link_state) {
		netif_carrier_on(real_dev);
		netif_carrier_on(qmap_net);
		if (netif_queue_stopped(qmap_net) && !netif_queue_stopped(real_dev))
			netif_wake_queue(qmap_net);
	}
	//printk("%s %s real_dev %d %d %d %d---\n", __func__, dev->name,
	//    netif_carrier_ok(real_dev), netif_queue_stopped(real_dev), netif_carrier_ok(dev), netif_queue_stopped(dev));

	return 0;
}

static int qmap_stop(struct net_device *qmap_net)
{
	//printk("%s %s %d %d+++\n", __func__, dev->name,
	//   netif_carrier_ok(dev), netif_queue_stopped(dev));

	netif_carrier_off(qmap_net);
	return 0;
}

static void qmap_wake_queue(sQmiWwanQmap *pQmapDev)
{
	uint i = 0;

	if (!pQmapDev || !pQmapDev->use_rmnet_usb)
		return;
		
	for (i = 0; i < pQmapDev->qmap_mode; i++) {
		struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];

		if (qmap_net && netif_carrier_ok(qmap_net) && netif_queue_stopped(qmap_net)) {
			netif_wake_queue(qmap_net);
		}
	}
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

static void rmnet_vnd_update_rx_stats(struct net_device *net,
			unsigned rx_packets, unsigned rx_bytes) {
#if defined(MHI_NETDEV_STATUS64)
	struct qmap_priv *dev = netdev_priv(net);
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(dev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
	stats64->rx_packets += rx_packets;
	stats64->rx_bytes += rx_bytes;
#else
	u64_stats_add(&stats64->rx_packets, rx_packets);
	u64_stats_add(&stats64->rx_bytes, rx_bytes);
#endif
	u64_stats_update_end(&stats64->syncp);
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
	net->stats.rx_packets += rx_packets;
	net->stats.rx_bytes += rx_bytes;
#else
	u64_stats_add(&net->stats.rx_packets, rx_packets);
	u64_stats_add(&net->stats.rx_bytes, rx_bytes);
#endif
#endif
}

static void rmnet_vnd_update_tx_stats(struct net_device *net,
			unsigned tx_packets, unsigned tx_bytes) {	
#if defined(MHI_NETDEV_STATUS64)
	struct qmap_priv *dev = netdev_priv(net);
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(dev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
	stats64->tx_packets += tx_packets;
	stats64->tx_bytes += tx_bytes;
#else
	u64_stats_add(&stats64->tx_packets, tx_packets);
	u64_stats_add(&stats64->tx_bytes, tx_bytes);
#endif
	u64_stats_update_end(&stats64->syncp);
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
	net->stats.tx_packets += tx_packets;
	net->stats.tx_bytes += tx_bytes;
#else
	u64_stats_add(&net->stats.tx_packets, tx_packets);
	u64_stats_add(&net->tx_bytes, tx_bytes);
#endif
#endif
}

#if defined(MHI_NETDEV_STATUS64)
static struct rtnl_link_stats64 *_rmnet_vnd_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats)
{
	struct qmap_priv *dev = netdev_priv(net);
	unsigned int start;
	int cpu;

	netdev_stats_to_stats64(stats, &net->stats);

	if (nss_cb && dev->use_qca_nss) { // rmnet_nss.c:rmnet_nss_tx() will update rx stats
		stats->rx_packets = 0;
		stats->rx_bytes = 0;
	}

	for_each_possible_cpu(cpu) {
		struct pcpu_sw_netstats *stats64;
		u64 rx_packets, rx_bytes;
		u64 tx_packets, tx_bytes;

		stats64 = per_cpu_ptr(dev->stats64, cpu);

		do {
			start = u64_stats_fetch_begin_irq(&stats64->syncp);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
			rx_packets = stats64->rx_packets;
			rx_bytes = stats64->rx_bytes;
			tx_packets = stats64->tx_packets;
			tx_bytes = stats64->tx_bytes;
#else
			rx_packets = u64_stats_read(&stats64->rx_packets);
			rx_bytes = u64_stats_read(&stats64->rx_bytes);
			tx_packets = u64_stats_read(&stats64->tx_packets);
			tx_bytes = u64_stats_read(&stats64->tx_bytes);
#endif
		} while (u64_stats_fetch_retry_irq(&stats64->syncp, start));


		stats->rx_packets += rx_packets;
		stats->rx_bytes += rx_bytes;
		stats->tx_packets += tx_packets;
		stats->tx_bytes += tx_bytes;

	}

	return stats;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,10,0 )) //bc1f44709cf27fb2a5766cadafe7e2ad5e9cb221
static void rmnet_vnd_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats) {
	_rmnet_vnd_get_stats64(net, stats);
}
#else
static struct rtnl_link_stats64 *rmnet_vnd_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats) {
	return _rmnet_vnd_get_stats64(net, stats);
}
#endif
#endif

#if defined(QUECTEL_UL_DATA_AGG)
static void usbnet_bh(unsigned long data) {
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)data;
	struct tasklet_struct *t = &pQmapDev->usbnet_bh;
	bool use_callback = false;

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 5,8,0 )) //c955e329bb9d44fab75cf2116542fcc0de0473c5
	use_callback = t->use_callback;
	if (use_callback)
		t->callback(&pQmapDev->mpNetDev->bh);
#endif

	if (!use_callback)
		t->func(t->data);

	if (!netif_queue_stopped(pQmapDev->mpNetDev->net)) {
		qmap_wake_queue((sQmiWwanQmap *)data);
	}
}

static void rmnet_usb_tx_wake_queue(unsigned long data) {
	qmap_wake_queue((sQmiWwanQmap *)data);
}

#if 0
static void rmnet_usb_tx_skb_destructor(struct sk_buff *skb) {
	struct net_device	*net = skb->dev;
	struct usbnet * dev = netdev_priv( net );
	struct qmi_wwan_state *info = (void *)&dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	if (pQmapDev && pQmapDev->use_rmnet_usb) {
		int i;
		
		for (i = 0; i < pQmapDev->qmap_mode; i++) {
			struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];

			if (qmap_net && netif_carrier_ok(qmap_net) && netif_queue_stopped(qmap_net)) {
				tasklet_schedule(&pQmapDev->txq);
				break;
			}
		}
	}
}
#endif

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
		int err;
#if 0
		if (!skb->destructor)
			skb->destructor = rmnet_usb_tx_skb_destructor;
#endif
		err = dev_queue_xmit(skb);
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

static long agg_time_limit __read_mostly = 1000000L; //reduce this time, can get better TPUT performance, but will increase USB interrupts
module_param(agg_time_limit, long, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(agg_time_limit, "Maximum time packets sit in the agg buf");

static long agg_bypass_time __read_mostly = 10000000L;
module_param(agg_bypass_time, long, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(agg_bypass_time, "Skip agg when apart spaced more than this");

static int rmnet_usb_tx_agg(struct sk_buff *skb, struct qmap_priv *priv) {
	struct qmi_wwan_state *info = (void *)&priv->dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	struct tx_agg_ctx *ctx = &pQmapDev->tx_ctx;
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

	if (ctx->ul_data_aggregation_max_datagrams == 1) {
		skb->protocol = htons(ETH_P_MAP);
		skb->dev = priv->real_dev;
#if 0
		if (!skb->destructor)
			skb->destructor = rmnet_usb_tx_skb_destructor;
#endif
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
		if ((priv->agg_skb->len + skb->len) < ctx->ul_data_aggregation_max_size) {
			memcpy(skb_put(priv->agg_skb, skb->len), skb->data, skb->len);
			priv->agg_count++;

			if (diff.tv_sec > 0 || diff.tv_nsec > agg_time_limit) {
				ready2send = 1;
			}
			else if (priv->agg_count == ctx->ul_data_aggregation_max_datagrams) {
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
			priv->agg_skb = alloc_skb(ctx->ul_data_aggregation_max_size, GFP_ATOMIC);
			if (priv->agg_skb) {
				skb_reset_network_header(priv->agg_skb); //protocol da1a is buggy, dev wwan0
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
#if 0
		if (!agg_skb->destructor)
			agg_skb->destructor = rmnet_usb_tx_skb_destructor;
#endif
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

static netdev_tx_t rmnet_vnd_start_xmit(struct sk_buff *skb,
					struct net_device *pNet)
{
	int err;
	struct qmap_priv *priv = netdev_priv(pNet);

	if (netif_queue_stopped(priv->real_dev)) {
		netif_stop_queue(pNet);
		return NETDEV_TX_BUSY;
	}

	//printk("%s 1 skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);
	if (pNet->type == ARPHRD_ETHER) {
		skb_reset_mac_header(skb);

#ifdef QUECTEL_BRIDGE_MODE
		if (priv->bridge_mode && bridge_mode_tx_fixup(pNet, skb, priv->bridge_ipv4, priv->bridge_mac) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
#endif

		if (skb_pull(skb, ETH_HLEN) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}
	//printk("%s 2 skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

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
	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	err = rmnet_usb_tx_agg(skb, priv);

	return err;
}

static int rmnet_vnd_change_mtu(struct net_device *rmnet_dev, int new_mtu)
{
	if (new_mtu < 0)
		return -EINVAL;

  if (new_mtu > 1500)
    printk("warning, set mtu greater than 1500, %d\n", new_mtu);

	rmnet_dev->mtu = new_mtu;
	return 0;
}

/* drivers may override default ethtool_ops in their bind() routine */
static const struct ethtool_ops rmnet_vnd_ethtool_ops = {
	.get_link		= ethtool_op_get_link,
};

static const struct net_device_ops rmnet_vnd_ops = {
	.ndo_open       = qmap_open,
	.ndo_stop       = qmap_stop,
	.ndo_start_xmit = rmnet_vnd_start_xmit,
	.ndo_change_mtu = rmnet_vnd_change_mtu,
#if defined(MHI_NETDEV_STATUS64)
	.ndo_get_stats64	= rmnet_vnd_get_stats64,
#endif
};

static void rmnet_usb_ether_setup(struct net_device *rmnet_dev)
{
	ether_setup(rmnet_dev);

	rmnet_dev->flags |= IFF_NOARP;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,10,0)
  rmnet_dev->max_mtu = 65535;
#endif

	rmnet_dev->ethtool_ops = &rmnet_vnd_ethtool_ops;
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
}

static void rmnet_usb_rawip_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->needed_headroom = 16;

	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
	rmnet_dev->type = ARPHRD_RAWIP;
	rmnet_dev->hard_header_len = 0;
	rmnet_dev->flags |= IFF_NOARP;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

	rmnet_dev->ethtool_ops = &rmnet_vnd_ethtool_ops;
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
}

static rx_handler_result_t qca_nss_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;

	if (!skb)
		return RX_HANDLER_CONSUMED;
	
	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	/* Check this so that we dont loop around netif_receive_skb */
	if (skb->cb[0] == 1) {
		skb->cb[0] = 0;

		return RX_HANDLER_PASS;
	}

	if (nss_cb) {
		nss_cb->nss_tx(skb);
		return RX_HANDLER_CONSUMED;
	}

	return RX_HANDLER_PASS;
}

static int qmap_register_device(sQmiWwanQmap * pDev, u8 offset_id)
{
	struct net_device *real_dev = pDev->mpNetDev->net;
	struct net_device *qmap_net;
	struct qmap_priv *priv;
	int err;
	char name[IFNAMSIZ];
	int use_qca_nss = !!nss_cb;

	sprintf(name, "%s_%d", real_dev->name, offset_id + 1);
#ifdef NET_NAME_UNKNOWN
	qmap_net = alloc_netdev(sizeof(struct qmap_priv), name,
				NET_NAME_UNKNOWN, rmnet_usb_ether_setup);
#else
	qmap_net = alloc_netdev(sizeof(struct qmap_priv), name,
				rmnet_usb_ether_setup);
#endif
    if (!qmap_net)
        return -ENOBUFS;

    SET_NETDEV_DEV(qmap_net, &real_dev->dev);
    priv = netdev_priv(qmap_net);
    priv->offset_id = offset_id;
    priv->real_dev = real_dev;
    priv->self_dev = qmap_net;
    priv->dev = pDev->mpNetDev;
    priv->qmap_version = pDev->qmap_version;
    priv->mux_id = QUECTEL_QMAP_MUX_ID + offset_id;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 17, 0)
    memcpy (qmap_net->dev_addr, real_dev->dev_addr, ETH_ALEN);
#else
    eth_hw_addr_set (real_dev, qmap_net->dev_addr);
#endif

#ifdef QUECTEL_BRIDGE_MODE
	priv->bridge_mode = !!(pDev->bridge_mode & BIT(offset_id));
	qmap_net->sysfs_groups[0] = &qmi_qmap_sysfs_attr_group;
	if (priv->bridge_mode)
		use_qca_nss = 0;
#ifdef CONFIG_BRIDGE_LAN
    memcpy(priv->bridge_self_mac, broadcast_mac_addr, ETH_ALEN);
#endif
#endif

	if (nss_cb && use_qca_nss) {
		rmnet_usb_rawip_setup(qmap_net);
	}

	priv->agg_skb = NULL;
	priv->agg_count = 0;
	hrtimer_init(&priv->agg_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->agg_hrtimer.function = rmnet_usb_tx_agg_timer_cb;
	INIT_WORK(&priv->agg_wq, rmnet_usb_tx_agg_work);
	ktime_get_ts64(&priv->agg_time);
	spin_lock_init(&priv->agg_lock);
	priv->use_qca_nss = 0;

#if defined(MHI_NETDEV_STATUS64)
	priv->stats64 = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!priv->stats64) {
		err = -ENOBUFS;
		goto out_free_newdev;
	}
#endif

	err = register_netdev(qmap_net);
	if (err)
		dev_info(&real_dev->dev, "%s(%s)=%d\n", __func__, qmap_net->name, err);
	if (err < 0)
		goto out_free_newdev;
	netif_device_attach (qmap_net);
	netif_carrier_off(qmap_net);

	if (nss_cb && use_qca_nss) {
		int rc = nss_cb->nss_create(qmap_net);
		if (rc) {
			/* Log, but don't fail the device creation */
			netdev_err(qmap_net, "Device will not use NSS path: %d\n", rc);
		} else {
			priv->use_qca_nss = 1;
			netdev_info(qmap_net, "NSS context created\n");
			rtnl_lock();
			netdev_rx_handler_register(qmap_net, qca_nss_rx_handler, NULL);
			rtnl_unlock();			
		}
	}

	strcpy(pDev->rmnet_info.ifname[offset_id], qmap_net->name);
	pDev->rmnet_info.mux_id[offset_id] = priv->mux_id;

	pDev->mpQmapNetDev[offset_id] = qmap_net;

	dev_info(&real_dev->dev, "%s %s\n", __func__, qmap_net->name);

	return 0;

out_free_newdev:
	free_netdev(qmap_net);
	return err;
}

static void qmap_unregister_device(sQmiWwanQmap * pDev, u8 offset_id) {
	struct net_device *qmap_net = pDev->mpQmapNetDev[offset_id];

	if (qmap_net != NULL && qmap_net != pDev->mpNetDev->net) {
		struct qmap_priv *priv = netdev_priv(qmap_net);
		unsigned long flags;

		pr_info("qmap_unregister_device(%s)\n", qmap_net->name);
		pDev->mpQmapNetDev[offset_id] = NULL;
		netif_carrier_off( qmap_net );
		netif_stop_queue( qmap_net );
		
		hrtimer_cancel(&priv->agg_hrtimer);
		cancel_work_sync(&priv->agg_wq);
		spin_lock_irqsave(&priv->agg_lock, flags);
		if (priv->agg_skb) {
			kfree_skb(priv->agg_skb);
		}
		spin_unlock_irqrestore(&priv->agg_lock, flags);

		if (nss_cb && priv->use_qca_nss) {
			rtnl_lock();
			netdev_rx_handler_unregister(qmap_net);
			rtnl_unlock();
			nss_cb->nss_free(qmap_net);
		}

#if defined(MHI_NETDEV_STATUS64)
		free_percpu(priv->stats64);
#endif
		unregister_netdev (qmap_net);
		free_netdev(qmap_net);
	}
}

typedef struct {
    unsigned int size;
    unsigned int rx_urb_size;
    unsigned int ep_type;
    unsigned int iface_id;
    unsigned int MuxId;
    unsigned int ul_data_aggregation_max_datagrams; //0x17
    unsigned int ul_data_aggregation_max_size ;//0x18
    unsigned int dl_minimum_padding; //0x1A
} QMAP_SETTING;

#ifdef CONFIG_BRIDGE_LAN
typedef struct {
    u8 id;
    u8 brmac[ETH_ALEN];
} BRMAC_SETTING;
#endif

int qma_setting_store(struct device *dev, QMAP_SETTING *qmap_settings, size_t size) {
	struct net_device *netdev = to_net_dev(dev);
	struct usbnet * usbnetdev = netdev_priv( netdev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	if (qmap_settings->size != size) {
		dev_err(dev, "ERROR: qmap_settings.size donot match!\n");
		return -EOPNOTSUPP;
	}

#ifdef QUECTEL_UL_DATA_AGG
	netif_tx_lock_bh(netdev);
	if (pQmapDev->tx_ctx.ul_data_aggregation_max_datagrams == 1 && qmap_settings->ul_data_aggregation_max_datagrams > 1) {
		pQmapDev->tx_ctx.ul_data_aggregation_max_datagrams = qmap_settings->ul_data_aggregation_max_datagrams;
		pQmapDev->tx_ctx.ul_data_aggregation_max_size = qmap_settings->ul_data_aggregation_max_size;
		pQmapDev->tx_ctx.dl_minimum_padding = qmap_settings->dl_minimum_padding;
		dev_info(dev, "ul_data_aggregation_max_datagrams=%d, ul_data_aggregation_max_size=%d, dl_minimum_padding=%d\n",
			pQmapDev->tx_ctx.ul_data_aggregation_max_datagrams,
			pQmapDev->tx_ctx.ul_data_aggregation_max_size,
			pQmapDev->tx_ctx.dl_minimum_padding);
	}
	netif_tx_unlock_bh(netdev);
	return 0;
#endif

	return -EOPNOTSUPP;
}

static int qmap_ndo_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd) {
	struct usbnet * usbnetdev = netdev_priv( dev );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	int rc = -EOPNOTSUPP;
	uint link_state = 0;
 	QMAP_SETTING qmap_settings = {0};
#ifdef CONFIG_BRIDGE_LAN
 	BRMAC_SETTING brmac_settings = {0};
#endif
 
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
		rc = copy_from_user(&qmap_settings, ifr->ifr_ifru.ifru_data, sizeof(qmap_settings));
		if (!rc) {
			rc = qma_setting_store(&dev->dev, &qmap_settings, sizeof(qmap_settings));
		}
	break;

	case 0x89F3: //SIOCDEVPRIVATE
		if (pQmapDev->use_rmnet_usb) {
			uint i;

			for (i = 0; i < pQmapDev->qmap_mode; i++) {
				struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];

				if (!qmap_net)
					break;

				strcpy(pQmapDev->rmnet_info.ifname[i], qmap_net->name);
			}
			rc = copy_to_user(ifr->ifr_ifru.ifru_data, &pQmapDev->rmnet_info, sizeof(pQmapDev->rmnet_info));
		}
	break;
	
#ifdef CONFIG_BRIDGE_LAN
    case 0x89F4: //SIOCDEVPRIVATE
        rc = copy_from_user(&brmac_settings, ifr->ifr_ifru.ifru_data, sizeof(brmac_settings));
		if (pQmapDev->use_rmnet_usb && brmac_settings.id < qmap_mode) {
		    struct net_device *qmap_net = pQmapDev->mpQmapNetDev[brmac_settings.id];
		    struct qmap_priv *priv = netdev_priv(qmap_net);
		    memcpy(priv->bridge_self_mac, brmac_settings.brmac, ETH_ALEN);
		    pr_info("ioctl 0x89F4 change qmapnet bridge(%d) lan mac -> %02x:%02x:%02x:%02x:%02x:%02x\n", brmac_settings.id, priv->bridge_self_mac[0],
                priv->bridge_self_mac[1], priv->bridge_self_mac[2], priv->bridge_self_mac[3], priv->bridge_self_mac[4], priv->bridge_self_mac[5]);
		}
		else if (!pQmapDev->use_rmnet_usb && brmac_settings.id == 0) {
	        memcpy(pQmapDev->bridge_self_mac, brmac_settings.brmac, ETH_ALEN);
	        pr_info("ioctl 0x89F4 change usbnet bridge(%d) lan mac -> %02x:%02x:%02x:%02x:%02x:%02x\n", brmac_settings.id, pQmapDev->bridge_self_mac[0],
                pQmapDev->bridge_self_mac[1], pQmapDev->bridge_self_mac[2], pQmapDev->bridge_self_mac[3], pQmapDev->bridge_self_mac[4], pQmapDev->bridge_self_mac[5]);
		}
		else {
		    pr_info("ioctl 0x89F4 change bridge(%d) lan mac -> error id\n", brmac_settings.id);
		    rc = -1;
		}
	break;
#endif

	default:
	break;
	}

	return rc;
}

#ifdef QUECTEL_BRIDGE_MODE
static int is_qmap_netdev(const struct net_device *netdev) {
	return netdev->netdev_ops == &rmnet_vnd_ops;
}
#endif
#endif

static struct sk_buff *qmi_wwan_tx_fixup(struct usbnet *dev, struct sk_buff *skb, gfp_t flags) {
	//MDM9x07,MDM9628,MDM9x40,SDX20,SDX24 only work on RAW IP mode
	if ((dev->driver_info->flags & FLAG_NOARP) == 0)
		return skb;

	// Skip Ethernet header from message
	if (dev->net->hard_header_len == 0)
		return skb;
	else
		skb_reset_mac_header(skb);

#ifdef QUECTEL_BRIDGE_MODE
{
	struct qmi_wwan_state *info = (void *)&dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	if (pQmapDev->bridge_mode && bridge_mode_tx_fixup(dev->net, skb, pQmapDev->bridge_ipv4, pQmapDev->bridge_mac) == NULL) {
	      dev_kfree_skb_any (skb);
	      return NULL;
	}
}
#endif

	if (skb_pull(skb, ETH_HLEN)) {
		return skb;
	} else {
		dev_err(&dev->intf->dev,  "Packet Dropped ");
	}

	// Filter the packet out, release it
	dev_kfree_skb_any(skb);
	return NULL;
}
#endif

/* Make up an ethernet header if the packet doesn't have one.
 *
 * A firmware bug common among several devices cause them to send raw
 * IP packets under some circumstances.  There is no way for the
 * driver/host to know when this will happen.  And even when the bug
 * hits, some packets will still arrive with an intact header.
 *
 * The supported devices are only capably of sending IPv4, IPv6 and
 * ARP packets on a point-to-point link. Any packet with an ethernet
 * header will have either our address or a broadcast/multicast
 * address as destination.  ARP packets will always have a header.
 *
 * This means that this function will reliably add the appropriate
 * header iff necessary, provided our hardware address does not start
 * with 4 or 6.
 *
 * Another common firmware bug results in all packets being addressed
 * to 00:a0:c6:00:00:00 despite the host address being different.
 * This function will also fixup such packets.
 */
static int qmi_wwan_rx_fixup(struct usbnet *dev, struct sk_buff *skb)
{
	__be16 proto;

	/* This check is no longer done by usbnet */
	if (skb->len < dev->net->hard_header_len)
		return 0;

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
	if (skb_headroom(skb) < ETH_HLEN)
		return 0;
	skb_push(skb, ETH_HLEN);
	skb_reset_mac_header(skb);
	eth_hdr(skb)->h_proto = proto;
	memset(eth_hdr(skb)->h_source, 0, ETH_ALEN);
#if 1 //Added by Quectel
	//some kernel will drop ethernet packet which's souce mac is all zero
	memcpy(eth_hdr(skb)->h_source, default_modem_addr, ETH_ALEN);
#endif

fix_dest:
#ifdef QUECTEL_BRIDGE_MODE
{
	struct qmi_wwan_state *info = (void *)&dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	bridge_mode_rx_fixup(pQmapDev, dev->net, skb);
}
#else
	memcpy(eth_hdr(skb)->h_dest, dev->net->dev_addr, ETH_ALEN);
#endif

	return 1;
}

#if defined(QUECTEL_WWAN_QMAP)
static struct sk_buff *qmap_qmi_wwan_tx_fixup(struct usbnet *dev, struct sk_buff *skb, gfp_t flags) {
	struct qmi_wwan_state *info = (void *)&dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;

	if (unlikely(pQmapDev == NULL)) {
		goto drop_skb;
	} else if (unlikely(pQmapDev->qmap_mode && !pQmapDev->link_state)) {
		dev_dbg(&dev->net->dev, "link_state 0x%x, drop skb, len = %u\n", pQmapDev->link_state, skb->len);
		goto drop_skb;
	} else if (pQmapDev->qmap_mode == 0) {
		skb = qmi_wwan_tx_fixup(dev, skb, flags);
	}
	else if (pQmapDev->qmap_mode > 1) {
		WARN_ON(1); //never reach here.
	}
	else {
		if (likely(skb)) {
			skb = qmi_wwan_tx_fixup(dev, skb, flags);

			if (skb) {
				if(pQmapDev->qmap_version == 5)
					add_qhdr(skb, QUECTEL_QMAP_MUX_ID);
				else
					add_qhdr_v5(skb, QUECTEL_QMAP_MUX_ID);                
			}
			else {
				return NULL;
			}
		}
	}

	return skb;
drop_skb:
	dev_kfree_skb_any (skb);
	return NULL;
}

static void qmap_packet_decode(sQmiWwanQmap *pQmapDev,
	struct sk_buff *skb_in, struct sk_buff_head *skb_chain)
{
	struct device *dev = &pQmapDev->mpNetDev->net->dev;
	struct sk_buff *qmap_skb;
	uint dl_minimum_padding = 0;

	if (pQmapDev->qmap_version == 9)
		dl_minimum_padding = pQmapDev->tx_ctx.dl_minimum_padding;

	/* __skb_queue_head_init() do not call spin_lock_init(&list->lock),
	    so should not call skb_queue_tail/queue later.  */
	__skb_queue_head_init(skb_chain);

	while (skb_in->len > sizeof(struct qmap_hdr)) {
		struct rmnet_map_header *map_header = (struct rmnet_map_header *)skb_in->data;
		struct rmnet_map_v5_csum_header *ul_header = NULL;
		size_t hdr_size = sizeof(struct rmnet_map_header);	
		struct net_device *qmap_net;
		int pkt_len = ntohs(map_header->pkt_len);
		int skb_len;
		__be16 protocol;
		int mux_id;
		int skip_nss = 0;

		if (map_header->next_hdr) {
			ul_header = (struct rmnet_map_v5_csum_header *)(map_header + 1);
			hdr_size += sizeof(struct rmnet_map_v5_csum_header);
		}
		
		skb_len = pkt_len - (map_header->pad_len&0x3F);
		skb_len -= dl_minimum_padding;

		mux_id = map_header->mux_id - QUECTEL_QMAP_MUX_ID;
		if (mux_id >= pQmapDev->qmap_mode) {
			dev_info(dev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto error_pkt;
		}

		qmap_net = pQmapDev->mpQmapNetDev[mux_id];
		if (qmap_net == NULL) {
			dev_info(dev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto skip_pkt;
		}
    
		if (skb_len > qmap_net->mtu) {
			dev_info(dev, "drop skb_len=%x larger than mtu=%d\n", skb_len, qmap_net->mtu);
			goto error_pkt;
		}

		if (skb_in->len < (pkt_len + hdr_size)) {
			dev_info(dev, "drop qmap unknow pkt, len=%d, pkt_len=%d\n", skb_in->len, pkt_len);
			goto error_pkt;
		}

		if (map_header->cd_bit) {
			dev_info(dev, "skip qmap command packet\n");
			goto skip_pkt;
		}

		switch (skb_in->data[hdr_size] & 0xf0) {
			case 0x40:
#ifdef CONFIG_QCA_NSS_PACKET_FILTER
				{
					struct iphdr *ip4h = (struct iphdr *)(&skb_in->data[hdr_size]);
					if (ip4h->protocol == IPPROTO_ICMP) {
						skip_nss = 1;
					}
				}
#endif
				protocol = htons(ETH_P_IP);
			break;
			case 0x60:
#ifdef CONFIG_QCA_NSS_PACKET_FILTER
				{
					struct ipv6hdr *ip6h = (struct ipv6hdr *)(&skb_in->data[hdr_size]);
					if (ip6h->nexthdr == NEXTHDR_ICMP) {
						skip_nss = 1;
					}
				}
#endif
			protocol = htons(ETH_P_IPV6);
			break;
			default:
				dev_info(dev, "unknow skb->protocol %02x\n", skb_in->data[hdr_size]);
				goto error_pkt;
		}
		
		qmap_skb = netdev_alloc_skb(qmap_net, skb_len);
		if (qmap_skb) {
			skb_put(qmap_skb, skb_len);
			memcpy(qmap_skb->data, skb_in->data + hdr_size, skb_len);
		}

		if (qmap_skb == NULL) {
			dev_info(dev, "fail to alloc skb, pkt_len = %d\n", skb_len);
			goto error_pkt;
		}

		skb_reset_transport_header(qmap_skb);
		skb_reset_network_header(qmap_skb);
		qmap_skb->pkt_type = PACKET_HOST;
		skb_set_mac_header(qmap_skb, 0);
		qmap_skb->protocol = protocol;

		if(skip_nss)
			qmap_skb->cb[0] = 1;

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

		__skb_queue_tail(skb_chain, qmap_skb);

skip_pkt:
		skb_pull(skb_in, pkt_len + hdr_size);
	}

error_pkt:
	return;
}

static int qmap_qmi_wwan_rx_fixup(struct usbnet *dev, struct sk_buff *skb_in)
{
	struct qmi_wwan_state *info = (void *)&dev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	struct sk_buff *qmap_skb;
	struct sk_buff_head skb_chain;
		
	if (pQmapDev->qmap_mode == 0)
		return qmi_wwan_rx_fixup(dev, skb_in);

	qmap_packet_decode(pQmapDev, skb_in, &skb_chain);

	while ((qmap_skb = __skb_dequeue (&skb_chain))) {
		if (qmap_skb->dev != dev->net) {
			WARN_ON(1); //never reach here.
		}
		else {
			qmap_skb->protocol = 0;
			usbnet_skb_return(dev, qmap_skb);
		}
	}

    return 0;
}
#endif

/* very simplistic detection of IPv4 or IPv6 headers */
static bool possibly_iphdr(const char *data)
{
	return (data[0] & 0xd0) == 0x40;
}

/* disallow addresses which may be confused with IP headers */
static int qmi_wwan_mac_addr(struct net_device *dev, void *p)
{
	int ret;
	struct sockaddr *addr = p;

	ret = eth_prepare_mac_addr_change(dev, p);
	if (ret < 0)
		return ret;
	if (possibly_iphdr(addr->sa_data))
		return -EADDRNOTAVAIL;
	eth_commit_mac_addr_change(dev, p);
	return 0;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,10,0 )) //bc1f44709cf27fb2a5766cadafe7e2ad5e9cb221
static void (*_usbnet_get_stats64)(struct net_device *net, struct rtnl_link_stats64 *stats);

static void qmi_wwan_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats) {
	if (_usbnet_get_stats64) ////c8b5d129ee293bcf972e7279ac996bb8a138505c
		return _usbnet_get_stats64(net, stats);
	
	netdev_stats_to_stats64(stats, &net->stats);
}
#else
static struct rtnl_link_stats64 * (*_usbnet_get_stats64)(struct net_device *net, struct rtnl_link_stats64 *stats);

static struct rtnl_link_stats64 * qmi_wwan_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats) {
	if (_usbnet_get_stats64)
		return _usbnet_get_stats64(net, stats);
	
	netdev_stats_to_stats64(stats, &net->stats);
	return stats;
}
#endif

static int qmi_wwan_open (struct net_device *net) {
	struct usbnet * usbnetdev = netdev_priv( net );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	int retval;

	retval = usbnet_open(net);

	if (!retval) {
		if (pQmapDev && pQmapDev->qmap_mode == 1) {
			if (pQmapDev->link_state)
				netif_carrier_on(net);
		}
	}

	return retval;
}

static netdev_tx_t qmi_wwan_start_xmit (struct sk_buff *skb,
				     struct net_device *net) 
{
	struct usbnet * usbnetdev = netdev_priv( net );
	struct qmi_wwan_state *info = (void *)&usbnetdev->data;
	sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
	int retval;

	retval = usbnet_start_xmit(skb, net);

	if (netif_queue_stopped(net) && pQmapDev && pQmapDev->use_rmnet_usb) {
		int i;
		
		for (i = 0; i < pQmapDev->qmap_mode; i++) {
			struct net_device *qmap_net = pQmapDev->mpQmapNetDev[i];
			if (qmap_net) {
				netif_stop_queue(qmap_net);
			}
		}
	}

	return retval;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 5,14,0 )) //b9067f5dc4a07c8e24e01a1b277c6722d91be39e
#define use_ndo_siocdevprivate
#endif
#ifdef use_ndo_siocdevprivate
static int qmap_ndo_siocdevprivate(struct net_device *dev, struct ifreq *ifr, void __user *data, int cmd) {
	return qmap_ndo_do_ioctl(dev, ifr, cmd);
}
#endif

static const struct net_device_ops qmi_wwan_netdev_ops = {
	.ndo_open		= qmi_wwan_open,
	.ndo_stop		= usbnet_stop,
	.ndo_start_xmit		= qmi_wwan_start_xmit,
	.ndo_tx_timeout		= usbnet_tx_timeout,
	.ndo_change_mtu		= usbnet_change_mtu,
	.ndo_get_stats64	= qmi_wwan_get_stats64,
	.ndo_set_mac_address	= qmi_wwan_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
#if defined(QUECTEL_WWAN_QMAP)// && defined(CONFIG_ANDROID)
	.ndo_do_ioctl = qmap_ndo_do_ioctl,
#ifdef use_ndo_siocdevprivate
	.ndo_siocdevprivate = qmap_ndo_siocdevprivate,
#endif
#endif
};

static void ql_net_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *info)
{
	/* Inherit standard device info */
	usbnet_get_drvinfo(net, info);
	strlcpy(info->driver, driver_name, sizeof(info->driver));
	strlcpy(info->version, VERSION_NUMBER, sizeof(info->version));
}

static struct ethtool_ops ql_net_ethtool_ops;

/* using a counter to merge subdriver requests with our own into a
 * combined state
 */
static int qmi_wwan_manage_power(struct usbnet *dev, int on)
{
	struct qmi_wwan_state *info = (void *)&dev->data;
	int rv;

	dev_dbg(&dev->intf->dev, "%s() pmcount=%d, on=%d\n", __func__,
		atomic_read(&info->pmcount), on);

	if ((on && atomic_add_return(1, &info->pmcount) == 1) ||
	    (!on && atomic_dec_and_test(&info->pmcount))) {
		/* need autopm_get/put here to ensure the usbcore sees
		 * the new value
		 */
		rv = usb_autopm_get_interface(dev->intf);
		dev->intf->needs_remote_wakeup = on;
		if (!rv)
			usb_autopm_put_interface(dev->intf);
	}
	return 0;
}

static int qmi_wwan_cdc_wdm_manage_power(struct usb_interface *intf, int on)
{
	struct usbnet *dev = usb_get_intfdata(intf);

	/* can be called while disconnecting */
	if (!dev)
		return 0;
	return qmi_wwan_manage_power(dev, on);
}

/* collect all three endpoints and register subdriver */
static int qmi_wwan_register_subdriver(struct usbnet *dev)
{
	int rv;
	struct usb_driver *subdriver = NULL;
	struct qmi_wwan_state *info = (void *)&dev->data;

	/* collect bulk endpoints */
	rv = usbnet_get_endpoints(dev, info->data);
	if (rv < 0)
		goto err;

	/* update status endpoint if separate control interface */
	if (info->control != info->data)
		dev->status = &info->control->cur_altsetting->endpoint[0];

	/* require interrupt endpoint for subdriver */
	if (!dev->status) {
		rv = -EINVAL;
		goto err;
	}

	/* for subdriver power management */
	atomic_set(&info->pmcount, 0);

	/* register subdriver */
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 5,12,0 )) //cac6fb015f719104e60b1c68c15ca5b734f57b9c
	subdriver = usb_cdc_wdm_register(info->control, &dev->status->desc,
					 4096, WWAN_PORT_QMI, &qmi_wwan_cdc_wdm_manage_power);
#else
	subdriver = usb_cdc_wdm_register(info->control, &dev->status->desc,
					 4096, &qmi_wwan_cdc_wdm_manage_power);

#endif
	if (IS_ERR(subdriver)) {
		dev_err(&info->control->dev, "subdriver registration failed\n");
		rv = PTR_ERR(subdriver);
		goto err;
	}

	/* prevent usbnet from using status endpoint */
	dev->status = NULL;

	/* save subdriver struct for suspend/resume wrappers */
	info->subdriver = subdriver;

err:
	return rv;
}

static int qmi_wwan_bind(struct usbnet *dev, struct usb_interface *intf)
{
	int status = -1;
	struct usb_driver *driver = driver_of(intf);
	struct qmi_wwan_state *info = (void *)&dev->data;

	BUILD_BUG_ON((sizeof(((struct usbnet *)0)->data) <
		      sizeof(struct qmi_wwan_state)));

	/* set up initial state */
	info->control = intf;
	info->data = intf;

	status = qmi_wwan_register_subdriver(dev);
	if (status < 0 && info->control != info->data) {
		usb_set_intfdata(info->data, NULL);
		usb_driver_release_interface(driver, info->data);
	}

	/* Never use the same address on both ends of the link, even
	 * if the buggy firmware told us to.
	 */
	if (ether_addr_equal(dev->net->dev_addr, default_modem_addr))
		eth_hw_addr_random(dev->net);

	/* make MAC addr easily distinguishable from an IP header */
	if (possibly_iphdr(dev->net->dev_addr)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
		dev->net->dev_addr[0] |= 0x02;	/* set local assignment bit */
		dev->net->dev_addr[0] &= 0xbf;	/* clear "IP" bit */
#else
		u8 addr = dev->net->dev_addr[0];

		addr |= 0x02;	/* set local assignment bit */
		addr &= 0xbf;	/* clear "IP" bit */
		dev_addr_mod(dev->net, 0, &addr, 1);
#endif
	}
	if (!_usbnet_get_stats64)
		_usbnet_get_stats64 = dev->net->netdev_ops->ndo_get_stats64;
	dev->net->netdev_ops = &qmi_wwan_netdev_ops;

	ql_net_ethtool_ops = *dev->net->ethtool_ops;
	ql_net_ethtool_ops.get_drvinfo = ql_net_get_drvinfo;
	dev->net->ethtool_ops = &ql_net_ethtool_ops;

#if 1 //Added by Quectel
	if (dev->driver_info->flags & FLAG_NOARP) {
		int ret;
		char buf[32] = "Module";

		ret = usb_string(dev->udev, dev->udev->descriptor.iProduct, buf, sizeof(buf));
		if (ret > 0) {
			buf[ret] = '\0';
		}

		dev_info(&intf->dev, "Quectel %s work on RawIP mode\n", buf);
		dev->net->flags |= IFF_NOARP;
		dev->net->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

		usb_control_msg(
			interface_to_usbdev(intf),
			usb_sndctrlpipe(interface_to_usbdev(intf), 0),
			0x22, //USB_CDC_REQ_SET_CONTROL_LINE_STATE
			0x21, //USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE
			1, //active CDC DTR
			intf->cur_altsetting->desc.bInterfaceNumber,
			NULL, 0, 100);
	}

	//to advoid module report mtu 1460, but rx 1500 bytes IP packets, and cause the customer's system crash
	//next setting can make usbnet.c:usbnet_change_mtu() do not modify rx_urb_size according to hard mtu
	dev->rx_urb_size = ETH_DATA_LEN + ETH_HLEN + 6;

#if defined(QUECTEL_WWAN_QMAP)
	if (qmap_mode > QUECTEL_WWAN_QMAP)
		qmap_mode = QUECTEL_WWAN_QMAP;

	if (!status)
	{
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)kzalloc(sizeof(sQmiWwanQmap), GFP_KERNEL);

		if (pQmapDev == NULL)
			return -ENODEV;

#ifdef QUECTEL_BRIDGE_MODE
		pQmapDev->bridge_mode = bridge_mode;
#ifdef CONFIG_BRIDGE_LAN
        memcpy(pQmapDev->bridge_self_mac, broadcast_mac_addr, ETH_ALEN);
#endif
#endif
		pQmapDev->mpNetDev = dev;
		pQmapDev->link_state = 1;
		//on OpenWrt, if set rmnet_usb0.1 as WAN, '/sbin/netifd' will auto create VLAN for rmnet_usb0
		dev->net->features |= (NETIF_F_VLAN_CHALLENGED);

		if (dev->driver_info->flags & FLAG_NOARP)
		{
			int qmap_version = (dev->driver_info->data>>8)&0xFF;
			int qmap_size = (dev->driver_info->data)&0xFF;
			int idProduct = le16_to_cpu(dev->udev->descriptor.idProduct);
			int lte_a = (idProduct == 0x0306 || idProduct == 0x030B || idProduct == 0x0512 || idProduct == 0x0620 || idProduct == 0x0800 || idProduct == 0x0801);

			if (qmap_size > 4096 || dev->udev->speed >= USB_SPEED_SUPER) { //if meet this requirements, must be LTE-A or 5G
				lte_a = 1;
			}
			
			pQmapDev->qmap_mode = qmap_mode;
			if (lte_a && pQmapDev->qmap_mode == 0) {
				pQmapDev->qmap_mode = 1; //force use QMAP
				if(qmap_mode == 0)
					qmap_mode = 1; //old quectel-CM only check sys/module/wwan0/parameters/qmap_mode
			}

			if (pQmapDev->qmap_mode) {
				pQmapDev->qmap_version = qmap_version;
				pQmapDev->qmap_size = qmap_size*1024;
				dev->rx_urb_size = pQmapDev->qmap_size;
				//for these modules, if send packet before qmi_start_network, or cause host PC crash, or cause modules crash
   				pQmapDev->link_state = !lte_a;

				if (pQmapDev->qmap_mode > 1)
					pQmapDev->use_rmnet_usb = 1;
				else if (idProduct == 0x0800 || idProduct == 0x0801)
					pQmapDev->use_rmnet_usb = 1; //benefit for ul data agg
#ifdef QMI_NETDEV_ONE_CARD_MODE
				if(pQmapDev->use_rmnet_usb == 1 && pQmapDev->qmap_mode == 1)
					one_card_mode = 1;
				pQmapDev->rmnet_info.mux_id[0] = QUECTEL_QMAP_MUX_ID;
#endif	
				pQmapDev->rmnet_info.size = sizeof(RMNET_INFO);
				pQmapDev->rmnet_info.rx_urb_size = pQmapDev->qmap_size;
				pQmapDev->rmnet_info.ep_type = 2; //DATA_EP_TYPE_HSUSB
				pQmapDev->rmnet_info.iface_id = 4;
				pQmapDev->rmnet_info.qmap_mode = pQmapDev->qmap_mode;
				pQmapDev->rmnet_info.qmap_version = pQmapDev->qmap_version;
				pQmapDev->rmnet_info.dl_minimum_padding = 0;

#if defined(QUECTEL_UL_DATA_AGG)
				pQmapDev->tx_ctx.ul_data_aggregation_max_datagrams = 1;
				pQmapDev->tx_ctx.ul_data_aggregation_max_size = 1500;
#endif

				if (pQmapDev->use_rmnet_usb && !one_card_mode) {					
					pQmapDev->driver_info = rmnet_usb_info;
					pQmapDev->driver_info.data = dev->driver_info->data;
					dev->driver_info = &pQmapDev->driver_info;
				}

				if (pQmapDev->use_rmnet_usb && !one_card_mode) {
					pQmapDev->usbnet_bh = dev->bh;
					tasklet_init(&dev->bh, usbnet_bh, (unsigned long)pQmapDev);
				}
			}
		}

		info->unused = (unsigned long)pQmapDev;
		dev->net->sysfs_groups[0] = &qmi_wwan_sysfs_attr_group;

		dev_info(&intf->dev, "rx_urb_size = %zd\n", dev->rx_urb_size);
	}
#endif
#endif

	return status;
}

static void qmi_wwan_unbind(struct usbnet *dev, struct usb_interface *intf)
{
	struct qmi_wwan_state *info = (void *)&dev->data;
	struct usb_driver *driver = driver_of(intf);
	struct usb_interface *other;

	if (dev->udev && dev->udev->state == USB_STATE_CONFIGURED) {
		usb_control_msg(
			interface_to_usbdev(intf),
			usb_sndctrlpipe(interface_to_usbdev(intf), 0),
			0x22, //USB_CDC_REQ_SET_CONTROL_LINE_STATE
			0x21, //USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE
			0, //deactive CDC DTR
			intf->cur_altsetting->desc.bInterfaceNumber,
			NULL, 0, 100);
	}

	if (info->subdriver && info->subdriver->disconnect)
		info->subdriver->disconnect(info->control);

	/* allow user to unbind using either control or data */
	if (intf == info->control)
		other = info->data;
	else
		other = info->control;

	/* only if not shared */
	if (other && intf != other) {
		usb_set_intfdata(other, NULL);
		usb_driver_release_interface(driver, other);
	}

	info->subdriver = NULL;
	info->data = NULL;
	info->control = NULL;
}

/* suspend/resume wrappers calling both usbnet and the cdc-wdm
 * subdriver if present.
 *
 * NOTE: cdc-wdm also supports pre/post_reset, but we cannot provide
 * wrappers for those without adding usbnet reset support first.
 */
static int qmi_wwan_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usbnet *dev = usb_get_intfdata(intf);
	struct qmi_wwan_state *info = (void *)&dev->data;
	int ret;

	/* Both usbnet_suspend() and subdriver->suspend() MUST return 0
	 * in system sleep context, otherwise, the resume callback has
	 * to recover device from previous suspend failure.
	 */
	ret = usbnet_suspend(intf, message);
	if (ret < 0)
		goto err;

	if (intf == info->control && info->subdriver &&
	    info->subdriver->suspend)
		ret = info->subdriver->suspend(intf, message);
	if (ret < 0)
		usbnet_resume(intf);
err:
	return ret;
}

static int qmi_wwan_resume(struct usb_interface *intf)
{
	struct usbnet *dev = usb_get_intfdata(intf);
	struct qmi_wwan_state *info = (void *)&dev->data;
	int ret = 0;
	bool callsub = (intf == info->control && info->subdriver &&
			info->subdriver->resume);

	if (callsub)
		ret = info->subdriver->resume(intf);
	if (ret < 0)
		goto err;
	ret = usbnet_resume(intf);
	if (ret < 0 && callsub)
		info->subdriver->suspend(intf, PMSG_SUSPEND);

#if defined(QUECTEL_WWAN_QMAP)
	if (!netif_queue_stopped(dev->net)) {
		qmap_wake_queue((sQmiWwanQmap *)info->unused);
	}
#endif

err:
	return ret;
}

static int qmi_wwan_reset_resume(struct usb_interface *intf)
{
	dev_info(&intf->dev, "device do not support reset_resume\n");
	intf->needs_binding = 1;
	return -EOPNOTSUPP;
}

static struct sk_buff *rmnet_usb_tx_fixup(struct usbnet *dev, struct sk_buff *skb, gfp_t flags)
{
	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);
	if (skb->protocol != htons(ETH_P_MAP)) {
		dev_kfree_skb_any(skb);
		return NULL;
	}

	return skb;
}

static int rmnet_usb_rx_fixup(struct usbnet *dev, struct sk_buff *skb)
{
	struct net_device	*net = dev->net;
	unsigned headroom = skb_headroom(skb);

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 3,3,1 )) //7bdd402706cf26bfef9050dfee3f229b7f33ee4f
//some customers port to v3.2
	if (net->type == ARPHRD_ETHER && headroom < ETH_HLEN) {
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

	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);
	if (net->type == ARPHRD_ETHER && headroom >= ETH_HLEN) {
		//usbnet.c rx_process() usbnet_skb_return() eth_type_trans()
		skb_push(skb, ETH_HLEN);
		skb_reset_mac_header(skb);
		memcpy(eth_hdr(skb)->h_source, default_modem_addr, ETH_ALEN);
		memcpy(eth_hdr(skb)->h_dest, net->dev_addr, ETH_ALEN);
		eth_hdr(skb)->h_proto = htons(ETH_P_MAP);

		return 1;
	}
	
	return 0;
}

static rx_handler_result_t rmnet_usb_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct usbnet *dev;
	struct qmi_wwan_state *info;
	sQmiWwanQmap *pQmapDev;
	struct sk_buff *qmap_skb;
	struct sk_buff_head skb_chain;

	if (!skb)
		goto done;

	//printk("%s skb=%p, protocol=%x, len=%d\n", __func__, skb, skb->protocol, skb->len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	if (skb->protocol != htons(ETH_P_MAP)) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}
    /* when open hyfi function, run cm will make system crash */
	//dev = rcu_dereference(skb->dev->rx_handler_data);
	dev = netdev_priv(skb->dev);

	if (dev == NULL) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}

	info = (struct qmi_wwan_state *)&dev->data;
	pQmapDev = (sQmiWwanQmap *)info->unused;

	qmap_packet_decode(pQmapDev, skb, &skb_chain);
	while ((qmap_skb = __skb_dequeue (&skb_chain))) {
		struct net_device	*qmap_net = qmap_skb->dev;

		rmnet_vnd_update_rx_stats(qmap_net, 1, qmap_skb->len);
		if (qmap_net->type == ARPHRD_ETHER)
			__skb_pull(qmap_skb, ETH_HLEN);
		netif_receive_skb(qmap_skb);
	}
	consume_skb(skb);

done:
	return RX_HANDLER_CONSUMED;
}

static const struct driver_info	qmi_wwan_info = {
	.description	= "WWAN/QMI device",
	.flags		= FLAG_WWAN,
	.bind		= qmi_wwan_bind,
	.unbind		= qmi_wwan_unbind,
	.manage_power	= qmi_wwan_manage_power,
};

#define qmi_wwan_raw_ip_info \
	.description	= "WWAN/QMI device", \
	.flags		= FLAG_WWAN | FLAG_RX_ASSEMBLE | FLAG_NOARP | FLAG_SEND_ZLP, \
	.bind		= qmi_wwan_bind, \
	.unbind		= qmi_wwan_unbind, \
	.manage_power	= qmi_wwan_manage_power, \
	.tx_fixup       = qmap_qmi_wwan_tx_fixup, \
	.rx_fixup       = qmap_qmi_wwan_rx_fixup, \

static const struct driver_info rmnet_usb_info = {
	.description = "RMNET/USB device",
	.flags		=  FLAG_WWAN | FLAG_NOARP | FLAG_SEND_ZLP,
	.bind = qmi_wwan_bind,
	.unbind = qmi_wwan_unbind,
	.manage_power = qmi_wwan_manage_power,
	.tx_fixup = rmnet_usb_tx_fixup,
	.rx_fixup = rmnet_usb_rx_fixup,
};

static const struct driver_info qmi_wwan_raw_ip_info_mdm9x07 = {
	qmi_wwan_raw_ip_info
	.data = (5<<8)|4, //QMAPV1 and 4KB
};

// mdm9x40/sdx12/sdx20/sdx24 share the same config
static const struct driver_info qmi_wwan_raw_ip_info_mdm9x40 = {
	qmi_wwan_raw_ip_info
	.data =  (5<<8)|16, //QMAPV1 and 16KB
};

static const struct driver_info qmi_wwan_raw_ip_info_sdx55 = {
	qmi_wwan_raw_ip_info
	.data = (9<<8)|31, //QMAPV5 and 31KB
};

/* map QMI/wwan function by a fixed interface number */
#define QMI_FIXED_INTF(vend, prod, num) \
	USB_DEVICE_INTERFACE_NUMBER(vend, prod, num), \
	.driver_info = (unsigned long)&qmi_wwan_info

#define QMI_FIXED_RAWIP_INTF(vend, prod, num, chip) \
	USB_DEVICE_INTERFACE_NUMBER(vend, prod, num), \
	.driver_info = (unsigned long)&qmi_wwan_raw_ip_info_##chip

static const struct usb_device_id products[] = {
	{ QMI_FIXED_INTF(0x05C6, 0x9003, 4) },  /* Quectel UC20 */
	{ QMI_FIXED_INTF(0x05C6, 0x9215, 4) },  /* Quectel EC20 (MDM9215) */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0125, 4, mdm9x07) },  /* Quectel EC20 (MDM9X07)/EC25/EG25 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0121, 4, mdm9x07) },  /* Quectel EC21 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0191, 4, mdm9x07) },  /* Quectel EG91 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0195, 4, mdm9x07) },  /* Quectel EG95 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0700, 3, mdm9x07) },  /* Quectel BG95 (at+qcfgext="usbnet","rmnet") */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0306, 4, mdm9x40) },  /* Quectel EG06/EP06/EM06 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x030B, 4, mdm9x40) },  /* Quectel EG065k/EG060K */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0512, 4, mdm9x40) },  /* Quectel EG12/EP12/EM12/EG16/EG18 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0296, 4, mdm9x07) },  /* Quectel BG96 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0435, 4, mdm9x07) },  /* Quectel AG35 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0620, 4, mdm9x40) },  /* Quectel EG20 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0800, 4, sdx55) },  /* Quectel RG500 */
	{ QMI_FIXED_RAWIP_INTF(0x2C7C, 0x0801, 4, sdx55) },  /* Quectel RG520 */
	{ }					/* END */
};
MODULE_DEVICE_TABLE(usb, products);

static int qmi_wwan_probe(struct usb_interface *intf,
			  const struct usb_device_id *prod)
{
	struct usb_device_id *id = (struct usb_device_id *)prod;

	/* Workaround to enable dynamic IDs.  This disables usbnet
	 * blacklisting functionality.  Which, if required, can be
	 * reimplemented here by using a magic "blacklist" value
	 * instead of 0 in the static device id table
	 */
	if (!id->driver_info) {
		dev_dbg(&intf->dev, "setting defaults for dynamic device id\n");
		id->driver_info = (unsigned long)&qmi_wwan_info;
	}

	if (intf->cur_altsetting->desc.bInterfaceClass != 0xff) {
		dev_info(&intf->dev,  "Quectel module not qmi_wwan mode! please check 'at+qcfg=\"usbnet\"'\n");
		return -ENODEV;
	}

	return usbnet_probe(intf, id);
}

#if defined(QUECTEL_WWAN_QMAP)
static int qmap_qmi_wwan_probe(struct usb_interface *intf,
			  const struct usb_device_id *prod)
{
	int status = qmi_wwan_probe(intf, prod);

	if (!status) {
		struct usbnet *dev = usb_get_intfdata(intf);
		struct qmi_wwan_state *info = (void *)&dev->data;
		sQmiWwanQmap *pQmapDev = (sQmiWwanQmap *)info->unused;
		unsigned i;

		if (!pQmapDev)
			return status;

		tasklet_init(&pQmapDev->txq, rmnet_usb_tx_wake_queue, (unsigned long)pQmapDev);

		if (pQmapDev->qmap_mode == 1) {
			pQmapDev->mpQmapNetDev[0] = dev->net;
			if (pQmapDev->use_rmnet_usb && !one_card_mode) {
				pQmapDev->mpQmapNetDev[0] = NULL;
				qmap_register_device(pQmapDev, 0);
			}
		}
		else if (pQmapDev->qmap_mode > 1) {
			for (i = 0; i < pQmapDev->qmap_mode; i++) {
				qmap_register_device(pQmapDev, i);
			}
		}

		if (pQmapDev->use_rmnet_usb && !one_card_mode) {
			rtnl_lock();
			/* when open hyfi function, run cm will make system crash */
			//netdev_rx_handler_register(dev->net, rmnet_usb_rx_handler, dev);
			netdev_rx_handler_register(dev->net, rmnet_usb_rx_handler, NULL);
			rtnl_unlock();
		}

		if (pQmapDev->link_state == 0) {
			netif_carrier_off(dev->net);
		}
	}

	return status;
}

static void qmap_qmi_wwan_disconnect(struct usb_interface *intf)
{
	struct usbnet *dev = usb_get_intfdata(intf);
	struct qmi_wwan_state *info;
	sQmiWwanQmap *pQmapDev;
	uint i;

	if (!dev)
		return;

	info = (void *)&dev->data;
	pQmapDev = (sQmiWwanQmap *)info->unused;

	if (!pQmapDev) {
		return usbnet_disconnect(intf);
	}

	pQmapDev->link_state = 0;

	if (pQmapDev->qmap_mode > 1) {
		for (i = 0; i < pQmapDev->qmap_mode; i++) {
			qmap_unregister_device(pQmapDev, i);
		}
	}

	if (pQmapDev->use_rmnet_usb && !one_card_mode) {
		qmap_unregister_device(pQmapDev, 0);
		rtnl_lock();
		netdev_rx_handler_unregister(dev->net);
		rtnl_unlock();
	}

	tasklet_kill(&pQmapDev->txq);
	
	usbnet_disconnect(intf);
	/* struct usbnet *dev had free by usbnet_disconnect()->free_netdev().
	    so we should access info. */
	//info->unused = 0;	
	kfree(pQmapDev);
}
#endif

static struct usb_driver qmi_wwan_driver = {
	.name		      = "qmi_wwan_q",
	.id_table	      = products,
	.probe		      = qmi_wwan_probe,
#if defined(QUECTEL_WWAN_QMAP)
	.probe		      = qmap_qmi_wwan_probe,
	.disconnect	      = qmap_qmi_wwan_disconnect,
#else
	.probe		      = qmi_wwan_probe,
	.disconnect	      = usbnet_disconnect,
#endif
	.suspend	      = qmi_wwan_suspend,
	.resume		      =	qmi_wwan_resume,
	.reset_resume         = qmi_wwan_reset_resume,
	.supports_autosuspend = 1,
	.disable_hub_initiated_lpm = 1,
};

static int __init qmi_wwan_driver_init(void)
{
#ifdef CONFIG_QCA_NSS_DRV
	nss_cb = rcu_dereference(rmnet_nss_callbacks);
	if (!nss_cb) {
		printk(KERN_ERR "qmi_wwan_driver_init: driver load must after '/etc/modules.d/42-rmnet-nss'\n");
	}
#endif
	return usb_register(&qmi_wwan_driver);
}
module_init(qmi_wwan_driver_init);
static void __exit qmi_wwan_driver_exit(void)
{
	usb_deregister(&qmi_wwan_driver);
}
module_exit(qmi_wwan_driver_exit);

MODULE_AUTHOR("Bjørn Mork <bjorn@mork.no>");
MODULE_DESCRIPTION("Qualcomm MSM Interface (QMI) WWAN driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(QUECTEL_WWAN_VERSION);
