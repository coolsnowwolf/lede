// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe host controller driver for Rockchip SoCs.
 *
 * Copyright (C) 2021 Rockchip Electronics Co., Ltd.
 *		http://www.rock-chips.com
 *
 * Author: Simon Xue <xxm@rock-chips.com>
 */

#include <dt-bindings/phy/phy.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/iopoll.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/irqdomain.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/phy/phy.h>
#include <linux/phy/pcie.h>
#include <linux/regmap.h>
#include <linux/reset.h>

#include "pcie-designware.h"
#include "rockchip-pcie-dma.h"

#define PCIE_DMA_OFFSET			0x380000

#define PCIE_DMA_CTRL_OFF		0x8
#define PCIE_DMA_WR_ENB			0xc
#define PCIE_DMA_WR_CTRL_LO		0x200
#define PCIE_DMA_WR_CTRL_HI		0x204
#define PCIE_DMA_WR_XFERSIZE		0x208
#define PCIE_DMA_WR_SAR_PTR_LO		0x20c
#define PCIE_DMA_WR_SAR_PTR_HI		0x210
#define PCIE_DMA_WR_DAR_PTR_LO		0x214
#define PCIE_DMA_WR_DAR_PTR_HI		0x218
#define PCIE_DMA_WR_WEILO		0x18
#define PCIE_DMA_WR_WEIHI		0x1c
#define PCIE_DMA_WR_DOORBELL		0x10
#define PCIE_DMA_WR_INT_STATUS		0x4c
#define PCIE_DMA_WR_INT_MASK		0x54
#define PCIE_DMA_WR_INT_CLEAR		0x58

#define PCIE_DMA_RD_ENB			0x2c
#define PCIE_DMA_RD_CTRL_LO		0x300
#define PCIE_DMA_RD_CTRL_HI		0x304
#define PCIE_DMA_RD_XFERSIZE		0x308
#define PCIE_DMA_RD_SAR_PTR_LO		0x30c
#define PCIE_DMA_RD_SAR_PTR_HI		0x310
#define PCIE_DMA_RD_DAR_PTR_LO		0x314
#define PCIE_DMA_RD_DAR_PTR_HI		0x318
#define PCIE_DMA_RD_WEILO		0x38
#define PCIE_DMA_RD_WEIHI		0x3c
#define PCIE_DMA_RD_DOORBELL		0x30
#define PCIE_DMA_RD_INT_STATUS		0xa0
#define PCIE_DMA_RD_INT_MASK		0xa8
#define PCIE_DMA_RD_INT_CLEAR		0xac

#define PCIE_DMA_CHANEL_MAX_NUM		2

#define PCIE_CAP_LINK_CONTROL2_LINK_STATUS	0xa0

#define PCIE_CLIENT_INTR_STATUS_MSG_RX	0x04
#define PME_TO_ACK			(BIT(9) | BIT(25))
#define PCIE_CLIENT_INTR_STATUS_LEGACY	0x08
#define PCIE_CLIENT_INTR_STATUS_MISC	0x10
#define PCIE_CLIENT_INTR_MASK_LEGACY	0x1c
#define UNMASK_ALL_LEGACY_INT		0xffff0000
#define MASK_LEGACY_INT(x)		(0x00110011 << x)
#define UNMASK_LEGACY_INT(x)		(0x00110000 << x)
#define PCIE_CLIENT_INTR_MASK		0x24
#define PCIE_CLIENT_POWER		0x2c
#define READY_ENTER_L23			BIT(3)
#define PCIE_CLIENT_MSG_GEN		0x34
#define PME_TURN_OFF			(BIT(4) | BIT(20))
#define PCIE_CLIENT_GENERAL_DEBUG	0x104
#define PCIE_CLIENT_HOT_RESET_CTRL	0x180
#define PCIE_LTSSM_APP_DLY1_EN		BIT(0)
#define PCIE_LTSSM_APP_DLY2_EN		BIT(1)
#define PCIE_LTSSM_APP_DLY1_DONE	BIT(2)
#define PCIE_LTSSM_APP_DLY2_DONE	BIT(3)
#define PCIE_LTSSM_ENABLE_ENHANCE	BIT(4)
#define PCIE_CLIENT_LTSSM_STATUS	0x300
#define SMLH_LINKUP			BIT(16)
#define RDLH_LINKUP			BIT(17)
#define PCIE_CLIENT_CDM_RASDES_TBA_INFO_CMN 0x154
#define PCIE_CLIENT_DBG_FIFO_MODE_CON	0x310
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D0 0x320
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D1 0x324
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D0 0x328
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D1 0x32c
#define PCIE_CLIENT_DBG_FIFO_STATUS	0x350
#define PCIE_CLIENT_DBG_TRANSITION_DATA	0xffff0000
#define PCIE_CLIENT_DBF_EN		0xffff0007

#define PCIE_PL_ORDER_RULE_CTRL_OFF	0x8B4
#define RK_PCIE_L2_TMOUT_US		5000
#define RK_PCIE_HOTRESET_TMOUT_US	10000
#define PORT_LOGIC_LTSSM_STATE_L2(ltssm) \
	((ltssm & PORT_LOGIC_LTSSM_STATE_MASK) == 0x15)
#define RK_PCIE_ENUM_HW_RETRYIES	2

#define PORT_LINK_LPBK_ENABLE		BIT(2)

struct rk_pcie {
	struct dw_pcie			*pci;
	void __iomem			*dbi_base;
	void __iomem			*apb_base;
	struct phy			*phy;
	struct clk_bulk_data		*clks;
	struct reset_control		*rsts;
	unsigned int			clk_cnt;
	struct gpio_desc		*rst_gpio;
	u32				perst_inactive_ms;
	struct dma_trx_obj		*dma_obj;
	bool				in_suspend;
	bool				skip_scan_in_resume;
	bool				is_signal_test;
	bool				bifurcation;
	bool				supports_clkreq;
	struct regulator		*vpcie3v3;
	struct irq_domain		*irq_domain;
	raw_spinlock_t			intx_lock;
	u16				aspm;
	u32				l1ss_ctl1;
	struct dentry			*debugfs;
	struct workqueue_struct		*hot_rst_wq;
	struct work_struct		hot_rst_work;
	u32				comp_prst[2];
	u32				intx;
};

#define to_rk_pcie(x)	dev_get_drvdata((x)->dev)
static int rk_pcie_disable_power(struct rk_pcie *rk_pcie);
static int rk_pcie_enable_power(struct rk_pcie *rk_pcie);

static int rk_pcie_read(void __iomem *addr, int size, u32 *val)
{
	if ((uintptr_t)addr & (size - 1)) {
		*val = 0;
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}

	if (size == 4) {
		*val = readl(addr);
	} else if (size == 2) {
		*val = readw(addr);
	} else if (size == 1) {
		*val = readb(addr);
	} else {
		*val = 0;
		return PCIBIOS_BAD_REGISTER_NUMBER;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int rk_pcie_write(void __iomem *addr, int size, u32 val)
{
	if ((uintptr_t)addr & (size - 1))
		return PCIBIOS_BAD_REGISTER_NUMBER;

	if (size == 4)
		writel(val, addr);
	else if (size == 2)
		writew(val, addr);
	else if (size == 1)
		writeb(val, addr);
	else
		return PCIBIOS_BAD_REGISTER_NUMBER;

	return PCIBIOS_SUCCESSFUL;
}

static u32 __rk_pcie_read_apb(struct rk_pcie *rk_pcie, void __iomem *base,
			u32 reg, size_t size)
{
	int ret;
	u32 val;

	ret = rk_pcie_read(base + reg, size, &val);
	if (ret)
		dev_err(rk_pcie->pci->dev, "Read APB address failed\n");

	return val;
}

static void __rk_pcie_write_apb(struct rk_pcie *rk_pcie, void __iomem *base,
			u32 reg, size_t size, u32 val)
{
	int ret;

	ret = rk_pcie_write(base + reg, size, val);
	if (ret)
		dev_err(rk_pcie->pci->dev, "Write APB address failed\n");
}

static inline u32 rk_pcie_readl_apb(struct rk_pcie *rk_pcie, u32 reg)
{
	return __rk_pcie_read_apb(rk_pcie, rk_pcie->apb_base, reg, 0x4);
}

static inline void rk_pcie_writel_apb(struct rk_pcie *rk_pcie, u32 reg,
					u32 val)
{
	__rk_pcie_write_apb(rk_pcie, rk_pcie->apb_base, reg, 0x4, val);
}

#if defined(CONFIG_PCIEASPM)
static void disable_aspm_l1ss(struct rk_pcie *rk_pcie)
{
	u32 val, cfg_link_cap_l1sub;

	val = dw_pcie_find_ext_capability(rk_pcie->pci, PCI_EXT_CAP_ID_L1SS);
	if (!val) {
		dev_err(rk_pcie->pci->dev, "can't find l1ss cap\n");

		return;
	}

	cfg_link_cap_l1sub = val + PCI_L1SS_CAP;

	val = dw_pcie_readl_dbi(rk_pcie->pci, cfg_link_cap_l1sub);
	val &= ~(PCI_L1SS_CAP_ASPM_L1_1 | PCI_L1SS_CAP_ASPM_L1_2 | PCI_L1SS_CAP_L1_PM_SS);
	dw_pcie_writel_dbi(rk_pcie->pci, cfg_link_cap_l1sub, val);
}
#else
static inline void disable_aspm_l1ss(struct rk_pcie *rk_pcie) { return; }
#endif

static inline void rk_pcie_set_mode(struct rk_pcie *rk_pcie)
{
	if (rk_pcie->supports_clkreq) {
		/* Application is ready to have reference clock removed */
		rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_POWER, 0x00010001);
	} else {
		/* Pull down CLKREQ# to assert the connecting CLOCK_GEN OE */
		rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_POWER, 0x30011000);
		disable_aspm_l1ss(rk_pcie);
	}
	rk_pcie_writel_apb(rk_pcie, 0x0, 0xf00040);
}

static inline void rk_pcie_link_status_clear(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_GENERAL_DEBUG, 0x0);
}

static inline void rk_pcie_disable_ltssm(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, 0x0, 0xc0008);
}

static inline void rk_pcie_enable_ltssm(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, 0x0, 0xC000C);
}

static int rk_pcie_link_up(struct dw_pcie *pci)
{
	struct rk_pcie *rk_pcie = to_rk_pcie(pci);
	u32 val;

	val = rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_LTSSM_STATUS);
	if ((val & (RDLH_LINKUP | SMLH_LINKUP)) == 0x30000)
		return 1;

	return 0;
}

static void rk_pcie_enable_debug(struct rk_pcie *rk_pcie)
{
	if (!IS_ENABLED(CONFIG_DEBUG_FS))
		return;

	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_PTN_HIT_D0,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_PTN_HIT_D1,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_TRN_HIT_D0,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_TRN_HIT_D1,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_MODE_CON,
			   PCIE_CLIENT_DBF_EN);
}

static int rk_pcie_establish_link(struct dw_pcie *pci)
{
	struct rk_pcie *rk_pcie = to_rk_pcie(pci);
	int hw_retries = 0;
	int retries;
	u32 ltssm;

	/*
	 * For standard RC, even if the link has been setup by firmware,
	 * we still need to reset link as we need to remove all resource info
	 * from devices, for instance BAR, as it wasn't assigned by kernel.
	 */
	if (dw_pcie_link_up(pci)) {
		dev_err(pci->dev, "link is already up\n");
		return 0;
	}

	for (hw_retries = 0; hw_retries < RK_PCIE_ENUM_HW_RETRYIES; hw_retries++) {
		/* Rest the device */
		gpiod_set_value_cansleep(rk_pcie->rst_gpio, 0);

		rk_pcie_disable_ltssm(rk_pcie);
		rk_pcie_link_status_clear(rk_pcie);
		rk_pcie_enable_debug(rk_pcie);

		/* Enable client reset or link down interrupt */
		rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK, 0x40000);

		/* Enable LTSSM */
		rk_pcie_enable_ltssm(rk_pcie);

		/*
		 * PCIe requires the refclk to be stable for 100µs prior to releasing
		 * PERST and T_PVPERL (Power stable to PERST# inactive) should be a
		 * minimum of 100ms.  See table 2-4 in section 2.6.2 AC, the PCI Express
		 * Card Electromechanical Specification 3.0. So 100ms in total is the min
		 * requuirement here. We add a 200ms by default for sake of hoping everthings
		 * work fine. If it doesn't, please add more in DT node by add rockchip,perst-inactive-ms.
		 */
		msleep(rk_pcie->perst_inactive_ms);
		gpiod_set_value_cansleep(rk_pcie->rst_gpio, 1);

		/*
		 * Add this 1ms delay because we observe link is always up stably after it and
		 * could help us save 20ms for scanning devices.
		 */
		usleep_range(1000, 1100);

		for (retries = 0; retries < 100; retries++) {
			if (dw_pcie_link_up(pci)) {
				/*
				 * We may be here in case of L0 in Gen1. But if EP is capable
				 * of Gen2 or Gen3, Gen switch may happen just in this time, but
				 * we keep on accessing devices in unstable link status. Given
				 * that LTSSM max timeout is 24ms per period, we can wait a bit
				 * more for Gen switch.
				 */
				msleep(50);
				/* In case link drop after linkup, double check it */
				if (dw_pcie_link_up(pci)) {
					dev_info(pci->dev, "PCIe Link up, LTSSM is 0x%x\n",
						rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_LTSSM_STATUS));
					return 0;
				}
			}

			dev_info_ratelimited(pci->dev, "PCIe Linking... LTSSM is 0x%x\n",
					rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_LTSSM_STATUS));
			msleep(20);
		}

		/*
		 * In response to the situation where PCIe peripherals cannot be
		 * enumerated due tosignal abnormalities, reset PERST# and reset
		 * the peripheral power supply, then restart the enumeration.
		 */
		ltssm = rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_LTSSM_STATUS);
		dev_err(pci->dev, "PCIe Link Fail, LTSSM is 0x%x, hw_retries=%d\n", ltssm, hw_retries);
		if (ltssm >= 3 && !rk_pcie->is_signal_test) {
			rk_pcie_disable_power(rk_pcie);
			msleep(1000);
			rk_pcie_enable_power(rk_pcie);
		} else {
			break;
		}
	}

	return rk_pcie->is_signal_test == true ? 0 : -EINVAL;
}

static bool rk_pcie_udma_enabled(struct rk_pcie *rk_pcie)
{
	return dw_pcie_readl_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
				 PCIE_DMA_CTRL_OFF);
}

static int rk_pcie_init_dma_trx(struct rk_pcie *rk_pcie)
{
	if (!rk_pcie_udma_enabled(rk_pcie))
		return 0;

	rk_pcie->dma_obj = pcie_dw_dmatest_register(rk_pcie->pci->dev, true);
	if (IS_ERR(rk_pcie->dma_obj)) {
		dev_err(rk_pcie->pci->dev, "failed to prepare dmatest\n");
		return -EINVAL;
	}

	/* Enable client write and read interrupt */
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK, 0xc000000);

	/* Enable core write interrupt */
	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_WR_INT_MASK,
			   0x0);
	/* Enable core read interrupt */
	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_RD_INT_MASK,
			   0x0);
	return 0;
}

static struct dw_pcie_host_ops rk_pcie_host_ops;

static int rk_add_pcie_port(struct rk_pcie *rk_pcie, struct platform_device *pdev)
{
	int ret;
	struct dw_pcie *pci = rk_pcie->pci;
	struct dw_pcie_rp *pp = &pci->pp;
	struct device *dev = pci->dev;

	pp->ops = &rk_pcie_host_ops;

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "failed to initialize host\n");
		return ret;
	}

	/* Disable BAR0 BAR1 */
	dw_pcie_writel_dbi2(pci, PCI_BASE_ADDRESS_0, 0x0);
	dw_pcie_writel_dbi2(pci, PCI_BASE_ADDRESS_1, 0x0);

	return 0;
}

static int rk_pcie_clk_init(struct rk_pcie *rk_pcie)
{
	struct device *dev = rk_pcie->pci->dev;
	int ret;

	rk_pcie->clk_cnt = devm_clk_bulk_get_all(dev, &rk_pcie->clks);
	if (rk_pcie->clk_cnt < 1)
		return -ENODEV;

	ret = clk_bulk_prepare_enable(rk_pcie->clk_cnt, rk_pcie->clks);
	if (ret) {
		dev_err(dev, "failed to prepare enable pcie bulk clks: %d\n", ret);
		return ret;
	}

	return 0;
}

static int rk_pcie_resource_get(struct platform_device *pdev,
					 struct rk_pcie *rk_pcie)
{
	struct resource *dbi_base;
	struct resource *apb_base;

	dbi_base = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						"pcie-dbi");
	if (!dbi_base) {
		dev_err(&pdev->dev, "get pcie-dbi failed\n");
		return -ENODEV;
	}

	rk_pcie->dbi_base = devm_ioremap_resource(&pdev->dev, dbi_base);
	if (IS_ERR(rk_pcie->dbi_base))
		return PTR_ERR(rk_pcie->dbi_base);

	rk_pcie->pci->dbi_base = rk_pcie->dbi_base;
	rk_pcie->pci->dbi_base2 = rk_pcie->pci->dbi_base + SZ_1M;

	apb_base = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						"pcie-apb");
	if (!apb_base) {
		dev_err(&pdev->dev, "get pcie-apb failed\n");
		return -ENODEV;
	}
	rk_pcie->apb_base = devm_ioremap_resource(&pdev->dev, apb_base);
	if (IS_ERR(rk_pcie->apb_base))
		return PTR_ERR(rk_pcie->apb_base);

	/*
	 * Rest the device before enabling power because some of the
	 * platforms may use external refclk input with the some power
	 * rail connect to 100MHz OSC chip. So once the power is up for
	 * the slot and the refclk is available, which isn't quite follow
	 * the spec. We should make sure it is in reset state before
	 * everthing's ready.
	 */
	rk_pcie->rst_gpio = devm_gpiod_get_optional(&pdev->dev, "reset",
						    GPIOD_OUT_LOW);
	if (IS_ERR(rk_pcie->rst_gpio)) {
		dev_err(&pdev->dev, "invalid reset-gpios property in node\n");
		return PTR_ERR(rk_pcie->rst_gpio);
	}

	if (device_property_read_u32(&pdev->dev, "rockchip,perst-inactive-ms",
				     &rk_pcie->perst_inactive_ms))
		rk_pcie->perst_inactive_ms = 200;

	return 0;
}

static int rk_pcie_phy_init(struct rk_pcie *rk_pcie)
{
	int ret;
	struct device *dev = rk_pcie->pci->dev;

	rk_pcie->phy = devm_phy_optional_get(dev, "pcie-phy");
	if (IS_ERR(rk_pcie->phy)) {
		if (PTR_ERR(rk_pcie->phy) != -EPROBE_DEFER)
			dev_err(dev, "missing phy\n");
		return PTR_ERR(rk_pcie->phy);
	}

	ret = phy_set_mode_ext(rk_pcie->phy, PHY_MODE_PCIE, PHY_MODE_PCIE_RC);
	if (ret) {
		dev_err(dev, "fail to set phy to rc mode, err %d\n", ret);
		return ret;
	}

	if (rk_pcie->bifurcation)
		phy_set_mode_ext(rk_pcie->phy, PHY_MODE_PCIE, PHY_MODE_PCIE_BIFURCATION);

	ret = phy_init(rk_pcie->phy);
	if (ret < 0) {
		dev_err(dev, "fail to init phy, err %d\n", ret);
		return ret;
	}

	phy_power_on(rk_pcie->phy);

	return 0;
}

static void rk_pcie_start_dma_rd(struct dma_trx_obj *obj, struct dma_table *cur, int ctr_off)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(obj->dev);

	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_RD_ENB,
			   cur->enb.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_CTRL_LO,
			   cur->ctx_reg.ctrllo.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_CTRL_HI,
			   cur->ctx_reg.ctrlhi.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_XFERSIZE,
			   cur->ctx_reg.xfersize);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_SAR_PTR_LO,
			   cur->ctx_reg.sarptrlo);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_SAR_PTR_HI,
			   cur->ctx_reg.sarptrhi);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_DAR_PTR_LO,
			   cur->ctx_reg.darptrlo);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_RD_DAR_PTR_HI,
			   cur->ctx_reg.darptrhi);
	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_RD_DOORBELL,
			   cur->start.asdword);
}

static void rk_pcie_start_dma_wr(struct dma_trx_obj *obj, struct dma_table *cur, int ctr_off)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(obj->dev);

	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_WR_ENB,
			   cur->enb.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_CTRL_LO,
			   cur->ctx_reg.ctrllo.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_CTRL_HI,
			   cur->ctx_reg.ctrlhi.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_XFERSIZE,
			   cur->ctx_reg.xfersize);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_SAR_PTR_LO,
			   cur->ctx_reg.sarptrlo);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_SAR_PTR_HI,
			   cur->ctx_reg.sarptrhi);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_DAR_PTR_LO,
			   cur->ctx_reg.darptrlo);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_DAR_PTR_HI,
			   cur->ctx_reg.darptrhi);
	dw_pcie_writel_dbi(rk_pcie->pci, ctr_off + PCIE_DMA_WR_WEILO,
			   cur->weilo.asdword);
	dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET + PCIE_DMA_WR_DOORBELL,
			   cur->start.asdword);
}

static void rk_pcie_start_dma_dwc(struct dma_trx_obj *obj, struct dma_table *table)
{
	int dir = table->dir;
	int chn = table->chn;

	int ctr_off = PCIE_DMA_OFFSET + chn * 0x200;

	if (dir == DMA_FROM_BUS)
		rk_pcie_start_dma_rd(obj, table, ctr_off);
	else if (dir == DMA_TO_BUS)
		rk_pcie_start_dma_wr(obj, table, ctr_off);
}

static void rk_pcie_config_dma_dwc(struct dma_table *table)
{
	table->enb.enb = 0x1;
	table->ctx_reg.ctrllo.lie = 0x1;
	table->ctx_reg.ctrllo.rie = 0x0;
	table->ctx_reg.ctrllo.td = 0x1;
	table->ctx_reg.ctrlhi.asdword = 0x0;
	table->ctx_reg.xfersize = table->buf_size;
	if (table->dir == DMA_FROM_BUS) {
		table->ctx_reg.sarptrlo = (u32)(table->bus & 0xffffffff);
		table->ctx_reg.sarptrhi = (u32)(table->bus >> 32);
		table->ctx_reg.darptrlo = (u32)(table->local & 0xffffffff);
		table->ctx_reg.darptrhi = (u32)(table->local >> 32);
	} else if (table->dir == DMA_TO_BUS) {
		table->ctx_reg.sarptrlo = (u32)(table->local & 0xffffffff);
		table->ctx_reg.sarptrhi = (u32)(table->local >> 32);
		table->ctx_reg.darptrlo = (u32)(table->bus & 0xffffffff);
		table->ctx_reg.darptrhi = (u32)(table->bus >> 32);
	}
	table->weilo.weight0 = 0x0;
	table->start.stop = 0x0;
	table->start.chnl = table->chn;
}

static void rk_pcie_hot_rst_work(struct work_struct *work)
{
	struct rk_pcie *rk_pcie = container_of(work, struct rk_pcie, hot_rst_work);
	u32 val, status;
	int ret;

	/* Setup command register */
	val = dw_pcie_readl_dbi(rk_pcie->pci, PCI_COMMAND);
	val &= 0xffff0000;
	val |= PCI_COMMAND_IO | PCI_COMMAND_MEMORY |
		PCI_COMMAND_MASTER | PCI_COMMAND_SERR;
	dw_pcie_writel_dbi(rk_pcie->pci, PCI_COMMAND, val);

	if (rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_HOT_RESET_CTRL) & PCIE_LTSSM_APP_DLY2_EN) {
		ret = readl_poll_timeout(rk_pcie->apb_base + PCIE_CLIENT_LTSSM_STATUS,
			 status, ((status & 0x3F) == 0), 100, RK_PCIE_HOTRESET_TMOUT_US);
		if (ret)
			dev_err(rk_pcie->pci->dev, "wait for detect quiet failed!\n");

		rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_HOT_RESET_CTRL,
			(PCIE_LTSSM_APP_DLY2_DONE) | ((PCIE_LTSSM_APP_DLY2_DONE) << 16));
	}
}

static irqreturn_t rk_pcie_sys_irq_handler(int irq, void *arg)
{
	struct rk_pcie *rk_pcie = arg;
	u32 chn;
	union int_status status;
	union int_clear clears;
	u32 reg;

	status.asdword = dw_pcie_readl_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					   PCIE_DMA_WR_INT_STATUS);
	for (chn = 0; chn < PCIE_DMA_CHANEL_MAX_NUM; chn++) {
		if (status.donesta & BIT(chn)) {
			clears.doneclr = 0x1 << chn;
			dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					PCIE_DMA_WR_INT_CLEAR, clears.asdword);
			if (rk_pcie->dma_obj && rk_pcie->dma_obj->cb)
				rk_pcie->dma_obj->cb(rk_pcie->dma_obj, chn, DMA_TO_BUS);
		}

		if (status.abortsta & BIT(chn)) {
			dev_err(rk_pcie->pci->dev, "%s, abort\n", __func__);
			clears.abortclr = 0x1 << chn;
			dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					PCIE_DMA_WR_INT_CLEAR, clears.asdword);
		}
	}

	status.asdword = dw_pcie_readl_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					   PCIE_DMA_RD_INT_STATUS);
	for (chn = 0; chn < PCIE_DMA_CHANEL_MAX_NUM; chn++) {
		if (status.donesta & BIT(chn)) {
			clears.doneclr = 0x1 << chn;
			dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					PCIE_DMA_RD_INT_CLEAR, clears.asdword);
			if (rk_pcie->dma_obj && rk_pcie->dma_obj->cb)
				rk_pcie->dma_obj->cb(rk_pcie->dma_obj, chn, DMA_FROM_BUS);
		}

		if (status.abortsta & BIT(chn)) {
			dev_err(rk_pcie->pci->dev, "%s, abort\n", __func__);
			clears.abortclr = 0x1 << chn;
			dw_pcie_writel_dbi(rk_pcie->pci, PCIE_DMA_OFFSET +
					PCIE_DMA_RD_INT_CLEAR, clears.asdword);
		}
	}

	reg = rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_INTR_STATUS_MISC);
	if (reg & BIT(2))
		queue_work(rk_pcie->hot_rst_wq, &rk_pcie->hot_rst_work);

	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_STATUS_MISC, reg);

	return IRQ_HANDLED;
}

static int rk_pcie_request_sys_irq(struct rk_pcie *rk_pcie,
					struct platform_device *pdev)
{
	int irq, ret;

	irq = platform_get_irq_byname(pdev, "sys");
	if (irq < 0) {
		dev_err(rk_pcie->pci->dev, "missing sys IRQ resource\n");
		return -EINVAL;
	}

	ret = devm_request_irq(rk_pcie->pci->dev, irq, rk_pcie_sys_irq_handler,
			       IRQF_SHARED, "pcie-sys", rk_pcie);
	if (ret) {
		dev_err(rk_pcie->pci->dev, "failed to request PCIe subsystem IRQ\n");
		return ret;
	}

	return 0;
}

static const struct of_device_id rk_pcie_of_match[] = {
	{ .compatible = "rockchip,rk3528-pcie", },
	{ .compatible = "rockchip,rk3562-pcie", },
	{ .compatible = "rockchip,rk3576-pcie", },
	{},
};

MODULE_DEVICE_TABLE(of, rk_pcie_of_match);

static const struct dw_pcie_ops dw_pcie_ops = {
	.start_link = rk_pcie_establish_link,
	.link_up = rk_pcie_link_up,
};

static void rk_pcie_fast_link_setup(struct rk_pcie *rk_pcie)
{
	u32 val;

	/* LTSSM EN ctrl mode */
	val = rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_HOT_RESET_CTRL);
	val |= (PCIE_LTSSM_ENABLE_ENHANCE | PCIE_LTSSM_APP_DLY2_EN)
		| ((PCIE_LTSSM_APP_DLY2_EN | PCIE_LTSSM_ENABLE_ENHANCE) << 16);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_HOT_RESET_CTRL, val);
}

static void rk_pcie_legacy_irq_mask(struct irq_data *d)
{
	struct rk_pcie *rk_pcie = irq_data_get_irq_chip_data(d);
	unsigned long flags;

	raw_spin_lock_irqsave(&rk_pcie->intx_lock, flags);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK_LEGACY,
			   MASK_LEGACY_INT(d->hwirq));
	raw_spin_unlock_irqrestore(&rk_pcie->intx_lock, flags);
}

static void rk_pcie_legacy_irq_unmask(struct irq_data *d)
{
	struct rk_pcie *rk_pcie = irq_data_get_irq_chip_data(d);
	unsigned long flags;

	raw_spin_lock_irqsave(&rk_pcie->intx_lock, flags);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK_LEGACY,
			   UNMASK_LEGACY_INT(d->hwirq));
	raw_spin_unlock_irqrestore(&rk_pcie->intx_lock, flags);
}

static struct irq_chip rk_pcie_legacy_irq_chip = {
	.name		= "rk-pcie-legacy-int",
	.irq_enable	= rk_pcie_legacy_irq_unmask,
	.irq_disable	= rk_pcie_legacy_irq_mask,
	.irq_mask	= rk_pcie_legacy_irq_mask,
	.irq_unmask	= rk_pcie_legacy_irq_unmask,
	.flags		= IRQCHIP_SKIP_SET_WAKE | IRQCHIP_MASK_ON_SUSPEND,
};

static int rk_pcie_intx_map(struct irq_domain *domain, unsigned int irq,
			    irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &rk_pcie_legacy_irq_chip, handle_level_irq);
	irq_set_chip_data(irq, domain->host_data);

	return 0;
}

static const struct irq_domain_ops intx_domain_ops = {
	.map = rk_pcie_intx_map,
};

static void rk_pcie_legacy_int_handler(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct rk_pcie *rockchip = irq_desc_get_handler_data(desc);
	struct device *dev = rockchip->pci->dev;
	u32 reg;
	u32 hwirq;
	int ret;

	chained_irq_enter(chip, desc);

	reg = rk_pcie_readl_apb(rockchip, PCIE_CLIENT_INTR_STATUS_LEGACY);
	reg = reg & 0xf;

	while (reg) {
		hwirq = ffs(reg) - 1;
		reg &= ~BIT(hwirq);

		ret = generic_handle_domain_irq(rockchip->irq_domain, hwirq);
		if (ret)
			dev_err(dev, "unexpected IRQ, INT%d\n", hwirq);
	}

	chained_irq_exit(chip, desc);
}

static int rk_pcie_init_irq_domain(struct rk_pcie *rockchip)
{
	struct device *dev = rockchip->pci->dev;
	struct device_node *intc = of_get_next_child(dev->of_node, NULL);

	if (!intc) {
		dev_err(dev, "missing child interrupt-controller node\n");
		return -EINVAL;
	}

	raw_spin_lock_init(&rockchip->intx_lock);
	rockchip->irq_domain = irq_domain_add_linear(intc, PCI_NUM_INTX,
						     &intx_domain_ops, rockchip);
	if (!rockchip->irq_domain) {
		dev_err(dev, "failed to get a INTx IRQ domain\n");
		return -EINVAL;
	}

	return 0;
}

static int rk_pcie_enable_power(struct rk_pcie *rk_pcie)
{
	int ret = 0;
	struct device *dev = rk_pcie->pci->dev;

	if (IS_ERR(rk_pcie->vpcie3v3))
		return ret;

	ret = regulator_enable(rk_pcie->vpcie3v3);
	if (ret)
		dev_err(dev, "fail to enable vpcie3v3 regulator\n");

	return ret;
}

static int rk_pcie_disable_power(struct rk_pcie *rk_pcie)
{
	int ret = 0;
	struct device *dev = rk_pcie->pci->dev;

	if (IS_ERR(rk_pcie->vpcie3v3))
		return ret;

	ret = regulator_disable(rk_pcie->vpcie3v3);
	if (ret)
		dev_err(dev, "fail to disable vpcie3v3 regulator\n");

	return ret;
}

#define RAS_DES_EVENT(ss, v) \
do { \
	dw_pcie_writel_dbi(pcie->pci, cap_base + 8, v); \
	seq_printf(s, ss "0x%x\n", dw_pcie_readl_dbi(pcie->pci, cap_base + 0xc)); \
} while (0)

static int rockchip_pcie_rasdes_show(struct seq_file *s, void *unused)
{
	struct rk_pcie *pcie = s->private;
	int cap_base;
	u32 val = rk_pcie_readl_apb(pcie, PCIE_CLIENT_CDM_RASDES_TBA_INFO_CMN);
	char *pm;

	if (val & BIT(6))
		pm = "In training";
	else if (val & BIT(5))
		pm = "L1.2";
	else if (val & BIT(4))
		pm = "L1.1";
	else if (val & BIT(3))
		pm = "L1";
	else if (val & BIT(2))
		pm = "L0";
	else if (val & 0x3)
		pm = (val == 0x3) ? "L0s" : (val & BIT(1) ? "RX L0s" : "TX L0s");
	else
		pm = "Invalid";

	seq_printf(s, "Common event signal status: 0x%s\n", pm);

	cap_base = dw_pcie_find_ext_capability(pcie->pci, PCI_EXT_CAP_ID_VNDR);
	if (!cap_base) {
		dev_err(pcie->pci->dev, "Not able to find RASDES CAP!\n");
		return 0;
	}

	RAS_DES_EVENT("EBUF Overflow: ", 0);
	RAS_DES_EVENT("EBUF Under-run: ", 0x0010000);
	RAS_DES_EVENT("Decode Error: ", 0x0020000);
	RAS_DES_EVENT("Running Disparity Error: ", 0x0030000);
	RAS_DES_EVENT("SKP OS Parity Error: ", 0x0040000);
	RAS_DES_EVENT("SYNC Header Error: ", 0x0050000);
	RAS_DES_EVENT("CTL SKP OS Parity Error: ", 0x0060000);
	RAS_DES_EVENT("Detect EI Infer: ", 0x1050000);
	RAS_DES_EVENT("Receiver Error: ", 0x1060000);
	RAS_DES_EVENT("Rx Recovery Request: ", 0x1070000);
	RAS_DES_EVENT("N_FTS Timeout: ", 0x1080000);
	RAS_DES_EVENT("Framing Error: ", 0x1090000);
	RAS_DES_EVENT("Deskew Error: ", 0x10a0000);
	RAS_DES_EVENT("BAD TLP: ", 0x2000000);
	RAS_DES_EVENT("LCRC Error: ", 0x2010000);
	RAS_DES_EVENT("BAD DLLP: ", 0x2020000);
	RAS_DES_EVENT("Replay Number Rollover: ", 0x2030000);
	RAS_DES_EVENT("Replay Timeout: ", 0x2040000);
	RAS_DES_EVENT("Rx Nak DLLP: ", 0x2050000);
	RAS_DES_EVENT("Tx Nak DLLP: ", 0x2060000);
	RAS_DES_EVENT("Retry TLP: ", 0x2070000);
	RAS_DES_EVENT("FC Timeout: ", 0x3000000);
	RAS_DES_EVENT("Poisoned TLP: ", 0x3010000);
	RAS_DES_EVENT("ECRC Error: ", 0x3020000);
	RAS_DES_EVENT("Unsupported Request: ", 0x3030000);
	RAS_DES_EVENT("Completer Abort: ", 0x3040000);
	RAS_DES_EVENT("Completion Timeout: ", 0x3050000);

	return 0;
}

static int rockchip_pcie_rasdes_open(struct inode *inode, struct file *file)
{
	return single_open(file, rockchip_pcie_rasdes_show,
			   inode->i_private);
}

static ssize_t rockchip_pcie_rasdes_write(struct file *file,
				       const char __user *ubuf,
				       size_t count, loff_t *ppos)
{
	struct seq_file *s = file->private_data;
	struct rk_pcie *pcie = s->private;
	char buf[32];
	int cap_base;

	if (copy_from_user(&buf, ubuf, min_t(size_t, sizeof(buf) - 1, count)))
		return -EFAULT;

	cap_base = dw_pcie_find_ext_capability(pcie->pci, PCI_EXT_CAP_ID_VNDR);
	if (!cap_base) {
		dev_err(pcie->pci->dev, "Not able to find RASDES CAP!\n");
		return 0;
	}

	if (!strncmp(buf, "enable", 6))	{
		dev_info(pcie->pci->dev, "RAS DES Event: Enable ALL!\n");
		dw_pcie_writel_dbi(pcie->pci, cap_base + 8, 0x1c);
		dw_pcie_writel_dbi(pcie->pci, cap_base + 8, 0x3);
	} else if (!strncmp(buf, "disable", 7)) {
		dev_info(pcie->pci->dev, "RAS DES Event: disable ALL!\n");
		dw_pcie_writel_dbi(pcie->pci, cap_base + 8, 0x14);
	} else if (!strncmp(buf, "clear", 5)) {
		dev_info(pcie->pci->dev, "RAS DES Event: Clear ALL!\n");
		dw_pcie_writel_dbi(pcie->pci, cap_base + 8, 0x3);
	} else {
		dev_info(pcie->pci->dev, "Not support command!\n");
	}

	return count;
}

static const struct file_operations rockchip_pcie_rasdes_ops = {
	.owner = THIS_MODULE,
	.open = rockchip_pcie_rasdes_open,
	.read = seq_read,
	.write = rockchip_pcie_rasdes_write,
};

static int rockchip_pcie_fifo_show(struct seq_file *s, void *data)
{
	struct rk_pcie *pcie = (struct rk_pcie *)dev_get_drvdata(s->private);
	u32 loop;

	seq_printf(s, "ltssm = 0x%x\n",
		   rk_pcie_readl_apb(pcie, PCIE_CLIENT_LTSSM_STATUS));
	for (loop = 0; loop < 64; loop++)
		seq_printf(s, "fifo_status = 0x%x\n",
			   rk_pcie_readl_apb(pcie, PCIE_CLIENT_DBG_FIFO_STATUS));

	return 0;
}

static void rockchip_pcie_debugfs_exit(struct rk_pcie *pcie)
{
	debugfs_remove_recursive(pcie->debugfs);
	pcie->debugfs = NULL;
}

static int rockchip_pcie_debugfs_init(struct rk_pcie *pcie)
{
	struct dentry *file;

	pcie->debugfs = debugfs_create_dir(dev_name(pcie->pci->dev), NULL);
	if (!pcie->debugfs)
		return -ENOMEM;

	debugfs_create_devm_seqfile(pcie->pci->dev, "dumpfifo",
				    pcie->debugfs,
				    rockchip_pcie_fifo_show);
	file = debugfs_create_file("err_event", 0644, pcie->debugfs,
				   pcie, &rockchip_pcie_rasdes_ops);
	if (!file)
		goto remove;

	return 0;

remove:
	rockchip_pcie_debugfs_exit(pcie);

	return -ENOMEM;
}

static int rk_pcie_really_probe(void *p)
{
	struct platform_device *pdev = p;
	struct device *dev = &pdev->dev;
	struct rk_pcie *rk_pcie;
	struct dw_pcie *pci;
	const struct of_device_id *match;
	int irq, ret;
	u32 val = 0;

	match = of_match_device(rk_pcie_of_match, dev);
	if (!match) {
		ret = -EINVAL;
		goto release_driver;
	}

	rk_pcie = devm_kzalloc(dev, sizeof(*rk_pcie), GFP_KERNEL);
	if (!rk_pcie) {
		ret = -ENOMEM;
		goto release_driver;
	}

	pci = devm_kzalloc(dev, sizeof(*pci), GFP_KERNEL);
	if (!pci) {
		ret = -ENOMEM;
		goto release_driver;
	}

	pci->dev = dev;
	pci->ops = &dw_pcie_ops;
	rk_pcie->pci = pci;

	if (device_property_read_bool(dev, "rockchip,bifurcation"))
		rk_pcie->bifurcation = true;

	ret = rk_pcie_resource_get(pdev, rk_pcie);
	if (ret) {
		dev_err(dev, "resource init failed\n");
		goto release_driver;
	}

	rk_pcie->supports_clkreq = device_property_read_bool(dev, "supports-clkreq");

retry_regulator:
	/* DON'T MOVE ME: must be enable before phy init */
	rk_pcie->vpcie3v3 = devm_regulator_get_optional(dev, "vpcie3v3");
	if (IS_ERR(rk_pcie->vpcie3v3)) {
		if (PTR_ERR(rk_pcie->vpcie3v3) != -ENODEV) {
			if (IS_ENABLED(CONFIG_PCIE_RK_THREADED_INIT)) {
				/* Deferred but in threaded context for most 10s */
				msleep(20);
				if (++val < 500)
					goto retry_regulator;
			}

			ret = PTR_ERR(rk_pcie->vpcie3v3);
			goto release_driver;
		}

		dev_err(dev, "no vpcie3v3 regulator found\n");
	}

	ret = rk_pcie_enable_power(rk_pcie);
	if (ret)
		goto release_driver;

	ret = rk_pcie_phy_init(rk_pcie);
	if (ret) {
		dev_err(dev, "phy init failed\n");
		goto disable_vpcie3v3;
	}

	rk_pcie->rsts = devm_reset_control_array_get_exclusive(dev);
	if (IS_ERR(rk_pcie->rsts)) {
		ret = PTR_ERR(rk_pcie->rsts);
		dev_err(dev, "failed to get reset lines\n");
		goto disable_phy;
	}

	reset_control_deassert(rk_pcie->rsts);

	ret = rk_pcie_clk_init(rk_pcie);
	if (ret) {
		dev_err(dev, "clock init failed\n");
		goto disable_phy;
	}

	/*
	 * Misc interrupts was masked by default. However, they will be
	 * unmasked by FW before jumpping into kernel. Mask all misc interrupts,
	 * as we don't need to ack them before registering irq. And they will be
	 * unmasked later.
	 */
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK, 0xffffffff);

	ret = rk_pcie_request_sys_irq(rk_pcie, pdev);
	if (ret) {
		dev_err(dev, "pcie irq init failed\n");
		goto disable_clk;
	}

	platform_set_drvdata(pdev, rk_pcie);

	dw_pcie_dbi_ro_wr_en(pci);

	rk_pcie_fast_link_setup(rk_pcie);

	/* Legacy interrupt is optional */
	ret = rk_pcie_init_irq_domain(rk_pcie);
	if (!ret) {
		irq = platform_get_irq_byname(pdev, "legacy");
		if (irq >= 0) {
			irq_set_chained_handler_and_data(irq, rk_pcie_legacy_int_handler,
							 rk_pcie);
			/* Unmask all legacy interrupt from INTA~INTD  */
			rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK_LEGACY,
					   UNMASK_ALL_LEGACY_INT);
		} else {
			dev_err(dev, "missing legacy IRQ resource\n");
		}
	}

	/* Set PCIe RC mode */
	rk_pcie_set_mode(rk_pcie);

	/* Force into loopback master mode */
	if (device_property_read_bool(dev, "rockchip,lpbk-master")) {
		val = dw_pcie_readl_dbi(pci, PCIE_PORT_LINK_CONTROL);
		val |= PORT_LINK_LPBK_ENABLE;
		dw_pcie_writel_dbi(pci, PCIE_PORT_LINK_CONTROL, val);
		rk_pcie->is_signal_test = true;
	}

	/*
	 * Force into compliance mode
	 * comp_prst is a two dimensional array of which the first element
	 * stands for speed mode, and the second one is preset value encoding:
	 * [0] 0->SMA tool control the signal switch, 1/2/3 is for manual Gen setting
	 * [1] transmitter setting for manual Gen setting, valid only if [0] isn't zero.
	 */
	if (!device_property_read_u32_array(dev, "rockchip,compliance-mode",
					    rk_pcie->comp_prst, 2)) {
		BUG_ON(rk_pcie->comp_prst[0] > 3 || rk_pcie->comp_prst[1] > 10);
		if (!rk_pcie->comp_prst[0]) {
			dev_info(dev, "Auto compliance mode for SMA tool.\n");
		} else {
			dev_info(dev, "compliance mode for soldered board Gen%d, P%d.\n",
				 rk_pcie->comp_prst[0], rk_pcie->comp_prst[1]);
			val = dw_pcie_readl_dbi(pci, PCIE_CAP_LINK_CONTROL2_LINK_STATUS);
			val |= BIT(4) | rk_pcie->comp_prst[0] | (rk_pcie->comp_prst[1] << 12);
			dw_pcie_writel_dbi(pci, PCIE_CAP_LINK_CONTROL2_LINK_STATUS, val);
		}
		rk_pcie->is_signal_test = true;
	}

	/* Skip waiting for training to pass in system PM routine */
	if (device_property_read_bool(dev, "rockchip,skip-scan-in-resume"))
		rk_pcie->skip_scan_in_resume = true;

	rk_pcie->hot_rst_wq = create_singlethread_workqueue("rk_pcie_hot_rst_wq");
	if (!rk_pcie->hot_rst_wq) {
		dev_err(dev, "failed to create hot_rst workqueue\n");
		ret = -ENOMEM;
		goto remove_irq_domain;
	}
	INIT_WORK(&rk_pcie->hot_rst_work, rk_pcie_hot_rst_work);

	ret = rk_add_pcie_port(rk_pcie, pdev);

	if (rk_pcie->is_signal_test == true)
		return 0;

	if (ret)
		goto remove_rst_wq;

	ret = rk_pcie_init_dma_trx(rk_pcie);
	if (ret) {
		dev_err(dev, "failed to add dma extension\n");
		goto remove_rst_wq;
	}

	if (rk_pcie->dma_obj) {
		rk_pcie->dma_obj->start_dma_func = rk_pcie_start_dma_dwc;
		rk_pcie->dma_obj->config_dma_func = rk_pcie_config_dma_dwc;
	}

	dw_pcie_dbi_ro_wr_dis(pci);

	device_init_wakeup(dev, true);

	/* Enable async system PM for multiports SoC */
	device_enable_async_suspend(dev);

	if (IS_ENABLED(CONFIG_DEBUG_FS)) {
		ret = rockchip_pcie_debugfs_init(rk_pcie);
		if (ret < 0)
			dev_err(dev, "failed to setup debugfs: %d\n", ret);

		/* Enable RASDES Error event by default */
		val = dw_pcie_find_ext_capability(rk_pcie->pci, PCI_EXT_CAP_ID_VNDR);
		if (!val) {
			dev_err(dev, "Not able to find RASDES CAP!\n");
			return 0;
		}

		dw_pcie_writel_dbi(rk_pcie->pci, val + 8, 0x1c);
		dw_pcie_writel_dbi(rk_pcie->pci, val + 8, 0x3);
	}

	return 0;

remove_rst_wq:
	destroy_workqueue(rk_pcie->hot_rst_wq);
remove_irq_domain:
	if (rk_pcie->irq_domain)
		irq_domain_remove(rk_pcie->irq_domain);
disable_clk:
	clk_bulk_disable_unprepare(rk_pcie->clk_cnt, rk_pcie->clks);
disable_phy:
	phy_power_off(rk_pcie->phy);
	phy_exit(rk_pcie->phy);
disable_vpcie3v3:
	rk_pcie_disable_power(rk_pcie);
release_driver:
	if (IS_ENABLED(CONFIG_PCIE_RK_THREADED_INIT))
		device_release_driver(dev);

	return ret;
}

static int rk_pcie_probe(struct platform_device *pdev)
{
	if (IS_ENABLED(CONFIG_PCIE_RK_THREADED_INIT)) {
		struct task_struct *tsk;

		tsk = kthread_run(rk_pcie_really_probe, pdev, "rk-pcie");
		if (IS_ERR(tsk)) {
			dev_err(&pdev->dev, "start rk-pcie thread failed\n");
			return PTR_ERR(tsk);
		}

		return 0;
	}

	return rk_pcie_really_probe(pdev);
}

#ifdef CONFIG_PCIEASPM
static void rk_pcie_downstream_dev_to_d0(struct rk_pcie *rk_pcie, bool enable)
{
	struct dw_pcie_rp *pp = &rk_pcie->pci->pp;
	struct pci_bus *child, *root_bus = NULL;
	struct pci_dev *pdev, *bridge;
	u32 val;

	list_for_each_entry(child, &pp->bridge->bus->children, node) {
		/* Bring downstream devices to D3 if they are not already in */
		if (child->parent == pp->bridge->bus) {
			root_bus = child;
			bridge = root_bus->self;
			break;
		}
	}

	if (!root_bus) {
		dev_err(rk_pcie->pci->dev, "Failed to find downstream devices\n");
		return;
	}

	/* Save and restore root bus ASPM */
	if (enable) {
		if (rk_pcie->l1ss_ctl1)
			dw_pcie_writel_dbi(rk_pcie->pci, bridge->l1ss + PCI_L1SS_CTL1, rk_pcie->l1ss_ctl1);

		/* rk_pcie->aspm woule be saved in advance when enable is false */
		dw_pcie_writel_dbi(rk_pcie->pci, bridge->pcie_cap + PCI_EXP_LNKCTL, rk_pcie->aspm);
	} else {
		val = dw_pcie_readl_dbi(rk_pcie->pci, bridge->l1ss + PCI_L1SS_CTL1);
		if (val & PCI_L1SS_CTL1_L1SS_MASK)
			rk_pcie->l1ss_ctl1 = val;
		else
			rk_pcie->l1ss_ctl1 = 0;

		val = dw_pcie_readl_dbi(rk_pcie->pci, bridge->pcie_cap + PCI_EXP_LNKCTL);
		rk_pcie->aspm = val & PCI_EXP_LNKCTL_ASPMC;
		val &= ~(PCI_EXP_LNKCAP_ASPM_L1 | PCI_EXP_LNKCAP_ASPM_L0S);
		dw_pcie_writel_dbi(rk_pcie->pci, bridge->pcie_cap + PCI_EXP_LNKCTL, val);
	}

	list_for_each_entry(pdev, &root_bus->devices, bus_list) {
		if (PCI_SLOT(pdev->devfn) == 0) {
			if (pci_set_power_state(pdev, PCI_D0))
				dev_err(rk_pcie->pci->dev,
					"Failed to transition %s to D3hot state\n",
					dev_name(&pdev->dev));
			if (enable) {
				if (rk_pcie->l1ss_ctl1) {
					pci_read_config_dword(pdev, pdev->l1ss + PCI_L1SS_CTL1, &val);
					val &= ~PCI_L1SS_CTL1_L1SS_MASK;
					val |= (rk_pcie->l1ss_ctl1 & PCI_L1SS_CTL1_L1SS_MASK);
					pci_write_config_dword(pdev, pdev->l1ss + PCI_L1SS_CTL1, val);
				}

				pcie_capability_clear_and_set_word(pdev, PCI_EXP_LNKCTL,
								   PCI_EXP_LNKCTL_ASPMC, rk_pcie->aspm);
			} else {
				pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S | PCIE_LINK_STATE_L1);
			}
		}
	}
}
#endif

static int __maybe_unused rockchip_dw_pcie_suspend(struct device *dev)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(dev);
	struct dw_pcie *pci = rk_pcie->pci;
	int ret = 0;
	u32 status;

	/*
	 * This is as per PCI Express Base r5.0 r1.0 May 22-2019,
	 * 5.2 Link State Power Management (Page #440).
	 *
	 * L2/L3 Ready entry negotiations happen while in the L0 state.
	 * L2/L3 Ready are entered only after the negotiation completes.
	 *
	 * The following example sequence illustrates the multi-step Link state
	 * transition process leading up to entering a system sleep state:
	 * 1. System software directs all Functions of a Downstream component to D3Hot.
	 * 2. The Downstream component then initiates the transition of the Link to L1
	 *    as required.
	 * 3. System software then causes the Root Complex to broadcast the PME_Turn_Off
	 *    Message in preparation for removing the main power source.
	 * 4. This Message causes the subject Link to transition back to L0 in order to
	 *    send it and to enable the Downstream component to respond with PME_TO_Ack.
	 * 5. After sending the PME_TO_Ack, the Downstream component initiates the L2/L3
	 *    Ready transition protocol.
	 */

	/* 1. All sub-devices are in D3hot by PCIe stack */
	dw_pcie_dbi_ro_wr_dis(rk_pcie->pci);

	rk_pcie_link_status_clear(rk_pcie);

	/* 2. Broadcast PME_Turn_Off Message */
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_MSG_GEN, PME_TURN_OFF);
	ret = readl_poll_timeout(rk_pcie->apb_base + PCIE_CLIENT_MSG_GEN,
				 status, !(status & BIT(4)), 20, RK_PCIE_L2_TMOUT_US);
	if (ret) {
		dev_err(dev, "Failed to send PME_Turn_Off\n");
		goto no_l2;
	}

	/* 3. Wait for PME_TO_Ack */
	ret = readl_poll_timeout(rk_pcie->apb_base + PCIE_CLIENT_INTR_STATUS_MSG_RX,
				 status, status & BIT(9), 20, RK_PCIE_L2_TMOUT_US);
	if (ret) {
		dev_err(dev, "Failed to receive PME_TO_Ack\n");
		goto no_l2;
	}

	/* 4. Clear PME_TO_Ack and Wait for ready to enter L23 message */
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_STATUS_MSG_RX, PME_TO_ACK);
	ret = readl_poll_timeout(rk_pcie->apb_base + PCIE_CLIENT_POWER,
				 status, status & READY_ENTER_L23, 20, RK_PCIE_L2_TMOUT_US);
	if (ret) {
		dev_err(dev, "Failed to ready to enter L23\n");
		goto no_l2;
	}

	/* 5. Check we are in L2 */
	ret = readl_poll_timeout(rk_pcie->apb_base + PCIE_CLIENT_LTSSM_STATUS,
				 status, PORT_LOGIC_LTSSM_STATE_L2(status), 20, RK_PCIE_L2_TMOUT_US);
	if (ret)
		dev_err(pci->dev, "Link isn't in L2 idle!\n");

no_l2:
	rk_pcie_disable_ltssm(rk_pcie);

	ret = phy_validate(rk_pcie->phy, PHY_TYPE_PCIE, 0, NULL);
	if (ret && ret != -EOPNOTSUPP) {
		dev_err(dev, "PHY is reused by other controller, check the dts!\n");
		return ret;
	}

	/* make sure assert phy success */
	usleep_range(200, 300);

	phy_power_off(rk_pcie->phy);
	phy_exit(rk_pcie->phy);

	rk_pcie->intx = rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_INTR_MASK_LEGACY);

	clk_bulk_disable_unprepare(rk_pcie->clk_cnt, rk_pcie->clks);

	rk_pcie->in_suspend = true;

	gpiod_set_value_cansleep(rk_pcie->rst_gpio, 0);
	ret = rk_pcie_disable_power(rk_pcie);

	return ret;
}

static int __maybe_unused rockchip_dw_pcie_resume(struct device *dev)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(dev);
	int ret;

	reset_control_assert(rk_pcie->rsts);
	udelay(10);
	reset_control_deassert(rk_pcie->rsts);

	ret = rk_pcie_enable_power(rk_pcie);
	if (ret)
		return ret;

	ret = clk_bulk_prepare_enable(rk_pcie->clk_cnt, rk_pcie->clks);
	if (ret) {
		dev_err(dev, "failed to prepare enable pcie bulk clks: %d\n", ret);
		return ret;
	}

	ret = phy_set_mode_ext(rk_pcie->phy, PHY_MODE_PCIE, PHY_MODE_PCIE_RC);
	if (ret) {
		dev_err(dev, "fail to set phy to rc mode, err %d\n", ret);
		return ret;
	}

	ret = phy_init(rk_pcie->phy);
	if (ret < 0) {
		dev_err(dev, "fail to init phy, err %d\n", ret);
		return ret;
	}

	phy_power_on(rk_pcie->phy);

	dw_pcie_dbi_ro_wr_en(rk_pcie->pci);

	rk_pcie_fast_link_setup(rk_pcie);

	/* Set PCIe RC mode */
	rk_pcie_set_mode(rk_pcie);

	dw_pcie_setup_rc(&rk_pcie->pci->pp);

	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_INTR_MASK_LEGACY,
			   rk_pcie->intx | 0xffff0000);

	ret = rk_pcie_establish_link(rk_pcie->pci);
	if (ret) {
		dev_err(dev, "failed to establish pcie link\n");
		goto err;
	}

	dw_pcie_dbi_ro_wr_dis(rk_pcie->pci);
	rk_pcie->in_suspend = false;

	return 0;
err:
	rk_pcie_disable_power(rk_pcie);

	return ret;
}

#ifdef CONFIG_PCIEASPM
static int rockchip_dw_pcie_prepare(struct device *dev)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(dev);

	dw_pcie_dbi_ro_wr_en(rk_pcie->pci);
	rk_pcie_downstream_dev_to_d0(rk_pcie, false);
	dw_pcie_dbi_ro_wr_dis(rk_pcie->pci);

	return 0;
}

static void rockchip_dw_pcie_complete(struct device *dev)
{
	struct rk_pcie *rk_pcie = dev_get_drvdata(dev);

	dw_pcie_dbi_ro_wr_en(rk_pcie->pci);
	rk_pcie_downstream_dev_to_d0(rk_pcie, true);
	dw_pcie_dbi_ro_wr_dis(rk_pcie->pci);
}
#endif

static const struct dev_pm_ops rockchip_dw_pcie_pm_ops = {
#ifdef CONFIG_PCIEASPM
	.prepare = rockchip_dw_pcie_prepare,
	.complete = rockchip_dw_pcie_complete,
#endif
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(rockchip_dw_pcie_suspend,
				      rockchip_dw_pcie_resume)
};

static struct platform_driver rk_plat_pcie_driver = {
	.driver = {
		.name	= "rk-pcie",
		.of_match_table = rk_pcie_of_match,
		.suppress_bind_attrs = true,
		.pm = &rockchip_dw_pcie_pm_ops,
	},
	.probe = rk_pcie_probe,
};

module_platform_driver(rk_plat_pcie_driver);

MODULE_AUTHOR("Simon Xue <xxm@rock-chips.com>");
MODULE_DESCRIPTION("RockChip PCIe Controller driver");
MODULE_LICENSE("GPL v2");
