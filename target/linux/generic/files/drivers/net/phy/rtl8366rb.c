/*
 * Platform driver for the Realtek RTL8366RB ethernet switch
 *
 * Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2010 Antti Seppälä <a.seppala@gmail.com>
 * Copyright (C) 2010 Roman Yeryomin <roman@advem.lv>
 * Copyright (C) 2011 Colin Leitner <colin.leitner@googlemail.com>
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
#include <linux/rtl8366.h>

#include "rtl8366_smi.h"

#define RTL8366RB_DRIVER_DESC	"Realtek RTL8366RB ethernet switch driver"
#define RTL8366RB_DRIVER_VER	"0.2.4"

#define RTL8366RB_PHY_NO_MAX	4
#define RTL8366RB_PHY_PAGE_MAX	7
#define RTL8366RB_PHY_ADDR_MAX	31

/* Switch Global Configuration register */
#define RTL8366RB_SGCR				0x0000
#define RTL8366RB_SGCR_EN_BC_STORM_CTRL		BIT(0)
#define RTL8366RB_SGCR_MAX_LENGTH(_x)		(_x << 4)
#define RTL8366RB_SGCR_MAX_LENGTH_MASK		RTL8366RB_SGCR_MAX_LENGTH(0x3)
#define RTL8366RB_SGCR_MAX_LENGTH_1522		RTL8366RB_SGCR_MAX_LENGTH(0x0)
#define RTL8366RB_SGCR_MAX_LENGTH_1536		RTL8366RB_SGCR_MAX_LENGTH(0x1)
#define RTL8366RB_SGCR_MAX_LENGTH_1552		RTL8366RB_SGCR_MAX_LENGTH(0x2)
#define RTL8366RB_SGCR_MAX_LENGTH_9216		RTL8366RB_SGCR_MAX_LENGTH(0x3)
#define RTL8366RB_SGCR_EN_VLAN			BIT(13)
#define RTL8366RB_SGCR_EN_VLAN_4KTB		BIT(14)

/* Port Enable Control register */
#define RTL8366RB_PECR				0x0001

/* Port Mirror Control Register */
#define RTL8366RB_PMCR				0x0007
#define RTL8366RB_PMCR_SOURCE_PORT(_x)		(_x)
#define RTL8366RB_PMCR_SOURCE_PORT_MASK		0x000f
#define RTL8366RB_PMCR_MONITOR_PORT(_x)		((_x) << 4)
#define RTL8366RB_PMCR_MONITOR_PORT_MASK	0x00f0
#define RTL8366RB_PMCR_MIRROR_RX		BIT(8)
#define RTL8366RB_PMCR_MIRROR_TX		BIT(9)
#define RTL8366RB_PMCR_MIRROR_SPC		BIT(10)
#define RTL8366RB_PMCR_MIRROR_ISO		BIT(11)

/* Switch Security Control registers */
#define RTL8366RB_SSCR0				0x0002
#define RTL8366RB_SSCR1				0x0003
#define RTL8366RB_SSCR2				0x0004
#define RTL8366RB_SSCR2_DROP_UNKNOWN_DA		BIT(0)

#define RTL8366RB_RESET_CTRL_REG		0x0100
#define RTL8366RB_CHIP_CTRL_RESET_HW		1
#define RTL8366RB_CHIP_CTRL_RESET_SW		(1 << 1)

#define RTL8366RB_CHIP_VERSION_CTRL_REG		0x050A
#define RTL8366RB_CHIP_VERSION_MASK		0xf
#define RTL8366RB_CHIP_ID_REG			0x0509
#define RTL8366RB_CHIP_ID_8366			0x5937

/* PHY registers control */
#define RTL8366RB_PHY_ACCESS_CTRL_REG		0x8000
#define RTL8366RB_PHY_ACCESS_DATA_REG		0x8002

#define RTL8366RB_PHY_CTRL_READ			1
#define RTL8366RB_PHY_CTRL_WRITE		0

#define RTL8366RB_PHY_REG_MASK			0x1f
#define RTL8366RB_PHY_PAGE_OFFSET		5
#define RTL8366RB_PHY_PAGE_MASK			(0xf << 5)
#define RTL8366RB_PHY_NO_OFFSET			9
#define RTL8366RB_PHY_NO_MASK			(0x1f << 9)

#define RTL8366RB_VLAN_INGRESS_CTRL2_REG	0x037f

/* LED control registers */
#define RTL8366RB_LED_BLINKRATE_REG		0x0430
#define RTL8366RB_LED_BLINKRATE_BIT		0
#define RTL8366RB_LED_BLINKRATE_MASK		0x0007

#define RTL8366RB_LED_CTRL_REG			0x0431
#define RTL8366RB_LED_0_1_CTRL_REG		0x0432
#define RTL8366RB_LED_2_3_CTRL_REG		0x0433

#define RTL8366RB_MIB_COUNT			33
#define RTL8366RB_GLOBAL_MIB_COUNT		1
#define RTL8366RB_MIB_COUNTER_PORT_OFFSET	0x0050
#define RTL8366RB_MIB_COUNTER_BASE		0x1000
#define RTL8366RB_MIB_CTRL_REG			0x13F0
#define RTL8366RB_MIB_CTRL_USER_MASK		0x0FFC
#define RTL8366RB_MIB_CTRL_BUSY_MASK		BIT(0)
#define RTL8366RB_MIB_CTRL_RESET_MASK		BIT(1)
#define RTL8366RB_MIB_CTRL_PORT_RESET(_p)	BIT(2 + (_p))
#define RTL8366RB_MIB_CTRL_GLOBAL_RESET		BIT(11)

#define RTL8366RB_PORT_VLAN_CTRL_BASE		0x0063
#define RTL8366RB_PORT_VLAN_CTRL_REG(_p)  \
		(RTL8366RB_PORT_VLAN_CTRL_BASE + (_p) / 4)
#define RTL8366RB_PORT_VLAN_CTRL_MASK		0xf
#define RTL8366RB_PORT_VLAN_CTRL_SHIFT(_p)	(4 * ((_p) % 4))


#define RTL8366RB_VLAN_TABLE_READ_BASE		0x018C
#define RTL8366RB_VLAN_TABLE_WRITE_BASE		0x0185


#define RTL8366RB_TABLE_ACCESS_CTRL_REG		0x0180
#define RTL8366RB_TABLE_VLAN_READ_CTRL		0x0E01
#define RTL8366RB_TABLE_VLAN_WRITE_CTRL		0x0F01

#define RTL8366RB_VLAN_MC_BASE(_x)		(0x0020 + (_x) * 3)


#define RTL8366RB_PORT_LINK_STATUS_BASE		0x0014
#define RTL8366RB_PORT_STATUS_SPEED_MASK	0x0003
#define RTL8366RB_PORT_STATUS_DUPLEX_MASK	0x0004
#define RTL8366RB_PORT_STATUS_LINK_MASK		0x0010
#define RTL8366RB_PORT_STATUS_TXPAUSE_MASK	0x0020
#define RTL8366RB_PORT_STATUS_RXPAUSE_MASK	0x0040
#define RTL8366RB_PORT_STATUS_AN_MASK		0x0080


#define RTL8366RB_PORT_NUM_CPU		5
#define RTL8366RB_NUM_PORTS		6
#define RTL8366RB_NUM_VLANS		16
#define RTL8366RB_NUM_LEDGROUPS		4
#define RTL8366RB_NUM_VIDS		4096
#define RTL8366RB_PRIORITYMAX		7
#define RTL8366RB_FIDMAX		7


#define RTL8366RB_PORT_1		(1 << 0) /* In userspace port 0 */
#define RTL8366RB_PORT_2		(1 << 1) /* In userspace port 1 */
#define RTL8366RB_PORT_3		(1 << 2) /* In userspace port 2 */
#define RTL8366RB_PORT_4		(1 << 3) /* In userspace port 3 */
#define RTL8366RB_PORT_5		(1 << 4) /* In userspace port 4 */

#define RTL8366RB_PORT_CPU		(1 << 5) /* CPU port */

#define RTL8366RB_PORT_ALL		(RTL8366RB_PORT_1 |	\
					 RTL8366RB_PORT_2 |	\
					 RTL8366RB_PORT_3 |	\
					 RTL8366RB_PORT_4 |	\
					 RTL8366RB_PORT_5 |	\
					 RTL8366RB_PORT_CPU)

#define RTL8366RB_PORT_ALL_BUT_CPU	(RTL8366RB_PORT_1 |	\
					 RTL8366RB_PORT_2 |	\
					 RTL8366RB_PORT_3 |	\
					 RTL8366RB_PORT_4 |	\
					 RTL8366RB_PORT_5)

#define RTL8366RB_PORT_ALL_EXTERNAL	(RTL8366RB_PORT_1 |	\
					 RTL8366RB_PORT_2 |	\
					 RTL8366RB_PORT_3 |	\
					 RTL8366RB_PORT_4)

#define RTL8366RB_PORT_ALL_INTERNAL	 RTL8366RB_PORT_CPU

#define RTL8366RB_VLAN_VID_MASK		0xfff
#define RTL8366RB_VLAN_PRIORITY_SHIFT	12
#define RTL8366RB_VLAN_PRIORITY_MASK	0x7
#define RTL8366RB_VLAN_UNTAG_SHIFT	8
#define RTL8366RB_VLAN_UNTAG_MASK	0xff
#define RTL8366RB_VLAN_MEMBER_MASK	0xff
#define RTL8366RB_VLAN_FID_MASK		0x7


/* Port ingress bandwidth control */
#define RTL8366RB_IB_BASE		0x0200
#define RTL8366RB_IB_REG(pnum)		(RTL8366RB_IB_BASE + pnum)
#define RTL8366RB_IB_BDTH_MASK		0x3fff
#define RTL8366RB_IB_PREIFG_OFFSET	14
#define RTL8366RB_IB_PREIFG_MASK	(1 << RTL8366RB_IB_PREIFG_OFFSET)

/* Port egress bandwidth control */
#define RTL8366RB_EB_BASE		0x02d1
#define RTL8366RB_EB_REG(pnum)		(RTL8366RB_EB_BASE + pnum)
#define RTL8366RB_EB_BDTH_MASK		0x3fff
#define RTL8366RB_EB_PREIFG_REG	0x02f8
#define RTL8366RB_EB_PREIFG_OFFSET	9
#define RTL8366RB_EB_PREIFG_MASK	(1 << RTL8366RB_EB_PREIFG_OFFSET)

#define RTL8366RB_BDTH_SW_MAX		1048512
#define RTL8366RB_BDTH_UNIT		64
#define RTL8366RB_BDTH_REG_DEFAULT	16383

/* QOS */
#define RTL8366RB_QOS_BIT		15
#define RTL8366RB_QOS_MASK		(1 << RTL8366RB_QOS_BIT)
/* Include/Exclude Preamble and IFG (20 bytes). 0:Exclude, 1:Include. */
#define RTL8366RB_QOS_DEFAULT_PREIFG	1


#define RTL8366RB_MIB_RXB_ID		0	/* IfInOctets */
#define RTL8366RB_MIB_TXB_ID		20	/* IfOutOctets */

static struct rtl8366_mib_counter rtl8366rb_mib_counters[] = {
	{ 0,  0, 4, "IfInOctets"				},
	{ 0,  4, 4, "EtherStatsOctets"				},
	{ 0,  8, 2, "EtherStatsUnderSizePkts"			},
	{ 0, 10, 2, "EtherFragments"				},
	{ 0, 12, 2, "EtherStatsPkts64Octets"			},
	{ 0, 14, 2, "EtherStatsPkts65to127Octets"		},
	{ 0, 16, 2, "EtherStatsPkts128to255Octets"		},
	{ 0, 18, 2, "EtherStatsPkts256to511Octets"		},
	{ 0, 20, 2, "EtherStatsPkts512to1023Octets"		},
	{ 0, 22, 2, "EtherStatsPkts1024to1518Octets"		},
	{ 0, 24, 2, "EtherOversizeStats"			},
	{ 0, 26, 2, "EtherStatsJabbers"				},
	{ 0, 28, 2, "IfInUcastPkts"				},
	{ 0, 30, 2, "EtherStatsMulticastPkts"			},
	{ 0, 32, 2, "EtherStatsBroadcastPkts"			},
	{ 0, 34, 2, "EtherStatsDropEvents"			},
	{ 0, 36, 2, "Dot3StatsFCSErrors"			},
	{ 0, 38, 2, "Dot3StatsSymbolErrors"			},
	{ 0, 40, 2, "Dot3InPauseFrames"				},
	{ 0, 42, 2, "Dot3ControlInUnknownOpcodes"		},
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
};

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

static int rtl8366rb_reset_chip(struct rtl8366_smi *smi)
{
	int timeout = 10;
	u32 data;

	rtl8366_smi_write_reg_noack(smi, RTL8366RB_RESET_CTRL_REG,
			 	    RTL8366RB_CHIP_CTRL_RESET_HW);
	do {
		msleep(1);
		if (rtl8366_smi_read_reg(smi, RTL8366RB_RESET_CTRL_REG, &data))
			return -EIO;

		if (!(data & RTL8366RB_CHIP_CTRL_RESET_HW))
			break;
	} while (--timeout);

	if (!timeout) {
		printk("Timeout waiting for the switch to reset\n");
		return -EIO;
	}

	return 0;
}

static int rtl8366rb_setup(struct rtl8366_smi *smi)
{
	int err;
#ifdef CONFIG_OF
	unsigned i;
	struct device_node *np;
	unsigned num_initvals;
	const __be32 *paddr;

	np = smi->parent->of_node;

	paddr = of_get_property(np, "realtek,initvals", &num_initvals);
	if (paddr) {
		dev_info(smi->parent, "applying initvals from DTS\n");

		if (num_initvals < (2 * sizeof(*paddr)))
			return -EINVAL;

		num_initvals /= sizeof(*paddr);

		for (i = 0; i < num_initvals - 1; i += 2) {
			u32 reg = be32_to_cpup(paddr + i);
			u32 val = be32_to_cpup(paddr + i + 1);

			REG_WR(smi, reg, val);
		}
	}
#endif

	/* set maximum packet length to 1536 bytes */
	REG_RMW(smi, RTL8366RB_SGCR, RTL8366RB_SGCR_MAX_LENGTH_MASK,
		RTL8366RB_SGCR_MAX_LENGTH_1536);

	/* enable learning for all ports */
	REG_WR(smi, RTL8366RB_SSCR0, 0);

	/* enable auto ageing for all ports */
	REG_WR(smi, RTL8366RB_SSCR1, 0);

	/*
	 * discard VLAN tagged packets if the port is not a member of
	 * the VLAN with which the packets is associated.
	 */
	REG_WR(smi, RTL8366RB_VLAN_INGRESS_CTRL2_REG, RTL8366RB_PORT_ALL);

	/* don't drop packets whose DA has not been learned */
	REG_RMW(smi, RTL8366RB_SSCR2, RTL8366RB_SSCR2_DROP_UNKNOWN_DA, 0);

	return 0;
}

static int rtl8366rb_read_phy_reg(struct rtl8366_smi *smi,
				 u32 phy_no, u32 page, u32 addr, u32 *data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366RB_PHY_NO_MAX)
		return -EINVAL;

	if (page > RTL8366RB_PHY_PAGE_MAX)
		return -EINVAL;

	if (addr > RTL8366RB_PHY_ADDR_MAX)
		return -EINVAL;

	ret = rtl8366_smi_write_reg(smi, RTL8366RB_PHY_ACCESS_CTRL_REG,
				    RTL8366RB_PHY_CTRL_READ);
	if (ret)
		return ret;

	reg = 0x8000 | (1 << (phy_no + RTL8366RB_PHY_NO_OFFSET)) |
	      ((page << RTL8366RB_PHY_PAGE_OFFSET) & RTL8366RB_PHY_PAGE_MASK) |
	      (addr & RTL8366RB_PHY_REG_MASK);

	ret = rtl8366_smi_write_reg(smi, reg, 0);
	if (ret)
		return ret;

	ret = rtl8366_smi_read_reg(smi, RTL8366RB_PHY_ACCESS_DATA_REG, data);
	if (ret)
		return ret;

	return 0;
}

static int rtl8366rb_write_phy_reg(struct rtl8366_smi *smi,
				  u32 phy_no, u32 page, u32 addr, u32 data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366RB_PHY_NO_MAX)
		return -EINVAL;

	if (page > RTL8366RB_PHY_PAGE_MAX)
		return -EINVAL;

	if (addr > RTL8366RB_PHY_ADDR_MAX)
		return -EINVAL;

	ret = rtl8366_smi_write_reg(smi, RTL8366RB_PHY_ACCESS_CTRL_REG,
				    RTL8366RB_PHY_CTRL_WRITE);
	if (ret)
		return ret;

	reg = 0x8000 | (1 << (phy_no + RTL8366RB_PHY_NO_OFFSET)) |
	      ((page << RTL8366RB_PHY_PAGE_OFFSET) & RTL8366RB_PHY_PAGE_MASK) |
	      (addr & RTL8366RB_PHY_REG_MASK);

	ret = rtl8366_smi_write_reg(smi, reg, data);
	if (ret)
		return ret;

	return 0;
}

static int rtl8366rb_get_mib_counter(struct rtl8366_smi *smi, int counter,
				     int port, unsigned long long *val)
{
	int i;
	int err;
	u32 addr, data;
	u64 mibvalue;

	if (port > RTL8366RB_NUM_PORTS || counter >= RTL8366RB_MIB_COUNT)
		return -EINVAL;

	addr = RTL8366RB_MIB_COUNTER_BASE +
	       RTL8366RB_MIB_COUNTER_PORT_OFFSET * (port) +
	       rtl8366rb_mib_counters[counter].offset;

	/*
	 * Writing access counter address first
	 * then ASIC will prepare 64bits counter wait for being retrived
	 */
	data = 0; /* writing data will be discard by ASIC */
	err = rtl8366_smi_write_reg(smi, addr, data);
	if (err)
		return err;

	/* read MIB control register */
	err =  rtl8366_smi_read_reg(smi, RTL8366RB_MIB_CTRL_REG, &data);
	if (err)
		return err;

	if (data & RTL8366RB_MIB_CTRL_BUSY_MASK)
		return -EBUSY;

	if (data & RTL8366RB_MIB_CTRL_RESET_MASK)
		return -EIO;

	mibvalue = 0;
	for (i = rtl8366rb_mib_counters[counter].length; i > 0; i--) {
		err = rtl8366_smi_read_reg(smi, addr + (i - 1), &data);
		if (err)
			return err;

		mibvalue = (mibvalue << 16) | (data & 0xFFFF);
	}

	*val = mibvalue;
	return 0;
}

static int rtl8366rb_get_vlan_4k(struct rtl8366_smi *smi, u32 vid,
				 struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[3];
	int err;
	int i;

	memset(vlan4k, '\0', sizeof(struct rtl8366_vlan_4k));

	if (vid >= RTL8366RB_NUM_VIDS)
		return -EINVAL;

	/* write VID */
	err = rtl8366_smi_write_reg(smi, RTL8366RB_VLAN_TABLE_WRITE_BASE,
				    vid & RTL8366RB_VLAN_VID_MASK);
	if (err)
		return err;

	/* write table access control word */
	err = rtl8366_smi_write_reg(smi, RTL8366RB_TABLE_ACCESS_CTRL_REG,
				    RTL8366RB_TABLE_VLAN_READ_CTRL);
	if (err)
		return err;

	for (i = 0; i < 3; i++) {
		err = rtl8366_smi_read_reg(smi,
					   RTL8366RB_VLAN_TABLE_READ_BASE + i,
					   &data[i]);
		if (err)
			return err;
	}

	vlan4k->vid = vid;
	vlan4k->untag = (data[1] >> RTL8366RB_VLAN_UNTAG_SHIFT) &
			RTL8366RB_VLAN_UNTAG_MASK;
	vlan4k->member = data[1] & RTL8366RB_VLAN_MEMBER_MASK;
	vlan4k->fid = data[2] & RTL8366RB_VLAN_FID_MASK;

	return 0;
}

static int rtl8366rb_set_vlan_4k(struct rtl8366_smi *smi,
				 const struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[3];
	int err;
	int i;

	if (vlan4k->vid >= RTL8366RB_NUM_VIDS ||
	    vlan4k->member > RTL8366RB_VLAN_MEMBER_MASK ||
	    vlan4k->untag > RTL8366RB_VLAN_UNTAG_MASK ||
	    vlan4k->fid > RTL8366RB_FIDMAX)
		return -EINVAL;

	data[0] = vlan4k->vid & RTL8366RB_VLAN_VID_MASK;
	data[1] = (vlan4k->member & RTL8366RB_VLAN_MEMBER_MASK) |
		  ((vlan4k->untag & RTL8366RB_VLAN_UNTAG_MASK) <<
			RTL8366RB_VLAN_UNTAG_SHIFT);
	data[2] = vlan4k->fid & RTL8366RB_VLAN_FID_MASK;

	for (i = 0; i < 3; i++) {
		err = rtl8366_smi_write_reg(smi,
					    RTL8366RB_VLAN_TABLE_WRITE_BASE + i,
					    data[i]);
		if (err)
			return err;
	}

	/* write table access control word */
	err = rtl8366_smi_write_reg(smi, RTL8366RB_TABLE_ACCESS_CTRL_REG,
				    RTL8366RB_TABLE_VLAN_WRITE_CTRL);

	return err;
}

static int rtl8366rb_get_vlan_mc(struct rtl8366_smi *smi, u32 index,
				 struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[3];
	int err;
	int i;

	memset(vlanmc, '\0', sizeof(struct rtl8366_vlan_mc));

	if (index >= RTL8366RB_NUM_VLANS)
		return -EINVAL;

	for (i = 0; i < 3; i++) {
		err = rtl8366_smi_read_reg(smi,
					   RTL8366RB_VLAN_MC_BASE(index) + i,
					   &data[i]);
		if (err)
			return err;
	}

	vlanmc->vid = data[0] & RTL8366RB_VLAN_VID_MASK;
	vlanmc->priority = (data[0] >> RTL8366RB_VLAN_PRIORITY_SHIFT) &
			   RTL8366RB_VLAN_PRIORITY_MASK;
	vlanmc->untag = (data[1] >> RTL8366RB_VLAN_UNTAG_SHIFT) &
			RTL8366RB_VLAN_UNTAG_MASK;
	vlanmc->member = data[1] & RTL8366RB_VLAN_MEMBER_MASK;
	vlanmc->fid = data[2] & RTL8366RB_VLAN_FID_MASK;

	return 0;
}

static int rtl8366rb_set_vlan_mc(struct rtl8366_smi *smi, u32 index,
				 const struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[3];
	int err;
	int i;

	if (index >= RTL8366RB_NUM_VLANS ||
	    vlanmc->vid >= RTL8366RB_NUM_VIDS ||
	    vlanmc->priority > RTL8366RB_PRIORITYMAX ||
	    vlanmc->member > RTL8366RB_VLAN_MEMBER_MASK ||
	    vlanmc->untag > RTL8366RB_VLAN_UNTAG_MASK ||
	    vlanmc->fid > RTL8366RB_FIDMAX)
		return -EINVAL;

	data[0] = (vlanmc->vid & RTL8366RB_VLAN_VID_MASK) |
		  ((vlanmc->priority & RTL8366RB_VLAN_PRIORITY_MASK) <<
			RTL8366RB_VLAN_PRIORITY_SHIFT);
	data[1] = (vlanmc->member & RTL8366RB_VLAN_MEMBER_MASK) |
		  ((vlanmc->untag & RTL8366RB_VLAN_UNTAG_MASK) <<
			RTL8366RB_VLAN_UNTAG_SHIFT);
	data[2] = vlanmc->fid & RTL8366RB_VLAN_FID_MASK;

	for (i = 0; i < 3; i++) {
		err = rtl8366_smi_write_reg(smi,
					    RTL8366RB_VLAN_MC_BASE(index) + i,
					    data[i]);
		if (err)
			return err;
	}

	return 0;
}

static int rtl8366rb_get_mc_index(struct rtl8366_smi *smi, int port, int *val)
{
	u32 data;
	int err;

	if (port >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	err = rtl8366_smi_read_reg(smi, RTL8366RB_PORT_VLAN_CTRL_REG(port),
				   &data);
	if (err)
		return err;

	*val = (data >> RTL8366RB_PORT_VLAN_CTRL_SHIFT(port)) &
	       RTL8366RB_PORT_VLAN_CTRL_MASK;

	return 0;

}

static int rtl8366rb_set_mc_index(struct rtl8366_smi *smi, int port, int index)
{
	if (port >= RTL8366RB_NUM_PORTS || index >= RTL8366RB_NUM_VLANS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PORT_VLAN_CTRL_REG(port),
				RTL8366RB_PORT_VLAN_CTRL_MASK <<
					RTL8366RB_PORT_VLAN_CTRL_SHIFT(port),
				(index & RTL8366RB_PORT_VLAN_CTRL_MASK) <<
					RTL8366RB_PORT_VLAN_CTRL_SHIFT(port));
}

static int rtl8366rb_is_vlan_valid(struct rtl8366_smi *smi, unsigned vlan)
{
	unsigned max = RTL8366RB_NUM_VLANS;

	if (smi->vlan4k_enabled)
		max = RTL8366RB_NUM_VIDS - 1;

	if (vlan == 0 || vlan >= max)
		return 0;

	return 1;
}

static int rtl8366rb_enable_vlan(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366RB_SGCR, RTL8366RB_SGCR_EN_VLAN,
				(enable) ? RTL8366RB_SGCR_EN_VLAN : 0);
}

static int rtl8366rb_enable_vlan4k(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366RB_SGCR,
				RTL8366RB_SGCR_EN_VLAN_4KTB,
				(enable) ? RTL8366RB_SGCR_EN_VLAN_4KTB : 0);
}

static int rtl8366rb_enable_port(struct rtl8366_smi *smi, int port, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366RB_PECR, (1 << port),
				(enable) ? 0 : (1 << port));
}

static int rtl8366rb_sw_reset_mibs(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	return rtl8366_smi_rmwr(smi, RTL8366RB_MIB_CTRL_REG, 0,
			        RTL8366RB_MIB_CTRL_GLOBAL_RESET);
}

static int rtl8366rb_sw_get_blinkrate(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_LED_BLINKRATE_REG, &data);

	val->value.i = (data & (RTL8366RB_LED_BLINKRATE_MASK));

	return 0;
}

static int rtl8366rb_sw_set_blinkrate(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->value.i >= 6)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8366RB_LED_BLINKRATE_REG,
				RTL8366RB_LED_BLINKRATE_MASK,
				val->value.i);
}

static int rtl8366rb_sw_get_learning_enable(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_SSCR0, &data);
	val->value.i = !data;

	return 0;
}


static int rtl8366rb_sw_set_learning_enable(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 portmask = 0;
	int err = 0;

	if (!val->value.i)
		portmask = RTL8366RB_PORT_ALL;

	/* set learning for all ports */
	REG_WR(smi, RTL8366RB_SSCR0, portmask);

	/* set auto ageing for all ports */
	REG_WR(smi, RTL8366RB_SSCR1, portmask);

	return 0;
}

static int rtl8366rb_sw_get_port_link(struct switch_dev *dev,
				     int port,
				     struct switch_port_link *link)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;
	u32 speed;

	if (port >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366RB_PORT_LINK_STATUS_BASE + (port / 2),
			     &data);

	if (port % 2)
		data = data >> 8;

	link->link = !!(data & RTL8366RB_PORT_STATUS_LINK_MASK);
	if (!link->link)
		return 0;

	link->duplex = !!(data & RTL8366RB_PORT_STATUS_DUPLEX_MASK);
	link->rx_flow = !!(data & RTL8366RB_PORT_STATUS_RXPAUSE_MASK);
	link->tx_flow = !!(data & RTL8366RB_PORT_STATUS_TXPAUSE_MASK);
	link->aneg = !!(data & RTL8366RB_PORT_STATUS_AN_MASK);

	speed = (data & RTL8366RB_PORT_STATUS_SPEED_MASK);
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

static int rtl8366rb_sw_set_port_led(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;
	u32 mask;
	u32 reg;

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	if (val->port_vlan == RTL8366RB_PORT_NUM_CPU) {
		reg = RTL8366RB_LED_BLINKRATE_REG;
		mask = 0xF << 4;
		data = val->value.i << 4;
	} else {
		reg = RTL8366RB_LED_CTRL_REG;
		mask = 0xF << (val->port_vlan * 4),
		data = val->value.i << (val->port_vlan * 4);
	}

	return rtl8366_smi_rmwr(smi, reg, mask, data);
}

static int rtl8366rb_sw_get_port_led(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;

	if (val->port_vlan >= RTL8366RB_NUM_LEDGROUPS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366RB_LED_CTRL_REG, &data);
	val->value.i = (data >> (val->port_vlan * 4)) & 0x000F;

	return 0;
}

static int rtl8366rb_sw_set_port_disable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 mask, data;

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	mask = 1 << val->port_vlan ;
	if (val->value.i)
		data = mask;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PECR, mask, data);
}

static int rtl8366rb_sw_get_port_disable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366RB_PECR, &data);
	if (data & (1 << val->port_vlan))
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_port_rate_in(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	if (val->value.i > 0 && val->value.i < RTL8366RB_BDTH_SW_MAX)
		val->value.i = (val->value.i - 1) / RTL8366RB_BDTH_UNIT;
	else
		val->value.i = RTL8366RB_BDTH_REG_DEFAULT;

	return rtl8366_smi_rmwr(smi, RTL8366RB_IB_REG(val->port_vlan),
		RTL8366RB_IB_BDTH_MASK | RTL8366RB_IB_PREIFG_MASK,
		val->value.i |
		(RTL8366RB_QOS_DEFAULT_PREIFG << RTL8366RB_IB_PREIFG_OFFSET));

}

static int rtl8366rb_sw_get_port_rate_in(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366RB_IB_REG(val->port_vlan), &data);
	data &= RTL8366RB_IB_BDTH_MASK;
	if (data < RTL8366RB_IB_BDTH_MASK)
		data += 1;

	val->value.i = (int)data * RTL8366RB_BDTH_UNIT;

	return 0;
}

static int rtl8366rb_sw_set_port_rate_out(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_rmwr(smi, RTL8366RB_EB_PREIFG_REG,
		RTL8366RB_EB_PREIFG_MASK,
		(RTL8366RB_QOS_DEFAULT_PREIFG << RTL8366RB_EB_PREIFG_OFFSET));

	if (val->value.i > 0 && val->value.i < RTL8366RB_BDTH_SW_MAX)
		val->value.i = (val->value.i - 1) / RTL8366RB_BDTH_UNIT;
	else
		val->value.i = RTL8366RB_BDTH_REG_DEFAULT;

	return rtl8366_smi_rmwr(smi, RTL8366RB_EB_REG(val->port_vlan),
			RTL8366RB_EB_BDTH_MASK, val->value.i );

}

static int rtl8366rb_sw_get_port_rate_out(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366RB_EB_REG(val->port_vlan), &data);
	data &= RTL8366RB_EB_BDTH_MASK;
	if (data < RTL8366RB_EB_BDTH_MASK)
		data += 1;

	val->value.i = (int)data * RTL8366RB_BDTH_UNIT;

	return 0;
}

static int rtl8366rb_sw_set_qos_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->value.i)
		data = RTL8366RB_QOS_MASK;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_SGCR, RTL8366RB_QOS_MASK, data);
}

static int rtl8366rb_sw_get_qos_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_SGCR, &data);
	if (data & RTL8366RB_QOS_MASK)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_mirror_rx_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->value.i)
		data = RTL8366RB_PMCR_MIRROR_RX;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_MIRROR_RX, data);
}

static int rtl8366rb_sw_get_mirror_rx_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	if (data & RTL8366RB_PMCR_MIRROR_RX)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_mirror_tx_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->value.i)
		data = RTL8366RB_PMCR_MIRROR_TX;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_MIRROR_TX, data);
}

static int rtl8366rb_sw_get_mirror_tx_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	if (data & RTL8366RB_PMCR_MIRROR_TX)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_monitor_isolation_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->value.i)
		data = RTL8366RB_PMCR_MIRROR_ISO;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_MIRROR_ISO, data);
}

static int rtl8366rb_sw_get_monitor_isolation_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	if (data & RTL8366RB_PMCR_MIRROR_ISO)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_mirror_pause_frames_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	if (val->value.i)
		data = RTL8366RB_PMCR_MIRROR_SPC;
	else
		data = 0;

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_MIRROR_SPC, data);
}

static int rtl8366rb_sw_get_mirror_pause_frames_enable(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	if (data & RTL8366RB_PMCR_MIRROR_SPC)
		val->value.i = 1;
	else
		val->value.i = 0;

	return 0;
}

static int rtl8366rb_sw_set_mirror_monitor_port(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	data = RTL8366RB_PMCR_MONITOR_PORT(val->value.i);

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_MONITOR_PORT_MASK, data);
}

static int rtl8366rb_sw_get_mirror_monitor_port(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	val->value.i = (data & RTL8366RB_PMCR_MONITOR_PORT_MASK) >> 4;

	return 0;
}

static int rtl8366rb_sw_set_mirror_source_port(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	data = RTL8366RB_PMCR_SOURCE_PORT(val->value.i);

	return rtl8366_smi_rmwr(smi, RTL8366RB_PMCR, RTL8366RB_PMCR_SOURCE_PORT_MASK, data);
}

static int rtl8366rb_sw_get_mirror_source_port(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366RB_PMCR, &data);
	val->value.i = data & RTL8366RB_PMCR_SOURCE_PORT_MASK;

	return 0;
}

static int rtl8366rb_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->port_vlan >= RTL8366RB_NUM_PORTS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8366RB_MIB_CTRL_REG, 0,
				RTL8366RB_MIB_CTRL_PORT_RESET(val->port_vlan));
}

static int rtl8366rb_sw_get_port_stats(struct switch_dev *dev, int port,
					struct switch_port_stats *stats)
{
	return (rtl8366_sw_get_port_stats(dev, port, stats,
				RTL8366RB_MIB_TXB_ID, RTL8366RB_MIB_RXB_ID));
}

static struct switch_attr rtl8366rb_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_learning",
		.description = "Enable learning, enable aging",
		.set = rtl8366rb_sw_set_learning_enable,
		.get = rtl8366rb_sw_get_learning_enable,
		.max = 1
	}, {
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
		.set = rtl8366rb_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "blinkrate",
		.description = "Get/Set LED blinking rate (0 = 43ms, 1 = 84ms,"
		" 2 = 120ms, 3 = 170ms, 4 = 340ms, 5 = 670ms)",
		.set = rtl8366rb_sw_set_blinkrate,
		.get = rtl8366rb_sw_get_blinkrate,
		.max = 5
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_qos",
		.description = "Enable QOS",
		.set = rtl8366rb_sw_set_qos_enable,
		.get = rtl8366rb_sw_get_qos_enable,
		.max = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_rx",
		.description = "Enable mirroring of RX packets",
		.set = rtl8366rb_sw_set_mirror_rx_enable,
		.get = rtl8366rb_sw_get_mirror_rx_enable,
		.max = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_tx",
		.description = "Enable mirroring of TX packets",
		.set = rtl8366rb_sw_set_mirror_tx_enable,
		.get = rtl8366rb_sw_get_mirror_tx_enable,
		.max = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_monitor_isolation",
		.description = "Enable isolation of monitor port (TX packets will be dropped)",
		.set = rtl8366rb_sw_set_monitor_isolation_enable,
		.get = rtl8366rb_sw_get_monitor_isolation_enable,
		.max = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "enable_mirror_pause_frames",
		.description = "Enable mirroring of RX pause frames",
		.set = rtl8366rb_sw_set_mirror_pause_frames_enable,
		.get = rtl8366rb_sw_get_mirror_pause_frames_enable,
		.max = 1
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "mirror_monitor_port",
		.description = "Mirror monitor port",
		.set = rtl8366rb_sw_set_mirror_monitor_port,
		.get = rtl8366rb_sw_get_mirror_monitor_port,
		.max = 5
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "mirror_source_port",
		.description = "Mirror source port",
		.set = rtl8366rb_sw_set_mirror_source_port,
		.get = rtl8366rb_sw_get_mirror_source_port,
		.max = 5
	},
};

static struct switch_attr rtl8366rb_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl8366rb_sw_reset_port_mibs,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.max = 33,
		.set = NULL,
		.get = rtl8366_sw_get_port_mib,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "led",
		.description = "Get/Set port group (0 - 3) led mode (0 - 15)",
		.max = 15,
		.set = rtl8366rb_sw_set_port_led,
		.get = rtl8366rb_sw_get_port_led,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "disable",
		.description = "Get/Set port state (enabled or disabled)",
		.max = 1,
		.set = rtl8366rb_sw_set_port_disable,
		.get = rtl8366rb_sw_get_port_disable,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "rate_in",
		.description = "Get/Set port ingress (incoming) bandwidth limit in kbps",
		.max = RTL8366RB_BDTH_SW_MAX,
		.set = rtl8366rb_sw_set_port_rate_in,
		.get = rtl8366rb_sw_get_port_rate_in,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "rate_out",
		.description = "Get/Set port egress (outgoing) bandwidth limit in kbps",
		.max = RTL8366RB_BDTH_SW_MAX,
		.set = rtl8366rb_sw_set_port_rate_out,
		.get = rtl8366rb_sw_get_port_rate_out,
	},
};

static struct switch_attr rtl8366rb_vlan[] = {
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
		.max = RTL8366RB_FIDMAX,
		.set = rtl8366_sw_set_vlan_fid,
		.get = rtl8366_sw_get_vlan_fid,
	},
};

static const struct switch_dev_ops rtl8366_ops = {
	.attr_global = {
		.attr = rtl8366rb_globals,
		.n_attr = ARRAY_SIZE(rtl8366rb_globals),
	},
	.attr_port = {
		.attr = rtl8366rb_port,
		.n_attr = ARRAY_SIZE(rtl8366rb_port),
	},
	.attr_vlan = {
		.attr = rtl8366rb_vlan,
		.n_attr = ARRAY_SIZE(rtl8366rb_vlan),
	},

	.get_vlan_ports = rtl8366_sw_get_vlan_ports,
	.set_vlan_ports = rtl8366_sw_set_vlan_ports,
	.get_port_pvid = rtl8366_sw_get_port_pvid,
	.set_port_pvid = rtl8366_sw_set_port_pvid,
	.reset_switch = rtl8366_sw_reset_switch,
	.get_port_link = rtl8366rb_sw_get_port_link,
	.get_port_stats = rtl8366rb_sw_get_port_stats,
};

static int rtl8366rb_switch_init(struct rtl8366_smi *smi)
{
	struct switch_dev *dev = &smi->sw_dev;
	int err;

	dev->name = "RTL8366RB";
	dev->cpu_port = RTL8366RB_PORT_NUM_CPU;
	dev->ports = RTL8366RB_NUM_PORTS;
	dev->vlans = RTL8366RB_NUM_VIDS;
	dev->ops = &rtl8366_ops;
	dev->alias = dev_name(smi->parent);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(smi->parent, "switch registration failed\n");

	return err;
}

static void rtl8366rb_switch_cleanup(struct rtl8366_smi *smi)
{
	unregister_switch(&smi->sw_dev);
}

static int rtl8366rb_mii_read(struct mii_bus *bus, int addr, int reg)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 val = 0;
	int err;

	err = rtl8366rb_read_phy_reg(smi, addr, 0, reg, &val);
	if (err)
		return 0xffff;

	return val;
}

static int rtl8366rb_mii_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 t;
	int err;

	err = rtl8366rb_write_phy_reg(smi, addr, 0, reg, val);
	/* flush write */
	(void) rtl8366rb_read_phy_reg(smi, addr, 0, reg, &t);

	return err;
}

static int rtl8366rb_detect(struct rtl8366_smi *smi)
{
	u32 chip_id = 0;
	u32 chip_ver = 0;
	int ret;

	ret = rtl8366_smi_read_reg(smi, RTL8366RB_CHIP_ID_REG, &chip_id);
	if (ret) {
		dev_err(smi->parent, "unable to read chip id\n");
		return ret;
	}

	switch (chip_id) {
	case RTL8366RB_CHIP_ID_8366:
		break;
	default:
		dev_err(smi->parent, "unknown chip id (%04x)\n", chip_id);
		return -ENODEV;
	}

	ret = rtl8366_smi_read_reg(smi, RTL8366RB_CHIP_VERSION_CTRL_REG,
				   &chip_ver);
	if (ret) {
		dev_err(smi->parent, "unable to read chip version\n");
		return ret;
	}

	dev_info(smi->parent, "RTL%04x ver. %u chip found\n",
		 chip_id, chip_ver & RTL8366RB_CHIP_VERSION_MASK);

	return 0;
}

static struct rtl8366_smi_ops rtl8366rb_smi_ops = {
	.detect		= rtl8366rb_detect,
	.reset_chip	= rtl8366rb_reset_chip,
	.setup		= rtl8366rb_setup,

	.mii_read	= rtl8366rb_mii_read,
	.mii_write	= rtl8366rb_mii_write,

	.get_vlan_mc	= rtl8366rb_get_vlan_mc,
	.set_vlan_mc	= rtl8366rb_set_vlan_mc,
	.get_vlan_4k	= rtl8366rb_get_vlan_4k,
	.set_vlan_4k	= rtl8366rb_set_vlan_4k,
	.get_mc_index	= rtl8366rb_get_mc_index,
	.set_mc_index	= rtl8366rb_set_mc_index,
	.get_mib_counter = rtl8366rb_get_mib_counter,
	.is_vlan_valid	= rtl8366rb_is_vlan_valid,
	.enable_vlan	= rtl8366rb_enable_vlan,
	.enable_vlan4k	= rtl8366rb_enable_vlan4k,
	.enable_port	= rtl8366rb_enable_port,
};

static int rtl8366rb_probe(struct platform_device *pdev)
{
	static int rtl8366_smi_version_printed;
	struct rtl8366_smi *smi;
	int err;

	if (!rtl8366_smi_version_printed++)
		printk(KERN_NOTICE RTL8366RB_DRIVER_DESC
		       " version " RTL8366RB_DRIVER_VER"\n");

	smi = rtl8366_smi_probe(pdev);
	if (!smi)
		return -ENODEV;

	smi->clk_delay = 10;
	smi->cmd_read = 0xa9;
	smi->cmd_write = 0xa8;
	smi->ops = &rtl8366rb_smi_ops;
	smi->cpu_port = RTL8366RB_PORT_NUM_CPU;
	smi->num_ports = RTL8366RB_NUM_PORTS;
	smi->num_vlan_mc = RTL8366RB_NUM_VLANS;
	smi->mib_counters = rtl8366rb_mib_counters;
	smi->num_mib_counters = ARRAY_SIZE(rtl8366rb_mib_counters);

	err = rtl8366_smi_init(smi);
	if (err)
		goto err_free_smi;

	platform_set_drvdata(pdev, smi);

	err = rtl8366rb_switch_init(smi);
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

static int rtl8366rb_remove(struct platform_device *pdev)
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi) {
		rtl8366rb_switch_cleanup(smi);
		platform_set_drvdata(pdev, NULL);
		rtl8366_smi_cleanup(smi);
		kfree(smi);
	}

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id rtl8366rb_match[] = {
	{ .compatible = "realtek,rtl8366rb" },
	{},
};
MODULE_DEVICE_TABLE(of, rtl8366rb_match);
#endif

static struct platform_driver rtl8366rb_driver = {
	.driver = {
		.name		= RTL8366RB_DRIVER_NAME,
		.owner		= THIS_MODULE,
		.of_match_table = of_match_ptr(rtl8366rb_match),
	},
	.probe		= rtl8366rb_probe,
	.remove		= rtl8366rb_remove,
};

static int __init rtl8366rb_module_init(void)
{
	return platform_driver_register(&rtl8366rb_driver);
}
module_init(rtl8366rb_module_init);

static void __exit rtl8366rb_module_exit(void)
{
	platform_driver_unregister(&rtl8366rb_driver);
}
module_exit(rtl8366rb_module_exit);

MODULE_DESCRIPTION(RTL8366RB_DRIVER_DESC);
MODULE_VERSION(RTL8366RB_DRIVER_VER);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Antti Seppälä <a.seppala@gmail.com>");
MODULE_AUTHOR("Roman Yeryomin <roman@advem.lv>");
MODULE_AUTHOR("Colin Leitner <colin.leitner@googlemail.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RTL8366RB_DRIVER_NAME);
