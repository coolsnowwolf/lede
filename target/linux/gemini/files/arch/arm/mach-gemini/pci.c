/*
 *  Support for Gemini PCI Controller
 *
 *  Copyright (C) 2009 Janos Laube <janos.dev@gmail.com>
 *  Copyright (C) 2009 Paulius Zaleckas <paulius.zaleckas@teltonika.lt>
 *
 * based on SL2312 PCI controller code
 *   Storlink (C) 2003
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/irq.h>
#include <linux/gpio.h>

#include <asm/mach/pci.h>

#include <mach/irqs.h>
#include <mach/hardware.h>

#define GEMINI_PCI_IOSIZE_1M		0x0000

#define GEMINI_PCI_PMC			0x40
#define GEMINI_PCI_PMCSR		0x44
#define GEMINI_PCI_CTRL1		0x48
#define GEMINI_PCI_CTRL2		0x4C
#define GEMINI_PCI_MEM1_BASE_SIZE	0x50
#define GEMINI_PCI_MEM2_BASE_SIZE	0x54
#define GEMINI_PCI_MEM3_BASE_SIZE	0x58

#define PCI_CTRL2_INTSTS_OFFSET		28
#define PCI_CTRL2_INTMASK_OFFSET	22

#define GEMINI_PCI_DMA_MASK		0xFFF00000
#define GEMINI_PCI_DMA_MEM1_BASE	0x00000000
#define GEMINI_PCI_DMA_MEM2_BASE	0x00000000
#define GEMINI_PCI_DMA_MEM3_BASE	0x00000000
#define GEMINI_PCI_DMA_MEM1_SIZE	7
#define GEMINI_PCI_DMA_MEM2_SIZE	6
#define GEMINI_PCI_DMA_MEM3_SIZE	6

#define PCI_CONF_ENABLE		(1 << 31)
#define PCI_CONF_WHERE(r)	((r) & 0xFC)
#define PCI_CONF_BUS(b)		(((b) & 0xFF) << 16)
#define PCI_CONF_DEVICE(d)	(((d) & 0x1F) << 11)
#define PCI_CONF_FUNCTION(f)	(((f) & 0x07) << 8)

#define PCI_IOSIZE_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE))
#define PCI_PROT_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE) + 0x04)
#define PCI_CTRL_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE) + 0x08)
#define PCI_SOFTRST_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE) + 0x10)
#define PCI_CONFIG_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE) + 0x28)
#define PCI_DATA_REG	(IO_ADDRESS(GEMINI_PCI_IO_BASE) + 0x2C)


static DEFINE_SPINLOCK(gemini_pci_lock);

static int gemini_pci_read_config(struct pci_bus* bus, unsigned int fn,
				  int config, int size, u32* value)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&gemini_pci_lock, irq_flags);

	__raw_writel(PCI_CONF_BUS(bus->number) |
			PCI_CONF_DEVICE(PCI_SLOT(fn)) |
			PCI_CONF_FUNCTION(PCI_FUNC(fn)) |
			PCI_CONF_WHERE(config) |
			PCI_CONF_ENABLE,
			PCI_CONFIG_REG);

	*value = __raw_readl(PCI_DATA_REG);

	if (size == 1)
		*value = (*value >> (8 * (config & 3))) & 0xFF;
	else if (size == 2)
		*value = (*value >> (8 * (config & 3))) & 0xFFFF;

	spin_unlock_irqrestore(&gemini_pci_lock, irq_flags);

	dev_dbg(&bus->dev,
		"[read]  slt: %.2d, fnc: %d, cnf: 0x%.2X, val (%d bytes): 0x%.8X\n",
		PCI_SLOT(fn), PCI_FUNC(fn), config, size, *value);

	return PCIBIOS_SUCCESSFUL;
}

static int gemini_pci_write_config(struct pci_bus* bus, unsigned int fn,
				   int config, int size, u32 value)
{
	unsigned long irq_flags = 0;
	int ret = PCIBIOS_SUCCESSFUL;

	dev_dbg(&bus->dev,
		"[write] slt: %.2d, fnc: %d, cnf: 0x%.2X, val (%d bytes): 0x%.8X\n",
		PCI_SLOT(fn), PCI_FUNC(fn), config, size, value);

	spin_lock_irqsave(&gemini_pci_lock, irq_flags);

	__raw_writel(PCI_CONF_BUS(bus->number) |
			PCI_CONF_DEVICE(PCI_SLOT(fn)) |
			PCI_CONF_FUNCTION(PCI_FUNC(fn)) |
			PCI_CONF_WHERE(config) |
			PCI_CONF_ENABLE,
			PCI_CONFIG_REG);

	switch(size) {
	case 4:
		__raw_writel(value, PCI_DATA_REG);
		break;
	case 2:
		__raw_writew(value, PCI_DATA_REG + (config & 3));
		break;
	case 1:
		__raw_writeb(value, PCI_DATA_REG + (config & 3));
		break;
	default:
		ret = PCIBIOS_BAD_REGISTER_NUMBER;
	}

	spin_unlock_irqrestore(&gemini_pci_lock, irq_flags);

	return ret;
}

static struct pci_ops gemini_pci_ops = {
	.read	= gemini_pci_read_config,
	.write	= gemini_pci_write_config,
};

static struct resource gemini_pci_resource_io = {
	.name	= "PCI I/O Space",
	.start	= GEMINI_PCI_IO_BASE,
	.end	= GEMINI_PCI_IO_BASE + SZ_1M - 1,
	.flags	= IORESOURCE_IO,
};

static struct resource gemini_pci_resource_mem = {
	.name	= "PCI Memory Space",
	.start	= GEMINI_PCI_MEM_BASE,
	.end	= GEMINI_PCI_MEM_BASE + SZ_128M - 1,
	.flags	= IORESOURCE_MEM,
};

static int __init gemini_pci_request_resources(struct pci_sys_data *sys)
{
	if (request_resource(&ioport_resource, &gemini_pci_resource_io))
		goto bad_resources;
	if (request_resource(&iomem_resource, &gemini_pci_resource_mem))
		goto bad_resources;

	pci_add_resource(&sys->resources, &gemini_pci_resource_io);
	pci_add_resource(&sys->resources, &gemini_pci_resource_mem);

	return 0;

bad_resources:
	pr_err("Gemini PCI: request_resource() failed. "
			"Abort PCI bus enumeration.\n");
	return -1;
}

static int __init gemini_pci_setup(int nr, struct pci_sys_data *sys)
{
	unsigned int cmd;

	pcibios_min_io = 0x100;
	pcibios_min_mem = 0;

	if ((nr > 0) || gemini_pci_request_resources(sys))
		return 0;

	/* setup I/O space to 1MB size */
	__raw_writel(GEMINI_PCI_IOSIZE_1M, PCI_IOSIZE_REG);

	/* setup hostbridge */
	cmd = __raw_readl(PCI_CTRL_REG);
	cmd |= PCI_COMMAND_IO;
	cmd |= PCI_COMMAND_MEMORY;
	cmd |= PCI_COMMAND_MASTER;
	__raw_writel(cmd, PCI_CTRL_REG);

	return 1;
}

static struct pci_bus* __init gemini_pci_scan_bus(int nr, struct pci_sys_data* sys)
{
	unsigned int reg = 0;
	struct pci_bus* bus = 0;

	bus = pci_scan_bus(nr, &gemini_pci_ops, sys);
	if (bus) {
		dev_dbg(&bus->dev, "setting up PCI DMA\n");
		reg = (GEMINI_PCI_DMA_MEM1_BASE & GEMINI_PCI_DMA_MASK)
			| (GEMINI_PCI_DMA_MEM1_SIZE << 16);
		gemini_pci_write_config(bus, 0, GEMINI_PCI_MEM1_BASE_SIZE, 4, reg);
		reg =	(GEMINI_PCI_DMA_MEM2_BASE & GEMINI_PCI_DMA_MASK)
			| (GEMINI_PCI_DMA_MEM2_SIZE << 16);
		gemini_pci_write_config(bus, 0, GEMINI_PCI_MEM2_BASE_SIZE, 4, reg);
		reg = (GEMINI_PCI_DMA_MEM3_BASE & GEMINI_PCI_DMA_MASK)
			| (GEMINI_PCI_DMA_MEM3_SIZE << 16);
		gemini_pci_write_config(bus, 0, GEMINI_PCI_MEM3_BASE_SIZE, 4, reg);
	}

	return bus;
}

/* Should work with all boards based on original Storlink EVB */
static int __init gemini_pci_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	if (slot < 9 || slot > 12)
		return -1;

	return PCI_IRQ_BASE + (((slot - 9) + (pin - 1)) & 0x3);
}

static struct hw_pci gemini_hw_pci __initdata = {
	.nr_controllers	= 1,
	.setup		= gemini_pci_setup,
	.scan           = gemini_pci_scan_bus,
	.map_irq	= gemini_pci_map_irq,
};

/* we need this for muxed PCI interrupts handling */
static struct pci_bus bogus_pci_bus;

static void gemini_pci_ack_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	unsigned int reg;

	gemini_pci_read_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, &reg);
	reg &= ~(0xF << PCI_CTRL2_INTSTS_OFFSET);
	reg |= 1 << (irq - PCI_IRQ_BASE + PCI_CTRL2_INTSTS_OFFSET);
	gemini_pci_write_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, reg);
}

static void gemini_pci_mask_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	unsigned int reg;

	gemini_pci_read_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, &reg);
	reg &= ~((0xF << PCI_CTRL2_INTSTS_OFFSET)
		| (1 << (irq - PCI_IRQ_BASE + PCI_CTRL2_INTMASK_OFFSET)));
	gemini_pci_write_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, reg);
}

static void gemini_pci_unmask_irq(struct irq_data *d)
{
	unsigned int irq = d->irq;
	unsigned int reg;

	gemini_pci_read_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, &reg);
	reg &= ~(0xF << PCI_CTRL2_INTSTS_OFFSET);
	reg |= 1 << (irq - PCI_IRQ_BASE + PCI_CTRL2_INTMASK_OFFSET);
	gemini_pci_write_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, reg);
}

static void gemini_pci_irq_handler(struct irq_desc *desc)
{
	unsigned int pci_irq_no, irq_stat, reg, i;

	gemini_pci_read_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2, 4, &reg);
	irq_stat = reg >> PCI_CTRL2_INTSTS_OFFSET;

	for (i = 0; i < 4; i++) {

		if ((irq_stat & (1 << i)) == 0)
			continue;

		pci_irq_no = PCI_IRQ_BASE + i;

		BUG_ON(!(irq_desc[pci_irq_no].handle_irq));
		irq_desc[pci_irq_no].handle_irq(&irq_desc[pci_irq_no]);
	}
}

static struct irq_chip gemini_pci_irq_chip = {
	.name = "PCI",
	.irq_ack = gemini_pci_ack_irq,
	.irq_mask = gemini_pci_mask_irq,
	.irq_unmask = gemini_pci_unmask_irq,
};

static int __init gemini_pci_init(void)
{
	int i;

	for (i = 72; i <= 95; i++)
		gpio_request(i, "PCI");

	/* initialize our bogus bus */
	dev_set_name(&bogus_pci_bus.dev, "PCI IRQ handler");
	bogus_pci_bus.number = 0;

	/* mask and clear all interrupts */
	gemini_pci_write_config(&bogus_pci_bus, 0, GEMINI_PCI_CTRL2 + 2, 2,
				0xF000);

	for (i = PCI_IRQ_BASE; i < PCI_IRQ_BASE + 4; i++) {
		irq_set_chip_and_handler(i, &gemini_pci_irq_chip,
					 handle_level_irq);
	}

	irq_set_chained_handler(IRQ_PCI, gemini_pci_irq_handler);

	pci_common_init(&gemini_hw_pci);

	return 0;
}

subsys_initcall(gemini_pci_init);
