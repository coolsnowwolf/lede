// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_gem.h>
#include <drm/drm_modeset_helper.h>
#include <drm/drm_fourcc.h>
#include "phytium_display_drv.h"
#include "phytium_fb.h"
#include "phytium_gem.h"

static int
phytium_fb_create_handle(struct drm_framebuffer *fb, struct drm_file *file_priv,
				   unsigned int *handle)
{
	struct phytium_framebuffer *phytium_fb = to_phytium_framebuffer(fb);

	return drm_gem_handle_create(file_priv, &phytium_fb->phytium_gem_obj[0]->base, handle);
}

static void phytium_fb_destroy(struct drm_framebuffer *fb)
{
	struct phytium_framebuffer *phytium_fb = to_phytium_framebuffer(fb);
	int i, num_planes;
	struct drm_gem_object *obj = NULL;
	const struct drm_format_info *info;

	info = drm_format_info(fb->format->format);
	num_planes = info ? info->num_planes : 1;

	for (i = 0; i < num_planes; i++) {
		obj = &phytium_fb->phytium_gem_obj[i]->base;
		if (obj)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
			drm_gem_object_put(obj);
#else
			drm_gem_object_unreference_unlocked(obj);
#endif
	}

	drm_framebuffer_cleanup(fb);
	kfree(phytium_fb);
}

static struct drm_framebuffer_funcs viv_fb_funcs = {
	.create_handle	= phytium_fb_create_handle,
	.destroy	= phytium_fb_destroy,
};

struct phytium_framebuffer *
phytium_fb_alloc(struct drm_device *dev, const struct drm_mode_fb_cmd2 *mode_cmd,
		       struct phytium_gem_object **phytium_gem_obj, unsigned int num_planes)
{
	struct phytium_framebuffer *phytium_fb;
	int ret = 0, i;

	phytium_fb = kzalloc(sizeof(*phytium_fb), GFP_KERNEL);
	if (!phytium_fb)
		return ERR_PTR(-ENOMEM);

	drm_helper_mode_fill_fb_struct(dev, &phytium_fb->base, mode_cmd);

	ret = drm_framebuffer_init(dev, &phytium_fb->base, &viv_fb_funcs);

	if (ret) {
		DRM_ERROR("Failed to initialize framebuffer: %d\n", ret);
		kfree(phytium_fb);
		return ERR_PTR(ret);
	}

	for (i = 0; i < num_planes; i++) {
		phytium_fb->phytium_gem_obj[i] = phytium_gem_obj[i];
		phytium_fb->base.obj[i] = &phytium_gem_obj[i]->base;
	}
	return phytium_fb;
}

struct drm_framebuffer *
phytium_fb_create(struct drm_device *dev, struct drm_file *file_priv,
			 const struct drm_mode_fb_cmd2 *mode_cmd)
{
	int ret = 0, i, num_planes;
	struct drm_gem_object *obj;
	unsigned int hsub, vsub, size;
	struct phytium_gem_object *phytium_gem_obj[PHYTIUM_FORMAT_MAX_PLANE] = {0};
	struct phytium_framebuffer *phytium_fb;
	struct phytium_display_private *priv = dev->dev_private;
	const struct drm_format_info *info;

	info = drm_format_info(mode_cmd->pixel_format);
	hsub = info ? info->hsub : 1;
	vsub = info ? info->vsub : 1;
	num_planes = info ? info->num_planes : 1;
	num_planes = min(num_planes, PHYTIUM_FORMAT_MAX_PLANE);

	for (i = 0; i < num_planes; i++) {
		unsigned int height = mode_cmd->height / (i ? vsub : 1);

		size = height * mode_cmd->pitches[i] + mode_cmd->offsets[i];
		obj = drm_gem_object_lookup(file_priv, mode_cmd->handles[i]);
		if (!obj) {
			DRM_ERROR("Failed to lookup GEM object\n");
			ret = -ENXIO;
			goto error;
		}

		if (obj->size < size) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
			drm_gem_object_put(obj);
#else
			drm_gem_object_unreference_unlocked(obj);
#endif
			ret = -EINVAL;
			goto error;
		}

		phytium_gem_obj[i] = to_phytium_gem_obj(obj);

		ret = priv->dc_hw_fb_format_check(mode_cmd, i);
		if (ret < 0)
			goto error;
	}

	phytium_fb = phytium_fb_alloc(dev, mode_cmd, phytium_gem_obj, i);
	if (IS_ERR(phytium_fb)) {
		DRM_DEBUG_KMS("phytium_fb_alloc failed\n");
		ret = PTR_ERR(phytium_fb);
		goto error;
	}

	return &phytium_fb->base;
error:
	for (i--; i >= 0; i--)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
		drm_gem_object_put(&phytium_gem_obj[i]->base);
#else
		drm_gem_object_unreference_unlocked(&phytium_gem_obj[i]->base);
#endif

	return ERR_PTR(ret);
}
