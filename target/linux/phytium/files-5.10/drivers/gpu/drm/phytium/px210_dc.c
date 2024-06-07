// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <asm/neon.h>
#include <linux/delay.h>
#include "phytium_display_drv.h"
#include "px210_reg.h"
#include "phytium_crtc.h"
#include "phytium_plane.h"
#include "phytium_fb.h"
#include "phytium_gem.h"

static const unsigned int px210_primary_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_RGBA1010102,
	DRM_FORMAT_BGRA1010102,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBX8888,
	DRM_FORMAT_BGRX8888,
	DRM_FORMAT_ARGB4444,
	DRM_FORMAT_ABGR4444,
	DRM_FORMAT_RGBA4444,
	DRM_FORMAT_BGRA4444,
	DRM_FORMAT_XRGB4444,
	DRM_FORMAT_XBGR4444,
	DRM_FORMAT_RGBX4444,
	DRM_FORMAT_BGRX4444,
	DRM_FORMAT_ARGB1555,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_BGRA5551,
	DRM_FORMAT_XRGB1555,
	DRM_FORMAT_XBGR1555,
	DRM_FORMAT_RGBX5551,
	DRM_FORMAT_BGRX5551,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_UYVY,
};

static uint64_t px210_primary_formats_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_PHYTIUM_TILE_MODE0_FBCDC,
	DRM_FORMAT_MOD_PHYTIUM_TILE_MODE3_FBCDC,
	DRM_FORMAT_MOD_INVALID
};

static uint64_t px210_cursor_formats_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

static const unsigned int px210_cursor_formats[] = {
	DRM_FORMAT_ARGB8888,
};

void px210_dc_hw_vram_init(struct phytium_display_private *priv, resource_size_t vram_addr,
				   resource_size_t vram_size)
{
	uint32_t config;
	uint32_t group_offset = priv->address_transform_base;

	config = phytium_readl_reg(priv, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_SRC_ADDR);
	if (config)
		phytium_writel_reg(priv, config, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_SRC_ADDR);

	config = phytium_readl_reg(priv, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_SIZE);
	if (config)
		phytium_writel_reg(priv, config, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_SIZE);

	config = phytium_readl_reg(priv, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_DST_ADDR);
	if (config)
		phytium_writel_reg(priv, config, group_offset,
				   PX210_GPU_ADDRESS_TRANSFORM_DST_ADDR);

	phytium_writel_reg(priv, (vram_addr & SRC_ADDR_MASK) >> SRC_ADDR_OFFSET,
			   group_offset, PX210_DC_ADDRESS_TRANSFORM_SRC_ADDR);
	phytium_writel_reg(priv, (vram_size >> SIZE_OFFSET) | ADDRESS_TRANSFORM_ENABLE,
			   group_offset, PX210_DC_ADDRESS_TRANSFORM_SIZE);
	config = phytium_readl_reg(priv, group_offset, PX210_DC_ADDRESS_TRANSFORM_DST_ADDR);
	phytium_writel_reg(priv, config, group_offset, PX210_DC_ADDRESS_TRANSFORM_DST_ADDR);
}

void px210_dc_hw_clear_msi_irq(struct phytium_display_private *priv, uint32_t phys_pipe)
{
	phytium_writel_reg(priv, MSI_CLEAR, priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_MSI_CLEAR);
}

void px210_dc_hw_config_pix_clock(struct drm_crtc *crtc, int clock)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	uint32_t group_offset = priv->dcreq_reg_base[phys_pipe];
	int ret = 0;

	/* config pix clock */
	phytium_writel_reg(priv, FLAG_REQUEST | CMD_PIXEL_CLOCK | (clock & PIXEL_CLOCK_MASK),
			   group_offset, PX210_DCREQ_CMD_REGISTER);
	ret = phytium_wait_cmd_done(priv, group_offset + PX210_DCREQ_CMD_REGISTER,
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to set pixel clock\n", __func__);
}

void px210_dc_hw_disable(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int reset_timeout = 100;
	int config = 0;
	int phys_pipe = phytium_crtc->phys_pipe;

	// reset dc
	config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_CONTROL);
	phytium_writel_reg(priv, config | SOFT_RESET, priv->dc_reg_base[phys_pipe],
			   PX210_DC_CLOCK_CONTROL);
	phytium_writel_reg(priv, 0, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_CONTROL);
	do {
		config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_IDLE);
		if (config | IS_IDLE)
			break;
		mdelay(1);
		reset_timeout--;
	} while (reset_timeout);

	/* reset pix clock */
	px210_dc_hw_config_pix_clock(crtc, 0);

	// reset dc
	reset_timeout = 100;
	config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_CONTROL);
	phytium_writel_reg(priv, config | SOFT_RESET, priv->dc_reg_base[phys_pipe],
			   PX210_DC_CLOCK_CONTROL);
	phytium_writel_reg(priv, 0, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_CONTROL);
	do {
		config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe], PX210_DC_CLOCK_IDLE);
		if (config | IS_IDLE)
			break;
		mdelay(1);
		reset_timeout--;
	} while (reset_timeout);

	/* reset dcreq */
	phytium_writel_reg(priv, DCREQ_PLAN_A, priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_PLAN);
	phytium_writel_reg(priv, 0, priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_CONTROL);
	phytium_writel_reg(priv, DCREQ_RESET, priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_RESET);
	msleep(20);
	phytium_writel_reg(priv, (~DCREQ_RESET)&DCREQ_RESET_MASK,
			   priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_RESET);
}

int px210_dc_hw_fb_format_check(const struct drm_mode_fb_cmd2 *mode_cmd, int count)
{
	int ret = 0;

	switch (mode_cmd->modifier[count]) {
	case DRM_FORMAT_MOD_PHYTIUM_TILE_MODE0_FBCDC:
		switch (mode_cmd->pixel_format) {
		case DRM_FORMAT_ARGB4444:
		case DRM_FORMAT_ABGR4444:
		case DRM_FORMAT_RGBA4444:
		case DRM_FORMAT_BGRA4444:
		case DRM_FORMAT_XRGB4444:
		case DRM_FORMAT_XBGR4444:
		case DRM_FORMAT_RGBX4444:
		case DRM_FORMAT_BGRX4444:
		case DRM_FORMAT_ARGB1555:
		case DRM_FORMAT_ABGR1555:
		case DRM_FORMAT_RGBA5551:
		case DRM_FORMAT_BGRA5551:
		case DRM_FORMAT_XRGB1555:
		case DRM_FORMAT_XBGR1555:
		case DRM_FORMAT_RGBX5551:
		case DRM_FORMAT_BGRX5551:
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_BGR565:
		case DRM_FORMAT_YUYV:
		case DRM_FORMAT_UYVY:
			break;
		default:
			DRM_ERROR("TILE_MODE0_FBCDC not support DRM_FORMAT %d",
				   mode_cmd->pixel_format);
			ret = -EINVAL;
			goto error;
		}
		break;
	case DRM_FORMAT_MOD_PHYTIUM_TILE_MODE3_FBCDC:
		switch (mode_cmd->pixel_format) {
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
			break;
		default:
			DRM_ERROR("TILE_MODE3_FBCDC not support DRM_FORMAT %d",
				   mode_cmd->pixel_format);
			ret = -EINVAL;
			goto error;
		}
		break;
	case DRM_FORMAT_MOD_LINEAR:
		break;
	default:
		DRM_ERROR("unsupported fb modifier 0x%llx\n", mode_cmd->modifier[0]);
		ret = -EINVAL;
		goto error;
	}

	return 0;
error:
	return ret;
}

void px210_dc_hw_plane_get_primary_format(const uint64_t **format_modifiers,
							 const uint32_t **formats,
							 uint32_t *format_count)
{
	*format_modifiers = px210_primary_formats_modifiers;
	*formats = px210_primary_formats;
	*format_count = ARRAY_SIZE(px210_primary_formats);
}

void px210_dc_hw_plane_get_cursor_format(const uint64_t **format_modifiers,
							const uint32_t **formats,
							uint32_t *format_count)
{
	*format_modifiers = px210_cursor_formats_modifiers;
	*formats = px210_cursor_formats;
	*format_count = ARRAY_SIZE(px210_cursor_formats);
}

void px210_dc_hw_update_dcreq(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	int phys_pipe = phytium_plane->phys_pipe;
	uint32_t group_offset = priv->dcreq_reg_base[phys_pipe];
	int config;

	if (phytium_plane->tiling[0] == FRAMEBUFFER_LINEAR) {
		phytium_writel_reg(priv, DCREQ_MODE_LINEAR,
				   group_offset, PX210_DCREQ_PLANE0_CONFIG);
	} else {
		config = DCREQ_NO_LOSSY;
		if (phytium_plane->tiling[0] == FRAMEBUFFER_TILE_MODE0)
			config |= DCREQ_TILE_TYPE_MODE0;
		else if (phytium_plane->tiling[0] == FRAMEBUFFER_TILE_MODE3)
			config |= DCREQ_TILE_TYPE_MODE3;
		else
			config |= DCREQ_TILE_TYPE_MODE0;

		switch (phytium_plane->format) {
		case FRAMEBUFFER_FORMAT_ARGB8888:
		case FRAMEBUFFER_FORMAT_XRGB8888:
			config |= DCREQ_COLOURFORMAT_BGRA8888;
			break;
		case FRAMEBUFFER_FORMAT_ARGB2101010:
			config |= DCREQ_COLOURFORMAT_ARGB2101010;
			break;
		case FRAMEBUFFER_FORMAT_XRGB4444:
		case FRAMEBUFFER_FORMAT_ARGB4444:
			config |= DCREQ_COLOURFORMAT_ARGB4444;
			break;
		case FRAMEBUFFER_FORMAT_XRGB1555:
		case FRAMEBUFFER_FORMAT_ARGB1555:
			config |= DCREQ_COLOURFORMAT_ARGB1555;
			break;
		case FRAMEBUFFER_FORMAT_RGB565:
			config |= DCREQ_COLOURFORMAT_RGB565;
			break;
		case FRAMEBUFFER_FORMAT_YUYV:
			config |= DCREQ_COLOURFORMAT_YUYV;
			break;
		case FRAMEBUFFER_FORMAT_UYVY:
			config |= DCREQ_COLOURFORMAT_UYVY;
			break;
		}
		config |= DCREQ_ARGBSWIZZLE_ARGB;
		config |= DCREQ_MODE_TILE;
		phytium_writel_reg(priv, phytium_plane->iova[0] & 0xffffffff,
				   group_offset, PX210_DCREQ_PLANE0_ADDR_START);
		phytium_writel_reg(priv, (phytium_plane->iova[0] + phytium_plane->size[0]) &
				   0xffffffff, group_offset, PX210_DCREQ_PLANE0_ADDR_END);
		phytium_writel_reg(priv, config, group_offset, PX210_DCREQ_PLANE0_CONFIG);
	}
}

void px210_dc_hw_update_primary_hi_addr(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	int phys_pipe = phytium_plane->phys_pipe;

	phytium_writel_reg(priv, (phytium_plane->iova[0] >> PREFIX_SHIFT) & PREFIX_MASK,
			   priv->dcreq_reg_base[phys_pipe], PX210_DCREQ_PIX_DMA_PREFIX);
}
