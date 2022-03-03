// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 Rafał Miłecki <rafal@milecki.pl>
 */

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le32(x)	bswap_32(x)
#define le32_to_cpu(x)	bswap_32(x)
#define cpu_to_be32(x)	(x)
#define be32_to_cpu(x)	(x)
#define cpu_to_le16(x)	bswap_16(x)
#define le16_to_cpu(x)	bswap_16(x)
#define cpu_to_be16(x)	(x)
#define be16_to_cpu(x)	(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)
#define cpu_to_be32(x)	bswap_32(x)
#define be32_to_cpu(x)	bswap_32(x)
#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#define cpu_to_be16(x)	bswap_16(x)
#define be16_to_cpu(x)	bswap_16(x)
#else
#error "Unsupported endianness"
#endif

#define WFI_VERSION			0x00005732
#define WFI_VERSION_NAND_1MB_DATA	0x00005731

#define WFI_NOR_FLASH			1
#define WFI_NAND16_FLASH		2
#define WFI_NAND128_FLASH		3
#define WFI_NAND256_FLASH		4
#define WFI_NAND512_FLASH		5
#define WFI_NAND1024_FLASH		6
#define WFI_NAND2048_FLASH		7

#define WFI_FLAG_HAS_PMC		0x1
#define WFI_FLAG_SUPPORTS_BTRM		0x2

#define UBI_EC_HDR_MAGIC		0x55424923

static int debug;

struct bcm4908img_tail {
	uint32_t crc32;
	uint32_t version;
	uint32_t chip_id;
	uint32_t flash_type;
	uint32_t flags;
};

/**
 * struct bcm4908img_info - info about BCM4908 image
 *
 * Standard BCM4908 image consists of:
 * 1. (Optional) vedor header
 * 2. (Optional) cferom
 * 3. bootfs  ─┐
 * 4. padding  ├─ firmware
 * 5. rootfs  ─┘
 * 6. BCM4908 tail
 */
struct bcm4908img_info {
	size_t file_size;
	size_t cferom_offset;
	size_t bootfs_offset;
	size_t padding_offset;
	size_t rootfs_offset;
	uint32_t crc32;			/* Calculated checksum */
	struct bcm4908img_tail tail;
};

char *pathname;

static inline size_t bcm4908img_min(size_t x, size_t y) {
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

uint32_t bcm4908img_crc32(uint32_t crc, const void *buf, size_t len) {
	const uint8_t *in = buf;

	while (len) {
		crc = crc32_tbl[(crc ^ *in) & 0xff] ^ (crc >> 8);
		in++;
		len--;
	}

	return crc;
}

/**************************************************
 * Helpers
 **************************************************/

static FILE *bcm4908img_open(const char *pathname, const char *mode) {
	struct stat st;

	if (pathname)
		return fopen(pathname, mode);

	if (isatty(fileno(stdin))) {
		fprintf(stderr, "Reading from TTY stdin is unsupported\n");
		return NULL;
	}

	if (fstat(fileno(stdin), &st)) {
		fprintf(stderr, "Failed to fstat stdin: %d\n", -errno);
		return NULL;
	}

	if (S_ISFIFO(st.st_mode)) {
		fprintf(stderr, "Reading from pipe stdin is unsupported\n");
		return NULL;
	}

	return stdin;
}

static void bcm4908img_close(FILE *fp) {
	if (fp != stdin)
		fclose(fp);
}

static int bcm4908img_calc_crc32(FILE *fp, struct bcm4908img_info *info) {
	uint8_t buf[1024];
	size_t length;
	size_t bytes;

	/* Start with cferom (or bootfs) - skip vendor header */
	fseek(fp, info->cferom_offset, SEEK_SET);

	info->crc32 = 0xffffffff;
	length = info->file_size - info->cferom_offset - sizeof(struct bcm4908img_tail);
	while (length && (bytes = fread(buf, 1, bcm4908img_min(sizeof(buf), length), fp)) > 0) {
		info->crc32 = bcm4908img_crc32(info->crc32, buf, bytes);
		length -= bytes;
	}
	if (length) {
		fprintf(stderr, "Failed to read last %zd B of data\n", length);
		return -EIO;
	}

	return 0;
}

/**************************************************
 * Existing firmware parser
 **************************************************/

struct chk_header {
	uint32_t magic;
	uint32_t header_len;
	uint8_t  reserved[8];
	uint32_t kernel_chksum;
	uint32_t rootfs_chksum;
	uint32_t kernel_len;
	uint32_t rootfs_len;
	uint32_t image_chksum;
	uint32_t header_chksum;
	char board_id[0];
};

static bool bcm4908img_is_all_ff(const void *buf, size_t length)
{
	const uint8_t *in = buf;
	int i;

	for (i = 0; i < length; i++) {
		if (in[i] != 0xff)
			return false;
	}

	return true;
}

static int bcm4908img_parse(FILE *fp, struct bcm4908img_info *info) {
	struct bcm4908img_tail *tail = &info->tail;
	struct chk_header *chk;
	struct stat st;
	uint8_t buf[1024];
	uint16_t tmp16;
	size_t length;
	size_t bytes;
	int err = 0;

	memset(info, 0, sizeof(*info));

	/* File size */

	if (fstat(fileno(fp), &st)) {
		err = -errno;
		fprintf(stderr, "Failed to fstat: %d\n", err);
		return err;
	}
	info->file_size = st.st_size;

	/* Vendor formats */

	rewind(fp);
	if (fread(buf, 1, sizeof(buf), fp) != sizeof(buf)) {
		fprintf(stderr, "Failed to read file header\n");
		return -EIO;
	}
	chk = (void *)buf;
	if (be32_to_cpu(chk->magic) == 0x2a23245e)
		info->cferom_offset = be32_to_cpu(chk->header_len);

	/* Offsets */

	for (info->bootfs_offset = info->cferom_offset;
	     info->bootfs_offset < info->file_size;
	     info->bootfs_offset += 0x20000) {
		if (fseek(fp, info->bootfs_offset, SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek to the 0x%zx\n", info->bootfs_offset);
			return err;
		}
		if (fread(&tmp16, 1, sizeof(tmp16), fp) != sizeof(tmp16)) {
			fprintf(stderr, "Failed to read while looking for JFFS2\n");
			return -EIO;
		}
		if (be16_to_cpu(tmp16) == 0x8519)
			break;
	}
	if (info->bootfs_offset >= info->file_size) {
		fprintf(stderr, "Failed to find bootfs offset\n");
		return -EPROTO;
	}

	for (info->rootfs_offset = info->bootfs_offset;
	     info->rootfs_offset < info->file_size;
	     info->rootfs_offset += 0x20000) {
		uint32_t *magic = (uint32_t *)&buf[0];

		if (fseek(fp, info->rootfs_offset, SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek: %d\n", err);
			return err;
		}

		length = info->padding_offset ? sizeof(*magic) : 256;
		bytes = fread(buf, 1, length, fp);
		if (bytes != length) {
			fprintf(stderr, "Failed to read %zu bytes\n", length);
			return -EIO;
		}

		if (!info->padding_offset && bcm4908img_is_all_ff(buf, length))
			info->padding_offset = info->rootfs_offset;

		if (be32_to_cpu(*magic) == UBI_EC_HDR_MAGIC)
			break;
	}
	if (info->rootfs_offset >= info->file_size) {
		fprintf(stderr, "Failed to find rootfs offset\n");
		return -EPROTO;
	}

	/* CRC32 */

	/* Start with cferom (or bootfs) - skip vendor header */
	fseek(fp, info->cferom_offset, SEEK_SET);

	info->crc32 = 0xffffffff;
	length = info->file_size - info->cferom_offset - sizeof(*tail);
	while (length && (bytes = fread(buf, 1, bcm4908img_min(sizeof(buf), length), fp)) > 0) {
		info->crc32 = bcm4908img_crc32(info->crc32, buf, bytes);
		length -= bytes;
	}
	if (length) {
		fprintf(stderr, "Failed to read last %zd B of data\n", length);
		return -EIO;
	}

	/* Tail */

	if (fread(tail, 1, sizeof(*tail), fp) != sizeof(*tail)) {
		fprintf(stderr, "Failed to read BCM4908 image tail\n");
		return -EIO;
	}

	/* Standard validation */

	if (info->crc32 != le32_to_cpu(tail->crc32)) {
		fprintf(stderr, "Invalid data crc32: 0x%08x instead of 0x%08x\n", info->crc32, le32_to_cpu(tail->crc32));
		return -EPROTO;
	}

	return 0;
}

/**************************************************
 * Info
 **************************************************/

static int bcm4908img_info(int argc, char **argv) {
	struct bcm4908img_info info;
	const char *pathname = NULL;
	FILE *fp;
	int c;
	int err = 0;

	while ((c = getopt(argc, argv, "i:")) != -1) {
		switch (c) {
		case 'i':
			pathname = optarg;
			break;
		}
	}

	fp = bcm4908img_open(pathname, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open BCM4908 image\n");
		err = -EACCES;
		goto out;
	}

	err = bcm4908img_parse(fp, &info);
	if (err) {
		fprintf(stderr, "Failed to parse BCM4908 image\n");
		goto err_close;
	}

	if (info.bootfs_offset != info.cferom_offset)
		printf("cferom offset:\t%zu\n", info.cferom_offset);
	printf("bootfs offset:\t0x%zx\n", info.bootfs_offset);
	if (info.padding_offset)
		printf("padding offset:\t0x%zx\n", info.padding_offset);
	printf("rootfs offset:\t0x%zx\n", info.rootfs_offset);
	printf("Checksum:\t0x%08x\n", info.crc32);

err_close:
	bcm4908img_close(fp);
out:
	return err;
}

/**************************************************
 * Create
 **************************************************/

static ssize_t bcm4908img_create_append_file(FILE *trx, const char *in_path, uint32_t *crc32) {
	FILE *in;
	size_t bytes;
	ssize_t length = 0;
	uint8_t buf[1024];

	in = fopen(in_path, "r");
	if (!in) {
		fprintf(stderr, "Failed to open %s\n", in_path);
		return -EACCES;
	}

	while ((bytes = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, bytes, trx) != bytes) {
			fprintf(stderr, "Failed to write %zu B to %s\n", bytes, pathname);
			length = -EIO;
			break;
		}
		*crc32 = bcm4908img_crc32(*crc32, buf, bytes);
		length += bytes;
	}

	fclose(in);

	return length;
}

static ssize_t bcm4908img_create_append_zeros(FILE *trx, size_t length) {
	uint8_t *buf;

	buf = malloc(length);
	if (!buf)
		return -ENOMEM;
	memset(buf, 0, length);

	if (fwrite(buf, 1, length, trx) != length) {
		fprintf(stderr, "Failed to write %zu B to %s\n", length, pathname);
		free(buf);
		return -EIO;
	}

	free(buf);

	return length;
}

static ssize_t bcm4908img_create_align(FILE *trx, size_t cur_offset, size_t alignment) {
	if (cur_offset & (alignment - 1)) {
		size_t length = alignment - (cur_offset % alignment);
		return bcm4908img_create_append_zeros(trx, length);
	}

	return 0;
}

static int bcm4908img_create(int argc, char **argv) {
	struct bcm4908img_tail tail = {
		.version = cpu_to_le32(WFI_VERSION),
		.chip_id = cpu_to_le32(0x4908),
		.flash_type = cpu_to_le32(WFI_NAND128_FLASH),
		.flags = cpu_to_le32(WFI_FLAG_SUPPORTS_BTRM),
	};
	uint32_t crc32 = 0xffffffff;
	size_t cur_offset = 0;
	ssize_t bytes;
	FILE *fp;
	int c;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No BCM4908 image pathname passed\n");
		err = -EINVAL;
		goto out;
	}
	pathname = argv[2];

	fp = fopen(pathname, "w+");
	if (!fp) {
		fprintf(stderr, "Failed to open %s\n", pathname);
		err = -EACCES;
		goto out;
	}

	optind = 3;
	while ((c = getopt(argc, argv, "f:a:A:")) != -1) {
		switch (c) {
		case 'f':
			bytes = bcm4908img_create_append_file(fp, optarg, &crc32);
			if (bytes < 0) {
				fprintf(stderr, "Failed to append file %s\n", optarg);
			} else {
				cur_offset += bytes;
			}
			break;
		case 'a':
			bytes = bcm4908img_create_align(fp, cur_offset, strtol(optarg, NULL, 0));
			if (bytes < 0)
				fprintf(stderr, "Failed to append zeros\n");
			else
				cur_offset += bytes;
			break;
		case 'A':
			bytes = strtol(optarg, NULL, 0) - cur_offset;
			if (bytes < 0) {
				fprintf(stderr, "Current BCM4908 image length is 0x%zx, can't pad it with zeros to 0x%lx\n", cur_offset, strtol(optarg, NULL, 0));
			} else {
				bytes = bcm4908img_create_append_zeros(fp, bytes);
				if (bytes < 0)
					fprintf(stderr, "Failed to append zeros\n");
				else
					cur_offset += bytes;
			}
			break;
		}
		if (err)
			goto err_close;
	}

	tail.crc32 = cpu_to_le32(crc32);

	bytes = fwrite(&tail, 1, sizeof(tail), fp);
	if (bytes != sizeof(tail)) {
		fprintf(stderr, "Failed to write BCM4908 image tail to %s\n", pathname);
		return -EIO;
	}

err_close:
	fclose(fp);
out:
	return err;
}

/**************************************************
 * Extract
 **************************************************/

static int bcm4908img_extract(int argc, char **argv) {
	struct bcm4908img_info info;
	const char *pathname = NULL;
	const char *type = NULL;
	uint8_t buf[1024];
	size_t offset;
	size_t length;
	size_t bytes;
	FILE *fp;
	int c;
	int err = 0;

	while ((c = getopt(argc, argv, "i:t:")) != -1) {
		switch (c) {
		case 'i':
			pathname = optarg;
			break;
		case 't':
			type = optarg;
			break;
		}
	}

	fp = bcm4908img_open(pathname, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open BCM4908 image\n");
		err = -EACCES;
		goto err_out;
	}

	err = bcm4908img_parse(fp, &info);
	if (err) {
		fprintf(stderr, "Failed to parse BCM4908 image\n");
		goto err_close;
	}

	if (!type) {
		err = -EINVAL;
		fprintf(stderr, "No data to extract specified\n");
		goto err_close;
	} else if (!strcmp(type, "cferom")) {
		offset = info.cferom_offset;
		length = info.bootfs_offset - offset;
		if (!length) {
			err = -ENOENT;
			fprintf(stderr, "This BCM4908 image doesn't contain cferom\n");
			goto err_close;
		}
	} else if (!strcmp(type, "bootfs")) {
		offset = info.bootfs_offset;
		length = (info.padding_offset ? info.padding_offset : info.rootfs_offset) - offset;
	} else if (!strcmp(type, "rootfs")) {
		offset = info.rootfs_offset;
		length = info.file_size - offset - sizeof(struct bcm4908img_tail);
	} else if (!strcmp(type, "firmware")) {
		offset = info.bootfs_offset;
		length = info.file_size - offset - sizeof(struct bcm4908img_tail);
	} else {
		err = -EINVAL;
		fprintf(stderr, "Unsupported extract type: %s\n", type);
		goto err_close;
	}

	if (!length) {
		err = -EINVAL;
		fprintf(stderr, "Failed to find requested data in input image\n");
		goto err_close;
	}

	fseek(fp, offset, SEEK_SET);
	while (length && (bytes = fread(buf, 1, bcm4908img_min(sizeof(buf), length), fp)) > 0) {
		fwrite(buf, bytes, 1, stdout);
		length -= bytes;
	}
	if (length) {
		err = -EIO;
		fprintf(stderr, "Failed to read last %zd B of data\n", length);
		goto err_close;
	}

err_close:
	bcm4908img_close(fp);
err_out:
	return err;
}

/**************************************************
 * bootfs
 **************************************************/

#define JFFS2_MAGIC_BITMASK 0x1985

#define JFFS2_COMPR_NONE	0x00
#define JFFS2_COMPR_ZERO	0x01
#define JFFS2_COMPR_RTIME	0x02
#define JFFS2_COMPR_RUBINMIPS	0x03
#define JFFS2_COMPR_COPY	0x04
#define JFFS2_COMPR_DYNRUBIN	0x05
#define JFFS2_COMPR_ZLIB	0x06
#define JFFS2_COMPR_LZO		0x07
/* Compatibility flags. */
#define JFFS2_COMPAT_MASK 0xc000      /* What do to if an unknown nodetype is found */
#define JFFS2_NODE_ACCURATE 0x2000
/* INCOMPAT: Fail to mount the filesystem */
#define JFFS2_FEATURE_INCOMPAT 0xc000
/* ROCOMPAT: Mount read-only */
#define JFFS2_FEATURE_ROCOMPAT 0x8000
/* RWCOMPAT_COPY: Mount read/write, and copy the node when it's GC'd */
#define JFFS2_FEATURE_RWCOMPAT_COPY 0x4000
/* RWCOMPAT_DELETE: Mount read/write, and delete the node when it's GC'd */
#define JFFS2_FEATURE_RWCOMPAT_DELETE 0x0000

#define JFFS2_NODETYPE_DIRENT (JFFS2_FEATURE_INCOMPAT | JFFS2_NODE_ACCURATE | 1)

typedef struct {
	uint32_t v32;
} __attribute__((packed)) jint32_t;

typedef struct {
	uint16_t v16;
} __attribute__((packed)) jint16_t;

struct jffs2_unknown_node
{
	/* All start like this */
	jint16_t magic;
	jint16_t nodetype;
	jint32_t totlen; /* So we can skip over nodes we don't grok */
	jint32_t hdr_crc;
};

struct jffs2_raw_dirent
{
	jint16_t magic;
	jint16_t nodetype;	/* == JFFS2_NODETYPE_DIRENT */
	jint32_t totlen;
	jint32_t hdr_crc;
	jint32_t pino;
	jint32_t version;
	jint32_t ino; /* == zero for unlink */
	jint32_t mctime;
	uint8_t nsize;
	uint8_t type;
	uint8_t unused[2];
	jint32_t node_crc;
	jint32_t name_crc;
	uint8_t name[0];
};

#define je16_to_cpu(x) ((x).v16)
#define je32_to_cpu(x) ((x).v32)

static int bcm4908img_bootfs_ls(FILE *fp, struct bcm4908img_info *info) {
	struct jffs2_unknown_node node;
	struct jffs2_raw_dirent dirent;
	size_t offset;
	size_t bytes;
	int err = 0;

	for (offset = info->bootfs_offset; ; offset += (je32_to_cpu(node.totlen) + 0x03) & ~0x03) {
		char name[FILENAME_MAX + 1];

		if (fseek(fp, offset, SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek: %d\n", err);
			return err;
		}

		bytes = fread(&node, 1, sizeof(node), fp);
		if (bytes != sizeof(node)) {
			fprintf(stderr, "Failed to read %zu bytes\n", sizeof(node));
			return -EIO;
		}

		if (je16_to_cpu(node.magic) != JFFS2_MAGIC_BITMASK) {
			break;
		}

		if (je16_to_cpu(node.nodetype) != JFFS2_NODETYPE_DIRENT) {
			continue;
		}

		memcpy(&dirent, &node, sizeof(node));
		bytes += fread((uint8_t *)&dirent + sizeof(node), 1, sizeof(dirent) - sizeof(node), fp);
		if (bytes != sizeof(dirent)) {
			fprintf(stderr, "Failed to read %zu bytes\n", sizeof(node));
			return -EIO;
		}

		if (dirent.nsize + 1 > sizeof(name)) {
			/* Keep reading & printing BUT exit with error code */
			fprintf(stderr, "Too long filename\n");
			err = -ENOMEM;
			continue;
		}

		bytes = fread(name, 1, dirent.nsize, fp);
		if (bytes != dirent.nsize) {
			fprintf(stderr, "Failed to read filename\n");
			return -EIO;
		}
		name[bytes] = '\0';

		printf("%s\n", name);
	}

	return err;
}

static int bcm4908img_bootfs_mv(FILE *fp, struct bcm4908img_info *info, int argc, char **argv) {
	struct jffs2_unknown_node node;
	struct jffs2_raw_dirent dirent;
	const char *oldname;
	const char *newname;
	size_t offset;
	size_t bytes;
	int err = -ENOENT;

	if (argc - optind < 2) {
		fprintf(stderr, "No enough arguments passed\n");
		return -EINVAL;
	}
	oldname = argv[optind++];
	newname = argv[optind++];

	if (strlen(newname) != strlen(oldname)) {
		fprintf(stderr, "New filename must have the same length as the old one\n");
		return -EINVAL;
	}

	for (offset = info->bootfs_offset; ; offset += (je32_to_cpu(node.totlen) + 0x03) & ~0x03) {
		char name[FILENAME_MAX];
		uint32_t crc32;

		if (fseek(fp, offset, SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek: %d\n", err);
			return err;
		}

		bytes = fread(&node, 1, sizeof(node), fp);
		if (bytes != sizeof(node)) {
			fprintf(stderr, "Failed to read %zu bytes\n", sizeof(node));
			return -EIO;
		}

		if (je16_to_cpu(node.magic) != JFFS2_MAGIC_BITMASK) {
			break;
		}

		if (je16_to_cpu(node.nodetype) != JFFS2_NODETYPE_DIRENT) {
			continue;
		}

		bytes += fread((uint8_t *)&dirent + sizeof(node), 1, sizeof(dirent) - sizeof(node), fp);
		if (bytes != sizeof(dirent)) {
			fprintf(stderr, "Failed to read %zu bytes\n", sizeof(node));
			return -EIO;
		}

		if (dirent.nsize + 1 > sizeof(name)) {
			fprintf(stderr, "Too long filename\n");
			err = -ENOMEM;
			continue;
		}

		bytes = fread(name, 1, dirent.nsize, fp);
		if (bytes != dirent.nsize) {
			fprintf(stderr, "Failed to read filename\n");
			return -EIO;
		}
		name[bytes] = '\0';

		if (debug)
			printf("offset:%08zx name_crc:%04x filename:%s\n", offset, je32_to_cpu(dirent.name_crc), name);

		if (strcmp(name, oldname)) {
			continue;
		}

		if (fseek(fp, offset + offsetof(struct jffs2_raw_dirent, name_crc), SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek: %d\n", err);
			return err;
		}
		crc32 = bcm4908img_crc32(0, newname, dirent.nsize);
		bytes = fwrite(&crc32, 1, sizeof(crc32), fp);
		if (bytes != sizeof(crc32)) {
			fprintf(stderr, "Failed to write new CRC32\n");
			return -EIO;
		}

		if (fseek(fp, offset + offsetof(struct jffs2_raw_dirent, name), SEEK_SET)) {
			err = -errno;
			fprintf(stderr, "Failed to fseek: %d\n", err);
			return err;
		}
		bytes = fwrite(newname, 1, dirent.nsize, fp);
		if (bytes != dirent.nsize) {
			fprintf(stderr, "Failed to write new filename\n");
			return -EIO;
		}

		/* Calculate new BCM4908 image checksum */

		err = bcm4908img_calc_crc32(fp, info);
		if (err) {
			fprintf(stderr, "Failed to write new filename\n");
			return err;
		}

		info->tail.crc32 = cpu_to_le32(info->crc32);
		if (fseek(fp, -sizeof(struct bcm4908img_tail), SEEK_END)) {
			err = -errno;
			fprintf(stderr, "Failed to write new filename\n");
			return err;
		}

		if (fwrite(&info->tail, 1, sizeof(struct bcm4908img_tail), fp) != sizeof(struct bcm4908img_tail)) {
			fprintf(stderr, "Failed to write updated tail\n");
			return -EIO;
		}

		printf("Successfully renamed %s to the %s\n", oldname, newname);

		return 0;
	}

	fprintf(stderr, "Failed to find %s\n", oldname);

	return -ENOENT;
}

static int bcm4908img_bootfs(int argc, char **argv) {
	struct bcm4908img_info info;
	const char *pathname = NULL;
	const char *mode;
	const char *cmd;
	FILE *fp;
	int c;
	int err = 0;

	while ((c = getopt(argc, argv, "i:")) != -1) {
		switch (c) {
		case 'i':
			pathname = optarg;
			break;
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, "No bootfs command specified\n");
		err = -EINVAL;
		goto out;
	}
	cmd = argv[optind++];

	mode = strcmp(cmd, "mv") ? "r" : "r+";
	fp = bcm4908img_open(pathname, mode);
	if (!fp) {
		fprintf(stderr, "Failed to open BCM4908 image\n");
		err = -EACCES;
		goto out;
	}

	err = bcm4908img_parse(fp, &info);
	if (err) {
		fprintf(stderr, "Failed to parse BCM4908 image\n");
		goto err_close;
	}

	if (!strcmp(cmd, "ls")) {
		err = bcm4908img_bootfs_ls(fp, &info);
	} else if (!strcmp(cmd, "mv")) {
		err = bcm4908img_bootfs_mv(fp, &info, argc, argv);
	} else {
		err = -EINVAL;
		fprintf(stderr, "Unsupported bootfs command: %s\n", cmd);
	}

err_close:
	bcm4908img_close(fp);
out:
	return err;
}

/**************************************************
 * Start
 **************************************************/

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("Info about a BCM4908 image:\n");
	printf("\tbcm4908img info <options>\n");
	printf("\t-i <file>\t\t\t\tinput BCM490 image\n");
	printf("\n");
	printf("Creating a new BCM4908 image:\n");
	printf("\tbcm4908img create <file> [options]\n");
	printf("\t-f file\t\t\t\tadd data from specified file\n");
	printf("\t-a alignment\t\t\tpad image with zeros to specified alignment\n");
	printf("\t-A offset\t\t\t\tappend zeros until reaching specified offset\n");
	printf("\n");
	printf("Extracting from a BCM4908 image:\n");
	printf("\tbcm4908img extract <options>\n");
	printf("\t-i <file>\t\t\t\tinput BCM490 image\n");
	printf("\t-t <type>\t\t\t\tone of: cferom, bootfs, rootfs, firmware\n");
	printf("\n");
	printf("Access bootfs in a BCM4908 image:\n");
	printf("\tbcm4908img bootfs <options> <command> <arguments>\n");
	printf("\t-i <file>\t\t\t\tinput BCM490 image\n");
	printf("\tls\t\t\t\t\tlist bootfs files\n");
	printf("\tmv <source> <dest>\t\t\trename bootfs file\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		optind++;
		if (!strcmp(argv[1], "info"))
			return bcm4908img_info(argc, argv);
		else if (!strcmp(argv[1], "create"))
			return bcm4908img_create(argc, argv);
		else if (!strcmp(argv[1], "extract"))
			return bcm4908img_extract(argc, argv);
		else if (!strcmp(argv[1], "bootfs"))
			return bcm4908img_bootfs(argc, argv);
	}

	usage();
	return 0;
}
