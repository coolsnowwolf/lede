/*
 *  Copyright (C) 2018 Paweł Dembicki <paweldembicki@gmail.com> 
 *
 *  Based on: mtdsplit_uimage.c
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
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
#include <linux/vmalloc.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define MAX_HEADER_LEN ( STAG_SIZE + SCH2_SIZE )

#define STAG_SIZE 16
#define STAG_ID 0x04
#define STAG_MAGIC 0x2B24

#define SCH2_SIZE 40
#define SCH2_MAGIC 0x2124
#define SCH2_VER 0x02

/*
 * Jboot image header,
 * all data in little endian.
 */

struct jimage_header		//stag + sch2 jboot joined headers
{
	uint8_t stag_cmark;		// in factory 0xFF , in sysupgrade must be the same as stag_id
	uint8_t stag_id;		// 0x04
	uint16_t stag_magic;		//magic 0x2B24
	uint32_t stag_time_stamp;	// timestamp calculated in jboot way
	uint32_t stag_image_length;	// lentgh of kernel + sch2 header
	uint16_t stag_image_checksum;	// negated jboot_checksum of sch2 + kernel
	uint16_t stag_tag_checksum;	// negated jboot_checksum of stag header data
	uint16_t sch2_magic;		// magic 0x2124
	uint8_t sch2_cp_type;	// 0x00 for flat, 0x01 for jz, 0x02 for gzip, 0x03 for lzma
	uint8_t sch2_version;	// 0x02 for sch2
	uint32_t sch2_ram_addr;	// ram entry address
	uint32_t sch2_image_len;	// kernel image length
	uint32_t sch2_image_crc32;	// kernel image crc
	uint32_t sch2_start_addr;	// ram start address
	uint32_t sch2_rootfs_addr;	// rootfs flash address
	uint32_t sch2_rootfs_len;	// rootfls length
	uint32_t sch2_rootfs_crc32;	// rootfs crc32
	uint32_t sch2_header_crc32;	// sch2 header crc32, durring calculation this area is replaced by zero
	uint16_t sch2_header_length;	// sch2 header length: 0x28
	uint16_t sch2_cmd_line_length;	// cmd line length, known zeros
};

static int
read_jimage_header(struct mtd_info *mtd, size_t offset, u_char *buf,
		   size_t header_len)
{
	size_t retlen;
	int ret;

	ret = mtd_read(mtd, offset, header_len, &retlen, buf);
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

/**
 * __mtdsplit_parse_jimage - scan partition and create kernel + rootfs parts
 *
 * @find_header: function to call for a block of data that will return offset
 *      of a valid jImage header if found
 */
static int __mtdsplit_parse_jimage(struct mtd_info *master,
				   const struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data,
				   ssize_t (*find_header)(u_char *buf, size_t len))
{
	struct mtd_partition *parts;
	u_char *buf;
	int nr_parts;
	size_t offset;
	size_t jimage_offset;
	size_t jimage_size = 0;
	size_t rootfs_offset;
	size_t rootfs_size = 0;
	int jimage_part, rf_part;
	int ret;
	enum mtdsplit_part_type type;

	nr_parts = 2;
	parts = kzalloc(nr_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	buf = vmalloc(MAX_HEADER_LEN);
	if (!buf) {
		ret = -ENOMEM;
		goto err_free_parts;
	}

	/* find jImage on erase block boundaries */
	for (offset = 0; offset < master->size; offset += master->erasesize) {
		struct jimage_header *header;

		jimage_size = 0;

		ret = read_jimage_header(master, offset, buf, MAX_HEADER_LEN);
		if (ret)
			continue;

		ret = find_header(buf, MAX_HEADER_LEN);
		if (ret < 0) {
			pr_debug("no valid jImage found in \"%s\" at offset %llx\n",
				 master->name, (unsigned long long) offset);
			continue;
		}
		header = (struct jimage_header *)(buf + ret);

		jimage_size = sizeof(*header) + header->sch2_image_len + ret;
		if ((offset + jimage_size) > master->size) {
			pr_debug("jImage exceeds MTD device \"%s\"\n",
				 master->name);
			continue;
		}
		break;
	}

	if (jimage_size == 0) {
		pr_debug("no jImage found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err_free_buf;
	}

	jimage_offset = offset;

	if (jimage_offset == 0) {
		jimage_part = 0;
		rf_part = 1;

		/* find the roots after the jImage */
		ret = mtd_find_rootfs_from(master, jimage_offset + jimage_size,
					   master->size, &rootfs_offset, &type);
		if (ret) {
			pr_debug("no rootfs after jImage in \"%s\"\n",
				 master->name);
			goto err_free_buf;
		}

		rootfs_size = master->size - rootfs_offset;
		jimage_size = rootfs_offset - jimage_offset;
	} else {
		rf_part = 0;
		jimage_part = 1;

		/* check rootfs presence at offset 0 */
		ret = mtd_check_rootfs_magic(master, 0, &type);
		if (ret) {
			pr_debug("no rootfs before jImage in \"%s\"\n",
				 master->name);
			goto err_free_buf;
		}

		rootfs_offset = 0;
		rootfs_size = jimage_offset;
	}

	if (rootfs_size == 0) {
		pr_debug("no rootfs found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err_free_buf;
	}

	parts[jimage_part].name = KERNEL_PART_NAME;
	parts[jimage_part].offset = jimage_offset;
	parts[jimage_part].size = jimage_size;

	if (type == MTDSPLIT_PART_TYPE_UBI)
		parts[rf_part].name = UBI_PART_NAME;
	else
		parts[rf_part].name = ROOTFS_PART_NAME;
	parts[rf_part].offset = rootfs_offset;
	parts[rf_part].size = rootfs_size;

	vfree(buf);

	*pparts = parts;
	return nr_parts;

err_free_buf:
	vfree(buf);

err_free_parts:
	kfree(parts);
	return ret;
}

static ssize_t jimage_verify_default(u_char *buf, size_t len)
{
	struct jimage_header *header = (struct jimage_header *)buf;

	/* default sanity checks */
	if (header->stag_magic != STAG_MAGIC) {
		pr_debug("invalid jImage stag header magic: %04x\n",
			 header->stag_magic);
		return -EINVAL;
	}
	if (header->sch2_magic != SCH2_MAGIC) {
		pr_debug("invalid jImage sch2 header magic: %04x\n",
			 header->stag_magic);
		return -EINVAL;
	}
	if (header->stag_cmark != header->stag_id) {
		pr_debug("invalid jImage stag header cmark: %02x\n",
			 header->stag_magic);
		return -EINVAL;
	}
	if (header->stag_id != STAG_ID) {
		pr_debug("invalid jImage stag header id: %02x\n",
			 header->stag_magic);
		return -EINVAL;
	}
	if (header->sch2_version != SCH2_VER) {
		pr_debug("invalid jImage sch2 header version: %02x\n",
			 header->stag_magic);
		return -EINVAL;
	}

	return 0;
}

static int
mtdsplit_jimage_parse_generic(struct mtd_info *master,
			      const struct mtd_partition **pparts,
			      struct mtd_part_parser_data *data)
{
	return __mtdsplit_parse_jimage(master, pparts, data,
				      jimage_verify_default);
}

static const struct of_device_id mtdsplit_jimage_of_match_table[] = {
	{ .compatible = "amit,jimage" },
	{},
};

static struct mtd_part_parser jimage_generic_parser = {
	.owner = THIS_MODULE,
	.name = "jimage-fw",
	.of_match_table = mtdsplit_jimage_of_match_table,
	.parse_fn = mtdsplit_jimage_parse_generic,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

/**************************************************
 * Init
 **************************************************/

static int __init mtdsplit_jimage_init(void)
{
	register_mtd_parser(&jimage_generic_parser);

	return 0;
}

module_init(mtdsplit_jimage_init);
