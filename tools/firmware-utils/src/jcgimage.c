/*
 * jcgimage - Create a JCG firmware image
 *
 * Copyright (C) 2015 Reinhard Max <reinhard@m4x.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/*
 * JCG firmware update images consist of a 512 byte header and a
 * modified uImage (details below) as the payload.
 *
 * The payload is obfuscated by XORing it with a key that is generated
 * from parts of the header. Fortunately only non-essential parts of
 * the header are used for this and zeroing them results in a zero
 * key, effectively disabling the obfuscation and allowing us to use
 * clear text payloads.
 *
 * The mandatory parts of the header are:
 *
 * - A magic string of "YSZJ" at offset 0.
 * - A value of 1 at offset 39 (header format version?)
 * - A CRC32 checksum of the payload at offset 504.
 * - A CRC32 checksum of the header at offset 508.
 *
 * An image constructed by these rules will be accepted by JCG's
 * U-Boot in resuce mode via TFTP and the payload will be written to
 * the flash starting at offset 0x00050000.
 *
 * JCG's U-Boot does check the content or size of the payload
 * image. If it is too large, it wraps around and overwrites U-Boot,
 * requiring JTAG to revive the board. To prevent such bricking from
 * happening, this tool refuses to build such overlong images.
 *
 * Two more conditions have to be met for a JCG image to be accepted
 * as a valid update by the web interface of the stock firware:
 *
 *   - The bytes at offsets 109 and 111 in the header must be a binary
 *   representation of the first two components of the firmware
 *   version as displayed in the update web form, or it will be
 *   rejected as "incorrect product".
 *
 *   - The payload must start with a valid uImage header whose data
 *   CRC checksum matches the whole rest of the update file rather
 *   than just the number of bytes specified in the size field of the
 *   header.
 *
 * This last condition is met by JCG's original firmware images,
 * because they have both, kernel and rootfs inside the uImage and
 * abuse the last four bytes of the name field to record the offset of
 * the file system from the start of the uImage header. This tool
 * produces such images when called with -k and -r, which are meant to
 * repack the original firmware after modifying the file systen,
 * e.g. to add debugging tools and enable shell access.
 *
 * In contrast, OpenWrt sysupgrade images consist of a uImage that
 * only contains the kernel and has the rootfs appended to it. Hence,
 * the CRC over kernel and file system does not match the one in the
 * uImage header. Fixing this by adjusting the uImage header is not
 * possible, because it makes the uImage unusable for booting. Instead
 * we append four "patch" bytes to the end of the file system, that
 * are calculated to force the checksum of kernel+fs to be the same as
 * for the kernel alone.
 *
 */

#include <zlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <time.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <assert.h>

/*
 * JCG Firmware image header
 */
#define JH_MAGIC 0x59535a4a        /* "YSZJ" */
struct jcg_header {
	uint32_t jh_magic;
	uint8_t  jh_version[32];   /* Firmware version string.
				      Fill with zeros to avoid encryption  */
	uint32_t jh_type;          /* must be 1                            */
	uint8_t  jh_info[64];      /* Firmware info string. Fill with
				      zeros to avoid encryption            */
	uint32_t jh_time;          /* Image creation time in seconds since
				    * the Epoch. Does not seem to be used
				    * by the stock firmware.               */
	uint16_t jh_major;         /* Major fimware version                */
	uint16_t jh_minor;         /* Minor fimrmware version              */
	uint8_t  jh_unknown[392];  /* Apparently unused and all zeros      */
	uint32_t jh_dcrc;          /* CRC checksum of the payload          */
	uint32_t jh_hcrc;          /* CRC checksum of the header           */
};

/*
 * JCG uses a modified uImage header that replaces the last four bytes
 * of the image name with the length of the kernel in the image.
 */
#define IH_MAGIC    0x27051956    /* Image Magic Number     */
#define IH_NMLEN    28            /* Image Name Length      */

struct uimage_header {
	uint32_t    ih_magic;         /* Image Header Magic Number   */
	uint32_t    ih_hcrc;          /* Image Header CRC Checksum   */
	uint32_t    ih_time;          /* Image Creation Timestamp    */
	uint32_t    ih_size;          /* Image Data Size             */
	uint32_t    ih_load;          /* Data     Load  Address      */
	uint32_t    ih_ep;            /* Entry Point Address         */
	uint32_t    ih_dcrc;          /* Image Data CRC Checksum     */
	uint8_t     ih_os;            /* Operating System            */
	uint8_t     ih_arch;          /* CPU architecture            */
	uint8_t     ih_type;          /* Image Type                  */
	uint8_t     ih_comp;          /* Compression Type            */
	uint8_t     ih_name[IH_NMLEN];/* Image Name                  */
	uint32_t    ih_fsoff;         /* Offset of the file system
					 partition from the start of
					 the header                  */
};

/*
 * Open the named file and return its size and file descriptor.
 * Exit in case of errors.
 */
int
opensize(char *name, size_t *size)
{
	struct stat s;
	int fd = open(name, O_RDONLY);
	if (fd < 0) {
		err(1, "cannot open \"%s\"", name);
	}
	if (fstat(fd, &s) == -1) {
		err(1, "cannot stat \"%s\"", name);
	}
	*size = s.st_size;
	return fd;
}

static time_t source_date_epoch = -1;
static void set_source_date_epoch() {
	char *env = getenv("SOURCE_DATE_EPOCH");
	char *endptr = env;
	errno = 0;
        if (env && *env) {
		source_date_epoch = strtoull(env, &endptr, 10);
		if (errno || (endptr && *endptr != '\0')) {
			fprintf(stderr, "Invalid SOURCE_DATE_EPOCH");
			exit(1);
		}
        }
}

/*
 * Write the JCG header
 */
void
mkjcgheader(struct jcg_header *h, size_t psize, char *version)
{
	uLong crc;
	uint16_t major = 0, minor = 0;
	void *payload = (void *)h + sizeof(*h);
	time_t t;

	if (source_date_epoch != -1) {
		t = source_date_epoch;
	} else if ((time(&t) == (time_t)(-1))) {
		err(1, "time call failed");
	}

	if (version != NULL) {
		if (sscanf(version, "%hu.%hu", &major, &minor) != 2) {
			err(1, "cannot parse version \"%s\"", version);
		}
	}

	memset(h, 0, sizeof(*h));
	h->jh_magic = htonl(JH_MAGIC);
	h->jh_type  = htonl(1);
	h->jh_time  = htonl(t);
	h->jh_major = htons(major);
	h->jh_minor = htons(minor);

	/* CRC over JCG payload (uImage) */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, payload, psize);
	h->jh_dcrc  = htonl(crc);

	/* CRC over JCG header */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (void *)h, sizeof(*h));
	h->jh_hcrc  = htonl(crc);
}

/*
 * Write the uImage header
 */
void
mkuheader(struct uimage_header *h, size_t ksize, size_t fsize)
{
	uLong crc;
	void *payload = (void *)h + sizeof(*h);

	// printf("mkuheader: %p, %zd, %zd\n", h, ksize, fsize);
	memset(h, 0, sizeof(*h));
	h->ih_magic = htonl(IH_MAGIC);
	h->ih_time  = htonl(time(NULL));
	h->ih_size  = htonl(ksize + fsize);
	h->ih_load  = htonl(0x80000000);
	h->ih_ep    = htonl(0x80292000);
	h->ih_os    = 0x05;
	h->ih_arch  = 0x05;
	h->ih_type  = 0x02;
	h->ih_comp  = 0x03;
	h->ih_fsoff = htonl(sizeof(*h) + ksize);
	strcpy((char *)h->ih_name, "Linux Kernel Image");

	/* CRC over uImage payload (kernel and file system) */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, payload, ntohl(h->ih_size));
	h->ih_dcrc  = htonl(crc);
	printf("CRC1: %08lx\n", crc);

	/* CRC over uImage header */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, (void *)h, sizeof(*h));
	h->ih_hcrc  = htonl(crc);
	printf("CRC2: %08lx\n", crc);
}

/*
 * Calculate a "patch" value and write it into the last four bytes of
 * buf, so that the CRC32 checksum of the whole buffer is dcrc.
 *
 * Based on: SAR-PR-2006-05: Reversing CRC – Theory and Practice.
 * Martin Stigge, Henryk Plötz, Wolf Müller, Jens-Peter Redlich.
 * http://sar.informatik.hu-berlin.de/research/publications/#SAR-PR-2006-05
 */
void
craftcrc(uint32_t dcrc, uint8_t *buf, size_t len)
{
	int i;
	uint32_t a;
	uint32_t patch = 0;
	uint32_t crc = crc32(0L, Z_NULL, 0);

	a = ~dcrc;
	for (i = 0; i < 32; i++) {
		if (patch & 1) {
			patch = (patch >> 1) ^ 0xedb88320L;
		} else {
			patch >>= 1;
		}
		if (a & 1) {
			patch ^= 0x5b358fd3L;
		}
		a >>= 1;
	}
	patch ^= ~crc32(crc, buf, len - 4);
	for (i = 0; i < 4; i++) {
		buf[len - 4 + i] = patch & 0xff;
		patch >>= 8;
	}
	/* Verify that we actually get the desired result */
	crc = crc32(0L, Z_NULL, 0);
	crc = crc32(crc, buf, len);
	if (crc != dcrc) {
		errx(1, "CRC patching is broken: wanted %08x, but got %08x.",
		     dcrc, crc);
	}
}

void
usage() {
	fprintf(stderr, "Usage:\n"
		"jcgimage -o outfile -u uImage [-v version]\n"
		"jcgimage -o outfile -k kernel -f rootfs [-v version]\n");
	exit(1);
}

#define MODE_UNKNOWN 0
#define MODE_UIMAGE 1
#define MODE_KR 2

/* The output image must not be larger than 4MiB - 5*64kiB */
#define MAXSIZE (size_t)(4 * 1024 * 1024 - 5 * 64 * 1024)

int
main(int argc, char **argv)
{
	struct jcg_header *jh;
	struct uimage_header *uh;
	int c;
	char *imagefile = NULL;
	char *file1 = NULL;
	char *file2 = NULL;
	char *version = NULL;
	int mode = MODE_UNKNOWN;
	int fdo, fd1, fd2;
	size_t size1, size2, sizeu, sizeo, off1, off2;
	void *map;

	/* Make sure the headers have the right size */
	assert(sizeof(struct jcg_header) == 512);
	assert(sizeof(struct uimage_header) == 64);
	set_source_date_epoch();

	while ((c = getopt(argc, argv, "o:k:f:u:v:h")) != -1) {
		switch (c) {
		case 'o':
			imagefile = optarg;
			break;
		case 'k':
			if (mode == MODE_UIMAGE) {
				errx(1,"-k cannot be combined with -u");
			}
			mode = MODE_KR;
			file1 = optarg;
			break;
		case 'f':
			if (mode == MODE_UIMAGE) {
				errx(1,"-f cannot be combined with -u");
			}
			mode = MODE_KR;
			file2 = optarg;
			break;
		case 'u':
			if (mode == MODE_KR) {
				errx(1,"-u cannot be combined with -k and -r");
			}
			mode = MODE_UIMAGE;
			file1 = optarg;
			break;
		case 'v':
			version = optarg;
			break;
		case 'h':
		default:
			usage();
		}
	}
	if (optind != argc) {
		errx(1, "illegal arg \"%s\"", argv[optind]);
	}
	if (imagefile == NULL) {
		errx(1, "no output file specified");
	}
	if (mode == MODE_UNKNOWN) {
		errx(1, "specify either -u or -k and -r");
	}
	if (mode == MODE_KR) {
		if (file1 == NULL || file2 == NULL) {
			errx(1,"need -k and -r");
		}
		fd2 = opensize(file2, &size2);
	}
	fd1 = opensize(file1, &size1);
	if (mode == MODE_UIMAGE) {
		off1 = sizeof(*jh);
		sizeu = size1 + 4;
		sizeo = sizeof(*jh) + sizeu;
	} else {
		off1 = sizeof(*jh) + sizeof(*uh);
		off2 = sizeof(*jh) + sizeof(*uh) + size1;
		sizeu = sizeof(*uh) + size1 + size2;
		sizeo = sizeof(*jh) + sizeu;
	}

	if (sizeo > MAXSIZE) {
		errx(1,"payload too large: %zd > %zd\n", sizeo, MAXSIZE);
	}

	fdo = open(imagefile, O_RDWR | O_CREAT | O_TRUNC, 00644);
	if (fdo < 0) {
		err(1, "cannot open \"%s\"", imagefile);
	}

	if (ftruncate(fdo, sizeo) == -1) {
		err(1, "cannot grow \"%s\" to %zd bytes", imagefile, sizeo);
	}
	map = mmap(NULL, sizeo, PROT_READ|PROT_WRITE, MAP_SHARED, fdo, 0);
	uh = map + sizeof(*jh);
	if (map == MAP_FAILED) {
		err(1, "cannot mmap \"%s\"", imagefile);
	}

	if (read(fd1, map + off1, size1) != size1) {
		err(1, "cannot copy %s", file1);
	}

	if (mode == MODE_KR) {
		if (read(fd2, map+off2, size2) != size2) {
			err(1, "cannot copy %s", file2);
		}
		mkuheader(uh, size1, size2);
	} else if (mode == MODE_UIMAGE) {
		craftcrc(ntohl(uh->ih_dcrc), (void*)uh + sizeof(*uh),
			 sizeu - sizeof(*uh));
	}
	mkjcgheader(map, sizeu, version);
	munmap(map, sizeo);
	close(fdo);
	return 0;
}
