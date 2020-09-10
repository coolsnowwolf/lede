/*
 * ptgen - partition table generator
 * Copyright (C) 2006 by Felix Fietkau <nbd@nbd.name>
 *
 * uses parts of afdisk
 * Copyright (C) 2002 by David Roetzel <david@roetzel.de>
 *
 * UUID/GUID definition stolen from kernel/include/uapi/linux/uuid.h
 * Copyright (C) 2010, Intel Corp. Huang Ying <ying.huang@intel.com>
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
#include <inttypes.h>
#include <fcntl.h>
#include <stdint.h>
#include "cyg_crc.h"

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le16(x) bswap_16(x)
#define cpu_to_le32(x) bswap_32(x)
#define cpu_to_le64(x) bswap_64(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#else
#error unknown endianness!
#endif

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

typedef struct {
	uint8_t b[16];
} guid_t;

#define GUID_INIT(a, b, c, d0, d1, d2, d3, d4, d5, d6, d7)			\
((guid_t)								\
{{ (a) & 0xff, ((a) >> 8) & 0xff, ((a) >> 16) & 0xff, ((a) >> 24) & 0xff, \
   (b) & 0xff, ((b) >> 8) & 0xff,					\
   (c) & 0xff, ((c) >> 8) & 0xff,					\
   (d0), (d1), (d2), (d3), (d4), (d5), (d6), (d7) }})

#define GUID_STRING_LENGTH      36

#define GPT_SIGNATURE 0x5452415020494645ULL
#define GPT_REVISION 0x00010000

#define GUID_PARTITION_SYSTEM \
	GUID_INIT( 0xC12A7328, 0xF81F, 0x11d2, \
			0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B)

#define GUID_PARTITION_BASIC_DATA \
	GUID_INIT( 0xEBD0A0A2, 0xB9E5, 0x4433, \
			0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7)

#define GUID_PARTITION_BIOS_BOOT \
	GUID_INIT( 0x21686148, 0x6449, 0x6E6F, \
			0x74, 0x4E, 0x65, 0x65, 0x64, 0x45, 0x46, 0x49)

#define GPT_HEADER_SIZE         92
#define GPT_ENTRY_SIZE          128
#define GPT_ENTRY_MAX           128
#define GPT_ENTRY_NAME_SIZE     72

#define GPT_HEADER_SECTOR       1
#define GPT_FIRST_ENTRY_SECTOR  2

#define MBR_ENTRY_MAX           4
#define MBR_DISK_SIGNATURE_OFFSET  440
#define MBR_PARTITION_ENTRY_OFFSET 446
#define MBR_BOOT_SIGNATURE_OFFSET  510

#define DISK_SECTOR_SIZE        512

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
	unsigned long start;
	unsigned long size;
	int type;
};

/* GPT Partition table header */
struct gpth {
	uint64_t signature;
	uint32_t revision;
	uint32_t size;
	uint32_t crc32;
	uint32_t reserved;
	uint64_t self;
	uint64_t alternate;
	uint64_t first_usable;
	uint64_t last_usable;
	guid_t disk_guid;
	uint64_t first_entry;
	uint32_t entry_num;
	uint32_t entry_size;
	uint32_t entry_crc32;
} __attribute__((packed));

/* GPT Partition table entry */
struct gpte {
	guid_t type;
	guid_t guid;
	uint64_t start;
	uint64_t end;
	uint64_t attr;
	char name[GPT_ENTRY_NAME_SIZE];
} __attribute__((packed));


int verbose = 0;
int active = 1;
int heads = -1;
int sectors = -1;
int kb_align = 0;
bool ignore_null_sized_partition = false;
bool use_guid_partition_table = false;
struct partinfo parts[GPT_ENTRY_MAX];
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
		fputs("garbage after end of number\n", stderr);
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

/* Compute a CRC for guid partition table */
static inline unsigned long gpt_crc32(void *buf, unsigned long len)
{
	return cyg_crc32_accumulate(~0L, buf, len) ^ ~0L;
}

/* Parse a guid string to guid_t struct */
static inline int guid_parse(char *buf, guid_t *guid)
{
	char b[4] = {0};
	char *p = buf;
	unsigned i = 0;
	if (strnlen(buf, GUID_STRING_LENGTH) != GUID_STRING_LENGTH)
		return -1;
	for (i = 0; i < sizeof(guid_t); i++) {
		if (*p == '-')
			p++;
		if (*p == '\0')
			return -1;
		memcpy(b, p, 2);
		guid->b[i] = strtol(b, 0, 16);
		p += 2;
	}
	swap(guid->b[0], guid->b[3]);
	swap(guid->b[1], guid->b[2]);
	swap(guid->b[4], guid->b[5]);
	swap(guid->b[6], guid->b[7]);
	return 0;
}

/* init an utf-16 string from utf-8 string */
static inline void init_utf16(char *str, uint16_t *buf, unsigned bufsize)
{
	unsigned i, n = 0;
	for (i = 0; i < bufsize; i++) {
		if (str[n] == 0x00) {
			buf[i] = 0x00;
			return ;
		} else if ((str[n] & 0x80) == 0x00) {//0xxxxxxx
			buf[i] = cpu_to_le16(str[n++]);
		} else if ((str[n] & 0xE0) == 0xC0) {//110xxxxx
			buf[i] = cpu_to_le16((str[n] & 0x1F) << 6 | (str[n + 1] & 0x3F));
			n += 2;
		} else if ((str[n] & 0xF0) == 0xE0) {//1110xxxx
			buf[i] = cpu_to_le16((str[n] & 0x0F) << 12 | (str[n + 1] & 0x3F) << 6 | (str[n + 2] & 0x3F));
			n += 3;
		} else {
			buf[i] = cpu_to_le16('?');
			n++;
		}
	}
}

/* check the partition sizes and write the partition table */
static int gen_ptable(uint32_t signature, int nr)
{
	struct pte pte[MBR_ENTRY_MAX];
	unsigned long start, len, sect = 0;
	int i, fd, ret = -1;

	memset(pte, 0, sizeof(struct pte) * MBR_ENTRY_MAX);
	for (i = 0; i < nr; i++) {
		if (!parts[i].size) {
			if (ignore_null_sized_partition)
				continue;
			fprintf(stderr, "Invalid size in partition %d!\n", i);
			return ret;
		}

		pte[i].active = ((i + 1) == active) ? 0x80 : 0;
		pte[i].type = parts[i].type;

		start = sect + sectors;
		if (parts[i].start != 0) {
			if (parts[i].start * 2 < start) {
				fprintf(stderr, "Invalid start %ld for partition %d!\n",
					parts[i].start, i);
				return ret;
			}
			start = parts[i].start * 2;
		} else if (kb_align != 0) {
			start = round_to_kb(start);
		}
		pte[i].start = cpu_to_le32(start);

		sect = start + parts[i].size * 2;
		if (kb_align == 0)
			sect = round_to_cyl(sect);
		pte[i].length = cpu_to_le32(len = sect - start);

		to_chs(start, pte[i].chs_start);
		to_chs(start + len - 1, pte[i].chs_end);

		if (verbose)
			fprintf(stderr, "Partition %d: start=%ld, end=%ld, size=%ld\n",
					i,
					(long)start * DISK_SECTOR_SIZE,
					(long)(start + len) * DISK_SECTOR_SIZE,
					(long)len * DISK_SECTOR_SIZE);
		printf("%ld\n", (long)start * DISK_SECTOR_SIZE);
		printf("%ld\n", (long)len * DISK_SECTOR_SIZE);
	}

	if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "Can't open output file '%s'\n",filename);
		return ret;
	}

	lseek(fd, MBR_DISK_SIGNATURE_OFFSET, SEEK_SET);
	if (write(fd, &signature, sizeof(signature)) != sizeof(signature)) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	lseek(fd, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
	if (write(fd, pte, sizeof(struct pte) * MBR_ENTRY_MAX) != sizeof(struct pte) * MBR_ENTRY_MAX) {
		fputs("write failed.\n", stderr);
		goto fail;
	}
	lseek(fd, MBR_BOOT_SIGNATURE_OFFSET, SEEK_SET);
	if (write(fd, "\x55\xaa", 2) != 2) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	ret = 0;
fail:
	close(fd);
	return ret;
}

/* check the partition sizes and write the guid partition table */
static int gen_gptable(uint32_t signature, guid_t guid, unsigned nr)
{
	struct pte pte;
	struct gpth gpth = {
		.signature = cpu_to_le64(GPT_SIGNATURE),
		.revision = cpu_to_le32(GPT_REVISION),
		.size = cpu_to_le32(GPT_HEADER_SIZE),
		.self = cpu_to_le64(GPT_HEADER_SECTOR),
		.first_usable = cpu_to_le64(GPT_FIRST_ENTRY_SECTOR + GPT_ENTRY_SIZE * GPT_ENTRY_MAX / DISK_SECTOR_SIZE),
		.first_entry = cpu_to_le64(GPT_FIRST_ENTRY_SECTOR),
		.disk_guid = guid,
		.entry_num = cpu_to_le32(GPT_ENTRY_MAX),
		.entry_size = cpu_to_le32(GPT_ENTRY_SIZE),
	};
	struct gpte  gpte[GPT_ENTRY_MAX];
	uint64_t start, end, sect = 0;
	int fd, ret = -1;
	unsigned i;

	memset(gpte, 0, GPT_ENTRY_SIZE * GPT_ENTRY_MAX);
	for (i = 0; i < nr; i++) {
		if (!parts[i].size) {
			if (ignore_null_sized_partition)
				continue;
			fprintf(stderr, "Invalid size in partition %d!\n", i);
			return ret;
		}
		start = sect + sectors;
		if (parts[i].start != 0) {
			if (parts[i].start * 2 < start) {
				fprintf(stderr, "Invalid start %ld for partition %d!\n",
					parts[i].start, i);
				return ret;
			}
			start = parts[i].start * 2;
		} else if (kb_align != 0) {
			start = round_to_kb(start);
		}
		gpte[i].start = cpu_to_le64(start);

		sect = start + parts[i].size * 2;
		if (kb_align == 0)
			sect = round_to_cyl(sect);
		gpte[i].end = cpu_to_le64(sect -1);
		gpte[i].guid = guid;
		gpte[i].guid.b[sizeof(guid_t) -1] += i + 1;
		if (parts[i].type == 0xEF || (i + 1) == (unsigned)active) {
			gpte[i].type = GUID_PARTITION_SYSTEM;
			init_utf16("EFI System Partition", (uint16_t *)gpte[i].name, GPT_ENTRY_NAME_SIZE / sizeof(uint16_t));
		} else {
			gpte[i].type = GUID_PARTITION_BASIC_DATA;
		}

		if (verbose)
			fprintf(stderr, "Partition %d: start=%" PRIu64 ", end=%" PRIu64 ", size=%"  PRIu64 "\n",
					i,
					start * DISK_SECTOR_SIZE, sect * DISK_SECTOR_SIZE,
					(sect - start) * DISK_SECTOR_SIZE);
		printf("%" PRIu64 "\n", start * DISK_SECTOR_SIZE);
		printf("%" PRIu64 "\n", (sect - start) * DISK_SECTOR_SIZE);
	}

	gpte[GPT_ENTRY_MAX - 1].start = cpu_to_le64(GPT_FIRST_ENTRY_SECTOR + GPT_ENTRY_SIZE * GPT_ENTRY_MAX / DISK_SECTOR_SIZE);
	gpte[GPT_ENTRY_MAX - 1].end = cpu_to_le64((kb_align ? round_to_kb(sectors) : (unsigned long)sectors) - 1);
	gpte[GPT_ENTRY_MAX - 1].type = GUID_PARTITION_BIOS_BOOT;
	gpte[GPT_ENTRY_MAX - 1].guid = guid;
	gpte[GPT_ENTRY_MAX - 1].guid.b[sizeof(guid_t) -1] += GPT_ENTRY_MAX;

	end = sect + sectors - 1;

	pte.type = 0xEE;
	pte.start = cpu_to_le32(GPT_HEADER_SECTOR);
	pte.length = cpu_to_le32(end);
	to_chs(GPT_HEADER_SECTOR, pte.chs_start);
	to_chs(end, pte.chs_end);

	gpth.last_usable = cpu_to_le64(end - GPT_ENTRY_SIZE * GPT_ENTRY_MAX / DISK_SECTOR_SIZE - 1);
	gpth.alternate = cpu_to_le64(end);
	gpth.entry_crc32 = cpu_to_le32(gpt_crc32(gpte, GPT_ENTRY_SIZE * GPT_ENTRY_MAX));
	gpth.crc32 = cpu_to_le32(gpt_crc32((char *)&gpth, GPT_HEADER_SIZE));

	if ((fd = open(filename, O_WRONLY|O_CREAT|O_TRUNC, 0644)) < 0) {
		fprintf(stderr, "Can't open output file '%s'\n",filename);
		return ret;
	}

	lseek(fd, MBR_DISK_SIGNATURE_OFFSET, SEEK_SET);
	if (write(fd, &signature, sizeof(signature)) != sizeof(signature)) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	lseek(fd, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
	if (write(fd, &pte, sizeof(struct pte)) != sizeof(struct pte)) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	lseek(fd, MBR_BOOT_SIGNATURE_OFFSET, SEEK_SET);
	if (write(fd, "\x55\xaa", 2) != 2) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	if (write(fd, &gpth, GPT_HEADER_SIZE) != GPT_HEADER_SIZE) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	lseek(fd, GPT_FIRST_ENTRY_SECTOR * DISK_SECTOR_SIZE, SEEK_SET);
	if (write(fd, &gpte, GPT_ENTRY_SIZE * GPT_ENTRY_MAX) != GPT_ENTRY_SIZE * GPT_ENTRY_MAX) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

#ifdef WANT_ALTERNATE_PTABLE
	/* The alternate partition table (We omit it by default) */
	swap(gpth.self, gpth.alternate);
	gpth.first_entry = cpu_to_le64(end - GPT_ENTRY_SIZE * GPT_ENTRY_MAX / DISK_SECTOR_SIZE),
	gpth.crc32 = 0;
	gpth.crc32 = cpu_to_le32(gpt_crc32(&gpth, GPT_HEADER_SIZE));

	lseek(fd, end * DISK_SECTOR_SIZE - GPT_ENTRY_SIZE * GPT_ENTRY_MAX, SEEK_SET);
	if (write(fd, &gpte, GPT_ENTRY_SIZE * GPT_ENTRY_MAX) != GPT_ENTRY_SIZE * GPT_ENTRY_MAX) {
		fputs("write failed.\n", stderr);
		goto fail;
	}

	lseek(fd, end * DISK_SECTOR_SIZE, SEEK_SET);
	if (write(fd, &gpth, GPT_HEADER_SIZE) != GPT_HEADER_SIZE) {
		fputs("write failed.\n", stderr);
		goto fail;
	}
	lseek(fd, (end + 1) * DISK_SECTOR_SIZE -1, SEEK_SET);
	if (write(fd, "\x00", 1) != 1) {
		fputs("write failed.\n", stderr);
		goto fail;
	}
#endif

	ret = 0;
fail:
	close(fd);
	return ret;
}

static void usage(char *prog)
{
	fprintf(stderr, "Usage: %s [-v] [-n] [-g] -h <heads> -s <sectors> -o <outputfile> [-a 0..4] [-l <align kB>] [-G <guid>] [[-t <type>] -p <size>[@<start>]...] \n", prog);
	exit(EXIT_FAILURE);
}

int main (int argc, char **argv)
{
	unsigned char type = 0x83;
	char *p;
	int ch;
	int part = 0;
	uint32_t signature = 0x5452574F; /* 'OWRT' */
	guid_t guid = GUID_INIT( signature, 0x2211, 0x4433, \
			0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0x00);

	while ((ch = getopt(argc, argv, "h:s:p:a:t:o:vngl:S:G:")) != -1) {
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
		case 'g':
			use_guid_partition_table = 1;
			break;
		case 'h':
			heads = (int)strtoul(optarg, NULL, 0);
			break;
		case 's':
			sectors = (int)strtoul(optarg, NULL, 0);
			break;
		case 'p':
			if (part > GPT_ENTRY_MAX - 1 || (!use_guid_partition_table && part > 3)) {
				fputs("Too many partitions\n", stderr);
				exit(EXIT_FAILURE);
			}
			p = strchr(optarg, '@');
			if (p) {
				*(p++) = 0;
				parts[part].start = to_kbytes(p);
			}
			parts[part].size = to_kbytes(optarg);
			fprintf(stderr, "part %ld %ld\n", parts[part].start, parts[part].size);
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
		case 'G':
			if (guid_parse(optarg, &guid)) {
				fputs("Invalid guid string\n", stderr);
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
		default:
			usage(argv[0]);
		}
	}
	argc -= optind;
	if (argc || (heads <= 0) || (sectors <= 0) || !filename)
		usage(argv[0]);

	if (use_guid_partition_table)
		return gen_gptable(signature, guid, part) ? EXIT_FAILURE : EXIT_SUCCESS;

	return gen_ptable(signature, part) ? EXIT_FAILURE : EXIT_SUCCESS;
}
