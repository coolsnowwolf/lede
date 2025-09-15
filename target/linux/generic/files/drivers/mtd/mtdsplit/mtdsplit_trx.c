/*
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Felix Fietkau <nbd@nbd.name>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#define pr_fmt(fmt)	KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>
#include <linux/of.h>

#include "mtdsplit.h"

#define TRX_MAGIC   0x30524448  /* "HDR0" */

struct trx_header {
	__le32 magic;
	__le32 len;
	__le32 crc32;
	__le32 flag_version;
	__le32 offset[4];
};

static int
read_trx_header(struct mtd_info *mtd, size_t offset,
		   struct trx_header *header)
{
	size_t header_len;
	size_t retlen;
	int ret;

	header_len = sizeof(*header);
	ret = mtd_read(mtd, offset, header_len, &retlen,
		       (unsigned char *) header);
	if (ret) {
		pr_debug("read error in \"%s\"\n", mtd->name);
		return ret;
	}

	if (retlen != header_len) {
		pr_debug("short read in \"%s\"\n", mtd->name);
		return -EIO;
	}

	return 0;
}

static int
mtdsplit_parse_trx(struct mtd_info *master,
		   const struct mtd_partition **pparts,
		   struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct trx_header hdr;
	int nr_parts;
	size_t offset;
	size_t trx_offset;
	size_t trx_size = 0;
	size_t rootfs_offset;
	size_t rootfs_size = 0;
	int ret;

	nr_parts = 2;
	parts = kzalloc(nr_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	/* find trx image on erase block boundaries */
	for (offset = 0; offset < master->size; offset += master->erasesize) {
		trx_size = 0;

		ret = read_trx_header(master, offset, &hdr);
		if (ret)
			continue;

		if (hdr.magic != cpu_to_le32(TRX_MAGIC)) {
			pr_debug("no valid trx header found in \"%s\" at offset %llx\n",
				 master->name, (unsigned long long) offset);
			continue;
		}

		trx_size = le32_to_cpu(hdr.len);
		if ((offset + trx_size) > master->size) {
			pr_debug("trx image exceeds MTD device \"%s\"\n",
				 master->name);
			continue;
		}
		break;
	}

	if (trx_size == 0) {
		pr_debug("no trx header found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err;
	}

	trx_offset = offset + hdr.offset[0];
	rootfs_offset = offset + hdr.offset[1];
	rootfs_size = master->size - rootfs_offset;
	trx_size = rootfs_offset - trx_offset;

	if (rootfs_size == 0) {
		pr_debug("no rootfs found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err;
	}

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = trx_offset;
	parts[0].size = trx_size;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = rootfs_size;

	*pparts = parts;
	return nr_parts;

err:
	kfree(parts);
	return ret;
}

static const struct of_device_id trx_parser_of_match_table[] = {
	{ .compatible = "openwrt,trx" },
	{},
};
MODULE_DEVICE_TABLE(of, trx_parser_of_match_table);

static struct mtd_part_parser trx_parser = {
	.owner = THIS_MODULE,
	.name = "trx-fw",
	.of_match_table = trx_parser_of_match_table,
	.parse_fn = mtdsplit_parse_trx,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_trx_init(void)
{
	register_mtd_parser(&trx_parser);

	return 0;
}

module_init(mtdsplit_trx_init);
