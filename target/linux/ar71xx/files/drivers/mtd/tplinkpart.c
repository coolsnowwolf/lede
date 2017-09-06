/*
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/magic.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#define TPLINK_NUM_PARTS	5
#define TPLINK_HEADER_V1	0x01000000
#define TPLINK_HEADER_V2	0x02000000
#define MD5SUM_LEN		16

#define TPLINK_ART_LEN		0x10000
#define TPLINK_KERNEL_OFFS	0x20000
#define TPLINK_64K_KERNEL_OFFS	0x10000

struct tplink_fw_header {
	uint32_t	version;	/* header version */
	char		vendor_name[24];
	char		fw_version[36];
	uint32_t	hw_id;		/* hardware id */
	uint32_t	hw_rev;		/* hardware revision */
	uint32_t	unk1;
	uint8_t		md5sum1[MD5SUM_LEN];
	uint32_t	unk2;
	uint8_t		md5sum2[MD5SUM_LEN];
	uint32_t	unk3;
	uint32_t	kernel_la;	/* kernel load address */
	uint32_t	kernel_ep;	/* kernel entry point */
	uint32_t	fw_length;	/* total length of the firmware */
	uint32_t	kernel_ofs;	/* kernel data offset */
	uint32_t	kernel_len;	/* kernel data length */
	uint32_t	rootfs_ofs;	/* rootfs data offset */
	uint32_t	rootfs_len;	/* rootfs data length */
	uint32_t	boot_ofs;	/* bootloader data offset */
	uint32_t	boot_len;	/* bootloader data length */
	uint8_t		pad[360];
} __attribute__ ((packed));

static struct tplink_fw_header *
tplink_read_header(struct mtd_info *mtd, size_t offset)
{
	struct tplink_fw_header *header;
	size_t header_len;
	size_t retlen;
	int ret;
	u32 t;

	header = vmalloc(sizeof(*header));
	if (!header)
		goto err;

	header_len = sizeof(struct tplink_fw_header);
	ret = mtd_read(mtd, offset, header_len, &retlen,
		       (unsigned char *) header);
	if (ret)
		goto err_free_header;

	if (retlen != header_len)
		goto err_free_header;

	/* sanity checks */
	t = be32_to_cpu(header->version);
	if ((t != TPLINK_HEADER_V1) && (t != TPLINK_HEADER_V2))
		goto err_free_header;

	t = be32_to_cpu(header->kernel_ofs);
	if (t != header_len)
		goto err_free_header;

	return header;

err_free_header:
	vfree(header);
err:
	return NULL;
}

static int tplink_check_rootfs_magic(struct mtd_info *mtd, size_t offset)
{
	u32 magic;
	size_t retlen;
	int ret;

	ret = mtd_read(mtd, offset, sizeof(magic), &retlen,
		       (unsigned char *) &magic);
	if (ret)
		return ret;

	if (retlen != sizeof(magic))
		return -EIO;

	if (le32_to_cpu(magic) != SQUASHFS_MAGIC &&
	    magic != 0x19852003)
		return -EINVAL;

	return 0;
}

static int tplink_parse_partitions_offset(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data,
				   size_t offset)
{
	struct mtd_partition *parts;
	struct tplink_fw_header *header;
	int nr_parts;
	size_t art_offset;
	size_t rootfs_offset;
	size_t squashfs_offset;
	int ret;

	nr_parts = TPLINK_NUM_PARTS;
	parts = kzalloc(nr_parts * sizeof(struct mtd_partition), GFP_KERNEL);
	if (!parts) {
		ret = -ENOMEM;
		goto err;
	}

	header = tplink_read_header(master, offset);
	if (!header) {
		pr_notice("%s: no TP-Link header found\n", master->name);
		ret = -ENODEV;
		goto err_free_parts;
	}

	squashfs_offset = offset + sizeof(struct tplink_fw_header) +
			  be32_to_cpu(header->kernel_len);

	ret = tplink_check_rootfs_magic(master, squashfs_offset);
	if (ret == 0)
		rootfs_offset = squashfs_offset;
	else
		rootfs_offset = offset + be32_to_cpu(header->rootfs_ofs);

	art_offset = master->size - TPLINK_ART_LEN;

	parts[0].name = "u-boot";
	parts[0].offset = 0;
	parts[0].size = offset;
	parts[0].mask_flags = MTD_WRITEABLE;

	parts[1].name = "kernel";
	parts[1].offset = offset;
	parts[1].size = rootfs_offset - offset;

	parts[2].name = "rootfs";
	parts[2].offset = rootfs_offset;
	parts[2].size = art_offset - rootfs_offset;

	parts[3].name = "art";
	parts[3].offset = art_offset;
	parts[3].size = TPLINK_ART_LEN;
	parts[3].mask_flags = MTD_WRITEABLE;

	parts[4].name = "firmware";
	parts[4].offset = offset;
	parts[4].size = art_offset - offset;

	vfree(header);

	*pparts = parts;
	return nr_parts;

err_free_parts:
	kfree(parts);
err:
	*pparts = NULL;
	return ret;
}

static int tplink_parse_partitions(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	return tplink_parse_partitions_offset(master, pparts, data,
		                              TPLINK_KERNEL_OFFS);
}

static int tplink_parse_64k_partitions(struct mtd_info *master,
				   struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	return tplink_parse_partitions_offset(master, pparts, data,
		                              TPLINK_64K_KERNEL_OFFS);
}

static struct mtd_part_parser tplink_parser = {
	.owner		= THIS_MODULE,
	.parse_fn	= tplink_parse_partitions,
	.name		= "tp-link",
};

static struct mtd_part_parser tplink_64k_parser = {
	.owner		= THIS_MODULE,
	.parse_fn	= tplink_parse_64k_partitions,
	.name		= "tp-link-64k",
};

static int __init tplink_parser_init(void)
{
	register_mtd_parser(&tplink_parser);
	register_mtd_parser(&tplink_64k_parser);

	return 0;
}

module_init(tplink_parser_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
