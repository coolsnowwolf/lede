/*
 *  Ethernet device driver for Gemini SoC (SL351x GMAC).
 *
 *  Copyright (C) 2011, Tobias Waldvogel <tobias.waldvogel@gmail.com>
 *
 *  Based on work by Michał Mirosław <mirq-linux@rere.qmqm.pl> and
 *  Paulius Zaleckas <paulius.zaleckas@gmail.com> and
 *  Giuseppe De Robertis <Giuseppe.DeRobertis@ba.infn.it> and
 *  GPLd spaghetti code from Raidsonic and other Gemini-based NAS vendors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/cache.h>
#include <linux/interrupt.h>

#include <linux/platform_device.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/skbuff.h>
#include <linux/phy.h>
#include <linux/crc32.h>
#include <linux/ethtool.h>
#include <linux/tcp.h>
#include <linux/u64_stats_sync.h>

#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>

#include <mach/hardware.h>
#include <mach/global_reg.h>

#include <mach/gmac.h>
#include "sl351x_hw.h"

#define DRV_NAME		"gmac-gemini"
#define DRV_VERSION		"1.0"

#define HSIZE_8			0b00
#define HSIZE_16		0b01
#define HSIZE_32		0b10

#define HBURST_SINGLE		0b00
#define HBURST_INCR		0b01
#define HBURST_INCR4		0b10
#define HBURST_INCR8		0b11

#define HPROT_DATA_CACHE	BIT(0)
#define HPROT_PRIVILIGED	BIT(1)
#define HPROT_BUFFERABLE	BIT(2)
#define HPROT_CACHABLE		BIT(3)

#define DEFAULT_RX_COALESCE_NSECS	0
#define DEFAULT_GMAC_RXQ_ORDER		9
#define DEFAULT_GMAC_TXQ_ORDER		8
#define DEFAULT_RX_BUF_ORDER		11
#define DEFAULT_NAPI_WEIGHT		64
#define TX_MAX_FRAGS			16
#define TX_QUEUE_NUM			1	/* max: 6 */
#define RX_MAX_ALLOC_ORDER		2

#define GMAC0_IRQ0_2 (GMAC0_TXDERR_INT_BIT|GMAC0_TXPERR_INT_BIT| \
	GMAC0_RXDERR_INT_BIT|GMAC0_RXPERR_INT_BIT)
#define GMAC0_IRQ0_TXQ0_INTS (GMAC0_SWTQ00_EOF_INT_BIT| \
			      GMAC0_SWTQ00_FIN_INT_BIT)
#define GMAC0_IRQ4_8 (GMAC0_MIB_INT_BIT|GMAC0_RX_OVERRUN_INT_BIT)

#define GMAC_OFFLOAD_FEATURES (NETIF_F_SG | NETIF_F_IP_CSUM | \
		NETIF_F_IPV6_CSUM | NETIF_F_RXCSUM | \
		NETIF_F_TSO | NETIF_F_TSO_ECN | NETIF_F_TSO6)

MODULE_AUTHOR("Tobias Waldvogel");
MODULE_DESCRIPTION("StorLink SL351x (Gemini) ethernet driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

struct toe_private {
	void __iomem	*iomem;
	spinlock_t	irq_lock;

	struct net_device *netdev[2];
	__le32		mac_addr[2][3];

	struct device	*dev;
	int		irq;

	unsigned int	freeq_order;
	unsigned int	freeq_frag_order;
	GMAC_RXDESC_T	*freeq_ring;
	dma_addr_t	freeq_dma_base;
	struct page	**freeq_page_tab;
	spinlock_t	freeq_lock;
};

struct gmac_txq {
	GMAC_TXDESC_T	*ring;
	struct sk_buff	**skb;
	unsigned int	cptr;
	unsigned int	noirq_packets;
};

struct gmac_private {
	unsigned int		num;
	struct toe_private	*toe;
	void __iomem		*ctl_iomem;
	void __iomem		*dma_iomem;

	void __iomem		*rxq_rwptr;
	GMAC_RXDESC_T		*rxq_ring;
	unsigned int		rxq_order;

	struct napi_struct	napi;
	struct hrtimer		rx_coalesce_timer;
	unsigned int 		rx_coalesce_nsecs; 
	unsigned int		freeq_refill;
	struct gmac_txq		txq[TX_QUEUE_NUM];
	unsigned int		txq_order;
	unsigned int		irq_every_tx_packets;

	dma_addr_t		rxq_dma_base;
	dma_addr_t		txq_dma_base;

	unsigned int		msg_enable;
	spinlock_t		config_lock;

	struct u64_stats_sync	tx_stats_syncp;
	struct u64_stats_sync	rx_stats_syncp;
	struct u64_stats_sync	ir_stats_syncp;

	struct rtnl_link_stats64 stats;
	u64			hw_stats[RX_STATS_NUM];
	u64			rx_stats[RX_STATUS_NUM];
	u64			rx_csum_stats[RX_CHKSUM_NUM];
	u64			rx_napi_exits;
	u64			tx_frag_stats[TX_MAX_FRAGS];
	u64			tx_frags_linearized;
	u64			tx_hw_csummed;
};

#define GMAC_STATS_NUM	( \
	RX_STATS_NUM + RX_STATUS_NUM + RX_CHKSUM_NUM + 1 + \
	TX_MAX_FRAGS + 2)

static const char gmac_stats_strings[GMAC_STATS_NUM][ETH_GSTRING_LEN] = {
	"GMAC_IN_DISCARDS",
	"GMAC_IN_ERRORS",
	"GMAC_IN_MCAST",
	"GMAC_IN_BCAST",
	"GMAC_IN_MAC1",
	"GMAC_IN_MAC2",
	"RX_STATUS_GOOD_FRAME",
	"RX_STATUS_TOO_LONG_GOOD_CRC",
	"RX_STATUS_RUNT_FRAME",
	"RX_STATUS_SFD_NOT_FOUND",
	"RX_STATUS_CRC_ERROR",
	"RX_STATUS_TOO_LONG_BAD_CRC",
	"RX_STATUS_ALIGNMENT_ERROR",
	"RX_STATUS_TOO_LONG_BAD_ALIGN",
	"RX_STATUS_RX_ERR",
	"RX_STATUS_DA_FILTERED",
	"RX_STATUS_BUFFER_FULL",
	"RX_STATUS_11",
	"RX_STATUS_12",
	"RX_STATUS_13",
	"RX_STATUS_14",
	"RX_STATUS_15",
	"RX_CHKSUM_IP_UDP_TCP_OK",
	"RX_CHKSUM_IP_OK_ONLY",
	"RX_CHKSUM_NONE",
	"RX_CHKSUM_3",
	"RX_CHKSUM_IP_ERR_UNKNOWN",
	"RX_CHKSUM_IP_ERR",
	"RX_CHKSUM_TCP_UDP_ERR",
	"RX_CHKSUM_7",
	"RX_NAPI_EXITS",
	"TX_FRAGS[1]",
	"TX_FRAGS[2]",
	"TX_FRAGS[3]",
	"TX_FRAGS[4]",
	"TX_FRAGS[5]",
	"TX_FRAGS[6]",
	"TX_FRAGS[7]",
	"TX_FRAGS[8]",
	"TX_FRAGS[9]",
	"TX_FRAGS[10]",
	"TX_FRAGS[11]",
	"TX_FRAGS[12]",
	"TX_FRAGS[13]",
	"TX_FRAGS[14]",
	"TX_FRAGS[15]",
	"TX_FRAGS[16+]",
	"TX_FRAGS_LINEARIZED",
	"TX_HW_CSUMMED",
};

static void gmac_dump_dma_state(struct net_device *dev);

static void gmac_update_config0_reg(struct net_device *dev, u32 val, u32 vmask)
{
	struct gmac_private *gmac = netdev_priv(dev);
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&gmac->config_lock, flags);

	reg = readl(gmac->ctl_iomem + GMAC_CONFIG0);
	reg = (reg & ~vmask) | val;
	writel(reg, gmac->ctl_iomem + GMAC_CONFIG0);

	spin_unlock_irqrestore(&gmac->config_lock, flags);
}

static void gmac_enable_tx_rx(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	void __iomem *config0 = gmac->ctl_iomem + GMAC_CONFIG0;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&gmac->config_lock, flags);

	reg = readl(config0);
	reg &= ~CONFIG0_TX_RX_DISABLE;
	writel(reg, config0);

	spin_unlock_irqrestore(&gmac->config_lock, flags);
}

static void gmac_disable_tx_rx(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	void __iomem *config0 = gmac->ctl_iomem + GMAC_CONFIG0;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&gmac->config_lock, flags);

	reg = readl(config0);
	reg |= CONFIG0_TX_RX_DISABLE;
	writel(reg, config0);

	spin_unlock_irqrestore(&gmac->config_lock, flags);

	mdelay(10);	/* let GMAC consume packet */
}

static void gmac_set_flow_control(struct net_device *dev, bool tx, bool rx)
{
	struct gmac_private *gmac = netdev_priv(dev);
	void __iomem *config0 = gmac->ctl_iomem + GMAC_CONFIG0;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&gmac->config_lock, flags);

	reg = readl(config0);
	reg &= ~CONFIG0_FLOW_CTL;
	if (tx)
		reg |= CONFIG0_FLOW_TX;
	if (rx)
		reg |= CONFIG0_FLOW_RX;
	writel(reg, config0);

	spin_unlock_irqrestore(&gmac->config_lock, flags);
}

static void gmac_update_link_state(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	void __iomem *status_reg = gmac->ctl_iomem + GMAC_STATUS;
	struct phy_device *phydev = dev->phydev;
	GMAC_STATUS_T status, old_status;
	int pause_tx=0, pause_rx=0;

	old_status.bits32 = status.bits32 = readl(status_reg);

	status.bits.link = phydev->link;
	status.bits.duplex = phydev->duplex;

	switch (phydev->speed) {
	case 1000:
		status.bits.speed = GMAC_SPEED_1000;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_1000;
		break;
	case 100:
		status.bits.speed = GMAC_SPEED_100;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	case 10:
		status.bits.speed = GMAC_SPEED_10;
		if (phydev->interface == PHY_INTERFACE_MODE_RGMII)
			status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	default:
		netdev_warn(dev, "Not supported PHY speed (%d)\n",
			phydev->speed);
	}

	if (phydev->duplex == DUPLEX_FULL) {
		u16 lcladv = phy_read(phydev, MII_ADVERTISE);
		u16 rmtadv = phy_read(phydev, MII_LPA);
		u8 cap = mii_resolve_flowctrl_fdx(lcladv, rmtadv);

		if (cap & FLOW_CTRL_RX)
			pause_rx=1;
		if (cap & FLOW_CTRL_TX)
			pause_tx=1;
	} 

	gmac_set_flow_control(dev, pause_tx, pause_rx);

	if (old_status.bits32 == status.bits32)
		return;

	if (netif_msg_link(gmac)) {
		phy_print_status(phydev);
		netdev_info(dev, "link flow control: %s\n",
			phydev->pause
				? (phydev->asym_pause ? "tx" : "both")
				: (phydev->asym_pause ? "rx" : "none")
		);
	}

	gmac_disable_tx_rx(dev);
	writel(status.bits32, status_reg);
	gmac_enable_tx_rx(dev);
}

static int gmac_setup_phy(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	struct gemini_gmac_platform_data *pdata = toe->dev->platform_data;
	GMAC_STATUS_T status = { .bits32 = 0 };
	int num = dev->dev_id;

	dev->phydev = phy_connect(dev, pdata->bus_id[num],
		&gmac_update_link_state, pdata->interface[num]);

	if (IS_ERR(dev->phydev)) {
		int err = PTR_ERR(dev->phydev);
		dev->phydev = NULL;
		return err;
	}

	dev->phydev->supported &= PHY_GBIT_FEATURES;
	dev->phydev->supported |= SUPPORTED_Asym_Pause | SUPPORTED_Pause;
	dev->phydev->advertising = dev->phydev->supported;

	/* set PHY interface type */
	switch (dev->phydev->interface) {
	case PHY_INTERFACE_MODE_MII:
		status.bits.mii_rmii = GMAC_PHY_MII;
		break;
	case PHY_INTERFACE_MODE_GMII:
		status.bits.mii_rmii = GMAC_PHY_GMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		status.bits.mii_rmii = GMAC_PHY_RGMII_100_10;
		break;
	default:
		netdev_err(dev, "Unsupported MII interface\n");
		phy_disconnect(dev->phydev);
		dev->phydev = NULL;
		return -EINVAL;
	}
	writel(status.bits32, gmac->ctl_iomem + GMAC_STATUS);

	return 0;
}

static int gmac_pick_rx_max_len(int max_l3_len)
{
	/* index = CONFIG_MAXLEN_XXX values */
	static const int max_len[8] = {
		1536, 1518, 1522, 1542,
		9212, 10236, 1518, 1518
	};
	int i, n = 5;

	max_l3_len += ETH_HLEN + VLAN_HLEN;

	if (max_l3_len > max_len[n])
		return -1;

	for (i = 0; i < 5; ++i) {
		if (max_len[i] >= max_l3_len && max_len[i] < max_len[n])
			n = i;
	}

	return n;
}

static int gmac_init(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	u32 val;

	GMAC_CONFIG0_T config0 = { .bits = {
		.dis_tx = 1,
		.dis_rx = 1,
		.ipv4_rx_chksum = 1,
		.ipv6_rx_chksum = 1,
		.rx_err_detect = 1,
		.rgmm_edge = 1,
		.port0_chk_hwq = 1,
		.port1_chk_hwq = 1,
		.port0_chk_toeq = 1,
		.port1_chk_toeq = 1,
		.port0_chk_classq = 1,
		.port1_chk_classq = 1,
	} };
	GMAC_AHB_WEIGHT_T ahb_weight = { .bits = {
		.rx_weight = 1,
		.tx_weight = 1,
		.hash_weight = 1,
		.pre_req = 0x1f,
		.tqDV_threshold = 0,
	} };
	GMAC_TX_WCR0_T hw_weigh = { .bits = {
		.hw_tq3 = 1,
		.hw_tq2 = 1,
		.hw_tq1 = 1,
		.hw_tq0 = 1,
	} };
	GMAC_TX_WCR1_T sw_weigh = { .bits = {
		.sw_tq5 = 1,
		.sw_tq4 = 1,
		.sw_tq3 = 1,
		.sw_tq2 = 1,
		.sw_tq1 = 1,
		.sw_tq0 = 1,
	} };
	GMAC_CONFIG1_T config1 = { .bits = {
		.set_threshold = 16,
		.rel_threshold = 24,
	} };
	GMAC_CONFIG2_T config2 = { .bits = {
		.set_threshold = 16,
		.rel_threshold = 32,
	} };
	GMAC_CONFIG3_T config3 = { .bits = {
		.set_threshold = 0,
		.rel_threshold = 0,
	} };

	config0.bits.max_len = gmac_pick_rx_max_len(dev->mtu);

	val = readl(gmac->ctl_iomem + GMAC_CONFIG0);
	config0.bits.reserved = ((GMAC_CONFIG0_T)val).bits.reserved;
	writel(config0.bits32, gmac->ctl_iomem + GMAC_CONFIG0);
	writel(config1.bits32, gmac->ctl_iomem + GMAC_CONFIG1);
	writel(config2.bits32, gmac->ctl_iomem + GMAC_CONFIG2);
	writel(config3.bits32, gmac->ctl_iomem + GMAC_CONFIG3);

	val = readl(gmac->dma_iomem + GMAC_AHB_WEIGHT_REG);
	writel(ahb_weight.bits32, gmac->dma_iomem + GMAC_AHB_WEIGHT_REG);

	writel(hw_weigh.bits32,
		gmac->dma_iomem + GMAC_TX_WEIGHTING_CTRL_0_REG);
	writel(sw_weigh.bits32,
		gmac->dma_iomem + GMAC_TX_WEIGHTING_CTRL_1_REG);

	gmac->rxq_order = DEFAULT_GMAC_RXQ_ORDER;
	gmac->txq_order = DEFAULT_GMAC_TXQ_ORDER;
	gmac->rx_coalesce_nsecs = DEFAULT_RX_COALESCE_NSECS;

	/* Mark every quarter of the queue a packet for interrupt
	   in order to be able to wake up the queue if it was stopped */
	gmac->irq_every_tx_packets = 1 << (gmac->txq_order - 2);

	return 0;
}

static void gmac_uninit(struct net_device *dev)
{
	if (dev->phydev)
		phy_disconnect(dev->phydev);
}

static int gmac_setup_txqs(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	void __iomem *rwptr_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE0_PTR_REG;
	void __iomem *base_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE_BASE_REG;

	unsigned int n_txq = dev->num_tx_queues;
	size_t entries = 1 <<gmac->txq_order;
	size_t len = n_txq * entries;
	struct gmac_txq *txq = gmac->txq;
	GMAC_TXDESC_T *desc_ring;
	struct sk_buff **skb_tab;
	unsigned int r;
	int i;

	skb_tab = kzalloc(len * sizeof(*skb_tab), GFP_KERNEL);
	if (!skb_tab)
		return -ENOMEM;

	desc_ring = dma_alloc_coherent(toe->dev, len * sizeof(*desc_ring),
		&gmac->txq_dma_base, GFP_KERNEL);

	if (!desc_ring) {
		kfree(skb_tab);
		return -ENOMEM;
	}

	BUG_ON(gmac->txq_dma_base & ~DMA_Q_BASE_MASK);

	writel(gmac->txq_dma_base | gmac->txq_order, base_reg);

	for (i = 0; i < n_txq; i++) {
		txq->ring = desc_ring;
		txq->skb = skb_tab;
		txq->noirq_packets = 0;

		r = readw(rwptr_reg);
		rwptr_reg += 2;
		writew(r, rwptr_reg);
		rwptr_reg +=2;
		txq->cptr = r;
		
		txq++;
		desc_ring += entries;
		skb_tab += entries;
	}

	return 0;
}

static void gmac_clean_txq(struct net_device *dev, struct gmac_txq *txq,
			   unsigned int r)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned int errs = 0;
	unsigned int pkts = 0;
	unsigned int hwchksum = 0;
	unsigned long bytes = 0;
	unsigned int m = (1 << gmac->txq_order) - 1;
	unsigned int c = txq->cptr;
	GMAC_TXDESC_0_T word0;
	GMAC_TXDESC_1_T word1;
	unsigned int word3;
	dma_addr_t mapping;
	GMAC_TXDESC_T *txd;
	unsigned short nfrags;

	if (unlikely(c == r))
		return;

	rmb();
	while (c != r) {
		txd = txq->ring + c;
		word0 = txd->word0;
		word1 = txd->word1;
		mapping = txd->word2.buf_adr;
		word3 = txd->word3.bits32;

		dma_unmap_single(toe->dev, mapping, word0.bits.buffer_size, DMA_TO_DEVICE);

		if (word3 & EOF_BIT)
			dev_kfree_skb(txq->skb[c]);

		c++;
		c &= m;

		if (!(word3 & SOF_BIT))
			continue;
		
		if (!word0.bits.status_tx_ok) {
			errs++;
			continue;
		}

		pkts++;
		bytes += txd->word1.bits.byte_count;

		if (word1.bits32 & TSS_CHECKUM_ENABLE)
			hwchksum++;

		nfrags = word0.bits.desc_count - 1;
		if (nfrags) {
			if (nfrags >= TX_MAX_FRAGS)
				nfrags = TX_MAX_FRAGS - 1;

			u64_stats_update_begin(&gmac->tx_stats_syncp);
			gmac->tx_frag_stats[nfrags]++;
			u64_stats_update_end(&gmac->ir_stats_syncp);
		}
	}

	u64_stats_update_begin(&gmac->ir_stats_syncp);
	gmac->stats.tx_errors += errs;
	gmac->stats.tx_packets += pkts;
	gmac->stats.tx_bytes += bytes;
	gmac->tx_hw_csummed += hwchksum;
	u64_stats_update_end(&gmac->ir_stats_syncp);

	txq->cptr = c;
}

static void gmac_cleanup_txqs(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	void __iomem *rwptr_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE0_PTR_REG;
	void __iomem *base_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE_BASE_REG;

	unsigned n_txq = dev->num_tx_queues;
	unsigned int r, i;

	for (i = 0; i < n_txq; i++) {
		r = readw(rwptr_reg);
		rwptr_reg += 2;
		writew(r, rwptr_reg);
		rwptr_reg += 2;

		gmac_clean_txq(dev, gmac->txq + i, r);
	}
	writel(0, base_reg);

	kfree(gmac->txq->skb);
	dma_free_coherent(toe->dev,
		n_txq * sizeof(*gmac->txq->ring) << gmac->txq_order,
		gmac->txq->ring, gmac->txq_dma_base);
}

static int gmac_setup_rxq(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	NONTOE_QHDR_T __iomem *qhdr = toe->iomem + TOE_DEFAULT_Q_HDR_BASE(dev->dev_id);

	gmac->rxq_rwptr = &qhdr->word1;
	gmac->rxq_ring = dma_alloc_coherent(toe->dev,
		sizeof(*gmac->rxq_ring) << gmac->rxq_order,
		&gmac->rxq_dma_base, GFP_KERNEL);
	if (!gmac->rxq_ring)
		return -ENOMEM;

	BUG_ON(gmac->rxq_dma_base & ~NONTOE_QHDR0_BASE_MASK);
	
	writel(gmac->rxq_dma_base | gmac->rxq_order, &qhdr->word0);
	writel(0, gmac->rxq_rwptr);
	return 0;
}

static void gmac_cleanup_rxq(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;

	NONTOE_QHDR_T __iomem *qhdr = toe->iomem + TOE_DEFAULT_Q_HDR_BASE(dev->dev_id);
	void __iomem *dma_reg = &qhdr->word0;
	void __iomem *ptr_reg = &qhdr->word1;
	GMAC_RXDESC_T *rxd = gmac->rxq_ring;
	DMA_RWPTR_T rw;
	unsigned int r, w;
	unsigned int m = (1 <<gmac->rxq_order) - 1;
	struct page *page;
	dma_addr_t mapping;

	rw.bits32 = readl(ptr_reg);
	r = rw.bits.rptr;
	w = rw.bits.wptr;
	writew(r, ptr_reg + 2);

	writel(0, dma_reg);

	rmb();
	while (r != w) {
		mapping = rxd[r].word2.buf_adr;
		r++;
		r &= m;

		if (!mapping)
			continue;

		page = pfn_to_page(dma_to_pfn(toe->dev, mapping));
		put_page(page);
	}

	dma_free_coherent(toe->dev, sizeof(*gmac->rxq_ring) << gmac->rxq_order,
		gmac->rxq_ring, gmac->rxq_dma_base);
}

static struct page *toe_freeq_alloc_map_page(struct toe_private *toe, int pn)
{
	unsigned int fpp_order = PAGE_SHIFT - toe->freeq_frag_order;
	unsigned int frag_len = 1 << toe->freeq_frag_order;
	GMAC_RXDESC_T *freeq_entry;
	dma_addr_t mapping;
	struct page *page;
	int i;

	page = alloc_page(__GFP_COLD | GFP_ATOMIC);
	if (!page)
		return NULL;

	mapping = dma_map_single(toe->dev, page_address(page),
				PAGE_SIZE, DMA_FROM_DEVICE);

	if (unlikely(dma_mapping_error(toe->dev, mapping) || !mapping)) {
		put_page(page);
		return NULL;
	}

	freeq_entry = toe->freeq_ring + (pn << fpp_order);
	for (i = 1 << fpp_order; i > 0; --i) {
		freeq_entry->word2.buf_adr = mapping;
		freeq_entry++;
		mapping += frag_len;
	}

	if (toe->freeq_page_tab[pn]) {
		mapping = toe->freeq_ring[pn << fpp_order].word2.buf_adr;
		dma_unmap_single(toe->dev, mapping, frag_len, DMA_FROM_DEVICE);
		put_page(toe->freeq_page_tab[pn]);
	}

	toe->freeq_page_tab[pn] = page;
	return page;
}

static unsigned int toe_fill_freeq(struct toe_private *toe, int reset)
{
	void __iomem *rwptr_reg = toe->iomem + GLOBAL_SWFQ_RWPTR_REG;

	DMA_RWPTR_T rw;
	unsigned int pn, epn;
	unsigned int fpp_order = PAGE_SHIFT - toe->freeq_frag_order;
	unsigned int m_pn = (1 << (toe->freeq_order - fpp_order)) - 1;
	struct page *page;
	unsigned int count = 0;
	unsigned long flags;

	spin_lock_irqsave(&toe->freeq_lock, flags);

	rw.bits32 = readl(rwptr_reg);
	pn = (reset ? rw.bits.rptr : rw.bits.wptr) >> fpp_order;
	epn = (rw.bits.rptr >> fpp_order) - 1;
	epn &= m_pn;

	while (pn != epn) {
		page = toe->freeq_page_tab[pn];

		if (atomic_read(&page->_count) > 1) {
			unsigned int fl = (pn -epn) & m_pn;

			if (fl > 64 >> fpp_order)
				break;

			page = toe_freeq_alloc_map_page(toe, pn);
			if (!page)
				break;
		}

		atomic_add(1 << fpp_order, &page->_count);
		count += 1 << fpp_order;
		pn++;
		pn &= m_pn;
	}

	wmb();
	writew(pn << fpp_order, rwptr_reg+2);

	spin_unlock_irqrestore(&toe->freeq_lock, flags);
	return count;
}

static int toe_setup_freeq(struct toe_private *toe)
{
	void __iomem *dma_reg = toe->iomem + GLOBAL_SW_FREEQ_BASE_SIZE_REG;
	QUEUE_THRESHOLD_T qt;
	DMA_SKB_SIZE_T skbsz;
	unsigned int filled;
	unsigned int frag_len = 1 << toe->freeq_frag_order;
	unsigned int len = 1 << toe->freeq_order;
	unsigned int fpp_order = PAGE_SHIFT - toe->freeq_frag_order;
	unsigned int pages = len >> fpp_order;
	dma_addr_t mapping;
	unsigned int pn;

	toe->freeq_ring = dma_alloc_coherent(toe->dev,
		sizeof(*toe->freeq_ring) << toe->freeq_order,
		&toe->freeq_dma_base, GFP_KERNEL);
	if (!toe->freeq_ring)
		return -ENOMEM;

	BUG_ON(toe->freeq_dma_base & ~DMA_Q_BASE_MASK);

	toe->freeq_page_tab = kzalloc(pages * sizeof(*toe->freeq_page_tab),
							GFP_KERNEL);
	if (!toe->freeq_page_tab)
		goto err_freeq;

	for (pn = 0; pn < pages; pn++)
		if (!toe_freeq_alloc_map_page(toe, pn))
			goto err_freeq_alloc;

	filled = toe_fill_freeq(toe, 1);
	if (!filled)
		goto err_freeq_alloc;

	qt.bits32 = readl(toe->iomem + GLOBAL_QUEUE_THRESHOLD_REG);
	qt.bits.swfq_empty = 32;
	writel(qt.bits32, toe->iomem + GLOBAL_QUEUE_THRESHOLD_REG);

	skbsz.bits.sw_skb_size = 1 << toe->freeq_frag_order;
	writel(skbsz.bits32, toe->iomem + GLOBAL_DMA_SKB_SIZE_REG);
	writel(toe->freeq_dma_base | toe->freeq_order, dma_reg);

	return 0;

err_freeq_alloc:
	while (pn > 0) {
		--pn;
		mapping = toe->freeq_ring[pn << fpp_order].word2.buf_adr;
		dma_unmap_single(toe->dev, mapping, frag_len, DMA_FROM_DEVICE);
		put_page(toe->freeq_page_tab[pn]);
	}

err_freeq:
	dma_free_coherent(toe->dev,
		sizeof(*toe->freeq_ring) << toe->freeq_order,
		toe->freeq_ring, toe->freeq_dma_base);
	toe->freeq_ring = NULL;
	return -ENOMEM;
}

static void toe_cleanup_freeq(struct toe_private *toe)
{
	void __iomem *dma_reg = toe->iomem + GLOBAL_SW_FREEQ_BASE_SIZE_REG;
	void __iomem *ptr_reg = toe->iomem + GLOBAL_SWFQ_RWPTR_REG;

	unsigned int frag_len = 1 << toe->freeq_frag_order;
	unsigned int len = 1 << toe->freeq_order;
	unsigned int fpp_order = PAGE_SHIFT - toe->freeq_frag_order;
	unsigned int pages = len >> fpp_order;
	struct page *page;
	dma_addr_t mapping;
	unsigned int pn;

	writew(readw(ptr_reg), ptr_reg + 2);
	writel(0, dma_reg);

	for (pn = 0; pn < pages; pn++) {
		mapping = toe->freeq_ring[pn << fpp_order].word2.buf_adr;
		dma_unmap_single(toe->dev, mapping, frag_len, DMA_FROM_DEVICE);

		page = toe->freeq_page_tab[pn];
		while (atomic_read(&page->_count) > 0)
			put_page(page);
	}

	kfree(toe->freeq_page_tab);

	dma_free_coherent(toe->dev,
		sizeof(*toe->freeq_ring) << toe->freeq_order,
		toe->freeq_ring, toe->freeq_dma_base);
}

static int toe_resize_freeq(struct toe_private *toe, int changing_dev_id)
{
	void __iomem *irqen_reg = toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG;
	struct gmac_private *gmac;
	struct net_device *other = toe->netdev[1 - changing_dev_id];
	unsigned new_size = 0;
	unsigned new_order;
	int err;
	unsigned long flags;
	unsigned en;

	if (other && netif_running(other))
		return -EBUSY;

	if (toe->netdev[0]) {
		gmac = netdev_priv(toe->netdev[0]);
		new_size  = 1 << (gmac->rxq_order + 1);
	}

	if (toe->netdev[1]) {
		gmac = netdev_priv(toe->netdev[1]);
		new_size  += 1 << (gmac->rxq_order + 1);
	}

	new_order = min(15, ilog2(new_size - 1) + 1);
	if (toe->freeq_order == new_order)
		return 0;

	spin_lock_irqsave(&toe->irq_lock, flags);
	en = readl(irqen_reg);
	en &= ~SWFQ_EMPTY_INT_BIT;
	writel(en, irqen_reg);

	if (toe->freeq_ring)
		toe_cleanup_freeq(toe);

	toe->freeq_order = new_order;
	err = toe_setup_freeq(toe);

	en |= SWFQ_EMPTY_INT_BIT;
	writel(en, irqen_reg);
	spin_unlock_irqrestore(&toe->irq_lock, flags);

	return err;
}

static void gmac_tx_irq_enable(struct net_device *dev, unsigned txq, int en)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned val, mask;

	mask = GMAC0_IRQ0_TXQ0_INTS << (6 * dev->dev_id + txq);

	if (en)
		writel(mask, toe->iomem + GLOBAL_INTERRUPT_STATUS_0_REG);

	val = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);
	val = en ? val | mask : val & ~mask;
	writel(val, toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);
}


static void gmac_tx_irq(struct net_device *dev, unsigned txq_num)
{
	struct netdev_queue *ntxq = netdev_get_tx_queue(dev, txq_num);

	gmac_tx_irq_enable(dev, txq_num, 0);
	netif_tx_wake_queue(ntxq);
}

static int gmac_map_tx_bufs(struct net_device *dev, struct sk_buff *skb,
			    struct gmac_txq *txq, unsigned short *desc)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	struct skb_shared_info *skb_si =  skb_shinfo(skb);
	skb_frag_t *skb_frag;
	short frag, last_frag = skb_si->nr_frags - 1;
	unsigned short m = (1 << gmac->txq_order) -1;
	unsigned short w = *desc;
	unsigned word1, word3, buflen;
	dma_addr_t mapping;
	void *buffer;
	unsigned short mtu;
	GMAC_TXDESC_T *txd;

	mtu  = ETH_HLEN;
	mtu += dev->mtu;
	if (skb->protocol == htons(ETH_P_8021Q))
		mtu += VLAN_HLEN;

	word1 = skb->len;
	word3 = SOF_BIT;
		
	if (word1 > mtu) {
		word1 |= TSS_MTU_ENABLE_BIT;
		word3 += mtu;
	}

	if (skb->ip_summed != CHECKSUM_NONE) {
		int tcp = 0;
		if (skb->protocol == htons(ETH_P_IP)) {
			word1 |= TSS_IP_CHKSUM_BIT;
			tcp = ip_hdr(skb)->protocol == IPPROTO_TCP;
		} else { /* IPv6 */
			word1 |= TSS_IPV6_ENABLE_BIT;
			tcp = ipv6_hdr(skb)->nexthdr == IPPROTO_TCP;
		}

		word1 |= tcp ? TSS_TCP_CHKSUM_BIT : TSS_UDP_CHKSUM_BIT;
	}

	frag = -1;
	while (frag <= last_frag) {
		if (frag == -1) {
			buffer = skb->data;
			buflen = skb_headlen(skb);
		} else {
			skb_frag = skb_si->frags + frag;
			buffer = page_address(skb_frag_page(skb_frag)) +
				 skb_frag->page_offset;
			buflen = skb_frag->size;
		}

		if (frag == last_frag) {
			word3 |= EOF_BIT;
			txq->skb[w] = skb;
		}

		mapping = dma_map_single(toe->dev, buffer, buflen,
					DMA_TO_DEVICE);
		if (dma_mapping_error(toe->dev, mapping) ||
			!(mapping & PAGE_MASK))
			goto map_error;

		txd = txq->ring + w;
		txd->word0.bits32 = buflen;
		txd->word1.bits32 = word1;
		txd->word2.buf_adr = mapping;
		txd->word3.bits32 = word3;

		word3 &= MTU_SIZE_BIT_MASK;
		w++;
		w &= m;
		frag++;
	}

	*desc = w;
	return 0;

map_error:
	while (w != *desc) {
		w--;
		w &= m;

		dma_unmap_page(toe->dev, txq->ring[w].word2.buf_adr,
			txq->ring[w].word0.bits.buffer_size, DMA_TO_DEVICE);
	}
	return ENOMEM;
}

static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);

	void __iomem *ptr_reg;
	struct gmac_txq *txq;
	struct netdev_queue *ntxq;
	int txq_num, nfrags;
	DMA_RWPTR_T rw;
	unsigned short r, w, d;
	unsigned short m = (1 << gmac->txq_order) - 1;

	SKB_FRAG_ASSERT(skb);

	if (unlikely(skb->len >= 0x10000))
		goto out_drop_free;

		txq_num = skb_get_queue_mapping(skb);
	ptr_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE_PTR_REG(txq_num);
	txq = &gmac->txq[txq_num];
	ntxq = netdev_get_tx_queue(dev, txq_num);
	nfrags = skb_shinfo(skb)->nr_frags;

	rw.bits32 = readl(ptr_reg);
	r = rw.bits.rptr;
	w = rw.bits.wptr;

	d = txq->cptr - w - 1;
	d &= m;

	if (unlikely(d < nfrags+2))
	{
		gmac_clean_txq(dev, txq, r);
		d = txq->cptr - w - 1;
		d &= m;

		if (unlikely(d < nfrags+2)) {
			netif_tx_stop_queue(ntxq);

			d = txq->cptr + nfrags + 16;
			d &= m;
			txq->ring[d].word3.bits.eofie = 1;
			gmac_tx_irq_enable(dev, txq_num, 1);

			u64_stats_update_begin(&gmac->tx_stats_syncp);
			dev->stats.tx_fifo_errors++;
			u64_stats_update_end(&gmac->tx_stats_syncp);
			return NETDEV_TX_BUSY;
		}
	}

	if (unlikely(gmac_map_tx_bufs(dev, skb, txq, &w))) {
		if (skb_linearize(skb))
			goto out_drop;

		if (unlikely(gmac_map_tx_bufs(dev, skb, txq, &w)))
			goto out_drop_free;

		u64_stats_update_begin(&gmac->tx_stats_syncp);
		gmac->tx_frags_linearized++;
		u64_stats_update_end(&gmac->tx_stats_syncp);
	}

	writew(w, ptr_reg+2);

	gmac_clean_txq(dev, txq, r);
	return NETDEV_TX_OK;

out_drop_free:
	dev_kfree_skb(skb);
out_drop:
	u64_stats_update_begin(&gmac->tx_stats_syncp);
	gmac->stats.tx_dropped++;
	u64_stats_update_end(&gmac->tx_stats_syncp);
	return NETDEV_TX_OK;
}

static void gmac_tx_timeout(struct net_device *dev)
{
	netdev_err(dev, "Tx timeout\n");
	gmac_dump_dma_state(dev);
}

static void gmac_enable_irq(struct net_device *dev, int enable)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned long flags;
	unsigned val, mask;

	spin_lock_irqsave(&toe->irq_lock, flags);

	mask = GMAC0_IRQ0_2 << (dev->dev_id * 2);
	val = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);
	val = enable ? (val | mask) : (val & ~mask);
	writel(val, toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);

	mask = DEFAULT_Q0_INT_BIT << dev->dev_id;
	val = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);
	val = enable ? (val | mask) : (val & ~mask);
	writel(val, toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);

	mask = GMAC0_IRQ4_8 << (dev->dev_id * 8);
	val = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG);
	val = enable ? (val | mask) : (val & ~mask);
	writel(val, toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG);

	spin_unlock_irqrestore(&toe->irq_lock, flags);
}

static void gmac_enable_rx_irq(struct net_device *dev, int enable)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned long flags;
	unsigned val, mask;

	spin_lock_irqsave(&toe->irq_lock, flags);
	mask = DEFAULT_Q0_INT_BIT << dev->dev_id;

	val = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);
	val = enable ? (val | mask) : (val & ~mask);
	writel(val, toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);

	spin_unlock_irqrestore(&toe->irq_lock, flags);
}

static struct sk_buff *gmac_skb_if_good_frame(struct gmac_private *gmac,
	GMAC_RXDESC_0_T word0, unsigned frame_len)
{
	struct sk_buff *skb = NULL;
	unsigned rx_status = word0.bits.status;
	unsigned rx_csum = word0.bits.chksum_status;

	gmac->rx_stats[rx_status]++;
	gmac->rx_csum_stats[rx_csum]++;

	if (word0.bits.derr || word0.bits.perr ||
	    rx_status || frame_len < ETH_ZLEN ||
	    rx_csum >= RX_CHKSUM_IP_ERR_UNKNOWN) {
		gmac->stats.rx_errors++;

		if (frame_len < ETH_ZLEN || RX_ERROR_LENGTH(rx_status))
			gmac->stats.rx_length_errors++;
		if (RX_ERROR_OVER(rx_status))
			gmac->stats.rx_over_errors++;
		if (RX_ERROR_CRC(rx_status))
			gmac->stats.rx_crc_errors++;
		if (RX_ERROR_FRAME(rx_status))
			gmac->stats.rx_frame_errors++;

		return NULL;
	}

	skb = napi_get_frags(&gmac->napi);
	if (!skb)
		return NULL;

	if (rx_csum == RX_CHKSUM_IP_UDP_TCP_OK)
		skb->ip_summed = CHECKSUM_UNNECESSARY;

	gmac->stats.rx_bytes += frame_len;
	gmac->stats.rx_packets++;
	return skb;
}

static unsigned gmac_rx(struct net_device *dev, unsigned budget)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	void __iomem *ptr_reg = gmac->rxq_rwptr;

	static struct sk_buff *skb;

	DMA_RWPTR_T rw;
	unsigned short r, w;
	unsigned short m = (1 << gmac->rxq_order) -1;
	GMAC_RXDESC_T *rx = NULL;
	struct page* page = NULL;
	unsigned page_offs;
	unsigned int frame_len, frag_len;
	int frag_nr = 0;
	
	GMAC_RXDESC_0_T word0;
	GMAC_RXDESC_1_T word1;
	dma_addr_t mapping;
	GMAC_RXDESC_3_T word3;

	rw.bits32 = readl(ptr_reg);
	/* Reset interrupt as all packages until here are taken into account */
	writel(DEFAULT_Q0_INT_BIT << dev->dev_id,
		toe->iomem + GLOBAL_INTERRUPT_STATUS_1_REG);
	r = rw.bits.rptr;
	w = rw.bits.wptr;
	
	while (budget && w != r) {
		rx = gmac->rxq_ring + r;
		word0 = rx->word0;
		word1 = rx->word1;
		mapping = rx->word2.buf_adr;
		word3 = rx->word3;
		
		r++;
		r &= m;

		frag_len = word0.bits.buffer_size;
		frame_len =word1.bits.byte_count;
		page_offs = mapping & ~PAGE_MASK;

		if (unlikely(!mapping)) {
			netdev_err(dev, "rxq[%u]: HW BUG: zero DMA desc\n", r);
			goto err_drop;
		}

		page = pfn_to_page(dma_to_pfn(toe->dev, mapping));

		if (word3.bits32 & SOF_BIT) {
			if (unlikely(skb)) {
				napi_free_frags(&gmac->napi);
				gmac->stats.rx_dropped++;
			}

			skb = gmac_skb_if_good_frame(gmac, word0, frame_len);
			if (unlikely(!skb))
				goto err_drop;

			page_offs += NET_IP_ALIGN;
			frag_len -= NET_IP_ALIGN;
			frag_nr = 0;

		} else if (!skb) {
			put_page(page);
			continue;
		}

		if (word3.bits32 & EOF_BIT)
			frag_len = frame_len - skb->len;

		/* append page frag to skb */
		if (unlikely(frag_nr == MAX_SKB_FRAGS))
			goto err_drop;

		if (frag_len == 0)
			netdev_err(dev, "Received fragment with len = 0\n");

		skb_fill_page_desc(skb, frag_nr, page, page_offs, frag_len);
		skb->len += frag_len;
		skb->data_len += frag_len;
		skb->truesize += frag_len;
		frag_nr++;

		if (word3.bits32 & EOF_BIT) {
			napi_gro_frags(&gmac->napi);
			skb = NULL;
			--budget;
		}
		continue;
		
err_drop:
		if (skb) {
			napi_free_frags(&gmac->napi);
			skb = NULL;
		}

		if (mapping)
			put_page(page);

		gmac->stats.rx_dropped++;
	}

	writew(r, ptr_reg);
	return budget;
}

static int gmac_napi_poll(struct napi_struct *napi, int budget)
{
	struct gmac_private *gmac = netdev_priv(napi->dev);
	struct toe_private *toe = gmac->toe;
	unsigned rx;
	unsigned freeq_threshold = 1 << (toe->freeq_order - 1);

	u64_stats_update_begin(&gmac->rx_stats_syncp);
	
	rx = budget - gmac_rx(napi->dev, budget);

	if (rx == 0) {
		napi_gro_flush(napi, false);
		__napi_complete(napi);
		gmac_enable_rx_irq(napi->dev, 1);
		++gmac->rx_napi_exits;
	}

	gmac->freeq_refill += rx;
	if (gmac->freeq_refill > freeq_threshold) {
		gmac->freeq_refill -= freeq_threshold;
		toe_fill_freeq(toe, 0);
	}

	u64_stats_update_end(&gmac->rx_stats_syncp);
	return budget;
}

static void gmac_dump_dma_state(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	void __iomem *ptr_reg;
	unsigned reg[5];

	/* Interrupt status */
	reg[0] = readl(toe->iomem + GLOBAL_INTERRUPT_STATUS_0_REG);
	reg[1] = readl(toe->iomem + GLOBAL_INTERRUPT_STATUS_1_REG);
	reg[2] = readl(toe->iomem + GLOBAL_INTERRUPT_STATUS_2_REG);
	reg[3] = readl(toe->iomem + GLOBAL_INTERRUPT_STATUS_3_REG);
	reg[4] = readl(toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG);
	netdev_err(dev, "IRQ status: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		reg[0], reg[1], reg[2], reg[3], reg[4]);

	/* Interrupt enable */
	reg[0] = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);
	reg[1] = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);
	reg[2] = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_2_REG);
	reg[3] = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_3_REG);
	reg[4] = readl(toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG);
	netdev_err(dev, "IRQ enable: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		reg[0], reg[1], reg[2], reg[3], reg[4]);

	/* RX DMA status */
	reg[0] = readl(gmac->dma_iomem + GMAC_DMA_RX_FIRST_DESC_REG);
	reg[1] = readl(gmac->dma_iomem + GMAC_DMA_RX_CURR_DESC_REG);
	reg[2] = GET_RPTR(gmac->rxq_rwptr);
	reg[3] = GET_WPTR(gmac->rxq_rwptr);
	netdev_err(dev, "RX DMA regs: 0x%08x 0x%08x, ptr: %u %u\n",
		reg[0], reg[1], reg[2], reg[3]);

	reg[0] = readl(gmac->dma_iomem + GMAC_DMA_RX_DESC_WORD0_REG);
	reg[1] = readl(gmac->dma_iomem + GMAC_DMA_RX_DESC_WORD1_REG);
	reg[2] = readl(gmac->dma_iomem + GMAC_DMA_RX_DESC_WORD2_REG);
	reg[3] = readl(gmac->dma_iomem + GMAC_DMA_RX_DESC_WORD3_REG);
	netdev_err(dev, "RX DMA descriptor: 0x%08x 0x%08x 0x%08x 0x%08x\n",
		reg[0], reg[1], reg[2], reg[3]);

	/* TX DMA status */
	ptr_reg = gmac->dma_iomem + GMAC_SW_TX_QUEUE0_PTR_REG;

	reg[0] = readl(gmac->dma_iomem + GMAC_DMA_TX_FIRST_DESC_REG);
	reg[1] = readl(gmac->dma_iomem + GMAC_DMA_TX_CURR_DESC_REG);
	reg[2] = GET_RPTR(ptr_reg);
	reg[3] = GET_WPTR(ptr_reg);
	netdev_err(dev, "TX DMA regs: 0x%08x 0x%08x, ptr: %u %u\n",
		reg[0], reg[1], reg[2], reg[3]);

	reg[0] = readl(gmac->dma_iomem + GMAC_DMA_TX_DESC_WORD0_REG);
	reg[1] = readl(gmac->dma_iomem + GMAC_DMA_TX_DESC_WORD1_REG);
	reg[2] = readl(gmac->dma_iomem + GMAC_DMA_TX_DESC_WORD2_REG);
	reg[3] = readl(gmac->dma_iomem + GMAC_DMA_TX_DESC_WORD3_REG);
	netdev_err(dev, "TX DMA descriptor: 0x%08x 0x%08x 0x%08x 0x%08x\n",
		reg[0], reg[1], reg[2], reg[3]);

	/* FREE queues status */
	ptr_reg = toe->iomem + GLOBAL_SWFQ_RWPTR_REG;

	reg[0] = GET_RPTR(ptr_reg);
	reg[1] = GET_WPTR(ptr_reg);

	ptr_reg = toe->iomem + GLOBAL_HWFQ_RWPTR_REG;

	reg[2] = GET_RPTR(ptr_reg);
	reg[3] = GET_WPTR(ptr_reg);
	netdev_err(dev, "FQ SW ptr: %u %u, HW ptr: %u %u\n",
		reg[0], reg[1], reg[2], reg[3]);
}

static void gmac_update_hw_stats(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned long flags;
	unsigned int rx_discards, rx_mcast, rx_bcast;

	spin_lock_irqsave(&toe->irq_lock, flags);
	u64_stats_update_begin(&gmac->ir_stats_syncp);

	gmac->hw_stats[0] += rx_discards = readl(gmac->ctl_iomem + GMAC_IN_DISCARDS);
	gmac->hw_stats[1] += readl(gmac->ctl_iomem + GMAC_IN_ERRORS);
	gmac->hw_stats[2] += rx_mcast = readl(gmac->ctl_iomem + GMAC_IN_MCAST);
	gmac->hw_stats[3] += rx_bcast = readl(gmac->ctl_iomem + GMAC_IN_BCAST);
	gmac->hw_stats[4] += readl(gmac->ctl_iomem + GMAC_IN_MAC1);
	gmac->hw_stats[5] += readl(gmac->ctl_iomem + GMAC_IN_MAC2);

	gmac->stats.rx_missed_errors += rx_discards;
	gmac->stats.multicast += rx_mcast;
	gmac->stats.multicast += rx_bcast;

	writel(GMAC0_MIB_INT_BIT << (dev->dev_id * 8),
		toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG);

	u64_stats_update_end(&gmac->ir_stats_syncp);
	spin_unlock_irqrestore(&toe->irq_lock, flags);
}

static inline unsigned gmac_get_intr_flags(struct net_device *dev, int i)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	void __iomem *irqif_reg, *irqen_reg;
	unsigned offs, val;

	offs = i * (GLOBAL_INTERRUPT_STATUS_1_REG - GLOBAL_INTERRUPT_STATUS_0_REG);

	irqif_reg = toe->iomem + GLOBAL_INTERRUPT_STATUS_0_REG + offs;
	irqen_reg = toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG + offs;

	val = readl(irqif_reg) & readl(irqen_reg);
	return val;
}

enum hrtimer_restart gmac_coalesce_delay_expired( struct hrtimer *timer )
{
	struct gmac_private *gmac = container_of(timer, struct gmac_private, rx_coalesce_timer);

	napi_schedule(&gmac->napi);
	return HRTIMER_NORESTART;
}

static irqreturn_t gmac_irq(int irq, void *data)
{
	struct net_device *dev = data;
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	unsigned val, orr = 0;

	orr |= val = gmac_get_intr_flags(dev, 0);

	if (unlikely(val & (GMAC0_IRQ0_2 << (dev->dev_id * 2)))) {
		/* oh, crap. */
		netdev_err(dev, "hw failure/sw bug\n");
		gmac_dump_dma_state(dev);

		/* don't know how to recover, just reduce losses */
		gmac_enable_irq(dev, 0);
		return IRQ_HANDLED;
	}

	if (val & (GMAC0_IRQ0_TXQ0_INTS << (dev->dev_id * 6)))
		gmac_tx_irq(dev, 0);

	orr |= val = gmac_get_intr_flags(dev, 1);

	if (val & (DEFAULT_Q0_INT_BIT << dev->dev_id)) {

		gmac_enable_rx_irq(dev, 0);

		if (!gmac->rx_coalesce_nsecs)
			napi_schedule(&gmac->napi);
		else {
			ktime_t ktime;
			ktime = ktime_set(0, gmac->rx_coalesce_nsecs);
			hrtimer_start(&gmac->rx_coalesce_timer, ktime, HRTIMER_MODE_REL);
		}
	}

	orr |= val = gmac_get_intr_flags(dev, 4);

	if (unlikely(val & (GMAC0_MIB_INT_BIT << (dev->dev_id * 8))))
		gmac_update_hw_stats(dev);

	if (unlikely(val & (GMAC0_RX_OVERRUN_INT_BIT << (dev->dev_id * 8)))) {
		writel(GMAC0_RXDERR_INT_BIT << (dev->dev_id * 8),
			toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG);

		spin_lock(&toe->irq_lock);
		u64_stats_update_begin(&gmac->ir_stats_syncp);
		++gmac->stats.rx_fifo_errors;
		u64_stats_update_end(&gmac->ir_stats_syncp);
		spin_unlock(&toe->irq_lock);
	}

	return orr ? IRQ_HANDLED : IRQ_NONE;
}

static void gmac_start_dma(struct gmac_private *gmac)
{
	void __iomem *dma_ctrl_reg = gmac->dma_iomem + GMAC_DMA_CTRL_REG;
	GMAC_DMA_CTRL_T	dma_ctrl;

	dma_ctrl.bits32 = readl(dma_ctrl_reg);
	dma_ctrl.bits.rd_enable = 1;
	dma_ctrl.bits.td_enable = 1;
	dma_ctrl.bits.loopback = 0;
	dma_ctrl.bits.drop_small_ack = 0;
	dma_ctrl.bits.rd_insert_bytes = NET_IP_ALIGN;
	dma_ctrl.bits.rd_prot = HPROT_DATA_CACHE | HPROT_PRIVILIGED;
	dma_ctrl.bits.rd_burst_size = HBURST_INCR8;
	dma_ctrl.bits.rd_bus = HSIZE_8;
	dma_ctrl.bits.td_prot = HPROT_DATA_CACHE;
	dma_ctrl.bits.td_burst_size = HBURST_INCR8;
	dma_ctrl.bits.td_bus = HSIZE_8;

	writel(dma_ctrl.bits32, dma_ctrl_reg);
}

static void gmac_stop_dma(struct gmac_private *gmac)
{
	void __iomem *dma_ctrl_reg = gmac->dma_iomem + GMAC_DMA_CTRL_REG;
	GMAC_DMA_CTRL_T	dma_ctrl;

	dma_ctrl.bits32 = readl(dma_ctrl_reg);
	dma_ctrl.bits.rd_enable = 0;
	dma_ctrl.bits.td_enable = 0;
	writel(dma_ctrl.bits32, dma_ctrl_reg);
}

static int gmac_open(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	int err;

	if (!dev->phydev) {
		err = gmac_setup_phy(dev);
		if (err) {
			netif_err(gmac, ifup, dev,
				"PHY init failed: %d\n", err);
			return err;
		}
	}

	err = request_irq(dev->irq, gmac_irq,
		IRQF_SHARED, dev->name, dev);
	if (unlikely(err))
		return err;

	netif_carrier_off(dev);
	phy_start(dev->phydev);

	err = toe_resize_freeq(gmac->toe, dev->dev_id);
	if (unlikely(err))
		goto err_stop_phy;

	err = gmac_setup_rxq(dev);
	if (unlikely(err))
		goto err_stop_phy;

	err = gmac_setup_txqs(dev);
	if (unlikely(err)) {
		gmac_cleanup_rxq(dev);
		goto err_stop_phy;
	}

	napi_enable(&gmac->napi);

	gmac_start_dma(gmac);
	gmac_enable_irq(dev, 1);
	gmac_enable_tx_rx(dev);
	netif_tx_start_all_queues(dev);

	hrtimer_init(&gmac->rx_coalesce_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	gmac->rx_coalesce_timer.function = &gmac_coalesce_delay_expired;
	return 0;

err_stop_phy:
	phy_stop(dev->phydev);
	free_irq(dev->irq, dev);
	return err;
}

static int gmac_stop(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);

	hrtimer_cancel(&gmac->rx_coalesce_timer);
	netif_tx_stop_all_queues(dev);
	gmac_disable_tx_rx(dev);
	gmac_stop_dma(gmac);
	napi_disable(&gmac->napi);

	gmac_enable_irq(dev, 0);
	gmac_cleanup_rxq(dev);
	gmac_cleanup_txqs(dev);

	phy_stop(dev->phydev);
	free_irq(dev->irq, dev);

	gmac_update_hw_stats(dev);
	return 0;
}

static void gmac_set_rx_mode(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct netdev_hw_addr *ha;
	__u32 mc_filter[2];
	unsigned bit_nr;
	GMAC_RX_FLTR_T filter = { .bits = {
		.broadcast = 1,
		.multicast = 1,
		.unicast = 1,
	} };

	mc_filter[1] = mc_filter[0] = 0;

	if (dev->flags & IFF_PROMISC) {
		filter.bits.error = 1;
		filter.bits.promiscuous = 1;
	} else if (!(dev->flags & IFF_ALLMULTI)) {
		mc_filter[1] = mc_filter[0] = 0;
		netdev_for_each_mc_addr(ha, dev) {
			bit_nr = ~crc32_le(~0, ha->addr, ETH_ALEN) & 0x3f;
			mc_filter[bit_nr >> 5] |= 1 << (bit_nr & 0x1f);
		}
	}

	writel(mc_filter[0], gmac->ctl_iomem + GMAC_MCAST_FIL0);
	writel(mc_filter[1], gmac->ctl_iomem + GMAC_MCAST_FIL1);
	writel(filter.bits32, gmac->ctl_iomem + GMAC_RX_FLTR);
}

static void __gmac_set_mac_address(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	__le32 addr[3];

	memset(addr, 0, sizeof(addr));
	memcpy(addr, dev->dev_addr, ETH_ALEN);

	writel(le32_to_cpu(addr[0]), gmac->ctl_iomem + GMAC_STA_ADD0);
	writel(le32_to_cpu(addr[1]), gmac->ctl_iomem + GMAC_STA_ADD1);
	writel(le32_to_cpu(addr[2]), gmac->ctl_iomem + GMAC_STA_ADD2);
}

static int gmac_set_mac_address(struct net_device *dev, void *addr)
{
	struct sockaddr *sa = addr;

	memcpy(dev->dev_addr, sa->sa_data, ETH_ALEN);
	__gmac_set_mac_address(dev);

	return 0;
}

static void gmac_clear_hw_stats(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);

	readl(gmac->ctl_iomem + GMAC_IN_DISCARDS);
	readl(gmac->ctl_iomem + GMAC_IN_ERRORS);
	readl(gmac->ctl_iomem + GMAC_IN_MCAST);
	readl(gmac->ctl_iomem + GMAC_IN_BCAST);
	readl(gmac->ctl_iomem + GMAC_IN_MAC1);
	readl(gmac->ctl_iomem + GMAC_IN_MAC2);
}

static struct rtnl_link_stats64 *gmac_get_stats64(struct net_device *dev,
	struct rtnl_link_stats64 *storage)
{
	struct gmac_private *gmac = netdev_priv(dev);
	unsigned int start;

	gmac_update_hw_stats(dev);

	/* racing with RX NAPI */
	do {
		start = u64_stats_fetch_begin(&gmac->rx_stats_syncp);

		storage->rx_packets = gmac->stats.rx_packets;
		storage->rx_bytes = gmac->stats.rx_bytes;
		storage->rx_errors = gmac->stats.rx_errors;
		storage->rx_dropped = gmac->stats.rx_dropped;

		storage->rx_length_errors = gmac->stats.rx_length_errors;
		storage->rx_over_errors = gmac->stats.rx_over_errors;
		storage->rx_crc_errors = gmac->stats.rx_crc_errors;
		storage->rx_frame_errors = gmac->stats.rx_frame_errors;

	} while (u64_stats_fetch_retry(&gmac->rx_stats_syncp, start));

	/* racing with MIB and TX completion interrupts */
	do {
		start = u64_stats_fetch_begin(&gmac->ir_stats_syncp);

		storage->tx_errors = gmac->stats.tx_errors;
		storage->tx_packets = gmac->stats.tx_packets;
		storage->tx_bytes = gmac->stats.tx_bytes;

		storage->multicast = gmac->stats.multicast;
		storage->rx_missed_errors = gmac->stats.rx_missed_errors;
		storage->rx_fifo_errors = gmac->stats.rx_fifo_errors;

	} while (u64_stats_fetch_retry(&gmac->ir_stats_syncp, start));

	/* racing with hard_start_xmit */
	do {
		start = u64_stats_fetch_begin(&gmac->tx_stats_syncp);

		storage->tx_dropped = gmac->stats.tx_dropped;

	} while (u64_stats_fetch_retry(&gmac->tx_stats_syncp, start));

	storage->rx_dropped += storage->rx_missed_errors;

	return storage;
}

static int gmac_change_mtu(struct net_device *dev, int new_mtu)
{
	int max_len = gmac_pick_rx_max_len(new_mtu);

	if (max_len < 0)
		return -EINVAL;

	gmac_disable_tx_rx(dev);

	dev->mtu = new_mtu;
	gmac_update_config0_reg(dev,
		max_len << CONFIG0_MAXLEN_SHIFT,
		CONFIG0_MAXLEN_MASK);

	netdev_update_features(dev);

	gmac_enable_tx_rx(dev);

	return 0;
}

static netdev_features_t gmac_fix_features(struct net_device *dev, netdev_features_t features)
{
	if (dev->mtu + ETH_HLEN + VLAN_HLEN > MTU_SIZE_BIT_MASK)
		features &= ~GMAC_OFFLOAD_FEATURES;

	return features;
}

static int gmac_set_features(struct net_device *dev, netdev_features_t features)
{
	struct gmac_private *gmac = netdev_priv(dev);
	int enable = features & NETIF_F_RXCSUM;
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&gmac->config_lock, flags);

	reg = readl(gmac->ctl_iomem + GMAC_CONFIG0);
	reg = enable ? reg | CONFIG0_RX_CHKSUM : reg & ~CONFIG0_RX_CHKSUM;
	writel(reg, gmac->ctl_iomem + GMAC_CONFIG0);

	spin_unlock_irqrestore(&gmac->config_lock, flags);
	return 0;
}

static int gmac_get_sset_count(struct net_device *dev, int sset)
{
	return sset == ETH_SS_STATS ? GMAC_STATS_NUM : 0;
}

static void gmac_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	if (stringset != ETH_SS_STATS)
		return;

	memcpy(data, gmac_stats_strings, sizeof(gmac_stats_strings));
}

static void gmac_get_ethtool_stats(struct net_device *dev,
	struct ethtool_stats *estats, u64 *values)
{
	struct gmac_private *gmac = netdev_priv(dev);
	unsigned int start;
	u64 *p;
	int i;

	gmac_update_hw_stats(dev);

	/* racing with MIB interrupt */
	do {
		p = values;
		start = u64_stats_fetch_begin(&gmac->ir_stats_syncp);

		for (i = 0; i < RX_STATS_NUM; ++i)
			*p++ = gmac->hw_stats[i];

	} while (u64_stats_fetch_retry(&gmac->ir_stats_syncp, start));
	values = p;

	/* racing with RX NAPI */
	do {
		p = values;
		start = u64_stats_fetch_begin(&gmac->rx_stats_syncp);

		for (i = 0; i < RX_STATUS_NUM; ++i)
			*p++ = gmac->rx_stats[i];
		for (i = 0; i < RX_CHKSUM_NUM; ++i)
			*p++ = gmac->rx_csum_stats[i];
		*p++ = gmac->rx_napi_exits;

	} while (u64_stats_fetch_retry(&gmac->rx_stats_syncp, start));
	values = p;

	/* racing with TX start_xmit */
	do {
		p = values;
		start = u64_stats_fetch_begin(&gmac->tx_stats_syncp);

		for (i = 0; i < TX_MAX_FRAGS; ++i) {
			*values++ = gmac->tx_frag_stats[i];
			gmac->tx_frag_stats[i] = 0;
		}
		*values++ = gmac->tx_frags_linearized;
		*values++ = gmac->tx_hw_csummed;

	} while (u64_stats_fetch_retry(&gmac->tx_stats_syncp, start));
}

static int gmac_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	if (!dev->phydev)
		return -ENXIO;
	return phy_ethtool_gset(dev->phydev, cmd);
}

static int gmac_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
	if (!dev->phydev)
		return -ENXIO;
	return phy_ethtool_sset(dev->phydev, cmd);
}

static int gmac_nway_reset(struct net_device *dev)
{
	if (!dev->phydev)
		return -ENXIO;
	return phy_start_aneg(dev->phydev);
}

static void gmac_get_pauseparam(struct net_device *dev,
	struct ethtool_pauseparam *pparam)
{
	struct gmac_private *gmac = netdev_priv(dev);
	GMAC_CONFIG0_T config0;

	config0.bits32 = readl(gmac->ctl_iomem + GMAC_CONFIG0);

	pparam->rx_pause = config0.bits.rx_fc_en;
	pparam->tx_pause = config0.bits.tx_fc_en;
	pparam->autoneg = true;
}

static void gmac_get_ringparam(struct net_device *dev,
	struct ethtool_ringparam *rp)
{
	struct gmac_private *gmac = netdev_priv(dev);
	GMAC_CONFIG0_T config0;

	config0.bits32 = readl(gmac->ctl_iomem + GMAC_CONFIG0);

	rp->rx_max_pending = 1 << 15;
	rp->rx_mini_max_pending = 0;
	rp->rx_jumbo_max_pending = 0;
	rp->tx_max_pending = 1 << 15;

	rp->rx_pending = 1 << gmac->rxq_order;
	rp->rx_mini_pending = 0;
	rp->rx_jumbo_pending = 0;
	rp->tx_pending = 1 << gmac->txq_order;
}

static int toe_resize_freeq(struct toe_private *toe, int changing_dev_id);

static int gmac_set_ringparam(struct net_device *dev,
	struct ethtool_ringparam *rp)
{
	struct gmac_private *gmac = netdev_priv(dev);
	struct toe_private *toe = gmac->toe;
	int err = 0;

	if (netif_running(dev))
		return -EBUSY;

	if (rp->rx_pending) {
		gmac->rxq_order = min(15, ilog2(rp->rx_pending - 1) + 1);
		err = toe_resize_freeq(toe, dev->dev_id);
	}

	if (rp->tx_pending)
	{
		gmac->txq_order = min(15, ilog2(rp->tx_pending - 1) + 1);
		gmac->irq_every_tx_packets = 1 << (gmac->txq_order - 2);
	}

	return err;
}

static int gmac_get_coalesce(struct net_device *dev,
	struct ethtool_coalesce *ecmd)
{
	struct gmac_private *gmac = netdev_priv(dev);

	ecmd->rx_max_coalesced_frames = 1;
	ecmd->tx_max_coalesced_frames = gmac->irq_every_tx_packets;
	ecmd->rx_coalesce_usecs = gmac->rx_coalesce_nsecs/1000;

	return 0;
}

static int gmac_set_coalesce(struct net_device *dev,
	struct ethtool_coalesce *ecmd)
{
	struct gmac_private *gmac = netdev_priv(dev);

	if (ecmd->tx_max_coalesced_frames < 1)
		return -EINVAL;
	if (ecmd->tx_max_coalesced_frames >= 1 << gmac->txq_order)
		return -EINVAL;

	gmac->irq_every_tx_packets = ecmd->tx_max_coalesced_frames;
	gmac->rx_coalesce_nsecs = ecmd->rx_coalesce_usecs * 1000;

	return 0;
}

static u32 gmac_get_msglevel(struct net_device *dev)
{
	struct gmac_private *gmac = netdev_priv(dev);
	return gmac->msg_enable;
}

static void gmac_set_msglevel(struct net_device *dev, u32 level)
{
	struct gmac_private *gmac = netdev_priv(dev);
	gmac->msg_enable = level;
}

static void gmac_get_drvinfo(struct net_device *dev,
	struct ethtool_drvinfo *info)
{
	strcpy(info->driver,  DRV_NAME);
	strcpy(info->version, DRV_VERSION);
	strcpy(info->bus_info, dev->dev_id ? "1" : "0");
}

static const struct net_device_ops gmac_351x_ops = {
	.ndo_init		= gmac_init,
	.ndo_uninit		= gmac_uninit,
	.ndo_open		= gmac_open,
	.ndo_stop		= gmac_stop,
	.ndo_start_xmit		= gmac_start_xmit,
	.ndo_tx_timeout		= gmac_tx_timeout,
	.ndo_set_rx_mode	= gmac_set_rx_mode,
	.ndo_set_mac_address	= gmac_set_mac_address,
	.ndo_get_stats64	= gmac_get_stats64,
	.ndo_change_mtu		= gmac_change_mtu,
	.ndo_fix_features	= gmac_fix_features,
	.ndo_set_features	= gmac_set_features,
};

static const struct ethtool_ops gmac_351x_ethtool_ops = {
	.get_sset_count	= gmac_get_sset_count,
	.get_strings	= gmac_get_strings,
	.get_ethtool_stats = gmac_get_ethtool_stats,
	.get_settings	= gmac_get_settings,
	.set_settings	= gmac_set_settings,
	.get_link	= ethtool_op_get_link,
	.nway_reset	= gmac_nway_reset,
	.get_pauseparam	= gmac_get_pauseparam,
	.get_ringparam	= gmac_get_ringparam,
	.set_ringparam	= gmac_set_ringparam,
	.get_coalesce	= gmac_get_coalesce,
	.set_coalesce	= gmac_set_coalesce,
	.get_msglevel	= gmac_get_msglevel,
	.set_msglevel	= gmac_set_msglevel,
	.get_drvinfo	= gmac_get_drvinfo,
};

static int gmac_init_netdev(struct toe_private *toe, int num,
	struct platform_device *pdev)
{
	struct gemini_gmac_platform_data *pdata = pdev->dev.platform_data;
	struct gmac_private *gmac;
	struct net_device *dev;
	int irq, err;

	if (!pdata->bus_id[num])
		return 0;

	irq = platform_get_irq(pdev, num);
	if (irq < 0) {
		dev_err(toe->dev, "No IRQ for ethernet device #%d\n", num);
		return irq;
	}

	dev = alloc_etherdev_mq(sizeof(*gmac), TX_QUEUE_NUM);
	if (!dev) {
		dev_err(toe->dev, "Can't allocate ethernet device #%d\n", num);
		return -ENOMEM;
	}

	gmac = netdev_priv(dev);
	gmac->num = num;
	gmac->toe = toe;
	SET_NETDEV_DEV(dev, toe->dev);

	toe->netdev[num] = dev;
	dev->dev_id = num;

	gmac->ctl_iomem = toe->iomem + TOE_GMAC_BASE(num);
	gmac->dma_iomem = toe->iomem + TOE_GMAC_DMA_BASE(num);
	dev->irq = irq;

	dev->netdev_ops = &gmac_351x_ops;
	dev->ethtool_ops = &gmac_351x_ethtool_ops;

	spin_lock_init(&gmac->config_lock);
	gmac_clear_hw_stats(dev);

	dev->hw_features = GMAC_OFFLOAD_FEATURES;
	dev->features |= GMAC_OFFLOAD_FEATURES | NETIF_F_GRO;

	gmac->freeq_refill = 0;
	netif_napi_add(dev, &gmac->napi, gmac_napi_poll, DEFAULT_NAPI_WEIGHT);

	if (is_valid_ether_addr((void *)toe->mac_addr[num]))
		memcpy(dev->dev_addr, toe->mac_addr[num], ETH_ALEN);
	else
		random_ether_addr(dev->dev_addr);
	__gmac_set_mac_address(dev);

	err = gmac_setup_phy(dev);
	if (err)
		netif_warn(gmac, probe, dev,
			"PHY init failed: %d, deferring to ifup time\n", err);

	err = register_netdev(dev);
	if (!err)
	{
		pr_info(DRV_NAME " %s: irq %d, dma base 0x%p, io base 0x%p\n",
			dev->name, irq, gmac->dma_iomem, gmac->ctl_iomem);
		return 0;
	}

	toe->netdev[num] = NULL;
	free_netdev(dev);
	return err;
}

static irqreturn_t toe_irq_thread(int irq, void *data)
{
	struct toe_private *toe = data;
	void __iomem *irqen_reg = toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG;
	void __iomem *irqif_reg = toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG;
	unsigned long irqmask = SWFQ_EMPTY_INT_BIT;
	unsigned long flags;

	toe_fill_freeq(toe, 0);

	/* Ack and enable interrupt */
	spin_lock_irqsave(&toe->irq_lock, flags);
	writel(irqmask, irqif_reg);
	irqmask |= readl(irqen_reg);
	writel(irqmask, irqen_reg);
	spin_unlock_irqrestore(&toe->irq_lock, flags);

	return IRQ_HANDLED;
}

static irqreturn_t toe_irq(int irq, void *data)
{
	struct toe_private *toe = data;
	void __iomem *irqif_reg = toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG;
	void __iomem *irqen_reg = toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG;
	unsigned long val, en;
	irqreturn_t ret = IRQ_NONE;

	spin_lock(&toe->irq_lock);

	val = readl(irqif_reg);
	en = readl(irqen_reg);

	if (val & en & SWFQ_EMPTY_INT_BIT) {
		en &= ~(SWFQ_EMPTY_INT_BIT | GMAC0_RX_OVERRUN_INT_BIT
					   | GMAC1_RX_OVERRUN_INT_BIT);
		writel(en, irqen_reg);
		ret = IRQ_WAKE_THREAD;
	}

	spin_unlock(&toe->irq_lock);
	return ret;
}

static int toe_init(struct toe_private *toe,
	struct platform_device *pdev)
{
	int err;

	writel(0, toe->iomem + GLOBAL_SW_FREEQ_BASE_SIZE_REG);
	writel(0, toe->iomem + GLOBAL_HW_FREEQ_BASE_SIZE_REG);
	writel(0, toe->iomem + GLOBAL_SWFQ_RWPTR_REG);
	writel(0, toe->iomem + GLOBAL_HWFQ_RWPTR_REG);

	toe->freeq_frag_order = DEFAULT_RX_BUF_ORDER;
	toe->freeq_order = ~0;

	err = request_threaded_irq(toe->irq, toe_irq,
		toe_irq_thread, IRQF_SHARED, DRV_NAME " toe", toe);
	if (err)
		goto err_freeq;

	return 0;

err_freeq:
	toe_cleanup_freeq(toe);
	return err;
}

static void toe_deinit(struct toe_private *toe)
{
	free_irq(toe->irq, toe);
	toe_cleanup_freeq(toe);
}

static int toe_reset(struct toe_private *toe)
{
	unsigned int reg = 0, retry = 5;

	reg = readl((void __iomem*)(IO_ADDRESS(GEMINI_GLOBAL_BASE) +
		GLOBAL_RESET));
	reg |= RESET_GMAC1 | RESET_GMAC0;
	writel(reg, (void __iomem*)(IO_ADDRESS(GEMINI_GLOBAL_BASE) +
		GLOBAL_RESET));

	do {
		udelay(2);
		reg = readl((void __iomem*)(toe->iomem +
			GLOBAL_TOE_VERSION_REG));
		barrier();
	} while (!reg && --retry);

	return reg ? 0 : -EIO;
}

/*
 * Interrupt config:
 *
 *	GMAC0 intr bits ------> int0 ----> eth0
 *	GMAC1 intr bits ------> int1 ----> eth1
 *	TOE intr -------------> int1 ----> eth1
 *	Classification Intr --> int0 ----> eth0
 *	Default Q0 -----------> int0 ----> eth0
 *	Default Q1 -----------> int1 ----> eth1
 *	FreeQ intr -----------> int1 ----> eth1
 */
static void toe_init_irq(struct toe_private *toe)
{
	writel(0, toe->iomem + GLOBAL_INTERRUPT_ENABLE_0_REG);
	writel(0, toe->iomem + GLOBAL_INTERRUPT_ENABLE_1_REG);
	writel(0, toe->iomem + GLOBAL_INTERRUPT_ENABLE_2_REG);
	writel(0, toe->iomem + GLOBAL_INTERRUPT_ENABLE_3_REG);
	writel(0, toe->iomem + GLOBAL_INTERRUPT_ENABLE_4_REG);

	writel(0xCCFC0FC0, toe->iomem + GLOBAL_INTERRUPT_SELECT_0_REG);
	writel(0x00F00002, toe->iomem + GLOBAL_INTERRUPT_SELECT_1_REG);
	writel(0xFFFFFFFF, toe->iomem + GLOBAL_INTERRUPT_SELECT_2_REG);
	writel(0xFFFFFFFF, toe->iomem + GLOBAL_INTERRUPT_SELECT_3_REG);
	writel(0xFF000003, toe->iomem + GLOBAL_INTERRUPT_SELECT_4_REG);

	/* edge-triggered interrupts packed to level-triggered one... */
	writel(~0, toe->iomem + GLOBAL_INTERRUPT_STATUS_0_REG);
	writel(~0, toe->iomem + GLOBAL_INTERRUPT_STATUS_1_REG);
	writel(~0, toe->iomem + GLOBAL_INTERRUPT_STATUS_2_REG);
	writel(~0, toe->iomem + GLOBAL_INTERRUPT_STATUS_3_REG);
	writel(~0, toe->iomem + GLOBAL_INTERRUPT_STATUS_4_REG);
}

static void toe_save_mac_addr(struct toe_private *toe,
			struct platform_device *pdev)
{
	struct gemini_gmac_platform_data *pdata = pdev->dev.platform_data;
	void __iomem *ctl;
	int i;

	for (i = 0; i < 2; i++) {
		if (pdata->bus_id[i]) {
			ctl = toe->iomem + TOE_GMAC_BASE(i);
			toe->mac_addr[i][0] = cpu_to_le32(readl(ctl + GMAC_STA_ADD0));
			toe->mac_addr[i][1] = cpu_to_le32(readl(ctl + GMAC_STA_ADD1));
			toe->mac_addr[i][2] = cpu_to_le32(readl(ctl + GMAC_STA_ADD2));
		}
	}
}

static int gemini_gmac_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct toe_private *toe;
	int irq, retval;

	if (!pdev->dev.platform_data)
		return -EINVAL;

	irq = platform_get_irq(pdev, 1);
	if (irq < 0)
		return irq;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can't get device resources\n");
		return -ENODEV;
	}

	toe = kzalloc(sizeof(*toe), GFP_KERNEL);
	if (!toe)
		return -ENOMEM;

	platform_set_drvdata(pdev, toe);
	toe->dev = &pdev->dev;
	toe->irq = irq;

	toe->iomem = ioremap(res->start, resource_size(res));
	if (!toe->iomem) {
		dev_err(toe->dev, "ioremap failed\n");
		retval = -EIO;
		goto err_data;
	}

	toe_save_mac_addr(toe, pdev);

	retval = toe_reset(toe);
	if (retval < 0)
		goto err_unmap;

	pr_info(DRV_NAME " toe: irq %d, io base 0x%08x, version %d\n",
		irq, res->start, retval);

	spin_lock_init(&toe->irq_lock);
	spin_lock_init(&toe->freeq_lock);

	toe_init_irq(toe);

	retval = toe_init(toe, pdev);
	if (retval)
		goto err_unmap;

	retval = gmac_init_netdev(toe, 0, pdev);
	if (retval)
		goto err_uninit;

	retval = gmac_init_netdev(toe, 1, pdev);
	if (retval)
		goto err_uninit;

	return 0;

err_uninit:
	if (toe->netdev[0])
		unregister_netdev(toe->netdev[0]);
	toe_deinit(toe);
err_unmap:
	iounmap(toe->iomem);
err_data:
	kfree(toe);
	return retval;
}

static int gemini_gmac_remove(struct platform_device *pdev)
{
	struct toe_private *toe = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < 2; i++)
		if (toe->netdev[i])
			unregister_netdev(toe->netdev[i]);

	toe_init_irq(toe);
	toe_deinit(toe);

	iounmap(toe->iomem);
	kfree(toe);

	return 0;
}

static struct platform_driver gemini_gmac_driver = {
	.probe		= gemini_gmac_probe,
	.remove		= gemini_gmac_remove,
	.driver.name	= DRV_NAME,
	.driver.owner	= THIS_MODULE,
};

static int __init gemini_gmac_init(void)
{
#ifdef CONFIG_MDIO_GPIO_MODULE
	request_module("mdio-gpio");
#endif
	return platform_driver_register(&gemini_gmac_driver);
}

static void __exit gemini_gmac_exit(void)
{
	platform_driver_unregister(&gemini_gmac_driver);
}

module_init(gemini_gmac_init);
module_exit(gemini_gmac_exit);
