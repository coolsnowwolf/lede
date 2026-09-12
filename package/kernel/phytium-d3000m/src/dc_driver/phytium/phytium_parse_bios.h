/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PARSE_BIOS_H__
#define __PHYTIUM_PARSE_BIOS_H__

#define PARASE_DEBUG 0
#define SUPPORT_MAJOR_VERSION 1
#define SUPPORT_MINOR_VERSION 0
/* Capability ID*/
#define DP_PHY_MGNFS 0x1
#define DP_PHY_CPOST 0x2
#define EDP_BACKLIGHT_TABLE 0x3
#define EDP_PANEL_TIMING 0x4
#define PANEL_INFO 0x5

#define INVALID_ADDR 0xFFFFFFFF

#define PORT_MASK 0xF0
#define LANE_MASK 0x0F

struct para_header {
	char signature[9];
	uint32_t total_length;
	int major_version;
	int minor_version;
	uint32_t capability_count;
	uint32_t capability_offset;
};

int parse_bios_para_table(struct bios_table_info *bios_info);
void print_all_parameter(const struct dp_phy_matrix *data,
						const struct edp_backlight_map *edp_bl_map,
						const struct edp_panel_timing *edp_panel_time);
#endif /* __PHYTIUM_PARSE_BIOS_H__ */
