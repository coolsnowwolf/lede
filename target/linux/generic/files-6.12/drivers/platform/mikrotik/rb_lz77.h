/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 John Thomson
 */

#ifndef __MIKROTIK_WLAN_LZ77_H__
#define __MIKROTIK_WLAN_LZ77_H__

#include <linux/errno.h>

#ifdef CONFIG_MIKROTIK_WLAN_DECOMPRESS_LZ77
/**
 * rb_lz77_decompress
 *
 * @in:			compressed data ptr
 * @in_len:		length of compressed data
 * @out:		buffer ptr to decompress into
 * @out_len:		length of decompressed buffer in input,
 *			length of decompressed data in success
 *
 * Returns 0 on success, or negative error
 */
int rb_lz77_decompress(const u8 *in, const size_t in_len, u8 *out,
		       size_t *out_len);

#else /* CONFIG_MIKROTIK_WLAN_DECOMPRESS_LZ77 */

static inline int rb_lz77_decompress(const u8 *in, const size_t in_len, u8 *out,
				     size_t *out_len)
{
	return -EOPNOTSUPP;
}

#endif /* CONFIG_MIKROTIK_WLAN_DECOMPRESS_LZ77 */
#endif /* __MIKROTIK_WLAN_LZ77_H__ */
