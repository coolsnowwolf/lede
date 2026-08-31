ENTRY(_start)

SECTIONS {
	.text : {
		*(.text)
	}

	.data : ALIGN(32) {
		*(.sdata*)
		*(.data*)
	}
/*
 * In MIPS position independent code (PIC), the global offset table (GOT) is a data structure
 * used to facilitate access to global variables and functions when the code's final memory
 * location is not known at compile time. The GOT contains absolute addresses of global symbols,
 * but is itself located using a relative reference. This allows the code to be relocated at
 * runtime without modification.
 */
	.got : ALIGN(32) {
		__got_start = .;
		*(.got*)
		__got_end = .;
	}
/*
 * Storage for the compressed kernel image that was integrated into the loader during link time.
 * No code just binary data.
 */
	.kernel : ALIGN(1) {
		__kernel_data_start = .;
		KEEP(*(.kernel))
		__kernel_data_end = .;
	}

	.bss (NOLOAD) : ALIGN(4) {
		__bss_start = .;
		*(.bss)
		*(.sbss)
		*(COMMON)
		__bss_end = .;
	}
}
