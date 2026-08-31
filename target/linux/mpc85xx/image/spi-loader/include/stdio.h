// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#pragma once

#include <serial.h>
#include <types.h>

static inline int getchar(void)
{
	return serial_console_getchar();
}

static inline int tstc(void)
{
	return serial_console_tstc();
}

static inline int putchar(char c)
{
	if (c == '\n')
		serial_console_putchar('\r');
	serial_console_putchar(c);
	return 0;
}

int puts(const char *s);

/* Utility functions */
void put_u4(uint8_t v);
void put_u8(uint8_t v);
void put_u16(uint16_t v);
void put_u32(uint32_t v);
void put_ptr(const void *p);
void put_array(const void *p, size_t l);

#define put_with_label(label, put, value) do { \
		puts(label); \
		put(value); \
		puts("\n"); \
	} while (0)
