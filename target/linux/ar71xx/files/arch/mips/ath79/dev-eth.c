/*
 *  Atheros AR71xx SoC platform devices
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros 2.6.15 BSP
 *  Parts of this file are based on Atheros 2.6.31 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/sizes.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/irq.h>

#include "common.h"
#include "dev-eth.h"

unsigned char ath79_mac_base[ETH_ALEN] __initdata;

static struct resource ath79_mdio0_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}
};

struct ag71xx_mdio_platform_data ath79_mdio0_data;

struct platform_device ath79_mdio0_device = {
	.name		= "ag71xx-mdio",
	.id		= 0,
	.resource	= ath79_mdio0_resources,
	.num_resources	= ARRAY_SIZE(ath79_mdio0_resources),
	.dev = {
		.platform_data = &ath79_mdio0_data,
	},
};

static struct resource ath79_mdio1_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}
};

struct ag71xx_mdio_platform_data ath79_mdio1_data;

struct platform_device ath79_mdio1_device = {
	.name		= "ag71xx-mdio",
	.id		= 1,
	.resource	= ath79_mdio1_resources,
	.num_resources	= ARRAY_SIZE(ath79_mdio1_resources),
	.dev = {
		.platform_data = &ath79_mdio1_data,
	},
};

static void ath79_set_pll(u32 cfg_reg, u32 pll_reg, u32 pll_val, u32 shift)
{
	void __iomem *base;
	u32 t;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	t = __raw_readl(base + cfg_reg);
	t &= ~(3 << shift);
	t |=  (2 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	__raw_writel(pll_val, base + pll_reg);

	t |= (3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	t &= ~(3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	printk(KERN_DEBUG "ar71xx: pll_reg %#x: %#x\n",
		(unsigned int)(base + pll_reg), __raw_readl(base + pll_reg));

	iounmap(base);
}

static void __init ath79_mii_ctrl_set_if(unsigned int reg,
					  unsigned int mii_if)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(AR71XX_MII_CTRL_IF_MASK);
	t |= (mii_if & AR71XX_MII_CTRL_IF_MASK);
	__raw_writel(t, base + reg);

	iounmap(base);
}

static void ath79_mii_ctrl_set_speed(unsigned int reg, unsigned int speed)
{
	void __iomem *base;
	unsigned int mii_speed;
	u32 t;

	switch (speed) {
	case SPEED_10:
		mii_speed =  AR71XX_MII_CTRL_SPEED_10;
		break;
	case SPEED_100:
		mii_speed =  AR71XX_MII_CTRL_SPEED_100;
		break;
	case SPEED_1000:
		mii_speed =  AR71XX_MII_CTRL_SPEED_1000;
		break;
	default:
		BUG();
	}

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(AR71XX_MII_CTRL_SPEED_MASK << AR71XX_MII_CTRL_SPEED_SHIFT);
	t |= mii_speed  << AR71XX_MII_CTRL_SPEED_SHIFT;
	__raw_writel(t, base + reg);

	iounmap(base);
}

static unsigned long ar934x_get_mdio_ref_clock(void)
{
	void __iomem *base;
	unsigned long ret;
	u32 t;

	base = ioremap(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	ret = 0;
	t = __raw_readl(base + AR934X_PLL_SWITCH_CLOCK_CONTROL_REG);
	if (t & AR934X_PLL_SWITCH_CLOCK_CONTROL_MDIO_CLK_SEL) {
		ret = 100 * 1000 * 1000;
	} else {
		struct clk *clk;

		clk = clk_get(NULL, "ref");
		if (!IS_ERR(clk))
			ret = clk_get_rate(clk);
	}

	iounmap(base);

	return ret;
}

void __init ath79_register_mdio(unsigned int id, u32 phy_mask)
{
	struct platform_device *mdio_dev;
	struct ag71xx_mdio_platform_data *mdio_data;
	unsigned int max_id;

	if (ath79_soc == ATH79_SOC_AR9341 ||
	    ath79_soc == ATH79_SOC_AR9342 ||
	    ath79_soc == ATH79_SOC_AR9344 ||
	    ath79_soc == ATH79_SOC_QCA9556 ||
	    ath79_soc == ATH79_SOC_QCA9558 ||
	    ath79_soc == ATH79_SOC_QCA956X)
		max_id = 1;
	else
		max_id = 0;

	if (id > max_id) {
		printk(KERN_ERR "ar71xx: invalid MDIO id %u\n", id);
		return;
	}

	switch (ath79_soc) {
	case ATH79_SOC_AR7241:
	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
	case ATH79_SOC_QCA9533:
	case ATH79_SOC_TP9343:
		mdio_dev = &ath79_mdio1_device;
		mdio_data = &ath79_mdio1_data;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
	case ATH79_SOC_QCA9556:
	case ATH79_SOC_QCA9558:
	case ATH79_SOC_QCA956X:
		if (id == 0) {
			mdio_dev = &ath79_mdio0_device;
			mdio_data = &ath79_mdio0_data;
		} else {
			mdio_dev = &ath79_mdio1_device;
			mdio_data = &ath79_mdio1_data;
		}
		break;

	case ATH79_SOC_AR7242:
		ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG,
			       AR7242_PLL_REG_ETH0_INT_CLOCK, 0x62000000,
			       AR71XX_ETH0_PLL_SHIFT);
		/* fall through */
	default:
		mdio_dev = &ath79_mdio0_device;
		mdio_data = &ath79_mdio0_data;
		break;
	}

	mdio_data->phy_mask = phy_mask;

	switch (ath79_soc) {
	case ATH79_SOC_AR7240:
		mdio_data->is_ar7240 = 1;
		/* fall through */
	case ATH79_SOC_AR7241:
		mdio_data->builtin_switch = 1;
		break;

	case ATH79_SOC_AR9330:
		mdio_data->is_ar9330 = 1;
		/* fall through */
	case ATH79_SOC_AR9331:
		mdio_data->builtin_switch = 1;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
		if (id == 1) {
			mdio_data->builtin_switch = 1;
			mdio_data->ref_clock = ar934x_get_mdio_ref_clock();
			mdio_data->mdio_clock = 6250000;
		}
		mdio_data->is_ar934x = 1;
		break;

	case ATH79_SOC_QCA9533:
	case ATH79_SOC_TP9343:
		mdio_data->builtin_switch = 1;
		break;

	case ATH79_SOC_QCA9556:
	case ATH79_SOC_QCA9558:
		mdio_data->is_ar934x = 1;
		break;

	case ATH79_SOC_QCA956X:
		if (id == 1)
			mdio_data->builtin_switch = 1;
		mdio_data->is_ar934x = 1;
		break;

	default:
		break;
	}

	platform_device_register(mdio_dev);
}

struct ath79_eth_pll_data ath79_eth0_pll_data;
struct ath79_eth_pll_data ath79_eth1_pll_data;

static u32 ath79_get_eth_pll(unsigned int mac, int speed)
{
	struct ath79_eth_pll_data *pll_data;
	u32 pll_val;

	switch (mac) {
	case 0:
		pll_data = &ath79_eth0_pll_data;
		break;
	case 1:
		pll_data = &ath79_eth1_pll_data;
		break;
	default:
		BUG();
	}

	switch (speed) {
	case SPEED_10:
		pll_val = pll_data->pll_10;
		break;
	case SPEED_100:
		pll_val = pll_data->pll_100;
		break;
	case SPEED_1000:
		pll_val = pll_data->pll_1000;
		break;
	default:
		BUG();
	}

	return pll_val;
}

static void ath79_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);

	ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH0_INT_CLOCK,
			val, AR71XX_ETH0_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII0_CTRL, speed);
}

static void ath79_set_speed_ge1(int speed)
{
	u32 val = ath79_get_eth_pll(1, speed);

	ath79_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH1_INT_CLOCK,
			 val, AR71XX_ETH1_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII1_CTRL, speed);
}

static void ar7242_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);
	void __iomem *base;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR7242_PLL_REG_ETH0_INT_CLOCK);
	iounmap(base);
}

static void ar91xx_set_speed_ge0(int speed)
{
	u32 val = ath79_get_eth_pll(0, speed);

	ath79_set_pll(AR913X_PLL_REG_ETH_CONFIG, AR913X_PLL_REG_ETH0_INT_CLOCK,
			 val, AR913X_ETH0_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII0_CTRL, speed);
}

static void ar91xx_set_speed_ge1(int speed)
{
	u32 val = ath79_get_eth_pll(1, speed);

	ath79_set_pll(AR913X_PLL_REG_ETH_CONFIG, AR913X_PLL_REG_ETH1_INT_CLOCK,
			 val, AR913X_ETH1_PLL_SHIFT);
	ath79_mii_ctrl_set_speed(AR71XX_MII_REG_MII1_CTRL, speed);
}

static void ar934x_set_speed_ge0(int speed)
{
	void __iomem *base;
	u32 val = ath79_get_eth_pll(0, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR934X_PLL_ETH_XMII_CONTROL_REG);
	iounmap(base);
}

static void qca955x_set_speed_xmii(int speed)
{
	void __iomem *base;
	u32 val = ath79_get_eth_pll(0, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + QCA955X_PLL_ETH_XMII_CONTROL_REG);
	iounmap(base);
}

static void qca955x_set_speed_sgmii(int id, int speed)
{
	void __iomem *base;
	u32 val = ath79_get_eth_pll(id, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + QCA955X_PLL_ETH_SGMII_CONTROL_REG);
	iounmap(base);
}

static void qca9556_set_speed_sgmii(int speed)
{
	qca955x_set_speed_sgmii(0, speed);
}

static void qca9558_set_speed_sgmii(int speed)
{
	qca955x_set_speed_sgmii(1, speed);
}

static void qca956x_set_speed_sgmii(int speed)
{
	void __iomem *base;
	u32 val = ath79_get_eth_pll(0, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + QCA955X_PLL_ETH_SGMII_CONTROL_REG);
	iounmap(base);
}

static void ath79_set_speed_dummy(int speed)
{
}

static void ath79_ddr_flush_ge0(void)
{
	ath79_ddr_wb_flush(0);
}

static void ath79_ddr_flush_ge1(void)
{
	ath79_ddr_wb_flush(1);
}

static struct resource ath79_eth0_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= ATH79_CPU_IRQ(4),
		.end	= ATH79_CPU_IRQ(4),
	},
};

struct ag71xx_platform_data ath79_eth0_data = {
	.reset_bit	= AR71XX_RESET_GE0_MAC,
};

struct platform_device ath79_eth0_device = {
	.name		= "ag71xx",
	.id		= 0,
	.resource	= ath79_eth0_resources,
	.num_resources	= ARRAY_SIZE(ath79_eth0_resources),
	.dev = {
		.platform_data = &ath79_eth0_data,
	},
};

static struct resource ath79_eth1_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= ATH79_CPU_IRQ(5),
		.end	= ATH79_CPU_IRQ(5),
	},
};

struct ag71xx_platform_data ath79_eth1_data = {
	.reset_bit	= AR71XX_RESET_GE1_MAC,
};

struct platform_device ath79_eth1_device = {
	.name		= "ag71xx",
	.id		= 1,
	.resource	= ath79_eth1_resources,
	.num_resources	= ARRAY_SIZE(ath79_eth1_resources),
	.dev = {
		.platform_data = &ath79_eth1_data,
	},
};

struct ag71xx_switch_platform_data ath79_switch_data;

#define AR71XX_PLL_VAL_1000	0x00110000
#define AR71XX_PLL_VAL_100	0x00001099
#define AR71XX_PLL_VAL_10	0x00991099

#define AR724X_PLL_VAL_1000	0x00110000
#define AR724X_PLL_VAL_100	0x00001099
#define AR724X_PLL_VAL_10	0x00991099

#define AR7242_PLL_VAL_1000	0x16000000
#define AR7242_PLL_VAL_100	0x00000101
#define AR7242_PLL_VAL_10	0x00001616

#define AR913X_PLL_VAL_1000	0x1a000000
#define AR913X_PLL_VAL_100	0x13000a44
#define AR913X_PLL_VAL_10	0x00441099

#define AR933X_PLL_VAL_1000	0x00110000
#define AR933X_PLL_VAL_100	0x00001099
#define AR933X_PLL_VAL_10	0x00991099

#define AR934X_PLL_VAL_1000	0x16000000
#define AR934X_PLL_VAL_100	0x00000101
#define AR934X_PLL_VAL_10	0x00001616

#define QCA956X_PLL_VAL_1000	0x03000000
#define QCA956X_PLL_VAL_100	0x00000101
#define QCA956X_PLL_VAL_10	0x00001919

static void __init ath79_init_eth_pll_data(unsigned int id)
{
	struct ath79_eth_pll_data *pll_data;
	u32 pll_10, pll_100, pll_1000;

	switch (id) {
	case 0:
		pll_data = &ath79_eth0_pll_data;
		break;
	case 1:
		pll_data = &ath79_eth1_pll_data;
		break;
	default:
		BUG();
	}

	switch (ath79_soc) {
	case ATH79_SOC_AR7130:
	case ATH79_SOC_AR7141:
	case ATH79_SOC_AR7161:
		pll_10 = AR71XX_PLL_VAL_10;
		pll_100 = AR71XX_PLL_VAL_100;
		pll_1000 = AR71XX_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR7240:
	case ATH79_SOC_AR7241:
		pll_10 = AR724X_PLL_VAL_10;
		pll_100 = AR724X_PLL_VAL_100;
		pll_1000 = AR724X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR7242:
		pll_10 = AR7242_PLL_VAL_10;
		pll_100 = AR7242_PLL_VAL_100;
		pll_1000 = AR7242_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9130:
	case ATH79_SOC_AR9132:
		pll_10 = AR913X_PLL_VAL_10;
		pll_100 = AR913X_PLL_VAL_100;
		pll_1000 = AR913X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		pll_10 = AR933X_PLL_VAL_10;
		pll_100 = AR933X_PLL_VAL_100;
		pll_1000 = AR933X_PLL_VAL_1000;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
	case ATH79_SOC_QCA9533:
	case ATH79_SOC_QCA9556:
	case ATH79_SOC_QCA9558:
	case ATH79_SOC_TP9343:
		pll_10 = AR934X_PLL_VAL_10;
		pll_100 = AR934X_PLL_VAL_100;
		pll_1000 = AR934X_PLL_VAL_1000;
		break;

	case ATH79_SOC_QCA956X:
		pll_10 = QCA956X_PLL_VAL_10;
		pll_100 = QCA956X_PLL_VAL_100;
		pll_1000 = QCA956X_PLL_VAL_1000;
		break;

	default:
		BUG();
	}

	if (!pll_data->pll_10)
		pll_data->pll_10 = pll_10;

	if (!pll_data->pll_100)
		pll_data->pll_100 = pll_100;

	if (!pll_data->pll_1000)
		pll_data->pll_1000 = pll_1000;
}

static int __init ath79_setup_phy_if_mode(unsigned int id,
					   struct ag71xx_platform_data *pdata)
{
	unsigned int mii_if;

	switch (id) {
	case 0:
		switch (ath79_soc) {
		case ATH79_SOC_AR7130:
		case ATH79_SOC_AR7141:
		case ATH79_SOC_AR7161:
		case ATH79_SOC_AR9130:
		case ATH79_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
				mii_if = AR71XX_MII0_CTRL_IF_MII;
				break;
			case PHY_INTERFACE_MODE_GMII:
				mii_if = AR71XX_MII0_CTRL_IF_GMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = AR71XX_MII0_CTRL_IF_RGMII;
				break;
			case PHY_INTERFACE_MODE_RMII:
				mii_if = AR71XX_MII0_CTRL_IF_RMII;
				break;
			default:
				return -EINVAL;
			}
			ath79_mii_ctrl_set_if(AR71XX_MII_REG_MII0_CTRL, mii_if);
			break;

		case ATH79_SOC_AR7240:
		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
		case ATH79_SOC_QCA9533:
		case ATH79_SOC_TP9343:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_MII;
			break;

		case ATH79_SOC_AR7242:
			/* FIXME */

		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_RMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		case ATH79_SOC_QCA9556:
		case ATH79_SOC_QCA9558:
		case ATH79_SOC_QCA956X:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_SGMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	case 1:
		switch (ath79_soc) {
		case ATH79_SOC_AR7130:
		case ATH79_SOC_AR7141:
		case ATH79_SOC_AR7161:
		case ATH79_SOC_AR9130:
		case ATH79_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_RMII:
				mii_if = AR71XX_MII1_CTRL_IF_RMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = AR71XX_MII1_CTRL_IF_RGMII;
				break;
			default:
				return -EINVAL;
			}
			ath79_mii_ctrl_set_if(AR71XX_MII_REG_MII1_CTRL, mii_if);
			break;

		case ATH79_SOC_AR7240:
		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
		case ATH79_SOC_TP9343:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_GMII;
			break;

		case ATH79_SOC_AR7242:
			/* FIXME */

		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
		case ATH79_SOC_QCA9533:
		case ATH79_SOC_QCA956X:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		case ATH79_SOC_QCA9556:
		case ATH79_SOC_QCA9558:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_SGMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	}

	return 0;
}

void __init ath79_setup_ar933x_phy4_switch(bool mac, bool mdio)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR933X_GMAC_BASE, AR933X_GMAC_SIZE);

	t = __raw_readl(base + AR933X_GMAC_REG_ETH_CFG);
	t &= ~(AR933X_ETH_CFG_SW_PHY_SWAP | AR933X_ETH_CFG_SW_PHY_ADDR_SWAP);
	if (mac)
		t |= AR933X_ETH_CFG_SW_PHY_SWAP;
	if (mdio)
		t |= AR933X_ETH_CFG_SW_PHY_ADDR_SWAP;
	__raw_writel(t, base + AR933X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_ar934x_eth_cfg(u32 mask)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 |
	       AR934X_ETH_CFG_MII_GMAC0 |
	       AR934X_ETH_CFG_GMII_GMAC0 |
	       AR934X_ETH_CFG_SW_ONLY_MODE |
	       AR934X_ETH_CFG_SW_PHY_SWAP);

	t |= mask;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_ar934x_eth_rx_delay(unsigned int rxd,
					    unsigned int rxdv)
{
	void __iomem *base;
	u32 t;

	rxd &= AR934X_ETH_CFG_RXD_DELAY_MASK;
	rxdv &= AR934X_ETH_CFG_RDV_DELAY_MASK;

	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RXD_DELAY_MASK << AR934X_ETH_CFG_RXD_DELAY_SHIFT |
	       AR934X_ETH_CFG_RDV_DELAY_MASK << AR934X_ETH_CFG_RDV_DELAY_SHIFT);

	t |= (rxd << AR934X_ETH_CFG_RXD_DELAY_SHIFT |
	      rxdv << AR934X_ETH_CFG_RDV_DELAY_SHIFT);

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_qca955x_eth_cfg(u32 mask)
{
	void __iomem *base;
	u32 t;

	base = ioremap(QCA955X_GMAC_BASE, QCA955X_GMAC_SIZE);

	t = __raw_readl(base + QCA955X_GMAC_REG_ETH_CFG);

	t &= ~(QCA955X_ETH_CFG_RGMII_EN | QCA955X_ETH_CFG_GE0_SGMII);

	t |= mask;

	__raw_writel(t, base + QCA955X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_qca956x_eth_cfg(u32 mask)
{
	void __iomem *base;
	u32 t;

	base = ioremap(QCA956X_GMAC_BASE, QCA956X_GMAC_SIZE);

	t = __raw_readl(base + QCA956X_GMAC_REG_ETH_CFG);

	t &= ~(QCA956X_ETH_CFG_SW_ONLY_MODE |
	       QCA956X_ETH_CFG_SW_PHY_SWAP);

	t |= mask;

	__raw_writel(t, base + QCA956X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + QCA956X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

static int ath79_eth_instance __initdata;
void __init ath79_register_eth(unsigned int id)
{
	struct platform_device *pdev;
	struct ag71xx_platform_data *pdata;
	int err;

	if (id > 1) {
		printk(KERN_ERR "ar71xx: invalid ethernet id %d\n", id);
		return;
	}

	ath79_init_eth_pll_data(id);

	if (id == 0)
		pdev = &ath79_eth0_device;
	else
		pdev = &ath79_eth1_device;

	pdata = pdev->dev.platform_data;

	pdata->max_frame_len = 1540;
	pdata->desc_pktlen_mask = 0xfff;

	err = ath79_setup_phy_if_mode(id, pdata);
	if (err) {
		printk(KERN_ERR
		       "ar71xx: invalid PHY interface mode for GE%u\n", id);
		return;
	}

	if (id == 0)
		pdata->ddr_flush = ath79_ddr_flush_ge0;
	else
		pdata->ddr_flush = ath79_ddr_flush_ge1;

	switch (ath79_soc) {
	case ATH79_SOC_AR7130:
		if (id == 0)
			pdata->set_speed = ath79_set_speed_ge0;
		else
			pdata->set_speed = ath79_set_speed_ge1;
		break;

	case ATH79_SOC_AR7141:
	case ATH79_SOC_AR7161:
		if (id == 0)
			pdata->set_speed = ath79_set_speed_ge0;
		else
			pdata->set_speed = ath79_set_speed_ge1;
		pdata->has_gbit = 1;
		break;

	case ATH79_SOC_AR7242:
		if (id == 0) {
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO |
					    AR71XX_RESET_GE0_PHY;
			pdata->set_speed = ar7242_set_speed_ge0;
		} else {
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO |
					    AR71XX_RESET_GE1_PHY;
			pdata->set_speed = ath79_set_speed_dummy;
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		break;

	case ATH79_SOC_AR7241:
		if (id == 0)
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO;
		else
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO;
		/* fall through */
	case ATH79_SOC_AR7240:
		if (id == 0) {
			pdata->reset_bit |= AR71XX_RESET_GE0_PHY;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit |= AR71XX_RESET_GE1_PHY;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ath79_switch_data;
			pdata->use_flow_control = 1;

			ath79_switch_data.phy_poll_mask |= BIT(4);
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		if (ath79_soc == ATH79_SOC_AR7240)
			pdata->is_ar7240 = 1;
		break;

	case ATH79_SOC_AR9132:
		pdata->has_gbit = 1;
		/* fall through */
	case ATH79_SOC_AR9130:
		if (id == 0)
			pdata->set_speed = ar91xx_set_speed_ge0;
		else
			pdata->set_speed = ar91xx_set_speed_ge1;
		pdata->is_ar91xx = 1;
		break;

	case ATH79_SOC_AR9330:
	case ATH79_SOC_AR9331:
		if (id == 0) {
			pdata->reset_bit = AR933X_RESET_GE0_MAC |
					   AR933X_RESET_GE0_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit = AR933X_RESET_GE1_MAC |
					   AR933X_RESET_GE1_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->speed = SPEED_1000;
			pdata->has_gbit = 1;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ath79_switch_data;
			pdata->use_flow_control = 1;

			ath79_switch_data.phy_poll_mask |= BIT(4);
		}

		pdata->is_ar724x = 1;
		break;

	case ATH79_SOC_AR9341:
	case ATH79_SOC_AR9342:
	case ATH79_SOC_AR9344:
	case ATH79_SOC_QCA9533:
		if (id == 0) {
			pdata->reset_bit = AR934X_RESET_GE0_MAC |
					   AR934X_RESET_GE0_MDIO;
			pdata->set_speed = ar934x_set_speed_ge0;

			if (ath79_soc == ATH79_SOC_QCA9533)
				pdata->disable_inline_checksum_engine = 1;
		} else {
			pdata->reset_bit = AR934X_RESET_GE1_MAC |
					   AR934X_RESET_GE1_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->switch_data = &ath79_switch_data;

			/* reset the built-in switch */
			ath79_device_reset_set(AR934X_RESET_ETH_SWITCH);
			ath79_device_reset_clear(AR934X_RESET_ETH_SWITCH);
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		pdata->max_frame_len = SZ_16K - 1;
		pdata->desc_pktlen_mask = SZ_16K - 1;
		break;

	case ATH79_SOC_TP9343:
		if (id == 0) {
			pdata->reset_bit = AR933X_RESET_GE0_MAC |
					   AR933X_RESET_GE0_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			if (!pdata->phy_mask)
				pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit = AR933X_RESET_GE1_MAC |
					   AR933X_RESET_GE1_MDIO;
			pdata->set_speed = ath79_set_speed_dummy;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ath79_switch_data;
			pdata->use_flow_control = 1;

			ath79_switch_data.phy_poll_mask |= BIT(4);
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		break;

	case ATH79_SOC_QCA9556:
	case ATH79_SOC_QCA9558:
		if (id == 0) {
			pdata->reset_bit = QCA955X_RESET_GE0_MAC |
					   QCA955X_RESET_GE0_MDIO;
			pdata->set_speed = qca955x_set_speed_xmii;

			/* QCA9556 only has SGMII interface */
			if (ath79_soc == ATH79_SOC_QCA9556)
				pdata->set_speed = qca9556_set_speed_sgmii;
		} else {
			pdata->reset_bit = QCA955X_RESET_GE1_MAC |
					   QCA955X_RESET_GE1_MDIO;
			pdata->set_speed = qca9558_set_speed_sgmii;
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		/*
		 * Limit the maximum frame length to 4095 bytes.
		 * Although the documentation says that the hardware
		 * limit is 16383 bytes but that does not work in
		 * practice. It seems that the hardware only updates
		 * the lowest 12 bits of the packet length field
		 * in the RX descriptor.
		 */
		pdata->max_frame_len = SZ_4K - 1;
		pdata->desc_pktlen_mask = SZ_16K - 1;
		break;

	case ATH79_SOC_QCA956X:
		if (id == 0) {
			pdata->reset_bit = QCA955X_RESET_GE0_MAC |
					   QCA955X_RESET_GE0_MDIO;

			if (pdata->phy_if_mode == PHY_INTERFACE_MODE_SGMII)
				pdata->set_speed = qca956x_set_speed_sgmii;
			else
				pdata->set_speed = ar934x_set_speed_ge0;

			pdata->disable_inline_checksum_engine = 1;
		} else {
			pdata->reset_bit = QCA955X_RESET_GE1_MAC |
					   QCA955X_RESET_GE1_MDIO;

			pdata->set_speed = ath79_set_speed_dummy;

			pdata->switch_data = &ath79_switch_data;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->use_flow_control = 1;

			/* reset the built-in switch */
			ath79_device_reset_set(AR934X_RESET_ETH_SWITCH);
			ath79_device_reset_clear(AR934X_RESET_ETH_SWITCH);
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		break;

	default:
		BUG();
	}

	switch (pdata->phy_if_mode) {
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_SGMII:
		if (!pdata->has_gbit) {
			printk(KERN_ERR "ar71xx: no gbit available on eth%d\n",
					id);
			return;
		}
		/* fallthrough */
	default:
		break;
	}

	if (!is_valid_ether_addr(pdata->mac_addr)) {
		random_ether_addr(pdata->mac_addr);
		printk(KERN_DEBUG
			"ar71xx: using random MAC address for eth%d\n",
			ath79_eth_instance);
	}

	if (pdata->mii_bus_dev == NULL) {
		switch (ath79_soc) {
		case ATH79_SOC_AR9341:
		case ATH79_SOC_AR9342:
		case ATH79_SOC_AR9344:
			if (id == 0)
				pdata->mii_bus_dev = &ath79_mdio0_device.dev;
			else
				pdata->mii_bus_dev = &ath79_mdio1_device.dev;
			break;

		case ATH79_SOC_AR7241:
		case ATH79_SOC_AR9330:
		case ATH79_SOC_AR9331:
		case ATH79_SOC_QCA9533:
		case ATH79_SOC_TP9343:
			pdata->mii_bus_dev = &ath79_mdio1_device.dev;
			break;

		case ATH79_SOC_QCA9556:
		case ATH79_SOC_QCA9558:
			/* don't assign any MDIO device by default */
			break;

		case ATH79_SOC_QCA956X:
			if (pdata->phy_if_mode != PHY_INTERFACE_MODE_SGMII)
				pdata->mii_bus_dev = &ath79_mdio1_device.dev;
			break;

		default:
			pdata->mii_bus_dev = &ath79_mdio0_device.dev;
			break;
		}
	}

	/* Reset the device */
	ath79_device_reset_set(pdata->reset_bit);
	msleep(100);

	ath79_device_reset_clear(pdata->reset_bit);
	msleep(100);

	platform_device_register(pdev);
	ath79_eth_instance++;
}

void __init ath79_set_mac_base(unsigned char *mac)
{
	memcpy(ath79_mac_base, mac, ETH_ALEN);
}

void __init ath79_parse_ascii_mac(char *mac_str, u8 *mac)
{
	int t;

	t = sscanf(mac_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	if (t != ETH_ALEN)
		t = sscanf(mac_str, "%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx",
			&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

	if (t != ETH_ALEN || !is_valid_ether_addr(mac)) {
		memset(mac, 0, ETH_ALEN);
		printk(KERN_DEBUG "ar71xx: invalid mac address \"%s\"\n",
		       mac_str);
	}
}

void __init ath79_extract_mac_reverse(u8 *ptr, u8 *out)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++) {
		out[i] = ptr[ETH_ALEN-i-1];
	}
}

static void __init ath79_set_mac_base_ascii(char *str)
{
	u8 mac[ETH_ALEN];

	ath79_parse_ascii_mac(str, mac);
	ath79_set_mac_base(mac);
}

static int __init ath79_ethaddr_setup(char *str)
{
	ath79_set_mac_base_ascii(str);
	return 1;
}
__setup("ethaddr=", ath79_ethaddr_setup);

static int __init ath79_kmac_setup(char *str)
{
	ath79_set_mac_base_ascii(str);
	return 1;
}
__setup("kmac=", ath79_kmac_setup);

void __init ath79_init_mac(unsigned char *dst, const unsigned char *src,
			    int offset)
{
	int t;

	if (!dst)
		return;

	if (!src || !is_valid_ether_addr(src)) {
		memset(dst, '\0', ETH_ALEN);
		return;
	}

	t = (((u32) src[3]) << 16) + (((u32) src[4]) << 8) + ((u32) src[5]);
	t += offset;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = (t >> 16) & 0xff;
	dst[4] = (t >> 8) & 0xff;
	dst[5] = t & 0xff;
}

void __init ath79_init_local_mac(unsigned char *dst, const unsigned char *src)
{
	int i;

	if (!dst)
		return;

	if (!src || !is_valid_ether_addr(src)) {
		memset(dst, '\0', ETH_ALEN);
		return;
	}

	for (i = 0; i < ETH_ALEN; i++)
		dst[i] = src[i];
	dst[0] |= 0x02;
}
