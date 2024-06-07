// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <drm/drm_gem_framebuffer_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/drm_atomic_uapi.h>
#include <drm/drm_fourcc.h>

#include <linux/dma-buf.h>

#include "phytium_display_drv.h"
#include "phytium_plane.h"
#include "phytium_fb.h"
#include "phytium_gem.h"
#include "phytium_crtc.h"
#include "px210_dc.h"
#include "pe220x_dc.h"
#include "phytium_reg.h"

#define PHYTIUM_CURS_W_SIZE 32
#define PHYTIUM_CURS_H_SIZE 32

void phytium_plane_destroy(struct drm_plane *plane)
{
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);

	drm_plane_cleanup(plane);
	kfree(phytium_plane);
}

/**
 * phytium_plane_atomic_get_property - fetch plane property value
 * @plane: plane to fetch property for
 * @state: state containing the property value
 * @property: property to look up
 * @val: pointer to write property value into
 *
 * The DRM core does not store shadow copies of properties for
 * atomic-capable drivers.  This entrypoint is used to fetch
 * the current value of a driver-specific plane property.
 */
static int
phytium_plane_atomic_get_property(struct drm_plane *plane,
					const struct drm_plane_state *state,
					struct drm_property *property,
					uint64_t *val)
{
	DRM_DEBUG_KMS("Unknown plane property [PROP:%d:%s]\n", property->base.id, property->name);
	return -EINVAL;
}

/**
 * phytium_plane_atomic_set_property - set plane property value
 * @plane: plane to set property for
 * @state: state to update property value in
 * @property: property to set
 * @val: value to set property to
 *
 * Writes the specified property value for a plane into the provided atomic
 * state object.
 *
 * Returns 0 on success, -EINVAL on unrecognized properties
 */
int
phytium_plane_atomic_set_property(struct drm_plane *plane,
					struct drm_plane_state *state,
					struct drm_property *property,
					uint64_t val)
{
	DRM_DEBUG_KMS("Unknown plane property [PROP:%d:%s]\n", property->base.id, property->name);
	return -EINVAL;
}

struct drm_plane_state *
phytium_plane_atomic_duplicate_state(struct drm_plane *plane)
{
	struct drm_plane_state *state = NULL;
	struct phytium_plane_state *phytium_state = NULL;

	phytium_state = kmemdup(plane->state, sizeof(*phytium_state), GFP_KERNEL);

	if (!phytium_state)
		return NULL;

	state = &phytium_state->base;
	if (state->fb)
		drm_framebuffer_get(state->fb);

	state->fence = NULL;
	state->commit = NULL;

	return state;
}

void
phytium_plane_atomic_destroy_state(struct drm_plane *plane, struct drm_plane_state *state)
{
	struct phytium_plane_state *phytium_state = to_phytium_plane_state(state);

	__drm_atomic_helper_plane_destroy_state(state);
	kfree(phytium_state);
}

static bool phytium_plane_format_mod_supported(struct drm_plane *plane,
						uint32_t format, uint64_t modifier)
{
	if (modifier == DRM_FORMAT_MOD_LINEAR)
		return true;

	if (modifier == DRM_FORMAT_MOD_PHYTIUM_TILE_MODE3_FBCDC) {
		switch (format) {
		case DRM_FORMAT_ARGB2101010:
		case DRM_FORMAT_ABGR2101010:
		case DRM_FORMAT_RGBA1010102:
		case DRM_FORMAT_BGRA1010102:
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_BGRA8888:
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_XBGR8888:
		case DRM_FORMAT_RGBX8888:
		case DRM_FORMAT_BGRX8888:
			return true;
		default:
			return false;
		}
	}

	return false;
}

const struct drm_plane_funcs phytium_plane_funcs = {
	.update_plane		= drm_atomic_helper_update_plane,
	.disable_plane		= drm_atomic_helper_disable_plane,
	.destroy		= phytium_plane_destroy,
	.reset			= drm_atomic_helper_plane_reset,
	.atomic_get_property	= phytium_plane_atomic_get_property,
	.atomic_set_property	= phytium_plane_atomic_set_property,
	.atomic_duplicate_state	= phytium_plane_atomic_duplicate_state,
	.atomic_destroy_state	= phytium_plane_atomic_destroy_state,
	.format_mod_supported	= phytium_plane_format_mod_supported,
};

static int phytium_plane_prepare_fb(struct drm_plane *plane,
					       struct drm_plane_state *state)
{
	struct dma_buf *dma_buf;
	struct dma_fence *fence;

	if (!state->fb)
		return 0;
	dma_buf = to_phytium_framebuffer(state->fb)->phytium_gem_obj[0]->base.dma_buf;
	if (dma_buf) {
		fence = dma_resv_get_excl_rcu(dma_buf->resv);
		drm_atomic_set_fence_for_plane(state, fence);
	}

	return 0;
}

static int
phytium_plane_atomic_check(struct drm_plane *plane, struct drm_plane_state *state)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct drm_framebuffer *fb = state->fb;
	struct drm_crtc *crtc = state->crtc;
	struct drm_crtc_state *crtc_state;
	int src_x, src_y, src_w, src_h;
	unsigned long base_offset;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);

	if ((!fb) || (!crtc))
		return 0;

	crtc_state = drm_atomic_get_crtc_state(state->state, crtc);
	if (IS_ERR(crtc_state))
		return PTR_ERR(crtc_state);

	if (plane->type == DRM_PLANE_TYPE_CURSOR) {
		src_w = state->src_w >> 16;
		src_h = state->src_h >> 16;
		if (phytium_crtc->scale_enable)
			return -EINVAL;
		if ((src_w != PHYTIUM_CURS_W_SIZE) || (src_h != PHYTIUM_CURS_W_SIZE)) {
			DRM_INFO("Invalid cursor size(%d, %d)\n", src_w, src_h);
			return -EINVAL;
		}
	} else if (plane->type == DRM_PLANE_TYPE_PRIMARY) {
		src_x = state->src_x >> 16;
		src_y = state->src_y >> 16;
		src_w = state->src_w >> 16;
		src_h = state->src_h >> 16;

		base_offset = src_x * fb->format->cpp[0] + src_y*fb->pitches[0];
		if (base_offset & (priv->info.address_mask)) {
			DRM_ERROR("fb base address is not aligned by 0x%lx byte\n",
				   priv->info.address_mask);
			return -EINVAL;
		}

		if (src_w != state->crtc_w || src_h != state->crtc_h) {
			DRM_ERROR("scale not support: crtc_w(0x%x)/h(0x%x) src_w(0x%x)/h(0x%x)\n",
				   state->crtc_w, state->crtc_h, src_w, src_h);
			return -EINVAL;
		}

		if ((state->crtc_x < 0) || (state->crtc_y < 0)) {
			DRM_ERROR("crtc_x(0x%x)/y(0x%x) of drm plane state is invalid\n",
				   state->crtc_x, state->crtc_y);
			return -EINVAL;
		}

		if ((state->crtc_x + state->crtc_w > crtc_state->adjusted_mode.hdisplay)
			|| (state->crtc_y + state->crtc_h > crtc_state->adjusted_mode.vdisplay)) {
			DRM_ERROR("plane out of crtc region\n");
			return -EINVAL;
		}
	}

	return 0;
}

static void phytium_dc_get_plane_parameter(struct drm_plane *plane)
{
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	struct drm_framebuffer *fb = plane->state->fb;
	struct phytium_framebuffer *phytium_fb = to_phytium_framebuffer(fb);
	struct phytium_gem_object *phytium_gem_obj = NULL;
	int i, num_planes = 0;
	const struct drm_format_info *info;

	info = drm_format_info(fb->format->format);
	num_planes = info ? info->num_planes : 1;

	for (i = 0; i < num_planes; i++) {
		phytium_gem_obj = phytium_fb->phytium_gem_obj[i];
		phytium_plane->iova[i] =  phytium_gem_obj->iova + fb->offsets[i];
		phytium_plane->size[i] = phytium_gem_obj->size - fb->offsets[i];

		if (fb->modifier == DRM_FORMAT_MOD_PHYTIUM_TILE_MODE0_FBCDC)
			phytium_plane->tiling[i] = FRAMEBUFFER_TILE_MODE0;
		else if (fb->modifier == DRM_FORMAT_MOD_PHYTIUM_TILE_MODE3_FBCDC)
			phytium_plane->tiling[i] = FRAMEBUFFER_TILE_MODE3;
		else if (fb->modifier == DRM_FORMAT_MOD_LINEAR)
			phytium_plane->tiling[i] = FRAMEBUFFER_LINEAR;
		else
			phytium_plane->tiling[i] = FRAMEBUFFER_LINEAR;

		if (i == 0) {
			switch (fb->format->format) {
			case DRM_FORMAT_ARGB2101010:
			case DRM_FORMAT_ABGR2101010:
			case DRM_FORMAT_RGBA1010102:
			case DRM_FORMAT_BGRA1010102:
				phytium_plane->format = FRAMEBUFFER_FORMAT_ARGB2101010;
				break;

			case DRM_FORMAT_ARGB8888:
			case DRM_FORMAT_ABGR8888:
			case DRM_FORMAT_RGBA8888:
			case DRM_FORMAT_BGRA8888:
				phytium_plane->format = FRAMEBUFFER_FORMAT_ARGB8888;
				break;

			case DRM_FORMAT_XRGB8888:
			case DRM_FORMAT_XBGR8888:
			case DRM_FORMAT_RGBX8888:
			case DRM_FORMAT_BGRX8888:
				phytium_plane->format = FRAMEBUFFER_FORMAT_XRGB8888;
				break;

			case DRM_FORMAT_ARGB4444:
			case DRM_FORMAT_ABGR4444:
			case DRM_FORMAT_RGBA4444:
			case DRM_FORMAT_BGRA4444:
				phytium_plane->format = FRAMEBUFFER_FORMAT_ARGB4444;
				break;

			case DRM_FORMAT_XRGB4444:
			case DRM_FORMAT_XBGR4444:
			case DRM_FORMAT_RGBX4444:
			case DRM_FORMAT_BGRX4444:
				phytium_plane->format = FRAMEBUFFER_FORMAT_XRGB4444;
				break;

			case DRM_FORMAT_ARGB1555:
			case DRM_FORMAT_ABGR1555:
			case DRM_FORMAT_RGBA5551:
			case DRM_FORMAT_BGRA5551:
				phytium_plane->format = FRAMEBUFFER_FORMAT_ARGB1555;
				break;

			case DRM_FORMAT_XRGB1555:
			case DRM_FORMAT_XBGR1555:
			case DRM_FORMAT_RGBX5551:
			case DRM_FORMAT_BGRX5551:
				phytium_plane->format = FRAMEBUFFER_FORMAT_XRGB1555;
				break;

			case DRM_FORMAT_RGB565:
			case DRM_FORMAT_BGR565:
				phytium_plane->format = FRAMEBUFFER_FORMAT_RGB565;
				break;

			case DRM_FORMAT_YUYV:
				phytium_plane->format = FRAMEBUFFER_FORMAT_YUYV;
				break;

			case DRM_FORMAT_UYVY:
				phytium_plane->format = FRAMEBUFFER_FORMAT_UYVY;
				break;
			case DRM_FORMAT_NV16:
				phytium_plane->format = FRAMEBUFFER_FORMAT_NV16;
				break;
			case DRM_FORMAT_NV12:
				phytium_plane->format = FRAMEBUFFER_FORMAT_NV12;
				break;
			case DRM_FORMAT_NV21:
				phytium_plane->format = FRAMEBUFFER_FORMAT_NV12;
				break;
			default:
				DRM_ERROR("unsupported pixel format (format = %d)\n",
					  fb->format->format);
				return;
			}

			switch (fb->format->format) {
			case DRM_FORMAT_ARGB2101010:
			case DRM_FORMAT_ARGB8888:
			case DRM_FORMAT_XRGB8888:
			case DRM_FORMAT_ARGB4444:
			case DRM_FORMAT_XRGB4444:
			case DRM_FORMAT_ARGB1555:
			case DRM_FORMAT_XRGB1555:
			case DRM_FORMAT_RGB565:
				phytium_plane->swizzle = FRAMEBUFFER_SWIZZLE_ARGB;
				phytium_plane->uv_swizzle = FRAMEBUFFER_UVSWIZZLE_DISABLE;
				break;

			case DRM_FORMAT_ABGR2101010:
			case DRM_FORMAT_ABGR8888:
			case DRM_FORMAT_XBGR8888:
			case DRM_FORMAT_ABGR4444:
			case DRM_FORMAT_XBGR4444:
			case DRM_FORMAT_ABGR1555:
			case DRM_FORMAT_XBGR1555:
			case DRM_FORMAT_BGR565:
				phytium_plane->swizzle = FRAMEBUFFER_SWIZZLE_ABGR;
				phytium_plane->uv_swizzle = FRAMEBUFFER_UVSWIZZLE_DISABLE;
				break;

			case DRM_FORMAT_RGBA1010102:
			case DRM_FORMAT_RGBA8888:
			case DRM_FORMAT_RGBX8888:
			case DRM_FORMAT_RGBA4444:
			case DRM_FORMAT_RGBX4444:
			case DRM_FORMAT_RGBA5551:
			case DRM_FORMAT_RGBX5551:
				phytium_plane->swizzle = FRAMEBUFFER_SWIZZLE_RGBA;
				phytium_plane->uv_swizzle = FRAMEBUFFER_UVSWIZZLE_DISABLE;
				break;

			case DRM_FORMAT_BGRA1010102:
			case DRM_FORMAT_BGRA8888:
			case DRM_FORMAT_BGRX8888:
			case DRM_FORMAT_BGRA4444:
			case DRM_FORMAT_BGRX4444:
			case DRM_FORMAT_BGRA5551:
			case DRM_FORMAT_BGRX5551:
				phytium_plane->swizzle = FRAMEBUFFER_SWIZZLE_BGRA;
				phytium_plane->uv_swizzle = FRAMEBUFFER_UVSWIZZLE_DISABLE;
				break;

			case DRM_FORMAT_YUYV:
			case DRM_FORMAT_UYVY:
			case DRM_FORMAT_NV16:
			case DRM_FORMAT_NV12:
				phytium_plane->swizzle = FRAMEBUFFER_SWIZZLE_ARGB;
				phytium_plane->uv_swizzle = FRAMEBUFFER_UVSWIZZLE_DISABLE;
				break;

			default:
				DRM_ERROR("unsupported pixel format (format = %d)\n",
					   fb->format->format);
				return;
			}
		}
	}
}

static void phytium_dc_primary_plane_update(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	struct drm_framebuffer *fb = plane->state->fb;
	int phys_pipe = phytium_plane->phys_pipe;
	int src_x, src_y, crtc_x, crtc_y, crtc_w, crtc_h;
	unsigned long base_offset;
	int config;

	src_x = plane->state->src_x >> 16;
	src_y = plane->state->src_y >> 16;
	crtc_x = plane->state->crtc_x;
	crtc_y = plane->state->crtc_y;
	crtc_w = plane->state->crtc_w;
	crtc_h = plane->state->crtc_h;

	if (phytium_plane->dc_hw_update_dcreq)
		phytium_plane->dc_hw_update_dcreq(plane);
	phytium_plane->dc_hw_update_primary_hi_addr(plane);

	/* config dc */
	/* Y */
	base_offset = src_x * fb->format->cpp[0] + src_y*fb->pitches[0];
	phytium_writel_reg(priv, (phytium_plane->iova[0] + base_offset) & ADDRESS_MASK,
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_Y_ADDRESS);
	phytium_writel_reg(priv, ALIGN(fb->pitches[0], 128),
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_Y_STRIDE);

	/* U */
	phytium_writel_reg(priv, phytium_plane->iova[1] & 0xffffffff,
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_U_ADDRESS);
	phytium_writel_reg(priv, ALIGN(fb->pitches[1], 128),
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_U_STRIDE);

	/* V */
	phytium_writel_reg(priv, phytium_plane->iova[2] & 0xffffffff,
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_V_ADDRESS);
	phytium_writel_reg(priv, ALIGN(fb->pitches[2], 128),
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_V_STRIDE);

	/* size */
	phytium_writel_reg(priv, (crtc_w & WIDTH_MASK) | ((crtc_h&HEIGHT_MASK) << HEIGHT_SHIFT),
			   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_FRAMEBUFFER_SIZE);
	/* config */
	config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe],
				   PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	config &= ~(FRAMEBUFFER_FORMAT_MASK << FRAMEBUFFER_FORMAT_SHIFT);
	config |= (phytium_plane->format << FRAMEBUFFER_FORMAT_SHIFT);
	config &= ~(1 << FRAMEBUFFER_UVSWIZZLE_SHIFT);
	config |= (phytium_plane->uv_swizzle << FRAMEBUFFER_UVSWIZZLE_SHIFT);
	config &= ~(FRAMEBUFFER_SWIZZLE_MASK << FRAMEBUFFER_SWIZZLE_SHIFT);
	config |= (phytium_plane->swizzle << FRAMEBUFFER_SWIZZLE_SHIFT);
	config &= ~(FRAMEBUFFER_TILE_MODE_MASK << FRAMEBUFFER_TILE_MODE_SHIFT);
	config |= (phytium_plane->tiling[0] << FRAMEBUFFER_TILE_MODE_SHIFT);
	config &= (~FRAMEBUFFER_CLEAR);
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_FRAMEBUFFER_CONFIG);
}

static void phytium_dc_cursor_plane_update(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	struct drm_framebuffer *fb = plane->state->fb;
	int phys_pipe = phytium_plane->phys_pipe;
	int config;
	unsigned long iova;

	phytium_plane->enable = 1;
	phytium_plane->cursor_hot_x = fb->hot_x;
	phytium_plane->cursor_hot_y = fb->hot_y;
	phytium_plane->cursor_x = plane->state->crtc_x + fb->hot_x;
	phytium_plane->cursor_y = plane->state->crtc_y + fb->hot_y;

	if (phytium_plane->cursor_x < 0) {
		phytium_plane->cursor_hot_x = plane->state->crtc_w - 1;
		phytium_plane->cursor_x = plane->state->crtc_x + phytium_plane->cursor_hot_x;
	}

	if (phytium_plane->cursor_y < 0) {
		phytium_plane->cursor_hot_y = plane->state->crtc_h - 1;
		phytium_plane->cursor_y = plane->state->crtc_y + phytium_plane->cursor_hot_y;
	}

	config = CURSOR_FORMAT_ARGB8888 |
		((phytium_plane->cursor_hot_y & CURSOR_HOT_Y_MASK) << CURSOR_HOT_Y_SHIFT) |
		((phytium_plane->cursor_hot_x & CURSOR_HOT_X_MASK) << CURSOR_HOT_X_SHIFT);
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe], PHYTIUM_DC_CURSOR_CONFIG);

	config = ((phytium_plane->cursor_x & CURSOR_X_MASK) << CURSOR_X_SHIFT) |
		 ((phytium_plane->cursor_y & CURSOR_Y_MASK) << CURSOR_Y_SHIFT);
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_CURSOR_LOCATION);
	iova = phytium_plane->iova[0];
	phytium_writel_reg(priv, iova & 0xffffffff, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_CURSOR_ADDRESS);
	if (phytium_plane->dc_hw_update_cursor_hi_addr)
		phytium_plane->dc_hw_update_cursor_hi_addr(plane, iova);
}

static void phytium_plane_atomic_update(struct drm_plane *plane,
						   struct drm_plane_state *old_state)
{
	struct drm_framebuffer *fb, *old_fb;

	DRM_DEBUG_KMS("update plane: type=%d\n", plane->type);
	if (!plane->state->crtc || !plane->state->fb)
		return;

	fb = plane->state->fb;
	old_fb = old_state->fb;

	if (fb)
		drm_framebuffer_get(fb);
	if (old_fb)
		drm_framebuffer_put(old_fb);

	phytium_dc_get_plane_parameter(plane);

	if (plane->type == DRM_PLANE_TYPE_PRIMARY)
		phytium_dc_primary_plane_update(plane);
	else if (plane->type == DRM_PLANE_TYPE_CURSOR)
		phytium_dc_cursor_plane_update(plane);
}

static void phytium_plane_atomic_disable(struct drm_plane *plane,
						    struct drm_plane_state *old_state)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	int phys_pipe = phytium_plane->phys_pipe;
	int config;
	struct drm_framebuffer *old_fb;

	old_fb = old_state->fb;
	if (old_fb)
		drm_framebuffer_put(old_fb);

	if (plane->type == DRM_PLANE_TYPE_PRIMARY) {
		phytium_writel_reg(priv, CLEAR_VALUE_RED, priv->dc_reg_base[phys_pipe],
				   PHYTIUM_DC_FRAMEBUFFER_CLEARVALUE);
		config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe],
					   PHYTIUM_DC_FRAMEBUFFER_CONFIG);
		config |= FRAMEBUFFER_CLEAR;
		phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe],
				   PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	} else if (plane->type == DRM_PLANE_TYPE_CURSOR) {
		phytium_writel_reg(priv, CURSOR_FORMAT_DISABLED,
				   priv->dc_reg_base[phys_pipe], PHYTIUM_DC_CURSOR_CONFIG);
	}
}

const struct drm_plane_helper_funcs phytium_plane_helper_funcs = {
	.prepare_fb =  phytium_plane_prepare_fb,
	.atomic_check	= phytium_plane_atomic_check,
	.atomic_update	= phytium_plane_atomic_update,
	.atomic_disable	= phytium_plane_atomic_disable,
};

struct phytium_plane *phytium_primary_plane_create(struct drm_device *dev, int phys_pipe)
{
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = NULL;
	struct phytium_plane_state *phytium_plane_state = NULL;
	int ret = 0;
	unsigned int flags = 0;
	const uint32_t *formats = NULL;
	uint32_t format_count;
	const uint64_t *format_modifiers;

	phytium_plane = kzalloc(sizeof(*phytium_plane), GFP_KERNEL);
	if (!phytium_plane) {
		ret = -ENOMEM;
		goto failed_malloc_plane;
	}

	phytium_plane_state = kzalloc(sizeof(*phytium_plane_state), GFP_KERNEL);
	if (!phytium_plane_state) {
		ret = -ENOMEM;
		goto failed_malloc_plane_state;
	}
	phytium_plane_state->base.plane = &phytium_plane->base;
	phytium_plane_state->base.rotation = DRM_MODE_ROTATE_0;
	phytium_plane->base.state = &phytium_plane_state->base;
	phytium_plane->phys_pipe = phys_pipe;

	if (IS_PX210(priv)) {
		phytium_plane->dc_hw_plane_get_format = px210_dc_hw_plane_get_primary_format;
		phytium_plane->dc_hw_update_dcreq = px210_dc_hw_update_dcreq;
		phytium_plane->dc_hw_update_primary_hi_addr = px210_dc_hw_update_primary_hi_addr;
		phytium_plane->dc_hw_update_cursor_hi_addr = NULL;
	}  else if (IS_PE220X(priv)) {
		phytium_plane->dc_hw_plane_get_format = pe220x_dc_hw_plane_get_primary_format;
		phytium_plane->dc_hw_update_dcreq = NULL;
		phytium_plane->dc_hw_update_primary_hi_addr = pe220x_dc_hw_update_primary_hi_addr;
		phytium_plane->dc_hw_update_cursor_hi_addr = NULL;
	}

	phytium_plane->dc_hw_plane_get_format(&format_modifiers, &formats, &format_count);
	ret = drm_universal_plane_init(dev, &phytium_plane->base, 0x0,
				       &phytium_plane_funcs, formats,
				       format_count,
				       format_modifiers,
				       DRM_PLANE_TYPE_PRIMARY, "primary %d", phys_pipe);

	if (ret)
		goto failed_plane_init;

	flags = DRM_MODE_ROTATE_0;
	drm_plane_create_rotation_property(&phytium_plane->base, DRM_MODE_ROTATE_0, flags);
	drm_plane_helper_add(&phytium_plane->base, &phytium_plane_helper_funcs);

	return phytium_plane;
failed_plane_init:
	kfree(phytium_plane_state);
failed_malloc_plane_state:
	kfree(phytium_plane);
failed_malloc_plane:
	return ERR_PTR(ret);
}

struct phytium_plane *phytium_cursor_plane_create(struct drm_device *dev, int phys_pipe)
{
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = NULL;
	struct phytium_plane_state *phytium_plane_state = NULL;
	int ret = 0;
	unsigned int flags = 0;
	const uint32_t *formats = NULL;
	uint32_t format_count;
	const uint64_t *format_modifiers;

	phytium_plane = kzalloc(sizeof(*phytium_plane), GFP_KERNEL);
	if (!phytium_plane) {
		ret = -ENOMEM;
		goto failed_malloc_plane;
	}

	phytium_plane_state = kzalloc(sizeof(*phytium_plane_state), GFP_KERNEL);
	if (!phytium_plane_state) {
		ret = -ENOMEM;
		goto failed_malloc_plane_state;
	}
	phytium_plane_state->base.plane = &phytium_plane->base;
	phytium_plane_state->base.rotation = DRM_MODE_ROTATE_0;
	phytium_plane->base.state = &phytium_plane_state->base;
	phytium_plane->phys_pipe = phys_pipe;

	if (IS_PX210(priv)) {
		phytium_plane->dc_hw_plane_get_format = px210_dc_hw_plane_get_cursor_format;
		phytium_plane->dc_hw_update_dcreq = NULL;
		phytium_plane->dc_hw_update_primary_hi_addr = NULL;
		phytium_plane->dc_hw_update_cursor_hi_addr = NULL;
	} else if (IS_PE220X(priv)) {
		phytium_plane->dc_hw_plane_get_format = pe220x_dc_hw_plane_get_cursor_format;
		phytium_plane->dc_hw_update_dcreq = NULL;
		phytium_plane->dc_hw_update_primary_hi_addr = NULL;
		phytium_plane->dc_hw_update_cursor_hi_addr = pe220x_dc_hw_update_cursor_hi_addr;
	}

	phytium_plane->dc_hw_plane_get_format(&format_modifiers, &formats, &format_count);
	ret = drm_universal_plane_init(dev, &phytium_plane->base, 0x0,
				       &phytium_plane_funcs,
				       formats, format_count,
				       format_modifiers,
				       DRM_PLANE_TYPE_CURSOR, "cursor %d", phys_pipe);

	if (ret)
		goto failed_plane_init;

	flags = DRM_MODE_ROTATE_0;
	drm_plane_create_rotation_property(&phytium_plane->base, DRM_MODE_ROTATE_0, flags);
	drm_plane_helper_add(&phytium_plane->base, &phytium_plane_helper_funcs);

	return phytium_plane;
failed_plane_init:
	kfree(phytium_plane_state);
failed_malloc_plane_state:
	kfree(phytium_plane);
failed_malloc_plane:
	return ERR_PTR(ret);
}
