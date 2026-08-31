OUTPUT_ARCH(powerpc:common)
ENTRY(_start)
EXTERN(_start)
SECTIONS {
	.text :
	{
		_text_start = .;
		*(.text*)
		_text_len = ABSOLUTE(. - _text_start);
	}
	.data :
	{
		_data_start = .;
		*(.rodata*)
		*(.data*)
		*(.sdata*)
		_data_len = ABSOLUTE(. - _data_start);
	}
	. = ALIGN(4096);
	.bss :
	{
		_bss_start = .;
		. += 4K;
		_stack_top = .;
		*(.bss*)
		*(.sbss*)
		_bss_len = ABSOLUTE(. - _bss_start);
	}
	. = ALIGN(4096);
	_end = .;
}
