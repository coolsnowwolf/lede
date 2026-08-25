
#ifndef _ARMV7_PMU_H
#define _ARMV7_PMU_H

//
// These are the events listed in section C9.10.1 of ARM DDI 0406B
// The ARM ARM provides a description for each code.  READ THESE
// BEFORE USING THEM!
//

#define V7_PMU_EVENT_SOFTWARE_INC       0x00
#define V7_PMU_EVENT_ICACHE_MISS        0x01
#define V7_PMU_EVENT_INSTR_TLB_MISS     0x02
#define V7_PMU_EVENT_DCACHE_LINE_FILL   0x03
#define V7_PMU_EVENT_DCACHE_MISS        0x04
#define V7_PMU_EVENT_DATA_TLB_MISS      0x05
#define V7_PMU_EVENT_MEM_READ           0x06
#define V7_PMU_EVENT_MEM_WRITE          0x07
#define V7_PMU_EVENT_INSTR_EXECUTED     0x08
#define V7_PMU_EVENT_EXCEPTION          0x09
#define V7_PMU_EVENT_EXCEPTION_RETURN   0x0A
#define V7_PMU_EVENT_CONTEXTID_CHANGE   0x0B
#define V7_PMU_EVENT_WRITE_TO_PC        0x0C
#define V7_PMU_EVENT_BRANCH_IMM         0x0D
#define V7_PMU_EVENT_BRANCH_DIRECT      0x0E
#define V7_PMU_EVENT_UNALIGNED_ACCESS   0x0F
#define V7_PMU_EVENT_BRANCH_MISPREDICT  0x10
#define V7_PMU_EVENT_CYCLE_COUNT        0x11
#define V7_PMU_EVENT_PREDICtABLE_BRANCH 0x13

#define V7_PMU_EVENT_FILTER_P           (1 << 31)
#define V7_PMU_EVENT_FILTER_U           (1 << 30)
#define V7_PMU_EVENT_FILTER_NSK         (1 << 29)
#define V7_PMU_EVENT_FILTER_NSU         (1 << 28)
#define V7_PMU_EVENT_FILTER_NSH         (1 << 27)
#define V7_PMU_EVENT_FILTER_ALL_PL      (1 << 27)
#define V7_PMU_EVENT_FILTER_NONE        (0)

// NOTE: USE 'V7_PMU_EVENT_FILTER_NONE' ON ANY PROCESSOR IMPLEMENTING PMUv1

#define V7_PMU_COUNTER0                  0
#define V7_PMU_COUNTER1                  1
#define V7_PMU_COUNTER2                  2
#define V7_PMU_COUNTER3                  3
#define V7_PMU_COUNTER4                  4
#define V7_PMU_COUNTER5                  5
#define V7_PMU_COUNTER6                  6
#define V7_PMU_COUNTEr7                  7
#define V7_PMU_COUNTER8                  8
#define V7_PMU_COUNTER9                  9
#define V7_PMU_COUNTER10                 10
#define V7_PMU_COUNTER11                 11
#define V7_PMU_COUNTER12                 12
#define V7_PMU_COUNTER13                 13
#define V7_PMU_COUNTER14                 14
#define V7_PMU_COUNTEr15                 15
#define V7_PMU_CCNT                      31

#ifdef TCSUPPORT_CPU_ARMV8
/* backward compatible with mips code */
#define read_c0_count readCCNT
#endif

// Returns the number of progammable counters
unsigned int getPMN(void);

// Sets the event for a programmable counter to record
// counter = Which counter to program  (e.g. 0 for PMN0, 1 for PMN1)
// event   = The event code (from appropiate TRM)
// filter  = For PMUv2, the filter bits.  Ignored on PMUv1
// NOTE: On processor
void pmnConfig(unsigned int counter, unsigned int event, unsigned filter);

// Enables/disables the divider (1/64) on CCNT
// divider = r0 = If 0 disable divider, else enable dvider
void ccntDivider(int divider);


//
// Enables and disables
//


// Global PMU enable
// On ARM11 this enables the PMU, and the counters start immediately
// On Cortex this enables the PMU, there are individual enables for the counters
void enablePMU(void);

// Global PMU disable
// On Cortex, this overrides the enable state of the individual counters
void disablePMU(void);

// Enable the CCNT
void enableCCNT(void);

// Disable the CCNT
void disableCCNT(void);

// Enable PMN{n}
// counter = The counter to enable (e.g. 0 for PMN0, 1 for PMN1)
void enablePMN(unsigned int counter);

// Enable PMN{n}
// counter = The counter to enable (e.g. 0 for PMN0, 1 for PMN1)
void disablePMN(unsigned int counter);


//
// Read counter values
//


// Returns the value of CCNT
unsigned int readCCNT(void);

// Writes CCNT
void writeCCNT(unsigned int);

// Returns the value of PMN{n}
// counter = The counter to read (e.g. 0 for PMN0, 1 for PMN1)
unsigned int readPMN(unsigned int counter);


//
// Overflow and interrupts
//


// Returns the value of the overflow flags
unsigned int readFlags(void);

// Writes the overflow flags
void writeFlags(unsigned int flags);

// Enables interrupt generation on overflow of the CCNT
void enableCCNTIrq(void);

// Disables interrupt generation on overflow of the CCNT
void disableCCNTIrq(void);

// Enables interrupt generation on overflow of PMN{x}
// counter = The counter to enable the interrupt for (e.g. 0 for PMN0, 1 for PMN1)
void enablePMNIrq(unsigned int counter);

// Disables interrupt generation on overflow of PMN{x}
// counter = r0 =  The counter to disable the interrupt for (e.g. 0 for PMN0, 1 for PMN1)
void disablePMNIrq(unsigned int counter);


//
// Counter reset functions
//


// Resets the programmable counters
void resetPMN(void);

// Resets the CCNT
void resetCCNT(void);


//
// Software Increment
//

         
// Writes to software increment register
// counter = The counter to increment (e.g. 0 for PMN0, 1 for PMN1)
void pmuSoftwareIncrement(unsigned int counter);


//
// User mode access
//


// Enables User mode access to the PMU (must be called in a priviledged mode)
void enablePMUUserAccess(void);

// Disables User mode access to the PMU (must be called in a priviledged mode)
void disablePMUserAccess(void);

#ifdef TCSUPPORT_CPU_ARMV8
/****************************************************************************
 **** Performance Index measurement *****************************************
 ****************************************************************************/

#define _measure_read_base_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
    asm volatile(   \
         "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
         "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
         : "=r" (_cnt1), "=r" (_cnt2)  \
     ); \
})
    
#define _measure_read_by_addr(_cnt1,_cnt2,_addr)				\
({                              \
    unsigned long _tmp;                 \
    asm volatile(   \
         "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
         "ldr     %2, [%3]\n\t"   \
         "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
         : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp) \
         : "r" (_addr) \
     ); \
})

#define _measure_write_base_by_addr(_cnt1,_cnt2,_addr)				\
({								\
    asm volatile(   \
         "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
         "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
         : "=r" (_cnt1), "=r" (_cnt2)  \
     ); \
})

#define _measure_write_by_addr(_cnt1,_cnt2,_addr)				\
({								\
	unsigned long _tmp;					\
    asm volatile(   \
         "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
         "str     %2, [%3]\n\t"   \
         "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
         : "=r" (_cnt1), "=r" (_cnt2), "=r" (_tmp) \
         : "r" (_addr) \
     ); \
})

#define _measure_sync(_cnt1,_cnt2)				\
({								\
    asm volatile(   \
        "dsb st\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "dsb st\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : : "memory" \
     ); \
})

#define _measure_dsb(_cnt1,_cnt2)				\
({								\
    asm volatile(   \
        "dsb\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "dsb\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : : "memory" \
     ); \
})

#define _measure_dsbst(_cnt1,_cnt2)				\
({								\
    asm volatile(   \
        "dsb st\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "dsb st\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : : "memory" \
     ); \
})

#define _measure_dmbosh(_cnt1,_cnt2)				\
({								\
    asm volatile(   \
        "dmb osh\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "dmb osh\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : : "memory" \
     ); \
})

#define _measure_dmboshst(_cnt1,_cnt2)				\
({								\
    asm volatile(   \
        "dmb oshst\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "dmb oshst\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : : "memory" \
     ); \
})


#define _dcache_inv_by_addr_to_PoC(_addr)				\
({								\
    asm volatile(   \
         "mcr	p15, 0, %0, c7, c6, 1\n\t"    \
         "dsb st\n\t"    \
         :              \
         : "r" (_addr)  \
     ); \
})

#define _measure_dcache_inv_by_addr_to_PoC(_cnt1,_cnt2,_addr)				\
({                              \
    asm volatile(   \
        "dsb st\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "mcr	 p15, 0, %2, c7, c6, 1\n\t"    \
        "dsb st\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : "r" (_addr)  \
    ); \
})

#define _measure_dcache_wback_inv_by_addr_to_PoC(_cnt1,_cnt2,_addr)				\
({                              \
    __asm__ __volatile__(   \
        "dsb st\n\t"    \
        "MRC     p15, 0, %0, c9, c13, 0\n\t"    \
        "mcr	 p15, 0, %2, c7, c14, 1\n\t"    \
        "dsb st\n\t"    \
        "MRC     p15, 0, %1, c9, c13, 0\n\t"    \
        : "=r" (_cnt1), "=r" (_cnt2)  \
        : "r" (_addr)  \
    ); \
})

#endif

#endif

