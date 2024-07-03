/*
 * Copyright (c) 2015 The Linux Foundation
 * Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/types.h>
#include <linux/byteorder/generic.h>
#include <linux/slab.h>
#include <linux/libfdt.h>
#include <linux/of_fdt.h>

#include "mtdsplit.h"

// string macros from git://git.denx.de/u-boot.git/include/image.h

#define FIT_IMAGES_PATH         "/images"
#define FIT_DATA_PROP           "data"
#define FIT_DATA_POSITION_PROP  "data-position"
#define FIT_DATA_OFFSET_PROP    "data-offset"
#define FIT_DATA_SIZE_PROP      "data-size"

// functions from git://git.denx.de/u-boot.git/common/image-fit.c

/**
 * fit_image_get_data - get data property and its size for a given component image node
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data() finds data property in a given component image node.
 * If the property is found its data start address and size are returned to
 * the caller.
 *
 * returns:
 *     0, on success
 *     -1, on failure
 */
static int fit_image_get_data(const void *fit, int noffset,
		const void **data, size_t *size)
{
	int len;

	*data = fdt_getprop(fit, noffset, FIT_DATA_PROP, &len);
	if (*data == NULL) {
		*size = 0;
		return -1;
	}

	*size = len;
	return 0;
}


/**
 * Get 'data-offset' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_offset: holds the data-offset property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
static int fit_image_get_data_offset(const void *fit, int noffset, int *data_offset)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_OFFSET_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_offset = fdt32_to_cpu(*val);

	return 0;
}

/**
 * Get 'data-position' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_position: holds the data-position property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
static int fit_image_get_data_position(const void *fit, int noffset,
				int *data_position)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_POSITION_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_position = fdt32_to_cpu(*val);

	return 0;
}

/**
 * Get 'data-size' property from a given image node.
 *
 * @fit: pointer to the FIT image header
 * @noffset: component image node offset
 * @data_size: holds the data-size property
 *
 * returns:
 *     0, on success
 *     -ENOENT if the property could not be found
 */
static int fit_image_get_data_size(const void *fit, int noffset, int *data_size)
{
	const fdt32_t *val;

	val = fdt_getprop(fit, noffset, FIT_DATA_SIZE_PROP, NULL);
	if (!val)
		return -ENOENT;

	*data_size = fdt32_to_cpu(*val);

	return 0;
}

/**
 * fit_image_get_data_and_size - get data and its size including
 *				 both embedded and external data
 * @fit: pointer to the FIT format image header
 * @noffset: component image node offset
 * @data: double pointer to void, will hold data property's data address
 * @size: pointer to size_t, will hold data property's data size
 *
 * fit_image_get_data_and_size() finds data and its size including
 * both embedded and external data. If the property is found
 * its data start address and size are returned to the caller.
 *
 * returns:
 *     0, on success
 *     otherwise, on failure
 */
static int fit_image_get_data_and_size(const void *fit, int noffset,
				const void **data, size_t *size)
{
	bool external_data = false;
	int offset;
	int len;
	int ret;

	if (!fit_image_get_data_position(fit, noffset, &offset)) {
		external_data = true;
	} else if (!fit_image_get_data_offset(fit, noffset, &offset)) {
		external_data = true;
		/*
		 * For FIT with external data, figure out where
		 * the external images start. This is the base
		 * for the data-offset properties in each image.
		 */
		offset += ((fdt_totalsize(fit) + 3) & ~3);
	}

	if (external_data) {
		ret = fit_image_get_data_size(fit, noffset, &len);
		if (!ret) {
			*data = fit + offset;
			*size = len;
		}
	} else {
		ret = fit_image_get_data(fit, noffset, data, size);
	}

	return ret;
}

static int
mtdsplit_fit_parse(struct mtd_info *mtd,
		   const struct mtd_partition **pparts,
	           struct mtd_part_parser_data *data)
{
	struct device_node *np = mtd_get_of_node(mtd);
	const char *cmdline_match = NULL;
	struct fdt_header hdr;
	size_t hdr_len, retlen;
	size_t offset;
	u32 offset_start = 0;
	size_t fit_offset, fit_size;
	size_t rootfs_offset, rootfs_size;
	size_t data_size, img_total, max_size = 0;
	struct mtd_partition *parts;
	int ret, ndepth, noffset, images_noffset;
	const void *img_data;
	void *fit;

	of_property_read_string(np, "openwrt,cmdline-match", &cmdline_match);
	if (cmdline_match && !strstr(saved_command_line, cmdline_match))
		return -ENODEV;

	of_property_read_u32(np, "openwrt,fit-offset", &offset_start);

	hdr_len = sizeof(struct fdt_header);

	/* Parse the MTD device & search for the FIT image location */
	for(offset = 0; offset + hdr_len <= mtd->size; offset += mtd->erasesize) {
		ret = mtd_read(mtd, offset + offset_start, hdr_len, &retlen, (void*) &hdr);
		if (ret) {
			pr_err("read error in \"%s\" at offset 0x%llx\n",
			       mtd->name, (unsigned long long) offset);
			return ret;
		}

		if (retlen != hdr_len) {
			pr_err("short read in \"%s\"\n", mtd->name);
			return -EIO;
		}

		/* Check the magic - see if this is a FIT image */
		if (be32_to_cpu(hdr.magic) != OF_DT_HEADER) {
			pr_debug("no valid FIT image found in \"%s\" at offset %llx\n",
				 mtd->name, (unsigned long long) offset);
			continue;
		}

		/* We found a FIT image. Let's keep going */
		break;
	}

	fit_offset = offset;
	fit_size = be32_to_cpu(hdr.totalsize);

	if (fit_size == 0) {
		pr_err("FIT image in \"%s\" at offset %llx has null size\n",
		       mtd->name, (unsigned long long) fit_offset);
		return -ENODEV;
	}

	/*
	 * Classic uImage.FIT has all data embedded into the FDT
	 * data structure. Hence the total size of the image equals
	 * the total size of the FDT structure.
	 * Modern uImage.FIT may have only references to data in FDT,
	 * hence we need to parse FDT structure to find the end of the
	 * last external data refernced.
	 */
	if (fit_size > 0x1000) {
		enum mtdsplit_part_type type;

		/* Search for the rootfs partition after the FIT image */
		ret = mtd_find_rootfs_from(mtd, fit_offset + fit_size + offset_start, mtd->size,
					   &rootfs_offset, &type);
		if (ret) {
			pr_info("no rootfs found after FIT image in \"%s\"\n",
				mtd->name);
			return ret;
		}

		rootfs_size = mtd->size - rootfs_offset;

		parts = kzalloc(2 * sizeof(*parts), GFP_KERNEL);
		if (!parts)
			return -ENOMEM;

		parts[0].name = KERNEL_PART_NAME;
		parts[0].offset = fit_offset;
		parts[0].size = mtd_rounddown_to_eb(fit_size + offset_start, mtd) + mtd->erasesize;

		if (type == MTDSPLIT_PART_TYPE_UBI)
			parts[1].name = UBI_PART_NAME;
		else
			parts[1].name = ROOTFS_PART_NAME;
		parts[1].offset = rootfs_offset;
		parts[1].size = rootfs_size;

		*pparts = parts;

		return 2;
	} else {
		/* Search for rootfs_data after FIT external data */
		fit = kzalloc(fit_size, GFP_KERNEL);
		ret = mtd_read(mtd, offset, fit_size + offset_start, &retlen, fit);
		if (ret) {
			pr_err("read error in \"%s\" at offset 0x%llx\n",
			       mtd->name, (unsigned long long) offset);
			return ret;
		}

		images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
		if (images_noffset < 0) {
			pr_err("Can't find images parent node '%s' (%s)\n",
			FIT_IMAGES_PATH, fdt_strerror(images_noffset));
			return -ENODEV;
		}

		for (ndepth = 0,
		     noffset = fdt_next_node(fit, images_noffset, &ndepth);
		     (noffset >= 0) && (ndepth > 0);
		     noffset = fdt_next_node(fit, noffset, &ndepth)) {
			if (ndepth == 1) {
				ret = fit_image_get_data_and_size(fit, noffset, &img_data, &data_size);
				if (ret)
					return 0;

				img_total = data_size + (img_data - fit);

				max_size = (max_size > img_total) ? max_size : img_total;
			}
		}

		parts = kzalloc(sizeof(*parts), GFP_KERNEL);
		if (!parts)
			return -ENOMEM;

		parts[0].name = ROOTFS_SPLIT_NAME;
		parts[0].offset = fit_offset + mtd_rounddown_to_eb(max_size, mtd) + mtd->erasesize;
		parts[0].size = mtd->size - parts[0].offset;

		*pparts = parts;

		kfree(fit);

		return 1;
	}
}

static const struct of_device_id mtdsplit_fit_of_match_table[] = {
	{ .compatible = "denx,fit" },
	{},
};

static struct mtd_part_parser uimage_parser = {
	.owner = THIS_MODULE,
	.name = "fit-fw",
	.of_match_table = mtdsplit_fit_of_match_table,
	.parse_fn = mtdsplit_fit_parse,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

/**************************************************
 * Init
 **************************************************/

static int __init mtdsplit_fit_init(void)
{
	register_mtd_parser(&uimage_parser);

	return 0;
}

module_init(mtdsplit_fit_init);
