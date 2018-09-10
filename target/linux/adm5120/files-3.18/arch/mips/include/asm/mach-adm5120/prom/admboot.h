/*
 *  ADMBoot specific definitions
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ADMBOOT_H
#define _ADMBOOT_H

extern int admboot_get_mac_base(u32 offset, u32 len, u8 *mac) __init;

#endif /* _ADMBOOT_H */
