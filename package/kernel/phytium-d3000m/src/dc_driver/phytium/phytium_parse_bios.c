// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#include <linux/io.h>
#include "phytium_dp.h"
#include "phytium_parse_bios.h"
#include "FTD330/ftd330_dc.h"
#include "ftd330_dp.h"

static void print_phy_matrix(const int matrix[4][4][4], const char *name) {
	int i, j;

	pr_info("%s:\n", name);
	for (i = 0; i < 4; i++) {
		pr_info("link rate index %d:\n", i);
		for (j = 0; j < 4; j++) {
			pr_info("0x%x 0x%x 0x%x 0x%x\n",
					matrix[i][j][0], matrix[i][j][1],
					matrix[i][j][2], matrix[i][j][3]);
		}
	}
	pr_info("\n");
}

static void print_edp_bl_map(const struct edp_backlight_map *map) {
	int i;
	char buf[256];
	int len = 0;

	pr_info("edp backlight map:\n");

	pr_info("edp bright to pwm mapping table\n");
	for (i = 0; i < 101; i++) {
		if (i % 10 == 0)
			len = 0;

		len += snprintf(buf + len, sizeof(buf) - len, "%d ",
								map->edp_bright_to_pwm[i]);

		if ((i + 1) % 10 == 0) {
			pr_info("%s\n", buf);
		}
	}
	pr_info("edp pwm to bright mapping table\n");
	for (i = 0; i < 101; i++) {
		if (i % 10 == 0)
			len = 0;

		len += snprintf(buf + len, sizeof(buf) - len, "%d ",
								map->edp_pwm_to_bright[i]);

		if ((i + 1) % 10 == 0) {
			pr_info("%s\n", buf);
		}
	}
}

static void print_edp_panel_timing(const struct edp_panel_timing *time) {

	pr_info("edp panel timing:\n");
	pr_info("panel power up delay is %d ms", time->panel_power_up_delay);
	pr_info("backlight on delay is %d ms", time->backlight_on_delay);
	pr_info("backlight off delay is %d ms", time->backlight_off_delay);
	pr_info("panel power down delay is %d ms", time->panel_power_down_delay);
	pr_info("panel power cycle delay is %d ms", time->panel_power_cycle_delay);

}

void print_all_parameter(const struct dp_phy_matrix *data,
								const struct edp_backlight_map *edp_bl_map,
								const struct edp_panel_timing *edp_panel_time
								) {
	print_phy_matrix(data->dp_mgnfs_val, "dp_mgnfs_val");
	print_phy_matrix(data->dp_cpost_val, "dp_cpost_val");

	print_edp_bl_map(edp_bl_map);

	print_edp_panel_timing(edp_panel_time);
}

static int parse_bios_para_header(void __iomem *base,
							struct para_header *header,
							bool *next_content_valid) {
	int i;
	char sig[9];

	for (i = 0; i < 8; i++) {
		sig[i] = readb(base + i);
	}
	sig[8] = '\0';

	if (memcmp(sig, "DCDPPART", 8) != 0) {
		pr_err("FTD330: header signature %s is illegal\n", sig);
		return -EINVAL;
	}

	memcpy(header->signature, sig, 9);

	header->total_length = readl(base + 8);

	header->major_version = readw(base + 12);

	header->minor_version = readw(base + 14);

	header->capability_count = readl(base + 16);

	header->capability_offset = readl(base + 20);
	if (header->capability_offset == INVALID_ADDR)
		*next_content_valid = false;

	pr_info("table info:\n"
			"total_length: %d\n"
			"version: %d.%d\n"
			"capability_count: %d\n"
			"capability_offset  0x%x\n",
			header->total_length, header->major_version,
			header->minor_version, header->capability_count,
			header->capability_offset);

	if (header->major_version > SUPPORT_MAJOR_VERSION
		|| header->minor_version > SUPPORT_MINOR_VERSION) {
		pr_info("Unsupport bios version %d.%d, only support version <= %d.%d\n",
				header->major_version, header->minor_version,
				SUPPORT_MAJOR_VERSION, SUPPORT_MINOR_VERSION
				);
		return -EINVAL;
	}

	return 0;
}

static int parse_phy_matrix_content(void __iomem *start, u32 id,
									struct bios_table_info *bios_info,
									struct para_header *header,
									bool *next_content_valid) {

	int ret = 0;
	u32 phy_matrix_count;
	int i, ch, lr, rate_idx;
	u32 base_offset = 4;

	phy_matrix_count = readl(start);

#if PARASE_DEBUG
	pr_info("phy_matrix_count is %d\n", phy_matrix_count);
#endif
	// 遍历每个matrix Struct
	for (i = 0; i < phy_matrix_count; i++) {
		u8 port_lane;
		u8 port;
		u8 lane_id;
		u8 link_rate;
		u16 phy_matrix[4][4];
		void __iomem *entry_base = start + base_offset + i * 34; // 每个结构34字节

		// 读取Channel 和 lane ID，1字节偏移0
		port_lane = readb(entry_base + 0);
		port = (port_lane & PORT_MASK) >> 4;
		lane_id = port_lane & LANE_MASK;

		if (port >= 4) {
			pr_warn("Invalid port %d, skip\n", port);
			ret = -EINVAL;
			goto out;
		}

		if (lane_id >= 4) {
			pr_warn("Invalid lane id %d, skip\n", lane_id);
			ret = -EINVAL;
			goto out;
		}

		/* 目前只支持解析lane0的参数，其它lane参数默认与lane0相同 */
		if (lane_id == 0) {
			// 读取Link Rate，1字节偏移1
			link_rate = readb(entry_base + 1);

			// 读取Mgnfs Table 4x4，每个元素2字节，从偏移2开始
			for (ch = 0; ch < 4; ch++) {
				for (lr = 0; lr < 4; lr++) {
					u16 val = readw(entry_base + 2 + (ch * 4 + lr) * 2);
					phy_matrix[ch][lr] = val;
				}
			}

			switch (link_rate) {
				case 0x1e:
					rate_idx = 3;
					break;
				case 0x14:
					rate_idx = 2;
					break;
				case 0xa:
					rate_idx = 1;
					break;
				case 0x6:
					rate_idx = 0;
					break;
				default:
					pr_warn("Unknown link rate %u, abort\n", link_rate);
					return -EINVAL;
			}

			if (id == DP_PHY_MGNFS) {
				switch (port) {
				case 0:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp0_bios_mgnfs_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp0_bios_mgnfs_valied = true;
					break;
				case 1:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp1_bios_mgnfs_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp1_bios_mgnfs_valied = true;
					break;
				case 2:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp2_bios_mgnfs_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp2_bios_mgnfs_valied = true;
					break;
				default:
					pr_warn("Invalid DP PHY port %d\n", port);
					ret = -EINVAL;
					goto out;
				}
			} else if (id == DP_PHY_CPOST) {
				switch (port) {
				case 0:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp0_bios_cpost_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp0_bios_cpost_valied = true;
					break;
				case 1:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp1_bios_cpost_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp1_bios_cpost_valied = true;
					break;
				case 2:
					for (ch = 0; ch < 4; ch++)
						for (lr = 0; lr < 4; lr++)
							bios_info->dp_bios_phy_val.dp2_bios_cpost_val[rate_idx][ch][lr] = phy_matrix[ch][lr];
					bios_info->dp2_bios_cpost_valied = true;
					break;
				default:
					pr_warn("Invalid DP PHY port %d\n", port);
					ret = -EINVAL;
					goto out;
				}
			}
		} else {
			continue;
		}
	}

	return ret;
out:
	pr_info("Failed to parse content id: 0x%x\n", id);
	return ret;

}

static int parse_bios_backlight_content(void __iomem *start, u32 id,
									struct bios_table_info *bios_info,
									struct para_header *header,
									bool *next_content_valid) {
	int ret = 0;
	u32 table_count;
	int i, j;
	u32 base_offset = 4;

	table_count = readl(start);
	// 遍历每个table Struct
	for (i = 0; i < table_count; i++) {
		u8 port;
		void __iomem *entry_base = start + base_offset + i * 204; // 每个结构204字节

		// 读取port，2字节偏移0
		port = readw(entry_base + 0);
		if (port >= 4) {
			pr_warn("Invalid port %d, skip\n", port);
			ret = -EINVAL;
			goto out;
		}

		// 读取bl Table 101x2，每个元素1字节，从偏移2开始
		for (j = 0; j < 101; j++)
			bios_info->edp_bios_bl_map.edp_bright_to_pwm[port][j] = readb(entry_base + 2 + j);
		for (j = 101; j < 202; j++)
			bios_info->edp_bios_bl_map.edp_pwm_to_bright[port][j-101] = readb(entry_base + 2 + j);

		switch (port) {
		case 0:
			bios_info->edp0_bios_bl_map_valied = true;
			break;
		case 1:
			bios_info->edp1_bios_bl_map_valied = true;
			break;
		case 2:
			bios_info->edp2_bios_bl_map_valied = true;
			break;
		default:
			pr_warn("Invalid DP PHY port %d\n", port);
			ret = -EINVAL;
			goto out;
		}
	}
	return ret;
out:
	pr_info("Failed to parse content id: 0x%x\n", id);
	return ret;
}

static int parse_bios_panel_timing_content(void __iomem *start, u32 id,
									struct bios_table_info *bios_info,
									struct para_header *header,
									bool *next_content_valid) {
	int ret = 0;
	u32 table_count;
	int i;
	u32 base_offset = 4;

	table_count = readl(start);

	// 遍历每个table Struct
	for (i = 0; i < table_count; i++) {
		u8 port;
		void __iomem *entry_base = start + base_offset + i * 20; // 每个结构20字节

		// 读取port，4字节偏移0
		port = readl(entry_base + 0);
		if (port >= 4) {
			pr_warn("Invalid port %d, skip\n", port);
			ret = -EINVAL;
			goto out;
		}

		// 读取panel Table，每个元素4字节，从偏移4开始
		bios_info->edp_bios_panel_time.panel_power_up_delay[port][0] = readl(entry_base + 4);
		bios_info->edp_bios_panel_time.backlight_on_delay[port][0] = readl(entry_base + 8);
		bios_info->edp_bios_panel_time.backlight_off_delay[port][0] = readl(entry_base + 12);
		bios_info->edp_bios_panel_time.panel_power_down_delay[port][0] = readl(entry_base + 16);
		bios_info->edp_bios_panel_time.panel_power_cycle_delay[port][0] = readl(entry_base + 20);

		switch (port) {
		case 0:
			bios_info->edp0_bios_panel_time_valied = true;
			break;
		case 1:
			bios_info->edp1_bios_panel_time_valied = true;
			break;
		case 2:
			bios_info->edp2_bios_panel_time_valied = true;
			break;
		default:
			pr_warn("Invalid DP PHY port %d\n", port);
			ret = -EINVAL;
			goto out;
		}
	}
	return ret;
out:
	pr_info("Failed to parse content id: 0x%x\n", id);
	return ret;
}

static int parse_bios_panel_info_content(void __iomem *start, u32 id,
									struct bios_table_info *bios_info,
									struct para_header *header,
									bool *next_content_valid) {
	int ret = 0;
	u32 table_count;
	int i;
	u32 base_offset = 4;

	table_count = readl(start);
	if (table_count == 0) {
		pr_info("ID 0x%x table count is 0\n", id);
		return ret;
	}

	bios_info->panel_count = table_count;

	if (bios_info->panels) {
		kfree(bios_info->panels);
		bios_info->panels = NULL;
	}

	bios_info->panels = kzalloc(sizeof(struct bios_panel_info) * table_count, GFP_KERNEL);
	if (!bios_info->panels) {
		pr_err("Failed to allocate memory for bios_info->panels\n");
		return -ENOMEM;
	}

	for (i = 0; i < table_count; i++) {
		u16 mfg_id;
		u16 prod_code;
		
		void __iomem *entry_base = start + base_offset + i * 8; // 每个结构8字节

		// 读取edid信息
		mfg_id = readw(entry_base);
		pr_info("mfg_id is 0x%x\n", mfg_id);
		prod_code = readw(entry_base + 2);
		pr_info("prod_code is 0x%x\n", prod_code);
		bios_info->panels[i].panel_id = (mfg_id & 0xFF) << 24 |
										((mfg_id >> 8) & 0xFF) << 16 |
										((prod_code >> 8) & 0xFF) << 8 |
										(prod_code & 0xFF);

		// 读取最大速率和lane数
		bios_info->panels[i].max_lane_count = readb(entry_base + 4);
		bios_info->panels[i].max_link_rate = readb(entry_base + 5);

#if PARASE_DEBUG
	pr_info("Parsing Capability ID: 0x%x\n", id);
	pr_info("bios_info->panels[%d].panel_id = 0x%x, max_lane_count = %d, max_link_rate = 0x%x\n",
			i, bios_info->panels[i].panel_id,
			bios_info->panels[i].max_lane_count, bios_info->panels[i].max_link_rate);
#endif
		switch (bios_info->panels[i].max_link_rate) {
			case 0x1e:
				bios_info->panels[i].num_link_rate = 4;
				break;
			case 0x14:
				bios_info->panels[i].num_link_rate = 3;
				break;
			case 0xa:
				bios_info->panels[i].num_link_rate = 2;
				break;
			case 0x6:
				bios_info->panels[i].num_link_rate = 1;
				break;
			default:
				pr_warn("Unknown link rate %u, abort\n",
						bios_info->panels[i].max_link_rate);
				return -EINVAL;
		}
		switch (bios_info->panels[i].max_lane_count)
		{
		case 1:
		case 2:
		case 4:
			break;
		default:
			pr_warn("Unsupport lane count %d, abort\n",
						bios_info->panels[i].max_lane_count);
				return -EINVAL;
		}

		bios_info->panels[i].valid = true;
	}
	return ret;
}

static int parse_bios_para_content(void __iomem *start,
									struct bios_table_info *bios_info,
									struct para_header *header,
									bool *next_content_valid) {
	int ret = 0;
	u32 id;
	void __iomem *content_start;

	if (!start || !bios_info || !header)
		return -EINVAL;
	
	/* parse content head */
	id = readl(start);
	header->capability_offset = readl(start + 8);
#if PARASE_DEBUG
	pr_info("Parsing Capability ID: 0x%x\n", id);
	pr_info("%s: capability_offset  0x%x\n",
			__func__, header->capability_offset);
#endif

	switch (id) {
		case DP_PHY_MGNFS:
		case DP_PHY_CPOST:
			content_start = start + 12;
			return parse_phy_matrix_content(content_start, id, bios_info,
											header, next_content_valid);
		case EDP_BACKLIGHT_TABLE:
			content_start = start + 12;
			return parse_bios_backlight_content(content_start, id, bios_info,
											header, next_content_valid);
		case EDP_PANEL_TIMING:
			content_start = start + 12;
			return parse_bios_panel_timing_content(content_start, id, bios_info,
											header, next_content_valid);
		case PANEL_INFO:
			content_start = start + 12;
			return parse_bios_panel_info_content(content_start, id, bios_info,
											header, next_content_valid);
		default:
			pr_err("Unsupported Capability ID: 0x%x\n", id);
			return -EINVAL;
		}

	return ret;
}

int parse_bios_para_table(struct bios_table_info *bios_info) {
	int i, ret = 0;
	void __iomem *base = NULL;
	void __iomem *next_content_start = NULL;
	struct para_header header;
	bool next_content_valid = true;
	
	base = bios_info->para_table_base;
	ret = parse_bios_para_header(base, &header, &next_content_valid);
	if (ret) {
		pr_info("FTD330 failed to parase bios parameter table header\n");
		return -EINVAL;
	}
	if (!next_content_valid)
		goto out;

	for (i = 0; i < header.capability_count; i++) {
		if (!next_content_valid)
			goto out;
		next_content_start = base + header.capability_offset;
		ret = parse_bios_para_content(next_content_start,
										bios_info, &header,
										&next_content_valid);
	}
out:
	return ret;
}
