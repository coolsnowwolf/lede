/*
 *  Atheros AR71xx DSA switch device support
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_DEV_DSA_H
#define _ATH79_DEV_DSA_H

#include <net/dsa.h>

void ath79_register_dsa(struct device *netdev,
			struct device *miidev,
			struct dsa_platform_data *d);

#endif /* _ATH79_DEV_DSA_H */
