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

#ifndef _LIBBRIDGE_H
#define _LIBBRIDGE_H

#ifdef linux

int br_init(void);
void br_shutdown(void);

int br_foreach_port(const char *brname,
		    int (*iterator)(const char *br, const char *port, void *arg),
		    void *arg);

int br_foreach_bridge(int (*iterator)(const char *, void *), void *arg);

#else

static inline int br_init(void)
{
	return 0;
}

static inline void br_shutdown(void)
{
}

static inline int
br_foreach_port(const char *brname,
		    int (*iterator)(const char *br, const char *port, void *arg),
		    void *arg)
{
	return 0;
}

static inline int
br_foreach_bridge(int (*iterator)(const char *, void *), void *arg)
{
	return 0;
}

#endif

#endif
