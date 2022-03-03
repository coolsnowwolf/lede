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
	mt_fdb.c
*/
#endif /* MTK_LICENSE */
#ifdef MT_FDB

#include "mcu/mt_fdb.h"

/************************* access FW CR API ***********************/
UINT32 HostAccessFwCr(RTMP_ADAPTER *pAd, UINT32 fw_cr_addr, UINT32 cr_op, UINT32 wrt_val)
{
        UINT32 ori_remap_cr_val = 0;
        UINT32 remapped_cr_val = 0;

        UINT32 rec_remap_cr_base_addr = 0;
        UINT32 target_cr_offset = 0;

		BOOLEAN fgFound;

        if (IS_MT7615(pAd) || IS_MT7622(pAd))
        {
                /* keep the origonal remap cr1 value for restore */
                HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &ori_remap_cr_val);
                /* do PCI-E remap for physical base address to 0x40000 */
                HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, fw_cr_addr);
                /* get real remap cr base addr */
                HW_IO_READ32(pAd, MCU_PCIE_REMAP_1, &rec_remap_cr_base_addr);

                if ((fw_cr_addr - rec_remap_cr_base_addr) > REMAP_1_OFFSET_MASK)
                {
                        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                                ("%s:Exceed remap range(offset: 0x%x > mask: 0x%x)!!\n", __FUNCTION__, 
                                        (fw_cr_addr - rec_remap_cr_base_addr), REMAP_1_OFFSET_MASK));
                        /* restore the origonal remap cr1 value */
                        HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, ori_remap_cr_val);
                }
                target_cr_offset = ((fw_cr_addr - rec_remap_cr_base_addr) & REMAP_1_OFFSET_MASK);

                if (cr_op == CR_OP_READ) {
                        RTMP_IO_READ32(pAd, 0x40000 + target_cr_offset, &remapped_cr_val);
                        
                        //MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:0x%x = 0x%x\n", 
                          //      __FUNCTION__, (0x40000 + target_cr_offset), remapped_cr_val));
                } else if (cr_op == CR_OP_WRITE) {
                        RTMP_IO_WRITE32(pAd, 0x40000 + target_cr_offset, wrt_val);
                }

                /* restore the origonal remap cr1 value */
                HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, ori_remap_cr_val);
		} else if (IS_MT7663(pAd)) {
			fgFound = mt_mac_cr_range_mapping(pAd, &fw_cr_addr);
			if (fgFound == FALSE) {/* not found*/
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s:7663 cannot access CR [%08X]!\n", __func__, fw_cr_addr));
			} else {
				if (cr_op == CR_OP_READ)
					RTMP_IO_READ32(pAd, fw_cr_addr, &remapped_cr_val);
				else if (cr_op == CR_OP_WRITE)
					RTMP_IO_WRITE32(pAd, fw_cr_addr, wrt_val);
			}
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s:Non 7615 don't support this cmd !\n", __func__));
		}
        return remapped_cr_val;
}


/************************* fdb.c *********************************/
uint32_t arg0, arg1, ret0, ret1, status, di_cmd_num, cmd_done_cnt;
char* exe_name;
char* arg0_str;
char* arg1_str;

char screen_buf[256];

struct di_cmd di_cmds[] = {
	{DIC_READ1, 					"rb", 		1, 1, false, NULL, 				memory_read_post, 	"\t\t\t-- read address(half-byte)"},
    {DIC_READ2, 					"rh", 		1, 1, false, NULL, 				memory_read_post, 	"\t\t\t-- read address(half-word)"},
    {DIC_READ4, 					"rw", 		1, 1, false, NULL, 				memory_read_post, 	"\t\t\t-- read address(word)"},
    {DIC_WRITE1, 					"wb", 		2, 0, false, NULL, 				NULL, 				"\t\t\t-- write address(half-byte)"},
    {DIC_WRITE2, 					"wh", 		2, 0, false, NULL, 				NULL, 				"\t\t\t-- write address(half-word)"},
    {DIC_WRITE4, 					"ww", 		2, 0, false, NULL, 				NULL, 				"\t\t\t-- write address(word)"},
    {DIC_GET_BUF,					"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_ENABLE, 				"enable", 	0, 0, true,	 NULL, 				NULL, 				"\t\t\t-- enable fdb"},
    {DIC_FDB_DISABLE, 				"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_HW_BKPT_SET, 			"hb", 		2, 0, true,	 NULL, 				NULL, 				"<index> <addr>\t\t-- set hardware breakpoint"},
    {DIC_FDB_HW_BKPT_DELETE, 		"d", 		1, 0, false, NULL, 				NULL, 				"<index>\t\t\t-- delect breakpoint"},
    {DIC_FDB_HW_BKPT_ENABLE, 		"en", 		1, 0, false, NULL, 				NULL, 				"<index>\t\t-- enable breakpoint"},
    {DIC_FDB_HW_BKPT_DISABLE, 		"dis", 		1, 0, false, NULL, 				NULL, 				"<index>\t\t-- disable breakpoint"},
    {DIC_FDB_CONTINUE, 				"c", 		0, 0, false, NULL, 				NULL, 				"\t\t\t-- continue"},
    {DIC_FDB_STEP, 					"si", 		0, 0, false, NULL, 				single_step_post, 	"\t\t\t-- single step"},
    {DIC_FDB_STOP, 					"stop", 	0, 0, true,	 NULL, 				NULL, 				"\t\t\t-- stop request"},
    {DIC_FDB_GET_CPU_REG, 			"get", 		1, 1, false, get_cpu_reg_pre, 	get_cpu_reg_post, 	"<CR name>\t\t-- read MCU register"},
    {DIC_FDB_SET_CPU_REG, 			"set", 		2, 0, false, set_cpu_reg_pre, 	NULL, 				"<CR name> <value>\t-- set MCU register"},
    {DIC_FDB_GET_TASK, 				"dump", 	0, 0, false, NULL, 				NULL, 				"\t\t\t-- dump call trace"},
    {DIC_FDB_GET_STACK, 			"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH, 				"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_RWATCH, 				"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_AWATCH, 				"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_RANGE, 			"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_SYMBOL, 			"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_EXPR_CONST, 		"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_EXPR_VARIABLE, 	"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_EXPR_DONE, 		"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_ENABLE, 			"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_DISABLE, 		"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_WATCH_DELETE, 			"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_FDB_FUNC_CALL, 			"call",		2, 1, false, NULL, 				NULL, 				"<addr> <arg0>\t-- function call with a argument"},
    {DIC_SYSTEM_STACK_INFO, 		"", 		0, 0, false, NULL, 				NULL, 				""},
    {DIC_SYSTEM_QUEUE_INFO, 		"", 		0, 0, false, NULL, 				NULL, 				""},
	{DIC_FDB_WMT_TEST, 				"test",		0, 0, true,	 NULL, 				NULL, 				"\t\t\t-- enter infinite loop"},
    {DIC_FDB_BKPT_INFO, 			"info",		0, 0, false, NULL, 				NULL, 				"\t\t\t-- breakpoint info"},
};

struct di_cmd intern_cmds[] = {
    {IC_CALL_TRACE,					"bt",	 	0, 0, false, dump_call_trace,	NULL,				"\t\t\t-- dump call trace"},
    {IC_CALL_HELP,					"help",	 	0, 0, false, print_help_msg,	NULL,				"\t\t\t-- print this help message"},
};

char *cr_mapping[] = {
    "PC",
    "PSW",
    "R28",
    "R29",
    "R30",
    "R15",
    "R3",
    "R4",
    "R5",
    "R6",
    "R7",
    "R8",
    "R9",
    "R10",
    "R0",
    "R1",
    "R2",
    "R31"
};

char cmd_str[CMD_STR_LEN];

uint32_t mcu_cr_mapping(char *mcu_cr_str){
	int i;

	for (i = 0; i < CONTEXT_NUM; i++){
		if (!strcmp(mcu_cr_str, cr_mapping[i])){
			return i;
		}
	}

	if (!strcmp(mcu_cr_str, "ITYPE")){
		return NDS32_SR_ITYPE + CONTEXT_NUM;
	}

	return 0;
}

void get_cpu_reg_pre(RTMP_ADAPTER *pAd){
	arg0 = mcu_cr_mapping(arg0_str);
}

void set_cpu_reg_pre(RTMP_ADAPTER *pAd){
	arg0 = mcu_cr_mapping(arg0_str);
}

void get_cpu_reg_post(RTMP_ADAPTER *pAd){
	//printk("%s = 0x%x\n", arg0_str, ret0);
}

void single_step_post(RTMP_ADAPTER *pAd){
	intern_debug_probe_cmd_call(pAd, DIC_FDB_GET_CPU_REG, "PC", NULL);
}

void memory_read_post(RTMP_ADAPTER *pAd){
	//printk("%s = 0x%x\n", arg0_str, ret0);
}

void dump_call_trace(RTMP_ADAPTER *pAd){

	uint32_t i;
	uint32_t context_sp;
	char addr_str[12];

	PRINT_CONTEXT_CR_T32(pAd, R28);
	PRINT_CONTEXT_CR_T32(pAd, R29);
	PRINT_CONTEXT_CR_T32(pAd, R30);
	PRINT_CONTEXT_CR_T32(pAd, R15);
	PRINT_CONTEXT_CR_T32(pAd, R3);
	PRINT_CONTEXT_CR_T32(pAd, R4);
	PRINT_CONTEXT_CR_T32(pAd, R5);
	PRINT_CONTEXT_CR_T32(pAd, R6);
	PRINT_CONTEXT_CR_T32(pAd, R7);
	PRINT_CONTEXT_CR_T32(pAd, R8);
	PRINT_CONTEXT_CR_T32(pAd, R9);
	PRINT_CONTEXT_CR_T32(pAd, R10);
	PRINT_CONTEXT_CR_T32(pAd, R0);
	PRINT_CONTEXT_CR_T32(pAd, R1);
	PRINT_CONTEXT_CR_T32(pAd, R2);
	PRINT_CONTEXT_CR_T32(pAd, R31);
	PRINT_CONTEXT_CR_T32(pAd, PSW);
	PRINT_CONTEXT_CR_T32(pAd, PC);

	intern_debug_probe_cmd_call(pAd, DIC_FDB_GET_CPU_REG, "R31", NULL);

	context_sp = ret0;

	for (i = 0; i < STACK_DEPTH; i++)
	{
		sprintf(addr_str, "0x%x", context_sp + (i*4));
		intern_debug_probe_cmd_call(pAd, DIC_READ4, addr_str, NULL);
		PREFIX_PRINTF(T32, "D.S D:0x%x %%LE %%LONG 0x%x\n",
				context_sp + (i*4),
				ret0);
	}
}

void write_cr(RTMP_ADAPTER *pAd, uint32_t addr, uint32_t val){
	HostAccessFwCr(pAd, addr, CR_OP_WRITE, val);
}

uint32_t read_cr(RTMP_ADAPTER *pAd, uint32_t addr){
	return HostAccessFwCr(pAd, addr, CR_OP_READ, 0);
}

void intern_cmd(RTMP_ADAPTER *pAd, struct di_cmd *di_cmd){
	if (di_cmd->pre_handle)
		di_cmd->pre_handle(pAd);

	if (di_cmd->post_handle)
		di_cmd->post_handle(pAd);
}

uint32_t get_cmd_done_cnt(RTMP_ADAPTER *pAd){
	return read_cr(pAd, DI_STATUS_REG) & DI_STATUS_CMD_DONE_CNT_MASK;
}

void increase_cmd_done_cnt(void){
	cmd_done_cnt++;
	cmd_done_cnt &= DI_STATUS_CMD_DONE_CNT_MASK;
}

bool is_cmd_done(RTMP_ADAPTER *pAd){
	return ((cmd_done_cnt + 1) & DI_STATUS_CMD_DONE_CNT_MASK) == get_cmd_done_cnt(pAd);
}

void debug_probe_cmd(RTMP_ADAPTER *pAd, struct di_cmd *cmd){

	PREFIX_PRINTF(INFO, "\n");
	PREFIX_PRINTF(INFO, "====== cmd id: 0x%x, cmd name = %s ======\n", cmd->id, cmd->name);

	if (cmd->pre_handle)
		cmd->pre_handle(pAd);

	if (cmd->arg_num > 0){
		PREFIX_PRINTF(INFO, "set arg0: 0x%x\n", arg0);
		write_cr(pAd, DI_ARG0_REG, arg0);
	}

	if (cmd->arg_num > 1){
		PREFIX_PRINTF(INFO, "set arg1: 0x%x\n", arg1);
		write_cr(pAd, DI_ARG1_REG, arg1);
	}

	PREFIX_PRINTF(INFO, "set cmd: 0x%x\n", arg1);
	write_cr(pAd, DI_CMD_REG, cmd->id);

	if (cmd->interrupt)
		write_cr(pAd, DI_IRQ_REG, 1);

	/* read status CR */
	PREFIX_PRINTF(INFO, "wait cmd done...\n");
	while(!is_cmd_done(pAd)){
		PREFIX_PRINTF(DBG, "cmd_done_cnt = 0x%x\n", cmd_done_cnt);
		RtmpusecDelay(WAIT_RET_US);
	}
	increase_cmd_done_cnt();

	PREFIX_PRINTF(INFO, "status:\n");
	status = read_cr(pAd, DI_STATUS_REG);

	/* read return CR */
	if (cmd->res_num > 0){
		PREFIX_PRINTF(INFO, "get ret0:\n");
		ret0 = read_cr(pAd, DI_RET0_REG);
	}

	if (cmd->res_num > 1){
		PREFIX_PRINTF(INFO, "get ret1:\n");
		ret1 = read_cr(pAd, DI_RET1_REG);
	}

	if (cmd->post_handle)
		cmd->post_handle(pAd);
}

void print_help_msg(RTMP_ADAPTER *pAd){
	int i;

	for (i = 0; i < di_cmd_num; i++){
		if (strcmp(di_cmds[i].name, ""))
			printk("./fdb %s %s\n", di_cmds[i].name, di_cmds[i].help_msg);
	}

	for (i = 0; i < IC_NUM; i++){
		printk("%s %s %s\n", exe_name, intern_cmds[i].name, intern_cmds[i].help_msg);
	}
}

int intern_debug_probe_cmd_call(RTMP_ADAPTER *pAd, uint32_t cmd_id, char *_arg0_str, char *_arg1_str){

	int i;

	arg0_str = _arg0_str;
	arg1_str = _arg1_str;
	
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,14))
	if (arg0_str)
		kstrtoul(arg0_str, 16, (unsigned long *)&arg0);

	if (arg1_str)
		kstrtoul(arg1_str, 16, (unsigned long *)&arg1);
#else
	if (arg0_str)
		strict_strtoul(arg0_str, 16, (unsigned long *)&arg0);

	if (arg1_str)
		strict_strtoul(arg1_str, 16, (unsigned long *)&arg1);
#endif

	for (i = 0; i < di_cmd_num; i++){
		if(cmd_id == di_cmds[i].id){
			debug_probe_cmd(pAd, &di_cmds[i]);
			break;
		}
	}

	return 0;
}

void fdb_enable(RTMP_ADAPTER *pAd)
{
	printk("%s()\n", __FUNCTION__);
	//pAd_tmp = pAd;
	/* enable FDB */
	write_cr(pAd, DI_CMD_REG, 0x1101);
	write_cr(pAd, DI_IRQ_REG, 1);
	RtmpOsMsDelay(100);
	read_cr(pAd, DI_STATUS_REG);

	/* enable FDB */
	write_cr(pAd, DI_CMD_REG, 0x1101);
	write_cr(pAd, DI_IRQ_REG, 1);
	RtmpOsMsDelay(100);
	read_cr(pAd, DI_STATUS_REG);

	/* set break point at cos_assert (0x2860) */
	write_cr(pAd, DI_ARG0_REG, 0);
	write_cr(pAd, DI_ARG1_REG, 0x2860);
	write_cr(pAd, DI_CMD_REG, 0x1103);
	write_cr(pAd, DI_IRQ_REG, 1);
	RtmpOsMsDelay(100);
	read_cr(pAd, DI_STATUS_REG);
}

#define N9_SER_TRIGGER_REASON 0x80000204

/* backup PSE Error */
#define N9_PSE_SPARE_DUMMY_CR1 0x820681e4

/* backup PLE Error */
#define N9_PSE_SPARE_DUMMY_CR2 0x820681e8

/* backup LMAC WDT Error */
#define N9_PSE_SPARE_DUMMY_CR3 0x820682e8

/* for 7663 SER status*/
#define MT7663_N9_SER_TRIGGER_REASON 0x80000604

/* backup 7663 PSE Error */
#define MT7663_N9_PSE_STATUS_CR 0x820681e4

/* backup 7663 PLE Error */
#define MT7663_N9_PLE_STATUS_CR 0x820681e8

/* backup 7663 LMAC WDT Error */
#define MT7663_N9_LMAC_STATUS_CR 0x820682ec


/* Bora CL51004 */
struct symbol_map{
	char* addr;
	char* name;
}symbol_map[] = {
	{"0x020a7240", "g_u4LastPhase1LmacInterruptTriggerTime"},
	{"0x020a72c4", "g_u4Band1TxLastData"},
	{"0x020a71a4", "g_u4TxTrb3NotIdleCnt"},
	{"0x020a7210", "g_u4PseHifDataOpCnt"},
	{"0x020a7124", "g_u4RxTimeStamp"},
	{"0x020a7250", "g_u4CurrentTrbRx0Psr2State"},
	{"0x020a7260", "g_u4LmacResetTotalCnt"},
	{"0x020a70dc", "g_eSerPrevState"},
	{"0x020a72ac", "g_u4Pdma2TxRing0Cnt"},
	{"0x020a71fc", "g_u4PseHifQueueCmdCnt"},
	{"0x020a7148", "g_u4EtxBf0NotIdleCnt"},
	{"0x020a7174", "g_u4Wtbl1NotIdleCnt"},
	{"0x020a7120", "g_u4RxDMARDPtr"},
	{"0x020a72c8", "g_txhang_check_time"},
	{"0x020a7164", "g_u4Trb1NotIdleCnt"},
	{"0x020a726c", "g_u4UmacLmacResetCausebyPseCnt"},
	{"0x020a71b0", "g_u4TxTrb0NotIdleCnt"},
	{"0x020a70f8", "g_u4SerPatchPleStatus"},
	{"0x020a728c", "g_u4Tmac0ResetCnt"},
	{"0x020a7114", "g_u4RxAbortByUnDef3Cnt"},
	{"0x020a7224", "g_u4PseHifFsmCnt"},
	{"0x020a7258", "g_u4CurrentTrbRx1Psr2State"},
	{"0x020a718c", "g_u4Agg1NotIdleCnt"},
	{"0x020a7220", "g_u4PsePageLinkCnt"},
	{"0x020a712c", "g_u4R1RxDMARDPtr"},
	{"0x020a7280", "g_u4UmacLmacResetCausebyTxhangChkTxCnt"},
	{"0x020a719c", "g_u4RxTrb1NotIdleCnt"},
	{"0x020a721c", "g_u4PseFramLinkCnt"},
	{"0x020a71e4", "g_u4PleFramLinkCnt"},
	{"0x020a70e0", "g_eSerCurrState"},
	{"0x020a70f0", "g_u4SerPseStatus"},
	{"0x020a71c4", "g_u4PleHifQueueCmdCnt"},
	{"0x020a7188", "g_u4Tmac0NotIdleCnt"},
	{"0x020a714c", "g_u4Lp1NotIdleCnt"},
	{"0x020a720c", "g_u4PseLmacAlloNullCnt"},
	{"0x020a7274", "g_u4UmacLmacResetCausebyPdmaCnt"},
	{"0x020a7128", "g_u4RxbusyCnt"},
	{"0x020a7158", "g_u4Pf0NotIdleCnt"},
	{"0x020a711c", "g_u4RxAbortByPoolingCnt"},
	{"0x020a717c", "g_u4RxDma1NotIdleCnt"},
	{"0x020a71ac", "g_u4TxTrb1NotIdleCnt"},
	{"0x020a72a8", "g_u4Pdma1TxRing1Cnt"},
	{"0x020a70ec", "g_u4SerLmacStatus"},
	{"0x020a71b8", "g_u4TxDma0NotIdleCnt"},
	{"0x020a7298", "g_u4Pdma0TxRing1Cnt"},
	{"0x020a7234", "g_u4LastRxAbortTriggerTime"},
	{"0x020a71e8", "g_u4PlePageLinkCnt"},
	{"0x020a71a0", "g_u4RxTrb0NotIdleCnt"},
	{"0x020a7218", "g_u4PseLmacDataOpCnt"},
	{"0x020a71b4", "g_u4TxDma1NotIdleCnt"},
	{"0x020a72b0", "g_u4Pdma2TxRing1Cnt"},
	{"0x020a7104", "g_u4Tmac1AbortCnt"},
	{"0x020a7268", "g_u4UmacLmacResetCausebyPleCnt"},
	{"0x020a7228", "g_u4PseCpuFsmCnt"},
	{"0x020a72bc", "g_u4Band1txHangCnt"},
	{"0x020a7238", "g_u4CurrectRxAbortTriggerTime"},
	{"0x020a7194", "g_u4Arb1NotIdleCnt"},
	{"0x020a7204", "g_u4PseLmacQueueCmdCnt"},
	{"0x020a71e0", "g_u4PleLmacDataOpCnt"},
	{"0x020a7254", "g_u4LastTrbRx0Psr2State"},
	{"0x020a7270", "g_u4UmacLmacResetCausebyPcieCnt"},
	{"0x020a70e8", "g_enumSER_Host_State"},
	{"0x020a7108", "g_u4Tmac0AbortCnt"},
	{"0x020a72c0", "g_u4Band0TxLastData"},
	{"0x020a72a0", "g_u4Pdma0TxRing3Cnt"},
	{"0x020a7118", "g_u4RxAbortByUnDef7Cnt"},
	{"0x020a725c", "g_u4LastTrbRx1Psr2State"},
	{"0x020a7290", "g_u4Tmac1ResetCnt"},
	{"0x020a71bc", "g_u4TxhangStep1CheckCnt"},
	{"0x020a71ec", "g_u4PleHifFsmCnt"},
	{"0x020a7160", "g_u4Rmac0NotIdleCnt"},
	{"0x020a7140", "g_u4MuNotIdleCnt"},
	{"0x020a7214", "g_u4PseCpuDataOpCnt"},
	{"0x020a71f8", "g_u4PleBwCnt"},
	{"0x020a71f4", "g_u4PleLmacFsmCnt"},
	{"0x020a724c", "g_u4CurrentRxDmaPointer"},
	{"0x020a7278", "g_u4UmacLmacResetCausebyLmacCnt"},
    {"0x020a727c", "g_u4UmacLmacResetCausebyEbfCnt"},
    {"0x020a7288", "g_u4LmacEbfTxResetCnt"},
    {"0x020a7208", "g_u4PseHifAlloNullCnt"},
    {"0x020a7200", "g_u4PseCpuQueueCmdCnt"},
    {"0x020a7244", "g_u4CurrentPhase1LmacInterruptTriggerTime"},
    {"0x020a7184", "g_u4Tmac1NotIdleCnt"},
    {"0x020a7264", "g_u4UmacLmacResetTotalCnt"},
    {"0x020a7100", "g_u4SerPCIeStatus"},
    {"0x020a723c", "g_u4ContinuousPhase1LmacInterruptCount"},
    {"0x020a71f0", "g_u4PleCpuFsmCnt"},
    {"0x020a716c", "g_u4Sec1NotIdleCnt"},
    {"0x020a7248", "g_u4LastRxDmaPointer"},
    {"0x020a72b4", "g_u4PdmaAllRxCnt"},
    {"0x020a7150", "g_u4Lp0NotIdleCnt"},
    {"0x020a7168", "g_u4Trb0NotIdleCnt"},
    {"0x020a70e4", "g_enumSER_CR4_State"},
    {"0x020a7154", "g_u4Pf1NotIdleCnt"},
    {"0x020a7170", "g_u4Sec0NotIdleCnt"},
    {"0x020a7178", "g_u4Wtbl0NotIdleCnt"},
    {"0x020a7144", "g_u4EtxBf1NotIdleCnt"},
    {"0x020a729c", "g_u4Pdma0TxRing2Cnt"},
    {"0x020a71d4", "g_u4PleLmacAlloNullCnt"},
    {"0x020a713c", "g_u4LmacResetByPhase1Cnt"},
    {"0x020a71c0", "g_u4TxhangStep2CheckCnt"},
    {"0x020a7134", "g_u4R1RxbusyCnt"},
    {"0x020a71a8", "g_u4TxTrb2NotIdleCnt"},
    {"0x020a71cc", "g_u4PleLmacQueueCmdCnt"},
    {"0x020a7284", "g_u4UmacLmacResetCausebyTxhangChkRxCnt"},
    {"0x020a71d8", "g_u4PleHifDataOpCnt"},
    {"0x020a72a4", "g_u4Pdma1TxRing0Cnt"},
    {"0x020a70f4", "g_u4SerPleStatus"},
    {"0x020a72b8", "g_u4Band0txHangCnt"},
    {"0x020a722c", "g_u4PseLmacFsmCnt"},
    {"0x020a7190", "g_u4Agg0NotIdleCnt"},
    {"0x020a70fc", "g_u4SerPdmaStatus"},
    {"0x020a71dc", "g_u4PleCpuDataOpCnt"},
    {"0x020a7110", "g_u4RxAbortByHdr0Cnt"},
    {"0x020a7180", "g_u4RxDma0NotIdleCnt"},
    {"0x020a71c8", "g_u4PleCpuQueueCmdCnt"},
    {"0x020a7138", "g_u4LmacResetBy3AbortCnt"},
    {"0x020a7294", "g_u4Pdma0TxRing0Cnt"},
    {"0x020a7198", "g_u4Arb0NotIdleCnt"},
    {"0x020a7230", "g_u4ContinuousTriggerRxAbortCount"},
    {"0x020a715c", "g_u4Rmac1NotIdleCnt"},
    {"0x020a7130", "g_u4R1RxTimeStamp"},
    {"0x020a71d0", "g_u4PleHifAlloNullCnt"},
    {"0x020a710c", "g_u4RxAbortStatus"},
	{NULL, NULL},
};

void n9_dump(RTMP_ADAPTER *pAd){

	uint32_t i = 0;
	uint32_t value = 0;
	uint32_t assert = 0;
	uint32_t exception = 0;
	struct symbol_map *symbol_map_p;

	cmd_done_cnt = get_cmd_done_cnt(pAd);
	di_cmd_num = sizeof(di_cmds) / sizeof(struct di_cmd);

	printk("============== N9 dump =============\n");

	value = HostAccessFwCr(pAd, DI_STATUS_REG, CR_OP_READ, 0);
	assert = (value & DI_STATUS_N9_ASSERT_MASK) >> DI_STATUS_N9_ASSERT_SHFT;
	exception = (value & DI_STATUS_N9_EXCEPTION_MASK) >> DI_STATUS_N9_EXCEPTION_SHFT;

	printk("ASSERT: %d\n", assert);
	printk("EXCEPTION: %d\n", exception);

	intern_debug_probe_cmd_call(pAd, DIC_FDB_STOP, NULL, NULL);

	if (assert){
		intern_debug_probe_cmd_call(pAd, DIC_READ1, "0x02090667", NULL);
		printk("exp_assert_state = 0x%x\n", ret0);
	}

	if (exception){
		intern_debug_probe_cmd_call(pAd, DIC_READ4, "0x0209a460", NULL);
		printk("exception_entry = 0x%x\n", ret0);
	}

	printk("call trace:\n");
	dump_call_trace(pAd);

	printk("get PC 10 times\n");
	for (i = 0; i < 10; i++){
		intern_debug_probe_cmd_call(pAd, DIC_FDB_GET_CPU_REG, "PC", NULL);
		printk("PC = 0x%x\n", ret0);

		if (ret0 == 0xa1c)
		break;

		intern_debug_probe_cmd_call(pAd, DIC_FDB_CONTINUE, NULL, NULL);
		RtmpusecDelay(10000);
		RtmpusecDelay(10000);
		RtmpusecDelay(10000);
		RtmpusecDelay(10000);
		RtmpusecDelay(10000);
		intern_debug_probe_cmd_call(pAd, DIC_FDB_STOP, NULL, NULL);
		RtmpusecDelay(10000);
		RtmpusecDelay(10000);
	}

	printk("============== N9 dump end =============\n");

	printk("============== SER dump (symbol ver. CL#160476) =============\n");

	if (IS_MT7663(pAd)) {
		printk("MT7663_N9_SER_TRIGGER_REASON = 0x%x\n",
				HostAccessFwCr(pAd, MT7663_N9_SER_TRIGGER_REASON, CR_OP_READ, 0));
		printk("MT7663_N9_PSE_STATUS_CR = 0x%x\n",
				HostAccessFwCr(pAd, MT7663_N9_PSE_STATUS_CR, CR_OP_READ, 0));
		printk("MT7663_N9_PLE_STATUS_CR = 0x%x\n",
				HostAccessFwCr(pAd, MT7663_N9_PLE_STATUS_CR, CR_OP_READ, 0));
		printk("MT7663_N9_LMAC_STATUS_CR = 0x%x\n",
				HostAccessFwCr(pAd, MT7663_N9_LMAC_STATUS_CR, CR_OP_READ, 0));
	} else {
		printk("SER_TRIGGER_REASON = 0x%x\n", HostAccessFwCr(pAd, N9_SER_TRIGGER_REASON, CR_OP_READ, 0));
		printk("PSE_SPARE_DUMMY_CR1 = 0x%x\n", HostAccessFwCr(pAd, N9_PSE_SPARE_DUMMY_CR1, CR_OP_READ, 0));
		printk("PSE_SPARE_DUMMY_CR2 = 0x%x\n", HostAccessFwCr(pAd, N9_PSE_SPARE_DUMMY_CR2, CR_OP_READ, 0));
		printk("PSE_SPARE_DUMMY_CR3 = 0x%x\n", HostAccessFwCr(pAd, N9_PSE_SPARE_DUMMY_CR3, CR_OP_READ, 0));
	}

	for (symbol_map_p = &symbol_map[0]; symbol_map_p->addr; symbol_map_p++){
		intern_debug_probe_cmd_call(pAd, DIC_READ4, symbol_map_p->addr, NULL);
		printk("%s(%s) = 0x%x\n", symbol_map_p->name, symbol_map_p->addr, ret0);
	}
	printk("============== SER dump end=============\n");
}

#define CR4_CMD_CR		0x82000208
#define CR4_TRIGGER_CR	0x820400C0
#define CR4_RET0_CR		0x8200020c

#define CR4_PRINT_CR(_pAd, addr)	\
	printk("0x%x = 0x%x\n", addr, HostAccessFwCr(_pAd, addr, CR_OP_READ, 0));

#define CR4_CMD(_pAd, cmd)	\
	printk("cmd = 0x%x, return = 0x%x\n", cmd, cr4_cmd(_pAd, cmd));

uint32_t cr4_cmd(RTMP_ADAPTER *pAd, uint32_t cmd){

	uint32_t val = 0;

	/* cmd */
	HostAccessFwCr(pAd, CR4_CMD_CR, CR_OP_WRITE, cmd);

	/* trigger */
	HostAccessFwCr(pAd, CR4_TRIGGER_CR, CR_OP_WRITE, 0x10);

	RtmpusecDelay(10000);
	RtmpusecDelay(10000);
	RtmpusecDelay(10000);
	RtmpusecDelay(10000);
	RtmpusecDelay(10000);

	/* read return */
	val = HostAccessFwCr(pAd, CR4_RET0_CR, CR_OP_READ, 0);

	HostAccessFwCr(pAd, CR4_RET0_CR, CR_OP_WRITE, 0);

	return val;
}

void cr4_dump(RTMP_ADAPTER *pAd){

	int i = 0;
	
	/*
		# CR4 HeartBeat and system info Status CR
		iwpriv ra0 mac 82000200
		# CR4 debug probe self test
		iwpriv ra0 mac 82000208=ffffffff;iwpriv ra0 mac 820400C0=10;sleep 1;iwpriv ra0 mac 8200020c
		iwpriv ra0 mac 8200020c=0
		iwpriv ra0 mac 82000208=ffffffff;iwpriv ra0 mac 820400C0=10;sleep 1;iwpriv ra0 mac 8200020c
		iwpriv ra0 mac 82000200
	 */
	printk("============== cr4 dump start=============\n");
	printk("CR4 HeartBeat and system info Status CR\n");
	CR4_PRINT_CR(pAd, 0x82000200);
	CR4_CMD(pAd, 0xffffffff);
	CR4_CMD(pAd, 0xffffffff);
	CR4_PRINT_CR(pAd, 0x82000200);

	/* Task and PC related: read three time to comfirm */
	printk("Task and PC related: read three time to comfirm\n");
	for (i = 0; i < 3; i++){
		CR4_CMD(pAd, 0x200);
		CR4_CMD(pAd, 0x201);
		CR4_CMD(pAd, 0x100);
		CR4_CMD(pAd, 0x101);
		CR4_CMD(pAd, 0x102);
		CR4_CMD(pAd, 0x103);
		CR4_CMD(pAd, 0x104);
		CR4_CMD(pAd, 0x226);
	}

	/* Packet monitor related */
		printk("Packet monitor related\n");
		CR4_CMD(pAd, 0x1);
		CR4_CMD(pAd, 0x4);
		CR4_CMD(pAd, 0x5);

	/* Exception related */
		printk("Exception related\n");
		CR4_CMD(pAd, 0x228);
		CR4_CMD(pAd, 0x229);
		CR4_CMD(pAd, 0x230);

	/* SER related */
		printk("SER related\n");
		CR4_CMD(pAd, 0x210);
		CR4_CMD(pAd, 0x211);
		CR4_CMD(pAd, 0x212);
		CR4_CMD(pAd, 0x213);
		CR4_CMD(pAd, 0x215);

		/* PDMA1 related */
		printk("PDMA1 related\n");
		CR4_PRINT_CR(pAd, 0x820b0204);
		CR4_PRINT_CR(pAd, 0x820b0004);
		CR4_PRINT_CR(pAd, 0x820b0008);
		CR4_PRINT_CR(pAd, 0x820b000c);

		CR4_PRINT_CR(pAd, 0x820b0014);
		CR4_PRINT_CR(pAd, 0x820b0018);
		CR4_PRINT_CR(pAd, 0x820b001c);

		CR4_PRINT_CR(pAd, 0x820b0104);
		CR4_PRINT_CR(pAd, 0x820b0108);
		CR4_PRINT_CR(pAd, 0x820b010c);

		CR4_PRINT_CR(pAd, 0x820b0114);
		CR4_PRINT_CR(pAd, 0x820b0118);
		CR4_PRINT_CR(pAd, 0x820b011c);


		/* PDMA2 related */
		printk("PDMA2 related\n");
		CR4_PRINT_CR(pAd, 0x820c0204);
		CR4_PRINT_CR(pAd, 0x820c0004);
		CR4_PRINT_CR(pAd, 0x820c0008);
		CR4_PRINT_CR(pAd, 0x820c000c);

		CR4_PRINT_CR(pAd, 0x820c0014);
		CR4_PRINT_CR(pAd, 0x820c0018);
		CR4_PRINT_CR(pAd, 0x820c001c);

		CR4_PRINT_CR(pAd, 0x820c0104);
		CR4_PRINT_CR(pAd, 0x820c0108);
		CR4_PRINT_CR(pAd, 0x820c010c);

		CR4_PRINT_CR(pAd, 0x820c0114);
		CR4_PRINT_CR(pAd, 0x820c0118);
		CR4_PRINT_CR(pAd, 0x820c011c);
	printk("============== cr4 dump end=============\n");

}

INT show_fdb_n9_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	n9_dump(pAd);
	return TRUE;
}


INT show_fdb_cr4_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	cr4_dump(pAd);
	return TRUE;
}

#endif /* MT_FDB */

