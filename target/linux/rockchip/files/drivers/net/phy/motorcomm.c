/*
 * drivers/net/phy/motorcomm.c
 *
 * Driver for Motorcomm PHYs
 *
 * Author: Leilei Zhao <leilei.zhao@motorcomm.com>
 *
 * Copyright (c) 2019 Motorcomm, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Motorcomm Phys:
 *		Giga phys: yt8511, yt8521
 *		100/10 Phys : yt8512, yt8512b, yt8510
 *		Automotive 100Mb Phys : yt8010
 *		Automotive 100/10 hyper range Phys: yt8510
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/motorcomm_phy.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/version.h>

static int ytphy_read_ext(struct phy_device *phydev, u32 regnum)
{
	int ret;
	int val;

	ret = phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, REG_DEBUG_DATA);

	return val;
}

static int ytphy_write_ext(struct phy_device *phydev, u32 regnum, u16 val)
{
	int ret;

	ret = phy_write(phydev, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = phy_write(phydev, REG_DEBUG_DATA, val);

	return ret;
}

static int yt8010_config_aneg(struct phy_device *phydev)
{
	phydev->speed = SPEED_100;
	return 0;
}

static int yt8511_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	/* disable auto sleep */
	val = ytphy_read_ext(phydev, YT8511_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8511_EN_SLEEP_SW_BIT));
	ret = ytphy_write_ext(phydev, YT8511_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* output SyncE clock (125mhz) even link is down */
	ret = ytphy_write_ext(phydev, 0xa012, 0xd0);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	val = ytphy_read_ext(phydev, 0xc);
	if (val < 0)
		return val;

	/* ext reg 0xc.b[2:1]
	00-----25M from pll;
	01---- 25M from xtl;(default)
	10-----62.5M from pll;
	11----125M from pll(here set to this value)
	*/
	val |= (3 << 1);
	ret = ytphy_write_ext(phydev, 0xc, val);
	if (ret < 0)
		return ret;

	/* LED0: Unused/Off, LED1: Link, LED2: Activity, 8Hz */
	ytphy_write_ext(phydev, 0xa00b, 0xe004);
	ytphy_write_ext(phydev, 0xa00c, 0);
	ytphy_write_ext(phydev, 0xa00d, 0x2600);
	ytphy_write_ext(phydev, 0xa00e, 0x0070);
	ytphy_write_ext(phydev, 0xa00f, 0x000a);

	return 0;
}

static int yt8512_clk_init(struct phy_device *phydev)
{
	int ret;
	int val;

	val = ytphy_read_ext(phydev, YT8512_EXTREG_AFE_PLL);
	if (val < 0)
		return val;

	val |= YT8512_CONFIG_PLL_REFCLK_SEL_EN;

	ret = ytphy_write_ext(phydev, YT8512_EXTREG_AFE_PLL, val);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, YT8512_EXTREG_EXTEND_COMBO);
	if (val < 0)
		return val;

	val |= YT8512_CONTROL1_RMII_EN;

	ret = ytphy_write_ext(phydev, YT8512_EXTREG_EXTEND_COMBO, val);
	if (ret < 0)
		return ret;

	val = phy_read(phydev, MII_BMCR);
	if (val < 0)
		return val;

	val |= YT_SOFTWARE_RESET;
	ret = phy_write(phydev, MII_BMCR, val);

	return ret;
}

static int yt8512_led_init(struct phy_device *phydev)
{
	int ret;
	int val;
	int mask;

	val = ytphy_read_ext(phydev, YT8512_EXTREG_LED0);
	if (val < 0)
		return val;

	val |= YT8512_LED0_ACT_BLK_IND;

	mask = YT8512_LED0_DIS_LED_AN_TRY | YT8512_LED0_BT_BLK_EN |
		YT8512_LED0_HT_BLK_EN | YT8512_LED0_COL_BLK_EN |
		YT8512_LED0_BT_ON_EN;
	val &= ~mask;

	ret = ytphy_write_ext(phydev, YT8512_EXTREG_LED0, val);
	if (ret < 0)
		return ret;

	val = ytphy_read_ext(phydev, YT8512_EXTREG_LED1);
	if (val < 0)
		return val;

	val |= YT8512_LED1_BT_ON_EN;

	mask = YT8512_LED1_TXACT_BLK_EN | YT8512_LED1_RXACT_BLK_EN;
	val &= ~mask;

	ret = ytphy_write_ext(phydev, YT8512_LED1_BT_ON_EN, val);

	return ret;
}

static int yt8512_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	ret = yt8512_clk_init(phydev);
	if (ret < 0)
		return ret;

	ret = yt8512_led_init(phydev);

	/* disable auto sleep */
	val = ytphy_read_ext(phydev, YT8512_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8512_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(phydev, YT8512_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	return ret;
}

static int yt8512_read_status(struct phy_device *phydev)
{
	int ret;
	int val;
	int speed, speed_mode, duplex;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	val = phy_read(phydev, REG_PHY_SPEC_STATUS);
	if (val < 0)
		return val;

	duplex = (val & YT8512_DUPLEX) >> YT8512_DUPLEX_BIT;
	speed_mode = (val & YT8512_SPEED_MODE) >> YT8512_SPEED_MODE_BIT;
	switch (speed_mode) {
	case 0:
		speed = SPEED_10;
		break;
	case 1:
		speed = SPEED_100;
		break;
	case 2:
	case 3:
	default:
		speed = SPEED_UNKNOWN;
		break;
	}

	phydev->speed = speed;
	phydev->duplex = duplex;

	return 0;
}

static int yt8521_config_init(struct phy_device *phydev)
{
	int ret;
	int val;

	/* disable auto sleep */
	val = ytphy_read_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8521_EN_SLEEP_SW_BIT));
	ret = ytphy_write_ext(phydev, YT8521_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	/* switch to access UTP */
	ret = ytphy_write_ext(phydev, 0xa000, 0);
	if (ret < 0)
		return ret;

	/* enable RXC clock when no wire plug */
	val = ytphy_read_ext(phydev, 0xc);
	if (val < 0)
		return val;

	val &= ~(1 << 12);
	ret = ytphy_write_ext(phydev, 0xc, val);
	if (ret < 0)
		return ret;

	/* output SyncE clock (125mhz) even link is down */
	ret = ytphy_write_ext(phydev, 0xa012, 0x38);
	if (ret < 0)
		return ret;

	/* disable rgmii clk 2ns delay */
	val = ytphy_read_ext(phydev, 0xa001);
	if (val < 0)
		return val;

	val &= ~(1 << 8);
	ret = ytphy_write_ext(phydev, 0xa001, val);
	if (ret < 0)
		return ret;

	/* setup delay */
	val = (1 << 10) | (0xf << 4) | 5;
	ret = ytphy_write_ext(phydev, 0xa003, val);
	if (ret < 0)
		return ret;

	/* LED0: Unused/Off, LED1: Link, LED2: Activity, 8Hz */
	ytphy_write_ext(phydev, 0xa00b, 0xe004);
	ytphy_write_ext(phydev, 0xa00c, 0);
	ytphy_write_ext(phydev, 0xa00d, 0x2600);
	ytphy_write_ext(phydev, 0xa00e, 0x0070);
	ytphy_write_ext(phydev, 0xa00f, 0x000a);

	return 0;
}

static int yt8521_config_intr(struct phy_device *phydev)
{
	int val;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		val = BIT(14) | BIT(13) | BIT(11) | BIT(10);
	else
		val = 0;

	return phy_write(phydev, REG_INT_MASK, val);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0)
static int yt8521_ack_interrupt(struct phy_device *phydev)
{
	int val;

	val = phy_read(phydev, REG_INT_STATUS);
	phydev_dbg(phydev, "intr status 0x04%x\n", val);

	return (val < 0) ? val : 0;
}
#else
static irqreturn_t yt8521_handle_interrupt(struct phy_device *phydev)
{
	int irq_status, int_enabled;

	irq_status = phy_read(phydev, REG_INT_STATUS);
	if (irq_status < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	/* Read the current enabled interrupts */
	int_enabled = phy_read(phydev, REG_INT_MASK);
	if (int_enabled < 0) {
		phy_error(phydev);
		return IRQ_NONE;
	}

	/* See if this was one of our enabled interrupts */
	if (!(irq_status & int_enabled))
		return IRQ_NONE;

	phy_trigger_machine(phydev);

	return IRQ_HANDLED;
}
#endif

static struct phy_driver ytphy_drvs[] = {
	{
		.phy_id		= PHY_ID_YT8010,
		.name		= "YT8010 Automotive Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_BASIC_FEATURES,
		.config_aneg	= yt8010_config_aneg,
		.read_status	= genphy_read_status,
	}, {
		.phy_id		= PHY_ID_YT8510,
		.name		= "YT8510 100/10Mb Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_BASIC_FEATURES,
		.read_status	= genphy_read_status,
	}, {
		.phy_id		= PHY_ID_YT8511,
		.name		= "YT8511 Gigabit Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_GBIT_FEATURES,
		.read_status	= genphy_read_status,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= PHY_ID_YT8512,
		.name		= "YT8512 Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_BASIC_FEATURES,
		.config_init	= yt8512_config_init,
		.read_status	= yt8512_read_status,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= PHY_ID_YT8512B,
		.name		= "YT8512B Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_BASIC_FEATURES,
		.config_init	= yt8512_config_init,
		.read_status	= yt8512_read_status,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= PHY_ID_YT8521,
		.name		= "YT8521 Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		/* PHY_GBIT_FEATURES */
		.config_init	= yt8521_config_init,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0)
		.ack_interrupt	= yt8521_ack_interrupt,
#else
		.handle_interrupt = yt8521_handle_interrupt,
#endif
		.config_intr	= yt8521_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		/* same as 8521 */
		.phy_id		= PHY_ID_YT8531S,
		.name		= "YT8531S Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		/* PHY_GBIT_FEATURES */
		.config_init	= yt8521_config_init,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,12,0)
		.ack_interrupt	= yt8521_ack_interrupt,
#else
		.handle_interrupt = yt8521_handle_interrupt,
#endif
		.config_intr	= yt8521_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		/* same as 8511 */
		.phy_id		= PHY_ID_YT8531,
		.name		= "YT8531 Gigabit Ethernet",
		.phy_id_mask	= MOTORCOMM_PHY_ID_MASK,
		.features	= PHY_GBIT_FEATURES,
		.config_init	= yt8511_config_init,
		.read_status	= genphy_read_status,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
};

module_phy_driver(ytphy_drvs);

static struct mdio_device_id __maybe_unused motorcomm_tbl[] = {
	{ PHY_ID_YT8010, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8510, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8511, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8512, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8512B, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8521, MOTORCOMM_PHY_ID_MASK },
	{ PHY_ID_YT8531S, MOTORCOMM_PHY_ID_8531_MASK },
	{ PHY_ID_YT8531, MOTORCOMM_PHY_ID_8531_MASK },
	{ }
};

MODULE_AUTHOR("Leilei Zhao");
MODULE_DESCRIPTION("Motorcomm PHY driver");
MODULE_DEVICE_TABLE(mdio, motorcomm_tbl);
MODULE_LICENSE("GPL");
