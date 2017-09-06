/*
 *  Parse MyLoader-style flash partition tables and produce a Linux partition
 *  array to match.
 *
 *  Copyright (C) 2007-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This file was based on drivers/mtd/redboot.c
 *  Author: Red Hat, Inc. - David Woodhouse <dwmw2@cambridge.redhat.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>
#include <linux/myloader.h>

#define BLOCK_LEN_MIN		0x10000
#define PART_NAME_LEN		32

struct part_data {
	struct mylo_partition_table	tab;
	char names[MYLO_MAX_PARTITIONS][PART_NAME_LEN];
};

static int myloader_parse_partitions(struct mtd_info *master,
				     const struct mtd_partition **pparts,
				     struct mtd_part_parser_data *data)
{
	struct part_data *buf;
	struct mylo_partition_table *tab;
	struct mylo_partition *part;
	struct mtd_partition *mtd_parts;
	struct mtd_partition *mtd_part;
	int num_parts;
	int ret, i;
	size_t retlen;
	char *names;
	unsigned long offset;
	unsigned long blocklen;

	buf = vmalloc(sizeof(*buf));
	if (!buf) {
		return -ENOMEM;
		goto out;
	}
	tab = &buf->tab;

	blocklen = master->erasesize;
	if (blocklen < BLOCK_LEN_MIN)
		blocklen = BLOCK_LEN_MIN;

	offset = blocklen;

	/* Find the partition table */
	for (i = 0; i < 4; i++, offset += blocklen) {
		printk(KERN_DEBUG "%s: searching for MyLoader partition table"
				" at offset 0x%lx\n", master->name, offset);

		ret = mtd_read(master, offset, sizeof(*buf), &retlen,
			       (void *)buf);
		if (ret)
			goto out_free_buf;

		if (retlen != sizeof(*buf)) {
			ret = -EIO;
			goto out_free_buf;
		}

		/* Check for Partition Table magic number */
		if (tab->magic == le32_to_cpu(MYLO_MAGIC_PARTITIONS))
			break;

	}

	if (tab->magic != le32_to_cpu(MYLO_MAGIC_PARTITIONS)) {
		printk(KERN_DEBUG "%s: no MyLoader partition table found\n",
			master->name);
		ret = 0;
		goto out_free_buf;
	}

	/* The MyLoader and the Partition Table is always present */
	num_parts = 2;

	/* Detect number of used partitions */
	for (i = 0; i < MYLO_MAX_PARTITIONS; i++) {
		part = &tab->partitions[i];

		if (le16_to_cpu(part->type) == PARTITION_TYPE_FREE)
			continue;

		num_parts++;
	}

	mtd_parts = kzalloc((num_parts * sizeof(*mtd_part) +
				num_parts * PART_NAME_LEN), GFP_KERNEL);

	if (!mtd_parts) {
		ret = -ENOMEM;
		goto out_free_buf;
	}

	mtd_part = mtd_parts;
	names = (char *)&mtd_parts[num_parts];

	strncpy(names, "myloader", PART_NAME_LEN);
	mtd_part->name = names;
	mtd_part->offset = 0;
	mtd_part->size = offset;
	mtd_part->mask_flags = MTD_WRITEABLE;
	mtd_part++;
	names += PART_NAME_LEN;

	strncpy(names, "partition_table", PART_NAME_LEN);
	mtd_part->name = names;
	mtd_part->offset = offset;
	mtd_part->size = blocklen;
	mtd_part->mask_flags = MTD_WRITEABLE;
	mtd_part++;
	names += PART_NAME_LEN;

	for (i = 0; i < MYLO_MAX_PARTITIONS; i++) {
		part = &tab->partitions[i];

		if (le16_to_cpu(part->type) == PARTITION_TYPE_FREE)
			continue;

		if ((buf->names[i][0]) && (buf->names[i][0] != '\xff'))
			strncpy(names, buf->names[i], PART_NAME_LEN);
		else
			snprintf(names, PART_NAME_LEN, "partition%d", i);

		mtd_part->offset = le32_to_cpu(part->addr);
		mtd_part->size = le32_to_cpu(part->size);
		mtd_part->name = names;
		mtd_part++;
		names += PART_NAME_LEN;
	}

	*pparts = mtd_parts;
	ret = num_parts;

 out_free_buf:
	vfree(buf);
 out:
	return ret;
}

static struct mtd_part_parser myloader_mtd_parser = {
	.owner		= THIS_MODULE,
	.parse_fn	= myloader_parse_partitions,
	.name		= "MyLoader",
};

static int __init myloader_mtd_parser_init(void)
{
	register_mtd_parser(&myloader_mtd_parser);

	return 0;
}

static void __exit myloader_mtd_parser_exit(void)
{
	deregister_mtd_parser(&myloader_mtd_parser);
}

module_init(myloader_mtd_parser_init);
module_exit(myloader_mtd_parser_exit);

MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION("Parsing code for MyLoader partition tables");
MODULE_LICENSE("GPL v2");
