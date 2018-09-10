/*
 * Copyright (C) 2010 Scott Nicholas <neutronscott@scottn.us>
 * Copyright (C) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/platform_data/tulip.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/mtd/physmap.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/amba/bus.h>
#include <linux/amba/serial.h>

#include <asm/reboot.h>
#include <asm/time.h>
#include <asm/addrspace.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <adm8668.h>

#define ADM8868_UBOOT_ENV		0x20000
#define ADM8868_UBOOT_WAN_MAC		0x5ac
#define ADM8868_UBOOT_LAN_MAC		0x404

static void adm8668_uart_set_mctrl(struct amba_device *dev,
					void __iomem *base,
					unsigned int mcrtl)
{
}

static struct amba_pl010_data adm8668_uart0_data = {
	.set_mctrl = adm8668_uart_set_mctrl,
};

static struct amba_device adm8668_uart0_device = {
	.dev = {
		.init_name	= "apb:uart0",
		.platform_data	= &adm8668_uart0_data,
	},
	.res = {
		.start		= ADM8668_UART0_BASE,
		.end		= ADM8668_UART0_BASE + 0xF,
		.flags		= IORESOURCE_MEM,
	},
	.irq = {
		ADM8668_UART0_IRQ,
		-1
	},
	.periphid = 0x0041010,
};

static struct resource eth0_resources[] = {
	{
		.start		= ADM8668_LAN_BASE,
		.end		= ADM8668_LAN_BASE + 256,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= ADM8668_LAN_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct tulip_platform_data eth0_pdata = {
	.chip_id	= ADM8668,
};

static struct platform_device adm8668_eth0_device = {
	.name		= "tulip",
	.id		= 0,
	.resource	= eth0_resources,
	.num_resources	= ARRAY_SIZE(eth0_resources),
	.dev.platform_data = &eth0_pdata,
};

static struct resource eth1_resources[] = {
	{
		.start		= ADM8668_WAN_BASE,
		.end		= ADM8668_WAN_BASE + 256,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= ADM8668_WAN_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};

static struct tulip_platform_data eth1_pdata = {
	.chip_id	= ADM8668,
};

static struct platform_device adm8668_eth1_device = {
	.name		= "tulip",
	.id		= 1,
	.resource	= eth1_resources,
	.num_resources	= ARRAY_SIZE(eth1_resources),
	.dev.platform_data = &eth1_pdata,
};

static struct resource usb_resources[] = {
	{
		.start	= ADM8668_USB_BASE,
		.end	= ADM8668_USB_BASE + 0x1FFFFF,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= ADM8668_USB_IRQ,
		.end	= ADM8668_USB_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct usb_ehci_pdata usb_pdata = {
	.caps_offset	= 0x100,
	.has_tt		= 1,
};

static struct platform_device adm8668_usb_device = {
	.name		= "ehci-platform",
	.id		= -1,
	.resource	= usb_resources,
	.num_resources	= ARRAY_SIZE(usb_resources),
	.dev.platform_data = &usb_pdata,
};

static struct platform_device *adm8668_devs[] = {
	&adm8668_eth0_device,
	&adm8668_eth1_device,
	&adm8668_usb_device,
};

static void adm8668_fetch_mac(int unit)
{
	u8 *mac;
	u32 offset;
	struct tulip_platform_data *pdata;

	switch (unit) {
	case -1:
	case 0:
		offset = ADM8868_UBOOT_LAN_MAC;
		pdata = &eth0_pdata;
		break;
	case 1:
		offset = ADM8868_UBOOT_WAN_MAC;
		pdata = &eth1_pdata;
		break;
	default:
		pr_err("unsupported ethernet unit: %d\n", unit);
		return;
	}

	mac = (u8 *)(KSEG1ADDR(ADM8668_SMEM1_BASE) + ADM8868_UBOOT_ENV + offset);

	memcpy(pdata->mac, mac, sizeof(pdata->mac));
}

static void adm8668_ehci_workaround(void)
{
	u32 chipid;

	chipid = ADM8668_CONFIG_REG(ADM8668_CR0);
	ADM8668_CONFIG_REG(ADM8668_CR66) = 0x0C1600D9;

	if (chipid == 0x86880001)
		return;

	ADM8668_CONFIG_REG(ADM8668_CR66) &= ~(3 << 20);
	ADM8668_CONFIG_REG(ADM8668_CR66) |= (1 << 20);
	pr_info("ADM8668: applied USB workaround\n");
}


int __init adm8668_devs_register(void)
{
	int ret;

	ret = amba_device_register(&adm8668_uart0_device, &iomem_resource);
	if (ret)
		panic("failed to register AMBA UART");

	adm8668_fetch_mac(0);
	adm8668_fetch_mac(1);
	adm8668_ehci_workaround();

	return platform_add_devices(adm8668_devs, ARRAY_SIZE(adm8668_devs));
}
arch_initcall(adm8668_devs_register);
