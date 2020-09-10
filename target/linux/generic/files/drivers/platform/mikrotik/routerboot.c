// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for MikroTik RouterBoot flash data. Common routines.
 *
 * Copyright (C) 2020 Thibaut VARÈNE <hacks+kernel@slashdirt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sysfs.h>

#include "routerboot.h"

static struct kobject *rb_kobj;

/**
 * routerboot_tag_find() - Locate a given tag in routerboot config data.
 * @bufhead: the buffer to look into. Must start with a tag node.
 * @buflen: size of bufhead
 * @tag_id: the tag identifier to look for
 * @pld_ofs: will be updated with tag payload offset in bufhead, if tag found
 * @pld_len: will be updated with tag payload size, if tag found
 *
 * This incarnation of tag_find() does only that: it finds a specific routerboot
 * tag node in the input buffer. Routerboot tag nodes are u32 values:
 * - The low nibble is the tag identification number,
 * - The high nibble is the tag payload length (node excluded) in bytes.
 * The payload immediately follows the tag node. Tag nodes are 32bit-aligned.
 * The returned pld_ofs will always be aligned. pld_len may not end on 32bit
 * boundary (the only known case is when parsing ERD data).
 * The nodes are cpu-endian on the flash media. The payload is cpu-endian when
 * applicable. Tag nodes are not ordered (by ID) on flash.
 *
 * Return: 0 on success (tag found) or errno
 */
int routerboot_tag_find(const u8 *bufhead, const size_t buflen, const u16 tag_id,
			u16 *pld_ofs, u16 *pld_len)
{
	const u32 *datum, *bufend;
	u32 node;
	u16 id, len;
	int ret;

	if (!bufhead || !tag_id)
		return -EINVAL;

	ret = -ENOENT;
	datum = (const u32 *)bufhead;
	bufend = (const u32 *)(bufhead + buflen);

	while (datum < bufend) {
		node = *datum++;

		/* Tag list ends with null node */
		if (!node)
			break;

		id = node & 0xFFFF;
		len = node >> 16;

		if (tag_id == id) {
			if (datum >= bufend)
				break;

			if (pld_ofs)
				*pld_ofs = (u16)((u8 *)datum - bufhead);
			if (pld_len)
				*pld_len = len;

			ret = 0;
			break;
		}

		/*
		 * The only known situation where len may not end on 32bit
		 * boundary is within ERD data. Since we're only extracting
		 * one tag (the first and only one) from that data, we should
		 * never need to forcefully ALIGN(). Do it anyway, this is not a
		 * performance path.
		 */
		len = ALIGN(len, sizeof(*datum));
		datum += len / sizeof(*datum);
	}

	return ret;
}

/**
 * routerboot_rle_decode() - Simple RLE (MikroTik variant) decoding routine.
 * @in: input buffer to decode
 * @inlen: size of in
 * @out: output buffer to write decoded data to
 * @outlen: pointer to out size when function is called, will be updated with
 * size of decoded output on return
 *
 * MikroTik's variant of RLE operates as follows, considering a signed run byte:
 * - positive run => classic RLE
 * - negative run => the next -<run> bytes must be copied verbatim
 * The API is matched to the lzo1x routines for convenience.
 *
 * NB: The output buffer cannot overlap with the input buffer.
 *
 * Return: 0 on success or errno
 */
int routerboot_rle_decode(const u8 *in, size_t inlen, u8 *out, size_t *outlen)
{
	int ret, run, nbytes;	// use native types for speed
	u8 byte;

	if (!in || (inlen < 2) || !out)
		return -EINVAL;

	ret = -ENOSPC;
	nbytes = 0;
	while (inlen >= 2) {
		run = *in++;
		inlen--;

		/* Verbatim copies */
		if (run & 0x80) {
			/* Invert run byte sign */
			run = ~run & 0xFF;
			run++;

			if (run > inlen)
				goto fail;

			inlen -= run;

			nbytes += run;
			if (nbytes > *outlen)
				goto fail;

			/* Basic memcpy */
			while (run-- > 0)
				*out++ = *in++;
		}
		/* Stream of half-words RLE: <run><byte>. run == 0 is ignored */
		else {
			byte = *in++;
			inlen--;

			nbytes += run;
			if (nbytes > *outlen)
				goto fail;

			while (run-- > 0)
				*out++ = byte;
		}
	}

	ret = 0;
fail:
	*outlen = nbytes;
	return ret;
}

static int __init routerboot_init(void)
{
	rb_kobj = kobject_create_and_add("mikrotik", firmware_kobj);
	if (!rb_kobj)
		return -ENOMEM;

	/*
	 * We ignore the following return values and always register.
	 * These init() routines are designed so that their failed state is
	 * always manageable by the corresponding exit() calls.
	 */
	rb_hardconfig_init(rb_kobj);
	rb_softconfig_init(rb_kobj);

	return 0;
}

static void __exit routerboot_exit(void)
{
	rb_softconfig_exit();
	rb_hardconfig_exit();
	kobject_put(rb_kobj);	// recursive afaict
}

/* Common routines */

ssize_t routerboot_tag_show_string(const u8 *pld, u16 pld_len, char *buf)
{
	return scnprintf(buf, pld_len+1, "%s\n", pld);
}

ssize_t routerboot_tag_show_u32s(const u8 *pld, u16 pld_len, char *buf)
{
	char *out = buf;
	u32 *data;	// cpu-endian

	/* Caller ensures pld_len > 0 */
	if (pld_len % sizeof(*data))
		return -EINVAL;

	data = (u32 *)pld;

	do {
		out += sprintf(out, "0x%08x\n", *data);
		data++;
	} while ((pld_len -= sizeof(*data)));

	return out - buf;
}

module_init(routerboot_init);
module_exit(routerboot_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MikroTik RouterBoot sysfs support");
MODULE_AUTHOR("Thibaut VARENE");
