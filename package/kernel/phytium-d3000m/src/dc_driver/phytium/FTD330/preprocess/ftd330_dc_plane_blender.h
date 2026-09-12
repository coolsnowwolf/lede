/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_PLANE_BLENDER_H__
#define __FTD330_DC_PLANE_BLENDER_H__

#include "drm/ftd330_drm.h"
#include "ftd330_type.h"
#include "ftd330_dc_property.h"

bool ftd330_dc_register_plane_blender_states(struct ftd330_dc_property_state_group *states,
				   const struct ftd330_plane_info *info);

#endif
