/* SPDX-License-Identifier: GPL-2.0-or-later */

OUTPUT_ARCH(mips)
SECTIONS {
	.data.lzma : {
		_lzma_data_start = .;
		*(.data)
		_lzma_data_end = .;
	}
}
