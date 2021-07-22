/*
 * Copyright (C) 2007 Ubiquiti Networks, Inc.
 * Copyright (C) 2008 Lukas Kuna <ValXdater@seznam.cz>
 * Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <zlib.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "fw.h"

#undef VERSION
#define VERSION "1.2-OpenWrt.1"

#define MAX_SECTIONS		8
#define DEFAULT_OUTPUT_FILE 	"firmware-image.bin"
#define DEFAULT_VERSION		"UNKNOWN"
#define DEFAULT_FLASH_BASE	(0xbfc00000)

#define FIRMWARE_MAX_LENGTH	(0x390000)

typedef struct part_data {
	char 		partition_name[64];
	int  		partition_index;
	u_int32_t	partition_baseaddr;
	u_int32_t	partition_offset;
	u_int32_t	partition_memaddr;
	u_int32_t	partition_entryaddr;
	u_int32_t	partition_length;

	char		filename[PATH_MAX];
	struct stat	stats;
} part_data_t;

typedef struct image_info {
	char		version[256];
	char		outputfile[PATH_MAX];
	char		magic[MAGIC_LENGTH];
	u_int32_t	flash_baseaddr;
	u_int32_t	part_count;
	part_data_t	parts[MAX_SECTIONS];
} image_info_t;

static image_info_t im;
static int zero_part_baseaddr = 0;

static void write_header(void* mem, const char* version)
{
	header_t* header = mem;
	memset(header, 0, sizeof(header_t));

	memcpy(header->magic, im.magic, MAGIC_LENGTH);
	strncpy(header->version, version, sizeof(header->version));
	header->crc = htonl(crc32(0L, (unsigned char *)header,
				sizeof(header_t) - 2 * sizeof(u_int32_t)));
	header->pad = 0L;
}

static void write_signature(void* mem, u_int32_t sig_offset)
{
	/* write signature */
	signature_t* sign = (signature_t*)(mem + sig_offset);
	memset(sign, 0, sizeof(signature_t));

	memcpy(sign->magic, MAGIC_END, MAGIC_LENGTH);
	sign->crc = htonl(crc32(0L,(unsigned char *)mem, sig_offset));
	sign->pad = 0L;
}

static int write_part(void* mem, part_data_t* d)
{
	char* addr;
	int fd;
	part_t* p = mem;
	part_crc_t* crc = mem + sizeof(part_t) + d->stats.st_size;

	fd = open(d->filename, O_RDONLY);
	if (fd < 0) {
		ERROR("Failed opening file '%s'\n", d->filename);
		return -1;
	}

	if ((addr=(char*)mmap(0, d->stats.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
		ERROR("Failed mmaping memory for file '%s'\n", d->filename);
		close(fd);
		return -2;
	}

	memcpy(mem + sizeof(part_t), addr, d->stats.st_size);
	munmap(addr, d->stats.st_size);

	memset(p->name, 0, sizeof(p->name));
	strncpy(p->magic, MAGIC_PART, MAGIC_LENGTH);
	strncpy(p->name, d->partition_name, sizeof(p->name));
	p->index = htonl(d->partition_index);
	p->data_size = htonl(d->stats.st_size);
	p->part_size = htonl(d->partition_length);
	p->baseaddr = htonl(d->partition_baseaddr);
	p->memaddr = htonl(d->partition_memaddr);
	p->entryaddr = htonl(d->partition_entryaddr);

	crc->crc = htonl(crc32(0L, mem, d->stats.st_size + sizeof(part_t)));
	crc->pad = 0L;

	return 0;
}

static void usage(const char* progname)
{
	INFO("Version %s\n"
             "Usage: %s [options]\n"
	     "\t-v <version string>\t - firmware version information, default: %s\n"
	     "\t-m <magic>\t\t - firmware magic, default: %s\n"
	     "\t-f <flash base>\t\t - flash base address, default: 0x%08x\n"
	     "\t-o <output file>\t - firmware output file, default: %s\n"
	     "\t-p <name>:<offset>:<len>:<memaddr>:<entry>:<file>\n "
	     "\t\t\t\t - create a partition from <file>\n"
	     "\t-z\t\t\t - set partition offsets to zero\n"
	     "\t-h\t\t\t - this help\n",
	     VERSION, progname, DEFAULT_VERSION, MAGIC_HEADER,
	     DEFAULT_FLASH_BASE, DEFAULT_OUTPUT_FILE);
}

static void print_image_info(void)
{
	int i;

	INFO("Firmware version : '%s'\n"
	     "Output file      : '%s'\n"
	     "Part count       : %u\n",
	     im.version, im.outputfile, im.part_count);

	for (i = 0; i < im.part_count; ++i) {
		const part_data_t* d = &im.parts[i];
		INFO("  %10s: %08x %08x %08x %08x %8ld bytes (free: %8ld)\n",
		     d->partition_name,
		     d->partition_baseaddr,
		     d->partition_length,
		     d->partition_entryaddr,
		     d->partition_memaddr,
		     d->stats.st_size,
		     d->partition_length - d->stats.st_size);
	}
}

static int filelength(const char* file)
{
	FILE *p;
	int ret = -1;

	if ( (p = fopen(file, "rb") ) == NULL) return (-1);

	fseek(p, 0, SEEK_END);
	ret = ftell(p);

	fclose (p);

	return (ret);
}

int str2u32(char *arg, u_int32_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno = 0;
	t = strtoul(arg, &err, 0);
	if (errno || (err == arg) || ((err != NULL) && *err)) {
		return -1;
	}

	*val = t;
	return 0;
}

#ifndef STRINGIFY
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)
#endif
static int image_layout_add_partition(const char *part_desc)
{
	part_data_t *d;
	char memaddr[16];
	char entryaddr[16];
	char offset[16];
	char length[16];
	int t;

	if (im.part_count >= MAX_SECTIONS) {
		ERROR("Too many partitions specified\n");
		return (-1);
	}

	d = &im.parts[im.part_count];
	t = sscanf(part_desc, "%15[-0-9a-zA-Z]:%15[0-9a-fA-Fx]:%15[0-9a-fA-Fx]:%15[0-9a-fA-Fx]:%15[0-9a-fA-Fx]:%"STRINGIFY(PATH_MAX)"s",
			d->partition_name,
			offset,
			length,
			memaddr,
			entryaddr,
			d->filename);

	if (t != 6) {
		ERROR("Bad partition parameter %d, '%s'\n", t, part_desc);
		return (-1);
	}

	if (strlen(d->partition_name) == 0) {
		ERROR("No partition name specified in '%s'\n", part_desc);
		return (-1);
	}

	if (str2u32(offset, &d->partition_offset)) {
		ERROR("Bad offset value '%s'\n", offset);
		return (-1);
	}

	if (str2u32(length, &d->partition_length)) {
		ERROR("Bad length value '%s'\n", length);
		return (-1);
	}

	if (d->partition_length == 0) {
		int flen;
		flen = filelength(d->filename);
		if (flen < 0) {
			ERROR("Unable to determine size of '%s'\n",
					d->filename);
			return (-1);
		}
		d->partition_length = flen;
	}

	if (str2u32(memaddr, &d->partition_memaddr)) {
		ERROR("Bad memaddr vaule '%s'\n", memaddr);
		return (-1);
	}

	if (str2u32(entryaddr, &d->partition_entryaddr)) {
		ERROR("Bad entry address value '%s'\n", entryaddr);
		return (-1);
	}

	im.part_count++;
	d->partition_index = im.part_count;

	return 0;
}

static int image_layout_verify(void)
{
	u_int32_t offset;
	int i;

	if (im.part_count == 0) {
		ERROR("No partitions specified\n");
		return -1;
	}

	offset = im.parts[0].partition_offset;
	for (i = 0; i < im.part_count; i++)
	{
		part_data_t* d = &im.parts[i];

		if (stat(d->filename, &d->stats) < 0) {
			ERROR("Couldn't stat file '%s' from part '%s'\n",
					d->filename, d->partition_name);
			return -2;
		}

		if (d->stats.st_size == 0) {
			ERROR("File '%s' from part '%s' is empty!\n",
					d->filename, d->partition_name);
			return -3;
		}

		if (d->stats.st_size > d->partition_length) {
			ERROR("File '%s' too big (%d) - max size: 0x%08X (exceeds %lu bytes)\n",
				d->filename, i, d->partition_length,
				d->stats.st_size - d->partition_length);
			return -4;
		}

		if (d->partition_offset < offset)
			d->partition_offset = offset;

		if (zero_part_baseaddr) {
			d->partition_baseaddr = 0;
		} else {
			d->partition_baseaddr =
				im.flash_baseaddr + d->partition_offset;
		}
		offset += d->partition_length;
	}

	return 0;
}

static int build_image(void)
{
	char* mem;
	char* ptr;
	u_int32_t mem_size;
	FILE* f;
	int i;

	/* build in-memory buffer */
	mem_size = sizeof(header_t) + sizeof(signature_t);
	for (i = 0; i < im.part_count; ++i) {
		part_data_t* d = &im.parts[i];
		mem_size += sizeof(part_t) + d->stats.st_size + sizeof(part_crc_t);
	}

	mem = (char*)calloc(mem_size, 1);
	if (mem == NULL) {
		ERROR("Cannot allocate memory chunk of size '%u'\n", mem_size);
		return -1;
	}

	/* write header */
	write_header(mem, im.version);
	ptr = mem + sizeof(header_t);

	/* write all parts */
	for (i = 0; i < im.part_count; ++i) {
		part_data_t* d = &im.parts[i];
		int rc;
		if ((rc = write_part(ptr, d)) != 0) {
			ERROR("ERROR: failed writing part %u '%s'\n", i, d->partition_name);
			return -1;
		}
		ptr += sizeof(part_t) + d->stats.st_size + sizeof(part_crc_t);
	}


	/* write signature */
	write_signature(mem, mem_size - sizeof(signature_t));

	/* write in-memory buffer into file */
	if ((f = fopen(im.outputfile, "w")) == NULL) {
		ERROR("Can not create output file: '%s'\n", im.outputfile);
		free(mem);
		return -10;
	}

	if (fwrite(mem, mem_size, 1, f) != 1) {
		ERROR("Could not write %d bytes into file: '%s'\n",
				mem_size, im.outputfile);
		free(mem);
		fclose(f);
		return -11;
	}

	free(mem);
	fclose(f);
	return 0;
}

int main(int argc, char* argv[])
{
	int o, rc;

	memset(&im, 0, sizeof(im));

	strcpy(im.outputfile, DEFAULT_OUTPUT_FILE);
	strcpy(im.version, DEFAULT_VERSION);
	memcpy(im.magic, MAGIC_HEADER, MAGIC_LENGTH);
	im.flash_baseaddr = DEFAULT_FLASH_BASE;

	while ((o = getopt(argc, argv, "f:hm:o:p:v:z")) != -1)
	{
		switch (o) {
		case 'f':
			if (optarg)
				if (str2u32(optarg, &im.flash_baseaddr)) {
					ERROR("Invalid flash start address %s\n", optarg);
					return -1;
				}
			break;
		case 'h':
			usage(argv[0]);
			return -1;
		case 'm':
			if (optarg) {
				if (strlen(optarg) != MAGIC_LENGTH) {
					ERROR("Invalid magic %s\n", optarg);
					return -1;
				}

				memcpy(im.magic, optarg, MAGIC_LENGTH);
			}
			break;
		case 'o':
			if (optarg)
				strncpy(im.outputfile, optarg, sizeof(im.outputfile));
			break;
		case 'p':
			if (optarg) {
				if (image_layout_add_partition(optarg))
					return -1;
			}
			break;
		case 'v':
			if (optarg)
				strncpy(im.version, optarg, sizeof(im.version));
			break;
		case 'z':
			zero_part_baseaddr = 1;
			break;
		}
	}

	rc = image_layout_verify();
	if (rc)	{
		ERROR("Failed validating firmware layout - error code: %d\n",
				rc);
		return -4;
	}

	print_image_info();

	rc = build_image();
	if (rc)	{
		ERROR("Failed building image file '%s' - error code: %d\n",
				im.outputfile, rc);
		return -5;
	}

	return 0;
}
