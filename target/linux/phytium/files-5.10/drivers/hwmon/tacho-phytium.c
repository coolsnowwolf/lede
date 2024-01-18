// SPDX-License-Identifier: GPL-2.0
/*
 * Hwmon driver for Phytium tachometer.
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/acpi.h>

#define	TIMER_CTRL_REG	0x00
#define		TIMER_CTRL_MODE_SHIFT		0//0:1
#define		TIMER_CTRL_RESET_SHIFT		BIT(2)
#define		TIMER_CTRL_FORCE_SHIFT		BIT(3)
#define		TIMER_CTRL_CAPTURE_EN_SHIFT	BIT(4)
#define		TIMER_CTRL_CAPTURE_CNT_SHIFT	5//5:11
#define		TIMER_CTRL_ANTI_JITTER_SHIFT	18//18:19
#define		TIMER_CTRL_TACHO_MODE_SHIFT	20//20:21
#define		TIMER_CTRL_TIMER_CNT_MODE_SHIFT	BIT(22)
#define		TIMER_CTRL_BIT_SET_SHIFT	24
#define		TIMER_CTRL_CNT_EN_SHIFT		BIT(25)
#define		TIMER_CTRL_CNT_CLR_SHIFT	BIT(26)
#define		TIMER_CTRL_TIMER_MODE_SHIFT	BIT(27)
#define		TIMER_CTRL_PULSE_NUM_SHIFT	28//28:30
#define		TIMER_CTRL_TACHO_EN_SHIFT	BIT(31)
#define	TIMER_TACHO_RES_REG	0x04
#define		TIMER_TACHO_RES_VALID_SHIFT	BIT(31)
#define		TIMER_TACHO_RES_MASK	GENMASK(30, 0)
#define	TIMER_CMP_VALUE_UP_REG	0x08
#define	TIMER_CMP_VALUE_LOW_REG	0x1C
#define	TIMER_CNT_VALUE_UP_REG	0x20
#define	TIMER_CNT_VALUE_LOW_REG	0x24
#define	TIMER_INT_MASK_REG	0x28
#define	TIMER_INT_STAT_REG	0x2C
#define		TIMER_INT_CAPTURE_SHIFT		BIT(5)
#define		TIMER_INT_CYC_COMP_SHIFT	BIT(4)
#define		TIMER_INT_ONE_COMP_SHIFT	BIT(3)
#define		TIMER_INT_ROLLOVER_SHIFT	BIT(2)
#define		TIMER_INT_TACHO_UNDER_SHIFT	BIT(1)
#define		TIMER_INT_TACHO_OVER_SHIFT	BIT(0)
#define	TIMER_TACHO_OVER_REG	0x30
#define	TIMER_TACHO_UNDER_REG	0x34
#define	TIMER_START_VALUE_REG	0x38

#define	TIMER_INT_CLR_MASK	GENMASK(5, 0)

enum tacho_modes {
tacho_mode = 1,
capture_mode,
};

enum edge_modes {
rising_edge,
falling_edge,
double_edge,
};

struct phytium_tacho {
	struct device *dev;
	struct device *hwmon;
	void __iomem *base;
	struct clk *clk;
	u32 freq;
	int irq;
	u8 work_mode;
	u8 edge_mode;
	u32 debounce;
};

static u16 capture_count;

static void phytium_tacho_init(struct phytium_tacho *tacho)
{
	u32 val;

	if (tacho->work_mode == tacho_mode) {
		val = (TIMER_CTRL_TACHO_EN_SHIFT |
			       TIMER_CTRL_CNT_EN_SHIFT |
			       (tacho->edge_mode << TIMER_CTRL_TACHO_MODE_SHIFT) |
			       (tacho->debounce << TIMER_CTRL_ANTI_JITTER_SHIFT) |
			       (tacho->work_mode << TIMER_CTRL_MODE_SHIFT));
		writel_relaxed(val, tacho->base + TIMER_CTRL_REG);
		writel_relaxed(0x2faf07f, tacho->base + TIMER_CMP_VALUE_LOW_REG);
	} else {
		val = (TIMER_CTRL_TACHO_EN_SHIFT |
			       TIMER_CTRL_CNT_EN_SHIFT |
			       (tacho->edge_mode << TIMER_CTRL_TACHO_MODE_SHIFT) |
			       (tacho->debounce << TIMER_CTRL_ANTI_JITTER_SHIFT) |
			       TIMER_CTRL_CAPTURE_EN_SHIFT |
			       (0x7f << TIMER_CTRL_CAPTURE_CNT_SHIFT) |
			       (tacho->work_mode << TIMER_CTRL_MODE_SHIFT)),
		writel_relaxed(val, tacho->base + TIMER_CTRL_REG);
		writel_relaxed(0x20, tacho->base + TIMER_INT_MASK_REG);
	}
}

static int phytium_get_fan_tach_rpm(struct phytium_tacho *priv)
{
	u64 raw_data, tach_div, clk_source;
	u8 mode, both;
	unsigned long timeout;
	unsigned long loopcounter;

	timeout = jiffies + msecs_to_jiffies(500);

	for (loopcounter = 0;; loopcounter++) {
		raw_data = readl_relaxed(priv->base + TIMER_TACHO_RES_REG);

		if (raw_data & TIMER_TACHO_RES_VALID_SHIFT)
			break;

		if (time_after(jiffies, timeout))
			return -ETIMEDOUT;

		if (loopcounter > 3000)
			msleep(20);
		else {
			udelay(100);
			cond_resched();
		}
	}

	raw_data = raw_data & TIMER_TACHO_RES_MASK;
	clk_source = priv->freq;
	mode = priv->edge_mode;
	both = (mode == double_edge) ? 1 : 0;
	tach_div = 1 << both;

	if (raw_data == 0)
		return 0;

	return (clk_source * 60 * raw_data) / 0x2faf080 / tach_div;
}

static ssize_t show_rpm(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	int rpm;
	struct phytium_tacho *priv = dev_get_drvdata(dev);

	rpm = phytium_get_fan_tach_rpm(priv);
	if (rpm < 0)
		return rpm;

	return sprintf(buf, "%d\n", rpm);
}

static SENSOR_DEVICE_ATTR(fan_input, 0444,
		show_rpm, NULL, 0);

static struct attribute *tacho_dev_attrs[] = {
	&sensor_dev_attr_fan_input.dev_attr.attr,
	NULL
};

static umode_t tacho_dev_is_visible(struct kobject *kobj,
			      struct attribute *a, int index)
{
	return a->mode;
}

static const struct attribute_group tacho_group = {
	.attrs = tacho_dev_attrs,
	.is_visible = tacho_dev_is_visible,
};

static const struct attribute_group *tacho_groups[] = {
	&tacho_group,
	NULL
};

static irqreturn_t capture_irq_handler(int irq, void *dev_id)
{
	struct phytium_tacho *priv = dev_id;
	u32 status = readl_relaxed(priv->base + TIMER_INT_STAT_REG);

	if (status & TIMER_INT_CAPTURE_SHIFT) {
		capture_count++;

		if (capture_count == 0)
			dev_err(priv->dev, "Capture counter is overflowed");

		writel_relaxed(status, priv->base + TIMER_INT_STAT_REG);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

static ssize_t show_capture(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	int cnt;
	struct phytium_tacho *priv = dev_get_drvdata(dev);

	cnt = capture_count * 0x7f + readl_relaxed(priv->base + TIMER_CNT_VALUE_LOW_REG);

	return sprintf(buf, "%d\n", cnt);
}

static SENSOR_DEVICE_ATTR(capture_input, 0444,
		show_capture, NULL, 0);

static struct attribute *capture_dev_attrs[] = {
	&sensor_dev_attr_capture_input.dev_attr.attr,
	NULL
};

static umode_t capture_dev_is_visible(struct kobject *kobj,
			      struct attribute *a, int index)
{
	return a->mode;
}

static const struct attribute_group capture_group = {
	.attrs = capture_dev_attrs,
	.is_visible = capture_dev_is_visible,
};

static const struct attribute_group *capture_groups[] = {
	&capture_group,
	NULL
};

static int phytium_tacho_get_work_mode(struct phytium_tacho *tacho)
{
	struct fwnode_handle *nc = dev_fwnode(tacho->dev);

	if (fwnode_property_read_bool(nc, "tacho"))
		return tacho_mode;
	if (fwnode_property_read_bool(nc, "capture"))
		return capture_mode;
	return tacho_mode;
}

static int phytium_tacho_get_edge_mode(struct phytium_tacho *tacho)
{
	struct fwnode_handle *nc = dev_fwnode(tacho->dev);

	if (fwnode_property_read_bool(nc, "up"))
		return rising_edge;
	if (fwnode_property_read_bool(nc, "down"))
		return falling_edge;
	if (fwnode_property_read_bool(nc, "double"))
		return double_edge;
	return rising_edge;
}

static int phytium_tacho_get_debounce(struct phytium_tacho *tacho)
{
	u32 value;
	struct fwnode_handle *nc = dev_fwnode(tacho->dev);

	if (!fwnode_property_read_u32(nc, "debounce-level", &value))
		return value;
	else
		return 0;
}

static void phytium_tacho_get_of_data(struct phytium_tacho *tacho)
{
	tacho->work_mode = phytium_tacho_get_work_mode(tacho);
	tacho->edge_mode = phytium_tacho_get_edge_mode(tacho);
	tacho->debounce = phytium_tacho_get_debounce(tacho);
}

static int phytium_tacho_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct phytium_tacho *tacho;
	int ret;

	tacho = devm_kzalloc(dev, sizeof(*tacho), GFP_KERNEL);
	if (!tacho)
		return -ENOMEM;

	tacho->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOENT;

	tacho->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(tacho->base)) {
		dev_err(&pdev->dev, "region map failed\n");
		return PTR_ERR(tacho->base);
	}
	if (dev->of_node) {
		tacho->clk = devm_clk_get(&pdev->dev, NULL);
		if (IS_ERR(tacho->clk))
			return PTR_ERR(tacho->clk);
		ret = clk_prepare_enable(tacho->clk);
		if (ret)
			return ret;

		tacho->freq = clk_get_rate(tacho->clk);
	} else if (has_acpi_companion(dev)){
		if(fwnode_property_read_u32(dev_fwnode(dev),"clock-frequency", (u32 *)&(tacho->freq) ) <0)
			tacho->freq = 50000000;
    }

	tacho->irq = platform_get_irq(pdev, 0);
	if (tacho->irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		return tacho->irq;
	}

	ret = devm_request_irq(dev, tacho->irq, capture_irq_handler,
			       0, "phytium_tacho", tacho);
	if (ret) {
		dev_err(&pdev->dev, "Cannot request IRQ\n");
		return ret;
	}

	phytium_tacho_get_of_data(tacho);

	phytium_tacho_init(tacho);

	if (tacho->work_mode == tacho_mode)
		tacho->hwmon = devm_hwmon_device_register_with_groups(dev,
						       "phytium_tacho",
						       tacho, tacho_groups);
	else
		tacho->hwmon = devm_hwmon_device_register_with_groups(dev,
						       "phytium_capture",
						       tacho, capture_groups);

	platform_set_drvdata(pdev, tacho);

	return PTR_ERR_OR_ZERO(tacho->hwmon);
}

#ifdef CONFIG_PM_SLEEP
static int phytium_tacho_suspend(struct device *dev)
{
	return 0;
}

static int phytium_tacho_resume(struct device *dev)
{
	struct phytium_tacho *tacho = dev_get_drvdata(dev);

	phytium_tacho_init(tacho);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_tacho_pm, phytium_tacho_suspend, phytium_tacho_resume);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_tacho_acpi_ids[] = {
       { "PHYT0033", 0 },
       { /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_tacho_acpi_ids);
#endif

static const struct of_device_id tacho_of_match[] = {
	{ .compatible = "phytium,tacho", },
	{},
};
MODULE_DEVICE_TABLE(of, tacho_of_match);

static struct platform_driver phytium_tacho_driver = {
	.probe		= phytium_tacho_probe,
	.driver	= {
		.name	= "phytium_tacho",
		.pm	= &phytium_tacho_pm,
		.of_match_table = of_match_ptr(tacho_of_match),
		.acpi_match_table = ACPI_PTR(phytium_tacho_acpi_ids),
	},
};

module_platform_driver(phytium_tacho_driver);

MODULE_AUTHOR("Zhang Yiqun <zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium tachometer driver");
MODULE_LICENSE("GPL");
