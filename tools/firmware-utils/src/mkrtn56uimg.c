/*
 *
 *  Copyright (C) 2014 OpenWrt.org
 *  Copyright (C) 2014 Mikko Hissa <mikko.hissa@werzek.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zlib.h>

#define IH_MAGIC	0x27051956
#define IH_NMLEN	32
#define IH_PRODLEN	23

#define IH_TYPE_INVALID		0
#define IH_TYPE_STANDALONE	1
#define IH_TYPE_KERNEL		2
#define IH_TYPE_RAMDISK		3
#define IH_TYPE_MULTI		4
#define IH_TYPE_FIRMWARE	5
#define IH_TYPE_SCRIPT		6
#define IH_TYPE_FILESYSTEM	7

/*
 * Compression Types
 */
#define IH_COMP_NONE		0
#define IH_COMP_GZIP		1
#define IH_COMP_BZIP2		2
#define IH_COMP_LZMA		3

typedef struct {
	uint8_t major;
	uint8_t minor;
} version_t;

typedef struct {
	version_t	kernel;
	version_t	fs;
	uint8_t		productid[IH_PRODLEN];
	uint8_t  	sub_fs;
	uint32_t	ih_ksz;
} asus_t;

typedef struct image_header {
	uint32_t	ih_magic;
	uint32_t	ih_hcrc;
	uint32_t	ih_time;
	uint32_t	ih_size;
	uint32_t	ih_load;
	uint32_t	ih_ep;
	uint32_t	ih_dcrc;
	uint8_t		ih_os;
	uint8_t		ih_arch;
	uint8_t		ih_type;
	uint8_t		ih_comp;
	union {
		uint8_t	ih_name[IH_NMLEN];
		asus_t	asus;
	} tail;
} image_header_t;

typedef struct squashfs_sb {
	uint32_t	s_magic;
	uint32_t	pad0[9];
	uint64_t	bytes_used;
} squashfs_sb_t;

typedef enum {
	NONE, FACTORY, SYSUPGRADE,
} op_mode_t;

void
calc_crc(image_header_t *hdr, void *data, uint32_t len)
{
	/*
	 * Calculate payload checksum
	 */
	hdr->ih_dcrc = htonl(crc32(0, (Bytef *)data, len));
	hdr->ih_size = htonl(len);
	/*
	 * Calculate header checksum
	 */
	hdr->ih_hcrc = 0;
	hdr->ih_hcrc = htonl(crc32(0, (Bytef *)hdr, sizeof(image_header_t)));
}


static void
usage(const char *progname, int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	int i;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream, "\n"
			"Options:\n"
			"  -f <file>		generate a factory flash image <file>\n"
			"  -s <file>		generate a sysupgrade flash image <file>\n"
			"  -h			show this screen\n");
	exit(status);
}

int
process_image(char *progname, char *filename, op_mode_t opmode)
{
	int 		fd, len;
	void 		*data, *ptr;
	char		namebuf[IH_NMLEN];
	struct 		stat sbuf;
	uint32_t	checksum, offset_kernel, offset_sqfs, offset_end,
				offset_sec_header, offset_eb, offset_image_end;
	squashfs_sb_t *sqs;
	image_header_t *hdr;

	if ((fd = open(filename, O_RDWR, 0666)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			progname, filename, strerror(errno));
		return (EXIT_FAILURE);
	}

	if (fstat(fd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			progname, filename, strerror(errno));
		return (EXIT_FAILURE);
	}

	if ((unsigned)sbuf.st_size < sizeof(image_header_t)) {
		fprintf (stderr,
			"%s: Bad size: \"%s\" is no valid image\n",
			progname, filename);
		return (EXIT_FAILURE);
	}

	ptr = (void *)mmap(0, sbuf.st_size,
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				fd, 0);

	if ((caddr_t)ptr == (caddr_t)-1) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
			progname, filename, strerror(errno));
		return (EXIT_FAILURE);
	}

	hdr = ptr;

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		fprintf (stderr,
			"%s: Bad Magic Number: \"%s\" is no valid image\n",
			progname, filename);
		return (EXIT_FAILURE);
	}

	if (opmode == FACTORY) {
		strncpy(namebuf, hdr->tail.ih_name, IH_NMLEN);
		hdr->tail.asus.kernel.major = 0;
		hdr->tail.asus.kernel.minor = 0;
		hdr->tail.asus.fs.major = 0;
		hdr->tail.asus.fs.minor = 0;
		strncpy((char *)&hdr->tail.asus.productid, "RT-N56U", IH_PRODLEN);
	}

	if (hdr->tail.asus.ih_ksz == 0)
		hdr->tail.asus.ih_ksz = htonl(ntohl(hdr->ih_size) + sizeof(image_header_t));

	offset_kernel = sizeof(image_header_t);
	offset_sqfs = ntohl(hdr->tail.asus.ih_ksz);
	sqs = ptr + offset_sqfs;
	offset_sec_header = offset_sqfs + sqs->bytes_used;

	/*
	 * Reserve space for the second header.
	 */
	offset_end = offset_sec_header + sizeof(image_header_t);
	offset_eb = ((offset_end>>16)+1)<<16;

	if (opmode == FACTORY)
		offset_image_end = offset_eb + 4;
	else
		offset_image_end = sbuf.st_size;
	/*
	 * Move the second header at the end of the image.
	 */
	offset_end = offset_sec_header;
	offset_sec_header = offset_eb - sizeof(image_header_t);

	/*
	 * Remove jffs2 markers between squashfs and eb boundary.
	 */
	if (opmode == FACTORY)
		memset(ptr+offset_end, 0xff ,offset_eb - offset_end);

	/*
	 * Grow the image if needed.
	 */
	if (offset_image_end > sbuf.st_size) {
		(void) munmap((void *)ptr, sbuf.st_size);
		ftruncate(fd, offset_image_end);
		ptr = (void *)mmap(0, offset_image_end,
						PROT_READ | PROT_WRITE,
						MAP_SHARED,
						fd, 0);
		/*
		 * jffs2 marker
		 */
		if (opmode == FACTORY) {
			*(uint8_t *)(ptr+offset_image_end-4) = 0xde;
			*(uint8_t *)(ptr+offset_image_end-3) = 0xad;
			*(uint8_t *)(ptr+offset_image_end-2) = 0xc0;
			*(uint8_t *)(ptr+offset_image_end-1) = 0xde;
		}
	}

	/*
	 * Calculate checksums for the second header to be used after flashing.
	 */
	if (opmode == FACTORY) {
		hdr = ptr+offset_sec_header;
		memcpy(hdr, ptr, sizeof(image_header_t));
		strncpy(hdr->tail.ih_name, namebuf, IH_NMLEN);
		calc_crc(hdr, ptr+offset_kernel, offset_sqfs - offset_kernel);
		calc_crc((image_header_t *)ptr, ptr+offset_kernel, offset_image_end - offset_kernel);
	} else {
		calc_crc((image_header_t *)ptr, ptr+offset_kernel, offset_sqfs - offset_kernel);
	}

	if (sbuf.st_size > offset_image_end)
		(void) munmap((void *)ptr, sbuf.st_size);
	else
		(void) munmap((void *)ptr, offset_image_end);

	ftruncate(fd, offset_image_end);
	(void) close (fd);

	return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
	int 		opt;
	char 		*filename, *progname;
	op_mode_t	opmode = NONE;

	progname = argv[0];

	while ((opt = getopt(argc, argv,":s:f:h?")) != -1) {
		switch (opt) {
		case 's':
			opmode = SYSUPGRADE;
			filename = optarg;
			break;
		case 'f':
			opmode = FACTORY;
			filename = optarg;
			break;
		case 'h':
			opmode = NONE;
		default:
			usage(progname, EXIT_FAILURE);
			opmode = NONE;
		}
	}

	if(filename == NULL)
		opmode = NONE;

	switch (opmode) {
	case NONE:
		usage(progname, EXIT_FAILURE);
		break;
	case FACTORY:
	case SYSUPGRADE:
		return process_image(progname, filename, opmode);
		break;
	}

	return EXIT_SUCCESS;
}

