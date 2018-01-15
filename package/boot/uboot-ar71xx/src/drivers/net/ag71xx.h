/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef __AG71XX_H
#define __AG71XX_H

#include <linux/types.h>
#include <linux/bitops.h>

#include <asm/ar71xx.h>

// controller has 2 ports
#define MAX_AG71XX_DEVS 2

#define ETH_FCS_LEN	4

#define SPEED_10        10
#define SPEED_100       100
#define SPEED_1000      1000


#define AG71XX_INT_ERR	(AG71XX_INT_RX_BE | AG71XX_INT_TX_BE)
#define AG71XX_INT_TX	(AG71XX_INT_TX_PS)
#define AG71XX_INT_RX	(AG71XX_INT_RX_PR | AG71XX_INT_RX_OF)

#define AG71XX_INT_POLL	(AG71XX_INT_RX | AG71XX_INT_TX)
#define AG71XX_INT_INIT	(AG71XX_INT_ERR | AG71XX_INT_POLL)

#define AG71XX_TX_FIFO_LEN	2048
#define AG71XX_TX_MTU_LEN	1536
#define AG71XX_RX_PKT_RESERVE	64
#define AG71XX_RX_PKT_SIZE	\
	(AG71XX_RX_PKT_RESERVE + ETH_HLEN + ETH_FRAME_LEN + ETH_FCS_LEN)

#ifndef CONFIG_SYS_RX_ETH_BUFFER
#define AG71XX_TX_RING_SIZE	4
#define AG71XX_RX_RING_SIZE	4
#else
#define AG71XX_TX_RING_SIZE	CONFIG_SYS_RX_ETH_BUFFER
#define AG71XX_RX_RING_SIZE	CONFIG_SYS_RX_ETH_BUFFER
#endif

#define AG71XX_TX_THRES_STOP	(AG71XX_TX_RING_SIZE - 4)
#define AG71XX_TX_THRES_WAKEUP	\
		(AG71XX_TX_RING_SIZE - (AG71XX_TX_RING_SIZE / 4))




struct ag71xx_desc {
	u32	data;
	u32	ctrl;
#define DESC_EMPTY	BIT(31)
#define DESC_MORE	BIT(24)
#define DESC_PKTLEN_M	0xfff
	u32	next;
	u32	pad;
} __attribute__((aligned(4)));

struct ag71xx_buf {
	struct sk_buff		*skb;
	struct ag71xx_desc 	*desc;
	dma_addr_t		dma_addr;
	u32			pad;
};

struct ag71xx_ring {
	struct ag71xx_buf	*buf;
	u8			*descs_cpu;
	u8		    *descs_dma;
	unsigned int		desc_size;
	unsigned int		curr;
	unsigned int		size;
};

struct ag71xx {
	uint32_t		    mac_base;
	uint32_t		    mii_ctrl;

	struct eth_device	*dev;

	struct ag71xx_ring	rx_ring;
	struct ag71xx_ring	tx_ring;

    char               *phyname;
    u16                 phyid;
    u16                 phyfixed;
	uint32_t	    	link;
	uint32_t	    	speed;
	int32_t			    duplex;
    uint32_t            macNum;
    uint32_t            mii_if;
};

void ag71xx_link_adjust(struct ag71xx *ag);

int ag71xx_phy_connect(struct ag71xx *ag);
void ag71xx_phy_disconnect(struct ag71xx *ag);
void ag71xx_phy_start(struct ag71xx *ag);
void ag71xx_phy_stop(struct ag71xx *ag);

static inline int ag71xx_desc_empty(struct ag71xx_desc *desc)
{
	return ((desc->ctrl & DESC_EMPTY) != 0);
}

static inline int ag71xx_desc_pktlen(struct ag71xx_desc *desc)
{
	return (desc->ctrl & DESC_PKTLEN_M);
}

/* Register offsets */
#define AG71XX_REG_MAC_CFG1	0x0000
#define AG71XX_REG_MAC_CFG2	0x0004
#define AG71XX_REG_MAC_IPG	0x0008
#define AG71XX_REG_MAC_HDX	0x000c
#define AG71XX_REG_MAC_MFL	0x0010
#define AG71XX_REG_MII_CFG	0x0020
#define AG71XX_REG_MII_CMD	0x0024
#define AG71XX_REG_MII_ADDR	0x0028
#define AG71XX_REG_MII_CTRL	0x002c
#define AG71XX_REG_MII_STATUS	0x0030
#define AG71XX_REG_MII_IND	0x0034
#define AG71XX_REG_MAC_IFCTL	0x0038
#define AG71XX_REG_MAC_ADDR1	0x0040
#define AG71XX_REG_MAC_ADDR2	0x0044
#define AG71XX_REG_FIFO_CFG0	0x0048
#define AG71XX_REG_FIFO_CFG1	0x004c
#define AG71XX_REG_FIFO_CFG2	0x0050
#define AG71XX_REG_FIFO_CFG3	0x0054
#define AG71XX_REG_FIFO_CFG4	0x0058
#define AG71XX_REG_FIFO_CFG5	0x005c
#define AG71XX_REG_FIFO_RAM0	0x0060
#define AG71XX_REG_FIFO_RAM1	0x0064
#define AG71XX_REG_FIFO_RAM2	0x0068
#define AG71XX_REG_FIFO_RAM3	0x006c
#define AG71XX_REG_FIFO_RAM4	0x0070
#define AG71XX_REG_FIFO_RAM5	0x0074
#define AG71XX_REG_FIFO_RAM6	0x0078
#define AG71XX_REG_FIFO_RAM7	0x007c

#define AG71XX_REG_TX_CTRL	0x0180
#define AG71XX_REG_TX_DESC	0x0184
#define AG71XX_REG_TX_STATUS	0x0188
#define AG71XX_REG_RX_CTRL	0x018c
#define AG71XX_REG_RX_DESC	0x0190
#define AG71XX_REG_RX_STATUS	0x0194
#define AG71XX_REG_INT_ENABLE	0x0198
#define AG71XX_REG_INT_STATUS	0x019c

#define MAC_CFG1_TXE		BIT(0)	/* Tx Enable */
#define MAC_CFG1_STX		BIT(1)	/* Synchronize Tx Enable */
#define MAC_CFG1_RXE		BIT(2)	/* Rx Enable */
#define MAC_CFG1_SRX		BIT(3)	/* Synchronize Rx Enable */
#define MAC_CFG1_TFC		BIT(4)	/* Tx Flow Control Enable */
#define MAC_CFG1_RFC		BIT(5)	/* Rx Flow Control Enable */
#define MAC_CFG1_LB		BIT(8)	/* Loopback mode */
#define MAC_CFG1_SR		BIT(31)	/* Soft Reset */

#define MAC_CFG2_FDX		BIT(0)
#define MAC_CFG2_CRC_EN		BIT(1)
#define MAC_CFG2_PAD_CRC_EN	BIT(2)
#define MAC_CFG2_LEN_CHECK	BIT(4)
#define MAC_CFG2_HUGE_FRAME_EN	BIT(5)
#define MAC_CFG2_IF_1000	BIT(9)
#define MAC_CFG2_IF_10_100	BIT(8)

#define FIFO_CFG0_WTM		BIT(0)	/* Watermark Module */
#define FIFO_CFG0_RXS		BIT(1)	/* Rx System Module */
#define FIFO_CFG0_RXF		BIT(2)	/* Rx Fabric Module */
#define FIFO_CFG0_TXS		BIT(3)	/* Tx System Module */
#define FIFO_CFG0_TXF		BIT(4)	/* Tx Fabric Module */
#define FIFO_CFG0_ALL	(FIFO_CFG0_WTM | FIFO_CFG0_RXS | FIFO_CFG0_RXF \
			| FIFO_CFG0_TXS | FIFO_CFG0_TXF)

#define FIFO_CFG0_ENABLE_SHIFT	8

#define FIFO_CFG4_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG4_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG4_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG4_CE		BIT(3)	/* Code Error */
#define FIFO_CFG4_CR		BIT(4)	/* CRC error */
#define FIFO_CFG4_LM		BIT(5)	/* Length Mismatch */
#define FIFO_CFG4_LO		BIT(6)	/* Length out of range */
#define FIFO_CFG4_OK		BIT(7)	/* Packet is OK */
#define FIFO_CFG4_MC		BIT(8)	/* Multicast Packet */
#define FIFO_CFG4_BC		BIT(9)	/* Broadcast Packet */
#define FIFO_CFG4_DR		BIT(10)	/* Dribble */
#define FIFO_CFG4_LE		BIT(11)	/* Long Event */
#define FIFO_CFG4_CF		BIT(12)	/* Control Frame */
#define FIFO_CFG4_PF		BIT(13)	/* Pause Frame */
#define FIFO_CFG4_UO		BIT(14)	/* Unsupported Opcode */
#define FIFO_CFG4_VT		BIT(15)	/* VLAN tag detected */
#define FIFO_CFG4_FT		BIT(16)	/* Frame Truncated */
#define FIFO_CFG4_UC		BIT(17)	/* Unicast Packet */

#define FIFO_CFG5_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG5_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG5_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG5_CE		BIT(3)	/* Code Error */
#define FIFO_CFG5_LM		BIT(4)	/* Length Mismatch */
#define FIFO_CFG5_LO		BIT(5)	/* Length Out of Range */
#define FIFO_CFG5_OK		BIT(6)	/* Packet is OK */
#define FIFO_CFG5_MC		BIT(7)	/* Multicast Packet */
#define FIFO_CFG5_BC		BIT(8)	/* Broadcast Packet */
#define FIFO_CFG5_DR		BIT(9)	/* Dribble */
#define FIFO_CFG5_CF		BIT(10)	/* Control Frame */
#define FIFO_CFG5_PF		BIT(11)	/* Pause Frame */
#define FIFO_CFG5_UO		BIT(12)	/* Unsupported Opcode */
#define FIFO_CFG5_VT		BIT(13)	/* VLAN tag detected */
#define FIFO_CFG5_LE		BIT(14)	/* Long Event */
#define FIFO_CFG5_FT		BIT(15)	/* Frame Truncated */
#define FIFO_CFG5_16		BIT(16)	/* unknown */
#define FIFO_CFG5_17		BIT(17)	/* unknown */
#define FIFO_CFG5_SF		BIT(18)	/* Short Frame */
#define FIFO_CFG5_BM		BIT(19)	/* Byte Mode */

#define AG71XX_INT_TX_PS	BIT(0)
#define AG71XX_INT_TX_UR	BIT(1)
#define AG71XX_INT_TX_BE	BIT(3)
#define AG71XX_INT_RX_PR	BIT(4)
#define AG71XX_INT_RX_OF	BIT(6)
#define AG71XX_INT_RX_BE	BIT(7)

#define MAC_IFCTL_SPEED		BIT(16)

#define MII_CFG_CLK_DIV_4	0
#define MII_CFG_CLK_DIV_6	2
#define MII_CFG_CLK_DIV_8	3
#define MII_CFG_CLK_DIV_10	4
#define MII_CFG_CLK_DIV_14	5
#define MII_CFG_CLK_DIV_20	6
#define MII_CFG_CLK_DIV_28	7
#define MII_CFG_RESET		BIT(31)

#define MII_CMD_WRITE		0x0
#define MII_CMD_READ		0x1
#define MII_ADDR_SHIFT		8
#define MII_IND_BUSY		BIT(0)
#define MII_IND_INVALID		BIT(2)

#define TX_CTRL_TXE		BIT(0)	/* Tx Enable */

#define TX_STATUS_PS		BIT(0)	/* Packet Sent */
#define TX_STATUS_UR		BIT(1)	/* Tx Underrun */
#define TX_STATUS_BE		BIT(3)	/* Bus Error */

#define RX_CTRL_RXE		BIT(0)	/* Rx Enable */

#define RX_STATUS_PR		BIT(0)	/* Packet Received */
#define RX_STATUS_OF		BIT(2)	/* Rx Overflow */
#define RX_STATUS_BE		BIT(3)	/* Bus Error */

#define MII_CTRL_IF_MASK	3
#define MII_CTRL_SPEED_SHIFT	4
#define MII_CTRL_SPEED_MASK	3
#define MII_CTRL_SPEED_10	0
#define MII_CTRL_SPEED_100	1
#define MII_CTRL_SPEED_1000	2

static inline void ag71xx_wr(struct ag71xx *ag, unsigned reg, u32 value)
{
	__raw_writel(value, ag->mac_base + reg);
	/* flush write */
	(void) __raw_readl(ag->mac_base + reg);
}

static inline u32 ag71xx_rr(struct ag71xx *ag, unsigned reg)
{
	return __raw_readl(ag->mac_base + reg);
}

static inline void ag71xx_sb(struct ag71xx *ag, unsigned reg, u32 mask)
{
	uint32_t r;

	r = ag->mac_base + reg;
	__raw_writel(__raw_readl(r) | mask, r);
	/* flush write */
	(void)__raw_readl(r);
}

static inline void ag71xx_cb(struct ag71xx *ag, unsigned reg, u32 mask)
{
	uint32_t r;

	r = ag->mac_base + reg;
	__raw_writel(__raw_readl(r) & ~mask, r);
	/* flush write */
	(void) __raw_readl(r);
}

static inline void ag71xx_int_enable(struct ag71xx *ag, u32 ints)
{
	ag71xx_sb(ag, AG71XX_REG_INT_ENABLE, ints);
}

static inline void ag71xx_int_disable(struct ag71xx *ag, u32 ints)
{
	ag71xx_cb(ag, AG71XX_REG_INT_ENABLE, ints);
}

static inline void ag71xx_mii_ctrl_wr(struct ag71xx *ag, u32 value)
{
	__raw_writel(value, ag->mii_ctrl);

	/* flush write */
	__raw_readl(ag->mii_ctrl);
}

static inline u32 ag71xx_mii_ctrl_rr(struct ag71xx *ag)
{
	return __raw_readl(ag->mii_ctrl);
}

static void inline ag71xx_mii_ctrl_set_if(struct ag71xx *ag,
					  unsigned int mii_if)
{
	u32 t;

	t = ag71xx_mii_ctrl_rr(ag);
	t &= ~(MII_CTRL_IF_MASK);
	t |= (mii_if & MII_CTRL_IF_MASK);
	ag71xx_mii_ctrl_wr(ag, t);
}

static void inline ag71xx_mii_ctrl_set_speed(struct ag71xx *ag,
					     unsigned int speed)
{
	u32 t;

	t = ag71xx_mii_ctrl_rr(ag);
	t &= ~(MII_CTRL_SPEED_MASK << MII_CTRL_SPEED_SHIFT);
	t |= (speed & MII_CTRL_SPEED_MASK) << MII_CTRL_SPEED_SHIFT;
	ag71xx_mii_ctrl_wr(ag, t);
}

#ifdef CONFIG_AG71XX_AR8216_SUPPORT
void ag71xx_add_ar8216_header(struct ag71xx *ag, struct sk_buff *skb);
int ag71xx_remove_ar8216_header(struct ag71xx *ag, struct sk_buff *skb,
				int pktlen);
static inline int ag71xx_has_ar8216(struct ag71xx *ag)
{
	return ag71xx_get_pdata(ag)->has_ar8216;
}
#else
static inline void ag71xx_add_ar8216_header(struct ag71xx *ag,
					   struct sk_buff *skb)
{
}

static inline int ag71xx_remove_ar8216_header(struct ag71xx *ag,
					      struct sk_buff *skb,
					      int pktlen)
{
	return 0;
}
static inline int ag71xx_has_ar8216(struct ag71xx *ag)
{
	return 0;
}
#endif

#endif /* _AG71XX_H */
