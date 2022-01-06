#ifndef __RTK_HAL_H__
#define __RTK_HAL_H__
#include "ra_ioctl.h"

#define RTK_SW_VID_RANGE        16
void rtk_hal_switch_init(void);
void rtk_hal_dump_mib(void);
void rtk_hal_dump_full_mib(void);
int rtk_hal_dump_vlan(void);
void rtk_hal_clear_vlan(void);
int rtk_hal_set_vlan(struct ra_switch_ioctl_data *data);
int rtk_hal_set_ingress_rate(struct ra_switch_ioctl_data *data);
int rtk_hal_set_egress_rate(struct ra_switch_ioctl_data *data);
void rtk_hal_dump_table(void);
void rtk_hal_clear_table(void);
void rtk_hal_get_phy_status(struct ra_switch_ioctl_data *data);
void rtk_hal_set_port_mirror(struct ra_switch_ioctl_data *data);
void rtk_hal_read_reg(struct ra_switch_ioctl_data *data);
void rtk_hal_write_reg(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_en(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_table2type(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_table2type(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_port2table(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_port2table(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_port2pri(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_port2pri(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_dscp2pri(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_dscp2pri(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_pri2queue(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_pri2queue(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_set_queue_weight(struct ra_switch_ioctl_data *data);
void rtk_hal_qos_get_queue_weight(struct ra_switch_ioctl_data *data);
void rtk_hal_enable_igmpsnoop(struct ra_switch_ioctl_data *data);
void rtk_hal_disable_igmpsnoop(void);
void rtk_hal_set_phy_test_mode(struct ra_switch_ioctl_data *data);
void rtk_hal_get_phy_reg(struct ra_switch_ioctl_data *data);
void rtk_hal_set_phy_reg(struct ra_switch_ioctl_data *data);
void rtk_hal_vlan_tag(struct ra_switch_ioctl_data *data);
void rtk_hal_vlan_portpvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);
void rtk_hal_add_table(struct ra_switch_ioctl_data *data);
void rtk_hal_del_table(struct ra_switch_ioctl_data *data);
void rtk_hal_vlan_mode(struct ra_switch_ioctl_data *data);
void rtk_hal_set_port_trunk(struct ra_switch_ioctl_data *data);
#endif
