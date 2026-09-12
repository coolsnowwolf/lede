// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/component.h>
#include <linux/iommu.h>
#include <linux/of_graph.h>
#include <linux/version.h>
#include <linux/async.h>

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_of.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0))
#include <drm/drm_aperture.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
#include <drm/drm_probe_helper.h>
#endif
#include <drm/drm_atomic_helper.h>
#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#include <drm/drm_debugfs.h>
#include <drm/drm_drv.h>
#include <drm/drm_file.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_prime.h>
#include <drm/drm_vblank.h>

#ifdef CONFIG_PHYTIUM_PCIE
#include <linux/pci.h>
#endif
#endif
#ifdef CONFIG_PHYTIUM_FBDEV_ON
#include "phytium_fbdev.h"
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18))
#include <linux/acpi.h>
#endif

#include "ftd330_crtc.h"
#include "ftd330_dc.h"
#include "ftd330_drv.h"
#include "ftd330_fb.h"
#include "ftd330_gem.h"
#include "ftd330_plane.h"
#include "ftd330_simple_enc.h"
#include "ftd330_virtual.h"
#include "phytium_dp.h"
#ifdef CONFIG_PHYTIUM_QSPI
#include "ftd330_dc_qspi.h"
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
#include "ftd330_debug.h"
#endif
#define DRV_NAME "ftd330"
#define DRV_DESC "Phytium DRM driver"
#define DRV_DATE "20191101"
#define DRV_MAJOR 1
#define DRV_MINOR 0


//extern struct platform_driver phyitum_dplp_platform_driver;

/* pcie driver and platfrom driver common */

static bool has_iommu = true;
struct drm_device *dev_drm;

/*for interrupts destinations*/
/*"NS" is non-security, "TZ" is trust zone, "GSA" is G security, "AOC" is ambient on computing*/
static bool intr_dest_ns = true;
module_param_named(NS, intr_dest_ns, bool, 0644);
static bool intr_dest_tz;
module_param_named(TZ, intr_dest_tz, bool, 0644);
static bool intr_dest_gsa;
module_param_named(GSA, intr_dest_gsa, bool, 0644);
static bool intr_dest_aoc;
module_param_named(AOC, intr_dest_aoc, bool, 0644);

int dc_fake_mode_enable = 1;
module_param(dc_fake_mode_enable, int, 0644);
MODULE_PARM_DESC(dc_fake_mode_enable, "Enable DC fake mode (0-disabled; 1-enabled; default-0)");

uint intr_mask = 0xF0000;
module_param_named(MASK, intr_mask, uint, 0644);

unsigned long start_address;
module_param(start_address, ulong, 0400);

int dc_fast_training_check = 1;
module_param(dc_fast_training_check, int, 0644);
MODULE_PARM_DESC(dc_fast_training_check, "Check dp fast training (0-disabled; 1-enabled; default-1)");

int link_dynamic_adjust;
module_param(link_dynamic_adjust, int, 0644);
MODULE_PARM_DESC(link_dynamic_adjust, "dynamic select the train pamameter according to the display mode (0-disabled; 1-enabled; default-1)");


int phytium_dc_num = 0;
module_param(phytium_dc_num, int, 0400);

int source0_max_lane_count = 4;
module_param(source0_max_lane_count, int, 0644);
MODULE_PARM_DESC(source_max_lane_count, "set the source lane count (1-1lane; 2-2lane; 4-4lane; default-4)");

int source1_max_lane_count = 4;
module_param(source1_max_lane_count, int, 0644);
MODULE_PARM_DESC(source_max_lane_count, "set the source lane count (1-1lane; 2-2lane; 4-4lane; default-4)");

int source2_max_lane_count = 4;
module_param(source2_max_lane_count, int, 0644);
MODULE_PARM_DESC(source_max_lane_count, "set the source lane count (1-1lane; 2-2lane; 4-4lane; default-4)");

int num_source_rates = 4;
module_param(num_source_rates, int, 0644);
MODULE_PARM_DESC(num_source_rates, "set the source max rates (1-1.62Gbps; 2-2.7Gbps; 3-5.4Gbps; 4-8.1Gbps; default-4)");

int ftd330_phytium_dc_log_level = FTD330_PHYTIUM_DC_NO_LOG;

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
	.unlocked_ioctl = drm_ioctl,
	.compat_ioctl = drm_compat_ioctl,
	.poll = drm_poll,
	.read = drm_read,
	.mmap = ftd330_gem_mmap,
};

#ifdef CONFIG_DEBUG_FS
static int ftd330_debugfs_planes_show(struct seq_file *s, void *data)
{
	struct drm_info_node *node = (struct drm_info_node *)s->private;
	struct drm_device *dev = node->minor->dev;
	struct drm_plane *plane;

	list_for_each_entry(plane, &dev->mode_config.plane_list, head) {
		struct drm_plane_state *state = plane->state;
		struct ftd330_plane_state *plane_state = to_ftd330_plane_state(state);

		seq_printf(s, "plane[%u]: %s\n", plane->base.id, plane->name);
		seq_printf(s, "\tcrtc = %s\n", state->crtc ? state->crtc->name : "(null)");
		seq_printf(s, "\tcrtc id = %u\n", state->crtc ? state->crtc->base.id : 0);
		seq_printf(s, "\tcrtc-pos = " DRM_RECT_FMT "\n",
			   DRM_RECT_ARG(&plane_state->status.dest));
		seq_printf(s, "\tsrc-pos = " DRM_RECT_FP_FMT "\n",
			   DRM_RECT_FP_ARG(&plane_state->status.src));
#if KERNEL_VERSION(5, 14, 0) <= LINUX_VERSION_CODE
		seq_printf(s, "\tformat = %p4cc\n", &state->fb->format->format);
#else
		seq_printf(s, "\tformat = %s\n",
			   state->fb ? plane_state->status.format_name.str : "(null)");
#endif
		seq_printf(s, "\trotation = 0x%x\n", state->rotation);
		seq_printf(s, "\ttiling = %u\n", plane_state->status.tile_mode);

		seq_puts(s, "\n");
	}

	return 0;
}

static struct drm_info_list ftd330_debugfs_list[] = {
	{ "planes", ftd330_debugfs_planes_show, 0, NULL },
};

#if KERNEL_VERSION(5, 8, 0) <= LINUX_VERSION_CODE
static void ftd330_debugfs_init(struct drm_minor *minor)
{
	drm_debugfs_create_files(ftd330_debugfs_list, ARRAY_SIZE(ftd330_debugfs_list), minor->debugfs_root,
				 minor);
}
#else
static int ftd330_debugfs_init(struct drm_minor *minor)
{
	struct drm_device *dev = minor->dev;
	int ret;

	ret = drm_debugfs_create_files(ftd330_debugfs_list, ARRAY_SIZE(ftd330_debugfs_list),
				       minor->debugfs_root, minor);
	if (ret)
		dev_err(dev->dev, "could not install ftd330_debugfs_list.\n");

	return ret;
}
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
static const struct vm_operations_struct ftd330_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};
#endif

static const struct drm_ioctl_desc ftd330_ioctls[] = {
	DRM_IOCTL_DEF_DRV(FTD330_GET_FBC_OFFSET, ftd330_get_fbc_offset_ioctl, DRM_MASTER),
	DRM_IOCTL_DEF_DRV(FTD330_SW_RESET, ftd330_sw_reset_ioctl, DRM_MASTER),
	DRM_IOCTL_DEF_DRV(FTD330_GEM_QUERY, ftd330_gem_query_ioctl, DRM_MASTER),
	DRM_IOCTL_DEF_DRV(FTD330_FB_GET, ftd330_display_getfb2_ioctl, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(FTD330_GET_FEATURE_CAP, ftd330_get_feature_cap_ioctl, DRM_MASTER),
	DRM_IOCTL_DEF_DRV(FTD330_GET_HIST_INFO, ftd330_get_hist_info_ioctl, DRM_MASTER),
	DRM_IOCTL_DEF_DRV(FTD330_GET_WB_FRM_DONE, ftd330_get_wb_frm_done_ioctl, DRM_MASTER),
};

static struct drm_driver ftd330_drm_driver = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0)
     	.driver_features = DRIVER_MODESET | DRIVER_ATOMIC | DRIVER_GEM | DRIVER_PRIME,
#else
	.driver_features = DRIVER_MODESET | DRIVER_ATOMIC | DRIVER_GEM,
#endif
	.lastclose = drm_fb_helper_lastclose,
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
	.gem_free_object	= ftd330_gem_free_object,
	.gem_vm_ops		= &ftd330_vm_ops,
	.gem_prime_get_sg_table	= ftd330_gem_prime_get_sg_table,
	.gem_prime_vmap		= ftd330_gem_prime_vmap,
	.gem_prime_vunmap	= ftd330_gem_prime_vunmap,
#endif
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	.gem_prime_export	= drm_gem_prime_export,
#endif
	.gem_prime_import = ftd330_gem_prime_import,
	.gem_prime_import_sg_table = ftd330_gem_prime_import_sg_table,
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	.gem_prime_mmap = ftd330_gem_prime_mmap,
#endif
	.dumb_create = ftd330_gem_dumb_create,
#if KERNEL_VERSION(5, 7, 0) > LINUX_VERSION_CODE
	//.get_vblank_timestamp = drm_calc_vbltimestamp_from_scanoutpos,
	//.get_scanout_position = ftd330_display_get_crtc_scanoutpos,
#endif
#ifdef CONFIG_DEBUG_FS
	.debugfs_init = ftd330_debugfs_init,
#endif
	.ioctls = ftd330_ioctls,
	.num_ioctls = ARRAY_SIZE(ftd330_ioctls),
	.fops = &fops,
	.name = DRV_NAME,
	.desc = DRV_DESC,
	.date = DRV_DATE,
	.major = DRV_MAJOR,
	.minor = DRV_MINOR,
};


void phytium_update_timing_for_drm_display_mode(struct drm_display_mode *drm_mode,
		const struct drm_display_mode *native_mode)
{
	if (native_mode->clock == drm_mode->clock &&
		native_mode->htotal == drm_mode->htotal &&
		native_mode->vtotal == drm_mode->vtotal) {
		drm_mode->crtc_hdisplay = native_mode->crtc_hdisplay;
		drm_mode->crtc_vdisplay = native_mode->crtc_vdisplay;
		drm_mode->crtc_clock = native_mode->crtc_clock;
		drm_mode->crtc_hblank_start = native_mode->crtc_hblank_start;
		drm_mode->crtc_hblank_end = native_mode->crtc_hblank_end;
		drm_mode->crtc_hsync_start =  native_mode->crtc_hsync_start;
		drm_mode->crtc_hsync_end = native_mode->crtc_hsync_end;
		drm_mode->crtc_htotal = native_mode->crtc_htotal;
		drm_mode->crtc_hskew = native_mode->crtc_hskew;
		drm_mode->crtc_vblank_start = native_mode->crtc_vblank_start;
		drm_mode->crtc_vblank_end = native_mode->crtc_vblank_end;
		drm_mode->crtc_vsync_start = native_mode->crtc_vsync_start;
		drm_mode->crtc_vsync_end = native_mode->crtc_vsync_end;
		drm_mode->crtc_vtotal = native_mode->crtc_vtotal;
	}
}


int ftd330_drm_iommu_attach_device(struct drm_device *drm_dev, struct device *dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	int ret;

	if (!has_iommu)
		return 0;

	if (!priv->domain) {
		priv->domain = iommu_get_domain_for_dev(dev);
		if (IS_ERR(priv->domain))
			return PTR_ERR(priv->domain);
		priv->dma_dev = dev;
	}

	ret = iommu_attach_device(priv->domain, dev);
	if (ret) {
		DRM_DEV_ERROR(dev, "Failed to attach iommu device\n");
		return ret;
	}

	return 0;
}

void ftd330_drm_iommu_detach_device(struct drm_device *drm_dev, struct device *dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;

	if (!has_iommu)
		return;

	iommu_detach_device(priv->domain, dev);

	if (priv->dma_dev == dev)
		priv->dma_dev = drm_dev->dev;
}

void ftd330_drm_update_alignment(struct drm_device *drm_dev, unsigned int pitch_align,
			     unsigned int addr_align)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;

		priv->pitch_alignment = pitch_align;

	if (addr_align > priv->addr_alignment)
		priv->addr_alignment = addr_align;
}

static int phytium_remove_conflicting_framebuffer(struct ftd330_drm_private *priv) {
	resource_size_t base,size;
	u32 framebuffer_address = 0;
	u32 framebuffer_size = 0;
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 20, 0)
	struct apertures_struct *ap;

	ap = alloc_apertures(1);
	if (!ap)
		return -ENOMEM;
#endif

	framebuffer_address = phytium_get_efifb_address(&dc->hw, DC_FRAMEBUFFER_ADDRESS);
	if (framebuffer_address == 0) {
		pr_info("PHYTIUM get efifb address fail,no efifb remove\n");
		return 0;
	}
/*if we want to get the presice framebuffer size,we need to read more registers here.but
 * framebuffer size can be larger than the actual size to make sure that the efifb can be removed
 * so we set 128M to make sure that efifb framebuffer are completely covered.
*/
	framebuffer_size = 128*1024*1024;
	base = (resource_size_t)framebuffer_address;
	size = (resource_size_t)framebuffer_size;

	if (base == 0 || size == 0) {
		DRM_ERROR("bad physical start address\n");
		return -1;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 20, 0)
	return drm_aperture_remove_conflicting_framebuffers(base,
					size,&ftd330_drm_driver);
#else
	ap->ranges[0].base = base;
	ap->ranges[0].size = size;
	drm_fb_helper_remove_conflicting_framebuffers(ap, "phytiumdrmfb", false);
	kfree(ap);
	return 0;
#endif
}

static int phytium_gem_suspend(struct drm_device *drm_dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_gem_object *phytium_gem_obj = NULL;
	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		phytium_gem_obj->vaddr_save = vmalloc(phytium_gem_obj->size);
		if (!phytium_gem_obj->vaddr_save)
			goto malloc_failed;
		memcpy(phytium_gem_obj->vaddr_save, phytium_gem_obj->cookie, phytium_gem_obj->size);
	}
	return 0;
malloc_failed:
	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		if (phytium_gem_obj->vaddr_save) {
			vfree(phytium_gem_obj->vaddr_save);
			phytium_gem_obj->vaddr_save = NULL;
		}
	}
	return -ENOMEM;
}
static void phytium_gem_resume(struct drm_device *drm_dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_gem_object *phytium_gem_obj = NULL;
	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		memcpy(phytium_gem_obj->cookie, phytium_gem_obj->vaddr_save, phytium_gem_obj->size);
		vfree(phytium_gem_obj->vaddr_save);
		phytium_gem_obj->vaddr_save = NULL;
	}
}
#ifdef CONFIG_PM_SLEEP

#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
static int ftd330_drm_mode_config_helper_suspend(struct drm_device *dev)
{
        struct drm_atomic_state *state;

        if (!dev)
                return 0;

        drm_fb_helper_set_suspend_unlocked(dev->fb_helper, 1);
        state = drm_atomic_helper_suspend(dev);
        if (IS_ERR(state)) {
                drm_fb_helper_set_suspend_unlocked(dev->fb_helper, 0);
                drm_kms_helper_poll_enable(dev);
                return PTR_ERR(state);
        }

        dev->mode_config.suspend_state = state;

        return 0;
}
#endif

static void phytium_fbdev_async_task(void *data, async_cookie_t cookie)
{
	struct ftd330_drm_private *priv = (struct ftd330_drm_private *)data;
        struct drm_device *drm_dev = priv->drm_dev;
        int i = 0;

        /*we need to do panel power off first to prevent abnormal display*/
        for(i = DISPLAY_0;i < DISPLAY_NUM;i++) {
                if (priv->phytium_dp[i] && priv->phytium_dp[i]->is_edp) {
                        phytium_panel_poweroff(&priv->phytium_dp[i]->panel);
                }
        }

#ifdef CONFIG_PHYTIUM_FBDEV_ON
        phytium_drm_fbdev_init(priv);
#endif
        phytium_dplp_init(priv);
        phytium_dp_hpd_irq_setup(drm_dev, true, true);
}


static int ftd330_drm_suspend(struct device *dev)
{
	int ret = 0;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm = dc->hw.drm_dev;
	struct ftd330_drm_private *priv = drm->dev_private;
	struct device *dc_dev = priv->dc_dev;
	int i = 0;

	phytium_dp_hpd_irq_setup(drm, false, false);
	phytium_dplp_deinit(priv);
	cancel_work_sync(&priv->hotplug_work);
	phytium_gem_suspend(drm);
#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
        ret = ftd330_drm_mode_config_helper_suspend(drm);
#else
	ret = drm_mode_config_helper_suspend(drm);
#endif
	if (ret < 0) {
		DRM_ERROR("failed to config helper suspend.\n");
		goto err_ret;
	}

	ret = phytium_dp_suspend(drm);

	ret = ftd330_dc_suspend(dc_dev);
	if (ret < 0) {
		DRM_ERROR("failed to phy dc suspend.\n");
		goto err_ret;
	}

#ifdef CONFIG_PHYTIUM_POWER_OPERATION
	for (i = DISPLAY_0;i < DISPLAY_NUM;i++) {
		if (priv->info.pipe_mask & BIT(i)) {
			phytium_display_power_request_off(drm, i);
		}
	}
#endif

err_ret:
	return ret;
}

static int ftd330_do_resume(struct device *dev, bool is_s3)
{
	int ret = 0;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm = dc->hw.drm_dev;
	struct ftd330_drm_private *priv = drm->dev_private;
	struct device *dc_dev = priv->dc_dev;

	if (!is_s3)
		phytium_gem_resume(drm);

#ifdef CONFIG_PHYTIUM_POWER_OPERATION
	if (is_s3) {
		if (priv->info.edp_s3_need_poweron) {
			if (priv->power_status_save[DISPLAY_0]) {
				phytium_display_power_request(priv, false, DISPLAY_0);
				mdelay(3);
				phytium_display_power_request(priv, true, DISPLAY_0);
			}
			if (priv->power_status_save[DISPLAY_1] || priv->power_status_save[DISPLAY_2]) {
				phytium_display_power_request(priv, false, DISPLAY_1);
				mdelay(3);
				phytium_display_power_request(priv, true, DISPLAY_1);
			}
		} else {
			if (priv->phytium_dp[DISPLAY_0]->is_edp) {
				if (priv->power_status_save[DISPLAY_1] || priv->power_status_save[DISPLAY_2]) {
					phytium_display_power_request(priv, false, DISPLAY_1);
					mdelay(3);
					phytium_display_power_request(priv, true, DISPLAY_1);
				}
			} else {
				if (priv->power_status_save[DISPLAY_0]) {
					phytium_display_power_request(priv, false, DISPLAY_0);
					mdelay(3);
					phytium_display_power_request(priv, true, DISPLAY_0);
				}
			}
		}
	}

	if (!is_s3) {
		if (priv->info.edp_s4_need_poweron) {
			if (priv->power_status_save[DISPLAY_0]) {
				phytium_display_power_request(priv, false, DISPLAY_0);
				mdelay(3);
				phytium_display_power_request(priv, true, DISPLAY_0);
			}
			if (priv->power_status_save[DISPLAY_1] || priv->power_status_save[DISPLAY_2]) {
				phytium_display_power_request(priv, false, DISPLAY_1);
				mdelay(3);
				phytium_display_power_request(priv, true, DISPLAY_1);
			}
		} else {
			if (priv->phytium_dp[DISPLAY_0]->is_edp) {
				if (priv->power_status_save[DISPLAY_1] || priv->power_status_save[DISPLAY_2]) {
					phytium_display_power_request(priv, false, DISPLAY_1);
					mdelay(3);
					phytium_display_power_request(priv, true, DISPLAY_1);
				}
			} else {
				if (priv->power_status_save[DISPLAY_0]) {
					phytium_display_power_request(priv, false, DISPLAY_0);
					mdelay(3);
					phytium_display_power_request(priv, true, DISPLAY_0);
				}
			}
		}
	}

#endif
	mdelay(3);
	ret = ftd330_dc_resume(dc_dev);
	if (ret < 0) {
		DRM_ERROR("failed to do dc resume.\n");
		goto err_ret;
	}
	ret = phytium_dp_resume(drm);

	ret = drm_mode_config_helper_resume(drm);
	if (ret < 0) {
		DRM_ERROR("failed to config helper resume.\n");
		goto err_ret;
	}
	phytium_dplp_init(priv);
	phytium_dp_hpd_irq_setup(drm, true, true);

err_ret:
	return ret;
}

static int ftd330_drm_resume(struct device *dev)
{
	bool is_s3 = 1;

	return ftd330_do_resume(dev, is_s3);
}

static int ftd330_drm_thaw(struct device *dev)
{
	bool is_s3 = 0;

	return ftd330_do_resume(dev, is_s3);
}

static int ftd330_drm_restore(struct device *dev)
{
	bool is_s3 = 0;

	return ftd330_do_resume(dev, is_s3);
}

static const struct dev_pm_ops ftd330_drm_pm_ops = {
	.suspend = ftd330_drm_suspend,
	.resume = ftd330_drm_resume,
	.freeze = ftd330_drm_suspend,
	.thaw = ftd330_drm_thaw,
	.poweroff = ftd330_drm_suspend,
	.restore = ftd330_drm_restore,
};
#endif

#ifdef CONFIG_PHYTIUM_PCIE
static const struct phytium_device_info ftd330_info = {
	.total_pipes = 3,
	.pipe_mask = 0x07,
	.edp_mask = 0x01,
	.water_mark = {0x5666, 0x5666, 0x5666},
	.qos = {0xf0, 0xf0},
	.crtc_clock_max = 0,
	.hdisplay_max = 0,
	.vdisplay_max = 0,
	.backlight_max = 0,
	.backlight_min = 0,
	.address_mask = 0,
	.pwm_periodns = 1000000,
	.pwm_clk_rate = 100000000,
	.pwm_div = 0x3E7,
};

/* pcie driver */
static struct pci_device_id ftd330_pci_table[] = {
					{
				       PCI_DEVICE(0x10ee, 0x7012),
				       .class = 0,
				       .class_mask = 0,
				       .driver_data = (kernel_ulong_t)&ftd330_info,
					},
					{
						PCI_DEVICE(0x10ee, 0x8011),
						.class = 0,
						.class_mask = 0,
						.driver_data = (kernel_ulong_t)&ftd330_info,
					},
};


static int ftd330_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pent)
{
	struct drm_device *drm_dev;
	struct ftd330_drm_private *priv;
	int ret;
	int i = 0;
	struct phytium_device_info *phytium_info = (struct phytium_device_info *)pent->driver_data;

	drm_dev = drm_dev_alloc(&ftd330_drm_driver, &pdev->dev);
	if (IS_ERR(drm_dev)) {
		DRM_ERROR("failed to allocate drm device\n");
		return PTR_ERR(drm_dev);
	}

	ret = pci_enable_device(pdev);
	if (ret) {
		DRM_ERROR("fail to enable PCI device: %d\n", ret);
		goto err_out;
	}

	pci_set_master(pdev);

#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	drm_dev->pdev = pdev;
#endif
	dev_drm = drm_dev;

	priv = devm_kzalloc(drm_dev->dev, sizeof(struct ftd330_drm_private), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto err_put;
	}

	priv->intr_dest = ((intr_dest_ns << 0) | (intr_dest_tz << 1) | (intr_dest_gsa << 2) |
			   (intr_dest_aoc << 3));
	priv->intr_mask = intr_mask;
	priv->addr_alignment = 128;
	priv->dma_dev = drm_dev->dev;
	priv->pdev = pdev;
	drm_dev->dev_private = priv;

	memcpy(&priv->info, phytium_info, sizeof(struct phytium_device_info));

	if (phytium_dc_num != 0) {
		priv->info.pipe_mask = phytium_dc_num;
	}

	priv->info.total_pipes = 0;
	for (i = DISPLAY_0;i < DISPLAY_NUM ; i++)
	    if (BIT(i) & priv->info.pipe_mask)
		priv->info.total_pipes++;
        dc->hw.total_pipes = priv->info.total_pipes;
        dc->hw.pipe_mask =  priv->info.pipe_mask;
	dc->hw.overlay_enable = priv->info.overlay_enable;
	pr_info("PHY total_pipes = %d,pipe_mask = %d\n", priv->info.total_pipes,
			priv->info.pipe_mask);


	drm_mode_config_init(drm_dev);

	/* todo. */
	has_iommu = false;

	ret = phytium_drm_device_init(drm_dev);
	if (ret) {
		DRM_ERROR("fail to init drm device: %d\n", ret);
		goto err_mode;
	}

	ftd330_mode_config_init(drm_dev);

	ret = drm_vblank_init(drm_dev, drm_dev->mode_config.num_crtc);
	if (ret) {
		DRM_ERROR("fail to init drm vblank: %d\n", ret);
		goto err_deinit;
	}

	drm_mode_config_reset(drm_dev);

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	drm_dev->irq_enabled = drm_dev->num_crtcs != 0;
#else
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	drm_dev->irq_enabled = drm_dev->num_crtcs != 0;
#endif
#endif

	ret = drm_dev_register(drm_dev, 0);
	if (ret) {
		DRM_ERROR("fail to register drm device: %d\n", ret);
		goto err_helper;
	}

	return 0;

err_helper:
err_deinit:
	phytium_drm_device_deinit(drm_dev);

	if (priv->domain)
		iommu_domain_free(priv->domain);

err_mode:
	drm_mode_config_cleanup(drm_dev);
err_put:
	pci_disable_device(pdev);
err_out:
	drm_dev_put(drm_dev);
	drm_dev->dev_private = NULL;
	pci_set_drvdata(pdev, NULL);

	return ret;
}

static void ftd330_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = dev_drm;

	drm_dev_unregister(dev);

	phytium_drm_device_deinit(dev);

	drm_atomic_helper_shutdown(dev);
	drm_mode_config_cleanup(dev);

	/* free IRQ */
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	free_irq(dev->pdev->irq, dev_get_drvdata(dev->dev));
#else
	free_irq(pdev->irq, dev_get_drvdata(dev->dev));
#endif

	pci_set_drvdata(pdev, NULL);

	pci_disable_device(pdev);

	drm_dev_put(dev);
}

static void phytium_pci_shutdown(struct pci_dev *pdev)
{
	struct device *dev = &pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm_dev = dc->hw.drm_dev;

	 drm_atomic_helper_shutdown(drm_dev);
}

struct pci_driver phytium_pci_driver = {
	.name = DRV_NAME,
	.id_table = ftd330_pci_table,
	.probe = ftd330_pci_probe,
	.remove = ftd330_pci_remove,
	.shutdown = phytium_pci_shutdown,
	.driver = {
		.pm = &ftd330_drm_pm_ops,
	},
};


static int __init ftd330_drm_init(void)
{
	int ret = pci_register_driver(&phytium_pci_driver);

	return ret;
}

static void __exit ftd330_drm_exit(void)
{
	pci_unregister_driver(&phytium_pci_driver);
}

#else

/* platfrom driver */
static int ftd330_drm_platform_probe(struct platform_device *pdev)
{
	struct drm_device *drm_dev;
	struct ftd330_drm_private *priv;
	struct device *dev = &pdev->dev;
	int ret;

	drm_dev = drm_dev_alloc(&ftd330_drm_driver, &pdev->dev);
	if (IS_ERR(drm_dev)) {
		DRM_ERROR("failed to allocate drm device\n");
		return PTR_ERR(drm_dev);
	}

	dev_drm = drm_dev;

	priv = devm_kzalloc(drm_dev->dev, sizeof(struct ftd330_drm_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->pitch_alignment = 64;
	priv->addr_alignment = 128;
	priv->dma_dev = drm_dev->dev;
	priv->pdev = pdev;
	priv->drm_dev = drm_dev;

	drm_dev->dev_private = priv;

	drm_mode_config_init(drm_dev);

	/* todo. */
	has_iommu = false;

	ret = phytium_drm_device_init(drm_dev);
	if (ret) {
		DRM_ERROR("fail to init drm device: %d\n", ret);
		goto err_mode;
	}
	
	ret = phytium_remove_conflicting_framebuffer(priv);
	if (ret) {
		DRM_ERROR("fail to remove conflicting efifb ret = %d\n", ret);
		goto err_mode;
	}

#ifdef CONFIG_PHYTIUM_POWER_OPERATION
	if(priv->info.edp_mask & BIT(DISPLAY_0)) {
		phytium_display_power_request(priv, true, DISPLAY_0);
	} else {
		phytium_display_power_request(priv, false, DISPLAY_0);
	}

	if (priv->info.edp_mask & BIT(DISPLAY_1) || priv->info.edp_mask & BIT(DISPLAY_2)) {
		phytium_display_power_request(priv, true, DISPLAY_1);
	} else {
		phytium_display_power_request(priv, false, DISPLAY_1);
	}
#endif

	if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
		phytium_dc_registers_init(priv, DC_0);
	}
	if (priv->info.pipe_mask & BIT(DISPLAY_1) || priv->info.pipe_mask & BIT(DISPLAY_2)) {
		phytium_dc_registers_init(priv, DC_1);
	}

	ftd330_mode_config_init(drm_dev);

#ifdef CONFIG_PHYTIUM_DEBUG
	ret = ftd330_debug_file_create(&priv->dc_capture_fp);
	if (ret) {
		DRM_ERROR("fail to create the phy debug file: %d\n", ret);
		goto err_put_dev;
	}
#endif

	ret = drm_vblank_init(drm_dev, drm_dev->mode_config.num_crtc);
	if (ret)
		goto err_bind;

	drm_mode_config_reset(drm_dev);

#if KERNEL_VERSION(5, 15, 0) > LINUX_VERSION_CODE
	drm_dev->irq_enabled = drm_dev->num_crtcs != 0;
#else
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	drm_dev->irq_enabled = drm_dev->num_crtcs != 0;
#endif
#endif

	INIT_LIST_HEAD(&priv->gem_list_head);
	INIT_WORK(&priv->hotplug_work, phytium_dp_hpd_work_func);
#ifdef CONFIG_PHYTIUM_NCC
	ret = phytium_mem_pool_init(drm_dev);
	if (ret) {
		DRM_ERROR("PHY mem pool init failed\n");
		goto err_mem_pool;
	}
#endif

	ret = drm_dev_register(drm_dev, 0);
	if (ret)
		goto err_helper;

        async_schedule(phytium_fbdev_async_task, priv);

	return 0;

#ifdef CONFIG_PHYTIUM_NCC
err_mem_pool:
	phytium_mem_pool_deinit(drm_dev);
#endif
err_helper:
err_bind:
	component_unbind_all(drm_dev->dev, drm_dev);
err_mode:
	drm_mode_config_cleanup(drm_dev);

	if (priv->domain)
		iommu_domain_free(priv->domain);

	drm_dev->dev_private = NULL;
	dev_set_drvdata(dev, NULL);
	drm_dev_put(drm_dev);

	return ret;
}

static int ftd330_drm_platform_remove(struct platform_device *pdev)
{
	struct drm_device *dev = dev_drm;
	struct ftd330_drm_private *priv = dev->dev_private;

#ifdef CONFIG_PHYTIUM_FBDEV_ON
        phytium_drm_fbdev_fini(dev);
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
	if (priv->dc_capture_fp)
		ftd330_debug_file_close(&priv->dc_capture_fp);
#endif
	drm_dev_unregister(dev);

	drm_atomic_helper_shutdown(dev);

	phytium_drm_device_deinit(dev);

	drm_mode_config_cleanup(dev);

	/* free IRQ */
	if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
		free_irq(platform_get_irq(pdev, 0), dev_get_drvdata(dev->dev));
	} 

	if (priv->info.pipe_mask & BIT(DISPLAY_1) || priv->info.pipe_mask & BIT(DISPLAY_2)) {
		free_irq(platform_get_irq(pdev, 1), dev_get_drvdata(dev->dev));
	}


#ifdef CONFIG_PHYTIUM_NCC
         phytium_mem_pool_deinit(dev);
#endif

	dev->dev_private = NULL;
	dev_set_drvdata(&pdev->dev, NULL);
	drm_dev_put(dev);

	return 0;
}


static void phytium_platform_shutdown(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm_dev = dc->hw.drm_dev;

	 drm_atomic_helper_shutdown(drm_dev);
}

static const struct of_device_id ftd330_drm_dt_ids[] = {

	{
		.compatible = "phytium,dc-1.0",
	},

	{ /* sentinel */ },

};

MODULE_DEVICE_TABLE(of, ftd330_drm_dt_ids);

#ifdef CONFIG_ACPI
static const struct acpi_device_id dc_display_acpi_ids[] = {
	{
		.id = "PHYT0057",
	},
	{},
};

MODULE_DEVICE_TABLE(acpi, dc_display_acpi_ids);
#else
#define dc_display_acpi_ids NULL
#endif

static struct platform_driver ftd330_drm_platform_driver = {
	.probe = ftd330_drm_platform_probe,
	.remove = ftd330_drm_platform_remove,
	.shutdown = phytium_platform_shutdown,

	.driver = {
		.name = DRV_NAME,
		.of_match_table = ftd330_drm_dt_ids,
		.acpi_match_table = ACPI_PTR(dc_display_acpi_ids),
		.pm = &ftd330_drm_pm_ops,
	},
};

static int __init ftd330_drm_init(void)
{
	int ret;

	ret = platform_driver_register(&ftd330_drm_platform_driver);

	return ret;
}

static void __exit ftd330_drm_exit(void)
{
	platform_driver_unregister(&ftd330_drm_platform_driver);
}
#endif

module_init(ftd330_drm_init);
module_exit(ftd330_drm_exit);

MODULE_DESCRIPTION("Phytium DRM Driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DC_DRIVER_VERSION);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
MODULE_IMPORT_NS(DMA_BUF);
#endif
