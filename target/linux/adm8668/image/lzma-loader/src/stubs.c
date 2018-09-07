#include <exports.h>

#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 1000 + __GNUC_MINOR__)
#endif /* GCC_VERSION */

/*
 * k0 ($26) holds the pointer to the global_data; t9 ($25) is a call-
 * clobbered register that is also used to set gp ($26). Note that the
 * jr instruction also executes the instruction immediately following
 * it; however, GCC/mips generates an additional `nop' after each asm
 * statement
 */
#define EXPORT_FUNC(x) \
	asm volatile (			\
"	.globl " #x "\n"		\
#x ":\n"				\
"	lw	$25, %0($26)\n"		\
"	lw	$25, %1($25)\n"		\
"	jr	$25\n"			\
	: : "i"(offsetof(gd_t, jt)), "i"(XF_ ## x * sizeof(void *)) : "t9");

/* This function is necessary to prevent the compiler from
 * generating prologue/epilogue, preparing stack frame etc.
 * The stub functions are special, they do not use the stack
 * frame passed to them, but pass it intact to the actual
 * implementation. On the other hand, asm() statements with
 * arguments can be used only inside the functions (gcc limitation)
 */
#if GCC_VERSION < 3004
static
#endif /* GCC_VERSION */
void __attribute__((unused)) dummy(void)
{
#include <_exports.h>
}

#if 0
extern unsigned long __bss_start, _end;

void app_startup(char **argv)
{
	unsigned long * cp = &__bss_start;

	/* Zero out BSS */
	while (cp < &_end) {
		*cp++ = 0;
	}
}
#endif

#undef EXPORT_FUNC
