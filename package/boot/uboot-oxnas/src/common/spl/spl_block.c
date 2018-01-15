/*
 * (C) Copyright 2013
 *
 * Ma Haijun <mahaijuns@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spl.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <version.h>
#include <part.h>
#include <fat.h>
#include <ext4fs.h>

/* should be implemented by board */
extern void spl_block_device_init(void);

block_dev_desc_t * spl_get_block_device(void)
{
	block_dev_desc_t * device;

	spl_block_device_init();

	device = get_dev(CONFIG_SPL_BLOCKDEV_INTERFACE, CONFIG_SPL_BLOCKDEV_ID);
	if (!device) {
		printf("blk device %s%d not exists\n",
			CONFIG_SPL_BLOCKDEV_INTERFACE,
			CONFIG_SPL_BLOCKDEV_ID);
		hang();
	}

	return device;
}

#ifdef CONFIG_SPL_FAT_SUPPORT
static int block_load_image_fat(const char *filename)
{
	int err;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	err = file_fat_read(filename, header, sizeof(struct image_header));
	if (err <= 0)
		goto end;

	spl_parse_image_header(header);

	err = file_fat_read(filename, (u8 *)spl_image.load_addr, 0);

end:
	if (err <= 0)
		printf("spl: error reading image %s, err - %d\n",
		       filename, err);

	return (err <= 0);
}

#ifdef CONFIG_SPL_OS_BOOT
static int block_load_image_fat_os(void)
{
	int err;

	err = file_fat_read(CONFIG_SPL_FAT_LOAD_ARGS_NAME,
			    (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0);
	if (err <= 0) {
		return -1;
	}

	return block_load_image_fat(CONFIG_SPL_FAT_LOAD_KERNEL_NAME);
}
#endif

void spl_block_load_image(void)
{
	int err;
	block_dev_desc_t * device;

	device = spl_get_block_device();
	err = fat_register_device(device, CONFIG_BLOCKDEV_FAT_BOOT_PARTITION);
	if (err) {
		printf("spl: fat register err - %d\n", err);
		hang();
	}
#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || block_load_image_fat_os())
#endif
	{
		err = block_load_image_fat(CONFIG_SPL_FAT_LOAD_PAYLOAD_NAME);
		if (err)
			hang();
	}
}
#elif defined(CONFIG_SPL_EXT4_SUPPORT) /* end CONFIG_SPL_FAT_SUPPORT */
static int block_load_image_ext4(const char *filename)
{
	int err;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	err = ext4_read_file(filename, header, 0, sizeof(struct image_header));
	if (err <= 0)
		goto end;

	spl_parse_image_header(header);

	err = ext4_read_file(filename, (u8 *)spl_image.load_addr, 0, 0);

end:
	return (err <= 0);
}

#ifdef CONFIG_SPL_OS_BOOT
static int block_load_image_ext4_os(void)
{
	int err;

	err = ext4_read_file(CONFIG_SPL_EXT4_LOAD_ARGS_NAME,
			    (void *)CONFIG_SYS_SPL_ARGS_ADDR, 0, 0);
	if (err <= 0) {
		return -1;
	}

	return block_load_image_ext4(CONFIG_SPL_EXT4_LOAD_KERNEL_NAME);
}
#endif

void spl_block_load_image(void)
{
	int err;
	block_dev_desc_t * device;

	device = spl_get_block_device();
	err = ext4_register_device(device, CONFIG_BLOCKDEV_EXT4_BOOT_PARTITION);
	if (err) {
		hang();
	}
#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || block_load_image_ext4_os())
#endif
	{
		err = block_load_image_ext4(CONFIG_SPL_EXT4_LOAD_PAYLOAD_NAME);
		if (err)
			hang();
	}
}
#else /* end CONFIG_SPL_EXT4_SUPPORT */
static int block_load_image_raw(block_dev_desc_t * device, lbaint_t sector)
{
	int n;
	u32 image_size_sectors;
	struct image_header *header;

	header = (struct image_header *)(CONFIG_SYS_TEXT_BASE -
						sizeof(struct image_header));

	/* read image header to find the image size & load address */
	n = device->block_read(device->dev, sector, 1, header);

	if (n != 1) {
		printf("spl: blk read err\n");
		return 1;
	}

	spl_parse_image_header(header);

	/* convert size to sectors - round up */
	image_size_sectors = (spl_image.size + 512 - 1) / 512;
	n = device->block_read(device->dev, sector, image_size_sectors,
					(void *)spl_image.load_addr);

	if (n != image_size_sectors) {
		printf("spl: blk read err\n");
		return 1;
	}
	return 0;
}

#ifdef CONFIG_SPL_OS_BOOT
static int block_load_image_raw_os(block_dev_desc_t * device)
{
	int n;

	n = device->block_read(device->dev, CONFIG_SYS_BLOCK_RAW_MODE_ARGS_SECTOR,
					CONFIG_SYS_BLOCK_RAW_MODE_ARGS_SECTORS,
					(u32 *)CONFIG_SYS_SPL_ARGS_ADDR);
	/* flush cache after read */
	flush_cache(addr, CONFIG_SYS_BLOCK_RAW_MODE_ARGS_SECTORS * 512);

	if (n != CONFIG_SYS_BLOCK_RAW_MODE_ARGS_SECTORS) {
		printf("args blk read error\n");
		return -1;
	}

	return block_load_image_raw(device, CONFIG_SYS_BLOCK_RAW_MODE_KERNEL_SECTOR);
}
#endif

void spl_block_load_image(void)
{
	int err;
	block_dev_desc_t * device;

	device = spl_get_block_device();
#ifdef CONFIG_SPL_OS_BOOT
	if (spl_start_uboot() || block_load_image_raw_os(device))
#endif
	{
		err = block_load_image_raw(device,
					 CONFIG_SYS_BLOCK_RAW_MODE_U_BOOT_SECTOR);
		if (err)
			hang();
	}
}
#endif /* CONFIG_SPL_FAT_SUPPORT */
