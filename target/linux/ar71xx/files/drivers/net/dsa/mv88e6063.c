/*
 * net/dsa/mv88e6063.c - Driver for Marvell 88e6063 switch chips
 * Copyright (c) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This driver was base on: net/dsa/mv88e6060.c
 *   net/dsa/mv88e6063.c - Driver for Marvell 88e6060 switch chips
 *   Copyright (c) 2008-2009 Marvell Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/version.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <net/dsa.h>

#define REG_BASE		0x10
#define REG_PHY(p)		(REG_BASE + (p))
#define REG_PORT(p)		(REG_BASE + 8 + (p))
#define REG_GLOBAL		(REG_BASE + 0x0f)
#define NUM_PORTS		7

static int reg_read(struct dsa_switch *ds, int addr, int reg)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	return mdiobus_read(ds->master_mii_bus, addr, reg);
#else
	struct mii_bus *bus = dsa_host_dev_to_mii_bus(ds->master_dev);
	return mdiobus_read(bus, addr, reg);
#endif
}

#define REG_READ(addr, reg)					\
	({							\
		int __ret;					\
								\
		__ret = reg_read(ds, addr, reg);		\
		if (__ret < 0)					\
			return __ret;				\
		__ret;						\
	})


static int reg_write(struct dsa_switch *ds, int addr, int reg, u16 val)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
	return mdiobus_write(ds->master_mii_bus, addr, reg, val);
#else
	struct mii_bus *bus = dsa_host_dev_to_mii_bus(ds->master_dev);
	return mdiobus_write(bus, addr, reg, val);
#endif
}

#define REG_WRITE(addr, reg, val)				\
	({							\
		int __ret;					\
								\
		__ret = reg_write(ds, addr, reg, val);		\
		if (__ret < 0)					\
			return __ret;				\
	})

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
static char *mv88e6063_probe(struct mii_bus *bus, int sw_addr)
{
#else
static char *mv88e6063_probe(struct device *host_dev, int sw_addr)
{
	struct mii_bus *bus = dsa_host_dev_to_mii_bus(host_dev);
#endif
	int ret;

	ret = mdiobus_read(bus, REG_PORT(0), 0x03);
	if (ret >= 0) {
		ret &= 0xfff0;
		if (ret == 0x1530)
			return "Marvell 88E6063";
	}

	return NULL;
}

static int mv88e6063_switch_reset(struct dsa_switch *ds)
{
	int i;
	int ret;

	/*
	 * Set all ports to the disabled state.
	 */
	for (i = 0; i < NUM_PORTS; i++) {
		ret = REG_READ(REG_PORT(i), 0x04);
		REG_WRITE(REG_PORT(i), 0x04, ret & 0xfffc);
	}

	/*
	 * Wait for transmit queues to drain.
	 */
	msleep(2);

	/*
	 * Reset the switch.
	 */
	REG_WRITE(REG_GLOBAL, 0x0a, 0xa130);

	/*
	 * Wait up to one second for reset to complete.
	 */
	for (i = 0; i < 1000; i++) {
		ret = REG_READ(REG_GLOBAL, 0x00);
		if ((ret & 0x8000) == 0x0000)
			break;

		msleep(1);
	}
	if (i == 1000)
		return -ETIMEDOUT;

	return 0;
}

static int mv88e6063_setup_global(struct dsa_switch *ds)
{
	/*
	 * Disable discarding of frames with excessive collisions,
	 * set the maximum frame size to 1536 bytes, and mask all
	 * interrupt sources.
	 */
	REG_WRITE(REG_GLOBAL, 0x04, 0x0800);

	/*
	 * Enable automatic address learning, set the address
	 * database size to 1024 entries, and set the default aging
	 * time to 5 minutes.
	 */
	REG_WRITE(REG_GLOBAL, 0x0a, 0x2130);

	return 0;
}

static int mv88e6063_setup_port(struct dsa_switch *ds, int p)
{
	int addr = REG_PORT(p);

	/*
	 * Do not force flow control, disable Ingress and Egress
	 * Header tagging, disable VLAN tunneling, and set the port
	 * state to Forwarding.  Additionally, if this is the CPU
	 * port, enable Ingress and Egress Trailer tagging mode.
	 */
	REG_WRITE(addr, 0x04, dsa_is_cpu_port(ds, p) ?  0x4103 : 0x0003);

	/*
	 * Port based VLAN map: give each port its own address
	 * database, allow the CPU port to talk to each of the 'real'
	 * ports, and allow each of the 'real' ports to only talk to
	 * the CPU port.
	 */
	REG_WRITE(addr, 0x06,
			((p & 0xf) << 12) |
			 (dsa_is_cpu_port(ds, p) ?
				ds->phys_port_mask :
				(1 << ds->dst->cpu_port)));

	/*
	 * Port Association Vector: when learning source addresses
	 * of packets, add the address to the address database using
	 * a port bitmap that has only the bit for this port set and
	 * the other bits clear.
	 */
	REG_WRITE(addr, 0x0b, 1 << p);

	return 0;
}

static int mv88e6063_setup(struct dsa_switch *ds)
{
	int i;
	int ret;

	ret = mv88e6063_switch_reset(ds);
	if (ret < 0)
		return ret;

	/* @@@ initialise atu */

	ret = mv88e6063_setup_global(ds);
	if (ret < 0)
		return ret;

	for (i = 0; i < NUM_PORTS; i++) {
		ret = mv88e6063_setup_port(ds, i);
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int mv88e6063_set_addr(struct dsa_switch *ds, u8 *addr)
{
	REG_WRITE(REG_GLOBAL, 0x01, (addr[0] << 8) | addr[1]);
	REG_WRITE(REG_GLOBAL, 0x02, (addr[2] << 8) | addr[3]);
	REG_WRITE(REG_GLOBAL, 0x03, (addr[4] << 8) | addr[5]);

	return 0;
}

static int mv88e6063_port_to_phy_addr(int port)
{
	if (port >= 0 && port <= NUM_PORTS)
		return REG_PHY(port);
	return -1;
}

static int mv88e6063_phy_read(struct dsa_switch *ds, int port, int regnum)
{
	int addr;

	addr = mv88e6063_port_to_phy_addr(port);
	if (addr == -1)
		return 0xffff;

	return reg_read(ds, addr, regnum);
}

static int
mv88e6063_phy_write(struct dsa_switch *ds, int port, int regnum, u16 val)
{
	int addr;

	addr = mv88e6063_port_to_phy_addr(port);
	if (addr == -1)
		return 0xffff;

	return reg_write(ds, addr, regnum, val);
}

static void mv88e6063_poll_link(struct dsa_switch *ds)
{
	int i;

	for (i = 0; i < DSA_MAX_PORTS; i++) {
		struct net_device *dev;
		int uninitialized_var(port_status);
		int link;
		int speed;
		int duplex;
		int fc;

		dev = ds->ports[i];
		if (dev == NULL)
			continue;

		link = 0;
		if (dev->flags & IFF_UP) {
			port_status = reg_read(ds, REG_PORT(i), 0x00);
			if (port_status < 0)
				continue;

			link = !!(port_status & 0x1000);
		}

		if (!link) {
			if (netif_carrier_ok(dev)) {
				printk(KERN_INFO "%s: link down\n", dev->name);
				netif_carrier_off(dev);
			}
			continue;
		}

		speed = (port_status & 0x0100) ? 100 : 10;
		duplex = (port_status & 0x0200) ? 1 : 0;
		fc = ((port_status & 0xc000) == 0xc000) ? 1 : 0;

		if (!netif_carrier_ok(dev)) {
			printk(KERN_INFO "%s: link up, %d Mb/s, %s duplex, "
					 "flow control %sabled\n", dev->name,
					 speed, duplex ? "full" : "half",
					 fc ? "en" : "dis");
			netif_carrier_on(dev);
		}
	}
}

static struct dsa_switch_driver mv88e6063_switch_driver = {
	.tag_protocol	= htons(ETH_P_TRAILER),
	.probe		= mv88e6063_probe,
	.setup		= mv88e6063_setup,
	.set_addr	= mv88e6063_set_addr,
	.phy_read	= mv88e6063_phy_read,
	.phy_write	= mv88e6063_phy_write,
	.poll_link	= mv88e6063_poll_link,
};

static int __init mv88e6063_init(void)
{
	register_switch_driver(&mv88e6063_switch_driver);
	return 0;
}
module_init(mv88e6063_init);

static void __exit mv88e6063_cleanup(void)
{
	unregister_switch_driver(&mv88e6063_switch_driver);
}
module_exit(mv88e6063_cleanup);
