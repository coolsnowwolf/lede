/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#ifndef _U_BOOT_H_
#define _U_BOOT_H_	1

typedef struct bd_info {
	int		bi_baudrate;	/* serial console baudrate */
	unsigned long	bi_ip_addr;	/* IP Address */
	unsigned char	bi_enetaddr[6];	/* Ethernet adress */
	unsigned long	bi_arch_number;	/* unique id for this board */
	unsigned long	bi_boot_params;	/* where this board expects params */
	unsigned long	bi_memstart;	/* start of DRAM memory */
	unsigned long	bi_memsize;	/* size	 of DRAM memory in bytes */
	unsigned long	bi_flashstart;	/* start of FLASH memory */
	unsigned long	bi_flashsize;	/* size  of FLASH memory */
	unsigned long	bi_flashoffset;	/* reserved area for startup monitor */
} bd_t;

struct global_data {
	bd_t		*bd;            /* board data... */
	unsigned long	flags;
	unsigned long	baudrate;
	unsigned long	have_console;   /* serial_init() was called */
	unsigned long	ram_size;       /* RAM size */
	unsigned long	reloc_off;      /* Relocation Offset */
	unsigned long	env_addr;       /* Address  of Environment struct */
	unsigned long	env_valid;      /* Checksum of Environment valid? */
	void		**jt;           /* jump table */
};

#endif	/* _U_BOOT_H_ */
