/*
 *  Copyright (C) Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/kmod.h>
#include <linux/root_dev.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/byteorder/generic.h>

#define PFX	"trxsplit: "

#define TRX_MAGIC	0x30524448	/* "HDR0" */
#define TRX_VERSION	1
#define TRX_MAX_LEN	0x3A0000
#define TRX_NO_HEADER	0x1	/* do not write TRX header */
#define TRX_GZ_FILES	0x2     /* contains individual gzip files */
#define TRX_MAX_OFFSET	3
#define TRX_MIN_KERNEL_SIZE	(256 * 1024)

struct trx_header {
	u32 magic;	/* "HDR0" */
	u32 len;	/* Length of file including header */
	u32 crc32;	/* 32-bit CRC from flag_version to end of file */
	u32 flag_version; /* 0:15 flags, 16:31 version */
	u32 offsets[TRX_MAX_OFFSET]; /* Offsets of partitions */
};

#define TRX_ALIGN	0x1000

static int trx_nr_parts;
static unsigned long trx_offset;
static struct mtd_info *trx_mtd;
static struct mtd_partition trx_parts[TRX_MAX_OFFSET];
static struct trx_header trx_hdr;

static int trxsplit_refresh_partitions(struct mtd_info *mtd);

static int trxsplit_checktrx(struct mtd_info *mtd, unsigned long offset)
{
	size_t retlen;
	int err;

	err = mtd_read(mtd, offset, sizeof(trx_hdr), &retlen, (void *)&trx_hdr);
	if (err) {
		printk(KERN_ALERT PFX "unable to read from '%s'\n", mtd->name);
		goto err_out;
	}

	if (retlen != sizeof(trx_hdr)) {
		printk(KERN_ALERT PFX "reading failed on '%s'\n", mtd->name);
		goto err_out;
	}

	trx_hdr.magic = le32_to_cpu(trx_hdr.magic);
	trx_hdr.len = le32_to_cpu(trx_hdr.len);
	trx_hdr.crc32 = le32_to_cpu(trx_hdr.crc32);
	trx_hdr.flag_version = le32_to_cpu(trx_hdr.flag_version);
	trx_hdr.offsets[0] = le32_to_cpu(trx_hdr.offsets[0]);
	trx_hdr.offsets[1] = le32_to_cpu(trx_hdr.offsets[1]);
	trx_hdr.offsets[2] = le32_to_cpu(trx_hdr.offsets[2]);

	/* sanity checks */
	if (trx_hdr.magic != TRX_MAGIC)
		goto err_out;

	if (trx_hdr.len > mtd->size - offset)
		goto err_out;

	/* TODO: add crc32 checking too? */

	return 0;

err_out:
	return -1;
}

static void trxsplit_findtrx(struct mtd_info *mtd)
{
	unsigned long offset;
	int err;

	printk(KERN_INFO PFX "searching TRX header in '%s'\n", mtd->name);

	err = 0;
	for (offset = 0; offset < mtd->size; offset += TRX_ALIGN) {
		err = trxsplit_checktrx(mtd, offset);
		if (err == 0)
			break;
	}

	if (err)
		return;

	printk(KERN_INFO PFX "TRX header found at 0x%lX\n", offset);

	trx_mtd = mtd;
	trx_offset = offset;
}

static void trxsplit_create_partitions(struct mtd_info *mtd)
{
	struct mtd_partition *part = trx_parts;
	int err;
	int i;

	for (i = 0; i < TRX_MAX_OFFSET; i++) {
		part = &trx_parts[i];
		if (trx_hdr.offsets[i] == 0)
			continue;
		part->offset = trx_offset + trx_hdr.offsets[i];
		trx_nr_parts++;
	}

	for (i = 0; i < trx_nr_parts-1; i++)
		trx_parts[i].size = trx_parts[i+1].offset - trx_parts[i].offset;

	trx_parts[i].size = mtd->size - trx_parts[i].offset;

	i = 0;
	part = &trx_parts[i];
	if (part->size < TRX_MIN_KERNEL_SIZE) {
		part->name = "loader";
		i++;
	}

	part = &trx_parts[i];
	part->name = "kernel";
	i++;

	part = &trx_parts[i];
	part->name = "rootfs";

	err = mtd_device_register(mtd, trx_parts, trx_nr_parts);
	if (err) {
		printk(KERN_ALERT PFX "adding TRX partitions failed\n");
		return;
	}

	mtd->refresh_device = trxsplit_refresh_partitions;
}

static int trxsplit_refresh_partitions(struct mtd_info *mtd)
{
	printk(KERN_INFO PFX "refreshing TRX partitions in '%s' (%d,%d)\n",
		mtd->name, MTD_BLOCK_MAJOR, mtd->index);

	/* remove old partitions */
	mtd_device_unregister(mtd);

	trxsplit_findtrx(mtd);
	if (!trx_mtd)
		goto err;

	trxsplit_create_partitions(trx_mtd);
	return 1;

err:
	return 0;
}

static void __init trxsplit_add_mtd(struct mtd_info *mtd)
{
	if (mtd->type != MTD_NORFLASH) {
		printk(KERN_INFO PFX "'%s' is not a NOR flash, skipped\n",
				mtd->name);
		return;
	}

	if (!trx_mtd)
		trxsplit_findtrx(mtd);
}

static void __init trxsplit_remove_mtd(struct mtd_info *mtd)
{
	/* nothing to do */
}

static struct mtd_notifier trxsplit_notifier __initdata = {
	.add	= trxsplit_add_mtd,
	.remove	= trxsplit_remove_mtd,
};

static void __init trxsplit_scan(void)
{
	register_mtd_user(&trxsplit_notifier);
	unregister_mtd_user(&trxsplit_notifier);
}

static int __init trxsplit_init(void)
{
	trxsplit_scan();

	if (trx_mtd) {
		printk(KERN_INFO PFX "creating TRX partitions in '%s' "
			"(%d,%d)\n", trx_mtd->name, MTD_BLOCK_MAJOR,
			trx_mtd->index);
		trxsplit_create_partitions(trx_mtd);
	}

	return 0;
}

late_initcall(trxsplit_init);
