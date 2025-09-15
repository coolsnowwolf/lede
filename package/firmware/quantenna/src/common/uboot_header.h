/*
 *  Copyright (c) 2015 Quantenna Communications, Inc.
 *  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
 * This code is taken from u-boot/include/image.h file
 */
#ifndef UBOOT_HEADER_H
#define UBOOT_HEADER_H

#ifndef __ASSEMBLY__
#define IH_MAGIC      0x27051956      /* Image Magic Number           */
#define IH_NMLEN              32      /* Image Name Length            */

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	uint32_t        ih_magic;       /* Image Header Magic Number    */
	uint32_t        ih_hcrc;        /* Image Header CRC Checksum    */
	uint32_t        ih_time;        /* Image Creation Timestamp     */
	uint32_t        ih_size;        /* Image Data Size              */
	uint32_t        ih_load;        /* Data  Load  Address          */
	uint32_t        ih_ep;          /* Entry Point Address          */
	uint32_t        ih_dcrc;        /* Image Data CRC Checksum      */
	uint8_t         ih_os;          /* Operating System             */
	uint8_t         ih_arch;        /* CPU architecture             */
	uint8_t         ih_type;        /* Image Type                   */
	uint8_t         ih_comp;        /* Compression Type             */
	uint8_t         ih_name[IH_NMLEN];      /* Image Name           */
} image_header_t;

static inline uint32_t image_get_header_size(void)
{
#define MAX_KNOWN_PAGE_SIZE 8192
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))
	return ROUND_UP(sizeof(image_header_t), MAX_KNOWN_PAGE_SIZE);
}

struct early_flash_config {
	uint32_t	method;
	uint32_t	ipaddr;
	uint32_t	serverip;
	uint8_t		built_time_utc_sec[11];
	uint8_t		uboot_type;
} __attribute__ ((packed));
#endif /* __ASSEMBLY__ */

#define RUBY_BOOT_METHOD_TRYLOOP        0
#define RUBY_BOOT_METHOD_TFTP           1
#define RUBY_BOOT_METHOD_BOOTP          2
#define RUBY_BOOT_METHOD_MAX            3

#endif /* UBOOT_HEADER_H */

