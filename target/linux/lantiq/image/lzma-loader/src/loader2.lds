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
