/*
 *  Copyright (C) 2010 Scott Nicholas <neutronscott@scottn.us>
 *  Copyright (C) 2006 Felix Fietkau <nbd@nbd.name>
 *  Copyright (C) 2005 Waldemar Brodkorb <wbx@openwrt.org>
 *  Copyright (C) 2004 Florian Schirmer (jolt@tuxbox.org)
 *
 *  original functions for finding root filesystem from Mike Baker
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  THIS  SOFTWARE  IS PROVIDED   ``AS  IS'' AND   ANY  EXPRESS OR IMPLIED
 *  WARRANTIES,   INCLUDING, BUT NOT  LIMITED  TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN
 *  NO  EVENT  SHALL   THE AUTHOR  BE    LIABLE FOR ANY   DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED   TO, PROCUREMENT OF  SUBSTITUTE GOODS  OR SERVICES; LOSS OF
 *  USE, DATA,  OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, WHETHER IN  CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Copyright 2004, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * Flash mapping for adm8668 boards
 *
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/slab.h>
#include <linux/mtd/partitions.h>
#include <linux/crc32.h>
#include <linux/magic.h>
#include <asm/io.h>

#define WINDOW_ADDR	0x10000000
#define WINDOW_SIZE	0x800000
#define BANKWIDTH	2

/* first a little bit about the headers i need.. */

/* just interested in part of the full struct */
struct squashfs_super_block {
        __le32  s_magic;
        __le32  pad0[9];        /* it's not really padding */
        __le64  bytes_used;
};

#define IH_MAGIC	0x56190527	/* Image Magic Number		*/
struct uboot_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	char		ih_name[32];	/* image name */
};

/************************************************/

static struct mtd_info *adm8668_mtd;

struct map_info adm8668_map = {
	name: "adm8668-nor",
	size: WINDOW_SIZE,
	phys: WINDOW_ADDR,
	bankwidth: BANKWIDTH,
};

/*
 * Copied from mtdblock.c
 *
 * Cache stuff...
 *
 * Since typical flash erasable sectors are much larger than what Linux's
 * buffer cache can handle, we must implement read-modify-write on flash
 * sectors for each block write requests.  To avoid over-erasing flash sectors
 * and to speed things up, we locally cache a whole flash sector while it is
 * being written to until a different sector is required.
 */

static void erase_callback(struct erase_info *done)
{
	wait_queue_head_t *wait_q = (wait_queue_head_t *)done->priv;
	wake_up(wait_q);
}

static int erase_write (struct mtd_info *mtd, unsigned long pos,
			int len, const char *buf)
{
	struct erase_info erase;
	DECLARE_WAITQUEUE(wait, current);
	wait_queue_head_t wait_q;
	size_t retlen;
	int ret;

	/*
	 * First, let's erase the flash block.
	 */

	init_waitqueue_head(&wait_q);
	erase.mtd = mtd;
	erase.callback = erase_callback;
	erase.addr = pos;
	erase.len = len;
	erase.priv = (u_long)&wait_q;

	set_current_state(TASK_INTERRUPTIBLE);
	add_wait_queue(&wait_q, &wait);

	ret = mtd->_erase(mtd, &erase);
	if (ret) {
		set_current_state(TASK_RUNNING);
		remove_wait_queue(&wait_q, &wait);
		printk (KERN_WARNING "erase of region [0x%lx, 0x%x] "
				     "on \"%s\" failed\n",
			pos, len, mtd->name);
		return ret;
	}

	schedule();  /* Wait for erase to finish. */
	remove_wait_queue(&wait_q, &wait);

	/*
	 * Next, write data to flash.
	 */

	ret = mtd->_write (mtd, pos, len, &retlen, buf);
	if (ret)
		return ret;
	if (retlen != len)
		return -EIO;
	return 0;
}

/* decent defaults in case... shrug */
static struct mtd_partition adm8668_parts[] = {
	{ name: "linux",	offset: 0x40000,	size: WINDOW_SIZE-0x40000, },
	{ name: "rootfs",	offset: 0xe0000,	size: 0x140000, },
	{ name: "uboot_env",	offset: 0x20000,	size: 0x20000, },
	{ name: NULL, },
};

/* in case i wanna change stuff later, and to clarify the math section... */
#define	PART_LINUX	0
#define	PART_ROOTFS	1
#define	NR_PARTS	3

static int __init
init_mtd_partitions(struct mtd_info *mtd, size_t size)
{
	struct uboot_header uhdr;
	int off, blocksize;
	size_t len, linux_len;
	struct squashfs_super_block shdr;

	blocksize = mtd->erasesize;
	if (blocksize < 0x10000)
		blocksize = 0x10000;

	/* now find squashfs */
	memset(&shdr, 0xe5, sizeof(shdr));
	for (off = adm8668_parts[PART_LINUX].offset; off < size; off += blocksize) {
		/*
		 * Read into buffer
		 */
		if (mtd->_read(mtd, off, sizeof(shdr), &len, (char *)&shdr) ||
		    len != sizeof(shdr))
			continue;

		if (shdr.s_magic == SQUASHFS_MAGIC) {
			uint32_t fs_size = (uint32_t)shdr.bytes_used;

			printk(KERN_INFO "%s: Filesystem type: squashfs, size=%dkB\n",
				mtd->name, fs_size>>10);

			/* Update rootfs based on the superblock info, and
			 * stretch to end of MTD. rootfs_split will split it */
			adm8668_parts[PART_ROOTFS].offset = off;
			adm8668_parts[PART_ROOTFS].size = mtd->size -
				adm8668_parts[PART_ROOTFS].offset;

			/* kernel ends where rootfs starts
			 * but we'll keep it full-length for upgrades */
			linux_len = adm8668_parts[PART_LINUX+1].offset -
				adm8668_parts[PART_LINUX].offset;
#if 1
			adm8668_parts[PART_LINUX].size = mtd->size -
				adm8668_parts[PART_LINUX].offset;
#else
			adm8668_parts[PART_LINUX].size = linux_len;
#endif
			goto found;
		}
	}

	printk(KERN_NOTICE
	       "%s: Couldn't find root filesystem\n",
	       mtd->name);
	return NR_PARTS;

 found:
	if (mtd->_read(mtd, adm8668_parts[PART_LINUX].offset, sizeof(uhdr), &len, (char *)&uhdr) ||
	    len != sizeof(uhdr))
		return NR_PARTS;

	/* that's odd. how'd ya boot it then */
	if (uhdr.ih_magic != IH_MAGIC)
		return NR_PARTS;

	if (be32_to_cpu(uhdr.ih_size) != (linux_len - sizeof(uhdr))) {
		unsigned char *block, *data;
		unsigned int offset;

		offset = adm8668_parts[PART_LINUX].offset +
			 sizeof(struct uboot_header);
		data = (unsigned char *)(WINDOW_ADDR | 0xA0000000 | offset);

		printk(KERN_NOTICE "Updating U-boot image:\n");
		printk(KERN_NOTICE "  old: [size: %8d crc32: 0x%08x]\n",
			be32_to_cpu(uhdr.ih_size), be32_to_cpu(uhdr.ih_dcrc));

		/* Update the data length & crc32 */
		uhdr.ih_size = cpu_to_be32(linux_len - sizeof(uhdr));
		uhdr.ih_dcrc = crc32_le(~0, data, linux_len - sizeof(uhdr)) ^ (~0);
		uhdr.ih_dcrc = cpu_to_be32(uhdr.ih_dcrc);

		printk(KERN_NOTICE "  new: [size: %8d crc32: 0x%08x]\n",
			be32_to_cpu(uhdr.ih_size), be32_to_cpu(uhdr.ih_dcrc));

		/* update header's crc... */
		uhdr.ih_hcrc = 0;
		uhdr.ih_hcrc = crc32_le(~0, (unsigned char *)&uhdr,
				sizeof(uhdr)) ^ (~0);
		uhdr.ih_hcrc = cpu_to_be32(uhdr.ih_hcrc);

		/* read first eraseblock from the image */
		block = kmalloc(mtd->erasesize, GFP_KERNEL);
		if (mtd->_read(mtd, adm8668_parts[PART_LINUX].offset, mtd->erasesize, &len, block) || len != mtd->erasesize) {
			printk("Error copying first eraseblock\n");
			return 0;
		}

		/* Write updated header to the flash */
		memcpy(block, &uhdr, sizeof(uhdr));
		if (mtd->_unlock)
			mtd->_unlock(mtd, off, mtd->erasesize);
		erase_write(mtd, adm8668_parts[PART_LINUX].offset, mtd->erasesize, block);
		if (mtd->_sync)
			mtd->_sync(mtd);
		kfree(block);
		printk(KERN_NOTICE "Done\n");
	}

	return NR_PARTS;
}

int __init init_adm8668_map(void)
{
	int nr_parts, ret;

	adm8668_map.virt = ioremap(WINDOW_ADDR, WINDOW_SIZE);

	if (!adm8668_map.virt) {
		printk(KERN_ERR "Failed to ioremap\n");
		return -EIO;
	}

	simple_map_init(&adm8668_map);
	if (!(adm8668_mtd = do_map_probe("cfi_probe", &adm8668_map))) {
		printk(KERN_ERR "cfi_probe failed\n");
		iounmap((void *)adm8668_map.virt);
		return -ENXIO;
	}

	adm8668_mtd->owner = THIS_MODULE;

	nr_parts = init_mtd_partitions(adm8668_mtd, adm8668_mtd->size);
	ret = mtd_device_register(adm8668_mtd, adm8668_parts, nr_parts);
	if (ret) {
		printk(KERN_ERR "Flash: mtd_device_register failed\n");
		goto fail;
	}

	return 0;

 fail:
	if (adm8668_mtd)
		map_destroy(adm8668_mtd);
	if (adm8668_map.virt)
		iounmap((void *) adm8668_map.virt);
	adm8668_map.virt = 0;
	return ret;
}

void __exit cleanup_adm8668_map(void)
{
	mtd_device_unregister(adm8668_mtd);
	map_destroy(adm8668_mtd);
	iounmap((void *) adm8668_map.virt);
	adm8668_map.virt = 0;
}

module_init(init_adm8668_map);
module_exit(cleanup_adm8668_map);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Scott Nicholas <neutronscott@scottn.us>");
MODULE_DESCRIPTION("MTD map driver for ADM8668 NOR Flash");
