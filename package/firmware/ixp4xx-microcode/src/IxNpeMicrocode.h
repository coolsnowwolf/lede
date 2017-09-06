/*
 * IxNpeMicrocode.h - Headerfile for compiling the Intel microcode C file
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 *
 *
 * compile with
 *
 * gcc -Wall IxNpeMicrocode.c -o IxNpeMicrocode
 *
 * Executing the resulting binary on your build-host creates the
 * "NPE-[ABC].xxxxxxxx" files containing the selected microcode
 *
 * fetch the IxNpeMicrocode.c from the Intel Access Library.
 * It will include this header.
 *
 * select Images for every NPE from the following
 * (used C++ comments for easy uncommenting ....)
 */

// #define IX_NPEDL_NPEIMAGE_NPEA_ETH_SPAN_MASK_FIREWALL_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEA_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEA_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEA_HSS_TSLOT_SWITCH
#define IX_NPEDL_NPEIMAGE_NPEA_ETH_SPAN_FIREWALL_VLAN_QOS_HDR_CONV
// #define IX_NPEDL_NPEIMAGE_NPEA_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS
// #define IX_NPEDL_NPEIMAGE_NPEA_ETH_LEARN_FILTER_SPAN_FIREWALL
#define IX_NPEDL_NPEIMAGE_NPEA_HSS_2_PORT
// #define IX_NPEDL_NPEIMAGE_NPEA_DMA
// #define IX_NPEDL_NPEIMAGE_NPEA_ATM_MPHY_12_PORT
// #define IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_MPHY_1_PORT
// #define IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_SPHY_1_PORT
// #define IX_NPEDL_NPEIMAGE_NPEA_HSS0
// #define IX_NPEDL_NPEIMAGE_NPEA_WEP


// #define IX_NPEDL_NPEIMAGE_NPEB_ETH_SPAN_MASK_FIREWALL_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEB_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEB_DMA
#define IX_NPEDL_NPEIMAGE_NPEB_ETH_SPAN_FIREWALL_VLAN_QOS_HDR_CONV
// #define IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS
// #define IX_NPEDL_NPEIMAGE_NPEB_ETH_LEARN_FILTER_SPAN_FIREWALL


// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_SPAN_MASK_FIREWALL_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_SPAN_VLAN_QOS_HDR_CONV_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_MASK_FIREWALL_VLAN_QOS_EXTMIB
// #define IX_NPEDL_NPEIMAGE_NPEC_DMA
// #define IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_ETH_LEARN_FILTER_SPAN
// #define IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_ETH_LEARN_FILTER_FIREWALL
#define IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_CCM_ETH
// #define IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_CCM_EXTSHA_ETH
// #define IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_ETH_LEARN_FILTER_SPAN_FIREWALL
// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_SPAN_FIREWALL_VLAN_QOS_HDR_CONV
// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_FIREWALL_VLAN_QOS
// #define IX_NPEDL_NPEIMAGE_NPEC_ETH_LEARN_FILTER_SPAN_FIREWALL


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <byteswap.h>
#include <string.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define to_le32(x) (x)
#define to_be32(x) bswap_32(x)
#else
#define to_be32(x) (x)
#define to_le32(x) bswap_32(x)
#endif

struct dl_image {
	unsigned magic;
	unsigned id;
	unsigned size;
	unsigned data[0];
};

const unsigned IxNpeMicrocode_array[];

int main(int argc, char *argv[])
{
	struct dl_image *image = (struct dl_image *)IxNpeMicrocode_array;
	int imgsiz, i, fd, cnt;
	const unsigned *arrayptr = IxNpeMicrocode_array;
	const char *names[] = { "IXP425", "IXP465", "unknown" };
	int bigendian = 1;

	if (argc > 1) {
		if (!strcmp(argv[1], "-le"))
			bigendian = 0;
		else if (!strcmp(argv[1], "-be"))
			bigendian = 1;
		else {
			printf("Usage: %s <-le|-be>\n", argv[0]);
			return EXIT_FAILURE;
		}
	}

	for (image = (struct dl_image *)arrayptr, cnt=0;
		(image->id != 0xfeedf00d) && (image->magic == 0xfeedf00d);
		image = (struct dl_image *)(arrayptr), cnt++)
	{
		unsigned char field[4];
		imgsiz = image->size + 3;
		*(unsigned*)field = to_be32(image->id);
		char filename[40], slnk[10];

		sprintf(filename, "NPE-%c.%08x", (field[0] & 0xf) + 'A',
			image->id);
		if (image->id == 0x00090000)
			sprintf(slnk, "NPE-%c-HSS", (field[0] & 0xf) + 'A');
		else
			sprintf(slnk, "NPE-%c", (field[0] & 0xf) + 'A');

		printf("Writing image: %s.NPE_%c Func: %2x Rev: %02x.%02x "
			"Size: %5d to: '%s'\n",
			names[field[0] >> 4], (field[0] & 0xf) + 'A',
			field[1], field[2], field[3], imgsiz*4, filename);
		fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
		if (fd >= 0) {
			for (i=0; i<imgsiz; i++) {
				*(unsigned*)field = bigendian ?
					to_be32(arrayptr[i]) :
					to_le32(arrayptr[i]);
				write(fd, field, sizeof(field));
			}
			close(fd);
			unlink(slnk);
			symlink(filename, slnk);
		} else {
			perror(filename);
		}
		arrayptr += imgsiz;
	}
	close(fd);
	return 0;
}
