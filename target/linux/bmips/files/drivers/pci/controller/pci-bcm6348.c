// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * BCM6348 PCI Controller Driver
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <linux/clk.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/reset.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include "../pci.h"

#define CARDBUS_DUMMY_ID		0x6348
#define CARDBUS_PCI_IDSEL		0x8
#define FAKE_CB_BRIDGE_SLOT		0x1e

#define BCMPCI_REG_TIMERS		0x40
#define REG_TIMER_TRDY_SHIFT		0
#define REG_TIMER_TRDY_MASK		(0xff << REG_TIMER_TRDY_SHIFT)
#define REG_TIMER_RETRY_SHIFT		8
#define REG_TIMER_RETRY_MASK		(0xff << REG_TIMER_RETRY_SHIFT)

#define MPI_SP0_RANGE_REG		0x100
#define MPI_SP0_REMAP_REG		0x104
#define MPI_SP0_REMAP_ENABLE_MASK	(1 << 0)
#define MPI_SP1_RANGE_REG		0x10C
#define MPI_SP1_REMAP_REG		0x110
#define MPI_SP1_REMAP_ENABLE_MASK	(1 << 0)

#define MPI_L2PCFG_REG			0x11c
#define MPI_L2PCFG_CFG_TYPE_SHIFT	0
#define MPI_L2PCFG_CFG_TYPE_MASK	(0x3 << MPI_L2PCFG_CFG_TYPE_SHIFT)
#define MPI_L2PCFG_REG_SHIFT		2
#define MPI_L2PCFG_REG_MASK		(0x3f << MPI_L2PCFG_REG_SHIFT)
#define MPI_L2PCFG_FUNC_SHIFT		8
#define MPI_L2PCFG_FUNC_MASK		(0x7 << MPI_L2PCFG_FUNC_SHIFT)
#define MPI_L2PCFG_DEVNUM_SHIFT		11
#define MPI_L2PCFG_DEVNUM_MASK		(0x1f << MPI_L2PCFG_DEVNUM_SHIFT)
#define MPI_L2PCFG_CFG_USEREG_MASK	(1 << 30)
#define MPI_L2PCFG_CFG_SEL_MASK		(1 << 31)

#define MPI_L2PMEMRANGE1_REG		0x120
#define MPI_L2PMEMBASE1_REG		0x124
#define MPI_L2PMEMREMAP1_REG		0x128
#define MPI_L2PMEMRANGE2_REG		0x12C
#define MPI_L2PMEMBASE2_REG		0x130
#define MPI_L2PMEMREMAP2_REG		0x134
#define MPI_L2PIORANGE_REG		0x138
#define MPI_L2PIOBASE_REG		0x13C
#define MPI_L2PIOREMAP_REG		0x140
#define MPI_L2P_BASE_MASK		(0xffff8000)
#define MPI_L2PREMAP_ENABLED_MASK	(1 << 0)
#define MPI_L2PREMAP_IS_CARDBUS_MASK	(1 << 2)

#define MPI_PCIMODESEL_REG		0x144
#define MPI_PCIMODESEL_BAR1_NOSWAP_MASK (1 << 0)
#define MPI_PCIMODESEL_BAR2_NOSWAP_MASK (1 << 1)
#define MPI_PCIMODESEL_EXT_ARB_MASK	(1 << 2)
#define MPI_PCIMODESEL_PREFETCH_SHIFT	4
#define MPI_PCIMODESEL_PREFETCH_MASK	(0xf << MPI_PCIMODESEL_PREFETCH_SHIFT)

#define MPI_LOCBUSCTL_REG		0x14c
#define MPI_LOCBUSCTL_EN_PCI_GPIO_MASK	(1 << 0)
#define MPI_LOCBUSCTL_U2P_NOSWAP_MASK	(1 << 1)

#define MPI_LOCINT_REG			0x150
#define MPI_LOCINT_MASK(x)		(1 << (x + 16))
#define MPI_LOCINT_STAT(x)		(1 << (x))
#define MPI_LOCINT_DIR_FAILED		6
#define MPI_LOCINT_EXT_PCI_INT		7
#define MPI_LOCINT_SERR			8
#define MPI_LOCINT_CSERR		9

#define MPI_PCICFGCTL_REG		0x178
#define MPI_PCICFGCTL_CFGADDR_SHIFT	2
#define MPI_PCICFGCTL_CFGADDR_MASK	(0x1f << MPI_PCICFGCTL_CFGADDR_SHIFT)
#define MPI_PCICFGCTL_WRITEEN_MASK	(1 << 7)

#define MPI_PCICFGDATA_REG		0x17c

#define PCMCIA_OFFSET			0x54

#define PCMCIA_C1_REG			0x0
#define PCMCIA_C1_CD1_MASK		(1 << 0)
#define PCMCIA_C1_CD2_MASK		(1 << 1)
#define PCMCIA_C1_VS1_MASK		(1 << 2)
#define PCMCIA_C1_VS2_MASK		(1 << 3)
#define PCMCIA_C1_VS1OE_MASK		(1 << 6)
#define PCMCIA_C1_VS2OE_MASK		(1 << 7)
#define PCMCIA_C1_CBIDSEL_SHIFT		(8)
#define PCMCIA_C1_CBIDSEL_MASK		(0x1f << PCMCIA_C1_CBIDSEL_SHIFT)
#define PCMCIA_C1_EN_PCMCIA_GPIO_MASK	(1 << 13)
#define PCMCIA_C1_EN_PCMCIA_MASK	(1 << 14)
#define PCMCIA_C1_EN_CARDBUS_MASK	(1 << 15)
#define PCMCIA_C1_RESET_MASK		(1 << 18)

#ifdef CONFIG_CARDBUS
struct bcm6348_cb {
	u16 pci_command;
	u8 cb_latency;
	u8 subordinate_busn;
	u8 cardbus_busn;
	u8 pci_busn;
	int bus_assigned;
	u16 bridge_control;

	u32 mem_base0;
	u32 mem_limit0;
	u32 mem_base1;
	u32 mem_limit1;

	u32 io_base0;
	u32 io_limit0;
	u32 io_base1;
	u32 io_limit1;
};
#endif /* CONFIG_CARDBUS */

struct bcm6348_pci {
	void __iomem *pci;
	void __iomem *pcmcia;
	void __iomem *io;
	int irq;
	struct reset_control *reset;
	bool remap;
#ifdef CONFIG_CARDBUS
	struct bcm6348_cb cb;
	int cb_bus;
#endif /* CONFIG_CARDBUS */
};

static struct bcm6348_pci bcm6348_pci;

extern int bmips_pci_irq;

static u32 bcm6348_int_cfg_readl(u32 reg)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	u32 tmp;

	tmp = reg & MPI_PCICFGCTL_CFGADDR_MASK;
	tmp |= MPI_PCICFGCTL_WRITEEN_MASK;
	__raw_writel(tmp, priv->pci + MPI_PCICFGCTL_REG);
	iob();
	return __raw_readl(priv->pci + MPI_PCICFGDATA_REG);
}

static void bcm6348_int_cfg_writel(u32 val, u32 reg)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	u32 tmp;

	tmp = reg & MPI_PCICFGCTL_CFGADDR_MASK;
	tmp |= MPI_PCICFGCTL_WRITEEN_MASK;
	__raw_writel(tmp, priv->pci + MPI_PCICFGCTL_REG);
	__raw_writel(val, priv->pci + MPI_PCICFGDATA_REG);
}

/*
 * swizzle 32bits data to return only the needed part
 */
static int postprocess_read(u32 data, int where, unsigned int size)
{
	u32 ret = 0;

	switch (size) {
	case 1:
		ret = (data >> ((where & 3) << 3)) & 0xff;
		break;
	case 2:
		ret = (data >> ((where & 3) << 3)) & 0xffff;
		break;
	case 4:
		ret = data;
		break;
	}

	return ret;
}

static int preprocess_write(u32 orig_data, u32 val, int where,
			    unsigned int size)
{
	u32 ret = 0;

	switch (size) {
	case 1:
		ret = (orig_data & ~(0xff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 2:
		ret = (orig_data & ~(0xffff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 4:
		ret = val;
		break;
	}

	return ret;
}

static int bcm6348_setup_cfg_access(int type, unsigned int busn,
				    unsigned int devfn, int where)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	unsigned int slot, func, reg;
	u32 val;

	slot = PCI_SLOT(devfn);
	func = PCI_FUNC(devfn);
	reg = where >> 2;

	/* sanity check */
	if (slot > (MPI_L2PCFG_DEVNUM_MASK >> MPI_L2PCFG_DEVNUM_SHIFT))
		return 1;

	if (func > (MPI_L2PCFG_FUNC_MASK >> MPI_L2PCFG_FUNC_SHIFT))
		return 1;

	if (reg > (MPI_L2PCFG_REG_MASK >> MPI_L2PCFG_REG_SHIFT))
		return 1;

	/* ok, setup config access */
	val = (reg << MPI_L2PCFG_REG_SHIFT);
	val |= (func << MPI_L2PCFG_FUNC_SHIFT);
	val |= (slot << MPI_L2PCFG_DEVNUM_SHIFT);
	val |= MPI_L2PCFG_CFG_USEREG_MASK;
	val |= MPI_L2PCFG_CFG_SEL_MASK;
	/* type 0 cycle for local bus, type 1 cycle for anything else */
	if (type != 0) {
		/* FIXME: how to specify bus ??? */
		val |= (1 << MPI_L2PCFG_CFG_TYPE_SHIFT);
	}
	__raw_writel(val, priv->pci + MPI_L2PCFG_REG);

	return 0;
}


static int bcm6348_do_cfg_read(int type, unsigned int busn,
			       unsigned int devfn, int where, int size,
			       u32 *val)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	u32 data;

	/* two phase cycle, first we write address, then read data at
	 * another location, caller already has a spinlock so no need
	 * to add one here */
	if (bcm6348_setup_cfg_access(type, busn, devfn, where))
		return PCIBIOS_DEVICE_NOT_FOUND;
	iob();
	data = le32_to_cpu(__raw_readl(priv->io));
	/* restore IO space normal behaviour */
	__raw_writel(0, priv->pci + MPI_L2PCFG_REG);

	*val = postprocess_read(data, where, size);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm6348_do_cfg_write(int type, unsigned int busn,
				unsigned int devfn, int where, int size,
				u32 val)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	u32 data;

	/* two phase cycle, first we write address, then write data to
	 * another location, caller already has a spinlock so no need
	 * to add one here */
	if (bcm6348_setup_cfg_access(type, busn, devfn, where))
		return PCIBIOS_DEVICE_NOT_FOUND;
	iob();

	data = le32_to_cpu(__raw_readl(priv->io));
	data = preprocess_write(data, val, where, size);

	__raw_writel(cpu_to_le32(data), priv->io);
	wmb();
	/* no way to know the access is done, we have to wait */
	udelay(500);
	/* restore IO space normal behaviour */
	__raw_writel(0, priv->pci + MPI_L2PCFG_REG);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm6348_pci_read(struct pci_bus *bus, unsigned int devfn,
			     int where, int size, u32 *val)
{
	int type;

	type = bus->parent ? 1 : 0;

	if (type == 0 && PCI_SLOT(devfn) == CARDBUS_PCI_IDSEL)
		return PCIBIOS_DEVICE_NOT_FOUND;

	return bcm6348_do_cfg_read(type, bus->number, devfn,
				   where, size, val);
}

static int bcm6348_pci_write(struct pci_bus *bus, unsigned int devfn,
			      int where, int size, u32 val)
{
	int type;

	type = bus->parent ? 1 : 0;

	if (type == 0 && PCI_SLOT(devfn) == CARDBUS_PCI_IDSEL)
		return PCIBIOS_DEVICE_NOT_FOUND;

	return bcm6348_do_cfg_write(type, bus->number, devfn,
				    where, size, val);
}

static struct pci_ops bcm6348_pci_ops = {
	.read = bcm6348_pci_read,
	.write = bcm6348_pci_write,
};

static struct resource bcm6348_pci_io_resource = {
	.name = "BCM6348 PCI IO space",
	.flags = IORESOURCE_IO,
};
static struct resource bcm6348_pci_mem_resource;
static struct resource bcm6348_pci_busn_resource;

static struct pci_controller bcm6348_pci_controller = {
	.pci_ops = &bcm6348_pci_ops,
	.io_resource = &bcm6348_pci_io_resource,
	.mem_resource = &bcm6348_pci_mem_resource,
};

#ifdef CONFIG_CARDBUS
static int bcm6348_cb_bridge_read(int where, int size, u32 *val)
{
	struct bcm6348_cb *cb = &bcm6348_pci.cb;
	unsigned int reg;
	u32 data;

	data = 0;
	reg = where >> 2;
	switch (reg) {
	case (PCI_VENDOR_ID >> 2):
	case (PCI_CB_SUBSYSTEM_VENDOR_ID >> 2):
		/* create dummy vendor/device id from our cpu id */
		data = (CARDBUS_DUMMY_ID << 16) | PCI_VENDOR_ID_BROADCOM;
		break;

	case (PCI_COMMAND >> 2):
		data = (PCI_STATUS_DEVSEL_SLOW << 16);
		data |= cb->pci_command;
		break;

	case (PCI_CLASS_REVISION >> 2):
		data = (PCI_CLASS_BRIDGE_CARDBUS << 16);
		break;

	case (PCI_CACHE_LINE_SIZE >> 2):
		data = (PCI_HEADER_TYPE_CARDBUS << 16);
		break;

	case (PCI_INTERRUPT_LINE >> 2):
		/* bridge control */
		data = (cb->bridge_control << 16);
		/* pin:intA line:0xff */
		data |= (0x1 << 8) | 0xff;
		break;

	case (PCI_CB_PRIMARY_BUS >> 2):
		data = (cb->cb_latency << 24);
		data |= (cb->subordinate_busn << 16);
		data |= (cb->cardbus_busn << 8);
		data |= cb->pci_busn;
		break;

	case (PCI_CB_MEMORY_BASE_0 >> 2):
		data = cb->mem_base0;
		break;

	case (PCI_CB_MEMORY_LIMIT_0 >> 2):
		data = cb->mem_limit0;
		break;

	case (PCI_CB_MEMORY_BASE_1 >> 2):
		data = cb->mem_base1;
		break;

	case (PCI_CB_MEMORY_LIMIT_1 >> 2):
		data = cb->mem_limit1;
		break;

	case (PCI_CB_IO_BASE_0 >> 2):
		/* | 1 for 32bits io support */
		data = cb->io_base0 | 0x1;
		break;

	case (PCI_CB_IO_LIMIT_0 >> 2):
		data = cb->io_limit0;
		break;

	case (PCI_CB_IO_BASE_1 >> 2):
		/* | 1 for 32bits io support */
		data = cb->io_base1 | 0x1;
		break;

	case (PCI_CB_IO_LIMIT_1 >> 2):
		data = cb->io_limit1;
		break;
	}

	*val = postprocess_read(data, where, size);
	return PCIBIOS_SUCCESSFUL;
}

/*
 * emulate configuration write access on a cardbus bridge
 */
static int bcm6348_cb_bridge_write(int where, int size, u32 val)
{
	struct bcm6348_cb *cb = &bcm6348_pci.cb;
	unsigned int reg;
	u32 data, tmp;
	int ret;

	ret = bcm6348_cb_bridge_read((where & ~0x3), 4, &data);
	if (ret != PCIBIOS_SUCCESSFUL)
		return ret;

	data = preprocess_write(data, val, where, size);

	reg = where >> 2;
	switch (reg) {
	case (PCI_COMMAND >> 2):
		cb->pci_command = (data & 0xffff);
		break;

	case (PCI_CB_PRIMARY_BUS >> 2):
		cb->cb_latency = (data >> 24) & 0xff;
		cb->subordinate_busn = (data >> 16) & 0xff;
		cb->cardbus_busn = (data >> 8) & 0xff;
		cb->pci_busn = data & 0xff;
		if (cb->cardbus_busn)
			cb->bus_assigned = 1;
		break;

	case (PCI_INTERRUPT_LINE >> 2):
		tmp = (data >> 16) & 0xffff;
		/* Disable memory prefetch support */
		tmp &= ~PCI_CB_BRIDGE_CTL_PREFETCH_MEM0;
		tmp &= ~PCI_CB_BRIDGE_CTL_PREFETCH_MEM1;
		cb->bridge_control = tmp;
		break;

	case (PCI_CB_MEMORY_BASE_0 >> 2):
		cb->mem_base0 = data;
		break;

	case (PCI_CB_MEMORY_LIMIT_0 >> 2):
		cb->mem_limit0 = data;
		break;

	case (PCI_CB_MEMORY_BASE_1 >> 2):
		cb->mem_base1 = data;
		break;

	case (PCI_CB_MEMORY_LIMIT_1 >> 2):
		cb->mem_limit1 = data;
		break;

	case (PCI_CB_IO_BASE_0 >> 2):
		cb->io_base0 = data;
		break;

	case (PCI_CB_IO_LIMIT_0 >> 2):
		cb->io_limit0 = data;
		break;

	case (PCI_CB_IO_BASE_1 >> 2):
		cb->io_base1 = data;
		break;

	case (PCI_CB_IO_LIMIT_1 >> 2):
		cb->io_limit1 = data;
		break;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int bcm6348_cb_read(struct pci_bus *bus, unsigned int devfn,
			   int where, int size, u32 *val)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	struct bcm6348_cb *cb = &priv->cb;

	/* Snoop access to slot 0x1e on root bus, we fake a cardbus
	 * bridge at this location */
	if (!bus->parent && PCI_SLOT(devfn) == FAKE_CB_BRIDGE_SLOT) {
		priv->cb_bus = bus->number;
		return bcm6348_cb_bridge_read(where, size, val);
	}

	/* A configuration cycle for the device behind the cardbus
	 * bridge is actually done as a type 0 cycle on the primary
	 * bus. This means that only one device can be on the cardbus
	 * bus */
	if (cb->bus_assigned &&
	    bus->number == cb->cardbus_busn &&
	    PCI_SLOT(devfn) == 0)
		return bcm6348_do_cfg_read(0, 0,
					   PCI_DEVFN(CARDBUS_PCI_IDSEL, 0),
					   where, size, val);

	return PCIBIOS_DEVICE_NOT_FOUND;
}

static int bcm6348_cb_write(struct pci_bus *bus, unsigned int devfn,
			    int where, int size, u32 val)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	struct bcm6348_cb *cb = &priv->cb;

	if (!bus->parent && PCI_SLOT(devfn) == FAKE_CB_BRIDGE_SLOT) {
		priv->cb_bus = bus->number;
		return bcm6348_cb_bridge_write(where, size, val);
	}

	if (cb->bus_assigned &&
	    bus->number == cb->cardbus_busn &&
	    PCI_SLOT(devfn) == 0)
		return bcm6348_do_cfg_write(0, 0,
					    PCI_DEVFN(CARDBUS_PCI_IDSEL, 0),
					    where, size, val);

	return PCIBIOS_DEVICE_NOT_FOUND;
}

static struct pci_ops bcm6348_cb_ops = {
	.read = bcm6348_cb_read,
	.write = bcm6348_cb_write,
};

/*
 * only one IO window, so it  cannot be shared by PCI and cardbus, use
 * fixup to choose and detect unhandled configuration
 */
static void bcm6348_pci_fixup(struct pci_dev *dev)
{
	struct bcm6348_pci *priv = &bcm6348_pci;
	struct bcm6348_cb *cb = &priv->cb;
	static int io_window = -1;
	int i, found, new_io_window;
	u32 val;

	/* look for any io resource */
	found = 0;
	for (i = 0; i < DEVICE_COUNT_RESOURCE; i++) {
		if (pci_resource_flags(dev, i) & IORESOURCE_IO) {
			found = 1;
			break;
		}
	}

	if (!found)
		return;

	/* skip our fake bus with only cardbus bridge on it */
	if (dev->bus->number == priv->cb_bus)
		return;

	/* find on which bus the device is */
	if (cb->bus_assigned &&
	    dev->bus->number == cb->cardbus_busn &&
	    PCI_SLOT(dev->devfn) == 0)
		new_io_window = 1;
	else
		new_io_window = 0;

	if (new_io_window == io_window)
		return;

	if (io_window != -1) {
		pr_err("bcm63xx: both PCI and cardbus devices "
		       "need IO, which hardware cannot do\n");
		return;
	}

	pr_info("bcm63xx: PCI IO window assigned to %s\n",
	       (new_io_window == 0) ? "PCI" : "cardbus");

	val = __raw_readl(priv->pci + MPI_L2PIOREMAP_REG);
	if (io_window)
		val |= MPI_L2PREMAP_IS_CARDBUS_MASK;
	else
		val &= ~MPI_L2PREMAP_IS_CARDBUS_MASK;
	__raw_writel(val, priv->pci + MPI_L2PIOREMAP_REG);

	io_window = new_io_window;
}
DECLARE_PCI_FIXUP_ENABLE(PCI_ANY_ID, PCI_ANY_ID, bcm6348_pci_fixup);

static struct resource bcm6348_cb_io_resource = {
	.name = "bcm6348 CB IO space",
	.flags = IORESOURCE_IO,
};
static struct resource bcm6348_cb_mem_resource;

static struct pci_controller bcm6348_cb_controller = {
	.pci_ops = &bcm6348_cb_ops,
	.io_resource = &bcm6348_cb_io_resource,
	.mem_resource = &bcm6348_cb_mem_resource,
};
#endif /* CONFIG_CARDBUS */

static void bcm6348_pci_setup(struct bcm6348_pci *priv)
{
	u32 val;

	/* Setup local bus to PCI access (PCI memory) */
	val = bcm6348_pci_mem_resource.start & MPI_L2P_BASE_MASK;
	__raw_writel(val, priv->pci + MPI_L2PMEMBASE1_REG);
	__raw_writel(~(resource_size(&bcm6348_pci_mem_resource) - 1),
		     priv->pci + MPI_L2PMEMRANGE1_REG);
	__raw_writel(val | MPI_L2PREMAP_ENABLED_MASK,
		     priv->pci + MPI_L2PMEMREMAP1_REG);

	/* Set Cardbus IDSEL (type 0 cfg access on primary bus for
	 * this IDSEL will be done on Cardbus instead) */
	val = __raw_readl(priv->pcmcia + PCMCIA_C1_REG);
	val &= ~PCMCIA_C1_CBIDSEL_MASK;
	val |= (CARDBUS_PCI_IDSEL << PCMCIA_C1_CBIDSEL_SHIFT);
	__raw_writel(val, priv->pcmcia + PCMCIA_C1_REG);

#ifdef CONFIG_CARDBUS
	/* setup local bus to PCI access (Cardbus memory) */
	val = bcm6348_cb_mem_resource.start & MPI_L2P_BASE_MASK;
	__raw_writel(val, priv->pci + MPI_L2PMEMBASE2_REG);
	__raw_writel(~(resource_size(&bcm6348_cb_mem_resource) - 1),
		     priv->pci + MPI_L2PMEMRANGE2_REG);
	val |= MPI_L2PREMAP_ENABLED_MASK | MPI_L2PREMAP_IS_CARDBUS_MASK;
	__raw_writel(val, priv->pci + MPI_L2PMEMREMAP2_REG);
#else
	/* disable second access windows */
	__raw_writel(0, priv->pci + MPI_L2PMEMREMAP2_REG);
#endif

	/* setup local bus to PCI access (IO memory), we have only 1
	 * IO window for both PCI and cardbus, but it cannot handle
	 * both at the same time, assume standard PCI for now, if
	 * cardbus card has IO zone, PCI fixup will change window to
	 * cardbus */
	val = bcm6348_pci_io_resource.start & MPI_L2P_BASE_MASK;
	__raw_writel(val, priv->pci + MPI_L2PIOBASE_REG);
	__raw_writel(~(resource_size(&bcm6348_pci_io_resource) - 1),
		     priv->pci + MPI_L2PIORANGE_REG);
	__raw_writel(val | MPI_L2PREMAP_ENABLED_MASK,
		     priv->pci + MPI_L2PIOREMAP_REG);

	/* Enable PCI related GPIO pins */
	__raw_writel(MPI_LOCBUSCTL_EN_PCI_GPIO_MASK,
		     priv->pci + MPI_LOCBUSCTL_REG);

	/* Setup PCI to local bus access, used by PCI device to target
	 * local RAM while bus mastering */
	bcm6348_int_cfg_writel(0, PCI_BASE_ADDRESS_3);
	if (priv->remap)
		val = MPI_SP0_REMAP_ENABLE_MASK;
	else
		val = 0;
	__raw_writel(val, priv->pci + MPI_SP0_REMAP_REG);

	bcm6348_int_cfg_writel(0, PCI_BASE_ADDRESS_4);
	__raw_writel(0, priv->pci + MPI_SP1_REMAP_REG);

	/* Setup sp0 range to local RAM size */
	__raw_writel(~(memblock_phys_mem_size() - 1),
		     priv->pci + MPI_SP0_RANGE_REG);
	__raw_writel(0, priv->pci + MPI_SP1_RANGE_REG);

	/* Change host bridge retry counter to infinite number of
	 * retries, needed for some broadcom wifi cards with Silicon
	 * Backplane bus where access to srom seems very slow */
	val = bcm6348_int_cfg_readl(BCMPCI_REG_TIMERS);
	val &= ~REG_TIMER_RETRY_MASK;
	bcm6348_int_cfg_writel(val, BCMPCI_REG_TIMERS);

	/* EEnable memory decoder and bus mastering */
	val = bcm6348_int_cfg_readl(PCI_COMMAND);
	val |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
	bcm6348_int_cfg_writel(val, PCI_COMMAND);

	/* Enable read prefetching & disable byte swapping for bus
	 * mastering transfers */
	val = __raw_readl(priv->pci + MPI_PCIMODESEL_REG);
	val &= ~MPI_PCIMODESEL_BAR1_NOSWAP_MASK;
	val &= ~MPI_PCIMODESEL_BAR2_NOSWAP_MASK;
	val &= ~MPI_PCIMODESEL_PREFETCH_MASK;
	val |= (8 << MPI_PCIMODESEL_PREFETCH_SHIFT);
	__raw_writel(val, priv->pci + MPI_PCIMODESEL_REG);

	/* Enable pci interrupt */
	val = __raw_readl(priv->pci + MPI_LOCINT_REG);
	val |= MPI_LOCINT_MASK(MPI_LOCINT_EXT_PCI_INT);
	__raw_writel(val, priv->pci + MPI_LOCINT_REG);
}

static int bcm6348_pci_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm6348_pci *priv = &bcm6348_pci;
	struct resource *res;
	LIST_HEAD(resources);

	of_pci_check_probe_only();

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci");
	priv->pci = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->pci))
		return PTR_ERR(priv->pci);

	priv->pcmcia = priv->pci + PCMCIA_OFFSET;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pci-io");
	if (!res)
		return -EINVAL;
#ifdef CONFIG_CARDBUS
	bcm6348_pci_io_resource.start = res->start;
	bcm6348_pci_io_resource.end = res->end - (resource_size(res) >> 1);
	bcm6348_cb_io_resource.start = res->start + (resource_size(res) >> 1);
	bcm6348_cb_io_resource.end = res->end;
#else
	bcm6348_pci_io_resource.start = res->start;
	bcm6348_pci_io_resource.end = res->end;
#endif

	priv->irq = platform_get_irq(pdev, 0);
	if (!priv->irq)
		return -ENODEV;

	bmips_pci_irq = priv->irq;

	priv->reset = devm_reset_control_get(dev, "pci");
	if (IS_ERR(priv->reset))
		return PTR_ERR(priv->reset);

	priv->remap = of_property_read_bool(np, "brcm,remap");

	reset_control_reset(priv->reset);

	pci_load_of_ranges(&bcm6348_pci_controller, np);
	if (!bcm6348_pci_mem_resource.start)
		return -EINVAL;

	of_pci_parse_bus_range(np, &bcm6348_pci_busn_resource);
	pci_add_resource(&resources, &bcm6348_pci_busn_resource);

	/*
	 * Configuration accesses are done through IO space, remap 4
	 * first bytes to access it from CPU.
	 *
	 * This means that no IO access from CPU should happen while
	 * we do a configuration cycle, but there's no way we can add
	 * a spinlock for each io access, so this is currently kind of
	 * broken on SMP.
	 */
	priv->io = ioremap(bcm6348_pci_io_resource.start, sizeof(u32));
	if (!priv->io)
		return -ENOMEM;

	bcm6348_pci_setup(priv);

	register_pci_controller(&bcm6348_pci_controller);

#ifdef CONFIG_CARDBUS
	priv->cb_bus = -1;
	register_pci_controller(&bcm6348_cb_controller);
#endif /* CONFIG_CARDBUS */

	/* Mark memory space used for IO mapping as reserved */
	request_mem_region(bcm6348_pci_io_resource.start,
			   resource_size(&bcm6348_pci_io_resource),
			   "BCM6348 PCI IO space");

	return 0;
}

static const struct of_device_id bcm6348_pci_of_match[] = {
	{ .compatible = "brcm,bcm6348-pci", },
	{ /* sentinel */ }
};

static struct platform_driver bcm6348_pci_driver = {
	.probe = bcm6348_pci_probe,
	.driver	= {
		.name = "bcm6348-pci",
		.of_match_table = bcm6348_pci_of_match,
	},
};

int __init bcm6348_pci_init(void)
{
	int ret = platform_driver_register(&bcm6348_pci_driver);
	if (ret)
		pr_err("pci-bcm6348: Error registering platform driver!\n");
	return ret;
}
late_initcall_sync(bcm6348_pci_init);
