/*
 * MTD split for Broadcom Whole Flash Image
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

#include <linux/crc32.h>
#include <linux/init.h>
#include <linux/jffs2.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/byteorder/generic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include "mtdsplit.h"

#define char_to_num(c)		((c >= '0' && c <= '9') ? (c - '0') : (0))

#define BCM_WFI_PARTS		3
#define BCM_WFI_SPLIT_PARTS	2

#define CFERAM_NAME		"cferam"
#define CFERAM_NAME_LEN		(sizeof(CFERAM_NAME) - 1)
#define CFERAM_NAME_MAX_LEN	32
#define KERNEL_NAME		"vmlinux.lz"
#define KERNEL_NAME_LEN		(sizeof(KERNEL_NAME) - 1)
#define OPENWRT_NAME		"1-openwrt"
#define OPENWRT_NAME_LEN	(sizeof(OPENWRT_NAME) - 1)

#define UBI_MAGIC		0x55424923

#define CFE_MAGIC_PFX		"cferam."
#define CFE_MAGIC_PFX_LEN	(sizeof(CFE_MAGIC_PFX) - 1)
#define CFE_MAGIC		"cferam.000"
#define CFE_MAGIC_LEN		(sizeof(CFE_MAGIC) - 1)
#define SERCOMM_MAGIC_PFX	"eRcOmM."
#define SERCOMM_MAGIC_PFX_LEN	(sizeof(SERCOMM_MAGIC_PFX) - 1)
#define SERCOMM_MAGIC		"eRcOmM.000"
#define SERCOMM_MAGIC_LEN	(sizeof(SERCOMM_MAGIC) - 1)

#define PART_CFERAM		"cferam"
#define PART_FIRMWARE		"firmware"
#define PART_IMAGE_1		"img1"
#define PART_IMAGE_2		"img2"

static u32 jffs2_dirent_crc(struct jffs2_raw_dirent *node)
{
	return crc32(0, node, sizeof(struct jffs2_raw_dirent) - 8);
}

static bool jffs2_dirent_valid(struct jffs2_raw_dirent *node)
{
	return ((je16_to_cpu(node->magic) == JFFS2_MAGIC_BITMASK) &&
		(je16_to_cpu(node->nodetype) == JFFS2_NODETYPE_DIRENT) &&
		je32_to_cpu(node->ino) &&
		je32_to_cpu(node->node_crc) == jffs2_dirent_crc(node));
}

static int jffs2_find_file(struct mtd_info *mtd, uint8_t *buf,
			   const char *name, size_t name_len,
			   loff_t *offs, loff_t size,
			   char **out_name, size_t *out_name_len)
{
	const loff_t end = *offs + size;
	struct jffs2_raw_dirent *node;
	bool valid = false;
	size_t retlen;
	uint16_t magic;
	int rc;

	for (; *offs < end; *offs += mtd->erasesize) {
		unsigned int block_offs = 0;

		/* Skip CFE erased blocks */
		rc = mtd_read(mtd, *offs, sizeof(magic), &retlen,
			      (void *) &magic);
		if (rc || retlen != sizeof(magic)) {
			continue;
		}

		/* Skip blocks not starting with JFFS2 magic */
		if (magic != JFFS2_MAGIC_BITMASK)
			continue;

		/* Read full block */
		rc = mtd_read(mtd, *offs, mtd->erasesize, &retlen,
			      (void *) buf);
		if (rc)
			return rc;
		if (retlen != mtd->erasesize)
			return -EINVAL;

		while (block_offs < mtd->erasesize) {
			node = (struct jffs2_raw_dirent *) &buf[block_offs];

			if (!jffs2_dirent_valid(node)) {
				block_offs += 4;
				continue;
			}

			if (!memcmp(node->name, OPENWRT_NAME,
				    OPENWRT_NAME_LEN)) {
				valid = true;
			} else if (!memcmp(node->name, name, name_len)) {
				if (!valid)
					return -EINVAL;

				if (out_name)
					*out_name = kstrndup(node->name,
							     node->nsize,
							     GFP_KERNEL);

				if (out_name_len)
					*out_name_len = node->nsize;

				return 0;
			}

			block_offs += je32_to_cpu(node->totlen);
			block_offs = (block_offs + 0x3) & ~0x3;
		}
	}

	return -ENOENT;
}

static int ubifs_find(struct mtd_info *mtd, loff_t *offs, loff_t size)
{
	const loff_t end = *offs + size;
	uint32_t magic;
	size_t retlen;
	int rc;

	for (; *offs < end; *offs += mtd->erasesize) {
		rc = mtd_read(mtd, *offs, sizeof(magic), &retlen,
			      (unsigned char *) &magic);
		if (rc || retlen != sizeof(magic))
			continue;

		if (be32_to_cpu(magic) == UBI_MAGIC)
			return 0;
	}

	return -ENOENT;
}

static int parse_bcm_wfi(struct mtd_info *master,
			 const struct mtd_partition **pparts,
			 uint8_t *buf, loff_t off, loff_t size, bool cfe_part)
{
	struct device_node *mtd_node;
	struct mtd_partition *parts;
	loff_t cfe_off, kernel_off, rootfs_off;
	unsigned int num_parts = BCM_WFI_PARTS, cur_part = 0;
	const char *cferam_name = CFERAM_NAME;
	size_t cferam_name_len;
	int ret;

	mtd_node = mtd_get_of_node(master);
	if (mtd_node)
		of_property_read_string(mtd_node, "brcm,cferam", &cferam_name);

	cferam_name_len = strnlen(cferam_name, CFERAM_NAME_MAX_LEN);
	if (cferam_name_len > 0)
		cferam_name_len--;

	if (cfe_part) {
		num_parts++;
		cfe_off = off;

		ret = jffs2_find_file(master, buf, cferam_name,
				      cferam_name_len, &cfe_off,
				      size - (cfe_off - off), NULL, NULL);
		if (ret)
			return ret;

		kernel_off = cfe_off + master->erasesize;
	} else {
		kernel_off = off;
	}

	ret = jffs2_find_file(master, buf, KERNEL_NAME, KERNEL_NAME_LEN,
			      &kernel_off, size - (kernel_off - off),
			      NULL, NULL);
	if (ret)
		return ret;

	rootfs_off = kernel_off + master->erasesize;
	ret = ubifs_find(master, &rootfs_off, size - (rootfs_off - off));
	if (ret)
		return ret;

	parts = kzalloc(num_parts * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	if (cfe_part) {
		parts[cur_part].name = PART_CFERAM;
		parts[cur_part].mask_flags = MTD_WRITEABLE;
		parts[cur_part].offset = cfe_off;
		parts[cur_part].size = kernel_off - cfe_off;
		cur_part++;
	}

	parts[cur_part].name = PART_FIRMWARE;
	parts[cur_part].offset = kernel_off;
	parts[cur_part].size = size - (kernel_off - off);
	cur_part++;

	parts[cur_part].name = KERNEL_PART_NAME;
	parts[cur_part].offset = kernel_off;
	parts[cur_part].size = rootfs_off - kernel_off;
	cur_part++;

	parts[cur_part].name = UBI_PART_NAME;
	parts[cur_part].offset = rootfs_off;
	parts[cur_part].size = size - (rootfs_off - off);
	cur_part++;

	*pparts = parts;

	return num_parts;
}

static int mtdsplit_parse_bcm_wfi(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	struct device_node *mtd_node;
	bool cfe_part = true;
	uint8_t *buf;
	int ret;

	mtd_node = mtd_get_of_node(master);
	if (!mtd_node)
		return -EINVAL;

	buf = kzalloc(master->erasesize, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (of_property_read_bool(mtd_node, "brcm,no-cferam"))
		cfe_part = false;

	ret = parse_bcm_wfi(master, pparts, buf, 0, master->size, cfe_part);

	kfree(buf);

	return ret;
}

static const struct of_device_id mtdsplit_bcm_wfi_of_match[] = {
	{ .compatible = "brcm,wfi" },
	{ },
};

static struct mtd_part_parser mtdsplit_bcm_wfi_parser = {
	.owner = THIS_MODULE,
	.name = "bcm-wfi-fw",
	.of_match_table = mtdsplit_bcm_wfi_of_match,
	.parse_fn = mtdsplit_parse_bcm_wfi,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int cferam_bootflag_value(const char *name, size_t name_len)
{
	int rc = -ENOENT;

	if (name &&
	    (name_len >= CFE_MAGIC_LEN) &&
	    !memcmp(name, CFE_MAGIC_PFX, CFE_MAGIC_PFX_LEN)) {
		rc = char_to_num(name[CFE_MAGIC_PFX_LEN + 0]) * 100;
		rc += char_to_num(name[CFE_MAGIC_PFX_LEN + 1]) * 10;
		rc += char_to_num(name[CFE_MAGIC_PFX_LEN + 2]) * 1;
	}

	return rc;
}

static int mtdsplit_parse_bcm_wfi_split(struct mtd_info *master,
					const struct mtd_partition **pparts,
					struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	loff_t cfe_off;
	loff_t img1_off = 0;
	loff_t img2_off = master->size / 2;
	loff_t img1_size = (img2_off - img1_off);
	loff_t img2_size = (master->size - img2_off);
	loff_t active_off, inactive_off;
	loff_t active_size, inactive_size;
	const char *inactive_name;
	uint8_t *buf;
	char *cfe1_name = NULL, *cfe2_name = NULL;
	size_t cfe1_size = 0, cfe2_size = 0;
	int ret;
	int bf1, bf2;

	buf = kzalloc(master->erasesize, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	cfe_off = img1_off;
	ret = jffs2_find_file(master, buf, CFERAM_NAME, CFERAM_NAME_LEN,
			      &cfe_off, img1_size, &cfe1_name, &cfe1_size);

	cfe_off = img2_off;
	ret = jffs2_find_file(master, buf, CFERAM_NAME, CFERAM_NAME_LEN,
			      &cfe_off, img2_size, &cfe2_name, &cfe2_size);

	bf1 = cferam_bootflag_value(cfe1_name, cfe1_size);
	if (bf1 >= 0)
		printk("cferam: bootflag1=%d\n", bf1);

	bf2 = cferam_bootflag_value(cfe2_name, cfe2_size);
	if (bf2 >= 0)
		printk("cferam: bootflag2=%d\n", bf2);

	kfree(cfe1_name);
	kfree(cfe2_name);

	if (bf1 >= bf2) {
		active_off = img1_off;
		active_size = img1_size;
		inactive_off = img2_off;
		inactive_size = img2_size;
		inactive_name = PART_IMAGE_2;
	} else {
		active_off = img2_off;
		active_size = img2_size;
		inactive_off = img1_off;
		inactive_size = img1_size;
		inactive_name = PART_IMAGE_1;
	}

	ret = parse_bcm_wfi(master, pparts, buf, active_off, active_size, true);

	kfree(buf);

	if (ret > 0) {
		parts = kzalloc((ret + 1) * sizeof(*parts), GFP_KERNEL);
		if (!parts)
			return -ENOMEM;

		memcpy(parts, *pparts, ret * sizeof(*parts));
		kfree(*pparts);

		parts[ret].name = inactive_name;
		parts[ret].offset = inactive_off;
		parts[ret].size = inactive_size;
		ret++;

		*pparts = parts;
	} else {
		parts = kzalloc(BCM_WFI_SPLIT_PARTS * sizeof(*parts), GFP_KERNEL);

		parts[0].name = PART_IMAGE_1;
		parts[0].offset = img1_off;
		parts[0].size = img1_size;

		parts[1].name = PART_IMAGE_2;
		parts[1].offset = img2_off;
		parts[1].size = img2_size;

		*pparts = parts;
	}

	return ret;
}

static const struct of_device_id mtdsplit_bcm_wfi_split_of_match[] = {
	{ .compatible = "brcm,wfi-split" },
	{ },
};

static struct mtd_part_parser mtdsplit_bcm_wfi_split_parser = {
	.owner = THIS_MODULE,
	.name = "bcm-wfi-split-fw",
	.of_match_table = mtdsplit_bcm_wfi_split_of_match,
	.parse_fn = mtdsplit_parse_bcm_wfi_split,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int sercomm_bootflag_value(struct mtd_info *mtd, uint8_t *buf)
{
	size_t retlen;
	loff_t offs;
	int rc;

	for (offs = 0; offs < mtd->size; offs += mtd->erasesize) {
		rc = mtd_read(mtd, offs, SERCOMM_MAGIC_LEN, &retlen, buf);
		if (rc || retlen != SERCOMM_MAGIC_LEN)
			continue;

		if (memcmp(buf, SERCOMM_MAGIC_PFX, SERCOMM_MAGIC_PFX_LEN))
			continue;

		rc = char_to_num(buf[SERCOMM_MAGIC_PFX_LEN + 0]) * 100;
		rc += char_to_num(buf[SERCOMM_MAGIC_PFX_LEN + 1]) * 10;
		rc += char_to_num(buf[SERCOMM_MAGIC_PFX_LEN + 2]) * 1;

		return rc;
	}

	return -ENOENT;
}

static int mtdsplit_parse_ser_wfi(struct mtd_info *master,
				  const struct mtd_partition **pparts,
				  struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct mtd_info *mtd_bf1, *mtd_bf2;
	loff_t img1_off = 0;
	loff_t img2_off = master->size / 2;
	loff_t img1_size = (img2_off - img1_off);
	loff_t img2_size = (master->size - img2_off);
	loff_t active_off, inactive_off;
	loff_t active_size, inactive_size;
	const char *inactive_name;
	uint8_t *buf;
	int bf1, bf2;
	int ret;

	mtd_bf1 = get_mtd_device_nm("bootflag1");
	if (IS_ERR(mtd_bf1))
		return -ENOENT;

	mtd_bf2 = get_mtd_device_nm("bootflag2");
	if (IS_ERR(mtd_bf2))
		return -ENOENT;

	buf = kzalloc(master->erasesize, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	bf1 = sercomm_bootflag_value(mtd_bf1, buf);
	if (bf1 >= 0)
		printk("sercomm: bootflag1=%d\n", bf1);

	bf2 = sercomm_bootflag_value(mtd_bf2, buf);
	if (bf2 >= 0)
		printk("sercomm: bootflag2=%d\n", bf2);

	if (bf1 == bf2 && bf2 >= 0) {
		struct erase_info bf_erase;

		bf2 = -ENOENT;
		bf_erase.addr = 0;
		bf_erase.len = mtd_bf2->size;
		mtd_erase(mtd_bf2, &bf_erase);
	}

	if (bf1 >= bf2) {
		active_off = img1_off;
		active_size = img1_size;
		inactive_off = img2_off;
		inactive_size = img2_size;
		inactive_name = PART_IMAGE_2;
	} else {
		active_off = img2_off;
		active_size = img2_size;
		inactive_off = img1_off;
		inactive_size = img1_size;
		inactive_name = PART_IMAGE_1;
	}

	ret = parse_bcm_wfi(master, pparts, buf, active_off, active_size, false);

	kfree(buf);

	if (ret > 0) {
		parts = kzalloc((ret + 1) * sizeof(*parts), GFP_KERNEL);
		if (!parts)
			return -ENOMEM;

		memcpy(parts, *pparts, ret * sizeof(*parts));
		kfree(*pparts);

		parts[ret].name = inactive_name;
		parts[ret].offset = inactive_off;
		parts[ret].size = inactive_size;
		ret++;

		*pparts = parts;
	} else {
		parts = kzalloc(BCM_WFI_SPLIT_PARTS * sizeof(*parts), GFP_KERNEL);

		parts[0].name = PART_IMAGE_1;
		parts[0].offset = img1_off;
		parts[0].size = img1_size;

		parts[1].name = PART_IMAGE_2;
		parts[1].offset = img2_off;
		parts[1].size = img2_size;

		*pparts = parts;
	}

	return ret;
}

static const struct of_device_id mtdsplit_ser_wfi_of_match[] = {
	{ .compatible = "sercomm,wfi" },
	{ },
};

static struct mtd_part_parser mtdsplit_ser_wfi_parser = {
	.owner = THIS_MODULE,
	.name = "ser-wfi-fw",
	.of_match_table = mtdsplit_ser_wfi_of_match,
	.parse_fn = mtdsplit_parse_ser_wfi,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_bcm_wfi_init(void)
{
	register_mtd_parser(&mtdsplit_bcm_wfi_parser);
	register_mtd_parser(&mtdsplit_bcm_wfi_split_parser);
	register_mtd_parser(&mtdsplit_ser_wfi_parser);

	return 0;
}

module_init(mtdsplit_bcm_wfi_init);
