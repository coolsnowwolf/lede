/*
 *  Cisco Meraki MR18 board support
 *
 *  Copyright (C) 2015 Chris Blake <chrisrblake93@gmail.com>
 *  Copyright (C) 2015 Christian Lamparter <chunkeey@googlemail.com>
 *  Copyright (C) 2015 Thomas Hebb <tommyhebb@gmail.com>
 *
 *  Based on Cisco Meraki GPL Release r23-20150601 MR18 Device Config
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/platform/ar934x_nfc.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/leds-nu801.h>
#include <linux/pci.h>

#include "common.h"
#include "dev-eth.h"
#include "pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define MR18_GPIO_LED_POWER_WHITE    18
#define MR18_GPIO_LED_POWER_ORANGE    21

#define MR18_GPIO_BTN_RESET    17
#define MR18_KEYS_POLL_INTERVAL    20  /* msecs */
#define MR18_KEYS_DEBOUNCE_INTERVAL  (3 * MR18_KEYS_POLL_INTERVAL)

#define MR18_WAN_PHYADDR    3

/* used for eth calibration */
#define MR18_OTP_BASE			(AR71XX_APB_BASE + 0x130000)
#define MR18_OTP_SIZE			(0x2000) /* just a guess */
#define MR18_OTP_MEM_0_REG		(0x0000)
#define MR18_OTP_INTF2_REG		(0x1008)
#define MR18_OTP_STATUS0_REG		(0x1018)
#define MR18_OTP_STATUS0_EFUSE_VALID	BIT(2)

#define MR18_OTP_STATUS1_REG		(0x101c)
#define MR18_OTP_LDO_CTRL_REG		(0x1024)
#define MR18_OTP_LDO_STATUS_REG		(0x102c)
#define MR18_OTP_LDO_STATUS_POWER_ON	BIT(0)

static struct gpio_led MR18_leds_gpio[] __initdata = {
	{
		.name = "mr18:white:power",
		.gpio = MR18_GPIO_LED_POWER_WHITE,
		.active_low  = 1,
	}, {
		.name = "mr18:orange:power",
		.gpio = MR18_GPIO_LED_POWER_ORANGE,
		.active_low  = 0,
	},
};

static struct gpio_keys_button MR18_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = MR18_KEYS_DEBOUNCE_INTERVAL,
		.gpio    = MR18_GPIO_BTN_RESET,
		.active_low  = 1,
	},
};

static struct led_nu801_template tricolor_led_template = {
	.device_name = "mr18",
	.name = "tricolor",
	.num_leds = 1,
	.cki = 11,
	.sdi = 12,
	.lei = -1,
	.ndelay = 500,
	.init_brightness = {
		LED_OFF,
		LED_OFF,
		LED_OFF,
	},
	.default_trigger = "none",
	.led_colors = { "red", "green", "blue" },
};

static struct led_nu801_platform_data tricolor_led_data = {
	.num_controllers = 1,
	.template = &tricolor_led_template,
};

static struct platform_device tricolor_leds = {
	.name = "leds-nu801",
	.id = -1,
	.dev.platform_data = &tricolor_led_data,
};

static int mr18_extract_sgmii_res_cal(void)
{
	void __iomem *base;
	unsigned int reversed_sgmii_value;

	unsigned int otp_value, otp_per_val, rbias_per, read_data;
	unsigned int rbias_pos_or_neg;
	unsigned int sgmii_res_cal_value;
	int res_cal_val;

	base = ioremap_nocache(MR18_OTP_BASE, MR18_OTP_SIZE);
	if (!base)
		return -EIO;

	__raw_writel(0x7d, base + MR18_OTP_INTF2_REG);
	__raw_writel(0x00, base + MR18_OTP_LDO_CTRL_REG);

	while (__raw_readl(base + MR18_OTP_LDO_STATUS_REG) &
		MR18_OTP_LDO_STATUS_POWER_ON);

	__raw_readl(base + MR18_OTP_MEM_0_REG + 4);

	while (!(__raw_readl(base + MR18_OTP_STATUS0_REG) &
		MR18_OTP_STATUS0_EFUSE_VALID));

	read_data = __raw_readl(base + MR18_OTP_STATUS1_REG);

	iounmap(base);

	if (!(read_data & 0x1fff))
		return -ENODEV;

	if (read_data & 0x00001000)
		otp_value = (read_data & 0xfc0) >> 6;
	else
		otp_value = read_data & 0x3f;

	if (otp_value > 31) {
		otp_per_val = 63 - otp_value;
		rbias_pos_or_neg = 1;
	} else {
		otp_per_val = otp_value;
		rbias_pos_or_neg = 0;
	}

	rbias_per = otp_per_val * 15;

	if (rbias_pos_or_neg == 1)
		res_cal_val = (rbias_per + 34) / 21;
	else if (rbias_per > 34)
		res_cal_val = -((rbias_per - 34) / 21);
	else
		res_cal_val = (34 - rbias_per) / 21;

	sgmii_res_cal_value = (8 + res_cal_val) & 0xf;

	reversed_sgmii_value  = (sgmii_res_cal_value & 8) >> 3;
	reversed_sgmii_value |= (sgmii_res_cal_value & 4) >> 1;
	reversed_sgmii_value |= (sgmii_res_cal_value & 2) << 1;
	reversed_sgmii_value |= (sgmii_res_cal_value & 1) << 3;
	printk(KERN_INFO "SGMII cal value = 0x%x\n", reversed_sgmii_value);
	return reversed_sgmii_value;
}

#define QCA955X_PLL_ETH_SGMII_SERDES_REG		0x004c
#define QCA955X_PLL_ETH_SGMII_SERDES_LOCK_DETECT	BIT(2)
#define QCA955X_PLL_ETH_SGMII_SERDES_PLL_REFCLK		BIT(1)
#define QCA955X_PLL_ETH_SGMII_SERDES_EN_PLL		BIT(0)

#define QCA955X_GMAC_REG_SGMII_SERDES			0x0018
#define QCA955X_SGMII_SERDES_RES_CALIBRATION		BIT(23)
#define QCA955X_SGMII_SERDES_RES_CALIBRATION_MASK	0xf
#define QCA955X_SGMII_SERDES_RES_CALIBRATION_SHIFT	23
#define QCA955X_SGMII_SERDES_LOCK_DETECT_STATUS		BIT(15)

static void mr18_setup_qca955x_eth_serdes_cal(unsigned int sgmii_value)
{
	void __iomem *ethbase, *pllbase;
	u32 t;

	ethbase = ioremap_nocache(QCA955X_GMAC_BASE, QCA955X_GMAC_SIZE);
	pllbase = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	/* To Check the locking of the SGMII PLL */
	t = __raw_readl(ethbase + QCA955X_GMAC_REG_SGMII_SERDES);
	t &= ~(QCA955X_SGMII_SERDES_RES_CALIBRATION_MASK <<
	       QCA955X_SGMII_SERDES_RES_CALIBRATION_SHIFT);
	t |= (sgmii_value & QCA955X_SGMII_SERDES_RES_CALIBRATION_MASK) <<
	     QCA955X_SGMII_SERDES_RES_CALIBRATION_SHIFT;
	__raw_writel(t, ethbase + QCA955X_GMAC_REG_SGMII_SERDES);

	__raw_writel(QCA955X_PLL_ETH_SGMII_SERDES_LOCK_DETECT |
		     QCA955X_PLL_ETH_SGMII_SERDES_PLL_REFCLK |
		     QCA955X_PLL_ETH_SGMII_SERDES_EN_PLL,
		     pllbase + QCA955X_PLL_ETH_SGMII_SERDES_REG);

	ath79_device_reset_clear(QCA955X_RESET_SGMII_ANALOG);
	ath79_device_reset_clear(QCA955X_RESET_SGMII);

	while (!(__raw_readl(ethbase + QCA955X_GMAC_REG_SGMII_SERDES) &
		QCA955X_SGMII_SERDES_LOCK_DETECT_STATUS));

	iounmap(ethbase);
	iounmap(pllbase);
}

static struct ath9k_platform_data pci_main_wifi_data = {
	.led_pin = -1,
};
static struct ath9k_platform_data pci_scan_wifi_data = {
	.led_pin = -1,
};

static int mr18_dual_pci_plat_dev_init(struct pci_dev *dev)
{
	/* The PCIE devices are attached to different busses but they
	 * both share the same slot number. Checking the PCI_SLOT vals
	 * does not work.
	 */
	switch (dev->bus->number) {
	case 0:
		dev->dev.platform_data = &pci_main_wifi_data;
		break;
	case 1:
		dev->dev.platform_data = &pci_scan_wifi_data;
		break;
	}

	return 0;
}

static void __init mr18_setup(void)
{
	int res;

	/* NAND */
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_SOFT_BCH);
	ath79_register_nfc();

	/* even though, the PHY is connected via RGMII,
	 * the SGMII/SERDES PLLs need to be calibrated and locked.
	 * Or else, the PHY won't be working for this platfrom.
	 *
	 * Figuring this out took such a long time, that we want to
	 * point this quirk out, before someone wants to remove it.
	 */
	res = mr18_extract_sgmii_res_cal();
	if (res >= 0) {
		/* Setup SoC Eth Config */
		ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN |
			(3 << QCA955X_ETH_CFG_RXD_DELAY_SHIFT) |
			(3 << QCA955X_ETH_CFG_RDV_DELAY_SHIFT));

		/* MDIO Interface */
		ath79_register_mdio(0, 0x0);

		mr18_setup_qca955x_eth_serdes_cal(res);

		/* GMAC0 is connected to an Atheros AR8035-A */
		ath79_init_mac(ath79_eth0_data.mac_addr, NULL, 0);
		ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
		ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
		ath79_eth0_data.phy_mask = BIT(MR18_WAN_PHYADDR);
		ath79_eth0_pll_data.pll_1000 = 0xa6000000;
		ath79_eth0_pll_data.pll_100 = 0xa0000101;
		ath79_eth0_pll_data.pll_10 = 0x80001313;
		ath79_register_eth(0);
	} else {
		printk(KERN_ERR "failed to read EFUSE for ethernet cal\n");
	}

	/* LEDs and Buttons */
	platform_device_register(&tricolor_leds);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(MR18_leds_gpio),
				 MR18_leds_gpio);
	ath79_register_gpio_keys_polled(-1, MR18_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(MR18_gpio_keys),
					MR18_gpio_keys);

	/* Clear RTC reset (Needed by SoC WiFi) */
	ath79_device_reset_clear(QCA955X_RESET_RTC);

	/* WiFi */
	ath79_register_wmac_simple();

	pci_main_wifi_data.eeprom_name = "pci_wmac0.eeprom";
	pci_scan_wifi_data.eeprom_name = "pci_wmac1.eeprom";
	ath79_pci_set_plat_dev_init(mr18_dual_pci_plat_dev_init);
	ath79_register_pci();
}
MIPS_MACHINE(ATH79_MACH_MR18, "MR18", "Meraki MR18", mr18_setup);
