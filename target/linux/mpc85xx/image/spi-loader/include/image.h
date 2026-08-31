// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Derived from U-Boot include/image.h:
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#pragma once

#include <types.h>

/*
 * Compression Types
 *
 * The following are exposed to uImage header.
 * New IDs *MUST* be appended at the end of the list and *NEVER*
 * inserted for backward compatibility.
 */
enum {
	IH_COMP_NONE		= 0,	/*  No	 Compression Used	*/
	IH_COMP_GZIP,			/* gzip	 Compression Used	*/
	IH_COMP_BZIP2,			/* bzip2 Compression Used	*/
	IH_COMP_LZMA,			/* lzma  Compression Used	*/
	IH_COMP_LZO,			/* lzo   Compression Used	*/
	IH_COMP_LZ4,			/* lz4   Compression Used	*/
	IH_COMP_ZSTD,			/* zstd   Compression Used	*/

	IH_COMP_COUNT,
};

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_MAGIC_OKLI	0x4f4b4c49	/* 'OKLI' Magic Number */
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;
