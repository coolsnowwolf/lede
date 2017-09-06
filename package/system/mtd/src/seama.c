/*
 * seama.c
 *
 * Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * Based on the trx fixup code:
 *   Copyright (C) 2005 Mike Baker
 *   Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
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
 */

#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "mtd.h"
#include "seama.h"
#include "md5.h"

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)           ((((X) & 0x000000FF) << 24) | (((X) & 0x0000FF00) << 8) | (((X) & 0x00FF0000) >> 8) | (((X) & 0xFF000000) >> 24))
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)           (X)
#else
#error unknown endianness!
#endif

ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

int
seama_fix_md5(struct seama_entity_header *shdr, int fd, size_t data_offset, size_t data_size)
{
	char *buf;
	ssize_t res;
	MD5_CTX ctx;
	unsigned char digest[16];
	int i;
	int err = 0;

	buf = malloc(data_size);
	if (!buf) {
		err = -ENOMEM;
		goto err_out;
	}

	res = pread(fd, buf, data_size, data_offset);
	if (res != data_size) {
		perror("pread");
		err = -EIO;
		goto err_free;
	}

	MD5_Init(&ctx);
	MD5_Update(&ctx, buf, data_size);
	MD5_Final(digest, &ctx);

	if (!memcmp(digest, shdr->md5, sizeof(digest))) {
		if (quiet < 2)
			fprintf(stderr, "the header is fixed already\n");
		return -1;
	}

	if (quiet < 2) {
		fprintf(stderr, "new size:%u, new MD5: ", data_size);
		for (i = 0; i < sizeof(digest); i++)
			fprintf(stderr, "%02x", digest[i]);

		fprintf(stderr, "\n");
	}

	/* update the size in the image */
	shdr->size = htonl(data_size);

	/* update the checksum in the image */
	memcpy(shdr->md5, digest, sizeof(digest));

err_free:
	free(buf);
err_out:
	return err;
}

int
mtd_fixseama(const char *mtd, size_t offset, size_t data_size)
{
	int fd;
	char *first_block;
	ssize_t res;
	size_t block_offset;
	size_t data_offset;
	struct seama_entity_header *shdr;

	if (quiet < 2)
		fprintf(stderr, "Trying to fix SEAMA header in %s at 0x%x...\n",
			mtd, offset);

	block_offset = offset & ~(erasesize - 1);
	offset -= block_offset;

	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if (block_offset + erasesize > mtdsize) {
		fprintf(stderr, "Offset too large, device size 0x%x\n",
			mtdsize);
		exit(1);
	}

	first_block = malloc(erasesize);
	if (!first_block) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, first_block, erasesize, block_offset);
	if (res != erasesize) {
		perror("pread");
		exit(1);
	}

	shdr = (struct seama_entity_header *)(first_block + offset);
	if (shdr->magic != htonl(SEAMA_MAGIC)) {
		fprintf(stderr, "No SEAMA header found\n");
		exit(1);
	} else if (!ntohl(shdr->size)) {
		fprintf(stderr, "Seama entity with empty image\n");
		exit(1);
	}

	data_offset = offset + sizeof(struct seama_entity_header) + ntohs(shdr->metasize);
	if (!data_size)
		data_size = mtdsize - data_offset;
	if (data_size > ntohl(shdr->size))
		data_size = ntohl(shdr->size);
	if (seama_fix_md5(shdr, fd, data_offset, data_size))
		goto out;

	if (mtd_erase_block(fd, block_offset)) {
		fprintf(stderr, "Can't erease block at 0x%x (%s)\n",
			block_offset, strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Rewriting block at 0x%x\n", block_offset);

	if (pwrite(fd, first_block, erasesize, block_offset) != erasesize) {
		fprintf(stderr, "Error writing block (%s)\n", strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Done.\n");

out:
	close (fd);
	sync();

	return 0;
}

