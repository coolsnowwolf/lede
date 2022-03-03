// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  fs/partitions/fit.c
 *  Copyright (C) 2021  Daniel Golle
 *
 *  headers extracted from U-Boot mkimage sources
 *  (C) Copyright 2008 Semihalf
 *  (C) Copyright 2000-2005
 *  Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 *  based on existing partition parsers
 *  Copyright (C) 1991-1998  Linus Torvalds
 *  Re-organised Feb 1998 Russell King
 */

#define pr_fmt(fmt) fmt

#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_fdt.h>
#include <linux/libfdt.h>

#include "check.h"

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature/key node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"
#define FIT_KEY_REQUIRED	"required"
#define FIT_KEY_HINT		"key-name-hint"

/* cipher node */
#define FIT_CIPHER_NODENAME	"cipher"
#define FIT_ALGO_PROP		"algo"

/* image node */
#define FIT_DATA_PROP		"data"
#define FIT_DATA_POSITION_PROP	"data-position"
#define FIT_DATA_OFFSET_PROP	"data-offset"
#define FIT_DATA_SIZE_PROP	"data-size"
#define FIT_TIMESTAMP_PROP	"timestamp"
#define FIT_DESC_PROP		"description"
#define FIT_ARCH_PROP		"arch"
#define FIT_TYPE_PROP		"type"
#define FIT_OS_PROP		"os"
#define FIT_COMP_PROP		"compression"
#define FIT_ENTRY_PROP		"entry"
#define FIT_LOAD_PROP		"load"

/* configuration node */
#define FIT_KERNEL_PROP		"kernel"
#define FIT_FILESYSTEM_PROP	"filesystem"
#define FIT_RAMDISK_PROP	"ramdisk"
#define FIT_FDT_PROP		"fdt"
#define FIT_LOADABLE_PROP	"loadables"
#define FIT_DEFAULT_PROP	"default"
#define FIT_SETUP_PROP		"setup"
#define FIT_FPGA_PROP		"fpga"
#define FIT_FIRMWARE_PROP	"firmware"
#define FIT_STANDALONE_PROP	"standalone"

#define FIT_MAX_HASH_LEN	HASH_MAX_DIGEST_SIZE

#define MIN_FREE_SECT		16
#define REMAIN_VOLNAME		"rootfs_data"

int parse_fit_partitions(struct parsed_partitions *state, u64 fit_start_sector, u64 sectors, int *slot, int add_remain)
{
	struct address_space *mapping = state->bdev->bd_inode->i_mapping;
	struct page *page;
	void *fit, *init_fit;
	struct partition_meta_info *info;
	char tmp[sizeof(info->volname)];
	u64 dsize, dsectors, imgmaxsect = 0;
	u32 size, image_pos, image_len;
	const u32 *image_offset_be, *image_len_be, *image_pos_be;
	int ret = 1, node, images, config;
	const char *image_name, *image_type, *image_description, *config_default,
		*config_description, *config_loadables;
	int image_name_len, image_type_len, image_description_len, config_default_len,
		config_description_len, config_loadables_len;
	sector_t start_sect, nr_sects;
	size_t label_min;

	if (fit_start_sector % (1<<(PAGE_SHIFT - SECTOR_SHIFT)))
		return -ERANGE;

	page = read_mapping_page(mapping, fit_start_sector >> (PAGE_SHIFT - SECTOR_SHIFT), NULL);
	if (IS_ERR(page))
		return -EFAULT;

	if (PageError(page))
		return -EFAULT;

	init_fit = page_address(page);

	if (!init_fit) {
		put_page(page);
		return -EFAULT;
	}

	if (fdt_check_header(init_fit)) {
		put_page(page);
		return 0;
	}

	dsectors = get_capacity(state->bdev->bd_disk);
	if (sectors)
		dsectors = (dsectors>sectors)?sectors:dsectors;

	dsize = dsectors << SECTOR_SHIFT;

	size = fdt_totalsize(init_fit);

	/* silently skip non-external-data legacy FIT images */
	if (size > PAGE_SIZE) {
		put_page(page);
		return 0;
	}

	if (size >= dsize) {
		state->access_beyond_eod = 1;
		put_page(page);
		return -EFBIG;
	}

	fit = kmemdup(init_fit, size, GFP_KERNEL);
	put_page(page);
	if (!fit)
		return -ENOMEM;

	config = fdt_path_offset(fit, FIT_CONFS_PATH);
	if (config < 0) {
		printk(KERN_ERR "FIT: Cannot find %s node: %d\n", FIT_CONFS_PATH, images);
		ret = -ENOENT;
		goto ret_out;
	}

	config_default = fdt_getprop(fit, config, FIT_DEFAULT_PROP, &config_default_len);

	if (!config_default) {
		printk(KERN_ERR "FIT: Cannot find default configuration\n");
		ret = -ENOENT;
		goto ret_out;
	}

	node = fdt_subnode_offset(fit, config, config_default);
	if (node < 0) {
		printk(KERN_ERR "FIT: Cannot find %s node: %d\n", config_default, node);
		ret = -ENOENT;
		goto ret_out;
	}

	config_description = fdt_getprop(fit, node, FIT_DESC_PROP, &config_description_len);
	config_loadables = fdt_getprop(fit, node, FIT_LOADABLE_PROP, &config_loadables_len);

	printk(KERN_DEBUG "FIT: Default configuration: \"%s\"%s%s%s\n", config_default,
		config_description?" (":"", config_description?:"", config_description?")":"");

	images = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images < 0) {
		printk(KERN_ERR "FIT: Cannot find %s node: %d\n", FIT_IMAGES_PATH, images);
		ret = -EINVAL;
		goto ret_out;
	}

	fdt_for_each_subnode(node, fit, images) {
		image_name = fdt_get_name(fit, node, &image_name_len);
		image_type = fdt_getprop(fit, node, FIT_TYPE_PROP, &image_type_len);
		image_offset_be = fdt_getprop(fit, node, FIT_DATA_OFFSET_PROP, NULL);
		image_pos_be = fdt_getprop(fit, node, FIT_DATA_POSITION_PROP, NULL);
		image_len_be = fdt_getprop(fit, node, FIT_DATA_SIZE_PROP, NULL);
		if (!image_name || !image_type || !image_len_be)
			continue;

		image_len = be32_to_cpu(*image_len_be);
		if (!image_len)
			continue;

		if (image_offset_be)
			image_pos = be32_to_cpu(*image_offset_be) + size;
		else if (image_pos_be)
			image_pos = be32_to_cpu(*image_pos_be);
		else
			continue;

		image_description = fdt_getprop(fit, node, FIT_DESC_PROP, &image_description_len);

		printk(KERN_DEBUG "FIT: %16s sub-image 0x%08x..0x%08x \"%s\" %s%s%s\n",
			image_type, image_pos, image_pos + image_len - 1, image_name,
			image_description?"(":"", image_description?:"", image_description?") ":"");

		if (strcmp(image_type, FIT_FILESYSTEM_PROP))
			continue;

		if (image_pos & ((1 << PAGE_SHIFT)-1)) {
			printk(KERN_ERR "FIT: image %s start not aligned to page boundaries, skipping\n", image_name);
			continue;
		}

		if (image_len & ((1 << PAGE_SHIFT)-1)) {
			printk(KERN_ERR "FIT: sub-image %s end not aligned to page boundaries, skipping\n", image_name);
			continue;
		}

		start_sect = image_pos >> SECTOR_SHIFT;
		nr_sects = image_len >> SECTOR_SHIFT;
		imgmaxsect = (imgmaxsect < (start_sect + nr_sects))?(start_sect + nr_sects):imgmaxsect;

		if (start_sect + nr_sects > dsectors) {
			state->access_beyond_eod = 1;
			continue;
		}

		put_partition(state, ++(*slot), fit_start_sector + start_sect, nr_sects);
		state->parts[*slot].flags = 0;
		info = &state->parts[*slot].info;

		label_min = min_t(int, sizeof(info->volname) - 1, image_name_len);
		strncpy(info->volname, image_name, label_min);
		info->volname[label_min] = '\0';

		snprintf(tmp, sizeof(tmp), "(%s)", info->volname);
		strlcat(state->pp_buf, tmp, PAGE_SIZE);

		state->parts[*slot].has_info = true;
		state->parts[*slot].flags |= ADDPART_FLAG_READONLY;
		if (config_loadables && !strcmp(image_name, config_loadables)) {
			printk(KERN_DEBUG "FIT: selecting configured loadable \"%s\" to be root filesystem\n", image_name);
			state->parts[*slot].flags |= ADDPART_FLAG_ROOTDEV;
		}
	}

	if (add_remain && (imgmaxsect + MIN_FREE_SECT) < dsectors) {
		put_partition(state, ++(*slot), fit_start_sector + imgmaxsect, dsectors - imgmaxsect);
		state->parts[*slot].flags = 0;
		info = &state->parts[*slot].info;
		strcpy(info->volname, REMAIN_VOLNAME);
		snprintf(tmp, sizeof(tmp), "(%s)", REMAIN_VOLNAME);
		strlcat(state->pp_buf, tmp, PAGE_SIZE);
	}
ret_out:
	kfree(fit);
	return ret;
}

int fit_partition(struct parsed_partitions *state) {
	int slot = 0;
	return parse_fit_partitions(state, 0, 0, &slot, 0);
}
