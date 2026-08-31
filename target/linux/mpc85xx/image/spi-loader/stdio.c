// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#include <stdio.h>

int puts(const char *s)
{
	while (*s)
		putchar(*s++);
	return 0;
}

void put_u4(uint8_t v)
{
	v &= 0xf;
	switch (v) {
	case 0x0 ... 0x9:
		putchar('0' + v);
		break;
	case 0xa ... 0xf:
		putchar('a' + (v - 0xa));
	}
}

void put_u8(uint8_t v)
{
	put_u4(v >> 4);
	put_u4(v);
}

void put_u16(uint16_t v)
{
	put_u8(v >> 8);
	put_u8(v);
}

void put_u32(uint32_t v)
{
	put_u16(v >> 16);
	put_u16(v);
}

void put_ptr(const void *p)
{
	put_u32((uint32_t)p);
}

void put_array(const void *p, size_t l)
{
	const uint8_t *c = p;
	size_t i;
	for (i = 0; i < l; i++) {
		put_u8(c[i]);
		putchar(' ');
	}
	putchar('\n');
}
