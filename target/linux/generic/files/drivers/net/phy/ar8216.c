/*
 * ar8216.c: AR8216 switch driver
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
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
 */

#include <linux/if.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/ar8216_platform.h>
#include <linux/workqueue.h>
#include <linux/version.h>

#include "ar8216.h"

extern const struct ar8xxx_chip ar8327_chip;
extern const struct ar8xxx_chip ar8337_chip;

#define MIB_DESC_BASIC(_s , _o, _n)		\
	{					\
		.size = (_s),			\
		.offset = (_o),			\
		.name = (_n),			\
		.type = AR8XXX_MIB_BASIC,	\
	}

#define MIB_DESC_EXT(_s , _o, _n)		\
	{					\
		.size = (_s),			\
		.offset = (_o),			\
		.name = (_n),			\
		.type = AR8XXX_MIB_EXTENDED,	\
	}

static const struct ar8xxx_mib_desc ar8216_mibs[] = {
	MIB_DESC_EXT(1, AR8216_STATS_RXBROAD, "RxBroad"),
	MIB_DESC_EXT(1, AR8216_STATS_RXPAUSE, "RxPause"),
	MIB_DESC_EXT(1, AR8216_STATS_RXMULTI, "RxMulti"),
	MIB_DESC_EXT(1, AR8216_STATS_RXFCSERR, "RxFcsErr"),
	MIB_DESC_EXT(1, AR8216_STATS_RXALIGNERR, "RxAlignErr"),
	MIB_DESC_EXT(1, AR8216_STATS_RXRUNT, "RxRunt"),
	MIB_DESC_EXT(1, AR8216_STATS_RXFRAGMENT, "RxFragment"),
	MIB_DESC_EXT(1, AR8216_STATS_RX64BYTE, "Rx64Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_RX128BYTE, "Rx128Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_RX256BYTE, "Rx256Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_RX512BYTE, "Rx512Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_RX1024BYTE, "Rx1024Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_RXMAXBYTE, "RxMaxByte"),
	MIB_DESC_EXT(1, AR8216_STATS_RXTOOLONG, "RxTooLong"),
	MIB_DESC_BASIC(2, AR8216_STATS_RXGOODBYTE, "RxGoodByte"),
	MIB_DESC_EXT(2, AR8216_STATS_RXBADBYTE, "RxBadByte"),
	MIB_DESC_EXT(1, AR8216_STATS_RXOVERFLOW, "RxOverFlow"),
	MIB_DESC_EXT(1, AR8216_STATS_FILTERED, "Filtered"),
	MIB_DESC_EXT(1, AR8216_STATS_TXBROAD, "TxBroad"),
	MIB_DESC_EXT(1, AR8216_STATS_TXPAUSE, "TxPause"),
	MIB_DESC_EXT(1, AR8216_STATS_TXMULTI, "TxMulti"),
	MIB_DESC_EXT(1, AR8216_STATS_TXUNDERRUN, "TxUnderRun"),
	MIB_DESC_EXT(1, AR8216_STATS_TX64BYTE, "Tx64Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_TX128BYTE, "Tx128Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_TX256BYTE, "Tx256Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_TX512BYTE, "Tx512Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_TX1024BYTE, "Tx1024Byte"),
	MIB_DESC_EXT(1, AR8216_STATS_TXMAXBYTE, "TxMaxByte"),
	MIB_DESC_EXT(1, AR8216_STATS_TXOVERSIZE, "TxOverSize"),
	MIB_DESC_BASIC(2, AR8216_STATS_TXBYTE, "TxByte"),
	MIB_DESC_EXT(1, AR8216_STATS_TXCOLLISION, "TxCollision"),
	MIB_DESC_EXT(1, AR8216_STATS_TXABORTCOL, "TxAbortCol"),
	MIB_DESC_EXT(1, AR8216_STATS_TXMULTICOL, "TxMultiCol"),
	MIB_DESC_EXT(1, AR8216_STATS_TXSINGLECOL, "TxSingleCol"),
	MIB_DESC_EXT(1, AR8216_STATS_TXEXCDEFER, "TxExcDefer"),
	MIB_DESC_EXT(1, AR8216_STATS_TXDEFER, "TxDefer"),
	MIB_DESC_EXT(1, AR8216_STATS_TXLATECOL, "TxLateCol"),
};

const struct ar8xxx_mib_desc ar8236_mibs[39] = {
	MIB_DESC_EXT(1, AR8236_STATS_RXBROAD, "RxBroad"),
	MIB_DESC_EXT(1, AR8236_STATS_RXPAUSE, "RxPause"),
	MIB_DESC_EXT(1, AR8236_STATS_RXMULTI, "RxMulti"),
	MIB_DESC_EXT(1, AR8236_STATS_RXFCSERR, "RxFcsErr"),
	MIB_DESC_EXT(1, AR8236_STATS_RXALIGNERR, "RxAlignErr"),
	MIB_DESC_EXT(1, AR8236_STATS_RXRUNT, "RxRunt"),
	MIB_DESC_EXT(1, AR8236_STATS_RXFRAGMENT, "RxFragment"),
	MIB_DESC_EXT(1, AR8236_STATS_RX64BYTE, "Rx64Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RX128BYTE, "Rx128Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RX256BYTE, "Rx256Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RX512BYTE, "Rx512Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RX1024BYTE, "Rx1024Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RX1518BYTE, "Rx1518Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_RXMAXBYTE, "RxMaxByte"),
	MIB_DESC_EXT(1, AR8236_STATS_RXTOOLONG, "RxTooLong"),
	MIB_DESC_BASIC(2, AR8236_STATS_RXGOODBYTE, "RxGoodByte"),
	MIB_DESC_EXT(2, AR8236_STATS_RXBADBYTE, "RxBadByte"),
	MIB_DESC_EXT(1, AR8236_STATS_RXOVERFLOW, "RxOverFlow"),
	MIB_DESC_EXT(1, AR8236_STATS_FILTERED, "Filtered"),
	MIB_DESC_EXT(1, AR8236_STATS_TXBROAD, "TxBroad"),
	MIB_DESC_EXT(1, AR8236_STATS_TXPAUSE, "TxPause"),
	MIB_DESC_EXT(1, AR8236_STATS_TXMULTI, "TxMulti"),
	MIB_DESC_EXT(1, AR8236_STATS_TXUNDERRUN, "TxUnderRun"),
	MIB_DESC_EXT(1, AR8236_STATS_TX64BYTE, "Tx64Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TX128BYTE, "Tx128Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TX256BYTE, "Tx256Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TX512BYTE, "Tx512Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TX1024BYTE, "Tx1024Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TX1518BYTE, "Tx1518Byte"),
	MIB_DESC_EXT(1, AR8236_STATS_TXMAXBYTE, "TxMaxByte"),
	MIB_DESC_EXT(1, AR8236_STATS_TXOVERSIZE, "TxOverSize"),
	MIB_DESC_BASIC(2, AR8236_STATS_TXBYTE, "TxByte"),
	MIB_DESC_EXT(1, AR8236_STATS_TXCOLLISION, "TxCollision"),
	MIB_DESC_EXT(1, AR8236_STATS_TXABORTCOL, "TxAbortCol"),
	MIB_DESC_EXT(1, AR8236_STATS_TXMULTICOL, "TxMultiCol"),
	MIB_DESC_EXT(1, AR8236_STATS_TXSINGLECOL, "TxSingleCol"),
	MIB_DESC_EXT(1, AR8236_STATS_TXEXCDEFER, "TxExcDefer"),
	MIB_DESC_EXT(1, AR8236_STATS_TXDEFER, "TxDefer"),
	MIB_DESC_EXT(1, AR8236_STATS_TXLATECOL, "TxLateCol"),
};

static DEFINE_MUTEX(ar8xxx_dev_list_lock);
static LIST_HEAD(ar8xxx_dev_list);

static void
ar8xxx_mib_start(struct ar8xxx_priv *priv);
static void
ar8xxx_mib_stop(struct ar8xxx_priv *priv);

/* inspired by phy_poll_reset in drivers/net/phy/phy_device.c */
static int
ar8xxx_phy_poll_reset(struct mii_bus *bus)
{
        unsigned int sleep_msecs = 20;
        int ret, elapsed, i;

        for (elapsed = sleep_msecs; elapsed <= 600;
	     elapsed += sleep_msecs) {
                msleep(sleep_msecs);
                for (i = 0; i < AR8XXX_NUM_PHYS; i++) {
                        ret = mdiobus_read(bus, i, MII_BMCR);
                        if (ret < 0)
				return ret;
                        if (ret & BMCR_RESET)
				break;
                        if (i == AR8XXX_NUM_PHYS - 1) {
                                usleep_range(1000, 2000);
                                return 0;
                        }
                }
        }
        return -ETIMEDOUT;
}

static int
ar8xxx_phy_check_aneg(struct phy_device *phydev)
{
	int ret;

	if (phydev->autoneg != AUTONEG_ENABLE)
		return 0;
	/*
	 * BMCR_ANENABLE might have been cleared
	 * by phy_init_hw in certain kernel versions
	 * therefore check for it
	 */
	ret = phy_read(phydev, MII_BMCR);
	if (ret < 0)
		return ret;
	if (ret & BMCR_ANENABLE)
		return 0;

	dev_info(&phydev->mdio.dev, "ANEG disabled, re-enabling ...\n");
	ret |= BMCR_ANENABLE | BMCR_ANRESTART;
	return phy_write(phydev, MII_BMCR, ret);
}

void
ar8xxx_phy_init(struct ar8xxx_priv *priv)
{
	int i;
	struct mii_bus *bus;

	bus = priv->sw_mii_bus ?: priv->mii_bus;
	for (i = 0; i < AR8XXX_NUM_PHYS; i++) {
		if (priv->chip->phy_fixup)
			priv->chip->phy_fixup(priv, i);

		/* initialize the port itself */
		mdiobus_write(bus, i, MII_ADVERTISE,
			ADVERTISE_ALL | ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);
		if (ar8xxx_has_gige(priv))
			mdiobus_write(bus, i, MII_CTRL1000, ADVERTISE_1000FULL);
		mdiobus_write(bus, i, MII_BMCR, BMCR_RESET | BMCR_ANENABLE);
	}

	ar8xxx_phy_poll_reset(bus);
}

u32
ar8xxx_mii_read32(struct ar8xxx_priv *priv, int phy_id, int regnum)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 lo, hi;

	lo = bus->read(bus, phy_id, regnum);
	hi = bus->read(bus, phy_id, regnum + 1);

	return (hi << 16) | lo;
}

void
ar8xxx_mii_write32(struct ar8xxx_priv *priv, int phy_id, int regnum, u32 val)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 lo, hi;

	lo = val & 0xffff;
	hi = (u16) (val >> 16);

	if (priv->chip->mii_lo_first)
	{
		bus->write(bus, phy_id, regnum, lo);
		bus->write(bus, phy_id, regnum + 1, hi);
	} else {
		bus->write(bus, phy_id, regnum + 1, hi);
		bus->write(bus, phy_id, regnum, lo);
	}
}

u32
ar8xxx_read(struct ar8xxx_priv *priv, int reg)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 r1, r2, page;
	u32 val;

	split_addr((u32) reg, &r1, &r2, &page);

	mutex_lock(&bus->mdio_lock);

	bus->write(bus, 0x18, 0, page);
	wait_for_page_switch();
	val = ar8xxx_mii_read32(priv, 0x10 | r2, r1);

	mutex_unlock(&bus->mdio_lock);

	return val;
}

void
ar8xxx_write(struct ar8xxx_priv *priv, int reg, u32 val)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 r1, r2, page;

	split_addr((u32) reg, &r1, &r2, &page);

	mutex_lock(&bus->mdio_lock);

	bus->write(bus, 0x18, 0, page);
	wait_for_page_switch();
	ar8xxx_mii_write32(priv, 0x10 | r2, r1, val);

	mutex_unlock(&bus->mdio_lock);
}

u32
ar8xxx_rmw(struct ar8xxx_priv *priv, int reg, u32 mask, u32 val)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 r1, r2, page;
	u32 ret;

	split_addr((u32) reg, &r1, &r2, &page);

	mutex_lock(&bus->mdio_lock);

	bus->write(bus, 0x18, 0, page);
	wait_for_page_switch();

	ret = ar8xxx_mii_read32(priv, 0x10 | r2, r1);
	ret &= ~mask;
	ret |= val;
	ar8xxx_mii_write32(priv, 0x10 | r2, r1, ret);

	mutex_unlock(&bus->mdio_lock);

	return ret;
}
void
ar8xxx_phy_dbg_read(struct ar8xxx_priv *priv, int phy_addr,
           u16 dbg_addr, u16 *dbg_data)
{
       struct mii_bus *bus = priv->mii_bus;

       mutex_lock(&bus->mdio_lock);
       bus->write(bus, phy_addr, MII_ATH_DBG_ADDR, dbg_addr);
       *dbg_data = bus->read(bus, phy_addr, MII_ATH_DBG_DATA);
       mutex_unlock(&bus->mdio_lock);
}

void
ar8xxx_phy_dbg_write(struct ar8xxx_priv *priv, int phy_addr,
		     u16 dbg_addr, u16 dbg_data)
{
	struct mii_bus *bus = priv->mii_bus;

	mutex_lock(&bus->mdio_lock);
	bus->write(bus, phy_addr, MII_ATH_DBG_ADDR, dbg_addr);
	bus->write(bus, phy_addr, MII_ATH_DBG_DATA, dbg_data);
	mutex_unlock(&bus->mdio_lock);
}

static inline void
ar8xxx_phy_mmd_prep(struct mii_bus *bus, int phy_addr, u16 addr, u16 reg)
{
	bus->write(bus, phy_addr, MII_ATH_MMD_ADDR, addr);
	bus->write(bus, phy_addr, MII_ATH_MMD_DATA, reg);
	bus->write(bus, phy_addr, MII_ATH_MMD_ADDR, addr | 0x4000);
}

void
ar8xxx_phy_mmd_write(struct ar8xxx_priv *priv, int phy_addr, u16 addr, u16 reg, u16 data)
{
	struct mii_bus *bus = priv->mii_bus;

	mutex_lock(&bus->mdio_lock);
	ar8xxx_phy_mmd_prep(bus, phy_addr, addr, reg);
	bus->write(bus, phy_addr, MII_ATH_MMD_DATA, data);
	mutex_unlock(&bus->mdio_lock);
}

u16
ar8xxx_phy_mmd_read(struct ar8xxx_priv *priv, int phy_addr, u16 addr, u16 reg)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 data;

	mutex_lock(&bus->mdio_lock);
	ar8xxx_phy_mmd_prep(bus, phy_addr, addr, reg);
	data = bus->read(bus, phy_addr, MII_ATH_MMD_DATA);
	mutex_unlock(&bus->mdio_lock);

	return data;
}

static int
ar8xxx_reg_wait(struct ar8xxx_priv *priv, u32 reg, u32 mask, u32 val,
		unsigned timeout)
{
	int i;

	for (i = 0; i < timeout; i++) {
		u32 t;

		t = ar8xxx_read(priv, reg);
		if ((t & mask) == val)
			return 0;

		usleep_range(1000, 2000);
		cond_resched();
	}

	return -ETIMEDOUT;
}

static int
ar8xxx_mib_op(struct ar8xxx_priv *priv, u32 op)
{
	unsigned mib_func = priv->chip->mib_func;
	int ret;

	lockdep_assert_held(&priv->mib_lock);

	/* Capture the hardware statistics for all ports */
	ar8xxx_rmw(priv, mib_func, AR8216_MIB_FUNC, (op << AR8216_MIB_FUNC_S));

	/* Wait for the capturing to complete. */
	ret = ar8xxx_reg_wait(priv, mib_func, AR8216_MIB_BUSY, 0, 10);
	if (ret)
		goto out;

	ret = 0;

out:
	return ret;
}

static int
ar8xxx_mib_capture(struct ar8xxx_priv *priv)
{
	return ar8xxx_mib_op(priv, AR8216_MIB_FUNC_CAPTURE);
}

static int
ar8xxx_mib_flush(struct ar8xxx_priv *priv)
{
	return ar8xxx_mib_op(priv, AR8216_MIB_FUNC_FLUSH);
}

static void
ar8xxx_mib_fetch_port_stat(struct ar8xxx_priv *priv, int port, bool flush)
{
	unsigned int base;
	u64 *mib_stats;
	int i;

	WARN_ON(port >= priv->dev.ports);

	lockdep_assert_held(&priv->mib_lock);

	base = priv->chip->reg_port_stats_start +
	       priv->chip->reg_port_stats_length * port;

	mib_stats = &priv->mib_stats[port * priv->chip->num_mibs];
	for (i = 0; i < priv->chip->num_mibs; i++) {
		const struct ar8xxx_mib_desc *mib;
		u64 t;

		mib = &priv->chip->mib_decs[i];
		if (mib->type > priv->mib_type)
			continue;
		t = ar8xxx_read(priv, base + mib->offset);
		if (mib->size == 2) {
			u64 hi;

			hi = ar8xxx_read(priv, base + mib->offset + 4);
			t |= hi << 32;
		}

		if (flush)
			mib_stats[i] = 0;
		else
			mib_stats[i] += t;
		cond_resched();
	}
}

static void
ar8216_read_port_link(struct ar8xxx_priv *priv, int port,
		      struct switch_port_link *link)
{
	u32 status;
	u32 speed;

	memset(link, '\0', sizeof(*link));

	status = priv->chip->read_port_status(priv, port);

	link->aneg = !!(status & AR8216_PORT_STATUS_LINK_AUTO);
	if (link->aneg) {
		link->link = !!(status & AR8216_PORT_STATUS_LINK_UP);
	} else {
		link->link = true;

		if (priv->get_port_link) {
			int err;

			err = priv->get_port_link(port);
			if (err >= 0)
				link->link = !!err;
		}
	}

	if (!link->link)
		return;

	link->duplex = !!(status & AR8216_PORT_STATUS_DUPLEX);
	link->tx_flow = !!(status & AR8216_PORT_STATUS_TXFLOW);
	link->rx_flow = !!(status & AR8216_PORT_STATUS_RXFLOW);

	if (link->aneg && link->duplex && priv->chip->read_port_eee_status)
		link->eee = priv->chip->read_port_eee_status(priv, port);

	speed = (status & AR8216_PORT_STATUS_SPEED) >>
		 AR8216_PORT_STATUS_SPEED_S;

	switch (speed) {
	case AR8216_PORT_SPEED_10M:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case AR8216_PORT_SPEED_100M:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case AR8216_PORT_SPEED_1000M:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}
}

#ifdef CONFIG_ETHERNET_PACKET_MANGLE

static struct sk_buff *
ar8216_mangle_tx(struct net_device *dev, struct sk_buff *skb)
{
	struct ar8xxx_priv *priv = dev->phy_ptr;
	unsigned char *buf;

	if (unlikely(!priv))
		goto error;

	if (!priv->vlan)
		goto send;

	if (unlikely(skb_headroom(skb) < 2)) {
		if (pskb_expand_head(skb, 2, 0, GFP_ATOMIC) < 0)
			goto error;
	}

	buf = skb_push(skb, 2);
	buf[0] = 0x10;
	buf[1] = 0x80;

send:
	return skb;

error:
	dev_kfree_skb_any(skb);
	return NULL;
}

static void
ar8216_mangle_rx(struct net_device *dev, struct sk_buff *skb)
{
	struct ar8xxx_priv *priv;
	unsigned char *buf;
	int port, vlan;

	priv = dev->phy_ptr;
	if (!priv)
		return;

	/* don't strip the header if vlan mode is disabled */
	if (!priv->vlan)
		return;

	/* strip header, get vlan id */
	buf = skb->data;
	skb_pull(skb, 2);

	/* check for vlan header presence */
	if ((buf[12 + 2] != 0x81) || (buf[13 + 2] != 0x00))
		return;

	port = buf[0] & 0x7;

	/* no need to fix up packets coming from a tagged source */
	if (priv->vlan_tagged & (1 << port))
		return;

	/* lookup port vid from local table, the switch passes an invalid vlan id */
	vlan = priv->vlan_id[priv->pvid[port]];

	buf[14 + 2] &= 0xf0;
	buf[14 + 2] |= vlan >> 8;
	buf[15 + 2] = vlan & 0xff;
}

#endif

int
ar8216_wait_bit(struct ar8xxx_priv *priv, int reg, u32 mask, u32 val)
{
	int timeout = 20;
	u32 t = 0;

	while (1) {
		t = ar8xxx_read(priv, reg);
		if ((t & mask) == val)
			return 0;

		if (timeout-- <= 0)
			break;

		udelay(10);
		cond_resched();
	}

	pr_err("ar8216: timeout on reg %08x: %08x & %08x != %08x\n",
	       (unsigned int) reg, t, mask, val);
	return -ETIMEDOUT;
}

static void
ar8216_vtu_op(struct ar8xxx_priv *priv, u32 op, u32 val)
{
	if (ar8216_wait_bit(priv, AR8216_REG_VTU, AR8216_VTU_ACTIVE, 0))
		return;
	if ((op & AR8216_VTU_OP) == AR8216_VTU_OP_LOAD) {
		val &= AR8216_VTUDATA_MEMBER;
		val |= AR8216_VTUDATA_VALID;
		ar8xxx_write(priv, AR8216_REG_VTU_DATA, val);
	}
	op |= AR8216_VTU_ACTIVE;
	ar8xxx_write(priv, AR8216_REG_VTU, op);
}

static void
ar8216_vtu_flush(struct ar8xxx_priv *priv)
{
	ar8216_vtu_op(priv, AR8216_VTU_OP_FLUSH, 0);
}

static void
ar8216_vtu_load_vlan(struct ar8xxx_priv *priv, u32 vid, u32 port_mask)
{
	u32 op;

	op = AR8216_VTU_OP_LOAD | (vid << AR8216_VTU_VID_S);
	ar8216_vtu_op(priv, op, port_mask);
}

static int
ar8216_atu_flush(struct ar8xxx_priv *priv)
{
	int ret;

	ret = ar8216_wait_bit(priv, AR8216_REG_ATU_FUNC0, AR8216_ATU_ACTIVE, 0);
	if (!ret)
		ar8xxx_write(priv, AR8216_REG_ATU_FUNC0, AR8216_ATU_OP_FLUSH |
							 AR8216_ATU_ACTIVE);

	return ret;
}

static int
ar8216_atu_flush_port(struct ar8xxx_priv *priv, int port)
{
	u32 t;
	int ret;

	ret = ar8216_wait_bit(priv, AR8216_REG_ATU_FUNC0, AR8216_ATU_ACTIVE, 0);
	if (!ret) {
		t = (port << AR8216_ATU_PORT_NUM_S) | AR8216_ATU_OP_FLUSH_PORT;
		t |= AR8216_ATU_ACTIVE;
		ar8xxx_write(priv, AR8216_REG_ATU_FUNC0, t);
	}

	return ret;
}

static u32
ar8216_read_port_status(struct ar8xxx_priv *priv, int port)
{
	return ar8xxx_read(priv, AR8216_REG_PORT_STATUS(port));
}

static void
__ar8216_setup_port(struct ar8xxx_priv *priv, int port, u32 members,
		    bool ath_hdr_en)
{
	u32 header;
	u32 egress, ingress;
	u32 pvid;

	if (priv->vlan) {
		pvid = priv->vlan_id[priv->pvid[port]];
		if (priv->vlan_tagged & (1 << port))
			egress = AR8216_OUT_ADD_VLAN;
		else
			egress = AR8216_OUT_STRIP_VLAN;
		ingress = AR8216_IN_SECURE;
	} else {
		pvid = port;
		egress = AR8216_OUT_KEEP;
		ingress = AR8216_IN_PORT_ONLY;
	}

	header = ath_hdr_en ? AR8216_PORT_CTRL_HEADER : 0;

	ar8xxx_rmw(priv, AR8216_REG_PORT_CTRL(port),
		   AR8216_PORT_CTRL_LEARN | AR8216_PORT_CTRL_VLAN_MODE |
		   AR8216_PORT_CTRL_SINGLE_VLAN | AR8216_PORT_CTRL_STATE |
		   AR8216_PORT_CTRL_HEADER | AR8216_PORT_CTRL_LEARN_LOCK,
		   AR8216_PORT_CTRL_LEARN | header |
		   (egress << AR8216_PORT_CTRL_VLAN_MODE_S) |
		   (AR8216_PORT_STATE_FORWARD << AR8216_PORT_CTRL_STATE_S));

	ar8xxx_rmw(priv, AR8216_REG_PORT_VLAN(port),
		   AR8216_PORT_VLAN_DEST_PORTS | AR8216_PORT_VLAN_MODE |
		   AR8216_PORT_VLAN_DEFAULT_ID,
		   (members << AR8216_PORT_VLAN_DEST_PORTS_S) |
		   (ingress << AR8216_PORT_VLAN_MODE_S) |
		   (pvid << AR8216_PORT_VLAN_DEFAULT_ID_S));
}

static void
ar8216_setup_port(struct ar8xxx_priv *priv, int port, u32 members)
{
	return __ar8216_setup_port(priv, port, members,
				   chip_is_ar8216(priv) && priv->vlan &&
				   port == AR8216_PORT_CPU);
}

static int
ar8216_hw_init(struct ar8xxx_priv *priv)
{
	if (priv->initialized)
		return 0;

	ar8xxx_write(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET);
	ar8xxx_reg_wait(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET, 0, 1000);

	ar8xxx_phy_init(priv);

	priv->initialized = true;
	return 0;
}

static void
ar8216_init_globals(struct ar8xxx_priv *priv)
{
	/* standard atheros magic */
	ar8xxx_write(priv, 0x38, 0xc000050e);

	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8216_GCTRL_MTU, 1518 + 8 + 2);
}

static void
__ar8216_init_port(struct ar8xxx_priv *priv, int port,
		   bool cpu_ge, bool flow_en)
{
	/* Enable port learning and tx */
	ar8xxx_write(priv, AR8216_REG_PORT_CTRL(port),
		AR8216_PORT_CTRL_LEARN |
		(4 << AR8216_PORT_CTRL_STATE_S));

	ar8xxx_write(priv, AR8216_REG_PORT_VLAN(port), 0);

	if (port == AR8216_PORT_CPU) {
		ar8xxx_write(priv, AR8216_REG_PORT_STATUS(port),
			AR8216_PORT_STATUS_LINK_UP |
			(cpu_ge ? AR8216_PORT_SPEED_1000M : AR8216_PORT_SPEED_100M) |
			AR8216_PORT_STATUS_TXMAC |
			AR8216_PORT_STATUS_RXMAC |
			(flow_en ? AR8216_PORT_STATUS_RXFLOW : 0) |
			(flow_en ? AR8216_PORT_STATUS_TXFLOW : 0) |
			AR8216_PORT_STATUS_DUPLEX);
	} else {
		ar8xxx_write(priv, AR8216_REG_PORT_STATUS(port),
			AR8216_PORT_STATUS_LINK_AUTO);
	}
}

static void
ar8216_init_port(struct ar8xxx_priv *priv, int port)
{
	__ar8216_init_port(priv, port, ar8xxx_has_gige(priv),
			   chip_is_ar8316(priv));
}

static void
ar8216_wait_atu_ready(struct ar8xxx_priv *priv, u16 r2, u16 r1)
{
	int timeout = 20;

	while (ar8xxx_mii_read32(priv, r2, r1) & AR8216_ATU_ACTIVE && --timeout) {
		udelay(10);
		cond_resched();
	}

	if (!timeout)
		pr_err("ar8216: timeout waiting for atu to become ready\n");
}

static void ar8216_get_arl_entry(struct ar8xxx_priv *priv,
				 struct arl_entry *a, u32 *status, enum arl_op op)
{
	struct mii_bus *bus = priv->mii_bus;
	u16 r2, page;
	u16 r1_func0, r1_func1, r1_func2;
	u32 t, val0, val1, val2;

	split_addr(AR8216_REG_ATU_FUNC0, &r1_func0, &r2, &page);
	r2 |= 0x10;

	r1_func1 = (AR8216_REG_ATU_FUNC1 >> 1) & 0x1e;
	r1_func2 = (AR8216_REG_ATU_FUNC2 >> 1) & 0x1e;

	switch (op) {
	case AR8XXX_ARL_INITIALIZE:
		/* all ATU registers are on the same page
		* therefore set page only once
		*/
		bus->write(bus, 0x18, 0, page);
		wait_for_page_switch();

		ar8216_wait_atu_ready(priv, r2, r1_func0);

		ar8xxx_mii_write32(priv, r2, r1_func0, AR8216_ATU_OP_GET_NEXT);
		ar8xxx_mii_write32(priv, r2, r1_func1, 0);
		ar8xxx_mii_write32(priv, r2, r1_func2, 0);
		break;
	case AR8XXX_ARL_GET_NEXT:
		t = ar8xxx_mii_read32(priv, r2, r1_func0);
		t |= AR8216_ATU_ACTIVE;
		ar8xxx_mii_write32(priv, r2, r1_func0, t);
		ar8216_wait_atu_ready(priv, r2, r1_func0);

		val0 = ar8xxx_mii_read32(priv, r2, r1_func0);
		val1 = ar8xxx_mii_read32(priv, r2, r1_func1);
		val2 = ar8xxx_mii_read32(priv, r2, r1_func2);

		*status = (val2 & AR8216_ATU_STATUS) >> AR8216_ATU_STATUS_S;
		if (!*status)
			break;

		a->portmap = (val2 & AR8216_ATU_PORTS) >> AR8216_ATU_PORTS_S;
		a->mac[0] = (val0 & AR8216_ATU_ADDR5) >> AR8216_ATU_ADDR5_S;
		a->mac[1] = (val0 & AR8216_ATU_ADDR4) >> AR8216_ATU_ADDR4_S;
		a->mac[2] = (val1 & AR8216_ATU_ADDR3) >> AR8216_ATU_ADDR3_S;
		a->mac[3] = (val1 & AR8216_ATU_ADDR2) >> AR8216_ATU_ADDR2_S;
		a->mac[4] = (val1 & AR8216_ATU_ADDR1) >> AR8216_ATU_ADDR1_S;
		a->mac[5] = (val1 & AR8216_ATU_ADDR0) >> AR8216_ATU_ADDR0_S;
		break;
	}
}

static int
ar8216_phy_read(struct ar8xxx_priv *priv, int addr, int regnum)
{
	u32 t, val = 0xffff;
	int err;

	if (addr >= AR8216_NUM_PORTS)
		return 0xffff;
	t = (regnum << AR8216_MDIO_CTRL_REG_ADDR_S) |
	    (addr << AR8216_MDIO_CTRL_PHY_ADDR_S) |
	    AR8216_MDIO_CTRL_MASTER_EN |
	    AR8216_MDIO_CTRL_BUSY |
	    AR8216_MDIO_CTRL_CMD_READ;

	ar8xxx_write(priv, AR8216_REG_MDIO_CTRL, t);
	err = ar8xxx_reg_wait(priv, AR8216_REG_MDIO_CTRL,
			      AR8216_MDIO_CTRL_BUSY, 0, 5);
	if (!err)
		val = ar8xxx_read(priv, AR8216_REG_MDIO_CTRL);

	return val & AR8216_MDIO_CTRL_DATA_M;
}

static int
ar8216_phy_write(struct ar8xxx_priv *priv, int addr, int regnum, u16 val)
{
	u32 t;
	int ret;

	if (addr >= AR8216_NUM_PORTS)
		return -EINVAL;

	t = (addr << AR8216_MDIO_CTRL_PHY_ADDR_S) |
	    (regnum << AR8216_MDIO_CTRL_REG_ADDR_S) |
	    AR8216_MDIO_CTRL_MASTER_EN |
	    AR8216_MDIO_CTRL_BUSY |
	    AR8216_MDIO_CTRL_CMD_WRITE |
	    val;

	ar8xxx_write(priv, AR8216_REG_MDIO_CTRL, t);
	ret = ar8xxx_reg_wait(priv, AR8216_REG_MDIO_CTRL,
			      AR8216_MDIO_CTRL_BUSY, 0, 5);

	return ret;
}

static int
ar8229_hw_init(struct ar8xxx_priv *priv)
{
	phy_interface_t phy_if_mode;

	if (priv->initialized)
		return 0;

	ar8xxx_write(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET);
	ar8xxx_reg_wait(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET, 0, 1000);

	of_get_phy_mode(priv->pdev->of_node, &phy_if_mode);

	if (phy_if_mode == PHY_INTERFACE_MODE_GMII) {
		ar8xxx_write(priv, AR8229_REG_OPER_MODE0,
				 AR8229_OPER_MODE0_MAC_GMII_EN);
	} else if (phy_if_mode == PHY_INTERFACE_MODE_MII) {
		ar8xxx_write(priv, AR8229_REG_OPER_MODE0,
				 AR8229_OPER_MODE0_PHY_MII_EN);
	} else {
		pr_err("ar8229: unsupported mii mode\n");
		return -EINVAL;
	}

	if (priv->port4_phy) {
		ar8xxx_write(priv, AR8229_REG_OPER_MODE1,
			     AR8229_REG_OPER_MODE1_PHY4_MII_EN);
		/* disable port5 to prevent mii conflict */
		ar8xxx_write(priv, AR8216_REG_PORT_STATUS(5), 0);
	}

	ar8xxx_phy_init(priv);

	priv->initialized = true;
	return 0;
}

static void
ar8229_init_globals(struct ar8xxx_priv *priv)
{

	/* Enable CPU port, and disable mirror port */
	ar8xxx_write(priv, AR8216_REG_GLOBAL_CPUPORT,
		     AR8216_GLOBAL_CPUPORT_EN |
		     (15 << AR8216_GLOBAL_CPUPORT_MIRROR_PORT_S));

	/* Setup TAG priority mapping */
	ar8xxx_write(priv, AR8216_REG_TAG_PRIORITY, 0xfa50);

	/* Enable aging, MAC replacing */
	ar8xxx_write(priv, AR8216_REG_ATU_CTRL,
		     0x2b /* 5 min age time */ |
		     AR8216_ATU_CTRL_AGE_EN |
		     AR8216_ATU_CTRL_LEARN_CHANGE);

	/* Enable ARP frame acknowledge */
	ar8xxx_reg_set(priv, AR8229_REG_QM_CTRL,
		       AR8229_QM_CTRL_ARP_EN);

	/*
	 * Enable Broadcast/unknown multicast and unicast frames
	 * transmitted to the CPU port.
	 */
	ar8xxx_reg_set(priv, AR8216_REG_FLOOD_MASK,
		       AR8229_FLOOD_MASK_BC_DP(0) |
		       AR8229_FLOOD_MASK_MC_DP(0) |
		       AR8229_FLOOD_MASK_UC_DP(0));

	/* setup MTU */
	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8236_GCTRL_MTU, AR8236_GCTRL_MTU);

	/* Enable MIB counters */
	ar8xxx_reg_set(priv, AR8216_REG_MIB_FUNC,
		       AR8236_MIB_EN);

	/* setup Service TAG */
	ar8xxx_rmw(priv, AR8216_REG_SERVICE_TAG, AR8216_SERVICE_TAG_M, 0);
}

static void
ar8229_init_port(struct ar8xxx_priv *priv, int port)
{
	__ar8216_init_port(priv, port, true, true);
}


static int
ar7240sw_hw_init(struct ar8xxx_priv *priv)
{
	if (priv->initialized)
		return 0;

	ar8xxx_write(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET);
	ar8xxx_reg_wait(priv, AR8216_REG_CTRL, AR8216_CTRL_RESET, 0, 1000);

	priv->port4_phy = 1;
	/* disable port5 to prevent mii conflict */
	ar8xxx_write(priv, AR8216_REG_PORT_STATUS(5), 0);

	ar8xxx_phy_init(priv);

	priv->initialized = true;
	return 0;
}

static void
ar7240sw_init_globals(struct ar8xxx_priv *priv)
{

	/* Enable CPU port, and disable mirror port */
	ar8xxx_write(priv, AR8216_REG_GLOBAL_CPUPORT,
		     AR8216_GLOBAL_CPUPORT_EN |
		     (15 << AR8216_GLOBAL_CPUPORT_MIRROR_PORT_S));

	/* Setup TAG priority mapping */
	ar8xxx_write(priv, AR8216_REG_TAG_PRIORITY, 0xfa50);

	/* Enable ARP frame acknowledge, aging, MAC replacing */
	ar8xxx_write(priv, AR8216_REG_ATU_CTRL,
		AR8216_ATU_CTRL_RESERVED |
		0x2b /* 5 min age time */ |
		AR8216_ATU_CTRL_AGE_EN |
		AR8216_ATU_CTRL_ARP_EN |
		AR8216_ATU_CTRL_LEARN_CHANGE);

	/* Enable Broadcast frames transmitted to the CPU */
	ar8xxx_reg_set(priv, AR8216_REG_FLOOD_MASK,
		       AR8216_FM_CPU_BROADCAST_EN);

	/* setup MTU */
	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8216_GCTRL_MTU,
		   AR8216_GCTRL_MTU);

	/* setup Service TAG */
	ar8xxx_rmw(priv, AR8216_REG_SERVICE_TAG, AR8216_SERVICE_TAG_M, 0);
}

static void
ar7240sw_setup_port(struct ar8xxx_priv *priv, int port, u32 members)
{
	return __ar8216_setup_port(priv, port, members, false);
}

static void
ar8236_setup_port(struct ar8xxx_priv *priv, int port, u32 members)
{
	u32 egress, ingress;
	u32 pvid;

	if (priv->vlan) {
		pvid = priv->vlan_id[priv->pvid[port]];
		if (priv->vlan_tagged & (1 << port))
			egress = AR8216_OUT_ADD_VLAN;
		else
			egress = AR8216_OUT_STRIP_VLAN;
		ingress = AR8216_IN_SECURE;
	} else {
		pvid = port;
		egress = AR8216_OUT_KEEP;
		ingress = AR8216_IN_PORT_ONLY;
	}

	ar8xxx_rmw(priv, AR8216_REG_PORT_CTRL(port),
		   AR8216_PORT_CTRL_LEARN | AR8216_PORT_CTRL_VLAN_MODE |
		   AR8216_PORT_CTRL_SINGLE_VLAN | AR8216_PORT_CTRL_STATE |
		   AR8216_PORT_CTRL_HEADER | AR8216_PORT_CTRL_LEARN_LOCK,
		   AR8216_PORT_CTRL_LEARN |
		   (egress << AR8216_PORT_CTRL_VLAN_MODE_S) |
		   (AR8216_PORT_STATE_FORWARD << AR8216_PORT_CTRL_STATE_S));

	ar8xxx_rmw(priv, AR8236_REG_PORT_VLAN(port),
		   AR8236_PORT_VLAN_DEFAULT_ID,
		   (pvid << AR8236_PORT_VLAN_DEFAULT_ID_S));

	ar8xxx_rmw(priv, AR8236_REG_PORT_VLAN2(port),
		   AR8236_PORT_VLAN2_VLAN_MODE |
		   AR8236_PORT_VLAN2_MEMBER,
		   (ingress << AR8236_PORT_VLAN2_VLAN_MODE_S) |
		   (members << AR8236_PORT_VLAN2_MEMBER_S));
}

static void
ar8236_init_globals(struct ar8xxx_priv *priv)
{
	/* enable jumbo frames */
	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8316_GCTRL_MTU, 9018 + 8 + 2);

	/* enable cpu port to receive arp frames */
	ar8xxx_reg_set(priv, AR8216_REG_ATU_CTRL,
		   AR8236_ATU_CTRL_RES);

	/*
	 * Enable Broadcast/unknown multicast and unicast frames
	 * transmitted to the CPU port.
	 */
	ar8xxx_reg_set(priv, AR8216_REG_FLOOD_MASK,
		       AR8229_FLOOD_MASK_BC_DP(0) |
		       AR8229_FLOOD_MASK_MC_DP(0) |
		       AR8229_FLOOD_MASK_UC_DP(0));

	/* Enable MIB counters */
	ar8xxx_rmw(priv, AR8216_REG_MIB_FUNC, AR8216_MIB_FUNC | AR8236_MIB_EN,
		   (AR8216_MIB_FUNC_NO_OP << AR8216_MIB_FUNC_S) |
		   AR8236_MIB_EN);
}

static int
ar8316_hw_init(struct ar8xxx_priv *priv)
{
	u32 val, newval;

	val = ar8xxx_read(priv, AR8316_REG_POSTRIP);

	if (priv->phy->interface == PHY_INTERFACE_MODE_RGMII) {
		if (priv->port4_phy) {
			/* value taken from Ubiquiti RouterStation Pro */
			newval = 0x81461bea;
			pr_info("ar8316: Using port 4 as PHY\n");
		} else {
			newval = 0x01261be2;
			pr_info("ar8316: Using port 4 as switch port\n");
		}
	} else if (priv->phy->interface == PHY_INTERFACE_MODE_GMII) {
		/* value taken from AVM Fritz!Box 7390 sources */
		newval = 0x010e5b71;
	} else {
		/* no known value for phy interface */
		pr_err("ar8316: unsupported mii mode: %d.\n",
		       priv->phy->interface);
		return -EINVAL;
	}

	if (val == newval)
		goto out;

	ar8xxx_write(priv, AR8316_REG_POSTRIP, newval);

	if (priv->port4_phy &&
	    priv->phy->interface == PHY_INTERFACE_MODE_RGMII) {
		/* work around for phy4 rgmii mode */
		ar8xxx_phy_dbg_write(priv, 4, 0x12, 0x480c);
		/* rx delay */
		ar8xxx_phy_dbg_write(priv, 4, 0x0, 0x824e);
		/* tx delay */
		ar8xxx_phy_dbg_write(priv, 4, 0x5, 0x3d47);
		msleep(1000);
	}

	ar8xxx_phy_init(priv);

out:
	priv->initialized = true;
	return 0;
}

static void
ar8316_init_globals(struct ar8xxx_priv *priv)
{
	/* standard atheros magic */
	ar8xxx_write(priv, 0x38, 0xc000050e);

	/* enable cpu port to receive multicast and broadcast frames */
	ar8xxx_write(priv, AR8216_REG_FLOOD_MASK, 0x003f003f);

	/* enable jumbo frames */
	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CTRL,
		   AR8316_GCTRL_MTU, 9018 + 8 + 2);

	/* Enable MIB counters */
	ar8xxx_rmw(priv, AR8216_REG_MIB_FUNC, AR8216_MIB_FUNC | AR8236_MIB_EN,
		   (AR8216_MIB_FUNC_NO_OP << AR8216_MIB_FUNC_S) |
		   AR8236_MIB_EN);
}

int
ar8xxx_sw_set_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	priv->vlan = !!val->value.i;
	return 0;
}

int
ar8xxx_sw_get_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->vlan;
	return 0;
}


int
ar8xxx_sw_set_pvid(struct switch_dev *dev, int port, int vlan)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	/* make sure no invalid PVIDs get set */

	if (vlan < 0 || vlan >= dev->vlans ||
	    port < 0 || port >= AR8X16_MAX_PORTS)
		return -EINVAL;

	priv->pvid[port] = vlan;
	return 0;
}

int
ar8xxx_sw_get_pvid(struct switch_dev *dev, int port, int *vlan)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (port < 0 || port >= AR8X16_MAX_PORTS)
		return -EINVAL;

	*vlan = priv->pvid[port];
	return 0;
}

static int
ar8xxx_sw_set_vid(struct switch_dev *dev, const struct switch_attr *attr,
		  struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	priv->vlan_id[val->port_vlan] = val->value.i;
	return 0;
}

static int
ar8xxx_sw_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		  struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->vlan_id[val->port_vlan];
	return 0;
}

int
ar8xxx_sw_get_port_link(struct switch_dev *dev, int port,
			struct switch_port_link *link)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	ar8216_read_port_link(priv, port, link);
	return 0;
}

static int
ar8xxx_sw_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	u8 ports;
	int i;

	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	ports = priv->vlan_table[val->port_vlan];
	val->len = 0;
	for (i = 0; i < dev->ports; i++) {
		struct switch_port *p;

		if (!(ports & (1 << i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (priv->vlan_tagged & (1 << i))
			p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
		else
			p->flags = 0;
	}
	return 0;
}

static int
ar8xxx_sw_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	u8 *vt = &priv->vlan_table[val->port_vlan];
	int i, j;

	*vt = 0;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
			priv->vlan_tagged |= (1 << p->id);
		} else {
			priv->vlan_tagged &= ~(1 << p->id);
			priv->pvid[p->id] = val->port_vlan;

			/* make sure that an untagged port does not
			 * appear in other vlans */
			for (j = 0; j < dev->vlans; j++) {
				if (j == val->port_vlan)
					continue;
				priv->vlan_table[j] &= ~(1 << p->id);
			}
		}

		*vt |= 1 << p->id;
	}
	return 0;
}

static void
ar8216_set_mirror_regs(struct ar8xxx_priv *priv)
{
	int port;

	/* reset all mirror registers */
	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CPUPORT,
		   AR8216_GLOBAL_CPUPORT_MIRROR_PORT,
		   (0xF << AR8216_GLOBAL_CPUPORT_MIRROR_PORT_S));
	for (port = 0; port < AR8216_NUM_PORTS; port++) {
		ar8xxx_reg_clear(priv, AR8216_REG_PORT_CTRL(port),
			   AR8216_PORT_CTRL_MIRROR_RX);

		ar8xxx_reg_clear(priv, AR8216_REG_PORT_CTRL(port),
			   AR8216_PORT_CTRL_MIRROR_TX);
	}

	/* now enable mirroring if necessary */
	if (priv->source_port >= AR8216_NUM_PORTS ||
	    priv->monitor_port >= AR8216_NUM_PORTS ||
	    priv->source_port == priv->monitor_port) {
		return;
	}

	ar8xxx_rmw(priv, AR8216_REG_GLOBAL_CPUPORT,
		   AR8216_GLOBAL_CPUPORT_MIRROR_PORT,
		   (priv->monitor_port << AR8216_GLOBAL_CPUPORT_MIRROR_PORT_S));

	if (priv->mirror_rx)
		ar8xxx_reg_set(priv, AR8216_REG_PORT_CTRL(priv->source_port),
			   AR8216_PORT_CTRL_MIRROR_RX);

	if (priv->mirror_tx)
		ar8xxx_reg_set(priv, AR8216_REG_PORT_CTRL(priv->source_port),
			   AR8216_PORT_CTRL_MIRROR_TX);
}

static inline u32
ar8xxx_age_time_val(int age_time)
{
	return (age_time + AR8XXX_REG_ARL_CTRL_AGE_TIME_SECS / 2) /
	       AR8XXX_REG_ARL_CTRL_AGE_TIME_SECS;
}

static inline void
ar8xxx_set_age_time(struct ar8xxx_priv *priv, int reg)
{
	u32 age_time = ar8xxx_age_time_val(priv->arl_age_time);
	ar8xxx_rmw(priv, reg, AR8216_ATU_CTRL_AGE_TIME, age_time << AR8216_ATU_CTRL_AGE_TIME_S);
}

int
ar8xxx_sw_hw_apply(struct switch_dev *dev)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	const struct ar8xxx_chip *chip = priv->chip;
	u8 portmask[AR8X16_MAX_PORTS];
	int i, j;

	mutex_lock(&priv->reg_mutex);
	/* flush all vlan translation unit entries */
	priv->chip->vtu_flush(priv);

	memset(portmask, 0, sizeof(portmask));
	if (!priv->init) {
		/* calculate the port destination masks and load vlans
		 * into the vlan translation unit */
		for (j = 0; j < dev->vlans; j++) {
			u8 vp = priv->vlan_table[j];

			if (!vp)
				continue;

			for (i = 0; i < dev->ports; i++) {
				u8 mask = (1 << i);
				if (vp & mask)
					portmask[i] |= vp & ~mask;
			}

			chip->vtu_load_vlan(priv, priv->vlan_id[j],
					    priv->vlan_table[j]);
		}
	} else {
		/* vlan disabled:
		 * isolate all ports, but connect them to the cpu port */
		for (i = 0; i < dev->ports; i++) {
			if (i == AR8216_PORT_CPU)
				continue;

			portmask[i] = 1 << AR8216_PORT_CPU;
			portmask[AR8216_PORT_CPU] |= (1 << i);
		}
	}

	/* update the port destination mask registers and tag settings */
	for (i = 0; i < dev->ports; i++) {
		chip->setup_port(priv, i, portmask[i]);
	}

	chip->set_mirror_regs(priv);

	/* set age time */
	if (chip->reg_arl_ctrl)
		ar8xxx_set_age_time(priv, chip->reg_arl_ctrl);

	mutex_unlock(&priv->reg_mutex);
	return 0;
}

int
ar8xxx_sw_reset_switch(struct switch_dev *dev)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	const struct ar8xxx_chip *chip = priv->chip;
	int i;

	mutex_lock(&priv->reg_mutex);
	memset(&priv->ar8xxx_priv_volatile, 0, sizeof(priv->ar8xxx_priv_volatile));

	for (i = 0; i < dev->vlans; i++)
		priv->vlan_id[i] = i;

	/* Configure all ports */
	for (i = 0; i < dev->ports; i++)
		chip->init_port(priv, i);

	priv->mirror_rx = false;
	priv->mirror_tx = false;
	priv->source_port = 0;
	priv->monitor_port = 0;
	priv->arl_age_time = AR8XXX_DEFAULT_ARL_AGE_TIME;

	chip->init_globals(priv);
	chip->atu_flush(priv);

	mutex_unlock(&priv->reg_mutex);

	return chip->sw_hw_apply(dev);
}

int
ar8xxx_sw_set_reset_mibs(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	unsigned int len;
	int ret;

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;

	mutex_lock(&priv->mib_lock);

	len = priv->dev.ports * priv->chip->num_mibs *
	      sizeof(*priv->mib_stats);
	memset(priv->mib_stats, '\0', len);
	ret = ar8xxx_mib_flush(priv);
	if (ret)
		goto unlock;

	ret = 0;

unlock:
	mutex_unlock(&priv->mib_lock);
	return ret;
}

int
ar8xxx_sw_set_mib_poll_interval(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;

	ar8xxx_mib_stop(priv);
	priv->mib_poll_interval = val->value.i;
	ar8xxx_mib_start(priv);

	return 0;
}

int
ar8xxx_sw_get_mib_poll_interval(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;
	val->value.i = priv->mib_poll_interval;
	return 0;
}

int
ar8xxx_sw_set_mib_type(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;
	priv->mib_type = val->value.i;
	return 0;
}

int
ar8xxx_sw_get_mib_type(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;
	val->value.i = priv->mib_type;
	return 0;
}

int
ar8xxx_sw_set_mirror_rx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	mutex_lock(&priv->reg_mutex);
	priv->mirror_rx = !!val->value.i;
	priv->chip->set_mirror_regs(priv);
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

int
ar8xxx_sw_get_mirror_rx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->mirror_rx;
	return 0;
}

int
ar8xxx_sw_set_mirror_tx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	mutex_lock(&priv->reg_mutex);
	priv->mirror_tx = !!val->value.i;
	priv->chip->set_mirror_regs(priv);
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

int
ar8xxx_sw_get_mirror_tx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->mirror_tx;
	return 0;
}

int
ar8xxx_sw_set_mirror_monitor_port(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	mutex_lock(&priv->reg_mutex);
	priv->monitor_port = val->value.i;
	priv->chip->set_mirror_regs(priv);
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

int
ar8xxx_sw_get_mirror_monitor_port(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->monitor_port;
	return 0;
}

int
ar8xxx_sw_set_mirror_source_port(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);

	mutex_lock(&priv->reg_mutex);
	priv->source_port = val->value.i;
	priv->chip->set_mirror_regs(priv);
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

int
ar8xxx_sw_get_mirror_source_port(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->source_port;
	return 0;
}

int
ar8xxx_sw_set_port_reset_mib(struct switch_dev *dev,
			     const struct switch_attr *attr,
			     struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int port;
	int ret;

	if (!ar8xxx_has_mib_counters(priv))
		return -EOPNOTSUPP;

	port = val->port_vlan;
	if (port >= dev->ports)
		return -EINVAL;

	mutex_lock(&priv->mib_lock);
	ret = ar8xxx_mib_capture(priv);
	if (ret)
		goto unlock;

	ar8xxx_mib_fetch_port_stat(priv, port, true);

	ret = 0;

unlock:
	mutex_unlock(&priv->mib_lock);
	return ret;
}

static void
ar8xxx_byte_to_str(char *buf, int len, u64 byte)
{
	unsigned long b;
	const char *unit;

	if (byte >= 0x40000000) { /* 1 GiB */
		b = byte * 10 / 0x40000000;
		unit = "GiB";
	} else if (byte >= 0x100000) { /* 1 MiB */
		b = byte * 10 / 0x100000;
		unit = "MiB";
	} else if (byte >= 0x400) { /* 1 KiB */
		b = byte * 10 / 0x400;
		unit = "KiB";
	} else {
		b = byte;
		unit = "Byte";
	}
	if (strcmp(unit, "Byte"))
		snprintf(buf, len, "%lu.%lu %s", b / 10, b % 10, unit);
	else
		snprintf(buf, len, "%lu %s", b, unit);
}

int
ar8xxx_sw_get_port_mib(struct switch_dev *dev,
		       const struct switch_attr *attr,
		       struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	const struct ar8xxx_chip *chip = priv->chip;
	u64 *mib_stats, mib_data;
	unsigned int port;
	int ret;
	char *buf = priv->buf;
	char buf1[64];
	const char *mib_name;
	int i, len = 0;
	bool mib_stats_empty = true;

	if (!ar8xxx_has_mib_counters(priv) || !priv->mib_poll_interval)
		return -EOPNOTSUPP;

	port = val->port_vlan;
	if (port >= dev->ports)
		return -EINVAL;

	mutex_lock(&priv->mib_lock);
	ret = ar8xxx_mib_capture(priv);
	if (ret)
		goto unlock;

	ar8xxx_mib_fetch_port_stat(priv, port, false);

	len += snprintf(buf + len, sizeof(priv->buf) - len,
			"MIB counters\n");

	mib_stats = &priv->mib_stats[port * chip->num_mibs];
	for (i = 0; i < chip->num_mibs; i++) {
		if (chip->mib_decs[i].type > priv->mib_type)
			continue;
		mib_name = chip->mib_decs[i].name;
		mib_data = mib_stats[i];
		len += snprintf(buf + len, sizeof(priv->buf) - len,
				"%-12s: %llu\n", mib_name, mib_data);
		if ((!strcmp(mib_name, "TxByte") ||
		    !strcmp(mib_name, "RxGoodByte")) &&
		    mib_data >= 1024) {
			ar8xxx_byte_to_str(buf1, sizeof(buf1), mib_data);
			--len; /* discard newline at the end of buf */
			len += snprintf(buf + len, sizeof(priv->buf) - len,
					" (%s)\n", buf1);
		}
		if (mib_stats_empty && mib_data)
			mib_stats_empty = false;
	}

	if (mib_stats_empty)
		len = snprintf(buf, sizeof(priv->buf), "No MIB data");

	val->value.s = buf;
	val->len = len;

	ret = 0;

unlock:
	mutex_unlock(&priv->mib_lock);
	return ret;
}

int
ar8xxx_sw_set_arl_age_time(struct switch_dev *dev, const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int age_time = val->value.i;
	u32 age_time_val;

	if (age_time < 0)
		return -EINVAL;

	age_time_val = ar8xxx_age_time_val(age_time);
	if (age_time_val == 0 || age_time_val > 0xffff)
		return -EINVAL;

	priv->arl_age_time = age_time;
	return 0;
}

int
ar8xxx_sw_get_arl_age_time(struct switch_dev *dev, const struct switch_attr *attr,
                   struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	val->value.i = priv->arl_age_time;
	return 0;
}

int
ar8xxx_sw_get_arl_table(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	struct mii_bus *bus = priv->mii_bus;
	const struct ar8xxx_chip *chip = priv->chip;
	char *buf = priv->arl_buf;
	int i, j, k, len = 0;
	struct arl_entry *a, *a1;
	u32 status;

	if (!chip->get_arl_entry)
		return -EOPNOTSUPP;

	mutex_lock(&priv->reg_mutex);
	mutex_lock(&bus->mdio_lock);

	chip->get_arl_entry(priv, NULL, NULL, AR8XXX_ARL_INITIALIZE);

	for(i = 0; i < AR8XXX_NUM_ARL_RECORDS; ++i) {
		a = &priv->arl_table[i];
		duplicate:
		chip->get_arl_entry(priv, a, &status, AR8XXX_ARL_GET_NEXT);

		if (!status)
			break;

		/* avoid duplicates
		 * ARL table can include multiple valid entries
		 * per MAC, just with differing status codes
		 */
		for (j = 0; j < i; ++j) {
			a1 = &priv->arl_table[j];
			if (!memcmp(a->mac, a1->mac, sizeof(a->mac))) {
				/* ignore ports already seen in former entry */
				a->portmap &= ~a1->portmap;
				if (!a->portmap)
					goto duplicate;
			}
		}
	}

	mutex_unlock(&bus->mdio_lock);

	len += snprintf(buf + len, sizeof(priv->arl_buf) - len,
                        "address resolution table\n");

	if (i == AR8XXX_NUM_ARL_RECORDS)
		len += snprintf(buf + len, sizeof(priv->arl_buf) - len,
				"Too many entries found, displaying the first %d only!\n",
				AR8XXX_NUM_ARL_RECORDS);

	for (j = 0; j < priv->dev.ports; ++j) {
		for (k = 0; k < i; ++k) {
			a = &priv->arl_table[k];
			if (!(a->portmap & BIT(j)))
				continue;
			len += snprintf(buf + len, sizeof(priv->arl_buf) - len,
					"Port %d: MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
					j,
					a->mac[5], a->mac[4], a->mac[3],
					a->mac[2], a->mac[1], a->mac[0]);
		}
	}

	val->value.s = buf;
	val->len = len;

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

int
ar8xxx_sw_set_flush_arl_table(struct switch_dev *dev,
			      const struct switch_attr *attr,
			      struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int ret;

	mutex_lock(&priv->reg_mutex);
	ret = priv->chip->atu_flush(priv);
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

int
ar8xxx_sw_set_flush_port_arl_table(struct switch_dev *dev,
				   const struct switch_attr *attr,
				   struct switch_val *val)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	int port, ret;

	port = val->port_vlan;
	if (port >= dev->ports)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	ret = priv->chip->atu_flush_port(priv, port);
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

int
ar8xxx_sw_get_port_stats(struct switch_dev *dev, int port,
			struct switch_port_stats *stats)
{
	struct ar8xxx_priv *priv = swdev_to_ar8xxx(dev);
	u64 *mib_stats;

	if (!ar8xxx_has_mib_counters(priv) || !priv->mib_poll_interval)
		return -EOPNOTSUPP;

	if (!(priv->chip->mib_rxb_id || priv->chip->mib_txb_id))
		return -EOPNOTSUPP;

	if (port >= dev->ports)
		return -EINVAL;

	mutex_lock(&priv->mib_lock);

	mib_stats = &priv->mib_stats[port * priv->chip->num_mibs];

	stats->tx_bytes = mib_stats[priv->chip->mib_txb_id];
	stats->rx_bytes = mib_stats[priv->chip->mib_rxb_id];

	mutex_unlock(&priv->mib_lock);
	return 0;
}

static int
ar8xxx_phy_read(struct mii_bus *bus, int phy_addr, int reg_addr)
{
	struct ar8xxx_priv *priv = bus->priv;
	return priv->chip->phy_read(priv, phy_addr, reg_addr);
}

static int
ar8xxx_phy_write(struct mii_bus *bus, int phy_addr, int reg_addr,
		 u16 reg_val)
{
	struct ar8xxx_priv *priv = bus->priv;
	return priv->chip->phy_write(priv, phy_addr, reg_addr, reg_val);
}

static const struct switch_attr ar8xxx_sw_attr_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = ar8xxx_sw_set_vlan,
		.get = ar8xxx_sw_get_vlan,
		.max = 1
	},
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = ar8xxx_sw_set_reset_mibs,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "ar8xxx_mib_poll_interval",
		.description = "MIB polling interval in msecs (0 to disable)",
		.set = ar8xxx_sw_set_mib_poll_interval,
		.get = ar8xxx_sw_get_mib_poll_interval
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "ar8xxx_mib_type",
		.description = "MIB type (0=basic 1=extended)",
		.set = ar8xxx_sw_set_mib_type,
		.get = ar8xxx_sw_get_mib_type
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_rx",
		.description = "Enable mirroring of RX packets",
		.set = ar8xxx_sw_set_mirror_rx_enable,
		.get = ar8xxx_sw_get_mirror_rx_enable,
		.max = 1
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_tx",
		.description = "Enable mirroring of TX packets",
		.set = ar8xxx_sw_set_mirror_tx_enable,
		.get = ar8xxx_sw_get_mirror_tx_enable,
		.max = 1
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "mirror_monitor_port",
		.description = "Mirror monitor port",
		.set = ar8xxx_sw_set_mirror_monitor_port,
		.get = ar8xxx_sw_get_mirror_monitor_port,
		.max = AR8216_NUM_PORTS - 1
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "mirror_source_port",
		.description = "Mirror source port",
		.set = ar8xxx_sw_set_mirror_source_port,
		.get = ar8xxx_sw_get_mirror_source_port,
		.max = AR8216_NUM_PORTS - 1
 	},
	{
		.type = SWITCH_TYPE_STRING,
		.name = "arl_table",
		.description = "Get ARL table",
		.set = NULL,
		.get = ar8xxx_sw_get_arl_table,
	},
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "flush_arl_table",
		.description = "Flush ARL table",
		.set = ar8xxx_sw_set_flush_arl_table,
	},
};

const struct switch_attr ar8xxx_sw_attr_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = ar8xxx_sw_set_port_reset_mib,
	},
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get port's MIB counters",
		.set = NULL,
		.get = ar8xxx_sw_get_port_mib,
	},
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "flush_arl_table",
		.description = "Flush port's ARL table entries",
		.set = ar8xxx_sw_set_flush_port_arl_table,
	},
};

const struct switch_attr ar8xxx_sw_attr_vlan[1] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = ar8xxx_sw_set_vid,
		.get = ar8xxx_sw_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops ar8xxx_sw_ops = {
	.attr_global = {
		.attr = ar8xxx_sw_attr_globals,
		.n_attr = ARRAY_SIZE(ar8xxx_sw_attr_globals),
	},
	.attr_port = {
		.attr = ar8xxx_sw_attr_port,
		.n_attr = ARRAY_SIZE(ar8xxx_sw_attr_port),
	},
	.attr_vlan = {
		.attr = ar8xxx_sw_attr_vlan,
		.n_attr = ARRAY_SIZE(ar8xxx_sw_attr_vlan),
	},
	.get_port_pvid = ar8xxx_sw_get_pvid,
	.set_port_pvid = ar8xxx_sw_set_pvid,
	.get_vlan_ports = ar8xxx_sw_get_ports,
	.set_vlan_ports = ar8xxx_sw_set_ports,
	.apply_config = ar8xxx_sw_hw_apply,
	.reset_switch = ar8xxx_sw_reset_switch,
	.get_port_link = ar8xxx_sw_get_port_link,
	.get_port_stats = ar8xxx_sw_get_port_stats,
};

static const struct ar8xxx_chip ar7240sw_chip = {
	.caps = AR8XXX_CAP_MIB_COUNTERS,

	.reg_port_stats_start = 0x20000,
	.reg_port_stats_length = 0x100,
	.reg_arl_ctrl = AR8216_REG_ATU_CTRL,

	.name = "Atheros AR724X/AR933X built-in",
	.ports = AR7240SW_NUM_PORTS,
	.vlans = AR8216_NUM_VLANS,
	.swops = &ar8xxx_sw_ops,

	.hw_init = ar7240sw_hw_init,
	.init_globals = ar7240sw_init_globals,
	.init_port = ar8229_init_port,
	.phy_read = ar8216_phy_read,
	.phy_write = ar8216_phy_write,
	.setup_port = ar7240sw_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.atu_flush_port = ar8216_atu_flush_port,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
	.set_mirror_regs = ar8216_set_mirror_regs,
	.get_arl_entry = ar8216_get_arl_entry,
	.sw_hw_apply = ar8xxx_sw_hw_apply,

	.num_mibs = ARRAY_SIZE(ar8236_mibs),
	.mib_decs = ar8236_mibs,
	.mib_func = AR8216_REG_MIB_FUNC,
	.mib_rxb_id = AR8236_MIB_RXB_ID,
	.mib_txb_id = AR8236_MIB_TXB_ID,
};

static const struct ar8xxx_chip ar8216_chip = {
	.caps = AR8XXX_CAP_MIB_COUNTERS,

	.reg_port_stats_start = 0x19000,
	.reg_port_stats_length = 0xa0,
	.reg_arl_ctrl = AR8216_REG_ATU_CTRL,

	.name = "Atheros AR8216",
	.ports = AR8216_NUM_PORTS,
	.vlans = AR8216_NUM_VLANS,
	.swops = &ar8xxx_sw_ops,

	.hw_init = ar8216_hw_init,
	.init_globals = ar8216_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8216_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.atu_flush_port = ar8216_atu_flush_port,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
	.set_mirror_regs = ar8216_set_mirror_regs,
	.get_arl_entry = ar8216_get_arl_entry,
	.sw_hw_apply = ar8xxx_sw_hw_apply,

	.num_mibs = ARRAY_SIZE(ar8216_mibs),
	.mib_decs = ar8216_mibs,
	.mib_func = AR8216_REG_MIB_FUNC,
	.mib_rxb_id = AR8216_MIB_RXB_ID,
	.mib_txb_id = AR8216_MIB_TXB_ID,
};

static const struct ar8xxx_chip ar8229_chip = {
	.caps = AR8XXX_CAP_MIB_COUNTERS,

	.reg_port_stats_start = 0x20000,
	.reg_port_stats_length = 0x100,
	.reg_arl_ctrl = AR8216_REG_ATU_CTRL,

	.name = "Atheros AR8229",
	.ports = AR8216_NUM_PORTS,
	.vlans = AR8216_NUM_VLANS,
	.swops = &ar8xxx_sw_ops,

	.hw_init = ar8229_hw_init,
	.init_globals = ar8229_init_globals,
	.init_port = ar8229_init_port,
	.phy_read = ar8216_phy_read,
	.phy_write = ar8216_phy_write,
	.setup_port = ar8236_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.atu_flush_port = ar8216_atu_flush_port,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
	.set_mirror_regs = ar8216_set_mirror_regs,
	.get_arl_entry = ar8216_get_arl_entry,
	.sw_hw_apply = ar8xxx_sw_hw_apply,

	.num_mibs = ARRAY_SIZE(ar8236_mibs),
	.mib_decs = ar8236_mibs,
	.mib_func = AR8216_REG_MIB_FUNC,
	.mib_rxb_id = AR8236_MIB_RXB_ID,
	.mib_txb_id = AR8236_MIB_TXB_ID,
};

static const struct ar8xxx_chip ar8236_chip = {
	.caps = AR8XXX_CAP_MIB_COUNTERS,

	.reg_port_stats_start = 0x20000,
	.reg_port_stats_length = 0x100,
	.reg_arl_ctrl = AR8216_REG_ATU_CTRL,

	.name = "Atheros AR8236",
	.ports = AR8216_NUM_PORTS,
	.vlans = AR8216_NUM_VLANS,
	.swops = &ar8xxx_sw_ops,

	.hw_init = ar8216_hw_init,
	.init_globals = ar8236_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8236_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.atu_flush_port = ar8216_atu_flush_port,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
	.set_mirror_regs = ar8216_set_mirror_regs,
	.get_arl_entry = ar8216_get_arl_entry,
	.sw_hw_apply = ar8xxx_sw_hw_apply,

	.num_mibs = ARRAY_SIZE(ar8236_mibs),
	.mib_decs = ar8236_mibs,
	.mib_func = AR8216_REG_MIB_FUNC,
	.mib_rxb_id = AR8236_MIB_RXB_ID,
	.mib_txb_id = AR8236_MIB_TXB_ID,
};

static const struct ar8xxx_chip ar8316_chip = {
	.caps = AR8XXX_CAP_GIGE | AR8XXX_CAP_MIB_COUNTERS,

	.reg_port_stats_start = 0x20000,
	.reg_port_stats_length = 0x100,
	.reg_arl_ctrl = AR8216_REG_ATU_CTRL,

	.name = "Atheros AR8316",
	.ports = AR8216_NUM_PORTS,
	.vlans = AR8X16_MAX_VLANS,
	.swops = &ar8xxx_sw_ops,

	.hw_init = ar8316_hw_init,
	.init_globals = ar8316_init_globals,
	.init_port = ar8216_init_port,
	.setup_port = ar8216_setup_port,
	.read_port_status = ar8216_read_port_status,
	.atu_flush = ar8216_atu_flush,
	.atu_flush_port = ar8216_atu_flush_port,
	.vtu_flush = ar8216_vtu_flush,
	.vtu_load_vlan = ar8216_vtu_load_vlan,
	.set_mirror_regs = ar8216_set_mirror_regs,
	.get_arl_entry = ar8216_get_arl_entry,
	.sw_hw_apply = ar8xxx_sw_hw_apply,

	.num_mibs = ARRAY_SIZE(ar8236_mibs),
	.mib_decs = ar8236_mibs,
	.mib_func = AR8216_REG_MIB_FUNC,
	.mib_rxb_id = AR8236_MIB_RXB_ID,
	.mib_txb_id = AR8236_MIB_TXB_ID,
};

static int
ar8xxx_read_id(struct ar8xxx_priv *priv)
{
	u32 val;
	u16 id;
	int i;

	val = ar8xxx_read(priv, AR8216_REG_CTRL);
	if (val == ~0)
		return -ENODEV;

	id = val & (AR8216_CTRL_REVISION | AR8216_CTRL_VERSION);
	for (i = 0; i < AR8X16_PROBE_RETRIES; i++) {
		u16 t;

		val = ar8xxx_read(priv, AR8216_REG_CTRL);
		if (val == ~0)
			return -ENODEV;

		t = val & (AR8216_CTRL_REVISION | AR8216_CTRL_VERSION);
		if (t != id)
			return -ENODEV;
	}

	priv->chip_ver = (id & AR8216_CTRL_VERSION) >> AR8216_CTRL_VERSION_S;
	priv->chip_rev = (id & AR8216_CTRL_REVISION);
	return 0;
}

static int
ar8xxx_id_chip(struct ar8xxx_priv *priv)
{
	int ret;

	ret = ar8xxx_read_id(priv);
	if(ret)
		return ret;

	switch (priv->chip_ver) {
	case AR8XXX_VER_AR8216:
		priv->chip = &ar8216_chip;
		break;
	case AR8XXX_VER_AR8236:
		priv->chip = &ar8236_chip;
		break;
	case AR8XXX_VER_AR8316:
		priv->chip = &ar8316_chip;
		break;
	case AR8XXX_VER_AR8327:
		priv->chip = &ar8327_chip;
		break;
	case AR8XXX_VER_AR8337:
		priv->chip = &ar8337_chip;
		break;
	default:
		pr_err("ar8216: Unknown Atheros device [ver=%d, rev=%d]\n",
		       priv->chip_ver, priv->chip_rev);

		return -ENODEV;
	}

	return 0;
}

static void
ar8xxx_mib_work_func(struct work_struct *work)
{
	struct ar8xxx_priv *priv;
	int err, i;

	priv = container_of(work, struct ar8xxx_priv, mib_work.work);

	mutex_lock(&priv->mib_lock);

	err = ar8xxx_mib_capture(priv);
	if (err)
		goto next_attempt;

	for (i = 0; i < priv->dev.ports; i++)
		ar8xxx_mib_fetch_port_stat(priv, i, false);

next_attempt:
	mutex_unlock(&priv->mib_lock);
	schedule_delayed_work(&priv->mib_work,
			      msecs_to_jiffies(priv->mib_poll_interval));
}

static int
ar8xxx_mib_init(struct ar8xxx_priv *priv)
{
	unsigned int len;

	if (!ar8xxx_has_mib_counters(priv))
		return 0;

	BUG_ON(!priv->chip->mib_decs || !priv->chip->num_mibs);

	len = priv->dev.ports * priv->chip->num_mibs *
	      sizeof(*priv->mib_stats);
	priv->mib_stats = kzalloc(len, GFP_KERNEL);

	if (!priv->mib_stats)
		return -ENOMEM;

	return 0;
}

static void
ar8xxx_mib_start(struct ar8xxx_priv *priv)
{
	if (!ar8xxx_has_mib_counters(priv) || !priv->mib_poll_interval)
		return;

	schedule_delayed_work(&priv->mib_work,
			      msecs_to_jiffies(priv->mib_poll_interval));
}

static void
ar8xxx_mib_stop(struct ar8xxx_priv *priv)
{
	if (!ar8xxx_has_mib_counters(priv) || !priv->mib_poll_interval)
		return;

	cancel_delayed_work_sync(&priv->mib_work);
}

static struct ar8xxx_priv *
ar8xxx_create(void)
{
	struct ar8xxx_priv *priv;

	priv = kzalloc(sizeof(struct ar8xxx_priv), GFP_KERNEL);
	if (priv == NULL)
		return NULL;

	mutex_init(&priv->reg_mutex);
	mutex_init(&priv->mib_lock);
	INIT_DELAYED_WORK(&priv->mib_work, ar8xxx_mib_work_func);

	return priv;
}

static void
ar8xxx_free(struct ar8xxx_priv *priv)
{
	if (priv->chip && priv->chip->cleanup)
		priv->chip->cleanup(priv);

	kfree(priv->chip_data);
	kfree(priv->mib_stats);
	kfree(priv);
}

static int
ar8xxx_probe_switch(struct ar8xxx_priv *priv)
{
	const struct ar8xxx_chip *chip;
	struct switch_dev *swdev;
	int ret;

	chip = priv->chip;

	swdev = &priv->dev;
	swdev->cpu_port = AR8216_PORT_CPU;
	swdev->name = chip->name;
	swdev->vlans = chip->vlans;
	swdev->ports = chip->ports;
	swdev->ops = chip->swops;

	ret = ar8xxx_mib_init(priv);
	if (ret)
		return ret;

	return 0;
}

static int
ar8xxx_start(struct ar8xxx_priv *priv)
{
	int ret;

	priv->init = true;

	ret = priv->chip->hw_init(priv);
	if (ret)
		return ret;

	ret = ar8xxx_sw_reset_switch(&priv->dev);
	if (ret)
		return ret;

	priv->init = false;

	ar8xxx_mib_start(priv);

	return 0;
}

static int
ar8xxx_phy_config_init(struct phy_device *phydev)
{
	struct ar8xxx_priv *priv = phydev->priv;
#ifdef CONFIG_ETHERNET_PACKET_MANGLE
	struct net_device *dev = phydev->attached_dev;
#endif
	int ret;

	if (WARN_ON(!priv))
		return -ENODEV;

	if (priv->chip->config_at_probe)
		return ar8xxx_phy_check_aneg(phydev);

	priv->phy = phydev;

	if (phydev->mdio.addr != 0) {
		if (chip_is_ar8316(priv)) {
			/* switch device has been initialized, reinit */
			priv->dev.ports = (AR8216_NUM_PORTS - 1);
			priv->initialized = false;
			priv->port4_phy = true;
			ar8316_hw_init(priv);
			return 0;
		}

		return 0;
	}

	ret = ar8xxx_start(priv);
	if (ret)
		return ret;

#ifdef CONFIG_ETHERNET_PACKET_MANGLE
	/* VID fixup only needed on ar8216 */
	if (chip_is_ar8216(priv)) {
		dev->phy_ptr = priv;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
		dev->priv_flags |= IFF_NO_IP_ALIGN;
#else
		dev->extra_priv_flags |= IFF_NO_IP_ALIGN;
#endif
		dev->eth_mangle_rx = ar8216_mangle_rx;
		dev->eth_mangle_tx = ar8216_mangle_tx;
	}
#endif

	return 0;
}

static bool
ar8xxx_check_link_states(struct ar8xxx_priv *priv)
{
	bool link_new, changed = false;
	u32 status;
	int i;

	mutex_lock(&priv->reg_mutex);

	for (i = 0; i < priv->dev.ports; i++) {
		status = priv->chip->read_port_status(priv, i);
		link_new = !!(status & AR8216_PORT_STATUS_LINK_UP);
		if (link_new == priv->link_up[i])
			continue;

		priv->link_up[i] = link_new;
		changed = true;
		/* flush ARL entries for this port if it went down*/
		if (!link_new)
			priv->chip->atu_flush_port(priv, i);
		dev_info(&priv->phy->mdio.dev, "Port %d is %s\n",
			 i, link_new ? "up" : "down");
	}

	mutex_unlock(&priv->reg_mutex);

	return changed;
}

static int
ar8xxx_phy_read_status(struct phy_device *phydev)
{
	struct ar8xxx_priv *priv = phydev->priv;
	struct switch_port_link link;

	/* check for switch port link changes */
	ar8xxx_check_link_states(priv);

	if (phydev->mdio.addr != 0)
		return genphy_read_status(phydev);

	ar8216_read_port_link(priv, phydev->mdio.addr, &link);
	phydev->link = !!link.link;
	if (!phydev->link)
		return 0;

	switch (link.speed) {
	case SWITCH_PORT_SPEED_10:
		phydev->speed = SPEED_10;
		break;
	case SWITCH_PORT_SPEED_100:
		phydev->speed = SPEED_100;
		break;
	case SWITCH_PORT_SPEED_1000:
		phydev->speed = SPEED_1000;
		break;
	default:
		phydev->speed = 0;
	}
	phydev->duplex = link.duplex ? DUPLEX_FULL : DUPLEX_HALF;

	phydev->state = PHY_RUNNING;
	netif_carrier_on(phydev->attached_dev);
	if (phydev->adjust_link)
		phydev->adjust_link(phydev->attached_dev);

	return 0;
}

static int
ar8xxx_phy_config_aneg(struct phy_device *phydev)
{
	if (phydev->mdio.addr == 0)
		return 0;

	return genphy_config_aneg(phydev);
}

static int
ar8xxx_get_features(struct phy_device *phydev)
{
	struct ar8xxx_priv *priv = phydev->priv;

	linkmode_copy(phydev->supported, PHY_BASIC_FEATURES);
	if (ar8xxx_has_gige(priv))
		linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, phydev->supported);

	return 0;
}

static const u32 ar8xxx_phy_ids[] = {
	0x004dd033,
	0x004dd034, /* AR8327 */
	0x004dd036, /* AR8337 */
	0x004dd041,
	0x004dd042,
	0x004dd043, /* AR8236 */
};

static bool
ar8xxx_phy_match(u32 phy_id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ar8xxx_phy_ids); i++)
		if (phy_id == ar8xxx_phy_ids[i])
			return true;

	return false;
}

static bool
ar8xxx_is_possible(struct mii_bus *bus)
{
	unsigned int i, found_phys = 0;

	for (i = 0; i < 5; i++) {
		u32 phy_id;

		phy_id = mdiobus_read(bus, i, MII_PHYSID1) << 16;
		phy_id |= mdiobus_read(bus, i, MII_PHYSID2);
		if (ar8xxx_phy_match(phy_id)) {
			found_phys++;
		} else if (phy_id) {
			pr_debug("ar8xxx: unknown PHY at %s:%02x id:%08x\n",
				 dev_name(&bus->dev), i, phy_id);
		}
	}
	return !!found_phys;
}

static int
ar8xxx_phy_probe(struct phy_device *phydev)
{
	struct ar8xxx_priv *priv;
	struct switch_dev *swdev;
	int ret;

	/* skip PHYs at unused adresses */
	if (phydev->mdio.addr != 0 && phydev->mdio.addr != 3 && phydev->mdio.addr != 4)
		return -ENODEV;

	if (!ar8xxx_is_possible(phydev->mdio.bus))
		return -ENODEV;

	mutex_lock(&ar8xxx_dev_list_lock);
	list_for_each_entry(priv, &ar8xxx_dev_list, list)
		if (priv->mii_bus == phydev->mdio.bus)
			goto found;

	priv = ar8xxx_create();
	if (priv == NULL) {
		ret = -ENOMEM;
		goto unlock;
	}

	priv->mii_bus = phydev->mdio.bus;
	priv->pdev = &phydev->mdio.dev;

	ret = of_property_read_u32(priv->pdev->of_node, "qca,mib-poll-interval",
				   &priv->mib_poll_interval);
	if (ret)
		priv->mib_poll_interval = 0;

	ret = ar8xxx_id_chip(priv);
	if (ret)
		goto free_priv;

	ret = ar8xxx_probe_switch(priv);
	if (ret)
		goto free_priv;

	swdev = &priv->dev;
	swdev->alias = dev_name(&priv->mii_bus->dev);
	ret = register_switch(swdev, NULL);
	if (ret)
		goto free_priv;

	pr_info("%s: %s rev. %u switch registered on %s\n",
		swdev->devname, swdev->name, priv->chip_rev,
		dev_name(&priv->mii_bus->dev));

	list_add(&priv->list, &ar8xxx_dev_list);

found:
	priv->use_count++;

	if (phydev->mdio.addr == 0 && priv->chip->config_at_probe) {
		priv->phy = phydev;

		ret = ar8xxx_start(priv);
		if (ret)
			goto err_unregister_switch;
	} else if (priv->chip->phy_rgmii_set) {
		priv->chip->phy_rgmii_set(priv, phydev);
	}

	phydev->priv = priv;

	mutex_unlock(&ar8xxx_dev_list_lock);

	return 0;

err_unregister_switch:
	if (--priv->use_count)
		goto unlock;

	unregister_switch(&priv->dev);

free_priv:
	ar8xxx_free(priv);
unlock:
	mutex_unlock(&ar8xxx_dev_list_lock);
	return ret;
}

static void
ar8xxx_phy_detach(struct phy_device *phydev)
{
	struct net_device *dev = phydev->attached_dev;

	if (!dev)
		return;

#ifdef CONFIG_ETHERNET_PACKET_MANGLE
	dev->phy_ptr = NULL;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,1,0)
	dev->priv_flags &= ~IFF_NO_IP_ALIGN;
#else
	dev->extra_priv_flags &= ~IFF_NO_IP_ALIGN;
#endif
	dev->eth_mangle_rx = NULL;
	dev->eth_mangle_tx = NULL;
#endif
}

static void
ar8xxx_phy_remove(struct phy_device *phydev)
{
	struct ar8xxx_priv *priv = phydev->priv;

	if (WARN_ON(!priv))
		return;

	phydev->priv = NULL;

	mutex_lock(&ar8xxx_dev_list_lock);

	if (--priv->use_count > 0) {
		mutex_unlock(&ar8xxx_dev_list_lock);
		return;
	}

	list_del(&priv->list);
	mutex_unlock(&ar8xxx_dev_list_lock);

	unregister_switch(&priv->dev);
	ar8xxx_mib_stop(priv);
	ar8xxx_free(priv);
}

static struct phy_driver ar8xxx_phy_driver[] = {
	{
		.phy_id		= 0x004d0000,
		.name		= "Atheros AR8216/AR8236/AR8316",
		.phy_id_mask	= 0xffff0000,
		.probe		= ar8xxx_phy_probe,
		.remove		= ar8xxx_phy_remove,
		.detach		= ar8xxx_phy_detach,
		.config_init	= ar8xxx_phy_config_init,
		.config_aneg	= ar8xxx_phy_config_aneg,
		.read_status	= ar8xxx_phy_read_status,
		.get_features	= ar8xxx_get_features,
	}
};

static const struct of_device_id ar8xxx_mdiodev_of_match[] = {
	{
		.compatible = "qca,ar7240sw",
		.data = &ar7240sw_chip,
	}, {
		.compatible = "qca,ar8229",
		.data = &ar8229_chip,
	}, {
		.compatible = "qca,ar8236",
		.data = &ar8236_chip,
	}, {
		.compatible = "qca,ar8327",
		.data = &ar8327_chip,
	},
	{ /* sentinel */ },
};

static int
ar8xxx_mdiodev_probe(struct mdio_device *mdiodev)
{
	const struct of_device_id *match;
	struct ar8xxx_priv *priv;
	struct switch_dev *swdev;
	struct device_node *mdio_node;
	int ret;

	match = of_match_device(ar8xxx_mdiodev_of_match, &mdiodev->dev);
	if (!match)
		return -EINVAL;

	priv = ar8xxx_create();
	if (priv == NULL)
		return -ENOMEM;

	priv->mii_bus = mdiodev->bus;
	priv->pdev = &mdiodev->dev;
	priv->chip = (const struct ar8xxx_chip *) match->data;

	ret = of_property_read_u32(priv->pdev->of_node, "qca,mib-poll-interval",
				   &priv->mib_poll_interval);
	if (ret)
		priv->mib_poll_interval = 0;

	ret = ar8xxx_read_id(priv);
	if (ret)
		goto free_priv;

	ret = ar8xxx_probe_switch(priv);
	if (ret)
		goto free_priv;

	if (priv->chip->phy_read && priv->chip->phy_write) {
		priv->sw_mii_bus = devm_mdiobus_alloc(&mdiodev->dev);
		priv->sw_mii_bus->name = "ar8xxx-mdio";
		priv->sw_mii_bus->read = ar8xxx_phy_read;
		priv->sw_mii_bus->write = ar8xxx_phy_write;
		priv->sw_mii_bus->priv = priv;
		priv->sw_mii_bus->parent = &mdiodev->dev;
		snprintf(priv->sw_mii_bus->id, MII_BUS_ID_SIZE, "%s",
			 dev_name(&mdiodev->dev));
		mdio_node = of_get_child_by_name(priv->pdev->of_node, "mdio-bus");
		ret = of_mdiobus_register(priv->sw_mii_bus, mdio_node);
		if (ret)
			goto free_priv;
	}

	swdev = &priv->dev;
	swdev->alias = dev_name(&mdiodev->dev);

	if (of_property_read_bool(priv->pdev->of_node, "qca,phy4-mii-enable")) {
		priv->port4_phy = true;
		swdev->ports--;
	}

	ret = register_switch(swdev, NULL);
	if (ret)
		goto free_priv;

	pr_info("%s: %s rev. %u switch registered on %s\n",
		swdev->devname, swdev->name, priv->chip_rev,
		dev_name(&priv->mii_bus->dev));

	mutex_lock(&ar8xxx_dev_list_lock);
	list_add(&priv->list, &ar8xxx_dev_list);
	mutex_unlock(&ar8xxx_dev_list_lock);

	priv->use_count++;

	ret = ar8xxx_start(priv);
	if (ret)
		goto err_unregister_switch;

	dev_set_drvdata(&mdiodev->dev, priv);

	return 0;

err_unregister_switch:
	if (--priv->use_count)
		return ret;

	unregister_switch(&priv->dev);

free_priv:
	ar8xxx_free(priv);
	return ret;
}

static void
ar8xxx_mdiodev_remove(struct mdio_device *mdiodev)
{
	struct ar8xxx_priv *priv = dev_get_drvdata(&mdiodev->dev);

	if (WARN_ON(!priv))
		return;

	mutex_lock(&ar8xxx_dev_list_lock);

	if (--priv->use_count > 0) {
		mutex_unlock(&ar8xxx_dev_list_lock);
		return;
	}

	list_del(&priv->list);
	mutex_unlock(&ar8xxx_dev_list_lock);

	unregister_switch(&priv->dev);
	ar8xxx_mib_stop(priv);
	if(priv->sw_mii_bus)
		mdiobus_unregister(priv->sw_mii_bus);
	ar8xxx_free(priv);
}

static struct mdio_driver ar8xxx_mdio_driver = {
	.probe  = ar8xxx_mdiodev_probe,
	.remove = ar8xxx_mdiodev_remove,
	.mdiodrv.driver = {
		.name = "ar8xxx-switch",
		.of_match_table = ar8xxx_mdiodev_of_match,
	},
};

static int __init ar8216_init(void)
{
	int ret;

	ret = phy_drivers_register(ar8xxx_phy_driver,
				   ARRAY_SIZE(ar8xxx_phy_driver),
				   THIS_MODULE);
	if (ret)
		return ret;

	ret = mdio_driver_register(&ar8xxx_mdio_driver);
	if (ret)
		phy_drivers_unregister(ar8xxx_phy_driver,
				       ARRAY_SIZE(ar8xxx_phy_driver));

	return ret;
}
module_init(ar8216_init);

static void __exit ar8216_exit(void)
{
	mdio_driver_unregister(&ar8xxx_mdio_driver);
	phy_drivers_unregister(ar8xxx_phy_driver,
			        ARRAY_SIZE(ar8xxx_phy_driver));
}
module_exit(ar8216_exit);

MODULE_LICENSE("GPL");
