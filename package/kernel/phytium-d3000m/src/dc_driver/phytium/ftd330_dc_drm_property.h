/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_DRM_PROPERTY_H__
#define __FTD330_DC_DRM_PROPERTY_H__

#include <drm/drm_device.h>
#include <drm/drm_mode_object.h>

#include "ftd330_dc_property.h"

/* DONOT include ftd330_dc.h since it containes ftd330_crtc.h
 * and ftd330_crtc.h contains this file
 */
struct ftd330_dc;
struct ftd330_plane_state;
struct ftd330_crtc_state;
struct ftd330_writeback_connector_state;

struct ftd330_drm_property {
	const struct ftd330_dc_property_proto *proto;
	struct drm_property *data;
};

struct ftd330_drm_property_group {
	u32 num;
	struct ftd330_drm_property items[FTD330_DC_MAX_PROPERTY_NUM];
};

struct ftd330_drm_property_state {
	const struct ftd330_dc_property_proto *proto;
	union value {
		struct drm_property_blob *blob;
		bool boolean;
		u32 u32val;
		u8 u8val;
		u16 u16val;
		u64 u64val;
		int enumval;
	} value;
	bool is_changed;
};

struct drm_property *ftd330_dc_create_drm_property(struct drm_device *drm_dev,
					       struct drm_mode_object *obj,
					       const struct ftd330_dc_property_proto *proto);

int ftd330_dc_create_drm_properties(struct drm_device *dev, struct drm_mode_object *obj,
						const struct ftd330_dc_property_state_group *dc_states,
						struct ftd330_drm_property_group *properties);

void ftd330_dc_duplicate_drm_properties(struct ftd330_drm_property_state *new_states,
							const struct ftd330_drm_property_state *old_states,
							const struct ftd330_drm_property_group *properties);

void ftd330_dc_destroy_drm_properties(struct ftd330_drm_property_state *states,
					const struct ftd330_drm_property_group *properties);

int ftd330_dc_set_drm_property(struct drm_device *dev, struct ftd330_drm_property_state *states,
					const struct ftd330_drm_property_group *properties,
					const struct drm_property *property, u64 val);

int ftd330_dc_get_drm_property(const struct ftd330_drm_property_state *states,
					const struct ftd330_drm_property_group *properties,
					const struct drm_property *property, u64 *val);

void ftd330_dc_update_drm_properties_to_dc(struct ftd330_dc *dc, u8 hw_id,
					const struct ftd330_drm_property_state *drm_states,
					u32 registered_drm_properties_num,
					struct ftd330_dc_property_state_group *dc_states,
					const void *obj_state);

struct ftd330_drm_property_state *ftd330_dc_get_drm_property_state(struct ftd330_drm_property_state *states,
							   u32 num, const char *name);
bool ftd330_dc_check_crtc_std_property(struct ftd330_dc *dc, u8 hw_id, struct drm_crtc *crtc);

bool ftd330_dc_check_drm_property(struct ftd330_dc *dc, u8 hw_id,
			      const struct ftd330_drm_property_state *states, u32 num,
			      const void *obj_state);

const void *ftd330_dc_drm_plane_property_get(const struct ftd330_plane_state *plane, const char *name,
					 u32 *out_len);

const void *ftd330_dc_drm_crtc_property_get(const struct ftd330_crtc_state *crtc, const char *name,
					u32 *out_len);
#ifdef CONFIG_PHYTIUM_WRITEBACK
const void *ftd330_dc_drm_connector_property_get(const struct ftd330_writeback_connector_state *wb,
					const char *name, u32 *out_len);
#endif
#endif
