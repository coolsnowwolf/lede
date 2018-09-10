/*
 * Copyright (C) 2003 Artec Design Ltd.
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARM_ARCH_IO_H
#define __ASM_ARM_ARCH_IO_H

#define IO_SPACE_LIMIT		0xffffffff

/*
 * We don't support ins[lb]/outs[lb].  Make them fault.
 */
#define __raw_readsb(p, d, l)	do { *(int *)0 = 0; } while (0)
#define __raw_readsl(p, d, l)	do { *(int *)0 = 0; } while (0)
#define __raw_writesb(p, d, l)	do { *(int *)0 = 0; } while (0)
#define __raw_writesl(p, d, l)	do { *(int *)0 = 0; } while (0)

#define __io(a)		__typesafe_io(a)
#define __mem_pci(a)	(a)

#endif
