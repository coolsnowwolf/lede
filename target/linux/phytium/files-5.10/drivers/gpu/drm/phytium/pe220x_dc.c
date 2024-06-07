// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium Pe220x display controller DRM driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <asm/neon.h>
#include <linux/delay.h>
#include "phytium_display_drv.h"
#include "pe220x_reg.h"
#include "phytium_crtc.h"
#include "phytium_plane.h"
#include "phytium_fb.h"
#include "phytium_gem.h"

void pe220x_dc_hw_disable(struct drm_crtc *crtc);

static const unsigned int pe220x_primary_formats[] = {
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
	DRM_FORMAT_NV16,
	DRM_FORMAT_NV12,
	DRM_FORMAT_NV21,
};

static uint64_t pe220x_primary_formats_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

static uint64_t pe220x_cursor_formats_modifiers[] = {
	DRM_FORMAT_MOD_LINEAR,
	DRM_FORMAT_MOD_INVALID
};

static const unsigned int pe220x_cursor_formats[] = {
	DRM_FORMAT_ARGB8888,
};

void pe220x_dc_hw_vram_init(struct phytium_display_private *priv, resource_size_t vram_addr,
			   resource_size_t vram_size)
{
	uint32_t config;
	uint32_t group_offset = priv->address_transform_base;

	phytium_writel_reg(priv, (vram_addr & SRC_ADDR_MASK) >> SRC_ADDR_OFFSET,
			   group_offset, PE220X_DC_ADDRESS_TRANSFORM_SRC_ADDR);
	phytium_writel_reg(priv, (vram_size >> SIZE_OFFSET) | ADDRESS_TRANSFORM_ENABLE,
			   group_offset, PE220X_DC_ADDRESS_TRANSFORM_SIZE);
	config = phytium_readl_reg(priv, group_offset, PE220X_DC_ADDRESS_TRANSFORM_DST_ADDR);
	phytium_writel_reg(priv, config, group_offset, PE220X_DC_ADDRESS_TRANSFORM_DST_ADDR);
}

void pe220x_dc_hw_config_pix_clock(struct drm_crtc *crtc, int clock)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	int ret = 0;

	/* config pix clock */
	phytium_writel_reg(priv, FLAG_REQUEST | CMD_PIXEL_CLOCK | (clock & PIXEL_CLOCK_MASK),
			   0, PE220X_DC_CMD_REGISTER(phys_pipe));
	ret = phytium_wait_cmd_done(priv, PE220X_DC_CMD_REGISTER(phys_pipe),
				    FLAG_REQUEST, FLAG_REPLY);
	if (ret < 0)
		DRM_ERROR("%s: failed to set pixel clock\n", __func__);
}

void pe220x_dc_hw_reset(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int config = 0;
	int phys_pipe = phytium_crtc->phys_pipe;

	/* disable pixel clock for bmc mode */
	if (phys_pipe == 0)
		pe220x_dc_hw_disable(crtc);

	config = phytium_readl_reg(priv, 0, PE220X_DC_CLOCK_CONTROL);
	config &= (~(DC0_CORE_RESET | DC1_CORE_RESET | AXI_RESET | AHB_RESET));

	if (phys_pipe == 0) {
		phytium_writel_reg(priv, config | DC0_CORE_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC0_CORE_RESET | AXI_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC0_CORE_RESET | AXI_RESET | AHB_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC0_CORE_RESET | AXI_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC0_CORE_RESET,
				      0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config, 0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
	} else {
		phytium_writel_reg(priv, config | DC1_CORE_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC1_CORE_RESET | AXI_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC1_CORE_RESET | AXI_RESET | AHB_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC1_CORE_RESET | AXI_RESET,
				   0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config | DC1_CORE_RESET,
				      0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config, 0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
	}
}

void pe220x_dc_hw_disable(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int config = 0;
	int phys_pipe = phytium_crtc->phys_pipe;

	/* clear framebuffer */
	phytium_writel_reg(priv, CLEAR_VALUE_BLACK, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_FRAMEBUFFER_CLEARVALUE);
	config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe],
				   PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	config |= FRAMEBUFFER_CLEAR;
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_FRAMEBUFFER_CONFIG);

	/* disable cursor */
	config = phytium_readl_reg(priv, priv->dc_reg_base[phys_pipe], PHYTIUM_DC_CURSOR_CONFIG);
	config = ((config & (~CURSOR_FORMAT_MASK)) | CURSOR_FORMAT_DISABLED);
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe], PHYTIUM_DC_CURSOR_CONFIG);
	mdelay(20);

	/* reset pix clock */
	pe220x_dc_hw_config_pix_clock(crtc, 0);

	if (phys_pipe == 0) {
		config = phytium_readl_reg(priv, 0, PE220X_DC_CLOCK_CONTROL);
		phytium_writel_reg(priv, config | DC0_CORE_RESET, 0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config & (~DC0_CORE_RESET), 0, PE220X_DC_CLOCK_CONTROL);
	} else {
		config = phytium_readl_reg(priv, 0, PE220X_DC_CLOCK_CONTROL);
		phytium_writel_reg(priv, config | DC1_CORE_RESET, 0, PE220X_DC_CLOCK_CONTROL);
		udelay(20);
		phytium_writel_reg(priv, config & (~DC1_CORE_RESET), 0, PE220X_DC_CLOCK_CONTROL);
	}
	udelay(20);
}

int pe220x_dc_hw_fb_format_check(const struct drm_mode_fb_cmd2 *mode_cmd, int count)
{
	int ret = 0;

	if (mode_cmd->modifier[count] != DRM_FORMAT_MOD_LINEAR) {
		DRM_ERROR("unsupported fb modifier 0x%llx\n", mode_cmd->modifier[count]);
		ret = -EINVAL;
	}

	return ret;
}

void pe220x_dc_hw_plane_get_primary_format(const uint64_t **format_modifiers,
					  const uint32_t **formats,
					  uint32_t *format_count)
{
	*format_modifiers = pe220x_primary_formats_modifiers;
	*formats = pe220x_primary_formats;
	*format_count = ARRAY_SIZE(pe220x_primary_formats);
}

void pe220x_dc_hw_plane_get_cursor_format(const uint64_t **format_modifiers,
					 const uint32_t **formats,
					 uint32_t *format_count)
{
	*format_modifiers = pe220x_cursor_formats_modifiers;
	*formats = pe220x_cursor_formats;
	*format_count = ARRAY_SIZE(pe220x_cursor_formats);
}

void pe220x_dc_hw_update_primary_hi_addr(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	int phys_pipe = phytium_plane->phys_pipe;

	phytium_writel_reg(priv, (phytium_plane->iova[0] >> PREFIX_SHIFT) & PREFIX_MASK,
			   priv->dc_reg_base[phys_pipe], PE220X_DC_FRAMEBUFFER_Y_HI_ADDRESS);

	phytium_writel_reg(priv, (phytium_plane->iova[1] >> U_PREFIX_SHIFT) & U_PREFIX_MASK,
			   priv->dc_reg_base[phys_pipe], PE220X_DC_FRAMEBUFFER_U_HI_ADDRESS);

	phytium_writel_reg(priv, (phytium_plane->iova[2] >> V_PREFIX_SHIFT) & V_PREFIX_MASK,
			   priv->dc_reg_base[phys_pipe], PE220X_DC_FRAMEBUFFER_V_HI_ADDRESS);
}

void pe220x_dc_hw_update_cursor_hi_addr(struct drm_plane *plane, uint64_t iova)
{
	struct drm_device *dev = plane->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_plane *phytium_plane = to_phytium_plane(plane);
	int phys_pipe = phytium_plane->phys_pipe;
	int config;

	config = ((iova >> CURSOR_PREFIX_SHIFT) & CURSOR_PREFIX_MASK);
	phytium_writel_reg(priv, config, priv->dc_reg_base[phys_pipe], PE220X_DC_CURSOR_HI_ADDRESS);
}
