/*
 * Gemini GMAC specific defines
 *
 * Copyright (C) 2008, Paulius Zaleckas <paulius.zaleckas@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __NET_GEMINI_PLATFORM_H__
#define __NET_GEMINI_PLATFORM_H__

#include <linux/phy.h>

struct gemini_gmac_platform_data {
	char *bus_id[2]; /* NULL means that this port is not used */
	phy_interface_t interface[2];
};

#endif /* __NET_GEMINI_PLATFORM_H__ */
