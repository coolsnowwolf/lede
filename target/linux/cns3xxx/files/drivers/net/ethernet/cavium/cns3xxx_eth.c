/*
 * Cavium CNS3xxx Gigabit driver for Linux
 *
 * Copyright 2011 Gateworks Corporation
 *		  Chris Lang <clang@gateworks.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License
 * as published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/platform_data/cns3xxx.h>
#include <linux/skbuff.h>

#define DRV_NAME "cns3xxx_eth"

#define RX_DESCS 256
#define TX_DESCS 128
#define TX_DESC_RESERVE	20

#define RX_POOL_ALLOC_SIZE (sizeof(struct rx_desc) * RX_DESCS)
#define TX_POOL_ALLOC_SIZE (sizeof(struct tx_desc) * TX_DESCS)
#define REGS_SIZE 336

#define RX_BUFFER_ALIGN 64
#define RX_BUFFER_ALIGN_MASK (~(RX_BUFFER_ALIGN - 1))

#define SKB_HEAD_ALIGN (((PAGE_SIZE - NET_SKB_PAD) % RX_BUFFER_ALIGN) + NET_SKB_PAD + NET_IP_ALIGN)
#define RX_SEGMENT_ALLOC_SIZE 2048
#define RX_SEGMENT_BUFSIZE (SKB_WITH_OVERHEAD(RX_SEGMENT_ALLOC_SIZE))
#define RX_SEGMENT_MRU (((RX_SEGMENT_BUFSIZE - SKB_HEAD_ALIGN) & RX_BUFFER_ALIGN_MASK) - NET_IP_ALIGN)
#define MAX_MTU	9500

#define NAPI_WEIGHT 64

/* MDIO Defines */
#define MDIO_CMD_COMPLETE 0x00008000
#define MDIO_WRITE_COMMAND 0x00002000
#define MDIO_READ_COMMAND 0x00004000
#define MDIO_REG_OFFSET 8
#define MDIO_VALUE_OFFSET 16

/* Descritor Defines */
#define END_OF_RING 0x40000000
#define FIRST_SEGMENT 0x20000000
#define LAST_SEGMENT 0x10000000
#define FORCE_ROUTE 0x04000000
#define UDP_CHECKSUM 0x00020000
#define TCP_CHECKSUM 0x00010000

/* Port Config Defines */
#define PORT_BP_ENABLE 0x00020000
#define PORT_DISABLE 0x00040000
#define PORT_LEARN_DIS 0x00080000
#define PORT_BLOCK_STATE 0x00100000
#define PORT_BLOCK_MODE 0x00200000

#define PROMISC_OFFSET 29

/* Global Config Defines */
#define UNKNOWN_VLAN_TO_CPU 0x02000000
#define ACCEPT_CRC_PACKET 0x00200000
#define CRC_STRIPPING 0x00100000

/* VLAN Config Defines */
#define NIC_MODE 0x00008000
#define VLAN_UNAWARE 0x00000001

/* DMA AUTO Poll Defines */
#define TS_POLL_EN 0x00000020
#define TS_SUSPEND 0x00000010
#define FS_POLL_EN 0x00000002
#define FS_SUSPEND 0x00000001

/* DMA Ring Control Defines */
#define QUEUE_THRESHOLD 0x000000f0
#define CLR_FS_STATE 0x80000000

/* Interrupt Status Defines */
#define MAC0_STATUS_CHANGE 0x00004000
#define MAC1_STATUS_CHANGE 0x00008000
#define MAC2_STATUS_CHANGE 0x00010000
#define MAC0_RX_ERROR 0x00100000
#define MAC1_RX_ERROR 0x00200000
#define MAC2_RX_ERROR 0x00400000

struct tx_desc
{
	u32 sdp; /* segment data pointer */

	union {
		struct {
			u32 sdl:16; /* segment data length */
			u32 tco:1;
			u32 uco:1;
			u32 ico:1;
			u32 rsv_1:3; /* reserve */
			u32 pri:3;
			u32 fp:1; /* force priority */
			u32 fr:1;
			u32 interrupt:1;
			u32 lsd:1;
			u32 fsd:1;
			u32 eor:1;
			u32 cown:1;
		};
		u32 config0;
	};

	union {
		struct {
			u32 ctv:1;
			u32 stv:1;
			u32 sid:4;
			u32 inss:1;
			u32 dels:1;
			u32 rsv_2:9;
			u32 pmap:5;
			u32 mark:3;
			u32 ewan:1;
			u32 fewan:1;
			u32 rsv_3:5;
		};
		u32 config1;
	};

	union {
		struct {
			u32 c_vid:12;
			u32 c_cfs:1;
			u32 c_pri:3;
			u32 s_vid:12;
			u32 s_dei:1;
			u32 s_pri:3;
		};
		u32 config2;
	};

	u8 alignment[16]; /* for 32 byte */
};

struct rx_desc
{
	u32 sdp; /* segment data pointer */

	union {
		struct {
			u32 sdl:16; /* segment data length */
			u32 l4f:1;
			u32 ipf:1;
			u32 prot:4;
			u32 hr:6;
			u32 lsd:1;
			u32 fsd:1;
			u32 eor:1;
			u32 cown:1;
		};
		u32 config0;
	};

	union {
		struct {
			u32 ctv:1;
			u32 stv:1;
			u32 unv:1;
			u32 iwan:1;
			u32 exdv:1;
			u32 e_wan:1;
			u32 rsv_1:2;
			u32 sp:3;
			u32 crc_err:1;
			u32 un_eth:1;
			u32 tc:2;
			u32 rsv_2:1;
			u32 ip_offset:5;
			u32 rsv_3:11;
		};
		u32 config1;
	};

	union {
		struct {
			u32 c_vid:12;
			u32 c_cfs:1;
			u32 c_pri:3;
			u32 s_vid:12;
			u32 s_dei:1;
			u32 s_pri:3;
		};
		u32 config2;
	};

	u8 alignment[16]; /* for 32 byte alignment */
};


struct switch_regs {
	u32 phy_control;
	u32 phy_auto_addr;
	u32 mac_glob_cfg;
	u32 mac_cfg[4];
	u32 mac_pri_ctrl[5], __res;
	u32 etype[2];
	u32 udp_range[4];
	u32 prio_etype_udp;
	u32 prio_ipdscp[8];
	u32 tc_ctrl;
	u32 rate_ctrl;
	u32 fc_glob_thrs;
	u32 fc_port_thrs;
	u32 mc_fc_glob_thrs;
	u32 dc_glob_thrs;
	u32 arl_vlan_cmd;
	u32 arl_ctrl[3];
	u32 vlan_cfg;
	u32 pvid[2];
	u32 vlan_ctrl[3];
	u32 session_id[8];
	u32 intr_stat;
	u32 intr_mask;
	u32 sram_test;
	u32 mem_queue;
	u32 farl_ctrl;
	u32 fc_input_thrs, __res1[2];
	u32 clk_skew_ctrl;
	u32 mac_glob_cfg_ext, __res2[2];
	u32 dma_ring_ctrl;
	u32 dma_auto_poll_cfg;
	u32 delay_intr_cfg, __res3;
	u32 ts_dma_ctrl0;
	u32 ts_desc_ptr0;
	u32 ts_desc_base_addr0, __res4;
	u32 fs_dma_ctrl0;
	u32 fs_desc_ptr0;
	u32 fs_desc_base_addr0, __res5;
	u32 ts_dma_ctrl1;
	u32 ts_desc_ptr1;
	u32 ts_desc_base_addr1, __res6;
	u32 fs_dma_ctrl1;
	u32 fs_desc_ptr1;
	u32 fs_desc_base_addr1;
	u32 __res7[109];
	u32 mac_counter0[13];
};

struct _tx_ring {
	struct tx_desc *desc;
	dma_addr_t phys_addr;
	struct tx_desc *cur_addr;
	struct sk_buff *buff_tab[TX_DESCS];
	unsigned int phys_tab[TX_DESCS];
	u32 free_index;
	u32 count_index;
	u32 cur_index;
	int num_used;
	int num_count;
	bool stopped;
};

struct _rx_ring {
	struct rx_desc *desc;
	dma_addr_t phys_addr;
	struct rx_desc *cur_addr;
	void *buff_tab[RX_DESCS];
	unsigned int phys_tab[RX_DESCS];
	u32 cur_index;
	u32 alloc_index;
	int alloc_count;
};

struct sw {
	struct switch_regs __iomem *regs;
	struct napi_struct napi;
	struct cns3xxx_plat_info *plat;
	struct _tx_ring tx_ring;
	struct _rx_ring rx_ring;
	struct sk_buff *frag_first;
	struct sk_buff *frag_last;
	struct device *dev;
	int rx_irq;
	int stat_irq;
};

struct port {
	struct net_device *netdev;
	struct phy_device *phydev;
	struct sw *sw;
	int id;			/* logical port ID */
	int speed, duplex;
};

static spinlock_t mdio_lock;
static DEFINE_SPINLOCK(tx_lock);
static struct switch_regs __iomem *mdio_regs; /* mdio command and status only */
struct mii_bus *mdio_bus;
static int ports_open;
static struct port *switch_port_tab[4];
struct net_device *napi_dev;

static int cns3xxx_mdio_cmd(struct mii_bus *bus, int phy_id, int location,
			   int write, u16 cmd)
{
	int cycles = 0;
	u32 temp = 0;

	temp = __raw_readl(&mdio_regs->phy_control);
	temp |= MDIO_CMD_COMPLETE;
	__raw_writel(temp, &mdio_regs->phy_control);
	udelay(10);

	if (write) {
		temp = (cmd << MDIO_VALUE_OFFSET);
		temp |= MDIO_WRITE_COMMAND;
	} else {
		temp = MDIO_READ_COMMAND;
	}
	temp |= ((location & 0x1f) << MDIO_REG_OFFSET);
	temp |= (phy_id & 0x1f);

	__raw_writel(temp, &mdio_regs->phy_control);

	while (((__raw_readl(&mdio_regs->phy_control) & MDIO_CMD_COMPLETE) == 0)
			&& cycles < 5000) {
		udelay(1);
		cycles++;
	}

	if (cycles == 5000) {
		printk(KERN_ERR "%s #%i: MII transaction failed\n", bus->name,
		       phy_id);
		return -1;
	}

	temp = __raw_readl(&mdio_regs->phy_control);
	temp |= MDIO_CMD_COMPLETE;
	__raw_writel(temp, &mdio_regs->phy_control);

	if (write)
		return 0;

	return ((temp >> MDIO_VALUE_OFFSET) & 0xFFFF);
}

static int cns3xxx_mdio_read(struct mii_bus *bus, int phy_id, int location)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&mdio_lock, flags);
	ret = cns3xxx_mdio_cmd(bus, phy_id, location, 0, 0);
	spin_unlock_irqrestore(&mdio_lock, flags);
	return ret;
}

static int cns3xxx_mdio_write(struct mii_bus *bus, int phy_id, int location,
			     u16 val)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&mdio_lock, flags);
	ret = cns3xxx_mdio_cmd(bus, phy_id, location, 1, val);
	spin_unlock_irqrestore(&mdio_lock, flags);
	return ret;
}

static int cns3xxx_mdio_register(void __iomem *base)
{
	int err;

	if (!(mdio_bus = mdiobus_alloc()))
		return -ENOMEM;

	mdio_regs = base;

	spin_lock_init(&mdio_lock);
	mdio_bus->name = "CNS3xxx MII Bus";
	mdio_bus->read = &cns3xxx_mdio_read;
	mdio_bus->write = &cns3xxx_mdio_write;
	strcpy(mdio_bus->id, "0");

	if ((err = mdiobus_register(mdio_bus)))
		mdiobus_free(mdio_bus);
	return err;
}

static void cns3xxx_mdio_remove(void)
{
	mdiobus_unregister(mdio_bus);
	mdiobus_free(mdio_bus);
}

static void enable_tx_dma(struct sw *sw)
{
	__raw_writel(0x1, &sw->regs->ts_dma_ctrl0);
}

static void enable_rx_dma(struct sw *sw)
{
	__raw_writel(0x1, &sw->regs->fs_dma_ctrl0);
}

static void cns3xxx_adjust_link(struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	struct phy_device *phydev = port->phydev;

	if (!phydev->link) {
		if (port->speed) {
			port->speed = 0;
			printk(KERN_INFO "%s: link down\n", dev->name);
		}
		return;
	}

	if (port->speed == phydev->speed && port->duplex == phydev->duplex)
		return;

	port->speed = phydev->speed;
	port->duplex = phydev->duplex;

	printk(KERN_INFO "%s: link up, speed %u Mb/s, %s duplex\n",
	       dev->name, port->speed, port->duplex ? "full" : "half");
}

static void eth_schedule_poll(struct sw *sw)
{
	if (unlikely(!napi_schedule_prep(&sw->napi)))
		return;

	disable_irq_nosync(sw->rx_irq);
	__napi_schedule(&sw->napi);
}

irqreturn_t eth_rx_irq(int irq, void *pdev)
{
	struct net_device *dev = pdev;
	struct sw *sw = netdev_priv(dev);
	eth_schedule_poll(sw);
	return (IRQ_HANDLED);
}

irqreturn_t eth_stat_irq(int irq, void *pdev)
{
	struct net_device *dev = pdev;
	struct sw *sw = netdev_priv(dev);
	u32 cfg;
	u32 stat = __raw_readl(&sw->regs->intr_stat);
	__raw_writel(0xffffffff, &sw->regs->intr_stat);

	if (stat & MAC2_RX_ERROR)
		switch_port_tab[3]->netdev->stats.rx_dropped++;
	if (stat & MAC1_RX_ERROR)
		switch_port_tab[1]->netdev->stats.rx_dropped++;
	if (stat & MAC0_RX_ERROR)
		switch_port_tab[0]->netdev->stats.rx_dropped++;

	if (stat & MAC0_STATUS_CHANGE) {
		cfg = __raw_readl(&sw->regs->mac_cfg[0]);
		switch_port_tab[0]->phydev->link = (cfg & 0x1);
		switch_port_tab[0]->phydev->duplex = ((cfg >> 4) & 0x1);
		if (((cfg >> 2) & 0x3) == 2)
			switch_port_tab[0]->phydev->speed = 1000;
		else if (((cfg >> 2) & 0x3) == 1)
			switch_port_tab[0]->phydev->speed = 100;
		else
			switch_port_tab[0]->phydev->speed = 10;
		cns3xxx_adjust_link(switch_port_tab[0]->netdev);
	}

	if (stat & MAC1_STATUS_CHANGE) {
		cfg = __raw_readl(&sw->regs->mac_cfg[1]);
		switch_port_tab[1]->phydev->link = (cfg & 0x1);
		switch_port_tab[1]->phydev->duplex = ((cfg >> 4) & 0x1);
		if (((cfg >> 2) & 0x3) == 2)
			switch_port_tab[1]->phydev->speed = 1000;
		else if (((cfg >> 2) & 0x3) == 1)
			switch_port_tab[1]->phydev->speed = 100;
		else
			switch_port_tab[1]->phydev->speed = 10;
		cns3xxx_adjust_link(switch_port_tab[1]->netdev);
	}

	if (stat & MAC2_STATUS_CHANGE) {
		cfg = __raw_readl(&sw->regs->mac_cfg[3]);
		switch_port_tab[3]->phydev->link = (cfg & 0x1);
		switch_port_tab[3]->phydev->duplex = ((cfg >> 4) & 0x1);
		if (((cfg >> 2) & 0x3) == 2)
			switch_port_tab[3]->phydev->speed = 1000;
		else if (((cfg >> 2) & 0x3) == 1)
			switch_port_tab[3]->phydev->speed = 100;
		else
			switch_port_tab[3]->phydev->speed = 10;
		cns3xxx_adjust_link(switch_port_tab[3]->netdev);
	}

	return (IRQ_HANDLED);
}


static void cns3xxx_alloc_rx_buf(struct sw *sw, int received)
{
	struct _rx_ring *rx_ring = &sw->rx_ring;
	unsigned int i = rx_ring->alloc_index;
	struct rx_desc *desc = &(rx_ring)->desc[i];
	void *buf;
	unsigned int phys;

	for (received += rx_ring->alloc_count; received > 0; received--) {
		buf = napi_alloc_frag(RX_SEGMENT_ALLOC_SIZE);
		if (!buf)
			break;

		phys = dma_map_single(sw->dev, buf + SKB_HEAD_ALIGN,
				      RX_SEGMENT_MRU, DMA_FROM_DEVICE);
		if (dma_mapping_error(sw->dev, phys)) {
			skb_free_frag(buf);
			break;
		}

		desc->sdl = RX_SEGMENT_MRU;
		desc->sdp = phys;

		wmb();

		/* put the new buffer on RX-free queue */
		rx_ring->buff_tab[i] = buf;
		rx_ring->phys_tab[i] = phys;
		if (i == RX_DESCS - 1) {
			i = 0;
			desc->config0 = END_OF_RING | FIRST_SEGMENT |
					LAST_SEGMENT | RX_SEGMENT_MRU;
			desc = &(rx_ring)->desc[i];
		} else {
			desc->config0 = FIRST_SEGMENT | LAST_SEGMENT |
					RX_SEGMENT_MRU;
			i++;
			desc++;
		}
	}

	rx_ring->alloc_count = received;
	rx_ring->alloc_index = i;
}

static void eth_check_num_used(struct _tx_ring *tx_ring)
{
	bool stop = false;
	int i;

	if (tx_ring->num_used >= TX_DESCS - TX_DESC_RESERVE)
		stop = true;

	if (tx_ring->stopped == stop)
		return;

	tx_ring->stopped = stop;
	for (i = 0; i < 4; i++) {
		struct port *port = switch_port_tab[i];
		struct net_device *dev;

		if (!port)
			continue;

		dev = port->netdev;
		if (stop)
			netif_stop_queue(dev);
		else
			netif_wake_queue(dev);
	}
}

static void eth_complete_tx(struct sw *sw)
{
	struct _tx_ring *tx_ring = &sw->tx_ring;
	struct tx_desc *desc;
	int i;
	int index;
	int num_used = tx_ring->num_used;
	struct sk_buff *skb;

	index = tx_ring->free_index;
	desc = &(tx_ring)->desc[index];
	for (i = 0; i < num_used; i++) {
		if (desc->cown) {
			skb = tx_ring->buff_tab[index];
			tx_ring->buff_tab[index] = 0;
			if (skb)
				dev_kfree_skb_any(skb);
			dma_unmap_single(sw->dev, tx_ring->phys_tab[index],
				desc->sdl, DMA_TO_DEVICE);
			if (++index == TX_DESCS) {
				index = 0;
				desc = &(tx_ring)->desc[index];
			} else {
				desc++;
			}
		} else {
			break;
		}
	}
	tx_ring->free_index = index;
	tx_ring->num_used -= i;
	eth_check_num_used(tx_ring);
}

static int eth_poll(struct napi_struct *napi, int budget)
{
	struct sw *sw = container_of(napi, struct sw, napi);
	struct _rx_ring *rx_ring = &sw->rx_ring;
	int received = 0;
	unsigned int length;
	unsigned int i = rx_ring->cur_index;
	struct rx_desc *desc = &(rx_ring)->desc[i];
	unsigned int alloc_count = rx_ring->alloc_count;

	while (desc->cown && alloc_count + received < RX_DESCS - 1) {
		struct sk_buff *skb;
		int reserve = SKB_HEAD_ALIGN;

		if (received >= budget)
			break;

		/* process received frame */
		dma_unmap_single(sw->dev, rx_ring->phys_tab[i],
				 RX_SEGMENT_MRU, DMA_FROM_DEVICE);

		skb = build_skb(rx_ring->buff_tab[i], RX_SEGMENT_ALLOC_SIZE);
		if (!skb)
			break;

		skb->dev = switch_port_tab[desc->sp]->netdev;

		length = desc->sdl;
		if (desc->fsd && !desc->lsd)
			length = RX_SEGMENT_MRU;

		if (!desc->fsd) {
			reserve -= NET_IP_ALIGN;
			if (!desc->lsd)
				length += NET_IP_ALIGN;
		}

		skb_reserve(skb, reserve);
		skb_put(skb, length);

		if (!sw->frag_first)
			sw->frag_first = skb;
		else {
			if (sw->frag_first == sw->frag_last)
				skb_shinfo(sw->frag_first)->frag_list = skb;
			else
				sw->frag_last->next = skb;
			sw->frag_first->len += skb->len;
			sw->frag_first->data_len += skb->len;
			sw->frag_first->truesize += skb->truesize;
		}
		sw->frag_last = skb;

		if (desc->lsd) {
			struct net_device *dev;

			skb = sw->frag_first;
			dev = skb->dev;
			skb->protocol = eth_type_trans(skb, dev);

			dev->stats.rx_packets++;
			dev->stats.rx_bytes += skb->len;

			/* RX Hardware checksum offload */
			skb->ip_summed = CHECKSUM_NONE;
			switch (desc->prot) {
				case 1:
				case 2:
				case 5:
				case 6:
				case 13:
				case 14:
					if (!desc->l4f) {
						skb->ip_summed = CHECKSUM_UNNECESSARY;
						napi_gro_receive(napi, skb);
						break;
					}
					/* fall through */
				default:
					netif_receive_skb(skb);
					break;
			}

			sw->frag_first = NULL;
			sw->frag_last = NULL;
		}

		received++;
		if (++i == RX_DESCS) {
			i = 0;
			desc = &(rx_ring)->desc[i];
		} else {
			desc++;
		}
	}

	rx_ring->cur_index = i;

	cns3xxx_alloc_rx_buf(sw, received);
	wmb();
	enable_rx_dma(sw);

	if (received < budget && napi_complete_done(napi, received)) {
		enable_irq(sw->rx_irq);
	}

	spin_lock_bh(&tx_lock);
	eth_complete_tx(sw);
	spin_unlock_bh(&tx_lock);

	return received;
}

static void eth_set_desc(struct sw *sw, struct _tx_ring *tx_ring, int index,
			 int index_last, void *data, int len, u32 config0,
			 u32 pmap)
{
	struct tx_desc *tx_desc = &(tx_ring)->desc[index];
	unsigned int phys;

	phys = dma_map_single(sw->dev, data, len, DMA_TO_DEVICE);
	tx_desc->sdp = phys;
	tx_desc->pmap = pmap;
	tx_ring->phys_tab[index] = phys;

	config0 |= len;
	if (index == TX_DESCS - 1)
		config0 |= END_OF_RING;
	if (index == index_last)
		config0 |= LAST_SEGMENT;

	wmb();
	tx_desc->config0 = config0;
}

static int eth_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	struct sw *sw = port->sw;
	struct _tx_ring *tx_ring = &sw->tx_ring;
	struct sk_buff *skb1;
	char pmap = (1 << port->id);
	int nr_frags = skb_shinfo(skb)->nr_frags;
	int nr_desc = nr_frags;
	int index0, index, index_last;
	int len0;
	unsigned int i;
	u32 config0;

	if (pmap == 8)
		pmap = (1 << 4);

	skb_walk_frags(skb, skb1)
		nr_desc++;

	eth_schedule_poll(sw);
	spin_lock_bh(&tx_lock);
	if ((tx_ring->num_used + nr_desc + 1) >= TX_DESCS) {
		spin_unlock_bh(&tx_lock);
		return NETDEV_TX_BUSY;
	}

	index = index0 = tx_ring->cur_index;
	index_last = (index0 + nr_desc) % TX_DESCS;
	tx_ring->cur_index = (index_last + 1) % TX_DESCS;

	spin_unlock_bh(&tx_lock);

	config0 = FORCE_ROUTE;
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		config0 |= UDP_CHECKSUM | TCP_CHECKSUM;

	len0 = skb->len;

	/* fragments */
	for (i = 0; i < nr_frags; i++) {
		struct skb_frag_struct *frag;
		void *addr;

		index = (index + 1) % TX_DESCS;

		frag = &skb_shinfo(skb)->frags[i];
		addr = page_address(skb_frag_page(frag)) + frag->page_offset;

		eth_set_desc(sw, tx_ring, index, index_last, addr, frag->size,
			     config0, pmap);
	}

	if (nr_frags)
		len0 = skb->len - skb->data_len;

	skb_walk_frags(skb, skb1) {
		index = (index + 1) % TX_DESCS;
		len0 -= skb1->len;

		eth_set_desc(sw, tx_ring, index, index_last, skb1->data,
			     skb1->len, config0, pmap);
	}

	tx_ring->buff_tab[index0] = skb;
	eth_set_desc(sw, tx_ring, index0, index_last, skb->data, len0,
		     config0 | FIRST_SEGMENT, pmap);

	wmb();

	spin_lock(&tx_lock);
	tx_ring->num_used += nr_desc + 1;
	spin_unlock(&tx_lock);

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += skb->len;

	enable_tx_dma(sw);

	return NETDEV_TX_OK;
}

static int eth_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
	struct port *port = netdev_priv(dev);

	if (!netif_running(dev))
		return -EINVAL;
	return phy_mii_ioctl(port->phydev, req, cmd);
}

/* ethtool support */

static void cns3xxx_get_drvinfo(struct net_device *dev,
			       struct ethtool_drvinfo *info)
{
	strcpy(info->driver, DRV_NAME);
	strcpy(info->bus_info, "internal");
}

static int cns3xxx_nway_reset(struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	return phy_start_aneg(port->phydev);
}

static struct ethtool_ops cns3xxx_ethtool_ops = {
	.get_drvinfo = cns3xxx_get_drvinfo,
	.get_link_ksettings = phy_ethtool_get_link_ksettings,
	.set_link_ksettings = phy_ethtool_set_link_ksettings,
	.nway_reset = cns3xxx_nway_reset,
	.get_link = ethtool_op_get_link,
};


static int init_rings(struct sw *sw)
{
	int i;
	struct _rx_ring *rx_ring = &sw->rx_ring;
	struct _tx_ring *tx_ring = &sw->tx_ring;

	__raw_writel(0, &sw->regs->fs_dma_ctrl0);
	__raw_writel(TS_SUSPEND | FS_SUSPEND, &sw->regs->dma_auto_poll_cfg);
	__raw_writel(QUEUE_THRESHOLD, &sw->regs->dma_ring_ctrl);
	__raw_writel(CLR_FS_STATE | QUEUE_THRESHOLD, &sw->regs->dma_ring_ctrl);

	__raw_writel(QUEUE_THRESHOLD, &sw->regs->dma_ring_ctrl);

	rx_ring->desc = dmam_alloc_coherent(sw->dev, RX_POOL_ALLOC_SIZE,
					    &rx_ring->phys_addr, GFP_KERNEL);
	if (!rx_ring->desc)
		return -ENOMEM;

	/* Setup RX buffers */
	memset(rx_ring->desc, 0, RX_POOL_ALLOC_SIZE);
	for (i = 0; i < RX_DESCS; i++) {
		struct rx_desc *desc = &(rx_ring)->desc[i];
		void *buf;

		buf = netdev_alloc_frag(RX_SEGMENT_ALLOC_SIZE);
		if (!buf)
			return -ENOMEM;

		desc->sdl = RX_SEGMENT_MRU;
		if (i == (RX_DESCS - 1))
			desc->eor = 1;
		desc->fsd = 1;
		desc->lsd = 1;

		desc->sdp = dma_map_single(sw->dev, buf + SKB_HEAD_ALIGN,
					   RX_SEGMENT_MRU, DMA_FROM_DEVICE);
		if (dma_mapping_error(sw->dev, desc->sdp))
			return -EIO;

		rx_ring->buff_tab[i] = buf;
		rx_ring->phys_tab[i] = desc->sdp;
		desc->cown = 0;
	}
	__raw_writel(rx_ring->phys_addr, &sw->regs->fs_desc_ptr0);
	__raw_writel(rx_ring->phys_addr, &sw->regs->fs_desc_base_addr0);

	tx_ring->desc = dmam_alloc_coherent(sw->dev, TX_POOL_ALLOC_SIZE,
					    &tx_ring->phys_addr, GFP_KERNEL);
	if (!tx_ring->desc)
		return -ENOMEM;

	/* Setup TX buffers */
	memset(tx_ring->desc, 0, TX_POOL_ALLOC_SIZE);
	for (i = 0; i < TX_DESCS; i++) {
		struct tx_desc *desc = &(tx_ring)->desc[i];
		tx_ring->buff_tab[i] = 0;

		if (i == (TX_DESCS - 1))
			desc->eor = 1;
		desc->cown = 1;
	}
	__raw_writel(tx_ring->phys_addr, &sw->regs->ts_desc_ptr0);
	__raw_writel(tx_ring->phys_addr, &sw->regs->ts_desc_base_addr0);

	return 0;
}

static void destroy_rings(struct sw *sw)
{
	int i;

	for (i = 0; i < RX_DESCS; i++) {
		struct _rx_ring *rx_ring = &sw->rx_ring;
		struct rx_desc *desc = &(rx_ring)->desc[i];
		void *buf = sw->rx_ring.buff_tab[i];

		if (!buf)
			continue;

		dma_unmap_single(sw->dev, desc->sdp, RX_SEGMENT_MRU,
				 DMA_FROM_DEVICE);
		skb_free_frag(buf);
	}

	for (i = 0; i < TX_DESCS; i++) {
		struct _tx_ring *tx_ring = &sw->tx_ring;
		struct tx_desc *desc = &(tx_ring)->desc[i];
		struct sk_buff *skb = sw->tx_ring.buff_tab[i];

		if (!skb)
			continue;

		dma_unmap_single(sw->dev, desc->sdp, skb->len, DMA_TO_DEVICE);
		dev_kfree_skb(skb);
	}
}

static int eth_open(struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	struct sw *sw = port->sw;
	u32 temp;

	port->speed = 0;	/* force "link up" message */
	phy_start(port->phydev);

	netif_start_queue(dev);

	if (!ports_open) {
		request_irq(sw->rx_irq, eth_rx_irq, IRQF_SHARED, "gig_switch", napi_dev);
		request_irq(sw->stat_irq, eth_stat_irq, IRQF_SHARED, "gig_stat", napi_dev);
		napi_enable(&sw->napi);
		netif_start_queue(napi_dev);

 		__raw_writel(~(MAC0_STATUS_CHANGE | MAC1_STATUS_CHANGE | MAC2_STATUS_CHANGE |
 			       MAC0_RX_ERROR | MAC1_RX_ERROR | MAC2_RX_ERROR), &sw->regs->intr_mask);

		temp = __raw_readl(&sw->regs->mac_cfg[2]);
		temp &= ~(PORT_DISABLE);
		__raw_writel(temp, &sw->regs->mac_cfg[2]);

		temp = __raw_readl(&sw->regs->dma_auto_poll_cfg);
		temp &= ~(TS_SUSPEND | FS_SUSPEND);
		__raw_writel(temp, &sw->regs->dma_auto_poll_cfg);

		enable_rx_dma(sw);
	}
	temp = __raw_readl(&sw->regs->mac_cfg[port->id]);
	temp &= ~(PORT_DISABLE);
	__raw_writel(temp, &sw->regs->mac_cfg[port->id]);

	ports_open++;
	netif_carrier_on(dev);

	return 0;
}

static int eth_close(struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	struct sw *sw = port->sw;
	u32 temp;

	ports_open--;

	temp = __raw_readl(&sw->regs->mac_cfg[port->id]);
	temp |= (PORT_DISABLE);
	__raw_writel(temp, &sw->regs->mac_cfg[port->id]);

	netif_stop_queue(dev);

	phy_stop(port->phydev);

	if (!ports_open) {
		disable_irq(sw->rx_irq);
		free_irq(sw->rx_irq, napi_dev);
		disable_irq(sw->stat_irq);
		free_irq(sw->stat_irq, napi_dev);
		napi_disable(&sw->napi);
		netif_stop_queue(napi_dev);
		temp = __raw_readl(&sw->regs->mac_cfg[2]);
		temp |= (PORT_DISABLE);
		__raw_writel(temp, &sw->regs->mac_cfg[2]);

		__raw_writel(TS_SUSPEND | FS_SUSPEND,
			     &sw->regs->dma_auto_poll_cfg);
	}

	netif_carrier_off(dev);
	return 0;
}

static void eth_rx_mode(struct net_device *dev)
{
	struct port *port = netdev_priv(dev);
	struct sw *sw = port->sw;
	u32 temp;

	temp = __raw_readl(&sw->regs->mac_glob_cfg);

	if (dev->flags & IFF_PROMISC) {
		if (port->id == 3)
			temp |= ((1 << 2) << PROMISC_OFFSET);
		else
			temp |= ((1 << port->id) << PROMISC_OFFSET);
	} else {
		if (port->id == 3)
			temp &= ~((1 << 2) << PROMISC_OFFSET);
		else
			temp &= ~((1 << port->id) << PROMISC_OFFSET);
	}
	__raw_writel(temp, &sw->regs->mac_glob_cfg);
}

static int eth_set_mac(struct net_device *netdev, void *p)
{
	struct port *port = netdev_priv(netdev);
	struct sw *sw = port->sw;
	struct sockaddr *addr = p;
	u32 cycles = 0;

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	/* Invalidate old ARL Entry */
	if (port->id == 3)
		__raw_writel((port->id << 16) | (0x4 << 9), &sw->regs->arl_ctrl[0]);
	else
		__raw_writel(((port->id + 1) << 16) | (0x4 << 9), &sw->regs->arl_ctrl[0]);
	__raw_writel( ((netdev->dev_addr[0] << 24) | (netdev->dev_addr[1] << 16) |
			(netdev->dev_addr[2] << 8) | (netdev->dev_addr[3])),
			&sw->regs->arl_ctrl[1]);

	__raw_writel( ((netdev->dev_addr[4] << 24) | (netdev->dev_addr[5] << 16) |
			(1 << 1)),
			&sw->regs->arl_ctrl[2]);
	__raw_writel((1 << 19), &sw->regs->arl_vlan_cmd);

	while (((__raw_readl(&sw->regs->arl_vlan_cmd) & (1 << 21)) == 0)
			&& cycles < 5000) {
		udelay(1);
		cycles++;
	}

	cycles = 0;
	memcpy(netdev->dev_addr, addr->sa_data, netdev->addr_len);

	if (port->id == 3)
		__raw_writel((port->id << 16) | (0x4 << 9), &sw->regs->arl_ctrl[0]);
	else
		__raw_writel(((port->id + 1) << 16) | (0x4 << 9), &sw->regs->arl_ctrl[0]);
	__raw_writel( ((addr->sa_data[0] << 24) | (addr->sa_data[1] << 16) |
			(addr->sa_data[2] << 8) | (addr->sa_data[3])),
			&sw->regs->arl_ctrl[1]);

	__raw_writel( ((addr->sa_data[4] << 24) | (addr->sa_data[5] << 16) |
			(7 << 4) | (1 << 1)), &sw->regs->arl_ctrl[2]);
	__raw_writel((1 << 19), &sw->regs->arl_vlan_cmd);

	while (((__raw_readl(&sw->regs->arl_vlan_cmd) & (1 << 21)) == 0)
		&& cycles < 5000) {
		udelay(1);
		cycles++;
	}
	return 0;
}

static int cns3xxx_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu > MAX_MTU)
		return -EINVAL;

	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops cns3xxx_netdev_ops = {
	.ndo_open = eth_open,
	.ndo_stop = eth_close,
	.ndo_start_xmit = eth_xmit,
	.ndo_set_rx_mode = eth_rx_mode,
	.ndo_do_ioctl = eth_ioctl,
	.ndo_change_mtu = cns3xxx_change_mtu,
	.ndo_set_mac_address = eth_set_mac,
	.ndo_validate_addr = eth_validate_addr,
};

static int eth_init_one(struct platform_device *pdev)
{
	int i;
	struct port *port;
	struct sw *sw;
	struct net_device *dev;
	struct cns3xxx_plat_info *plat = pdev->dev.platform_data;
	char phy_id[MII_BUS_ID_SIZE + 3];
	int err;
	u32 temp;
	struct resource *res;
	void __iomem *regs;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(regs))
		return PTR_ERR(regs);

	err = cns3xxx_mdio_register(regs);
	if (err)
		return err;

	if (!(napi_dev = alloc_etherdev(sizeof(struct sw)))) {
		err = -ENOMEM;
		goto err_remove_mdio;
	}

	strcpy(napi_dev->name, "cns3xxx_eth");
	napi_dev->features = NETIF_F_IP_CSUM | NETIF_F_SG | NETIF_F_FRAGLIST;

	SET_NETDEV_DEV(napi_dev, &pdev->dev);
	sw = netdev_priv(napi_dev);
	memset(sw, 0, sizeof(struct sw));
	sw->regs = regs;
	sw->dev = &pdev->dev;

	sw->rx_irq = platform_get_irq_byname(pdev, "eth_rx");
	sw->stat_irq = platform_get_irq_byname(pdev, "eth_stat");

	temp = __raw_readl(&sw->regs->phy_auto_addr);
	temp |= (3 << 30); /* maximum frame length: 9600 bytes */
	__raw_writel(temp, &sw->regs->phy_auto_addr);

	for (i = 0; i < 4; i++) {
		temp = __raw_readl(&sw->regs->mac_cfg[i]);
		temp |= (PORT_DISABLE);
		__raw_writel(temp, &sw->regs->mac_cfg[i]);
	}

	temp = PORT_DISABLE;
	__raw_writel(temp, &sw->regs->mac_cfg[2]);

	temp = __raw_readl(&sw->regs->vlan_cfg);
	temp |= NIC_MODE | VLAN_UNAWARE;
	__raw_writel(temp, &sw->regs->vlan_cfg);

	__raw_writel(UNKNOWN_VLAN_TO_CPU |
		     CRC_STRIPPING, &sw->regs->mac_glob_cfg);

	if ((err = init_rings(sw)) != 0) {
		err = -ENOMEM;
		goto err_free;
	}
	platform_set_drvdata(pdev, napi_dev);

	netif_napi_add(napi_dev, &sw->napi, eth_poll, NAPI_WEIGHT);

	for (i = 0; i < 3; i++) {
		if (!(plat->ports & (1 << i))) {
			continue;
		}

		if (!(dev = alloc_etherdev(sizeof(struct port)))) {
			goto free_ports;
		}

		port = netdev_priv(dev);
		port->netdev = dev;
		if (i == 2)
			port->id = 3;
		else
			port->id = i;
		port->sw = sw;

		temp = __raw_readl(&sw->regs->mac_cfg[port->id]);
		temp |= (PORT_DISABLE | PORT_BLOCK_STATE | PORT_LEARN_DIS);
		__raw_writel(temp, &sw->regs->mac_cfg[port->id]);

		SET_NETDEV_DEV(dev, &pdev->dev);
		dev->netdev_ops = &cns3xxx_netdev_ops;
		dev->ethtool_ops = &cns3xxx_ethtool_ops;
		dev->tx_queue_len = 1000;
		dev->features = NETIF_F_IP_CSUM | NETIF_F_SG | NETIF_F_FRAGLIST;

		switch_port_tab[port->id] = port;
		memcpy(dev->dev_addr, &plat->hwaddr[i], ETH_ALEN);

		snprintf(phy_id, MII_BUS_ID_SIZE + 3, PHY_ID_FMT, "0", plat->phy[i]);
		port->phydev = phy_connect(dev, phy_id, &cns3xxx_adjust_link,
			PHY_INTERFACE_MODE_RGMII);
		if ((err = IS_ERR(port->phydev))) {
			switch_port_tab[port->id] = 0;
			free_netdev(dev);
			goto free_ports;
		}

		port->phydev->irq = PHY_IGNORE_INTERRUPT;

		if ((err = register_netdev(dev))) {
			phy_disconnect(port->phydev);
			switch_port_tab[port->id] = 0;
			free_netdev(dev);
			goto free_ports;
		}

		printk(KERN_INFO "%s: RGMII PHY %i on cns3xxx Switch\n", dev->name, plat->phy[i]);
		netif_carrier_off(dev);
		dev = 0;
	}

	return 0;

free_ports:
	err = -ENOMEM;
	for (--i; i >= 0; i--) {
		if (switch_port_tab[i]) {
			port = switch_port_tab[i];
			dev = port->netdev;
			unregister_netdev(dev);
			phy_disconnect(port->phydev);
			switch_port_tab[i] = 0;
			free_netdev(dev);
		}
	}
err_free:
	free_netdev(napi_dev);
err_remove_mdio:
	cns3xxx_mdio_remove();
	return err;
}

static int eth_remove_one(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct sw *sw = netdev_priv(dev);
	int i;

	destroy_rings(sw);
	for (i = 3; i >= 0; i--) {
		if (switch_port_tab[i]) {
			struct port *port = switch_port_tab[i];
			struct net_device *dev = port->netdev;
			unregister_netdev(dev);
			phy_disconnect(port->phydev);
			switch_port_tab[i] = 0;
			free_netdev(dev);
		}
	}

	free_netdev(napi_dev);
	cns3xxx_mdio_remove();

	return 0;
}

static struct platform_driver cns3xxx_eth_driver = {
	.driver.name	= DRV_NAME,
	.probe		= eth_init_one,
	.remove		= eth_remove_one,
};

static int __init eth_init_module(void)
{
	return platform_driver_register(&cns3xxx_eth_driver);
}

static void __exit eth_cleanup_module(void)
{
	platform_driver_unregister(&cns3xxx_eth_driver);
}

module_init(eth_init_module);
module_exit(eth_cleanup_module);

MODULE_AUTHOR("Chris Lang");
MODULE_DESCRIPTION("Cavium CNS3xxx Ethernet driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:cns3xxx_eth");
