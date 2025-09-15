/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <net/netlink.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_platform.h"

#ifdef CONFIG_RUBY_PCIE_HOST
#include <qtn/skb_recycle.h>
#include <qtn/qtn_global.h>
#endif

#define MAX(X,Y) ((X) > (Y) ? X : Y)
#define MIN(X,Y) ((X) < (Y) ? X : Y)

/* Net device function prototypes */
int qdpc_veth_open(struct net_device *ndev);
int qdpc_veth_release(struct net_device *ndev);
struct net_device_stats *qdpc_veth_stats(struct net_device *ndev);
int qdpc_veth_change_mtu(struct net_device *ndev, int new_mtu);
int qdpc_veth_set_mac_addr(struct net_device *ndev, void *paddr);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
netdev_tx_t qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev);
#else
int qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev);
#endif

/* Net device operations structure */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
static struct net_device_ops veth_ops = {
	.ndo_open               = qdpc_veth_open,
	.ndo_stop               = qdpc_veth_release,
	.ndo_get_stats          = qdpc_veth_stats,
	.ndo_set_mac_address    = qdpc_veth_set_mac_addr,
	.ndo_change_mtu         = qdpc_veth_change_mtu,
	.ndo_start_xmit         = qdpc_veth_tx,
};
#endif

#define  QDPC_NET_STATS_LEN            10
#define  QDPC_VETH_STATS_LEN           ARRAY_SIZE(qdpc_veth_gstrings_stats)


/* Ethtool related definitions */
static const char qdpc_veth_gstrings_stats[][ETH_GSTRING_LEN] = {
	"rx_packets", "tx_packets", "rx_bytes", "tx_bytes", "rx_errors",
	"tx_errors", "rx_dropped", "tx_dropped", "multicast", "collisions",
};

static void qdpc_veth_get_drvinfo(struct net_device *ndev,
				struct ethtool_drvinfo *info);
static void qdpc_get_ethtool_stats(struct net_device *netdev,
			struct ethtool_stats *stats, u64 *data);
static void qdpc_veth_get_strings(struct net_device *netdev,
				u32 stringset, u8 *data);
static int qdpc_veth_get_sset_count(struct net_device *netdev, int sset);

static struct ethtool_ops qdpc_veth_ethtool_ops = {
	.get_drvinfo            = qdpc_veth_get_drvinfo,
	.get_strings            = qdpc_veth_get_strings,
	.get_sset_count         = qdpc_veth_get_sset_count,
	.get_ethtool_stats      = qdpc_get_ethtool_stats,
};

/* Driver Name */
extern char qdpc_pcie_driver_name[];

static inline bool check_netlink_magic(qdpc_cmd_hdr_t *cmd_hdr)
{
	return ((memcmp(cmd_hdr->dst_magic, QDPC_NETLINK_DST_MAGIC, ETH_ALEN) == 0)
		&& (memcmp(cmd_hdr->src_magic, QDPC_NETLINK_SRC_MAGIC, ETH_ALEN) == 0));
}

int qdpc_veth_open(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);

	if (!qdpc_isbootstate(priv, QDPC_BDA_FW_RUNNING))
		return -ENODEV;

	flush_scheduled_work();
	netif_start_queue(ndev);

	return SUCCESS;
}

int qdpc_veth_release(struct net_device *ndev)
{
	/* release ports, irq and such -- like fops->close */
	netif_stop_queue(ndev); /* can't transmit any more */

	return SUCCESS;
}

SRAM_TEXT struct sk_buff *qdpc_get_skb(struct qdpc_priv *priv, size_t len)
{
	const uint32_t align = dma_get_cache_alignment();
	struct sk_buff *skb = NULL;
	uint32_t off;

#if (defined(CONFIG_RUBY_PCIE_HOST) && defined(QDPC_USE_SKB_RECYCLE))
	uint32_t size;
	struct qtn_skb_recycle_list *recycle_list = qtn_get_shared_recycle_list();
	if (recycle_list) {
		skb = qtn_skb_recycle_list_pop(recycle_list, &recycle_list->stats_pcie);
	}

	if (!skb) {
		size = qtn_rx_buf_size();
		if (len > size)
			size = len;
		skb = dev_alloc_skb(size + align);
	}
	if (skb)
		skb->recycle_list = recycle_list;
#else
	skb = dev_alloc_skb(len + align);
#endif

	if (skb) {
		off = ((uint32_t)((unsigned long)skb->data)) & (align - 1);
		/* skb->data should be cache aligned - do calculation here to be sure. */
		if (off) {
			skb_reserve(skb, align - off);
		}
	}

	return skb;
}

static inline SRAM_TEXT void qdpc_tx_skb_recycle(struct sk_buff *skb)
{
#if (defined(CONFIG_RUBY_PCIE_HOST) && defined(QDPC_USE_SKB_RECYCLE))

	struct qtn_skb_recycle_list *recycle_list = qtn_get_shared_recycle_list();

	if (qtn_skb_recycle_list_push(recycle_list,
			&recycle_list->stats_pcie, skb))
		return;
#endif
	dev_kfree_skb_any(skb);
}

void qdpc_netlink_rx(struct net_device *ndev, void *buf, size_t len)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	struct sk_buff *skb = nlmsg_new(len, 0);
	struct nlmsghdr *nlh;

	if (skb == NULL) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "WARNING: out of netlink SKBs\n");
		return;
	}

	nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, len, 0);  ;
	memcpy(nlmsg_data(nlh), buf, len);
	NETLINK_CB(skb).dst_group = 0;

	/* Send the netlink message to user application */
	nlmsg_unicast(priv->netlink_socket, skb, priv->netlink_pid);

}

static inline SRAM_TEXT int qdpc_map_buffer(struct qdpc_priv *priv, qdpc_desc_t *desc, int mapdir)
{
	desc->dd_paddr = pci_map_single(priv->pdev, desc->dd_vaddr, desc->dd_size, mapdir);
	return 0;
}

static inline SRAM_TEXT int qdpc_unmap_buffer(struct qdpc_priv *priv, qdpc_desc_t *desc, uint32_t mapdir)
{
	pci_unmap_single(priv->pdev, desc->dd_paddr, desc->dd_size, mapdir);
	desc->dd_paddr = 0;
	return 0;
}

static SRAM_TEXT size_t qdpc_rx_process_frame(struct net_device *ndev,
	qdpc_desc_t *rx_desc, uint32_t dma_status, bool lastdesc)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_dmadesc_t *rx_hwdesc = rx_desc->dd_hwdesc;
	struct sk_buff *skb = NULL;
	const uint32_t buffer_size = QDPC_DMA_MAXBUF;
	uint32_t dma_data = 0;
	uint32_t dma_control = 0;
	size_t dmalen;
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	void *data;
#endif
	struct ethhdr *eth;
	qdpc_cmd_hdr_t *cmd_hdr;

	dmalen = QDPC_DMA_RXLEN(dma_status);
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	data = rx_desc->dd_metadata;

	if ((dmalen >= QDPC_DMA_MINBUF) && (dmalen <= QDPC_DMA_MAXBUF)
		&& (QDPC_DMA_SINGLE_BUFFER(dma_status))){
		eth = (struct ethhdr *)rx_desc->dd_metadata;

		switch (ntohs(eth->h_proto)) {
		case QDPC_APP_NETLINK_TYPE:
			/* Double Check if it's netlink packet*/
			cmd_hdr = (qdpc_cmd_hdr_t *)rx_desc->dd_metadata;
			if (check_netlink_magic(cmd_hdr)) {
				qdpc_netlink_rx(ndev, rx_desc->dd_metadata
					+ sizeof(qdpc_cmd_hdr_t), ntohs(cmd_hdr->len));
			}
			break;
		default:
			skb = qdpc_get_skb(priv, dmalen);
			if (skb) {
				skb_put(skb, dmalen);
				memcpy(skb->data, rx_desc->dd_vaddr, dmalen);
				skb->dev = ndev;
				skb->ip_summed = CHECKSUM_UNNECESSARY;
				skb->protocol = eth_type_trans(skb, ndev);
				netif_rx(skb);
			} else {
				DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
					"WARNING: out of SKBs\n");
			}
			break;
		}
	} else {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"RX: Drop packet.S:0x%x B:0x%x L:%u\n", dma_status, rx_hwdesc->dma_data, (uint32_t)dmalen);
	}

	rx_desc->dd_vaddr = data;
	rx_desc->dd_size = buffer_size;
	qdpc_map_buffer(priv, rx_desc, PCI_DMA_FROMDEVICE);
	dma_data = rx_desc->dd_paddr + priv->epmem.eps_dma_offset;
	dma_control = ((buffer_size & QDPC_DMA_LEN_MASK)|(lastdesc ? QDPC_DMA_LAST_DESC : 0));
#else
	skb = (struct sk_buff *)rx_desc->dd_metadata;

	if (skb && (dmalen >= QDPC_DMA_MINBUF) && (dmalen <= QDPC_DMA_MAXBUF)
		&& (QDPC_DMA_SINGLE_BUFFER(dma_status))) {
		eth = (struct ethhdr *)skb->data;
		switch (ntohs(eth->h_proto)) {
		case QDPC_APP_NETLINK_TYPE:
			/* Double Check if it's netlink packet*/
			cmd_hdr = (qdpc_cmd_hdr_t *)skb->data;
			if (check_netlink_magic(cmd_hdr)) {
				qdpc_netlink_rx(ndev, skb->data +
					sizeof(qdpc_cmd_hdr_t), ntohs(cmd_hdr->len));
			}
			break;
		default:
			skb->dev = ndev;
			skb->len = 0;
			skb_reset_tail_pointer(skb);
			skb_put(skb, dmalen);
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb->protocol = eth_type_trans(skb, ndev);
			netif_rx(skb);
			skb = NULL;
			break;
		}
	} else {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"RX: Drop packet. Skb: 0x%p Status:0x%x Len:%u\n", skb, dma_status, dmalen);
	}

	if (skb)
		dev_kfree_skb_any(skb);

	skb = qdpc_get_skb(priv, buffer_size);

	if (skb == NULL) {
		rx_desc->dd_vaddr = 0;
		rx_desc->dd_size = 0;
		dma_data = 0;
		dma_control = (lastdesc ? QDPC_DMA_LAST_DESC : 0);
	} else {
		rx_desc->dd_vaddr = skb->data;
		rx_desc->dd_size = buffer_size;
		qdpc_map_buffer(priv, rx_desc, PCI_DMA_FROMDEVICE);
		dma_data = rx_desc->dd_paddr + priv->epmem.eps_dma_offset;
		dma_control = ((buffer_size & QDPC_DMA_LEN_MASK)|(lastdesc ? QDPC_DMA_LAST_DESC : 0));
	}

	rx_desc->dd_metadata = (void *)skb;
#endif

	qdpc_pci_writel(dma_data, &rx_hwdesc->dma_data);
	qdpc_pci_writel(dma_control, &rx_hwdesc->dma_control);

	/*
	  * Memory mapped IO barrier, if defined on systems forces the other parts of the descriptor to complete
	  * before ownership status is changed
	  */
	mmiowb();
	qdpc_pci_writel(QDPC_DMA_OWN, &rx_hwdesc->dma_status);

	return dmalen;
}

static SRAM_TEXT void qdpc_indicate_peer_rx_nfree(struct qdpc_priv *priv,
	qdpc_pdring_t *rxq, qdpc_desc_t *rx_desc, uint32_t dma_status)
{
	uint32_t nfree = rxq->pd_ringsize;
	qdpc_desc_t *rx_last_desc;
	unsigned long flags;

	local_irq_save(flags);
	if (unlikely(QDPC_DMA_OWNED(dma_status) == 0)) {
		rx_last_desc = rx_desc;
		for(rx_desc = rxq->pd_nextdesc; (rx_desc != rx_last_desc) && (nfree > 0);) {
			dma_status = qdpc_pci_readl(&rx_desc->dd_hwdesc->dma_status);
			if (QDPC_DMA_OWNED(dma_status) == 0) {
				nfree--;
			} else {
				break;
			}

			if (rx_desc == rxq->pd_lastdesc) {
				rx_desc = rxq->pd_firstdesc;
			} else {
				rx_desc++;
			}
		}
		if (nfree <= QDPC_VETH_RX_LOW_WATERMARK)
			nfree = 0;
	}

	qdpc_pci_writel(nfree, priv->host_h2ep_txd_budget);
	mmiowb();
	local_irq_restore(flags);
}

SRAM_TEXT void qdpc_veth_rx(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *rxq = &priv->pktq.pkt_usq;
	uint32_t budget = QDPC_DESC_RING_SIZE << 1;
	uint32_t dma_status = 0;
	qdpc_desc_t *rx_desc;
	qdpc_dmadesc_t *rx_hwdesc;
	bool lastdesc;
	size_t pktlen;

	if (rxq->pd_nextdesc == NULL)
		return;

	rx_desc = rxq->pd_nextdesc;
	rx_hwdesc = rx_desc->dd_hwdesc;
	dma_status = qdpc_pci_readl(&rx_hwdesc->dma_status);

	while (budget-- > 0 && (QDPC_DMA_OWNED(dma_status) == 0)) {
		rx_desc = rxq->pd_nextdesc;
		lastdesc = (rxq->pd_nextdesc == rxq->pd_lastdesc);
		rx_hwdesc = rx_desc->dd_hwdesc;

		qdpc_unmap_buffer(priv, rx_desc, PCI_DMA_FROMDEVICE);
		pktlen = qdpc_rx_process_frame(ndev, rx_desc,dma_status, lastdesc);

		/* Check for end of ring, and loop around */
		if (lastdesc) {
			rxq->pd_nextdesc = rxq->pd_firstdesc;
		} else {
			rxq->pd_nextdesc++;
		}

		/*  Update the statistics */
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pktlen;

		rx_hwdesc = rxq->pd_nextdesc->dd_hwdesc;
		dma_status = qdpc_pci_readl(&rx_hwdesc->dma_status);
	}

	qdpc_indicate_peer_rx_nfree(priv, rxq, rx_desc, dma_status);
	qdpc_interrupt_target(priv, QDPC_HOST_TXDONE);

}

SRAM_TEXT uint32_t qdpc_veth_txprocessq(struct net_device *ndev, uint32_t maxpkts)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_dsq;
	qdpc_desc_t *tx_desc;
	uint32_t nprocessed = 0;

	for (nprocessed = 0; nprocessed < maxpkts; nprocessed++) {
		tx_desc = STAILQ_FIRST(&txq->pd_pending);
		if (!tx_desc || !(QDPC_DMA_OWNED(qdpc_pci_readl(&tx_desc->dd_hwdesc->dma_status)) == 0)) {
			break;
		}
		STAILQ_REMOVE_HEAD(&txq->pd_pending, dd_entry);

		qdpc_unmap_buffer(priv, tx_desc, PCI_DMA_TODEVICE);

#if !defined(QTN_PCIE_USE_LOCAL_BUFFER)
		qdpc_tx_skb_recycle((struct sk_buff *)tx_desc->dd_metadata);
		tx_desc->dd_metadata = NULL;
#endif
		tx_desc->dd_qtime = 0;
	}

	txq->pd_nfree += nprocessed;
	txq->pd_npending -= nprocessed;

	return nprocessed;
}

static SRAM_TEXT inline void qdpc_start_txdma(struct qdpc_priv *priv)
{
	qdpc_interrupt_target(priv, QDPC_HOST_TXREADY);
}

static SRAM_TEXT inline void qdpc_start_rxtasklet(struct qdpc_priv *priv)
{
	qdpc_interrupt_target(priv, QDPC_HOST_START_RX);
}

SRAM_DATA static int peer_rx_nfree = QDPC_DESC_RING_SIZE;

static SRAM_TEXT int32_t qdpc_update_peer_nfree(struct qdpc_priv *priv)
{
	int32_t budget;

	budget = qdpc_pci_readl(priv->host_ep2h_txd_budget);
	if (budget < 0) {
		budget = peer_rx_nfree;
	} else {
		peer_rx_nfree = budget;
		qdpc_pci_writel(-1, priv->host_ep2h_txd_budget);
	}

	return budget;
}

/* TX completion routine, Runs as tasklet/softirq priority */
SRAM_TEXT void qdpc_veth_txdone(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_dsq;
	int32_t npending = (int32_t)txq->pd_npending;
	uint32_t high_watermark = txq->pd_ringsize >> 1;

	spin_lock_bh(&txq->pd_lock);

	qdpc_veth_txprocessq(ndev, npending >> 1);
	npending = (int32_t)txq->pd_npending;

	if (npending <= high_watermark && netif_queue_stopped(ndev)) {
		del_timer(&priv->txq_enable_timer);
		netif_wake_queue(ndev);
	} else if (npending > high_watermark) {
		netif_stop_queue(ndev);
		mod_timer(&priv->txq_enable_timer, jiffies + msecs_to_jiffies(QDPC_STOP_QUEUE_TIMER_DELAY));
	}

	spin_unlock_bh(&txq->pd_lock);
}

inline static SRAM_TEXT unsigned long qdpc_align_val_up(unsigned long val, unsigned long step)
{
	return ((val + step - 1) & (~(step - 1)));
}

inline SRAM_TEXT int qdpc_send_packet_invalid (struct qdpc_priv *priv, struct sk_buff *skb)
{
	qdpc_pdring_t *txq = &priv->pktq.pkt_dsq;
	return (!txq->pd_nextdesc || (!skb->len) || (skb->len > QDPC_MAX_MTU));
}

static SRAM_TEXT int qdpc_send_desc_check(struct qdpc_priv *priv,
	qdpc_pdring_t *txq)
{
	int32_t ret = 1;
	int32_t budget;
	unsigned long flags;

	local_irq_save(flags);
	if (txq->pd_nfree > QDPC_VETH_TX_LOW_WATERMARK) {
		budget = qdpc_update_peer_nfree(priv);
		if (budget > (txq->pd_npending + QDPC_VETH_RX_LOW_WATERMARK)) {
			peer_rx_nfree--;
			ret = 0;
		} else {
			qdpc_start_rxtasklet(priv);
		}
	} else {
		qdpc_start_txdma(priv);
	}
	local_irq_restore(flags);

	return ret;
}

SRAM_TEXT int qdpc_send_packet(struct sk_buff *skb, struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_dsq;
	qdpc_dmadesc_t* tx_hwdesc;
	qdpc_desc_t *tx_desc;
	bool lastdesc;
	uint32_t dma_len = 0;
	void *dma_data;

	if (unlikely(qdpc_send_packet_invalid(priv, skb))) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"Xmit packet invalid: len %d\n", skb->len);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	spin_lock_bh(&txq->pd_lock);

	if (qdpc_send_desc_check(priv, txq)) {
		netif_stop_queue(ndev);
		mod_timer(&priv->txq_enable_timer, jiffies + msecs_to_jiffies(QDPC_STOP_QUEUE_TIMER_DELAY));
		spin_unlock_bh(&txq->pd_lock);

		priv->stats.tx_errors++;
		return NETDEV_TX_BUSY;
	}

	if (skb->len <= priv->epmem.eps_minbuf) {
		dma_len = priv->epmem.eps_minbuf;
	} else {
		dma_len = skb->len & QDPC_DMA_LEN_MASK;
	}

	if (unlikely(dma_len >= QDPC_DMA_MAXBUF)) {
		spin_unlock_bh(&txq->pd_lock);
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"Xmit packet too big: len %d dmalen %d\n", skb->len, dma_len);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	if (unlikely(skb_linearize(skb) != 0)) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"WARNING:%u Linearize failed\n", __LINE__);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	tx_desc = txq->pd_nextdesc;
	tx_hwdesc = tx_desc->dd_hwdesc;
	lastdesc = (tx_desc == txq->pd_lastdesc);

	if (lastdesc) {
		txq->pd_nextdesc = txq->pd_firstdesc;
	} else {
		txq->pd_nextdesc++;
	}


#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	/* On X86 copy to local buffer
	  */
	dma_data = tx_desc->dd_metadata;
	memcpy(tx_desc->dd_metadata, skb->data, skb->len);
#else
	dma_data = skb->data;
	/* Hold onto skb. Release when we get a txdone */
	tx_desc->dd_metadata = skb;
#endif

	tx_desc->dd_vaddr = dma_data;
	tx_desc->dd_size = dma_len;
	qdpc_map_buffer(priv, tx_desc, PCI_DMA_TODEVICE);
	qdpc_pci_writel(tx_desc->dd_paddr + priv->epmem.eps_dma_offset,
		&tx_hwdesc->dma_data);
	qdpc_pci_writel((QDPC_DMA_SINGLE_TXBUFFER | QDPC_DMA_TX_NOCRC | dma_len |
		(lastdesc ? QDPC_DMA_LAST_DESC : 0)), &tx_hwdesc->dma_control);
	mmiowb(); /* Memory mapped io barrrier */
	qdpc_pci_writel((QDPC_DMA_OWN), &tx_hwdesc->dma_status);

	/*
	*  Add delay equivalent to enlarge the IFG
	*  It's a workaround for BBIC3 RX Run-out
	*/
#define	RUBY_PCIE_HOST_WR_DELAY	50
	udelay(RUBY_PCIE_HOST_WR_DELAY);

	tx_desc->dd_qtime = jiffies;
	STAILQ_INSERT_TAIL(&txq->pd_pending, tx_desc, dd_entry);
	txq->pd_nfree--;
	txq->pd_npending++;
	qdpc_start_txdma(priv);

	spin_unlock_bh(&txq->pd_lock);

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;

/* Do this for X86 */
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	qdpc_tx_skb_recycle(skb);
#endif

	return NETDEV_TX_OK;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
netdev_tx_t qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev)
#else
int qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev)
#endif
{
	return qdpc_send_packet(skb, ndev);
}

struct net_device_stats *qdpc_veth_stats(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);

	return &priv->stats;
}

int qdpc_veth_change_mtu(struct net_device *ndev, int new_mtu)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	spinlock_t *lock = &priv->lock;
	unsigned long flags;

	/* check ranges */
	if ((new_mtu < QDPC_MIN_MTU) || (new_mtu > QDPC_MAX_MTU))
		return -EINVAL;

	/*
	 * Do anything you need, and the accept the value
	 */
	spin_lock_irqsave(lock, flags);
	ndev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);

	return SUCCESS; /* success */
}

int qdpc_veth_set_mac_addr(struct net_device *ndev, void *paddr)
{
	struct sockaddr *addr = (struct sockaddr *)paddr;

	if (netif_running(ndev))
		return -EBUSY;

	/* Check the validity */
	if (!is_valid_ether_addr((const u8 *)addr->sa_data))
		return -EINVAL;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);

	return SUCCESS;
}

int qdpc_init_netdev(struct net_device  **net_dev, struct pci_dev *pdev)
{
	struct qdpc_priv *priv;
	struct net_device *ndev;
	int ret  = 0;

	/* Allocate the devices */
	ndev = alloc_netdev(sizeof(struct qdpc_priv), "host%d", ether_setup);
	if (!ndev) {
		PRINT_ERROR("Error in allocating the net device \n");
		return -ENOMEM;
	}

	SET_NETDEV_DEV(ndev, &pdev->dev);
	priv = netdev_priv(ndev);
	memset(priv, 0, sizeof(struct qdpc_priv));
	priv->pdev = pdev;
	priv->ndev = ndev;
	pci_set_drvdata(pdev, ndev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)
	ndev->netdev_ops = &veth_ops;
#else
	ndev->open            = qdpc_veth_open;
	ndev->stop            = qdpc_veth_release;
	ndev->hard_start_xmit = qdpc_veth_tx;
	ndev->get_stats       = qdpc_veth_stats;
	ndev->change_mtu      = qdpc_veth_change_mtu;
	ndev->set_mac_address = qdpc_veth_set_mac_addr;
#endif
	ndev->ethtool_ops = &qdpc_veth_ethtool_ops;
	/* Initialize locks */
	spin_lock_init(&priv->lock);
	memcpy(ndev->dev_addr, "\0HOST%", ETH_ALEN);
	*net_dev = ndev;

	return ret;
}
static void qdpc_veth_get_drvinfo(struct net_device *ndev,
				struct ethtool_drvinfo  *info)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	struct pci_dev *pci_dev = (struct pci_dev *) priv->pdev;

	strcpy(info->driver, qdpc_pcie_driver_name);
	strcpy(info->version, QDPC_MODULE_VERSION);
	strcpy(info->fw_version, "N/A");
	strcpy(info->bus_info, pci_name(pci_dev));

	return;
}

static void qdpc_get_ethtool_stats(struct net_device *netdev,
				struct ethtool_stats *stats, u64 *data)
{
	struct qdpc_priv *priv = netdev_priv(netdev);
	int i;

	for (i = 0; i < QDPC_NET_STATS_LEN; i++)	{
		data[i] = ((unsigned long *)&priv->stats)[i];
	}

	return;
}

static void qdpc_veth_get_strings(struct net_device *netdev,
				u32 stringset, u8 *data)
{
	if (ETH_SS_STATS == stringset) {
		memcpy(data, *qdpc_veth_gstrings_stats, sizeof(qdpc_veth_gstrings_stats));
	}

	return;
}

static int qdpc_veth_get_sset_count(struct net_device *netdev, int sset)
{
	switch (sset) {
		case ETH_SS_STATS:
			return QDPC_VETH_STATS_LEN;
		default:
			return -EOPNOTSUPP;
	}
}
