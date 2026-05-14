#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/reset.h>

#include "dpns.h"



static int dpns_probe(struct platform_device *pdev)
{
	struct dpns_priv *priv;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->dev = &pdev->dev;
	priv->ioaddr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->ioaddr))
		return PTR_ERR(priv->ioaddr);

	priv->clk = devm_clk_get_enabled(priv->dev, NULL);
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	priv->npu_rst = devm_reset_control_get_exclusive(priv->dev, "npu");
	if (IS_ERR(priv->npu_rst))
		return PTR_ERR(priv->npu_rst);

	reset_control_assert(priv->npu_rst);
	reset_control_deassert(priv->npu_rst);

	ret = dpns_se_init(priv);
	if (ret)
		return dev_err_probe(priv->dev, ret, "failed to initialize SE.\n");

	ret = dpns_tmu_init(priv);
	if (ret)
		return dev_err_probe(priv->dev, ret, "failed to initialize TMU.\n");

	sf_dpns_debugfs_init(priv);
	platform_set_drvdata(pdev, priv);
	return 0;
}

static void dpns_remove(struct platform_device *pdev) {
	struct dpns_priv *priv = platform_get_drvdata(pdev);
	debugfs_remove_recursive(priv->debugfs);
	reset_control_assert(priv->npu_rst);
}

static const struct of_device_id dpns_match[] = {
	{ .compatible = "siflower,sf21-dpns" },
	{},
};
MODULE_DEVICE_TABLE(of, dpns_match);

static struct platform_driver dpns_driver = {
	.probe	= dpns_probe,
	.remove = dpns_remove,
	.driver	= {
		.name		= "sfdpns",
		.of_match_table	= dpns_match,
	},
};
module_platform_driver(dpns_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qingfang Deng <qingfang.deng@siflower.com.cn>");
MODULE_DESCRIPTION("NPU stub driver for SF21A6826/SF21H8898 SoC");
