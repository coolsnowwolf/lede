/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_VIRTUAL_H_
#define __FTD330_VIRTUAL_H_

#include <linux/debugfs.h>

struct ftd330_virtual_display {
	struct drm_encoder *encoder;
	struct drm_connector connector;
	struct device *dc;
	u32 bus_format;
};
/*
static inline struct ftd330_virtual_display *
to_virtual_display_with_connector(struct drm_connector *connector)
{
	return container_of(connector, struct ftd330_virtual_display, connector);
}

static inline struct ftd330_virtual_display *
to_virtual_display_with_encoder(struct drm_encoder *encoder)
{
	return container_of(encoder, struct ftd330_virtual_display, encoder);
}
*/
int ftd330_vd_pci_init(struct drm_device *drm_dev);
void ftd330_vd_pci_deinit(struct drm_device *drm_dev);

extern struct platform_driver virtual_display_platform_driver;
#endif /* __FTD330_VIRTUAL_H_ */
