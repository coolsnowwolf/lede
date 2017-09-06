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

// Usage: mkdapimg [-p] [-m <model>] -s <sig> -i <input> -o <output>
//
// e.g.: mkdapimg -s RT3052-AP-DAP1350-3 -i sysupgarde.bin -o factory.bin
//
// If the model string <model> is not given, we will assume that
// the leading characters upto the first "-" is the model.
//
// The "-p" (patch) option is used to patch the exisiting image with the
// specified model and signature.
// The "-x" (fix) option will recalculate the payload size and checksum
// during the patch mode operation.

// The img_hdr_struct was taken from the D-Link SDK:
// DAP-1350_A1_FW1.11NA_GPL/GPL_Source_Code/Uboot/DAP-1350/httpd/header.h

#define MAX_MODEL_NAME_LEN	20
#define MAX_SIG_LEN		30
#define MAX_REGION_LEN		4
#define MAX_VERSION_LEN		12

struct img_hdr_struct {
	uint32_t checksum;
	char model[MAX_MODEL_NAME_LEN];
	char sig[MAX_SIG_LEN];
	uint8_t	partition;       
	uint8_t hdr_len;
	uint8_t rsv1;
	uint8_t rsv2;    
	uint32_t flash_byte_cnt;  
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
	fprintf(stderr, "usage: %s [-p] [-m model] [-r region] [-v version] -s signature -i input -o output\n", progname);
	exit(1);
}

int
main(int ac, char *av[])
{
	char model[MAX_MODEL_NAME_LEN+1];
	char signature[MAX_SIG_LEN+1];
	char region[MAX_REGION_LEN+1];
	char version[MAX_VERSION_LEN+1];
	int patchmode = 0;
	int fixmode = 0;
	int have_regionversion = 0;

	FILE *ifile, *ofile;
	int c;
	uint32_t cksum;
	uint32_t bcnt;

	progname = basename(av[0]);
	memset(model, 0, sizeof(model));
	memset(signature, 0, sizeof(signature));
	memset(region, 0, sizeof(region));
	memset(version, 0, sizeof(version));

	while ( 1 ) {
		int c;

		c = getopt(ac, av, "pxm:r:v:s:i:o:");
		if (c == -1)
			break;

		switch (c) {
		case 'p':
			patchmode = 1;
			break;
		case 'x':
			fixmode = 1;
			break;
		case 'm':
			if (strlen(optarg) > MAX_MODEL_NAME_LEN) {
				fprintf(stderr, "%s: model name exceeds %d chars\n",
					progname, MAX_MODEL_NAME_LEN);
				exit(1);
			}
			strcpy(model, optarg);
			break;
		case 'r':
			if (strlen(optarg) > MAX_REGION_LEN) {
				fprintf(stderr, "%s: region exceeds %d chars\n",
					progname, MAX_REGION_LEN);
				exit(1);
			}
			have_regionversion = 1;
			strcpy(region, optarg);
			break;
		case 'v':
			if (strlen(optarg) > MAX_VERSION_LEN) {
				fprintf(stderr, "%s: version exceeds %d chars\n",
					progname, MAX_VERSION_LEN);
				exit(1);
			}
			have_regionversion = 1;
			strcpy(version, optarg);
			break;
		case 's':
			if (strlen(optarg) > MAX_SIG_LEN) {
				fprintf(stderr, "%s: signature exceeds %d chars\n",
					progname, MAX_SIG_LEN);
				exit(1);
			}
			strcpy(signature, optarg);
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
	}

	if (model[0] == 0) {
		char *p = strchr(signature, '-');
		if (p == NULL) {
			fprintf(stderr, "%s: model name unknown\n", progname);
			exit(1);
		}
		if (p - signature > MAX_MODEL_NAME_LEN) {
			*p = 0;
			fprintf(stderr, "%s: auto model name failed, string %s too long\n", progname, signature);
			exit(1);
		}
		strncpy(model, signature, p - signature);
	}

	if (patchmode) {
		if (fread(&imghdr, sizeof(imghdr), 1, ifile) < 0)
			perrexit(2, "fread on input");
	}

	for (bcnt = 0, cksum = 0 ; (c = fgetc(ifile)) != EOF ; bcnt++)
		cksum += c & 0xff;

	if (fseek(ifile, patchmode ? sizeof(imghdr) : 0, SEEK_SET) < 0)
		perrexit(2, "fseek on input");

	if (patchmode == 0) {
		// Fill in the header
		memset(&imghdr, 0, sizeof(imghdr));
		imghdr.checksum = htonl(cksum);
		imghdr.partition = 0 ; // don't care?
		imghdr.hdr_len = sizeof(imghdr);
		if (have_regionversion) {
			imghdr.hdr_len += MAX_REGION_LEN;
			imghdr.hdr_len += MAX_VERSION_LEN;
		}
		imghdr.flash_byte_cnt = htonl(bcnt);
	} else {
		if (ntohl(imghdr.checksum) != cksum) {
			fprintf(stderr, "%s: patch mode, checksum mismatch\n",
				progname);
			if (fixmode) {
				fprintf(stderr, "%s: fixing\n", progname);
				imghdr.checksum = htonl(cksum);
			} else
				exit(3);
		} else if (ntohl(imghdr.flash_byte_cnt) != bcnt) {
			fprintf(stderr, "%s: patch mode, size mismatch\n",
				progname);
			if (fixmode) {
				fprintf(stderr, "%s: fixing\n", progname);
				imghdr.flash_byte_cnt = htonl(bcnt);
			} else
				exit(3);
		}
	}

	strncpy(imghdr.model, model, MAX_MODEL_NAME_LEN);
	strncpy(imghdr.sig, signature, MAX_SIG_LEN);

	if (fwrite(&imghdr, sizeof(imghdr), 1, ofile) < 0)
		perrexit(2, "fwrite header on output");
	if (have_regionversion) {
		if (fwrite(&region, MAX_REGION_LEN, 1, ofile) < 0)
			perrexit(2, "fwrite header on output");
		if (fwrite(&version, MAX_VERSION_LEN, 1, ofile) < 0)
			perrexit(2, "fwrite header on output");
	}

	while ((c = fgetc(ifile)) != EOF) {
		if (fputc(c, ofile) == EOF)
			perrexit(2, "fputc on output");
	}

	if (ferror(ifile))
		perrexit(2, "fgetc on input");


	fclose(ofile);
	fclose(ifile);
}
