#ifndef __ARCH_MCS814X_COMMON_H
#define __ARCH_MCS814X_COMMON_H

#include <linux/reboot.h>
#include <asm/mach/time.h>

void mcs814x_map_io(void);
void mcs814x_clk_init(void);
void mcs814x_of_irq_init(void);
void mcs814x_init_machine(void);
void mcs814x_handle_irq(struct pt_regs *regs);
void mcs814x_restart(enum reboot_mode mode, const char *cmd);
void mcs814x_timer_init(void);
extern void __iomem *mcs814x_sysdbg_base;

#endif /* __ARCH_MCS814X_COMMON_H */
