/*
 * Copyright (C) 2009 Christian Daniel <cd@maintech.de>
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * TRX flash partition table.
 * Based on ar7 map by Felix Fietkau <nbd@nbd.name>
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/version.h>

struct cybertan_header {
	char	magic[4];
	u8	res1[4];
	char	fw_date[3];
	char	fw_ver[3];
	char	id[4];
	char	hw_ver;
	char	unused;
	u8	flags[2];
	u8	res2[10];
} __packed;

#define TRX_PARTS	3
#define TRX_MAGIC	0x30524448
#define TRX_MAX_OFFSET	3

struct trx_header {
	__le32 magic;           /* "HDR0" */
	__le32 len;             /* Length of file including header */
	__le32 crc32;           /* 32-bit CRC from flag_version to end of file */
	__le32 flag_version;    /* 0:15 flags, 16:31 version */
	__le32 offsets[TRX_MAX_OFFSET]; /* Offsets of partitions from start of header */
} __packed;

#define IH_MAGIC	0x27051956	/* Image Magic Number */
#define IH_NMLEN	32		/* Image Name Length */

struct uimage_header {
	__be32	ih_magic;	/* Image Header Magic Number */
	__be32	ih_hcrc;	/* Image Header CRC Checksum */
	__be32	ih_time;	/* Image Creation Timestamp */
	__be32	ih_size;	/* Image Data Size */
	__be32	ih_load;	/* DataÂ» Load  Address */
	__be32	ih_ep;		/* Entry Point Address */
	__be32	ih_dcrc;	/* Image Data CRC Checksum */
	uint8_t	ih_os;		/* Operating System */
	uint8_t	ih_arch;	/* CPU architecture */
	uint8_t	ih_type;	/* Image Type */
	uint8_t	ih_comp;	/* Compression Type */
	uint8_t	ih_name[IH_NMLEN];	/* Image Name */
} __packed;

struct firmware_header {
	struct cybertan_header	cybertan;
	struct trx_header	trx;
	struct uimage_header	uimage;
} __packed;

static int cybertan_parse_partitions(struct mtd_info *master,
				     const struct mtd_partition **pparts,
				     struct mtd_part_parser_data *data)
{
	struct firmware_header header;
	struct trx_header *theader;
	struct uimage_header *uheader;
	struct mtd_partition *trx_parts;
	size_t retlen;
	unsigned int kernel_len;
	int ret;

	trx_parts = kcalloc(TRX_PARTS, sizeof(struct mtd_partition),
			    GFP_KERNEL);
	if (!trx_parts) {
		ret = -ENOMEM;
		goto out;
	}

	ret = mtd_read(master, 0, sizeof(header),
		       &retlen, (uint8_t *)&header);
	if (ret)
		goto free_parts;

	if (retlen != sizeof(header)) {
		ret = -EIO;
		goto free_parts;
	}

	theader = &header.trx;
	if (theader->magic != cpu_to_le32(TRX_MAGIC)) {
		printk(KERN_NOTICE "%s: no TRX header found\n", master->name);
		goto free_parts;
	}

	uheader = &header.uimage;
	if (uheader->ih_magic != cpu_to_be32(IH_MAGIC)) {
		printk(KERN_NOTICE "%s: no uImage found\n", master->name);
		goto free_parts;
	}

	kernel_len = le32_to_cpu(theader->offsets[1]) +
		sizeof(struct cybertan_header);

	trx_parts[0].name = "header";
	trx_parts[0].offset = 0;
	trx_parts[0].size = offsetof(struct firmware_header, uimage);
	trx_parts[0].mask_flags = 0;

	trx_parts[1].name = "kernel";
	trx_parts[1].offset = trx_parts[0].offset + trx_parts[0].size;
	trx_parts[1].size = kernel_len - trx_parts[0].size;
	trx_parts[1].mask_flags = 0;

	trx_parts[2].name = "rootfs";
	trx_parts[2].offset = trx_parts[1].offset + trx_parts[1].size;
	trx_parts[2].size = master->size - trx_parts[1].size - trx_parts[0].size;
	trx_parts[2].mask_flags = 0;

	*pparts = trx_parts;
	return TRX_PARTS;

free_parts:
	kfree(trx_parts);
out:
	return ret;
}

static const struct of_device_id mtd_parser_cybertan_of_match_table[] = {
	{ .compatible = "cybertan,trx" },
	{},
};
MODULE_DEVICE_TABLE(of, mtd_parser_cybertan_of_match_table);

static struct mtd_part_parser mtd_parser_cybertan = {
	.parse_fn = cybertan_parse_partitions,
	.name = "cybertan-trx",
	.of_match_table = mtd_parser_cybertan_of_match_table,
};
module_mtd_part_parser(mtd_parser_cybertan);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Daniel <cd@maintech.de>");
