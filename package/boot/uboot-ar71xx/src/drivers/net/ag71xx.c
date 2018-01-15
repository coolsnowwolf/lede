/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2010 Michael Kurz <michi.kurz@googlemail.com>
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
 
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include <asm/ar71xx.h>

#include "ag71xx.h"

#ifdef AG71XX_DEBUG
#define DBG(fmt,args...)		printf(fmt ,##args)
#else
#define DBG(fmt,args...)
#endif


static struct ag71xx agtable[] = {
	{
		.mac_base = KSEG1ADDR(AR71XX_GE0_BASE),
		.mii_ctrl = KSEG1ADDR(AR71XX_MII_BASE + MII_REG_MII0_CTRL),
		.mii_if = CONFIG_AG71XX_MII0_IIF,
	} , {
		.mac_base = KSEG1ADDR(AR71XX_GE1_BASE),
		.mii_ctrl = KSEG1ADDR(AR71XX_MII_BASE + MII_REG_MII1_CTRL),
		.mii_if = CONFIG_AG71XX_MII1_IIF,
	}
};

static int ag71xx_ring_alloc(struct ag71xx_ring *ring, unsigned int size)
{
	int err;
	int i;
	int rsize;

	ring->desc_size = sizeof(struct ag71xx_desc);
	if (ring->desc_size % (CONFIG_SYS_CACHELINE_SIZE)) {
		rsize = roundup(ring->desc_size, CONFIG_SYS_CACHELINE_SIZE);
		DBG("ag71xx: ring %p, desc size %u rounded to %u\n",
			ring, ring->desc_size,
			rsize);
		ring->desc_size = rsize;
	}

	ring->descs_cpu = (u8 *) malloc((size * ring->desc_size)
		+ CONFIG_SYS_CACHELINE_SIZE - 1);
	if (!ring->descs_cpu) {
		err = -1;
		goto err;
	}
	ring->descs_cpu = (u8 *) UNCACHED_SDRAM((((u32) ring->descs_cpu + 
		CONFIG_SYS_CACHELINE_SIZE - 1) & ~(CONFIG_SYS_CACHELINE_SIZE - 1)));
    ring->descs_dma = (u8 *) virt_to_phys(ring->descs_cpu);

	ring->size = size;

	ring->buf = malloc(size * sizeof(*ring->buf));
	if (!ring->buf) {
		err = -1;
		goto err;
	}
    memset(ring->buf, 0, size * sizeof(*ring->buf));

	for (i = 0; i < size; i++) {
		ring->buf[i].desc =
			(struct ag71xx_desc *)&ring->descs_cpu[i * ring->desc_size];
		DBG("ag71xx: ring %p, desc %d at %p\n",
			ring, i, ring->buf[i].desc);
	}

	flush_cache( (u32) ring->buf, size * sizeof(*ring->buf));
	
	return 0;

 err:
	return err;
}

static void ag71xx_ring_tx_init(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->tx_ring;
	int i;

	for (i = 0; i < AG71XX_TX_RING_SIZE; i++) {
		ring->buf[i].desc->next = (u32) virt_to_phys((ring->descs_dma +
			ring->desc_size * ((i + 1) % AG71XX_TX_RING_SIZE)));

		ring->buf[i].desc->ctrl = DESC_EMPTY;
		ring->buf[i].skb = NULL;
	}

	ring->curr = 0;
}

static void ag71xx_ring_rx_clean(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	int i;

	if (!ring->buf)
		return;

	for (i = 0; i < AG71XX_RX_RING_SIZE; i++) {
	    ring->buf[i].desc->data = (u32) virt_to_phys(NetRxPackets[i]);
	    flush_cache((u32) NetRxPackets[i], PKTSIZE_ALIGN);
        ring->buf[i].desc->ctrl = DESC_EMPTY;
    }

	ring->curr = 0;
}

static int ag71xx_ring_rx_init(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	unsigned int i;

	for (i = 0; i < AG71XX_RX_RING_SIZE; i++) {
		ring->buf[i].desc->next = (u32) virt_to_phys((ring->descs_dma +
			ring->desc_size * ((i + 1) % AG71XX_RX_RING_SIZE)));

		DBG("ag71xx: RX desc at %p, next is %08x\n",
			ring->buf[i].desc,
			ring->buf[i].desc->next);
	}

	for (i = 0; i < AG71XX_RX_RING_SIZE; i++) {
		ring->buf[i].desc->data = (u32) virt_to_phys(NetRxPackets[i]);
		ring->buf[i].desc->ctrl = DESC_EMPTY;
	}

	ring->curr = 0;

	return 0;
}

static int ag71xx_rings_init(struct ag71xx *ag)
{
	int ret;

	ret = ag71xx_ring_alloc(&ag->tx_ring, AG71XX_TX_RING_SIZE);
	if (ret)
		return ret;

	ag71xx_ring_tx_init(ag);

	ret = ag71xx_ring_alloc(&ag->rx_ring, AG71XX_RX_RING_SIZE);
	if (ret)
		return ret;

	ret = ag71xx_ring_rx_init(ag);
	return ret;
}

static void ar71xx_set_pll(u32 cfg_reg, u32 pll_reg, u32 pll_val, u32 shift)
{
	uint32_t base = KSEG1ADDR(AR71XX_PLL_BASE);
	u32 t;

	t = readl(base + cfg_reg);
	t &= ~(3 << shift);
	t |=  (2 << shift);
	writel(t, base + cfg_reg);
	udelay(100);

	writel(pll_val, base + pll_reg);

	t |= (3 << shift);
	writel(t, base + cfg_reg);
	udelay(100);

	t &= ~(3 << shift);
	writel(t, base + cfg_reg);
	udelay(100);

	debug("ar71xx: pll_reg %#x: %#x\n", (unsigned int)(base + pll_reg),
       readl(base + pll_reg));
}

static void ar91xx_set_pll_ge0(int speed)
{
	//u32 val = ar71xx_get_eth_pll(0, speed);
	u32 pll_val;

	switch (speed) {
	case SPEED_10:
		pll_val = 0x00441099;
		break;
	case SPEED_100:
		pll_val = 0x13000a44;
		break;
	case SPEED_1000:
		pll_val = 0x1a000000;
		break;
	default:
		BUG();
	}

	ar71xx_set_pll(AR91XX_PLL_REG_ETH_CONFIG, AR91XX_PLL_REG_ETH0_INT_CLOCK,
			 pll_val, AR91XX_ETH0_PLL_SHIFT);
}

static void ar91xx_set_pll_ge1(int speed)
{
	//u32 val = ar71xx_get_eth_pll(1, speed);
    u32 pll_val;

	switch (speed) {
	case SPEED_10:
		pll_val = 0x00441099;
		break;
	case SPEED_100:
		pll_val = 0x13000a44;
		break;
	case SPEED_1000:
		pll_val = 0x1a000000;
		break;
	default:
		BUG();
	}

	ar71xx_set_pll(AR91XX_PLL_REG_ETH_CONFIG, AR91XX_PLL_REG_ETH1_INT_CLOCK,
			 pll_val, AR91XX_ETH1_PLL_SHIFT);
}

static void ag71xx_hw_set_macaddr(struct ag71xx *ag, unsigned char *mac)
{
	u32 t;

	t = (((u32) mac[5]) << 24) | (((u32) mac[4]) << 16)
	  | (((u32) mac[3]) << 8) | ((u32) mac[2]);

	ag71xx_wr(ag, AG71XX_REG_MAC_ADDR1, t);

	t = (((u32) mac[1]) << 24) | (((u32) mac[0]) << 16);
	ag71xx_wr(ag, AG71XX_REG_MAC_ADDR2, t);
}

static void ag71xx_dma_reset(struct ag71xx *ag)
{
	u32 val;
	int i;

	DBG("%s: txdesc reg: 0x%08x rxdesc reg: 0x%08x\n",
			ag->dev->name,
			ag71xx_rr(ag, AG71XX_REG_TX_DESC),
			ag71xx_rr(ag, AG71XX_REG_RX_DESC));
	
	/* stop RX and TX */
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, 0);
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, 0);

	/* clear descriptor addresses */
	ag71xx_wr(ag, AG71XX_REG_TX_DESC, 0);
	ag71xx_wr(ag, AG71XX_REG_RX_DESC, 0);

	/* clear pending RX/TX interrupts */
	for (i = 0; i < 256; i++) {
		ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_PR);
		ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_PS);
	}

	/* clear pending errors */
	ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_BE | RX_STATUS_OF);
	ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_BE | TX_STATUS_UR);

	val = ag71xx_rr(ag, AG71XX_REG_RX_STATUS);
	if (val)
		printf("%s: unable to clear DMA Rx status: %08x\n",
			ag->dev->name, val);

	val = ag71xx_rr(ag, AG71XX_REG_TX_STATUS);

	/* mask out reserved bits */
	val &= ~0xff000000;

	if (val)
		printf("%s: unable to clear DMA Tx status: %08x\n",
			ag->dev->name, val);
}

static void ag71xx_halt(struct eth_device *dev)
{
    struct ag71xx *ag = (struct ag71xx *) dev->priv;

    /* stop RX engine */
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, 0);

	ag71xx_dma_reset(ag);
}

#define MAX_WAIT        1000

static int ag71xx_send(struct eth_device *dev, volatile void *packet,
                       int length)
{
    struct ag71xx *ag = (struct ag71xx *) dev->priv;
	struct ag71xx_ring *ring = &ag->tx_ring;
	struct ag71xx_desc *desc;
	int i;

	i = ring->curr % AG71XX_TX_RING_SIZE;
	desc = ring->buf[i].desc;

	if (!ag71xx_desc_empty(desc)) {
		printf("%s: tx buffer full\n", ag->dev->name);
		return 1;
	}

	flush_cache((u32) packet, length);
    desc->data = (u32) virt_to_phys(packet);
    desc->ctrl = (length & DESC_PKTLEN_M);
	
	DBG("%s: sending %#08x length %#08x\n",
		ag->dev->name, desc->data, desc->ctrl);
	
	ring->curr++;
	if (ring->curr >= AG71XX_TX_RING_SIZE){
		ring->curr = 0;
	}
	
	/* enable TX engine */
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, TX_CTRL_TXE);

    for (i = 0; i < MAX_WAIT; i++)
    {
        if (ag71xx_desc_empty(desc))
            break;
        udelay(10);
    }
    if (i == MAX_WAIT) {
        printf("%s: tx timed out!\n", ag->dev->name);
		return -1;
	}
	
	/* disable TX engine */
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, 0);
	desc->data = 0;
	desc->ctrl = DESC_EMPTY;
	
	return 0;
}

static int ag71xx_recv(struct eth_device *dev)
{
    struct ag71xx *ag = (struct ag71xx *) dev->priv;
	struct ag71xx_ring *ring = &ag->rx_ring;

    for (;;) {
		unsigned int i = ring->curr % AG71XX_RX_RING_SIZE;
		struct ag71xx_desc *desc = ring->buf[i].desc;
		int pktlen;
		
		if (ag71xx_desc_empty(desc))
			break;

		DBG("%s: rx packets, curr=%u\n", dev->name, ring->curr);

        pktlen = ag71xx_desc_pktlen(desc);
		pktlen -= ETH_FCS_LEN;


		NetReceive(NetRxPackets[i] , pktlen);
		flush_cache( (u32) NetRxPackets[i], PKTSIZE_ALIGN);

        ring->buf[i].desc->ctrl = DESC_EMPTY;
		ring->curr++;
		if (ring->curr >= AG71XX_RX_RING_SIZE){
			ring->curr = 0;
		}

    }

	if ((ag71xx_rr(ag, AG71XX_REG_RX_CTRL) & RX_CTRL_RXE) == 0) {
		/* start RX engine */
		ag71xx_wr(ag, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);
	}
	
	return 0;
}

#ifdef AG71XX_DEBUG
static char *ag71xx_speed_str(struct ag71xx *ag)
{
	switch (ag->speed) {
	case SPEED_1000:
		return "1000";
	case SPEED_100:
		return "100";
	case SPEED_10:
		return "10";
	}

	return "?";
}
#endif

void ag71xx_link_adjust(struct ag71xx *ag)
{
	u32 cfg2;
	u32 ifctl;
	u32 fifo5;
	u32 mii_speed;

	if (!ag->link) {
		DBG("%s: link down\n", ag->dev->name);
		return;
	}

	cfg2 = ag71xx_rr(ag, AG71XX_REG_MAC_CFG2);
	cfg2 &= ~(MAC_CFG2_IF_1000 | MAC_CFG2_IF_10_100 | MAC_CFG2_FDX);
	cfg2 |= (ag->duplex) ? MAC_CFG2_FDX : 0;

	ifctl = ag71xx_rr(ag, AG71XX_REG_MAC_IFCTL);
	ifctl &= ~(MAC_IFCTL_SPEED);

	fifo5 = ag71xx_rr(ag, AG71XX_REG_FIFO_CFG5);
	fifo5 &= ~FIFO_CFG5_BM;

	switch (ag->speed) {
	case SPEED_1000:
		mii_speed =  MII_CTRL_SPEED_1000;
		cfg2 |= MAC_CFG2_IF_1000;
		fifo5 |= FIFO_CFG5_BM;
		break;
	case SPEED_100:
		mii_speed = MII_CTRL_SPEED_100;
		cfg2 |= MAC_CFG2_IF_10_100;
		ifctl |= MAC_IFCTL_SPEED;
		break;
	case SPEED_10:
		mii_speed = MII_CTRL_SPEED_10;
		cfg2 |= MAC_CFG2_IF_10_100;
		break;
	default:
		BUG();
		return;
	}

    ag71xx_wr(ag, AG71XX_REG_FIFO_CFG3, 0x00780fff);

    if (ag->macNum == 0)
        ar91xx_set_pll_ge0(ag->speed);
    else
        ar91xx_set_pll_ge1(ag->speed);

	ag71xx_mii_ctrl_set_speed(ag, mii_speed);

	ag71xx_wr(ag, AG71XX_REG_MAC_CFG2, cfg2);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, fifo5);
	ag71xx_wr(ag, AG71XX_REG_MAC_IFCTL, ifctl);

    DBG("%s: link up (%sMbps/%s duplex)\n",
        ag->dev->name,
        ag71xx_speed_str(ag),
        (1 == ag->duplex) ? "Full" : "Half");

	DBG("%s: fifo_cfg0=%#x, fifo_cfg1=%#x, fifo_cfg2=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG0),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG1),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG2));

	DBG("%s: fifo_cfg3=%#x, fifo_cfg4=%#x, fifo_cfg5=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG3),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG4),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG5));

	DBG("%s: mac_cfg2=%#x, mac_ifctl=%#x, mii_ctrl=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_MAC_CFG2),
		ag71xx_rr(ag, AG71XX_REG_MAC_IFCTL),
		ag71xx_mii_ctrl_rr(ag));
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int ag71xx_getMiiSpeed(struct ag71xx *ag) 
{
    uint16_t phyreg, cap;

    if (miiphy_read(ag->phyname, ag->phyid,
                    PHY_BMSR, &phyreg)) {
        puts("PHY_BMSR read failed, assuming no link\n");
        return -1;
    }

    if ((phyreg & PHY_BMSR_LS) == 0) {
        return -1;
    }

    if (miiphy_read(ag->phyname, ag->phyid,
                PHY_1000BTSR, &phyreg))
        return -1;

    if (phyreg & PHY_1000BTSR_1000FD) {
        ag->speed = SPEED_1000;
        ag->duplex = 1;
    } else if (phyreg & PHY_1000BTSR_1000HD) {
        ag->speed = SPEED_1000;
        ag->duplex = 0;
    } else {
        if (miiphy_read(ag->phyname, ag->phyid,
                PHY_ANAR, &cap))
            return -1;

        if (miiphy_read(ag->phyname, ag->phyid,
                PHY_ANLPAR, &phyreg))
            return -1;

        cap &= phyreg;
        if (cap & PHY_ANLPAR_TXFD) {
            ag->speed = SPEED_100;
            ag->duplex = 1;
        } else if (cap & PHY_ANLPAR_TX) {
            ag->speed = SPEED_100;
            ag->duplex = 0;
        } else if (cap & PHY_ANLPAR_10FD) {
            ag->speed = SPEED_10;
            ag->duplex = 1;
        } else {
            ag->speed = SPEED_10;
            ag->duplex = 0;
        }
    }
	
	ag->link = 1;
	
	return 0;
}
#endif

static int ag71xx_hw_start(struct eth_device *dev, bd_t * bd)
{
	struct ag71xx *ag = (struct ag71xx *) dev->priv;

	ag71xx_dma_reset(ag);

    ag71xx_ring_rx_clean(ag);
	ag71xx_ring_tx_init(ag);
	
	ag71xx_wr(ag, AG71XX_REG_TX_DESC, 
				(u32) virt_to_phys(ag->tx_ring.descs_dma));
	ag71xx_wr(ag, AG71XX_REG_RX_DESC,
				(u32) virt_to_phys(ag->rx_ring.descs_dma));

	ag71xx_hw_set_macaddr(ag, ag->dev->enetaddr);

    if (ag->phyfixed) {
        ag->link = 1;
        ag->duplex = 1;
        ag->speed = SPEED_1000;
    } else {

#if (defined(CONFIG_MII) || defined(CONFIG_CMD_MII))
		if (ag71xx_getMiiSpeed(ag))
			return -1;
#else
		/* only fixed, without mii */
		return -1;
#endif

    }
    ag71xx_link_adjust(ag);
	
	DBG("%s: txdesc reg: %#08x rxdesc reg: %#08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_TX_DESC),
		ag71xx_rr(ag, AG71XX_REG_RX_DESC));
	
	/* start RX engine */
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);
	
	return 0;
}

#define FIFO_CFG0_INIT	(FIFO_CFG0_ALL << FIFO_CFG0_ENABLE_SHIFT)

#define FIFO_CFG4_INIT	(FIFO_CFG4_DE | FIFO_CFG4_DV | FIFO_CFG4_FC | \
			 FIFO_CFG4_CE | FIFO_CFG4_CR | FIFO_CFG4_LM | \
			 FIFO_CFG4_LO | FIFO_CFG4_OK | FIFO_CFG4_MC | \
			 FIFO_CFG4_BC | FIFO_CFG4_DR | FIFO_CFG4_LE | \
			 FIFO_CFG4_CF | FIFO_CFG4_PF | FIFO_CFG4_UO | \
			 FIFO_CFG4_VT)

#define FIFO_CFG5_INIT	(FIFO_CFG5_DE | FIFO_CFG5_DV | FIFO_CFG5_FC | \
			 FIFO_CFG5_CE | FIFO_CFG5_LO | FIFO_CFG5_OK | \
			 FIFO_CFG5_MC | FIFO_CFG5_BC | FIFO_CFG5_DR | \
			 FIFO_CFG5_CF | FIFO_CFG5_PF | FIFO_CFG5_VT | \
			 FIFO_CFG5_LE | FIFO_CFG5_FT | FIFO_CFG5_16 | \
			 FIFO_CFG5_17 | FIFO_CFG5_SF)

static int ag71xx_hw_init(struct ag71xx *ag)
{
    int ret = 0;
	uint32_t reg;
	uint32_t mask, mii_type;

    if (ag->macNum == 0) {
        mask = (RESET_MODULE_GE0_MAC | RESET_MODULE_GE0_PHY);
        mii_type = 0x13;
    } else {
        mask = (RESET_MODULE_GE1_MAC | RESET_MODULE_GE1_PHY);
        mii_type = 0x11;
    }

    // mac soft reset
    ag71xx_sb(ag, AG71XX_REG_MAC_CFG1, MAC_CFG1_SR);
    udelay(20);
	
	// device stop
	reg = ar71xx_reset_rr(AR91XX_RESET_REG_RESET_MODULE);
	ar71xx_reset_wr(AR91XX_RESET_REG_RESET_MODULE, reg | mask);
	udelay(100 * 1000);
	
    // device start
    reg = ar71xx_reset_rr(AR91XX_RESET_REG_RESET_MODULE);
    ar71xx_reset_wr(AR91XX_RESET_REG_RESET_MODULE, reg & ~mask);
    udelay(100 * 1000);

    /* setup MAC configuration registers */
    ag71xx_wr(ag, AG71XX_REG_MAC_CFG1, (MAC_CFG1_RXE | MAC_CFG1_TXE));

    ag71xx_sb(ag, AG71XX_REG_MAC_CFG2,
          MAC_CFG2_PAD_CRC_EN | MAC_CFG2_LEN_CHECK);

    /* setup FIFO configuration register 0 */
    ag71xx_wr(ag, AG71XX_REG_FIFO_CFG0, FIFO_CFG0_INIT);

    /* setup MII interface type */
    ag71xx_mii_ctrl_set_if(ag, ag->mii_if);

    /* setup mdio clock divisor */
    ag71xx_wr(ag, AG71XX_REG_MII_CFG, MII_CFG_CLK_DIV_20);
	
	/* setup FIFO configuration registers */
	ag71xx_sb(ag, AG71XX_REG_FIFO_CFG4, FIFO_CFG4_INIT);
    ag71xx_wr(ag, AG71XX_REG_FIFO_CFG1, 0x0fff0000);
    ag71xx_wr(ag, AG71XX_REG_FIFO_CFG2, 0x00001fff);
    ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, FIFO_CFG5_INIT);

    ag71xx_dma_reset(ag);

    ret = ag71xx_rings_init(ag);
    if (ret)
        return -1;

	ag71xx_wr(ag, AG71XX_REG_TX_DESC, 
				(u32) virt_to_phys(ag->tx_ring.descs_dma));
	ag71xx_wr(ag, AG71XX_REG_RX_DESC,
				(u32) virt_to_phys(ag->rx_ring.descs_dma));
		
	ag71xx_hw_set_macaddr(ag, ag->dev->enetaddr);
	
    return 0;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
#define AG71XX_MDIO_RETRY	1000
#define AG71XX_MDIO_DELAY	5

static inline struct ag71xx *ag71xx_name2mac(char *devname)
{
    if (strcmp(devname, agtable[0].dev->name) == 0)
        return &agtable[0];
    else if (strcmp(devname, agtable[1].dev->name) == 0)
        return &agtable[1];
    else
        return NULL;
}

static inline void ag71xx_mdio_wr(struct ag71xx *ag, unsigned reg,
				  u32 value)
{
	uint32_t r;

	r = ag->mac_base + reg;
	writel(value, r);

	/* flush write */
	(void) readl(r);
}

static inline u32 ag71xx_mdio_rr(struct ag71xx *ag, unsigned reg)
{
	return readl(ag->mac_base + reg);
}

static int ag71xx_mdio_read(char *devname, unsigned char addr,
                            unsigned char reg, unsigned short *val)
{
	struct ag71xx *ag = ag71xx_name2mac(devname);
	uint16_t regData;
	int i;

	ag71xx_mdio_wr(ag, AG71XX_REG_MII_CMD, MII_CMD_WRITE);
	ag71xx_mdio_wr(ag, AG71XX_REG_MII_ADDR,
			((addr & 0xff) << MII_ADDR_SHIFT) | (reg & 0xff));
	ag71xx_mdio_wr(ag, AG71XX_REG_MII_CMD, MII_CMD_READ);

	i = AG71XX_MDIO_RETRY;
	while (ag71xx_mdio_rr(ag, AG71XX_REG_MII_IND) & MII_IND_BUSY) {
		if (i-- == 0) {
			printf("%s: mii_read timed out\n",
				ag->dev->name);
			return -1;
		}
		udelay(AG71XX_MDIO_DELAY);
	}

	regData = (uint16_t) ag71xx_mdio_rr(ag, AG71XX_REG_MII_STATUS) & 0xffff;
	ag71xx_mdio_wr(ag, AG71XX_REG_MII_CMD, MII_CMD_WRITE);

	DBG("mii_read: addr=%04x, reg=%04x, value=%04x\n", addr, reg, regData);

    if (val)
        *val = regData;

	return 0;
}

static int ag71xx_mdio_write(char *devname, unsigned char addr,
                            unsigned char reg, unsigned short val)
{
	struct ag71xx *ag = ag71xx_name2mac(devname);
	int i;

    if (ag == NULL)
        return 1;

	DBG("mii_write: addr=%04x, reg=%04x, value=%04x\n", addr, reg, val);

	ag71xx_mdio_wr(ag, AG71XX_REG_MII_ADDR,
			((addr & 0xff) << MII_ADDR_SHIFT) | (reg & 0xff));
	ag71xx_mdio_wr(ag, AG71XX_REG_MII_CTRL, val);

	i = AG71XX_MDIO_RETRY;
	while (ag71xx_mdio_rr(ag, AG71XX_REG_MII_IND) & MII_IND_BUSY) {
		if (i-- == 0) {
			printf("%s: mii_write timed out\n",
				ag->dev->name);
			break;
		}
		udelay(AG71XX_MDIO_DELAY);
	}

	return 0;
}
#endif

int ag71xx_register(bd_t * bis, char *phyname[], uint16_t phyid[], uint16_t phyfixed[])
{
    int i, num = 0;
    u8 used_ports[MAX_AG71XX_DEVS] = CONFIG_AG71XX_PORTS;

	for (i = 0; i < MAX_AG71XX_DEVS; i++) {
		/*skip if port is configured not to use */
		if (used_ports[i] == 0)
			continue;

		agtable[i].dev = malloc(sizeof(struct eth_device));
		if (agtable[i].dev == NULL) {
			puts("malloc failed\n");
			return 0;
        }
		memset(agtable[i].dev, 0, sizeof(struct eth_device));
		sprintf(agtable[i].dev->name, "eth%d", i);

		agtable[i].dev->iobase = 0;
		agtable[i].dev->init = ag71xx_hw_start;
		agtable[i].dev->halt = ag71xx_halt;
		agtable[i].dev->send = ag71xx_send;
		agtable[i].dev->recv = ag71xx_recv;
		agtable[i].dev->priv = (void *) (&agtable[i]);
		agtable[i].macNum = i;
		eth_register(agtable[i].dev);
#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

        if ((phyname == NULL) || (phyid == NULL) || (phyfixed == NULL))
            return -1;

        agtable[i].phyname = strdup(phyname[i]);
        agtable[i].phyid = phyid[i];
        agtable[i].phyfixed = phyfixed[i];

        miiphy_register(agtable[i].dev->name, ag71xx_mdio_read,
			ag71xx_mdio_write);
#endif

		if (ag71xx_hw_init(&agtable[i]))
			continue;

        num++;
	}

    return num;
}
