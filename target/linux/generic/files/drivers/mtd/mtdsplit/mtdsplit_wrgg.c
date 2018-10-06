/*
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Felix Fietkau <nbd@nbd.name>
 *  Copyright (C) 2016 Stijn Tintel <stijn@linux-ipv6.be>
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

#define WRGG_NR_PARTS		2
#define WRGG_MIN_ROOTFS_OFFS	0x80000	/* 512KiB */
#define WRGG03_MAGIC		0x20080321
#define WRG_MAGIC		0x20040220

struct wrgg03_header {
	char		signature[32];
	uint32_t	magic1;
	uint32_t	magic2;
	char		version[16];
	char		model[16];
	uint32_t	flag[2];
	uint32_t	reserve[2];
	char		buildno[16];
	uint32_t	size;
	uint32_t	offset;
	char		devname[32];
	char		digest[16];
} __attribute__ ((packed));

struct wrg_header {
	char		signature[32];
	uint32_t	magic1;
	uint32_t	magic2;
	uint32_t	size;
	uint32_t	offset;
	char		devname[32];
	char		digest[16];
} __attribute__ ((packed));


static int mtdsplit_parse_wrgg(struct mtd_info *master,
			       const struct mtd_partition **pparts,
			       struct mtd_part_parser_data *data)
{
	struct wrgg03_header hdr;
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
	if (le32_to_cpu(hdr.magic1) == WRGG03_MAGIC) {
		kernel_ent_size = hdr_len + be32_to_cpu(hdr.size);
	} else if (le32_to_cpu(hdr.magic1) == WRG_MAGIC) {
		kernel_ent_size = sizeof(struct wrg_header) + le32_to_cpu(
		                  ((struct wrg_header*)&hdr)->size);
	} else {
		return -EINVAL;
	}

	if (kernel_ent_size > master->size)
		return -EINVAL;

	/*
	 * The size in the header covers the rootfs as well.
	 * Start the search from an arbitrary offset.
	 */
	err = mtd_find_rootfs_from(master, WRGG_MIN_ROOTFS_OFFS,
				   master->size, &rootfs_offset, &type);
	if (err)
		return err;

	parts = kzalloc(WRGG_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return WRGG_NR_PARTS;
}

static struct mtd_part_parser mtdsplit_wrgg_parser = {
	.owner = THIS_MODULE,
	.name = "wrgg-fw",
	.parse_fn = mtdsplit_parse_wrgg,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_wrgg_init(void)
{
	register_mtd_parser(&mtdsplit_wrgg_parser);

	return 0;
}

subsys_initcall(mtdsplit_wrgg_init);
