/*
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  This code was based on:
 *	PC1 Cipher Algorithm ( Pukall Cipher 1 )
 *	By Alexander PUKALL 1991
 *	free code no restriction to use
 *	please include the name of the Author in the final software
 *	the Key is 128 bits
 *	http://membres.lycos.fr/pc1/
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     /* for unlink() */
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

struct pc1_ctx {
	unsigned short	ax;
	unsigned short	bx;
	unsigned short	cx;
	unsigned short	dx;
	unsigned short	si;
	unsigned short	tmp;
	unsigned short	x1a2;
	unsigned short	x1a0[8];
	unsigned short	res;
	unsigned short	i;
	unsigned short	inter;
	unsigned short	cfc;
	unsigned short	cfd;
	unsigned short	compte;
	unsigned char	cle[17];
	short		c;
};

static void pc1_finish(struct pc1_ctx *pc1)
{
	/* erase all variables */
	memset(pc1, 0, sizeof(struct pc1_ctx));
}

static void pc1_code(struct pc1_ctx *pc1)
{
	pc1->dx = pc1->x1a2 + pc1->i;
	pc1->ax = pc1->x1a0[pc1->i];
	pc1->cx = 0x015a;
	pc1->bx = 0x4e35;

	pc1->tmp = pc1->ax;
	pc1->ax = pc1->si;
	pc1->si = pc1->tmp;

	pc1->tmp = pc1->ax;
	pc1->ax = pc1->dx;
	pc1->dx = pc1->tmp;

	if (pc1->ax != 0) {
		pc1->ax = pc1->ax * pc1->bx;
	}

	pc1->tmp = pc1->ax;
	pc1->ax = pc1->cx;
	pc1->cx = pc1->tmp;

	if (pc1->ax != 0) {
		pc1->ax = pc1->ax * pc1->si;
		pc1->cx = pc1->ax + pc1->cx;
	}

	pc1->tmp = pc1->ax;
	pc1->ax = pc1->si;
	pc1->si = pc1->tmp;
	pc1->ax = pc1->ax * pc1->bx;
	pc1->dx = pc1->cx + pc1->dx;

	pc1->ax = pc1->ax + 1;

	pc1->x1a2 = pc1->dx;
	pc1->x1a0[pc1->i] = pc1->ax;

	pc1->res = pc1->ax ^ pc1->dx;
	pc1->i = pc1->i + 1;
}

static void pc1_assemble(struct pc1_ctx *pc1)
{
	pc1->x1a0[0] = (pc1->cle[0] * 256) + pc1->cle[1];

	pc1_code(pc1);
	pc1->inter = pc1->res;

	pc1->x1a0[1] = pc1->x1a0[0] ^ ((pc1->cle[2]*256) + pc1->cle[3]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[2] = pc1->x1a0[1] ^ ((pc1->cle[4]*256) + pc1->cle[5]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[3] = pc1->x1a0[2] ^ ((pc1->cle[6]*256) + pc1->cle[7]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[4] = pc1->x1a0[3] ^ ((pc1->cle[8]*256) + pc1->cle[9]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[5] = pc1->x1a0[4] ^ ((pc1->cle[10]*256) + pc1->cle[11]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[6] = pc1->x1a0[5] ^ ((pc1->cle[12]*256) + pc1->cle[13]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->x1a0[7] = pc1->x1a0[6] ^ ((pc1->cle[14]*256) + pc1->cle[15]);
	pc1_code(pc1);
	pc1->inter = pc1->inter ^ pc1->res;

	pc1->i = 0;
}

static unsigned char pc1_decrypt(struct pc1_ctx *pc1, short c)
{
	pc1_assemble(pc1);
	pc1->cfc = pc1->inter >> 8;
	pc1->cfd = pc1->inter & 255; /* cfc^cfd = random byte */

	c = c ^ (pc1->cfc ^ pc1->cfd);
	for (pc1->compte = 0; pc1->compte <= 15; pc1->compte++) {
		/* we mix the plaintext byte with the key */
		pc1->cle[pc1->compte] = pc1->cle[pc1->compte] ^ c;
	}

	return c;
}

static unsigned char pc1_encrypt(struct pc1_ctx *pc1, short c)
{
	pc1_assemble(pc1);
	pc1->cfc = pc1->inter >> 8;
	pc1->cfd = pc1->inter & 255; /* cfc^cfd = random byte */

	for (pc1->compte = 0; pc1->compte <= 15; pc1->compte++) {
		/* we mix the plaintext byte with the key */
		pc1->cle[pc1->compte] = pc1->cle[pc1->compte] ^ c;
	}
	c = c ^ (pc1->cfc ^ pc1->cfd);

	return c;
}

static void pc1_init(struct pc1_ctx *pc1)
{
	memset(pc1, 0, sizeof(struct pc1_ctx));

	/* ('Remsaalps!123456') is the key used, you can change it */
	strcpy(pc1->cle, "Remsaalps!123456");
}

static void pc1_decrypt_buf(struct pc1_ctx *pc1, unsigned char *buf,
			    unsigned len)
{
	unsigned i;

	for (i = 0; i < len; i++)
		buf[i] = pc1_decrypt(pc1, buf[i]);
}

static void pc1_encrypt_buf(struct pc1_ctx *pc1, unsigned char *buf,
			    unsigned len)
{
	unsigned i;

	for (i = 0; i < len; i++)
		buf[i] = pc1_encrypt(pc1, buf[i]);
}

/*
 * Globals
 */
static char *ifname;
static char *progname;
static char *ofname;
static int decrypt;

/*
 * Message macros
 */
#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ": %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -d              decrypt instead of encrypt"
"  -i <file>       read input from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

#define BUFSIZE		(64 * 1024)

int main(int argc, char *argv[])
{
	struct pc1_ctx pc1;
	int res = EXIT_FAILURE;
	int err;
	struct stat st;
	char *buf;
	unsigned total;

	FILE *outfile, *infile;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "di:o:h");
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			decrypt = 1;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	if (ifname == NULL) {
		ERR("no input file specified");
		goto err;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		goto err;
	}

	err = stat(ifname, &st);
	if (err){
		ERRS("stat failed on %s", ifname);
		goto err;
	}

	total = st.st_size;
	buf = malloc(BUFSIZE);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto err;
	}

	infile = fopen(ifname, "r");
	if (infile == NULL) {
		ERRS("could not open \"%s\" for reading", ifname);
		goto err_free;
	}

	outfile = fopen(ofname, "w");
	if (outfile == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto err_close_in;
	}

	pc1_init(&pc1);
	while (total > 0) {
		unsigned datalen;

		if (total > BUFSIZE)
			datalen = BUFSIZE;
		else
			datalen = total;

		errno = 0;
		fread(buf, datalen, 1, infile);
		if (errno != 0) {
			ERRS("unable to read from file %s", ifname);
			goto err_close_out;
		}

		if (decrypt)
			pc1_decrypt_buf(&pc1, buf, datalen);
		else
			pc1_encrypt_buf(&pc1, buf, datalen);

		errno = 0;
		fwrite(buf, datalen, 1, outfile);
		if (errno) {
			ERRS("unable to write to file %s", ofname);
			goto err_close_out;
		}

		total -= datalen;
	}
	pc1_finish(&pc1);

	res = EXIT_SUCCESS;

 out_flush:
	fflush(outfile);

 err_close_out:
	fclose(outfile);
	if (res != EXIT_SUCCESS) {
		unlink(ofname);
	}

 err_close_in:
	fclose(infile);

 err_free:
	free(buf);

 err:
	return res;
}

