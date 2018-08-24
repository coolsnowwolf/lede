/*
 *  ALFA Network AP96 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/mmc/host.h>
#include <linux/spi/spi.h>
#include <linux/spi/mmc_spi.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define ALFA_AP96_GPIO_PCIE_RESET	2
#define ALFA_AP96_GPIO_SIM_DETECT	3
#define ALFA_AP96_GPIO_MICROSD_CD	4
#define ALFA_AP96_GPIO_PCIE_W_DISABLE	5

#define ALFA_AP96_GPIO_BUTTON_RESET	11

#define ALFA_AP96_KEYS_POLL_INTERVAL		20	/* msecs */
#define ALFA_AP96_KEYS_DEBOUNCE_INTERVAL	(3 * ALFA_AP96_KEYS_POLL_INTERVAL)

static struct gpio_keys_button alfa_ap96_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ALFA_AP96_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ALFA_AP96_GPIO_BUTTON_RESET,
		.active_low	= 1,
	}
};

static struct mmc_spi_platform_data alfa_ap96_mmc_data = {
	.flags		= MMC_SPI_USE_CD_GPIO,
	.cd_gpio	= ALFA_AP96_GPIO_MICROSD_CD,
	.cd_debounce	= 1,
	.caps		= MMC_CAP_NEEDS_POLL,
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
};

static struct spi_board_info alfa_ap96_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
	}, {
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 25000000,
		.modalias	= "mmc_spi",
		.platform_data	= &alfa_ap96_mmc_data,
	}, {
		.bus_num	= 0,
		.chip_select	= 2,
		.max_speed_hz	= 6250000,
		.modalias	= "rtc-pcf2123",
	},
};

static struct ath79_spi_platform_data alfa_ap96_spi_data = {
	.bus_num		= 0,
	.num_chipselect		= 3,
};

static void __init alfa_ap96_gpio_setup(void)
{
	ath79_gpio_function_enable(AR71XX_GPIO_FUNC_SPI_CS1_EN |
				   AR71XX_GPIO_FUNC_SPI_CS2_EN);

	gpio_request(ALFA_AP96_GPIO_MICROSD_CD, "microSD CD");
	gpio_direction_input(ALFA_AP96_GPIO_MICROSD_CD);
	gpio_request(ALFA_AP96_GPIO_PCIE_RESET, "PCIe reset");
	gpio_direction_output(ALFA_AP96_GPIO_PCIE_RESET, 1);
	gpio_request(ALFA_AP96_GPIO_PCIE_W_DISABLE, "PCIe write disable");
	gpio_direction_output(ALFA_AP96_GPIO_PCIE_W_DISABLE, 1);
}

#define ALFA_AP96_WAN_PHYMASK	BIT(4)
#define ALFA_AP96_LAN_PHYMASK	BIT(5)
#define ALFA_AP96_MDIO_PHYMASK	(ALFA_AP96_LAN_PHYMASK | ALFA_AP96_WAN_PHYMASK)

static void __init alfa_ap96_init(void)
{
	alfa_ap96_gpio_setup();

	ath79_register_mdio(0, ~ALFA_AP96_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = ALFA_AP96_WAN_PHYMASK;
	ath79_eth1_pll_data.pll_1000 = 0x110000;

	ath79_register_eth(0);

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = ALFA_AP96_LAN_PHYMASK;
	ath79_eth1_pll_data.pll_1000 = 0x110000;

	ath79_register_eth(1);

	ath79_register_pci();
	ath79_register_spi(&alfa_ap96_spi_data, alfa_ap96_spi_info,
			   ARRAY_SIZE(alfa_ap96_spi_info));

	ath79_register_gpio_keys_polled(-1, ALFA_AP96_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(alfa_ap96_gpio_keys),
					 alfa_ap96_gpio_keys);
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_ALFA_AP96, "ALFA-AP96", "ALFA Network AP96",
	     alfa_ap96_init);
