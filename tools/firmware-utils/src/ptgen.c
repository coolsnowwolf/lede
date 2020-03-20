/*
 * ptgen - partition table generator
 * Copyright (C) 2006 by Felix Fietkau <nbd@nbd.name>
 *
 * uses parts of afdisk
 * Copyright (C) 2002 by David Roetzel <david@roetzel.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdint.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le32(x) bswap_32(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le32(x) (x)
#else
#error unknown endianness!
#endif

/* Partition table entry */
struct pte {
	uint8_t active;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t start;
	uint32_t length;
};

struct partinfo {
	unsigned long size;
	int type;
};

int verbose = 0;
int active = 1;
int heads = -1;
int sectors = -1;
int kb_align = 0;
bool ignore_null_sized_partition = false;
struct partinfo parts[4];
char *filename = NULL;


/*
 * parse the size argument, which is either
 * a simple number (K assumed) or
 * K, M or G
 *
 * returns the size in KByte
 */
static long to_kbytes(const char *string)
{
	int exp = 0;
	long result;
	char *end;

	result = strtoul(string, &end, 0);
	switch (tolower(*end)) {
		case 'k' :
		case '\0' : exp = 0; break;
		case 'm' : exp = 1; break;
		case 'g' : exp = 2; break;
		default: return 0;
	}

	if (*end)
		end++;

	if (*end) {
		fprintf(stderr, "garbage after end of number\n");
		return 0;
	}

	/* result: number + 1024^(exp) */
	if (exp == 0)
		return result;
	return result * (2 << ((10 * exp) - 1));
}

/* convert the sector number into a CHS value for the partition table */
static void to_chs(long sect, unsigned char chs[3])
{
	int c,h,s;

	s = (sect % sectors) + 1;
	sect = sect / sectors;
	h = sect % heads;
	sect = sect / heads;
	c = sect;

	chs[0] = h;
	chs[1] = s | ((c >> 2) & 0xC0);
	chs[2] = c & 0xFF;

	return;
}

/* round the sector number up to the next cylinder */
static inline unsigned long round_to_cyl(long sect)
{
	int cyl_size = heads * sectors;

	return sect + cyl_size - (sect % cyl_size);
}

/* round the sector number up to the kb_align boundary */
static inline unsigned long round_to_kb(long sect) {
        return ((sect - 1) / kb_align + 1) * kb_align;
}

/* check the partition sizes and write the partition table */
static int gen_ptable(uint32_t signature, int nr)
{
	struct pte pte[4];
	unsigned long sect = 0;
	int i, fd, ret = -1, start, len;

	memset(pte, 0, sizeof(struct pte) * 4);
	for (i = 0; i < nr; i++) {
		if (!parts[i].size) {
			if (ignore_null_sized_partition)
				continue;
			fprintf(stderr, "Invalid size in partition %d!\n", i);
			return -1;
		}

		pte[i].active = ((i + 1) == active) ? 0x80 : 0;
		pte[i].type = parts[i].type;

		start = sect + sectors;
		if (kb_align != 0)
			start = round_to_kb(start);
		pte[i].start = cpu_to_le32(start);

		sect = start + parts[i].size * 2;
		if (kb_align == 0)
			sect = round_to_cyl(sect);
		pte[i].length = cpu_to_le32(len = sect - start);

		to_chs(start, pte[i].chs_start);
		to_chs(start + len - 1, pte[i].chs_end);

		if (verbose)
			fprintf(stderr, "Partition %d: start=%ld, end=%ld, size=%ld\n", i, (long)start * 512, ((long)start + (long)len) * 512, (long)len * 512);
		printf("%ld\n", (long)start * 512);
		printf("%ld\n", (long)len * 512);
	}

	if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "Can't open output file '%s'\n",filename);
		return -1;
	}

	lseek(fd, 440, SEEK_SET);
	if (write(fd, &signature, sizeof(signature)) != sizeof(signature)) {
		fprintf(stderr, "write failed.\n");
		goto fail;
	}

	lseek(fd, 446, SEEK_SET);
	if (write(fd, pte, sizeof(struct pte) * 4) != sizeof(struct pte) * 4) {
		fprintf(stderr, "write failed.\n");
		goto fail;
	}
	lseek(fd, 510, SEEK_SET);
	if (write(fd, "\x55\xaa", 2) != 2) {
		fprintf(stderr, "write failed.\n");
		goto fail;
	}

	ret = 0;
fail:
	close(fd);
	return ret;
}

static void usage(char *prog)
{
	fprintf(stderr, "Usage: %s [-v] [-n] -h <heads> -s <sectors> -o <outputfile> [-a 0..4] [-l <align kB>] [[-t <type>] -p <size>...] \n", prog);
	exit(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
	char type = 0x83;
	int ch;
	int part = 0;
	uint32_t signature = 0x5452574F; /* 'OWRT' */

	while ((ch = getopt(argc, argv, "h:s:p:a:t:o:vnl:S:")) != -1) {
		switch (ch) {
		case 'o':
			filename = optarg;
			break;
		case 'v':
			verbose++;
			break;
		case 'n':
			ignore_null_sized_partition = true;
			break;
		case 'h':
			heads = (int)strtoul(optarg, NULL, 0);
			break;
		case 's':
			sectors = (int)strtoul(optarg, NULL, 0);
			break;
		case 'p':
			if (part > 3) {
				fprintf(stderr, "Too many partitions\n");
				exit(EXIT_FAILURE);
			}
			parts[part].size = to_kbytes(optarg);
			parts[part++].type = type;
			break;
		case 't':
			type = (char)strtoul(optarg, NULL, 16);
			break;
		case 'a':
			active = (int)strtoul(optarg, NULL, 0);
			if ((active < 0) || (active > 4))
				active = 0;
			break;
		case 'l':
			kb_align = (int)strtoul(optarg, NULL, 0) * 2;
			break;
		case 'S':
			signature = strtoul(optarg, NULL, 0);
			break;
		case '?':
		default:
			usage(argv[0]);
		}
	}
	argc -= optind;
	if (argc || (heads <= 0) || (sectors <= 0) || !filename)
		usage(argv[0]);

	return gen_ptable(signature, part) ? EXIT_FAILURE : EXIT_SUCCESS;
}
