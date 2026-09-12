// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_drm_property.h"

#include <drm/drm_device.h>
#include <drm/drm_mode_object.h>
#include <drm/drm_property.h>
#include "drm/ftd330_drm.h"
#include "ftd330_dc.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_drm_property.h"

static int _set_property_blob_from_id(struct drm_device *dev, struct drm_property_blob **blob,
				      uint64_t blob_id, size_t expected_size, size_t element_size,
				      bool *changed)
{
	struct drm_property_blob *new_blob = NULL;
	bool data_changed = false;

	if (blob_id) {
		new_blob = drm_property_lookup_blob(dev, blob_id);
		if (new_blob == NULL)
			return -EINVAL;

		if (expected_size > 0 && new_blob->length != expected_size) {
			drm_property_blob_put(new_blob);
			return -EINVAL;
		} else if (element_size > 0 && (new_blob->length % element_size) != 0) {
			/* For dynamic array */
			drm_property_blob_put(new_blob);
			return -EINVAL;
		}
	}

	if ((*blob) && blob_id) {
		if (memcmp(new_blob->data, (*blob)->data, expected_size) == 0) {
			drm_property_blob_put(new_blob);

			if (changed)
				*changed = false;
			return 0;
		}
	}

	data_changed = drm_property_replace_blob(blob, new_blob);
	if (changed)
		*changed = data_changed;

	drm_property_blob_put(new_blob);

	return 0;
}

struct drm_property *ftd330_dc_create_drm_property(struct drm_device *drm_dev,
					       struct drm_mode_object *obj,
					       const struct ftd330_dc_property_proto *proto)
{
	struct drm_property *property = NULL;

	if (!proto) {
		pr_err("%s: Invalid proto %p\n", __func__, proto);
		return NULL;
	}
	switch (proto->type) {
	case FTD330_DC_PROPERTY_BLOB:
		property = drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, proto->name,
					       0);
		break;
	case FTD330_DC_PROPERTY_ARRAY: {
		/* u32 type_size = proto->sub_proto.array.dynamic ? 0 : proto->type_size; */

		property = drm_property_create(drm_dev, DRM_MODE_PROP_BLOB, proto->name, 0);
	} break;
	case FTD330_DC_PROPERTY_BOOL:
		property = drm_property_create_bool(drm_dev, 0, proto->name);
		break;
	case FTD330_DC_PROPERTY_ENUM:
		property = drm_property_create_enum(drm_dev, 0, proto->name,
						    proto->sub_proto.enum_list.data,
						    proto->sub_proto.enum_list.size);
		break;
	case FTD330_DC_PROPERTY_BITMASK:
		property = drm_property_create_bitmask(drm_dev, 0, proto->name,
						       proto->sub_proto.enum_list.data,
						       proto->sub_proto.enum_list.size,
						       proto->sub_proto.enum_list.supported_bits);
		break;
	case FTD330_DC_PROPERTY_RANGE:
		property = drm_property_create_range(drm_dev, 0, proto->name,
						     proto->sub_proto.range.min_val,
						     proto->sub_proto.range.max_val);
		break;
	default:
		break;
	}
	if (property)
		drm_object_attach_property(obj, property, proto->init_val);
	else
		pr_err("%s: Create property %s fail\n", __func__, proto->name);
	return property;
}

int ftd330_dc_create_drm_properties(struct drm_device *dev, struct drm_mode_object *obj,
				const struct ftd330_dc_property_state_group *dc_states,
				struct ftd330_drm_property_group *properties)
{
	u32 i;

	for (i = 0; i < dc_states->num; i++) {
		properties->items[i].data =
			ftd330_dc_create_drm_property(dev, obj, dc_states->items[i].proto);
		if (!properties->items[i].data)
			return -1;
		properties->num++;
		properties->items[i].proto = dc_states->items[i].proto;
	}
	return 0;
}

void ftd330_dc_duplicate_drm_properties(struct ftd330_drm_property_state *new_states,
				    const struct ftd330_drm_property_state *old_states,
				    const struct ftd330_drm_property_group *properties)
{
	u32 i;

	for (i = 0; i < properties->num; i++) {
		const struct ftd330_dc_property_proto *proto = properties->items[i].proto;

		if (!proto)
			continue;
		new_states[i] = old_states[i];
		new_states[i].is_changed = false;
		/* Increase blob reference count */
		if ((proto->type == FTD330_DC_PROPERTY_BLOB || proto->type == FTD330_DC_PROPERTY_ARRAY) &&
		    new_states[i].value.blob)
			drm_property_blob_get(new_states[i].value.blob);
	}
}

void ftd330_dc_destroy_drm_properties(struct ftd330_drm_property_state *states,
				  const struct ftd330_drm_property_group *properties)
{
	u32 i;

	for (i = 0; i < properties->num; i++) {
		/* Decrease blob reference count */
		const struct ftd330_dc_property_proto *proto = properties->items[i].proto;

		if (proto &&
		    (proto->type == FTD330_DC_PROPERTY_BLOB || proto->type == FTD330_DC_PROPERTY_ARRAY))
			drm_property_blob_put(states[i].value.blob);
	}
}

int ftd330_dc_set_drm_property(struct drm_device *dev, struct ftd330_drm_property_state *states,
			   const struct ftd330_drm_property_group *properties,
			   const struct drm_property *property, u64 val)
{
	int ret;
	u32 i;
	bool replaced = false;

	for (i = 0; i < properties->num; i++) {
		if (property == properties->items[i].data) {
			const struct ftd330_dc_property_proto *proto = properties->items[i].proto;
			struct ftd330_drm_property_state *state = &states[i];

			switch (proto->type) {
			case FTD330_DC_PROPERTY_BLOB:
				ret = _set_property_blob_from_id(dev, &state->value.blob, val,
								 proto->type_size, 0,
								 &replaced);
				state->is_changed |= replaced;
				return ret;
			case FTD330_DC_PROPERTY_ARRAY: {
				u32 type_size = proto->sub_proto.array.dynamic ? 0 :
								proto->type_size;

				ret = _set_property_blob_from_id(
					dev, &state->value.blob, val, type_size,
					proto->sub_proto.array.element_size, &replaced);
				state->is_changed |= replaced;
			}
				return ret;
			case FTD330_DC_PROPERTY_BOOL:
				state->value.boolean = !!val;
				state->is_changed = true;
				return 0;
			case FTD330_DC_PROPERTY_ENUM:
				state->value.enumval = (int)val;
				state->is_changed = true;
				return 0;
			case FTD330_DC_PROPERTY_BITMASK:
				state->value.u32val = (u32)val;
				state->is_changed = true;
				return 0;
			case FTD330_DC_PROPERTY_RANGE:
				state->value.u64val = val;
				state->is_changed = true;
				return 0;
			default:
				return -EINVAL;
			}
			break;
		}
	}
	return -EINVAL;
}

int ftd330_dc_get_drm_property(const struct ftd330_drm_property_state *states,
			   const struct ftd330_drm_property_group *properties,
			   const struct drm_property *property, u64 *val)
{
	u32 i;

	for (i = 0; i < properties->num; i++) {
		if (property == properties->items[i].data) {
			const struct ftd330_dc_property_proto *proto = properties->items[i].proto;
			const struct ftd330_drm_property_state *state = &states[i];

			switch (proto->type) {
			case FTD330_DC_PROPERTY_BLOB:
			case FTD330_DC_PROPERTY_ARRAY:
				*val = state->value.blob ? state->value.blob->base.id : 0;
				return 0;
			case FTD330_DC_PROPERTY_BOOL:
				*val = state->value.boolean;
				return 0;
			case FTD330_DC_PROPERTY_ENUM:
				*val = state->value.enumval;
				return 0;
			case FTD330_DC_PROPERTY_BITMASK:
				*val = state->value.u32val;
				return 0;
			case FTD330_DC_PROPERTY_RANGE:
				*val = state->value.u64val;
				return 0;
			default:
				pr_err("%s: Unsupport type %#x\n", __func__, proto->type);
				return -EINVAL;
			}
			break;
		}
	}
	return -EINVAL;
}

void ftd330_dc_update_drm_properties_to_dc(struct ftd330_dc *dc, u8 hw_id,
				       const struct ftd330_drm_property_state *drm_states,
				       u32 registered_drm_properties_num,
				       struct ftd330_dc_property_state_group *dc_states,
				       const void *obj_state)
{
	u32 i;

	for (i = 0; i < registered_drm_properties_num; i++) {
		const struct ftd330_drm_property_state *state = &drm_states[i];
		struct ftd330_dc_property_state *hw_state;
		const void *new_data = NULL;
		const struct ftd330_dc_property_proto *proto = dc_states->items[i].proto;
		u32 size = 0;

		if (!state->is_changed)
			continue;
		if (!proto) {
			pr_err("%s: Unknown error, proto is NULL\n", __func__);
			continue;
		}


		hw_state = &dc_states->items[i];
		switch (proto->type) {
		case FTD330_DC_PROPERTY_BLOB:
			if (state->value.blob) {
				new_data = state->value.blob->data;
				size = state->value.blob->length;
			}
			ftd330_dc_blob_property_update(&dc->hw, hw_id, hw_state, new_data, size,
						   obj_state);
			break;
		case FTD330_DC_PROPERTY_ARRAY:
			if (state->value.blob) {
				new_data = state->value.blob->data;
				size = state->value.blob->length;
			}
			ftd330_dc_array_property_update(&dc->hw, hw_id, hw_state, new_data, size,
						    obj_state);
			break;
		case FTD330_DC_PROPERTY_BOOL:
			ftd330_dc_bool_property_update(&dc->hw, hw_id, hw_state, state->value.boolean,
						   obj_state);
			break;
		case FTD330_DC_PROPERTY_ENUM:
			ftd330_dc_enum_property_update(&dc->hw, hw_id, hw_state, state->value.enumval,
						   obj_state);
			break;
		case FTD330_DC_PROPERTY_BITMASK:
			ftd330_dc_bitmask_property_update(&dc->hw, hw_id, hw_state, state->value.u32val,
						      obj_state);
			break;
		case FTD330_DC_PROPERTY_RANGE:
			ftd330_dc_range_property_update(&dc->hw, hw_id, hw_state, state->value.u64val,
						    obj_state);
			break;
		default:
			pr_err("%s: Unsupport propety type %#x\n", __func__, proto->type);
			break;
		}
	}
}

struct ftd330_drm_property_state *ftd330_dc_get_drm_property_state(struct ftd330_drm_property_state *states,
							   u32 num, const char *name)
{
	u32 i;

	if (!name) {
		pr_err("%s: name is null\n", __func__);
		return NULL;
	}

	for (i = 0; i < num; i++) {
		if (!states[i].proto)
			break;
		if (strcmp(states[i].proto->name, name) == 0)
			return &states[i];
	}
	pr_err("%s: Drm propety \"%s\" not found\n", __func__, name);
	return NULL;
}

bool ftd330_dc_check_crtc_std_property(struct ftd330_dc *dc, u8 hw_id, struct drm_crtc *crtc)
{
	struct drm_crtc_state *state = crtc->state;
	const struct dc_hw_display *hw_display = ftd330_dc_hw_get_display(&dc->hw, hw_id);
	const struct ftd330_display_info *display_info = hw_display->info;

	/* check ctm */
	if (state->ctm) {
		if (!display_info->ccm_linear) {
			pr_err("%s The display is not support set ctm.\n", __func__);
			return false;
		}
		return true;
	}
	return true;
}

bool ftd330_dc_check_drm_property(struct ftd330_dc *dc, u8 hw_id,
			      const struct ftd330_drm_property_state *states, u32 num,
			      const void *obj_state)
{
	u32 i;
	bool ret = true;
	const struct ftd330_drm_property_state *state;
	const void *data = NULL;
	u32 size = 0;

	for (i = 0; i < num; i++) {
		state = &states[i];
		if (!state->proto)
			break;
		if (!state->proto->check)
			continue;
		if (!state->is_changed)
			continue;
		switch (state->proto->type) {
		case FTD330_DC_PROPERTY_BLOB:
		case FTD330_DC_PROPERTY_ARRAY:
			/* Always allow pass null to disable the features. */
			if (!state->value.blob) {
				continue;
			} else {
				data = state->value.blob->data;
				size = state->value.blob->length;
			}
			break;
		case FTD330_DC_PROPERTY_BOOL:
			data = &state->value.boolean;
			size = sizeof(bool);
			break;
		case FTD330_DC_PROPERTY_ENUM:
			data = &state->value.enumval;
			size = sizeof(int);
			break;
		case FTD330_DC_PROPERTY_BITMASK:
			data = &state->value.u32val;
			size = sizeof(u32);
			break;
		case FTD330_DC_PROPERTY_RANGE:
			data = &state->value.u64val;
			size = sizeof(u64);
			break;
		default:
			pr_err("%s: Unsupport propety type %#x\n", __func__, state->proto->type);
			break;
		}
		ret = state->proto->check(&dc->hw, hw_id, data, size, obj_state);
		if (!ret)
			break;
	}
	return ret;
}

static const void *__get_drm_property(const struct ftd330_drm_property_state *states, u32 num,
				      const char *name, u32 *out_len)
{
	u32 i;
	const struct ftd330_drm_property_state *state = NULL;
	const void *ptr = NULL;
	u32 size = 0;

	if (!name) {
		pr_err("%s: property name is NULL!\n", __func__);
		return NULL;
	}
	for (i = 0; i < num; i++) {
		state = &states[i];
		if (!state->proto)
			break;
		if (strcmp(name, state->proto->name) == 0) {
			switch (state->proto->type) {
			case FTD330_DC_PROPERTY_BLOB:
			case FTD330_DC_PROPERTY_ARRAY:
				if (!state->value.blob) {
					size = 0;
					ptr = NULL;
				} else {
					size = state->value.blob->length;
					ptr = state->value.blob->data;
				}
				break;
			case FTD330_DC_PROPERTY_BOOL:
				size = sizeof(bool);
				ptr = &state->value.boolean;
				break;
			case FTD330_DC_PROPERTY_ENUM:
				size = sizeof(int);
				ptr = &state->value.enumval;
				break;
			case FTD330_DC_PROPERTY_BITMASK:
				size = sizeof(u32);
				ptr = &state->value.u32val;
				break;
			case FTD330_DC_PROPERTY_RANGE:
				size = sizeof(u64);
				ptr = &state->value.u64val;
				break;
			default:
				pr_err("%s: Unsupport propety type %#x\n", __func__,
				       state->proto->type);
				break;
			}
		}
	}
	if (out_len)
		*out_len = size;
	if (i == num)
		pr_err("%s: property %s not found!\n", __func__, name);
	return ptr;
}

const void *ftd330_dc_drm_plane_property_get(const struct ftd330_plane_state *plane, const char *name,
					 u32 *out_len)
{
	return __get_drm_property(plane->drm_states, FTD330_DC_MAX_PROPERTY_NUM, name, out_len);
}

const void *ftd330_dc_drm_crtc_property_get(const struct ftd330_crtc_state *crtc, const char *name,
					u32 *out_len)
{
	return __get_drm_property(crtc->drm_states, FTD330_DC_MAX_PROPERTY_NUM, name, out_len);
}

#ifdef CONFIG_PHYTIUM_WRITEBACK
const void *ftd330_dc_drm_connector_property_get(const struct ftd330_writeback_connector_state *wb,
					const char *name, u32 *out_len)
{
	return __get_drm_property(wb->drm_states, FTD330_DC_MAX_PROPERTY_NUM, name, out_len);
}
#endif
