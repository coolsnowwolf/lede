/*
 * ar8216.h: AR8216 switch driver
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
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

#ifndef __AR8216_H
#define __AR8216_H

#define BITS(_s, _n)	(((1UL << (_n)) - 1) << _s)

#define AR8XXX_CAP_GIGE			BIT(0)
#define AR8XXX_CAP_MIB_COUNTERS		BIT(1)

#define AR8XXX_NUM_PHYS 	5
#define AR8216_PORT_CPU	0
#define AR8216_NUM_PORTS	6
#define AR8216_NUM_VLANS	16
#define AR7240SW_NUM_PORTS	5
#define AR8316_NUM_VLANS	4096

/* size of the vlan table */
#define AR8X16_MAX_VLANS	128
#define AR83X7_MAX_VLANS	4096
#define AR8XXX_MAX_VLANS	AR83X7_MAX_VLANS

#define AR8X16_PROBE_RETRIES	10
#define AR8X16_MAX_PORTS	8

#define AR8XXX_REG_ARL_CTRL_AGE_TIME_SECS	7
#define AR8XXX_DEFAULT_ARL_AGE_TIME		300

/* Atheros specific MII registers */
#define MII_ATH_MMD_ADDR		0x0d
#define MII_ATH_MMD_DATA		0x0e
#define MII_ATH_DBG_ADDR		0x1d
#define MII_ATH_DBG_DATA		0x1e

#define AR8216_REG_CTRL			0x0000
#define   AR8216_CTRL_REVISION		BITS(0, 8)
#define   AR8216_CTRL_REVISION_S	0
#define   AR8216_CTRL_VERSION		BITS(8, 8)
#define   AR8216_CTRL_VERSION_S		8
#define   AR8216_CTRL_RESET		BIT(31)

#define AR8216_REG_FLOOD_MASK		0x002C
#define   AR8216_FM_UNI_DEST_PORTS	BITS(0, 6)
#define   AR8216_FM_MULTI_DEST_PORTS	BITS(16, 6)
#define   AR8216_FM_CPU_BROADCAST_EN	BIT(26)
#define   AR8229_FLOOD_MASK_UC_DP(_p)	BIT(_p)
#define   AR8229_FLOOD_MASK_MC_DP(_p)	BIT(16 + (_p))
#define   AR8229_FLOOD_MASK_BC_DP(_p)	BIT(25 + (_p))

#define AR8216_REG_GLOBAL_CTRL		0x0030
#define   AR8216_GCTRL_MTU		BITS(0, 11)
#define   AR8236_GCTRL_MTU		BITS(0, 14)
#define   AR8316_GCTRL_MTU		BITS(0, 14)

#define AR8216_REG_VTU			0x0040
#define   AR8216_VTU_OP			BITS(0, 3)
#define   AR8216_VTU_OP_NOOP		0x0
#define   AR8216_VTU_OP_FLUSH		0x1
#define   AR8216_VTU_OP_LOAD		0x2
#define   AR8216_VTU_OP_PURGE		0x3
#define   AR8216_VTU_OP_REMOVE_PORT	0x4
#define   AR8216_VTU_ACTIVE		BIT(3)
#define   AR8216_VTU_FULL		BIT(4)
#define   AR8216_VTU_PORT		BITS(8, 4)
#define   AR8216_VTU_PORT_S		8
#define   AR8216_VTU_VID		BITS(16, 12)
#define   AR8216_VTU_VID_S		16
#define   AR8216_VTU_PRIO		BITS(28, 3)
#define   AR8216_VTU_PRIO_S		28
#define   AR8216_VTU_PRIO_EN		BIT(31)

#define AR8216_REG_VTU_DATA		0x0044
#define   AR8216_VTUDATA_MEMBER		BITS(0, 10)
#define   AR8236_VTUDATA_MEMBER		BITS(0, 7)
#define   AR8216_VTUDATA_VALID		BIT(11)

#define AR8216_REG_ATU_FUNC0		0x0050
#define   AR8216_ATU_OP			BITS(0, 3)
#define   AR8216_ATU_OP_NOOP		0x0
#define   AR8216_ATU_OP_FLUSH		0x1
#define   AR8216_ATU_OP_LOAD		0x2
#define   AR8216_ATU_OP_PURGE		0x3
#define   AR8216_ATU_OP_FLUSH_UNLOCKED	0x4
#define   AR8216_ATU_OP_FLUSH_PORT	0x5
#define   AR8216_ATU_OP_GET_NEXT	0x6
#define   AR8216_ATU_ACTIVE		BIT(3)
#define   AR8216_ATU_PORT_NUM		BITS(8, 4)
#define   AR8216_ATU_PORT_NUM_S		8
#define   AR8216_ATU_FULL_VIO		BIT(12)
#define   AR8216_ATU_ADDR5		BITS(16, 8)
#define   AR8216_ATU_ADDR5_S		16
#define   AR8216_ATU_ADDR4		BITS(24, 8)
#define   AR8216_ATU_ADDR4_S		24

#define AR8216_REG_ATU_FUNC1		0x0054
#define   AR8216_ATU_ADDR3		BITS(0, 8)
#define   AR8216_ATU_ADDR3_S		0
#define   AR8216_ATU_ADDR2		BITS(8, 8)
#define   AR8216_ATU_ADDR2_S		8
#define   AR8216_ATU_ADDR1		BITS(16, 8)
#define   AR8216_ATU_ADDR1_S		16
#define   AR8216_ATU_ADDR0		BITS(24, 8)
#define   AR8216_ATU_ADDR0_S		24

#define AR8216_REG_ATU_FUNC2		0x0058
#define   AR8216_ATU_PORTS		BITS(0, 6)
#define   AR8216_ATU_PORTS_S		0
#define   AR8216_ATU_PORT0		BIT(0)
#define   AR8216_ATU_PORT1		BIT(1)
#define   AR8216_ATU_PORT2		BIT(2)
#define   AR8216_ATU_PORT3		BIT(3)
#define   AR8216_ATU_PORT4		BIT(4)
#define   AR8216_ATU_PORT5		BIT(5)
#define   AR8216_ATU_STATUS		BITS(16, 4)
#define   AR8216_ATU_STATUS_S		16

#define AR8216_REG_ATU_CTRL		0x005C
#define   AR8216_ATU_CTRL_AGE_EN	BIT(17)
#define   AR8216_ATU_CTRL_AGE_TIME	BITS(0, 16)
#define   AR8216_ATU_CTRL_AGE_TIME_S	0
#define   AR8236_ATU_CTRL_RES		BIT(20)
#define   AR8216_ATU_CTRL_LEARN_CHANGE	BIT(18)
#define   AR8216_ATU_CTRL_RESERVED	BIT(19)
#define   AR8216_ATU_CTRL_ARP_EN	BIT(20)

#define AR8216_REG_TAG_PRIORITY	0x0070

#define AR8216_REG_SERVICE_TAG		0x0074
#define  AR8216_SERVICE_TAG_M		BITS(0, 16)

#define AR8216_REG_MIB_FUNC		0x0080
#define   AR8216_MIB_TIMER		BITS(0, 16)
#define   AR8216_MIB_AT_HALF_EN		BIT(16)
#define   AR8216_MIB_BUSY		BIT(17)
#define   AR8216_MIB_FUNC		BITS(24, 3)
#define   AR8216_MIB_FUNC_S		24
#define   AR8216_MIB_FUNC_NO_OP		0x0
#define   AR8216_MIB_FUNC_FLUSH		0x1
#define   AR8216_MIB_FUNC_CAPTURE	0x3
#define   AR8236_MIB_EN			BIT(30)

#define AR8216_REG_GLOBAL_CPUPORT		0x0078
#define   AR8216_GLOBAL_CPUPORT_MIRROR_PORT	BITS(4, 4)
#define   AR8216_GLOBAL_CPUPORT_MIRROR_PORT_S	4
#define   AR8216_GLOBAL_CPUPORT_EN		BIT(8)

#define AR8216_REG_MDIO_CTRL		0x98
#define   AR8216_MDIO_CTRL_DATA_M	BITS(0, 16)
#define   AR8216_MDIO_CTRL_REG_ADDR_S	16
#define   AR8216_MDIO_CTRL_PHY_ADDR_S	21
#define   AR8216_MDIO_CTRL_CMD_WRITE	0
#define   AR8216_MDIO_CTRL_CMD_READ	BIT(27)
#define   AR8216_MDIO_CTRL_MASTER_EN	BIT(30)
#define   AR8216_MDIO_CTRL_BUSY	BIT(31)

#define AR8216_PORT_OFFSET(_i)		(0x0100 * (_i + 1))
#define AR8216_REG_PORT_STATUS(_i)	(AR8216_PORT_OFFSET(_i) + 0x0000)
#define   AR8216_PORT_STATUS_SPEED	BITS(0,2)
#define   AR8216_PORT_STATUS_SPEED_S	0
#define   AR8216_PORT_STATUS_TXMAC	BIT(2)
#define   AR8216_PORT_STATUS_RXMAC	BIT(3)
#define   AR8216_PORT_STATUS_TXFLOW	BIT(4)
#define   AR8216_PORT_STATUS_RXFLOW	BIT(5)
#define   AR8216_PORT_STATUS_DUPLEX	BIT(6)
#define   AR8216_PORT_STATUS_LINK_UP	BIT(8)
#define   AR8216_PORT_STATUS_LINK_AUTO	BIT(9)
#define   AR8216_PORT_STATUS_LINK_PAUSE	BIT(10)
#define   AR8216_PORT_STATUS_FLOW_CONTROL  BIT(12)

#define AR8216_REG_PORT_CTRL(_i)	(AR8216_PORT_OFFSET(_i) + 0x0004)

/* port forwarding state */
#define   AR8216_PORT_CTRL_STATE	BITS(0, 3)
#define   AR8216_PORT_CTRL_STATE_S	0

#define   AR8216_PORT_CTRL_LEARN_LOCK	BIT(7)

/* egress 802.1q mode */
#define   AR8216_PORT_CTRL_VLAN_MODE	BITS(8, 2)
#define   AR8216_PORT_CTRL_VLAN_MODE_S	8

#define   AR8216_PORT_CTRL_IGMP_SNOOP	BIT(10)
#define   AR8216_PORT_CTRL_HEADER	BIT(11)
#define   AR8216_PORT_CTRL_MAC_LOOP	BIT(12)
#define   AR8216_PORT_CTRL_SINGLE_VLAN	BIT(13)
#define   AR8216_PORT_CTRL_LEARN	BIT(14)
#define   AR8216_PORT_CTRL_MIRROR_TX	BIT(16)
#define   AR8216_PORT_CTRL_MIRROR_RX	BIT(17)

#define AR8216_REG_PORT_VLAN(_i)	(AR8216_PORT_OFFSET(_i) + 0x0008)

#define   AR8216_PORT_VLAN_DEFAULT_ID	BITS(0, 12)
#define   AR8216_PORT_VLAN_DEFAULT_ID_S	0

#define   AR8216_PORT_VLAN_DEST_PORTS	BITS(16, 9)
#define   AR8216_PORT_VLAN_DEST_PORTS_S	16

/* bit0 added to the priority field of egress frames */
#define   AR8216_PORT_VLAN_TX_PRIO	BIT(27)

/* port default priority */
#define   AR8216_PORT_VLAN_PRIORITY	BITS(28, 2)
#define   AR8216_PORT_VLAN_PRIORITY_S	28

/* ingress 802.1q mode */
#define   AR8216_PORT_VLAN_MODE		BITS(30, 2)
#define   AR8216_PORT_VLAN_MODE_S	30

#define AR8216_REG_PORT_RATE(_i)	(AR8216_PORT_OFFSET(_i) + 0x000c)
#define AR8216_REG_PORT_PRIO(_i)	(AR8216_PORT_OFFSET(_i) + 0x0010)

#define AR8216_STATS_RXBROAD		0x00
#define AR8216_STATS_RXPAUSE		0x04
#define AR8216_STATS_RXMULTI		0x08
#define AR8216_STATS_RXFCSERR		0x0c
#define AR8216_STATS_RXALIGNERR		0x10
#define AR8216_STATS_RXRUNT		0x14
#define AR8216_STATS_RXFRAGMENT		0x18
#define AR8216_STATS_RX64BYTE		0x1c
#define AR8216_STATS_RX128BYTE		0x20
#define AR8216_STATS_RX256BYTE		0x24
#define AR8216_STATS_RX512BYTE		0x28
#define AR8216_STATS_RX1024BYTE		0x2c
#define AR8216_STATS_RXMAXBYTE		0x30
#define AR8216_STATS_RXTOOLONG		0x34
#define AR8216_STATS_RXGOODBYTE		0x38
#define AR8216_STATS_RXBADBYTE		0x40
#define AR8216_STATS_RXOVERFLOW		0x48
#define AR8216_STATS_FILTERED		0x4c
#define AR8216_STATS_TXBROAD		0x50
#define AR8216_STATS_TXPAUSE		0x54
#define AR8216_STATS_TXMULTI		0x58
#define AR8216_STATS_TXUNDERRUN		0x5c
#define AR8216_STATS_TX64BYTE		0x60
#define AR8216_STATS_TX128BYTE		0x64
#define AR8216_STATS_TX256BYTE		0x68
#define AR8216_STATS_TX512BYTE		0x6c
#define AR8216_STATS_TX1024BYTE		0x70
#define AR8216_STATS_TXMAXBYTE		0x74
#define AR8216_STATS_TXOVERSIZE		0x78
#define AR8216_STATS_TXBYTE		0x7c
#define AR8216_STATS_TXCOLLISION	0x84
#define AR8216_STATS_TXABORTCOL		0x88
#define AR8216_STATS_TXMULTICOL		0x8c
#define AR8216_STATS_TXSINGLECOL	0x90
#define AR8216_STATS_TXEXCDEFER		0x94
#define AR8216_STATS_TXDEFER		0x98
#define AR8216_STATS_TXLATECOL		0x9c

#define AR8216_MIB_RXB_ID		14	/* RxGoodByte */
#define AR8216_MIB_TXB_ID		29	/* TxByte */

#define AR8229_REG_OPER_MODE0		0x04
#define   AR8229_OPER_MODE0_MAC_GMII_EN	BIT(6)
#define   AR8229_OPER_MODE0_PHY_MII_EN	BIT(10)

#define AR8229_REG_OPER_MODE1		0x08
#define   AR8229_REG_OPER_MODE1_PHY4_MII_EN	BIT(28)

#define AR8229_REG_QM_CTRL		0x3c
#define   AR8229_QM_CTRL_ARP_EN		BIT(15)

#define AR8236_REG_PORT_VLAN(_i)	(AR8216_PORT_OFFSET((_i)) + 0x0008)
#define   AR8236_PORT_VLAN_DEFAULT_ID	BITS(16, 12)
#define   AR8236_PORT_VLAN_DEFAULT_ID_S	16
#define   AR8236_PORT_VLAN_PRIORITY	BITS(29, 3)
#define   AR8236_PORT_VLAN_PRIORITY_S	28

#define AR8236_REG_PORT_VLAN2(_i)	(AR8216_PORT_OFFSET((_i)) + 0x000c)
#define   AR8236_PORT_VLAN2_MEMBER	BITS(16, 7)
#define   AR8236_PORT_VLAN2_MEMBER_S	16
#define   AR8236_PORT_VLAN2_TX_PRIO	BIT(23)
#define   AR8236_PORT_VLAN2_VLAN_MODE	BITS(30, 2)
#define   AR8236_PORT_VLAN2_VLAN_MODE_S	30

#define AR8236_STATS_RXBROAD		0x00
#define AR8236_STATS_RXPAUSE		0x04
#define AR8236_STATS_RXMULTI		0x08
#define AR8236_STATS_RXFCSERR		0x0c
#define AR8236_STATS_RXALIGNERR		0x10
#define AR8236_STATS_RXRUNT		0x14
#define AR8236_STATS_RXFRAGMENT		0x18
#define AR8236_STATS_RX64BYTE		0x1c
#define AR8236_STATS_RX128BYTE		0x20
#define AR8236_STATS_RX256BYTE		0x24
#define AR8236_STATS_RX512BYTE		0x28
#define AR8236_STATS_RX1024BYTE		0x2c
#define AR8236_STATS_RX1518BYTE		0x30
#define AR8236_STATS_RXMAXBYTE		0x34
#define AR8236_STATS_RXTOOLONG		0x38
#define AR8236_STATS_RXGOODBYTE		0x3c
#define AR8236_STATS_RXBADBYTE		0x44
#define AR8236_STATS_RXOVERFLOW		0x4c
#define AR8236_STATS_FILTERED		0x50
#define AR8236_STATS_TXBROAD		0x54
#define AR8236_STATS_TXPAUSE		0x58
#define AR8236_STATS_TXMULTI		0x5c
#define AR8236_STATS_TXUNDERRUN		0x60
#define AR8236_STATS_TX64BYTE		0x64
#define AR8236_STATS_TX128BYTE		0x68
#define AR8236_STATS_TX256BYTE		0x6c
#define AR8236_STATS_TX512BYTE		0x70
#define AR8236_STATS_TX1024BYTE		0x74
#define AR8236_STATS_TX1518BYTE		0x78
#define AR8236_STATS_TXMAXBYTE		0x7c
#define AR8236_STATS_TXOVERSIZE		0x80
#define AR8236_STATS_TXBYTE		0x84
#define AR8236_STATS_TXCOLLISION	0x8c
#define AR8236_STATS_TXABORTCOL		0x90
#define AR8236_STATS_TXMULTICOL		0x94
#define AR8236_STATS_TXSINGLECOL	0x98
#define AR8236_STATS_TXEXCDEFER		0x9c
#define AR8236_STATS_TXDEFER		0xa0
#define AR8236_STATS_TXLATECOL		0xa4

#define AR8236_MIB_RXB_ID		15	/* RxGoodByte */
#define AR8236_MIB_TXB_ID		31	/* TxByte */

#define AR8316_REG_POSTRIP			0x0008
#define   AR8316_POSTRIP_MAC0_GMII_EN		BIT(0)
#define   AR8316_POSTRIP_MAC0_RGMII_EN		BIT(1)
#define   AR8316_POSTRIP_PHY4_GMII_EN		BIT(2)
#define   AR8316_POSTRIP_PHY4_RGMII_EN		BIT(3)
#define   AR8316_POSTRIP_MAC0_MAC_MODE		BIT(4)
#define   AR8316_POSTRIP_RTL_MODE		BIT(5)
#define   AR8316_POSTRIP_RGMII_RXCLK_DELAY_EN	BIT(6)
#define   AR8316_POSTRIP_RGMII_TXCLK_DELAY_EN	BIT(7)
#define   AR8316_POSTRIP_SERDES_EN		BIT(8)
#define   AR8316_POSTRIP_SEL_ANA_RST		BIT(9)
#define   AR8316_POSTRIP_GATE_25M_EN		BIT(10)
#define   AR8316_POSTRIP_SEL_CLK25M		BIT(11)
#define   AR8316_POSTRIP_HIB_PULSE_HW		BIT(12)
#define   AR8316_POSTRIP_DBG_MODE_I		BIT(13)
#define   AR8316_POSTRIP_MAC5_MAC_MODE		BIT(14)
#define   AR8316_POSTRIP_MAC5_PHY_MODE		BIT(15)
#define   AR8316_POSTRIP_POWER_DOWN_HW		BIT(16)
#define   AR8316_POSTRIP_LPW_STATE_EN		BIT(17)
#define   AR8316_POSTRIP_MAN_EN			BIT(18)
#define   AR8316_POSTRIP_PHY_PLL_ON		BIT(19)
#define   AR8316_POSTRIP_LPW_EXIT		BIT(20)
#define   AR8316_POSTRIP_TXDELAY_S0		BIT(21)
#define   AR8316_POSTRIP_TXDELAY_S1		BIT(22)
#define   AR8316_POSTRIP_RXDELAY_S0		BIT(23)
#define   AR8316_POSTRIP_LED_OPEN_EN		BIT(24)
#define   AR8316_POSTRIP_SPI_EN			BIT(25)
#define   AR8316_POSTRIP_RXDELAY_S1		BIT(26)
#define   AR8316_POSTRIP_POWER_ON_SEL		BIT(31)

/* port speed */
enum {
        AR8216_PORT_SPEED_10M = 0,
        AR8216_PORT_SPEED_100M = 1,
        AR8216_PORT_SPEED_1000M = 2,
        AR8216_PORT_SPEED_ERR = 3,
};

/* ingress 802.1q mode */
enum {
	AR8216_IN_PORT_ONLY = 0,
	AR8216_IN_PORT_FALLBACK = 1,
	AR8216_IN_VLAN_ONLY = 2,
	AR8216_IN_SECURE = 3
};

/* egress 802.1q mode */
enum {
	AR8216_OUT_KEEP = 0,
	AR8216_OUT_STRIP_VLAN = 1,
	AR8216_OUT_ADD_VLAN = 2
};

/* port forwarding state */
enum {
	AR8216_PORT_STATE_DISABLED = 0,
	AR8216_PORT_STATE_BLOCK = 1,
	AR8216_PORT_STATE_LISTEN = 2,
	AR8216_PORT_STATE_LEARN = 3,
	AR8216_PORT_STATE_FORWARD = 4
};

/* mib counter type */
enum {
	AR8XXX_MIB_BASIC = 0,
	AR8XXX_MIB_EXTENDED = 1
};

enum {
	AR8XXX_VER_AR8216 = 0x01,
	AR8XXX_VER_AR8236 = 0x03,
	AR8XXX_VER_AR8316 = 0x10,
	AR8XXX_VER_AR8327 = 0x12,
	AR8XXX_VER_AR8337 = 0x13,
};

#define AR8XXX_NUM_ARL_RECORDS	100

enum arl_op {
	AR8XXX_ARL_INITIALIZE,
	AR8XXX_ARL_GET_NEXT
};

struct arl_entry {
	u16 portmap;
	u8 mac[6];
};

struct ar8xxx_priv;

struct ar8xxx_mib_desc {
	unsigned int size;
	unsigned int offset;
	const char *name;
	u8 type;
};

struct ar8xxx_chip {
	unsigned long caps;
	bool config_at_probe;
	bool mii_lo_first;

	/* parameters to calculate REG_PORT_STATS_BASE */
	unsigned reg_port_stats_start;
	unsigned reg_port_stats_length;

	unsigned reg_arl_ctrl;

	int (*hw_init)(struct ar8xxx_priv *priv);
	void (*cleanup)(struct ar8xxx_priv *priv);

	const char *name;
	int vlans;
	int ports;
	const struct switch_dev_ops *swops;

	void (*init_globals)(struct ar8xxx_priv *priv);
	void (*init_port)(struct ar8xxx_priv *priv, int port);
	void (*setup_port)(struct ar8xxx_priv *priv, int port, u32 members);
	u32 (*read_port_status)(struct ar8xxx_priv *priv, int port);
	u32 (*read_port_eee_status)(struct ar8xxx_priv *priv, int port);
	int (*atu_flush)(struct ar8xxx_priv *priv);
	int (*atu_flush_port)(struct ar8xxx_priv *priv, int port);
	void (*vtu_flush)(struct ar8xxx_priv *priv);
	void (*vtu_load_vlan)(struct ar8xxx_priv *priv, u32 vid, u32 port_mask);
	void (*phy_fixup)(struct ar8xxx_priv *priv, int phy);
	void (*set_mirror_regs)(struct ar8xxx_priv *priv);
	void (*get_arl_entry)(struct ar8xxx_priv *priv, struct arl_entry *a,
			      u32 *status, enum arl_op op);
	int (*sw_hw_apply)(struct switch_dev *dev);
	void (*phy_rgmii_set)(struct ar8xxx_priv *priv, struct phy_device *phydev);
	int (*phy_read)(struct ar8xxx_priv *priv, int addr, int regnum);
	int (*phy_write)(struct ar8xxx_priv *priv, int addr, int regnum, u16 val);

	const struct ar8xxx_mib_desc *mib_decs;
	unsigned num_mibs;
	unsigned mib_func;
	int mib_rxb_id;
	int mib_txb_id;
};

struct ar8xxx_priv {
	struct switch_dev dev;
	struct mii_bus *mii_bus;
	struct mii_bus *sw_mii_bus;
	struct phy_device *phy;
	struct device *pdev;

	int (*get_port_link)(unsigned port);

	const struct net_device_ops *ndo_old;
	struct net_device_ops ndo;
	struct mutex reg_mutex;
	u8 chip_ver;
	u8 chip_rev;
	const struct ar8xxx_chip *chip;
	void *chip_data;
	bool initialized;
	bool port4_phy;
	char buf[2048];
	struct arl_entry arl_table[AR8XXX_NUM_ARL_RECORDS];
	char arl_buf[AR8XXX_NUM_ARL_RECORDS * 32 + 256];
	bool link_up[AR8X16_MAX_PORTS];

	bool init;

	struct mutex mib_lock;
	struct delayed_work mib_work;
	u64 *mib_stats;
	u32 mib_poll_interval;
	u8 mib_type;

	struct list_head list;
	unsigned int use_count;

	/* all fields below are cleared on reset */
	bool vlan;

	u16 vlan_id[AR8XXX_MAX_VLANS];
	u8 vlan_table[AR8XXX_MAX_VLANS];
	u8 vlan_tagged;
	u16 pvid[AR8X16_MAX_PORTS];
	int arl_age_time;

	/* mirroring */
	bool mirror_rx;
	bool mirror_tx;
	int source_port;
	int monitor_port;
	u8 port_vlan_prio[AR8X16_MAX_PORTS];
};

u32
ar8xxx_mii_read32(struct ar8xxx_priv *priv, int phy_id, int regnum);
void
ar8xxx_mii_write32(struct ar8xxx_priv *priv, int phy_id, int regnum, u32 val);
u32
ar8xxx_read(struct ar8xxx_priv *priv, int reg);
void
ar8xxx_write(struct ar8xxx_priv *priv, int reg, u32 val);
u32
ar8xxx_rmw(struct ar8xxx_priv *priv, int reg, u32 mask, u32 val);

void
ar8xxx_phy_dbg_read(struct ar8xxx_priv *priv, int phy_addr,
		u16 dbg_addr, u16 *dbg_data);
void
ar8xxx_phy_dbg_write(struct ar8xxx_priv *priv, int phy_addr,
		     u16 dbg_addr, u16 dbg_data);
void
ar8xxx_phy_mmd_write(struct ar8xxx_priv *priv, int phy_addr, u16 addr, u16 reg, u16 data);
u16
ar8xxx_phy_mmd_read(struct ar8xxx_priv *priv, int phy_addr, u16 addr, u16 reg);
void
ar8xxx_phy_init(struct ar8xxx_priv *priv);
int
ar8xxx_sw_set_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val);
int
ar8xxx_sw_get_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		   struct switch_val *val);
int
ar8xxx_sw_set_reset_mibs(struct switch_dev *dev,
			 const struct switch_attr *attr,
			 struct switch_val *val);
int
ar8xxx_sw_set_mib_poll_interval(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_get_mib_poll_interval(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_set_mib_type(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_get_mib_type(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_set_mirror_rx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_get_mirror_rx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_set_mirror_tx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_get_mirror_tx_enable(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val);
int
ar8xxx_sw_set_mirror_monitor_port(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val);
int
ar8xxx_sw_get_mirror_monitor_port(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val);
int
ar8xxx_sw_set_mirror_source_port(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val);
int
ar8xxx_sw_get_mirror_source_port(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val);
int
ar8xxx_sw_set_pvid(struct switch_dev *dev, int port, int vlan);
int
ar8xxx_sw_get_pvid(struct switch_dev *dev, int port, int *vlan);
int
ar8xxx_sw_hw_apply(struct switch_dev *dev);
int
ar8xxx_sw_reset_switch(struct switch_dev *dev);
int
ar8xxx_sw_get_port_link(struct switch_dev *dev, int port,
			struct switch_port_link *link);
int
ar8xxx_sw_set_port_reset_mib(struct switch_dev *dev,
                             const struct switch_attr *attr,
                             struct switch_val *val);
int
ar8xxx_sw_get_port_mib(struct switch_dev *dev,
                       const struct switch_attr *attr,
                       struct switch_val *val);
int
ar8xxx_sw_get_arl_age_time(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val);
int
ar8xxx_sw_set_arl_age_time(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val);
int
ar8xxx_sw_get_arl_table(struct switch_dev *dev,
			const struct switch_attr *attr,
			struct switch_val *val);
int
ar8xxx_sw_set_flush_arl_table(struct switch_dev *dev,
			      const struct switch_attr *attr,
			      struct switch_val *val);
int
ar8xxx_sw_set_flush_port_arl_table(struct switch_dev *dev,
				   const struct switch_attr *attr,
				   struct switch_val *val);
int
ar8xxx_sw_get_port_stats(struct switch_dev *dev, int port,
			struct switch_port_stats *stats);
int
ar8216_wait_bit(struct ar8xxx_priv *priv, int reg, u32 mask, u32 val);

static inline struct ar8xxx_priv *
swdev_to_ar8xxx(struct switch_dev *swdev)
{
	return container_of(swdev, struct ar8xxx_priv, dev);
}

static inline bool ar8xxx_has_gige(struct ar8xxx_priv *priv)
{
	return priv->chip->caps & AR8XXX_CAP_GIGE;
}

static inline bool ar8xxx_has_mib_counters(struct ar8xxx_priv *priv)
{
	return priv->chip->caps & AR8XXX_CAP_MIB_COUNTERS;
}

static inline bool chip_is_ar8216(struct ar8xxx_priv *priv)
{
	return priv->chip_ver == AR8XXX_VER_AR8216;
}

static inline bool chip_is_ar8236(struct ar8xxx_priv *priv)
{
	return priv->chip_ver == AR8XXX_VER_AR8236;
}

static inline bool chip_is_ar8316(struct ar8xxx_priv *priv)
{
	return priv->chip_ver == AR8XXX_VER_AR8316;
}

static inline bool chip_is_ar8327(struct ar8xxx_priv *priv)
{
	return priv->chip_ver == AR8XXX_VER_AR8327;
}

static inline bool chip_is_ar8337(struct ar8xxx_priv *priv)
{
	return priv->chip_ver == AR8XXX_VER_AR8337;
}

static inline void
ar8xxx_reg_set(struct ar8xxx_priv *priv, int reg, u32 val)
{
	ar8xxx_rmw(priv, reg, 0, val);
}

static inline void
ar8xxx_reg_clear(struct ar8xxx_priv *priv, int reg, u32 val)
{
	ar8xxx_rmw(priv, reg, val, 0);
}

static inline void
split_addr(u32 regaddr, u16 *r1, u16 *r2, u16 *page)
{
	regaddr >>= 1;
	*r1 = regaddr & 0x1e;

	regaddr >>= 5;
	*r2 = regaddr & 0x7;

	regaddr >>= 3;
	*page = regaddr & 0x1ff;
}

static inline void
wait_for_page_switch(void)
{
	udelay(5);
}

#endif
