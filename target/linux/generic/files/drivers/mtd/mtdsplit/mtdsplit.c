/*
 * Copyright (C) 2009-2013 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2009-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2012 Jonas Gorski <jogo@openwrt.org>
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#define pr_fmt(fmt)	"mtdsplit: " fmt

#include <linux/export.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/magic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define UBI_EC_MAGIC			0x55424923	/* UBI# */

struct squashfs_super_block {
	__le32 s_magic;
	__le32 pad0[9];
	__le64 bytes_used;
};

int mtd_get_squashfs_len(struct mtd_info *master,
			 size_t offset,
			 size_t *squashfs_len)
{
	struct squashfs_super_block sb;
	size_t retlen;
	int err;

	err = mtd_read(master, offset, sizeof(sb), &retlen, (void *)&sb);
	if (err || (retlen != sizeof(sb))) {
		pr_alert("error occured while reading from \"%s\"\n",
			 master->name);
		return -EIO;
	}

	if (le32_to_cpu(sb.s_magic) != SQUASHFS_MAGIC) {
		pr_alert("no squashfs found in \"%s\"\n", master->name);
		return -EINVAL;
	}

	retlen = le64_to_cpu(sb.bytes_used);
	if (retlen <= 0) {
		pr_alert("squashfs is empty in \"%s\"\n", master->name);
		return -ENODEV;
	}

	if (offset + retlen > master->size) {
		pr_alert("squashfs has invalid size in \"%s\"\n",
			 master->name);
		return -EINVAL;
	}

	*squashfs_len = retlen;
	return 0;
}
EXPORT_SYMBOL_GPL(mtd_get_squashfs_len);

static ssize_t mtd_next_eb(struct mtd_info *mtd, size_t offset)
{
	return mtd_rounddown_to_eb(offset, mtd) + mtd->erasesize;
}

int mtd_check_rootfs_magic(struct mtd_info *mtd, size_t offset,
			   enum mtdsplit_part_type *type)
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

	if (le32_to_cpu(magic) == SQUASHFS_MAGIC) {
		if (type)
			*type = MTDSPLIT_PART_TYPE_SQUASHFS;
		return 0;
	} else if (magic == 0x19852003) {
		if (type)
			*type = MTDSPLIT_PART_TYPE_JFFS2;
		return 0;
	} else if (be32_to_cpu(magic) == UBI_EC_MAGIC) {
		if (type)
			*type = MTDSPLIT_PART_TYPE_UBI;
		return 0;
	}

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(mtd_check_rootfs_magic);

int mtd_find_rootfs_from(struct mtd_info *mtd,
			 size_t from,
			 size_t limit,
			 size_t *ret_offset,
			 enum mtdsplit_part_type *type)
{
	size_t offset;
	int err;

	for (offset = from; offset < limit;
	     offset = mtd_next_eb(mtd, offset)) {
		err = mtd_check_rootfs_magic(mtd, offset, type);
		if (err)
			continue;

		*ret_offset = offset;
		return 0;
	}

	return -ENODEV;
}
EXPORT_SYMBOL_GPL(mtd_find_rootfs_from);

