// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Phytium I2C adapter driver.
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dmi.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/property.h>
#include <linux/reset.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/suspend.h>

#include "i2c-phytium-core.h"

#define DRV_NAME "i2c-phytium-platform"

static u32 i2c_phytium_get_clk_rate_khz(struct phytium_i2c_dev *dev)
{
	return clk_get_rate(dev->clk)/1000;
}

#ifdef CONFIG_ACPI
static void phytium_i2c_acpi_params(struct platform_device *pdev, char method[],
				    u16 *hcnt, u16 *lcnt, u32 *sda_hold)
{
	struct acpi_buffer buf = { ACPI_ALLOCATE_BUFFER };
	acpi_handle handle = ACPI_HANDLE(&pdev->dev);
	union acpi_object *obj;

	if (ACPI_FAILURE(acpi_evaluate_object(handle, method, NULL, &buf)))
		return;

	obj = (union acpi_object *)buf.pointer;
	if (obj->type == ACPI_TYPE_PACKAGE && obj->package.count == 3) {
		const union acpi_object *objs = obj->package.elements;

		*hcnt = (u16)objs[0].integer.value;
		*lcnt = (u16)objs[1].integer.value;
		*sda_hold = (u32)objs[2].integer.value;
	}

	kfree(buf.pointer);
}

static int phytium_i2c_acpi_configure(struct platform_device *pdev)
{
	struct phytium_i2c_dev *dev = platform_get_drvdata(pdev);
	struct i2c_timings *t = &dev->timings;
	u32 ss_ht = 0, fp_ht = 0, hs_ht = 0, fs_ht = 0;
	acpi_handle handle = ACPI_HANDLE(&pdev->dev);
	const struct acpi_device_id *id;
	struct acpi_device *adev;

	dev->adapter.nr = -1;
	dev->tx_fifo_depth = 32;
	dev->rx_fifo_depth = 32;

	/*
	 * Try to get SDA hold time and *CNT values from an ACPI method for
	 * selected speed modes.
	 */
	phytium_i2c_acpi_params(pdev, "SSCN", &dev->ss_hcnt, &dev->ss_lcnt, &ss_ht);
	phytium_i2c_acpi_params(pdev, "FPCN", &dev->fp_hcnt, &dev->fp_lcnt, &fp_ht);
	phytium_i2c_acpi_params(pdev, "HSCN", &dev->hs_hcnt, &dev->hs_lcnt, &hs_ht);
	phytium_i2c_acpi_params(pdev, "FMCN", &dev->fs_hcnt, &dev->fs_lcnt, &fs_ht);

	switch (t->bus_freq_hz) {
	case 100000:
		dev->sda_hold_time = ss_ht;
		break;
	case 1000000:
		dev->sda_hold_time = fp_ht;
		break;
	case 3400000:
		dev->sda_hold_time = hs_ht;
		break;
	case 400000:
	default:
		dev->sda_hold_time = fs_ht;
		break;
	}

	id = acpi_match_device(pdev->dev.driver->acpi_match_table, &pdev->dev);
	if (id && id->driver_data)
		dev->flags |= (u32)id->driver_data;

	if (acpi_bus_get_device(handle, &adev))
		return -ENODEV;

	return 0;
}

static const struct acpi_device_id phytium_i2c_acpi_match[] = {
	{ "PHYT0038", 0 },
	{ }
};
MODULE_DEVICE_TABLE(acpi, phytium_i2c_acpi_match);
#else
static inline int phytium_i2c_acpi_configure(struct platform_device *pdev)
{
	return -ENODEV;
}
#endif

static void i2c_phytium_configure_master(struct phytium_i2c_dev *dev)
{
	struct i2c_timings *t = &dev->timings;

	dev->functionality = I2C_FUNC_10BIT_ADDR | IC_DEFAULT_FUNCTIONALITY;

	dev->master_cfg = IC_CON_MASTER | IC_CON_SLAVE_DISABLE |
			  IC_CON_RESTART_EN;

	dev->mode = PHYTIUM_IC_MASTER;

	switch (t->bus_freq_hz) {
	case 100000:
		dev->master_cfg |= IC_CON_SPEED_STD;
		break;
	case 3400000:
		dev->master_cfg |= IC_CON_SPEED_HIGH;
		break;
	default:
		dev->master_cfg |= IC_CON_SPEED_FAST;
	}
}

static void i2c_phytium_configure_slave(struct phytium_i2c_dev *dev)
{
	dev->functionality = I2C_FUNC_SLAVE | IC_DEFAULT_FUNCTIONALITY;

	dev->slave_cfg = IC_CON_RX_FIFO_FULL_HLD_CTRL |
			 IC_CON_RESTART_EN | IC_CON_STOP_DET_IFADDRESSED;

	dev->mode = PHYTIUM_IC_SLAVE;
}

static int phytium_i2c_plat_probe(struct platform_device *pdev)
{
	struct i2c_adapter *adap;
	struct phytium_i2c_dev *dev;
	struct i2c_timings *t;
	u32 acpi_speed;
	struct resource *mem;
	int irq, ret, i;
	static const int supported_speeds[] = {
		0, 100000, 400000, 1000000, 3400000
	};


	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	dev = devm_kzalloc(&pdev->dev, sizeof(struct phytium_i2c_dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev->base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(dev->base))
		return PTR_ERR(dev->base);

	dev->dev = &pdev->dev;
	dev->irq = irq;
	platform_set_drvdata(pdev, dev);

	dev->rst = devm_reset_control_get_optional_exclusive(&pdev->dev, NULL);
	if (IS_ERR(dev->rst)) {
		if (PTR_ERR(dev->rst) == -EPROBE_DEFER)
			return -EPROBE_DEFER;
	} else {
		reset_control_deassert(dev->rst);
	}

	t = &dev->timings;
	i2c_parse_fw_timings(&pdev->dev, t, false);

	acpi_speed = i2c_acpi_find_bus_speed(&pdev->dev);
	/*
	 * Some DSTDs use a non standard speed, round down to the lowest
	 * standard speed.
	 */
	for (i = 1; i < ARRAY_SIZE(supported_speeds); i++) {
		if (acpi_speed < supported_speeds[i])
			break;
	}
	acpi_speed = supported_speeds[i - 1];

	/*
	 * Find bus speed from the "clock-frequency" device property, ACPI
	 * or by using fast mode if neither is set.
	 */
	if (acpi_speed && t->bus_freq_hz)
		t->bus_freq_hz = min(t->bus_freq_hz, acpi_speed);
	else if (acpi_speed || t->bus_freq_hz)
		t->bus_freq_hz = max(t->bus_freq_hz, acpi_speed);
	else
		t->bus_freq_hz = 400000;

	if (has_acpi_companion(&pdev->dev))
		phytium_i2c_acpi_configure(pdev);

	/*
	 * Only standard mode at 100kHz, fast mode at 400kHz,
	 * fast mode plus at 1MHz and high speed mode at 3.4MHz are supported.
	 */
	if (t->bus_freq_hz != 100000 && t->bus_freq_hz != 400000 &&
	    t->bus_freq_hz != 1000000 && t->bus_freq_hz != 3400000) {
		dev_err(&pdev->dev,
			"%d Hz is unsupported, only 100kHz, 400kHz, 1MHz and 3.4MHz are supported\n",
			t->bus_freq_hz);
		ret = -EINVAL;
		goto exit_reset;
	}

	if (i2c_detect_slave_mode(&pdev->dev))
		i2c_phytium_configure_slave(dev);
	else
		i2c_phytium_configure_master(dev);

	dev->clk = devm_clk_get(&pdev->dev, NULL);
	if (!i2c_phytium_prepare_clk(dev, true)) {
		u64 clk_khz;

		dev->get_clk_rate_khz = i2c_phytium_get_clk_rate_khz;
		clk_khz = dev->get_clk_rate_khz(dev);

		if (!dev->sda_hold_time && t->sda_hold_ns)
			dev->sda_hold_time =
				div_u64(clk_khz * t->sda_hold_ns + 500000, 1000000);
	}

	dev->tx_fifo_depth = 7;
	dev->rx_fifo_depth = 7;
	dev->adapter.nr = pdev->id;

	adap = &dev->adapter;
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_DEPRECATED;
	ACPI_COMPANION_SET(&adap->dev, ACPI_COMPANION(&pdev->dev));
	adap->dev.of_node = pdev->dev.of_node;

	dev_pm_set_driver_flags(&pdev->dev,
				DPM_FLAG_SMART_PREPARE |
				DPM_FLAG_SMART_SUSPEND |
				DPM_FLAG_MAY_SKIP_RESUME);

	/* The code below assumes runtime PM to be disabled. */
	WARN_ON(pm_runtime_enabled(&pdev->dev));

	pm_runtime_set_autosuspend_delay(&pdev->dev, 1000);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_active(&pdev->dev);

	pm_runtime_enable(&pdev->dev);

	if (dev->mode == PHYTIUM_IC_SLAVE)
		ret = i2c_phytium_probe_slave(dev);
	else
		ret = i2c_phytium_probe(dev);

	if (ret)
		goto exit_probe;

	return ret;

exit_probe:
	pm_runtime_disable(dev->dev);
exit_reset:
	if (!IS_ERR_OR_NULL(dev->rst))
		reset_control_assert(dev->rst);
	return ret;
}

static int phytium_i2c_plat_remove(struct platform_device *pdev)
{
	struct phytium_i2c_dev *dev = platform_get_drvdata(pdev);

	pm_runtime_get_sync(&pdev->dev);

	i2c_del_adapter(&dev->adapter);

	dev->disable(dev);

	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(dev->dev);

	if (!IS_ERR_OR_NULL(dev->rst))
		reset_control_assert(dev->rst);

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id phytium_i2c_of_match[] = {
	{ .compatible = "phytium,i2c", },
	{},
};
MODULE_DEVICE_TABLE(of, phytium_i2c_of_match);
#endif

static int __maybe_unused phytium_i2c_plat_suspend(struct device *dev)
{
	struct phytium_i2c_dev *idev = dev_get_drvdata(dev);

	idev->disable(idev);
	i2c_phytium_prepare_clk(idev, false);

	return 0;
}

static int __maybe_unused phytium_i2c_plat_resume(struct device *dev)
{
	struct phytium_i2c_dev *idev = dev_get_drvdata(dev);

	i2c_phytium_prepare_clk(idev, true);

	idev->init(idev);

	return 0;
}

static const struct dev_pm_ops phytium_i2c_dev_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(phytium_i2c_plat_suspend,
				     phytium_i2c_plat_resume)
	SET_RUNTIME_PM_OPS(phytium_i2c_plat_suspend,
			   phytium_i2c_plat_resume, NULL)
};

static struct platform_driver phytium_i2c_driver = {
	.probe = phytium_i2c_plat_probe,
	.remove = phytium_i2c_plat_remove,
	.driver = {
		.name = DRV_NAME,
		.of_match_table = of_match_ptr(phytium_i2c_of_match),
		.acpi_match_table = ACPI_PTR(phytium_i2c_acpi_match),
		.pm = &phytium_i2c_dev_pm_ops,
	},
};
module_platform_driver(phytium_i2c_driver);

MODULE_ALIAS("platform:i2c-phytium");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium I2C bus adapter");
MODULE_LICENSE("GPL");
