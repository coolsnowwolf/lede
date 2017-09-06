/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2008 Axel Gembe <ago@bastart.eu.org>
 * Copyright (C) 2009-2010 Daniel Dickinson <openwrt@cshore.neomailbox.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <inttypes.h>

#include "bcm_tag.h"
#include "imagetag_cmdline.h"
#include "cyg_crc.h"

#define DEADCODE			0xDEADC0DE

/* Kernel header */
struct kernelhdr {
	uint32_t		loadaddr;	/* Kernel load address */
	uint32_t		entry;		/* Kernel entry point address */
	uint32_t		lzmalen;	/* Compressed length of the LZMA data that follows */
};

static char pirellitab[NUM_PIRELLI][BOARDID_LEN] = PIRELLI_BOARDS;

void int2tag(char *tag, uint32_t value) {
  uint32_t network = htonl(value);
  memcpy(tag, (char *)(&network), 4);
}

uint32_t compute_crc32(uint32_t crc, FILE *binfile, size_t compute_start, size_t compute_len)
{
	uint8_t readbuf[1024];
	size_t read;

	fseek(binfile, compute_start, SEEK_SET);

	/* read block of 1024 bytes */
	while (binfile && !feof(binfile) && !ferror(binfile) && (compute_len >= sizeof(readbuf))) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), binfile);
		crc = cyg_crc32_accumulate(crc, readbuf, read);
		compute_len = compute_len - read;
	}

	/* Less than 1024 bytes remains, read compute_len bytes */
	if (binfile && !feof(binfile) && !ferror(binfile) && (compute_len > 0)) {
		read = fread(readbuf, sizeof(uint8_t), compute_len, binfile);
		crc = cyg_crc32_accumulate(crc, readbuf, read);
	}

	return crc;
}

size_t getlen(FILE *fp)
{
	size_t retval, curpos;

	if (!fp)
		return 0;

	curpos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	retval = ftell(fp);
	fseek(fp, curpos, SEEK_SET);

	return retval;
}

int tagfile(const char *kernel, const char *rootfs, const char *bin, \
			const struct gengetopt_args_info *args, \
			uint32_t flash_start, uint32_t image_offset, \
			uint32_t block_size, uint32_t load_address, uint32_t entry)
{
	struct bcm_tag tag;
	struct kernelhdr khdr;
	FILE *kernelfile = NULL, *rootfsfile = NULL, *binfile = NULL, *cfefile = NULL;
	size_t cfeoff, cfelen, kerneloff, kernellen, rootfsoff, rootfslen, \
	  read, imagelen, rootfsoffpadlen = 0, kernelfslen, kerneloffpadlen = 0, oldrootfslen, \
	  rootfsend;
	uint8_t readbuf[1024];
	uint32_t imagecrc = IMAGETAG_CRC_START;
	uint32_t kernelcrc = IMAGETAG_CRC_START;
	uint32_t rootfscrc = IMAGETAG_CRC_START;
	uint32_t kernelfscrc = IMAGETAG_CRC_START;
	uint32_t fwaddr = 0;
	uint8_t crc_val;
	const uint32_t deadcode = htonl(DEADCODE);
	int i;
	int is_pirelli = 0;


	memset(&tag, 0, sizeof(struct bcm_tag));

	if (!kernel || !rootfs) {
		fprintf(stderr, "imagetag can't create an image without both kernel and rootfs\n");
	}

	if (kernel && !(kernelfile = fopen(kernel, "rb"))) {
		fprintf(stderr, "Unable to open kernel \"%s\"\n", kernel);
		return 1;
	}

	if (rootfs && !(rootfsfile = fopen(rootfs, "rb"))) {
		fprintf(stderr, "Unable to open rootfs \"%s\"\n", rootfs);
		return 1;
	}

	if (!bin || !(binfile = fopen(bin, "wb+"))) {
		fprintf(stderr, "Unable to open output file \"%s\"\n", bin);
		return 1;
	}

	if ((args->cfe_given) && (args->cfe_arg)) {
	  if (!(cfefile = fopen(args->cfe_arg, "rb"))) {
		fprintf(stderr, "Unable to open CFE file \"%s\"\n", args->cfe_arg);
	  }
	}

	fwaddr = flash_start + image_offset;
	if (cfefile) {
	  cfeoff = flash_start;		  
	  cfelen = getlen(cfefile);
	  /* Seek to the start of the file after tag */
	  fseek(binfile, sizeof(tag), SEEK_SET);
	  
	  /* Write the cfe */
	  while (cfefile && !feof(cfefile) && !ferror(cfefile)) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), cfefile);
		fwrite(readbuf, sizeof(uint8_t), read, binfile);
	  }

	} else {
	  cfeoff = 0;
	  cfelen = 0;
	}

	if (!args->root_first_flag) {
	  /* Build the kernel address and length (doesn't need to be aligned, read only) */
	  kerneloff = fwaddr + sizeof(tag);
	  
	  kernellen = getlen(kernelfile);
	  
	  if (!args->kernel_file_has_header_flag) {
		/* Build the kernel header */
		khdr.loadaddr	= htonl(load_address);
		khdr.entry	= htonl(entry);
		khdr.lzmalen	= htonl(kernellen);
		
		/* Increase the kernel size by the header size */
		kernellen += sizeof(khdr);	  
	  }
	  
	  /* Build the rootfs address and length */
	  rootfsoff = kerneloff + kernellen;
	  /* align the start if requested */
	  if (args->align_rootfs_flag)
		rootfsoff = (rootfsoff % block_size) > 0 ? (((rootfsoff / block_size) + 1) * block_size) : rootfsoff;
          else
		rootfsoff = (rootfsoff % 4) > 0 ? (((rootfsoff / 4) + 1) * 4) : rootfsoff;

	  /* align the end */
	  rootfsend = rootfsoff + getlen(rootfsfile);
	  if ((rootfsend % block_size) > 0)
		rootfsend = (((rootfsend / block_size) + 1) * block_size);
	  rootfslen = rootfsend - rootfsoff;
	  imagelen = rootfsoff + rootfslen - kerneloff + sizeof(deadcode);
	  rootfsoffpadlen = rootfsoff - (kerneloff + kernellen);
	  
	  /* Seek to the start of the kernel */
	  fseek(binfile, kerneloff - fwaddr + cfelen, SEEK_SET);
	  
	  /* Write the kernel header */
	  fwrite(&khdr, sizeof(khdr), 1, binfile);
	  
	  /* Write the kernel */
	  while (kernelfile && !feof(kernelfile) && !ferror(kernelfile)) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), kernelfile);
		fwrite(readbuf, sizeof(uint8_t), read, binfile);
	  }

	  /* Write the RootFS */
	  fseek(binfile, rootfsoff - fwaddr + cfelen, SEEK_SET);
	  while (rootfsfile && !feof(rootfsfile) && !ferror(rootfsfile)) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), rootfsfile);
		fwrite(readbuf, sizeof(uint8_t), read, binfile);
	  }

	  /* Align image to specified erase block size and append deadc0de */
	  printf("Data alignment to %dk with 'deadc0de' appended\n", block_size/1024);
	  fseek(binfile, rootfsoff + rootfslen - fwaddr + cfelen, SEEK_SET);
	  fwrite(&deadcode, sizeof(uint32_t), 1, binfile);

	  oldrootfslen = rootfslen;
	  if (args->pad_given) {
		uint32_t allfs = 0xffffffff;
		uint32_t pad_size = args->pad_arg * 1024 * 1024;

		printf("Padding image to %d bytes ...\n", pad_size);
		while (imagelen < pad_size) {
			fwrite(&allfs, sizeof(uint32_t), 1, binfile);
			imagelen += 4;
			rootfslen += 4;
		}
	  }

	  /* Flush the binfile buffer so that when we read from file, it contains
	   * everything in the buffer
	   */
	  fflush(binfile);

	  /* Compute the crc32 of the entire image (deadC0de included) */
	  imagecrc = compute_crc32(imagecrc, binfile, kerneloff - fwaddr + cfelen, imagelen);
	  /* Compute the crc32 of the kernel and padding between kernel and rootfs) */
	  kernelcrc = compute_crc32(kernelcrc, binfile, kerneloff - fwaddr + cfelen, kernellen + rootfsoffpadlen);
	  /* Compute the crc32 of the kernel and padding between kernel and rootfs) */
	  kernelfscrc = compute_crc32(kernelfscrc, binfile, kerneloff - fwaddr + cfelen, kernellen + rootfsoffpadlen + rootfslen + sizeof(deadcode));
	  /* Compute the crc32 of the flashImageStart to rootLength.
	   * The broadcom firmware assumes the rootfs starts the image,
	   * therefore uses the rootfs start to determine where to flash
	   * the image.  Since we have the kernel first we have to give
	   * it the kernel address, but the crc uses the length
	   * associated with this address, which is added to the kernel
	   * length to determine the length of image to flash and thus
	   * needs to be rootfs + deadcode
	   */
	  rootfscrc = compute_crc32(rootfscrc, binfile, kerneloff - fwaddr + cfelen, rootfslen + sizeof(deadcode));

	} else {
	  /* Build the kernel address and length (doesn't need to be aligned, read only) */
	  rootfsoff = fwaddr + sizeof(tag);
	  oldrootfslen = getlen(rootfsfile);
	  rootfslen = oldrootfslen;
	  rootfslen = ( (rootfslen % block_size) > 0 ? (((rootfslen / block_size) + 1) * block_size) : rootfslen );
	  kerneloffpadlen = rootfslen - oldrootfslen;
	  oldrootfslen = rootfslen;

	  kerneloff = rootfsoff + rootfslen;
	  kernellen = getlen(kernelfile);

	  imagelen = cfelen + rootfslen + kernellen;
	  
	  /* Seek to the start of the kernel */
	  fseek(binfile, kerneloff - fwaddr + cfelen, SEEK_SET);
	  
	  if (!args->kernel_file_has_header_flag) {
		/* Build the kernel header */
		khdr.loadaddr	= htonl(load_address);
		khdr.entry	= htonl(entry);
		khdr.lzmalen	= htonl(kernellen);
		
		/* Write the kernel header */
		fwrite(&khdr, sizeof(khdr), 1, binfile);
	  
		/* Increase the kernel size by the header size */
		kernellen += sizeof(khdr);	  
	  }
	  
	  /* Write the kernel */
	  while (kernelfile && !feof(kernelfile) && !ferror(kernelfile)) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), kernelfile);
		fwrite(readbuf, sizeof(uint8_t), read, binfile);
	  }

	  /* Write the RootFS */
	  fseek(binfile, rootfsoff - fwaddr + cfelen, SEEK_SET);
	  while (rootfsfile && !feof(rootfsfile) && !ferror(rootfsfile)) {
		read = fread(readbuf, sizeof(uint8_t), sizeof(readbuf), rootfsfile);
		fwrite(readbuf, sizeof(uint8_t), read, binfile);
	  }

	  /* Flush the binfile buffer so that when we read from file, it contains
	   * everything in the buffer
	   */
	  fflush(binfile);

	  /* Compute the crc32 of the entire image (deadC0de included) */
	  imagecrc = compute_crc32(imagecrc, binfile, sizeof(tag), imagelen);
	  /* Compute the crc32 of the kernel and padding between kernel and rootfs) */
	  kernelcrc = compute_crc32(kernelcrc, binfile, kerneloff - fwaddr + cfelen, kernellen + rootfsoffpadlen);
	  kernelfscrc = compute_crc32(kernelfscrc, binfile, rootfsoff - fwaddr + cfelen, kernellen + rootfslen);
	  rootfscrc = compute_crc32(rootfscrc, binfile, rootfsoff - fwaddr + cfelen, rootfslen);
	}

	/* Close the files */
	if (cfefile) {
	  fclose(cfefile);
	}
	fclose(kernelfile);
	fclose(rootfsfile);

	/* Build the tag */
	strncpy(tag.tagVersion, args->tag_version_arg, sizeof(tag.tagVersion) - 1);
	strncpy(tag.sig_1, args->signature_arg, sizeof(tag.sig_1) - 1);
	strncpy(tag.sig_2, args->signature2_arg, sizeof(tag.sig_2) - 1);
	strncpy(tag.chipid, args->chipid_arg, sizeof(tag.chipid) - 1);
	strncpy(tag.boardid, args->boardid_arg, sizeof(tag.boardid) - 1);
	strcpy(tag.big_endian, "1");
	sprintf(tag.totalLength, "%lu", imagelen);

	if (args->cfe_given) {
	  sprintf(tag.cfeAddress, "%" PRIu32, flash_start);
	  sprintf(tag.cfeLength, "%lu", cfelen);
	} else {
	  /* We don't include CFE */
	  strcpy(tag.cfeAddress, "0");
	  strcpy(tag.cfeLength, "0");
	}

	sprintf(tag.kernelAddress, "%lu", kerneloff);
	sprintf(tag.kernelLength, "%lu", kernellen + rootfsoffpadlen);

	if (args->root_first_flag) {
	  sprintf(tag.flashImageStart, "%lu", rootfsoff);
	  sprintf(tag.flashRootLength, "%lu", rootfslen);	  
	} else {
	  sprintf(tag.flashImageStart, "%lu", kerneloff);
	  sprintf(tag.flashRootLength, "%lu", rootfslen + sizeof(deadcode));
	}
	int2tag(tag.rootLength, oldrootfslen + sizeof(deadcode));

	if (args->rsa_signature_given) {
	    strncpy(tag.rsa_signature, args->rsa_signature_arg, RSASIG_LEN);
	}

	if (args->layoutver_given) {
	    strncpy(tag.flashLayoutVer, args->layoutver_arg, TAGLAYOUT_LEN);
	}

	if (args->info1_given) {
	  strncpy(tag.information1, args->info1_arg, TAGINFO1_LEN);
	}

	if (args->info2_given) {
	  strncpy(tag.information2, args->info2_arg, TAGINFO2_LEN);
	}

	if (args->reserved2_given) {
	  strncpy(tag.reserved2, args->reserved2_arg, 16);
	}

	if (args->altinfo_given) {
	  strncpy(tag.information1, args->altinfo_arg, TAGINFO1_LEN);
	}

	if (args->second_image_flag_given) {
	  if (strncmp(args->second_image_flag_arg, "2", DUALFLAG_LEN) != 0) {		
		strncpy(tag.dualImage, args->second_image_flag_arg, DUALFLAG_LEN);
	  }
	}

	if (args->inactive_given) {
	  if (strncmp(args->inactive_arg, "2", INACTIVEFLAG_LEN) != 0) {		
		strncpy(tag.inactiveFlag, args->second_image_flag_arg, INACTIVEFLAG_LEN);
	  }
	}

	for (i = 0; i < NUM_PIRELLI; i++) {
		if (strncmp(args->boardid_arg, pirellitab[i], BOARDID_LEN) == 0) {
			is_pirelli = 1;
			break;
		}
	}

	if ( !is_pirelli ) {
	  int2tag(tag.imageCRC, kernelfscrc);
	} else {
	  int2tag(tag.imageCRC, kernelcrc);
	}

	int2tag(&(tag.rootfsCRC[0]), rootfscrc);
	int2tag(tag.kernelCRC, kernelcrc);
	int2tag(tag.fskernelCRC, kernelfscrc);
	int2tag(tag.headerCRC, cyg_crc32_accumulate(IMAGETAG_CRC_START, (uint8_t*)&tag, sizeof(tag) - 20));

	fseek(binfile, 0L, SEEK_SET);
	fwrite(&tag, sizeof(uint8_t), sizeof(tag), binfile);

    fflush(binfile);
	fclose(binfile);

	return 0;
}

int main(int argc, char **argv)
{
    int c, i;
	char *kernel, *rootfs, *bin;
	uint32_t flash_start, image_offset, block_size, load_address, entry;
	flash_start = image_offset = block_size = load_address = entry = 0;
	struct gengetopt_args_info parsed_args;

	kernel = rootfs = bin = NULL;

	if (imagetag_cmdline(argc, argv, &parsed_args)) {
	  exit(1);
	}

	printf("Broadcom 63xx image tagger - v2.0.0\n");
	printf("Copyright (C) 2008 Axel Gembe\n");
	printf("Copyright (C) 2009-2010 Daniel Dickinson\n");
	printf("Licensed under the terms of the Gnu General Public License\n");

	kernel = parsed_args.kernel_arg;
	rootfs = parsed_args.rootfs_arg;
	bin = parsed_args.output_arg;
	if (strlen(parsed_args.tag_version_arg) >= TAGVER_LEN) {
	  fprintf(stderr, "Error: Tag Version (tag_version,v) too long.\n");
	  exit(1);
	}
	if (strlen(parsed_args.boardid_arg) >= BOARDID_LEN) {
	  fprintf(stderr, "Error: Board ID (boardid,b) too long.\n");
	  exit(1);
	}
	if (strlen(parsed_args.chipid_arg) >= CHIPID_LEN) {
	  fprintf(stderr, "Error: Chip ID (chipid,c) too long.\n");
	  exit(1);
	}
	if (strlen(parsed_args.signature_arg) >= SIG1_LEN) {
	  fprintf(stderr, "Error: Magic string (signature,a) too long.\n");
	  exit(1);
	}
	if (strlen(parsed_args.signature2_arg) >= SIG2_LEN) {
	  fprintf(stderr, "Error: Second magic string (signature2,m) too long.\n");
	  exit(1);
	}
	if (parsed_args.layoutver_given) {
	  if (strlen(parsed_args.layoutver_arg) > FLASHLAYOUTVER_LEN) {
		fprintf(stderr, "Error: Flash layout version (layoutver,y) too long.\n");
		exit(1);
	  }
	}
	if (parsed_args.rsa_signature_given) {
	  if (strlen(parsed_args.rsa_signature_arg) > RSASIG_LEN) {
		fprintf(stderr, "Error: RSA Signature (rsa_signature,r) too long.\n");
		exit(1);
	  }
	}

	if (parsed_args.info1_given) {
	  if (strlen(parsed_args.info1_arg) >= TAGINFO1_LEN) {
		fprintf(stderr, "Error: Vendor Information 1 (info1) too long.\n");
		exit(1);
	  }
	}

	if (parsed_args.info2_given) {
	  if (strlen(parsed_args.info2_arg) >= TAGINFO2_LEN) {
		fprintf(stderr, "Error: Vendor Information 2 (info2) too long.\n");
		exit(1);
	  }
	}

	if (parsed_args.altinfo_given) {
	  if (strlen(parsed_args.altinfo_arg) >= ALTTAGINFO_LEN) {
		fprintf(stderr, "Error: Vendor Information 1 (info1) too long.\n");
		exit(1);
	  }
	}

	if (parsed_args.pad_given) {
	  if (parsed_args.pad_arg < 0) {
		fprintf(stderr, "Error: pad size must be positive.\r");
		exit(1);
	  }
	}

	flash_start = strtoul(parsed_args.flash_start_arg, NULL, 16);
	image_offset = strtoul(parsed_args.image_offset_arg, NULL, 16);
	block_size = strtoul(parsed_args.block_size_arg, NULL, 16);

	if (!parsed_args.kernel_file_has_header_flag) {
	  load_address = strtoul(parsed_args.load_addr_arg, NULL, 16);
	  entry = strtoul(parsed_args.entry_arg, NULL, 16);
	  if (load_address == 0) {
		fprintf(stderr, "Error: Invalid value for load address\n");
	  }
	  if (entry == 0) {
		fprintf(stderr, "Error: Invalid value for entry\n");
	  }
	}
	
	return tagfile(kernel, rootfs, bin, &parsed_args, flash_start, image_offset, block_size, load_address, entry);
}
