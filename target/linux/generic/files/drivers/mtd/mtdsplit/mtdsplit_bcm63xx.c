/*
 * Firmware MTD split for BCM63XX, based on bcm63xxpart.c
 *
 * Copyright (C) 2006-2008 Florian Fainelli <florian@openwrt.org>
 * Copyright (C) 2006-2008 Mike Albon <malbon@openwrt.org>
 * Copyright (C) 2009-2010 Daniel Dickinson <openwrt@cshore.neomailbox.net>
 * Copyright (C) 2011-2013 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2015 Simon Arlott <simon@fire.lp0.eu>
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/bcm963xx_tag.h>
#include <linux/crc32.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/byteorder/generic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include "mtdsplit.h"

/* Ensure strings read from flash structs are null terminated */
#define STR_NULL_TERMINATE(x) \
	do { char *_str = (x); _str[sizeof(x) - 1] = 0; } while (0)

#define BCM63XX_NR_PARTS 2

static int bcm63xx_read_image_tag(struct mtd_info *master, loff_t offset,
				  struct bcm_tag *hdr)
{
	int ret;
	size_t retlen;
	u32 computed_crc;

	ret = mtd_read(master, offset, sizeof(*hdr), &retlen, (void *) hdr);
	if (ret)
		return ret;

	if (retlen != sizeof(*hdr))
		return -EIO;

	computed_crc = crc32_le(IMAGETAG_CRC_START, (u8 *)hdr,
				offsetof(struct bcm_tag, header_crc));
	if (computed_crc == hdr->header_crc) {
	    STR_NULL_TERMINATE(hdr->board_id);
	    STR_NULL_TERMINATE(hdr->tag_version);

		pr_info("CFE image tag found at 0x%llx with version %s, "
			"board type %s\n", offset, hdr->tag_version,
			hdr->board_id);

		return 0;
	} else {
		pr_err("CFE image tag at 0x%llx CRC invalid "
		       "(expected %08x, actual %08x)\n",
		       offset, hdr->header_crc, computed_crc);

		return 1;
	}
}

static int bcm63xx_parse_partitions(struct mtd_info *master,
				    const struct mtd_partition **pparts,
				    struct bcm_tag *hdr)
{
	struct mtd_partition *parts;
	unsigned int flash_image_start;
	unsigned int kernel_address;
	unsigned int kernel_length;
	size_t kernel_offset = 0, kernel_size = 0;
	size_t rootfs_offset = 0, rootfs_size = 0;
	int kernel_part, rootfs_part;

	STR_NULL_TERMINATE(hdr->flash_image_start);
	if (kstrtouint(hdr->flash_image_start, 10, &flash_image_start) ||
	    flash_image_start < BCM963XX_EXTENDED_SIZE) {
		pr_err("invalid rootfs address: %*ph\n",
		       (int) sizeof(hdr->flash_image_start),
		       hdr->flash_image_start);
		return -EINVAL;
	}

	STR_NULL_TERMINATE(hdr->kernel_address);
	if (kstrtouint(hdr->kernel_address, 10, &kernel_address) ||
	    kernel_address < BCM963XX_EXTENDED_SIZE) {
		pr_err("invalid kernel address: %*ph\n",
		       (int) sizeof(hdr->kernel_address), hdr->kernel_address);
		return -EINVAL;
	}

	STR_NULL_TERMINATE(hdr->kernel_length);
	if (kstrtouint(hdr->kernel_length, 10, &kernel_length) ||
	    !kernel_length) {
		pr_err("invalid kernel length: %*ph\n",
		       (int) sizeof(hdr->kernel_length), hdr->kernel_length);
		return -EINVAL;
	}

	kernel_offset = kernel_address - BCM963XX_EXTENDED_SIZE -
			mtdpart_get_offset(master);
	kernel_size = kernel_length;

	if (flash_image_start < kernel_address) {
		/* rootfs first */
		rootfs_part = 0;
		kernel_part = 1;
		rootfs_offset = flash_image_start - BCM963XX_EXTENDED_SIZE -
				mtdpart_get_offset(master);
		rootfs_size = kernel_offset - rootfs_offset;
	} else {
		/* kernel first */
		kernel_part = 0;
		rootfs_part = 1;
		rootfs_offset = kernel_offset + kernel_size;
		rootfs_size = master->size - rootfs_offset;
	}

	if (mtd_check_rootfs_magic(master, rootfs_offset, NULL))
		pr_warn("rootfs magic not found\n");

	parts = kzalloc(BCM63XX_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[kernel_part].name = KERNEL_PART_NAME;
	parts[kernel_part].offset = kernel_offset;
	parts[kernel_part].size = kernel_size;

	parts[rootfs_part].name = ROOTFS_PART_NAME;
	parts[rootfs_part].offset = rootfs_offset;
	parts[rootfs_part].size = rootfs_size;

	*pparts = parts;
	return BCM63XX_NR_PARTS;
}

static int mtdsplit_parse_bcm63xx(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	struct bcm_tag hdr;
	loff_t offset;

	if (mtd_type_is_nand(master))
		return -EINVAL;

	/* find bcm63xx_cfe image on erase block boundaries */
	for (offset = 0; offset < master->size; offset += master->erasesize) {
		if (!bcm63xx_read_image_tag(master, offset, (void *) &hdr))
			return bcm63xx_parse_partitions(master, pparts,
							(void *) &hdr);
	}

	return -EINVAL;
}

static const struct of_device_id mtdsplit_fit_of_match_table[] = {
	{ .compatible = "brcm,bcm963xx-imagetag" },
	{ },
};

static struct mtd_part_parser mtdsplit_bcm63xx_parser = {
	.owner = THIS_MODULE,
	.name = "bcm63xx-fw",
	.of_match_table = mtdsplit_fit_of_match_table,
	.parse_fn = mtdsplit_parse_bcm63xx,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_bcm63xx_init(void)
{
	register_mtd_parser(&mtdsplit_bcm63xx_parser);

	return 0;
}

module_init(mtdsplit_bcm63xx_init);
