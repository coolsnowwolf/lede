/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MTK_SNAND_DEF_H_
#define _MTK_SNAND_DEF_H_

#include "mtk-snand-os.h"

#ifdef PRIVATE_MTK_SNAND_HEADER
#include "mtk-snand.h"
#else
#include <mtk-snand.h>
#endif

struct mtk_snand_plat_dev;

enum snand_flash_io {
	SNAND_IO_1_1_1,
	SNAND_IO_1_1_2,
	SNAND_IO_1_2_2,
	SNAND_IO_1_1_4,
	SNAND_IO_1_4_4,

	__SNAND_IO_MAX
};

#define SPI_IO_1_1_1			BIT(SNAND_IO_1_1_1)
#define SPI_IO_1_1_2			BIT(SNAND_IO_1_1_2)
#define SPI_IO_1_2_2			BIT(SNAND_IO_1_2_2)
#define SPI_IO_1_1_4			BIT(SNAND_IO_1_1_4)
#define SPI_IO_1_4_4			BIT(SNAND_IO_1_4_4)

struct snand_opcode {
	uint8_t opcode;
	uint8_t dummy;
};

struct snand_io_cap {
	uint8_t caps;
	struct snand_opcode opcodes[__SNAND_IO_MAX];
};

#define SNAND_OP(_io, _opcode, _dummy) [_io] = { .opcode = (_opcode), \
						 .dummy = (_dummy) }

#define SNAND_IO_CAP(_name, _caps, ...) \
	struct snand_io_cap _name = { .caps = (_caps), \
				      .opcodes = { __VA_ARGS__ } }

#define SNAND_MAX_ID_LEN		4

enum snand_id_type {
	SNAND_ID_DYMMY,
	SNAND_ID_ADDR = SNAND_ID_DYMMY,
	SNAND_ID_DIRECT,

	__SNAND_ID_TYPE_MAX
};

struct snand_id {
	uint8_t type;	/* enum snand_id_type */
	uint8_t len;
	uint8_t id[SNAND_MAX_ID_LEN];
};

#define SNAND_ID(_type, ...) \
	{ .type = (_type), .id = { __VA_ARGS__ }, \
	  .len = sizeof((uint8_t[]) { __VA_ARGS__ }) }

struct snand_mem_org {
	uint16_t pagesize;
	uint16_t sparesize;
	uint16_t pages_per_block;
	uint16_t blocks_per_die;
	uint16_t planes_per_die;
	uint16_t ndies;
};

#define SNAND_MEMORG(_ps, _ss, _ppb, _bpd, _ppd, _nd) \
	{ .pagesize = (_ps), .sparesize = (_ss), .pages_per_block = (_ppb), \
	  .blocks_per_die = (_bpd), .planes_per_die = (_ppd), .ndies = (_nd) }

typedef int (*snand_select_die_t)(struct mtk_snand *snf, uint32_t dieidx);

struct snand_flash_info {
	const char *model;
	struct snand_id id;
	const struct snand_mem_org memorg;
	const struct snand_io_cap *cap_rd;
	const struct snand_io_cap *cap_pl;
	snand_select_die_t select_die;
};

#define SNAND_INFO(_model, _id, _memorg, _cap_rd, _cap_pl, ...) \
	{ .model = (_model), .id = _id, .memorg = _memorg, \
	  .cap_rd = (_cap_rd), .cap_pl = (_cap_pl), __VA_ARGS__ }

const struct snand_flash_info *snand_flash_id_lookup(enum snand_id_type type,
						     const uint8_t *id);

struct mtk_snand_soc_data {
	uint16_t sector_size;
	uint16_t max_sectors;
	uint16_t fdm_size;
	uint16_t fdm_ecc_size;
	uint16_t fifo_size;

	bool bbm_swap;
	bool empty_page_check;
	uint32_t mastersta_mask;

	const uint8_t *spare_sizes;
	uint32_t num_spare_size;
};

enum mtk_ecc_regs {
	ECC_DECDONE,
};

struct mtk_ecc_soc_data {
	const uint8_t *ecc_caps;
	uint32_t num_ecc_cap;
	const uint32_t *regs;
	uint16_t mode_shift;
	uint8_t errnum_bits;
	uint8_t errnum_shift;
};

struct mtk_snand {
	struct mtk_snand_plat_dev *pdev;

	void __iomem *nfi_base;
	void __iomem *ecc_base;

	enum mtk_snand_soc soc;
	const struct mtk_snand_soc_data *nfi_soc;
	const struct mtk_ecc_soc_data *ecc_soc;
	bool snfi_quad_spi;
	bool quad_spi_op;

	const char *model;
	uint64_t size;
	uint64_t die_size;
	uint32_t erasesize;
	uint32_t writesize;
	uint32_t oobsize;

	uint32_t num_dies;
	snand_select_die_t select_die;

	uint8_t opcode_rfc;
	uint8_t opcode_pl;
	uint8_t dummy_rfc;
	uint8_t mode_rfc;
	uint8_t mode_pl;

	uint32_t writesize_mask;
	uint32_t writesize_shift;
	uint32_t erasesize_mask;
	uint32_t erasesize_shift;
	uint64_t die_mask;
	uint32_t die_shift;

	uint32_t spare_per_sector;
	uint32_t raw_sector_size;
	uint32_t ecc_strength;
	uint32_t ecc_steps;
	uint32_t ecc_bytes;
	uint32_t ecc_parity_bits;

	uint8_t *page_cache;	/* Used by read/write page */
	uint8_t *buf_cache;	/* Used by block bad/markbad & auto_oob */
	int *sect_bf;		/* Used by ECC correction */
};

enum mtk_snand_log_category {
	SNAND_LOG_NFI,
	SNAND_LOG_SNFI,
	SNAND_LOG_ECC,
	SNAND_LOG_CHIP,

	__SNAND_LOG_CAT_MAX
};

int mtk_ecc_setup(struct mtk_snand *snf, void *fmdaddr, uint32_t max_ecc_bytes,
		  uint32_t msg_size);
int mtk_snand_ecc_encoder_start(struct mtk_snand *snf);
void mtk_snand_ecc_encoder_stop(struct mtk_snand *snf);
int mtk_snand_ecc_decoder_start(struct mtk_snand *snf);
void mtk_snand_ecc_decoder_stop(struct mtk_snand *snf);
int mtk_ecc_wait_decoder_done(struct mtk_snand *snf);
int mtk_ecc_check_decode_error(struct mtk_snand *snf);
int mtk_ecc_fixup_empty_sector(struct mtk_snand *snf, uint32_t sect);

int mtk_snand_mac_io(struct mtk_snand *snf, const uint8_t *out, uint32_t outlen,
		     uint8_t *in, uint32_t inlen);
int mtk_snand_set_feature(struct mtk_snand *snf, uint32_t addr, uint32_t val);

int mtk_snand_log(struct mtk_snand_plat_dev *pdev,
		  enum mtk_snand_log_category cat, const char *fmt, ...);

#define snand_log_nfi(pdev, fmt, ...) \
	mtk_snand_log(pdev, SNAND_LOG_NFI, fmt, ##__VA_ARGS__)

#define snand_log_snfi(pdev, fmt, ...) \
	mtk_snand_log(pdev, SNAND_LOG_SNFI, fmt, ##__VA_ARGS__)

#define snand_log_ecc(pdev, fmt, ...) \
	mtk_snand_log(pdev, SNAND_LOG_ECC, fmt, ##__VA_ARGS__)

#define snand_log_chip(pdev, fmt, ...) \
	mtk_snand_log(pdev, SNAND_LOG_CHIP, fmt, ##__VA_ARGS__)

/* ffs64 */
static inline int mtk_snand_ffs64(uint64_t x)
{
	if (!x)
		return 0;

	if (!(x & 0xffffffff))
		return ffs((uint32_t)(x >> 32)) + 32;

	return ffs((uint32_t)(x & 0xffffffff));
}

/* NFI dummy commands */
#define NFI_CMD_DUMMY_READ		0x00
#define NFI_CMD_DUMMY_WRITE		0x80

/* SPI-NAND opcodes */
#define SNAND_CMD_RESET			0xff
#define SNAND_CMD_BLOCK_ERASE		0xd8
#define SNAND_CMD_READ_FROM_CACHE_QUAD	0xeb
#define SNAND_CMD_WINBOND_SELECT_DIE	0xc2
#define SNAND_CMD_READ_FROM_CACHE_DUAL	0xbb
#define SNAND_CMD_READID		0x9f
#define SNAND_CMD_READ_FROM_CACHE_X4	0x6b
#define SNAND_CMD_READ_FROM_CACHE_X2	0x3b
#define SNAND_CMD_PROGRAM_LOAD_X4	0x32
#define SNAND_CMD_SET_FEATURE		0x1f
#define SNAND_CMD_READ_TO_CACHE		0x13
#define SNAND_CMD_PROGRAM_EXECUTE	0x10
#define SNAND_CMD_GET_FEATURE		0x0f
#define SNAND_CMD_READ_FROM_CACHE	0x0b
#define SNAND_CMD_WRITE_ENABLE		0x06
#define SNAND_CMD_PROGRAM_LOAD		0x02

/* SPI-NAND feature addresses */
#define SNAND_FEATURE_MICRON_DIE_ADDR	0xd0
#define SNAND_MICRON_DIE_SEL_1		BIT(6)

#define SNAND_FEATURE_STATUS_ADDR	0xc0
#define SNAND_STATUS_OIP		BIT(0)
#define SNAND_STATUS_WEL		BIT(1)
#define SNAND_STATUS_ERASE_FAIL		BIT(2)
#define SNAND_STATUS_PROGRAM_FAIL	BIT(3)

#define SNAND_FEATURE_CONFIG_ADDR	0xb0
#define SNAND_FEATURE_QUAD_ENABLE	BIT(0)
#define SNAND_FEATURE_ECC_EN		BIT(4)

#define SNAND_FEATURE_PROTECT_ADDR	0xa0

#endif /* _MTK_SNAND_DEF_H_ */
