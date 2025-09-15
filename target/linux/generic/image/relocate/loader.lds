OUTPUT_ARCH(mips)
SECTIONS {
	.text : {
		_code_start = .;
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		*(.data)
		*(.data.*)
	}

	. = ALIGN(32);

	_code_end = .;
}
