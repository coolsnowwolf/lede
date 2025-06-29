/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _NET_DSA_RTL83XX_H
#define _NET_DSA_RTL83XX_H

#include <net/dsa.h>
#include "rtl838x.h"


#define RTL8380_VERSION_A 'A'
#define RTL8390_VERSION_A 'A'
#define RTL8380_VERSION_B 'B'

struct fdb_update_work {
	struct work_struct work;
	struct net_device *ndev;
	u64 macs[];
};

#define MIB_DESC(_size, _offset, _name) {.size = _size, .offset = _offset, .name = _name}
struct rtl83xx_mib_desc {
	unsigned int size;
	unsigned int offset;
	const char *name;
};

/* API for switch table access */
struct table_reg {
	u16 addr;
	u16 data;
	u8  max_data;
	u8 c_bit;
	u8 t_bit;
	u8 rmode;
	u8 tbl;
	struct mutex lock;
};

#define TBL_DESC(_addr, _data, _max_data, _c_bit, _t_bit, _rmode) \
		{  .addr = _addr, .data = _data, .max_data = _max_data, .c_bit = _c_bit, \
		    .t_bit = _t_bit, .rmode = _rmode \
		}

typedef enum {
	RTL8380_TBL_L2 = 0,
	RTL8380_TBL_0,
	RTL8380_TBL_1,
	RTL8390_TBL_L2,
	RTL8390_TBL_0,
	RTL8390_TBL_1,
	RTL8390_TBL_2,
	RTL9300_TBL_L2,
	RTL9300_TBL_0,
	RTL9300_TBL_1,
	RTL9300_TBL_2,
	RTL9300_TBL_HSB,
	RTL9300_TBL_HSA,
	RTL9310_TBL_0,
	RTL9310_TBL_1,
	RTL9310_TBL_2,
	RTL9310_TBL_3,
	RTL9310_TBL_4,
	RTL9310_TBL_5,
	RTL_TBL_END
} rtl838x_tbl_reg_t;

void rtl_table_init(void);
struct table_reg *rtl_table_get(rtl838x_tbl_reg_t r, int t);
void rtl_table_release(struct table_reg *r);
int rtl_table_read(struct table_reg *r, int idx);
int rtl_table_write(struct table_reg *r, int idx);
inline u16 rtl_table_data(struct table_reg *r, int i);
inline u32 rtl_table_data_r(struct table_reg *r, int i);
inline void rtl_table_data_w(struct table_reg *r, u32 v, int i);

void __init rtl83xx_setup_qos(struct rtl838x_switch_priv *priv);

void rtl83xx_fast_age(struct dsa_switch *ds, int port);
int rtl83xx_packet_cntr_alloc(struct rtl838x_switch_priv *priv);
int rtl83xx_port_get_stp_state(struct rtl838x_switch_priv *priv, int port);
int rtl83xx_port_is_under(const struct net_device * dev, struct rtl838x_switch_priv *priv);
void rtl83xx_port_stp_state_set(struct dsa_switch *ds, int port, u8 state);
int rtl83xx_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data);

int read_phy(u32 port, u32 page, u32 reg, u32 *val);
int write_phy(u32 port, u32 page, u32 reg, u32 val);

/* Port register accessor functions for the RTL839x and RTL931X SoCs */
void rtl839x_mask_port_reg_be(u64 clear, u64 set, int reg);
u32 rtl839x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
u64 rtl839x_get_port_reg_be(int reg);
void rtl839x_set_port_reg_be(u64 set, int reg);
void rtl839x_mask_port_reg_le(u64 clear, u64 set, int reg);
int rtl839x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);
void rtl839x_set_port_reg_le(u64 set, int reg);
u64 rtl839x_get_port_reg_le(int reg);

/* Port register accessor functions for the RTL838x and RTL930X SoCs */
void rtl838x_mask_port_reg(u64 clear, u64 set, int reg);
void rtl838x_set_port_reg(u64 set, int reg);
u32 rtl838x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
u64 rtl838x_get_port_reg(int reg);
int rtl838x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);

/* RTL838x-specific */
u32 rtl838x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl838x_switch_irq(int irq, void *dev_id);
void rtl8380_get_version(struct rtl838x_switch_priv *priv);
void rtl838x_vlan_profile_dump(int index);
void rtl8380_sds_rst(int mac);
int rtl8380_sds_power(int mac, int val);
void rtl838x_print_matrix(void);

/* RTL839x-specific */
u32 rtl839x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl839x_switch_irq(int irq, void *dev_id);
void rtl8390_get_version(struct rtl838x_switch_priv *priv);
void rtl839x_vlan_profile_dump(int index);
void rtl839x_exec_tbl2_cmd(u32 cmd);
void rtl839x_print_matrix(void);

/* RTL930x-specific */
u32 rtl930x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl930x_switch_irq(int irq, void *dev_id);
irqreturn_t rtl839x_switch_irq(int irq, void *dev_id);
void rtl930x_vlan_profile_dump(int index);
int rtl9300_sds_power(int mac, int val);
extern int rtl9300_serdes_setup(int port, int sds_num, phy_interface_t phy_mode);
void rtl930x_print_matrix(void);

/* RTL931x-specific */
irqreturn_t rtl931x_switch_irq(int irq, void *dev_id);
int rtl931x_sds_cmu_band_get(int sds, phy_interface_t mode);
int rtl931x_sds_cmu_band_set(int sds, bool enable, u32 band, phy_interface_t mode);
extern void rtl931x_sds_init(u32 sds, phy_interface_t mode);

int rtl83xx_lag_add(struct dsa_switch *ds, int group, int port, struct netdev_lag_upper_info *info);
int rtl83xx_lag_del(struct dsa_switch *ds, int group, int port);

/* phy functions that will need to be moved to the future mdio driver */

int rtl838x_read_mmd_phy(u32 port, u32 addr, u32 reg, u32 *val);
int rtl838x_write_mmd_phy(u32 port, u32 addr, u32 reg, u32 val);

int rtl839x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
int rtl839x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);

int rtl930x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
int rtl930x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);

int rtl931x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
int rtl931x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);

/*
 * TODO: The following functions are currently not in use. So compiler will complain if
 * they are static and not made available externally. To preserve them for future use
 * collect them in this section.
 */

void rtl838x_egress_rate_queue_limit(struct rtl838x_switch_priv *priv, int port,
				     int queue, u32 rate);

int rtl8390_sds_power(int mac, int val);
void rtl839x_pie_rule_dump(struct  pie_rule *pr);
void rtl839x_set_egress_queue(int port, int queue);

void rtl9300_dump_debug(void);
void rtl930x_pie_rule_dump_raw(u32 r[]);

void rtl931x_print_matrix(void);
void rtl931x_set_receive_management_action(int port, rma_ctrl_t type, action_type_t action);
void rtl931x_sw_init(struct rtl838x_switch_priv *priv);

#endif /* _NET_DSA_RTL83XX_H */
