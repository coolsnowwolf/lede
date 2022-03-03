/*
 * A tool for reading the TFFS partitions (a name-value storage usually
 * found in AVM Fritz!Box based devices).
 *
 * Copyright (c) 2015-2016 Martin Blumenstingl <martin.blumenstingl@googlemail.com>
 *
 * Based on the TFFS 2.0 kernel driver from AVM:
 *     Copyright (c) 2004-2007 AVM GmbH <fritzbox_info@avm.de>
 * and the OpenWrt TFFS kernel driver:
 *     Copyright (c) 2013 John Crispin <blogic@openwrt.org>
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define TFFS_ID_END		0xffff
#define TFFS_ID_TABLE_NAME	0x01ff

static char *progname;
static char *input_file;
static unsigned long tffs_size;
static char *name_filter = NULL;
static bool show_all = false;
static bool print_all_key_names = false;
static bool swap_bytes = false;

struct tffs_entry_header {
	uint16_t id;
	uint16_t len;
};

struct tffs_entry {
	const struct tffs_entry_header *header;
	char *name;
	uint8_t *val;
};

struct tffs_name_table_entry {
	const uint32_t *id;
	const char *val;
};

struct tffs_key_name_table {
	uint32_t size;
	struct tffs_name_table_entry *entries;
};

static inline uint16_t get_header_len(const struct tffs_entry_header *header)
{
	if (swap_bytes)
		return ntohs(header->len);

	return header->len;
}

static inline uint16_t get_header_id(const struct tffs_entry_header *header)
{
	if (swap_bytes)
		return ntohs(header->id);

	return header->id;
}

static inline uint16_t to_entry_header_id(uint32_t name_id)
{
	if (swap_bytes)
		return ntohl(name_id) & 0xffff;

	return name_id & 0xffff;
}

static inline uint32_t get_walk_size(uint32_t entry_len)
{
	return (entry_len + 3) & ~0x03;
}

static void print_entry_value(const struct tffs_entry *entry)
{
	int i;

	/* These are NOT NULL terminated. */
	for (i = 0; i < get_header_len(entry->header); i++)
		fprintf(stdout, "%c", entry->val[i]);
}

static void parse_entry(uint8_t *buffer, uint32_t pos,
			struct tffs_entry *entry)
{
	entry->header = (struct tffs_entry_header *) &buffer[pos];
	entry->val = &buffer[pos + sizeof(struct tffs_entry_header)];
}

static int find_entry(uint8_t *buffer, uint16_t id, struct tffs_entry *entry)
{
	uint32_t pos = 0;

	do {
		parse_entry(buffer, pos, entry);

		if (get_header_id(entry->header) == id)
			return 1;

		pos += sizeof(struct tffs_entry_header);
		pos += get_walk_size(get_header_len(entry->header));
	} while (pos < tffs_size && entry->header->id != TFFS_ID_END);

	return 0;
}

static void parse_key_names(struct tffs_entry *names_entry,
			    struct tffs_key_name_table *key_names)
{
	uint32_t pos = 0, i = 0;
	struct tffs_name_table_entry *name_item;

	key_names->entries = calloc(sizeof(*name_item), 1);

	do {
		name_item = &key_names->entries[i];

		name_item->id = (uint32_t *) &names_entry->val[pos];
		pos += sizeof(*name_item->id);
		name_item->val = (const char *) &names_entry->val[pos];

		/*
		 * There is no "length" field because the string values are
		 * simply NULL-terminated -> strlen() gives us the size.
		 */
		pos += get_walk_size(strlen(name_item->val) + 1);

		++i;
		key_names->entries = realloc(key_names->entries,
						sizeof(*name_item) * (i + 1));
	} while (pos < get_header_len(names_entry->header));

	key_names->size = i;
}

static void show_all_key_names(struct tffs_key_name_table *key_names)
{
	int i;

	for (i = 0; i < key_names->size; i++)
		printf("%s\n", key_names->entries[i].val);
}

static int show_all_key_value_pairs(uint8_t *buffer,
				    struct tffs_key_name_table *key_names)
{
	int i, has_value = 0;
	uint16_t id;
	struct tffs_entry tmp;

	for (i = 0; i < key_names->size; i++) {
		id = to_entry_header_id(*key_names->entries[i].id);

		if (find_entry(buffer, id, &tmp)) {
			printf("%s=", key_names->entries[i].val);
			print_entry_value(&tmp);
			printf("\n");
			has_value++;
		}
	}

	if (!has_value) {
		fprintf(stderr, "ERROR: no values found!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

static int show_matching_key_value(uint8_t *buffer,
				   struct tffs_key_name_table *key_names)
{
	int i;
	uint16_t id;
	struct tffs_entry tmp;
	const char *name;

	for (i = 0; i < key_names->size; i++) {
		name = key_names->entries[i].val;

		if (strncmp(name, name_filter, strlen(name)) == 0) {
			id = to_entry_header_id(*key_names->entries[i].id);

			if (find_entry(buffer, id, &tmp)) {
				print_entry_value(&tmp);
				printf("\n");
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

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
	"\n"
	"Options:\n"
	"  -a              list all key value pairs found in the TFFS file/device\n"
	"  -b              swap bytes while parsing the TFFS file/device\n"
	"  -h              show this screen\n"
	"  -i <file>       inspect the given TFFS file/device <file>\n"
	"  -l              list all supported keys\n"
	"  -n <key name>   display the value of the given key\n"
	"  -s <size>       the (max) size of the TFFS file/device <size>\n"
	);

	exit(status);
}

static int file_exist(char *filename)
{
	struct stat buffer;

	return stat(filename, &buffer) == 0;
}

static void parse_options(int argc, char *argv[])
{
	while (1)
	{
		int c;

		c = getopt(argc, argv, "abhi:ln:s:");
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
			case 'h':
				usage(EXIT_SUCCESS);
				break;
			case 'i':
				input_file = optarg;
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
			case 's':
				tffs_size = strtoul(optarg, NULL, 0);
				break;
			default:
				usage(EXIT_FAILURE);
				break;
		}
	}

	if (!input_file) {
		fprintf(stderr, "ERROR: No input file (-i <file>) given!\n");
		exit(EXIT_FAILURE);
	}

	if (!file_exist(input_file)) {
		fprintf(stderr, "ERROR: %s does not exist\n", input_file);
		exit(EXIT_FAILURE);
	}

	if (!show_all && !name_filter && !print_all_key_names) {
		fprintf(stderr,
			"ERROR: either -l, -a or -n <key name> is required!\n");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	uint8_t *buffer;
	FILE *fp;
	struct tffs_entry name_table;
	struct tffs_key_name_table key_names;

	progname = basename(argv[0]);

	parse_options(argc, argv);

	fp = fopen(input_file, "r");

	if (!fp) {
		fprintf(stderr, "ERROR: Failed to open tffs input file %s\n",
			input_file);
		goto out;
	}

	if (tffs_size == 0) {
		fseek(fp, 0L, SEEK_END);
		tffs_size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
	}

	buffer = malloc(tffs_size);

	if (fread(buffer, 1, tffs_size, fp) != tffs_size) {
		fprintf(stderr, "ERROR: Failed read tffs file %s\n",
			input_file);
		goto out_free;
	}

	if (!find_entry(buffer, TFFS_ID_TABLE_NAME, &name_table)) {
		fprintf(stderr,"ERROR: No name table found in tffs file %s\n",
			input_file);
		fprintf(stderr,"       Is byte-swapping (-b) required?\n");
		goto out_free;
	}

	parse_key_names(&name_table, &key_names);
	if (key_names.size < 1) {
		fprintf(stderr, "ERROR: No name table found in tffs file %s\n",
			input_file);
		goto out_free_names;
	}

	if (print_all_key_names) {
		show_all_key_names(&key_names);
		ret = EXIT_SUCCESS;
	} else if (show_all) {
		ret = show_all_key_value_pairs(buffer, &key_names);
	} else {
		ret = show_matching_key_value(buffer, &key_names);
	}

out_free_names:
	free(key_names.entries);
out_free:
	fclose(fp);
	free(buffer);
out:
	return ret;
}
