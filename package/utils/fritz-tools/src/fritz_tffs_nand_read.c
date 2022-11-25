/*
 * A tool for reading the TFFS partitions (a name-value storage usually
 * found in AVM Fritz!Box based devices) on nand flash.
 *
 * Copyright (c) 2018 Valentin Spreckels <Valentin.Spreckels@Informatik.Uni-Oldenburg.DE>
 *
 * Based on the fritz_tffs_read tool:
 *     Copyright (c) 2015-2016 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 * and on the TFFS 2.0 kernel driver from AVM:
 *     Copyright (c) 2004-2007 AVM GmbH <fritzbox_info@avm.de>
 * and the TFFS 3.0 kernel driver from AVM:
 *     Copyright (C) 2004-2014 AVM GmbH <fritzbox_info@avm.de>
 * and the OpenWrt TFFS kernel driver:
 *     Copyright (c) 2013 John Crispin <john@phrozen.org>
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <mtd/mtd-user.h>
#include <assert.h>

#define DEFAULT_TFFS_SIZE	(256 * 1024)

#define TFFS_ID_END		0xffffffff
#define TFFS_ID_TABLE_NAME	0x000001ff

#define TFFS_BLOCK_HEADER_MAGIC	0x41564d5f54464653ULL
#define TFFS_VERSION		0x0003
#define TFFS_ENTRY_HEADER_SIZE	0x18
#define TFFS_MAXIMUM_SEGMENT_SIZE	(0x800 - TFFS_ENTRY_HEADER_SIZE)

#define TFFS_SECTOR_SIZE 0x0800
#define TFFS_SECTOR_OOB_SIZE 0x0040
#define TFFS_SECTORS_PER_PAGE 2

#define TFFS_SEGMENT_CLEARED 0xffffffff

static char *progname;
static char *mtddev;
static char *name_filter = NULL;
static bool show_all = false;
static bool print_all_key_names = false;
static bool read_oob_sector_health = false;
static bool swap_bytes = false;
static uint8_t readbuf[TFFS_SECTOR_SIZE];
static uint8_t oobbuf[TFFS_SECTOR_OOB_SIZE];
static uint32_t blocksize;
static int mtdfd;
struct tffs_sectors *sectors;

struct tffs_sectors {
	uint32_t num_sectors;
	uint8_t sectors[0];
};

static inline void sector_mark_bad(int num)
{
	sectors->sectors[num / 8] &= ~(0x80 >> (num % 8));
};

static inline uint8_t sector_get_good(int num)
{
	return sectors->sectors[num / 8] & 0x80 >> (num % 8);
};

struct tffs_entry_segment {
	uint32_t len;
	void *val;
};

struct tffs_entry {
	uint32_t len;
	void *val;
};

struct tffs_name_table_entry {
	uint32_t id;
	char *val;
};

struct tffs_key_name_table {
	uint32_t size;
	struct tffs_name_table_entry *entries;
};

static inline uint8_t read_uint8(void *buf, ptrdiff_t off)
{
	return *(uint8_t *)(buf + off);
}

static inline uint32_t read_uint32(void *buf, ptrdiff_t off)
{
	uint32_t tmp = *(uint32_t *)(buf + off);
	if (swap_bytes) {
		tmp = be32toh(tmp);
	}
	return tmp;
}

static inline uint64_t read_uint64(void *buf, ptrdiff_t off)
{
	uint64_t tmp = *(uint64_t *)(buf + off);
	if (swap_bytes) {
		tmp = be64toh(tmp);
	}
	return tmp;
}

static int read_sector(off_t pos)
{
	if (pread(mtdfd, readbuf, TFFS_SECTOR_SIZE, pos) != TFFS_SECTOR_SIZE) {
		return -1;
	}

	return 0;
}

static int read_sectoroob(off_t pos)
{
	struct mtd_oob_buf oob = {
		.start = pos,
		.length = TFFS_SECTOR_OOB_SIZE,
		.ptr = oobbuf
	};

	if (ioctl(mtdfd, MEMREADOOB, &oob) < 0)	{
		return -1;
	}

	return 0;
}

static inline uint32_t get_walk_size(uint32_t entry_len)
{
	return (entry_len + 3) & ~0x03;
}

static void print_entry_value(const struct tffs_entry *entry)
{
	/* These are NOT NULL terminated. */
	fwrite(entry->val, 1, entry->len, stdout);
}

static int find_entry(uint32_t id, struct tffs_entry *entry)
{
	uint32_t rev = 0;
	uint32_t num_segments = 0;
	struct tffs_entry_segment *segments = NULL;

	off_t pos = 0;
	uint8_t block_end = 0;
	for (uint32_t sector = 0; sector < sectors->num_sectors; sector++, pos += TFFS_SECTOR_SIZE) {
		if (block_end) {
			if (pos % blocksize == 0) {
				block_end = 0;
			}
		} else if (sector_get_good(sector)) {
			if (read_sectoroob(pos) || read_sector(pos)) {
				fprintf(stderr, "ERROR: sector isn't readable, but has been previously!\n");
				exit(EXIT_FAILURE);
			}
			uint32_t oob_id = read_uint32(oobbuf, 0x02);
			uint32_t oob_len = read_uint32(oobbuf, 0x06);
			uint32_t oob_rev = read_uint32(oobbuf, 0x0a);
			uint32_t read_id = read_uint32(readbuf, 0x00);
			uint32_t read_len = read_uint32(readbuf, 0x04);
			uint32_t read_rev = read_uint32(readbuf, 0x0c);
			if (read_oob_sector_health && (oob_id != read_id || oob_len != read_len || oob_rev != read_rev)) {
				fprintf(stderr, "Warning: sector has inconsistent metadata\n");
				continue;
			}
			if (read_id == TFFS_ID_END) {
				/* no more entries in this block */
				block_end = 1;
				continue;
			}
			if (read_len > TFFS_MAXIMUM_SEGMENT_SIZE) {
				fprintf(stderr, "Warning: segment is longer than possible\n");
				continue;
			}
			if (read_id == id) {
				if (read_rev < rev) {
					/* obsolete revision => ignore this */
					continue;
				}
				if (read_rev > rev) {
					/* newer revision => clear old data */
					for (uint32_t i = 0; i < num_segments; i++) {
						free(segments[i].val);
					}
					free (segments);
					rev = read_rev;
					num_segments = 0;
					segments = NULL;
				}

				uint32_t seg = read_uint32(readbuf, 0x10);

				if (seg == TFFS_SEGMENT_CLEARED) {
					continue;
				}

				uint32_t next_seg = read_uint32(readbuf, 0x14);

				uint32_t new_num_segs = next_seg == 0 ? seg + 1 : next_seg + 1;
				if (new_num_segs > num_segments) {
					segments = realloc(segments, new_num_segs * sizeof(struct tffs_entry_segment));
					memset(segments + (num_segments * sizeof(struct tffs_entry_segment)), 0x0,
							(new_num_segs - num_segments) * sizeof(struct tffs_entry_segment));
					num_segments = new_num_segs;
				}
				segments[seg].len = read_len;
				segments[seg].val = malloc(read_len);
				memcpy(segments[seg].val, readbuf + TFFS_ENTRY_HEADER_SIZE, read_len);
			}
		}
	}

	if (num_segments == 0) {
		return 0;
	}

	assert (segments != NULL);

	uint32_t len = 0;
	for (uint32_t i = 0; i < num_segments; i++) {
		if (segments[i].val == NULL) {
			/* missing segment */
			return 0;
		}

		len += segments[i].len;
	}

	void *p = malloc(len);
	entry->val = p;
	entry->len = len;
	for (uint32_t i = 0; i < num_segments; i++) {
		memcpy(p, segments[i].val, segments[i].len);
		p += segments[i].len;
	}

	return 1;
}

static void parse_key_names(struct tffs_entry *names_entry,
			     struct tffs_key_name_table *key_names)
{
	uint32_t pos = 0, i = 0;
	struct tffs_name_table_entry *name_item;

	key_names->entries = NULL;

	do {
		key_names->entries = realloc(key_names->entries,
			 sizeof(struct tffs_name_table_entry) * (i + 1));
		if (key_names->entries == NULL) {
			fprintf(stderr, "ERROR: memory allocation failed!\n");
			exit(EXIT_FAILURE);
		}
		name_item = &key_names->entries[i];

		name_item->id = read_uint32(names_entry->val, pos);
		pos += sizeof(uint32_t);
		name_item->val = strdup((const char *)(names_entry->val + pos));

		/*
		 * There is no "length" field because the string values are
		 * simply NULL-terminated -> strlen() gives us the size.
		 */
		pos += get_walk_size(strlen(name_item->val) + 1);

		++i;
	} while (pos < names_entry->len);

	key_names->size = i;
}

static void show_all_key_names(struct tffs_key_name_table *key_names)
{
	for (uint32_t i = 0; i < key_names->size; i++)
		printf("%s\n", key_names->entries[i].val);
}

static int show_all_key_value_pairs(struct tffs_key_name_table *key_names)
{
	uint8_t has_value = 0;
	struct tffs_entry tmp;

	for (uint32_t i = 0; i < key_names->size; i++) {
		if (find_entry(key_names->entries[i].id, &tmp)) {
			printf("%s=", (const char *)key_names->entries[i].val);
			print_entry_value(&tmp);
			printf("\n");
			has_value++;
			free(tmp.val);
		}
	}

	if (!has_value) {
		fprintf(stderr, "ERROR: no values found!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int show_matching_key_value(struct tffs_key_name_table *key_names)
{
	struct tffs_entry tmp;
	const char *name;

	for (uint32_t i = 0; i < key_names->size; i++) {
		name = key_names->entries[i].val;

		if (strcmp(name, name_filter) == 0) {
			if (find_entry(key_names->entries[i].id, &tmp)) {
				print_entry_value(&tmp);
				printf("\n");
				free(tmp.val);
				return EXIT_SUCCESS;
			} else {
				fprintf(stderr,
					"ERROR: no value found for name %s!\n",
					name);
				return EXIT_FAILURE;
			}
		}
	}

	fprintf(stderr, "ERROR: Unknown key name %s!\n", name_filter);
	return EXIT_FAILURE;
}

static int check_sector(off_t pos)
{
	if (!read_oob_sector_health) {
		return 1;
	}
	if (read_sectoroob(pos)) {
		return 0;
	}
	if (read_uint8(oobbuf, 0x00) != 0xff) {
		/* block is bad */
		return 0;
	}
	if (read_uint8(oobbuf, 0x01) != 0xff) {
		/* sector is bad */
		return 0;
	}
	return 1;
}

static int check_block(off_t pos, uint32_t sector)
{
	if (!check_sector(pos)) {
		return 0;
	}
	if (read_sector(pos)) {
		return 0;
	}
	if (read_uint64(readbuf, 0x00) != TFFS_BLOCK_HEADER_MAGIC) {
		fprintf(stderr, "Warning: block without magic header. Skipping block\n");
		return 0;
	}
	if (read_uint32(readbuf, 0x0c) != TFFS_SECTORS_PER_PAGE) {
		fprintf(stderr, "Warning: block with wrong number of sectors per page. Skipping block\n");
		return 0;
	}

	uint32_t num_hdr_bad = read_uint32(readbuf, 0x0c);
	for (uint32_t i = 0; i < num_hdr_bad; i++) {
		uint32_t bad = sector + read_uint64(readbuf, 0x1c + sizeof(uint64_t)*i);
		sector_mark_bad(bad);
	}

	return 1;
}

static int scan_mtd(void)
{
	struct mtd_info_user info;

	if (ioctl(mtdfd, MEMGETINFO, &info)) {
		return 0;
	}

	blocksize = info.erasesize;

	sectors = malloc(sizeof(*sectors) + (info.size / TFFS_SECTOR_SIZE + 7) / 8);
	if (sectors == NULL) {
		fprintf(stderr, "ERROR: memory allocation failed!\n");
		exit(EXIT_FAILURE);
	}
	sectors->num_sectors = info.size / TFFS_SECTOR_SIZE;
	memset(sectors->sectors, 0xff, (info.size / TFFS_SECTOR_SIZE + 7) / 8);

	uint32_t sector = 0, valid_blocks = 0;
	uint8_t block_ok = 0;
	for (off_t pos = 0; pos < info.size; sector++, pos += TFFS_SECTOR_SIZE) {
		if (pos % info.erasesize == 0) {
			block_ok = check_block(pos, sector);
			/* first sector of the block contains metadata
			   => handle it like a bad sector */
			sector_mark_bad(sector);
			if (block_ok) {
				valid_blocks++;
			}
		} else if (!block_ok || !sector_get_good(sector) || !check_sector(pos)) {
			sector_mark_bad(sector);
		}
	}

	return valid_blocks;
}

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
	"\n"
	"Options:\n"
	"  -a              list all key value pairs found in the TFFS file/device\n"
	"  -d <mtd>        inspect the TFFS on mtd device <mtd>\n"
	"  -h              show this screen\n"
	"  -l              list all supported keys\n"
	"  -n <key name>   display the value of the given key\n"
	"  -o              read OOB information about sector health\n"
	);

	exit(status);
}

static void parse_options(int argc, char *argv[])
{
	while (1) {
		int c;

		c = getopt(argc, argv, "abd:hln:o");
		if (c == -1)
			break;

		switch (c) {
		case 'a':
			show_all = true;
			name_filter = NULL;
			print_all_key_names = false;
			break;
		case 'b':
			swap_bytes = 1;
			break;
		case 'd':
			mtddev = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'l':
			print_all_key_names = true;
			show_all = false;
			name_filter = NULL;
			break;
		case 'n':
			name_filter = optarg;
			show_all = false;
			print_all_key_names = false;
			break;
		case 'o':
			read_oob_sector_health = true;
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	if (!mtddev) {
		fprintf(stderr, "ERROR: No input file (-d <file>) given!\n");
		usage(EXIT_FAILURE);
	}

	if (!show_all && !name_filter && !print_all_key_names) {
		fprintf(stderr,
			"ERROR: either -l, -a or -n <key name> is required!\n");
		usage(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	struct tffs_entry name_table;
	struct tffs_key_name_table key_names;

	progname = basename(argv[0]);

	parse_options(argc, argv);

	mtdfd = open(mtddev, O_RDONLY);
	if (mtdfd < 0) {
		fprintf(stderr, "ERROR: Failed to open tffs device %s\n",
			mtddev);
		goto out;
	}

	if (!scan_mtd()) {
		fprintf(stderr, "ERROR: Parsing blocks from tffs device %s failed\n", mtddev);
		fprintf(stderr, "       Is byte-swapping (-b) required?\n");
		goto out_close;
	}

	if (!find_entry(TFFS_ID_TABLE_NAME, &name_table)) {
		fprintf(stderr, "ERROR: No name table found on tffs device %s\n",
			mtddev);
		goto out_free_sectors;
	}

	parse_key_names(&name_table, &key_names);
	if (key_names.size < 1) {
		fprintf(stderr, "ERROR: No name table found on tffs device %s\n",
			mtddev);
		goto out_free_entry;
	}

	if (print_all_key_names) {
		show_all_key_names(&key_names);
		ret = EXIT_SUCCESS;
	} else if (show_all) {
		ret = show_all_key_value_pairs(&key_names);
	} else {
		ret = show_matching_key_value(&key_names);
	}

	free(key_names.entries);
out_free_entry:
	free(name_table.val);
out_free_sectors:
	free(sectors);
out_close:
	close(mtdfd);
out:
	return ret;
}
