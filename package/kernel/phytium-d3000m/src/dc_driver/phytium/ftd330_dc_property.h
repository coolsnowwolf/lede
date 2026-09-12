/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_PROPERTY_H__
#define __FTD330_DC_PROPERTY_H__

#include <linux/version.h>
#include <linux/types.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
#include <drm/drm_mode.h>
#endif
#include <drm/drm_property.h>

#define FTD330_DC_MAX_PROPERTY_NUM 64
#define FTD330_DC_MAX_PROPERTY_STATE_MEM_MAX_SIZE (128 * 2048)

#define FTD330_DC_BLOB_PROPERTY_PROTO(protoname, strname, state_type, check_func, update_func,      \
				  config_func)                                                  \
	static const struct ftd330_dc_property_proto protoname = { .name = strname,                 \
							       .type = FTD330_DC_PROPERTY_BLOB,     \
							       .type_size = sizeof(state_type), \
							       .init_val = 0,                   \
							       .check = check_func,             \
							       .update = update_func,           \
							       .config_hw = config_func }

#define FTD330_DC_ARRAY_PROPERTY_PROTO(protoname, strname, element_type, max_count, dynamic, \
				   check_func, update_func, config_func)                 \
	static const struct ftd330_dc_property_proto protoname = {                           \
		.name = strname,                                                         \
		.type = FTD330_DC_PROPERTY_ARRAY,                                            \
		.type_size = max_count * sizeof(element_type),                           \
		.init_val = 0,                                                           \
		.sub_proto = { .array = { max_count, dynamic, sizeof(element_type) } },  \
		.check = check_func,                                                     \
		.update = update_func,                                                   \
		.config_hw = config_func                                                 \
	}

#define FTD330_DC_BOOL_PROPERTY_PROTO(protoname, strname, check_func, update_func, config_func) \
	static const struct ftd330_dc_property_proto protoname = { .name = strname,             \
							       .type = FTD330_DC_PROPERTY_BOOL, \
							       .type_size = sizeof(bool),   \
							       .init_val = 0,               \
							       .check = check_func,         \
							       .update = update_func,       \
							       .config_hw = config_func }

#define FTD330_DC_ENUM_PROPERTY_PROTO(protoname, strname, _enum_list, _init_val, check_func, \
				  update_func, config_func)                              \
	static const struct ftd330_dc_property_proto protoname = {                           \
		.name = strname,                                                         \
		.type = FTD330_DC_PROPERTY_ENUM,                                             \
		.type_size = sizeof(int),                                                \
		.init_val = _init_val,                                                   \
		.sub_proto = { .enum_list = { _enum_list, ARRAY_SIZE(_enum_list) } },    \
		.check = check_func,                                                     \
		.update = update_func,                                                   \
		.config_hw = config_func                                                 \
	}

/* due to @supported_bits may depend on hw info, the const attribute of
 * bitmask property is removed
 */
#define FTD330_DC_BITMASK_PROPERTY_PROTO(protoname, strname, _enum_list, supported_bits, check_func, \
				     update_func, config_func)                                   \
	static struct ftd330_dc_property_proto protoname = {                                         \
		.name = strname,                                                                 \
		.type = FTD330_DC_PROPERTY_BITMASK,                                                  \
		.type_size = sizeof(unsigned int),                                               \
		.init_val = 0,                                                                   \
		.sub_proto = { .enum_list = { _enum_list, ARRAY_SIZE(_enum_list),                \
					      supported_bits } },                                \
		.check = check_func,                                                             \
		.update = update_func,                                                           \
		.config_hw = config_func                                                         \
	}

#define FTD330_DC_RANGE_PROPERTY_PROTO(protoname, strname, min_val, max_val, check_func, update_func, \
				   config_func)                                                   \
	static const struct ftd330_dc_property_proto protoname = {                                    \
		.name = strname,                                                                  \
		.type = FTD330_DC_PROPERTY_RANGE,                                                     \
		.type_size = sizeof(u64),                                                         \
		.init_val = 0,                                                                    \
		.sub_proto = { .range = { min_val, max_val } },                                   \
		.check = check_func,                                                              \
		.update = update_func,                                                            \
		.config_hw = config_func                                                          \
	}

#define FTD330_DC_PROPERTY_VAL(data, type) (*((const type *)(data)))

#ifndef __ERR_CHECK
#define __ERR_CHECK(cond, lable)    \
	do {                        \
		if (!(cond))        \
			goto lable; \
	} while (0)
#endif

struct dc_hw;
struct ftd330_dc_property_state;

typedef enum _ftd330_dc_property_type {
	FTD330_DC_PROPERTY_BOOL,
	FTD330_DC_PROPERTY_ENUM,
	FTD330_DC_PROPERTY_RANGE,
	FTD330_DC_PROPERTY_BLOB,
	FTD330_DC_PROPERTY_ARRAY,
	FTD330_DC_PROPERTY_BITMASK,
} ftd330_dc_property_type;

union ftd330_dc_property_sub_proto {
	struct {
		const struct drm_prop_enum_list *data;
		u32 size;
		u32 supported_bits;
	} enum_list;
	struct {
		u64 min_val;
		u64 max_val;
	} range;
	struct {
		u32 max_count;
		bool dynamic;
		u32 element_size;
	} array;
};

struct ftd330_dc_property_proto {
	const char *name;
	ftd330_dc_property_type type;
	u32 type_size;
	u64 init_val;
	union ftd330_dc_property_sub_proto sub_proto;
	bool (*check)(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
		      const void *obj_state);
	bool (*update)(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
		       const void *data, u32 size, const void *obj_state);
	bool (*config_hw)(struct dc_hw *hw, u8 hw_id, bool enable, const void *data);
};

struct ftd330_dc_property_state_mem {
	u8 pool[FTD330_DC_MAX_PROPERTY_STATE_MEM_MAX_SIZE];
	size_t used_size;
	size_t total_size;
};

struct ftd330_dc_property_state {
	const struct ftd330_dc_property_proto *proto;
	bool enable;
	bool dirty;
	bool valid;
	void *data;
};

struct ftd330_dc_property_state_group {
	u32 num;
	struct ftd330_dc_property_state items[FTD330_DC_MAX_PROPERTY_NUM];
	struct ftd330_dc_property_state_mem mem;
};

bool ftd330_dc_property_register_state(struct ftd330_dc_property_state_group *states,
				   const struct ftd330_dc_property_proto *proto);
bool ftd330_dc_blob_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				const void *new_data, u32 size, const void *obj_state);
bool ftd330_dc_bool_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				bool enable, const void *obj_state);
bool ftd330_dc_enum_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				int val, const void *obj_state);
bool ftd330_dc_bitmask_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				   u32 val, const void *obj_state);
bool ftd330_dc_range_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				 u64 val, const void *obj_state);
bool ftd330_dc_array_property_update(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state,
				 const void *new_data, u32 size, const void *obj_state);
bool ftd330_dc_property_config_hw(struct dc_hw *hw, u8 hw_id, struct ftd330_dc_property_state *state);
const void *ftd330_dc_property_get_by_name(const struct ftd330_dc_property_state_group *states,
				       const char *name, bool *out_enabled);
bool ftd330_dc_initialize_property_states(struct ftd330_dc_property_state_group *states);
void ftd330_dc_deinitialize_property_states(struct ftd330_dc_property_state_group *states);

#endif
