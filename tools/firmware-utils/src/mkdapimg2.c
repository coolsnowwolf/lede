/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 *
 * (C) Nicolò Veronese <nicveronese@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>	// htonl

// Usage: mkdapimg2 -s signature [-v version] [-r region]
//                  [-k uImage block size] -i <input> -o <output>
//
// NOTE: The kernel block size is used to know the offset of the rootfs
// in the image file.
//
// The system writes in the uImage partition until the end of uImage
// is reached, after that, the system jumps to the offset specified with the -k
// parameter and begin writing at the beginning of the rootfs MTD partition.
//
// If the -k parameter is the size of the original uImage partition, the system
// continue writing in the rootfs partition starting from the last block
// that has been wrote. (This is useful if the new kernel size is
// different from the original one)
//
// Example:
// ------------------------------------------
// Creating 7 MTD partitions on "ath-nor0":
// 0x000000000000-0x000000010000 : "u-boot"
// 0x000000010000-0x000000020000 : "ART"
// 0x000000020000-0x000000030000 : "MP"
// 0x000000030000-0x000000040000 : "config"
// 0x000000040000-0x000000120000 : "uImage"
// 0x000000120000-0x000000800000 : "rootfs"
// 0x000000040000-0x000000800000 : "firmware"
// ------------------------------------------
//
// 0x000000120000-0x000000040000 = 0xE0000 -> 917504
//
// e.g.: mkdapimg2 -s HONEYBEE-FIRMWARE-DAP-1330 -v 1.00.21 -r Default
//                 -k 917504 -i sysupgarde.bin -o factory.bin
//
//
// The img_hdr_struct was taken from the D-Link SDK:
// DAP-1330_OSS-firmware_1.00b21/DAP-1330_OSS-firmware_1.00b21/uboot/uboot.patch

#define MAX_SIGN_LEN	32
#define MAX_FW_VER_LEN	16
#define MAX_REG_LEN	8

struct img_hdr_struct {
	uint32_t hdr_len;
	uint32_t checksum;
	uint32_t total_size;
	uint32_t kernel_size;
	char signature[MAX_SIGN_LEN];
	char fw_ver[MAX_FW_VER_LEN];
	char fw_reg[MAX_REG_LEN];
} imghdr ;

char *progname;

void
perrexit(int code, char *msg)
{
	fprintf(stderr, "%s: %s: %s\n", progname, msg, strerror(errno));
	exit(code);
}

void
usage()
{
	fprintf(stderr, "usage: %s -s signature [-v version] [-r region] [-k uImage part size] -i <input> -o <output>\n", progname);
	exit(1);
}

int
main(int ac, char *av[])
{
	char signature[MAX_SIGN_LEN];
	char version[MAX_FW_VER_LEN];
	char region[MAX_REG_LEN];
	int kernel = 0;

	FILE *ifile, *ofile;
	int c;

	uint32_t cksum;
	uint32_t bcnt;

	progname = basename(av[0]);

	memset(signature, 0, sizeof(signature));
	memset(version, 0, sizeof(version));
	memset(region, 0, sizeof(region));

	while ( 1 ) {
		char *ptr;
		int c;

		c = getopt(ac, av, "s:v:r:k:i:o:");
		if (c == -1)
			break;

		switch (c) {
		case 's':
			if (strlen(optarg) > MAX_SIGN_LEN + 1) {
				fprintf(stderr, "%s: signature exceeds %d chars\n",
					progname, MAX_SIGN_LEN);
				exit(1);
			}
			strcpy(signature, optarg);
			break;
		case 'v':
			if (strlen(optarg) > MAX_FW_VER_LEN + 1) {
				fprintf(stderr, "%s: version exceeds %d chars\n",
					progname, MAX_FW_VER_LEN);
				exit(1);
			}
			strcpy(version, optarg);
			break;
		case 'r':
			if (strlen(optarg) > MAX_REG_LEN + 1) {
				fprintf(stderr, "%s: region exceeds %d chars\n",
					progname, MAX_REG_LEN);
				exit(1);
			}
			strcpy(region, optarg);
			break;
		case 'k':
			kernel = strtoul(optarg, &ptr, 0);
			if(ptr[0] == 'k'){
				kernel *= 1000;
			}
			break;
		case 'i':
			if ((ifile = fopen(optarg, "r")) == NULL)
				perrexit(1, optarg);
			break;
		case 'o':
			if ((ofile = fopen(optarg, "w")) == NULL)
				perrexit(1, optarg);
			break;
		default:
			usage();
		}
	}

	if (signature[0] == 0 || ifile == NULL || ofile == NULL) {
		usage();
		exit(1);
	}

	for (bcnt = 0, cksum = 0 ; (c = fgetc(ifile)) != EOF ; bcnt++)
		cksum += c & 0xff;

	if (fseek(ifile, 0, SEEK_SET) < 0)
		perrexit(2, "fseek on input");

	// Fill in the header
	memset(&imghdr, 0, sizeof(imghdr));
	imghdr.hdr_len = sizeof(imghdr);
	imghdr.checksum = htonl(cksum);
	imghdr.total_size = htonl(bcnt);
	imghdr.kernel_size = htonl(kernel);

	strncpy(imghdr.signature, signature, MAX_SIGN_LEN);
	strncpy(imghdr.fw_ver, version, MAX_FW_VER_LEN);
	strncpy(imghdr.fw_reg, region, MAX_REG_LEN);

	if (fwrite(&imghdr, sizeof(imghdr), 1, ofile) < 0)
		perrexit(2, "fwrite header on output");

	while ((c = fgetc(ifile)) != EOF) {
		if (fputc(c, ofile) == EOF)
			perrexit(2, "fputc on output");
	}

	if (ferror(ifile))
		perrexit(2, "fgetc on input");

	fclose(ofile);
	fclose(ifile);

	fprintf(stderr, "imgHdr.hdr_len = %lu\n", sizeof(imghdr));
	fprintf(stderr, "imgHdr.checksum = 0x%08x\n", cksum);
	fprintf(stderr, "imgHdr.total_size = 0x%08x\n", bcnt);
	fprintf(stderr, "imgHdr.kernel_size = 0x%08x\n", kernel);
	fprintf(stderr, "imgHdr.header = %s\n", signature);
	fprintf(stderr, "imgHdr.fw_ver = %s\n", version);
	fprintf(stderr, "imgHdr.fw_reg = %s\n", region);

	return 0;
}
