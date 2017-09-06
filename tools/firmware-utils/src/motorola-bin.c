/*
 * motorola-bin.c
 *
 * Copyright (C) 2005-2006 Mike Baker,
 *                         Imre Kaloz <kaloz@openwrt.org>
 *                         D. Hugh Redelmeier
 *                         OpenWrt.org
 *
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
 */

/*
 * Motorola's firmware flashing code requires an extra header.
 * The header is eight bytes (see struct motorola below).
 * This program will take a firmware file and create a new one
 * with this header:
 *	motorola-bin --wr850g WR850G_V403.stripped.trx WR850G_V403.trx
 *
 * Note: Motorola's firmware is distributed with this header.
 * If you need to flash Motorola firmware on a router running OpenWRT,
 * you will to remove this header.  Use the --strip flag:
 *	motorola-bin --strip WR850G_V403.trx WR850G_V403.stripped.trx
 */

/*
 * February 1, 2006
 *
 * Add support for for creating WA840G and WE800G images
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <netinet/in.h>
#include <inttypes.h>

#define BPB 8 /* bits/byte */

static uint32_t crc32[1<<BPB];

static void init_crc32()
{
	const uint32_t poly = ntohl(0x2083b8ed);
	int n;

	for (n = 0; n < 1<<BPB; n++) {
		uint32_t crc = n;
		int bit;

		for (bit = 0; bit < BPB; bit++)
			crc = (crc & 1) ? (poly ^ (crc >> 1)) : (crc >> 1);
		crc32[n] = crc;
	}
}

static uint32_t crc32buf(unsigned char *buf, size_t len)
{
	uint32_t crc = 0xFFFFFFFF;

	for (; len; len--, buf++)
		crc = crc32[(uint8_t)crc ^ *buf] ^ (crc >> BPB);
	return crc;
}

struct motorola {
	uint32_t crc;	// crc32 of the remainder
	uint32_t flags;	// unknown, 105770*
};

static const struct model {
	char digit;	/* a digit signifying model (historical) */
	const char *name;
	uint32_t flags;
} models[] = {
	{ '1', "WR850G", 0x10577050LU },
	{ '2', "WA840G", 0x10577040LU },
	{ '3', "WE800G", 0x10577000LU },
	{ '\0', NULL, 0 }
};

static void usage(const char *) __attribute__ (( __noreturn__ ));

static void usage(const char *mess)
{
	const struct model *m;

	fprintf(stderr, "Error: %s\n", mess);
	fprintf(stderr, "Usage: motorola-bin -device|--strip infile outfile\n");
	fprintf(stderr, "Known devices: ");

	for (m = models; m->digit != '\0'; m++)
		fprintf(stderr, " %c - %s", m->digit, m->name);

	fprintf(stderr, "\n");
	exit(1);
}

int main(int argc, char **argv)
{
	off_t len;	// of original firmware
	int fd;
	void *trx;	// pointer to original firmware (mmmapped)
	struct motorola *firmware;	// pionter to prefix + copy of original firmware
	uint32_t flags;

	// verify parameters

	if (argc != 4)
		usage("wrong number of arguments");

	// mmap trx file
	if ((fd = open(argv[2], O_RDONLY))  < 0
	|| (len = lseek(fd, 0, SEEK_END)) < 0
	|| (trx = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0)) == (void *) (-1)
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error loading file %s: %s\n", argv[2], strerror(errno));
		exit(1);
	}

	init_crc32();

	if (strcmp(argv[1], "--strip") == 0)
	{
		const char *ugh = NULL;

		if (len < sizeof(struct motorola)) {
			ugh = "input file too short";
		} else {
			const struct model *m;

			firmware = trx;
			if (htonl(crc32buf(trx + offsetof(struct motorola, flags), len - offsetof(struct motorola, flags))) != firmware->crc)
				ugh = "Invalid CRC";
			for (m = models; ; m++) {
				if (m->digit == '\0') {
					if (ugh == NULL)
						ugh = "unrecognized flags field";
					break;
				}
				if (firmware->flags == htonl(m->flags)) {
					fprintf(stderr, "Firmware for Motorola %s\n", m->name);
					break;
				}
			}
		}

		if (ugh != NULL) {
			fprintf(stderr, "%s\n", ugh);
			exit(3);
		} else {
			// all is well, write the file without the prefix
			if ((fd = open(argv[3], O_CREAT|O_WRONLY|O_TRUNC,0644)) < 0
			|| write(fd, trx + sizeof(struct motorola), len - sizeof(struct motorola)) !=  len - sizeof(struct motorola)
			|| close(fd) < 0)
			{
				fprintf(stderr, "Error storing file %s: %s\n", argv[3], strerror(errno));
				exit(2);
			}
		}
		
	} else {
		// setup the firmware flags magic number
		const struct model *m;
		const char *df = argv[1];

		if (*df != '-')
			usage("first argument must start with -");
		if (*++df == '-')
			++df;	/* allow but don't require second - */

		for (m = models; ; m++) {
			if (m->digit == '\0')
				usage("unrecognized device specified");
			if ((df[0] == m->digit && df[1] == '\0') || strcasecmp(df, m->name) == 0) {
				flags = m->flags;
				break;
			}
		}


		// create a firmware image in memory
		// and copy the trx to it
		firmware = malloc(sizeof(struct motorola) + len);
		memcpy(&firmware[1], trx, len);

		// setup the motorola headers
		firmware->flags = htonl(flags);

		// CRC of flags + firmware
		firmware->crc = htonl(crc32buf((unsigned char *)&firmware->flags, sizeof(firmware->flags) + len));

		// write the firmware
		if ((fd = open(argv[3], O_CREAT|O_WRONLY|O_TRUNC,0644)) < 0
		|| write(fd, firmware, sizeof(struct motorola) + len) != sizeof(struct motorola) + len
		|| close(fd) < 0)
		{
			fprintf(stderr, "Error storing file %s: %s\n", argv[3], strerror(errno));
			exit(2);
		}

		free(firmware);
	}

	munmap(trx,len);

	return 0;
}
