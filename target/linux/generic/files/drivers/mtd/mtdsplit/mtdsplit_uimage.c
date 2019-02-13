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

#include "mtdsplit.h"

/*
 * uimage_header itself is only 64B, but it may be prepended with another data.
 * Currently the biggest size is for Edimax devices: 20B + 64B
 */
#define MAX_HEADER_LEN		84

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

#define IH_OS_LINUX		5	/* Linux	*/

#define IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define IH_TYPE_FILESYSTEM	7	/* Filesystem Image		*/

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

/**
 * __mtdsplit_parse_uimage - scan partition and create kernel + rootfs parts
 *
 * @find_header: function to call for a block of data that will return offset
 *      of a valid uImage header if found
 */
static int __mtdsplit_parse_uimage(struct mtd_info *master,
				   const struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data,
				   ssize_t (*find_header)(u_char *buf, size_t len))
{
	struct mtd_partition *parts;
	u_char *buf;
	int nr_parts;
	size_t offset;
	size_t uimage_offset;
	size_t uimage_size = 0;
	size_t rootfs_offset;
	size_t rootfs_size = 0;
	int uimage_part, rf_part;
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

	/* find uImage on erase block boundaries */
	for (offset = 0; offset < master->size; offset += master->erasesize) {
		struct uimage_header *header;

		uimage_size = 0;

		ret = read_uimage_header(master, offset, buf, MAX_HEADER_LEN);
		if (ret)
			continue;

		ret = find_header(buf, MAX_HEADER_LEN);
		if (ret < 0) {
			pr_debug("no valid uImage found in \"%s\" at offset %llx\n",
				 master->name, (unsigned long long) offset);
			continue;
		}
		header = (struct uimage_header *)(buf + ret);

		uimage_size = sizeof(*header) + be32_to_cpu(header->ih_size) + ret;
		if ((offset + uimage_size) > master->size) {
			pr_debug("uImage exceeds MTD device \"%s\"\n",
				 master->name);
			continue;
		}
		break;
	}

	if (uimage_size == 0) {
		pr_debug("no uImage found in \"%s\"\n", master->name);
		ret = -ENODEV;
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
			pr_debug("no rootfs after uImage in \"%s\"\n",
				 master->name);
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
			pr_debug("no rootfs before uImage in \"%s\"\n",
				 master->name);
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

static ssize_t uimage_verify_default(u_char *buf, size_t len)
{
	struct uimage_header *header = (struct uimage_header *)buf;

	/* default sanity checks */
	if (be32_to_cpu(header->ih_magic) != IH_MAGIC) {
		pr_debug("invalid uImage magic: %08x\n",
			 be32_to_cpu(header->ih_magic));
		return -EINVAL;
	}

	if (header->ih_os != IH_OS_LINUX) {
		pr_debug("invalid uImage OS: %08x\n",
			 be32_to_cpu(header->ih_os));
		return -EINVAL;
	}

	if (header->ih_type != IH_TYPE_KERNEL) {
		pr_debug("invalid uImage type: %08x\n",
			 be32_to_cpu(header->ih_type));
		return -EINVAL;
	}

	return 0;
}

static int
mtdsplit_uimage_parse_generic(struct mtd_info *master,
			      const struct mtd_partition **pparts,
			      struct mtd_part_parser_data *data)
{
	return __mtdsplit_parse_uimage(master, pparts, data,
				      uimage_verify_default);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static const struct of_device_id mtdsplit_uimage_of_match_table[] = {
	{ .compatible = "denx,uimage" },
	{},
};
#endif

static struct mtd_part_parser uimage_generic_parser = {
	.owner = THIS_MODULE,
	.name = "uimage-fw",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	.of_match_table = mtdsplit_uimage_of_match_table,
#endif
	.parse_fn = mtdsplit_uimage_parse_generic,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

#define FW_MAGIC_WNR2000V1	0x32303031
#define FW_MAGIC_WNR2000V3	0x32303033
#define FW_MAGIC_WNR2000V4	0x32303034
#define FW_MAGIC_WNR2200	0x32323030
#define FW_MAGIC_WNR612V2	0x32303631
#define FW_MAGIC_WNR1000V2	0x31303031
#define FW_MAGIC_WNR1000V2_VC	0x31303030
#define FW_MAGIC_WNDR3700	0x33373030
#define FW_MAGIC_WNDR3700V2	0x33373031
#define FW_MAGIC_WPN824N	0x31313030

static ssize_t uimage_verify_wndr3700(u_char *buf, size_t len)
{
	struct uimage_header *header = (struct uimage_header *)buf;
	uint8_t expected_type = IH_TYPE_FILESYSTEM;

	switch (be32_to_cpu(header->ih_magic)) {
	case FW_MAGIC_WNR612V2:
	case FW_MAGIC_WNR1000V2:
	case FW_MAGIC_WNR1000V2_VC:
	case FW_MAGIC_WNR2000V1:
	case FW_MAGIC_WNR2000V3:
	case FW_MAGIC_WNR2200:
	case FW_MAGIC_WNDR3700:
	case FW_MAGIC_WNDR3700V2:
	case FW_MAGIC_WPN824N:
		break;
	case FW_MAGIC_WNR2000V4:
		expected_type = IH_TYPE_KERNEL;
		break;
	default:
		return -EINVAL;
	}

	if (header->ih_os != IH_OS_LINUX ||
	    header->ih_type != expected_type)
		return -EINVAL;

	return 0;
}

static int
mtdsplit_uimage_parse_netgear(struct mtd_info *master,
			      const struct mtd_partition **pparts,
			      struct mtd_part_parser_data *data)
{
	return __mtdsplit_parse_uimage(master, pparts, data,
				      uimage_verify_wndr3700);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static const struct of_device_id mtdsplit_uimage_netgear_of_match_table[] = {
	{ .compatible = "netgear,uimage" },
	{},
};
#endif

static struct mtd_part_parser uimage_netgear_parser = {
	.owner = THIS_MODULE,
	.name = "netgear-fw",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	.of_match_table = mtdsplit_uimage_netgear_of_match_table,
#endif
	.parse_fn = mtdsplit_uimage_parse_netgear,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

/**************************************************
 * Edimax
 **************************************************/

#define FW_EDIMAX_OFFSET	20
#define FW_MAGIC_EDIMAX		0x43535953

static ssize_t uimage_find_edimax(u_char *buf, size_t len)
{
	u32 *magic;

	if (len < FW_EDIMAX_OFFSET + sizeof(struct uimage_header)) {
		pr_err("Buffer too small for checking Edimax header\n");
		return -ENOSPC;
	}

	magic = (u32 *)buf;
	if (be32_to_cpu(*magic) != FW_MAGIC_EDIMAX)
		return -EINVAL;

	if (!uimage_verify_default(buf + FW_EDIMAX_OFFSET, len))
		return FW_EDIMAX_OFFSET;

	return -EINVAL;
}

static int
mtdsplit_uimage_parse_edimax(struct mtd_info *master,
			      const struct mtd_partition **pparts,
			      struct mtd_part_parser_data *data)
{
	return __mtdsplit_parse_uimage(master, pparts, data,
				       uimage_find_edimax);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static const struct of_device_id mtdsplit_uimage_edimax_of_match_table[] = {
	{ .compatible = "edimax,uimage" },
	{},
};
#endif

static struct mtd_part_parser uimage_edimax_parser = {
	.owner = THIS_MODULE,
	.name = "edimax-fw",
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	.of_match_table = mtdsplit_uimage_edimax_of_match_table,
#endif
	.parse_fn = mtdsplit_uimage_parse_edimax,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

/**************************************************
 * Init
 **************************************************/

static int __init mtdsplit_uimage_init(void)
{
	register_mtd_parser(&uimage_generic_parser);
	register_mtd_parser(&uimage_netgear_parser);
	register_mtd_parser(&uimage_edimax_parser);

	return 0;
}

module_init(mtdsplit_uimage_init);
