/*
 * Copyright (C) 2004  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* July 29, 2004
 *
 * This is a hacked replacement for the 'trx' utility used to create
 * wrt54g .trx firmware files.  It isn't pretty, but it does the job
 * for me.
 *
 * As an extension, you can specify a larger maximum length for the
 * .trx file using '-m'.  It will be rounded up to be a multiple of 4K.
 * NOTE: This space will be malloc()'d.
 *
 * August 16, 2004
 *
 * Sigh... Make it endian-neutral.
 *
 * TODO: Support '-b' option to specify offsets for each file.
 *
 * February 19, 2005 - mbm
 *
 * Add -a (align offset) and -b (absolute offset)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <machine/endian.h>
#include <machine/byte_order.h>
#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define bswap_16(x) NXSwapShort(x)
#define bswap_32(x) NXSwapInt(x)
#define bswap_64(x) NXSwapLongLong(x)
#else
#include <endian.h>
#include <byteswap.h>
#endif
#include "trx.h"
#include <sys/stat.h>
#ifndef TRENDCHIP
#define __BYTE_ORDER __BIG_ENDIAN
#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif
#else
#ifdef TCSUPPORT_LITTLE_ENDIAN
#define STORE32_LE(X)   (X)
#else
#define STORE32_LE(X)		bswap_32(X)
#endif
#endif

uint32_t crc32buf(char *buf, size_t len);

void get_config(char *key, char *name, int name_size,char *file_path);
/**********************************************************************/
/* from trxhdr.h */

#if defined(TCSUPPORT_BB_256KB)
#define TCBOOT_SIZE     	(0x40000) //256K
#define TCBOOT_CRC32_LOC	(TCBOOT_SIZE - 512 - 4)
#elif defined(BOOTROM_LARGE_SIZE)
#define TCBOOT_SIZE     0x20000 //128K
#else
#define TCBOOT_SIZE     0x10000 //64K
#endif
#define TRX_VERSION	    1
//#define TRX_MAX_LEN	0x5A0000
#define TRX_MAX_LEN     0x8000000 //128M

#define TRX_NO_HEADER	1		/* Do not write TRX header */

#ifdef TRENDCHIP
#define NEED_PADDING 0
#define NO_PADDING 1
#define NO_ALIGN 1
#define ALIGN_4B 4
#define ALIGN_64K 0x1000
unsigned long int padding = NO_PADDING;
#endif

/**********************************************************************/

void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
#if 0
	fprintf(stderr, "Usage: trx [-o outfile] [-m maxlen] [-a align] [-b offset] [-f file] [-f file [-f file]]\n");
#else
	//fprintf(stderr, "Usage: trx [-p] [-k kernellen] [-o outfile] [-m maxlen] [-a align] [-b offset] [-f file] [-f file [-f file]]\n");
//	fprintf(stderr, "Usage: trx [-p] [-o outfile] [-m maxlen] [-a align] [-b offset] [-c config_file] [-f file] [-f file [-f file]] \n");
	//fprintf(stderr, "Usage: trx [-p] [-k kernel_len] [-r rootfs_len] [-o outfile] [-m maxlen] [-a align] [-b offset] [-c config_file] [-f file] [-f file [-f file]] [-t boot_file]\n");
	fprintf(stderr, "Usage: trx [-p] [-k kernel_len] [-r rootfs_len] [-o outfile] [-m maxlen] [-a align] [-b offset] [-u start_addr] [-c config_file] [-f file] [-f file [-f file]] [-t boot_file] [-e bootext_input_file bootext_output_file]\n");
#endif
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	FILE *out = stdout;
	FILE *in;
	char *ofn = NULL;
	char *buf;
	char *e;
	char config_file[128];
	int c, i;
	size_t n;
	uint32_t cur_len;
	uint32_t round;
    uint32_t tcboot_crc32;
	unsigned long cal_len = TCBOOT_SIZE;
	unsigned long maxlen = TRX_MAX_LEN;
	struct trx_header *p;
	struct multi_bl2_h bl2_h;
	struct stat stbuf;
	int input_size;
	int is_pad;
	int index = 0;
#ifdef TCSUPPORT_BL2_OPTIMIZATION
	FILE *in2;
	FILE *in3;
	FILE *flash_table;	
	struct f_header_t f;
	int input_size2;
	int output_size;
	int flash_table_size;
	unsigned int version = 0;
#endif

    uint32_t allinone_crc32;
	long count = 0;
	unsigned char * crc_ptr = NULL;

	fprintf(stderr, "mjn3's trx replacement - v0.81.1\n");

	if (!(buf = malloc(maxlen))) {
		fprintf(stderr, "malloc failed\n");
		return EXIT_FAILURE;
	}

	p = (struct trx_header *) buf;
#ifndef TRENDCHIP
	p->magic = STORE32_LE(TRX_MAGIC);
#endif
	cur_len = sizeof(struct trx_header);
	//p->flag_version = STORE32_LE((TRX_VERSION << 16));

	in = NULL;
	i = 0;

	while ((c = getopt(argc, argv, "-:k:r:s:o:m:a:b:u:c:d:f:p:t:gh:x:z:y:i")) != -1) {

		switch (c) {
			/*pork 20090313 added*/
#ifdef TRENDCHIP
			//case 'k':
			//	p->kernel_len = STORE32_LE(atoi(optarg));
			//	break;
			case 'y':
			/* uses for ARM multi-boot generation*/
				if (!(in = fopen(argv[2], "rb"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}

				if (!(out = fopen(argv[3], "wb"))) {
						fprintf(stderr, "can not open file for writing\n");
					usage();
				}			

			if((in == NULL) ||  (out == NULL))
			{
				if(in != NULL)
					fclose(in);
				if(out != NULL)
					fclose(out);
				return 0;
			}

			/* get input file size */
			fseek(in, 0L, SEEK_END);
			input_size = ftell(in);
			/* back to beginning of file */
			fseek(in, 0L, SEEK_SET);
			fprintf(stderr, "input_size is %d\n",input_size);

			bl2_h.magic = 0xAA640001;
			bl2_h.version = 0x1;
			fread(buf ,1 ,input_size+1-32 , in);
			
			tcboot_crc32 = crc32buf((char *)buf, input_size-32);
			tcboot_crc32 = STORE32_LE(tcboot_crc32);
			bl2_h.bl2_crc = tcboot_crc32;
			fprintf(stderr, "crc length = %x crc = %x\n",input_size-32, bl2_h.bl2_crc);

			memset(bl2_h.reserved,0,12);
			
			for (i = 0; i < 8; i++)
			{
				fseek(in, 0L, SEEK_SET);
				memset(buf + (0x20000 * i), 0, 0x800);
				memcpy(buf + 0x800 + (0x20000 * i), &bl2_h, sizeof (struct multi_bl2_h));

				tcboot_crc32 = crc32buf((char *)buf + 0x800 + (0x20000 * i), sizeof (struct multi_bl2_h));
        	    tcboot_crc32 = STORE32_LE(tcboot_crc32);
		        *(unsigned int*)(buf+sizeof (struct multi_bl2_h) + 0x800 + (0x20000 * i)) = tcboot_crc32;
	
				n = fread(buf + sizeof(struct multi_bl2_h)+4 + 0x800 + (0x20000 * i) ,1 ,input_size+1-32 , in);

			}
			fwrite(buf, 0x20000 * 8, 1, out);
			fclose(in);
			fclose(out);
			return 0;
			case 'z':
#ifdef TCSUPPORT_BL2_OPTIMIZATION
				if (!(in = fopen(argv[2], "rb"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}
				if (!(in2 = fopen(argv[3], "rb"))) {
						fprintf(stderr, "can not open file for writing\n");
					usage();
				}
				if (!(in3 = fopen(argv[4], "rb"))) {
						fprintf(stderr, "can not open file for writing\n");
					usage();
				}
				if (!(flash_table = fopen(argv[5], "rb"))) {
						fprintf(stderr, "can not open file for writing\n");
					usage();
				}

				if (!(out = fopen(argv[6], "wb"))) {
						fprintf(stderr, "can not open file for writing\n");
					usage();
				}
				if (argc > 7)
				{
					version = atoi(argv[7]);
					fprintf(stderr,"version = %d\n",version);
				}

				if((in == NULL) || (in2== NULL) || (in3 == NULL) || (flash_table == NULL) || (out == NULL))
				{
					if(in != NULL)
						fclose(in);
					if(in2 != NULL)
						fclose(in2);
					if(in3 != NULL)
						fclose(in3);
					if(flash_table != NULL)
						fclose(flash_table);
					if(out != NULL)
						fclose(out);
				}

				memset(&f, 0, sizeof (f));

				/* get input file size */
				fseek(in, 0L, SEEK_END);
				input_size = ftell(in);
				/* back to beginning of file */
				fseek(in, 0L, SEEK_SET);
				fprintf(stderr, "input_size is %d\n",input_size);

			
				/* get input file size */
				fseek(in2, 0L, SEEK_END);
				output_size = ftell(in2);
				/* back to beginning of file */
				fseek(in2, 0L, SEEK_SET);
				fprintf(stderr, "input_size2 is %d\n",output_size);

				/* get input file size */
				fseek(in3, 0L, SEEK_END);
				input_size2 = ftell(in3);
				/* back to beginning of file */
				fseek(in3, 0L, SEEK_SET);
				fprintf(stderr, "input_size3 is %d\n",input_size2);

				/* get input file size */
				fseek(flash_table, 0L, SEEK_END);
				flash_table_size = ftell(flash_table);
				/* back to beginning of file */
				fseek(flash_table, 0L, SEEK_SET);
				fprintf(stderr, "flash_table_size is %d\n", flash_table_size);

				
				memset(buf, 0 , input_size + output_size + input_size2 + flash_table_size);
				
				f.bl22_length = output_size;
				f.bl23_length = input_size2;
				f.flash_table_length = flash_table_size;
				f.lzma_src = 0x1e843c00 + sizeof(struct f_header_t);
				f.lzma_length =output_size;
				f.lzma_des = 0x08004000;
				if (version >0)
				{
					f.fw_ver= version;
				}
				n = fread(buf, 1, input_size+1, in);
			
				if (!feof(in)) {
					fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
					fclose(in);
					fclose(in2);
					fclose(in3);
					fclose(flash_table);
					fclose(out);
					return EXIT_FAILURE;
				}
			
				fclose(in);
				memcpy(buf+input_size , &f, sizeof(struct f_header_t));

				n = fread(buf + input_size + sizeof(struct f_header_t),1, output_size+1, in2);
				if (!feof(in2)) {
					fprintf(stderr, "fread failure or file \"%s\" too large2\n",optarg);
					fclose(in2);
					fclose(in3);
					fclose(flash_table);
					fclose(out);
					return EXIT_FAILURE;
				}
				fclose(in2);		
				n = fread(buf + input_size + sizeof(struct f_header_t) + output_size, 1 , input_size2+1, in3);
				if (!feof(in3)) {
					fprintf(stderr, "fread failure or file \"%s\" too large2\n",optarg);
					fclose(in3);
					fclose(flash_table);
					fclose(out);
					return EXIT_FAILURE;
				}
				fclose(in3);
				
				n = fread(buf + input_size + sizeof(struct f_header_t) + output_size + input_size2, 1 , flash_table_size+1, flash_table);
				if (!feof(flash_table)) {
					fprintf(stderr, "fread failure or file \"%s\" too large3\n",optarg);
					fclose(flash_table);
					fclose(out);					
					return EXIT_FAILURE;
				}
				fclose(flash_table);	

				fwrite(buf, input_size+output_size + sizeof(struct f_header_t)+input_size2 + flash_table_size, 1, out);
				fclose(out);
				return 0;
#else
				return 0;
#endif
			case 'k':
                fprintf(stderr, "kernel size = %d\r\n", atoi(optarg));
                p->kernel_len = STORE32_LE(atoi(optarg));
                break;
            case 'r':
                fprintf(stderr, "rootfs size = %d\r\n", atoi(optarg));
                p->rootfs_len = STORE32_LE(atoi(optarg));
                break;
		case 'd':
               fprintf(stderr, "ctromfile size = %d\r\n", atoi(optarg));
               p->romfile_len = STORE32_LE(atoi(optarg));
                break;
			case 'p':
				padding = NEED_PADDING;
				break;
			/*use config file*/	
			case 'c':
			//	fprintf(stderr,"11 %s %d\n",optarg,sizeof(struct trx_header));
				if(optarg != NULL){
					strncpy(config_file,optarg,sizeof(config_file));	
					config_file[sizeof(config_file)-1] = '\0';
				}
			break;
            /* frankliao 20100510 added over*/
            case 't':
                if (!(in = fopen(argv[2], "rb"))) {
                    fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
                    usage();
                }

				/* get input file size */
				fseek(in, 0L, SEEK_END);
				input_size = ftell(in);
				/* back to beginning of file */
				fseek(in, 0L, SEEK_SET);

				if(input_size < 0 || input_size > TRX_MAX_LEN) {
					fprintf(stderr, "file \"%s\" size:0x%x error.\n", argv[2], input_size);
					free(buf);
					fclose(in);
					fclose(out);
                    return EXIT_FAILURE;
				}
#ifdef TCSUPPORT_BB_256KB
				memset(buf, 0x0, input_size);
				n = fread(buf, 1, input_size, in);
				fclose(in);

				if((argc >= 4) && (strcmp(argv[3], "bootext.ram") == 0)) {
					strncpy(config_file, "bootext.ram", sizeof(config_file));
					tcboot_crc32 = crc32buf((char *)buf, input_size - 4);
	                tcboot_crc32 = STORE32_LE(tcboot_crc32);
	                *(unsigned int*)(buf+input_size - 4) = tcboot_crc32;
				} else if((argc >= 4) && (strcmp(argv[3], "rsa_pub.txt") == 0)) {
					strncpy(config_file, "rsa_pub.txt", sizeof(config_file));
					tcboot_crc32 = crc32buf((char *)buf, input_size);
	                tcboot_crc32 = STORE32_LE(tcboot_crc32);
	                *(unsigned int*)(buf+input_size) = tcboot_crc32;
					input_size += 4;
				} else {
					strncpy(config_file, "tcboot.bin", sizeof(config_file));

					/* CRC32_1 */
					tcboot_crc32 = crc32buf((char *)buf, 0x20000 - 4);
	                tcboot_crc32 = STORE32_LE(tcboot_crc32);
	                *(unsigned int*)(buf + 0x20000 - 4) = tcboot_crc32;
					/* CRC32_2 */
					tcboot_crc32 = crc32buf((char *)buf, TCBOOT_CRC32_LOC);
	                tcboot_crc32 = STORE32_LE(tcboot_crc32);
	                *(unsigned int*)(buf+TCBOOT_CRC32_LOC) = tcboot_crc32;
				}

				if (!(out = fopen(config_file, "wb"))) {
                    fprintf(stderr, "can not open file %s for writing\n", config_file);
                    return EXIT_FAILURE;
                }

				fwrite(buf, input_size, 1, out);
				fclose(out);
#else
				if (!(out = fopen("tcboot.bin", "wb"))) {
                    fprintf(stderr, "can not open file tcboot.bin for writing\n");
                    usage();
                }
				if(argc >= 4)
				{
					cal_len = strtoul(argv[3],NULL,16);
				} 

                n = fread(buf, 1, input_size + 1, in);

				if (!feof(in)) {
                    fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
                    fclose(in);
                    return EXIT_FAILURE;
                }

                fclose(in);
                tcboot_crc32 = crc32buf((char *)buf, cal_len-4);
                tcboot_crc32 = STORE32_LE(tcboot_crc32);

                *(unsigned int*)(buf+cal_len-4) = tcboot_crc32;
                fwrite(buf, input_size, 1, out);
#endif
                return 0;
			case 'x':
				if (!(in = fopen(argv[2], "rb"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}
				
				/* check if the crc is padding to the image*/
				if(argc == 5)
				{
					is_pad = strtoul(argv[4],NULL,16);	
				}


				/* get input file size */
				fseek(in, 0L, SEEK_END);
				input_size = ftell(in);
				
				/* back to beginning of file */
				fseek(in, 0L, SEEK_SET);

				if(input_size < 0 || input_size > TRX_MAX_LEN) {
					fprintf(stderr, "file \"%s\" size:0x%x error.\n", argv[2], input_size);
					free(buf);
					fclose(in);
					fclose(out);
					return EXIT_FAILURE;
				}
				memset(buf, 0x0, input_size+4);
				
				if (!(out = fopen(argv[3], "wb"))) {
					fprintf(stderr, "can not open file tcboot.bin for writing\n");
					usage();
				}

				n = fread(buf, 1, input_size + 1, in);

				if (!feof(in)) {
					fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
					fclose(in);
					return EXIT_FAILURE;
                }

                fclose(in);
				/*cal crc with input_size-4 and pad the crc result to last 4 byte inside the image*/
				if(is_pad == 1)
				{
					input_size = input_size - 4;
				}
				/* cal crc with input_size and pad the crc result to last 4 byte*/
				else
				{
					;//do nothing
				}

                tcboot_crc32 = crc32buf((char *)buf, input_size);
                tcboot_crc32 = STORE32_LE(tcboot_crc32);

                *(unsigned int*)(buf+input_size) = tcboot_crc32;
                fwrite(buf, input_size+4, 1, out);
				return 0;

#endif
			/*pork 20090313 added over*/
			case 'f':
			case 1:
			#ifdef TRENDCHIP
#if 0
				if(strcmp(optarg,"linux.7z") == 0)
				{
					p->magic = STORE32_LE(TRX_MAGIC);
				}
				else if(strcmp(optarg,"rootfs") == 0)
				{
					p->magic = STORE32_LE(TRX_MAGIC1);
				}
#else
				if (optarg == NULL) {
					usage();
				}
				
				if(strstr(optarg,"linux.7z"))
				{
					p->magic = STORE32_LE(TRX_MAGIC);
				}
#endif
				else if(strstr(optarg,"rootfs"))
				{
					p->magic = STORE32_LE(TRX_MAGIC1);
				}
				else if(strstr(optarg,"tclinux")){
					p->magic = STORE32_LE(TRX_MAGIC2);
				}
				else if(strstr(optarg,"romfile.cfg") || strstr(optarg,"romfile_f.cfg") || strstr(optarg,"romfile_epon.cfg") || strstr(optarg,"romfile_")){
					p->magic = STORE32_LE(TRX_MAGIC3);
				}
			#endif
#if 0
				p->offsets[i++] = STORE32_LE(cur_len);
#endif
				if (!(in = fopen(optarg, "r"))) {
					fprintf(stderr, "can not open \"%s\" for reading\n", optarg);
					usage();
				}
				n = fread(buf + cur_len, 1, maxlen - cur_len, in);
				if (!feof(in)) {
					fprintf(stderr, "fread failure or file \"%s\" too large\n",optarg);
					fclose(in);
					return EXIT_FAILURE;
				}
				fclose(in);
				if(padding == NO_PADDING){
					round = NO_ALIGN;
				}
				else{
					round = ALIGN_4B;
				}
				if (n & (round-1)) {
					memset(buf + cur_len + n, 0, round - (n & (round-1)));
					n += round - (n & (round-1));
				}
				cur_len += n;

				break;
			case 'o':
				ofn = optarg;
				if (ofn && !(out = fopen(ofn, "w"))) {
					fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
					usage();
				}

				break;
			case 'm':
				errno = 0;
				if (optarg == NULL) {
					usage();
				}
				maxlen = strtoul(optarg, &e, 0);
				if (errno || (e == optarg) || *e) {
					fprintf(stderr, "illegal numeric string\n");
					usage();
				}
				round = ALIGN_4B;
				if (maxlen & (round-1)) {
					maxlen += (round - (maxlen & (round-1)));
				}
				if (maxlen < round) {
					fprintf(stderr, "maxlen too small (or wrapped)\n");
					usage();
				}
				if (maxlen > TRX_MAX_LEN) {
					fprintf(stderr, "WARNING: maxlen exceeds default maximum!  Beware of overwriting nvram!\n");
				}
				if (!(buf = realloc(buf,maxlen))) {
					fprintf(stderr, "realloc failed");
					return EXIT_FAILURE;
				}
				break;
			case 'a':
				errno = 0;
				if (optarg == NULL) {
					usage();
				}
				n = strtoul(optarg, &e, 0);
				if (errno || (e == optarg) || *e) {
					fprintf(stderr, "illegal numeric string\n");
					usage();
				}
				if (cur_len & (n-1)) {
					if((cur_len & (n-1)) > n) {
						usage();
					}
					n = n - (cur_len & (n-1));
					memset(buf + cur_len, 0, n);
					cur_len += n;
				}
				break;
			case 'b':
				errno = 0;
				if (optarg == NULL) {
					usage();
				}
				n = strtoul(optarg, &e, 0);
				if (errno || (e == optarg) || *e) {
					fprintf(stderr, "illegal numeric string\n");
					usage();
				}
				if (n < cur_len) {
					fprintf(stderr, "WARNING: current length exceeds -b %zu offset\n",n);
				} else {
					memset(buf + cur_len, 0, n - cur_len);
					cur_len = n;
				}
				break;
			case 'u':
				errno = 0;
				if (optarg == NULL) {
					usage();
				}
				n = strtoul(optarg, &e, 0);
				if (errno || (e == optarg) || *e) {
					fprintf(stderr, "illegal numeric string\n");
					usage();
				}
				p->decompAddr= STORE32_LE(n);
		       	break;
           		case 'g':
				if (!(in = fopen("tclinux_allinone", "r+"))) {
					fprintf(stderr, "can not open file tclinux_allinone for writing\n");
					usage();
					return 0;
				}

				fseek(in,0, SEEK_END);
				count = ftell(in);
				fseek(in,0, SEEK_SET);

				if(count < 0) {
					fprintf(stderr, "file tclinux_allinone size:0x%lx error.\n", count);
					free(buf);
					fclose(in);
                    return EXIT_FAILURE;
				}

				cur_len = fread(buf, 1, count, in);
				allinone_crc32 = crc32buf((char *)buf, count);
				allinone_crc32 = STORE32_LE(allinone_crc32);  
				crc_ptr = buf+count;

				*crc_ptr = (allinone_crc32 & 0xff000000)>>24;
				*(crc_ptr+1) = (allinone_crc32 & 0x00ff0000)>>16;
				*(crc_ptr+2) = (allinone_crc32 & 0x0000ff00)>>8;
				*(crc_ptr+3) = (allinone_crc32 & 0x000000ff);  

				fwrite(buf+count, 4, 1, in);					
				fclose(in);
				return 0;		
			case 'h':
				if (!(in = fopen("tclinux_allinone_nand", "r+"))) {
					fprintf(stderr, "can not open file tclinux_allinone_nand for writing\n");
					usage();
					return 0;
				}

				fseek(in,0, SEEK_END);
				count = ftell(in);
				fseek(in,0, SEEK_SET); 

				if(count < 0) {
					fprintf(stderr, "file tclinux_allinone_nand size:0x%lx error.\n", count);
					free(buf);
					fclose(in);
                    return EXIT_FAILURE;
				}

				cur_len = fread(buf, 1, count, in);
				allinone_crc32 = crc32buf((char *)buf, count);
				allinone_crc32 = STORE32_LE(allinone_crc32);  
				crc_ptr = buf+count;

				*crc_ptr = (allinone_crc32 & 0xff000000)>>24;
				*(crc_ptr+1) = (allinone_crc32 & 0x00ff0000)>>16;
				*(crc_ptr+2) = (allinone_crc32 & 0x0000ff00)>>8;
				*(crc_ptr+3) = (allinone_crc32 & 0x000000ff);  
				
				fwrite(buf+count, 4, 1, in);					
				fclose(in);
				return 0;
			case 's':
				p->saflen = STORE32_LE(atoi(optarg));
				p->saflag = STORE32_LE(0x1);
				break;
			default:
				usage();
		}
	}

	if (!in) {
		fprintf(stderr, "we require atleast one filename\n");
		usage();
	}
	if(padding == NO_PADDING){
		round = NO_ALIGN;
	}
	else{
		round = ALIGN_64K;
	}
	n = cur_len & (round-1);
	if (n) {
		memset(buf + cur_len, 0, round - n);
		cur_len += round - n;
	}
	//p->crc32 = crc32buf((char *) &p->flag_version,
	//					cur_len - offsetof(struct trx_header, flag_version));
		
	p->crc32 = crc32buf((char *)(buf+sizeof(struct trx_header)), (cur_len-sizeof(struct trx_header)));
	p->crc32 = STORE32_LE(p->crc32);
	p->header_len = STORE32_LE(sizeof(struct trx_header));
	//load config value from config file
	if(stat(config_file,&stbuf) == 0){
		#if 1
		get_config("Model",p->Model,sizeof(p->Model),config_file);
		fprintf(stderr,"Model %s \n",p->Model);
		#endif
		get_config("version",p->version,sizeof(p->version),config_file);
		fprintf(stderr,"version %s \n",p->version);
		get_config("customerversion",p->customerversion,sizeof(p->customerversion),config_file);
		fprintf(stderr,"customerversion %s \n",p->customerversion);
	}else{
		fprintf(stderr,"no config file\n");	
	}	
	p->len = STORE32_LE(cur_len);
	if (!fwrite(buf, cur_len, 1, out) || fflush(out)) {
		fprintf(stderr, "fwrite failed\n");
		return EXIT_FAILURE;
	}

	free(buf);
	fclose(out);

	return EXIT_SUCCESS;
}

void get_config(char *key, char *name, int name_size, char *file_path){

	FILE *fp;
	char buf[256], buf2[256];
	char *point1;

	sprintf(buf2,"%s=",key);
	fp = fopen(file_path,"r");
	if(fp != NULL){
		while(fgets(buf,sizeof(buf),fp) > 0 ){
			point1=strstr(buf,buf2);
			if(point1 != NULL){
				strncpy(name,(point1+strlen(buf2)),name_size);
				break;
			}else{
				continue;	
			}
		}	
		fclose(fp);
	}else{
		fprintf(stderr,"can not open config file\n");	
	}
}
/**********************************************************************/
/* The following was grabbed and tweaked from the old snippets collection
 * of public domain C code. */

/**********************************************************************\
|* Demonstration program to compute the 32-bit CRC used as the frame  *|
|* check sequence in ADCCP (ANSI X3.66, also known as FIPS PUB 71     *|
|* and FED-STD-1003, the U.S. versions of CCITT's X.25 link-level     *|
|* protocol).  The 32-bit FCS was added via the Federal Register,     *|
|* 1 June 1982, p.23798.  I presume but don't know for certain that   *|
|* this polynomial is or will be included in CCITT V.41, which        *|
|* defines the 16-bit CRC (often called CRC-CCITT) polynomial.  FIPS  *|
|* PUB 78 says that the 32-bit FCS reduces otherwise undetected       *|
|* errors by a factor of 10^-5 over 16-bit FCS.                       *|
\**********************************************************************/

/* Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.*/

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*  1. The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*  2. The CRC accumulation logic is the same for all CRC polynomials, */
/*     be they sixteen or thirty-two bits wide.  You simply choose the */
/*     appropriate table.  Alternatively, because the table can be     */
/*     generated at runtime, you can start by generating the table for */
/*     the polynomial in question and use exactly the same "updcrc",   */
/*     if your application needn't simultaneously handle two CRC       */
/*     polynomials.  (Note, however, that XMODEM is strange.)          */
/*                                                                     */
/*  3. For 16-bit CRCs, the table entries need be only 16 bits wide;   */
/*     of course, 32-bit entries work OK if the high 16 bits are zero. */
/*                                                                     */
/*  4. The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

static const uint32_t crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define UPDC32(octet,crc) (crc_32_tab[((crc) ^ (octet)) & 0xff] ^ ((crc) >> 8))

uint32_t crc32buf(char *buf, size_t len)
{
      uint32_t crc;

      crc = 0xFFFFFFFF;

      for ( ; len; --len, ++buf)
      {
            crc = UPDC32(*buf, crc);
      }

      return crc;
}
