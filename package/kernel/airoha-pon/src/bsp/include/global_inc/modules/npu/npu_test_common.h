#ifndef _NPU_TEST_COMMON_H
#define _NPU_TEST_COMMON_H

#define S_32      (0x20)
#define S_128     (0x80)
#define S_256     (0x100)
#define S_512     (0x200)
#define S_1K      (0x400)
#define S_2K      (0x800)
#define S_4K      (0x1000)
#define S_8K      (0x2000)
#define S_16K     (0x4000)
#define S_32K     (0x8000)
#define S_40K     (0xa000)
#define S_48K     (0xc000)
#define S_60K     (0xf000)
#define S_64K     (0x10000)
#define S_128K    (0x20000)
#define S_256K    (0x40000)
#define S_384K    (0x60000)
#define S_480K    (0x78000)
#define S_512K    (0x80000)
#define S_1M      (0x100000)
#define S_2M      (0x200000)
#define S_10M     (0xa00000)
#define S_1G      (0x40000000)

/* for 7581/7552 ASIC or 
 * for bitfile with gsw,fe,spi,rbus,gdmp,uart,gpio,timer,scu,hsdma,npu,tzasc,tzpc,sec,pbus,pcie0,pcie1 modules */
#define MULTI_MODULES_SUPPORT

#define WIFI_7916
//#define INTEL_WIFI_CARD

#ifdef TCSUPPORT_NPU_V2
#ifdef TCSUPPORT_NPU_V2_S /*7552*/
#define MAX_CORE_NUM        (2)
#define NPU_SRAM_T_SIZE     (S_64K)
#define L2C_SRAM_SIZE       S_128K
#define MAX_Q_SIZE	        (4)
#elif TCSUPPORT_NPU_V2_P
#define MAX_CORE_NUM        (6)
#define NPU_SRAM_T_SIZE     (S_64K)
#define L2C_SRAM_SIZE       S_128K
#define MAX_Q_SIZE	        (4)
#else /*7581*/
#define MAX_CORE_NUM        (8)
#define NPU_SRAM_T_SIZE     (S_48K)
#define L2C_SRAM_SIZE       S_256K
#define MAX_Q_SIZE	        (8)
#endif
#define CSR_REG_NUM		    (3)
#define CSR_PC_BASE_ADDR    (0x1ec05000)

#else /*7523*/
#define MAX_CORE_NUM        (4)
#define NPU_SRAM_T_SIZE     (S_64K)
#define CSR_REG_NUM		    (12)
#define L2C_SRAM_SIZE       S_128K
#define MAX_Q_SIZE	        (8)
#ifdef TCSUPPORT_CPU_ARMV8
#define CSR_PC_BASE_ADDR	(0x1ec08800)
#else
#define CSR_PC_BASE_ADDR	(0xbec08800)
#endif
#endif

#ifdef TCSUPPORT_NPU_V2_P
#define MBOX_IRQ_TEST_NUM	8
#else
#define MBOX_IRQ_TEST_NUM	MAX_CORE_NUM
#endif
#define MAX_ARRAY_SIZE    (8) 
#define MDELAY_TEST_CNT (1000)
#define ALL_FF  (0xffffffff)

#define NPU_MIB_BASE    (0x1ec0c000)
#define NPU_MIB0        (NPU_MIB_BASE+0x140)
#define NPU_MIB8        (NPU_MIB_BASE+0x160)
#define NPU_MIB9        (NPU_MIB_BASE+0x164)
#define NPU_MIB10       (NPU_MIB_BASE+0x168)
#define NPU_MIB11       (NPU_MIB_BASE+0x16c)
#define NPU_MIB12       (NPU_MIB_BASE+0x170)
#define NPU_MIB13       (NPU_MIB_BASE+0x174)
#define NPU_MIB14       (NPU_MIB_BASE+0x178)
#define NPU_MIB15       (NPU_MIB_BASE+0x17c)
#define NPU_MIB16       (NPU_MIB_BASE+0x180)
#define NPU_MIB17       (NPU_MIB_BASE+0x184)
#define NPU_MIB18       (NPU_MIB_BASE+0x188)
#define NPU_MIB19       (NPU_MIB_BASE+0x18c)
#define NPU_MIB28       (NPU_MIB_BASE+0x1B0)
#define NPU_MIB29       (NPU_MIB_BASE+0x1B4)
#define NPU_MIB30       (NPU_MIB_BASE+0x1B8)
#define NPU_MIB31       (NPU_MIB_BASE+0x1BC)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_NPU_V2_P)
#define PCIE0_MAC_BASE      (0x1fc20000)
#define PCIE1_MAC_BASE      (0x1fa92000)
#define PCIE0_OFF0          (0x0180)
#define PCIE0_OFF1          (0x8030)
#define PCIE0_OFF2          (0x0D00)
#define PCIE0_OFF3          (0x0D04)
#define PCIE0_OFF4          (0x0D08)
#define PCIE0_OFF5          (0x0D0C)
#define PCIE0_OFF6          (0x0D10)
#define PCIE0_OFF7          (0x0D14)
#define PCIE0_OFF8          (0x0D18)
#define PCIE0_OFF9          (0x0D1c)
#define PCIE1_OFF0          (0x404)
#define PCIE1_OFF1          (0x408)
#define PCIE1_OFF2          (0x410)
#define PCIE1_OFF3          (0x414)
#define PCIE1_OFF4          (0x418)
#define PCIE1_OFF5          (0x420)
#define PCIE1_OFF6          (0x464)
#define PCIE1_OFF7          (0x468)
#define PCIE1_OFF8          (0x4e4)
#define PCIE1_OFF9          (0x51c)

#elif defined(TCSUPPORT_CPU_AN7552) || defined(TCSUPPORT_NPU_V2_S)
#define PCIE0_MAC_BASE      (0x1fa91000)
#define PCIE1_MAC_BASE      (0x1fa92000)
#define PCIE_OFF0           (0x404)
#define PCIE_OFF1           (0x414)
#define PCIE_OFF2           (0x464)
#define PCIE_OFF3           (0x468)
#define PCIE_OFF4           (0x4e4)
#define PCIE_OFF5           (0x51c)
#define REG_PCIE0_BASE      (0x1fa91438)
#define PCIE_BASE_OFFSET    (0x1000)

#else /*7581*/
#define PCIE0_MAC_BASE      (0x1fc00000)
#define PCIE1_MAC_BASE      (0x1fc20000)
#define PCIE2_MAC_BASE      (0x1fc40000)
#define PCIE_OFF0           (0x0180)
#define PCIE_OFF1           (0x8030)
#define PCIE_OFF2           (0x0D00)
#define PCIE_OFF3           (0x0D04)
#define PCIE_OFF4           (0x0D08)
#define PCIE_OFF5           (0x0D0C)
#define PCIE_OFF6           (0x0D10)
#define PCIE_OFF7           (0x0D14)
#define PCIE_OFF8           (0x0D18)
#define PCIE_OFF9           (0x0D1c)
#define PCIE_OFF10          (0x8010)
#define PCIE_OFF11          (0x8014)
#define PCIE_OFF12          (0x8018)
#define PCIE_OFF13          (0x801c)
#define REG_PCIE0_BASE      (0x1fc00000)
#define PCIE_CFG0_OFFSET    (0x0140)
#define PCIE_CFG1_OFFSET    (0x1010)
#define PCIE_CFG2_OFFSET    (0x1018)
#define PCIE_CFG3_OFFSET    (0x1020)
#define PCIE_BASE_OFFSET    (0x20000)
#endif
#define MIB_NO_SYNC     (13) /* sync by NPU_MIB13 reg */
#define NO_PCIE_MAGIC   (100)

#if defined(TCSUPPORT_CPU_AN7583) || defined(TCSUPPORT_NPU_V2_P)
#define CR_PCIE0_STATUS         (0x1fc20154)
#define PCIE0_STS_BIT           (8)
#define CR_PCIE1_STATUS         (0x1fa92804)
#define PCIE1_STS_BIT           (10)
#elif defined(TCSUPPORT_CPU_EN7581)
#define CR_PCIE0_STATUS         (0x1fc00154)
#define PCIE_STS_OFFSET         (0x20000)
#define PCIE_STS_BIT            (8)
#else /* 7523, 7552 */
#define CR_PCIE0_STATUS         (0x1fa91804)
#define PCIE_STS_OFFSET         (0x1000)
#define PCIE_STS_BIT            (10)
#endif

enum tCases {
    TEST_NONE=0,
    TEST_THREAD_Q_FULL, /*1*/
    TEST_THREAD_Q_OVERFULL,
    TEST_THREAD_PRIORITY, /*3*/
    TEST_THREAD_JOIN,
    TEST_THREAD_CORE1_MASTER, /*5*/
    TEST_MUTEX_ONE_BLK_TYPE,
    TEST_MUTEX_TWO_BLK_TYPE,
    TEST_MUTEX_ONE_REG_TYPE, /*8*/
    TEST_MUTEX_TWO_REG_TYPE,
    TEST_MUTEX_ONE_BLK_REG_TYPE, /*10*/
    TEST_MUTEX_64,
    TEST_MUTEX_NOT_OWNER_UNLOCK,
    TEST_MUTEX_TRYLOCK, /*13*/
    TEST_MUTEX_SIGNAL,
    TEST_IPC_THREAD_API, /*15*/
    TEST_IPC_MUTEX_API,
    TEST_UNC_ACCESS_RAMS,
    TEST_CAC_ACCESS_RAMS, /*18*/
    TEST_READ_STARVING,
    TEST_READ_PERF, /*20*/
    TEST_PLIC_EL_TRIG,
    TEST_PLIC_PRIORITY,
    TEST_PLIC_THRESHOLD, /*23*/
    TEST_PLIC_ISR_PERF,
    TEST_MBOX_INTR, /*25*/
    TEST_MBOX_RW_REGS,
    TEST_NPU_TIMER,
    TEST_NPU_WDOG, /*28*/
    TEST_NPU_CPUTMR,
    TEST_NPU_RESET, /*30*/
    TEST_PER_CORE_LOCK,
    TEST_MULTI_CORE_LOCK,
    TEST_CORE_ON_OFF, /*33*/
    TEST_HANG_CORE_CSR,
    TEST_DEAD_CORE_CSR, /*35*/
    TEST_EXCEP_CORE_CSR,
    TEST_PLIC_RESET,
    TEST_HW_KERN_RESET, /*38*/
	TEST_HW_KERN_RESET_2,
	TEST_WT_DRAM_STARVING, /*40*/
    TEST_HOST_L2C_SRAM,
    TEST_NPU_AS_DMA,
    TEST_NPU_DRAM_COHERENCE, /*43*/
    TEST_NPU_RW_L2C_SRAM,
	TEST_NPU_BLOCK_CNT, /*45*/
	TEST_NPU_TZPC_MULTI_ISSUE,
	TEST_NPU_RBUS_STRESS,
	TEST_NPU_CPU_ACCESS, /*48*/
	TEST_NPU_DO_DMA,
	TEST_NPU_RD_BYPASS_CNT, /*50*/
	TEST_NPU_RD_BYPASS_STRESS,
	CMD_NPU_SYS_MEM_RW,
	TEST_NPU_BUS_READ_LATENCY, /*53*/
	TEST_NPU_WATCH_POINT,
	TEST_NPU_VDMA, /*55*/
	TEST_NPU_PCIE_PBUS_MAPPING,
	TEST_PLIC_TWO_INTR,
	TEST_NPU_DBG_CNT, /*58*/
	TEST_PCIE_RD_WAIT_WT_RESP,
    TEST_LAST_ITEM /* the last one */
};

#endif

