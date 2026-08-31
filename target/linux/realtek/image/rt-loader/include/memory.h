/*
 * rt-loader header
 * (c) 2025 Markus Stockhausen
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stddef.h>
#include "globals.h"

#define CACHE_HIT_INVALIDATE_I		0x10
#define CACHE_HIT_WRITEBACK_INV_D	0x15

#define ioread32(reg)			(*(volatile int *)(reg))
#define iowrite32(val, reg)		(*(volatile int *)(reg) = val)

void flush_cache(void *start_addr, size_t count);
void free(void *ptr);
void *malloc(size_t count);
int memcmp(const void *s1, const void *s2, size_t count);
void *memmove(void *dst, const void *src, size_t count);
void *memcpy(void *dst, const void *src, size_t count);
void *memset(void *dst, int value, size_t count);
size_t strlen(const char *s);
unsigned int crc32(void *m, size_t count);

extern void *_heap_addr;
extern void *_heap_addr_max;

#endif  // _MEMORY_H_
