// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/reset.h>
#include <linux/hrtimer.h>
#include <linux/mii.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/phy.h>

#include "mt753x.h"
#include "mt753x_swconfig.h"
#include "mt753x_regs.h"
#include "mt753x_nl.h"
#include "mt7530.h"
#include "mt7531.h"

static u32 mt753x_id;
struct list_head mt753x_devs;
static DEFINE_MUTEX(mt753x_devs_lock);

static struct mt753x_sw_id *mt753x_sw_ids[] = {
	&mt7530_id,
	&mt7531_id,
};

u32 mt753x_reg_read(struct gsw_mt753x *gsw, u32 reg)
{
	u32 high, low;

	if (!IS_ERR_OR_NULL(gsw->base)) {
#if MT7988_FPGA
		/* Top registers are removed on Jaguar,
		   Temporarily, let switch application can get the chip-name.
		*/
		if (reg == 0x781C)
			return 0x75310001;
#endif
		mutex_lock(&gsw->host_bus->mdio_lock);
		low = __raw_readl(gsw->base + reg);
		mutex_unlock(&gsw->host_bus->mdio_lock);
		return low;
	} else {
		mutex_lock(&gsw->host_bus->mdio_lock);
	gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f,
		(reg & MT753X_REG_PAGE_ADDR_M) >> MT753X_REG_PAGE_ADDR_S);

	low = gsw->host_bus->read(gsw->host_bus, gsw->smi_addr,
		(reg & MT753X_REG_ADDR_M) >> MT753X_REG_ADDR_S);

	high = gsw->host_bus->read(gsw->host_bus, gsw->smi_addr, 0x10);

	mutex_unlock(&gsw->host_bus->mdio_lock);

	return (high << 16) | (low & 0xffff);
	}
}

void mt753x_reg_write(struct gsw_mt753x *gsw, u32 reg, u32 val)
{
	mutex_lock(&gsw->host_bus->mdio_lock);

	if (!IS_ERR_OR_NULL(gsw->base)) {
		__raw_writel(val, gsw->base + reg);
	} else {
		gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x1f,
			(reg & MT753X_REG_PAGE_ADDR_M) >> MT753X_REG_PAGE_ADDR_S);

		gsw->host_bus->write(gsw->host_bus, gsw->smi_addr,
			(reg & MT753X_REG_ADDR_M) >> MT753X_REG_ADDR_S, val & 0xffff);

		gsw->host_bus->write(gsw->host_bus, gsw->smi_addr, 0x10, val >> 16);
	}

	mutex_unlock(&gsw->host_bus->mdio_lock);
}

/* Indirect MDIO clause 22/45 access */
static int mt753x_mii_rw(struct gsw_mt753x *gsw, int phy, int reg, u16 data,
			 u32 cmd, u32 st)
{
	ktime_t timeout;
	u32 val, timeout_us;
	int ret = 0;

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = mt753x_reg_read(gsw, PHY_IAC);

		if ((val & PHY_ACS_ST) == 0)
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;
	}

	val = (st << MDIO_ST_S) |
	      ((cmd << MDIO_CMD_S) & MDIO_CMD_M) |
	      ((phy << MDIO_PHY_ADDR_S) & MDIO_PHY_ADDR_M) |
	      ((reg << MDIO_REG_ADDR_S) & MDIO_REG_ADDR_M);

	if (cmd == MDIO_CMD_WRITE || cmd == MDIO_CMD_ADDR)
		val |= data & MDIO_RW_DATA_M;

	mt753x_reg_write(gsw, PHY_IAC, val | PHY_ACS_ST);

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = mt753x_reg_read(gsw, PHY_IAC);

		if ((val & PHY_ACS_ST) == 0)
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;
	}

	if (cmd == MDIO_CMD_READ || cmd == MDIO_CMD_READ_C45) {
		val = mt753x_reg_read(gsw, PHY_IAC);
		ret = val & MDIO_RW_DATA_M;
	}

	return ret;
}

int mt753x_mii_read(struct gsw_mt753x *gsw, int phy, int reg)
{
	int val;

	if (phy < MT753X_NUM_PHYS)
		phy = (gsw->phy_base + phy) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);
	val = mt753x_mii_rw(gsw, phy, reg, 0, MDIO_CMD_READ, MDIO_ST_C22);
	mutex_unlock(&gsw->mii_lock);

	return val;
}

void mt753x_mii_write(struct gsw_mt753x *gsw, int phy, int reg, u16 val)
{
	if (phy < MT753X_NUM_PHYS)
		phy = (gsw->phy_base + phy) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);
	mt753x_mii_rw(gsw, phy, reg, val, MDIO_CMD_WRITE, MDIO_ST_C22);
	mutex_unlock(&gsw->mii_lock);
}

int mt753x_mmd_read(struct gsw_mt753x *gsw, int addr, int devad, u16 reg)
{
	int val;

	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);
	mt753x_mii_rw(gsw, addr, devad, reg, MDIO_CMD_ADDR, MDIO_ST_C45);
	val = mt753x_mii_rw(gsw, addr, devad, 0, MDIO_CMD_READ_C45,
			    MDIO_ST_C45);
	mutex_unlock(&gsw->mii_lock);

	return val;
}

void mt753x_mmd_write(struct gsw_mt753x *gsw, int addr, int devad, u16 reg,
		      u16 val)
{
	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);
	mt753x_mii_rw(gsw, addr, devad, reg, MDIO_CMD_ADDR, MDIO_ST_C45);
	mt753x_mii_rw(gsw, addr, devad, val, MDIO_CMD_WRITE, MDIO_ST_C45);
	mutex_unlock(&gsw->mii_lock);
}

int mt753x_mmd_ind_read(struct gsw_mt753x *gsw, int addr, int devad, u16 reg)
{
	u16 val;

	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);

	mt753x_mii_rw(gsw, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_ADDR << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M),
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mt753x_mii_rw(gsw, addr, MII_MMD_ADDR_DATA_REG, reg,
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mt753x_mii_rw(gsw, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_DATA << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M),
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	val = mt753x_mii_rw(gsw, addr, MII_MMD_ADDR_DATA_REG, 0,
			    MDIO_CMD_READ, MDIO_ST_C22);

	mutex_unlock(&gsw->mii_lock);

	return val;
}

void mt753x_mmd_ind_write(struct gsw_mt753x *gsw, int addr, int devad, u16 reg,
			  u16 val)
{
	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->mii_lock);

	mt753x_mii_rw(gsw, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_ADDR << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M),
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mt753x_mii_rw(gsw, addr, MII_MMD_ADDR_DATA_REG, reg,
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mt753x_mii_rw(gsw, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_DATA << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M),
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mt753x_mii_rw(gsw, addr, MII_MMD_ADDR_DATA_REG, val,
		      MDIO_CMD_WRITE, MDIO_ST_C22);

	mutex_unlock(&gsw->mii_lock);
}

static inline int mt753x_get_duplex(const struct device_node *np)
{
	return of_property_read_bool(np, "full-duplex");
}

static void mt753x_load_port_cfg(struct gsw_mt753x *gsw)
{
	struct device_node *port_np;
	struct device_node *fixed_link_node;
	struct mt753x_port_cfg *port_cfg;
	u32 port;
	int ret;

	for_each_child_of_node(gsw->dev->of_node, port_np) {
		if (!of_device_is_compatible(port_np, "mediatek,mt753x-port"))
			continue;

		if (!of_device_is_available(port_np))
			continue;

		if (of_property_read_u32(port_np, "reg", &port))
			continue;

		switch (port) {
		case 5:
			port_cfg = &gsw->port5_cfg;
			break;
		case 6:
			port_cfg = &gsw->port6_cfg;
			break;
		default:
			continue;
		}

		if (port_cfg->enabled) {
			dev_info(gsw->dev, "duplicated node for port%d\n",
				 port_cfg->phy_mode);
			continue;
		}

		port_cfg->np = port_np;

		ret = of_get_phy_mode(port_np, &port_cfg->phy_mode);
		if (ret < 0) {
			dev_info(gsw->dev, "incorrect phy-mode %d\n", port);
			continue;
		}

		fixed_link_node = of_get_child_by_name(port_np, "fixed-link");
		if (fixed_link_node) {
			u32 speed;

			port_cfg->force_link = 1;
			port_cfg->duplex = mt753x_get_duplex(fixed_link_node);

			if (of_property_read_u32(fixed_link_node, "speed",
						 &speed)) {
				speed = 0;
				continue;
			}

			of_node_put(fixed_link_node);

			switch (speed) {
			case 10:
				port_cfg->speed = MAC_SPD_10;
				break;
			case 100:
				port_cfg->speed = MAC_SPD_100;
				break;
			case 1000:
				port_cfg->speed = MAC_SPD_1000;
				break;
			case 2500:
				port_cfg->speed = MAC_SPD_2500;
				break;

			default:
				dev_info(gsw->dev, "incorrect speed %d\n",
					 speed);
				continue;
			}
		}

		port_cfg->ssc_on = of_property_read_bool(port_cfg->np,
							 "mediatek,ssc-on");
		port_cfg->stag_on = of_property_read_bool(port_cfg->np,
							  "mediatek,stag-on");
		port_cfg->enabled = 1;
	}
}

void mt753x_tr_write(struct gsw_mt753x *gsw, int addr, u8 ch, u8 node, u8 daddr,
		     u32 data)
{
	ktime_t timeout;
	u32 timeout_us;
	u32 val;

	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, PHY_TR_PAGE);

	val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

		if (!!(val & PHY_TR_PKT_XMT_STA))
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			goto out;
	}

	gsw->mii_write(gsw, addr, PHY_TR_LOW_DATA, PHY_TR_LOW_VAL(data));
	gsw->mii_write(gsw, addr, PHY_TR_HIGH_DATA, PHY_TR_HIGH_VAL(data));
	val = PHY_TR_PKT_XMT_STA | (PHY_TR_WRITE << PHY_TR_WR_S) |
	      (ch << PHY_TR_CH_ADDR_S) | (node << PHY_TR_NODE_ADDR_S) |
	      (daddr << PHY_TR_DATA_ADDR_S);
	gsw->mii_write(gsw, addr, PHY_TR_CTRL, val);

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

		if (!!(val & PHY_TR_PKT_XMT_STA))
			break;

		if (ktime_compare(ktime_get(), timeout) > 0)
			goto out;
	}
out:
	gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, 0);
}

int mt753x_tr_read(struct gsw_mt753x *gsw, int addr, u8 ch, u8 node, u8 daddr)
{
	ktime_t timeout;
	u32 timeout_us;
	u32 val;
	u8 val_h;

	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, PHY_TR_PAGE);

	val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

		if (!!(val & PHY_TR_PKT_XMT_STA))
			break;

		if (ktime_compare(ktime_get(), timeout) > 0) {
			gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, 0);
			return -ETIMEDOUT;
		}
	}

	val = PHY_TR_PKT_XMT_STA | (PHY_TR_READ << PHY_TR_WR_S) |
	      (ch << PHY_TR_CH_ADDR_S) | (node << PHY_TR_NODE_ADDR_S) |
	      (daddr << PHY_TR_DATA_ADDR_S);
	gsw->mii_write(gsw, addr, PHY_TR_CTRL, val);

	timeout_us = 100000;
	timeout = ktime_add_us(ktime_get(), timeout_us);
	while (1) {
		val = gsw->mii_read(gsw, addr, PHY_TR_CTRL);

		if (!!(val & PHY_TR_PKT_XMT_STA))
			break;

		if (ktime_compare(ktime_get(), timeout) > 0) {
			gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, 0);
			return -ETIMEDOUT;
		}
	}

	val = gsw->mii_read(gsw, addr, PHY_TR_LOW_DATA);
	val_h = gsw->mii_read(gsw, addr, PHY_TR_HIGH_DATA);
	val |= (val_h << 16);

	gsw->mii_write(gsw, addr, PHY_CL22_PAGE_CTRL, 0);

	return val;
}

static void mt753x_add_gsw(struct gsw_mt753x *gsw)
{
	mutex_lock(&mt753x_devs_lock);
	gsw->id = mt753x_id++;
	INIT_LIST_HEAD(&gsw->list);
	list_add_tail(&gsw->list, &mt753x_devs);
	mutex_unlock(&mt753x_devs_lock);
}

static void mt753x_remove_gsw(struct gsw_mt753x *gsw)
{
	mutex_lock(&mt753x_devs_lock);
	list_del(&gsw->list);
	mutex_unlock(&mt753x_devs_lock);
}


struct gsw_mt753x *mt753x_get_gsw(u32 id)
{
	struct gsw_mt753x *dev;

	mutex_lock(&mt753x_devs_lock);

	list_for_each_entry(dev, &mt753x_devs, list) {
		if (dev->id == id) {
			mutex_unlock(&mt753x_devs_lock);
			return dev;
		}
	}

	mutex_unlock(&mt753x_devs_lock);

	return NULL;
}

struct gsw_mt753x *mt753x_get_first_gsw(void)
{
	struct gsw_mt753x *dev;

	mutex_lock(&mt753x_devs_lock);

	list_for_each_entry(dev, &mt753x_devs, list) {
		mutex_unlock(&mt753x_devs_lock);
		return dev;
	}

	mutex_unlock(&mt753x_devs_lock);

	return NULL;
}

void mt753x_put_gsw(void)
{
	mutex_unlock(&mt753x_devs_lock);
}

void mt753x_lock_gsw(void)
{
	mutex_lock(&mt753x_devs_lock);
}

static int mt753x_hw_reset(struct gsw_mt753x *gsw)
{
	struct device_node *np = gsw->dev->of_node;
	struct reset_control *rstc;
	int mcm;
	int ret = -EINVAL;

	mcm = of_property_read_bool(np, "mediatek,mcm");
	if (mcm) {
		rstc = devm_reset_control_get(gsw->dev, "mcm");
		ret = IS_ERR(rstc);
		if (IS_ERR(rstc)) {
			dev_err(gsw->dev, "Missing reset ctrl of switch\n");
			return ret;
		}

		reset_control_assert(rstc);
		msleep(30);
		reset_control_deassert(rstc);

		gsw->reset_pin = -1;
		return 0;
	}

	gsw->reset_pin = of_get_named_gpio(np, "reset-gpios", 0);
	if (gsw->reset_pin < 0) {
		dev_err(gsw->dev, "Missing reset pin of switch\n");
		return ret;
	}

	ret = devm_gpio_request(gsw->dev, gsw->reset_pin, "mt753x-reset");
	if (ret) {
		dev_info(gsw->dev, "Failed to request gpio %d\n",
			 gsw->reset_pin);
		return ret;
	}

	gpio_direction_output(gsw->reset_pin, 0);
	msleep(30);
	gpio_set_value(gsw->reset_pin, 1);
	msleep(500);

	return 0;
}
#if 1 //XDXDXDXD
static int mt753x_mdio_read(struct mii_bus *bus, int addr, int reg)
{
	struct gsw_mt753x *gsw = bus->priv;

	return gsw->mii_read(gsw, addr, reg);
}

static int mt753x_mdio_write(struct mii_bus *bus, int addr, int reg, u16 val)
{
	struct gsw_mt753x *gsw = bus->priv;

	gsw->mii_write(gsw, addr, reg, val);

	return 0;
}

static const struct net_device_ops mt753x_dummy_netdev_ops = {
};

static void mt753x_phy_link_handler(struct net_device *dev)
{
	struct mt753x_phy *phy = container_of(dev, struct mt753x_phy, netdev);
	struct phy_device *phydev = phy->phydev;
	struct gsw_mt753x *gsw = phy->gsw;
	u32 port = phy - gsw->phys;

	if (phydev->link) {
		dev_info(gsw->dev,
			 "Port %d Link is Up - %s/%s - flow control %s\n",
			 port, phy_speed_to_str(phydev->speed),
			 (phydev->duplex == DUPLEX_FULL) ? "Full" : "Half",
			 phydev->pause ? "rx/tx" : "off");
	} else {
		dev_info(gsw->dev, "Port %d Link is Down\n", port);
	}
}

static void mt753x_connect_internal_phys(struct gsw_mt753x *gsw,
					 struct device_node *mii_np)
{
	struct device_node *phy_np;
	struct mt753x_phy *phy;
	phy_interface_t iface;
	int ret;
	u32 phyad;

	if (!mii_np)
		return;

	for_each_child_of_node(mii_np, phy_np) {
		if (of_property_read_u32(phy_np, "reg", &phyad))
			continue;

		if (phyad >= MT753X_NUM_PHYS)
			continue;

		ret = of_get_phy_mode(phy_np, &iface);
		if (ret < 0) {
			dev_info(gsw->dev, "incorrect phy-mode %d for PHY %d\n",
				 iface, phyad);
			continue;
		}

		phy = &gsw->phys[phyad];
		phy->gsw = gsw;

		init_dummy_netdev(&phy->netdev);
		phy->netdev.netdev_ops = &mt753x_dummy_netdev_ops;

		phy->phydev = of_phy_connect(&phy->netdev, phy_np,
					mt753x_phy_link_handler, 0, iface);
		if (!phy->phydev) {
			dev_info(gsw->dev, "could not connect to PHY %d\n",
				 phyad);
			continue;
		}

		phy_start(phy->phydev);
	}
}

static void mt753x_disconnect_internal_phys(struct gsw_mt753x *gsw)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(gsw->phys); i++) {
		if (gsw->phys[i].phydev) {
			phy_stop(gsw->phys[i].phydev);
			phy_disconnect(gsw->phys[i].phydev);
			gsw->phys[i].phydev = NULL;
		}
	}
}

static int mt753x_mdio_register(struct gsw_mt753x *gsw)
{
	struct device_node *mii_np;
	int i, ret;

	mii_np = of_get_child_by_name(gsw->dev->of_node, "mdio-bus");
	if (mii_np && !of_device_is_available(mii_np)) {
		ret = -ENODEV;
		goto err_put_node;
	}

	gsw->gphy_bus = devm_mdiobus_alloc(gsw->dev);
	if (!gsw->gphy_bus) {
		ret = -ENOMEM;
		goto err_put_node;
	}

	gsw->gphy_bus->name = "mt753x_mdio";
	gsw->gphy_bus->read = mt753x_mdio_read;
	gsw->gphy_bus->write = mt753x_mdio_write;
	gsw->gphy_bus->priv = gsw;
	gsw->gphy_bus->parent = gsw->dev;
	gsw->gphy_bus->phy_mask = BIT(MT753X_NUM_PHYS) - 1;
//	gsw->gphy_bus->irq = gsw->phy_irqs;

	for (i = 0; i < PHY_MAX_ADDR; i++)
		gsw->gphy_bus->irq[i] = PHY_POLL;

	if (mii_np)
		snprintf(gsw->gphy_bus->id, MII_BUS_ID_SIZE, "%s@%s",
			 mii_np->name, gsw->dev->of_node->name);
	else
		snprintf(gsw->gphy_bus->id, MII_BUS_ID_SIZE, "mdio@%s",
			 gsw->dev->of_node->name);

	ret = of_mdiobus_register(gsw->gphy_bus, mii_np);

	if (ret) {
		gsw->gphy_bus = NULL;
	} else {
		if (gsw->phy_status_poll)
			mt753x_connect_internal_phys(gsw, mii_np);
	}

err_put_node:
	if (mii_np)
		of_node_put(mii_np);

	return ret;
}
#endif

#ifdef MT7988_FPGA
u32 mt753x_direct_read(u32 reg)
{
	struct gsw_mt753x *gsw = mt753x_get_first_gsw();

	if (gsw)
		return mt753x_reg_read(gsw, reg);
	else
		return 0;
}
EXPORT_SYMBOL(mt753x_direct_read);

void mt753x_direct_write(u32 reg, u32 value)
{
	struct gsw_mt753x *gsw = mt753x_get_first_gsw();

	if (gsw) {
		mt753x_reg_write(gsw, reg, value);
	}
}
EXPORT_SYMBOL(mt753x_direct_write);
#endif

static irqreturn_t mt753x_irq_handler(int irq, void *dev)
{
	struct gsw_mt753x *gsw = dev;

	disable_irq_nosync(gsw->irq);

	schedule_work(&gsw->irq_worker);

	return IRQ_HANDLED;
}

static int mt753x_probe(struct platform_device *pdev)
{
	struct gsw_mt753x *gsw;
	struct mt753x_sw_id *sw;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mdio;
	struct mii_bus *mdio_bus;
	int ret = -EINVAL;
	struct chip_rev rev;
	struct mt753x_mapping *map;
	int i;

	mdio = of_parse_phandle(np, "mediatek,mdio", 0);
	if (!mdio)
		return -EINVAL;

	mdio_bus = of_mdio_find_bus(mdio);
	if (!mdio_bus)
		return -EPROBE_DEFER;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct gsw_mt753x), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

#if MT7988_FPGA
	gsw->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR_OR_NULL(gsw->base)) {
		dev_err(&pdev->dev, "switch ioremap fail.\n");
		goto fail;
	}
#endif
	gsw->host_bus = mdio_bus;
	gsw->dev = &pdev->dev;
	mutex_init(&gsw->mii_lock);

	/* remove it temporarily */
#if !MT7988_FPGA
	/* Switch hard reset */
	if (mt753x_hw_reset(gsw)) {
		dev_info(&pdev->dev, "reset switch fail.\n");
		goto fail;
	}
#endif

	/* Fetch the SMI address dirst */
	if (of_property_read_u32(np, "mediatek,smi-addr", &gsw->smi_addr))
		gsw->smi_addr = MT753X_DFL_SMI_ADDR;

	/* Get LAN/WAN port mapping */
	map = mt753x_find_mapping(np);
	if (map) {
		mt753x_apply_mapping(gsw, map);
		gsw->global_vlan_enable = 1;
		dev_info(gsw->dev, "LAN/WAN VLAN setting=%s\n", map->name);
	}

	/* Load MAC port configurations */
	mt753x_load_port_cfg(gsw);

	/* Check for valid switch and then initialize */
	for (i = 0; i < ARRAY_SIZE(mt753x_sw_ids); i++) {
		if (!mt753x_sw_ids[i]->detect(gsw, &rev)) {
			sw = mt753x_sw_ids[i];

			gsw->name = rev.name;
			gsw->model = sw->model;

			dev_info(gsw->dev, "Switch is MediaTek %s rev %d",
				 gsw->name, rev.rev);

			/* Initialize the switch */
			ret = sw->init(gsw);
			if (ret)
				goto fail;

			break;
		}
	}

	if (i >= ARRAY_SIZE(mt753x_sw_ids)) {
		dev_err(gsw->dev, "No mt753x switch found\n");
		goto fail;
	}

	gsw->irq = platform_get_irq(pdev, 0);
	if (gsw->irq >= 0) {
		ret = devm_request_irq(gsw->dev, gsw->irq, mt753x_irq_handler,
				       0, dev_name(gsw->dev), gsw);
		if (ret) {
			dev_err(gsw->dev, "Failed to request irq %d\n",
				gsw->irq);
			goto fail;
		}

		INIT_WORK(&gsw->irq_worker, mt753x_irq_worker);
	}

	platform_set_drvdata(pdev, gsw);

	gsw->phy_status_poll = of_property_read_bool(gsw->dev->of_node,
						     "mediatek,phy-poll");

	mt753x_add_gsw(gsw);
#if 1 //XDXD
	mt753x_mdio_register(gsw);
#endif

	mt753x_swconfig_init(gsw);

	if (sw->post_init)
		sw->post_init(gsw);

	if (gsw->irq >= 0)
		mt753x_irq_enable(gsw);

	return 0;

fail:
	devm_kfree(&pdev->dev, gsw);

	return ret;
}

static int mt753x_remove(struct platform_device *pdev)
{
	struct gsw_mt753x *gsw = platform_get_drvdata(pdev);

	if (gsw->irq >= 0)
		cancel_work_sync(&gsw->irq_worker);

	if (gsw->reset_pin >= 0)
		devm_gpio_free(&pdev->dev, gsw->reset_pin);

#ifdef CONFIG_SWCONFIG
	mt753x_swconfig_destroy(gsw);
#endif

#if 1 //XDXD
	mt753x_disconnect_internal_phys(gsw);

	mdiobus_unregister(gsw->gphy_bus);
#endif

	mt753x_remove_gsw(gsw);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id mt753x_ids[] = {
	{ .compatible = "mediatek,mt753x" },
	{ },
};

MODULE_DEVICE_TABLE(of, mt753x_ids);

static struct platform_driver mt753x_driver = {
	.probe = mt753x_probe,
	.remove = mt753x_remove,
	.driver = {
		.name = "mt753x",
		.of_match_table = mt753x_ids,
	},
};

static int __init mt753x_init(void)
{
	int ret;

	INIT_LIST_HEAD(&mt753x_devs);
	ret = platform_driver_register(&mt753x_driver);

	mt753x_nl_init();

	return ret;
}
module_init(mt753x_init);

static void __exit mt753x_exit(void)
{
	mt753x_nl_exit();

	platform_driver_unregister(&mt753x_driver);
}
module_exit(mt753x_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Weijie Gao <weijie.gao@mediatek.com>");
MODULE_DESCRIPTION("Driver for MediaTek MT753x Gigabit Switch");
