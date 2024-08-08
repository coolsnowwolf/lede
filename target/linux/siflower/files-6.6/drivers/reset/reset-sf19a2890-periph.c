// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/spinlock.h>

struct reset_sf19a28_periph_data {
	struct reset_controller_dev rcdev;
	void __iomem *base;
	spinlock_t lock;
	u32 reset_masks[];
};

static inline struct reset_sf19a28_periph_data *
to_reset_sf19a28_periph_data(struct reset_controller_dev *rcdev)
{
	return container_of(rcdev, struct reset_sf19a28_periph_data, rcdev);
}

static int reset_sf19a28_periph_update(struct reset_controller_dev *rcdev,
			       unsigned long id, bool assert)
{
	struct reset_sf19a28_periph_data *data = to_reset_sf19a28_periph_data(rcdev);
	unsigned long flags;
	u32 reg;

	spin_lock_irqsave(&data->lock, flags);
	reg = readl(data->base);
	if (assert)
		reg |= data->reset_masks[id];
	else
		reg &= ~data->reset_masks[id];
	writel(reg, data->base);
	spin_unlock_irqrestore(&data->lock, flags);
	return 0;
}

static int reset_sf19a28_periph_assert(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	return reset_sf19a28_periph_update(rcdev, id, true);
}

static int reset_sf19a28_periph_deassert(struct reset_controller_dev *rcdev,
				 unsigned long id)
{
	return reset_sf19a28_periph_update(rcdev, id, false);
}

static int reset_sf19a28_periph_status(struct reset_controller_dev *rcdev,
			       unsigned long id)
{
	struct reset_sf19a28_periph_data *data = to_reset_sf19a28_periph_data(rcdev);
	u32 reg;

	reg = readl(data->base);
	return !!(reg & data->reset_masks[id]);
}

const struct reset_control_ops reset_sf19a28_periph_ops = {
	.assert		= reset_sf19a28_periph_assert,
	.deassert	= reset_sf19a28_periph_deassert,
	.status		= reset_sf19a28_periph_status,
};

static const struct of_device_id reset_sf19a28_periph_dt_ids[] = {
	{ .compatible = "siflower,sf19a2890-periph-reset", },
	{ /* sentinel */ },
};

static int reset_sf19a28_periph_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct reset_sf19a28_periph_data *data;
	void __iomem *base;
	int nr_resets;
	int ret, i;
	u32 tmp;

	nr_resets = of_property_count_u32_elems(node, "siflower,reset-masks");

	if (nr_resets < 1) {
		ret = of_property_read_u32(node, "siflower,num-resets", &tmp);
		if (ret < 0 || tmp < 1)
			return -EINVAL;
		nr_resets = tmp;
	}

	if (nr_resets >= 32) {
		dev_err(dev, "too many resets.");
		return -EINVAL;
	}

	data = devm_kzalloc(dev, struct_size(data, reset_masks, nr_resets), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	ret = of_property_read_u32_array(node, "siflower,reset-masks",
					 data->reset_masks, nr_resets);
	if (ret)
		for (i = 0; i < nr_resets; i++)
			data->reset_masks[i] = BIT(i);

	base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(base))
		return PTR_ERR(base);

	spin_lock_init(&data->lock);
	data->base = base;
	data->rcdev.owner = THIS_MODULE;
	data->rcdev.nr_resets = nr_resets;
	data->rcdev.ops = &reset_sf19a28_periph_ops;
	data->rcdev.of_node = dev->of_node;

	return devm_reset_controller_register(dev, &data->rcdev);
}

static struct platform_driver reset_sf19a28_periph_driver = {
	.probe	= reset_sf19a28_periph_probe,
	.driver = {
		.name		= "reset-sf19a2890-periph",
		.of_match_table	= reset_sf19a28_periph_dt_ids,
	},
};
builtin_platform_driver(reset_sf19a28_periph_driver);
