#ifndef __ECNT_PROFILING_H
#define __ECNT_PROFILING_H

#ifdef TCSUPPORT_CPU_ARMV8
#ifdef TCSUPPORT_CPU_ARMV8_64
#include <asm/tc3162/arm64_pmu.h>
#else
#include <asm/tc3162/arm_v7_pmu.h>
#endif

#else /* ! TCSUPPORT_CPU_ARMV8 */

#define _measure_read_base_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "addiu   %2, 1\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	 ); \
})

#define _measure_read_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "lw      %2, 0(%2)\n\t"   \
		 "addiu   %2, 1\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})


#define _measure_write_base_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})

#define _measure_write_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned int _tmp;					\
	__asm__ __volatile__(   \
		 "mfc0    %0, $9, 0\n\t"    \
		 "la      %2, %3\n\t"    \
		 "sw      %2, 0(%2)\n\t"   \
		 "mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp)  \
		 : "i" (_addr) \
	); \
})

#define _measure_base(_cnt1,_cnt2)				\
({								\
	__asm__ __volatile__(   \
		"mfc0    %0, $9, 0\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		 : "=r" (_cnt1), "=r" (_cnt2)  \
	); \
})

#define _measure_sync(_cnt1,_cnt2)				\
({								\
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
	); \
})

#define _Dcache_inv_by_addr(_addr)				\
({								\
	__asm__ __volatile__(   \
		 "cache   0x11, %0\n\t"    \
		 "sync\n\t"    \
		 :              \
		 : "i" (_addr)  \
	); \
})

#define _L2cache_inv_by_addr(_addr)				\
({                              \
	__asm__ __volatile__(   \
		"cache   0x13, %0\n\t"    \
		"sync\n\t"    \
		:               \
		: "i" (_addr)  \
	); \
})

#define _Dcache_wback_inv_by_addr(_addr)				\
({                              \
	__asm__ __volatile__(   \
		"cache   0x15, %0\n\t"    \
		"sync\n\t"    \
		:               \
		: "i" (_addr)  \
	); \
})

#define _measure_Dcache_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x11, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_L2cache_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x13, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_Dcache_wback_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x15, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})

#define _measure_L2cache_wback_inv_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
	__asm__ __volatile__(   \
		"sync\n\t"    \
		"mfc0    %0, $9, 0\n\t"    \
		"cache   0x17, %2\n\t"    \
		"sync\n\t"    \
		"mfc0    %1, $9, 0\n\t"    \
		: "=r" (_cnt1), "=r" (_cnt2)  \
		: "i" (_addr)  \
	); \
})
#endif /* TCSUPPORT_CPU_ARMV8 */

#endif

