/*
 * FIS table updating code for mtd
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <sys/mman.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "crc32.h"
#include "mtd.h"
#include "fis.h"

struct fis_image_hdr {
	unsigned char name[16];
	uint32_t flash_base;
	uint32_t mem_base;
	uint32_t size;
	uint32_t entry_point;
	uint32_t data_length;
} __attribute__((packed));

struct fis_image_crc {
	uint32_t desc;
	uint32_t file;
} __attribute__((packed));

struct fis_image_desc {
	struct fis_image_hdr hdr;
	char _pad[256 - sizeof(struct fis_image_hdr) - sizeof(struct fis_image_crc)];
	struct fis_image_crc crc;
} __attribute__((packed));

static int fis_fd = -1;
static struct fis_image_desc *fis_desc;
static int fis_erasesize = 0;

static void
fis_close(void)
{
	if (fis_desc)
		munmap(fis_desc, fis_erasesize);

	if (fis_fd >= 0)
		close(fis_fd);

	fis_fd = -1;
	fis_desc = NULL;
}

static struct fis_image_desc *
fis_open(void)
{
	struct fis_image_desc *desc;

	if (fis_fd >= 0)
		fis_close();

	fis_fd = mtd_check_open("FIS directory");
	if (fis_fd < 0)
		goto error;

	close(fis_fd);
	fis_fd = mtd_open("FIS directory", true);
	if (fis_fd < 0)
		goto error;

	fis_erasesize = erasesize;
	desc = mmap(NULL, erasesize, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, fis_fd, 0);
	if (desc == MAP_FAILED)
		goto error;

	fis_desc = desc;
	return desc;

error:
	fis_close();
	return NULL;
}

int
fis_validate(struct fis_part *old, int n_old, struct fis_part *new, int n_new)
{
	struct fis_image_desc *desc;
	void *end;
	int found = 0;
	int i;

	desc = fis_open();
	if (!desc)
		return -1;

	for (i = 0; i < n_new - 1; i++) {
		if (!new[i].size) {
			fprintf(stderr, "FIS error: only the last partition can detect the size automatically\n");
			i = -1;
			goto done;
		}
	}

	end = desc;
	end = (char *) end + fis_erasesize;
	while ((void *) desc < end) {
		if (!desc->hdr.name[0] || (desc->hdr.name[0] == 0xff))
			break;

		for (i = 0; i < n_old; i++) {
			if (!strncmp((char *) desc->hdr.name, (char *) old[i].name, sizeof(desc->hdr.name))) {
				found++;
				goto next;
			}
		}
next:
		desc++;
		continue;
	}

	if (found == n_old)
		i = 1;
	else
		i = -1;

done:
	fis_close();
	return i;
}

int
fis_remap(struct fis_part *old, int n_old, struct fis_part *new, int n_new)
{
	struct fis_image_desc *first = NULL;
	struct fis_image_desc *last = NULL;
	struct fis_image_desc *first_fb = NULL;
	struct fis_image_desc *last_fb = NULL;
	struct fis_image_desc *desc;
	struct fis_part *part;
	uint32_t offset = 0, size = 0;
	char *start, *end, *tmp;
	int i;

	desc = fis_open();
	if (!desc)
		return -1;

	if (!quiet)
		fprintf(stderr, "Updating FIS table... \n");

	start = (char *) desc;
	end = (char *) desc + fis_erasesize;
	while ((char *) desc < end) {
		if (!desc->hdr.name[0] || (desc->hdr.name[0] == 0xff))
			break;

		/* update max offset */
		if (offset < desc->hdr.flash_base)
			offset = desc->hdr.flash_base;

		for (i = 0; i < n_old; i++) {
			if (!strncmp((char *) desc->hdr.name, (char *) old[i].name, sizeof(desc->hdr.name))) {
				last = desc;
				if (!first)
					first = desc;
				break;
			}
		}
		desc++;
	}
	desc--;

	first_fb = first;
	last_fb = last;

	if (first_fb->hdr.flash_base > last_fb->hdr.flash_base) {
		first_fb = last;
		last_fb = first;
	}

	/* determine size of available space */
	desc = (struct fis_image_desc *) start;
	while ((char *) desc < end) {
		if (!desc->hdr.name[0] || (desc->hdr.name[0] == 0xff))
			break;

		if (desc->hdr.flash_base > last_fb->hdr.flash_base &&
		    desc->hdr.flash_base < offset)
			offset = desc->hdr.flash_base;

		desc++;
	}
	desc--;

	size = offset - first_fb->hdr.flash_base;

	last++;
	desc = first + n_new;
	offset = first_fb->hdr.flash_base;

	if (desc != last) {
		if (desc > last)
			tmp = (char *) desc;
		else
			tmp = (char *) last;

		memmove(desc, last, end - tmp);
		if (desc < last) {
			tmp = end - (last - desc) * sizeof(struct fis_image_desc);
			memset(tmp, 0xff, tmp - end);
		}
	}

	for (part = new, desc = first; desc < first + n_new; desc++, part++) {
		memset(desc, 0, sizeof(struct fis_image_desc));
		memcpy(desc->hdr.name, part->name, sizeof(desc->hdr.name));
		desc->crc.desc = 0;
		desc->crc.file = part->crc;

		desc->hdr.flash_base = offset;
		desc->hdr.mem_base = part->loadaddr;
		desc->hdr.entry_point = part->loadaddr;
		desc->hdr.size = (part->size > 0) ? part->size : size;
		desc->hdr.data_length = (part->length > 0) ? part->length :
								desc->hdr.size;
		offset += desc->hdr.size;
		size -= desc->hdr.size;
	}

	msync(fis_desc, fis_erasesize, MS_SYNC|MS_INVALIDATE);
	fis_close();

	return 0;
}
