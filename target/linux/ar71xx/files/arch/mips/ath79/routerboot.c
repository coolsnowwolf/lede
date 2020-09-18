/*
 *  RouterBoot helper routines
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "rb: " fmt

#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/routerboot.h>
#include <linux/rle.h>
#include <linux/lzo.h>

#include "routerboot.h"

#define RB_BLOCK_SIZE		0x1000
#define RB_ART_SIZE		0x10000
#define RB_MAGIC_ERD		0x00455244	/* extended radio data */

static struct rb_info rb_info;

static u32 get_u32(void *buf)
{
	u8 *p = buf;

	return ((u32) p[3] + ((u32) p[2] << 8) + ((u32) p[1] << 16) +
	       ((u32) p[0] << 24));
}

static u16 get_u16(void *buf)
{
	u8 *p = buf;

	return (u16) p[1] + ((u16) p[0] << 8);
}

__init int
routerboot_find_magic(u8 *buf, unsigned int buflen, u32 *offset, bool hard)
{
	u32 magic_ref = hard ? RB_MAGIC_HARD : RB_MAGIC_SOFT;
	u32 magic;
	u32 cur = *offset;

	while (cur < buflen) {
		magic = get_u32(buf + cur);
		if (magic == magic_ref) {
			*offset = cur;
			return 0;
		}

		cur += 0x1000;
	}

	return -ENOENT;
}

__init int
routerboot_find_tag(u8 *buf, unsigned int buflen, u16 tag_id,
		    u8 **tag_data, u16 *tag_len)
{
	uint32_t magic;
	bool align = false;
	int ret;

	if (buflen < 4)
		return -EINVAL;

	magic = get_u32(buf);
	switch (magic) {
	case RB_MAGIC_ERD:
		align = true;
		/* fall trough */
	case RB_MAGIC_HARD:
		/* skip magic value */
		buf += 4;
		buflen -= 4;
		break;

	case RB_MAGIC_SOFT:
		if (buflen < 8)
			return -EINVAL;

		/* skip magic and CRC value */
		buf += 8;
		buflen -= 8;

		break;

	default:
		return -EINVAL;
	}

	ret = -ENOENT;
	while (buflen > 2) {
		u16 id;
		u16 len;

		len = get_u16(buf);
		buf += 2;
		buflen -= 2;

		if (buflen < 2)
			break;

		id = get_u16(buf);
		buf += 2;
		buflen -= 2;

		if (id == RB_ID_TERMINATOR)
			break;

		if (buflen < len)
			break;

		if (id == tag_id) {
			if (tag_len)
				*tag_len = len;
			if (tag_data)
				*tag_data = buf;
			ret = 0;
			break;
		}

		if (align)
			len = (len + 3) / 4;

		buf += len;
		buflen -= len;
	}

	return ret;
}

static inline int
rb_find_hard_cfg_tag(u16 tag_id, u8 **tag_data, u16 *tag_len)
{
	if (!rb_info.hard_cfg_data ||
	    !rb_info.hard_cfg_size)
		return -ENOENT;

	return routerboot_find_tag(rb_info.hard_cfg_data,
				   rb_info.hard_cfg_size,
				   tag_id, tag_data, tag_len);
}

__init const char *
rb_get_board_name(void)
{
	u16 tag_len;
	u8 *tag;
	int err;

	err = rb_find_hard_cfg_tag(RB_ID_BOARD_NAME, &tag, &tag_len);
	if (err)
		return NULL;

	return tag;
}

__init u32
rb_get_hw_options(void)
{
	u16 tag_len;
	u8 *tag;
	int err;

	err = rb_find_hard_cfg_tag(RB_ID_HW_OPTIONS, &tag, &tag_len);
	if (err)
		return 0;

	return get_u32(tag);
}

static void * __init
__rb_get_wlan_data(u16 id)
{
	u16 tag_len;
	u8 *tag;
	void *buf;
	int err;
	u32 magic;
	size_t src_done;
	size_t dst_done;

	err = rb_find_hard_cfg_tag(RB_ID_WLAN_DATA, &tag, &tag_len);
	if (err) {
		pr_err("no calibration data found\n");
		goto err;
	}

	buf = kmalloc(RB_ART_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		pr_err("no memory for calibration data\n");
		goto err;
	}

	magic = get_u32(tag);
	if (magic == RB_MAGIC_ERD) {
		u8 *erd_data;
		u16 erd_len;

		err = routerboot_find_tag(tag, tag_len, 0x1,
					  &erd_data, &erd_len);
		if (err) {
			pr_err("no ERD data found for id %u\n", id);
			goto err_free;
		}

		dst_done = RB_ART_SIZE;
		err = lzo1x_decompress_safe(erd_data, erd_len, buf, &dst_done);
		if (err) {
			pr_err("unable to decompress calibration data %d\n",
			       err);
			goto err_free;
		}
	} else {
		err = rle_decode((char *) tag, tag_len, buf, RB_ART_SIZE,
				 &src_done, &dst_done);
		if (err) {
			pr_err("unable to decode calibration data\n");
			goto err_free;
		}
	}

	return buf;

err_free:
	kfree(buf);
err:
	return NULL;
}

__init void *
rb_get_wlan_data(void)
{
	return __rb_get_wlan_data(0);
}

__init void *
rb_get_ext_wlan_data(u16 id)
{
	return __rb_get_wlan_data(id);
}

__init const struct rb_info *
rb_init_info(void *data, unsigned int size)
{
	unsigned int offset;

	if (size == 0 || (size % RB_BLOCK_SIZE) != 0)
		return NULL;

	for (offset = 0; offset < size; offset += RB_BLOCK_SIZE) {
		u32 magic;

		magic = get_u32(data + offset);
		switch (magic) {
		case RB_MAGIC_HARD:
			rb_info.hard_cfg_offs = offset;
			break;

		case RB_MAGIC_SOFT:
			rb_info.soft_cfg_offs = offset;
			break;
		}
	}

	if (!rb_info.hard_cfg_offs) {
		pr_err("could not find a valid RouterBOOT hard config\n");
		return NULL;
	}

	if (!rb_info.soft_cfg_offs) {
		pr_err("could not find a valid RouterBOOT soft config\n");
		return NULL;
	}

	rb_info.hard_cfg_size = RB_BLOCK_SIZE;
	rb_info.hard_cfg_data = kmemdup(data + rb_info.hard_cfg_offs,
					RB_BLOCK_SIZE, GFP_KERNEL);
	if (!rb_info.hard_cfg_data)
		return NULL;

	rb_info.board_name = rb_get_board_name();
	rb_info.hw_options = rb_get_hw_options();

	return &rb_info;
}

#if 0
static char *rb_ext_wlan_data;

static ssize_t
rb_ext_wlan_data_read(struct file *filp, struct kobject *kobj,
		      struct bin_attribute *attr, char *buf,
		      loff_t off, size_t count)
{
         if (off + count > attr->size)
                 return -EFBIG;

         memcpy(buf, &rb_ext_wlan_data[off], count);

         return count;
}

static const struct bin_attribute rb_ext_wlan_data_attr = {
	.attr = {
		.name = "ext_wlan_data",
		.mode = S_IRUSR | S_IWUSR,
	},
	.read = rb_ext_wlan_data_read,
	.size = RB_ART_SIZE,
};

static int __init rb_sysfs_init(void)
{
	struct kobject *rb_kobj;
	int ret;

	rb_ext_wlan_data = rb_get_ext_wlan_data(1);
	if (rb_ext_wlan_data == NULL)
		return -ENOENT;

	rb_kobj = kobject_create_and_add("routerboot", firmware_kobj);
	if (rb_kobj == NULL) {
		ret = -ENOMEM;
		pr_err("unable to create sysfs entry\n");
		goto err_free_wlan_data;
	}

	ret = sysfs_create_bin_file(rb_kobj, &rb_ext_wlan_data_attr);
	if (ret) {
		pr_err("unable to create sysfs file, %d\n", ret);
		goto err_put_kobj;
	}

	return 0;

err_put_kobj:
	kobject_put(rb_kobj);
err_free_wlan_data:
	kfree(rb_ext_wlan_data);
	return ret;
}

late_initcall(rb_sysfs_init);
#endif
