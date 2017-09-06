OUTPUT_ARCH(mips)
SECTIONS {
	.text : {
		_code_start = .;
		*(.text)
		*(.text.*)
		*(.rodata)
		*(.rodata.*)
		. = ALIGN(16);
		*(.data.lzma)
	}

	.data : {
		*(.data)
		*(.data.*)
	}
	_code_end = .;

	.bss : {
		*(.bss)
		*(.bss.*)
	}

	. = ALIGN(16);
	. = . + 8192;
	_stack = .;

	workspace = .;
}
