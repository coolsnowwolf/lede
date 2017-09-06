/*
 *  MikroTik RouterBOARD 4xx series support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/irq.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
#include <linux/mdio-gpio.h>
#else
#include <linux/platform_data/mdio-gpio.h> 
#endif
#include <linux/mmc/host.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/mmc_spi.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/rb4xx_cpld.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define RB4XX_GPIO_USER_LED	4
#define RB4XX_GPIO_RESET_SWITCH	7

#define RB4XX_GPIO_CPLD_BASE	32
#define RB4XX_GPIO_CPLD_LED1	(RB4XX_GPIO_CPLD_BASE + CPLD_GPIO_nLED1)
#define RB4XX_GPIO_CPLD_LED2	(RB4XX_GPIO_CPLD_BASE + CPLD_GPIO_nLED2)
#define RB4XX_GPIO_CPLD_LED3	(RB4XX_GPIO_CPLD_BASE + CPLD_GPIO_nLED3)
#define RB4XX_GPIO_CPLD_LED4	(RB4XX_GPIO_CPLD_BASE + CPLD_GPIO_nLED4)
#define RB4XX_GPIO_CPLD_LED5	(RB4XX_GPIO_CPLD_BASE + CPLD_GPIO_nLED5)

#define RB4XX_KEYS_POLL_INTERVAL	20	/* msecs */
#define RB4XX_KEYS_DEBOUNCE_INTERVAL	(3 * RB4XX_KEYS_POLL_INTERVAL)

static struct gpio_led rb4xx_leds_gpio[] __initdata = {
	{
		.name		= "rb4xx:yellow:user",
		.gpio		= RB4XX_GPIO_USER_LED,
		.active_low	= 0,
	}, {
		.name		= "rb4xx:green:led1",
		.gpio		= RB4XX_GPIO_CPLD_LED1,
		.active_low	= 1,
	}, {
		.name		= "rb4xx:green:led2",
		.gpio		= RB4XX_GPIO_CPLD_LED2,
		.active_low	= 1,
	}, {
		.name		= "rb4xx:green:led3",
		.gpio		= RB4XX_GPIO_CPLD_LED3,
		.active_low	= 1,
	}, {
		.name		= "rb4xx:green:led4",
		.gpio		= RB4XX_GPIO_CPLD_LED4,
		.active_low	= 1,
	}, {
		.name		= "rb4xx:green:led5",
		.gpio		= RB4XX_GPIO_CPLD_LED5,
		.active_low	= 0,
	},
};

static struct gpio_keys_button rb4xx_gpio_keys[] __initdata = {
	{
		.desc		= "reset_switch",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = RB4XX_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RB4XX_GPIO_RESET_SWITCH,
		.active_low	= 1,
	}
};

static struct platform_device rb4xx_nand_device = {
	.name	= "rb4xx-nand",
	.id	= -1,
};

static struct ath79_pci_irq rb4xx_pci_irqs[] __initdata = {
	{
		.slot	= 17,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(2),
	}, {
		.slot	= 18,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(0),
	}, {
		.slot	= 18,
		.pin	= 2,
		.irq	= ATH79_PCI_IRQ(1),
	}, {
		.slot	= 19,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(1),
	}, {
		.slot	= 19,
		.pin	= 2,
		.irq	= ATH79_PCI_IRQ(2),
	}, {
		.slot	= 20,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(2),
	}, {
		.slot	= 20,
		.pin	= 2,
		.irq	= ATH79_PCI_IRQ(0),
	}, {
		.slot	= 21,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(0),
	}, {
		.slot	= 22,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(1),
	}, {
		.slot	= 22,
		.pin	= 2,
		.irq	= ATH79_PCI_IRQ(2),
	}, {
		.slot	= 23,
		.pin	= 1,
		.irq	= ATH79_PCI_IRQ(2),
	}, {
		.slot	= 23,
		.pin	= 2,
		.irq	= ATH79_PCI_IRQ(0),
	}
};

static struct mtd_partition rb4xx_partitions[] = {
	{
		.name		= "routerboot",
		.offset		= 0,
		.size		= 0x0b000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "hard_config",
		.offset		= 0x0b000,
		.size		= 0x01000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "bios",
		.offset		= 0x0d000,
		.size		= 0x02000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "soft_config",
		.offset		= 0x0f000,
		.size		= 0x01000,
	}
};

static struct flash_platform_data rb4xx_flash_data = {
	.type		= "pm25lv512",
	.parts		= rb4xx_partitions,
	.nr_parts	= ARRAY_SIZE(rb4xx_partitions),
};

static struct rb4xx_cpld_platform_data rb4xx_cpld_data = {
	.gpio_base	= RB4XX_GPIO_CPLD_BASE,
};

static struct mmc_spi_platform_data rb4xx_mmc_data = {
	.ocr_mask	= MMC_VDD_32_33 | MMC_VDD_33_34,
};

static struct spi_board_info rb4xx_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
		.platform_data	= &rb4xx_flash_data,
	}, {
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 25000000,
		.modalias	= "spi-rb4xx-cpld",
		.platform_data	= &rb4xx_cpld_data,
	}
};

static struct spi_board_info rb4xx_microsd_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 2,
		.max_speed_hz	= 25000000,
		.modalias	= "mmc_spi",
		.platform_data	= &rb4xx_mmc_data,
	}
};


static struct resource rb4xx_spi_resources[] = {
	{
		.start	= AR71XX_SPI_BASE,
		.end	= AR71XX_SPI_BASE + AR71XX_SPI_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device rb4xx_spi_device = {
	.name		= "rb4xx-spi",
	.id		= -1,
	.resource	= rb4xx_spi_resources,
	.num_resources	= ARRAY_SIZE(rb4xx_spi_resources),
};

static void __init rb4xx_generic_setup(void)
{
	ath79_gpio_function_enable(AR71XX_GPIO_FUNC_SPI_CS1_EN |
				   AR71XX_GPIO_FUNC_SPI_CS2_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb4xx_leds_gpio),
					rb4xx_leds_gpio);

	ath79_register_gpio_keys_polled(-1, RB4XX_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rb4xx_gpio_keys),
					rb4xx_gpio_keys);

	spi_register_board_info(rb4xx_spi_info, ARRAY_SIZE(rb4xx_spi_info));
	platform_device_register(&rb4xx_spi_device);
	platform_device_register(&rb4xx_nand_device);
}

static void __init rb411_setup(void)
{
	rb4xx_generic_setup();
	spi_register_board_info(rb4xx_microsd_info,
				ARRAY_SIZE(rb4xx_microsd_info));

	ath79_register_mdio(0, 0xfffffffc);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = 0x00000003;

	ath79_register_eth(0);

	ath79_pci_set_irq_map(ARRAY_SIZE(rb4xx_pci_irqs), rb4xx_pci_irqs);
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_RB_411, "411", "MikroTik RouterBOARD 411/A/AH",
	     rb411_setup);

static void __init rb411u_setup(void)
{
	rb411_setup();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RB_411U, "411U", "MikroTik RouterBOARD 411U",
	     rb411u_setup);

#define RB433_LAN_PHYMASK	BIT(0)
#define RB433_WAN_PHYMASK	BIT(4)
#define RB433_MDIO_PHYMASK	(RB433_LAN_PHYMASK | RB433_WAN_PHYMASK)

static void __init rb433_setup(void)
{
	rb4xx_generic_setup();
	spi_register_board_info(rb4xx_microsd_info,
				ARRAY_SIZE(rb4xx_microsd_info));

	ath79_register_mdio(0, ~RB433_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = RB433_LAN_PHYMASK;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = RB433_WAN_PHYMASK;

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_pci_set_irq_map(ARRAY_SIZE(rb4xx_pci_irqs), rb4xx_pci_irqs);
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_RB_433, "433", "MikroTik RouterBOARD 433/AH",
	     rb433_setup);

static void __init rb433u_setup(void)
{
	rb433_setup();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RB_433U, "433U", "MikroTik RouterBOARD 433UAH",
	     rb433u_setup);

static void __init rb435g_setup(void)
{
	rb4xx_generic_setup();

	spi_register_board_info(rb4xx_microsd_info,
				ARRAY_SIZE(rb4xx_microsd_info));

	ath79_register_mdio(0, ~RB433_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = RB433_LAN_PHYMASK;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = RB433_WAN_PHYMASK;

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_pci_set_irq_map(ARRAY_SIZE(rb4xx_pci_irqs), rb4xx_pci_irqs);
	ath79_register_pci();

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RB_435G, "435G", "MikroTik RouterBOARD 435G",
	     rb435g_setup);

#define RB450_LAN_PHYMASK	BIT(0)
#define RB450_WAN_PHYMASK	BIT(4)
#define RB450_MDIO_PHYMASK	(RB450_LAN_PHYMASK | RB450_WAN_PHYMASK)

static void __init rb450_generic_setup(int gige)
{
	rb4xx_generic_setup();
	ath79_register_mdio(0, ~RB450_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 1);
	ath79_eth0_data.phy_if_mode = (gige) ?
		PHY_INTERFACE_MODE_RGMII : PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = RB450_LAN_PHYMASK;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);
	ath79_eth1_data.phy_if_mode = (gige) ?
		PHY_INTERFACE_MODE_RGMII : PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = RB450_WAN_PHYMASK;

	ath79_register_eth(1);
	ath79_register_eth(0);
}

static void __init rb450_setup(void)
{
	rb450_generic_setup(0);
}

MIPS_MACHINE(ATH79_MACH_RB_450, "450", "MikroTik RouterBOARD 450",
	     rb450_setup);

static void __init rb450g_setup(void)
{
	rb450_generic_setup(1);
	spi_register_board_info(rb4xx_microsd_info,
				ARRAY_SIZE(rb4xx_microsd_info));
}

MIPS_MACHINE(ATH79_MACH_RB_450G, "450G", "MikroTik RouterBOARD 450G",
	     rb450g_setup);

static void __init rb493_setup(void)
{
	rb4xx_generic_setup();

	ath79_register_mdio(0, 0x3fffff00);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x00000001;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_pci_set_irq_map(ARRAY_SIZE(rb4xx_pci_irqs), rb4xx_pci_irqs);
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_RB_493, "493", "MikroTik RouterBOARD 493/AH",
	     rb493_setup);

#define RB493G_GPIO_MDIO_MDC		7
#define RB493G_GPIO_MDIO_DATA		8

#define RB493G_MDIO_PHYMASK		BIT(0)

static struct mdio_gpio_platform_data rb493g_mdio_data = {
	.mdc		= RB493G_GPIO_MDIO_MDC,
	.mdio		= RB493G_GPIO_MDIO_DATA,

	.phy_mask	= ~RB493G_MDIO_PHYMASK,
};

static struct platform_device rb493g_mdio_device = {
	.name 		= "mdio-gpio",
	.id 		= -1,
	.dev 		= {
		.platform_data	= &rb493g_mdio_data,
	},
};

static void __init rb493g_setup(void)
{
	ath79_gpio_function_enable(AR71XX_GPIO_FUNC_SPI_CS1_EN |
				    AR71XX_GPIO_FUNC_SPI_CS2_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb4xx_leds_gpio),
				    rb4xx_leds_gpio);

	spi_register_board_info(rb4xx_spi_info, ARRAY_SIZE(rb4xx_spi_info));
	spi_register_board_info(rb4xx_microsd_info,
				ARRAY_SIZE(rb4xx_microsd_info));

	platform_device_register(&rb4xx_spi_device);
	platform_device_register(&rb4xx_nand_device);

	ath79_register_mdio(0, ~RB493G_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = RB493G_MDIO_PHYMASK;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.mii_bus_dev = &rb493g_mdio_device.dev;
	ath79_eth1_data.phy_mask = RB493G_MDIO_PHYMASK;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	platform_device_register(&rb493g_mdio_device);

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_usb();

	ath79_pci_set_irq_map(ARRAY_SIZE(rb4xx_pci_irqs), rb4xx_pci_irqs);
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_RB_493G, "493G", "MikroTik RouterBOARD 493G",
	     rb493g_setup);
