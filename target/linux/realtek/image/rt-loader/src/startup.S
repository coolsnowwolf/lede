# rt-loader assembler startup code
# (c) 2025 Markus Stockhausen

#include "globals.h"

# This start code allows to run a position independent code (PIC) on bare metal. In that case
# all addresses are looked up via the global offset table (GOT). That must be filled during
# this initialization sequence. Without a proper GOT using standard "la" instruction in the
# code will not work. Provide a macro that avoids the dependency.

.macro _LA reg, symbol
	lui \reg, %hi(\symbol)
	addi \reg, \reg, %lo(\symbol)
	add \reg, $t9
.endm

.macro  _EHB
	sll     $zero, 3
.endm

	.section .text
	.globl _start
	.ent _start
_start:
	.set noreorder

# Determine current program load address and store it into t9.

	bal	_where_am_i
	nop
_where_am_i:
	move	$t9, $ra
	subu	$t9, $t9, 0x8

# This loader might be run in environments that are not properly initialized - e.g. ZyXEL
# devices with BootBase loader. Be careful and setup the coprocessor registers.

	mtc0	$zero, CP0_WATCHLO
	mtc0	$zero, CP0_WATCHHI
	mtc0	$zero, CP0_CAUSE
	mfc0	$t0, CP0_STATUS
	li	$t1, 0x1000001f
	or	$t0, $t1
	xori	$t0, 0x1f
	mtc0	$t0, CP0_STATUS
	_EHB
	mtc0	$zero, CP0_COUNT
	mtc0	$zero, CP0_COMPARE
	_EHB

# Increase run counter and check if this is the first run

	_LA	$t6, _my_run_count
	lw	$t7, 0($t6)
	addi	$t8, $t7, 1
	sw	$t8, 0($t6)
	bne	$zero, $t7, _init_done
	nop

# During first run store the current load address as the target kernel load address.

	_LA     $t6, _kernel_load_addr
	sw	$t9, 0($t6)

# Same for the global variables in the BSS section. Clear them only during the first run. This
# way the "global program state" can be copied over to the relocation address.

	_LA	$t3, __bss_start
	_LA	$t4, __bss_end
_bss_zero:
	beq	$t3, $t4, _init_done
	nop
	sw	$zero, 0($t3)
	addiu	$t3, $t3, 4
	b	_bss_zero
	nop

_init_done:

# Code is running bare metal and no one initializes the global offset table. After the build
# process the table is relative to address 0x0. Starting from anywhere else breaks the program.
# A manual update is required during startup. Usually this is quite easy by simply adding the
# current load address to all entries.
# But this code relocates itself to another memory address and starts itself over. At the new
# address it will find a global offset table that fits to the previous execution. To solve this
# store a copy of the last load address in got_delta variable and only add the difference after
# a relocation. Sequence is as follows
#
# - U-Boot loads the code to 0x80100000
# - U-Boot runs the code at 0x80100000
# - code identifies its dynamic start_address = 0x80100000
# - code reads (initial) _got_delta = 0x00000000
# - code adds 0x80100000 to all GOT entries
# - code stores _got_delta with 0x80100000
# - code copies itself over to a new location 0x85000000
# - code starts itself from 0x85000000
# - code identifies its dynamic start_address = 0x85000000
# - code reads (pre-filled) _got_delta = 0x80100000
# - code adds 0x4f00000 (= 0x85000000 - 0x80100000) to all GOT entries
# - ...
#

	_LA	$t6, _got_delta
	lw	$t5, 0($t6)
	subu    $t7, $t9, $t5
	sw	$t9, 0($t6)
	_LA	$t3, __got_start
	_LA	$t4, __got_end
_got_patch:
	beq	$t3, $t4, _got_done
	nop
	lw	$t5, 0($t3)
	addu	$t5, $t5, $t7
	sw	$t5, 0($t3)
	addiu	$t3, $t3, 4
	b	_got_patch
	nop
_got_done:

# Linker attached kernel to end of package. Store addresses in global variables

	_LA	$t8, _my_load_addr
	sw	$t9, 0($t8)

	_LA	$t5, __kernel_data_start
	_LA	$t4, _kernel_data_addr
	sw	$t5, 0($t4)

	_LA	$t3, __kernel_data_end
	subu	$t3, $t3, $t5
	_LA	$t4, _kernel_data_size
	sw	$t3, 0($t4)

# Determine own code size by looking where BSS ends.

	_LA	$t3, __bss_end
	subu	$t6, $t3, $t9
	_LA	$t4, _my_load_size
	sw	$t6, 0($t4)

# Setup heap. It will start directly behind BSS

	addiu	$t3, MEMORY_ALIGNMENT
	li	$t4, ~(MEMORY_ALIGNMENT - 1)
	and	$t3, $t4

	_LA	$t5, _heap_addr
	sw	$t3, 0($t5)

	li	$t4, HEAP_SIZE
	add	$t3, $t4

	_LA	$t5, _heap_addr_max
	sw	$t3, 0($t5)

# Setup stack that is located on top of heap.

	li	$t4, STACK_SIZE
	add	$sp, $t3, $t4

# Adapt t9 so it points to main(). This is needed so main() can find the GOT via t9/gp

	_LA     $t8, main
	move	$t9, $t8

# Call main() with parameters a0, a3, __kernel_start, __kernel_end
	bal	main
	nop

	.end _start

	.section .data
	.align 4
# delta for global offset table initialization
_got_delta:
	.word 0
# current heap address for malloc() / free()
	.globl _heap_addr
_heap_addr:
	.word 0
# maximum heap address
	.globl _heap_addr_max
_heap_addr_max:
	.word 0
# number of loads
	.globl _my_run_count
_my_run_count:
	.word 0
# current program load address
	.globl _my_load_addr
_my_load_addr:
	.word 0
# total size of code including attached kernel and bss (uninitialized global variables)
	.globl _my_load_size
_my_load_size:
	.word 0
# target load address of kernel = this programs address during initial run
	.globl _kernel_load_addr
_kernel_load_addr:
	.word 0
# absolute start address of attached kernel
	.globl _kernel_data_addr
_kernel_data_addr:
	.word 0
# size of attached kernel
	.globl _kernel_data_size
_kernel_data_size:
	.word 0
# compression method of attached kernel (usually LZMA)
	.globl _kernel_comp_type
_kernel_comp_type:
	.word UIMAGE_COMP_LZMA
