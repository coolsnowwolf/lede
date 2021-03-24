/*
 * lmo - Lua Machine Objects - General header
 *
 *   Copyright (C) 2009-2012 Jo-Philipp Wich <xm@subsignal.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef _TEMPLATE_LMO_H_
#define _TEMPLATE_LMO_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fnmatch.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>

#if (defined(__GNUC__) && defined(__i386__))
#define sfh_get16(d) (*((const uint16_t *) (d)))
#else
#define sfh_get16(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
					   +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif


struct lmo_entry {
	uint32_t key_id;
	uint32_t val_id;
	uint32_t offset;
	uint32_t length;
} __attribute__((packed));

typedef struct lmo_entry lmo_entry_t;


struct lmo_archive {
	int         fd;
	int	        length;
	uint32_t    size;
	lmo_entry_t *index;
	char        *mmap;
	char		*end;
	struct lmo_archive *next;
};

typedef struct lmo_archive lmo_archive_t;


struct lmo_catalog {
	char lang[6];
	struct lmo_archive *archives;
	struct lmo_catalog *next;
};

typedef struct lmo_catalog lmo_catalog_t;


uint32_t sfh_hash(const char *data, int len);
uint32_t lmo_canon_hash(const char *data, int len);

lmo_archive_t * lmo_open(const char *file);
void lmo_close(lmo_archive_t *ar);


extern lmo_catalog_t *_lmo_catalogs;
extern lmo_catalog_t *_lmo_active_catalog;

int lmo_load_catalog(const char *lang, const char *dir);
int lmo_change_catalog(const char *lang);
int lmo_translate(const char *key, int keylen, char **out, int *outlen);
void lmo_close_catalog(const char *lang);

#endif
