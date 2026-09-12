/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_SIMPLE_ENC_H_
#define __FTD330_SIMPLE_ENC_H_

#include <drm/drm_encoder.h>

#define FTD330_SIMPLE_ENC_OUTPUT_ID_MASK 0xFF00
#define FTD330_SIMPLE_ENC_ENCODER_ID_MASK 0x00FF

#define FTD330_SIMPLE_ENC_MUX_ID(output_id, encoder_id) ((output_id << 8) | encoder_id)

#define FTD330_SIMPLE_ENC_OUTPUT_ID(mux_id) ((mux_id & FTD330_SIMPLE_ENC_OUTPUT_ID_MASK) >> 8)

#define FTD330_SIMPLE_ENC_ENCODER_ID(mux_id) (mux_id & FTD330_SIMPLE_ENC_ENCODER_ID_MASK)

#define FTD330_SIMPLE_ENC_OUTPUT_MODE_CMD BIT(0)
#define FTD330_SIMPLE_ENC_OUTPUT_MODE_CMD_AUTO BIT(1)
#define FTD330_SIMPLE_ENC_OUTPUT_MODE_CMD_DE_SYNC BIT(2)
#define FTD330_SIMPLE_ENC_OUTPUT_MODE_STANDARD_DPI BIT(3)
#define FTD330_SIMPLE_ENC_OUTPUT_MODE_HW_TE_EDPI BIT(4)
#define FTD330_SIMPLE_ENC_OUTPUT_MODE_SW_TE_EDPI BIT(5)

struct simple_encoder_priv {
	u8 encoder_type;
	u32 output_mode;
	u32 max_w;
	u32 max_h;
};

struct dss_data {
	u32 mask;
	u32 value;
};

struct simple_encoder {
	struct drm_encoder encoder;
	u16 mux_id;
	struct device *dev;
	const struct simple_encoder_priv *priv;
	struct regmap *dss_regmap;
	struct dss_data *dss_regdatas;
};

extern struct platform_driver simple_encoder_driver;

struct drm_device;

int ftd330_simple_encoder_pci_init(struct drm_device *drm_dev);
void ftd330_simple_encoder_pci_deinit(struct drm_device *drm_dev);

#endif /* __FTD330_SIMPLE_ENC_H_ */
