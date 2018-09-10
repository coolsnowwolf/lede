/*
 * Platform driver for the Realtek RTL8367R-VB ethernet switches
 *
 * Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
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

#include "rtl8366_smi.h"

#define RTL8367B_RESET_DELAY	1000	/* msecs*/

#define RTL8367B_PHY_ADDR_MAX	8
#define RTL8367B_PHY_REG_MAX	31

#define RTL8367B_VID_MASK	0x3fff
#define RTL8367B_FID_MASK	0xf
#define RTL8367B_UNTAG_MASK	0xff
#define RTL8367B_MEMBER_MASK	0xff

#define RTL8367B_PORT_MISC_CFG_REG(_p)		(0x000e + 0x20 * (_p))
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_SHIFT	4
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_MASK	0x3
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_ORIGINAL	0
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_KEEP	1
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_PRI	2
#define   RTL8367B_PORT_MISC_CFG_EGRESS_MODE_REAL	3

#define RTL8367B_BYPASS_LINE_RATE_REG		0x03f7

#define RTL8367B_TA_CTRL_REG			0x0500 /*GOOD*/
#define   RTL8367B_TA_CTRL_SPA_SHIFT		8
#define   RTL8367B_TA_CTRL_SPA_MASK		0x7
#define   RTL8367B_TA_CTRL_METHOD		BIT(4)/*GOOD*/
#define   RTL8367B_TA_CTRL_CMD_SHIFT		3
#define   RTL8367B_TA_CTRL_CMD_READ		0
#define   RTL8367B_TA_CTRL_CMD_WRITE		1
#define   RTL8367B_TA_CTRL_TABLE_SHIFT		0 /*GOOD*/
#define   RTL8367B_TA_CTRL_TABLE_ACLRULE	1
#define   RTL8367B_TA_CTRL_TABLE_ACLACT		2
#define   RTL8367B_TA_CTRL_TABLE_CVLAN		3
#define   RTL8367B_TA_CTRL_TABLE_L2		4
#define   RTL8367B_TA_CTRL_CVLAN_READ \
		((RTL8367B_TA_CTRL_CMD_READ << RTL8367B_TA_CTRL_CMD_SHIFT) | \
		 RTL8367B_TA_CTRL_TABLE_CVLAN)
#define   RTL8367B_TA_CTRL_CVLAN_WRITE \
		((RTL8367B_TA_CTRL_CMD_WRITE << RTL8367B_TA_CTRL_CMD_SHIFT) | \
		 RTL8367B_TA_CTRL_TABLE_CVLAN)

#define RTL8367B_TA_ADDR_REG			0x0501/*GOOD*/
#define   RTL8367B_TA_ADDR_MASK			0x3fff/*GOOD*/

#define RTL8367B_TA_LUT_REG			0x0502/*GOOD*/

#define RTL8367B_TA_WRDATA_REG(_x)		(0x0510 + (_x))/*GOOD*/
#define   RTL8367B_TA_VLAN_NUM_WORDS		2
#define   RTL8367B_TA_VLAN_VID_MASK		RTL8367B_VID_MASK
#define   RTL8367B_TA_VLAN0_MEMBER_SHIFT	0
#define   RTL8367B_TA_VLAN0_MEMBER_MASK		RTL8367B_MEMBER_MASK
#define   RTL8367B_TA_VLAN0_UNTAG_SHIFT		8
#define   RTL8367B_TA_VLAN0_UNTAG_MASK		RTL8367B_MEMBER_MASK
#define   RTL8367B_TA_VLAN1_FID_SHIFT		0
#define   RTL8367B_TA_VLAN1_FID_MASK		RTL8367B_FID_MASK

#define RTL8367B_TA_RDDATA_REG(_x)		(0x0520 + (_x))/*GOOD*/

#define RTL8367B_VLAN_PVID_CTRL_REG(_p)		(0x0700 + (_p) / 2) /*GOOD*/
#define RTL8367B_VLAN_PVID_CTRL_MASK		0x1f /*GOOD*/
#define RTL8367B_VLAN_PVID_CTRL_SHIFT(_p)	(8 * ((_p) % 2)) /*GOOD*/

#define RTL8367B_VLAN_MC_BASE(_x)		(0x0728 + (_x) * 4) /*GOOD*/
#define   RTL8367B_VLAN_MC_NUM_WORDS		4 /*GOOD*/
#define   RTL8367B_VLAN_MC0_MEMBER_SHIFT	0/*GOOD*/
#define   RTL8367B_VLAN_MC0_MEMBER_MASK		RTL8367B_MEMBER_MASK/*GOOD*/
#define   RTL8367B_VLAN_MC1_FID_SHIFT		0/*GOOD*/
#define   RTL8367B_VLAN_MC1_FID_MASK		RTL8367B_FID_MASK/*GOOD*/
#define   RTL8367B_VLAN_MC3_EVID_SHIFT		0/*GOOD*/
#define   RTL8367B_VLAN_MC3_EVID_MASK		RTL8367B_VID_MASK/*GOOD*/

#define RTL8367B_VLAN_CTRL_REG			0x07a8 /*GOOD*/
#define   RTL8367B_VLAN_CTRL_ENABLE		BIT(0)

#define RTL8367B_VLAN_INGRESS_REG		0x07a9 /*GOOD*/

#define RTL8367B_PORT_ISOLATION_REG(_p)		(0x08a2 + (_p)) /*GOOD*/

#define RTL8367B_MIB_COUNTER_REG(_x)		(0x1000 + (_x))	/*GOOD*/
#define RTL8367B_MIB_COUNTER_PORT_OFFSET	0x007c /*GOOD*/

#define RTL8367B_MIB_ADDRESS_REG		0x1004 /*GOOD*/

#define RTL8367B_MIB_CTRL0_REG(_x)		(0x1005 + (_x)) /*GOOD*/
#define   RTL8367B_MIB_CTRL0_GLOBAL_RESET_MASK	BIT(11)	/*GOOD*/
#define   RTL8367B_MIB_CTRL0_QM_RESET_MASK	BIT(10) /*GOOD*/
#define   RTL8367B_MIB_CTRL0_PORT_RESET_MASK(_p) BIT(2 + (_p)) /*GOOD*/
#define   RTL8367B_MIB_CTRL0_RESET_MASK		BIT(1) /*GOOD*/
#define   RTL8367B_MIB_CTRL0_BUSY_MASK		BIT(0) /*GOOD*/

#define RTL8367B_SWC0_REG			0x1200/*GOOD*/
#define   RTL8367B_SWC0_MAX_LENGTH_SHIFT	13/*GOOD*/
#define   RTL8367B_SWC0_MAX_LENGTH(_x)		((_x) << 13) /*GOOD*/
#define   RTL8367B_SWC0_MAX_LENGTH_MASK		RTL8367B_SWC0_MAX_LENGTH(0x3)
#define   RTL8367B_SWC0_MAX_LENGTH_1522		RTL8367B_SWC0_MAX_LENGTH(0)
#define   RTL8367B_SWC0_MAX_LENGTH_1536		RTL8367B_SWC0_MAX_LENGTH(1)
#define   RTL8367B_SWC0_MAX_LENGTH_1552		RTL8367B_SWC0_MAX_LENGTH(2)
#define   RTL8367B_SWC0_MAX_LENGTH_16000	RTL8367B_SWC0_MAX_LENGTH(3)

#define RTL8367B_CHIP_NUMBER_REG		0x1300/*GOOD*/

#define RTL8367B_CHIP_VER_REG			0x1301/*GOOD*/
#define   RTL8367B_CHIP_VER_RLVID_SHIFT		12/*GOOD*/
#define   RTL8367B_CHIP_VER_RLVID_MASK		0xf/*GOOD*/
#define   RTL8367B_CHIP_VER_MCID_SHIFT		8/*GOOD*/
#define   RTL8367B_CHIP_VER_MCID_MASK		0xf/*GOOD*/
#define   RTL8367B_CHIP_VER_BOID_SHIFT		4/*GOOD*/
#define   RTL8367B_CHIP_VER_BOID_MASK		0xf/*GOOD*/
#define   RTL8367B_CHIP_VER_AFE_SHIFT		0/*GOOD*/
#define   RTL8367B_CHIP_VER_AFE_MASK		0x1/*GOOD*/

#define RTL8367B_CHIP_MODE_REG			0x1302
#define   RTL8367B_CHIP_MODE_MASK		0x7

#define RTL8367B_CHIP_DEBUG0_REG		0x1303
#define   RTL8367B_CHIP_DEBUG0_DUMMY0(_x)	BIT(8 + (_x))

#define RTL8367B_CHIP_DEBUG1_REG		0x1304

#define RTL8367B_DIS_REG			0x1305
#define   RTL8367B_DIS_SKIP_MII_RXER(_x)	BIT(12 + (_x))
#define   RTL8367B_DIS_RGMII_SHIFT(_x)		(4 * (_x))
#define   RTL8367B_DIS_RGMII_MASK		0x7

#define RTL8367B_EXT_RGMXF_REG(_x)		(0x1306 + (_x))
#define   RTL8367B_EXT_RGMXF_DUMMY0_SHIFT	5
#define   RTL8367B_EXT_RGMXF_DUMMY0_MASK	0x7ff
#define   RTL8367B_EXT_RGMXF_TXDELAY_SHIFT	3
#define   RTL8367B_EXT_RGMXF_TXDELAY_MASK	1
#define   RTL8367B_EXT_RGMXF_RXDELAY_MASK	0x7

#define RTL8367B_DI_FORCE_REG(_x)		(0x1310 + (_x))
#define   RTL8367B_DI_FORCE_MODE		BIT(12)
#define   RTL8367B_DI_FORCE_NWAY		BIT(7)
#define   RTL8367B_DI_FORCE_TXPAUSE		BIT(6)
#define   RTL8367B_DI_FORCE_RXPAUSE		BIT(5)
#define   RTL8367B_DI_FORCE_LINK		BIT(4)
#define   RTL8367B_DI_FORCE_DUPLEX		BIT(2)
#define   RTL8367B_DI_FORCE_SPEED_MASK		3
#define   RTL8367B_DI_FORCE_SPEED_10		0
#define   RTL8367B_DI_FORCE_SPEED_100		1
#define   RTL8367B_DI_FORCE_SPEED_1000		2

#define RTL8367B_MAC_FORCE_REG(_x)		(0x1312 + (_x))

#define RTL8367B_CHIP_RESET_REG			0x1322 /*GOOD*/
#define   RTL8367B_CHIP_RESET_SW		BIT(1) /*GOOD*/
#define   RTL8367B_CHIP_RESET_HW		BIT(0) /*GOOD*/

#define RTL8367B_PORT_STATUS_REG(_p)		(0x1352 + (_p)) /*GOOD*/
#define   RTL8367B_PORT_STATUS_EN_1000_SPI	BIT(11) /*GOOD*/
#define   RTL8367B_PORT_STATUS_EN_100_SPI	BIT(10)/*GOOD*/
#define   RTL8367B_PORT_STATUS_NWAY_FAULT	BIT(9)/*GOOD*/
#define   RTL8367B_PORT_STATUS_LINK_MASTER	BIT(8)/*GOOD*/
#define   RTL8367B_PORT_STATUS_NWAY		BIT(7)/*GOOD*/
#define   RTL8367B_PORT_STATUS_TXPAUSE		BIT(6)/*GOOD*/
#define   RTL8367B_PORT_STATUS_RXPAUSE		BIT(5)/*GOOD*/
#define   RTL8367B_PORT_STATUS_LINK		BIT(4)/*GOOD*/
#define   RTL8367B_PORT_STATUS_DUPLEX		BIT(2)/*GOOD*/
#define   RTL8367B_PORT_STATUS_SPEED_MASK	0x0003/*GOOD*/
#define   RTL8367B_PORT_STATUS_SPEED_10		0/*GOOD*/
#define   RTL8367B_PORT_STATUS_SPEED_100	1/*GOOD*/
#define   RTL8367B_PORT_STATUS_SPEED_1000	2/*GOOD*/

#define RTL8367B_RTL_MAGIC_ID_REG		0x13c2
#define   RTL8367B_RTL_MAGIC_ID_VAL		0x0249

#define RTL8367B_IA_CTRL_REG			0x1f00
#define   RTL8367B_IA_CTRL_RW(_x)		((_x) << 1)
#define   RTL8367B_IA_CTRL_RW_READ		RTL8367B_IA_CTRL_RW(0)
#define   RTL8367B_IA_CTRL_RW_WRITE		RTL8367B_IA_CTRL_RW(1)
#define   RTL8367B_IA_CTRL_CMD_MASK		BIT(0)

#define RTL8367B_IA_STATUS_REG			0x1f01
#define   RTL8367B_IA_STATUS_PHY_BUSY		BIT(2)
#define   RTL8367B_IA_STATUS_SDS_BUSY		BIT(1)
#define   RTL8367B_IA_STATUS_MDX_BUSY		BIT(0)

#define RTL8367B_IA_ADDRESS_REG			0x1f02
#define RTL8367B_IA_WRITE_DATA_REG		0x1f03
#define RTL8367B_IA_READ_DATA_REG		0x1f04

#define RTL8367B_INTERNAL_PHY_REG(_a, _r)	(0x2000 + 32 * (_a) + (_r))

#define RTL8367B_NUM_MIB_COUNTERS	58

#define RTL8367B_CPU_PORT_NUM		5
#define RTL8367B_NUM_PORTS		8
#define RTL8367B_NUM_VLANS		32
#define RTL8367B_NUM_VIDS		4096
#define RTL8367B_PRIORITYMAX		7
#define RTL8367B_FIDMAX			7

#define RTL8367B_PORT_0			BIT(0)
#define RTL8367B_PORT_1			BIT(1)
#define RTL8367B_PORT_2			BIT(2)
#define RTL8367B_PORT_3			BIT(3)
#define RTL8367B_PORT_4			BIT(4)
#define RTL8367B_PORT_E0		BIT(5)	/* External port 0 */
#define RTL8367B_PORT_E1		BIT(6)	/* External port 1 */
#define RTL8367B_PORT_E2		BIT(7)	/* External port 2 */

#define RTL8367B_PORTS_ALL					\
	(RTL8367B_PORT_0 | RTL8367B_PORT_1 | RTL8367B_PORT_2 |	\
	 RTL8367B_PORT_3 | RTL8367B_PORT_4 | RTL8367B_PORT_E0 | \
	 RTL8367B_PORT_E1 | RTL8367B_PORT_E2)

#define RTL8367B_PORTS_ALL_BUT_CPU				\
	(RTL8367B_PORT_0 | RTL8367B_PORT_1 | RTL8367B_PORT_2 |	\
	 RTL8367B_PORT_3 | RTL8367B_PORT_4 | RTL8367B_PORT_E1 |	\
	 RTL8367B_PORT_E2)

struct rtl8367b_initval {
	u16 reg;
	u16 val;
};

#define RTL8367B_MIB_RXB_ID		0	/* IfInOctets */
#define RTL8367B_MIB_TXB_ID		28	/* IfOutOctets */

static struct rtl8366_mib_counter
rtl8367b_mib_counters[RTL8367B_NUM_MIB_COUNTERS] = {
	{0,   0, 4, "ifInOctets"			},
	{0,   4, 2, "dot3StatsFCSErrors"		},
	{0,   6, 2, "dot3StatsSymbolErrors"		},
	{0,   8, 2, "dot3InPauseFrames"			},
	{0,  10, 2, "dot3ControlInUnknownOpcodes"	},
	{0,  12, 2, "etherStatsFragments"		},
	{0,  14, 2, "etherStatsJabbers"			},
	{0,  16, 2, "ifInUcastPkts"			},
	{0,  18, 2, "etherStatsDropEvents"		},
	{0,  20, 2, "ifInMulticastPkts"			},
	{0,  22, 2, "ifInBroadcastPkts"			},
	{0,  24, 2, "inMldChecksumError"		},
	{0,  26, 2, "inIgmpChecksumError"		},
	{0,  28, 2, "inMldSpecificQuery"		},
	{0,  30, 2, "inMldGeneralQuery"			},
	{0,  32, 2, "inIgmpSpecificQuery"		},
	{0,  34, 2, "inIgmpGeneralQuery"		},
	{0,  36, 2, "inMldLeaves"			},
	{0,  38, 2, "inIgmpLeaves"			},

	{0,  40, 4, "etherStatsOctets"			},
	{0,  44, 2, "etherStatsUnderSizePkts"		},
	{0,  46, 2, "etherOversizeStats"		},
	{0,  48, 2, "etherStatsPkts64Octets"		},
	{0,  50, 2, "etherStatsPkts65to127Octets"	},
	{0,  52, 2, "etherStatsPkts128to255Octets"	},
	{0,  54, 2, "etherStatsPkts256to511Octets"	},
	{0,  56, 2, "etherStatsPkts512to1023Octets"	},
	{0,  58, 2, "etherStatsPkts1024to1518Octets"	},

	{0,  60, 4, "ifOutOctets"			},
	{0,  64, 2, "dot3StatsSingleCollisionFrames"	},
	{0,  66, 2, "dot3StatMultipleCollisionFrames"	},
	{0,  68, 2, "dot3sDeferredTransmissions"	},
	{0,  70, 2, "dot3StatsLateCollisions"		},
	{0,  72, 2, "etherStatsCollisions"		},
	{0,  74, 2, "dot3StatsExcessiveCollisions"	},
	{0,  76, 2, "dot3OutPauseFrames"		},
	{0,  78, 2, "ifOutDiscards"			},
	{0,  80, 2, "dot1dTpPortInDiscards"		},
	{0,  82, 2, "ifOutUcastPkts"			},
	{0,  84, 2, "ifOutMulticastPkts"		},
	{0,  86, 2, "ifOutBroadcastPkts"		},
	{0,  88, 2, "outOampduPkts"			},
	{0,  90, 2, "inOampduPkts"			},
	{0,  92, 2, "inIgmpJoinsSuccess"		},
	{0,  94, 2, "inIgmpJoinsFail"			},
	{0,  96, 2, "inMldJoinsSuccess"			},
	{0,  98, 2, "inMldJoinsFail"			},
	{0, 100, 2, "inReportSuppressionDrop"		},
	{0, 102, 2, "inLeaveSuppressionDrop"		},
	{0, 104, 2, "outIgmpReports"			},
	{0, 106, 2, "outIgmpLeaves"			},
	{0, 108, 2, "outIgmpGeneralQuery"		},
	{0, 110, 2, "outIgmpSpecificQuery"		},
	{0, 112, 2, "outMldReports"			},
	{0, 114, 2, "outMldLeaves"			},
	{0, 116, 2, "outMldGeneralQuery"		},
	{0, 118, 2, "outMldSpecificQuery"		},
	{0, 120, 2, "inKnownMulticastPkts"		},
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

static const struct rtl8367b_initval rtl8367r_vb_initvals_0[] = {
	{0x1B03, 0x0876}, {0x1200, 0x7FC4}, {0x0301, 0x0026}, {0x1722, 0x0E14},
	{0x205F, 0x0002}, {0x2059, 0x1A00}, {0x205F, 0x0000}, {0x207F, 0x0002},
	{0x2077, 0x0000}, {0x2078, 0x0000}, {0x2079, 0x0000}, {0x207A, 0x0000},
	{0x207B, 0x0000}, {0x207F, 0x0000}, {0x205F, 0x0002}, {0x2053, 0x0000},
	{0x2054, 0x0000}, {0x2055, 0x0000}, {0x2056, 0x0000}, {0x2057, 0x0000},
	{0x205F, 0x0000}, {0x12A4, 0x110A}, {0x12A6, 0x150A}, {0x13F1, 0x0013},
	{0x13F4, 0x0010}, {0x13F5, 0x0000}, {0x0018, 0x0F00}, {0x0038, 0x0F00},
	{0x0058, 0x0F00}, {0x0078, 0x0F00}, {0x0098, 0x0F00}, {0x12B6, 0x0C02},
	{0x12B7, 0x030F}, {0x12B8, 0x11FF}, {0x12BC, 0x0004}, {0x1362, 0x0115},
	{0x1363, 0x0002}, {0x1363, 0x0000}, {0x133F, 0x0030}, {0x133E, 0x000E},
	{0x221F, 0x0007}, {0x221E, 0x002D}, {0x2218, 0xF030}, {0x221F, 0x0007},
	{0x221E, 0x0023}, {0x2216, 0x0005}, {0x2215, 0x00B9}, {0x2219, 0x0044},
	{0x2215, 0x00BA}, {0x2219, 0x0020}, {0x2215, 0x00BB}, {0x2219, 0x00C1},
	{0x2215, 0x0148}, {0x2219, 0x0096}, {0x2215, 0x016E}, {0x2219, 0x0026},
	{0x2216, 0x0000}, {0x2216, 0x0000}, {0x221E, 0x002D}, {0x2218, 0xF010},
	{0x221F, 0x0007}, {0x221E, 0x0020}, {0x2215, 0x0D00}, {0x221F, 0x0000},
	{0x221F, 0x0000}, {0x2217, 0x2160}, {0x221F, 0x0001}, {0x2210, 0xF25E},
	{0x221F, 0x0007}, {0x221E, 0x0042}, {0x2215, 0x0F00}, {0x2215, 0x0F00},
	{0x2216, 0x7408}, {0x2215, 0x0E00}, {0x2215, 0x0F00}, {0x2215, 0x0F01},
	{0x2216, 0x4000}, {0x2215, 0x0E01}, {0x2215, 0x0F01}, {0x2215, 0x0F02},
	{0x2216, 0x9400}, {0x2215, 0x0E02}, {0x2215, 0x0F02}, {0x2215, 0x0F03},
	{0x2216, 0x7408}, {0x2215, 0x0E03}, {0x2215, 0x0F03}, {0x2215, 0x0F04},
	{0x2216, 0x4008}, {0x2215, 0x0E04}, {0x2215, 0x0F04}, {0x2215, 0x0F05},
	{0x2216, 0x9400}, {0x2215, 0x0E05}, {0x2215, 0x0F05}, {0x2215, 0x0F06},
	{0x2216, 0x0803}, {0x2215, 0x0E06}, {0x2215, 0x0F06}, {0x2215, 0x0D00},
	{0x2215, 0x0100}, {0x221F, 0x0001}, {0x2210, 0xF05E}, {0x221F, 0x0000},
	{0x2217, 0x2100}, {0x221F, 0x0000}, {0x220D, 0x0003}, {0x220E, 0x0015},
	{0x220D, 0x4003}, {0x220E, 0x0006}, {0x221F, 0x0000}, {0x2200, 0x1340},
	{0x133F, 0x0010}, {0x12A0, 0x0058}, {0x12A1, 0x0058}, {0x133E, 0x000E},
	{0x133F, 0x0030}, {0x221F, 0x0000}, {0x2210, 0x0166}, {0x221F, 0x0000},
	{0x133E, 0x000E}, {0x133F, 0x0010}, {0x133F, 0x0030}, {0x133E, 0x000E},
	{0x221F, 0x0005}, {0x2205, 0xFFF6}, {0x2206, 0x0080}, {0x2205, 0x8B6E},
	{0x2206, 0x0000}, {0x220F, 0x0100}, {0x2205, 0x8000}, {0x2206, 0x0280},
	{0x2206, 0x28F7}, {0x2206, 0x00E0}, {0x2206, 0xFFF7}, {0x2206, 0xA080},
	{0x2206, 0x02AE}, {0x2206, 0xF602}, {0x2206, 0x0153}, {0x2206, 0x0201},
	{0x2206, 0x6602}, {0x2206, 0x80B9}, {0x2206, 0xE08B}, {0x2206, 0x8CE1},
	{0x2206, 0x8B8D}, {0x2206, 0x1E01}, {0x2206, 0xE18B}, {0x2206, 0x8E1E},
	{0x2206, 0x01A0}, {0x2206, 0x00E7}, {0x2206, 0xAEDB}, {0x2206, 0xEEE0},
	{0x2206, 0x120E}, {0x2206, 0xEEE0}, {0x2206, 0x1300}, {0x2206, 0xEEE0},
	{0x2206, 0x2001}, {0x2206, 0xEEE0}, {0x2206, 0x2166}, {0x2206, 0xEEE0},
	{0x2206, 0xC463}, {0x2206, 0xEEE0}, {0x2206, 0xC5E8}, {0x2206, 0xEEE0},
	{0x2206, 0xC699}, {0x2206, 0xEEE0}, {0x2206, 0xC7C2}, {0x2206, 0xEEE0},
	{0x2206, 0xC801}, {0x2206, 0xEEE0}, {0x2206, 0xC913}, {0x2206, 0xEEE0},
	{0x2206, 0xCA30}, {0x2206, 0xEEE0}, {0x2206, 0xCB3E}, {0x2206, 0xEEE0},
	{0x2206, 0xDCE1}, {0x2206, 0xEEE0}, {0x2206, 0xDD00}, {0x2206, 0xEEE2},
	{0x2206, 0x0001}, {0x2206, 0xEEE2}, {0x2206, 0x0100}, {0x2206, 0xEEE4},
	{0x2206, 0x8860}, {0x2206, 0xEEE4}, {0x2206, 0x8902}, {0x2206, 0xEEE4},
	{0x2206, 0x8C00}, {0x2206, 0xEEE4}, {0x2206, 0x8D30}, {0x2206, 0xEEEA},
	{0x2206, 0x1480}, {0x2206, 0xEEEA}, {0x2206, 0x1503}, {0x2206, 0xEEEA},
	{0x2206, 0xC600}, {0x2206, 0xEEEA}, {0x2206, 0xC706}, {0x2206, 0xEE85},
	{0x2206, 0xEE00}, {0x2206, 0xEE85}, {0x2206, 0xEF00}, {0x2206, 0xEE8B},
	{0x2206, 0x6750}, {0x2206, 0xEE8B}, {0x2206, 0x6632}, {0x2206, 0xEE8A},
	{0x2206, 0xD448}, {0x2206, 0xEE8A}, {0x2206, 0xD548}, {0x2206, 0xEE8A},
	{0x2206, 0xD649}, {0x2206, 0xEE8A}, {0x2206, 0xD7F8}, {0x2206, 0xEE8B},
	{0x2206, 0x85E2}, {0x2206, 0xEE8B}, {0x2206, 0x8700}, {0x2206, 0xEEFF},
	{0x2206, 0xF600}, {0x2206, 0xEEFF}, {0x2206, 0xF7FC}, {0x2206, 0x04F8},
	{0x2206, 0xE08B}, {0x2206, 0x8EAD}, {0x2206, 0x2023}, {0x2206, 0xF620},
	{0x2206, 0xE48B}, {0x2206, 0x8E02}, {0x2206, 0x2877}, {0x2206, 0x0225},
	{0x2206, 0xC702}, {0x2206, 0x26A1}, {0x2206, 0x0281}, {0x2206, 0xB302},
	{0x2206, 0x8496}, {0x2206, 0x0202}, {0x2206, 0xA102}, {0x2206, 0x27F1},
	{0x2206, 0x0228}, {0x2206, 0xF902}, {0x2206, 0x2AA0}, {0x2206, 0x0282},
	{0x2206, 0xB8E0}, {0x2206, 0x8B8E}, {0x2206, 0xAD21}, {0x2206, 0x08F6},
	{0x2206, 0x21E4}, {0x2206, 0x8B8E}, {0x2206, 0x0202}, {0x2206, 0x80E0},
	{0x2206, 0x8B8E}, {0x2206, 0xAD22}, {0x2206, 0x05F6}, {0x2206, 0x22E4},
	{0x2206, 0x8B8E}, {0x2206, 0xE08B}, {0x2206, 0x8EAD}, {0x2206, 0x2305},
	{0x2206, 0xF623}, {0x2206, 0xE48B}, {0x2206, 0x8EE0}, {0x2206, 0x8B8E},
	{0x2206, 0xAD24}, {0x2206, 0x08F6}, {0x2206, 0x24E4}, {0x2206, 0x8B8E},
	{0x2206, 0x0227}, {0x2206, 0x6AE0}, {0x2206, 0x8B8E}, {0x2206, 0xAD25},
	{0x2206, 0x05F6}, {0x2206, 0x25E4}, {0x2206, 0x8B8E}, {0x2206, 0xE08B},
	{0x2206, 0x8EAD}, {0x2206, 0x260B}, {0x2206, 0xF626}, {0x2206, 0xE48B},
	{0x2206, 0x8E02}, {0x2206, 0x830D}, {0x2206, 0x021D}, {0x2206, 0x6BE0},
	{0x2206, 0x8B8E}, {0x2206, 0xAD27}, {0x2206, 0x05F6}, {0x2206, 0x27E4},
	{0x2206, 0x8B8E}, {0x2206, 0x0281}, {0x2206, 0x4402}, {0x2206, 0x045C},
	{0x2206, 0xFC04}, {0x2206, 0xF8E0}, {0x2206, 0x8B83}, {0x2206, 0xAD23},
	{0x2206, 0x30E0}, {0x2206, 0xE022}, {0x2206, 0xE1E0}, {0x2206, 0x2359},
	{0x2206, 0x02E0}, {0x2206, 0x85EF}, {0x2206, 0xE585}, {0x2206, 0xEFAC},
	{0x2206, 0x2907}, {0x2206, 0x1F01}, {0x2206, 0x9E51}, {0x2206, 0xAD29},
	{0x2206, 0x20E0}, {0x2206, 0x8B83}, {0x2206, 0xAD21}, {0x2206, 0x06E1},
	{0x2206, 0x8B84}, {0x2206, 0xAD28}, {0x2206, 0x42E0}, {0x2206, 0x8B85},
	{0x2206, 0xAD21}, {0x2206, 0x06E1}, {0x2206, 0x8B84}, {0x2206, 0xAD29},
	{0x2206, 0x36BF}, {0x2206, 0x34BF}, {0x2206, 0x022C}, {0x2206, 0x31AE},
	{0x2206, 0x2EE0}, {0x2206, 0x8B83}, {0x2206, 0xAD21}, {0x2206, 0x10E0},
	{0x2206, 0x8B84}, {0x2206, 0xF620}, {0x2206, 0xE48B}, {0x2206, 0x84EE},
	{0x2206, 0x8ADA}, {0x2206, 0x00EE}, {0x2206, 0x8ADB}, {0x2206, 0x00E0},
	{0x2206, 0x8B85}, {0x2206, 0xAD21}, {0x2206, 0x0CE0}, {0x2206, 0x8B84},
	{0x2206, 0xF621}, {0x2206, 0xE48B}, {0x2206, 0x84EE}, {0x2206, 0x8B72},
	{0x2206, 0xFFBF}, {0x2206, 0x34C2}, {0x2206, 0x022C}, {0x2206, 0x31FC},
	{0x2206, 0x04F8}, {0x2206, 0xFAEF}, {0x2206, 0x69E0}, {0x2206, 0x8B85},
	{0x2206, 0xAD21}, {0x2206, 0x42E0}, {0x2206, 0xE022}, {0x2206, 0xE1E0},
	{0x2206, 0x2358}, {0x2206, 0xC059}, {0x2206, 0x021E}, {0x2206, 0x01E1},
	{0x2206, 0x8B72}, {0x2206, 0x1F10}, {0x2206, 0x9E2F}, {0x2206, 0xE48B},
	{0x2206, 0x72AD}, {0x2206, 0x2123}, {0x2206, 0xE18B}, {0x2206, 0x84F7},
	{0x2206, 0x29E5}, {0x2206, 0x8B84}, {0x2206, 0xAC27}, {0x2206, 0x10AC},
	{0x2206, 0x2605}, {0x2206, 0x0205}, {0x2206, 0x23AE}, {0x2206, 0x1602},
	{0x2206, 0x0535}, {0x2206, 0x0282}, {0x2206, 0x30AE}, {0x2206, 0x0E02},
	{0x2206, 0x056A}, {0x2206, 0x0282}, {0x2206, 0x75AE}, {0x2206, 0x0602},
	{0x2206, 0x04DC}, {0x2206, 0x0282}, {0x2206, 0x04EF}, {0x2206, 0x96FE},
	{0x2206, 0xFC04}, {0x2206, 0xF8F9}, {0x2206, 0xE08B}, {0x2206, 0x87AD},
	{0x2206, 0x2321}, {0x2206, 0xE0EA}, {0x2206, 0x14E1}, {0x2206, 0xEA15},
	{0x2206, 0xAD26}, {0x2206, 0x18F6}, {0x2206, 0x27E4}, {0x2206, 0xEA14},
	{0x2206, 0xE5EA}, {0x2206, 0x15F6}, {0x2206, 0x26E4}, {0x2206, 0xEA14},
	{0x2206, 0xE5EA}, {0x2206, 0x15F7}, {0x2206, 0x27E4}, {0x2206, 0xEA14},
	{0x2206, 0xE5EA}, {0x2206, 0x15FD}, {0x2206, 0xFC04}, {0x2206, 0xF8F9},
	{0x2206, 0xE08B}, {0x2206, 0x87AD}, {0x2206, 0x233A}, {0x2206, 0xAD22},
	{0x2206, 0x37E0}, {0x2206, 0xE020}, {0x2206, 0xE1E0}, {0x2206, 0x21AC},
	{0x2206, 0x212E}, {0x2206, 0xE0EA}, {0x2206, 0x14E1}, {0x2206, 0xEA15},
	{0x2206, 0xF627}, {0x2206, 0xE4EA}, {0x2206, 0x14E5}, {0x2206, 0xEA15},
	{0x2206, 0xE2EA}, {0x2206, 0x12E3}, {0x2206, 0xEA13}, {0x2206, 0x5A8F},
	{0x2206, 0x6A20}, {0x2206, 0xE6EA}, {0x2206, 0x12E7}, {0x2206, 0xEA13},
	{0x2206, 0xF726}, {0x2206, 0xE4EA}, {0x2206, 0x14E5}, {0x2206, 0xEA15},
	{0x2206, 0xF727}, {0x2206, 0xE4EA}, {0x2206, 0x14E5}, {0x2206, 0xEA15},
	{0x2206, 0xFDFC}, {0x2206, 0x04F8}, {0x2206, 0xF9E0}, {0x2206, 0x8B87},
	{0x2206, 0xAD23}, {0x2206, 0x38AD}, {0x2206, 0x2135}, {0x2206, 0xE0E0},
	{0x2206, 0x20E1}, {0x2206, 0xE021}, {0x2206, 0xAC21}, {0x2206, 0x2CE0},
	{0x2206, 0xEA14}, {0x2206, 0xE1EA}, {0x2206, 0x15F6}, {0x2206, 0x27E4},
	{0x2206, 0xEA14}, {0x2206, 0xE5EA}, {0x2206, 0x15E2}, {0x2206, 0xEA12},
	{0x2206, 0xE3EA}, {0x2206, 0x135A}, {0x2206, 0x8FE6}, {0x2206, 0xEA12},
	{0x2206, 0xE7EA}, {0x2206, 0x13F7}, {0x2206, 0x26E4}, {0x2206, 0xEA14},
	{0x2206, 0xE5EA}, {0x2206, 0x15F7}, {0x2206, 0x27E4}, {0x2206, 0xEA14},
	{0x2206, 0xE5EA}, {0x2206, 0x15FD}, {0x2206, 0xFC04}, {0x2206, 0xF8FA},
	{0x2206, 0xEF69}, {0x2206, 0xE08B}, {0x2206, 0x86AD}, {0x2206, 0x2146},
	{0x2206, 0xE0E0}, {0x2206, 0x22E1}, {0x2206, 0xE023}, {0x2206, 0x58C0},
	{0x2206, 0x5902}, {0x2206, 0x1E01}, {0x2206, 0xE18B}, {0x2206, 0x651F},
	{0x2206, 0x109E}, {0x2206, 0x33E4}, {0x2206, 0x8B65}, {0x2206, 0xAD21},
	{0x2206, 0x22AD}, {0x2206, 0x272A}, {0x2206, 0xD400}, {0x2206, 0x01BF},
	{0x2206, 0x34F2}, {0x2206, 0x022C}, {0x2206, 0xA2BF}, {0x2206, 0x34F5},
	{0x2206, 0x022C}, {0x2206, 0xE0E0}, {0x2206, 0x8B67}, {0x2206, 0x1B10},
	{0x2206, 0xAA14}, {0x2206, 0xE18B}, {0x2206, 0x660D}, {0x2206, 0x1459},
	{0x2206, 0x0FAE}, {0x2206, 0x05E1}, {0x2206, 0x8B66}, {0x2206, 0x590F},
	{0x2206, 0xBF85}, {0x2206, 0x6102}, {0x2206, 0x2CA2}, {0x2206, 0xEF96},
	{0x2206, 0xFEFC}, {0x2206, 0x04F8}, {0x2206, 0xF9FA}, {0x2206, 0xFBEF},
	{0x2206, 0x79E2}, {0x2206, 0x8AD2}, {0x2206, 0xAC19}, {0x2206, 0x2DE0},
	{0x2206, 0xE036}, {0x2206, 0xE1E0}, {0x2206, 0x37EF}, {0x2206, 0x311F},
	{0x2206, 0x325B}, {0x2206, 0x019E}, {0x2206, 0x1F7A}, {0x2206, 0x0159},
	{0x2206, 0x019F}, {0x2206, 0x0ABF}, {0x2206, 0x348E}, {0x2206, 0x022C},
	{0x2206, 0x31F6}, {0x2206, 0x06AE}, {0x2206, 0x0FF6}, {0x2206, 0x0302},
	{0x2206, 0x0470}, {0x2206, 0xF703}, {0x2206, 0xF706}, {0x2206, 0xBF34},
	{0x2206, 0x9302}, {0x2206, 0x2C31}, {0x2206, 0xAC1A}, {0x2206, 0x25E0},
	{0x2206, 0xE022}, {0x2206, 0xE1E0}, {0x2206, 0x23EF}, {0x2206, 0x300D},
	{0x2206, 0x311F}, {0x2206, 0x325B}, {0x2206, 0x029E}, {0x2206, 0x157A},
	{0x2206, 0x0258}, {0x2206, 0xC4A0}, {0x2206, 0x0408}, {0x2206, 0xBF34},
	{0x2206, 0x9E02}, {0x2206, 0x2C31}, {0x2206, 0xAE06}, {0x2206, 0xBF34},
	{0x2206, 0x9C02}, {0x2206, 0x2C31}, {0x2206, 0xAC1B}, {0x2206, 0x4AE0},
	{0x2206, 0xE012}, {0x2206, 0xE1E0}, {0x2206, 0x13EF}, {0x2206, 0x300D},
	{0x2206, 0x331F}, {0x2206, 0x325B}, {0x2206, 0x1C9E}, {0x2206, 0x3AEF},
	{0x2206, 0x325B}, {0x2206, 0x1C9F}, {0x2206, 0x09BF}, {0x2206, 0x3498},
	{0x2206, 0x022C}, {0x2206, 0x3102}, {0x2206, 0x83C5}, {0x2206, 0x5A03},
	{0x2206, 0x0D03}, {0x2206, 0x581C}, {0x2206, 0x1E20}, {0x2206, 0x0207},
	{0x2206, 0xA0A0}, {0x2206, 0x000E}, {0x2206, 0x0284}, {0x2206, 0x17AD},
	{0x2206, 0x1817}, {0x2206, 0xBF34}, {0x2206, 0x9A02}, {0x2206, 0x2C31},
	{0x2206, 0xAE0F}, {0x2206, 0xBF34}, {0x2206, 0xC802}, {0x2206, 0x2C31},
	{0x2206, 0xBF34}, {0x2206, 0xC502}, {0x2206, 0x2C31}, {0x2206, 0x0284},
	{0x2206, 0x52E6}, {0x2206, 0x8AD2}, {0x2206, 0xEF97}, {0x2206, 0xFFFE},
	{0x2206, 0xFDFC}, {0x2206, 0x04F8}, {0x2206, 0xBF34}, {0x2206, 0xDA02},
	{0x2206, 0x2CE0}, {0x2206, 0xE58A}, {0x2206, 0xD3BF}, {0x2206, 0x34D4},
	{0x2206, 0x022C}, {0x2206, 0xE00C}, {0x2206, 0x1159}, {0x2206, 0x02E0},
	{0x2206, 0x8AD3}, {0x2206, 0x1E01}, {0x2206, 0xE48A}, {0x2206, 0xD3D1},
	{0x2206, 0x00BF}, {0x2206, 0x34DA}, {0x2206, 0x022C}, {0x2206, 0xA2D1},
	{0x2206, 0x01BF}, {0x2206, 0x34D4}, {0x2206, 0x022C}, {0x2206, 0xA2BF},
	{0x2206, 0x34CB}, {0x2206, 0x022C}, {0x2206, 0xE0E5}, {0x2206, 0x8ACE},
	{0x2206, 0xBF85}, {0x2206, 0x6702}, {0x2206, 0x2CE0}, {0x2206, 0xE58A},
	{0x2206, 0xCFBF}, {0x2206, 0x8564}, {0x2206, 0x022C}, {0x2206, 0xE0E5},
	{0x2206, 0x8AD0}, {0x2206, 0xBF85}, {0x2206, 0x6A02}, {0x2206, 0x2CE0},
	{0x2206, 0xE58A}, {0x2206, 0xD1FC}, {0x2206, 0x04F8}, {0x2206, 0xE18A},
	{0x2206, 0xD1BF}, {0x2206, 0x856A}, {0x2206, 0x022C}, {0x2206, 0xA2E1},
	{0x2206, 0x8AD0}, {0x2206, 0xBF85}, {0x2206, 0x6402}, {0x2206, 0x2CA2},
	{0x2206, 0xE18A}, {0x2206, 0xCFBF}, {0x2206, 0x8567}, {0x2206, 0x022C},
	{0x2206, 0xA2E1}, {0x2206, 0x8ACE}, {0x2206, 0xBF34}, {0x2206, 0xCB02},
	{0x2206, 0x2CA2}, {0x2206, 0xE18A}, {0x2206, 0xD3BF}, {0x2206, 0x34DA},
	{0x2206, 0x022C}, {0x2206, 0xA2E1}, {0x2206, 0x8AD3}, {0x2206, 0x0D11},
	{0x2206, 0xBF34}, {0x2206, 0xD402}, {0x2206, 0x2CA2}, {0x2206, 0xFC04},
	{0x2206, 0xF9A0}, {0x2206, 0x0405}, {0x2206, 0xE38A}, {0x2206, 0xD4AE},
	{0x2206, 0x13A0}, {0x2206, 0x0805}, {0x2206, 0xE38A}, {0x2206, 0xD5AE},
	{0x2206, 0x0BA0}, {0x2206, 0x0C05}, {0x2206, 0xE38A}, {0x2206, 0xD6AE},
	{0x2206, 0x03E3}, {0x2206, 0x8AD7}, {0x2206, 0xEF13}, {0x2206, 0xBF34},
	{0x2206, 0xCB02}, {0x2206, 0x2CA2}, {0x2206, 0xEF13}, {0x2206, 0x0D11},
	{0x2206, 0xBF85}, {0x2206, 0x6702}, {0x2206, 0x2CA2}, {0x2206, 0xEF13},
	{0x2206, 0x0D14}, {0x2206, 0xBF85}, {0x2206, 0x6402}, {0x2206, 0x2CA2},
	{0x2206, 0xEF13}, {0x2206, 0x0D17}, {0x2206, 0xBF85}, {0x2206, 0x6A02},
	{0x2206, 0x2CA2}, {0x2206, 0xFD04}, {0x2206, 0xF8E0}, {0x2206, 0x8B85},
	{0x2206, 0xAD27}, {0x2206, 0x2DE0}, {0x2206, 0xE036}, {0x2206, 0xE1E0},
	{0x2206, 0x37E1}, {0x2206, 0x8B73}, {0x2206, 0x1F10}, {0x2206, 0x9E20},
	{0x2206, 0xE48B}, {0x2206, 0x73AC}, {0x2206, 0x200B}, {0x2206, 0xAC21},
	{0x2206, 0x0DAC}, {0x2206, 0x250F}, {0x2206, 0xAC27}, {0x2206, 0x0EAE},
	{0x2206, 0x0F02}, {0x2206, 0x84CC}, {0x2206, 0xAE0A}, {0x2206, 0x0284},
	{0x2206, 0xD1AE}, {0x2206, 0x05AE}, {0x2206, 0x0302}, {0x2206, 0x84D8},
	{0x2206, 0xFC04}, {0x2206, 0xEE8B}, {0x2206, 0x6800}, {0x2206, 0x0402},
	{0x2206, 0x84E5}, {0x2206, 0x0285}, {0x2206, 0x2804}, {0x2206, 0x0285},
	{0x2206, 0x4904}, {0x2206, 0xEE8B}, {0x2206, 0x6800}, {0x2206, 0xEE8B},
	{0x2206, 0x6902}, {0x2206, 0x04F8}, {0x2206, 0xF9E0}, {0x2206, 0x8B85},
	{0x2206, 0xAD26}, {0x2206, 0x38D0}, {0x2206, 0x0B02}, {0x2206, 0x2B4D},
	{0x2206, 0x5882}, {0x2206, 0x7882}, {0x2206, 0x9F2D}, {0x2206, 0xE08B},
	{0x2206, 0x68E1}, {0x2206, 0x8B69}, {0x2206, 0x1F10}, {0x2206, 0x9EC8},
	{0x2206, 0x10E4}, {0x2206, 0x8B68}, {0x2206, 0xE0E0}, {0x2206, 0x00E1},
	{0x2206, 0xE001}, {0x2206, 0xF727}, {0x2206, 0xE4E0}, {0x2206, 0x00E5},
	{0x2206, 0xE001}, {0x2206, 0xE2E0}, {0x2206, 0x20E3}, {0x2206, 0xE021},
	{0x2206, 0xAD30}, {0x2206, 0xF7F6}, {0x2206, 0x27E4}, {0x2206, 0xE000},
	{0x2206, 0xE5E0}, {0x2206, 0x01FD}, {0x2206, 0xFC04}, {0x2206, 0xF8FA},
	{0x2206, 0xEF69}, {0x2206, 0xE08B}, {0x2206, 0x86AD}, {0x2206, 0x2212},
	{0x2206, 0xE0E0}, {0x2206, 0x14E1}, {0x2206, 0xE015}, {0x2206, 0xAD26},
	{0x2206, 0x9CE1}, {0x2206, 0x85E0}, {0x2206, 0xBF85}, {0x2206, 0x6D02},
	{0x2206, 0x2CA2}, {0x2206, 0xEF96}, {0x2206, 0xFEFC}, {0x2206, 0x04F8},
	{0x2206, 0xFAEF}, {0x2206, 0x69E0}, {0x2206, 0x8B86}, {0x2206, 0xAD22},
	{0x2206, 0x09E1}, {0x2206, 0x85E1}, {0x2206, 0xBF85}, {0x2206, 0x6D02},
	{0x2206, 0x2CA2}, {0x2206, 0xEF96}, {0x2206, 0xFEFC}, {0x2206, 0x0464},
	{0x2206, 0xE48C}, {0x2206, 0xFDE4}, {0x2206, 0x80CA}, {0x2206, 0xE480},
	{0x2206, 0x66E0}, {0x2206, 0x8E70}, {0x2206, 0xE076}, {0x2205, 0xE142},
	{0x2206, 0x0701}, {0x2205, 0xE140}, {0x2206, 0x0405}, {0x220F, 0x0000},
	{0x221F, 0x0000}, {0x2200, 0x1340}, {0x133E, 0x000E}, {0x133F, 0x0010},
	{0x13EB, 0x11BB}
};

static const struct rtl8367b_initval rtl8367r_vb_initvals_1[] = {
	{0x1B03, 0x0876}, {0x1200, 0x7FC4}, {0x1305, 0xC000}, {0x121E, 0x03CA},
	{0x1233, 0x0352}, {0x1234, 0x0064}, {0x1237, 0x0096}, {0x1238, 0x0078},
	{0x1239, 0x0084}, {0x123A, 0x0030}, {0x205F, 0x0002}, {0x2059, 0x1A00},
	{0x205F, 0x0000}, {0x207F, 0x0002}, {0x2077, 0x0000}, {0x2078, 0x0000},
	{0x2079, 0x0000}, {0x207A, 0x0000}, {0x207B, 0x0000}, {0x207F, 0x0000},
	{0x205F, 0x0002}, {0x2053, 0x0000}, {0x2054, 0x0000}, {0x2055, 0x0000},
	{0x2056, 0x0000}, {0x2057, 0x0000}, {0x205F, 0x0000}, {0x133F, 0x0030},
	{0x133E, 0x000E}, {0x221F, 0x0005}, {0x2205, 0x8B86}, {0x2206, 0x800E},
	{0x221F, 0x0000}, {0x133F, 0x0010}, {0x12A3, 0x2200}, {0x6107, 0xE58B},
	{0x6103, 0xA970}, {0x0018, 0x0F00}, {0x0038, 0x0F00}, {0x0058, 0x0F00},
	{0x0078, 0x0F00}, {0x0098, 0x0F00}, {0x133F, 0x0030}, {0x133E, 0x000E},
	{0x221F, 0x0005}, {0x2205, 0x8B6E}, {0x2206, 0x0000}, {0x220F, 0x0100},
	{0x2205, 0xFFF6}, {0x2206, 0x0080}, {0x2205, 0x8000}, {0x2206, 0x0280},
	{0x2206, 0x2BF7}, {0x2206, 0x00E0}, {0x2206, 0xFFF7}, {0x2206, 0xA080},
	{0x2206, 0x02AE}, {0x2206, 0xF602}, {0x2206, 0x0153}, {0x2206, 0x0201},
	{0x2206, 0x6602}, {0x2206, 0x8044}, {0x2206, 0x0201}, {0x2206, 0x7CE0},
	{0x2206, 0x8B8C}, {0x2206, 0xE18B}, {0x2206, 0x8D1E}, {0x2206, 0x01E1},
	{0x2206, 0x8B8E}, {0x2206, 0x1E01}, {0x2206, 0xA000}, {0x2206, 0xE4AE},
	{0x2206, 0xD8EE}, {0x2206, 0x85C0}, {0x2206, 0x00EE}, {0x2206, 0x85C1},
	{0x2206, 0x00EE}, {0x2206, 0x8AFC}, {0x2206, 0x07EE}, {0x2206, 0x8AFD},
	{0x2206, 0x73EE}, {0x2206, 0xFFF6}, {0x2206, 0x00EE}, {0x2206, 0xFFF7},
	{0x2206, 0xFC04}, {0x2206, 0xF8E0}, {0x2206, 0x8B8E}, {0x2206, 0xAD20},
	{0x2206, 0x0302}, {0x2206, 0x8050}, {0x2206, 0xFC04}, {0x2206, 0xF8F9},
	{0x2206, 0xE08B}, {0x2206, 0x85AD}, {0x2206, 0x2548}, {0x2206, 0xE08A},
	{0x2206, 0xE4E1}, {0x2206, 0x8AE5}, {0x2206, 0x7C00}, {0x2206, 0x009E},
	{0x2206, 0x35EE}, {0x2206, 0x8AE4}, {0x2206, 0x00EE}, {0x2206, 0x8AE5},
	{0x2206, 0x00E0}, {0x2206, 0x8AFC}, {0x2206, 0xE18A}, {0x2206, 0xFDE2},
	{0x2206, 0x85C0}, {0x2206, 0xE385}, {0x2206, 0xC102}, {0x2206, 0x2DAC},
	{0x2206, 0xAD20}, {0x2206, 0x12EE}, {0x2206, 0x8AE4}, {0x2206, 0x03EE},
	{0x2206, 0x8AE5}, {0x2206, 0xB7EE}, {0x2206, 0x85C0}, {0x2206, 0x00EE},
	{0x2206, 0x85C1}, {0x2206, 0x00AE}, {0x2206, 0x1115}, {0x2206, 0xE685},
	{0x2206, 0xC0E7}, {0x2206, 0x85C1}, {0x2206, 0xAE08}, {0x2206, 0xEE85},
	{0x2206, 0xC000}, {0x2206, 0xEE85}, {0x2206, 0xC100}, {0x2206, 0xFDFC},
	{0x2206, 0x0400}, {0x2205, 0xE142}, {0x2206, 0x0701}, {0x2205, 0xE140},
	{0x2206, 0x0405}, {0x220F, 0x0000}, {0x221F, 0x0000}, {0x133E, 0x000E},
	{0x133F, 0x0010}, {0x13EB, 0x11BB}, {0x207F, 0x0002}, {0x2073, 0x1D22},
	{0x207F, 0x0000}, {0x133F, 0x0030}, {0x133E, 0x000E}, {0x2200, 0x1340},
	{0x133E, 0x000E}, {0x133F, 0x0010},
};

static int rtl8367b_write_initvals(struct rtl8366_smi *smi,
				  const struct rtl8367b_initval *initvals,
				  int count)
{
	int err;
	int i;

	for (i = 0; i < count; i++)
		REG_WR(smi, initvals[i].reg, initvals[i].val);

	return 0;
}

static int rtl8367b_read_phy_reg(struct rtl8366_smi *smi,
				u32 phy_addr, u32 phy_reg, u32 *val)
{
	int timeout;
	u32 data;
	int err;

	if (phy_addr > RTL8367B_PHY_ADDR_MAX)
		return -EINVAL;

	if (phy_reg > RTL8367B_PHY_REG_MAX)
		return -EINVAL;

	REG_RD(smi, RTL8367B_IA_STATUS_REG, &data);
	if (data & RTL8367B_IA_STATUS_PHY_BUSY)
		return -ETIMEDOUT;

	/* prepare address */
	REG_WR(smi, RTL8367B_IA_ADDRESS_REG,
	       RTL8367B_INTERNAL_PHY_REG(phy_addr, phy_reg));

	/* send read command */
	REG_WR(smi, RTL8367B_IA_CTRL_REG,
	       RTL8367B_IA_CTRL_CMD_MASK | RTL8367B_IA_CTRL_RW_READ);

	timeout = 5;
	do {
		REG_RD(smi, RTL8367B_IA_STATUS_REG, &data);
		if ((data & RTL8367B_IA_STATUS_PHY_BUSY) == 0)
			break;

		if (timeout--) {
			dev_err(smi->parent, "phy read timed out\n");
			return -ETIMEDOUT;
		}

		udelay(1);
	} while (1);

	/* read data */
	REG_RD(smi, RTL8367B_IA_READ_DATA_REG, val);

	dev_dbg(smi->parent, "phy_read: addr:%02x, reg:%02x, val:%04x\n",
		phy_addr, phy_reg, *val);
	return 0;
}

static int rtl8367b_write_phy_reg(struct rtl8366_smi *smi,
				 u32 phy_addr, u32 phy_reg, u32 val)
{
	int timeout;
	u32 data;
	int err;

	dev_dbg(smi->parent, "phy_write: addr:%02x, reg:%02x, val:%04x\n",
		phy_addr, phy_reg, val);

	if (phy_addr > RTL8367B_PHY_ADDR_MAX)
		return -EINVAL;

	if (phy_reg > RTL8367B_PHY_REG_MAX)
		return -EINVAL;

	REG_RD(smi, RTL8367B_IA_STATUS_REG, &data);
	if (data & RTL8367B_IA_STATUS_PHY_BUSY)
		return -ETIMEDOUT;

	/* preapre data */
	REG_WR(smi, RTL8367B_IA_WRITE_DATA_REG, val);

	/* prepare address */
	REG_WR(smi, RTL8367B_IA_ADDRESS_REG,
	       RTL8367B_INTERNAL_PHY_REG(phy_addr, phy_reg));

	/* send write command */
	REG_WR(smi, RTL8367B_IA_CTRL_REG,
	       RTL8367B_IA_CTRL_CMD_MASK | RTL8367B_IA_CTRL_RW_WRITE);

	timeout = 5;
	do {
		REG_RD(smi, RTL8367B_IA_STATUS_REG, &data);
		if ((data & RTL8367B_IA_STATUS_PHY_BUSY) == 0)
			break;

		if (timeout--) {
			dev_err(smi->parent, "phy write timed out\n");
			return -ETIMEDOUT;
		}

		udelay(1);
	} while (1);

	return 0;
}

static int rtl8367b_init_regs(struct rtl8366_smi *smi)
{
	const struct rtl8367b_initval *initvals;
	u32 chip_ver;
	u32 rlvid;
	int count;
	int err;

	REG_WR(smi, RTL8367B_RTL_MAGIC_ID_REG, RTL8367B_RTL_MAGIC_ID_VAL);
	REG_RD(smi, RTL8367B_CHIP_VER_REG, &chip_ver);

	rlvid = (chip_ver >> RTL8367B_CHIP_VER_RLVID_SHIFT) &
		RTL8367B_CHIP_VER_RLVID_MASK;

	switch (rlvid) {
	case 0:
		initvals = rtl8367r_vb_initvals_0;
		count = ARRAY_SIZE(rtl8367r_vb_initvals_0);
		break;

	case 1:
		initvals = rtl8367r_vb_initvals_1;
		count = ARRAY_SIZE(rtl8367r_vb_initvals_1);
		break;

	default:
		dev_err(smi->parent, "unknow rlvid %u\n", rlvid);
		return -ENODEV;
	}

	/* TODO: disable RLTP */

	return rtl8367b_write_initvals(smi, initvals, count);
}

static int rtl8367b_reset_chip(struct rtl8366_smi *smi)
{
	int timeout = 10;
	int err;
	u32 data;

	REG_WR(smi, RTL8367B_CHIP_RESET_REG, RTL8367B_CHIP_RESET_HW);
	msleep(RTL8367B_RESET_DELAY);

	do {
		REG_RD(smi, RTL8367B_CHIP_RESET_REG, &data);
		if (!(data & RTL8367B_CHIP_RESET_HW))
			break;

		msleep(1);
	} while (--timeout);

	if (!timeout) {
		dev_err(smi->parent, "chip reset timed out\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int rtl8367b_extif_set_mode(struct rtl8366_smi *smi, int id,
				   enum rtl8367_extif_mode mode)
{
	int err;

	/* set port mode */
	switch (mode) {
	case RTL8367_EXTIF_MODE_RGMII:
	case RTL8367_EXTIF_MODE_RGMII_33V:
		REG_WR(smi, RTL8367B_CHIP_DEBUG0_REG, 0x0367);
		REG_WR(smi, RTL8367B_CHIP_DEBUG1_REG, 0x7777);
		break;

	case RTL8367_EXTIF_MODE_TMII_MAC:
	case RTL8367_EXTIF_MODE_TMII_PHY:
		REG_RMW(smi, RTL8367B_BYPASS_LINE_RATE_REG,
			BIT((id + 1) % 2), BIT((id + 1) % 2));
		break;

	case RTL8367_EXTIF_MODE_GMII:
		REG_RMW(smi, RTL8367B_CHIP_DEBUG0_REG,
		        RTL8367B_CHIP_DEBUG0_DUMMY0(id),
			RTL8367B_CHIP_DEBUG0_DUMMY0(id));
		REG_RMW(smi, RTL8367B_EXT_RGMXF_REG(id), BIT(6), BIT(6));
		break;

	case RTL8367_EXTIF_MODE_MII_MAC:
	case RTL8367_EXTIF_MODE_MII_PHY:
	case RTL8367_EXTIF_MODE_DISABLED:
		REG_RMW(smi, RTL8367B_BYPASS_LINE_RATE_REG,
			BIT((id + 1) % 2), 0);
		REG_RMW(smi, RTL8367B_EXT_RGMXF_REG(id), BIT(6), 0);
		break;

	default:
		dev_err(smi->parent,
			"invalid mode for external interface %d\n", id);
		return -EINVAL;
	}

	REG_RMW(smi, RTL8367B_DIS_REG,
		RTL8367B_DIS_RGMII_MASK << RTL8367B_DIS_RGMII_SHIFT(id),
		mode << RTL8367B_DIS_RGMII_SHIFT(id));

	return 0;
}

static int rtl8367b_extif_set_force(struct rtl8366_smi *smi, int id,
				    struct rtl8367_port_ability *pa)
{
	u32 mask;
	u32 val;
	int err;

	mask = (RTL8367B_DI_FORCE_MODE |
		RTL8367B_DI_FORCE_NWAY |
		RTL8367B_DI_FORCE_TXPAUSE |
		RTL8367B_DI_FORCE_RXPAUSE |
		RTL8367B_DI_FORCE_LINK |
		RTL8367B_DI_FORCE_DUPLEX |
		RTL8367B_DI_FORCE_SPEED_MASK);

	val = pa->speed;
	val |= pa->force_mode ? RTL8367B_DI_FORCE_MODE : 0;
	val |= pa->nway ? RTL8367B_DI_FORCE_NWAY : 0;
	val |= pa->txpause ? RTL8367B_DI_FORCE_TXPAUSE : 0;
	val |= pa->rxpause ? RTL8367B_DI_FORCE_RXPAUSE : 0;
	val |= pa->link ? RTL8367B_DI_FORCE_LINK : 0;
	val |= pa->duplex ? RTL8367B_DI_FORCE_DUPLEX : 0;

	REG_RMW(smi, RTL8367B_DI_FORCE_REG(id), mask, val);

	return 0;
}

static int rtl8367b_extif_set_rgmii_delay(struct rtl8366_smi *smi, int id,
					 unsigned txdelay, unsigned rxdelay)
{
	u32 mask;
	u32 val;
	int err;

	mask = (RTL8367B_EXT_RGMXF_RXDELAY_MASK |
		(RTL8367B_EXT_RGMXF_TXDELAY_MASK <<
			RTL8367B_EXT_RGMXF_TXDELAY_SHIFT));

	val = rxdelay;
	val |= txdelay << RTL8367B_EXT_RGMXF_TXDELAY_SHIFT;

	REG_RMW(smi, RTL8367B_EXT_RGMXF_REG(id), mask, val);

	return 0;
}

static int rtl8367b_extif_init(struct rtl8366_smi *smi, int id,
			       struct rtl8367_extif_config *cfg)
{
	enum rtl8367_extif_mode mode;
	int err;

	mode = (cfg) ? cfg->mode : RTL8367_EXTIF_MODE_DISABLED;

	err = rtl8367b_extif_set_mode(smi, id, mode);
	if (err)
		return err;

	if (mode != RTL8367_EXTIF_MODE_DISABLED) {
		err = rtl8367b_extif_set_force(smi, id, &cfg->ability);
		if (err)
			return err;

		err = rtl8367b_extif_set_rgmii_delay(smi, id, cfg->txdelay,
						     cfg->rxdelay);
		if (err)
			return err;
	}

	return 0;
}

#ifdef CONFIG_OF
static int rtl8367b_extif_init_of(struct rtl8366_smi *smi, int id,
				  const char *name)
{
	struct rtl8367_extif_config *cfg;
	const __be32 *prop;
	int size;
	int err;

	prop = of_get_property(smi->parent->of_node, name, &size);
	if (!prop)
		return rtl8367b_extif_init(smi, id, NULL);

	if (size != (9 * sizeof(*prop))) {
		dev_err(smi->parent, "%s property is invalid\n", name);
		return -EINVAL;
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

	err = rtl8367b_extif_init(smi, id, cfg);
	kfree(cfg);

	return err;
}
#else
static int rtl8367b_extif_init_of(struct rtl8366_smi *smi, int id,
				  const char *name)
{
	return -EINVAL;
}
#endif

static int rtl8367b_setup(struct rtl8366_smi *smi)
{
	struct rtl8367_platform_data *pdata;
	int err;
	int i;

	pdata = smi->parent->platform_data;

	err = rtl8367b_init_regs(smi);
	if (err)
		return err;

	/* initialize external interfaces */
	if (smi->parent->of_node) {
		err = rtl8367b_extif_init_of(smi, 0, "realtek,extif0");
		if (err)
			return err;

		err = rtl8367b_extif_init_of(smi, 1, "realtek,extif1");
		if (err)
			return err;
	} else {
		err = rtl8367b_extif_init(smi, 0, pdata->extif0_cfg);
		if (err)
			return err;

		err = rtl8367b_extif_init(smi, 1, pdata->extif1_cfg);
		if (err)
			return err;
	}

	/* set maximum packet length to 1536 bytes */
	REG_RMW(smi, RTL8367B_SWC0_REG, RTL8367B_SWC0_MAX_LENGTH_MASK,
		RTL8367B_SWC0_MAX_LENGTH_1536);

	/*
	 * discard VLAN tagged packets if the port is not a member of
	 * the VLAN with which the packets is associated.
	 */
	REG_WR(smi, RTL8367B_VLAN_INGRESS_REG, RTL8367B_PORTS_ALL);

	/*
	 * Setup egress tag mode for each port.
	 */
	for (i = 0; i < RTL8367B_NUM_PORTS; i++)
		REG_RMW(smi,
			RTL8367B_PORT_MISC_CFG_REG(i),
			RTL8367B_PORT_MISC_CFG_EGRESS_MODE_MASK <<
				RTL8367B_PORT_MISC_CFG_EGRESS_MODE_SHIFT,
			RTL8367B_PORT_MISC_CFG_EGRESS_MODE_ORIGINAL <<
				RTL8367B_PORT_MISC_CFG_EGRESS_MODE_SHIFT);

	return 0;
}

static int rtl8367b_get_mib_counter(struct rtl8366_smi *smi, int counter,
				    int port, unsigned long long *val)
{
	struct rtl8366_mib_counter *mib;
	int offset;
	int i;
	int err;
	u32 addr, data;
	u64 mibvalue;

	if (port > RTL8367B_NUM_PORTS ||
	    counter >= RTL8367B_NUM_MIB_COUNTERS)
		return -EINVAL;

	mib = &rtl8367b_mib_counters[counter];
	addr = RTL8367B_MIB_COUNTER_PORT_OFFSET * port + mib->offset;

	/*
	 * Writing access counter address first
	 * then ASIC will prepare 64bits counter wait for being retrived
	 */
	REG_WR(smi, RTL8367B_MIB_ADDRESS_REG, addr >> 2);

	/* read MIB control register */
	REG_RD(smi, RTL8367B_MIB_CTRL0_REG(0), &data);

	if (data & RTL8367B_MIB_CTRL0_BUSY_MASK)
		return -EBUSY;

	if (data & RTL8367B_MIB_CTRL0_RESET_MASK)
		return -EIO;

	if (mib->length == 4)
		offset = 3;
	else
		offset = (mib->offset + 1) % 4;

	mibvalue = 0;
	for (i = 0; i < mib->length; i++) {
		REG_RD(smi, RTL8367B_MIB_COUNTER_REG(offset - i), &data);
		mibvalue = (mibvalue << 16) | (data & 0xFFFF);
	}

	*val = mibvalue;
	return 0;
}

static int rtl8367b_get_vlan_4k(struct rtl8366_smi *smi, u32 vid,
				struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[RTL8367B_TA_VLAN_NUM_WORDS];
	int err;
	int i;

	memset(vlan4k, '\0', sizeof(struct rtl8366_vlan_4k));

	if (vid >= RTL8367B_NUM_VIDS)
		return -EINVAL;

	/* write VID */
	REG_WR(smi, RTL8367B_TA_ADDR_REG, vid);

	/* write table access control word */
	REG_WR(smi, RTL8367B_TA_CTRL_REG, RTL8367B_TA_CTRL_CVLAN_READ);

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_RD(smi, RTL8367B_TA_RDDATA_REG(i), &data[i]);

	vlan4k->vid = vid;
	vlan4k->member = (data[0] >> RTL8367B_TA_VLAN0_MEMBER_SHIFT) &
			 RTL8367B_TA_VLAN0_MEMBER_MASK;
	vlan4k->untag = (data[0] >> RTL8367B_TA_VLAN0_UNTAG_SHIFT) &
			RTL8367B_TA_VLAN0_UNTAG_MASK;
	vlan4k->fid = (data[1] >> RTL8367B_TA_VLAN1_FID_SHIFT) &
		      RTL8367B_TA_VLAN1_FID_MASK;

	return 0;
}

static int rtl8367b_set_vlan_4k(struct rtl8366_smi *smi,
				const struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[RTL8367B_TA_VLAN_NUM_WORDS];
	int err;
	int i;

	if (vlan4k->vid >= RTL8367B_NUM_VIDS ||
	    vlan4k->member > RTL8367B_TA_VLAN0_MEMBER_MASK ||
	    vlan4k->untag > RTL8367B_UNTAG_MASK ||
	    vlan4k->fid > RTL8367B_FIDMAX)
		return -EINVAL;

	memset(data, 0, sizeof(data));

	data[0] = (vlan4k->member & RTL8367B_TA_VLAN0_MEMBER_MASK) <<
		  RTL8367B_TA_VLAN0_MEMBER_SHIFT;
	data[0] |= (vlan4k->untag & RTL8367B_TA_VLAN0_UNTAG_MASK) <<
		   RTL8367B_TA_VLAN0_UNTAG_SHIFT;
	data[1] = (vlan4k->fid & RTL8367B_TA_VLAN1_FID_MASK) <<
		  RTL8367B_TA_VLAN1_FID_SHIFT;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_WR(smi, RTL8367B_TA_WRDATA_REG(i), data[i]);

	/* write VID */
	REG_WR(smi, RTL8367B_TA_ADDR_REG,
	       vlan4k->vid & RTL8367B_TA_VLAN_VID_MASK);

	/* write table access control word */
	REG_WR(smi, RTL8367B_TA_CTRL_REG, RTL8367B_TA_CTRL_CVLAN_WRITE);

	return 0;
}

static int rtl8367b_get_vlan_mc(struct rtl8366_smi *smi, u32 index,
				struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[RTL8367B_VLAN_MC_NUM_WORDS];
	int err;
	int i;

	memset(vlanmc, '\0', sizeof(struct rtl8366_vlan_mc));

	if (index >= RTL8367B_NUM_VLANS)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_RD(smi, RTL8367B_VLAN_MC_BASE(index) + i, &data[i]);

	vlanmc->member = (data[0] >> RTL8367B_VLAN_MC0_MEMBER_SHIFT) &
			 RTL8367B_VLAN_MC0_MEMBER_MASK;
	vlanmc->fid = (data[1] >> RTL8367B_VLAN_MC1_FID_SHIFT) &
		      RTL8367B_VLAN_MC1_FID_MASK;
	vlanmc->vid = (data[3] >> RTL8367B_VLAN_MC3_EVID_SHIFT) &
		      RTL8367B_VLAN_MC3_EVID_MASK;

	return 0;
}

static int rtl8367b_set_vlan_mc(struct rtl8366_smi *smi, u32 index,
				const struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[RTL8367B_VLAN_MC_NUM_WORDS];
	int err;
	int i;

	if (index >= RTL8367B_NUM_VLANS ||
	    vlanmc->vid >= RTL8367B_NUM_VIDS ||
	    vlanmc->priority > RTL8367B_PRIORITYMAX ||
	    vlanmc->member > RTL8367B_VLAN_MC0_MEMBER_MASK ||
	    vlanmc->untag > RTL8367B_UNTAG_MASK ||
	    vlanmc->fid > RTL8367B_FIDMAX)
		return -EINVAL;

	data[0] = (vlanmc->member & RTL8367B_VLAN_MC0_MEMBER_MASK) <<
		  RTL8367B_VLAN_MC0_MEMBER_SHIFT;
	data[1] = (vlanmc->fid & RTL8367B_VLAN_MC1_FID_MASK) <<
		  RTL8367B_VLAN_MC1_FID_SHIFT;
	data[2] = 0;
	data[3] = (vlanmc->vid & RTL8367B_VLAN_MC3_EVID_MASK) <<
		   RTL8367B_VLAN_MC3_EVID_SHIFT;

	for (i = 0; i < ARRAY_SIZE(data); i++)
		REG_WR(smi, RTL8367B_VLAN_MC_BASE(index) + i, data[i]);

	return 0;
}

static int rtl8367b_get_mc_index(struct rtl8366_smi *smi, int port, int *val)
{
	u32 data;
	int err;

	if (port >= RTL8367B_NUM_PORTS)
		return -EINVAL;

	REG_RD(smi, RTL8367B_VLAN_PVID_CTRL_REG(port), &data);

	*val = (data >> RTL8367B_VLAN_PVID_CTRL_SHIFT(port)) &
	       RTL8367B_VLAN_PVID_CTRL_MASK;

	return 0;
}

static int rtl8367b_set_mc_index(struct rtl8366_smi *smi, int port, int index)
{
	if (port >= RTL8367B_NUM_PORTS || index >= RTL8367B_NUM_VLANS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8367B_VLAN_PVID_CTRL_REG(port),
				RTL8367B_VLAN_PVID_CTRL_MASK <<
					RTL8367B_VLAN_PVID_CTRL_SHIFT(port),
				(index & RTL8367B_VLAN_PVID_CTRL_MASK) <<
					RTL8367B_VLAN_PVID_CTRL_SHIFT(port));
}

static int rtl8367b_enable_vlan(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8367B_VLAN_CTRL_REG,
				RTL8367B_VLAN_CTRL_ENABLE,
				(enable) ? RTL8367B_VLAN_CTRL_ENABLE : 0);
}

static int rtl8367b_enable_vlan4k(struct rtl8366_smi *smi, int enable)
{
	return 0;
}

static int rtl8367b_is_vlan_valid(struct rtl8366_smi *smi, unsigned vlan)
{
	unsigned max = RTL8367B_NUM_VLANS;

	if (smi->vlan4k_enabled)
		max = RTL8367B_NUM_VIDS - 1;

	if (vlan == 0 || vlan >= max)
		return 0;

	return 1;
}

static int rtl8367b_enable_port(struct rtl8366_smi *smi, int port, int enable)
{
	int err;

	REG_WR(smi, RTL8367B_PORT_ISOLATION_REG(port),
	       (enable) ? RTL8367B_PORTS_ALL : 0);

	return 0;
}

static int rtl8367b_sw_reset_mibs(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	return rtl8366_smi_rmwr(smi, RTL8367B_MIB_CTRL0_REG(0), 0,
				RTL8367B_MIB_CTRL0_GLOBAL_RESET_MASK);
}

static int rtl8367b_sw_get_port_link(struct switch_dev *dev,
				    int port,
				    struct switch_port_link *link)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;
	u32 speed;

	if (port >= RTL8367B_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8367B_PORT_STATUS_REG(port), &data);

	link->link = !!(data & RTL8367B_PORT_STATUS_LINK);
	if (!link->link)
		return 0;

	link->duplex = !!(data & RTL8367B_PORT_STATUS_DUPLEX);
	link->rx_flow = !!(data & RTL8367B_PORT_STATUS_RXPAUSE);
	link->tx_flow = !!(data & RTL8367B_PORT_STATUS_TXPAUSE);
	link->aneg = !!(data & RTL8367B_PORT_STATUS_NWAY);

	speed = (data & RTL8367B_PORT_STATUS_SPEED_MASK);
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

static int rtl8367b_sw_get_max_length(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8367B_SWC0_REG, &data);
	val->value.i = (data & RTL8367B_SWC0_MAX_LENGTH_MASK) >>
			RTL8367B_SWC0_MAX_LENGTH_SHIFT;

	return 0;
}

static int rtl8367b_sw_set_max_length(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 max_len;

	switch (val->value.i) {
	case 0:
		max_len = RTL8367B_SWC0_MAX_LENGTH_1522;
		break;
	case 1:
		max_len = RTL8367B_SWC0_MAX_LENGTH_1536;
		break;
	case 2:
		max_len = RTL8367B_SWC0_MAX_LENGTH_1552;
		break;
	case 3:
		max_len = RTL8367B_SWC0_MAX_LENGTH_16000;
		break;
	default:
		return -EINVAL;
	}

	return rtl8366_smi_rmwr(smi, RTL8367B_SWC0_REG,
			        RTL8367B_SWC0_MAX_LENGTH_MASK, max_len);
}


static int rtl8367b_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int port;

	port = val->port_vlan;
	if (port >= RTL8367B_NUM_PORTS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8367B_MIB_CTRL0_REG(port / 8), 0,
				RTL8367B_MIB_CTRL0_PORT_RESET_MASK(port % 8));
}

static int rtl8367b_sw_get_port_stats(struct switch_dev *dev, int port,
                                        struct switch_port_stats *stats)
{
	return (rtl8366_sw_get_port_stats(dev, port, stats,
				RTL8367B_MIB_TXB_ID, RTL8367B_MIB_RXB_ID));
}

static struct switch_attr rtl8367b_globals[] = {
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
		.set = rtl8367b_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "max_length",
		.description = "Get/Set the maximum length of valid packets"
			       "(0:1522, 1:1536, 2:1552, 3:16000)",
		.set = rtl8367b_sw_set_max_length,
		.get = rtl8367b_sw_get_max_length,
		.max = 3,
	}
};

static struct switch_attr rtl8367b_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl8367b_sw_reset_port_mibs,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.max = 33,
		.set = NULL,
		.get = rtl8366_sw_get_port_mib,
	},
};

static struct switch_attr rtl8367b_vlan[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "info",
		.description = "Get vlan information",
		.max = 1,
		.set = NULL,
		.get = rtl8366_sw_get_vlan_info,
	},
};

static const struct switch_dev_ops rtl8367b_sw_ops = {
	.attr_global = {
		.attr = rtl8367b_globals,
		.n_attr = ARRAY_SIZE(rtl8367b_globals),
	},
	.attr_port = {
		.attr = rtl8367b_port,
		.n_attr = ARRAY_SIZE(rtl8367b_port),
	},
	.attr_vlan = {
		.attr = rtl8367b_vlan,
		.n_attr = ARRAY_SIZE(rtl8367b_vlan),
	},

	.get_vlan_ports = rtl8366_sw_get_vlan_ports,
	.set_vlan_ports = rtl8366_sw_set_vlan_ports,
	.get_port_pvid = rtl8366_sw_get_port_pvid,
	.set_port_pvid = rtl8366_sw_set_port_pvid,
	.reset_switch = rtl8366_sw_reset_switch,
	.get_port_link = rtl8367b_sw_get_port_link,
	.get_port_stats = rtl8367b_sw_get_port_stats,
};

static int rtl8367b_switch_init(struct rtl8366_smi *smi)
{
	struct switch_dev *dev = &smi->sw_dev;
	int err;

	dev->name = "RTL8367B";
	dev->cpu_port = RTL8367B_CPU_PORT_NUM;
	dev->ports = RTL8367B_NUM_PORTS;
	dev->vlans = RTL8367B_NUM_VIDS;
	dev->ops = &rtl8367b_sw_ops;
	dev->alias = dev_name(smi->parent);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(smi->parent, "switch registration failed\n");

	return err;
}

static void rtl8367b_switch_cleanup(struct rtl8366_smi *smi)
{
	unregister_switch(&smi->sw_dev);
}

static int rtl8367b_mii_read(struct mii_bus *bus, int addr, int reg)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 val = 0;
	int err;

	err = rtl8367b_read_phy_reg(smi, addr, reg, &val);
	if (err)
		return 0xffff;

	return val;
}

static int rtl8367b_mii_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 t;
	int err;

	err = rtl8367b_write_phy_reg(smi, addr, reg, val);
	if (err)
		return err;

	/* flush write */
	(void) rtl8367b_read_phy_reg(smi, addr, reg, &t);

	return err;
}

static int rtl8367b_detect(struct rtl8366_smi *smi)
{
	const char *chip_name;
	u32 chip_num;
	u32 chip_ver;
	u32 chip_mode;
	int ret;

	/* TODO: improve chip detection */
	rtl8366_smi_write_reg(smi, RTL8367B_RTL_MAGIC_ID_REG,
			      RTL8367B_RTL_MAGIC_ID_VAL);

	ret = rtl8366_smi_read_reg(smi, RTL8367B_CHIP_NUMBER_REG, &chip_num);
	if (ret) {
		dev_err(smi->parent, "unable to read %s register\n",
			"chip number");
		return ret;
	}

	ret = rtl8366_smi_read_reg(smi, RTL8367B_CHIP_VER_REG, &chip_ver);
	if (ret) {
		dev_err(smi->parent, "unable to read %s register\n",
			"chip version");
		return ret;
	}

	ret = rtl8366_smi_read_reg(smi, RTL8367B_CHIP_MODE_REG, &chip_mode);
	if (ret) {
		dev_err(smi->parent, "unable to read %s register\n",
			"chip mode");
		return ret;
	}

	switch (chip_ver) {
	case 0x1000:
		chip_name = "8367RB";
		break;
	case 0x1010:
		chip_name = "8367R-VB";
		break;
	default:
		dev_err(smi->parent,
			"unknown chip num:%04x ver:%04x, mode:%04x\n",
			chip_num, chip_ver, chip_mode);
		return -ENODEV;
	}

	dev_info(smi->parent, "RTL%s chip found\n", chip_name);

	return 0;
}

static struct rtl8366_smi_ops rtl8367b_smi_ops = {
	.detect		= rtl8367b_detect,
	.reset_chip	= rtl8367b_reset_chip,
	.setup		= rtl8367b_setup,

	.mii_read	= rtl8367b_mii_read,
	.mii_write	= rtl8367b_mii_write,

	.get_vlan_mc	= rtl8367b_get_vlan_mc,
	.set_vlan_mc	= rtl8367b_set_vlan_mc,
	.get_vlan_4k	= rtl8367b_get_vlan_4k,
	.set_vlan_4k	= rtl8367b_set_vlan_4k,
	.get_mc_index	= rtl8367b_get_mc_index,
	.set_mc_index	= rtl8367b_set_mc_index,
	.get_mib_counter = rtl8367b_get_mib_counter,
	.is_vlan_valid	= rtl8367b_is_vlan_valid,
	.enable_vlan	= rtl8367b_enable_vlan,
	.enable_vlan4k	= rtl8367b_enable_vlan4k,
	.enable_port	= rtl8367b_enable_port,
};

static int  rtl8367b_probe(struct platform_device *pdev)
{
	struct rtl8366_smi *smi;
	int err;

	smi = rtl8366_smi_probe(pdev);
	if (!smi)
		return -ENODEV;

	smi->clk_delay = 1500;
	smi->cmd_read = 0xb9;
	smi->cmd_write = 0xb8;
	smi->ops = &rtl8367b_smi_ops;
	smi->cpu_port = RTL8367B_CPU_PORT_NUM;
	smi->num_ports = RTL8367B_NUM_PORTS;
	smi->num_vlan_mc = RTL8367B_NUM_VLANS;
	smi->mib_counters = rtl8367b_mib_counters;
	smi->num_mib_counters = ARRAY_SIZE(rtl8367b_mib_counters);

	err = rtl8366_smi_init(smi);
	if (err)
		goto err_free_smi;

	platform_set_drvdata(pdev, smi);

	err = rtl8367b_switch_init(smi);
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

static int rtl8367b_remove(struct platform_device *pdev)
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi) {
		rtl8367b_switch_cleanup(smi);
		platform_set_drvdata(pdev, NULL);
		rtl8366_smi_cleanup(smi);
		kfree(smi);
	}

	return 0;
}

static void rtl8367b_shutdown(struct platform_device *pdev)
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi)
		rtl8367b_reset_chip(smi);
}

#ifdef CONFIG_OF
static const struct of_device_id rtl8367b_match[] = {
	{ .compatible = "realtek,rtl8367b" },
	{},
};
MODULE_DEVICE_TABLE(of, rtl8367b_match);
#endif

static struct platform_driver rtl8367b_driver = {
	.driver = {
		.name		= RTL8367B_DRIVER_NAME,
		.owner		= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(rtl8367b_match),
#endif
	},
	.probe		= rtl8367b_probe,
	.remove		= rtl8367b_remove,
	.shutdown	= rtl8367b_shutdown,
};

module_platform_driver(rtl8367b_driver);

MODULE_DESCRIPTION("Realtek RTL8367B ethernet switch driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RTL8367B_DRIVER_NAME);

