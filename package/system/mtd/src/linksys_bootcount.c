/*
 * Linksys boot counter reset code for mtd
 *
 * Copyright (C) 2013 Jonas Gorski <jogo@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <endian.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#include "mtd.h"

#define BOOTCOUNT_MAGIC	0x20110811

struct bootcounter {
	uint32_t magic;
	uint32_t count;
	uint32_t checksum;
};

static char page[2048];

int mtd_resetbc(const char *mtd)
{
	struct mtd_info_user mtd_info;
	struct bootcounter *curr = (struct bootcounter *)page;
	unsigned int i;
	int last_count = 0;
	int num_bc;
	int fd;
	int ret;

	fd = mtd_check_open(mtd);

	if (ioctl(fd, MEMGETINFO, &mtd_info) < 0) {
		fprintf(stderr, "failed to get mtd info!\n");
		return -1;
	}

	num_bc = mtd_info.size / mtd_info.writesize;

	for (i = 0; i < num_bc; i++) {
		pread(fd, curr, sizeof(*curr), i * mtd_info.writesize);

		if (curr->magic != BOOTCOUNT_MAGIC && curr->magic != 0xffffffff) {
			fprintf(stderr, "unexpected magic %08x, bailing out\n", curr->magic);
			goto out;
		}

		if (curr->magic == 0xffffffff)
			break;

		last_count = curr->count;
	}

	/* no need to do writes when last boot count is already 0 */
	if (last_count == 0)
		goto out;


	if (i == num_bc) {
		struct erase_info_user erase_info;
		erase_info.start = 0;
		erase_info.length = mtd_info.size;

		/* erase block */
		ret = ioctl(fd, MEMERASE, &erase_info);
		if (ret < 0) {
			fprintf(stderr, "failed to erase block: %i\n", ret);
			return -1;
		}

		i = 0;
	}

	memset(curr, 0xff, mtd_info.writesize);

	curr->magic = BOOTCOUNT_MAGIC;
	curr->count = 0;
	curr->checksum = BOOTCOUNT_MAGIC;

	ret = pwrite(fd, curr, mtd_info.writesize, i * mtd_info.writesize);
	if (ret < 0)
		fprintf(stderr, "failed to write: %i\n", ret);
	sync();
out:
	close(fd);

	return 0;
}
