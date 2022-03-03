/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "LzmaWrapper.h"

#define FW_NAME		"/tmp/firmware-speedport-w-921v-1.48.000.bin"

#define MAGIC		0x50
#define MAGIC_SZ	0x3FFC00
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define MAGIC_PART	0x12345678
#define MAGIC_LZMA	0x8000005D
#define MAGIC_ANNEX_B	0x3C
#define MAGIC_TAPI	0x5A
#else
#define MAGIC_PART	0x78563412
#define MAGIC_LZMA	0x5D000080
#define MAGIC_ANNEX_B	0x3C000000
#define MAGIC_TAPI	0x5A000000
#endif


const char* part_type(unsigned int id)
{
	switch(id) {
	case MAGIC_ANNEX_B:
		return "/tmp/vr9_dsl_fw_annex_b.bin";
	case MAGIC_TAPI:
		return "/tmp/vr9_tapi_fw.bin";
	}
	printf("\tUnknown lzma type 0x%02X\n", id);
	return "/tmp/unknown.lzma";
}

int main(int argc, char **argv)
{
	struct stat s;
	unsigned char *buf_orig;
	unsigned int *buf;
	int buflen;
	int fd;
	int i;
	int err;
	int start = 0, end = 0;

	printf("Arcadyan Firmware cutter v0.1\n");
	printf("-----------------------------\n");
	printf("This tool extracts the different parts of an arcadyan firmware update file\n");
	printf("This tool is for private use only. The Firmware that gets extracted has a license that forbids redistribution\n");
	printf("Please only run this if you understand the risks\n\n");
	printf("I understand the risks ? (y/N)\n");

	if (getchar() != 'y')
		return -1;

	if (stat(FW_NAME, &s) != 0) {
		printf("Failed to find %s\n", FW_NAME);
		printf("Ask Google or try https://www.telekom.de/hilfe/downloads/firmware-speedport-w-921v-1.48.000.bin\n");
		return -1;
	}

	buf_orig = malloc(s.st_size);
	buf = malloc(s.st_size);
	if (!buf_orig || !buf) {
		printf("Failed to alloc %d bytes\n", s.st_size);
		return -1;
	}

	fd = open(FW_NAME, O_RDONLY);
	if (fd < 0) {
		printf("Unable to open %s\n", FW_NAME);
		return -1;
	}


	buflen = read(fd, buf_orig, s.st_size);
	close(fd);
	if (buflen != s.st_size) {
		printf("Loaded %d instead of %d bytes inside %s\n", buflen, s.st_size, FW_NAME);
		return -1;
	}

	/* <magic> */
	buf_orig++;
	buflen -= 1;
	for (i = 0; i < MAGIC_SZ; i++) {
		if ((i % 16) < 3)
			buf_orig[i] = buf_orig[i + 16] ^ MAGIC;
		else
			buf_orig[i] = buf_orig[i] ^ MAGIC;
	}
	buflen -= 3;
	memmove(&buf_orig[MAGIC_SZ], &buf_orig[MAGIC_SZ + 3], buflen - MAGIC_SZ);
	memcpy(buf, buf_orig, s.st_size);

	/* </magic> */
	do {
		if (buf[end] == MAGIC_PART) {
			end += 2;
			printf("Found partition at 0x%08X with size %d\n",
				start * sizeof(unsigned int),
				(end - start) * sizeof(unsigned int));
			if (buf[start] == MAGIC_LZMA) {
				int dest_len = 1024 * 1024;
				int len = buf[end - 3];
				unsigned int id = buf[end - 6];
				const char *type = part_type(id);
				unsigned char *dest;

				dest = malloc(dest_len);
				if (!dest) {
					printf("Failed to alloc dest buffer\n");
					return -1;
				}

				if (lzma_inflate((unsigned char*)&buf[start], len, dest, &dest_len)) {
					printf("Failed to decompress data\n");
					return -1;
				}

				fd = creat(type, S_IRUSR | S_IWUSR);
				if (fd != -1) {
					if (write(fd, dest, dest_len) != dest_len)
						printf("\tFailed to write %d bytes\n", dest_len);
					else
						printf("\tWrote %d bytes to %s\n", dest_len, type);
					close(fd);
				} else {
					printf("\tFailed to open %s\n", type);
				}
				free(dest);
			} else {
				printf("\tThis is not lzma\n");
			}
			start = end;
		} else {
			end++;
		}
	} while(end < buflen / sizeof(unsigned int));

	return 0;
}
