/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/switch.h>

//include from rtl8367c dir
#include  "./rtl8367c/include/rtk_switch.h"
#include  "./rtl8367c/include/vlan.h"
#include  "./rtl8367c/include/stat.h"
#include  "./rtl8367c/include/port.h"

#define RTL8367C_SW_CPU_PORT    6

 //RTL8367C_PHY_PORT_NUM + ext0 + ext1
#define RTL8367C_NUM_PORTS 7
#define RTL8367C_NUM_VIDS  4096

struct rtl8367_priv {
	struct switch_dev	swdev;
	bool			global_vlan_enable;
};

struct rtl8367_mib_counter {
	const char *name;
};

struct rtl8367_vlan_info {
	unsigned short	vid;
	unsigned int	untag;
	unsigned int	member;
	unsigned char		fid;
};

struct rtl8367_priv  rtl8367_priv_data;

unsigned int rtl8367c_port_id[RTL8367C_NUM_PORTS]={0,1,2,3,4,EXT_PORT1,EXT_PORT0};

void (*rtl8367_switch_reset_func)(void)=NULL;

static  struct rtl8367_mib_counter  rtl8367c_mib_counters[] = {
	{"ifInOctets"},
	{"dot3StatsFCSErrors"},
	{"dot3StatsSymbolErrors"},
	{"dot3InPauseFrames"},
	{"dot3ControlInUnknownOpcodes"},
	{"etherStatsFragments"},
	{"etherStatsJabbers"},
	{"ifInUcastPkts"},
	{"etherStatsDropEvents"},
	{"etherStatsOctets"},
	{"etherStatsUndersizePkts"},
	{"etherStatsOversizePkts"},
	{"etherStatsPkts64Octets"},
	{"etherStatsPkts65to127Octets"},
	{"etherStatsPkts128to255Octets"},
	{"etherStatsPkts256to511Octets"},
	{"etherStatsPkts512to1023Octets"},
	{"etherStatsPkts1024toMaxOctets"},
	{"etherStatsMcastPkts"},
	{"etherStatsBcastPkts"},
	{"ifOutOctets"},
	{"dot3StatsSingleCollisionFrames"},
	{"dot3StatsMultipleCollisionFrames"},
	{"dot3StatsDeferredTransmissions"},
	{"dot3StatsLateCollisions"},
	{"etherStatsCollisions"},
	{"dot3StatsExcessiveCollisions"},
	{"dot3OutPauseFrames"},
	{"dot1dBasePortDelayExceededDiscards"},
	{"dot1dTpPortInDiscards"},
	{"ifOutUcastPkts"},
	{"ifOutMulticastPkts"},
	{"ifOutBrocastPkts"},
	{"outOampduPkts"},
	{"inOampduPkts"},
	{"pktgenPkts"},
	{"inMldChecksumError"},
	{"inIgmpChecksumError"},
	{"inMldSpecificQuery"},
	{"inMldGeneralQuery"},
	{"inIgmpSpecificQuery"},
	{"inIgmpGeneralQuery"},
	{"inMldLeaves"},
	{"inIgmpLeaves"},
	{"inIgmpJoinsSuccess"},
	{"inIgmpJoinsFail"},
	{"inMldJoinsSuccess"},
	{"inMldJoinsFail"},
	{"inReportSuppressionDrop"},
	{"inLeaveSuppressionDrop"},
	{"outIgmpReports"},
	{"outIgmpLeaves"},
	{"outIgmpGeneralQuery"},
	{"outIgmpSpecificQuery"},
	{"outMldReports"},
	{"outMldLeaves"},
	{"outMldGeneralQuery"},
	{"outMldSpecificQuery"},
	{"inKnownMulticastPkts"},
	{"ifInMulticastPkts"},
	{"ifInBroadcastPkts"},
	{"ifOutDiscards"}
};

/*rtl8367c  proprietary switch API wrapper */
static inline unsigned int rtl8367c_sw_to_phy_port(int port)
{
	return rtl8367c_port_id[port];
}

static inline unsigned int rtl8367c_portmask_phy_to_sw(rtk_portmask_t phy_portmask)
{
	int i;
	for (i = 0; i < RTL8367C_NUM_PORTS; i++) {
		if(RTK_PORTMASK_IS_PORT_SET(phy_portmask,rtl8367c_sw_to_phy_port(i))) {
			RTK_PORTMASK_PORT_CLEAR(phy_portmask,rtl8367c_sw_to_phy_port(i));
			RTK_PORTMASK_PORT_SET(phy_portmask,i);
		}

	}
	return (unsigned int)phy_portmask.bits[0];
}

static int rtl8367c_reset_mibs(void)
{
	return rtk_stat_global_reset();
}

static int rtl8367c_reset_port_mibs(int port)
{

	return rtk_stat_port_reset(rtl8367c_sw_to_phy_port(port));
}

static int rtl8367c_get_mibs_num(void)
{
	return ARRAY_SIZE(rtl8367c_mib_counters);
}

static const char *rtl8367c_get_mib_name(int idx)
{

	return rtl8367c_mib_counters[idx].name;
}

static int rtl8367c_get_port_mib_counter(int idx, int port, unsigned long long *counter)
{
	return rtk_stat_port_get(rtl8367c_sw_to_phy_port(port), idx, counter);
}

static int rtl8367c_is_vlan_valid(unsigned int vlan)
{
	unsigned max = RTL8367C_NUM_VIDS;

	if (vlan == 0 || vlan >= max)
		return 0;

	return 1;
}

static int rtl8367c_get_vlan( unsigned short vid, struct rtl8367_vlan_info *vlan)
{
	rtk_vlan_cfg_t vlan_cfg;

	memset(vlan, '\0', sizeof(struct rtl8367_vlan_info));

	if (vid >= RTL8367C_NUM_VIDS)
		return -EINVAL;

	if(rtk_vlan_get(vid,&vlan_cfg))
       	return -EINVAL;

	vlan->vid = vid;
	vlan->member = rtl8367c_portmask_phy_to_sw(vlan_cfg.mbr);
	vlan->untag = rtl8367c_portmask_phy_to_sw(vlan_cfg.untag);
	vlan->fid = vlan_cfg.fid_msti;

	return 0;
}

static int rtl8367c_set_vlan( unsigned short vid, u32 mbr, u32 untag, u8 fid)
{
	rtk_vlan_cfg_t vlan_cfg;
	int i;

	memset(&vlan_cfg, 0x00, sizeof(rtk_vlan_cfg_t));

	for (i = 0; i < RTL8367C_NUM_PORTS; i++) {
		if (mbr & (1 << i)) {
			RTK_PORTMASK_PORT_SET(vlan_cfg.mbr, rtl8367c_sw_to_phy_port(i));
			if(untag & (1 << i))
				RTK_PORTMASK_PORT_SET(vlan_cfg.untag, rtl8367c_sw_to_phy_port(i));
		}
	}
	vlan_cfg.fid_msti=fid;
	vlan_cfg.ivl_en = 1;
	return rtk_vlan_set(vid, &vlan_cfg);
}


static int rtl8367c_get_pvid( int port, int *pvid)
{
	u32 prio=0;

	if (port >= RTL8367C_NUM_PORTS)
		return -EINVAL;

	return rtk_vlan_portPvid_get(rtl8367c_sw_to_phy_port(port),pvid,&prio);
}


static int rtl8367c_set_pvid( int port, int pvid)
{
	u32 prio=0;

	if (port >= RTL8367C_NUM_PORTS)
		return -EINVAL;

	return rtk_vlan_portPvid_set(rtl8367c_sw_to_phy_port(port),pvid,prio);
}

static int rtl8367c_get_port_link(int port, int *link, int *speed, int *duplex)
{

	if(rtk_port_phyStatus_get(rtl8367c_sw_to_phy_port(port),(rtk_port_linkStatus_t *)link,
					(rtk_port_speed_t *)speed,(rtk_port_duplex_t *)duplex))
		return -EINVAL;

	return 0;
}

/*common rtl8367 swconfig entry API*/

static int
rtl8367_sw_set_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct rtl8367_priv *priv = container_of(dev, struct rtl8367_priv, swdev);

	priv->global_vlan_enable = val->value.i ;

	return 0;
}

static int
rtl8367_sw_get_vlan_enable(struct switch_dev *dev,
			   const struct switch_attr *attr,
			   struct switch_val *val)
{
	struct rtl8367_priv *priv = container_of(dev, struct rtl8367_priv, swdev);

	val->value.i = priv->global_vlan_enable;

	return 0;
}

static int rtl8367_sw_reset_mibs(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	return rtl8367c_reset_mibs();
}


static int rtl8367_sw_reset_port_mibs(struct switch_dev *dev,
				       const struct switch_attr *attr,
				       struct switch_val *val)
{
	int port;

	port = val->port_vlan;
	if (port >= RTL8367C_NUM_PORTS)
		return -EINVAL;

	return rtl8367c_reset_port_mibs(port);
}

static int rtl8367_sw_get_port_mib(struct switch_dev *dev,
			    const struct switch_attr *attr,
			    struct switch_val *val)
{
	int i, len = 0;
	unsigned long long counter = 0;
	static char mib_buf[4096];

	if (val->port_vlan >= RTL8367C_NUM_PORTS)
		return -EINVAL;

	len += snprintf(mib_buf + len, sizeof(mib_buf) - len,
			"Port %d MIB counters\n",
			val->port_vlan);

	for (i = 0; i <rtl8367c_get_mibs_num(); ++i) {
		len += snprintf(mib_buf + len, sizeof(mib_buf) - len,
				"%-36s: ",rtl8367c_get_mib_name(i));
		if (!rtl8367c_get_port_mib_counter(i, val->port_vlan,
					       &counter))
			len += snprintf(mib_buf + len, sizeof(mib_buf) - len,
					"%llu\n", counter);
		else
			len += snprintf(mib_buf + len, sizeof(mib_buf) - len,
					"%s\n", "N/A");
	}

	val->value.s = mib_buf;
	val->len = len;
	return 0;
}


static int rtl8367_sw_get_vlan_info(struct switch_dev *dev,
			     const struct switch_attr *attr,
			     struct switch_val *val)
{
	int i;
	u32 len = 0;
	struct rtl8367_vlan_info vlan;
	static char vlan_buf[256];
	int err;

	if (!rtl8367c_is_vlan_valid(val->port_vlan))
		return -EINVAL;

	memset(vlan_buf, '\0', sizeof(vlan_buf));

	err = rtl8367c_get_vlan(val->port_vlan, &vlan);
	if (err)
		return err;

	len += snprintf(vlan_buf + len, sizeof(vlan_buf) - len,
			"VLAN %d: Ports: '", vlan.vid);

	for (i = 0; i <RTL8367C_NUM_PORTS; i++) {
		if (!(vlan.member & (1 << i)))
			continue;

		len += snprintf(vlan_buf + len, sizeof(vlan_buf) - len, "%d%s", i,
				(vlan.untag & (1 << i)) ? "" : "t");
	}

	len += snprintf(vlan_buf + len, sizeof(vlan_buf) - len,
			"', members=%04x, untag=%04x, fid=%u",
			vlan.member, vlan.untag, vlan.fid);

	val->value.s = vlan_buf;
	val->len = len;

	return 0;
}


static int rtl8367_sw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct switch_port *port;
	struct rtl8367_vlan_info vlan;
	int i;

	if (!rtl8367c_is_vlan_valid(val->port_vlan))
		return -EINVAL;

	if(rtl8367c_get_vlan(val->port_vlan, &vlan))
		return -EINVAL;

	port = &val->value.ports[0];
	val->len = 0;
	for (i = 0; i <RTL8367C_NUM_PORTS ; i++) {
		if (!(vlan.member & BIT(i)))
			continue;

		port->id = i;
		port->flags = (vlan.untag & BIT(i)) ?
					0 : BIT(SWITCH_PORT_FLAG_TAGGED);
		val->len++;
		port++;
	}
	return 0;
}


static int rtl8367_sw_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct switch_port *port;
	u32 member = 0;
	u32 untag = 0;
	u8 fid=0;
	int err;
	int i;

	if (!rtl8367c_is_vlan_valid(val->port_vlan))
		return -EINVAL;

	port = &val->value.ports[0];
	for (i = 0; i < val->len; i++, port++) {
		int pvid = 0;
		member |= BIT(port->id);

		if (!(port->flags & BIT(SWITCH_PORT_FLAG_TAGGED)))
			untag |= BIT(port->id);

		/*
		 * To ensure that we have a valid MC entry for this VLAN,
		 * initialize the port VLAN ID here.
		 */
		err = rtl8367c_get_pvid(port->id, &pvid);
		if (err < 0)
			return err;
		if (pvid == 0) {
			err = rtl8367c_set_pvid(port->id, val->port_vlan);
			if (err < 0)
				return err;
		}
	}

	//pr_info("[%s] vid=%d , mem=%x,untag=%x,fid=%d \n",__func__,val->port_vlan,member,untag,fid);

	return rtl8367c_set_vlan(val->port_vlan, member, untag, fid);

}


static int rtl8367_sw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	return rtl8367c_get_pvid(port, val);
}


static int rtl8367_sw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	return rtl8367c_set_pvid(port, val);
}


static int rtl8367_sw_reset_switch(struct switch_dev *dev)
{
	if(rtl8367_switch_reset_func)
		(*rtl8367_switch_reset_func)();
	else
		printk("rest switch is not supported\n");

	return 0;
}

static int rtl8367_sw_get_port_link(struct switch_dev *dev, int port,
				    struct switch_port_link *link)
{
	int speed;

	if (port >= RTL8367C_NUM_PORTS)
		return -EINVAL;

	if(rtl8367c_get_port_link(port,(int *)&link->link,(int *)&speed,(int *)&link->duplex))
		return -EINVAL;

	if (!link->link)
		return 0;

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


static struct switch_attr rtl8367_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = rtl8367_sw_set_vlan_enable,
		.get = rtl8367_sw_get_vlan_enable,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = rtl8367_sw_reset_mibs,
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
		//.max = 33,
		.set = NULL,
		.get = rtl8367_sw_get_port_mib,
	},
};

static struct switch_attr rtl8367_vlan[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "info",
		.description = "Get vlan information",
		.max = 1,
		.set = NULL,
		.get = rtl8367_sw_get_vlan_info,
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

	.get_vlan_ports = rtl8367_sw_get_vlan_ports,
	.set_vlan_ports = rtl8367_sw_set_vlan_ports,
	.get_port_pvid = rtl8367_sw_get_port_pvid,
	.set_port_pvid = rtl8367_sw_set_port_pvid,
	.reset_switch = rtl8367_sw_reset_switch,
	.get_port_link = rtl8367_sw_get_port_link,
};

int rtl8367s_swconfig_init(void (*reset_func)(void))
{
	struct rtl8367_priv  *priv = &rtl8367_priv_data;
	struct switch_dev *dev=&priv->swdev;
	int err=0;

	rtl8367_switch_reset_func = reset_func ;

	memset(priv, 0, sizeof(struct rtl8367_priv));
	priv->global_vlan_enable =0;

	dev->name = "RTL8367C";
	dev->cpu_port = RTL8367C_SW_CPU_PORT;
	dev->ports = RTL8367C_NUM_PORTS;
	dev->vlans = RTL8367C_NUM_VIDS;
	dev->ops = &rtl8367_sw_ops;
	dev->alias = "RTL8367C";
	err = register_switch(dev, NULL);

	pr_info("[%s]\n",__func__);

	return err;
}
