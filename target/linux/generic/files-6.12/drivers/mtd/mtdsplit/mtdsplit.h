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

#ifndef _MTDSPLIT_H
#define _MTDSPLIT_H

#define KERNEL_PART_NAME	"kernel"
#define ROOTFS_PART_NAME	"rootfs"
#define UBI_PART_NAME		"ubi"

#define ROOTFS_SPLIT_NAME	"rootfs_data"

enum mtdsplit_part_type {
	MTDSPLIT_PART_TYPE_UNK = 0,
	MTDSPLIT_PART_TYPE_SQUASHFS,
	MTDSPLIT_PART_TYPE_JFFS2,
	MTDSPLIT_PART_TYPE_UBI,
};

#ifdef CONFIG_MTD_SPLIT
int mtd_get_squashfs_len(struct mtd_info *master,
			 size_t offset,
			 size_t *squashfs_len);

int mtd_check_rootfs_magic(struct mtd_info *mtd, size_t offset,
			   enum mtdsplit_part_type *type);

int mtd_find_rootfs_from(struct mtd_info *mtd,
			 size_t from,
			 size_t limit,
			 size_t *ret_offset,
			 enum mtdsplit_part_type *type);

#else
static inline int mtd_get_squashfs_len(struct mtd_info *master,
				       size_t offset,
				       size_t *squashfs_len)
{
	return -ENODEV;
}

static inline int mtd_check_rootfs_magic(struct mtd_info *mtd, size_t offset,
					 enum mtdsplit_part_type *type)
{
	return -EINVAL;
}

static inline int mtd_find_rootfs_from(struct mtd_info *mtd,
				       size_t from,
				       size_t limit,
				       size_t *ret_offset,
				       enum mtdsplit_part_type *type)
{
	return -ENODEV;
}
#endif /* CONFIG_MTD_SPLIT */

#endif /* _MTDSPLIT_H */
