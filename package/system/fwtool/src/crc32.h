/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 *
 * Based on busybox code:
 *   CRC32 table fill function
 *   Copyright (C) 2006 by Rob Sullivan <cogito.ergo.cogito@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __BB_CRC32_H
#define __BB_CRC32_H

static inline void
crc32_filltable(uint32_t *crc_table)
{
	uint32_t polynomial = 0xedb88320;
	uint32_t c;
	int i, j;

	for (i = 0; i < 256; i++) {
		c = i;
		for (j = 8; j; j--)
			c = (c&1) ? ((c >> 1) ^ polynomial) : (c >> 1);

		*crc_table++ = c;
	}
}

static inline uint32_t
crc32_block(uint32_t val, const void *buf, unsigned len, uint32_t *crc_table)
{
	const void *end = (uint8_t*)buf + len;

	while (buf != end) {
		val = crc_table[(uint8_t)val ^ *(uint8_t*)buf] ^ (val >> 8);
		buf = (uint8_t*)buf + 1;
	}
	return val;
}

#endif
