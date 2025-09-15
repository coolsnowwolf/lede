/*
 * NVRAM variable manipulation (common)
 *
 * Copyright 2004, Broadcom Corporation
 * Copyright 2009-2010, OpenWrt.org
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include "nvram.h"

#define TRACE(msg) \
	printf("%s(%i) in %s(): %s\n", \
		__FILE__, __LINE__, __FUNCTION__, msg ? msg : "?")

/* Size of "nvram" MTD partition */
size_t nvram_part_size = 0;


/*
 * -- Helper functions --
 */

/* String hash */
static uint32_t hash(const char *s)
{
	uint32_t hash = 0;

	while (*s)
		hash = 31 * hash + *s++;

	return hash;
}

/* Free all tuples. */
static void _nvram_free(nvram_handle_t *h)
{
	uint32_t i;
	nvram_tuple_t *t, *next;

	/* Free hash table */
	for (i = 0; i < NVRAM_ARRAYSIZE(h->nvram_hash); i++) {
		for (t = h->nvram_hash[i]; t; t = next) {
			next = t->next;
			if (t->value)
				free(t->value);
			free(t);
		}
		h->nvram_hash[i] = NULL;
	}

	/* Free dead table */
	for (t = h->nvram_dead; t; t = next) {
		next = t->next;
		if (t->value)
			free(t->value);
		free(t);
	}

	h->nvram_dead = NULL;
}

/* (Re)allocate NVRAM tuples. */
static nvram_tuple_t * _nvram_realloc( nvram_handle_t *h, nvram_tuple_t *t,
	const char *name, const char *value )
{
	if ((strlen(value) + 1) > h->length - h->offset)
		return NULL;

	if (!t) {
		if (!(t = malloc(sizeof(nvram_tuple_t) + strlen(name) + 1)))
			return NULL;

		/* Copy name */
		t->name = (char *) &t[1];
		strcpy(t->name, name);

		t->value = NULL;
	}

	/* Copy value */
	if (!t->value || strcmp(t->value, value))
	{
		if(!(t->value = (char *) realloc(t->value, strlen(value)+1)))
			return NULL;

		strcpy(t->value, value);
		t->value[strlen(value)] = '\0';
	}

	return t;
}

/* (Re)initialize the hash table. */
static int _nvram_rehash(nvram_handle_t *h)
{
	nvram_header_t *header = nvram_header(h);
	char buf[] = "0xXXXXXXXX", *name, *value, *eq;

	/* (Re)initialize hash table */
	_nvram_free(h);

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header[1];

	for (; *name; name = value + strlen(value) + 1) {
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		nvram_set(h, name, value);
		*eq = '=';
	}

	/* Set special SDRAM parameters */
	if (!nvram_get(h, "sdram_init")) {
		sprintf(buf, "0x%04X", (uint16_t)(header->crc_ver_init >> 16));
		nvram_set(h, "sdram_init", buf);
	}
	if (!nvram_get(h, "sdram_config")) {
		sprintf(buf, "0x%04X", (uint16_t)(header->config_refresh & 0xffff));
		nvram_set(h, "sdram_config", buf);
	}
	if (!nvram_get(h, "sdram_refresh")) {
		sprintf(buf, "0x%04X",
			(uint16_t)((header->config_refresh >> 16) & 0xffff));
		nvram_set(h, "sdram_refresh", buf);
	}
	if (!nvram_get(h, "sdram_ncdl")) {
		sprintf(buf, "0x%08X", header->config_ncdl);
		nvram_set(h, "sdram_ncdl", buf);
	}

	return 0;
}


/*
 * -- Public functions --
 */

/* Get nvram header. */
nvram_header_t * nvram_header(nvram_handle_t *h)
{
	return (nvram_header_t *) &h->mmap[h->offset];
}

/* Get the value of an NVRAM variable. */
char * nvram_get(nvram_handle_t *h, const char *name)
{
	uint32_t i;
	nvram_tuple_t *t;
	char *value;

	if (!name)
		return NULL;

	/* Hash the name */
	i = hash(name) % NVRAM_ARRAYSIZE(h->nvram_hash);

	/* Find the associated tuple in the hash table */
	for (t = h->nvram_hash[i]; t && strcmp(t->name, name); t = t->next);

	value = t ? t->value : NULL;

	return value;
}

/* Set the value of an NVRAM variable. */
int nvram_set(nvram_handle_t *h, const char *name, const char *value)
{
	uint32_t i;
	nvram_tuple_t *t, *u, **prev;

	/* Hash the name */
	i = hash(name) % NVRAM_ARRAYSIZE(h->nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &h->nvram_hash[i], t = *prev;
		 t && strcmp(t->name, name); prev = &t->next, t = *prev);

	/* (Re)allocate tuple */
	if (!(u = _nvram_realloc(h, t, name, value)))
		return -12; /* -ENOMEM */

	/* Value reallocated */
	if (t && t == u)
		return 0;

	/* Move old tuple to the dead table */
	if (t) {
		*prev = t->next;
		t->next = h->nvram_dead;
		h->nvram_dead = t;
	}

	/* Add new tuple to the hash table */
	u->next = h->nvram_hash[i];
	h->nvram_hash[i] = u;

	return 0;
}

/* Unset the value of an NVRAM variable. */
int nvram_unset(nvram_handle_t *h, const char *name)
{
	uint32_t i;
	nvram_tuple_t *t, **prev;

	if (!name)
		return 0;

	/* Hash the name */
	i = hash(name) % NVRAM_ARRAYSIZE(h->nvram_hash);

	/* Find the associated tuple in the hash table */
	for (prev = &h->nvram_hash[i], t = *prev;
		 t && strcmp(t->name, name); prev = &t->next, t = *prev);

	/* Move it to the dead table */
	if (t) {
		*prev = t->next;
		t->next = h->nvram_dead;
		h->nvram_dead = t;
	}

	return 0;
}

/* Get all NVRAM variables. */
nvram_tuple_t * nvram_getall(nvram_handle_t *h)
{
	int i;
	nvram_tuple_t *t, *l, *x;

	l = NULL;

	for (i = 0; i < NVRAM_ARRAYSIZE(h->nvram_hash); i++) {
		for (t = h->nvram_hash[i]; t; t = t->next) {
			if( (x = (nvram_tuple_t *) malloc(sizeof(nvram_tuple_t))) != NULL )
			{
				x->name  = t->name;
				x->value = t->value;
				x->next  = l;
				l = x;
			}
			else
			{
				break;
			}
		}
	}

	return l;
}

/* Regenerate NVRAM. */
int nvram_commit(nvram_handle_t *h)
{
	nvram_header_t *header = nvram_header(h);
	char *init, *config, *refresh, *ncdl;
	char *ptr, *end;
	int i;
	nvram_tuple_t *t;
	nvram_header_t tmp;
	uint8_t crc;

	/* Regenerate header */
	header->magic = NVRAM_MAGIC;
	header->crc_ver_init = (NVRAM_VERSION << 8);
	if (!(init = nvram_get(h, "sdram_init")) ||
		!(config = nvram_get(h, "sdram_config")) ||
		!(refresh = nvram_get(h, "sdram_refresh")) ||
		!(ncdl = nvram_get(h, "sdram_ncdl"))) {
		header->crc_ver_init |= SDRAM_INIT << 16;
		header->config_refresh = SDRAM_CONFIG;
		header->config_refresh |= SDRAM_REFRESH << 16;
		header->config_ncdl = 0;
	} else {
		header->crc_ver_init |= (strtoul(init, NULL, 0) & 0xffff) << 16;
		header->config_refresh = strtoul(config, NULL, 0) & 0xffff;
		header->config_refresh |= (strtoul(refresh, NULL, 0) & 0xffff) << 16;
		header->config_ncdl = strtoul(ncdl, NULL, 0);
	}

	/* Clear data area */
	ptr = (char *) header + sizeof(nvram_header_t);
	memset(ptr, 0xFF, nvram_part_size - h->offset - sizeof(nvram_header_t));
	memset(&tmp, 0, sizeof(nvram_header_t));

	/* Leave space for a double NUL at the end */
	end = (char *) header + nvram_part_size - h->offset - 2;

	/* Write out all tuples */
	for (i = 0; i < NVRAM_ARRAYSIZE(h->nvram_hash); i++) {
		for (t = h->nvram_hash[i]; t; t = t->next) {
			if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
				break;
			ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
		}
	}

	/* End with a double NULL and pad to 4 bytes */
	*ptr = '\0';
	ptr++;

	if( (int)ptr % 4 )
		memset(ptr, 0, 4 - ((int)ptr % 4));

	ptr++;

	/* Set new length */
	header->len = NVRAM_ROUNDUP(ptr - (char *) header, 4);

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init   = header->crc_ver_init;
	tmp.config_refresh = header->config_refresh;
	tmp.config_ncdl    = header->config_ncdl;
	crc = hndcrc8((unsigned char *) &tmp + NVRAM_CRC_START_POSITION,
		sizeof(nvram_header_t) - NVRAM_CRC_START_POSITION, 0xff);

	/* Continue CRC8 over data bytes */
	crc = hndcrc8((unsigned char *) &header[0] + sizeof(nvram_header_t),
		header->len - sizeof(nvram_header_t), crc);

	/* Set new CRC8 */
	header->crc_ver_init |= crc;

	/* Write out */
	msync(h->mmap, h->length, MS_SYNC);
	fsync(h->fd);

	/* Reinitialize hash table */
	return _nvram_rehash(h);
}

/* Open NVRAM and obtain a handle. */
nvram_handle_t * nvram_open(const char *file, int rdonly)
{
	int i;
	int fd;
	char *mtd = NULL;
	nvram_handle_t *h;
	nvram_header_t *header;
	int offset = -1;

	/* If erase size or file are undefined then try to define them */
	if( (nvram_part_size == 0) || (file == NULL) )
	{
		/* Finding the mtd will set the appropriate erase size */
		if( (mtd = nvram_find_mtd()) == NULL || nvram_part_size == 0 )
		{
			free(mtd);
			return NULL;
		}
	}

	if( (fd = open(file ? file : mtd, O_RDWR)) > -1 )
	{
		char *mmap_area = (char *) mmap(
			NULL, nvram_part_size, PROT_READ | PROT_WRITE,
			(( rdonly == NVRAM_RO ) ? MAP_PRIVATE : MAP_SHARED) | MAP_LOCKED, fd, 0);

		if( mmap_area != MAP_FAILED )
		{
			/*
			 * Start looking for NVRAM_MAGIC at beginning of MTD
			 * partition. Stop if there is less than NVRAM_MIN_SPACE
			 * to check, that was the lowest used size.
			 */
			for( i = 0; i <= ((nvram_part_size - NVRAM_MIN_SPACE) / sizeof(uint32_t)); i++ )
			{
				if( ((uint32_t *)mmap_area)[i] == NVRAM_MAGIC )
				{
					offset = i * sizeof(uint32_t);
					break;
				}
			}

			if( offset < 0 )
			{
				munmap(mmap_area, nvram_part_size);
				free(mtd);
				close(fd);
				return NULL;
			}
			else if( (h = malloc(sizeof(nvram_handle_t))) != NULL )
			{
				memset(h, 0, sizeof(nvram_handle_t));

				h->fd     = fd;
				h->mmap   = mmap_area;
				h->length = nvram_part_size;
				h->offset = offset;

				header = nvram_header(h);

				if (header->magic == NVRAM_MAGIC &&
				    (rdonly || header->len < h->length - h->offset)) {
					_nvram_rehash(h);
					free(mtd);
					return h;
				}
				else
				{
					munmap(h->mmap, h->length);
					free(h);
				}
			}
		}
	}

	free(mtd);
	close(fd);
	return NULL;
}

/* Close NVRAM and free memory. */
int nvram_close(nvram_handle_t *h)
{
	_nvram_free(h);
	munmap(h->mmap, h->length);
	close(h->fd);
	free(h);

	return 0;
}

/* Determine NVRAM device node. */
char * nvram_find_mtd(void)
{
	FILE *fp;
	int i, part_size;
	char dev[PATH_MAX];
	char *path = NULL;
	struct stat s;

	if ((fp = fopen("/proc/mtd", "r")))
	{
		while( fgets(dev, sizeof(dev), fp) )
		{
			if( strstr(dev, "nvram") && sscanf(dev, "mtd%d: %08x", &i, &part_size) )
			{
				nvram_part_size = part_size;

				sprintf(dev, "/dev/mtdblock%d", i);
				if( stat(dev, &s) > -1 && (s.st_mode & S_IFBLK) )
				{
					if( (path = (char *) malloc(strlen(dev)+1)) != NULL )
					{
						strncpy(path, dev, strlen(dev)+1);
						break;
					}
				}
			}
		}
		fclose(fp);
	}

	return path;
}

/* Check NVRAM staging file. */
char * nvram_find_staging(void)
{
	struct stat s;

	if( (stat(NVRAM_STAGING, &s) > -1) && (s.st_mode & S_IFREG) )
	{
		return NVRAM_STAGING;
	}

	return NULL;
}

/* Copy NVRAM contents to staging file. */
int nvram_to_staging(void)
{
	int fdmtd, fdstg, stat;
	char *mtd = nvram_find_mtd();
	char buf[nvram_part_size];

	stat = -1;

	if( (mtd != NULL) && (nvram_part_size > 0) )
	{
		if( (fdmtd = open(mtd, O_RDONLY)) > -1 )
		{
			if( read(fdmtd, buf, sizeof(buf)) == sizeof(buf) )
			{
				if((fdstg = open(NVRAM_STAGING, O_WRONLY | O_CREAT, 0600)) > -1)
				{
					write(fdstg, buf, sizeof(buf));
					fsync(fdstg);
					close(fdstg);

					stat = 0;
				}
			}

			close(fdmtd);
		}
	}

	free(mtd);
	return stat;
}

/* Copy staging file to NVRAM device. */
int staging_to_nvram(void)
{
	int fdmtd, fdstg, stat;
	char *mtd = nvram_find_mtd();
	char buf[nvram_part_size];

	stat = -1;

	if( (mtd != NULL) && (nvram_part_size > 0) )
	{
		if( (fdstg = open(NVRAM_STAGING, O_RDONLY)) > -1 )
		{
			if( read(fdstg, buf, sizeof(buf)) == sizeof(buf) )
			{
				if( (fdmtd = open(mtd, O_WRONLY | O_SYNC)) > -1 )
				{
					write(fdmtd, buf, sizeof(buf));
					fsync(fdmtd);
					close(fdmtd);
					stat = 0;
				}
			}

			close(fdstg);

			if( !stat )
				stat = unlink(NVRAM_STAGING) ? 1 : 0;
		}
	}

	free(mtd);
	return stat;
}
