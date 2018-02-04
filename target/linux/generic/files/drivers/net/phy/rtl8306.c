/*
 * rtl8306.c: RTL8306S switch driver
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
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
#include <net/genetlink.h>
#include <linux/switch.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/version.h>

//#define DEBUG 1

/* Global (PHY0) */
#define RTL8306_REG_PAGE		16
#define RTL8306_REG_PAGE_LO		(1 << 15)
#define RTL8306_REG_PAGE_HI		(1 << 1) /* inverted */

#define RTL8306_NUM_VLANS		16
#define RTL8306_NUM_PORTS		6
#define RTL8306_PORT_CPU		5
#define RTL8306_NUM_PAGES		4
#define RTL8306_NUM_REGS		32

#define RTL_NAME_S          "RTL8306S"
#define RTL_NAME_SD         "RTL8306SD"
#define RTL_NAME_SDM        "RTL8306SDM"
#define RTL_NAME_UNKNOWN    "RTL8306(unknown)"

#define RTL8306_MAGIC	0x8306

static LIST_HEAD(phydevs);

struct rtl_priv {
	struct list_head list;
	struct switch_dev dev;
	int page;
	int type;
	int do_cpu;
	struct mii_bus *bus;
	char hwname[sizeof(RTL_NAME_UNKNOWN)];
	bool fixup;
};

struct rtl_phyregs {
	int nway;
	int speed;
	int duplex;
};

#define to_rtl(_dev) container_of(_dev, struct rtl_priv, dev)

enum {
	RTL_TYPE_S,
	RTL_TYPE_SD,
	RTL_TYPE_SDM,
};

struct rtl_reg {
	int page;
	int phy;
	int reg;
	int bits;
	int shift;
	int inverted;
};

#define RTL_VLAN_REGOFS(name) \
	(RTL_REG_VLAN1_##name - RTL_REG_VLAN0_##name)

#define RTL_PORT_REGOFS(name) \
	(RTL_REG_PORT1_##name - RTL_REG_PORT0_##name)

#define RTL_PORT_REG(id, reg) \
	(RTL_REG_PORT0_##reg + (id * RTL_PORT_REGOFS(reg)))

#define RTL_VLAN_REG(id, reg) \
	(RTL_REG_VLAN0_##reg + (id * RTL_VLAN_REGOFS(reg)))

#define RTL_GLOBAL_REGATTR(reg) \
	.id = RTL_REG_##reg, \
	.type = SWITCH_TYPE_INT, \
	.ofs = 0, \
	.set = rtl_attr_set_int, \
	.get = rtl_attr_get_int

#define RTL_PORT_REGATTR(reg) \
	.id = RTL_REG_PORT0_##reg, \
	.type = SWITCH_TYPE_INT, \
	.ofs = RTL_PORT_REGOFS(reg), \
	.set = rtl_attr_set_port_int, \
	.get = rtl_attr_get_port_int

#define RTL_VLAN_REGATTR(reg) \
	.id = RTL_REG_VLAN0_##reg, \
	.type = SWITCH_TYPE_INT, \
	.ofs = RTL_VLAN_REGOFS(reg), \
	.set = rtl_attr_set_vlan_int, \
	.get = rtl_attr_get_vlan_int

enum rtl_regidx {
	RTL_REG_CHIPID,
	RTL_REG_CHIPVER,
	RTL_REG_CHIPTYPE,
	RTL_REG_CPUPORT,

	RTL_REG_EN_CPUPORT,
	RTL_REG_EN_TAG_OUT,
	RTL_REG_EN_TAG_CLR,
	RTL_REG_EN_TAG_IN,
	RTL_REG_TRAP_CPU,
	RTL_REG_CPU_LINKUP,
	RTL_REG_TRUNK_PORTSEL,
	RTL_REG_EN_TRUNK,
	RTL_REG_RESET,

	RTL_REG_VLAN_ENABLE,
	RTL_REG_VLAN_FILTER,
	RTL_REG_VLAN_TAG_ONLY,
	RTL_REG_VLAN_TAG_AWARE,
#define RTL_VLAN_ENUM(id) \
	RTL_REG_VLAN##id##_VID, \
	RTL_REG_VLAN##id##_PORTMASK
	RTL_VLAN_ENUM(0),
	RTL_VLAN_ENUM(1),
	RTL_VLAN_ENUM(2),
	RTL_VLAN_ENUM(3),
	RTL_VLAN_ENUM(4),
	RTL_VLAN_ENUM(5),
	RTL_VLAN_ENUM(6),
	RTL_VLAN_ENUM(7),
	RTL_VLAN_ENUM(8),
	RTL_VLAN_ENUM(9),
	RTL_VLAN_ENUM(10),
	RTL_VLAN_ENUM(11),
	RTL_VLAN_ENUM(12),
	RTL_VLAN_ENUM(13),
	RTL_VLAN_ENUM(14),
	RTL_VLAN_ENUM(15),
#define RTL_PORT_ENUM(id) \
	RTL_REG_PORT##id##_PVID, \
	RTL_REG_PORT##id##_NULL_VID_REPLACE, \
	RTL_REG_PORT##id##_NON_PVID_DISCARD, \
	RTL_REG_PORT##id##_VID_INSERT, \
	RTL_REG_PORT##id##_TAG_INSERT, \
	RTL_REG_PORT##id##_LINK, \
	RTL_REG_PORT##id##_SPEED, \
	RTL_REG_PORT##id##_NWAY, \
	RTL_REG_PORT##id##_NRESTART, \
	RTL_REG_PORT##id##_DUPLEX, \
	RTL_REG_PORT##id##_RXEN, \
	RTL_REG_PORT##id##_TXEN
	RTL_PORT_ENUM(0),
	RTL_PORT_ENUM(1),
	RTL_PORT_ENUM(2),
	RTL_PORT_ENUM(3),
	RTL_PORT_ENUM(4),
	RTL_PORT_ENUM(5),
};

static const struct rtl_reg rtl_regs[] = {
	[RTL_REG_CHIPID]         = { 0, 4, 30, 16,  0, 0 },
	[RTL_REG_CHIPVER]        = { 0, 4, 31,  8,  0, 0 },
	[RTL_REG_CHIPTYPE]       = { 0, 4, 31,  2,  8, 0 },

	/* CPU port number */
	[RTL_REG_CPUPORT]        = { 2, 4, 21,  3,  0, 0 },
	/* Enable CPU port function */
	[RTL_REG_EN_CPUPORT]     = { 3, 2, 21,  1, 15, 1 },
	/* Enable CPU port tag insertion */
	[RTL_REG_EN_TAG_OUT]     = { 3, 2, 21,  1, 12, 0 },
	/* Enable CPU port tag removal */
	[RTL_REG_EN_TAG_CLR]     = { 3, 2, 21,  1, 11, 0 },
	/* Enable CPU port tag checking */
	[RTL_REG_EN_TAG_IN]      = { 0, 4, 21,  1,  7, 0 },
	[RTL_REG_EN_TRUNK]       = { 0, 0, 19,  1, 11, 1 },
	[RTL_REG_TRUNK_PORTSEL]  = { 0, 0, 16,  1,  6, 1 },
	[RTL_REG_RESET]          = { 0, 0, 16,  1, 12, 0 },

	[RTL_REG_TRAP_CPU]       = { 3, 2, 22,  1,  6, 0 },
	[RTL_REG_CPU_LINKUP]     = { 0, 6, 22,  1, 15, 0 },

	[RTL_REG_VLAN_TAG_ONLY]  = { 0, 0, 16,  1,  8, 1 },
	[RTL_REG_VLAN_FILTER]    = { 0, 0, 16,  1,  9, 1 },
	[RTL_REG_VLAN_TAG_AWARE] = { 0, 0, 16,  1, 10, 1 },
	[RTL_REG_VLAN_ENABLE]    = { 0, 0, 18,  1,  8, 1 },

#define RTL_VLAN_REGS(id, phy, page, regofs) \
	[RTL_REG_VLAN##id##_VID] = { page, phy, 25 + regofs, 12, 0, 0 }, \
	[RTL_REG_VLAN##id##_PORTMASK] = { page, phy, 24 + regofs, 6, 0, 0 }
	RTL_VLAN_REGS( 0, 0, 0, 0),
	RTL_VLAN_REGS( 1, 1, 0, 0),
	RTL_VLAN_REGS( 2, 2, 0, 0),
	RTL_VLAN_REGS( 3, 3, 0, 0),
	RTL_VLAN_REGS( 4, 4, 0, 0),
	RTL_VLAN_REGS( 5, 0, 1, 2),
	RTL_VLAN_REGS( 6, 1, 1, 2),
	RTL_VLAN_REGS( 7, 2, 1, 2),
	RTL_VLAN_REGS( 8, 3, 1, 2),
	RTL_VLAN_REGS( 9, 4, 1, 2),
	RTL_VLAN_REGS(10, 0, 1, 4),
	RTL_VLAN_REGS(11, 1, 1, 4),
	RTL_VLAN_REGS(12, 2, 1, 4),
	RTL_VLAN_REGS(13, 3, 1, 4),
	RTL_VLAN_REGS(14, 4, 1, 4),
	RTL_VLAN_REGS(15, 0, 1, 6),

#define REG_PORT_SETTING(port, phy) \
	[RTL_REG_PORT##port##_SPEED] = { 0, phy, 0, 1, 13, 0 }, \
	[RTL_REG_PORT##port##_NWAY] = { 0, phy, 0, 1, 12, 0 }, \
	[RTL_REG_PORT##port##_NRESTART] = { 0, phy, 0, 1, 9, 0 }, \
	[RTL_REG_PORT##port##_DUPLEX] = { 0, phy, 0, 1, 8, 0 }, \
	[RTL_REG_PORT##port##_TXEN] = { 0, phy, 24, 1, 11, 0 }, \
	[RTL_REG_PORT##port##_RXEN] = { 0, phy, 24, 1, 10, 0 }, \
	[RTL_REG_PORT##port##_LINK] = { 0, phy, 1, 1, 2, 0 }, \
	[RTL_REG_PORT##port##_NULL_VID_REPLACE] = { 0, phy, 22, 1, 12, 0 }, \
	[RTL_REG_PORT##port##_NON_PVID_DISCARD] = { 0, phy, 22, 1, 11, 0 }, \
	[RTL_REG_PORT##port##_VID_INSERT] = { 0, phy, 22, 2, 9, 0 }, \
	[RTL_REG_PORT##port##_TAG_INSERT] = { 0, phy, 22, 2, 0, 0 }

	REG_PORT_SETTING(0, 0),
	REG_PORT_SETTING(1, 1),
	REG_PORT_SETTING(2, 2),
	REG_PORT_SETTING(3, 3),
	REG_PORT_SETTING(4, 4),
	REG_PORT_SETTING(5, 6),

#define REG_PORT_PVID(phy, page, regofs) \
	{ page, phy, 24 + regofs, 4, 12, 0 }
	[RTL_REG_PORT0_PVID] = REG_PORT_PVID(0, 0, 0),
	[RTL_REG_PORT1_PVID] = REG_PORT_PVID(1, 0, 0),
	[RTL_REG_PORT2_PVID] = REG_PORT_PVID(2, 0, 0),
	[RTL_REG_PORT3_PVID] = REG_PORT_PVID(3, 0, 0),
	[RTL_REG_PORT4_PVID] = REG_PORT_PVID(4, 0, 0),
	[RTL_REG_PORT5_PVID] = REG_PORT_PVID(0, 1, 2),
};


static inline void
rtl_set_page(struct rtl_priv *priv, unsigned int page)
{
	struct mii_bus *bus = priv->bus;
	u16 pgsel;

	if (priv->fixup)
		return;

	if (priv->page == page)
		return;

	BUG_ON(page > RTL8306_NUM_PAGES);
	pgsel = bus->read(bus, 0, RTL8306_REG_PAGE);
	pgsel &= ~(RTL8306_REG_PAGE_LO | RTL8306_REG_PAGE_HI);
	if (page & (1 << 0))
		pgsel |= RTL8306_REG_PAGE_LO;
	if (!(page & (1 << 1))) /* bit is inverted */
		pgsel |= RTL8306_REG_PAGE_HI;
	bus->write(bus, 0, RTL8306_REG_PAGE, pgsel);
}

static inline int
rtl_w16(struct switch_dev *dev, unsigned int page, unsigned int phy, unsigned int reg, u16 val)
{
	struct rtl_priv *priv = to_rtl(dev);
	struct mii_bus *bus = priv->bus;

	rtl_set_page(priv, page);
	bus->write(bus, phy, reg, val);
	bus->read(bus, phy, reg); /* flush */
	return 0;
}

static inline int
rtl_r16(struct switch_dev *dev, unsigned int page, unsigned int phy, unsigned int reg)
{
	struct rtl_priv *priv = to_rtl(dev);
	struct mii_bus *bus = priv->bus;

	rtl_set_page(priv, page);
	return bus->read(bus, phy, reg);
}

static inline u16
rtl_rmw(struct switch_dev *dev, unsigned int page, unsigned int phy, unsigned int reg, u16 mask, u16 val)
{
	struct rtl_priv *priv = to_rtl(dev);
	struct mii_bus *bus = priv->bus;
	u16 r;

	rtl_set_page(priv, page);
	r = bus->read(bus, phy, reg);
	r &= ~mask;
	r |= val;
	bus->write(bus, phy, reg, r);
	return bus->read(bus, phy, reg); /* flush */
}


static inline int
rtl_get(struct switch_dev *dev, enum rtl_regidx s)
{
	const struct rtl_reg *r = &rtl_regs[s];
	u16 val;

	BUG_ON(s >= ARRAY_SIZE(rtl_regs));
	if (r->bits == 0) /* unimplemented */
		return 0;

	val = rtl_r16(dev, r->page, r->phy, r->reg);

	if (r->shift > 0)
		val >>= r->shift;

	if (r->inverted)
		val = ~val;

	val &= (1 << r->bits) - 1;

	return val;
}

static int
rtl_set(struct switch_dev *dev, enum rtl_regidx s, unsigned int val)
{
	const struct rtl_reg *r = &rtl_regs[s];
	u16 mask = 0xffff;

	BUG_ON(s >= ARRAY_SIZE(rtl_regs));

	if (r->bits == 0) /* unimplemented */
		return 0;

	if (r->shift > 0)
		val <<= r->shift;

	if (r->inverted)
		val = ~val;

	if (r->bits != 16) {
		mask = (1 << r->bits) - 1;
		mask <<= r->shift;
	}
	val &= mask;
	return rtl_rmw(dev, r->page, r->phy, r->reg, mask, val);
}

static void
rtl_phy_save(struct switch_dev *dev, int port, struct rtl_phyregs *regs)
{
	regs->nway = rtl_get(dev, RTL_PORT_REG(port, NWAY));
	regs->speed = rtl_get(dev, RTL_PORT_REG(port, SPEED));
	regs->duplex = rtl_get(dev, RTL_PORT_REG(port, DUPLEX));
}

static void
rtl_phy_restore(struct switch_dev *dev, int port, struct rtl_phyregs *regs)
{
	rtl_set(dev, RTL_PORT_REG(port, NWAY), regs->nway);
	rtl_set(dev, RTL_PORT_REG(port, SPEED), regs->speed);
	rtl_set(dev, RTL_PORT_REG(port, DUPLEX), regs->duplex);
}

static void
rtl_port_set_enable(struct switch_dev *dev, int port, int enabled)
{
	rtl_set(dev, RTL_PORT_REG(port, RXEN), enabled);
	rtl_set(dev, RTL_PORT_REG(port, TXEN), enabled);

	if ((port >= 5) || !enabled)
		return;

	/* restart autonegotiation if enabled */
	rtl_set(dev, RTL_PORT_REG(port, NRESTART), 1);
}

static int
rtl_hw_apply(struct switch_dev *dev)
{
	int i;
	int trunk_en, trunk_psel;
	struct rtl_phyregs port5;

	rtl_phy_save(dev, 5, &port5);

	/* disable rx/tx from PHYs */
	for (i = 0; i < RTL8306_NUM_PORTS - 1; i++) {
		rtl_port_set_enable(dev, i, 0);
	}

	/* save trunking status */
	trunk_en = rtl_get(dev, RTL_REG_EN_TRUNK);
	trunk_psel = rtl_get(dev, RTL_REG_TRUNK_PORTSEL);

	/* trunk port 3 and 4
	 * XXX: Big WTF, but RealTek seems to do it */
	rtl_set(dev, RTL_REG_EN_TRUNK, 1);
	rtl_set(dev, RTL_REG_TRUNK_PORTSEL, 1);

	/* execute the software reset */
	rtl_set(dev, RTL_REG_RESET, 1);

	/* wait for the reset to complete,
	 * but don't wait for too long */
	for (i = 0; i < 10; i++) {
		if (rtl_get(dev, RTL_REG_RESET) == 0)
			break;

		msleep(1);
	}

	/* enable rx/tx from PHYs */
	for (i = 0; i < RTL8306_NUM_PORTS - 1; i++) {
		rtl_port_set_enable(dev, i, 1);
	}

	/* restore trunking settings */
	rtl_set(dev, RTL_REG_EN_TRUNK, trunk_en);
	rtl_set(dev, RTL_REG_TRUNK_PORTSEL, trunk_psel);
	rtl_phy_restore(dev, 5, &port5);

	rtl_set(dev, RTL_REG_CPU_LINKUP, 1);

	return 0;
}

static void
rtl_hw_init(struct switch_dev *dev)
{
	struct rtl_priv *priv = to_rtl(dev);
	int cpu_mask = 1 << dev->cpu_port;
	int i;

	rtl_set(dev, RTL_REG_VLAN_ENABLE, 0);
	rtl_set(dev, RTL_REG_VLAN_FILTER, 0);
	rtl_set(dev, RTL_REG_EN_TRUNK, 0);
	rtl_set(dev, RTL_REG_TRUNK_PORTSEL, 0);

	/* initialize cpu port settings */
	if (priv->do_cpu) {
		rtl_set(dev, RTL_REG_CPUPORT, dev->cpu_port);
		rtl_set(dev, RTL_REG_EN_CPUPORT, 1);
	} else {
		rtl_set(dev, RTL_REG_CPUPORT, 7);
		rtl_set(dev, RTL_REG_EN_CPUPORT, 0);
	}
	rtl_set(dev, RTL_REG_EN_TAG_OUT, 0);
	rtl_set(dev, RTL_REG_EN_TAG_IN, 0);
	rtl_set(dev, RTL_REG_EN_TAG_CLR, 0);

	/* reset all vlans */
	for (i = 0; i < RTL8306_NUM_VLANS; i++) {
		rtl_set(dev, RTL_VLAN_REG(i, VID), i);
		rtl_set(dev, RTL_VLAN_REG(i, PORTMASK), 0);
	}

	/* default to port isolation */
	for (i = 0; i < RTL8306_NUM_PORTS; i++) {
		unsigned long mask;

		if ((1 << i) == cpu_mask)
			mask = ((1 << RTL8306_NUM_PORTS) - 1) & ~cpu_mask; /* all bits set */
		else
			mask = cpu_mask | (1 << i);

		rtl_set(dev, RTL_VLAN_REG(i, PORTMASK), mask);
		rtl_set(dev, RTL_PORT_REG(i, PVID), i);
		rtl_set(dev, RTL_PORT_REG(i, NULL_VID_REPLACE), 1);
		rtl_set(dev, RTL_PORT_REG(i, VID_INSERT), 1);
		rtl_set(dev, RTL_PORT_REG(i, TAG_INSERT), 3);
	}
	rtl_hw_apply(dev);
}

#ifdef DEBUG
static int
rtl_set_use_cpuport(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtl_priv *priv = to_rtl(dev);
	priv->do_cpu = val->value.i;
	rtl_hw_init(dev);
	return 0;
}

static int
rtl_get_use_cpuport(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtl_priv *priv = to_rtl(dev);
	val->value.i = priv->do_cpu;
	return 0;
}

static int
rtl_set_cpuport(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	dev->cpu_port = val->value.i;
	rtl_hw_init(dev);
	return 0;
}

static int
rtl_get_cpuport(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	val->value.i = dev->cpu_port;
	return 0;
}
#endif

static int
rtl_reset(struct switch_dev *dev)
{
	rtl_hw_init(dev);
	return 0;
}

static int
rtl_attr_set_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	int idx = attr->id + (val->port_vlan * attr->ofs);
	struct rtl_phyregs port;

	if (attr->id >= ARRAY_SIZE(rtl_regs))
		return -EINVAL;

	if ((attr->max > 0) && (val->value.i > attr->max))
		return -EINVAL;

	/* access to phy register 22 on port 4/5
	 * needs phy status save/restore */
	if ((val->port_vlan > 3) &&
		(rtl_regs[idx].reg == 22) &&
		(rtl_regs[idx].page == 0)) {

		rtl_phy_save(dev, val->port_vlan, &port);
		rtl_set(dev, idx, val->value.i);
		rtl_phy_restore(dev, val->port_vlan, &port);
	} else {
		rtl_set(dev, idx, val->value.i);
	}

	return 0;
}

static int
rtl_attr_get_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	int idx = attr->id + (val->port_vlan * attr->ofs);

	if (idx >= ARRAY_SIZE(rtl_regs))
		return -EINVAL;

	val->value.i = rtl_get(dev, idx);
	return 0;
}

static int
rtl_attr_set_port_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	if (val->port_vlan >= RTL8306_NUM_PORTS)
		return -EINVAL;

	return rtl_attr_set_int(dev, attr, val);
}

static int
rtl_attr_get_port_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	if (val->port_vlan >= RTL8306_NUM_PORTS)
		return -EINVAL;
	return rtl_attr_get_int(dev, attr, val);
}

static int 
rtl_get_port_link(struct switch_dev *dev, int port, struct switch_port_link *link)
{
	if (port >= RTL8306_NUM_PORTS)
		return -EINVAL;

	/* in case the link changes from down to up, the register is only updated on read */
	link->link = rtl_get(dev, RTL_PORT_REG(port, LINK));
	if (!link->link)
		link->link = rtl_get(dev, RTL_PORT_REG(port, LINK));

	if (!link->link)
		return 0;

	link->duplex = rtl_get(dev, RTL_PORT_REG(port, DUPLEX));
	link->aneg = rtl_get(dev, RTL_PORT_REG(port, NWAY));

	if (rtl_get(dev, RTL_PORT_REG(port, SPEED)))
		link->speed = SWITCH_PORT_SPEED_100;
	else
		link->speed = SWITCH_PORT_SPEED_10;

	return 0;
}

static int
rtl_attr_set_vlan_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	return rtl_attr_set_int(dev, attr, val);
}

static int
rtl_attr_get_vlan_int(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	return rtl_attr_get_int(dev, attr, val);
}

static int
rtl_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	unsigned int i, mask;

	mask = rtl_get(dev, RTL_VLAN_REG(val->port_vlan, PORTMASK));
	for (i = 0; i < RTL8306_NUM_PORTS; i++) {
		struct switch_port *port;

		if (!(mask & (1 << i)))
			continue;

		port = &val->value.ports[val->len];
		port->id = i;
		if (rtl_get(dev, RTL_PORT_REG(i, TAG_INSERT)) == 2 || i == dev->cpu_port)
			port->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
		val->len++;
	}

	return 0;
}

static int
rtl_set_vlan(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtl_priv *priv = to_rtl(dev);
	struct rtl_phyregs port;
	int en = val->value.i;
	int i;

	rtl_set(dev, RTL_REG_EN_TAG_OUT, en && priv->do_cpu);
	rtl_set(dev, RTL_REG_EN_TAG_IN, en && priv->do_cpu);
	rtl_set(dev, RTL_REG_EN_TAG_CLR, en && priv->do_cpu);
	rtl_set(dev, RTL_REG_VLAN_TAG_AWARE, en);
	if (en)
		rtl_set(dev, RTL_REG_VLAN_FILTER, en);

	for (i = 0; i < RTL8306_NUM_PORTS; i++) {
		if (i > 3)
			rtl_phy_save(dev, val->port_vlan, &port);
		rtl_set(dev, RTL_PORT_REG(i, NULL_VID_REPLACE), 1);
		rtl_set(dev, RTL_PORT_REG(i, VID_INSERT), (en ? (i == dev->cpu_port ? 0 : 1) : 1));
		rtl_set(dev, RTL_PORT_REG(i, TAG_INSERT), (en ? (i == dev->cpu_port ? 2 : 1) : 3));
		if (i > 3)
			rtl_phy_restore(dev, val->port_vlan, &port);
	}
	rtl_set(dev, RTL_REG_VLAN_ENABLE, en);

	return 0;
}

static int
rtl_get_vlan(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	val->value.i = rtl_get(dev, RTL_REG_VLAN_ENABLE);
	return 0;
}

static int
rtl_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	unsigned int mask = 0;
	unsigned int oldmask;
	int i;

	for(i = 0; i < val->len; i++)
	{
		struct switch_port *port = &val->value.ports[i];
		bool tagged = false;

		mask |= (1 << port->id);

		if (port->id == dev->cpu_port)
			continue;

		if ((i == dev->cpu_port) ||
			(port->flags & (1 << SWITCH_PORT_FLAG_TAGGED)))
			tagged = true;

		/* fix up PVIDs for added ports */
		if (!tagged)
			rtl_set(dev, RTL_PORT_REG(port->id, PVID), val->port_vlan);

		rtl_set(dev, RTL_PORT_REG(port->id, NON_PVID_DISCARD), (tagged ? 0 : 1));
		rtl_set(dev, RTL_PORT_REG(port->id, VID_INSERT), (tagged ? 0 : 1));
		rtl_set(dev, RTL_PORT_REG(port->id, TAG_INSERT), (tagged ? 2 : 1));
	}

	oldmask = rtl_get(dev, RTL_VLAN_REG(val->port_vlan, PORTMASK));
	rtl_set(dev, RTL_VLAN_REG(val->port_vlan, PORTMASK), mask);

	/* fix up PVIDs for removed ports, default to last vlan */
	oldmask &= ~mask;
	for (i = 0; i < RTL8306_NUM_PORTS; i++) {
		if (!(oldmask & (1 << i)))
			continue;

		if (i == dev->cpu_port)
			continue;

		if (rtl_get(dev, RTL_PORT_REG(i, PVID)) == val->port_vlan)
			rtl_set(dev, RTL_PORT_REG(i, PVID), dev->vlans - 1);
	}

	return 0;
}

static struct switch_attr rtl_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.max = 1,
		.set = rtl_set_vlan,
		.get = rtl_get_vlan,
	},
	{
		RTL_GLOBAL_REGATTR(EN_TRUNK),
		.name = "trunk",
		.description = "Enable port trunking",
		.max = 1,
	},
	{
		RTL_GLOBAL_REGATTR(TRUNK_PORTSEL),
		.name = "trunk_sel",
		.description = "Select ports for trunking (0: 0,1 - 1: 3,4)",
		.max = 1,
	},
#ifdef DEBUG
	{
		RTL_GLOBAL_REGATTR(VLAN_FILTER),
		.name = "vlan_filter",
		.description = "Filter incoming packets for allowed VLANS",
		.max = 1,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "cpuport",
		.description = "CPU Port",
		.set = rtl_set_cpuport,
		.get = rtl_get_cpuport,
		.max = RTL8306_NUM_PORTS,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "use_cpuport",
		.description = "CPU Port handling flag",
		.set = rtl_set_use_cpuport,
		.get = rtl_get_use_cpuport,
		.max = RTL8306_NUM_PORTS,
	},
	{
		RTL_GLOBAL_REGATTR(TRAP_CPU),
		.name = "trap_cpu",
		.description = "VLAN trap to CPU",
		.max = 1,
	},
	{
		RTL_GLOBAL_REGATTR(VLAN_TAG_AWARE),
		.name = "vlan_tag_aware",
		.description = "Enable VLAN tag awareness",
		.max = 1,
	},
	{
		RTL_GLOBAL_REGATTR(VLAN_TAG_ONLY),
		.name = "tag_only",
		.description = "Only accept tagged packets",
		.max = 1,
	},
#endif
};
static struct switch_attr rtl_port[] = {
	{
		RTL_PORT_REGATTR(PVID),
		.name = "pvid",
		.description = "Port VLAN ID",
		.max = RTL8306_NUM_VLANS - 1,
	},
#ifdef DEBUG
	{
		RTL_PORT_REGATTR(NULL_VID_REPLACE),
		.name = "null_vid",
		.description = "NULL VID gets replaced by port default vid",
		.max = 1,
	},
	{
		RTL_PORT_REGATTR(NON_PVID_DISCARD),
		.name = "non_pvid_discard",
		.description = "discard packets with VID != PVID",
		.max = 1,
	},
	{
		RTL_PORT_REGATTR(VID_INSERT),
		.name = "vid_insert_remove",
		.description = "how should the switch insert and remove vids ?",
		.max = 3,
	},
	{
		RTL_PORT_REGATTR(TAG_INSERT),
		.name = "tag_insert",
		.description = "tag insertion handling",
		.max = 3,
	},
#endif
};

static struct switch_attr rtl_vlan[] = {
	{
		RTL_VLAN_REGATTR(VID),
		.name = "vid",
		.description = "VLAN ID (1-4095)",
		.max = 4095,
	},
};

static const struct switch_dev_ops rtl8306_ops = {
	.attr_global = {
		.attr = rtl_globals,
		.n_attr = ARRAY_SIZE(rtl_globals),
	},
	.attr_port = {
		.attr = rtl_port,
		.n_attr = ARRAY_SIZE(rtl_port),
	},
	.attr_vlan = {
		.attr = rtl_vlan,
		.n_attr = ARRAY_SIZE(rtl_vlan),
	},

	.get_vlan_ports = rtl_get_ports,
	.set_vlan_ports = rtl_set_ports,
	.apply_config = rtl_hw_apply,
	.reset_switch = rtl_reset,
	.get_port_link = rtl_get_port_link,
};

static int
rtl8306_config_init(struct phy_device *pdev)
{
	struct net_device *netdev = pdev->attached_dev;
	struct rtl_priv *priv = pdev->priv;
	struct switch_dev *dev = &priv->dev;
	struct switch_val val;
	unsigned int chipid, chipver, chiptype;
	int err;

	/* Only init the switch for the primary PHY */
	if (pdev->mdio.addr != 0)
		return 0;

	val.value.i = 1;
	priv->dev.cpu_port = RTL8306_PORT_CPU;
	priv->dev.ports = RTL8306_NUM_PORTS;
	priv->dev.vlans = RTL8306_NUM_VLANS;
	priv->dev.ops = &rtl8306_ops;
	priv->do_cpu = 0;
	priv->page = -1;
	priv->bus = pdev->mdio.bus;

	chipid = rtl_get(dev, RTL_REG_CHIPID);
	chipver = rtl_get(dev, RTL_REG_CHIPVER);
	chiptype = rtl_get(dev, RTL_REG_CHIPTYPE);
	switch(chiptype) {
	case 0:
	case 2:
		strncpy(priv->hwname, RTL_NAME_S, sizeof(priv->hwname));
		priv->type = RTL_TYPE_S;
		break;
	case 1:
		strncpy(priv->hwname, RTL_NAME_SD, sizeof(priv->hwname));
		priv->type = RTL_TYPE_SD;
		break;
	case 3:
		strncpy(priv->hwname, RTL_NAME_SDM, sizeof(priv->hwname));
		priv->type = RTL_TYPE_SDM;
		break;
	default:
		strncpy(priv->hwname, RTL_NAME_UNKNOWN, sizeof(priv->hwname));
		break;
	}

	dev->name = priv->hwname;
	rtl_hw_init(dev);

	printk(KERN_INFO "Registering %s switch with Chip ID: 0x%04x, version: 0x%04x\n", priv->hwname, chipid, chipver);

	err = register_switch(dev, netdev);
	if (err < 0) {
		kfree(priv);
		return err;
	}

	return 0;
}


static int
rtl8306_fixup(struct phy_device *pdev)
{
	struct rtl_priv priv;
	u16 chipid;

	/* Attach to primary LAN port and WAN port */
	if (pdev->mdio.addr != 0 && pdev->mdio.addr != 4)
		return 0;

	memset(&priv, 0, sizeof(priv));
	priv.fixup = true;
	priv.page = -1;
	priv.bus = pdev->mdio.bus;
	chipid = rtl_get(&priv.dev, RTL_REG_CHIPID);
	if (chipid == 0x5988)
		pdev->phy_id = RTL8306_MAGIC;

	return 0;
}

static int
rtl8306_probe(struct phy_device *pdev)
{
	struct rtl_priv *priv;

	list_for_each_entry(priv, &phydevs, list) {
		/*
		 * share one rtl_priv instance between virtual phy
		 * devices on the same bus
		 */
		if (priv->bus == pdev->mdio.bus)
			goto found;
	}
	priv = kzalloc(sizeof(struct rtl_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->bus = pdev->mdio.bus;

found:
	pdev->priv = priv;
	return 0;
}

static void
rtl8306_remove(struct phy_device *pdev)
{
	struct rtl_priv *priv = pdev->priv;
	unregister_switch(&priv->dev);
	kfree(priv);
}

static int
rtl8306_config_aneg(struct phy_device *pdev)
{
	struct rtl_priv *priv = pdev->priv;

	/* Only for WAN */
	if (pdev->mdio.addr == 0)
		return 0;

	/* Restart autonegotiation */
	rtl_set(&priv->dev, RTL_PORT_REG(4, NWAY), 1);
	rtl_set(&priv->dev, RTL_PORT_REG(4, NRESTART), 1);

	return 0;
}

static int
rtl8306_read_status(struct phy_device *pdev)
{
	struct rtl_priv *priv = pdev->priv;
	struct switch_dev *dev = &priv->dev;

	if (pdev->mdio.addr == 4) {
		/* WAN */
		pdev->speed = rtl_get(dev, RTL_PORT_REG(4, SPEED)) ? SPEED_100 : SPEED_10;
		pdev->duplex = rtl_get(dev, RTL_PORT_REG(4, DUPLEX)) ? DUPLEX_FULL : DUPLEX_HALF;
		pdev->link = !!rtl_get(dev, RTL_PORT_REG(4, LINK));
	} else {
		/* LAN */
		pdev->speed = SPEED_100;
		pdev->duplex = DUPLEX_FULL;
		pdev->link = 1;
	}

	/*
	 * Bypass generic PHY status read,
	 * it doesn't work with this switch
	 */
	if (pdev->link) {
		pdev->state = PHY_RUNNING;
		netif_carrier_on(pdev->attached_dev);
		pdev->adjust_link(pdev->attached_dev);
	} else {
		pdev->state = PHY_NOLINK;
		netif_carrier_off(pdev->attached_dev);
		pdev->adjust_link(pdev->attached_dev);
	}

	return 0;
}


static struct phy_driver rtl8306_driver = {
	.name		= "Realtek RTL8306S",
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0))
	.flags		= PHY_HAS_MAGICANEG,
#endif
	.phy_id		= RTL8306_MAGIC,
	.phy_id_mask	= 0xffffffff,
	.features	= PHY_BASIC_FEATURES,
	.probe		= &rtl8306_probe,
	.remove		= &rtl8306_remove,
	.config_init	= &rtl8306_config_init,
	.config_aneg	= &rtl8306_config_aneg,
	.read_status	= &rtl8306_read_status,
};


static int __init
rtl_init(void)
{
	phy_register_fixup_for_id(PHY_ANY_ID, rtl8306_fixup);
	return phy_driver_register(&rtl8306_driver, THIS_MODULE);
}

static void __exit
rtl_exit(void)
{
	phy_driver_unregister(&rtl8306_driver);
}

module_init(rtl_init);
module_exit(rtl_exit);
MODULE_LICENSE("GPL");

