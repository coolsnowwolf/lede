// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for Siflower SF19A2890 pinctrl.
 *
 * Based on:
 * Driver for Broadcom BCM2835 GPIO unit (pinctrl + GPIO)
 *
 * Copyright (C) 2012 Chris Boot, Simon Arlott, Stephen Warren
 */

#include <linux/bitmap.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinconf.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#define MODULE_NAME "sf19a2890-pinctrl"

struct sf_pinctrl {
	struct device *dev;
	void __iomem *base;

	struct pinctrl_dev *pctl_dev;
	struct pinctrl_desc pctl_desc;
	struct pinctrl_gpio_range gpio_range;
};

#define SF19A28_NUM_GPIOS	49

#define SF19A28_REG_PC(pin)	((pin) * 0x8)
#define  PC_OEN			BIT(7)
#define  PC_ST			BIT(6)
#define  PC_IE			BIT(5)
#define  PC_PD			BIT(4)
#define  PC_PU			BIT(3)
#define  PC_DS			GENMASK(2, 0)

#define DRIVE_MIN		6
#define DRIVE_STEP		3
#define DRIVE_MAX		(7 * DRIVE_STEP)

#define SF19A28_REG_PMX(pin)	((pin) * 0x8 + 0x4)
/*
 * FUNC_SW:
 *  0: Override pad output enable with PC_OEN
 *  1: take OEN from GPIO or alternative function
 * FMUX_SEL:
 *  0: Alternative function mode
 *  1: GPIO mode
 */
#define  PMX_FUNC_SW		BIT(3)
#define  PMX_FMUX_SEL		BIT(2)
#define  PMX_MODE		GENMASK(1, 0)

static struct pinctrl_pin_desc sf19a2890_gpio_pins[] = {
	PINCTRL_PIN(0, "JTAG_TDO"),
	PINCTRL_PIN(1, "JTAG_TDI"),
	PINCTRL_PIN(2, "JTAG_TMS"),
	PINCTRL_PIN(3, "JTAG_TCK"),
	PINCTRL_PIN(4, "JTAG_RST"),
	PINCTRL_PIN(5, "SPI_TXD"),
	PINCTRL_PIN(6, "SPI_RXD"),
	PINCTRL_PIN(7, "SPI_CLK"),
	PINCTRL_PIN(8, "SPI_CSN"),
	PINCTRL_PIN(9, "UART_TX"),
	PINCTRL_PIN(10, "UART_RX"),
	PINCTRL_PIN(11, "I2C_DAT"),
	PINCTRL_PIN(12, "I2C_CLK"),
	PINCTRL_PIN(13, "RGMII_GTX_CLK"),
	PINCTRL_PIN(14, "RGMII_TXCLK"),
	PINCTRL_PIN(15, "RGMII_TXD0"),
	PINCTRL_PIN(16, "RGMII_TXD1"),
	PINCTRL_PIN(17, "RGMII_TXD2"),
	PINCTRL_PIN(18, "RGMII_TXD3"),
	PINCTRL_PIN(19, "RGMII_TXCTL"),
	PINCTRL_PIN(20, "RGMII_RXCLK"),
	PINCTRL_PIN(21, "RGMII_RXD0"),
	PINCTRL_PIN(22, "RGMII_RXD1"),
	PINCTRL_PIN(23, "RGMII_RXD2"),
	PINCTRL_PIN(24, "RGMII_RXD3"),
	PINCTRL_PIN(25, "RGMII_RXCTL"),
	PINCTRL_PIN(26, "RGMII_COL"),
	PINCTRL_PIN(27, "RGMII_CRS"),
	PINCTRL_PIN(28, "RGMII_MDC"),
	PINCTRL_PIN(29, "RGMII_MDIO"),
	PINCTRL_PIN(30, "HB0_PA_EN"),
	PINCTRL_PIN(31, "HB0_LNA_EN"),
	PINCTRL_PIN(32, "HB0_SW_CTRL0"),
	PINCTRL_PIN(33, "HB0_SW_CTRL1"),
	PINCTRL_PIN(34, "HB1_PA_EN"),
	PINCTRL_PIN(35, "HB1_LNA_EN"),
	PINCTRL_PIN(36, "HB1_SW_CTRL0"),
	PINCTRL_PIN(37, "HB1_SW_CTRL1"),
	PINCTRL_PIN(38, "LB0_PA_EN"),
	PINCTRL_PIN(39, "LB0_LNA_EN"),
	PINCTRL_PIN(40, "LB0_SW_CTRL0"),
	PINCTRL_PIN(41, "LB0_SW_CTRL1"),
	PINCTRL_PIN(42, "LB1_PA_EN"),
	PINCTRL_PIN(43, "LB1_LNA_EN"),
	PINCTRL_PIN(44, "LB1_SW_CTRL0"),
	PINCTRL_PIN(45, "LB1_SW_CTRL1"),
	PINCTRL_PIN(46, "CLK_OUT"),
	PINCTRL_PIN(47, "EXT_CLK_IN"),
	PINCTRL_PIN(48, "DRVVBUS0"),
};

static const char * const sf19a2890_gpio_groups[] = {
	"JTAG_TDO",
	"JTAG_TDI",
	"JTAG_TMS",
	"JTAG_TCK",
	"JTAG_RST",
	"SPI_TXD",
	"SPI_RXD",
	"SPI_CLK",
	"SPI_CSN",
	"UART_TX",
	"UART_RX",
	"I2C_DAT",
	"I2C_CLK",
	"RGMII_GTX_CLK",
	"RGMII_TXCLK",
	"RGMII_TXD0",
	"RGMII_TXD1",
	"RGMII_TXD2",
	"RGMII_TXD3",
	"RGMII_TXCTL",
	"RGMII_RXCLK",
	"RGMII_RXD0",
	"RGMII_RXD1",
	"RGMII_RXD2",
	"RGMII_RXD3",
	"RGMII_RXCTL",
	"RGMII_COL",
	"RGMII_CRS",
	"RGMII_MDC",
	"RGMII_MDIO",
	"HB0_PA_EN",
	"HB0_LNA_EN",
	"HB0_SW_CTRL0",
	"HB0_SW_CTRL1",
	"HB1_PA_EN",
	"HB1_LNA_EN",
	"HB1_SW_CTRL0",
	"HB1_SW_CTRL1",
	"LB0_PA_EN",
	"LB0_LNA_EN",
	"LB0_SW_CTRL0",
	"LB0_SW_CTRL1",
	"LB1_PA_EN",
	"LB1_LNA_EN",
	"LB1_SW_CTRL0",
	"LB1_SW_CTRL1",
	"CLK_OUT",
	"EXT_CLK_IN",
	"DRVVBUS0",
};

#define SF19A28_FUNC0		0
#define SF19A28_FUNC1		1
#define SF19A28_FUNC2		2
#define SF19A28_FUNC3		3
#define SF19A28_NUM_FUNCS	4

static const char * const sf19a2890_functions[] = {
	"func0", "func1", "func2", "func3"
};

static inline u32 sf_pinctrl_rd(struct sf_pinctrl *pc, ulong reg)
{
	return readl(pc->base + reg);
}

static inline void sf_pinctrl_wr(struct sf_pinctrl *pc, ulong reg, u32 val)
{
	writel(val, pc->base + reg);
}

static inline void sf_pinctrl_rmw(struct sf_pinctrl *pc, ulong reg, u32 clr,
				  u32 set)
{
	u32 val;

	val = sf_pinctrl_rd(pc, reg);
	val &= ~clr;
	val |= set;
	sf_pinctrl_wr(pc, reg, val);
}

static int sf19a2890_pctl_get_groups_count(struct pinctrl_dev *pctldev)
{
	return SF19A28_NUM_GPIOS;
}

static const char *sf19a2890_pctl_get_group_name(struct pinctrl_dev *pctldev,
						 unsigned selector)
{
	return sf19a2890_gpio_groups[selector];
}

static int sf19a2890_pctl_get_group_pins(struct pinctrl_dev *pctldev,
					 unsigned selector,
					 const unsigned **pins,
					 unsigned *num_pins)
{
	*pins = &sf19a2890_gpio_pins[selector].number;
	*num_pins = 1;

	return 0;
}

static void sf19a2890_pctl_pin_dbg_show(struct pinctrl_dev *pctldev,
					struct seq_file *s, unsigned offset)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	u32 conf = sf_pinctrl_rd(pc, SF19A28_REG_PC(offset));
	u32 mux = sf_pinctrl_rd(pc, SF19A28_REG_PMX(offset));

	if (!(mux & PMX_FUNC_SW))
		seq_puts(s, "Forced OE");
	else if (mux & PMX_FMUX_SEL)
		seq_puts(s, "GPIO");
	else
		seq_printf(s, "Func%lu", mux & PMX_MODE);
	seq_puts(s, " |");

	if (!(conf & PC_OEN) && !(mux & PMX_FUNC_SW))
		seq_puts(s, " Output");
	if ((conf & PC_ST))
		seq_puts(s, " Schmitt_Trigger");
	if ((conf & PC_IE))
		seq_puts(s, " Input");
	if ((conf & PC_PD))
		seq_puts(s, " Pull_Down");
	if ((conf & PC_PU))
		seq_puts(s, " Pull_Up");

	seq_printf(s, " Drive: %lu mA",
		   DRIVE_MIN + (conf & PC_DS) * DRIVE_STEP);
}

static const struct pinctrl_ops sf19a2890_pctl_ops = {
	.get_groups_count = sf19a2890_pctl_get_groups_count,
	.get_group_name = sf19a2890_pctl_get_group_name,
	.get_group_pins = sf19a2890_pctl_get_group_pins,
	.pin_dbg_show = sf19a2890_pctl_pin_dbg_show,
	.dt_node_to_map = pinconf_generic_dt_node_to_map_all,
	.dt_free_map = pinconf_generic_dt_free_map,
};

static int sf19a2890_pmx_free(struct pinctrl_dev *pctldev, unsigned offset)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	sf_pinctrl_rmw(pc, SF19A28_REG_PC(offset), PC_IE, PC_OEN);
	sf_pinctrl_rmw(pc, SF19A28_REG_PMX(offset), PMX_FUNC_SW, 0);
	return 0;
}

static int sf19a2890_pmx_get_functions_count(struct pinctrl_dev *pctldev)
{
	return SF19A28_NUM_FUNCS;
}

static const char *sf19a2890_pmx_get_function_name(struct pinctrl_dev *pctldev,
						   unsigned selector)
{
	return sf19a2890_functions[selector];
}

static int sf19a2890_pmx_get_function_groups(struct pinctrl_dev *pctldev,
					     unsigned selector,
					     const char *const **groups,
					     unsigned *const num_groups)
{
	/* every pin can do every function */
	*groups = sf19a2890_gpio_groups;
	*num_groups = SF19A28_NUM_GPIOS;

	return 0;
}

static int sf19a2890_pmx_set(struct pinctrl_dev *pctldev,
			     unsigned func_selector, unsigned group_selector)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	unsigned pin = group_selector;

	sf_pinctrl_wr(pc, SF19A28_REG_PMX(pin),
		      PMX_FUNC_SW | FIELD_PREP(PMX_MODE, func_selector));
	return 0;
}

static int sf19a2890_pmx_gpio_request_enable(struct pinctrl_dev *pctldev,
					     struct pinctrl_gpio_range *range,
					     unsigned offset)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	/* Set to GPIO mode & Let peripheral control OEN */
	sf_pinctrl_wr(pc, SF19A28_REG_PMX(offset), PMX_FUNC_SW | PMX_FMUX_SEL);
	/*
	 * Set PC_IE regardless of whether GPIO is in input mode.
	 * Otherwise GPIO driver can't read back its status in output mode.
	 */
	sf_pinctrl_rmw(pc, SF19A28_REG_PC(offset), 0, PC_IE);
	return 0;
}

static void sf19a2890_pmx_gpio_disable_free(struct pinctrl_dev *pctldev,
					    struct pinctrl_gpio_range *range,
					    unsigned offset)
{
	sf19a2890_pmx_free(pctldev, offset);
}

static const struct pinmux_ops sf19a2890_pmx_ops = {
	.free = sf19a2890_pmx_free,
	.get_functions_count = sf19a2890_pmx_get_functions_count,
	.get_function_name = sf19a2890_pmx_get_function_name,
	.get_function_groups = sf19a2890_pmx_get_function_groups,
	.set_mux = sf19a2890_pmx_set,
	.gpio_request_enable = sf19a2890_pmx_gpio_request_enable,
	.gpio_disable_free = sf19a2890_pmx_gpio_disable_free,
};

static int sf19a2890_pinconf_get(struct pinctrl_dev *pctldev, unsigned pin,
				 unsigned long *config)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param = pinconf_to_config_param(*config);
	u32 arg = 0;
	u32 val = 0;

	if (pin >= SF19A28_NUM_GPIOS)
		return -EINVAL;

	val = sf_pinctrl_rd(pc, SF19A28_REG_PC(pin));

	switch (param) {
	case PIN_CONFIG_INPUT_SCHMITT:
		val &= PC_ST;
		if (val)
			arg = 1;
		break;

	case PIN_CONFIG_INPUT_ENABLE:
		val &= PC_IE;
		if (val)
			arg = 1;
		break;

	case PIN_CONFIG_BIAS_PULL_DOWN:
		val &= PC_PD;
		if (val)
			arg = 1;
		break;

	case PIN_CONFIG_BIAS_PULL_UP:
		val &= PC_PU;
		if (val)
			arg = 1;
		break;

	case PIN_CONFIG_DRIVE_STRENGTH:
		arg = DRIVE_MIN + (val & PC_DS) * DRIVE_STEP;
		break;

	default:
		return -ENOTSUPP;
	}

	*config = pinconf_to_config_packed(param, arg);
	return 0;
}

static int sf19a2890_pinconf_set(struct pinctrl_dev *pctldev, unsigned pin,
				 unsigned long *configs, unsigned num_configs)
{
	struct sf_pinctrl *pc = pinctrl_dev_get_drvdata(pctldev);
	enum pin_config_param param;
	u32 arg, val;
	int i;

	val = sf_pinctrl_rd(pc, SF19A28_REG_PC(pin));

	if (pin >= SF19A28_NUM_GPIOS)
		return -EINVAL;

	for (i = 0; i < num_configs; i++) {
		param = pinconf_to_config_param(configs[i]);
		arg = pinconf_to_config_argument(configs[i]);
		switch (param) {
		case PIN_CONFIG_INPUT_SCHMITT_ENABLE:
			if (arg)
				val |= PC_ST;
			else
				val &= ~PC_ST;
			break;

		case PIN_CONFIG_INPUT_ENABLE:
			if (arg)
				val |= PC_IE;
			else
				val &= ~PC_IE;
			break;

		case PIN_CONFIG_BIAS_PULL_DOWN:
			if (arg) {
				val |= PC_PD;
				val &= ~PC_PU;
			} else {
				val &= ~PC_PD;
			}
			break;

		case PIN_CONFIG_BIAS_PULL_UP:
			if (arg) {
				val |= PC_PU;
				val &= ~PC_PD;
			} else {
				val &= ~PC_PU;
			}
			break;

		case PIN_CONFIG_DRIVE_STRENGTH:
			val &= ~PC_DS;
			if (arg > DRIVE_MAX)
				val |= PC_DS;
			else if (arg > DRIVE_MIN)
				val |= FIELD_PREP(PC_DS, (arg - DRIVE_MIN) /
								 DRIVE_STEP);
			break;
		default:
			break;
		}
		sf_pinctrl_wr(pc, SF19A28_REG_PC(pin), val);
	}
	return 0;
}

static const struct pinconf_ops sf19a2890_pinconf_ops = {
	.is_generic = true,
	.pin_config_get = sf19a2890_pinconf_get,
	.pin_config_set = sf19a2890_pinconf_set,
};

static const struct pinctrl_desc sf19a2890_pinctrl_desc = {
	.name = MODULE_NAME,
	.pins = sf19a2890_gpio_pins,
	.npins = SF19A28_NUM_GPIOS,
	.pctlops = &sf19a2890_pctl_ops,
	.pmxops = &sf19a2890_pmx_ops,
	.confops = &sf19a2890_pinconf_ops,
	.owner = THIS_MODULE,
};

static const struct pinctrl_gpio_range sf_pinctrl_gpio_range = {
	.name = MODULE_NAME,
	.npins = SF19A28_NUM_GPIOS,
};

static const struct of_device_id sf_pinctrl_match[] = {
	{ .compatible = "siflower,sf19a2890-pinctrl" },
	{}
};

static int sf_pinctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sf_pinctrl *pc;

	pc = devm_kzalloc(dev, sizeof(*pc), GFP_KERNEL);
	if (!pc)
		return -ENOMEM;

	platform_set_drvdata(pdev, pc);
	pc->dev = dev;

	pc->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(pc->base))
		return PTR_ERR(pc->base);

	pc->pctl_desc = sf19a2890_pinctrl_desc;
	pc->pctl_dev = devm_pinctrl_register(dev, &pc->pctl_desc, pc);
	if (IS_ERR(pc->pctl_dev))
		return PTR_ERR(pc->pctl_dev);

	return 0;
}

static struct platform_driver sf_pinctrl_driver = {
	.probe = sf_pinctrl_probe,
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = sf_pinctrl_match,
		.suppress_bind_attrs = true,
	},
};
module_platform_driver(sf_pinctrl_driver);

MODULE_AUTHOR("Chuanhong Guo <gch981213@gmail.com>");
MODULE_DESCRIPTION("Siflower SF19A2890 pinctrl driver");
MODULE_LICENSE("GPL");
