/*
 * Command line interface for libnvram
 *
 * Copyright 2009, Jo-Philipp Wich <xm@subsignal.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * The libnvram code is based on Broadcom code for Linux 2.4.x .
 *
 */

#include "nvram.h"


static nvram_handle_t * nvram_open_rdonly(void)
{
	char *file = nvram_find_staging();

	if( file == NULL )
		file = nvram_find_mtd();

	if( file != NULL ) {
		nvram_handle_t *h = nvram_open(file, NVRAM_RO);
		if( strcmp(file, NVRAM_STAGING) )
			free(file);
		return h;
	}

	return NULL;
}

static nvram_handle_t * nvram_open_staging(void)
{
	if( nvram_find_staging() != NULL || nvram_to_staging() == 0 )
		return nvram_open(NVRAM_STAGING, NVRAM_RW);

	return NULL;
}

static int do_show(nvram_handle_t *nvram)
{
	nvram_tuple_t *t;
	int stat = 1;

	if( (t = nvram_getall(nvram)) != NULL )
	{
		while( t )
		{
			printf("%s=%s\n", t->name, t->value);
			t = t->next;
		}

		stat = 0;
	}

	return stat;
}

static int do_get(nvram_handle_t *nvram, const char *var)
{
	const char *val;
	int stat = 1;

	if( (val = nvram_get(nvram, var)) != NULL )
	{
		printf("%s\n", val);
		stat = 0;
	}

	return stat;
}

static int do_unset(nvram_handle_t *nvram, const char *var)
{
	return nvram_unset(nvram, var);
}

static int do_set(nvram_handle_t *nvram, const char *pair)
{
	char *val = strstr(pair, "=");
	char var[strlen(pair)];
	int stat = 1;

	if( val != NULL )
	{
		memset(var, 0, sizeof(var));
		strncpy(var, pair, (int)(val-pair));
		stat = nvram_set(nvram, var, (char *)(val + 1));
	}

	return stat;
}

static int do_info(nvram_handle_t *nvram)
{
	nvram_header_t *hdr = nvram_header(nvram);

	/* CRC8 over the last 11 bytes of the header and data bytes */
	uint8_t crc = hndcrc8((unsigned char *) &hdr[0] + NVRAM_CRC_START_POSITION,
		hdr->len - NVRAM_CRC_START_POSITION, 0xff);

	/* Show info */
	printf("Magic:         0x%08X\n",   hdr->magic);
	printf("Length:        0x%08X\n",   hdr->len);
	printf("Offset:        0x%08X\n",   nvram->offset);

	printf("CRC8:          0x%02X (calculated: 0x%02X)\n",
		hdr->crc_ver_init & 0xFF, crc);

	printf("Version:       0x%02X\n",   (hdr->crc_ver_init >> 8) & 0xFF);
	printf("SDRAM init:    0x%04X\n",   (hdr->crc_ver_init >> 16) & 0xFFFF);
	printf("SDRAM config:  0x%04X\n",   hdr->config_refresh & 0xFFFF);
	printf("SDRAM refresh: 0x%04X\n",   (hdr->config_refresh >> 16) & 0xFFFF);
	printf("NCDL values:   0x%08X\n\n", hdr->config_ncdl);

	printf("%i bytes used / %i bytes available (%.2f%%)\n",
		hdr->len, nvram->length - nvram->offset - hdr->len,
		(100.00 / (double)(nvram->length - nvram->offset)) * (double)hdr->len);

	return 0;
}

static void usage(void)
{
	fprintf(stderr,
		"Usage:\n"
		"	nvram show\n"
		"	nvram info\n"
		"	nvram get variable\n"
		"	nvram set variable=value [set ...]\n"
		"	nvram unset variable [unset ...]\n"
		"	nvram commit\n"
	);
}

int main( int argc, const char *argv[] )
{
	nvram_handle_t *nvram;
	int commit = 0;
	int write = 0;
	int stat = 1;
	int done = 0;
	int i;

	if( argc < 2 ) {
		usage();
		return 1;
	}

	/* Ugly... iterate over arguments to see whether we can expect a write */
	if( ( !strcmp(argv[1], "set")  && 2 < argc ) ||
		( !strcmp(argv[1], "unset") && 2 < argc ) ||
		!strcmp(argv[1], "commit") )
		write = 1;


	nvram = write ? nvram_open_staging() : nvram_open_rdonly();

	if( nvram != NULL && argc > 1 )
	{
		for( i = 1; i < argc; i++ )
		{
			if( !strcmp(argv[i], "show") )
			{
				stat = do_show(nvram);
				done++;
			}
			else if( !strcmp(argv[i], "info") )
			{
				stat = do_info(nvram);
				done++;
			}
			else if( !strcmp(argv[i], "get") || !strcmp(argv[i], "unset") || !strcmp(argv[i], "set") )
			{
				if( (i+1) < argc )
				{
					switch(argv[i++][0])
					{
						case 'g':
							stat = do_get(nvram, argv[i]);
							break;

						case 'u':
							stat = do_unset(nvram, argv[i]);
							break;

						case 's':
							stat = do_set(nvram, argv[i]);
							break;
					}
					done++;
				}
				else
				{
					fprintf(stderr, "Command '%s' requires an argument!\n", argv[i]);
					done = 0;
					break;
				}
			}
			else if( !strcmp(argv[i], "commit") )
			{
				commit = 1;
				done++;
			}
			else
			{
				fprintf(stderr, "Unknown option '%s' !\n", argv[i]);
				done = 0;
				break;
			}
		}

		if( write )
			stat = nvram_commit(nvram);

		nvram_close(nvram);

		if( commit )
			stat = staging_to_nvram();
	}

	if( !nvram )
	{
		fprintf(stderr,
			"Could not open nvram! Possible reasons are:\n"
			"	- No device found (/proc not mounted or no nvram present)\n"
			"	- Insufficient permissions to open mtd device\n"
			"	- Insufficient memory to complete operation\n"
			"	- Memory mapping failed or not supported\n"
			"	- Nvram magic not found in specific nvram partition\n"
		);

		stat = 1;
	}
	else if( !done )
	{
		usage();
		stat = 1;
	}

	return stat;
}
