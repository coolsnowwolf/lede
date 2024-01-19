// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium display engine DRM driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/of_device.h>
#include <linux/acpi.h>
#include <drm/drm_drv.h>
#include <linux/dma-mapping.h>
#include "phytium_display_drv.h"
#include "phytium_platform.h"
#include "phytium_dp.h"
#include "phytium_gem.h"
#include "pe220x_dc.h"
#include "pe220x_dp.h"

int phytium_platform_carveout_mem_init(struct platform_device *pdev,
						      struct phytium_display_private *priv)
{
	struct resource *res;
	int ret = 0;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res) {
		priv->pool_size = resource_size(res);
		priv->pool_phys_addr = res->start;
	}

	if ((priv->pool_phys_addr != 0) && (priv->pool_size != 0)) {
		priv->pool_virt_addr = ioremap_cache(priv->pool_phys_addr, priv->pool_size);
		if (priv->pool_virt_addr == NULL) {
			DRM_ERROR("failed to remap carveout mem(0x%llx)\n", priv->pool_phys_addr);
			ret = -EINVAL;
			goto failed_ioremap;
		}
		ret = phytium_memory_pool_init(&pdev->dev, priv);
		if (ret)
			goto failed_init_memory_pool;

		priv->mem_state[PHYTIUM_MEM_SYSTEM_CARVEOUT_TOTAL] = priv->pool_size;
		priv->support_memory_type = MEMORY_TYPE_SYSTEM_CARVEOUT;
		priv->vram_hw_init = NULL;
	} else {
		DRM_DEBUG_KMS("not support carveout memory\n");
		priv->mem_state[PHYTIUM_MEM_SYSTEM_CARVEOUT_TOTAL] = 0;
		priv->support_memory_type = MEMORY_TYPE_SYSTEM_UNIFIED;
		priv->vram_hw_init = NULL;
	}

	return 0;

failed_init_memory_pool:
	iounmap(priv->pool_virt_addr);
failed_ioremap:
	return ret;
}

void phytium_platform_carveout_mem_fini(struct platform_device *pdev,
						       struct phytium_display_private *priv)
{
	if (priv->support_memory_type == MEMORY_TYPE_SYSTEM_CARVEOUT) {
		phytium_memory_pool_fini(&pdev->dev, priv);
		iounmap(priv->pool_virt_addr);
	}
}

static struct phytium_display_private *
phytium_platform_private_init(struct platform_device *pdev)
{
	struct drm_device *dev = dev_get_drvdata(&pdev->dev);
	struct device_node *node;
	struct fwnode_handle *np;
	struct phytium_display_private *priv = NULL;
	struct phytium_platform_private *platform_priv = NULL;
	struct phytium_device_info *phytium_info = NULL;
	int i = 0, ret = 0;
	struct resource *res;

	platform_priv = devm_kzalloc(&pdev->dev, sizeof(*platform_priv), GFP_KERNEL);
	if (!platform_priv) {
		DRM_ERROR("no memory to allocate for phytium_platform_private\n");
		goto exit;
	}

	memset(platform_priv, 0, sizeof(*platform_priv));
	priv = &platform_priv->base;
	phytium_display_private_init(priv, dev);

	if (pdev->dev.of_node) {
		phytium_info = (struct phytium_device_info *)of_device_get_match_data(&pdev->dev);
		if (!phytium_info) {
			DRM_ERROR("failed to get dts id data(phytium_info)\n");
			goto failed;
		}

		memcpy(&(priv->info), phytium_info, sizeof(struct phytium_device_info));
		node = pdev->dev.of_node;
		ret = of_property_read_u8(node, "pipe_mask", &priv->info.pipe_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing pipe_mask property from dts\n");
			goto failed;
		}

		ret = of_property_read_u8(node, "edp_mask", &priv->info.edp_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing edp_mask property from dts\n");
			goto failed;
		}
	} else if (has_acpi_companion(&pdev->dev)) {
		phytium_info = (struct phytium_device_info *)acpi_device_get_match_data(&pdev->dev);
		if (!phytium_info) {
			DRM_ERROR("failed to get acpi id data(phytium_info)\n");
			goto failed;
		}

		memcpy(&(priv->info), phytium_info, sizeof(struct phytium_device_info));
		np = dev_fwnode(&(pdev->dev));
		ret = fwnode_property_read_u8(np, "pipe_mask", &priv->info.pipe_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing pipe_mask property from acpi\n");
			goto failed;
		}
		ret = fwnode_property_read_u8(np, "edp_mask", &priv->info.edp_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing edp_mask property from acpi\n");
			goto failed;
		}
	}

	priv->info.num_pipes = 0;
	for_each_pipe_masked(priv, i)
		priv->info.num_pipes++;
	if (priv->info.num_pipes == 0) {
		DRM_ERROR("num_pipes is zero, so exit init\n");
		goto failed;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->regs = devm_ioremap_resource(&pdev->dev, res);
	if (priv->regs == NULL) {
		DRM_ERROR("ioremap fail, addr:0x%llx, size:0x%llx\n", res->start, res->end);
		goto failed;
	}

	priv->irq = platform_get_irq(pdev, 0);
	if (priv->irq < 0) {
		dev_err(&pdev->dev, "failed to get irq\n");
		goto failed;
	}

	if (IS_PE220X(priv)) {
		priv->dc_hw_clear_msi_irq = NULL;
		priv->dc_hw_fb_format_check = pe220x_dc_hw_fb_format_check;
	}

	return priv;

failed:
	devm_kfree(&pdev->dev, platform_priv);
exit:
	return NULL;
}

static void phytium_platform_private_fini(struct platform_device *pdev)
{
	struct drm_device *dev = dev_get_drvdata(&pdev->dev);
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_platform_private *platform_priv = to_platform_priv(priv);

	devm_kfree(&pdev->dev, platform_priv);
}

static int phytium_platform_probe(struct platform_device *pdev)
{
	struct phytium_display_private *priv = NULL;
	struct drm_device *dev = NULL;
	int ret = 0;

	dev = drm_dev_alloc(&phytium_display_drm_driver, &pdev->dev);
	if (IS_ERR(dev)) {
		DRM_ERROR("failed to allocate drm_device\n");
		return PTR_ERR(dev);
	}

	dev_set_drvdata(&pdev->dev, dev);
	dma_set_mask(&pdev->dev, DMA_BIT_MASK(40));

	priv = phytium_platform_private_init(pdev);
	if (priv)
		dev->dev_private = priv;
	else
		goto failed_platform_private_init;

	ret = phytium_platform_carveout_mem_init(pdev, priv);
	if (ret) {
		DRM_ERROR("failed to init system carveout memory\n");
		goto failed_carveout_mem_init;
	}

	ret = drm_dev_register(dev, 0);
	if (ret) {
		DRM_ERROR("failed to register drm dev\n");
		goto failed_register_drm;
	}

	phytium_dp_hpd_irq_setup(dev, true);

	return 0;

failed_register_drm:
	phytium_platform_carveout_mem_fini(pdev, priv);
failed_carveout_mem_init:
	phytium_platform_private_fini(pdev);
failed_platform_private_init:
	dev_set_drvdata(&pdev->dev, NULL);
	drm_dev_put(dev);
	return -1;
}

static int phytium_platform_remove(struct platform_device *pdev)
{
	struct drm_device *dev = dev_get_drvdata(&pdev->dev);
	struct phytium_display_private *priv = dev->dev_private;

	phytium_dp_hpd_irq_setup(dev, false);
	cancel_work_sync(&priv->hotplug_work);
	drm_dev_unregister(dev);
	phytium_platform_private_fini(pdev);
	dev_set_drvdata(&pdev->dev, NULL);
	drm_dev_put(dev);

	return 0;
}

static void phytium_platform_shutdown(struct platform_device *pdev)
{
	struct drm_device *dev = dev_get_drvdata(&pdev->dev);
	struct phytium_display_private *priv = dev->dev_private;

	priv->display_shutdown(dev);
}

static int phytium_platform_pm_suspend(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);
	struct phytium_display_private *priv = drm_dev->dev_private;

	return priv->display_pm_suspend(drm_dev);
}

static int phytium_platform_pm_resume(struct device *dev)
{
	struct drm_device *drm_dev = dev_get_drvdata(dev);
	struct phytium_display_private *priv = drm_dev->dev_private;

	return priv->display_pm_resume(drm_dev);
}

static const struct dev_pm_ops phytium_platform_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_platform_pm_suspend, phytium_platform_pm_resume)
};

static const struct phytium_device_info pe220x_info = {
	.platform_mask = BIT(PHYTIUM_PLATFORM_PE220X),
	.total_pipes = 2,
	.crtc_clock_max = PE220X_DC_PIX_CLOCK_MAX,
	.hdisplay_max = PE220X_DC_HDISPLAY_MAX,
	.vdisplay_max = PE220X_DC_VDISPLAY_MAX,
	.address_mask = PE220X_DC_ADDRESS_MASK,
	.backlight_max = PE220X_DP_BACKLIGHT_MAX,
};

static const struct of_device_id display_of_match[] = {
	{
		.compatible = "phytium,dc",
		.data = (void*)&pe220x_info,
	},
	{ }
};

MODULE_DEVICE_TABLE(of, display_of_match);

#ifdef CONFIG_ACPI
static const struct acpi_device_id display_acpi_ids[] = {
	{
		.id = "PHYT0015",
		.driver_data = (kernel_ulong_t)&pe220x_info,
	},
	{},
};

MODULE_DEVICE_TABLE(acpi, display_acpi_ids);
#else
#define display_acpi_ids NULL
#endif

struct platform_driver phytium_platform_driver = {
	.driver = {
		.name = "phytium_display_platform",
		.of_match_table = of_match_ptr(display_of_match),
		.acpi_match_table = ACPI_PTR(display_acpi_ids),
	},
	.probe = phytium_platform_probe,
	.remove = phytium_platform_remove,
	.shutdown = phytium_platform_shutdown,
	.driver.pm = &phytium_platform_pm_ops,
};
