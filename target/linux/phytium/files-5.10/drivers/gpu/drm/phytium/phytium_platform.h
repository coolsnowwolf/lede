/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PLATFORM_H__
#define __PHYTIUM_PLATFORM_H__

struct phytium_platform_private {
	struct phytium_display_private base;
};

#define	to_platform_priv(priv)		container_of(priv, struct phytium_platform_private, base)

extern struct platform_driver phytium_platform_driver;

#endif /* __PHYTIUM_PLATFORM_H__ */
