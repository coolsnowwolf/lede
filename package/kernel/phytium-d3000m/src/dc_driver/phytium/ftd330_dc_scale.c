// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_scale.h"

static uint32_t dc_scaling_get_factor(uint32_t src_size, uint32_t dst_size)
{
	uint32_t factor = 0;

	factor = ((src_size - 1) << SCALE_FACTOR_SRC_OFFSET) / (dst_size - 1);

	return factor;
}

static float dc_sint(float x)
{
	const float B = 1.2732395477;
	const float C = -0.4052847346;
	const float P = 0.2310792853;
	float y;

	if (x < 0)
		y = B*x - C*x*x;
	else
		y = B*x + C*x*x;
	if (y < 0)
		y = P * (y * (0 - y) - y) + y;
	else
		y = P * (y * y - y) + y;
	return y;
}

static float dc_sinc_filter(float x, int radius)
{
	float pit, pitd, f1, f2, result;
	float f_radius = MATH_I2Float(radius);

	if (x == 0.0f) {
		result = 1.0f;
	} else if ((x < -f_radius) || (x > f_radius)) {
		result = 0.0f;
	} else {
		pit  = MATH_Multiply(PHYPI, x);
		pitd = MATH_Divide(pit, f_radius);
		f1 = MATH_Divide(dc_sint(pit), pit);
		f2 = MATH_Divide(dc_sint(pitd), pitd);
		result = MATH_Multiply(f1, f2);
	}

	return result;
}

static int dc_calculate_sync_table(
	uint8_t kernel_size,
	uint32_t src_size,
	uint32_t dst_size,
	struct filter_blit_array *kernel_info)
{
	uint32_t scale_factor;
	float f_scale;
	int kernel_half;
	float f_subpixel_step;
	float f_subpixel_offset;
	uint32_t subpixel_pos;
	int kernel_pos;
	int padding;
	uint16_t *kernel_array;
	int range = 0;

	do {
		/* Compute the scale factor. */
		scale_factor = dc_scaling_get_factor(src_size, dst_size);

		/* Same kernel size and ratio as before? */
		if ((kernel_info->kernelSize  == kernel_size) &&
		(kernel_info->scaleFactor == kernel_size)) {
			break;
		}

		/* check the array */
		if (kernel_info->kernelStates == NULL)
			break;

		/* Store new parameters. */
		kernel_info->kernelSize  = kernel_size;
		kernel_info->scaleFactor = scale_factor;

		/* Compute the scale factor. */
		f_scale = MATH_DivideFromUInteger(dst_size, src_size);

		/* Adjust the factor for magnification. */
		if (f_scale > 1.0f)
			f_scale = 1.0f;

		/* Calculate the kernel half. */
		kernel_half = (int) (kernel_info->kernelSize >> 1);

		/* Calculate the subpixel step. */
		f_subpixel_step = MATH_Divide(1.0f, MATH_I2Float(SUBPIXELCOUNT));

		/* Init the subpixel offset. */
		f_subpixel_offset = 0.5f;

		/* Determine kernel padding size. */
		padding = (MAXKERNELSIZE - kernel_info->kernelSize) / 2;

		/* Set initial kernel array pointer. */
		kernel_array = (uint16_t *) (kernel_info->kernelStates + 1);

		/* Loop through each subpixel. */
		for (subpixel_pos = 0; subpixel_pos < SUBPIXELLOADCOUNT; subpixel_pos++) {
			/* Define a temporary set of weights. */
			float fSubpixelSet[MAXKERNELSIZE];

			/* Init the sum of all weights for the current subpixel. */
			float fWeightSum = 0.0f;
			uint16_t weightSum = 0;
			short int adjustCount, adjustFrom;
			short int adjustment;

			/* Compute weights. */
			for (kernel_pos = 0; kernel_pos < MAXKERNELSIZE; kernel_pos++) {
				/* Determine the current index. */
				int index = kernel_pos - padding;

				/* Pad with zeros. */
				if ((index < 0) || (index >= kernel_info->kernelSize)) {
					fSubpixelSet[kernel_pos] = 0.0f;
				} else {
					if (kernel_info->kernelSize == 1) {
						fSubpixelSet[kernel_pos] = 1.0f;
					} else {
						/* Compute the x position for filter function. */
						float fX = MATH_Add(
							MATH_I2Float(index - kernel_half),
							f_subpixel_offset);
						fX = MATH_Multiply(fX, f_scale);

						/* Compute the weight. */
						fSubpixelSet[kernel_pos] = dc_sinc_filter(fX,
									   kernel_half);
					}

					/* Update the sum of weights. */
					fWeightSum = MATH_Add(fWeightSum,
								      fSubpixelSet[kernel_pos]);
				}
			}

			/* Adjust weights so that the sum will be 1.0. */
			for (kernel_pos = 0; kernel_pos < MAXKERNELSIZE; kernel_pos++) {
				/* Normalize the current weight. */
				float fWeight = MATH_Divide(fSubpixelSet[kernel_pos],
								    fWeightSum);

				/* Convert the weight to fixed point and store in the table. */
				if (fWeight == 0.0f)
					kernel_array[kernel_pos] = 0x0000;
				else if (fWeight >= 1.0f)
					kernel_array[kernel_pos] = 0x4000;
				else if (fWeight <= -1.0f)
					kernel_array[kernel_pos] = 0xC000;
				else
					kernel_array[kernel_pos] =
						(int16_t) MATH_Multiply(fWeight, 16384.0f);
				weightSum += kernel_array[kernel_pos];
			}

			/* Adjust the fixed point coefficients. */
			adjustCount = 0x4000 - weightSum;
			if (adjustCount < 0) {
				adjustCount = -adjustCount;
				adjustment = -1;
			} else {
				adjustment = 1;
			}

			adjustFrom = (MAXKERNELSIZE - adjustCount) / 2;
			for (kernel_pos = 0; kernel_pos < adjustCount; kernel_pos++) {
				range = (MAXKERNELSIZE*subpixel_pos + adjustFrom + kernel_pos) *
					sizeof(uint16_t);
				if ((range >= 0) && (range < KERNELTABLESIZE))
					kernel_array[adjustFrom + kernel_pos] += adjustment;
				else
					DRM_ERROR("%s failed\n", __func__);
			}

			kernel_array += MAXKERNELSIZE;

			/* Advance to the next subpixel. */
			f_subpixel_offset = MATH_Add(f_subpixel_offset, -f_subpixel_step);
		}
	} while (0);

	return 0;
}

void phytium_dc_scaling_config(struct dc_hw *hw, u8 plane_id, struct dc_hw_position *pos)
{

	// struct drm_plane drm_plane = plane->base;
	// struct drm_crtc *crtc = drm_plane.crtc;
	// struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	// struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	// struct drm_ftd330_scale_config *scale = &ftd330_crtc->scale;

	struct filter_blit_array kernel_info_width;
	void *tmp =  NULL;
	struct dc_hw_plane *hw_plane = &hw->plane[plane_id];
	struct ftd330_plane *plane = hw_plane->ftd330_plane;
	struct drm_plane *drm_plane = &plane->base;
	struct drm_plane_state *state = drm_plane->state;
	struct drm_crtc *crtc = state->crtc;
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct ftd330_crtc *ftd330_crtc = to_ftd330_crtc(crtc);
	struct drm_ftd330_scale_config *scale = &ftd330_crtc->scale;

	if (mode->hdisplay != mode->crtc_hdisplay || mode->vdisplay != mode->crtc_vdisplay) {
		scale->src_w = mode->hdisplay;
		scale->src_h = mode->vdisplay;
		scale->dst_w = mode->crtc_hdisplay;
		scale->dst_h = mode->crtc_vdisplay;

		pos->end_x = pos->end_x + (scale->dst_w - scale->src_w);
		pos->end_y = pos->end_y + (scale->dst_h - scale->src_h);

		scale->factor_x = dc_scaling_get_factor(scale->src_w, scale->dst_w);
		scale->factor_y = dc_scaling_get_factor(scale->src_h, scale->dst_h);

		if (scale->factor_y > (SCALE_FACTOR_Y_MAX << SCALE_FACTOR_SRC_OFFSET))
			scale->factor_y = (SCALE_FACTOR_Y_MAX << SCALE_FACTOR_SRC_OFFSET);

		tmp = kmalloc(KERNELSTATES, GFP_KERNEL);
		if (!tmp) {
			DRM_ERROR("malloc %ld failed\n", KERNELSTATES);
			return;
		}

		memset(&kernel_info_width, 0, sizeof(struct filter_blit_array));
		kernel_info_width.kernelStates = tmp;
		memset(kernel_info_width.kernelStates, 0, KERNELSTATES);
		kernel_neon_begin();
		dc_calculate_sync_table(FRAMEBUFFER_HORIZONTAL_FILTER_TAP,
					scale->src_w,
					scale->dst_w,
					&kernel_info_width);
		memset(scale->coef_h, 0, sizeof(scale->coef_h));
		memcpy(scale->coef_h, kernel_info_width.kernelStates + 1, KERNELSTATES - 4);
		kernel_neon_end();

		memset(&kernel_info_width, 0, sizeof(struct filter_blit_array));
		kernel_info_width.kernelStates = tmp;
		memset(kernel_info_width.kernelStates, 0, KERNELSTATES);
		kernel_neon_begin();
		dc_calculate_sync_table(FRAMEBUFFER_FILTER_TAP, scale->src_h,
				     scale->dst_h, &kernel_info_width);

		memset(scale->coef_v, 0, sizeof(scale->coef_v));
		memcpy(scale->coef_v, kernel_info_width.kernelStates + 1, KERNELSTATES - 4);
		kernel_neon_end();

		kfree(tmp);
	}

	/*pr_info("in %s mode(%dx%d), crtc_mode(%dx%d)\n", __func__,
			mode->hdisplay, mode->vdisplay, mode->crtc_hdisplay, mode->crtc_vdisplay);*/

}

void phytium_dc_scale_register_config(struct dc_hw *hw, uint32_t display_id, uint32_t hw_id)
{
	uint32_t offset = 0, i = 0;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	struct drm_device *drm_dev = hw->drm_dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct device *dev = priv->dc_dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct ftd330_crtc *ftd330_crtc = dc->crtc[display_id];
	struct drm_crtc *crtc = &ftd330_crtc->base;
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct drm_ftd330_scale_config *scale = &ftd330_crtc->scale;
	bool enable = false;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (mode->hdisplay != mode->crtc_hdisplay || mode->vdisplay != mode->crtc_vdisplay)
		enable = true;
	else
		enable = false;

	if (primary)
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG + offset, enable << 22, BIT(22));
	else
		dc_set_clear(hw, DC_OVERLAY_SCALE_CONFIG + offset, enable << 8, BIT(8));

	if (enable) {
		dc_write(hw, reg->scale_factor_x + offset, scale->factor_x);
		dc_write(hw, reg->scale_factor_y + offset, scale->factor_y);
		dc_write(hw, reg->scale_config + offset, 0x33);
		dc_write(hw, reg->init_offset + offset, 0x80008000);
		dc_write(hw, reg->h_filter_coef_index + offset, 0x00);
		for (i = 0; i < 128; i++)
			dc_write(hw, reg->h_filter_coef_data + offset, scale->coef_h[i]);

		dc_write(hw, reg->v_filter_coef_index + offset, 0x00);

		for (i = 0; i < 128; i++)
			dc_write(hw, reg->v_filter_coef_data + offset, scale->coef_v[i]);
	}
}
