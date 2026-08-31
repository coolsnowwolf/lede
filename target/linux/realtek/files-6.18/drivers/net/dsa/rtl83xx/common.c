// SPDX-License-Identifier: GPL-2.0-only

#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <net/arp.h>
#include <net/nexthop.h>
#include <net/neighbour.h>
#include <net/netevent.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/inetdevice.h>
#include <linux/platform_device.h>
#include <linux/rhashtable.h>
#include <linux/of_net.h>
#include <asm/mach-rtl-otto/mach-rtl-otto.h>

#include "l3.h"
#include "rtl-otto.h"

int rtldsa_port_get_stp_state(struct rtl838x_switch_priv *priv, int port)
{
	u32 msti = 0;
	int state;

	if (port >= priv->r->cpu_port)
		return -EINVAL;

	mutex_lock(&priv->reg_mutex);
	state = priv->r->stp_get(priv, msti, port);
	mutex_unlock(&priv->reg_mutex);

	return state;
}

static struct table_reg rtl838x_tbl_regs[] = {
	TBL_DESC(0x6900, 0x6908, 3, 15, 13, 1),		/* RTL8380_TBL_L2 */
	TBL_DESC(0x6914, 0x6918, 18, 14, 12, 1),	/* RTL8380_TBL_0 */
	TBL_DESC(0xA4C8, 0xA4CC, 6, 14, 12, 1),		/* RTL8380_TBL_1 */

	TBL_DESC(0x1180, 0x1184, 3, 16, 14, 0),		/* RTL8390_TBL_L2 */
	TBL_DESC(0x1190, 0x1194, 17, 15, 12, 0),	/* RTL8390_TBL_0 */
	TBL_DESC(0x6B80, 0x6B84, 4, 14, 12, 0),		/* RTL8390_TBL_1 */
	TBL_DESC(0x611C, 0x6120, 9, 8, 6, 0),		/* RTL8390_TBL_2 */

	TBL_DESC(0xB320, 0xB334, 3, 18, 16, 0),		/* RTL9300_TBL_L2 */
	TBL_DESC(0xB340, 0xB344, 19, 16, 12, 0),	/* RTL9300_TBL_0 */
	TBL_DESC(0xB3A0, 0xB3A4, 20, 16, 13, 0),	/* RTL9300_TBL_1 */
	TBL_DESC(0xCE04, 0xCE08, 6, 14, 12, 0),		/* RTL9300_TBL_2 */
	TBL_DESC(0xD600, 0xD604, 30, 7, 6, 0),		/* RTL9300_TBL_HSB */
	TBL_DESC(0x7880, 0x7884, 22, 9, 8, 0),		/* RTL9300_TBL_HSA */

	TBL_DESC(0x8500, 0x8508, 8, 19, 15, 0),		/* RTL9310_TBL_0 */
	TBL_DESC(0x40C0, 0x40C4, 22, 16, 14, 0),	/* RTL9310_TBL_1 */
	TBL_DESC(0x8528, 0x852C, 6, 18, 14, 0),		/* RTL9310_TBL_2 */
	TBL_DESC(0x0200, 0x0204, 9, 15, 12, 0),		/* RTL9310_TBL_3 */
	TBL_DESC(0x20dc, 0x20e0, 29, 7, 6, 0),		/* RTL9310_TBL_4 */
	TBL_DESC(0x7e1c, 0x7e20, 53, 8, 6, 0),		/* RTL9310_TBL_5 */
};

void rtl_table_init(void)
{
	for (int i = 0; i < RTL_TBL_END; i++)
		mutex_init(&rtl838x_tbl_regs[i].lock);
}

/* Request access to table t in table access register r
 * Returns a handle to a lock for that table
 */
struct table_reg *rtl_table_get(rtl838x_tbl_reg_t r, int t)
{
	if (r >= RTL_TBL_END)
		return NULL;

	if (t >= BIT(rtl838x_tbl_regs[r].c_bit - rtl838x_tbl_regs[r].t_bit))
		return NULL;

	mutex_lock(&rtl838x_tbl_regs[r].lock);
	rtl838x_tbl_regs[r].tbl = t;

	return &rtl838x_tbl_regs[r];
}

/* Release a table r, unlock the corresponding lock */
void rtl_table_release(struct table_reg *r)
{
	if (!r)
		return;

/*	pr_info("Unlocking %08x\n", (u32)r); */
	mutex_unlock(&r->lock);
/*	pr_info("Unlock done\n"); */
}

static int rtl_table_exec(struct table_reg *r, bool is_write, int idx)
{
	int ret = 0;
	u32 cmd, val;

	/* Read/write bit has inverted meaning on RTL838x */
	if (r->rmode)
		cmd = is_write ? 0 : BIT(r->c_bit);
	else
		cmd = is_write ? BIT(r->c_bit) : 0;

	cmd |= BIT(r->c_bit + 1); /* Execute bit */
	cmd |= r->tbl << r->t_bit; /* Table type */
	cmd |= idx & (BIT(r->t_bit) - 1); /* Index */

	sw_w32(cmd, r->addr);

	ret = readx_poll_timeout(sw_r32, r->addr, val,
				 !(val & BIT(r->c_bit + 1)), 20, 10000);
	if (ret)
		pr_err("%s: timeout\n", __func__);

	return ret;
}

/* Reads table index idx into the data registers of the table */
int rtl_table_read(struct table_reg *r, int idx)
{
	return rtl_table_exec(r, false, idx);
}

/* Writes the content of the table data registers into the table at index idx */
int rtl_table_write(struct table_reg *r, int idx)
{
	return rtl_table_exec(r, true, idx);
}

/* Returns the address of the ith data register of table register r
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
	return ((u64)sw_r32(reg));
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

static bool rtldsa_phy_load_deferred(struct phy_device *phydev)
{
	struct device *d = &phydev->mdio.dev;

	if (d->driver)
		return false;

	return driver_deferred_probe_check_state(d) == -EPROBE_DEFER;
}

static bool rtldsa_phys_load_deferred(void)
{
	struct device_node *phy_node;
	struct phy_device *phydev;
	struct device_node *dn;
	bool deferred;

	for_each_node_by_name(dn, "port") {
		if (!of_device_is_available(dn))
			continue;

		phy_node = of_parse_phandle(dn, "phy-handle", 0);
		if (!phy_node)
			continue;

		phydev = of_phy_find_device(phy_node);
		of_node_put(phy_node);
		if (!phydev)
			continue;

		deferred = rtldsa_phy_load_deferred(phydev);
		put_device(&phydev->mdio.dev);

		if (deferred) {
			of_node_put(dn);
			return true;
		}
	}

	return false;
}

static int rtldsa_mdio_loaded(void)
{
	struct device_node *dn;

	/* Check if all buses of the Realtek MDIO controller are registered. */
	dn = of_find_compatible_node(NULL, NULL, "realtek,otto-mdio");
	if (!of_device_is_available(dn)) {
		of_node_put(dn);
		return -ENODEV;
	}

	for_each_child_of_node_scoped(dn, bn) {
		struct mii_bus *bus = of_mdio_find_bus(bn);

		if (!bus) {
			of_node_put(dn);
			return -EPROBE_DEFER;
		}
		put_device(&bus->dev);
	}
	of_node_put(dn);

	return 0;
}

static int rtldsa_ports_probe(struct rtl838x_switch_priv *priv)
{
	struct device_node *dn, *phy_node, *led_node;
	u32 pn;

	dn = of_find_compatible_node(NULL, NULL, "realtek,otto-switch");
	if (!dn) {
		dev_err(priv->dev, "No RTL switch node in DTS\n");
		return -ENODEV;
	}

	led_node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300-leds");

	for_each_node_by_name(dn, "port") {
		u32 led_set;
		char led_set_str[16] = {0};

		if (!of_device_is_available(dn))
			continue;

		if (of_property_read_u32(dn, "reg", &pn))
			continue;

		phy_node = of_parse_phandle(dn, "phy-handle", 0);
		priv->ports[pn].has_pcs = fwnode_property_present(of_fwnode_handle(dn),
								  "pcs-handle");
		if (pn != priv->r->cpu_port && !phy_node && !priv->ports[pn].has_pcs) {
			dev_err(priv->dev, "Port node %d has neither pcs-handle nor phy-handle\n", pn);
			continue;
		}

		priv->ports[pn].leds_on_this_port = 0;
		if (led_node) {
			if (of_property_read_u32(dn, "led-set", &led_set))
				led_set = 0;
			priv->ports[pn].led_set = led_set;
			sprintf(led_set_str, "led_set%d", led_set);
			priv->ports[pn].leds_on_this_port = of_property_count_u32_elems(led_node, led_set_str);
			if (priv->ports[pn].leds_on_this_port > 4) {
				of_node_put(dn);
				dev_err(priv->dev, "led_set %d for port %d configuration is invalid\n", led_set, pn);
				return -ENODEV;
			}
		}

		priv->ports[pn].phy = !!phy_node;
	}

	return 0;
}

static int rtl83xx_get_l2aging(struct rtl838x_switch_priv *priv)
{
	int t = sw_r32(priv->r->l2_ctrl_1);

	/* RTL838x uses a high-resolution 23-bit AGE_UNIT where one unit is
	 * 204.8 ms. RTL839x and RTL93xx use a 21-bit AGE_UNIT where one unit
	 * is 600 ms. An AGE_UNIT value of 0 disables dynamic address aging.
	 */
	if (priv->r->high_res_l2_age) {
		t &= GENMASK(22, 0);
		t = t * 128 / 625;
	} else {
		t &= GENMASK(20, 0);
		t = t * 3 / 5;
	}

	pr_debug("L2 AGING time: %d sec\n", t);
	pr_debug("Dynamic aging for ports: %x\n", sw_r32(priv->r->l2_port_aging_out));

	return t;
}

/* Caller must hold priv->reg_mutex */
int rtl83xx_lag_add(struct dsa_switch *ds, int group, int port, struct netdev_lag_upper_info *info)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int ret;
	int i;

	for (i = 0; i < priv->ds->num_lag_ids; i++) {
		if (priv->lags_port_members[i] & BIT_ULL(port))
			break;
	}
	if (i != priv->ds->num_lag_ids) {
		pr_err("%s: Port %d already member of LAG %d.\n", __func__, port, i);
		return -ENOSPC;
	}

	if (priv->r->lag_setup_algomask) {
		ret = priv->r->lag_setup_algomask(priv, group, info);
		if (ret)
			return ret;
	}

	ret = priv->r->lag_set_port_members(priv, group,
					    priv->lags_port_members[group] | BIT_ULL(port), info);
	if (ret)
		return ret;

	pr_info("%s: Added port %d to LAG %d. Members now %016llx.\n",
		__func__, port, group, priv->lags_port_members[group]);

	return 0;
}

/* Caller must hold priv->reg_mutex */
int rtl83xx_lag_del(struct dsa_switch *ds, int group, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int ret;

	if (group >= priv->ds->num_lag_ids) {
		pr_err("%s: LAG %d invalid.\n", __func__, group);
		return -EINVAL;
	}

	if (!(priv->lags_port_members[group] & BIT_ULL(port))) {
		pr_err("%s: Port %d not member of LAG %d.\n", __func__, port, group);
		return -ENOSPC;
	}

	/* Don't touch hash mask bits, as only the port might be removed from
	 * the LAG group. This means the lag group stays valid with existing
	 * mask algo bits. If there are no lag members left, then
	 * rtl83xx_lag_add will reconfigure hash mask when new LAG group is
	 * created.
	 */
	ret = priv->r->lag_set_port_members(priv, group,
					    priv->lags_port_members[group] & ~BIT_ULL(port),
					    NULL);
	if (ret)
		return ret;

	pr_info("%s: Removed port %d from LAG %d. Members now %016llx.\n",
		__func__, port, group, priv->lags_port_members[group]);

	return 0;
}

int rtldsa_93xx_lag_set_distribution_algorithm(struct rtl838x_switch_priv *priv,
					       int group, int algoidx, u32 algomsk)
{
	bool is_l3 = false;
	u32 newmask = 0;

	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SIP_BIT) {
		is_l3 = true;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_DIP_BIT) {
		is_l3 = true;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT;
	}
	if (algomsk & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT) {
		is_l3 = true;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT;
	}

	if (algomsk & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT) {
		is_l3 = true;
		newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DST_L4PORT_BIT;
	}

	if (is_l3) {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SPA_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_VLAN_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L3_VLAN_BIT;
	} else {
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_SPA_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT;
		if (algomsk & TRUNK_DISTRIBUTION_ALGO_VLAN_BIT)
			newmask |= TRUNK_DISTRIBUTION_ALGO_L2_VLAN_BIT;
	}

	sw_w32(newmask, priv->r->trk_hash_ctrl + (algoidx << 2));

	return 0;
}

int rtldsa_83xx_lag_setup_algomask(struct rtl838x_switch_priv *priv, int group,
				   struct netdev_lag_upper_info *info)
{
	u32 algomsk = 0;
	u32 algoidx = 0;

	switch (info->hash_type) {
	case NETDEV_LAG_HASH_L2:
		algomsk |= TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;
		algomsk |= TRUNK_DISTRIBUTION_ALGO_SMAC_BIT;
		break;
	case NETDEV_LAG_HASH_L23:
		algomsk |= TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;
		algomsk |= TRUNK_DISTRIBUTION_ALGO_SMAC_BIT;
		algomsk |= TRUNK_DISTRIBUTION_ALGO_SIP_BIT; /* source ip */
		algomsk |= TRUNK_DISTRIBUTION_ALGO_DIP_BIT; /* dest ip */
		algoidx = 1;
		break;
	case NETDEV_LAG_HASH_L34:
		algomsk |= TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT; /* sport */
		algomsk |= TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT; /* dport */
		algomsk |= TRUNK_DISTRIBUTION_ALGO_SIP_BIT; /* source ip */
		algomsk |= TRUNK_DISTRIBUTION_ALGO_DIP_BIT; /* dest ip */
		algoidx = 2;
		break;
	default:
		algomsk |= TRUNK_DISTRIBUTION_ALGO_MASKALL;
	}

	return priv->r->lag_set_distribution_algorithm(priv, group, algoidx, algomsk);
}

static int rtldsa_93xx_lag_set_group2ports(struct rtl838x_switch_priv *priv, int group,
					   struct netdev_lag_upper_info *info)
{
	DECLARE_BITMAP(ports, ARRAY_SIZE(priv->ports));
	struct rtldsa_93xx_lag_entry e;
	unsigned int table_pos = 0;
	u8 num_of_lag_ports = 0;
	u8 group_ports[8];
	u32 data[3];
	int i;

	/* Read lag table using Table control register 2 */
	struct table_reg *r = priv->r->lag_table();

	rtl_table_read(r, group);

	bitmap_clear(ports, 0, ARRAY_SIZE(priv->ports));
	bitmap_from_arr64(ports, &priv->lags_port_members[group],
			  ARRAY_SIZE(priv->ports));

	for (i = 0; i < 3; i++)
		data[i] = sw_r32(rtl_table_data(r, i));

	priv->r->lag_fill_data(data, &e);

	num_of_lag_ports = bitmap_weight(ports, ARRAY_SIZE(priv->ports));
	if (num_of_lag_ports > ARRAY_SIZE(group_ports)) {
		pr_err("%s: Number of LAG ports too high: %u", __func__,
		       num_of_lag_ports);

		return -ENOSPC;
	}

	memset(group_ports, 0x3f, sizeof(group_ports));

	table_pos = 0;
	for_each_set_bit(i, ports, ARRAY_SIZE(priv->ports)) {
		if (!priv->ports[i].dp->lag_tx_enabled)
			continue;

		group_ports[table_pos] = i;
		table_pos++;
	}

	/* Remove tx disabled ports */
	num_of_lag_ports = table_pos;

	e.trk_dev0 = 0;
	e.trk_port0 = group_ports[0];
	e.trk_dev1 = 0;
	e.trk_port1 = group_ports[1];
	e.trk_dev2 = 0;
	e.trk_port2 = group_ports[2];
	e.trk_dev3 = 0;
	e.trk_port3 = group_ports[3];
	e.trk_dev4 = 0;
	e.trk_port4 = group_ports[4];
	e.trk_dev5 = 0;
	e.trk_port5 = group_ports[5];
	e.trk_dev6 = 0;
	e.trk_port6 = group_ports[6];
	e.trk_dev7 = 0;
	e.trk_port7 = group_ports[7];

	e.num_tx_candi = num_of_lag_ports;

	/* set hash_mask_idx to 0 if we are deleting lag group */
	if (info) {
		if (info->hash_type == NETDEV_LAG_HASH_L2) {
			e.l2_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L2;
			e.ip4_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L2;
			e.ip6_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L2;
		} else if (info->hash_type == NETDEV_LAG_HASH_L23) {
			e.l2_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L23;
			e.ip4_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L23;
			e.ip6_hash_mask_idx = RTL93XX_HASH_MASK_INDEX_L23;
		} else {
			return -EOPNOTSUPP;
		}
	}

	priv->r->lag_write_data(data, &e);

	for (i = 0; i < 3; i++)
		sw_w32(data[i], rtl_table_data(r, i));
	rtl_table_write(r, group);
	rtl_table_release(r);

	return 0;
}

static inline void rtldsa_93xx_lag_set_local_group2ports(struct rtl838x_switch_priv *priv, int group,
						  u64 ports)
{
	priv->r->set_port_reg_be(ports, priv->r->trk_mbr_ctr(group));
}

int rtldsa_93xx_lag_set_port_members(struct rtl838x_switch_priv *priv, int group,
				     u64 members, struct netdev_lag_upper_info *info)
{
	DECLARE_BITMAP(affected_members, ARRAY_SIZE(priv->ports));
	bool valid_group;
	u64 old_members;
	u64 affected;
	size_t port;
	int ret;

	/* calculate modifications of the LAG group */
	old_members = priv->lags_port_members[group];
	priv->lags_port_members[group] = members;

	affected = old_members | priv->lags_port_members[group];

	bitmap_clear(affected_members, 0, ARRAY_SIZE(priv->ports));
	bitmap_from_arr64(affected_members, &affected, BITS_PER_TYPE(affected));

	valid_group = __sw_hweight64(priv->lags_port_members[group]);

	/* apply global group and port settings */
	ret = rtldsa_93xx_lag_set_group2ports(priv, group, info);
	if (ret)
		return ret;

	for_each_set_bit(port, affected_members, ARRAY_SIZE(priv->ports)) {
		bool valid = priv->lags_port_members[group] & BIT_ULL(port);

		priv->r->lag_set_port2group(group, port, valid);
	}

	/* apply local group and port settings */
	priv->r->lag_set_local_group_id(group, group, valid_group);
	rtldsa_93xx_lag_set_local_group2ports(priv, group, priv->lags_port_members[group]);

	for_each_set_bit(port, affected_members, ARRAY_SIZE(priv->ports)) {
		bool valid = priv->lags_port_members[group] & BIT_ULL(port);

		priv->r->lag_set_local_port2group(group, port, valid);
	}

	/* write lag table (and maybe additional information) to SRAM */
	priv->r->lag_sync_tables();

	return 0;
}

// Currently Unused
// /* Allocate a 64 bit octet counter located in the LOG HW table */
// static int rtl83xx_octet_cntr_alloc(struct rtl838x_switch_priv *priv)
// {
// 	int idx;

// 	mutex_lock(&priv->reg_mutex);

// 	idx = find_first_zero_bit(priv->octet_cntr_use_bm, MAX_COUNTERS);
// 	if (idx >= priv->r->n_counters) {
// 		mutex_unlock(&priv->reg_mutex);
// 		return -1;
// 	}

// 	set_bit(idx, priv->octet_cntr_use_bm);
// 	mutex_unlock(&priv->reg_mutex);

// 	return idx;
// }

/* Allocate a 32-bit packet counter
 * 2 32-bit packet counters share the location of a 64-bit octet counter
 * Initially there are no free packet counters and 2 new ones need to be freed
 * by allocating the corresponding octet counter
 */
int rtl83xx_packet_cntr_alloc(struct rtl838x_switch_priv *priv)
{
	int idx, j;

	mutex_lock(&priv->reg_mutex);

	/* Because initially no packet counters are free, the logic is reversed:
	 * a 0-bit means the counter is already allocated (for octets)
	 */
	idx = find_first_bit(priv->packet_cntr_use_bm, MAX_COUNTERS * 2);
	if (idx >= priv->r->n_counters * 2) {
		j = find_first_zero_bit(priv->octet_cntr_use_bm, MAX_COUNTERS);
		if (j >= priv->r->n_counters) {
			mutex_unlock(&priv->reg_mutex);
			return -1;
		}
		set_bit(j, priv->octet_cntr_use_bm);
		idx = j * 2;
		set_bit(j * 2 + 1, priv->packet_cntr_use_bm);

	} else {
		clear_bit(idx, priv->packet_cntr_use_bm);
	}

	mutex_unlock(&priv->reg_mutex);

	return idx;
}

/* Add an L2 nexthop entry for the L3 routing system / PIE forwarding in the SoC
 * Use VID and MAC in rtl838x_l2_entry to identify either a free slot in the L2 hash table
 * or mark an existing entry as a nexthop by setting it's nexthop bit
 * Called from the L3 layer
 * The index in the L2 hash table is filled into nh->l2_id;
 */
int rtl83xx_l2_nexthop_add(struct rtl838x_switch_priv *priv, struct otto_l3_nexthop *nh)
{
	struct rtl838x_l2_entry e;
	u64 seed = priv->r->l2_hash_seed(nh->mac, nh->rvid);
	u32 key = priv->r->l2_hash_key(priv, seed);
	int idx = -1;
	u64 entry;

	pr_debug("%s searching for %08llx vid %d with key %d, seed: %016llx\n",
		 __func__, nh->mac, nh->rvid, key, seed);

	e.type = L2_UNICAST;
	u64_to_ether_addr(nh->mac, &e.mac[0]);
	e.port = nh->port;

	/* Loop over all entries in the hash-bucket and over the second block on 93xx SoCs */
	for (int i = 0; i < priv->r->l2_bucket_size; i++) {
		entry = priv->r->read_l2_entry_using_hash(key, i, &e);

		if (!e.valid || ((entry & 0x0fffffffffffffffULL) == seed)) {
			idx = i > 3 ? ((key >> 14) & 0xffff) | i >> 1
					: ((key << 2) | i) & 0xffff;
			break;
		}
	}

	if (idx < 0) {
		pr_err("%s: No more L2 forwarding entries available\n", __func__);
		return -1;
	}

	/* Found an existing (e->valid is true) or empty entry, make it a nexthop entry */
	nh->l2_id = idx;
	if (e.valid) {
		nh->port = e.port;
		nh->vid = e.vid;		/* Save VID */
		nh->rvid = e.rvid;
		nh->dev_id = e.stack_dev;
		/* If the entry is already a valid next hop entry, don't change it */
		if (e.next_hop)
			return 0;
	} else {
		e.valid = true;
		e.is_static = true;
		e.rvid = nh->rvid;
		e.is_ip_mc = false;
		e.is_ipv6_mc = false;
		e.block_da = false;
		e.block_sa = false;
		e.suspended = false;
		e.age = 0;			/* With port-ignore */
		e.port = priv->r->port_ignore;
		u64_to_ether_addr(nh->mac, &e.mac[0]);
	}
	e.next_hop = true;
	e.nh_route_id = nh->id;			/* NH route ID takes place of VID */
	e.nh_vlan_target = false;

	priv->r->write_l2_entry_using_hash(idx >> 2, idx & 0x3, &e);

	return 0;
}

/* Removes a Layer 2 next hop entry in the forwarding database
 * If it was static, the entire entry is removed, otherwise the nexthop bit is cleared
 * and we wait until the entry ages out
 */
int rtl83xx_l2_nexthop_rm(struct rtl838x_switch_priv *priv, struct otto_l3_nexthop *nh)
{
	struct rtl838x_l2_entry e;
	u32 key = nh->l2_id >> 2;
	int i = nh->l2_id & 0x3;
	u64 entry = entry = priv->r->read_l2_entry_using_hash(key, i, &e);

	pr_debug("%s: id %d, key %d, index %d\n", __func__, nh->l2_id, key, i);
	if (!e.valid) {
		dev_err(priv->dev, "unknown nexthop, id %x\n", nh->l2_id);
		return -1;
	}

	if (e.is_static)
		e.valid = false;
	e.next_hop = false;
	e.vid = nh->vid;		/* Restore VID */
	e.rvid = nh->rvid;

	priv->r->write_l2_entry_using_hash(key, i, &e);

	return 0;
}

int rtl83xx_port_is_under(const struct net_device *dev, struct rtl838x_switch_priv *priv)
{
	/* Is the lower network device a DSA user network device of our RTL930X-switch?
	 * Unfortunately we cannot just follow dev->dsa_prt as this is only set for the
	 * DSA conduit device. TODO: since 6.12:
	 *
	 * if(!dsa_user_dev_check(dev)) {
	 *   netdev_info(dev, "%s: not a DSA device.\n", __func__);
	 *   return -EINVAL;
	 * }
	 */

	for (int i = 0; i < priv->r->cpu_port; i++) {
		if (!priv->ports[i].dp)
			continue;
		if (priv->ports[i].dp->user == dev)
			return i;
	}

	return -EINVAL;
}

static irqreturn_t rtldsa_switch_irq(int irq, void *dev_id)
{
	struct rtl838x_switch_priv *priv;
	struct dsa_switch *ds = dev_id;
	u64 link, ports;

	priv = ds->priv;
	ports = priv->r->get_port_reg_le(priv->r->isr_port_link_sts_chg);
	priv->r->set_port_reg_le(ports, priv->r->isr_port_link_sts_chg);

	/* read latched */
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);
	link = priv->r->get_port_reg_le(priv->r->mac_link_sts);

	for (int port = 0; port < priv->r->cpu_port; port++)
		if (ports & BIT_ULL(port))
			dsa_port_phylink_mac_change(ds, port, link & BIT_ULL(port));

	return IRQ_HANDLED;
}

/*
 * TODO: This check is usually built into the DSA initialization functions. After carving
 * out the mdio driver from the ethernet driver, there are two drivers that must be loaded
 * before the DSA setup can start. This driver has severe issues with handling of deferred
 * probing. For now provide this function for early dependency checks.
 */
static int rtldsa_ethernet_loaded(struct platform_device *pdev)
{
	struct platform_device *eth_pdev;
	struct device_node *port_np;
	struct device_node *eth_np;
	int ret = -EPROBE_DEFER;

	for_each_node_with_property(port_np, "ethernet") {
		eth_np = of_parse_phandle(port_np, "ethernet", 0);
		if (!eth_np)
			continue;

		eth_pdev = of_find_device_by_node(eth_np);
		of_node_put(eth_np);

		if (!eth_pdev)
			continue;

		if (eth_pdev->dev.driver)
			ret = 0;

		put_device(&eth_pdev->dev);
	}

	return ret;
}

static int rtl83xx_sw_probe(struct platform_device *pdev)
{
	struct rtl838x_switch_priv *priv;
	struct device *dev = &pdev->dev;
	const struct rtldsa_config *r;
	u64 bpdu_mask;
	int err = 0;

	pr_debug("Probing RTL838X switch device\n");
	if (!pdev->dev.of_node) {
		dev_err(dev, "No DT found\n");
		return -EINVAL;
	}

	if (rtldsa_phys_load_deferred())
		return -EPROBE_DEFER;

	err = rtldsa_ethernet_loaded(pdev);
	if (err)
		return err;

	/* Initialize access to RTL switch tables */
	rtl_table_init();

	r = device_get_match_data(&pdev->dev);
	priv = devm_kzalloc(dev, struct_size(priv, msts, r->n_mst - 1), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->r = r;

	priv->ds = devm_kzalloc(dev, sizeof(*priv->ds), GFP_KERNEL);
	if (!priv->ds)
		return -ENOMEM;

	priv->ds->dev = dev;
	priv->ds->priv = priv;
	priv->ds->needs_standalone_vlan_filtering = true;
	priv->ds->ops = priv->r->switch_ops;
	priv->ds->phylink_mac_ops = priv->r->phylink_mac_ops;
	priv->ds->num_lag_ids = priv->r->num_lag_ids;
	priv->ds->num_ports = priv->r->cpu_port + 1;

	priv->dev = dev;
	dev_set_drvdata(dev, priv);

	err = devm_mutex_init(dev, &priv->reg_mutex);
	if (err)
		return err;

	err = devm_mutex_init(dev, &priv->counters_lock);
	if (err)
		return err;

	priv->family_id = soc_info.family;
	sw_w32(0, priv->r->spanning_tree_ctrl);
	priv->irq_mask = GENMASK_ULL(priv->r->cpu_port - 1, 0);

	err = rtldsa_mdio_loaded();
	if (err)
		return err;

	err = rtldsa_ports_probe(priv);
	if (err)
		return err;

	priv->wq = create_singlethread_workqueue("rtl83xx");
	if (!priv->wq) {
		dev_err(dev, "Error creating workqueue: %d\n", err);
		return -ENOMEM;
	}

	err = dsa_register_switch(priv->ds);
	if (err) {
		dev_err(dev, "Error registering switch: %d\n", err);
		goto err_register_switch;
	}

	/* dsa_to_port returns dsa_port from the port list in
	 * dsa_switch_tree, the tree is built when the switch
	 * is registered by dsa_register_switch
	 */
	for (int i = 0; i <= priv->r->cpu_port; i++)
		priv->ports[i].dp = dsa_to_port(priv->ds, i);

	/* Enable link and media change interrupts. Are the SERDES masks needed? */
	sw_w32_mask(0, 3, priv->r->isr_glb_src);

	priv->r->set_port_reg_le(priv->irq_mask, priv->r->isr_port_link_sts_chg);
	priv->r->set_port_reg_le(priv->irq_mask, priv->r->imr_port_link_sts_chg);

	priv->link_state_irq = platform_get_irq(pdev, 0);
	pr_info("LINK state irq: %d\n", priv->link_state_irq);
	err = request_irq(priv->link_state_irq, rtldsa_switch_irq,
			  IRQF_SHARED, "rtldsa-link-state", priv->ds);
	if (err) {
		dev_err(dev, "Error setting up switch interrupt.\n");
		/* Need to free allocated switch here */
	}

	/* Enable interrupts for switch, on RTL931x, the IRQ is always on globally */
	if (soc_info.family != RTL9310_FAMILY_ID)
		sw_w32(0x1, priv->r->imr_glb);

	rtl83xx_get_l2aging(priv);

	if (priv->r->qos_init)
		priv->r->qos_init(priv);

	/* Clear all destination ports for mirror groups */
	for (int i = 0; i < 4; i++)
		priv->mirror_group_ports[i] = -1;

	err = otto_l3_probe(dev, priv);
	if (err)
		goto err_register_l3;

	/* TODO: put this into l2_setup() */
	switch (soc_info.family) {
	default:
		/* Flood BPDUs to all ports including cpu-port */
		bpdu_mask = soc_info.family == RTL8380_FAMILY_ID ? 0x1FFFFFFF : 0x1FFFFFFFFFFFFF;
		priv->r->set_port_reg_be(bpdu_mask, priv->r->rma_bpdu_fld_pmask);

		/* TRAP 802.1X frames (EAPOL) to the CPU-Port, bypass STP and VLANs */
		sw_w32(7, priv->r->spcl_trap_eapol_ctrl);

		rtl838x_dbgfs_init(priv);
		break;
	case RTL9300_FAMILY_ID:
	case RTL9310_FAMILY_ID:
		rtl930x_dbgfs_init(priv);
		break;
	}

	if (priv->r->lag_switch_init)
		priv->r->lag_switch_init(priv);

	return 0;

err_register_l3:
	dsa_switch_shutdown(priv->ds);
err_register_switch:
	destroy_workqueue(priv->wq);

	return err;
}

void rtldsa_93xx_lag_switch_init(struct rtl838x_switch_priv *priv)
{
	u32 trk_ctrlmask = 0;
	u32 algomask;

	trk_ctrlmask |= RTL93XX_TRK_CTRL_NON_TMN_TUNNEL_HASH_SEL;
	trk_ctrlmask |= RTL93XX_TRK_CTRL_TRK_STAND_ALONE_MODE;
	trk_ctrlmask |= RTL93XX_TRK_CTRL_LOCAL_FIRST;

	sw_w32(trk_ctrlmask, priv->r->trk_ctrl);

	/* Setup NETDEV_LAG_HASH_L2 on slot 0 */
	algomask = TRUNK_DISTRIBUTION_ALGO_SMAC_BIT |
		   TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;
	priv->r->lag_set_distribution_algorithm(priv, 0, RTL93XX_HASH_MASK_INDEX_L2, algomask);

	/* Setup NETDEV_LAG_HASH_L23 on slot 1 */
	algomask = TRUNK_DISTRIBUTION_ALGO_SMAC_BIT |
		   TRUNK_DISTRIBUTION_ALGO_DMAC_BIT |
		   TRUNK_DISTRIBUTION_ALGO_SIP_BIT |
		   TRUNK_DISTRIBUTION_ALGO_DIP_BIT;
	priv->r->lag_set_distribution_algorithm(priv, 0, RTL93XX_HASH_MASK_INDEX_L23, algomask);
}

static void rtl83xx_sw_remove(struct platform_device *pdev)
{
	struct rtl838x_switch_priv *priv = platform_get_drvdata(pdev);

	if (!priv)
		return;

	/* TODO: */
	pr_debug("Removing platform driver for rtl83xx-sw\n");

	/* unregister notifiers which will create workqueue entries with
	 * references to the switch structures. Also stop self-arming delayed
	 * work items to avoid them still accessing the DSA structures
	 * when they are getting shut down.
	 */
	otto_l3_remove(priv);
	cancel_delayed_work_sync(&priv->counters_work);

	dsa_switch_shutdown(priv->ds);

	destroy_workqueue(priv->wq);

	dev_set_drvdata(&pdev->dev, NULL);
}

static const struct of_device_id rtl83xx_switch_of_ids[] = {
	{
		.compatible = "realtek,rtl8380-switch",
		.data = &rtldsa_838x_cfg,
	},
	{
		.compatible = "realtek,rtl8392-switch",
		.data = &rtldsa_839x_cfg,
	},
	{
		.compatible = "realtek,rtl9301-switch",
		.data = &rtldsa_930x_cfg,
	},
	{
		.compatible = "realtek,rtl9311-switch",
		.data = &rtldsa_931x_cfg,
	},
	{ /* sentinel */ }
};

MODULE_DEVICE_TABLE(of, rtl83xx_switch_of_ids);

static struct platform_driver rtl83xx_switch_driver = {
	.probe  = rtl83xx_sw_probe,
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
