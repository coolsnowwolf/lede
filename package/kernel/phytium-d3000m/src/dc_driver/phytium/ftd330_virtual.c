// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/component.h>
#include <linux/media-bus-format.h>
#include <linux/of_platform.h>

#include <drm/drm_atomic_helper.h>
#include <drm/drm_encoder.h>
#include <drm/drm_of.h>
#include <drm/drm_plane_helper.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
#include <drm/drm_probe_helper.h>
#endif

#include "ftd330_dc.h"
#include "ftd330_gem.h"
#include "ftd330_virtual.h"

#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#ifdef CONFIG_PCI
#include <linux/pci.h>
#endif
#endif

static int vd_get_modes(struct drm_connector *connector)
{
	struct drm_device *dev = connector->dev;
	struct drm_display_mode *mode = NULL;
	unsigned int i;
	static const struct display_mode {
		int w, h, refresh;
	} cvt_mode[] = { { 640, 480, 60 },    { 720, 480, 60 },	   { 800, 600, 60 },
			 { 1024, 768, 60 },   { 1280, 720, 60 },   { 1280, 1024, 60 },
			 { 1400, 1050, 60 },  { 1440, 3520, 120 }, { 1440, 3520, 144 },
			 { 1680, 1050, 60 },  { 1600, 1200, 60 },  { 1920, 1080, 60 },
			 { 1920, 1080, 120 }, { 1920, 1200, 60 },  { 2340, 3404, 120 },
			 { 2500, 2820, 120 }, { 2500, 2820, 144 }, { 2700, 2600, 120 },
			 { 2700, 2600, 144 }, { 3200, 1920, 120 }, { 3840, 2160, 60 },
			 { 4096, 2160, 60 }, { 3840, 2160, 120 }, { 7680, 4320, 30 } };

	for (i = 0; i < ARRAY_SIZE(cvt_mode); i++) {
		mode = drm_cvt_mode(dev, cvt_mode[i].w, cvt_mode[i].h, cvt_mode[i].refresh, false,
				    false, false);
		drm_mode_probed_add(connector, mode);
	}
	return 0;
}

static struct drm_encoder *vd_best_encoder(struct drm_connector *connector)
{
	struct ftd330_virtual_display *vd;

	vd = to_virtual_display_with_connector(connector);
	return vd->encoder;
}

static enum drm_mode_status vd_mode_valid(struct drm_connector *connector,
					  struct drm_display_mode *mode)
{
	return MODE_OK;
}

static const struct drm_connector_helper_funcs vd_connector_helper_funcs = {
	.get_modes = vd_get_modes,
	.mode_valid = vd_mode_valid,
	.best_encoder = vd_best_encoder,
};

static int vd_connector_debugfs_show(struct seq_file *s, void *data)
{
	struct drm_connector *connector = s->private;

	seq_printf(s, "connector[%u]: %s\n", connector->base.id, connector->name);
	seq_printf(s, "\tbus format instructions:\n\t\tbus format\t\t\tvalue\n\
\t\tMEDIA_BUS_FMT_RGB565_1X16\t0x1017\n\t\tMEDIA_BUS_FMT_RGB666_1X18\t0x1009\n\
\t\tMEDIA_BUS_FMT_RGB666_1X24_CPADHI\t0x1015\n\t\tMEDIA_BUS_FMT_RGB888_1X24\t0x100a\n\
\t\tMEDIA_BUS_FMT_RGB101010_1X30\t0x1018\n\t\tMEDIA_BUS_FMT_RGB121212_1X36\t0x1019\n");
	seq_printf(s, "\tbus format = 0x%x\n", connector->display_info.bus_formats[0]);

	return 0;
}

static int vd_connector_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, vd_connector_debugfs_show, inode->i_private);
}

static ssize_t vd_connector_debugfs_write(struct file *file, const char __user *ubuf, size_t len,
					  loff_t *offp)
{
	struct seq_file *s = file->private_data;
	struct drm_connector *connector = s->private;
	char buf[96], *cur = buf;
	unsigned long long bus_format_val;

	if (len > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, len))
		return -EINVAL;

	buf[len] = '\0';

	cur = strstr(buf, "bus_format:0x");
	if (cur) {
		cur += 13;
		if (kstrtoull(cur, 16, &bus_format_val))
			return -EINVAL;
		drm_display_info_set_bus_formats(&connector->display_info, (u32 *)(&bus_format_val),
						 1);
	} else {
		return -EINVAL;
	}

	return len;
}

static const struct file_operations vd_connector_debugfs_fops = {
	.open = vd_connector_debugfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
	.write = vd_connector_debugfs_write,
};

static int vd_connector_debugfs_init(struct drm_connector *connector)
{
	debugfs_create_file("output_format", 0644, connector->debugfs_entry, connector,
			    &vd_connector_debugfs_fops);

	return 0;
}

static int vd_connector_late_register(struct drm_connector *connector)
{
	return vd_connector_debugfs_init(connector);
}

static void vd_connector_destroy(struct drm_connector *connector)
{
	drm_connector_unregister(connector);
	drm_connector_cleanup(connector);
}

static enum drm_connector_status vd_connector_detect(struct drm_connector *connector, bool force)
{
	return connector_status_connected;
}

static void vd_connector_reset(struct drm_connector *connector)
{
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
	struct drm_device *dev = connector->dev;
	struct drm_minor *minor = container_of(&dev, struct drm_minor, dev);
	struct drm_file *file_priv = container_of(&minor, struct drm_file, minor);
#endif
	struct drm_encoder *encoder = NULL;

	struct drm_connector_state *conn_state = kzalloc(sizeof(*conn_state), GFP_KERNEL);

	if (connector->state)
		__drm_atomic_helper_connector_destroy_state(connector->state);

	kfree(connector->state);
	__drm_atomic_helper_connector_reset(connector, conn_state);

	/* fill out the first possible encoder as the best encoder. */
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
	encoder = drm_encoder_find(dev, file_priv, connector->encoder_ids[0]);
#else
	drm_for_each_encoder_mask(encoder, connector->dev, connector->possible_encoders)
		break;
#endif

	conn_state->best_encoder = encoder;
}

static const struct drm_connector_funcs vd_connector_funcs = {
	.fill_modes = drm_helper_probe_single_connector_modes,
	.late_register = vd_connector_late_register,
	.destroy = vd_connector_destroy,
	.detect = vd_connector_detect,
	.atomic_duplicate_state = drm_atomic_helper_connector_duplicate_state,
	.atomic_destroy_state = drm_atomic_helper_connector_destroy_state,
	.reset = vd_connector_reset,
};

static int vd_bind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);
	struct drm_connector *connector = NULL;
	int ret;
	u32 i = 0;
	u16 id = 0;
	u8 bpp = 0;
	u32 port_count = 0;
	struct device_node *parent_node, *remote_port;
	struct ftd330_virtual_display *vd = NULL;

	parent_node = of_find_node_by_name(dev->of_node, "ports");
	if (!parent_node) {
		pr_err("failed to find ports in virtual display.\n");
		return -ENODEV;
	}

	port_count = of_get_child_count(parent_node);

	for (i = 0; i < port_count && i < DC_OUTPUT_NUM; i++) {
		/* get the remote encoder */
		remote_port = of_graph_get_remote_node(dev->of_node, i, 0);
		if (!remote_port) {
			of_node_put(parent_node);
			ret = -EINVAL;
			goto attach_err;
		}

		of_property_read_u16(remote_port, "phytium,mux-id", &id);
		of_node_put(remote_port);
		of_property_read_u8(dev->of_node, "bpp", &bpp);
		/* create virtual display */
		vd = devm_kzalloc(dev, sizeof(*vd), GFP_KERNEL);
		if (!vd)
			return -ENOMEM;

		vd->bus_format = MEDIA_BUS_FMT_RGB101010_1X30;
		if (bpp == 8)
			vd->bus_format = MEDIA_BUS_FMT_RGB888_1X24;

		vd->dc = priv->dc_dev;
		/* bind the encoder */
		vd->encoder = &dc->encoder[FTD330_SIMPLE_ENC_ENCODER_ID(id)]->encoder;
		/* Connector */
		connector = &vd->connector;
		ret = drm_connector_init(drm_dev, connector, &vd_connector_funcs,
					 DRM_MODE_CONNECTOR_VIRTUAL);
		if (ret)
			goto connector_init_err;

		drm_connector_helper_add(connector, &vd_connector_helper_funcs);
		connector->interlace_allowed = false;
		connector->doublescan_allowed = false;
		connector->dpms = DRM_MODE_DPMS_OFF;
		connector->polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;
		ret = drm_connector_register(connector);
		if (ret)
			goto connector_reg_err;

		drm_display_info_set_bus_formats(&connector->display_info, &vd->bus_format, 1);
		/* attach */
		ret = drm_connector_attach_encoder(connector, vd->encoder);
		if (ret)
			goto attach_err;

		dc->vd[i] = vd;
	}

	dev_set_drvdata(dev, dc->vd);
	return 0;

attach_err:
	drm_connector_unregister(connector);
connector_reg_err:
	drm_connector_cleanup(connector);
connector_init_err:
	drm_encoder_cleanup(vd->encoder);
	return ret;
}

static void vd_unbind(struct device *dev, struct device *master, void *data)
{
	struct drm_device *drm_dev = data;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
	struct ftd330_dc *dc = dev_get_drvdata(priv->dc_dev);
	struct device_node *parent_node = NULL;
	u32 port_count = 0, i = 0;

	parent_node = of_find_node_by_name(dev->of_node, "ports");
	port_count = of_get_child_count(parent_node);

	for (i = 0; i < port_count && i < DC_OUTPUT_NUM; i++) {
		drm_connector_unregister(&dc->vd[i]->connector);
		drm_connector_cleanup(&dc->vd[i]->connector);
	}
}

const struct component_ops vd_component_ops = {
	.bind = vd_bind,
	.unbind = vd_unbind,
};

static const struct of_device_id vd_driver_dt_match[] = {
	{
		.compatible = "phytium,virtual_display",
	},
	{},
};
MODULE_DEVICE_TABLE(of, vd_driver_dt_match);

int ftd330_vd_pci_init(struct drm_device *drm_dev)
{
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct device *dev = &drm_dev->pdev->dev;
#else
	struct pci_dev *pdev = to_pci_dev(drm_dev->dev);
	struct device *dev = &pdev->dev;
#endif
	struct ftd330_virtual_display *vd = NULL;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	const struct ftd330_dc_info *dc_info = dc->hw.info;
	struct simple_encoder *simple_encoder = NULL;
	struct drm_connector *connector = NULL;
	int ret;
	u32 i;

	for (i = 0; i < dc->hw.info->output_num; i++) {
		vd = devm_kzalloc(dev, sizeof(*vd), GFP_KERNEL);
		if (!vd)
			return -ENOMEM;

		vd->bus_format = MEDIA_BUS_FMT_RGB888_1X24;
		vd->dc = dev;
		dc->vd[i] = vd;
	}

	for (i = 0; i < dc_info->output_num; i++) {
		vd = dc->vd[i];
		simple_encoder = dc->encoder[i];
		vd->encoder = &simple_encoder->encoder;

		/* Connector */
		connector = &vd->connector;
		ret = drm_connector_init(drm_dev, connector, &vd_connector_funcs,
					 DRM_MODE_CONNECTOR_VIRTUAL);
		if (ret)
			goto connector_init_err;

		drm_connector_helper_add(connector, &vd_connector_helper_funcs);
		connector->interlace_allowed = false;
		connector->doublescan_allowed = false;
		connector->dpms = DRM_MODE_DPMS_OFF;
		connector->polled = DRM_CONNECTOR_POLL_CONNECT | DRM_CONNECTOR_POLL_DISCONNECT;
		ret = drm_connector_register(connector);
		if (ret)
			goto connector_reg_err;

		drm_display_info_set_bus_formats(&connector->display_info, &vd->bus_format, 1);

		/* attach */
		ret = drm_connector_attach_encoder(connector, vd->encoder);
		if (ret)
			goto attach_err;
	}

	return 0;

attach_err:
	drm_connector_unregister(connector);
connector_reg_err:
	drm_connector_cleanup(connector);
connector_init_err:
	drm_encoder_cleanup(vd->encoder);
	return ret;
}

void ftd330_vd_pci_deinit(struct drm_device *drm_dev)
{
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct device *dev = &drm_dev->pdev->dev;
#else
	struct pci_dev *pdev = to_pci_dev(drm_dev->dev);
	struct device *dev = &pdev->dev;
#endif
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	const struct ftd330_dc_info *dc_info = dc->hw.info;
	struct ftd330_virtual_display *vd = NULL;
	u8 i = 0;

	for (i = 0; i < dc_info->output_num; i++) {
		vd = dc->vd[i];
		drm_connector_unregister(&vd->connector);
		drm_connector_cleanup(&vd->connector);
	}
}

static int vd_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	return component_add(dev, &vd_component_ops);
}

static int vd_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	component_del(dev, &vd_component_ops);

	dev_set_drvdata(dev, NULL);

	return 0;
}

struct platform_driver virtual_display_platform_driver = {
	.probe = vd_probe,
	.remove = vd_remove,
	.driver = {
	    .name = "phy-virtual-display",
	    .of_match_table = of_match_ptr(vd_driver_dt_match),
	},
};

MODULE_DESCRIPTION("Phytium Virtual Display Driver");
MODULE_LICENSE("GPL v2");
