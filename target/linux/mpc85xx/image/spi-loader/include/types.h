// SPDX-License-Identifier: GPL-2.0-only
/*
 * Code originates from Linux kernel arch/powerpc/boot
 * (types.h, swab.h, of.h)
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define BIT(nr)	(1UL << (nr))

#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x > _y ? _x : _y; })

#define min_t(type, a, b) min(((type) a), ((type) b))
#define max_t(type, a, b) max(((type) a), ((type) b))

static inline uint16_t swab16(uint16_t x)
{
	return  ((x & (uint16_t)0x00ffU) << 8) |
		((x & (uint16_t)0xff00U) >> 8);
}

static inline uint32_t swab32(uint32_t x)
{
	return  ((x & (uint32_t)0x000000ffUL) << 24) |
		((x & (uint32_t)0x0000ff00UL) <<  8) |
		((x & (uint32_t)0x00ff0000UL) >>  8) |
		((x & (uint32_t)0xff000000UL) >> 24);
}

static inline uint64_t swab64(uint64_t x)
{
	return  (uint64_t)((x & (uint64_t)0x00000000000000ffULL) << 56) |
		(uint64_t)((x & (uint64_t)0x000000000000ff00ULL) << 40) |
		(uint64_t)((x & (uint64_t)0x0000000000ff0000ULL) << 24) |
		(uint64_t)((x & (uint64_t)0x00000000ff000000ULL) <<  8) |
		(uint64_t)((x & (uint64_t)0x000000ff00000000ULL) >>  8) |
		(uint64_t)((x & (uint64_t)0x0000ff0000000000ULL) >> 24) |
		(uint64_t)((x & (uint64_t)0x00ff000000000000ULL) >> 40) |
		(uint64_t)((x & (uint64_t)0xff00000000000000ULL) >> 56);
}

#ifdef __LITTLE_ENDIAN__
#define cpu_to_be16(x) swab16(x)
#define be16_to_cpu(x) swab16(x)
#define cpu_to_be32(x) swab32(x)
#define be32_to_cpu(x) swab32(x)
#define cpu_to_be64(x) swab64(x)
#define be64_to_cpu(x) swab64(x)
#else
#define cpu_to_be16(x) (x)
#define be16_to_cpu(x) (x)
#define cpu_to_be32(x) (x)
#define be32_to_cpu(x) (x)
#define cpu_to_be64(x) (x)
#define be64_to_cpu(x) (x)
#endif
