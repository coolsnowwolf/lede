/*
 * Platform driver for the Realtek RTL8366S ethernet switch
 *
 * Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2010 Antti Seppälä <a.seppala@gmail.com>
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
#include <linux/version.h>

#include "rtl8366_smi.h"

#define RTL8366S_DRIVER_DESC	"Realtek RTL8366S ethernet switch driver"
#define RTL8366S_DRIVER_VER	"0.2.2"

#define RTL8366S_PHY_NO_MAX	4
#define RTL8366S_PHY_PAGE_MAX	7
#define RTL8366S_PHY_ADDR_MAX	31

/* Switch Global Configuration register */
#define RTL8366S_SGCR				0x0000
#define RTL8366S_SGCR_EN_BC_STORM_CTRL		BIT(0)
#define RTL8366S_SGCR_MAX_LENGTH(_x)		(_x << 4)
#define RTL8366S_SGCR_MAX_LENGTH_MASK		RTL8366S_SGCR_MAX_LENGTH(0x3)
#define RTL8366S_SGCR_MAX_LENGTH_1522		RTL8366S_SGCR_MAX_LENGTH(0x0)
#define RTL8366S_SGCR_MAX_LENGTH_1536		RTL8366S_SGCR_MAX_LENGTH(0x1)
#define RTL8366S_SGCR_MAX_LENGTH_1552		RTL8366S_SGCR_MAX_LENGTH(0x2)
#define RTL8366S_SGCR_MAX_LENGTH_16000		RTL8366S_SGCR_MAX_LENGTH(0x3)
#define RTL8366S_SGCR_EN_VLAN			BIT(13)

/* Port Enable Control register */
#define RTL8366S_PECR				0x0001

/* Green Ethernet Feature (based on GPL_BELKIN_F5D8235-4_v1000 v1.01.24) */
#define RTL8366S_GREEN_ETHERNET_CTRL_REG	0x000a
#define RTL8366S_GREEN_ETHERNET_CTRL_MASK	0x0018
#define RTL8366S_GREEN_ETHERNET_TX_BIT		(1 << 3)
#define RTL8366S_GREEN_ETHERNET_RX_BIT		(1 << 4)

/* Switch Security Control registers */
#define RTL8366S_SSCR0				0x0002
#define RTL8366S_SSCR1				0x0003
#define RTL8366S_SSCR2				0x0004
#define RTL8366S_SSCR2_DROP_UNKNOWN_DA		BIT(0)

#define RTL8366S_RESET_CTRL_REG			0x0100
#define RTL8366S_CHIP_CTRL_RESET_HW		1
#define RTL8366S_CHIP_CTRL_RESET_SW		(1 << 1)

#define RTL8366S_CHIP_VERSION_CTRL_REG		0x0104
#define RTL8366S_CHIP_VERSION_MASK		0xf
#define RTL8366S_CHIP_ID_REG			0x0105
#define RTL8366S_CHIP_ID_8366			0x8366

/* PHY registers control */
#define RTL8366S_PHY_ACCESS_CTRL_REG		0x8028
#define RTL8366S_PHY_ACCESS_DATA_REG		0x8029

#define RTL8366S_PHY_CTRL_READ			1
#define RTL8366S_PHY_CTRL_WRITE			0

#define RTL8366S_PHY_REG_MASK			0x1f
#define RTL8366S_PHY_PAGE_OFFSET		5
#define RTL8366S_PHY_PAGE_MASK			(0x7 << 5)
#define RTL8366S_PHY_NO_OFFSET			9
#define RTL8366S_PHY_NO_MASK			(0x1f << 9)

/* Green Ethernet Feature for PHY ports */
#define RTL8366S_PHY_POWER_SAVING_CTRL_REG	12
#define RTL8366S_PHY_POWER_SAVING_MASK		0x1000

/* LED control registers */
#define RTL8366S_LED_BLINKRATE_REG		0x0420
#define RTL8366S_LED_BLINKRATE_BIT		0
#define RTL8366S_LED_BLINKRATE_MASK		0x0007

#define RTL8366S_LED_CTRL_REG			0x0421
#define RTL8366S_LED_0_1_CTRL_REG		0x0422
#define RTL8366S_LED_2_3_CTRL_REG		0x0423

#define RTL8366S_MIB_COUNT			33
#define RTL8366S_GLOBAL_MIB_COUNT		1
#define RTL8366S_MIB_COUNTER_PORT_OFFSET	0x0040
#define RTL8366S_MIB_COUNTER_BASE		0x1000
#define RTL8366S_MIB_COUNTER_PORT_OFFSET2	0x0008
#define RTL8366S_MIB_COUNTER_BASE2		0x1180
#define RTL8366S_MIB_CTRL_REG			0x11F0
#define RTL8366S_MIB_CTRL_USER_MASK		0x01FF
#define RTL8366S_MIB_CTRL_BUSY_MASK		0x0001
#define RTL8366S_MIB_CTRL_RESET_MASK		0x0002

#define RTL8366S_MIB_CTRL_GLOBAL_RESET_MASK	0x0004
#define RTL8366S_MIB_CTRL_PORT_RESET_BIT	0x0003
#define RTL8366S_MIB_CTRL_PORT_RESET_MASK	0x01FC


#define RTL8366S_PORT_VLAN_CTRL_BASE		0x0058
#define RTL8366S_PORT_VLAN_CTRL_REG(_p)  \
		(RTL8366S_PORT_VLAN_CTRL_BASE + (_p) / 4)
#define RTL8366S_PORT_VLAN_CTRL_MASK		0xf
#define RTL8366S_PORT_VLAN_CTRL_SHIFT(_p)	(4 * ((_p) % 4))


#define RTL8366S_VLAN_TABLE_READ_BASE		0x018B
#define RTL8366S_VLAN_TABLE_WRITE_BASE		0x0185

#define RTL8366S_VLAN_TB_CTRL_REG		0x010F

#define RTL8366S_TABLE_ACCESS_CTRL_REG		0x0180
#define RTL8366S_TABLE_VLAN_READ_CTRL		0x0E01
#define RTL8366S_TABLE_VLAN_WRITE_CTRL		0x0F01

#define RTL8366S_VLAN_MC_BASE(_x)		(0x0016 + (_x) * 2)

#define RTL8366S_VLAN_MEMBERINGRESS_REG		0x0379

#define RTL8366S_PORT_LINK_STATUS_BASE		0x0060
#define RTL8366S_PORT_STATUS_SPEED_MASK		0x0003
#define RTL8366S_PORT_STATUS_DUPLEX_MASK	0x0004
#define RTL8366S_PORT_STATUS_LINK_MASK		0x0010
#define RTL8366S_PORT_STATUS_TXPAUSE_MASK	0x0020
#define RTL8366S_PORT_STATUS_RXPAUSE_MASK	0x0040
#define RTL8366S_PORT_STATUS_AN_MASK		0x0080


#define RTL8366S_PORT_NUM_CPU		5
#define RTL8366S_NUM_PORTS		6
#define RTL8366S_NUM_VLANS		16
#define RTL8366S_NUM_LEDGROUPS		4
#define RTL8366S_NUM_VIDS		4096
#define RTL8366S_PRIORITYMAX		7
#define RTL8366S_FIDMAX			7


#define RTL8366S_PORT_1			(1 << 0) /* In userspace port 0 */
#define RTL8366S_PORT_2			(1 << 1) /* In userspace port 1 */
#define RTL8366S_PORT_3			(1 << 2) /* In userspace port 2 */
#define RTL8366S_PORT_4			(1 << 3) /* In userspace port 3 */

#define RTL8366S_PORT_UNKNOWN		(1 << 4) /* No known connection */
#define RTL8366S_PORT_CPU		(1 << 5) /* CPU port */

#define RTL8366S_PORT_ALL		(RTL8366S_PORT_1 |	\
					 RTL8366S_PORT_2 |	\
					 RTL8366S_PORT_3 |	\
					 RTL8366S_PORT_4 |	\
					 RTL8366S_PORT_UNKNOWN | \
					 RTL8366S_PORT_CPU)

#define RTL8366S_PORT_ALL_BUT_CPU	(RTL8366S_PORT_1 |	\
					 RTL8366S_PORT_2 |	\
					 RTL8366S_PORT_3 |	\
					 RTL8366S_PORT_4 |	\
					 RTL8366S_PORT_UNKNOWN)

#define RTL8366S_PORT_ALL_EXTERNAL	(RTL8366S_PORT_1 |	\
					 RTL8366S_PORT_2 |	\
					 RTL8366S_PORT_3 |	\
					 RTL8366S_PORT_4)

#define RTL8366S_PORT_ALL_INTERNAL	(RTL8366S_PORT_UNKNOWN | \
					 RTL8366S_PORT_CPU)

#define RTL8366S_VLAN_VID_MASK		0xfff
#define RTL8366S_VLAN_PRIORITY_SHIFT	12
#define RTL8366S_VLAN_PRIORITY_MASK	0x7
#define RTL8366S_VLAN_MEMBER_MASK	0x3f
#define RTL8366S_VLAN_UNTAG_SHIFT	6
#define RTL8366S_VLAN_UNTAG_MASK	0x3f
#define RTL8366S_VLAN_FID_SHIFT		12
#define RTL8366S_VLAN_FID_MASK		0x7

#define RTL8366S_MIB_RXB_ID		0	/* IfInOctets */
#define RTL8366S_MIB_TXB_ID		20	/* IfOutOctets */

static struct rtl8366_mib_counter rtl8366s_mib_counters[] = {
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

	/*
	 * The following counters are accessible at a different
	 * base address.
	 */
	{ 1,  0, 2, "Dot1dTpPortInDiscards"			},
	{ 1,  2, 2, "IfOutUcastPkts"				},
	{ 1,  4, 2, "IfOutMulticastPkts"			},
	{ 1,  6, 2, "IfOutBroadcastPkts"			},
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

static int rtl8366s_reset_chip(struct rtl8366_smi *smi)
{
	int timeout = 10;
	u32 data;

	rtl8366_smi_write_reg_noack(smi, RTL8366S_RESET_CTRL_REG,
				    RTL8366S_CHIP_CTRL_RESET_HW);
	do {
		msleep(1);
		if (rtl8366_smi_read_reg(smi, RTL8366S_RESET_CTRL_REG, &data))
			return -EIO;

		if (!(data & RTL8366S_CHIP_CTRL_RESET_HW))
			break;
	} while (--timeout);

	if (!timeout) {
		printk("Timeout waiting for the switch to reset\n");
		return -EIO;
	}

	return 0;
}

static int rtl8366s_read_phy_reg(struct rtl8366_smi *smi,
				 u32 phy_no, u32 page, u32 addr, u32 *data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366S_PHY_NO_MAX)
		return -EINVAL;

	if (page > RTL8366S_PHY_PAGE_MAX)
		return -EINVAL;

	if (addr > RTL8366S_PHY_ADDR_MAX)
		return -EINVAL;

	ret = rtl8366_smi_write_reg(smi, RTL8366S_PHY_ACCESS_CTRL_REG,
				    RTL8366S_PHY_CTRL_READ);
	if (ret)
		return ret;

	reg = 0x8000 | (1 << (phy_no + RTL8366S_PHY_NO_OFFSET)) |
	      ((page << RTL8366S_PHY_PAGE_OFFSET) & RTL8366S_PHY_PAGE_MASK) |
	      (addr & RTL8366S_PHY_REG_MASK);

	ret = rtl8366_smi_write_reg(smi, reg, 0);
	if (ret)
		return ret;

	ret = rtl8366_smi_read_reg(smi, RTL8366S_PHY_ACCESS_DATA_REG, data);
	if (ret)
		return ret;

	return 0;
}

static int rtl8366s_write_phy_reg(struct rtl8366_smi *smi,
				  u32 phy_no, u32 page, u32 addr, u32 data)
{
	u32 reg;
	int ret;

	if (phy_no > RTL8366S_PHY_NO_MAX)
		return -EINVAL;

	if (page > RTL8366S_PHY_PAGE_MAX)
		return -EINVAL;

	if (addr > RTL8366S_PHY_ADDR_MAX)
		return -EINVAL;

	ret = rtl8366_smi_write_reg(smi, RTL8366S_PHY_ACCESS_CTRL_REG,
				    RTL8366S_PHY_CTRL_WRITE);
	if (ret)
		return ret;

	reg = 0x8000 | (1 << (phy_no + RTL8366S_PHY_NO_OFFSET)) |
	      ((page << RTL8366S_PHY_PAGE_OFFSET) & RTL8366S_PHY_PAGE_MASK) |
	      (addr & RTL8366S_PHY_REG_MASK);

	ret = rtl8366_smi_write_reg(smi, reg, data);
	if (ret)
		return ret;

	return 0;
}

static int rtl8366s_set_green_port(struct rtl8366_smi *smi, int port, int enable)
{
	int err;
	u32 phyData;

	if (port >= RTL8366S_NUM_PORTS)
		return -EINVAL;

	err = rtl8366s_read_phy_reg(smi, port, 0, RTL8366S_PHY_POWER_SAVING_CTRL_REG, &phyData);
	if (err)
		return err;

	if (enable)
		phyData |= RTL8366S_PHY_POWER_SAVING_MASK;
	else
		phyData &= ~RTL8366S_PHY_POWER_SAVING_MASK;

	err = rtl8366s_write_phy_reg(smi, port, 0, RTL8366S_PHY_POWER_SAVING_CTRL_REG, phyData);
	if (err)
		return err;

	return 0;
}

static int rtl8366s_set_green(struct rtl8366_smi *smi, int enable)
{
	int err;
	unsigned i;
	u32 data = 0;

	if (!enable) {
		for (i = 0; i <= RTL8366S_PHY_NO_MAX; i++) {
			rtl8366s_set_green_port(smi, i, 0);
		}
	}

	if (enable)
		data = (RTL8366S_GREEN_ETHERNET_TX_BIT | RTL8366S_GREEN_ETHERNET_RX_BIT);

	REG_RMW(smi, RTL8366S_GREEN_ETHERNET_CTRL_REG, RTL8366S_GREEN_ETHERNET_CTRL_MASK, data);

	return 0;
}

static int rtl8366s_setup(struct rtl8366_smi *smi)
{
	struct rtl8366_platform_data *pdata;
	int err;
	unsigned i;
#ifdef CONFIG_OF
	struct device_node *np;
	unsigned num_initvals;
	const __be32 *paddr;
#endif

	pdata = smi->parent->platform_data;
	if (pdata && pdata->num_initvals && pdata->initvals) {
		dev_info(smi->parent, "applying initvals\n");
		for (i = 0; i < pdata->num_initvals; i++)
			REG_WR(smi, pdata->initvals[i].reg,
			       pdata->initvals[i].val);
	}

#ifdef CONFIG_OF
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

	if (of_property_read_bool(np, "realtek,green-ethernet-features")) {
		dev_info(smi->parent, "activating Green Ethernet features\n");

		err = rtl8366s_set_green(smi, 1);
		if (err)
			return err;

		for (i = 0; i <= RTL8366S_PHY_NO_MAX; i++) {
			err = rtl8366s_set_green_port(smi, i, 1);
			if (err)
				return err;
		}
	}
#endif

	/* set maximum packet length to 1536 bytes */
	REG_RMW(smi, RTL8366S_SGCR, RTL8366S_SGCR_MAX_LENGTH_MASK,
		RTL8366S_SGCR_MAX_LENGTH_1536);

	/* enable learning for all ports */
	REG_WR(smi, RTL8366S_SSCR0, 0);

	/* enable auto ageing for all ports */
	REG_WR(smi, RTL8366S_SSCR1, 0);

	/*
	 * discard VLAN tagged packets if the port is not a member of
	 * the VLAN with which the packets is associated.
	 */
	REG_WR(smi, RTL8366S_VLAN_MEMBERINGRESS_REG, RTL8366S_PORT_ALL);

	/* don't drop packets whose DA has not been learned */
	REG_RMW(smi, RTL8366S_SSCR2, RTL8366S_SSCR2_DROP_UNKNOWN_DA, 0);

	return 0;
}

static int rtl8366_get_mib_counter(struct rtl8366_smi *smi, int counter,
				   int port, unsigned long long *val)
{
	int i;
	int err;
	u32 addr, data;
	u64 mibvalue;

	if (port > RTL8366S_NUM_PORTS || counter >= RTL8366S_MIB_COUNT)
		return -EINVAL;

	switch (rtl8366s_mib_counters[counter].base) {
	case 0:
		addr = RTL8366S_MIB_COUNTER_BASE +
		       RTL8366S_MIB_COUNTER_PORT_OFFSET * port;
		break;

	case 1:
		addr = RTL8366S_MIB_COUNTER_BASE2 +
			RTL8366S_MIB_COUNTER_PORT_OFFSET2 * port;
		break;

	default:
		return -EINVAL;
	}

	addr += rtl8366s_mib_counters[counter].offset;

	/*
	 * Writing access counter address first
	 * then ASIC will prepare 64bits counter wait for being retrived
	 */
	data = 0; /* writing data will be discard by ASIC */
	err = rtl8366_smi_write_reg(smi, addr, data);
	if (err)
		return err;

	/* read MIB control register */
	err =  rtl8366_smi_read_reg(smi, RTL8366S_MIB_CTRL_REG, &data);
	if (err)
		return err;

	if (data & RTL8366S_MIB_CTRL_BUSY_MASK)
		return -EBUSY;

	if (data & RTL8366S_MIB_CTRL_RESET_MASK)
		return -EIO;

	mibvalue = 0;
	for (i = rtl8366s_mib_counters[counter].length; i > 0; i--) {
		err = rtl8366_smi_read_reg(smi, addr + (i - 1), &data);
		if (err)
			return err;

		mibvalue = (mibvalue << 16) | (data & 0xFFFF);
	}

	*val = mibvalue;
	return 0;
}

static int rtl8366s_get_vlan_4k(struct rtl8366_smi *smi, u32 vid,
				struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[2];
	int err;
	int i;

	memset(vlan4k, '\0', sizeof(struct rtl8366_vlan_4k));

	if (vid >= RTL8366S_NUM_VIDS)
		return -EINVAL;

	/* write VID */
	err = rtl8366_smi_write_reg(smi, RTL8366S_VLAN_TABLE_WRITE_BASE,
				    vid & RTL8366S_VLAN_VID_MASK);
	if (err)
		return err;

	/* write table access control word */
	err = rtl8366_smi_write_reg(smi, RTL8366S_TABLE_ACCESS_CTRL_REG,
				    RTL8366S_TABLE_VLAN_READ_CTRL);
	if (err)
		return err;

	for (i = 0; i < 2; i++) {
		err = rtl8366_smi_read_reg(smi,
					   RTL8366S_VLAN_TABLE_READ_BASE + i,
					   &data[i]);
		if (err)
			return err;
	}

	vlan4k->vid = vid;
	vlan4k->untag = (data[1] >> RTL8366S_VLAN_UNTAG_SHIFT) &
			RTL8366S_VLAN_UNTAG_MASK;
	vlan4k->member = data[1] & RTL8366S_VLAN_MEMBER_MASK;
	vlan4k->fid = (data[1] >> RTL8366S_VLAN_FID_SHIFT) &
			RTL8366S_VLAN_FID_MASK;

	return 0;
}

static int rtl8366s_set_vlan_4k(struct rtl8366_smi *smi,
				const struct rtl8366_vlan_4k *vlan4k)
{
	u32 data[2];
	int err;
	int i;

	if (vlan4k->vid >= RTL8366S_NUM_VIDS ||
	    vlan4k->member > RTL8366S_VLAN_MEMBER_MASK ||
	    vlan4k->untag > RTL8366S_VLAN_UNTAG_MASK ||
	    vlan4k->fid > RTL8366S_FIDMAX)
		return -EINVAL;

	data[0] = vlan4k->vid & RTL8366S_VLAN_VID_MASK;
	data[1] = (vlan4k->member & RTL8366S_VLAN_MEMBER_MASK) |
		  ((vlan4k->untag & RTL8366S_VLAN_UNTAG_MASK) <<
			RTL8366S_VLAN_UNTAG_SHIFT) |
		  ((vlan4k->fid & RTL8366S_VLAN_FID_MASK) <<
			RTL8366S_VLAN_FID_SHIFT);

	for (i = 0; i < 2; i++) {
		err = rtl8366_smi_write_reg(smi,
					    RTL8366S_VLAN_TABLE_WRITE_BASE + i,
					    data[i]);
		if (err)
			return err;
	}

	/* write table access control word */
	err = rtl8366_smi_write_reg(smi, RTL8366S_TABLE_ACCESS_CTRL_REG,
				    RTL8366S_TABLE_VLAN_WRITE_CTRL);

	return err;
}

static int rtl8366s_get_vlan_mc(struct rtl8366_smi *smi, u32 index,
				struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[2];
	int err;
	int i;

	memset(vlanmc, '\0', sizeof(struct rtl8366_vlan_mc));

	if (index >= RTL8366S_NUM_VLANS)
		return -EINVAL;

	for (i = 0; i < 2; i++) {
		err = rtl8366_smi_read_reg(smi,
					   RTL8366S_VLAN_MC_BASE(index) + i,
					   &data[i]);
		if (err)
			return err;
	}

	vlanmc->vid = data[0] & RTL8366S_VLAN_VID_MASK;
	vlanmc->priority = (data[0] >> RTL8366S_VLAN_PRIORITY_SHIFT) &
			   RTL8366S_VLAN_PRIORITY_MASK;
	vlanmc->untag = (data[1] >> RTL8366S_VLAN_UNTAG_SHIFT) &
			RTL8366S_VLAN_UNTAG_MASK;
	vlanmc->member = data[1] & RTL8366S_VLAN_MEMBER_MASK;
	vlanmc->fid = (data[1] >> RTL8366S_VLAN_FID_SHIFT) &
		      RTL8366S_VLAN_FID_MASK;

	return 0;
}

static int rtl8366s_set_vlan_mc(struct rtl8366_smi *smi, u32 index,
				const struct rtl8366_vlan_mc *vlanmc)
{
	u32 data[2];
	int err;
	int i;

	if (index >= RTL8366S_NUM_VLANS ||
	    vlanmc->vid >= RTL8366S_NUM_VIDS ||
	    vlanmc->priority > RTL8366S_PRIORITYMAX ||
	    vlanmc->member > RTL8366S_VLAN_MEMBER_MASK ||
	    vlanmc->untag > RTL8366S_VLAN_UNTAG_MASK ||
	    vlanmc->fid > RTL8366S_FIDMAX)
		return -EINVAL;

	data[0] = (vlanmc->vid & RTL8366S_VLAN_VID_MASK) |
		  ((vlanmc->priority & RTL8366S_VLAN_PRIORITY_MASK) <<
			RTL8366S_VLAN_PRIORITY_SHIFT);
	data[1] = (vlanmc->member & RTL8366S_VLAN_MEMBER_MASK) |
		  ((vlanmc->untag & RTL8366S_VLAN_UNTAG_MASK) <<
			RTL8366S_VLAN_UNTAG_SHIFT) |
		  ((vlanmc->fid & RTL8366S_VLAN_FID_MASK) <<
			RTL8366S_VLAN_FID_SHIFT);

	for (i = 0; i < 2; i++) {
		err = rtl8366_smi_write_reg(smi,
					    RTL8366S_VLAN_MC_BASE(index) + i,
					    data[i]);
		if (err)
			return err;
	}

	return 0;
}

static int rtl8366s_get_mc_index(struct rtl8366_smi *smi, int port, int *val)
{
	u32 data;
	int err;

	if (port >= RTL8366S_NUM_PORTS)
		return -EINVAL;

	err = rtl8366_smi_read_reg(smi, RTL8366S_PORT_VLAN_CTRL_REG(port),
				   &data);
	if (err)
		return err;

	*val = (data >> RTL8366S_PORT_VLAN_CTRL_SHIFT(port)) &
	       RTL8366S_PORT_VLAN_CTRL_MASK;

	return 0;
}

static int rtl8366s_set_mc_index(struct rtl8366_smi *smi, int port, int index)
{
	if (port >= RTL8366S_NUM_PORTS || index >= RTL8366S_NUM_VLANS)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8366S_PORT_VLAN_CTRL_REG(port),
				RTL8366S_PORT_VLAN_CTRL_MASK <<
					RTL8366S_PORT_VLAN_CTRL_SHIFT(port),
				(index & RTL8366S_PORT_VLAN_CTRL_MASK) <<
					RTL8366S_PORT_VLAN_CTRL_SHIFT(port));
}

static int rtl8366s_enable_vlan(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366S_SGCR, RTL8366S_SGCR_EN_VLAN,
				(enable) ? RTL8366S_SGCR_EN_VLAN : 0);
}

static int rtl8366s_enable_vlan4k(struct rtl8366_smi *smi, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366S_VLAN_TB_CTRL_REG,
				1, (enable) ? 1 : 0);
}

static int rtl8366s_is_vlan_valid(struct rtl8366_smi *smi, unsigned vlan)
{
	unsigned max = RTL8366S_NUM_VLANS;

	if (smi->vlan4k_enabled)
		max = RTL8366S_NUM_VIDS - 1;

	if (vlan == 0 || vlan >= max)
		return 0;

	return 1;
}

static int rtl8366s_enable_port(struct rtl8366_smi *smi, int port, int enable)
{
	return rtl8366_smi_rmwr(smi, RTL8366S_PECR, (1 << port),
				(enable) ? 0 : (1 << port));
}

static int rtl8366s_sw_reset_mibs(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	return rtl8366_smi_rmwr(smi, RTL8366S_MIB_CTRL_REG, 0, (1 << 2));
}

static int rtl8366s_sw_get_blinkrate(struct switch_dev *dev,
				     const struct switch_attr *attr,
				     struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366S_LED_BLINKRATE_REG, &data);

	val->value.i = (data & (RTL8366S_LED_BLINKRATE_MASK));

	return 0;
}

static int rtl8366s_sw_set_blinkrate(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->value.i >= 6)
		return -EINVAL;

	return rtl8366_smi_rmwr(smi, RTL8366S_LED_BLINKRATE_REG,
				RTL8366S_LED_BLINKRATE_MASK,
				val->value.i);
}

static int rtl8366s_sw_get_max_length(struct switch_dev *dev,
					const struct switch_attr *attr,
					struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi, RTL8366S_SGCR, &data);

	val->value.i = ((data & (RTL8366S_SGCR_MAX_LENGTH_MASK)) >> 4);

	return 0;
}

static int rtl8366s_sw_set_max_length(struct switch_dev *dev,
					const struct switch_attr *attr,
					struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	char length_code;

	switch (val->value.i) {
		case 0:
			length_code = RTL8366S_SGCR_MAX_LENGTH_1522;
			break;
		case 1:
			length_code = RTL8366S_SGCR_MAX_LENGTH_1536;
			break;
		case 2:
			length_code = RTL8366S_SGCR_MAX_LENGTH_1552;
			break;
		case 3:
			length_code = RTL8366S_SGCR_MAX_LENGTH_16000;
			break;
		default:
			return -EINVAL;
	}

	return rtl8366_smi_rmwr(smi, RTL8366S_SGCR,
			RTL8366S_SGCR_MAX_LENGTH_MASK,
			length_code);
}

static int rtl8366s_sw_get_learning_enable(struct switch_dev *dev,
					   const struct switch_attr *attr,
					   struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;

	rtl8366_smi_read_reg(smi,RTL8366S_SSCR0, &data);
	val->value.i = !data;

	return 0;
}


static int rtl8366s_sw_set_learning_enable(struct switch_dev *dev,
					   const struct switch_attr *attr,
					   struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 portmask = 0;
	int err = 0;

	if (!val->value.i)
		portmask = RTL8366S_PORT_ALL;

	/* set learning for all ports */
	REG_WR(smi, RTL8366S_SSCR0, portmask);

	/* set auto ageing for all ports */
	REG_WR(smi, RTL8366S_SSCR1, portmask);

	return 0;
}

static int rtl8366s_sw_get_green(struct switch_dev *dev,
			      const struct switch_attr *attr,
			      struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;
	int err;

	err = rtl8366_smi_read_reg(smi, RTL8366S_GREEN_ETHERNET_CTRL_REG, &data);
	if (err)
		return err;

	val->value.i = ((data & (RTL8366S_GREEN_ETHERNET_TX_BIT | RTL8366S_GREEN_ETHERNET_RX_BIT)) != 0) ? 1 : 0;

	return 0;
}

static int rtl8366s_sw_set_green(struct switch_dev *dev,
				 const struct switch_attr *attr,
				 struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	return rtl8366s_set_green(smi, val->value.i);
}

static int rtl8366s_sw_get_port_link(struct switch_dev *dev,
				     int port,
				     struct switch_port_link *link)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;
	u32 speed;

	if (port >= RTL8366S_NUM_PORTS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366S_PORT_LINK_STATUS_BASE + (port / 2),
			     &data);

	if (port % 2)
		data = data >> 8;

	link->link = !!(data & RTL8366S_PORT_STATUS_LINK_MASK);
	if (!link->link)
		return 0;

	link->duplex = !!(data & RTL8366S_PORT_STATUS_DUPLEX_MASK);
	link->rx_flow = !!(data & RTL8366S_PORT_STATUS_RXPAUSE_MASK);
	link->tx_flow = !!(data & RTL8366S_PORT_STATUS_TXPAUSE_MASK);
	link->aneg = !!(data & RTL8366S_PORT_STATUS_AN_MASK);

	speed = (data & RTL8366S_PORT_STATUS_SPEED_MASK);
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

static int rtl8366s_sw_set_port_led(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data;
	u32 mask;
	u32 reg;

	if (val->port_vlan >= RTL8366S_NUM_PORTS ||
	    (1 << val->port_vlan) == RTL8366S_PORT_UNKNOWN)
		return -EINVAL;

	if (val->port_vlan == RTL8366S_PORT_NUM_CPU) {
		reg = RTL8366S_LED_BLINKRATE_REG;
		mask = 0xF << 4;
		data = val->value.i << 4;
	} else {
		reg = RTL8366S_LED_CTRL_REG;
		mask = 0xF << (val->port_vlan * 4),
		data = val->value.i << (val->port_vlan * 4);
	}

	return rtl8366_smi_rmwr(smi, reg, mask, data);
}

static int rtl8366s_sw_get_port_led(struct switch_dev *dev,
				    const struct switch_attr *attr,
				    struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	u32 data = 0;

	if (val->port_vlan >= RTL8366S_NUM_LEDGROUPS)
		return -EINVAL;

	rtl8366_smi_read_reg(smi, RTL8366S_LED_CTRL_REG, &data);
	val->value.i = (data >> (val->port_vlan * 4)) & 0x000F;

	return 0;
}

static int rtl8366s_sw_get_green_port(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	int err;
	u32 phyData;

	if (val->port_vlan >= RTL8366S_NUM_PORTS)
		return -EINVAL;

	err = rtl8366s_read_phy_reg(smi, val->port_vlan, 0, RTL8366S_PHY_POWER_SAVING_CTRL_REG, &phyData);
	if (err)
		return err;

	val->value.i = ((phyData & RTL8366S_PHY_POWER_SAVING_MASK) != 0) ? 1 : 0;

	return 0;
}

static int rtl8366s_sw_set_green_port(struct switch_dev *dev,
				      const struct switch_attr *attr,
				      struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);
	return rtl8366s_set_green_port(smi, val->port_vlan, val->value.i);
}

static int rtl8366s_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	struct rtl8366_smi *smi = sw_to_rtl8366_smi(dev);

	if (val->port_vlan >= RTL8366S_NUM_PORTS)
		return -EINVAL;


	return rtl8366_smi_rmwr(smi, RTL8366S_MIB_CTRL_REG,
				0, (1 << (val->port_vlan + 3)));
}

static int rtl8366s_sw_get_port_stats(struct switch_dev *dev, int port,
                                        struct switch_port_stats *stats)
{
	return (rtl8366_sw_get_port_stats(dev, port, stats,
				RTL8366S_MIB_TXB_ID, RTL8366S_MIB_RXB_ID));
}

static struct switch_attr rtl8366s_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_learning",
		.description = "Enable learning, enable aging",
		.set = rtl8366s_sw_set_learning_enable,
		.get = rtl8366s_sw_get_learning_enable,
		.max = 1,
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
		.set = rtl8366s_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "blinkrate",
		.description = "Get/Set LED blinking rate (0 = 43ms, 1 = 84ms,"
		" 2 = 120ms, 3 = 170ms, 4 = 340ms, 5 = 670ms)",
		.set = rtl8366s_sw_set_blinkrate,
		.get = rtl8366s_sw_get_blinkrate,
		.max = 5
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "max_length",
		.description = "Get/Set the maximum length of valid packets"
		" (0 = 1522, 1 = 1536, 2 = 1552, 3 = 16000 (9216?))",
		.set = rtl8366s_sw_set_max_length,
		.get = rtl8366s_sw_get_max_length,
		.max = 3,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "green_mode",
		.description = "Get/Set the router green feature",
		.set = rtl8366s_sw_set_green,
		.get = rtl8366s_sw_get_green,
		.max = 1,
	},
};

static struct switch_attr rtl8366s_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl8366s_sw_reset_port_mibs,
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
		.set = rtl8366s_sw_set_port_led,
		.get = rtl8366s_sw_get_port_led,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "green_port",
		.description = "Get/Set port green feature (0 - 1)",
		.max = 1,
		.set = rtl8366s_sw_set_green_port,
		.get = rtl8366s_sw_get_green_port,
	},
};

static struct switch_attr rtl8366s_vlan[] = {
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
		.max = RTL8366S_FIDMAX,
		.set = rtl8366_sw_set_vlan_fid,
		.get = rtl8366_sw_get_vlan_fid,
	},
};

static const struct switch_dev_ops rtl8366_ops = {
	.attr_global = {
		.attr = rtl8366s_globals,
		.n_attr = ARRAY_SIZE(rtl8366s_globals),
	},
	.attr_port = {
		.attr = rtl8366s_port,
		.n_attr = ARRAY_SIZE(rtl8366s_port),
	},
	.attr_vlan = {
		.attr = rtl8366s_vlan,
		.n_attr = ARRAY_SIZE(rtl8366s_vlan),
	},

	.get_vlan_ports = rtl8366_sw_get_vlan_ports,
	.set_vlan_ports = rtl8366_sw_set_vlan_ports,
	.get_port_pvid = rtl8366_sw_get_port_pvid,
	.set_port_pvid = rtl8366_sw_set_port_pvid,
	.reset_switch = rtl8366_sw_reset_switch,
	.get_port_link = rtl8366s_sw_get_port_link,
	.get_port_stats = rtl8366s_sw_get_port_stats,
};

static int rtl8366s_switch_init(struct rtl8366_smi *smi)
{
	struct switch_dev *dev = &smi->sw_dev;
	int err;

	dev->name = "RTL8366S";
	dev->cpu_port = RTL8366S_PORT_NUM_CPU;
	dev->ports = RTL8366S_NUM_PORTS;
	dev->vlans = RTL8366S_NUM_VIDS;
	dev->ops = &rtl8366_ops;
	dev->alias = dev_name(smi->parent);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(smi->parent, "switch registration failed\n");

	return err;
}

static void rtl8366s_switch_cleanup(struct rtl8366_smi *smi)
{
	unregister_switch(&smi->sw_dev);
}

static int rtl8366s_mii_read(struct mii_bus *bus, int addr, int reg)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 val = 0;
	int err;

	err = rtl8366s_read_phy_reg(smi, addr, 0, reg, &val);
	if (err)
		return 0xffff;

	return val;
}

static int rtl8366s_mii_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct rtl8366_smi *smi = bus->priv;
	u32 t;
	int err;

	err = rtl8366s_write_phy_reg(smi, addr, 0, reg, val);
	/* flush write */
	(void) rtl8366s_read_phy_reg(smi, addr, 0, reg, &t);

	return err;
}

static int rtl8366s_detect(struct rtl8366_smi *smi)
{
	u32 chip_id = 0;
	u32 chip_ver = 0;
	int ret;

	ret = rtl8366_smi_read_reg(smi, RTL8366S_CHIP_ID_REG, &chip_id);
	if (ret) {
		dev_err(smi->parent, "unable to read chip id\n");
		return ret;
	}

	switch (chip_id) {
	case RTL8366S_CHIP_ID_8366:
		break;
	default:
		dev_err(smi->parent, "unknown chip id (%04x)\n", chip_id);
		return -ENODEV;
	}

	ret = rtl8366_smi_read_reg(smi, RTL8366S_CHIP_VERSION_CTRL_REG,
				   &chip_ver);
	if (ret) {
		dev_err(smi->parent, "unable to read chip version\n");
		return ret;
	}

	dev_info(smi->parent, "RTL%04x ver. %u chip found\n",
		 chip_id, chip_ver & RTL8366S_CHIP_VERSION_MASK);

	return 0;
}

static struct rtl8366_smi_ops rtl8366s_smi_ops = {
	.detect		= rtl8366s_detect,
	.reset_chip	= rtl8366s_reset_chip,
	.setup		= rtl8366s_setup,

	.mii_read	= rtl8366s_mii_read,
	.mii_write	= rtl8366s_mii_write,

	.get_vlan_mc	= rtl8366s_get_vlan_mc,
	.set_vlan_mc	= rtl8366s_set_vlan_mc,
	.get_vlan_4k	= rtl8366s_get_vlan_4k,
	.set_vlan_4k	= rtl8366s_set_vlan_4k,
	.get_mc_index	= rtl8366s_get_mc_index,
	.set_mc_index	= rtl8366s_set_mc_index,
	.get_mib_counter = rtl8366_get_mib_counter,
	.is_vlan_valid	= rtl8366s_is_vlan_valid,
	.enable_vlan	= rtl8366s_enable_vlan,
	.enable_vlan4k	= rtl8366s_enable_vlan4k,
	.enable_port	= rtl8366s_enable_port,
};

static int rtl8366s_probe(struct platform_device *pdev)
{
	static int rtl8366_smi_version_printed;
	struct rtl8366_smi *smi;
	int err;

	if (!rtl8366_smi_version_printed++)
		printk(KERN_NOTICE RTL8366S_DRIVER_DESC
		       " version " RTL8366S_DRIVER_VER"\n");

	smi = rtl8366_smi_probe(pdev);
	if (IS_ERR(smi))
		return PTR_ERR(smi);

	smi->clk_delay = 10;
	smi->cmd_read = 0xa9;
	smi->cmd_write = 0xa8;
	smi->ops = &rtl8366s_smi_ops;
	smi->cpu_port = RTL8366S_PORT_NUM_CPU;
	smi->num_ports = RTL8366S_NUM_PORTS;
	smi->num_vlan_mc = RTL8366S_NUM_VLANS;
	smi->mib_counters = rtl8366s_mib_counters;
	smi->num_mib_counters = ARRAY_SIZE(rtl8366s_mib_counters);

	err = rtl8366_smi_init(smi);
	if (err)
		goto err_free_smi;

	platform_set_drvdata(pdev, smi);

	err = rtl8366s_switch_init(smi);
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
static int rtl8366s_remove(struct platform_device *pdev)
#else
static void rtl8366s_remove(struct platform_device *pdev)
#endif
{
	struct rtl8366_smi *smi = platform_get_drvdata(pdev);

	if (smi) {
		rtl8366s_switch_cleanup(smi);
		platform_set_drvdata(pdev, NULL);
		rtl8366_smi_cleanup(smi);
		kfree(smi);
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,11,0)
	return 0;
#endif
}

#ifdef CONFIG_OF
static const struct of_device_id rtl8366s_match[] = {
	{ .compatible = "realtek,rtl8366s" },
	{},
};
MODULE_DEVICE_TABLE(of, rtl8366s_match);
#endif

static struct platform_driver rtl8366s_driver = {
	.driver = {
		.name		= RTL8366S_DRIVER_NAME,
		.owner		= THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(rtl8366s_match),
#endif
	},
	.probe		= rtl8366s_probe,
	.remove		= rtl8366s_remove,
};

static int __init rtl8366s_module_init(void)
{
	return platform_driver_register(&rtl8366s_driver);
}
module_init(rtl8366s_module_init);

static void __exit rtl8366s_module_exit(void)
{
	platform_driver_unregister(&rtl8366s_driver);
}
module_exit(rtl8366s_module_exit);

MODULE_DESCRIPTION(RTL8366S_DRIVER_DESC);
MODULE_VERSION(RTL8366S_DRIVER_VER);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Antti Seppälä <a.seppala@gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" RTL8366S_DRIVER_NAME);
