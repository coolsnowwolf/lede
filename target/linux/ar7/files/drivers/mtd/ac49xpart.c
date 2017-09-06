/*
 * AudioCodes AC49x PSPBoot-based flash partition table
 * Copyright 2012 Daniel Golle <daniel.golle@gmail.com>
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
 */

#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/bootmem.h>
#include <linux/magic.h>
#include <linux/module.h>

#include <asm/mach-ar7/prom.h>

#define AC49X_MAXENVPARTS	8

#define AC49X_PARTTYPE_LOADER	0
#define AC49X_PARTTYPE_BOOTENV	1
#define AC49X_PARTTYPE_LINUX	2
#define AC49X_PARTTYPE_ROOTFS	3
#define AC49X_PARTTYPE_UNKNOWN	4
#define AC49X_NUM_PARTTYPES	5

#define AC49X_FLASH_ADDRMASK	0x00FFFFFF

#define AC49X_LOADER_MAGIC	0x40809000
#define AC49X_LINUX_MAGIC	0x464c457f /*  ELF */
#define AC49X_BOOTENV_MAGIC	0x4578614d /* MaxE */

#define ROOTFS_MIN_OFFSET	0xC0000

int parse_partvar(const unsigned char *partvar, struct mtd_partition *part)
{
	unsigned int partstart, partend;
	unsigned int pnum;

	pnum = sscanf(partvar, "0x%x,0x%x", &partstart, &partend);
	if (pnum != 2)
		return 1;

	part->offset = partstart & AC49X_FLASH_ADDRMASK;
	part->size = partend - partstart;

	return 0;
}

int detect_parttype(struct mtd_info *master, struct mtd_partition part)
{
	unsigned int magic;
	size_t len;

	if (part.size < 4)
		return -1;

	mtd_read(master, part.offset, sizeof(magic), &len,
		 (uint8_t *)&magic);

	if (len != sizeof(magic))
		return -1;

	switch (magic) {
	case AC49X_LOADER_MAGIC:
		return AC49X_PARTTYPE_LOADER;
	case AC49X_LINUX_MAGIC:
		return AC49X_PARTTYPE_LINUX;
	case SQUASHFS_MAGIC:
	case CRAMFS_MAGIC:
	case CRAMFS_MAGIC_WEND:
		return AC49X_PARTTYPE_ROOTFS;
	case AC49X_BOOTENV_MAGIC:
		return AC49X_PARTTYPE_BOOTENV;
	default:
		switch (magic & 0xFF) {
		case JFFS2_SUPER_MAGIC:
			return AC49X_PARTTYPE_ROOTFS;
		}
		switch (magic >> 8) {
		case JFFS2_SUPER_MAGIC:
			return AC49X_PARTTYPE_ROOTFS;
		}
		return AC49X_PARTTYPE_UNKNOWN;
	}
}

const char *partnames[] = {
	"loader",
	"config",
	"linux",
	"rootfs",
	"data"
};

void gen_partname(unsigned int type,
		  unsigned int *typenumeration,
		  struct mtd_partition *part)
{
	char *s  = kzalloc(sizeof(char) * 8, GFP_KERNEL);

	(typenumeration[type])++;
	if (typenumeration[type] == 1)
		sprintf(s, "%s", partnames[type]);
	else
		sprintf(s, "%s%d", partnames[type], typenumeration[type]);

	part->name = s;
}

static int create_mtd_partitions(struct mtd_info *master,
				 struct mtd_partition **pparts,
				 struct mtd_part_parser_data *data)
{
	unsigned int envpartnum = 0, linuxpartnum = 0;
	unsigned int typenumeration[5] = { 0, 0, 0, 0, 0 };
	unsigned char evn[5];
	const unsigned char *partvar = NULL;

	struct mtd_partition *ac49x_parts;

	ac49x_parts = kzalloc(sizeof(*ac49x_parts) * AC49X_MAXENVPARTS,
				GFP_KERNEL);

	if (!ac49x_parts)
		return -ENOMEM;

	linuxpartnum = 0;
	for (envpartnum = 0; envpartnum < AC49X_MAXENVPARTS; envpartnum++) {
		struct mtd_partition parsepart;
		unsigned int offset, size, type;
		int err;
		sprintf(evn, "mtd%d", envpartnum);
		partvar = prom_getenv(evn);
		if (!partvar)
			continue;
		err = parse_partvar(partvar, &parsepart);
		if (err)
			continue;
		offset = parsepart.offset;
		size = parsepart.size;
		type = detect_parttype(master, parsepart);
		gen_partname(type, typenumeration, &parsepart);
		/* protect loader */
		if (type == AC49X_PARTTYPE_LOADER)
			parsepart.mask_flags = MTD_WRITEABLE;
		else
			parsepart.mask_flags = 0;

		memcpy(&(ac49x_parts[linuxpartnum]), &parsepart,
			sizeof(struct mtd_partition));

		/* scan for contained rootfs */
		if (type == AC49X_PARTTYPE_LINUX) {
			parsepart.offset += ROOTFS_MIN_OFFSET &
						~(master->erasesize - 1);
			parsepart.size -= ROOTFS_MIN_OFFSET &
						~(master->erasesize - 1);
			do {
				unsigned int size, offset;
				size = parsepart.size;
				offset = parsepart.offset;

				type = detect_parttype(master, parsepart);
				if (type == AC49X_PARTTYPE_ROOTFS) {
					gen_partname(type, typenumeration,
							&parsepart);
					printk(KERN_INFO
						"%s %s: 0x%08x@0x%08x\n",
						"detected sub-partition",
						parsepart.name,
						(unsigned int)parsepart.size,
						(unsigned int)parsepart.offset);
					linuxpartnum++;
					memcpy(&(ac49x_parts[linuxpartnum]),
						&parsepart,
						sizeof(struct mtd_partition));
					break;
				}
				parsepart.offset += master->erasesize;
				parsepart.size -= master->erasesize;
			} while (parsepart.size >= master->erasesize);
		}
		linuxpartnum++;
	}

	*pparts = ac49x_parts;
	return linuxpartnum;
}

static struct mtd_part_parser ac49x_parser = {
	.owner = THIS_MODULE,
	.parse_fn = create_mtd_partitions,
	.name = "ac49xpart",
};

static int __init ac49x_parser_init(void)
{
	register_mtd_parser(&ac49x_parser);
	return 0;
}

module_init(ac49x_parser_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Golle <daniel.golle@gmail.com>");
MODULE_DESCRIPTION("MTD partitioning for AudioCodes AC49x");
