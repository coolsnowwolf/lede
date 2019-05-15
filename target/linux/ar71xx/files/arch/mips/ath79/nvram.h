/*
 *  Atheros AR71xx minimal nvram support
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_NVRAM_H
#define _ATH79_NVRAM_H

char *ath79_nvram_find_var(const char *name, const char *buf,
			   unsigned buf_len);
int ath79_nvram_parse_mac_addr(const char *nvram, unsigned nvram_len,
			       const char *name, char *mac);

#endif /* _ATH79_NVRAM_H */
