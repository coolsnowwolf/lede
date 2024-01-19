// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_gem.h>
#include <drm/drm_vblank.h>
#include <drm/drm_irq.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_drv.h>
#include <linux/atomic.h>
#include <drm/drm_atomic.h>
#include <linux/workqueue.h>
#include <linux/pci.h>
#include "phytium_display_drv.h"
#include "phytium_plane.h"
#include "phytium_crtc.h"
#include "phytium_dp.h"
#include "phytium_gem.h"
#include "phytium_fb.h"
#include "phytium_fbdev.h"
#include "phytium_reg.h"
#include "phytium_pci.h"
#include "phytium_platform.h"
#include "phytium_debugfs.h"

int dc_fake_mode_enable;
module_param(dc_fake_mode_enable, int, 0644);
MODULE_PARM_DESC(dc_fake_mode_enable, "Enable DC fake mode (0-disabled; 1-enabled; default-0)");

int dc_fast_training_check = 1;
module_param(dc_fast_training_check, int, 0644);
MODULE_PARM_DESC(dc_fast_training_check, "Check dp fast training (0-disabled; 1-enabled; default-1)");

int num_source_rates = 4;
module_param(num_source_rates, int, 0644);
MODULE_PARM_DESC(num_source_rates, "set the source max rates (1-1.62Gbps; 2-2.7Gbps; 3-5.4Gbps; 4-8.1Gbps; default-4)");

int source_max_lane_count = 4;
module_param(source_max_lane_count, int, 0644);
MODULE_PARM_DESC(source_max_lane_count, "set the source lane count (1-1lane; 2-2lane; 4-4lane; default-4)");

int link_dynamic_adjust;
module_param(link_dynamic_adjust, int, 0644);
MODULE_PARM_DESC(link_dynamic_adjust, "dynamic select the train pamameter according to the display mode (0-disabled; 1-enabled; default-1)");

int phytium_wait_cmd_done(struct phytium_display_private *priv,
				    uint32_t register_offset,
				    uint32_t request_bit,
				    uint32_t reply_bit)
{
	int timeout = 500, config = 0, ret = 0;

	do {
		mdelay(1);
		timeout--;
		config = phytium_readl_reg(priv, 0, register_offset);
	} while ((!(config & reply_bit)) && timeout);

	phytium_writel_reg(priv, config & (~request_bit), 0, register_offset);

	if (timeout == 0) {
		DRM_ERROR("wait cmd reply timeout\n");
		ret = -EBUSY;
	} else {
		timeout = 500;
		do {
			mdelay(1);
			timeout--;
			config = phytium_readl_reg(priv, 0, register_offset);
		} while ((config & reply_bit) && timeout);
		if (timeout == 0) {
			DRM_ERROR("clear cmd timeout\n");
			ret = -EBUSY;
		}
	}
	mdelay(5);

	return ret;
}

static void phytium_irq_preinstall(struct drm_device *dev)
{
	struct phytium_display_private *priv = dev->dev_private;
	int i, status;

	for_each_pipe_masked(priv, i) {
		status = phytium_readl_reg(priv, priv->dc_reg_base[i], PHYTIUM_DC_INT_STATUS);
		phytium_writel_reg(priv, INT_DISABLE, priv->dc_reg_base[i], PHYTIUM_DC_INT_ENABLE);
	}
}

static void phytium_irq_uninstall(struct drm_device *dev)
{
	struct phytium_display_private *priv = dev->dev_private;
	int i, status;

	for_each_pipe_masked(priv, i) {
		status = phytium_readl_reg(priv, priv->dc_reg_base[i], PHYTIUM_DC_INT_STATUS);
		phytium_writel_reg(priv, INT_DISABLE, priv->dc_reg_base[i], PHYTIUM_DC_INT_ENABLE);
	}
}

static irqreturn_t phytium_display_irq_handler(int irq, void *data)
{
	struct drm_device *dev = data;
	struct phytium_display_private *priv = dev->dev_private;
	bool enabled = 0;
	int i = 0, virt_pipe = 0;
	irqreturn_t ret = IRQ_NONE, ret1 = IRQ_NONE;

	for_each_pipe_masked(priv, i) {
		enabled = phytium_readl_reg(priv, priv->dc_reg_base[i], PHYTIUM_DC_INT_STATUS);
		if (enabled & INT_STATUS) {
			virt_pipe = phytium_get_virt_pipe(priv, i);
			if (virt_pipe < 0)
				return IRQ_NONE;
			drm_handle_vblank(dev, virt_pipe);
			ret = IRQ_HANDLED;
			if (priv->dc_hw_clear_msi_irq)
				priv->dc_hw_clear_msi_irq(priv, i);
		}
	}

	ret1 = phytium_dp_hpd_irq_handler(priv);
	if (ret == IRQ_HANDLED || ret1 == IRQ_HANDLED)
		return IRQ_HANDLED;

	return IRQ_NONE;
}

static const struct drm_mode_config_funcs phytium_mode_funcs = {
	.fb_create		= phytium_fb_create,
	.output_poll_changed	= drm_fb_helper_output_poll_changed,
	.atomic_check		= drm_atomic_helper_check,
	.atomic_commit		= drm_atomic_helper_commit,
};

static void phytium_atomic_commit_tail(struct drm_atomic_state *state)
{
	struct drm_device *dev = state->dev;

	drm_atomic_helper_commit_modeset_disables(dev, state);
	drm_atomic_helper_commit_planes(dev, state, false);
	drm_atomic_helper_commit_modeset_enables(dev, state);
	drm_atomic_helper_commit_hw_done(state);
	drm_atomic_helper_wait_for_flip_done(dev, state);
	drm_atomic_helper_cleanup_planes(dev, state);
}

static struct drm_mode_config_helper_funcs phytium_mode_config_helpers = {
	.atomic_commit_tail = phytium_atomic_commit_tail,
};

static int phytium_modeset_init(struct drm_device *dev)
{
	struct phytium_display_private *priv = dev->dev_private;
	int i = 0, ret;

	drm_mode_config_init(dev);
	dev->mode_config.min_width = 0;
	dev->mode_config.min_height = 0;
	dev->mode_config.max_width = 16384;
	dev->mode_config.max_height = 16384;
	dev->mode_config.cursor_width = 32;
	dev->mode_config.cursor_height = 32;

	dev->mode_config.preferred_depth = 24;
	dev->mode_config.prefer_shadow = 1;
	dev->mode_config.allow_fb_modifiers = true;

	dev->mode_config.funcs	= &phytium_mode_funcs;
	dev->mode_config.helper_private = &phytium_mode_config_helpers;

	for_each_pipe_masked(priv, i) {
		ret = phytium_crtc_init(dev, i);
		if (ret) {
			DRM_ERROR("phytium_crtc_init(pipe %d) return failed\n", i);
			goto failed_crtc_init;
		}
	}

	for_each_pipe_masked(priv, i) {
		ret = phytium_dp_init(dev, i);
		if (ret) {
			DRM_ERROR("phytium_dp_init(pipe %d) return failed\n", i);
			goto failed_dp_init;
		}
	}

	drm_mode_config_reset(dev);

	return 0;
failed_dp_init:
failed_crtc_init:
	drm_mode_config_cleanup(dev);
	return ret;
}

int phytium_get_virt_pipe(struct phytium_display_private *priv, int phys_pipe)
{
	int i = 0;
	int virt_pipe = 0;

	for_each_pipe_masked(priv, i) {
		if (i != phys_pipe)
			virt_pipe++;
		else
			return virt_pipe;
	}

	DRM_ERROR("%s %d failed\n", __func__, phys_pipe);
	return -EINVAL;
}

int phytium_get_phys_pipe(struct phytium_display_private *priv, int virt_pipe)
{
	int i = 0;
	int tmp = 0;

	for_each_pipe_masked(priv, i) {
		if (tmp != virt_pipe)
			tmp++;
		else
			return i;
	}

	DRM_ERROR("%s %d failed\n", __func__, virt_pipe);
	return -EINVAL;
}

static int phytium_display_load(struct drm_device *dev, unsigned long flags)
{
	struct phytium_display_private *priv = dev->dev_private;
	int ret = 0;

	ret = drm_vblank_init(dev, priv->info.num_pipes);
	if (ret) {
		DRM_ERROR("vblank init failed\n");
		goto failed_vblank_init;
	}

	ret = phytium_modeset_init(dev);
	if (ret) {
		DRM_ERROR("phytium_modeset_init failed\n");
		goto failed_modeset_init;
	}

	if (priv->support_memory_type & MEMORY_TYPE_VRAM)
		priv->vram_hw_init(priv);

	ret = drm_irq_install(dev, priv->irq);
	if (ret) {
		DRM_ERROR("install irq failed\n");
		goto failed_irq_install;
	}

	ret = phytium_drm_fbdev_init(dev);
	if (ret)
		DRM_ERROR("failed to init dev\n");

	phytium_debugfs_display_register(priv);

	return ret;

failed_irq_install:
	drm_mode_config_cleanup(dev);
failed_modeset_init:
failed_vblank_init:
	return ret;
}

static void phytium_display_unload(struct drm_device *dev)
{
	phytium_drm_fbdev_fini(dev);
	drm_irq_uninstall(dev);
	drm_mode_config_cleanup(dev);
}

static const struct drm_ioctl_desc phytium_ioctls[] = {
	/* for test, none so far */
};

static const struct file_operations phytium_drm_driver_fops = {
	.owner		= THIS_MODULE,
	.open		= drm_open,
	.release	= drm_release,
	.unlocked_ioctl	= drm_ioctl,
	.compat_ioctl	= drm_compat_ioctl,
	.poll		= drm_poll,
	.read		= drm_read,
	.llseek		= no_llseek,
	.mmap		= phytium_gem_mmap,
};

struct drm_driver phytium_display_drm_driver = {
	.driver_features	= DRIVER_HAVE_IRQ   |
				  DRIVER_MODESET    |
				  DRIVER_ATOMIC     |
				  DRIVER_GEM,
	.load			= phytium_display_load,
	.unload			= phytium_display_unload,
	.lastclose		= drm_fb_helper_lastclose,
	.irq_handler		= phytium_display_irq_handler,
	.irq_preinstall		= phytium_irq_preinstall,
	.irq_uninstall		= phytium_irq_uninstall,
	.prime_handle_to_fd	= drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle	= drm_gem_prime_fd_to_handle,
	.gem_prime_export	= drm_gem_prime_export,
	.gem_prime_import	= drm_gem_prime_import,
	.gem_prime_import_sg_table = phytium_gem_prime_import_sg_table,
	.gem_prime_mmap		= phytium_gem_prime_mmap,
	.dumb_create		= phytium_gem_dumb_create,
	.dumb_destroy		= phytium_gem_dumb_destroy,
	.ioctls			= phytium_ioctls,
	.num_ioctls		= ARRAY_SIZE(phytium_ioctls),
	.fops			= &phytium_drm_driver_fops,
	.name			= DRV_NAME,
	.desc			= DRV_DESC,
	.date			= DRV_DATE,
	.major			= DRV_MAJOR,
	.minor			= DRV_MINOR,
};

static void phytium_display_shutdown(struct drm_device *dev)
{
	drm_atomic_helper_shutdown(dev);
}

static int phytium_display_pm_suspend(struct drm_device *dev)
{
	struct drm_atomic_state *state;
	struct phytium_display_private *priv = dev->dev_private;
	int ret, ret1;

	phytium_dp_hpd_irq_setup(dev, false);
	cancel_work_sync(&priv->hotplug_work);
	drm_fb_helper_set_suspend_unlocked(dev->fb_helper, 1);
	state = drm_atomic_helper_suspend(dev);
	if (IS_ERR(state)) {
		DRM_ERROR("drm_atomic_helper_suspend failed: %ld\n", PTR_ERR(state));
		ret = PTR_ERR(state);
		goto suspend_failed;
	}
	dev->mode_config.suspend_state = state;
	ret = phytium_gem_suspend(dev);
	if (ret) {
		DRM_ERROR("phytium_gem_suspend failed: %d\n", ret);
		goto gem_suspend_failed;
	}

	return 0;

gem_suspend_failed:
	ret1 = drm_atomic_helper_resume(dev, dev->mode_config.suspend_state);
	if (ret1)
		DRM_ERROR("Failed to resume (%d)\n", ret1);
	dev->mode_config.suspend_state = NULL;
suspend_failed:
	drm_fb_helper_set_suspend_unlocked(dev->fb_helper, 0);
	phytium_dp_hpd_irq_setup(dev, true);

	return ret;
}

static int phytium_display_pm_resume(struct drm_device *dev)
{
	struct phytium_display_private *priv = dev->dev_private;
	int ret = 0;

	if (WARN_ON(!dev->mode_config.suspend_state))
		return -EINVAL;

	ret = phytium_dp_resume(dev);
	if (ret)
		return -EIO;

	phytium_crtc_resume(dev);
	phytium_gem_resume(dev);

	if (priv->support_memory_type & MEMORY_TYPE_VRAM)
		priv->vram_hw_init(priv);

	ret = drm_atomic_helper_resume(dev, dev->mode_config.suspend_state);
	if (ret) {
		DRM_ERROR("Failed to resume (%d)\n", ret);
		return ret;
	}

	dev->mode_config.suspend_state = NULL;
	drm_fb_helper_set_suspend_unlocked(dev->fb_helper, 0);
	phytium_dp_hpd_irq_setup(dev, true);

	return 0;
}

void phytium_display_private_init(struct phytium_display_private *priv, struct drm_device *dev)
{
	INIT_LIST_HEAD(&priv->gem_list_head);
	spin_lock_init(&priv->hotplug_irq_lock);
	INIT_WORK(&priv->hotplug_work, phytium_dp_hpd_work_func);
	memset(priv->mem_state, 0, sizeof(priv->mem_state));
	priv->dev = dev;
	priv->display_shutdown = phytium_display_shutdown;
	priv->display_pm_suspend = phytium_display_pm_suspend;
	priv->display_pm_resume = phytium_display_pm_resume;
}

static int __init phytium_display_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&phytium_platform_driver);
	if (ret)
		return ret;

	ret = pci_register_driver(&phytium_pci_driver);

	return ret;
}

static void __exit phytium_display_exit(void)
{
	pci_unregister_driver(&phytium_pci_driver);

	platform_driver_unregister(&phytium_platform_driver);
}

module_init(phytium_display_init);
module_exit(phytium_display_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yang Xun <yangxun@phytium.com.cn>");
MODULE_AUTHOR("Shaojun Yang <yangshaojun@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium Display Controller");
