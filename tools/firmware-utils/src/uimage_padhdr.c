/*
 * uimage_padhdr.c : add zero paddings after the tail of uimage header
 *
 * Copyright (C) 2019 NOGUCHI Hiroshi <drvlabo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <zlib.h>


/* from u-boot/include/image.h */
#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;


/* default padding size */
#define	IH_PAD_BYTES		(32)


static void usage(char *prog)
{
	fprintf(stderr,
		"%s -i <input_uimage_file> -o <output_file> [-l <padding bytes>]\n",
		prog);
}

int main(int argc, char *argv[])
{
	struct stat statbuf;
	u_int8_t *filebuf;
	int ifd;
	int ofd;
	ssize_t rsz;
	u_int32_t crc_recalc;
	image_header_t *imgh;
	int opt;
	char *infname = NULL;
	char *outfname = NULL;
	int padsz = IH_PAD_BYTES;
	int ltmp;

	while ((opt = getopt(argc, argv, "i:o:l:")) != -1) {
		switch (opt) {
		case 'i':
			infname = optarg;
			break;
		case 'o':
			outfname = optarg;
			break;
		case 'l':
			ltmp = strtol(optarg, NULL, 0);
			if (ltmp > 0)
				padsz = ltmp;
			break;
		default:
			break;
		}
	}

	if (!infname || !outfname) {
		usage(argv[0]);
		exit(1);
	}

	ifd = open(infname, O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr,
			"could not open input file. (errno = %d)\n", errno);
		exit(1);
	}

	ofd = open(outfname, O_WRONLY | O_CREAT, 0644);
	if (ofd < 0) {
		fprintf(stderr,
			"could not open output file. (errno = %d)\n", errno);
		exit(1);
	}

	if (fstat(ifd, &statbuf) < 0) {
		fprintf(stderr,
			"could not fstat input file. (errno = %d)\n", errno);
		exit(1);
	}

	filebuf = malloc(statbuf.st_size + padsz);
	if (!filebuf) {
		fprintf(stderr, "buffer allocation failed\n");
		exit(1);
	}

	rsz = read(ifd, filebuf, sizeof(*imgh));
	if (rsz != sizeof(*imgh)) {
		fprintf(stderr,
			"could not read input file (errno = %d).\n", errno);
		exit(1);
	}

	memset(&(filebuf[sizeof(*imgh)]), 0, padsz);

	rsz = read(ifd, &(filebuf[sizeof(*imgh) + padsz]),
				statbuf.st_size - sizeof(*imgh));
	if (rsz != (int32_t)(statbuf.st_size - sizeof(*imgh))) {
		fprintf(stderr,
			"could not read input file (errno = %d).\n", errno);
		exit(1);
	}

	imgh = (image_header_t *)filebuf;

	imgh->ih_hcrc = 0;
	crc_recalc = crc32(0, filebuf, sizeof(*imgh) + padsz);
	imgh->ih_hcrc = htonl(crc_recalc);

	rsz = write(ofd, filebuf, statbuf.st_size + padsz);
	if (rsz != (int32_t)statbuf.st_size + padsz) {
		fprintf(stderr,
			"could not write output file (errnor = %d).\n", errno);
		exit(1);
	}

	return 0;
}
