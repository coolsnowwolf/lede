/*
 * trx.c
 *
 * Copyright (C) 2005 Mike Baker
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
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

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include "mtd.h"
#include "crc32.h"

#define TRX_CRC32_DATA_OFFSET	12	/* First 12 bytes are not covered by CRC32 */
#define TRX_CRC32_DATA_SIZE	16
struct trx_header {
	uint32_t magic;		/* "HDR0" */
	uint32_t len;		/* Length of file including header */
	uint32_t crc32;		/* 32-bit CRC from flag_version to end of file */
	uint32_t flag_version;	/* 0:15 flags, 16:31 version */
	uint32_t offsets[3];    /* Offsets of partitions from start of header */
};

#define min(x,y) ({		\
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x < _y ? _x : _y; })

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
trx_fixup(int fd, const char *name)
{
	struct mtd_info_user mtdInfo;
	unsigned long len;
	struct trx_header *trx;
	void *ptr, *scan;
	int bfd;

	if (ioctl(fd, MEMGETINFO, &mtdInfo) < 0) {
		fprintf(stderr, "Failed to get mtd info\n");
		goto err;
	}

	len = mtdInfo.size;
	if (mtdInfo.size <= 0) {
		fprintf(stderr, "Invalid MTD device size\n");
		goto err;
	}

	bfd = mtd_open(name, true);
	ptr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, bfd, 0);
	if (!ptr || (ptr == (void *) -1)) {
		perror("mmap");
		fprintf(stderr, "Mapping the TRX header failed\n");
		goto err1;
	}

	trx = ptr;
	if (ntohl(trx->magic) != opt_trxmagic) {
		fprintf(stderr, "TRX header not found\n");
		goto err;
	}

	scan = ptr + offsetof(struct trx_header, flag_version);
	trx->crc32 = crc32buf(scan, trx->len - (scan - ptr));
	msync(ptr, sizeof(struct trx_header), MS_SYNC|MS_INVALIDATE);
	munmap(ptr, len);
	close(bfd);
	return 0;

err1:
	close(bfd);
err:
	return -1;
}

int
trx_check(int imagefd, const char *mtd, char *buf, int *len)
{
	const struct trx_header *trx = (const struct trx_header *) buf;
	int fd;

	if (strcmp(mtd, "firmware") != 0)
		return 1;

	if (*len < 32) {
		*len += read(imagefd, buf + *len, 32 - *len);
		if (*len < 32) {
			fprintf(stdout, "Could not get image header, file too small (%d bytes)\n", *len);
			return 0;
		}
	}

	if (ntohl(trx->magic) != opt_trxmagic ||
	    trx->len < sizeof(struct trx_header)) {
		if (quiet < 2) {
			fprintf(stderr, "Bad trx header\n");
			fprintf(stderr, "This is not the correct file format; refusing to flash.\n"
					"Please specify the correct file or use -f to force.\n");
		}
		return 0;
	}

	/* check if image fits to mtd device */
	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if(mtdsize < trx->len) {
		fprintf(stderr, "Image too big for partition: %s\n", mtd);
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

int
mtd_fixtrx(const char *mtd, size_t offset, size_t data_size)
{
	size_t data_offset;
	int fd;
	struct trx_header *trx;
	char *first_block;
	char *buf, *to;
	ssize_t res;
	size_t block_offset;

	if (quiet < 2)
		fprintf(stderr, "Trying to fix trx header in %s at 0x%x...\n", mtd, offset);

	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	data_offset = offset + TRX_CRC32_DATA_OFFSET;
	if (data_size)
		data_size += TRX_CRC32_DATA_SIZE;
	else
		data_size = erasesize - TRX_CRC32_DATA_OFFSET;

	block_offset = offset & ~(erasesize - 1);
	offset -= block_offset;

	if (data_offset + data_size > mtdsize) {
		fprintf(stderr, "Offset too large, device size 0x%x\n", mtdsize);
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

	trx = (struct trx_header *)(first_block + offset);
	if (ntohl(trx->magic) != opt_trxmagic) {
		fprintf(stderr, "No trx magic found\n");
		exit(1);
	}

	buf = malloc(data_size);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	to = buf;
	while (data_size) {
		size_t read_block_offset = data_offset & ~(erasesize - 1);
		size_t read_chunk;

		read_chunk = erasesize - (data_offset & (erasesize - 1));
		read_chunk = min(read_chunk, data_size);

		/* Read from good blocks only to match CFE behavior */
		if (!mtd_block_is_bad(fd, read_block_offset)) {
			res = pread(fd, to, read_chunk, data_offset);
			if (res != read_chunk) {
				perror("pread");
				exit(1);
			}
			to += read_chunk;
		}

		data_offset += read_chunk;
		data_size -= read_chunk;
	}
	data_size = to - buf;

	if (trx->len == STORE32_LE(data_size + TRX_CRC32_DATA_OFFSET) &&
	    trx->crc32 == STORE32_LE(crc32buf(buf, data_size))) {
		if (quiet < 2)
			fprintf(stderr, "Header already fixed, exiting\n");
		close(fd);
		return 0;
	}

	trx->len = STORE32_LE(data_size + offsetof(struct trx_header, flag_version));

	trx->crc32 = STORE32_LE(crc32buf(buf, data_size));
	if (mtd_erase_block(fd, block_offset)) {
		fprintf(stderr, "Can't erease block at 0x%x (%s)\n", block_offset, strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "New crc32: 0x%x, rewriting block\n", trx->crc32);

	if (pwrite(fd, first_block, erasesize, block_offset) != erasesize) {
		fprintf(stderr, "Error writing block (%s)\n", strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Done.\n");

	close (fd);
	sync();
	return 0;

}
