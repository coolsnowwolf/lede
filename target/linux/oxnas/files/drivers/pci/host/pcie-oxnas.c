/*
 * PCIe driver for PLX NAS782X SoCs
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/mbus.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <mach/iomap.h>
#include <mach/hardware.h>
#include <mach/utils.h>

#define VERSION_ID_MAGIC		0x082510b5
#define LINK_UP_TIMEOUT_SECONDS		1
#define NUM_CONTROLLERS			1

enum {
	PCIE_DEVICE_TYPE_MASK = 0x0F,
	PCIE_DEVICE_TYPE_ENDPOINT = 0,
	PCIE_DEVICE_TYPE_LEGACY_ENDPOINT = 1,
	PCIE_DEVICE_TYPE_ROOT = 4,

	PCIE_LTSSM = BIT(4),
	PCIE_READY_ENTR_L23 = BIT(9),
	PCIE_LINK_UP = BIT(11),
	PCIE_OBTRANS = BIT(12),
};

enum {
	HCSL_BIAS_ON = BIT(0),
	HCSL_PCIE_EN = BIT(1),
	HCSL_PCIEA_EN = BIT(2),
	HCSL_PCIEB_EN = BIT(3),
};

enum {
	/* pcie phy reg offset */
	PHY_ADDR = 0,
	PHY_DATA = 4,
	/* phy data reg bits */
	READ_EN = BIT(16),
	WRITE_EN = BIT(17),
	CAP_DATA = BIT(18),
};

/* core config registers */
enum {
	PCI_CONFIG_VERSION_DEVICEID = 0,
	PCI_CONFIG_COMMAND_STATUS = 4,
};

/* inbound config registers */
enum {
	IB_ADDR_XLATE_ENABLE = 0xFC,

	/* bits */
	ENABLE_IN_ADDR_TRANS = BIT(0),
};

/* outbound config registers, offset relative to PCIE_POM0_MEM_ADDR */
enum {
	PCIE_POM0_MEM_ADDR	= 0,
	PCIE_POM1_MEM_ADDR	= 4,
	PCIE_IN0_MEM_ADDR	= 8,
	PCIE_IN1_MEM_ADDR	= 12,
	PCIE_IN_IO_ADDR		= 16,
	PCIE_IN_CFG0_ADDR	= 20,
	PCIE_IN_CFG1_ADDR	= 24,
	PCIE_IN_MSG_ADDR	= 28,
	PCIE_IN0_MEM_LIMIT	= 32,
	PCIE_IN1_MEM_LIMIT	= 36,
	PCIE_IN_IO_LIMIT	= 40,
	PCIE_IN_CFG0_LIMIT	= 44,
	PCIE_IN_CFG1_LIMIT	= 48,
	PCIE_IN_MSG_LIMIT	= 52,
	PCIE_AHB_SLAVE_CTRL	= 56,

	PCIE_SLAVE_BE_SHIFT	= 22,
};

#define ADDR_VAL(val)	((val) & 0xFFFF)
#define DATA_VAL(val)	((val) & 0xFFFF)

#define PCIE_SLAVE_BE(val)	((val) << PCIE_SLAVE_BE_SHIFT)
#define PCIE_SLAVE_BE_MASK	PCIE_SLAVE_BE(0xF)

struct oxnas_pcie_shared {
	/* seems all access are serialized, no lock required */
	int refcount;
};

/* Structure representing one PCIe interfaces */
struct oxnas_pcie {
	void __iomem *cfgbase;
	void __iomem *base;
	void __iomem *inbound;
	void __iomem *outbound;
	void __iomem *pcie_ctrl;

	int haslink;
	struct platform_device *pdev;
	struct resource io;
	struct resource cfg;
	struct resource pre_mem;	/* prefetchable */
	struct resource non_mem;	/* non-prefetchable */
	struct resource busn;		/* max available bus numbers */
	int card_reset;			/* gpio pin, optional */
	unsigned hcsl_en;		/* hcsl pci enable bit */
	struct clk *clk;
	struct clk *busclk;		/* for pcie bus, actually the PLLB */
	void *private_data[1];
	spinlock_t lock;
};

static struct oxnas_pcie_shared pcie_shared = {
	.refcount = 0,
};

static inline struct oxnas_pcie *sys_to_pcie(struct pci_sys_data *sys)
{
	return sys->private_data;
}


static inline void set_out_lanes(struct oxnas_pcie *pcie, unsigned lanes)
{
	oxnas_register_value_mask(pcie->outbound + PCIE_AHB_SLAVE_CTRL,
				  PCIE_SLAVE_BE_MASK, PCIE_SLAVE_BE(lanes));
	wmb();
}

static int oxnas_pcie_link_up(struct oxnas_pcie *pcie)
{
	unsigned long end;

	/* Poll for PCIE link up */
	end = jiffies + (LINK_UP_TIMEOUT_SECONDS * HZ);
	while (!time_after(jiffies, end)) {
		if (readl(pcie->pcie_ctrl) & PCIE_LINK_UP)
			return 1;
	}
	return 0;
}

static void __init oxnas_pcie_setup_hw(struct oxnas_pcie *pcie)
{
	/* We won't have any inbound address translation. This allows PCI
	 * devices to access anywhere in the AHB address map. Might be regarded
	 * as a bit dangerous, but let's get things working before we worry
	 * about that
	 */
	oxnas_register_clear_mask(pcie->inbound + IB_ADDR_XLATE_ENABLE,
				  ENABLE_IN_ADDR_TRANS);
	wmb();

	/*
	 * Program outbound translation windows
	 *
	 * Outbound window is what is referred to as "PCI client" region in HRM
	 *
	 * Could use the larger alternative address space to get >>64M regions
	 * for graphics cards etc., but will not bother at this point.
	 *
	 * IP bug means that AMBA window size must be a power of 2
	 *
	 * Set mem0 window for first 16MB of outbound window non-prefetchable
	 * Set mem1 window for second 16MB of outbound window prefetchable
	 * Set io window for next 16MB of outbound window
	 * Set cfg0 for final 1MB of outbound window
	 *
	 * Ignore mem1, cfg1 and msg windows for now as no obvious use cases for
	 * 820 that would need them
	 *
	 * Probably ideally want no offset between mem0 window start as seen by
	 * ARM and as seen on PCI bus and get Linux to assign memory regions to
	 * PCI devices using the same "PCI client" region start address as seen
	 * by ARM
	 */

	/* Set PCIeA mem0 region to be 1st 16MB of the 64MB PCIeA window */
	writel_relaxed(pcie->non_mem.start, pcie->outbound + PCIE_IN0_MEM_ADDR);
	writel_relaxed(pcie->non_mem.end, pcie->outbound + PCIE_IN0_MEM_LIMIT);
	writel_relaxed(pcie->non_mem.start, pcie->outbound + PCIE_POM0_MEM_ADDR);

	/* Set PCIeA mem1 region to be 2nd 16MB of the 64MB PCIeA window */
	writel_relaxed(pcie->pre_mem.start, pcie->outbound + PCIE_IN1_MEM_ADDR);
	writel_relaxed(pcie->pre_mem.end, pcie->outbound + PCIE_IN1_MEM_LIMIT);
	writel_relaxed(pcie->pre_mem.start, pcie->outbound + PCIE_POM1_MEM_ADDR);

	/* Set PCIeA io to be third 16M region of the 64MB PCIeA window*/
	writel_relaxed(pcie->io.start,	pcie->outbound + PCIE_IN_IO_ADDR);
	writel_relaxed(pcie->io.end,	pcie->outbound + PCIE_IN_IO_LIMIT);

	/* Set PCIeA cgf0 to be last 16M region of the 64MB PCIeA window*/
	writel_relaxed(pcie->cfg.start,	pcie->outbound + PCIE_IN_CFG0_ADDR);
	writel_relaxed(pcie->cfg.end, pcie->outbound + PCIE_IN_CFG0_LIMIT);
	wmb();

	/* Enable outbound address translation */
	oxnas_register_set_mask(pcie->pcie_ctrl, PCIE_OBTRANS);
	wmb();

	/*
	 * Program PCIe command register for core to:
	 *  enable memory space
	 *  enable bus master
	 *  enable io
	 */
	writel_relaxed(7, pcie->base + PCI_CONFIG_COMMAND_STATUS);
	/* which is which */
	wmb();
}

static unsigned oxnas_pcie_cfg_to_offset(
	struct pci_sys_data *sys,
	unsigned char bus_number,
	unsigned int devfn,
	int where)
{
	unsigned int function = PCI_FUNC(devfn);
	unsigned int slot = PCI_SLOT(devfn);
	unsigned char bus_number_offset;

	bus_number_offset = bus_number - sys->busnr;

	/*
	 * We'll assume for now that the offset, function, slot, bus encoding
	 * should map onto linear, contiguous addresses in PCIe config space,
	 * albeit that the majority will be unused as only slot 0 is valid for
	 * any PCIe bus and most devices have only function 0
	 *
	 * Could be that PCIe in fact works by not encoding the slot number into
	 * the config space address as it's known that only slot 0 is valid.
	 * We'll have to experiment if/when we get a PCIe switch connected to
	 * the PCIe host
	 */
	return (bus_number_offset << 20) | (slot << 15) | (function << 12) |
		(where & ~3);
}

/* PCI configuration space write function */
static int oxnas_pcie_wr_conf(struct pci_bus *bus, u32 devfn,
			      int where, int size, u32 val)
{
	unsigned long flags;
	struct oxnas_pcie *pcie = sys_to_pcie(bus->sysdata);
	unsigned offset;
	u32 value;
	u32 lanes;

	/* Only a single device per bus for PCIe point-to-point links */
	if (PCI_SLOT(devfn) > 0)
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (!pcie->haslink)
		return PCIBIOS_DEVICE_NOT_FOUND;

	offset = oxnas_pcie_cfg_to_offset(bus->sysdata, bus->number, devfn,
					  where);

	value = val << (8 * (where & 3));
	lanes = (0xf >> (4-size)) << (where & 3);
	/* it race with mem and io write, but the possibility is low, normally
	 * all config writes happens at driver initialize stage, wont interleave
	 * with others.
	 * and many pcie cards use dword (4bytes) access mem/io access only,
	 * so not bother to copy that ugly work-around now. */
	spin_lock_irqsave(&pcie->lock, flags);
	set_out_lanes(pcie, lanes);
	writel_relaxed(value, pcie->cfgbase + offset);
	set_out_lanes(pcie, 0xf);
	spin_unlock_irqrestore(&pcie->lock, flags);

	return PCIBIOS_SUCCESSFUL;
}

/* PCI configuration space read function */
static int oxnas_pcie_rd_conf(struct pci_bus *bus, u32 devfn, int where,
			      int size, u32 *val)
{
	struct oxnas_pcie *pcie = sys_to_pcie(bus->sysdata);
	unsigned offset;
	u32 value;
	u32 left_bytes, right_bytes;

	/* Only a single device per bus for PCIe point-to-point links */
	if (PCI_SLOT(devfn) > 0) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	if (!pcie->haslink) {
		*val = 0xffffffff;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	offset = oxnas_pcie_cfg_to_offset(bus->sysdata, bus->number, devfn,
					  where);
	value = readl_relaxed(pcie->cfgbase + offset);
	left_bytes = where & 3;
	right_bytes = 4 - left_bytes - size;
	value <<= right_bytes * 8;
	value >>= (left_bytes + right_bytes) * 8;
	*val = value;

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops oxnas_pcie_ops = {
	.read = oxnas_pcie_rd_conf,
	.write = oxnas_pcie_wr_conf,
};

static int __init oxnas_pcie_setup(int nr, struct pci_sys_data *sys)
{
	struct oxnas_pcie *pcie = sys_to_pcie(sys);

	pci_add_resource_offset(&sys->resources, &pcie->non_mem, sys->mem_offset);
	pci_add_resource_offset(&sys->resources, &pcie->pre_mem, sys->mem_offset);
	pci_add_resource_offset(&sys->resources, &pcie->io, sys->io_offset);
	pci_add_resource(&sys->resources, &pcie->busn);
	if (sys->busnr == 0) { /* default one */
		sys->busnr = pcie->busn.start;
	}
	/* do not use devm_ioremap_resource, it does not like cfg resource */
	pcie->cfgbase = devm_ioremap(&pcie->pdev->dev, pcie->cfg.start,
				     resource_size(&pcie->cfg));
	if (!pcie->cfgbase)
		return -ENOMEM;

	oxnas_pcie_setup_hw(pcie);

	return 1;
}

static void __init oxnas_pcie_enable(struct device *dev, struct oxnas_pcie *pcie)
{
	struct hw_pci hw;
	int i;

	memset(&hw, 0, sizeof(hw));
	for (i = 0; i < NUM_CONTROLLERS; i++)
		pcie->private_data[i] = pcie;

	hw.nr_controllers = NUM_CONTROLLERS;
/* I think use stack pointer is a bad idea though it is valid in this case */
	hw.private_data   = pcie->private_data;
	hw.setup          = oxnas_pcie_setup;
	hw.map_irq        = of_irq_parse_and_map_pci;
	hw.ops            = &oxnas_pcie_ops;

	/* pass dev to maintain of tree, interrupt mapping rely on this */
	pci_common_init_dev(dev, &hw);
}

void oxnas_pcie_init_shared_hw(struct platform_device *pdev,
			       void __iomem *phybase)
{
	struct reset_control *rstc;
	int ret;

	/* generate clocks from HCSL buffers, shared parts */
	writel(HCSL_BIAS_ON|HCSL_PCIE_EN, SYS_CTRL_HCSL_CTRL);

	/* Ensure PCIe PHY is properly reset */
	rstc = reset_control_get(&pdev->dev, "phy");
	if (IS_ERR(rstc)) {
		ret = PTR_ERR(rstc);
	} else {
		ret = reset_control_reset(rstc);
		reset_control_put(rstc);
	}

	if (ret) {
		dev_err(&pdev->dev, "phy reset failed %d\n", ret);
		return;
	}

	/* Enable PCIe Pre-Emphasis: What these value means? */

	writel(ADDR_VAL(0x0014), phybase + PHY_ADDR);
	writel(DATA_VAL(0xce10) | CAP_DATA, phybase + PHY_DATA);
	writel(DATA_VAL(0xce10) | WRITE_EN, phybase + PHY_DATA);

	writel(ADDR_VAL(0x2004), phybase + PHY_ADDR);
	writel(DATA_VAL(0x82c7) | CAP_DATA, phybase + PHY_DATA);
	writel(DATA_VAL(0x82c7) | WRITE_EN, phybase + PHY_DATA);
}

static int oxnas_pcie_shared_init(struct platform_device *pdev)
{
	if (++pcie_shared.refcount == 1) {
		/* we are the first */
		struct device_node *np = pdev->dev.of_node;
		void __iomem *phy = of_iomap(np, 2);
		if (!phy) {
			--pcie_shared.refcount;
			return -ENOMEM;
		}
		oxnas_pcie_init_shared_hw(pdev, phy);
		iounmap(phy);
		return 0;
	} else {
		return 0;
	}
}

#if 0
/* maybe we will call it when enter low power state */
static void oxnas_pcie_shared_deinit(struct platform_device *pdev)
{
	if (--pcie_shared.refcount == 0) {
		/* no cleanup needed */;
	}
}
#endif

static int __init
oxnas_pcie_map_registers(struct platform_device *pdev,
			 struct device_node *np,
			 struct oxnas_pcie *pcie)
{
	struct resource regs;
	int ret = 0;
	u32 outbound_ctrl_offset;
	u32 pcie_ctrl_offset;

	/* 2 is reserved for shared phy */
	ret = of_address_to_resource(np, 0, &regs);
	if (ret)
		return -EINVAL;
	pcie->base = devm_ioremap_resource(&pdev->dev, &regs);
	if (!pcie->base)
		return -ENOMEM;

	ret = of_address_to_resource(np, 1, &regs);
	if (ret)
		return -EINVAL;
	pcie->inbound = devm_ioremap_resource(&pdev->dev, &regs);
	if (!pcie->inbound)
		return -ENOMEM;


	if (of_property_read_u32(np, "plxtech,pcie-outbound-offset",
				 &outbound_ctrl_offset))
		return -EINVAL;
	/* SYSCRTL is shared by too many drivers, so is mapped by board file */
	pcie->outbound = IOMEM(OXNAS_SYSCRTL_BASE_VA + outbound_ctrl_offset);

	if (of_property_read_u32(np, "plxtech,pcie-ctrl-offset",
				 &pcie_ctrl_offset))
		return -EINVAL;
	pcie->pcie_ctrl = IOMEM(OXNAS_SYSCRTL_BASE_VA + pcie_ctrl_offset);

	return 0;
}

static int __init oxnas_pcie_init_res(struct platform_device *pdev,
				      struct oxnas_pcie *pcie,
				      struct device_node *np)
{
	struct of_pci_range range;
	struct of_pci_range_parser parser;
	int ret;

	if (of_pci_range_parser_init(&parser, np))
		return -EINVAL;

	/* Get the I/O and memory ranges from DT */
	for_each_of_pci_range(&parser, &range) {

		unsigned long restype = range.flags & IORESOURCE_TYPE_BITS;
		if (restype == IORESOURCE_IO) {
			of_pci_range_to_resource(&range, np, &pcie->io);
			pcie->io.name = "I/O";
		}
		if (restype == IORESOURCE_MEM) {
			if (range.flags & IORESOURCE_PREFETCH) {
				of_pci_range_to_resource(&range, np, &pcie->pre_mem);
				pcie->pre_mem.name = "PRE MEM";
			} else {
				of_pci_range_to_resource(&range, np, &pcie->non_mem);
				pcie->non_mem.name = "NON MEM";
			}

		}
		if (restype == 0)
			of_pci_range_to_resource(&range, np, &pcie->cfg);
	}

	/* Get the bus range */
	ret = of_pci_parse_bus_range(np, &pcie->busn);

	if (ret) {
		dev_err(&pdev->dev, "failed to parse bus-range property: %d\n",
			ret);
		return ret;
	}

	pcie->card_reset = of_get_gpio(np, 0);
	if (pcie->card_reset < 0)
		dev_info(&pdev->dev, "card reset gpio pin not exists\n");

	if (of_property_read_u32(np, "plxtech,pcie-hcsl-bit", &pcie->hcsl_en))
		return -EINVAL;

	pcie->clk = of_clk_get_by_name(np, "pcie");
	if (IS_ERR(pcie->clk)) {
		return PTR_ERR(pcie->clk);
	}

	pcie->busclk = of_clk_get_by_name(np, "busclk");
	if (IS_ERR(pcie->busclk)) {
		clk_put(pcie->clk);
		return PTR_ERR(pcie->busclk);
	}

	return 0;
}

static void oxnas_pcie_init_hw(struct platform_device *pdev,
			       struct oxnas_pcie *pcie)
{
	u32 version_id;
	int ret;

	clk_prepare_enable(pcie->busclk);

	/* reset PCIe cards use hard-wired gpio pin */
	if (pcie->card_reset >= 0 &&
	    !gpio_direction_output(pcie->card_reset, 0)) {
		wmb();
		mdelay(10);
		/* must tri-state the pin to pull it up */
		gpio_direction_input(pcie->card_reset);
		wmb();
		mdelay(100);
	}

	oxnas_register_set_mask(SYS_CTRL_HCSL_CTRL, BIT(pcie->hcsl_en));

	/* core */
	ret = device_reset(&pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "core reset failed %d\n", ret);
		return;
	}

	/* Start PCIe core clocks */
	clk_prepare_enable(pcie->clk);

	version_id = readl_relaxed(pcie->base + PCI_CONFIG_VERSION_DEVICEID);
	dev_info(&pdev->dev, "PCIe version/deviceID 0x%x\n", version_id);

	if (version_id != VERSION_ID_MAGIC) {
		dev_info(&pdev->dev, "PCIe controller not found\n");
		pcie->haslink = 0;
		return;
	}

	/* allow entry to L23 state */
	oxnas_register_set_mask(pcie->pcie_ctrl, PCIE_READY_ENTR_L23);

	/* Set PCIe core into RootCore mode */
	oxnas_register_value_mask(pcie->pcie_ctrl, PCIE_DEVICE_TYPE_MASK,
				  PCIE_DEVICE_TYPE_ROOT);
	wmb();

	/* Bring up the PCI core */
	oxnas_register_set_mask(pcie->pcie_ctrl, PCIE_LTSSM);
	wmb();
}

static int __init oxnas_pcie_probe(struct platform_device *pdev)
{
	struct oxnas_pcie *pcie;
	struct device_node *np = pdev->dev.of_node;
	int ret;

	pcie = devm_kzalloc(&pdev->dev, sizeof(struct oxnas_pcie),
			    GFP_KERNEL);
	if (!pcie)
		return -ENOMEM;

	pcie->pdev = pdev;
	pcie->haslink = 1;
	spin_lock_init(&pcie->lock);

	ret = oxnas_pcie_init_res(pdev, pcie, np);
	if (ret)
		return ret;
	if (pcie->card_reset >= 0) {
		ret = gpio_request_one(pcie->card_reset, GPIOF_DIR_IN,
				       dev_name(&pdev->dev));
		if (ret) {
			dev_err(&pdev->dev, "cannot request gpio pin %d\n",
				pcie->card_reset);
			return ret;
		}
	}

	ret = oxnas_pcie_map_registers(pdev, np, pcie);
	if (ret) {
		dev_err(&pdev->dev, "cannot map registers\n");
		goto err_free_gpio;
	}

	ret = oxnas_pcie_shared_init(pdev);
	if (ret)
		goto err_free_gpio;

	/* if hw not found, haslink cleared */
	oxnas_pcie_init_hw(pdev, pcie);

	if (pcie->haslink && oxnas_pcie_link_up(pcie)) {
		pcie->haslink = 1;
		dev_info(&pdev->dev, "link up\n");
	} else {
		pcie->haslink = 0;
		dev_info(&pdev->dev, "link down\n");
	}
	/* should we register our controller even when pcie->haslink is 0 ? */
	/* register the controller with framework */
	oxnas_pcie_enable(&pdev->dev, pcie);

	return 0;

err_free_gpio:
	if (pcie->card_reset)
		gpio_free(pcie->card_reset);

	return ret;
}

static const struct of_device_id oxnas_pcie_of_match_table[] = {
	{ .compatible = "plxtech,nas782x-pcie", },
	{},
};
MODULE_DEVICE_TABLE(of, oxnas_pcie_of_match_table);

static struct platform_driver oxnas_pcie_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "oxnas-pcie",
		.of_match_table =
		   of_match_ptr(oxnas_pcie_of_match_table),
	},
};

static int __init oxnas_pcie_init(void)
{
	return platform_driver_probe(&oxnas_pcie_driver,
				     oxnas_pcie_probe);
}

subsys_initcall(oxnas_pcie_init);

MODULE_AUTHOR("Ma Haijun <mahaijuns@gmail.com>");
MODULE_DESCRIPTION("NAS782x PCIe driver");
MODULE_LICENSE("GPLv2");
