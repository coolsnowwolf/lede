/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Xiangsheng Hou <xiangsheng.hou@mediatek.com>
 * Copyright (c) 2020 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/mtk_bmt.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/bits.h>

#define MAIN_SIGNATURE_OFFSET   0
#define OOB_SIGNATURE_OFFSET    1
#define BBPOOL_RATIO		2

#define BBT_LOG(fmt, ...) pr_debug("[BBT][%s|%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

/* Maximum 8k blocks */
#define BB_TABLE_MAX	bmtd.table_size
#define BMT_TABLE_MAX	(BB_TABLE_MAX * BBPOOL_RATIO / 100)
#define BMT_TBL_DEF_VAL	0x0

struct mtk_bmt_ops {
	char *sig;
	unsigned int sig_len;
	int (*init)(struct device_node *np);
	bool (*remap_block)(u16 block, u16 mapped_block, int copy_len);
	void (*unmap_block)(u16 block);
	u16 (*get_mapping_block)(int block);
	int (*debug)(void *data, u64 val);
};

struct bbbt {
	char signature[3];
	/* This version is used to distinguish the legacy and new algorithm */
#define BBMT_VERSION		2
	unsigned char version;
	/* Below 2 tables will be written in SLC */
	u16 bb_tbl[];
};

struct bbmt {
	u16 block;
#define NO_MAPPED		0
#define NORMAL_MAPPED	1
#define BMT_MAPPED		2
	u16 mapped;
};

static struct bmt_desc {
	struct mtd_info *mtd;

	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			   struct mtd_oob_ops *ops);
	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);

	const struct mtk_bmt_ops *ops;

	struct bbbt *bbt;

	struct dentry *debugfs_dir;

	u32 table_size;
	u32 pg_size;
	u32 blk_size;
	u16 pg_shift;
	u16 blk_shift;
	/* bbt logical address */
	u16 pool_lba;
	/* bbt physical address */
	u16 pool_pba;
	/* Maximum count of bad blocks that the vendor guaranteed */
	u16 bb_max;
	/* Total blocks of the Nand Chip */
	u16 total_blks;
	/* The block(n) BMT is located at (bmt_tbl[n]) */
	u16 bmt_blk_idx;
	/* How many pages needs to store 'struct bbbt' */
	u32 bmt_pgs;

	const __be32 *remap_range;
	int remap_range_len;

	/* to compensate for driver level remapping */
	u8 oob_offset;
} bmtd = {0};

static unsigned char *nand_bbt_buf;
static unsigned char *nand_data_buf;

/* -------- Unit conversions -------- */
static inline u32 blk_pg(u16 block)
{
	return (u32)(block << (bmtd.blk_shift - bmtd.pg_shift));
}

/* -------- Nand operations wrapper -------- */
static inline int
bbt_nand_read(u32 page, unsigned char *dat, int dat_len,
	      unsigned char *fdm, int fdm_len)
{
	struct mtd_oob_ops ops = {
		.mode = MTD_OPS_PLACE_OOB,
		.ooboffs = bmtd.oob_offset,
		.oobbuf = fdm,
		.ooblen = fdm_len,
		.datbuf = dat,
		.len = dat_len,
	};

	return bmtd._read_oob(bmtd.mtd, page << bmtd.pg_shift, &ops);
}

static inline int bbt_nand_erase(u16 block)
{
	struct mtd_info *mtd = bmtd.mtd;
	struct erase_info instr = {
		.addr = (loff_t)block << bmtd.blk_shift,
		.len = bmtd.blk_size,
	};

	return bmtd._erase(mtd, &instr);
}

static inline int bbt_nand_copy(u16 dest_blk, u16 src_blk, loff_t max_offset)
{
	int pages = bmtd.blk_size >> bmtd.pg_shift;
	loff_t src = (loff_t)src_blk << bmtd.blk_shift;
	loff_t dest = (loff_t)dest_blk << bmtd.blk_shift;
	loff_t offset = 0;
	uint8_t oob[64];
	int i, ret;

	for (i = 0; i < pages; i++) {
		struct mtd_oob_ops rd_ops = {
			.mode = MTD_OPS_PLACE_OOB,
			.oobbuf = oob,
			.ooblen = min_t(int, bmtd.mtd->oobsize / pages, sizeof(oob)),
			.datbuf = nand_data_buf,
			.len = bmtd.pg_size,
		};
		struct mtd_oob_ops wr_ops = {
			.mode = MTD_OPS_PLACE_OOB,
			.oobbuf = oob,
			.datbuf = nand_data_buf,
			.len = bmtd.pg_size,
		};

		if (offset >= max_offset)
			break;

		ret = bmtd._read_oob(bmtd.mtd, src + offset, &rd_ops);
		if (ret < 0 && !mtd_is_bitflip(ret))
			return ret;

		if (!rd_ops.retlen)
			break;

		ret = bmtd._write_oob(bmtd.mtd, dest + offset, &wr_ops);
		if (ret < 0)
			return ret;

		wr_ops.ooblen = rd_ops.oobretlen;
		offset += rd_ops.retlen;
	}

	return 0;
}

/* -------- Bad Blocks Management -------- */
static inline struct bbmt *bmt_tbl(struct bbbt *bbbt)
{
	return (struct bbmt *)&bbbt->bb_tbl[bmtd.table_size];
}

static int
read_bmt(u16 block, unsigned char *dat, unsigned char *fdm, int fdm_len)
{
	u32 len = bmtd.bmt_pgs << bmtd.pg_shift;

	return bbt_nand_read(blk_pg(block), dat, len, fdm, fdm_len);
}

static int write_bmt(u16 block, unsigned char *dat)
{
	struct mtd_oob_ops ops = {
		.mode = MTD_OPS_PLACE_OOB,
		.ooboffs = OOB_SIGNATURE_OFFSET + bmtd.oob_offset,
		.oobbuf = bmtd.ops->sig,
		.ooblen = bmtd.ops->sig_len,
		.datbuf = dat,
		.len = bmtd.bmt_pgs << bmtd.pg_shift,
	};
	loff_t addr = (loff_t)block << bmtd.blk_shift;

	return bmtd._write_oob(bmtd.mtd, addr, &ops);
}

static u16 find_valid_block(u16 block)
{
	u8 fdm[4];
	int ret;
	int loop = 0;

retry:
	if (block >= bmtd.total_blks)
		return 0;

	ret = bbt_nand_read(blk_pg(block), nand_data_buf, bmtd.pg_size,
			    fdm, sizeof(fdm));
	/* Read the 1st byte of FDM to judge whether it's a bad
	 * or not
	 */
	if (ret || fdm[0] != 0xff) {
		pr_info("nand: found bad block 0x%x\n", block);
		if (loop >= bmtd.bb_max) {
			pr_info("nand: FATAL ERR: too many bad blocks!!\n");
			return 0;
		}

		loop++;
		block++;
		goto retry;
	}

	return block;
}

/* Find out all bad blocks, and fill in the mapping table */
static int scan_bad_blocks(struct bbbt *bbt)
{
	int i;
	u16 block = 0;

	/* First time download, the block0 MUST NOT be a bad block,
	 * this is guaranteed by vendor
	 */
	bbt->bb_tbl[0] = 0;

	/*
	 * Construct the mapping table of Normal data area(non-PMT/BMTPOOL)
	 * G - Good block; B - Bad block
	 *			---------------------------
	 * physical |G|G|B|G|B|B|G|G|G|G|B|G|B|
	 *			---------------------------
	 * What bb_tbl[i] looks like:
	 *   physical block(i):
	 *			 0 1 2 3 4 5 6 7 8 9 a b c
	 *   mapped block(bb_tbl[i]):
	 *			 0 1 3 6 7 8 9 b ......
	 * ATTENTION:
	 *		If new bad block ocurred(n), search bmt_tbl to find
	 *		a available block(x), and fill in the bb_tbl[n] = x;
	 */
	for (i = 1; i < bmtd.pool_lba; i++) {
		bbt->bb_tbl[i] = find_valid_block(bbt->bb_tbl[i - 1] + 1);
		BBT_LOG("bb_tbl[0x%x] = 0x%x", i, bbt->bb_tbl[i]);
		if (bbt->bb_tbl[i] == 0)
			return -1;
	}

	/* Physical Block start Address of BMT pool */
	bmtd.pool_pba = bbt->bb_tbl[i - 1] + 1;
	if (bmtd.pool_pba >= bmtd.total_blks - 2) {
		pr_info("nand: FATAL ERR: Too many bad blocks!!\n");
		return -1;
	}

	BBT_LOG("pool_pba=0x%x", bmtd.pool_pba);
	i = 0;
	block = bmtd.pool_pba;
	/*
	 * The bmt table is used for runtime bad block mapping
	 * G - Good block; B - Bad block
	 *			---------------------------
	 * physical |G|G|B|G|B|B|G|G|G|G|B|G|B|
	 *			---------------------------
	 *   block:	 0 1 2 3 4 5 6 7 8 9 a b c
	 * What bmt_tbl[i] looks like in initial state:
	 *   i:
	 *			 0 1 2 3 4 5 6 7
	 *   bmt_tbl[i].block:
	 *			 0 1 3 6 7 8 9 b
	 *   bmt_tbl[i].mapped:
	 *			 N N N N N N N B
	 *		N - Not mapped(Available)
	 *		M - Mapped
	 *		B - BMT
	 * ATTENTION:
	 *		BMT always in the last valid block in pool
	 */
	while ((block = find_valid_block(block)) != 0) {
		bmt_tbl(bbt)[i].block = block;
		bmt_tbl(bbt)[i].mapped = NO_MAPPED;
		BBT_LOG("bmt_tbl[%d].block = 0x%x", i, block);
		block++;
		i++;
	}

	/* i - How many available blocks in pool, which is the length of bmt_tbl[]
	 * bmtd.bmt_blk_idx - bmt_tbl[bmtd.bmt_blk_idx].block => the BMT block
	 */
	bmtd.bmt_blk_idx = i - 1;
	bmt_tbl(bbt)[bmtd.bmt_blk_idx].mapped = BMT_MAPPED;

	if (i < 1) {
		pr_info("nand: FATAL ERR: no space to store BMT!!\n");
		return -1;
	}

	pr_info("[BBT] %d available blocks in BMT pool\n", i);

	return 0;
}

static bool is_valid_bmt(unsigned char *buf, unsigned char *fdm)
{
	struct bbbt *bbt = (struct bbbt *)buf;
	u8 *sig = (u8*)bbt->signature + MAIN_SIGNATURE_OFFSET;


	if (memcmp(bbt->signature + MAIN_SIGNATURE_OFFSET, "BMT", 3) == 0 &&
		memcmp(fdm + OOB_SIGNATURE_OFFSET, "bmt", 3) == 0) {
		if (bbt->version == BBMT_VERSION)
			return true;
	}
	BBT_LOG("[BBT] BMT Version not match,upgrage preloader and uboot please! sig=%02x%02x%02x, fdm=%02x%02x%02x",
		sig[0], sig[1], sig[2],
		fdm[1], fdm[2], fdm[3]);
	return false;
}

static u16 get_bmt_index(struct bbmt *bmt)
{
	int i = 0;

	while (bmt[i].block != BMT_TBL_DEF_VAL) {
		if (bmt[i].mapped == BMT_MAPPED)
			return i;
		i++;
	}
	return 0;
}

static struct bbbt *scan_bmt(u16 block)
{
	u8 fdm[4];

	if (block < bmtd.pool_lba)
		return NULL;

	if (read_bmt(block, nand_bbt_buf, fdm, sizeof(fdm)))
		return scan_bmt(block - 1);

	if (is_valid_bmt(nand_bbt_buf, fdm)) {
		bmtd.bmt_blk_idx = get_bmt_index(bmt_tbl((struct bbbt *)nand_bbt_buf));
		if (bmtd.bmt_blk_idx == 0) {
			pr_info("[BBT] FATAL ERR: bmt block index is wrong!\n");
			return NULL;
		}
		pr_info("[BBT] BMT.v2 is found at 0x%x\n", block);
		return (struct bbbt *)nand_bbt_buf;
	} else
		return scan_bmt(block - 1);
}

/* Write the Burner Bad Block Table to Nand Flash
 * n - write BMT to bmt_tbl[n]
 */
static u16 upload_bmt(struct bbbt *bbt, int n)
{
	u16 block;

retry:
	if (n < 0 || bmt_tbl(bbt)[n].mapped == NORMAL_MAPPED) {
		pr_info("nand: FATAL ERR: no space to store BMT!\n");
		return (u16)-1;
	}

	block = bmt_tbl(bbt)[n].block;
	BBT_LOG("n = 0x%x, block = 0x%x", n, block);
	if (bbt_nand_erase(block)) {
		bmt_tbl(bbt)[n].block = 0;
		/* erase failed, try the previous block: bmt_tbl[n - 1].block */
		n--;
		goto retry;
	}

	/* The signature offset is fixed set to 0,
	 * oob signature offset is fixed set to 1
	 */
	memcpy(bbt->signature + MAIN_SIGNATURE_OFFSET, "BMT", 3);
	bbt->version = BBMT_VERSION;

	if (write_bmt(block, (unsigned char *)bbt)) {
		bmt_tbl(bbt)[n].block = 0;

		/* write failed, try the previous block in bmt_tbl[n - 1] */
		n--;
		goto retry;
	}

	/* Return the current index(n) of BMT pool (bmt_tbl[n]) */
	return n;
}

static u16 find_valid_block_in_pool(struct bbbt *bbt)
{
	int i;

	if (bmtd.bmt_blk_idx == 0)
		goto error;

	for (i = 0; i < bmtd.bmt_blk_idx; i++) {
		if (bmt_tbl(bbt)[i].block != 0 && bmt_tbl(bbt)[i].mapped == NO_MAPPED) {
			bmt_tbl(bbt)[i].mapped = NORMAL_MAPPED;
			return bmt_tbl(bbt)[i].block;
		}
	}

error:
	pr_info("nand: FATAL ERR: BMT pool is run out!\n");
	return 0;
}

/* We met a bad block, mark it as bad and map it to a valid block in pool,
 * if it's a write failure, we need to write the data to mapped block
 */
static bool remap_block_v2(u16 block, u16 mapped_block, int copy_len)
{
	u16 mapped_blk;
	struct bbbt *bbt;

	bbt = bmtd.bbt;
	mapped_blk = find_valid_block_in_pool(bbt);
	if (mapped_blk == 0)
		return false;

	/* Map new bad block to available block in pool */
	bbt->bb_tbl[block] = mapped_blk;

	/* Erase new block */
	bbt_nand_erase(mapped_blk);
	if (copy_len > 0)
		bbt_nand_copy(mapped_blk, block, copy_len);

	bmtd.bmt_blk_idx = upload_bmt(bbt, bmtd.bmt_blk_idx);

	return true;
}

static bool
mapping_block_in_range(int block, int *start, int *end)
{
	const __be32 *cur = bmtd.remap_range;
	u32 addr = block << bmtd.blk_shift;
	int i;

	if (!cur || !bmtd.remap_range_len) {
		*start = 0;
		*end = bmtd.total_blks;
		return true;
	}

	for (i = 0; i < bmtd.remap_range_len; i++, cur += 2) {
		if (addr < be32_to_cpu(cur[0]) || addr >= be32_to_cpu(cur[1]))
			continue;

		*start = be32_to_cpu(cur[0]);
		*end = be32_to_cpu(cur[1]);
		return true;
	}

	return false;
}

static u16
get_mapping_block_index_v2(int block)
{
	int start, end;

	if (block >= bmtd.pool_lba)
		return block;

	if (!mapping_block_in_range(block, &start, &end))
		return block;

	return bmtd.bbt->bb_tbl[block];
}

static int
mtk_bmt_read(struct mtd_info *mtd, loff_t from,
	     struct mtd_oob_ops *ops)
{
	struct mtd_oob_ops cur_ops = *ops;
	int retry_count = 0;
	loff_t cur_from;
	int ret = 0;
	int max_bitflips = 0;
	int start, end;

	ops->retlen = 0;
	ops->oobretlen = 0;

	while (ops->retlen < ops->len || ops->oobretlen < ops->ooblen) {
		int cur_ret;

		u32 offset = from & (bmtd.blk_size - 1);
		u32 block = from >> bmtd.blk_shift;
		u32 cur_block;

		cur_block = bmtd.ops->get_mapping_block(block);
		cur_from = ((loff_t)cur_block << bmtd.blk_shift) + offset;

		cur_ops.oobretlen = 0;
		cur_ops.retlen = 0;
		cur_ops.len = min_t(u32, mtd->erasesize - offset,
					 ops->len - ops->retlen);
		cur_ret = bmtd._read_oob(mtd, cur_from, &cur_ops);
		if (cur_ret < 0)
			ret = cur_ret;
		else
			max_bitflips = max_t(int, max_bitflips, cur_ret);
		if (cur_ret < 0 && !mtd_is_bitflip(cur_ret)) {
			bmtd.ops->remap_block(block, cur_block, mtd->erasesize);
			if (retry_count++ < 10)
				continue;

			goto out;
		}

		if (cur_ret >= mtd->bitflip_threshold &&
		    mapping_block_in_range(block, &start, &end))
			bmtd.ops->remap_block(block, cur_block, mtd->erasesize);

		ops->retlen += cur_ops.retlen;
		ops->oobretlen += cur_ops.oobretlen;

		cur_ops.ooboffs = 0;
		cur_ops.datbuf += cur_ops.retlen;
		cur_ops.oobbuf += cur_ops.oobretlen;
		cur_ops.ooblen -= cur_ops.oobretlen;

		if (!cur_ops.len)
			cur_ops.len = mtd->erasesize - offset;

		from += cur_ops.len;
		retry_count = 0;
	}

out:
	if (ret < 0)
		return ret;

	return max_bitflips;
}

static int
mtk_bmt_write(struct mtd_info *mtd, loff_t to,
	      struct mtd_oob_ops *ops)
{
	struct mtd_oob_ops cur_ops = *ops;
	int retry_count = 0;
	loff_t cur_to;
	int ret;

	ops->retlen = 0;
	ops->oobretlen = 0;

	while (ops->retlen < ops->len || ops->oobretlen < ops->ooblen) {
		u32 offset = to & (bmtd.blk_size - 1);
		u32 block = to >> bmtd.blk_shift;
		u32 cur_block;

		cur_block = bmtd.ops->get_mapping_block(block);
		cur_to = ((loff_t)cur_block << bmtd.blk_shift) + offset;

		cur_ops.oobretlen = 0;
		cur_ops.retlen = 0;
		cur_ops.len = min_t(u32, bmtd.blk_size - offset,
					 ops->len - ops->retlen);
		ret = bmtd._write_oob(mtd, cur_to, &cur_ops);
		if (ret < 0) {
			bmtd.ops->remap_block(block, cur_block, offset);
			if (retry_count++ < 10)
				continue;

			return ret;
		}

		ops->retlen += cur_ops.retlen;
		ops->oobretlen += cur_ops.oobretlen;

		cur_ops.ooboffs = 0;
		cur_ops.datbuf += cur_ops.retlen;
		cur_ops.oobbuf += cur_ops.oobretlen;
		cur_ops.ooblen -= cur_ops.oobretlen;

		if (!cur_ops.len)
			cur_ops.len = mtd->erasesize - offset;

		to += cur_ops.len;
		retry_count = 0;
	}

	return 0;
}

static int
mtk_bmt_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct erase_info mapped_instr = {
		.len = bmtd.blk_size,
	};
	int retry_count = 0;
	u64 start_addr, end_addr;
	int ret;
	u16 orig_block, block;

	start_addr = instr->addr & (~mtd->erasesize_mask);
	end_addr = instr->addr + instr->len;

	while (start_addr < end_addr) {
		orig_block = start_addr >> bmtd.blk_shift;
		block = bmtd.ops->get_mapping_block(orig_block);
		mapped_instr.addr = (loff_t)block << bmtd.blk_shift;
		ret = bmtd._erase(mtd, &mapped_instr);
		if (ret) {
			bmtd.ops->remap_block(orig_block, block, 0);
			if (retry_count++ < 10)
				continue;
			instr->fail_addr = start_addr;
			break;
		}
		start_addr += mtd->erasesize;
		retry_count = 0;
	}

	return ret;
}
static int
mtk_bmt_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	int retry_count = 0;
	u16 orig_block = ofs >> bmtd.blk_shift;
	u16 block;
	int ret;

retry:
	block = bmtd.ops->get_mapping_block(orig_block);
	ret = bmtd._block_isbad(mtd, (loff_t)block << bmtd.blk_shift);
	if (ret) {
		bmtd.ops->remap_block(orig_block, block, bmtd.blk_size);
		if (retry_count++ < 10)
			goto retry;
	}
	return ret;
}

static int
mtk_bmt_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	u16 orig_block = ofs >> bmtd.blk_shift;
	u16 block = bmtd.ops->get_mapping_block(orig_block);

	bmtd.ops->remap_block(orig_block, block, bmtd.blk_size);

	return bmtd._block_markbad(mtd, (loff_t)block << bmtd.blk_shift);
}

static void
mtk_bmt_replace_ops(struct mtd_info *mtd)
{
	bmtd._read_oob = mtd->_read_oob;
	bmtd._write_oob = mtd->_write_oob;
	bmtd._erase = mtd->_erase;
	bmtd._block_isbad = mtd->_block_isbad;
	bmtd._block_markbad = mtd->_block_markbad;

	mtd->_read_oob = mtk_bmt_read;
	mtd->_write_oob = mtk_bmt_write;
	mtd->_erase = mtk_bmt_mtd_erase;
	mtd->_block_isbad = mtk_bmt_block_isbad;
	mtd->_block_markbad = mtk_bmt_block_markbad;
}

static void
unmap_block_v2(u16 block)
{
	bmtd.bbt->bb_tbl[block] = block;
	bmtd.bmt_blk_idx = upload_bmt(bmtd.bbt, bmtd.bmt_blk_idx);
}

static int mtk_bmt_debug_mark_good(void *data, u64 val)
{
	 bmtd.ops->unmap_block(val >> bmtd.blk_shift);

	return 0;
}

static int mtk_bmt_debug_mark_bad(void *data, u64 val)
{
	u32 block = val >> bmtd.blk_shift;
	u16 cur_block = bmtd.ops->get_mapping_block(block);

	bmtd.ops->remap_block(block, cur_block, bmtd.blk_size);

	return 0;
}

static unsigned long *
mtk_bmt_get_mapping_mask(void)
{
	struct bbmt *bbmt = bmt_tbl(bmtd.bbt);
	int main_blocks = bmtd.mtd->size >> bmtd.blk_shift;
	unsigned long *used;
	int i, k;

	used = kcalloc(sizeof(unsigned long), BIT_WORD(bmtd.bmt_blk_idx) + 1, GFP_KERNEL);
	if (!used)
		return NULL;

	for (i = 1; i < main_blocks; i++) {
		if (bmtd.bbt->bb_tbl[i] == i)
			continue;

		for (k = 0; k < bmtd.bmt_blk_idx; k++) {
			if (bmtd.bbt->bb_tbl[i] != bbmt[k].block)
				continue;

			set_bit(k, used);
			break;
		}
	}

	return used;
}

static int mtk_bmt_debug_v2(void *data, u64 val)
{
	struct bbmt *bbmt = bmt_tbl(bmtd.bbt);
	struct mtd_info *mtd = bmtd.mtd;
	unsigned long *used;
	int main_blocks = mtd->size >> bmtd.blk_shift;
	int n_remap = 0;
	int i;

	used = mtk_bmt_get_mapping_mask();
	if (!used)
		return -ENOMEM;

	switch (val) {
	case 0:
		for (i = 1; i < main_blocks; i++) {
			if (bmtd.bbt->bb_tbl[i] == i)
				continue;

			printk("remap [%x->%x]\n", i, bmtd.bbt->bb_tbl[i]);
			n_remap++;
		}
		for (i = 0; i <= bmtd.bmt_blk_idx; i++) {
			char c;

			switch (bbmt[i].mapped) {
			case NO_MAPPED:
				continue;
			case NORMAL_MAPPED:
				c = 'm';
				if (test_bit(i, used))
					c = 'M';
				break;
			case BMT_MAPPED:
				c = 'B';
				break;
			default:
				c = 'X';
				break;
			}
			printk("[%x:%c] = 0x%x\n", i, c, bbmt[i].block);
		}
		break;
	case 100:
		for (i = 0; i <= bmtd.bmt_blk_idx; i++) {
			if (bbmt[i].mapped != NORMAL_MAPPED)
				continue;

			if (test_bit(i, used))
				continue;

			n_remap++;
			bbmt[i].mapped = NO_MAPPED;
			printk("free block [%d:%x]\n", i, bbmt[i].block);
		}
		if (n_remap)
			bmtd.bmt_blk_idx = upload_bmt(bmtd.bbt, bmtd.bmt_blk_idx);
		break;
	}

	kfree(used);

	return 0;
}

static int mtk_bmt_debug(void *data, u64 val)
{
	return bmtd.ops->debug(data, val);
}


DEFINE_DEBUGFS_ATTRIBUTE(fops_mark_good, NULL, mtk_bmt_debug_mark_good, "%llu\n");
DEFINE_DEBUGFS_ATTRIBUTE(fops_mark_bad, NULL, mtk_bmt_debug_mark_bad, "%llu\n");
DEFINE_DEBUGFS_ATTRIBUTE(fops_debug, NULL, mtk_bmt_debug, "%llu\n");

static void
mtk_bmt_add_debugfs(void)
{
	struct dentry *dir;

	dir = bmtd.debugfs_dir = debugfs_create_dir("mtk-bmt", NULL);
	if (!dir)
		return;

	debugfs_create_file_unsafe("mark_good", S_IWUSR, dir, NULL, &fops_mark_good);
	debugfs_create_file_unsafe("mark_bad", S_IWUSR, dir, NULL, &fops_mark_bad);
	debugfs_create_file_unsafe("debug", S_IWUSR, dir, NULL, &fops_debug);
}

void mtk_bmt_detach(struct mtd_info *mtd)
{
	if (bmtd.mtd != mtd)
		return;

	if (bmtd.debugfs_dir)
		debugfs_remove_recursive(bmtd.debugfs_dir);
	bmtd.debugfs_dir = NULL;

	kfree(nand_bbt_buf);
	kfree(nand_data_buf);

	mtd->_read_oob = bmtd._read_oob;
	mtd->_write_oob = bmtd._write_oob;
	mtd->_erase = bmtd._erase;
	mtd->_block_isbad = bmtd._block_isbad;
	mtd->_block_markbad = bmtd._block_markbad;
	mtd->size = bmtd.total_blks << bmtd.blk_shift;

	memset(&bmtd, 0, sizeof(bmtd));
}

static int mtk_bmt_init_v2(struct device_node *np)
{
	u32 bmt_pool_size, bmt_table_size;
	u32 bufsz, block;
	u16 pmt_block;

	if (of_property_read_u32(np, "mediatek,bmt-pool-size",
				 &bmt_pool_size) != 0)
		bmt_pool_size = 80;

	if (of_property_read_u8(np, "mediatek,bmt-oob-offset",
				 &bmtd.oob_offset) != 0)
		bmtd.oob_offset = 0;

	if (of_property_read_u32(np, "mediatek,bmt-table-size",
				 &bmt_table_size) != 0)
		bmt_table_size = 0x2000U;

	bmtd.table_size = bmt_table_size;

	pmt_block = bmtd.total_blks - bmt_pool_size - 2;

	bmtd.mtd->size = pmt_block << bmtd.blk_shift;

	/*
	 *  ---------------------------------------
	 * | PMT(2blks) | BMT POOL(totalblks * 2%) |
	 *  ---------------------------------------
	 * ^            ^
	 * |            |
	 * pmt_block	pmt_block + 2blocks(pool_lba)
	 *
	 * ATTETION!!!!!!
	 *     The blocks ahead of the boundary block are stored in bb_tbl
	 *     and blocks behind are stored in bmt_tbl
	 */

	bmtd.pool_lba = (u16)(pmt_block + 2);
	bmtd.bb_max = bmtd.total_blks * BBPOOL_RATIO / 100;

	bufsz = round_up(sizeof(struct bbbt) +
			 bmt_table_size * sizeof(struct bbmt), bmtd.pg_size);
	bmtd.bmt_pgs = bufsz >> bmtd.pg_shift;

	nand_bbt_buf = kzalloc(bufsz, GFP_KERNEL);
	if (!nand_bbt_buf)
		return -ENOMEM;

	memset(nand_bbt_buf, 0xff, bufsz);

	/* Scanning start from the first page of the last block
	 * of whole flash
	 */
	bmtd.bbt = scan_bmt(bmtd.total_blks - 1);
	if (!bmtd.bbt) {
		/* BMT not found */
		if (bmtd.total_blks > BB_TABLE_MAX + BMT_TABLE_MAX) {
			pr_info("nand: FATAL: Too many blocks, can not support!\n");
			return -1;
		}

		bmtd.bbt = (struct bbbt *)nand_bbt_buf;
		memset(bmt_tbl(bmtd.bbt), BMT_TBL_DEF_VAL,
		       bmtd.table_size * sizeof(struct bbmt));

		if (scan_bad_blocks(bmtd.bbt))
			return -1;

		/* BMT always in the last valid block in pool */
		bmtd.bmt_blk_idx = upload_bmt(bmtd.bbt, bmtd.bmt_blk_idx);
		block = bmt_tbl(bmtd.bbt)[bmtd.bmt_blk_idx].block;
		pr_notice("[BBT] BMT.v2 is written into PBA:0x%x\n", block);

		if (bmtd.bmt_blk_idx == 0)
			pr_info("nand: Warning: no available block in BMT pool!\n");
		else if (bmtd.bmt_blk_idx == (u16)-1)
			return -1;
	}

	return 0;
}

static bool
bbt_block_is_bad(u16 block)
{
	u8 cur = nand_bbt_buf[block / 4];

	return cur & (3 << ((block % 4) * 2));
}

static void
bbt_set_block_state(u16 block, bool bad)
{
	u8 mask = (3 << ((block % 4) * 2));

	if (bad)
		nand_bbt_buf[block / 4] |= mask;
	else
		nand_bbt_buf[block / 4] &= ~mask;

	bbt_nand_erase(bmtd.bmt_blk_idx);
	write_bmt(bmtd.bmt_blk_idx, nand_bbt_buf);
}

static u16
get_mapping_block_index_bbt(int block)
{
	int start, end, ofs;
	int bad_blocks = 0;
	int i;

	if (!mapping_block_in_range(block, &start, &end))
		return block;

	start >>= bmtd.blk_shift;
	end >>= bmtd.blk_shift;
	/* skip bad blocks within the mapping range */
	ofs = block - start;
	for (i = start; i < end; i++) {
		if (bbt_block_is_bad(i))
			bad_blocks++;
		else if (ofs)
			ofs--;
		else
			break;
	}

	if (i < end)
		return i;

	/* when overflowing, remap remaining blocks to bad ones */
	for (i = end - 1; bad_blocks > 0; i--) {
		if (!bbt_block_is_bad(i))
			continue;

		bad_blocks--;
		if (bad_blocks <= ofs)
			return i;
	}

	return block;
}

static bool remap_block_bbt(u16 block, u16 mapped_blk, int copy_len)
{
	int start, end;
	u16 new_blk;

	if (!mapping_block_in_range(block, &start, &end))
		return false;

	bbt_set_block_state(mapped_blk, true);

	new_blk = get_mapping_block_index_bbt(block);
	bbt_nand_erase(new_blk);
	if (copy_len > 0)
		bbt_nand_copy(new_blk, mapped_blk, copy_len);

	return false;
}

static void
unmap_block_bbt(u16 block)
{
	bbt_set_block_state(block, false);
}

static int
mtk_bmt_read_bbt(void)
{
	u8 oob_buf[8];
	int i;

	for (i = bmtd.total_blks - 1; i >= bmtd.total_blks - 5; i--) {
		u32 page = i << (bmtd.blk_shift - bmtd.pg_shift);

		if (bbt_nand_read(page, nand_bbt_buf, bmtd.pg_size,
				  oob_buf, sizeof(oob_buf))) {
			pr_info("read_bbt: could not read block %d\n", i);
			continue;
		}

		if (oob_buf[0] != 0xff) {
			pr_info("read_bbt: bad block at %d\n", i);
			continue;
		}

		if (memcmp(&oob_buf[1], "mtknand", 7) != 0) {
			pr_info("read_bbt: signature mismatch in block %d\n", i);
			print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET, 16, 1, oob_buf, 8, 1);
			continue;
		}

		pr_info("read_bbt: found bbt at block %d\n", i);
		bmtd.bmt_blk_idx = i;
		return 0;
	}

	return -EIO;
}


static int
mtk_bmt_init_bbt(struct device_node *np)
{
	int buf_size = round_up(bmtd.total_blks >> 2, bmtd.blk_size);
	int ret;

	nand_bbt_buf = kmalloc(buf_size, GFP_KERNEL);
	if (!nand_bbt_buf)
		return -ENOMEM;

	memset(nand_bbt_buf, 0xff, buf_size);
	bmtd.mtd->size -= 4 * bmtd.mtd->erasesize;

	ret = mtk_bmt_read_bbt();
	if (ret)
		return ret;

	bmtd.bmt_pgs = buf_size / bmtd.pg_size;

	return 0;
}

static int mtk_bmt_debug_bbt(void *data, u64 val)
{
	char buf[5];
	int i, k;

	switch (val) {
	case 0:
		for (i = 0; i < bmtd.total_blks; i += 4) {
			u8 cur = nand_bbt_buf[i / 4];

			for (k = 0; k < 4; k++, cur >>= 2)
				buf[k] = (cur & 3) ? 'B' : '.';

			buf[4] = 0;
			printk("[%06x] %s\n", i * bmtd.blk_size, buf);
		}
		break;
	case 100:
#if 0
		for (i = bmtd.bmt_blk_idx; i < bmtd.total_blks - 1; i++)
			bbt_nand_erase(bmtd.bmt_blk_idx);
#endif

		bmtd.bmt_blk_idx = bmtd.total_blks - 1;
		bbt_nand_erase(bmtd.bmt_blk_idx);
		write_bmt(bmtd.bmt_blk_idx, nand_bbt_buf);
		break;
	default:
		break;
	}
	return 0;
}

int mtk_bmt_attach(struct mtd_info *mtd)
{
	static const struct mtk_bmt_ops v2_ops = {
		.sig = "bmt",
		.sig_len = 3,
		.init = mtk_bmt_init_v2,
		.remap_block = remap_block_v2,
		.unmap_block = unmap_block_v2,
		.get_mapping_block = get_mapping_block_index_v2,
		.debug = mtk_bmt_debug_v2,
	};
	static const struct mtk_bmt_ops bbt_ops = {
		.sig = "mtknand",
		.sig_len = 7,
		.init = mtk_bmt_init_bbt,
		.remap_block = remap_block_bbt,
		.unmap_block = unmap_block_bbt,
		.get_mapping_block = get_mapping_block_index_bbt,
		.debug = mtk_bmt_debug_bbt,
	};
	struct device_node *np;
	int ret = 0;

	if (bmtd.mtd)
		return -ENOSPC;

	np = mtd_get_of_node(mtd);
	if (!np)
		return 0;

	if (of_property_read_bool(np, "mediatek,bmt-v2"))
		bmtd.ops = &v2_ops;
	else if (of_property_read_bool(np, "mediatek,bbt"))
		bmtd.ops = &bbt_ops;
	else
		return 0;

	bmtd.remap_range = of_get_property(np, "mediatek,bmt-remap-range",
					   &bmtd.remap_range_len);
	bmtd.remap_range_len /= 8;

	bmtd.mtd = mtd;
	mtk_bmt_replace_ops(mtd);

	bmtd.blk_size = mtd->erasesize;
	bmtd.blk_shift = ffs(bmtd.blk_size) - 1;
	bmtd.pg_size = mtd->writesize;
	bmtd.pg_shift = ffs(bmtd.pg_size) - 1;
	bmtd.total_blks = mtd->size >> bmtd.blk_shift;

	nand_data_buf = kzalloc(bmtd.pg_size, GFP_KERNEL);
	if (!nand_data_buf) {
		pr_info("nand: FATAL ERR: allocate buffer failed!\n");
		ret = -1;
		goto error;
	}

	memset(nand_data_buf, 0xff, bmtd.pg_size);

	ret = bmtd.ops->init(np);
	if (ret)
		goto error;

	mtk_bmt_add_debugfs();
	return 0;

error:
	mtk_bmt_detach(mtd);
	return ret;
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Xiangsheng Hou <xiangsheng.hou@mediatek.com>, Felix Fietkau <nbd@nbd.name>");
MODULE_DESCRIPTION("Bad Block mapping management v2 for MediaTek NAND Flash Driver");

