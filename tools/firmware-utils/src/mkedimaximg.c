/*
 * Copyright (C) 2011 Vasilis Tsiligiannis <b_tsiligiannis@silverton.gr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <endian.h>	/* for __BYTE_ORDER */

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_LE16(x)	(x)
#  define HOST_TO_LE32(x)	(x)
#else
#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#endif

struct header
{
    unsigned char sign[4];
    unsigned int start;
    unsigned int flash;
    unsigned char model[4];
    unsigned int size;
} __attribute__ ((packed));

struct finfo
{
    char *name;
    off_t size;
};

struct buf
{
    char *start;
    size_t size;
};

static char *progname;

static void usage(int status)
{
    FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

    fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
    fprintf(stream,
	    "\n"
	    "Options:\n"
	    "  -s <sig>        set image signature to <sig>\n"
	    "  -m <model>      set model to <model>\n"
	    "  -i <file>       read input from file <file>\n"
	    "  -o <file>       write output to file <file>\n"
	    "  -f <flash>      set flash address to <flash>\n"
	    "  -S <start>      set start address to <start>\n");

    exit(status);
}

static int strtou32(char *arg, unsigned int *val)
{
    char *endptr = NULL;

    errno = 0;
    *val = strtoul(arg, &endptr, 0);
    if (errno || (endptr == arg) || (*endptr && (endptr != NULL))) {
	return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

static unsigned short fwcsum (struct buf *buf) {
    int i;
    unsigned short ret = 0;

    for (i = 0; i < buf->size / 2; i++)
	ret -= ((unsigned short *) buf->start)[i];
    
    return ret;
}

static int fwread(struct finfo *finfo, struct buf *buf)
{
    FILE *f;
    
    f = fopen(finfo->name, "r");
    if (!f) {
	fprintf(stderr, "could not open \"%s\" for reading\n", finfo->name);
	usage(EXIT_FAILURE);
    }

    buf->size = fread(buf->start, 1, finfo->size, f);
    if (buf->size != finfo->size) {
	fprintf(stderr, "unable to read from file \"%s\"\n", finfo->name);
	usage(EXIT_FAILURE);
    }

    fclose(f);

    return EXIT_SUCCESS;
}

static int fwwrite(struct finfo *finfo, struct buf *buf)
{
    FILE *f;

    f = fopen(finfo->name, "w");
    if (!f) {
	fprintf(stderr, "could not open \"%s\" for writing\n", finfo->name);
	usage(EXIT_FAILURE);
    }

    buf->size = fwrite(buf->start, 1, finfo->size, f);
    if (buf->size != finfo->size) {
	fprintf(stderr, "unable to write to file \"%s\"\n", finfo->name);
	usage(EXIT_FAILURE);
    }

    fclose(f);

    return EXIT_SUCCESS;
}	
  
int main(int argc, char **argv)
{
    struct stat st;
    struct header header;
    struct buf ibuf, obuf;
    struct finfo ifinfo, ofinfo;
    unsigned short csum;
    int c;

    ifinfo.name = ofinfo.name = NULL;
    header.flash = header.size = header.start = 0;
    progname = basename(argv[0]);

    while((c = getopt(argc, argv, "i:o:m:s:f:S:h")) != -1) {
	switch (c) {
	case 'i':
	    ifinfo.name = optarg;
	    break;
	case 'o':
	    ofinfo.name = optarg;
	    break;
	case 'm':
	    if (strlen(optarg) != 4) {
		fprintf(stderr, "model must be 4 characters long\n");
		usage(EXIT_FAILURE);
	    }
	    memcpy(header.model, optarg, 4);
	    break;
	case 's':
	    if (strlen(optarg) != 4) {
		fprintf(stderr, "signature must be 4 characters long\n");
		usage(EXIT_FAILURE);
	    }
	    memcpy(header.sign, optarg, 4);
	    break;
	case 'h':
	    usage(EXIT_SUCCESS);
	    break;
	case 'f':
	    if (!strtou32(optarg, &header.flash)) {
		fprintf(stderr, "invalid flash address specified\n");
		usage(EXIT_FAILURE);
	    }
	    break;
	case 'S':
	    if (!strtou32(optarg, &header.start)) {
		fprintf(stderr, "invalid start address specified\n");
		usage(EXIT_FAILURE);
	    }
	    break;
	default:
	    usage(EXIT_FAILURE);
	    break;
	}
    }

    if (ifinfo.name == NULL) {
	fprintf(stderr, "no input file specified\n");
	usage(EXIT_FAILURE);
    }

    if (ofinfo.name == NULL) {
	fprintf(stderr, "no output file specified\n");
	usage(EXIT_FAILURE);
    }

    if (stat(ifinfo.name, &st)) {
	fprintf(stderr, "stat failed on %s\n", ifinfo.name);
	usage(EXIT_FAILURE);
    }

    if (header.sign == NULL) {
	fprintf(stderr, "no signature specified\n");
	usage(EXIT_FAILURE);
    }

    if (header.model == NULL) {
	fprintf(stderr, "no model specified\n");
	usage(EXIT_FAILURE);
    }

    if (!header.flash) {
	fprintf(stderr, "no flash address specified\n");
	usage(EXIT_FAILURE);
    }

    if (!header.start) {
	fprintf(stderr, "no start address specified\n");
	usage(EXIT_FAILURE);
    }

    ifinfo.size = st.st_size;

    obuf.size = ifinfo.size + sizeof(struct header) + sizeof(unsigned short);
    if (obuf.size % sizeof(unsigned short))
	obuf.size++;

    obuf.start = malloc(obuf.size);
    if (!obuf.start) {
	fprintf(stderr, "no memory for buffer\n");
	usage(EXIT_FAILURE);
    }
    memset(obuf.start, 0, obuf.size);

    ibuf.size = ifinfo.size;
    ibuf.start = obuf.start + sizeof(struct header);
    
    if (fwread(&ifinfo, &ibuf))
	usage(EXIT_FAILURE);

    header.flash = HOST_TO_LE32(header.flash);
    header.size = HOST_TO_LE32(obuf.size - sizeof(struct header));
    header.start = HOST_TO_LE32(header.start);
    memcpy (obuf.start, &header, sizeof(struct header));

    csum = HOST_TO_LE16(fwcsum(&ibuf));
    memcpy(obuf.start + obuf.size - sizeof(unsigned short),
	   &csum, sizeof(unsigned short));

    ofinfo.size = obuf.size;

    if (fwwrite(&ofinfo, &obuf))
	usage(EXIT_FAILURE);

    return EXIT_SUCCESS;
}
