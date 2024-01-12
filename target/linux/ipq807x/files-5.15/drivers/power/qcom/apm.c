/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/power/qcom/apm.h>

/*
 *        VDD_APCC
 * =============================================================
 *       |      VDD_MX                  |                    |
 *       |    ==========================|=============       |
 *    ___|___   ___|___    ___|___   ___|___    ___|___   ___|___
 *   |       | |       |  |       | |       |  |       | |       |
 *   | APCC  | | MX HS |  | MX HS | | APCC  |  | MX HS | | APCC  |
 *   |  HS   | |       |  |       | |  HS   |  |       | |  HS   |
 *   |_______| |_______|  |_______| |_______|  |_______| |_______|
 *       |_________|          |_________|         |__________|
 *            |                    |                    |
 *      ______|_____         ______|_____        _______|_____
 *     |            |       |            |      |             |
 *     |            |       |            |      |             |
 *     |  CPU MEM   |       |   L2 MEM   |      |    L3 MEM   |
 *     |   Arrays   |       |   Arrays   |      |    Arrays   |
 *     |            |       |            |      |             |
 *     |____________|       |____________|      |_____________|
 *
 */

/* Register value definitions */
#define APCS_GFMUXA_SEL_VAL            0x13
#define APCS_GFMUXA_DESEL_VAL          0x03
#define MSM_APM_MX_MODE_VAL            0x00
#define MSM_APM_APCC_MODE_VAL          0x10
#define MSM_APM_MX_DONE_VAL            0x00
#define MSM_APM_APCC_DONE_VAL          0x03
#define MSM_APM_OVERRIDE_SEL_VAL       0xb0
#define MSM_APM_SEC_CLK_SEL_VAL        0x30
#define SPM_EVENT_SET_VAL              0x01
#define SPM_EVENT_CLEAR_VAL            0x00

/* Register bit mask definitions */
#define MSM_APM_CTL_STS_MASK            0x0f

/* Register offset definitions */
#define APCC_APM_MODE              0x00000098
#define APCC_APM_CTL_STS           0x000000a8
#define APCS_SPARE                 0x00000068
#define APCS_VERSION               0x00000fd0

#define HMSS_VERSION_1P2           0x10020000

#define MSM_APM_SWITCH_TIMEOUT_US  10
#define SPM_WAKEUP_DELAY_US        2
#define SPM_EVENT_NUM              6

#define MSM_APM_DRIVER_NAME        "qcom,msm-apm"

enum {
	MSM8996_ID,
	MSM8953_ID,
	IPQ807x_ID,
};

struct msm_apm_ctrl_dev {
	struct list_head	list;
	struct device		*dev;
	enum msm_apm_supply	supply;
	spinlock_t		lock;
	void __iomem		*reg_base;
	void __iomem		*apcs_csr_base;
	void __iomem		**apcs_spm_events_addr;
	void __iomem		*apc0_pll_ctl_addr;
	void __iomem		*apc1_pll_ctl_addr;
	u32			version;
	struct dentry		*debugfs;
	u32			msm_id;
};

#if defined(CONFIG_DEBUG_FS)
static struct dentry *apm_debugfs_base;
#endif

static DEFINE_MUTEX(apm_ctrl_list_mutex);
static LIST_HEAD(apm_ctrl_list);

/*
 * Get the resources associated with the APM controller from device tree
 * and remap all I/O addresses that are relevant to this HW revision.
 */
static int msm_apm_ctrl_devm_ioremap(struct platform_device *pdev,
				     struct msm_apm_ctrl_dev *ctrl)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	static const char *res_name[SPM_EVENT_NUM] = {
		"apc0-l2-spm",
		"apc1-l2-spm",
		"apc0-cpu0-spm",
		"apc0-cpu1-spm",
		"apc1-cpu0-spm",
		"apc1-cpu1-spm"
	};
	int i, ret = 0;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pm-apcc-glb");
	if (!res) {
		dev_err(dev, "Missing PM APCC Global register physical address");
		return -EINVAL;
	}
	ctrl->reg_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!ctrl->reg_base) {
		dev_err(dev, "Failed to map PM APCC Global registers\n");
		return -ENOMEM;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "apcs-csr");
	if (!res) {
		dev_err(dev, "Missing APCS CSR physical base address");
		return -EINVAL;
	}
	ctrl->apcs_csr_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!ctrl->apcs_csr_base) {
		dev_err(dev, "Failed to map APCS CSR registers\n");
		return -ENOMEM;
	}

	ctrl->version = readl_relaxed(ctrl->apcs_csr_base + APCS_VERSION);

	if (ctrl->version >= HMSS_VERSION_1P2)
		return ret;

	ctrl->apcs_spm_events_addr = devm_kzalloc(&pdev->dev,
						  SPM_EVENT_NUM
						  * sizeof(void __iomem *),
						  GFP_KERNEL);
	if (!ctrl->apcs_spm_events_addr) {
		dev_err(dev, "Failed to allocate memory for APCS SPM event registers\n");
		return -ENOMEM;
	}

	for (i = 0; i < SPM_EVENT_NUM; i++) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
						   res_name[i]);
		if (!res) {
			dev_err(dev, "Missing address for %s\n", res_name[i]);
			ret = -EINVAL;
			goto free_events;
		}

		ctrl->apcs_spm_events_addr[i] = devm_ioremap(dev, res->start,
						resource_size(res));
		if (!ctrl->apcs_spm_events_addr[i]) {
			dev_err(dev, "Failed to map %s\n", res_name[i]);
			ret = -ENOMEM;
			goto free_events;
		}

		dev_dbg(dev, "%s event phys: %pa virt:0x%p\n", res_name[i],
			&res->start, ctrl->apcs_spm_events_addr[i]);
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   "apc0-pll-ctl");
	if (!res) {
		dev_err(dev, "Missing APC0 PLL CTL physical address\n");
		ret = -EINVAL;
		goto free_events;
	}

	ctrl->apc0_pll_ctl_addr = devm_ioremap(dev,
					   res->start,
					   resource_size(res));
	if (!ctrl->apc0_pll_ctl_addr) {
		dev_err(dev, "Failed to map APC0 PLL CTL register\n");
		ret = -ENOMEM;
		goto free_events;
	}

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   "apc1-pll-ctl");
	if (!res) {
		dev_err(dev, "Missing APC1 PLL CTL physical address\n");
		ret = -EINVAL;
		goto free_events;
	}

	ctrl->apc1_pll_ctl_addr = devm_ioremap(dev,
					   res->start,
					   resource_size(res));
	if (!ctrl->apc1_pll_ctl_addr) {
		dev_err(dev, "Failed to map APC1 PLL CTL register\n");
		ret = -ENOMEM;
		goto free_events;
	}

	return ret;

free_events:
	devm_kfree(dev, ctrl->apcs_spm_events_addr);
	return ret;
}

/* 8953 register offset definition */
#define MSM8953_APM_DLY_CNTR	0x2ac

/* Register field shift definitions */
#define APM_CTL_SEL_SWITCH_DLY_SHIFT	0
#define APM_CTL_RESUME_CLK_DLY_SHIFT	8
#define APM_CTL_HALT_CLK_DLY_SHIFT	16
#define APM_CTL_POST_HALT_DLY_SHIFT	24

/* Register field mask definitions */
#define APM_CTL_SEL_SWITCH_DLY_MASK	GENMASK(7, 0)
#define APM_CTL_RESUME_CLK_DLY_MASK	GENMASK(15, 8)
#define APM_CTL_HALT_CLK_DLY_MASK	GENMASK(23, 16)
#define APM_CTL_POST_HALT_DLY_MASK	GENMASK(31, 24)

/*
 * Get the resources associated with the msm8953 APM controller from
 * device tree, remap all I/O addresses, and program the initial
 * register configuration required for the 8953 APM controller device.
 */
static int msm8953_apm_ctrl_init(struct platform_device *pdev,
				     struct msm_apm_ctrl_dev *ctrl)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	u32 delay_counter, val = 0, regval = 0;
	int rc = 0;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "pm-apcc-glb");
	if (!res) {
		dev_err(dev, "Missing PM APCC Global register physical address\n");
		return -ENODEV;
	}
	ctrl->reg_base = devm_ioremap(dev, res->start, resource_size(res));
	if (!ctrl->reg_base) {
		dev_err(dev, "Failed to map PM APCC Global registers\n");
		return -ENOMEM;
	}

	/*
	 * Initial APM register configuration required before starting
	 * APM HW controller.
	 */
	regval = readl_relaxed(ctrl->reg_base + MSM8953_APM_DLY_CNTR);
	val = regval;

	if (of_find_property(dev->of_node, "qcom,apm-post-halt-delay", NULL)) {
		rc = of_property_read_u32(dev->of_node,
				"qcom,apm-post-halt-delay", &delay_counter);
		if (rc < 0) {
			dev_err(dev, "apm-post-halt-delay read failed, rc = %d",
				rc);
			return rc;
		}

		val &= ~APM_CTL_POST_HALT_DLY_MASK;
		val |= (delay_counter << APM_CTL_POST_HALT_DLY_SHIFT)
			& APM_CTL_POST_HALT_DLY_MASK;
	}

	if (of_find_property(dev->of_node, "qcom,apm-halt-clk-delay", NULL)) {
		rc = of_property_read_u32(dev->of_node,
				"qcom,apm-halt-clk-delay", &delay_counter);
		if (rc < 0) {
			dev_err(dev, "apm-halt-clk-delay read failed, rc = %d",
				rc);
			return rc;
		}

		val &= ~APM_CTL_HALT_CLK_DLY_MASK;
		val |= (delay_counter << APM_CTL_HALT_CLK_DLY_SHIFT)
			& APM_CTL_HALT_CLK_DLY_MASK;
	}

	if (of_find_property(dev->of_node, "qcom,apm-resume-clk-delay", NULL)) {
		rc = of_property_read_u32(dev->of_node,
				"qcom,apm-resume-clk-delay", &delay_counter);
		if (rc < 0) {
			dev_err(dev, "apm-resume-clk-delay read failed, rc = %d",
				rc);
			return rc;
		}

		val &= ~APM_CTL_RESUME_CLK_DLY_MASK;
		val |= (delay_counter << APM_CTL_RESUME_CLK_DLY_SHIFT)
			& APM_CTL_RESUME_CLK_DLY_MASK;
	}

	if (of_find_property(dev->of_node, "qcom,apm-sel-switch-delay", NULL)) {
		rc = of_property_read_u32(dev->of_node,
				"qcom,apm-sel-switch-delay", &delay_counter);
		if (rc < 0) {
			dev_err(dev, "apm-sel-switch-delay read failed, rc = %d",
				rc);
			return rc;
		}

		val &= ~APM_CTL_SEL_SWITCH_DLY_MASK;
		val |= (delay_counter << APM_CTL_SEL_SWITCH_DLY_SHIFT)
			& APM_CTL_SEL_SWITCH_DLY_MASK;
	}

	if (val != regval) {
		writel_relaxed(val, ctrl->reg_base + MSM8953_APM_DLY_CNTR);
		/* make sure write completes before return */
		mb();
	}

	return rc;
}

static int msm8996_apm_switch_to_mx(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int i, timeout = MSM_APM_SWITCH_TIMEOUT_US;
	u32 regval;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&ctrl_dev->lock, flags);

	/* Perform revision-specific programming steps */
	if (ctrl_dev->version < HMSS_VERSION_1P2) {
		/* Clear SPM events */
		for (i = 0; i < SPM_EVENT_NUM; i++)
			writel_relaxed(SPM_EVENT_CLEAR_VAL,
				       ctrl_dev->apcs_spm_events_addr[i]);

		udelay(SPM_WAKEUP_DELAY_US);

		/* Switch APC/CBF to GPLL0 clock */
		writel_relaxed(APCS_GFMUXA_SEL_VAL,
			       ctrl_dev->apcs_csr_base + APCS_SPARE);
		ndelay(200);
		writel_relaxed(MSM_APM_OVERRIDE_SEL_VAL,
			       ctrl_dev->apc0_pll_ctl_addr);
		ndelay(200);
		writel_relaxed(MSM_APM_OVERRIDE_SEL_VAL,
			       ctrl_dev->apc1_pll_ctl_addr);

		/* Ensure writes complete before proceeding */
		mb();
	}

	/* Switch arrays to MX supply and wait for its completion */
	writel_relaxed(MSM_APM_MX_MODE_VAL, ctrl_dev->reg_base +
		       APCC_APM_MODE);

	/* Ensure write above completes before delaying */
	mb();

	while (timeout > 0) {
		regval = readl_relaxed(ctrl_dev->reg_base + APCC_APM_CTL_STS);
		if ((regval & MSM_APM_CTL_STS_MASK) ==
		    MSM_APM_MX_DONE_VAL)
			break;

		udelay(1);
		timeout--;
	}

	if (timeout == 0) {
		ret = -ETIMEDOUT;
		dev_err(ctrl_dev->dev, "APCC to MX APM switch timed out. APCC_APM_CTL_STS=0x%x\n",
			regval);
	}

	/* Perform revision-specific programming steps */
	if (ctrl_dev->version < HMSS_VERSION_1P2) {
		/* Switch APC/CBF clocks to original source */
		writel_relaxed(APCS_GFMUXA_DESEL_VAL,
			       ctrl_dev->apcs_csr_base + APCS_SPARE);
		ndelay(200);
		writel_relaxed(MSM_APM_SEC_CLK_SEL_VAL,
			       ctrl_dev->apc0_pll_ctl_addr);
		ndelay(200);
		writel_relaxed(MSM_APM_SEC_CLK_SEL_VAL,
			       ctrl_dev->apc1_pll_ctl_addr);

		/* Complete clock source switch before SPM event sequence */
		mb();

		/* Set SPM events */
		for (i = 0; i < SPM_EVENT_NUM; i++)
			writel_relaxed(SPM_EVENT_SET_VAL,
				       ctrl_dev->apcs_spm_events_addr[i]);
	}

	if (!ret) {
		ctrl_dev->supply = MSM_APM_SUPPLY_MX;
		dev_dbg(ctrl_dev->dev, "APM supply switched to MX\n");
	}

	spin_unlock_irqrestore(&ctrl_dev->lock, flags);

	return ret;
}

static int msm8996_apm_switch_to_apcc(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int i, timeout = MSM_APM_SWITCH_TIMEOUT_US;
	u32 regval;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&ctrl_dev->lock, flags);

	/* Perform revision-specific programming steps */
	if (ctrl_dev->version < HMSS_VERSION_1P2) {
		/* Clear SPM events */
		for (i = 0; i < SPM_EVENT_NUM; i++)
			writel_relaxed(SPM_EVENT_CLEAR_VAL,
				       ctrl_dev->apcs_spm_events_addr[i]);

		udelay(SPM_WAKEUP_DELAY_US);

		/* Switch APC/CBF to GPLL0 clock */
		writel_relaxed(APCS_GFMUXA_SEL_VAL,
			       ctrl_dev->apcs_csr_base + APCS_SPARE);
		ndelay(200);
		writel_relaxed(MSM_APM_OVERRIDE_SEL_VAL,
			       ctrl_dev->apc0_pll_ctl_addr);
		ndelay(200);
		writel_relaxed(MSM_APM_OVERRIDE_SEL_VAL,
			       ctrl_dev->apc1_pll_ctl_addr);

		/* Ensure previous writes complete before proceeding */
		mb();
	}

	/* Switch arrays to APCC supply and wait for its completion */
	writel_relaxed(MSM_APM_APCC_MODE_VAL, ctrl_dev->reg_base +
		       APCC_APM_MODE);

	/* Ensure write above completes before delaying */
	mb();

	while (timeout > 0) {
		regval = readl_relaxed(ctrl_dev->reg_base + APCC_APM_CTL_STS);
		if ((regval & MSM_APM_CTL_STS_MASK) ==
		    MSM_APM_APCC_DONE_VAL)
			break;

		udelay(1);
		timeout--;
	}

	if (timeout == 0) {
		ret = -ETIMEDOUT;
		dev_err(ctrl_dev->dev, "MX to APCC APM switch timed out. APCC_APM_CTL_STS=0x%x\n",
			regval);
	}

	/* Perform revision-specific programming steps */
	if (ctrl_dev->version < HMSS_VERSION_1P2) {
		/* Set SPM events */
		for (i = 0; i < SPM_EVENT_NUM; i++)
			writel_relaxed(SPM_EVENT_SET_VAL,
				       ctrl_dev->apcs_spm_events_addr[i]);

		/* Complete SPM event sequence before clock source switch */
		mb();

		/* Switch APC/CBF clocks to original source */
		writel_relaxed(APCS_GFMUXA_DESEL_VAL,
			       ctrl_dev->apcs_csr_base + APCS_SPARE);
		ndelay(200);
		writel_relaxed(MSM_APM_SEC_CLK_SEL_VAL,
			       ctrl_dev->apc0_pll_ctl_addr);
		ndelay(200);
		writel_relaxed(MSM_APM_SEC_CLK_SEL_VAL,
			       ctrl_dev->apc1_pll_ctl_addr);
	}

	if (!ret) {
		ctrl_dev->supply = MSM_APM_SUPPLY_APCC;
		dev_dbg(ctrl_dev->dev, "APM supply switched to APCC\n");
	}

	spin_unlock_irqrestore(&ctrl_dev->lock, flags);

	return ret;
}

/* 8953 register value definitions */
#define MSM8953_APM_MX_MODE_VAL            0x00
#define MSM8953_APM_APCC_MODE_VAL          0x02
#define MSM8953_APM_MX_DONE_VAL            0x00
#define MSM8953_APM_APCC_DONE_VAL          0x03

/* 8953 register offset definitions */
#define MSM8953_APCC_APM_MODE              0x000002a8
#define MSM8953_APCC_APM_CTL_STS           0x000002b0

/* 8953 constants */
#define MSM8953_APM_SWITCH_TIMEOUT_US      500

/* Register bit mask definitions */
#define MSM8953_APM_CTL_STS_MASK           0x1f

static int msm8953_apm_switch_to_mx(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int timeout = MSM8953_APM_SWITCH_TIMEOUT_US;
	u32 regval;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&ctrl_dev->lock, flags);

	/* Switch arrays to MX supply and wait for its completion */
	writel_relaxed(MSM8953_APM_MX_MODE_VAL, ctrl_dev->reg_base +
		       MSM8953_APCC_APM_MODE);

	/* Ensure write above completes before delaying */
	mb();

	while (timeout > 0) {
		regval = readl_relaxed(ctrl_dev->reg_base +
					MSM8953_APCC_APM_CTL_STS);
		if ((regval & MSM8953_APM_CTL_STS_MASK) ==
				MSM8953_APM_MX_DONE_VAL)
			break;

		udelay(1);
		timeout--;
	}

	if (timeout == 0) {
		ret = -ETIMEDOUT;
		dev_err(ctrl_dev->dev, "APCC to MX APM switch timed out. APCC_APM_CTL_STS=0x%x\n",
			regval);
	} else {
		ctrl_dev->supply = MSM_APM_SUPPLY_MX;
		dev_dbg(ctrl_dev->dev, "APM supply switched to MX\n");
	}

	spin_unlock_irqrestore(&ctrl_dev->lock, flags);

	return ret;
}

static int msm8953_apm_switch_to_apcc(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int timeout = MSM8953_APM_SWITCH_TIMEOUT_US;
	u32 regval;
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&ctrl_dev->lock, flags);

	/* Switch arrays to APCC supply and wait for its completion */
	writel_relaxed(MSM8953_APM_APCC_MODE_VAL, ctrl_dev->reg_base +
		       MSM8953_APCC_APM_MODE);

	/* Ensure write above completes before delaying */
	mb();

	while (timeout > 0) {
		regval = readl_relaxed(ctrl_dev->reg_base +
					MSM8953_APCC_APM_CTL_STS);
		if ((regval & MSM8953_APM_CTL_STS_MASK) ==
				MSM8953_APM_APCC_DONE_VAL)
			break;

		udelay(1);
		timeout--;
	}

	if (timeout == 0) {
		ret = -ETIMEDOUT;
		dev_err(ctrl_dev->dev, "MX to APCC APM switch timed out. APCC_APM_CTL_STS=0x%x\n",
			regval);
	} else {
		ctrl_dev->supply = MSM_APM_SUPPLY_APCC;
		dev_dbg(ctrl_dev->dev, "APM supply switched to APCC\n");
	}

	spin_unlock_irqrestore(&ctrl_dev->lock, flags);

	return ret;
}

static int msm_apm_switch_to_mx(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int ret = 0;

	switch (ctrl_dev->msm_id) {
	case MSM8996_ID:
		ret = msm8996_apm_switch_to_mx(ctrl_dev);
		break;
	case MSM8953_ID:
	case IPQ807x_ID:
		ret = msm8953_apm_switch_to_mx(ctrl_dev);
		break;
	}

	return ret;
}

static int msm_apm_switch_to_apcc(struct msm_apm_ctrl_dev *ctrl_dev)
{
	int ret = 0;

	switch (ctrl_dev->msm_id) {
	case MSM8996_ID:
		ret = msm8996_apm_switch_to_apcc(ctrl_dev);
		break;
	case MSM8953_ID:
	case IPQ807x_ID:
		ret = msm8953_apm_switch_to_apcc(ctrl_dev);
		break;
	}

	return ret;
}

/**
 * msm_apm_get_supply() - Returns the supply that is currently
 *			powering the memory arrays
 * @ctrl_dev:                   Pointer to an MSM APM controller device
 *
 * Returns the supply currently selected by the APM.
 */
int msm_apm_get_supply(struct msm_apm_ctrl_dev *ctrl_dev)
{
	return ctrl_dev->supply;
}
EXPORT_SYMBOL(msm_apm_get_supply);

/**
 * msm_apm_set_supply() - Perform the necessary steps to switch the voltage
 *                        source of the memory arrays to a given supply
 * @ctrl_dev:                   Pointer to an MSM APM controller device
 * @supply:                     Power rail to use as supply for the memory
 *                              arrays
 *
 * Returns 0 on success, -ETIMEDOUT on APM switch timeout, or -EPERM if
 * the supply is not supported.
 */
int msm_apm_set_supply(struct msm_apm_ctrl_dev *ctrl_dev,
		       enum msm_apm_supply supply)
{
	int ret;

	switch (supply) {
	case MSM_APM_SUPPLY_APCC:
		ret = msm_apm_switch_to_apcc(ctrl_dev);
		break;
	case MSM_APM_SUPPLY_MX:
		ret = msm_apm_switch_to_mx(ctrl_dev);
		break;
	default:
		ret = -EPERM;
		break;
	}

	return ret;
}
EXPORT_SYMBOL(msm_apm_set_supply);

/**
 * msm_apm_ctrl_dev_get() - get a handle to the MSM APM controller linked to
 *                          the device in device tree
 * @dev:                    Pointer to the device
 *
 * The device must specify "qcom,apm-ctrl" property in its device tree
 * node which points to an MSM APM controller device node.
 *
 * Returns an MSM APM controller handle if successful or ERR_PTR on any error.
 * If the APM controller device hasn't probed yet, ERR_PTR(-EPROBE_DEFER) is
 * returned.
 */
struct msm_apm_ctrl_dev *msm_apm_ctrl_dev_get(struct device *dev)
{
	struct msm_apm_ctrl_dev *ctrl_dev = NULL;
	struct msm_apm_ctrl_dev *dev_found = ERR_PTR(-EPROBE_DEFER);
	struct device_node *ctrl_node;

	if (!dev || !dev->of_node) {
		pr_err("Invalid device node\n");
		return ERR_PTR(-EINVAL);
	}

	ctrl_node = of_parse_phandle(dev->of_node, "qcom,apm-ctrl", 0);
	if (!ctrl_node) {
		pr_err("Could not find qcom,apm-ctrl property in %s\n",
		       dev->of_node->full_name);
		return ERR_PTR(-ENXIO);
	}

	mutex_lock(&apm_ctrl_list_mutex);
	list_for_each_entry(ctrl_dev, &apm_ctrl_list, list) {
		if (ctrl_dev->dev && ctrl_dev->dev->of_node == ctrl_node) {
			dev_found = ctrl_dev;
			break;
		}
	}
	mutex_unlock(&apm_ctrl_list_mutex);

	of_node_put(ctrl_node);
	return dev_found;
}
EXPORT_SYMBOL(msm_apm_ctrl_dev_get);

#if defined(CONFIG_DEBUG_FS)

static int apm_supply_dbg_open(struct inode *inode, struct file *filep)
{
	filep->private_data = inode->i_private;

	return 0;
}

static ssize_t apm_supply_dbg_read(struct file *filep, char __user *ubuf,
				   size_t count, loff_t *ppos)
{
	struct msm_apm_ctrl_dev *ctrl_dev = filep->private_data;
	char buf[10];
	int len;

	if (!ctrl_dev) {
		pr_err("invalid apm ctrl handle\n");
		return -ENODEV;
	}

	if (ctrl_dev->supply == MSM_APM_SUPPLY_APCC)
		len = snprintf(buf, sizeof(buf), "APCC\n");
	else if (ctrl_dev->supply == MSM_APM_SUPPLY_MX)
		len = snprintf(buf, sizeof(buf), "MX\n");
	else
		len = snprintf(buf, sizeof(buf), "ERR\n");

	return simple_read_from_buffer(ubuf, count, ppos, buf, len);
}

static const struct file_operations apm_supply_fops = {
	.open = apm_supply_dbg_open,
	.read = apm_supply_dbg_read,
};

static void apm_debugfs_base_init(void)
{
	apm_debugfs_base = debugfs_create_dir("msm-apm", NULL);

	if (IS_ERR_OR_NULL(apm_debugfs_base))
		pr_err("msm-apm debugfs base directory creation failed\n");
}

static void apm_debugfs_init(struct msm_apm_ctrl_dev *ctrl_dev)
{
	struct dentry *temp;

	if (IS_ERR_OR_NULL(apm_debugfs_base)) {
		pr_err("Base directory missing, cannot create apm debugfs nodes\n");
		return;
	}

	ctrl_dev->debugfs = debugfs_create_dir(dev_name(ctrl_dev->dev),
					       apm_debugfs_base);
	if (IS_ERR_OR_NULL(ctrl_dev->debugfs)) {
		pr_err("%s debugfs directory creation failed\n",
		       dev_name(ctrl_dev->dev));
		return;
	}

	temp = debugfs_create_file("supply", S_IRUGO, ctrl_dev->debugfs,
				   ctrl_dev, &apm_supply_fops);
	if (IS_ERR_OR_NULL(temp)) {
		pr_err("supply mode creation failed\n");
		return;
	}
}

static void apm_debugfs_deinit(struct msm_apm_ctrl_dev *ctrl_dev)
{
	if (!IS_ERR_OR_NULL(ctrl_dev->debugfs))
		debugfs_remove_recursive(ctrl_dev->debugfs);
}

static void apm_debugfs_base_remove(void)
{
	debugfs_remove_recursive(apm_debugfs_base);
}
#else

static void apm_debugfs_base_init(void)
{}

static void apm_debugfs_init(struct msm_apm_ctrl_dev *ctrl_dev)
{}

static void apm_debugfs_deinit(struct msm_apm_ctrl_dev *ctrl_dev)
{}

static void apm_debugfs_base_remove(void)
{}

#endif

static struct of_device_id msm_apm_match_table[] = {
	{
		.compatible = "qcom,msm-apm",
		.data = (void *)(uintptr_t)MSM8996_ID,
	},
	{
		.compatible = "qcom,msm8953-apm",
		.data = (void *)(uintptr_t)MSM8953_ID,
	},
	{
		.compatible = "qcom,ipq807x-apm",
		.data = (void *)(uintptr_t)IPQ807x_ID,
	},
	{}
};

static int msm_apm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct msm_apm_ctrl_dev *ctrl;
	const struct of_device_id *match;
	int ret = 0;

	dev_dbg(dev, "probing MSM Array Power Mux driver\n");

	if (!dev->of_node) {
		dev_err(dev, "Device tree node is missing\n");
		return -ENODEV;
	}

	match = of_match_device(msm_apm_match_table, dev);
	if (!match)
		return -ENODEV;

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl) {
		dev_err(dev, "MSM APM controller memory allocation failed\n");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&ctrl->list);
	spin_lock_init(&ctrl->lock);
	ctrl->dev = dev;
	ctrl->msm_id = (uintptr_t)match->data;
	platform_set_drvdata(pdev, ctrl);

	switch (ctrl->msm_id) {
	case MSM8996_ID:
		ret = msm_apm_ctrl_devm_ioremap(pdev, ctrl);
		if (ret) {
			dev_err(dev, "Failed to add APM controller device\n");
			return ret;
		}
		break;
	case MSM8953_ID:
	case IPQ807x_ID:
		ret = msm8953_apm_ctrl_init(pdev, ctrl);
		if (ret) {
			dev_err(dev, "Failed to initialize APM controller device: ret=%d\n",
				ret);
			return ret;
		}
		break;
	default:
		dev_err(dev, "unable to add APM controller device for msm_id:%d\n",
			ctrl->msm_id);
		return -ENODEV;
	}

	apm_debugfs_init(ctrl);
	mutex_lock(&apm_ctrl_list_mutex);
	list_add_tail(&ctrl->list, &apm_ctrl_list);
	mutex_unlock(&apm_ctrl_list_mutex);

	dev_dbg(dev, "MSM Array Power Mux driver probe successful");

	return ret;
}

static int msm_apm_remove(struct platform_device *pdev)
{
	struct msm_apm_ctrl_dev *ctrl_dev;

	ctrl_dev = platform_get_drvdata(pdev);
	if (ctrl_dev) {
		mutex_lock(&apm_ctrl_list_mutex);
		list_del(&ctrl_dev->list);
		mutex_unlock(&apm_ctrl_list_mutex);
		apm_debugfs_deinit(ctrl_dev);
	}

	return 0;
}

static struct platform_driver msm_apm_driver = {
	.driver		= {
		.name		= MSM_APM_DRIVER_NAME,
		.of_match_table	= msm_apm_match_table,
		.owner		= THIS_MODULE,
	},
	.probe		= msm_apm_probe,
	.remove		= msm_apm_remove,
};

static int __init msm_apm_init(void)
{
	apm_debugfs_base_init();
	return platform_driver_register(&msm_apm_driver);
}

static void __exit msm_apm_exit(void)
{
	platform_driver_unregister(&msm_apm_driver);
	apm_debugfs_base_remove();
}

arch_initcall(msm_apm_init);
module_exit(msm_apm_exit);

MODULE_DESCRIPTION("MSM Array Power Mux driver");
MODULE_LICENSE("GPL v2");
