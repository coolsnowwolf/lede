/*
 * mkdlinkfw
 *
 * Copyright (C) 2018 Paweł Dembicki <paweldembicki@gmail.com>
 *
 * This tool is based on mktplinkfw.
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
#include <zlib.h>		/*for crc32 */

#include "mkdlinkfw-lib.h"

/* ARM update header 2.0
 * used only in factory images to erase and flash selected area
 */
struct auh_header {
	uint8_t rom_id[12];	/* 12-bit rom-id unique per router type */
	uint16_t derange;	/* used for scramble header */
	uint16_t image_checksum;	/* jboot_checksum of flashed data */

	uint32_t space1;	/* zeros */
	uint32_t space2;	/* zeros */
	uint16_t space3;	/* zerosu */
	uint8_t lpvs;		/* must be 0x01 */
	uint8_t mbz;		/* bust be 0 */
	uint32_t time_stamp;	/* timestamp calculated in jboot way */

	uint32_t erase_start;	/* erase start address */
	uint32_t erase_length;	/* erase length address */
	uint32_t data_offset;	/* data start address */
	uint32_t data_length;	/* data length address */

	uint32_t space4;	/* zeros */
	uint32_t space5;	/* zeros */
	uint32_t space6;	/* zeros */
	uint32_t space7;	/* zeros */

	uint16_t header_id;	/* magic 0x4842 */
	uint16_t header_version;	/* 0x02 for 2.0 */
	uint16_t space8;	/* zeros */
	uint8_t section_id;	/* section id */
	uint8_t image_info_type;	/* (?) 0x04 in factory images */
	uint32_t image_info_offset;	/* (?) zeros in factory images */
	uint16_t family_member;	/* unique per router type */
	uint16_t header_checksum;	/* negated jboot_checksum of header data */
};

struct stag_header {		/* used only of sch2 wrapped kernel data */
	uint8_t cmark;		/* in factory 0xFF ,in sysuograde must be the same as id */
	uint8_t id;		/* 0x04 */
	uint16_t magic;		/* magic 0x2B24 */
	uint32_t time_stamp;	/* timestamp calculated in jboot way */
	uint32_t image_length;	/* lentgh of kernel + sch2 header */
	uint16_t image_checksum;	/* negated jboot_checksum of sch2 + kernel */
	uint16_t tag_checksum;	/* negated jboot_checksum of stag header data */
};

struct sch2_header {		/* used only in kernel partitions */
	uint16_t magic;		/* magic 0x2124 */
	uint8_t cp_type;	/* 0x00 for flat, 0x01 for jz, 0x02 for gzip, 0x03 for lzma */
	uint8_t version;	/* 0x02 for sch2 */
	uint32_t ram_addr;	/* ram entry address */
	uint32_t image_len;	/* kernel image length */
	uint32_t image_crc32;	/* kernel image crc */
	uint32_t start_addr;	/* ram start address */
	uint32_t rootfs_addr;	/* rootfs flash address */
	uint32_t rootfs_len;	/* rootfls length */
	uint32_t rootfs_crc32;	/* rootfs crc32 */
	uint32_t header_crc32;	/* sch2 header crc32, durring calculation this area is replaced by zero */
	uint16_t header_length;	/* sch2 header length: 0x28 */
	uint16_t cmd_line_length;	/* cmd line length, known zeros */
};

/* globals */
static struct file_info inspect_info;
struct file_info kernel_info;
struct file_info rootfs_info;
struct file_info image_info;

char *ofname;
char *progname;
uint32_t firmware_size;
uint32_t image_offset;
uint16_t family_member;
char *rom_id[12] = { 0 };
char image_type;

static void usage(int status)
{
	fprintf(stderr, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stderr,
		"\n"
		"Options:\n"
		"  -i <file>       inspect given firmware file <file>\n"
		"  -f              set family member id (hexval prefixed with 0x)\n"
		"  -F <file>       read image and convert it to FACTORY\n"
		"  -k <file>       read kernel image from the file <file>\n"
		"  -r <file>       read rootfs image from the file <file>\n"
		"  -o <file>       write output to the file <file>\n"
		"  -s <size>       set firmware partition size\n"
		"  -m <version>    set rom id to <version> (12-bit string val: \"DLK*********\")\n"
		"  -h              show this screen\n");

	exit(status);
}

void print_auh_header(struct auh_header *printed_header)
{
	printf("\trom_id: %s\n"
	       "\tderange: 0x%04X\n"
	       "\timage_checksum: 0x%04X\n"
	       "\tspace1: 0x%08X\n"
	       "\tspace2: 0x%08X\n"
	       "\tspace3: 0x%04X\n"
	       "\tlpvs: 0x%02X\n"
	       "\tmbz: 0x%02X\n"
	       "\ttime_stamp: 0x%08X\n"
	       "\terase_start: 0x%08X\n"
	       "\terase_length: 0x%08X\n"
	       "\tdata_offset: 0x%08X\n"
	       "\tdata_length: 0x%08X\n"
	       "\tspace4: 0x%08X\n"
	       "\tspace5: 0x%08X\n"
	       "\tspace6: 0x%08X\n"
	       "\tspace7: 0x%08X\n"
	       "\theader_id: 0x%04X\n"
	       "\theader_version: 0x%02X\n"
	       "\tspace8: 0x%04X\n"
	       "\tsection_id: 0x%02X\n"
	       "\timage_info_type: 0x%02X\n"
	       "\timage_info_offset 0x%08X\n"
	       "\tfamily_member: 0x%04X\n"
	       "\theader_checksum: 0x%04X\n",
	       printed_header->rom_id,
	       printed_header->derange,
	       printed_header->image_checksum,
	       printed_header->space1,
	       printed_header->space2,
	       printed_header->space3,
	       printed_header->lpvs,
	       printed_header->mbz,
	       printed_header->time_stamp,
	       printed_header->erase_start,
	       printed_header->erase_length,
	       printed_header->data_offset,
	       printed_header->data_length,
	       printed_header->space4,
	       printed_header->space5,
	       printed_header->space6,
	       printed_header->space7,
	       printed_header->header_id,
	       printed_header->header_version,
	       printed_header->space8,
	       printed_header->section_id,
	       printed_header->image_info_type,
	       printed_header->image_info_offset,
	       printed_header->family_member, printed_header->header_checksum);
}

void print_stag_header(struct stag_header *printed_header)
{
	printf("\tcmark: 0x%02X\n"
	       "\tid: 0x%02X\n"
	       "\tmagic: 0x%04X\n"
	       "\ttime_stamp: 0x%08X\n"
	       "\timage_length: 0x%04X\n"
	       "\timage_checksum: 0x%04X\n"
	       "\ttag_checksum: 0x%04X\n",
	       printed_header->cmark,
	       printed_header->id,
	       printed_header->magic,
	       printed_header->time_stamp,
	       printed_header->image_length,
	       printed_header->image_checksum, printed_header->tag_checksum);
}

void print_sch2_header(struct sch2_header *printed_header)
{
	printf("\tmagic: 0x%04X\n"
	       "\tcp_type: 0x%02X\n"
	       "\tversion: 0x%02X\n"
	       "\tram_addr: 0x%08X\n"
	       "\timage_len: 0x%08X\n"
	       "\timage_crc32: 0x%08X\n"
	       "\tstart_addr: 0x%08X\n"
	       "\trootfs_addr: 0x%08X\n"
	       "\trootfs_len: 0x%08X\n"
	       "\trootfs_crc32: 0x%08X\n"
	       "\theader_crc32: 0x%08X\n"
	       "\theader_length: 0x%04X\n"
	       "\tcmd_line_length: 0x%04X\n",
	       printed_header->magic,
	       printed_header->cp_type,
	       printed_header->version,
	       printed_header->ram_addr,
	       printed_header->image_len,
	       printed_header->image_crc32,
	       printed_header->start_addr,
	       printed_header->rootfs_addr,
	       printed_header->rootfs_len,
	       printed_header->rootfs_crc32,
	       printed_header->header_crc32,
	       printed_header->header_length, printed_header->cmd_line_length);
}

static int find_auh_headers(char *buf)
{
	char *tmp_buf = buf;
	struct auh_header *tmp_header[MAX_HEADER_COUNTER];
	int header_counter = 0;

	int ret = EXIT_FAILURE;

	while (tmp_buf - buf <= inspect_info.file_size - AUH_SIZE) {
		if (!memcmp(tmp_buf, AUH_MAGIC, 3)) {
			if (((struct auh_header *)tmp_buf)->header_checksum ==
			    (uint16_t) ~jboot_checksum(0, (uint16_t *) tmp_buf,
							AUH_SIZE - 2)) {
				uint16_t checksum = 0;
				printf("Find proper AUH header at: 0x%lX!\n",
				       tmp_buf - buf);
				tmp_header[header_counter] =
				    (struct auh_header *)tmp_buf;
				checksum =
				    jboot_checksum(0, (uint16_t *) ((char *)
								    tmp_header
								    [header_counter]
								    + AUH_SIZE),
						   tmp_header
						   [header_counter]->data_length);
				if (tmp_header[header_counter]->image_checksum
				    == checksum)
					printf("Image checksum ok.\n");
				else
					ERR("Image checksum incorrect! Stored: 0x%X Calculated: 0x%X\n", tmp_header[header_counter]->image_checksum, checksum);
				header_counter++;
				if (header_counter > MAX_HEADER_COUNTER)
					break;
			}
		}
		tmp_buf++;
	}

	if (header_counter == 0)
		ERR("Can't find proper AUH header!\n");
	else if (header_counter > MAX_HEADER_COUNTER)
		ERR("To many AUH headers!\n");
	else {
		for (int i = 0; i < header_counter; i++) {
			printf("AUH %d:\n", i);
			print_auh_header(tmp_header[i]);
		}

		ret = EXIT_SUCCESS;
	}

	return ret;
}

static int check_stag_header(char *buf, struct stag_header *header)
{

	int ret = EXIT_FAILURE;

	uint8_t cmark_tmp = header->cmark;
	header->cmark = header->id;

	if (header->tag_checksum ==
	    (uint16_t) ~jboot_checksum(0, (uint16_t *) header,
					STAG_SIZE - 2)) {
		uint16_t checksum = 0;
		printf("Find proper STAG header at: 0x%lX!\n",
		       (char *)header - buf);
		checksum =
		    jboot_checksum(0, (uint16_t *) ((char *)header + STAG_SIZE),
				   header->image_length);
		if (header->image_checksum == checksum) {
			printf("Image checksum ok.\n");
			header->cmark = cmark_tmp;
			print_stag_header(header);
			ret = EXIT_SUCCESS;
		} else
			ERR("Image checksum incorrect! Stored: 0x%X Calculated: 0x%X\n", header->image_checksum, checksum);
	} else
		ERR("STAG header checksum incorrect!");

	header->cmark = cmark_tmp;
	return ret;
}

static int check_sch2_header(char *buf, struct sch2_header *header)
{

	int ret = EXIT_FAILURE;

	uint32_t crc32_tmp = header->header_crc32;
	header->header_crc32 = 0;

	if (crc32_tmp == crc32(0, (uint8_t *) header, header->header_length)) {
		uint32_t crc32_val;
		printf("Find proper SCH2 header at: 0x%lX!\n",
		       (char *)header - buf);

		crc32_val =
		    crc32(0, (uint8_t *) header + header->header_length,
			  header->image_len);
		if (header->image_crc32 == crc32_val) {
			printf("Kernel checksum ok.\n");

			header->header_crc32 = crc32_tmp;
			print_sch2_header(header);
			ret = EXIT_SUCCESS;
		} else
			ERR("Kernel checksum incorrect! Stored: 0x%X Calculated: 0x%X\n", header->image_crc32, crc32_val);

	} else
		ERR("SCH2 header checksum incorrect!");

	header->header_crc32 = crc32_tmp;
	return ret;
}

static int inspect_fw(void)
{
	char *buf;
	struct stag_header *stag_header_kernel;
	struct sch2_header *sch2_header_kernel;
	int ret = EXIT_FAILURE;

	buf = malloc(inspect_info.file_size);
	if (!buf) {
		ERR("no memory for buffer!\n");
		goto out;
	}

	ret = read_to_buf(&inspect_info, buf);
	if (ret)
		goto out_free_buf;

	ret = find_auh_headers(buf);
	if (ret)
		goto out_free_buf;

	stag_header_kernel = (struct stag_header *)(buf + AUH_SIZE);

	ret = check_stag_header(buf, stag_header_kernel);
	if (ret)
		goto out_free_buf;

	sch2_header_kernel = (struct sch2_header *)(buf + AUH_SIZE + STAG_SIZE);

	ret = check_sch2_header(buf, sch2_header_kernel);
	if (ret)
		goto out_free_buf;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

static int check_options(void)
{
	int ret;

	if (inspect_info.file_name) {
		ret = get_file_stat(&inspect_info);
		if (ret)
			return ret;

		return 0;
	}

	return 0;
}

int fill_sch2(struct sch2_header *header, char *kernel_ptr, char *rootfs_ptr)
{

	header->magic = SCH2_MAGIC;
	header->cp_type = LZMA;
	header->version = SCH2_VER;
	header->ram_addr = RAM_LOAD_ADDR;
	header->image_len = kernel_info.file_size;
	header->image_crc32 = crc32(0, (uint8_t *) kernel_ptr, kernel_info.file_size);
	header->start_addr = RAM_ENTRY_ADDR;
	header->rootfs_addr =
	    image_offset + STAG_SIZE + SCH2_SIZE + kernel_info.file_size;
	header->rootfs_len = rootfs_info.file_size;
	header->rootfs_crc32 = crc32(0, (uint8_t *) rootfs_ptr, rootfs_info.file_size);
	header->header_crc32 = 0;
	header->header_length = SCH2_SIZE;
	header->cmd_line_length = 0;

	header->header_crc32 = crc32(0, (uint8_t *) header, header->header_length);

	return EXIT_SUCCESS;
}

int fill_stag(struct stag_header *header, uint32_t length)
{
	header->cmark = STAG_ID;
	header->id = STAG_ID;
	header->magic = STAG_MAGIC;
	header->time_stamp = jboot_timestamp();
	header->image_length = length + SCH2_SIZE;
	header->image_checksum =
	    jboot_checksum(0, (uint16_t *) ((char *)header + STAG_SIZE),
			   header->image_length);
	header->tag_checksum =
	    ~jboot_checksum(0, (uint16_t *) header, STAG_SIZE - 2);

	if (image_type == FACTORY)
		header->cmark = STAG_CMARK_FACTORY;

	return EXIT_SUCCESS;
};

int fill_auh(struct auh_header *header, uint32_t length)
{
	memcpy(header->rom_id, rom_id, 12);
	header->derange = 0;
	header->image_checksum =
	    jboot_checksum(0, (uint16_t *) ((char *)header + AUH_SIZE), length);
	header->space1 = 0;
	header->space2 = 0;
	header->space3 = 0;
	header->lpvs = AUH_LVPS;
	header->mbz = 0;
	header->time_stamp = jboot_timestamp();
	header->erase_start = image_offset;
	header->erase_length = firmware_size;
	header->data_offset = image_offset;
	header->data_length = length;
	header->space4 = 0;
	header->space5 = 0;
	header->space6 = 0;
	header->space7 = 0;
	header->header_id = AUH_HDR_ID;
	header->header_version = AUH_HDR_VER;
	header->space8 = 0;
	header->section_id = AUH_SEC_ID;
	header->image_info_type = AUH_INFO_TYPE;
	header->image_info_offset = 0;
	header->family_member = family_member;
	header->header_checksum =
	    ~jboot_checksum(0, (uint16_t *) header, AUH_SIZE - 2);

	return EXIT_SUCCESS;
}

int build_fw(void)
{
	char *buf;
	char *kernel_ptr;
	char *rootfs_ptr;
	int ret = EXIT_FAILURE;
	int writelen;

	struct stag_header *stag_header_kernel;
	struct sch2_header *sch2_header_kernel;

	if (!kernel_info.file_name | !rootfs_info.file_name)
		goto out;

	ret = get_file_stat(&kernel_info);
	if (ret)
		goto out;
	ret = get_file_stat(&rootfs_info);
	if (ret)
		goto out;

	buf = malloc(firmware_size);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	if (rootfs_info.file_size + kernel_info.file_size + ALL_HEADERS_SIZE >
	    firmware_size) {
		ERR("data is bigger than firmware_size!\n");
		goto out;
	}

	memset(buf, 0xff, firmware_size);

	stag_header_kernel = (struct stag_header *)buf;

	sch2_header_kernel =
	    (struct sch2_header *)((char *)stag_header_kernel + STAG_SIZE);
	kernel_ptr = (char *)sch2_header_kernel + SCH2_SIZE;

	ret = read_to_buf(&kernel_info, kernel_ptr);
	if (ret)
		goto out_free_buf;

	rootfs_ptr = kernel_ptr + kernel_info.file_size;

	ret = read_to_buf(&rootfs_info, rootfs_ptr);
	if (ret)
		goto out_free_buf;

	writelen = rootfs_ptr + rootfs_info.file_size - buf;

	fill_sch2(sch2_header_kernel, kernel_ptr, rootfs_ptr);
	fill_stag(stag_header_kernel, kernel_info.file_size);

	ret = write_fw(ofname, buf, writelen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

int wrap_fw(void)
{
	char *buf;
	char *image_ptr;
	int ret = EXIT_FAILURE;
	int writelen;

	struct auh_header *auh_header_kernel;

	if (!image_info.file_name)
		goto out;

	ret = get_file_stat(&image_info);
	if (ret)
		goto out;

	buf = malloc(firmware_size);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	if (image_info.file_size + AUH_SIZE >
	    firmware_size) {
		ERR("data is bigger than firmware_size!\n");
		goto out;
	}
	if (!family_member) {
		ERR("No family_member!\n");
		goto out;
	}
	if (!(rom_id[0])) {
		ERR("No rom_id!\n");
		goto out;
	}
	memset(buf, 0xff, firmware_size);

	image_ptr = (char *)(buf + AUH_SIZE);

	ret = read_to_buf(&image_info, image_ptr);
	if (ret)
		goto out_free_buf;

	writelen = image_ptr + image_info.file_size - buf;

	auh_header_kernel = (struct auh_header *)buf;
	fill_auh(auh_header_kernel, writelen - AUH_SIZE);

	ret = write_fw(ofname, buf, writelen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	progname = basename(argv[0]);
	image_type = SYSUPGRADE;
	family_member = 0;
	firmware_size = 0;
	image_offset = JBOOT_SIZE;

	while (1) {
		int c;

		c = getopt(argc, argv, "f:F:i:hk:m:o:O:r:s:");
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			sscanf(optarg, "0x%hx", &family_member);
			break;
		case 'F':
			image_info.file_name = optarg;
			image_type = FACTORY;
			break;
		case 'i':
			inspect_info.file_name = optarg;
			break;
		case 'k':
			kernel_info.file_name = optarg;
			break;
		case 'm':
			if (strlen(optarg) == 12)
				memcpy(rom_id, optarg, 12);
			break;
		case 'r':
			rootfs_info.file_name = optarg;
			break;
		case 'O':
			sscanf(optarg, "0x%x", &image_offset);
			break;
		case 'o':
			ofname = optarg;
			break;
		case 's':
			sscanf(optarg, "0x%x", &firmware_size);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	ret = check_options();
	if (ret)
		goto out;

	if (!inspect_info.file_name) {
		if (image_type == FACTORY)
			ret = wrap_fw();
		else
			ret = build_fw();
		}
	else
		ret = inspect_fw();

 out:
	return ret;

}
