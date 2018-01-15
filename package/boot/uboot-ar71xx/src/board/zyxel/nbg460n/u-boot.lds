OUTPUT_FORMAT("elf32-tradbigmips", "elf32-tradbigmips", "elf32-tradbigmips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text       :
	{
	  *(.text)
	}

	. = ALIGN(4);
	.rodata  : { *(.rodata) }

	. = ALIGN(4);
	.data  : { *(.data) }

	. = ALIGN(4);
	.sdata  : { *(.sdata) }

	_gp = ALIGN(16);

	__got_start = .;
	.got  : { *(.got) }
	__got_end = .;

	.sdata  : { *(.sdata) }

	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	__u_boot_cmd_end = .;

	uboot_end_data = .;
	num_got_entries = (__got_end - __got_start) >> 2;

	. = ALIGN(4);
	.sbss  : { *(.sbss) }
	.bss  : { *(.bss) }
	uboot_end = .;
}
