/*
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/unaligned.h>

#include "mtdsplit.h"

#define LZMA_NR_PARTS		2
#define LZMA_PROPERTIES_SIZE	5

struct lzma_header {
	u8 props[LZMA_PROPERTIES_SIZE];
	u8 size_low[4];
	u8 size_high[4];
};

static int mtdsplit_parse_lzma(struct mtd_info *master,
			       const struct mtd_partition **pparts,
			       struct mtd_part_parser_data *data)
{
	struct lzma_header hdr;
	size_t hdr_len, retlen;
	size_t rootfs_offset;
	u32 t;
	struct mtd_partition *parts;
	int err;

	hdr_len = sizeof(hdr);
	err = mtd_read(master, 0, hdr_len, &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != hdr_len)
		return -EIO;

	/* verify LZMA properties */
	if (hdr.props[0] >= (9 * 5 * 5))
		return -EINVAL;

	t = get_unaligned_le32(&hdr.props[1]);
	if (!is_power_of_2(t))
		return -EINVAL;

	t = get_unaligned_le32(&hdr.size_high);
	if (t)
		return -EINVAL;

	err = mtd_find_rootfs_from(master, master->erasesize, master->size,
				   &rootfs_offset, NULL);
	if (err)
		return err;

	parts = kzalloc(LZMA_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return LZMA_NR_PARTS;
}

static struct mtd_part_parser mtdsplit_lzma_parser = {
	.owner = THIS_MODULE,
	.name = "lzma-fw",
	.parse_fn = mtdsplit_parse_lzma,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_lzma_init(void)
{
	register_mtd_parser(&mtdsplit_lzma_parser);

	return 0;
}

subsys_initcall(mtdsplit_lzma_init);
