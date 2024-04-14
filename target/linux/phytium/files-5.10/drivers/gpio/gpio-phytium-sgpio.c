// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SGPIO Driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/gpio/driver.h>
#include <linux/hashtable.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/string.h>

#define SGPIO_CTL0_REG			0x00
#define  SGPIO_CTL0_REG_ENABLE		BIT(0)
#define  SGPIO_CTL0_REG_RX_DISABLE	BIT(1)
#define  SGPIO_CTL0_REG_L3_L0		GENMASK(11, 8)
#define  SGPIO_CTL0_REG_CLK_DIV_NUM	GENMASK(31, 12)
#define SGPIO_CTL1_REG			0x04
#define  SGPIO_CTL1_REG_READY		BIT(0)
#define  SGPIO_CTL1_REG_W_UPDATA	BIT(1)
#define  SGPIO_CTL1_REG_OP_MODE		BIT(2)
#define  SGPIO_CTL1_REG_OP_STATE	BIT(3)
#define  SGPIO_CTL1_REG_BIT_NUM		GENMASK(14, 8)
#define  SGPIO_CTL1_REG_INTERVAL_TIMER	GENMASK(31, 16)
#define SGPIO_SOFT_RESET_REG		0x08
#define  SGPIO_SOFT_RESET_REG_MASK	BIT(0)
#define SGPIO_IRQ_REG			0x0c
#define  SGPIO_IRQ_REG_MASK		BIT(0)
#define SGPIO_IRQ_M_REG			0x10
#define  SGPIO_IRQ_M_REG_MASK		BIT(0)
#define SGPIO_WDATA0_REG		0x14
#define SGPIO_WDATA_REG(x)		(SGPIO_WDATA0_REG + ((x) == 2 ? 3 : (x))* 4) // 0x14, 0x18, 0x20
#define SGPIO_RDATA0_REG		0x24
#define SGPIO_RDATA_REG(x)		(SGPIO_RDATA0_REG + (x) * 4)

#define DEFAULT_L3_L0 0

#define GPIO_GROUP(x)	((x) >> 6)
#define GPIO_OFFSET(x)	((x) & GENMASK(5, 0))
#define GPIO_BIT(x)	BIT(GPIO_OFFSET(x) >> 1)

struct phytium_sgpio {
	struct gpio_chip gc;
	void __iomem *regs;
	unsigned int ngpios;
	struct clk *pclk;

	struct mutex lock;
	struct completion completion;
};

static bool phytium_sgpio_is_input(unsigned int offset)
{
	return !(offset % 2);
}

static int sgpio_set_value(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct phytium_sgpio *gpio = gpiochip_get_data(gc);
	u32 reg;
	int rc = 0;

	if (phytium_sgpio_is_input(offset))
		return -EINVAL;

	reinit_completion(&gpio->completion);

	/*
	 * Since this is an output, read the cached value from rdata,
	 * then update value.
	 */
	/* cached data from wdata? */
	reg = readl(gpio->regs + SGPIO_WDATA_REG(GPIO_GROUP(offset)));
	if (val)
		reg |= GPIO_BIT(offset);
	else
		reg &= GPIO_BIT(offset);
	writel(reg, gpio->regs + SGPIO_WDATA_REG(GPIO_GROUP(offset)));

	/* Start transmission and wait for completion */
	writel(readl(gpio->regs + SGPIO_CTL1_REG) | SGPIO_CTL1_REG_W_UPDATA,
	       gpio->regs + SGPIO_CTL1_REG);
	if (!wait_for_completion_timeout(&gpio->completion, msecs_to_jiffies(1000)))
		rc = -EINVAL;

	return rc;
}

static int phytium_sgpio_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	return phytium_sgpio_is_input(offset) ? 0 : -EINVAL;
}

static int phytium_sgpio_direction_output(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct phytium_sgpio *gpio = gpiochip_get_data(gc);
	int rc;

	mutex_lock(&gpio->lock);

	/*
	 * No special action is required for setting the direction; we'll
	 * error-out in sgpio_set_value if this isn't an output GPIO
	 */
	rc = sgpio_set_value(&gpio->gc, offset, val);

	mutex_unlock(&gpio->lock);

	return rc;
}

static int phytium_sgpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	return !!phytium_sgpio_is_input(offset);
}

static int phytium_sgpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct phytium_sgpio *gpio = gpiochip_get_data(gc);
	int rc = 0;
	u32 val, ctl0;

	mutex_lock(&gpio->lock);

	if (!phytium_sgpio_is_input(offset)) {
		val = readl(gpio->regs + SGPIO_WDATA_REG(GPIO_GROUP(offset)));
		rc = !!(val & GPIO_BIT(offset));
		mutex_unlock(&gpio->lock);
		return rc;
	}

	reinit_completion(&gpio->completion);

	/* Enable Rx */
	ctl0 = readl(gpio->regs + SGPIO_CTL0_REG);
	writel(ctl0 & ~SGPIO_CTL0_REG_RX_DISABLE, gpio->regs + SGPIO_CTL0_REG);

	/* Start reading transaction and wait for completion */
	writel(readl(gpio->regs + SGPIO_CTL1_REG) | SGPIO_CTL1_REG_W_UPDATA,
	       gpio->regs + SGPIO_CTL1_REG);
	if (!wait_for_completion_timeout(&gpio->completion, msecs_to_jiffies(1000))) {
		rc = -EINVAL;
		goto err;
	}

	val = readl(gpio->regs + SGPIO_RDATA_REG(GPIO_GROUP(offset)));
	rc = !!(val & GPIO_BIT(offset));

err:
	/* Disalbe Rx to hold the value */
	writel(ctl0 | SGPIO_CTL0_REG_RX_DISABLE, gpio->regs + SGPIO_CTL0_REG);
	mutex_unlock(&gpio->lock);

	return rc;
}

static void phytium_sgpio_set(struct gpio_chip *gc, unsigned int offset, int val)
{
	struct phytium_sgpio *gpio = gpiochip_get_data(gc);

	mutex_lock(&gpio->lock);

	sgpio_set_value(gc, offset, val);

	mutex_unlock(&gpio->lock);
}

static irqreturn_t phytium_sgpio_irq_handler(int irq, void *data)
{
	struct phytium_sgpio *gpio = data;

	if (!readl(gpio->regs + SGPIO_IRQ_REG))
		return IRQ_NONE;

	/* Clear the interrupt */
	writel(0, gpio->regs + SGPIO_IRQ_REG);

	/* Check if tx/rx has been done */
	if (!(readl(gpio->regs + SGPIO_CTL1_REG) & SGPIO_CTL1_REG_OP_STATE))
		complete(&gpio->completion);

	return IRQ_HANDLED;
}

static int phytium_sgpio_probe(struct platform_device *pdev)
{
	u32 pclk_freq, sclk_freq, clk_div;
	struct phytium_sgpio *gpio;
	struct resource *res;
	struct device *dev = &pdev->dev;
	int rc;

	gpio = devm_kzalloc(dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gpio->regs))
		return PTR_ERR(gpio->regs);

	if (devm_request_irq(dev, platform_get_irq(pdev, 0),
			     phytium_sgpio_irq_handler,
			     IRQF_SHARED, dev_name(dev), gpio)) {
		dev_err(dev, "failed to request IRQ\n");
		return -ENOENT;
	}

	rc = fwnode_property_read_u32(dev_fwnode(dev), "ngpios", &gpio->ngpios);
	if (rc < 0) {
		dev_err(dev, "Could not read ngpios property\n");
		return -EINVAL;
	} else if (gpio->ngpios % 32) {
		dev_err(&pdev->dev, "Number of GPIOs not multiple of 32: %d\n",
			gpio->ngpios);
		return -EINVAL;
	}

	rc = fwnode_property_read_u32(dev_fwnode(dev), "bus-frequency", &sclk_freq);
	if (rc < 0) {
		dev_err(dev, "Could not read bus-frequency property\n");
		return -EINVAL;
	}

	gpio->pclk = devm_clk_get(dev, NULL);
	if (IS_ERR(gpio->pclk)) {
		dev_err(dev, "Could not get the APB clock property\n");
		return PTR_ERR(gpio->pclk);
	}
	rc = clk_prepare_enable(gpio->pclk);
	if (rc) {
		dev_err(dev, "failed to enable pclk: %d\n", rc);
		return rc;
	}
	pclk_freq = clk_get_rate(gpio->pclk);

	/*
	 * From the datasheet:
	 *   (pclk / 2) / (clk_div + 1) = sclk
	 */
	if (sclk_freq == 0) {
		dev_err(dev, "SCLK should not be 0\n");
		return -EINVAL;
	}

	clk_div = (pclk_freq / (sclk_freq * 2)) - 1;
	if (clk_div > (1 << 20) - 1) {
		dev_err(dev, "clk_div is overflow\n");
		return -EINVAL;
	}

	writel(FIELD_PREP(SGPIO_CTL0_REG_CLK_DIV_NUM, clk_div) |
	       FIELD_PREP(SGPIO_CTL0_REG_L3_L0, DEFAULT_L3_L0) |
	       SGPIO_CTL0_REG_RX_DISABLE | SGPIO_CTL0_REG_ENABLE,
	       gpio->regs + SGPIO_CTL0_REG);

	writel(FIELD_PREP(SGPIO_CTL1_REG_BIT_NUM, gpio->ngpios) |
	       SGPIO_CTL1_REG_READY, gpio->regs + SGPIO_CTL1_REG);

	mutex_init(&gpio->lock);
	init_completion(&gpio->completion);
	platform_set_drvdata(pdev, gpio);

	gpio->gc.parent = dev;
	gpio->gc.base = -1;
	gpio->gc.ngpio = gpio->ngpios * 2;
	gpio->gc.label = dev_name(dev);
	gpio->gc.direction_input = phytium_sgpio_direction_input;
	gpio->gc.direction_output = phytium_sgpio_direction_output;
	gpio->gc.get_direction = phytium_sgpio_get_direction;
	gpio->gc.get = phytium_sgpio_get;
	gpio->gc.set = phytium_sgpio_set;

	return devm_gpiochip_add_data(dev, &gpio->gc, gpio);
}

static const struct of_device_id phytium_sgpio_of_match[] = {
	{ .compatible = "phytium,sgpio", },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_sgpio_of_match);

static struct platform_driver phytium_sgpio_driver = {
	.driver = {
		.name = KBUILD_MODNAME,
		.of_match_table = of_match_ptr(phytium_sgpio_of_match),
	},
	.probe = phytium_sgpio_probe,
};
module_platform_driver(phytium_sgpio_driver);

MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium SGPIO driver");
MODULE_LICENSE("GPL");
