/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_SNAND_H_
#define _MTK_SNAND_H_

#ifndef PRIVATE_MTK_SNAND_HEADER
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#endif

enum mtk_snand_soc {
	SNAND_SOC_MT7622,
	SNAND_SOC_MT7629,

	__SNAND_SOC_MAX
};

struct mtk_snand_platdata {
	void *nfi_base;
	void *ecc_base;
	enum mtk_snand_soc soc;
	bool quad_spi;
};

struct mtk_snand_chip_info {
	const char *model;
	uint64_t chipsize;
	uint32_t blocksize;
	uint32_t pagesize;
	uint32_t sparesize;
	uint32_t spare_per_sector;
	uint32_t fdm_size;
	uint32_t fdm_ecc_size;
	uint32_t num_sectors;
	uint32_t sector_size;
	uint32_t ecc_strength;
	uint32_t ecc_bytes;
};

struct mtk_snand;
struct snand_flash_info;

int mtk_snand_init(void *dev, const struct mtk_snand_platdata *pdata,
		   struct mtk_snand **psnf);
int mtk_snand_cleanup(struct mtk_snand *snf);

int mtk_snand_chip_reset(struct mtk_snand *snf);
int mtk_snand_read_page(struct mtk_snand *snf, uint64_t addr, void *buf,
			void *oob, bool raw);
int mtk_snand_write_page(struct mtk_snand *snf, uint64_t addr, const void *buf,
			 const void *oob, bool raw);
int mtk_snand_erase_block(struct mtk_snand *snf, uint64_t addr);
int mtk_snand_block_isbad(struct mtk_snand *snf, uint64_t addr);
int mtk_snand_block_markbad(struct mtk_snand *snf, uint64_t addr);
int mtk_snand_fill_oob(struct mtk_snand *snf, uint8_t *oobraw,
		       const uint8_t *oobbuf, size_t ooblen);
int mtk_snand_transfer_oob(struct mtk_snand *snf, uint8_t *oobbuf,
			   size_t ooblen, const uint8_t *oobraw);
int mtk_snand_read_page_auto_oob(struct mtk_snand *snf, uint64_t addr,
				 void *buf, void *oob, size_t ooblen,
				 size_t *actualooblen, bool raw);
int mtk_snand_write_page_auto_oob(struct mtk_snand *snf, uint64_t addr,
				  const void *buf, const void *oob,
				  size_t ooblen, size_t *actualooblen,
				  bool raw);
int mtk_snand_get_chip_info(struct mtk_snand *snf,
			    struct mtk_snand_chip_info *info);
int mtk_snand_irq_process(struct mtk_snand *snf);

#endif /* _MTK_SNAND_H_ */
