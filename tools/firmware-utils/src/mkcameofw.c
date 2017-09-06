/*
 * Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
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

#define MAX_MODEL_LEN		20
#define MAX_SIGNATURE_LEN	30
#define MAX_REGION_LEN		4
#define MAX_VERSION_LEN		12

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

struct file_info {
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */
	uint32_t	write_size;
};

struct img_header {
	uint32_t	checksum;
	uint32_t	image_size;
	uint32_t	kernel_size;
	char		model[MAX_MODEL_LEN];
	char		signature[MAX_SIGNATURE_LEN];
	char		region[MAX_REGION_LEN];
	char		version[MAX_VERSION_LEN];
	unsigned char	header_len;
	unsigned char	is_tgz;
	unsigned char	pad[4];
} __attribute__ ((packed));

/*
 * Globals
 */
static char *ofname;
static char *progname;

static char *model;
static char *signature;
static char *region = "DEF";
static char *version;
static struct file_info kernel_info;
static struct file_info rootfs_info;
static uint32_t kernel_size;
static uint32_t image_size;
static int combined;

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
"  -k <file>       read kernel image from the file <file>\n"
"  -c              use the kernel image as a combined image\n"
"  -M <model>      set model to <model>\n"
"  -o <file>       write output to the file <file>\n"
"  -r <file>       read rootfs image from the file <file>\n"
"  -S <signature>  set image signature to <signature>\n"
"  -R <region>     set image region to <region>\n"
"  -V <version>    set image version to <version>\n"
"  -I <size>       set image size to <size>\n"
"  -K <size>       set kernel size to <size>\n"
"  -h              show this screen\n"
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

static int get_file_stat(struct file_info *fdata)
{
	struct stat st;
	int res;

	if (fdata->file_name == NULL)
		return 0;

	res = stat(fdata->file_name, &st);
	if (res){
		ERRS("stat failed on %s", fdata->file_name);
		return res;
	}

	fdata->file_size = st.st_size;
	fdata->write_size = fdata->file_size;
	return 0;
}

static int read_to_buf(struct file_info *fdata, char *buf)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(fdata->file_name, "r");
	if (f == NULL) {
		ERRS("could not open \"%s\" for reading", fdata->file_name);
		goto out;
	}

	errno = 0;
	fread(buf, fdata->file_size, 1, f);
	if (errno != 0) {
		ERRS("unable to read from file \"%s\"", fdata->file_name);
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
	int ret;

#define CHKSTR(_name, _msg)				\
	do {						\
		if (_name == NULL) {			\
			ERR("no %s specified", _msg);	\
			return -1;			\
		}					\
	} while (0)

#define CHKSTRLEN(_name, _msg)					\
	do {							\
		int field_len;					\
		CHKSTR(_name, _msg);				\
		field_len = FIELD_SIZEOF(struct img_header, _name) - 1; \
		if (strlen(_name) > field_len) { 		\
			ERR("%s is too long, max length is %d",	\
			    _msg, field_len);			\
			return -1;				\
		}						\
	} while (0)

	CHKSTRLEN(model, "model");
	CHKSTRLEN(signature, "signature");
	CHKSTRLEN(region, "region");
	CHKSTRLEN(version, "version");
	CHKSTR(ofname, "output file");
	CHKSTR(kernel_info.file_name, "kernel image");

	ret = get_file_stat(&kernel_info);
	if (ret)
		return ret;

	if (combined) {
		if (!kernel_size) {
			ERR("kernel size must be specified for combined images");
			return -1;				\
		}

		if (!image_size)
			image_size = kernel_info.file_size;

		if (kernel_info.file_size > image_size) {
			ERR("kernel image is too big");
			return -1;
		}

		kernel_info.write_size = image_size;
	} else {
		CHKSTR(rootfs_info.file_name, "rootfs image");

		ret = get_file_stat(&rootfs_info);
		if (ret)
			return ret;

		if (kernel_size) {
			/* override kernel size */
			kernel_info.write_size = kernel_size;
		}

		if (image_size) {
			if (image_size < kernel_info.write_size)
				kernel_info.write_size = image_size;

			/* override rootfs size */
			rootfs_info.write_size = image_size - kernel_info.write_size;
		}

		if (kernel_info.file_size > kernel_info.write_size) {
			ERR("kernel image is too big");
			return -1;
		}

		if (rootfs_info.file_size > rootfs_info.write_size) {
			ERR("rootfs image is too big");
			return -1;
		}
	}

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

static uint32_t get_csum(unsigned char *p, uint32_t len)
{
	uint32_t csum = 0;

	while (len--)
		csum += *p++;

	return csum;
}

static int build_fw(void)
{
	int buflen;
	char *buf;
	char *p;
	uint32_t csum;
	struct img_header *hdr;
	int ret = EXIT_FAILURE;

	buflen = sizeof(struct img_header) +
		 kernel_info.write_size + rootfs_info.write_size;

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	memset(buf, 0, buflen);

	p = buf + sizeof(struct img_header);

	/* read kernel data */
	ret = read_to_buf(&kernel_info, p);
	if (ret)
		goto out_free_buf;

	if (!combined) {
		p += kernel_info.write_size;

		/* read rootfs data */
		ret = read_to_buf(&rootfs_info, p);
		if (ret)
			goto out_free_buf;
	}

	csum = get_csum((unsigned char *)(buf + sizeof(struct img_header)),
			buflen - sizeof(struct img_header));

	/* fill firmware header */
	hdr = (struct img_header *) buf;

	hdr->checksum = htonl(csum);
	hdr->image_size = htonl(buflen - sizeof(struct img_header));
	if (!combined)
		hdr->kernel_size = htonl(kernel_info.write_size);
	else
		hdr->kernel_size = htonl(kernel_size);
	hdr->header_len = sizeof(struct img_header);
	strncpy(hdr->model, model, sizeof(hdr->model));
	strncpy(hdr->signature, signature, sizeof(hdr->signature));
	strncpy(hdr->version, version, sizeof(hdr->version));
	strncpy(hdr->region, region, sizeof(hdr->region));

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

		c = getopt(argc, argv, "M:S:V:R:k:K:I:r:o:hc");
		if (c == -1)
			break;

		switch (c) {
		case 'M':
			model = optarg;
			break;
		case 'S':
			signature = optarg;
			break;
		case 'V':
			version = optarg;
			break;
		case 'R':
			region = optarg;
			break;
		case 'k':
			kernel_info.file_name = optarg;
			break;
		case 'K':
			if (str2u32(optarg, &kernel_size)) {
				ERR("%s is invalid '%s'",
				    "kernel size", optarg);
				goto out;
			}
			break;
		case 'I':
			if (str2u32(optarg, &image_size)) {
				ERR("%s is invalid '%s'",
				    "image size", optarg);
				goto out;
			}
			break;
		case 'r':
			rootfs_info.file_name = optarg;
			break;
		case 'c':
			combined = 1;
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

	ret = check_options();
	if (ret)
		goto out;

	ret = build_fw();

out:
	return ret;
}

