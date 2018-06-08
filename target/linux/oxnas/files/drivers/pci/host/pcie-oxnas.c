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
#include <linux/io.h>
#include <linux/sizes.h>

#define OXNAS_UART1_BASE	0x44200000
#define OXNAS_UART1_SIZE	SZ_32
#define OXNAS_UART1_BASE_VA	0xF0000000

#define OXNAS_UART2_BASE	0x44300000
#define OXNAS_UART2_SIZE	SZ_32

#define OXNAS_PERCPU_BASE	0x47000000
#define OXNAS_PERCPU_SIZE	SZ_8K
#define OXNAS_PERCPU_BASE_VA	0xF0002000

#define OXNAS_SYSCRTL_BASE	0x44E00000
#define OXNAS_SYSCRTL_SIZE	SZ_4K
#define OXNAS_SYSCRTL_BASE_VA	0xF0004000

#define OXNAS_SECCRTL_BASE	0x44F00000
#define OXNAS_SECCRTL_SIZE	SZ_4K
#define OXNAS_SECCRTL_BASE_VA	0xF0005000

#define OXNAS_RPSA_BASE		0x44400000
#define OXNAS_RPSA_SIZE		SZ_4K
#define OXNAS_RPSA_BASE_VA	0xF0006000

#define OXNAS_RPSC_BASE		0x44500000
#define OXNAS_RPSC_SIZE		SZ_4K
#define OXNAS_RPSC_BASE_VA	0xF0007000


/*
 * Location of flags and vectors in SRAM for controlling the booting of the
 * secondary ARM11 processors.
 */

#define OXNAS_SCU_BASE_VA		OXNAS_PERCPU_BASE_VA
#define OXNAS_GICN_BASE_VA(n)		(OXNAS_PERCPU_BASE_VA + 0x200 + n*0x100)

#define HOLDINGPEN_CPU			IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xc8)
#define HOLDINGPEN_LOCATION		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xc4)

/**
 * System block reset and clock control
 */
#define SYS_CTRL_PCI_STAT		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x20)
#define SYSCTRL_CLK_STAT		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x24)
#define SYS_CTRL_CLK_SET_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x2C)
#define SYS_CTRL_CLK_CLR_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x30)
#define SYS_CTRL_RST_SET_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x34)
#define SYS_CTRL_RST_CLR_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x38)

#define SYS_CTRL_PLLSYS_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x48)
#define SYS_CTRL_CLK_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x64)
#define SYS_CTRL_PLLSYS_KEY_CTRL	IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x6C)
#define SYS_CTRL_GMAC_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x78)
#define SYS_CTRL_GMAC_DELAY_CTRL	IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x100)

/* Scratch registers */
#define SYS_CTRL_SCRATCHWORD0		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xc4)
#define SYS_CTRL_SCRATCHWORD1		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xc8)
#define SYS_CTRL_SCRATCHWORD2		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xcc)
#define SYS_CTRL_SCRATCHWORD3		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xd0)

#define SYS_CTRL_PLLA_CTRL0		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x1F0)
#define SYS_CTRL_PLLA_CTRL1		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x1F4)
#define SYS_CTRL_PLLA_CTRL2		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x1F8)
#define SYS_CTRL_PLLA_CTRL3		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x1FC)

#define SYS_CTRL_USBHSMPH_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x40)
#define SYS_CTRL_USBHSMPH_STAT		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x44)
#define SYS_CTRL_REF300_DIV		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xF8)
#define SYS_CTRL_USBHSPHY_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x84)
#define SYS_CTRL_USB_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x90)

/* pcie */
#define SYS_CTRL_HCSL_CTRL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x114)

/* System control multi-function pin function selection */
#define SYS_CTRL_SECONDARY_SEL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x14)
#define SYS_CTRL_TERTIARY_SEL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x8c)
#define SYS_CTRL_QUATERNARY_SEL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x94)
#define SYS_CTRL_DEBUG_SEL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0x9c)
#define SYS_CTRL_ALTERNATIVE_SEL	IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xa4)
#define SYS_CTRL_PULLUP_SEL		IOMEM(OXNAS_SYSCRTL_BASE_VA + 0xac)

/* Secure control multi-function pin function selection */
#define SEC_CTRL_SECONDARY_SEL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x14)
#define SEC_CTRL_TERTIARY_SEL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x8c)
#define SEC_CTRL_QUATERNARY_SEL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x94)
#define SEC_CTRL_DEBUG_SEL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x9c)
#define SEC_CTRL_ALTERNATIVE_SEL	IOMEM(OXNAS_SECCRTL_BASE_VA + 0xa4)
#define SEC_CTRL_PULLUP_SEL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0xac)

#define SEC_CTRL_COPRO_CTRL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x68)
#define SEC_CTRL_SECURE_CTRL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x98)
#define SEC_CTRL_LEON_DEBUG		IOMEM(OXNAS_SECCRTL_BASE_VA + 0xF0)
#define SEC_CTRL_PLLB_DIV_CTRL		IOMEM(OXNAS_SECCRTL_BASE_VA + 0xF8)
#define SEC_CTRL_PLLB_CTRL0		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x1F0)
#define SEC_CTRL_PLLB_CTRL1		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x1F4)
#define SEC_CTRL_PLLB_CTRL8		IOMEM(OXNAS_SECCRTL_BASE_VA + 0x1F4)

#define RPSA_IRQ_SOFT			IOMEM(OXNAS_RPSA_BASE_VA + 0x10)
#define RPSA_FIQ_ENABLE			IOMEM(OXNAS_RPSA_BASE_VA + 0x108)
#define RPSA_FIQ_DISABLE		IOMEM(OXNAS_RPSA_BASE_VA + 0x10C)
#define RPSA_FIQ_IRQ_TO_FIQ		IOMEM(OXNAS_RPSA_BASE_VA + 0x1FC)

#define RPSC_IRQ_SOFT			IOMEM(OXNAS_RPSC_BASE_VA + 0x10)
#define RPSC_FIQ_ENABLE			IOMEM(OXNAS_RPSC_BASE_VA + 0x108)
#define RPSC_FIQ_DISABLE		IOMEM(OXNAS_RPSC_BASE_VA + 0x10C)
#define RPSC_FIQ_IRQ_TO_FIQ		IOMEM(OXNAS_RPSC_BASE_VA + 0x1FC)

#define RPSA_TIMER2_VAL			IOMEM(OXNAS_RPSA_BASE_VA + 0x224)

#define REF300_DIV_INT_SHIFT		8
#define REF300_DIV_FRAC_SHIFT		0
#define REF300_DIV_INT(val)		((val) << REF300_DIV_INT_SHIFT)
#define REF300_DIV_FRAC(val)		((val) << REF300_DIV_FRAC_SHIFT)

#define USBHSPHY_SUSPENDM_MANUAL_ENABLE		16
#define USBHSPHY_SUSPENDM_MANUAL_STATE		15
#define USBHSPHY_ATE_ESET			14
#define USBHSPHY_TEST_DIN			6
#define USBHSPHY_TEST_ADD			2
#define USBHSPHY_TEST_DOUT_SEL			1
#define USBHSPHY_TEST_CLK			0

#define USB_CTRL_USBAPHY_CKSEL_SHIFT	5
#define USB_CLK_XTAL0_XTAL1		(0 << USB_CTRL_USBAPHY_CKSEL_SHIFT)
#define USB_CLK_XTAL0			(1 << USB_CTRL_USBAPHY_CKSEL_SHIFT)
#define USB_CLK_INTERNAL		(2 << USB_CTRL_USBAPHY_CKSEL_SHIFT)

#define USBAMUX_DEVICE			BIT(4)

#define USBPHY_REFCLKDIV_SHIFT		2
#define USB_PHY_REF_12MHZ		(0 << USBPHY_REFCLKDIV_SHIFT)
#define USB_PHY_REF_24MHZ		(1 << USBPHY_REFCLKDIV_SHIFT)
#define USB_PHY_REF_48MHZ		(2 << USBPHY_REFCLKDIV_SHIFT)

#define USB_CTRL_USB_CKO_SEL_BIT	0

#define USB_INT_CLK_XTAL		0
#define USB_INT_CLK_REF300		2
#define USB_INT_CLK_PLLB		3

#define SYS_CTRL_GMAC_CKEN_RX_IN	14
#define SYS_CTRL_GMAC_CKEN_RXN_OUT	13
#define SYS_CTRL_GMAC_CKEN_RX_OUT	12
#define SYS_CTRL_GMAC_CKEN_TX_IN	10
#define SYS_CTRL_GMAC_CKEN_TXN_OUT	9
#define SYS_CTRL_GMAC_CKEN_TX_OUT	8
#define SYS_CTRL_GMAC_RX_SOURCE		7
#define SYS_CTRL_GMAC_TX_SOURCE		6
#define SYS_CTRL_GMAC_LOW_TX_SOURCE	4
#define SYS_CTRL_GMAC_AUTO_TX_SOURCE	3
#define SYS_CTRL_GMAC_RGMII		2
#define SYS_CTRL_GMAC_SIMPLE_MUX	1
#define SYS_CTRL_GMAC_CKEN_GTX		0
#define SYS_CTRL_GMAC_TX_VARDELAY_SHIFT		0
#define SYS_CTRL_GMAC_TXN_VARDELAY_SHIFT	8
#define SYS_CTRL_GMAC_RX_VARDELAY_SHIFT		16
#define SYS_CTRL_GMAC_RXN_VARDELAY_SHIFT	24
#define SYS_CTRL_GMAC_TX_VARDELAY(d)	((d)<<SYS_CTRL_GMAC_TX_VARDELAY_SHIFT)
#define SYS_CTRL_GMAC_TXN_VARDELAY(d)	((d)<<SYS_CTRL_GMAC_TXN_VARDELAY_SHIFT)
#define SYS_CTRL_GMAC_RX_VARDELAY(d)	((d)<<SYS_CTRL_GMAC_RX_VARDELAY_SHIFT)
#define SYS_CTRL_GMAC_RXN_VARDELAY(d)	((d)<<SYS_CTRL_GMAC_RXN_VARDELAY_SHIFT)

#define PLLB_BYPASS			1
#define PLLB_ENSAT			3
#define PLLB_OUTDIV			4
#define PLLB_REFDIV			8
#define PLLB_DIV_INT_SHIFT		8
#define PLLB_DIV_FRAC_SHIFT		0
#define PLLB_DIV_INT(val)		((val) << PLLB_DIV_INT_SHIFT)
#define PLLB_DIV_FRAC(val)		((val) << PLLB_DIV_FRAC_SHIFT)

#define SYS_CTRL_CKCTRL_PCI_DIV_BIT	0
#define SYS_CTRL_CKCTRL_SLOW_BIT	8

#define SYS_CTRL_UART2_DEQ_EN		0
#define SYS_CTRL_UART3_DEQ_EN		1
#define SYS_CTRL_UART3_IQ_EN		2
#define SYS_CTRL_UART4_IQ_EN		3
#define SYS_CTRL_UART4_NOT_PCI_MODE	4

#define SYS_CTRL_PCI_CTRL1_PCI_STATIC_RQ_BIT	11

#define PLLA_REFDIV_MASK		0x3F
#define PLLA_REFDIV_SHIFT		8
#define PLLA_OUTDIV_MASK		0x7
#define PLLA_OUTDIV_SHIFT		4

/* bit numbers of clock control register */
#define SYS_CTRL_CLK_COPRO		0
#define SYS_CTRL_CLK_DMA		1
#define SYS_CTRL_CLK_CIPHER		2
#define SYS_CTRL_CLK_SD			3
#define SYS_CTRL_CLK_SATA		4
#define SYS_CTRL_CLK_I2S		5
#define SYS_CTRL_CLK_USBHS		6
#define SYS_CTRL_CLK_MACA		7
#define SYS_CTRL_CLK_MAC		SYS_CTRL_CLK_MACA
#define SYS_CTRL_CLK_PCIEA		8
#define SYS_CTRL_CLK_STATIC		9
#define SYS_CTRL_CLK_MACB		10
#define SYS_CTRL_CLK_PCIEB		11
#define SYS_CTRL_CLK_REF600		12
#define SYS_CTRL_CLK_USBDEV		13
#define SYS_CTRL_CLK_DDR		14
#define SYS_CTRL_CLK_DDRPHY		15
#define SYS_CTRL_CLK_DDRCK		16


/* bit numbers of reset control register */
#define SYS_CTRL_RST_SCU		0
#define SYS_CTRL_RST_COPRO		1
#define SYS_CTRL_RST_ARM0		2
#define SYS_CTRL_RST_ARM1		3
#define SYS_CTRL_RST_USBHS		4
#define SYS_CTRL_RST_USBHSPHYA		5
#define SYS_CTRL_RST_MACA		6
#define SYS_CTRL_RST_MAC		SYS_CTRL_RST_MACA
#define SYS_CTRL_RST_PCIEA		7
#define SYS_CTRL_RST_SGDMA		8
#define SYS_CTRL_RST_CIPHER		9
#define SYS_CTRL_RST_DDR		10
#define SYS_CTRL_RST_SATA		11
#define SYS_CTRL_RST_SATA_LINK		12
#define SYS_CTRL_RST_SATA_PHY		13
#define SYS_CTRL_RST_PCIEPHY		14
#define SYS_CTRL_RST_STATIC		15
#define SYS_CTRL_RST_GPIO		16
#define SYS_CTRL_RST_UART1		17
#define SYS_CTRL_RST_UART2		18
#define SYS_CTRL_RST_MISC		19
#define SYS_CTRL_RST_I2S		20
#define SYS_CTRL_RST_SD			21
#define SYS_CTRL_RST_MACB		22
#define SYS_CTRL_RST_PCIEB		23
#define SYS_CTRL_RST_VIDEO		24
#define SYS_CTRL_RST_DDR_PHY		25
#define SYS_CTRL_RST_USBHSPHYB		26
#define SYS_CTRL_RST_USBDEV		27
#define SYS_CTRL_RST_ARMDBG		29
#define SYS_CTRL_RST_PLLA		30
#define SYS_CTRL_RST_PLLB		31

static inline void oxnas_register_clear_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val &= ~mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_set_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val |= mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_value_mask(void __iomem *p,
					     unsigned mask, unsigned new_value)
{
	/* TODO sanity check mask & new_value = new_value */
	u32 val = readl_relaxed(p);

	val &= ~mask;
	val |= new_value;
	writel_relaxed(val, p);
}

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
