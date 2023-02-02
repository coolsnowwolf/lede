/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "sipa_eth: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/udp.h>
#include <linux/if_arp.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/of_device.h>
#include <linux/interrupt.h>
#include <linux/netdev_features.h>
#include <linux/mutex.h>
#include <linux/version.h>
#include <linux/debugfs.h>

#include "sipa_eth.h"
#include "../include/sipa.h"

#ifndef ARPHRD_RAWIP
#define ARPHRD_RAWIP ARPHRD_NONE
#endif

#ifdef CONFIG_PINCTRL_IPQ807x
//#define CONFIG_QCA_NSS_DRV
#endif

#if 1//def CONFIG_QCA_NSS_DRV
#define _RMNET_NSS_H_
#define _RMENT_NSS_H_
struct rmnet_nss_cb {
        int (*nss_create)(struct net_device *dev);
        int (*nss_free)(struct net_device *dev);
        int (*nss_tx)(struct sk_buff *skb);
};
static struct rmnet_nss_cb *rmnet_nss_callbacks __rcu __read_mostly;
#ifdef CONFIG_QCA_NSS_DRV
static uint __read_mostly qca_nss_enabled = 1;
module_param( qca_nss_enabled, uint, S_IRUGO);
#define rmnet_nss_dereference(nss_cb) do { \
	rcu_read_lock(); \
	nss_cb = rcu_dereference(rmnet_nss_callbacks); \
	rcu_read_unlock(); \
} while(0)
#else
#define rmnet_nss_dereference(nss_cb) do { nss_cb = NULL; } while(0)
#endif
#endif

/* Device status */
#define DEV_ON 1
#define DEV_OFF 0

#define SIPA_ETH_NAPI_WEIGHT 64
#define SIPA_ETH_IFACE_PREF "seth"
#define SIPA_ETH_VPCIE_PREF "pcie"
#define SIPA_ETH_VPCIE_IDX 8

#define SIPA_DUMMY_IFACE_NUM 4

static struct dentry *root;
static int sipa_eth_debugfs_mknod(void *root, void *data);
static void sipa_eth_poll_rx_handler (void *priv);
static u64 gro_enable;

struct sipa_eth_netid_device * dev_list[SIPA_DUMMY_IFACE_NUM];


static const unsigned char dhcp_dst_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

//static const u8 default_modem_addr[ETH_ALEN] = {0x02, 0x50, 0xf3};
//static const u8 default_dev_addr[ETH_ALEN] = {0x02, 0x50, 0xf4};

static inline void sipa_eth_dt_stats_init(struct sipa_eth_dtrans_stats *stats)
{
	memset(stats, 0, sizeof(*stats));
}

static inline void sipa_eth_rx_stats_update(
			struct sipa_eth_dtrans_stats *stats, u32 len)
{
	stats->rx_sum += len;
	stats->rx_cnt++;
}

static inline void sipa_eth_tx_stats_update(
			struct sipa_eth_dtrans_stats *stats, u32 len)
{
	stats->tx_sum += len;
	stats->tx_cnt++;
}

static void sipa_eth_prepare_skb(struct SIPA_ETH *sipa_eth, struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;
	struct net_device *dev;
	unsigned int real_len = 0, payload_len = 0;
	bool ip_arp = true;

	dev = sipa_eth->netdev;

	skb->protocol = eth_type_trans(skb, dev);
	skb_reset_network_header(skb);

	switch (ntohs(skb->protocol)) {
	case ETH_P_IP:
		iph = ip_hdr(skb);
		real_len = ntohs(iph->tot_len);
		break;
	case ETH_P_IPV6:
		ipv6h = ipv6_hdr(skb);
		payload_len = ntohs(ipv6h->payload_len);
		real_len = payload_len + sizeof(struct ipv6hdr);
		break;
	case ETH_P_ARP:
		real_len = arp_hdr_len(dev);
		break;
	default:
		ip_arp = false;
		break;
	}

	if (ip_arp)
		skb_trim(skb, real_len);

	/* TODO chechsum ... */
	skb->ip_summed = CHECKSUM_NONE;
	skb->dev = dev;
}

static int sipa_eth_rx(struct SIPA_ETH *sipa_eth, int budget)
{
	struct sk_buff *skb;
	struct net_device *dev;
	struct sipa_eth_dtrans_stats *dt_stats;
	int netid;
	int skb_cnt = 0;
	int ret;

	dt_stats = &sipa_eth->dt_stats;

	if (!sipa_eth) {
		pr_err("no sipa_eth device\n");
		return -EINVAL;
	}

	dev = sipa_eth->netdev;
	atomic_set(&sipa_eth->rx_evt, 0);
	while (skb_cnt < budget) {
		ret = sipa_nic_rx(&netid, &skb, skb_cnt);
		if (ret) {
			switch (ret) {
			case -ENODEV:
				pr_err("fail to find dev");
				sipa_eth->stats.rx_errors++;
				dt_stats->rx_fail++;
				dev->stats.rx_errors++;
				break;
			case -ENODATA:
				atomic_set(&sipa_eth->rx_busy, 0);
				break;
			}
			break;
		}

		if (!skb) {
			pr_err("recv skb is null\n");
			return -EINVAL;
		}

		sipa_eth_prepare_skb(sipa_eth, skb);

		sipa_eth->stats.rx_packets++;
		sipa_eth->stats.rx_bytes += skb->len;
		sipa_eth_rx_stats_update(dt_stats, skb->len);
		napi_gro_receive(&sipa_eth->napi, skb);
        //netif_receive_skb(skb);
		skb_cnt++;
	}

	return skb_cnt;
}

static int sipa_eth_rx_poll_handler(struct napi_struct *napi, int budget)
{
	struct SIPA_ETH *sipa_eth = container_of(napi, struct SIPA_ETH, napi);
	int tmp = 0, pkts;

	/* If the number of pkt is more than weight(64),
	 * we cannot read them all with a single poll.
	 * When the return value of poll func equals to weight(64),
	 * napi structure invokes the poll func one more time by
	 * __raise_softirq_irqoff.(See napi_poll for details)
	 * So do not do napi_complete in that case.
	 */
READ_AGAIN:
	/* For example:
	 * pkts = 60, tmp = 60, budget = 4
	 * if rx_evt is true, we goto READ_AGAIN,
	 * pkts = 4, tmp = 64, budget = 0,
	 * then we goto out, return 64 to napi,
	 * In that case, we force napi to do polling again.
	 */
	pkts = sipa_eth_rx(sipa_eth, budget);
	tmp += pkts;
	budget -= pkts;
	/*
	 * If budget is 0 here, means we has not finished reading yet,
	 * so we should return a weight-number(64) to napi to ask it
	 * do another polling.
	 */
	if (!budget)
		goto out;

	/* Due to a cuncurrency issue, we have to do napi_complete
	 * cautiously. If a socket is in the process of napi polling,
	 * a SIPA_RECEIVE is arriving to trigger another socket to do receiving,
	 * we must record it because it will be blocked by rx_busy
	 * at the first beginning.
	 * Since this SIPA_RECEIVE notification is a one-shot behaviour
	 * in sipa_nic. if we chose to ignore this event, we may lose
	 * the chance to receive forever.
	 */
	if (atomic_read(&sipa_eth->rx_evt))
		goto READ_AGAIN;

	/* If the number of budget is more than 0, it means the pkts
	 * we received is smaller than napi weight(64).
	 * Then we are okay to do napi_complete.
	 */
	if (budget) {
		napi_complete(napi);
		/* Test in a lab,  ten threads of TCP streams,
		 * TPUT reaches to 1Gbps, another edge case occurs,
		 * rx_busy might be 0, and rx_evt might be 1,
		 * after we do napi_complete.
		 * So do rx_handler manually to prevent
		 * sipa_eth from stopping receiving pkts.
		 */
		if (atomic_read(&sipa_eth->rx_evt) ||
		    atomic_read(&sipa_eth->rx_busy)) {
			pr_debug("rx evt recv after napi complete");
			atomic_set(&sipa_eth->rx_evt, 0);
			napi_schedule(&sipa_eth->napi);
		}
	}

out:
	return tmp;
}

/* Attention, The whole RX is deprecated, we use sipa_dummy to rx. */
static void sipa_eth_poll_rx_handler (void *priv)
{
	struct SIPA_ETH *sipa_eth = (struct SIPA_ETH *)priv;

	if (!sipa_eth) {
		pr_err("data is NULL\n");
		return;
	}

	if (!atomic_cmpxchg(&sipa_eth->rx_busy, 0, 1)) {
		atomic_set(&sipa_eth->rx_evt, 0);
		napi_schedule(&sipa_eth->napi);
		/* Trigger a NET_RX_SOFTIRQ softirq directly,
		 * or there will be a delay
		 */
		//raise_softirq(NET_RX_SOFTIRQ);
	}
}

static void sipa_eth_flowctrl_handler(void *priv, int flowctrl)
{
	struct SIPA_ETH *sipa_eth = (struct SIPA_ETH *)priv;
	struct net_device *dev = sipa_eth->netdev;

	if (flowctrl) {
		netif_stop_queue(dev);
	} else if (netif_queue_stopped(dev)) {
		netif_wake_queue(dev);
	}
}

static void sipa_eth_notify_cb(void *priv, enum sipa_evt_type evt,
			       unsigned int data)
{
	struct SIPA_ETH *sipa_eth = (struct SIPA_ETH *)priv;

	switch (evt) {
	case SIPA_RECEIVE:
		atomic_set(&sipa_eth->rx_evt, 1);
		sipa_eth_poll_rx_handler(priv);
		break;
	case SIPA_LEAVE_FLOWCTRL:
		pr_info("SIPA LEAVE FLOWCTRL\n");
		sipa_eth_flowctrl_handler(priv, 0);
		break;
	case SIPA_ENTER_FLOWCTRL:
		pr_info("SIPA ENTER FLOWCTRL\n");
		sipa_eth_flowctrl_handler(priv, 1);
		break;
	default:
		break;
	}
}

static int sipa_eth_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct SIPA_ETH *sipa_eth = netdev_priv(dev);
	struct sipa_eth_init_data *pdata = sipa_eth->pdata;
	struct sipa_eth_dtrans_stats *dt_stats;
	int ret = 0, dhcp = 0;
	int netid;

	dt_stats = &sipa_eth->dt_stats;
	if (sipa_eth->state != DEV_ON) {
		pr_err("called when %s is down\n", dev->name);
		dt_stats->tx_fail++;
		netif_carrier_off(dev);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	netid = pdata->netid;
	
        
	if (dev->type != ARPHRD_ETHER) {
		switch (skb->data[0] & 0xf0) {
			case 0x40:
			{
				struct iphdr *ip4h = (struct iphdr *)(&skb->data[0]);
				if (ip4h->protocol == IPPROTO_UDP && ip4h->saddr == 0x00000000 && ip4h->daddr == 0xFFFFFFFF) {
					dhcp = 1;
				}
				skb->protocol = htons(ETH_P_IP);			
			}
			break;
			case 0x60:
				skb->protocol = htons(ETH_P_IPV6);
			break;
			default:
				if(skb->protocol != htons(ETH_P_ARP)) {
					pr_err("unknow skb->protocol %02x\n", skb->data[0]);
					goto err;
				}
				skb->protocol = htons(ETH_P_ARP);
				arp_hdr(skb)->ar_hrd = htons(ARPHRD_ETHER);
			break;

		}
		skb_push(skb, ETH_HLEN);
		skb_reset_mac_header(skb);

		memcpy(eth_hdr(skb)->h_source, dev->dev_addr, ETH_ALEN);
		if(dhcp)
			memcpy(eth_hdr(skb)->h_dest, dhcp_dst_addr, ETH_ALEN);
		else 	
			memcpy(eth_hdr(skb)->h_dest, pdata->modem_mac, ETH_ALEN);
		eth_hdr(skb)->h_proto = skb->protocol;
	}

	ret = sipa_nic_tx(sipa_eth->nic_id, pdata->term_type, netid, skb);
	if (unlikely(ret != 0)) {
		if (ret == -EAGAIN) {
			/*
			 * resume skb, otherwise
			 * we may pull this skb ETH_HLEN-bytes twice
			 */
			if (!pdata->mac_h)
				skb_push(skb, ETH_HLEN);
			dt_stats->tx_fail++;
			sipa_eth->stats.tx_errors++;
 			/*
            if (ret == -EAGAIN) {
 				netif_stop_queue(dev);
 				sipa_nic_trigger_flow_ctrl_work(sipa_eth->nic_id, ret);
 			}
            */
			return NETDEV_TX_BUSY;
		}
		pr_err("fail to send skb, dev 0x%p eth 0x%p nic_id %d, ret %d\n",
		       dev, sipa_eth, sipa_eth->nic_id, ret);
		goto err;
	}

	/* update netdev statistics */
	sipa_eth->stats.tx_packets++;
	sipa_eth->stats.tx_bytes += skb->len;
	sipa_eth_tx_stats_update(dt_stats, skb->len);

	return NETDEV_TX_OK;

err:
	sipa_eth->netdev->stats.tx_dropped++;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

static int sipa_eth_change_dev_list_state(struct SIPA_ETH *sipa_eth, int state) {
	struct sipa_eth_netid_device *netid_dev_info;
	int netid = sipa_eth->pdata->netid;

	if (strncmp(sipa_eth->netdev->name,
		    SIPA_ETH_VPCIE_PREF,
		    strlen(SIPA_ETH_VPCIE_PREF)))
		return 0;

	if (netid < 0 || netid >= SIPA_DUMMY_IFACE_NUM) {
		pr_info("illegal netid %d\n", netid);
		return -EINVAL;
	}

	netid_dev_info = dev_list[netid];
	netid_dev_info->state = state;

	pr_info("set %s netid %d %s for dummy\n",
		sipa_eth->netdev->name, netid, state ? "DEV_ON" : "DEV_OFF");
	return 0;
}

/* Open interface */
static int sipa_eth_open(struct net_device *dev)
{
	struct SIPA_ETH *sipa_eth = netdev_priv(dev);
	struct sipa_eth_init_data *pdata = sipa_eth->pdata;
	int ret = 0;

	pr_info("dev 0x%p eth 0x%p open %s netid %d term %d mac_h %d\n",
		dev, sipa_eth, dev->name, pdata->netid, pdata->term_type,
		pdata->mac_h);
	ret = sipa_nic_open(
		pdata->term_type,
		pdata->netid,
		sipa_eth_notify_cb,
		(void *)sipa_eth);

	if (ret < 0)
		return -EINVAL;

	sipa_eth->nic_id = ret;
	sipa_eth->state = DEV_ON;
	sipa_eth_dt_stats_init(&sipa_eth->dt_stats);
	memset(&sipa_eth->stats, 0, sizeof(sipa_eth->stats));

	if (!netif_carrier_ok(sipa_eth->netdev)) {
		pr_info("set netif_carrier_on\n");
		netif_carrier_on(sipa_eth->netdev);
	}

	atomic_set(&sipa_eth->rx_busy, 0);
	sipa_eth_change_dev_list_state(sipa_eth, DEV_ON);
	netif_start_queue(dev);
	napi_enable(&sipa_eth->napi);

	return 0;
}

/* Close interface */
static int sipa_eth_close(struct net_device *dev)
{
	struct SIPA_ETH *sipa_eth = netdev_priv(dev);
	pr_info("close %s!\n", dev->name);

	sipa_nic_close(sipa_eth->nic_id);
	sipa_eth->state = DEV_OFF;

	napi_disable(&sipa_eth->napi);
	netif_stop_queue(dev);

	sipa_eth_change_dev_list_state(sipa_eth, DEV_OFF);
	return 0;
}

static struct net_device_stats *sipa_eth_get_stats(struct net_device *dev)
{
	struct SIPA_ETH *sipa_eth = netdev_priv(dev);

	return &sipa_eth->stats;
}

/*
 * For example, if an application starts a tcp connection,
 * it finally invokes tcp_connect func to send out a TCP SYN.
 * In a func tcp_init_nondata_skb, the skb->ip_summed is set
 * to CHECKSUM_PARTIAL. Because only the pesuode header is
 * calculated and stored. It expects that the netdevice
 * to calculate the checksum for TCP header&TCP paylod and store
 * the final checksum into tcphdr->check.
 * Then __dev_queue_xmit -> validate_xmit_skb, it check the features
 * of this current network card, if it with
 * NETIF_F_IP_CSUM/NETIF_F_IPV6_CSUM/NETIF_F_HW_CSUM,
 * skb_checksum_help func will be invoked for this calculation.
 *
 * So we have to implement ndo_features_check func, since we
 * have any ability to calculate a checksum for a pkt.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 3,11,0 ))
netdev_features_t sipa_eth_features_check(struct sk_buff *skb, struct net_device *dev,
					 netdev_features_t features)
{
	features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM);

	return features;
}
#endif

static const struct net_device_ops sipa_eth_ops = {
	.ndo_open = sipa_eth_open,
	.ndo_stop = sipa_eth_close,
	.ndo_start_xmit = sipa_eth_start_xmit,
	.ndo_get_stats = sipa_eth_get_stats,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 3,11,0 ))
	.ndo_features_check = sipa_eth_features_check,
#endif
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 3,11,0 ))
static int sipa_eth_parse_dt(
	struct sipa_eth_init_data **init,
	struct device *dev)
{
	struct sipa_eth_init_data *pdata = NULL;
	struct device_node *np = dev->of_node;
	int ret;
	u32 udata, id;
	s32 sdata;

	if (!np)
		pr_err("dev of_node np is null\n");

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	id = of_alias_get_id(np, "eth");
	switch (id) {
	case 0 ... 7:
		snprintf(pdata->name, IFNAMSIZ, "%s%d",
			 SIPA_ETH_IFACE_PREF, id);
		break;
	case 8 ... 11:
		snprintf(pdata->name, IFNAMSIZ, "%s%d",
			 SIPA_ETH_VPCIE_PREF, id - SIPA_ETH_VPCIE_IDX);
		break;
	default:
		pr_err("wrong alias id from dts, id %d\n", id);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "sprd,netid", &sdata);
	if (ret) {
		pr_err("read sprd,netid ret %d\n", ret);
		return ret;
	}
	/* dts reflect */
	pdata->netid = sdata - 1;

	ret = of_property_read_u32(np, "sprd,term-type", &udata);
	if (ret) {
		pr_err("read sprd,term-type ret %d\n", ret);
		return ret;
	}

	pdata->term_type = udata;

	pdata->mac_h = of_property_read_bool(np, "sprd,mac-header");

	*init = pdata;
	pr_debug("after dt parse, name %s netid %d term-type %d mac_h %d\n",
		 pdata->name, pdata->netid, pdata->term_type, pdata->mac_h);
	return 0;
}
#endif

static void s_setup(struct net_device *dev)
{
#ifndef CONFIG_SPRD_ETHERNET
        ether_setup(dev);
	dev->needed_headroom = 16;

	/* Raw IP mode */
	dev->header_ops = NULL;  /* No header */
	dev->type = ARPHRD_RAWIP;
	dev->hard_header_len = 0;
	dev->flags |= IFF_NOARP | IFF_PROMISC;
	dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);
    dev->addr_len = ETH_ALEN;
    eth_hw_addr_random(dev);
#else
	ether_setup(dev);
	dev->flags |= IFF_PROMISC;
#endif
}

static ssize_t sipa_eth_get_direct_mode(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	struct net_device *netdev = to_net_dev(dev);
	struct SIPA_ETH *sipa_eth = netdev_priv(netdev);


	if (sipa_eth->pdata->term_type == 0x0)
		return sprintf(buf, "\n %s in mode: normal\n", sipa_eth->netdev->name);
	else if (sipa_eth->pdata->term_type == 0x6)
		return sprintf(buf, "\n %s in mode: direct\n", sipa_eth->netdev->name);
	else
		return sprintf(buf, "\n %s in mode: illegal\n", sipa_eth->netdev->name);
}

static ssize_t sipa_eth_set_direct_mode(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct net_device *netdev = to_net_dev(dev);
	struct SIPA_ETH *sipa_eth = netdev_priv(netdev);

	if (!strncmp(buf, "normal", count - 1)) {
		sipa_eth->pdata->term_type = 0x0;
	} else if (!strncmp(buf, "direct", count - 1)) {
#ifndef CONFIG_SPRD_ETHERNET
		sipa_eth->pdata->term_type = 0x6;
#else
		pr_info("mode only can be set as normal when CONFIG_SPRD_ETHERNET is enabled!\n");
		return -EINVAL;
#endif
	} else {
		return -EINVAL;
    }

	return count;
}

static DEVICE_ATTR(mode, 0664, sipa_eth_get_direct_mode, sipa_eth_set_direct_mode);

static struct attribute *sipa_eth_attributes[] = {
    &dev_attr_mode.attr,
    NULL,
};

static struct attribute_group sipa_eth_attribute_group = {
    .attrs = sipa_eth_attributes,
};

static rx_handler_result_t sipa_eth_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct rmnet_nss_cb *nss_cb;

	if (!skb)
		return RX_HANDLER_CONSUMED;
	
	//printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);
 
	/* Check this so that we dont loop around netif_receive_skb */
	
	if (skb->cb[0] == 1) {
		skb->cb[0] = 0;

		return RX_HANDLER_PASS;
	}

	rmnet_nss_dereference(nss_cb);
	if (nss_cb) {
		nss_cb->nss_tx(skb);
		return RX_HANDLER_CONSUMED;
	}

	return RX_HANDLER_PASS;
}

static int sipa_eth_probe(struct platform_device *pdev)
{
	struct sipa_eth_netid_device *netid_dev_info = NULL;
	struct sipa_eth_init_data *pdata = pdev->dev.platform_data;
	struct net_device *netdev;
	struct SIPA_ETH *sipa_eth;
	char ifname[IFNAMSIZ];
	struct rmnet_nss_cb *nss_cb;
	int ret;
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION( 3,11,0 ))
	if (pdev->dev.of_node && !pdata) {
		ret = sipa_eth_parse_dt(&pdata, &pdev->dev);
		if (ret) {
			pr_err("failed to parse device tree, ret=%d\n", ret);
			return ret;
		}
		pdev->dev.platform_data = pdata;
	}
#endif

    rmnet_nss_dereference(nss_cb);
	strlcpy(ifname, pdata->name, IFNAMSIZ);
#ifdef NET_NAME_PREDICTABLE
	netdev = alloc_netdev(
		sizeof(struct SIPA_ETH),
		ifname,
		NET_NAME_PREDICTABLE,
		s_setup);
#else
	netdev = alloc_netdev(
		sizeof(struct SIPA_ETH),
		ifname,
		s_setup);
#endif

	if (!netdev) {
		pr_err("alloc_netdev() failed.\n");
		return -ENOMEM;
	}

	sipa_eth = netdev_priv(netdev);
	sipa_eth_dt_stats_init(&sipa_eth->dt_stats);
	sipa_eth->netdev = netdev;
	sipa_eth->pdata = pdata;
	atomic_set(&sipa_eth->rx_busy, 0);
	atomic_set(&sipa_eth->rx_evt, 0);
	netdev->netdev_ops = &sipa_eth_ops;
	netdev->watchdog_timeo = 1 * HZ;
	netdev->irq = 0;
	netdev->dma = 0;
	netdev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM);
	random_ether_addr(netdev->dev_addr);
        netdev->sysfs_groups[0] = &sipa_eth_attribute_group;

	netif_napi_add(netdev,
		       &sipa_eth->napi,
		       sipa_eth_rx_poll_handler,
		       SIPA_ETH_NAPI_WEIGHT);

	/* Register new Ethernet interface */
	ret = register_netdev(netdev);
	if (ret) {
		pr_err("register_netdev() failed (%d)\n", ret);
		netif_napi_del(&sipa_eth->napi);
		free_netdev(netdev);
		return ret;
	}

	sipa_eth->state = DEV_OFF;
	/* Set link as disconnected */
	netif_carrier_off(netdev);
	platform_set_drvdata(pdev, sipa_eth);
	sipa_eth_debugfs_mknod(root, (void *)sipa_eth);

	if (!strncmp(netdev->name, SIPA_ETH_VPCIE_PREF, strlen(SIPA_ETH_VPCIE_PREF))) {
		netid_dev_info = kzalloc(sizeof(*netid_dev_info), GFP_ATOMIC);
		if (!netid_dev_info)
			return -ENOMEM;

		if (nss_cb) {
			int rc = nss_cb->nss_create(netdev);
			if (rc) {
				/* Log, but don't fail the device creation */
				netdev_err(netdev, "Device will not use NSS path: %d\n", rc);
			} else {
				netdev_info(netdev, "NSS context created\n");
				rtnl_lock();
                netdev_rx_handler_register(netdev, sipa_eth_rx_handler, NULL);
				rtnl_unlock();
			}
		}

		netid_dev_info->ndev = netdev;
		netid_dev_info->napi = sipa_eth->napi;
		netid_dev_info->state = DEV_OFF;
		netid_dev_info->netid = pdata->netid;
		dev_list[pdata->netid] = netid_dev_info;
	}
	//sysfs_create_group(&pdev->dev.kobj, &sipa_eth_attribute_group);
	return 0;
}

/* Cleanup Ethernet device driver. */
static int sipa_eth_remove(struct platform_device *pdev)
{
	struct SIPA_ETH *sipa_eth = platform_get_drvdata(pdev);
	struct rmnet_nss_cb *nss_cb;
	rx_handler_func_t *rx_handler;
    struct sipa_eth_init_data *pdata = pdev->dev.platform_data;


	rmnet_nss_dereference(nss_cb);
	rcu_read_lock();
	rx_handler = rcu_dereference(sipa_eth->netdev->rx_handler);
	rcu_read_unlock();
	if (nss_cb && rx_handler == sipa_eth_rx_handler) {
			rtnl_lock();
			netdev_rx_handler_unregister(sipa_eth->netdev);
			rtnl_unlock();
			nss_cb->nss_free(sipa_eth->netdev);
	}
	
	netif_napi_del(&sipa_eth->napi);
	unregister_netdev(sipa_eth->netdev);
	
    if(dev_list[pdata->netid]) {
    	kfree(dev_list[pdata->netid]);
    	dev_list[pdata->netid] = NULL;
	}
	
	if (!IS_ERR_OR_NULL(sipa_eth->subroot))
		debugfs_remove_recursive(sipa_eth->subroot);
	free_netdev(sipa_eth->netdev);
	return 0;
}

#ifdef SPRD_PCIE_USE_DTS
static const struct of_device_id sipa_eth_match_table[] = {
	{ .compatible = "sprd,sipa_eth"},
	{ }
};
#endif

static struct platform_driver sipa_eth_driver = {
	.probe = sipa_eth_probe,
	.remove = sipa_eth_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = SIPA_ETH_IFACE_PREF,
#ifdef SPRD_PCIE_USE_DTS
		.of_match_table = sipa_eth_match_table
#endif
	}
};

#ifndef SPRD_PCIE_USE_DTS
static struct platform_device *sipa_eth_pdev[SIPA_ETH_NUM];
static struct sipa_eth_init_data *sipa_eth_data[SIPA_ETH_NUM];

static int sipa_eth_platform_device_reigster(void) 
{
	int retval = -ENOMEM;
	int i;

        for(i = 0; i < SIPA_ETH_NUM; i++) {
		sipa_eth_pdev[i] = platform_device_alloc(SIPA_ETH_IFACE_PREF, i);
		if (!sipa_eth_pdev[i]) {
			i--;
			while (i >= 0)
				platform_device_put(sipa_eth_pdev[i--]);
			return retval;
		}
        }

	for (i = 0; i < SIPA_ETH_NUM; i++) {
		sipa_eth_data[i] = kzalloc(sizeof(struct sipa_eth_init_data), GFP_KERNEL);
		if (!sipa_eth_data[i]) {
			retval = -ENOMEM;
			goto err_add_pdata;
		}

		snprintf(sipa_eth_data[i]->name, IFNAMSIZ, "%s%d",
			 	SIPA_ETH_VPCIE_PREF, i);
#ifndef CONFIG_SPRD_ETHERNET
		sipa_eth_data[i]->term_type = 0x6;
#else
		sipa_eth_data[i]->term_type = 0x0;
#endif
		sipa_eth_data[i]->netid = i;	
		sipa_eth_data[i]->mac_h = true;			

		retval = platform_device_add_data(sipa_eth_pdev[i], sipa_eth_data[i],
				sizeof(struct sipa_eth_init_data));
		if (retval)
			goto err_add_pdata;
	}

	for (i = 0; i < SIPA_ETH_NUM; i++) {
		retval = platform_device_add(sipa_eth_pdev[i]);
		if (retval < 0) {
			i--;
			while (i >= 0)
				platform_device_del(sipa_eth_pdev[i]);
			goto err_add_pdata;
		}
	}

       return retval;
err_add_pdata:
	for (i = 0; i < SIPA_ETH_NUM; i++) {
        	if(sipa_eth_data[i])
		kfree(sipa_eth_data[i]);
    }
	for (i = 0; i < SIPA_ETH_NUM; i++)
		platform_device_put(sipa_eth_pdev[i]);
	return retval;           
}

static void sipa_eth_platform_device_unreigster(void) 
{
	int i;

	for (i = 0; i < SIPA_ETH_NUM; i++) {
		struct sipa_eth_init_data *init_data;

		init_data = dev_get_platdata(&sipa_eth_pdev[i]->dev);
		platform_device_unregister(sipa_eth_pdev[i]);
	}
	
    if (!IS_ERR_OR_NULL(root))
	   debugfs_remove_recursive(root);

}
#endif

static int sipa_eth_debug_show(struct seq_file *m, void *v)
{
	struct SIPA_ETH *sipa_eth = (struct SIPA_ETH *)(m->private);
	struct sipa_eth_dtrans_stats *stats;
	struct sipa_eth_init_data *pdata;
	int i;

	if (!sipa_eth) {
		pr_err("invalid data, sipa_eth is NULL\n");
		return -EINVAL;
	}
	pdata = sipa_eth->pdata;
	stats = &sipa_eth->dt_stats;

	seq_puts(m, "*************************************************\n");
	seq_printf(m, "DEVICE: %s, term_type %d, netid %d, state %s mac_h %d\n",
		   pdata->name, pdata->term_type, pdata->netid,
		   sipa_eth->state == DEV_ON ? "UP" : "DOWN", pdata->mac_h);
	seq_puts(m, "\nRX statistics:\n");
	seq_printf(m, "rx_sum=%u, rx_cnt=%u\n",
		   stats->rx_sum,
		   stats->rx_cnt);
	seq_printf(m, "rx_fail=%u\n",
		   stats->rx_fail);

	seq_printf(m, "rx_busy=%d\n", atomic_read(&sipa_eth->rx_busy));
	seq_printf(m, "rx_evt=%d\n", atomic_read(&sipa_eth->rx_evt));

	seq_puts(m, "\nTX statistics:\n");
	seq_printf(m, "tx_sum=%u, tx_cnt=%u\n",
		   stats->tx_sum,
		   stats->tx_cnt);
	seq_printf(m, "tx_fail=%u\n",
		   stats->tx_fail);

	for (i = 0; i < SIPA_DUMMY_IFACE_NUM; i++) {
		if(dev_list[i] == NULL) {
			break;
		}
		seq_printf(m, "dev %s, netid %d state %d\n",
		   dev_list[i]->ndev->name, dev_list[i]->netid, dev_list[i]->state);
	}

	seq_puts(m, "*************************************************\n");

	return 0;
}

static int sipa_eth_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, sipa_eth_debug_show, inode->i_private);
}

static const struct file_operations sipa_eth_debug_fops = {
	.open = sipa_eth_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int debugfs_gro_enable_get(void *data, u64 *val)
{
	*val = *(u64 *)data;
	return 0;
}

static int debugfs_gro_enable_set(void *data, u64 val)
{
	*(u64 *)data = val;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(fops_gro_enable,
			debugfs_gro_enable_get,
			debugfs_gro_enable_set,
			"%llu\n");

static int sipa_eth_debugfs_mknod(void *root, void *data)
{
	struct SIPA_ETH *sipa_eth = (struct SIPA_ETH *)data;
	struct dentry *subroot;

	if (!sipa_eth)
		return -ENODEV;

	if (!root)
		return -ENXIO;
	subroot = debugfs_create_dir(sipa_eth->netdev->name, (struct dentry *)root);
	if (!subroot)
		return -ENOMEM;
		
    sipa_eth->subroot = subroot;
	debugfs_create_file("stats",
			    0444,
			    subroot,
			    data,
			    &sipa_eth_debug_fops);

	debugfs_create_file("gro_enable",
			    0600,
			    (struct dentry *)root,
			    &gro_enable,
			    &fops_gro_enable);

	return 0;
}

static void __init sipa_eth_debugfs_init(void)
{
	root = debugfs_create_dir(SIPA_ETH_IFACE_PREF, NULL);
	if (!root)
		pr_err("failed to create sipa_eth debugfs dir\n");
}

#ifdef CONFIG_QCA_NSS_DRV
/*
	EXTRA_CFLAGS="-I$(STAGING_DIR)/usr/include/qca-nss-drv  $(EXTRA_CFLAGS)"
	qsdk/qca/src/data-kernel/drivers/rmnet-nss/rmnet_nss.c 
*/
#include "rmnet_nss.c"
#endif

int sipa_eth_init(void)
{
	sipa_eth_debugfs_init();

	RCU_INIT_POINTER(rmnet_nss_callbacks, NULL);
#ifdef CONFIG_QCA_NSS_DRV
	if (qca_nss_enabled)
		rmnet_nss_init();
#endif

#ifndef SPRD_PCIE_USE_DTS
	sipa_eth_platform_device_reigster();
#endif
	return platform_driver_register(&sipa_eth_driver);
}
EXPORT_SYMBOL(sipa_eth_init);

void sipa_eth_exit(void)
{
    int i;
#ifdef CONFIG_QCA_NSS_DRV
	if (qca_nss_enabled)
		rmnet_nss_exit();
#endif
	platform_driver_unregister(&sipa_eth_driver);
#ifndef SPRD_PCIE_USE_DTS
	sipa_eth_platform_device_unreigster();
#endif
	for (i = 0; i < SIPA_ETH_NUM; i++) {
        if(sipa_eth_data[i]){
		    kfree(sipa_eth_data[i]);
        }
    }
}
EXPORT_SYMBOL(sipa_eth_exit);
