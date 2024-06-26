// SPDX-License-Identifier: GPL-2.0
/* FILE NAME:  en8801sc.c
 * PURPOSE:
 *      EN8801SC phy driver for Linux
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/delay.h>

#include <linux/uaccess.h>
#include <linux/version.h>

#include "en8801sc.h"

MODULE_DESCRIPTION("Airoha EN8801S PHY drivers for MediaTek SoC");
MODULE_AUTHOR("Airoha");
MODULE_LICENSE("GPL");

#define airoha_mdio_lock(bus)   mutex_lock(&((bus)->mdio_lock))
#define airoha_mdio_unlock(bus) mutex_unlock(&((bus)->mdio_lock))

#define phydev_mdio_bus(_dev) (_dev->mdio.bus)
#define phydev_phy_addr(_dev) (_dev->mdio.addr)
#define phydev_dev(_dev) (&_dev->mdio.dev)
#define phydev_pbus_addr(dev) ((dev)->mdio.addr + 1)

enum {
	PHY_STATE_DONE = 0,
	PHY_STATE_INIT = 1,
	PHY_STATE_PROCESS = 2,
	PHY_STATE_FAIL = 3,
};

struct en8801s_priv {
	bool first_init;
	u16 count;
	u16 pro_version;
};

/*
The following led_cfg example is for reference only.
LED5 1000M/LINK/ACT   (GPIO5)  <-> BASE_T_LED0,
LED6 10/100M/LINK/ACT (GPIO9)  <-> BASE_T_LED1,
LED4 100M/LINK/ACT    (GPIO8)  <-> BASE_T_LED2,
*/
/* User-defined.B */
#define AIR_LED_SUPPORT
#ifdef AIR_LED_SUPPORT
static const struct AIR_BASE_T_LED_CFG_S led_cfg[4] = {
/*
*   {LED Enable,    GPIO,   LED Polarity,   LED ON, LED Blink}
*/
	/* BASE-T LED0 */
	{LED_ENABLE, 5, AIR_ACTIVE_LOW,
	 BASE_T_LED0_ON_CFG, BASE_T_LED0_BLK_CFG},
	/* BASE-T LED1 */
	{LED_ENABLE, 9, AIR_ACTIVE_LOW,
	 BASE_T_LED1_ON_CFG, BASE_T_LED1_BLK_CFG},
	/* BASE-T LED2 */
	{LED_ENABLE, 8, AIR_ACTIVE_LOW,
	 BASE_T_LED2_ON_CFG, BASE_T_LED2_BLK_CFG},
	/* BASE-T LED3 */
	{LED_DISABLE, 1, AIR_ACTIVE_LOW,
	 BASE_T_LED3_ON_CFG, BASE_T_LED3_BLK_CFG},
};
static const u16 led_dur = UNIT_LED_BLINK_DURATION << AIR_LED_BLK_DUR_64M;
#endif

/* User-defined.E */

/************************************************************************
*                  F U N C T I O N S
************************************************************************/
static int en8801s_phase2_init(struct phy_device *phydev);

static int __airoha_cl45_write(struct mii_bus *bus, int port,
			u32 devad, u32 reg, u16 val)
{
	int ret = 0;
	struct device *dev = &bus->dev;

	ret = __mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, devad);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = __mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, reg);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = __mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG,
				MMD_OP_MODE_DATA | devad);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = __mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, val);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}

	return ret;
}

static int __airoha_cl45_read(struct mii_bus *bus, int port,
			u32 devad, u32 reg, u16 *read_data)
{
	int ret = 0;
	struct device *dev = &bus->dev;

	ret = __mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG, devad);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = __mdiobus_write(bus, port, MII_MMD_ADDR_DATA_REG, reg);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	ret = __mdiobus_write(bus, port, MII_MMD_ACC_CTL_REG,
				MMD_OP_MODE_DATA | devad);
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return ret;
	}
	*read_data = __mdiobus_read(bus, port, MII_MMD_ADDR_DATA_REG);

	return ret;
}

static int airoha_cl45_write(struct mii_bus *bus, int port,
			u32 devad, u32 reg, u16 val)
{
	int ret = 0;

	airoha_mdio_lock(bus);
	ret = __airoha_cl45_write(bus, port, devad, reg, val);
	airoha_mdio_unlock(bus);

	return ret;
}

static int airoha_cl45_read(struct mii_bus *bus, int port,
			u32 devad, u32 reg, u16 *read_data)
{
	int ret = 0;

	airoha_mdio_lock(bus);
	ret = __airoha_cl45_read(bus, port, devad, reg, read_data);
	airoha_mdio_unlock(bus);

	return ret;
}

static int __airoha_pbus_write(struct mii_bus *ebus, int pbus_id,
			unsigned long pbus_address, unsigned long pbus_data)
{
	int ret = 0;

	ret = __mdiobus_write(ebus, pbus_id, 0x1F,
				(unsigned int)(pbus_address >> 6));
	if (ret < 0)
		return ret;
	ret = __mdiobus_write(ebus, pbus_id,
				(unsigned int)((pbus_address >> 2) & 0xf),
				(unsigned int)(pbus_data & 0xFFFF));
	if (ret < 0)
		return ret;
	ret = __mdiobus_write(ebus, pbus_id, 0x10,
				(unsigned int)(pbus_data >> 16));
	if (ret < 0)
		return ret;
	return ret;
}

static unsigned long __airoha_pbus_read(struct mii_bus *ebus, int pbus_id,
			unsigned long pbus_address)
{
	unsigned long pbus_data;
	unsigned int pbus_data_low, pbus_data_high;
	int ret = 0;
	struct device *dev = &ebus->dev;

	ret = __mdiobus_write(ebus, pbus_id, 0x1F,
				(unsigned int)(pbus_address >> 6));
	if (ret < 0) {
		dev_err(dev, "%s fail. (ret=%d)\n", __func__, ret);
		return INVALID_DATA;
	}
	pbus_data_low = __mdiobus_read(ebus, pbus_id,
				(unsigned int)((pbus_address >> 2) & 0xf));
	pbus_data_high = __mdiobus_read(ebus, pbus_id, 0x10);
	pbus_data = (pbus_data_high << 16) + pbus_data_low;
	return pbus_data;
}

static int airoha_pbus_write(struct mii_bus *ebus, int pbus_id,
			unsigned long pbus_address, unsigned long pbus_data)
{
	int ret = 0;

	airoha_mdio_lock(ebus);
	ret = __airoha_pbus_write(ebus, pbus_id, pbus_address, pbus_data);
	airoha_mdio_unlock(ebus);

	return ret;
}

static unsigned long airoha_pbus_read(struct mii_bus *ebus, int pbus_id,
			unsigned long pbus_address)
{
	unsigned long pbus_data;

	airoha_mdio_lock(ebus);
	pbus_data = __airoha_pbus_read(ebus, pbus_id, pbus_address);
	airoha_mdio_unlock(ebus);

	return pbus_data;
}

/* Airoha Token Ring Write function */
static int airoha_tr_reg_write(struct phy_device *phydev,
			unsigned long tr_address, unsigned long tr_data)
{
	int ret = 0;
	int phy_addr = phydev_phy_addr(phydev);
	struct mii_bus *ebus = phydev_mdio_bus(phydev);

	airoha_mdio_lock(ebus);
	ret = __mdiobus_write(ebus, phy_addr, 0x1F, 0x52b5); /* page select */
	ret = __mdiobus_write(ebus, phy_addr, 0x11,
				(unsigned int)(tr_data & 0xffff));
	ret = __mdiobus_write(ebus, phy_addr, 0x12,
				(unsigned int)(tr_data >> 16));
	ret = __mdiobus_write(ebus, phy_addr, 0x10,
				(unsigned int)(tr_address | TrReg_WR));
	ret = __mdiobus_write(ebus, phy_addr, 0x1F, 0x0);    /* page resetore */
	airoha_mdio_unlock(ebus);

	return ret;
}

#ifdef AIR_LED_SUPPORT
static int airoha_led_set_usr_def(struct phy_device *phydev, u8 entity,
			int polar, u16 on_evt, u16 blk_evt)
{
	int ret = 0;
	int phy_addr = phydev_phy_addr(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	if (polar == AIR_ACTIVE_HIGH)
		on_evt |= LED_ON_POL;
	else
		on_evt &= ~LED_ON_POL;

	ret = airoha_cl45_write(mbus, phy_addr, 0x1f,
				LED_ON_CTRL(entity), on_evt | LED_ON_EN);
	if (ret < 0)
		return ret;

	ret = airoha_cl45_write(mbus, phy_addr, 0x1f,
				LED_BLK_CTRL(entity), blk_evt);
	if (ret < 0)
		return ret;

	return 0;
}

static int airoha_led_set_mode(struct phy_device *phydev, u8 mode)
{
	u16 cl45_data;
	int err = 0;
	int phy_addr = phydev_phy_addr(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	err = airoha_cl45_read(mbus, phy_addr, 0x1f, LED_BCR, &cl45_data);
	if (err < 0)
		return err;

	switch (mode) {
	case AIR_LED_MODE_DISABLE:
		cl45_data &= ~LED_BCR_EXT_CTRL;
		cl45_data &= ~LED_BCR_MODE_MASK;
		cl45_data |= LED_BCR_MODE_DISABLE;
		break;
	case AIR_LED_MODE_USER_DEFINE:
		cl45_data |= LED_BCR_EXT_CTRL;
		cl45_data |= LED_BCR_CLK_EN;
		break;
	default:
		return -EINVAL;
	}

	err = airoha_cl45_write(mbus, phy_addr, 0x1f, LED_BCR, cl45_data);
	if (err < 0)
		return err;
	return 0;
}

static int airoha_led_set_state(struct phy_device *phydev, u8 entity, u8 state)
{
	u16 cl45_data;
	int err;
	int phy_addr = phydev_phy_addr(phydev);
	struct mii_bus *mbus = phydev_mdio_bus(phydev);

	err = airoha_cl45_read(mbus, phy_addr, 0x1f,
				LED_ON_CTRL(entity), &cl45_data);
	if (err < 0)
		return err;
	if (state == LED_ENABLE)
		cl45_data |= LED_ON_EN;
	else
		cl45_data &= ~LED_ON_EN;

	err = airoha_cl45_write(mbus, phy_addr, 0x1f,
				LED_ON_CTRL(entity), cl45_data);
	if (err < 0)
		return err;
	return 0;
}

static int en8801s_led_init(struct phy_device *phydev)
{

	unsigned long led_gpio = 0, reg_value = 0;
	int ret = 0, led_id;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int gpio_led_rg[3] = {0x1870, 0x1874, 0x1878};
	u16 cl45_data = led_dur;
	struct device *dev = phydev_dev(phydev);
	int phy_addr = phydev_phy_addr(phydev);
	int pbus_addr = phydev_pbus_addr(phydev);

	ret = airoha_cl45_write(mbus, phy_addr, 0x1f, LED_BLK_DUR, cl45_data);
	if (ret < 0)
		return ret;
	cl45_data >>= 1;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1f, LED_ON_DUR, cl45_data);
	if (ret < 0)
		return ret;
	ret = airoha_led_set_mode(phydev, AIR_LED_MODE_USER_DEFINE);
	if (ret != 0) {
		dev_err(dev, "LED fail to set mode, ret %d !\n", ret);
		return ret;
	}
	for (led_id = 0; led_id < EN8801S_LED_COUNT; led_id++) {
		reg_value = 0;
		ret = airoha_led_set_state(phydev, led_id, led_cfg[led_id].en);
		if (ret != 0) {
			dev_err(dev, "LED fail to set state, ret %d !\n", ret);
			return ret;
		}
		if (led_cfg[led_id].en == LED_ENABLE) {
			if ((led_cfg[led_id].gpio < 0)
				|| led_cfg[led_id].gpio > 9) {
				dev_err(dev, "GPIO%d is out of range!! GPIO number is 0~9.\n",
					led_cfg[led_id].gpio);
				return -EIO;
			}
			led_gpio |= BIT(led_cfg[led_id].gpio);
			reg_value = airoha_pbus_read(mbus, pbus_addr,
					gpio_led_rg[led_cfg[led_id].gpio / 4]);
			LED_SET_GPIO_SEL(led_cfg[led_id].gpio,
					led_id, reg_value);
			dev_dbg(dev, "[Airoha] gpio%d, reg_value 0x%lx\n",
					led_cfg[led_id].gpio, reg_value);
			ret = airoha_pbus_write(mbus, pbus_addr,
					gpio_led_rg[led_cfg[led_id].gpio / 4],
					reg_value);
			if (ret < 0)
				return ret;
			ret = airoha_led_set_usr_def(phydev, led_id,
					led_cfg[led_id].pol,
					led_cfg[led_id].on_cfg,
					led_cfg[led_id].blk_cfg);
			if (ret != 0) {
				dev_err(dev, "LED fail to set usr def, ret %d !\n",
				ret);
				return ret;
			}
		}
	}
	reg_value = (airoha_pbus_read(mbus, pbus_addr, 0x1880) & ~led_gpio);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x1880, reg_value);
	if (ret < 0)
		return ret;
	ret = airoha_pbus_write(mbus, pbus_addr, 0x186c, led_gpio);
	if (ret < 0)
		return ret;
	dev_info(dev, "LED initialize OK !\n");
	return 0;
}
#endif
static int en8801s_phy_process(struct phy_device *phydev)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	unsigned long reg_value = 0;
	int ret = 0;
	int pbus_addr = phydev_pbus_addr(phydev);

	reg_value = airoha_pbus_read(mbus, pbus_addr, 0x19e0);
	reg_value |= BIT(0);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x19e0, reg_value);
	if (ret < 0)
		return ret;
	reg_value = airoha_pbus_read(mbus, pbus_addr, 0x19e0);
	reg_value &= ~BIT(0);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x19e0, reg_value);
	if (ret < 0)
		return ret;
	return ret;
}

static int en8801s_phase1_init(struct phy_device *phydev)
{
	unsigned long pbus_data;
	int pbus_addr = EN8801S_PBUS_DEFAULT_ADDR;
	u16 reg_value;
	int retry, ret = 0;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	struct device *dev = phydev_dev(phydev);
	struct en8801s_priv *priv = phydev->priv;

	priv->count = 1;
	msleep(1000);

	retry = MAX_OUI_CHECK;
	while (1) {
		pbus_data = airoha_pbus_read(mbus, pbus_addr,
				EN8801S_RG_ETHER_PHY_OUI);      /* PHY OUI */
		if (pbus_data == EN8801S_PBUS_OUI) {
			dev_info(dev, "PBUS addr 0x%x: Start initialized.\n",
					pbus_addr);
			break;
		}
		pbus_addr = phydev_pbus_addr(phydev);
		if (0 == --retry) {
			dev_err(dev, "Probe fail !\n");
			return 0;
		}
	}

	ret = airoha_pbus_write(mbus, pbus_addr, EN8801S_RG_BUCK_CTL, 0x03);
	if (ret < 0)
		return ret;
	pbus_data = airoha_pbus_read(mbus, pbus_addr, EN8801S_RG_PROD_VER);
	priv->pro_version = pbus_data & 0xf;
	dev_info(dev, "EN8801S Procduct Version :E%d\n", priv->pro_version);
	mdelay(10);
	pbus_data = (airoha_pbus_read(mbus, pbus_addr, EN8801S_RG_LTR_CTL)
				 & 0xfffffffc) | BIT(2);
	ret = airoha_pbus_write(mbus, pbus_addr,
				EN8801S_RG_LTR_CTL, pbus_data);
	if (ret < 0)
		return ret;
	mdelay(500);
	pbus_data = (pbus_data & ~BIT(2)) |
				EN8801S_RX_POLARITY_NORMAL |
				EN8801S_TX_POLARITY_NORMAL;
	ret = airoha_pbus_write(mbus, pbus_addr,
				EN8801S_RG_LTR_CTL, pbus_data);
	if (ret < 0)
		return ret;
	mdelay(500);
	if (priv->pro_version == 4) {
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1900);
		dev_dbg(dev, "Before 0x1900 0x%lx\n", pbus_data);
		ret = airoha_pbus_write(mbus, pbus_addr, 0x1900, 0x101009f);
		if (ret < 0)
			return ret;
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1900);
		dev_dbg(dev, "After 0x1900 0x%lx\n", pbus_data);
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x19a8);
		dev_dbg(dev, "Before 19a8 0x%lx\n", pbus_data);
		ret = airoha_pbus_write(mbus, pbus_addr,
				0x19a8, pbus_data & ~BIT(16));
		if (ret < 0)
			return ret;
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x19a8);
		dev_dbg(dev, "After 19a8 0x%lx\n", pbus_data);
	}
	pbus_data = airoha_pbus_read(mbus, pbus_addr,
				EN8801S_RG_SMI_ADDR); /* SMI ADDR */
	pbus_data = (pbus_data & 0xffff0000) |
				(unsigned long)(phydev_pbus_addr(phydev) << 8) |
				(unsigned long)(phydev_phy_addr(phydev));
	dev_info(phydev_dev(phydev), "SMI_ADDR=%lx (renew)\n", pbus_data);
	ret = airoha_pbus_write(mbus, pbus_addr,
				EN8801S_RG_SMI_ADDR, pbus_data);
	mdelay(10);

	retry = MAX_RETRY;
	while (1) {
		mdelay(10);
		reg_value = phy_read(phydev, MII_PHYSID2);
		if (reg_value == EN8801S_PHY_ID2)
			break;    /* wait GPHY ready */

		retry--;
		if (retry == 0) {
			dev_err(dev, "Initialize fail !\n");
			return 0;
		}
	}
	/* Software Reset PHY */
	reg_value = phy_read(phydev, MII_BMCR);
	reg_value |= BMCR_RESET;
	ret = phy_write(phydev, MII_BMCR, reg_value);
	if (ret < 0)
		return ret;
	retry = MAX_RETRY;
	do {
		mdelay(10);
		reg_value = phy_read(phydev, MII_BMCR);
		retry--;
		if (retry == 0) {
			dev_err(dev, "Reset fail !\n");
			return 0;
		}
	} while (reg_value & BMCR_RESET);

	phydev->dev_flags = PHY_STATE_INIT;

	dev_info(dev, "Phase1 initialize OK ! (%s)\n", EN8801S_DRIVER_VERSION);
	if (priv->pro_version == 4) {
		ret = en8801s_phase2_init(phydev);
		if (ret != 0) {
			dev_info(dev, "en8801_phase2_init failed\n");
			phydev->dev_flags = PHY_STATE_FAIL;
			return 0;
		}
		phydev->dev_flags = PHY_STATE_PROCESS;
	}

	return 0;
}

static int en8801s_phase2_init(struct phy_device *phydev)
{
	union gephy_all_REG_LpiReg1Ch      GPHY_RG_LPI_1C;
	union gephy_all_REG_dev1Eh_reg324h GPHY_RG_1E_324;
	union gephy_all_REG_dev1Eh_reg012h GPHY_RG_1E_012;
	union gephy_all_REG_dev1Eh_reg017h GPHY_RG_1E_017;
	unsigned long pbus_data;
	int phy_addr = phydev_phy_addr(phydev);
	int pbus_addr = phydev_pbus_addr(phydev);
	u16 cl45_value;
	int retry, ret = 0;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	struct device *dev = phydev_dev(phydev);
	struct en8801s_priv *priv = phydev->priv;

	pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1690);
	pbus_data |= BIT(31);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x1690, pbus_data);
	if (ret < 0)
		return ret;

	ret = airoha_pbus_write(mbus, pbus_addr, 0x0600, 0x0c000c00);
	if (ret < 0)
		return ret;
	ret = airoha_pbus_write(mbus, pbus_addr, 0x10, 0xD801);
	if (ret < 0)
		return ret;
	ret = airoha_pbus_write(mbus, pbus_addr, 0x0,  0x9140);
	if (ret < 0)
		return ret;

	ret = airoha_pbus_write(mbus, pbus_addr, 0x0A14, 0x0003);
	if (ret < 0)
		return ret;
	ret = airoha_pbus_write(mbus, pbus_addr, 0x0600, 0x0c000c00);
	if (ret < 0)
		return ret;
	/* Set FCM control */
	ret = airoha_pbus_write(mbus, pbus_addr, 0x1404, 0x004b);
	if (ret < 0)
		return ret;
	ret = airoha_pbus_write(mbus, pbus_addr, 0x140c, 0x0007);
	if (ret < 0)
		return ret;

	ret = airoha_pbus_write(mbus, pbus_addr, 0x142c, 0x05050505);
	if (ret < 0)
		return ret;
	pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1440);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x1440, pbus_data & ~BIT(11));
	if (ret < 0)
		return ret;

	pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1408);
	ret = airoha_pbus_write(mbus, pbus_addr, 0x1408, pbus_data | BIT(5));
	if (ret < 0)
		return ret;

	/* Set GPHY Perfomance*/
	/* Token Ring */
	ret = airoha_tr_reg_write(phydev, RgAddr_R1000DEC_15h, 0x0055A0);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_R1000DEC_17h, 0x07FF3F);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_PMA_00h,      0x00001E);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_PMA_01h,      0x6FB90A);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_PMA_17h,      0x060671);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_PMA_18h,      0x0E2F00);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_TR_26h,       0x444444);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_03h,     0x000000);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_06h,     0x2EBAEF);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_08h,     0x00000B);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_0Ch,     0x00504D);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_0Dh,     0x02314F);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_0Fh,     0x003028);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_10h,     0x005010);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_11h,     0x040001);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_13h,     0x018670);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_14h,     0x00024A);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_1Bh,     0x000072);
	if (ret < 0)
		return ret;
	ret = airoha_tr_reg_write(phydev, RgAddr_DSPF_1Ch,     0x003210);
	if (ret < 0)
		return ret;

	/* CL22 & CL45 */
	ret = phy_write(phydev, 0x1f, 0x03);
	if (ret < 0)
		return ret;
	GPHY_RG_LPI_1C.DATA = phy_read(phydev, RgAddr_LPI_1Ch);
	GPHY_RG_LPI_1C.DataBitField.smi_deton_th = 0x0C;
	ret = phy_write(phydev, RgAddr_LPI_1Ch, GPHY_RG_LPI_1C.DATA);
	if (ret < 0)
		return ret;
	ret = phy_write(phydev, RgAddr_LPI_1Ch, 0xC92);
	if (ret < 0)
		return ret;
	ret = phy_write(phydev, RgAddr_AUXILIARY_1Dh, 0x1);
	if (ret < 0)
		return ret;
	ret = phy_write(phydev, 0x1f, 0x0);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x120, 0x8014);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x122, 0xffff);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x123, 0xffff);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x144, 0x0200);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x14A, 0xEE20);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x189, 0x0110);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x19B, 0x0111);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x234, 0x0181);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x238, 0x0120);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x239, 0x0117);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x268, 0x07F4);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x2D1, 0x0733);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x323, 0x0011);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x324, 0x013F);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x326, 0x0037);
	if (ret < 0)
		return ret;

	ret = airoha_cl45_read(mbus, phy_addr, 0x1E, 0x324, &cl45_value);
	if (ret < 0)
		return ret;
	GPHY_RG_1E_324.DATA = cl45_value;
	GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off = 0;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x324,
				GPHY_RG_1E_324.DATA);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x19E, 0xC2);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x013, 0x0);
	if (ret < 0)
		return ret;

	/* EFUSE */
	airoha_pbus_write(mbus, pbus_addr, 0x1C08, 0x40000040);
	retry = MAX_RETRY;
	while (retry != 0) {
		mdelay(1);
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1C08);
		if ((pbus_data & BIT(30)) == 0)
			break;

		retry--;
	}
	pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1C38); /* RAW#2 */
	ret = airoha_cl45_read(mbus, phy_addr, 0x1E, 0x12, &cl45_value);
	if (ret < 0)
		return ret;
	GPHY_RG_1E_012.DATA = cl45_value;
	GPHY_RG_1E_012.DataBitField.da_tx_i2mpb_a_tbt =
				(u16)(pbus_data & 0x03f);
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x12,
				GPHY_RG_1E_012.DATA);
	if (ret < 0)
		return ret;
	ret = airoha_cl45_read(mbus, phy_addr, 0x1E, 0x17, &cl45_value);
	if (ret < 0)
		return ret;
	GPHY_RG_1E_017.DATA = cl45_value;
	GPHY_RG_1E_017.DataBitField.da_tx_i2mpb_b_tbt =
				(u16)((pbus_data >> 8) & 0x03f);
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x17,
				GPHY_RG_1E_017.DATA);
	if (ret < 0)
		return ret;

	airoha_pbus_write(mbus, pbus_addr, 0x1C08, 0x40400040);
	retry = MAX_RETRY;
	while (retry != 0) {
		mdelay(1);
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1C08);
		if ((pbus_data & BIT(30)) == 0)
			break;

		retry--;
	}
	pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1C30); /* RAW#16 */
	GPHY_RG_1E_324.DataBitField.smi_det_deglitch_off =
				(u16)((pbus_data >> 12) & 0x01);
	ret = airoha_cl45_write(mbus, phy_addr, 0x1E, 0x324,
				GPHY_RG_1E_324.DATA);
	if (ret < 0)
		return ret;
#ifdef AIR_LED_SUPPORT
	ret = en8801s_led_init(phydev);
	if (ret != 0)
		dev_err(dev, "en8801s_led_init fail (ret:%d) !\n", ret);
#endif

	ret = airoha_cl45_read(mbus, phy_addr, MDIO_MMD_AN,
				MDIO_AN_EEE_ADV, &cl45_value);
	if (ret < 0)
		return ret;
	if (cl45_value == 0) {
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1960);
		if (0xA == ((pbus_data & 0x07c00000) >> 22)) {
			pbus_data = (pbus_data & 0xf83fffff) | (0xC << 22);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1960,
						pbus_data);
			if (ret < 0)
				return ret;
			mdelay(10);
			pbus_data = (pbus_data & 0xf83fffff) | (0xE << 22);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1960,
						pbus_data);
			if (ret < 0)
				return ret;
			mdelay(10);
		}
	} else {
		pbus_data = airoha_pbus_read(mbus, pbus_addr, 0x1960);
		if (0xE == ((pbus_data & 0x07c00000) >> 22)) {
			pbus_data = (pbus_data & 0xf83fffff) | (0xC << 22);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1960,
						pbus_data);
			if (ret < 0)
				return ret;
			mdelay(10);
			pbus_data = (pbus_data & 0xf83fffff) | (0xA << 22);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1960,
						pbus_data);
			if (ret < 0)
				return ret;
			mdelay(10);
		}
	}

	priv->first_init = false;
	dev_info(phydev_dev(phydev), "Phase2 initialize OK !\n");
	return 0;
}

static int en8801s_read_status(struct phy_device *phydev)
{
	int ret = 0, preSpeed = phydev->speed;
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	u32 reg_value;
	struct device *dev = phydev_dev(phydev);
	int pbus_addr = phydev_pbus_addr(phydev);
	struct en8801s_priv *priv = phydev->priv;

	ret = genphy_read_status(phydev);
	if (phydev->link == LINK_DOWN)
		preSpeed = phydev->speed = 0;

	if (phydev->dev_flags == PHY_STATE_PROCESS) {
		en8801s_phy_process(phydev);
		phydev->dev_flags = PHY_STATE_DONE;
	}

	if (phydev->dev_flags == PHY_STATE_INIT) {
		dev_dbg(dev, "phydev->link %d, count %d\n",
					phydev->link, priv->count);
		if ((phydev->link) || (priv->count == 5)) {
			if (priv->pro_version != 4) {
				ret = en8801s_phase2_init(phydev);
				if (ret != 0) {
					dev_info(dev, "en8801_phase2_init failed\n");
					phydev->dev_flags = PHY_STATE_FAIL;
					return 0;
				}
				phydev->dev_flags = PHY_STATE_PROCESS;
			}
		}
		priv->count++;
	}

	if ((preSpeed != phydev->speed) && (phydev->link == LINK_UP)) {
		preSpeed = phydev->speed;

		if (preSpeed == SPEED_10) {
			reg_value = airoha_pbus_read(mbus, pbus_addr, 0x1694);
			reg_value |= BIT(31);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1694,
					reg_value);
			if (ret < 0)
				return ret;
			phydev->dev_flags = PHY_STATE_PROCESS;
		} else {
			reg_value = airoha_pbus_read(mbus, pbus_addr, 0x1694);
			reg_value &= ~BIT(31);
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1694,
					reg_value);
			if (ret < 0)
				return ret;
			phydev->dev_flags = PHY_STATE_PROCESS;
		}

		airoha_pbus_write(mbus, pbus_addr, 0x0600,
				0x0c000c00);
		if (preSpeed == SPEED_1000) {
			dev_dbg(dev, "SPEED_1000\n");
			ret = airoha_pbus_write(mbus, pbus_addr, 0x10,
					0xD801);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0,
					0x9140);
			if (ret < 0)
				return ret;

			ret = airoha_pbus_write(mbus, pbus_addr, 0x0A14,
					0x0003);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0600,
					0x0c000c00);
			if (ret < 0)
				return ret;
			mdelay(2);      /* delay 2 ms */
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1404,
					0x004b);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x140c,
					0x0007);
			if (ret < 0)
				return ret;
		} else if (preSpeed == SPEED_100) {
			dev_dbg(dev, "SPEED_100\n");
			ret = airoha_pbus_write(mbus, pbus_addr, 0x10,
					0xD401);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0,
					0x9140);
			if (ret < 0)
				return ret;

			ret = airoha_pbus_write(mbus, pbus_addr, 0x0A14,
					0x0007);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0600,
					0x0c11);
			if (ret < 0)
				return ret;
			mdelay(2);      /* delay 2 ms */
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1404,
					0x0027);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x140c,
					0x0007);
			if (ret < 0)
				return ret;
		} else if (preSpeed == SPEED_10) {
			dev_dbg(dev, "SPEED_10\n");
			ret = airoha_pbus_write(mbus, pbus_addr, 0x10,
					0xD001);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0,
					0x9140);
			if (ret < 0)
				return ret;

			ret = airoha_pbus_write(mbus, pbus_addr, 0x0A14,
					0x000b);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x0600,
					0x0c11);
			if (ret < 0)
				return ret;
			mdelay(2);      /* delay 2 ms */
			ret = airoha_pbus_write(mbus, pbus_addr, 0x1404,
					0x0027);
			if (ret < 0)
				return ret;
			ret = airoha_pbus_write(mbus, pbus_addr, 0x140c,
					0x0007);
			if (ret < 0)
				return ret;
		}
	}
	return ret;
}

static int en8801s_probe(struct phy_device *phydev)
{
	struct en8801s_priv *priv;
	unsigned long phy_addr = phydev_phy_addr(phydev);
	struct mdio_device *mdiodev = &phydev->mdio;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->count = 0;
	priv->first_init = true;

	if (mdiodev->reset_gpio) {
		dev_dbg(phydev_dev(phydev),
				"Assert PHY %lx HWRST until phy_init_hw\n",
				phy_addr);
		phy_device_reset(phydev, 1);
	}

	phydev->priv = priv;

	return 0;
}

static int airoha_mmd_read(struct phy_device *phydev,
			int devad, u16 reg)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int phy_addr = phydev_phy_addr(phydev);
	int ret = 0;
	u16 cl45_value;

	ret = __airoha_cl45_read(mbus, phy_addr, devad, reg, &cl45_value);
	if (ret < 0)
		return ret;

	return cl45_value;
}

static int airoha_mmd_write(struct phy_device *phydev,
			int devad, u16 reg, u16 val)
{
	struct mii_bus *mbus = phydev_mdio_bus(phydev);
	int phy_addr = phydev_phy_addr(phydev);
	int pbus_addr = phydev_pbus_addr(phydev);
	unsigned long pbus_data;
	int ret = 0;

	if (MDIO_MMD_AN == devad && MDIO_AN_EEE_ADV == reg) {
		if (val == 0) {
			pbus_data = __airoha_pbus_read(mbus, pbus_addr, 0x1960);
			if (0xA == ((pbus_data & 0x07c00000) >> 22)) {
				pbus_data = (pbus_data & 0xf83fffff) |
							(0xC << 22);
				__airoha_pbus_write(mbus, pbus_addr, 0x1960,
							pbus_data);
				mdelay(10);
				pbus_data = (pbus_data & 0xf83fffff) |
							(0xE << 22);
				__airoha_pbus_write(mbus, pbus_addr, 0x1960,
							pbus_data);
				mdelay(10);
			}
		} else {
			pbus_data = __airoha_pbus_read(mbus, pbus_addr, 0x1960);
			if (0xE == ((pbus_data & 0x07c00000) >> 22)) {
				pbus_data = (pbus_data & 0xf83fffff) |
							(0xC << 22);
				__airoha_pbus_write(mbus, pbus_addr, 0x1960,
							pbus_data);
				mdelay(10);
				pbus_data = (pbus_data & 0xf83fffff) |
							(0xA << 22);
				__airoha_pbus_write(mbus, pbus_addr, 0x1960,
							pbus_data);
				mdelay(10);
			}
		}
	}
	ret = __airoha_cl45_write(mbus, phy_addr, devad, reg, val);
	if (ret < 0)
		return ret;

	return 0;
}

static struct phy_driver Airoha_driver[] = {
	{
		.phy_id         = EN8801SC_PHY_ID,
		.name           = "Airoha EN8801SC",
		.phy_id_mask    = 0x0ffffff0,
		.features       = PHY_GBIT_FEATURES,
		.probe          = en8801s_probe,
		.config_init    = en8801s_phase1_init,
		.config_aneg    = genphy_config_aneg,
		.read_status    = en8801s_read_status,
		.suspend        = genphy_suspend,
		.resume         = genphy_resume,
		.read_mmd       = airoha_mmd_read,
		.write_mmd      = airoha_mmd_write,
	}
};

module_phy_driver(Airoha_driver);

static struct mdio_device_id __maybe_unused Airoha_tbl[] = {
	{ EN8801SC_PHY_ID, 0x0ffffff0 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, Airoha_tbl);
