/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Xiangsheng Hou <xiangsheng.hou@mediatek.com>
 * Copyright (c) 2020-2022 Felix Fietkau <nbd@nbd.name>
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

#include <linux/kernel.h>
#include <linux/slab.h>
#include "mtk_bmt.h"

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

/* Maximum 8k blocks */
#define BBPOOL_RATIO		2
#define BB_TABLE_MAX	bmtd.table_size
#define BMT_TABLE_MAX	(BB_TABLE_MAX * BBPOOL_RATIO / 100)
#define BMT_TBL_DEF_VAL	0x0

static inline struct bbmt *bmt_tbl(struct bbbt *bbbt)
{
	return (struct bbmt *)&bbbt->bb_tbl[bmtd.table_size];
}

static u16 find_valid_block(u16 block)
{
	u8 fdm[4];
	int ret;
	int loop = 0;

retry:
	if (block >= bmtd.total_blks)
		return 0;

	ret = bbt_nand_read(blk_pg(block), bmtd.data_buf, bmtd.pg_size,
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
	u16 new_block;
	struct bbbt *bbt;

	bbt = bmtd.bbt;
	new_block = find_valid_block_in_pool(bbt);
	if (new_block == 0)
		return false;

	/* Map new bad block to available block in pool */
	bbt->bb_tbl[block] = new_block;

	/* Erase new block */
	bbt_nand_erase(new_block);
	if (copy_len > 0)
		bbt_nand_copy(new_block, mapped_block, copy_len);

	bmtd.bmt_blk_idx = upload_bmt(bbt, bmtd.bmt_blk_idx);

	return true;
}

static int get_mapping_block_index_v2(int block)
{
	int start, end;

	if (block >= bmtd.pool_lba)
		return block;

	if (!mapping_block_in_range(block, &start, &end))
		return block;

	return bmtd.bbt->bb_tbl[block];
}

static void
unmap_block_v2(u16 block)
{
	bmtd.bbt->bb_tbl[block] = block;
	bmtd.bmt_blk_idx = upload_bmt(bmtd.bbt, bmtd.bmt_blk_idx);
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

	bmtd.bbt_buf = kzalloc(bufsz, GFP_KERNEL);
	if (!bmtd.bbt_buf)
		return -ENOMEM;

	memset(bmtd.bbt_buf, 0xff, bufsz);

	/* Scanning start from the first page of the last block
	 * of whole flash
	 */
	bmtd.bbt = NULL;
	for (u16 block = bmtd.total_blks - 1; !bmtd.bbt && block >= bmtd.pool_lba; block--) {
		u8 fdm[4];

		if (bbt_nand_read(blk_pg(block), bmtd.bbt_buf, bufsz, fdm, sizeof(fdm))) {
			/* Read failed, try the previous block */
			continue;
		}

		if (!is_valid_bmt(bmtd.bbt_buf, fdm)) {
			/* No valid BMT found, try the previous block */
			continue;
		}

		bmtd.bmt_blk_idx = get_bmt_index(bmt_tbl((struct bbbt *)bmtd.bbt_buf));
		if (bmtd.bmt_blk_idx == 0) {
			pr_info("[BBT] FATAL ERR: bmt block index is wrong!\n");
			break;
		}

		pr_info("[BBT] BMT.v2 is found at 0x%x\n", block);
		bmtd.bbt = (struct bbbt *)bmtd.bbt_buf;
	}

	if (!bmtd.bbt) {
		/* BMT not found */
		if (bmtd.total_blks > BB_TABLE_MAX + BMT_TABLE_MAX) {
			pr_info("nand: FATAL: Too many blocks, can not support!\n");
			return -1;
		}

		bmtd.bbt = (struct bbbt *)bmtd.bbt_buf;
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


const struct mtk_bmt_ops mtk_bmt_v2_ops = {
	.sig = "bmt",
	.sig_len = 3,
	.init = mtk_bmt_init_v2,
	.remap_block = remap_block_v2,
	.unmap_block = unmap_block_v2,
	.get_mapping_block = get_mapping_block_index_v2,
	.debug = mtk_bmt_debug_v2,
};
