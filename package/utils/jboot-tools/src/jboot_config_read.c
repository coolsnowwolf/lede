/*
 * jboot_config_read
 *
 * Copyright (C) 2018 Paweł Dembicki <paweldembicki@gmail.com>
 *
 * This tool is based on mkdlinkfw.
 * Copyright (C) 2018 Paweł Dembicki <paweldembicki@gmail.com>
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008,2009 Wang Jian <lark@linux.net.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>		/* for unlink() */
#include <libgen.h>
#include <getopt.h>		/* for getopt() */
#include <stdarg.h>
#include <stdbool.h>
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>



#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ": %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define VERBOSE(fmt, ...) do { \
	if (verbose) { \
		fprintf(stdout, "[%s] " fmt "\n", progname, ## __VA_ARGS__); \
	} \
} while (0)

#define STAG_SIZE 16
#define STAG_MAGIC	0x2B24
#define STAG_ID		0x02

#define CSXF_SIZE 16
#define CSXF_MAGIC	0x5343

#define MAX_DATA_HEADER 128
#define DATA_HEADER_UNKNOWN 0x8000
#define DATA_HEADER_EEPROM 0xF5
#define DATA_HEADER_CONFIG 0x42
#define DATA_HEADER_SIZE 6

#define DATA_HEADER_ID_MAC 0x30
#define DATA_HEADER_ID_CAL 0x0

/* ARM update header 2.0
 * used only in factory images to erase and flash selected area
 */
struct stag_header {		/* used only of sch2 wrapped kernel data */
	uint8_t cmark;		/* in factory 0xFF ,in sysuograde must be the same as id */
	uint8_t id;		/* 0x04 */
	uint16_t magic;		/* magic 0x2B24 */
	uint32_t time_stamp;	/* timestamp calculated in jboot way */
	uint32_t image_length;	/* lentgh of kernel + sch2 header */
	uint16_t image_checksum;	/* negated jboot_checksum of sch2 + kernel */
	uint16_t tag_checksum;	/* negated jboot_checksum of stag header data */
};

struct csxf_header {
	uint16_t magic;		/* 0x5343, 'CS' in little endian */
	uint16_t checksum;	/* checksum, include header & body */
	uint32_t body_length;	/* length of body */
	uint8_t body_encoding;	/* encoding method of body */
	uint8_t reserved[3];
	uint32_t raw_length;	/* length of body before encoded */
};

struct data_header {
	uint8_t id;
	uint8_t type;		/* 0x42xx for config 0xF5xx for eeprom */
	uint16_t unknown;
	uint16_t length;	/* length of body */
	uint8_t data[];		/* encoding method of body */
};

/* globals */

char *ofname;
char *ifname;
char *progname;

uint8_t *buffer;
uint32_t config_size;

uint32_t start_offset;
uint8_t mac_duplicate;
uint8_t mac_print;
uint8_t print_data;
uint8_t verbose;

static void usage(int status)
{
	fprintf(stderr, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stderr,
		"\n"
		"Options:\n"
		"  -i <file>       config partition file <file>\n"
		"  -m              print mac address\n"
		"  -e <file>       save eeprom calibration data image to the file <file>\n"
		"  -o <offset>     set start offset to <ofset>\n"
		"  -p              print config data\n"
		"  -v              verbose\n"
		"  -h              show this screen\n");

	exit(status);
}

static void print_data_header(struct data_header *printed_header)
{
	printf("id: 0x%02X "
	       "type: 0x%02X "
	       "unknown: 0x%04X "
	       "length: 0x%04X\n"
	       "data: ",
	       printed_header->id,
	       printed_header->type,
	       printed_header->unknown, printed_header->length);

	for (uint16_t i = 0; i < printed_header->length; i++)
		printf("%02X ", printed_header->data[i]);

	printf("\n");

}

static uint16_t jboot_checksum(uint16_t start_val, uint16_t *data, int size)
{
	uint32_t counter = start_val;
	uint16_t *ptr = data;

	while (size > 1) {
		counter += *ptr;
		++ptr;
		while (counter >> 16)
			counter = (uint16_t) counter + (counter >> 16);
		size -= 2;
	}
	if (size > 0) {
		counter += *(uint8_t *) ptr;
		counter -= 0xFF;
	}
	while (counter >> 16)
		counter = (uint16_t) counter + (counter >> 16);
	return counter;
}

static int find_header(uint8_t *buf, uint32_t buf_size,
		       struct data_header **data_table)
{
	uint8_t *tmp_buf = buf + start_offset;
	uint8_t tmp_hdr[4] = { STAG_ID, STAG_ID, (STAG_MAGIC & 0xFF), (STAG_MAGIC >> 8) };
	struct csxf_header *tmp_csxf_header;
	uint16_t tmp_checksum = 0;
	uint16_t data_header_counter = 0;
	int ret = EXIT_FAILURE;

	VERBOSE("Looking for STAG header!");

	while ((uint32_t) tmp_buf - (uint32_t) buf <= buf_size) {
		if (!memcmp(tmp_buf, tmp_hdr, 4)) {
			if (((struct stag_header *)tmp_buf)->tag_checksum ==
			    (uint16_t) ~jboot_checksum(0, (uint16_t *) tmp_buf,
							STAG_SIZE - 2)) {
				VERBOSE("Found proper STAG header at: 0x%X.",
					tmp_buf - buf);
				break;
			}
		}
		tmp_buf++;
	}

	tmp_csxf_header = (struct csxf_header *)(tmp_buf + STAG_SIZE);
	if (tmp_csxf_header->magic != CSXF_MAGIC) {
		ERR("CSXF magic incorrect! 0x%X != 0x%X",
		    tmp_csxf_header->magic, CSXF_MAGIC);
		goto out;
	}
	VERBOSE("CSXF magic ok.");
	tmp_checksum = tmp_csxf_header->checksum;
	tmp_csxf_header->checksum = 0;

	tmp_csxf_header->checksum =
	    (uint16_t) ~jboot_checksum(0, (uint16_t *) (tmp_buf + STAG_SIZE),
					tmp_csxf_header->raw_length +
					CSXF_SIZE);

	if (tmp_checksum != tmp_csxf_header->checksum) {
		ERR("CSXF checksum incorrect! Stored: 0x%X Calculated: 0x%X",
		    tmp_checksum, tmp_csxf_header->checksum);
		goto out;
	}
	VERBOSE("CSXF image checksum ok.");

	tmp_buf = tmp_buf + STAG_SIZE + CSXF_SIZE;

	while ((uint32_t) tmp_buf - (uint32_t) buf <= buf_size) {

		struct data_header *tmp_data_header =
		    (struct data_header *)tmp_buf;

		if (tmp_data_header->unknown != DATA_HEADER_UNKNOWN) {
			tmp_buf++;
			continue;
		}
		if (tmp_data_header->type != DATA_HEADER_EEPROM
		    && tmp_data_header->type != DATA_HEADER_CONFIG) {
			tmp_buf++;
			continue;
		}

		data_table[data_header_counter] = tmp_data_header;
		tmp_buf +=
		    DATA_HEADER_SIZE + data_table[data_header_counter]->length;
		data_header_counter++;

	}

	ret = data_header_counter;

 out:
	return ret;
}

static int read_file(char *file_name)
{
	int ret = EXIT_FAILURE;
	uint32_t file_size = 0;
	FILE *fp;

	fp = fopen(file_name, "r");

	if (!fp) {
		ERR("Failed to open config input file %s", file_name);
		goto out;
	}

	fseek(fp, 0L, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0L, SEEK_SET);

	buffer = malloc(file_size);
	VERBOSE("Allocated %d bytes.", file_size);

	if (fread(buffer, 1, file_size, fp) != file_size) {
		ERR("Failed to read config input file %s", file_name);
		goto out_free_buf;
	}

	VERBOSE("Read %d bytes of config input file %s", file_size, file_name);
	config_size = file_size;
	ret = EXIT_SUCCESS;
	goto out;

 out_free_buf:
	free(buffer);
	fclose(fp);
 out:
	return ret;
}

static int write_file(const char *ofname, const uint8_t *data, int len)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(ofname, "w");
	if (f == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto out;
	}

	errno = 0;
	fwrite(data, len, 1, f);
	if (errno) {
		ERRS("unable to write output file");
		goto out_flush;
	}

	VERBOSE("firmware file \"%s\" completed", ofname);

	ret = EXIT_SUCCESS;

 out_flush:
	fflush(f);
	fclose(f);
	if (ret != EXIT_SUCCESS)
		unlink(ofname);
 out:
	return ret;
}

static void print_mac(struct data_header **data_table, int cnt)
{

	for (int i = 0; i < cnt; i++) {
		if (data_table[i]->type == DATA_HEADER_CONFIG
		    && data_table[i]->id == DATA_HEADER_ID_MAC) {
			int j;
			for (j = 0; j < 5; j++)
				printf("%02x:", data_table[i]->data[j]);
			printf("%02x\n", data_table[i]->data[j]);
		}

	}

}

static int write_eeprom(struct data_header **data_table, int cnt)
{
	int ret = EXIT_FAILURE;

	for (int i = 0; i < cnt; i++) {
		if (data_table[i]->type == DATA_HEADER_EEPROM
		    && data_table[i]->id == DATA_HEADER_ID_CAL) {
			ret =
			    write_file(ofname, data_table[i]->data,
				       data_table[i]->length);
			break;
		}

	}

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int configs_counter = 0;
	struct data_header *configs_table[MAX_DATA_HEADER];
	buffer = NULL;
	config_size = 0;

	progname = basename(argv[0]);
	start_offset = 0;
	mac_print = 0;
	print_data = 0;
	verbose = 0;
	ofname = NULL;
	ifname = NULL;

	while (1) {
		int c;

		c = getopt(argc, argv, "de:hi:mo:pv");
		if (c == -1)
			break;

		switch (c) {
		case 'm':
			mac_print = 1;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'e':
			ofname = optarg;
			break;
		case 'o':
			sscanf(optarg, "0x%x", &start_offset);
			break;
		case 'p':
			print_data = 1;
			break;
		case 'v':
			verbose = 1;
			VERBOSE("Enable verbose!");
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	if (!ifname)
		usage(EXIT_FAILURE);

	ret = read_file(ifname);

	if (ret || config_size <= 0)
		goto out;

	configs_counter = find_header(buffer, config_size, configs_table);

	if (configs_counter <= 0)
		goto out_free_buf;

	if (print_data || verbose) {
		for (int i = 0; i < configs_counter; i++)
			print_data_header(configs_table[i]);
	}

	if (mac_print)
		print_mac(configs_table, configs_counter);

	ret = EXIT_SUCCESS;

	if (ofname)
		ret = write_eeprom(configs_table, configs_counter);

 out_free_buf:
	free(buffer);
 out:
	return ret;

}
