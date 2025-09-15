/*
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
#include <linux/version.h>
#include <linux/byteorder/generic.h>
#include <linux/of.h>
#include <dt-bindings/mtd/partitions/uimage.h>

#include "mtdsplit.h"

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
struct uimage_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
};

static int
read_uimage_header(struct mtd_info *mtd, size_t offset, u_char *buf,
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

static void uimage_parse_dt(struct mtd_info *master, int *extralen,
			    u32 *ih_magic, u32 *ih_type,
			    u32 *header_offset, u32 *part_magic)
{
	struct device_node *np = mtd_get_of_node(master);

	if (!np || !of_device_is_compatible(np, "openwrt,uimage"))
		return;

	if (!of_property_read_u32(np, "openwrt,padding", extralen))
		pr_debug("got openwrt,padding=%d from device-tree\n", *extralen);
	if (!of_property_read_u32(np, "openwrt,ih-magic", ih_magic))
		pr_debug("got openwrt,ih-magic=%08x from device-tree\n", *ih_magic);
	if (!of_property_read_u32(np, "openwrt,ih-type", ih_type))
		pr_debug("got openwrt,ih-type=%08x from device-tree\n", *ih_type);
	if (!of_property_read_u32(np, "openwrt,offset", header_offset))
		pr_debug("got ih-start=%u from device-tree\n", *header_offset);
	if (!of_property_read_u32(np, "openwrt,partition-magic", part_magic))
		pr_debug("got openwrt,partition-magic=%08x from device-tree\n", *part_magic);
}

static ssize_t uimage_verify_default(u_char *buf, u32 ih_magic, u32 ih_type)
{
	struct uimage_header *header = (struct uimage_header *)buf;

	/* default sanity checks */
	if (be32_to_cpu(header->ih_magic) != ih_magic) {
		pr_debug("invalid uImage magic: %08x != %08x\n",
			 be32_to_cpu(header->ih_magic), ih_magic);
		return -EINVAL;
	}

	if (header->ih_os != IH_OS_LINUX) {
		pr_debug("invalid uImage OS: %08x != %08x\n",
			 be32_to_cpu(header->ih_os), IH_OS_LINUX);
		return -EINVAL;
	}

	if (header->ih_type != ih_type) {
		pr_debug("invalid uImage type: %08x != %08x\n",
			 be32_to_cpu(header->ih_type), ih_type);
		return -EINVAL;
	}

	return 0;
}

/**
 * __mtdsplit_parse_uimage - scan partition and create kernel + rootfs parts
 *
 * @find_header: function to call for a block of data that will return offset
 *      and tail padding length of a valid uImage header if found
 */
static int __mtdsplit_parse_uimage(struct mtd_info *master,
				   const struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	u_char *buf;
	int nr_parts;
	size_t offset;
	size_t uimage_offset;
	size_t uimage_size = 0;
	size_t rootfs_offset;
	size_t rootfs_size = 0;
	size_t buflen;
	int uimage_part, rf_part;
	int ret;
	int extralen = 0;
	u32 ih_magic = IH_MAGIC;
	u32 ih_type = IH_TYPE_KERNEL;
	u32 header_offset = 0;
	u32 part_magic = 0;
	enum mtdsplit_part_type type;

	nr_parts = 2;
	parts = kzalloc(nr_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	uimage_parse_dt(master, &extralen, &ih_magic, &ih_type, &header_offset, &part_magic);
	buflen = sizeof(struct uimage_header) + header_offset;
	buf = vmalloc(buflen);
	if (!buf) {
		ret = -ENOMEM;
		goto err_free_parts;
	}

	/* find uImage on erase block boundaries */
	for (offset = 0; offset < master->size; offset += master->erasesize) {
		struct uimage_header *header;

		uimage_size = 0;

		ret = read_uimage_header(master, offset, buf, buflen);
		if (ret)
			continue;

		/* verify optional partition magic before uimage header */
		if (header_offset && part_magic && (be32_to_cpu(*(u32 *)buf) != part_magic))
			continue;

		ret = uimage_verify_default(buf + header_offset, ih_magic, ih_type);
		if (ret < 0) {
			pr_debug("no valid uImage found in \"%s\" at offset %llx\n",
				 master->name, (unsigned long long) offset);
			continue;
		}

		header = (struct uimage_header *)(buf + header_offset);

		uimage_size = sizeof(*header) +
				be32_to_cpu(header->ih_size) + header_offset + extralen;

		if ((offset + uimage_size) > master->size) {
			pr_debug("uImage exceeds MTD device \"%s\"\n",
				 master->name);
			continue;
		}
		break;
	}

	if (uimage_size == 0) {
		pr_info("no uImage found in \"%s\"\n", master->name);
		ret = 0;
		goto err_free_buf;
	}

	uimage_offset = offset;

	if (uimage_offset == 0) {
		uimage_part = 0;
		rf_part = 1;

		/* find the roots after the uImage */
		ret = mtd_find_rootfs_from(master, uimage_offset + uimage_size,
					   master->size, &rootfs_offset, &type);
		if (ret) {
			pr_info("no rootfs after uImage in \"%s\"\n", master->name);
			ret = 0;
			goto err_free_buf;
		}

		rootfs_size = master->size - rootfs_offset;
		uimage_size = rootfs_offset - uimage_offset;
	} else {
		rf_part = 0;
		uimage_part = 1;

		/* check rootfs presence at offset 0 */
		ret = mtd_check_rootfs_magic(master, 0, &type);
		if (ret) {
			pr_info("no rootfs before uImage in \"%s\"\n", master->name);
			ret = 0;
			goto err_free_buf;
		}

		rootfs_offset = 0;
		rootfs_size = uimage_offset;
	}

	if (rootfs_size == 0) {
		pr_debug("no rootfs found in \"%s\"\n", master->name);
		ret = -ENODEV;
		goto err_free_buf;
	}

	parts[uimage_part].name = KERNEL_PART_NAME;
	parts[uimage_part].offset = uimage_offset;
	parts[uimage_part].size = uimage_size;

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

static const struct of_device_id mtdsplit_uimage_of_match_table[] = {
	{ .compatible = "denx,uimage" },
	{ .compatible = "openwrt,uimage" },
	{},
};

static struct mtd_part_parser uimage_generic_parser = {
	.owner = THIS_MODULE,
	.name = "uimage-fw",
	.of_match_table = mtdsplit_uimage_of_match_table,
	.parse_fn = __mtdsplit_parse_uimage,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

/**************************************************
 * Init
 **************************************************/

static int __init mtdsplit_uimage_init(void)
{
	register_mtd_parser(&uimage_generic_parser);

	return 0;
}

module_init(mtdsplit_uimage_init);
