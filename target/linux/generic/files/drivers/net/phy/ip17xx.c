/*
 * ip17xx.c: Swconfig configuration for IC+ IP17xx switch family
 *
 * Copyright (C) 2008 Patrick Horn <patrick.horn@gmail.com>
 * Copyright (C) 2008, 2010 Martin Mares <mj@ucw.cz>
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/mii.h>
#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/switch.h>
#include <linux/device.h>

#define MAX_VLANS 16
#define MAX_PORTS 9
#undef DUMP_MII_IO

typedef struct ip17xx_reg {
	u16 p;			// phy
	u16 m;			// mii
} reg;
typedef char bitnum;

#define NOTSUPPORTED {-1,-1}

#define REG_SUPP(x) (((x).m != ((u16)-1)) && ((x).p != (u16)-1))

struct ip17xx_state;

/*********** CONSTANTS ***********/
struct register_mappings {
	char *NAME;
	u16 MODEL_NO;			// Compare to bits 4-9 of MII register 0,3.
	bitnum NUM_PORTS;
	bitnum CPU_PORT;

/* The default VLAN for each port.
	 Default: 0x0001 for Ports 0,1,2,3
		  0x0002 for Ports 4,5 */
	reg VLAN_DEFAULT_TAG_REG[MAX_PORTS];

/* These ports are tagged.
	 Default: 0x00 */
	reg ADD_TAG_REG;
	reg REMOVE_TAG_REG;
	bitnum ADD_TAG_BIT[MAX_PORTS];
/* These ports are untagged.
	 Default: 0x00 (i.e. do not alter any VLAN tags...)
	 Maybe set to 0 if user disables VLANs. */
	bitnum REMOVE_TAG_BIT[MAX_PORTS];

/* Port M and Port N are on the same VLAN.
	 Default: All ports on all VLANs. */
// Use register {29, 19+N/2}
	reg VLAN_LOOKUP_REG;
// Port 5 uses register {30, 18} but same as odd bits.
	reg VLAN_LOOKUP_REG_5;		// in a different register on IP175C.
	bitnum VLAN_LOOKUP_EVEN_BIT[MAX_PORTS];
	bitnum VLAN_LOOKUP_ODD_BIT[MAX_PORTS];

/* This VLAN corresponds to which ports.
	 Default: 0x2f,0x30,0x3f,0x3f... */
	reg TAG_VLAN_MASK_REG;
	bitnum TAG_VLAN_MASK_EVEN_BIT[MAX_PORTS];
	bitnum TAG_VLAN_MASK_ODD_BIT[MAX_PORTS];

	int RESET_VAL;
	reg RESET_REG;

	reg MODE_REG;
	int MODE_VAL;

/* General flags */
	reg ROUTER_CONTROL_REG;
	reg VLAN_CONTROL_REG;
	bitnum TAG_VLAN_BIT;
	bitnum ROUTER_EN_BIT;
	bitnum NUMLAN_GROUPS_MAX;
	bitnum NUMLAN_GROUPS_BIT;

	reg MII_REGISTER_EN;
	bitnum MII_REGISTER_EN_BIT;

	// set to 1 for 178C, 0 for 175C.
	bitnum SIMPLE_VLAN_REGISTERS;	// 175C has two vlans per register but 178C has only one.

	// Pointers to functions which manipulate hardware state
	int (*update_state)(struct ip17xx_state *state);
	int (*set_vlan_mode)(struct ip17xx_state *state);
	int (*reset)(struct ip17xx_state *state);
};

static int ip175c_update_state(struct ip17xx_state *state);
static int ip175c_set_vlan_mode(struct ip17xx_state *state);
static int ip175c_reset(struct ip17xx_state *state);

static const struct register_mappings IP178C = {
	.NAME = "IP178C",
	.MODEL_NO = 0x18,
	.VLAN_DEFAULT_TAG_REG = {
		{30,3},{30,4},{30,5},{30,6},{30,7},{30,8},
		{30,9},{30,10},{30,11},
	},

	.ADD_TAG_REG = {30,12},
	.ADD_TAG_BIT = {0,1,2,3,4,5,6,7,8},
	.REMOVE_TAG_REG = {30,13},
	.REMOVE_TAG_BIT = {4,5,6,7,8,9,10,11,12},

	.SIMPLE_VLAN_REGISTERS = 1,

	.VLAN_LOOKUP_REG = {31,0},// +N
	.VLAN_LOOKUP_REG_5 = NOTSUPPORTED, // not used with SIMPLE_VLAN_REGISTERS
	.VLAN_LOOKUP_EVEN_BIT = {0,1,2,3,4,5,6,7,8},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,5,6,7,8},

	.TAG_VLAN_MASK_REG = {30,14}, // +N
	.TAG_VLAN_MASK_EVEN_BIT = {0,1,2,3,4,5,6,7,8},
	.TAG_VLAN_MASK_ODD_BIT = {0,1,2,3,4,5,6,7,8},

	.RESET_VAL = 0x55AA,
	.RESET_REG = {30,0},
	.MODE_VAL = 0,
	.MODE_REG = NOTSUPPORTED,

	.ROUTER_CONTROL_REG = {30,30},
	.ROUTER_EN_BIT = 11,
	.NUMLAN_GROUPS_MAX = 8,
	.NUMLAN_GROUPS_BIT = 8, // {0-2}

	.VLAN_CONTROL_REG = {30,13},
	.TAG_VLAN_BIT = 3,

	.CPU_PORT = 8,
	.NUM_PORTS = 9,

	.MII_REGISTER_EN = NOTSUPPORTED,

	.update_state = ip175c_update_state,
	.set_vlan_mode = ip175c_set_vlan_mode,
	.reset = ip175c_reset,
};

static const struct register_mappings IP175C = {
	.NAME = "IP175C",
	.MODEL_NO = 0x18,
	.VLAN_DEFAULT_TAG_REG = {
		{29,24},{29,25},{29,26},{29,27},{29,28},{29,30},
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED
	},

	.ADD_TAG_REG = {29,23},
	.REMOVE_TAG_REG = {29,23},
	.ADD_TAG_BIT = {11,12,13,14,15,1,-1,-1,-1},
	.REMOVE_TAG_BIT = {6,7,8,9,10,0,-1,-1,-1},

	.SIMPLE_VLAN_REGISTERS = 0,

	.VLAN_LOOKUP_REG = {29,19},// +N/2
	.VLAN_LOOKUP_REG_5 = {30,18},
	.VLAN_LOOKUP_EVEN_BIT = {8,9,10,11,12,15,-1,-1,-1},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,7,-1,-1,-1},

	.TAG_VLAN_MASK_REG = {30,1}, // +N/2
	.TAG_VLAN_MASK_EVEN_BIT = {0,1,2,3,4,5,-1,-1,-1},
	.TAG_VLAN_MASK_ODD_BIT = {8,9,10,11,12,13,-1,-1,-1},

	.RESET_VAL = 0x175C,
	.RESET_REG = {30,0},
	.MODE_VAL = 0x175C,
	.MODE_REG = {29,31},

	.ROUTER_CONTROL_REG = {30,9},
	.ROUTER_EN_BIT = 3,
	.NUMLAN_GROUPS_MAX = 8,
	.NUMLAN_GROUPS_BIT = 0, // {0-2}

	.VLAN_CONTROL_REG = {30,9},
	.TAG_VLAN_BIT = 7,

	.NUM_PORTS = 6,
	.CPU_PORT = 5,

	.MII_REGISTER_EN = NOTSUPPORTED,

	.update_state = ip175c_update_state,
	.set_vlan_mode = ip175c_set_vlan_mode,
	.reset = ip175c_reset,
};

static const struct register_mappings IP175A = {
	.NAME = "IP175A",
	.MODEL_NO = 0x05,
	.VLAN_DEFAULT_TAG_REG = {
		{0,24},{0,25},{0,26},{0,27},{0,28},NOTSUPPORTED,
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED
	},

	.ADD_TAG_REG = {0,23},
	.REMOVE_TAG_REG = {0,23},
	.ADD_TAG_BIT = {11,12,13,14,15,-1,-1,-1,-1},
	.REMOVE_TAG_BIT = {6,7,8,9,10,-1,-1,-1,-1},

	.SIMPLE_VLAN_REGISTERS = 0,

	// Only programmable via EEPROM
	.VLAN_LOOKUP_REG = NOTSUPPORTED,// +N/2
	.VLAN_LOOKUP_REG_5 = NOTSUPPORTED,
	.VLAN_LOOKUP_EVEN_BIT = {8,9,10,11,12,-1,-1,-1,-1},
	.VLAN_LOOKUP_ODD_BIT = {0,1,2,3,4,-1,-1,-1,-1},

	.TAG_VLAN_MASK_REG = NOTSUPPORTED, // +N/2,
	.TAG_VLAN_MASK_EVEN_BIT = {-1,-1,-1,-1,-1,-1,-1,-1,-1},
	.TAG_VLAN_MASK_ODD_BIT = {-1,-1,-1,-1,-1,-1,-1,-1,-1},

	.RESET_VAL = -1,
	.RESET_REG = NOTSUPPORTED,
	.MODE_VAL = 0,
	.MODE_REG = NOTSUPPORTED,

	.ROUTER_CONTROL_REG = NOTSUPPORTED,
	.VLAN_CONTROL_REG = NOTSUPPORTED,
	.TAG_VLAN_BIT = -1,
	.ROUTER_EN_BIT = -1,
	.NUMLAN_GROUPS_MAX = -1,
	.NUMLAN_GROUPS_BIT = -1, // {0-2}

	.NUM_PORTS = 5,
	.CPU_PORT = 4,

	.MII_REGISTER_EN = {0, 18},
	.MII_REGISTER_EN_BIT = 7,

	.update_state = ip175c_update_state,
	.set_vlan_mode = ip175c_set_vlan_mode,
	.reset = ip175c_reset,
};


static int ip175d_update_state(struct ip17xx_state *state);
static int ip175d_set_vlan_mode(struct ip17xx_state *state);
static int ip175d_reset(struct ip17xx_state *state);

static const struct register_mappings IP175D = {
	.NAME = "IP175D",
	.MODEL_NO = 0x18,

	// The IP175D has a completely different interface, so we leave most
	// of the registers undefined and switch to different code paths.

	.VLAN_DEFAULT_TAG_REG = {
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED,
		NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED,NOTSUPPORTED,
	},

	.ADD_TAG_REG = NOTSUPPORTED,
	.REMOVE_TAG_REG = NOTSUPPORTED,

	.SIMPLE_VLAN_REGISTERS = 0,

	.VLAN_LOOKUP_REG = NOTSUPPORTED,
	.VLAN_LOOKUP_REG_5 = NOTSUPPORTED,
	.TAG_VLAN_MASK_REG = NOTSUPPORTED,

	.RESET_VAL = 0x175D,
	.RESET_REG = {20,2},
	.MODE_REG = NOTSUPPORTED,

	.ROUTER_CONTROL_REG = NOTSUPPORTED,
	.ROUTER_EN_BIT = -1,
	.NUMLAN_GROUPS_BIT = -1,

	.VLAN_CONTROL_REG = NOTSUPPORTED,
	.TAG_VLAN_BIT = -1,

	.NUM_PORTS = 6,
	.CPU_PORT = 5,

	.MII_REGISTER_EN = NOTSUPPORTED,

	.update_state = ip175d_update_state,
	.set_vlan_mode = ip175d_set_vlan_mode,
	.reset = ip175d_reset,
};

struct ip17xx_state {
	struct switch_dev dev;
	struct mii_bus *mii_bus;
	bool registered;

	int router_mode;		// ROUTER_EN
	int vlan_enabled;		// TAG_VLAN_EN
	struct port_state {
		u16 pvid;
		unsigned int shareports;
	} ports[MAX_PORTS];
	unsigned int add_tag;
	unsigned int remove_tag;
	int num_vlans;
	struct vlan_state {
		unsigned int ports;
		unsigned int tag;	// VLAN tag (IP175D only)
	} vlans[MAX_VLANS];
	const struct register_mappings *regs;
	reg proc_mii; 	// phy/reg for the low level register access via swconfig

	char buf[80];
};

#define get_state(_dev) container_of((_dev), struct ip17xx_state, dev)

static int ip_phy_read(struct ip17xx_state *state, int port, int reg)
{
	int val = mdiobus_read(state->mii_bus, port, reg);
	if (val < 0)
		pr_warn("IP17xx: Unable to get MII register %d,%d: error %d\n", port, reg, -val);
#ifdef DUMP_MII_IO
	else
		pr_debug("IP17xx: Read MII(%d,%d) -> %04x\n", port, reg, val);
#endif
	return val;
}

static int ip_phy_write(struct ip17xx_state *state, int port, int reg, u16 val)
{
	int err;

#ifdef DUMP_MII_IO
	pr_debug("IP17xx: Write MII(%d,%d) <- %04x\n", port, reg, val);
#endif
	err = mdiobus_write(state->mii_bus, port, reg, val);
	if (err < 0)
		pr_warn("IP17xx: Unable to write MII register %d,%d: error %d\n", port, reg, -err);
	return err;
}

static int ip_phy_write_masked(struct ip17xx_state *state, int port, int reg, unsigned int mask, unsigned int data)
{
	int val = ip_phy_read(state, port, reg);
	if (val < 0)
		return 0;
	return ip_phy_write(state, port, reg, (val & ~mask) | data);
}

static int getPhy(struct ip17xx_state *state, reg mii)
{
	if (!REG_SUPP(mii))
		return -EFAULT;
	return ip_phy_read(state, mii.p, mii.m);
}

static int setPhy(struct ip17xx_state *state, reg mii, u16 value)
{
	int err;

	if (!REG_SUPP(mii))
		return -EFAULT;
	err = ip_phy_write(state, mii.p, mii.m, value);
	if (err < 0)
		return err;
	mdelay(2);
	getPhy(state, mii);
	return 0;
}


/**
 * These two macros are to simplify the mapping of logical bits to the bits in hardware.
 * NOTE: these macros will return if there is an error!
 */

#define GET_PORT_BITS(state, bits, addr, bit_lookup)		\
	do {							\
		int i, val = getPhy((state), (addr));		\
		if (val < 0)					\
			return val;				\
		(bits) = 0;					\
		for (i = 0; i < MAX_PORTS; i++) {		\
			if ((bit_lookup)[i] == -1) continue;	\
			if (val & (1<<(bit_lookup)[i]))		\
				(bits) |= (1<<i);		\
		}						\
	} while (0)

#define SET_PORT_BITS(state, bits, addr, bit_lookup)		\
	do {							\
		int i, val = getPhy((state), (addr));		\
		if (val < 0)					\
			return val;				\
		for (i = 0; i < MAX_PORTS; i++) {		\
			unsigned int newmask = ((bits)&(1<<i));	\
			if ((bit_lookup)[i] == -1) continue;	\
			val &= ~(1<<(bit_lookup)[i]);		\
			val |= ((newmask>>i)<<(bit_lookup)[i]);	\
		}						\
		val = setPhy((state), (addr), val);		\
		if (val < 0)					\
			return val;				\
	} while (0)


static int get_model(struct ip17xx_state *state)
{
	int id1, id2;
	int oui_id, model_no, rev_no, chip_no;

	id1 = ip_phy_read(state, 0, 2);
	id2 = ip_phy_read(state, 0, 3);
	oui_id = (id1 << 6) | ((id2 >> 10) & 0x3f);
	model_no = (id2 >> 4) & 0x3f;
	rev_no = id2 & 0xf;
	pr_debug("IP17xx: Identified oui=%06x model=%02x rev=%X\n", oui_id, model_no, rev_no);

	if (oui_id != 0x0090c3)  // No other oui_id should have reached us anyway
		return -ENODEV;

	if (model_no == IP175A.MODEL_NO) {
		state->regs = &IP175A;
	} else if (model_no == IP175C.MODEL_NO) {
		/*
		 *  Several models share the same model_no:
		 *  178C has more PHYs, so we try whether the device responds to a read from PHY5
		 *  175D has a new chip ID register
		 *  175C has neither
		 */
		if (ip_phy_read(state, 5, 2) == 0x0243) {
			state->regs = &IP178C;
		} else {
			chip_no = ip_phy_read(state, 20, 0);
			pr_debug("IP17xx: Chip ID register reads %04x\n", chip_no);
			if (chip_no == 0x175d) {
				state->regs = &IP175D;
			} else {
				state->regs = &IP175C;
			}
		}
	} else {
		pr_warn("IP17xx: Found an unknown IC+ switch with model number %02x, revision %X.\n", model_no, rev_no);
		return -EPERM;
	}
	return 0;
}

/*** Low-level functions for the older models ***/

/** Only set vlan and router flags in the switch **/
static int ip175c_set_flags(struct ip17xx_state *state)
{
	int val;

	if (!REG_SUPP(state->regs->ROUTER_CONTROL_REG)) {
		return 0;
	}

	val = getPhy(state, state->regs->ROUTER_CONTROL_REG);
	if (val < 0) {
		return val;
	}
	if (state->regs->ROUTER_EN_BIT >= 0) {
		if (state->router_mode) {
			val |= (1<<state->regs->ROUTER_EN_BIT);
		} else {
			val &= (~(1<<state->regs->ROUTER_EN_BIT));
		}
	}
	if (state->regs->TAG_VLAN_BIT >= 0) {
		if (state->vlan_enabled) {
			val |= (1<<state->regs->TAG_VLAN_BIT);
		} else {
			val &= (~(1<<state->regs->TAG_VLAN_BIT));
		}
	}
	if (state->regs->NUMLAN_GROUPS_BIT >= 0) {
		val &= (~((state->regs->NUMLAN_GROUPS_MAX-1)<<state->regs->NUMLAN_GROUPS_BIT));
		if (state->num_vlans > state->regs->NUMLAN_GROUPS_MAX) {
			val |= state->regs->NUMLAN_GROUPS_MAX << state->regs->NUMLAN_GROUPS_BIT;
		} else if (state->num_vlans >= 1) {
			val |= (state->num_vlans-1) << state->regs->NUMLAN_GROUPS_BIT;
		}
	}
	return setPhy(state, state->regs->ROUTER_CONTROL_REG, val);
}

/** Set all VLAN and port state.  Usually you should call "correct_vlan_state" first. **/
static int ip175c_set_state(struct ip17xx_state *state)
{
	int j;
	int i;
	SET_PORT_BITS(state, state->add_tag,
				  state->regs->ADD_TAG_REG, state->regs->ADD_TAG_BIT);
	SET_PORT_BITS(state, state->remove_tag,
				  state->regs->REMOVE_TAG_REG, state->regs->REMOVE_TAG_BIT);

	if (REG_SUPP(state->regs->VLAN_LOOKUP_REG)) {
		for (j=0; j<state->regs->NUM_PORTS; j++) {
			reg addr;
			const bitnum *bit_lookup = (j%2==0)?
				state->regs->VLAN_LOOKUP_EVEN_BIT:
				state->regs->VLAN_LOOKUP_ODD_BIT;

			addr = state->regs->VLAN_LOOKUP_REG;
			if (state->regs->SIMPLE_VLAN_REGISTERS) {
				addr.m += j;
			} else {
				switch (j) {
				case 0:
				case 1:
					break;
				case 2:
				case 3:
					addr.m+=1;
					break;
				case 4:
					addr.m+=2;
					break;
				case 5:
					addr = state->regs->VLAN_LOOKUP_REG_5;
					break;
				default:
					addr.m = -1; // shouldn't get here, but...
					break;
				}
			}
			//printf("shareports for %d is %02X\n",j,state->ports[j].shareports);
			if (REG_SUPP(addr)) {
				SET_PORT_BITS(state, state->ports[j].shareports, addr, bit_lookup);
			}
		}
	}
	if (REG_SUPP(state->regs->TAG_VLAN_MASK_REG)) {
		for (j=0; j<MAX_VLANS; j++) {
			reg addr = state->regs->TAG_VLAN_MASK_REG;
			const bitnum *bit_lookup = (j%2==0)?
				state->regs->TAG_VLAN_MASK_EVEN_BIT:
				state->regs->TAG_VLAN_MASK_ODD_BIT;
			unsigned int vlan_mask;
			if (state->regs->SIMPLE_VLAN_REGISTERS) {
				addr.m += j;
			} else {
				addr.m += j/2;
			}
			vlan_mask = state->vlans[j].ports;
			SET_PORT_BITS(state, vlan_mask, addr, bit_lookup);
		}
	}

	for (i=0; i<MAX_PORTS; i++) {
		if (REG_SUPP(state->regs->VLAN_DEFAULT_TAG_REG[i])) {
			int err = setPhy(state, state->regs->VLAN_DEFAULT_TAG_REG[i],
					state->ports[i].pvid);
			if (err < 0) {
				return err;
			}
		}
	}

	return ip175c_set_flags(state);
}

/**
 *  Uses only the VLAN port mask and the add tag mask to generate the other fields:
 *  which ports are part of the same VLAN, removing vlan tags, and VLAN tag ids.
 */
static void ip175c_correct_vlan_state(struct ip17xx_state *state)
{
	int i, j;
	state->num_vlans = 0;
	for (i=0; i<MAX_VLANS; i++) {
		if (state->vlans[i].ports != 0) {
			state->num_vlans = i+1; // Hack -- we need to store the "set" vlans somewhere...
		}
	}

	for (i=0; i<state->regs->NUM_PORTS; i++) {
		unsigned int portmask = (1<<i);
		if (!state->vlan_enabled) {
			// Share with everybody!
			state->ports[i].shareports = (1<<state->regs->NUM_PORTS)-1;
			continue;
		}
		state->ports[i].shareports = portmask;
		for (j=0; j<MAX_VLANS; j++) {
			if (state->vlans[j].ports & portmask)
				state->ports[i].shareports |= state->vlans[j].ports;
		}
	}
}

static int ip175c_update_state(struct ip17xx_state *state)
{
	ip175c_correct_vlan_state(state);
	return ip175c_set_state(state);
}

static int ip175c_set_vlan_mode(struct ip17xx_state *state)
{
	return ip175c_update_state(state);
}

static int ip175c_reset(struct ip17xx_state *state)
{
	int err;

	if (REG_SUPP(state->regs->MODE_REG)) {
		err = setPhy(state, state->regs->MODE_REG, state->regs->MODE_VAL);
		if (err < 0)
			return err;
		err = getPhy(state, state->regs->MODE_REG);
		if (err < 0)
			return err;
	}

	return ip175c_update_state(state);
}

/*** Low-level functions for IP175D ***/

static int ip175d_update_state(struct ip17xx_state *state)
{
	unsigned int filter_mask = 0;
	unsigned int ports[16], add[16], rem[16];
	int i, j;
	int err = 0;

	for (i = 0; i < 16; i++) {
		ports[i] = 0;
		add[i] = 0;
		rem[i] = 0;
		if (!state->vlan_enabled) {
			err |= ip_phy_write(state, 22, 14+i, i+1);	// default tags
			ports[i] = 0x3f;
			continue;
		}
		if (!state->vlans[i].tag) {
			// Reset the filter
			err |= ip_phy_write(state, 22, 14+i, 0);	// tag
			continue;
		}
		filter_mask |= 1 << i;
		err |= ip_phy_write(state, 22, 14+i, state->vlans[i].tag);
		ports[i] = state->vlans[i].ports;
		for (j = 0; j < 6; j++) {
			if (ports[i] & (1 << j)) {
				if (state->add_tag & (1 << j))
					add[i] |= 1 << j;
				if (state->remove_tag & (1 << j))
					rem[i] |= 1 << j;
			}
		}
	}

	// Port masks, tag adds and removals
	for (i = 0; i < 8; i++) {
		err |= ip_phy_write(state, 23, i, ports[2*i] | (ports[2*i+1] << 8));
		err |= ip_phy_write(state, 23, 8+i, add[2*i] | (add[2*i+1] << 8));
		err |= ip_phy_write(state, 23, 16+i, rem[2*i] | (rem[2*i+1] << 8));
	}
	err |= ip_phy_write(state, 22, 10, filter_mask);

	// Default VLAN tag for each port
	for (i = 0; i < 6; i++)
		err |= ip_phy_write(state, 22, 4+i, state->vlans[state->ports[i].pvid].tag);

	return (err ? -EIO : 0);
}

static int ip175d_set_vlan_mode(struct ip17xx_state *state)
{
	int i;
	int err = 0;

	if (state->vlan_enabled) {
		// VLAN classification rules: tag-based VLANs, use VID to classify,
		// drop packets that cannot be classified.
		err |= ip_phy_write_masked(state, 22, 0, 0x3fff, 0x003f);

		// Ingress rules: CFI=1 dropped, null VID is untagged, VID=1 passed,
		// VID=0xfff discarded, admin both tagged and untagged, ingress
		// filters enabled.
		err |= ip_phy_write_masked(state, 22, 1, 0x0fff, 0x0c3f);

		// Egress rules: IGMP processing off, keep VLAN header off
		err |= ip_phy_write_masked(state, 22, 2, 0x0fff, 0x0000);
	} else {
		// VLAN classification rules: everything off & clear table
		err |= ip_phy_write_masked(state, 22, 0, 0xbfff, 0x8000);

		// Ingress and egress rules: set to defaults
		err |= ip_phy_write_masked(state, 22, 1, 0x0fff, 0x0c3f);
		err |= ip_phy_write_masked(state, 22, 2, 0x0fff, 0x0000);
	}

	// Reset default VLAN for each port to 0
	for (i = 0; i < 6; i++)
		state->ports[i].pvid = 0;

	err |= ip175d_update_state(state);

	return (err ? -EIO : 0);
}

static int ip175d_reset(struct ip17xx_state *state)
{
	int err = 0;

	// Disable the special tagging mode
	err |= ip_phy_write_masked(state, 21, 22, 0x0003, 0x0000);

	// Set 802.1q protocol type
	err |= ip_phy_write(state, 22, 3, 0x8100);

	state->vlan_enabled = 0;
	err |= ip175d_set_vlan_mode(state);

	return (err ? -EIO : 0);
}

/*** High-level functions ***/

static int ip17xx_get_enable_vlan(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);

	val->value.i = state->vlan_enabled;
	return 0;
}

static void ip17xx_reset_vlan_config(struct ip17xx_state *state)
{
	int i;

	state->remove_tag = (state->vlan_enabled ? ((1<<state->regs->NUM_PORTS)-1) : 0x0000);
	state->add_tag = 0x0000;
	for (i = 0; i < MAX_VLANS; i++) {
		state->vlans[i].ports = 0x0000;
		state->vlans[i].tag = (i ? i : 16);
	}
	for (i = 0; i < MAX_PORTS; i++)
		state->ports[i].pvid = 0;
}

static int ip17xx_set_enable_vlan(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int enable;

	enable = val->value.i;
	if (state->vlan_enabled == enable) {
		// Do not change any state.
		return 0;
	}
	state->vlan_enabled = enable;

	// Otherwise, if we are switching state, set fields to a known default.
	ip17xx_reset_vlan_config(state);

	return state->regs->set_vlan_mode(state);
}

static int ip17xx_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int b;
	int ind;
	unsigned int ports;

	if (val->port_vlan >= dev->vlans || val->port_vlan < 0)
		return -EINVAL;

	ports = state->vlans[val->port_vlan].ports;
	b = 0;
	ind = 0;
	while (b < MAX_PORTS) {
		if (ports&1) {
			int istagged = ((state->add_tag >> b) & 1);
			val->value.ports[ind].id = b;
			val->value.ports[ind].flags = (istagged << SWITCH_PORT_FLAG_TAGGED);
			ind++;
		}
		b++;
		ports >>= 1;
	}
	val->len = ind;

	return 0;
}

static int ip17xx_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int i;

	if (val->port_vlan >= dev->vlans || val->port_vlan < 0)
		return -EINVAL;

	state->vlans[val->port_vlan].ports = 0;
	for (i = 0; i < val->len; i++) {
		unsigned int bitmask = (1<<val->value.ports[i].id);
		state->vlans[val->port_vlan].ports |= bitmask;
		if (val->value.ports[i].flags & (1<<SWITCH_PORT_FLAG_TAGGED)) {
			state->add_tag |= bitmask;
			state->remove_tag &= (~bitmask);
		} else {
			state->add_tag &= (~bitmask);
			state->remove_tag |= bitmask;
		}
	}

	return state->regs->update_state(state);
}

static int ip17xx_apply(struct switch_dev *dev)
{
	struct ip17xx_state *state = get_state(dev);

	if (REG_SUPP(state->regs->MII_REGISTER_EN)) {
		int val = getPhy(state, state->regs->MII_REGISTER_EN);
		if (val < 0) {
			return val;
		}
		val |= (1<<state->regs->MII_REGISTER_EN_BIT);
		return setPhy(state, state->regs->MII_REGISTER_EN, val);
	}
	return 0;
}

static int ip17xx_reset(struct switch_dev *dev)
{
	struct ip17xx_state *state = get_state(dev);
	int i, err;

	if (REG_SUPP(state->regs->RESET_REG)) {
		err = setPhy(state, state->regs->RESET_REG, state->regs->RESET_VAL);
		if (err < 0)
			return err;
		err = getPhy(state, state->regs->RESET_REG);

		/*
		 *  Data sheet specifies reset period to be 2 msec.
		 *  (I don't see any mention of the 2ms delay in the IP178C spec, only
		 *  in IP175C, but it can't hurt.)
		 */
		mdelay(2);
	}

	/* reset switch ports */
	for (i = 0; i < state->regs->NUM_PORTS-1; i++) {
		err = ip_phy_write(state, i, MII_BMCR, BMCR_RESET);
		if (err < 0)
			return err;
	}

	state->router_mode = 0;
	state->vlan_enabled = 0;
	ip17xx_reset_vlan_config(state);

	return state->regs->reset(state);
}

static int ip17xx_get_tagged(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);

	if (state->add_tag & (1<<val->port_vlan)) {
		if (state->remove_tag & (1<<val->port_vlan))
			val->value.i = 3; // shouldn't ever happen.
		else
			val->value.i = 1;
	} else {
		if (state->remove_tag & (1<<val->port_vlan))
			val->value.i = 0;
		else
			val->value.i = 2;
	}
	return 0;
}

static int ip17xx_set_tagged(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);

	state->add_tag &= ~(1<<val->port_vlan);
	state->remove_tag &= ~(1<<val->port_vlan);

	if (val->value.i == 0)
		state->remove_tag |= (1<<val->port_vlan);
	if (val->value.i == 1)
		state->add_tag |= (1<<val->port_vlan);

	return state->regs->update_state(state);
}

/** Get the current phy address */
static int ip17xx_get_phy(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);

	val->value.i = state->proc_mii.p;
	return 0;
}

/** Set a new phy address for low level access to registers */
static int ip17xx_set_phy(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int new_reg = val->value.i;

	if (new_reg < 0 || new_reg > 31)
		state->proc_mii.p = (u16)-1;
	else
		state->proc_mii.p = (u16)new_reg;
	return 0;
}

/** Get the current register number */
static int ip17xx_get_reg(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);

	val->value.i = state->proc_mii.m;
	return 0;
}

/** Set a new register address for low level access to registers */
static int ip17xx_set_reg(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int new_reg = val->value.i;

	if (new_reg < 0 || new_reg > 31)
		state->proc_mii.m = (u16)-1;
	else
		state->proc_mii.m = (u16)new_reg;
	return 0;
}

/** Get the register content of state->proc_mii */
static int ip17xx_get_val(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int retval = -EINVAL;
	if (REG_SUPP(state->proc_mii))
		retval = getPhy(state, state->proc_mii);

	if (retval < 0) {
		return retval;
	} else {
		val->value.i = retval;
		return 0;
	}
}

/** Write a value to the register defined by phy/reg above */
static int ip17xx_set_val(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int myval, err = -EINVAL;

	myval = val->value.i;
	if (myval <= 0xffff && myval >= 0 && REG_SUPP(state->proc_mii)) {
		err = setPhy(state, state->proc_mii, (u16)myval);
	}
	return err;
}

static int ip17xx_read_name(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	val->value.s = state->regs->NAME; // Just a const pointer, won't be freed by swconfig.
	return 0;
}

static int ip17xx_get_tag(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int vlan = val->port_vlan;

	if (vlan < 0 || vlan >= MAX_VLANS)
		return -EINVAL;

	val->value.i = state->vlans[vlan].tag;
	return 0;
}

static int ip17xx_set_tag(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int vlan = val->port_vlan;
	int tag = val->value.i;

	if (vlan < 0 || vlan >= MAX_VLANS)
		return -EINVAL;

	if (tag < 0 || tag > 4095)
		return -EINVAL;

	state->vlans[vlan].tag = tag;
	return state->regs->update_state(state);
}

static int ip17xx_set_port_speed(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int nr = val->port_vlan;
	int ctrl;
	int autoneg;
	int speed;
	if (val->value.i == 100) {
		speed = 1;
		autoneg = 0;
	} else if (val->value.i == 10) {
		speed = 0;
		autoneg = 0;
	} else {
		autoneg = 1;
		speed = 1;
	}

	/* Can't set speed for cpu port */
	if (nr == state->regs->CPU_PORT)
		return -EINVAL;

	if (nr >= dev->ports || nr < 0)
		return -EINVAL;

	ctrl = ip_phy_read(state, nr, 0);
	if (ctrl < 0)
		return -EIO;

	ctrl &= (~(1<<12));
	ctrl &= (~(1<<13));
	ctrl |= (autoneg<<12);
	ctrl |= (speed<<13);

	return ip_phy_write(state, nr, 0, ctrl);
}

static int ip17xx_get_port_speed(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int nr = val->port_vlan;
	int speed, status;

	if (nr == state->regs->CPU_PORT) {
		val->value.i = 100;
		return 0;
	}

	if (nr >= dev->ports || nr < 0)
		return -EINVAL;

	status = ip_phy_read(state, nr, 1);
	speed = ip_phy_read(state, nr, 18);
	if (status < 0 || speed < 0)
		return -EIO;

	if (status & 4)
		val->value.i = ((speed & (1<<11)) ? 100 : 10);
	else
		val->value.i = 0;

	return 0;
}

static int ip17xx_get_port_status(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct ip17xx_state *state = get_state(dev);
	int ctrl, speed, status;
	int nr = val->port_vlan;
	int len;
	char *buf = state->buf; // fixed-length at 80.

	if (nr == state->regs->CPU_PORT) {
		sprintf(buf, "up, 100 Mbps, cpu port");
		val->value.s = buf;
		return 0;
	}

	if (nr >= dev->ports || nr < 0)
		return -EINVAL;

	ctrl = ip_phy_read(state, nr, 0);
	status = ip_phy_read(state, nr, 1);
	speed = ip_phy_read(state, nr, 18);
	if (ctrl < 0 || status < 0 || speed < 0)
		return -EIO;

	if (status & 4)
		len = sprintf(buf, "up, %d Mbps, %s duplex",
			((speed & (1<<11)) ? 100 : 10),
			((speed & (1<<10)) ? "full" : "half"));
	else
		len = sprintf(buf, "down");

	if (ctrl & (1<<12)) {
		len += sprintf(buf+len, ", auto-negotiate");
		if (!(status & (1<<5)))
			len += sprintf(buf+len, " (in progress)");
	} else {
		len += sprintf(buf+len, ", fixed speed (%d)",
			((ctrl & (1<<13)) ? 100 : 10));
	}

	buf[len] = '\0';
	val->value.s = buf;
	return 0;
}

static int ip17xx_get_pvid(struct switch_dev *dev, int port, int *val)
{
	struct ip17xx_state *state = get_state(dev);

	*val = state->ports[port].pvid;
	return 0;
}

static int ip17xx_set_pvid(struct switch_dev *dev, int port, int val)
{
	struct ip17xx_state *state = get_state(dev);

	if (val < 0 || val >= MAX_VLANS)
		return -EINVAL;

	state->ports[port].pvid = val;
	return state->regs->update_state(state);
}


enum Ports {
	IP17XX_PORT_STATUS,
	IP17XX_PORT_LINK,
	IP17XX_PORT_TAGGED,
	IP17XX_PORT_PVID,
};

enum Globals {
	IP17XX_ENABLE_VLAN,
	IP17XX_GET_NAME,
	IP17XX_REGISTER_PHY,
	IP17XX_REGISTER_MII,
	IP17XX_REGISTER_VALUE,
	IP17XX_REGISTER_ERRNO,
};

enum Vlans {
	IP17XX_VLAN_TAG,
};

static const struct switch_attr ip17xx_global[] = {
	[IP17XX_ENABLE_VLAN] = {
		.id = IP17XX_ENABLE_VLAN,
		.type = SWITCH_TYPE_INT,
		.name  = "enable_vlan",
		.description = "Flag to enable or disable VLANs and tagging",
		.get  = ip17xx_get_enable_vlan,
		.set = ip17xx_set_enable_vlan,
	},
	[IP17XX_GET_NAME] = {
		.id = IP17XX_GET_NAME,
		.type = SWITCH_TYPE_STRING,
		.description = "Returns the type of IC+ chip.",
		.name  = "name",
		.get  = ip17xx_read_name,
		.set = NULL,
	},
	/* jal: added for low level debugging etc. */
	[IP17XX_REGISTER_PHY] = {
		.id = IP17XX_REGISTER_PHY,
		.type = SWITCH_TYPE_INT,
		.description = "Direct register access: set PHY (0-4, or 29,30,31)",
		.name  = "phy",
		.get  = ip17xx_get_phy,
		.set = ip17xx_set_phy,
	},
	[IP17XX_REGISTER_MII] = {
		.id = IP17XX_REGISTER_MII,
		.type = SWITCH_TYPE_INT,
		.description = "Direct register access: set MII register number (0-31)",
		.name  = "reg",
		.get  = ip17xx_get_reg,
		.set = ip17xx_set_reg,
	},
	[IP17XX_REGISTER_VALUE] = {
		.id = IP17XX_REGISTER_VALUE,
		.type = SWITCH_TYPE_INT,
		.description = "Direct register access: read/write to register (0-65535)",
		.name  = "val",
		.get  = ip17xx_get_val,
		.set = ip17xx_set_val,
	},
};

static const struct switch_attr ip17xx_vlan[] = {
	[IP17XX_VLAN_TAG] = {
		.id = IP17XX_VLAN_TAG,
		.type = SWITCH_TYPE_INT,
		.description = "VLAN ID (0-4095) [IP175D only]",
		.name = "vid",
		.get = ip17xx_get_tag,
		.set = ip17xx_set_tag,
	}
};

static const struct switch_attr ip17xx_port[] = {
	[IP17XX_PORT_STATUS] = {
		.id = IP17XX_PORT_STATUS,
		.type = SWITCH_TYPE_STRING,
		.description = "Returns Detailed port status",
		.name  = "status",
		.get  = ip17xx_get_port_status,
		.set = NULL,
	},
	[IP17XX_PORT_LINK] = {
		.id = IP17XX_PORT_LINK,
		.type = SWITCH_TYPE_INT,
		.description = "Link speed. Can write 0 for auto-negotiate, or 10 or 100",
		.name  = "link",
		.get  = ip17xx_get_port_speed,
		.set = ip17xx_set_port_speed,
	},
	[IP17XX_PORT_TAGGED] = {
		.id = IP17XX_PORT_LINK,
		.type = SWITCH_TYPE_INT,
		.description = "0 = untag, 1 = add tags, 2 = do not alter (This value is reset if vlans are altered)",
		.name  = "tagged",
		.get  = ip17xx_get_tagged,
		.set = ip17xx_set_tagged,
	},
};

static const struct switch_dev_ops ip17xx_ops = {
	.attr_global = {
		.attr = ip17xx_global,
		.n_attr = ARRAY_SIZE(ip17xx_global),
	},
	.attr_port = {
		.attr = ip17xx_port,
		.n_attr = ARRAY_SIZE(ip17xx_port),
	},
	.attr_vlan = {
		.attr = ip17xx_vlan,
		.n_attr = ARRAY_SIZE(ip17xx_vlan),
	},

	.get_port_pvid = ip17xx_get_pvid,
	.set_port_pvid = ip17xx_set_pvid,
	.get_vlan_ports = ip17xx_get_ports,
	.set_vlan_ports = ip17xx_set_ports,
	.apply_config = ip17xx_apply,
	.reset_switch = ip17xx_reset,
};

static int ip17xx_probe(struct phy_device *pdev)
{
	struct ip17xx_state *state;
	struct switch_dev *dev;
	int err;

	/* We only attach to PHY 0, but use all available PHYs */
	if (pdev->mdio.addr != 0)
		return -ENODEV;

	state = kzalloc(sizeof(*state), GFP_KERNEL);
	if (!state)
		return -ENOMEM;

	dev = &state->dev;

	pdev->priv = state;
	state->mii_bus = pdev->mdio.bus;

	err = get_model(state);
	if (err < 0)
		goto error;

	dev->vlans = MAX_VLANS;
	dev->cpu_port = state->regs->CPU_PORT;
	dev->ports = state->regs->NUM_PORTS;
	dev->name = state->regs->NAME;
	dev->ops = &ip17xx_ops;

	pr_info("IP17xx: Found %s at %s\n", dev->name, dev_name(&pdev->mdio.dev));
	return 0;

error:
	kfree(state);
	return err;
}

static int ip17xx_config_init(struct phy_device *pdev)
{
	struct ip17xx_state *state = pdev->priv;
	struct net_device *dev = pdev->attached_dev;
	int err;

	err = register_switch(&state->dev, dev);
	if (err < 0)
		return err;

	state->registered = true;
	ip17xx_reset(&state->dev);
	return 0;
}

static void ip17xx_remove(struct phy_device *pdev)
{
	struct ip17xx_state *state = pdev->priv;

	if (state->registered)
		unregister_switch(&state->dev);
	kfree(state);
}

static int ip17xx_config_aneg(struct phy_device *pdev)
{
	return 0;
}

static int ip17xx_aneg_done(struct phy_device *pdev)
{
	return 1;	/* Return any positive value */
}

static int ip17xx_read_status(struct phy_device *pdev)
{
	pdev->speed = SPEED_100;
	pdev->duplex = DUPLEX_FULL;
	pdev->pause = pdev->asym_pause = 0;
	pdev->link = 1;

	return 0;
}

static struct phy_driver ip17xx_driver[] = {
	{
		.name		= "IC+ IP17xx",
		.phy_id		= 0x02430c00,
		.phy_id_mask	= 0x0ffffc00,
		.features	= PHY_BASIC_FEATURES,
		.probe		= ip17xx_probe,
		.remove		= ip17xx_remove,
		.config_init	= ip17xx_config_init,
		.config_aneg	= ip17xx_config_aneg,
		.aneg_done	= ip17xx_aneg_done,
		.read_status	= ip17xx_read_status,
	}
};

module_phy_driver(ip17xx_driver);

MODULE_AUTHOR("Patrick Horn <patrick.horn@gmail.com>");
MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_AUTHOR("Martin Mares <mj@ucw.cz>");
MODULE_LICENSE("GPL");
