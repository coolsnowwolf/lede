// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <drm/drm_atomic_helper.h>
#include <drm/drm_atomic.h>
#include <asm/neon.h>
#include <drm/drm_vblank.h>
#include "phytium_display_drv.h"
#include "phytium_crtc.h"
#include "phytium_plane.h"
#include "phytium_dp.h"
#include "px210_dc.h"
#include "pe220x_dc.h"
#include "phytium_reg.h"

#define MAXKERNELSIZE		9
#define SUBPIXELINDEXBITS	5
#define SUBPIXELCOUNT		(1 << SUBPIXELINDEXBITS)
#define SUBPIXELLOADCOUNT	(SUBPIXELCOUNT / 2 + 1)
#define WEIGHTSTATECOUNT	(((SUBPIXELLOADCOUNT * MAXKERNELSIZE + 1) & ~1) / 2)
#define KERNELTABLESIZE		(SUBPIXELLOADCOUNT * MAXKERNELSIZE * sizeof(uint16_t))
#define PHYALIGN(n, align)	(((n) + ((align) - 1)) & ~((align) - 1))
#define KERNELSTATES		(PHYALIGN(KERNELTABLESIZE + 4, 8))
#define PHYPI			3.14159265358979323846f

#define MATH_Add(X, Y)			(float)((X) + (Y))
#define MATH_Multiply(X, Y)		(float)((X) * (Y))
#define MATH_Divide(X, Y)		(float)((X) / (Y))
#define MATH_DivideFromUInteger(X, Y)	((float)(X) / (float)(Y))
#define MATH_I2Float(X)		(float)(X)

struct filter_blit_array {
	uint8_t kernelSize;
	uint32_t scaleFactor;
	uint32_t *kernelStates;
};

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

static void phytium_dc_scaling_config(struct drm_crtc *crtc,
				     struct drm_crtc_state *old_state)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];
	uint32_t scale_factor_x, scale_factor_y, i;
	uint32_t kernelStates[128];
	struct filter_blit_array kernel_info_width;
	void *tmp =  NULL;

	if (mode->hdisplay != mode->crtc_hdisplay || mode->vdisplay != mode->crtc_vdisplay) {
		phytium_crtc->src_width = mode->hdisplay;
		phytium_crtc->src_height = mode->vdisplay;
		phytium_crtc->dst_width = mode->crtc_hdisplay;
		phytium_crtc->dst_height = mode->crtc_vdisplay;

		phytium_crtc->dst_x = (mode->crtc_hdisplay - phytium_crtc->dst_width) / 2;
		phytium_crtc->dst_y = (mode->crtc_vdisplay - phytium_crtc->dst_height) / 2;

		scale_factor_x = dc_scaling_get_factor(phytium_crtc->src_width,
								 phytium_crtc->dst_width);
		scale_factor_y = dc_scaling_get_factor(phytium_crtc->src_height,
								 phytium_crtc->dst_height);
		if (scale_factor_y > (SCALE_FACTOR_Y_MAX << SCALE_FACTOR_SRC_OFFSET))
			scale_factor_y = (SCALE_FACTOR_Y_MAX << SCALE_FACTOR_SRC_OFFSET);

		phytium_writel_reg(priv, scale_factor_x & SCALE_FACTOR_X_MASK,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_SCALE_FACTOR_X);
		phytium_writel_reg(priv, scale_factor_y & SCALE_FACTOR_Y_MASK,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_SCALE_FACTOR_Y);
		phytium_writel_reg(priv, FRAMEBUFFER_TAP,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_SCALECONFIG);

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
					phytium_crtc->src_width,
					phytium_crtc->dst_width,
					&kernel_info_width);
		memset(kernelStates, 0, sizeof(kernelStates));
		memcpy(kernelStates, kernel_info_width.kernelStates + 1, KERNELSTATES - 4);
		kernel_neon_end();
		phytium_writel_reg(priv, HORI_FILTER_INDEX,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_HORI_FILTER_INDEX);
		for (i = 0; i < 128; i++) {
			phytium_writel_reg(priv, kernelStates[i],
					   group_offset, PHYTIUM_DC_FRAMEBUFFER_HORI_FILTER);
		}

		memset(&kernel_info_width, 0, sizeof(struct filter_blit_array));
		kernel_info_width.kernelStates = tmp;
		memset(kernel_info_width.kernelStates, 0, KERNELSTATES);
		kernel_neon_begin();
		dc_calculate_sync_table(FRAMEBUFFER_FILTER_TAP, phytium_crtc->src_height,
				     phytium_crtc->dst_height, &kernel_info_width);
		memset(kernelStates, 0, sizeof(kernelStates));
		memcpy(kernelStates, kernel_info_width.kernelStates + 1, KERNELSTATES - 4);
		kernel_neon_end();
		phytium_writel_reg(priv, VERT_FILTER_INDEX,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_VERT_FILTER_INDEX);
		for (i = 0; i < 128; i++)
			phytium_writel_reg(priv, kernelStates[i],
					   group_offset, PHYTIUM_DC_FRAMEBUFFER_VERT_FILTER);
		phytium_writel_reg(priv, INITIALOFFSET,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_INITIALOFFSET);
		kfree(tmp);
		phytium_crtc->scale_enable = true;
	} else {
		phytium_crtc->scale_enable = false;
	}
}

static void phytium_crtc_gamma_set(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];
	uint32_t config = 0, data;
	struct drm_crtc_state *state = crtc->state;
	struct drm_color_lut *lut;
	unsigned long flags;
	uint32_t active_line = 0, timeout = 500;
	int i;

	if (state->gamma_lut) {
		if (WARN((state->gamma_lut->length/sizeof(struct drm_color_lut) != GAMMA_INDEX_MAX),
			"gamma size is not match\n"))
			return;
		lut = (struct drm_color_lut *)state->gamma_lut->data;

		config = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
		if (config & FRAMEBUFFER_OUTPUT) {
			struct drm_display_mode *mode = &state->adjusted_mode;
			uint32_t frame_time;
			uint32_t value_a, value_b;

			frame_time = mode->crtc_vtotal * mode->crtc_htotal / mode->crtc_clock;
			value_b = (frame_time - 2) * mode->crtc_vtotal;
			local_irq_save(flags);
			do {
				active_line = phytium_readl_reg(priv, group_offset,
								PHYTIUM_DC_LOCATION);
				active_line = active_line >> LOVATION_Y_SHIFT;
				value_a = (mode->crtc_vblank_end - mode->crtc_vblank_start +
					   active_line) * frame_time;
				if (value_a < value_b)
					break;
				local_irq_restore(flags);
				udelay(1000);
				timeout--;
				local_irq_save(flags);
			} while (timeout);

			if (timeout == 0)
				DRM_ERROR("wait gamma active line timeout\n");
		}

		phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DC_GAMMA_INDEX);
		for (i = 0; i < GAMMA_INDEX_MAX; i++) {
			data = ((lut[i].red >> 6) & GAMMA_RED_MASK) << GAMMA_RED_SHIFT;
			data |= (((lut[i].green >> 6) & GAMMA_GREEN_MASK) << GAMMA_GREEN_SHIFT);
			data |= (((lut[i].blue >> 6) & GAMMA_BLUE_MASK) << GAMMA_BLUE_SHIFT);
			phytium_writel_reg(priv, data, group_offset, PHYTIUM_DC_GAMMA_DATA);
		}

		if (config & FRAMEBUFFER_OUTPUT)
			local_irq_restore(flags);
	}
}

static void phytium_crtc_gamma_init(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];
	struct drm_crtc_state *state = crtc->state;
	uint32_t config = 0, data;
	uint16_t *red, *green, *blue;
	unsigned long flags;
	uint32_t active_line = 0, timeout = 500;
	int i;

	if (WARN((crtc->gamma_size != GAMMA_INDEX_MAX), "gamma size is not match\n"))
		return;

	red = crtc->gamma_store;
	green = red + crtc->gamma_size;
	blue = green + crtc->gamma_size;

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	if (config & FRAMEBUFFER_OUTPUT) {
		struct drm_display_mode *mode = &state->adjusted_mode;
		uint32_t frame_time;
		uint32_t value_a, value_b;

		frame_time = mode->crtc_vtotal * mode->crtc_htotal / mode->crtc_clock;
		value_b = (frame_time - 2) * mode->crtc_vtotal;
		local_irq_save(flags);
		do {
			active_line = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_LOCATION);
			active_line = active_line >> LOVATION_Y_SHIFT;
			value_a = (mode->crtc_vblank_end - mode->crtc_vblank_start +
				   active_line) * frame_time;
			if (value_a < value_b)
				break;
			local_irq_restore(flags);
			udelay(1000);
			timeout--;
			local_irq_save(flags);
		} while (timeout);

		if (timeout == 0)
			DRM_ERROR("wait gamma active line timeout\n");
	}

	phytium_writel_reg(priv, 0, group_offset, PHYTIUM_DC_GAMMA_INDEX);
	for (i = 0; i < GAMMA_INDEX_MAX; i++) {
		data = ((*red++ >> 6) & GAMMA_RED_MASK) << GAMMA_RED_SHIFT;
		data |= (((*green++ >> 6) & GAMMA_GREEN_MASK) << GAMMA_GREEN_SHIFT);
		data |= (((*blue++ >> 6) & GAMMA_BLUE_MASK) << GAMMA_BLUE_SHIFT);
		phytium_writel_reg(priv, data, group_offset, PHYTIUM_DC_GAMMA_DATA);
	}

	if (config & FRAMEBUFFER_OUTPUT)
		local_irq_restore(flags);
}

static void phytium_crtc_destroy(struct drm_crtc *crtc)
{
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);

	drm_crtc_cleanup(crtc);
	kfree(phytium_crtc);
}

struct drm_crtc_state *
phytium_crtc_atomic_duplicate_state(struct drm_crtc *crtc)
{
	struct phytium_crtc_state *phytium_crtc_state = NULL;

	phytium_crtc_state = kmemdup(crtc->state, sizeof(*phytium_crtc_state),
				     GFP_KERNEL);
	if (!phytium_crtc_state)
		return NULL;
	__drm_atomic_helper_crtc_duplicate_state(crtc,
						 &phytium_crtc_state->base);

	return &phytium_crtc_state->base;
}

void
phytium_crtc_atomic_destroy_state(struct drm_crtc *crtc,
					       struct drm_crtc_state *state)
{
	struct phytium_crtc_state *phytium_crtc_state =
					to_phytium_crtc_state(state);

	phytium_crtc_state = to_phytium_crtc_state(state);
	__drm_atomic_helper_crtc_destroy_state(state);
	kfree(phytium_crtc_state);
}

static int phytium_enable_vblank(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;

	phytium_writel_reg(priv, INT_ENABLE, priv->dc_reg_base[phys_pipe], PHYTIUM_DC_INT_ENABLE);

	return 0;
}

static void phytium_disable_vblank(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;

	phytium_writel_reg(priv, INT_DISABLE, priv->dc_reg_base[phys_pipe],
			   PHYTIUM_DC_INT_ENABLE);
}

static const struct drm_crtc_funcs phytium_crtc_funcs = {
	.gamma_set		= drm_atomic_helper_legacy_gamma_set,
	.set_config		= drm_atomic_helper_set_config,
	.destroy		= phytium_crtc_destroy,
	.page_flip		= drm_atomic_helper_page_flip,
	.reset			= drm_atomic_helper_crtc_reset,
	.atomic_duplicate_state = phytium_crtc_atomic_duplicate_state,
	.atomic_destroy_state   = phytium_crtc_atomic_destroy_state,
	.enable_vblank          = phytium_enable_vblank,
	.disable_vblank         = phytium_disable_vblank,
};

static void
phytium_crtc_atomic_enable(struct drm_crtc *crtc,
				     struct drm_crtc_state *old_state)
{
	struct drm_atomic_state *state = old_state->state;
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct drm_display_mode *mode = &crtc->state->adjusted_mode;
	struct drm_connector_state *new_conn_state;
	struct drm_connector *conn;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];
	int config = 0, i = 0;

	for_each_new_connector_in_state(state, conn, new_conn_state, i) {
		if (new_conn_state->crtc != crtc)
			continue;

		switch (conn->display_info.bpc) {
		case 10:
			phytium_crtc->bpc = DP_RGB101010;
			break;
		case 6:
			phytium_crtc->bpc = DP_RGB666;
			break;
		default:
			phytium_crtc->bpc = DP_RGB888;
			break;
		}
	}

	/* config pix clock */
	phytium_crtc->dc_hw_config_pix_clock(crtc, mode->clock);

	phytium_dc_scaling_config(crtc, old_state);
	config = ((mode->crtc_hdisplay & HDISPLAY_END_MASK) << HDISPLAY_END_SHIFT)
		| ((mode->crtc_htotal&HDISPLAY_TOTAL_MASK) << HDISPLAY_TOTAL_SHIFT);
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_HDISPLAY);
	config = ((mode->crtc_hsync_start & HSYNC_START_MASK) << HSYNC_START_SHIFT)
			| ((mode->crtc_hsync_end & HSYNC_END_MASK) << HSYNC_END_SHIFT)
			| HSYNC_PULSE_ENABLED;
	config |= (mode->flags & DRM_MODE_FLAG_PHSYNC) ? 0 : HSYNC_NEGATIVE;
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_HSYNC);
	config = ((mode->crtc_vdisplay & VDISPLAY_END_MASK) << VDISPLAY_END_SHIFT)
		| ((mode->crtc_vtotal & VDISPLAY_TOTAL_MASK) << VDISPLAY_TOTAL_SHIFT);
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_VDISPLAY);
	config = ((mode->crtc_vsync_start & VSYNC_START_MASK) << VSYNC_START_SHIFT)
		| ((mode->crtc_vsync_end & VSYNC_END_MASK) << VSYNC_END_SHIFT)
		| VSYNC_PULSE_ENABLED;
	config |= (mode->flags & DRM_MODE_FLAG_PVSYNC) ? 0 : VSYNC_NEGATIVE;
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_VSYNC);
	config = PANEL_DATAENABLE_ENABLE | PANEL_DATA_ENABLE | PANEL_CLOCK_ENABLE;
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_PANEL_CONFIG);
	config = phytium_crtc->bpc | OUTPUT_DP;
	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_DP_CONFIG);

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);

	if (crtc->state->active)
		config |= FRAMEBUFFER_OUTPUT | FRAMEBUFFER_RESET;
	else
		config &= (~(FRAMEBUFFER_OUTPUT | FRAMEBUFFER_RESET));

	if (phytium_crtc->scale_enable)
		config |= FRAMEBUFFER_SCALE_ENABLE;
	else
		config &= (~FRAMEBUFFER_SCALE_ENABLE);

	config |= FRAMEBUFFER_GAMMA_ENABLE;

	if (crtc->state->gamma_lut)
		phytium_crtc_gamma_set(crtc);
	else
		phytium_crtc_gamma_init(crtc);

	phytium_writel_reg(priv, config, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	drm_crtc_vblank_on(crtc);
}

static void
phytium_crtc_atomic_disable(struct drm_crtc *crtc,
				       struct drm_crtc_state *old_state)
{
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);

	drm_crtc_vblank_off(crtc);
	phytium_crtc->dc_hw_disable(crtc);
}

static void phytium_crtc_update_timing_for_drm_display_mode(struct drm_display_mode *drm_mode,
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

static int
phytium_crtc_atomic_check(struct drm_crtc *crtc, struct drm_crtc_state *crtc_state)
{
	struct drm_atomic_state *state = crtc_state->state;
	struct drm_plane_state *new_plane_state = NULL;
	int ret = 0;
	struct drm_connector *connector;
	struct drm_connector_state *new_con_state;
	uint32_t i;
	struct phytium_dp_device *phytium_dp = NULL;

	for_each_new_connector_in_state(state, connector, new_con_state, i) {
		if (new_con_state->crtc == crtc) {
			phytium_dp = connector_to_dp_device(connector);
			break;
		}
	}
	if (phytium_dp)
		phytium_crtc_update_timing_for_drm_display_mode(&crtc_state->adjusted_mode,
								&phytium_dp->native_mode);

	new_plane_state = drm_atomic_get_new_plane_state(crtc_state->state,
							crtc->primary);
	if (crtc_state->enable && new_plane_state && !new_plane_state->crtc) {
		ret = -EINVAL;
		goto fail;
	}

	return 0;
fail:
	return ret;
}

static void
phytium_crtc_atomic_begin(struct drm_crtc *crtc,
				    struct drm_crtc_state *old_crtc_state)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	int phys_pipe = phytium_crtc->phys_pipe, config;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	if (config & FRAMEBUFFER_RESET) {
		phytium_writel_reg(priv, config | FRAMEBUFFER_VALID_PENDING,
				   group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	}
}

static void phytium_crtc_atomic_flush(struct drm_crtc *crtc,
				struct drm_crtc_state *old_crtc_state)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;
	struct phytium_crtc *phytium_crtc = to_phytium_crtc(crtc);
	struct phytium_crtc_state *phytium_crtc_state = NULL;
	int phys_pipe = phytium_crtc->phys_pipe, config;
	uint32_t group_offset = priv->dc_reg_base[phys_pipe];

	DRM_DEBUG_KMS("crtc->state active:%d enable:%d\n",
		       crtc->state->active, crtc->state->enable);
	phytium_crtc_state = to_phytium_crtc_state(crtc->state);

	if (crtc->state->color_mgmt_changed)
		phytium_crtc_gamma_set(crtc);

	config = phytium_readl_reg(priv, group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);
	phytium_writel_reg(priv, config&(~FRAMEBUFFER_VALID_PENDING),
			   group_offset, PHYTIUM_DC_FRAMEBUFFER_CONFIG);

	if (crtc->state->event) {
		DRM_DEBUG_KMS("vblank->refcount:%d\n",
			       atomic_read(&dev->vblank[0].refcount));
		spin_lock_irq(&dev->event_lock);
		if (drm_crtc_vblank_get(crtc) == 0)
			drm_crtc_arm_vblank_event(crtc, crtc->state->event);
		else
			drm_crtc_send_vblank_event(crtc, crtc->state->event);
		crtc->state->event = NULL;
		spin_unlock_irq(&dev->event_lock);
	}
}

static enum drm_mode_status
phytium_crtc_mode_valid(struct drm_crtc *crtc, const struct drm_display_mode *mode)
{
	struct drm_device *dev = crtc->dev;
	struct phytium_display_private *priv = dev->dev_private;

	if (mode->crtc_clock > priv->info.crtc_clock_max)
		return MODE_CLOCK_HIGH;

	if (mode->hdisplay > priv->info.hdisplay_max)
		return MODE_BAD_HVALUE;

	if (mode->vdisplay > priv->info.vdisplay_max)
		return MODE_BAD_VVALUE;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		return MODE_NO_INTERLACE;

	return MODE_OK;
}

static const struct drm_crtc_helper_funcs phytium_crtc_helper_funcs = {
	.mode_valid	= phytium_crtc_mode_valid,
	.atomic_check	= phytium_crtc_atomic_check,
	.atomic_begin	= phytium_crtc_atomic_begin,
	.atomic_flush	= phytium_crtc_atomic_flush,
	.atomic_enable	= phytium_crtc_atomic_enable,
	.atomic_disable	= phytium_crtc_atomic_disable,
};

void phytium_crtc_resume(struct drm_device *drm_dev)
{
	struct drm_crtc *crtc;
	struct phytium_crtc *phytium_crtc = NULL;

	drm_for_each_crtc(crtc, drm_dev) {
		phytium_crtc = to_phytium_crtc(crtc);
		if (phytium_crtc->dc_hw_reset)
			phytium_crtc->dc_hw_reset(crtc);
		phytium_crtc_gamma_init(crtc);
	}
}

int phytium_crtc_init(struct drm_device *dev, int phys_pipe)
{
	struct phytium_crtc *phytium_crtc;
	struct phytium_crtc_state *phytium_crtc_state;
	struct phytium_plane *phytium_primary_plane = NULL;
	struct phytium_plane *phytium_cursor_plane = NULL;
	struct phytium_display_private *priv = dev->dev_private;
	int ret;

	phytium_crtc = kzalloc(sizeof(*phytium_crtc), GFP_KERNEL);
	if (!phytium_crtc) {
		ret = -ENOMEM;
		goto failed_malloc_crtc;
	}

	phytium_crtc_state = kzalloc(sizeof(*phytium_crtc_state), GFP_KERNEL);
	if (!phytium_crtc_state) {
		ret = -ENOMEM;
		goto failed_malloc_crtc_state;
	}

	phytium_crtc_state->base.crtc = &phytium_crtc->base;
	phytium_crtc->base.state = &phytium_crtc_state->base;
	phytium_crtc->phys_pipe = phys_pipe;

	if (IS_PX210(priv)) {
		phytium_crtc->dc_hw_config_pix_clock = px210_dc_hw_config_pix_clock;
		phytium_crtc->dc_hw_disable = px210_dc_hw_disable;
		phytium_crtc->dc_hw_reset = NULL;
		priv->dc_reg_base[phys_pipe] = PX210_DC_BASE(phys_pipe);
		priv->dcreq_reg_base[phys_pipe] = PX210_DCREQ_BASE(phys_pipe);
		priv->address_transform_base = PX210_ADDRESS_TRANSFORM_BASE;
	} else if (IS_PE220X(priv)) {
		phytium_crtc->dc_hw_config_pix_clock = pe220x_dc_hw_config_pix_clock;
		phytium_crtc->dc_hw_disable = pe220x_dc_hw_disable;
		phytium_crtc->dc_hw_reset = pe220x_dc_hw_reset;
		priv->dc_reg_base[phys_pipe] = PE220X_DC_BASE(phys_pipe);
		priv->dcreq_reg_base[phys_pipe] = 0x0;
		priv->address_transform_base = PE220X_ADDRESS_TRANSFORM_BASE;
	}

	phytium_primary_plane = phytium_primary_plane_create(dev, phys_pipe);
	if (IS_ERR(phytium_primary_plane)) {
		ret = PTR_ERR(phytium_primary_plane);
		DRM_ERROR("create primary plane failed, phys_pipe(%d)\n", phys_pipe);
		goto failed_create_primary;
	}

	phytium_cursor_plane = phytium_cursor_plane_create(dev, phys_pipe);
	if (IS_ERR(phytium_cursor_plane)) {
		ret = PTR_ERR(phytium_cursor_plane);
		DRM_ERROR("create cursor plane failed, phys_pipe(%d)\n", phys_pipe);
		goto failed_create_cursor;
	}

	ret = drm_crtc_init_with_planes(dev, &phytium_crtc->base,
					&phytium_primary_plane->base,
					&phytium_cursor_plane->base,
					&phytium_crtc_funcs,
					"phys_pipe %d", phys_pipe);

	if (ret) {
		DRM_ERROR("init crtc with plane failed, phys_pipe(%d)\n", phys_pipe);
		goto failed_crtc_init;
	}
	drm_crtc_helper_add(&phytium_crtc->base, &phytium_crtc_helper_funcs);
	drm_crtc_vblank_reset(&phytium_crtc->base);
	drm_mode_crtc_set_gamma_size(&phytium_crtc->base, GAMMA_INDEX_MAX);
	drm_crtc_enable_color_mgmt(&phytium_crtc->base, 0, false, GAMMA_INDEX_MAX);
	if (phytium_crtc->dc_hw_reset)
		phytium_crtc->dc_hw_reset(&phytium_crtc->base);

	return 0;

failed_crtc_init:
failed_create_cursor:
	/* drm_mode_config_cleanup() will free any crtcs/planes already initialized */
failed_create_primary:
	kfree(phytium_crtc_state);
failed_malloc_crtc_state:
	kfree(phytium_crtc);
failed_malloc_crtc:
	return ret;
}
