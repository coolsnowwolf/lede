#ifndef RTL8261D_H
#define RTL8261D_H

#include <linux/phy.h>

typedef struct rtk_autoNegoAbility_cfg {
	uint8_t a10base_te_half:1;
	uint8_t a10base_te_full:1;
	uint8_t a100base_tx_half:1;
	uint8_t a100base_tx_full:1;
	uint8_t a2500base_t:1;
	uint8_t a5000base_t:1;
	uint8_t a10Gbase_t:1;
	uint8_t apause:1;
	uint8_t pause:1;
	uint8_t unknow_bit4:1;

} rtk_autoNegoAbility_cfg_t;

typedef struct rtk_eee_cfg {
	uint8_t e100base_tx:1;
	uint8_t e1000base_tx:1;
	uint8_t e10Gbase_t:1;
	uint8_t e2500base_t:1;
	uint8_t e5000base_t:1;

} rtk_eee_cfg_t;

int rtl822x_hwmon_init(struct phy_device *phydev);

int rtl8261x_config_init(struct phy_device *phydev);
int rtl8261x_get_features(struct phy_device *phydev);
int rtl8261x_read_status(struct phy_device *phydev);

int rtl8261x_suspend(struct phy_device *phydev);
int rtl8261x_resume(struct phy_device *phydev);

int rtl8261x_config_aneg(struct phy_device *phydev);
int rtl8261x_aneg_done(struct phy_device *phydev);

int rtl8261x_ack_intr(struct phy_device *phydev);
irqreturn_t rtl8261x_handle_intr(struct phy_device *phydev);

int rtl8261x_set_loopback(struct phy_device *phydev, bool enable);

int rtl8261x_get_tunable(struct phy_device *phydev,
			   struct ethtool_tunable *tuna, void *data);

int rtl8261x_set_tunable(struct phy_device *phydev,
			    struct ethtool_tunable *tuna,
			    const void *data);

#endif