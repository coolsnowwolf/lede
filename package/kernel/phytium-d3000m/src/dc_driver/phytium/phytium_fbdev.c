// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#include <drm/drm.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_fourcc.h>
#include "drm/drm_framebuffer.h"
#include "ftd330_gem.h"
#include "ftd330_fb.h"
#include "phytium_fbdev.h"
#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
#include <drm/drm_fbdev_generic.h>
#endif

#define	PHYTIUM_MAX_CONNECTOR	1
#define	helper_to_drm_private(x) container_of(x, struct ftd330_drm_private, fbdev_helper)

#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int phytium_fbdev_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	struct drm_fb_helper *helper = info->par;
	struct ftd330_drm_private *priv = helper_to_drm_private(helper);

	return ftd330_gem_mmap_obj(&priv->fbdev_phytium_gem->base, vma);
}

static struct fb_ops phytium_fbdev_ops = {
	.owner = THIS_MODULE,
	 DRM_FB_HELPER_DEFAULT_OPS,
	.fb_mmap = phytium_fbdev_mmap,
	.fb_fillrect = drm_fb_helper_cfb_fillrect,
	.fb_copyarea = drm_fb_helper_cfb_copyarea,
	.fb_imageblit = drm_fb_helper_cfb_imageblit,
};

static int
phytium_drm_fbdev_create(struct drm_fb_helper *helper, struct drm_fb_helper_surface_size *sizes)
{
	struct ftd330_drm_private *priv = helper_to_drm_private(helper);
	struct drm_device *dev = helper->dev;
	unsigned int bytes_per_pixel;
	struct drm_mode_fb_cmd2	mode_cmd = {0};
	//struct phytium_framebuffer *phytium_fb =  NULL;
	struct drm_framebuffer *phytium_fb =  NULL;
	struct fb_info *fbi =  NULL;
	struct drm_framebuffer *fb = NULL;
	size_t size = 0;
	int ret = 0;
	unsigned long offset;

	bytes_per_pixel = DIV_ROUND_UP(sizes->surface_bpp, 8);
	mode_cmd.width = sizes->surface_width;
	mode_cmd.height = sizes->surface_height;
	mode_cmd.pitches[0] =  ALIGN(sizes->surface_width * bytes_per_pixel, 128);
	mode_cmd.pixel_format = drm_mode_legacy_fb_format(sizes->surface_bpp, sizes->surface_depth);
	size = PAGE_ALIGN(mode_cmd.pitches[0] * mode_cmd.height);

	ret = mutex_lock_interruptible(&dev->struct_mutex);
	if (ret < 0) {
		DRM_ERROR("failed to get mutex lock\n");
		return ret;
	}

	//priv->fbdev_phytium_gem = phytium_gem_create_object(dev, size);
	priv->fbdev_phytium_gem = ftd330_gem_create_object(dev, size);
	if (!priv->fbdev_phytium_gem) {
		DRM_ERROR("failed to create gem object\n");
		return -ENOMEM;
	}
	mutex_unlock(&dev->struct_mutex);

	fbi = drm_fb_helper_alloc_fbi(helper);
	if (IS_ERR(fbi)) {
		DRM_DEV_ERROR(dev->dev, "Failed to create framebuffer info.");
		ret = PTR_ERR(fbi);
		goto out;
	}

	phytium_fb = ftd330_fb_alloc(dev, &mode_cmd,  &priv->fbdev_phytium_gem, 1);
	if (IS_ERR(phytium_fb)) {
		DRM_DEV_ERROR(dev->dev, "Failed to alloc DRM framebuffer.\n");
		ret = PTR_ERR(phytium_fb);
		goto out;
	}

	helper->fb = phytium_fb;
	fbi->par = helper;
	fbi->flags = FBINFO_FLAG_DEFAULT;
	fbi->fbops = &phytium_fbdev_ops;

	fb = helper->fb;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	drm_fb_helper_fill_info(fbi, helper, sizes);
#else
	drm_fb_helper_fill_fix(fbi, fb->pitches[0], fb->format->depth);
	drm_fb_helper_fill_var(fbi, helper, sizes->fb_width, sizes->fb_height);
#endif
	offset = fbi->var.xoffset * bytes_per_pixel;
	offset += fbi->var.yoffset * fb->pitches[0];
	dev->mode_config.fb_base = 0;
	fbi->screen_base = priv->fbdev_phytium_gem->cookie + offset;
	fbi->screen_size = priv->fbdev_phytium_gem->base.size;
	fbi->fix.smem_len = priv->fbdev_phytium_gem->base.size;
	DRM_DEBUG_KMS("FB [%dx%d]-%d kvaddr=%pa offset=%ld size=%zu\n", fb->width, fb->height,
		       fb->format->depth, &priv->fbdev_phytium_gem->cookie, offset, size);
	fbi->skip_vt_switch = false;

	return 0;
out:
	ftd330_gem_free_object(&priv->fbdev_phytium_gem->base);
	return ret;
}

static const struct drm_fb_helper_funcs phytium_drm_fb_helper_funcs = {
	.fb_probe = phytium_drm_fbdev_create,
};
#endif


void phytium_drm_fbdev_init(struct ftd330_drm_private *priv)
{
	struct drm_device *dev = priv->drm_dev;
#if KERNEL_VERSION(6, 1, 0) >= LINUX_VERSION_CODE
	struct drm_fb_helper *helper;
	int ret;

	if (!dev->mode_config.num_crtc || !dev->mode_config.num_connector) {
		DRM_DEV_ERROR(dev->dev, "Failed to init fbdev because of wrong num_crtc\n");
		return;
	}

	helper = &priv->fbdev_helper;
	drm_fb_helper_prepare(dev, helper, &phytium_drm_fb_helper_funcs);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	ret = drm_fb_helper_init(dev, helper, PHYTIUM_MAX_CONNECTOR);
#else
	ret = drm_fb_helper_init(dev, helper);
#endif
	if (ret < 0) {
		DRM_DEV_ERROR(dev->dev, "Failed to initialize drm fb helper\n");
	}
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
	ret = drm_fb_helper_single_add_all_connectors(helper);
	if (ret < 0) {
		DRM_DEV_ERROR(dev->dev, "Failed to add connectors - %d/\n", ret);
		drm_fb_helper_fini(helper);
		return;
	}
#endif
	drm_fb_helper_initial_config(helper, 32);
	return;
#else 
	drm_fbdev_generic_setup(dev, 32);
	return;
#endif

}

void phytium_drm_fbdev_fini(struct drm_device *dev)
{
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	struct ftd330_drm_private *priv = dev->dev_private;
	struct drm_fb_helper *helper;

	helper = &priv->fbdev_helper;
	drm_fb_helper_unregister_fbi(helper);

	if (helper->fb)
		drm_framebuffer_put(helper->fb);

	drm_fb_helper_fini(helper);
	return;
#else
	return;
#endif
}

