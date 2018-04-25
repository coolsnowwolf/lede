/*
 * Gateworks Corporation Laguna Platform
 *
 * Copyright 2000 Deep Blue Solutions Ltd
 * Copyright 2008 ARM Limited
 * Copyright 2008 Cavium Networks
 *		  Scott Shu
 * Copyright 2010 MontaVista Software, LLC.
 *		  Anton Vorontsov <avorontsov@mvista.com>
 * Copyright 2011 Gateworks Corporation
 *		  Chris Lang <clang@gateworks.com>
 * Copyright 2012-2013 Gateworks Corporation
 *		  Tim Harvey <tharvey@gateworks.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/dma-mapping.h>
#include <linux/serial_core.h>
#include <linux/serial_8250.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/leds.h>
#include <linux/i2c.h>
#include <linux/platform_data/at24.h>
#include <linux/platform_data/pca953x.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/if_ether.h>
#include <linux/pps-gpio.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/platform_data/cns3xxx.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <mach/gpio.h>
#include "core.h"
#include "devices.h"
#include "cns3xxx.h"
#include "pm.h"

#define ARRAY_AND_SIZE(x)       (x), ARRAY_SIZE(x)

// Config 1 Bitmap
#define ETH0_LOAD           BIT(0)
#define ETH1_LOAD           BIT(1)
#define ETH2_LOAD           BIT(2)
#define SATA0_LOAD          BIT(3)
#define SATA1_LOAD          BIT(4)
#define PCM_LOAD            BIT(5)
#define I2S_LOAD            BIT(6)
#define SPI0_LOAD           BIT(7)
#define SPI1_LOAD           BIT(8)
#define PCIE0_LOAD          BIT(9)
#define PCIE1_LOAD          BIT(10)
#define USB0_LOAD           BIT(11)
#define USB1_LOAD           BIT(12)
#define USB1_ROUTE          BIT(13)
#define SD_LOAD             BIT(14)
#define UART0_LOAD          BIT(15)
#define UART1_LOAD          BIT(16)
#define UART2_LOAD          BIT(17)
#define MPCI0_LOAD          BIT(18)
#define MPCI1_LOAD          BIT(19)
#define MPCI2_LOAD          BIT(20)
#define MPCI3_LOAD          BIT(21)
#define FP_BUT_LOAD         BIT(22)
#define FP_BUT_HEADER_LOAD  BIT(23)
#define FP_LED_LOAD         BIT(24)
#define FP_LED_HEADER_LOAD  BIT(25)
#define FP_TAMPER_LOAD      BIT(26)
#define HEADER_33V_LOAD     BIT(27)
#define SATA_POWER_LOAD     BIT(28)
#define FP_POWER_LOAD       BIT(29)
#define GPIO_HEADER_LOAD    BIT(30)
#define GSP_BAT_LOAD        BIT(31)

// Config 2 Bitmap
#define FAN_LOAD            BIT(0)
#define SPI_FLASH_LOAD      BIT(1)
#define NOR_FLASH_LOAD      BIT(2)
#define GPS_LOAD            BIT(3)
#define SUPPLY_5V_LOAD      BIT(6)
#define SUPPLY_33V_LOAD     BIT(7)

struct laguna_board_info {
	char model[16];
	u32 config_bitmap;
	u32 config2_bitmap;
	u8 nor_flash_size;
	u8 spi_flash_size;
};

static struct laguna_board_info laguna_info __initdata;

/*
 * NOR Flash
 */
static struct mtd_partition laguna_nor_partitions[] = {
	{
		.name		= "uboot",
		.size		= SZ_256K,
		.offset		= 0,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "params",
		.size		= SZ_128K,
		.offset		= SZ_256K,
	}, {
		.name		= "firmware",
		.offset		= SZ_256K + SZ_128K,
	},
};

static struct physmap_flash_data laguna_nor_pdata = {
	.width = 2,
	.parts = laguna_nor_partitions,
	.nr_parts = ARRAY_SIZE(laguna_nor_partitions),
};

static struct resource laguna_nor_res = {
	.start = CNS3XXX_FLASH_BASE,
	.end = CNS3XXX_FLASH_BASE + SZ_128M - 1,
	.flags = IORESOURCE_MEM | IORESOURCE_MEM_32BIT,
};

static struct platform_device laguna_nor_pdev = {
	.name = "physmap-flash",
	.id = 0,
	.resource = &laguna_nor_res,
	.num_resources = 1,
	.dev = {
		.platform_data = &laguna_nor_pdata,
	},
};

/*
 * SPI
 */
static struct mtd_partition laguna_spi_partitions[] = {
	{
		.name		= "uboot",
		.size		= SZ_256K,
		.offset		= 0,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "params",
		.size		= SZ_256K,
		.offset		= SZ_256K,
	}, {
		.name		= "firmware",
		.offset		= SZ_512K,
	},
};

static struct flash_platform_data laguna_spi_pdata = {
	.parts = laguna_spi_partitions,
	.nr_parts = ARRAY_SIZE(laguna_spi_partitions),
};

static struct spi_board_info __initdata laguna_spi_devices[] = {
	{
		.modalias = "m25p80",
		.platform_data = &laguna_spi_pdata,
		.max_speed_hz = 50000000,
		.bus_num = 1,
		.chip_select = 0,
	},
};

static struct resource laguna_spi_resource = {
	.start    = CNS3XXX_SSP_BASE + 0x40,
	.end      = CNS3XXX_SSP_BASE + 0x6f,
	.flags    = IORESOURCE_MEM,
};

static struct platform_device laguna_spi_controller = {
	.name = "cns3xxx_spi",
	.resource = &laguna_spi_resource,
	.num_resources = 1,
};

/*
 * LED's
 */
static struct gpio_led laguna_gpio_leds[] = {
	{
		.name = "user1", /* Green Led */
		.gpio = 115,
		.active_low = 1,
	},{
		.name = "user2", /* Red Led */
		.gpio = 114,
		.active_low = 1,
	},{
		.name = "pwr1", /* Green Led */
		.gpio = 116,
		.active_low = 1,
	},{
		.name = "pwr2", /* Yellow Led */
		.gpio = 117,
		.active_low = 1,
	},{
		.name = "txd1", /* Green Led */
		.gpio = 118,
		.active_low = 1,
	},{
		.name = "txd2", /* Yellow Led */
		.gpio = 119,
		.active_low = 1,
	},{
		.name = "rxd1", /* Green Led */
		.gpio = 120,
		.active_low = 1,
	},{
		.name = "rxd2", /* Yellow Led */
		.gpio = 121,
		.active_low = 1,
	},{
		.name = "ser1", /* Green Led */
		.gpio = 122,
		.active_low = 1,
	},{
		.name = "ser2", /* Yellow Led */
		.gpio = 123,
		.active_low = 1,
	},{
		.name = "enet1", /* Green Led */
		.gpio = 124,
		.active_low = 1,
	},{
		.name = "enet2", /* Yellow Led */
		.gpio = 125,
		.active_low = 1,
	},{
		.name = "sig1_1", /* Green Led */
		.gpio = 126,
		.active_low = 1,
	},{
		.name = "sig1_2", /* Yellow Led */
		.gpio = 127,
		.active_low = 1,
	},{
		.name = "sig2_1", /* Green Led */
		.gpio = 128,
		.active_low = 1,
	},{
		.name = "sig2_2", /* Yellow Led */
		.gpio = 129,
		.active_low = 1,
	},{
		.name = "sig3_1", /* Green Led */
		.gpio = 130,
		.active_low = 1,
	},{
		.name = "sig3_2", /* Yellow Led */
		.gpio = 131,
		.active_low = 1,
	},{
		.name = "net1", /*Green Led */
		.gpio = 109,
		.active_low = 1,
	},{
		.name = "net2", /* Red Led */
		.gpio = 110,
		.active_low = 1,
	},{
		.name = "mod1", /* Green Led */
		.gpio = 111,
		.active_low = 1,
	},{
		.name = "mod2", /* Red Led */
		.gpio = 112,
		.active_low = 1,
	},
};

static struct gpio_led_platform_data laguna_gpio_leds_data = {
	.num_leds = 22,
	.leds = laguna_gpio_leds,
};

static struct platform_device laguna_gpio_leds_device = {
	.name = "leds-gpio",
	.id = PLATFORM_DEVID_NONE,
	.dev.platform_data = &laguna_gpio_leds_data,
};

/*
 * Ethernet
 */
static struct cns3xxx_plat_info laguna_net_data = {
	.ports = 0,
	.phy = {
		0,
		1,
		2,
	},
};

static struct resource laguna_net_resource[] = {
	{
		.name = "eth0_mem",
		.start = CNS3XXX_SWITCH_BASE,
		.end = CNS3XXX_SWITCH_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM
	}, {
		.name = "eth_rx",
		.start = IRQ_CNS3XXX_SW_R0RXC,
		.end = IRQ_CNS3XXX_SW_R0RXC,
		.flags = IORESOURCE_IRQ
	}, {
		.name = "eth_stat",
		.start = IRQ_CNS3XXX_SW_STATUS,
		.end = IRQ_CNS3XXX_SW_STATUS,
		.flags = IORESOURCE_IRQ
	}
};

static u64 laguna_net_dmamask = DMA_BIT_MASK(32);
static struct platform_device laguna_net_device = {
	.name = "cns3xxx_eth",
	.id = 0,
	.resource = laguna_net_resource,
	.num_resources = ARRAY_SIZE(laguna_net_resource),
	.dev = {
		.dma_mask = &laguna_net_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data = &laguna_net_data,
	}
};

/*
 * UART
 */
static void __init laguna_early_serial_setup(void)
{
#ifdef CONFIG_SERIAL_8250_CONSOLE
	static struct uart_port laguna_serial_port = {
		.membase        = (void __iomem *)CNS3XXX_UART0_BASE_VIRT,
		.mapbase        = CNS3XXX_UART0_BASE,
		.irq            = IRQ_CNS3XXX_UART0,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE,
		.regshift       = 2,
		.uartclk        = 24000000,
		.line           = 0,
		.type           = PORT_16550A,
		.fifosize       = 16,
	};

	early_serial_setup(&laguna_serial_port);
#endif
}

static struct resource laguna_uart_resources[] = {
	{
		.start = CNS3XXX_UART0_BASE,
		.end   = CNS3XXX_UART0_BASE + SZ_4K - 1,
		.flags    = IORESOURCE_MEM
	},{
		.start = CNS3XXX_UART1_BASE,
		.end   = CNS3XXX_UART1_BASE + SZ_4K - 1,
		.flags    = IORESOURCE_MEM
	},{
		.start = CNS3XXX_UART2_BASE,
		.end   = CNS3XXX_UART2_BASE + SZ_4K - 1,
		.flags    = IORESOURCE_MEM
	},
};

static struct plat_serial8250_port laguna_uart_data[] = {
	{
		.mapbase        = (CNS3XXX_UART0_BASE),
		.irq            = IRQ_CNS3XXX_UART0,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_IOREMAP,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type           = PORT_16550A,
	},{
		.mapbase        = (CNS3XXX_UART1_BASE),
		.irq            = IRQ_CNS3XXX_UART1,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_IOREMAP,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type           = PORT_16550A,
	},{
		.mapbase        = (CNS3XXX_UART2_BASE),
		.irq            = IRQ_CNS3XXX_UART2,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_FIXED_TYPE | UPF_IOREMAP,
		.regshift       = 2,
		.uartclk        = 24000000,
		.type           = PORT_16550A,
	},
	{ },
};

static struct platform_device laguna_uart = {
	.name     = "serial8250",
	.id     = PLAT8250_DEV_PLATFORM,
	.dev.platform_data  = laguna_uart_data,
	.num_resources    = 3,
	.resource   = laguna_uart_resources
};

/*
 * USB
 */
static struct resource cns3xxx_usb_ehci_resources[] = {
	[0] = {
		.start = CNS3XXX_USB_BASE,
		.end   = CNS3XXX_USB_BASE + SZ_16M - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CNS3XXX_USB_EHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 cns3xxx_usb_ehci_dma_mask = DMA_BIT_MASK(32);

static int csn3xxx_usb_power_on(struct platform_device *pdev)
{
	/*
	 * EHCI and OHCI share the same clock and power,
	 * resetting twice would cause the 1st controller been reset.
	 * Therefore only do power up  at the first up device, and
	 * power down at the last down device.
	 *
	 * Set USB AHB INCR length to 16
	 */
	if (atomic_inc_return(&usb_pwr_ref) == 1) {
		cns3xxx_pwr_power_up(1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB);
		cns3xxx_pwr_clk_en(1 << PM_CLK_GATE_REG_OFFSET_USB_HOST);
		cns3xxx_pwr_soft_rst(1 << PM_SOFT_RST_REG_OFFST_USB_HOST);
		__raw_writel((__raw_readl(MISC_CHIP_CONFIG_REG) | (0X2 << 24)),
			MISC_CHIP_CONFIG_REG);
	}

	return 0;
}

static void csn3xxx_usb_power_off(struct platform_device *pdev)
{
	/*
	 * EHCI and OHCI share the same clock and power,
	 * resetting twice would cause the 1st controller been reset.
	 * Therefore only do power up  at the first up device, and
	 * power down at the last down device.
	 */
	if (atomic_dec_return(&usb_pwr_ref) == 0)
		cns3xxx_pwr_clk_dis(1 << PM_CLK_GATE_REG_OFFSET_USB_HOST);
}

static struct usb_ehci_pdata cns3xxx_usb_ehci_pdata = {
	.power_on	= csn3xxx_usb_power_on,
	.power_off	= csn3xxx_usb_power_off,
};

static struct platform_device cns3xxx_usb_ehci_device = {
	.name          = "ehci-platform",
	.num_resources = ARRAY_SIZE(cns3xxx_usb_ehci_resources),
	.resource      = cns3xxx_usb_ehci_resources,
	.dev           = {
		.dma_mask          = &cns3xxx_usb_ehci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data     = &cns3xxx_usb_ehci_pdata,
	},
};

static struct resource cns3xxx_usb_ohci_resources[] = {
	[0] = {
		.start = CNS3XXX_USB_OHCI_BASE,
		.end   = CNS3XXX_USB_OHCI_BASE + SZ_16M - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CNS3XXX_USB_OHCI,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 cns3xxx_usb_ohci_dma_mask = DMA_BIT_MASK(32);

static struct usb_ohci_pdata cns3xxx_usb_ohci_pdata = {
	.num_ports	= 1,
	.power_on	= csn3xxx_usb_power_on,
	.power_off	= csn3xxx_usb_power_off,
};

static struct platform_device cns3xxx_usb_ohci_device = {
	.name          = "ohci-platform",
	.num_resources = ARRAY_SIZE(cns3xxx_usb_ohci_resources),
	.resource      = cns3xxx_usb_ohci_resources,
	.dev           = {
		.dma_mask          = &cns3xxx_usb_ohci_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
		.platform_data	   = &cns3xxx_usb_ohci_pdata,
	},
};

static struct resource cns3xxx_usb_otg_resources[] = {
	[0] = {
		.start = CNS3XXX_USBOTG_BASE,
		.end   = CNS3XXX_USBOTG_BASE + SZ_16M - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_CNS3XXX_USB_OTG,
		.flags = IORESOURCE_IRQ,
	},
};

static u64 cns3xxx_usb_otg_dma_mask = DMA_BIT_MASK(32);

static struct platform_device cns3xxx_usb_otg_device = {
	.name          = "dwc2",
	.num_resources = ARRAY_SIZE(cns3xxx_usb_otg_resources),
	.resource      = cns3xxx_usb_otg_resources,
	.dev           = {
		.dma_mask          = &cns3xxx_usb_otg_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
	},
};

/*
 * I2C
 */
static struct resource laguna_i2c_resource[] = {
	{
		.start    = CNS3XXX_SSP_BASE + 0x20,
		.end      = CNS3XXX_SSP_BASE + 0x3f,
		.flags    = IORESOURCE_MEM,
	},{
		.start    = IRQ_CNS3XXX_I2C,
		.flags    = IORESOURCE_IRQ,
	},
};

static struct platform_device laguna_i2c_controller = {
	.name   = "cns3xxx-i2c",
	.num_resources  = 2,
	.resource = laguna_i2c_resource,
};

static struct nvmem_device *at24_nvmem;

static void at24_setup(struct nvmem_device *mem_acc, void *context)
{
	char buf[16];

	at24_nvmem = mem_acc;

	/* Read MAC addresses */
	if (nvmem_device_read(at24_nvmem, 0x100, 6, buf) == 6)
		memcpy(&laguna_net_data.hwaddr[0], buf, ETH_ALEN);
	if (nvmem_device_read(at24_nvmem, 0x106, 6, buf) == 6)
		memcpy(&laguna_net_data.hwaddr[1], buf, ETH_ALEN);
	if (nvmem_device_read(at24_nvmem, 0x10C, 6, buf) == 6)
		memcpy(&laguna_net_data.hwaddr[2], buf, ETH_ALEN);
	if (nvmem_device_read(at24_nvmem, 0x112, 6, buf) == 6)
		memcpy(&laguna_net_data.hwaddr[3], buf, ETH_ALEN);

	/* Read out Model Information */
	if (nvmem_device_read(at24_nvmem, 0x130, 16, buf) == 16)
		memcpy(&laguna_info.model, buf, 16);
	if (nvmem_device_read(at24_nvmem, 0x140, 1, buf) == 1)
		memcpy(&laguna_info.nor_flash_size, buf, 1);
	if (nvmem_device_read(at24_nvmem, 0x141, 1, buf) == 1)
		memcpy(&laguna_info.spi_flash_size, buf, 1);
	if (nvmem_device_read(at24_nvmem, 0x142, 4, buf) == 4)
		memcpy(&laguna_info.config_bitmap, buf, 4);
	if (nvmem_device_read(at24_nvmem, 0x146, 4, buf) == 4)
		memcpy(&laguna_info.config2_bitmap, buf, 4);
};

static struct at24_platform_data laguna_eeprom_info = {
	.byte_len = 1024,
	.page_size = 16,
	.flags = AT24_FLAG_READONLY,
	.setup = at24_setup,
};

static struct pca953x_platform_data laguna_pca_data = {
 	.gpio_base = 100,
	.irq_base = -1,
};

static struct pca953x_platform_data laguna_pca2_data = {
 	.gpio_base = 116,
	.irq_base = -1,
};

static struct i2c_board_info __initdata laguna_i2c_devices[] = {
	{
		I2C_BOARD_INFO("pca9555", 0x23),
		.platform_data = &laguna_pca_data,
	},{
		I2C_BOARD_INFO("pca9555", 0x27),
		.platform_data = &laguna_pca2_data,
	},{
		I2C_BOARD_INFO("gsp", 0x29),
	},{
		I2C_BOARD_INFO ("24c08",0x50),
		.platform_data = &laguna_eeprom_info,
	},{
		I2C_BOARD_INFO("ds1672", 0x68),
	},
};

/*
 * Watchdog
 */

static struct resource laguna_watchdog_resources[] = {
	[0] = {
		.start	= CNS3XXX_TC11MP_TWD_BASE + 0x100, // CPU0 watchdog
		.end	= CNS3XXX_TC11MP_TWD_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device laguna_watchdog = {
	.name		= "mpcore_wdt",
	.id		= PLATFORM_DEVID_NONE,
	.num_resources	= ARRAY_SIZE(laguna_watchdog_resources),
	.resource	= laguna_watchdog_resources,
};

/*
 * GPS PPS
 */
static struct pps_gpio_platform_data laguna_pps_data = {
	.gpio_pin = 0,
	.gpio_label = "GPS_PPS",
	.assert_falling_edge = 0,
	.capture_clear = 0,
};

static struct platform_device laguna_pps_device = {
	.name = "pps-gpio",
	.id = PLATFORM_DEVID_NONE,
	.dev.platform_data = &laguna_pps_data,
};

/*
 * GPIO
 */

static struct gpio laguna_gpio_gw2391[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   2, GPIOF_IN           , "*USB_FAULT#" },
	{   5, GPIOF_OUT_INIT_LOW , "*USB0_PCI_SEL" },
	{   6, GPIOF_OUT_INIT_HIGH, "*USB_VBUS_EN" },
	{   7, GPIOF_OUT_INIT_LOW , "*USB1_PCI_SEL" },
	{   8, GPIOF_OUT_INIT_HIGH, "*PERST#" },
	{   9, GPIOF_OUT_INIT_LOW , "*FP_SER_EN#" },
	{ 100, GPIOF_IN           , "*USER_PB#" },
	{ 103, GPIOF_OUT_INIT_HIGH, "*V5_EN" },
	{ 108, GPIOF_IN           , "DIO0" },
	{ 109, GPIOF_IN           , "DIO1" },
	{ 110, GPIOF_IN           , "DIO2" },
	{ 111, GPIOF_IN           , "DIO3" },
	{ 112, GPIOF_IN           , "DIO4" },
};

static struct gpio laguna_gpio_gw2388[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   3, GPIOF_IN           , "*USB_FAULT#" },
	{   6, GPIOF_OUT_INIT_HIGH, "*USB_VBUS_EN" },
	{   7, GPIOF_OUT_INIT_LOW , "*GSM_SEL0" },
	{   8, GPIOF_OUT_INIT_LOW , "*GSM_SEL1" },
	{   9, GPIOF_OUT_INIT_LOW , "*FP_SER_EN" },
	{ 100, GPIOF_OUT_INIT_HIGH, "*USER_PB#" },
	{ 108, GPIOF_IN           , "DIO0" },
	{ 109, GPIOF_IN           , "DIO1" },
	{ 110, GPIOF_IN           , "DIO2" },
	{ 111, GPIOF_IN           , "DIO3" },
	{ 112, GPIOF_IN           , "DIO4" },
};

static struct gpio laguna_gpio_gw2387[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   2, GPIOF_IN           , "*USB_FAULT#" },
	{   5, GPIOF_OUT_INIT_LOW , "*USB_PCI_SEL" },
	{   6, GPIOF_OUT_INIT_HIGH, "*USB_VBUS_EN" },
	{   7, GPIOF_OUT_INIT_LOW , "*GSM_SEL0" },
	{   8, GPIOF_OUT_INIT_LOW , "*GSM_SEL1" },
	{   9, GPIOF_OUT_INIT_LOW , "*FP_SER_EN" },
	{ 100, GPIOF_IN           , "*USER_PB#" },
	{ 103, GPIOF_OUT_INIT_HIGH, "*V5_EN" },
	{ 108, GPIOF_IN           , "DIO0" },
	{ 109, GPIOF_IN           , "DIO1" },
	{ 110, GPIOF_IN           , "DIO2" },
	{ 111, GPIOF_IN           , "DIO3" },
	{ 112, GPIOF_IN           , "DIO4" },
	{ 113, GPIOF_IN           , "DIO5" },
};

static struct gpio laguna_gpio_gw2386[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   2, GPIOF_IN           , "*USB_FAULT#" },
	{   6, GPIOF_OUT_INIT_LOW , "*USB_PCI_SEL" },
	{   7, GPIOF_OUT_INIT_LOW , "*GSM_SEL0" },
	{   8, GPIOF_OUT_INIT_LOW , "*GSM_SEL1" },
	{   9, GPIOF_OUT_INIT_LOW , "*FP_SER_EN" },
	{ 108, GPIOF_IN           , "DIO0" },
	{ 109, GPIOF_IN           , "DIO1" },
	{ 110, GPIOF_IN           , "DIO2" },
	{ 111, GPIOF_IN           , "DIO3" },
	{ 112, GPIOF_IN           , "DIO4" },
	{ 113, GPIOF_IN           , "DIO5" },
};

static struct gpio laguna_gpio_gw2385[] = {
	{   0, GPIOF_IN           , "*GSC_IRQ#" },
	{   1, GPIOF_OUT_INIT_HIGH, "*USB_HST_VBUS_EN" },
	{   2, GPIOF_IN           , "*USB_HST_FAULT#" },
	{   5, GPIOF_IN           , "*USB_OTG_FAULT#" },
	{   6, GPIOF_OUT_INIT_LOW , "*USB_HST_PCI_SEL" },
	{   7, GPIOF_OUT_INIT_LOW , "*GSM_SEL0" },
	{   8, GPIOF_OUT_INIT_LOW , "*GSM_SEL1" },
	{   9, GPIOF_OUT_INIT_LOW , "*SER_EN" },
	{  10, GPIOF_IN,            "*USER_PB#" },
	{  11, GPIOF_OUT_INIT_HIGH, "*PERST#" },
	{ 100, GPIOF_IN           , "*USER_PB#" },
	{ 103, GPIOF_OUT_INIT_HIGH, "V5_EN" },
};

static struct gpio laguna_gpio_gw2384[] = {
	{   0, GPIOF_IN           , "*GSC_IRQ#" },
	{   1, GPIOF_OUT_INIT_HIGH, "*USB_HST_VBUS_EN" },
	{   2, GPIOF_IN           , "*USB_HST_FAULT#" },
	{   5, GPIOF_IN           , "*USB_OTG_FAULT#" },
	{   6, GPIOF_OUT_INIT_LOW , "*USB_HST_PCI_SEL" },
	{   7, GPIOF_OUT_INIT_LOW , "*GSM_SEL0" },
	{   8, GPIOF_OUT_INIT_LOW , "*GSM_SEL1" },
	{   9, GPIOF_OUT_INIT_LOW , "*FP_SER_EN" },
	{  12, GPIOF_OUT_INIT_LOW , "J10_DIOLED0" },
	{  13, GPIOF_OUT_INIT_HIGH, "*I2CMUX_RST#" },
	{  14, GPIOF_OUT_INIT_LOW , "J10_DIOLED1" },
	{  15, GPIOF_OUT_INIT_LOW , "J10_DIOLED2" },
	{ 100, GPIOF_IN           , "*USER_PB#" },
	{ 103, GPIOF_OUT_INIT_HIGH, "V5_EN" },
	{ 108, GPIOF_IN           , "J9_DIOGSC0" },
};

static struct gpio laguna_gpio_gw2383[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   2, GPIOF_OUT_INIT_HIGH, "*PCIE_RST#" },
	{   3, GPIOF_IN           , "GPIO0" },
	{   8, GPIOF_IN           , "GPIO1" },
	{ 100, GPIOF_IN           , "DIO0" },
	{ 101, GPIOF_IN           , "DIO1" },
	{ 108, GPIOF_IN           , "*USER_PB#" },
};

static struct gpio laguna_gpio_gw2382[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   2, GPIOF_OUT_INIT_HIGH, "*PCIE_RST#" },
	{   3, GPIOF_IN           , "GPIO0" },
	{   4, GPIOF_IN           , "GPIO1" },
	{   9, GPIOF_OUT_INIT_HIGH, "*USB_VBUS_EN" },
	{  10, GPIOF_OUT_INIT_HIGH, "*USB_PCI_SEL#" },
	{ 100, GPIOF_IN           , "DIO0" },
	{ 101, GPIOF_IN           , "DIO1" },
	{ 108, GPIOF_IN           , "*USER_PB#" },
};

static struct gpio laguna_gpio_gw2380[] = {
	{   0, GPIOF_IN           , "*GPS_PPS" },
	{   1, GPIOF_IN           , "*GSC_IRQ#" },
	{   3, GPIOF_IN           , "GPIO0" },
	{   8, GPIOF_IN           , "GPIO1" },
	{ 100, GPIOF_IN           , "DIO0" },
	{ 101, GPIOF_IN           , "DIO1" },
	{ 102, GPIOF_IN           , "DIO2" },
	{ 103, GPIOF_IN           , "DIO3" },
	{ 108, GPIOF_IN           , "*USER_PB#" },
};

/*
 * Initialization
 */
static void __init laguna_init(void)
{
	struct clk *clk;
	u32 __iomem *reg;

	clk = clk_register_fixed_rate(NULL, "cpu", NULL,
				      CLK_IGNORE_UNUSED,
				      cns3xxx_cpu_clock() * (1000000 / 8));
	clk_register_clkdev(clk, "cpu", NULL);

	platform_device_register(&laguna_watchdog);

	platform_device_register(&laguna_i2c_controller);

	/* Set I2C 0-3 drive strength to 21 mA */
	reg = MISC_IO_PAD_DRIVE_STRENGTH_CTRL_B;
	*reg |= 0x300;

	/* Enable SCL/SDA for I2C */
	reg = MISC_GPIOB_PIN_ENABLE_REG;
	*reg |= BIT(12) | BIT(13);

	/* Enable MMC/SD pins */
	*reg |= BIT(7) | BIT(8) | BIT(9) | BIT(10) | BIT(11);

	cns3xxx_pwr_clk_en(1 << PM_CLK_GATE_REG_OFFSET_SPI_PCM_I2C);
	cns3xxx_pwr_power_up(1 << PM_CLK_GATE_REG_OFFSET_SPI_PCM_I2C);
	cns3xxx_pwr_soft_rst(1 << PM_CLK_GATE_REG_OFFSET_SPI_PCM_I2C);

	cns3xxx_pwr_clk_en(CNS3XXX_PWR_CLK_EN(SPI_PCM_I2C));
	cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(SPI_PCM_I2C));

	i2c_register_board_info(0, ARRAY_AND_SIZE(laguna_i2c_devices));

	pm_power_off = cns3xxx_power_off;
}

static struct map_desc laguna_io_desc[] __initdata = {
	{
		.virtual	= CNS3XXX_UART0_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_UART0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	},
};

static void __init laguna_map_io(void)
{
	cns3xxx_map_io();
	iotable_init(ARRAY_AND_SIZE(laguna_io_desc));
	laguna_early_serial_setup();
}

static int laguna_register_gpio(struct gpio *array, size_t num)
{
	int i, err, ret;

	ret = 0;
	for (i = 0; i < num; i++, array++) {
		const char *label = array->label;
		if (label[0] == '*')
			label++;
		err = gpio_request_one(array->gpio, array->flags, label);
		if (err)
			ret = err;
		else {
			err = gpio_export(array->gpio, array->label[0] != '*');
		}
	}
	return ret;
}

/* allow disabling of external isolated PCIe IRQs */
static int cns3xxx_pciextirq = 1;
static int __init cns3xxx_pciextirq_disable(char *s)
{
      cns3xxx_pciextirq = 0;
      return 1;
}
__setup("noextirq", cns3xxx_pciextirq_disable);

static int __init laguna_pcie_init_irq(void)
{
	u32 __iomem *mem = (void __iomem *)(CNS3XXX_GPIOB_BASE_VIRT + 0x0004);
	u32 reg = (__raw_readl(mem) >> 26) & 0xf;
	int irqs[] = {
		IRQ_CNS3XXX_EXTERNAL_PIN0,
		IRQ_CNS3XXX_EXTERNAL_PIN1,
		IRQ_CNS3XXX_EXTERNAL_PIN2,
		154,
	};

	if (!machine_is_gw2388())
		return 0;

	/* Verify GPIOB[26:29] == 0001b indicating support for ext irqs */
	if (cns3xxx_pciextirq && reg != 1)
		cns3xxx_pciextirq = 0;

	if (cns3xxx_pciextirq) {
		printk("laguna: using isolated PCI interrupts:"
		       " irq%d/irq%d/irq%d/irq%d\n",
		       irqs[0], irqs[1], irqs[2], irqs[3]);
		cns3xxx_pcie_set_irqs(0, irqs);
	} else {
		printk("laguna: using shared PCI interrupts: irq%d\n",
		       IRQ_CNS3XXX_PCIE0_DEVICE);
	}

	return 0;
}
subsys_initcall(laguna_pcie_init_irq);

static int __init laguna_model_setup(void)
{
	u32 __iomem *mem;
	u32 reg;

	if (!machine_is_gw2388())
		return 0;

	printk("Running on Gateworks Laguna %s\n", laguna_info.model);
	cns3xxx_gpio_init( 0, 32, CNS3XXX_GPIOA_BASE_VIRT, IRQ_CNS3XXX_GPIOA,
		NR_IRQS_CNS3XXX);

	/*
	 * If pcie external interrupts are supported and desired
	 * configure IRQ types and configure pin function.
	 * Note that cns3xxx_pciextirq is enabled by default, but can be
	 * unset via the 'noextirq' kernel param or by laguna_pcie_init() if
	 * the baseboard model does not support this hardware feature.
	 */
	if (cns3xxx_pciextirq) {
		mem = (void __iomem *)(CNS3XXX_MISC_BASE_VIRT + 0x0018);
		reg = __raw_readl(mem);
		/* GPIO26 is gpio, EXT_INT[0:2] not gpio func */
		reg &= ~0x3c000000;
		reg |= 0x38000000;
		__raw_writel(reg, mem);

		cns3xxx_gpio_init(32, 32, CNS3XXX_GPIOB_BASE_VIRT,
				  IRQ_CNS3XXX_GPIOB, NR_IRQS_CNS3XXX + 32);

		irq_set_irq_type(154, IRQ_TYPE_LEVEL_LOW);
		irq_set_irq_type(93, IRQ_TYPE_LEVEL_HIGH);
		irq_set_irq_type(94, IRQ_TYPE_LEVEL_HIGH);
		irq_set_irq_type(95, IRQ_TYPE_LEVEL_HIGH);
	} else {
		cns3xxx_gpio_init(32, 32, CNS3XXX_GPIOB_BASE_VIRT,
				  IRQ_CNS3XXX_GPIOB, NR_IRQS_CNS3XXX + 32);
	}

	if (strncmp(laguna_info.model, "GW", 2) == 0) {
		if (laguna_info.config_bitmap & ETH0_LOAD)
			laguna_net_data.ports |= BIT(0);
		if (laguna_info.config_bitmap & ETH1_LOAD)
			laguna_net_data.ports |= BIT(1);
		if (laguna_info.config_bitmap & ETH2_LOAD)
			laguna_net_data.ports |= BIT(2);
		if (laguna_net_data.ports)
			platform_device_register(&laguna_net_device);

		if ((laguna_info.config_bitmap & SATA0_LOAD) ||
		    (laguna_info.config_bitmap & SATA1_LOAD))
			cns3xxx_ahci_init();

		if (laguna_info.config_bitmap & (USB0_LOAD)) {
			cns3xxx_pwr_power_up(1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB);

			/* DRVVBUS pins share with GPIOA */
			mem = (void __iomem *)(CNS3XXX_MISC_BASE_VIRT + 0x0014);
			reg = __raw_readl(mem);
			reg |= 0x8;
			__raw_writel(reg, mem);

			/* Enable OTG */
			mem = (void __iomem *)(CNS3XXX_MISC_BASE_VIRT + 0x0808);
			reg = __raw_readl(mem);
			reg &= ~(1 << 10);
			__raw_writel(reg, mem);

			platform_device_register(&cns3xxx_usb_otg_device);
		}

		if (laguna_info.config_bitmap & (USB1_LOAD)) {
			platform_device_register(&cns3xxx_usb_ehci_device);
			platform_device_register(&cns3xxx_usb_ohci_device);
		}

		if (laguna_info.config_bitmap & (SD_LOAD))
			cns3xxx_sdhci_init();

		if (laguna_info.config_bitmap & (UART0_LOAD))
			laguna_uart.num_resources = 1;
		if (laguna_info.config_bitmap & (UART1_LOAD))
			laguna_uart.num_resources = 2;
		if (laguna_info.config_bitmap & (UART2_LOAD))
			laguna_uart.num_resources = 3;
		platform_device_register(&laguna_uart);

		if (laguna_info.config2_bitmap & (NOR_FLASH_LOAD)) {
			laguna_nor_partitions[2].size =
				(SZ_4M << laguna_info.nor_flash_size) -
				laguna_nor_partitions[2].offset;
			laguna_nor_res.end = CNS3XXX_FLASH_BASE +
				laguna_nor_partitions[2].offset +
				laguna_nor_partitions[2].size - 1;
			platform_device_register(&laguna_nor_pdev);
		}

		if (laguna_info.config2_bitmap & (SPI_FLASH_LOAD)) {
			laguna_spi_partitions[2].size =
				(SZ_2M << laguna_info.spi_flash_size) -
				laguna_spi_partitions[2].offset;
			spi_register_board_info(ARRAY_AND_SIZE(laguna_spi_devices));
		}

		if ((laguna_info.config_bitmap & SPI0_LOAD) ||
		    (laguna_info.config_bitmap & SPI1_LOAD))
			platform_device_register(&laguna_spi_controller);

		if (laguna_info.config2_bitmap & GPS_LOAD)
			platform_device_register(&laguna_pps_device);

		/*
		 * Do any model specific setup not known by the bitmap by matching
		 *  the first 6 characters of the model name
		 */

		if ( (strncmp(laguna_info.model, "GW2388", 6) == 0)
		  || (strncmp(laguna_info.model, "GW2389", 6) == 0) )
		{
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2388));
			// configure LED's
			laguna_gpio_leds_data.num_leds = 2;
		} else if (strncmp(laguna_info.model, "GW2387", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2387));
			// configure LED's
			laguna_gpio_leds_data.num_leds = 2;
		} else if (strncmp(laguna_info.model, "GW2386", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2386));
			// configure LED's
			laguna_gpio_leds_data.num_leds = 2;
		} else if (strncmp(laguna_info.model, "GW2385", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2385));
			// configure LED's
			laguna_gpio_leds[0].gpio = 115;
			laguna_gpio_leds[1].gpio = 12;
			laguna_gpio_leds[1].name = "red";
			laguna_gpio_leds[1].active_low = 0,
			laguna_gpio_leds[2].gpio = 14;
			laguna_gpio_leds[2].name = "green";
			laguna_gpio_leds[2].active_low = 0,
			laguna_gpio_leds[3].gpio = 15;
			laguna_gpio_leds[3].name = "blue";
			laguna_gpio_leds[3].active_low = 0,
			laguna_gpio_leds_data.num_leds = 4;
		} else if ( (strncmp(laguna_info.model, "GW2384", 6) == 0)
			 || (strncmp(laguna_info.model, "GW2394", 6) == 0) )
		{
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2384));
			// configure LED's
			laguna_gpio_leds_data.num_leds = 1;
		} else if (strncmp(laguna_info.model, "GW2383", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2383));
			// configure LED's
			laguna_gpio_leds[0].gpio = 107;
			laguna_gpio_leds_data.num_leds = 1;
		} else if (strncmp(laguna_info.model, "GW2382", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2382));
			// configure LED's
			laguna_gpio_leds[0].gpio = 107;
			laguna_gpio_leds_data.num_leds = 1;
		} else if (strncmp(laguna_info.model, "GW2380", 6) == 0) {
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2380));
			// configure LED's
			laguna_gpio_leds[0].gpio = 107;
			laguna_gpio_leds[1].gpio = 106;
			laguna_gpio_leds_data.num_leds = 2;
		} else if ( (strncmp(laguna_info.model, "GW2391", 6) == 0)
			 || (strncmp(laguna_info.model, "GW2393", 6) == 0) )
		{
			// configure GPIO's
			laguna_register_gpio(ARRAY_AND_SIZE(laguna_gpio_gw2391));
			// configure LED's
			laguna_gpio_leds_data.num_leds = 2;
		}
		platform_device_register(&laguna_gpio_leds_device);
	} else {
		// Do some defaults here, not sure what yet
	}
	return 0;
}
late_initcall(laguna_model_setup);

MACHINE_START(GW2388, "Gateworks Corporation Laguna Platform")
	.smp		= smp_ops(cns3xxx_smp_ops),
	.atag_offset	= 0x100,
	.map_io		= laguna_map_io,
	.init_irq	= cns3xxx_init_irq,
	.init_time	= cns3xxx_timer_init,
	.init_machine	= laguna_init,
	.init_late      = cns3xxx_pcie_init_late,
	.restart	= cns3xxx_restart,
MACHINE_END
