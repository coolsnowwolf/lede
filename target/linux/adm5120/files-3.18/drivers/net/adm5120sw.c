/*
 *  ADM5120 built-in ethernet switch driver
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This code was based on a driver for Linux 2.6.xx by Jeroen Vreeken.
 *    Copyright Jeroen Vreeken (pe1rxq@amsat.org), 2005
 *  NAPI extension for the Jeroen's driver
 *    Copyright Thomas Langer (Thomas.Langer@infineon.com), 2007
 *    Copyright Friedrich Beckmann (Friedrich.Beckmann@infineon.com), 2007
 *  Inspiration for the Jeroen's driver came from the ADMtek 2.4 driver.
 *    Copyright ADMtek Inc.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2  as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/irq.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>

#include <asm/mipsregs.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>

#include "adm5120sw.h"
#include <linux/dma-mapping.h>

#define DRV_NAME	"adm5120-switch"
#define DRV_DESC	"ADM5120 built-in ethernet switch driver"
#define DRV_VERSION	"0.1.1"

#define CONFIG_ADM5120_SWITCH_NAPI	1
#undef CONFIG_ADM5120_SWITCH_DEBUG

/* ------------------------------------------------------------------------ */

#ifdef CONFIG_ADM5120_SWITCH_DEBUG
#define SW_DBG(f, a...)		printk(KERN_DEBUG "%s: " f, DRV_NAME , ## a)
#else
#define SW_DBG(f, a...)		do {} while (0)
#endif
#define SW_ERR(f, a...)		printk(KERN_ERR "%s: " f, DRV_NAME , ## a)
#define SW_INFO(f, a...)	printk(KERN_INFO "%s: " f, DRV_NAME , ## a)

#define SWITCH_NUM_PORTS	6
#define ETH_CSUM_LEN		4

#define RX_MAX_PKTLEN	1550
#define RX_RING_SIZE	64

#define TX_RING_SIZE	32
#define TX_QUEUE_LEN	28	/* Limit ring entries actually used. */
#define TX_TIMEOUT	(HZ * 400)

#define RX_DESCS_SIZE	(RX_RING_SIZE * sizeof(struct dma_desc *))
#define RX_SKBS_SIZE	(RX_RING_SIZE * sizeof(struct sk_buff *))
#define TX_DESCS_SIZE	(TX_RING_SIZE * sizeof(struct dma_desc *))
#define TX_SKBS_SIZE	(TX_RING_SIZE * sizeof(struct sk_buff *))

#define SKB_ALLOC_LEN		(RX_MAX_PKTLEN + 32)
#define SKB_RESERVE_LEN		(NET_IP_ALIGN + NET_SKB_PAD)

#define SWITCH_INTS_HIGH (SWITCH_INT_SHD | SWITCH_INT_RHD | SWITCH_INT_HDF)
#define SWITCH_INTS_LOW (SWITCH_INT_SLD | SWITCH_INT_RLD | SWITCH_INT_LDF)
#define SWITCH_INTS_ERR (SWITCH_INT_RDE | SWITCH_INT_SDE | SWITCH_INT_CPUH)
#define SWITCH_INTS_Q (SWITCH_INT_P0QF | SWITCH_INT_P1QF | SWITCH_INT_P2QF | \
			SWITCH_INT_P3QF | SWITCH_INT_P4QF | SWITCH_INT_P5QF | \
			SWITCH_INT_CPQF | SWITCH_INT_GQF)

#define SWITCH_INTS_ALL (SWITCH_INTS_HIGH | SWITCH_INTS_LOW | \
			SWITCH_INTS_ERR | SWITCH_INTS_Q | \
			SWITCH_INT_MD | SWITCH_INT_PSC)

#define SWITCH_INTS_USED (SWITCH_INTS_LOW | SWITCH_INT_PSC)
#define SWITCH_INTS_POLL (SWITCH_INT_RLD | SWITCH_INT_LDF | SWITCH_INT_SLD)

/* ------------------------------------------------------------------------ */

struct adm5120_if_priv {
	struct net_device *dev;

	unsigned int	vlan_no;
	unsigned int	port_mask;

#ifdef CONFIG_ADM5120_SWITCH_NAPI
	struct napi_struct napi;
#endif
};

struct dma_desc {
	__u32			buf1;
#define DESC_OWN		(1UL << 31)	/* Owned by the switch */
#define DESC_EOR		(1UL << 28)	/* End of Ring */
#define DESC_ADDR_MASK		0x1FFFFFF
#define DESC_ADDR(x)	((__u32)(x) & DESC_ADDR_MASK)
	__u32			buf2;
#define DESC_BUF2_EN		(1UL << 31)	/* Buffer 2 enable */
	__u32			buflen;
	__u32			misc;
/* definitions for tx/rx descriptors */
#define DESC_PKTLEN_SHIFT	16
#define DESC_PKTLEN_MASK	0x7FF
/* tx descriptor specific part */
#define DESC_CSUM		(1UL << 31)	/* Append checksum */
#define DESC_DSTPORT_SHIFT	8
#define DESC_DSTPORT_MASK	0x3F
#define DESC_VLAN_MASK		0x3F
/* rx descriptor specific part */
#define DESC_SRCPORT_SHIFT	12
#define DESC_SRCPORT_MASK	0x7
#define DESC_DA_MASK		0x3
#define DESC_DA_SHIFT		4
#define DESC_IPCSUM_FAIL	(1UL << 3)	/* IP checksum fail */
#define DESC_VLAN_TAG		(1UL << 2)	/* VLAN tag present */
#define DESC_TYPE_MASK		0x3		/* mask for Packet type */
#define DESC_TYPE_IP		0x0		/* IP packet */
#define DESC_TYPE_PPPoE		0x1		/* PPPoE packet */
} __attribute__ ((aligned(16)));

/* ------------------------------------------------------------------------ */

static int adm5120_nrdevs;

static struct net_device *adm5120_devs[SWITCH_NUM_PORTS];
/* Lookup table port -> device */
static struct net_device *adm5120_port[SWITCH_NUM_PORTS];

static struct dma_desc *txl_descs;
static struct dma_desc *rxl_descs;

static dma_addr_t txl_descs_dma;
static dma_addr_t rxl_descs_dma;

static struct sk_buff **txl_skbuff;
static struct sk_buff **rxl_skbuff;

static unsigned int cur_rxl, dirty_rxl; /* producer/consumer ring indices */
static unsigned int cur_txl, dirty_txl;

static unsigned int sw_used;

static DEFINE_SPINLOCK(tx_lock);

/* ------------------------------------------------------------------------ */

static inline u32 sw_read_reg(u32 reg)
{
	return __raw_readl((void __iomem *)KSEG1ADDR(ADM5120_SWITCH_BASE)+reg);
}

static inline void sw_write_reg(u32 reg, u32 val)
{
	__raw_writel(val, (void __iomem *)KSEG1ADDR(ADM5120_SWITCH_BASE)+reg);
}

static inline void sw_int_mask(u32 mask)
{
	u32	t;

	t = sw_read_reg(SWITCH_REG_INT_MASK);
	t |= mask;
	sw_write_reg(SWITCH_REG_INT_MASK, t);
}

static inline void sw_int_unmask(u32 mask)
{
	u32	t;

	t = sw_read_reg(SWITCH_REG_INT_MASK);
	t &= ~mask;
	sw_write_reg(SWITCH_REG_INT_MASK, t);
}

static inline void sw_int_ack(u32 mask)
{
	sw_write_reg(SWITCH_REG_INT_STATUS, mask);
}

static inline u32 sw_int_status(void)
{
	u32	t;

	t = sw_read_reg(SWITCH_REG_INT_STATUS);
	t &= ~sw_read_reg(SWITCH_REG_INT_MASK);
	return t;
}

static inline u32 desc_get_srcport(struct dma_desc *desc)
{
	return (desc->misc >> DESC_SRCPORT_SHIFT) & DESC_SRCPORT_MASK;
}

static inline u32 desc_get_pktlen(struct dma_desc *desc)
{
	return (desc->misc >> DESC_PKTLEN_SHIFT) & DESC_PKTLEN_MASK;
}

static inline int desc_ipcsum_fail(struct dma_desc *desc)
{
	return ((desc->misc & DESC_IPCSUM_FAIL) != 0);
}

/* ------------------------------------------------------------------------ */

#ifdef CONFIG_ADM5120_SWITCH_DEBUG
static void sw_dump_desc(char *label, struct dma_desc *desc, int tx)
{
	u32 t;

	SW_DBG("%s %s desc/%p\n", label, tx ? "tx" : "rx", desc);

	t = desc->buf1;
	SW_DBG("    buf1 %08X addr=%08X; len=%08X %s%s\n", t,
		t & DESC_ADDR_MASK,
		desc->buflen,
		(t & DESC_OWN) ? "SWITCH" : "CPU",
		(t & DESC_EOR) ? " RE" : "");

	t = desc->buf2;
	SW_DBG("    buf2 %08X addr=%08X%s\n", desc->buf2,
		t & DESC_ADDR_MASK,
		(t & DESC_BUF2_EN) ? " EN" : "");

	t = desc->misc;
	if (tx)
		SW_DBG("    misc %08X%s pktlen=%04X ports=%02X vlan=%02X\n", t,
			(t & DESC_CSUM) ? " CSUM" : "",
			(t >> DESC_PKTLEN_SHIFT) & DESC_PKTLEN_MASK,
			(t >> DESC_DSTPORT_SHIFT) & DESC_DSTPORT_MASK,
			t & DESC_VLAN_MASK);
	else
		SW_DBG("    misc %08X pktlen=%04X port=%d DA=%d%s%s type=%d\n",
			t,
			(t >> DESC_PKTLEN_SHIFT) & DESC_PKTLEN_MASK,
			(t >> DESC_SRCPORT_SHIFT) & DESC_SRCPORT_MASK,
			(t >> DESC_DA_SHIFT) & DESC_DA_MASK,
			(t & DESC_IPCSUM_FAIL) ? " IPCF" : "",
			(t & DESC_VLAN_TAG) ? " VLAN" : "",
			(t & DESC_TYPE_MASK));
}

static void sw_dump_intr_mask(char *label, u32 mask)
{
	SW_DBG("%s %08X%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		label, mask,
		(mask & SWITCH_INT_SHD) ? " SHD" : "",
		(mask & SWITCH_INT_SLD) ? " SLD" : "",
		(mask & SWITCH_INT_RHD) ? " RHD" : "",
		(mask & SWITCH_INT_RLD) ? " RLD" : "",
		(mask & SWITCH_INT_HDF) ? " HDF" : "",
		(mask & SWITCH_INT_LDF) ? " LDF" : "",
		(mask & SWITCH_INT_P0QF) ? " P0QF" : "",
		(mask & SWITCH_INT_P1QF) ? " P1QF" : "",
		(mask & SWITCH_INT_P2QF) ? " P2QF" : "",
		(mask & SWITCH_INT_P3QF) ? " P3QF" : "",
		(mask & SWITCH_INT_P4QF) ? " P4QF" : "",
		(mask & SWITCH_INT_CPQF) ? " CPQF" : "",
		(mask & SWITCH_INT_GQF) ? " GQF" : "",
		(mask & SWITCH_INT_MD) ? " MD" : "",
		(mask & SWITCH_INT_BCS) ? " BCS" : "",
		(mask & SWITCH_INT_PSC) ? " PSC" : "",
		(mask & SWITCH_INT_ID) ? " ID" : "",
		(mask & SWITCH_INT_W0TE) ? " W0TE" : "",
		(mask & SWITCH_INT_W1TE) ? " W1TE" : "",
		(mask & SWITCH_INT_RDE) ? " RDE" : "",
		(mask & SWITCH_INT_SDE) ? " SDE" : "",
		(mask & SWITCH_INT_CPUH) ? " CPUH" : "");
}

static void sw_dump_regs(void)
{
	u32 t;

	t = sw_read_reg(SWITCH_REG_PHY_STATUS);
	SW_DBG("phy_status: %08X\n", t);

	t = sw_read_reg(SWITCH_REG_CPUP_CONF);
	SW_DBG("cpup_conf: %08X%s%s%s\n", t,
		(t & CPUP_CONF_DCPUP) ? " DCPUP" : "",
		(t & CPUP_CONF_CRCP) ? " CRCP" : "",
		(t & CPUP_CONF_BTM) ? " BTM" : "");

	t = sw_read_reg(SWITCH_REG_PORT_CONF0);
	SW_DBG("port_conf0: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PORT_CONF1);
	SW_DBG("port_conf1: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PORT_CONF2);
	SW_DBG("port_conf2: %08X\n", t);

	t = sw_read_reg(SWITCH_REG_VLAN_G1);
	SW_DBG("vlan g1: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_VLAN_G2);
	SW_DBG("vlan g2: %08X\n", t);

	t = sw_read_reg(SWITCH_REG_BW_CNTL0);
	SW_DBG("bw_cntl0: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_BW_CNTL1);
	SW_DBG("bw_cntl1: %08X\n", t);

	t = sw_read_reg(SWITCH_REG_PHY_CNTL0);
	SW_DBG("phy_cntl0: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PHY_CNTL1);
	SW_DBG("phy_cntl1: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PHY_CNTL2);
	SW_DBG("phy_cntl2: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PHY_CNTL3);
	SW_DBG("phy_cntl3: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_PHY_CNTL4);
	SW_DBG("phy_cntl4: %08X\n", t);

	t = sw_read_reg(SWITCH_REG_INT_STATUS);
	sw_dump_intr_mask("int_status: ", t);

	t = sw_read_reg(SWITCH_REG_INT_MASK);
	sw_dump_intr_mask("int_mask: ", t);

	t = sw_read_reg(SWITCH_REG_SHDA);
	SW_DBG("shda: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_SLDA);
	SW_DBG("slda: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_RHDA);
	SW_DBG("rhda: %08X\n", t);
	t = sw_read_reg(SWITCH_REG_RLDA);
	SW_DBG("rlda: %08X\n", t);
}
#else
static inline void sw_dump_desc(char *label, struct dma_desc *desc, int tx) {}
static void sw_dump_intr_mask(char *label, u32 mask) {}
static inline void sw_dump_regs(void) {}
#endif /* CONFIG_ADM5120_SWITCH_DEBUG */

/* ------------------------------------------------------------------------ */

static inline void adm5120_rx_dma_update(struct dma_desc *desc,
	struct sk_buff *skb, int end)
{
	desc->misc = 0;
	desc->buf2 = 0;
	desc->buflen = RX_MAX_PKTLEN;
	desc->buf1 = DESC_ADDR(skb->data) |
		DESC_OWN | (end ? DESC_EOR : 0);
}

static void adm5120_switch_rx_refill(void)
{
	unsigned int entry;

	for (; cur_rxl - dirty_rxl > 0; dirty_rxl++) {
		struct dma_desc *desc;
		struct sk_buff *skb;

		entry = dirty_rxl % RX_RING_SIZE;
		desc = &rxl_descs[entry];

		skb = rxl_skbuff[entry];
		if (skb == NULL) {
			skb = alloc_skb(SKB_ALLOC_LEN, GFP_ATOMIC);
			if (skb) {
				skb_reserve(skb, SKB_RESERVE_LEN);
				rxl_skbuff[entry] = skb;
			} else {
				SW_ERR("no memory for skb\n");
				desc->buflen = 0;
				desc->buf2 = 0;
				desc->misc = 0;
				desc->buf1 = (desc->buf1 & DESC_EOR) | DESC_OWN;
				break;
			}
		}

		desc->buf2 = 0;
		desc->buflen = RX_MAX_PKTLEN;
		desc->misc = 0;
		desc->buf1 = (desc->buf1 & DESC_EOR) | DESC_OWN |
				DESC_ADDR(skb->data);
	}
}

static int adm5120_switch_rx(int limit)
{
	unsigned int done = 0;

	SW_DBG("rx start, limit=%d, cur_rxl=%u, dirty_rxl=%u\n",
				limit, cur_rxl, dirty_rxl);

	while (done < limit) {
		int entry = cur_rxl % RX_RING_SIZE;
		struct dma_desc *desc = &rxl_descs[entry];
		struct net_device *rdev;
		unsigned int port;

		if (desc->buf1 & DESC_OWN)
			break;

		if (dirty_rxl + RX_RING_SIZE == cur_rxl)
			break;

		port = desc_get_srcport(desc);
		rdev = adm5120_port[port];

		SW_DBG("rx descriptor %u, desc=%p, skb=%p\n", entry, desc,
				rxl_skbuff[entry]);

		if ((rdev) && netif_running(rdev)) {
			struct sk_buff *skb = rxl_skbuff[entry];
			int pktlen;

			pktlen = desc_get_pktlen(desc);
			pktlen -= ETH_CSUM_LEN;

			if ((pktlen == 0) || desc_ipcsum_fail(desc)) {
				rdev->stats.rx_errors++;
				if (pktlen == 0)
					rdev->stats.rx_length_errors++;
				if (desc_ipcsum_fail(desc))
					rdev->stats.rx_crc_errors++;
				SW_DBG("rx error, recycling skb %u\n", entry);
			} else {
				skb_put(skb, pktlen);

				skb->dev = rdev;
				skb->protocol = eth_type_trans(skb, rdev);
				skb->ip_summed = CHECKSUM_UNNECESSARY;

				dma_cache_wback_inv((unsigned long)skb->data,
					skb->len);

#ifdef CONFIG_ADM5120_SWITCH_NAPI
				netif_receive_skb(skb);
#else
				netif_rx(skb);
#endif

				rdev->last_rx = jiffies;
				rdev->stats.rx_packets++;
				rdev->stats.rx_bytes += pktlen;

				rxl_skbuff[entry] = NULL;
				done++;
			}
		} else {
			SW_DBG("no rx device, recycling skb %u\n", entry);
		}

		cur_rxl++;
		if (cur_rxl - dirty_rxl > RX_RING_SIZE / 4)
			adm5120_switch_rx_refill();
	}

	adm5120_switch_rx_refill();

	SW_DBG("rx finished, cur_rxl=%u, dirty_rxl=%u, processed %d\n",
				cur_rxl, dirty_rxl, done);

	return done;
}

static void adm5120_switch_tx(void)
{
	unsigned int entry;

	spin_lock(&tx_lock);
	entry = dirty_txl % TX_RING_SIZE;
	while (dirty_txl != cur_txl) {
		struct dma_desc *desc = &txl_descs[entry];
		struct sk_buff *skb = txl_skbuff[entry];

		if (desc->buf1 & DESC_OWN)
			break;

		if (netif_running(skb->dev)) {
			skb->dev->stats.tx_bytes += skb->len;
			skb->dev->stats.tx_packets++;
		}

		dev_kfree_skb_irq(skb);
		txl_skbuff[entry] = NULL;
		entry = (++dirty_txl) % TX_RING_SIZE;
	}

	if ((cur_txl - dirty_txl) < TX_QUEUE_LEN - 4) {
		int i;
		for (i = 0; i < SWITCH_NUM_PORTS; i++) {
			if (!adm5120_devs[i])
				continue;
			netif_wake_queue(adm5120_devs[i]);
		}
	}
	spin_unlock(&tx_lock);
}

#ifdef CONFIG_ADM5120_SWITCH_NAPI
static int adm5120_if_poll(struct napi_struct *napi, int limit)
{
	struct adm5120_if_priv *priv = container_of(napi,
				struct adm5120_if_priv, napi);
	struct net_device *dev  __maybe_unused = priv->dev;
	int done;
	u32 status;

	sw_int_ack(SWITCH_INTS_POLL);

	SW_DBG("%s: processing TX ring\n", dev->name);
	adm5120_switch_tx();

	SW_DBG("%s: processing RX ring\n", dev->name);
	done = adm5120_switch_rx(limit);

	status = sw_int_status() & SWITCH_INTS_POLL;
	if ((done < limit) && (!status)) {
		SW_DBG("disable polling mode for %s\n", dev->name);
		napi_complete(napi);
		sw_int_unmask(SWITCH_INTS_POLL);
		return 0;
	}

	SW_DBG("%s still in polling mode, done=%d, status=%x\n",
			dev->name, done, status);
	return 1;
}
#endif /* CONFIG_ADM5120_SWITCH_NAPI */


static irqreturn_t adm5120_switch_irq(int irq, void *dev_id)
{
	u32 status;

	status = sw_int_status();
	status &= SWITCH_INTS_ALL;
	if (!status)
		return IRQ_NONE;

#ifdef CONFIG_ADM5120_SWITCH_NAPI
	sw_int_ack(status & ~SWITCH_INTS_POLL);

	if (status & SWITCH_INTS_POLL) {
		struct net_device *dev = dev_id;
		struct adm5120_if_priv *priv = netdev_priv(dev);

		sw_dump_intr_mask("poll ints", status);
		SW_DBG("enable polling mode for %s\n", dev->name);
		sw_int_mask(SWITCH_INTS_POLL);
		napi_schedule(&priv->napi);
	}
#else
	sw_int_ack(status);

	if (status & (SWITCH_INT_RLD | SWITCH_INT_LDF))
		adm5120_switch_rx(RX_RING_SIZE);

	if (status & SWITCH_INT_SLD)
		adm5120_switch_tx();
#endif

	return IRQ_HANDLED;
}

static void adm5120_set_bw(char *matrix)
{
	unsigned long val;

	/* Port 0 to 3 are set using the bandwidth control 0 register */
	val = matrix[0] + (matrix[1]<<8) + (matrix[2]<<16) + (matrix[3]<<24);
	sw_write_reg(SWITCH_REG_BW_CNTL0, val);

	/* Port 4 and 5 are set using the bandwidth control 1 register */
	val = matrix[4];
	if (matrix[5] == 1)
		sw_write_reg(SWITCH_REG_BW_CNTL1, val | 0x80000000);
	else
		sw_write_reg(SWITCH_REG_BW_CNTL1, val & ~0x8000000);

	SW_DBG("D: ctl0 0x%ux, ctl1 0x%ux\n", sw_read_reg(SWITCH_REG_BW_CNTL0),
		sw_read_reg(SWITCH_REG_BW_CNTL1));
}

static void adm5120_switch_tx_ring_reset(struct dma_desc *desc,
		struct sk_buff **skbl, int num)
{
	memset(desc, 0, num * sizeof(*desc));
	desc[num-1].buf1 |= DESC_EOR;
	memset(skbl, 0, sizeof(struct skb *) * num);

	cur_txl = 0;
	dirty_txl = 0;
}

static void adm5120_switch_rx_ring_reset(struct dma_desc *desc,
		struct sk_buff **skbl, int num)
{
	int i;

	memset(desc, 0, num * sizeof(*desc));
	for (i = 0; i < num; i++) {
		skbl[i] = dev_alloc_skb(SKB_ALLOC_LEN);
		if (!skbl[i]) {
			i = num;
			break;
		}
		skb_reserve(skbl[i], SKB_RESERVE_LEN);
		adm5120_rx_dma_update(&desc[i], skbl[i], (num - 1 == i));
	}

	cur_rxl = 0;
	dirty_rxl = 0;
}

static int adm5120_switch_tx_ring_alloc(void)
{
	int err;

	txl_descs = dma_alloc_coherent(NULL, TX_DESCS_SIZE, &txl_descs_dma,
					GFP_ATOMIC);
	if (!txl_descs) {
		err = -ENOMEM;
		goto err;
	}

	txl_skbuff = kzalloc(TX_SKBS_SIZE, GFP_KERNEL);
	if (!txl_skbuff) {
		err = -ENOMEM;
		goto err;
	}

	return 0;

err:
	return err;
}

static void adm5120_switch_tx_ring_free(void)
{
	int i;

	if (txl_skbuff) {
		for (i = 0; i < TX_RING_SIZE; i++)
			if (txl_skbuff[i])
				kfree_skb(txl_skbuff[i]);
		kfree(txl_skbuff);
	}

	if (txl_descs)
		dma_free_coherent(NULL, TX_DESCS_SIZE, txl_descs,
			txl_descs_dma);
}

static int adm5120_switch_rx_ring_alloc(void)
{
	int err;
	int i;

	/* init RX ring */
	rxl_descs = dma_alloc_coherent(NULL, RX_DESCS_SIZE, &rxl_descs_dma,
					GFP_ATOMIC);
	if (!rxl_descs) {
		err = -ENOMEM;
		goto err;
	}

	rxl_skbuff = kzalloc(RX_SKBS_SIZE, GFP_KERNEL);
	if (!rxl_skbuff) {
		err = -ENOMEM;
		goto err;
	}

	for (i = 0; i < RX_RING_SIZE; i++) {
		struct sk_buff *skb;
		skb = alloc_skb(SKB_ALLOC_LEN, GFP_ATOMIC);
		if (!skb) {
			err = -ENOMEM;
			goto err;
		}
		rxl_skbuff[i] = skb;
		skb_reserve(skb, SKB_RESERVE_LEN);
	}

	return 0;

err:
	return err;
}

static void adm5120_switch_rx_ring_free(void)
{
	int i;

	if (rxl_skbuff) {
		for (i = 0; i < RX_RING_SIZE; i++)
			if (rxl_skbuff[i])
				kfree_skb(rxl_skbuff[i]);
		kfree(rxl_skbuff);
	}

	if (rxl_descs)
		dma_free_coherent(NULL, RX_DESCS_SIZE, rxl_descs,
			rxl_descs_dma);
}

static void adm5120_write_mac(struct net_device *dev)
{
	struct adm5120_if_priv *priv = netdev_priv(dev);
	unsigned char *mac = dev->dev_addr;
	u32 t;

	t = mac[2] | (mac[3] << MAC_WT1_MAC3_SHIFT) |
		(mac[4] << MAC_WT1_MAC4_SHIFT) | (mac[5] << MAC_WT1_MAC5_SHIFT);
	sw_write_reg(SWITCH_REG_MAC_WT1, t);

	t = (mac[0] << MAC_WT0_MAC0_SHIFT) | (mac[1] << MAC_WT0_MAC1_SHIFT) |
		MAC_WT0_MAWC | MAC_WT0_WVE | (priv->vlan_no<<3);

	sw_write_reg(SWITCH_REG_MAC_WT0, t);

	while (!(sw_read_reg(SWITCH_REG_MAC_WT0) & MAC_WT0_MWD))
		;
}

static void adm5120_set_vlan(char *matrix)
{
	unsigned long val;
	int vlan_port, port;

	val = matrix[0] + (matrix[1]<<8) + (matrix[2]<<16) + (matrix[3]<<24);
	sw_write_reg(SWITCH_REG_VLAN_G1, val);
	val = matrix[4] + (matrix[5]<<8);
	sw_write_reg(SWITCH_REG_VLAN_G2, val);

	/* Now set/update the port vs. device lookup table */
	for (port = 0; port < SWITCH_NUM_PORTS; port++) {
		for (vlan_port = 0; vlan_port < SWITCH_NUM_PORTS && !(matrix[vlan_port] & (0x00000001 << port)); vlan_port++)
			;
		if (vlan_port < SWITCH_NUM_PORTS)
			adm5120_port[port] = adm5120_devs[vlan_port];
		else
			adm5120_port[port] = NULL;
	}
}

static void adm5120_switch_set_vlan_mac(unsigned int vlan, unsigned char *mac)
{
	u32 t;

	t = mac[2] | (mac[3] << MAC_WT1_MAC3_SHIFT)
		| (mac[4] << MAC_WT1_MAC4_SHIFT)
		| (mac[5] << MAC_WT1_MAC5_SHIFT);
	sw_write_reg(SWITCH_REG_MAC_WT1, t);

	t = (mac[0] << MAC_WT0_MAC0_SHIFT) | (mac[1] << MAC_WT0_MAC1_SHIFT) |
		MAC_WT0_MAWC | MAC_WT0_WVE | (vlan << MAC_WT0_WVN_SHIFT) |
		(MAC_WT0_WAF_STATIC << MAC_WT0_WAF_SHIFT);
	sw_write_reg(SWITCH_REG_MAC_WT0, t);

	do {
		t = sw_read_reg(SWITCH_REG_MAC_WT0);
	} while ((t & MAC_WT0_MWD) == 0);
}

static void adm5120_switch_set_vlan_ports(unsigned int vlan, u32 ports)
{
	unsigned int reg;
	u32 t;

	if (vlan < 4)
		reg = SWITCH_REG_VLAN_G1;
	else {
		vlan -= 4;
		reg = SWITCH_REG_VLAN_G2;
	}

	t = sw_read_reg(reg);
	t &= ~(0xFF << (vlan*8));
	t |= (ports << (vlan*8));
	sw_write_reg(reg, t);
}

/* ------------------------------------------------------------------------ */

#ifdef CONFIG_ADM5120_SWITCH_NAPI
static inline void adm5120_if_napi_enable(struct net_device *dev)
{
	struct adm5120_if_priv *priv = netdev_priv(dev);
	napi_enable(&priv->napi);
}

static inline void adm5120_if_napi_disable(struct net_device *dev)
{
	struct adm5120_if_priv *priv = netdev_priv(dev);
	napi_disable(&priv->napi);
}
#else
static inline void adm5120_if_napi_enable(struct net_device *dev) {}
static inline void adm5120_if_napi_disable(struct net_device *dev) {}
#endif /* CONFIG_ADM5120_SWITCH_NAPI */

static int adm5120_if_open(struct net_device *dev)
{
	u32 t;
	int err;
	int i;

	adm5120_if_napi_enable(dev);

	err = request_irq(dev->irq, adm5120_switch_irq, IRQF_SHARED,
			  dev->name, dev);
	if (err) {
		SW_ERR("unable to get irq for %s\n", dev->name);
		goto err;
	}

	if (!sw_used++)
		/* enable interrupts on first open */
		sw_int_unmask(SWITCH_INTS_USED);

	/* enable (additional) port */
	t = sw_read_reg(SWITCH_REG_PORT_CONF0);
	for (i = 0; i < SWITCH_NUM_PORTS; i++) {
		if (dev == adm5120_devs[i])
			t &= ~adm5120_eth_vlans[i];
	}
	sw_write_reg(SWITCH_REG_PORT_CONF0, t);

	netif_start_queue(dev);

	return 0;

err:
	adm5120_if_napi_disable(dev);
	return err;
}

static int adm5120_if_stop(struct net_device *dev)
{
	u32 t;
	int i;

	netif_stop_queue(dev);
	adm5120_if_napi_disable(dev);

	/* disable port if not assigned to other devices */
	t = sw_read_reg(SWITCH_REG_PORT_CONF0);
	t |= SWITCH_PORTS_NOCPU;
	for (i = 0; i < SWITCH_NUM_PORTS; i++) {
		if ((dev != adm5120_devs[i]) && netif_running(adm5120_devs[i]))
			t &= ~adm5120_eth_vlans[i];
	}
	sw_write_reg(SWITCH_REG_PORT_CONF0, t);

	if (!--sw_used)
		sw_int_mask(SWITCH_INTS_USED);

	free_irq(dev->irq, dev);

	return 0;
}

static int adm5120_if_hard_start_xmit(struct sk_buff *skb,
		struct net_device *dev)
{
	struct dma_desc *desc;
	struct adm5120_if_priv *priv = netdev_priv(dev);
	unsigned int entry;
	unsigned long data;
	int i;

	/* lock switch irq */
	spin_lock_irq(&tx_lock);

	/* calculate the next TX descriptor entry. */
	entry = cur_txl % TX_RING_SIZE;

	desc = &txl_descs[entry];
	if (desc->buf1 & DESC_OWN) {
		/* We want to write a packet but the TX queue is still
		 * occupied by the DMA. We are faster than the DMA... */
		SW_DBG("%s unable to transmit, packet dopped\n", dev->name);
		dev_kfree_skb(skb);
		dev->stats.tx_dropped++;
		return 0;
	}

	txl_skbuff[entry] = skb;
	data = (desc->buf1 & DESC_EOR);
	data |= DESC_ADDR(skb->data);

	desc->misc =
	    ((skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len) << DESC_PKTLEN_SHIFT) |
	    (0x1 << priv->vlan_no);

	desc->buflen = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;

	desc->buf1 = data | DESC_OWN;
	sw_write_reg(SWITCH_REG_SEND_TRIG, SEND_TRIG_STL);

	cur_txl++;
	if (cur_txl == dirty_txl + TX_QUEUE_LEN) {
		for (i = 0; i < SWITCH_NUM_PORTS; i++) {
			if (!adm5120_devs[i])
				continue;
			netif_stop_queue(adm5120_devs[i]);
		}
	}

	dev->trans_start = jiffies;

	spin_unlock_irq(&tx_lock);

	return 0;
}

static void adm5120_if_tx_timeout(struct net_device *dev)
{
	SW_INFO("TX timeout on %s\n", dev->name);
}

static void adm5120_if_set_rx_mode(struct net_device *dev)
{
	struct adm5120_if_priv *priv = netdev_priv(dev);
	u32 ports;
	u32 t;

	ports = adm5120_eth_vlans[priv->vlan_no] & SWITCH_PORTS_NOCPU;

	t = sw_read_reg(SWITCH_REG_CPUP_CONF);
	if (dev->flags & IFF_PROMISC)
		/* enable unknown packets */
		t &= ~(ports << CPUP_CONF_DUNP_SHIFT);
	else
		/* disable unknown packets */
		t |= (ports << CPUP_CONF_DUNP_SHIFT);

	if (dev->flags & IFF_PROMISC || dev->flags & IFF_ALLMULTI ||
					netdev_mc_count(dev))
		/* enable multicast packets */
		t &= ~(ports << CPUP_CONF_DMCP_SHIFT);
	else
		/* disable multicast packets */
		t |= (ports << CPUP_CONF_DMCP_SHIFT);

	/* If there is any port configured to be in promiscuous mode, then the */
	/* Bridge Test Mode has to be activated. This will result in           */
	/* transporting also packets learned in another VLAN to be forwarded   */
	/* to the CPU.                                                         */
	/* The difficult scenario is when we want to build a bridge on the CPU.*/
	/* Assume we have port0 and the CPU port in VLAN0 and port1 and the    */
	/* CPU port in VLAN1. Now we build a bridge on the CPU between         */
	/* VLAN0 and VLAN1. Both ports of the VLANs are set in promisc mode.   */
	/* Now assume a packet with ethernet source address 99 enters port 0   */
	/* It will be forwarded to the CPU because it is unknown. Then the     */
	/* bridge in the CPU will send it to VLAN1 and it goes out at port 1.  */
	/* When now a packet with ethernet destination address 99 comes in at  */
	/* port 1 in VLAN1, then the switch has learned that this address is   */
	/* located at port 0 in VLAN0. Therefore the switch will drop          */
	/* this packet. In order to avoid this and to send the packet still    */
	/* to the CPU, the Bridge Test Mode has to be activated.               */

	/* Check if there is any vlan in promisc mode. */
	if (~t & (SWITCH_PORTS_NOCPU << CPUP_CONF_DUNP_SHIFT))
		t |= CPUP_CONF_BTM;  /* Enable Bridge Testing Mode */
	else
		t &= ~CPUP_CONF_BTM; /* Disable Bridge Testing Mode */

	sw_write_reg(SWITCH_REG_CPUP_CONF, t);

}

static int adm5120_if_set_mac_address(struct net_device *dev, void *p)
{
	int ret;

	ret = eth_mac_addr(dev, p);
	if (ret)
		return ret;

	adm5120_write_mac(dev);
	return 0;
}

static int adm5120_if_do_ioctl(struct net_device *dev, struct ifreq *rq,
		int cmd)
{
	int err;
	struct adm5120_sw_info info;
	struct adm5120_if_priv *priv = netdev_priv(dev);

	switch (cmd) {
	case SIOCGADMINFO:
		info.magic = 0x5120;
		info.ports = adm5120_nrdevs;
		info.vlan = priv->vlan_no;
		err = copy_to_user(rq->ifr_data, &info, sizeof(info));
		if (err)
			return -EFAULT;
		break;
	case SIOCSMATRIX:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		err = copy_from_user(adm5120_eth_vlans, rq->ifr_data,
					sizeof(adm5120_eth_vlans));
		if (err)
			return -EFAULT;
		adm5120_set_vlan(adm5120_eth_vlans);
		break;
	case SIOCGMATRIX:
		err = copy_to_user(rq->ifr_data, adm5120_eth_vlans,
					sizeof(adm5120_eth_vlans));
		if (err)
			return -EFAULT;
		break;
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static const struct net_device_ops adm5120sw_netdev_ops = {
	.ndo_open		= adm5120_if_open,
	.ndo_stop		= adm5120_if_stop,
	.ndo_start_xmit		= adm5120_if_hard_start_xmit,
	.ndo_set_rx_mode	= adm5120_if_set_rx_mode,
	.ndo_do_ioctl		= adm5120_if_do_ioctl,
	.ndo_tx_timeout		= adm5120_if_tx_timeout,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_set_mac_address	= adm5120_if_set_mac_address,
};

static struct net_device *adm5120_if_alloc(void)
{
	struct net_device *dev;
	struct adm5120_if_priv *priv;

	dev = alloc_etherdev(sizeof(*priv));
	if (!dev)
		return NULL;

	priv = netdev_priv(dev);
	priv->dev = dev;

	dev->irq		= ADM5120_IRQ_SWITCH;
	dev->netdev_ops		= &adm5120sw_netdev_ops;
	dev->watchdog_timeo	= TX_TIMEOUT;

#ifdef CONFIG_ADM5120_SWITCH_NAPI
	netif_napi_add(dev, &priv->napi, adm5120_if_poll, 64);
#endif

	return dev;
}

/* ------------------------------------------------------------------------ */

static void adm5120_switch_cleanup(void)
{
	int i;

	/* disable interrupts */
	sw_int_mask(SWITCH_INTS_ALL);

	for (i = 0; i < SWITCH_NUM_PORTS; i++) {
		struct net_device *dev = adm5120_devs[i];
		if (dev) {
			unregister_netdev(dev);
			free_netdev(dev);
		}
	}

	adm5120_switch_tx_ring_free();
	adm5120_switch_rx_ring_free();
}

static int adm5120_switch_probe(struct platform_device *pdev)
{
	u32 t;
	int i, err;

	adm5120_nrdevs = adm5120_eth_num_ports;

	t = CPUP_CONF_DCPUP | CPUP_CONF_CRCP |
		SWITCH_PORTS_NOCPU << CPUP_CONF_DUNP_SHIFT |
		SWITCH_PORTS_NOCPU << CPUP_CONF_DMCP_SHIFT ;
	sw_write_reg(SWITCH_REG_CPUP_CONF, t);

	t = (SWITCH_PORTS_NOCPU << PORT_CONF0_EMCP_SHIFT) |
		(SWITCH_PORTS_NOCPU << PORT_CONF0_BP_SHIFT) |
		(SWITCH_PORTS_NOCPU);
	sw_write_reg(SWITCH_REG_PORT_CONF0, t);

	/* setup ports to Autoneg/100M/Full duplex/Auto MDIX */
	t =  SWITCH_PORTS_PHY |
		(SWITCH_PORTS_PHY << PHY_CNTL2_SC_SHIFT) |
		(SWITCH_PORTS_PHY << PHY_CNTL2_DC_SHIFT) |
		(SWITCH_PORTS_PHY << PHY_CNTL2_PHYR_SHIFT) |
		(SWITCH_PORTS_PHY << PHY_CNTL2_AMDIX_SHIFT) |
		PHY_CNTL2_RMAE;
	sw_write_reg(SWITCH_REG_PHY_CNTL2, t);

	t = sw_read_reg(SWITCH_REG_PHY_CNTL3);
	t |= PHY_CNTL3_RNT;
	sw_write_reg(SWITCH_REG_PHY_CNTL3, t);

	/* Force all the packets from all ports are low priority */
	sw_write_reg(SWITCH_REG_PRI_CNTL, 0);

	sw_int_mask(SWITCH_INTS_ALL);
	sw_int_ack(SWITCH_INTS_ALL);

	err = adm5120_switch_rx_ring_alloc();
	if (err)
		goto err;

	err = adm5120_switch_tx_ring_alloc();
	if (err)
		goto err;

	adm5120_switch_tx_ring_reset(txl_descs, txl_skbuff, TX_RING_SIZE);
	adm5120_switch_rx_ring_reset(rxl_descs, rxl_skbuff, RX_RING_SIZE);

	sw_write_reg(SWITCH_REG_SHDA, 0);
	sw_write_reg(SWITCH_REG_SLDA, KSEG1ADDR(txl_descs));
	sw_write_reg(SWITCH_REG_RHDA, 0);
	sw_write_reg(SWITCH_REG_RLDA, KSEG1ADDR(rxl_descs));

	for (i = 0; i < SWITCH_NUM_PORTS; i++) {
		struct net_device *dev;
		struct adm5120_if_priv *priv;

		dev = adm5120_if_alloc();
		if (!dev) {
			err = -ENOMEM;
			goto err;
		}

		adm5120_devs[i] = dev;
		priv = netdev_priv(dev);

		priv->vlan_no = i;
		priv->port_mask = adm5120_eth_vlans[i];

		memcpy(dev->dev_addr, adm5120_eth_macs[i], 6);
		adm5120_write_mac(dev);

		err = register_netdev(dev);
		if (err) {
			SW_INFO("%s register failed, error=%d\n",
					dev->name, err);
			goto err;
		}
	}

	/* setup vlan/port mapping after devs are filled up */
	adm5120_set_vlan(adm5120_eth_vlans);

	/* enable CPU port */
	t = sw_read_reg(SWITCH_REG_CPUP_CONF);
	t &= ~CPUP_CONF_DCPUP;
	sw_write_reg(SWITCH_REG_CPUP_CONF, t);

	return 0;

err:
	adm5120_switch_cleanup();

	SW_ERR("init failed\n");
	return err;
}

static int adm5120_switch_remove(struct platform_device *pdev)
{
	adm5120_switch_cleanup();
	return 0;
}

static struct platform_driver adm5120_switch_driver = {
	.probe		= adm5120_switch_probe,
	.remove		= adm5120_switch_remove,
	.driver		= {
		.name	= DRV_NAME,
	},
};

/* -------------------------------------------------------------------------- */

static int __init adm5120_switch_mod_init(void)
{
	int err;

	pr_info(DRV_DESC " version " DRV_VERSION "\n");
	err = platform_driver_register(&adm5120_switch_driver);

	return err;
}

static void __exit adm5120_switch_mod_exit(void)
{
	platform_driver_unregister(&adm5120_switch_driver);
}

module_init(adm5120_switch_mod_init);
module_exit(adm5120_switch_mod_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
