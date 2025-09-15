/*
 * patch-dtb.c - patch a dtb into an image
 *
 * Copyright (C) 2006 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * based on patch-cmdline.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

#define DTB_MAX	(16 * 1024)

int main(int argc, char **argv)
{
	int fd, fddtb, found = 0, len, ret = -1;
	char *ptr, *ptrdtb, *p;
	struct stat s;
	unsigned int search_space , dtb_max_size;

	if (argc <= 2 || argc > 4) {
		fprintf(stderr, "Usage: %s <file> <dtb> [size]\n", argv[0]);
		goto err1;
	} else if (argc == 3) {
		fprintf(stdout, "DT size used is default of 16KB\n");
		search_space = dtb_max_size = DTB_MAX;
	} else {
		search_space = dtb_max_size = atoi(argv[3]);
	}

	if (stat(argv[2], &s)) {
		fprintf(stderr, "DTB not found\n");
		goto err1;
	}

	len = s.st_size;
	if (len + 8 > dtb_max_size) {
		fprintf(stderr, "DTB too big\n");
		goto err1;
	}

        if (((fddtb = open(argv[2], O_RDONLY)) < 0) ||
		(ptrdtb = (char *) mmap(0, dtb_max_size, PROT_READ, MAP_SHARED, fddtb, 0)) == (void *) (-1)) {
		fprintf(stderr, "Could not open DTB");
		goto err2;
	}

	if (((fd = open(argv[1], O_RDWR)) < 0) ||
		(ptr = (char *) mmap(0, search_space + dtb_max_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == (void *) (-1)) {
		fprintf(stderr, "Could not open kernel image");
		goto err3;
	}

	for (p = ptr; p < (ptr + search_space); p += 4) {
		if (memcmp(p, "OWRTDTB:", 8) == 0) {
			found = 1;
			p += 8;
			break;
		}
	}
	if (!found) {
		fprintf(stderr, "DTB marker not found!\n");
		goto err4;
	}

	memset(p, 0, dtb_max_size - 8);
	memcpy(p, ptrdtb, len);
	msync(p, len, MS_SYNC|MS_INVALIDATE);
	ret = 0;

err4:
	munmap((void *) ptr, len);
err3:
	if (fd > 0)
		close(fd);
	munmap((void *) ptrdtb, len);
err2:
	if (fddtb > 0)
		close(fddtb);
err1:
	return ret;
}
