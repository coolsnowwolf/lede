// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SPI core controller platform driver.
 *
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/property.h>
#include <linux/acpi.h>

#include "spi-phytium.h"

#define DRIVER_NAME "phytium_spi"

static int phytium_spi_probe(struct platform_device *pdev)
{
	struct phytium_spi *fts;
	struct resource *mem;
	int ret;
	int num_cs;
	int cs_gpio;
	int global_cs;
	int i;

	fts = devm_kzalloc(&pdev->dev, sizeof(struct phytium_spi),
			GFP_KERNEL);
	if (!fts)
		return -ENOMEM;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "no mem resource?\n");
		return -EINVAL;
	}

	fts->paddr = mem->start;
	fts->regs = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(fts->regs)) {
		dev_err(&pdev->dev, "SPI region map failed\n");
		return PTR_ERR(fts->regs);
	}

	fts->irq = platform_get_irq(pdev, 0);
	if (fts->irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		return fts->irq; /* -ENXIO */
	}

	if (pdev->dev.of_node) {
		fts->clk = devm_clk_get(&pdev->dev, NULL);

		if (IS_ERR(fts->clk))
			return PTR_ERR(fts->clk);
		ret = clk_prepare_enable(fts->clk);
		if (ret)
			return ret;

		fts->max_freq = clk_get_rate(fts->clk);
	} else if (has_acpi_companion(&pdev->dev)) {
		fts->max_freq = 48000000;
	}

	fts->bus_num = pdev->id;
	device_property_read_u32(&pdev->dev, "reg-io-width", &fts->reg_io_width);

	num_cs = 4;
	device_property_read_u32(&pdev->dev, "num-cs", &num_cs);
	fts->num_cs = num_cs;

	if (pdev->dev.of_node) {
		int i;

		for (i = 0; i < fts->num_cs; i++) {
			cs_gpio = of_get_named_gpio(pdev->dev.of_node,
					"cs-gpios", i);

			if (cs_gpio == -EPROBE_DEFER) {
				ret = cs_gpio;
				goto out;
			}

			if (gpio_is_valid(cs_gpio)) {
				ret = devm_gpio_request(&pdev->dev, cs_gpio,
						dev_name(&pdev->dev));
				if (ret)
					goto out;
			}
		}
	} else if(has_acpi_companion(&pdev->dev)) {
		int n;
		int *cs;
		struct gpio_desc *gpiod;

		n = gpiod_count(&pdev->dev, "cs");

		cs = devm_kcalloc(&pdev->dev, n, sizeof(int), GFP_KERNEL);
		fts->cs = cs;

		for (i = 0; i < n; i++) {
			gpiod = devm_gpiod_get_index_optional(&pdev->dev, "cs", i,
							      GPIOD_OUT_LOW);

			if (IS_ERR(gpiod)) {
				ret = PTR_ERR(gpiod);
				goto out;
			}

			cs_gpio = desc_to_gpio(gpiod);
			cs[i] = cs_gpio;
		}
	}

	device_property_read_u32(&pdev->dev, "global-cs", &global_cs);
	fts->global_cs = global_cs;

	/* check is use dma transfer */
	if ((device_property_read_string_array(&pdev->dev, "dma-names",
					NULL, 0) > 0) &&
		device_property_present(&pdev->dev, "dmas")) {
		fts->dma_en = true;
		phytium_spi_dmaops_set(fts);
	}

	ret = phytium_spi_add_host(&pdev->dev, fts);
	if (ret)
		goto out;

	platform_set_drvdata(pdev, fts);
	return 0;

out:
	clk_disable_unprepare(fts->clk);
	return ret;
}

static int phytium_spi_remove(struct platform_device *pdev)
{
	struct phytium_spi *fts = platform_get_drvdata(pdev);

	phytium_spi_remove_host(fts);
	clk_disable_unprepare(fts->clk);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int spi_suspend(struct device *dev)
{
	struct phytium_spi *fts = dev_get_drvdata(dev);

	return phytium_spi_suspend_host(fts);
}

static int spi_resume(struct device *dev)
{
	struct phytium_spi *fts = dev_get_drvdata(dev);

	return phytium_spi_resume_host(fts);
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_spi_pm_ops, spi_suspend, spi_resume);

static const struct of_device_id phytium_spi_of_match[] = {
	{ .compatible = "phytium,spi", .data = (void *)0 },
	{ /* end of table */}
};
MODULE_DEVICE_TABLE(of, phytium_spi_of_match);

static const struct acpi_device_id phytium_spi_acpi_match[] = {
	{"PHYT000E", 0},
	{}
};
MODULE_DEVICE_TABLE(acpi, phytium_spi_acpi_match);

static struct platform_driver phytium_spi_driver = {
	.probe		= phytium_spi_probe,
	.remove		= phytium_spi_remove,
	.driver		= {
		.name	= DRIVER_NAME,
		.of_match_table = of_match_ptr(phytium_spi_of_match),
		.acpi_match_table = ACPI_PTR(phytium_spi_acpi_match),
		.pm = &phytium_spi_pm_ops,
	},
};
module_platform_driver(phytium_spi_driver);

MODULE_AUTHOR("Yiqun Zhang <zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("Platform Driver for Phytium SPI controller core");
MODULE_LICENSE("GPL v2");
