/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/if_vlan.h>
#include <linux/reset.h>
#include <linux/tcp.h>
#include <linux/io.h>
#include <linux/bug.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_flow_table.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>

#include <asm/mach-ralink/ralink_regs.h>

#include "mtk_eth_soc.h"
#include "mdio.h"
#include "ethtool.h"

#define	MAX_RX_LENGTH		1536
#define FE_RX_ETH_HLEN		(VLAN_ETH_HLEN + VLAN_HLEN + ETH_FCS_LEN)
#define FE_RX_HLEN		(NET_SKB_PAD + FE_RX_ETH_HLEN + NET_IP_ALIGN)
#define DMA_DUMMY_DESC		0xffffffff
#define FE_DEFAULT_MSG_ENABLE \
		(NETIF_MSG_DRV | \
		NETIF_MSG_PROBE | \
		NETIF_MSG_LINK | \
		NETIF_MSG_TIMER | \
		NETIF_MSG_IFDOWN | \
		NETIF_MSG_IFUP | \
		NETIF_MSG_RX_ERR | \
		NETIF_MSG_TX_ERR)

#define TX_DMA_DESP2_DEF	(TX_DMA_LS0 | TX_DMA_DONE)
#define TX_DMA_DESP4_DEF	(TX_DMA_QN(3) | TX_DMA_PN(1))
#define NEXT_TX_DESP_IDX(X)	(((X) + 1) & (ring->tx_ring_size - 1))
#define NEXT_RX_DESP_IDX(X)	(((X) + 1) & (ring->rx_ring_size - 1))

#define SYSC_REG_RSTCTRL	0x34

static int fe_msg_level = -1;
module_param_named(msg_level, fe_msg_level, int, 0);
MODULE_PARM_DESC(msg_level, "Message level (-1=defaults,0=none,...,16=all)");

static const u16 fe_reg_table_default[FE_REG_COUNT] = {
	[FE_REG_PDMA_GLO_CFG] = FE_PDMA_GLO_CFG,
	[FE_REG_PDMA_RST_CFG] = FE_PDMA_RST_CFG,
	[FE_REG_DLY_INT_CFG] = FE_DLY_INT_CFG,
	[FE_REG_TX_BASE_PTR0] = FE_TX_BASE_PTR0,
	[FE_REG_TX_MAX_CNT0] = FE_TX_MAX_CNT0,
	[FE_REG_TX_CTX_IDX0] = FE_TX_CTX_IDX0,
	[FE_REG_TX_DTX_IDX0] = FE_TX_DTX_IDX0,
	[FE_REG_RX_BASE_PTR0] = FE_RX_BASE_PTR0,
	[FE_REG_RX_MAX_CNT0] = FE_RX_MAX_CNT0,
	[FE_REG_RX_CALC_IDX0] = FE_RX_CALC_IDX0,
	[FE_REG_RX_DRX_IDX0] = FE_RX_DRX_IDX0,
	[FE_REG_FE_INT_ENABLE] = FE_FE_INT_ENABLE,
	[FE_REG_FE_INT_STATUS] = FE_FE_INT_STATUS,
	[FE_REG_FE_DMA_VID_BASE] = FE_DMA_VID0,
	[FE_REG_FE_COUNTER_BASE] = FE_GDMA1_TX_GBCNT,
	[FE_REG_FE_RST_GL] = FE_FE_RST_GL,
};

static const u16 *fe_reg_table = fe_reg_table_default;

struct fe_work_t {
	int bitnr;
	void (*action)(struct fe_priv *);
};

static void __iomem *fe_base;

void fe_w32(u32 val, unsigned reg)
{
	__raw_writel(val, fe_base + reg);
}

u32 fe_r32(unsigned reg)
{
	return __raw_readl(fe_base + reg);
}

void fe_reg_w32(u32 val, enum fe_reg reg)
{
	fe_w32(val, fe_reg_table[reg]);
}

u32 fe_reg_r32(enum fe_reg reg)
{
	return fe_r32(fe_reg_table[reg]);
}

void fe_m32(struct fe_priv *eth, u32 clear, u32 set, unsigned reg)
{
	u32 val;

	spin_lock(&eth->page_lock);
	val = __raw_readl(fe_base + reg);
	val &= ~clear;
	val |= set;
	__raw_writel(val, fe_base + reg);
	spin_unlock(&eth->page_lock);
}

void fe_reset(u32 reset_bits)
{
	u32 t;

	t = rt_sysc_r32(SYSC_REG_RSTCTRL);
	t |= reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);

	t &= ~reset_bits;
	rt_sysc_w32(t, SYSC_REG_RSTCTRL);
	usleep_range(10, 20);
}

static inline void fe_int_disable(u32 mask)
{
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) & ~mask,
		   FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
}

static inline void fe_int_enable(u32 mask)
{
	fe_reg_w32(fe_reg_r32(FE_REG_FE_INT_ENABLE) | mask,
		   FE_REG_FE_INT_ENABLE);
	/* flush write */
	fe_reg_r32(FE_REG_FE_INT_ENABLE);
}

static inline void fe_hw_set_macaddr(struct fe_priv *priv, unsigned char *mac)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->page_lock, flags);
	fe_w32((mac[0] << 8) | mac[1], FE_GDMA1_MAC_ADRH);
	fe_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
	       FE_GDMA1_MAC_ADRL);
	spin_unlock_irqrestore(&priv->page_lock, flags);
}

static int fe_set_mac_address(struct net_device *dev, void *p)
{
	int ret = eth_mac_addr(dev, p);

	if (!ret) {
		struct fe_priv *priv = netdev_priv(dev);

		if (priv->soc->set_mac)
			priv->soc->set_mac(priv, dev->dev_addr);
		else
			fe_hw_set_macaddr(priv, p);
	}

	return ret;
}

static inline int fe_max_frag_size(int mtu)
{
	/* make sure buf_size will be at least MAX_RX_LENGTH */
	if (mtu + FE_RX_ETH_HLEN < MAX_RX_LENGTH)
		mtu = MAX_RX_LENGTH - FE_RX_ETH_HLEN;

	return SKB_DATA_ALIGN(FE_RX_HLEN + mtu) +
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
}

static inline int fe_max_buf_size(int frag_size)
{
	int buf_size = frag_size - NET_SKB_PAD - NET_IP_ALIGN -
		       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	BUG_ON(buf_size < MAX_RX_LENGTH);
	return buf_size;
}

static inline void fe_get_rxd(struct fe_rx_dma *rxd, struct fe_rx_dma *dma_rxd)
{
	rxd->rxd1 = dma_rxd->rxd1;
	rxd->rxd2 = dma_rxd->rxd2;
	rxd->rxd3 = dma_rxd->rxd3;
	rxd->rxd4 = dma_rxd->rxd4;
}

static inline void fe_set_txd(struct fe_tx_dma *txd, struct fe_tx_dma *dma_txd)
{
	dma_txd->txd1 = txd->txd1;
	dma_txd->txd3 = txd->txd3;
	dma_txd->txd4 = txd->txd4;
	/* clean dma done flag last */
	dma_txd->txd2 = txd->txd2;
}

static void fe_clean_rx(struct fe_priv *priv)
{
	struct fe_rx_ring *ring = &priv->rx_ring;
	struct page *page;
	int i;

	if (ring->rx_data) {
		for (i = 0; i < ring->rx_ring_size; i++)
			if (ring->rx_data[i]) {
				if (ring->rx_dma && ring->rx_dma[i].rxd1)
					dma_unmap_single(&priv->netdev->dev,
							 ring->rx_dma[i].rxd1,
							 ring->rx_buf_size,
							 DMA_FROM_DEVICE);
				skb_free_frag(ring->rx_data[i]);
			}

		kfree(ring->rx_data);
		ring->rx_data = NULL;
	}

	if (ring->rx_dma) {
		dma_free_coherent(&priv->netdev->dev,
				  ring->rx_ring_size * sizeof(*ring->rx_dma),
				  ring->rx_dma,
				  ring->rx_phys);
		ring->rx_dma = NULL;
	}

	if (!ring->frag_cache.va)
	    return;

	page = virt_to_page(ring->frag_cache.va);
	__page_frag_cache_drain(page, ring->frag_cache.pagecnt_bias);
	memset(&ring->frag_cache, 0, sizeof(ring->frag_cache));
}

static int fe_alloc_rx(struct fe_priv *priv)
{
	struct net_device *netdev = priv->netdev;
	struct fe_rx_ring *ring = &priv->rx_ring;
	int i, pad;

	ring->rx_data = kcalloc(ring->rx_ring_size, sizeof(*ring->rx_data),
			GFP_KERNEL);
	if (!ring->rx_data)
		goto no_rx_mem;

	for (i = 0; i < ring->rx_ring_size; i++) {
		ring->rx_data[i] = page_frag_alloc(&ring->frag_cache,
						   ring->frag_size,
						   GFP_KERNEL);
		if (!ring->rx_data[i])
			goto no_rx_mem;
	}

	ring->rx_dma = dma_alloc_coherent(&netdev->dev,
			ring->rx_ring_size * sizeof(*ring->rx_dma),
			&ring->rx_phys,
			GFP_ATOMIC | __GFP_ZERO);
	if (!ring->rx_dma)
		goto no_rx_mem;

	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		pad = 0;
	else
		pad = NET_IP_ALIGN;
	for (i = 0; i < ring->rx_ring_size; i++) {
		dma_addr_t dma_addr = dma_map_single(&netdev->dev,
				ring->rx_data[i] + NET_SKB_PAD + pad,
				ring->rx_buf_size,
				DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(&netdev->dev, dma_addr)))
			goto no_rx_mem;
		ring->rx_dma[i].rxd1 = (unsigned int)dma_addr;

		if (priv->flags & FE_FLAG_RX_SG_DMA)
			ring->rx_dma[i].rxd2 = RX_DMA_PLEN0(ring->rx_buf_size);
		else
			ring->rx_dma[i].rxd2 = RX_DMA_LSO;
	}
	ring->rx_calc_idx = ring->rx_ring_size - 1;
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	fe_reg_w32(ring->rx_phys, FE_REG_RX_BASE_PTR0);
	fe_reg_w32(ring->rx_ring_size, FE_REG_RX_MAX_CNT0);
	fe_reg_w32(ring->rx_calc_idx, FE_REG_RX_CALC_IDX0);
	fe_reg_w32(FE_PST_DRX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;

no_rx_mem:
	return -ENOMEM;
}

static void fe_txd_unmap(struct device *dev, struct fe_tx_buf *tx_buf)
{
	if (dma_unmap_len(tx_buf, dma_len0))
		dma_unmap_page(dev,
			       dma_unmap_addr(tx_buf, dma_addr0),
			       dma_unmap_len(tx_buf, dma_len0),
			       DMA_TO_DEVICE);

	if (dma_unmap_len(tx_buf, dma_len1))
		dma_unmap_page(dev,
			       dma_unmap_addr(tx_buf, dma_addr1),
			       dma_unmap_len(tx_buf, dma_len1),
			       DMA_TO_DEVICE);

	dma_unmap_len_set(tx_buf, dma_addr0, 0);
	dma_unmap_len_set(tx_buf, dma_addr1, 0);
	if (tx_buf->skb && (tx_buf->skb != (struct sk_buff *)DMA_DUMMY_DESC))
		dev_kfree_skb_any(tx_buf->skb);
	tx_buf->skb = NULL;
}

static void fe_clean_tx(struct fe_priv *priv)
{
	int i;
	struct device *dev = &priv->netdev->dev;
	struct fe_tx_ring *ring = &priv->tx_ring;

	if (ring->tx_buf) {
		for (i = 0; i < ring->tx_ring_size; i++)
			fe_txd_unmap(dev, &ring->tx_buf[i]);
		kfree(ring->tx_buf);
		ring->tx_buf = NULL;
	}

	if (ring->tx_dma) {
		dma_free_coherent(dev,
				  ring->tx_ring_size * sizeof(*ring->tx_dma),
				  ring->tx_dma,
				  ring->tx_phys);
		ring->tx_dma = NULL;
	}

	netdev_reset_queue(priv->netdev);
}

static int fe_alloc_tx(struct fe_priv *priv)
{
	int i;
	struct fe_tx_ring *ring = &priv->tx_ring;

	ring->tx_free_idx = 0;
	ring->tx_next_idx = 0;
	ring->tx_thresh = max((unsigned long)ring->tx_ring_size >> 2,
			      MAX_SKB_FRAGS);

	ring->tx_buf = kcalloc(ring->tx_ring_size, sizeof(*ring->tx_buf),
			GFP_KERNEL);
	if (!ring->tx_buf)
		goto no_tx_mem;

	ring->tx_dma = dma_alloc_coherent(&priv->netdev->dev,
			ring->tx_ring_size * sizeof(*ring->tx_dma),
			&ring->tx_phys,
			GFP_ATOMIC | __GFP_ZERO);
	if (!ring->tx_dma)
		goto no_tx_mem;

	for (i = 0; i < ring->tx_ring_size; i++) {
		if (priv->soc->tx_dma)
			priv->soc->tx_dma(&ring->tx_dma[i]);
		ring->tx_dma[i].txd2 = TX_DMA_DESP2_DEF;
	}
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	fe_reg_w32(ring->tx_phys, FE_REG_TX_BASE_PTR0);
	fe_reg_w32(ring->tx_ring_size, FE_REG_TX_MAX_CNT0);
	fe_reg_w32(0, FE_REG_TX_CTX_IDX0);
	fe_reg_w32(FE_PST_DTX_IDX0, FE_REG_PDMA_RST_CFG);

	return 0;

no_tx_mem:
	return -ENOMEM;
}

static int fe_init_dma(struct fe_priv *priv)
{
	int err;

	err = fe_alloc_tx(priv);
	if (err)
		return err;

	err = fe_alloc_rx(priv);
	if (err)
		return err;

	return 0;
}

static void fe_free_dma(struct fe_priv *priv)
{
	fe_clean_tx(priv);
	fe_clean_rx(priv);
}

void fe_stats_update(struct fe_priv *priv)
{
	struct fe_hw_stats *hwstats = priv->hw_stats;
	unsigned int base = fe_reg_table[FE_REG_FE_COUNTER_BASE];
	u64 stats;

	u64_stats_update_begin(&hwstats->syncp);

	if (IS_ENABLED(CONFIG_SOC_MT7621)) {
		hwstats->rx_bytes			+= fe_r32(base);
		stats					=  fe_r32(base + 0x04);
		if (stats)
			hwstats->rx_bytes		+= (stats << 32);
		hwstats->rx_packets			+= fe_r32(base + 0x08);
		hwstats->rx_overflow			+= fe_r32(base + 0x10);
		hwstats->rx_fcs_errors			+= fe_r32(base + 0x14);
		hwstats->rx_short_errors		+= fe_r32(base + 0x18);
		hwstats->rx_long_errors			+= fe_r32(base + 0x1c);
		hwstats->rx_checksum_errors		+= fe_r32(base + 0x20);
		hwstats->rx_flow_control_packets	+= fe_r32(base + 0x24);
		hwstats->tx_skip			+= fe_r32(base + 0x28);
		hwstats->tx_collisions			+= fe_r32(base + 0x2c);
		hwstats->tx_bytes			+= fe_r32(base + 0x30);
		stats					=  fe_r32(base + 0x34);
		if (stats)
			hwstats->tx_bytes		+= (stats << 32);
		hwstats->tx_packets			+= fe_r32(base + 0x38);
	} else {
		hwstats->tx_bytes			+= fe_r32(base);
		hwstats->tx_packets			+= fe_r32(base + 0x04);
		hwstats->tx_skip			+= fe_r32(base + 0x08);
		hwstats->tx_collisions			+= fe_r32(base + 0x0c);
		hwstats->rx_bytes			+= fe_r32(base + 0x20);
		hwstats->rx_packets			+= fe_r32(base + 0x24);
		hwstats->rx_overflow			+= fe_r32(base + 0x28);
		hwstats->rx_fcs_errors			+= fe_r32(base + 0x2c);
		hwstats->rx_short_errors		+= fe_r32(base + 0x30);
		hwstats->rx_long_errors			+= fe_r32(base + 0x34);
		hwstats->rx_checksum_errors		+= fe_r32(base + 0x38);
		hwstats->rx_flow_control_packets	+= fe_r32(base + 0x3c);
	}

	u64_stats_update_end(&hwstats->syncp);
}

static void fe_get_stats64(struct net_device *dev,
				struct rtnl_link_stats64 *storage)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_hw_stats *hwstats = priv->hw_stats;
	unsigned int base = fe_reg_table[FE_REG_FE_COUNTER_BASE];
	unsigned int start;

	if (!base) {
		netdev_stats_to_stats64(storage, &dev->stats);
		return;
	}

	if (netif_running(dev) && netif_device_present(dev)) {
		if (spin_trylock_bh(&hwstats->stats_lock)) {
			fe_stats_update(priv);
			spin_unlock_bh(&hwstats->stats_lock);
		}
	}

	do {
		start = u64_stats_fetch_begin_irq(&hwstats->syncp);
		storage->rx_packets = hwstats->rx_packets;
		storage->tx_packets = hwstats->tx_packets;
		storage->rx_bytes = hwstats->rx_bytes;
		storage->tx_bytes = hwstats->tx_bytes;
		storage->collisions = hwstats->tx_collisions;
		storage->rx_length_errors = hwstats->rx_short_errors +
			hwstats->rx_long_errors;
		storage->rx_over_errors = hwstats->rx_overflow;
		storage->rx_crc_errors = hwstats->rx_fcs_errors;
		storage->rx_errors = hwstats->rx_checksum_errors;
		storage->tx_aborted_errors = hwstats->tx_skip;
	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));

	storage->tx_errors = priv->netdev->stats.tx_errors;
	storage->rx_dropped = priv->netdev->stats.rx_dropped;
	storage->tx_dropped = priv->netdev->stats.tx_dropped;
}

static int fe_vlan_rx_add_vid(struct net_device *dev,
			      __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = (vid & 0xf);
	u32 vlan_cfg;

	if (!((fe_reg_table[FE_REG_FE_DMA_VID_BASE]) &&
	      (dev->features & NETIF_F_HW_VLAN_CTAG_TX)))
		return 0;

	if (test_bit(idx, &priv->vlan_map)) {
		netdev_warn(dev, "disable tx vlan offload\n");
		dev->wanted_features &= ~NETIF_F_HW_VLAN_CTAG_TX;
		netdev_update_features(dev);
	} else {
		vlan_cfg = fe_r32(fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
				((idx >> 1) << 2));
		if (idx & 0x1) {
			vlan_cfg &= 0xffff;
			vlan_cfg |= (vid << 16);
		} else {
			vlan_cfg &= 0xffff0000;
			vlan_cfg |= vid;
		}
		fe_w32(vlan_cfg, fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
				((idx >> 1) << 2));
		set_bit(idx, &priv->vlan_map);
	}

	return 0;
}

static int fe_vlan_rx_kill_vid(struct net_device *dev,
			       __be16 proto, u16 vid)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 idx = (vid & 0xf);

	if (!((fe_reg_table[FE_REG_FE_DMA_VID_BASE]) &&
	      (dev->features & NETIF_F_HW_VLAN_CTAG_TX)))
		return 0;

	clear_bit(idx, &priv->vlan_map);

	return 0;
}

static inline u32 fe_empty_txd(struct fe_tx_ring *ring)
{
	barrier();
	return (u32)(ring->tx_ring_size -
			((ring->tx_next_idx - ring->tx_free_idx) &
			 (ring->tx_ring_size - 1)) - 1);
}

struct fe_map_state {
	struct device *dev;
	struct fe_tx_dma txd;
	u32 def_txd4;
	int ring_idx;
	int i;
};

static void fe_tx_dma_write_desc(struct fe_tx_ring *ring, struct fe_map_state *st)
{
	fe_set_txd(&st->txd, &ring->tx_dma[st->ring_idx]);
	memset(&st->txd, 0, sizeof(st->txd));
	st->txd.txd4 = st->def_txd4;
	st->ring_idx = NEXT_TX_DESP_IDX(st->ring_idx);
}

static int __fe_tx_dma_map_page(struct fe_tx_ring *ring, struct fe_map_state *st,
				struct page *page, size_t offset, size_t size)
{
	struct device *dev = st->dev;
	struct fe_tx_buf *tx_buf;
	dma_addr_t mapped_addr;

	mapped_addr = dma_map_page(dev, page, offset, size, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(dev, mapped_addr)))
		return -EIO;

	if (st->i && !(st->i & 1))
	    fe_tx_dma_write_desc(ring, st);

	tx_buf = &ring->tx_buf[st->ring_idx];
	if (st->i & 1) {
		st->txd.txd3 = mapped_addr;
		st->txd.txd2 |= TX_DMA_PLEN1(size);
		dma_unmap_addr_set(tx_buf, dma_addr1, mapped_addr);
		dma_unmap_len_set(tx_buf, dma_len1, size);
	} else {
		tx_buf->skb = (struct sk_buff *)DMA_DUMMY_DESC;
		st->txd.txd1 = mapped_addr;
		st->txd.txd2 = TX_DMA_PLEN0(size);
		dma_unmap_addr_set(tx_buf, dma_addr0, mapped_addr);
		dma_unmap_len_set(tx_buf, dma_len0, size);
	}
	st->i++;

	return 0;
}

static int fe_tx_dma_map_page(struct fe_tx_ring *ring, struct fe_map_state *st,
			      struct page *page, size_t offset, size_t size)
{
	int cur_size;
	int ret;

	while (size > 0) {
		cur_size = min_t(size_t, size, TX_DMA_BUF_LEN);

		ret = __fe_tx_dma_map_page(ring, st, page, offset, cur_size);
		if (ret)
			return ret;

		size -= cur_size;
		offset += cur_size;
	}

	return 0;
}

static int fe_tx_dma_map_skb(struct fe_tx_ring *ring, struct fe_map_state *st,
			     struct sk_buff *skb)
{
	struct page *page = virt_to_page(skb->data);
	size_t offset = offset_in_page(skb->data);
	size_t size = skb_headlen(skb);

	return fe_tx_dma_map_page(ring, st, page, offset, size);
}

static inline struct sk_buff *
fe_next_frag(struct sk_buff *head, struct sk_buff *skb)
{
	if (skb != head)
		return skb->next;

	if (skb_has_frag_list(skb))
		return skb_shinfo(skb)->frag_list;

	return NULL;
}


static int fe_tx_map_dma(struct sk_buff *skb, struct net_device *dev,
			 int tx_num, struct fe_tx_ring *ring)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_map_state st = {
		.dev = &dev->dev,
		.ring_idx = ring->tx_next_idx,
	};
	struct sk_buff *head = skb;
	struct fe_tx_buf *tx_buf;
	unsigned int nr_frags;
	int i, j;

	/* init tx descriptor */
	if (priv->soc->tx_dma)
		priv->soc->tx_dma(&st.txd);
	else
		st.txd.txd4 = TX_DMA_DESP4_DEF;
	st.def_txd4 = st.txd.txd4;

	/* TX Checksum offload */
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		st.txd.txd4 |= TX_DMA_CHKSUM;

	/* VLAN header offload */
	if (skb_vlan_tag_present(skb)) {
		u16 tag = skb_vlan_tag_get(skb);

		if (IS_ENABLED(CONFIG_SOC_MT7621))
			st.txd.txd4 |= TX_DMA_INS_VLAN_MT7621 | tag;
		else
			st.txd.txd4 |= TX_DMA_INS_VLAN |
				((tag >> VLAN_PRIO_SHIFT) << 4) |
				(tag & 0xF);
	}

	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0)) {
			netif_warn(priv, tx_err, dev,
				   "GSO expand head fail.\n");
			goto err_out;
		}
		if (skb_shinfo(skb)->gso_type &
				(SKB_GSO_TCPV4 | SKB_GSO_TCPV6)) {
			st.txd.txd4 |= TX_DMA_TSO;
			tcp_hdr(skb)->check = htons(skb_shinfo(skb)->gso_size);
		}
	}

next_frag:
	if (skb_headlen(skb) && fe_tx_dma_map_skb(ring, &st, skb))
		goto err_dma;

	/* TX SG offload */
	nr_frags = skb_shinfo(skb)->nr_frags;
	for (i = 0; i < nr_frags; i++) {
		struct skb_frag_struct *frag;

		frag = &skb_shinfo(skb)->frags[i];
		if (fe_tx_dma_map_page(ring, &st, skb_frag_page(frag),
				       frag->page_offset, skb_frag_size(frag)))
			goto err_dma;
	}

	skb = fe_next_frag(head, skb);
	if (skb)
		goto next_frag;

	/* set last segment */
	if (st.i & 0x1)
		st.txd.txd2 |= TX_DMA_LS0;
	else
		st.txd.txd2 |= TX_DMA_LS1;

	/* store skb to cleanup */
	tx_buf = &ring->tx_buf[st.ring_idx];
	tx_buf->skb = head;

	netdev_sent_queue(dev, head->len);
	skb_tx_timestamp(head);

	fe_tx_dma_write_desc(ring, &st);
	ring->tx_next_idx = st.ring_idx;

	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();
	if (unlikely(fe_empty_txd(ring) <= ring->tx_thresh)) {
		netif_stop_queue(dev);
		smp_mb();
		if (unlikely(fe_empty_txd(ring) > ring->tx_thresh))
			netif_wake_queue(dev);
	}

	if (netif_xmit_stopped(netdev_get_tx_queue(dev, 0)) || !head->xmit_more)
		fe_reg_w32(ring->tx_next_idx, FE_REG_TX_CTX_IDX0);

	return 0;

err_dma:
	j = ring->tx_next_idx;
	for (i = 0; i < tx_num; i++) {
		/* unmap dma */
		fe_txd_unmap(&dev->dev, &ring->tx_buf[j]);
		ring->tx_dma[j].txd2 = TX_DMA_DESP2_DEF;

		j = NEXT_TX_DESP_IDX(j);
	}
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

err_out:
	return -1;
}

static inline int fe_skb_padto(struct sk_buff *skb, struct fe_priv *priv)
{
	unsigned int len;
	int ret;

	ret = 0;
	if (unlikely(skb->len < VLAN_ETH_ZLEN)) {
		if ((priv->flags & FE_FLAG_PADDING_64B) &&
		    !(priv->flags & FE_FLAG_PADDING_BUG))
			return ret;

		if (skb_vlan_tag_present(skb))
			len = ETH_ZLEN;
		else if (skb->protocol == cpu_to_be16(ETH_P_8021Q))
			len = VLAN_ETH_ZLEN;
		else if (!(priv->flags & FE_FLAG_PADDING_64B))
			len = ETH_ZLEN;
		else
			return ret;

		if (skb->len < len) {
			ret = skb_pad(skb, len - skb->len);
			if (ret < 0)
				return ret;
			skb->len = len;
			skb_set_tail_pointer(skb, len);
		}
	}

	return ret;
}

static inline int fe_cal_txd_req(struct sk_buff *skb)
{
	struct sk_buff *head = skb;
	int i, nfrags = 0;
	struct skb_frag_struct *frag;

next_frag:
	nfrags++;
	if (skb_is_gso(skb)) {
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			frag = &skb_shinfo(skb)->frags[i];
			nfrags += DIV_ROUND_UP(frag->size, TX_DMA_BUF_LEN);
		}
	} else {
		nfrags += skb_shinfo(skb)->nr_frags;
	}

	skb = fe_next_frag(head, skb);
	if (skb)
		goto next_frag;

	return DIV_ROUND_UP(nfrags, 2);
}

static int fe_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_tx_ring *ring = &priv->tx_ring;
	struct net_device_stats *stats = &dev->stats;
	int tx_num;
	int len = skb->len;

	if (fe_skb_padto(skb, priv)) {
		netif_warn(priv, tx_err, dev, "tx padding failed!\n");
		return NETDEV_TX_OK;
	}

	tx_num = fe_cal_txd_req(skb);
	if (unlikely(fe_empty_txd(ring) <= tx_num)) {
		netif_stop_queue(dev);
		netif_err(priv, tx_queued, dev,
			  "Tx Ring full when queue awake!\n");
		return NETDEV_TX_BUSY;
	}

	if (fe_tx_map_dma(skb, dev, tx_num, ring) < 0) {
		stats->tx_dropped++;
	} else {
		stats->tx_packets++;
		stats->tx_bytes += len;
	}

	return NETDEV_TX_OK;
}

static int fe_poll_rx(struct napi_struct *napi, int budget,
		      struct fe_priv *priv, u32 rx_intr)
{
	struct net_device *netdev = priv->netdev;
	struct net_device_stats *stats = &netdev->stats;
	struct fe_soc_data *soc = priv->soc;
	struct fe_rx_ring *ring = &priv->rx_ring;
	int idx = ring->rx_calc_idx;
	u32 checksum_bit;
	struct sk_buff *skb;
	u8 *data, *new_data;
	struct fe_rx_dma *rxd, trxd;
	int done = 0, pad;

	if (netdev->features & NETIF_F_RXCSUM)
		checksum_bit = soc->checksum_bit;
	else
		checksum_bit = 0;

	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		pad = 0;
	else
		pad = NET_IP_ALIGN;

	while (done < budget) {
		unsigned int pktlen;
		dma_addr_t dma_addr;

		idx = NEXT_RX_DESP_IDX(idx);
		rxd = &ring->rx_dma[idx];
		data = ring->rx_data[idx];

		fe_get_rxd(&trxd, rxd);
		if (!(trxd.rxd2 & RX_DMA_DONE))
			break;

		/* alloc new buffer */
		new_data = page_frag_alloc(&ring->frag_cache, ring->frag_size,
					   GFP_ATOMIC);
		if (unlikely(!new_data)) {
			stats->rx_dropped++;
			goto release_desc;
		}
		dma_addr = dma_map_single(&netdev->dev,
					  new_data + NET_SKB_PAD + pad,
					  ring->rx_buf_size,
					  DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(&netdev->dev, dma_addr))) {
			skb_free_frag(new_data);
			goto release_desc;
		}

		/* receive data */
		skb = build_skb(data, ring->frag_size);
		if (unlikely(!skb)) {
			skb_free_frag(new_data);
			goto release_desc;
		}
		skb_reserve(skb, NET_SKB_PAD + NET_IP_ALIGN);

		dma_unmap_single(&netdev->dev, trxd.rxd1,
				 ring->rx_buf_size, DMA_FROM_DEVICE);
		pktlen = RX_DMA_GET_PLEN0(trxd.rxd2);
		skb->dev = netdev;
		skb_put(skb, pktlen);
		if (trxd.rxd4 & checksum_bit)
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
		skb->protocol = eth_type_trans(skb, netdev);

		if (netdev->features & NETIF_F_HW_VLAN_CTAG_RX &&
		    RX_DMA_VID(trxd.rxd3))
			__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q),
					       RX_DMA_VID(trxd.rxd3));

#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
		if (mtk_offload_check_rx(priv, skb, trxd.rxd4) == 0) {
#endif
			stats->rx_packets++;
			stats->rx_bytes += pktlen;

			napi_gro_receive(napi, skb);
#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
		} else {
			dev_kfree_skb(skb);
		}
#endif
		ring->rx_data[idx] = new_data;
		rxd->rxd1 = (unsigned int)dma_addr;

release_desc:
		if (priv->flags & FE_FLAG_RX_SG_DMA)
			rxd->rxd2 = RX_DMA_PLEN0(ring->rx_buf_size);
		else
			rxd->rxd2 = RX_DMA_LSO;

		ring->rx_calc_idx = idx;
		/* make sure that all changes to the dma ring are flushed before
		 * we continue
		 */
		wmb();
		fe_reg_w32(ring->rx_calc_idx, FE_REG_RX_CALC_IDX0);
		done++;
	}

	if (done < budget)
		fe_reg_w32(rx_intr, FE_REG_FE_INT_STATUS);

	return done;
}

static int fe_poll_tx(struct fe_priv *priv, int budget, u32 tx_intr,
		      int *tx_again)
{
	struct net_device *netdev = priv->netdev;
	struct device *dev = &netdev->dev;
	unsigned int bytes_compl = 0;
	struct sk_buff *skb;
	struct fe_tx_buf *tx_buf;
	int done = 0;
	u32 idx, hwidx;
	struct fe_tx_ring *ring = &priv->tx_ring;

	idx = ring->tx_free_idx;
	hwidx = fe_reg_r32(FE_REG_TX_DTX_IDX0);

	while ((idx != hwidx) && budget) {
		tx_buf = &ring->tx_buf[idx];
		skb = tx_buf->skb;

		if (!skb)
			break;

		if (skb != (struct sk_buff *)DMA_DUMMY_DESC) {
			bytes_compl += skb->len;
			done++;
			budget--;
		}
		fe_txd_unmap(dev, tx_buf);
		idx = NEXT_TX_DESP_IDX(idx);
	}
	ring->tx_free_idx = idx;

	if (idx == hwidx) {
		/* read hw index again make sure no new tx packet */
		hwidx = fe_reg_r32(FE_REG_TX_DTX_IDX0);
		if (idx == hwidx)
			fe_reg_w32(tx_intr, FE_REG_FE_INT_STATUS);
		else
			*tx_again = 1;
	} else {
		*tx_again = 1;
	}

	if (done) {
		netdev_completed_queue(netdev, done, bytes_compl);
		smp_mb();
		if (unlikely(netif_queue_stopped(netdev) &&
			     (fe_empty_txd(ring) > ring->tx_thresh)))
			netif_wake_queue(netdev);
	}

	return done;
}

static int fe_poll(struct napi_struct *napi, int budget)
{
	struct fe_priv *priv = container_of(napi, struct fe_priv, rx_napi);
	struct fe_hw_stats *hwstat = priv->hw_stats;
	int tx_done, rx_done, tx_again;
	u32 status, fe_status, status_reg, mask;
	u32 tx_intr, rx_intr, status_intr;

	status = fe_reg_r32(FE_REG_FE_INT_STATUS);
	fe_status = status;
	tx_intr = priv->soc->tx_int;
	rx_intr = priv->soc->rx_int;
	status_intr = priv->soc->status_int;
	tx_done = 0;
	rx_done = 0;
	tx_again = 0;

	if (fe_reg_table[FE_REG_FE_INT_STATUS2]) {
		fe_status = fe_reg_r32(FE_REG_FE_INT_STATUS2);
		status_reg = FE_REG_FE_INT_STATUS2;
	} else {
		status_reg = FE_REG_FE_INT_STATUS;
	}

	if (status & tx_intr)
		tx_done = fe_poll_tx(priv, budget, tx_intr, &tx_again);

	if (status & rx_intr)
		rx_done = fe_poll_rx(napi, budget, priv, rx_intr);

	if (unlikely(fe_status & status_intr)) {
		if (hwstat && spin_trylock(&hwstat->stats_lock)) {
			fe_stats_update(priv);
			spin_unlock(&hwstat->stats_lock);
		}
		fe_reg_w32(status_intr, status_reg);
	}

	if (unlikely(netif_msg_intr(priv))) {
		mask = fe_reg_r32(FE_REG_FE_INT_ENABLE);
		netdev_info(priv->netdev,
			    "done tx %d, rx %d, intr 0x%08x/0x%x\n",
			    tx_done, rx_done, status, mask);
	}

	if (!tx_again && (rx_done < budget)) {
		status = fe_reg_r32(FE_REG_FE_INT_STATUS);
		if (status & (tx_intr | rx_intr)) {
			/* let napi poll again */
			rx_done = budget;
			goto poll_again;
		}

		napi_complete_done(napi, rx_done);
		fe_int_enable(tx_intr | rx_intr);
	} else {
		rx_done = budget;
	}

poll_again:
	return rx_done;
}

static void fe_tx_timeout(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct fe_tx_ring *ring = &priv->tx_ring;

	priv->netdev->stats.tx_errors++;
	netif_err(priv, tx_err, dev,
		  "transmit timed out\n");
	netif_info(priv, drv, dev, "dma_cfg:%08x\n",
		   fe_reg_r32(FE_REG_PDMA_GLO_CFG));
	netif_info(priv, drv, dev, "tx_ring=%d, "
		   "base=%08x, max=%u, ctx=%u, dtx=%u, fdx=%hu, next=%hu\n",
		   0, fe_reg_r32(FE_REG_TX_BASE_PTR0),
		   fe_reg_r32(FE_REG_TX_MAX_CNT0),
		   fe_reg_r32(FE_REG_TX_CTX_IDX0),
		   fe_reg_r32(FE_REG_TX_DTX_IDX0),
		   ring->tx_free_idx,
		   ring->tx_next_idx);
	netif_info(priv, drv, dev,
		   "rx_ring=%d, base=%08x, max=%u, calc=%u, drx=%u\n",
		   0, fe_reg_r32(FE_REG_RX_BASE_PTR0),
		   fe_reg_r32(FE_REG_RX_MAX_CNT0),
		   fe_reg_r32(FE_REG_RX_CALC_IDX0),
		   fe_reg_r32(FE_REG_RX_DRX_IDX0));

	if (!test_and_set_bit(FE_FLAG_RESET_PENDING, priv->pending_flags))
		schedule_work(&priv->pending_work);
}

static irqreturn_t fe_handle_irq(int irq, void *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 status, int_mask;

	status = fe_reg_r32(FE_REG_FE_INT_STATUS);

	if (unlikely(!status))
		return IRQ_NONE;

	int_mask = (priv->soc->rx_int | priv->soc->tx_int);
	if (likely(status & int_mask)) {
		if (likely(napi_schedule_prep(&priv->rx_napi))) {
			fe_int_disable(int_mask);
			__napi_schedule(&priv->rx_napi);
		}
	} else {
		fe_reg_w32(status, FE_REG_FE_INT_STATUS);
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void fe_poll_controller(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	u32 int_mask = priv->soc->tx_int | priv->soc->rx_int;

	fe_int_disable(int_mask);
	fe_handle_irq(dev->irq, dev);
	fe_int_enable(int_mask);
}
#endif

int fe_set_clock_cycle(struct fe_priv *priv)
{
	unsigned long sysclk = priv->sysclk;

	sysclk /= FE_US_CYC_CNT_DIVISOR;
	sysclk <<= FE_US_CYC_CNT_SHIFT;

	fe_w32((fe_r32(FE_FE_GLO_CFG) &
			~(FE_US_CYC_CNT_MASK << FE_US_CYC_CNT_SHIFT)) |
			sysclk,
			FE_FE_GLO_CFG);
	return 0;
}

void fe_fwd_config(struct fe_priv *priv)
{
	u32 fwd_cfg;

	fwd_cfg = fe_r32(FE_GDMA1_FWD_CFG);

	/* disable jumbo frame */
	if (priv->flags & FE_FLAG_JUMBO_FRAME)
		fwd_cfg &= ~FE_GDM1_JMB_EN;

	/* set unicast/multicast/broadcast frame to cpu */
	fwd_cfg &= ~0xffff;

	fe_w32(fwd_cfg, FE_GDMA1_FWD_CFG);
}

static void fe_rxcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) | (FE_GDM1_ICS_EN |
					FE_GDM1_TCS_EN | FE_GDM1_UCS_EN),
				FE_GDMA1_FWD_CFG);
	else
		fe_w32(fe_r32(FE_GDMA1_FWD_CFG) & ~(FE_GDM1_ICS_EN |
					FE_GDM1_TCS_EN | FE_GDM1_UCS_EN),
				FE_GDMA1_FWD_CFG);
}

static void fe_txcsum_config(bool enable)
{
	if (enable)
		fe_w32(fe_r32(FE_CDMA_CSG_CFG) | (FE_ICS_GEN_EN |
					FE_TCS_GEN_EN | FE_UCS_GEN_EN),
				FE_CDMA_CSG_CFG);
	else
		fe_w32(fe_r32(FE_CDMA_CSG_CFG) & ~(FE_ICS_GEN_EN |
					FE_TCS_GEN_EN | FE_UCS_GEN_EN),
				FE_CDMA_CSG_CFG);
}

void fe_csum_config(struct fe_priv *priv)
{
	struct net_device *dev = priv_netdev(priv);

	fe_txcsum_config((dev->features & NETIF_F_IP_CSUM));
	fe_rxcsum_config((dev->features & NETIF_F_RXCSUM));
}

static int fe_hw_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	int i, err;

	err = devm_request_irq(priv->dev, dev->irq, fe_handle_irq, 0,
			       dev_name(priv->dev), dev);
	if (err)
		return err;

	if (priv->soc->set_mac)
		priv->soc->set_mac(priv, dev->dev_addr);
	else
		fe_hw_set_macaddr(priv, dev->dev_addr);

	/* disable delay interrupt */
	fe_reg_w32(0, FE_REG_DLY_INT_CFG);

	fe_int_disable(priv->soc->tx_int | priv->soc->rx_int);

	/* frame engine will push VLAN tag regarding to VIDX feild in Tx desc */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		for (i = 0; i < 16; i += 2)
			fe_w32(((i + 1) << 16) + i,
			       fe_reg_table[FE_REG_FE_DMA_VID_BASE] +
			       (i * 2));

	if (priv->soc->fwd_config(priv))
		netdev_err(dev, "unable to get clock\n");

	if (fe_reg_table[FE_REG_FE_RST_GL]) {
		fe_reg_w32(1, FE_REG_FE_RST_GL);
		fe_reg_w32(0, FE_REG_FE_RST_GL);
	}

	return 0;
}

static int fe_open(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	u32 val;
	int err;

	err = fe_init_dma(priv);
	if (err) {
		fe_free_dma(priv);
		return err;
	}

	spin_lock_irqsave(&priv->page_lock, flags);

	val = FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN;
	if (priv->flags & FE_FLAG_RX_2B_OFFSET)
		val |= FE_RX_2B_OFFSET;
	val |= priv->soc->pdma_glo_cfg;
	fe_reg_w32(val, FE_REG_PDMA_GLO_CFG);

	spin_unlock_irqrestore(&priv->page_lock, flags);

	if (priv->phy)
		priv->phy->start(priv);

	if (priv->soc->has_carrier && priv->soc->has_carrier(priv))
		netif_carrier_on(dev);

	napi_enable(&priv->rx_napi);
	fe_int_enable(priv->soc->tx_int | priv->soc->rx_int);
	netif_start_queue(dev);
#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
	mtk_ppe_probe(priv);
#endif

	return 0;
}

static int fe_stop(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	netif_tx_disable(dev);
	fe_int_disable(priv->soc->tx_int | priv->soc->rx_int);
	napi_disable(&priv->rx_napi);

	if (priv->phy)
		priv->phy->stop(priv);

	spin_lock_irqsave(&priv->page_lock, flags);

	fe_reg_w32(fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
		     ~(FE_TX_WB_DDONE | FE_RX_DMA_EN | FE_TX_DMA_EN),
		     FE_REG_PDMA_GLO_CFG);
	spin_unlock_irqrestore(&priv->page_lock, flags);

	/* wait dma stop */
	for (i = 0; i < 10; i++) {
		if (fe_reg_r32(FE_REG_PDMA_GLO_CFG) &
				(FE_TX_DMA_BUSY | FE_RX_DMA_BUSY)) {
			msleep(20);
			continue;
		}
		break;
	}

	fe_free_dma(priv);

#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
	mtk_ppe_remove(priv);
#endif

	return 0;
}

static void fe_reset_phy(struct fe_priv *priv)
{
	int err, msec = 30;
	struct gpio_desc *phy_reset;

	phy_reset = devm_gpiod_get_optional(priv->dev, "phy-reset",
					    GPIOD_OUT_HIGH);
	if (!phy_reset)
		return;

	if (IS_ERR(phy_reset)) {
		dev_err(priv->dev, "Error acquiring reset gpio pins: %ld\n",
			PTR_ERR(phy_reset));
		return;
	}

	err = of_property_read_u32(priv->dev->of_node, "phy-reset-duration",
				   &msec);
	if (!err && msec > 1000)
		msec = 30;

	if (msec > 20)
		msleep(msec);
	else
		usleep_range(msec * 1000, msec * 1000 + 1000);

	gpiod_set_value(phy_reset, 0);
}

static int __init fe_init(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);
	struct device_node *port;
	const char *mac_addr;
	int err;

	priv->soc->reset_fe();

	if (priv->soc->switch_init)
		if (priv->soc->switch_init(priv)) {
			netdev_err(dev, "failed to initialize switch core\n");
			return -ENODEV;
		}

	fe_reset_phy(priv);

	mac_addr = of_get_mac_address(priv->dev->of_node);
	if (mac_addr)
		ether_addr_copy(dev->dev_addr, mac_addr);

	/* If the mac address is invalid, use random mac address  */
	if (!is_valid_ether_addr(dev->dev_addr)) {
		eth_hw_addr_random(dev);
		dev_err(priv->dev, "generated random MAC address %pM\n",
			dev->dev_addr);
	}

	err = fe_mdio_init(priv);
	if (err)
		return err;

	if (priv->soc->port_init)
		for_each_child_of_node(priv->dev->of_node, port)
			if (of_device_is_compatible(port, "mediatek,eth-port") &&
			    of_device_is_available(port))
				priv->soc->port_init(priv, port);

	if (priv->phy) {
		err = priv->phy->connect(priv);
		if (err)
			goto err_phy_disconnect;
	}

	err = fe_hw_init(dev);
	if (err)
		goto err_phy_disconnect;

	if ((priv->flags & FE_FLAG_HAS_SWITCH) && priv->soc->switch_config)
		priv->soc->switch_config(priv);

	return 0;

err_phy_disconnect:
	if (priv->phy)
		priv->phy->disconnect(priv);
	fe_mdio_cleanup(priv);

	return err;
}

static void fe_uninit(struct net_device *dev)
{
	struct fe_priv *priv = netdev_priv(dev);

	if (priv->phy)
		priv->phy->disconnect(priv);
	fe_mdio_cleanup(priv);

	fe_reg_w32(0, FE_REG_FE_INT_ENABLE);
	free_irq(dev->irq, dev);
}

static int fe_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct fe_priv *priv = netdev_priv(dev);

	if (!priv->phy_dev)
		return -ENODEV;


	return phy_mii_ioctl(priv->phy_dev, ifr, cmd);
}

static int fe_change_mtu(struct net_device *dev, int new_mtu)
{
	struct fe_priv *priv = netdev_priv(dev);
	int frag_size, old_mtu;
	u32 fwd_cfg;

	old_mtu = dev->mtu;
	dev->mtu = new_mtu;

	if (!(priv->flags & FE_FLAG_JUMBO_FRAME))
		return 0;

	/* return early if the buffer sizes will not change */
	if (old_mtu <= ETH_DATA_LEN && new_mtu <= ETH_DATA_LEN)
		return 0;
	if (old_mtu > ETH_DATA_LEN && new_mtu > ETH_DATA_LEN)
		return 0;

	if (new_mtu <= ETH_DATA_LEN)
		priv->rx_ring.frag_size = fe_max_frag_size(ETH_DATA_LEN);
	else
		priv->rx_ring.frag_size = PAGE_SIZE;
	priv->rx_ring.rx_buf_size = fe_max_buf_size(priv->rx_ring.frag_size);

	if (!netif_running(dev))
		return 0;

	fe_stop(dev);
	if (!IS_ENABLED(CONFIG_SOC_MT7621)) {
		fwd_cfg = fe_r32(FE_GDMA1_FWD_CFG);
		if (new_mtu <= ETH_DATA_LEN) {
			fwd_cfg &= ~FE_GDM1_JMB_EN;
		} else {
			frag_size = fe_max_frag_size(new_mtu);
			fwd_cfg &= ~(FE_GDM1_JMB_LEN_MASK << FE_GDM1_JMB_LEN_SHIFT);
			fwd_cfg |= (DIV_ROUND_UP(frag_size, 1024) <<
			FE_GDM1_JMB_LEN_SHIFT) | FE_GDM1_JMB_EN;
		}
		fe_w32(fwd_cfg, FE_GDMA1_FWD_CFG);
	}

	return fe_open(dev);
}

#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
static int
fe_flow_offload(enum flow_offload_type type, struct flow_offload *flow,
		struct flow_offload_hw_path *src,
		struct flow_offload_hw_path *dest)
{
	struct fe_priv *priv;

	if (src->dev != dest->dev)
		return -EINVAL;

	priv = netdev_priv(src->dev);

	return mtk_flow_offload(priv, type, flow, src, dest);
}
#endif

static const struct net_device_ops fe_netdev_ops = {
	.ndo_init		= fe_init,
	.ndo_uninit		= fe_uninit,
	.ndo_open		= fe_open,
	.ndo_stop		= fe_stop,
	.ndo_start_xmit		= fe_start_xmit,
	.ndo_set_mac_address	= fe_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= fe_do_ioctl,
	.ndo_change_mtu		= fe_change_mtu,
	.ndo_tx_timeout		= fe_tx_timeout,
	.ndo_get_stats64        = fe_get_stats64,
	.ndo_vlan_rx_add_vid	= fe_vlan_rx_add_vid,
	.ndo_vlan_rx_kill_vid	= fe_vlan_rx_kill_vid,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= fe_poll_controller,
#endif
#ifdef CONFIG_NET_MEDIATEK_OFFLOAD
	.ndo_flow_offload	= fe_flow_offload,
#endif
};

static void fe_reset_pending(struct fe_priv *priv)
{
	struct net_device *dev = priv->netdev;
	int err;

	rtnl_lock();
	fe_stop(dev);

	err = fe_open(dev);
	if (err) {
		netif_alert(priv, ifup, dev,
			    "Driver up/down cycle failed, closing device.\n");
		dev_close(dev);
	}
	rtnl_unlock();
}

static const struct fe_work_t fe_work[] = {
	{FE_FLAG_RESET_PENDING, fe_reset_pending},
};

static void fe_pending_work(struct work_struct *work)
{
	struct fe_priv *priv = container_of(work, struct fe_priv, pending_work);
	int i;
	bool pending;

	for (i = 0; i < ARRAY_SIZE(fe_work); i++) {
		pending = test_and_clear_bit(fe_work[i].bitnr,
					     priv->pending_flags);
		if (pending)
			fe_work[i].action(priv);
	}
}

static int fe_probe(struct platform_device *pdev)
{
	struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	const struct of_device_id *match;
	struct fe_soc_data *soc;
	struct net_device *netdev;
	struct fe_priv *priv;
	struct clk *sysclk;
	int err, napi_weight;

	device_reset(&pdev->dev);

	match = of_match_device(of_fe_match, &pdev->dev);
	soc = (struct fe_soc_data *)match->data;

	if (soc->reg_table)
		fe_reg_table = soc->reg_table;
	else
		soc->reg_table = fe_reg_table;

	fe_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(fe_base)) {
		err = -EADDRNOTAVAIL;
		goto err_out;
	}

	netdev = alloc_etherdev(sizeof(*priv));
	if (!netdev) {
		dev_err(&pdev->dev, "alloc_etherdev failed\n");
		err = -ENOMEM;
		goto err_iounmap;
	}

	SET_NETDEV_DEV(netdev, &pdev->dev);
	netdev->netdev_ops = &fe_netdev_ops;
	netdev->base_addr = (unsigned long)fe_base;

	netdev->irq = platform_get_irq(pdev, 0);
	if (netdev->irq < 0) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		err = -ENXIO;
		goto err_free_dev;
	}

	if (soc->init_data)
		soc->init_data(soc, netdev);
	netdev->vlan_features = netdev->hw_features &
				~(NETIF_F_HW_VLAN_CTAG_TX |
				  NETIF_F_HW_VLAN_CTAG_RX);
	netdev->features |= netdev->hw_features;

	if (IS_ENABLED(CONFIG_SOC_MT7621))
		netdev->max_mtu = 2048;

	/* fake rx vlan filter func. to support tx vlan offload func */
	if (fe_reg_table[FE_REG_FE_DMA_VID_BASE])
		netdev->features |= NETIF_F_HW_VLAN_CTAG_FILTER;

	priv = netdev_priv(netdev);
	spin_lock_init(&priv->page_lock);
	if (fe_reg_table[FE_REG_FE_COUNTER_BASE]) {
		priv->hw_stats = kzalloc(sizeof(*priv->hw_stats), GFP_KERNEL);
		if (!priv->hw_stats) {
			err = -ENOMEM;
			goto err_free_dev;
		}
		spin_lock_init(&priv->hw_stats->stats_lock);
	}

	sysclk = devm_clk_get(&pdev->dev, NULL);
	if (!IS_ERR(sysclk)) {
		priv->sysclk = clk_get_rate(sysclk);
	} else if ((priv->flags & FE_FLAG_CALIBRATE_CLK)) {
		dev_err(&pdev->dev, "this soc needs a clk for calibration\n");
		err = -ENXIO;
		goto err_free_dev;
	}

	priv->switch_np = of_parse_phandle(pdev->dev.of_node, "mediatek,switch", 0);
	if ((priv->flags & FE_FLAG_HAS_SWITCH) && !priv->switch_np) {
		dev_err(&pdev->dev, "failed to read switch phandle\n");
		err = -ENODEV;
		goto err_free_dev;
	}

	priv->netdev = netdev;
	priv->dev = &pdev->dev;
	priv->soc = soc;
	priv->msg_enable = netif_msg_init(fe_msg_level, FE_DEFAULT_MSG_ENABLE);
	priv->rx_ring.frag_size = fe_max_frag_size(ETH_DATA_LEN);
	priv->rx_ring.rx_buf_size = fe_max_buf_size(priv->rx_ring.frag_size);
	priv->tx_ring.tx_ring_size = NUM_DMA_DESC;
	priv->rx_ring.rx_ring_size = NUM_DMA_DESC;
	INIT_WORK(&priv->pending_work, fe_pending_work);
	u64_stats_init(&priv->hw_stats->syncp);

	napi_weight = 16;
	if (priv->flags & FE_FLAG_NAPI_WEIGHT) {
		napi_weight *= 4;
		priv->tx_ring.tx_ring_size *= 4;
		priv->rx_ring.rx_ring_size *= 4;
	}
	netif_napi_add(netdev, &priv->rx_napi, fe_poll, napi_weight);
	fe_set_ethtool_ops(netdev);

	err = register_netdev(netdev);
	if (err) {
		dev_err(&pdev->dev, "error bringing up device\n");
		goto err_free_dev;
	}

	platform_set_drvdata(pdev, netdev);

	netif_info(priv, probe, netdev, "mediatek frame engine at 0x%08lx, irq %d\n",
		   netdev->base_addr, netdev->irq);

	return 0;

err_free_dev:
	free_netdev(netdev);
err_iounmap:
	devm_iounmap(&pdev->dev, fe_base);
err_out:
	return err;
}

static int fe_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct fe_priv *priv = netdev_priv(dev);

	netif_napi_del(&priv->rx_napi);
	kfree(priv->hw_stats);

	cancel_work_sync(&priv->pending_work);

	unregister_netdev(dev);
	free_netdev(dev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver fe_driver = {
	.probe = fe_probe,
	.remove = fe_remove,
	.driver = {
		.name = "mtk_soc_eth",
		.owner = THIS_MODULE,
		.of_match_table = of_fe_match,
	},
};

module_platform_driver(fe_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Ethernet driver for Ralink SoC");
MODULE_VERSION(MTK_FE_DRV_VERSION);
