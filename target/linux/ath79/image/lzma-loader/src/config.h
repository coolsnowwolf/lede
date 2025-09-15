/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CONFIG_ICACHE_SIZE	(32 * 1024)
#define CONFIG_DCACHE_SIZE	(64 * 1024)
#define CONFIG_CACHELINE_SIZE	32

#ifndef CONFIG_FLASH_OFFS
#define CONFIG_FLASH_OFFS	0
#endif

#ifndef CONFIG_FLASH_MAX
#define CONFIG_FLASH_MAX	0
#endif

#ifndef CONFIG_FLASH_STEP
#define CONFIG_FLASH_STEP	0x1000
#endif

#endif /* _CONFIG_H_ */
