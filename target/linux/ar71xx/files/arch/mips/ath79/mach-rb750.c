/*
 *  MikroTik RouterBOARD 750/750GL support
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/export.h>
#include <linux/pci.h>
#include <linux/ath9k_platform.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/ar8216_platform.h>
#include <linux/rle.h>
#include <linux/routerboot.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/mach-rb750.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-usb.h"
#include "dev-eth.h"
#include "machtypes.h"
#include "routerboot.h"

static struct rb750_led_data rb750_leds[] = {
	{
		.name		= "rb750:green:act",
		.mask		= RB750_LED_ACT,
		.active_low	= 1,
	}, {
		.name		= "rb750:green:port1",
		.mask		= RB750_LED_PORT5,
		.active_low	= 1,
	}, {
		.name		= "rb750:green:port2",
		.mask		= RB750_LED_PORT4,
		.active_low	= 1,
	}, {
		.name		= "rb750:green:port3",
		.mask		= RB750_LED_PORT3,
		.active_low	= 1,
	}, {
		.name		= "rb750:green:port4",
		.mask		= RB750_LED_PORT2,
		.active_low	= 1,
	}, {
		.name		= "rb750:green:port5",
		.mask		= RB750_LED_PORT1,
		.active_low	= 1,
	}
};

static struct rb750_led_data rb750gr3_leds[] = {
	{
		.name		= "rb750:green:act",
		.mask		= RB7XX_LED_ACT,
		.active_low	= 1,
	},
};

static struct rb750_led_platform_data rb750_leds_data;
static struct platform_device rb750_leds_device = {
	.name	= "leds-rb750",
	.dev	= {
		.platform_data = &rb750_leds_data,
	}
};

static struct rb7xx_nand_platform_data rb750_nand_data;
static struct platform_device rb750_nand_device = {
	.name	= "rb750-nand",
	.id	= -1,
	.dev	= {
		.platform_data = &rb750_nand_data,
	}
};

static void rb750_latch_change(u32 mask_clr, u32 mask_set)
{
	static DEFINE_SPINLOCK(lock);
	static u32 latch_set = RB750_LED_BITS | RB750_LVC573_LE;
	static u32 latch_oe;
	static u32 latch_clr;
	unsigned long flags;
	u32 t;

	spin_lock_irqsave(&lock, flags);

	if ((mask_clr & BIT(31)) != 0 &&
	    (latch_set & RB750_LVC573_LE) == 0) {
		goto unlock;
	}

	latch_set = (latch_set | mask_set) & ~mask_clr;
	latch_clr = (latch_clr | mask_clr) & ~mask_set;

	if (latch_oe == 0)
		latch_oe = __raw_readl(ath79_gpio_base + AR71XX_GPIO_REG_OE);

	if (likely(latch_set & RB750_LVC573_LE)) {
		void __iomem *base = ath79_gpio_base;

		t = __raw_readl(base + AR71XX_GPIO_REG_OE);
		t |= mask_clr | latch_oe | mask_set;

		__raw_writel(t, base + AR71XX_GPIO_REG_OE);
		__raw_writel(latch_clr, base + AR71XX_GPIO_REG_CLEAR);
		__raw_writel(latch_set, base + AR71XX_GPIO_REG_SET);
	} else if (mask_clr & RB750_LVC573_LE) {
		void __iomem *base = ath79_gpio_base;

		latch_oe = __raw_readl(base + AR71XX_GPIO_REG_OE);
		__raw_writel(RB750_LVC573_LE, base + AR71XX_GPIO_REG_CLEAR);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_CLEAR);
	}

unlock:
	spin_unlock_irqrestore(&lock, flags);
}

static void rb750_nand_enable_pins(void)
{
	rb750_latch_change(RB750_LVC573_LE, 0);
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_SPI_EN);
}

static void rb750_nand_disable_pins(void)
{
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_SPI_EN,
				  AR724X_GPIO_FUNC_JTAG_DISABLE);
	rb750_latch_change(0, RB750_LVC573_LE);
}

static void __init rb750_setup(void)
{
	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				     AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				     AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				     AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				     AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);

	ath79_register_mdio(0, 0x0);

	/* LAN ports */
	ath79_register_eth(1);

	/* WAN port */
	ath79_register_eth(0);

	rb750_leds_data.num_leds = ARRAY_SIZE(rb750_leds);
	rb750_leds_data.leds = rb750_leds;
	rb750_leds_data.latch_change = rb750_latch_change;
	platform_device_register(&rb750_leds_device);

	rb750_nand_data.nce_line = RB750_NAND_NCE;
	rb750_nand_data.enable_pins = rb750_nand_enable_pins;
	rb750_nand_data.disable_pins = rb750_nand_disable_pins;
	rb750_nand_data.latch_change = rb750_latch_change;
	platform_device_register(&rb750_nand_device);

	/* USB */
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RB_750, "750i", "MikroTik RouterBOARD 750",
	     rb750_setup);

static struct ar8327_pad_cfg rb750gr3_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data rb750gr3_ar8327_data = {
	.pad0_cfg = &rb750gr3_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	}
};

static struct mdio_board_info rb750g3_mdio_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &rb750gr3_ar8327_data,
	},
};

static void rb750gr3_nand_enable_pins(void)
{
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_SPI_EN |
				  AR724X_GPIO_FUNC_SPI_CS_EN2);
}

static void rb750gr3_nand_disable_pins(void)
{
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_SPI_EN |
				  AR724X_GPIO_FUNC_SPI_CS_EN2,
				  AR724X_GPIO_FUNC_JTAG_DISABLE);
}

static void rb750gr3_latch_change(u32 mask_clr, u32 mask_set)
{
	static DEFINE_SPINLOCK(lock);
	static u32 latch_set = RB7XX_LED_ACT;
	static u32 latch_clr;
	void __iomem *base = ath79_gpio_base;
	unsigned long flags;
	u32 t;

	spin_lock_irqsave(&lock, flags);

	latch_set = (latch_set | mask_set) & ~mask_clr;
	latch_clr = (latch_clr | mask_clr) & ~mask_set;

	mask_set = latch_set & (RB7XX_USB_POWERON | RB7XX_MONITOR);
	mask_clr = latch_clr & (RB7XX_USB_POWERON | RB7XX_MONITOR);

	if ((latch_set ^ RB7XX_LED_ACT) & RB7XX_LED_ACT) {
		/* enable output mode */
		t = __raw_readl(base + AR71XX_GPIO_REG_OE);
		t |= RB7XX_LED_ACT;
		__raw_writel(t, base + AR71XX_GPIO_REG_OE);

		mask_clr |= RB7XX_LED_ACT;
	} else {
		/* disable output mode */
		t = __raw_readl(base + AR71XX_GPIO_REG_OE);
		t &= ~RB7XX_LED_ACT;
		__raw_writel(t, base + AR71XX_GPIO_REG_OE);
	}

	__raw_writel(mask_set, base + AR71XX_GPIO_REG_SET);
	__raw_writel(mask_clr, base + AR71XX_GPIO_REG_CLEAR);

	spin_unlock_irqrestore(&lock, flags);
}

static void __init rb750gr3_setup(void)
{
	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(rb750g3_mdio_info,
				    ARRAY_SIZE(rb750g3_mdio_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_pll_data.pll_1000 = 0x62000000;

	ath79_register_eth(0);

	rb750_leds_data.num_leds = ARRAY_SIZE(rb750gr3_leds);
	rb750_leds_data.leds = rb750gr3_leds;
	rb750_leds_data.latch_change = rb750gr3_latch_change;
	platform_device_register(&rb750_leds_device);

	rb750_nand_data.nce_line = RB7XX_NAND_NCE;
	rb750_nand_data.enable_pins = rb750gr3_nand_enable_pins;
	rb750_nand_data.disable_pins = rb750gr3_nand_disable_pins;
	rb750_nand_data.latch_change = rb750gr3_latch_change;
	platform_device_register(&rb750_nand_device);
}

MIPS_MACHINE(ATH79_MACH_RB_750G_R3, "750Gr3", "MikroTik RouterBOARD 750GL",
	     rb750gr3_setup);

#define RB751_HARDCONFIG	0x1f00b000
#define RB751_HARDCONFIG_SIZE	0x1000

static void __init rb751_wlan_setup(void)
{
	u8 *hardconfig = (u8 *) KSEG1ADDR(RB751_HARDCONFIG);
	struct ath9k_platform_data *wmac_data;
	u16 tag_len;
	u8 *tag;
	u16 mac_len;
	u8 *mac;
	int err;

	wmac_data = ap9x_pci_get_wmac_data(0);
	if (!wmac_data) {
		pr_err("rb75x: unable to get address of wlan data\n");
		return;
	}

	ap9x_pci_setup_wmac_led_pin(0, 9);

	err = routerboot_find_tag(hardconfig, RB751_HARDCONFIG_SIZE,
				  RB_ID_WLAN_DATA, &tag, &tag_len);
	if (err) {
		pr_err("rb75x: no calibration data found\n");
		return;
	}

	err = rle_decode(tag, tag_len, (unsigned char *) wmac_data->eeprom_data,
			 sizeof(wmac_data->eeprom_data), NULL, NULL);
	if (err) {
		pr_err("rb75x: unable to decode wlan eeprom data\n");
		return;
	}

	err = routerboot_find_tag(hardconfig, RB751_HARDCONFIG_SIZE,
                                 RB_ID_MAC_ADDRESS_PACK, &mac, &mac_len);
	if (err) {
		pr_err("rb75x: no mac address found\n");
		return;
	}

	ap91_pci_init(NULL, mac);
}

static void __init rb751_setup(void)
{
	rb750_setup();
	ath79_register_usb();
	rb751_wlan_setup();
}

MIPS_MACHINE(ATH79_MACH_RB_751, "751", "MikroTik RouterBOARD 751",
	     rb751_setup);

static void __init rb751g_setup(void)
{
	rb750gr3_setup();
	ath79_register_usb();
	rb751_wlan_setup();
}

MIPS_MACHINE(ATH79_MACH_RB_751G, "751g", "MikroTik RouterBOARD 751G",
	     rb751g_setup);
