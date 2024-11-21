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

static bool
bbt_block_is_bad(u16 block)
{
	u8 cur = bmtd.bbt_buf[block / 4];

	return cur & (3 << ((block % 4) * 2));
}

static void
bbt_set_block_state(u16 block, bool bad)
{
	u8 mask = (3 << ((block % 4) * 2));

	if (bad)
		bmtd.bbt_buf[block / 4] |= mask;
	else
		bmtd.bbt_buf[block / 4] &= ~mask;

	bbt_nand_erase(bmtd.bmt_blk_idx);
	write_bmt(bmtd.bmt_blk_idx, bmtd.bbt_buf);
}

static int
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

	return true;
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

		if (bbt_nand_read(page, bmtd.bbt_buf, bmtd.pg_size,
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

	bmtd.bbt_buf = kmalloc(buf_size, GFP_KERNEL);
	if (!bmtd.bbt_buf)
		return -ENOMEM;

	memset(bmtd.bbt_buf, 0xff, buf_size);
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
			u8 cur = bmtd.bbt_buf[i / 4];

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
		write_bmt(bmtd.bmt_blk_idx, bmtd.bbt_buf);
		break;
	default:
		break;
	}
	return 0;
}

const struct mtk_bmt_ops mtk_bmt_bbt_ops = {
	.sig = "mtknand",
	.sig_len = 7,
	.init = mtk_bmt_init_bbt,
	.remap_block = remap_block_bbt,
	.unmap_block = unmap_block_bbt,
	.get_mapping_block = get_mapping_block_index_bbt,
	.debug = mtk_bmt_debug_bbt,
};
