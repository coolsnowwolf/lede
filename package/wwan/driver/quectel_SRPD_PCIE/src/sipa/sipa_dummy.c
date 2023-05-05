/*
 * Copyright (C) 2020 Spreadtrum Communications Inc.
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

#define pr_fmt(fmt) "sipa_dummy: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/device.h>
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
#include <net/arp.h>

#include "sipa_eth.h"
#include "sipa_core.h"
#include "../include/sipa.h"


/* Device status */
#define DEV_ON 1
#define DEV_OFF 0

#define SIPA_DUMMY_NAPI_WEIGHT 64

extern struct sipa_eth_netid_device * dev_list[];
static struct net_device *dummy_dev;
static struct dentry *dummy_root;
static int sipa_dummy_debugfs_mknod(void *data);

#ifndef CONFIG_SPRD_ETHERNET
static int sipa_arp_reply(struct net_device *net, struct sk_buff *skb) {
	struct arphdr *parp;
    u8 *arpptr, *sha;
   	u8  sip[4], tip[4];
    struct sk_buff *reply = NULL;

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

        pr_info("%s sip = %d.%d.%d.%d, tip=%d.%d.%d.%d\n", netdev_name(net), sip[0], sip[1], sip[2], sip[3], tip[0], tip[1], tip[2], tip[3]);
       	reply = arp_create(ARPOP_REPLY, ETH_P_ARP, *((__be32 *)sip), skb->dev, *((__be32 *)tip), sha, net->dev_addr, sha);
        if (reply) {
	    	dev_queue_xmit(reply);
        }
       	return 1;
    }

    return 0;
}

static void sipa_get_modem_mac(struct sk_buff *skb, struct SIPA_ETH *sipa_eth)
{
	struct ethhdr *ehdr;
	struct iphdr *iph;
	struct udphdr *udph;
	struct sipa_eth_init_data *pdata = sipa_eth->pdata;

	ehdr = (struct ethhdr *)(skb->data - ETH_HLEN);
	iph = ip_hdr(skb);
        udph = (struct udphdr *)(skb->data + iph->ihl*4);

	if (ehdr->h_proto == htons(ETH_P_ARP)) {
		sipa_arp_reply(skb->dev, skb);
		return;
	}

        //printk("%s skb=%p, h_proto=%x, protocol=%x, saddr=%x, daddr=%x dest=%x\n", __func__, skb, ehdr->h_proto, iph->protocol, iph->saddr, iph->daddr, udph->dest);
	if (ehdr->h_proto == htons(ETH_P_IP) && iph->protocol == IPPROTO_UDP && iph->saddr != 0x00000000 && iph->daddr == 0xFFFFFFFF) {
		if (udph->dest == htons(68)) //DHCP offer/ACK
		{
			memcpy(pdata->modem_mac, ehdr->h_source, ETH_ALEN);
			pr_info("Modem Mac Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
				pdata->modem_mac[0], pdata->modem_mac[1], pdata->modem_mac[2], pdata->modem_mac[3], pdata->modem_mac[4], pdata->modem_mac[5]);
		}
	}
}
#endif

/* Term type 0x6 means we are in direct mode, currently.
 * we will recv pkt with a dummy mac header, which will
 * cause us fail to get skb->pkt_type and skb->protocol.
 */
static void sipa_dummy_prepare_skb(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;
	struct net_device *dev;
	unsigned int real_len = 0, payload_len = 0;
	bool ip_arp = true;

	dev = skb->dev;

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
	skb->pkt_type = PACKET_HOST;
}

/* Term type 0x6 means we are in direct mode, currently.
 * we will recv pkt with a dummy mac header, which will
 * cause us fail to get skb->pkt_type and skb->protocol.
 */
static void sipa_dummy_direct_mode_prepare_skb(struct sk_buff *skb)
{
	struct iphdr *iph;
	struct ipv6hdr *ipv6h;
	unsigned int real_len = 0, payload_len = 0;

	skb_pull_inline(skb, ETH_HLEN);
	skb_reset_network_header(skb);
	iph = ip_hdr(skb);
	if (iph->version == 4) {
		skb->protocol = htons(ETH_P_IP);
		iph = ip_hdr(skb);
		real_len = ntohs(iph->tot_len);
		skb_trim(skb, real_len);
	} else if(iph->version ==  6){
		skb->protocol = htons(ETH_P_IPV6);
		ipv6h = ipv6_hdr(skb);
		payload_len = ntohs(ipv6h->payload_len);
		real_len = payload_len + sizeof(struct ipv6hdr);
		skb_trim(skb, real_len);
	} else {
		pr_err("unrecognized ip version %d\n", iph->version);
	}

	skb->ip_summed = CHECKSUM_NONE;
	skb->pkt_type = PACKET_HOST;
}

static int sipa_dummy_rx(struct SIPA_DUMMY *sipa_dummy, int budget)
{
	struct sk_buff *skb;
	struct sipa_eth_netid_device *netid_dev_info;
	struct SIPA_ETH *sipa_eth;
	int real_netid = 0;
	int skb_cnt = 0;
	int ret;

	if (!sipa_dummy) {
		pr_err("no sipa_dummy device\n");
		return -EINVAL;
	}

	atomic_set(&sipa_dummy->rx_evt, 0);
	while (skb_cnt < budget) {
		ret = sipa_nic_rx(&real_netid, &skb, skb_cnt);

		if (ret) {
			switch (ret) {
			case -ENODEV:
				pr_err("sipa fail to find dev\n");
				sipa_dummy->stats.rx_errors++;
				sipa_dummy->netdev->stats.rx_errors++;
				break;
			case -ENODATA:
				pr_err("sipa no data\n");
				atomic_set(&sipa_dummy->rx_busy, 0);
				break;
			}
			break;
		}

		skb_cnt++;
		sipa_dummy->stats.rx_packets++;
		sipa_dummy->stats.rx_bytes += skb->len;
		if (real_netid < 0) {
		    pr_err("sipa invaild netid");
		    break;
		}
		/*
		 * We should determine the real device before we do eth_types_tran,
		 */
		if (real_netid < 0 || real_netid >= SIPA_DUMMY_IFACE_NUM) {
			pr_err("illegal real_netid %d\n", real_netid);
			dev_kfree_skb_any(skb);
			break;
		}
		netid_dev_info = dev_list[real_netid];
		if (!netid_dev_info || netid_dev_info->state == DEV_OFF) {
			pr_info("netid= %d net is not DEV_ON\n", real_netid);
			dev_kfree_skb_any(skb);
			break;
		}

		skb->dev = netid_dev_info->ndev;
		sipa_eth = netdev_priv(skb->dev);
        sipa_eth->stats.rx_packets++;
		sipa_eth->stats.rx_bytes += skb->len;
		if (sipa_eth->pdata->term_type == 0x6) {
			sipa_dummy_direct_mode_prepare_skb(skb);
		} else {
			sipa_dummy_prepare_skb(skb);
#ifndef CONFIG_SPRD_ETHERNET
			 sipa_get_modem_mac(skb, sipa_eth);
#endif
		}
		napi_gro_receive(&sipa_dummy->napi, skb);
	}

	return skb_cnt;
}

static int sipa_dummy_rx_poll_handler(struct napi_struct *napi, int budget)
{
     int pkts = 0, num, tmp = 0;

     struct SIPA_DUMMY *sipa_dummy = container_of(napi, struct SIPA_DUMMY, napi);

READ_AGAIN:
	num = sipa_nic_get_filled_num();
	if (!num)
	     goto check;
	if (num > budget)
	    num = budget;

	pkts = sipa_dummy_rx(sipa_dummy, num);
	if (pkts > 0)
		sipa_nic_set_tx_fifo_rp(pkts);
	tmp += pkts;

	budget -= pkts;
	if (!budget)
		goto out;

check:
	if (!sipa_check_recv_tx_fifo_empty() ||
	    atomic_read(&sipa_dummy->rx_evt)) {
		atomic_set(&sipa_dummy->rx_evt, 0);
		goto READ_AGAIN;
	}

	atomic_set(&sipa_dummy->rx_busy, 0);
	napi_complete(napi);
	sipa_nic_restore_irq();
	if (atomic_read(&sipa_dummy->rx_evt) ||
	    atomic_read(&sipa_dummy->rx_busy) ||
	    !sipa_check_recv_tx_fifo_empty()) {
		atomic_set(&sipa_dummy->rx_evt, 0);
		napi_schedule(&sipa_dummy->napi);
	}

out:
	return tmp;
}

static void sipa_dummy_rx_handler (void *priv)
{
	struct SIPA_DUMMY *sipa_dummy = (struct SIPA_DUMMY *)priv;

	if (!sipa_dummy) {
		pr_err("data is NULL\n");
		return;
	}

	if (!atomic_cmpxchg(&sipa_dummy->rx_busy, 0, 1)) {
		atomic_set(&sipa_dummy->rx_evt, 0);
		napi_schedule(&sipa_dummy->napi);
	}
}

/* for sipa to invoke */
void sipa_dummy_recv_trigger(void)
{
	struct SIPA_DUMMY *sipa_dummy;

	if (!dummy_dev)
		return;

	sipa_dummy = netdev_priv(dummy_dev);

	atomic_set(&sipa_dummy->rx_evt, 1);
	sipa_dummy_rx_handler(sipa_dummy);
}

static int sipa_dummy_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct SIPA_DUMMY *sipa_dummy = netdev_priv(dev);

        /* update netdev statistics */
        sipa_dummy->stats.tx_packets++;
        sipa_dummy->stats.tx_bytes += skb->len;

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

/* Open interface */
static int sipa_dummy_open(struct net_device *dev)
{
	struct SIPA_DUMMY *sipa_dummy = netdev_priv(dev);
    struct sipa_core *ctrl = sipa_get_ctrl_pointer();
	
    if (!ctrl) {
		return -EINVAL;
	}
			
	if(!ctrl->remote_ready)
		return -EINVAL;

	pr_info("dummy open\n");
	if (!netif_carrier_ok(sipa_dummy->netdev)) {
		netif_carrier_on(sipa_dummy->netdev);
	}

	netif_start_queue(dev);
	//napi_enable(&sipa_dummy->napi);

	napi_schedule(&sipa_dummy->napi);
	return 0;
}

/* Close interface */
static int sipa_dummy_close(struct net_device *dev)
{
	//struct SIPA_DUMMY *sipa_dummy = netdev_priv(dev);
	pr_info("close dummy!\n");

	//napi_disable(&sipa_dummy->napi);
	netif_stop_queue(dev);
	netif_carrier_off(dev);

	return 0;
}

static struct net_device_stats *sipa_dummy_get_stats(struct net_device *dev)
{
	struct SIPA_DUMMY *sipa_dummy = netdev_priv(dev);

	return &sipa_dummy->stats;
}

static const struct net_device_ops sipa_dummy_ops = {
	.ndo_open = sipa_dummy_open,
	.ndo_stop = sipa_dummy_close,
	.ndo_start_xmit = sipa_dummy_start_xmit,
	.ndo_get_stats = sipa_dummy_get_stats,
};

static void s_setup(struct net_device *dev)
{
	ether_setup(dev);
}

static int sipa_dummy_probe(struct platform_device *pdev)
{
	struct SIPA_DUMMY *sipa_dummy;
	struct net_device *netdev;
	int ret;

#ifdef NET_NAME_PREDICTABLE
	netdev = alloc_netdev(
		sizeof(struct SIPA_DUMMY),
		"sipa_dummy0",
		NET_NAME_PREDICTABLE,
		s_setup);
#else
	netdev = alloc_netdev(
		sizeof(struct SIPA_DUMMY),
		"sipa_dummy0",
		s_setup);
#endif

	if (!netdev) {
		pr_err("alloc_netdev() failed.\n");
		return -ENOMEM;
	}

	dummy_dev = netdev;
	netdev->type = ARPHRD_ETHER;
	sipa_dummy = netdev_priv(netdev);
	sipa_dummy->netdev = netdev;
	netdev->netdev_ops = &sipa_dummy_ops;
	netdev->watchdog_timeo = 1 * HZ;
	netdev->irq = 0;
	netdev->dma = 0;
	netdev->features &= ~(NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_HW_CSUM);
	random_ether_addr(netdev->dev_addr);

	netif_napi_add(netdev,
		       &sipa_dummy->napi,
		       sipa_dummy_rx_poll_handler,
		       SIPA_DUMMY_NAPI_WEIGHT);

	/* Register new Ethernet interface */
	ret = register_netdev(netdev);
	if (ret) {
		pr_err("register_netdev() failed (%d)\n", ret);
		netif_napi_del(&sipa_dummy->napi);
		free_netdev(netdev);
		return ret;
	}

	/* Set link as disconnected */
	netif_carrier_off(netdev);
	platform_set_drvdata(pdev, sipa_dummy);
	sipa_dummy_debugfs_mknod((void *)sipa_dummy);
	napi_enable(&sipa_dummy->napi);
	return 0;
}

/* Cleanup Ethernet device driver. */
static int sipa_dummy_remove(struct platform_device *pdev)
{
	struct SIPA_DUMMY *sipa_dummy= platform_get_drvdata(pdev);
	netif_stop_queue(sipa_dummy->netdev);
	napi_disable(&sipa_dummy->napi);
	netif_napi_del(&sipa_dummy->napi);
	unregister_netdev(sipa_dummy->netdev);
	free_netdev(sipa_dummy->netdev);
	platform_set_drvdata(pdev, NULL);
	if (!IS_ERR_OR_NULL(dummy_root))
		debugfs_remove_recursive(dummy_root);

	return 0;
}

#ifdef SPRD_PCIE_USE_DTS
static const struct of_device_id sipa_dummy_match_table[] = {
	{ .compatible = "sprd,sipa_dummy"},
	{ }
};
#endif

static struct platform_driver sipa_dummy_driver = {
	.probe = sipa_dummy_probe,
	.remove = sipa_dummy_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "sipa_dummy",
#ifdef SPRD_PCIE_USE_DTS
		.of_match_table = sipa_dummy_match_table
#endif
	}
};

#ifndef SPRD_PCIE_USE_DTS
static struct platform_device *sipa_dummy_device;

static int sipa_dummy_platform_device_reigster(void) 
{
	int retval = -ENOMEM;
	
	sipa_dummy_device = platform_device_alloc("sipa_dummy", -1);
	if (!sipa_dummy_device)
		return retval;
		
	retval = platform_device_add(sipa_dummy_device);
	if (retval < 0)
		platform_device_put(sipa_dummy_device);
		
	return retval;
}
#endif


static int sipa_dummy_debug_show(struct seq_file *m, void *v)
{
	struct SIPA_DUMMY *sipa_dummy = (struct SIPA_DUMMY *)(m->private);

	if (!sipa_dummy) {
		pr_err("invalid data, sipa_dummy is NULL\n");
		return -EINVAL;
	}

	seq_puts(m, "*************************************************\n");
	seq_printf(m, "DEVICE: %s rx_busy=%d rx_evt=%d\n",
		   sipa_dummy->netdev->name, atomic_read(&sipa_dummy->rx_busy),
		   atomic_read(&sipa_dummy->rx_evt));
	seq_puts(m, "*************************************************\n");

	return 0;
}

static int sipa_dummy_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, sipa_dummy_debug_show, inode->i_private);
}

static const struct file_operations sipa_dummy_debug_fops = {
	.open = sipa_dummy_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int sipa_dummy_debugfs_mknod(void *data)
{
	if (!dummy_root) {
		pr_err("dummy dir is NULL\n");
		return -ENXIO;
	}
	debugfs_create_file("stats",
			    0444,
			    dummy_root,
			    data,
			    &sipa_dummy_debug_fops);

	return 0;
}

static void __init sipa_dummy_debugfs_init(void)
{
	dummy_root = debugfs_create_dir("sipa_dummy", NULL);
	if (!dummy_root)
		pr_err("failed to create sipa_dummy debugfs dir\n");
}

int sipa_dummy_init(void)
{
	sipa_dummy_debugfs_init();
#ifndef SPRD_PCIE_USE_DTS
    sipa_dummy_platform_device_reigster();
#endif
	return platform_driver_register(&sipa_dummy_driver);
}
EXPORT_SYMBOL(sipa_dummy_init);

void sipa_dummy_exit(void)
{
	platform_driver_unregister(&sipa_dummy_driver);
#ifndef SPRD_PCIE_USE_DTS
	platform_device_unregister(sipa_dummy_device);
#endif
}
EXPORT_SYMBOL(sipa_dummy_exit);
