#ifndef __ASM_ARCH_CPU_H__
#define __ASM_ARCH_CPU_H__

#include <asm/cputype.h>

#define MCS8140_ID	0x41069260	/* ARM926EJ-S */
#define MCS814X_MASK	0xff0ffff0

#ifdef CONFIG_MCS8140
/* Moschip MCS8140 is based on an ARM926EJ-S core */
#define soc_is_mcs8140()	((read_cpuid_id() & MCS814X_MASK) == MCS8140_ID)
#else
#define soc_is_mcs8140()	(0)
#endif /* !CONFIG_MCS8140 */

#endif /* __ASM_ARCH_CPU_H__ */
