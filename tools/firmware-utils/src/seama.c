/* vi: set sw=4 ts=4: */
/*
 *	Copyright (C) 2008, Alpha Networks, Inc.
 *	Created by David Hsieh <david_hsieh@alphanetworks.com>
 *	All right reserved.
 *
 *	(SEA)ttle i(MA)ge is the image which used in project seattle.
 *
 *	Redistribution and use in source and binary forms, with or
 *	without modification, are permitted provided that the following
 *	conditions are met:
 *
 *	1.	Redistributions of source code must retain the above
 *		copyright notice, this list of conditions and the following
 *		disclaimer.
 *
 *	2.	Redistributions in binary form must reproduce the above
 *		copyright notice, this list of conditions and the following
 *		disclaimer in the documentation and/or other materials
 *		provided with the distribution.
 *
 *	3.	The name of the author may not be used to endorse or promote
 *		products derived from this software without specific prior
 *		written permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *	PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *	BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *	IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *	THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>

#include "md5.h"
#include "seama.h"

#define PROGNAME			"seama"
#define VERSION				"0.20"
#define MAX_SEAMA_META_SIZE	1024
#define MAX_META			128
#define MAX_IMAGE			128

extern int optind;
extern char * optarg;

static int		o_verbose = 0;		/* verbose mode. */
static char *	o_dump = NULL;		/* Seama file to dump. */
static char *	o_seal = NULL;		/* Seal the input images when file name exist. */
static char *	o_extract = NULL;	/* Extract the seama file. */
static char *	o_images[MAX_IMAGE];/* The image files to pack or seal */
static int		o_isize = 0;		/* number of images */
static char *	o_meta[MAX_META];	/* meta data array */
static int		o_msize = 0;		/* size of meta array */

static void verbose(const char * format, ...)
{
	va_list marker;
	if (o_verbose)
	{
		va_start(marker, format);
		vfprintf(stdout, format, marker);
		va_end(marker);
	}
}

static void cleanup_exit(int exit_code)
{
	verbose("%s: exit with code %d\n", PROGNAME, exit_code);
	exit(exit_code);
}

static void show_usage(int exit_code)
{
	printf(	PROGNAME " version " VERSION "\n"
			"usage: " PROGNAME " [OPTIONS]\n"
			"  -h                 show this help message.\n"
			"  -v                 verbose mode.\n"
			"  -m {META data}     META data.\n"
			"  -d {file}          dump the info of the seama file.\n"
			"  -i {input file}    image file name.\n"
			"  -s {file}          Seal the images to the seama file.\n"
			"  -x {seama file}    Extract the seama file.\n"
			"\n"
			"  SEAMA can pack the input file (with -i) into a seama file.\n"
			"  ex: seama -i target.file\n"
			"  SEAMA can also seal multiple seama files into a single seama file.\n"
			"  ex: seama -s final.file -i taget1.seama -i target2.seama\n"
			"  To extract the raw image from SEAMA, you need to specify the meta.\n"
			"  The first image match the specified meta will be extract to\n"
			"  the output file which was specified with '-x'.\n"
			"  ex: seama -x output -i seama.image -m file=sealpac\n"
			);
	cleanup_exit(exit_code);
}

static int parse_args(int argc, char * argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "hvd:s:i:m:x:")) > 0)
	{
		switch (opt)
		{
		default:	show_usage(-1); break;
		case 'h':	show_usage(0); break;
		case 'v':	o_verbose++; break;
		case 'd':	o_dump = optarg; break;
		case 's':	o_seal = optarg; break;
		case 'x':	o_extract = optarg; break;
		case 'i':
			if (o_isize < MAX_IMAGE) o_images[o_isize++] = optarg;
			else printf("Exceed the maximum acceptable image files.!\n");
			break;
		case 'm':
			if (o_msize < MAX_META) o_meta[o_msize++] = optarg;
			else printf("Exceed the maximum acceptable META data.!\n");
			break;
		}
	}
	return 0;
}

/*******************************************************************/

static size_t calculate_digest(FILE * fh, size_t size, uint8_t * digest)
{
	MD5_CTX ctx;
	size_t bytes_left, bytes_read, i;
	uint8_t buf[MAX_SEAMA_META_SIZE];

	bytes_left = size ? size : sizeof(buf);
	bytes_read = 0;

	MD5_Init(&ctx);
	while (!feof(fh) && !ferror(fh) && bytes_left > 0)
	{
		i = bytes_left < sizeof(buf) ? bytes_left : sizeof(buf);
		i = fread(buf, sizeof(char), i, fh);
		if (i > 0)
		{
			MD5_Update(&ctx, buf, i);
			bytes_read += i;
		}
		if (size) bytes_left -= i;
	}
	MD5_Final(digest, &ctx);
	return bytes_read;
}

#define READ_BUFF_SIZE 8*1024
static size_t copy_file(FILE * to, FILE * from)
{
	size_t i, fsize = 0;
	uint8_t buf[READ_BUFF_SIZE];

	while (!feof(from) && !ferror(from))
	{
		i = fread(buf, sizeof(uint8_t), READ_BUFF_SIZE, from);
		if (i > 0)
		{
			fsize += i;
			fwrite(buf, sizeof(uint8_t), i, to);
		}
	}
	return fsize;
}

static int verify_seama(const char * fname, int msg)
{
	FILE * fh = NULL;
	struct stat st;
	seamahdr_t shdr;
	uint8_t checksum[16];
	uint8_t digest[16];
	uint8_t buf[MAX_SEAMA_META_SIZE];
	size_t msize, isize, i;
	int ret = -1;

#define ERRBREAK(fmt, args...) { if (msg) printf(fmt, ##args); break; }

	do
	{
		if (stat(fname, &st) < 0)				ERRBREAK("Unable to get the info of '%s'\n",fname);
		if ((fh = fopen(fname, "r+"))==NULL)	ERRBREAK("Unable to open '%s' for reading!\n",fname);

		/* Dump SEAMA header */
		if (msg) printf("FILE - %s (%d bytes)\n", fname, (int)st.st_size);

		/* SEAMA */
		while (!feof(fh) && !ferror(fh))
		{
			/* read header */
			if (fread(&shdr, sizeof(shdr), 1, fh) != 1) break;

			/* Check the magic number */
			if (shdr.magic != htonl(SEAMA_MAGIC)) ERRBREAK("Invalid SEAMA magic. Probably no more SEAMA!\n");

			/* Get the size */
			isize = ntohl(shdr.size);
			msize = ntohs(shdr.metasize);

			/* The checksum exist only if size is greater than zero. */
			if (isize > 0)
			{
				if (fread(checksum, sizeof(checksum), 1, fh) != 1)
					ERRBREAK("Error reading checksum !\n");
			}

			/* Check the META size. */
			if (msize > sizeof(buf)) ERRBREAK("META data in SEAMA header is too large!\n");

			/* Read META data. */
			if (fread(buf, sizeof(char), msize, fh) != msize)
				ERRBREAK("Unable to read SEAMA META data!\n");

			/* dump header */
			if (msg)
			{
				printf("SEAMA ==========================================\n");
				printf("  magic      : %08x\n", ntohl(shdr.magic));
				printf("  meta size  : %zu bytes\n", msize);
				for (i=0; i<msize; i+=(strlen((const char *)&buf[i])+1))
					printf("  meta data  : %s\n", &buf[i]);
				printf("  image size : %zu bytes\n", isize);
			}

			/* verify checksum */
			if (isize > 0)
			{
				if (msg)
				{
					printf("  checksum   : ");
					for (i=0; i<16; i++) printf("%02X", checksum[i]);
					printf("\n");
				}

				/* Calculate the checksum */
				calculate_digest(fh, isize, digest);
				if (msg)
				{
					printf("  digest     : ");
					for (i=0; i<16; i++) printf("%02X", digest[i]);
					printf("\n");
				}

				if (memcmp(checksum, digest, 16)!=0) ERRBREAK("!!ERROR!! checksum error !!\n");
				ret = 0;
			}
		}
		if (msg) printf("================================================\n");
	} while (0);
	if (fh) fclose(fh);
	return ret;
}

static size_t write_seama_header(FILE * fh, char * meta[], size_t msize, size_t size)
{
	seamahdr_t shdr;
	size_t i;
	uint16_t metasize = 0;

	/* Calculate the META size */
	for (i=0; i<msize; i++) metasize += (strlen(meta[i]) + 1);
	//+++ let meta data end on 4 alignment by siyou. 2010/3/1 03:58pm
	metasize = ((metasize+3)/4)*4;
	verbose("SEAMA META : %d bytes\n", metasize);

	/* Fill up the header, all the data endian should be network byte order. */
	shdr.magic		= htonl(SEAMA_MAGIC);
	shdr.reserved	= 0;
	shdr.metasize	= htons(metasize);
	shdr.size		= htonl(size);

	/* Write the header */
	return fwrite(&shdr, sizeof(seamahdr_t), 1, fh);
}

static size_t write_checksum(FILE * fh, uint8_t * checksum)
{
	return fwrite(checksum, sizeof(uint8_t), 16, fh);
}

static size_t write_meta_data(FILE * fh, char * meta[], size_t size)
{
	size_t i,j;
	size_t ret = 0;

	for (i=0; i<size; i++)
	{
		verbose("SEAMA META data : %s\n", meta[i]);
		j = fwrite(meta[i], sizeof(char), strlen(meta[i])+1, fh);
		if (j != strlen(meta[i])+1) return 0;
		ret += j;
	}
	//+++ let meta data end on 4 alignment by siyou. 2010/3/1 03:58pm
	j = ((ret+3)/4)*4;
	for ( ; ret < j; ret++)
		fwrite("", sizeof(char), 1, fh);

	return ret;
}

/*******************************************************************/

static void dump_seama(const char * fname)
{
	verify_seama(fname, 1);
}

static void seal_files(const char * file)
{
	FILE * fh;
	FILE * ifh;
	size_t i;

	/* Each image should be seama. */
	for (i = 0; i < o_isize; i++)
	{
		if (verify_seama(o_images[i], 0) < 0)
		{
			printf("'%s' is not a seama file !\n",o_images[i]);
			return;
		}
	}

	/* Open file for write */
	fh = fopen(file, "w+");
	if (fh)
	{
		/* Write the header. */
		write_seama_header(fh, o_meta, o_msize, 0);
		write_meta_data(fh, o_meta, o_msize);

		/* Write image files */
		for (i=0; i<o_isize; i++)
		{
			ifh = fopen(o_images[i], "r+");
			if (ifh)
			{
				copy_file(fh, ifh);
				fclose(ifh);
			}
		}

		fclose(fh);
	}
}

static void pack_files(void)
{
	FILE * fh;
	FILE * ifh;
	size_t i, fsize;
	char filename[512];
	uint8_t digest[16];

	for (i=0; i<o_isize; i++)
	{
		/* Open the input file. */
		ifh = fopen(o_images[i], "r+");
		if (ifh)
		{
			fsize = calculate_digest(ifh, 0, digest);
			verbose("file size (%s) : %d\n", o_images[i], fsize);
			rewind(ifh);

			/* Open the output file. */
			sprintf(filename, "%s.seama", o_images[i]);
			fh = fopen(filename, "w+");
			if (fh)
			{
				write_seama_header(fh, o_meta, o_msize, fsize);
				write_checksum(fh, digest);
				write_meta_data(fh, o_meta, o_msize);
				copy_file(fh, ifh);
				fclose(fh);
			}
			fclose(ifh);
		}
		else
		{
			printf("Unable to open image file '%s'\n",o_images[i]);
		}
	}
}

/**************************************************************************/

static int match_meta(const char * meta, size_t size)
{
	size_t i, j;
	int match;

	for (i = 0; i < o_msize; i++)
	{
		for (match = 0, j = 0; j < size; j += (strlen(&meta[j])+1))
			if (strcmp(&meta[j], o_meta[i])==0) { match++; break; }
		if (!match) return 0;
	}
	return 1;
}


static void extract_file(const char * output)
{
	FILE * ifh = NULL;
	FILE * ofh = NULL;
	size_t msize, isize, i, m;
	seamahdr_t shdr;
	uint8_t buf[MAX_SEAMA_META_SIZE];
	int done = 0;

	/* We need meta for searching the target image. */
	if (o_msize == 0)
	{
		printf("SEAMA: need meta for searching image.\n");
		return;
	}

	/* Walk through each input file */
	for (i = 0; i < o_isize; i++)
	{
		/* verify the input file */
		if (verify_seama(o_images[i], 0) < 0)
		{
			printf("SEAMA: '%s' is not a seama file !\n", o_images[i]);
			continue;
		}
		/* open the input file */
		ifh  = fopen(o_images[i], "r");
		if (!ifh) continue;
		/* read file */
		while (!feof(ifh) && !ferror(ifh))
		{
			/* read header */
			fread(&shdr, sizeof(shdr), 1, ifh);
			if (shdr.magic != htonl(SEAMA_MAGIC)) break;
			/* Get the size */
			isize = ntohl(shdr.size);
			msize = ntohs(shdr.metasize);
			if (isize == 0)
			{
				while (msize > 0)
				{
					m = fread(buf, sizeof(char), (msize < MAX_SEAMA_META_SIZE) ? msize : MAX_SEAMA_META_SIZE, ifh);
					if (m <= 0) break;
					msize -= m;
				}
				continue;
			}
			/* read checksum */
			fread(buf, sizeof(char), 16, ifh);
			if (msize > 0)
			{
				/* read META */
				fread(buf, sizeof(char), msize, ifh);
				if (match_meta((const char *)buf, msize))
				{
					printf("SEAMA: found image @ '%s', image size: %zu\n", o_images[i], isize);
					/* open output file */
					ofh = fopen(output, "w");
					if (!ofh) printf("SEAMA: unable to open '%s' for writting.\n",output);
					else
					{
						while (isize > 0)
						{
							m = fread(buf, sizeof(char), (isize < MAX_SEAMA_META_SIZE) ? isize : MAX_SEAMA_META_SIZE, ifh);
							if (m <= 0) break;
							fwrite(buf, sizeof(char), m, ofh);
							isize -= m;
						}
						fclose(ofh);
					}
					done++;
					break;
				}
			}
			while (isize > 0)
			{
				m = fread(buf, sizeof(char), (isize < MAX_SEAMA_META_SIZE) ? isize : MAX_SEAMA_META_SIZE, ifh);
				if (m <= 0) break;
				isize -= m;
			}
		}
		/* close the file. */
		fclose(ifh);
		if (done) break;
	}
	return;
}

/*******************************************************************/
#ifdef RGBIN_BOX
int seama_main(int argc, char * argv[], char * env[])
#else
int main(int argc, char * argv[], char * env[])
#endif
{
	verbose("SEAMA version " VERSION "\n");

	/* parse the arguments */
	if (parse_args(argc, argv) < 0) show_usage(9);

	/* Do the works */
	if		(o_dump)	dump_seama(o_dump);
	else if (o_seal)	seal_files(o_seal);
	else if	(o_extract)	extract_file(o_extract);
	else				pack_files();

	cleanup_exit(0);
	return 0;
}
