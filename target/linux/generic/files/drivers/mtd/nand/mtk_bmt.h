#ifndef __MTK_BMT_PRIV_H
#define __MTK_BMT_PRIV_H

#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/mtk_bmt.h>
#include <linux/debugfs.h>

#define MAIN_SIGNATURE_OFFSET   0
#define OOB_SIGNATURE_OFFSET    1

#define BBT_LOG(fmt, ...) pr_debug("[BBT][%s|%d] "fmt"\n", __func__, __LINE__, ##__VA_ARGS__)

struct mtk_bmt_ops {
	char *sig;
	unsigned int sig_len;
	int (*init)(struct device_node *np);
	bool (*remap_block)(u16 block, u16 mapped_block, int copy_len);
	void (*unmap_block)(u16 block);
	int (*get_mapping_block)(int block);
	int (*debug)(void *data, u64 val);
};

struct bbbt;
struct nmbm_instance;

struct bmt_desc {
	struct mtd_info *mtd;
	unsigned char *bbt_buf;
	unsigned char *data_buf;

	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			   struct mtd_oob_ops *ops);
	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);

	const struct mtk_bmt_ops *ops;

	union {
		struct bbbt *bbt;
		struct nmbm_instance *ni;
	};

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
};

extern struct bmt_desc bmtd;
extern const struct mtk_bmt_ops mtk_bmt_v2_ops;
extern const struct mtk_bmt_ops mtk_bmt_bbt_ops;
extern const struct mtk_bmt_ops mtk_bmt_nmbm_ops;

static inline u32 blk_pg(u16 block)
{
	return (u32)(block << (bmtd.blk_shift - bmtd.pg_shift));
}

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

static inline int write_bmt(u16 block, unsigned char *dat)
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

int bbt_nand_copy(u16 dest_blk, u16 src_blk, loff_t max_offset);
bool mapping_block_in_range(int block, int *start, int *end);

#endif
