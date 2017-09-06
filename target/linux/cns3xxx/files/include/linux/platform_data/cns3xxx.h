/*
 * arch/arm/mach-cns3xxx/include/mach/platform.h
 *
 * Copyright 2011 Gateworks Corporation
 *		  Chris Lang <clang@gateworks.com
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_PLATFORM_H
#define __ASM_ARCH_PLATFORM_H

#ifndef __ASSEMBLY__

/* Information about built-in Ethernet MAC interfaces */
struct cns3xxx_plat_info {
	u8 ports; /* Bitmap of enabled Ports */
	u8 hwaddr[4][6];
	u32 phy[3];
};

#endif /* __ASM_ARCH_PLATFORM_H */
#endif
