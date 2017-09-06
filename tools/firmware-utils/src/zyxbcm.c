/*
 * zyxbcm.c - based on Jonas Gorski's spw303v.c
 *
 * Copyright (C) 2014 Álvaro Fernández Rojas <noltari@gmail.com>
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

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

#define TAGVER_LEN 4			/* Length of Tag Version */
#define SIG1_LEN 20			/* Company Signature 1 Length */
#define SIG2_LEN 14			/* Company Signature 2 Lenght */
#define BOARDID_LEN 16			/* Length of BoardId */
#define ENDIANFLAG_LEN 2		/* Endian Flag Length */
#define CHIPID_LEN 6			/* Chip Id Length */
#define IMAGE_LEN 10			/* Length of Length Field */
#define ADDRESS_LEN 12			/* Length of Address field */
#define DUALFLAG_LEN 2			/* Dual Image flag Length */
#define INACTIVEFLAG_LEN 2		/* Inactie Flag Length */
#define RSASIG_LEN 20			/* Length of RSA Signature in tag */
#define TAGINFO1_LEN 30			/* Length of vendor information field1 in tag */
#define ZYX_TAGINFO1_LEN 20		/* Length of vendor information field1 in tag */
#define FLASHLAYOUTVER_LEN 4		/* Length of Flash Layout Version String tag */
#define TAGINFO2_LEN 16			/* Length of vendor information field2 in tag */
#define CRC_LEN 4			/* Length of CRC in bytes */

#define IMAGETAG_CRC_START 0xFFFFFFFF

struct bcm_tag {
	char tagVersion[TAGVER_LEN];			// 0-3: Version of the image tag
	char sig_1[SIG1_LEN];				// 4-23: Company Line 1
	char sig_2[SIG2_LEN];				// 24-37: Company Line 2
	char chipid[CHIPID_LEN];			// 38-43: Chip this image is for
	char boardid[BOARDID_LEN];			// 44-59: Board name
	char big_endian[ENDIANFLAG_LEN];		// 60-61: Map endianness -- 1 BE 0 LE
	char totalLength[IMAGE_LEN];			// 62-71: Total length of image
	char cfeAddress[ADDRESS_LEN];			// 72-83: Address in memory of CFE
	char cfeLength[IMAGE_LEN];			// 84-93: Size of CFE
	char flashImageStart[ADDRESS_LEN];		// 94-105: Address in memory of image start (kernel for OpenWRT, rootfs for stock firmware)
	char flashRootLength[IMAGE_LEN];		// 106-115: Size of rootfs for flashing
	char kernelAddress[ADDRESS_LEN];		// 116-127: Address in memory of kernel
	char kernelLength[IMAGE_LEN];			// 128-137: Size of kernel
	char dualImage[DUALFLAG_LEN];			// 138-139: Unused at present
	char inactiveFlag[INACTIVEFLAG_LEN];		// 140-141: Unused at present
	char rsa_signature[RSASIG_LEN];			// 142-161: RSA Signature (unused at present; some vendors may use this)
	char information1[TAGINFO1_LEN];		// 162-191: Compilation and related information (not generated/used by OpenWRT)
	char flashLayoutVer[FLASHLAYOUTVER_LEN];	// 192-195: Version flash layout
	char fskernelCRC[CRC_LEN];			// 196-199: kernel+rootfs CRC32
	char information2[TAGINFO2_LEN];		// 200-215: Unused at present except Alice Gate where is is information
	char imageCRC[CRC_LEN];				// 216-219: CRC32 of image less imagetag (kernel for Alice Gate)
	char rootfsCRC[CRC_LEN];			// 220-223: CRC32 of rootfs partition
	char kernelCRC[CRC_LEN];			// 224-227: CRC32 of kernel partition
	char imageSequence[4];				// 228-231: Image sequence number
	char rootLength[4];				// 232-235: steal from reserved1 to keep the real root length so we can use in the flash map even after we have change the rootLength to 0 to satisfy devices that check CRC on every boot
	char headerCRC[CRC_LEN];			// 236-239: CRC32 of header excluding tagVersion
	char reserved2[16];				// 240-255: Unused at present
};

struct zyxbcm_tag {
	char tagVersion[TAGVER_LEN];			// 0-3: Version of the image tag
	char sig_1[SIG1_LEN];				// 4-23: Company Line 1
	char sig_2[SIG2_LEN];				// 24-37: Company Line 2
	char chipid[CHIPID_LEN];			// 38-43: Chip this image is for
	char boardid[BOARDID_LEN];			// 44-59: Board name
	char big_endian[ENDIANFLAG_LEN];		// 60-61: Map endianness -- 1 BE 0 LE
	char totalLength[IMAGE_LEN];			// 62-71: Total length of image
	char cfeAddress[ADDRESS_LEN];			// 72-83: Address in memory of CFE
	char cfeLength[IMAGE_LEN];			// 84-93: Size of CFE
	char flashImageStart[ADDRESS_LEN];		// 94-105: Address in memory of image start (kernel for OpenWRT, rootfs for stock firmware)
	char flashRootLength[IMAGE_LEN];		// 106-115: Size of rootfs for flashing
	char kernelAddress[ADDRESS_LEN];		// 116-127: Address in memory of kernel
	char kernelLength[IMAGE_LEN];			// 128-137: Size of kernel
	char dualImage[DUALFLAG_LEN];			// 138-139: Unused at present
	char inactiveFlag[INACTIVEFLAG_LEN];		// 140-141: Unused at present
	char rsa_signature[RSASIG_LEN];			// 142-161: RSA Signature (unused at present; some vendors may use this)
	char information1[ZYX_TAGINFO1_LEN];		// 162-181: Compilation and related information (not generated/used by OpenWRT)
	char flashImageEnd[ADDRESS_LEN];		// 182-193: Address in memory of image end
	char fskernelCRC[CRC_LEN];			// 194-197: kernel+rootfs CRC32
	char reserved1[2];				// 198-199: Unused at present
	char information2[TAGINFO2_LEN];		// 200-215: Unused at present except Alice Gate where is is information
	char imageCRC[CRC_LEN];				// 216-219: CRC32 of image less imagetag (kernel for Alice Gate)
	char rootfsCRC[CRC_LEN];			// 220-223: CRC32 of rootfs partition
	char kernelCRC[CRC_LEN];			// 224-227: CRC32 of kernel partition
	char imageSequence[4];				// 228-231: Image sequence number
	char rootLength[4];				// 232-235: steal from reserved1 to keep the real root length so we can use in the flash map even after we have change the rootLength to 0 to satisfy devices that check CRC on every boot
	char headerCRC[CRC_LEN];			// 236-239: CRC32 of header excluding tagVersion
	char reserved2[16];				// 240-255: Unused at present
};

static uint32_t crc32tab[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

uint32_t crc32(uint32_t crc, uint8_t *data, size_t len)
{
	while (len--)
		crc = (crc >> 8) ^ crc32tab[(crc ^ *data++) & 0xFF];

	return crc;
}

void fix_header(void *buf)
{
	struct bcm_tag *bcmtag = buf;
	struct zyxbcm_tag *zyxtag = buf;
	uint8_t fskernel_crc[CRC_LEN];
	uint32_t crc;
	uint64_t flash_start, rootfs_len, kernel_len;

	/* Backup values */
	flash_start = strtoul(bcmtag->flashImageStart, NULL, 10);
	rootfs_len = strtoul(bcmtag->flashRootLength, NULL, 10);
	kernel_len = strtoul(bcmtag->kernelLength, NULL, 10);
	memcpy(fskernel_crc, bcmtag->fskernelCRC, CRC_LEN);

	/* Clear values */
	zyxtag->information1[ZYX_TAGINFO1_LEN - 1] = 0;
	memset(zyxtag->flashImageEnd, 0, ADDRESS_LEN);
	memset(zyxtag->fskernelCRC, 0, CRC_LEN);
	memset(zyxtag->reserved1, 0, 2);

	/* Replace values */
	sprintf(zyxtag->flashImageEnd, "%lu", flash_start + rootfs_len + kernel_len);
	memcpy(zyxtag->fskernelCRC, fskernel_crc, CRC_LEN);

	/* Update tag crc */
	crc = htonl(crc32(IMAGETAG_CRC_START, buf, 236));
	memcpy(zyxtag->headerCRC, &crc, 4);
}

void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: zyxbcm [-i <inputfile>] [-o <outputfile>]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	char buf[1024];	/* keep this at 1k or adjust garbage calc below */
	FILE *in = stdin, *out = stdout;
	char *ifn = NULL, *ofn = NULL;
	size_t n;
	int c, first_block = 1;

	while ((c = getopt(argc, argv, "i:o:h")) != -1) {
		switch (c) {
			case 'i':
				ifn = optarg;
				break;
			case 'o':
				ofn = optarg;
				break;
			case 'h':
			default:
				usage();
		}
	}

	if (optind != argc || optind == 1) {
		fprintf(stderr, "illegal arg \"%s\"\n", argv[optind]);
		usage();
	}

	if (ifn && !(in = fopen(ifn, "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", ifn);
		usage();
	}

	if (ofn && !(out = fopen(ofn, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
		usage();
	}

	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (n < sizeof(buf)) {
			if (ferror(in)) {
			FREAD_ERROR:
				fprintf(stderr, "fread error\n");
				return EXIT_FAILURE;
			}
		}

		if (first_block && n >= 256) {
			fix_header(buf);
			first_block = 0;
		}

		if (!fwrite(buf, n, 1, out)) {
		FWRITE_ERROR:
			fprintf(stderr, "fwrite error\n");
			return EXIT_FAILURE;
		}
	}

	if (ferror(in)) {
		goto FREAD_ERROR;
	}

	if (fflush(out)) {
		goto FWRITE_ERROR;
	}

	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}
