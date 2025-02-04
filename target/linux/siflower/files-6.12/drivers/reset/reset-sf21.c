// SPDX-License-Identifier: GPL-2.0-only

#include <linux/init.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset-controller.h>
#include <linux/slab.h>
#include <linux/mfd/syscon.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <dt-bindings/reset/siflower,sf21-reset.h>

#define SF21_SOFT_RESET 0xC0

struct sf21_reset_data {
	struct reset_controller_dev rcdev;
	struct regmap *regmap;
};

static inline int sf21_reset_shift(unsigned long id)
{
	switch (id) {
	case SF21_RESET_GIC:
	case SF21_RESET_AXI:
	case SF21_RESET_AHB:
	case SF21_RESET_APB:
	case SF21_RESET_IRAM:
		return id + 1;
	case SF21_RESET_NPU:
	case SF21_RESET_DDR_CTL:
	case SF21_RESET_DDR_PHY:
	case SF21_RESET_DDR_PWR_OK_IN:
	case SF21_RESET_DDR_CTL_APB:
	case SF21_RESET_DDR_PHY_APB:
		return id + 2;
	case SF21_RESET_USB:
		return id + 8;
	case SF21_RESET_PVT:
	case SF21_RESET_SERDES_CSR:
		return id + 11;
	case SF21_RESET_CRYPT_CSR:
	case SF21_RESET_CRYPT_APP:
	case SF21_RESET_NPU2DDR_ASYNCBRIDGE:
	case SF21_RESET_IROM:
		return id + 14;
	default:
		return -EINVAL;
	}
}

static int sf21_reset_assert(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	struct sf21_reset_data *rd;
	int shift;
	u32 mask;

	rd = container_of(rcdev, struct sf21_reset_data, rcdev);

	shift = sf21_reset_shift(id);
	mask = BIT(shift);
	return regmap_update_bits(rd->regmap, SF21_SOFT_RESET, mask, 0);
}

static int sf21_reset_deassert(struct reset_controller_dev *rcdev,
				    unsigned long id)
{
	struct sf21_reset_data *rd;
	int shift;
	u32 mask;

	rd = container_of(rcdev, struct sf21_reset_data, rcdev);

	shift = sf21_reset_shift(id);
	mask = BIT(shift);
	return regmap_update_bits(rd->regmap, SF21_SOFT_RESET, mask, mask);
}

static int sf21_reset_status(struct reset_controller_dev *rcdev,
				  unsigned long id)
{
	struct sf21_reset_data *rd;
	int shift, ret;
	u32 mask;
	u32 reg;

	rd = container_of(rcdev, struct sf21_reset_data, rcdev);
	ret = regmap_read(rd->regmap, SF21_SOFT_RESET, &reg);
	if (ret)
		return ret;

	shift = sf21_reset_shift(id);
	mask = BIT(shift);
	return !!(reg & mask);
}

static const struct reset_control_ops sf21_reset_ops = {
	.assert = sf21_reset_assert,
	.deassert = sf21_reset_deassert,
	.status = sf21_reset_status,
};

static int sf21_reset_probe(struct platform_device *pdev)
{
	struct sf21_reset_data *rd;
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *node;

	rd = devm_kzalloc(dev, sizeof(*rd), GFP_KERNEL);
	if (!rd)
		return -ENOMEM;

	node = of_parse_phandle(np, "siflower,crm", 0);
	rd->regmap = syscon_node_to_regmap(node);

	if (IS_ERR(rd->regmap))
		return PTR_ERR(rd->regmap);

	rd->rcdev.owner = THIS_MODULE;
	rd->rcdev.nr_resets = SF21_RESET_MAX + 1;
	rd->rcdev.ops = &sf21_reset_ops;
	rd->rcdev.of_node = np;

	return devm_reset_controller_register(dev, &rd->rcdev);
}

static const struct of_device_id sf21_reset_dt_ids[] = {
	{ .compatible = "siflower,sf21-reset" },
	{},
};

static struct platform_driver sf21_reset_driver = {
	.probe	= sf21_reset_probe,
	.driver = {
		.name		= "sf21-reset",
		.of_match_table	= sf21_reset_dt_ids,
	},
};
builtin_platform_driver(sf21_reset_driver);
