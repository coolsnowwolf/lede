/* SPDX-License-Identifier: GPL-2.0-or-later */

OUTPUT_ARCH(mips)
SECTIONS {
	.text : {
		_code_start = .;
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		*(.data.lzma)
	}

	. = ALIGN(32);
	.data : {
		*(.data)
		*(.data.*)
	}

	. = ALIGN(32);
	_code_end = .;

	_bss_start = .;
	.bss : {
		*(.bss)
		*(.bss.*)
	}

	. = ALIGN(32);
	_bss_end = .;

	. = . + 8192;
	_stack = .;

	workspace = .;
}
