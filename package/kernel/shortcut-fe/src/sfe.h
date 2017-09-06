/*
 * sfe.h
 *	Shortcut forwarding engine.
 *
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Debug output verbosity level.
 */
#define DEBUG_LEVEL 0

#if (DEBUG_LEVEL < 1)
#define DEBUG_ERROR(s, ...)
#else
#define DEBUG_ERROR(s, ...) \
do { \
	printk("%s[%u]: ERROR:", __FILE__, __LINE__); \
	printk(s, ##__VA_ARGS__); \
} while (0)
#endif

#if (DEBUG_LEVEL < 2)
#define DEBUG_WARN(s, ...)
#else
#define DEBUG_WARN(s, ...) \
do { \
	printk("%s[%u]: WARN:", __FILE__, __LINE__); \
	printk(s, ##__VA_ARGS__); \
} while (0)
#endif

#if (DEBUG_LEVEL < 3)
#define DEBUG_INFO(s, ...)
#else
#define DEBUG_INFO(s, ...) \
do { \
	printk("%s[%u]: INFO:", __FILE__, __LINE__); \
	printk(s, ##__VA_ARGS__); \
} while (0)
#endif

#if (DEBUG_LEVEL < 4)
#define DEBUG_TRACE(s, ...)
#else
#define DEBUG_TRACE(s, ...) \
do { \
	printk("%s[%u]: TRACE:", __FILE__, __LINE__); \
	printk(s, ##__VA_ARGS__); \
} while (0)
#endif
