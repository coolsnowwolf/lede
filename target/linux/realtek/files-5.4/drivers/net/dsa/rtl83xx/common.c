// SPDX-License-Identifier: GPL-2.0-only

#include <linux/of_mdio.h>
#include <linux/of_platform.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;

extern const struct rtl838x_reg rtl838x_reg;
extern const struct rtl838x_reg rtl839x_reg;
extern const struct rtl838x_reg rtl930x_reg;
extern const struct rtl838x_reg rtl931x_reg;

extern const struct dsa_switch_ops rtl83xx_switch_ops;
extern const struct dsa_switch_ops rtl930x_switch_ops;

DEFINE_MUTEX(smi_lock);

int rtl83xx_port_get_stp_state(struct rtl838x_switch_priv *priv, int port)
{
	u32 msti = 0;
	u32 port_state[4];
	int index, bit;
	int pos = port;
	int n = priv->port_width << 1;

	/* Ports above or equal CPU port can never be configured */
	if (port >= priv->cpu_port)
		return -1;

	mutex_lock(&priv->reg_mutex);

	/* For the RTL839x and following, the bits are left-aligned in the 64/128 bit field */
	if (priv->family_id == RTL8390_FAMILY_ID)
		pos += 12;
	if (priv->family_id == RTL9300_FAMILY_ID)
		pos += 3;
	if (priv->family_id == RTL9310_FAMILY_ID)
		pos += 8;

	index = n - (pos >> 4) - 1;
	bit = (pos << 1) % 32;

	priv->r->stp_get(priv, msti, port_state);

	mutex_unlock(&priv->reg_mutex);

	return (port_state[index] >> bit) & 3;
}

static struct table_reg rtl838x_tbl_regs[] = {
	TBL_DESC(0x6900, 0x6908, 3, 15, 13, 1),		// RTL8380_TBL_L2
	TBL_DESC(0x6914, 0x6918, 18, 14, 12, 1),	// RTL8380_TBL_0
	TBL_DESC(0xA4C8, 0xA4CC, 6, 14, 12, 1),		// RTL8380_TBL_1

	TBL_DESC(0x1180, 0x1184, 3, 16, 14, 0),		// RTL8390_TBL_L2
	TBL_DESC(0x1190, 0x1194, 17, 15, 12, 0),	// RTL8390_TBL_0
	TBL_DESC(0x6B80, 0x6B84, 4, 14, 12, 0),		// RTL8390_TBL_1
	TBL_DESC(0x611C, 0x6120, 9, 8, 6, 0),		// RTL8390_TBL_2

	TBL_DESC(0xB320, 0xB334, 3, 18, 16, 0),		// RTL9300_TBL_L2
	TBL_DESC(0xB340, 0xB344, 19, 16, 12, 0),	// RTL9300_TBL_0
	TBL_DESC(0xB3A0, 0xB3A4, 20, 16, 13, 0),	// RTL9300_TBL_1
	TBL_DESC(0xCE04, 0xCE08, 6, 14, 12, 0),		// RTL9300_TBL_2
	TBL_DESC(0xD600, 0xD604, 30, 7, 6, 0),		// RTL9300_TBL_HSB
	TBL_DESC(0x7880, 0x7884, 22, 9, 8, 0),		// RTL9300_TBL_HSA

	TBL_DESC(0x8500, 0x8508, 8, 19, 15, 0),		// RTL9310_TBL_0
	TBL_DESC(0x40C0, 0x40C4, 22, 16, 14, 0),	// RTL9310_TBL_1
	TBL_DESC(0x8528, 0x852C, 6, 18, 14, 0),		// RTL9310_TBL_2
	TBL_DESC(0x0200, 0x0204, 9, 15, 12, 0),		// RTL9310_TBL_3
	TBL_DESC(0x20dc, 0x20e0, 29, 7, 6, 0),		// RTL9310_TBL_4
	TBL_DESC(0x7e1c, 0x7e20, 53, 8, 6, 0),		// RTL9310_TBL_5
};

void rtl_table_init(void)
{
	int i;

	for (i = 0; i < RTL_TBL_END; i++)
		mutex_init(&rtl838x_tbl_regs[i].lock);
}

/*
 * Request access to table t in table access register r
 * Returns a handle to a lock for that table
 */
struct table_reg *rtl_table_get(rtl838x_tbl_reg_t r, int t)
{
	if (r >= RTL_TBL_END)
		return NULL;

	if (t >= BIT(rtl838x_tbl_regs[r].c_bit-rtl838x_tbl_regs[r].t_bit))
		return NULL;

	mutex_lock(&rtl838x_tbl_regs[r].lock);
	rtl838x_tbl_regs[r].tbl = t;

	return &rtl838x_tbl_regs[r];
}

/*
 * Release a table r, unlock the corresponding lock
 */
void rtl_table_release(struct table_reg *r)
{
	if (!r)
		return;

//	pr_info("Unlocking %08x\n", (u32)r);
	mutex_unlock(&r->lock);
//	pr_info("Unlock done\n");
}

/*
 * Reads table index idx into the data registers of the table
 */
void rtl_table_read(struct table_reg *r, int idx)
{
	u32 cmd = r->rmode ? BIT(r->c_bit) : 0;

	cmd |= BIT(r->c_bit + 1) | (r->tbl << r->t_bit) | (idx & (BIT(r->t_bit) - 1));
	sw_w32(cmd, r->addr);
	do { } while (sw_r32(r->addr) & BIT(r->c_bit + 1));
}

/*
 * Writes the content of the table data registers into the table at index idx
 */
void rtl_table_write(struct table_reg *r, int idx)
{
	u32 cmd = r->rmode ? 0 : BIT(r->c_bit);

	cmd |= BIT(r->c_bit + 1) | (r->tbl << r->t_bit) | (idx & (BIT(r->t_bit) - 1));
	sw_w32(cmd, r->addr);
	do { } while (sw_r32(r->addr) & BIT(r->c_bit + 1));
}

/*
 * Returns the address of the ith data register of table register r
 * the address is relative to the beginning of the Switch-IO block at 0xbb000000
 */
inline u16 rtl_table_data(struct table_reg *r, int i)
{
	if (i >= r->max_data)
		i = r->max_data - 1;
	return r->data + i * 4;
}

inline u32 rtl_table_data_r(struct table_reg *r, int i)
{
	return sw_r32(rtl_table_data(r, i));
}

inline void rtl_table_data_w(struct table_reg *r, u32 v, int i)
{
	sw_w32(v, rtl_table_data(r, i));
}

/* Port register accessor functions for the RTL838x and RTL930X SoCs */
void rtl838x_mask_port_reg(u64 clear, u64 set, int reg)
{
	sw_w32_mask((u32)clear, (u32)set, reg);
}

void rtl838x_set_port_reg(u64 set, int reg)
{
	sw_w32((u32)set, reg);
}

u64 rtl838x_get_port_reg(int reg)
{
	return ((u64) sw_r32(reg));
}

/* Port register accessor functions for the RTL839x and RTL931X SoCs */
void rtl839x_mask_port_reg_be(u64 clear, u64 set, int reg)
{
	sw_w32_mask((u32)(clear >> 32), (u32)(set >> 32), reg);
	sw_w32_mask((u32)(clear & 0xffffffff), (u32)(set & 0xffffffff), reg + 4);
}

u64 rtl839x_get_port_reg_be(int reg)
{
	u64 v = sw_r32(reg);

	v <<= 32;
	v |= sw_r32(reg + 4);
	return v;
}

void rtl839x_set_port_reg_be(u64 set, int reg)
{
	sw_w32(set >> 32, reg);
	sw_w32(set & 0xffffffff, reg + 4);
}

void rtl839x_mask_port_reg_le(u64 clear, u64 set, int reg)
{
	sw_w32_mask((u32)clear, (u32)set, reg);
	sw_w32_mask((u32)(clear >> 32), (u32)(set >> 32), reg + 4);
}

void rtl839x_set_port_reg_le(u64 set, int reg)
{
	sw_w32(set, reg);
	sw_w32(set >> 32, reg + 4);
}

u64 rtl839x_get_port_reg_le(int reg)
{
	u64 v = sw_r32(reg + 4);

	v <<= 32;
	v |= sw_r32(reg);
	return v;
}

int read_phy(u32 port, u32 page, u32 reg, u32 *val)
{
	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		return rtl838x_read_phy(port, page, reg, val);
	case RTL8390_FAMILY_ID:
		return rtl839x_read_phy(port, page, reg, val);
	case RTL9300_FAMILY_ID:
		return rtl930x_read_phy(port, page, reg, val);
	case RTL9310_FAMILY_ID:
		return rtl931x_read_phy(port, page, reg, val);
	}
	return -1;
}

int write_phy(u32 port, u32 page, u32 reg, u32 val)
{
	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		return rtl838x_write_phy(port, page, reg, val);
	case RTL8390_FAMILY_ID:
		return rtl839x_write_phy(port, page, reg, val);
	case RTL9300_FAMILY_ID:
		return rtl930x_write_phy(port, page, reg, val);
	case RTL9310_FAMILY_ID:
		return rtl931x_write_phy(port, page, reg, val);
	}
	return -1;
}

static int __init rtl83xx_mdio_probe(struct rtl838x_switch_priv *priv)
{
	struct device *dev = priv->dev;
	struct device_node *dn, *mii_np = dev->of_node;
	struct mii_bus *bus;
	int ret;
	u32 pn;

	pr_debug("In %s\n", __func__);
	mii_np = of_find_compatible_node(NULL, NULL, "realtek,rtl838x-mdio");
	if (mii_np) {
		pr_debug("Found compatible MDIO node!\n");
	} else {
		dev_err(priv->dev, "no %s child node found", "mdio-bus");
		return -ENODEV;
	}

	priv->mii_bus = of_mdio_find_bus(mii_np);
	if (!priv->mii_bus) {
		pr_debug("Deferring probe of mdio bus\n");
		return -EPROBE_DEFER;
	}
	if (!of_device_is_available(mii_np))
		ret = -ENODEV;

	bus = devm_mdiobus_alloc(priv->ds->dev);
	if (!bus)
		return -ENOMEM;

	bus->name = "rtl838x slave mii";

	/*
	 * Since the NIC driver is loaded first, we can use the mdio rw functions
	 * assigned there.
	 */
	bus->read = priv->mii_bus->read;
	bus->write = priv->mii_bus->write;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s-%d", bus->name, dev->id);

	bus->parent = dev;
	priv->ds->slave_mii_bus = bus;
	priv->ds->slave_mii_bus->priv = priv;

	ret = mdiobus_register(priv->ds->slave_mii_bus);
	if (ret && mii_np) {
		of_node_put(dn);
		return ret;
	}

	dn = mii_np;
	for_each_node_by_name(dn, "ethernet-phy") {
		if (of_property_read_u32(dn, "reg", &pn))
			continue;

		priv->ports[pn].dp = dsa_to_port(priv->ds, pn);

		// Check for the integrated SerDes of the RTL8380M first
		if (of_property_read_bool(dn, "phy-is-integrated")
			&& priv->id == 0x8380 && pn >= 24) {
			pr_debug("----> FÓUND A SERDES\n");
			priv->ports[pn].phy = PHY_RTL838X_SDS;
			continue;
		}

		if (of_property_read_bool(dn, "phy-is-integrated")
			&& !of_property_read_bool(dn, "sfp")) {
			priv->ports[pn].phy = PHY_RTL8218B_INT;
			continue;
		}

		if (!of_property_read_bool(dn, "phy-is-integrated")
			&& of_property_read_bool(dn, "sfp")) {
			priv->ports[pn].phy = PHY_RTL8214FC;
			continue;
		}

		if (!of_property_read_bool(dn, "phy-is-integrated")
			&& !of_property_read_bool(dn, "sfp")) {
			priv->ports[pn].phy = PHY_RTL8218B_EXT;
			continue;
		}
	}

	// TODO: Do this needs to come from the .dts, at least the SerDes number
	if (priv->family_id == RTL9300_FAMILY_ID) {
		priv->ports[24].is2G5 = true;
		priv->ports[25].is2G5 = true;
		priv->ports[24].sds_num = 1;
		priv->ports[24].sds_num = 2;
	}

	/* Disable MAC polling the PHY so that we can start configuration */
	priv->r->set_port_reg_le(0ULL, priv->r->smi_poll_ctrl);

	/* Enable PHY control via SoC */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		/* Enable SerDes NWAY and PHY control via SoC */
		sw_w32_mask(BIT(7), BIT(15), RTL838X_SMI_GLB_CTRL);
	} else {
		/* Disable PHY polling via SoC */
		sw_w32_mask(BIT(7), 0, RTL839X_SMI_GLB_CTRL);
	}

	/* Power on fibre ports and reset them if necessary */
	if (priv->ports[24].phy == PHY_RTL838X_SDS) {
		pr_debug("Powering on fibre ports & reset\n");
		rtl8380_sds_power(24, 1);
		rtl8380_sds_power(26, 1);
	}

	// TODO: Only power on SerDes with external PHYs connected
	if (priv->family_id == RTL9300_FAMILY_ID) {
		pr_info("RTL9300 Powering on SerDes ports\n");
		rtl9300_sds_power(24, 1);
		rtl9300_sds_power(25, 1);
		rtl9300_sds_power(26, 1);
		rtl9300_sds_power(27, 1);
	}

	pr_debug("%s done\n", __func__);
	return 0;
}

static int __init rtl83xx_get_l2aging(struct rtl838x_switch_priv *priv)
{
	int t = sw_r32(priv->r->l2_ctrl_1);

	t &= priv->family_id == RTL8380_FAMILY_ID ? 0x7fffff : 0x1FFFFF;

	if (priv->family_id == RTL8380_FAMILY_ID)
		t = t * 128 / 625; /* Aging time in seconds. 0: L2 aging disabled */
	else
		t = (t * 3) / 5;

	pr_debug("L2 AGING time: %d sec\n", t);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(priv->r->l2_port_aging_out));
	return t;
}

/* Caller must hold priv->reg_mutex */
int rtl83xx_lag_add(struct dsa_switch *ds, int group, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int i;

	pr_info("%s: Adding port %d to LA-group %d\n", __func__, port, group);
	if (group >= priv->n_lags) {
		pr_err("Link Agrregation group too large.\n");
		return -EINVAL;
	}

	if (port >= priv->cpu_port) {
		pr_err("Invalid port number.\n");
		return -EINVAL;
	}

	for (i = 0; i < priv->n_lags; i++) {
		if (priv->lags_port_members[i] & BIT_ULL(i))
			break;
	}
	if (i != priv->n_lags) {
		pr_err("%s: Port already member of LAG: %d\n", __func__, i);
		return -ENOSPC;
	}

	priv->r->mask_port_reg_be(0, BIT_ULL(port), priv->r->trk_mbr_ctr(group));
	priv->lags_port_members[group] |= BIT_ULL(port);

	pr_info("lags_port_members %d now %016llx\n", group, priv->lags_port_members[group]);
	return 0;
}

/* Caller must hold priv->reg_mutex */
int rtl83xx_lag_del(struct dsa_switch *ds, int group, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	pr_info("%s: Removing port %d from LA-group %d\n", __func__, port, group);

	if (group >= priv->n_lags) {
		pr_err("Link Agrregation group too large.\n");
		return -EINVAL;
	}

	if (port >= priv->cpu_port) {
		pr_err("Invalid port number.\n");
		return -EINVAL;
	}


	if (!(priv->lags_port_members[group] & BIT_ULL(port))) {
		pr_err("%s: Port not member of LAG: %d\n", __func__, group
		);
		return -ENOSPC;
	}

	priv->r->mask_port_reg_be(BIT_ULL(port), 0, priv->r->trk_mbr_ctr(group));
	priv->lags_port_members[group] &= ~BIT_ULL(port);

	pr_info("lags_port_members %d now %016llx\n", group, priv->lags_port_members[group]);
	return 0;
}

static int rtl83xx_handle_changeupper(struct rtl838x_switch_priv *priv,
				      struct net_device *ndev,
				      struct netdev_notifier_changeupper_info *info)
{
	struct net_device *upper = info->upper_dev;
	int i, j, err;

	if (!netif_is_lag_master(upper))
		return 0;

	mutex_lock(&priv->reg_mutex);

	for (i = 0; i < priv->n_lags; i++) {
		if ((!priv->lag_devs[i]) || (priv->lag_devs[i] == upper))
			break;
	}
	for (j = 0; j < priv->cpu_port; j++) {
		if (priv->ports[j].dp->slave == ndev)
			break;
	}
	if (j >= priv->cpu_port) {
		err = -EINVAL;
		goto out;
	}

	if (info->linking) {
		if (!priv->lag_devs[i])
			priv->lag_devs[i] = upper;
		err = rtl83xx_lag_add(priv->ds, i, priv->ports[j].dp->index);
		if (err) {
			err = -EINVAL;
			goto out;
		}
	} else {
		if (!priv->lag_devs[i])
			err = -EINVAL;
		err = rtl83xx_lag_del(priv->ds, i, priv->ports[j].dp->index);
		if (err) {
			err = -EINVAL;
			goto out;
		}
		if (!priv->lags_port_members[i])
			priv->lag_devs[i] = NULL;
	}

out:
	mutex_unlock(&priv->reg_mutex);
	return 0;
}

static int rtl83xx_netdevice_event(struct notifier_block *this,
				   unsigned long event, void *ptr)
{
	struct net_device *ndev = netdev_notifier_info_to_dev(ptr);
	struct rtl838x_switch_priv *priv;
	int err;

	pr_debug("In: %s, event: %lu\n", __func__, event);

	if ((event != NETDEV_CHANGEUPPER) && (event != NETDEV_CHANGELOWERSTATE))
		return NOTIFY_DONE;

	priv = container_of(this, struct rtl838x_switch_priv, nb);
	switch (event) {
	case NETDEV_CHANGEUPPER:
		err = rtl83xx_handle_changeupper(priv, ndev, ptr);
		break;
	}

	if (err)
		return err;

	return NOTIFY_DONE;
}

static int __init rtl83xx_sw_probe(struct platform_device *pdev)
{
	int err = 0, i;
	struct rtl838x_switch_priv *priv;
	struct device *dev = &pdev->dev;
	u64 bpdu_mask;

	pr_debug("Probing RTL838X switch device\n");
	if (!pdev->dev.of_node) {
		dev_err(dev, "No DT found\n");
		return -EINVAL;
	}

	// Initialize access to RTL switch tables
	rtl_table_init();

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->ds = dsa_switch_alloc(dev, DSA_MAX_PORTS);

	if (!priv->ds)
		return -ENOMEM;
	priv->ds->dev = dev;
	priv->ds->priv = priv;
	priv->ds->ops = &rtl83xx_switch_ops;
	priv->dev = dev;

	priv->family_id = soc_info.family;
	priv->id = soc_info.id;
	switch(soc_info.family) {
	case RTL8380_FAMILY_ID:
		priv->ds->ops = &rtl83xx_switch_ops;
		priv->cpu_port = RTL838X_CPU_PORT;
		priv->port_mask = 0x1f;
		priv->port_width = 1;
		priv->irq_mask = 0x0FFFFFFF;
		priv->r = &rtl838x_reg;
		priv->ds->num_ports = 29;
		priv->fib_entries = 8192;
		rtl8380_get_version(priv);
		priv->n_lags = 8;
		priv->l2_bucket_size = 4;
		break;
	case RTL8390_FAMILY_ID:
		priv->ds->ops = &rtl83xx_switch_ops;
		priv->cpu_port = RTL839X_CPU_PORT;
		priv->port_mask = 0x3f;
		priv->port_width = 2;
		priv->irq_mask = 0xFFFFFFFFFFFFFULL;
		priv->r = &rtl839x_reg;
		priv->ds->num_ports = 53;
		priv->fib_entries = 16384;
		rtl8390_get_version(priv);
		priv->n_lags = 16;
		priv->l2_bucket_size = 4;
		break;
	case RTL9300_FAMILY_ID:
		priv->ds->ops = &rtl930x_switch_ops;
		priv->cpu_port = RTL930X_CPU_PORT;
		priv->port_mask = 0x1f;
		priv->port_width = 1;
		priv->irq_mask = 0x0FFFFFFF;
		priv->r = &rtl930x_reg;
		priv->ds->num_ports = 29;
		priv->fib_entries = 16384;
		priv->version = RTL8390_VERSION_A;
		priv->n_lags = 16;
		sw_w32(1, RTL930X_ST_CTRL);
		priv->l2_bucket_size = 8;
		break;
	case RTL9310_FAMILY_ID:
		priv->ds->ops = &rtl930x_switch_ops;
		priv->cpu_port = RTL931X_CPU_PORT;
		priv->port_mask = 0x3f;
		priv->port_width = 2;
		priv->irq_mask = 0xFFFFFFFFFFFFFULL;
		priv->r = &rtl931x_reg;
		priv->ds->num_ports = 57;
		priv->fib_entries = 16384;
		priv->version = RTL8390_VERSION_A;
		priv->n_lags = 16;
		priv->l2_bucket_size = 8;
		break;
	}
	pr_debug("Chip version %c\n", priv->version);

	err = rtl83xx_mdio_probe(priv);
	if (err) {
		/* Probing fails the 1st time because of missing ethernet driver
		 * initialization. Use this to disable traffic in case the bootloader left if on
		 */
		return err;
	}
	err = dsa_register_switch(priv->ds);
	if (err) {
		dev_err(dev, "Error registering switch: %d\n", err);
		return err;
	}

	/* Enable link and media change interrupts. Are the SERDES masks needed? */
	sw_w32_mask(0, 3, priv->r->isr_glb_src);

	priv->r->set_port_reg_le(priv->irq_mask, priv->r->isr_port_link_sts_chg);
	priv->r->set_port_reg_le(priv->irq_mask, priv->r->imr_port_link_sts_chg);

	priv->link_state_irq = platform_get_irq(pdev, 0);
	pr_info("LINK state irq: %d\n", priv->link_state_irq);
	switch (priv->family_id) {
	case RTL8380_FAMILY_ID:
		err = request_irq(priv->link_state_irq, rtl838x_switch_irq,
				IRQF_SHARED, "rtl838x-link-state", priv->ds);
		break;
	case RTL8390_FAMILY_ID:
		err = request_irq(priv->link_state_irq, rtl839x_switch_irq,
				IRQF_SHARED, "rtl839x-link-state", priv->ds);
		break;
	case RTL9300_FAMILY_ID:
		err = request_irq(priv->link_state_irq, rtl930x_switch_irq,
				IRQF_SHARED, "rtl930x-link-state", priv->ds);
		break;
	case RTL9310_FAMILY_ID:
		err = request_irq(priv->link_state_irq, rtl931x_switch_irq,
				IRQF_SHARED, "rtl931x-link-state", priv->ds);
		break;
	}
	if (err) {
		dev_err(dev, "Error setting up switch interrupt.\n");
		/* Need to free allocated switch here */
	}

	/* Enable interrupts for switch, on RTL931x, the IRQ is always on globally */
	if (soc_info.family != RTL9310_FAMILY_ID)
		sw_w32(0x1, priv->r->imr_glb);

	rtl83xx_get_l2aging(priv);

	rtl83xx_setup_qos(priv);

	/* Clear all destination ports for mirror groups */
	for (i = 0; i < 4; i++)
		priv->mirror_group_ports[i] = -1;

	priv->nb.notifier_call = rtl83xx_netdevice_event;
		if (register_netdevice_notifier(&priv->nb)) {
			priv->nb.notifier_call = NULL;
			dev_err(dev, "Failed to register LAG netdev notifier\n");
	}

	// Flood BPDUs to all ports including cpu-port
	if (soc_info.family != RTL9300_FAMILY_ID) { // TODO: Port this functionality
		bpdu_mask = soc_info.family == RTL8380_FAMILY_ID ? 0x1FFFFFFF : 0x1FFFFFFFFFFFFF;
		priv->r->set_port_reg_be(bpdu_mask, priv->r->rma_bpdu_fld_pmask);

		// TRAP 802.1X frames (EAPOL) to the CPU-Port, bypass STP and VLANs
		sw_w32(7, priv->r->spcl_trap_eapol_ctrl);

		rtl838x_dbgfs_init(priv);
	}

	return err;
}

static int rtl83xx_sw_remove(struct platform_device *pdev)
{
	// TODO:
	pr_debug("Removing platform driver for rtl83xx-sw\n");
	return 0;
}

static const struct of_device_id rtl83xx_switch_of_ids[] = {
	{ .compatible = "realtek,rtl83xx-switch"},
	{ /* sentinel */ }
};


MODULE_DEVICE_TABLE(of, rtl83xx_switch_of_ids);

static struct platform_driver rtl83xx_switch_driver = {
	.probe = rtl83xx_sw_probe,
	.remove = rtl83xx_sw_remove,
	.driver = {
		.name = "rtl83xx-switch",
		.pm = NULL,
		.of_match_table = rtl83xx_switch_of_ids,
	},
};

module_platform_driver(rtl83xx_switch_driver);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL83XX SoC Switch Driver");
MODULE_LICENSE("GPL");
