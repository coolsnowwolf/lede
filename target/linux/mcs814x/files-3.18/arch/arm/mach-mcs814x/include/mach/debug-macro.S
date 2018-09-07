#include <mach/mcs814x.h>

                .macro  addruart, rp, rv, tmp
		ldr	\rp, =MCS814X_PHYS_BASE
		ldr	\rv, =MCS814X_VIRT_BASE
		orr	\rp, \rp, #MCS814X_UART
		orr	\rv, \rv, #MCS814X_UART
                .endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
