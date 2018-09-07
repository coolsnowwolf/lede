/*
 *  ADM5120 generic platform devices
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/export.h>

#include <asm/bootinfo.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_switch.h>
#include <asm/mach-adm5120/adm5120_nand.h>
#include <asm/mach-adm5120/adm5120_platform.h>

#if 1
/*
 * TODO:remove global adm5120_eth* variables when the switch driver will be
 *	converted into a real platform driver
 */
unsigned int adm5120_eth_num_ports = 6;
EXPORT_SYMBOL_GPL(adm5120_eth_num_ports);

unsigned char adm5120_eth_macs[6][6] = {
	{'\00', 'A', 'D', 'M', '\x51', '\x20' },
	{'\00', 'A', 'D', 'M', '\x51', '\x21' },
	{'\00', 'A', 'D', 'M', '\x51', '\x22' },
	{'\00', 'A', 'D', 'M', '\x51', '\x23' },
	{'\00', 'A', 'D', 'M', '\x51', '\x24' },
	{'\00', 'A', 'D', 'M', '\x51', '\x25' }
};
EXPORT_SYMBOL_GPL(adm5120_eth_macs);

unsigned char adm5120_eth_vlans[6] = {
	0x41, 0x42, 0x44, 0x48, 0x50, 0x60
};
EXPORT_SYMBOL_GPL(adm5120_eth_vlans);
#endif

void __init adm5120_setup_eth_macs(u8 *mac_base)
{
	u32 t;
	int i, j;

	t = ((u32) mac_base[3] << 16) | ((u32) mac_base[4] << 8)
		| ((u32) mac_base[5]);

	for (i = 0; i < ARRAY_SIZE(adm5120_eth_macs); i++) {
		for (j = 0; j < 3; j++)
			adm5120_eth_macs[i][j] = mac_base[j];

		adm5120_eth_macs[i][3] = (t >> 16) & 0xff;
		adm5120_eth_macs[i][4] = (t >> 8) & 0xff;
		adm5120_eth_macs[i][5] = t & 0xff;

		t++;
	}
}

/*
 * Built-in ethernet switch
 */
struct resource adm5120_switch_resources[] = {
	[0] = {
		.start	= ADM5120_SWITCH_BASE,
		.end	= ADM5120_SWITCH_BASE+ADM5120_SWITCH_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= ADM5120_IRQ_SWITCH,
		.end	= ADM5120_IRQ_SWITCH,
		.flags	= IORESOURCE_IRQ,
	},
};

struct adm5120_switch_platform_data adm5120_switch_data;
struct platform_device adm5120_switch_device = {
	.name		= "adm5120-switch",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(adm5120_switch_resources),
	.resource	= adm5120_switch_resources,
	.dev.platform_data = &adm5120_switch_data,
};

void __init adm5120_add_device_switch(unsigned num_ports, u8 *vlan_map)
{
	if (num_ports > 0)
		adm5120_eth_num_ports = num_ports;

	if (vlan_map)
		memcpy(adm5120_eth_vlans, vlan_map, sizeof(adm5120_eth_vlans));

	platform_device_register(&adm5120_switch_device);
}

/*
 * USB Host Controller
 */
struct resource adm5120_hcd_resources[] = {
	[0] = {
		.start	= ADM5120_USBC_BASE,
		.end	= ADM5120_USBC_BASE+ADM5120_USBC_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= ADM5120_IRQ_USBC,
		.end	= ADM5120_IRQ_USBC,
		.flags	= IORESOURCE_IRQ,
	},
};

static u64 adm5120_hcd_dma_mask = DMA_BIT_MASK(24);
struct platform_device adm5120_hcd_device = {
	.name		= "adm5120-hcd",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(adm5120_hcd_resources),
	.resource	= adm5120_hcd_resources,
	.dev = {
		.dma_mask		= &adm5120_hcd_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(24),
	}
};

void __init adm5120_add_device_usb(void)
{
	platform_device_register(&adm5120_hcd_device);
}

/*
 * NOR flash devices
 */
struct adm5120_flash_platform_data adm5120_flash0_data;
struct platform_device adm5120_flash0_device =	{
	.name			= "adm5120-flash",
	.id			= 0,
	.dev.platform_data	= &adm5120_flash0_data,
};

struct adm5120_flash_platform_data adm5120_flash1_data;
struct platform_device adm5120_flash1_device =	{
	.name			= "adm5120-flash",
	.id			= 1,
	.dev.platform_data	= &adm5120_flash1_data,
};

void __init adm5120_add_device_flash(unsigned id)
{
	struct platform_device *pdev;

	switch (id) {
	case 0:
		pdev = &adm5120_flash0_device;
		break;
	case 1:
		pdev = &adm5120_flash1_device;
		break;
	default:
		pdev = NULL;
		break;
	}

	if (pdev)
		platform_device_register(pdev);
}

/*
 * built-in UARTs
 */
static void adm5120_uart_set_mctrl(struct amba_device *dev, void __iomem *base,
		unsigned int mctrl)
{
}

struct amba_pl010_data adm5120_uart0_data = {
	.set_mctrl = adm5120_uart_set_mctrl
};

struct amba_device adm5120_uart0_device = {
	.dev		= {
		.init_name = "apb:uart0",
		.platform_data = &adm5120_uart0_data,
	},
	.res		= {
		.start	= ADM5120_UART0_BASE,
		.end	= ADM5120_UART0_BASE + ADM5120_UART_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	.irq		= { ADM5120_IRQ_UART0, 0 },
	.periphid	= 0x0041010,
};

struct amba_pl010_data adm5120_uart1_data = {
	.set_mctrl = adm5120_uart_set_mctrl
};

struct amba_device adm5120_uart1_device = {
	.dev		= {
		.init_name = "apb:uart1",
		.platform_data = &adm5120_uart1_data,
	},
	.res		= {
		.start	= ADM5120_UART1_BASE,
		.end	= ADM5120_UART1_BASE + ADM5120_UART_SIZE - 1,
		.flags	= IORESOURCE_MEM,
	},
	.irq		= { ADM5120_IRQ_UART1, 0 },
	.periphid	= 0x0041010,
};

void __init adm5120_add_device_uart(unsigned id)
{
	struct amba_device *dev;

	switch (id) {
	case 0:
		dev = &adm5120_uart0_device;
		break;
	case 1:
		dev = &adm5120_uart1_device;
		break;
	default:
		dev = NULL;
		break;
	}

	if (dev)
		amba_device_register(dev, &iomem_resource);
}

/*
 * GPIO buttons
 */
void __init adm5120_register_gpio_buttons(int id,
					  unsigned poll_interval,
					  unsigned nbuttons,
					  struct gpio_keys_button *buttons)
{
	struct platform_device *pdev;
	struct gpio_keys_platform_data pdata;
	struct gpio_keys_button *p;
	int err;

	p = kmemdup(buttons, nbuttons * sizeof(*p), GFP_KERNEL);
	if (!p)
		return;

	pdev = platform_device_alloc("gpio-keys-polled", id);
	if (!pdev)
		goto err_free_buttons;

	memset(&pdata, 0, sizeof(pdata));
	pdata.poll_interval = poll_interval;
	pdata.nbuttons = nbuttons;
	pdata.buttons = p;

	err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
	if (err)
		goto err_put_pdev;

	err = platform_device_add(pdev);
	if (err)
		goto err_put_pdev;

	return;

err_put_pdev:
	platform_device_put(pdev);

err_free_buttons:
	kfree(p);
}

/*
 * GPIO LEDS
 */
struct gpio_led_platform_data adm5120_gpio_leds_data;
struct platform_device adm5120_gpio_leds_device = {
	.name		= "leds-gpio",
	.id		= -1,
	.dev.platform_data = &adm5120_gpio_leds_data,
};

void __init adm5120_add_device_gpio_leds(unsigned num_leds,
				    struct gpio_led *leds)
{
	struct gpio_led *p;

	p = kmalloc(num_leds * sizeof(*p), GFP_KERNEL);
	if (!p)
		return;

	memcpy(p, leds, num_leds * sizeof(*p));
	adm5120_gpio_leds_data.num_leds = num_leds;
	adm5120_gpio_leds_data.leds = p;

	platform_device_register(&adm5120_gpio_leds_device);
}

/*
 * NAND flash
 */
struct resource adm5120_nand_resources[] = {
	[0] = {
		.start	= ADM5120_NAND_BASE,
		.end	= ADM5120_NAND_BASE + ADM5120_NAND_SIZE-1,
		.flags	= IORESOURCE_MEM,
	},
};

static int adm5120_nand_ready(struct mtd_info *mtd)
{
	return ((adm5120_nand_get_status() & ADM5120_NAND_STATUS_READY) != 0);
}

static void adm5120_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
					unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		adm5120_nand_set_cle(ctrl & NAND_CLE);
		adm5120_nand_set_ale(ctrl & NAND_ALE);
		adm5120_nand_set_cen(ctrl & NAND_NCE);
	}

	if (cmd != NAND_CMD_NONE)
		NAND_WRITE_REG(NAND_REG_DATA, cmd);
}

void __init adm5120_add_device_nand(struct platform_nand_data *pdata)
{
	struct platform_device *pdev;
	int err;

	pdev = platform_device_alloc("gen_nand", -1);
	if (!pdev)
		goto err_out;

	err = platform_device_add_resources(pdev, adm5120_nand_resources,
					ARRAY_SIZE(adm5120_nand_resources));
	if (err)
		goto err_put;

	err = platform_device_add_data(pdev, pdata, sizeof(*pdata));
	if (err)
		goto err_put;

	pdata = pdev->dev.platform_data;
	pdata->ctrl.dev_ready = adm5120_nand_ready;
	pdata->ctrl.cmd_ctrl = adm5120_nand_cmd_ctrl;

	err = platform_device_add(pdev);
	if (err)
		goto err_put;

	return;

err_put:
	platform_device_put(pdev);
err_out:
	return;
}
