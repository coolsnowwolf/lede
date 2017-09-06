/*
 * Copyright (C) 2004  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* July 29, 2004
 *
 * This is a hacked replacement for the 'addpattern' utility used to
 * create wrt54g .bin firmware files.  It isn't pretty, but it does
 * the job for me.
 *
 * Extensions:
 *  -v allows setting the version string on the command line.
 *  -{0|1} sets the (currently ignored) hw_ver flag in the header
 *      to 0 or 1 respectively.
 */

/* January 12, 2005
 *
 * Modified by rodent at rodent dot za dot net
 * Support added for the new WRT54G v2.2 and WRT54GS v1.1 "flags"
 * Without the flags set to 0x7, the above units will refuse to flash.
 *
 * Extensions:
 *  -{0|1|2} sets {0|1} sets hw_ver flag to 0/1. {2} sets hw_ver to 1
 *     and adds the new hardware "flags" for the v2.2/v1.1 units
*/

/* January 1, 2007
 *
 * Modified by juan.i.gonzalez at subdown dot net
 * Support added for the AG241v2  and similar
 *
 * Extensions:
 *  -r #.# adds revision hardware flags. AG241v2 and similar.
 *
 * AG241V2 firmware sets the hw_ver to 0x44.
 *
 * Example: -r 2.0
 *
 * Convert 2.0 to 20 to be an integer, and add 0x30 to skip special ASCII
 * #define HW_Version ((HW_REV * 10) + 0x30)  -> from cyutils.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

/**********************************************************************/

#define CODE_ID		"U2ND"		/* from code_pattern.h */
#define CODE_PATTERN   "W54S"	/* from code_pattern.h */
#define PBOT_PATTERN   "PBOT"

#define CYBERTAN_VERSION	"v3.37.2" /* from cyutils.h */

/* WRT54G v2.2 and WRT54GS v1.1 "flags" (from 3.37.32 firmware cyutils.h) */
#define SUPPORT_4712_CHIP      0x0001
#define SUPPORT_INTEL_FLASH    0x0002
#define SUPPORT_5325E_SWITCH   0x0004
/* (from 3.00.24 firmware cyutils.h) */
#define SUPPORT_4704_CHIP      0x0008
#define SUPPORT_5352E_CHIP     0x0010
/* (from WD My Net Wi-Fi Range Extender's cyutils.s) */
#define SUPPORT_4703_CHIP      0x0020

struct code_header {			/* from cyutils.h */
	char magic[8];
	char fwdate[3];
	char fwvern[3];
	char id[4];					/* U2ND */
	char hw_ver;    			/* 0: for 4702, 1: for 4712 -- new in 2.04.3 */

	unsigned char  sn;		// Serial Number
	unsigned char  flags[2];	/* SUPPORT_ flags new for 3.37.2 (WRT54G v2.2 and WRT54GS v1.1) */
	unsigned char  stable[2];	// The image is stable (for dual image)
	unsigned char  try1[2];		// Try to boot image first time (for dual image)
	unsigned char  try2[2];		// Try to boot image second time (for dual image)
	unsigned char  try3[2];		// Try to boot image third time (for dual_image)
	unsigned char  res3[2];
} ;

struct board_info {
	char	*id;
	char	*pattern;
	char	hw_ver;
	char	sn;
	char	flags[2];
};

struct board_info boards[] = {
	{
		.id             = "E2100L",
		.pattern        = "NL1X",
		.hw_ver         = 0x00,
		.sn             = 0x0f,
		.flags          = {0x3f, 0x00},
	},
	{
		.id		= "WRT160NL",
		.pattern	= "NL16",
		.hw_ver		= 0x00,
		.sn		= 0x0f,
		.flags		= {0x3f, 0x00},
	},
	{
		.id		= "mynet-rext",
		.pattern	= "WDHNSTFH",
		.hw_ver		= 0x00,
		.sn		= 0x00,
		.flags		= {0x3f, 0x00},
	}, {
		/* Terminating entry */
		.id	= NULL,
	}
};

/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: addpattern [-i trxfile] [-o binfile] [-B board_id] [-p pattern] [-s serial] [-g] [-b] [-v v#.#.#] [-r #.#] [-{0|1|2|4|5}] -h\n");
	exit(EXIT_FAILURE);
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

struct board_info *find_board(char *id)
{
	struct board_info *board;

	for (board = boards; board->id != NULL; board++)
		if (strcasecmp(id, board->id) == 0)
			return board;

	return NULL;
}

int main(int argc, char **argv)
{
	char buf[1024];	/* keep this at 1k or adjust garbage calc below */
	struct code_header *hdr;
	FILE *in = stdin;
	FILE *out = stdout;
	char *ifn = NULL;
	char *ofn = NULL;
	char *pattern = CODE_PATTERN;
	char *pbotpat = PBOT_PATTERN;
	char *version = CYBERTAN_VERSION;
	char *board_id = NULL;
	struct board_info *board = NULL;
	int gflag = 0;
	int pbotflag = 0;
	int c;
	int v0, v1, v2;
	size_t off, n;
	time_t t;
	struct tm *ptm;

	fprintf(stderr, "mjn3's addpattern replacement - v0.81\n");

	hdr = (struct code_header *) buf;
	memset(hdr, 0, sizeof(struct code_header));

	while ((c = getopt(argc, argv, "i:o:p:s:gbv:01245hr:B:")) != -1) {
		switch (c) {
			case 'i':
				ifn = optarg;
				break;
			case 'o':
				ofn = optarg;
				break;
			case 'p':
				pattern = optarg;
				break;
			case 's':
				hdr->sn = (unsigned char) atoi (optarg);
				break;
			case 'g':
				gflag = 1;
				break;
			case 'b':
				pbotflag = 1;
				break;
			case 'v':			/* extension to allow setting version */
				version = optarg;
				break;
			case '0':
				hdr->hw_ver = 0;
				break;
			case '1':
				hdr->hw_ver = 1;
				break;
			case '2': 			/* new 54G v2.2 and 54GS v1.1 flags */
				hdr->hw_ver = 1;
				hdr->flags[0] |= SUPPORT_4712_CHIP;
				hdr->flags[0] |= SUPPORT_INTEL_FLASH;
				hdr->flags[0] |= SUPPORT_5325E_SWITCH;
				break;
			case '4':
				/* V4 firmware sets the flags to 0x1f */
				hdr->hw_ver = 0;
				hdr->flags[0] = 0x1f;
				break;
			case '5':
				/* V5 is appended to trxV2 image */
				hdr->stable[0] = 0x73; // force image to be stable
				hdr->stable[1] = 0x00;
				hdr->try1[0]   = 0x74; // force try1 to be set
				hdr->try1[1]   = 0x00;
				hdr->try2[0]   = hdr->try2[1]   = 0xFF;
				hdr->try3[0]   = hdr->try3[1]   = 0xFF;
				break;
                        case 'r':
                                hdr->hw_ver = (char)(atof(optarg)*10)+0x30;
                                break;
                        case 'B':
                                board_id = optarg;
                                break;

                        case 'h':
			default:
				usage();
		}
	}

    	if (optind != argc || optind == 1) {
		fprintf(stderr, "illegal arg \"%s\"\n", argv[optind]);
		usage();
	}

	if (board_id) {
		board = find_board(board_id);
		if (board == NULL) {
			fprintf(stderr, "unknown board \"%s\"\n", board_id);
			usage();
		}
		pattern = board->pattern;
		hdr->hw_ver = board->hw_ver;
		hdr->sn = board->sn;
		hdr->flags[0] = board->flags[0];
		hdr->flags[1] = board->flags[1];
	}

	if (strlen(pattern) > 8) {
		fprintf(stderr, "illegal pattern \"%s\"\n", pattern);
		usage();
	}

	if (ifn && !(in = fopen(ifn, "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", ifn);
		usage();
	}

	if (ofn && !(out = fopen(ofn, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
		usage();
	}

	set_source_date_epoch();
	if (source_date_epoch != -1) {
		t = source_date_epoch;
	} else if ((time(&t) == (time_t)(-1))) {
		fprintf(stderr, "time call failed\n");
		return EXIT_FAILURE;
	}

	ptm = localtime(&t);

	if (3 != sscanf(version, "v%d.%d.%d", &v0, &v1, &v2)) {
		fprintf(stderr, "bad version string \"%s\"\n", version);
		return EXIT_FAILURE;
	}

	memcpy(hdr->magic, pattern, strlen(pattern));
	if (pbotflag)
		memcpy(&hdr->magic[4], pbotpat, 4);
	hdr->fwdate[0] = ptm->tm_year % 100;
	hdr->fwdate[1] = ptm->tm_mon + 1;
	hdr->fwdate[2] = ptm->tm_mday;
	hdr->fwvern[0] = v0;
	hdr->fwvern[1] = v1;
	hdr->fwvern[2] = v2;
	memcpy(hdr->id, CODE_ID, strlen(CODE_ID));

	off = sizeof(struct code_header);

	fprintf(stderr, "writing firmware v%d.%d.%d on %d/%d/%d (y/m/d)\n",
			v0, v1, v2,
			hdr->fwdate[0], hdr->fwdate[1], hdr->fwdate[2]);


	while ((n = fread(buf + off, 1, sizeof(buf)-off, in) + off) > 0) {
		off = 0;
		if (n < sizeof(buf)) {
			if (ferror(in)) {
			FREAD_ERROR:
				fprintf(stderr, "fread error\n");
				return EXIT_FAILURE;
			}
			if (gflag) {
				gflag = sizeof(buf) - n;
				memset(buf + n, 0xff, gflag);
				fprintf(stderr, "adding %d bytes of garbage\n", gflag);
				n = sizeof(buf);
			}
		}
		if (!fwrite(buf, n, 1, out)) {
		FWRITE_ERROR:
			fprintf(stderr, "fwrite error\n");
			return EXIT_FAILURE;
		}
	}

	if (ferror(in)) {
		goto FREAD_ERROR;
	}

	if (fflush(out)) {
		goto FWRITE_ERROR;
	}

	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}
