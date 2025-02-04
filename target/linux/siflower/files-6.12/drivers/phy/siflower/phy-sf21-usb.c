// SPDX-License-Identifier: GPL-2.0

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>

#define SF21_USB_PHY_RESET_OFFSET		0xC
#define  RST_PRST				BIT(0)
#define  RST_PHY_POR				BIT(1)
#define  RST_PHY_PORT				BIT(2)

struct sf21_usb_phy {
	struct device *dev;
	struct clk *phy_clk;
	struct clk *bus_clk;
	void __iomem *base;
};

static int sf21_usb_phy_power_on(struct phy *phy)
{
	struct sf21_usb_phy *p_phy = phy_get_drvdata(phy);
	int ret;

	writel(RST_PRST | RST_PHY_POR | RST_PHY_PORT,
	       p_phy->base + SF21_USB_PHY_RESET_OFFSET);

	ret = clk_prepare_enable(p_phy->phy_clk);
	if (ret < 0) {
		dev_err(p_phy->dev, "Failed to enable PHY clock: %d\n", ret);
		return ret;
	}

	writel(RST_PRST, p_phy->base + SF21_USB_PHY_RESET_OFFSET);
	usleep_range(50, 1000);
	writel(0, p_phy->base + SF21_USB_PHY_RESET_OFFSET);
	udelay(5);

	return ret;
}

static int sf21_usb_phy_power_off(struct phy *phy)
{
	struct sf21_usb_phy *p_phy = phy_get_drvdata(phy);

	writel(RST_PRST | RST_PHY_POR | RST_PHY_PORT,
	       p_phy->base + SF21_USB_PHY_RESET_OFFSET);
	clk_disable_unprepare(p_phy->phy_clk);
	return 0;
}

static const struct phy_ops sf21_usb_phy_ops = {
	.power_on = sf21_usb_phy_power_on,
	.power_off = sf21_usb_phy_power_off,
	.owner = THIS_MODULE,
};

static int sf21_usb_phy_probe(struct platform_device *pdev)
{
	struct sf21_usb_phy *p_phy;
	struct phy_provider *provider;
	struct phy *phy;

	p_phy = devm_kzalloc(&pdev->dev, sizeof(*p_phy), GFP_KERNEL);
	if (!p_phy)
		return -ENOMEM;

	p_phy->dev = &pdev->dev;
	platform_set_drvdata(pdev, p_phy);

	p_phy->phy_clk = devm_clk_get(p_phy->dev, "usb_phy_clk");
	if (IS_ERR(p_phy->phy_clk))
		return dev_err_probe(p_phy->dev, PTR_ERR(p_phy->phy_clk),
				     "Failed to get usb_phy clock.\n");

	p_phy->base = devm_platform_ioremap_resource(pdev, 0);

	phy = devm_phy_create(p_phy->dev, NULL, &sf21_usb_phy_ops);
	if (IS_ERR(phy))
		return dev_err_probe(p_phy->dev, PTR_ERR(phy),
				     "Failed to create PHY.\n");

	phy_set_drvdata(phy, p_phy);

	provider = devm_of_phy_provider_register(p_phy->dev, of_phy_simple_xlate);
	if (IS_ERR(provider))
		return dev_err_probe(p_phy->dev, PTR_ERR(provider),
				     "Failed to register PHY provider.\n");

	return 0;
}

static const struct of_device_id sf21_usb_phy_of_match[] = {
	{ .compatible = "siflower,sf21-usb-phy", },
	{ },
};
MODULE_DEVICE_TABLE(of, sf21_usb_phy_of_match);

static struct platform_driver sf21_usb_phy_driver = {
	.probe		= sf21_usb_phy_probe,
	.driver		= {
		.name	= "sf21-usb-phy",
		.of_match_table = sf21_usb_phy_of_match,
	},
};
module_platform_driver(sf21_usb_phy_driver);

MODULE_AUTHOR("Ziying Wu <ziying.wu@siflower.com.cn>");
MODULE_DESCRIPTION("Siflower SF21A6826/SF21H8898 USB2.0 PHY driver");
MODULE_LICENSE("GPL");
