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
#include <linux/rhashtable.h>
#include <linux/of_net.h>
#include <asm/mach-rtl838x/mach-rtl83xx.h>

#include "rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;

extern const struct rtl838x_reg rtl838x_reg;
extern const struct rtl838x_reg rtl839x_reg;
extern const struct rtl838x_reg rtl930x_reg;
extern const struct rtl838x_reg rtl931x_reg;

extern const struct dsa_switch_ops rtl83xx_switch_ops;
extern const struct dsa_switch_ops rtl930x_switch_ops;

extern const struct phylink_pcs_ops rtl83xx_pcs_ops;
extern const struct phylink_pcs_ops rtl93xx_pcs_ops;

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

	if (t >= BIT(rtl838x_tbl_regs[r].c_bit-rtl838x_tbl_regs[r].t_bit))
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

static int rtldsa_mdio_read(struct mii_bus *bus, int addr, int regnum)
{
	struct rtl838x_switch_priv *priv = bus->priv;

	return mdiobus_read_nested(priv->parent_bus, addr, regnum);
}

static int rtldsa_mdio_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	struct rtl838x_switch_priv *priv = bus->priv;

	return mdiobus_write_nested(priv->parent_bus, addr, regnum, val);
}

static int __init rtl83xx_mdio_probe(struct rtl838x_switch_priv *priv)
{
	struct device *dev = priv->dev;
	struct device_node *dn, *phy_node, *led_node, *mii_np = dev->of_node;
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

	priv->parent_bus = of_mdio_find_bus(mii_np);
	if (!priv->parent_bus) {
		pr_debug("Deferring probe of mdio bus\n");
		return -EPROBE_DEFER;
	}
	if (!of_device_is_available(mii_np))
		ret = -ENODEV;

	bus = devm_mdiobus_alloc(priv->ds->dev);
	if (!bus)
		return -ENOMEM;

	bus->name = "rtldsa_mdio";
	bus->read = rtldsa_mdio_read;
	bus->write = rtldsa_mdio_write;
	snprintf(bus->id, MII_BUS_ID_SIZE, "%s-%d", bus->name, dev->id);

	bus->parent = dev;
	priv->ds->slave_mii_bus = bus;
	priv->ds->slave_mii_bus->priv = priv;

	ret = mdiobus_register(priv->ds->slave_mii_bus);
	if (ret && mii_np) {
		of_node_put(dn);
		return ret;
	}

	dn = of_find_compatible_node(NULL, NULL, "realtek,rtl83xx-switch");
	if (!dn) {
		dev_err(priv->dev, "No RTL switch node in DTS\n");
		return -ENODEV;
	}

	led_node = of_find_compatible_node(NULL, NULL, "realtek,rtl9300-leds");

	for_each_node_by_name(dn, "port") {
		phy_interface_t interface;
		u32 led_set;
		char led_set_str[16] = {0};

		if (!of_device_is_available(dn))
			continue;

		if (of_property_read_u32(dn, "reg", &pn))
			continue;

		phy_node = of_parse_phandle(dn, "phy-handle", 0);

		/* Major cleanup is needed...
		 *
		 * We use virtual "phys" as containers for mac
		 * properties like the SERDES channel, even for simple
		 * SFP slots.  "pseudo-phy-handle" is a hack to
		 * support this construct and still allow pluggable
		 * phys.
		 *
		 * The SERDES map is most likely static by port number
		 * for each SoC.  No need to put that into the device
		 * tree in the first place.
		 */
		if (!phy_node)
			phy_node = of_parse_phandle(dn, "pseudo-phy-handle", 0);

		if (!phy_node) {
			if (pn != priv->cpu_port)
				dev_err(priv->dev, "Port node %d misses phy-handle\n", pn);
			continue;
		}

		if (of_property_read_u32(phy_node, "sds", &priv->ports[pn].sds_num))
			priv->ports[pn].sds_num = -1;
		pr_debug("%s port %d has SDS %d\n", __func__, pn, priv->ports[pn].sds_num);

		if (of_get_phy_mode(dn, &interface))
			interface = PHY_INTERFACE_MODE_NA;
		if (interface == PHY_INTERFACE_MODE_HSGMII)
			priv->ports[pn].is2G5 = true;
		if (interface == PHY_INTERFACE_MODE_USXGMII)
			priv->ports[pn].is2G5 = priv->ports[pn].is10G = true;
		if (interface == PHY_INTERFACE_MODE_10GBASER)
			priv->ports[pn].is10G = true;

		priv->ports[pn].leds_on_this_port = 0;
		if (led_node) {
			if (of_property_read_u32(dn, "led-set", &led_set))
				led_set = 0;
			priv->ports[pn].led_set = led_set;
			sprintf(led_set_str, "led_set%d", led_set);
			priv->ports[pn].leds_on_this_port = of_property_count_u32_elems(led_node, led_set_str);
			if (priv->ports[pn].leds_on_this_port > 4) {
				dev_err(priv->dev, "led_set %d for port %d configuration is invalid\n", led_set, pn);
				return -ENODEV;
			}
		}

		/* Check for the integrated SerDes of the RTL8380M first */
		if (of_property_read_bool(phy_node, "phy-is-integrated")
		    && priv->id == 0x8380 && pn >= 24) {
			pr_debug("----> FOUND A SERDES\n");
			priv->ports[pn].phy = PHY_RTL838X_SDS;
			continue;
		}

		if (priv->id >= 0x9300) {
			priv->ports[pn].phy_is_integrated = false;
			if (of_property_read_bool(phy_node, "phy-is-integrated")) {
				priv->ports[pn].phy_is_integrated = true;
				priv->ports[pn].phy = PHY_RTL930X_SDS;
			}
		} else {
			if (of_property_read_bool(phy_node, "phy-is-integrated") &&
			    !of_property_read_bool(phy_node, "sfp")) {
				priv->ports[pn].phy = PHY_RTL8218B_INT;
				continue;
			}
		}

		if (!of_property_read_bool(phy_node, "phy-is-integrated") &&
		    of_property_read_bool(phy_node, "sfp")) {
			priv->ports[pn].phy = PHY_RTL8214FC;
			continue;
		}

		if (!of_property_read_bool(phy_node, "phy-is-integrated") &&
		    !of_property_read_bool(phy_node, "sfp")) {
			priv->ports[pn].phy = PHY_RTL8218B_EXT;
			continue;
		}
	}

	/* Disable MAC polling the PHY so that we can start configuration */
	priv->r->set_port_reg_le(0ULL, priv->r->smi_poll_ctrl);

	/* Enable PHY control via SoC */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		/* Enable SerDes NWAY and PHY control via SoC */
		sw_w32_mask(BIT(7), BIT(15), RTL838X_SMI_GLB_CTRL);
	} else if (priv->family_id == RTL8390_FAMILY_ID) {
		/* Disable PHY polling via SoC */
		sw_w32_mask(BIT(7), 0, RTL839X_SMI_GLB_CTRL);
	}

	/* Power on fibre ports and reset them if necessary */
	if (priv->ports[24].phy == PHY_RTL838X_SDS) {
		pr_debug("Powering on fibre ports & reset\n");
		rtl8380_sds_power(24, 1);
		rtl8380_sds_power(26, 1);
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
int rtl83xx_lag_add(struct dsa_switch *ds, int group, int port, struct netdev_lag_upper_info *info)
{
	struct rtl838x_switch_priv *priv = ds->priv;
	int i;
	u32 algomsk = 0;
	u32 algoidx = 0;

	if (info->tx_type != NETDEV_LAG_TX_TYPE_HASH) {
		pr_err("%s: Only mode LACP 802.3ad (4) allowed.\n", __func__);
		return -EINVAL;
	}

	if (group >= priv->n_lags) {
		pr_err("%s: LAG %d invalid.\n", __func__, group);
		return -EINVAL;
	}

	if (port >= priv->cpu_port) {
		pr_err("%s: Port %d invalid.\n", __func__, port);
		return -EINVAL;
	}

	for (i = 0; i < priv->n_lags; i++) {
		if (priv->lags_port_members[i] & BIT_ULL(port))
			break;
	}
	if (i != priv->n_lags) {
		pr_err("%s: Port %d already member of LAG %d.\n", __func__, port, i);
		return -ENOSPC;
	}

	switch(info->hash_type) {
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
		algomsk |= 0x7f;
	}
	priv->r->set_distribution_algorithm(group, algoidx, algomsk);
	priv->r->mask_port_reg_be(0, BIT_ULL(port), priv->r->trk_mbr_ctr(group));
	priv->lags_port_members[group] |= BIT_ULL(port);

	pr_info("%s: Added port %d to LAG %d. Members now %016llx.\n",
		 __func__, port, group, priv->lags_port_members[group]);

	return 0;
}

/* Caller must hold priv->reg_mutex */
int rtl83xx_lag_del(struct dsa_switch *ds, int group, int port)
{
	struct rtl838x_switch_priv *priv = ds->priv;

	if (group >= priv->n_lags) {
		pr_err("%s: LAG %d invalid.\n", __func__, group);
		return -EINVAL;
	}

	if (port >= priv->cpu_port) {
		pr_err("%s: Port %d invalid.\n", __func__, port);
		return -EINVAL;
	}

	if (!(priv->lags_port_members[group] & BIT_ULL(port))) {
		pr_err("%s: Port %d not member of LAG %d.\n", __func__, port, group);
		return -ENOSPC;
	}

	/* 0x7f algo mask all */
	priv->r->mask_port_reg_be(BIT_ULL(port), 0, priv->r->trk_mbr_ctr(group));
	priv->lags_port_members[group] &= ~BIT_ULL(port);

	pr_info("%s: Removed port %d from LAG %d. Members now %016llx.\n",
		 __func__, port, group, priv->lags_port_members[group]);

	return 0;
}

// Currently Unused
// /* Allocate a 64 bit octet counter located in the LOG HW table */
// static int rtl83xx_octet_cntr_alloc(struct rtl838x_switch_priv *priv)
// {
// 	int idx;

// 	mutex_lock(&priv->reg_mutex);

// 	idx = find_first_zero_bit(priv->octet_cntr_use_bm, MAX_COUNTERS);
// 	if (idx >= priv->n_counters) {
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
	if (idx >= priv->n_counters * 2) {
		j = find_first_zero_bit(priv->octet_cntr_use_bm, MAX_COUNTERS);
		if (j >= priv->n_counters) {
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
static int rtl83xx_l2_nexthop_add(struct rtl838x_switch_priv *priv, struct rtl83xx_nexthop *nh)
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
	for (int i = 0; i < priv->l2_bucket_size; i++) {
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
		e.port = priv->port_ignore;
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
static int rtl83xx_l2_nexthop_rm(struct rtl838x_switch_priv *priv, struct rtl83xx_nexthop *nh)
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

static int rtl83xx_handle_changeupper(struct rtl838x_switch_priv *priv,
				      struct net_device *ndev,
				      struct netdev_notifier_changeupper_info *info)
{
	struct net_device *upper = info->upper_dev;
	struct netdev_lag_upper_info *lag_upper_info = NULL;
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
		lag_upper_info = info->upper_info;
		if (!priv->lag_devs[i])
			priv->lag_devs[i] = upper;
		err = rtl83xx_lag_add(priv->ds, i, priv->ports[j].dp->index, lag_upper_info);
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

/* Is the lower network device a DSA slave network device of our RTL930X-switch?
 * Unfortunately we cannot just follow dev->dsa_prt as this is only set for the
 * DSA master device.
 */
int rtl83xx_port_is_under(const struct net_device * dev, struct rtl838x_switch_priv *priv)
{
/* TODO: On 5.12:
 * 	if(!dsa_slave_dev_check(dev)) {
 *		netdev_info(dev, "%s: not a DSA device.\n", __func__);
 *		return -EINVAL;
 *	}
 */

	for (int i = 0; i < priv->cpu_port; i++) {
		if (!priv->ports[i].dp)
			continue;
		if (priv->ports[i].dp->slave == dev)
			return i;
	}

	return -EINVAL;
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

static const struct rhashtable_params route_ht_params = {
	.key_len     = sizeof(u32),
	.key_offset  = offsetof(struct rtl83xx_route, gw_ip),
	.head_offset = offsetof(struct rtl83xx_route, linkage),
};

/* Updates an L3 next hop entry in the ROUTING table */
static int rtl83xx_l3_nexthop_update(struct rtl838x_switch_priv *priv,  __be32 ip_addr, u64 mac)
{
	struct rtl83xx_route *r;
	struct rhlist_head *tmp, *list;

	rcu_read_lock();
	list = rhltable_lookup(&priv->routes, &ip_addr, route_ht_params);
	if (!list) {
		rcu_read_unlock();
		return -ENOENT;
	}

	rhl_for_each_entry_rcu(r, tmp, list, linkage) {
		pr_debug("%s: Setting up fwding: ip %pI4, GW mac %016llx\n",
			__func__, &ip_addr, mac);

		/* Reads the ROUTING table entry associated with the route */
		priv->r->route_read(r->id, r);
		pr_debug("Route with id %d to %pI4 / %d\n", r->id, &r->dst_ip, r->prefix_len);

		r->nh.mac = r->nh.gw = mac;
		r->nh.port = priv->port_ignore;
		r->nh.id = r->id;

		/* Do we need to explicitly add a DMAC entry with the route's nh index? */
		if (priv->r->set_l3_egress_mac)
			priv->r->set_l3_egress_mac(r->id, mac);

		/* Update ROUTING table: map gateway-mac and switch-mac id to route id */
		rtl83xx_l2_nexthop_add(priv, &r->nh);

		r->attr.valid = true;
		r->attr.action = ROUTE_ACT_FORWARD;
		r->attr.type = 0;
		r->attr.hit = false; /* Reset route-used indicator */

		/* Add PIE entry with dst_ip and prefix_len */
		r->pr.dip = r->dst_ip;
		r->pr.dip_m = inet_make_mask(r->prefix_len);

		if (r->is_host_route) {
			int slot = priv->r->find_l3_slot(r, false);

			pr_info("%s: Got slot for route: %d\n", __func__, slot);
			priv->r->host_route_write(slot, r);
		} else {
			priv->r->route_write(r->id, r);
			r->pr.fwd_sel = true;
			r->pr.fwd_data = r->nh.l2_id;
			r->pr.fwd_act = PIE_ACT_ROUTE_UC;
		}

		if (priv->r->set_l3_nexthop)
			priv->r->set_l3_nexthop(r->nh.id, r->nh.l2_id, r->nh.if_id);

		if (r->pr.id < 0) {
			r->pr.packet_cntr = rtl83xx_packet_cntr_alloc(priv);
			if (r->pr.packet_cntr >= 0) {
				pr_info("Using packet counter %d\n", r->pr.packet_cntr);
				r->pr.log_sel = true;
				r->pr.log_data = r->pr.packet_cntr;
			}
			priv->r->pie_rule_add(priv, &r->pr);
		} else {
			int pkts = priv->r->packet_cntr_read(r->pr.packet_cntr);
			pr_debug("%s: total packets: %d\n", __func__, pkts);

			priv->r->pie_rule_write(priv, r->pr.id, &r->pr);
		}
	}
	rcu_read_unlock();

	return 0;
}

static int rtl83xx_port_ipv4_resolve(struct rtl838x_switch_priv *priv,
				     struct net_device *dev, __be32 ip_addr)
{
	struct neighbour *n = neigh_lookup(&arp_tbl, &ip_addr, dev);
	int err = 0;
	u64 mac;

	if (!n) {
		n = neigh_create(&arp_tbl, &ip_addr, dev);
		if (IS_ERR(n))
			return PTR_ERR(n);
	}

	/* If the neigh is already resolved, then go ahead and
	 * install the entry, otherwise start the ARP process to
	 * resolve the neigh.
	 */
	if (n->nud_state & NUD_VALID) {
		mac = ether_addr_to_u64(n->ha);
		pr_info("%s: resolved mac: %016llx\n", __func__, mac);
		rtl83xx_l3_nexthop_update(priv, ip_addr, mac);
	} else {
		pr_info("%s: need to wait\n", __func__);
		neigh_event_send(n, NULL);
	}

	neigh_release(n);

	return err;
}

struct rtl83xx_walk_data {
	struct rtl838x_switch_priv *priv;
	int port;
};

static int rtl83xx_port_lower_walk(struct net_device *lower, struct netdev_nested_priv *_priv)
{
	struct rtl83xx_walk_data *data = (struct rtl83xx_walk_data *)_priv->data;
	struct rtl838x_switch_priv *priv = data->priv;
	int ret = 0;
	int index;

	index = rtl83xx_port_is_under(lower, priv);
	data->port = index;
	if (index >= 0) {
		pr_debug("Found DSA-port, index %d\n", index);
		ret = 1;
	}

	return ret;
}

static int rtl83xx_port_dev_lower_find(struct net_device *dev, struct rtl838x_switch_priv *priv)
{
	struct rtl83xx_walk_data data;
	struct netdev_nested_priv _priv;

	data.priv = priv;
	data.port = 0;
	_priv.data = (void *)&data;

	netdev_walk_all_lower_dev(dev, rtl83xx_port_lower_walk, &_priv);

	return data.port;
}

static struct rtl83xx_route *rtl83xx_route_alloc(struct rtl838x_switch_priv *priv, u32 ip)
{
	struct rtl83xx_route *r;
	int idx = 0, err;

	mutex_lock(&priv->reg_mutex);

	idx = find_first_zero_bit(priv->route_use_bm, MAX_ROUTES);
	pr_debug("%s id: %d, ip %pI4\n", __func__, idx, &ip);

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (!r) {
		mutex_unlock(&priv->reg_mutex);
		return r;
	}

	r->id = idx;
	r->gw_ip = ip;
	r->pr.id = -1; /* We still need to allocate a rule in HW */
	r->is_host_route = false;

	err = rhltable_insert(&priv->routes, &r->linkage, route_ht_params);
	if (err) {
		pr_err("Could not insert new rule\n");
		mutex_unlock(&priv->reg_mutex);
		goto out_free;
	}

	set_bit(idx, priv->route_use_bm);

	mutex_unlock(&priv->reg_mutex);

	return r;

out_free:
	kfree(r);

	return NULL;
}


static struct rtl83xx_route *rtl83xx_host_route_alloc(struct rtl838x_switch_priv *priv, u32 ip)
{
	struct rtl83xx_route *r;
	int idx = 0, err;

	mutex_lock(&priv->reg_mutex);

	idx = find_first_zero_bit(priv->host_route_use_bm, MAX_HOST_ROUTES);
	pr_debug("%s id: %d, ip %pI4\n", __func__, idx, &ip);

	r = kzalloc(sizeof(*r), GFP_KERNEL);
	if (!r) {
		mutex_unlock(&priv->reg_mutex);
		return r;
	}

	/* We require a unique route ID irrespective of whether it is a prefix or host
	 * route (on RTL93xx) as we use this ID to associate a DMAC and next-hop entry
	 */
	r->id = idx + MAX_ROUTES;

	r->gw_ip = ip;
	r->pr.id = -1; /* We still need to allocate a rule in HW */
	r->is_host_route = true;

	err = rhltable_insert(&priv->routes, &r->linkage, route_ht_params);
	if (err) {
		pr_err("Could not insert new rule\n");
		mutex_unlock(&priv->reg_mutex);
		goto out_free;
	}

	set_bit(idx, priv->host_route_use_bm);

	mutex_unlock(&priv->reg_mutex);

	return r;

out_free:
	kfree(r);

	return NULL;
}



static void rtl83xx_route_rm(struct rtl838x_switch_priv *priv, struct rtl83xx_route *r)
{
	int id;

	if (rhltable_remove(&priv->routes, &r->linkage, route_ht_params))
		dev_warn(priv->dev, "Could not remove route\n");

	if (r->is_host_route) {
		id = priv->r->find_l3_slot(r, false);
		pr_debug("%s: Got id for host route: %d\n", __func__, id);
		r->attr.valid = false;
		priv->r->host_route_write(id, r);
		clear_bit(r->id - MAX_ROUTES, priv->host_route_use_bm);
	} else {
		/* If there is a HW representation of the route, delete it */
		if (priv->r->route_lookup_hw) {
			id = priv->r->route_lookup_hw(r);
			pr_info("%s: Got id for prefix route: %d\n", __func__, id);
			r->attr.valid = false;
			priv->r->route_write(id, r);
		}
		clear_bit(r->id, priv->route_use_bm);
	}

	kfree(r);
}

static int rtl83xx_fib4_del(struct rtl838x_switch_priv *priv,
			    struct fib_entry_notifier_info *info)
{
	struct fib_nh *nh = fib_info_nh(info->fi, 0);
	struct rtl83xx_route *r;
	struct rhlist_head *tmp, *list;

	pr_debug("In %s, ip %pI4, len %d\n", __func__, &info->dst, info->dst_len);
	rcu_read_lock();
	list = rhltable_lookup(&priv->routes, &nh->fib_nh_gw4, route_ht_params);
	if (!list) {
		rcu_read_unlock();
		pr_err("%s: no such gateway: %pI4\n", __func__, &nh->fib_nh_gw4);
		return -ENOENT;
	}
	rhl_for_each_entry_rcu(r, tmp, list, linkage) {
		if (r->dst_ip == info->dst && r->prefix_len == info->dst_len) {
			pr_info("%s: found a route with id %d, nh-id %d\n",
				__func__, r->id, r->nh.id);
			break;
		}
	}
	rcu_read_unlock();

	rtl83xx_l2_nexthop_rm(priv, &r->nh);

	pr_debug("%s: Releasing packet counter %d\n", __func__, r->pr.packet_cntr);
	set_bit(r->pr.packet_cntr, priv->packet_cntr_use_bm);
	priv->r->pie_rule_rm(priv, &r->pr);

	rtl83xx_route_rm(priv, r);

	nh->fib_nh_flags &= ~RTNH_F_OFFLOAD;

	return 0;
}

/* On the RTL93xx, an L3 termination endpoint MAC address on which the router waits
 * for packets to be routed needs to be allocated.
 */
static int rtl83xx_alloc_router_mac(struct rtl838x_switch_priv *priv, u64 mac)
{
	int free_mac = -1;
	struct rtl93xx_rt_mac m;

	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < MAX_ROUTER_MACS; i++) {
		priv->r->get_l3_router_mac(i, &m);
		if (free_mac < 0 && !m.valid) {
			free_mac = i;
			continue;
		}
		if (m.valid && m.mac == mac) {
			free_mac = i;
			break;
		}
	}

	if (free_mac < 0) {
		pr_err("No free router MACs, cannot offload\n");
		mutex_unlock(&priv->reg_mutex);
		return -1;
	}

	m.valid = true;
	m.mac = mac;
	m.p_type = 0; /* An individual port, not a trunk port */
	m.p_id = 0x3f;			/* Listen on any port */
	m.p_id_mask = 0;
	m.vid = 0;			/* Listen on any VLAN... */
	m.vid_mask = 0; 		/* ... so mask needs to be 0 */
	m.mac_mask = 0xffffffffffffULL;	/* We want an exact match of the interface MAC */
	m.action = L3_FORWARD;		/* Route the packet */
	priv->r->set_l3_router_mac(free_mac, &m);

	mutex_unlock(&priv->reg_mutex);

	return 0;
}

static int rtl83xx_alloc_egress_intf(struct rtl838x_switch_priv *priv, u64 mac, int vlan)
{
	int free_mac = -1;
	struct rtl838x_l3_intf intf;
	u64 m;

	mutex_lock(&priv->reg_mutex);
	for (int i = 0; i < MAX_SMACS; i++) {
		m = priv->r->get_l3_egress_mac(L3_EGRESS_DMACS + i);
		if (free_mac < 0 && !m) {
			free_mac = i;
			continue;
		}
		if (m == mac) {
			mutex_unlock(&priv->reg_mutex);
			return i;
		}
	}

	if (free_mac < 0) {
		pr_err("No free egress interface, cannot offload\n");
		return -1;
	}

	/* Set up default egress interface 1 */
	intf.vid = vlan;
	intf.smac_idx = free_mac;
	intf.ip4_mtu_id = 1;
	intf.ip6_mtu_id = 1;
	intf.ttl_scope = 1; /* TTL */
	intf.hl_scope = 1;  /* Hop Limit */
	intf.ip4_icmp_redirect = intf.ip6_icmp_redirect = 2;  /* FORWARD */
	intf.ip4_pbr_icmp_redirect = intf.ip6_pbr_icmp_redirect = 2; /* FORWARD; */
	priv->r->set_l3_egress_intf(free_mac, &intf);

	priv->r->set_l3_egress_mac(L3_EGRESS_DMACS + free_mac, mac);

	mutex_unlock(&priv->reg_mutex);

	return free_mac;
}

static int rtl83xx_fib4_add(struct rtl838x_switch_priv *priv,
			    struct fib_entry_notifier_info *info)
{
	struct fib_nh *nh = fib_info_nh(info->fi, 0);
	struct net_device *dev = fib_info_nh(info->fi, 0)->fib_nh_dev;
	int port;
	struct rtl83xx_route *r;
	bool to_localhost;
	int vlan = is_vlan_dev(dev) ? vlan_dev_vlan_id(dev) : 0;

	pr_debug("In %s, ip %pI4, len %d\n", __func__, &info->dst, info->dst_len);
	if (!info->dst) {
		pr_info("Not offloading default route for now\n");
		return 0;
	}

	pr_debug("GW: %pI4, interface name %s, mac %016llx, vlan %d\n", &nh->fib_nh_gw4, dev->name,
		ether_addr_to_u64(dev->dev_addr), vlan
	);

	port = rtl83xx_port_dev_lower_find(dev, priv);
	if (port < 0)
		return -1;

	/* For now we only work with routes that have a gateway and are not ourself */
/*	if ((!nh->fib_nh_gw4) && (info->dst_len != 32)) */
/*		return 0; */

	if ((info->dst & 0xff) == 0xff)
		return 0;

	/* Do not offload routes to 192.168.100.x */
	if ((info->dst & 0xffffff00) == 0xc0a86400)
		return 0;

	/* Do not offload routes to 127.x.x.x */
	if ((info->dst & 0xff000000) == 0x7f000000)
		return 0;

	/* Allocate route or host-route (entry if hardware supports this) */
	if (info->dst_len == 32 && priv->r->host_route_write)
		r = rtl83xx_host_route_alloc(priv, nh->fib_nh_gw4);
	else
		r = rtl83xx_route_alloc(priv, nh->fib_nh_gw4);

	if (!r) {
		pr_err("%s: No more free route entries\n", __func__);
		return -1;
	}

	r->dst_ip = info->dst;
	r->prefix_len = info->dst_len;
	r->nh.rvid = vlan;
	to_localhost = !nh->fib_nh_gw4;

	if (priv->r->set_l3_router_mac) {
		u64 mac = ether_addr_to_u64(dev->dev_addr);

		pr_debug("Local route and router mac %016llx\n", mac);

		if (rtl83xx_alloc_router_mac(priv, mac))
			goto out_free_rt;

		/* vid = 0: Do not care about VID */
		r->nh.if_id = rtl83xx_alloc_egress_intf(priv, mac, vlan);
		if (r->nh.if_id < 0)
			goto out_free_rmac;

		if (to_localhost) {
			int slot;

			r->nh.mac = mac;
			r->nh.port = priv->port_ignore;
			r->attr.valid = true;
			r->attr.action = ROUTE_ACT_TRAP2CPU;
			r->attr.type = 0;

			slot = priv->r->find_l3_slot(r, false);
			pr_debug("%s: Got slot for route: %d\n", __func__, slot);
			priv->r->host_route_write(slot, r);
		}
	}

	/* We need to resolve the mac address of the GW */
	if (!to_localhost)
		rtl83xx_port_ipv4_resolve(priv, dev, nh->fib_nh_gw4);

	nh->fib_nh_flags |= RTNH_F_OFFLOAD;

	return 0;

out_free_rmac:
out_free_rt:
	return 0;
}

static int rtl83xx_fib6_add(struct rtl838x_switch_priv *priv,
			    struct fib6_entry_notifier_info *info)
{
	pr_debug("In %s\n", __func__);
/*	nh->fib_nh_flags |= RTNH_F_OFFLOAD; */

	return 0;
}

struct net_event_work {
	struct work_struct work;
	struct rtl838x_switch_priv *priv;
	u64 mac;
	u32 gw_addr;
};

static void rtl83xx_net_event_work_do(struct work_struct *work)
{
	struct net_event_work *net_work =
		container_of(work, struct net_event_work, work);
	struct rtl838x_switch_priv *priv = net_work->priv;

	rtl83xx_l3_nexthop_update(priv, net_work->gw_addr, net_work->mac);

	kfree(net_work);
}

static int rtl83xx_netevent_event(struct notifier_block *this,
				 unsigned long event, void *ptr)
{
	struct rtl838x_switch_priv *priv;
	struct net_device *dev;
	struct neighbour *n = ptr;
	int err, port;
	struct net_event_work *net_work;

	priv = container_of(this, struct rtl838x_switch_priv, ne_nb);

	switch (event) {
	case NETEVENT_NEIGH_UPDATE:
		if (n->tbl != &arp_tbl)
			return NOTIFY_DONE;
		dev = n->dev;
		port = rtl83xx_port_dev_lower_find(dev, priv);
		if (port < 0 || !(n->nud_state & NUD_VALID)) {
			pr_debug("%s: Neigbour invalid, not updating\n", __func__);
			return NOTIFY_DONE;
		}

		net_work = kzalloc(sizeof(*net_work), GFP_ATOMIC);
		if (!net_work)
			return NOTIFY_BAD;

		INIT_WORK(&net_work->work, rtl83xx_net_event_work_do);
		net_work->priv = priv;

		net_work->mac = ether_addr_to_u64(n->ha);
		net_work->gw_addr = *(__be32 *) n->primary_key;

		pr_debug("%s: updating neighbour on port %d, mac %016llx\n",
			__func__, port, net_work->mac);
		schedule_work(&net_work->work);
		if (err)
			netdev_warn(dev, "failed to handle neigh update (err %d)\n", err);
		break;
	}

	return NOTIFY_DONE;
}

struct rtl83xx_fib_event_work {
	struct work_struct work;
	union {
		struct fib_entry_notifier_info fen_info;
		struct fib6_entry_notifier_info fen6_info;
		struct fib_rule_notifier_info fr_info;
	};
	struct rtl838x_switch_priv *priv;
	bool is_fib6;
	unsigned long event;
};

static void rtl83xx_fib_event_work_do(struct work_struct *work)
{
	struct rtl83xx_fib_event_work *fib_work =
		container_of(work, struct rtl83xx_fib_event_work, work);
	struct rtl838x_switch_priv *priv = fib_work->priv;
	struct fib_rule *rule;
	int err;

	/* Protect internal structures from changes */
	rtnl_lock();
	pr_debug("%s: doing work, event %ld\n", __func__, fib_work->event);
	switch (fib_work->event) {
	case FIB_EVENT_ENTRY_ADD:
	case FIB_EVENT_ENTRY_REPLACE:
	case FIB_EVENT_ENTRY_APPEND:
		if (fib_work->is_fib6) {
			err = rtl83xx_fib6_add(priv, &fib_work->fen6_info);
		} else {
			err = rtl83xx_fib4_add(priv, &fib_work->fen_info);
			fib_info_put(fib_work->fen_info.fi);
		}
		if (err)
			pr_err("%s: FIB4 failed\n", __func__);
		break;
	case FIB_EVENT_ENTRY_DEL:
		rtl83xx_fib4_del(priv, &fib_work->fen_info);
		fib_info_put(fib_work->fen_info.fi);
		break;
	case FIB_EVENT_RULE_ADD:
	case FIB_EVENT_RULE_DEL:
		rule = fib_work->fr_info.rule;
		if (!fib4_rule_default(rule))
			pr_err("%s: FIB4 default rule failed\n", __func__);
		fib_rule_put(rule);
		break;
	}
	rtnl_unlock();
	kfree(fib_work);
}

/* Called with rcu_read_lock() */
static int rtl83xx_fib_event(struct notifier_block *this, unsigned long event, void *ptr)
{
	struct fib_notifier_info *info = ptr;
	struct rtl838x_switch_priv *priv;
	struct rtl83xx_fib_event_work *fib_work;

	if ((info->family != AF_INET && info->family != AF_INET6 &&
	     info->family != RTNL_FAMILY_IPMR &&
	     info->family != RTNL_FAMILY_IP6MR))
		return NOTIFY_DONE;

	priv = container_of(this, struct rtl838x_switch_priv, fib_nb);

	fib_work = kzalloc(sizeof(*fib_work), GFP_ATOMIC);
	if (!fib_work)
		return NOTIFY_BAD;

	INIT_WORK(&fib_work->work, rtl83xx_fib_event_work_do);
	fib_work->priv = priv;
	fib_work->event = event;
	fib_work->is_fib6 = false;

	switch (event) {
	case FIB_EVENT_ENTRY_ADD:
	case FIB_EVENT_ENTRY_REPLACE:
	case FIB_EVENT_ENTRY_APPEND:
	case FIB_EVENT_ENTRY_DEL:
		pr_debug("%s: FIB_ENTRY ADD/DEL, event %ld\n", __func__, event);
		if (info->family == AF_INET) {
			struct fib_entry_notifier_info *fen_info = ptr;

			if (fen_info->fi->fib_nh_is_v6) {
				NL_SET_ERR_MSG_MOD(info->extack,
					"IPv6 gateway with IPv4 route is not supported");
				kfree(fib_work);
				return notifier_from_errno(-EINVAL);
			}

			memcpy(&fib_work->fen_info, ptr, sizeof(fib_work->fen_info));
			/* Take referece on fib_info to prevent it from being
			* freed while work is queued. Release it afterwards.
			*/
			fib_info_hold(fib_work->fen_info.fi);

		} else if (info->family == AF_INET6) {
			//struct fib6_entry_notifier_info *fen6_info = ptr;
			pr_warn("%s: FIB_RULE ADD/DEL for IPv6 not supported\n", __func__);
			kfree(fib_work);
			return NOTIFY_DONE;
		}
		break;

	case FIB_EVENT_RULE_ADD:
	case FIB_EVENT_RULE_DEL:
		pr_debug("%s: FIB_RULE ADD/DEL, event: %ld\n", __func__, event);
		memcpy(&fib_work->fr_info, ptr, sizeof(fib_work->fr_info));
		fib_rule_get(fib_work->fr_info.rule);
		break;
	}

	schedule_work(&fib_work->work);

	return NOTIFY_DONE;
}

static int __init rtl83xx_sw_probe(struct platform_device *pdev)
{
	int i, err = 0;
	struct rtl838x_switch_priv *priv;
	struct device *dev = &pdev->dev;
	u64 bpdu_mask;

	pr_debug("Probing RTL838X switch device\n");
	if (!pdev->dev.of_node) {
		dev_err(dev, "No DT found\n");
		return -EINVAL;
	}

	/* Initialize access to RTL switch tables */
	rtl_table_init();

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->ds = devm_kzalloc(dev, sizeof(*priv->ds), GFP_KERNEL);

	if (!priv->ds)
		return -ENOMEM;
	priv->ds->dev = dev;
	priv->ds->priv = priv;
	priv->ds->ops = &rtl83xx_switch_ops;
	priv->ds->needs_standalone_vlan_filtering = true;
	priv->dev = dev;

	err = devm_mutex_init(dev, &priv->reg_mutex);
	if (err)
		return err;

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
		priv->n_pie_blocks = 12;
		priv->port_ignore = 0x1f;
		priv->n_counters = 128;
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
		priv->n_pie_blocks = 18;
		priv->port_ignore = 0x3f;
		priv->n_counters = 1024;
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
		priv->n_pie_blocks = 16;
		priv->port_ignore = 0x3f;
		priv->n_counters = 2048;
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

	for (i = 0; i <= priv->cpu_port; i++) {
		switch (soc_info.family) {
		case RTL8380_FAMILY_ID:
		case RTL8390_FAMILY_ID:
			priv->pcs[i].pcs.ops = &rtl83xx_pcs_ops;
			break;
		case RTL9300_FAMILY_ID:
		case RTL9310_FAMILY_ID:
			priv->pcs[i].pcs.ops = &rtl93xx_pcs_ops;
			break;
		}
		priv->pcs[i].pcs.neg_mode = true;
		priv->pcs[i].priv = priv;
		priv->pcs[i].port = i;
	}

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

	/* dsa_to_port returns dsa_port from the port list in
	 * dsa_switch_tree, the tree is built when the switch
	 * is registered by dsa_register_switch
	 */
	for (int i = 0; i <= priv->cpu_port; i++)
		priv->ports[i].dp = dsa_to_port(priv->ds, i);

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

	priv->r->l3_setup(priv);

	/* Clear all destination ports for mirror groups */
	for (int i = 0; i < 4; i++)
		priv->mirror_group_ports[i] = -1;

	/* Register netdevice event callback to catch changes in link aggregation groups */
	priv->nb.notifier_call = rtl83xx_netdevice_event;
	if (register_netdevice_notifier(&priv->nb)) {
		priv->nb.notifier_call = NULL;
		dev_err(dev, "Failed to register LAG netdev notifier\n");
		goto err_register_nb;
	}

	/* Initialize hash table for L3 routing */
	rhltable_init(&priv->routes, &route_ht_params);

	/* Register netevent notifier callback to catch notifications about neighboring
	 * changes to update nexthop entries for L3 routing.
	 */
	priv->ne_nb.notifier_call = rtl83xx_netevent_event;
	if (register_netevent_notifier(&priv->ne_nb)) {
		priv->ne_nb.notifier_call = NULL;
		dev_err(dev, "Failed to register netevent notifier\n");
		goto err_register_ne_nb;
	}

	priv->fib_nb.notifier_call = rtl83xx_fib_event;

	/* Register Forwarding Information Base notifier to offload routes where
	 * where possible
	 * Only FIBs pointing to our own netdevs are programmed into
	 * the device, so no need to pass a callback.
	 */
	err = register_fib_notifier(&init_net, &priv->fib_nb, NULL, NULL);
	if (err)
		goto err_register_fib_nb;

	/* TODO: put this into l2_setup() */
	/* Flood BPDUs to all ports including cpu-port */
	if (soc_info.family != RTL9300_FAMILY_ID) {
		bpdu_mask = soc_info.family == RTL8380_FAMILY_ID ? 0x1FFFFFFF : 0x1FFFFFFFFFFFFF;
		priv->r->set_port_reg_be(bpdu_mask, priv->r->rma_bpdu_fld_pmask);

		/* TRAP 802.1X frames (EAPOL) to the CPU-Port, bypass STP and VLANs */
		sw_w32(7, priv->r->spcl_trap_eapol_ctrl);

		rtl838x_dbgfs_init(priv);
	} else {
		rtl930x_dbgfs_init(priv);
	}

	return 0;

err_register_fib_nb:
	unregister_netevent_notifier(&priv->ne_nb);
err_register_ne_nb:
	unregister_netdevice_notifier(&priv->nb);
err_register_nb:
	return err;
}

static void rtl83xx_sw_remove(struct platform_device *pdev)
{
	/* TODO: */
	pr_debug("Removing platform driver for rtl83xx-sw\n");
}

static const struct of_device_id rtl83xx_switch_of_ids[] = {
	{ .compatible = "realtek,rtl83xx-switch"},
	{ /* sentinel */ }
};


MODULE_DEVICE_TABLE(of, rtl83xx_switch_of_ids);

static struct platform_driver rtl83xx_switch_driver = {
	.probe = rtl83xx_sw_probe,
	.remove_new = rtl83xx_sw_remove,
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
