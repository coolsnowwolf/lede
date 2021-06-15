// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 Rafał Miłecki <rafal@milecki.pl>
 */

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le32(x)	bswap_32(x)
#define le32_to_cpu(x)	bswap_32(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)
#else
#error "Unsupported endianness"
#endif

struct bcm4908kernel_header {
	uint32_t boot_load_addr;	/* AKA la_address */
	uint32_t boot_addr;		/* AKA la_entrypt */
	uint32_t data_len;
	uint8_t magic[4];
	uint32_t uncomplen;		/* Empty for LZMA, used for LZ4 */
};

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("\t-i pathname\t\t\tinput kernel filepath\n");
	printf("\t-o pathname\t\t\toutput kernel filepath\n");
}

int main(int argc, char **argv) {
	struct bcm4908kernel_header header;
	uint8_t buf[1024];
	FILE *out = NULL;
	FILE *in = NULL;
	size_t length;
	size_t bytes;
	int err = 0;
	char c;

	if (argc >= 2 && !strcmp(argv[1], "-h")) {
		usage();
		return 0;
	}

	while ((c = getopt(argc, argv, "i:o:")) != -1) {
		switch (c) {
		case 'i':
			in = fopen(optarg, "r");
			break;
		case 'o':
			out = fopen(optarg, "w+");
			break;
		}
	}

	if (!in || !out) {
		fprintf(stderr, "Failed to open input and/or output file\n");
		usage();
		return -EINVAL;
	}

	if (fread(&header, 1, sizeof(header), in) != sizeof(header)) {
		fprintf(stderr, "Failed to read %zu bytes from input file\n", sizeof(header));
		err = -EIO;
		goto err_close;
	}

	if (!memcmp(header.magic, "BRCM", 4)) {
		fprintf(stderr, "Input file already contains BCM4908 kernel header\n");
		err = -EIO;
		goto err_close;
	}

	err = fseek(out, sizeof(header), SEEK_SET);
	if (err) {
		err = -errno;
		fprintf(stderr, "Failed to fseek(): %d\n", err);
		goto err_close;
	}

	length = 0;
	rewind(in);
	while ((bytes = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, bytes, out) != bytes) {
			fprintf(stderr, "Failed to write %zu B to the output file\n", bytes);
			err = -EIO;
			goto err_close;
		}
		length += bytes;
	}

	header.boot_load_addr = cpu_to_le32(0x00080000);
	header.boot_addr = cpu_to_le32(0x00080000);
	header.data_len = cpu_to_le32(length);
	header.magic[0] = 'B';
	header.magic[1] = 'R';
	header.magic[2] = 'C';
	header.magic[3] = 'M';
	header.uncomplen = 0;

	fseek(out, 0, SEEK_SET);

	if (fwrite(&header, 1, sizeof(header), out) != sizeof(header)) {
		fprintf(stderr, "Failed to write header to the output file\n");
		err = -EIO;
		goto err_close;
	}

err_close:
	fclose(out);
	fclose(in);
	return err;
}
