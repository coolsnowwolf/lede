/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 StarField Xu <air_jinkela@163.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/switch.h>
#include "./rtl837x_common.h"

#include <linux/printk.h>

#ifndef SWITCH_PORT_SPEED_10000
#define SWITCH_PORT_SPEED_10000 10000
#endif

#ifndef SWITCH_PORT_SPEED_2500
#define SWITCH_PORT_SPEED_2500 2500
#endif

#ifndef SWITCH_PORT_SPEED_5000
#define SWITCH_PORT_SPEED_5000 5000
#endif

static int rtl837x_sw_get_port_stats(struct switch_dev *dev, int port,struct switch_port_stats *stats)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	rtk_stat_port_get(PORT_MAPPED(port), 0, &(stats->tx_bytes));             // tx_bytes
	rtk_stat_port_get(PORT_MAPPED(port), 2u, &(stats->rx_bytes));                // rx_bytes
	return 0;
}

static rtk_api_ret_t _phy_on(rtk_uint32 port)
{	
	rtk_api_ret_t ret;
	rtk_uint32 data;
	//READ, PHY_MMD_PCS (3) -> PCS control 1 (0)
	ret = rtk_port_phyReg_get(port, PHY_MMD_PCS, 0, &data);
	if (ret == RT_ERR_OK)
		data &= 0xFFFFF7FF; // Low power (BIT11)
	ret = rtk_port_phyReg_set(1 << port, PHY_MMD_PCS, 0, data);
	return ret;
}

static rtk_api_ret_t _phy_off(rtk_uint32 port)
{	
	rtk_api_ret_t ret;
	rtk_uint32 data;
	//READ, PHY_MMD_PCS (3) -> PCS control 1 (0)
	ret = rtk_port_phyReg_get(port, PHY_MMD_PCS, 0, &data);
	if (ret == RT_ERR_OK)
		data |= 0x800; // Low power (BIT11)
	ret = rtk_port_phyReg_set(1 << port, PHY_MMD_PCS, 0, data);
	return ret;
}

/**
 * @brief 应用交换机配置
 * 
 * @param switch_dev 交换机设备结构指针
 * @return int 返回状态码 (0 = 成功)
 */
static int rtl837x_sw_apply_config(struct switch_dev *swdev)
{
	struct rtk_gsw *gsw = container_of(swdev, struct rtk_gsw, sw_dev);

	int ret = 0;

	dev_info(gsw->dev, "rtl837x Apply Config\n");
	
	// ====================== 1. 应用流控配置 ======================
	dev_info(gsw->dev, "rtl837x Apply flow control\n");
	for (int port = 0; port < gsw->num_ports; port++) {
		// 跳过CPU端口和特定端口
		rtk_uint32 port_mapped = PORT_MAPPED(port);
		if (port_mapped != UTP_PORT3 && port_mapped != UTP_PORT8) {
			rtk_port_phy_ability_t ana = {
				.Half_10 = 1,
				.Full_10 = 1,
				.Half_100 = 1,
				.Full_100 = 1,
				.Half_1000 = 1,
				.Full_1000 = 1,
				.adv_2_5G = 1,
				.adv_5G = 0,
				.adv_10GBase_T = 0,
				.FC = 0,
				.AsyFC = 0,
			};
			if ((gsw->flow_control_map >> port_mapped) & 1)
			{
				ana.FC = 1;
				ana.AsyFC = 1;
			}
			// 设置端口自动协商能力
			_phy_off(port_mapped);
			ret = rtk_phy_common_c45_autoSpeed_set(port_mapped, &ana);
			_phy_on(port_mapped);
			if (ret) {
				dev_err(gsw->dev, "Port %d autoNegoAbility configure Failed: %d", port, ret);
				return -EIO;
			}
		}
	}

	// ====================== 2. 应用VLAN配置 ======================
	if (gsw->global_vlan_enable)
	{
		dev_info(gsw->dev, "rtl837x Apply Vlan config\n");
		// 重置VLAN配置
		rtk_vlan_reset();
		
		// 设置所有VLAN条目
		for (int vlan_id = 0; vlan_id < swdev->vlans; vlan_id++) {
			if (gsw->vlan_table[vlan_id].valid == 1) {
				rtk_vlan_entry_t vlan_cfg = {
					.mbr.bits[0] = gsw->vlan_table[vlan_id].mbr,
					.untag.bits[0] = gsw->vlan_table[vlan_id].untag,
					.svlan_chk_ivl_svl = 0,
					.fid_msti = 0,
					.ivl_svl = 1,
				};
				dev_info(gsw->dev, "rtl837x VLAN mbr:%u\tntag:%u\n",vlan_cfg.mbr.bits[0], vlan_cfg.untag.bits[0]);
				
				ret = rtk_vlan_set(vlan_id, &vlan_cfg);
				if (ret) {
					dev_err(gsw->dev, "VLAN %d configure Failed: %d", vlan_id, ret);
					return -EIO;
				}
			}
		}

		dev_info(gsw->dev, "rtl837x Apply PVID\n");
		// ====================== 3. 应用PVID配置 ======================
		for (int port = 0; port < swdev->ports; port++) {
			ret = rtk_vlan_portPvid_set(
				PORT_MAPPED(port), 
				gsw->port_pvid[port]
			);
			
			if (ret) {
				dev_err(gsw->dev, "port %d PVID configure Failed: %d", port, ret);
				return -EIO;
			}
		}
	}
	// ====================== 4. 应用端口隔离配置 ======================
	else
	{
		// 获取CPU端口的物理端口号
		rtk_uint32 isolation_map = 0;
		
		// 构建隔离映射
		for (int port = 0; port < swdev->ports; port++) {
			// 跳过CPU端口
			if (port != swdev->cpu_port) {
				
				// 添加端口到隔离映射
				isolation_map |= (1 << PORT_MAPPED(port));
				
				// 设置端口隔离
				ret = rtk_port_isolation_set(PORT_MAPPED(port), (1 << PORT_MAPPED(swdev->cpu_port)));
				if (ret) {
					dev_err(gsw->dev, "Port %d isolation configure Failed: %d", port, ret);
					return -EIO;
				}
			}
		}
		
		// 设置CPU端口的隔离
		ret = rtk_port_isolation_set(PORT_MAPPED(swdev->cpu_port), isolation_map);
		if (ret) {
			dev_err(gsw->dev, "CPU port isolation configure Failed: %d", ret);
			return -EIO;
		}
	}
	return 0;
}

static int rtl837x_sw_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	val->len = 0;
	if(!(gsw->vlan_table[val->port_vlan].valid)) return 0;
	rtk_vlan_entry_t vlan_cfg;
	if (rtk_vlan_get(val->port_vlan, &vlan_cfg)) return -EINVAL;
	if (!vlan_cfg.ivl_svl) return 0; //跳过下面的多余循环

	struct switch_port *port = &val->value.ports[0];
	for(int i = 0;i < gsw->num_ports;i++){
		if (!(vlan_cfg.mbr.bits[0] & BIT(PORT_MAPPED(i)))) continue;

		port->id = i;
		port->flags = (vlan_cfg.untag.bits[0] & BIT(PORT_MAPPED(i))) ? 0 : BIT(SWITCH_PORT_FLAG_TAGGED);
		val->len++;
		port++;
	}

	return 0;
}

/**
 * @brief 设置 VLAN 的端口成员
 * 
 * @param switch_dev 交换机设备结构指针
 * @param vlan_val VLAN 值结构指针
 * @return 返回状态码 (0 = 成功)
 */
static int rtl837x_sw_set_vlan_ports(struct switch_dev *dev, struct switch_val *vlan_val)
{
	// 获取 VLAN ID
	rtk_uint32 vlan_id = vlan_val->port_vlan;
	
	// 验证 VLAN ID 范围 (1-4094)
	if (vlan_id < 1 || vlan_id > 4094) return -EINVAL;
	
	// 获取设备数据
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	
	// 验证端口数量
	uint32_t port_count = vlan_val->len;
	

	rtk_uint32 vlan_mbr = 0;      // 所有成员端口位图
	rtk_uint32 vlan_untag = 0; // 未标记端口位图
	
	// 处理每个端口
	if (port_count > 0) {
		struct switch_port *port_list = vlan_val->value.ports;
		
		for (uint32_t i = 0; i < port_count; i++) {
			// 获取物理端口号
			rtk_uint32 physical_port = port_list[i].id;
			
			// 计算端口位掩码
			rtk_uint32 port_mask = BIT(PORT_MAPPED(physical_port));
			
			// 添加到所有端口位图
			vlan_mbr |= port_mask;
			
			// 如果是未标记端口，添加到未标记位图
			if (!(port_list[i].flags & BIT(SWITCH_PORT_FLAG_TAGGED))) {
				vlan_untag |= port_mask;
			}
		}
	}
	
	// 更新 VLAN 配置
	gsw->vlan_table[vlan_id].vid = vlan_id;
	gsw->vlan_table[vlan_id].mbr = vlan_mbr;
	gsw->vlan_table[vlan_id].untag = vlan_untag;
	gsw->vlan_table[vlan_id].valid = 1;
	dev_info(gsw->dev, "vlanid:%u\tportmap:%016x\tuntag:%016x\tvalid:%u\n",gsw->vlan_table[vlan_id].vid, gsw->vlan_table[vlan_id].mbr, gsw->vlan_table[vlan_id].untag, gsw->vlan_table[vlan_id].valid);
	// rtl837x_apply_config(dev);
	return 0;
}

static int rtl837x_sw_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	int result; // x0
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	if (port > gsw->num_ports) return -EINVAL;

	result = rtk_vlan_portPvid_get(PORT_MAPPED(port), val);
	if ( result )
	{
		dev_err(gsw->dev, "%s: rtk_vlan_portPvid_get failed, ret=%d\n", "rtl837x_get_port_pvid", result);
		return -EINVAL;
	}
	return 0;
}

static int rtl837x_sw_set_port_pvid(struct switch_dev *dev, int port, int val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	if (port > gsw->num_ports) return -22;

	gsw->port_pvid[port] = val;

	return 0;
}

/**
 * @brief 转换速度代码为具体速率值
 * 
 * @param speed_code 硬件速度代码
 * 
 * @return uint32_t 实际速率值 (Mbps)
 */
static uint32_t convert_speed_code(uint32_t speed_code)
{
	switch (speed_code) {
		case 0:		return SWITCH_PORT_SPEED_10;
		case 1:   	return SWITCH_PORT_SPEED_100;
		case 2:     return SWITCH_PORT_SPEED_1000;
		case 4:    	return SWITCH_PORT_SPEED_10000;
		case 5:    	return SWITCH_PORT_SPEED_2500;
		case 6:    	return SWITCH_PORT_SPEED_5000;
		default:    return SWITCH_PORT_SPEED_UNKNOWN; // 未知状态
	}
}

static int rtl837x_sw_get_port_link_status(struct switch_dev *dev, int port, struct switch_port_link *link)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	ret_t ret;
	// 检查端口有效性
	if (port >= gsw->num_ports) {
		dev_err(gsw->dev, "Invalid Port: %u", port);
		return -EINVAL;
	}

	// 获取MAC状态信息
	rtk_port_status_t port_status;
	ret = rtk_port_macStatus_get(PORT_MAPPED(port), &port_status);
	if(ret)
	{
		dev_err(gsw->dev, 
				"get port:%u MAC status Failed: %d", 
				port, ret);
		return -EINVAL;
	}

	// 获取自动协商状态（特殊端口除外）
	rtk_uint32 auto_neg_value;

	if (!((PORT_MAPPED(port) == 3) || (PORT_MAPPED(port) == 8))) {
		ret = rtk_phy_common_c45_autoNegoEnable_get(PORT_MAPPED(port), &auto_neg_value);
		if (ret != RT_ERR_OK) {
			dev_err(gsw->dev, 
					"get port:%u autoNegoAbility status Failed: %d", 
					port, auto_neg_value);
			return -EIO;
		}
	}

	// 解析并填充链路状态
	struct switch_port_link result = {
		.link = (port_status.link != 0),
		.duplex = (port_status.duplex != 0),
		.aneg = (auto_neg_value != 0),
		.tx_flow = (port_status.txpause != 0),
		.rx_flow = (port_status.rxpause != 0),
		.speed = convert_speed_code(port_status.speed)
	};

	*link = result;
	return 0;
}

static int rtl837x_sw_set_port_link(struct switch_dev *dev, int port, struct switch_port_link *link)
{
	return 0;
}

static int rtl837x_sw_reset_switch(struct switch_dev *dev)
{
	return 0;
}

static int rtl837x_sw_set_vlan_enable(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	gsw->global_vlan_enable = val->value.i;
	return 0;
}

static int rtl837x_sw_get_port_flowcontrol(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	rtk_uint32 port;
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	port = val->port_vlan;
	if (port >= gsw->num_ports) return -EINVAL;

	val->value.i = (gsw->flow_control_map >> PORT_MAPPED(port)) & 0x1;
	return 0;
}

static int rtl837x_sw_set_port_flowcontrol(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	rtk_uint32 port;
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	port = val->port_vlan;
	if (port >= gsw->num_ports) return -EINVAL;

	rtk_uint32 port_mapped = PORT_MAPPED(port);
	if (port_mapped == UTP_PORT3 ||
		port_mapped != UTP_PORT8) return -EINVAL;

	rtk_uint32 enabled = val->value.i;
	if (enabled)
		gsw->flow_control_map |= BIT(port_mapped);
	else
		gsw->flow_control_map &= ~BIT(port_mapped);
	return 0;
}

static int rtl837x_sw_get_vlan_enable(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	val->value.i = gsw->global_vlan_enable;
	return 0;
}

static int rtl837x_sw_reset_mibs(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	// struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	rtk_stat_global_reset();
	return 0;
}

static int rtl837x_sw_reset_port_mibs(struct switch_dev *dev,const struct switch_attr *attr,struct switch_val *val)
{
	rtk_uint32 port;
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	port = val->port_vlan;
	if (port >= gsw->num_ports) return -EINVAL;

	rtk_stat_port_reset(PORT_MAPPED(port));

	return 0;
}

static int rtl837x_sw_get_port_mib(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{	
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	int i, len = 0;
	rtk_stat_counter_t counter = 0;
	char *buf = gsw->buf;

	if (val->port_vlan >= gsw->num_ports)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(gsw->buf) - len, "Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < gsw->num_mib_counters; ++i) {
		len += snprintf(buf + len, sizeof(gsw->buf) - len, "%-36s: ", gsw->mib_counters[i].name);

		if (!rtk_stat_port_get(PORT_MAPPED(val->port_vlan), gsw->mib_counters[i].base, &counter))
			len += snprintf(buf + len, sizeof(gsw->buf) - len, "%llu\n", counter);
		else
			len += snprintf(buf + len, sizeof(gsw->buf) - len, "%s\n", "error");
	}

	val->value.s = buf;
	val->len = len;
	return 0;
}

static int rtl837x_sw_reset_sdsx(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	if (val->value.i != 0 && val->value.i != 1)
		return -EINVAL;

	rtk_sds_mode_t mode = (val->value.i != 0) ? gsw->sds1mode : gsw->sds0mode;
	dev_info(gsw->dev, "Reset Serdes%d, mode: %x\n", val->value.i, mode);

	if (rtk_sdsMode_set(val->value.i, SERDES_OFF))
		return -EPERM;
	mdelay(20);

	if (rtk_sdsMode_set(val->value.i, mode))
		return -EPERM;
	mdelay(20);

	return 0;
}

static int rtl837x_sw_reset_hw(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
	struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	cancel_delayed_work_sync(&gsw->status_check_work);

	int ret = rtl8372n_hw_init(gsw, gsw->swap_cfg);
	if (ret)
	{
		dev_err(gsw->dev, "rtl8372n_hw_init failed, ret=%d\n",ret);
		return -ENODEV;
	}

	queue_delayed_work_on(smp_processor_id(), 
					system_wq, 
					&gsw->status_check_work, 
					msecs_to_jiffies(gsw->default_work_delay_ms)
				);

	rtl837x_sw_apply_config(dev);
	return 0;
}

static struct switch_attr rtl832n_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = rtl837x_sw_set_vlan_enable,
		.get = rtl837x_sw_get_vlan_enable,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = rtl837x_sw_reset_mibs,
	}, {
		.type = SWITCH_TYPE_INT,
		.name = "reset_serdes",
		.description = "Reset Serdes",
		.set = rtl837x_sw_reset_sdsx,
	}, {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_hw",
		.description = "HW reset switch",
		.set = rtl837x_sw_reset_hw,
	}
};

static struct switch_attr rtl837x_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl837x_sw_reset_port_mibs,
	},
	{
		.type = SWITCH_TYPE_INT,
		.name = "flowcontrol",
		.description = "set hw flow control",
		.set = rtl837x_sw_set_port_flowcontrol,
		.get = rtl837x_sw_get_port_flowcontrol,
	},
#ifdef CONFIG_RTL837x_GSW_PORT_MIB_FEATURE
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.set = NULL,
		.get = rtl837x_sw_get_port_mib,
	},
#endif
};

static const struct switch_dev_ops rtl8372n_sw_ops = {
	.attr_global = { .attr = rtl832n_globals, .n_attr = ARRAY_SIZE(rtl832n_globals)},
	.attr_port = { .attr = rtl837x_port, .n_attr = ARRAY_SIZE(rtl837x_port) },
	.attr_vlan = { .attr = NULL, .n_attr = 0 },

	.get_vlan_ports = rtl837x_sw_get_vlan_ports,
	.set_vlan_ports = rtl837x_sw_set_vlan_ports,

	.get_port_pvid = rtl837x_sw_get_port_pvid,
	.set_port_pvid = rtl837x_sw_set_port_pvid,
	
	.apply_config = rtl837x_sw_apply_config,
	.reset_switch = rtl837x_sw_reset_switch,

	.get_port_link = rtl837x_sw_get_port_link_status,
	.set_port_link = rtl837x_sw_set_port_link,

	.get_port_stats = rtl837x_sw_get_port_stats,
};

int rtl837x_swconfig_init(struct rtk_gsw *gsw)
{   

	struct switch_dev *dev = &gsw->sw_dev;
	int err;

	dev->name = "RTL8372n";
	dev->cpu_port = gsw->cpu_port;
	dev->ports = gsw->num_ports;
	dev->vlans = 4096;
	dev->ops = &rtl8372n_sw_ops;
	dev->alias = dev_name(gsw->dev);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(gsw->dev, "switch registration failed\n");

	gsw->global_vlan_enable = true; 
	gsw->flow_control_map = 0xF0; //启用所有端口的autoNegoAbility

	rtl837x_sw_apply_config(dev);
	return err;
}
