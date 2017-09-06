/*
 * osafeloader
 *
 * Copyright (C) 2016 Rafał Miłecki <zajec5@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "md5.h"

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_be32(x)	(x)
#define be32_to_cpu(x)	(x)
#define cpu_to_be16(x)	(x)
#define be16_to_cpu(x)	(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be32(x)	bswap_32(x)
#define be32_to_cpu(x)	bswap_32(x)
#define cpu_to_be16(x)	bswap_16(x)
#define be16_to_cpu(x)	bswap_16(x)
#else
#error "Unsupported endianness"
#endif

struct safeloader_header {
	uint32_t imagesize;
	uint8_t md5[16];
} __attribute__ ((packed));

char *safeloader_path;
char *partition_name;
char *out_path;

static inline size_t osafeloader_min(size_t x, size_t y) {
	return x < y ? x : y;
}

static const uint8_t md5_salt[16] = {
	0x7a, 0x2b, 0x15, 0xed,
	0x9b, 0x98, 0x59, 0x6d,
	0xe5, 0x04, 0xab, 0x44,
	0xac, 0x2a, 0x9f, 0x4e,
};

/**************************************************
 * Info
 **************************************************/

static int osafeloader_info(int argc, char **argv) {
	FILE *safeloader;
	struct safeloader_header hdr;
	MD5_CTX ctx;
	size_t bytes, imagesize;
	uint8_t buf[1024];
	uint8_t md5[16];
	char name[32];
	int base, size, i;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No SafeLoader file passed\n");
		err = -EINVAL;
		goto out;
	}
	safeloader_path = argv[2];

	safeloader = fopen(safeloader_path, "r");
	if (!safeloader) {
		fprintf(stderr, "Couldn't open %s\n", safeloader_path);
		err = -EACCES;
		goto out;
	}

	bytes = fread(&hdr, 1, sizeof(hdr), safeloader);
	if (bytes != sizeof(hdr)) {
		fprintf(stderr, "Couldn't read %s header\n", safeloader_path);
		err =  -EIO;
		goto err_close;
	}
	imagesize = be32_to_cpu(hdr.imagesize);

	MD5_Init(&ctx);
	MD5_Update(&ctx, md5_salt, sizeof(md5_salt));
	while ((bytes = fread(buf, 1, osafeloader_min(sizeof(buf), imagesize), safeloader)) > 0) {
		MD5_Update(&ctx, buf, bytes);
		imagesize -= bytes;
	}
	MD5_Final(md5, &ctx);

	if (memcmp(md5, hdr.md5, 16)) {
		fprintf(stderr, "Broken SafeLoader file with invalid MD5\n");
		err =  -EIO;
		goto err_close;
	}

	printf("%10s: %d\n", "Image size", be32_to_cpu(hdr.imagesize));
	printf("%10s: ", "MD5");
	for (i = 0; i < 16; i++)
		printf("%02x", md5[i]);
	printf("\n");

	/* Skip header & vendor info */
	fseek(safeloader, 0x1014, SEEK_SET);

	while (fscanf(safeloader, "fwup-ptn %s base 0x%x size 0x%x\t\r\n", name, &base, &size) == 3) {
		printf("%10s: %s (0x%x - 0x%x)\n", "Partition", name, base, base + size);
	}

err_close:
	fclose(safeloader);
out:
	return err;
}

/**************************************************
 * Extract
 **************************************************/

static void osafeloader_extract_parse_options(int argc, char **argv) {
	int c;

	while ((c = getopt(argc, argv, "p:o:")) != -1) {
		switch (c) {
		case 'p':
			partition_name = optarg;
			break;
		case 'o':
			out_path = optarg;
			break;
		}
	}
}

static int osafeloader_extract(int argc, char **argv) {
	FILE *safeloader;
	FILE *out;
	struct safeloader_header hdr;
	size_t bytes;
	char name[32];
	int base, size;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No SafeLoader file passed\n");
		err = -EINVAL;
		goto out;
	}
	safeloader_path = argv[2];

	optind = 3;
	osafeloader_extract_parse_options(argc, argv);
	if (!partition_name) {
		fprintf(stderr, "No partition name specified\n");
		err = -EINVAL;
		goto out;
	} else if (!out_path) {
		fprintf(stderr, "No output file specified\n");
		err = -EINVAL;
		goto out;
	}

	safeloader = fopen(safeloader_path, "r");
	if (!safeloader) {
		fprintf(stderr, "Couldn't open %s\n", safeloader_path);
		err = -EACCES;
		goto out;
	}

	out = fopen(out_path, "w");
	if (!out) {
		fprintf(stderr, "Couldn't open %s\n", out_path);
		err = -EACCES;
		goto err_close_safeloader;
	}

	bytes = fread(&hdr, 1, sizeof(hdr), safeloader);
	if (bytes != sizeof(hdr)) {
		fprintf(stderr, "Couldn't read %s header\n", safeloader_path);
		err =  -EIO;
		goto err_close_out;
	}

	/* Skip vendor info */
	fseek(safeloader, 0x1000, SEEK_CUR);

	err = -ENOENT;
	while (fscanf(safeloader, "fwup-ptn %s base 0x%x size 0x%x\t\r\n", name, &base, &size) == 3) {
		uint8_t buf[1024];

		if (strcmp(name, partition_name))
			continue;

		err = 0;

		fseek(safeloader, sizeof(hdr) + 0x1000 + base, SEEK_SET);

		while ((bytes = fread(buf, 1, osafeloader_min(sizeof(buf), size), safeloader)) > 0) {
			if (fwrite(buf, 1, bytes, out) != bytes) {
				fprintf(stderr, "Couldn't write %zu B to %s\n", bytes, out_path);
				err = -EIO;
				break;
			}
			size -= bytes;
		}

		if (size) {
			fprintf(stderr, "Couldn't extract whole partition %s from %s (%d B left)\n", partition_name, safeloader_path, size);
			err = -EIO;
		}

		break;
	}

err_close_out:
	fclose(out);
err_close_safeloader:
	fclose(safeloader);
out:
	return err;
}

/**************************************************
 * Start
 **************************************************/

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("Info about SafeLoader:\n");
	printf("\tosafeloader info <file>\n");
	printf("\n");
	printf("Extract from SafeLoader:\n");
	printf("\tosafeloader extract <file> [options]\n");
	printf("\t-p name\t\t\t\tname of partition to extract\n");
	printf("\t-o file\t\t\t\toutput file\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		if (!strcmp(argv[1], "info"))
			return osafeloader_info(argc, argv);
		else if (!strcmp(argv[1], "extract"))
			return osafeloader_extract(argc, argv);
	}

	usage();
	return 0;
}
