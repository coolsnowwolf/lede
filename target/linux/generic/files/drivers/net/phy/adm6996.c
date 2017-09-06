/*
 * ADM6996 switch driver
 *
 * swconfig interface based on ar8216.c
 *
 * Copyright (c) 2008 Felix Fietkau <nbd@nbd.name>
 * VLAN support Copyright (c) 2010, 2011 Peter Lebbing <peter@digitalbrains.com>
 * Copyright (c) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 * Copyright (c) 2014 Matti Laakso <malaakso@elisanet.fi>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

/*#define DEBUG 1*/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/platform_device.h>
#include <linux/platform_data/adm6996-gpio.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/switch.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include "adm6996.h"

MODULE_DESCRIPTION("Infineon ADM6996 Switch");
MODULE_AUTHOR("Felix Fietkau, Peter Lebbing <peter@digitalbrains.com>");
MODULE_LICENSE("GPL");

static const char * const adm6996_model_name[] =
{
	NULL,
	"ADM6996FC",
	"ADM6996M",
	"ADM6996L"
};

struct adm6996_mib_desc {
	unsigned int offset;
	const char *name;
};

struct adm6996_priv {
	struct switch_dev dev;
	void *priv;

	u8 eecs;
	u8 eesk;
	u8 eedi;

	enum adm6996_model model;

	bool enable_vlan;
	bool vlan_enabled;	/* Current hardware state */

#ifdef DEBUG
	u16 addr;		/* Debugging: register address to operate on */
#endif

	u16 pvid[ADM_NUM_PORTS];	/* Primary VLAN ID */
	u8 tagged_ports;

	u16 vlan_id[ADM_NUM_VLANS];
	u8 vlan_table[ADM_NUM_VLANS];	/* bitmap, 1 = port is member */
	u8 vlan_tagged[ADM_NUM_VLANS];	/* bitmap, 1 = tagged member */
	
	struct mutex mib_lock;
	char buf[2048];

	struct mutex reg_mutex;

	/* use abstraction for regops, we want to add gpio support in the future */
	u16 (*read)(struct adm6996_priv *priv, enum admreg reg);
	void (*write)(struct adm6996_priv *priv, enum admreg reg, u16 val);
};

#define to_adm(_dev) container_of(_dev, struct adm6996_priv, dev)
#define phy_to_adm(_phy) ((struct adm6996_priv *) (_phy)->priv)

#define MIB_DESC(_o, _n)	\
	{			\
		.offset = (_o),	\
		.name = (_n),	\
	}

static const struct adm6996_mib_desc adm6996_mibs[] = {
	MIB_DESC(ADM_CL0, "RxPacket"),
	MIB_DESC(ADM_CL6, "RxByte"),
	MIB_DESC(ADM_CL12, "TxPacket"),
	MIB_DESC(ADM_CL18, "TxByte"),
	MIB_DESC(ADM_CL24, "Collision"),
	MIB_DESC(ADM_CL30, "Error"),
};

#define ADM6996_MIB_RXB_ID	1
#define ADM6996_MIB_TXB_ID	3

static inline u16
r16(struct adm6996_priv *priv, enum admreg reg)
{
	return priv->read(priv, reg);
}

static inline void
w16(struct adm6996_priv *priv, enum admreg reg, u16 val)
{
	priv->write(priv, reg, val);
}

/* Minimum timing constants */
#define EECK_EDGE_TIME  3   /* 3us - max(adm 2.5us, 93c 1us) */
#define EEDI_SETUP_TIME 1   /* 1us - max(adm 10ns, 93c 400ns) */
#define EECS_SETUP_TIME 1   /* 1us - max(adm no, 93c 200ns) */

static void adm6996_gpio_write(struct adm6996_priv *priv, int cs, char *buf, unsigned int bits)
{
	int i, len = (bits + 7) / 8;
	u8 mask;

	gpio_set_value(priv->eecs, cs);
	udelay(EECK_EDGE_TIME);

	/* Byte assemble from MSB to LSB */
	for (i = 0; i < len; i++) {
		/* Bit bang from MSB to LSB */
		for (mask = 0x80; mask && bits > 0; mask >>= 1, bits --) {
			/* Clock low */
			gpio_set_value(priv->eesk, 0);
			udelay(EECK_EDGE_TIME);

			/* Output on rising edge */
			gpio_set_value(priv->eedi, (mask & buf[i]));
			udelay(EEDI_SETUP_TIME);

			/* Clock high */
			gpio_set_value(priv->eesk, 1);
			udelay(EECK_EDGE_TIME);
		}
	}

	/* Clock low */
	gpio_set_value(priv->eesk, 0);
	udelay(EECK_EDGE_TIME);

	if (cs)
		gpio_set_value(priv->eecs, 0);
}

static void adm6996_gpio_read(struct adm6996_priv *priv, int cs, char *buf, unsigned int bits)
{
	int i, len = (bits + 7) / 8;
	u8 mask;

	gpio_set_value(priv->eecs, cs);
	udelay(EECK_EDGE_TIME);

	/* Byte assemble from MSB to LSB */
	for (i = 0; i < len; i++) {
		u8 byte;

		/* Bit bang from MSB to LSB */
		for (mask = 0x80, byte = 0; mask && bits > 0; mask >>= 1, bits --) {
			u8 gp;

			/* Clock low */
			gpio_set_value(priv->eesk, 0);
			udelay(EECK_EDGE_TIME);

			/* Input on rising edge */
			gp = gpio_get_value(priv->eedi);
			if (gp)
				byte |= mask;

			/* Clock high */
			gpio_set_value(priv->eesk, 1);
			udelay(EECK_EDGE_TIME);
		}

		*buf++ = byte;
	}

	/* Clock low */
	gpio_set_value(priv->eesk, 0);
	udelay(EECK_EDGE_TIME);

	if (cs)
		gpio_set_value(priv->eecs, 0);
}

/* Advance clock(s) */
static void adm6996_gpio_adclk(struct adm6996_priv *priv, int clocks)
{
	int i;
	for (i = 0; i < clocks; i++) {
		/* Clock high */
		gpio_set_value(priv->eesk, 1);
		udelay(EECK_EDGE_TIME);

		/* Clock low */
		gpio_set_value(priv->eesk, 0);
		udelay(EECK_EDGE_TIME);
	}
}

static u16
adm6996_read_gpio_reg(struct adm6996_priv *priv, enum admreg reg)
{
	/* cmd: 01 10 T DD R RRRRRR */
	u8 bits[6] = {
		0xFF, 0xFF, 0xFF, 0xFF,
		(0x06 << 4) | ((0 & 0x01) << 3 | (reg&64)>>6),
		((reg&63)<<2)
	};

	u8 rbits[4];

	/* Enable GPIO outputs with all pins to 0 */
	gpio_direction_output(priv->eecs, 0);
	gpio_direction_output(priv->eesk, 0);
	gpio_direction_output(priv->eedi, 0);

	adm6996_gpio_write(priv, 0, bits, 46);
	gpio_direction_input(priv->eedi);
	adm6996_gpio_adclk(priv, 2);
	adm6996_gpio_read(priv, 0, rbits, 32);

	/* Extra clock(s) required per datasheet */
	adm6996_gpio_adclk(priv, 2);

	/* Disable GPIO outputs */
	gpio_direction_input(priv->eecs);
	gpio_direction_input(priv->eesk);

	 /* EEPROM has 16-bit registers, but pumps out two registers in one request */
	return (reg & 0x01 ?  (rbits[0]<<8) | rbits[1] : (rbits[2]<<8) | (rbits[3]));
}

/* Write chip configuration register */
/* Follow 93c66 timing and chip's min EEPROM timing requirement */
static void
adm6996_write_gpio_reg(struct adm6996_priv *priv, enum admreg reg, u16 val)
{
	/* cmd(27bits): sb(1) + opc(01) + addr(bbbbbbbb) + data(bbbbbbbbbbbbbbbb) */
	u8 bits[4] = {
		(0x05 << 5) | (reg >> 3),
		(reg << 5) | (u8)(val >> 11),
		(u8)(val >> 3),
		(u8)(val << 5)
	};

	/* Enable GPIO outputs with all pins to 0 */
	gpio_direction_output(priv->eecs, 0);
	gpio_direction_output(priv->eesk, 0);
	gpio_direction_output(priv->eedi, 0);

	/* Write cmd. Total 27 bits */
	adm6996_gpio_write(priv, 1, bits, 27);

	/* Extra clock(s) required per datasheet */
	adm6996_gpio_adclk(priv, 2);

	/* Disable GPIO outputs */
	gpio_direction_input(priv->eecs);
	gpio_direction_input(priv->eesk);
	gpio_direction_input(priv->eedi);
}

static u16
adm6996_read_mii_reg(struct adm6996_priv *priv, enum admreg reg)
{
	struct phy_device *phydev = priv->priv;
	struct mii_bus *bus = phydev->mdio.bus;

	return bus->read(bus, PHYADDR(reg));
}

static void
adm6996_write_mii_reg(struct adm6996_priv *priv, enum admreg reg, u16 val)
{
	struct phy_device *phydev = priv->priv;
	struct mii_bus *bus = phydev->mdio.bus;

	bus->write(bus, PHYADDR(reg), val);
}

static int
adm6996_set_enable_vlan(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	if (val->value.i > 1)
		return -EINVAL;

	priv->enable_vlan = val->value.i;

	return 0;
};

static int
adm6996_get_enable_vlan(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	val->value.i = priv->enable_vlan;

	return 0;
};

#ifdef DEBUG

static int
adm6996_set_addr(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	if (val->value.i > 1023)
		return -EINVAL;

	priv->addr = val->value.i;

	return 0;
};

static int
adm6996_get_addr(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	val->value.i = priv->addr;

	return 0;
};

static int
adm6996_set_data(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	if (val->value.i > 65535)
		return -EINVAL;

	w16(priv, priv->addr, val->value.i);

	return 0;
};

static int
adm6996_get_data(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	val->value.i = r16(priv, priv->addr);

	return 0;
};

#endif /* def DEBUG */

static int
adm6996_set_pvid(struct switch_dev *dev, int port, int vlan)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("set_pvid port %d vlan %d\n", port, vlan);

	if (vlan > ADM_VLAN_MAX_ID)
		return -EINVAL;

	priv->pvid[port] = vlan;

	return 0;
}

static int
adm6996_get_pvid(struct switch_dev *dev, int port, int *vlan)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("get_pvid port %d\n", port);
	*vlan = priv->pvid[port];

	return 0;
}

static int
adm6996_set_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("set_vid port %d vid %d\n", val->port_vlan, val->value.i);

	if (val->value.i > ADM_VLAN_MAX_ID)
		return -EINVAL;

	priv->vlan_id[val->port_vlan] = val->value.i;

	return 0;
};

static int
adm6996_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("get_vid port %d\n", val->port_vlan);

	val->value.i = priv->vlan_id[val->port_vlan];

	return 0;
};

static int
adm6996_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);
	u8 ports = priv->vlan_table[val->port_vlan];
	u8 tagged = priv->vlan_tagged[val->port_vlan];
	int i;

	pr_devel("get_ports port_vlan %d\n", val->port_vlan);

	val->len = 0;

	for (i = 0; i < ADM_NUM_PORTS; i++) {
		struct switch_port *p;

		if (!(ports & (1 << i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (tagged & (1 << i))
			p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
		else
			p->flags = 0;
	}

	return 0;
};

static int
adm6996_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);
	u8 *ports = &priv->vlan_table[val->port_vlan];
	u8 *tagged = &priv->vlan_tagged[val->port_vlan];
	int i;

	pr_devel("set_ports port_vlan %d ports", val->port_vlan);

	*ports = 0;
	*tagged = 0;

	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

#ifdef DEBUG
		pr_cont(" %d%s", p->id,
		       ((p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) ? "T" :
			""));
#endif

		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
			*tagged |= (1 << p->id);
			priv->tagged_ports |= (1 << p->id);
		}

		*ports |= (1 << p->id);
	}

#ifdef DEBUG
	pr_cont("\n");
#endif

	return 0;
};

/*
 * Precondition: reg_mutex must be held
 */
static void
adm6996_enable_vlan(struct adm6996_priv *priv)
{
	u16 reg;

	reg = r16(priv, ADM_OTBE_P2_PVID);
	reg &= ~(ADM_OTBE_MASK);
	w16(priv, ADM_OTBE_P2_PVID, reg);
	reg = r16(priv, ADM_IFNTE);
	reg &= ~(ADM_IFNTE_MASK);
	w16(priv, ADM_IFNTE, reg);
	reg = r16(priv, ADM_VID_CHECK);
	reg |= ADM_VID_CHECK_MASK;
	w16(priv, ADM_VID_CHECK, reg);
	reg = r16(priv, ADM_SYSC0);
	reg |= ADM_NTTE;
	reg &= ~(ADM_RVID1);
	w16(priv, ADM_SYSC0, reg);
	reg = r16(priv, ADM_SYSC3);
	reg |= ADM_TBV;
	w16(priv, ADM_SYSC3, reg);
}

static void
adm6996_enable_vlan_6996l(struct adm6996_priv *priv)
{
	u16 reg;

	reg = r16(priv, ADM_SYSC3);
	reg |= ADM_TBV;
	reg |= ADM_MAC_CLONE;
	w16(priv, ADM_SYSC3, reg);
}

/*
 * Disable VLANs
 *
 * Sets VLAN mapping for port-based VLAN with all ports connected to
 * eachother (this is also the power-on default).
 *
 * Precondition: reg_mutex must be held
 */
static void
adm6996_disable_vlan(struct adm6996_priv *priv)
{
	u16 reg;
	int i;

	for (i = 0; i < ADM_NUM_VLANS; i++) {
		reg = ADM_VLAN_FILT_MEMBER_MASK;
		w16(priv, ADM_VLAN_FILT_L(i), reg);
		reg = ADM_VLAN_FILT_VALID | ADM_VLAN_FILT_VID(1);
		w16(priv, ADM_VLAN_FILT_H(i), reg);
	}

	reg = r16(priv, ADM_OTBE_P2_PVID);
	reg |= ADM_OTBE_MASK;
	w16(priv, ADM_OTBE_P2_PVID, reg);
	reg = r16(priv, ADM_IFNTE);
	reg |= ADM_IFNTE_MASK;
	w16(priv, ADM_IFNTE, reg);
	reg = r16(priv, ADM_VID_CHECK);
	reg &= ~(ADM_VID_CHECK_MASK);
	w16(priv, ADM_VID_CHECK, reg);
	reg = r16(priv, ADM_SYSC0);
	reg &= ~(ADM_NTTE);
	reg |= ADM_RVID1;
	w16(priv, ADM_SYSC0, reg);
	reg = r16(priv, ADM_SYSC3);
	reg &= ~(ADM_TBV);
	w16(priv, ADM_SYSC3, reg);
}

/*
 * Disable VLANs
 *
 * Sets VLAN mapping for port-based VLAN with all ports connected to
 * eachother (this is also the power-on default).
 *
 * Precondition: reg_mutex must be held
 */
static void
adm6996_disable_vlan_6996l(struct adm6996_priv *priv)
{
	u16 reg;
	int i;

	for (i = 0; i < ADM_NUM_VLANS; i++) {
		w16(priv, ADM_VLAN_MAP(i), 0);
	}

	reg = r16(priv, ADM_SYSC3);
	reg &= ~(ADM_TBV);
	reg &= ~(ADM_MAC_CLONE);
	w16(priv, ADM_SYSC3, reg);
}

/*
 * Precondition: reg_mutex must be held
 */
static void
adm6996_apply_port_pvids(struct adm6996_priv *priv)
{
	u16 reg;
	int i;

	for (i = 0; i < ADM_NUM_PORTS; i++) {
		reg = r16(priv, adm_portcfg[i]);
		reg &= ~(ADM_PORTCFG_PVID_MASK);
		reg |= ADM_PORTCFG_PVID(priv->pvid[i]);
		if (priv->model == ADM6996L) {
			if (priv->tagged_ports & (1 << i))
				reg |= (1 << 4);
			else
				reg &= ~(1 << 4);
		}
		w16(priv, adm_portcfg[i], reg);
	}

	w16(priv, ADM_P0_PVID, ADM_P0_PVID_VAL(priv->pvid[0]));
	w16(priv, ADM_P1_PVID, ADM_P1_PVID_VAL(priv->pvid[1]));
	reg = r16(priv, ADM_OTBE_P2_PVID);
	reg &= ~(ADM_P2_PVID_MASK);
	reg |= ADM_P2_PVID_VAL(priv->pvid[2]);
	w16(priv, ADM_OTBE_P2_PVID, reg);
	reg = ADM_P3_PVID_VAL(priv->pvid[3]);
	reg |= ADM_P4_PVID_VAL(priv->pvid[4]);
	w16(priv, ADM_P3_P4_PVID, reg);
	reg = r16(priv, ADM_P5_PVID);
	reg &= ~(ADM_P2_PVID_MASK);
	reg |= ADM_P5_PVID_VAL(priv->pvid[5]);
	w16(priv, ADM_P5_PVID, reg);
}

/*
 * Precondition: reg_mutex must be held
 */
static void
adm6996_apply_vlan_filters(struct adm6996_priv *priv)
{
	u8 ports, tagged;
	u16 vid, reg;
	int i;

	for (i = 0; i < ADM_NUM_VLANS; i++) {
		vid = priv->vlan_id[i];
		ports = priv->vlan_table[i];
		tagged = priv->vlan_tagged[i];

		if (ports == 0) {
			/* Disable VLAN entry */
			w16(priv, ADM_VLAN_FILT_H(i), 0);
			w16(priv, ADM_VLAN_FILT_L(i), 0);
			continue;
		}

		reg = ADM_VLAN_FILT_MEMBER(ports);
		reg |= ADM_VLAN_FILT_TAGGED(tagged);
		w16(priv, ADM_VLAN_FILT_L(i), reg);
		reg = ADM_VLAN_FILT_VALID | ADM_VLAN_FILT_VID(vid);
		w16(priv, ADM_VLAN_FILT_H(i), reg);
	}
}

static void
adm6996_apply_vlan_filters_6996l(struct adm6996_priv *priv)
{
	u8 ports;
	u16 reg;
	int i;

	for (i = 0; i < ADM_NUM_VLANS; i++) {
		ports = priv->vlan_table[i];

		if (ports == 0) {
			/* Disable VLAN entry */
			w16(priv, ADM_VLAN_MAP(i), 0);
			continue;
		} else {
			reg = ADM_VLAN_FILT(ports);
			w16(priv, ADM_VLAN_MAP(i), reg);
		}
	}
}

static int
adm6996_hw_apply(struct switch_dev *dev)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("hw_apply\n");

	mutex_lock(&priv->reg_mutex);

	if (!priv->enable_vlan) {
		if (priv->vlan_enabled) {
			if (priv->model == ADM6996L)
				adm6996_disable_vlan_6996l(priv);
			else
				adm6996_disable_vlan(priv);
			priv->vlan_enabled = 0;
		}
		goto out;
	}

	if (!priv->vlan_enabled) {
		if (priv->model == ADM6996L)
			adm6996_enable_vlan_6996l(priv);
		else
			adm6996_enable_vlan(priv);
		priv->vlan_enabled = 1;
	}

	adm6996_apply_port_pvids(priv);
	if (priv->model == ADM6996L)
		adm6996_apply_vlan_filters_6996l(priv);
	else
		adm6996_apply_vlan_filters(priv);

out:
	mutex_unlock(&priv->reg_mutex);

	return 0;
}

/*
 * Reset the switch
 *
 * The ADM6996 can't do a software-initiated reset, so we just initialise the
 * registers we support in this driver.
 *
 * Precondition: reg_mutex must be held
 */
static void
adm6996_perform_reset (struct adm6996_priv *priv)
{
	int i;

	/* initialize port and vlan settings */
	for (i = 0; i < ADM_NUM_PORTS - 1; i++) {
		w16(priv, adm_portcfg[i], ADM_PORTCFG_INIT |
			ADM_PORTCFG_PVID(0));
	}
	w16(priv, adm_portcfg[5], ADM_PORTCFG_CPU);

	if (priv->model == ADM6996M || priv->model == ADM6996FC) {
		/* reset all PHY ports */
		for (i = 0; i < ADM_PHY_PORTS; i++) {
			w16(priv, ADM_PHY_PORT(i), ADM_PHYCFG_INIT);
		}
	}

	priv->enable_vlan = 0;
	priv->vlan_enabled = 0;

	for (i = 0; i < ADM_NUM_PORTS; i++) {
		priv->pvid[i] = 0;
	}

	for (i = 0; i < ADM_NUM_VLANS; i++) {
		priv->vlan_id[i] = i;
		priv->vlan_table[i] = 0;
		priv->vlan_tagged[i] = 0;
	}

	if (priv->model == ADM6996M) {
		/* Clear VLAN priority map so prio's are unused */
		w16 (priv, ADM_VLAN_PRIOMAP, 0);

		adm6996_disable_vlan(priv);
		adm6996_apply_port_pvids(priv);
	} else if (priv->model == ADM6996L) {
		/* Clear VLAN priority map so prio's are unused */
		w16 (priv, ADM_VLAN_PRIOMAP, 0);

		adm6996_disable_vlan_6996l(priv);
		adm6996_apply_port_pvids(priv);
	}
}

static int
adm6996_reset_switch(struct switch_dev *dev)
{
	struct adm6996_priv *priv = to_adm(dev);

	pr_devel("reset\n");

	mutex_lock(&priv->reg_mutex);
	adm6996_perform_reset (priv);
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int
adm6996_get_port_link(struct switch_dev *dev, int port,
		struct switch_port_link *link)
{
	struct adm6996_priv *priv = to_adm(dev);
	
	u16 reg = 0;
	
	if (port >= ADM_NUM_PORTS)
		return -EINVAL;
	
	switch (port) {
	case 0:
		reg = r16(priv, ADM_PS0);
		break;
	case 1:
		reg = r16(priv, ADM_PS0);
		reg = reg >> 8;
		break;
	case 2:
		reg = r16(priv, ADM_PS1);
		break;
	case 3:
		reg = r16(priv, ADM_PS1);
		reg = reg >> 8;
		break;
	case 4:
		reg = r16(priv, ADM_PS1);
		reg = reg >> 12;
		break;
	case 5:
		reg = r16(priv, ADM_PS2);
		/* Bits 0, 1, 3 and 4. */
		reg = (reg & 3) | ((reg & 24) >> 1);
		break;
	default:
		return -EINVAL;
	}
	
	link->link = reg & ADM_PS_LS;
	if (!link->link)
		return 0;
	link->aneg = true;
	link->duplex = reg & ADM_PS_DS;
	link->tx_flow = reg & ADM_PS_FCS;
	link->rx_flow = reg & ADM_PS_FCS;
	if (reg & ADM_PS_SS)
		link->speed = SWITCH_PORT_SPEED_100;
	else
		link->speed = SWITCH_PORT_SPEED_10;

	return 0;
}

static int
adm6996_sw_get_port_mib(struct switch_dev *dev,
		       const struct switch_attr *attr,
		       struct switch_val *val)
{
	struct adm6996_priv *priv = to_adm(dev);
	int port;
	char *buf = priv->buf;
	int i, len = 0;
	u32 reg = 0;

	port = val->port_vlan;
	if (port >= ADM_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->mib_lock);

	len += snprintf(buf + len, sizeof(priv->buf) - len,
			"Port %d MIB counters\n",
			port);

	for (i = 0; i < ARRAY_SIZE(adm6996_mibs); i++) {
		reg = r16(priv, adm6996_mibs[i].offset + ADM_OFFSET_PORT(port));
		reg += r16(priv, adm6996_mibs[i].offset + ADM_OFFSET_PORT(port) + 1) << 16;
		len += snprintf(buf + len, sizeof(priv->buf) - len,
				"%-12s: %u\n",
				adm6996_mibs[i].name,
				reg);
	}

	mutex_unlock(&priv->mib_lock);

	val->value.s = buf;
	val->len = len;

	return 0;
}

static int
adm6996_get_port_stats(struct switch_dev *dev, int port,
			struct switch_port_stats *stats)
{
	struct adm6996_priv *priv = to_adm(dev);
	int id;
	u32 reg = 0;

	if (port >= ADM_NUM_PORTS)
		return -EINVAL;

	mutex_lock(&priv->mib_lock);

	id = ADM6996_MIB_TXB_ID;
	reg = r16(priv, adm6996_mibs[id].offset + ADM_OFFSET_PORT(port));
	reg += r16(priv, adm6996_mibs[id].offset + ADM_OFFSET_PORT(port) + 1) << 16;
	stats->tx_bytes = reg;

	id = ADM6996_MIB_RXB_ID;
	reg = r16(priv, adm6996_mibs[id].offset + ADM_OFFSET_PORT(port));
	reg += r16(priv, adm6996_mibs[id].offset + ADM_OFFSET_PORT(port) + 1) << 16;
	stats->rx_bytes = reg;

	mutex_unlock(&priv->mib_lock);

	return 0;
}

static struct switch_attr adm6996_globals[] = {
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "enable_vlan",
	 .description = "Enable VLANs",
	 .set = adm6996_set_enable_vlan,
	 .get = adm6996_get_enable_vlan,
	},
#ifdef DEBUG
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "addr",
	 .description =
	 "Direct register access: set register address (0 - 1023)",
	 .set = adm6996_set_addr,
	 .get = adm6996_get_addr,
	 },
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "data",
	 .description =
	 "Direct register access: read/write to register (0 - 65535)",
	 .set = adm6996_set_data,
	 .get = adm6996_get_data,
	 },
#endif /* def DEBUG */
};

static struct switch_attr adm6996_port[] = {
	{
	 .type = SWITCH_TYPE_STRING,
	 .name = "mib",
	 .description = "Get port's MIB counters",
	 .set = NULL,
	 .get = adm6996_sw_get_port_mib,
	},
};

static struct switch_attr adm6996_vlan[] = {
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "vid",
	 .description = "VLAN ID",
	 .set = adm6996_set_vid,
	 .get = adm6996_get_vid,
	 },
};

static struct switch_dev_ops adm6996_ops = {
	.attr_global = {
			.attr = adm6996_globals,
			.n_attr = ARRAY_SIZE(adm6996_globals),
			},
	.attr_port = {
		      .attr = adm6996_port,
		      .n_attr = ARRAY_SIZE(adm6996_port),
		      },
	.attr_vlan = {
		      .attr = adm6996_vlan,
		      .n_attr = ARRAY_SIZE(adm6996_vlan),
		      },
	.get_port_pvid = adm6996_get_pvid,
	.set_port_pvid = adm6996_set_pvid,
	.get_vlan_ports = adm6996_get_ports,
	.set_vlan_ports = adm6996_set_ports,
	.apply_config = adm6996_hw_apply,
	.reset_switch = adm6996_reset_switch,
	.get_port_link = adm6996_get_port_link,
	.get_port_stats = adm6996_get_port_stats,
};

static int adm6996_switch_init(struct adm6996_priv *priv, const char *alias, struct net_device *netdev)
{
	struct switch_dev *swdev;
	u16 test, old;

	if (!priv->model) {
		/* Detect type of chip */
		old = r16(priv, ADM_VID_CHECK);
		test = old ^ (1 << 12);
		w16(priv, ADM_VID_CHECK, test);
		test ^= r16(priv, ADM_VID_CHECK);
		if (test & (1 << 12)) {
			/* 
			 * Bit 12 of this register is read-only. 
			 * This is the FC model. 
			 */
			priv->model = ADM6996FC;
		} else {
			/* Bit 12 is read-write. This is the M model. */
			priv->model = ADM6996M;
			w16(priv, ADM_VID_CHECK, old);
		}
	}

	swdev = &priv->dev;
	swdev->name = (adm6996_model_name[priv->model]);
	swdev->cpu_port = ADM_CPU_PORT;
	swdev->ports = ADM_NUM_PORTS;
	swdev->vlans = ADM_NUM_VLANS;
	swdev->ops = &adm6996_ops;
	swdev->alias = alias;

	/* The ADM6996L connected through GPIOs does not support any switch
	   status calls */
	if (priv->model == ADM6996L) {
		adm6996_ops.attr_port.n_attr = 0;
		adm6996_ops.get_port_link = NULL;
	}

	pr_info ("%s: %s model PHY found.\n", alias, swdev->name);

	mutex_lock(&priv->reg_mutex);
	adm6996_perform_reset (priv);
	mutex_unlock(&priv->reg_mutex);

	if (priv->model == ADM6996M || priv->model == ADM6996L) {
		return register_switch(swdev, netdev);
	}

	return -ENODEV;
}

static int adm6996_config_init(struct phy_device *pdev)
{
	struct adm6996_priv *priv;
	int ret;

	pdev->supported = ADVERTISED_100baseT_Full;
	pdev->advertising = ADVERTISED_100baseT_Full;

	if (pdev->mdio.addr != 0) {
		pr_info ("%s: PHY overlaps ADM6996, providing fixed PHY 0x%x.\n"
				, pdev->attached_dev->name, pdev->mdio.addr);
		return 0;
	}

	priv = devm_kzalloc(&pdev->mdio.dev, sizeof(struct adm6996_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->reg_mutex);
	mutex_init(&priv->mib_lock);
	priv->priv = pdev;
	priv->read = adm6996_read_mii_reg;
	priv->write = adm6996_write_mii_reg;

	ret = adm6996_switch_init(priv, pdev->attached_dev->name, pdev->attached_dev);
	if (ret < 0)
		return ret;

	pdev->priv = priv;

	return 0;
}

/*
 * Warning: phydev->priv is NULL if phydev->mdio.addr != 0
 */
static int adm6996_read_status(struct phy_device *phydev)
{
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->link = 1;

	phydev->state = PHY_RUNNING;
	netif_carrier_on(phydev->attached_dev);
	phydev->adjust_link(phydev->attached_dev);

	return 0;
}

/*
 * Warning: phydev->priv is NULL if phydev->mdio.addr != 0
 */
static int adm6996_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static int adm6996_fixup(struct phy_device *dev)
{
	struct mii_bus *bus = dev->mdio.bus;
	u16 reg;

	/* Our custom registers are at PHY addresses 0-10. Claim those. */
	if (dev->mdio.addr > 10)
		return 0;

	/* look for the switch on the bus */
	reg = bus->read(bus, PHYADDR(ADM_SIG0)) & ADM_SIG0_MASK;
	if (reg != ADM_SIG0_VAL)
		return 0;

	reg = bus->read(bus, PHYADDR(ADM_SIG1)) & ADM_SIG1_MASK;
	if (reg != ADM_SIG1_VAL)
		return 0;

	dev->phy_id = (ADM_SIG0_VAL << 16) | ADM_SIG1_VAL;

	return 0;
}

static int adm6996_probe(struct phy_device *pdev)
{
	return 0;
}

static void adm6996_remove(struct phy_device *pdev)
{
	struct adm6996_priv *priv = phy_to_adm(pdev);

	if (priv && (priv->model == ADM6996M || priv->model == ADM6996L))
		unregister_switch(&priv->dev);
}

static int adm6996_soft_reset(struct phy_device *phydev)
{
	/* we don't need an extra reset */
	return 0;
}

static struct phy_driver adm6996_phy_driver = {
	.name		= "Infineon ADM6996",
	.phy_id		= (ADM_SIG0_VAL << 16) | ADM_SIG1_VAL,
	.phy_id_mask	= 0xffffffff,
	.features	= PHY_BASIC_FEATURES,
	.probe		= adm6996_probe,
	.remove		= adm6996_remove,
	.config_init	= &adm6996_config_init,
	.config_aneg	= &adm6996_config_aneg,
	.read_status	= &adm6996_read_status,
	.soft_reset	= adm6996_soft_reset,
};

static int adm6996_gpio_probe(struct platform_device *pdev)
{
	struct adm6996_gpio_platform_data *pdata = pdev->dev.platform_data;
	struct adm6996_priv *priv;
	int ret;

	if (!pdata)
		return -EINVAL;

	priv = devm_kzalloc(&pdev->dev, sizeof(struct adm6996_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mutex_init(&priv->reg_mutex);
	mutex_init(&priv->mib_lock);

	priv->eecs = pdata->eecs;
	priv->eedi = pdata->eedi;
	priv->eesk = pdata->eesk;

	priv->model = pdata->model;
	priv->read = adm6996_read_gpio_reg;
	priv->write = adm6996_write_gpio_reg;

	ret = devm_gpio_request(&pdev->dev, priv->eecs, "adm_eecs");
	if (ret)
		return ret;
	ret = devm_gpio_request(&pdev->dev, priv->eedi, "adm_eedi");
	if (ret)
		return ret;
	ret = devm_gpio_request(&pdev->dev, priv->eesk, "adm_eesk");
	if (ret)
		return ret;

	ret = adm6996_switch_init(priv, dev_name(&pdev->dev), NULL);
	if (ret < 0)
		return ret;

	platform_set_drvdata(pdev, priv);

	return 0;
}

static int adm6996_gpio_remove(struct platform_device *pdev)
{
	struct adm6996_priv *priv = platform_get_drvdata(pdev);

	if (priv && (priv->model == ADM6996M || priv->model == ADM6996L))
		unregister_switch(&priv->dev);

	return 0;
}

static struct platform_driver adm6996_gpio_driver = {
	.probe = adm6996_gpio_probe,
	.remove = adm6996_gpio_remove,
	.driver = {
		.name = "adm6996_gpio",
	},
};

static int __init adm6996_init(void)
{
	int err;

	phy_register_fixup_for_id(PHY_ANY_ID, adm6996_fixup);
	err = phy_driver_register(&adm6996_phy_driver, THIS_MODULE);
	if (err)
		return err;

	err = platform_driver_register(&adm6996_gpio_driver);
	if (err)
		phy_driver_unregister(&adm6996_phy_driver);

	return err;
}

static void __exit adm6996_exit(void)
{
	platform_driver_unregister(&adm6996_gpio_driver);
	phy_driver_unregister(&adm6996_phy_driver);
}

module_init(adm6996_init);
module_exit(adm6996_exit);
