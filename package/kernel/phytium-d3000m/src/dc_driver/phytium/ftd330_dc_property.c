// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_property.h"

#include <linux/kernel.h>
#include "ftd330_dc_hw.h"

static void *get_buffer(struct ftd330_dc_property_state_mem *mem, u32 size)
{
	u32 used_size = mem->used_size;
	u32 new_size = mem->used_size + ALIGN(size, 4);

	if (new_size > mem->total_size)
		return NULL;
	mem->used_size = new_size;
	return &mem->pool[used_size];
}

bool ftd330_dc_initialize_property_states(struct ftd330_dc_property_state_group *states)
{
	const struct ftd330_dc_property_proto *proto = NULL;
	struct ftd330_dc_property_state *item = NULL;
	size_t req_mem = 0;
	u32 i = 0;

	if (!states)
		return true;
	for (i = 0; i < states->num; i++)
		req_mem += ALIGN(states->items[i].proto->type_size, 4);
	if (req_mem > FTD330_DC_MAX_PROPERTY_STATE_MEM_MAX_SIZE) {
		pr_err("%s: Request mem over the max mem size, %#lx > %#x\n", __func__, req_mem,
		       FTD330_DC_MAX_PROPERTY_STATE_MEM_MAX_SIZE);
		return false;
	}
	/* TODO: Fixme, use dynamic mem
	 *states->mem.pool = kzalloc(req_mem, GFP_KERNEL);
	 *if (!states->mem.pool)
	 *	return false;
	 */
	states->mem.total_size = req_mem;

	for (i = 0; i < states->num; i++) {
		item = &states->items[i];
		proto = item->proto;
		switch (proto->type) {
		case FTD330_DC_PROPERTY_BLOB:
		case FTD330_DC_PROPERTY_ARRAY:
		case FTD330_DC_PROPERTY_ENUM:
		case FTD330_DC_PROPERTY_RANGE:
		case FTD330_DC_PROPERTY_BITMASK:
			item->data = get_buffer(&states->mem, proto->type_size);
			break;
		case FTD330_DC_PROPERTY_BOOL:
			item->data = &item->enable;
			break;
		default:
			item->data = NULL;
			pr_err("%s: Unsupport property type %#x\n", __func__, proto->type);
			goto err_cleanup;
		}
		if (!item->data) {
			pr_err("%s: No more state mem for %s\n", __func__, proto->name);
			goto err_cleanup;
		}
		item->valid = true;
	}
	return true;
err_cleanup:
	ftd330_dc_deinitialize_property_states(states);
	return false;
}

void ftd330_dc_deinitialize_property_states(struct ftd330_dc_property_state_group *states)
{
	if (!states || !states->mem.pool)
		return;
	/* TODO: Fixme
	 *kfree(states->mem.pool);
	 */
	memset(states, 0, sizeof(*states));
}

bool ftd330_dc_property_register_state(struct ftd330_dc_property_state_group *states,
				   const struct ftd330_dc_property_proto *proto)
{
	struct ftd330_dc_property_state *item;

	if (states->num == FTD330_DC_MAX_PROPERTY_NUM) {
		pr_err("%s: No more empty state for %s\n", __func__, proto->name);
		return false;
	}
	item = &states->items[states->num];
	item->enable = false;
	item->dirty = false;
	item->valid = false;
	item->proto = proto;
	states->num++;
	return true;
}

bool ftd330_dc_property_config_hw(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state)
{
	bool ret;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}

	ret = state->proto->config_hw(hw, hw_id, state->enable, state->data);
	if (ret)
		state->dirty = false;
	return ret;
}

bool ftd330_dc_blob_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				const void *new_data, u32 size, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, new_data, size, obj_state);
	} else if (!new_data) {
		/* enable -> disable, set dirty */
		state->dirty = state->enable;
		state->enable = false;
	} else {
		state->dirty = true;
		memcpy(state->data, new_data, state->proto->type_size);
		state->enable = true;
	}

	return ret;
}

bool ftd330_dc_bool_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				bool enable, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, &enable, sizeof(bool), obj_state);
	} else {
		state->dirty = true;
		state->enable = enable;
	}
	/* bool property state data is pointer to state enable flag
	 * so there is no need to update state->data.
	 */
	return ret;
}

bool ftd330_dc_enum_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				int val, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, &val, sizeof(int), obj_state);
	} else {
		state->dirty = true;
		*(int *)state->data = val;
		state->enable = true;
	}
	return ret;
}

bool ftd330_dc_bitmask_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				   u32 val, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, &val, sizeof(u32), obj_state);
	} else {
		state->dirty = true;
		*(u32 *)state->data = val;
		state->enable = true;
	}
	return ret;
}

bool ftd330_dc_range_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				 u64 val, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, &val, sizeof(u64), obj_state);
	} else {
		state->dirty = true;
		*(u64 *)state->data = val;
		state->enable = true;
	}
	return ret;
}

bool ftd330_dc_array_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				 const void *new_data, u32 size, const void *obj_state)
{
	bool ret = true;

	if (!state->valid) {
		pr_err("%s: property %s is not valid!\n", __func__, state->proto->name);
		return false;
	}
	if (state->proto->update != NULL) {
		ret = state->proto->update(hw, hw_id, state, new_data, size, obj_state);
	} else if (!new_data) {
		/* enable -> disable, set dirty */
		state->dirty = state->enable;
		state->enable = false;
	} else {
		state->dirty = true;
		memcpy(state->data, new_data, state->proto->type_size);
		state->enable = true;
	}
	return ret;
}

const void *ftd330_dc_property_get_by_name(const struct ftd330_dc_property_state_group *states,
				       const char *name, bool *out_enabled)
{
	u32 i;

	if (!name) {
		pr_err("%s: property name is NULL!\n", __func__);
		return NULL;
	}
	for (i = 0; i < states->num; i++) {
		if (strcmp(name, states->items[i].proto->name) == 0) {
			if (out_enabled)
				*out_enabled = states->items[i].enable;
			return states->items[i].data;
		}
	}
	pr_err("%s: property %s not found!\n", __func__, name);
	return NULL;
}
