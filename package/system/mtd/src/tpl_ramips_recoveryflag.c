/*
 * TP-Link recovery flag set and unset code for ramips target
 *
 * Copyright (C) 2018 David Bauer <mail@david-bauer.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>

#include <mtd/mtd-user.h>
#include <sys/ioctl.h>

#include "mtd.h"


#define TPL_RECOVER_MAGIC	0x89abcdef
#define TPL_NO_RECOVER_MAGIC	0x00000000


struct uboot_args {
	uint32_t magic;
};

int mtd_tpl_recoverflag_write(const char *mtd, const bool recovery_active)
{
	struct erase_info_user erase_info;
	struct uboot_args *args;
	uint32_t magic;
	int ret = 0;
	int fd;

	args = malloc(erasesize);
	if (!args) {
		fprintf(stderr, "Could not allocate memory!\n");
		return -1;
	}

	fd = mtd_check_open(mtd);
	if (fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		ret = -1;
		goto out;
	}

	/* read first block (containing the magic) */
	pread(fd, args, erasesize, 0);

	/* set magic to desired value */
	magic = TPL_RECOVER_MAGIC;
	if (!recovery_active)
		magic = TPL_NO_RECOVER_MAGIC;

	/* no need to write when magic is already set correctly */
	if (magic == args->magic)
		goto out;

	/* erase first block (containing the magic) */
	erase_info.start = 0;
	erase_info.length = erasesize;

	ret = ioctl(fd, MEMERASE, &erase_info);
	if (ret < 0) {
		fprintf(stderr, "failed to erase block: %i\n", ret);
		goto out;
	}

	/* write magic to flash */
	args->magic = magic;

	ret = pwrite(fd, args, erasesize, 0);
	if (ret < 0)
		fprintf(stderr, "failed to write: %i\n", ret);

	sync();
out:
	free(args);
	close(fd);

	return ret;
}
