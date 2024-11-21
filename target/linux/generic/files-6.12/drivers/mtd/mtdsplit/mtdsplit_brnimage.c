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

#include "mtdsplit.h"

#define BRNIMAGE_NR_PARTS	2

#define BRNIMAGE_ALIGN_BYTES	0x400
#define BRNIMAGE_FOOTER_SIZE	12

#define BRNIMAGE_MIN_OVERHEAD	(BRNIMAGE_FOOTER_SIZE)
#define BRNIMAGE_MAX_OVERHEAD	(BRNIMAGE_ALIGN_BYTES + BRNIMAGE_FOOTER_SIZE)

static int mtdsplit_parse_brnimage(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	uint32_t buf;
	unsigned long rootfs_offset, rootfs_size, kernel_size;
	size_t len;
	int ret = 0;

	for (rootfs_offset = 0; rootfs_offset < master->size;
	     rootfs_offset += BRNIMAGE_ALIGN_BYTES) {
		ret = mtd_check_rootfs_magic(master, rootfs_offset, NULL);
		if (!ret)
			break;
	}

	if (ret)
		return ret;

	if (rootfs_offset >= master->size)
		return -EINVAL;

	ret = mtd_read(master, rootfs_offset - BRNIMAGE_FOOTER_SIZE, 4, &len,
			(void *)&buf);
	if (ret)
		return ret;

	if (len != 4)
		return -EIO;

	kernel_size = le32_to_cpu(buf);

	if (kernel_size > (rootfs_offset - BRNIMAGE_MIN_OVERHEAD))
		return -EINVAL;

	if (kernel_size < (rootfs_offset - BRNIMAGE_MAX_OVERHEAD))
		return -EINVAL;

	/*
	 * The footer must be untouched as it contains the checksum of the
	 * original brnImage (kernel + squashfs)!
	 */
	rootfs_size = master->size - rootfs_offset - BRNIMAGE_FOOTER_SIZE;

	parts = kzalloc(BRNIMAGE_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = kernel_size;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = rootfs_size;

	*pparts = parts;
	return BRNIMAGE_NR_PARTS;
}

static struct mtd_part_parser mtdsplit_brnimage_parser = {
	.owner = THIS_MODULE,
	.name = "brnimage-fw",
	.parse_fn = mtdsplit_parse_brnimage,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_brnimage_init(void)
{
	register_mtd_parser(&mtdsplit_brnimage_parser);

	return 0;
}

subsys_initcall(mtdsplit_brnimage_init);
