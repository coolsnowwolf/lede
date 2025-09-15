// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 */

#ifndef __CACHE_H
#define __CACHE_H

void flush_cache(unsigned long start_addr, unsigned long size);

#endif /* __CACHE_H */
