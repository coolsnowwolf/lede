/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_DEBUGFS_H__
#define __PHYTIUM_DEBUGFS_H__

int phytium_debugfs_connector_add(struct drm_connector *connector);
int phytium_debugfs_display_register(struct phytium_display_private *priv);

#endif /* __PHYTIUM_DEBUGFS_H__ */
