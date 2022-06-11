// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include "mtk-snand-def.h"

/* NFI registers */
#define NFI_CNFG			0x000
#define CNFG_OP_MODE_S			12
#define   CNFG_OP_MODE_CUST		6
#define   CNFG_OP_MODE_PROGRAM		3
#define CNFG_AUTO_FMT_EN		BIT(9)
#define CNFG_HW_ECC_EN			BIT(8)
#define CNFG_DMA_BURST_EN		BIT(2)
#define CNFG_READ_MODE			BIT(1)
#define CNFG_DMA_MODE			BIT(0)

#define NFI_PAGEFMT			0x0004
#define NFI_SPARE_SIZE_LS_S		16
#define NFI_FDM_ECC_NUM_S		12
#define NFI_FDM_NUM_S			8
#define NFI_SPARE_SIZE_S		4
#define NFI_SEC_SEL_512			BIT(2)
#define NFI_PAGE_SIZE_S			0
#define   NFI_PAGE_SIZE_512_2K		0
#define   NFI_PAGE_SIZE_2K_4K		1
#define   NFI_PAGE_SIZE_4K_8K		2
#define   NFI_PAGE_SIZE_8K_16K		3

#define NFI_CON				0x008
#define CON_SEC_NUM_S			12
#define CON_BWR				BIT(9)
#define CON_BRD				BIT(8)
#define CON_NFI_RST			BIT(1)
#define CON_FIFO_FLUSH			BIT(0)

#define NFI_INTR_EN			0x010
#define NFI_INTR_STA			0x014
#define NFI_IRQ_INTR_EN			BIT(31)
#define NFI_IRQ_CUS_READ		BIT(8)
#define NFI_IRQ_CUS_PG			BIT(7)

#define NFI_CMD				0x020

#define NFI_STRDATA			0x040
#define STR_DATA			BIT(0)

#define NFI_STA				0x060
#define NFI_NAND_FSM			GENMASK(28, 24)
#define NFI_FSM				GENMASK(19, 16)
#define READ_EMPTY			BIT(12)

#define NFI_FIFOSTA			0x064
#define FIFO_WR_REMAIN_S		8
#define FIFO_RD_REMAIN_S		0

#define NFI_ADDRCNTR			0x070
#define SEC_CNTR			GENMASK(16, 12)
#define SEC_CNTR_S			12
#define NFI_SEC_CNTR(val)		(((val) & SEC_CNTR) >> SEC_CNTR_S)

#define NFI_STRADDR			0x080

#define NFI_BYTELEN			0x084
#define BUS_SEC_CNTR(val)		(((val) & SEC_CNTR) >> SEC_CNTR_S)

#define NFI_FDM0L			0x0a0
#define NFI_FDM0M			0x0a4
#define NFI_FDML(n)			(NFI_FDM0L + (n) * 8)
#define NFI_FDMM(n)			(NFI_FDM0M + (n) * 8)

#define NFI_DEBUG_CON1			0x220
#define WBUF_EN				BIT(2)

#define NFI_MASTERSTA			0x224
#define MAS_ADDR			GENMASK(11, 9)
#define MAS_RD				GENMASK(8, 6)
#define MAS_WR				GENMASK(5, 3)
#define MAS_RDDLY			GENMASK(2, 0)
#define NFI_MASTERSTA_MASK_7622		(MAS_ADDR | MAS_RD | MAS_WR | MAS_RDDLY)

/* SNFI registers */
#define SNF_MAC_CTL			0x500
#define MAC_XIO_SEL			BIT(4)
#define SF_MAC_EN			BIT(3)
#define SF_TRIG				BIT(2)
#define WIP_READY			BIT(1)
#define WIP				BIT(0)

#define SNF_MAC_OUTL			0x504
#define SNF_MAC_INL			0x508

#define SNF_RD_CTL2			0x510
#define DATA_READ_DUMMY_S		8
#define DATA_READ_CMD_S			0

#define SNF_RD_CTL3			0x514

#define SNF_PG_CTL1			0x524
#define PG_LOAD_CMD_S			8

#define SNF_PG_CTL2			0x528

#define SNF_MISC_CTL			0x538
#define SW_RST				BIT(28)
#define FIFO_RD_LTC_S			25
#define PG_LOAD_X4_EN			BIT(20)
#define DATA_READ_MODE_S		16
#define DATA_READ_MODE			GENMASK(18, 16)
#define   DATA_READ_MODE_X1		0
#define   DATA_READ_MODE_X2		1
#define   DATA_READ_MODE_X4		2
#define   DATA_READ_MODE_DUAL		5
#define   DATA_READ_MODE_QUAD		6
#define PG_LOAD_CUSTOM_EN		BIT(7)
#define DATARD_CUSTOM_EN		BIT(6)
#define CS_DESELECT_CYC_S		0

#define SNF_MISC_CTL2			0x53c
#define PROGRAM_LOAD_BYTE_NUM_S		16
#define READ_DATA_BYTE_NUM_S		11

#define SNF_DLY_CTL3			0x548
#define SFCK_SAM_DLY_S			0

#define SNF_STA_CTL1			0x550
#define CUS_PG_DONE			BIT(28)
#define CUS_READ_DONE			BIT(27)
#define SPI_STATE_S			0
#define SPI_STATE			GENMASK(3, 0)

#define SNF_CFG				0x55c
#define SPI_MODE			BIT(0)

#define SNF_GPRAM			0x800
#define SNF_GPRAM_SIZE			0xa0

#define SNFI_POLL_INTERVAL		1000000

static const uint8_t mt7622_spare_sizes[] = { 16, 26, 27, 28 };

static const struct mtk_snand_soc_data mtk_snand_socs[__SNAND_SOC_MAX] = {
	[SNAND_SOC_MT7622] = {
		.sector_size = 512,
		.max_sectors = 8,
		.fdm_size = 8,
		.fdm_ecc_size = 1,
		.fifo_size = 32,
		.bbm_swap = false,
		.empty_page_check = false,
		.mastersta_mask = NFI_MASTERSTA_MASK_7622,
		.spare_sizes = mt7622_spare_sizes,
		.num_spare_size = ARRAY_SIZE(mt7622_spare_sizes)
	},
	[SNAND_SOC_MT7629] = {
		.sector_size = 512,
		.max_sectors = 8,
		.fdm_size = 8,
		.fdm_ecc_size = 1,
		.fifo_size = 32,
		.bbm_swap = true,
		.empty_page_check = false,
		.mastersta_mask = NFI_MASTERSTA_MASK_7622,
		.spare_sizes = mt7622_spare_sizes,
		.num_spare_size = ARRAY_SIZE(mt7622_spare_sizes)
	},
};

static inline uint32_t nfi_read32(struct mtk_snand *snf, uint32_t reg)
{
	return readl(snf->nfi_base + reg);
}

static inline void nfi_write32(struct mtk_snand *snf, uint32_t reg,
			       uint32_t val)
{
	writel(val, snf->nfi_base + reg);
}

static inline void nfi_write16(struct mtk_snand *snf, uint32_t reg,
			       uint16_t val)
{
	writew(val, snf->nfi_base + reg);
}

static inline void nfi_rmw32(struct mtk_snand *snf, uint32_t reg, uint32_t clr,
			     uint32_t set)
{
	uint32_t val;

	val = readl(snf->nfi_base + reg);
	val &= ~clr;
	val |= set;
	writel(val, snf->nfi_base + reg);
}

static void nfi_write_data(struct mtk_snand *snf, uint32_t reg,
			   const uint8_t *data, uint32_t len)
{
	uint32_t i, val = 0, es = sizeof(uint32_t);

	for (i = reg; i < reg + len; i++) {
		val |= ((uint32_t)*data++) << (8 * (i % es));

		if (i % es == es - 1 || i == reg + len - 1) {
			nfi_write32(snf, i & ~(es - 1), val);
			val = 0;
		}
	}
}

static void nfi_read_data(struct mtk_snand *snf, uint32_t reg, uint8_t *data,
			  uint32_t len)
{
	uint32_t i, val = 0, es = sizeof(uint32_t);

	for (i = reg; i < reg + len; i++) {
		if (i == reg || i % es == 0)
			val = nfi_read32(snf, i & ~(es - 1));

		*data++ = (uint8_t)(val >> (8 * (i % es)));
	}
}

static inline void do_bm_swap(uint8_t *bm1, uint8_t *bm2)
{
	uint8_t tmp = *bm1;
	*bm1 = *bm2;
	*bm2 = tmp;
}

static void mtk_snand_bm_swap_raw(struct mtk_snand *snf)
{
	uint32_t fdm_bbm_pos;

	if (!snf->nfi_soc->bbm_swap || snf->ecc_steps == 1)
		return;

	fdm_bbm_pos = (snf->ecc_steps - 1) * snf->raw_sector_size +
		      snf->nfi_soc->sector_size;
	do_bm_swap(&snf->page_cache[fdm_bbm_pos],
		   &snf->page_cache[snf->writesize]);
}

static void mtk_snand_bm_swap(struct mtk_snand *snf)
{
	uint32_t buf_bbm_pos, fdm_bbm_pos;

	if (!snf->nfi_soc->bbm_swap || snf->ecc_steps == 1)
		return;

	buf_bbm_pos = snf->writesize -
		      (snf->ecc_steps - 1) * snf->spare_per_sector;
	fdm_bbm_pos = snf->writesize +
		      (snf->ecc_steps - 1) * snf->nfi_soc->fdm_size;
	do_bm_swap(&snf->page_cache[fdm_bbm_pos],
		   &snf->page_cache[buf_bbm_pos]);
}

static void mtk_snand_fdm_bm_swap_raw(struct mtk_snand *snf)
{
	uint32_t fdm_bbm_pos1, fdm_bbm_pos2;

	if (!snf->nfi_soc->bbm_swap || snf->ecc_steps == 1)
		return;

	fdm_bbm_pos1 = snf->nfi_soc->sector_size;
	fdm_bbm_pos2 = (snf->ecc_steps - 1) * snf->raw_sector_size +
		       snf->nfi_soc->sector_size;
	do_bm_swap(&snf->page_cache[fdm_bbm_pos1],
		   &snf->page_cache[fdm_bbm_pos2]);
}

static void mtk_snand_fdm_bm_swap(struct mtk_snand *snf)
{
	uint32_t fdm_bbm_pos1, fdm_bbm_pos2;

	if (!snf->nfi_soc->bbm_swap || snf->ecc_steps == 1)
		return;

	fdm_bbm_pos1 = snf->writesize;
	fdm_bbm_pos2 = snf->writesize +
		       (snf->ecc_steps - 1) * snf->nfi_soc->fdm_size;
	do_bm_swap(&snf->page_cache[fdm_bbm_pos1],
		   &snf->page_cache[fdm_bbm_pos2]);
}

static int mtk_nfi_reset(struct mtk_snand *snf)
{
	uint32_t val, fifo_mask;
	int ret;

	nfi_write32(snf, NFI_CON, CON_FIFO_FLUSH | CON_NFI_RST);

	ret = read16_poll_timeout(snf->nfi_base + NFI_MASTERSTA, val,
				  !(val & snf->nfi_soc->mastersta_mask), 0,
				  SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "NFI master is still busy after reset\n");
		return ret;
	}

	ret = read32_poll_timeout(snf->nfi_base + NFI_STA, val,
				  !(val & (NFI_FSM | NFI_NAND_FSM)), 0,
				  SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev, "Failed to reset NFI\n");
		return ret;
	}

	fifo_mask = ((snf->nfi_soc->fifo_size - 1) << FIFO_RD_REMAIN_S) |
		    ((snf->nfi_soc->fifo_size - 1) << FIFO_WR_REMAIN_S);
	ret = read16_poll_timeout(snf->nfi_base + NFI_FIFOSTA, val,
				  !(val & fifo_mask), 0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev, "NFI FIFOs are not empty\n");
		return ret;
	}

	return 0;
}

static int mtk_snand_mac_reset(struct mtk_snand *snf)
{
	int ret;
	uint32_t val;

	nfi_rmw32(snf, SNF_MISC_CTL, 0, SW_RST);

	ret = read32_poll_timeout(snf->nfi_base + SNF_STA_CTL1, val,
				  !(val & SPI_STATE), 0, SNFI_POLL_INTERVAL);
	if (ret)
		snand_log_snfi(snf->pdev, "Failed to reset SNFI MAC\n");

	nfi_write32(snf, SNF_MISC_CTL, (2 << FIFO_RD_LTC_S) |
		    (10 << CS_DESELECT_CYC_S));

	return ret;
}

static int mtk_snand_mac_trigger(struct mtk_snand *snf, uint32_t outlen,
				 uint32_t inlen)
{
	int ret;
	uint32_t val;

	nfi_write32(snf, SNF_MAC_CTL, SF_MAC_EN);
	nfi_write32(snf, SNF_MAC_OUTL, outlen);
	nfi_write32(snf, SNF_MAC_INL, inlen);

	nfi_write32(snf, SNF_MAC_CTL, SF_MAC_EN | SF_TRIG);

	ret = read32_poll_timeout(snf->nfi_base + SNF_MAC_CTL, val,
				  val & WIP_READY, 0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_snfi(snf->pdev, "Timed out waiting for WIP_READY\n");
		goto cleanup;
	}

	ret = read32_poll_timeout(snf->nfi_base + SNF_MAC_CTL, val,
				  !(val & WIP), 0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_snfi(snf->pdev,
			       "Timed out waiting for WIP cleared\n");
	}

cleanup:
	nfi_write32(snf, SNF_MAC_CTL, 0);

	return ret;
}

int mtk_snand_mac_io(struct mtk_snand *snf, const uint8_t *out, uint32_t outlen,
		     uint8_t *in, uint32_t inlen)
{
	int ret;

	if (outlen + inlen > SNF_GPRAM_SIZE)
		return -EINVAL;

	mtk_snand_mac_reset(snf);

	nfi_write_data(snf, SNF_GPRAM, out, outlen);

	ret = mtk_snand_mac_trigger(snf, outlen, inlen);
	if (ret)
		return ret;

	if (!inlen)
		return 0;

	nfi_read_data(snf, SNF_GPRAM + outlen, in, inlen);

	return 0;
}

static int mtk_snand_get_feature(struct mtk_snand *snf, uint32_t addr)
{
	uint8_t op[2], val;
	int ret;

	op[0] = SNAND_CMD_GET_FEATURE;
	op[1] = (uint8_t)addr;

	ret = mtk_snand_mac_io(snf, op, sizeof(op), &val, 1);
	if (ret)
		return ret;

	return val;
}

int mtk_snand_set_feature(struct mtk_snand *snf, uint32_t addr, uint32_t val)
{
	uint8_t op[3];

	op[0] = SNAND_CMD_SET_FEATURE;
	op[1] = (uint8_t)addr;
	op[2] = (uint8_t)val;

	return mtk_snand_mac_io(snf, op, sizeof(op), NULL, 0);
}

static int mtk_snand_poll_status(struct mtk_snand *snf, uint32_t wait_us)
{
	int val;
	mtk_snand_time_t time_start, tmo;

	time_start = timer_get_ticks();
	tmo = timer_time_to_tick(wait_us);

	do {
		val = mtk_snand_get_feature(snf, SNAND_FEATURE_STATUS_ADDR);
		if (!(val & SNAND_STATUS_OIP))
			return val & (SNAND_STATUS_ERASE_FAIL |
				      SNAND_STATUS_PROGRAM_FAIL);
	} while (!timer_is_timeout(time_start, tmo));

	return -ETIMEDOUT;
}

int mtk_snand_chip_reset(struct mtk_snand *snf)
{
	uint8_t op = SNAND_CMD_RESET;
	int ret;

	ret = mtk_snand_mac_io(snf, &op, 1, NULL, 0);
	if (ret)
		return ret;

	ret = mtk_snand_poll_status(snf, SNFI_POLL_INTERVAL);
	if (ret < 0)
		return ret;

	return 0;
}

static int mtk_snand_config_feature(struct mtk_snand *snf, uint8_t clr,
				    uint8_t set)
{
	int val, newval;
	int ret;

	val = mtk_snand_get_feature(snf, SNAND_FEATURE_CONFIG_ADDR);
	if (val < 0) {
		snand_log_chip(snf->pdev,
			       "Failed to get configuration feature\n");
		return val;
	}

	newval = (val & (~clr)) | set;

	if (newval == val)
		return 0;

	ret = mtk_snand_set_feature(snf, SNAND_FEATURE_CONFIG_ADDR,
				    (uint8_t)newval);
	if (val < 0) {
		snand_log_chip(snf->pdev,
			       "Failed to set configuration feature\n");
		return ret;
	}

	val = mtk_snand_get_feature(snf, SNAND_FEATURE_CONFIG_ADDR);
	if (val < 0) {
		snand_log_chip(snf->pdev,
			       "Failed to get configuration feature\n");
		return val;
	}

	if (newval != val)
		return -ENOTSUPP;

	return 0;
}

static int mtk_snand_ondie_ecc_control(struct mtk_snand *snf, bool enable)
{
	int ret;

	if (enable)
		ret = mtk_snand_config_feature(snf, 0, SNAND_FEATURE_ECC_EN);
	else
		ret = mtk_snand_config_feature(snf, SNAND_FEATURE_ECC_EN, 0);

	if (ret) {
		snand_log_chip(snf->pdev, "Failed to %s On-Die ECC engine\n",
			       enable ? "enable" : "disable");
	}

	return ret;
}

static int mtk_snand_qspi_control(struct mtk_snand *snf, bool enable)
{
	int ret;

	if (enable) {
		ret = mtk_snand_config_feature(snf, 0,
					       SNAND_FEATURE_QUAD_ENABLE);
	} else {
		ret = mtk_snand_config_feature(snf,
					       SNAND_FEATURE_QUAD_ENABLE, 0);
	}

	if (ret) {
		snand_log_chip(snf->pdev, "Failed to %s quad spi\n",
			       enable ? "enable" : "disable");
	}

	return ret;
}

static int mtk_snand_unlock(struct mtk_snand *snf)
{
	int ret;

	ret = mtk_snand_set_feature(snf, SNAND_FEATURE_PROTECT_ADDR, 0);
	if (ret) {
		snand_log_chip(snf->pdev, "Failed to set protection feature\n");
		return ret;
	}

	return 0;
}

static int mtk_snand_write_enable(struct mtk_snand *snf)
{
	uint8_t op = SNAND_CMD_WRITE_ENABLE;
	int ret, val;

	ret = mtk_snand_mac_io(snf, &op, 1, NULL, 0);
	if (ret)
		return ret;

	val = mtk_snand_get_feature(snf, SNAND_FEATURE_STATUS_ADDR);
	if (val < 0)
		return ret;

	if (val & SNAND_STATUS_WEL)
		return 0;

	snand_log_chip(snf->pdev, "Failed to send write-enable command\n");

	return -ENOTSUPP;
}

static int mtk_snand_select_die(struct mtk_snand *snf, uint32_t dieidx)
{
	if (!snf->select_die)
		return 0;

	return snf->select_die(snf, dieidx);
}

static uint64_t mtk_snand_select_die_address(struct mtk_snand *snf,
					     uint64_t addr)
{
	uint32_t dieidx;

	if (!snf->select_die)
		return addr;

	dieidx = addr >> snf->die_shift;

	mtk_snand_select_die(snf, dieidx);

	return addr & snf->die_mask;
}

static uint32_t mtk_snand_get_plane_address(struct mtk_snand *snf,
					    uint32_t page)
{
	uint32_t pages_per_block;

	pages_per_block = 1 << (snf->erasesize_shift - snf->writesize_shift);

	if (page & pages_per_block)
		return 1 << (snf->writesize_shift + 1);

	return 0;
}

static int mtk_snand_page_op(struct mtk_snand *snf, uint32_t page, uint8_t cmd)
{
	uint8_t op[4];

	op[0] = cmd;
	op[1] = (page >> 16) & 0xff;
	op[2] = (page >> 8) & 0xff;
	op[3] = page & 0xff;

	return mtk_snand_mac_io(snf, op, sizeof(op), NULL, 0);
}

static void mtk_snand_read_fdm(struct mtk_snand *snf, uint8_t *buf)
{
	uint32_t vall, valm;
	uint8_t *oobptr = buf;
	int i, j;

	for (i = 0; i < snf->ecc_steps; i++) {
		vall = nfi_read32(snf, NFI_FDML(i));
		valm = nfi_read32(snf, NFI_FDMM(i));

		for (j = 0; j < snf->nfi_soc->fdm_size; j++)
			oobptr[j] = (j >= 4 ? valm : vall) >> ((j % 4) * 8);

		oobptr += snf->nfi_soc->fdm_size;
	}
}

static int mtk_snand_read_ecc_parity(struct mtk_snand *snf, uint32_t page,
				     uint32_t sect, uint8_t *oob)
{
	uint32_t ecc_bytes = snf->spare_per_sector - snf->nfi_soc->fdm_size;
	uint32_t coladdr, raw_offs, offs;
	uint8_t op[4];

	if (sizeof(op) + ecc_bytes > SNF_GPRAM_SIZE) {
		snand_log_snfi(snf->pdev,
			       "ECC parity size does not fit the GPRAM\n");
		return -ENOTSUPP;
	}

	raw_offs = sect * snf->raw_sector_size + snf->nfi_soc->sector_size +
		   snf->nfi_soc->fdm_size;
	offs = snf->ecc_steps * snf->nfi_soc->fdm_size + sect * ecc_bytes;

	/* Column address with plane bit */
	coladdr = raw_offs | mtk_snand_get_plane_address(snf, page);

	op[0] = SNAND_CMD_READ_FROM_CACHE;
	op[1] = (coladdr >> 8) & 0xff;
	op[2] = coladdr & 0xff;
	op[3] = 0;

	return mtk_snand_mac_io(snf, op, sizeof(op), oob + offs, ecc_bytes);
}

static int mtk_snand_check_ecc_result(struct mtk_snand *snf, uint32_t page)
{
	uint8_t *oob = snf->page_cache + snf->writesize;
	int i, rc, ret = 0, max_bitflips = 0;

	for (i = 0; i < snf->ecc_steps; i++) {
		if (snf->sect_bf[i] >= 0) {
			if (snf->sect_bf[i] > max_bitflips)
				max_bitflips = snf->sect_bf[i];
			continue;
		}

		rc = mtk_snand_read_ecc_parity(snf, page, i, oob);
		if (rc)
			return rc;

		rc = mtk_ecc_fixup_empty_sector(snf, i);
		if (rc < 0) {
			ret = -EBADMSG;

			snand_log_ecc(snf->pdev,
			      "Uncorrectable bitflips in page %u sect %u\n",
			      page, i);
		} else if (rc) {
			snf->sect_bf[i] = rc;

			if (snf->sect_bf[i] > max_bitflips)
				max_bitflips = snf->sect_bf[i];

			snand_log_ecc(snf->pdev,
			      "%u bitflip%s corrected in page %u sect %u\n",
			      rc, rc > 1 ? "s" : "", page, i);
		} else {
			snf->sect_bf[i] = 0;
		}
	}

	return ret ? ret : max_bitflips;
}

static int mtk_snand_read_cache(struct mtk_snand *snf, uint32_t page, bool raw)
{
	uint32_t coladdr, rwbytes, mode, len, val;
	uintptr_t dma_addr;
	int ret;

	/* Column address with plane bit */
	coladdr = mtk_snand_get_plane_address(snf, page);

	mtk_snand_mac_reset(snf);
	mtk_nfi_reset(snf);

	/* Command and dummy cycles */
	nfi_write32(snf, SNF_RD_CTL2,
		    ((uint32_t)snf->dummy_rfc << DATA_READ_DUMMY_S) |
		    (snf->opcode_rfc << DATA_READ_CMD_S));

	/* Column address */
	nfi_write32(snf, SNF_RD_CTL3, coladdr);

	/* Set read mode */
	mode = (uint32_t)snf->mode_rfc << DATA_READ_MODE_S;
	nfi_rmw32(snf, SNF_MISC_CTL, DATA_READ_MODE, mode | DATARD_CUSTOM_EN);

	/* Set bytes to read */
	rwbytes = snf->ecc_steps * snf->raw_sector_size;
	nfi_write32(snf, SNF_MISC_CTL2, (rwbytes << PROGRAM_LOAD_BYTE_NUM_S) |
		    rwbytes);

	/* NFI read prepare */
	mode = raw ? 0 : CNFG_HW_ECC_EN | CNFG_AUTO_FMT_EN;
	nfi_write16(snf, NFI_CNFG, (CNFG_OP_MODE_CUST << CNFG_OP_MODE_S) |
		    CNFG_DMA_BURST_EN | CNFG_READ_MODE | CNFG_DMA_MODE | mode);

	nfi_write32(snf, NFI_CON, (snf->ecc_steps << CON_SEC_NUM_S));

	/* Prepare for DMA read */
	len = snf->writesize + snf->oobsize;
	ret = dma_mem_map(snf->pdev, snf->page_cache, &dma_addr, len, false);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "DMA map from device failed with %d\n", ret);
		return ret;
	}

	nfi_write32(snf, NFI_STRADDR, (uint32_t)dma_addr);

	if (!raw)
		mtk_snand_ecc_decoder_start(snf);

	/* Prepare for custom read interrupt */
	nfi_write32(snf, NFI_INTR_EN, NFI_IRQ_INTR_EN | NFI_IRQ_CUS_READ);
	irq_completion_init(snf->pdev);

	/* Trigger NFI into custom mode */
	nfi_write16(snf, NFI_CMD, NFI_CMD_DUMMY_READ);

	/* Start DMA read */
	nfi_rmw32(snf, NFI_CON, 0, CON_BRD);
	nfi_write16(snf, NFI_STRDATA, STR_DATA);

	/* Wait for operation finished */
	ret = irq_completion_wait(snf->pdev, snf->nfi_base + SNF_STA_CTL1,
				  CUS_READ_DONE, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "DMA timed out for reading from cache\n");
		goto cleanup;
	}

	/* Wait for BUS_SEC_CNTR returning expected value */
	ret = read32_poll_timeout(snf->nfi_base + NFI_BYTELEN, val,
				  BUS_SEC_CNTR(val) >= snf->ecc_steps,
				  0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "Timed out waiting for BUS_SEC_CNTR\n");
		goto cleanup;
	}

	/* Wait for bus becoming idle */
	ret = read32_poll_timeout(snf->nfi_base + NFI_MASTERSTA, val,
				  !(val & snf->nfi_soc->mastersta_mask),
				  0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "Timed out waiting for bus becoming idle\n");
		goto cleanup;
	}

	if (!raw) {
		ret = mtk_ecc_wait_decoder_done(snf);
		if (ret)
			goto cleanup;

		mtk_snand_read_fdm(snf, snf->page_cache + snf->writesize);

		mtk_ecc_check_decode_error(snf);
		mtk_snand_ecc_decoder_stop(snf);

		ret = mtk_snand_check_ecc_result(snf, page);
	}

cleanup:
	/* DMA cleanup */
	dma_mem_unmap(snf->pdev, dma_addr, len, false);

	/* Stop read */
	nfi_write32(snf, NFI_CON, 0);
	nfi_write16(snf, NFI_CNFG, 0);

	/* Clear SNF done flag */
	nfi_rmw32(snf, SNF_STA_CTL1, 0, CUS_READ_DONE);
	nfi_write32(snf, SNF_STA_CTL1, 0);

	/* Disable interrupt */
	nfi_read32(snf, NFI_INTR_STA);
	nfi_write32(snf, NFI_INTR_EN, 0);

	nfi_rmw32(snf, SNF_MISC_CTL, DATARD_CUSTOM_EN, 0);

	return ret;
}

static void mtk_snand_from_raw_page(struct mtk_snand *snf, void *buf, void *oob)
{
	uint32_t i, ecc_bytes = snf->spare_per_sector - snf->nfi_soc->fdm_size;
	uint8_t *eccptr = oob + snf->ecc_steps * snf->nfi_soc->fdm_size;
	uint8_t *bufptr = buf, *oobptr = oob, *raw_sector;

	for (i = 0; i < snf->ecc_steps; i++) {
		raw_sector = snf->page_cache + i * snf->raw_sector_size;

		if (buf) {
			memcpy(bufptr, raw_sector, snf->nfi_soc->sector_size);
			bufptr += snf->nfi_soc->sector_size;
		}

		raw_sector += snf->nfi_soc->sector_size;

		if (oob) {
			memcpy(oobptr, raw_sector, snf->nfi_soc->fdm_size);
			oobptr += snf->nfi_soc->fdm_size;
			raw_sector += snf->nfi_soc->fdm_size;

			memcpy(eccptr, raw_sector, ecc_bytes);
			eccptr += ecc_bytes;
		}
	}
}

static int mtk_snand_do_read_page(struct mtk_snand *snf, uint64_t addr,
				  void *buf, void *oob, bool raw, bool format)
{
	uint64_t die_addr;
	uint32_t page;
	int ret;

	die_addr = mtk_snand_select_die_address(snf, addr);
	page = die_addr >> snf->writesize_shift;

	ret = mtk_snand_page_op(snf, page, SNAND_CMD_READ_TO_CACHE);
	if (ret)
		return ret;

	ret = mtk_snand_poll_status(snf, SNFI_POLL_INTERVAL);
	if (ret < 0) {
		snand_log_chip(snf->pdev, "Read to cache command timed out\n");
		return ret;
	}

	ret = mtk_snand_read_cache(snf, page, raw);
	if (ret < 0 && ret != -EBADMSG)
		return ret;

	if (raw) {
		if (format) {
			mtk_snand_bm_swap_raw(snf);
			mtk_snand_fdm_bm_swap_raw(snf);
			mtk_snand_from_raw_page(snf, buf, oob);
		} else {
			if (buf)
				memcpy(buf, snf->page_cache, snf->writesize);

			if (oob) {
				memset(oob, 0xff, snf->oobsize);
				memcpy(oob, snf->page_cache + snf->writesize,
				       snf->ecc_steps * snf->spare_per_sector);
			}
		}
	} else {
		mtk_snand_bm_swap(snf);
		mtk_snand_fdm_bm_swap(snf);

		if (buf)
			memcpy(buf, snf->page_cache, snf->writesize);

		if (oob) {
			memset(oob, 0xff, snf->oobsize);
			memcpy(oob, snf->page_cache + snf->writesize,
			       snf->ecc_steps * snf->nfi_soc->fdm_size);
		}
	}

	return ret;
}

int mtk_snand_read_page(struct mtk_snand *snf, uint64_t addr, void *buf,
			void *oob, bool raw)
{
	if (!snf || (!buf && !oob))
		return -EINVAL;

	if (addr >= snf->size)
		return -EINVAL;

	return mtk_snand_do_read_page(snf, addr, buf, oob, raw, true);
}

static void mtk_snand_write_fdm(struct mtk_snand *snf, const uint8_t *buf)
{
	uint32_t vall, valm, fdm_size = snf->nfi_soc->fdm_size;
	const uint8_t *oobptr = buf;
	int i, j;

	for (i = 0; i < snf->ecc_steps; i++) {
		vall = 0;
		valm = 0;

		for (j = 0; j < 8; j++) {
			if (j < 4)
				vall |= (j < fdm_size ? oobptr[j] : 0xff)
						<< (j * 8);
			else
				valm |= (j < fdm_size ? oobptr[j] : 0xff)
						<< ((j - 4) * 8);
		}

		nfi_write32(snf, NFI_FDML(i), vall);
		nfi_write32(snf, NFI_FDMM(i), valm);

		oobptr += fdm_size;
	}
}

static int mtk_snand_program_load(struct mtk_snand *snf, uint32_t page,
				  bool raw)
{
	uint32_t coladdr, rwbytes, mode, len, val;
	uintptr_t dma_addr;
	int ret;

	/* Column address with plane bit */
	coladdr = mtk_snand_get_plane_address(snf, page);

	mtk_snand_mac_reset(snf);
	mtk_nfi_reset(snf);

	/* Write FDM registers if necessary */
	if (!raw)
		mtk_snand_write_fdm(snf, snf->page_cache + snf->writesize);

	/* Command */
	nfi_write32(snf, SNF_PG_CTL1, (snf->opcode_pl << PG_LOAD_CMD_S));

	/* Column address */
	nfi_write32(snf, SNF_PG_CTL2, coladdr);

	/* Set write mode */
	mode = snf->mode_pl ? PG_LOAD_X4_EN : 0;
	nfi_rmw32(snf, SNF_MISC_CTL, PG_LOAD_X4_EN, mode | PG_LOAD_CUSTOM_EN);

	/* Set bytes to write */
	rwbytes = snf->ecc_steps * snf->raw_sector_size;
	nfi_write32(snf, SNF_MISC_CTL2, (rwbytes << PROGRAM_LOAD_BYTE_NUM_S) |
		    rwbytes);

	/* NFI write prepare */
	mode = raw ? 0 : CNFG_HW_ECC_EN | CNFG_AUTO_FMT_EN;
	nfi_write16(snf, NFI_CNFG, (CNFG_OP_MODE_PROGRAM << CNFG_OP_MODE_S) |
		    CNFG_DMA_BURST_EN | CNFG_DMA_MODE | mode);

	nfi_write32(snf, NFI_CON, (snf->ecc_steps << CON_SEC_NUM_S));

	/* Prepare for DMA write */
	len = snf->writesize + snf->oobsize;
	ret = dma_mem_map(snf->pdev, snf->page_cache, &dma_addr, len, true);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "DMA map to device failed with %d\n", ret);
		return ret;
	}

	nfi_write32(snf, NFI_STRADDR, (uint32_t)dma_addr);

	if (!raw)
		mtk_snand_ecc_encoder_start(snf);

	/* Prepare for custom write interrupt */
	nfi_write32(snf, NFI_INTR_EN, NFI_IRQ_INTR_EN | NFI_IRQ_CUS_PG);
	irq_completion_init(snf->pdev);

	/* Trigger NFI into custom mode */
	nfi_write16(snf, NFI_CMD, NFI_CMD_DUMMY_WRITE);

	/* Start DMA write */
	nfi_rmw32(snf, NFI_CON, 0, CON_BWR);
	nfi_write16(snf, NFI_STRDATA, STR_DATA);

	/* Wait for operation finished */
	ret = irq_completion_wait(snf->pdev, snf->nfi_base + SNF_STA_CTL1,
				  CUS_PG_DONE, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "DMA timed out for program load\n");
		goto cleanup;
	}

	/* Wait for NFI_SEC_CNTR returning expected value */
	ret = read32_poll_timeout(snf->nfi_base + NFI_ADDRCNTR, val,
				  NFI_SEC_CNTR(val) >= snf->ecc_steps,
				  0, SNFI_POLL_INTERVAL);
	if (ret) {
		snand_log_nfi(snf->pdev,
			      "Timed out waiting for NFI_SEC_CNTR\n");
		goto cleanup;
	}

	if (!raw)
		mtk_snand_ecc_encoder_stop(snf);

cleanup:
	/* DMA cleanup */
	dma_mem_unmap(snf->pdev, dma_addr, len, true);

	/* Stop write */
	nfi_write32(snf, NFI_CON, 0);
	nfi_write16(snf, NFI_CNFG, 0);

	/* Clear SNF done flag */
	nfi_rmw32(snf, SNF_STA_CTL1, 0, CUS_PG_DONE);
	nfi_write32(snf, SNF_STA_CTL1, 0);

	/* Disable interrupt */
	nfi_read32(snf, NFI_INTR_STA);
	nfi_write32(snf, NFI_INTR_EN, 0);

	nfi_rmw32(snf, SNF_MISC_CTL, PG_LOAD_CUSTOM_EN, 0);

	return ret;
}

static void mtk_snand_to_raw_page(struct mtk_snand *snf,
				  const void *buf, const void *oob,
				  bool empty_ecc)
{
	uint32_t i, ecc_bytes = snf->spare_per_sector - snf->nfi_soc->fdm_size;
	const uint8_t *eccptr = oob + snf->ecc_steps * snf->nfi_soc->fdm_size;
	const uint8_t *bufptr = buf, *oobptr = oob;
	uint8_t *raw_sector;

	memset(snf->page_cache, 0xff, snf->writesize + snf->oobsize);
	for (i = 0; i < snf->ecc_steps; i++) {
		raw_sector = snf->page_cache + i * snf->raw_sector_size;

		if (buf) {
			memcpy(raw_sector, bufptr, snf->nfi_soc->sector_size);
			bufptr += snf->nfi_soc->sector_size;
		}

		raw_sector += snf->nfi_soc->sector_size;

		if (oob) {
			memcpy(raw_sector, oobptr, snf->nfi_soc->fdm_size);
			oobptr += snf->nfi_soc->fdm_size;
			raw_sector += snf->nfi_soc->fdm_size;

			if (empty_ecc)
				memset(raw_sector, 0xff, ecc_bytes);
			else
				memcpy(raw_sector, eccptr, ecc_bytes);
			eccptr += ecc_bytes;
		}
	}
}

static bool mtk_snand_is_empty_page(struct mtk_snand *snf, const void *buf,
				    const void *oob)
{
	const uint8_t *p = buf;
	uint32_t i, j;

	if (buf) {
		for (i = 0; i < snf->writesize; i++) {
			if (p[i] != 0xff)
				return false;
		}
	}

	if (oob) {
		for (j = 0; j < snf->ecc_steps; j++) {
			p = oob + j * snf->nfi_soc->fdm_size;

			for (i = 0; i < snf->nfi_soc->fdm_ecc_size; i++) {
				if (p[i] != 0xff)
					return false;
			}
		}
	}

	return true;
}

static int mtk_snand_do_write_page(struct mtk_snand *snf, uint64_t addr,
				   const void *buf, const void *oob,
				   bool raw, bool format)
{
	uint64_t die_addr;
	bool empty_ecc = false;
	uint32_t page;
	int ret;

	die_addr = mtk_snand_select_die_address(snf, addr);
	page = die_addr >> snf->writesize_shift;

	if (!raw && mtk_snand_is_empty_page(snf, buf, oob)) {
		/*
		 * If the data in the page to be ecc-ed is full 0xff,
		 * change to raw write mode
		 */
		raw = true;
		format = true;

		/* fill ecc parity code region with 0xff */
		empty_ecc = true;
	}

	if (raw) {
		if (format) {
			mtk_snand_to_raw_page(snf, buf, oob, empty_ecc);
			mtk_snand_fdm_bm_swap_raw(snf);
			mtk_snand_bm_swap_raw(snf);
		} else {
			memset(snf->page_cache, 0xff,
			       snf->writesize + snf->oobsize);

			if (buf)
				memcpy(snf->page_cache, buf, snf->writesize);

			if (oob) {
				memcpy(snf->page_cache + snf->writesize, oob,
				       snf->ecc_steps * snf->spare_per_sector);
			}
		}
	} else {
		memset(snf->page_cache, 0xff, snf->writesize + snf->oobsize);
		if (buf)
			memcpy(snf->page_cache, buf, snf->writesize);

		if (oob) {
			memcpy(snf->page_cache + snf->writesize, oob,
			       snf->ecc_steps * snf->nfi_soc->fdm_size);
		}

		mtk_snand_fdm_bm_swap(snf);
		mtk_snand_bm_swap(snf);
	}

	ret = mtk_snand_write_enable(snf);
	if (ret)
		return ret;

	ret = mtk_snand_program_load(snf, page, raw);
	if (ret)
		return ret;

	ret = mtk_snand_page_op(snf, page, SNAND_CMD_PROGRAM_EXECUTE);
	if (ret)
		return ret;

	ret = mtk_snand_poll_status(snf, SNFI_POLL_INTERVAL);
	if (ret < 0) {
		snand_log_chip(snf->pdev,
			       "Page program command timed out on page %u\n",
			       page);
		return ret;
	}

	if (ret & SNAND_STATUS_PROGRAM_FAIL) {
		snand_log_chip(snf->pdev,
			       "Page program failed on page %u\n", page);
		return -EIO;
	}

	return 0;
}

int mtk_snand_write_page(struct mtk_snand *snf, uint64_t addr, const void *buf,
			 const void *oob, bool raw)
{
	if (!snf || (!buf && !oob))
		return -EINVAL;

	if (addr >= snf->size)
		return -EINVAL;

	return mtk_snand_do_write_page(snf, addr, buf, oob, raw, true);
}

int mtk_snand_erase_block(struct mtk_snand *snf, uint64_t addr)
{
	uint64_t die_addr;
	uint32_t page, block;
	int ret;

	if (!snf)
		return -EINVAL;

	if (addr >= snf->size)
		return -EINVAL;

	die_addr = mtk_snand_select_die_address(snf, addr);
	block = die_addr >> snf->erasesize_shift;
	page = block << (snf->erasesize_shift - snf->writesize_shift);

	ret = mtk_snand_write_enable(snf);
	if (ret)
		return ret;

	ret = mtk_snand_page_op(snf, page, SNAND_CMD_BLOCK_ERASE);
	if (ret)
		return ret;

	ret = mtk_snand_poll_status(snf, SNFI_POLL_INTERVAL);
	if (ret < 0) {
		snand_log_chip(snf->pdev,
			       "Block erase command timed out on block %u\n",
			       block);
		return ret;
	}

	if (ret & SNAND_STATUS_ERASE_FAIL) {
		snand_log_chip(snf->pdev,
			       "Block erase failed on block %u\n", block);
		return -EIO;
	}

	return 0;
}

static int mtk_snand_block_isbad_std(struct mtk_snand *snf, uint64_t addr)
{
	int ret;

	ret = mtk_snand_do_read_page(snf, addr, NULL, snf->buf_cache, true,
				     false);
	if (ret && ret != -EBADMSG)
		return ret;

	return snf->buf_cache[0] != 0xff;
}

static int mtk_snand_block_isbad_mtk(struct mtk_snand *snf, uint64_t addr)
{
	int ret;

	ret = mtk_snand_do_read_page(snf, addr, NULL, snf->buf_cache, true,
				     true);
	if (ret && ret != -EBADMSG)
		return ret;

	return snf->buf_cache[0] != 0xff;
}

int mtk_snand_block_isbad(struct mtk_snand *snf, uint64_t addr)
{
	if (!snf)
		return -EINVAL;

	if (addr >= snf->size)
		return -EINVAL;

	addr &= ~snf->erasesize_mask;

	if (snf->nfi_soc->bbm_swap)
		return mtk_snand_block_isbad_std(snf, addr);

	return mtk_snand_block_isbad_mtk(snf, addr);
}

static int mtk_snand_block_markbad_std(struct mtk_snand *snf, uint64_t addr)
{
	/* Standard BBM position */
	memset(snf->buf_cache, 0xff, snf->oobsize);
	snf->buf_cache[0] = 0;

	return mtk_snand_do_write_page(snf, addr, NULL, snf->buf_cache, true,
				       false);
}

static int mtk_snand_block_markbad_mtk(struct mtk_snand *snf, uint64_t addr)
{
	/* Write the whole page with zeros */
	memset(snf->buf_cache, 0, snf->writesize + snf->oobsize);

	return mtk_snand_do_write_page(snf, addr, snf->buf_cache,
				       snf->buf_cache + snf->writesize, true,
				       true);
}

int mtk_snand_block_markbad(struct mtk_snand *snf, uint64_t addr)
{
	if (!snf)
		return -EINVAL;

	if (addr >= snf->size)
		return -EINVAL;

	addr &= ~snf->erasesize_mask;

	if (snf->nfi_soc->bbm_swap)
		return mtk_snand_block_markbad_std(snf, addr);

	return mtk_snand_block_markbad_mtk(snf, addr);
}

int mtk_snand_fill_oob(struct mtk_snand *snf, uint8_t *oobraw,
		       const uint8_t *oobbuf, size_t ooblen)
{
	size_t len = ooblen, sect_fdm_len;
	const uint8_t *oob = oobbuf;
	uint32_t step = 0;

	if (!snf || !oobraw || !oob)
		return -EINVAL;

	while (len && step < snf->ecc_steps) {
		sect_fdm_len = snf->nfi_soc->fdm_size - 1;
		if (sect_fdm_len > len)
			sect_fdm_len = len;

		memcpy(oobraw + step * snf->nfi_soc->fdm_size + 1, oob,
		       sect_fdm_len);

		len -= sect_fdm_len;
		oob += sect_fdm_len;
		step++;
	}

	return len;
}

int mtk_snand_transfer_oob(struct mtk_snand *snf, uint8_t *oobbuf,
			   size_t ooblen, const uint8_t *oobraw)
{
	size_t len = ooblen, sect_fdm_len;
	uint8_t *oob = oobbuf;
	uint32_t step = 0;

	if (!snf || !oobraw || !oob)
		return -EINVAL;

	while (len && step < snf->ecc_steps) {
		sect_fdm_len = snf->nfi_soc->fdm_size - 1;
		if (sect_fdm_len > len)
			sect_fdm_len = len;

		memcpy(oob, oobraw + step * snf->nfi_soc->fdm_size + 1,
		       sect_fdm_len);

		len -= sect_fdm_len;
		oob += sect_fdm_len;
		step++;
	}

	return len;
}

int mtk_snand_read_page_auto_oob(struct mtk_snand *snf, uint64_t addr,
				 void *buf, void *oob, size_t ooblen,
				 size_t *actualooblen, bool raw)
{
	int ret, oobremain;

	if (!snf)
		return -EINVAL;

	if (!oob)
		return mtk_snand_read_page(snf, addr, buf, NULL, raw);

	ret = mtk_snand_read_page(snf, addr, buf, snf->buf_cache, raw);
	if (ret && ret != -EBADMSG) {
		if (actualooblen)
			*actualooblen = 0;
		return ret;
	}

	oobremain = mtk_snand_transfer_oob(snf, oob, ooblen, snf->buf_cache);
	if (actualooblen)
		*actualooblen = ooblen - oobremain;

	return ret;
}

int mtk_snand_write_page_auto_oob(struct mtk_snand *snf, uint64_t addr,
				  const void *buf, const void *oob,
				  size_t ooblen, size_t *actualooblen, bool raw)
{
	int oobremain;

	if (!snf)
		return -EINVAL;

	if (!oob)
		return mtk_snand_write_page(snf, addr, buf, NULL, raw);

	memset(snf->buf_cache, 0xff, snf->oobsize);
	oobremain = mtk_snand_fill_oob(snf, snf->buf_cache, oob, ooblen);
	if (actualooblen)
		*actualooblen = ooblen - oobremain;

	return mtk_snand_write_page(snf, addr, buf, snf->buf_cache, raw);
}

int mtk_snand_get_chip_info(struct mtk_snand *snf,
			    struct mtk_snand_chip_info *info)
{
	if (!snf || !info)
		return -EINVAL;

	info->model = snf->model;
	info->chipsize = snf->size;
	info->blocksize = snf->erasesize;
	info->pagesize = snf->writesize;
	info->sparesize = snf->oobsize;
	info->spare_per_sector = snf->spare_per_sector;
	info->fdm_size = snf->nfi_soc->fdm_size;
	info->fdm_ecc_size = snf->nfi_soc->fdm_ecc_size;
	info->num_sectors = snf->ecc_steps;
	info->sector_size = snf->nfi_soc->sector_size;
	info->ecc_strength = snf->ecc_strength;
	info->ecc_bytes = snf->ecc_bytes;

	return 0;
}

int mtk_snand_irq_process(struct mtk_snand *snf)
{
	uint32_t sta, ien;

	if (!snf)
		return -EINVAL;

	sta = nfi_read32(snf, NFI_INTR_STA);
	ien = nfi_read32(snf, NFI_INTR_EN);

	if (!(sta & ien))
		return 0;

	nfi_write32(snf, NFI_INTR_EN, 0);
	irq_completion_done(snf->pdev);

	return 1;
}

static int mtk_snand_select_spare_per_sector(struct mtk_snand *snf)
{
	uint32_t spare_per_step = snf->oobsize / snf->ecc_steps;
	int i, mul = 1;

	/*
	 * If we're using the 1KB sector size, HW will automatically
	 * double the spare size. So we should only use half of the value.
	 */
	if (snf->nfi_soc->sector_size == 1024)
		mul = 2;

	spare_per_step /= mul;

	for (i = snf->nfi_soc->num_spare_size - 1; i >= 0; i--) {
		if (snf->nfi_soc->spare_sizes[i] <= spare_per_step) {
			snf->spare_per_sector = snf->nfi_soc->spare_sizes[i];
			snf->spare_per_sector *= mul;
			return i;
		}
	}

	snand_log_nfi(snf->pdev,
		      "Page size %u+%u is not supported\n", snf->writesize,
		      snf->oobsize);

	return -1;
}

static int mtk_snand_pagefmt_setup(struct mtk_snand *snf)
{
	uint32_t spare_size_idx, spare_size_shift, pagesize_idx;
	uint32_t sector_size_512;

	if (snf->nfi_soc->sector_size == 512) {
		sector_size_512 = NFI_SEC_SEL_512;
		spare_size_shift = NFI_SPARE_SIZE_S;
	} else {
		sector_size_512 = 0;
		spare_size_shift = NFI_SPARE_SIZE_LS_S;
	}

	switch (snf->writesize) {
	case SZ_512:
		pagesize_idx = NFI_PAGE_SIZE_512_2K;
		break;
	case SZ_2K:
		if (snf->nfi_soc->sector_size == 512)
			pagesize_idx = NFI_PAGE_SIZE_2K_4K;
		else
			pagesize_idx = NFI_PAGE_SIZE_512_2K;
		break;
	case SZ_4K:
		if (snf->nfi_soc->sector_size == 512)
			pagesize_idx = NFI_PAGE_SIZE_4K_8K;
		else
			pagesize_idx = NFI_PAGE_SIZE_2K_4K;
		break;
	case SZ_8K:
		if (snf->nfi_soc->sector_size == 512)
			pagesize_idx = NFI_PAGE_SIZE_8K_16K;
		else
			pagesize_idx = NFI_PAGE_SIZE_4K_8K;
		break;
	case SZ_16K:
		pagesize_idx = NFI_PAGE_SIZE_8K_16K;
		break;
	default:
		snand_log_nfi(snf->pdev, "Page size %u is not supported\n",
			      snf->writesize);
		return -ENOTSUPP;
	}

	spare_size_idx = mtk_snand_select_spare_per_sector(snf);
	if (unlikely(spare_size_idx < 0))
		return -ENOTSUPP;

	snf->raw_sector_size = snf->nfi_soc->sector_size +
			       snf->spare_per_sector;

	/* Setup page format */
	nfi_write32(snf, NFI_PAGEFMT,
		    (snf->nfi_soc->fdm_ecc_size << NFI_FDM_ECC_NUM_S) |
		    (snf->nfi_soc->fdm_size << NFI_FDM_NUM_S) |
		    (spare_size_idx << spare_size_shift) |
		    (pagesize_idx << NFI_PAGE_SIZE_S) |
		    sector_size_512);

	return 0;
}

static enum snand_flash_io mtk_snand_select_opcode(struct mtk_snand *snf,
				   uint32_t snfi_caps, uint8_t *opcode,
				   uint8_t *dummy,
				   const struct snand_io_cap *op_cap)
{
	uint32_t i, caps;

	caps = snfi_caps & op_cap->caps;

	i = fls(caps);
	if (i > 0) {
		*opcode = op_cap->opcodes[i - 1].opcode;
		if (dummy)
			*dummy = op_cap->opcodes[i - 1].dummy;
		return i - 1;
	}

	return __SNAND_IO_MAX;
}

static int mtk_snand_select_opcode_rfc(struct mtk_snand *snf,
				       uint32_t snfi_caps,
				       const struct snand_io_cap *op_cap)
{
	enum snand_flash_io idx;

	static const uint8_t rfc_modes[__SNAND_IO_MAX] = {
		[SNAND_IO_1_1_1] = DATA_READ_MODE_X1,
		[SNAND_IO_1_1_2] = DATA_READ_MODE_X2,
		[SNAND_IO_1_2_2] = DATA_READ_MODE_DUAL,
		[SNAND_IO_1_1_4] = DATA_READ_MODE_X4,
		[SNAND_IO_1_4_4] = DATA_READ_MODE_QUAD,
	};

	idx = mtk_snand_select_opcode(snf, snfi_caps, &snf->opcode_rfc,
				      &snf->dummy_rfc, op_cap);
	if (idx >= __SNAND_IO_MAX) {
		snand_log_snfi(snf->pdev,
			       "No capable opcode for read from cache\n");
		return -ENOTSUPP;
	}

	snf->mode_rfc = rfc_modes[idx];

	if (idx == SNAND_IO_1_1_4 || idx == SNAND_IO_1_4_4)
		snf->quad_spi_op = true;

	return 0;
}

static int mtk_snand_select_opcode_pl(struct mtk_snand *snf, uint32_t snfi_caps,
				      const struct snand_io_cap *op_cap)
{
	enum snand_flash_io idx;

	static const uint8_t pl_modes[__SNAND_IO_MAX] = {
		[SNAND_IO_1_1_1] = 0,
		[SNAND_IO_1_1_4] = 1,
	};

	idx = mtk_snand_select_opcode(snf, snfi_caps, &snf->opcode_pl,
				      NULL, op_cap);
	if (idx >= __SNAND_IO_MAX) {
		snand_log_snfi(snf->pdev,
			       "No capable opcode for program load\n");
		return -ENOTSUPP;
	}

	snf->mode_pl = pl_modes[idx];

	if (idx == SNAND_IO_1_1_4)
		snf->quad_spi_op = true;

	return 0;
}

static int mtk_snand_setup(struct mtk_snand *snf,
			   const struct snand_flash_info *snand_info)
{
	const struct snand_mem_org *memorg = &snand_info->memorg;
	uint32_t i, msg_size, snfi_caps;
	int ret;

	/* Calculate flash memory organization */
	snf->model = snand_info->model;
	snf->writesize = memorg->pagesize;
	snf->oobsize = memorg->sparesize;
	snf->erasesize = snf->writesize * memorg->pages_per_block;
	snf->die_size = (uint64_t)snf->erasesize * memorg->blocks_per_die;
	snf->size = snf->die_size * memorg->ndies;
	snf->num_dies = memorg->ndies;

	snf->writesize_mask = snf->writesize - 1;
	snf->erasesize_mask = snf->erasesize - 1;
	snf->die_mask = snf->die_size - 1;

	snf->writesize_shift = ffs(snf->writesize) - 1;
	snf->erasesize_shift = ffs(snf->erasesize) - 1;
	snf->die_shift = mtk_snand_ffs64(snf->die_size) - 1;

	snf->select_die = snand_info->select_die;

	/* Determine opcodes for read from cache/program load */
	snfi_caps = SPI_IO_1_1_1 | SPI_IO_1_1_2 | SPI_IO_1_2_2;
	if (snf->snfi_quad_spi)
		snfi_caps |= SPI_IO_1_1_4 | SPI_IO_1_4_4;

	ret = mtk_snand_select_opcode_rfc(snf, snfi_caps, snand_info->cap_rd);
	if (ret)
		return ret;

	ret = mtk_snand_select_opcode_pl(snf, snfi_caps, snand_info->cap_pl);
	if (ret)
		return ret;

	/* ECC and page format */
	snf->ecc_steps = snf->writesize / snf->nfi_soc->sector_size;
	if (snf->ecc_steps > snf->nfi_soc->max_sectors) {
		snand_log_nfi(snf->pdev, "Page size %u is not supported\n",
			      snf->writesize);
		return -ENOTSUPP;
	}

	ret = mtk_snand_pagefmt_setup(snf);
	if (ret)
		return ret;

	msg_size = snf->nfi_soc->sector_size + snf->nfi_soc->fdm_ecc_size;
	ret = mtk_ecc_setup(snf, snf->nfi_base + NFI_FDM0L,
			    snf->spare_per_sector - snf->nfi_soc->fdm_size,
			    msg_size);
	if (ret)
		return ret;

	nfi_write16(snf, NFI_CNFG, 0);

	/* Tuning options */
	nfi_write16(snf, NFI_DEBUG_CON1, WBUF_EN);
	nfi_write32(snf, SNF_DLY_CTL3, (40 << SFCK_SAM_DLY_S));

	/* Interrupts */
	nfi_read32(snf, NFI_INTR_STA);
	nfi_write32(snf, NFI_INTR_EN, 0);

	/* Clear SNF done flag */
	nfi_rmw32(snf, SNF_STA_CTL1, 0, CUS_READ_DONE | CUS_PG_DONE);
	nfi_write32(snf, SNF_STA_CTL1, 0);

	/* Initialization on all dies */
	for (i = 0; i < snf->num_dies; i++) {
		mtk_snand_select_die(snf, i);

		/* Disable On-Die ECC engine */
		ret = mtk_snand_ondie_ecc_control(snf, false);
		if (ret)
			return ret;

		/* Disable block protection */
		mtk_snand_unlock(snf);

		/* Enable/disable quad-spi */
		mtk_snand_qspi_control(snf, snf->quad_spi_op);
	}

	mtk_snand_select_die(snf, 0);

	return 0;
}

static int mtk_snand_id_probe(struct mtk_snand *snf,
			      const struct snand_flash_info **snand_info)
{
	uint8_t id[4], op[2];
	int ret;

	/* Read SPI-NAND JEDEC ID, OP + dummy/addr + ID */
	op[0] = SNAND_CMD_READID;
	op[1] = 0;
	ret = mtk_snand_mac_io(snf, op, 2, id, sizeof(id));
	if (ret)
		return ret;

	*snand_info = snand_flash_id_lookup(SNAND_ID_DYMMY, id);
	if (*snand_info)
		return 0;

	/* Read SPI-NAND JEDEC ID, OP + ID */
	op[0] = SNAND_CMD_READID;
	ret = mtk_snand_mac_io(snf, op, 1, id, sizeof(id));
	if (ret)
		return ret;

	*snand_info = snand_flash_id_lookup(SNAND_ID_DYMMY, id);
	if (*snand_info)
		return 0;

	snand_log_chip(snf->pdev,
		       "Unrecognized SPI-NAND ID: %02x %02x %02x %02x\n",
		       id[0], id[1], id[2], id[3]);

	return -EINVAL;
}

int mtk_snand_init(void *dev, const struct mtk_snand_platdata *pdata,
		   struct mtk_snand **psnf)
{
	const struct snand_flash_info *snand_info;
	uint32_t rawpage_size, sect_bf_size;
	struct mtk_snand tmpsnf, *snf;
	int ret;

	if (!pdata || !psnf)
		return -EINVAL;

	if (pdata->soc >= __SNAND_SOC_MAX) {
		snand_log_chip(dev, "Invalid SOC %u for MTK-SNAND\n",
			       pdata->soc);
		return -EINVAL;
	}

	/* Dummy instance only for initial reset and id probe */
	tmpsnf.nfi_base = pdata->nfi_base;
	tmpsnf.ecc_base = pdata->ecc_base;
	tmpsnf.soc = pdata->soc;
	tmpsnf.nfi_soc = &mtk_snand_socs[pdata->soc];
	tmpsnf.pdev = dev;

	/* Switch to SNFI mode */
	writel(SPI_MODE, tmpsnf.nfi_base + SNF_CFG);

	/* Reset SNFI & NFI */
	mtk_snand_mac_reset(&tmpsnf);
	mtk_nfi_reset(&tmpsnf);

	/* Reset SPI-NAND chip */
	ret = mtk_snand_chip_reset(&tmpsnf);
	if (ret) {
		snand_log_chip(dev, "Failed to reset SPI-NAND chip\n");
		return ret;
	}

	/* Probe SPI-NAND flash by JEDEC ID */
	ret = mtk_snand_id_probe(&tmpsnf, &snand_info);
	if (ret)
		return ret;

	rawpage_size = snand_info->memorg.pagesize +
		       snand_info->memorg.sparesize;

	sect_bf_size = mtk_snand_socs[pdata->soc].max_sectors *
		       sizeof(*snf->sect_bf);

	/* Allocate memory for instance and cache */
	snf = generic_mem_alloc(dev,
				sizeof(*snf) + rawpage_size + sect_bf_size);
	if (!snf) {
		snand_log_chip(dev, "Failed to allocate memory for instance\n");
		return -ENOMEM;
	}

	snf->sect_bf = (int *)((uintptr_t)snf + sizeof(*snf));
	snf->buf_cache = (uint8_t *)((uintptr_t)snf->sect_bf + sect_bf_size);

	/* Allocate memory for DMA buffer */
	snf->page_cache = dma_mem_alloc(dev, rawpage_size);
	if (!snf->page_cache) {
		generic_mem_free(dev, snf);
		snand_log_chip(dev,
			       "Failed to allocate memory for DMA buffer\n");
		return -ENOMEM;
	}

	/* Fill up instance */
	snf->pdev = dev;
	snf->nfi_base = pdata->nfi_base;
	snf->ecc_base = pdata->ecc_base;
	snf->soc = pdata->soc;
	snf->nfi_soc = &mtk_snand_socs[pdata->soc];
	snf->snfi_quad_spi = pdata->quad_spi;

	/* Initialize SNFI & ECC engine */
	ret = mtk_snand_setup(snf, snand_info);
	if (ret) {
		dma_mem_free(dev, snf->page_cache);
		generic_mem_free(dev, snf);
		return ret;
	}

	*psnf = snf;

	return 0;
}

int mtk_snand_cleanup(struct mtk_snand *snf)
{
	if (!snf)
		return 0;

	dma_mem_free(snf->pdev, snf->page_cache);
	generic_mem_free(snf->pdev, snf);

	return 0;
}
