/*
 * Copyright (C) 2013 Jeff Kent <jeff@jkent.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This tool encrypts and decrypts uImage formatted firmware for Hilink
 * HLK-RM04 wireless modules.  It will also truncate a dump of mtd6 and make
 * it an image suitable for flashing via the stock firmware upgrade page.
 *
 * Build instructions: 
 *   gcc -lcrypto hlkcrypt.c -o hlkcrypt
 */
 
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <openssl/des.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
 
#define DES_KEY "H@L9K*(3"
 
#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif
 
#define IH_MAGIC    0x27051956
#define IH_NMLEN    32
typedef struct image_header {
    uint32_t    ih_magic;   /* Image Header Magic Number    */
    uint32_t    ih_hcrc;    /* Image Header CRC Checksum    */
    uint32_t    ih_time;    /* Image Creation Timestamp */
    uint32_t    ih_size;    /* Image Data Size      */
    uint32_t    ih_load;    /* Data  Load  Address      */
    uint32_t    ih_ep;      /* Entry Point Address      */
    uint32_t    ih_dcrc;    /* Image Data CRC Checksum  */
    uint8_t     ih_os;      /* Operating System     */
    uint8_t     ih_arch;    /* CPU architecture     */
    uint8_t     ih_type;    /* Image Type           */
    uint8_t     ih_comp;    /* Compression Type     */
    uint8_t     ih_name[IH_NMLEN];  /* Image Name       */
} image_header_t;
 
static int temp_fd = -1;
static DES_key_schedule schedule;
 
static void show_usage(const char *arg0);
static void exit_cleanup(void);
static void copy_file(int src, int dst);
static void do_encrypt(void *p, off_t len);
static void do_decrypt(void *p, off_t len);
 
 
int main(int argc, char **argv)
{
	int encrypt_opt = 0;
	int decrypt_opt = 0;
	int input_opt = 0;
	int output_opt = 0;
	char *input_filename = NULL;
	char *output_filename = NULL;
 
	int input_fd;
	int output_fd;
	off_t file_len;
	char *p;
	char buf[sizeof(image_header_t) + 3];
	image_header_t *header;
 
	while (1) {
		static struct option long_options[] = {
			{"encrypt", no_argument,       0, 'e'},
			{"decrypt", no_argument,       0, 'd'},
			{"input",   required_argument, 0, 'i'},
			{"output",  required_argument, 0, 'o'},
			{0,         0,                 0, 0  }
		};
		int option_index = 0;
		int c = getopt_long(argc, argv, "dei:o:",
		                long_options, &option_index);
		if (c == -1)
			break;
 
		switch (c) {
		case 'd':
			decrypt_opt++;
			if (decrypt_opt > 1) {
				fprintf(stderr, "%s: decrypt may only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			break;
 
		case 'e':
			encrypt_opt++;
			if (encrypt_opt > 1) {
				fprintf(stderr, "%s: encrypt may only be specified once\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			break;
 
		case 'i':
			input_opt++;
			if (input_opt > 1) {
				fprintf(stderr, "%s: only one input file may be specified\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			if (strcmp("-", optarg) != 0) {
				input_filename = optarg;
			}
			break;
 
		case 'o':
			output_opt++;
			if (output_opt > 1) {
				fprintf(stderr, "%s: only one output file may be specified\n",
				        argv[0]);
				show_usage(argv[0]);
			}
			if (strcmp("-", optarg) != 0) {
				output_filename = optarg;
			}
			break;
 
		case '?':
			exit(-1);
 
		default:
			abort();
		}
	}
 
	if (decrypt_opt && encrypt_opt) {
		fprintf(stderr, "%s: decrypt and encrypt may not be used together\n",
		        argv[0]);
		show_usage(argv[0]);
	}
 
	if (!decrypt_opt && !encrypt_opt) {
		fprintf(stderr, "%s: neither decrypt or encrypt were specified\n",
		        argv[0]);
		show_usage(argv[0]);
	}
 
	temp_fd = fileno(tmpfile());
	if (temp_fd < 0) {
		fprintf(stderr, "Can't create temporary file\n");
		exit(EXIT_FAILURE);
	}
 
	atexit(exit_cleanup);
	DES_set_key_unchecked((const_DES_cblock *)DES_KEY, &schedule);
 
	if (input_filename) {
		input_fd = open(input_filename, O_RDONLY);
		if (input_fd < 0) {
			fprintf(stderr, "Can't open %s for reading: %s\n", input_filename,
			        strerror(errno));
			exit(EXIT_FAILURE);
		}
		copy_file(input_fd, temp_fd);
		close(input_fd);
	}
	else {
		copy_file(STDIN_FILENO, temp_fd);
	}
 
	file_len = lseek(temp_fd, 0, SEEK_CUR);
	if (file_len < 64) {
		fprintf(stderr, "Not enough data\n");
		exit(EXIT_FAILURE);
	}
 
	p = mmap(0, file_len, PROT_READ|PROT_WRITE, MAP_SHARED, temp_fd, 0);
	if (p == MAP_FAILED) {
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}	
 
	if (encrypt_opt) {
		header = (image_header_t *)p;
		off_t len = min(file_len,
		                ntohl(header->ih_size) + sizeof(image_header_t));
		if (ntohl(header->ih_magic) != IH_MAGIC) {
			fprintf(stderr, "Header magic incorrect: "
			        "expected 0x%08X, got 0x%08X\n",
			        IH_MAGIC, ntohl(header->ih_magic));
			munmap(p, file_len);
			exit(EXIT_FAILURE);
		}
		do_encrypt(p, len);
		munmap(p, file_len);
		if (len != file_len) {
			if (ftruncate(temp_fd, len) < 0) {
				fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}		
	}
 
	if (decrypt_opt) {
		off_t header_len = min(file_len, sizeof(image_header_t) + 3);
		memcpy(buf, p, header_len);
		do_decrypt(buf, header_len);
		header = (image_header_t *)buf;
		if (ntohl(header->ih_magic) != IH_MAGIC) {
			fprintf(stderr, "Header magic incorrect: "
			        "expected 0x%08X, got 0x%08X\n",
			        IH_MAGIC, ntohl(header->ih_magic));
			exit(EXIT_FAILURE);
		}
		do_decrypt(p, file_len);
		munmap(p, file_len);
	}
 
	lseek(temp_fd, 0, SEEK_SET);
	if (output_filename) {
		output_fd = creat(output_filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if (output_fd < 0) {
			fprintf(stderr, "Can't open %s for writing: %s\n",
			        output_filename, strerror(errno));
			exit(EXIT_FAILURE);
		}
		copy_file(temp_fd, output_fd);
		close(output_fd);
	}
	else {
		copy_file(temp_fd, STDOUT_FILENO);
	}
 
	exit(EXIT_SUCCESS);
	return 0;
}
 
static void show_usage(const char *arg0)
{
	fprintf(stderr, "usage: %s -d|-e [-i FILE] [-o FILE]\n\n", arg0);
	fprintf(stderr, "%-15s %s\n", "-d, --decrypt", "decrypt data");
	fprintf(stderr, "%-15s %s\n", "-e, --encrypt", "encrypt data");
	fprintf(stderr, "%-15s %s\n", "-i, --input", "intput file (defaults to stdin)");
	fprintf(stderr, "%-15s %s\n", "-o, --output", "output file (defaults to stdout)");
	exit(-1);
}
 
static void exit_cleanup(void)
{
	if (temp_fd >= 0) {
		close(temp_fd);
	}
}
 
static void copy_file(int src, int dst)
{
	char buf[4096];
	ssize_t size;
 
	while ((size = read(src, buf, 4096)) > 0) {
        write(dst, buf, size);
    }
}
 
static void do_encrypt(void *p, off_t len)
{
	DES_cblock *pblock;
	int num_blocks;
 
	num_blocks = len / 8;
	pblock = (DES_cblock *) p;
	while (num_blocks--) {
		DES_ecb_encrypt(pblock, pblock, &schedule, DES_ENCRYPT);
		pblock++;
	}
 
	num_blocks = (len - 3) / 8;
	pblock = (DES_cblock *) (p + 3);
	while (num_blocks--) {
		DES_ecb_encrypt(pblock, pblock, &schedule, DES_ENCRYPT);
		pblock++;
	}
}
 
static void do_decrypt(void *p, off_t len)
{
	DES_cblock *pblock;
	int num_blocks;
 
	num_blocks = (len - 3) / 8;
	pblock = (DES_cblock *) (p + 3);
	while (num_blocks--) {
		DES_ecb_encrypt(pblock, pblock, &schedule, DES_DECRYPT);
		pblock++;
	}
 
	num_blocks = len / 8;
	pblock = (DES_cblock *) p;
	while (num_blocks--) {
		DES_ecb_encrypt(pblock, pblock, &schedule, DES_DECRYPT);
		pblock++;
	}
}
