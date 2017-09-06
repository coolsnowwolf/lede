/*
 *  makeamitbin - create firmware binaries for MGB100
 *
 *  Copyright (C) 2007 Volker Weiss     <dev@tintuc.de>
 *                     Christian Welzel <dev@welzel-online.ch>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* defaults: Level One WAP-0007 */
static char *ascii1 = "DDC_RUS001";
static char *ascii2 = "Queen";

static struct hdrinfo {
  char *name;
	unsigned long unknown; /* can probably be any number, maybe version number */
	int topalign;
	unsigned int addr;
	unsigned int size;
} hdrinfo[] = {
	{ "bios", 0xc76be111, 1, 0x3fa000, 0x006000 },        /* BIOS */
	{ "recovery", 0xc76be222, 0, 0x3f0000, 0x004000 },    /* Recovery Loader */
	{ "linux", 0xc76bee9d, 0, 0x000000, 0x100000 },       /* Linux */
	{ "ramdisk", 0xc76bee9d, 0, 0x100000, 0x280000 },     /* ramdisk */
	{ "amitconfig", 0xc76bee8b, 0, 0x380000, 0x060000 },  /* AMIT config */
	{ "redboot", 0x00000000, 1, 0x3d0000, 0x030000 },     /* Redboot 128kB image */
	{ "redbootlow", 0, 0, 0x3e0000, 0x18000 },            /* Redboot 1. part */
	{ "redboothigh", 0, 0, 0x3fa000, 0x6000 },            /* Redboot 2. part */
	{ "linux3g", 0xcb5f06b5, 0, 0x000000, 0x100000 },       /* Linux */
	{ "ramdisk3g", 0xcb5f06b5, 0, 0x100000, 0x280000 },     /* ramdisk */
	{ NULL }
};

/*
CHD2WLANU_R400b7

11e1 6bc7
22e2 6bc7
5dc3 47c8
5cc3 47c8
21c3 47c8
*/

/*
20060106_DDC_WAP-0007_R400b4

11e1 6bc7
22e2 6bc7
9dee 6bc7
9dee 6bc7
8bee 6bc7
*/

/*
WMU-6000FS_R400b6

11e1 6bc7
22e2 6bc7
6d2d 0fc8
6c2d 0fc8
542d 0fc8
*/

/*
WAP-0007(R4.00b8)_2006-10-02

9979 5fc8
22e2 6bc7
c46e cec8
c36e cec8
a76e cec8
*/



#define HDRSIZE              80

#define COPY_SHORT(d, o, v)  d[o+0] = (unsigned char)((v) & 0xff); \
                             d[o+1] = (unsigned char)(((v) >> 8) & 0xff)
#define COPY_LONG(d, o, v)   d[o+0] = (unsigned char)((v) & 0xff); \
                             d[o+1] = (unsigned char)(((v) >> 8) & 0xff); \
													   d[o+2] = (unsigned char)(((v) >> 16) & 0xff); \
													   d[o+3] = (unsigned char)(((v) >> 24) & 0xff)
#define READ_SHORT(d, o)     ((unsigned short)(d[o+0]) + \
                             (((unsigned short)(d[o+1])) << 8))

/*
00..0d ASCII product ID
0e..0f checksum of payload
10..1b ASCII Queen
1c..1f AMIT BIOS: 11e1 6bc7, Recovery Tool: 22e2 6bc7
       Linux: 5dc3 47c8, ramdisk: 5cc3 47c8
			 AMIT FS: 21c3 47c8    VERSION NUMBER??????
20..23 offset in flash aligned to segment boundary
24..27 length in flash aligned to segment boundary
28..2b offset in flash (payload)
2c..2f length (payload)
30..3f always 0
40..47 always 4248 0101 5000 0001 (last maybe .....0501)
48..4b same as 20..23
4c..4d always 0b00
4e..4f inverted checksum of header
*/

unsigned short checksum(unsigned char *data, long size)
{
	long n;
	unsigned short d, cs = 0;
	for (n = 0; n < size; n += 2)
	{
		d = READ_SHORT(data, n);
		cs += d;
		if (cs < d)
			cs++;
	}
	if (size & 1)
	{
		d = data[n];
		cs += d;
		if (cs < d)
			cs++;
	}
	return cs;
}

void showhdr(unsigned char *hdr)
{
	int i, j;
	for (j = 0; j < 5; j++)
	{
		for (i = 0; i < 16; i++)
		{
			printf("%02x ", (unsigned int)(hdr[j * 16 + i]));
		}
		printf("   ");
		for (i = 0; i < 16; i++)
		{
			unsigned char d = hdr[j * 16 + i];
			printf("%c", (d >= ' ' && d < 127) ? d : '.');
		}
		printf("\n");
	}
}

void makehdr(unsigned char *hdr, struct hdrinfo *info,
             unsigned char *data, long size, int last)
{
	unsigned int offset = info->addr + 0x10;
	memset(hdr, 0, HDRSIZE);
	if (info->topalign)
		offset = info->addr + info->size - size;	/* top align */
	strncpy((char *)hdr + 0x00, ascii1, 14);
	strncpy((char *)hdr + 0x10, ascii2, 12);
	COPY_LONG(hdr, 0x1c, info->unknown);
	COPY_LONG(hdr, 0x20, info->addr);
	COPY_LONG(hdr, 0x24, info->size);
	COPY_LONG(hdr, 0x28, offset);
	COPY_LONG(hdr, 0x2c, size);
	COPY_LONG(hdr, 0x40, 0x01014842);
	COPY_LONG(hdr, 0x44, last ? 0x01050050 : 0x01000050);
	COPY_LONG(hdr, 0x48, info->addr);
	COPY_SHORT(hdr, 0x4c, info->unknown == 0xcb5f06b5 ? 0x0016 : 0x000b);
	COPY_SHORT(hdr, 0x0e, checksum(data, size));
	COPY_SHORT(hdr, 0x4e, ~checksum(hdr, HDRSIZE));
}

unsigned char *read_file(const char *name, long *size)
{
	FILE *f;
	unsigned char *data = NULL;
	*size = 0;
	f = fopen(name, "r");
	if (f != NULL)
	{
		if (fseek(f, 0, SEEK_END) == 0)
		{
	    *size = ftell(f);
			if (*size != -1)
			{
				if (fseek(f, 0, SEEK_SET) == 0)
				{
					data = (unsigned char *)malloc(*size);
					if (data != NULL)
					{
						if (fread(data, sizeof(char), *size, f) != *size)
						{
							free(data);
							data = NULL;
						}
					}
				}
			}
		}
		fclose(f);
	}
	return data;
}

struct hdrinfo *find_hdrinfo(const char *name)
{
	int n;
	for (n = 0; hdrinfo[n].name != NULL; n++)
	{
		if (strcmp(name, hdrinfo[n].name) == 0)
			return &hdrinfo[n];
	}
	return NULL;
}

void oferror(FILE *f)
{
	printf("file error\n");
	exit(2);
}

void showhelp(void)
{
	printf("Syntax: makeamitbin [options]\n");
	printf("Options:\n");
	printf("  -1 ID1\tFirmware identifier 1, e.g. 'DDC_RUS001' for manufacturer LevelOne\n");
	printf("  -2 ID2\tFirmware identifier 2, 'Queen' in all known cases\n");
	printf("  -o FILE\tOutput file\n");
	printf("  -ids\t\tShow a list of known firmware identifiers.\n");
	exit(1);
}

void show_fwids(void)
{
	printf("List of known firmware identifiers:\n");
	printf("Manufacturer\t\tProduct\t\tIdentifier\n");
	printf("=====================================================\n");
	printf("Conceptronic\t\tCHD2WLANU\tLLM_RUS001\n");
	printf("Pearl\t\t\tPE6643\t\tQueen\n");
	printf("Micronica\t\tMGB100\t\tQueen\n");
	printf("LevelOne\t\tWAP-0007\tDDC_RUS001\n");
	printf("SMC\t\t\tWAPS-G\t\tSMC_RUS001\n");
	printf("OvisLink (AirLive)\tWMU-6\t\tOVS_RUS001\n");
	printf("SafeCom SWSAPUR-5\tFMW\t\tSafeco_RPS001\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	unsigned char hdr[HDRSIZE];
	unsigned char *data;
	FILE *of;
	char *outfile = NULL;
	char *type;
	struct hdrinfo *info;
	long size;
	int last = 0;
	int n;
	for (n = 1; n < argc; n++)
	{
		if (strcmp(argv[n], "-1") == 0)
			ascii1 = argv[n+1];
		if (strcmp(argv[n], "-2") == 0)
			ascii2 = argv[n+1];
		if (strcmp(argv[n], "-o") == 0)
			outfile = argv[n+1];
		if (strcmp(argv[n], "-ids") == 0)
			show_fwids();
	}
	if (ascii1 == NULL || ascii2 == NULL || outfile == NULL)
		showhelp();
	of = fopen(outfile, "w");
	if (of == NULL)
		oferror(of);
	for (n = 1; n < argc; n++)
	{
		if (strncmp(argv[n], "-", 1) != 0)
		{
			type = argv[n++];
			if (n >= argc)
				showhelp();
			last = ((n + 1) >= argc);		/* dirty, options first! */
			info = find_hdrinfo(type);
			if (info == NULL)
				showhelp();
			data = read_file(argv[n], &size);
			if (data == NULL)
				showhelp();
			makehdr(hdr, info, data, size, last);
			/* showhdr(hdr); */
			if (fwrite(hdr, HDRSIZE, 1, of) != 1)
				oferror(of);
			if (fwrite(data, size, 1, of) != 1)
				oferror(of);
			free(data);
		}
		else
			n++;
	}
	if (fclose(of) != 0)
		oferror(NULL);
	return 0;
}
