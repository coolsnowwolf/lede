/*
 * Marvell 88E61xx switch driver
 *
 * Copyright (c) 2014 Claudio Leite <leitec@staticky.com>
 * Copyright (c) 2014 Nikita Nazarenko <nnazarenko@radiofid.com>
 *
 * Based on code (c) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/delay.h>
#include <linux/switch.h>
#include <linux/device.h>
#include <linux/platform_device.h>

#include "mvsw61xx.h"

MODULE_DESCRIPTION("Marvell 88E61xx Switch driver");
MODULE_AUTHOR("Claudio Leite <leitec@staticky.com>");
MODULE_AUTHOR("Nikita Nazarenko <nnazarenko@radiofid.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:mvsw61xx");

/*
 * Register access is done through direct or indirect addressing,
 * depending on how the switch is physically connected.
 *
 * Direct addressing: all port and global registers directly
 *   accessible via an address/register pair
 *
 * Indirect addressing: switch is mapped at a single address,
 *   port and global registers accessible via a single command/data
 *   register pair
 */

static int
mvsw61xx_wait_mask_raw(struct mii_bus *bus, int addr,
		int reg, u16 mask, u16 val)
{
	int i = 100;
	u16 r;

	do {
		r = bus->read(bus, addr, reg);
		if ((r & mask) == val)
			return 0;
	} while (--i > 0);

	return -ETIMEDOUT;
}

static u16
r16(struct mii_bus *bus, bool indirect, int base_addr, int addr, int reg)
{
	u16 ind_addr;

	if (!indirect)
		return bus->read(bus, addr, reg);

	/* Indirect read: First, make sure switch is free */
	mvsw61xx_wait_mask_raw(bus, base_addr, MV_INDIRECT_REG_CMD,
			MV_INDIRECT_INPROGRESS, 0);

	/* Load address and request read */
	ind_addr = MV_INDIRECT_READ | (addr << MV_INDIRECT_ADDR_S) | reg;
	bus->write(bus, base_addr, MV_INDIRECT_REG_CMD,
			ind_addr);

	/* Wait until it's ready */
	mvsw61xx_wait_mask_raw(bus, base_addr, MV_INDIRECT_REG_CMD,
			MV_INDIRECT_INPROGRESS, 0);

	/* Read the requested data */
	return bus->read(bus, base_addr, MV_INDIRECT_REG_DATA);
}

static void
w16(struct mii_bus *bus, bool indirect, int base_addr, int addr,
		int reg, u16 val)
{
	u16 ind_addr;

	if (!indirect) {
		bus->write(bus, addr, reg, val);
		return;
	}

	/* Indirect write: First, make sure switch is free */
	mvsw61xx_wait_mask_raw(bus, base_addr, MV_INDIRECT_REG_CMD,
			MV_INDIRECT_INPROGRESS, 0);

	/* Load the data to be written */
	bus->write(bus, base_addr, MV_INDIRECT_REG_DATA, val);

	/* Wait again for switch to be free */
	mvsw61xx_wait_mask_raw(bus, base_addr, MV_INDIRECT_REG_CMD,
			MV_INDIRECT_INPROGRESS, 0);

	/* Load address, and issue write command */
	ind_addr = MV_INDIRECT_WRITE | (addr << MV_INDIRECT_ADDR_S) | reg;
	bus->write(bus, base_addr, MV_INDIRECT_REG_CMD,
			ind_addr);
}

/* swconfig support */

static inline u16
sr16(struct switch_dev *dev, int addr, int reg)
{
	struct mvsw61xx_state *state = get_state(dev);

	return r16(state->bus, state->is_indirect, state->base_addr, addr, reg);
}

static inline void
sw16(struct switch_dev *dev, int addr, int reg, u16 val)
{
	struct mvsw61xx_state *state = get_state(dev);

	w16(state->bus, state->is_indirect, state->base_addr, addr, reg, val);
}

static int
mvsw61xx_wait_mask_s(struct switch_dev *dev, int addr,
		int reg, u16 mask, u16 val)
{
	int i = 100;
	u16 r;

	do {
		r = sr16(dev, addr, reg) & mask;
		if (r == val)
			return 0;
	} while (--i > 0);

	return -ETIMEDOUT;
}

static int
mvsw61xx_mdio_read(struct switch_dev *dev, int addr, int reg)
{
	sw16(dev, MV_GLOBAL2REG(SMI_OP),
	     MV_INDIRECT_READ | (addr << MV_INDIRECT_ADDR_S) | reg);

	if (mvsw61xx_wait_mask_s(dev,  MV_GLOBAL2REG(SMI_OP),
				 MV_INDIRECT_INPROGRESS, 0) < 0)
		return -ETIMEDOUT;

	return sr16(dev, MV_GLOBAL2REG(SMI_DATA));
}

static int
mvsw61xx_mdio_write(struct switch_dev *dev, int addr, int reg, u16 val)
{
	sw16(dev, MV_GLOBAL2REG(SMI_DATA), val);

	sw16(dev, MV_GLOBAL2REG(SMI_OP),
	     MV_INDIRECT_WRITE | (addr << MV_INDIRECT_ADDR_S) | reg);

	return mvsw61xx_wait_mask_s(dev,  MV_GLOBAL2REG(SMI_OP),
				    MV_INDIRECT_INPROGRESS, 0) < 0;
}

static int
mvsw61xx_mdio_page_read(struct switch_dev *dev, int port, int page, int reg)
{
	int ret;

	mvsw61xx_mdio_write(dev, port, MII_MV_PAGE, page);
	ret = mvsw61xx_mdio_read(dev, port, reg);
	mvsw61xx_mdio_write(dev, port, MII_MV_PAGE, 0);

	return ret;
}

static void
mvsw61xx_mdio_page_write(struct switch_dev *dev, int port, int page, int reg,
			 u16 val)
{
	mvsw61xx_mdio_write(dev, port, MII_MV_PAGE, page);
	mvsw61xx_mdio_write(dev, port, reg, val);
	mvsw61xx_mdio_write(dev, port, MII_MV_PAGE, 0);
}

static int
mvsw61xx_get_port_mask(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	char *buf = state->buf;
	int port, len, i;
	u16 reg;

	port = val->port_vlan;
	reg = sr16(dev, MV_PORTREG(VLANMAP, port)) & MV_PORTS_MASK;

	len = sprintf(buf, "0x%04x: ", reg);

	for (i = 0; i < MV_PORTS; i++) {
		if (reg & (1 << i))
			len += sprintf(buf + len, "%d ", i);
		else if (i == port)
			len += sprintf(buf + len, "(%d) ", i);
	}

	val->value.s = buf;

	return 0;
}

static int
mvsw61xx_get_port_qmode(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);

	val->value.i = state->ports[val->port_vlan].qmode;

	return 0;
}

static int
mvsw61xx_set_port_qmode(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);

	state->ports[val->port_vlan].qmode = val->value.i;

	return 0;
}

static int
mvsw61xx_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct mvsw61xx_state *state = get_state(dev);

	*val = state->ports[port].pvid;

	return 0;
}

static int
mvsw61xx_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct mvsw61xx_state *state = get_state(dev);

	if (val < 0 || val >= MV_VLANS)
		return -EINVAL;

	state->ports[port].pvid = (u16)val;

	return 0;
}

static int
mvsw61xx_get_port_link(struct switch_dev *dev, int port,
		struct switch_port_link *link)
{
	u16 status, speed;

	status = sr16(dev, MV_PORTREG(STATUS, port));

	link->link = status & MV_PORT_STATUS_LINK;
	if (!link->link)
		return 0;

	link->duplex = status & MV_PORT_STATUS_FDX;

	speed = (status & MV_PORT_STATUS_SPEED_MASK) >>
			MV_PORT_STATUS_SPEED_SHIFT;

	switch (speed) {
	case MV_PORT_STATUS_SPEED_10:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case MV_PORT_STATUS_SPEED_100:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case MV_PORT_STATUS_SPEED_1000:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	}

	return 0;
}

static int mvsw61xx_get_vlan_ports(struct switch_dev *dev,
		struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int i, j, mode, vno;

	vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	for (i = 0, j = 0; i < dev->ports; i++) {
		if (state->vlans[vno].mask & (1 << i)) {
			val->value.ports[j].id = i;

			mode = (state->vlans[vno].port_mode >> (i * 4)) & 0xf;
			if (mode == MV_VTUCTL_EGRESS_TAGGED)
				val->value.ports[j].flags =
					(1 << SWITCH_PORT_FLAG_TAGGED);
			else
				val->value.ports[j].flags = 0;

			j++;
		}
	}

	val->len = j;

	return 0;
}

static int mvsw61xx_set_vlan_ports(struct switch_dev *dev,
		struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int i, mode, pno, vno;

	vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	state->vlans[vno].mask = 0;
	state->vlans[vno].port_mode = 0;
	state->vlans[vno].port_sstate = 0;

	if(state->vlans[vno].vid == 0)
		state->vlans[vno].vid = vno;

	for (i = 0; i < val->len; i++) {
		pno = val->value.ports[i].id;

		state->vlans[vno].mask |= (1 << pno);
		if (val->value.ports[i].flags &
				(1 << SWITCH_PORT_FLAG_TAGGED))
			mode = MV_VTUCTL_EGRESS_TAGGED;
		else
			mode = MV_VTUCTL_EGRESS_UNTAGGED;

		state->vlans[vno].port_mode |= mode << (pno * 4);
		state->vlans[vno].port_sstate |=
			MV_STUCTL_STATE_FORWARDING << (pno * 4 + 2);
	}

	/*
	 * DISCARD is nonzero, so it must be explicitly
	 * set on ports not in the VLAN.
	 */
	for (i = 0; i < dev->ports; i++)
		if (!(state->vlans[vno].mask & (1 << i)))
			state->vlans[vno].port_mode |=
				MV_VTUCTL_DISCARD << (i * 4);

	return 0;
}

static int mvsw61xx_get_vlan_port_based(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	if (state->vlans[vno].port_based)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int mvsw61xx_set_vlan_port_based(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	if (val->value.i == 1)
		state->vlans[vno].port_based = true;
	else
		state->vlans[vno].port_based = false;

	return 0;
}

static int mvsw61xx_get_vid(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	val->value.i = state->vlans[vno].vid;

	return 0;
}

static int mvsw61xx_set_vid(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);
	int vno = val->port_vlan;

	if (vno <= 0 || vno >= dev->vlans)
		return -EINVAL;

	state->vlans[vno].vid = val->value.i;

	return 0;
}

static int mvsw61xx_get_enable_vlan(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);

	val->value.i = state->vlan_enabled;

	return 0;
}

static int mvsw61xx_set_enable_vlan(struct switch_dev *dev,
		const struct switch_attr *attr, struct switch_val *val)
{
	struct mvsw61xx_state *state = get_state(dev);

	state->vlan_enabled = val->value.i;

	return 0;
}

static int mvsw61xx_vtu_program(struct switch_dev *dev)
{
	struct mvsw61xx_state *state = get_state(dev);
	u16 v1, v2, s1, s2;
	int i;

	/* Flush */
	mvsw61xx_wait_mask_s(dev, MV_GLOBALREG(VTU_OP),
			MV_VTUOP_INPROGRESS, 0);
	sw16(dev, MV_GLOBALREG(VTU_OP),
			MV_VTUOP_INPROGRESS | MV_VTUOP_PURGE);

	/* Write VLAN table */
	for (i = 1; i < dev->vlans; i++) {
		if (state->vlans[i].mask == 0 ||
				state->vlans[i].vid == 0 ||
				state->vlans[i].port_based == true)
			continue;

		mvsw61xx_wait_mask_s(dev, MV_GLOBALREG(VTU_OP),
				MV_VTUOP_INPROGRESS, 0);

		/* Write per-VLAN port state into STU */
		s1 = (u16) (state->vlans[i].port_sstate & 0xffff);
		s2 = (u16) ((state->vlans[i].port_sstate >> 16) & 0xffff);

		sw16(dev, MV_GLOBALREG(VTU_VID), MV_VTU_VID_VALID);
		sw16(dev, MV_GLOBALREG(VTU_SID), i);
		sw16(dev, MV_GLOBALREG(VTU_DATA1), s1);
		sw16(dev, MV_GLOBALREG(VTU_DATA2), s2);
		sw16(dev, MV_GLOBALREG(VTU_DATA3), 0);

		sw16(dev, MV_GLOBALREG(VTU_OP),
				MV_VTUOP_INPROGRESS | MV_VTUOP_STULOAD);
		mvsw61xx_wait_mask_s(dev, MV_GLOBALREG(VTU_OP),
				MV_VTUOP_INPROGRESS, 0);

		/* Write VLAN information into VTU */
		v1 = (u16) (state->vlans[i].port_mode & 0xffff);
		v2 = (u16) ((state->vlans[i].port_mode >> 16) & 0xffff);

		sw16(dev, MV_GLOBALREG(VTU_VID),
				MV_VTU_VID_VALID | state->vlans[i].vid);
		sw16(dev, MV_GLOBALREG(VTU_SID), i);
		sw16(dev, MV_GLOBALREG(VTU_FID), i);
		sw16(dev, MV_GLOBALREG(VTU_DATA1), v1);
		sw16(dev, MV_GLOBALREG(VTU_DATA2), v2);
		sw16(dev, MV_GLOBALREG(VTU_DATA3), 0);

		sw16(dev, MV_GLOBALREG(VTU_OP),
				MV_VTUOP_INPROGRESS | MV_VTUOP_LOAD);
		mvsw61xx_wait_mask_s(dev, MV_GLOBALREG(VTU_OP),
				MV_VTUOP_INPROGRESS, 0);
	}

	return 0;
}

static void mvsw61xx_vlan_port_config(struct switch_dev *dev, int vno)
{
	struct mvsw61xx_state *state = get_state(dev);
	int i, mode;

	for (i = 0; i < dev->ports; i++) {
		if (!(state->vlans[vno].mask & (1 << i)))
			continue;

		mode = (state->vlans[vno].port_mode >> (i * 4)) & 0xf;

		if(mode != MV_VTUCTL_EGRESS_TAGGED)
			state->ports[i].pvid = state->vlans[vno].vid;

		if (state->vlans[vno].port_based) {
			state->ports[i].mask |= state->vlans[vno].mask;
			state->ports[i].fdb = vno;
		}
		else
			state->ports[i].qmode = MV_8021Q_MODE_SECURE;
	}
}

static int mvsw61xx_update_state(struct switch_dev *dev)
{
	struct mvsw61xx_state *state = get_state(dev);
	int i;
	u16 reg;

	if (!state->registered)
		return -EINVAL;

	/*
	 * Set 802.1q-only mode if vlan_enabled is true.
	 *
	 * Without this, even if 802.1q is enabled for
	 * a port/VLAN, it still depends on the port-based
	 * VLAN mask being set.
	 *
	 * With this setting, port-based VLANs are still
	 * functional, provided the VID is not in the VTU.
	 */
	reg = sr16(dev, MV_GLOBAL2REG(SDET_POLARITY));

	if (state->vlan_enabled)
		reg |= MV_8021Q_VLAN_ONLY;
	else
		reg &= ~MV_8021Q_VLAN_ONLY;

	sw16(dev, MV_GLOBAL2REG(SDET_POLARITY), reg);

	/*
	 * Set port-based VLAN masks on each port
	 * based only on VLAN definitions known to
	 * the driver (i.e. in state).
	 *
	 * This means any pre-existing port mapping is
	 * wiped out once our driver is initialized.
	 */
	for (i = 0; i < dev->ports; i++) {
		state->ports[i].mask = 0;
		state->ports[i].qmode = MV_8021Q_MODE_DISABLE;
	}

	for (i = 0; i < dev->vlans; i++)
		mvsw61xx_vlan_port_config(dev, i);

	for (i = 0; i < dev->ports; i++) {
		reg = sr16(dev, MV_PORTREG(VLANID, i)) & ~MV_PVID_MASK;
		reg |= state->ports[i].pvid;
		sw16(dev, MV_PORTREG(VLANID, i), reg);

		state->ports[i].mask &= ~(1 << i);

		/* set default forwarding DB number and port mask */
		reg = sr16(dev, MV_PORTREG(CONTROL1, i)) & ~MV_FDB_HI_MASK;
		reg |= (state->ports[i].fdb >> MV_FDB_HI_SHIFT) &
			MV_FDB_HI_MASK;
		sw16(dev, MV_PORTREG(CONTROL1, i), reg);

		reg = ((state->ports[i].fdb & 0xf) << MV_FDB_LO_SHIFT) |
			state->ports[i].mask;
		sw16(dev, MV_PORTREG(VLANMAP, i), reg);

		reg = sr16(dev, MV_PORTREG(CONTROL2, i)) &
			~MV_8021Q_MODE_MASK;
		reg |= state->ports[i].qmode << MV_8021Q_MODE_SHIFT;
		sw16(dev, MV_PORTREG(CONTROL2, i), reg);
	}

	mvsw61xx_vtu_program(dev);

	return 0;
}

static int mvsw61xx_apply(struct switch_dev *dev)
{
	return mvsw61xx_update_state(dev);
}

static void mvsw61xx_enable_serdes(struct switch_dev *dev)
{
	int bmcr = mvsw61xx_mdio_page_read(dev, MV_REG_FIBER_SERDES,
					   MV_PAGE_FIBER_SERDES, MII_BMCR);
	if (bmcr < 0)
		return;

	if (bmcr & BMCR_PDOWN)
		mvsw61xx_mdio_page_write(dev, MV_REG_FIBER_SERDES,
					 MV_PAGE_FIBER_SERDES, MII_BMCR,
					 bmcr & ~BMCR_PDOWN);
}

static int _mvsw61xx_reset(struct switch_dev *dev, bool full)
{
	struct mvsw61xx_state *state = get_state(dev);
	int i;
	u16 reg;

	/* Disable all ports before reset */
	for (i = 0; i < dev->ports; i++) {
		reg = sr16(dev, MV_PORTREG(CONTROL, i)) &
			~MV_PORTCTRL_FORWARDING;
		sw16(dev, MV_PORTREG(CONTROL, i), reg);
	}

	reg = sr16(dev, MV_GLOBALREG(CONTROL)) | MV_CONTROL_RESET;

	sw16(dev, MV_GLOBALREG(CONTROL), reg);
	if (mvsw61xx_wait_mask_s(dev, MV_GLOBALREG(CONTROL),
				MV_CONTROL_RESET, 0) < 0)
		return -ETIMEDOUT;

	for (i = 0; i < dev->ports; i++) {
		state->ports[i].fdb = 0;
		state->ports[i].qmode = 0;
		state->ports[i].mask = 0;
		state->ports[i].pvid = 0;

		/* Force flow control off */
		reg = sr16(dev, MV_PORTREG(PHYCTL, i)) & ~MV_PHYCTL_FC_MASK;
		reg |= MV_PHYCTL_FC_DISABLE;
		sw16(dev, MV_PORTREG(PHYCTL, i), reg);

		/* Set port association vector */
		sw16(dev, MV_PORTREG(ASSOC, i), (1 << i));

		/* power up phys */
		if (full && i < 5) {
			mvsw61xx_mdio_write(dev, i, MII_MV_SPEC_CTRL,
					    MV_SPEC_MDI_CROSS_AUTO |
					    MV_SPEC_ENERGY_DETECT |
					    MV_SPEC_DOWNSHIFT_COUNTER);
			mvsw61xx_mdio_write(dev, i, MII_BMCR, BMCR_RESET |
					    BMCR_ANENABLE | BMCR_FULLDPLX |
					    BMCR_SPEED1000);
		}

		/* enable SerDes if necessary */
		if (full && i >= 5 && state->model == MV_IDENT_VALUE_6176) {
			u16 sts = sr16(dev, MV_PORTREG(STATUS, i));
			u16 mode = sts & MV_PORT_STATUS_CMODE_MASK;

			if (mode == MV_PORT_STATUS_CMODE_100BASE_X ||
			    mode == MV_PORT_STATUS_CMODE_1000BASE_X ||
			    mode == MV_PORT_STATUS_CMODE_SGMII) {
				mvsw61xx_enable_serdes(dev);
			}
		}
	}

	for (i = 0; i < dev->vlans; i++) {
		state->vlans[i].port_based = false;
		state->vlans[i].mask = 0;
		state->vlans[i].vid = 0;
		state->vlans[i].port_mode = 0;
		state->vlans[i].port_sstate = 0;
	}

	state->vlan_enabled = 0;

	mvsw61xx_update_state(dev);

	/* Re-enable ports */
	for (i = 0; i < dev->ports; i++) {
		reg = sr16(dev, MV_PORTREG(CONTROL, i)) |
			MV_PORTCTRL_FORWARDING;
		sw16(dev, MV_PORTREG(CONTROL, i), reg);
	}

	return 0;
}

static int mvsw61xx_reset(struct switch_dev *dev)
{
	return _mvsw61xx_reset(dev, false);
}

enum {
	MVSW61XX_ENABLE_VLAN,
};

enum {
	MVSW61XX_VLAN_PORT_BASED,
	MVSW61XX_VLAN_ID,
};

enum {
	MVSW61XX_PORT_MASK,
	MVSW61XX_PORT_QMODE,
};

static const struct switch_attr mvsw61xx_global[] = {
	[MVSW61XX_ENABLE_VLAN] = {
		.id = MVSW61XX_ENABLE_VLAN,
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable 802.1q VLAN support",
		.get = mvsw61xx_get_enable_vlan,
		.set = mvsw61xx_set_enable_vlan,
	},
};

static const struct switch_attr mvsw61xx_vlan[] = {
	[MVSW61XX_VLAN_PORT_BASED] = {
		.id = MVSW61XX_VLAN_PORT_BASED,
		.type = SWITCH_TYPE_INT,
		.name = "port_based",
		.description = "Use port-based (non-802.1q) VLAN only",
		.get = mvsw61xx_get_vlan_port_based,
		.set = mvsw61xx_set_vlan_port_based,
	},
	[MVSW61XX_VLAN_ID] = {
		.id = MVSW61XX_VLAN_ID,
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "Get/set VLAN ID",
		.get = mvsw61xx_get_vid,
		.set = mvsw61xx_set_vid,
	},
};

static const struct switch_attr mvsw61xx_port[] = {
	[MVSW61XX_PORT_MASK] = {
		.id = MVSW61XX_PORT_MASK,
		.type = SWITCH_TYPE_STRING,
		.description = "Port-based VLAN mask",
		.name = "mask",
		.get = mvsw61xx_get_port_mask,
		.set = NULL,
	},
	[MVSW61XX_PORT_QMODE] = {
		.id = MVSW61XX_PORT_QMODE,
		.type = SWITCH_TYPE_INT,
		.description = "802.1q mode: 0=off/1=fallback/2=check/3=secure",
		.name = "qmode",
		.get = mvsw61xx_get_port_qmode,
		.set = mvsw61xx_set_port_qmode,
	},
};

static const struct switch_dev_ops mvsw61xx_ops = {
	.attr_global = {
		.attr = mvsw61xx_global,
		.n_attr = ARRAY_SIZE(mvsw61xx_global),
	},
	.attr_vlan = {
		.attr = mvsw61xx_vlan,
		.n_attr = ARRAY_SIZE(mvsw61xx_vlan),
	},
	.attr_port = {
		.attr = mvsw61xx_port,
		.n_attr = ARRAY_SIZE(mvsw61xx_port),
	},
	.get_port_link = mvsw61xx_get_port_link,
	.get_port_pvid = mvsw61xx_get_port_pvid,
	.set_port_pvid = mvsw61xx_set_port_pvid,
	.get_vlan_ports = mvsw61xx_get_vlan_ports,
	.set_vlan_ports = mvsw61xx_set_vlan_ports,
	.apply_config = mvsw61xx_apply,
	.reset_switch = mvsw61xx_reset,
};

/* end swconfig stuff */

static int mvsw61xx_probe(struct platform_device *pdev)
{
	struct mvsw61xx_state *state;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mdio;
	char *model_str;
	u32 val;
	int err;

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	mdio = of_parse_phandle(np, "mii-bus", 0);
	if (!mdio) {
		dev_err(&pdev->dev, "Couldn't get MII bus handle\n");
		err = -ENODEV;
		goto out_err;
	}

	state->bus = of_mdio_find_bus(mdio);
	if (!state->bus) {
		dev_err(&pdev->dev, "Couldn't find MII bus from handle\n");
		err = -ENODEV;
		goto out_err;
	}

	state->is_indirect = of_property_read_bool(np, "is-indirect");

	if (state->is_indirect) {
		if (of_property_read_u32(np, "reg", &val)) {
			dev_err(&pdev->dev, "Switch address not specified\n");
			err = -ENODEV;
			goto out_err;
		}

		state->base_addr = val;
	} else {
		state->base_addr = MV_BASE;
	}

	state->model = r16(state->bus, state->is_indirect, state->base_addr,
				MV_PORTREG(IDENT, 0)) & MV_IDENT_MASK;

	switch(state->model) {
	case MV_IDENT_VALUE_6171:
		model_str = MV_IDENT_STR_6171;
		break;
	case MV_IDENT_VALUE_6172:
		model_str = MV_IDENT_STR_6172;
		break;
	case MV_IDENT_VALUE_6176:
		model_str = MV_IDENT_STR_6176;
		break;
	case MV_IDENT_VALUE_6352:
		model_str = MV_IDENT_STR_6352;
		break;
	default:
		dev_err(&pdev->dev, "No compatible switch found at 0x%02x\n",
				state->base_addr);
		err = -ENODEV;
		goto out_err;
	}

	platform_set_drvdata(pdev, state);
	dev_info(&pdev->dev, "Found %s at %s:%02x\n", model_str,
			state->bus->id, state->base_addr);

	dev_info(&pdev->dev, "Using %sdirect addressing\n",
			(state->is_indirect ? "in" : ""));

	if (of_property_read_u32(np, "cpu-port-0", &val)) {
		dev_err(&pdev->dev, "CPU port not set\n");
		err = -ENODEV;
		goto out_err;
	}

	state->cpu_port0 = val;

	if (!of_property_read_u32(np, "cpu-port-1", &val))
		state->cpu_port1 = val;
	else
		state->cpu_port1 = -1;

	state->dev.vlans = MV_VLANS;
	state->dev.cpu_port = state->cpu_port0;
	state->dev.ports = MV_PORTS;
	state->dev.name = model_str;
	state->dev.ops = &mvsw61xx_ops;
	state->dev.alias = dev_name(&pdev->dev);

	_mvsw61xx_reset(&state->dev, true);

	err = register_switch(&state->dev, NULL);
	if (err < 0)
		goto out_err;

	state->registered = true;

	return 0;
out_err:
	kfree(state);
	return err;
}

static int
mvsw61xx_remove(struct platform_device *pdev)
{
	struct mvsw61xx_state *state = platform_get_drvdata(pdev);

	if (state->registered)
		unregister_switch(&state->dev);

	kfree(state);

	return 0;
}

static const struct of_device_id mvsw61xx_match[] = {
	{ .compatible = "marvell,88e6171" },
	{ .compatible = "marvell,88e6172" },
	{ .compatible = "marvell,88e6176" },
	{ .compatible = "marvell,88e6352" },
	{ }
};
MODULE_DEVICE_TABLE(of, mvsw61xx_match);

static struct platform_driver mvsw61xx_driver = {
	.probe = mvsw61xx_probe,
	.remove = mvsw61xx_remove,
	.driver = {
		.name = "mvsw61xx",
		.of_match_table = of_match_ptr(mvsw61xx_match),
		.owner = THIS_MODULE,
	},
};

static int __init mvsw61xx_module_init(void)
{
	return platform_driver_register(&mvsw61xx_driver);
}
late_initcall(mvsw61xx_module_init);

static void __exit mvsw61xx_module_exit(void)
{
	platform_driver_unregister(&mvsw61xx_driver);
}
module_exit(mvsw61xx_module_exit);
