/*
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
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
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define SEAMA_MAGIC		0x5EA3A417
#define SEAMA_NR_PARTS		2
#define SEAMA_MIN_ROOTFS_OFFS	0x80000	/* 512KiB */

struct seama_header {
	__be32	magic;		/* should always be SEAMA_MAGIC. */
	__be16	reserved;	/* reserved for  */
	__be16	metasize;	/* size of the META data */
	__be32	size;		/* size of the image */
	u8	md5[16];	/* digest */
};

static int mtdsplit_parse_seama(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct seama_header hdr;
	size_t hdr_len, retlen, kernel_ent_size;
	size_t rootfs_offset;
	struct mtd_partition *parts;
	enum mtdsplit_part_type type;
	int err;

	hdr_len = sizeof(hdr);
	err = mtd_read(master, 0, hdr_len, &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != hdr_len)
		return -EIO;

	/* sanity checks */
	if (be32_to_cpu(hdr.magic) != SEAMA_MAGIC)
		return -EINVAL;

	kernel_ent_size = hdr_len + be32_to_cpu(hdr.size) +
			  be16_to_cpu(hdr.metasize);
	if (kernel_ent_size > master->size)
		return -EINVAL;

	/* Check for the rootfs right after Seama entity with a kernel. */
	err = mtd_check_rootfs_magic(master, kernel_ent_size, &type);
	if (!err) {
		rootfs_offset = kernel_ent_size;
	} else {
		/*
		 * On some devices firmware entity might contain both: kernel
		 * and rootfs. We can't determine kernel size so we just have to
		 * look for rootfs magic.
		 * Start the search from an arbitrary offset.
		 */
		err = mtd_find_rootfs_from(master, SEAMA_MIN_ROOTFS_OFFS,
					   master->size, &rootfs_offset, &type);
		if (err)
			return err;
	}

	parts = kzalloc(SEAMA_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = sizeof hdr + be16_to_cpu(hdr.metasize);
	parts[0].size = rootfs_offset - parts[0].offset;

	if (type == MTDSPLIT_PART_TYPE_UBI)
		parts[1].name = UBI_PART_NAME;
	else
		parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return SEAMA_NR_PARTS;
}

static const struct of_device_id mtdsplit_seama_of_match_table[] = {
	{ .compatible = "seama" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_seama_of_match_table);

static struct mtd_part_parser mtdsplit_seama_parser = {
	.owner = THIS_MODULE,
	.name = "seama-fw",
	.of_match_table = mtdsplit_seama_of_match_table,
	.parse_fn = mtdsplit_parse_seama,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_seama_init(void)
{
	register_mtd_parser(&mtdsplit_seama_parser);

	return 0;
}

subsys_initcall(mtdsplit_seama_init);
