/*
 * jffs2 on-disk structure generator for mtd
 *
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * Based on:
 *   JFFS2 -- Journalling Flash File System, Version 2.
 *   Copyright © 2001-2007 Red Hat, Inc.
 *   Created by David Woodhouse <dwmw2@infradead.org>
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <endian.h>
#include "jffs2.h"
#include "crc32.h"
#include "mtd.h"

#define PAD(x) (((x)+3)&~3)

#if BYTE_ORDER == BIG_ENDIAN
# define CLEANMARKER "\x19\x85\x20\x03\x00\x00\x00\x0c\xf0\x60\xdc\x98"
#else
# define CLEANMARKER "\x85\x19\x03\x20\x0c\x00\x00\x00\xb1\xb0\x1e\xe4"
#endif

static int last_ino = 0;
static int last_version = 0;
static char *buf = NULL;
static int ofs = 0;
static int outfd = -1;
static int mtdofs = 0;
static int target_ino = 0;

static void prep_eraseblock(void);

static void pad(int size)
{
	if ((ofs % size == 0) && (ofs < erasesize))
		return;

	if (ofs < erasesize) {
		memset(buf + ofs, 0xff, (size - (ofs % size)));
		ofs += (size - (ofs % size));
	}
	ofs = ofs % erasesize;
	if (ofs == 0) {
		while (mtd_block_is_bad(outfd, mtdofs) && (mtdofs < mtdsize)) {
			if (!quiet)
				fprintf(stderr, "\nSkipping bad block at 0x%08x   ", mtdofs);

			mtdofs += erasesize;

			/* Move the file pointer along over the bad block. */
			lseek(outfd, erasesize, SEEK_CUR);
		}
		mtd_erase_block(outfd, mtdofs);
		write(outfd, buf, erasesize);
		mtdofs += erasesize;
	}
}

static inline int rbytes(void)
{
	return erasesize - (ofs % erasesize);
}

static inline void add_data(char *ptr, int len)
{
	if (ofs + len > erasesize) {
		pad(erasesize);
		prep_eraseblock();
	}
	memcpy(buf + ofs, ptr, len);
	ofs += len;
}

static void prep_eraseblock(void)
{
	if (ofs > 0)
		return;

	add_data(CLEANMARKER, sizeof(CLEANMARKER) - 1);
}

static int add_dirent(const char *name, const char type, int parent)
{
	struct jffs2_raw_dirent *de;

	if (ofs - erasesize < sizeof(struct jffs2_raw_dirent) + strlen(name))
		pad(erasesize);

	prep_eraseblock();
	last_ino++;
	memset(buf + ofs, 0, sizeof(struct jffs2_raw_dirent));
	de = (struct jffs2_raw_dirent *) (buf + ofs);

	de->magic = JFFS2_MAGIC_BITMASK;
	de->nodetype = JFFS2_NODETYPE_DIRENT;
	de->type = type;
	de->name_crc = crc32(0, name, strlen(name));
	de->ino = last_ino++;
	de->pino = parent;
	de->totlen = sizeof(*de) + strlen(name);
	de->hdr_crc = crc32(0, (void *) de, sizeof(struct jffs2_unknown_node) - 4);
	de->version = last_version++;
	de->mctime = 0;
	de->nsize = strlen(name);
	de->node_crc = crc32(0, (void *) de, sizeof(*de) - 8);
	memcpy(de->name, name, strlen(name));

	ofs += sizeof(struct jffs2_raw_dirent) + de->nsize;
	pad(4);

	return de->ino;
}

static int add_dir(const char *name, int parent)
{
	struct jffs2_raw_inode ri;
	int inode;

	inode = add_dirent(name, IFTODT(S_IFDIR), parent);

	if (rbytes() < sizeof(ri))
		pad(erasesize);
	prep_eraseblock();

	memset(&ri, 0, sizeof(ri));
	ri.magic = JFFS2_MAGIC_BITMASK;
	ri.nodetype = JFFS2_NODETYPE_INODE;
	ri.totlen = sizeof(ri);
	ri.hdr_crc = crc32(0, &ri, sizeof(struct jffs2_unknown_node) - 4);

	ri.ino = inode;
	ri.mode = S_IFDIR | 0755;
	ri.uid = ri.gid = 0;
	ri.atime = ri.ctime = ri.mtime = 0;
	ri.isize = ri.csize = ri.dsize = 0;
	ri.version = 1;
	ri.node_crc = crc32(0, &ri, sizeof(ri) - 8);
	ri.data_crc = 0;

	add_data((char *) &ri, sizeof(ri));
	pad(4);
	return inode;
}

static void add_file(const char *name, int parent)
{
	int inode, f_offset = 0, fd;
	struct jffs2_raw_inode ri;
	struct stat st;
	char wbuf[4096];
	const char *fname;

	if (stat(name, &st)) {
		fprintf(stderr, "File %s does not exist\n", name);
		return;
	}

	fname = strrchr(name, '/');
	if (fname)
		fname++;
	else
		fname = name;

	inode = add_dirent(fname, IFTODT(S_IFREG), parent);
	memset(&ri, 0, sizeof(ri));
	ri.magic = JFFS2_MAGIC_BITMASK;
	ri.nodetype = JFFS2_NODETYPE_INODE;

	ri.ino = inode;
	ri.mode = st.st_mode;
	ri.uid = ri.gid = 0;
	ri.atime = st.st_atime;
	ri.ctime = st.st_ctime;
	ri.mtime = st.st_mtime;
	ri.isize = st.st_size;
	ri.compr = 0;
	ri.usercompr = 0;

	fd = open(name, 0);
	if (fd < 0) {
		fprintf(stderr, "File %s does not exist\n", name);
		return;
	}

	for (;;) {
		int len = 0;

		for (;;) {
			len = rbytes() - sizeof(ri);
			if (len > 128)
				break;

			pad(erasesize);
			prep_eraseblock();
		}

		if (len > sizeof(wbuf))
			len = sizeof(wbuf);

		len = read(fd, wbuf, len);
		if (len <= 0)
			break;

		ri.totlen = sizeof(ri) + len;
		ri.hdr_crc = crc32(0, &ri, sizeof(struct jffs2_unknown_node) - 4);
		ri.version = ++last_version;
		ri.offset = f_offset;
		ri.csize = ri.dsize = len;
		ri.node_crc = crc32(0, &ri, sizeof(ri) - 8);
		ri.data_crc = crc32(0, wbuf, len);
		f_offset += len;
		add_data((char *) &ri, sizeof(ri));
		add_data(wbuf, len);
		pad(4);
		prep_eraseblock();
	}

	close(fd);
}

int mtd_replace_jffs2(const char *mtd, int fd, int ofs, const char *filename)
{
	outfd = fd;
	mtdofs = ofs;

	buf = malloc(erasesize);
	target_ino = 1;
	if (!last_ino)
		last_ino = 1;
	add_file(filename, target_ino);
	pad(erasesize);

	/* add eof marker, pad to eraseblock size and write the data */
	add_data(JFFS2_EOF, sizeof(JFFS2_EOF) - 1);
	pad(erasesize);
	free(buf);

	return (mtdofs - ofs);
}

void mtd_parse_jffs2data(const char *buf, const char *dir)
{
	struct jffs2_unknown_node *node = (struct jffs2_unknown_node *) buf;
	unsigned int ofs = 0;

	while (ofs < erasesize) {
		node = (struct jffs2_unknown_node *) (buf + ofs);
		if (node->magic != 0x1985)
			break;

		ofs += PAD(node->totlen);
		if (node->nodetype == JFFS2_NODETYPE_DIRENT) {
			struct jffs2_raw_dirent *de = (struct jffs2_raw_dirent *) node;

			/* is this the right directory name and is it a subdirectory of / */
			if (*dir && (de->pino == 1) && !strncmp((char *) de->name, dir, de->nsize))
				target_ino = de->ino;

			/* store the last inode and version numbers for adding extra files */
			if (last_ino < de->ino)
				last_ino = de->ino;
			if (last_version < de->version)
				last_version = de->version;
		}
	}
}

int mtd_write_jffs2(const char *mtd, const char *filename, const char *dir)
{
	int err = -1, fdeof = 0;

	outfd = mtd_check_open(mtd);
	if (outfd < 0)
		return -1;

	if (quiet < 2)
		fprintf(stderr, "Appending %s to jffs2 partition %s\n", filename, mtd);
	
	buf = malloc(erasesize);
	if (!buf) {
		fprintf(stderr, "Out of memory!\n");
		goto done;
	}

	if (!*dir)
		target_ino = 1;

	/* parse the structure of the jffs2 first
	 * locate the directory that the file is going to be placed in */
	for(;;) {
		struct jffs2_unknown_node *node = (struct jffs2_unknown_node *) buf;

		if (read(outfd, buf, erasesize) != erasesize) {
			fdeof = 1;
			break;
		}
		mtdofs += erasesize;

		if (node->magic == 0x8519) {
			fprintf(stderr, "Error: wrong endianness filesystem\n");
			goto done;
		}

		/* assume  no magic == end of filesystem
		 * the filesystem will probably end with be32(0xdeadc0de) */
		if (node->magic != 0x1985)
			break;

		mtd_parse_jffs2data(buf, dir);
	}

	if (fdeof) {
		fprintf(stderr, "Error: No room for additional data\n");
		goto done;
	}

	/* jump back one eraseblock */
	mtdofs -= erasesize;
	lseek(outfd, mtdofs, SEEK_SET);

	ofs = 0;

	if (!last_ino)
		last_ino = 1;

	if (!target_ino)
		target_ino = add_dir(dir, 1);

	add_file(filename, target_ino);
	pad(erasesize);

	/* add eof marker, pad to eraseblock size and write the data */
	add_data(JFFS2_EOF, sizeof(JFFS2_EOF) - 1);
	pad(erasesize);

	err = 0;

	if (trx_fixup) {
	  trx_fixup(outfd, mtd);
	}

done:
	close(outfd);
	if (buf)
		free(buf);

	return err;
}
