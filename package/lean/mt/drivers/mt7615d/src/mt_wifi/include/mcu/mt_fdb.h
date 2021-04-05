#ifdef MTK_LICENSE
/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mt_fdb.h
*/
#endif /* MTK_LICENSE */
#ifdef MT_FDB

#ifndef __MT_FDB_H__
#define __MT_FDB_H__

#include <linux/types.h>
#include "rt_config.h"

/************************* access FW CR API ***********************/
#define CR_OP_READ (0)
#define CR_OP_WRITE (1)
#define FW_EINT_SW_TRIGGER_CR        0x81040118
#define FW_EINT_PAD_NO               BIT(0)

/************************* nds32_intrinsic.h ***********************/
enum nds32_sr {
NDS32_SR_CPU_VER,
NDS32_SR_ICM_CFG,
NDS32_SR_DCM_CFG,
NDS32_SR_MMU_CFG,
NDS32_SR_MSC_CFG,
NDS32_SR_CORE_ID,
NDS32_SR_FUCOP_EXIST,
NDS32_SR_PSW,
NDS32_SR_IPSW,
NDS32_SR_P_IPSW,
NDS32_SR_IVB,
NDS32_SR_EVA,
NDS32_SR_P_EVA,
NDS32_SR_ITYPE,
NDS32_SR_P_ITYPE,
NDS32_SR_MERR,
NDS32_SR_IPC,
NDS32_SR_P_IPC,
NDS32_SR_OIPC,
NDS32_SR_P_P0,
NDS32_SR_P_P1,
NDS32_SR_INT_MASK,
NDS32_SR_INT_PEND,
NDS32_SR_SP_USR,
NDS32_SR_SP_PRIV,
NDS32_SR_INT_PRI,
NDS32_SR_INT_CTRL,
NDS32_SR_MMU_CTL,
NDS32_SR_L1_PPTB,
NDS32_SR_TLB_VPN,
NDS32_SR_TLB_DATA,
NDS32_SR_TLB_MISC,
NDS32_SR_VLPT_IDX,
NDS32_SR_ILMB,
NDS32_SR_DLMB,
NDS32_SR_CACHE_CTL,
NDS32_SR_HSMP_SADDR,
NDS32_SR_HSMP_EADDR,
NDS32_SR_SDZ_CTL,
NDS32_SR_N12MISC_CTL,
NDS32_SR_MISC_CTL,
NDS32_SR_BPC0,
NDS32_SR_BPC1,
NDS32_SR_BPC2,
NDS32_SR_BPC3,
NDS32_SR_BPC4,
NDS32_SR_BPC5,
NDS32_SR_BPC6,
NDS32_SR_BPC7,
NDS32_SR_BPA0,
NDS32_SR_BPA1,
NDS32_SR_BPA2,
NDS32_SR_BPA3,
NDS32_SR_BPA4,
NDS32_SR_BPA5,
NDS32_SR_BPA6,
NDS32_SR_BPA7,
NDS32_SR_BPAM0,
NDS32_SR_BPAM1,
NDS32_SR_BPAM2,
NDS32_SR_BPAM3,
NDS32_SR_BPAM4,
NDS32_SR_BPAM5,
NDS32_SR_BPAM6,
NDS32_SR_BPAM7,
NDS32_SR_BPV0,
NDS32_SR_BPV1,
NDS32_SR_BPV2,
NDS32_SR_BPV3,
NDS32_SR_BPV4,
NDS32_SR_BPV5,
NDS32_SR_BPV6,
NDS32_SR_BPV7,
NDS32_SR_BPCID0,
NDS32_SR_BPCID1,
NDS32_SR_BPCID2,
NDS32_SR_BPCID3,
NDS32_SR_BPCID4,
NDS32_SR_BPCID5,
NDS32_SR_BPCID6,
NDS32_SR_BPCID7,
NDS32_SR_EDM_CFG,
NDS32_SR_EDMSW,
NDS32_SR_EDM_CTL,
NDS32_SR_EDM_DTR,
NDS32_SR_BPMTC,
NDS32_SR_DIMBR,
NDS32_SR_TECR0,
NDS32_SR_TECR1,
NDS32_SR_EDM_PROBE,
NDS32_SR_PFMC0,
NDS32_SR_PFMC1,
NDS32_SR_PFMC2,
NDS32_SR_PFM_CTL,
NDS32_SR_FUCOP_CTL,
NDS32_SR_PRUSR_ACC_CTL,
NDS32_SR_DMA_CFG,
NDS32_SR_DMA_GCSW,
NDS32_SR_DMA_CHNSEL,
NDS32_SR_DMA_ACT,
NDS32_SR_DMA_SETUP,
NDS32_SR_DMA_ISADDR,
NDS32_SR_DMA_ESADDR,
NDS32_SR_DMA_TCNT,
NDS32_SR_DMA_STATUS,
NDS32_SR_DMA_2DSET,
NDS32_SR_DMA_2DSCTL
};


/************************* nds32_v2_fdb.h ***********************/
enum target_debug_reason {
    DBG_REASON_DBGRQ = 0,
    DBG_REASON_BREAKPOINT = 1,
    DBG_REASON_WATCHPOINT = 2,
    DBG_REASON_SINGLESTEP = 3,
    DBG_REASON_UNDEFINED = 4
};

enum nds32_debug_reason {
    NDS32_DEBUG_BREAK = 0,
    NDS32_DEBUG_BREAK_16,
    NDS32_DEBUG_INST_BREAK,
    NDS32_DEBUG_DATA_ADDR_WATCHPOINT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_IMPRECISE,
    NDS32_DEBUG_DEBUG_INTERRUPT,
    NDS32_DEBUG_HARDWARE_SINGLE_STEP,
    NDS32_DEBUG_DATA_ADDR_WATCHPOINT_NEXT_PRECISE,
    NDS32_DEBUG_DATA_VALUE_WATCHPOINT_NEXT_PRECISE,
    NDS32_DEBUG_LOAD_STORE_GLOBAL_STOP,
};

enum target_state {
    TARGET_RUNNING = 0,
    TARGET_HALTED = 1,
};

enum nds32_v2_context{
    PCX,
    PSW,
    R28,
    R29,
    R30,
    R15,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R0,
    R1,
    R2,
    R31,
    CONTEXT_NUM
};


/************************* debug_probe.h ***********************/
/* MCU Debug Prob */
#define DI_CMD_REG      0x8206c068
#define DI_ARG0_REG     0x8206c06c
#define DI_ARG1_REG     0x8206c070
#define DI_RET0_REG     DI_ARG0_REG
#define DI_RET1_REG     DI_ARG1_REG
#define DI_STATUS_REG   0x8206c064

#define DI_STATUS_CMD_DONE_CNT_ADDR           DI_STATUS_REG
#define DI_STATUS_CMD_DONE_CNT_SHFT           0
#define DI_STATUS_CMD_DONE_CNT_MASK           (0xF << DI_STATUS_CMD_DONE_CNT_SHFT)

#define DI_STATUS_CMD_ERR_CODE_ADDR           DI_STATUS_REG
#define DI_STATUS_CMD_ERR_CODE_SHFT           4
#define DI_STATUS_CMD_ERR_CODE_MASK           (0xF << DI_STATUS_CMD_ERR_CODE_SHFT)

#define DI_STATUS_N9_EXCEPTION_ADDR           DI_STATUS_REG
#define DI_STATUS_N9_EXCEPTION_SHFT           8
#define DI_STATUS_N9_EXCEPTION_MASK           (0x1 << DI_STATUS_N9_EXCEPTION_SHFT)

#define DI_STATUS_N9_SER_ADDR                 DI_STATUS_REG
#define DI_STATUS_N9_SER_SHFT                 9
#define DI_STATUS_N9_SER_MASK                 (0x1 << DI_STATUS_N9_SER_SHFT)

#define DI_STATUS_N9_ASSERT_ADDR              DI_STATUS_REG
#define DI_STATUS_N9_ASSERT_SHFT              10
#define DI_STATUS_N9_ASSERT_MASK              (0x1 << DI_STATUS_N9_ASSERT_SHFT)

#define DI_STATUS_TARGET_STATE_ADDR           DI_STATUS_REG
#define DI_STATUS_TARGET_STATE_SHFT           31
#define DI_STATUS_TARGET_STATE_MASK           (0x1 << DI_STATUS_TARGET_STATE_SHFT)

#define DI_STATUS_TARGET_DEBUG_REASON_ADDR    DI_STATUS_REG
#define DI_STATUS_TARGET_DEBUG_REASON_SHFT    28
#define DI_STATUS_TARGET_DEBUG_REASON_MASK    (0x7 << DI_STATUS_TARGET_DEBUG_REASON_SHFT)

enum DI_CMD {

    /* common debug interface command */
    DIC_READ1 = 0x1001,
    DIC_READ2,
    DIC_READ4,
    DIC_WRITE1,
    DIC_WRITE2,
    DIC_WRITE4,
    DIC_GET_BUF,

    /* FDB debug interface command */
    DIC_FDB_ENABLE = 0x1101,
    DIC_FDB_DISABLE,
    DIC_FDB_HW_BKPT_SET,
    DIC_FDB_HW_BKPT_DELETE,
    DIC_FDB_HW_BKPT_ENABLE,
    DIC_FDB_HW_BKPT_DISABLE,
    DIC_FDB_CONTINUE,
    DIC_FDB_STEP,
    DIC_FDB_STOP,
    DIC_FDB_GET_CPU_REG,
    DIC_FDB_SET_CPU_REG,
    DIC_FDB_GET_TASK,
    DIC_FDB_GET_STACK,
    DIC_FDB_WATCH,
    DIC_FDB_RWATCH,
    DIC_FDB_AWATCH,
    DIC_FDB_WATCH_RANGE,
    DIC_FDB_WATCH_SYMBOL,
    DIC_FDB_WATCH_EXPR_CONST,
    DIC_FDB_WATCH_EXPR_VARIABLE,
    DIC_FDB_WATCH_EXPR_DONE,
    DIC_FDB_WATCH_ENABLE,
    DIC_FDB_WATCH_DISABLE,
    DIC_FDB_WATCH_DELETE,
	DIC_FDB_FUNC_CALL,

    /* System debug interface commnad */
    DIC_SYSTEM_STACK_INFO = 0x1201,
    DIC_SYSTEM_QUEUE_INFO,

    /* For test */
	DIC_FDB_WMT_TEST = 0x1f01,
    DIC_FDB_BKPT_INFO,
};

enum DI_CMD_ERR_CODE {
    ERR_OK,
    ERR_FAIL,
    ERR_TARGET_ACCESS_REG_FAIL,
    ERR_TARGET_NOT_HALTED,
    ERR_TARGET_NOT_RUNNING,
    ERR_TARGET_UNALIGNED_ACCESS,
    ERR_TARGET_RESOURCE_NOT_AVAILABLE
};


/************************* fdb.c *********************************/
#define DI_IRQ_REG 0x81040118

#define CMD_STR_LEN 100
#define WAIT_RET_US 10000

#define CMD		argv[1]
#define ARG0	argv[2]
#define ARG1	argv[3]

#define STACK_DEPTH 100

#define PRINT_T32_MSG		1
#define PRINT_IWPRIV_MSG	0
#define PRINT_INFO_MSG		0
#define PRINT_DBG_MSG		0

#define NON_PREFIX_PRINTF(prefix, fmt, arg...) \
	{ \
	if (PRINT_##prefix##_MSG)	\
		printk(fmt, ## arg);	\
	}

#define PREFIX_PRINTF(prefix, fmt, arg...) \
	{ \
	if (PRINT_##prefix##_MSG)	\
		printk("[" #prefix "] " fmt, ## arg);	\
	}

#define PRINT_CONTEXT_CR_T32(_pAd, srname)  { \
	intern_debug_probe_cmd_call(_pAd, DIC_FDB_GET_CPU_REG, #srname, NULL); \
	PREFIX_PRINTF(T32, "R.S " #srname " 0x%x\n", ret0); \
	}

uint32_t mcu_cr_mapping(char *mcu_cr_str);
void set_cpu_reg_pre(RTMP_ADAPTER *pAd);
void get_cpu_reg_pre(RTMP_ADAPTER *pAd);
void get_cpu_reg_post(RTMP_ADAPTER *pAd);
void single_step_post(RTMP_ADAPTER *pAd);
void memory_read_post(RTMP_ADAPTER *pAd);
void dump_call_trace(RTMP_ADAPTER *pAd);
void print_help_msg(RTMP_ADAPTER *pAd);
int intern_debug_probe_cmd_call(RTMP_ADAPTER *pAd, uint32_t cmd_id, char *_arg0_str, char *_arg1_str);

struct di_cmd{
	uint32_t id;
	char *name;
	uint8_t arg_num;
	uint8_t res_num;
	bool interrupt;
	void (*pre_handle)(RTMP_ADAPTER *pAd);
	void (*post_handle)(RTMP_ADAPTER *pAd);
	char *help_msg;
};

enum intern_cmds_id{
	IC_CALL_TRACE,
	IC_CALL_HELP,
	IC_NUM
};

#endif /* __MT_FDB_H__ */
#endif /* MT_FDB */


