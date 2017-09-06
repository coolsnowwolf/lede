/*
 * ADM6996 switch driver
 *
 * Copyright (c) 2008 Felix Fietkau <nbd@nbd.name>
 * Copyright (c) 2010,2011 Peter Lebbing <peter@digitalbrains.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation
 */
#ifndef __ADM6996_H
#define __ADM6996_H

/*
 * ADM_PHY_PORTS: Number of ports with a PHY.
 * We only control ports 0 to 3, because if 4 is connected, it is most likely
 * not connected to the switch but to a separate MII and MAC for the WAN port.
 */
#define ADM_PHY_PORTS	4
#define ADM_NUM_PORTS	6
#define ADM_CPU_PORT	5

#define ADM_NUM_VLANS 16
#define ADM_VLAN_MAX_ID 4094

enum admreg {
	ADM_EEPROM_BASE		= 0x0,
		ADM_P0_CFG		= ADM_EEPROM_BASE + 1,
		ADM_P1_CFG		= ADM_EEPROM_BASE + 3,
		ADM_P2_CFG		= ADM_EEPROM_BASE + 5,
		ADM_P3_CFG		= ADM_EEPROM_BASE + 7,
		ADM_P4_CFG		= ADM_EEPROM_BASE + 8,
		ADM_P5_CFG		= ADM_EEPROM_BASE + 9,
		ADM_SYSC0		= ADM_EEPROM_BASE + 0xa,
		ADM_VLAN_PRIOMAP	= ADM_EEPROM_BASE + 0xe,
		ADM_SYSC3		= ADM_EEPROM_BASE + 0x11,
		/* Input Force No Tag Enable */
		ADM_IFNTE		= ADM_EEPROM_BASE + 0x20,
		ADM_VID_CHECK		= ADM_EEPROM_BASE + 0x26,
		ADM_P0_PVID		= ADM_EEPROM_BASE + 0x28,
		ADM_P1_PVID		= ADM_EEPROM_BASE + 0x29,
		/* Output Tag Bypass Enable and P2 PVID */
		ADM_OTBE_P2_PVID	= ADM_EEPROM_BASE + 0x2a,
		ADM_P3_P4_PVID		= ADM_EEPROM_BASE + 0x2b,
		ADM_P5_PVID		= ADM_EEPROM_BASE + 0x2c,
	ADM_EEPROM_EXT_BASE	= 0x40,
#define ADM_VLAN_FILT_L(n) (ADM_EEPROM_EXT_BASE + 2 * (n))
#define ADM_VLAN_FILT_H(n) (ADM_EEPROM_EXT_BASE + 1 + 2 * (n))
#define ADM_VLAN_MAP(n) (ADM_EEPROM_BASE + 0x13 + n)
	ADM_COUNTER_BASE	= 0xa0,
		ADM_SIG0		= ADM_COUNTER_BASE + 0,
		ADM_SIG1		= ADM_COUNTER_BASE + 1,
		ADM_PS0		= ADM_COUNTER_BASE + 2,
		ADM_PS1		= ADM_COUNTER_BASE + 3,
		ADM_PS2		= ADM_COUNTER_BASE + 4,
		ADM_CL0		= ADM_COUNTER_BASE + 8, /* RxPacket */
		ADM_CL6		= ADM_COUNTER_BASE + 0x1a, /* RxByte */
		ADM_CL12		= ADM_COUNTER_BASE + 0x2c, /* TxPacket */
		ADM_CL18		= ADM_COUNTER_BASE + 0x3e, /* TxByte */
		ADM_CL24		= ADM_COUNTER_BASE + 0x50, /* Coll */
		ADM_CL30		= ADM_COUNTER_BASE + 0x62, /* Err */
#define ADM_OFFSET_PORT(n) ((n * 4) - (n / 4) * 2 - (n / 5) * 2)
	ADM_PHY_BASE		= 0x200,
#define ADM_PHY_PORT(n) (ADM_PHY_BASE + (0x20 * n))
};

/* Chip identification patterns */
#define	ADM_SIG0_MASK	0xffff
#define ADM_SIG0_VAL	0x1023
#define ADM_SIG1_MASK	0xffff
#define ADM_SIG1_VAL	0x0007

enum {
	ADM_PHYCFG_COLTST     = (1 << 7),	/* Enable collision test */
	ADM_PHYCFG_DPLX       = (1 << 8),	/* Enable full duplex */
	ADM_PHYCFG_ANEN_RST   = (1 << 9),	/* Restart auto negotiation (self clear) */
	ADM_PHYCFG_ISO        = (1 << 10),	/* Isolate PHY */
	ADM_PHYCFG_PDN        = (1 << 11),	/* Power down PHY */
	ADM_PHYCFG_ANEN       = (1 << 12),	/* Enable auto negotiation */
	ADM_PHYCFG_SPEED_100  = (1 << 13),	/* Enable 100 Mbit/s */
	ADM_PHYCFG_LPBK       = (1 << 14),	/* Enable loopback operation */
	ADM_PHYCFG_RST        = (1 << 15),	/* Reset the port (self clear) */
	ADM_PHYCFG_INIT = (
		ADM_PHYCFG_RST |
		ADM_PHYCFG_SPEED_100 |
		ADM_PHYCFG_ANEN |
		ADM_PHYCFG_ANEN_RST
	)
};

enum {
	ADM_PORTCFG_FC        = (1 << 0),	/* Enable 802.x flow control */
	ADM_PORTCFG_AN        = (1 << 1),	/* Enable auto-negotiation */
	ADM_PORTCFG_SPEED_100 = (1 << 2),	/* Enable 100 Mbit/s */
	ADM_PORTCFG_DPLX      = (1 << 3),	/* Enable full duplex */
	ADM_PORTCFG_OT        = (1 << 4),	/* Output tagged packets */
	ADM_PORTCFG_PD        = (1 << 5),	/* Port disable */
	ADM_PORTCFG_TV_PRIO   = (1 << 6),	/* 0 = VLAN based priority
	                                 	 * 1 = TOS based priority */
	ADM_PORTCFG_PPE       = (1 << 7),	/* Port based priority enable */
	ADM_PORTCFG_PP_S      = (1 << 8),	/* Port based priority, 2 bits */
	ADM_PORTCFG_PVID_BASE = (1 << 10),	/* Primary VLAN id, 4 bits */
	ADM_PORTCFG_FSE	      = (1 << 14),	/* Fx select enable */
	ADM_PORTCFG_CAM       = (1 << 15),	/* Crossover Auto MDIX */

	ADM_PORTCFG_INIT = (
		ADM_PORTCFG_FC |
		ADM_PORTCFG_AN |
		ADM_PORTCFG_SPEED_100 |
		ADM_PORTCFG_DPLX |
		ADM_PORTCFG_CAM
	),
	ADM_PORTCFG_CPU = (
		ADM_PORTCFG_FC |
		ADM_PORTCFG_SPEED_100 |
		ADM_PORTCFG_OT |
		ADM_PORTCFG_DPLX
	),
};

#define ADM_PORTCFG_PPID(n) ((n & 0x3) << 8)
#define ADM_PORTCFG_PVID(n) ((n & 0xf) << 10)
#define ADM_PORTCFG_PVID_MASK (0xf << 10)

#define ADM_IFNTE_MASK (0x3f << 9)
#define ADM_VID_CHECK_MASK (0x3f << 6)

#define ADM_P0_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 0)
#define ADM_P1_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 0)
#define ADM_P2_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 0)
#define ADM_P3_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 0)
#define ADM_P4_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 8)
#define ADM_P5_PVID_VAL(n) ((((n) & 0xff0) >> 4) << 0)
#define ADM_P2_PVID_MASK 0xff

#define ADM_OTBE(n) (((n) & 0x3f) << 8)
#define ADM_OTBE_MASK (0x3f << 8)

/* ADM_SYSC0 */
enum {
	ADM_NTTE	= (1 << 2),	/* New Tag Transmit Enable */
	ADM_RVID1	= (1 << 8)	/* Replace VLAN ID 1 */
};

/* Tag Based VLAN in ADM_SYSC3 */
#define ADM_MAC_CLONE	BIT(4)
#define ADM_TBV		BIT(5)

static const u8 adm_portcfg[] = {
	[0] = ADM_P0_CFG,
	[1] = ADM_P1_CFG,
	[2] = ADM_P2_CFG,
	[3] = ADM_P3_CFG,
	[4] = ADM_P4_CFG,
	[5] = ADM_P5_CFG,
};

/* Fields in ADM_VLAN_FILT_L(x) */
#define ADM_VLAN_FILT_FID(n) (((n) & 0xf) << 12)
#define ADM_VLAN_FILT_TAGGED(n) (((n) & 0x3f) << 6)
#define ADM_VLAN_FILT_MEMBER(n) (((n) & 0x3f) << 0)
#define ADM_VLAN_FILT_MEMBER_MASK 0x3f
/* Fields in ADM_VLAN_FILT_H(x) */
#define ADM_VLAN_FILT_VALID (1 << 15)
#define ADM_VLAN_FILT_VID(n) (((n) & 0xfff) << 0)

/* Convert ports to a form for ADM6996L VLAN map */
#define ADM_VLAN_FILT(ports) ((ports & 0x01) | ((ports & 0x02) << 1) | \
			((ports & 0x04) << 2) | ((ports & 0x08) << 3) | \
			((ports & 0x10) << 3) | ((ports & 0x20) << 3))

/* Port status register */
enum {
	ADM_PS_LS = (1 << 0),	/* Link status */
	ADM_PS_SS = (1 << 1),	/* Speed status */
	ADM_PS_DS = (1 << 2),	/* Duplex status */
	ADM_PS_FCS = (1 << 3)	/* Flow control status */
};

/*
 * Split the register address in phy id and register
 * it will get combined again by the mdio bus op
 */
#define PHYADDR(_reg)	((_reg >> 5) & 0xff), (_reg & 0x1f)

#endif
