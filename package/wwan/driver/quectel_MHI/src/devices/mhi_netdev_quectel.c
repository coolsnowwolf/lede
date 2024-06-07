/* Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/dma-mapping.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/rtnetlink.h>
#include <linux/time.h>
#include <net/arp.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include <linux/usb/cdc.h>

//#define CONFIG_IPQ5018_RATE_CONTROL //Only used with spf11.5 for IPQ5018
#if defined(CONFIG_IPQ5018_RATE_CONTROL)
//#include <linux/jiffies.h>
#include <asm/arch_timer.h>
#endif

#include "../core/mhi.h"
//#define MHI_NETDEV_ONE_CARD_MODE
//#define ANDROID_gki //some fuction not allow used in this TEST

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

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 4,2,0 ))
static bool netdev_is_rx_handler_busy(struct net_device *dev)
{
	ASSERT_RTNL();
	return dev && rtnl_dereference(dev->rx_handler);
}
#endif

struct rmnet_nss_cb {
	int (*nss_create)(struct net_device *dev);
	int (*nss_free)(struct net_device *dev);
	int (*nss_tx)(struct sk_buff *skb);
};
static struct rmnet_nss_cb __read_mostly *nss_cb = NULL;
#if defined(CONFIG_PINCTRL_IPQ807x) || defined(CONFIG_PINCTRL_IPQ5018) || defined(CONFIG_PINCTRL_IPQ8074)
//#ifdef CONFIG_RMNET_DATA //spf12.x have no macro defined, just for spf11.x
#define CONFIG_QCA_NSS_DRV
/* define at qsdk/qca/src/linux-4.4/net/rmnet_data/rmnet_data_main.c */ //for spf11.x
/* define at qsdk/qca/src/datarmnet/core/rmnet_config.c */ //for spf12.x
/* set at qsdk/qca/src/data-kernel/drivers/rmnet-nss/rmnet_nss.c */
/* need add DEPENDS:= kmod-rmnet-core in feeds/makefile */
extern struct rmnet_nss_cb *rmnet_nss_callbacks __rcu __read_mostly;
//#endif
#endif

static const unsigned char node_id[ETH_ALEN] = {0x02, 0x50, 0xf4, 0x00, 0x00, 0x00};
static const unsigned char default_modem_addr[ETH_ALEN] = {0x02, 0x50, 0xf3, 0x00, 0x00, 0x00};

#if defined(CONFIG_BRIDGE) || defined(CONFIG_BRIDGE_MODULE)
#define QUECTEL_BRIDGE_MODE
#endif

#define QUECTEL_RMNET_MODE

#ifdef QUECTEL_BRIDGE_MODE
static uint __read_mostly bridge_mode = 0/*|BIT(1)*/;
module_param( bridge_mode, uint, S_IRUGO );
#endif

struct qmap_hdr {
    u8 cd_rsvd_pad;
    u8 mux_id;
    u16 pkt_len;
} __packed;
#define QUECTEL_QMAP_MUX_ID 0x81

enum rmnet_map_v5_header_type {
	RMNET_MAP_HEADER_TYPE_UNKNOWN,
	RMNET_MAP_HEADER_TYPE_COALESCING = 0x1,
	RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD = 0x2,
	RMNET_MAP_HEADER_TYPE_ENUM_LENGTH
};

enum rmnet_map_commands {
	RMNET_MAP_COMMAND_NONE,
	RMNET_MAP_COMMAND_FLOW_DISABLE,
	RMNET_MAP_COMMAND_FLOW_ENABLE,
	RMNET_MAP_COMMAND_FLOW_START = 7,
	RMNET_MAP_COMMAND_FLOW_END = 8,
	/* These should always be the last 2 elements */
	RMNET_MAP_COMMAND_UNKNOWN,
	RMNET_MAP_COMMAND_ENUM_LENGTH
};

#define RMNET_MAP_COMMAND_REQUEST     0
#define RMNET_MAP_COMMAND_ACK         1
#define RMNET_MAP_COMMAND_UNSUPPORTED 2
#define RMNET_MAP_COMMAND_INVALID     3

/* Main QMAP header */
struct rmnet_map_header {
	u8  pad_len:6;
	u8  next_hdr:1;
	u8  cd_bit:1;
	u8  mux_id;
	__be16 pkt_len;
}  __aligned(1);

/* QMAP v5 headers */
struct rmnet_map_v5_csum_header {
	u8  next_hdr:1;
	u8  header_type:7;
	u8  hw_reserved:7;
	u8  csum_valid_required:1;
	__be16 reserved;
} __aligned(1);

struct rmnet_map_control_command {
	u8  command_name;
	u8  cmd_type:2;
	u8  reserved:6;
	u16 reserved2;
	u32 transaction_id;
	union {
		struct {
			u8 reserved2;
			u8 ip_family:2;
			u8 reserved:6;
			__be16 flow_control_seq_num;
			__be32 qos_id;
		} flow_control;
		u8 data[0];
	};
}  __aligned(1);

struct mhi_mbim_hdr {
	struct usb_cdc_ncm_nth16 nth16;
	struct usb_cdc_ncm_ndp16 ndp16;
	struct usb_cdc_ncm_dpe16 dpe16[2];
} __attribute__ ((packed));

#define QCUSB_MRECEIVE_MAX_BUFFER_SIZE (1024*32) //maybe 31KB is enough
#define QCUSB_MTRANSMIT_MAX_BUFFER_SIZE (1024*16)
#define NTB_OUT_MAX_DATAGRAMS        16

static const struct usb_cdc_ncm_ntb_parameters ncmNTBParams = {
	.bmNtbFormatsSupported = USB_CDC_NCM_NTB16_SUPPORTED,
	.dwNtbInMaxSize = QCUSB_MRECEIVE_MAX_BUFFER_SIZE,
	.wNdpInDivisor = 0x04,
	.wNdpInPayloadRemainder = 0x0,
	.wNdpInAlignment = 0x4,

	.dwNtbOutMaxSize = QCUSB_MTRANSMIT_MAX_BUFFER_SIZE,
	.wNdpOutDivisor = 0x04,
	.wNdpOutPayloadRemainder = 0x0,
	.wNdpOutAlignment = 0x4,
	.wNtbOutMaxDatagrams = NTB_OUT_MAX_DATAGRAMS,
};

#if 0
static void qmap_hex_dump(const char *tag, unsigned char *data, unsigned len) {
	uint i;
	uint *d = (uint *)data;

	printk(KERN_DEBUG "%s data=%p, len=%x\n", tag, data, len);
	len = (len+3)/4;
	for (i = 0; i < len; i+=4) {
		printk(KERN_DEBUG "%08x %08x %08x %08x %08x\n", i*4, d[i+0], d[i+1], d[i+2], d[i+3]);
	}
}
#else
static void qmap_hex_dump(const char *tag, unsigned char *data, unsigned len) {
}
#endif

static uint __read_mostly mhi_mbim_enabled = 0;
module_param(mhi_mbim_enabled, uint, S_IRUGO);
int mhi_netdev_mbin_enabled(void) { return mhi_mbim_enabled; }

static uint __read_mostly qmap_mode = 1;
module_param(qmap_mode, uint, S_IRUGO);

static uint __read_mostly poll_weight = NAPI_POLL_WEIGHT;
module_param(poll_weight, uint, S_IRUGO);

#define MHI_NETDEV_DRIVER_NAME "mhi_netdev"
#define WATCHDOG_TIMEOUT (30 * HZ)

#define MSG_VERB(fmt, ...) do { \
	if (mhi_netdev->msg_lvl <= MHI_MSG_LVL_VERBOSE) \
		pr_err("[D][%s] " fmt, __func__, ##__VA_ARGS__);\
} while (0)

#define MHI_ASSERT(cond, msg) do { \
	if (cond) { \
		MSG_ERR(msg); \
		WARN_ON(cond); \
	} \
} while (0)

#define MSG_LOG(fmt, ...) do { \
	if (mhi_netdev->msg_lvl <= MHI_MSG_LVL_INFO) \
		pr_err("[I][%s] " fmt, __func__, ##__VA_ARGS__);\
} while (0)

#define MSG_ERR(fmt, ...) do { \
	if (mhi_netdev->msg_lvl <= MHI_MSG_LVL_ERROR) \
		pr_err("[E][%s] " fmt, __func__, ##__VA_ARGS__); \
} while (0)

struct mhi_stats {
	u32 rx_int;
	u32 tx_full;
	u32 tx_pkts;
	u32 rx_budget_overflow;
	u32 tx_allocated;
	u32 rx_allocated;
	u32 alloc_failed;
};

/* important: do not exceed sk_buf->cb (48 bytes) */
struct mhi_skb_priv {
	void *buf;
	size_t size;
	struct mhi_netdev *bind_netdev;
};

struct skb_data {	/* skb->cb is one of these */
	struct mhi_netdev *bind_netdev;
	unsigned int length;
	unsigned int packets;
};

#define MHI_NETDEV_STATUS64 1

typedef struct {
    uint size;
    uint rx_urb_size;
    uint ep_type;
    uint iface_id;
    uint MuxId;
    uint ul_data_aggregation_max_datagrams; //0x17
    uint ul_data_aggregation_max_size ;//0x18
    uint dl_minimum_padding; //0x1A
} QMAP_SETTING;

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

typedef struct {
	u16 tx_seq;
	u16 rx_seq;
	u32 rx_max;
} MHI_MBIM_CTX;

enum mhi_net_type {
	MHI_NET_UNKNOW,
	MHI_NET_RMNET,
	MHI_NET_MBIM,
	MHI_NET_ETHER
};

//#define TS_DEBUG
struct mhi_netdev {
	int alias;
	struct mhi_device *mhi_dev;
	spinlock_t rx_lock;
	bool enabled;
	rwlock_t pm_lock; /* state change lock */
	int (*rx_queue)(struct mhi_netdev *mhi_netdev, gfp_t gfp_t);
	struct delayed_work alloc_work;
	int wake;

	struct sk_buff_head tx_allocated;
	struct sk_buff_head rx_allocated;
	struct sk_buff_head qmap_chain;
	struct sk_buff_head skb_chain;
#ifdef TS_DEBUG
	uint clear_ts;
	struct timespec diff_ts;
	struct timespec qmap_ts;
	struct timespec skb_ts;
#endif

	MHI_MBIM_CTX mbim_ctx;

	u32 mru;
	u32 max_mtu;
	const char *interface_name;
	struct napi_struct napi;
	struct net_device *ndev;
	enum mhi_net_type net_type;
	struct sk_buff *frag_skb;
	bool recycle_buf;

#if defined(MHI_NETDEV_STATUS64)
	struct pcpu_sw_netstats __percpu *stats64;
#endif
	struct mhi_stats stats;

	struct dentry *dentry;
	enum MHI_DEBUG_LEVEL msg_lvl;

	struct net_device *mpQmapNetDev[8];
	u32 qmap_mode;
	u32 qmap_version; // 5 ~ QMAP V1, 9 ~ QMAP V5
	u32 qmap_size;
	u32 link_state;
	u32 flow_control;
	u32 dl_minimum_padding;

#ifdef QUECTEL_BRIDGE_MODE
	uint bridge_mode;
	uint bridge_ipv4;
	unsigned char bridge_mac[ETH_ALEN];
#endif
	uint use_rmnet_usb;
	RMNET_INFO rmnet_info;

#if defined(CONFIG_IPQ5018_RATE_CONTROL)
	u64 first_jiffy;
	u64 bytes_received_1;
	u64 bytes_received_2;
	u32 cntfrq_per_msec;
	bool mhi_rate_control;
#endif

	u32 rmnet_map_command_stats[RMNET_MAP_COMMAND_ENUM_LENGTH];
};

struct mhi_netdev_priv {
	struct mhi_netdev *mhi_netdev;
};

struct qmap_priv {
	void *pQmapDev;
	struct net_device *real_dev;
	struct net_device *self_dev;
	u8 offset_id;
	u8 mux_id;
	u8 qmap_version; // 5~v1, 9~v5

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
#endif
	uint use_qca_nss;	
};

static struct mhi_netdev *ndev_to_mhi(struct net_device *ndev) {
	struct mhi_netdev_priv *mhi_netdev_priv = netdev_priv(ndev);
	struct mhi_netdev *mhi_netdev = mhi_netdev_priv->mhi_netdev;
	return mhi_netdev;
}

static struct mhi_driver mhi_netdev_driver;
static void mhi_netdev_create_debugfs(struct mhi_netdev *mhi_netdev);

#if 0
static void mhi_netdev_skb_destructor(struct sk_buff *skb)
{
	struct mhi_skb_priv *skb_priv = (struct mhi_skb_priv *)(skb->cb);
	struct mhi_netdev *mhi_netdev = skb_priv->mhi_netdev;

	skb->data = skb->head;
	skb_reset_tail_pointer(skb);
	skb->len = 0;
	MHI_ASSERT(skb->data != skb_priv->buf, "incorrect buf");
	skb_queue_tail(&mhi_netdev->rx_allocated, skb);
}
#endif
	
#ifdef QUECTEL_BRIDGE_MODE
static const struct net_device_ops mhi_netdev_ops_ip;
static const struct net_device_ops rmnet_vnd_ops;

static int is_qmap_netdev(const struct net_device *ndev) {
	return ndev->netdev_ops == &rmnet_vnd_ops;
}

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
#ifndef ANDROID_gki
		if (tip[0] == ipv4[0] && tip[1] == ipv4[1] && (tip[2]&0xFC) == (ipv4[2]&0xFC) && tip[3] != ipv4[3])
			reply = arp_create(ARPOP_REPLY, ETH_P_ARP, *((__be32 *)sip), net, *((__be32 *)tip), sha, default_modem_addr, sha);
#endif

		if (reply) {
			skb_reset_mac_header(reply);
			__skb_pull(reply, skb_network_offset(reply));
			reply->ip_summed = CHECKSUM_UNNECESSARY;
			reply->pkt_type = PACKET_HOST;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0))
			netif_rx(reply);
#else
			netif_rx_ni(reply);
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

	if (memcmp(ehdr->h_source, bridge_mac, ETH_ALEN)) {
		return NULL;
	}

	return skb;
}

static void bridge_mode_rx_fixup(struct mhi_netdev *mhi_netdev, struct net_device *net, struct sk_buff *skb) {
	uint bridge_mode = 0;
	unsigned char *bridge_mac;

	if (mhi_netdev->qmap_mode > 0) {
		struct qmap_priv *priv = netdev_priv(net);
		bridge_mode = priv->bridge_mode;
		bridge_mac = priv->bridge_mac;
	}
	else {
		bridge_mode = mhi_netdev->bridge_mode;
		bridge_mac = mhi_netdev->bridge_mac;
	}

	if (bridge_mode)
		memcpy(eth_hdr(skb)->h_dest, bridge_mac, ETH_ALEN);
}

static ssize_t bridge_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *ndev = to_net_dev(dev);
	uint bridge_mode = 0;

	if (is_qmap_netdev(ndev)) {
		struct qmap_priv *priv = netdev_priv(ndev);
		bridge_mode = priv->bridge_mode;
	}
	else {
		struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
		bridge_mode = mhi_netdev->bridge_mode;
	}

	return snprintf(buf, PAGE_SIZE, "%u\n", bridge_mode);
}

static ssize_t bridge_mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *ndev = to_net_dev(dev);
	uint bridge_mode = simple_strtoul(buf, NULL, 0);
	
	if (ndev->type != ARPHRD_ETHER) {
		if (bridge_mode)
			netdev_err(ndev, "netdevice is not ARPHRD_ETHER\n");
		return count;
	}

	if (is_qmap_netdev(ndev)) {
		struct qmap_priv *priv = netdev_priv(ndev);
		priv->bridge_mode = bridge_mode;
	}
	else {
		struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
		mhi_netdev->bridge_mode = bridge_mode;
	}

	return count;
}


static ssize_t bridge_ipv4_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *ndev = to_net_dev(dev);
	unsigned int bridge_ipv4 = 0;
	unsigned char ipv4[4];

	if (is_qmap_netdev(ndev)) {
		struct qmap_priv *priv = netdev_priv(ndev);
		bridge_ipv4 = priv->bridge_ipv4;
	}
	else {
		struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
		bridge_ipv4 = mhi_netdev->bridge_ipv4;
	}

	ipv4[0]  = (bridge_ipv4 >> 24) & 0xFF;
	ipv4[1]  = (bridge_ipv4 >> 16) & 0xFF;
	ipv4[2]  = (bridge_ipv4 >> 8) & 0xFF;
	ipv4[3]  = (bridge_ipv4 >> 0) & 0xFF;

	return snprintf(buf, PAGE_SIZE, "%d.%d.%d.%d\n",  ipv4[0], ipv4[1], ipv4[2], ipv4[3]);
}

static ssize_t bridge_ipv4_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *ndev = to_net_dev(dev);

	if (is_qmap_netdev(ndev)) {
		struct qmap_priv *priv = netdev_priv(ndev);
		priv->bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}
	else {
		struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
		mhi_netdev->bridge_ipv4 = simple_strtoul(buf, NULL, 16);
	}

	return count;
}

static DEVICE_ATTR(bridge_mode, S_IWUSR | S_IRUGO, bridge_mode_show, bridge_mode_store);
static DEVICE_ATTR(bridge_ipv4, S_IWUSR | S_IRUGO, bridge_ipv4_show, bridge_ipv4_store);

#ifndef MHI_NETDEV_ONE_CARD_MODE
static struct attribute *pcie_mhi_qmap_sysfs_attrs[] = {
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
	NULL,
};

static struct attribute_group pcie_mhi_qmap_sysfs_attr_group = {
	.attrs = pcie_mhi_qmap_sysfs_attrs,
};
#endif
#endif

static struct sk_buff * add_mbim_hdr(struct sk_buff *skb, u8 mux_id) {
	struct mhi_mbim_hdr *mhdr;
	__le32 sign;
	u8 *c;
	u16 tci = mux_id - QUECTEL_QMAP_MUX_ID;
	unsigned int skb_len = skb->len;

	if (qmap_mode > 1)
		tci += 1; //rmnet_mhi0.X map to session X

	if (skb_headroom(skb) < sizeof(struct mhi_mbim_hdr)) {
		printk("skb_headroom small! headroom is %u, need %zd\n", skb_headroom(skb), sizeof(struct mhi_mbim_hdr));
		return NULL;
	}

	skb_push(skb, sizeof(struct mhi_mbim_hdr));

	mhdr = (struct mhi_mbim_hdr *)skb->data;

	//printk("%s %p\n", __func__, skb->data);
	mhdr->nth16.dwSignature = cpu_to_le32(USB_CDC_NCM_NTH16_SIGN);
	mhdr->nth16.wHeaderLength = cpu_to_le16(sizeof(struct usb_cdc_ncm_nth16));
/*
	Sequence number. The transmitter of a block shall set this to zero in the first NTB transferred after every 'function reset' event,
	and shall increment for every NTB subsequently transferred.
	The effect of an out-of-sequence block on the receiver is not specified.
	Thespecification allows the receiver to decide whether tocheck the sequence number, 
	and to decide how to respond if it's incorrect. The sequence number is pri-marily supplied for debugging purposes.
*/
	//mhdr->nth16.wSequence = cpu_to_le16(mhi_netdev->tx_seq++);
/*
	Size of this NTB in bytes. Represented in little-endian form.
	NTB size (IN/OUT) shall not exceed dwNtbInMaxSize or dwNtbOutMaxSize respectively
*/
	mhdr->nth16.wBlockLength = cpu_to_le16(skb->len);
/*
	Offset, in little endian, of the first NDP16 from byte zeroof the NTB.
	This value must be a multiple of 4, and must be >= 0x000C
*/
	mhdr->nth16.wNdpIndex = cpu_to_le16(sizeof(struct usb_cdc_ncm_nth16));

	sign = cpu_to_le32(USB_CDC_MBIM_NDP16_IPS_SIGN);
	c = (u8 *)&sign;
	c[3] = tci;

	mhdr->ndp16.dwSignature = sign;
	mhdr->ndp16.wLength = cpu_to_le16(sizeof(struct usb_cdc_ncm_ndp16) + sizeof(struct usb_cdc_ncm_dpe16) * 2);
	mhdr->ndp16.wNextNdpIndex = 0;

	mhdr->ndp16.dpe16[0].wDatagramIndex = sizeof(struct mhi_mbim_hdr);
	mhdr->ndp16.dpe16[0].wDatagramLength = skb_len;

	mhdr->ndp16.dpe16[1].wDatagramIndex = 0;
	mhdr->ndp16.dpe16[1].wDatagramLength = 0;

	return skb;
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

static void rmnet_map_send_ack(struct mhi_netdev *pQmapDev,
					 unsigned char type,	
					 struct rmnet_map_header *map_header)
{
	struct rmnet_map_control_command *cmd;
	struct sk_buff *skb;
	size_t skb_len = sizeof(struct rmnet_map_header) + sizeof(struct rmnet_map_control_command);

	skb = alloc_skb(skb_len, GFP_ATOMIC);
	if (!skb)
		return;
	
	skb_put(skb, skb_len);
	memcpy(skb->data, map_header, skb_len);
	cmd = (struct rmnet_map_control_command *)(skb->data + sizeof(struct rmnet_map_header));
	cmd->cmd_type = type & 0x03;
	skb->protocol = htons(ETH_P_MAP);
	skb->dev = pQmapDev->ndev;
	dev_queue_xmit(skb);
}

static int rmnet_data_vnd_do_flow_control(struct net_device *dev,
			       uint32_t map_flow_id,
			       uint16_t v4_seq,
			       uint16_t v6_seq,
			       int enable)
{
	//TODO
	return 0;
}

static uint8_t rmnet_map_do_flow_control(struct mhi_netdev *pQmapDev,
					 struct rmnet_map_header *map_header,
					 int enable) {
	struct net_device *ndev = pQmapDev->ndev;
	struct rmnet_map_control_command *cmd;
	struct net_device *vnd;
	uint8_t mux_id;
	uint16_t  ip_family;
	uint16_t  fc_seq;
	uint32_t  qos_id;
	int r;

	cmd = (struct rmnet_map_control_command *)(map_header + 1);

	mux_id = map_header->mux_id - QUECTEL_QMAP_MUX_ID;
	if (mux_id >= pQmapDev->qmap_mode) {
		netdev_info(ndev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
		return RMNET_MAP_COMMAND_UNSUPPORTED;
	}

	vnd = pQmapDev->mpQmapNetDev[mux_id];
	if (vnd == NULL) {
		netdev_info(ndev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
		return RMNET_MAP_COMMAND_UNSUPPORTED;
	}

	ip_family = cmd->flow_control.ip_family;
	fc_seq = ntohs(cmd->flow_control.flow_control_seq_num);
	qos_id = ntohl(cmd->flow_control.qos_id);

	 if (enable)
		 pQmapDev->flow_control |= (1 << mux_id);
	 else
		 pQmapDev->flow_control &= ~(1 << mux_id);
	/* Ignore the ip family and pass the sequence number for both v4 and v6
	 * sequence. User space does not support creating dedicated flows for
	 * the 2 protocols
	 */
	r = rmnet_data_vnd_do_flow_control(vnd, qos_id, fc_seq, fc_seq, enable);
	netdev_dbg(vnd, "qos_id:0x%08X, ip_family:%hd, fc_seq %hd, en:%d",
	     qos_id, ip_family & 3, fc_seq, enable);

	return RMNET_MAP_COMMAND_ACK;
}

static void rmnet_data_map_command(struct mhi_netdev *pQmapDev,
					 struct rmnet_map_header *map_header) {
	struct net_device *ndev = pQmapDev->ndev;
	struct rmnet_map_control_command *cmd;
	unsigned char command_name;
	unsigned char rc = 0;

	cmd = (struct rmnet_map_control_command *)(map_header + 1);
	command_name = cmd->command_name;

	if (command_name < RMNET_MAP_COMMAND_ENUM_LENGTH)
		pQmapDev->rmnet_map_command_stats[command_name]++;

	switch (command_name) {
	case RMNET_MAP_COMMAND_FLOW_ENABLE:
		rc = rmnet_map_do_flow_control(pQmapDev, map_header, 1);
		break;

	case RMNET_MAP_COMMAND_FLOW_DISABLE:
		rc = rmnet_map_do_flow_control(pQmapDev, map_header, 0);
		break;

	default:
		pQmapDev->rmnet_map_command_stats[RMNET_MAP_COMMAND_UNKNOWN]++;
		netdev_info(ndev, "UNSupport MAP command: %d", command_name);
		rc = RMNET_MAP_COMMAND_UNSUPPORTED;
		break;
	}

	if (rc == RMNET_MAP_COMMAND_ACK)
		rmnet_map_send_ack(pQmapDev, rc, map_header);

	return;
}

#ifndef MHI_NETDEV_ONE_CARD_MODE
static void rmnet_vnd_upate_rx_stats(struct net_device *net,
			unsigned rx_packets, unsigned rx_bytes) {
#if defined(MHI_NETDEV_STATUS64)
	struct qmap_priv *dev = netdev_priv(net);
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(dev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	stats64->rx_packets += rx_packets;
	stats64->rx_bytes += rx_bytes;
#else
	u64_stats_add(&stats64->rx_packets, rx_packets);
	u64_stats_add(&stats64->rx_bytes, rx_bytes);
#endif
	u64_stats_update_end(&stats64->syncp);
#else
	priv->self_dev->stats.rx_packets += rx_packets;
	priv->self_dev->stats.rx_bytes += rx_bytes;
#endif
}

static void rmnet_vnd_upate_tx_stats(struct net_device *net,
			unsigned tx_packets, unsigned tx_bytes) {	
#if defined(MHI_NETDEV_STATUS64)
	struct qmap_priv *dev = netdev_priv(net);
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(dev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	stats64->tx_packets += tx_packets;
	stats64->tx_bytes += tx_bytes;
#else
	u64_stats_add(&stats64->tx_packets, tx_packets);
	u64_stats_add(&stats64->tx_bytes, tx_bytes);
#endif
	u64_stats_update_end(&stats64->syncp);
#else
	net->stats.rx_packets += tx_packets;
	net->stats.rx_bytes += tx_bytes;
#endif
}

#if defined(MHI_NETDEV_STATUS64)
#ifdef ANDROID_gki
static void _netdev_stats_to_stats64(struct rtnl_link_stats64 *stats64,
			     const struct net_device_stats *netdev_stats)
{
#if BITS_PER_LONG == 64
	BUILD_BUG_ON(sizeof(*stats64) < sizeof(*netdev_stats));
	memcpy(stats64, netdev_stats, sizeof(*netdev_stats));
	/* zero out counters that only exist in rtnl_link_stats64 */
	memset((char *)stats64 + sizeof(*netdev_stats), 0,
	       sizeof(*stats64) - sizeof(*netdev_stats));
#else
	size_t i, n = sizeof(*netdev_stats) / sizeof(unsigned long);
	const unsigned long *src = (const unsigned long *)netdev_stats;
	u64 *dst = (u64 *)stats64;

	BUILD_BUG_ON(n > sizeof(*stats64) / sizeof(u64));
	for (i = 0; i < n; i++)
		dst[i] = src[i];
	/* zero out counters that only exist in rtnl_link_stats64 */
	memset((char *)stats64 + n * sizeof(u64), 0,
	       sizeof(*stats64) - n * sizeof(u64));
#endif
}
#else
static void my_netdev_stats_to_stats64(struct rtnl_link_stats64 *stats64,
			     const struct net_device_stats *netdev_stats)
{
	netdev_stats_to_stats64(stats64, netdev_stats);
}
#endif

static struct rtnl_link_stats64 *_rmnet_vnd_get_stats64(struct net_device *net, struct rtnl_link_stats64 *stats)
{
	struct qmap_priv *dev = netdev_priv(net);
	unsigned int start;
	int cpu;

	my_netdev_stats_to_stats64(stats, &net->stats);

	if (nss_cb && dev->use_qca_nss) { // rmnet_nss.c:rmnet_nss_tx() will update rx stats
		stats->rx_packets = 0;
		stats->rx_bytes = 0;
	}

	for_each_possible_cpu(cpu) {
		struct pcpu_sw_netstats *stats64;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		u64 rx_packets, rx_bytes;
		u64 tx_packets, tx_bytes;

		stats64 = per_cpu_ptr(dev->stats64, cpu);

		do {
			start = u64_stats_fetch_begin_irq(&stats64->syncp);
			rx_packets = stats64->rx_packets;
			rx_bytes = stats64->rx_bytes;
			tx_packets = stats64->tx_packets;
			tx_bytes = stats64->tx_bytes;
		} while (u64_stats_fetch_retry_irq(&stats64->syncp, start));

		stats->rx_packets += rx_packets;
		stats->rx_bytes += rx_bytes;
		stats->tx_packets += tx_packets;
		stats->tx_bytes += tx_bytes;
#else
		u64_stats_t rx_packets, rx_bytes;
		u64_stats_t tx_packets, tx_bytes;

		stats64 = per_cpu_ptr(dev->stats64, cpu);

		do {
			start = u64_stats_fetch_begin(&stats64->syncp);
			rx_packets = stats64->rx_packets;
			rx_bytes = stats64->rx_bytes;
			tx_packets = stats64->tx_packets;
			tx_bytes = stats64->tx_bytes;
		} while (u64_stats_fetch_retry(&stats64->syncp, start));

		stats->rx_packets += u64_stats_read(&rx_packets);
		stats->rx_bytes += u64_stats_read(&rx_bytes);
		stats->tx_packets += u64_stats_read(&tx_packets);
		stats->tx_bytes += u64_stats_read(&tx_bytes);
#endif
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

static void rmnet_vnd_tx_agg_work(struct work_struct *work)
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
	
	if (skb)
		dev_queue_xmit(skb);
}

static enum hrtimer_restart  rmnet_vnd_tx_agg_timer_cb(struct hrtimer *timer)
{
	struct qmap_priv *priv =
			container_of(timer, struct qmap_priv, agg_hrtimer);

	schedule_work(&priv->agg_wq);
	return HRTIMER_NORESTART;
}

static int rmnet_vnd_tx_agg(struct sk_buff *skb, struct qmap_priv *priv) {
	skb->protocol = htons(ETH_P_MAP);
	skb->dev = priv->real_dev;

	return dev_queue_xmit(skb);
}


static int rmnet_vnd_open(struct net_device *dev)
{
	struct qmap_priv *priv = netdev_priv(dev);
	struct net_device *real_dev = priv->real_dev;

	if (!(priv->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	if (netif_carrier_ok(real_dev))
		netif_carrier_on(dev);

	return 0;
}

static int rmnet_vnd_stop(struct net_device *pNet)
{
	netif_carrier_off(pNet);
	return 0;
}

static netdev_tx_t rmnet_vnd_start_xmit(struct sk_buff *skb,
					struct net_device *pNet)
{
	int err;
	struct qmap_priv *priv = netdev_priv(pNet);
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(priv->real_dev);
	int skb_len = skb->len;

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
	if (mhi_netdev->net_type == MHI_NET_MBIM) {
		if (add_mbim_hdr(skb, priv->mux_id) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}
	else {
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
	}
	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	err = rmnet_vnd_tx_agg(skb, priv);
	if (err == NET_XMIT_SUCCESS) {
		rmnet_vnd_upate_tx_stats(pNet, 1, skb_len);
	} else {
		pNet->stats.tx_errors++;
	}

	return err;
}

static int rmnet_vnd_change_mtu(struct net_device *rmnet_dev, int new_mtu)
{
	struct mhi_netdev *mhi_netdev;

	mhi_netdev = (struct mhi_netdev *)ndev_to_mhi(rmnet_dev);
	
	if (mhi_netdev == NULL) {
		printk("warning, mhi_netdev == null\n");
		return -EINVAL;
	}

	if (new_mtu < 0 )	
		return -EINVAL;

	if (new_mtu > mhi_netdev->max_mtu) {
		printk("warning, set mtu=%d greater than max mtu=%d\n", new_mtu, mhi_netdev->max_mtu);
		return -EINVAL;
	}

	rmnet_dev->mtu = new_mtu;
	return 0;
}

/* drivers may override default ethtool_ops in their bind() routine */
static const struct ethtool_ops rmnet_vnd_ethtool_ops = {
	.get_link		= ethtool_op_get_link,
};

static void rmnet_vnd_rawip_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->needed_headroom = 16;

	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
//for Qualcomm's NSS, must set type as ARPHRD_RAWIP, or NSS performace is very bad.
	rmnet_dev->type = ARPHRD_RAWIP; // do not support moify mac, for dev_set_mac_address() need ARPHRD_ETHER
	rmnet_dev->hard_header_len = 0;
//for Qualcomm's SFE, do not add IFF_POINTOPOINT to type, or SFE donot work.
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
}

static const struct net_device_ops rmnet_vnd_ops = {
	.ndo_open       = rmnet_vnd_open,
	.ndo_stop       = rmnet_vnd_stop,
	.ndo_start_xmit = rmnet_vnd_start_xmit,
#if defined(MHI_NETDEV_STATUS64)
	.ndo_get_stats64	= rmnet_vnd_get_stats64,
#endif
	.ndo_change_mtu = rmnet_vnd_change_mtu,
	.ndo_set_mac_address 	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
};

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
#endif

static void rmnet_mbim_rx_handler(void *dev, struct sk_buff *skb_in)
{
	struct mhi_netdev *pQmapDev = (struct mhi_netdev *)dev;
	struct mhi_netdev *mhi_netdev = (struct mhi_netdev *)dev;
	MHI_MBIM_CTX *ctx = &pQmapDev->mbim_ctx;
	//struct net_device *ndev = pQmapDev->ndev;
	struct usb_cdc_ncm_nth16 *nth16;
	int ndpoffset, len;
	u16 wSequence;
	struct sk_buff_head skb_chain;
	struct sk_buff *qmap_skb;

	__skb_queue_head_init(&skb_chain);

	if (skb_in->len < (sizeof(struct usb_cdc_ncm_nth16) + sizeof(struct usb_cdc_ncm_ndp16))) {
		MSG_ERR("frame too short\n");
		goto error;
	}

	nth16 = (struct usb_cdc_ncm_nth16 *)skb_in->data;

	if (nth16->dwSignature != cpu_to_le32(USB_CDC_NCM_NTH16_SIGN)) {
		MSG_ERR("invalid NTH16 signature <%#010x>\n", le32_to_cpu(nth16->dwSignature));
		goto error;
	}

	len = le16_to_cpu(nth16->wBlockLength);
	if (len > ctx->rx_max) {
		MSG_ERR("unsupported NTB block length %u/%u\n", len, ctx->rx_max);
		goto error;
	}

	wSequence = le16_to_cpu(nth16->wSequence);
	if (ctx->rx_seq !=  wSequence) {
		MSG_ERR("sequence number glitch prev=%d curr=%d\n", ctx->rx_seq, wSequence);
	}
	ctx->rx_seq = wSequence + 1;

	ndpoffset = nth16->wNdpIndex;

	while (ndpoffset > 0) {
		struct usb_cdc_ncm_ndp16 *ndp16 ;
		struct usb_cdc_ncm_dpe16 *dpe16;
		int nframes, x;
		u8 *c;
		u16 tci = 0;
		struct net_device *qmap_net;

		if (skb_in->len < (ndpoffset + sizeof(struct usb_cdc_ncm_ndp16))) {
			MSG_ERR("invalid NDP offset  <%u>\n", ndpoffset);
			goto error;
		}

		ndp16 = (struct usb_cdc_ncm_ndp16 *)(skb_in->data + ndpoffset);

		if (le16_to_cpu(ndp16->wLength) < 0x10) {
			MSG_ERR("invalid DPT16 length <%u>\n", le16_to_cpu(ndp16->wLength));
			goto error;
		}

		nframes = ((le16_to_cpu(ndp16->wLength) - sizeof(struct usb_cdc_ncm_ndp16)) / sizeof(struct usb_cdc_ncm_dpe16));

		if (skb_in->len < (sizeof(struct usb_cdc_ncm_ndp16) + nframes * (sizeof(struct usb_cdc_ncm_dpe16)))) {
			MSG_ERR("Invalid nframes = %d\n", nframes);
			goto error;
		}

		switch (ndp16->dwSignature & cpu_to_le32(0x00ffffff)) {
			case cpu_to_le32(USB_CDC_MBIM_NDP16_IPS_SIGN):
				c = (u8 *)&ndp16->dwSignature;
				tci = c[3];
				/* tag IPS<0> packets too if MBIM_IPS0_VID exists */
				//if (!tci && info->flags & FLAG_IPS0_VLAN)
				//	tci = MBIM_IPS0_VID;
			break;
			case cpu_to_le32(USB_CDC_MBIM_NDP16_DSS_SIGN):
				c = (u8 *)&ndp16->dwSignature;
				tci = c[3] + 256;
			break;
			default:
				MSG_ERR("unsupported NDP signature <0x%08x>\n", le32_to_cpu(ndp16->dwSignature));
			goto error;
		}

		if ((qmap_mode == 1 && tci != 0) || (qmap_mode > 1 && tci > qmap_mode)) {
			MSG_ERR("unsupported tci %d by now\n", tci);
			goto error;
		}
		tci = abs(tci);
		qmap_net = pQmapDev->mpQmapNetDev[qmap_mode == 1 ? 0 : tci - 1];

		dpe16 = ndp16->dpe16;

		for (x = 0; x < nframes; x++, dpe16++) {
			int offset = le16_to_cpu(dpe16->wDatagramIndex);
			int skb_len = le16_to_cpu(dpe16->wDatagramLength);

			if (offset == 0 || skb_len == 0) {
				break;
			}

			/* sanity checking */
			if (((offset + skb_len) > skb_in->len) || (skb_len > ctx->rx_max)) {
				MSG_ERR("invalid frame detected (ignored) x=%d, offset=%d, skb_len=%u\n", x, offset, skb_len);
				goto error;
			}

			qmap_skb = netdev_alloc_skb(qmap_net,  skb_len);
			if (!qmap_skb) {
				mhi_netdev->stats.alloc_failed++;
				//MSG_ERR("skb_clone fail\n"); //do not print in softirq
				goto error;
			}

			switch (skb_in->data[offset] & 0xf0) {
				case 0x40:
#ifdef CONFIG_QCA_NSS_PACKET_FILTER
					{
						struct iphdr *ip4h = (struct iphdr *)(&skb_in->data[offset]);
						if (ip4h->protocol == IPPROTO_ICMP) {
							qmap_skb->cb[0] = 1;
						}
					}
#endif
					qmap_skb->protocol = htons(ETH_P_IP);
				break;
				case 0x60:
#ifdef CONFIG_QCA_NSS_PACKET_FILTER
					{
						struct ipv6hdr *ip6h = (struct ipv6hdr *)(&skb_in->data[offset]);
						if (ip6h->nexthdr == NEXTHDR_ICMP) {
							qmap_skb->cb[0] = 1;
						}
		  }
#endif
					qmap_skb->protocol = htons(ETH_P_IPV6);
				break;
				default:
					MSG_ERR("unknow skb->protocol %02x\n", skb_in->data[offset]);
					goto error;
			}
			
			skb_put(qmap_skb, skb_len);
			memcpy(qmap_skb->data, skb_in->data + offset, skb_len);

			skb_reset_transport_header(qmap_skb);
			skb_reset_network_header(qmap_skb);
			qmap_skb->pkt_type = PACKET_HOST;
			skb_set_mac_header(qmap_skb, 0);

			if (qmap_skb->dev->type == ARPHRD_ETHER) {
				skb_push(qmap_skb, ETH_HLEN);
				skb_reset_mac_header(qmap_skb);
				memcpy(eth_hdr(qmap_skb)->h_source, default_modem_addr, ETH_ALEN);
				memcpy(eth_hdr(qmap_skb)->h_dest, qmap_net->dev_addr, ETH_ALEN);
				eth_hdr(qmap_skb)->h_proto = qmap_skb->protocol;
#ifdef QUECTEL_BRIDGE_MODE
				bridge_mode_rx_fixup(pQmapDev, qmap_net, qmap_skb);
#endif
				__skb_pull(qmap_skb, ETH_HLEN);
			}

#ifndef MHI_NETDEV_ONE_CARD_MODE
			rmnet_vnd_upate_rx_stats(qmap_net, 1, skb_len);
#endif
			__skb_queue_tail(&skb_chain, qmap_skb);
		}

		/* are there more NDPs to process? */
		ndpoffset = le16_to_cpu(ndp16->wNextNdpIndex);
	}

error:
	while ((qmap_skb = __skb_dequeue (&skb_chain))) {
		netif_receive_skb(qmap_skb);
	}	
}

static void rmnet_qmi_rx_handler(void *dev, struct sk_buff *skb_in)
{
	struct mhi_netdev *pQmapDev = (struct mhi_netdev *)dev;
	struct net_device *ndev = pQmapDev->ndev;
	struct sk_buff *qmap_skb;
	struct sk_buff_head skb_chain;
	uint dl_minimum_padding = 0;

	if (pQmapDev->qmap_version == 9)
		dl_minimum_padding = pQmapDev->dl_minimum_padding;

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
		int skip_nss = 0;

		if (map_header->next_hdr) {
			ul_header = (struct rmnet_map_v5_csum_header *)(map_header + 1);
			hdr_size += sizeof(struct rmnet_map_v5_csum_header);
		}
			
		skb_len = pkt_len - (map_header->pad_len&0x3F);
#if 0 //just for debug dl_minimum_padding BUG
		if ((skb_in->data[hdr_size] & 0xf0) == 0x45) {
			struct iphdr *ip4h = (struct iphdr *)(&skb_in->data[hdr_size]);
			if (ntohs(ip4h->tot_len) != skb_len) {
				netdev_info(ndev, "tot_len=%d skb_len=%d\n", ntohs(ip4h->tot_len), skb_len);
			}
		}
#endif
		skb_len -= dl_minimum_padding;

		mux_id = map_header->mux_id - QUECTEL_QMAP_MUX_ID;
		if (mux_id >= pQmapDev->qmap_mode) {
			netdev_info(ndev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto error_pkt;
		}
		mux_id = abs(mux_id);
		qmap_net = pQmapDev->mpQmapNetDev[mux_id];
		if (qmap_net == NULL) {
			netdev_info(ndev, "drop qmap unknow mux_id %x\n", map_header->mux_id);
			goto skip_pkt;
		}

		if (skb_len > qmap_net->mtu) {
			netdev_info(ndev, "drop skb_len=%x larger than qmap mtu=%d\n", skb_len, qmap_net->mtu);
			goto error_pkt;
		}

		if (skb_in->len < (pkt_len + hdr_size)) {
			netdev_info(ndev, "drop qmap unknow pkt, len=%d, pkt_len=%d\n", skb_in->len, pkt_len);
			goto error_pkt;
		}

		if (map_header->cd_bit) {
			rmnet_data_map_command(pQmapDev, map_header);
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
				netdev_info(ndev, "unknow skb->protocol %02x\n", skb_in->data[hdr_size]);
				goto error_pkt;
		}

//for Qualcomm's SFE, do not use skb_clone(), or SFE 's performace is very bad.
//for Qualcomm's NSS, do not use skb_clone(), or NSS 's performace is very bad.
		qmap_skb = netdev_alloc_skb(qmap_net,  skb_len);
		if (qmap_skb) {
			skb_put(qmap_skb, skb_len);
			memcpy(qmap_skb->data, skb_in->data + hdr_size, skb_len);
		}

		if (qmap_skb == NULL) {
			pQmapDev->stats.alloc_failed++;
			//netdev_info(ndev, "fail to alloc skb, pkt_len = %d\n", skb_len); //do not print in softirq
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
			__skb_pull(qmap_skb, ETH_HLEN);
		}

#ifndef MHI_NETDEV_ONE_CARD_MODE
		rmnet_vnd_upate_rx_stats(qmap_net, 1, skb_len);
#endif
		__skb_queue_tail(&skb_chain, qmap_skb);

skip_pkt:
		skb_pull(skb_in, pkt_len + hdr_size);
	}

error_pkt:
	while ((qmap_skb = __skb_dequeue (&skb_chain))) {
		netif_receive_skb(qmap_skb);
	}
}

#ifndef MHI_NETDEV_ONE_CARD_MODE
static rx_handler_result_t rmnet_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct mhi_netdev *mhi_netdev;

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
	mhi_netdev = (struct mhi_netdev *)ndev_to_mhi(skb->dev);
	
	if (mhi_netdev == NULL) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}

	if (mhi_netdev->net_type == MHI_NET_MBIM)
		rmnet_mbim_rx_handler(mhi_netdev, skb);
	else
		rmnet_qmi_rx_handler(mhi_netdev, skb);

	if (!skb_cloned(skb)) {
		if (skb_queue_len(&mhi_netdev->rx_allocated) < 128) {
			skb->data = skb->head;
			skb_reset_tail_pointer(skb);
			skb->len = 0;
			skb_queue_tail(&mhi_netdev->rx_allocated, skb);
			return RX_HANDLER_CONSUMED;
		}
	}

	consume_skb(skb);

done:
	return RX_HANDLER_CONSUMED;
}

static struct net_device * rmnet_vnd_register_device(struct mhi_netdev *pQmapDev, u8 offset_id, u8 mux_id)
{
	struct net_device *real_dev = pQmapDev->ndev;
	struct net_device *qmap_net;
	struct qmap_priv *priv;
	int err;
	int use_qca_nss = !!nss_cb;
	unsigned char temp_addr[ETH_ALEN];

	qmap_net = alloc_etherdev(sizeof(*priv));
	if (!qmap_net)
		return NULL;

	SET_NETDEV_DEV(qmap_net, &real_dev->dev);
	priv = netdev_priv(qmap_net);
	priv->offset_id = offset_id;
	priv->real_dev = pQmapDev->ndev;
	priv->self_dev = qmap_net;
	priv->pQmapDev = pQmapDev;
	priv->qmap_version = pQmapDev->qmap_version;
	priv->mux_id = mux_id;
	sprintf(qmap_net->name, "%.12s.%d", real_dev->name, offset_id + 1);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	__dev_addr_set(qmap_net, real_dev->dev_addr, ETH_ALEN);
#else
	memcpy (qmap_net->dev_addr, real_dev->dev_addr, ETH_ALEN);
#endif
	//qmap_net->dev_addr[5] = offset_id + 1;
	//eth_random_addr(qmap_net->dev_addr);	
	memcpy(temp_addr, qmap_net->dev_addr, ETH_ALEN);
	temp_addr[5] = offset_id + 1;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	__dev_addr_set(qmap_net, temp_addr, ETH_ALEN);
#else
	memcpy(qmap_net->dev_addr, temp_addr, ETH_ALEN);
#endif
#if defined(MHI_NETDEV_STATUS64)
	priv->stats64 = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!priv->stats64)
		goto out_free_newdev;
#endif

#ifdef QUECTEL_BRIDGE_MODE
	priv->bridge_mode = !!(pQmapDev->bridge_mode & BIT(offset_id));
	qmap_net->sysfs_groups[0] = &pcie_mhi_qmap_sysfs_attr_group;
	if (priv->bridge_mode)
		use_qca_nss = 0;
#endif

	priv->agg_skb = NULL;
	priv->agg_count = 0;
	hrtimer_init(&priv->agg_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	priv->agg_hrtimer.function = rmnet_vnd_tx_agg_timer_cb;
	INIT_WORK(&priv->agg_wq, rmnet_vnd_tx_agg_work);
	ktime_get_ts64(&priv->agg_time);
	spin_lock_init(&priv->agg_lock);
	priv->use_qca_nss = 0;

	qmap_net->ethtool_ops = &rmnet_vnd_ethtool_ops;
	qmap_net->netdev_ops = &rmnet_vnd_ops;
	qmap_net->flags |= IFF_NOARP;
	qmap_net->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	qmap_net->max_mtu = pQmapDev->max_mtu;
#endif

	if (nss_cb && use_qca_nss) {
		rmnet_vnd_rawip_setup(qmap_net);
	}
#ifdef CONFIG_PINCTRL_IPQ9574
	rmnet_vnd_rawip_setup(qmap_net);
#endif
	if (pQmapDev->net_type == MHI_NET_MBIM) {
		qmap_net->needed_headroom = sizeof(struct mhi_mbim_hdr);
	}
	
	err = register_netdev(qmap_net);
	pr_info("%s(%s)=%d\n", __func__, qmap_net->name, err);
	if (err == -EEXIST) {
		//'ifdown wan' for openwrt, why?
	}
	if (err < 0)
		goto out_free_newdev;

	netif_device_attach (qmap_net);
	netif_carrier_off(qmap_net);

	if (nss_cb && use_qca_nss) {
		int rc = nss_cb->nss_create(qmap_net);
		WARN_ON(rc);
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

	return qmap_net;

out_free_newdev:
	free_netdev(qmap_net);
	return qmap_net;
}

static void  rmnet_vnd_unregister_device(struct net_device *qmap_net) {
	struct qmap_priv *priv;
	unsigned long flags;

	pr_info("%s(%s)\n", __func__, qmap_net->name);
	netif_carrier_off(qmap_net);

	priv = netdev_priv(qmap_net);
	hrtimer_cancel(&priv->agg_hrtimer);
	cancel_work_sync(&priv->agg_wq);

	spin_lock_irqsave(&priv->agg_lock, flags);
	if (priv->agg_skb) {
		kfree_skb(priv->agg_skb);
		priv->agg_skb = NULL;
		priv->agg_count = 0;
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
#endif

static void rmnet_info_set(struct mhi_netdev *pQmapDev, RMNET_INFO *rmnet_info)
{	
	rmnet_info->size = sizeof(RMNET_INFO);
	rmnet_info->rx_urb_size = pQmapDev->qmap_size;
	rmnet_info->ep_type = 3; //DATA_EP_TYPE_PCIE
	rmnet_info->iface_id = 4;
	rmnet_info->qmap_mode = pQmapDev->qmap_mode;
	rmnet_info->qmap_version = pQmapDev->qmap_version;
	rmnet_info->dl_minimum_padding = 0;
}

static ssize_t qmap_mode_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *ndev = to_net_dev(dev);
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);

	return snprintf(buf, PAGE_SIZE, "%u\n",  mhi_netdev->qmap_mode);
}

static DEVICE_ATTR(qmap_mode, S_IRUGO, qmap_mode_show, NULL);

static ssize_t qmap_size_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *ndev = to_net_dev(dev);
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);

	return snprintf(buf, PAGE_SIZE, "%u\n",  mhi_netdev->qmap_size);
}

static DEVICE_ATTR(qmap_size, S_IRUGO, qmap_size_show, NULL);

static ssize_t link_state_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct net_device *ndev = to_net_dev(dev);
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);

	return snprintf(buf, PAGE_SIZE, "0x%x\n",  mhi_netdev->link_state);
}

static ssize_t link_state_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
	struct net_device *ndev = to_net_dev(dev);
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
	//struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	unsigned link_state = 0;
	unsigned old_link = mhi_netdev->link_state;
	uint offset_id = 0;

	link_state = simple_strtoul(buf, NULL, 0);
	if (mhi_netdev->qmap_mode > 1) {
		offset_id = ((link_state&0xF) - 1);

		if (0 < link_state && link_state <= mhi_netdev->qmap_mode)
			mhi_netdev->link_state |= (1 << offset_id);
		else if (0x80 < link_state && link_state <= (0x80 + mhi_netdev->qmap_mode))
			mhi_netdev->link_state &= ~(1 << offset_id);
	}
	else {
		mhi_netdev->link_state = !!link_state;
	}

	if (old_link != mhi_netdev->link_state) {
		struct net_device *qmap_net = mhi_netdev->mpQmapNetDev[offset_id];

		if (mhi_netdev->link_state)
			netif_carrier_on(mhi_netdev->ndev);
		else {
			netif_carrier_off(mhi_netdev->ndev);
		}

		if (qmap_net) {
			if (mhi_netdev->link_state & (1 << offset_id))
				netif_carrier_on(qmap_net);
			else
				netif_carrier_off(qmap_net);
		}

		dev_info(dev, "link_state 0x%x -> 0x%x\n", old_link, mhi_netdev->link_state);
	}

	return count;
}

static DEVICE_ATTR(link_state, S_IWUSR | S_IRUGO, link_state_show, link_state_store);

static struct attribute *pcie_mhi_sysfs_attrs[] = {
	&dev_attr_qmap_mode.attr,
	&dev_attr_qmap_size.attr,
	&dev_attr_link_state.attr,
#ifdef QUECTEL_BRIDGE_MODE
	&dev_attr_bridge_mode.attr,
	&dev_attr_bridge_ipv4.attr,
#endif
	NULL,
};

static struct attribute_group pcie_mhi_sysfs_attr_group = {
	.attrs = pcie_mhi_sysfs_attrs,
};

static void mhi_netdev_upate_rx_stats(struct mhi_netdev *mhi_netdev,
			unsigned rx_packets, unsigned rx_bytes) {
#if defined(MHI_NETDEV_STATUS64)
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(mhi_netdev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	stats64->rx_packets += rx_packets;
	stats64->rx_bytes += rx_bytes;
#else
	u64_stats_add(&stats64->rx_packets, rx_packets);
	u64_stats_add(&stats64->rx_bytes, rx_bytes);
#endif	
	u64_stats_update_begin(&stats64->syncp);
#else
	mhi_netdev->ndev->stats.rx_packets += rx_packets;
	mhi_netdev->ndev->stats.rx_bytes += rx_bytes;
#endif
}

static void mhi_netdev_upate_tx_stats(struct mhi_netdev *mhi_netdev,
			unsigned tx_packets, unsigned tx_bytes) {
#if defined(MHI_NETDEV_STATUS64)
	struct pcpu_sw_netstats *stats64 = this_cpu_ptr(mhi_netdev->stats64);

	u64_stats_update_begin(&stats64->syncp);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	stats64->tx_packets += tx_packets;
	stats64->tx_bytes += tx_bytes;
#else
	u64_stats_add(&stats64->tx_packets, tx_packets);
	u64_stats_add(&stats64->tx_bytes, tx_bytes);
#endif
	u64_stats_update_begin(&stats64->syncp);
#else
	mhi_netdev->ndev->stats.tx_packets += tx_packets;
	mhi_netdev->ndev->stats.tx_bytes += tx_bytes;
#endif
}

static __be16 mhi_netdev_ip_type_trans(u8 data)
{
	__be16 protocol = 0;

	/* determine L3 protocol */
	switch (data & 0xf0) {
	case 0x40:
		protocol = htons(ETH_P_IP);
		break;
	case 0x60:
		protocol = htons(ETH_P_IPV6);
		break;
	default:
		protocol = htons(ETH_P_MAP);
		break;
	}

	return protocol;
}

static int mhi_netdev_alloc_skb(struct mhi_netdev *mhi_netdev, gfp_t gfp_t)
{
	u32 cur_mru = mhi_netdev->mru;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	struct mhi_skb_priv *skb_priv;
	int ret;
	struct sk_buff *skb;
	int no_tre = mhi_get_no_free_descriptors(mhi_dev, DMA_FROM_DEVICE);
	int i;

	for (i = 0; i < no_tre; i++) {
		skb = skb_dequeue(&mhi_netdev->rx_allocated);
		if (!skb) {
			skb = alloc_skb(/*32+*/cur_mru, gfp_t);
			if (skb)
				mhi_netdev->stats.rx_allocated++;
		}
		if (!skb)
			return -ENOMEM;

		read_lock_bh(&mhi_netdev->pm_lock);
		if (unlikely(!mhi_netdev->enabled)) {
			MSG_ERR("Interface not enabled\n");
			ret = -EIO;
			goto error_queue;
		}

		skb_priv = (struct mhi_skb_priv *)skb->cb;
		skb_priv->buf = skb->data;
		skb_priv->size = cur_mru;
		skb_priv->bind_netdev = mhi_netdev;
		skb->dev = mhi_netdev->ndev;
		//skb_reserve(skb, 32); //for ethernet header

		spin_lock_bh(&mhi_netdev->rx_lock);
		ret = mhi_queue_transfer(mhi_dev, DMA_FROM_DEVICE, skb,
					 skb_priv->size, MHI_EOT);
		spin_unlock_bh(&mhi_netdev->rx_lock);

		if (ret) {
			skb_priv->bind_netdev = NULL;
			MSG_ERR("Failed to queue skb, ret:%d\n", ret);
			ret = -EIO;
			goto error_queue;
		}

		read_unlock_bh(&mhi_netdev->pm_lock);
	}

	return 0;

error_queue:
	skb->destructor = NULL;
	read_unlock_bh(&mhi_netdev->pm_lock);
	dev_kfree_skb_any(skb);

	return ret;
}

static void mhi_netdev_alloc_work(struct work_struct *work)
{
	struct mhi_netdev *mhi_netdev = container_of(work, struct mhi_netdev,
						   alloc_work.work);
	/* sleep about 1 sec and retry, that should be enough time
	 * for system to reclaim freed memory back.
	 */
	const int sleep_ms =  1000;
	int retry = 60;
	int ret;

	MSG_LOG("Entered\n");
	do {
		ret = mhi_netdev_alloc_skb(mhi_netdev, GFP_KERNEL);
		/* sleep and try again */
		if (ret == -ENOMEM) {
			schedule_delayed_work(&mhi_netdev->alloc_work, msecs_to_jiffies(20));
			return;
			msleep(sleep_ms);
			retry--;
		}
	} while (ret == -ENOMEM && retry);

	MSG_LOG("Exit with status:%d retry:%d\n", ret, retry);
}

static void mhi_netdev_dealloc(struct mhi_netdev *mhi_netdev)
{
	struct sk_buff *skb;

	skb = skb_dequeue(&mhi_netdev->rx_allocated);
	while (skb) {
		skb->destructor = NULL;
		kfree_skb(skb);
		skb = skb_dequeue(&mhi_netdev->rx_allocated);
	}
}

static int mhi_netdev_poll(struct napi_struct *napi, int budget)
{
	struct net_device *dev = napi->dev;
	struct mhi_netdev_priv *mhi_netdev_priv = netdev_priv(dev);
	struct mhi_netdev *mhi_netdev = mhi_netdev_priv->mhi_netdev;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	struct sk_buff		*skb;
	int rx_work = 0;
	int ret;

	MSG_VERB("Entered\n");

	rx_work = mhi_poll(mhi_dev, budget);

	if (rx_work < 0) {
		MSG_ERR("Error polling ret:%d\n", rx_work);
		napi_complete(napi);
		return 0;
	}

	if (mhi_netdev->net_type == MHI_NET_MBIM || mhi_netdev->net_type == MHI_NET_RMNET) {
	while ((skb = skb_dequeue (&mhi_netdev->qmap_chain))) {
#ifdef MHI_NETDEV_ONE_CARD_MODE
		int recly_skb = 0;

		mhi_netdev_upate_rx_stats(mhi_netdev, 1, skb->len);
		if (mhi_netdev->net_type == MHI_NET_MBIM)
			rmnet_mbim_rx_handler(mhi_netdev, skb);
		else
			rmnet_qmi_rx_handler(mhi_netdev, skb);

		if (!skb_cloned(skb)) {
			if (skb_queue_len(&mhi_netdev->rx_allocated) < 128) {
				skb->data = skb->head;
				skb_reset_tail_pointer(skb);
				skb->len = 0;
				skb_queue_tail(&mhi_netdev->rx_allocated, skb);
				recly_skb = 1;
			}
		}
		if (recly_skb == 0)
			dev_kfree_skb(skb);
#else
		mhi_netdev_upate_rx_stats(mhi_netdev, 1, skb->len);
		skb->dev = mhi_netdev->ndev;
		skb->protocol = htons(ETH_P_MAP);
		netif_receive_skb(skb);
#endif
	}
	}
	else  if (mhi_netdev->net_type == MHI_NET_ETHER) {
		while ((skb = skb_dequeue (&mhi_netdev->qmap_chain))) {
			mhi_netdev_upate_rx_stats(mhi_netdev, 1, skb->len);
			skb->dev = mhi_netdev->ndev;
			skb->protocol = mhi_netdev_ip_type_trans(skb->data[0]);
			netif_receive_skb(skb);
		}
	}

	/* queue new buffers */
  	if (!delayed_work_pending(&mhi_netdev->alloc_work)) {
		ret = mhi_netdev->rx_queue(mhi_netdev, GFP_ATOMIC);
		if (ret == -ENOMEM) {
			//MSG_LOG("out of tre, queuing bg worker\n"); //do not print in softirq
			mhi_netdev->stats.alloc_failed++;
			schedule_delayed_work(&mhi_netdev->alloc_work, msecs_to_jiffies(20));
		}
  	}
	
	/* complete work if # of packet processed less than allocated budget */
	if (rx_work < budget)
		napi_complete(napi);

	MSG_VERB("polled %d pkts\n", rx_work);

	return rx_work;
}

static int mhi_netdev_open(struct net_device *ndev)
{
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;

	MSG_LOG("Opened net dev interface\n");

	/* tx queue may not necessarily be stopped already
	 * so stop the queue if tx path is not enabled
	 */
	if (!mhi_dev->ul_chan)
		netif_stop_queue(ndev);
	else
		netif_start_queue(ndev);

	return 0;

}

static int mhi_netdev_change_mtu(struct net_device *ndev, int new_mtu)
{
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;

	if (new_mtu < 0 || mhi_dev->mtu < new_mtu)
		return -EINVAL;

	ndev->mtu = new_mtu;
	return 0;
}

static netdev_tx_t mhi_netdev_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct mhi_netdev_priv *mhi_netdev_priv = netdev_priv(dev);
	struct mhi_netdev *mhi_netdev = mhi_netdev_priv->mhi_netdev;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	int res = 0;
	struct skb_data *entry = (struct skb_data *)(skb->cb);

	entry->packets = 1;
	entry->length = skb->len;
	entry->bind_netdev = mhi_netdev;
		
	MSG_VERB("Entered\n");

	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);
	//qmap_hex_dump(__func__, skb->data, 32);
	
#ifdef MHI_NETDEV_ONE_CARD_MODE
	//printk("%s dev->type=%d\n", __func__, dev->type);

	if (dev->type == ARPHRD_ETHER) {		
		skb_reset_mac_header(skb);

#ifdef QUECTEL_BRIDGE_MODE
		if (mhi_netdev->bridge_mode && bridge_mode_tx_fixup(dev, skb, mhi_netdev->bridge_ipv4, mhi_netdev->bridge_mac) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
#endif

		if ((mhi_netdev->net_type == MHI_NET_RMNET || mhi_netdev->net_type == MHI_NET_MBIM)
			&& (skb_pull(skb, ETH_HLEN) == NULL)) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}

	if (mhi_netdev->net_type == MHI_NET_MBIM) {
		if (add_mbim_hdr(skb, QUECTEL_QMAP_MUX_ID) == NULL) {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}
	else if (mhi_netdev->net_type == MHI_NET_RMNET) {
		if (mhi_netdev->qmap_version == 5) {
			add_qhdr(skb, QUECTEL_QMAP_MUX_ID);
		}
		else if (mhi_netdev->qmap_version == 9) {
			add_qhdr_v5(skb, QUECTEL_QMAP_MUX_ID);
		}
		else {
			dev_kfree_skb_any (skb);
			return NETDEV_TX_OK;
		}
	}
#else
	if ((mhi_netdev->net_type == MHI_NET_RMNET || mhi_netdev->net_type == MHI_NET_MBIM)
		&& skb->protocol != htons(ETH_P_MAP)) {
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}
#endif

	if (mhi_netdev->net_type == MHI_NET_MBIM) {
		struct mhi_mbim_hdr *mhdr = (struct mhi_mbim_hdr *)skb->data;
		mhdr->nth16.wSequence = cpu_to_le16(mhi_netdev->mbim_ctx.tx_seq++);
	}

	if (unlikely(mhi_get_no_free_descriptors(mhi_dev, DMA_TO_DEVICE) < 16)) {
		u32 i = 0;
		for (i = 0; i < mhi_netdev->qmap_mode; i++) {
			struct net_device *qmap_net = mhi_netdev->mpQmapNetDev[i];
			if (qmap_net) {
				netif_stop_queue(qmap_net);
			}
		}			

		netif_stop_queue(dev);
	}

	res = mhi_queue_transfer(mhi_dev, DMA_TO_DEVICE, skb, skb->len,
				 MHI_EOT);

	//printk("%s transfer res=%d\n", __func__, res);
	if (unlikely(res)) {
		dev_kfree_skb_any(skb);
		dev->stats.tx_errors++;
	}

	MSG_VERB("Exited\n");

	return NETDEV_TX_OK;
}

#if defined(MHI_NETDEV_STATUS64)
static struct rtnl_link_stats64 * _mhi_netdev_get_stats64(struct net_device *ndev, struct rtnl_link_stats64 *stats)
{
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);

	unsigned int start;
	int cpu;

	netdev_stats_to_stats64(stats, &ndev->stats);

	for_each_possible_cpu(cpu) {
		struct pcpu_sw_netstats *stats64;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))	
		u64 rx_packets, rx_bytes;
		u64 tx_packets, tx_bytes;

		stats64 = per_cpu_ptr(mhi_netdev->stats64, cpu);

		do {
			start = u64_stats_fetch_begin_irq(&stats64->syncp);
			rx_packets = stats64->rx_packets;
			rx_bytes = stats64->rx_bytes;
			tx_packets = stats64->tx_packets;
			tx_bytes = stats64->tx_bytes;
		} while (u64_stats_fetch_retry_irq(&stats64->syncp, start));

		stats->rx_packets += rx_packets;
		stats->rx_bytes += rx_bytes;
		stats->tx_packets += tx_packets;
		stats->tx_bytes += tx_bytes;
#else
		u64_stats_t rx_packets, rx_bytes;
		u64_stats_t tx_packets, tx_bytes;

		stats64 = per_cpu_ptr(mhi_netdev->stats64, cpu);

		do {
			start = u64_stats_fetch_begin(&stats64->syncp);
			rx_packets = stats64->rx_packets;
			rx_bytes = stats64->rx_bytes;
			tx_packets = stats64->tx_packets;
			tx_bytes = stats64->tx_bytes;
		} while (u64_stats_fetch_retry(&stats64->syncp, start));

		stats->rx_packets += u64_stats_read(&rx_packets);
		stats->rx_bytes += u64_stats_read(&rx_bytes);
		stats->tx_packets += u64_stats_read(&tx_packets);
		stats->tx_bytes += u64_stats_read(&tx_bytes);
#endif			
	}

	return stats;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,10,0 )) //bc1f44709cf27fb2a5766cadafe7e2ad5e9cb221
static void mhi_netdev_get_stats64(struct net_device *ndev, struct rtnl_link_stats64 *stats) {
	_mhi_netdev_get_stats64(ndev, stats);
}
#else
static struct rtnl_link_stats64 * mhi_netdev_get_stats64(struct net_device *ndev, struct rtnl_link_stats64 *stats) {
	_mhi_netdev_get_stats64(ndev, stats);
	return stats;
}
#endif
#endif

static int qmap_setting_store(struct mhi_netdev *mhi_netdev, QMAP_SETTING *qmap_settings, size_t size) {	
	if (qmap_settings->size != size) {
		netdev_err(mhi_netdev->ndev, "ERROR: qmap_settings.size donot match!\n");
		return -EOPNOTSUPP;
	}

	mhi_netdev->dl_minimum_padding = qmap_settings->dl_minimum_padding;

	return 0;
}

static int qmap_ndo_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd) {
	struct mhi_netdev *mhi_netdev = ndev_to_mhi(dev);
	int rc = -EOPNOTSUPP;
	uint link_state = 0;
 	QMAP_SETTING qmap_settings = {0};
 
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
			rc = qmap_setting_store(mhi_netdev, &qmap_settings, sizeof(qmap_settings));
		}
	break;

	case 0x89F3: //SIOCDEVPRIVATE
		if (mhi_netdev->use_rmnet_usb) {
			rc = copy_to_user(ifr->ifr_ifru.ifru_data, &mhi_netdev->rmnet_info, sizeof(RMNET_INFO));
		}
	break;

	default:
	break;
	}

	return rc;
}

#if (LINUX_VERSION_CODE > KERNEL_VERSION( 5,14,0 )) //b9067f5dc4a07c8e24e01a1b277c6722d91be39e
#define use_ndo_siocdevprivate
#endif
#ifdef use_ndo_siocdevprivate
static int qmap_ndo_siocdevprivate(struct net_device *dev, struct ifreq *ifr, void __user *data, int cmd) {
	return qmap_ndo_do_ioctl(dev, ifr, cmd);
}
#endif

static const struct net_device_ops mhi_netdev_ops_ip = {
	.ndo_open = mhi_netdev_open,
	.ndo_start_xmit = mhi_netdev_xmit,
	//.ndo_do_ioctl = mhi_netdev_ioctl,
	.ndo_change_mtu = mhi_netdev_change_mtu,
#if defined(MHI_NETDEV_STATUS64)
	.ndo_get_stats64	= mhi_netdev_get_stats64,
#endif
	.ndo_set_mac_address = eth_mac_addr,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_do_ioctl = qmap_ndo_do_ioctl,
#ifdef use_ndo_siocdevprivate
	.ndo_siocdevprivate = qmap_ndo_siocdevprivate,
#endif
};

static void mhi_netdev_get_drvinfo (struct net_device *ndev, struct ethtool_drvinfo *info)
{
	//struct mhi_netdev *mhi_netdev = ndev_to_mhi(ndev);

	strlcpy (info->driver, "pcie_mhi", sizeof info->driver);
	strlcpy (info->version, PCIE_MHI_DRIVER_VERSION, sizeof info->version);
}

static const struct ethtool_ops mhi_netdev_ethtool_ops = {
	.get_drvinfo		= mhi_netdev_get_drvinfo,
	.get_link		= ethtool_op_get_link,
};

static void mhi_netdev_setup(struct net_device *dev)
{
	dev->netdev_ops = &mhi_netdev_ops_ip;
	ether_setup(dev);

	dev->ethtool_ops = &mhi_netdev_ethtool_ops;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0))
	__dev_addr_set (dev, node_id, sizeof node_id);
#else
	memcpy (dev->dev_addr, node_id, sizeof node_id);
#endif
	/* set this after calling ether_setup */
	dev->header_ops = 0;  /* No header */
	dev->hard_header_len = 0;
	dev->type = ARPHRD_NONE;
	dev->addr_len = 0;
	dev->flags |= IFF_NOARP;
	dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST); //POINTOPOINT will make SFE work wrong
	dev->watchdog_timeo = WATCHDOG_TIMEOUT;
	//on OpenWrt, if set rmnet_mhi0.1 as WAN, '/sbin/netifd' will auto create VLAN for rmnet_mhi0
	dev->features |= (NETIF_F_VLAN_CHALLENGED);

#ifdef MHI_NETDEV_ONE_CARD_MODE
	if (mhi_mbim_enabled) {
		dev->needed_headroom = sizeof(struct mhi_mbim_hdr);
	}
#endif
}

/* enable mhi_netdev netdev, call only after grabbing mhi_netdev.mutex */
static int mhi_netdev_enable_iface(struct mhi_netdev *mhi_netdev)
{
	int ret = 0;
#if 0
	char ifalias[IFALIASZ];
#endif
	char ifname[IFNAMSIZ];
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	int no_tre;

	MSG_LOG("Prepare the channels for transfer\n");

	ret = mhi_prepare_for_transfer(mhi_dev);
	if (ret) {
		MSG_ERR("Failed to start TX chan ret %d\n", ret);
		goto mhi_failed_to_start;
	}

	/* first time enabling the node */
	if (!mhi_netdev->ndev) {
		struct mhi_netdev_priv *mhi_netdev_priv;

#if 0
		snprintf(ifalias, sizeof(ifalias), "%s_%04x_%02u.%02u.%02u_%u",
			 mhi_netdev->interface_name, mhi_dev->dev_id,
			 mhi_dev->domain, mhi_dev->bus, mhi_dev->slot,
			 mhi_netdev->alias);
#endif

		snprintf(ifname, sizeof(ifname), "%s%d",
			 mhi_netdev->interface_name, mhi_netdev->mhi_dev->mhi_cntrl->cntrl_idx);

		rtnl_lock();
#ifdef NET_NAME_PREDICTABLE
		mhi_netdev->ndev = alloc_netdev(sizeof(*mhi_netdev_priv),
					ifname, NET_NAME_PREDICTABLE,
					mhi_netdev_setup);
#else
		mhi_netdev->ndev = alloc_netdev(sizeof(*mhi_netdev_priv),
					ifname,
					mhi_netdev_setup);
#endif

		if (!mhi_netdev->ndev) {
			ret = -ENOMEM;
			rtnl_unlock();
			goto net_dev_alloc_fail;
		}

		//mhi_netdev->ndev->mtu = mhi_dev->mtu;
		SET_NETDEV_DEV(mhi_netdev->ndev, &mhi_dev->dev);
#if 0
		dev_set_alias(mhi_netdev->ndev, ifalias, strlen(ifalias));
#endif
		mhi_netdev_priv = netdev_priv(mhi_netdev->ndev);
		mhi_netdev_priv->mhi_netdev = mhi_netdev;

		if (mhi_netdev->net_type == MHI_NET_RMNET || mhi_netdev->net_type == MHI_NET_MBIM) {
#ifdef QUECTEL_BRIDGE_MODE
		mhi_netdev->bridge_mode = bridge_mode;
#endif
		mhi_netdev->ndev->sysfs_groups[0] = &pcie_mhi_sysfs_attr_group;
		}
		else if (mhi_netdev->net_type == MHI_NET_ETHER) {
			mhi_netdev->ndev->mtu = mhi_netdev->mru;
		}
		rtnl_unlock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
		mhi_netdev->ndev->max_mtu = mhi_netdev->max_mtu; //first net card
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
		netif_napi_add_weight(mhi_netdev->ndev, &mhi_netdev->napi, mhi_netdev_poll, poll_weight);
#else
		netif_napi_add(mhi_netdev->ndev, &mhi_netdev->napi, mhi_netdev_poll, poll_weight);
#endif
		ret = register_netdev(mhi_netdev->ndev);
		if (ret) {
			MSG_ERR("Network device registration failed\n");
			goto net_dev_reg_fail;
		}

		netif_carrier_off(mhi_netdev->ndev);
	}

	write_lock_irq(&mhi_netdev->pm_lock);
	mhi_netdev->enabled =  true;
	write_unlock_irq(&mhi_netdev->pm_lock);

	/* queue buffer for rx path */
	no_tre = mhi_get_no_free_descriptors(mhi_dev, DMA_FROM_DEVICE);
	ret = mhi_netdev_alloc_skb(mhi_netdev, GFP_KERNEL);
	if (ret)
		schedule_delayed_work(&mhi_netdev->alloc_work, msecs_to_jiffies(20));

	napi_enable(&mhi_netdev->napi);

	MSG_LOG("Exited.\n");

	return 0;

net_dev_reg_fail:
	netif_napi_del(&mhi_netdev->napi);
	free_netdev(mhi_netdev->ndev);
	mhi_netdev->ndev = NULL;

net_dev_alloc_fail:
	mhi_unprepare_from_transfer(mhi_dev);

mhi_failed_to_start:
	MSG_ERR("Exited ret %d.\n", ret);

	return ret;
}

static void mhi_netdev_xfer_ul_cb(struct mhi_device *mhi_dev,
				  struct mhi_result *mhi_result)
{
	struct mhi_netdev *mhi_netdev = mhi_device_get_devdata(mhi_dev);
	struct sk_buff *skb = mhi_result->buf_addr;
	struct net_device *ndev = mhi_netdev->ndev;
	struct skb_data *entry = (struct skb_data *)(skb->cb);

	if (entry->bind_netdev != mhi_netdev) {
		MSG_ERR("%s error!\n", __func__);
		return;
	}

	if (likely(mhi_result->transaction_status == 0)) {
		mhi_netdev_upate_tx_stats(mhi_netdev, entry->packets, entry->length);

		if (netif_queue_stopped(ndev) && mhi_netdev->enabled
			&& mhi_get_no_free_descriptors(mhi_dev, DMA_TO_DEVICE) > 32) {
			int i = 0;

			netif_wake_queue(ndev);
			for (i = 0; i < mhi_netdev->qmap_mode; i++) {
				struct net_device *qmap_net = mhi_netdev->mpQmapNetDev[i];
				if (qmap_net) {
					if (netif_queue_stopped(qmap_net))
						netif_wake_queue(qmap_net);
				}
			}			
		}
	}

	entry->bind_netdev = NULL;
	entry->packets = 1;
	entry->length = 0;
	dev_kfree_skb(skb);
}

static void mhi_netdev_xfer_dl_cb(struct mhi_device *mhi_dev,
				  struct mhi_result *mhi_result)
{
	struct mhi_netdev *mhi_netdev = mhi_device_get_devdata(mhi_dev);
	struct sk_buff *skb = mhi_result->buf_addr;
	struct mhi_skb_priv *skb_priv = (struct mhi_skb_priv *)(skb->cb);

	if (unlikely(skb_priv->bind_netdev != mhi_netdev)) {
		MSG_ERR("%s error!\n", __func__);
		return;
	}

	if (unlikely(mhi_result->transaction_status)) {
		if (mhi_result->transaction_status != -ENOTCONN)
			MSG_ERR("%s transaction_status = %d!\n", __func__, mhi_result->transaction_status);
		skb_priv->bind_netdev = NULL;
		dev_kfree_skb(skb);
		return;
	}

#if defined(CONFIG_IPQ5018_RATE_CONTROL)
	if (likely(mhi_netdev->mhi_rate_control)) {
		u32 time_interval = 0;
		u32 time_difference = 0;
		u32 cntfrq;
		u64 second_jiffy;
		u64 bytes_received_2;
		struct net_device *ndev = mhi_netdev->ndev;

		if (mhi_netdev->first_jiffy) {
			#if LINUX_VERSION_CODE < KERNEL_VERSION(5,2,0)
			second_jiffy = arch_counter_get_cntvct();
			#else
			second_jiffy = __arch_counter_get_cntvct();
			#endif
			bytes_received_2 = mhi_netdev->bytes_received_2;
			if ((second_jiffy > mhi_netdev->first_jiffy) &&
					(bytes_received_2 > mhi_netdev->bytes_received_1)) {

				time_difference = (second_jiffy - mhi_netdev->first_jiffy);
				time_interval = (time_difference / mhi_netdev->cntfrq_per_msec);

				/* 1.8Gbps is 225,000,000bytes per second */
				/* We wills sample at 100ms interval */
				/* For 1ms 225000 bytes */
				/* For 100ms 22,500,000 bytes */
				/* For 10ms 2,250,000 bytes */

				/* 1.7Gbps is 212,500,000bytes per second */
				/* We wills sample at 100ms interval */
				/* For 1ms 212500 bytes */
				/* For 100ms 21,250,000 bytes */
				/* For 10ms 2,125,000 bytes */

				/* 1.6Gbps is 200,000,000bytes per second */
				/* We wills sample at 100ms interval */
				/* For 1ms 200,000 bytes */
				/* For 100ms 20,000,000 bytes */
				/* For 10ms 2,000,000 bytes */

				if (time_interval < 100) {
					if ((bytes_received_2 - mhi_netdev->bytes_received_1) > 22500000) {
						ndev->stats.rx_dropped ++;
						dev_kfree_skb(skb);
						return;
					}
				} else {
					mhi_netdev->first_jiffy = second_jiffy;
					mhi_netdev->bytes_received_1 = bytes_received_2;
				}
			} else {
				mhi_netdev->first_jiffy = second_jiffy;
				mhi_netdev->bytes_received_1 = bytes_received_2;
			}
		} else {
			#if LINUX_VERSION_CODE < KERNEL_VERSION(5,2,0)
			mhi_netdev->first_jiffy = arch_counter_get_cntvct();
			#else
			mhi_netdev->first_jiffy = __arch_counter_get_cntvct();
			#endif

			cntfrq = arch_timer_get_cntfrq();
			mhi_netdev->cntfrq_per_msec = cntfrq / 1000;
		}
		mhi_netdev->bytes_received_2 += mhi_result->bytes_xferd;
	}
#endif

#if 0
	{
		static size_t bytes_xferd = 0;
		if (mhi_result->bytes_xferd > bytes_xferd) {
			bytes_xferd = mhi_result->bytes_xferd;
			printk(KERN_DEBUG "bytes_xferd=%zd\n", bytes_xferd);
		}
	}
#endif

	skb_put(skb, mhi_result->bytes_xferd);

	qmap_hex_dump(__func__, skb->data, skb->len);

	skb_priv->bind_netdev = NULL;
	skb_queue_tail(&mhi_netdev->qmap_chain, skb);	
}

static void mhi_netdev_status_cb(struct mhi_device *mhi_dev, enum MHI_CB mhi_cb)
{
	struct mhi_netdev *mhi_netdev = mhi_device_get_devdata(mhi_dev);

	if (mhi_cb != MHI_CB_PENDING_DATA)
		return;

	if (napi_schedule_prep(&mhi_netdev->napi)) {
		__napi_schedule(&mhi_netdev->napi);
		mhi_netdev->stats.rx_int++;
		return;
	}
}

#ifdef CONFIG_DEBUG_FS

struct dentry *mhi_netdev_debugfs_dentry;

static int mhi_netdev_init_debugfs_states_show(struct seq_file *m, void *d)
{
	struct mhi_netdev *mhi_netdev = m->private;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;

#ifdef TS_DEBUG
	struct timespec now_ts, diff_ts;
	getnstimeofday(&now_ts);
	diff_ts = timespec_sub(now_ts, mhi_netdev->diff_ts);
	mhi_netdev->diff_ts = now_ts;
#endif

	seq_printf(m,
		   "tx_tre:%d rx_tre:%d qmap_chain:%u skb_chain:%u tx_allocated:%u rx_allocated:%u\n",
		    mhi_get_no_free_descriptors(mhi_dev, DMA_TO_DEVICE),
		    mhi_get_no_free_descriptors(mhi_dev, DMA_FROM_DEVICE),
		    mhi_netdev->qmap_chain.qlen,
		    mhi_netdev->skb_chain.qlen,
		    mhi_netdev->tx_allocated.qlen,
		    mhi_netdev->rx_allocated.qlen);

	seq_printf(m,
		   "netif_queue_stopped:%d, link_state:0x%x, flow_control:0x%x\n",
		    netif_queue_stopped(mhi_netdev->ndev), mhi_netdev->link_state, mhi_netdev->flow_control);

	seq_printf(m,
		   "rmnet_map_command_stats: %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n",
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_NONE],
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_FLOW_DISABLE],
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_FLOW_ENABLE],
		    mhi_netdev->rmnet_map_command_stats[3],
		    mhi_netdev->rmnet_map_command_stats[4],
		    mhi_netdev->rmnet_map_command_stats[5],
		    mhi_netdev->rmnet_map_command_stats[6],
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_FLOW_START],
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_FLOW_END],
		    mhi_netdev->rmnet_map_command_stats[RMNET_MAP_COMMAND_UNKNOWN]);

#ifdef TS_DEBUG
	seq_printf(m,
		   "qmap_ts:%ld.%ld, skb_ts:%ld.%ld, diff_ts:%ld.%ld\n",
		    mhi_netdev->qmap_ts.tv_sec, mhi_netdev->qmap_ts.tv_nsec,
		    mhi_netdev->skb_ts.tv_sec, mhi_netdev->skb_ts.tv_nsec,
		    diff_ts.tv_sec, diff_ts.tv_nsec);
	mhi_netdev->clear_ts = 1;
#endif

	return 0;
}

static int mhi_netdev_init_debugfs_states_open(struct inode *inode,
					    struct file *fp)
{
	return single_open(fp, mhi_netdev_init_debugfs_states_show, inode->i_private);
}

static const struct file_operations mhi_netdev_debugfs_state_ops = {
	.open = mhi_netdev_init_debugfs_states_open,
	.release = single_release,
	.read = seq_read,
};

static int mhi_netdev_debugfs_trigger_reset(void *data, u64 val)
{
	struct mhi_netdev *mhi_netdev = data;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	int ret;

	MSG_LOG("Triggering channel reset\n");

	/* disable the interface so no data processing */
	write_lock_irq(&mhi_netdev->pm_lock);
	mhi_netdev->enabled = false;
	write_unlock_irq(&mhi_netdev->pm_lock);
	napi_disable(&mhi_netdev->napi);

	/* disable all hardware channels */
	mhi_unprepare_from_transfer(mhi_dev);

	/* clean up all alocated buffers */
	mhi_netdev_dealloc(mhi_netdev);

	MSG_LOG("Restarting iface\n");

	ret = mhi_netdev_enable_iface(mhi_netdev);
	if (ret)
		return ret;

	return 0;
}
DEFINE_SIMPLE_ATTRIBUTE(mhi_netdev_debugfs_trigger_reset_fops, NULL,
			mhi_netdev_debugfs_trigger_reset, "%llu\n");

static void mhi_netdev_create_debugfs(struct mhi_netdev *mhi_netdev)
{
	char node_name[32];
	int i;
	const umode_t mode = 0600;
	struct mhi_device *mhi_dev = mhi_netdev->mhi_dev;
	struct dentry *dentry = mhi_netdev_debugfs_dentry;

	const struct {
		char *name;
		u32 *ptr;
	} debugfs_table[] = {
		{
			"rx_int",
			&mhi_netdev->stats.rx_int
		},
		{
			"tx_full",
			&mhi_netdev->stats.tx_full
		},
		{
			"tx_pkts",
			&mhi_netdev->stats.tx_pkts
		},
		{
			"rx_budget_overflow",
			&mhi_netdev->stats.rx_budget_overflow
		},
		{
			"rx_allocated",
			&mhi_netdev->stats.rx_allocated
		},
		{
			"tx_allocated",
			&mhi_netdev->stats.tx_allocated
		},
		{
			"alloc_failed",
			&mhi_netdev->stats.alloc_failed
		},
		{
			NULL, NULL
		},
	};

	/* Both tx & rx client handle contain same device info */
	snprintf(node_name, sizeof(node_name), "%s_%04x_%02u.%02u.%02u_%u",
		 mhi_netdev->interface_name, mhi_dev->dev_id, mhi_dev->domain,
		 mhi_dev->bus, mhi_dev->slot, mhi_netdev->alias);

	if (IS_ERR_OR_NULL(dentry))
		return;

	mhi_netdev->dentry = debugfs_create_dir(node_name, dentry);
	if (IS_ERR_OR_NULL(mhi_netdev->dentry))
		return;

	debugfs_create_u32("msg_lvl", mode, mhi_netdev->dentry,
				  (u32 *)&mhi_netdev->msg_lvl);

	/* Add debug stats table */
	for (i = 0; debugfs_table[i].name; i++) {
		debugfs_create_u32(debugfs_table[i].name, mode,
					  mhi_netdev->dentry,
					  debugfs_table[i].ptr);
	}

	debugfs_create_file("reset", mode, mhi_netdev->dentry, mhi_netdev,
			&mhi_netdev_debugfs_trigger_reset_fops);
	debugfs_create_file("states", 0444, mhi_netdev->dentry, mhi_netdev,
				   &mhi_netdev_debugfs_state_ops);
}

static void mhi_netdev_create_debugfs_dir(struct dentry *parent)
{
	mhi_netdev_debugfs_dentry = debugfs_create_dir(MHI_NETDEV_DRIVER_NAME, parent);
}

#else

static void mhi_netdev_create_debugfs(struct mhi_netdev *mhi_netdev)
{
}

static void mhi_netdev_create_debugfs_dir(struct dentry *parent)
{
}

#endif

static void mhi_netdev_remove(struct mhi_device *mhi_dev)
{
	struct mhi_netdev *mhi_netdev = mhi_device_get_devdata(mhi_dev);
	struct sk_buff *skb;

	MSG_LOG("Remove notification received\n");

	write_lock_irq(&mhi_netdev->pm_lock);
	mhi_netdev->enabled = false;
	write_unlock_irq(&mhi_netdev->pm_lock);

	if (mhi_netdev->use_rmnet_usb) {
#ifndef MHI_NETDEV_ONE_CARD_MODE
		unsigned i;

		for (i = 0; i < mhi_netdev->qmap_mode; i++) {
			if (mhi_netdev->mpQmapNetDev[i]) {
				rmnet_vnd_unregister_device(mhi_netdev->mpQmapNetDev[i]);
				mhi_netdev->mpQmapNetDev[i] = NULL;
			}
		}
		
		rtnl_lock();
#ifdef ANDROID_gki
		if (mhi_netdev->ndev && rtnl_dereference(mhi_netdev->ndev->rx_handler))
#else
		if (netdev_is_rx_handler_busy(mhi_netdev->ndev))
#endif
			netdev_rx_handler_unregister(mhi_netdev->ndev);
		rtnl_unlock();
#endif
	}

	while ((skb = skb_dequeue (&mhi_netdev->skb_chain)))
		dev_kfree_skb_any(skb);
	while ((skb = skb_dequeue (&mhi_netdev->qmap_chain)))
		dev_kfree_skb_any(skb);
	while ((skb = skb_dequeue (&mhi_netdev->rx_allocated)))
		dev_kfree_skb_any(skb);
	while ((skb = skb_dequeue (&mhi_netdev->tx_allocated)))
		dev_kfree_skb_any(skb);
	
	napi_disable(&mhi_netdev->napi);
	netif_napi_del(&mhi_netdev->napi);
	mhi_netdev_dealloc(mhi_netdev);
	unregister_netdev(mhi_netdev->ndev);
#if defined(MHI_NETDEV_STATUS64)
	free_percpu(mhi_netdev->stats64);
#endif
	free_netdev(mhi_netdev->ndev);
	flush_delayed_work(&mhi_netdev->alloc_work);

	if (!IS_ERR_OR_NULL(mhi_netdev->dentry))
		debugfs_remove_recursive(mhi_netdev->dentry);
}

static int mhi_netdev_probe(struct mhi_device *mhi_dev,
			    const struct mhi_device_id *id)
{
	int ret;
	struct mhi_netdev *mhi_netdev;

	mhi_netdev = devm_kzalloc(&mhi_dev->dev, sizeof(*mhi_netdev),
				  GFP_KERNEL);
	if (!mhi_netdev)
		return -ENOMEM;

	if (!strcmp(id->chan, "IP_HW0")) {
		if (mhi_mbim_enabled)
			mhi_netdev->net_type = MHI_NET_MBIM;
		else
			mhi_netdev->net_type = MHI_NET_RMNET;
	}
	else if (!strcmp(id->chan, "IP_SW0")) {
		mhi_netdev->net_type = MHI_NET_ETHER;
	}
	else {
		return -EINVAL;
	}

	mhi_netdev->alias = 0;

	mhi_netdev->mhi_dev = mhi_dev;
	mhi_device_set_devdata(mhi_dev, mhi_netdev);

	mhi_netdev->mru = (15*1024); ///etc/data/qnicorn_config.xml dataformat_agg_dl_size 15*1024
	mhi_netdev->max_mtu = mhi_netdev->mru - (sizeof(struct rmnet_map_v5_csum_header) + sizeof(struct rmnet_map_header));
	if (mhi_netdev->net_type == MHI_NET_MBIM) {
		mhi_netdev->mru = ncmNTBParams.dwNtbInMaxSize;
		mhi_netdev->mbim_ctx.rx_max = mhi_netdev->mru;
		mhi_netdev->max_mtu = mhi_netdev->mru - sizeof(struct mhi_mbim_hdr);
	}
	else if (mhi_netdev->net_type == MHI_NET_ETHER) {
		mhi_netdev->mru = 8*1024;
		mhi_netdev->max_mtu = mhi_netdev->mru;
	}
	mhi_netdev->qmap_size = mhi_netdev->mru;

#if defined(MHI_NETDEV_STATUS64)
	mhi_netdev->stats64 = netdev_alloc_pcpu_stats(struct pcpu_sw_netstats);
	if (!mhi_netdev->stats64)
		return -ENOMEM;
#endif

	if (!strcmp(id->chan, "IP_HW0"))
		mhi_netdev->interface_name = "rmnet_mhi";
	else if (!strcmp(id->chan, "IP_SW0"))
		mhi_netdev->interface_name = "mhi_swip";
	else
		mhi_netdev->interface_name = id->chan;

	mhi_netdev->qmap_mode = qmap_mode;
	mhi_netdev->qmap_version = 5; 
	mhi_netdev->use_rmnet_usb = 1;
	if ((mhi_dev->vendor == 0x17cb && mhi_dev->dev_id == 0x0306)
		|| (mhi_dev->vendor == 0x17cb && mhi_dev->dev_id == 0x0308)
		|| (mhi_dev->vendor == 0x1eac && mhi_dev->dev_id == 0x1004)
	) {
		mhi_netdev->qmap_version = 9;
	}
	if (mhi_netdev->net_type == MHI_NET_ETHER) {
		mhi_netdev->qmap_mode = 1;
		mhi_netdev->qmap_version = 0; 
		mhi_netdev->use_rmnet_usb = 0;
	}
	rmnet_info_set(mhi_netdev, &mhi_netdev->rmnet_info);

	mhi_netdev->rx_queue = mhi_netdev_alloc_skb;

	spin_lock_init(&mhi_netdev->rx_lock);
	rwlock_init(&mhi_netdev->pm_lock);
	INIT_DELAYED_WORK(&mhi_netdev->alloc_work, mhi_netdev_alloc_work);
	skb_queue_head_init(&mhi_netdev->qmap_chain);
	skb_queue_head_init(&mhi_netdev->skb_chain);
	skb_queue_head_init(&mhi_netdev->tx_allocated);
	skb_queue_head_init(&mhi_netdev->rx_allocated);

	mhi_netdev->msg_lvl = MHI_MSG_LVL_INFO;

	/* setup network interface */
	ret = mhi_netdev_enable_iface(mhi_netdev);
	if (ret) {
		pr_err("Error mhi_netdev_enable_iface ret:%d\n", ret);
		return ret;
	}

	mhi_netdev_create_debugfs(mhi_netdev);

	if (mhi_netdev->net_type == MHI_NET_ETHER) {
		mhi_netdev->mpQmapNetDev[0] = mhi_netdev->ndev;
		netif_carrier_on(mhi_netdev->ndev);
	}
	else if (mhi_netdev->use_rmnet_usb) {
#ifdef MHI_NETDEV_ONE_CARD_MODE
		mhi_netdev->mpQmapNetDev[0] = mhi_netdev->ndev;
		strcpy(mhi_netdev->rmnet_info.ifname[0], mhi_netdev->mpQmapNetDev[0]->name);
		mhi_netdev->rmnet_info.mux_id[0] = QUECTEL_QMAP_MUX_ID;
#else
		unsigned i;

		for (i = 0; i < mhi_netdev->qmap_mode; i++) {
			u8 mux_id = QUECTEL_QMAP_MUX_ID+i;
			mhi_netdev->mpQmapNetDev[i] = rmnet_vnd_register_device(mhi_netdev, i, mux_id);
			if (mhi_netdev->mpQmapNetDev[i]) {
				strcpy(mhi_netdev->rmnet_info.ifname[i], mhi_netdev->mpQmapNetDev[i]->name);
				mhi_netdev->rmnet_info.mux_id[i] = mux_id;
			}
		}

		rtnl_lock();
		/* when open hyfi function, run cm will make system crash */
		//netdev_rx_handler_register(mhi_netdev->ndev, rmnet_rx_handler, mhi_netdev);
		netdev_rx_handler_register(mhi_netdev->ndev, rmnet_rx_handler, NULL);
		rtnl_unlock();
#endif
	}

#if defined(CONFIG_IPQ5018_RATE_CONTROL)
	mhi_netdev->mhi_rate_control = 1;
#endif

	return 0;
}

static const struct mhi_device_id mhi_netdev_match_table[] = {
	{ .chan = "IP_HW0" },
	{ .chan = "IP_SW0" },
	// ADPL do not register as a netcard. xingduo.du 2023-02-20
	// { .chan = "IP_HW_ADPL" },
	{ },
};

static struct mhi_driver mhi_netdev_driver = {
	.id_table = mhi_netdev_match_table,
	.probe = mhi_netdev_probe,
	.remove = mhi_netdev_remove,
	.ul_xfer_cb = mhi_netdev_xfer_ul_cb,
	.dl_xfer_cb = mhi_netdev_xfer_dl_cb,
	.status_cb = mhi_netdev_status_cb,
	.driver = {
		.name = "mhi_netdev",
		.owner = THIS_MODULE,
	}
};

int __init mhi_device_netdev_init(struct dentry *parent)
{
#ifdef CONFIG_QCA_NSS_DRV
	nss_cb = rcu_dereference(rmnet_nss_callbacks);
	if (!nss_cb) {
		printk(KERN_ERR "mhi_device_netdev_init: driver load must after '/etc/modules.d/42-rmnet-nss'\n");
	}
#endif

	mhi_netdev_create_debugfs_dir(parent);

	return mhi_driver_register(&mhi_netdev_driver);
}

void mhi_device_netdev_exit(void)
{
#ifdef CONFIG_DEBUG_FS
	debugfs_remove_recursive(mhi_netdev_debugfs_dentry);
#endif
	mhi_driver_unregister(&mhi_netdev_driver);
}
