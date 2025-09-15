/*
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _LIBBRIDGE_PRIVATE_H
#define _LIBBRIDGE_PRIVATE_H

#include <linux/sockios.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/if_bridge.h>

#define MAX_BRIDGES	1024
#define MAX_PORTS	1024

#define SYSFS_CLASS_NET "/sys/class/net/"
#define SYSFS_PATH_MAX	256

#define dprintf(fmt,arg...)

#endif
