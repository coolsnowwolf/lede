/*
 * 
 * Copyright (C) 2007 OpenWrt.org
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
 * Atmel AT91 flash partition table. (Modified by Hamish Guthrie).
 * Based on ar7 map by Felix Fietkau.
 *
 */

#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/bootmem.h>
#include <linux/squashfs_fs.h>

static struct mtd_partition at91_parts[6];

static int create_mtd_partitions(struct mtd_info *master, 
				 struct mtd_partition **pparts, 
				 unsigned long origin)
{
	unsigned int offset, len;
	unsigned int pre_size = 0x42000, root_max = 0x362400;
	unsigned char buf[512];
	struct squashfs_super_block *sb = (struct squashfs_super_block *) buf;

	printk("Parsing AT91 partition map...\n");

	at91_parts[0].name = "loaders";
	at91_parts[0].offset = 0;
	at91_parts[0].size = 0x21000;
	at91_parts[0].mask_flags = MTD_WRITEABLE;

	at91_parts[1].name = "ubparams";
	at91_parts[1].offset = 0x21000;
	at91_parts[1].size = 0x8400;
	at91_parts[1].mask_flags = 0;

	at91_parts[2].name = "kernel";
	at91_parts[2].offset = pre_size;
	at91_parts[2].size = 0;
	at91_parts[2].mask_flags = 0;

	at91_parts[3].name = "rootfs";
	at91_parts[3].offset = 0;
	at91_parts[3].size = 0;
	at91_parts[3].mask_flags = 0;

	for(offset = pre_size; offset < root_max; offset += master->erasesize) {

		memset(&buf, 0xe5, sizeof(buf));

		if (master->read(master, offset, sizeof(buf), &len, buf) || len != sizeof(buf))
			break;

		if (*((__u32 *) buf) == SQUASHFS_MAGIC) {
			printk(KERN_INFO "%s: Filesystem type: squashfs, size=0x%x\n",
							master->name, (u32) sb->bytes_used);

			at91_parts[3].size = sb->bytes_used;
			at91_parts[3].offset = offset;
			len = at91_parts[3].offset + at91_parts[3].size;
			len = ((len / (master->erasesize * 8)) + 1) * master->erasesize * 8;
			at91_parts[3].size = len - at91_parts[3].offset;
			at91_parts[2].size = offset - at91_parts[2].offset;
			break;
			}
		}

	if (at91_parts[3].size == 0) {
		printk(KERN_NOTICE "%s: Couldn't find root filesystem\n", master->name);
		return -1;
		}

	at91_parts[4].name = "rootfs_data";
	at91_parts[4].offset = root_max;
	at91_parts[4].size = master->size - root_max;
	at91_parts[4].mask_flags = 0;

	at91_parts[5].name = "complete";
	at91_parts[5].offset = 0;
	at91_parts[5].size = master->size;
	at91_parts[5].mask_flags = 0;

	*pparts = at91_parts;
	return 6;
}

static struct mtd_part_parser at91_parser = {
	.owner = THIS_MODULE,
	.parse_fn = create_mtd_partitions,
	.name = "at91part",
};

static int __init at91_parser_init(void)
{
	register_mtd_parser(&at91_parser);

	return 0;
}

module_init(at91_parser_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Felix Fietkau, Eugene Konev, Hamish Guthrie");
MODULE_DESCRIPTION("MTD partitioning for Atmel at91");
