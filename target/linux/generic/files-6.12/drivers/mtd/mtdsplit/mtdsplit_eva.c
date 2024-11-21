/*
 *  Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 *  Copyright (C) 2015 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
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
#include <linux/of.h>

#include "mtdsplit.h"

#define EVA_NR_PARTS		2
#define EVA_MAGIC		0xfeed1281
#define EVA_FOOTER_SIZE		0x18
#define EVA_DUMMY_SQUASHFS_SIZE	0x100

struct eva_image_header {
	uint32_t	magic;
	uint32_t	size;
};

static int mtdsplit_parse_eva(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct eva_image_header hdr;
	size_t retlen;
	unsigned long kernel_size, rootfs_offset;
	int err;

	err = mtd_read(master, 0, sizeof(hdr), &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != sizeof(hdr))
		return -EIO;

	if (le32_to_cpu(hdr.magic) != EVA_MAGIC)
		return -EINVAL;

	kernel_size = le32_to_cpu(hdr.size) + EVA_FOOTER_SIZE;

	/* rootfs starts at the next 0x10000 boundary: */
	rootfs_offset = round_up(kernel_size, 0x10000);

	/* skip the dummy EVA squashfs partition (with wrong endianness): */
	rootfs_offset += EVA_DUMMY_SQUASHFS_SIZE;

	if (rootfs_offset >= master->size)
		return -EINVAL;

	err = mtd_check_rootfs_magic(master, rootfs_offset, NULL);
	if (err)
		return err;

	parts = kzalloc(EVA_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = kernel_size;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return EVA_NR_PARTS;
}

static const struct of_device_id mtdsplit_eva_of_match_table[] = {
	{ .compatible = "avm,eva-firmware" },
	{},
};

static struct mtd_part_parser mtdsplit_eva_parser = {
	.owner = THIS_MODULE,
	.name = "eva-fw",
	.of_match_table = mtdsplit_eva_of_match_table,
	.parse_fn = mtdsplit_parse_eva,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_eva_init(void)
{
	register_mtd_parser(&mtdsplit_eva_parser);

	return 0;
}

subsys_initcall(mtdsplit_eva_init);
