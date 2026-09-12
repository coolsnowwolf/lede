/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_SCALE_H_
#define __FTD330_DC_SCALE_H_


#include <drm/ftd330_drm.h>
#include <linux/math64.h>
#include <asm/neon.h>
#include "ftd330_crtc.h"
#include "ftd330_plane.h"
#include "phytium_dp.h"
#include "ftd330_dc.h"

#define MAXKERNELSIZE           9
#define SUBPIXELINDEXBITS       5
#define SUBPIXELCOUNT           (1 << SUBPIXELINDEXBITS)
#define SUBPIXELLOADCOUNT       (SUBPIXELCOUNT / 2 + 1)
#define WEIGHTSTATECOUNT        (((SUBPIXELLOADCOUNT * MAXKERNELSIZE + 1) & ~1) / 2)
#define KERNELTABLESIZE         (SUBPIXELLOADCOUNT * MAXKERNELSIZE * sizeof(uint16_t))
#define PHYALIGN(n, align)      (((n) + ((align) - 1)) & ~((align) - 1))
#define KERNELSTATES            (PHYALIGN(KERNELTABLESIZE + 4, 8))
#define PHYPI                   3.14159265358979323846f

#define MATH_Add(X, Y)                  ((float)((X) + (Y)))
#define MATH_Multiply(X, Y)             ((float)((X) * (Y)))
#define MATH_Divide(X, Y)               ((float)((X) / (Y)))
#define MATH_DivideFromUInteger(X, Y)   ((float)(X) / (float)(Y))
#define MATH_I2Float(X)         ((float)(X))

#define PHYTIUM_DC_FRAMEBUFFER_SCALE_FACTOR_X           0x1828
	#define SCALE_FACTOR_X_MASK                             0x7fffffff
#define PHYTIUM_DC_FRAMEBUFFER_SCALE_FACTOR_Y           0x1830
	#define SCALE_FACTOR_Y_MASK                             0x7fffffff
	#define SCALE_FACTOR_Y_MAX                              0x3
	#define SCALE_FACTOR_SRC_OFFSET                         16
#define PHYTIUM_DC_FRAMEBUFFER_SCALECONFIG              0x1520
	#define FRAMEBUFFER_FILTER_TAP                          3
	#define FRAMEBUFFER_HORIZONTAL_FILTER_TAP               3
	#define FRAMEBUFFER_TAP                                 0x33

void phytium_dc_scale_register_config(struct dc_hw *hw, uint32_t display_id, uint32_t hw_id);
void phytium_dc_scaling_config(struct dc_hw *hw, u8 plane_id, struct dc_hw_position *pos);

struct filter_blit_array {
	uint8_t kernelSize;
	uint32_t scaleFactor;
	uint32_t *kernelStates;
};

enum phytium_scale_status {
	SCALE_FALSE,
	SCALE_TRUE,
};

#endif
