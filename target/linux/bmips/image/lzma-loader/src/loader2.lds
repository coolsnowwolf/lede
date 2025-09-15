/* SPDX-License-Identifier: GPL-2.0-or-later */

OUTPUT_ARCH(mips)
SECTIONS {
	.text : {
		startup = .;
		*(.text)
		*(.text.*)
		*(.data)
		*(.data.*)
	}
}
