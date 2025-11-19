// SPDX-License-Identifier: GPL-2.0+

#include <linux/pinctrl/consumer.h>
#include <linux/clk.h>
#include <linux/gpio/driver.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <asm/div64.h>

#define GPIO_IR(n)	(0x40 * (n) + 0x00)
#define GPIO_OR(n)	(0x40 * (n) + 0x04)
#define GPIO_OEN(n)	(0x40 * (n) + 0x08)
#define GPIO_IMR(n)	(0x40 * (n) + 0x0c)
#define GPIO_GPIMR(n)	(0x40 * (n) + 0x10)
#define GPIO_PIR(n)	(0x40 * (n) + 0x14)
#define GPIO_ITR(n)	(0x40 * (n) + 0x18)
#define GPIO_IFR(n)	(0x40 * (n) + 0x1c)
#define GPIO_ICR(n)	(0x40 * (n) + 0x20)
#define GPIO_GPxIR(n)	(0x4 * (n) + 0x4000)

#define GPIOS_PER_GROUP	16

struct sf_gpio_priv {
	struct gpio_chip gc;
	void __iomem *base;
	struct clk *clk;
	struct reset_control *rstc;
	unsigned int irq[];
};

#define to_sf_gpio(x)	container_of(x, struct sf_gpio_priv, gc)

static u32 sf_gpio_rd(struct sf_gpio_priv *priv, unsigned long reg)
{
	return readl_relaxed(priv->base + reg);
}

static void sf_gpio_wr(struct sf_gpio_priv *priv, unsigned long reg,
		       u32 val)
{
	writel_relaxed(val, priv->base + reg);
}

static int sf_gpio_get_value(struct gpio_chip *gc, unsigned int offset)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);

	return sf_gpio_rd(priv, GPIO_IR(offset));
}

static void sf_gpio_set_value(struct gpio_chip *gc, unsigned int offset,
			      int value)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);

	sf_gpio_wr(priv, GPIO_OR(offset), value);
}

static int sf_gpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);

	if (sf_gpio_rd(priv, GPIO_OEN(offset)))
		return GPIO_LINE_DIRECTION_IN;
	else
		return GPIO_LINE_DIRECTION_OUT;
}

static int sf_gpio_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);

	sf_gpio_wr(priv, GPIO_OEN(offset), 1);
	return 0;
}

static int sf_gpio_direction_output(struct gpio_chip *gc, unsigned int offset,
				    int value)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);

	sf_gpio_wr(priv, GPIO_OR(offset), value);
	sf_gpio_wr(priv, GPIO_OEN(offset), 0);
	return 0;
}

static int sf_gpio_set_debounce(struct gpio_chip *gc, unsigned int offset,
				u32 debounce)
{
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned long freq = clk_get_rate(priv->clk);
	u64 mul;

	/* (ICR + 1) * IFR = debounce_us * clkfreq_mhz / 4 */
	mul = (u64)debounce * freq;
	do_div(mul, 1000000 * 4);
	if (mul > 0xff00)
		return -EINVAL;

	sf_gpio_wr(priv, GPIO_ICR(offset), 0xff);
	sf_gpio_wr(priv, GPIO_IFR(offset), DIV_ROUND_UP(mul, 0x100));

	return 0;
}

static int sf_gpio_set_config(struct gpio_chip *gc, unsigned int offset,
			      unsigned long config)
{
	switch (pinconf_to_config_param(config)) {
	case PIN_CONFIG_INPUT_DEBOUNCE:
		return sf_gpio_set_debounce(gc, offset,
			pinconf_to_config_argument(config));
	default:
		return gpiochip_generic_config(gc, offset, config);
	}
}

static void sf_gpio_irq_ack(struct irq_data *data)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned long offset = irqd_to_hwirq(data);

	sf_gpio_wr(priv, GPIO_PIR(offset), 0);
}

static void sf_gpio_irq_mask(struct irq_data *data)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned long offset = irqd_to_hwirq(data);

	sf_gpio_wr(priv, GPIO_IMR(offset), 1);
	sf_gpio_wr(priv, GPIO_GPIMR(offset), 1);
}

static void sf_gpio_irq_unmask(struct irq_data *data)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned long offset = irqd_to_hwirq(data);

	sf_gpio_wr(priv, GPIO_IMR(offset), 0);
	sf_gpio_wr(priv, GPIO_GPIMR(offset), 0);
}

/* We are actually setting the parents' affinity. */
static int sf_gpio_irq_set_affinity(struct irq_data *data,
				    const struct cpumask *dest, bool force)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
	unsigned long offset = irqd_to_hwirq(data);
	const struct cpumask *pdest;
	struct irq_desc *pdesc;
	struct irq_data *pdata;
	unsigned int group;
	int ret;

	/* Find the parent IRQ and call its irq_set_affinity */
	group = offset / GPIOS_PER_GROUP;
	if (group >= gc->irq.num_parents)
		return -EINVAL;

	pdesc = irq_to_desc(gc->irq.parents[group]);
	if (!pdesc)
		return -EINVAL;

	pdata = irq_desc_get_irq_data(pdesc);
	if (!pdata->chip->irq_set_affinity)
		return -EINVAL;

	ret = pdata->chip->irq_set_affinity(pdata, dest, force);
	if (ret < 0)
		return ret;

	/* Copy its effective_affinity back */
	pdest = irq_data_get_effective_affinity_mask(pdata);
	irq_data_update_effective_affinity(data, pdest);
	return ret;
}

static int sf_gpio_irq_set_type(struct irq_data *data, unsigned int flow_type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(data);
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned long offset = irqd_to_hwirq(data);
	u32 val;

	switch (flow_type) {
	case IRQ_TYPE_EDGE_RISING:
		val = 4;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		val = 2;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		val = 6;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		val = 1;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		val = 0;
		break;
	default:
		return -EINVAL;
	}
	sf_gpio_wr(priv, GPIO_ITR(offset), val);

	if (flow_type & IRQ_TYPE_LEVEL_MASK)
		irq_set_handler_locked(data, handle_level_irq);
	else
		irq_set_handler_locked(data, handle_edge_irq);

	return 0;
}

static const struct irq_chip sf_gpio_irqchip = {
	.name			= KBUILD_MODNAME,
	.irq_ack		= sf_gpio_irq_ack,
	.irq_mask		= sf_gpio_irq_mask,
	.irq_unmask		= sf_gpio_irq_unmask,
	.irq_set_affinity	= sf_gpio_irq_set_affinity,
	.irq_set_type		= sf_gpio_irq_set_type,
	.flags			= IRQCHIP_IMMUTABLE,
	GPIOCHIP_IRQ_RESOURCE_HELPERS,
};

static void sf_gpio_irq_handler(struct irq_desc *desc)
{
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	struct irq_chip *ic = irq_desc_get_chip(desc);
	struct sf_gpio_priv *priv = to_sf_gpio(gc);
	unsigned int irq = irq_desc_get_irq(desc);
	unsigned int group = irq - priv->irq[0];
	unsigned long pending;
	unsigned int n;

	chained_irq_enter(ic, desc);

	pending = sf_gpio_rd(priv, GPIO_GPxIR(group));
	for_each_set_bit(n, &pending, GPIOS_PER_GROUP) {
		generic_handle_domain_irq(gc->irq.domain,
					  n + group * GPIOS_PER_GROUP);
	}

	chained_irq_exit(ic, desc);
}

static int sf_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sf_gpio_priv *priv;
	struct gpio_irq_chip *girq;
	struct gpio_chip *gc;
	u32 ngpios, ngroups;
	int ret, i;

	ngpios = (unsigned int) device_get_match_data(dev);
	ngroups = DIV_ROUND_UP(ngpios, GPIOS_PER_GROUP);
	priv = devm_kzalloc(dev, struct_size(priv, irq, ngroups), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	priv->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->clk = devm_clk_get_enabled(dev, NULL);
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	priv->rstc = devm_reset_control_get_optional(&pdev->dev, NULL);
	if (IS_ERR(priv->rstc))
		return PTR_ERR(priv->rstc);

	ret = reset_control_deassert(priv->rstc);
	if (ret)
		return ret;

	for (i = 0; i < ngroups; i++) {
		ret = platform_get_irq(pdev, i);
		if (ret < 0)
			return ret;

		priv->irq[i] = ret;
	}

	gc = &priv->gc;
	gc->label = KBUILD_MODNAME;
	gc->parent = dev;
	gc->owner = THIS_MODULE;
	gc->request = gpiochip_generic_request;
	gc->free = gpiochip_generic_free;
	gc->get_direction = sf_gpio_get_direction;
	gc->direction_input = sf_gpio_direction_input;
	gc->direction_output = sf_gpio_direction_output;
	gc->get = sf_gpio_get_value;
	gc->set = sf_gpio_set_value;
	gc->set_config = sf_gpio_set_config;
	gc->base = -1;
	gc->ngpio = ngpios;

	girq = &gc->irq;
	gpio_irq_chip_set_chip(girq, &sf_gpio_irqchip);
	girq->num_parents = ngroups;
	girq->parents = priv->irq;
	girq->parent_handler = sf_gpio_irq_handler;
	girq->default_type = IRQ_TYPE_NONE;
	girq->handler = handle_bad_irq;

	platform_set_drvdata(pdev, priv);
	return devm_gpiochip_add_data(dev, gc, priv);
}

static int sf_gpio_remove(struct platform_device *pdev)
{
	struct sf_gpio_priv *priv = platform_get_drvdata(pdev);

	reset_control_assert(priv->rstc);
	return 0;
}

static const struct of_device_id sf_gpio_ids[] = {
	{ .compatible = "siflower,sf19a2890-gpio", .data = (void *)49 },
	{},
};
MODULE_DEVICE_TABLE(of, sf_gpio_ids);

static struct platform_driver sf_gpio_driver = {
	.probe		= sf_gpio_probe,
	.remove		= sf_gpio_remove,
	.driver = {
		.name		= "siflower_gpio",
		.owner		= THIS_MODULE,
		.of_match_table	= sf_gpio_ids,
	},
};
module_platform_driver(sf_gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qingfang Deng <qingfang.deng@siflower.com.cn>");
MODULE_DESCRIPTION("GPIO driver for SiFlower SoCs");
