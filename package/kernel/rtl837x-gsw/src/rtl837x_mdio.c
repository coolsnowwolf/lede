/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 StarField Xu <air_jinkela@163.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_mdio.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/gpio/consumer.h>
#include <linux/mutex.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/sfp.h>
#include <net/rtnetlink.h>

#include "./rtl837x_common.h"

#include <linux/printk.h>

const uint8_t rtl8373_port_map[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, // 物理端口0-8
    0, 0, 0, 0, 0, 0, 0        // 填充
};

const uint8_t rtl8372_port_map[16] = {
    3, 4, 5, 6, 7, 8, // 物理端口3-8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 填充
};

static struct rtl837x_mib_counter rtl837x_mib_counters[] ={
	{0,"ifInOctets"},
	{2,"ifOutOctets"},
	{4,"ifInUcastPkts"},
	{6,"ifInMulticastPkts"},
	{8,"ifInBroadcastPkts"},
	{0xA,"ifOutUcastPkts"},
	{0xC,"ifOutMulticastPkts"},
	{0xE,"ifOutBroadcastPkts"},
	{0x10,"ifOutDiscards"},
	{0x19,"InPauseFrames"},
	{0x1A,"OutPauseFrames"},
	{0x1C,"TxBroadcastPkts"},
	{0x1D,"TxMulticastPkts"},
	{0x20,"TxUndersizePkts"},
	{0x21,"RxUndersizePkts"},
	{0x22,"TxOversizePkts"},
	{0x23,"RxOversizePkts"},
	{0x24,"TxFragments"},
	{0x25,"RxFragments"},
	{0x26,"TxJabbers"},
	{0x27,"RxJabbers"},
	{0x28,"TxCollisions"},
	{0x29,"Tx64Octets"},
	{0x2A,"Rx64Octets"},
	{0x2B,"Tx65to127Bytes"},
	{0x2C,"Rx65to127Bytes"},
	{0x2D,"Tx128to255Bytes"},
	{0x2E,"Rx128to255Bytes"},
	{0x2F,"Tx256to511Bytes"},
	{0x30,"Rx256to511Bytes"},
	{0x31,"Tx512to1023Bytes"},
	{0x32,"Rx512to1023Bytes"},
	{0x33,"Tx1024to1518Bytes"},
	{0x34,"Rx1024to1518Bytes"},
	{0x36,"RxUndersizedropPkts"},
	{0x37,"Tx1519toMaxBytes"},
	{0x38,"Rx1519toMaxBytes"},
	{0x39,"TxOverMaxBytes"},
	{0x3A,"RxOverMaxBytes"}
};

static int rtl837x_mdio_write(void *ctx, u32 reg, u32 val)
{
	struct rtk_gsw *priv = ctx;
	struct mii_bus *bus = priv->bus;
	int ret;

	mutex_lock(&bus->mdio_lock);

	// check busy
	ret = bus->read(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG);
    if (ret & 0x4) {
		ret = RT_ERR_BUSYWAIT_TIMEOUT;
		goto out_unlock;
    }

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_ADDR_REG, reg);
	if (ret)
		goto out_unlock;

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_DATA_LOW, (val & 0xFFFF));
	if (ret)
		goto out_unlock;

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_DATA_HIGH, ((val >> 16) & 0xFFFF));
	if (ret)
		goto out_unlock;

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG, MDC_MDIO_WRITE_CMD);
	if (ret)
		goto out_unlock;

	// check busy
	ret = bus->read(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG);
    if (ret & 0x4) {
		ret = RT_ERR_BUSYWAIT_TIMEOUT;
		goto out_unlock;
    }
	ret = 0;
out_unlock:
	mutex_unlock(&bus->mdio_lock);
	// printk("rtl837x_mdio_write ret:%d\n", ret);
	return ret;
}

static int rtl837x_mdio_read(void *ctx, u32 reg, u32 *val)
{
	struct rtk_gsw *priv = ctx;
	struct mii_bus *bus = priv->bus;
	int ret, val_l, val_h;

	mutex_lock(&bus->mdio_lock);

	// check busy
	ret = bus->read(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG);
    if (ret & 0x4) {
		ret = RT_ERR_BUSYWAIT_TIMEOUT;
		goto out_unlock;
    }

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_ADDR_REG, reg);
	if (ret)
		goto out_unlock;

	ret = bus->write(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG, MDC_MDIO_READ_CMD);
	if (ret)
		goto out_unlock;

	// check busy
	ret = bus->read(bus, priv->mdio_addr, MDC_MDIO_CTRL_REG);
    if (ret & 0x4) {
		ret = RT_ERR_BUSYWAIT_TIMEOUT;
		goto out_unlock;
    }


	val_l = bus->read(bus, priv->mdio_addr, MDC_MDIO_DATA_LOW);
	val_h = bus->read(bus, priv->mdio_addr, MDC_MDIO_DATA_HIGH);

    *val = val_l & 0xffff;
    *val |= (val_h & 0xffff) << 16;
	ret = 0;

out_unlock:
	mutex_unlock(&bus->mdio_lock);
	// printk("rtl837x_mdio_read ret:%d\n", ret);
	return ret;
}

static void rtl837x_mdio_lock(void *ctx)
{
	struct rtk_gsw *priv = ctx;

	mutex_lock(&priv->map_lock);
}

static void rtl837x_mdio_unlock(void *ctx)
{
	struct rtk_gsw *priv = ctx;

	mutex_unlock(&priv->map_lock);
}

static const struct regmap_config rtl837x_mdio_regmap_config = {
	.reg_bits = 16,
	.val_bits = 32,
	.reg_stride = 4,

	.max_register = 0xffff,
	.reg_format_endian = REGMAP_ENDIAN_BIG,
	.reg_read = rtl837x_mdio_read,
	.reg_write = rtl837x_mdio_write,
	.cache_type = REGCACHE_NONE,
	.lock = rtl837x_mdio_lock,
	.unlock = rtl837x_mdio_unlock,
};

static const struct regmap_config rtl837x_mdio_nolock_regmap_config = {
	.reg_bits = 16,
	.val_bits = 32,
	.reg_stride = 4,

	.max_register = 0xffff,
	.reg_format_endian = REGMAP_ENDIAN_BIG,
	.reg_read = rtl837x_mdio_read,
	.reg_write = rtl837x_mdio_write,
	.cache_type = REGCACHE_NONE,
	.disable_locking = true,
};

static char* chipid_to_chip_name(switch_chip_t id)
{
	switch (id)
	{
	case CHIP_RTL8373:
		return "RTL8373";
	case CHIP_RTL8372:
		return "RTL8372";
	case CHIP_RTL8224:
		return "RTL8224";
	case CHIP_RTL8373N:
		return "RTL8373N";
	case CHIP_RTL8372N:
		return "RTL8372N";
	case CHIP_RTL8224N:
		return "RTL8224N";
	case CHIP_RTL8366U:
		return "RTL8366U";
	default:
		return "Unknow";
	}
}

static int rtl837x_switch_probe(struct rtk_gsw *gsw)
{
	switch_chip_t sw_chip;

	for(int i = 0;; i++)
	{
		if(i>3)
			goto CHIP_NOT_SUPPORTED;

		if (switch_probe(&sw_chip) != RT_ERR_OK) {
			dev_warn(gsw->dev , "Error: Detect switch type failed\n");
			mdelay(50);
		}else
			break;
	}

	switch (sw_chip)
	{
	case CHIP_RTL8372:
	case CHIP_RTL8372N:
		gsw->chip_name = chipid_to_chip_name(sw_chip);
		gsw->num_ports = 6;
		gsw->port_map = rtl8372_port_map;
		goto END_DETECT_CHIP;
	case CHIP_RTL8373:
	case CHIP_RTL8373N:
		gsw->chip_name = chipid_to_chip_name(sw_chip);
		gsw->num_ports = 9;
		gsw->port_map = rtl8373_port_map;
		goto END_DETECT_CHIP;
	default:
		goto CHIP_NOT_SUPPORTED;
	}

CHIP_NOT_SUPPORTED:
	//未知芯片ID
	rtk_uint32 regValue;
	rtl8373_getAsicReg(0x4, &regValue);
	dev_err(gsw->dev, "Error: Can not support this device, devid 0x%x\n", regValue);
	return RT_ERR_CHIP_NOT_SUPPORTED;

END_DETECT_CHIP:
	gsw->pMapper = dal_rtl8373_mapper_get();
	gsw->chip_id = sw_chip;
	dev_info(gsw->dev, "Found Realtek RTL chip %s\n", gsw->chip_name);
	return RT_ERR_OK;
}

static int rtl837x_hw_reset(struct rtk_gsw *gsw)
{
	if (!IS_ERR(gsw->reset_pin)) {
		dev_info(gsw->dev, "START HW RESET");
		gpiod_set_value(gsw->reset_pin, 1);
		mdelay(100);
		gpiod_set_value(gsw->reset_pin, 0);
		mdelay(100);
		gpiod_set_value(gsw->reset_pin, 1);
		mdelay(100);
		dev_info(gsw->dev, "FINISH HW RESET");
	}

	return 0;
}

static const struct rtl837x_sdsmode_map _rtl837x_sdsmode[] = {
	{ SERDES_10GQXG, "10g-qxg" },
	{ SERDES_10GUSXG, "10g-usxg" },
	{ SERDES_10GR, "10g-kr" },
	{ SERDES_HSG, "hsgmii" },
	{ SERDES_2500BASEX, "2500base-x" },
	{ SERDES_SG, "sgmii" },
	{ SERDES_1000BASEX, "1000base-x" },
	{ SERDES_100FX, "100base-fx" },
	{ SERDES_8221B, "8221b" },
};

static int rtl837x_sdsmode(const char *name, rtk_sds_mode_t *mode)
{
	int i;

	for (i=0; i<ARRAY_SIZE(_rtl837x_sdsmode); i++)
	{
		if (!strcmp(name, _rtl837x_sdsmode[i].name))
		{
			*mode = _rtl837x_sdsmode[i].mode;
			return 0;
		}
	}

	return -1;
}

static int rtl8372n_igmp_init(struct rtk_gsw *gsw)
{

	unsigned int ret;
	ret = rtk_igmp_init();
	if (ret) return ret;

	ret = rtk_igmp_state_set(TRUE);
	if (ret) return ret;

	ret = rtk_igmp_fastLeave_set(TRUE);
	if (ret) return ret;

	if(gsw->num_ports > 0)
	{
		for(int port = 0;port < gsw->num_ports;port++){
			rtk_uint32 phy_port = PORT_MAPPED(port);
			ret = rtk_igmp_maxGroup_set(phy_port, 255);
			if (ret)
			{
				dev_err(gsw->dev, "rtk_igmp_maxGroup_set failed, error:%d\n",ret);
				return ret;
			}
		}
		
	}
	return rtk_igmp_suppressionEnable_set(TRUE, TRUE);
}

static int of_extra_init(struct rtk_gsw *gsw)
{
	struct device_node *node = gsw->dev->of_node;
	const __be32 *list;
	int size, data_len;
	u32 reg, mask, val;

	list = of_get_property(node, "extra-init", &size);
	if (!list || !size) return 0;

	data_len = size / (3*sizeof(__be32));
	for (int i=0; i<data_len; i++)
	{
		reg = be32_to_cpu(*list);
		list++;
		mask = be32_to_cpu(*list);
		list++;
		val = be32_to_cpu(*list);
		list++;
		// dev_info(gsw->dev, "of_extra_init: reg:0x%X mask:0x%X val:0x%X\n", 
		// 					reg, mask, val);
		rtl8373_setAsicRegBits(reg, mask, val);
	}
	return 0;
}

int rtl8372n_hw_init(struct rtk_gsw *gsw, rtl837x_pnswap_cfg_t swap_cfg)
{

	unsigned int ret;
	rtl837x_hw_reset(gsw);
	ret = rtl837x_switch_probe(gsw);
	if(ret){
		dev_err(gsw->dev, "rtl837x_switch_probe Fail, error:%d\n", ret);
		return -EPERM;
	}

	// Sx PN swap:
    // 	RX:
    // 		page0 reg0 bit9:1
    // 		page6 reg2 bit13:1
    // 	TX:
    // 		page0 reg0 bit8:1
    // 		page6 reg2 bit14:1
	if (swap_cfg.sds0_rx_swap)
	{
		gsw->pMapper->rtl8373_sds_regbits_write(0, 0, 0, 0x200, 1); //#SDS0RX PN swap
		gsw->pMapper->rtl8373_sds_regbits_write(0, 6, 2, 0x2000, 1);
	}

	if (swap_cfg.sds0_tx_swap)
	{
		gsw->pMapper->rtl8373_sds_regbits_write(0, 0, 0, 1 << 8, 1); //#SDS0RTX PN swap
		gsw->pMapper->rtl8373_sds_regbits_write(0, 6, 2, 1 << 14, 1);
	}

	if (swap_cfg.sds1_rx_swap)
	{
		gsw->pMapper->rtl8373_sds_regbits_write(1, 0, 0, 0x200, 1); //#SDS1RX PN swap
		gsw->pMapper->rtl8373_sds_regbits_write(1, 6, 2, 0x2000, 1);
	}

	if (swap_cfg.sds1_tx_swap)
	{
		gsw->pMapper->rtl8373_sds_regbits_write(1, 0, 0, 1 << 8, 1); //#SDS1TX PN swap
		gsw->pMapper->rtl8373_sds_regbits_write(1, 6, 2, 1 << 14, 1);
	}

    // ##MDI reverse configuration for Demo Tap UP RJ45, RTL8366U/RTL8373N/RTL8372N
	if (swap_cfg.phy_mdi_reverse)
		gsw->pMapper->rtl8373_setAsicRegBits(RTL8373_CFG_PHY_MDI_REVERSE_ADDR, 0xF, 0xC);

	if (swap_cfg.phy_tx_polarity_swap)
    	gsw->pMapper->rtl8373_setAsicRegBits(RTL8373_CFG_PHY_TX_POLARITY_SWAP_ADDR, 0xFFFF, 0x596A); //#TX_POLARITY_SWAP

	ret = rtk_switch_init();
	if(ret){
		dev_err(gsw->dev, "rtk_switch_init Fail, error:%d\n", ret);
		return -EPERM;
	}

	of_extra_init(gsw);

	ret = rtk_vlan_reset();
	if (ret)
	{
		dev_err(gsw->dev, "rtk_vlan_reset failed, error:%d\n", ret);
		return -EPERM;
	}

	ret = rtk_vlan_init();
	if (ret)
	{
		dev_err(gsw->dev, "rtk_vlan_init failed, error:%d\n", ret);
		return -EPERM;
	}

	ret = rtl8372n_igmp_init(gsw);
	if (ret)
	{
		dev_err(gsw->dev, "rtl8372n_igmp_init failed, error:%d\n", ret);
		return -EPERM;
	}

	rtk_rmaParam_t pRmacfg;
	ret = rtk_rma_get(2, &pRmacfg);
	if ( ret )
	{
		dev_err(gsw->dev, "rtk_rma_get get rma failed, error:%d\n", ret);
	return -EPERM;
	}

	pRmacfg.operation = RMAOP_FORWARD; // 清零配置
	ret = rtk_rma_set(2, &pRmacfg);
	if ( ret )
	{
		dev_err(gsw->dev, "rtk_rma_get set rma failed, error:%d\n", ret);
		return -EPERM;
	}

	for(int port = 0;port < gsw->num_ports;port++){
		ret = rtk_eee_portTxRxEn_set(PORT_MAPPED(port), 0u, 0u);
		if (ret)
		{
			dev_err(gsw->dev, "rtk_eee_portTxRxEn_set failed, error:%d\n",ret);
			return -EPERM;
		}
	}

	ret = rtk_sdsMode_set(0, SERDES_10GR);
	if (ret) return -EPERM;
	ret = rtk_sdsMode_set(0, gsw->sds0mode);
	if (ret) return -EPERM;

	ret = rtk_sdsMode_set(1, SERDES_10GR);
	if (ret) return -EPERM;
	ret = rtk_sdsMode_set(1, gsw->sds1mode);
	if (ret) return -EPERM;

	ret = rtk_cpu_externalCpuPort_set(PORT_MAPPED(gsw->cpu_port));
	if (ret)
	{
		dev_err(gsw->dev, "rtk_cpu_externalCpuPort_set failed, error:%d\n",ret);
		return -EPERM;
	}

	// TODO
	// res = rtl8372n_igrAcl_init();
	// if (res != RT_ERR_OK){
	// 	dev_err(gsw->dev, "ACL init failed, ret=%d\n", res);
	// 	return res;
	// }

	// TODO
	// res = rtl837x_acl_add_u(a1);
	// if (res != RT_ERR_OK){
	// 	dev_err(gsw->dev, "rtl837x_acl_add failed, ret=%d\n", res);
	// 	return res;
	// }

	return 0;
}

static void rtl837x_status_check_work_func(struct work_struct *work)
{
	struct rtk_gsw *gsw = container_of(work, struct rtk_gsw, status_check_work.work);

	rtk_port_status_t port_status;

	rtk_port_macStatus_get(PORT_MAPPED(gsw->cpu_port), &port_status);
	if (!port_status.link)
	{
		if (PORT_MAPPED(gsw->cpu_port) != UTP_PORT3 && PORT_MAPPED(gsw->cpu_port) != UTP_PORT8)
		{
			dev_warn(gsw->dev, "CPU Port Down, But the CPU port is not Serdes Port, Skip Reset and stop CPU port check work\n");
			return;
		}

		dev_info(gsw->dev, "CPU Port Down, Try to reset Serdes, mode: 0x%x\n", gsw->sds0mode);

		rtnl_lock();
		dev_close(gsw->ethernet_master);
		rtnl_unlock();
		rtk_sdsMode_set(0, SERDES_OFF);
		mdelay(200);

		rtnl_lock();
		dev_open(gsw->ethernet_master, NULL);
		rtnl_unlock();
		rtk_sdsMode_set(0, gsw->sds0mode);

		mdelay(2000);
	}

	queue_delayed_work_on(smp_processor_id(), 
						system_wq, 
						&gsw->status_check_work, 
						msecs_to_jiffies(gsw->default_work_delay_ms)
					);
}

/* unused */
static void rtl837x_sfp_attach(void *upstream, struct sfp_bus *bus)
{
	struct rtk_gsw *gsw = upstream;
	dev_info(gsw->dev, "SFP module attach\n");
}

/* unused */
static void rtl837x_sfp_detach(void *upstream, struct sfp_bus *bus)
{
	struct rtk_gsw *gsw = upstream;
	dev_info(gsw->dev, "SFP module detach\n");
}

static int rtl837x_sfp_module_insert(void *upstream, const struct sfp_eeprom_id *id)
{
	struct rtk_gsw *gsw = upstream;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(support) = { 0, };
	DECLARE_PHY_INTERFACE_MASK(interfaces);
	phy_interface_t iface;

	sfp_parse_support(gsw->sfp_bus, id, support, interfaces);
	iface = sfp_select_interface(gsw->sfp_bus, support);

	dev_info(gsw->dev, "%s SFP module inserted\n", phy_modes(iface));

	switch (iface) {
	case PHY_INTERFACE_MODE_10GBASER:
		gsw->sds1mode = SERDES_10GR;
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		gsw->sds1mode = SERDES_2500BASEX;
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_SGMII:
		gsw->sds1mode = SERDES_1000BASEX;
		break;
	case PHY_INTERFACE_MODE_100BASEX:
		gsw->sds1mode = SERDES_100FX;
		break;
	default:
		dev_err(gsw->dev, "Incompatible SFP module inserted\n");
		return -EINVAL;
	}

	rtk_sdsMode_set(1, gsw->sds1mode);
	return 0;
}

static void rtl837x_sfp_module_remove(void *upstream)
{
	struct rtk_gsw *gsw = upstream;
	dev_info(gsw->dev, "SFP module remove\n");

	gsw->sds1mode = SERDES_OFF;
	rtk_sdsMode_set(1, gsw->sds1mode);
}

static const struct sfp_upstream_ops sfp_ops = {
	.attach = rtl837x_sfp_attach,
	.detach = rtl837x_sfp_detach,
	.module_insert = rtl837x_sfp_module_insert,
	.module_remove = rtl837x_sfp_module_remove,
	// .module_start = rtl837x_sfp_module_start,
	// .module_stop = rtl837x_sfp_module_stop,
	// .link_up = rtl837x_sfp_link_up,
	// .link_down = rtl837x_sfp_link_down,
	// .connect_phy = rtl837x_sfp_connect_phy,
	// .disconnect_phy = rtl837x_sfp_disconnect_phy,
};

static int rtl837x_sfp_probe(struct rtk_gsw *gsw)
{
	int ret;

	struct sfp_bus *bus = sfp_bus_find_fwnode(gsw->dev->fwnode);
	if (IS_ERR(bus))
	{
		dev_err(gsw->dev, "unable to attach SFP bus: %pe\n", bus);
		return PTR_ERR(bus);
	}

	gsw->sfp_bus = bus;

	ret = sfp_bus_add_upstream(bus, gsw, &sfp_ops);
	sfp_bus_put(bus);

	return ret;
}

static int rtl837x_status_check_work_init(struct rtk_gsw *gsw)
{
	gsw->default_work_delay_ms = 1000;
	INIT_DELAYED_WORK(&gsw->status_check_work, rtl837x_status_check_work_func);
	queue_delayed_work_on(smp_processor_id(), 
						system_wq, 
						&gsw->status_check_work, 
						msecs_to_jiffies(gsw->default_work_delay_ms)
					);
	return 0;
}

// below are platform driver
static const struct of_device_id rtk_gsw_match[] = {
	{ .compatible = "realtek,rtl837x" },
	{},
};

MODULE_DEVICE_TABLE(of, rtk_gsw_match);

static int rtl837x_gsw_probe(struct mdio_device *mdiodev)
{
	struct device *dev =&mdiodev->dev;
	struct device_node *np = dev->of_node;
	struct rtk_gsw *gsw;
	struct device_node *ethernet;
	struct net_device *master;
	const char *sdsmode_name;
	rtk_sds_mode_t sdsmode;
	struct regmap_config rc;
	
	int ret;
	dev_info(dev,"start rtl837x_gsw_probe");

	ethernet = of_parse_phandle(np, "ethernet", 0);
	if (!ethernet) 
		return -EINVAL;

	master = of_find_net_device_by_node(ethernet);
	of_node_put(ethernet);
	if (!master)
		return -EPROBE_DEFER;

	gsw = devm_kzalloc(dev, sizeof(struct rtk_gsw), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;	

	mutex_init(&gsw->map_lock);
	
	rc = rtl837x_mdio_regmap_config;
	rc.lock_arg = gsw;
	gsw->map = devm_regmap_init(dev, NULL, gsw, &rc);
	if (IS_ERR(gsw->map)) {
		ret = PTR_ERR(gsw->map);
		dev_err(dev, "regmap init failed: %d\n", ret);
		return -EINVAL;
	}

	rc = rtl837x_mdio_nolock_regmap_config;
	gsw->map_nolock = devm_regmap_init(dev, NULL, gsw, &rc);
	if (IS_ERR(gsw->map_nolock)) {
		ret = PTR_ERR(gsw->map_nolock);
		dev_err(dev, "regmap init failed: %d\n", ret);
		return -EINVAL;
	}

	gsw->dev = dev;
	gsw->bus = mdiodev->bus;
	gsw->ethernet_master = master;
	gsw->sds0mode = SERDES_OFF;
	gsw->sds1mode = SERDES_OFF;

	gsw->reset_pin = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(gsw->reset_pin)) {
		dev_warn(dev, "failed to get RESET GPIO!!!\n");
	}

	if (of_property_read_u32(np, "rtl837x,cpu-port", &gsw->cpu_port)) {
		dev_err(gsw->dev, "failed to get cpu port\n");
		devm_kfree(dev, gsw);
		return ret;
	}

	if (!of_property_read_string(np, "rtl837x,sds0mode", &sdsmode_name) &&
			!rtl837x_sdsmode(sdsmode_name, &sdsmode))
		gsw->sds0mode = sdsmode;

	if (!of_property_read_string(np, "rtl837x,sds1mode", &sdsmode_name) &&
			!rtl837x_sdsmode(sdsmode_name, &sdsmode))
		gsw->sds1mode = sdsmode;

	memset(&(gsw->swap_cfg),0,sizeof(rtl837x_pnswap_cfg_t));
	if (of_property_read_bool(np, "sds0-rx-swap"))
		gsw->swap_cfg.sds0_rx_swap = 1;
	if (of_property_read_bool(np, "sds0-tx-swap"))
		gsw->swap_cfg.sds0_tx_swap = 1;
	if (of_property_read_bool(np, "sds1-rx-swap"))
		gsw->swap_cfg.sds1_rx_swap = 1;
	if (of_property_read_bool(np, "sds1-tx-swap"))
		gsw->swap_cfg.sds1_tx_swap = 1;

	if (of_property_read_bool(np, "phy-mdi-reverse"))
		gsw->swap_cfg.phy_mdi_reverse = 1;
	if (of_property_read_bool(np, "phy-tx-polarity-swap"))
		gsw->swap_cfg.phy_tx_polarity_swap = 1;

	gsw->mdio_addr = mdiodev->addr;
	gsw->mib_counters = rtl837x_mib_counters;
	gsw->num_mib_counters = ARRAY_SIZE(rtl837x_mib_counters);

	dev_info(gsw->dev, "rtl837x dev info:smi-addr:%d cpu_port:%d serdes-mode:%d swap_cfg:0x%x\n",
						 gsw->mdio_addr, gsw->cpu_port, gsw->sds0mode, *(uint8_t*)&(gsw->swap_cfg));

	dev_set_drvdata(dev, gsw);
	gsw_regmap = gsw->map;

	ret = rtl8372n_hw_init(gsw, gsw->swap_cfg);
	if (ret)
	{
		dev_err(gsw->dev, "rtl8372n_hw_init failed, ret=%d\n",ret);
		devm_kfree(dev, gsw);
		return -ENODEV;
	}

	ret = rtl837x_swconfig_init(gsw);
	if (ret){
		dev_err(gsw->dev, "rtl837x_swconfig_init failed, ret=%d\n", ret);
		devm_kfree(dev, gsw);
		return ret;
	}

#ifdef CONFIG_GPIOLIB
	if (of_property_read_bool(np, "gpio-controller")) 
		rtl837x_gpiochip_init(gsw);
#endif /* CONFIG_GPIOLIB */

	rtl837x_sfp_probe(gsw);

	rtl837x_debug_proc_init();
	rtl837x_status_check_work_init(gsw);
	return 0;
}

static void rtl837x_gsw_remove(struct mdio_device *mdiodev)
{
	struct rtk_gsw *gsw = dev_get_drvdata(&mdiodev->dev);;
	cancel_delayed_work_sync(&gsw->status_check_work);
	unregister_switch(&gsw->sw_dev);
	rtl837x_debug_proc_deinit();
}

static void rtl837x_gsw_shutdown(struct mdio_device *mdiodev)
{
	struct rtk_gsw *gsw = dev_get_drvdata(&mdiodev->dev);

	if (!gsw)
		return;

	dev_set_drvdata(&mdiodev->dev, NULL);
}

static struct mdio_driver rtl837x_mdio_driver = {
	.mdiodrv.driver = {
		.name = "rtl837x-gsw",
		.of_match_table = rtk_gsw_match,
	},
	.probe  = rtl837x_gsw_probe,
	.remove = rtl837x_gsw_remove,
	.shutdown = rtl837x_gsw_shutdown,
};
mdio_module_driver(rtl837x_mdio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("StarField Xu <air_jinkela@163.com>");
MODULE_DESCRIPTION("rtl8372n switch driver for MT7988");
MODULE_ALIAS("platform:rtl837x-gpio");
