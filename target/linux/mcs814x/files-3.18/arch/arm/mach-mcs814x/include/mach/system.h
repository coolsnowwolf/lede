/*
 * Copyright (C) 2003 Artec Design Ltd.
 * Copyright (C) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_SYSTEM_H
#define __ASM_ARCH_SYSTEM_H

static inline void arch_idle(void)
{
	cpu_do_idle();
}
#endif
