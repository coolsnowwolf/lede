// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#include <linux/component.h>
#include <linux/of_device.h>
#include <linux/version.h>
#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#include <linux/module.h>

#include <drm/drm_bridge.h>
#else
#include <drm/drmP.h>
#endif

#include <drm/drm_modeset_helper_vtables.h>
#include <drm/drm_crtc_helper.h>
#include <drm/drm_of.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/pci.h>

#include "ftd330_crtc.h"
#include "ftd330_dc.h"
#include "ftd330_simple_enc.h"
#include "ftd330_drv.h"

static const struct simple_encoder_priv hdmi_priv = { .encoder_type = DRM_MODE_ENCODER_TMDS };

static const struct simple_encoder_priv dp_priv = { .encoder_type = DRM_MODE_ENCODER_DPMST };

static const struct simple_encoder_priv dsi_video_priv = { .encoder_type = DRM_MODE_ENCODER_DSI };

static const struct simple_encoder_priv dsi_command_priv = {
	.encoder_type = DRM_MODE_ENCODER_DSI,
	.output_mode = FTD330_SIMPLE_ENC_OUTPUT_MODE_CMD | FTD330_SIMPLE_ENC_OUTPUT_MODE_CMD_DE_SYNC
};

static const struct simple_encoder_priv dpi_priv = { .encoder_type = DRM_MODE_ENCODER_DPI };

static const struct drm_encoder_funcs encoder_funcs = { .destroy = drm_encoder_cleanup };

static inline struct simple_encoder *to_simple_encoder(struct drm_encoder *enc)
{
	return container_of(enc, struct simple_encoder, encoder);
}

static int encoder_parse_dt(struct device *dev)
{
	struct simple_encoder *simple = dev_get_drvdata(dev);
	int ret = 0;
	int cnt, i;
	u32 *vals;
	u32 *masks;

	/* TODO: dss-syscon not used now */
	/*
	 *simple->dss_regmap =
	 *	syscon_regmap_lookup_by_phandle(dev->of_node, "phytium,dss-syscon");

	 *if (IS_ERR(simple->dss_regmap)) {
	 *	if (PTR_ERR(simple->dss_regmap) != -ENODEV) {
	 *		dev_err(dev, "failed to get dss-syscon\n");
	 *		ret = PTR_ERR(simple->dss_regmap);
	 *		goto err;
	 *	}
	 *	simple->dss_regmap = NULL;
	 *	goto err;
	 *}
	 */

	cnt = of_property_count_elems_of_size(dev->of_node, "phytium,mux-mask", 4);
	if (!cnt) {
		ret = cnt;
		goto err;
	}

	simple->dss_regdatas = devm_kzalloc(dev, sizeof(*simple->dss_regdatas) * cnt, GFP_KERNEL);

	masks = kcalloc(cnt, sizeof(*masks), GFP_KERNEL);
	if (!masks) {
		ret = -ENOMEM;
		goto err;
	}

	vals = kcalloc(cnt, sizeof(*vals), GFP_KERNEL);
	if (!vals) {
		ret = -ENOMEM;
		goto err_free_masks;
	}

	ret = of_property_read_u32_array(dev->of_node, "phytium,mux-mask", masks, cnt);
	if (ret)
		goto err_free_vals;

	ret = of_property_read_u32_array(dev->of_node, "phytium,mux-val", vals, cnt);
	if (ret)
		goto err_free_vals;

	ret = of_property_read_u16(dev->of_node, "phytium,mux-id", &simple->mux_id);
	if (ret)
		return ret;

	if (FTD330_SIMPLE_ENC_ENCODER_ID(simple->mux_id) > DC_OUTPUT_NUM) {
		pr_err("encoder mux id out of range.\n");
		return -ENODEV;
	}

	for (i = 0; i < cnt; i++) {
		simple->dss_regdatas[i].mask = masks[i];
		simple->dss_regdatas[i].value = vals[i];
	}

err_free_vals:
	kfree(vals);
err_free_masks:
	kfree(masks);
err:
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
static void encoder_atomic_enable(struct drm_encoder *encoder, struct drm_atomic_state *state)
{
	struct simple_encoder *simple = to_simple_encoder(encoder);
	struct dss_data *data = simple->dss_regdatas;
	int crtc_id;

	if (!simple->dss_regmap)
		return;

	/* TODO: dss not used now. */
	crtc_id = drm_of_encoder_active_endpoint_id(simple->dev->of_node, encoder);

	regmap_update_bits(simple->dss_regmap, 0, data[crtc_id].mask, data[crtc_id].value);
}
#endif

static int encoder_atomic_check(struct drm_encoder *encoder, struct drm_crtc_state *crtc_state,
				struct drm_connector_state *conn_state)
{
	struct ftd330_crtc_state *ftd330_crtc_state = to_ftd330_crtc_state(crtc_state);
	struct drm_connector *connector = conn_state->connector;
	struct simple_encoder *simple_enc = to_simple_encoder(encoder);
	struct drm_display_mode *mode = &crtc_state->adjusted_mode;
	u32 max_with = simple_enc->priv->max_w;
	u32 max_height = simple_enc->priv->max_h;
	int ret = 0;

#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	struct drm_bridge *first_bridge = drm_bridge_chain_get_first_bridge(encoder);
	struct drm_bridge_state *bridge_state = ERR_PTR(-EINVAL);
#endif

	ftd330_crtc_state->encoder_type = encoder->encoder_type;
	ftd330_crtc_state->output_id = FTD330_SIMPLE_ENC_OUTPUT_ID(simple_enc->mux_id);
	ftd330_crtc_state->output_mode = simple_enc->priv->output_mode;

#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	if (first_bridge && first_bridge->funcs->atomic_duplicate_state)
		bridge_state = drm_atomic_get_bridge_state(crtc_state->state, first_bridge);

	if (IS_ERR(bridge_state)) {
		if (connector->display_info.num_bus_formats)
			ftd330_crtc_state->output_fmt = connector->display_info.bus_formats[0];
		else
			ftd330_crtc_state->output_fmt = MEDIA_BUS_FMT_FIXED;
	} else {
		ftd330_crtc_state->output_fmt = bridge_state->input_bus_cfg.format;
	}
#else
	if (connector->display_info.num_bus_formats)
		ftd330_crtc_state->output_fmt = connector->display_info.bus_formats[0];
	else
		ftd330_crtc_state->output_fmt = MEDIA_BUS_FMT_RGB888_1X24;
#endif

	switch (ftd330_crtc_state->output_fmt) {
	case MEDIA_BUS_FMT_FIXED:
	case MEDIA_BUS_FMT_RGB565_1X16:
	case MEDIA_BUS_FMT_RGB666_1X18:
	case MEDIA_BUS_FMT_RGB888_1X24:
	case MEDIA_BUS_FMT_RGB666_1X24_CPADHI:
	case MEDIA_BUS_FMT_RGB101010_1X30:
	case MEDIA_BUS_FMT_RGB121212_1X36:
	case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
	case MEDIA_BUS_FMT_UYVY8_1X16:
	case MEDIA_BUS_FMT_YUV8_1X24:
	case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
	case MEDIA_BUS_FMT_UYVY10_1X20:
	case MEDIA_BUS_FMT_YUV10_1X30:
		ret = 0;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	/* If MEDIA_BUS_FMT_FIXED, set it to default value */
	if (ftd330_crtc_state->output_fmt == MEDIA_BUS_FMT_FIXED)
		ftd330_crtc_state->output_fmt = MEDIA_BUS_FMT_RGB888_1X24;

	if (max_with || max_height) {
		if (mode->hdisplay > max_with || mode->vdisplay > max_height) {
			pr_err("%s the encoder_type %d is not support the display size,max_w:%d, max_h:%d.\n",
			       __func__, encoder->encoder_type, max_with, max_height);
			ret = -EINVAL;
		}
	}
	return ret;
}

static const struct drm_encoder_helper_funcs encoder_helper_funcs = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	.atomic_enable = encoder_atomic_enable,
#endif
	.atomic_check = encoder_atomic_check,
};

static int encoder_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);
	struct simple_encoder *simple = dev_get_drvdata(dev);
	struct drm_encoder *encoder;
	int ret;

	encoder = &simple->encoder;

	/* Encoder. */
	ret = drm_encoder_init(drm_dev, encoder, &encoder_funcs, simple->priv->encoder_type, NULL);
	if (ret)
		return ret;

	drm_encoder_helper_add(encoder, &encoder_helper_funcs);
	encoder->possible_crtcs = drm_of_find_possible_crtcs(drm_dev, dev->of_node);
	simple->dev = priv->dc_dev;
	dc->encoder[FTD330_SIMPLE_ENC_ENCODER_ID(simple->mux_id)] = simple;

	/* TODO: panel/bridge part connet to encoder */
	/*
	 *output port is port1
	 *struct drm_bridge *bridge = NULL;
	 *#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	 *	struct drm_panel *panel = NULL;
	 *#endif

	 *#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	 *ret = drm_of_find_panel_or_bridge(dev->of_node, 1, -1, &panel, &bridge);
	 *#else
	 *ret = drm_of_find_panel_or_bridge(dev->of_node, 1, -1, NULL, &bridge);
	 *#endif
	 *if (ret)
	 *	goto err;

	 *#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
	 *if (panel)
	 *	bridge = devm_drm_panel_bridge_add(dev, panel);
	 *#endif

	 *#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
	 *ret = drm_bridge_attach(encoder, bridge, NULL, 0);
	 *#else
	 *ret = drm_bridge_attach(encoder, bridge, NULL);
	 *#endif
	 *if (ret)
	 *	goto err;

	 *return 0;
	 *err:
	 *	drm_encoder_cleanup(encoder);
	 */

	return ret;
}

static void encoder_unbind(struct device *dev, struct device *master, void *data)
{
	struct simple_encoder *simple = dev_get_drvdata(dev);

	drm_encoder_cleanup(&simple->encoder);
}

/* For PCI, encoder create */
static struct simple_encoder *_ftd330_simple_encoder_create(struct drm_device *drm_dev,
							const struct ftd330_output_info *info, u8 index)
{
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct device *dev = &drm_dev->pdev->dev;
#else
	struct pci_dev *pdev = to_pci_dev(drm_dev->dev);
	struct device *dev = &pdev->dev;
#endif
	struct simple_encoder *simple = NULL;
	struct drm_encoder *encoder = NULL;
	const struct simple_encoder_priv *priv = NULL;
	struct simple_encoder_priv *temp_priv = NULL;

	simple = devm_kzalloc(dev, sizeof(*simple), GFP_KERNEL);
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (simple == NULL || priv == NULL)
		return NULL;

	simple->priv = priv;
	encoder = &simple->encoder;
	temp_priv = (struct simple_encoder_priv *)simple->priv;
	temp_priv->encoder_type = info[index].type;
	temp_priv->output_mode = info[index].output_mode;
	temp_priv->max_w = info[index].max_w;
	temp_priv->max_h = info[index].max_h;
	simple->mux_id = info[index].mux_id;

	/* Encoder. */
	if (drm_encoder_init(drm_dev, encoder, &encoder_funcs, simple->priv->encoder_type, NULL))
		return NULL;

	drm_encoder_helper_add(encoder, &encoder_helper_funcs);

	return simple;
}

int ftd330_simple_encoder_pci_init(struct drm_device *drm_dev)
{
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct device *dev = &drm_dev->pdev->dev;
#else
	struct pci_dev *pdev = to_pci_dev(drm_dev->dev);
	struct device *dev = &pdev->dev;
#endif
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct drm_crtc *crtc = NULL;
	struct simple_encoder *simple_encoder = NULL;
	const struct ftd330_output_info *output_info = dc->hw.output_info;
	const struct ftd330_dc_info *dc_info = dc->hw.info;
	u8 i = 0;
	u32 crtc_mask = 0;

	drm_for_each_crtc(crtc, drm_dev)
		crtc_mask |= drm_crtc_mask(crtc);

	/* simple encoder */
	for (i = 0; i < dc_info->output_num; i++) {
		simple_encoder = _ftd330_simple_encoder_create(drm_dev, output_info, i);
		if (simple_encoder == NULL)
			return -EINVAL;

		simple_encoder->encoder.possible_crtcs = crtc_mask;
		dc->encoder[FTD330_SIMPLE_ENC_ENCODER_ID(simple_encoder->mux_id)] = simple_encoder;
	}

	return 0;
}

void ftd330_simple_encoder_pci_deinit(struct drm_device *drm_dev)
{
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct device *dev = &drm_dev->pdev->dev;
#else
	struct pci_dev *pdev = to_pci_dev(drm_dev->dev);
	struct device *dev = &pdev->dev;
#endif
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct simple_encoder *simple = NULL;
	u8 i = 0;

	for (i = 0; i < dc->hw.info->output_num; i++) {
		simple = dc->encoder[i];
		drm_encoder_cleanup(&simple->encoder);
	}
}

static const struct component_ops encoder_component_ops = {
	.bind = encoder_bind,
	.unbind = encoder_unbind,
};

static const struct of_device_id simple_encoder_dt_match[] = {
	{ .compatible = "phytium,hdmi-encoder", .data = &hdmi_priv },
	{ .compatible = "phytium,dp-encoder", .data = &dp_priv },
	{ .compatible = "phytium,dsi-encoder", .data = &dsi_video_priv },
	{ .compatible = "phytium,dsi-cmd-encoder", .data = &dsi_command_priv },
	{ .compatible = "phytium,dpi-encoder", .data = &dpi_priv },
	{},

};
MODULE_DEVICE_TABLE(of, simple_encoder_dt_match);

static int encoder_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct simple_encoder *simple;
	int ret;

	simple = devm_kzalloc(dev, sizeof(*simple), GFP_KERNEL);
	if (!simple)
		return -ENOMEM;

	simple->priv = of_device_get_match_data(dev);

	simple->dev = dev;

	dev_set_drvdata(dev, simple);

	ret = encoder_parse_dt(dev);
	if (ret)
		return ret;

	return component_add(dev, &encoder_component_ops);
}

static int encoder_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	component_del(dev, &encoder_component_ops);

	dev_set_drvdata(dev, NULL);

	return 0;
}

struct platform_driver simple_encoder_driver = {
	.probe = encoder_probe,
	.remove = encoder_remove,
	.driver = {
		.name = "phy-simple-encoder",
		.of_match_table = of_match_ptr(simple_encoder_dt_match),
	},
};

MODULE_DESCRIPTION("Simple Encoder Driver");
MODULE_LICENSE("GPL v2");
