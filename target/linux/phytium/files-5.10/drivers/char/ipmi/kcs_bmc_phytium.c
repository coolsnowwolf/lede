// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 *
 * Derived from drivers/char/ipmi/kcs_bmc_aspeed.c
 *   Copyright (c) 2015-2018, Intel Corporation.
 */

#define pr_fmt(fmt) "phytium-kcs-bmc: " fmt

#include <linux/atomic.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/regmap.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/timer.h>

#include "kcs_bmc.h"

#define DEVICE_NAME     "phytium-kcs-bmc"

#define KCS_CHANNEL_MAX     4

/* mapped to lpc-bmc@0 IO space */
#define LPC_HICR0		0x000
#define		LPC_HICR0_LPC3E		BIT(7)
#define		LPC_HICR0_LPC2E		BIT(6)
#define		LPC_HICR0_LPC1E		BIT(5)
#define		LPC_HICR0_SDWNE		BIT(3) /* 0:disable, 1:enable for HICR1_SDWNB */
#define		LPC_HICR0_PMEE		BIT(2) /* 0:disable, 1:enable for HICR1_PMEB */
#define LPC_HICR1		0x004
#define		LPC_HICR1_LPCBSY	BIT(7) /* 0:idle, 1:trans busy */
#define		LPC_HICR1_IRQBSY	BIT(5) /* 0:idle, 1:trans data */
#define		LPC_HICR1_LPCSWRST	BIT(4) /* 0:normal, 1:reset */
#define		LPC_HICR1_SDWNB		BIT(3) /* 0:normal, 1:software shutdown */
#define		LPC_HICR1_PMEB		BIT(2) /* 0:LPCPD low, 1:LPCPD high */
#define	LPC_HICR2		0x008
#define		LPC_LPCSWRST_ERRIRQ	BIT(6) /* 0:normal, 1:reset irq */
#define		LPC_SDWN_ERRIRQ		BIT(5) /* 0:normal, 1:lpcpd irq */
#define		LPC_ABRT_ERRIRQ         BIT(4) /* 0:normal, 1:lframe low when busy*/
#define		LPC_HICR2_IBFIF3	BIT(3) /* 0:normal, 1:enable irq 3*/
#define		LPC_HICR2_IBFIF2	BIT(2)
#define		LPC_HICR2_IBFIF1	BIT(1)
/* 0:normal,1:enable err irq-reset,power down,abort */
#define		LPC_HICR2_ERRIE		BIT(0)
#define LPC_HICR3		0x00C
#define		LPC_LFRAME_STATUS	BIT(7) /* R */
#define		LPC_SERIRQ_STATUS	BIT(5) /* R */
#define		LPC_LREST_STATUS	BIT(4) /* R */
#define		LPC_LPCPD_STATUS	BIT(3) /* R */
#define		LPC_PME_STATUS		BIT(2) /* R */
#define LPC_HICR4		0x010
#define		LPC_HICR4_LADR12AS	BIT(7)
#define		LPC_HICR4_KCSENBL	BIT(2)
#define		LPC_BTENABLE		BIT(0) /* share bt channel enable, 0:disable, 1:enable */
#define LPC_LADR3H           0x014
#define LPC_LADR3L           0x018
#define LPC_LADR12H          0x01C
#define LPC_LADR12L          0x020
#define LPC_IDR1             0x024
#define LPC_IDR2             0x028
#define LPC_IDR3             0x02C
#define LPC_ODR1             0x030
#define LPC_ODR2             0x034
#define LPC_ODR3             0x038
#define LPC_STR1             0x03C
#define LPC_STR2             0x040
#define LPC_STR3             0x044

#define LPC_HICRB            0x80
#define		LPC_HICRB_IBFIF4         BIT(1)
#define		LPC_HICRB_LPC4E          BIT(0)
#define LPC_LADR4            0x88
#define LPC_IDR4             0x8c
#define LPC_ODR4             0x90
#define LPC_STR4             0x94

struct phytium_kcs_bmc {
	struct regmap *map;
};

static u8 phytium_kcs_inb(struct kcs_bmc *kcs_bmc, u32 reg)
{
	struct phytium_kcs_bmc *priv = kcs_bmc_priv(kcs_bmc);
	u32 val = 0;
	int rc;

	rc = regmap_read(priv->map, reg, &val);
	WARN(rc != 0, "regmap_read() failed: %d\n", rc);

	return rc == 0 ? (u8) val : 0;
}

static void phytium_kcs_outb(struct kcs_bmc *kcs_bmc, u32 reg, u8 data)
{
	struct phytium_kcs_bmc *priv = kcs_bmc_priv(kcs_bmc);
	int rc;

	rc = regmap_write(priv->map, reg, data);
	WARN(rc != 0, "regmap_write() failed: %d\n", rc);
}

/*
 * Background:
 *   we note D for Data, and C for Cmd/Status, default rules are
 *     A. KCS1 / KCS2 ( D / C:X / X+4 )
 *        D / C : CA0h / CA4h
 *        D / C : CA8h / CACh
 *     B. KCS3 ( D / C:XX2h / XX3h )
 *        D / C : CA2h / CA3h
 *        D / C : CB2h / CB3h -use
 *     C. KCS4
 *        D / C : CA4h / CA5h
 *        D / C : CB0h / CB1h -use
 */
static void phytium_kcs_set_address(struct kcs_bmc *kcs_bmc, u16 addr)
{
	struct phytium_kcs_bmc *priv = kcs_bmc_priv(kcs_bmc);

	switch (kcs_bmc->channel) {
	case 1:
		regmap_update_bits(priv->map, LPC_HICR4, LPC_HICR4_LADR12AS, 0);
		regmap_write(priv->map, LPC_LADR12H, addr >> 8);
		regmap_write(priv->map, LPC_LADR12L, addr & 0xFF);
		break;
	case 2:
		regmap_update_bits(priv->map, LPC_HICR4,
				   LPC_HICR4_LADR12AS, LPC_HICR4_LADR12AS);
		regmap_write(priv->map, LPC_LADR12H, addr >> 8);
		regmap_write(priv->map, LPC_LADR12L, addr & 0xFF);
		break;
	case 3:
		regmap_write(priv->map, LPC_LADR3H, addr >> 8);
		regmap_write(priv->map, LPC_LADR3L, addr & 0xFF);
		break;
	case 4:
		regmap_write(priv->map, LPC_LADR4, ((addr + 1) << 16) |
			addr);
		break;
	default:
		break;
	}
}

static void phytium_kcs_enable_channel(struct kcs_bmc *kcs_bmc, bool enable)
{
	struct phytium_kcs_bmc *priv = kcs_bmc_priv(kcs_bmc);

	switch (kcs_bmc->channel) {
	case 1:
		if (enable) {
			regmap_update_bits(priv->map, LPC_HICR2,
					   LPC_HICR2_IBFIF1, LPC_HICR2_IBFIF1);
			regmap_update_bits(priv->map, LPC_HICR0,
					   LPC_HICR0_LPC1E, LPC_HICR0_LPC1E);
		} else {
			regmap_update_bits(priv->map, LPC_HICR0,
					   LPC_HICR0_LPC1E, 0);
			regmap_update_bits(priv->map, LPC_HICR2,
					   LPC_HICR2_IBFIF1, 0);
		}
		break;
	case 2:
		if (enable) {
			regmap_update_bits(priv->map, LPC_HICR2,
					LPC_HICR2_IBFIF2, LPC_HICR2_IBFIF2);
			regmap_update_bits(priv->map, LPC_HICR0,
					LPC_HICR0_LPC2E, LPC_HICR0_LPC2E);
		} else {
			regmap_update_bits(priv->map, LPC_HICR0,
					LPC_HICR0_LPC2E, 0);
			regmap_update_bits(priv->map, LPC_HICR2,
					LPC_HICR2_IBFIF2, 0);
		}
		break;
	case 3:
		if (enable) {
			regmap_update_bits(priv->map, LPC_HICR2,
					   LPC_HICR2_IBFIF3, LPC_HICR2_IBFIF3);
			regmap_update_bits(priv->map, LPC_HICR0,
					   LPC_HICR0_LPC3E, LPC_HICR0_LPC3E);
			regmap_update_bits(priv->map, LPC_HICR4,
					   LPC_HICR4_KCSENBL, LPC_HICR4_KCSENBL);
		} else {
			regmap_update_bits(priv->map, LPC_HICR0,
					   LPC_HICR0_LPC3E, 0);
			regmap_update_bits(priv->map, LPC_HICR4,
					   LPC_HICR4_KCSENBL, 0);
			regmap_update_bits(priv->map, LPC_HICR2,
					   LPC_HICR2_IBFIF3, 0);
		}
		break;
	case 4:
		if (enable)
			regmap_update_bits(priv->map, LPC_HICRB,
					   LPC_HICRB_IBFIF4 | LPC_HICRB_LPC4E,
					   LPC_HICRB_IBFIF4 | LPC_HICRB_LPC4E);
		else
			regmap_update_bits(priv->map, LPC_HICRB,
					   LPC_HICRB_IBFIF4 | LPC_HICRB_LPC4E,
					   0);
		break;
	default:
		break;
	}
}

static irqreturn_t phytium_kcs_irq(int irq, void *arg)
{
	struct kcs_bmc *kcs_bmc = arg;

	if (!kcs_bmc_handle_event(kcs_bmc))
		return IRQ_HANDLED;

	return IRQ_NONE;
}

static int phytium_kcs_config_irq(struct kcs_bmc *kcs_bmc, struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	return devm_request_irq(dev, irq, phytium_kcs_irq, IRQF_SHARED,
				dev_name(dev), kcs_bmc);
}

static const struct kcs_ioreg phytium_kcs_bmc_ioregs[KCS_CHANNEL_MAX] = {
	{ .idr = LPC_IDR1, .odr = LPC_ODR1, .str = LPC_STR1 },
	{ .idr = LPC_IDR2, .odr = LPC_ODR2, .str = LPC_STR2 },
	{ .idr = LPC_IDR3, .odr = LPC_ODR3, .str = LPC_STR3 },
	{ .idr = LPC_IDR4, .odr = LPC_ODR4, .str = LPC_STR4 },
};

static int phytium_kcs_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct phytium_kcs_bmc *priv;
	struct kcs_bmc *kcs_bmc;
	u32 chan, addr;
	int rc;

	rc = of_property_read_u32(dev->of_node, "kcs_chan", &chan);
	if ((rc != 0) || (chan == 0 || chan > KCS_CHANNEL_MAX)) {
		dev_err(dev, "no valid 'kcs_chan' configured\n");
		return -ENODEV;
	}

	rc = of_property_read_u32(dev->of_node, "kcs_addr", &addr);
	if (rc) {
		dev_err(dev, "no valid 'kcs_addr' configured\n");
		return -ENODEV;
	}

	kcs_bmc = kcs_bmc_alloc(dev, sizeof(*priv), chan);
	if (!kcs_bmc)
		return -ENOMEM;

	priv = kcs_bmc_priv(kcs_bmc);
	priv->map = syscon_node_to_regmap(dev->parent->of_node);
	if (IS_ERR(priv->map)) {
		dev_err(dev, "Couldn't get regmap\n");
		return -ENODEV;
	}

	kcs_bmc->ioreg = phytium_kcs_bmc_ioregs[chan - 1];
	kcs_bmc->io_inputb = phytium_kcs_inb;
	kcs_bmc->io_outputb = phytium_kcs_outb;

	dev_set_drvdata(dev, kcs_bmc);

	phytium_kcs_set_address(kcs_bmc, addr);
	phytium_kcs_enable_channel(kcs_bmc, true);
	rc = phytium_kcs_config_irq(kcs_bmc, pdev);
	if (rc)
		return rc;

	rc = misc_register(&kcs_bmc->miscdev);
	if (rc) {
		dev_err(dev, "Unable to register device\n");
		return rc;
	}

	pr_info("channel=%u addr=0x%x idr=0x%x odr=0x%x str=0x%x\n",
		chan, addr, kcs_bmc->ioreg.idr, kcs_bmc->ioreg.odr, kcs_bmc->ioreg.str);

	return 0;
}

static int phytium_kcs_remove(struct platform_device *pdev)
{
	struct kcs_bmc *kcs_bmc = dev_get_drvdata(&pdev->dev);

	misc_deregister(&kcs_bmc->miscdev);

	return 0;
}

static const struct of_device_id phytium_kcs_bmc_match[] = {
	{ .compatible = "phytium,kcs-bmc" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_kcs_bmc_match);

static struct platform_driver phytium_kcs_bmc_driver = {
	.driver = {
		.name           = DEVICE_NAME,
		.of_match_table = phytium_kcs_bmc_match,
	},
	.probe  = phytium_kcs_probe,
	.remove = phytium_kcs_remove,
};
module_platform_driver(phytium_kcs_bmc_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium device interface to the KCS BMC device");
