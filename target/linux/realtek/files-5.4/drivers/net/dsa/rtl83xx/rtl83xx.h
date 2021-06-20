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
void rtl_table_read(struct table_reg *r, int idx);
void rtl_table_write(struct table_reg *r, int idx);
inline u16 rtl_table_data(struct table_reg *r, int i);
inline u32 rtl_table_data_r(struct table_reg *r, int i);
inline void rtl_table_data_w(struct table_reg *r, u32 v, int i);

void __init rtl83xx_setup_qos(struct rtl838x_switch_priv *priv);
int read_phy(u32 port, u32 page, u32 reg, u32 *val);
int write_phy(u32 port, u32 page, u32 reg, u32 val);

/* Port register accessor functions for the RTL839x and RTL931X SoCs */
void rtl839x_mask_port_reg_be(u64 clear, u64 set, int reg);
u64 rtl839x_get_port_reg_be(int reg);
void rtl839x_set_port_reg_be(u64 set, int reg);
void rtl839x_mask_port_reg_le(u64 clear, u64 set, int reg);
void rtl839x_set_port_reg_le(u64 set, int reg);
u64 rtl839x_get_port_reg_le(int reg);

/* Port register accessor functions for the RTL838x and RTL930X SoCs */
void rtl838x_mask_port_reg(u64 clear, u64 set, int reg);
void rtl838x_set_port_reg(u64 set, int reg);
u64 rtl838x_get_port_reg(int reg);

/* RTL838x-specific */
u32 rtl838x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl838x_switch_irq(int irq, void *dev_id);
void rtl8380_get_version(struct rtl838x_switch_priv *priv);
void rtl838x_vlan_profile_dump(int index);
int rtl83xx_dsa_phy_read(struct dsa_switch *ds, int phy_addr, int phy_reg);
void rtl8380_sds_rst(int mac);
int rtl8380_sds_power(int mac, int val);
void rtl838x_print_matrix(void);

/* RTL839x-specific */
u32 rtl839x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl839x_switch_irq(int irq, void *dev_id);
void rtl8390_get_version(struct rtl838x_switch_priv *priv);
void rtl839x_vlan_profile_dump(int index);
int rtl83xx_dsa_phy_write(struct dsa_switch *ds, int phy_addr, int phy_reg, u16 val);
void rtl839x_exec_tbl2_cmd(u32 cmd);
void rtl839x_print_matrix(void);

/* RTL930x-specific */
u32 rtl930x_hash(struct rtl838x_switch_priv *priv, u64 seed);
irqreturn_t rtl930x_switch_irq(int irq, void *dev_id);
irqreturn_t rtl839x_switch_irq(int irq, void *dev_id);
void rtl930x_vlan_profile_dump(int index);
int rtl9300_sds_power(int mac, int val);
void rtl9300_sds_rst(int sds_num, u32 mode);
void rtl930x_print_matrix(void);

/* RTL931x-specific */
irqreturn_t rtl931x_switch_irq(int irq, void *dev_id);

#endif /* _NET_DSA_RTL83XX_H */

