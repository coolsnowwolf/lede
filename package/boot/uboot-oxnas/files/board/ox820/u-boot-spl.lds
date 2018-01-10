/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * January 2004 - Changed to support H4 device
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

MEMORY
{
	 sram (rwx) : ORIGIN = CONFIG_SPL_TEXT_BASE, LENGTH = CONFIG_SPL_MAX_SIZE
	 dram : ORIGIN = CONFIG_SPL_BSS_DRAM_START, LENGTH = CONFIG_SPL_BSS_DRAM_SIZE
}

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_spl_start)
SECTIONS
{
	.text.0	:
	{
		*(.init*)
	}


	/* Start of the rest of the SPL */
	code_start = . ;

	.text.1	:
	{
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);

	__image_copy_end = .;
	code_size = . - code_start;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	. = ALIGN(0x800);

	_end = .;

	.bss.sram __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.bss.stdio_devices)
		*(.bss.serial_current)
		 . = ALIGN(4);
		__bss_end = .;
	}

	.bss : {
		__bss_dram_start = .;
		*(.bss*)
		__bss_dram_end = .;
	} > dram

	/DISCARD/ : { *(.bss*) }
	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynsym*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.hash*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
