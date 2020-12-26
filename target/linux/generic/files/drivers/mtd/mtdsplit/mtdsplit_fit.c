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
#include <linux/of_fdt.h>

#include "mtdsplit.h"

struct fdt_header {
	uint32_t magic;			 /* magic word FDT_MAGIC */
	uint32_t totalsize;		 /* total size of DT block */
	uint32_t off_dt_struct;		 /* offset to structure */
	uint32_t off_dt_strings;	 /* offset to strings */
	uint32_t off_mem_rsvmap;	 /* offset to memory reserve map */
	uint32_t version;		 /* format version */
	uint32_t last_comp_version;	 /* last compatible version */

	/* version 2 fields below */
	uint32_t boot_cpuid_phys;	 /* Which physical CPU id we're
					    booting on */
	/* version 3 fields below */
	uint32_t size_dt_strings;	 /* size of the strings block */

	/* version 17 fields below */
	uint32_t size_dt_struct;	 /* size of the structure block */
};

static int
mtdsplit_fit_parse(struct mtd_info *mtd,
		   const struct mtd_partition **pparts,
	           struct mtd_part_parser_data *data)
{
	struct fdt_header hdr;
	size_t hdr_len, retlen;
	size_t offset;
	size_t fit_offset, fit_size;
	size_t rootfs_offset, rootfs_size;
	struct mtd_partition *parts;
	int ret;

	hdr_len = sizeof(struct fdt_header);

	/* Parse the MTD device & search for the FIT image location */
	for(offset = 0; offset + hdr_len <= mtd->size; offset += mtd->erasesize) {
		ret = mtd_read(mtd, offset, hdr_len, &retlen, (void*) &hdr);
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

	/* Search for the rootfs partition after the FIT image */
	ret = mtd_find_rootfs_from(mtd, fit_offset + fit_size, mtd->size,
				   &rootfs_offset, NULL);
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
	parts[0].size = mtd_rounddown_to_eb(fit_size, mtd) + mtd->erasesize;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = rootfs_size;

	*pparts = parts;
	return 2;
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
