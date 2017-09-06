/*
 * imagetag.c
 *
 * Copyright (C) 2005 Mike Baker
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 * Copyrigth (C) 2010 Daniel Dickinson <openwrt@cshore.neomailbox.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#include "mtd.h"
#include "crc32.h"

#define TAGVER_LEN		4	/* Length of Tag Version */
#define TAGLAYOUT_LEN		4	/* Length of FlashLayoutVer */
#define SIG1_LEN		20	/* Company Signature 1 Length */
#define SIG2_LEN		14	/* Company Signature 2 Length */
#define BOARDID_LEN		16	/* Length of BoardId */
#define ENDIANFLAG_LEN		2	/* Endian Flag Length */
#define CHIPID_LEN		6	/* Chip Id Length */
#define IMAGE_LEN		10	/* Length of Length Field */
#define ADDRESS_LEN		12	/* Length of Address field */
#define DUALFLAG_LEN		2	/* Dual Image flag Length */
#define INACTIVEFLAG_LEN	2	/* Inactie Flag Length */
#define RSASIG_LEN		20	/* Length of RSA Signature in tag */
#define TAGINFO1_LEN		30	/* Length of vendor information field1 in tag */
#define FLASHLAYOUTVER_LEN	4	/* Length of Flash Layout Version String tag */
#define TAGINFO2_LEN		16	/* Length of vendor information field2 in tag */
#define ALTTAGINFO_LEN		54	/* Alternate length for vendor information; Pirelli */

#define NUM_PIRELLI		2
#define IMAGETAG_CRC_START	0xFFFFFFFF

#define PIRELLI_BOARDS { \
	"AGPF-S0", \
	"DWV-S0", \
}
/*
 * The broadcom firmware assumes the rootfs starts the image,
 * therefore uses the rootfs start (flash_image_address)
 * to determine where to flash the image.  Since we have the kernel first
 * we have to give it the kernel address, but the crc uses the length
 * associated with this address (root_length), which is added to the kernel
 * length (kernel_length) to determine the length of image to flash and thus
 * needs to be rootfs + deadcode (jffs2 EOF marker)
*/

struct bcm_tag {
	/* 0-3: Version of the image tag */
	char tag_version[TAGVER_LEN];
	/* 4-23: Company Line 1 */
	char sig_1[SIG1_LEN];
	/*  24-37: Company Line 2 */
	char sig_2[SIG2_LEN];
	/* 38-43: Chip this image is for */
	char chip_id[CHIPID_LEN];
	/* 44-59: Board name */
	char board_id[BOARDID_LEN];
	/* 60-61: Map endianness -- 1 BE 0 LE */
	char big_endian[ENDIANFLAG_LEN];
	/* 62-71: Total length of image */
	char total_length[IMAGE_LEN];
	/* 72-83: Address in memory of CFE */
	char cfe__address[ADDRESS_LEN];
	/* 84-93: Size of CFE */
	char cfe_length[IMAGE_LEN];
	/* 94-105: Address in memory of image start
	 * (kernel for OpenWRT, rootfs for stock firmware)
	 */
	char flash_image_start[ADDRESS_LEN];
	/* 106-115: Size of rootfs */
	char root_length[IMAGE_LEN];
	/* 116-127: Address in memory of kernel */
	char kernel_address[ADDRESS_LEN];
	/* 128-137: Size of kernel */
	char kernel_length[IMAGE_LEN];
	/* 138-139: Unused at the moment */
	char dual_image[DUALFLAG_LEN];
	/* 140-141: Unused at the moment */
	char inactive_flag[INACTIVEFLAG_LEN];
	/* 142-161: RSA Signature (not used; some vendors may use this) */
	char rsa_signature[RSASIG_LEN];
	/* 162-191: Compilation and related information (not used in OpenWrt) */
	char information1[TAGINFO1_LEN];
	/* 192-195: Version flash layout */
	char flash_layout_ver[FLASHLAYOUTVER_LEN];
	/* 196-199: kernel+rootfs CRC32 */
	__u32 fskernel_crc;
	/* 200-215: Unused except on Alice Gate where is is information */
	char information2[TAGINFO2_LEN];
	/* 216-219: CRC32 of image less imagetag (kernel for Alice Gate) */
	__u32 image_crc;
	/* 220-223: CRC32 of rootfs partition */
	__u32 rootfs_crc;
	/* 224-227: CRC32 of kernel partition */
	__u32 kernel_crc;
	/* 228-231: Image sequence number */
	char image_sequence[4];
	/* 222-235: Openwrt: real rootfs length */
	__u32 real_rootfs_length;
	/* 236-239: CRC32 of header excluding last 20 bytes */
	__u32 header_crc;
	/* 240-255: Unused at present */
	char reserved2[16];
};
ssize_t pread(int fd, void *buf, size_t count, off_t offset);
ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);

#define CRC_START 0xFFFFFFFF

static uint32_t strntoul(char *str, char **endptr, int base, size_t len) {
  char *newstr;
  uint32_t res = 0;

  newstr = calloc(len + 1, sizeof(char));
  if (newstr) {
	strncpy(newstr, str, len); 
	res = strtoul(newstr, endptr, base);
	free(newstr);
  }
  return res;
}

uint32_t compute_crc32(uint32_t crc, off_t start, size_t compute_len, int fd)
{
	uint8_t readbuf[1024];
	ssize_t res;
	off_t offset = start;

	/* Read a buffer's worth of bytes  */
	while (fd && (compute_len >= sizeof(readbuf))) {
		res = pread(fd, readbuf, sizeof(readbuf), offset);
		crc = crc32(crc, readbuf, res);
		compute_len = compute_len - res;
		offset += res;
	}

	/* Less than buffer-size bytes remains, read compute_len bytes */
	if (fd && (compute_len > 0)) {
	  res = pread(fd, readbuf, compute_len, offset);
	  crc = crc32(crc, readbuf, res);
	}

	return crc;
}

int
trx_fixup(int fd, const char *name)
{
	struct mtd_info_user mtdInfo;
	unsigned long len;
	void *ptr, *scan;
	int bfd;
	struct bcm_tag *tag;
	ssize_t res;
	uint32_t cfelen, imagelen, imagestart, rootfslen;
	uint32_t imagecrc, rootfscrc, headercrc;
	uint32_t offset = 0;
	cfelen = imagelen = imagestart = imagecrc = rootfscrc = headercrc = rootfslen = 0;


	if (ioctl(fd, MEMGETINFO, &mtdInfo) < 0) {
		fprintf(stderr, "Failed to get mtd info\n");
		goto err;
	}

	len = mtdInfo.size;
	if (mtdInfo.size <= 0) {
		fprintf(stderr, "Invalid MTD device size\n");
		goto err;
	}

	bfd = mtd_open(name, true);
	ptr = mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, bfd, 0);
	if (!ptr || (ptr == (void *) -1)) {
		perror("mmap");
		goto err1;
	}

	tag = (struct bcm_tag *) (ptr);

	cfelen = strntoul(&tag->cfe_length[0], NULL, 10, IMAGE_LEN);
	if (cfelen) {
	  fprintf(stderr, "Non-zero CFE length.  This is currently unsupported.\n");
	  exit(1);
	}

	headercrc = compute_crc32(CRC_START, offset, offsetof(struct bcm_tag, header_crc), fd);
	if (headercrc != *(uint32_t *)(&tag->header_crc)) {
		fprintf(stderr, "Tag verify failed.  This may not be a valid image.\n");
		exit(1);
	}

	sprintf(&tag->root_length[0], "%u", 0);
	strncpy(&tag->total_length[0], &tag->kernel_length[0], IMAGE_LEN);

	imagestart = sizeof(tag);
	memcpy(&tag->image_crc, &tag->kernel_crc, sizeof(uint32_t));
	memcpy(&tag->fskernel_crc, &tag->kernel_crc, sizeof(uint32_t));
	rootfscrc = CRC_START;
	memcpy(&tag->rootfs_crc, &rootfscrc, sizeof(uint32_t));
	headercrc = crc32(CRC_START, tag, offsetof(struct bcm_tag, header_crc));
	memcpy(&tag->header_crc, &headercrc, sizeof(uint32_t));

	msync(ptr, sizeof(struct bcm_tag), MS_SYNC|MS_INVALIDATE);
	munmap(ptr, len);
	close(bfd);
	return 0;

err1:
	close(bfd);
err:
	fprintf(stderr, "Error fixing up imagetag header\n");
	return -1;
}


int
trx_check(int imagefd, const char *mtd, char *buf, int *len)
{
    struct bcm_tag *tag = (const struct bcm_tag *) buf;
	int fd;
	uint32_t headerCRC;
	uint32_t imageLen;

	if (strcmp(mtd, "linux") != 0)
		return 1;

	*len = read(imagefd, buf, sizeof(struct bcm_tag));
	if (*len < sizeof(struct bcm_tag)) {
		fprintf(stdout, "Could not get image header, file too small (%d bytes)\n", *len);
		return 0;
	}
	headerCRC = crc32buf(buf, offsetof(struct bcm_tag, header_crc));
	if (*(uint32_t *)(&tag->header_crc) != headerCRC) {
  
	  if (quiet < 2) {
		fprintf(stderr, "Bad header CRC got %08x, calculated %08x\n",
				*(uint32_t *)(&tag->header_crc), headerCRC);
		fprintf(stderr, "This is not the correct file format; refusing to flash.\n"
				"Please specify the correct file or use -f to force.\n");
	  }
	  return 0;
	}

	/* check if image fits to mtd device */
	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	imageLen = strntoul(&tag->total_length[0], NULL, 10, IMAGE_LEN);
	
	if(mtdsize < imageLen) {
		fprintf(stderr, "Image too big for partition: %s\n", mtd);
		close(fd);
		return 0;
	}

	close(fd);
	return 1;
}

int
mtd_fixtrx(const char *mtd, size_t offset, size_t data_size)
{
	int fd;
	struct bcm_tag *tag;
	char *buf;
	ssize_t res;
	size_t block_offset;
	uint32_t cfelen, imagelen, imagestart, rootfslen;
	uint32_t imagecrc, rootfscrc, headercrc;
	cfelen = imagelen = imagestart = imagecrc = rootfscrc = headercrc = rootfslen = 0;

	if (data_size)
		fprintf(stderr, "Specifying data size in unsupported for imagetag\n");

	if (quiet < 2)
		fprintf(stderr, "Trying to fix trx header in %s at 0x%x...\n", mtd, offset);

	block_offset = offset & ~(erasesize - 1);
	offset -= block_offset;

	fd = mtd_check_open(mtd);
	if(fd < 0) {
		fprintf(stderr, "Could not open mtd device: %s\n", mtd);
		exit(1);
	}

	if (block_offset + erasesize > mtdsize) {
		fprintf(stderr, "Offset too large, device size 0x%x\n", mtdsize);
		exit(1);
	}

	buf = malloc(erasesize);
	if (!buf) {
		perror("malloc");
		exit(1);
	}

	res = pread(fd, buf, erasesize, block_offset);
	if (res != erasesize) {
		perror("pread");
		exit(1);
	}

	tag = (struct bcm_tag *) (buf + offset);

	cfelen = strntoul(tag->cfe_length, NULL, 10, IMAGE_LEN);
	if (cfelen) {
	  fprintf(stderr, "Non-zero CFE length.  This is currently unsupported.\n");
	  exit(1);
	}

	if (quiet < 2) {
	  fprintf(stderr, "Verifying we actually have an imagetag.\n");
	}

	headercrc = compute_crc32(CRC_START, offset, offsetof(struct bcm_tag, header_crc), fd);
	if (headercrc != *(uint32_t *)(&tag->header_crc)) {
		fprintf(stderr, "Tag verify failed.  This may not be a valid image.\n");
		exit(1);
	}

	if (quiet < 2) {
	  fprintf(stderr, "Checking current fixed status.\n");
	}

	rootfslen = strntoul(&tag->root_length[0], NULL, 10, IMAGE_LEN);
	if (rootfslen == 0) {
	  if (quiet < 2) 
		fprintf(stderr, "Header already fixed, exiting\n");
	  close(fd);
	  return 0;
	}

	if (quiet < 2) {
	  fprintf(stderr, "Setting root length to 0.\n");
	}

	sprintf(&tag->root_length[0], "%u", 0);
	strncpy(&tag->total_length[0], &tag->kernel_length[0], IMAGE_LEN);

	if (quiet < 2) {
	  fprintf(stderr, "Recalculating CRCs.\n");
	}

	imagestart = sizeof(tag);
	memcpy(&tag->image_crc, &tag->kernel_crc, sizeof(uint32_t));
	memcpy(&tag->fskernel_crc, &tag->kernel_crc, sizeof(uint32_t));
	rootfscrc = CRC_START;
	memcpy(&tag->rootfs_crc, &rootfscrc, sizeof(uint32_t));
	headercrc = crc32(CRC_START, tag, offsetof(struct bcm_tag, header_crc));
	memcpy(&tag->header_crc, &headercrc, sizeof(uint32_t));

	if (quiet < 2) {
	  fprintf(stderr, "Erasing imagetag block\n");
	}

	if (mtd_erase_block(fd, block_offset)) {
		fprintf(stderr, "Can't erase block at 0x%x (%s)\n", block_offset, strerror(errno));
		exit(1);
	}

	if (quiet < 2) {
	  fprintf(stderr, "New image crc32: 0x%x, rewriting block\n", 
			  *(uint32_t *)(&tag->image_crc));
	  fprintf(stderr, "New header crc32: 0x%x, rewriting block\n", headercrc);  
	}

	if (pwrite(fd, buf, erasesize, block_offset) != erasesize) {
		fprintf(stderr, "Error writing block (%s)\n", strerror(errno));
		exit(1);
	}

	if (quiet < 2)
		fprintf(stderr, "Done.\n");

	close (fd);
	sync();
	return 0;

}
