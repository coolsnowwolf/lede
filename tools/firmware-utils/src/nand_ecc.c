/*
 * calculate ecc code for nand flash
 *
 * Copyright (C) 2008 yajin <yajin@vm-kernel.org>
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

#define DEF_NAND_PAGE_SIZE   2048
#define DEF_NAND_OOB_SIZE     64
#define DEF_NAND_ECC_OFFSET   0x28

static int page_size = DEF_NAND_PAGE_SIZE;
static int oob_size = DEF_NAND_OOB_SIZE;
static int ecc_offset = DEF_NAND_ECC_OFFSET;

/*
 * Pre-calculated 256-way 1 byte column parity
 */
static const uint8_t nand_ecc_precalc_table[] = {
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x6a, 0x3f, 0x3c, 0x69, 0x33, 0x66, 0x65, 0x30, 0x30, 0x65, 0x66, 0x33, 0x69, 0x3c, 0x3f, 0x6a,
	0x0f, 0x5a, 0x59, 0x0c, 0x56, 0x03, 0x00, 0x55, 0x55, 0x00, 0x03, 0x56, 0x0c, 0x59, 0x5a, 0x0f,
	0x0c, 0x59, 0x5a, 0x0f, 0x55, 0x00, 0x03, 0x56, 0x56, 0x03, 0x00, 0x55, 0x0f, 0x5a, 0x59, 0x0c,
	0x69, 0x3c, 0x3f, 0x6a, 0x30, 0x65, 0x66, 0x33, 0x33, 0x66, 0x65, 0x30, 0x6a, 0x3f, 0x3c, 0x69,
	0x03, 0x56, 0x55, 0x00, 0x5a, 0x0f, 0x0c, 0x59, 0x59, 0x0c, 0x0f, 0x5a, 0x00, 0x55, 0x56, 0x03,
	0x66, 0x33, 0x30, 0x65, 0x3f, 0x6a, 0x69, 0x3c, 0x3c, 0x69, 0x6a, 0x3f, 0x65, 0x30, 0x33, 0x66,
	0x65, 0x30, 0x33, 0x66, 0x3c, 0x69, 0x6a, 0x3f, 0x3f, 0x6a, 0x69, 0x3c, 0x66, 0x33, 0x30, 0x65,
	0x00, 0x55, 0x56, 0x03, 0x59, 0x0c, 0x0f, 0x5a, 0x5a, 0x0f, 0x0c, 0x59, 0x03, 0x56, 0x55, 0x00
};

/**
 * nand_calculate_ecc - [NAND Interface] Calculate 3-byte ECC for 256-byte block
 * @dat:	raw data
 * @ecc_code:	buffer for ECC
 */
int nand_calculate_ecc(const uint8_t *dat,
		       uint8_t *ecc_code)
{
	uint8_t idx, reg1, reg2, reg3, tmp1, tmp2;
	int i;

	/* Initialize variables */
	reg1 = reg2 = reg3 = 0;

	/* Build up column parity */
	for(i = 0; i < 256; i++) {
		/* Get CP0 - CP5 from table */
		idx = nand_ecc_precalc_table[*dat++];
		reg1 ^= (idx & 0x3f);

		/* All bit XOR = 1 ? */
		if (idx & 0x40) {
			reg3 ^= (uint8_t) i;
			reg2 ^= ~((uint8_t) i);
		}
	}

	/* Create non-inverted ECC code from line parity */
	tmp1  = (reg3 & 0x80) >> 0; /* B7 -> B7 */
	tmp1 |= (reg2 & 0x80) >> 1; /* B7 -> B6 */
	tmp1 |= (reg3 & 0x40) >> 1; /* B6 -> B5 */
	tmp1 |= (reg2 & 0x40) >> 2; /* B6 -> B4 */
	tmp1 |= (reg3 & 0x20) >> 2; /* B5 -> B3 */
	tmp1 |= (reg2 & 0x20) >> 3; /* B5 -> B2 */
	tmp1 |= (reg3 & 0x10) >> 3; /* B4 -> B1 */
	tmp1 |= (reg2 & 0x10) >> 4; /* B4 -> B0 */

	tmp2  = (reg3 & 0x08) << 4; /* B3 -> B7 */
	tmp2 |= (reg2 & 0x08) << 3; /* B3 -> B6 */
	tmp2 |= (reg3 & 0x04) << 3; /* B2 -> B5 */
	tmp2 |= (reg2 & 0x04) << 2; /* B2 -> B4 */
	tmp2 |= (reg3 & 0x02) << 2; /* B1 -> B3 */
	tmp2 |= (reg2 & 0x02) << 1; /* B1 -> B2 */
	tmp2 |= (reg3 & 0x01) << 1; /* B0 -> B1 */
	tmp2 |= (reg2 & 0x01) << 0; /* B7 -> B0 */

	/* Calculate final ECC code */
#ifdef CONFIG_MTD_NAND_ECC_SMC
	ecc_code[0] = ~tmp2;
	ecc_code[1] = ~tmp1;
#else
	ecc_code[0] = ~tmp1;
	ecc_code[1] = ~tmp2;
#endif
	ecc_code[2] = ((~reg1) << 2) | 0x03;

	return 0;
}

/*
 *  usage: bb-nandflash-ecc    start_address  size
 */
void usage(const char *prog)
{
	fprintf(stderr, "Usage: %s [options] <input> <output>\n"
		"Options:\n"
		"    -p <pagesize>      NAND page size (default: %d)\n"
		"    -o <oobsize>       NAND OOB size (default: %d)\n"
		"    -e <offset>        NAND ECC offset (default: %d)\n"
		"\n", prog, DEF_NAND_PAGE_SIZE, DEF_NAND_OOB_SIZE,
		DEF_NAND_ECC_OFFSET);
	exit(1);
}

/*start_address/size does not include oob
  */
int main(int argc, char **argv)
{
	uint8_t *page_data = NULL;
	uint8_t *ecc_data;
	int infd = -1, outfd = -1;
	int ret = 1;
	ssize_t bytes;
	int ch;

	while ((ch = getopt(argc, argv, "e:o:p:")) != -1) {
		switch(ch) {
		case 'p':
			page_size = strtoul(optarg, NULL, 0);
			break;
		case 'o':
			oob_size = strtoul(optarg, NULL, 0);
			break;
		case 'e':
			ecc_offset = strtoul(optarg, NULL, 0);
			break;
		default:
			usage(argv[0]);
		}
	}
	argc -= optind;
	if (argc < 2)
		usage(argv[0]);

	argv += optind;

	infd = open(argv[0], O_RDONLY, 0);
	if (infd < 0) {
		perror("open input file");
		goto out;
	}

	outfd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 0644);
	if (outfd < 0) {
		perror("open output file");
		goto out;
	}

	page_data = malloc(page_size + oob_size);

	while ((bytes = read(infd, page_data, page_size)) == page_size) {
		int j;

		ecc_data = page_data + page_size + ecc_offset;
		for (j = 0; j < page_size / 256; j++)
		{
			nand_calculate_ecc(page_data + j * 256, ecc_data);
			ecc_data += 3;
		}
		write(outfd, page_data, page_size + oob_size);
	}

	ret = 0;
out:
	if (infd >= 0)
		close(infd);
	if (outfd >= 0)
		close(outfd);
	if (page_data)
		free(page_data);
	return ret;
}

