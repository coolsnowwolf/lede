// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include "mtk-snand-def.h"

static int mtk_snand_winbond_select_die(struct mtk_snand *snf, uint32_t dieidx);
static int mtk_snand_micron_select_die(struct mtk_snand *snf, uint32_t dieidx);

#define SNAND_MEMORG_512M_2K_64		SNAND_MEMORG(2048, 64, 64, 512, 1, 1)
#define SNAND_MEMORG_1G_2K_64		SNAND_MEMORG(2048, 64, 64, 1024, 1, 1)
#define SNAND_MEMORG_2G_2K_64		SNAND_MEMORG(2048, 64, 64, 2048, 1, 1)
#define SNAND_MEMORG_2G_2K_120		SNAND_MEMORG(2048, 120, 64, 2048, 1, 1)
#define SNAND_MEMORG_4G_2K_64		SNAND_MEMORG(2048, 64, 64, 4096, 1, 1)
#define SNAND_MEMORG_1G_2K_120		SNAND_MEMORG(2048, 120, 64, 1024, 1, 1)
#define SNAND_MEMORG_1G_2K_128		SNAND_MEMORG(2048, 128, 64, 1024, 1, 1)
#define SNAND_MEMORG_2G_2K_128		SNAND_MEMORG(2048, 128, 64, 2048, 1, 1)
#define SNAND_MEMORG_4G_2K_128		SNAND_MEMORG(2048, 128, 64, 4096, 1, 1)
#define SNAND_MEMORG_4G_4K_240		SNAND_MEMORG(4096, 240, 64, 2048, 1, 1)
#define SNAND_MEMORG_4G_4K_256		SNAND_MEMORG(4096, 256, 64, 2048, 1, 1)
#define SNAND_MEMORG_8G_4K_256		SNAND_MEMORG(4096, 256, 64, 4096, 1, 1)
#define SNAND_MEMORG_2G_2K_64_2P	SNAND_MEMORG(2048, 64, 64, 2048, 2, 1)
#define SNAND_MEMORG_2G_2K_64_2D	SNAND_MEMORG(2048, 64, 64, 1024, 1, 2)
#define SNAND_MEMORG_2G_2K_128_2P	SNAND_MEMORG(2048, 128, 64, 2048, 2, 1)
#define SNAND_MEMORG_4G_2K_64_2P	SNAND_MEMORG(2048, 64, 64, 4096, 2, 1)
#define SNAND_MEMORG_4G_2K_128_2P_2D	SNAND_MEMORG(2048, 128, 64, 2048, 2, 2)
#define SNAND_MEMORG_8G_4K_256_2D	SNAND_MEMORG(4096, 256, 64, 2048, 1, 2)

static const SNAND_IO_CAP(snand_cap_read_from_cache_quad,
	SPI_IO_1_1_1 | SPI_IO_1_1_2 | SPI_IO_1_2_2 | SPI_IO_1_1_4 |
	SPI_IO_1_4_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_READ_FROM_CACHE, 8),
	SNAND_OP(SNAND_IO_1_1_2, SNAND_CMD_READ_FROM_CACHE_X2, 8),
	SNAND_OP(SNAND_IO_1_2_2, SNAND_CMD_READ_FROM_CACHE_DUAL, 4),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_READ_FROM_CACHE_X4, 8),
	SNAND_OP(SNAND_IO_1_4_4, SNAND_CMD_READ_FROM_CACHE_QUAD, 4));

static const SNAND_IO_CAP(snand_cap_read_from_cache_quad_q2d,
	SPI_IO_1_1_1 | SPI_IO_1_1_2 | SPI_IO_1_2_2 | SPI_IO_1_1_4 |
	SPI_IO_1_4_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_READ_FROM_CACHE, 8),
	SNAND_OP(SNAND_IO_1_1_2, SNAND_CMD_READ_FROM_CACHE_X2, 8),
	SNAND_OP(SNAND_IO_1_2_2, SNAND_CMD_READ_FROM_CACHE_DUAL, 4),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_READ_FROM_CACHE_X4, 8),
	SNAND_OP(SNAND_IO_1_4_4, SNAND_CMD_READ_FROM_CACHE_QUAD, 2));

static const SNAND_IO_CAP(snand_cap_read_from_cache_quad_a8d,
	SPI_IO_1_1_1 | SPI_IO_1_1_2 | SPI_IO_1_2_2 | SPI_IO_1_1_4 |
	SPI_IO_1_4_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_READ_FROM_CACHE, 8),
	SNAND_OP(SNAND_IO_1_1_2, SNAND_CMD_READ_FROM_CACHE_X2, 8),
	SNAND_OP(SNAND_IO_1_2_2, SNAND_CMD_READ_FROM_CACHE_DUAL, 8),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_READ_FROM_CACHE_X4, 8),
	SNAND_OP(SNAND_IO_1_4_4, SNAND_CMD_READ_FROM_CACHE_QUAD, 8));

static const SNAND_IO_CAP(snand_cap_read_from_cache_x4,
	SPI_IO_1_1_1 | SPI_IO_1_1_2 | SPI_IO_1_1_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_READ_FROM_CACHE, 8),
	SNAND_OP(SNAND_IO_1_1_2, SNAND_CMD_READ_FROM_CACHE_X2, 8),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_READ_FROM_CACHE_X4, 8));

static const SNAND_IO_CAP(snand_cap_read_from_cache_x4_only,
	SPI_IO_1_1_1 | SPI_IO_1_1_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_READ_FROM_CACHE, 8),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_READ_FROM_CACHE_X4, 8));

static const SNAND_IO_CAP(snand_cap_program_load_x1,
	SPI_IO_1_1_1,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_PROGRAM_LOAD, 0));

static const SNAND_IO_CAP(snand_cap_program_load_x4,
	SPI_IO_1_1_1 | SPI_IO_1_1_4,
	SNAND_OP(SNAND_IO_1_1_1, SNAND_CMD_PROGRAM_LOAD, 0),
	SNAND_OP(SNAND_IO_1_1_4, SNAND_CMD_PROGRAM_LOAD_X4, 0));

static const struct snand_flash_info snand_flash_ids[] = {
	SNAND_INFO("W25N512GV", SNAND_ID(SNAND_ID_DYMMY, 0xef, 0xaa, 0x20),
		   SNAND_MEMORG_512M_2K_64,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("W25N01GV", SNAND_ID(SNAND_ID_DYMMY, 0xef, 0xaa, 0x21),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("W25M02GV", SNAND_ID(SNAND_ID_DYMMY, 0xef, 0xab, 0x21),
		   SNAND_MEMORG_2G_2K_64_2D,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4,
		   mtk_snand_winbond_select_die),
	SNAND_INFO("W25N02KV", SNAND_ID(SNAND_ID_DYMMY, 0xef, 0xaa, 0x22),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),

	SNAND_INFO("GD5F1GQ4UAWxx", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0x10),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F1GQ4UExIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xd1),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F1GQ4UExxH", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xd9),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F1GQ4xAYIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xf1),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F2GQ4UExIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xd2),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F2GQ5UExxH", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0x32),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_a8d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F2GQ4xAYIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xf2),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F4GQ4UBxIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xd4),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F4GQ4xAYIG", SNAND_ID(SNAND_ID_ADDR, 0xc8, 0xf4),
		   SNAND_MEMORG_4G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F2GQ5UExxG", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x52),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("GD5F4GQ4UCxIG", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0xb4),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),

	SNAND_INFO("MX35LF1GE4AB", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x12),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF1G24AD", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x14),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX31LF1GE4BC", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x1e),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF2GE4AB", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x22),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF2G24AD", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x24),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF2GE4AD", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x26),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF2G14AC", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x20),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF4G24AD", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x35),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MX35LF4GE4AD", SNAND_ID(SNAND_ID_DYMMY, 0xc2, 0x37),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),

	SNAND_INFO("MT29F1G01AAADD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x12),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("MT29F1G01ABAFD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x14),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MT29F2G01AAAED", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x9f),
		   SNAND_MEMORG_2G_2K_64_2P,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("MT29F2G01ABAGD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x24),
		   SNAND_MEMORG_2G_2K_128_2P,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MT29F4G01AAADD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x32),
		   SNAND_MEMORG_4G_2K_64_2P,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("MT29F4G01ABAFD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x34),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("MT29F4G01ADAGD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x36),
		   SNAND_MEMORG_4G_2K_128_2P_2D,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4,
		   mtk_snand_micron_select_die),
	SNAND_INFO("MT29F8G01ADAFD", SNAND_ID(SNAND_ID_DYMMY, 0x2c, 0x46),
		   SNAND_MEMORG_8G_4K_256_2D,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4,
		   mtk_snand_micron_select_die),

	SNAND_INFO("TC58CVG0S3HRAIG", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xc2),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("TC58CVG1S3HRAIG", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xcb),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("TC58CVG2S0HRAIG", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xcd),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x1),
	SNAND_INFO("TC58CVG0S3HRAIJ", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xe2),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("TC58CVG1S3HRAIJ", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xeb),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("TC58CVG2S0HRAIJ", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xed),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("TH58CVG3S0HRAIJ", SNAND_ID(SNAND_ID_DYMMY, 0x98, 0xe4),
		   SNAND_MEMORG_8G_4K_256,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),

	SNAND_INFO("F50L512M41A", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x20),
		   SNAND_MEMORG_512M_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("F50L1G41A", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x21),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),
	SNAND_INFO("F50L1G41LB", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x01),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4),
	SNAND_INFO("F50L2G41LB", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x0a),
		   SNAND_MEMORG_2G_2K_64_2D,
		   &snand_cap_read_from_cache_quad,
		   &snand_cap_program_load_x4,
		   mtk_snand_winbond_select_die),

	SNAND_INFO("CS11G0T0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x00),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G0G0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x10),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G0S0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x20),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G1T0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x01),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G1S0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x21),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G2T0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x02),
		   SNAND_MEMORG_4G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("CS11G2S0A0AA", SNAND_ID(SNAND_ID_DYMMY, 0x6b, 0x22),
		   SNAND_MEMORG_4G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),

	SNAND_INFO("EM73B044VCA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x01),
		   SNAND_MEMORG_512M_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SNB", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x11),
		   SNAND_MEMORG_1G_2K_120,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SNF", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x09),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044VCA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x18),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SNA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x19),
		   SNAND_MEMORG(2048, 64, 128, 512, 1, 1),
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044VCD", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1c),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SND", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1d),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SND", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1e),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044VCC", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x22),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044VCF", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x25),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SNC", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x31),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SNC", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x0a),
		   SNAND_MEMORG_2G_2K_120,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SNA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x12),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SNF", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x10),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x13),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCB", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x14),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCD", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x17),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCH", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1b),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SND", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1d),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCG", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x1f),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCE", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x20),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCL", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x2e),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044SNB", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x32),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044SNA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x03),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044SND", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x0b),
		   SNAND_MEMORG_4G_4K_240,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044SNB", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x23),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044VCA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x2c),
		   SNAND_MEMORG_4G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044VCB", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x2f),
		   SNAND_MEMORG_4G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73F044SNA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x24),
		   SNAND_MEMORG_8G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73F044VCA", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x2d),
		   SNAND_MEMORG_8G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73E044SNE", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x0e),
		   SNAND_MEMORG_8G_4K_256,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73C044SNG", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x0c),
		   SNAND_MEMORG_1G_2K_120,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("EM73D044VCN", SNAND_ID(SNAND_ID_DYMMY, 0xd5, 0x0f),
		   SNAND_MEMORG_2G_2K_64,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),

	SNAND_INFO("FM35Q1GA", SNAND_ID(SNAND_ID_DYMMY, 0xe5, 0x71),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),

	SNAND_INFO("PN26G01A", SNAND_ID(SNAND_ID_DYMMY, 0xa1, 0xe1),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("PN26G02A", SNAND_ID(SNAND_ID_DYMMY, 0xa1, 0xe2),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),

	SNAND_INFO("IS37SML01G1", SNAND_ID(SNAND_ID_DYMMY, 0xc8, 0x21),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4,
		   &snand_cap_program_load_x4),

	SNAND_INFO("ATO25D1GA", SNAND_ID(SNAND_ID_DYMMY, 0x9b, 0x12),
		   SNAND_MEMORG_1G_2K_64,
		   &snand_cap_read_from_cache_x4_only,
		   &snand_cap_program_load_x4),

	SNAND_INFO("HYF1GQ4U", SNAND_ID(SNAND_ID_DYMMY, 0xc9, 0x51),
		   SNAND_MEMORG_1G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
	SNAND_INFO("HYF2GQ4U", SNAND_ID(SNAND_ID_DYMMY, 0xc9, 0x52),
		   SNAND_MEMORG_2G_2K_128,
		   &snand_cap_read_from_cache_quad_q2d,
		   &snand_cap_program_load_x4),
};

static int mtk_snand_winbond_select_die(struct mtk_snand *snf, uint32_t dieidx)
{
	uint8_t op[2];

	if (dieidx > 1) {
		snand_log_chip(snf->pdev, "Invalid die index %u\n", dieidx);
		return -EINVAL;
	}

	op[0] = SNAND_CMD_WINBOND_SELECT_DIE;
	op[1] = (uint8_t)dieidx;

	return mtk_snand_mac_io(snf, op, sizeof(op), NULL, 0);
}

static int mtk_snand_micron_select_die(struct mtk_snand *snf, uint32_t dieidx)
{
	int ret;

	if (dieidx > 1) {
		snand_log_chip(snf->pdev, "Invalid die index %u\n", dieidx);
		return -EINVAL;
	}

	ret = mtk_snand_set_feature(snf, SNAND_FEATURE_MICRON_DIE_ADDR,
				    SNAND_MICRON_DIE_SEL_1);
	if (ret) {
		snand_log_chip(snf->pdev,
			       "Failed to set die selection feature\n");
		return ret;
	}

	return 0;
}

const struct snand_flash_info *snand_flash_id_lookup(enum snand_id_type type,
						     const uint8_t *id)
{
	const struct snand_id *fid;
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(snand_flash_ids); i++) {
		if (snand_flash_ids[i].id.type != type)
			continue;

		fid = &snand_flash_ids[i].id;
		if (memcmp(fid->id, id, fid->len))
			continue;

		return &snand_flash_ids[i];
	}

	return NULL;
}
