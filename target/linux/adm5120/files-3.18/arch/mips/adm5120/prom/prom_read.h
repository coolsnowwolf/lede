/*
 *  Generic prom definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ADM5120_PROM_H_
#define _ADM5120_PROM_H_

/*
 * Helper routines
 */
static inline u16 prom_read_le16(void *buf)
{
	u8 *p = buf;

	return ((u16)p[0] + ((u16)p[1] << 8));
}

static inline u32 prom_read_le32(void *buf)
{
	u8 *p = buf;

	return ((u32)p[0] + ((u32)p[1] << 8) + ((u32)p[2] << 16) +
		((u32)p[3] << 24));
}

static inline u16 prom_read_be16(void *buf)
{
	u8 *p = buf;

	return (((u16)p[0] << 8) + (u16)p[1]);
}

static inline u32 prom_read_be32(void *buf)
{
	u8 *p = buf;

	return (((u32)p[0] << 24) + ((u32)p[1] << 16) + ((u32)p[2] << 8) +
		((u32)p[3]));
}

#endif /* _ADM5120_PROM_H_ */


