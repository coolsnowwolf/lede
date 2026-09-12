// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/version.h>

#include <drm/ftd330_drm_fourcc.h>
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#endif
#include <drm/drm_atomic.h>
#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
#include <drm/drm_damage_helper.h>
#endif
#include <drm/drm_fb_helper.h>
#include <drm/drm_gem.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_vblank.h>
#include <drm/drm_framebuffer.h>
#include "ftd330_fb.h"
#include "ftd330_gem.h"
#include "ftd330_crtc.h"
#include "phytium_dp.h"
#include "phytium_psr.h"

#define fourcc_mod_ftd330_get_type(val) (((val)&DRM_FORMAT_MOD_FTD330_TYPE_MASK) >> 53)
#define _FTD330_WAIT_VBLANK_TIME_OUT 100000

#ifdef CONFIG_PHYTIUM_PSR

struct phytium_dp_device *
find_phytium_dp_from_fb(struct drm_framebuffer *fb)
{
	struct drm_device *dev = fb->dev;
	struct drm_plane *plane = NULL;
	struct drm_crtc *crtc = NULL;
	struct drm_connector *connector = NULL;
	struct phytium_dp_device *phytium_dp = NULL;
	struct ftd330_drm_private *priv = dev->dev_private;
	uint32_t crtc_idx, dp_idx;

	drm_for_each_plane(plane, dev) {
		if (plane->state && plane->state->fb == fb) {
			crtc = plane->state->crtc;
			if (!crtc) {
#if PSR_DEBUG
				pr_info("Can not find crtc bound to plane\n");
#endif
				goto nullptr_handle;
			}

			crtc_idx = drm_crtc_index(crtc);
			dp_idx = phytium_display_virtual_to_physical(priv->info.pipe_mask, crtc_idx);
			phytium_dp = priv->phytium_dp[dp_idx];
			if (phytium_dp->is_edp)
				return phytium_dp;
		}
	}
	if (phytium_dp && !phytium_dp->is_edp)
		return NULL;

	if (!phytium_dp) {
#if PSR_DEBUG
		pr_info("Can not find phytium_edp from crtc_state\n");
#endif
	}
	return phytium_dp;

nullptr_handle:
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (connector->status == connector_status_connected &&
			connector->connector_type == DRM_MODE_CONNECTOR_eDP) {
				phytium_dp = connector_to_dp_device(connector);
				break;
		}
	}
	if (!phytium_dp) {
#if PSR_DEBUG
		pr_info("Can not find phytium_edp from connector list\n");
#endif
	}
	return phytium_dp;
}

static int ftd330_atomic_dirtyfb(struct drm_framebuffer *fb,
			      struct drm_file *file_priv, unsigned int flags,
			      unsigned int color, struct drm_clip_rect *clips,
			      unsigned int num_clips)
{
	struct phytium_dp_device *phytium_dp = NULL;
	bool psr_is_enabled = false;
	unsigned long irq_flag;
	struct drm_device *drm_dev = NULL;
	struct ftd330_drm_private *priv = NULL;

	phytium_dp = find_phytium_dp_from_fb(fb);

	if (phytium_dp) {
		mutex_lock(&phytium_dp->low_power_mutex);
		psr_is_enabled = phytium_dp->psr.enabled;
		mutex_unlock(&phytium_dp->low_power_mutex);
		if (psr_is_enabled) {
			drm_dev = phytium_dp->dev;
			priv = drm_dev->dev_private;
			spin_lock_irqsave(&priv->hotplug_irq_lock,irq_flag);
			phytium_dp->dplp_frame_compare_state.psr_available = false;
			phytium_dp->dplp_frame_compare_state.frame_change_in_psr = true;
			spin_unlock_irqrestore(&priv->hotplug_irq_lock,irq_flag);
			schedule_work(&phytium_dp->psr_work);
		}
	}

	return 0;
}
#endif

static struct drm_framebuffer_funcs ftd330_fb_funcs = {
	.create_handle = drm_gem_fb_create_handle,
	.destroy = drm_gem_fb_destroy,
#ifdef CONFIG_PHYTIUM_PSR
	.dirty = ftd330_atomic_dirtyfb,
#endif
};

struct drm_framebuffer *ftd330_fb_alloc(struct drm_device *dev,
					   const struct drm_mode_fb_cmd2 *mode_cmd,
					   struct ftd330_gem_object **obj, unsigned int num_planes)
{
	struct drm_framebuffer *fb;
	struct ftd330_drm_private *priv = dev->dev_private;
	int ret, i;
	u64 addr;

	fb = kzalloc(sizeof(*fb), GFP_KERNEL);
	if (!fb)
		return ERR_PTR(-ENOMEM);

	drm_helper_mode_fill_fb_struct(dev, fb, mode_cmd);

	for (i = 0; i < num_planes; i++) {
		addr = obj[i]->iova + mode_cmd->offsets[i];
		fb->obj[i] = &obj[i]->base;

		if (fb->pitches[i] % priv->pitch_alignment) {
			dev_err(dev->dev, "The framebuffer stride should aligment with %d\n",priv->pitch_alignment);
			kfree(fb);
			return ERR_PTR(-EINVAL);
		}

		if (addr % priv->addr_alignment) {
			dev_err(dev->dev, "The framebuffer address should aligment with %d\n",
				priv->addr_alignment);
			kfree(fb);
			return ERR_PTR(-EINVAL);
		}
	}

	ret = drm_framebuffer_init(dev, fb, &ftd330_fb_funcs);
	if (ret) {
		dev_err(dev->dev, "Failed to initialize framebuffer: %d\n", ret);
		kfree(fb);
		return ERR_PTR(ret);
	}

	return fb;
}

static struct drm_framebuffer *ftd330_fb_create(struct drm_device *dev, struct drm_file *file_priv,
					    const struct drm_mode_fb_cmd2 *mode_cmd)
{
	struct drm_framebuffer *fb;
	const struct drm_format_info *info;
	struct ftd330_gem_object *objs[MAX_NUM_PLANES];
	struct drm_gem_object *obj;
	unsigned int height, size;
	unsigned char i, num_planes;
	int ret = 0;

	info = drm_get_format_info(dev, mode_cmd);
	if (!info)
		return ERR_PTR(-EINVAL);

	num_planes = info->num_planes;
	if (num_planes > MAX_NUM_PLANES)
		return ERR_PTR(-EINVAL);

	for (i = 0; i < num_planes; i++) {
		obj = drm_gem_object_lookup(file_priv, mode_cmd->handles[i]);
		if (!obj) {
			dev_err(dev->dev, "Failed to lookup GEM object.\n");
			ret = -ENXIO;
			goto err;
		}

		if (!((fourcc_mod_ftd330_get_type(mode_cmd->modifier[i]) ==
		       DRM_FORMAT_MOD_FTD330_TYPE_PVRIC) &&
		      (mode_cmd->modifier[i] & DRM_FORMAT_MOD_FTD330_DEC_LOSSY))) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
			height = drm_format_info_plane_height(info, mode_cmd->height, i);
#else
		if (i == 0)
			height = mode_cmd->height;
		else
			height = mode_cmd->height / info->vsub;
#endif
			size = height * mode_cmd->pitches[i] + mode_cmd->offsets[i];

			if (obj->size < size) {
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
				drm_gem_object_put(obj);
#else
				drm_gem_object_put_unlocked(obj);
#endif
				ret = -EINVAL;
				goto err;
			}
		}

		objs[i] = to_ftd330_gem_object(obj);
	}

	fb = ftd330_fb_alloc(dev, mode_cmd, objs, i);
	if (IS_ERR(fb)) {
		ret = PTR_ERR(fb);
		goto err;
	}

	return fb;

err:
	for (; i > 0; i--)
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
		drm_gem_object_put(&objs[i - 1]->base);
#else
		drm_gem_object_put_unlocked(&objs[i - 1]->base);
#endif

	return ERR_PTR(ret);
}

struct ftd330_gem_object *ftd330_fb_get_gem_obj(struct drm_framebuffer *fb, unsigned char plane)
{
	if (plane > MAX_NUM_PLANES)
		return NULL;

	return to_ftd330_gem_object(fb->obj[plane]);
}

static const struct drm_format_info ftd330_formats_custom[] = {
	{ .format = DRM_FORMAT_NV12,
	  .depth = 0,
	  .num_planes = 2,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	  .char_per_block = { 20, 40, 0 },
	  .block_w = { 4, 4, 0 },
	  .block_h = { 4, 4, 0 },
#endif
	  .hsub = 2,
	  .vsub = 2,
	  .is_yuv = true },
	{ .format = DRM_FORMAT_YUV444,
	  .depth = 0,
	  .num_planes = 3,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	  .char_per_block = { 20, 20, 20 },
	  .block_w = { 4, 4, 4 },
	  .block_h = { 4, 4, 4 },
#endif
	  .hsub = 1,
	  .vsub = 1,
	  .is_yuv = true },
	{ .format = DRM_FORMAT_RGB565_A8,
	  .depth = 0,
	  .num_planes = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	  .char_per_block = { 3, 0, 0 },
	  .block_w = { 1, 0, 0 },
	  .block_h = { 1, 0, 0 },
#endif
	  .hsub = 1,
	  .vsub = 1,
	  .has_alpha = true },
	{ .format = DRM_FORMAT_BGR565_A8,
	  .depth = 0,
	  .num_planes = 1,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	  .char_per_block = { 3, 0, 0 },
	  .block_w = { 1, 0, 0 },
	  .block_h = { 1, 0, 0 },
#endif
	  .hsub = 1,
	  .vsub = 1,
	  .has_alpha = true },
	{ .format = DRM_FORMAT_RGB888, /* RGB888-planer */
	  .num_planes = 3,
	  .cpp = { 1, 1, 1 },
	  .hsub = 1,
	  .vsub = 1 },
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	{ .format = DRM_FORMAT_YUV420_10BIT,
	  .depth = 0,
	  .num_planes = 2,
	  .char_per_block = { 4, 8, 0 },
	  .block_w = { 3, 3, 0 },
	  .block_h = { 1, 1, 0 },
	  .hsub = 2,
	  .vsub = 2,
	  .is_yuv = true }
#endif
};

static const struct drm_format_info *ftd330_lookup_format_info(const struct drm_format_info formats[],
							   int num_formats, u32 format)
{
	int i;

	for (i = 0; i < num_formats; i++) {
		if (formats[i].format == format)
			return &formats[i];
	}

	return NULL;
}

static const struct drm_format_info *ftd330_get_format_info(const struct drm_mode_fb_cmd2 *cmd)
{
	if (fourcc_mod_is_custom_format(cmd->modifier[0]))
		return ftd330_lookup_format_info(ftd330_formats_custom, ARRAY_SIZE(ftd330_formats_custom),
					     cmd->pixel_format);
	else
		return NULL;
}

static void _ftd330_drm_atomic_helper_commit_hw_done(struct drm_atomic_state *old_state)
{
	struct drm_crtc_state *old_crtc_state, *new_crtc_state;
	struct drm_crtc_commit *commit;
	struct drm_crtc *crtc;
	struct ftd330_crtc *ftd330_crtc;
	int i;

	for_each_oldnew_crtc_in_state(old_state, crtc, old_crtc_state, new_crtc_state, i) {
		commit = new_crtc_state->commit;

		if (!commit)
			continue;

		if (crtc->state != new_crtc_state)
			continue;

		if (old_crtc_state->commit)
			drm_crtc_commit_put(old_crtc_state->commit);

		old_crtc_state->commit = drm_crtc_commit_get(commit);

		WARN_ON(new_crtc_state->event);
		complete_all(&commit->hw_done);
		ftd330_crtc = to_ftd330_crtc(crtc);
		ftd330_crtc->commit_hw_done = true;
	}

	if (old_state->fake_commit) {
		complete_all(&old_state->fake_commit->hw_done);
		complete_all(&old_state->fake_commit->flip_done);
	}
}


static void _ftd330_drm_atomic_helper_wait_for_flip_done(struct drm_device *dev,
						     struct drm_atomic_state *old_state)
{
	struct drm_crtc *crtc;
	int i;

	for (i = 0; i < dev->mode_config.num_crtc; i++) {
		struct drm_crtc_commit *commit = old_state->crtcs[i].commit;
		int ret;

		crtc = old_state->crtcs[i].ptr;

		if (!crtc || !commit)
			continue;

		ret = wait_for_completion_timeout(&commit->flip_done, _FTD330_WAIT_VBLANK_TIME_OUT);
		if (ret == 0)
			DRM_ERROR("[CRTC:%d:%s] flip_done timed out\n", crtc->base.id, crtc->name);
	}

	if (old_state->fake_commit)
		complete_all(&old_state->fake_commit->flip_done);
 }

static void ftd330_atomic_commit_tail(struct drm_atomic_state *old_state)
{
	struct drm_device *dev = old_state->dev;

	drm_atomic_helper_commit_modeset_disables(dev, old_state);

	drm_atomic_helper_commit_modeset_enables(dev, old_state);

	drm_atomic_helper_commit_planes(dev, old_state, DRM_PLANE_COMMIT_ACTIVE_ONLY);

	_ftd330_drm_atomic_helper_commit_hw_done(old_state);
	drm_atomic_helper_cleanup_planes(dev, old_state);
	/* wait for the commit effective */
	_ftd330_drm_atomic_helper_wait_for_flip_done(dev, old_state);
}

static const struct drm_mode_config_funcs ftd330_mode_config_funcs = {
	.fb_create = ftd330_fb_create,
	.get_format_info = ftd330_get_format_info,
	.output_poll_changed = drm_fb_helper_output_poll_changed,
	.atomic_check = drm_atomic_helper_check,
	.atomic_commit = drm_atomic_helper_commit,
};

static struct drm_mode_config_helper_funcs ftd330_mode_config_helpers = {
	.atomic_commit_tail = ftd330_atomic_commit_tail,
};

void ftd330_mode_config_init(struct drm_device *dev)
{
#if (KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE)
	/* this is removed in 5.18 and becomes default behavior */
	dev->mode_config.allow_fb_modifiers = true;
#endif

	if (dev->mode_config.max_width == 0 || dev->mode_config.max_height == 0) {
		dev->mode_config.min_width = 0;
		dev->mode_config.min_height = 0;
		dev->mode_config.max_width = 4096;
		dev->mode_config.max_height = 4096;
	}
	dev->mode_config.funcs = &ftd330_mode_config_funcs;
	dev->mode_config.helper_private = &ftd330_mode_config_helpers;
#ifndef CONFIG_PHYTIUM_WRITEBACK
	phytium_dp_platform_init(dev);
#endif
}

int ftd330_get_fbc_offset_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_ftd330_pvric_offset *args = data;
	const struct drm_format_info *info;
	struct drm_gem_object *obj;
	struct ftd330_gem_object *objs[MAX_NUM_PLANES];
	u64 base_addr = 0;
	u8 i, num_planes;
	int ret = 0;

	info = drm_format_info(args->format);
	if (!info)
		return -EINVAL;

	num_planes = info->num_planes;
	if (num_planes > MAX_NUM_PLANES)
		return -EINVAL;

	for (i = 0; i < num_planes; i++) {
		obj = drm_gem_object_lookup(file_priv, args->handles[i]);
		if (!obj) {
			dev_err(dev->dev, "Failed to lookup GEM object.\n");
			ret = -ENXIO;
			goto err;
		}

		objs[i] = to_ftd330_gem_object(obj);

		base_addr = ALIGN(objs[i]->iova + args->header_size[i], 256);
		args->offsets[i] = base_addr - objs[i]->iova;
	}

	return 0;

err:
	for (; i > 0; i--)
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
		drm_gem_object_put(&objs[i - 1]->base);
#else
		drm_gem_object_put_unlocked(&objs[i - 1]->base);
#endif

	return ret;
}

int ftd330_display_getfb2_ioctl(struct drm_device *dev,
			  void *data, struct drm_file *file_priv)
{
	struct drm_phytium_display_fb_cmd2 *r = data;
	struct drm_framebuffer *fb;
	unsigned int i;
	int ret = 0;

	fb = drm_framebuffer_lookup(dev, file_priv, r->fb_id);
	if (!fb)
		return -ENOENT;

	/* For multi-plane framebuffers, we require the driver to place the
	 * GEM objects directly in the drm_framebuffer. For single-plane
	 * framebuffers, we can fall back to create_handle.
	 */
	if (!fb->obj[0] &&
	    (fb->format->num_planes > 1 || !fb->funcs->create_handle)) {
		ret = -ENODEV;
		goto out;
	}

	r->height = fb->height;
	r->width = fb->width;
	r->pixel_format = fb->format->format;

	r->flags = 0;

	for (i = 0; i < ARRAY_SIZE(r->handles); i++) {
		r->handles[i] = 0;
		r->pitches[i] = 0;
		r->offsets[i] = 0;
		r->modifier[i] = 0;
	}

	for (i = 0; i < fb->format->num_planes; i++) {
		r->pitches[i] = fb->pitches[i];
		r->offsets[i] = fb->offsets[i];
		r->modifier[i] = fb->modifier;
	}

	for (i = 0; i < fb->format->num_planes; i++) {
		int j;

		/* If we reuse the same object for multiple planes, also
		 * return the same handle.
		 */
		for (j = 0; j < i; j++) {
			if (fb->obj[i] == fb->obj[j]) {
				r->handles[i] = r->handles[j];
				break;
			}
		}

		if (r->handles[i])
			continue;

		if (fb->obj[i]) {
			ret = drm_gem_handle_create(file_priv, fb->obj[i],
						    &r->handles[i]);
		} else {
			WARN_ON(i > 0);
			ret = fb->funcs->create_handle(fb, file_priv,
						       &r->handles[i]);
		}

		if (ret != 0)
			goto out;
	}

out:
	if (ret != 0) {
		/* Delete any previously-created handles on failure. */
		for (i = 0; i < ARRAY_SIZE(r->handles); i++) {
			int j;

			if (r->handles[i])
				drm_gem_handle_delete(file_priv, r->handles[i]);

			/* Zero out any handles identical to the one we just
			 * deleted.
			 */
			for (j = i + 1; j < ARRAY_SIZE(r->handles); j++) {
				if (r->handles[j] == r->handles[i])
					r->handles[j] = 0;
			}
		}
	}

	drm_framebuffer_put(fb);
	return ret;
}

