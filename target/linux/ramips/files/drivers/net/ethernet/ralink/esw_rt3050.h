/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#ifndef _RALINK_ESW_RT3052_H__
#define _RALINK_ESW_RT3052_H__

#ifdef CONFIG_NET_RALINK_ESW_RT3052

int __init mtk_switch_init(void);
void mtk_switch_exit(void);

#else

static inline int __init mtk_switch_init(void) { return 0; }
static inline void mtk_switch_exit(void) { }

#endif
#endif
