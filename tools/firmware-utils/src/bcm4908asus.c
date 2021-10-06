// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2021 Rafał Miłecki <rafal@milecki.pl>
 */

#include <byteswap.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le32(x)	bswap_32(x)
#define le32_to_cpu(x)	bswap_32(x)
#define cpu_to_le16(x)	bswap_16(x)
#define le16_to_cpu(x)	bswap_16(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le32(x)	(x)
#define le32_to_cpu(x)	(x)
#define cpu_to_le16(x)	(x)
#define le16_to_cpu(x)	(x)
#else
#error "Unsupported endianness"
#endif

/* BCM4908 specific tail - appended to the firmware image */
struct bcm4908img_tail {
	uint32_t crc32;
	uint32_t unk1;
	uint32_t family;
	uint32_t unk2;
	uint32_t unk3;
};

/* Asus BCM4908 tail - placed at the end of BCM4908 firmware, right before BCM4908 tail */
struct bcm4908asus_tail {
	uint8_t fw_ver[4];
	uint16_t build_no;
	uint16_t extend_no_u16;
	char productid[12];	/* The longest seen was 9 (e.g. GT-AC5300) */
	uint8_t unused1[8];
	uint32_t extend_no_u32;
	uint8_t unused2[31];
	uint8_t ver_flags;	/* Version or flags (only seen values: 0x00 and 0x01) */
};

/*
 * Example:
 *
 * 0053ffb0  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
 * 0053ffc0  03 00 00 04 80 01 94 52  47 54 2d 41 43 35 33 30  |.......RGT-AC530|
 * 0053ffd0  30 00 00 00 00 00 00 00  00 00 00 00 94 52 00 00  |0............R..|
 * 0053ffe0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
 * 0053fff0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 01  |................|
 * 00540000  c4 20 e6 72 32 57 00 00  08 49 00 00 03 00 00 00  |. .r2W...I......|
 * 00540010  02 00 00 00                                       |....|
 */

char *in_path = NULL;
char *out_path = NULL;

static inline size_t bcm4908asus_min(size_t x, size_t y) {
	return x < y ? x : y;
}

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

uint32_t bcm4908img_crc32(uint32_t crc, uint8_t *buf, size_t len) {
	while (len) {
		crc = crc32_tbl[(crc ^ *buf) & 0xff] ^ (crc >> 8);
		buf++;
		len--;
	}

	return crc;
}

/**************************************************
 * Info
 **************************************************/

static int bcm4908asus_info(int argc, char **argv)
{
	struct bcm4908asus_tail asus_tail;
	struct bcm4908img_tail img_tail;
	struct stat st;
	const char *pathname;
	size_t bytes, length;
	uint8_t buf[1024];
	uint32_t crc32;
	bool empty;
	FILE *fp;
	int i;
	int err = 0;

	if (argc < 3) {
		fprintf(stderr, "No BCM4908 Asus image pathname passed\n");
		err = -EINVAL;
		goto out;
	}
	pathname = argv[2];

	if (stat(pathname, &st)) {
		fprintf(stderr, "Failed to stat %s\n", pathname);
		err = -EIO;
		goto out;
	}

	fp = fopen(pathname, "r");
	if (!fp) {
		fprintf(stderr, "Failed to open %s\n", pathname);
		err = -EACCES;
		goto out;
	}

	crc32 = 0xffffffff;
	length = st.st_size - sizeof(asus_tail) - sizeof(img_tail);
	while (length && (bytes = fread(buf, 1, bcm4908asus_min(sizeof(buf), length), fp)) > 0) {
		crc32 = bcm4908img_crc32(crc32, buf, bytes);
		length -= bytes;
	}

	if (length) {
		fprintf(stderr, "Failed to read from %s\n", pathname);
		err = -EIO;
		goto err_close;
	}

	if (fread(&asus_tail, 1, sizeof(asus_tail), fp) != sizeof(asus_tail)) {
		fprintf(stderr, "Failed to read BCM4908 Asus image tail\n");
		err = -EIO;
		goto err_close;
	}
	crc32 = bcm4908img_crc32(crc32, (uint8_t *)&asus_tail, sizeof(asus_tail));

	if (fread(&img_tail, 1, sizeof(img_tail), fp) != sizeof(img_tail)) {
		fprintf(stderr, "Failed to read BCM4908 Asus image tail\n");
		err = -EIO;
		goto err_close;
	}

	if (crc32 != le32_to_cpu(img_tail.crc32)) {
		fprintf(stderr, "Invalid crc32 (calculated 0x%08x expected 0x%08x)\n", crc32, le32_to_cpu(img_tail.crc32));
		err =  -EINVAL;
		goto err_close;
	}

	empty = true;
	for (i = 0; i < sizeof(asus_tail); i++) {
		if (((uint8_t *)&asus_tail)[i] != 0xff) {
			empty = false;
			break;
		}
	}
	if (empty) {
		fprintf(stderr, "BCM4908 image doesn't contain Asus tail\n");
		err =  -EINVAL;
		goto err_close;
	}

	printf("Firmware version:\t%u.%u.%u.%u\n", asus_tail.fw_ver[0], asus_tail.fw_ver[1], asus_tail.fw_ver[2], asus_tail.fw_ver[3]);
	printf("Build number:\t\t%u\n", le16_to_cpu(asus_tail.build_no));
	printf("Extended number:\t%u\n", asus_tail.ver_flags & 0x1 ? le32_to_cpu(asus_tail.extend_no_u32) : le16_to_cpu(asus_tail.extend_no_u16));
	printf("Product ID:\t\t%s\n", asus_tail.productid);

err_close:
	fclose(fp);
out:
	return err;
}

/**************************************************
 * Create
 **************************************************/

static void bcm4908asus_create_parse_options(int argc, char **argv, struct bcm4908asus_tail *tail)
{
	uint32_t tmp32;
	uint16_t tmp16;
	int c;

	while ((c = getopt(argc, argv, "i:o:p:f:b:e:")) != -1) {
		switch (c) {
		case 'i':
			in_path = optarg;
			break;
		case 'o':
			out_path = optarg;
			break;
		case 'p':
			strncpy(tail->productid, optarg, sizeof(tail->productid));
			break;
		case 'f':
			if (sscanf(optarg, "%hhu.%hhu.%hhu.%hhu", &(tail->fw_ver[0]), &tail->fw_ver[1], &tail->fw_ver[2], &tail->fw_ver[3]) != 4)
				fprintf(stderr, "Version %s doesn't match suppored 4-digits format\n", optarg);
			break;
		case 'b':
			tmp16 = strtol(optarg, NULL, 0);
			tail->build_no = cpu_to_le16(tmp16);
			break;
		case 'e':
			tmp32 = strtol(optarg, NULL, 0);
			tail->ver_flags = 0x01;
			tail->extend_no_u32 = cpu_to_le32(tmp32);
			tail->extend_no_u16 = cpu_to_le16((uint16_t)tmp32);
			break;
		}
	}
}

static int bcm4908asus_create(int argc, char **argv)
{
	struct bcm4908asus_tail asus_tail = {};
	struct bcm4908img_tail img_tail = {};
	struct stat st;
	uint32_t crc32_old;
	uint32_t crc32_new;
	uint8_t buf[1024];
	FILE *out = NULL;
	FILE *in = NULL;
	size_t length;
	size_t bytes;
	FILE *fp;
	int i;
	int err = 0;

	/* Parse & validate arguments */
	bcm4908asus_create_parse_options(argc, argv, &asus_tail);
	if (!in_path) {
		fprintf(stderr, "No BCM4908 Asus image pathname passed\n");
		err = -EINVAL;
		goto err;
	}

	/* Check input file: size, access, empty space for Asus tail */

	if (stat(in_path, &st)) {
		fprintf(stderr, "Failed to stat %s\n", in_path);
		err = -EIO;
		goto err;
	}

	in = fopen(in_path, "r+");
	if (!in) {
		fprintf(stderr, "Failed to open %s\n", in_path);
		err = -EIO;
		goto err;
	}

	length = st.st_size - sizeof(asus_tail) - sizeof(img_tail);
	fseek(in, length, SEEK_SET);
	if (fread(buf, 1, sizeof(asus_tail), in) != sizeof(asus_tail)) {
		fprintf(stderr, "Failed to read BCM4908 image from %s\n", in_path);
		err = -EIO;
		goto err;
	}
	for (i = 0; i < sizeof(asus_tail); i++) {
		if (buf[i] != 0xff) {
			fprintf(stderr, "Input BCM4908 image doesn't have empty 64 B tail\n");
			err = -ENOSPC;
			goto err;
		}
	}
	rewind(in);

	/* Create new BCM4908 Asus image file if requested (otherwise input file will get modified) */

	if (out_path && !(out = fopen(out_path, "w+"))) {
		fprintf(stderr, "Failed to open %s\n", out_path);
		err = -EIO;
		goto err;
	}

	/* Calculate CRC for data that doesn't get modified. Optionally copy input file if requested */

	crc32_old = 0xffffffff;
	length = st.st_size - sizeof(asus_tail) - sizeof(img_tail);
	while (length && (bytes = fread(buf, 1, bcm4908asus_min(sizeof(buf), length), in)) > 0) {
		if (out && fwrite(buf, 1, bytes, out) != bytes) {
			fprintf(stderr, "Failed to write %zu B to %s\n", bytes, out_path);
			err = -EIO;
			goto err;
		}
		crc32_old = bcm4908img_crc32(crc32_old, buf, bytes);
		length -= bytes;
	}

	if (length) {
		fprintf(stderr, "Failed to read from %s\n", in_path);
		err = -EIO;
		goto err;
	}

	crc32_new = crc32_old;

	/* Finish calculating old checksum & verify it */

	for (i = 0; i < sizeof(asus_tail); i++) {
		uint8_t val = 0xff;

		crc32_old = bcm4908img_crc32(crc32_old, &val, 1);
	}
	fseek(in, sizeof(asus_tail), SEEK_CUR);

	if (fread(&img_tail, 1, sizeof(img_tail), in) != sizeof(img_tail)) {
		fprintf(stderr, "Failed to read BCM4908 image tail from %s\n", in_path);
		err = -EIO;
		goto err;
	}

	if (crc32_old != le32_to_cpu(img_tail.crc32)) {
		fprintf(stderr, "Invalid data crc32: calculated 0x%08x instead of 0x%08x\n", crc32_old, le32_to_cpu(img_tail.crc32));
		err =  -EPROTO;
		goto err;
	}

	/* Write Asus tail & updated BCM4908 tail */

	if (out) {
		fp = out;
	} else {
		fp = in;
		fseek(in, -sizeof(asus_tail) - sizeof(img_tail), SEEK_CUR);
	}

	if (fwrite(&asus_tail, 1, sizeof(asus_tail), fp) != sizeof(asus_tail)) {
		fprintf(stderr, "Failed to write BCM4908 image Asus tail to %s\n", out_path);
		err = -EIO;
		goto err;
	}

	crc32_new = bcm4908img_crc32(crc32_new, (uint8_t *)&asus_tail, sizeof(asus_tail));
	img_tail.crc32 = cpu_to_le32(crc32_new);
	if (fwrite(&img_tail, 1, sizeof(img_tail), fp) != sizeof(img_tail)) {
		fprintf(stderr, "Failed to write BCM4908 image tail to %s\n", out_path);
		err = -EIO;
		goto err;
	}

err:
	if (out)
		fclose(out);
	if (in)
		fclose(in);
	return err;
}

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("Info about BCM4908 Asus image:\n");
	printf("\tbcm4908asus info <file>\tget info about BCM4908 Asus image\n");
	printf("\n");
	printf("Create a BCM4908 Asus image:\n");
	printf("\tbcm4908asus create\tinsert Asus info into BCM4908 image\n");
	printf("\t-i file\t\t\t\tinput BCM4908 image file (required)\n");
	printf("\t-o file\t\t\t\toutput BCM4908 Asus image file\n");
	printf("\t-p productid\t\t\tproduct (device) ID\n");
	printf("\t-f firmware version\t\tfirmware version formatted with 4 digits like: 1.2.3.4\n");
	printf("\t-b build number\t\tbuild number (e.g. 380, 382, 384)\n");
	printf("\t-e extend number\t\textended number (e.g. 21140, 81622, 81695, 82037)\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		if (!strcmp(argv[1], "info"))
			return bcm4908asus_info(argc, argv);
		else if (!strcmp(argv[1], "create"))
			return bcm4908asus_create(argc, argv);
	}

	usage();

	return 0;
}
