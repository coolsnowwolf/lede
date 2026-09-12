// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2022-2024 Phytium Technology Co., Ltd.
 */

#include <linux/mtd/spi-nor.h>

#include "core.h"

static const struct flash_info boya_nor_parts[] = {
	/* Boya */
	{ "by25q64as", INFO(0x684017, 0, 64 * 1024, 128)
        	FLAGS(SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
		NO_SFDP_FLAGS(SECT_4K | SPI_NOR_DUAL_READ |
			      SPI_NOR_QUAD_READ) },
	{ "by25q128as", INFO(0x684018, 0, 64 * 1024, 256)
        	FLAGS(SPI_NOR_HAS_LOCK | SPI_NOR_HAS_TB)
		NO_SFDP_FLAGS(SECT_4K | SPI_NOR_DUAL_READ |
			      SPI_NOR_QUAD_READ) },
};

const struct spi_nor_manufacturer spi_nor_boya = {
	.name = "boya",
	.parts = boya_nor_parts,
	.nparts = ARRAY_SIZE(boya_nor_parts),
};
