/*
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_DEV_M25P80_H
#define _ATH79_DEV_M25P80_H

#include <linux/spi/flash.h>

void ath79_register_m25p80(struct flash_platform_data *pdata) __init;
void ath79_register_m25p80_multi(struct flash_platform_data *pdata) __init;

#endif /* _ATH79_DEV_M25P80_H */
