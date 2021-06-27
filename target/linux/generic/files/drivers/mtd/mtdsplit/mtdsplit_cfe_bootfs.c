// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2021 Rafał Miłecki <rafal@milecki.pl>
 */

#include <linux/init.h>
#include <linux/jffs2.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/slab.h>

#include "mtdsplit.h"

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

#define NR_PARTS		2

static int mtdsplit_cfe_bootfs_parse(struct mtd_info *mtd,
				     const struct mtd_partition **pparts,
				     struct mtd_part_parser_data *data)
{
	struct jffs2_raw_dirent node;
	enum mtdsplit_part_type type;
	struct mtd_partition *parts;
	size_t rootfs_offset;
	size_t retlen;
	size_t offset;
	int err;

	/* Don't parse backup partitions */
	if (strcmp(mtd->name, "firmware"))
		return -EINVAL;

	/* Find the end of JFFS2 bootfs partition */
	offset = 0;
	do {
		err = mtd_read(mtd, offset, sizeof(node), &retlen, (void *)&node);
		if (err || retlen != sizeof(node))
			break;

		if (je16_to_cpu(node.magic) != JFFS2_MAGIC_BITMASK)
			break;

		offset += je32_to_cpu(node.totlen);
		offset = (offset + 0x3) & ~0x3;
	} while (offset < mtd->size);

	/* Find rootfs partition that follows the bootfs */
	err = mtd_find_rootfs_from(mtd, mtd->erasesize, mtd->size, &rootfs_offset, &type);
	if (err)
		return err;

	parts = kzalloc(NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = "bootfs";
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	if (type == MTDSPLIT_PART_TYPE_UBI)
		parts[1].name = UBI_PART_NAME;
	else
		parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = mtd->size - rootfs_offset;

	*pparts = parts;

	return NR_PARTS;
}

static const struct of_device_id mtdsplit_cfe_bootfs_of_match_table[] = {
	{ .compatible = "brcm,bcm4908-firmware" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_cfe_bootfs_of_match_table);

static struct mtd_part_parser mtdsplit_cfe_bootfs_parser = {
	.owner = THIS_MODULE,
	.name = "cfe-bootfs",
	.of_match_table = mtdsplit_cfe_bootfs_of_match_table,
	.parse_fn = mtdsplit_cfe_bootfs_parse,
};

module_mtd_part_parser(mtdsplit_cfe_bootfs_parser);
