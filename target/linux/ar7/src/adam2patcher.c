/*
 * patcher.c - ADAM2 patcher for Netgear DG834 (and compatible)
 *
 * Copyright (C) 2006 Felix Fietkau
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#include <sys/ioctl.h>

int main(int argc, char **argv)
{
	int fd;
	char *ptr;
	uint32_t *i;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	if (((fd = open(argv[1], O_RDWR)) < 0)
			|| ((ptr = mmap(0, 128 * 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == (void *) (-1))) {
		fprintf(stderr, "Can't open file\n");
		exit(1);
	}

	i = (uint32_t *) &ptr[0x3944];
	if (*i == 0x0c000944) {
		fprintf(stderr, "Unpatched ADAM2 detected. Patching... ");
		*i = 0x00000000;
		msync(i, sizeof(*i), MS_SYNC|MS_INVALIDATE);
		fprintf(stderr, "done!\n");
	} else if (*i == 0x00000000) {
		fprintf(stderr, "Patched ADAM2 detected.\n");
	} else {
		fprintf(stderr, "Unknown ADAM2 detected. Can't patch!\n");
	}

	close(fd);		
}
