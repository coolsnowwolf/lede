/*
 * Moschip MCS8140 Ethernet MAC driver
 *
 * Copyright (C) 2003, Moschip Semiconductors
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under GPLv2
 */
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/phy.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>

#include <asm/unaligned.h>
#include <asm/sizes.h>

/* Hardware registers */
#define MAC_BASE_ADDR		((priv->mac_base))

#define CTRL_REG		(MAC_BASE_ADDR)
#define  MII_BUSY		(1 << 0)
#define  MII_WRITE		(1 << 1)
#define  RX_ENABLE		(1 << 2)
#define  TX_ENABLE		(1 << 3)
#define  DEFER_CHECK		(1 << 5)
#define  STRIP_PAD		(1 << 8)
#define  DRTRY_DISABLE		(1 << 10)
#define  FULL_DUPLEX		(1 << 20)
#define  HBD_DISABLE		(1 << 28)
#define MAC_ADDR_HIGH_REG	(MAC_BASE_ADDR + 0x04)
#define MAC_ADDR_LOW_REG	(MAC_BASE_ADDR + 0x08)
#define MII_ADDR_REG		(MAC_BASE_ADDR + 0x14)
#define  MII_ADDR_SHIFT		(11)
#define  MII_REG_SHIFT		(6)
#define MII_DATA_REG		(MAC_BASE_ADDR + 0x18)
/* Link interrupt registers */
#define LINK_INT_CSR		(MAC_BASE_ADDR + 0xD0)
#define  LINK_INT_EN		(1 << 0)
#define  LINK_PHY_ADDR_SHIFT	(1)
#define  LINK_PHY_REG_SHIFT	(6)
#define  LINK_BIT_UP_SHIFT	(11)
#define  LINK_UP		(1 << 16)
#define LINK_INT_POLL_TIME	(MAC_BASE_ADDR + 0xD4)
#define  LINK_POLL_MASK		((1 << 20) - 1)

#define DMA_CHAN_WIDTH		32
#define DMA_RX_CHAN		0
#define DMA_TX_CHAN		2

/* Receive DMA registers */
#define RX_DMA_BASE		((priv->dma_base) + \
				(DMA_CHAN_WIDTH * DMA_RX_CHAN))
#define RX_BUFFER_ADDR		(RX_DMA_BASE + 0x00)
#define RX_MAX_BYTES		(RX_DMA_BASE + 0x04)
#define RX_ACT_BYTES		(RX_DMA_BASE + 0x08)
#define RX_START_DMA		(RX_DMA_BASE + 0x0C)
#define  RX_DMA_ENABLE		(1 << 0)
#define  RX_DMA_RESET		(1 << 1)
#define  RX_DMA_STATUS_FIFO	(1 << 12)
#define RX_DMA_ENH		(RX_DMA_BASE + 0x14)
#define  RX_DMA_INT_ENABLE	(1 << 1)

/* Transmit DMA registers */
#define TX_DMA_BASE		((priv->dma_base) + \
				(DMA_CHAN_WIDTH * DMA_TX_CHAN))
#define TX_BUFFER_ADDR		(TX_DMA_BASE + 0x00)
#define TX_PKT_BYTES		(TX_DMA_BASE + 0x04)
#define TX_BYTES_SENT		(TX_DMA_BASE + 0x08)
#define TX_START_DMA		(TX_DMA_BASE + 0x0C)
#define  TX_DMA_ENABLE		(1 << 0)
#define  TX_DMA_START_FRAME	(1 << 2)
#define  TX_DMA_END_FRAME	(1 << 3)
#define  TX_DMA_PAD_DISABLE	(1 << 8)
#define  TX_DMA_CRC_DISABLE	(1 << 9)
#define  TX_DMA_FIFO_FULL	(1 << 16)
#define  TX_DMA_FIFO_EMPTY	(1 << 17)
#define  TX_DMA_STATUS_AVAIL	(1 << 18)
#define  TX_DMA_RESET		(1 << 24)
#define TX_DMA_STATUS		(TX_DMA_BASE + 0x10)
#define TX_DMA_ENH		(TX_DMA_BASE + 0x14)
#define  TX_DMA_ENH_ENABLE	(1 << 0)
#define  TX_DMA_INT_FIFO	(1 << 1)

#define RX_ALLOC_SIZE		SZ_2K
#define MAX_ETH_FRAME_SIZE	1536
#define RX_SKB_TAILROOM		128
#define RX_SKB_HEADROOM		(RX_ALLOC_SIZE  - \
				(MAX_ETH_FRAME_SIZE + RX_SKB_TAILROOM) + 0)

			/* WDT     Late COL    Lenght     COL      Type */
#define ERROR_FILTER_MASK ((1<<14) | (1<<15) | (1<<16) | (1<<17) | (0<<18) | \
			/* MII    Dribbling    CRC    Len/type   Control */\
			(1<<19) | (1<<20) | (1<<21) | (0<<24) | (1<<25) | \
			/* Unsup   Missed */\
			(1<<26) | (0<<31))
#define  TX_RING_SIZE  30
#define  RX_RING_SIZE  30

static inline u32 nuport_mac_readl(void __iomem *reg)
{
	return readl_relaxed(reg);
}

static inline u8 nuport_mac_readb(void __iomem *reg)
{
	return readb_relaxed(reg);
}

static inline void nuport_mac_writel(u32 value, void __iomem *reg)
{
	writel_relaxed(value, reg);
}

static inline void nuport_mac_writeb(u8 value, void __iomem *reg)
{
	writel_relaxed(value, reg);
}

/* MAC private data */
struct nuport_mac_priv {
	spinlock_t lock;

	void __iomem	*mac_base;
	void __iomem	*dma_base;

	int		rx_irq;
	int		tx_irq;
	int		link_irq;
	struct clk	*emac_clk;
	struct clk	*ephy_clk;

	/* Transmit buffers */
	struct sk_buff *tx_skb[TX_RING_SIZE];
	dma_addr_t tx_addr;
	unsigned int valid_txskb[TX_RING_SIZE];
	unsigned int cur_tx;
	unsigned int dma_tx;
	unsigned int tx_full;

	/* Receive buffers */
	struct sk_buff *rx_skb[RX_RING_SIZE];
	dma_addr_t rx_addr;
	unsigned int irq_rxskb[RX_RING_SIZE];
	int pkt_len[RX_RING_SIZE];
	unsigned int cur_rx;
	unsigned int dma_rx;
	unsigned int rx_full;

	unsigned int first_pkt;

	/* Private data */
	struct napi_struct napi;
	struct net_device	*dev;
	struct platform_device	*pdev;
	struct mii_bus		*mii_bus;
	struct phy_device	*phydev;
	struct device_node	*phy_node;
	phy_interface_t		phy_interface;
	int			old_link;
	int			old_duplex;
	u32			msg_level;
	unsigned int		buffer_shifting_len;
};

static inline int nuport_mac_mii_busy_wait(struct nuport_mac_priv *priv)
{
	unsigned long curr;
	unsigned long finish = jiffies + 3 * HZ;

	do {
		curr = jiffies;
		if (!(nuport_mac_readl(MII_ADDR_REG) & MII_BUSY))
			return 0;
		cpu_relax();
	} while (!time_after_eq(curr, finish));

	return -EBUSY;
}

/* Read from PHY registers */
static int nuport_mac_mii_read(struct mii_bus *bus,
				int mii_id, int regnum)
{
	struct net_device *dev = bus->priv;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	int ret;
	u32 val = 0;

	ret = nuport_mac_mii_busy_wait(priv);
	if (ret)
		return ret;

	val |= (mii_id << MII_ADDR_SHIFT) | (regnum << MII_REG_SHIFT) | MII_BUSY;
	nuport_mac_writel(val, MII_ADDR_REG);
	ret = nuport_mac_mii_busy_wait(priv);
	if (ret)
		return ret;

	return nuport_mac_readl(MII_DATA_REG);
}

static int nuport_mac_mii_write(struct mii_bus *bus, int mii_id,
				int regnum, u16 value)
{
	struct net_device *dev = bus->priv;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	int ret;
	u32 val = 0;

	ret = nuport_mac_mii_busy_wait(priv);
	if (ret)
		return ret;

	val |= (mii_id << MII_ADDR_SHIFT) | (regnum << MII_REG_SHIFT);
	val |= MII_BUSY | MII_WRITE;
	nuport_mac_writel(value, MII_DATA_REG);
	nuport_mac_writel(val, MII_ADDR_REG);

	return nuport_mac_mii_busy_wait(priv);
}

static int nuport_mac_mii_reset(struct mii_bus *bus)
{
	return 0;
}

static int nuport_mac_start_tx_dma(struct nuport_mac_priv *priv,
					struct sk_buff *skb)
{
	u32 reg;
	unsigned int timeout = 2048;

	while (timeout--) {
		reg = nuport_mac_readl(TX_START_DMA);
		if (!(reg & TX_DMA_ENABLE)) {
			netdev_dbg(priv->dev, "dma ready\n");
			break;
		}
		cpu_relax();
	}

	if (!timeout)
		return -EBUSY;

	priv->tx_addr = dma_map_single(&priv->pdev->dev, skb->data,
			skb->len, DMA_TO_DEVICE);
	if (dma_mapping_error(&priv->pdev->dev, priv->tx_addr))
		return -ENOMEM;

	/* enable enhanced mode */
	nuport_mac_writel(TX_DMA_ENH_ENABLE, TX_DMA_ENH);
	nuport_mac_writel(priv->tx_addr, TX_BUFFER_ADDR);
	nuport_mac_writel((skb->len) - 1, TX_PKT_BYTES);
	wmb();
	reg = TX_DMA_ENABLE | TX_DMA_START_FRAME | TX_DMA_END_FRAME;
	nuport_mac_writel(reg, TX_START_DMA);

	return 0;
}

static void nuport_mac_reset_tx_dma(struct nuport_mac_priv *priv)
{
	u32 reg;

	reg = nuport_mac_readl(TX_START_DMA);
	reg |= TX_DMA_RESET;
	nuport_mac_writel(reg, TX_START_DMA);
}

static int nuport_mac_start_rx_dma(struct nuport_mac_priv *priv,
					struct sk_buff *skb)
{
	u32 reg;
	unsigned int timeout = 2048;

	while (timeout--) {
		reg = nuport_mac_readl(RX_START_DMA);
		if (!(reg & RX_DMA_ENABLE)) {
			netdev_dbg(priv->dev, "dma ready\n");
			break;
		}
		cpu_relax();
	}

	if (!timeout)
		return -EBUSY;

	priv->rx_addr = dma_map_single(&priv->pdev->dev, skb->data,
				RX_ALLOC_SIZE, DMA_FROM_DEVICE);
	if (dma_mapping_error(&priv->pdev->dev, priv->rx_addr))
		return -ENOMEM;

	nuport_mac_writel(priv->rx_addr, RX_BUFFER_ADDR);
	wmb();
	nuport_mac_writel(RX_DMA_ENABLE, RX_START_DMA);

	return 0;
}

static void nuport_mac_reset_rx_dma(struct nuport_mac_priv *priv)
{
	u32 reg;

	reg = nuport_mac_readl(RX_START_DMA);
	reg |= RX_DMA_RESET;
	nuport_mac_writel(reg, RX_START_DMA);
}

/* I suppose this might do something, but I am not sure actually */
static void nuport_mac_disable_rx_dma(struct nuport_mac_priv *priv)
{
	u32 reg;

	reg = nuport_mac_readl(RX_DMA_ENH);
	reg &= ~RX_DMA_INT_ENABLE;
	nuport_mac_writel(reg, RX_DMA_ENH);
}

static void nuport_mac_enable_rx_dma(struct nuport_mac_priv *priv)
{
	u32 reg;

	reg = nuport_mac_readl(RX_DMA_ENH);
	reg |= RX_DMA_INT_ENABLE;
	nuport_mac_writel(reg, RX_DMA_ENH);
}

/* Add packets to the transmit queue */
static int nuport_mac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	unsigned long flags;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	int ret;

	if (netif_queue_stopped(dev)) {
		netdev_warn(dev, "netif queue was stopped, restarting\n");
		netif_start_queue(dev);
	}

	spin_lock_irqsave(&priv->lock, flags);
	if (priv->first_pkt) {
		ret = nuport_mac_start_tx_dma(priv, skb);
		if (ret) {
			netif_stop_queue(dev);
			spin_unlock_irqrestore(&priv->lock, flags);
			netdev_err(dev, "transmit path busy\n");
			return NETDEV_TX_BUSY;
		}
		priv->first_pkt = 0;
	}

	priv->tx_skb[priv->cur_tx] = skb;
	dev->stats.tx_bytes += skb->len;
	dev->stats.tx_packets++;
	priv->valid_txskb[priv->cur_tx] = 1;
	priv->cur_tx++;
	dev->trans_start = jiffies;

	if (priv->cur_tx >= TX_RING_SIZE)
		priv->cur_tx = 0;

	spin_unlock_irqrestore(&priv->lock, flags);

	if (priv->valid_txskb[priv->cur_tx]) {
		priv->tx_full = 1;
		netdev_err(dev, "stopping queue\n");
		netif_stop_queue(dev);
	}

	return NETDEV_TX_OK;
}

static void nuport_mac_adjust_link(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = priv->phydev;
	unsigned int status_changed = 0;
	u32 reg;

	BUG_ON(!phydev);

	if (priv->old_link != phydev->link) {
		status_changed = 1;
		priv->old_link = phydev->link;
	}

	if (phydev->link && (priv->old_duplex != phydev->duplex)) {
		reg = nuport_mac_readl(CTRL_REG);
		if (phydev->duplex == DUPLEX_FULL)
			reg |= DUPLEX_FULL;
		else
			reg &= ~DUPLEX_FULL;
		nuport_mac_writel(reg, CTRL_REG);

		status_changed = 1;
		priv->old_duplex = phydev->duplex;
	}

	if (!status_changed)
		return;

	pr_info("%s: link %s", dev->name, phydev->link ?
		"UP" : "DOWN");
	if (phydev->link) {
		pr_cont(" - %d/%s", phydev->speed,
		phydev->duplex == DUPLEX_FULL ? "full" : "half");
	}
	pr_cont("\n");
}

static irqreturn_t nuport_mac_link_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	u32 reg;
	u8 phy_addr;
	unsigned long flags;
	irqreturn_t ret = IRQ_HANDLED;

	spin_lock_irqsave(&priv->lock, flags);
	reg = nuport_mac_readl(LINK_INT_CSR);
	phy_addr = (reg >> LINK_PHY_ADDR_SHIFT) & (PHY_MAX_ADDR - 1);

	if (phy_addr != priv->phydev->addr) {
		netdev_err(dev, "spurious PHY irq (phy: %d)\n", phy_addr);
		ret = IRQ_NONE;
		goto out;
	}

	priv->phydev->link = (reg & LINK_UP);
	nuport_mac_adjust_link(dev);

out:
	spin_unlock_irqrestore(&priv->lock, flags);
	return ret;
}

static irqreturn_t nuport_mac_tx_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	unsigned long flags;
	int ret;
	u32 reg;

	spin_lock_irqsave(&priv->lock, flags);
	/* clear status word available if ready */
	reg = nuport_mac_readl(TX_START_DMA);
	if (reg & TX_DMA_STATUS_AVAIL) {
		nuport_mac_writel(reg, TX_START_DMA);
		reg = nuport_mac_readl(TX_DMA_STATUS);

		if (reg & 1)
			dev->stats.tx_errors++;
	} else
		netdev_dbg(dev, "no status word: %08x\n", reg);

	skb = priv->tx_skb[priv->dma_tx];
	priv->tx_skb[priv->dma_tx] = NULL;
	priv->valid_txskb[priv->dma_tx] = 0;
	dma_unmap_single(&priv->pdev->dev, priv->rx_addr, skb->len,
				DMA_TO_DEVICE);
	dev_kfree_skb_irq(skb);

	priv->dma_tx++;
	if (priv->dma_tx >= TX_RING_SIZE)
		priv->dma_tx = 0;

	if (!priv->valid_txskb[priv->dma_tx])
		priv->first_pkt = 1;
	else {
		ret = nuport_mac_start_tx_dma(priv, priv->tx_skb[priv->dma_tx]);
		if (ret)
			netdev_err(dev, "failed to restart TX dma\n");
	}

	if (priv->tx_full) {
		netdev_dbg(dev, "restarting transmit queue\n");
		netif_wake_queue(dev);
		priv->tx_full = 0;
	}

	spin_unlock_irqrestore(&priv->lock, flags);

	return IRQ_HANDLED;
}

static unsigned int nuport_mac_has_work(struct nuport_mac_priv *priv)
{
	unsigned int i;

	for (i = 0; i < RX_RING_SIZE; i++)
		if (priv->rx_skb[i])
			return 1;

	return 0;
}

static irqreturn_t nuport_mac_rx_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = (struct net_device *)dev_id;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&priv->lock, flags);
	if (!priv->rx_full) {
		priv->pkt_len[priv->dma_rx] = nuport_mac_readl(RX_ACT_BYTES) - 4;
		priv->irq_rxskb[priv->dma_rx] = 0;
		priv->dma_rx++;

		if (priv->dma_rx >= RX_RING_SIZE)
			priv->dma_rx = 0;
	} else
		priv->rx_full = 0;

	if (priv->irq_rxskb[priv->dma_rx] == 1) {
		ret = nuport_mac_start_rx_dma(priv, priv->rx_skb[priv->dma_rx]);
		if (ret)
			netdev_err(dev, "failed to start rx dma\n");
	} else {
		priv->rx_full = 1;
		netdev_dbg(dev, "RX ring full\n");
	}

	if (likely(nuport_mac_has_work(priv))) {
		/* find a way to disable DMA rx irq */
		nuport_mac_disable_rx_dma(priv);
		napi_schedule(&priv->napi);
	}
	spin_unlock_irqrestore(&priv->lock, flags);

	return IRQ_HANDLED;
}

/* Process received packets in tasklet */
static int nuport_mac_rx(struct net_device *dev, int limit)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	int len, status;
	int count = 0;

	while (count < limit && !priv->irq_rxskb[priv->cur_rx]) {
		skb = priv->rx_skb[priv->cur_rx];
		len = priv->pkt_len[priv->cur_rx];

		/* Remove 2 bytes added by RX buffer shifting */
		len = len - priv->buffer_shifting_len;
		skb->data = skb->data + priv->buffer_shifting_len;

		/* Get packet status */
		status = get_unaligned((u32 *) (skb->data + len));

		dma_unmap_single(&priv->pdev->dev, priv->rx_addr, skb->len,
				DMA_FROM_DEVICE);

		/* packet filter failed */
		if (!(status & (1 << 30))) {
			dev_kfree_skb_irq(skb);
			goto exit;
		}

		/* missed frame */
		if (status & (1 << 31)) {
			dev->stats.rx_missed_errors++;
			dev_kfree_skb_irq(skb);
			goto exit;
		}

		/* Not ethernet type */
		if ((!(status & (1 << 18))) || (status & ERROR_FILTER_MASK))
			dev->stats.rx_errors++;

		if (len > MAX_ETH_FRAME_SIZE) {
			dev_kfree_skb_irq(skb);
			goto exit;
		} else
			skb_put(skb, len);

		skb->protocol = eth_type_trans(skb, dev);
		dev->stats.rx_packets++;

		if (status & (1 << 29))
			skb->pkt_type = PACKET_OTHERHOST;
		if (status & (1 << 27))
			skb->pkt_type = PACKET_MULTICAST;
		if (status & (1 << 28))
			skb->pkt_type = PACKET_BROADCAST;

		skb->ip_summed = CHECKSUM_UNNECESSARY;

		/* Pass the received packet to network layer */
		status = netif_receive_skb(skb);
		if (status != NET_RX_DROP)
			dev->stats.rx_bytes += len - 4;	/* Without CRC */
		else
			dev->stats.rx_dropped++;

		dev->last_rx = jiffies;

exit:
		skb = netdev_alloc_skb(dev, RX_ALLOC_SIZE);
		if (!skb)
			goto out;

		skb_reserve(skb, RX_SKB_HEADROOM);
		priv->rx_skb[priv->cur_rx] = skb;
		priv->irq_rxskb[priv->cur_rx] = 1;
		priv->cur_rx++;

		if (priv->cur_rx >= RX_RING_SIZE)
			priv->cur_rx = 0;
		count++;
	}
out:
	return count;
}

static int nuport_mac_poll(struct napi_struct *napi, int budget)
{
	struct nuport_mac_priv *priv =
		container_of(napi, struct nuport_mac_priv, napi);
	struct net_device *dev = priv->dev;
	int work_done;

	work_done = nuport_mac_rx(dev, budget);

	if (work_done < budget) {
		napi_complete(napi);
		nuport_mac_enable_rx_dma(priv);
	}

	return work_done;
}

static void nuport_mac_init_tx_ring(struct nuport_mac_priv *priv)
{
	int i;

	priv->cur_tx = priv->dma_tx = priv->tx_full = 0;
	for (i = 0; i < TX_RING_SIZE; i++) {
		priv->tx_skb[i] = NULL;
		priv->valid_txskb[i] = 0;
	}
	priv->first_pkt = 1;
}

static int nuport_mac_init_rx_ring(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	struct sk_buff *skb;
	int i;

	priv->cur_rx = priv->dma_rx = priv->rx_full = 0;

	for (i = 0; i < RX_RING_SIZE; i++) {
		skb = netdev_alloc_skb(dev, RX_ALLOC_SIZE);
		if (!skb)
			return -ENOMEM;
		skb_reserve(skb, RX_SKB_HEADROOM);
		priv->rx_skb[i] = skb;
		priv->irq_rxskb[i] = 1;
	}

	return 0;
}

static void nuport_mac_free_rx_ring(struct nuport_mac_priv *priv)
{
	int i;

	for (i = 0; i < RX_RING_SIZE; i++) {
		if (!priv->rx_skb[i])
			continue;

		dev_kfree_skb(priv->rx_skb[i]);
		priv->rx_skb[i] = NULL;
	}

	if (priv->rx_addr)
		dma_unmap_single(&priv->pdev->dev, priv->rx_addr, RX_ALLOC_SIZE,
				DMA_TO_DEVICE);
}

static void nuport_mac_read_mac_address(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	int i;

	for (i = 0; i < 4; i++)
		dev->dev_addr[i] = nuport_mac_readb(MAC_ADDR_LOW_REG + i);
	dev->dev_addr[4] = nuport_mac_readb(MAC_ADDR_HIGH_REG);
	dev->dev_addr[5] = nuport_mac_readb(MAC_ADDR_HIGH_REG + 1);

	if (!is_valid_ether_addr(dev->dev_addr)) {
		dev_info(&priv->pdev->dev, "using random address\n");
		random_ether_addr(dev->dev_addr);
	}
}

static int nuport_mac_change_mac_address(struct net_device *dev, void *mac_addr)
{
	struct sockaddr *addr = mac_addr;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	unsigned long *temp = (unsigned long *)dev->dev_addr;
	u32 high, low;

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);

	spin_lock_irq(&priv->lock);

	nuport_mac_writel(*temp, MAC_ADDR_LOW_REG);
	temp = (unsigned long *)(dev->dev_addr + 4);
	nuport_mac_writel(*temp, MAC_ADDR_HIGH_REG);

	low = nuport_mac_readl(MAC_ADDR_LOW_REG);
	high = nuport_mac_readl(MAC_ADDR_HIGH_REG);

	spin_unlock_irq(&priv->lock);

	return 0;
}

static int nuport_mac_open(struct net_device *dev)
{
	int ret;
	struct nuport_mac_priv *priv = netdev_priv(dev);
	unsigned long flags;
	u32 reg = 0;

	ret = clk_enable(priv->emac_clk);
	if (ret) {
		netdev_err(dev, "failed to enable EMAC clock\n");
		return ret;
	}

	/* Set MAC into full duplex mode by default */
	reg |= RX_ENABLE | TX_ENABLE;
	reg |= DEFER_CHECK | STRIP_PAD | DRTRY_DISABLE;
	reg |= FULL_DUPLEX | HBD_DISABLE;
	nuport_mac_writel(reg, CTRL_REG);

	/* set mac address in hardware in case it was not already */
	nuport_mac_change_mac_address(dev, dev->dev_addr);

	ret = request_irq(priv->link_irq, &nuport_mac_link_interrupt,
				0, dev->name, dev);
	if (ret) {
		netdev_err(dev, "unable to request link interrupt\n");
		goto out_emac_clk;
	}

	ret = request_irq(priv->tx_irq, &nuport_mac_tx_interrupt,
				0, dev->name, dev);
	if (ret) {
		netdev_err(dev, "unable to request rx interrupt\n");
		goto out_link_irq;
	}

	/* Enable link interrupt monitoring for our PHY address */
	reg = LINK_INT_EN | (priv->phydev->addr << LINK_PHY_ADDR_SHIFT);
	/* MII_BMSR register to be watched */
	reg |= (1 << LINK_PHY_REG_SHIFT);
	/* BMSR_STATUS to be watched in particular */
	reg |= (2 << LINK_BIT_UP_SHIFT);

	spin_lock_irqsave(&priv->lock, flags);
	nuport_mac_writel(reg, LINK_INT_CSR);
	nuport_mac_writel(LINK_POLL_MASK, LINK_INT_POLL_TIME);
	spin_unlock_irqrestore(&priv->lock, flags);

	phy_start(priv->phydev);

	ret = request_irq(priv->rx_irq, &nuport_mac_rx_interrupt,
				0, dev->name, dev);
	if (ret) {
		netdev_err(dev, "unable to request tx interrupt\n");
		goto out_tx_irq;
	}

	netif_start_queue(dev);

	nuport_mac_init_tx_ring(priv);

	ret = nuport_mac_init_rx_ring(dev);
	if (ret) {
		netdev_err(dev, "rx ring init failed\n");
		goto out_rx_skb;
	}

	nuport_mac_reset_tx_dma(priv);
	nuport_mac_reset_rx_dma(priv);

	/* Start RX DMA */
	spin_lock_irqsave(&priv->lock, flags);
	ret = nuport_mac_start_rx_dma(priv, priv->rx_skb[0]);
	spin_unlock_irqrestore(&priv->lock, flags);

	napi_enable(&priv->napi);

	return ret;

out_rx_skb:
	nuport_mac_free_rx_ring(priv);
	free_irq(priv->rx_irq, dev);
out_tx_irq:
	free_irq(priv->tx_irq, dev);
out_link_irq:
	free_irq(priv->link_irq, dev);
out_emac_clk:
	clk_disable(priv->emac_clk);
	return ret;
}

static int nuport_mac_close(struct net_device *dev)
{
	u32 reg;
	struct nuport_mac_priv *priv = netdev_priv(dev);

	spin_lock_irq(&priv->lock);
	reg = nuport_mac_readl(CTRL_REG);
	reg &= ~(RX_ENABLE | TX_ENABLE);
	nuport_mac_writel(reg, CTRL_REG);

	napi_disable(&priv->napi);
	netif_stop_queue(dev);

	free_irq(priv->link_irq, dev);
	/* disable PHY polling */
	nuport_mac_writel(0, LINK_INT_CSR);
	nuport_mac_writel(0, LINK_INT_POLL_TIME);
	phy_stop(priv->phydev);

	free_irq(priv->tx_irq, dev);
	free_irq(priv->rx_irq, dev);
	spin_unlock_irq(&priv->lock);

	nuport_mac_free_rx_ring(priv);

	clk_disable(priv->emac_clk);

	return 0;
}

static void nuport_mac_tx_timeout(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	unsigned int i;

	netdev_warn(dev, "transmit timeout, attempting recovery\n");

	netdev_info(dev, "TX DMA regs\n");
	for (i = 0; i < DMA_CHAN_WIDTH; i += 4)
		netdev_info(dev, "[%02x]: 0x%08x\n", i, nuport_mac_readl(TX_DMA_BASE + i));
	netdev_info(dev, "RX DMA regs\n");
	for (i = 0; i < DMA_CHAN_WIDTH; i += 4)
		netdev_info(dev, "[%02x]: 0x%08x\n", i, nuport_mac_readl(RX_DMA_BASE + i));

	nuport_mac_init_tx_ring(priv);
	nuport_mac_reset_tx_dma(priv);

	netif_wake_queue(dev);
}

static int nuport_mac_mii_probe(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = NULL;
	int ret;

	ret = clk_enable(priv->ephy_clk);
	if (ret) {
		netdev_err(dev, "unable to enable ePHY clk\n");
		return ret;
	}

	phydev = phy_find_first(priv->mii_bus);
	if (!phydev) {
		netdev_err(dev, "no PHYs found\n");
		ret = -ENODEV;
		goto out;
	}

	phydev = of_phy_connect(dev, priv->phy_node,
			nuport_mac_adjust_link, 0,
			priv->phy_interface);
	if (IS_ERR(phydev)) {
		netdev_err(dev, "could not attach PHY\n");
		ret = PTR_ERR(phydev);
		goto out;
	}

	phydev->supported &= PHY_BASIC_FEATURES;
	phydev->advertising = phydev->supported;
	priv->phydev = phydev;
	priv->old_link = 1;
	priv->old_duplex = DUPLEX_FULL;

	dev_info(&priv->pdev->dev, "attached PHY driver [%s] "
		"(mii_bus:phy_addr=%d)\n",
		phydev->drv->name, phydev->addr);

	return 0;

out:
	/* disable the Ethernet PHY clock for the moment */
	clk_disable(priv->ephy_clk);

	return ret;
}

static void nuport_mac_ethtool_drvinfo(struct net_device *dev,
					struct ethtool_drvinfo *info)
{
	strncpy(info->driver, "nuport-mac", sizeof(info->driver));
	strncpy(info->version, "0.1", sizeof(info->version));
	strncpy(info->fw_version, "N/A", sizeof(info->fw_version));
	strncpy(info->bus_info, "internal", sizeof(info->bus_info));
	info->n_stats = 0;
	info->testinfo_len = 0;
	info->regdump_len = 0;
	info->eedump_len = 0;
}

static int nuport_mac_ethtool_get_settings(struct net_device *dev,
					struct ethtool_cmd *cmd)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);

	if (priv->phydev)
		return phy_ethtool_gset(priv->phydev, cmd);

	return -EINVAL;
}

static int nuport_mac_ethtool_set_settings(struct net_device *dev,
					struct ethtool_cmd *cmd)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);

	if (priv->phydev)
		return phy_ethtool_sset(priv->phydev, cmd);

	return -EINVAL;
}

static void nuport_mac_set_msglevel(struct net_device *dev, u32 msg_level)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);

	priv->msg_level = msg_level;
}

static u32 nuport_mac_get_msglevel(struct net_device *dev)
{
	struct nuport_mac_priv *priv = netdev_priv(dev);

	return priv->msg_level;
}

static const struct ethtool_ops nuport_mac_ethtool_ops = {
	.get_drvinfo		= nuport_mac_ethtool_drvinfo,
	.get_link		= ethtool_op_get_link,
	.get_settings		= nuport_mac_ethtool_get_settings,
	.set_settings		= nuport_mac_ethtool_set_settings,
	.set_msglevel		= nuport_mac_set_msglevel,
	.get_msglevel		= nuport_mac_get_msglevel,
};

static const struct net_device_ops nuport_mac_ops = {
	.ndo_open		= nuport_mac_open,
	.ndo_stop		= nuport_mac_close,
	.ndo_start_xmit		= nuport_mac_start_xmit,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= nuport_mac_change_mac_address,
	.ndo_tx_timeout		= nuport_mac_tx_timeout,
};

static int __init nuport_mac_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct nuport_mac_priv *priv = NULL;
	struct resource *regs, *dma;
	int ret = 0;
	int rx_irq, tx_irq, link_irq;
	int i;
	const unsigned int *intspec;

	dev = alloc_etherdev(sizeof(struct nuport_mac_priv));
	if (!dev) {
		dev_err(&pdev->dev, "no memory for net_device\n");
		return -ENOMEM;
	}

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dma = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!regs || !dma) {
		dev_err(&pdev->dev, "failed to get regs resources\n");
		ret = -ENODEV;
		goto out;
	}

	rx_irq = platform_get_irq(pdev, 0);
	tx_irq = platform_get_irq(pdev, 1);
	link_irq = platform_get_irq(pdev, 2);
	if (rx_irq < 0 || tx_irq < 0 || link_irq < 0) {
		ret = -ENODEV;
		goto out;
	}

	platform_set_drvdata(pdev, dev);
	SET_NETDEV_DEV(dev, &pdev->dev);
	priv = netdev_priv(dev);
	priv->pdev = pdev;
	priv->dev = dev;
	spin_lock_init(&priv->lock);

	intspec = of_get_property(pdev->dev.of_node,
				"nuport-mac,buffer-shifting", NULL);
	if (!intspec)
		priv->buffer_shifting_len = 0;
	else
		priv->buffer_shifting_len = 2;

	priv->mac_base = devm_ioremap_resource(&pdev->dev, regs);
	if (!priv->mac_base) {
		dev_err(&pdev->dev, "failed to remap regs\n");
		ret = -ENOMEM;
		goto out_platform;
	}

	priv->dma_base = devm_ioremap_resource(&pdev->dev, dma);
	if (!priv->dma_base) {
		dev_err(&pdev->dev, "failed to remap dma-regs\n");
		ret = -ENOMEM;
		goto out_platform;
	}

	priv->emac_clk = clk_get(&pdev->dev, "emac");
	if (IS_ERR_OR_NULL(priv->emac_clk)) {
		dev_err(&pdev->dev, "failed to get emac clk\n");
		ret = PTR_ERR(priv->emac_clk);
		goto out_platform;
	}

	priv->ephy_clk = clk_get(&pdev->dev, "ephy");
	if (IS_ERR_OR_NULL(priv->ephy_clk)) {
		dev_err(&pdev->dev, "failed to get ephy clk\n");
		ret = PTR_ERR(priv->ephy_clk);
		goto out_platform;
	}

	priv->link_irq = link_irq;
	priv->rx_irq = rx_irq;
	priv->tx_irq = tx_irq;
	priv->msg_level = NETIF_MSG_DRV | NETIF_MSG_PROBE | NETIF_MSG_LINK;
	dev->netdev_ops = &nuport_mac_ops;
	dev->ethtool_ops = &nuport_mac_ethtool_ops;
	dev->watchdog_timeo = HZ;
	dev->flags = IFF_BROADCAST;	/* Supports Broadcast */
	dev->tx_queue_len = TX_RING_SIZE / 2;

	netif_napi_add(dev, &priv->napi, nuport_mac_poll, 64);

	priv->phy_node = of_parse_phandle(pdev->dev.of_node, "phy", 0);
	if (!priv->phy_node) {
		dev_err(&pdev->dev, "no associated PHY\n");
		ret = -ENODEV;
		goto out;
	}

	priv->phy_interface = of_get_phy_mode(pdev->dev.of_node);
	if (priv->phy_interface < 0) {
		dev_err(&pdev->dev, "invalid PHY mode\n");
		ret = -EINVAL;
		goto out;
	}

	priv->mii_bus = mdiobus_alloc();
	if (!priv->mii_bus) {
		dev_err(&pdev->dev, "mii bus allocation failed\n");
		goto out;
	}

	priv->mii_bus->priv = dev;
	priv->mii_bus->read = nuport_mac_mii_read;
	priv->mii_bus->write = nuport_mac_mii_write;
	priv->mii_bus->reset = nuport_mac_mii_reset;
	priv->mii_bus->name = "nuport-mac-mii";
	priv->mii_bus->phy_mask = (1 << 0);
	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%s", pdev->name);
	priv->mii_bus->irq = kzalloc(PHY_MAX_ADDR * sizeof(int), GFP_KERNEL);
	if (!priv->mii_bus->irq) {
		dev_err(&pdev->dev, "failed to allocate mii_bus irqs\n");
		ret = -ENOMEM;
		goto out_mdio;
	}

	/* We support PHY interrupts routed back to the MAC */
	for (i = 0; i < PHY_MAX_ADDR; i++)
		priv->mii_bus->irq[i] = PHY_IGNORE_INTERRUPT;

	ret = of_mdiobus_register(priv->mii_bus, pdev->dev.of_node);
	if (ret) {
		dev_err(&pdev->dev, "failed to register mii_bus\n");
		goto out_mdio_irq;
	}

	ret = nuport_mac_mii_probe(dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to probe MII bus\n");
		goto out_mdio_unregister;
	}

	ret = register_netdev(dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to register net_device\n");
		goto out_mdio_probe;
	}

	/* read existing mac address */
	nuport_mac_read_mac_address(dev);

	dev_info(&pdev->dev, "registered (MAC: %pM)\n", dev->dev_addr);

	return ret;

out_mdio_probe:
	phy_disconnect(priv->phydev);
out_mdio_unregister:
	mdiobus_unregister(priv->mii_bus);
out_mdio_irq:
	kfree(priv->mii_bus->irq);
out_mdio:
	mdiobus_free(priv->mii_bus);
out_platform:
	platform_set_drvdata(pdev, NULL);
out:
	clk_put(priv->ephy_clk);
	clk_put(priv->emac_clk);
	free_netdev(dev);
	platform_set_drvdata(pdev, NULL);
	return ret;
}

static int nuport_mac_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct nuport_mac_priv *priv = netdev_priv(dev);

	unregister_netdev(dev);
	phy_disconnect(priv->phydev);
	mdiobus_unregister(priv->mii_bus);
	kfree(priv->mii_bus->irq);
	mdiobus_free(priv->mii_bus);
	clk_put(priv->ephy_clk);
	clk_put(priv->emac_clk);
	free_netdev(dev);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct of_device_id nuport_eth_ids[] __initdata = {
	{.compatible = "moschip,nuport-mac",},
	{ /* sentinel */ },
};

static struct platform_driver nuport_eth_driver = {
	.driver = {
		.name = "nuport-mac",
		.owner = THIS_MODULE,
		.of_match_table = nuport_eth_ids,
	},
	.probe	= nuport_mac_probe,
	.remove	= nuport_mac_remove,
};

module_platform_driver(nuport_eth_driver);

MODULE_AUTHOR("Moschip Semiconductors Ltd.");
MODULE_DESCRIPTION("Moschip MCS8140 Ethernet MAC driver");
MODULE_LICENSE("GPL");
