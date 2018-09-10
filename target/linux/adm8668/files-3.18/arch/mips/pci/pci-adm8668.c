/*
 * Copyright (C) 2010 Scott Nicholas <neutronscott@scottn.us>
 * Copyright (C) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Note that this controller is identical to the ADM5120 one
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/types.h>
#include <linux/spinlock.h>

#include <asm/byteorder.h>
#include <asm/pci.h>
#include <adm8668.h>

static DEFINE_SPINLOCK(pci_lock);

#define PCI_ENABLE			0x80000000
#define	ADMPCI_IO_BASE			0x12600000
#define	ADMPCI_IO_SIZE			0x1fffff
#define	ADMPCI_MEM_BASE			0x16000000
#define	ADMPCI_MEM_SIZE			0x7ffffff

static inline void write_cfgaddr(u32 addr)
{
	__raw_writel((addr | PCI_ENABLE),
			(void __iomem *)KSEG1ADDR(ADM8668_PCICFG_BASE));
}

static inline void write_cfgdata(u32 data)
{
	__raw_writel(data, (void __iomem *)KSEG1ADDR(ADM8668_PCIDAT_BASE));
}

static inline u32 read_cfgdata(void)
{
	return __raw_readl((void __iomem *)KSEG1ADDR(ADM8668_PCIDAT_BASE));
}

static inline u32 mkaddr(struct pci_bus *bus, unsigned int devfn, int where)
{
	return ((bus->number & 0xff) << 16) | ((devfn & 0xff) << 8) |
		(where & 0xfc);
}

static int pci_read_config(struct pci_bus *bus, unsigned int devfn,
				int where, int size, u32 *val)
{
	unsigned long flags;
	u32 data;

	spin_lock_irqsave(&pci_lock, flags);
	write_cfgaddr(mkaddr(bus, devfn, where));
	data = read_cfgdata();

	switch (size) {
	case 1:
		if (where & 1)
			data >>= 8;
		if (where & 2)
			data >>= 16;
		data &= 0xff;
		break;
	case 2:
		if (where & 2)
			data >>= 16;
		data &= 0xffff;
		break;
	}

	*val = data;

	spin_unlock_irqrestore(&pci_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

static int pci_write_config(struct pci_bus *bus, unsigned int devfn,
				int where, int size, u32 val)
{
	unsigned long flags;
	u32 data;
	int s;

	spin_lock_irqsave(&pci_lock, flags);

	write_cfgaddr(mkaddr(bus, devfn, where));
	data = read_cfgdata();

	switch (size) {
	case 1:
		s = ((where & 3) << 3);
		data &= ~(0xff << s);
		data |= ((val & 0xff) << s);
		break;
	case 2:
		s = ((where & 2) << 4);
		data &= ~(0xffff << s);
		data |= ((val & 0xffff) << s);
		break;
	case 4:
		data = val;
		break;
	}

	write_cfgdata(data);

	spin_unlock_irqrestore(&pci_lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops adm8668_pci_ops = {
	.read = pci_read_config,
	.write = pci_write_config
};


struct resource pciioport_resource = {
	.name	= "adm8668_pci",
	.start	= ADMPCI_IO_BASE,
	.end	= ADMPCI_IO_BASE + ADMPCI_IO_SIZE,
	.flags	= IORESOURCE_IO
};

struct resource pciiomem_resource = {
	.name	= "adm8668_pci",
	.start	= ADMPCI_MEM_BASE,
	.end	= ADMPCI_MEM_BASE + ADMPCI_MEM_SIZE,
	.flags	= IORESOURCE_MEM
};

struct pci_controller adm8668_pci_controller = {
	.pci_ops	= &adm8668_pci_ops,
	.io_resource	= &pciioport_resource,
	.mem_resource	= &pciiomem_resource,
};

int pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	switch (slot) {
	case 1:
		return 14;
	case 2:
		return 13;
	case 3:
		return 12;
	default:
		return dev->irq;
	}
}

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

static void adm8668_pci_fixup(struct pci_dev *dev)
{
	if (dev->devfn != 0)
		return;

	pr_info("PCI: fixing up ADM8668 controller\n");

	/* setup COMMAND register */
	pci_write_config_word(dev, PCI_COMMAND,
		(PCI_COMMAND_IO | PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER));

	/* setup CACHE_LINE_SIZE register */
	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 4);

	/* setup BARS */
	pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0);
	pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, 0);
}
DECLARE_PCI_FIXUP_HEADER(0x1317, 0x8688, adm8668_pci_fixup);

static int __init adm8668_pci_init(void)
{
	void __iomem *io_map_base;

	ioport_resource.start   = ADMPCI_IO_BASE;
	ioport_resource.end     = ADMPCI_IO_BASE + ADMPCI_IO_SIZE;

	io_map_base = ioremap(ADMPCI_IO_BASE, ADMPCI_IO_SIZE);
	if (!io_map_base)
		printk("io_map_base didn't work.\n");

	adm8668_pci_controller.io_map_base = (unsigned long)io_map_base;
	register_pci_controller(&adm8668_pci_controller);

	return 0;
}
arch_initcall(adm8668_pci_init);
