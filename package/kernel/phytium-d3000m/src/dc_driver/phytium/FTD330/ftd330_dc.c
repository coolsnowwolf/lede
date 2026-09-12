// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/component.h>
#include <linux/delay.h>
#include <linux/media-bus-format.h>

#include <drm/drm_atomic.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/ftd330_drm.h>
#include <linux/of_graph.h>

#include "phytium_dp.h"
#include "ftd330_crtc.h"
#include "ftd330_dc.h"
#include "ftd330_dc_hw.h"
#include "ftd330_drv.h"
#include "ftd330_type.h"
#include "ftd330_writeback.h"

#include <drm/ftd330_drm_fourcc.h>
#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#include <drm/drm_vblank.h>

#ifdef CONFIG_PCI
#include <linux/pci.h>
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
#include <linux/acpi.h>
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
#include "ftd330_debug.h"
#endif
#include "drm/ftd330_drm.h"
#include "phytium_parse_bios.h"

static inline void update_format(u32 format, u64 mod, struct dc_hw_fb *fb)
{
	u8 f = FORMAT_A8R8G8B8;

	switch (format) {
	case DRM_FORMAT_XRGB4444:
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_BGRX4444:
		f = FORMAT_X4R4G4B4;
		break;
	case DRM_FORMAT_ARGB4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_BGRA4444:
		f = FORMAT_A4R4G4B4;
		break;
	case DRM_FORMAT_XRGB1555:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_BGRX5551:
		f = FORMAT_X1R5G5B5;
		break;
	case DRM_FORMAT_ARGB1555:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_BGRA5551:
		f = FORMAT_A1R5G5B5;
		break;
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		f = FORMAT_R5G6B5;
		break;
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_BGRX8888:
		f = FORMAT_X8R8G8B8;
		break;
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_BGRA8888:
		f = FORMAT_A8R8G8B8;
		break;
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
		f = FORMAT_YUY2;
		break;
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
		f = FORMAT_UYVY;
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		f = FORMAT_YV12;
		break;
	case DRM_FORMAT_NV21:
		f = FORMAT_NV12;
		break;
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
		f = FORMAT_NV16;
		break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	case DRM_FORMAT_P010:
		f = FORMAT_P010;
		break;
#endif
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_BGRA1010102:
		f = FORMAT_A2R10G10B10;
		break;
	case DRM_FORMAT_NV12:
		if (fourcc_mod_is_custom_format(mod))
			f = FORMAT_NV12_10BIT;
		else
			f = FORMAT_NV12;
		break;
	case DRM_FORMAT_YUV444:
		if (fourcc_mod_is_custom_format(mod))
			f = FORMAT_YUV444_10BIT;
		else
			f = FORMAT_YUV444;
		break;
	case DRM_FORMAT_RGB888:
		if (fourcc_mod_is_custom_format(mod))
			f = FORMAT_RGB888P;
		break;
	default:
		break;
	}

	fb->format = f;
}

static inline void update_swizzle(u32 format, struct dc_hw_fb *fb)
{
	fb->swizzle = SWIZZLE_ARGB;
	fb->uv_swizzle = 0;

	switch (format) {
	case DRM_FORMAT_RGBX4444:
	case DRM_FORMAT_RGBA4444:
	case DRM_FORMAT_RGBX5551:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBA1010102:
		fb->swizzle = SWIZZLE_RGBA;
		break;
	case DRM_FORMAT_XBGR4444:
	case DRM_FORMAT_ABGR4444:
	case DRM_FORMAT_XBGR1555:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_ABGR2101010:
		fb->swizzle = SWIZZLE_ABGR;
		break;
	case DRM_FORMAT_BGRX4444:
	case DRM_FORMAT_BGRA4444:
	case DRM_FORMAT_BGRX5551:
	case DRM_FORMAT_BGRA5551:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRA1010102:
		fb->swizzle = SWIZZLE_BGRA;
		break;
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV420:
		fb->uv_swizzle = 1;
		break;
	default:
		break;
	}
}

static inline void update_watermark(struct drm_property_blob *watermark, struct dc_hw_fb *fb)
{
	struct drm_ftd330_watermark *data;

	fb->water_mark = 0;

	if (watermark) {
		data = watermark->data;
		fb->water_mark = data->watermark & 0xFFFFF;
	}
}

static inline u8 to_ftd330_rotation(u32 rotation)
{
		u8 rot;

	switch (rotation & (DRM_MODE_ROTATE_MASK | DRM_MODE_REFLECT_MASK)) {
	case DRM_MODE_ROTATE_0:
		rot = ROT_0;
		break;
	case DRM_MODE_ROTATE_90:
		rot = ROT_90;
		break;
	case DRM_MODE_ROTATE_180:
		rot = ROT_180;
		break;
	case DRM_MODE_ROTATE_270:
		rot = ROT_270;
		break;
	case DRM_MODE_REFLECT_X | DRM_MODE_ROTATE_0:
		rot = FLIP_X;
		break;
	case DRM_MODE_REFLECT_Y | DRM_MODE_ROTATE_0:
		rot = FLIP_Y;
		break;
	case DRM_MODE_REFLECT_X | DRM_MODE_REFLECT_Y | DRM_MODE_ROTATE_0:
		rot = FLIP_XY;
		break;
	default:
		rot = ROT_0;
		break;
	}

	return rot;
}

static inline u8 to_ftd330_yuv_gamut(u32 color_space)
{
	u8 gamut;

	switch (color_space) {
	case DRM_COLOR_YCBCR_BT601:
		gamut = CSC_GAMUT_601;
		break;
	case DRM_COLOR_YCBCR_BT709:
		gamut = CSC_GAMUT_709;
		break;
	case DRM_COLOR_YCBCR_BT2020:
		gamut = CSC_GAMUT_2020;
		break;
	default:
		gamut = CSC_GAMUT_2020;
		break;
	}

	return gamut;
}

static inline u8 to_ftd330_tile_mode(u64 modifier)
{
	if (modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED)
		return DRM_FORMAT_MOD_FTD330_SUPER_TILED_XMAJOR_8X4;
	else
		return (u8)(modifier & DRM_FORMAT_MOD_FTD330_NORM_MODE_MASK);
}

static inline u8 to_ftd330_display_id(struct ftd330_dc *dc, struct drm_crtc *crtc)
{
	u8 display_num = dc->hw.info->display_num;
	u32 index = drm_crtc_index(crtc);
	int i;

	for (i = 0; i < display_num; i++) {
		if (index == dc->crtc[i]->base.index)
			return i;
	}

	return 0;
}

#ifdef CONFIG_PM_SLEEP
int ftd330_dc_suspend(struct device *dev)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm = dc->hw.drm_dev;
	struct ftd330_drm_private *priv = drm->dev_private;
	int i = 0;

	for (i = 0;i < priv->info.total_pipes;i++) {
		 dc_hw_enable_interrupt(&dc->hw, false, i);
	}
	
	return 0;
}
int ftd330_dc_resume(struct device *dev)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_device *drm = dc->hw.drm_dev;
	struct ftd330_drm_private *priv = drm->dev_private;

	if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
		phytium_dc_registers_init(priv, DC_0);
	}

	if ((priv->info.pipe_mask & BIT(DISPLAY_1)) || (priv->info.pipe_mask & BIT(DISPLAY_2))) {
		phytium_dc_registers_init(priv, DC_1);

     }

	return 0;

}
#endif
static void dc_deinit(struct device *dev)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	int i = 0;

	for (i = 0; i < dc->hw.total_pipes; i++)
		dc_hw_enable_interrupt(&dc->hw, false, i);
	dc_hw_deinit(&dc->hw);

	// clk_disable_unprepare(dc->core_clk);
	// clk_disable_unprepare(dc->pix_clk);
	// clk_disable_unprepare(dc->axi_clk);

}

static int dc_init(struct device *dev)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	int ret;

	dc->first_frame = true;

	//ret = clk_prepare_enable(dc->core_clk);
	//if (ret < 0) {
	//	dev_err(dev, "failed to prepare/enable core_clk\n");
	//	return ret;
	//}

	//ret = clk_prepare_enable(dc->pix_clk);
	//if (ret < 0) {
	//	dev_err(dev, "failed to prepare/enable pix_clk\n");
	//	goto err_unprepare_core_clk;
	//}

	//ret = clk_prepare_enable(dc->axi_clk);
	//if (ret < 0) {
	//	dev_err(dev, "failed to prepare/enable axi_clk\n");
	//	goto err_unprepare_pix_clk;
	//}

	//dc->pix_clk_rate = clk_get_rate(dc->pix_clk);

	ret = dc_hw_init(&dc->hw);
	if (ret) {
		dev_err(dev, "failed to init DC HW\n");
		return ret;
	}

	return 0;

//err_unprepare_axi_clk:
//	clk_disable_unprepare(dc->axi_clk);
//err_unprepare_core_clk:
//	clk_disable_unprepare(dc->core_clk);
//err_unprepare_pix_clk:
//	clk_disable_unprepare(dc->pix_clk);
//	return ret;

}

static void ftd330_dc_enable(struct device *dev, struct drm_crtc *crtc)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	const struct ftd330_display_info display_info = dc->hw.info->displays[ftd330_crtc->id];
	struct ftd330_crtc_state *crtc_state = to_ftd330_crtc_state(crtc->state);
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct dc_hw_display_mode display = { 0 };
	const struct drm_ftd330_r2y_config *r2y_config = NULL;

	/* For convernienting to debug,
	 * get the output bus format from r2y config,
	 * the default output bus format is MEDIA_BUS_FMT_RGB888_1X24
	 */
	if (display_info.color_formats &
	    (DRM_COLOR_FORMAT_YCBCR444 | DRM_COLOR_FORMAT_YCBCR422 | DRM_COLOR_FORMAT_YCBCR420))
		r2y_config = ftd330_dc_drm_crtc_property_get(crtc_state, "R2Y", NULL);
	if (r2y_config)
		crtc_state->output_fmt = r2y_config->output_bus_format;

	display.bus_format = crtc_state->output_fmt;

	if (dc_fake_mode_enable == 1) {
		display.clock = mode->crtc_clock;
		display.h_active = mode->crtc_hdisplay;
		display.h_total = mode->crtc_htotal;
		display.h_sync_start = mode->crtc_hsync_start;
		display.h_sync_end = mode->crtc_hsync_end;

		display.v_active = mode->crtc_vdisplay;
		display.v_total = mode->crtc_vtotal;
		display.v_sync_start = mode->crtc_vsync_start;
		display.v_sync_end = mode->crtc_vsync_end;
	} else {
		display.clock = mode->clock;
		display.h_active = mode->hdisplay;
		display.h_total = mode->htotal;
		display.h_sync_start = mode->hsync_start;
		display.h_sync_end = mode->hsync_end;

		display.v_active = mode->vdisplay;
		display.v_total = mode->vtotal;
		display.v_sync_start = mode->vsync_start;
		display.v_sync_end = mode->vsync_end;
	}

	if (mode->flags & DRM_MODE_FLAG_PHSYNC)
		display.h_sync_polarity = true;
	else
		display.h_sync_polarity = false;

	if (mode->flags & DRM_MODE_FLAG_PVSYNC)
		display.v_sync_polarity = true;
	else
		display.v_sync_polarity = false;

	display.enable = true;

	//if (dc->pix_clk_rate != mode->clock) {
	//	clk_set_rate(dc->pix_clk, mode->clock * 1000);
	//	dc->pix_clk_rate = mode->clock;
	//}

	if (crtc_state->encoder_type == DRM_MODE_ENCODER_VIRTUAL) {
		if (crtc_state->out_dp)
			display.out = OUT_DP;
		else
			display.out = OUT_DPI;
	} else if (crtc_state->encoder_type == DRM_MODE_ENCODER_DSI ||
			   crtc_state->encoder_type == DRM_MODE_ENCODER_DPI) {
		display.out = OUT_DPI;
	} else if (crtc_state->encoder_type == DRM_MODE_ENCODER_DPMST) {
		display.out = OUT_DP;
	} else {
		display.out = OUT_DPI;
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (crtc_state->mmu_prefetch == FTD330_MMU_PREFETCH_ENABLE)
		dc_hw_enable_mmu_prefetch(&dc->hw, true);
	else
		dc_hw_enable_mmu_prefetch(&dc->hw, false);
#endif

	dc_hw_setup_display_mode(&dc->hw, ftd330_crtc->id, &display);
}

static void ftd330_dc_disable(struct device *dev, struct drm_crtc *crtc)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct dc_hw_display_mode display;

	display.enable = false;

	dc_hw_setup_display_mode(&dc->hw, ftd330_crtc->id, &display);
}

static bool ftd330_dc_mode_fixup(struct device *dev, const struct drm_display_mode *mode,
			     struct drm_display_mode *adjusted_mode)
{

	// struct ftd330_dc *dc = dev_get_drvdata(dev);
	// long clk_rate;

	// if (dc->pix_clk) {
	//	clk_rate = clk_round_rate(dc->pix_clk, adjusted_mode->clock * 1000);
	//	adjusted_mode->clock = DIV_ROUND_UP(clk_rate, 1000);
	// }

	return true;
}

static void update_display_ctm(struct ftd330_dc *dc, u8 id, struct drm_crtc *crtc)
{
	struct drm_crtc_state *crtc_state = crtc->state;
	struct drm_property_blob *blob = crtc_state->ctm;
	struct drm_color_ctm *ctm;

	if (crtc_state->color_mgmt_changed) {
		if (blob && blob->length) {
			ctm = blob->data;
			dc_hw_update_ctm(&dc->hw, id, ctm->matrix);
			dc_hw_enable_ctm(&dc->hw, id, true);
		} else {
			dc_hw_enable_ctm(&dc->hw, id, false);
		}
	}
}

/*map receives 256entry, 16bit gamma table from xorg*/
static void mapXorg256Gammato300(struct drm_color_lut** lut) {
	u16 i, index;
	u16 pointLeft_r = (*lut)[0].red;
	u16 pointRight_r = (*lut)[1].red;
	u16 pointLeft_g = (*lut)[0].green;
	u16 pointRight_g = (*lut)[1].green;
	u16 pointLeft_b = (*lut)[0].blue;
	u16 pointRight_b = (*lut)[1].blue;
	u16 Step = 1 << 6;
	u16 Weight;
	struct drm_color_lut *out_lut =  (struct drm_color_lut *)kmalloc(300 * sizeof(struct drm_color_lut), GFP_KERNEL);

	index = 0;
	for (i = 0; i < 32; i++) {
		Weight = i;
		out_lut[index].red = (((u32)pointLeft_r * (Step - Weight) + (u32)pointRight_r * Weight + (1 << 9)) >> 10) & 0xFFF;
		out_lut[index].green = (((u32)pointLeft_g * (Step - Weight) + (u32)pointRight_g * Weight + (1 << 9)) >> 10) & 0xFFF;
		out_lut[index].blue = (((u32)pointLeft_b * (Step - Weight) + (u32)pointRight_b * Weight + (1 << 9)) >> 10) & 0xFFF;
		index++;
	}

	for (i = 32; i <= 256; i += 16) {
		pointLeft_r = (*lut)[(int)(i / 64)].red;
		pointRight_r = (*lut)[(int)(i / 64) + 1].red;
		pointLeft_g = (*lut)[(int)(i / 64)].green;
		pointRight_g = (*lut)[(int)(i / 64) + 1].green;
		pointLeft_b = (*lut)[(int)(i / 64)].blue;
		pointRight_b = (*lut)[(int)(i / 64) + 1].blue;
		Weight = i % 64;
		out_lut[index].red = (((u32)pointLeft_r * (Step - Weight) + (u32)pointRight_r * Weight + (1 << 9)) >> 10) & 0xFFF;
		out_lut[index].green = (((u32)pointLeft_g * (Step - Weight) + (u32)pointRight_g * Weight + (1 << 9)) >> 10) & 0xFFF;
		out_lut[index].blue = (((u32)pointLeft_b * (Step - Weight) + (u32)pointRight_b * Weight + (1 << 9)) >> 10) & 0xFFF;
		index++;
	}

	for (i = 4; i < 256; i++) {
		out_lut[index].red = ((*lut)[i].red >> 4) & 0xFFF;
		out_lut[index].green = ((*lut)[i].green >> 4) & 0xFFF;
		out_lut[index].blue = ((*lut)[i].blue >> 4) & 0xFFF;
		index++;
	}
	out_lut[299].red = out_lut[298].red;
	out_lut[299].green = out_lut[298].green;
	out_lut[299].blue = out_lut[298].blue;

	*lut = out_lut;
}


static void update_display_gamma(struct ftd330_dc *dc, u8 id, struct drm_crtc *crtc)
{
	struct drm_crtc_state *crtc_state = crtc->state;
	struct drm_property_blob *blob = crtc_state->gamma_lut;
	struct drm_color_lut *lut = NULL;
	u16 i, size;

	/* Standard GAMMA_LUT property entry point. */
	if (crtc_state->color_mgmt_changed) {

		if (blob && blob->length) {
			lut = blob->data;
			size = blob->length / sizeof(*lut);

			if (size != dc->hw.info->max_gamma_size) {
				pr_err("gamma size does not match!\n");
				return;
			}

			mapXorg256Gammato300(&lut);
			for (i = 0; i < GAMMA_EX_SIZE; i++) {
				dc_hw_update_gamma(&dc->hw, id, i, lut[i].red, lut[i].green,
						   lut[i].blue);
			}

			dc_hw_enable_gamma(&dc->hw, id, true);
		} else {
			dc_hw_enable_gamma(&dc->hw, id, false);
		}
	}

	if (lut != NULL)
		kfree(lut);
}

static void ftd330_dc_conf_display(struct device *dev, struct drm_crtc *crtc)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct ftd330_crtc_state *crtc_state = to_ftd330_crtc_state(crtc->state);
	struct ftd330_display_info *display_info;
	struct dc_hw_display *display = &dc->hw.display[ftd330_crtc->id];

	display_info = (struct ftd330_display_info *)&dc->hw.info->displays[ftd330_crtc->id];
	if (!display_info) {
		pr_err("%s: Invalid ftd330_crtc index.\n", __func__);
		return;
	}

	display->sync_mode = crtc_state->sync_mode;
	display->sync_enable = crtc_state->sync_enable;

	update_display_ctm(dc, ftd330_crtc->id, crtc);
	update_display_gamma(dc, ftd330_crtc->id, crtc);

	/* update dc porperty */
	ftd330_dc_update_drm_properties_to_dc(dc, ftd330_crtc->id, crtc_state->drm_states,
					  ftd330_crtc->properties.num, &display->states, crtc_state);
}


static int ftd330_dc_check_display(struct device *dev, struct drm_crtc *crtc,
			       struct drm_crtc_state *state)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(state);
	struct ftd330_display_info *display_info;
	int ret = 0;
	struct drm_atomic_state *atomic_state = state->state;
	struct drm_connector *connector;
	struct drm_connector_state *new_con_state;
	uint32_t i;
	struct phytium_dp_device *phytium_dp = NULL;


	display_info = (struct ftd330_display_info *)&dc->hw.info->displays[ftd330_crtc->id];
	if (!display_info)
		return -EINVAL;

	if (!ftd330_dc_check_crtc_std_property(dc, display_info->id, crtc))
		return -EINVAL;
	/* check dc porperty */
	if (!ftd330_dc_check_drm_property(dc, display_info->id, ftd330_crtc_state->drm_states,
				      ftd330_crtc->properties.num, ftd330_crtc_state))
		return -EINVAL;

	for_each_new_connector_in_state(atomic_state, connector, new_con_state, i) {
		if (new_con_state->crtc == crtc) {
			phytium_dp = connector_to_dp_device(connector);
			break;
		}
	}

	if (phytium_dp)
		phytium_update_timing_for_drm_display_mode(&state->adjusted_mode,&phytium_dp->native_mode);

	return ret;
}

static void ftd330_dc_enable_vblank(struct ftd330_crtc *crtc, bool enable)
{
	struct device *dev = crtc->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);

	dc_hw_enable_interrupt(&dc->hw, enable, crtc->id);
}

static u32 ftd330_dc_get_vblank_count(struct ftd330_crtc *crtc)
{
	struct device *dev = crtc->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);

	return dc_hw_get_vblank_count(&dc->hw, crtc->id);
}

static void update_fb(struct ftd330_plane *plane, u8 display_id, struct dc_hw_fb *fb)
{
	struct drm_plane_state *state = plane->base.state;
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(state);
	struct drm_framebuffer *drm_fb = state->fb;

	update_format(drm_fb->format->format, drm_fb->modifier, fb);

	fb->display_id = display_id;
	fb->y_address = (u32)plane->dma_addr[0];
	fb->y_stride = drm_fb->pitches[0];
	if (fb->format == FORMAT_YV12) {
		fb->u_address = (u32)plane->dma_addr[2];
		fb->v_address = (u32)plane->dma_addr[1];
	} else {
		fb->u_address = (u32)plane->dma_addr[1];
		fb->v_address = (u32)plane->dma_addr[2];
	}
	fb->u_stride = drm_fb->pitches[1];
	fb->v_stride = drm_fb->pitches[2];

	fb->width = drm_fb->width;
	fb->height = drm_fb->height;
	fb->tile_mode = to_ftd330_tile_mode(drm_fb->modifier);
	fb->rotation = to_ftd330_rotation(state->rotation);
	fb->yuv_gamut = to_ftd330_yuv_gamut(state->color_encoding);
	fb->zpos = state->zpos;
	fb->enable = state->visible;

	update_swizzle(drm_fb->format->format, fb);
	update_watermark(plane_state->watermark, fb);
	plane_state->status.tile_mode = fb->tile_mode;
}

#ifdef CONFIG_PHYTIUM_DEC
static u8 get_stream_base(struct ftd330_dc *dc, u8 id)
{
	u8 stream_base = 0;

	switch (id) {
	case PRIMARY_PLANE_0:
		stream_base = 0;
		break;
	case OVERLAY_PLANE_0:
		stream_base = 3;
		break;
	case OVERLAY_PLANE_1:
		stream_base = 6;
		break;
	case PRIMARY_PLANE_1:
		stream_base = 9;
		break;
	case OVERLAY_PLANE_2:
		stream_base = 12;
		break;
	case OVERLAY_PLANE_3:
		stream_base = 15;
		break;
	case PRIMARY_PLANE_2:
		stream_base = 18;
		break;
	case OVERLAY_PLANE_4:
		stream_base = 21;
		break;
	case OVERLAY_PLANE_5:
		stream_base = 24;
		break;
	default:
		break;
	}

	return stream_base;
}

static void update_fbc(struct ftd330_dc *dc, struct ftd330_plane *plane, bool *enable)
{
	struct dc_dec_fb dec_fb;
	struct drm_plane_state *state = plane->base.state;
	struct drm_framebuffer *drm_fb = state->fb;
	struct ftd330_dc_plane *dc_plane = &dc->planes[plane->id];
	u8 i, stream_id;

	if (!dc->hw.info->cap_dec) {
		*enable = false;
		return;
	}

	stream_id = get_stream_base(dc, dc_plane->id);
	memset(&dec_fb, 0, sizeof(struct dc_dec_fb));
	dec_fb.fb = drm_fb;

	if ((fourcc_mod_ftd330_get_type(drm_fb->modifier) == DRM_FORMAT_MOD_FTD330_TYPE_COMPRESSED) ||
	    drm_fb->modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC) {
		*enable = true;

		for (i = 0; i < DEC_PLANE_MAX; i++) {
			dec_fb.addr[i] = (u32)plane->dma_addr[i];
			dec_fb.stride[i] = drm_fb->pitches[i];
			dec_fb.ts_addr[i] = (u32)plane->ts_addr[i];
			dec_fb.ts_dma_buf[i] = plane->ts_dma_buf[i];
		}
	} else {
		*enable = false;
	}

	dc_dec_config(&dc->dec400l, &dec_fb, stream_id);
}

static void disable_fbc(struct ftd330_dc *dc, struct ftd330_plane *plane)
{
	struct ftd330_dc_plane *dc_plane = &dc->planes[plane->id];
	u8 stream_id;

	if (!dc->hw.info->cap_dec)
		return;

	stream_id = get_stream_base(dc, dc_plane->id);
	dc_dec_config(&dc->dec400l, NULL, stream_id);
}
#endif


static bool ftd330_dc_is_yuv_format(u32 format)
{
	bool is_yuv = false;

	switch (format) {
	case FORMAT_YUY2:
	case FORMAT_UYVY:
	case FORMAT_YV12:
	case FORMAT_NV12:
	case FORMAT_NV16:
	case FORMAT_P010:
	case FORMAT_NV12_10BIT:
	case FORMAT_NV16_10BIT:
	case FORMAT_YUV444:
	case FORMAT_YUV444_10BIT:
		is_yuv = true;
		break;
	default:
		break;
	}

	return is_yuv;
}
static void update_plane_y2r(struct ftd330_dc *dc, u8 id, struct ftd330_plane_state *plane_state)
{
	struct dc_hw_y2r y2r_conf = { 0 };

	y2r_conf.gamut = to_ftd330_yuv_gamut(plane_state->base.color_encoding);

	if (plane_state->base.color_range == DRM_COLOR_YCBCR_FULL_RANGE)
		y2r_conf.mode = CSC_MODE_F2F;
	else
		y2r_conf.mode = CSC_MODE_L2L;

	dc_hw_update_plane_y2r(&dc->hw, id, &y2r_conf);
}
static void update_std_blend(struct ftd330_plane_info *plane_info, struct ftd330_plane_state *plane_state,
			     struct dc_hw_blend *blend)
{
	struct ftd330_drm_property_state *bld_mode = NULL;

	if (!plane_info->blend_config && !plane_info->blend_mode)
		return;

	bld_mode = ftd330_dc_get_drm_property_state(plane_state->drm_states, FTD330_DC_MAX_PROPERTY_NUM,
						"BLEND_MODE");

	if (!bld_mode->is_changed && plane_info->blend_mode) {
		blend->alpha = (u8)(plane_state->base.alpha & 0xFF);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
		blend->blend_mode = (u8)(plane_state->base.pixel_blend_mode);
#endif
		blend->enable = true;
	} else {
		blend->enable = false;
	}
}

static void update_std_roi(struct ftd330_plane_info *plane_info, struct ftd330_plane_state *plane_state,
			     struct dc_hw_roi *roi)
{
	struct drm_framebuffer *drm_fb = plane_state->base.fb;
	u32 in_x = plane_state->base.src_x >> 16;
	u32 in_y = plane_state->base.src_y >> 16;
	u32 in_width = plane_state->base.src_w >> 16;
	u32 in_height = plane_state->base.src_h >> 16;
	
	if (!plane_info->roi)
		return;
	
	if (in_width <= drm_fb->width || in_height <= drm_fb->height) {
		roi->enable = true;
		roi->dirty = true;
		roi->x = in_x;
		roi->y = in_y;
		roi->width = in_width;
 		roi->height = in_height;
 	} else {
		roi->dirty = true;
		roi->enable = false;
	}
	return;
}

static void update_std_scale(struct dc_hw *hw, u8 plane_id, struct dc_hw_position *pos)
{
	phytium_dc_scaling_config(hw, plane_id, pos);
}



static void update_plane(struct ftd330_dc *dc, struct ftd330_plane *plane)
{
	struct dc_hw_fb fb = { 0 };
	struct dc_hw_position pos;
	struct dc_hw_blend blend;
	struct dc_hw_roi roi = { 0 };
	struct drm_plane_state *state = plane->base.state;
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(state);
	u64 fb_modifier = plane_state->base.fb->modifier;
	struct drm_rect *dest = &state->dst;
	bool dec_enable = false;
	u8 display_id = 0;
	struct ftd330_plane_info *plane_info;
	struct dc_hw_plane *hw_plane = &dc->hw.plane[plane->id];

	plane_info = (struct ftd330_plane_info *)&dc->hw.info->planes[plane->id];
	if (!plane_info) {
		pr_err("%s: Invalid ftd330_plane index.\n", __func__);
		return;
	}

#ifdef CONFIG_PHYTIUM_DEC
	update_fbc(dc, plane, &dec_enable);
#endif

	display_id = to_ftd330_display_id(dc, state->crtc);
	update_fb(plane, display_id, &fb);
	fb.dec_enable = dec_enable;
	plane_state->base.fb->modifier = fb_modifier;

	if (ftd330_dc_is_yuv_format(fb.format))
		update_plane_y2r(dc, plane->id, plane_state);

	pos.start_x = dest->x1;
	pos.start_y = dest->y1;
	pos.end_x = dest->x2;
	pos.end_y = dest->y2;

	update_std_blend(plane_info, plane_state, &blend);

	update_std_roi(plane_info, plane_state, &roi);

	update_std_scale(&dc->hw, plane->id, &pos);

	dc_hw_update_plane(&dc->hw, plane->id, &fb, &pos, &blend, &roi);

	/* dc porperty */
	ftd330_dc_update_drm_properties_to_dc(dc, plane_info->id, plane_state->drm_states,
		plane->properties.num, &hw_plane->states, plane_state);
}

static void update_qos(struct ftd330_dc *dc, struct ftd330_plane *plane)
{
	struct drm_plane_state *state = plane->base.state;
	struct ftd330_plane_state *plane_state = to_ftd330_plane_state(state);
	struct drm_ftd330_watermark *data;
	struct dc_hw_qos qos;

	if (plane_state->watermark) {
		data = plane_state->watermark->data;

		if (data->qos_high) {
			if (data->qos_low > data->qos_high)
				data->qos_low = data->qos_high;

			qos.low_value = data->qos_low & 0x0F;
			qos.high_value = data->qos_high & 0x0F;
			dc_hw_update_qos(&dc->hw, &qos);
		}
	}
}

static void update_cursor_size(struct drm_plane_state *state, struct dc_hw_cursor *cursor)
{
	u8 size_type;

	switch (state->crtc_w) {
	case 32:
		size_type = CURSOR_SIZE_32X32;
		break;
	case 64:
		size_type = CURSOR_SIZE_64X64;
		break;
	case 128:
		size_type = CURSOR_SIZE_128X128;
		break;
	default:
		size_type = CURSOR_SIZE_32X32;
		break;
	}

	cursor->size = size_type;
}

static void update_cursor_plane(struct ftd330_dc *dc, struct ftd330_plane *plane)
{
	struct drm_plane_state *state = plane->base.state;
	struct dc_hw_cursor cursor;
	struct drm_crtc *crtc = state->crtc;
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct drm_framebuffer *fb = state->fb;

	cursor.address = (u32)plane->dma_addr[0];
	cursor.x = state->crtc_x;
	cursor.y = state->crtc_y;
	
	cursor.hot_x = 0;
	cursor.hot_y = 0;
		
	if (cursor.x < 0)
	{
		cursor.hot_x = cursor.hot_x - cursor.x;
		cursor.x = 0;
	}
	if (cursor.y < 0)
	{
		cursor.hot_y = cursor.hot_y - cursor.y;
		cursor.y = 0;
	}

	if (dc_fake_mode_enable &&
		((mode->crtc_hdisplay != mode->hdisplay) ||
	         (mode->crtc_vdisplay != mode->vdisplay)))	{
		
		if (cursor.x > 0) {
			cursor.x = ((((cursor.x + fb->hot_x) << 13) * mode->crtc_hdisplay / mode->hdisplay) >> 13) - fb->hot_x;
		} else {
			cursor.x = ((((fb->hot_x - cursor.hot_x) << 13) * mode->crtc_hdisplay / mode->hdisplay) >> 13) - fb->hot_x;
			if (cursor.x >= 0) {
				cursor.hot_x = 0;
			} else {
				cursor.hot_x = -cursor.x;
				cursor.x = 0;
			}
		}
	
		if (cursor.y > 0) {
			cursor.y = ((((cursor.y + fb->hot_y) << 13) * mode->crtc_vdisplay / mode->vdisplay) >> 13) - fb->hot_y;
		} else {
			cursor.y = ((((fb->hot_y - cursor.hot_y) << 13) * mode->crtc_vdisplay / mode->vdisplay) >> 13) - fb->hot_y;
			if (cursor.y >= 0) {
                                cursor.hot_y = 0;
                        } else {
                                cursor.hot_y = -cursor.y;
                                cursor.y = 0;
			}
		}

		if (cursor.x < 0)
			cursor.x = 0;

		if (cursor.y < 0)
			cursor.y = 0;

		if (cursor.hot_x < 0)
			cursor.hot_x = 0;

		if (cursor.hot_y < 0)
			cursor.hot_y = 0;
	}

	cursor.display_id = to_ftd330_display_id(dc, state->crtc);
	update_cursor_size(state, &cursor);
	cursor.enable = true;

	dc_hw_update_cursor(&dc->hw, cursor.display_id, &cursor);
}

static void ftd330_dc_update_plane(struct device *dev, struct ftd330_plane *plane)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	enum drm_plane_type type = plane->base.type;

#ifdef CONFIG_PHYTIUM_PSR
	struct drm_plane *drm_base_plane = &plane->base;
	struct drm_framebuffer *fb = NULL;
	struct phytium_dp_device *phytium_dp = NULL;
	bool psr_is_enabled = false;
	unsigned long irq_flag;
	struct drm_device *drm_dev = dc->hw.drm_dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;

	fb = drm_base_plane->state->fb;
	if (fb)
		phytium_dp = find_phytium_dp_from_fb(fb);
	if (phytium_dp) {
		mutex_lock(&phytium_dp->low_power_mutex);
		psr_is_enabled = phytium_dp->psr.enabled;
		mutex_unlock(&phytium_dp->low_power_mutex);
		if (psr_is_enabled) {
			spin_lock_irqsave(&priv->hotplug_irq_lock,irq_flag);
			phytium_dp->dplp_frame_compare_state.psr_available = false;
			phytium_dp->dplp_frame_compare_state.frame_change_in_psr = true;
			spin_unlock_irqrestore(&priv->hotplug_irq_lock,irq_flag);
			schedule_work(&phytium_dp->psr_work);
		}
	}

#endif
	switch (type) {
	case DRM_PLANE_TYPE_PRIMARY:
	case DRM_PLANE_TYPE_OVERLAY:
		update_plane(dc, plane);
		update_qos(dc, plane);
		break;
	case DRM_PLANE_TYPE_CURSOR:
		update_cursor_plane(dc, plane);
		break;
	default:
		break;
	}
}

static inline u8 find_cursor_display_id(struct ftd330_dc *dc, struct drm_plane *plane)
{
	u8 display_num = dc->hw.info->display_num;
	int i;

	for (i = 0; i < display_num; i++) {
		if (plane == dc->crtc[i]->base.cursor)
			return i;
	}

	pr_err("no matched display with this cursor\n");
	return 0;

}

static void ftd330_dc_disable_plane(struct device *dev, struct ftd330_plane *plane,
				struct drm_plane_state *old_state)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	enum drm_plane_type type = plane->base.type;
	struct dc_hw_fb fb = { 0 };
	struct dc_hw_cursor cursor = { 0 };

	switch (type) {
	case DRM_PLANE_TYPE_PRIMARY:
	case DRM_PLANE_TYPE_OVERLAY:
		fb.enable = false;
		dc_hw_update_plane(&dc->hw, plane->id, &fb, NULL, NULL, NULL);
#ifdef CONFIG_PHYTIUM_DEC
		disable_fbc(dc, plane);
#endif
		break;
	case DRM_PLANE_TYPE_CURSOR:
		cursor.enable = false;
		cursor.display_id = find_cursor_display_id(dc, &plane->base);
		dc_hw_update_cursor(&dc->hw, cursor.display_id, &cursor);
		break;
	default:
		break;
	}
}

static bool ftd330_dc_mod_supported(struct ftd330_plane_state *plane_state,
		const struct ftd330_plane_info *plane_info, u64 modifier, u32 format)
{
	const u64 *mods;
	int i, ret = false;
	u32 rot = plane_state->base.rotation;

	if (!plane_info->modifiers)
		return false;

	for (mods = plane_info->modifiers; *mods != DRM_FORMAT_MOD_INVALID; mods++) {
		if ((*mods == modifier) ||
		    ((*mods | DRM_FORMAT_MOD_FTD330_CUSTOM_FORMAT_ENABLE) == modifier)) {
			ret = true;
			break;
		}
	}

	if (plane_info->num_support_custom_formats && fourcc_mod_is_custom_format(modifier)) {
		for (i = 0; i < plane_info->num_support_custom_formats; i++) {
			if (format == plane_info->support_custom_formats[i]) {
				ret &= true;
				break;
			}
		}
		if (i == plane_info->num_support_custom_formats)
			ret = false;
	}

	if (plane_info->rot_supp_mods) {
		if ((rot & DRM_MODE_ROTATE_90) || (rot & DRM_MODE_ROTATE_270)) {
			mods = plane_info->rot_supp_mods;
			for (; *mods != DRM_FORMAT_MOD_INVALID; mods++) {
				if (*mods == modifier) {
					ret &= true;
					break;
				}
			}
			if (*mods == DRM_FORMAT_MOD_INVALID) {
				ret = false;
				pr_err("%s The modifier not support with rotation 90/270.\n",__func__);
			}
		}
	}


	return ret;
}

static int ftd330_dc_check_plane(struct device *dev, struct ftd330_plane *plane,
			     struct drm_plane_state *state)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_framebuffer *fb = state->fb;
	const struct ftd330_plane_info *plane_info;
	struct drm_crtc *crtc = state->crtc;
	struct drm_crtc_state *crtc_state;
	struct ftd330_plane_state *ftd330_plane_state = to_ftd330_plane_state(state);
	const struct drm_ftd330_color *color = NULL;

	plane_info = &dc->hw.info->planes[plane->id];
	if (!plane_info)
		return -EINVAL;

	if (fb->width < plane_info->min_width || fb->width > plane_info->max_width ||
	    fb->height < plane_info->min_height || fb->height > plane_info->max_height) {
		dev_err_once(dev, "buffer size not support on plane%d.\n", plane->id);
		return -EOPNOTSUPP;
	}

	if (plane->base.type != DRM_PLANE_TYPE_CURSOR &&
		!ftd330_dc_mod_supported(ftd330_plane_state, plane_info, fb->modifier, fb->format->format)) {
			dev_err(dev, "unsupported modifier on plane%d.\n", plane->id);
		return -EOPNOTSUPP;
	}

	if (plane_info->color_mgmt) {
		color = ftd330_dc_drm_plane_property_get(ftd330_plane_state, "CLEAR", NULL);

		if (color && fb->format->format != DRM_FORMAT_ARGB8888) {
			dev_err(dev,
				"plane%d: when Clear is enabled, input format needs to be A8R8G8B8.\n",
				plane->id);
			return -EINVAL;
		}
	}

	crtc_state = drm_atomic_get_existing_crtc_state(state->state, crtc);
	if (IS_ERR(crtc_state))
		return -EINVAL;

	if (!ftd330_dc_check_drm_property(dc, plane_info->id, ftd330_plane_state->drm_states,
				      plane->properties.num, ftd330_plane_state))
		return -EINVAL;

	return drm_atomic_helper_check_plane_state(state, crtc_state, plane_info->min_scale,
						   plane_info->max_scale, true, true);
}


static irqreturn_t dc0_isr(int irq, void *data)
{
	struct ftd330_dc *dc = data;
	const struct ftd330_dc_info *dc_info = dc->hw.info;
	struct dc_hw_interrupt_status status = {0};
	u32 i;

	dc_hw_get_interrupt(&dc->hw, &status, DC_0);

#ifdef CONFIG_PHYTIUM_WRITEBACK
	for (i = 0; i < dc_info->wb_num; i++)
		ftd330_writeback_handle_vblank(dc->writeback[i]);
#endif

	for (i = 0; i < dc_info->display_num; i++) {
		u8 display_id = dc_info->displays[i].id;
		u8 display_mask = BIT(display_id);

		if (display_mask & status.display_underflow)
			pr_warn_ratelimited("%s: display[%d] underflow\n", __func__, display_id);

		if (display_mask & status.display_frm_done) {
			ftd330_crtc_handle_vblank(&dc->crtc[i]->base);
			ftd330_crtc_handle_flip_done_while_hw_done(&dc->crtc[i]->base);
			ftd330_crtc_handle_frame_done(&dc->crtc[i]->base);
		}

	}
#ifdef CONFIG_PHYTIUM_WRITEBACK
	if (status.wb_frm_done || status.wb_datalost) {
		struct ftd330_writeback_connector *ftd330_wb_conn;
		u32 wb_mask = status.wb_frm_done | status.wb_datalost;

		for (i = 0; i < dc_info->wb_num; i++) {
			ftd330_wb_conn = dc->writeback[i];

			if (BIT(ftd330_wb_conn->id) & status.wb_datalost)
				pr_warn("%s: data lost intr for wb%d\n", __func__, ftd330_wb_conn->id);

			if (BIT(ftd330_wb_conn->id) & wb_mask)
				ftd330_writeback_handle_vblank(ftd330_wb_conn);
		}
	}

#endif

	return IRQ_HANDLED;
}


static irqreturn_t dc1_isr(int irq, void *data)
{
	struct ftd330_dc *dc = data;
	const struct ftd330_dc_info *dc_info = dc->hw.info;
	struct dc_hw_interrupt_status status = {0};
	u32 i;

	dc_hw_get_interrupt(&dc->hw, &status, DC_1);

#ifdef CONFIG_PHYTIUM_WRITEBACK
	for (i = 0; i < dc_info->wb_num; i++)
		ftd330_writeback_handle_vblank(dc->writeback[i]);
#endif

	for (i = 0; i < dc_info->display_num; i++) {
		u8 display_id = dc_info->displays[i].id;
		u8 display_mask = BIT(display_id);

		if (display_mask & status.display_underflow)
			pr_warn_ratelimited("%s: display[%d] underflow\n", __func__, display_id);
		
                if (display_mask & status.display_frm_done) {
                        ftd330_crtc_handle_vblank(&dc->crtc[i]->base);
                        ftd330_crtc_handle_flip_done_while_hw_done(&dc->crtc[i]->base);
                        ftd330_crtc_handle_frame_done(&dc->crtc[i]->base);
                }	
	}
#ifdef CONFIG_PHYTIUM_WRITEBACK
	if (status.wb_frm_done || status.wb_datalost) {
		struct ftd330_writeback_connector *ftd330_wb_conn;
		u32 wb_mask = status.wb_frm_done | status.wb_datalost;

		for (i = 0; i < dc_info->wb_num; i++) {
			ftd330_wb_conn = dc->writeback[i];

			if (BIT(ftd330_wb_conn->id) & status.wb_datalost)
				pr_warn("%s: data lost intr for wb%d\n", __func__, ftd330_wb_conn->id);

			if (BIT(ftd330_wb_conn->id) & wb_mask)
				ftd330_writeback_handle_vblank(ftd330_wb_conn);
		}
	}

#endif

	return IRQ_HANDLED;
}


static void ftd330_dc_commit(struct device *dev, struct drm_crtc *crtc)
{
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	u8 display_id = to_ftd330_display_id(dc, crtc);

#ifdef CONFIG_PHYTIUM_DEC
	if (dc->hw.info->cap_dec)
		dc_dec_commit(&dc->dec400l, &dc->hw);
#endif

	dc_hw_enable_shadow_register(&dc->hw, false, display_id);

	dc_hw_commit(&dc->hw, display_id);

	if (dc->first_frame)
		dc->first_frame = false;

	dc_hw_enable_shadow_register(&dc->hw, true, display_id);

	dc_hw_start_trigger(&dc->hw, display_id);
}


#ifdef CONFIG_PHYTIUM_WRITEBACK
static void update_wb_fb(struct ftd330_dc *dc, struct ftd330_writeback_connector *wb_connector,
					struct drm_framebuffer *fb)
{
	struct dc_hw_fb wb_fb = { 0 };

	update_format(fb->format->format, fb->modifier, &wb_fb);

	wb_fb.y_address = (u64)wb_connector->dma_addr[0];
	if (wb_fb.format == FORMAT_YV12) {
		wb_fb.u_address = (u64)wb_connector->dma_addr[2];
		wb_fb.v_address = (u64)wb_connector->dma_addr[1];
	} else {
		wb_fb.u_address = (u64)wb_connector->dma_addr[1];
		wb_fb.v_address = (u64)wb_connector->dma_addr[2];
	}

	/* stride of the destinatin buffer in bytes, no need to be aligned. */
	if (!dc->hw.info->write_back[wb_connector->id].program_point)
		wb_fb.y_stride = fb->width * fb->format->cpp[0];
	else
		wb_fb.y_stride = wb_connector->pitch[0];
	wb_fb.enable = true;

	update_swizzle(fb->format->format, &wb_fb);
	/* TBD */

	dc_hw_update_wb_fb(&dc->hw, wb_connector->id, &wb_fb);
}


static void ftd330_dc_conf_writeback(struct ftd330_writeback_connector *wb_connector,
				 struct drm_framebuffer *fb)
{
	struct device *dev = wb_connector->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_connector_state *state = wb_connector->base.base.state;
	struct ftd330_writeback_connector_state *ftd330_wb_state = to_ftd330_writeback_connector_state(state);
	struct dc_hw_wb *wb = &dc->hw.wb[wb_connector->id];

	update_wb_fb(dc, wb_connector, fb);
	ftd330_dc_update_drm_properties_to_dc(dc, wb_connector->id, ftd330_wb_state->drm_states,
					  wb_connector->properties.num, &wb->states, ftd330_wb_state);
	wb->wb_frm_done = false;
	dc_hw_setup_wb(&dc->hw, wb_connector->id);
}

static void ftd330_dc_disable_writeback(struct ftd330_writeback_connector *wb_connector,
				    struct drm_crtc *crtc)
{
	struct device *dev = wb_connector->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct dc_hw_fb wb_fb = { 0 };

	wb_fb.enable = false;
	dc_hw_update_wb_fb(&dc->hw, wb_connector->id, &wb_fb);

	dc_hw_setup_wb(&dc->hw, wb_connector->id);
}

static int ftd330_dc_check_wb_pixel_format(const struct ftd330_wb_info *wb_info, u32 format,
				       uint64_t modifier)
{
	unsigned int i;
	const u64 *mods = NULL;
	int ret = false;

	for (i = 0; i < wb_info->num_formats; i++) {
		if (format == wb_info->formats[i])
			break;
	}
	if (i == wb_info->num_formats)
		return false;

	if (!wb_info->modifiers)
		return true;

	for (mods = wb_info->modifiers; *mods != DRM_FORMAT_MOD_INVALID; mods++) {
		if ((*mods == modifier) ||
		    ((*mods | DRM_FORMAT_MOD_FTD330_CUSTOM_FORMAT_ENABLE) == modifier)) {
			ret = true;
			break;
		}
	}

	if (wb_info->num_support_custom_formats && fourcc_mod_is_custom_format(modifier)) {
		for (i = 0; i < wb_info->num_support_custom_formats; i++) {
			if (format == wb_info->support_custom_formats[i]) {
				ret &= true;
				break;
			}
		}

		if (i == wb_info->num_support_custom_formats)
			ret = false;
	}

	return ret;
}

static int ftd330_dc_check_writeback(struct ftd330_writeback_connector *wb_connector,
				 struct drm_framebuffer *fb, struct drm_display_mode *mode,
				 struct drm_connector_state *state)
{
	struct device *dev = wb_connector->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	const struct ftd330_wb_info *wb_info;
	struct ftd330_writeback_connector_state *ftd330_wb_state = to_ftd330_writeback_connector_state(state);
	int ret = 0;

	if (fb->width != mode->hdisplay || fb->height != mode->vdisplay) {
		dev_err(dev, "Invalid framebuffer size %ux%u for writeback\n", fb->width,
			fb->height);
		return -EINVAL;
	}

	wb_info = &dc->hw.info->write_back[wb_connector->id];
	ret = ftd330_dc_check_wb_pixel_format(wb_info, fb->format->format, fb->modifier);
	if (!ret) {
		dev_err(dev, "Unsupported framebuffer modifier or format for writeback\n");
		return -EOPNOTSUPP;
	}

	if (!ftd330_dc_check_drm_property(dc, wb_info->id, ftd330_wb_state->drm_states,
				      wb_connector->properties.num, ftd330_wb_state))
		return -EINVAL;

	return 0;
}

static const struct ftd330_writeback_funcs dc_writeback_funcs = {
	.config = ftd330_dc_conf_writeback,
	.disable = ftd330_dc_disable_writeback,
	.check = ftd330_dc_check_writeback,
};
#endif


static const struct ftd330_crtc_funcs dc_crtc_funcs = {
	.enable = ftd330_dc_enable,
	.disable = ftd330_dc_disable,
	.mode_fixup = ftd330_dc_mode_fixup,
	.config = ftd330_dc_conf_display,
	.enable_vblank = ftd330_dc_enable_vblank,
	.get_vblank_count = ftd330_dc_get_vblank_count,
	.commit = ftd330_dc_commit,
	.check = ftd330_dc_check_display,
};

static const struct ftd330_plane_funcs dc_plane_funcs = {
	.update = ftd330_dc_update_plane,
	.disable = ftd330_dc_disable_plane,
	.check = ftd330_dc_check_plane,
};

int ftd330_get_wb_frm_done_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_ftd330_wb_frm_done *args = data;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_dc *dc = NULL;
	int ret = -EBUSY;

	if (!priv->dc_dev)
		return -EINVAL;

	dc = dev_get_drvdata(priv->dc_dev);
	if (!dc)
		return -EINVAL;

	switch (args->wb_id) {
	case WB_0:
		args->wb_frm_done = dc->hw.wb[WB_0].wb_frm_done;
		if (args->wb_frm_done) {
			dc->hw.wb[WB_0].wb_frm_done = false;
			ret = 0;
		}
		break;
	case WB_1:
		args->wb_frm_done = dc->hw.wb[WB_1].wb_frm_done;
		if (args->wb_frm_done) {
			dc->hw.wb[WB_1].wb_frm_done = false;
			ret = 0;
		}
		break;
	default:
		return -EINVAL;
	}

	return ret;
}
int ftd330_sw_reset_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_ftd330_reset *args = data;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_dc *dc = NULL;
	int ret = 0;

	if (!priv->dc_dev)
		return -EINVAL;

	dc = dev_get_drvdata(priv->dc_dev);
	if (!dc)
		return -EINVAL;

	switch (args->mode) {
	case FTD330_RESET:
	case FTD330_FE0_RESET:
	case FTD330_FE1_RESET:
	case FTD330_BE_RESET:
		if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
			dc_hw_do_reset(&dc->hw, DC_0);
		}

		if ((priv->info.pipe_mask & BIT(DISPLAY_1)) || (priv->info.pipe_mask & BIT(DISPLAY_2))) {
			dc_hw_do_reset(&dc->hw, DC_1);
		}
		break;
	default:
		return -EINVAL;
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
		ret = dc_hw_mmu_init(&dc->hw, priv->mmu, DC_0);
	}

	if ((priv->info.pipe_mask & BIT(DISPLAY_1)) || (priv->info.pipe_mask & BIT(DISPLAY_2))) {
		ret = dc_hw_mmu_init(&dc->hw, priv->mmu, DC_1);
	}

	if (ret) {
		dev_err(dev->dev, "failed to re-init DC MMU\n");
		return ret;
	}
#endif

	/* reset the projects state. */
	drm_mode_config_reset(dev);

#ifdef CONFIG_PHYTIUM_DEBUG
	ret = ftd330_debug_reset(&priv->dc_capture_fp);
	dc->hw.dc_capture_fp = priv->dc_capture_fp;
#endif

	return ret;
}

int ftd330_get_feature_cap_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	struct drm_ftd330_query_feature_cap *args = data;
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_dc *dc = NULL;

	if (!priv->dc_dev)
		return -EINVAL;

	dc = dev_get_drvdata(priv->dc_dev);
	if (!dc)
		return -EINVAL;
	switch (args->type) {
	case FTD330_FEATURE_CAP_FBC:
		args->cap = !!(dc->hw.info->cap_dec);
		break;
	case FTD330_FEATURE_CAP_MAX_BLEND_LAYER:
		args->cap = dc->hw.info->max_blend_layer;
		break;
	case FTD330_FEATURE_CAP_CURSOR_WIDTH:
		args->cap = dev->mode_config.cursor_width;
		break;
	case FTD330_FEATURE_CAP_CURSOR_HEIGHT:
		args->cap = dev->mode_config.cursor_height;
		break;
	case FTD330_FEATURE_CAP_LINEAR_YUV_ROTATION:
		args->cap = dc->hw.info->linear_yuv_rotation;
		break;
	case FTD330_FEATURE_CAP_ANY_RESOLUTION:
		args->cap = dc->hw.info->any_resolution;
		break;
	case FTD330_FEATURE_CAP_MAX_WIDTH:
		args->cap = dc->hw.wb[0].info->max_width;
		break;
	case FTD330_FEATURE_CAP_MAX_HEIGHT:
		args->cap = dc->hw.wb[0].info->max_height;
			break;
	default:
		return -EINVAL;
	}
	return 0;
}

int ftd330_get_hist_info_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv)
{
	return 0;
}

static int dc_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *crtc;
	struct drm_crtc *drm_crtc;
	const struct ftd330_dc_info *dc_info;
	struct ftd330_plane *plane;
	struct drm_plane *drm_plane, *tmp;
	struct ftd330_display_info *display_info;
	struct ftd330_plane_info *plane_info;
	int i, ret;
	u32 crtc_mask = 0;
	u32 max_width = 0, max_height = 0;
	u32 min_width = 0xffff, min_heigth = 0xffff;
	struct dc_hw_plane *hw_plane;
	struct dc_hw_display *hw_display;

#ifdef CONFIG_PHYTIUM_WRITEBACK
	struct ftd330_writeback_connector *writeback;
	struct ftd330_wb_info *wb_info;
	struct dc_hw_wb *hw_wb;
	u32 j, valid_crtcs = 0;
#endif
#ifdef CONFIG_PHYTIUM_DEBUG
	dc->hw.dc_capture_fp = priv->dc_capture_fp;
#endif

	if (!drm_dev || !dc) {
		dev_err(dev, "devices are not created.\n");
		return -ENODEV;
	}

	ret = dc_init(dev);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize DC hardware.\n");
		return ret;
	}

#ifdef CONFIG_PHYTIUM_MMU
	dma_set_mask(priv->dma_dev, DMA_BIT_MASK(40));
#endif

	ret = ftd330_drm_iommu_attach_device(drm_dev, dev);
	if (ret < 0) {
		dev_err(dev, "Failed to attached iommu device.\n");
		goto err_clean_dc;
	}

	dc_info = dc->hw.info;

	for (i = 0; i < dc_info->display_num; i++) {
		hw_display = &dc->hw.display[i];

		crtc = ftd330_crtc_create(hw_display, drm_dev, dc_info, i);
		if (!crtc) {
			dev_err(dev, "Failed to create CRTC.\n");
			ret = -ENOMEM;
			goto err_detach_dev;
		}

		crtc->dev = dev;
		crtc->funcs = &dc_crtc_funcs;
		dc->crtc[i] = crtc;
		crtc_mask |= drm_crtc_mask(&crtc->base);

		display_info = (struct ftd330_display_info *)&dc_info->displays[i];
		max_width = (max_width < display_info->max_width) ? display_info->max_width :
								    max_width;
		max_height = (max_height < display_info->max_height) ? display_info->max_height :
								       max_height;
	}

	for (i = 0; i < dc_info->plane_num; i++) {
		plane_info = (struct ftd330_plane_info *)&dc_info->planes[i];
		hw_plane = &dc->hw.plane[i];

		if (!strcmp(plane_info->name, "Primary") ||
				!strcmp(plane_info->name, "Cursor")) {
			plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							drm_crtc_mask(&dc->crtc[0]->base),
							&dc_plane_funcs);
		} else if (!strcmp(plane_info->name, "Primary_1") ||
				!strcmp(plane_info->name, "Cursor_1")) {
			plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							drm_crtc_mask(&dc->crtc[1]->base),
							&dc_plane_funcs);
		} else if (!strcmp(plane_info->name, "Primary_2") ||
				!strcmp(plane_info->name, "Cursor_2")) {
			plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							drm_crtc_mask(&dc->crtc[2]->base),
							&dc_plane_funcs);
		} else {
			if (dc_info->display_num == 2 && (dc->hw.pipe_mask & BIT(DISPLAY_1) && dc->hw.pipe_mask & BIT(DISPLAY_2))) {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							(drm_crtc_mask(&dc->crtc[0]->base) | drm_crtc_mask(&dc->crtc[1]->base)),
							&dc_plane_funcs);
			} else if (dc_info->display_num == 3 && (dc->hw.pipe_mask & BIT(DISPLAY_1) && dc->hw.pipe_mask & BIT(DISPLAY_2))) {
				if (!strcmp(plane_info->name, "Overlay") ||
					!strcmp(plane_info->name, "Overlay_1")) {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							drm_crtc_mask(&dc->crtc[0]->base),
							&dc_plane_funcs);
				} else {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
							(drm_crtc_mask(&dc->crtc[1]->base) |
							drm_crtc_mask(&dc->crtc[2]->base)),
							&dc_plane_funcs);
				}
			} else {
				if (!strcmp(plane_info->name, "Overlay") ||
						!strcmp(plane_info->name, "Overlay_1")) {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
								drm_crtc_mask(&dc->crtc[0]->base),
								&dc_plane_funcs);
				} else if (!strcmp(plane_info->name, "Overlay_2") ||
							!strcmp(plane_info->name, "Overlay_3")) {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
								drm_crtc_mask(&dc->crtc[1]->base),
								&dc_plane_funcs);
				} else {
					plane = ftd330_plane_create(hw_plane, drm_dev, dc_info, i,
								drm_crtc_mask(&dc->crtc[2]->base),
								&dc_plane_funcs);
				}
			}
		}


		if (!plane) {
			dev_err(dev, "Failed to create plane.\n");
			ret = -ENOMEM;
			goto err_cleanup_planes;
		}

		plane->id = i;
		dc->planes[i].id = plane_info->id;

		if (plane_info->type == DRM_PLANE_TYPE_PRIMARY) {
			if (!strcmp(plane_info->name, "Primary"))
				dc->crtc[0]->base.primary = &plane->base;
			else if (!strcmp(plane_info->name, "Primary_1"))
				dc->crtc[1]->base.primary = &plane->base;
			else
				dc->crtc[2]->base.primary = &plane->base;

			min_width = (min_width > plane_info->min_width) ? plane_info->min_width :
									  min_width;
			min_heigth = (min_heigth > plane_info->min_height) ?
					     plane_info->min_height :
					     min_heigth;
			/*
			 * Note: these values are used for multiple independent things:
			 * hw display mode filtering, plane buffer sizes, writeback buffer size ...
			 * Use the combined maximum values here to cover all use cases, and do more
			 * specific checking in the respective code paths.
			 */
			max_width = (max_width < plane_info->max_width) ? plane_info->max_width :
									  max_width;
			max_height = (max_height < plane_info->max_height) ?
					     plane_info->max_height :
					     max_height;
		}

		if (plane_info->type == DRM_PLANE_TYPE_CURSOR) {
			if (!strcmp(plane_info->name, "Cursor"))
				dc->crtc[0]->base.cursor = &plane->base;
			else if (!strcmp(plane_info->name, "Cursor_1"))
				dc->crtc[1]->base.cursor = &plane->base;
			else
				dc->crtc[2]->base.cursor = &plane->base;
			drm_dev->mode_config.cursor_width = plane_info->max_width;
			drm_dev->mode_config.cursor_height = plane_info->max_height;
		}
		hw_plane->ftd330_plane = plane;
	}

#ifdef CONFIG_PHYTIUM_WRITEBACK
	for (i = 0; i < dc_info->wb_num; i++) {
		wb_info = (struct ftd330_wb_info *)&dc_info->write_back[i];
		hw_wb = &dc->hw.wb[i];
		valid_crtcs = 0;
		for (j = 0; j < dc_info->display_num; j++) {
			display_info = (struct ftd330_display_info *)&dc_info->displays[j];
			if (wb_info->src_mask & BIT(display_info->id))
				valid_crtcs |= drm_crtc_mask(&dc->crtc[j]->base);
		}

		writeback = ftd330_writeback_create(hw_wb, drm_dev, wb_info, valid_crtcs);

		if (!writeback) {
			dev_err(dev, "Failed to create writeback connector.\n");
			ret = -ENOMEM;
			goto err_cleanup_planes;
		}

		writeback->id = i;
		writeback->dev = dev;
		writeback->funcs = &dc_writeback_funcs;
		dc->writeback[i] = writeback;

		/*
		 * Note: these values are used for multiple independent things:
		 * hw display mode filtering, plane buffer sizes, writeback buffer size ...
		 * Use the combined maximum values here to cover all use cases, and do more
		 * specific checking in the respective code paths.
		 */
		max_width = (max_width < wb_info->max_width) ? wb_info->max_width : max_width;
		max_height = (max_height < wb_info->max_height) ? wb_info->max_height : max_height;
	}
#endif

	drm_dev->mode_config.min_width = min_width;
	drm_dev->mode_config.min_height = min_heigth;
	drm_dev->mode_config.max_width = max_width;
	drm_dev->mode_config.max_height = max_height;

	priv->dc_dev = dev;

	ftd330_drm_update_alignment(drm_dev, dc_info->pitch_alignment, dc_info->addr_alignment);



	return 0;

err_cleanup_planes:
	list_for_each_entry_safe(drm_plane, tmp, &drm_dev->mode_config.plane_list, head)
		if (drm_plane->possible_crtcs & crtc_mask)
			ftd330_plane_destory(drm_plane);

	drm_for_each_crtc(drm_crtc, drm_dev)
		ftd330_crtc_destroy(drm_crtc);
err_detach_dev:
	ftd330_drm_iommu_detach_device(drm_dev, dev);
err_clean_dc:
	dc_deinit(dev);
	return ret;
}

static void dc_unbind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
#ifdef CONFIG_PHYTIUM_MMU
	struct ftd330_drm_private *priv = drm_dev->dev_private;
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
	struct ftd330_dc *dc = dev_get_drvdata(dev);

	dc->hw.dc_capture_fp = NULL;
#endif
	dc_deinit(dev);
#ifdef CONFIG_PHYTIUM_MMU
	dc_mmu_deconstruct(priv->dma_dev, priv->mmu);
#endif

	ftd330_drm_iommu_detach_device(drm_dev, dev);
}

const struct component_ops dc_component_ops = {
	.bind = dc_bind,
	.unbind = dc_unbind,
};

static const struct of_device_id dc_driver_dt_match[] = {
	{
		.compatible = "phytium,dcFTD330",
	},
	{},
};
MODULE_DEVICE_TABLE(of, dc_driver_dt_match);

static int dc_construct(struct device *dev, struct ftd330_dc **ftd330_dc)
{
	struct ftd330_dc *dc;
	int ret = 0;

	dc = devm_kzalloc(dev, sizeof(*dc), GFP_KERNEL);
	if (!dc)
		return -ENOMEM;

	*ftd330_dc = dc;

	return ret;
}

#ifdef CONFIG_PHYTIUM_PCIE
static irqreturn_t pci_drm_isr(int irq, void *data)
{
	struct ftd330_dc *dc = data;
	struct drm_device *drm_dev = dc->hw.drm_dev;
	struct phytium_dp_device *phytium_dp;
	struct drm_encoder *encoder;

	dc0_isr(irq, data);
	dc1_isr(irq, data);

	drm_for_each_encoder(encoder, drm_dev) {
		phytium_dp = encoder_to_dp_device(encoder);
		phytium_dp_hpd_irq_handler(irq, (void *)phytium_dp);
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
		phytium_dp_power_on_irq_handler(irq, (void *)phytium_dp);
#endif
	}

	return IRQ_HANDLED;
}


static int ftd330_dc_pci_init(struct drm_device *drm_dev, struct ftd330_dc *dc)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct pci_dev *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	resource_size_t addr;
	resource_size_t len;
	int ret;

#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	drm_dev->irq = drm_dev->pdev->irq;
	pr_info("drm_dev->pdev->irq = %d\n", drm_dev->irq);
#else
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	drm_dev->irq = pdev->irq;
#endif
	pr_info("pdev->irq = %d\n", pdev->irq);
#endif

	dc->hw.total_pipes = priv->info.total_pipes;

	addr = pci_resource_start(pdev, 0);
	len = pci_resource_len(pdev, 0);
	dc->hw.hi_base = devm_ioremap(dev, addr, len);

	if (IS_ERR(dc->hw.hi_base))
		return PTR_ERR(dc->hw.hi_base);

	if (pdev->dev.of_node) {
		addr = pci_resource_start(pdev, 2);
		len = pci_resource_len(pdev, 2);
		priv->se_regs = devm_ioremap(dev, addr, len);
	} else {
		pr_info("ACPI case.no se_regs set\n");
	}


#ifdef CONFIG_PHYTIUM_NCC
	priv->mem_pool_start_address_phy = pci_resource_start(pdev, 1);
	priv->mem_pool_size = pci_resource_len(pdev, 1);
#endif

	ret = request_irq(pdev->irq, pci_drm_isr, IRQF_SHARED, dev_name(dev), drm_dev);
	if (ret) {
		DRM_ERROR("install irq failed\n");
		return -1;
	}

	return ret;
}
#else

/* platform driver */
static int ftd330_dc_platform_init(struct drm_device *drm_dev, struct ftd330_dc *dc)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct platform_device *pdev = priv->pdev;
	struct device *dev = &pdev->dev;
	int ret = 0;
	struct device_node *node;
	struct fwnode_handle *np;
	int i = 0;
	u8 edp_need_poweron_state = 0;
#ifdef CONFIG_PHYTIUM_NCC
	struct resource *res;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
	struct resource *resour;
#endif

	if (pdev->dev.of_node) {
		node = pdev->dev.of_node;
		ret = of_property_read_u8(node, "pipe_mask", &priv->info.pipe_mask);

		pr_info("FTD330 pipe_mask: %d\n", priv->info.pipe_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing pipe_mask property from dts\n");
			return ret;
		}

		ret = of_property_read_u8(node, "edp_mask", &priv->info.edp_mask);
		pr_info("FTD330 edp_mask: is %d\n", priv->info.edp_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing edp_mask property from dts\n");
			return ret;
		}
		ret = of_property_read_u32_array(node, "water_mark", priv->info.water_mark, 3);
		pr_info("FTD330 water_mark: is %d %d %d\n",
				priv->info.water_mark[0], priv->info.water_mark[1],
				priv->info.water_mark[2]);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing water_mark property from dts\n");
			return ret;
		}
		ret = of_property_read_u32_array(node, "qos", priv->info.qos, 2);
		pr_info("FTD330 qos: is %d %d %d\n", priv->info.qos[0],
				priv->info.qos[1], priv->info.qos[2]);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing qos property from dts\n");
			return ret;
		}

		ret = of_property_read_u8(node, "overlay_enable", &priv->info.overlay_enable);
		pr_info("FTD330 overlay_enable: %d\n", priv->info.overlay_enable);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing overlay_enable property from dts\n");
			return ret;
		}
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
		ret = of_property_read_u32_array(node, "phy_mode", priv->info.phy_mode, 3);
		pr_info("FTD330 phy_mode: is %d %d %d\n", priv->info.phy_mode[0],
				priv->info.phy_mode[1], priv->info.phy_mode[2]);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing phy_mode property from dts\n");
			return ret;
		}
#endif
		ret = of_property_read_u32_array(node, "parameter_table_address",
													priv->info.para_table_addr, 2);
		pr_info("FTD330 parameter_table_address: is 0x%x, length is %d\n",
					priv->info.para_table_addr[0], priv->info.para_table_addr[1]);
		if (ret < 0) {
			pr_info("missing parameter_table_address property from dts\n");
			priv->info.para_table_valid = false;
		} else {
			priv->info.para_table_valid = true;
		}
#ifdef CONFIG_PHYTIUM_EDP_BL
		if (priv->info.edp_mask) {
			priv->info.pwm_clk_rate = 100000000;
			ret =
			of_property_read_u32(node, "pwm_output_freq",
									&priv->info.pwm_freq);
			if (ret < 0) {
				dev_info(&pdev->dev,
				"missing pwm freq property from acpi, using default setting\n");
				priv->info.pwm_freq = 1000;
			}
			pr_info("FTD330 pwm_output_freq: %dhz\n", priv->info.pwm_freq);

			priv->info.pwm_periodns = 1000000;
			priv->info.pwm_div = priv->info.pwm_clk_rate / priv->info.pwm_freq / 100 - 1;
			if (priv->info.pwm_div < 1 || priv->info.pwm_div > 4096) {
				dev_err(&pdev->dev,
						"pwm_freq %d exceeds hardware support range 244hz~500khz\n",
						priv->info.pwm_freq);
				return -EINVAL;
			}
			ret =
			of_property_read_u8(node, "edp_need_poweron",
									&edp_need_poweron_state);
			if (ret < 0) {
				dev_info(&pdev->dev,
					"missing edp_need_poweron from acpi, poweron all cases\n");
				priv->info.edp_boot_need_poweron = true;
				priv->info.edp_s3_need_poweron = true;
				priv->info.edp_s4_need_poweron = true;
			}
			if (edp_need_poweron_state & BIT(0))
				priv->info.edp_boot_need_poweron = true;
			if (edp_need_poweron_state & BIT(1))
				priv->info.edp_s3_need_poweron = true;
			if (edp_need_poweron_state & BIT(2))
				priv->info.edp_s4_need_poweron = true;
		}
#endif
	} else if (has_acpi_companion(&pdev->dev)) {
		np = dev_fwnode(&(pdev->dev));
		ret = fwnode_property_read_u8(np, "pipe_mask", &priv->info.pipe_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing pipe_mask property from acpi\n");
			return ret;
		}
		ret = fwnode_property_read_u8(np, "edp_mask", &priv->info.edp_mask);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing edp_mask property from acpi\n");
			return ret;
		}
		pr_info("FTD330 edp_mask: is %d\n", priv->info.edp_mask);
		ret = fwnode_property_read_u32_array(np, "water_mark", priv->info.water_mark, 3);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing water_mark property from acpi\n");
			return ret;
		}
		ret = fwnode_property_read_u32_array(np, "qos", priv->info.qos, 2);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing qos property from acpi\n");
			return ret;
		}

		ret = fwnode_property_read_u8(np, "overlay_enable", &priv->info.overlay_enable);
		pr_info("FTD330 overlay_enable: %d\n", priv->info.overlay_enable);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing overlay_enable property from dts\n");
			return ret;
		}
#ifdef CONFIG_PHYTIUM_LANE_TRAIN
		ret = fwnode_property_read_u32_array(np, "phy_mode", priv->info.phy_mode, 3);
		pr_info("FTD330 phy_mode: is %d %d %d\n", priv->info.phy_mode[0],
				priv->info.phy_mode[1], priv->info.phy_mode[2]);
		if (ret < 0) {
			dev_err(&pdev->dev, "missing phy_mode property from dts\n");
			return ret;
		}
#endif
		ret = fwnode_property_read_u32_array(np, "parameter_table_address",
														priv->info.para_table_addr, 2);
		pr_info("FTD330 parameter_table_address: is 0x%x, length is %d\n",
					priv->info.para_table_addr[0], priv->info.para_table_addr[1]);
		if (ret < 0) {
			pr_info("missing parameter_table_address property from acpi _DSD\n");
			priv->info.para_table_valid = false;
		} else {
			priv->info.para_table_valid = true;
		}
	
#ifdef CONFIG_PHYTIUM_EDP_BL
		if (priv->info.edp_mask) {
			priv->info.pwm_clk_rate = 100000000;
			ret =
			fwnode_property_read_u32(np, "pwm_output_freq",
									&priv->info.pwm_freq);
			if (ret < 0) {
				dev_info(&pdev->dev,
				"missing pwm freq property from acpi, using default setting\n");
				priv->info.pwm_freq = 1000;
			}
			pr_info("FTD330 pwm_output_freq: %dhz\n", priv->info.pwm_freq);

			priv->info.pwm_periodns = 1000000;
			priv->info.pwm_div = priv->info.pwm_clk_rate / priv->info.pwm_freq / 100 - 1;
			if (priv->info.pwm_div < 1 || priv->info.pwm_div > 4096) {
				dev_err(&pdev->dev,
						"pwm_freq %d exceeds hardware support range 244hz~500khz\n",
						priv->info.pwm_freq);
				return -EINVAL;
			}

			ret =
			fwnode_property_read_u8(np, "edp_need_poweron",
									&edp_need_poweron_state);
			if (ret < 0) {
				dev_info(&pdev->dev,
					"missing edp_need_poweron from acpi, poweron all cases\n");
				priv->info.edp_boot_need_poweron = true;
				priv->info.edp_s3_need_poweron = true;
				priv->info.edp_s4_need_poweron = true;
			}
			if (edp_need_poweron_state & BIT(0))
				priv->info.edp_boot_need_poweron = true;
			if (edp_need_poweron_state & BIT(1))
				priv->info.edp_s3_need_poweron = true;
			if (edp_need_poweron_state & BIT(2))
				priv->info.edp_s4_need_poweron = true;
		}
#endif
	}
	/* Parse the bios parameters */
	if (priv->info.para_table_valid) {
		priv->info.bios_info.para_table_base =
			memremap(priv->info.para_table_addr[0],
						priv->info.para_table_addr[1], MEMREMAP_WB);
		if (!priv->info.bios_info.para_table_base) {
			pr_err("memremap failed for para_table_base\n");
		} else {
			parse_bios_para_table(&priv->info.bios_info);
			memunmap(priv->info.bios_info.para_table_base);
		}
	}

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


	pr_info("FTD330 total_pipes = %d,pipe_mask = %d\n",
			priv->info.total_pipes, priv->info.pipe_mask);

	if (priv->info.pipe_mask & BIT(DISPLAY_0)) {
		ret = request_irq(platform_get_irq(pdev, 0),
						  dc0_isr, IRQF_SHARED, dev_name(dev), dc);
		if (ret < 0) {
			dev_err(dev, "Failed to install irq\n");
			return ret;
		}
	} 

	if (priv->info.pipe_mask & BIT(DISPLAY_1) || priv->info.pipe_mask & BIT(DISPLAY_2)) {
		ret = request_irq(platform_get_irq(pdev, 1),
						  dc1_isr, IRQF_SHARED, dev_name(dev), dc);
		if (ret < 0) {
			dev_err(dev, "Failed to install irq\n");
			return ret;
		}
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	dc->hw.hi_base = devm_platform_ioremap_resource(pdev, 0);
#else
	resour = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dc->hw.hi_base = devm_ioremap_resource(&pdev->dev, resour);
#endif
	if (IS_ERR(dc->hw.hi_base)) {
		pr_info("fatal error,dc->hw.hi_base[DC_0] is wrong,init stopped\n");
		return PTR_ERR(dc->hw.hi_base);
	}

#ifdef CONFIG_PHYTIUM_NCC
	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res) {
		priv->mem_pool_size = resource_size(res);
		priv->mem_pool_start_address_phy = res->start;
	} else {
		pr_info("platform_get_resource fail .pls check\n");
	}
#endif

	if (pdev->dev.of_node) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
		priv->se_regs = devm_platform_ioremap_resource(pdev, 2);
#else
	resour = platform_get_resource(pdev, IORESOURCE_MEM, 2);
	priv->se_regs = devm_ioremap_resource(&pdev->dev, resour);
#endif
	} else
		pr_info("ACPI case.no se_regs set\n");

	return ret;
}
#endif


void phytium_drm_device_deinit(struct drm_device *drm_dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
#ifdef CONFIG_PHYTIUM_PCIE
	struct pci_dev *pdev = priv->pdev;
#else
	struct platform_device *pdev = priv->pdev;
#endif
	struct device *dev = &pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct bios_table_info *bios_info = &priv->info.bios_info;

	dc_unbind(dev, NULL, drm_dev);
	devm_iounmap(dev, dc->hw.hi_base);

	if (bios_info->panels) {
		kfree(bios_info->panels);
		bios_info->panels = NULL;
	}
}


/* platform driver */
int phytium_drm_device_init(struct drm_device *drm_dev)
{
	struct ftd330_drm_private *priv = drm_dev->dev_private;
#ifdef CONFIG_PHYTIUM_PCIE
	struct pci_dev *pdev = priv->pdev;
#else
	struct platform_device *pdev = priv->pdev;
#endif
	struct device *dev = &pdev->dev;
	struct ftd330_dc *dc;
	int ret;
	int i;

	ret = dc_construct(dev, &dc);
	if (ret)
		return ret;

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 1, 0)
#if IS_ENABLED(CONFIG_DRM_LEGACY)
	drm_dev->irq_enabled = true;
#endif
#else
	drm_dev->irq_enabled = true;
#endif

#ifdef CONFIG_PHYTIUM_PCIE
		ftd330_dc_pci_init(drm_dev, dc);
#else
		ftd330_dc_platform_init(drm_dev, dc);
#endif

	dc->hw.reg_base = dc->hw.hi_base;
#if defined(CONFIG_PHYTIUM_MMU) || defined(CONFIG_PHYTIUM_WRITEBACK)
	dc->hw.sec_base = dc->hw.hi_base + 0x4000;
#endif
	priv->regs = dc->hw.hi_base;

#ifdef CONFIG_PHYTIUM_NCC
	if (start_address) {
		priv->mem_pool_size = 1 << 27;
		priv->mem_pool_start_address_phy = start_address;
	}
	priv->mem_pool_start_address_virt = (void *)ioremap_wc(priv->mem_pool_start_address_phy,
									priv->mem_pool_size);
#endif

	for (i = 0; i < priv->info.total_pipes; i++) {
		priv->low_power_enable[i] = true;
	}



#ifdef CONFIG_PHYTIUM_MMU
	dma_set_mask(dev, DMA_BIT_MASK(40));
#endif

	dc->hw.phytium_log_enable = false;
	dc->hw.drm_dev = drm_dev;
	dev_set_drvdata(dev, dc);

	return dc_bind(dev, NULL, drm_dev);
}


static int dc_be_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_be_remove(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_fe0_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_fe0_remove(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_fe1_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_fe1_remove(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_wb_probe(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

static int dc_wb_remove(struct platform_device *pdev)
{
	int ret = 0;
	/* TODO */
	return ret;
}

//struct platform_driver dc_platform_driver = {
//	.probe = dc_probe,
//	.remove = dc_remove,
//	.driver = {
//		.name = "phy-dc",
//		.of_match_table = of_match_ptr(dc_driver_dt_match),
//	},
//};

struct platform_driver dc_be_platform_driver = {
	.probe = dc_be_probe,
	.remove = dc_be_remove,

	.driver = {
		.name = "phy-dc-be",
		.of_match_table = NULL,
	},
};

struct platform_driver dc_fe0_platform_driver = {
	.probe = dc_fe0_probe,
	.remove = dc_fe0_remove,
	.driver = {
		.name = "phy-dc-fe0",
		.of_match_table = NULL,
	},
};

struct platform_driver dc_fe1_platform_driver = {
	.probe = dc_fe1_probe,
	.remove = dc_fe1_remove,

	.driver = {
		.name = "phy-dc-fe1",
		.of_match_table = NULL,
	},
};

struct platform_driver dc_wb_platform_driver = {
	.probe = dc_wb_probe,
	.remove = dc_wb_remove,

	.driver = {
		.name = "phy-dc-wb",
		.of_match_table = NULL,
	},
};
MODULE_DESCRIPTION("Phytium DC Driver");
MODULE_LICENSE("GPL");
