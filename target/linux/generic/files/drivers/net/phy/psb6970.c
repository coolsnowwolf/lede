/*
 * Lantiq PSB6970 (Tantos) Switch driver
 *
 * Copyright (c) 2009,2010 Team Embedded.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 *
 * The switch programming done in this driver follows the 
 * "Ethernet Traffic Separation using VLAN" Application Note as
 * published by Lantiq.
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/switch.h>
#include <linux/phy.h>
#include <linux/version.h>

#define PSB6970_MAX_VLANS		16
#define PSB6970_NUM_PORTS		7
#define PSB6970_DEFAULT_PORT_CPU	6
#define PSB6970_IS_CPU_PORT(x)		((x) > 4)

#define PHYADDR(_reg)		((_reg >> 5) & 0xff), (_reg & 0x1f)

/* --- Identification --- */
#define PSB6970_CI0		0x0100
#define PSB6970_CI0_MASK	0x000f
#define PSB6970_CI1		0x0101
#define PSB6970_CI1_VAL		0x2599
#define PSB6970_CI1_MASK	0xffff

/* --- VLAN filter table --- */
#define PSB6970_VFxL(i)		((i)*2+0x10)	/* VLAN Filter Low */
#define PSB6970_VFxL_VV		(1 << 15)	/* VLAN_Valid */

#define PSB6970_VFxH(i)		((i)*2+0x11)	/* VLAN Filter High */
#define PSB6970_VFxH_TM_SHIFT	7		/* Tagged Member */

/* --- Port registers --- */
#define PSB6970_EC(p)		((p)*0x20+2)	/* Extended Control */
#define PSB6970_EC_IFNTE	(1 << 1)	/* Input Force No Tag Enable */

#define PSB6970_PBVM(p)		((p)*0x20+3)	/* Port Base VLAN Map */
#define PSB6970_PBVM_VMCE	(1 << 8)
#define PSB6970_PBVM_AOVTP	(1 << 9)
#define PSB6970_PBVM_VSD	(1 << 10)
#define PSB6970_PBVM_VC		(1 << 11)	/* VID Check with VID table */
#define PSB6970_PBVM_TBVE	(1 << 13)	/* Tag-Based VLAN enable */

#define PSB6970_DVID(p)		((p)*0x20+4)	/* Default VLAN ID & Priority */

struct psb6970_priv {
	struct switch_dev dev;
	struct phy_device *phy;
	u16 (*read) (struct phy_device* phydev, int reg);
	void (*write) (struct phy_device* phydev, int reg, u16 val);
	struct mutex reg_mutex;

	/* all fields below are cleared on reset */
	bool vlan;
	u16 vlan_id[PSB6970_MAX_VLANS];
	u8 vlan_table[PSB6970_MAX_VLANS];
	u8 vlan_tagged;
	u16 pvid[PSB6970_NUM_PORTS];
};

#define to_psb6970(_dev) container_of(_dev, struct psb6970_priv, dev)

static u16 psb6970_mii_read(struct phy_device *phydev, int reg)
{
	struct mii_bus *bus = phydev->mdio.bus;

	return bus->read(bus, PHYADDR(reg));
}

static void psb6970_mii_write(struct phy_device *phydev, int reg, u16 val)
{
	struct mii_bus *bus = phydev->mdio.bus;

	bus->write(bus, PHYADDR(reg), val);
}

static int
psb6970_set_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	priv->vlan = !!val->value.i;
	return 0;
}

static int
psb6970_get_vlan(struct switch_dev *dev, const struct switch_attr *attr,
		 struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	val->value.i = priv->vlan;
	return 0;
}

static int psb6970_set_pvid(struct switch_dev *dev, int port, int vlan)
{
	struct psb6970_priv *priv = to_psb6970(dev);

	/* make sure no invalid PVIDs get set */
	if (vlan >= dev->vlans)
		return -EINVAL;

	priv->pvid[port] = vlan;
	return 0;
}

static int psb6970_get_pvid(struct switch_dev *dev, int port, int *vlan)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	*vlan = priv->pvid[port];
	return 0;
}

static int
psb6970_set_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	priv->vlan_id[val->port_vlan] = val->value.i;
	return 0;
}

static int
psb6970_get_vid(struct switch_dev *dev, const struct switch_attr *attr,
		struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	val->value.i = priv->vlan_id[val->port_vlan];
	return 0;
}

static struct switch_attr psb6970_globals[] = {
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "enable_vlan",
	 .description = "Enable VLAN mode",
	 .set = psb6970_set_vlan,
	 .get = psb6970_get_vlan,
	 .max = 1},
};

static struct switch_attr psb6970_port[] = {
};

static struct switch_attr psb6970_vlan[] = {
	{
	 .type = SWITCH_TYPE_INT,
	 .name = "vid",
	 .description = "VLAN ID (0-4094)",
	 .set = psb6970_set_vid,
	 .get = psb6970_get_vid,
	 .max = 4094,
	 },
};

static int psb6970_get_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	u8 ports = priv->vlan_table[val->port_vlan];
	int i;

	val->len = 0;
	for (i = 0; i < PSB6970_NUM_PORTS; i++) {
		struct switch_port *p;

		if (!(ports & (1 << i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;
		if (priv->vlan_tagged & (1 << i))
			p->flags = (1 << SWITCH_PORT_FLAG_TAGGED);
		else
			p->flags = 0;
	}
	return 0;
}

static int psb6970_set_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	u8 *vt = &priv->vlan_table[val->port_vlan];
	int i, j;

	*vt = 0;
	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->flags & (1 << SWITCH_PORT_FLAG_TAGGED))
			priv->vlan_tagged |= (1 << p->id);
		else {
			priv->vlan_tagged &= ~(1 << p->id);
			priv->pvid[p->id] = val->port_vlan;

			/* make sure that an untagged port does not
			 * appear in other vlans */
			for (j = 0; j < PSB6970_MAX_VLANS; j++) {
				if (j == val->port_vlan)
					continue;
				priv->vlan_table[j] &= ~(1 << p->id);
			}
		}

		*vt |= 1 << p->id;
	}
	return 0;
}

static int psb6970_hw_apply(struct switch_dev *dev)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	int i, j;

	mutex_lock(&priv->reg_mutex);

	if (priv->vlan) {
		/* into the vlan translation unit */
		for (j = 0; j < PSB6970_MAX_VLANS; j++) {
			u8 vp = priv->vlan_table[j];

			if (vp) {
				priv->write(priv->phy, PSB6970_VFxL(j),
					    PSB6970_VFxL_VV | priv->vlan_id[j]);
				priv->write(priv->phy, PSB6970_VFxH(j),
					    ((vp & priv->
					      vlan_tagged) <<
					     PSB6970_VFxH_TM_SHIFT) | vp);
			} else	/* clear VLAN Valid flag for unused vlans */
				priv->write(priv->phy, PSB6970_VFxL(j), 0);

		}
	}

	/* update the port destination mask registers and tag settings */
	for (i = 0; i < PSB6970_NUM_PORTS; i++) {
		int dvid = 1, pbvm = 0x7f | PSB6970_PBVM_VSD, ec = 0;

		if (priv->vlan) {
			ec = PSB6970_EC_IFNTE;
			dvid = priv->vlan_id[priv->pvid[i]];
			pbvm |= PSB6970_PBVM_TBVE | PSB6970_PBVM_VMCE;

			if ((i << 1) & priv->vlan_tagged)
				pbvm |= PSB6970_PBVM_AOVTP | PSB6970_PBVM_VC;
		}

		priv->write(priv->phy, PSB6970_PBVM(i), pbvm);

		if (!PSB6970_IS_CPU_PORT(i)) {
			priv->write(priv->phy, PSB6970_EC(i), ec);
			priv->write(priv->phy, PSB6970_DVID(i), dvid);
		}
	}

	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int psb6970_reset_switch(struct switch_dev *dev)
{
	struct psb6970_priv *priv = to_psb6970(dev);
	int i;

	mutex_lock(&priv->reg_mutex);

	memset(&priv->vlan, 0, sizeof(struct psb6970_priv) -
	       offsetof(struct psb6970_priv, vlan));

	for (i = 0; i < PSB6970_MAX_VLANS; i++)
		priv->vlan_id[i] = i;

	mutex_unlock(&priv->reg_mutex);

	return psb6970_hw_apply(dev);
}

static const struct switch_dev_ops psb6970_ops = {
	.attr_global = {
			.attr = psb6970_globals,
			.n_attr = ARRAY_SIZE(psb6970_globals),
			},
	.attr_port = {
		      .attr = psb6970_port,
		      .n_attr = ARRAY_SIZE(psb6970_port),
		      },
	.attr_vlan = {
		      .attr = psb6970_vlan,
		      .n_attr = ARRAY_SIZE(psb6970_vlan),
		      },
	.get_port_pvid = psb6970_get_pvid,
	.set_port_pvid = psb6970_set_pvid,
	.get_vlan_ports = psb6970_get_ports,
	.set_vlan_ports = psb6970_set_ports,
	.apply_config = psb6970_hw_apply,
	.reset_switch = psb6970_reset_switch,
};

static int psb6970_config_init(struct phy_device *pdev)
{
	struct psb6970_priv *priv;
	struct net_device *dev = pdev->attached_dev;
	struct switch_dev *swdev;
	int ret;

	priv = kzalloc(sizeof(struct psb6970_priv), GFP_KERNEL);
	if (priv == NULL)
		return -ENOMEM;

	priv->phy = pdev;

	if (pdev->mdio.addr == 0)
		printk(KERN_INFO "%s: psb6970 switch driver attached.\n",
		       pdev->attached_dev->name);

	if (pdev->mdio.addr != 0) {
		kfree(priv);
		return 0;
	}

	linkmode_zero(pdev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, pdev->supported);
	linkmode_copy(pdev->advertising, pdev->supported);

	mutex_init(&priv->reg_mutex);
	priv->read = psb6970_mii_read;
	priv->write = psb6970_mii_write;

	pdev->priv = priv;

	swdev = &priv->dev;
	swdev->cpu_port = PSB6970_DEFAULT_PORT_CPU;
	swdev->ops = &psb6970_ops;

	swdev->name = "Lantiq PSB6970";
	swdev->vlans = PSB6970_MAX_VLANS;
	swdev->ports = PSB6970_NUM_PORTS;

	if ((ret = register_switch(&priv->dev, pdev->attached_dev)) < 0) {
		kfree(priv);
		goto done;
	}

	ret = psb6970_reset_switch(&priv->dev);
	if (ret) {
		kfree(priv);
		goto done;
	}

done:
	return ret;
}

static int psb6970_read_status(struct phy_device *phydev)
{
	phydev->speed = SPEED_100;
	phydev->duplex = DUPLEX_FULL;
	phydev->link = 1;

	phydev->state = PHY_RUNNING;
	netif_carrier_on(phydev->attached_dev);
	phydev->adjust_link(phydev->attached_dev);

	return 0;
}

static int psb6970_config_aneg(struct phy_device *phydev)
{
	return 0;
}

static int psb6970_probe(struct phy_device *pdev)
{
	return 0;
}

static void psb6970_remove(struct phy_device *pdev)
{
	struct psb6970_priv *priv = pdev->priv;

	if (!priv)
		return;

	if (pdev->mdio.addr == 0)
		unregister_switch(&priv->dev);
	kfree(priv);
}

static int psb6970_fixup(struct phy_device *dev)
{
	struct mii_bus *bus = dev->mdio.bus;
	u16 reg;

	/* look for the switch on the bus */
	reg = bus->read(bus, PHYADDR(PSB6970_CI1)) & PSB6970_CI1_MASK;
	if (reg != PSB6970_CI1_VAL)
		return 0;

	dev->phy_id = (reg << 16);
	dev->phy_id |= bus->read(bus, PHYADDR(PSB6970_CI0)) & PSB6970_CI0_MASK;

	return 0;
}

static struct phy_driver psb6970_driver = {
	.name = "Lantiq PSB6970",
	.phy_id = PSB6970_CI1_VAL << 16,
	.phy_id_mask = 0xffff0000,
	.features = PHY_BASIC_FEATURES,
	.probe = psb6970_probe,
	.remove = psb6970_remove,
	.config_init = &psb6970_config_init,
	.config_aneg = &psb6970_config_aneg,
	.read_status = &psb6970_read_status,
};

int __init psb6970_init(void)
{
	phy_register_fixup_for_id(PHY_ANY_ID, psb6970_fixup);
	return phy_driver_register(&psb6970_driver, THIS_MODULE);
}

module_init(psb6970_init);

void __exit psb6970_exit(void)
{
	phy_driver_unregister(&psb6970_driver);
}

module_exit(psb6970_exit);

MODULE_DESCRIPTION("Lantiq PSB6970 Switch");
MODULE_AUTHOR("Ithamar R. Adema <ithamar.adema@team-embedded.nl>");
MODULE_LICENSE("GPL");
