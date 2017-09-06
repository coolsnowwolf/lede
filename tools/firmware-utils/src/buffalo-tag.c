/*
 *  Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <netinet/in.h>

#include "buffalo-lib.h"

#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)

static char *region_table[] = {
	"JP", "US", "EU", "AP", "TW", "KR"
};

#define MAX_INPUT_FILES	2

static char *progname;
static char *ifname[MAX_INPUT_FILES];
static ssize_t fsize[MAX_INPUT_FILES];
static int num_files;
static char *ofname;
static char *product;
static char *brand;
static char *language;
static char *hwver;
static char *platform;
static int flag;
static char *major;
static char *minor = "1.01";
static int skipcrc;
static uint32_t base1;
static uint32_t base2;
static char *region_code;
static uint32_t region_mask;
static int num_regions;
static int dhp;

void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -a <platform>   set platform to <platform>\n"
"  -b <brand>      set brand to <brand>\n"
"  -c <base1>\n"
"  -d <base2>\n"
"  -f <flag>       set flag to <flag>\n"
"  -i <file>       read input from the file <file>\n"
"  -I <file>       read input from the file <file> for DHP series\n"
"  -l <language>   set language to <language>\n"
"  -m <version>    set minor version to <version>\n"
"  -o <file>       write output to the file <file>\n"
"  -p <product>    set product to <product>\n"
"  -r <region>     set image region to <region>\n"
"		   valid regions: JP, US, EU, AP, TW, KR, M_\n"
"  -s              skip CRC calculation\n"
"  -v <version>    set major version to <version>\n"
"  -w <version>    set harwdware version to <version>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

static int check_params(void)
{

#define CHECKSTR(_var, _name, _len)	do {		\
	if ((_var) == NULL) {				\
		ERR("no %s specified", (_name));	\
		return -1;				\
	}						\
	if ((_len) > 0 &&				\
	    strlen((_var)) > ((_len) - 1)) {		\
		ERR("%s is too long", (_name));		\
		return -1;				\
	}						\
} while (0)

	if (num_files == 0)
		ERR("no input files specified");

	CHECKSTR(ofname, "output file", 0);
	CHECKSTR(brand, "brand", TAG_BRAND_LEN);
	CHECKSTR(product, "product", TAG_PRODUCT_LEN);
	CHECKSTR(platform, "platform", TAG_PLATFORM_LEN);
	CHECKSTR(major, "major version", TAG_VERSION_LEN);
	CHECKSTR(minor, "minor version", TAG_VERSION_LEN);
	CHECKSTR(language, "language", TAG_LANGUAGE_LEN);

	if (hwver)
		CHECKSTR(hwver, "hardware version", 2);

	if (num_regions == 0) {
		ERR("no region code specified");
		return -1;
	}

	return 0;

#undef CHECKSTR
}

static int process_region(char *reg)
{
	int i;

	if (strlen(reg) != 2) {
		ERR("invalid region code '%s'", reg);
		return -1;
	}

	if (strcmp(reg, "M_") == 0) {
		region_code = reg;
		region_mask |= ~0;
		num_regions = 32;
		return 0;
	}

	for (i = 0; i < ARRAY_SIZE(region_table); i++)
		if (strcmp(reg, region_table[i]) == 0) {
			region_code = reg;
			region_mask |= 1 << i;
			num_regions++;
			return 0;
		}

	ERR("unknown region code '%s'", reg);
	return -1;
}

static int process_ifname(char *name)
{
	if (num_files >= ARRAY_SIZE(ifname)) {
		ERR("too many input files specified");
		return -1;
	}

	ifname[num_files++] = name;
	return 0;
}

static void fixup_tag(unsigned char *buf, ssize_t buflen)
{
	struct buffalo_tag *tag = (struct buffalo_tag *) buf;

	memset(tag, '\0', sizeof(*tag));

	memcpy(tag->brand, brand, strlen(brand));
	memcpy(tag->product, product, strlen(product));
	memcpy(tag->platform, platform, strlen(platform));
	memcpy(tag->ver_major, major, strlen(major));
	memcpy(tag->ver_minor, minor, strlen(minor));
	memcpy(tag->language, language, strlen(language));

	if (num_regions > 1) {
		tag->region_code[0] = 'M';
		tag->region_code[1] = '_';
		tag->region_mask = htonl(region_mask);
	} else {
		memcpy(tag->region_code, region_code, 2);
	}

	tag->len = htonl(buflen);
	tag->data_len = htonl(fsize[0]);
	tag->base1 = htonl(base1);
	tag->base2 = htonl(base2);
	tag->flag = flag;

	if (hwver) {
		memcpy(tag->hwv, "hwv", 3);
		memcpy(tag->hwv_val, hwver, strlen(hwver));
	}

	if (!skipcrc)
		tag->crc = htonl(buffalo_crc(buf, buflen));
}

static void fixup_tag2(unsigned char *buf, ssize_t buflen)
{
	struct buffalo_tag2 *tag = (struct buffalo_tag2 *) buf;

	memset(tag, '\0', sizeof(*tag));

	memcpy(tag->brand, brand, strlen(brand));
	memcpy(tag->product, product, strlen(product));
	memcpy(tag->platform, platform, strlen(platform));
	memcpy(tag->ver_major, major, strlen(major));
	memcpy(tag->ver_minor, minor, strlen(minor));
	memcpy(tag->language, language, strlen(language));

	if (num_regions > 1) {
		tag->region_code[0] = 'M';
		tag->region_code[1] = '_';
		tag->region_mask = htonl(region_mask);
	} else {
		memcpy(tag->region_code, region_code, 2);
	}

	tag->total_len = htonl(buflen);
	tag->len1 = htonl(fsize[0]);
	tag->len2 = htonl(fsize[1]);
	tag->flag = flag;

	if (hwver) {
		memcpy(tag->hwv, "hwv", 3);
		memcpy(tag->hwv_val, hwver, strlen(hwver));
	}

	if (!skipcrc)
		tag->crc = htonl(buffalo_crc(buf, buflen));
}

static void fixup_tag3(unsigned char *buf, ssize_t totlen)
{
	struct buffalo_tag3 *tag = (struct buffalo_tag3 *) buf;

	memset(tag, '\0', sizeof(*tag));

	memcpy(tag->brand, brand, strlen(brand));
	memcpy(tag->product, product, strlen(product));
	memcpy(tag->platform, platform, strlen(platform));
	memcpy(tag->ver_major, major, strlen(major));
	memcpy(tag->ver_minor, minor, strlen(minor));
	memcpy(tag->language, language, strlen(language));

	if (num_regions > 1) {
		tag->region_code[0] = 'M';
		tag->region_code[1] = '_';
		tag->region_mask = htonl(region_mask);
	} else {
		memcpy(tag->region_code, region_code, 2);
	}

	tag->total_len = htonl(totlen);
	tag->len1 = htonl(fsize[0]);
	tag->base2 = htonl(base2);

	if (hwver) {
		memcpy(tag->hwv, "hwv", 3);
		memcpy(tag->hwv_val, hwver, strlen(hwver));
	}
}

static int tag_file(void)
{
	unsigned char *buf;
	ssize_t offset;
	ssize_t hdrlen;
	ssize_t buflen;
	int err;
	int ret = -1;
	int i;

	if (dhp)
		hdrlen = sizeof(struct buffalo_tag3);
	else if (num_files == 1)
		hdrlen = sizeof(struct buffalo_tag);
	else
		hdrlen = sizeof(struct buffalo_tag2);

	buflen = hdrlen;

	for (i = 0; i < num_files; i++) {
		fsize[i] = get_file_size(ifname[i]);
		if (fsize[i] < 0) {
			ERR("unable to get size of '%s'", ifname[i]);
			goto out;
		}
		buflen += fsize[i];
	}

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	offset = hdrlen;
	for (i = 0; i < num_files; i++) {
		err = read_file_to_buf(ifname[i], buf + offset, fsize[i]);
		if (err) {
			ERR("unable to read from file '%s'", ifname[i]);
			goto free_buf;
		}

		offset += fsize[i];
	}

	if (dhp)
		fixup_tag3(buf, fsize[0] + 200);
	else if (num_files == 1)
		fixup_tag(buf, buflen);
	else
		fixup_tag2(buf, buflen);

	err = write_buf_to_file(ofname, buf, buflen);
	if (err) {
		ERR("unable to write to file '%s'", ofname);
		goto free_buf;
	}

	ret = 0;

free_buf:
	free(buf);
out:
	return ret;
}

int main(int argc, char *argv[])
{
	int res = EXIT_FAILURE;
	int err;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "a:b:c:d:f:hi:l:m:o:p:r:sv:w:I:");
		if (c == -1)
			break;

		switch (c) {
		case 'a':
			platform = optarg;
			break;
		case 'b':
			brand = optarg;
			break;
		case 'c':
			base1 = strtoul(optarg, NULL, 16);
			break;
		case 'd':
			base2 = strtoul(optarg, NULL, 16);
			break;
		case 'f':
			flag = strtoul(optarg, NULL, 2);
			break;
		case 'I':
			dhp = 1;
			/* FALLTHROUGH */
		case 'i':
			err = process_ifname(optarg);
			if (err)
				goto out;
			break;
		case 'l':
			language = optarg;
			break;
		case 'm':
			minor = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'p':
			product = optarg;
			break;
		case 'r':
			err = process_region(optarg);
			if (err)
				goto out;
			break;
		case 's':
			skipcrc = 1;
			break;
		case 'v':
			major = optarg;
			break;
		case 'w':
			hwver = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	err = check_params();
	if (err)
		goto out;

	err = tag_file();
	if (err)
		goto out;

	res = EXIT_SUCCESS;

out:
	return res;
}
