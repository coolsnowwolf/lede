/*
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Felix Fietkau <nbd@nbd.name>
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

#define TPLINK_NR_PARTS		2
#define TPLINK_MIN_ROOTFS_OFFS	0x80000	/* 512KiB */

#define MD5SUM_LEN  16

struct fw_v1 {
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

struct fw_v2 {
	char		fw_version[48]; /* 0x04: fw version string */
	uint32_t	hw_id;		/* 0x34: hardware id */
	uint32_t	hw_rev;		/* 0x38: FIXME: hardware revision? */
	uint32_t	unk1;	        /* 0x3c: 0x00000000 */
	uint8_t		md5sum1[MD5SUM_LEN]; /* 0x40 */
	uint32_t	unk2;		/* 0x50: 0x00000000 */
	uint8_t		md5sum2[MD5SUM_LEN]; /* 0x54 */
	uint32_t	unk3;		/* 0x64: 0xffffffff */

	uint32_t	kernel_la;	/* 0x68: kernel load address */
	uint32_t	kernel_ep;	/* 0x6c: kernel entry point */
	uint32_t	fw_length;	/* 0x70: total length of the image */
	uint32_t	kernel_ofs;	/* 0x74: kernel data offset */
	uint32_t	kernel_len;	/* 0x78: kernel data length */
	uint32_t	rootfs_ofs;	/* 0x7c: rootfs data offset */
	uint32_t	rootfs_len;	/* 0x80: rootfs data length */
	uint32_t	boot_ofs;	/* 0x84: FIXME: seems to be unused */
	uint32_t	boot_len;	/* 0x88: FIXME: seems to be unused */
	uint16_t	unk4;		/* 0x8c: 0x55aa */
	uint8_t		sver_hi;	/* 0x8e */
	uint8_t		sver_lo;	/* 0x8f */
	uint8_t		unk5;		/* 0x90: magic: 0xa5 */
	uint8_t		ver_hi;         /* 0x91 */
	uint8_t		ver_mid;        /* 0x92 */
	uint8_t		ver_lo;         /* 0x93 */
	uint8_t		pad[364];
} __attribute__ ((packed));

struct tplink_fw_header {
	uint32_t version;
	union {
		struct fw_v1 v1;
		struct fw_v2 v2;
	};
};

static int mtdsplit_parse_tplink(struct mtd_info *master,
				 const struct mtd_partition **pparts,
				 struct mtd_part_parser_data *data)
{
	struct tplink_fw_header hdr;
	size_t hdr_len, retlen, kernel_size;
	size_t rootfs_offset;
	struct mtd_partition *parts;
	int err;

	hdr_len = sizeof(hdr);
	err = mtd_read(master, 0, hdr_len, &retlen, (void *) &hdr);
	if (err)
		return err;

	if (retlen != hdr_len)
		return -EIO;

	switch (le32_to_cpu(hdr.version)) {
	case 1:
		if (be32_to_cpu(hdr.v1.kernel_ofs) != sizeof(hdr))
			return -EINVAL;

		kernel_size = sizeof(hdr) + be32_to_cpu(hdr.v1.kernel_len);
		rootfs_offset = be32_to_cpu(hdr.v1.rootfs_ofs);
		break;
	case 2:
	case 3:
		if (be32_to_cpu(hdr.v2.kernel_ofs) != sizeof(hdr))
			return -EINVAL;

		kernel_size = sizeof(hdr) + be32_to_cpu(hdr.v2.kernel_len);
		rootfs_offset = be32_to_cpu(hdr.v2.rootfs_ofs);
		break;
	default:
		return -EINVAL;
	}

	if (kernel_size > master->size)
		return -EINVAL;

	/* Find the rootfs */
	err = mtd_check_rootfs_magic(master, rootfs_offset, NULL);
	if (err) {
		/*
		 * The size in the header might cover the rootfs as well.
		 * Start the search from an arbitrary offset.
		 */
		err = mtd_find_rootfs_from(master, TPLINK_MIN_ROOTFS_OFFS,
					   master->size, &rootfs_offset, NULL);
		if (err)
			return err;
	}

	parts = kzalloc(TPLINK_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = kernel_size;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return TPLINK_NR_PARTS;
}

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
static const struct of_device_id mtdsplit_tplink_of_match_table[] = {
	{ .compatible = "tplink,firmware" },
	{},
};
#endif

static struct mtd_part_parser mtdsplit_tplink_parser = {
	.owner = THIS_MODULE,
	.name = "tplink-fw",
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	.of_match_table = mtdsplit_tplink_of_match_table,
	#endif
	.parse_fn = mtdsplit_parse_tplink,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_tplink_init(void)
{
	register_mtd_parser(&mtdsplit_tplink_parser);

	return 0;
}

subsys_initcall(mtdsplit_tplink_init);
