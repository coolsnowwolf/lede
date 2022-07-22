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

#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/bits.h>
#include "mtk_bmt.h"

struct bmt_desc bmtd = {};

/* -------- Nand operations wrapper -------- */
int bbt_nand_copy(u16 dest_blk, u16 src_blk, loff_t max_offset)
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
			.datbuf = bmtd.data_buf,
			.len = bmtd.pg_size,
		};
		struct mtd_oob_ops wr_ops = {
			.mode = MTD_OPS_PLACE_OOB,
			.oobbuf = oob,
			.datbuf = bmtd.data_buf,
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
bool mapping_block_in_range(int block, int *start, int *end)
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

static bool
mtk_bmt_remap_block(u32 block, u32 mapped_block, int copy_len)
{
	int start, end;

	if (!mapping_block_in_range(block, &start, &end))
		return false;

	return bmtd.ops->remap_block(block, mapped_block, copy_len);
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

	ops->retlen = 0;
	ops->oobretlen = 0;

	while (ops->retlen < ops->len || ops->oobretlen < ops->ooblen) {
		int cur_ret;

		u32 offset = from & (bmtd.blk_size - 1);
		u32 block = from >> bmtd.blk_shift;
		int cur_block;

		cur_block = bmtd.ops->get_mapping_block(block);
		if (cur_block < 0)
			return -EIO;

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
			if (mtk_bmt_remap_block(block, cur_block, mtd->erasesize) &&
				retry_count++ < 10)
				continue;

			goto out;
		}

		if (mtd->bitflip_threshold && cur_ret >= mtd->bitflip_threshold)
			mtk_bmt_remap_block(block, cur_block, mtd->erasesize);

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
		int cur_block;

		cur_block = bmtd.ops->get_mapping_block(block);
		if (cur_block < 0)
			return -EIO;

		cur_to = ((loff_t)cur_block << bmtd.blk_shift) + offset;

		cur_ops.oobretlen = 0;
		cur_ops.retlen = 0;
		cur_ops.len = min_t(u32, bmtd.blk_size - offset,
					 ops->len - ops->retlen);
		ret = bmtd._write_oob(mtd, cur_to, &cur_ops);
		if (ret < 0) {
			if (mtk_bmt_remap_block(block, cur_block, offset) &&
			    retry_count++ < 10)
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
	u16 orig_block;
	int block;

	start_addr = instr->addr & (~mtd->erasesize_mask);
	end_addr = instr->addr + instr->len;

	while (start_addr < end_addr) {
		orig_block = start_addr >> bmtd.blk_shift;
		block = bmtd.ops->get_mapping_block(orig_block);
		if (block < 0)
			return -EIO;
		mapped_instr.addr = (loff_t)block << bmtd.blk_shift;
		ret = bmtd._erase(mtd, &mapped_instr);
		if (ret) {
			if (mtk_bmt_remap_block(orig_block, block, 0) &&
			    retry_count++ < 10)
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
		if (mtk_bmt_remap_block(orig_block, block, bmtd.blk_size) &&
		    retry_count++ < 10)
			goto retry;
	}
	return ret;
}

static int
mtk_bmt_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	u16 orig_block = ofs >> bmtd.blk_shift;
	int block;

	block = bmtd.ops->get_mapping_block(orig_block);
	if (block < 0)
		return -EIO;

	mtk_bmt_remap_block(orig_block, block, bmtd.blk_size);

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

static int mtk_bmt_debug_repair(void *data, u64 val)
{
	int block = val >> bmtd.blk_shift;
	int prev_block, new_block;

	prev_block = bmtd.ops->get_mapping_block(block);
	if (prev_block < 0)
		return -EIO;

	bmtd.ops->unmap_block(block);
	new_block = bmtd.ops->get_mapping_block(block);
	if (new_block < 0)
		return -EIO;

	if (prev_block == new_block)
		return 0;

	bbt_nand_erase(new_block);
	bbt_nand_copy(new_block, prev_block, bmtd.blk_size);

	return 0;
}

static int mtk_bmt_debug_mark_good(void *data, u64 val)
{
	bmtd.ops->unmap_block(val >> bmtd.blk_shift);

	return 0;
}

static int mtk_bmt_debug_mark_bad(void *data, u64 val)
{
	u32 block = val >> bmtd.blk_shift;
	int cur_block;

	cur_block = bmtd.ops->get_mapping_block(block);
	if (cur_block < 0)
		return -EIO;

	mtk_bmt_remap_block(block, cur_block, bmtd.blk_size);

	return 0;
}

static int mtk_bmt_debug(void *data, u64 val)
{
	return bmtd.ops->debug(data, val);
}


DEFINE_DEBUGFS_ATTRIBUTE(fops_repair, NULL, mtk_bmt_debug_repair, "%llu\n");
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

	debugfs_create_file_unsafe("repair", S_IWUSR, dir, NULL, &fops_repair);
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

	kfree(bmtd.bbt_buf);
	kfree(bmtd.data_buf);

	mtd->_read_oob = bmtd._read_oob;
	mtd->_write_oob = bmtd._write_oob;
	mtd->_erase = bmtd._erase;
	mtd->_block_isbad = bmtd._block_isbad;
	mtd->_block_markbad = bmtd._block_markbad;
	mtd->size = bmtd.total_blks << bmtd.blk_shift;

	memset(&bmtd, 0, sizeof(bmtd));
}


int mtk_bmt_attach(struct mtd_info *mtd)
{
	struct device_node *np;
	int ret = 0;

	if (bmtd.mtd)
		return -ENOSPC;

	np = mtd_get_of_node(mtd);
	if (!np)
		return 0;

	if (of_property_read_bool(np, "mediatek,bmt-v2"))
		bmtd.ops = &mtk_bmt_v2_ops;
	else if (of_property_read_bool(np, "mediatek,nmbm"))
		bmtd.ops = &mtk_bmt_nmbm_ops;
	else if (of_property_read_bool(np, "mediatek,bbt"))
		bmtd.ops = &mtk_bmt_bbt_ops;
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

	bmtd.data_buf = kzalloc(bmtd.pg_size + bmtd.mtd->oobsize, GFP_KERNEL);
	if (!bmtd.data_buf) {
		pr_info("nand: FATAL ERR: allocate buffer failed!\n");
		ret = -1;
		goto error;
	}

	memset(bmtd.data_buf, 0xff, bmtd.pg_size + bmtd.mtd->oobsize);

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

