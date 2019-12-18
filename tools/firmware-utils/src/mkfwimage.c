/*
 * Copyright (C) 2007 Ubiquiti Networks, Inc.
 * Copyright (C) 2008 Lukas Kuna <ValXdater@seznam.cz>
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
#include <inttypes.h>
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
#include <stdbool.h>
#include "fw.h"

typedef struct fw_layout_data {
	u_int32_t	kern_start;
	u_int32_t	kern_entry;
	u_int32_t	firmware_max_length;
} fw_layout_t;

struct fw_info {
	char			name[PATH_MAX];
	struct fw_layout_data	fw_layout;
	bool			sign;
};

struct fw_info fw_info[] = {
	{
		.name = "XS2",
		.fw_layout = {
			.kern_start	=	0xbfc30000,
			.kern_entry	=	0x80041000,
			.firmware_max_length=	0x00390000,
		},
		.sign = false,
	},
	{
		.name = "XS5",
		.fw_layout = {
			.kern_start	=	0xbe030000,
			.kern_entry	=	0x80041000,
			.firmware_max_length=	0x00390000,
		},
		.sign = false,
	},
	{
		.name = "RS",
		.fw_layout = {
			.kern_start	=	0xbf030000,
			.kern_entry	=	0x80060000,
			.firmware_max_length=	0x00B00000,
		},
		.sign = false,
	},
	{
		.name = "RSPRO",
		.fw_layout = {
			.kern_start	=	0xbf030000,
			.kern_entry	=	0x80060000,
			.firmware_max_length=	0x00F00000,
		},
		.sign = false,
	},
	{
		.name = "LS-SR71",
		.fw_layout = {
			.kern_start	=	0xbf030000,
			.kern_entry	=	0x80060000,
			.firmware_max_length=	0x00640000,
		},
		.sign = false,
	},
	{
		.name = "XS2-8",
		.fw_layout = {
			.kern_start	=	0xa8030000,
			.kern_entry	=	0x80041000,
			.firmware_max_length=	0x006C0000,
		},
		.sign = false,

	},
	{
		.name = "XM",
		.fw_layout = {
			.kern_start	=	0x9f050000,
			.kern_entry	=	0x80002000,
			.firmware_max_length=	0x00760000,
		},
		.sign = false,
	},
	{
		.name = "UBDEV01",
		.fw_layout = {
			.kern_start	=	0x9f050000,
			.kern_entry	=	0x80002000,
			.firmware_max_length=	0x006A0000,
		},
		.sign = false,
	},
	{
		.name = "WA",
		.fw_layout = {
			.kern_start	=	0x9f050000,
			.kern_entry	=	0x80002000,
			.firmware_max_length=	0x00F60000,
		},
		.sign = true,
	},
	{
		.name = "",
	},
};

typedef struct part_data {
	char 	partition_name[64];
	int  	partition_index;
	u_int32_t	partition_baseaddr;
	u_int32_t	partition_startaddr;
	u_int32_t	partition_memaddr;
	u_int32_t	partition_entryaddr;
	u_int32_t  partition_length;

	char	filename[PATH_MAX];
	struct stat stats;
} part_data_t;

#define MAX_SECTIONS	8
#define DEFAULT_OUTPUT_FILE 	"firmware-image.bin"
#define DEFAULT_VERSION		"UNKNOWN"

#define OPTIONS "B:hv:m:o:r:k:"

typedef struct image_info {
	char magic[16];
	char version[256];
	char outputfile[PATH_MAX];
	u_int32_t	part_count;
	part_data_t parts[MAX_SECTIONS];
	struct fw_info* fwinfo;
} image_info_t;

static struct fw_info* get_fwinfo(char* board_name) {
	struct fw_info *fwinfo = fw_info;
	while(strlen(fwinfo->name)) {
		if(strcmp(fwinfo->name, board_name) == 0) {
			return fwinfo;
		}
		fwinfo++;
	}
	return NULL;
}

static void write_header(void* mem, const char *magic, const char* version)
{
	header_t* header = mem;
	memset(header, 0, sizeof(header_t));

	memcpy(header->magic, magic, MAGIC_LENGTH);
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

static void write_signature_rsa(void* mem, u_int32_t sig_offset)
{
	/* write signature */
	signature_rsa_t* sign = (signature_rsa_t*)(mem + sig_offset);
	memset(sign, 0, sizeof(signature_rsa_t));

	memcpy(sign->magic, MAGIC_ENDS, MAGIC_LENGTH);
//	sign->crc = htonl(crc32(0L,(unsigned char *)mem, sig_offset));
	sign->pad = 0L;
}

static int write_part(void* mem, part_data_t* d)
{
	char* addr;
	int fd;
	part_t* p = mem;
	part_crc_t* crc = mem + sizeof(part_t) + d->stats.st_size;

	fd = open(d->filename, O_RDONLY);
	if (fd < 0)
	{
		ERROR("Failed opening file '%s'\n", d->filename);
		return -1;
	}

	if ((addr=(char*)mmap(0, d->stats.st_size, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
	{
		ERROR("Failed mmaping memory for file '%s'\n", d->filename);
		close(fd);
		return -2;
	}

	memcpy(mem + sizeof(part_t), addr, d->stats.st_size);
	munmap(addr, d->stats.st_size);

	memset(p->name, 0, PART_NAME_LENGTH);
	memcpy(p->magic, MAGIC_PART, MAGIC_LENGTH);
	memcpy(p->name, d->partition_name, PART_NAME_LENGTH);

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
	     "\t-o <output file>\t - firmware output file, default: %s\n"
	     "\t-m <magic>\t - firmware magic, default: %s\n"
	     "\t-k <kernel file>\t\t - kernel file\n"
	     "\t-r <rootfs file>\t\t - rootfs file\n"
	     "\t-B <board name>\t\t - choose firmware layout for specified board (XS2, XS5, RS, XM)\n"
	     "\t-h\t\t\t - this help\n", VERSION,
	     progname, DEFAULT_VERSION, DEFAULT_OUTPUT_FILE, MAGIC_HEADER);
}

static void print_image_info(const image_info_t* im)
{
	unsigned int i = 0;

	INFO("Firmware version: '%s'\n"
	     "Output file: '%s'\n"
	     "Part count: %u\n",
	     im->version, im->outputfile,
	     im->part_count);

	for (i = 0; i < im->part_count; ++i)
	{
		const part_data_t* d = &im->parts[i];
		INFO(" %10s: %8" PRId64 " bytes (free: %8" PRId64 ")\n",
		     d->partition_name,
		     d->stats.st_size,
		     d->partition_length - d->stats.st_size);
	}
}

static u_int32_t filelength(const char* file)
{
	FILE *p;
	int ret = -1;

	if ( (p = fopen(file, "rb") ) == NULL) return (-1);

	fseek(p, 0, SEEK_END);
	ret = ftell(p);

	fclose (p);

	return (ret);
}

static int create_image_layout(const char* kernelfile, const char* rootfsfile, image_info_t* im)
{
	uint32_t rootfs_len = 0;
	part_data_t* kernel = &im->parts[0];
	part_data_t* rootfs = &im->parts[1];

	fw_layout_t* p = &im->fwinfo->fw_layout;

	printf("board = %s\n", im->fwinfo->name);
	strcpy(kernel->partition_name, "kernel");
	kernel->partition_index = 1;
	kernel->partition_baseaddr = p->kern_start;
	if ( (kernel->partition_length = filelength(kernelfile)) == (u_int32_t)-1) return (-1);
	kernel->partition_memaddr = p->kern_entry;
	kernel->partition_entryaddr = p->kern_entry;
	strncpy(kernel->filename, kernelfile, sizeof(kernel->filename));

	rootfs_len = filelength(rootfsfile);
	if (rootfs_len + kernel->partition_length > p->firmware_max_length) {
		ERROR("File '%s' too big (0x%08X) - max size: 0x%08X (exceeds %u bytes)\n",
		       rootfsfile, rootfs_len, p->firmware_max_length,
		       (rootfs_len + kernel->partition_length) - p->firmware_max_length);
		return (-2);
	}

	strcpy(rootfs->partition_name, "rootfs");
	rootfs->partition_index = 2;
	rootfs->partition_baseaddr = kernel->partition_baseaddr + kernel->partition_length;
	rootfs->partition_length = p->firmware_max_length - kernel->partition_length;
	rootfs->partition_memaddr = 0x00000000;
	rootfs->partition_entryaddr = 0x00000000;
	strncpy(rootfs->filename, rootfsfile, sizeof(rootfs->filename));

	printf("kernel: %d 0x%08x\n", kernel->partition_length, kernel->partition_baseaddr);
	printf("root: %d 0x%08x\n", rootfs->partition_length, rootfs->partition_baseaddr);
	im->part_count = 2;

	return 0;
}

/**
 * Checks the availability and validity of all image components.
 * Fills in stats member of the part_data structure.
 */
static int validate_image_layout(image_info_t* im)
{
	unsigned int i;

	if (im->part_count == 0 || im->part_count > MAX_SECTIONS)
	{
		ERROR("Invalid part count '%d'\n", im->part_count);
		return -1;
	}

	for (i = 0; i < im->part_count; ++i)
	{
		part_data_t* d = &im->parts[i];
		int len = strlen(d->partition_name);
		if (len == 0 || len > 16)
		{
			ERROR("Invalid partition name '%s' of the part %d\n",
					d->partition_name, i);
			return -1;
		}
		if (stat(d->filename, &d->stats) < 0)
		{
			ERROR("Couldn't stat file '%s' from part '%s'\n",
				       	d->filename, d->partition_name);
			return -2;
		}
		if (d->stats.st_size == 0)
		{
			ERROR("File '%s' from part '%s' is empty!\n",
				       	d->filename, d->partition_name);
			return -3;
		}
		if (d->stats.st_size > d->partition_length) {
			ERROR("File '%s' too big (%d) - max size: 0x%08X (exceeds %" PRId64 " bytes)\n",
				       	d->filename, i, d->partition_length,
					d->stats.st_size - d->partition_length);
			return -4;
		}
	}

	return 0;
}

static int build_image(image_info_t* im)
{
	char* mem;
	char* ptr;
	u_int32_t mem_size;
	FILE* f;
	unsigned int i;

	// build in-memory buffer
	mem_size = sizeof(header_t);
	if(im->fwinfo->sign) {
		mem_size += sizeof(signature_rsa_t);
	} else {
		mem_size += sizeof(signature_t);
	}
	for (i = 0; i < im->part_count; ++i)
	{
		part_data_t* d = &im->parts[i];
		mem_size += sizeof(part_t) + d->stats.st_size + sizeof(part_crc_t);
	}

	mem = (char*)calloc(mem_size, 1);
	if (mem == NULL)
	{
		ERROR("Cannot allocate memory chunk of size '%u'\n", mem_size);
		return -1;
	}

	// write header
	write_header(mem, im->magic, im->version);
	ptr = mem + sizeof(header_t);
	// write all parts
	for (i = 0; i < im->part_count; ++i)
	{
		part_data_t* d = &im->parts[i];
		int rc;
		if ((rc = write_part(ptr, d)) != 0)
		{
			ERROR("ERROR: failed writing part %u '%s'\n", i, d->partition_name);
		}
		ptr += sizeof(part_t) + d->stats.st_size + sizeof(part_crc_t);
	}
	// write signature
	if(im->fwinfo->sign) {
		write_signature_rsa(mem, mem_size - sizeof(signature_rsa_t));
	} else {
		write_signature(mem, mem_size - sizeof(signature_t));
	}

	// write in-memory buffer into file
	if ((f = fopen(im->outputfile, "w")) == NULL)
	{
		ERROR("Can not create output file: '%s'\n", im->outputfile);
		return -10;
	}

	if (fwrite(mem, mem_size, 1, f) != 1)
	{
		ERROR("Could not write %d bytes into file: '%s'\n",
				mem_size, im->outputfile);
		return -11;
	}

	free(mem);
	fclose(f);
	return 0;
}


int main(int argc, char* argv[])
{
	char kernelfile[PATH_MAX];
	char rootfsfile[PATH_MAX];
	char board_name[PATH_MAX];
	int o, rc;
	image_info_t im;
	struct fw_info *fwinfo;

	memset(&im, 0, sizeof(im));
	memset(kernelfile, 0, sizeof(kernelfile));
	memset(rootfsfile, 0, sizeof(rootfsfile));
	memset(board_name, 0, sizeof(board_name));

	strcpy(im.outputfile, DEFAULT_OUTPUT_FILE);
	strcpy(im.version, DEFAULT_VERSION);
	strncpy(im.magic, MAGIC_HEADER, sizeof(im.magic));

	while ((o = getopt(argc, argv, OPTIONS)) != -1)
	{
		switch (o) {
		case 'v':
			if (optarg)
				strncpy(im.version, optarg, sizeof(im.version) - 1);
			break;
		case 'o':
			if (optarg)
				strncpy(im.outputfile, optarg, sizeof(im.outputfile) - 1);
			break;
		case 'm':
			if (optarg)
				strncpy(im.magic, optarg, sizeof(im.magic) - 1);
			break;
		case 'h':
			usage(argv[0]);
			return -1;
		case 'k':
			if (optarg)
				strncpy(kernelfile, optarg, sizeof(kernelfile) - 1);
			break;
		case 'r':
			if (optarg)
				strncpy(rootfsfile, optarg, sizeof(rootfsfile) - 1);
			break;
		case 'B':
			if (optarg)
				strncpy(board_name, optarg, sizeof(board_name) - 1);
			break;
		}
	}
	if (strlen(board_name) == 0)
		strcpy(board_name, "XS2"); /* default to XS2 */

	if (strlen(kernelfile) == 0)
	{
		ERROR("Kernel file is not specified, cannot continue\n");
		usage(argv[0]);
		return -2;
	}

	if (strlen(rootfsfile) == 0)
	{
		ERROR("Root FS file is not specified, cannot continue\n");
		usage(argv[0]);
		return -2;
	}

	if ((fwinfo = get_fwinfo(board_name)) == NULL) {
		ERROR("Invalid baord name '%s'\n", board_name);
		usage(argv[0]);
		return -2;
	}

	im.fwinfo = fwinfo;

	if ((rc = create_image_layout(kernelfile, rootfsfile, &im)) != 0)
	{
		ERROR("Failed creating firmware layout description - error code: %d\n", rc);
		return -3;
	}

	if ((rc = validate_image_layout(&im)) != 0)
	{
		ERROR("Failed validating firmware layout - error code: %d\n", rc);
		return -4;
	}

	print_image_info(&im);

	if ((rc = build_image(&im)) != 0)
	{
		ERROR("Failed building image file '%s' - error code: %d\n", im.outputfile, rc);
		return -5;
	}

	return 0;
}
