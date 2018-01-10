/*
 * otrx
 *
 * Copyright (C) 2015 Rafał Miłecki <zajec5@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
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

#define TRX_MAGIC			0x30524448
#define TRX_FLAGS_OFFSET		12
#define TRX_MAX_PARTS			3

struct trx_header {
	uint32_t magic;
	uint32_t length;
	uint32_t crc32;
	uint16_t flags;
	uint16_t version;
	uint32_t offset[3];
};

char *trx_path;
size_t trx_offset = 0;
char *partition[TRX_MAX_PARTS] = {};

static inline size_t otrx_min(size_t x, size_t y) {
	return x < y ? x : y;
}

/**************************************************
 * CRC32
 **************************************************/

static const uint32_t crc32_tbl[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

uint32_t otrx_crc32(uint8_t *buf, size_t len) {
	uint32_t crc = 0xffffffff;

	while (len) {
		crc = crc32_tbl[(crc ^ *buf) & 0xff] ^ (crc >> 8);
		buf++;
		len--;
	}

	return crc;
}

/**************************************************
 * Check
 **************************************************/

static void otrx_check_parse_options(int argc, char **argv) {
	int c;

	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch (c) {
		case 'o':
			trx_offset = atoi(optarg);
			break;
		}
	}
}

static int otrx_check(int argc, char **argv) {
	FILE *trx;
	struct trx_header hdr;
	size_t bytes, length;
	uint8_t buf[1024];
	uint32_t crc32;
	int i;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No TRX file passed\n");
		err = -EINVAL;
		goto out;
	}
	trx_path = argv[2];

	optind = 3;
	otrx_check_parse_options(argc, argv);

	trx = fopen(trx_path, "r");
	if (!trx) {
		fprintf(stderr, "Couldn't open %s\n", trx_path);
		err = -EACCES;
		goto out;
	}

	fseek(trx, trx_offset, SEEK_SET);
	bytes = fread(&hdr, 1, sizeof(hdr), trx);
	if (bytes != sizeof(hdr)) {
		fprintf(stderr, "Couldn't read %s header\n", trx_path);
		err =  -EIO;
		goto err_close;
	}

	if (le32_to_cpu(hdr.magic) != TRX_MAGIC) {
		fprintf(stderr, "Invalid TRX magic: 0x%08x\n", le32_to_cpu(hdr.magic));
		err =  -EINVAL;
		goto err_close;
	}

	length = le32_to_cpu(hdr.length);
	if (length < sizeof(hdr)) {
		fprintf(stderr, "Length read from TRX too low (%zu B)\n", length);
		err = -EINVAL;
		goto err_close;
	}

	crc32 = 0xffffffff;
	fseek(trx, trx_offset + TRX_FLAGS_OFFSET, SEEK_SET);
	length -= TRX_FLAGS_OFFSET;
	while ((bytes = fread(buf, 1, otrx_min(sizeof(buf), length), trx)) > 0) {
		for (i = 0; i < bytes; i++)
			crc32 = crc32_tbl[(crc32 ^ buf[i]) & 0xff] ^ (crc32 >> 8);
		length -= bytes;
	}

	if (length) {
		fprintf(stderr, "Couldn't read last %zd B of data from %s\n", length, trx_path);
		err = -EIO;
		goto err_close;
	}

	if (crc32 != le32_to_cpu(hdr.crc32)) {
		fprintf(stderr, "Invalid data crc32: 0x%08x instead of 0x%08x\n", crc32, le32_to_cpu(hdr.crc32));
		err =  -EINVAL;
		goto err_close;
	}

	printf("Found a valid TRX version %d\n", le32_to_cpu(hdr.version));

err_close:
	fclose(trx);
out:
	return err;
}

/**************************************************
 * Create
 **************************************************/

static void otrx_create_parse_options(int argc, char **argv) {
}

static ssize_t otrx_create_append_file(FILE *trx, const char *in_path) {
	FILE *in;
	size_t bytes;
	ssize_t length = 0;
	uint8_t buf[128];

	in = fopen(in_path, "r");
	if (!in) {
		fprintf(stderr, "Couldn't open %s\n", in_path);
		return -EACCES;
	}

	while ((bytes = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, bytes, trx) != bytes) {
			fprintf(stderr, "Couldn't write %zu B to %s\n", bytes, trx_path);
			length = -EIO;
			break;
		}
		length += bytes;
	}

	fclose(in);

	return length;
}

static ssize_t otrx_create_append_zeros(FILE *trx, size_t length) {
	uint8_t *buf;

	buf = malloc(length);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, length);

	if (fwrite(buf, 1, length, trx) != length) {
		fprintf(stderr, "Couldn't write %zu B to %s\n", length, trx_path);
		return -EIO;
	}

	return length;
}

static ssize_t otrx_create_align(FILE *trx, size_t curr_offset, size_t alignment) {
	if (curr_offset & (alignment - 1)) {
		size_t length = alignment - (curr_offset % alignment);
		return otrx_create_append_zeros(trx, length);
	}

	return 0;
}

static int otrx_create_write_hdr(FILE *trx, struct trx_header *hdr) {
	size_t bytes, length;
	uint8_t *buf;
	uint32_t crc32;

	hdr->magic = cpu_to_le32(TRX_MAGIC);
	hdr->version = 1;

	fseek(trx, 0, SEEK_SET);
	bytes = fwrite(hdr, 1, sizeof(struct trx_header), trx);
	if (bytes != sizeof(struct trx_header)) {
		fprintf(stderr, "Couldn't write TRX header to %s\n", trx_path);
		return -EIO;
	}

	length = le32_to_cpu(hdr->length);

	buf = malloc(length);
	if (!buf) {
		fprintf(stderr, "Couldn't alloc %zu B buffer\n", length);
		return -ENOMEM;
	}

	fseek(trx, 0, SEEK_SET);
	bytes = fread(buf, 1, length, trx);
	if (bytes != length) {
		fprintf(stderr, "Couldn't read %zu B of data from %s\n", length, trx_path);
		return -ENOMEM;
	}

	crc32 = otrx_crc32(buf + TRX_FLAGS_OFFSET, length - TRX_FLAGS_OFFSET);
	hdr->crc32 = cpu_to_le32(crc32);

	fseek(trx, 0, SEEK_SET);
	bytes = fwrite(hdr, 1, sizeof(struct trx_header), trx);
	if (bytes != sizeof(struct trx_header)) {
		fprintf(stderr, "Couldn't write TRX header to %s\n", trx_path);
		return -EIO;
	}

	return 0;
}

static int otrx_create(int argc, char **argv) {
	FILE *trx;
	struct trx_header hdr = {};
	ssize_t sbytes;
	size_t curr_idx = 0;
	size_t curr_offset = sizeof(hdr);
	int c;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No TRX file passed\n");
		err = -EINVAL;
		goto out;
	}
	trx_path = argv[2];

	optind = 3;
	otrx_create_parse_options(argc, argv);

	trx = fopen(trx_path, "w+");
	if (!trx) {
		fprintf(stderr, "Couldn't open %s\n", trx_path);
		err = -EACCES;
		goto out;
	}
	fseek(trx, curr_offset, SEEK_SET);

	optind = 3;
	while ((c = getopt(argc, argv, "f:b:")) != -1) {
		switch (c) {
		case 'f':
			if (curr_idx >= TRX_MAX_PARTS) {
				err = -ENOSPC;
				fprintf(stderr, "Reached TRX partitions limit, no place for %s\n", optarg);
				goto err_close;
			}

			sbytes = otrx_create_append_file(trx, optarg);
			if (sbytes < 0) {
				fprintf(stderr, "Failed to append file %s\n", optarg);
			} else {
				hdr.offset[curr_idx++] = curr_offset;
				curr_offset += sbytes;
			}

			sbytes = otrx_create_align(trx, curr_offset, 4);
			if (sbytes < 0)
				fprintf(stderr, "Failed to append zeros\n");
			else
				curr_offset += sbytes;

			break;
		case 'b':
			sbytes = strtol(optarg, NULL, 0) - curr_offset;
			if (sbytes < 0) {
				fprintf(stderr, "Current TRX length is 0x%zx, can't pad it with zeros to 0x%lx\n", curr_offset, strtol(optarg, NULL, 0));
			} else {
				sbytes = otrx_create_append_zeros(trx, sbytes);
				if (sbytes < 0)
					fprintf(stderr, "Failed to append zeros\n");
				else
					curr_offset += sbytes;
			}
			break;
		}
		if (err)
			break;
	}

	hdr.length = curr_offset;
	otrx_create_write_hdr(trx, &hdr);
err_close:
	fclose(trx);
out:
	return err;
}

/**************************************************
 * Extract
 **************************************************/

static void otrx_extract_parse_options(int argc, char **argv) {
	int c;

	while ((c = getopt(argc, argv, "c:e:o:1:2:3:")) != -1) {
		switch (c) {
		case 'o':
			trx_offset = atoi(optarg);
			break;
		case '1':
			partition[0] = optarg;
			break;
		case '2':
			partition[1] = optarg;
			break;
		case '3':
			partition[2] = optarg;
			break;
		}
	}
}

static int otrx_extract_copy(FILE *trx, size_t offset, size_t length, char *out_path) {
	FILE *out;
	size_t bytes;
	uint8_t *buf;
	int err = 0;

	out = fopen(out_path, "w");
	if (!out) {
		fprintf(stderr, "Couldn't open %s\n", out_path);
		err = -EACCES;
		goto out;
	}

	buf = malloc(length);
	if (!buf) {
		fprintf(stderr, "Couldn't alloc %zu B buffer\n", length);
		err =  -ENOMEM;
		goto err_close;
	}

	fseek(trx, offset, SEEK_SET);
	bytes = fread(buf, 1, length, trx);
	if (bytes != length) {
		fprintf(stderr, "Couldn't read %zu B of data from %s\n", length, trx_path);
		err =  -ENOMEM;
		goto err_free_buf;
	};

	bytes = fwrite(buf, 1, length, out);
	if (bytes != length) {
		fprintf(stderr, "Couldn't write %zu B to %s\n", length, out_path);
		err =  -ENOMEM;
		goto err_free_buf;
	}

	printf("Extracted 0x%zx bytes into %s\n", length, out_path);

err_free_buf:
	free(buf);
err_close:
	fclose(out);
out:
	return err;
}

static int otrx_extract(int argc, char **argv) {
	FILE *trx;
	struct trx_header hdr;
	size_t bytes;
	int i;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No TRX file passed\n");
		err = -EINVAL;
		goto out;
	}
	trx_path = argv[2];

	optind = 3;
	otrx_extract_parse_options(argc, argv);

	trx = fopen(trx_path, "r");
	if (!trx) {
		fprintf(stderr, "Couldn't open %s\n", trx_path);
		err = -EACCES;
		goto out;
	}

	fseek(trx, trx_offset, SEEK_SET);
	bytes = fread(&hdr, 1, sizeof(hdr), trx);
	if (bytes != sizeof(hdr)) {
		fprintf(stderr, "Couldn't read %s header\n", trx_path);
		err =  -EIO;
		goto err_close;
	}

	if (le32_to_cpu(hdr.magic) != TRX_MAGIC) {
		fprintf(stderr, "Invalid TRX magic: 0x%08x\n", le32_to_cpu(hdr.magic));
		err =  -EINVAL;
		goto err_close;
	}

	for (i = 0; i < TRX_MAX_PARTS; i++) {
		size_t length;

		if (!partition[i])
			continue;
		if (!hdr.offset[i]) {
			printf("TRX doesn't contain partition %d, can't extract %s\n", i + 1, partition[i]);
			continue;
		}

		if (i + 1 >= TRX_MAX_PARTS || !hdr.offset[i + 1])
			length = le32_to_cpu(hdr.length) - le32_to_cpu(hdr.offset[i]);
		else
			length = le32_to_cpu(hdr.offset[i + 1]) - le32_to_cpu(hdr.offset[i]);

		otrx_extract_copy(trx, trx_offset + le32_to_cpu(hdr.offset[i]), length, partition[i]);
	}

err_close:
	fclose(trx);
out:
	return err;
}

/**************************************************
 * Start
 **************************************************/

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("Checking TRX file:\n");
	printf("\totrx check <file> [options]\tcheck if file is a valid TRX\n");
	printf("\t-o offset\t\t\toffset of TRX data in file (default: 0)\n");
	printf("\n");
	printf("Creating new TRX file:\n");
	printf("\totrx create <file> [options] [partitions]\n");
	printf("\t-f file\t\t\t\t[partition] start new partition with content copied from file\n");
	printf("\t-b offset\t\t\t[partition] append zeros to partition till reaching absolute offset\n");
	printf("\n");
	printf("Extracting from TRX file:\n");
	printf("\totrx extract <file> [options]\textract partitions from TRX file\n");
	printf("\t-o offset\t\t\toffset of TRX data in file (default: 0)\n");
	printf("\t-1 file\t\t\t\tfile to extract 1st partition to (optional)\n");
	printf("\t-2 file\t\t\t\tfile to extract 2nd partition to (optional)\n");
	printf("\t-3 file\t\t\t\tfile to extract 3rd partition to (optional)\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		if (!strcmp(argv[1], "check"))
			return otrx_check(argc, argv);
		else if (!strcmp(argv[1], "create"))
			return otrx_create(argc, argv);
		else if (!strcmp(argv[1], "extract"))
			return otrx_extract(argc, argv);
	}

	usage();
	return 0;
}
