/*
 * 	Make CHK Image
 *
 * 	This utility creates Netgear .chk files.
 *
 * 	Copyright (C) 2008 Dave C. Reeve <Dave.Reeve@dreeve.org>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *	
 *	You should have received a copy of the GNU General Public License along
 *	with this program; if not, write to the Free Software Foundation, Inc.,
 *	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUF_LEN (2048)

#define MAX_BOARD_ID_LEN (64)

/*
 * Note on the reserved field of the chk_header:
 * OFW naming scheme is typically: DEVICENAME-VA.B.C.D_E.F.G.chk, with A-G
 * between 0 and 255. For instance: EX3700_EX3800-V1.0.0.58_1.0.38.chk
 * The reserved field works like this:
 * reserved[0]: region code. 1 for WW (WorldWide) and 2 for NA (North America)
 * reserved[1]: A
 * reserved[2]: B
 * reserved[3]: C
 * reserved[4]: D
 * reserved[5]: E
 * reserved[6]: F
 * reserved[7]: G
 */
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
	/* char board_id[] - upto MAX_BOARD_ID_LEN */
};

static void __attribute__ ((format (printf, 2, 3)))
fatal_error (int maybe_errno, const char * format, ...)
{
	va_list ap;

	fprintf (stderr, "mkchkimg: ");
	va_start (ap, format);
	vfprintf (stderr, format, ap);
	va_end (ap);

	if (maybe_errno) {
		fprintf (stderr, ": %s\n", strerror (maybe_errno));
	} else {
		fprintf (stderr, "\n");
	}

	exit (EXIT_FAILURE);
}

static void __attribute__ ((format (printf, 1, 2)))
message (const char * format, ...)
{
	va_list ap;

	fprintf (stderr, "mkchkimg: ");
	va_start (ap, format);
	vfprintf (stderr, format, ap);
	va_end (ap);
	fprintf (stderr, "\n");
}

struct ngr_checksum {
	uint32_t c0;
	uint32_t c1;
};

static inline void
netgear_checksum_init (struct ngr_checksum * c)
{
	c->c0 = c->c1 = 0;
}

static inline void
netgear_checksum_add (struct ngr_checksum * c, unsigned char * buf, size_t len)
{
	size_t i;

	for (i=0; i<len; i++) {
		c->c0 += buf[i] & 0xff;
		c->c1 += c->c0;
	}
}

static inline unsigned long
netgear_checksum_fini (struct ngr_checksum * c)
{
	uint32_t b, checksum;

	b = (c->c0 & 65535) + ((c->c0 >> 16) & 65535);
	c->c0 = ((b >> 16) + b) & 65535;
	b = (c->c1 & 65535) + ((c->c1 >> 16) & 65535);
	c->c1 = ((b >> 16) + b) & 65535;
	checksum = ((c->c1 << 16) | c->c0);
	return checksum;
}

static void
print_help (void)
{
	fprintf (stderr, "Usage: mkchkimg -o output -k kernel [-f filesys] [-b board_id] [-r region]\n");
}

int
main (int argc, char * argv[])
{
	int opt;
	char * ptr;
	size_t len;
	size_t header_len;
	struct chk_header * hdr;
	struct ngr_checksum chk_part, chk_whole;
	char buf[BUF_LEN];
	char * output_file, * kern_file, * fs_file;
	FILE * out_fp, * kern_fp, * fs_fp;
	char * board_id;
	unsigned long region;

	/* Default values */
	board_id = "U12H072T00_NETGEAR";
	region = 1;	/* 1=WW, 2=NA */
	output_file = NULL;
	kern_file = NULL;
	fs_file = NULL;
	fs_fp = NULL;

	while ((opt = getopt (argc, argv, ":b:r:k:f:o:h")) != -1) {
		switch (opt) {
		    case 'b':
		    	/* Board Identity */
			if (strlen (optarg) > MAX_BOARD_ID_LEN) {
				fatal_error (0, "Board lenght exceeds %d", 
					MAX_BOARD_ID_LEN);
			}
			board_id = optarg;
			break;

		    case 'r':
		    	/* Region */
			errno = 0;
			region = strtoul (optarg, &ptr, 0);
			if (errno || ptr==optarg || *ptr!='\0') {
				fatal_error (0, "Cannot parse region %s", optarg);
			}
			if (region > 0xff) {
				fatal_error (0, "Region cannot exceed 0xff");
			}
			break;

		    case 'k':
		    	/* Kernel */
			kern_file = optarg;
			break;

		    case 'f':
		    	/* Filing System */
			fs_file = optarg;
			break;

		    case 'o':
		    	/* Output file */
			output_file = optarg;
			break;

		    case 'h':
		    	print_help ();
			return EXIT_SUCCESS;

		    case ':':
		    	print_help ();
		    	fatal_error (0, "Option -%c missing argument", optopt);
			break;

		    case '?':
		    	print_help ();
		    	fatal_error (0, "Unknown argument -%c", optopt);
			break;
		    
		    default:
		    	break;
		}
	}

	/* Check we have all the options expected */
	if (!kern_file) {
		print_help ();
		fatal_error (0, "Kernel file expected");
	}
	if (!output_file) {
		print_help ();
		fatal_error (0, "Output file required");
	}
	message ("Netgear CHK writer - v0.1");

	/* Open the input file */
	kern_fp = fopen (kern_file, "r");
	if (!kern_fp) {
		fatal_error (errno, "Cannot open %s", kern_file);
	}

	/* Open the fs file, if specified */
	if (fs_file) {
		fs_fp = fopen (fs_file, "r");
		if (!fs_fp) {
			fatal_error (errno, "Cannot open %s", fs_file);
		}
	}

	/* Open the output file */
	out_fp = fopen (output_file, "w+");
	if (!out_fp) {
		fatal_error (errno, "Cannot open %s", output_file);
	}

	/* Write zeros when the chk header will be */
	buf[0] = '\0';
	header_len = sizeof (struct chk_header) + strlen (board_id);
	if (fwrite (buf, 1, header_len, out_fp) != header_len) {
		fatal_error (errno, "Cannot write header");
	}

	/* Allocate storage for header, we fill in as we go */
	hdr = malloc (sizeof (struct chk_header));
	if (!hdr) {
		fatal_error (0, "malloc failed");
	}
	bzero (hdr, sizeof (struct chk_header));

	/* Fill in known values */
	hdr->magic = htonl (0x2a23245e);
	hdr->header_len = htonl(header_len);
	hdr->reserved[0] = (unsigned char)(region & 0xff);
	hdr->reserved[1] = 1;		/* Major */
	hdr->reserved[2] = 1;		/* Minor */
	hdr->reserved[3] = 99;		/* Build */
	hdr->reserved[4] = 0;
	hdr->reserved[5] = 0;
	hdr->reserved[6] = 0;
	hdr->reserved[7] = 0;
	message ("       Board Id: %s", board_id);
	message ("         Region: %s", region == 1 ? "World Wide (WW)" 
			: (region == 2 ? "North America (NA)" : "Unknown"));

	/* Copy the trx file, calculating the checksum as we go */
	netgear_checksum_init (&chk_part);
	netgear_checksum_init (&chk_whole);
	while (!feof (kern_fp)) {
		len = fread (buf, 1, BUF_LEN, kern_fp);
		if (len < 1) {
			break;
		}
		if (fwrite (buf, len, 1, out_fp) != 1) {
			fatal_error (errno, "Write error");
		}
		hdr->kernel_len += len;
		netgear_checksum_add (&chk_part, (unsigned char *)buf, len);
		netgear_checksum_add (&chk_whole, (unsigned char *)buf, len);
	}
	hdr->kernel_chksum = netgear_checksum_fini (&chk_part);
	message ("     Kernel Len: %u", hdr->kernel_len);
	message ("Kernel Checksum: 0x%08x", hdr->kernel_chksum);
	hdr->kernel_len = htonl (hdr->kernel_len);
	hdr->kernel_chksum = htonl (hdr->kernel_chksum);

	/* Now copy the root fs, calculating the checksum as we go */
	if (fs_fp) {
		netgear_checksum_init (&chk_part);
		while (!feof (fs_fp)) {
			len = fread (buf, 1, BUF_LEN, fs_fp);
			if (len < 1) {
				break;
			}
			if (fwrite (buf, len, 1, out_fp) != 1) {
				fatal_error (errno, "Write error");
			}
			hdr->rootfs_len += len;
			netgear_checksum_add (&chk_part, (unsigned char *)buf, len);
			netgear_checksum_add (&chk_whole, (unsigned char *)buf, len);
		}
		hdr->rootfs_chksum = (netgear_checksum_fini (&chk_part));
		message ("     Rootfs Len: %u", hdr->rootfs_len);
		message ("Rootfs Checksum: 0x%08x", hdr->rootfs_chksum);
		hdr->rootfs_len = htonl (hdr->rootfs_len);
		hdr->rootfs_chksum = htonl (hdr->rootfs_chksum);
	}

	/* Calcautate the image checksum */
	hdr->image_chksum = netgear_checksum_fini (&chk_whole);
	message (" Image Checksum: 0x%08x", hdr->image_chksum);
	hdr->image_chksum = htonl (hdr->image_chksum);

	/* Calculate the header checksum */
	netgear_checksum_init (&chk_part);
	netgear_checksum_add (&chk_part, (unsigned char *)hdr, 
				sizeof (struct chk_header));
	netgear_checksum_add (&chk_part, (unsigned char *)board_id,
				strlen (board_id));
	hdr->header_chksum = htonl (netgear_checksum_fini (&chk_part));

	/* Finally rewind the output and write headers */
	rewind (out_fp);
	if (fwrite (hdr, sizeof (struct chk_header), 1, out_fp) != 1) {
		fatal_error (errno, "Cannot write header");
	}
	if (fwrite (board_id, strlen (board_id), 1, out_fp) != 1) {
		fatal_error (errno, "Cannot write board id");
	}

	/* Success */
	return EXIT_SUCCESS;
}

