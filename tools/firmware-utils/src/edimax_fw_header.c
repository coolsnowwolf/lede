/*
 * Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
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

#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_MAGIC_LEN		16
#define MAX_MODEL_LEN		32
#define MAX_VERSION_LEN		14
#define MAX_MTD_NAME_LEN	16

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

struct edimax_header {
	char		magic[MAX_MAGIC_LEN];
	char		model[MAX_MODEL_LEN];
	unsigned char	force;
	unsigned char	header_csum;
	unsigned char	data_csum;
	uint32_t	data_size;
	uint32_t	start_addr;
	uint32_t	end_addr;
	char		fw_version[MAX_VERSION_LEN];
	unsigned char	type;
	char		mtd_name[MAX_MTD_NAME_LEN];
} __attribute__ ((packed));

/*
 * Globals
 */
static char *ofname;
static char *ifname;
static char *progname;

static char *model;
static char *magic = "eDiMaX";
static char *fw_version = "";
static char *mtd_name;
static int force;
static uint32_t start_addr;
static uint32_t end_addr;
static uint8_t image_type;
static int data_size;

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
	fprintf(stderr, "[%s] *** error: " fmt " (%s)\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define DBG(fmt, ...) do { \
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -e <addr>       set end addr to <addr>\n"
"  -f              set force flag\n"
"  -h              show this screen\n"
"  -i <file>       read input data from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -m <model>      set model to <model>\n"
"  -M <magic>      set image magic to <magic>\n"
"  -n <name>       set MTD device name to <name>\n"
"  -s <addr>       set start address to <addr>\n"
"  -t <type>       set image type to <type>\n"
"  -v <version>    set firmware version to <version>\n"
	);

	exit(status);
}

int
str2u32(char *arg, uint32_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno=0;
	t = strtoul(arg, &err, 0);
	if (errno || (err==arg) || ((err != NULL) && *err)) {
		return -1;
	}

	*val = t;
	return 0;
}

int
str2u8(char *arg, uint8_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno=0;
	t = strtoul(arg, &err, 0);
	if (errno || (err==arg) || ((err != NULL) && *err) || (t >= 0x100)) {
		return -1;
	}

	*val = t & 0xFF;
	return 0;
}

static int get_file_size(char *name)
{
	struct stat st;
	int res;

	res = stat(name, &st);
	if (res){
		ERRS("stat failed on %s", name);
		return -1;
	}

	return st.st_size;
}

static int read_to_buf(char *name, char *buf, int buflen)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(name, "r");
	if (f == NULL) {
		ERRS("could not open \"%s\" for reading", name);
		goto out;
	}

	errno = 0;
	fread(buf, buflen, 1, f);
	if (errno != 0) {
		ERRS("unable to read from file \"%s\"", name);
		goto out_close;
	}

	ret = EXIT_SUCCESS;

out_close:
	fclose(f);
out:
	return ret;
}

static int check_options(void)
{
#define CHKSTR(_name, _msg)				\
	do {						\
		if (_name == NULL) {			\
			ERR("no %s specified", _msg);	\
			return -1;			\
		}					\
	} while (0)

#define CHKSTRLEN(_name, _msg)						\
	do {								\
		int field_len;						\
		CHKSTR(_name, _msg);					\
		field_len = FIELD_SIZEOF(struct edimax_header, _name) - 1; \
		if (strlen(_name) > field_len) { 			\
			ERR("'%s' is too long, max %s length is %d",	\
			    _name, _msg, field_len);			\
			return -1;					\
		}							\
	} while (0)

	CHKSTR(ofname, "output file");
	CHKSTR(ifname, "input file");

	CHKSTRLEN(magic, "magic");
	CHKSTRLEN(model, "model");
	CHKSTRLEN(mtd_name, "MTD device name");
	CHKSTRLEN(fw_version, "firware version");

	data_size = get_file_size(ifname);
	if (data_size < 0)
		return -1;

	return 0;
}

static int write_fw(char *data, int len)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(ofname, "w");
	if (f == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto out;
	}

	errno = 0;
	fwrite(data, len, 1, f);
	if (errno) {
		ERRS("unable to write output file");
		goto out_flush;
	}

	DBG("firmware file \"%s\" completed", ofname);

	ret = EXIT_SUCCESS;

out_flush:
	fflush(f);
	fclose(f);
	if (ret != EXIT_SUCCESS) {
		unlink(ofname);
	}
out:
	return ret;
}

static unsigned char checksum(unsigned char *p, unsigned len)
{
	unsigned char csum = 0;

	while (len--)
		csum += *p++;

	csum ^= 0xb9;

	return csum;
}

static int build_fw(void)
{
	int buflen;
	char *buf;
	char *data;
	struct edimax_header *hdr;
	int ret = EXIT_FAILURE;

	buflen = sizeof(struct edimax_header) + data_size;

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	data = buf + sizeof(struct edimax_header);

	/* read input file */
	ret = read_to_buf(ifname, data, data_size);
	if (ret)
		goto out_free_buf;

	/* fill firmware header */
	hdr = (struct edimax_header *)buf;
	memset(hdr, 0, sizeof(struct edimax_header));

	strncpy(hdr->model, model, sizeof(hdr->model));
	strncpy(hdr->magic, magic, sizeof(hdr->magic));
	strncpy(hdr->fw_version, fw_version, sizeof(hdr->fw_version));
	strncpy(hdr->mtd_name, mtd_name, sizeof(hdr->mtd_name));

	hdr->force = force;
	hdr->start_addr = htonl(start_addr);
	hdr->end_addr = htonl(end_addr);
	hdr->data_size = htonl(data_size);
	hdr->type = image_type;

	hdr->data_csum = checksum((unsigned char *)data, data_size);
	hdr->header_csum = checksum((unsigned char *)hdr,
				    sizeof(struct edimax_header));

	ret = write_fw(buf, buflen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

out_free_buf:
	free(buf);
out:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	progname = basename(argv[0]);

	while (1) {
		int c;

		c = getopt(argc, argv, "e:fhi:o:m:M:n:s:t:v:");
		if (c == -1)
			break;

		switch (c) {
		case 'e':
			if (str2u32(optarg, &end_addr)) {
				ERR("%s is invalid '%s'",
				    "end address", optarg);
				goto out;
			}
			break;
		case 'f':
			force = 1;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'm':
			model = optarg;
			break;
		case 'M':
			magic = optarg;
			break;
		case 'n':
			mtd_name = optarg;
			break;
		case 's':
			if (str2u32(optarg, &start_addr)) {
				ERR("%s is invalid '%s'",
				    "start address", optarg);
				goto out;
			}
			break;
		case 't':
			if (str2u8(optarg, &image_type)) {
				ERR("%s is invalid '%s'",
				    "image type", optarg);
				goto out;
			}
			break;
		case 'v':
			fw_version = optarg;
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	ret = check_options();
	if (ret)
		goto out;

	ret = build_fw();

out:
	return ret;
}
