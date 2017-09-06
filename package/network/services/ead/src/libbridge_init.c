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

#ifdef linux

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/in6.h>
#include <linux/if_bridge.h>

#include "libbridge.h"
#include "libbridge_private.h"

static int br_socket_fd = -1;

int br_init(void)
{
	if ((br_socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
		return errno;
	return 0;
}

void br_shutdown(void)
{
	close(br_socket_fd);
	br_socket_fd = -1;
}

/* If /sys/class/net/XXX/bridge exists then it must be a bridge */
static int isbridge(const struct dirent *entry)
{
	char path[SYSFS_PATH_MAX];
	struct stat st;

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/bridge", entry->d_name);
	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

/*
 * New interface uses sysfs to find bridges
 */
static int new_foreach_bridge(int (*iterator)(const char *name, void *),
			      void *arg)
{
	struct dirent **namelist;
	int i, count = 0;

	count = scandir(SYSFS_CLASS_NET, &namelist, isbridge, alphasort);
	if (count < 0)
		return -1;

	for (i = 0; i < count; i++) {
		if (iterator(namelist[i]->d_name, arg))
			break;
	}

	for (i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return count;
}

/*
 * Go over all bridges and call iterator function.
 * if iterator returns non-zero then stop.
 */
int br_foreach_bridge(int (*iterator)(const char *, void *), void *arg)
{
	return new_foreach_bridge(iterator, arg);
}

/*
 * Iterate over all ports in bridge (using sysfs).
 */
int br_foreach_port(const char *brname,
		    int (*iterator)(const char *br, const char *port, void *arg),
		    void *arg)
{
	int i, count;
	struct dirent **namelist;
	char path[SYSFS_PATH_MAX];

	snprintf(path, SYSFS_PATH_MAX, SYSFS_CLASS_NET "%s/brif", brname);
	count = scandir(path, &namelist, 0, alphasort);

	for (i = 0; i < count; i++) {
		if (namelist[i]->d_name[0] == '.'
		    && (namelist[i]->d_name[1] == '\0'
			|| (namelist[i]->d_name[1] == '.'
			    && namelist[i]->d_name[2] == '\0')))
			continue;

		if (iterator(brname, namelist[i]->d_name, arg))
			break;
	}
	for (i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return count;
}

#endif
