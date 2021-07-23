/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *  MTD splitter for ELF loader firmware partitions
 *
 *  Copyright (C) 2020 Sander Vanheule <sander@svanheule.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; version 2.
 *
 *  To parse the ELF kernel loader, a small ELF parser is used that can
 *  handle both ELF32 or ELF64 class loaders. The splitter assumes that the
 *  kernel is always located before the rootfs, whether it is embedded in the
 *  loader or not.
 *
 *  The kernel image is preferably embedded inside the ELF loader, so the end
 *  of the loader equals the end of the kernel partition. This is due to the
 *  way mtd_find_rootfs_from searches for the the rootfs:
 *  - if the kernel image is embedded in the loader, the appended rootfs may
 *    follow the loader immediately, within the same erase block.
 *  - if the kernel image is not embedded in the loader, but placed at some
 *    offset behind the loader (OKLI-style loader), the rootfs must be
 *    aligned to an erase-block after the loader and kernel image.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/byteorder/generic.h>

#include "mtdsplit.h"

#define ELF_NR_PARTS	2

#define ELF_MAGIC	0x7f454c46 /* 0x7f E L F */
#define ELF_CLASS_32	1
#define ELF_CLASS_64	2

struct elf_header_ident {
	uint32_t magic;
	uint8_t class;
	uint8_t data;
	uint8_t version;
	uint8_t osabi;
	uint8_t abiversion;
	uint8_t pad[7];
};

struct elf_header_32 {
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t phoff;
	uint32_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

struct elf_header_64 {
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint64_t entry;
	uint64_t phoff;
	uint64_t shoff;
	uint32_t flags;
	uint16_t ehsize;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

struct elf_header {
	struct elf_header_ident ident;
	union {
		struct elf_header_32 elf32;
		struct elf_header_64 elf64;
	};
};

struct elf_program_header_32 {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t filesize;
	uint32_t memsize;
	uint32_t flags;
};

struct elf_program_header_64 {
	uint32_t type;
	uint32_t flags;
	uint64_t offset;
	uint64_t vaddr;
	uint64_t paddr;
	uint64_t filesize;
	uint64_t memsize;
};


static int mtdsplit_elf_read_mtd(struct mtd_info *mtd, size_t offset,
				 uint8_t *dst, size_t len)
{
	size_t retlen;
	int ret;

	ret = mtd_read(mtd, offset, len, &retlen, dst);
	if (ret) {
		pr_debug("read error in \"%s\"\n", mtd->name);
		return ret;
	}

	if (retlen != len) {
		pr_debug("short read in \"%s\"\n", mtd->name);
		return -EIO;
	}

	return 0;
}

static int elf32_determine_size(struct mtd_info *mtd, struct elf_header *hdr,
				size_t *size)
{
	struct elf_header_32 *hdr32 = &(hdr->elf32);
	int err;
	size_t section_end, ph_table_end, ph_entry;
	struct elf_program_header_32 ph;

	*size = 0;

	if (hdr32->shoff > 0) {
		*size = hdr32->shoff + hdr32->shentsize * hdr32->shnum;
		return 0;
	}

	ph_entry = hdr32->phoff;
	ph_table_end = hdr32->phoff + hdr32->phentsize * hdr32->phnum;

	while (ph_entry < ph_table_end) {
		err = mtdsplit_elf_read_mtd(mtd, ph_entry, (uint8_t *)(&ph),
					    sizeof(ph));
		if (err)
			return err;

		section_end = ph.offset + ph.filesize;
		if (section_end > *size)
			*size = section_end;

		ph_entry += hdr32->phentsize;
	}

	return 0;
}

static int elf64_determine_size(struct mtd_info *mtd, struct elf_header *hdr,
				size_t *size)
{
	struct elf_header_64 *hdr64 = &(hdr->elf64);
	int err;
	size_t section_end, ph_table_end, ph_entry;
	struct elf_program_header_64 ph;

	*size = 0;

	if (hdr64->shoff > 0) {
		*size = hdr64->shoff + hdr64->shentsize * hdr64->shnum;
		return 0;
	}

	ph_entry = hdr64->phoff;
	ph_table_end = hdr64->phoff + hdr64->phentsize * hdr64->phnum;

	while (ph_entry < ph_table_end) {
		err = mtdsplit_elf_read_mtd(mtd, ph_entry, (uint8_t *)(&ph),
					    sizeof(ph));
		if (err)
			return err;

		section_end = ph.offset + ph.filesize;
		if (section_end > *size)
			*size = section_end;

		ph_entry += hdr64->phentsize;
	}

	return 0;
}

static int mtdsplit_parse_elf(struct mtd_info *mtd,
			      const struct mtd_partition **pparts,
			      struct mtd_part_parser_data *data)
{
	struct elf_header hdr;
	size_t loader_size, rootfs_offset;
	enum mtdsplit_part_type type;
	struct mtd_partition *parts;
	int err;

	err = mtdsplit_elf_read_mtd(mtd, 0, (uint8_t *)&hdr, sizeof(hdr));
	if (err)
		return err;

	if (be32_to_cpu(hdr.ident.magic) != ELF_MAGIC) {
		pr_debug("invalid ELF magic %08x\n",
			 be32_to_cpu(hdr.ident.magic));
		return -EINVAL;
	}

	switch (hdr.ident.class) {
	case ELF_CLASS_32:
		err = elf32_determine_size(mtd, &hdr, &loader_size);
		break;
	case ELF_CLASS_64:
		err = elf64_determine_size(mtd, &hdr, &loader_size);
		break;
	default:
		pr_debug("invalid ELF class %i\n", hdr.ident.class);
		err = -EINVAL;
	}

	if (err)
		return err;

	err = mtd_find_rootfs_from(mtd, loader_size, mtd->size,
				   &rootfs_offset, &type);
	if (err)
		return err;

	if (rootfs_offset == mtd->size) {
		pr_debug("no rootfs found in \"%s\"\n", mtd->name);
		return -ENODEV;
	}

	parts = kzalloc(ELF_NR_PARTS * sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	if (type == MTDSPLIT_PART_TYPE_UBI)
		parts[1].name = UBI_PART_NAME;
	else
		parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = mtd->size - rootfs_offset;

	*pparts = parts;
	return ELF_NR_PARTS;
}

static const struct of_device_id mtdsplit_elf_of_match_table[] = {
	{ .compatible = "openwrt,elf" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_elf_of_match_table);

static struct mtd_part_parser mtdsplit_elf_parser = {
	.owner = THIS_MODULE,
	.name = "elf-loader-fw",
	.of_match_table = mtdsplit_elf_of_match_table,
	.parse_fn = mtdsplit_parse_elf,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_elf_init(void)
{
	register_mtd_parser(&mtdsplit_elf_parser);

	return 0;
}

subsys_initcall(mtdsplit_elf_init);
