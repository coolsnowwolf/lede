/*
 * Platform driver for the Realtek RTL8367R/M ethernet switches
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/rtl8367.h>
#include <linux/version.h>

#include "rtl8366_smi.h"

#define RTL8367_RESET_DELAY	1000	/* msecs*/

#define RTL8367_PHY_ADDR_MAX	8
#define RTL8367_PHY_REG_MAX	31

#define RTL8367_VID_MASK	0xffff
#define RTL8367_FID_MASK	0xfff
#define RTL8367_UNTAG_MASK	0xffff
#define RTL8367_MEMBER_MASK	0xffff

#define RTL8367_PORT_CFG_REG(_p)		(0x000e + 0x20 * (_p))
#define   RTL8367_PORT_CFG_EGRESS_MODE_SHIFT	4
#define   RTL8367_PORT_CFG_EGRESS_MODE_MASK	0x3
#define   RTL8367_PORT_CFG_EGRESS_MODE_ORIGINAL	0
#define   RTL8367_PORT_CFG_EGRESS_MODE_KEEP	1
#define   RTL8367_PORT_CFG_EGRESS_MODE_PRI	2
#define   RTL8367_PORT_CFG_EGRESS_MODE_REAL	3

#define RTL8367_BYPASS_LINE_RATE_REG		0x03f7

#define RTL8367_TA_CTRL_REG			0x0500
#define   RTL8367_TA_CTRL_STATUS		BIT(12)
#define   RTL8367_TA_CTRL_METHOD		BIT(5)
#define   RTL8367_TA_CTRL_CMD_SHIFT		4
#define   RTL8367_TA_CTRL_CMD_READ		0
#define   RTL8367_TA_CTRL_CMD_WRITE		1
#define   RTL8367_TA_CTRL_TABLE_SHIFT		0
#define   RTL8367_TA_CTRL_TABLE_ACLRULE		1
#define   RTL8367_TA_CTRL_TABLE_ACLACT		2
#define   RTL8367_TA_CTRL_TABLE_CVLAN		3
#define   RTL8367_TA_CTRL_TABLE_L2		4
#define   RTL8367_TA_CTRL_CVLAN_READ \
		((RTL8367_TA_CTRL_CMD_READ << RTL8367_TA_CTRL_CMD_SHIFT) | \
		 RTL8367_TA_CTRL_TABLE_CVLAN)
#define   RTL8367_TA_CTRL_CVLAN_WRITE \
		((RTL8367_TA_CTRL_CMD_WRITE << RTL8367_TA_CTRL_CMD_SHIFT) | \
		 RTL8367_TA_CTRL_TABLE_CVLAN)

#define RTL8367_TA_ADDR_REG			0x0501
#define   RTL8367_TA_ADDR_MASK			0x3fff

#define RTL8367_TA_DATA_REG(_x)			(0x0503 + (_x))
#define   RTL8367_TA_VLAN_DATA_SIZE		4
#define   RTL8367_TA_VLAN_VID_MASK		RTL8367_VID_MASK
#define   RTL8367_TA_VLAN_MEMBER_SHIFT		0
#define   RTL8367_TA_VLAN_MEMBER_MASK		RTL8367_MEMBER_MASK
#define   RTL8367_TA_VLAN_FID_SHIFT		0
#define   RTL8367_TA_VLAN_FID_MASK		RTL8367_FID_MASK
#define   RTL8367_TA_VLAN_UNTAG1_SHIFT		14
#define   RTL8367_TA_VLAN_UNTAG1_MASK		0x3
#define   RTL8367_TA_VLAN_UNTAG2_SHIFT		0
#define   RTL8367_TA_VLAN_UNTAG2_MASK		0x3fff

#define RTL8367_VLAN_PVID_CTRL_REG(_p)		(0x0700 + (_p) / 2)
#define RTL8367_VLAN_PVID_CTRL_MASK		0x1f
#define RTL8367_VLAN_PVID_CTRL_SHIFT(_p)	(8 * ((_p) % 2))

#define RTL8367_VLAN_MC_BASE(_x)		(0x0728 + (_x) * 4)
#define   RTL8367_VLAN_MC_DATA_SIZE		4
#define   RTL8367_VLAN_MC_MEMBER_SHIFT		0
#define   RTL8367_VLAN_MC_MEMBER_MASK		RTL8367_MEMBER_MASK
#define   RTL8367_VLAN_MC_FID_SHIFT		0
#define   RTL8367_VLAN_MC_FID_MASK		RTL8367_FID_MASK
#define   RTL8367_VLAN_MC_EVID_SHIFT		0
#define   RTL8367_VLAN_MC_EVID_MASK		RTL8367_VID_MASK

#define RTL8367_VLAN_CTRL_REG			0x07a8
#define   RTL8367_VLAN_CTRL_ENABLE		BIT(0)

#define RTL8367_VLAN_INGRESS_REG		0x07a9

#define RTL8367_PORT_ISOLATION_REG(_p)		(0x08a2 + (_p))

#define RTL8367_MIB_COUNTER_REG(_x)		(0x1000 + (_x))

#define RTL8367_MIB_ADDRESS_REG			0x1004

#define RTL8367_MIB_CTRL_REG(_x)		(0x1005 + (_x))
#define   RTL8367_MIB_CTRL_GLOBAL_RESET_MASK	BIT(11)
#define   RTL8367_MIB_CTRL_QM_RESET_MASK	BIT(10)
#define   RTL8367_MIB_CTRL_PORT_RESET_MASK(_p)	BIT(2 + (_p))
#define   RTL8367_MIB_CTRL_RESET_MASK		BIT(1)
#define   RTL8367_MIB_CTRL_BUSY_MASK		BIT(0)

#define RTL8367_MIB_COUNT			36
#define RTL8367_MIB_COUNTER_PORT_OFFSET		0x0050

#define RTL8367_SWC0_REG			0x1200
#define   RTL8367_SWC0_MAX_LENGTH_SHIFT		13
#define   RTL8367_SWC0_MAX_LENGTH(_x)		((_x) << 13)
#define   RTL8367_SWC0_MAX_LENGTH_MASK		RTL8367_SWC0_MAX_LENGTH(0x3)
#define   RTL8367_SWC0_MAX_LENGTH_1522		RTL8367_SWC0_MAX_LENGTH(0)
#define   RTL8367_SWC0_MAX_LENGTH_1536		RTL8367_SWC0_MAX_LENGTH(1)
#define   RTL8367_SWC0_MAX_LENGTH_1552		RTL8367_SWC0_MAX_LENGTH(2)
#define   RTL8367_SWC0_MAX_LENGTH_16000		RTL8367_SWC0_MAX_LENGTH(3)

#define RTL8367_CHIP_NUMBER_REG			0x1300

#define RTL8367_CHIP_VER_REG			0x1301
#define   RTL8367_CHIP_VER_RLVID_SHIFT		12
#define   RTL8367_CHIP_VER_RLVID_MASK		0xf
#define   RTL8367_CHIP_VER_MCID_SHIFT		8
#define   RTL8367_CHIP_VER_MCID_MASK		0xf
#define   RTL8367_CHIP_VER_BOID_SHIFT		4
#define   RTL8367_CHIP_VER_BOID_MASK		0xf

#define RTL8367_CHIP_MODE_REG			0x1302
#define   RTL8367_CHIP_MODE_MASK		0x7

#define RTL8367_CHIP_DEBUG0_REG			0x1303
#define   RTL8367_CHIP_DEBUG0_DUMMY0(_x)	BIT(8 + (_x))

#define RTL8367_CHIP_DEBUG1_REG			0x1304

#define RTL8367_DIS_REG				0x1305
#define   RTL8367_DIS_SKIP_MII_RXER(_x)		BIT(12 + (_x))
#define   RTL8367_DIS_RGMII_SHIFT(_x)		(4 * (_x))
#define   RTL8367_DIS_RGMII_MASK		0x7

#define RTL8367_EXT_RGMXF_REG(_x)		(0x1306 + (_x))
#define   RTL8367_EXT_RGMXF_DUMMY0_SHIFT	5
#define   RTL8367_EXT_RGMXF_DUMMY0_MASK	0x7ff
#define   RTL8367_EXT_RGMXF_TXDELAY_SHIFT	3
#define   RTL8367_EXT_RGMXF_TXDELAY_MASK	1
#define   RTL8367_EXT_RGMXF_RXDELAY_MASK	0x7

#define RTL8367_DI_FORCE_REG(_x)		(0x1310 + (_x))
#define   RTL8367_DI_FORCE_MODE			BIT(12)
#define   RTL8367_DI_FORCE_NWAY			BIT(7)
#define   RTL8367_DI_FORCE_TXPAUSE		BIT(6)
#define   RTL8367_DI_FORCE_RXPAUSE		BIT(5)
#define   RTL8367_DI_FORCE_LINK			BIT(4)
#define   RTL8367_DI_FORCE_DUPLEX		BIT(2)
#define   RTL8367_DI_FORCE_SPEED_MASK		3
#define   RTL8367_DI_FORCE_SPEED_10		0
#define   RTL8367_DI_FORCE_SPEED_100		1
#define   RTL8367_DI_FORCE_SPEED_1000		2

#define RTL8367_MAC_FORCE_REG(_x)		(0x1312 + (_x))

#define RTL8367_CHIP_RESET_REG			0x1322
#define   RTL8367_CHIP_RESET_SW			BIT(1)
#define   RTL8367_CHIP_RESET_HW			BIT(0)

#define RTL8367_PORT_STATUS_REG(_p)		(0x1352 + (_p))
#define   RTL8367_PORT_STATUS_NWAY		BIT(7)
#define   RTL8367_PORT_STATUS_TXPAUSE		BIT(6)
#define   RTL8367_PORT_STATUS_RXPAUSE		BIT(5)
#define   RTL8367_PORT_STATUS_LINK		BIT(4)
#define   RTL8367_PORT_STATUS_DUPLEX		BIT(2)
#define   RTL8367_PORT_STATUS_SPEED_MASK	0x0003
#define   RTL8367_PORT_STATUS_SPEED_10		0
#define   RTL8367_PORT_STATUS_SPEED_100		1
#define   RTL8367_PORT_STATUS_SPEED_1000	2

#define RTL8367_RTL_NO_REG			0x13c0
#define   RTL8367_RTL_NO_8367R			0x3670
#define   RTL8367_RTL_NO_8367M			0x3671

#define RTL8367_RTL_VER_REG			0x13c1
#define   RTL8367_RTL_VER_MASK			0xf

#define RTL8367_RTL_MAGIC_ID_REG		0x13c2
#define   RTL8367_RTL_MAGIC_ID_VAL		0x0249

#define RTL8367_LED_SYS_CONFIG_REG		0x1b00
#define RTL8367_LED_MODE_REG			0x1b02
#define   RTL8367_LED_MODE_RATE_M		0x7
#define   RTL8367_LED_MODE_RATE_S		1

#define RTL8367_LED_CONFIG_REG			0x1b03
#define   RTL8367_LED_CONFIG_DATA_S		12
#define   RTL8367_LED_CONFIG_DATA_M		0x3
#define   RTL8367_LED_CONFIG_SEL		BIT(14)
#define   RTL8367_LED_CONFIG_LED_CFG_M		0xf

#define RTL8367_PARA_LED_IO_EN1_REG		0x1b24
#define RTL8367_PARA_LED_IO_EN2_REG		0x1b25
#define   RTL8367_PARA_LED_IO_EN_PMASK		0xff

#define RTL8367_IA_CTRL_REG			0x1f00
#define   RTL8367_IA_CTRL_RW(_x)		((_x) << 1)
#define   RTL8367_IA_CTRL_RW_READ		RTL8367_IA_CTRL_RW(0)
#define   RTL8367_IA_CTRL_RW_WRITE		RTL8367_IA_CTRL_RW(1)
#define   RTL8367_IA_CTRL_CMD_MASK		BIT(0)

#define RTL8367_IA_STATUS_REG			0x1f01
#define   RTL8367_IA_STATUS_PHY_BUSY		BIT(2)
#define   RTL8367_IA_STATUS_SDS_BUSY		BIT(1)
#define   RTL8367_IA_STATUS_MDX_BUSY		BIT(0)

#define RTL8367_IA_ADDRESS_REG			0x1f02

#define RTL8367_IA_WRITE_DATA_REG		0x1f03
#define RTL8367_IA_READ_DATA_REG		0x1f04

#define RTL8367_INTERNAL_PHY_REG(_a, _r)	(0x2000 + 32 * (_a) + (_r))

#define RTL8367_CPU_PORT_NUM		9
#define RTL8367_NUM_PORTS		10
#define RTL8367_NUM_VLANS		32
#define RTL8367_NUM_LEDGROUPS		4
#define RTL8367_NUM_VIDS		4096
#define RTL8367_PRIORITYMAX		7
#define RTL8367_FIDMAX			7

#define RTL8367_PORT_0			BIT(0)
#define RTL8367_PORT_1			BIT(1)
#define RTL8367_PORT_2			BIT(2)
#define RTL8367_PORT_3			BIT(3)
#define RTL8367_PORT_4			BIT(4)
#define RTL8367_PORT_5			BIT(5)
#define RTL8367_PORT_6			BIT(6)
#define RTL8367_PORT_7			BIT(7)
#define RTL8367_PORT_E1			BIT(8)	/* external port 1 */
#define RTL8367_PORT_E0			BIT(9)	/* external port 0 */

#define RTL8367_PORTS_ALL					\
	(RTL8367_PORT_0 | RTL8367_PORT_1 | RTL8367_PORT_2 |	\
	 RTL8367_PORT_3 | RTL8367_PORT_4 | RTL8367_PORT_5 |	\
	 RTL8367_PORT_6 | RTL8367_PORT_7 | RTL8367_PORT_E1 |	\
	 RTL8367_PORT_E0)

#define RTL8367_PORTS_ALL_BUT_CPU				\
	(RTL8367_PORT_0 | RTL8367_PORT_1 | RTL8367_PORT_2 |	\
	 RTL8367_PORT_3 | RTL8367_PORT_4 | RTL8367_PORT_5 |	\
	 RTL8367_PORT_6 | RTL8367_PORT_7 | RTL8367_PORT_E1)

struct rtl8367_initval {
	u16 reg;
	u16 val;
};

#define RTL8367_MIB_RXB_ID		0	/* IfInOctets */
#define RTL8367_MIB_TXB_ID		20	/* IfOutOctets */

static struct rtl8366_mib_counter rtl8367_mib_counters[] = {
	{ 0,  0, 4, "IfInOctets"				},
	{ 0,  4, 2, "Dot3StatsFCSErrors"			},
	{ 0,  6, 2, "Dot3StatsSymbolErrors"			},
	{ 0,  8, 2, "Dot3InPauseFrames"				},
	{ 0, 10, 2, "Dot3ControlInUnknownOpcodes"		},
	{ 0, 12, 2, "EtherStatsFragments"			},
	{ 0, 14, 2, "EtherStatsJabbers"				},
	{ 0, 16, 2, "IfInUcastPkts"				},
	{ 0, 18, 2, "EtherStatsDropEvents"			},
	{ 0, 20, 4, "EtherStatsOctets"				},

	{ 0, 24, 2, "EtherStatsUnderSizePkts"			},
	{ 0, 26, 2, "EtherOversizeStats"			},
	{ 0, 28, 2, "EtherStatsPkts64Octets"			},
	{ 0, 30, 2, "EtherStatsPkts65to127Octets"		},
	{ 0, 32, 2, "EtherStatsPkts128to255Octets"		},
	{ 0, 34, 2, "EtherStatsPkts256to511Octets"		},
	{ 0, 36, 2, "EtherStatsPkts512to1023Octets"		},
	{ 0, 38, 2, "EtherStatsPkts1024to1518Octets"		},
	{ 0, 40, 2, "EtherStatsMulticastPkts"			},
	{ 0, 42, 2, "EtherStatsBroadcastPkts"			},

	{ 0, 44, 4, "IfOutOctets"				},

	{ 0, 48, 2, "Dot3StatsSingleCollisionFrames"		},
	{ 0, 50, 2, "Dot3StatMultipleCollisionFrames"		},
	{ 0, 52, 2, "Dot3sDeferredTransmissions"		},
	{ 0, 54, 2, "Dot3StatsLateCollisions"			},
	{ 0, 56, 2, "EtherStatsCollisions"			},
	{ 0, 58, 2, "Dot3StatsExcessiveCollisions"		},
	{ 0, 60, 2, "Dot3OutPauseFrames"			},
	{ 0, 62, 2, "Dot1dBasePortDelayExceededDiscards"	},
	{ 0, 64, 2, "Dot1dTpPortInDiscards"			},
	{ 0, 66, 2, "IfOutUcastPkts"				},
	{ 0, 68, 2, "IfOutMulticastPkts"			},
	{ 0, 70, 2, "IfOutBroadcastPkts"			},
	{ 0, 72, 2, "OutOampduPkts"				},
	{ 0, 74, 2, "InOampduPkts"				},
	{ 0, 76, 2, "PktgenPkts"				},
};

#define REG_RD(_smi, _reg, _val)					\
	do {								\
		err = rtl8366_smi_read_reg(_smi, _reg, _val);		\
		if (err)						\
			return err;					\
	} while (0)

#define REG_WR(_smi, _reg, _val)					\
	do {								\
		err = rtl8366_smi_write_reg(_smi, _reg, _val);		\
		if (err)						\
			return err;					\
	} while (0)

#define REG_RMW(_smi, _reg, _mask, _val)				\
	do {								\
		err = rtl8366_smi_rmwr(_smi, _reg, _mask, _val);	\
		if (err)						\
			return err;					\
	} while (0)

static const struct rtl8367_initval rtl8367_initvals_0_0[] = {
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0000}, {0x2215, 0x1006},
	{0x221f, 0x0005}, {0x2200, 0x00c6}, {0x221f, 0x0007}, {0x221e, 0x0048},
	{0x2215, 0x6412}, {0x2216, 0x6412}, {0x2217, 0x6412}, {0x2218, 0x6412},
	{0x2219, 0x6412}, {0x221A, 0x6412}, {0x221f, 0x0001}, {0x220c, 0xdbf0},
	{0x2209, 0x2576}, {0x2207, 0x287E}, {0x220A, 0x68E5}, {0x221D, 0x3DA4},
	{0x221C, 0xE7F7}, {0x2214, 0x7F52}, {0x2218, 0x7FCE}, {0x2208, 0x04B7},
	{0x2206, 0x4072}, {0x2210, 0xF05E}, {0x221B, 0xB414}, {0x221F, 0x0003},
	{0x221A, 0x06A6}, {0x2210, 0xF05E}, {0x2213, 0x06EB}, {0x2212, 0xF4D2},
	{0x220E, 0xE120}, {0x2200, 0x7C00}, {0x2202, 0x5FD0}, {0x220D, 0x0207},
	{0x221f, 0x0002}, {0x2205, 0x0978}, {0x2202, 0x8C01}, {0x2207, 0x3620},
	{0x221C, 0x0001}, {0x2203, 0x0420}, {0x2204, 0x80C8}, {0x133e, 0x0ede},
	{0x221f, 0x0002}, {0x220c, 0x0073}, {0x220d, 0xEB65}, {0x220e, 0x51d1},
	{0x220f, 0x5dcb}, {0x2210, 0x3044}, {0x2211, 0x1800}, {0x2212, 0x7E00},
	{0x2213, 0x0000}, {0x133f, 0x0010}, {0x133e, 0x0ffe}, {0x207f, 0x0002},
	{0x2074, 0x3D22}, {0x2075, 0x2000}, {0x2076, 0x6040}, {0x2077, 0x0000},
	{0x2078, 0x0f0a}, {0x2079, 0x50AB}, {0x207a, 0x0000}, {0x207b, 0x0f0f},
	{0x205f, 0x0002}, {0x2054, 0xFF00}, {0x2055, 0x000A}, {0x2056, 0x000A},
	{0x2057, 0x0005}, {0x2058, 0x0005}, {0x2059, 0x0000}, {0x205A, 0x0005},
	{0x205B, 0x0005}, {0x205C, 0x0005}, {0x209f, 0x0002}, {0x2094, 0x00AA},
	{0x2095, 0x00AA}, {0x2096, 0x00AA}, {0x2097, 0x00AA}, {0x2098, 0x0055},
	{0x2099, 0x00AA}, {0x209A, 0x00AA}, {0x209B, 0x00AA}, {0x1363, 0x8354},
	{0x1270, 0x3333}, {0x1271, 0x3333}, {0x1272, 0x3333}, {0x1330, 0x00DB},
	{0x1203, 0xff00}, {0x1200, 0x7fc4}, {0x121d, 0x1006}, {0x121e, 0x03e8},
	{0x121f, 0x02b3}, {0x1220, 0x028f}, {0x1221, 0x029b}, {0x1222, 0x0277},
	{0x1223, 0x02b3}, {0x1224, 0x028f}, {0x1225, 0x029b}, {0x1226, 0x0277},
	{0x1227, 0x00c0}, {0x1228, 0x00b4}, {0x122f, 0x00c0}, {0x1230, 0x00b4},
	{0x1229, 0x0020}, {0x122a, 0x000c}, {0x1231, 0x0030}, {0x1232, 0x0024},
	{0x0219, 0x0032}, {0x0200, 0x03e8}, {0x0201, 0x03e8}, {0x0202, 0x03e8},
	{0x0203, 0x03e8}, {0x0204, 0x03e8}, {0x0205, 0x03e8}, {0x0206, 0x03e8},
	{0x0207, 0x03e8}, {0x0218, 0x0032}, {0x0208, 0x029b}, {0x0209, 0x029b},
	{0x020a, 0x029b}, {0x020b, 0x029b}, {0x020c, 0x029b}, {0x020d, 0x029b},
	{0x020e, 0x029b}, {0x020f, 0x029b}, {0x0210, 0x029b}, {0x0211, 0x029b},
	{0x0212, 0x029b}, {0x0213, 0x029b}, {0x0214, 0x029b}, {0x0215, 0x029b},
	{0x0216, 0x029b}, {0x0217, 0x029b}, {0x0900, 0x0000}, {0x0901, 0x0000},
	{0x0902, 0x0000}, {0x0903, 0x0000}, {0x0865, 0x3210}, {0x087b, 0x0000},
	{0x087c, 0xff00}, {0x087d, 0x0000}, {0x087e, 0x0000}, {0x0801, 0x0100},
	{0x0802, 0x0100}, {0x1700, 0x014C}, {0x0301, 0x00FF}, {0x12AA, 0x0096},
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0005}, {0x2200, 0x00C4},
	{0x221f, 0x0000}, {0x2210, 0x05EF}, {0x2204, 0x05E1}, {0x2200, 0x1340},
	{0x133f, 0x0010}, {0x20A0, 0x1940}, {0x20C0, 0x1940}, {0x20E0, 0x1940},
};

static const struct rtl8367_initval rtl8367_initvals_0_1[] = {
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0000}, {0x2215, 0x1006},
	{0x221f, 0x0005}, {0x2200, 0x00c6}, {0x221f, 0x0007}, {0x221e, 0x0048},
	{0x2215, 0x6412}, {0x2216, 0x6412}, {0x2217, 0x6412}, {0x2218, 0x6412},
	{0x2219, 0x6412}, {0x221A, 0x6412}, {0x221f, 0x0001}, {0x220c, 0xdbf0},
	{0x2209, 0x2576}, {0x2207, 0x287E}, {0x220A, 0x68E5}, {0x221D, 0x3DA4},
	{0x221C, 0xE7F7}, {0x2214, 0x7F52}, {0x2218, 0x7FCE}, {0x2208, 0x04B7},
	{0x2206, 0x4072}, {0x2210, 0xF05E}, {0x221B, 0xB414}, {0x221F, 0x0003},
	{0x221A, 0x06A6}, {0x2210, 0xF05E}, {0x2213, 0x06EB}, {0x2212, 0xF4D2},
	{0x220E, 0xE120}, {0x2200, 0x7C00}, {0x2202, 0x5FD0}, {0x220D, 0x0207},
	{0x221f, 0x0002}, {0x2205, 0x0978}, {0x2202, 0x8C01}, {0x2207, 0x3620},
	{0x221C, 0x0001}, {0x2203, 0x0420}, {0x2204, 0x80C8}, {0x133e, 0x0ede},
	{0x221f, 0x0002}, {0x220c, 0x0073}, {0x220d, 0xEB65}, {0x220e, 0x51d1},
	{0x220f, 0x5dcb}, {0x2210, 0x3044}, {0x2211, 0x1800}, {0x2212, 0x7E00},
	{0x2213, 0x0000}, {0x133f, 0x0010}, {0x133e, 0x0ffe}, {0x207f, 0x0002},
	{0x2074, 0x3D22}, {0x2075, 0x2000}, {0x2076, 0x6040}, {0x2077, 0x0000},
	{0x2078, 0x0f0a}, {0x2079, 0x50AB}, {0x207a, 0x0000}, {0x207b, 0x0f0f},
	{0x205f, 0x0002}, {0x2054, 0xFF00}, {0x2055, 0x000A}, {0x2056, 0x000A},
	{0x2057, 0x0005}, {0x2058, 0x0005}, {0x2059, 0x0000}, {0x205A, 0x0005},
	{0x205B, 0x0005}, {0x205C, 0x0005}, {0x209f, 0x0002}, {0x2094, 0x00AA},
	{0x2095, 0x00AA}, {0x2096, 0x00AA}, {0x2097, 0x00AA}, {0x2098, 0x0055},
	{0x2099, 0x00AA}, {0x209A, 0x00AA}, {0x209B, 0x00AA}, {0x1363, 0x8354},
	{0x1270, 0x3333}, {0x1271, 0x3333}, {0x1272, 0x3333}, {0x1330, 0x00DB},
	{0x1203, 0xff00}, {0x1200, 0x7fc4}, {0x121d, 0x1b06}, {0x121e, 0x07f0},
	{0x121f, 0x0438}, {0x1220, 0x040f}, {0x1221, 0x040f}, {0x1222, 0x03eb},
	{0x1223, 0x0438}, {0x1224, 0x040f}, {0x1225, 0x040f}, {0x1226, 0x03eb},
	{0x1227, 0x0144}, {0x1228, 0x0138}, {0x122f, 0x0144}, {0x1230, 0x0138},
	{0x1229, 0x0020}, {0x122a, 0x000c}, {0x1231, 0x0030}, {0x1232, 0x0024},
	{0x0219, 0x0032}, {0x0200, 0x07d0}, {0x0201, 0x07d0}, {0x0202, 0x07d0},
	{0x0203, 0x07d0}, {0x0204, 0x07d0}, {0x0205, 0x07d0}, {0x0206, 0x07d0},
	{0x0207, 0x07d0}, {0x0218, 0x0032}, {0x0208, 0x0190}, {0x0209, 0x0190},
	{0x020a, 0x0190}, {0x020b, 0x0190}, {0x020c, 0x0190}, {0x020d, 0x0190},
	{0x020e, 0x0190}, {0x020f, 0x0190}, {0x0210, 0x0190}, {0x0211, 0x0190},
	{0x0212, 0x0190}, {0x0213, 0x0190}, {0x0214, 0x0190}, {0x0215, 0x0190},
	{0x0216, 0x0190}, {0x0217, 0x0190}, {0x0900, 0x0000}, {0x0901, 0x0000},
	{0x0902, 0x0000}, {0x0903, 0x0000}, {0x0865, 0x3210}, {0x087b, 0x0000},
	{0x087c, 0xff00}, {0x087d, 0x0000}, {0x087e, 0x0000}, {0x0801, 0x0100},
	{0x0802, 0x0100}, {0x1700, 0x0125}, {0x0301, 0x00FF}, {0x12AA, 0x0096},
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0005}, {0x2200, 0x00C4},
	{0x221f, 0x0000}, {0x2210, 0x05EF}, {0x2204, 0x05E1}, {0x2200, 0x1340},
	{0x133f, 0x0010},
};

static const struct rtl8367_initval rtl8367_initvals_1_0[] = {
	{0x1B24, 0x0000}, {0x1B25, 0x0000}, {0x1B26, 0x0000}, {0x1B27, 0x0000},
	{0x207F, 0x0002}, {0x2079, 0x0200}, {0x207F, 0x0000}, {0x133F, 0x0030},
	{0x133E, 0x000E}, {0x221F, 0x0005}, {0x2201, 0x0700}, {0x2205, 0x8B82},
	{0x2206, 0x05CB}, {0x221F, 0x0002}, {0x2204, 0x80C2}, {0x2205, 0x0938},
	{0x221F, 0x0003}, {0x2212, 0xC4D2}, {0x220D, 0x0207}, {0x221F, 0x0001},
	{0x2207, 0x267E}, {0x221C, 0xE5F7}, {0x221B, 0x0424}, {0x221F, 0x0007},
	{0x221E, 0x0040}, {0x2218, 0x0000}, {0x221F, 0x0007}, {0x221E, 0x002C},
	{0x2218, 0x008B}, {0x221F, 0x0005}, {0x2205, 0xFFF6}, {0x2206, 0x0080},
	{0x2205, 0x8000}, {0x2206, 0xF8E0}, {0x2206, 0xE000}, {0x2206, 0xE1E0},
	{0x2206, 0x01AC}, {0x2206, 0x2408}, {0x2206, 0xE08B}, {0x2206, 0x84F7},
	{0x2206, 0x20E4}, {0x2206, 0x8B84}, {0x2206, 0xFC05}, {0x2206, 0xF8FA},
	{0x2206, 0xEF69}, {0x2206, 0xE08B}, {0x2206, 0x86AC}, {0x2206, 0x201A},
	{0x2206, 0xBF80}, {0x2206, 0x59D0}, {0x2206, 0x2402}, {0x2206, 0x803D},
	{0x2206, 0xE0E0}, {0x2206, 0xE4E1}, {0x2206, 0xE0E5}, {0x2206, 0x5806},
	{0x2206, 0x68C0}, {0x2206, 0xD1D2}, {0x2206, 0xE4E0}, {0x2206, 0xE4E5},
	{0x2206, 0xE0E5}, {0x2206, 0xEF96}, {0x2206, 0xFEFC}, {0x2206, 0x05FB},
	{0x2206, 0x0BFB}, {0x2206, 0x58FF}, {0x2206, 0x9E11}, {0x2206, 0x06F0},
	{0x2206, 0x0C81}, {0x2206, 0x8AE0}, {0x2206, 0x0019}, {0x2206, 0x1B89},
	{0x2206, 0xCFEB}, {0x2206, 0x19EB}, {0x2206, 0x19B0}, {0x2206, 0xEFFF},
	{0x2206, 0x0BFF}, {0x2206, 0x0425}, {0x2206, 0x0807}, {0x2206, 0x2640},
	{0x2206, 0x7227}, {0x2206, 0x267E}, {0x2206, 0x2804}, {0x2206, 0xB729},
	{0x2206, 0x2576}, {0x2206, 0x2A68}, {0x2206, 0xE52B}, {0x2206, 0xAD00},
	{0x2206, 0x2CDB}, {0x2206, 0xF02D}, {0x2206, 0x67BB}, {0x2206, 0x2E7B},
	{0x2206, 0x0F2F}, {0x2206, 0x7365}, {0x2206, 0x31AC}, {0x2206, 0xCC32},
	{0x2206, 0x2300}, {0x2206, 0x332D}, {0x2206, 0x1734}, {0x2206, 0x7F52},
	{0x2206, 0x3510}, {0x2206, 0x0036}, {0x2206, 0x0600}, {0x2206, 0x370C},
	{0x2206, 0xC038}, {0x2206, 0x7FCE}, {0x2206, 0x3CE5}, {0x2206, 0xF73D},
	{0x2206, 0x3DA4}, {0x2206, 0x6530}, {0x2206, 0x3E67}, {0x2206, 0x0053},
	{0x2206, 0x69D2}, {0x2206, 0x0F6A}, {0x2206, 0x012C}, {0x2206, 0x6C2B},
	{0x2206, 0x136E}, {0x2206, 0xE100}, {0x2206, 0x6F12}, {0x2206, 0xF771},
	{0x2206, 0x006B}, {0x2206, 0x7306}, {0x2206, 0xEB74}, {0x2206, 0x94C7},
	{0x2206, 0x7698}, {0x2206, 0x0A77}, {0x2206, 0x5000}, {0x2206, 0x788A},
	{0x2206, 0x1579}, {0x2206, 0x7F6F}, {0x2206, 0x7A06}, {0x2206, 0xA600},
	{0x2205, 0x8B90}, {0x2206, 0x8000}, {0x2205, 0x8B92}, {0x2206, 0x8000},
	{0x2205, 0x8B94}, {0x2206, 0x8014}, {0x2208, 0xFFFA}, {0x2202, 0x3C65},
	{0x2205, 0xFFF6}, {0x2206, 0x00F7}, {0x221F, 0x0000}, {0x221F, 0x0007},
	{0x221E, 0x0042}, {0x2218, 0x0000}, {0x221E, 0x002D}, {0x2218, 0xF010},
	{0x221E, 0x0020}, {0x2215, 0x0000}, {0x221E, 0x0023}, {0x2216, 0x8000},
	{0x221F, 0x0000}, {0x133F, 0x0010}, {0x133E, 0x0FFE}, {0x1362, 0x0115},
	{0x1363, 0x0002}, {0x1363, 0x0000}, {0x1306, 0x000C}, {0x1307, 0x000C},
	{0x1303, 0x0067}, {0x1304, 0x4444}, {0x1203, 0xFF00}, {0x1200, 0x7FC4},
	{0x121D, 0x7D16}, {0x121E, 0x03E8}, {0x121F, 0x024E}, {0x1220, 0x0230},
	{0x1221, 0x0244}, {0x1222, 0x0226}, {0x1223, 0x024E}, {0x1224, 0x0230},
	{0x1225, 0x0244}, {0x1226, 0x0226}, {0x1227, 0x00C0}, {0x1228, 0x00B4},
	{0x122F, 0x00C0}, {0x1230, 0x00B4}, {0x0208, 0x03E8}, {0x0209, 0x03E8},
	{0x020A, 0x03E8}, {0x020B, 0x03E8}, {0x020C, 0x03E8}, {0x020D, 0x03E8},
	{0x020E, 0x03E8}, {0x020F, 0x03E8}, {0x0210, 0x03E8}, {0x0211, 0x03E8},
	{0x0212, 0x03E8}, {0x0213, 0x03E8}, {0x0214, 0x03E8}, {0x0215, 0x03E8},
	{0x0216, 0x03E8}, {0x0217, 0x03E8}, {0x0900, 0x0000}, {0x0901, 0x0000},
	{0x0902, 0x0000}, {0x0903, 0x0000}, {0x0865, 0x3210}, {0x087B, 0x0000},
	{0x087C, 0xFF00}, {0x087D, 0x0000}, {0x087E, 0x0000}, {0x0801, 0x0100},
	{0x0802, 0x0100}, {0x0A20, 0x2040}, {0x0A21, 0x2040}, {0x0A22, 0x2040},
	{0x0A23, 0x2040}, {0x0A24, 0x2040}, {0x0A28, 0x2040}, {0x0A29, 0x2040},
	{0x133F, 0x0030}, {0x133E, 0x000E}, {0x221F, 0x0000}, {0x2200, 0x1340},
	{0x221F, 0x0000}, {0x133F, 0x0010}, {0x133E, 0x0FFE}, {0x20A0, 0x1940},
	{0x20C0, 0x1940}, {0x20E0, 0x1940}, {0x130c, 0x0050},
};

static const struct rtl8367_initval rtl8367_initvals_1_1[] = {
	{0x1B24, 0x0000}, {0x1B25, 0x0000}, {0x1B26, 0x0000}, {0x1B27, 0x0000},
	{0x207F, 0x0002}, {0x2079, 0x0200}, {0x207F, 0x0000}, {0x133F, 0x0030},
	{0x133E, 0x000E}, {0x221F, 0x0005}, {0x2201, 0x0700}, {0x2205, 0x8B82},
	{0x2206, 0x05CB}, {0x221F, 0x0002}, {0x2204, 0x80C2}, {0x2205, 0x0938},
	{0x221F, 0x0003}, {0x2212, 0xC4D2}, {0x220D, 0x0207}, {0x221F, 0x0001},
	{0x2207, 0x267E}, {0x221C, 0xE5F7}, {0x221B, 0x0424}, {0x221F, 0x0007},
	{0x221E, 0x0040}, {0x2218, 0x0000}, {0x221F, 0x0007}, {0x221E, 0x002C},
	{0x2218, 0x008B}, {0x221F, 0x0005}, {0x2205, 0xFFF6}, {0x2206, 0x0080},
	{0x2205, 0x8000}, {0x2206, 0xF8E0}, {0x2206, 0xE000}, {0x2206, 0xE1E0},
	{0x2206, 0x01AC}, {0x2206, 0x2408}, {0x2206, 0xE08B}, {0x2206, 0x84F7},
	{0x2206, 0x20E4}, {0x2206, 0x8B84}, {0x2206, 0xFC05}, {0x2206, 0xF8FA},
	{0x2206, 0xEF69}, {0x2206, 0xE08B}, {0x2206, 0x86AC}, {0x2206, 0x201A},
	{0x2206, 0xBF80}, {0x2206, 0x59D0}, {0x2206, 0x2402}, {0x2206, 0x803D},
	{0x2206, 0xE0E0}, {0x2206, 0xE4E1}, {0x2206, 0xE0E5}, {0x2206, 0x5806},
	{0x2206, 0x68C0}, {0x2206, 0xD1D2}, {0x2206, 0xE4E0}, {0x2206, 0xE4E5},
	{0x2206, 0xE0E5}, {0x2206, 0xEF96}, {0x2206, 0xFEFC}, {0x2206, 0x05FB},
	{0x2206, 0x0BFB}, {0x2206, 0x58FF}, {0x2206, 0x9E11}, {0x2206, 0x06F0},
	{0x2206, 0x0C81}, {0x2206, 0x8AE0}, {0x2206, 0x0019}, {0x2206, 0x1B89},
	{0x2206, 0xCFEB}, {0x2206, 0x19EB}, {0x2206, 0x19B0}, {0x2206, 0xEFFF},
	{0x2206, 0x0BFF}, {0x2206, 0x0425}, {0x2206, 0x0807}, {0x2206, 0x2640},
	{0x2206, 0x7227}, {0x2206, 0x267E}, {0x2206, 0x2804}, {0x2206, 0xB729},
	{0x2206, 0x2576}, {0x2206, 0x2A68}, {0x2206, 0xE52B}, {0x2206, 0xAD00},
	{0x2206, 0x2CDB}, {0x2206, 0xF02D}, {0x2206, 0x67BB}, {0x2206, 0x2E7B},
	{0x2206, 0x0F2F}, {0x2206, 0x7365}, {0x2206, 0x31AC}, {0x2206, 0xCC32},
	{0x2206, 0x2300}, {0x2206, 0x332D}, {0x2206, 0x1734}, {0x2206, 0x7F52},
	{0x2206, 0x3510}, {0x2206, 0x0036}, {0x2206, 0x0600}, {0x2206, 0x370C},
	{0x2206, 0xC038}, {0x2206, 0x7FCE}, {0x2206, 0x3CE5}, {0x2206, 0xF73D},
	{0x2206, 0x3DA4}, {0x2206, 0x6530}, {0x2206, 0x3E67}, {0x2206, 0x0053},
	{0x2206, 0x69D2}, {0x2206, 0x0F6A}, {0x2206, 0x012C}, {0x2206, 0x6C2B},
	{0x2206, 0x136E}, {0x2206, 0xE100}, {0x2206, 0x6F12}, {0x2206, 0xF771},
	{0x2206, 0x006B}, {0x2206, 0x7306}, {0x2206, 0xEB74}, {0x2206, 0x94C7},
	{0x2206, 0x7698}, {0x2206, 0x0A77}, {0x2206, 0x5000}, {0x2206, 0x788A},
	{0x2206, 0x1579}, {0x2206, 0x7F6F}, {0x2206, 0x7A06}, {0x2206, 0xA600},
	{0x2205, 0x8B90}, {0x2206, 0x8000}, {0x2205, 0x8B92}, {0x2206, 0x8000},
	{0x2205, 0x8B94}, {0x2206, 0x8014}, {0x2208, 0xFFFA}, {0x2202, 0x3C65},
	{0x2205, 0xFFF6}, {0x2206, 0x00F7}, {0x221F, 0x0000}, {0x221F, 0x0007},
	{0x221E, 0x0042}, {0x2218, 0x0000}, {0x221E, 0x002D}, {0x2218, 0xF010},
	{0x221E, 0x0020}, {0x2215, 0x0000}, {0x221E, 0x0023}, {0x2216, 0x8000},
	{0x221F, 0x0000}, {0x133F, 0x0010}, {0x133E, 0x0FFE}, {0x1362, 0x0115},
	{0x1363, 0x0002}, {0x1363, 0x0000}, {0x1306, 0x000C}, {0x1307, 0x000C},
	{0x1303, 0x0067}, {0x1304, 0x4444}, {0x1203, 0xFF00}, {0x1200, 0x7FC4},
	{0x0900, 0x0000}, {0x0901, 0x0000}, {0x0902, 0x0000}, {0x0903, 0x0000},
	{0x0865, 0x3210}, {0x087B, 0x0000}, {0x087C, 0xFF00}, {0x087D, 0x0000},
	{0x087E, 0x0000}, {0x0801, 0x0100}, {0x0802, 0x0100}, {0x0A20, 0x2040},
	{0x0A21, 0x2040}, {0x0A22, 0x2040}, {0x0A23, 0x2040}, {0x0A24, 0x2040},
	{0x0A25, 0x2040}, {0x0A26, 0x2040}, {0x0A27, 0x2040}, {0x0A28, 0x2040},
	{0x0A29, 0x2040}, {0x133F, 0x0030}, {0x133E, 0x000E}, {0x221F, 0x0000},
	{0x2200, 0x1340}, {0x221F, 0x0000}, {0x133F, 0x0010}, {0x133E, 0x0FFE},
	{0x1B03, 0x0876},
};

static const struct rtl8367_initval rtl8367_initvals_2_0[] = {
	{0x1b24, 0x0000}, {0x1b25, 0x0000}, {0x1b26, 0x0000}, {0x1b27, 0x0000},
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0007}, {0x221e, 0x0048},
	{0x2219, 0x4012}, {0x221f, 0x0003}, {0x2201, 0x3554}, {0x2202, 0x63e8},
	{0x2203, 0x99c2}, {0x2204, 0x0113}, {0x2205, 0x303e}, {0x220d, 0x0207},
	{0x220e, 0xe100}, {0x221f, 0x0007}, {0x221e, 0x0040}, {0x2218, 0x0000},
	{0x221f, 0x0007}, {0x221e, 0x002c}, {0x2218, 0x008b}, {0x221f, 0x0005},
	{0x2205, 0xfff6}, {0x2206, 0x0080}, {0x221f, 0x0005}, {0x2205, 0x8000},
	{0x2206, 0x0280}, {0x2206, 0x2bf7}, {0x2206, 0x00e0}, {0x2206, 0xfff7},
	{0x2206, 0xa080}, {0x2206, 0x02ae}, {0x2206, 0xf602}, {0x2206, 0x804e},
	{0x2206, 0x0201}, {0x2206, 0x5002}, {0x2206, 0x0163}, {0x2206, 0x0201},
	{0x2206, 0x79e0}, {0x2206, 0x8b8c}, {0x2206, 0xe18b}, {0x2206, 0x8d1e},
	{0x2206, 0x01e1}, {0x2206, 0x8b8e}, {0x2206, 0x1e01}, {0x2206, 0xa000},
	{0x2206, 0xe4ae}, {0x2206, 0xd8bf}, {0x2206, 0x8b88}, {0x2206, 0xec00},
	{0x2206, 0x19a9}, {0x2206, 0x8b90}, {0x2206, 0xf9ee}, {0x2206, 0xfff6},
	{0x2206, 0x00ee}, {0x2206, 0xfff7}, {0x2206, 0xfce0}, {0x2206, 0xe140},
	{0x2206, 0xe1e1}, {0x2206, 0x41f7}, {0x2206, 0x2ff6}, {0x2206, 0x28e4},
	{0x2206, 0xe140}, {0x2206, 0xe5e1}, {0x2206, 0x4104}, {0x2206, 0xf8fa},
	{0x2206, 0xef69}, {0x2206, 0xe08b}, {0x2206, 0x86ac}, {0x2206, 0x201a},
	{0x2206, 0xbf80}, {0x2206, 0x77d0}, {0x2206, 0x6c02}, {0x2206, 0x2978},
	{0x2206, 0xe0e0}, {0x2206, 0xe4e1}, {0x2206, 0xe0e5}, {0x2206, 0x5806},
	{0x2206, 0x68c0}, {0x2206, 0xd1d2}, {0x2206, 0xe4e0}, {0x2206, 0xe4e5},
	{0x2206, 0xe0e5}, {0x2206, 0xef96}, {0x2206, 0xfefc}, {0x2206, 0x0425},
	{0x2206, 0x0807}, {0x2206, 0x2640}, {0x2206, 0x7227}, {0x2206, 0x267e},
	{0x2206, 0x2804}, {0x2206, 0xb729}, {0x2206, 0x2576}, {0x2206, 0x2a68},
	{0x2206, 0xe52b}, {0x2206, 0xad00}, {0x2206, 0x2cdb}, {0x2206, 0xf02d},
	{0x2206, 0x67bb}, {0x2206, 0x2e7b}, {0x2206, 0x0f2f}, {0x2206, 0x7365},
	{0x2206, 0x31ac}, {0x2206, 0xcc32}, {0x2206, 0x2300}, {0x2206, 0x332d},
	{0x2206, 0x1734}, {0x2206, 0x7f52}, {0x2206, 0x3510}, {0x2206, 0x0036},
	{0x2206, 0x0600}, {0x2206, 0x370c}, {0x2206, 0xc038}, {0x2206, 0x7fce},
	{0x2206, 0x3ce5}, {0x2206, 0xf73d}, {0x2206, 0x3da4}, {0x2206, 0x6530},
	{0x2206, 0x3e67}, {0x2206, 0x0053}, {0x2206, 0x69d2}, {0x2206, 0x0f6a},
	{0x2206, 0x012c}, {0x2206, 0x6c2b}, {0x2206, 0x136e}, {0x2206, 0xe100},
	{0x2206, 0x6f12}, {0x2206, 0xf771}, {0x2206, 0x006b}, {0x2206, 0x7306},
	{0x2206, 0xeb74}, {0x2206, 0x94c7}, {0x2206, 0x7698}, {0x2206, 0x0a77},
	{0x2206, 0x5000}, {0x2206, 0x788a}, {0x2206, 0x1579}, {0x2206, 0x7f6f},
	{0x2206, 0x7a06}, {0x2206, 0xa600}, {0x2201, 0x0701}, {0x2200, 0x0405},
	{0x221f, 0x0000}, {0x2200, 0x1340}, {0x221f, 0x0000}, {0x133f, 0x0010},
	{0x133e, 0x0ffe}, {0x1203, 0xff00}, {0x1200, 0x7fc4}, {0x121d, 0x7D16},
	{0x121e, 0x03e8}, {0x121f, 0x024e}, {0x1220, 0x0230}, {0x1221, 0x0244},
	{0x1222, 0x0226}, {0x1223, 0x024e}, {0x1224, 0x0230}, {0x1225, 0x0244},
	{0x1226, 0x0226}, {0x1227, 0x00c0}, {0x1228, 0x00b4}, {0x122f, 0x00c0},
	{0x1230, 0x00b4}, {0x0208, 0x03e8}, {0x0209, 0x03e8}, {0x020a, 0x03e8},
	{0x020b, 0x03e8}, {0x020c, 0x03e8}, {0x020d, 0x03e8}, {0x020e, 0x03e8},
	{0x020f, 0x03e8}, {0x0210, 0x03e8}, {0x0211, 0x03e8}, {0x0212, 0x03e8},
	{0x0213, 0x03e8}, {0x0214, 0x03e8}, {0x0215, 0x03e8}, {0x0216, 0x03e8},
	{0x0217, 0x03e8}, {0x0900, 0x0000}, {0x0901, 0x0000}, {0x0902, 0x0000},
	{0x0903, 0x0000}, {0x0865, 0x3210}, {0x087b, 0x0000}, {0x087c, 0xff00},
	{0x087d, 0x0000}, {0x087e, 0x0000}, {0x0801, 0x0100}, {0x0802, 0x0100},
	{0x0A20, 0x2040}, {0x0A21, 0x2040}, {0x0A22, 0x2040}, {0x0A23, 0x2040},
	{0x0A24, 0x2040}, {0x0A28, 0x2040}, {0x0A29, 0x2040}, {0x20A0, 0x1940},
	{0x20C0, 0x1940}, {0x20E0, 0x1940}, {0x130c, 0x0050},
};

static const struct rtl8367_initval rtl8367_initvals_2_1[] = {
	{0x1b24, 0x0000}, {0x1b25, 0x0000}, {0x1b26, 0x0000}, {0x1b27, 0x0000},
	{0x133f, 0x0030}, {0x133e, 0x000e}, {0x221f, 0x0007}, {0x221e, 0x0048},
	{0x2219, 0x4012}, {0x221f, 0x0003}, {0x2201, 0x3554}, {0x2202, 0x63e8},
	{0x2203, 0x99c2}, {0x2204, 0x0113}, {0x2205, 0x303e}, {0x220d, 0x0207},
	{0x220e, 0xe100}, {0x221f, 0x0007}, {0x221e, 0x0040}, {0x2218, 0x0000},
	{0x221f, 0x0007}, {0x221e, 0x002c}, {0x2218, 0x008b}, {0x221f, 0x0005},
	{0x2205, 0xfff6}, {0x2206, 0x0080}, {0x221f, 0x0005}, {0x2205, 0x8000},
	{0x2206, 0x0280}, {0x2206, 0x2bf7}, {0x2206, 0x00e0}, {0x2206, 0xfff7},
	{0x2206, 0xa080}, {0x2206, 0x02ae}, {0x2206, 0xf602}, {0x2206, 0x804e},
	{0x2206, 0x0201}, {0x2206, 0x5002}, {0x2206, 0x0163}, {0x2206, 0x0201},
	{0x2206, 0x79e0}, {0x2206, 0x8b8c}, {0x2206, 0xe18b}, {0x2206, 0x8d1e},
	{0x2206, 0x01e1}, {0x2206, 0x8b8e}, {0x2206, 0x1e01}, {0x2206, 0xa000},
	{0x2206, 0xe4ae}, {0x2206, 0xd8bf}, {0x2206, 0x8b88}, {0x2206, 0xec00},
	{0x2206, 0x19a9}, {0x2206, 0x8b90}, {0x2206, 0xf9ee}, {0x2206, 0xfff6},
	{0x2206, 0x00ee}, {0x2206, 0xfff7}, {0x2206, 0xfce0}, {0x2206, 0xe140},
	{0x2206, 0xe1e1}, {0x2206, 0x41f7}, {0x2206, 0x2ff6}, {0x2206, 0x28e4},
	{0x2206, 0xe140}, {0x2206, 0xe5e1}, {0x2206, 0x4104}, {0x2206, 0xf8fa},
	{0x2206, 0xef69}, {0x2206, 0xe08b}, {0x2206, 0x86ac}, {0x2206, 0x201a},
	{0x2206, 0xbf80}, {0x2206, 0x77d0}, {0x2206, 0x6c02}, {0x2206, 0x2978},
	{0x2206, 0xe0e0}, {0x2206, 0xe4e1}, {0x2206, 0xe0e5}, {0x2206, 0x5806},
	{0x2206, 0x68c0}, {0x2206, 0xd1d2}, {0x2206, 0xe4e0}, {0x2206, 0xe4e5},
	{0x2206, 0xe0e5}, {0x2206, 0xef96}, {0x2206, 0xfefc}, {0x2206, 0x0425},
	{0x2206, 0x0807}, {0x2206, 0x2640}, {0x2206, 0x7227}, {0x2206, 0x267e},
	{0x2206, 0x2804}, {0x2206, 0xb729}, {0x2206, 0x2576}, {0x2206, 0x2a68},
	{0x2206, 0xe52b}, {0x2206, 0xad00}, {0x2206, 0x2cdb}, {0x2206, 0xf02d},
	{0x2206, 0x67bb}, {0x2206, 0x2e7b}, {0x2206, 0x0f2f}, {0x2206, 0x7365},
	{0x2206, 0x31ac}, {0x2206, 0xcc32}, {0x2206, 0x2300}, {0x2206, 0x332d},
	{0x2206, 0x1734}, {0x2206, 0x7f52}, {0x2206, 0x3510}, {0x2206, 0x0036},
	{0x2206, 0x0600}, {0x2206, 0x370c}, {0x2206, 0xc038}, {0x2206, 0x7fce},
	{0x2206, 0x3ce5}, {0x2206, 0xf73d}, {0x2206, 0x3da4}, {0x2206, 0x6530},
	{0x2206, 0x3e67}, {0x2206, 0x0053}, {0x2206, 0x69d2}, {0x2206, 0x0f6a},
	{0x2206, 0x012c}, {0x2206, 0x6c2b}, {0x2206, 0x136e}, {0x2206, 0xe100},
	{0x2206, 0x6f12}, {0x2206, 0xf771}, {0x2206, 0x006b}, {0x2206, 0x7306},
	{0x2206, 0xeb74}, {0x2206, 0x94c7}, {0x2206, 0x7698}, {0x2206, 0x0a77},
	{0x2206, 0x5000}, {0x2206, 0x788a}, {0x2206, 0x1579}, {0x2206, 0x7f6f},
	{0x2206, 0x7a06}, {0x2206, 0xa600}, {0x2201, 0x0701}, {0x2200, 0x0405},
	{0x221f, 0x0000}, {0x2200, 0x1340}, {0x221f, 0x0000}, {0x133f, 0x0010},
	{0x133e, 0x0ffe}, {0x1203, 0xff00}, {0x1200, 0x7fc4}, {0x0900, 0x0000},
	{0x0901, 0x0000}, {0x0902, 0x0000}, {0x0903, 0x0000}, {0x0865, 0x3210},
	{0x087b, 0x0000}, {0x087c, 0xff00}, {0x087d, 0x0000}, {0x087e, 0x0000},
	{0x0801, 0x0100}, {0x0802, 0x0100}, {0x0A20, 0x2040}, {0x0A21, 0x2040},
	{0x0A22, 0x2040}, {0x0A23, 0x2040}, {0x0A24, 0x2040}, {0x0A25, 0x2040},
	{0x0A26, 0x2040}, {0x0A27, 0x2040}, {0x0A28, 0x2040}, {0x0A29, 0x2040},
	{0x130c, 0x0050},
};

static int rtl8367_write_initvals(struct rtl8366_smi *smi,
				  const struct rtl8367_initval *initvals,
				  int count)
{
	int err;
	int i;

	for (i = 0; i < count; i++)
		REG_WR(smi, initvals[i].reg, initvals[i].val);

	return 0;
}

static int rtl8367_read_phy_reg(struct rtl8366_smi *smi,
				u32 phy_addr, u32 phy_reg, u32 *val)
{
	int timeout;
	u32 data;
	int err;

	if (phy_addr > RTL8367_PHY_ADDR_MAX)
		return -EINVAL;

	if (phy_reg > RTL8367_PHY_REG_MAX)
		return -EINVAL;

	REG_RD(smi, RTL8367_IA_STATUS_REG, &data);
	if (data & RTL8367_IA_STATUS_PHY_BUSY)
		return -ETIMEDOUT;

	/* prepare address */
	REG_WR(smi, RTL8367_IA_ADDRESS_REG,
	       RTL8367_INTERNAL_PHY_REG(phy_addr, phy_reg));

	/* send read command */
	REG_WR(smi, RTL8367_IA_CTRL_REG,
	       RTL8367_IA_CTRL_CMD_MASK | RTL8367_IA_CTRL_RW_READ);

	timeout = 5;
	do {
		REG_RD(smi, RTL8367_IA_STATUS_REG, &data);
		if ((data & RTL8367_IA_STATUS_PHY_BUSY) == 0)
			break;

		if (timeout--) {
			dev_err(smi->parent, "phy read timed out\n");
			return -ETIMEDOUT;
		}

		udelay(1);
	} while (1);

	/* read data */
	REG_RD(smi, RTL8367_IA_READ_DATA_REG, val);

	dev_dbg(smi->parent, "phy_read: addr:%02x, reg:%02x, val:%04x\n",
		phy_addr, phy_reg, *val);
	return 0;
}

static int rtl8367_write_phy_reg(struct rtl8366_smi *smi,
				 u32 phy_addr, u32 phy_reg, u32 val)
{
	int timeout;
	u32 data;
	int err;

	dev_dbg(smi->parent, "phy_write: addr:%02x, reg:%02x, val:%04x\n",
		phy_addr, phy_reg, val);

	if (phy_addr > RTL8367_PHY_ADDR_MAX)
		return -EINVAL;

	if (phy_reg > RTL8367_PHY_REG_MAX)
		return -EINVAL;

	REG_RD(smi, RTL8367_IA_STATUS_REG, &data);
	if (data & RTL8367_IA_STATUS_PHY_BUSY)
		return -ETIMEDOUT;

	/* preapre data */
	REG_WR(smi, RTL8367_IA_WRITE_DATA_REG, val);

	/* prepare address */
	REG_WR(smi, RTL8367_IA_ADDRESS_REG,
	       RTL8367_INTERNAL_PHY_REG(phy_addr, phy_reg));

	/* send write command */
	REG_WR(smi, RTL8367_IA_CTRL_REG,
	       RTL8367_IA_CTRL_CMD_MASK | RTL8367_IA_CTRL_RW_WRITE);

	timeout = 5;
	do {
		REG_RD(smi, RTL8367_IA_STATUS_REG, &data);
		if ((data & RTL8367_IA_STATUS_PHY_BUSY) == 0)
			break;

		if (timeout--) {
			dev_err(smi->parent, "phy write timed out\n");
			return -ETIMEDOUT;
		}

		udelay(1);
	} while (1);

	return 0;
}

static int rtl8367_init_regs0(struct rtl8366_smi *smi, unsigned mode)
{
	const struct rtl8367_initval *initvals;
	int count;
	int err;

	switch (mode) {
	case 0:
		initvals = rtl8367_initvals_0_0;
		count = ARRAY_SIZE(rtl8367_initvals_0_0);
		break;

	case 1:
	case 2:
		initvals = rtl8367_initvals_0_1;
		count = ARRAY_SIZE(rtl8367_initvals_0_1);
		break;

	default:
		dev_err(smi->parent, "%s: unknow mode %u\n", __func__, mode);
		return -ENODEV;
	}

	err = rtl8367_write_initvals(smi, initvals, count);
	if (err)
		return err;

	/* TODO: complete this */

	return 0;
}

static int rtl8367_init_regs1(struct rtl8366_smi *smi, unsigned mode)
{
	const struct rtl8367_initval *initvals;
	int count;

	switch (mode) {
	case 0:
		initvals = rtl8367_initvals_1_0;
		count = ARRAY_SIZE(rtl8367_initvals_1_0);
		break;

	case 1:
	case 2:
		initvals = rtl8367_initvals_1_1;
		count = ARRAY_SIZE(rtl8367_initvals_1_1);
		break;

	default:
		dev_err(smi->parent, "%s: unknow mode %u\n", __func__, mode);
		return -ENODEV;
	}

	return rtl8367_write_initvals(smi, initvals, count);
}

static int rtl8367_init_regs2(struct rtl8366_smi *smi, unsigned mode)
{
	const struct rtl8367_initval *initvals;
	int count;

	switch (mode) {
	case 0:
		initvals = rtl8367_initvals_2_0;
		count = ARRAY_SIZE(rtl8367_initvals_2_0);
		break;

	case 1:
	case 2:
		initvals = rtl8367_initvals_2_1;
		count = ARRAY_SIZE(rtl8367_initvals_2_1);
		break;

	default:
		dev_err(smi->parent, "%s: unknow mode %u\n", __func__, mode);
		return -ENODEV;
	}

	return rtl8367_write_initvals(smi, initvals, count);
}

static int rtl8367_init_regs(struct rtl8366_smi *smi)
{
	u32 data;
	u32 rlvid;
	u32 mode;
	int err;

	REG_WR(smi, RTL8367_RTL_MAGIC_ID_REG, RTL8367_RTL_MAGIC_ID_VAL);

	REG_RD(smi, RTL8367_CHIP_VER_REG, &data);
	rlvid = (data >> RTL8367_CHIP_VER_RLVID_SHIFT) &
		RTL8367_CHIP_VER_RLVID_MASK;

	REG_RD(smi, RTL8367_CHIP_MODE_REG, &data);
	mode = data & RTL8367_CHIP_MODE_MASK;

	switch (rlvid) {
	case 0:
		err = rtl8367_init_regs0(smi, mode);
		break;

	case 1:
		err = rtl8367_write_phy_reg(smi, 0, 31, 5);
		if (err)
			break;

		err = rtl8367_write_phy_reg(smi, 0, 5, 0x3ffe);
		if (err)
			break;

		err = rtl8367_read_phy_reg(smi, 0, 6, &data);
		if (err)
			break;

		if (data == 0x94eb) {
			err = rtl8367_init_regs1(smi, mode);
		} else if (data == 0x2104) {
			err = rtl8367_init_regs2(smi, mode);
		} else {
			dev_err(smi->parent, "unknow phy data %04x\n", data);
			return -ENODEV;
		}

		break;

	default:
		dev_err(smi->parent, "unknow rlvid %u\n", rlvid);
		err = -ENODEV;
		break;
	}

	return err;
}

static int rtl8367_reset_chip(struct rtl8366_smi *smi)
{
	int timeout = 10;
	int err;
	u32 data;

	REG_WR(smi, RTL8367_CHIP_RESET_REG, RTL8367_CHIP_RESET_HW);
	msleep(RTL8367_RESET_DELAY);

	do {
		REG_RD(smi, RTL8367_CHIP_RESET_REG, &data);
		if (!(data & RTL8367_CHIP_RESET_HW))
			break;

		msleep(1);
	} while (--timeout);

	if (!timeout) {
		dev_err(smi->parent, "chip reset timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int rtl8367_extif_set_mode(struct rtl8366_smi *smi, int id,
				  enum rtl8367_extif_mode mode)
{
	int err;

	/* set port mode */
	switch (mode) {
	case RTL8367_EXTIF_MODE_RGMII:
	case RTL8367_EXTIF_MODE_RGMII_33V:
		REG_WR(smi, RTL8367_CHIP_DEBUG0_REG, 0x0367);
		REG_WR(smi, RTL8367_CHIP_DEBUG1_REG, 0x7777);
		break;

	case RTL8367_EXTIF_MODE_TMII_MAC:
	case RTL8367_EXTIF_MODE_TMII_PHY:
		REG_RMW(smi, RTL8367_BYPASS_LINE_RATE_REG,
			BIT((id + 1) % 2), BIT((id + 1) % 2));
		break;

	case RTL8367_EXTIF_MODE_GMII:
		REG_RMW(smi, RTL8367_CHIP_DEBUG0_REG,
		        RTL8367_CHIP_DEBUG0_DUMMY0(id),
			RTL8367_CHIP_DEBUG0_DUMMY0(id));
		REG_RMW(smi, RTL8367_EXT_RGMXF_REG(id), BIT(6), BIT(6));
		break;

	case RTL8367_EXTIF_MODE_MII_MAC:
	case RTL8367_EXTIF_MODE_MII_PHY:
	case RTL8367_EXTIF_MODE_DISABLED:
		REG_RMW(smi, RTL8367_BYPASS_LINE_RATE_REG,
			BIT((id + 1) % 2), 0);
		REG_RMW(smi, RTL8367_EXT_RGMXF_REG(id), BIT(6), 0);
		break;

	default:
		dev_err(smi->parent,
			"invalid mode for external interface %d\n", id);
		return -EINVAL;
	}

	REG_RMW(smi, RTL8367_DIS_REG,
		RTL8367_DIS_RGMII_MASK << RTL8367_DIS_RGMII_SHIFT(id),
		mode << RTL8367_DIS_RGMII_SHIFT(id));

	return 0;
}

static int rtl8367_extif_set_force(struct rtl8366_smi *smi, int id,
				   struct rtl8367_port_ability *pa)
{
	u32 mask;
	u32 val;
	int err;

	mask = (RTL8367_DI_FORCE_MODE |
		RTL8367_DI_FORCE_NWAY |
		RTL8367_DI_FORCE_TXPAUSE |
		RTL8367_DI_FORCE_RXPAUSE |
		RTL8367_DI_FORCE_LINK |
		RTL8367_DI_FORCE_DUPLEX |
		RTL8367_DI_FORCE_SPEED_MASK);

	val = pa->speed;
	val |= pa->force_mode ? RTL8367_DI_FORCE_MODE : 0;
	val |= pa->nway ? RTL8367_DI_FORCE_NWAY : 0;
	val |= pa->txpause ? RTL8367_DI_FORCE_TXPAUSE : 0;
	val |= pa->rxpause ? RTL8367_DI_FORCE_RXPAUSE : 0;
	val |= pa->link ? RTL8367_DI_FORCE_LINK : 0;
	val |= pa->duplex ? RTL8367_DI_FORCE_DUPLEX : 0;

	REG_RMW(smi, RTL8367_DI_FORCE_REG(id), mask, val);

	return 0;
}

static int rtl8367_extif_set_rgmii_delay(struct rtl8366_smi *smi, int id,
					 unsigned txdelay, unsigned rxdelay)
{
	u32 mask;
	u32 val;
	int err;

	mask = (RTL8367_EXT_RGMXF_RXDELAY_MASK |
		(RTL8367_EXT_RGMXF_TXDELAY_MASK <<
			RTL8367_EXT_RGMXF_TXDELAY_SHIFT));

	val = rxdelay;
	val |= txdelay << RTL8367_EXT_RGMXF_TXDELAY_SHIFT;

	REG_RMW(smi, RTL8367_EXT_RGMXF_REG(id), mask, val);

	return 0;
}

static int rtl8367_extif_init(struct rtl8366_smi *smi, int id,
			      struct rtl8367_extif_config *cfg)
{
	enum rtl8367_extif_mode mode;
	int err;

	mode = (cfg) ? cfg->mode : RTL8367_EXTIF_MODE_DISABLED;

	err = rtl8367_extif_set_mode(smi, id, mode);
	if (err)
		return err;

	if (mode != RTL8367_EXTIF_MODE_DISABLED) {
		err = rtl8367_extif_set_force(smi, id, &cfg->ability);
		if (err)
			return err;

		err = rtl8367_extif_set_rgmii_delay(smi, id, cfg->txdelay,
						     cfg->rxdelay);
		if (err)
			return err;
	}

	return 0;
}

static int rtl8367_led_group_set_ports(struct rtl8366_smi *smi,
				       unsigned int group, u16 port_mask)
{
	u32 reg;
	u32 s;
	int err;

	port_mask &= RTL8367_PARA_LED_IO_EN_PMASK;
	s = (group % 2) * 8;
	reg = RTL8367_PARA_LED_IO_EN1_REG + (group / 2);

	REG_RMW(smi, reg, (RTL8367_PARA_LED_IO_EN_PMASK << s), port_mask << s);

	return 0;
}

static int rtl8367_led_group_set_mode(struct rtl8366_smi *smi,
				      unsigned int mode)
{
	u16 mask;
	u16 set;
	int err;

	mode &= RTL8367_LED_CONFIG_DATA_M;

	mask = (RTL8367_LED_CONFIG_DATA_M << RTL8367_LED_CONFIG_DATA_S) |
		RTL8367_LED_CONFIG_SEL;
	set = (mode << RTL8367_LED_CONFIG_DATA_S) | RTL8367_LED_CONFIG_SEL;

	REG_RMW(smi, RTL8367_LED_CONFIG_REG, mask, set);

	return 0;
}

static int rtl8367_led_group_set_config(struct rtl8366_smi *smi,
				        unsigned int led, unsigned int cfg)
{
	u16 mask;
	u16 set;
	int err;

	mask = (RTL8367_LED_CONFIG_LED_CFG_M << (led * 4)) |
		RTL8367_LED_CONFIG_SEL;
	set = (cfg & RTL8367_LED_CONFIG_LED_CFG_M) << (led * 4);

	REG_RMW(smi, RTL8367_LED_CONFIG_REG, mask, set);
	return 0;
}

static int rtl8367_led_op_select_parallel(struct rtl8366_smi *smi)
{
	int err;

	REG_WR(smi, RTL8367_LED_SYS_CONFIG_REG, 0x1472);
	return 0;
}

static int rtl8367_led_blinkrate_set(struct rtl8366_smi *smi, unsigned int rate)
{
	u16 mask;
	u16 set;
	int err;

	mask = RTL8367_LED_MODE_RATE_M << RTL8367_LED_MODE_RATE_S;
	set = (rate & RTL8367_LED_MODE_RATE_M) << RTL8367_LED_MODE_RATE_S;
	REG_RMW(smi, RTL8367_LED_MODE_REG, mask, set);

	return 0;
}

#ifdef CONFIG_OF
static int rtl8367_extif_init_of(struct rtl8366_smi *smi,
				 const char *name)
{
	struct rtl8367_extif_config *cfg;
	const __be32 *prop;
	int size;
	int err;
	unsigned cpu_port;
	unsigned id = UINT_MAX;

	prop = of_get_property(smi->parent->of_node, name, &size);
	if (!prop || (size != (10 * sizeof(*prop)))) {
		dev_err(smi->parent, "%s property is not defined or invalid\n", name);
		err = -EINVAL;
		goto err_init;
	}

	cpu_port = be32_to_cpup(prop++);
	switch (cpu_port) {
	case RTL8367_CPU_PORT_NUM - 1:
	case RTL8367_CPU_PORT_NUM:
		id = RTL8367_CPU_PORT_NUM - cpu_port;
		if (smi->cpu_port == UINT_MAX) {
			dev_info(smi->parent, "cpu_port:%u, assigned to extif%u\n", cpu_port, id);
			smi->cpu_port = cpu_port;
		}
		break;
	default:
		dev_err(smi->parent, "wrong cpu_port %u in %s property\n", cpu_port, name);
		err = -EINVAL;
		goto err_init;
	}

	cfg = kzalloc(sizeof(struct rtl8367_extif_config), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	cfg->txdelay = be32_to_cpup(prop++);
	cfg->rxdelay = be32_to_cpup(prop++);
	cfg->mode = be32_to_cpup(prop++);
	cfg->ability.force_mode = be32_to_cpup(prop++);
	cfg->ability.txpause = be32_to_cpup(prop++);
	cfg->ability.rxpause = be32_to_cpup(prop++);
	cfg->ability.link = be32_to_cpup(prop++);
	cfg->ability.duplex = be32_to_cpup(prop++);
	cfg->ability.speed = be32_to_cpup(prop++);

	err = rtl8367_extif_init(smi, id, cfg);
	kfree(cfg);

err_init:
	if (id != 0) rtl8367_extif_init(smi, 0, NULL);
	if (id != 1) rtl8367_extif_init(smi, 1, NULL);

	return err;
}
#else
static int rtl8367_extif_init_of(struct rtl8366_smi *smi,
				 const char *name)
{
	return -EINVAL;
}
#endif

static int rtl8367_setup(struct rtl8366_smi *smi)
{
	struct rtl8367_platform_data *pdata;
	int err;
	int i;

	pdata = smi->parent->platform_data;

	err = rtl8367_init_regs(smi);
	if (err)
		return err;

	/* initialize external interfaces */
	if (smi->parent->of_node) {
		err = rtl8367_extif_init_of(smi, "realtek,extif");
		if (err)
			return err;
	} else {
		err = rtl8367_extif_init(smi, 0, pdata->extif0_cfg);
		if (err)
			return err;

		err = rtl8367_extif_init(smi, 1, pdata->extif1_cfg);
		if (err)
			return err;
	}

	/* set maximum packet length to 1536 bytes */
	REG_RMW(smi, RTL8367_SWC0_REG, RTL8367_SWC0_MAX_LENGTH_MASK,
		RTL8367_SWC0_MAX_LENGTH_1536);

	/*
	 * discard VLAN tagged packets if the port is not a member of
	 * the VLAN with which the packets is associated.
	 */
	REG_WR(smi, RTL8367_VLAN_INGRESS_REG, RTL8367_PORTS_ALL);

	/*
	 * Setup egress tag mode for each port.
	 */
	for (i = 0; i < RTL8367_NUM_PORTS; i++)
		REG_RMW(smi,
			RTL8367_PORT_CFG_REG(i),
			RTL8367_PORT_CFG_EGRESS_MODE_MASK <<
				RTL8367_PORT_CFG_EGRESS_MODE_SHIFT,
			RTL8367_PORT_CFG_EGRESS_MODE_ORIGINAL <<
				RTL8367_PORT_CFG_EGRESS_MODE_SHIFT);

	/* setup LEDs */
	err = rtl8367_led_group_set_ports(smi, 0, RTL8367_PORTS_ALL);
	if (err)
		return err;

	err = rtl8367_led_group_set_mode(smi, 0);
	if (err)
		return err;

	err = rtl8367_led_op_select_parallel(smi);
	if (err)
		return err;

	err = rtl8367_led_blinkrate_set(smi, 1);
	if (err)
		return err;

	err = rtl8367_led_group_set_config(smi, 0, 2);
	if (err)
		return err;

	return 0;
}

static int rtl8367_get_mib_counter(struct rtl8366_smi *smi, int counter,
				   int port, unsigned long long *val)
{
	struct rtl8366_mib_counter *mib;
	int offset;
	int i;
	int err;
	u32 addr, data;
	u64 mibvalue;

	if (port > RTL8367_NUM_PORTS || counter >= RTL8367_MIB_COUNT)
		return -EINVAL;

	mib = &rtl8367_mib_counters[counter];
	addr = RTL8367_MIB_COUNTER_PORT_OFFSET * port + mib->offset;

	/*
	 * Writing access counter address first
	 * then ASIC will prepare 64bits counter wait for being retrived
	 */
	REG_WR(smi, RTL8367_MIB_ADDRESS_REG, addr >> 2);

	/* read MIB control register */
	REG_RD(smi, RTL8367_MIB_CTRL_REG(0), &data);

	if (data & RTL8367_MIB_CTRL_BUSY_MASK)
		return -EBUSY;

	if (data & RTL8367_MIB_CTRL_RESET_MASK)
		return -EIO;

	if (mib->length == 4)
		offset = 3;
	else
		offset = (mib->offset + 1) % 4;

	mibvalue = 0;
	for (i = 0; i < mib->length; i++) {
		REG_RD(smi, RTL8367_MIB_COUNTER_REG(offset - i), &data);
		mibvalue = (mibvalue << 16) | (data & 0xFFFF);
	}

	*val = mibvalue;
	return 0;
}

static int rtl8367_get_vlan_4k(struct rtl8366_smi *smi, u32 vid,
				struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[RTL8367_TA_VLAN_DATA_SIZE];
	int err;
	int i;

	memset(vlan4k, '\0', sizeof(struct rtl8366_vlan_4k));

	if (vid >= RTL8367_NUM_VIDS)
		return -EINVAL;

	/* write VID */
	REG_WR(smi, RTL8367_TA_ADDR_REG, vid);

	/* write table access control word */
	REG_WR(smi, RTL8367_TA_CTRL_REG, RTL8367_TA_CTRL_CVLAN_READ);

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_RD(smi, RTL8367_TA_DATA_REG(i), &data[i]);

	vlan4k->vid = vid;
	vlan4k->member = (data[0] >> RTL8367_TA_VLAN_MEMBER_SHIFT) &
			 RTL8367_TA_VLAN_MEMBER_MASK;
	vlan4k->fid = (data[1] >> RTL8367_TA_VLAN_FID_SHIFT) &
		      RTL8367_TA_VLAN_FID_MASK;
	vlan4k->untag = (data[2] >> RTL8367_TA_VLAN_UNTAG1_SHIFT) &
			RTL8367_TA_VLAN_UNTAG1_MASK;
	vlan4k->untag |= ((data[3] >> RTL8367_TA_VLAN_UNTAG2_SHIFT) &
			  RTL8367_TA_VLAN_UNTAG2_MASK) << 2;

	return 0;
}

static int rtl8367_set_vlan_4k(struct rtl8366_smi *smi,
				const struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[RTL8367_TA_VLAN_DATA_SIZE];
	int err;
	int i;

	if (vlan4k->vid >= RTL8367_NUM_VIDS ||
	    vlan4k->member > RTL8367_TA_VLAN_MEMBER_MASK ||
	    vlan4k->untag > RTL8367_UNTAG_MASK ||
	    vlan4k->fid > RTL8367_FIDMAX)
		return -EINVAL;

	data[0] = (vlan4k->member & RTL8367_TA_VLAN_MEMBER_MASK) <<
		  RTL8367_TA_VLAN_MEMBER_SHIFT;
	data[1] = (vlan4k->fid & RTL8367_TA_VLAN_FID_MASK) <<
		  RTL8367_TA_VLAN_FID_SHIFT;
	data[2] = (vlan4k->untag & RTL8367_TA_VLAN_UNTAG1_MASK) <<
		  RTL8367_TA_VLAN_UNTAG1_SHIFT;
	data[3] = ((vlan4k->untag >> 2) & RTL8367_TA_VLAN_UNTAG2_MASK) <<
		  RTL8367_TA_VLAN_UNTAG2_SHIFT;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_WR(smi, RTL8367_TA_DATA_REG(i), data[i]);

	/* write VID */
	REG_WR(smi, RTL8367_TA_ADDR_REG,
	       vlan4k->vid & RTL8367_TA_VLAN_VID_MASK);

	/* write table access control word */
	REG_WR(smi, RTL8367_TA_CTRL_REG, RTL8367_TA_CTRL_CVLAN_WRITE);

	return 0;
}

static int rtl8367_get_vlan_mc(struct rtl8366_smi *smi, u32 index,
				struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[RTL8367_VLAN_MC_DATA_SIZE];
	int err;
	int i;

	memset(vlanmc, '\0', sizeof(struct rtl8366_vlan_mc));

	if (index >= RTL8367_NUM_VLANS)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_RD(smi, RTL8367_VLAN_MC_BASE(index) + i, &data[i]);

	vlanmc->member = (data[0] >> RTL8367_VLAN_MC_MEMBER_SHIFT) &
			 RTL8367_VLAN_MC_MEMBER_MASK;
	vlanmc->fid = (data[1] >> RTL8367_VLAN_MC_FID_SHIFT) &
		      RTL8367_VLAN_MC_FID_MASK;
	vlanmc->vid = (data[3] >> RTL8367_VLAN_MC_EVID_SHIFT) &
		      RTL8367_VLAN_MC_EVID_MASK;

	return 0;
}

static int rtl8367_set_vlan_mc(struct rtl8366_smi *smi, u32 index,
				const struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[RTL8367_VLAN_MC_DATA_SIZE];
	int err;
	int i;

	if (index >= RTL8367_NUM_VLANS ||
	    vlanmc->vid >= RTL8367_NUM_VIDS ||
	    vlanmc->priority > RTL8367_PRIORITYMAX ||
	    vlanmc->member > RTL8367_VLAN_MC_MEMBER_MASK ||
	    vlanmc->untag > RTL8367_UNTAG_MASK ||
	    vlanmc->fid > RTL8367_FIDMAX)
		return -EINVAL;

	data[0] = (vlanmc->member & RTL8367_VLAN_MC_MEMBER_MASK) <<
		  RTL8367_VLAN_MC_MEMBER_SHIFT;
	data[1] = (vlanmc->fid & RTL8367_VLAN_MC_FID_MASK) <<
		  RTL8367_VLAN_MC_FID_SHIFT;
	data[2] = 0;
	data[3] = (vlanmc->vid & RTL8367_VLAN_MC_EVID_MASK) <<
		   RTL8367_VLAN_MC_EVID_SHIFT;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_WR(smi, RTL8367_VLAN_MC_BASE(index) + i, data[i]);

	return 0;
}

static int rtl8367_get_mc_index(struct rtl8366_smi *smi, int port, int *val)
{
	u32 data;
	int err;

	if (port >= RTL8367_NUM_PORTS)
		return -EINVAL;

	REG_RD(smi, RTL8367_VLAN_PVID_CTRL_REG(port), &data);

	*val = (data >> RTL8367_VLAN_PVID_CTRL_SHIFT(port)) &
	       RTL8367_VLAN_PVID_CTRL_MASK;

	return 0;
}

static int rtl8367_set_mc_index(struct rtl8366_smi *smi, int port, int index)
{
	if (port >= RTL8367_NUM_PORTS || index >= RTL8367_NUM_VLANS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8367_VLAN_PVID_CTRL_REG(port),
				RTL8367_VLAN_PVID_CTRL_MASK <<
					RTL8367_VLAN_PVID_CTRL_SHIFT(port),
				(index & RTL8367_VLAN_PVID_CTRL_MASK) <<
					RTL8367_VLAN_PVID_CTRL_SHIFT(port));
}

static int rtl8367_enable_vlan(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8367_VLAN_CTRL_REG,
				RTL8367_VLAN_CTRL_ENABLE,
				(enable) ? RTL8367_VLAN_CTRL_ENABLE : 0);
}

static int rtl8367_enable_vlan4k(struct rtl8366_smi *smi, int enable)
{
	return 0;
}

static int rtl8367_is_vlan_valid(struct rtl8366_smi *smi, unsigned vlan)
{
	unsigned max = RTL8367_NUM_VLANS;

	if (smi->vlan4k_enabled)
		max = RTL8367_NUM_VIDS - 1;

	if (vlan == 0 || vlan >= max)
		return 0;

	return 1;
}

static int rtl8367_enable_port(struct rtl8366_smi *smi, int port, int enable)
{
	int err;

	REG_WR(smi, RTL8367_PORT_ISOLATION_REG(port),
	       (enable) ? RTL8367_PORTS_ALL : 0);

	return 0;
}

static int rtl8367_sw_reset_mibs(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	return rtl8366_smi_rmwr(smi, RTL8367_MIB_CTRL_REG(0), 0,
				RTL8367_MIB_CTRL_GLOBAL_RESET_MASK);
}

static int rtl8367_sw_get_port_link(struct switch_dev *dev,
				    int port,
				    struct switch_port_link *link)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;
	u32 speed;

	if (port >= RTL8367_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8367_PORT_STATUS_REG(port), &data);

	link->link = !!(data & RTL8367_PORT_STATUS_LINK);
	if (!link->link)
		return 0;

	link->duplex = !!(data & RTL8367_PORT_STATUS_DUPLEX);
	link->rx_flow = !!(data & RTL8367_PORT_STATUS_RXPAUSE);
	link->tx_flow = !!(data & RTL8367_PORT_STATUS_TXPAUSE);
	link->aneg = !!(data & RTL8367_PORT_STATUS_NWAY);

	speed = (data & RTL8367_PORT_STATUS_SPEED_MASK);
	switch (speed) {
	case 0:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case 1:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case 2:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	default:
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static int rtl8367_sw_get_max_length(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8367_SWC0_REG, &data);
	val->value.i = (data & RTL8367_SWC0_MAX_LENGTH_MASK) >>
			RTL8367_SWC0_MAX_LENGTH_SHIFT;

	return 0;
}

static int rtl8367_sw_set_max_length(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 max_len;

	switch (val->value.i) {
	case 0:
		max_len = RTL8367_SWC0_MAX_LENGTH_1522;
		break;
	case 1:
		max_len = RTL8367_SWC0_MAX_LENGTH_1536;
		break;
	case 2:
		max_len = RTL8367_SWC0_MAX_LENGTH_1552;
		break;
	case 3:
		max_len = RTL8367_SWC0_MAX_LENGTH_16000;
		break;
	default:
		return -EINVAL;
	}

	return rtl8366_smi_rmwr(smi, RTL8367_SWC0_REG,
			        RTL8367_SWC0_MAX_LENGTH_MASK, max_len);
}


static int rtl8367_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int port;

	port = val->port_vlan;
	if (port >= RTL8367_NUM_PORTS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8367_MIB_CTRL_REG(port / 8), 0,
				RTL8367_MIB_CTRL_PORT_RESET_MASK(port % 8));
}

static int rtl8367_sw_get_port_stats(struct switch_dev *dev, int port,
                                        struct switch_port_stats *stats)
{
	return (rtl8366_sw_get_port_stats(dev, port, stats,
				RTL8367_MIB_TXB_ID, RTL8367_MIB_RXB_ID));
}

static struct switch_attr rtl8367_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = rtl8366_sw_set_vlan_enable,
		.get = rtl8366_sw_get_vlan_enable,
		.max = 1,
		.ofs = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan4k",
		.description = "Enable VLAN 4K mode",
		.set = rtl8366_sw_set_vlan_enable,
		.get = rtl8366_sw_get_vlan_enable,
		.max = 1,
		.ofs = 2
	}, {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = rtl8367_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "max_length",
		.description = "Get/Set the maximum length of valid packets"
			       "(0:1522, 1:1536, 2:1552, 3:16000)",
		.set = rtl8367_sw_set_max_length,
		.get = rtl8367_sw_get_max_length,
		.max = 3,
	}
};

static struct switch_attr rtl8367_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl8367_sw_reset_port_mibs,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.max = 33,
		.set = NULL,
		.get = rtl8366_sw_get_port_mib,
	},
};

static struct switch_attr rtl8367_vlan[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "info",
		.description = "Get vlan information",
		.max = 1,
		.set = NULL,
		.get = rtl8366_sw_get_vlan_info,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "fid",
		.description = "Get/Set vlan FID",
		.max = RTL8367_FIDMAX,
		.set = rtl8366_sw_set_vlan_fid,
		.get = rtl8366_sw_get_vlan_fid,
	},
};

static const struct switch_dev_ops rtl8367_sw_ops = {
	.attr_global = {
		.attr = rtl8367_globals,
		.n_attr = ARRAY_SIZE(rtl8367_globals),
	},
	.attr_port = {
		.attr = rtl8367_port,
		.n_attr = ARRAY_SIZE(rtl8367_port),
	},
	.attr_vlan = {
		.attr = rtl8367_vlan,
		.n_attr = ARRAY_SIZE(rtl8367_vlan),
	},

	.get_vlan_ports = rtl8366_sw_get_vlan_ports,
	.set_vlan_ports = rtl8366_sw_set_vlan_ports,
	.get_port_pvid = rtl8366_sw_get_port_pvid,
	.set_port_pvid = rtl8366_sw_set_port_pvid,
	.reset_switch = rtl8366_sw_reset_switch,
	.get_port_link = rtl8367_sw_get_port_link,
	.get_port_stats = rtl8367_sw_get_port_stats,
};

static int rtl8367_switch_init(struct rtl8366_smi *smi)
{
	struct switch_dev *dev = &smi->sw_dev;
	int err;

	dev->name = "RTL8367";
	dev->cpu_port = smi->cpu_port;
	dev->ports = RTL8367_NUM_PORTS;
	dev->vlans = RTL8367_NUM_VIDS;
	dev->ops = &rtl8367_sw_ops;
	dev->alias = dev_name(smi->parent);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(smi->parent, "switch registration failed\n");

	return err;
}

static void rtl8367_switch_cleanup(struct rtl8366_smi *smi)
{
	unregister_switch(&smi->sw_dev);
}

static int rtl8367_mii_read(struct mii_bus *bus, int addr, int reg)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 val = 0;
	int err;

	err = rtl8367_read_phy_reg(smi, addr, reg, &val);
	if (err)
		return 0xffff;

	return val;
}

static int rtl8367_mii_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 t;
	int err;

	err = rtl8367_write_phy_reg(smi, addr, reg, val);
	if (err)
		return err;

	/* flush write */
	(void) rtl8367_read_phy_reg(smi, addr, reg, &t);

	return err;
}

static int rtl8367_detect(struct rtl8366_smi *smi)
{
	u32 rtl_no = 0;
	u32 rtl_ver = 0;
	char *chip_name;
	int ret;

	ret = rtl8366_smi_read_reg(smi, RTL8367_RTL_NO_REG, &rtl_no);
	if (ret) {
		dev_err(smi->parent, "unable to read chip number\n");
		return ret;
	}

	switch (rtl_no) {
	case RTL8367_RTL_NO_8367R:
		chip_name = "8367R";
		break;
	case RTL8367_RTL_NO_8367M:
		chip_name = "8367M";
		break;
	default:
		dev_err(smi->parent, "unknown chip number (%04x)\n", rtl_no);
		return -ENODEV;
	}

	ret = rtl8366_smi_read_reg(smi, RTL8367_RTL_VER_REG, &rtl_ver);
	if (ret) {
		dev_err(smi->parent, "unable to read chip version\n");
		return ret;
	}

	dev_info(smi->parent, "RTL%s ver. %u chip found\n",
		 chip_name, rtl_ver & RTL8367_RTL_VER_MASK);

	return 0;
}

static struct rtl8366_smi_ops rtl8367_smi_ops = {
	.detect		= rtl8367_detect,
	.reset_chip	= rtl8367_reset_chip,
	.setup		= rtl8367_setup,

	.mii_read	= rtl8367_mii_read,
	.mii_write	= rtl8367_mii_write,

	.get_vlan_mc	= rtl8367_get_vlan_mc,
	.set_vlan_mc	= rtl8367_set_vlan_mc,
	.get_vlan_4k	= rtl8367_get_vlan_4k,
	.set_vlan_4k	= rtl8367_set_vlan_4k,
	.get_mc_index	= rtl8367_get_mc_index,
	.set_mc_index	= rtl8367_set_mc_index,
	.get_mib_counter = rtl8367_get_mib_counter,
	.is_vlan_valid	= rtl8367_is_vlan_valid,
	.enable_vlan	= rtl8367_enable_vlan,
	.enable_vlan4k	= rtl8367_enable_vlan4k,
	.enable_port	= rtl8367_enable_port,
};

static int rtl8367_probe(struct platform_device *pdev)
{
	struct rtl8366_smi *smi;
	int err;

	smi = rtl8366_smi_probe(pdev);
	if (IS_ERR(smi))
		return PTR_ERR(smi);

	smi->clk_delay = 1500;
	smi->cmd_read = 0xb9;
	smi->cmd_write = 0xb8;
	smi->ops = &rtl8367_smi_ops;
	smi->cpu_port = UINT_MAX; /* not defined yet */
	smi->num_ports = RTL8367_NUM_PORTS;
	smi->num_vlan_mc = RTL8367_NUM_VLANS;
	smi->mib_counters = rtl8367_mib_counters;
	smi->num_mib_counters = ARRAY_SIZE(rtl8367_mib_counters);

	err = rtl8366_smi_init(smi);
	if (err)
		goto err_free_smi;

	platform_set_drvdata(pdev, smi);

	err = rtl8367_switch_init(smi);
	if (err)
		goto err_clear_drvdata;

	return 0;

 err_clear_drvdata:
	platform_set_drvdata(pdev, NULL);
	rtl8366_smi_cleanup(smi);
 err_free_smi:
	kfree(smi);
	return err;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,11,0)
static int rtl8367_remove(struct platform_device *pdev)
#else
static void rtl8367_remove(struct platform_device *pdev)
#endif
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi) {
		rtl8367_switch_cleanup(smi);
		platform_set_drvdata(pdev, NULL);
		rtl8366_smi_cleanup(smi);
		kfree(smi);
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,11,0)
	return 0;
#endif
}

static void rtl8367_shutdown(struct platform_device *pdev)
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi)
		rtl8367_reset_chip(smi);
}

#ifdef CONFIG_OF
static const struct of_device_id rtl8367_match[] = {
       { .compatible = "realtek,rtl8367" },
       {},
};
MODULE_DEVICE_TABLE(of, rtl8367_match);
#endif

static struct platform_driver rtl8367_driver = {
	.driver = {
		.name		= RTL8367_DRIVER_NAME,
		.owner		= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(rtl8367_match),
#endif
	},
	.probe		= rtl8367_probe,
	.remove		= rtl8367_remove,
	.shutdown	= rtl8367_shutdown,
};

static int __init rtl8367_module_init(void)
{
	return platform_driver_register(&rtl8367_driver);
}
module_init(rtl8367_module_init);

static void __exit rtl8367_module_exit(void)
{
	platform_driver_unregister(&rtl8367_driver);
}
module_exit(rtl8367_module_exit);

MODULE_DESCRIPTION("Realtek RTL8367 ethernet switch driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RTL8367_DRIVER_NAME);
