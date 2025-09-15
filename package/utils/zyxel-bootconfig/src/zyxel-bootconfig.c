/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright (C) 2022 David Bauer <mail@david-bauer.net> */

/*
 * First byte: Image status
 *
 * Possible status-codes:
 * 0x0: none
 * 0x1: new
 * 0x2: valid
 * 0x3: invalid
 *
 * Example: Image 0 valid; Image 1 invalid
 * 11001000
 * ||  ||
 * img1||
 *     img0
 *
 * Second byte: Active Image
 * Possible values:
 * 0x0: Image0 active
 * 0x1: Image1 active
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#define BOOTCONFIG_SIZE			0x20
#define BOOTCONFIG_IMAGE_STATUS		0x0
#define BOOTCONFIG_ACTIVE_IMAGE		0x1

#define IMAGE_0_SHIFT			2
#define IMAGE_0_MASK			0x0c
#define IMAGE_1_SHIFT			6
#define IMAGE_1_MASK			0xc0

#define IMAGE_STATUS(img0, img1)	(((img0 << IMAGE_0_SHIFT) & IMAGE_0_MASK) | ((img1 << IMAGE_1_SHIFT) & IMAGE_1_MASK))

#define ACTIVE_IMAGE_MASK		0x1
#define ACTIVE_IMAGE(img)		(img & ACTIVE_IMAGE_MASK)

enum zyxel_bootconfig_image_status {
	IMAGE_STATUS_NONE = 0x0,
	IMAGE_STATUS_NEW = 0x1,
	IMAGE_STATUS_VALID = 0x2,
	IMAGE_STATUS_INVALID = 0x3,
	__IMAGE_STATUS_EINVAL,
};

struct zyxel_bootconfig {
	enum zyxel_bootconfig_image_status image0_status;
	enum zyxel_bootconfig_image_status image1_status;
	unsigned int active_image;
};

struct zyxel_bootconfig_mtd {
	struct mtd_info_user mtd_info;
	int fd;
};

struct zyxel_image_status {
	enum zyxel_bootconfig_image_status code;
	const char *name;
};

struct zyxel_image_status image_status_codes[] = {
	{ .code = IMAGE_STATUS_NONE, .name = "none" },
	{ .code = IMAGE_STATUS_NEW, .name = "new" },
	{ .code = IMAGE_STATUS_VALID, .name = "valid" },
	{ .code = IMAGE_STATUS_INVALID, .name = "invalid" },
	{},
};


static enum zyxel_bootconfig_image_status zyxel_bootconfig_image_status_parse(const char *status) {
	struct zyxel_image_status* s;

	for (s = image_status_codes; s->name; s++) {
		if (!strcmp(status, s->name)) {
			return s->code;
		}
	}

	return __IMAGE_STATUS_EINVAL;
}

const char *zyxel_bootconfig_image_status_name(const enum zyxel_bootconfig_image_status bootconfig) {
	struct zyxel_image_status* s;

	for (s = image_status_codes; s->name; s++) {
		if (bootconfig == s->code) {
			return s->name;
		}
	}

	return "N/A";
}

static void zyxel_bootconfig_mtd_close(struct zyxel_bootconfig_mtd *mtd) {
	close(mtd->fd);
}


static int zyxel_bootconfig_mtd_open(struct zyxel_bootconfig_mtd *mtd, const char *mtd_name) {
	int ret = 0;

	mtd->fd = open(mtd_name, O_RDWR | O_SYNC);
	if (mtd->fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd_name);
		ret = -1;
		goto out;
	}

	if (ioctl(mtd->fd, MEMGETINFO, &mtd->mtd_info)) {
		fprintf(stderr, "Could not get MTD device info from %s\n", mtd_name);
		ret = -1;
		zyxel_bootconfig_mtd_close(mtd);
		goto out;
	}

out:
	return ret;
}


static int zyxel_bootconfig_read(struct zyxel_bootconfig *config, struct zyxel_bootconfig_mtd *mtd) {
	char *args = NULL;
	int ret = 0;

	/* Allocate memory for reading boot-config partition */
	args = calloc(1, mtd->mtd_info.erasesize);
	if (!args) {
		fprintf(stderr, "Could not allocate memory!\n");
		ret = -1;
		goto out;
	}

	/* Read bootconfig partition */
	pread(mtd->fd, args, mtd->mtd_info.erasesize, 0);

	/* Parse config */
	memset(config, 0, sizeof(*config));

	config->image0_status = (args[BOOTCONFIG_IMAGE_STATUS] & IMAGE_0_MASK) >> IMAGE_0_SHIFT;
	config->image1_status = (args[BOOTCONFIG_IMAGE_STATUS] & IMAGE_1_MASK) >> IMAGE_1_SHIFT;
	config->active_image = (args[BOOTCONFIG_ACTIVE_IMAGE] & ACTIVE_IMAGE_MASK);

out:
	if (args)
		free(args);
	return ret;
}


static int zyxel_bootconfig_write(struct zyxel_bootconfig *config, struct zyxel_bootconfig_mtd *mtd)
{
	struct erase_info_user erase_info;
	char img_status, img_active;
	char *args = NULL;
	int ret = 0;

	/* Allocate memory for reading boot-config partition */
	args = calloc(1, mtd->mtd_info.erasesize);
	if (!args) {
		fprintf(stderr, "Could not allocate memory!\n");
		ret = -1;
		goto out;
	}

	/* Read bootconfig partition */
	pread(mtd->fd, args, mtd->mtd_info.erasesize, 0);

	img_status = IMAGE_STATUS(config->image0_status, config->image1_status);
	img_active = ACTIVE_IMAGE(config->active_image);

	/* Check if bootconfig has to be written */
	if (args[BOOTCONFIG_IMAGE_STATUS] == img_status && args[BOOTCONFIG_ACTIVE_IMAGE] == img_active) {
		ret = 0;
		goto out;
	}

	/* Erase first block (containing the magic) */
	erase_info.start = 0;
	erase_info.length = mtd->mtd_info.erasesize;
	ret = ioctl(mtd->fd, MEMERASE, &erase_info);
	if (ret < 0) {
		fprintf(stderr, "Failed to erase block: %i\n", ret);
		goto out;
	}


	/* Write bootconfig */
	args[BOOTCONFIG_IMAGE_STATUS] = img_status;
	args[BOOTCONFIG_ACTIVE_IMAGE] = img_active;

	if (pwrite(mtd->fd, args, mtd->mtd_info.erasesize, 0) != mtd->mtd_info.erasesize) {
		fprintf(stderr, "Error writing bootconfig!\n");
		ret = -1;
		goto out;
	}

out:
	if (args)
		free(args);
	return ret;
}


static void zyxel_bootconfig_print_usage(char *programm)
{
	struct zyxel_image_status* s = image_status_codes;

	printf("Usage: %s <mtd-device> <command> [args]\n", programm);
	printf("Available commands:\n");
	printf("	get-status\n");
	printf("	set-image-status [0/1] [");

	while (s->name) {
		printf("%s", s->name);
		s++;

		if (s->name)
			printf(",");
	}

	printf("]\n");
	printf("	set-active-image [0/1]\n");
}

int main(int argc, char *argv[])
{
	enum zyxel_bootconfig_image_status image_status;
	struct zyxel_bootconfig_mtd mtd;
	struct zyxel_bootconfig config;
	const char *mtd_name, *command;
	bool writeback = false;
	int image_idx;

	if (argc < 3) {
		zyxel_bootconfig_print_usage(argv[0]);
		return 1;
	}

	mtd_name = argv[1];
	command = argv[2];

	if (zyxel_bootconfig_mtd_open(&mtd, mtd_name)) {
		fprintf(stderr, "Error opening %s!\n", mtd_name);
		return 1;
	}

	if (zyxel_bootconfig_read(&config, &mtd)) {
		fprintf(stderr, "Error reading bootconfig!\n");
		zyxel_bootconfig_mtd_close(&mtd);
		return 1;
	}

	if (!strcmp(command, "set-image-status")) {
		if (argc < 5) {
			zyxel_bootconfig_print_usage(argv[0]);
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		image_idx = atoi(argv[3]);
		if (image_idx > 1 || image_idx < 0) {
			fprintf(stderr, "Invalid image-slot set!\n");
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		image_status = zyxel_bootconfig_image_status_parse(argv[4]);
		if (image_status == __IMAGE_STATUS_EINVAL) {
			fprintf(stderr, "Invalid image-status!\n");
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		if (image_idx == 0) {
			config.image0_status = image_status;
		} else {
			config.image1_status = image_status;
		}

		writeback = true;
	} else if (!strcmp(command, "set-active-image")) {
		if (argc < 4) {
			zyxel_bootconfig_print_usage(argv[0]);
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		image_idx = atoi(argv[3]);
		if (image_idx > 1 || image_idx < 0) {
			fprintf(stderr, "Invalid image-slot set!\n");
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		config.active_image = image_idx;

		writeback = true;
	} else if (!strcmp(command, "get-status")) {
		printf("Active Image: %d\n", config.active_image);
		printf("Image 0 Status: %s\n", zyxel_bootconfig_image_status_name(config.image0_status));
		printf("Image 1 Status: %s\n", zyxel_bootconfig_image_status_name(config.image1_status));

		writeback = false;
	}

	if (writeback) {
		if (zyxel_bootconfig_write(&config, &mtd)) {
			fprintf(stderr, "Error writing bootconfig!\n");
			zyxel_bootconfig_mtd_close(&mtd);
			return 1;
		}

		zyxel_bootconfig_mtd_close(&mtd);
	}

	return 0;
}
