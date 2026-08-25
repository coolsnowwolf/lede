/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/proc_fs.h>
#include "common/xpon_global.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_security.h"
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include "gpon/gpon_recovery.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
/* 0: slow mode,  1: fast mode */
int xgpon_fast_mode_flag = 0;

int drop_print_flag =0;
uint bwmap_print_flag = 0x7fff;
uint sw_resync_flag = 1;
int max_cnt = 20;
int omciMicErrSwCnt = 0;

#if defined(TCSUPPORT_CPU_EN7581)
int gpon_tod_adjust = 0;
uint hw_gem_config = 1;
#else
uint hw_gem_config = 0;
#endif

RDK_GTC_Dbg_T rdk_gtc_dbg = {};
RDK_MIC_Dbg_T rdk_mic_err_dbg = {};


/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
typedef struct	s_proc_cmds
{
	char cmd[20];
	read_proc_t *read_proc;
	write_proc_t *write_proc;	 
} T_PROC_CMDS;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern REG_INT_STATUS intStatus;
extern REG_INT_ENABLE intEnable;
extern int is_hwnat_dont_clean;
extern int ploam_recv_disable_serial_number(PLOAM_RAW_Disable_SN_T *);
int xmcs_clear_rogue_status(void);
int xmcs_get_rogue_status(PON_RogueStatus_t *status);
#ifdef CONFIG_USE_FOR_TEST
extern uint eqdDbgFlag;
extern uint EqdCounter;
extern uint EqdArray[1001];
#endif /* CONFIG_USE_FOR_TEST */
extern int g_rm_tcont_delay_us;
extern int xmcs_get_olt_distance(uint32_t * oltDistance);
extern uint distanceAdjust;
extern uint hw_cnt_enable;


int omciIkIdxExchange = 0;
int dropCpuTxPktsFlag = 0;
int xpon_mac_print_open =0;

/*************for NG2 channel swap debug************/
int ng2_mon_not_gnt = 1;
int ng2_tun_resp_key = 1;
int ng2_o4_to_09 = 0;
int ng2_o8_to_05 = 0;
int ng2_no_rollback = 0;
int ng2_man_set_09 = 0;
int ng2_set_state = 0;
int ng2_ignore_disable = 0;

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static int rdkb_write_gtc_count(char * subcmd,uint action);
static int	gpon_alarm_proc_write(char *cmd, char *subcmd);
static int gpon_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
static int gpon_emulate_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data);
static int gpon_emulate_proc_write(struct file *file, const char *buffer,unsigned long count, void *data);
static int gpon_crypto_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_crypto_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);
static int gpon_normal_counter_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_normal_counter_write_proc(struct file *file, const char *buffer,unsigned long count, void *data);
static int gpon_err_counter_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_err_counter_write_proc(struct file *file, const char *buffer,unsigned long count, void *data);
static int gpon_err_status_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_err_status_write_proc(struct file *file, const char *buffer,unsigned long count, void *data);
static int gpon_key_info_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_key_info_write_proc(struct file *file, const char *buffer,unsigned long count, void *data);
#ifdef CONFIG_TP_IMAGE
static int gpon_linkstate_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_state_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_onuid_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
#endif /* CONFIG_TP_IMAGE */
static int gpon_bbf247_flag_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_fast_slow_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data);
static int gpon_fast_slow_write_proc(struct file *file, const char *buffer, unsigned long count, void *data);


extern int enQueue(ploam_queue_t *queue, uint data);

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
static struct proc_dir_entry *gpon_proc_dir = NULL;
static	T_PROC_CMDS proc_gpon_cmds[] =
{
	{"debug", gpon_debug_read_proc, gpon_debug_write_proc},
	{"emulate", gpon_emulate_proc_read, gpon_emulate_proc_write},
	{"crypto", gpon_crypto_read_proc, gpon_crypto_write_proc},	
	{"counter", gpon_normal_counter_read_proc, gpon_normal_counter_write_proc},
	{"errcnt", gpon_err_counter_read_proc, gpon_err_counter_write_proc},
	{"errsts", gpon_err_status_read_proc, gpon_err_status_write_proc},
	{"keyinfo", gpon_key_info_read_proc, gpon_key_info_write_proc},
#ifdef CONFIG_TP_IMAGE
	{"linkstate", gpon_linkstate_read_proc, NULL},
	{"state", gpon_state_read_proc, NULL},
	{"onuid", gpon_onuid_read_proc, NULL},
#endif /* CONFIG_TP_IMAGE */
	{"bbf247Flag", gpon_bbf247_flag_read_proc, NULL},
	{"fastmode", gpon_fast_slow_read_proc, gpon_fast_slow_write_proc},
	{"0",0, 0},
};
/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
void XGPON_MAC_EVENT_HANDLER(PON_PHY_Event_data_t * pEvent)
{
    struct xpon_mac_hook_data_s data = {0} ;
    data.src_module  = XPON_PHY_MODULE  ;
    data.pEvent      = pEvent ;
	
	PON_MSG(MSG_DBG, "PHY ready, ECNT hook XPON MAC\n") ;
    
    if(ECNT_HOOK_ERROR == __ECNT_HOOK(ECNT_XPON_MAC, ECNT_XPON_MAC_HOOK, (struct ecnt_data * )&data) ){
        printk("ECNT_HOOK_ERROR occur with event id:%d. %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
    }
}
/*______________________________________________________________________________
**  function name
**		gpon_debug_read_proc
**  description:
**		gpon debug read proc
**  parameters:
**		buf: a single page used as a buffer
**		start: begining of the returned data
**		off: current offset into proc file
**		count: amount of data to read
**		eof: eof marker
**		data: data passed that was registered earlier
**  global:
**		gpPonSysData.
**  return:
**		0:  ok
**		others: failure
**  call:
**		None.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
static int gpon_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	printk("Debug Level: %x MSG_EQD:%x  MSG_OAM|MSG_EQD:%x (MSG_OAM|MSG_EQD):%x\n", gpPonSysData->debugLevel,MSG_EQD,MSG_OAM|MSG_EQD,(MSG_OAM|MSG_EQD)) ;
	return 0;
}
/*______________________________________________________________________________
**  function name
**		gpon_debug_write_proc
**  description:
**		gpon debug write proc
**  parameters:
**		file:
**		buffer: the pointer pointing to the data received from user
**		count: amount of data to write
**		data: data passed that was registered earlier
**  global:
**		gpPonSysData.
**  return:
**		0:  ok
**		others: failure
**  call:
**		None.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
static int gpon_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[128]={0}, cmd[32]={0}, subcmd[32]={0} ;
	uint action = 0 ;
	int i = 0;
	int ret = 0;
    uint tmp = 0;
	PLOAM_RAW_Disable_SN_T SnMsg;
	int bwmapLevel = 0;
	unchar index = 0;
	PHY_Xgpon_Profile_Msg_T xgponPhyProfile;
	char *endpo=NULL;
	REG_INT_ENABLE gponIntEnable;

	if (count > (sizeof(val_string) - 1))
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%31s %31s %x", cmd, subcmd, &action) ;

	if(!strcmp(cmd, "msg")) {
		int level = gpPonSysData->debugLevel ;
		
		if(!strcmp(subcmd, "act")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_ACT) : (level&~MSG_ACT) ;
		} else if(!strcmp(subcmd, "omci")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_OMCI) : (level&~MSG_OMCI) ;
		} else if(!strcmp(subcmd, "oam")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_OAM) : (level&~MSG_OAM) ;
		} else if(!strcmp(subcmd, "int")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_INT) : (level&~MSG_INT) ;
		} else if(!strcmp(subcmd, "trace")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_TRACE) : (level&~MSG_TRACE) ;
		} else if(!strcmp(subcmd, "warning")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_WARN) : (level&~MSG_WARN) ;
		} else if(!strcmp(subcmd, "content")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_CONTENT) : (level&~MSG_CONTENT) ;
		} else if(!strcmp(subcmd, "debug")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_DBG) : (level&~MSG_DBG) ;
		} else if(!strcmp(subcmd, "secure")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_SECUR) : (level&~MSG_SECUR) ;
		}else if(!strcmp(subcmd, "err")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_ERR) : (level&~MSG_ERR) ;
		}else if(!strcmp(subcmd, "eqd")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_EQD) : (level&~MSG_EQD) ;
		} else if(!strcmp(subcmd, "xmcs")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_XMCS) : (level&~MSG_XMCS) ;
		} else if(!strcmp(subcmd, "allenable")) {
			gpPonSysData->debugLevel = 0xfff ;
		} else if(!strcmp(subcmd, "alldisable")) {
			gpPonSysData->debugLevel = 0 ;
		}else{
            printk("wrong msg subcmd\n");
		}
		printk("Debug Level: %x\n", gpPonSysData->debugLevel) ; 
	}else if(!strcmp(cmd, "dump_gem_tcont")) {
		printk("dump XGEM/Tcont info form hw\n");
		if(0 != gponDevDumpGemInfo()){
			printk("gponDevDumpGemInfo return error!\n");
		}
		if(0 != gponDevDumpTcontInfo()){
			printk("gponDevDumpTcontInfo return error!\n");
		}
	}else if(!strcmp(cmd, "drop")){	
		drop_print_flag =action ;
		printk("drop_print_flag =%d\n",drop_print_flag);
	}
#ifdef TCSUPPORT_CPU_EN7581
	else if(!strcmp(cmd, "sreg"))
	{
		if(!strcmp(subcmd, "get_rx_idle_xgem"))
		{
			gponDevGetRxIdleXgem(action);
		}else
		{
			printk("wrong write sreg subcmd\n");
		}
	}
	else if(!strcmp(cmd, "change_O4"))
	{
		REG_ACTIVATION_ST xgponActState;
		REG_DBG_DS_SPF_CNT_L spfAcount;
		REG_DBG_RESYNC resync;
		unsigned int spfAcount_O2 = 0;
		unsigned int spfAcount_O4 = 0;
		int i = 0;
		xgponActState.Raw = IO_GREG(ACTIVATION_ST) ;
		xgponActState.Bits.act_st = 2 ;
		IO_SREG(ACTIVATION_ST, xgponActState.Raw) ;
		spfAcount.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
		spfAcount_O2 = spfAcount.Bits.ds_spf_cnt_l32;
		xgponActState.Raw = IO_GREG(ACTIVATION_ST) ;
		xgponActState.Bits.act_st = 4 ;
		IO_SREG(ACTIVATION_ST, xgponActState.Raw) ;
		spfAcount.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
		spfAcount_O4 = spfAcount.Bits.ds_spf_cnt_l32;
		printk("spfAcount on O2_O4 spfAcount_O2 = %.8x, spfAcount_O4 = %.8x\n",spfAcount_O2,spfAcount_O4);
		while(i < 3)
		{
			udelay(500);
			resync.Raw = IO_GREG(DBG_RESYNC);
			printk("resync_%d = %d\n",i,resync.Bits.tx_sync_rdy);
			i++;
		}
		gpon_err_status_read_proc(NULL,NULL,1,1,NULL,(void *)NULL);
	}
	else if(!strcmp(cmd, "mbi"))
	{
		REG_MBI_MPI_STOP xgponMbiMpiStop;
		xgponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
		xgponMbiMpiStop.Bits.del_rx_stop = 1 ;
		IO_SREG(MBI_MPI_STOP, xgponMbiMpiStop.Raw) ;
		udelay(1000);
		xgponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
		xgponMbiMpiStop.Bits.mbi_tx_stop = 1 ;
		xgponMbiMpiStop.Bits.mbi_rx_stop = 1 ;
		xgponMbiMpiStop.Bits.dbru_stop	= 1 ;
		IO_SREG(MBI_MPI_STOP, xgponMbiMpiStop.Raw) ;
		udelay(1000);
		xgponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
		printk("Read mbi_tx/rx_stop=1 bfb65004 = %.8x\n",xgponMbiMpiStop.Raw);
		msleep(20000);
		xgponMbiMpiStop.Bits.mbi_tx_stop = 0 ;
		xgponMbiMpiStop.Bits.mbi_rx_stop = 0 ;
		xgponMbiMpiStop.Bits.dbru_stop	= 0 ;
		IO_SREG(MBI_MPI_STOP, xgponMbiMpiStop.Raw) ;
		udelay(1000);
		xgponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
		xgponMbiMpiStop.Bits.del_rx_stop = 0 ;
		IO_SREG(MBI_MPI_STOP, xgponMbiMpiStop.Raw) ;
		udelay(1000);
		xgponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;
		printk("Read mbi_tx/rx_stop=0 bfb65004 = %.8x\n",xgponMbiMpiStop.Raw);
	}
	else if(!strcmp(cmd, "sw"))
	{
		if(!strcmp(subcmd, "resync"))
		{
			sw_resync_flag = action;
			printk("sw_resync_flag = %d, %s\n",sw_resync_flag,sw_resync_flag ? "sw resync enable" : "sw resync disable");
		}
	}
	else if(!strcmp(cmd,"max_cnt"))
	{
		if(!strcmp(subcmd,"set"))
		{
			max_cnt= action;
			printk("max_cnt = %x\n",max_cnt);
		}

	}
	else if(!strcmp(cmd, "switch"))
	{
		if(!strcmp(subcmd, "mpi_rx"))
		{
			uint time = 0;
			REG_MBI_MPI_STOP gponMbiMpiStop ;

		    /* STOP/START MPI Interface ; 1:stop; 0:start*/
			gponMbiMpiStop.Raw = IO_GREG(MBI_MPI_STOP) ;

		    gponMbiMpiStop.Bits.mpi_rx_stop = 1;
			IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

			time = action;
			udelay(time);

			gponMbiMpiStop.Bits.mpi_rx_stop = 0;
			IO_SREG(MBI_MPI_STOP, gponMbiMpiStop.Raw) ;

			printk("Mpi rx switch time = %d\n",time);
		}
	}
	else if(!strcmp(cmd, "config"))
	{
		if(!strcmp(subcmd, "cal_gnt_size_start"))
		{			
			uint data = 0;
			uint tcont_sel = 0;
			uint times = 1;			
			REG_CAL_GNT_SIZE_TCONT_EN calGntSizeTcontEn;
			REG_CAL_GNT_SIZE_CTRL_STS calGntSizeCtrlSts;
			REG_CAL_GNT_SIZE_SUM_TCONT_SEL calGntSizeSumTcontSel;
			REG_CAL_GNT_SIZE_SUM_L32 calGntSizeSumL32;
			REG_CAL_GNT_SIZE_SUM_H32 calGntSizeSumH32;
			REG_DBG_DS_SPF_CNT_L spfAcount;
			
			sscanf(val_string, "%s %s %x %x %u", cmd, subcmd, &action,&data,&tcont_sel);
			
			calGntSizeTcontEn.Raw = IO_GREG(CAL_GNT_SIZE_TCONT_EN);
			calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS);			
			calGntSizeTcontEn.Bits.cal_gnt_size_tcont_en = action;
			calGntSizeCtrlSts.Bits.cal_gnt_size_gtc_num = data;
			calGntSizeCtrlSts.Bits.cal_gnt_size_start = 1;

			IO_SREG(CAL_GNT_SIZE_TCONT_EN, calGntSizeTcontEn.Raw);
			IO_SREG(CAL_GNT_SIZE_CTRL_STS, calGntSizeCtrlSts.Raw);
			
			spfAcount.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
			printk("Calculate grant size start super frame counter = %.8x\n", spfAcount.Raw);
			
			while(times++) 
			{
				calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS) ;
				if(1 == calGntSizeCtrlSts.Bits.cal_gnt_size_done) 
				{
					calGntSizeSumTcontSel.Raw = IO_GREG(CAL_GNT_SIZE_SUM_TCONT_SEL);
					calGntSizeSumTcontSel.Bits.cal_gnt_size_sum_tcont_sel = tcont_sel;
					IO_SREG(CAL_GNT_SIZE_SUM_TCONT_SEL, calGntSizeSumTcontSel.Raw);

					calGntSizeSumL32.Raw = IO_GREG(CAL_GNT_SIZE_SUM_L32);
					calGntSizeSumH32.Raw = IO_GREG(CAL_GNT_SIZE_SUM_H32);	
					
					printk("Calculate grant size sum_h32 = %.8x, sum_l32 = %.8x\n", calGntSizeSumH32.Raw, calGntSizeSumL32.Raw);
					printk("Calculate grant size done times: %d\n",times);
					break ;
				} 
			}
		}
		else if(!strcmp(subcmd, "tod_spf"))
		{
			XMCS_GponTodCfg_t debug_tod;
			REG_DBG_DS_SPF_CNT_L dsSpfCntL = {0}; 

			dsSpfCntL.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
			
			debug_tod.superframe = dsSpfCntL.Raw + action;
			debug_tod.sec_H16 = 0;
			debug_tod.sec = 0x54dd8005;
			debug_tod.nanosec = 0x1185453f;			
			
			gponDevSetNewTod(debug_tod);
		}
	}
#if defined(TCSUPPORT_CPU_AN7583)
	else if(!strcmp(cmd, "tx_idle_bcnt")) {
		if(!strcmp(subcmd, "sfc")) {
			uint times = 1;
			REG_DBG_DS_SPF_CNT_L spfAcount_start = {0}, spfAcount_stop = {0};
			REG_TX_IDLE_BCNT txIdleBcnt_start = {0}, txIdleBcnt_stop = {0};

			spfAcount_start.Raw = IO_GREG(DBG_DS_SPF_CNT_L);	
			txIdleBcnt_start.Raw = IO_GREG(TX_IDLE_BCNT);
			while(times++) {
				spfAcount_stop.Raw = IO_GREG(DBG_DS_SPF_CNT_L);
				if((spfAcount_start.Raw + action) == spfAcount_stop.Raw ) {
					txIdleBcnt_stop.Raw =IO_GREG(TX_IDLE_BCNT); 
					break;	
				} else if((spfAcount_start.Raw + action) < spfAcount_stop.Raw ){
					printk("current cnt invalid,please input cmd and retry\n");
					break;	
				}
			} 
			printk("Get TX_IDLE_BCNT when sfc is 0x%.8x : 0x%.8x\n",spfAcount_start.Raw,txIdleBcnt_start.Raw);
			printk("Get TX_IDLE_BCNT after %d sfc : 0x%.8x\n",action,txIdleBcnt_stop.Raw);
		}	
	}
	else if(!strcmp(cmd, "wan2wanTrafficTest")) {
		if(!strcmp(subcmd, "flag")) {
			gpGponPriv->gponCfg.Wan2WanTrafficTest = action;
			printk("Flag of Wan2WanTrafficTest is %d\n",gpGponPriv->gponCfg.Wan2WanTrafficTest);
		} else {
			printk("subcmd must be flag\n");
		}
	}
#endif
	else if(!strcmp(cmd, "tod"))
	{
		if(!strcmp(subcmd, "adjust"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &gpon_tod_adjust) ;
			printk("xpon_tod_adjust = %dns\n",gpon_tod_adjust);
		}
		else
		{
			printk("subcmd input error,do nothing");
		}
	}
#endif
	else if(!strcmp(cmd, "gem_config"))
	{
		if(!strcmp(subcmd, "hw_gem"))
		{
			printk("Before set value, 7581 hw_gem_config = %d\n", hw_gem_config);
			hw_gem_config = action;
			printk("After set value, gponDevCheckArrayGemInfo by %s, 7581 hw_gem_config  = %d\n", hw_gem_config ? "HW" : "SW", hw_gem_config); 
		}
		else if(!strcmp(subcmd, "gem_table"))
		{
			uint i = 0;
			uint times = 0 ;
			uint loop_count = 0;

			if(action <= 100000)
			{
				loop_count = action;
			}

			printk("[%s] [%d] SW config gem_table loop_count = %u\n",__FUNCTION__,__LINE__,loop_count);
			
			for(times = 0; times < loop_count; times++)
			{
				for(i = 0; i < CONFIG_GPON_10G_MAX_GEMPORT; i++)
				{
					gponDevCheckGemInfoInvalid(gpWanPriv->gpon.gemPort[i].info.portId); 
					msleep(2);
				}
				
				msleep(50);
				printk("[%s] [%d] SW config gem_table times = %u\n",__FUNCTION__,__LINE__,times);
							
			}
		}
	}
#ifdef TCSUPPORT_CPU_ARMV8_64
	else if(!strcmp(cmd, "readreg"))
	{
			uint reg ,value =0;
			reg = simple_strtoul(subcmd,NULL,16);
			value = IO_GREG(reg);
			printk("reg:0x%.8x  value:0x%.8x",reg,value);
	}
	else if(!strcmp(cmd, "writereg"))
	{
			uint reg ,value =0;
			reg = simple_strtoul(subcmd,NULL,16);
			value = action;
			IO_SREG(reg,value);
			printk("reg:0x%.8x  value:0x%.8x",reg,value);
	}
#endif
	else if(!strcmp(cmd, "csr")) {
		if(!strcmp(subcmd, "gem")) {
			gpon_dvt_gemport(action);
		}else if(!strcmp(subcmd, "gpidx")){
            gpon_index_table(action);
		}else{
            printk("wrong csr subcmd\n");
		}
	}else if(!strcmp(cmd, "rogue")) {
		if(!strcmp(subcmd, "get")) {
            PON_RogueStatus_t status;
            xmcs_get_rogue_status(&status);
            if(PON_STATUS_ROGUE ==	status)
				printk("Rogue ONU was detected!\n");
            else
				printk("No rogue ONU was detected!\n");
		}else if(!strcmp(subcmd, "clear")) {
            xmcs_clear_rogue_status();
            printk("Clear rogue ONU status!\n");
		}
	}else if(!strcmp(cmd, "gpon")) {
		if(!strcmp(subcmd, "startup")) {
			if((action == 0) || (action == 2)) { /*Disable or PowerDown*/
				xmcs_set_connection_start(action);
				printk("Disable GPON ONU\n") ;
			} else {
				xmcs_set_connection_start(action);
				printk("Enable GPON ONU,MAC %s reset\n",(action==1 ? "plain":"with PHY")) ;
			}
		}
	}else if(!strcmp(cmd, "show")) {
		 if(!strcmp(subcmd, "gem")) {
			struct XMCS_GemPortInfo_S gemInfo ;
			uint gemPortId=0, gemIdx=0 ;
			if(xmcs_get_gem_port_info(&gemInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<gemInfo.entryNum ; i++) {
					gemPortId = gemInfo.info[i].gemPortId;
					gemIdx =  (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);		
					printk("GEM ID:%-5d,Alloc ID:%-5d,If Idx:%-3d,Type:%-9s,Loopback:%-7s,rxEncrypt:%-7s,txEncrypt:%-7s,valid:%d,chn:%d\n", 
								gemInfo.info[i].gemPortId, 
								gemInfo.info[i].allocId, 
								gemInfo.info[i].aniIdx, 
								(gemInfo.info[i].gemType)?"Multicast":"Unicast",
								(gemInfo.info[i].lbMode)?"ENABLE":"DISABLE", 
								(gemInfo.info[i].enMode)?"ENABLE":"DISABLE",
								(gemInfo.info[i].txEncrypt)?"ENABLE":"DISABLE",
								gpWanPriv->gpon.gemPort[gemIdx].info.valid,
								gpWanPriv->gpon.gemPort[gemIdx].info.channel) ;
				}
			}
		}else if(!strcmp(subcmd, "tcont")) {
			struct XMCS_TcontInfo_S tcontInfo;
			memset(&tcontInfo, 0, sizeof(struct XMCS_TcontInfo_S));
			if(xmcs_get_tcont_info(&tcontInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<tcontInfo.entryNum ; i++) {
					printk("TCONT ID:%d, Channel:%d\n", tcontInfo.info[i].allocId, tcontInfo.info[i].channel) ;
				}
			}
		}else if(!strcmp(subcmd, "counter")){
			uint64_t rdata =0;
			uint64_t tdata =0;;
			for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
				if(gpWanPriv->gpon.gemPort[i].info.valid) {
					if(action == gpWanPriv->gpon.gemPort[i].info.portId || action==0x10000) {
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_RX_FRAME_CNT, &rdata) ;
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_TX_FRAME_CNT, &tdata) ;
						printk("Gem Port:%d, SW Tx:%u, SW Rx:%u, HW Tx:%llu, HW Rx:%llu\n", 
										gpWanPriv->gpon.gemPort[i].info.portId,
										(uint)gpWanPriv->gpon.gemPort[i].stats.tx_packets, 
										(uint)gpWanPriv->gpon.gemPort[i].stats.rx_packets, 
										tdata, rdata) ;
					}
					else if(action == 0x20000)
					{
						gponDevClearGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId);
						memset(&(gpWanPriv->gpon.gemPort[i].stats), 0, sizeof(gpWanPriv->gpon.gemPort[i].stats));
					}
				}
			}
		}else if(!strcmp(subcmd, "status")) {
			printk("ONU Info:\n") ;
			printk("ONU ID: 	%d\n", GPON_ONU_ID) ;
			printk("ONU State:	%d\n", GPON_CURR_STATE) ;
			printk("SN: 		") ;
			for(i=0 ; i<4 ; i++) {
				printk("%c", gpGponPriv->gponCfg.sn[i]) ;
			}
			for(i=4 ; i<8 ; i++) {
				printk("%.2x", gpGponPriv->gponCfg.sn[i]) ;
			}
		
			printk("\nO1 Timer: 	%d\n", gpGponPriv->gponCfg.to1Timer) ;
			printk("O2 Timer:	%d\n", gpGponPriv->gponCfg.to2Timer) ;
			printk("OMCC ID:	%d\n", GPON_OMCC_ID) ;

		}else if(!strcmp(subcmd, "gpon_recovery_backup")){		
			printk("------ bakGemport ------\n");			
			gponRecovery.dbgPrint = 1;
		 	for(i = 0; i < gponRecovery.gemPortNum; i++){
				 GPON_RECOVER_DBG_PRINT_GEM(i);
		 	}
		}else if(!strcmp(subcmd, "pwan_drop")) {
        	drop_print_flag = action;
		}else if(!strcmp(subcmd, "xpon_print")) {
        	xpon_mac_print_open = action;
		}else if(!strcmp(subcmd, "phyProfile")) {
			index = action;
			memset(&xgponPhyProfile,0,sizeof(xgponPhyProfile));
			gponDevGetPhyProfile(&xgponPhyProfile,index);
			printk("profile index:%d, preamble repeat count:%d, preamble length:%d, delimiter_length:%d, fec indication:%d\n",\
					xgponPhyProfile.profile_index,\
					xgponPhyProfile.preamble_repeat_count,\
					xgponPhyProfile.preamble_length,\
					xgponPhyProfile.delimiter_length,\
					xgponPhyProfile.fec_indication);
			for(i=0; i<8; i++){
				printk("profile_preamble[%d]: %.8X, profile_delimiter[%d]: %.8X\n",\
						i,xgponPhyProfile.preamble[i],\
						i,xgponPhyProfile.delimiter[i]);
			}
		}else{
            printk("wrong show subcmd\n");
		}
	}else if(!strcmp(cmd, "aesCmac")) {
		gpon_aec_cmac_test();
	}else if(!strcmp(cmd, "test")){
	    if(!strcmp(subcmd, "sleepCfg"))
	    {
            ret = xgpon_power_saving_test();
            if(0 != ret){
                printk("[%s %d]ERROR: XG-PON sleep test error.\n", __FUNCTION__, __LINE__);
            }
        }else if(!strcmp(subcmd, "regCheck")){
            tmp = action;
            if(tmp < 1000){
            ret = xgpon_register_test(tmp);
            if(0 != ret){
                printk("[%s %d]ERROR: XG-PON register check error.\n", __FUNCTION__, __LINE__);
            }else{
                printk("[%s %d]XG-PON register check finish.\n", __FUNCTION__, __LINE__);
            }
            }else
                printk("regCheck cmd check num should less than 1000\n");                        
        }else{
            printk("[%s %d]cmd error.\n", __FUNCTION__, __LINE__);
        }
	}else if(!strcmp(cmd, "hwnat")) {
		if (!strcmp(subcmd, "noclean")){
			is_hwnat_dont_clean = ((action==0)?0:1);			
			printk("hw_nat table is %s!\n", (action==0)?"Clean":"Not clean");
		}else{
            printk("wrong hwnat subcmd\n");
		}
	}else if(!strcmp(cmd, "rogue_test")) {
		if (!strcmp(subcmd, "allDisable")){
			memset(&SnMsg,0,sizeof(PLOAM_RAW_Disable_SN_T));
			SnMsg.raw.mode = PLOAM_DISABLE_DENIED_ALL;
			ploam_recv_disable_serial_number(&SnMsg);			
			printk("rogue_test disable all\n");
		}else if (!strcmp(subcmd, "allEnable")){
			memset(&SnMsg,0,sizeof(PLOAM_RAW_Disable_SN_T));
			SnMsg.raw.mode = PLAOM_DISABLE_ALLOWED_ALL;
			ploam_recv_disable_serial_number(&SnMsg);			
			printk("rogue_test enable all\n");
		}else if (!strcmp(subcmd, "specDisable")){
			memset(&SnMsg,0,sizeof(PLOAM_RAW_Disable_SN_T));
			SnMsg.raw.mode = PLOAM_DISABLE_DENIED_SPECIFIC;
            memcpy(SnMsg.raw.sn,gpGponPriv->gponCfg.sn, GPON_SN_LENS);
			ploam_recv_disable_serial_number(&SnMsg);			
			printk("rogue_test disable specific\n");
		}else if (!strcmp(subcmd, "specEnable")){
			memset(&SnMsg,0,sizeof(PLOAM_RAW_Disable_SN_T));
			SnMsg.raw.mode = PLAOM_DISABLE_ALLOWED_SPECIFIC;
            memcpy(SnMsg.raw.sn,gpGponPriv->gponCfg.sn, GPON_SN_LENS);
			ploam_recv_disable_serial_number(&SnMsg);			
			printk("rogue_test enable specific\n");
		}else{
            printk("wrong rogue_test subcmd\n");
        }
	}else if(!strcmp(cmd, "hgu")){
		if(!strcmp(subcmd, "txq")){
			gpWanPriv->gpon.hgu_mode_txq = action;
		}
		printk("Hgu mode txq is: %d %s\n", gpWanPriv->gpon.hgu_mode_txq,
			gpWanPriv->gpon.hgu_mode_txq ? "OMCI Config" : "Local Config"); 
	}else if(!strcmp(cmd, "hec3Err_test")) {
		if (!strcmp(subcmd, "clear")){
			memset(&(gpGponPriv->gponCfg.hec3errCtrl),0,sizeof(GPON_10G_HEC3ERR_T));
			printk("hec3Err_test cnt clear\n");
		}else if (!strcmp(subcmd, "ctrl")){
			gpGponPriv->gponCfg.hec3errCtrl.hec_3err_enable = ((action==0)?0:1);			
			printk("hec3Err_test is %s!\n", (action==0)?"disable":"enable");
		}else{
            printk("wrong hec3Err_test subcmd\n");
		}
    }else if(!strcmp(cmd, "bwmap")) {
		bwmapLevel = bwmap_print_flag ;
		
		if(!strcmp(subcmd, "min_bst_intvl_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MIN_bst_intvl_err) : (bwmapLevel&~MIN_bst_intvl_err) ;
		} else if(!strcmp(subcmd, "max_start_time_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_start_time_err) : (bwmapLevel&~MAX_start_time_err) ;
		} else if(!strcmp(subcmd, "start_time_order_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|START_time_order_err) : (bwmapLevel&~START_time_order_err) ;
		} else if(!strcmp(subcmd, "max_gsize_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_gsize_err) : (bwmapLevel&~MAX_gsize_err) ;
		} else if(!strcmp(subcmd, "min_gsize_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MIN_gsize_err) : (bwmapLevel&~MIN_gsize_err) ;
		} else if(!strcmp(subcmd, "my_tid_ins_bst_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MY_tid_ins_bst_err) : (bwmapLevel&~MY_tid_ins_bst_err) ;
		} else if(!strcmp(subcmd, "bst_split_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|BST_split_err) : (bwmapLevel&~BST_split_err) ;
		} else if(!strcmp(subcmd, "alloc_hec_uc_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|ALLOC_hec_uc_err) : (bwmapLevel&~ALLOC_hec_uc_err) ;
		} else if(!strcmp(subcmd, "max_bwm_alloc_num_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_bwm_alloc_num_err) : (bwmapLevel&~MAX_bwm_alloc_num_err) ;
		}else if(!strcmp(subcmd, "max_my_alloc_num_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_my_alloc_num_err) : (bwmapLevel&~MAX_my_alloc_num_err) ;
		}else if(!strcmp(subcmd, "max_my_bst_num_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_my_bst_num_err) : (bwmapLevel&~MAX_my_bst_num_err) ;
		} else if(!strcmp(subcmd, "max_bst_len_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|MAX_bst_len_err) : (bwmapLevel&~MAX_bst_len_err) ;
		} else if(!strcmp(subcmd, "other_tid_ins_my_bst_err")) {
			bwmap_print_flag = (action) ? (bwmapLevel|OTHER_tid_ins_my_bst_err) : (bwmapLevel&~OTHER_tid_ins_my_bst_err) ;
		} else if(!strcmp(subcmd, "o23_o4_data_gnt_recv")) {
			bwmap_print_flag = (action) ? (bwmapLevel|O23_o4_data_gnt_recv) : (bwmapLevel&~O23_o4_data_gnt_recv) ;
		}else{
            printk("wrong bwmap subcmd\n");
		}
		printk("Bwmap Debug Level: %x\n", bwmap_print_flag) ; 
	}
#ifdef CONFIG_USE_FOR_TEST
	else if(!strcmp(cmd, "eqdAdjust")) {
		if(!strcmp(subcmd, "debug")) {
			eqdDbgFlag = action;
		}else if(!strcmp(subcmd, "show")) {
			printk("###EqdCounter == %d###\n", EqdCounter);
			for(i=0; i<=EqdCounter; i++)
				printk("EqdArray[%d] == %d\n", i,EqdArray[i]);
		}else if (!strcmp(subcmd, "empty")){
			EqdCounter = 0;
			memset(EqdArray,0,1001*sizeof(uint));
		}else{
            printk("wrong eqdAdjust subcmd\n");
        }
	}
#endif /* CONFIG_USE_FOR_TEST */
	else if(!strcmp(cmd, "gpon_recovery"))
	{
		if(!strcmp(subcmd, "1")){
			gponRecovery.dbgPrint = 1;
		}else{
			gponRecovery.dbgPrint = 0;
		}		
		printk("recover Debug Level: %x\n",gponRecovery.dbgPrint) ; 
	}else if(!strcmp(cmd, "sniffer"))	{
		GPON_10G_DEV_SNIFFER_MODE_T sniffer;
		memset(&sniffer,0,sizeof(GPON_10G_DEV_SNIFFER_MODE_T));
		sniffer.tx_da =         0x0001;
	    sniffer.tx_sa =         0x0002;	    
	    sniffer.rx_da =         0x0011;
	    sniffer.rx_sa =         0x0012;
	    sniffer.ethertype =     0x88b6;
	    sniffer.gemPortId =     0xffff;
		sniffer.ds_enable =     XPON_ENABLE;
	    sniffer.us_enable =     XPON_ENABLE;
		if(!strcmp(subcmd, "allport")){
			sniffer.tpid=			0xf;
			sniffer.lan_port =		 0xf;
		}else if(!strcmp(subcmd, "lanport")){
			sniffer.lan_port =		action;			
			sniffer.tpid=			sniffer.lan_port;
		}else if(!strcmp(subcmd, "disable")){
			sniffer.ds_enable =     XPON_DISABLE;
	    	sniffer.us_enable =     XPON_DISABLE;			
			sniffer.lan_port =		0xf;			
		}
		FE_API_SET_PACKET_LENGTH(FE_GDM_SEL_GDMA2,2000,32);
		if(gponDevSetSniffMode(&sniffer) !=0){
			printk("fail to start sniffer \n") ; 
		}else{
			printk("Start capture sniffer on port %x,us:%d ds:%d\n",sniffer.lan_port,sniffer.us_enable,sniffer.ds_enable) ; 
		}
	}
	else if(!strcmp(cmd, "rm_tcont_delay") )
	{
		g_rm_tcont_delay_us = simple_strtoul(subcmd, &endpo, 10);
		printk("g_rm_tcont_delay_us = %d\n",g_rm_tcont_delay_us);
	}else if(!strcmp(cmd, "omciIkIdxExchange")){
		if(!strcmp(subcmd,"enable"))
			omciIkIdxExchange = 1;
		else
			omciIkIdxExchange = 0;
		printk("Exchange OMCI key index in tx DSCP %s\n", omciIkIdxExchange?"enable":"disable");
	}else if(!strcmp(cmd,"dropCpuTxPkts")){
		if(!strcmp(subcmd,"enable")){
			dropCpuTxPktsFlag = 1;
		}else{
			dropCpuTxPktsFlag = 0;
		}

		printk("Drop CPU tx packets: %s\n",dropCpuTxPktsFlag?"enable":"disable");
	}else if(!strcmp(cmd,"swResync")){
		if(gponDevSwResyncCompleteProcess() !=0){
			printk("Sw resync failed!\n");
		}else{
			printk("Sw resync done.\n");
		}
	}else if(!strcmp(cmd, "sw_reset"))
	{
		gpon_dvt_sw_reset();
	}
    else if(!strcmp(cmd, "keyvalid")) 
    {
        if(!strcmp(subcmd, "show")){
            printk("Now key valid flag is %s \n", ((1 == gpGponPriv->gponSecurity.txKeyValid)?"VALID":"INVALID")) ;
            printk("This flag is used to set AES Tx Key to invalid, even if OLT is set the AES TX Key to valid. \n") ;
        }else if(!strcmp(subcmd, "set")) {
            if(0 != action){
                action = 1;
            }

            if(action ==  gpGponPriv->gponSecurity.txKeyValid){
                printk("Now key valid flag is %s \n", ((1 == gpGponPriv->gponSecurity.txKeyValid)?"VALID":"INVALID")) ;
            }else{
                gpGponPriv->gponSecurity.txKeyValid = action;
                if(0 == action ){
                    gponDevSetAesTxKeyInvalid();
                }
                /* 1 == action no need do anything, OLT wil set AesTx Key to valid */
                printk("Now key valid flag is %s \n", ((1 == gpGponPriv->gponSecurity.txKeyValid)?"VALID":"INVALID")) ;
            }
        }else{
            printk("wrong keyvalid subcmd\n");
        }
    }
	else if(!strcmp(cmd, "ng2"))
	{
		if(!strcmp(subcmd, "ignore_disable")){
			ng2_ignore_disable =  action;
			printk("ng2_ignore_disable = %d\n",ng2_ignore_disable);
		}
		else if(!strcmp(subcmd, "man_set_o9"))
		{
			ng2_man_set_09 = action;
			printk("ng2_man_set_09 = %d\n",ng2_man_set_09) ;
		}
		else if(!strcmp(subcmd, "set_state"))
		{
			ng2_set_state = action;
			gpon_act_change_state(ng2_set_state);
			printk("ng2_set_state = %d\n",ng2_set_state) ;
		}
		else if(!strcmp(subcmd, "noRollback"))
		{
			ng2_no_rollback = action;
			printk("ng2_no_rollback = %d\n",ng2_no_rollback) ;
		}
		else if(!strcmp(subcmd, "8to5"))
		{
			ng2_o8_to_05 = action;
			printk("ng2_o8_to_05 = %d\n",ng2_o8_to_05) ;
		}
		else if(!strcmp(subcmd, "4to9"))
		{
			ng2_o4_to_09 = action;
			//disable 09_recv int if enabled 4to9
			gponIntEnable.Raw = IO_GREG(INT_ENABLE);
			gponIntEnable.Bits.o9_gnt_recv_int_en = ng2_o4_to_09 == 1 ? 0 : 1;
			IO_SREG(INT_ENABLE, gponIntEnable.Raw);
			printk("ng2_o4_to_09 = %d\n",ng2_o4_to_09) ;
		}
		else if(!strcmp(subcmd, "tun_resp_key"))
		{
			ng2_tun_resp_key = action;
			printk("ng2_tun_resp_key = %d\n",ng2_tun_resp_key) ;
		}
		else if(!strcmp(subcmd, "mon_not_gnt"))
		{
			ng2_mon_not_gnt = action;
			printk("ng2_mon_not_gnt = %d\n",ng2_mon_not_gnt) ;
		}
		else if(!strcmp(subcmd, "tun_resp"))
		{
			ng2_o9_recv_grant_handler();
			printk("send tuning respon \n") ;
		}
		else{
			printk("invalid command!\n") ;
		}

    }
else if(!strcmp(cmd, "fec")){
		if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGPON)&&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGSPON) )
      		return 0;
    	if(!strcmp(subcmd, "on")){
			gpGponPriv->gponCfg.dsFecMode = GPON_10G_DS_FEC_FORCE_ON;
                        XPON_PHY_SET_RX_FEC(DS_FEC_SETTING_FORCE_ON);
	                printk("xgpon set fec force on!\n") ;
		}
		else if(!strcmp(subcmd, "off")){
			gpGponPriv->gponCfg.dsFecMode = GPON_10G_DS_FEC_FORCE_OFF;
                        XPON_PHY_SET_RX_FEC(DS_FEC_SETTING_FORCE_OFF);
                        printk("xgpon set fec force off!\n") ;     
		}
		else if(!strcmp(subcmd, "spec_set")){
			gpGponPriv->gponCfg.dsFecMode = GPON_10G_USE_DEFAULT_SET_AS_SEPC;
                        XPON_PHY_SET_RX_FEC(DS_FEC_SETTING_AS_SPEC);
			printk("xgpon set fec accodring to spec!\n") ;
		}
		else if(!strcmp(subcmd, "force_oc")){
			gpGponPriv->gponCfg.dsFecMode = GPON_10G_USE_OCBODY_FIELD;
                        XPON_PHY_SET_RX_FEC(DS_FEC_SETTING_FORCE_OC);
			printk("xgpon set fec force ref to ocbody!\n") ;
		}else if(!strcmp(subcmd, "show")){
			printk("current ds fec working mode is mode %d !\n",gpGponPriv->gponCfg.dsFecMode) ;
		}
	}
	else if(!strcmp(cmd, "mbi_tx_stop")){
		if(!strcmp(subcmd, "enable")){
			gpGponPriv->gponCfg.MbiTxStopEn = XPON_ENABLE;
		}else if(!strcmp(subcmd, "disable")){
			gpGponPriv->gponCfg.MbiTxStopEn = XPON_DISABLE;
		}
		printk("current MBI tx stop enable flag is  %d !\n",gpGponPriv->gponCfg.MbiTxStopEn) ;
	}
	else if(!strcmp(cmd, "alarm"))
	{
		if(!strcmp(subcmd, "keep_time"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &action) ; //Decimal easy to use
			gpGponPriv->gponAlmKeepTime = action;
			if(gpGponPriv->gponAlmKeepTime == 0){
				gpon_reset_alarm_jiffiesClk();
			}
		}
		else if(!strcmp(subcmd, "clear_all"))
		{
			gpon_clear_all_alarm();
		}
		else{
			printk("invalue command!\noptional command: keep_time [TIME]; clear_all\n");
			printk("Note: if want to see the alarm history, i.e. no any clear action, set the [TIME] as 0\n");
		}

		printk("cur alarm keep_time = %d\n",gpGponPriv->gponAlmKeepTime);
	}
	else if(!strcmp(cmd, "set_alarm") || !strcmp(cmd, "clear_alarm"))
	{
		gpon_alarm_proc_write(cmd, subcmd);
	}else if(!strcmp(cmd, "RDK_GTC")){

        rdkb_write_gtc_count(subcmd,action);
	}
	else if(!strcmp(cmd, "asb_ploam"))
	{
		PLOAM_RAW_General_T msg;
		memset(&msg, 0, sizeof(PLOAM_RAW_General_T));
		msg.value[0] = 0x00770000;
		msg.value[1] = 0x65626565;
		msg.value[2] = 0x73745f72;
		msg.value[3] = 0x00003f74;
		ploam_parser_down_message(&msg);
    }
	else if(!strcmp(cmd, "RDK_MIC"))
	{
        rdk_mic_err_dbg.proc_micErr_flag = 1;
        if(!strcmp(subcmd, "omci")){
            rdk_mic_err_dbg.OmciMicErrCount = action;
        }else if(!strcmp(subcmd, "ploam")){
            rdk_mic_err_dbg.PloamMicErrCount = action;
        }
    }
    else if(!strcmp(cmd, "mic_err_ctrl"))
    {
        printk("Old DownStream OMCI MIC error ctl is [%s] mode\n",(XGPON_HW == gpGponPriv->gponCfg.dsOmciMicCtrl)?"HW":"SW");
        if(!strcmp(subcmd, "HW")){
			gpGponPriv->gponCfg.dsOmciMicCtrl = XGPON_HW;
		}else if(!strcmp(subcmd, "SW")){
			gpGponPriv->gponCfg.dsOmciMicCtrl = XGPON_SW;
    	}else{
            printk("Invalid command, usage: echo mic_err_ctrl [HW|SW] > /proc/xgpon/debug \n");
        }
        printk("New DownStream OMCI MIC error ctl is [%s] mode\n",(XGPON_HW == gpGponPriv->gponCfg.dsOmciMicCtrl)?"HW":"SW");
     }
    else if(!strcmp(cmd, "phy_ready_set")){
        PHY_Event_Source_t src = 0;
        gpon_phy_ready_handler(src);
    }
	else if(!strcmp(cmd, "distance_adjust"))
	{
		if(!strcmp(subcmd, "add"))
		{
			distanceAdjust = action;
			printk("add distanceAdjust = %d\r\n",distanceAdjust);
		}else if(!strcmp(subcmd, "del")){
			distanceAdjust = -action;
			printk("delete distanceAdjust = %d\r\n",distanceAdjust);
		}else if(!strcmp(subcmd, "show")){
			uint32_t olt_distance = 0;
			xmcs_get_olt_distance(&olt_distance);
			printk("onu cal olt distance = %d\n", olt_distance);
		}else{
			printk("distanceAdjust = %d\r\n",distanceAdjust);
		}
	}
	else if(!strcmp(cmd, "wait_queue")){
		if(!strcmp(subcmd, "stop")){
			gpMcsPriv->xmcsEventStatus.items = 1;
		    wake_up_interruptible(&gpMcsPriv->xmcsWaitQueue) ;
		}
    }
	else if(!strcmp(cmd, "hw_cnt"))
	{
		if(!strcmp(subcmd, "enable"))
		{
			hw_cnt_enable = action;
			printk("hw_cnt_enable = %d\r\n",hw_cnt_enable);
		}
	}
	else{
		printk("invalid command!\n") ;
	}		
	return count;
}
/******************************************************************************
******************************************************************************/
static int rdkb_write_gtc_count(char * subcmd,uint action)
{
    rdk_gtc_dbg.proc_flag = 1;
    if(!strcmp(subcmd, "corrBytes")){
        rdk_gtc_dbg.fecCounter.corrBytes = action;
    }else if(!strcmp(subcmd, "corrCodeWords")){
        rdk_gtc_dbg.fecCounter.corrCodeWords = action;
    }else if(!strcmp(subcmd, "unCorrCodeWords")){
        rdk_gtc_dbg.fecCounter.unCorrCodeWords = action;
    }else if(!strcmp(subcmd, "totalRxCodeWords")){
        rdk_gtc_dbg.fecCounter.totalRxCodeWords = action;

    }else if(!strcmp(subcmd, "XGTCHECErrCount")){
        rdk_gtc_dbg.hecCounter.XGTCHECErrCount = action;
    }else if(!strcmp(subcmd, "PSBdHECErrCount")){
        rdk_gtc_dbg.hecCounter.PSBdHECErrCount = action;
    }else if(!strcmp(subcmd, "XGEMHECErrCount")){
        rdk_gtc_dbg.hecCounter.XGEMHECErrCount = action;

    }else if(!strcmp(subcmd, "XGEMHECLostWordCount")){
        rdk_gtc_dbg.hecCounter.XGEMHECLostWordCount = action;
    }else{
		printk("invalid command!\n") ;
		return 0;
	}

    return 0;
}

/*****************************************************************************
******************************************************************************/
static int	gpon_alarm_proc_write(char *cmd, char *subcmd)
{
	ALARM_INDEX_T index;
	if(!strcmp(subcmd, "RDI"))
		index = RDI_INDEX;
	else if(!strcmp(subcmd, "PEE"))
		index = PEE_INDEX;
	else if(!strcmp(subcmd, "LOS"))
		index = LOS_INDEX;
	else if(!strcmp(subcmd, "LOF"))
		index = LOF_INDEX;
	else if(!strcmp(subcmd, "DIS"))
		index = DIS_INDEX;
	else if(!strcmp(subcmd, "DACT"))
		index = DACT_INDEX;
	else if(!strcmp(subcmd, "MIS"))
		index = MIS_INDEX;
	else if(!strcmp(subcmd, "MEM"))
		index = MEM_INDEX;
	else if(!strcmp(subcmd, "SUF"))
		index = SUF_INDEX;
	else if(!strcmp(subcmd, "SF"))
		index = SF_INDEX;
	else if(!strcmp(subcmd, "SD"))
		index = SD_INDEX;
	else if(!strcmp(subcmd, "LCDG"))
		index = LCDG_INDEX;
	else if(!strcmp(subcmd, "TF"))
		index = TF_INDEX;
	else if(!strcmp(subcmd, "ROGUE"))
		index = ROGUE_INDEX;
	else if(!strcmp(subcmd, "LODS"))
		index = LODS_INDEX;
	else if(!strcmp(subcmd, "MAX"))
		index = MAX_INDEX;
	else{
		printk("invalid command!\n") ;
		return 0;
	}

	if(!strcmp(cmd, "set_alarm"))
		gpon_set_alarmBit(index);
	else
		gpon_clear_alarmBit(index);

	return 0;
}

/*****************************************************************************
******************************************************************************/
static int	gpon_emulate_proc_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	
    printk("\r\n xgpon_emulate_proc_read\n");
	
	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
/******************************************************************************
******************************************************************************/
static int gpon_emulate_proc_write(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[140]={0},cmd[32]={0};
	uint ploamData[13]={0};
	uint i=0;	
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ};
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	sscanf(val_string,"%31s %x %x %x %x %x %x %x %x %x %x %x %x %x",cmd, &ploamData[0], &ploamData[1], &ploamData[2],&ploamData[3],&ploamData[4],
		&ploamData[5],&ploamData[6],&ploamData[7],&ploamData[8],&ploamData[9],&ploamData[10],&ploamData[11],&ploamData[12]);

	if(!strcmp(cmd, "phyready")){
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		XGPON_MAC_EVENT_HANDLER(&phy_event);
        
	}else if(!strcmp(cmd, "phyloss")){
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		XGPON_MAC_EVENT_HANDLER(&phy_event);
	}else if(!strcmp(cmd, "ploam")){
		for(i=0;i<13;i++){
			enQueue(ploam_test_queue,ploamData[i]);
		}
		intStatus.Bits.ploamd_recv_int = 1;
		intEnable.Bits.ploamd_recv_int_en = 1;
	}else{
        printk("wrong cmd\n");
    }
	return count;
}
/******************************************************************************
******************************************************************************/
static int gpon_crypto_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	printk("Debug Level: %x\n", gpPonSysData->debugLevel) ;
	return 0;
}
/******************************************************************************
******************************************************************************/
static int gpon_crypto_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64]={0}, cmd[32]={0} ;

	unchar sk_string[] ={"SessionK"};
	unchar ploam_string[]= {"PLOAMIntegrtyKey"};
	unchar omci_string[]= {"OMCIIntegrityKey"};
	unchar kek_string[]= {"KeyEncryptionKey"};	

	unchar defaultKey[16] ={0x55,0x55,0x55,0x55, 0x55,0x55,0x55,0x55, 0x55,0x55,0x55,0x55, 0x55,0x55,0x55,0x55};
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;
	
	sscanf(val_string, "%31s", cmd) ;

	if(!strcmp(cmd, "goldenkeys")) {
		unchar golden_msk[16] ={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00};	
		unchar golden_onu_sn[8] ={0x56,0x4e,0x44,0x52,0x00,0x11,0x22,0x33};
		unchar golden_pon_tag[8] ={0x4f,0x4c,0x54,0x23,0x44,0x55,0x66,0x77};
		unchar golden_sk_msg[24]={0x56,0x4e,0x44,0x52,0x00,0x11,0x22,0x33,  0x4f,0x4c,0x54,0x23,0x44,0x55,0x66,0x77,
				0x53,0x65,0x73,0x73,0x69,0x6f,0x6e,0x4b};
		unchar golden_sk[16] = {0};
		unchar golden_ploam_ik[16] = {0};
		unchar golden_omci_ik[16] = {0};
		unchar golden_kek[16] = {0};
		
		printk("sw calculate keys based on Gloden vectors IV6\n");
		XPON_ARR_PRINT(MSG_DBG,"golden MSK_128: ","%x ",golden_msk,16);
		XPON_ARR_PRINT(MSG_DBG,"golden PON-TAG: ","%x ",golden_pon_tag,8);
		XPON_ARR_PRINT(MSG_DBG,"golden ONU SN: ","%x ",golden_onu_sn,8);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, golden_msk, golden_sk_msg ,24, golden_sk);
		XPON_ARR_PRINT(MSG_DBG,"golden SK: ","%x ",golden_sk,16);
		
 		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, golden_sk, ploam_string ,16, golden_ploam_ik);
		XPON_ARR_PRINT(MSG_DBG,"golden PLOAM_IK: ","%x ",golden_ploam_ik,16);
		 
 		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, golden_sk, omci_string ,16, golden_omci_ik);
		XPON_ARR_PRINT(MSG_DBG,"golden OMCI_IK: ","%x ",golden_omci_ik,16);
		 
 		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, golden_sk, kek_string ,16, golden_kek);
		XPON_ARR_PRINT(MSG_DBG,"golden KEK: ","%x ",golden_kek,16);
	}
	else if(!strcmp(cmd, "key")) {	
		unchar msk1[16] = {0};
		unchar sk1[16] = {0};
		unchar omciIk1[16] = {0};
		unchar kek1[16] = {0};
		unchar defaultRegId[36] ={0};
		unchar msk0[16] = {0};
		unchar sk0[16] = {0};
		unchar ploamIk0[16] = {0};
		unchar omciIk0[16] = {0};
		unchar kek0[16] = {0};		
		unchar sk_msg[24]={0};
	
		memcpy(sk_msg, gpGponPriv->gponCfg.sn, 8);
		memcpy(&sk_msg[8], gpGponPriv->gponCfg.ponTag, 8);
		memcpy(&sk_msg[16], sk_string, 8);		
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, defaultKey, defaultRegId ,36, msk1);
		XPON_ARR_PRINT(MSG_DBG,"msk1: ","%x ",msk1,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, msk1, sk_msg ,24, sk1);
		XPON_ARR_PRINT(MSG_DBG,"sk1: ","%x ",sk1,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, sk1, omci_string ,16, omciIk1);
		XPON_ARR_PRINT(MSG_DBG,"omci1: ","%x ",omciIk1,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, sk1, kek_string ,16, kek1);
		XPON_ARR_PRINT(MSG_DBG,"kek1: ","%x ",kek1,16);		
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, defaultKey, gpGponPriv->gponCfg.reg_id ,36, msk0);
		XPON_ARR_PRINT(MSG_DBG,"msk0: ","%x ",msk0,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, msk0, sk_msg ,24, sk0);
		XPON_ARR_PRINT(MSG_DBG,"sk0: ","%x ",sk0,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, sk0, omci_string ,16, omciIk0);
		XPON_ARR_PRINT(MSG_DBG,"omci0: ","%x ",omciIk0,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, sk0, ploam_string ,16, ploamIk0);
		XPON_ARR_PRINT(MSG_DBG,"ploam0: ","%x ",ploamIk0,16);
		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, sk0, kek_string ,16, kek0);
		XPON_ARR_PRINT(MSG_DBG,"kek0: ","%x ",kek0,16);		
	}
	else if(!strcmp(cmd, "goldendatakey")){
		unchar golden_data_encryption_key[16] ={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00};
		unchar golden_data_encryption_key_msg[32] ={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00,
			0x33,0x31,0x34,0x31,0x35,0x39,0x32,0x36,0x35,0x33,0x35,0x38,0x39,0x37,0x39,0x33};
		unchar golden_KEK[16] ={0x6f,0x9c,0x99,0xb8,0x36,0x17,0x68,0x93,0x7e,0x45,0x3b,0x16,0x5f,0x60,0x97,0x10};
		unchar golden_ecb[16]={0};
		unchar golden_cmac[16]={0};
		gpon_aes_ecb_encrypt(gpGponPriv->gponSecurity.aesEcbTfm,golden_KEK,golden_data_encryption_key,16,golden_ecb);		
		gpon_aes_cmac_encrypt(gpGponPriv->gponSecurity.tfm, golden_KEK, golden_data_encryption_key_msg ,32, golden_cmac);
		printk("sw calculate keys based on Gloden vectors IV9\n");
		XPON_ARR_PRINT(MSG_DBG,"golden data_encryption_key: ","%x ",golden_data_encryption_key,16);
		XPON_ARR_PRINT(MSG_DBG,"golden KEK: ","%x ",golden_KEK,16);
		XPON_ARR_PRINT(MSG_DBG,"golden AES-ECB: ","%x ",golden_ecb,16);
		XPON_ARR_PRINT(MSG_DBG,"golden AES-CMAC: ","%x ",golden_cmac,16);
	}
	else if(!strcmp(cmd, "IV7")){
        gponGoldenDsPloamMicCheck(GPON_CMAC_IDX0);
        gponGoldenDsPloamMicCheck(GPON_CMAC_IDX1);
    }
	else if(!strcmp(cmd, "IV8")){
        gponGoldenUsPloamMicCheck(GPON_CMAC_IDX0);
        gponGoldenUsPloamMicCheck(GPON_CMAC_IDX1);
    }
	else if(!strcmp(cmd, "IV9")){
        gponGoldenKeyReportingCheck(GPON_CMAC_IDX0);
        gponGoldenKeyReportingCheck(GPON_CMAC_IDX1);
    }
	else if(!strcmp(cmd, "IV10")){
        gponGoldenDsOmciMicCheck(GPON_CMAC_IDX0);
        gponGoldenDsOmciMicCheck(GPON_CMAC_IDX1);
    }
	else if(!strcmp(cmd, "IV11")){
        gponGoldenNormalEncryptCheck(GPON_CMAC_IDX0);
        gponGoldenNormalEncryptCheck(GPON_CMAC_IDX1);
    }
	
	return count;
}
/******************************************************************************
******************************************************************************/
static int gpon_normal_counter_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	GPON_DEV_NORMAL_COUNTER_T counter;

	gponDevGetNormalCounter(&counter);

	printk("rx_mpi_sof_cnt:   %.8x         tx_mpi_sof_cnt:   %.8x\n", counter.rx_mpi_sof_cnt,counter.tx_mpi_sof_cnt) ;
	printk("rx_ploamd_cnt:    %.8x         tx_ploamu_cnt:    %.8x\n", counter.rx_ploamd_cnt, counter.tx_ploamu_cnt) ;
	printk("rx_omci_cnt_fe:   %.8x         tx_omci_cnt_fe:   %.8x\n", counter.rx_omci_cnt_fe,counter.tx_omci_cnt_fe) ;
	printk("rx_omci_cnt_mac:  %.8x         tx_omci_cnt_mac:  %.8x\n", counter.rx_omci_cnt_mac,counter.tx_omci_cnt_mac) ;
	printk("rx_xgem_cnt:      %.8x         tx_xgem_cnt:      %.8x\n", counter.rx_xgem_cnt, counter.tx_xgem_cnt) ;
	printk("rx_mbi_ack_cnt:   %.8x         tx_mbi_ack_cnt:   %.8x\n", counter.rx_mbi_ack_cnt, counter.tx_mbi_ack_cnt) ;
	printk("rx_non_idle_bcnt: %.8x         tx_non_idle_bcnt: %.8x\n", counter.rx_non_idle_bcnt,counter.tx_non_idle_bcnt) ;
	printk("rx_bip_protect_wcnt: %.8x      tx_nlf_xgem_cnt:  %.8x\n", counter.rx_bip_protect_wcnt,counter.tx_nlf_xgem_cnt) ;
	printk("                               tx_ack_ploamu_cnt:%.8x\n", counter.tx_ack_ploamu_cnt) ;
	printk("                               tx_idle_bcnt:     %.8x\n", counter.tx_idle_bcnt) ;
		
	return 0;
}
/******************************************************************************
******************************************************************************/
static int gpon_normal_counter_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[40]={0};
	int cmd = 0;
	GPON_DEV_NORMAL_COUNTER_T counter;	

	memset(&counter, 0x0, sizeof(GPON_DEV_NORMAL_COUNTER_T));
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';
	
	if(cmd >= 1){
		counter.clear = GPON_NORMAL_COUNTER_CLEAR;
		gponDevSetCounterClear(&counter);
	}

	return count;
}

/******************************************************************************
******************************************************************************/
static int gpon_err_counter_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	GPON_DEV_NORMAL_COUNTER_T counter;	 	
	
	memset(&counter, 0x0, sizeof(GPON_DEV_NORMAL_COUNTER_T));

	gponDevGetErrCounter(&counter);

	printk("rx_hlend_hec_1err_cnt:   %.8x\n", counter.rx_hlend_hec_1err_cnt) ;
	printk("rx_hlend_hec_2err_cnt:   %.8x\n", counter.rx_hlend_hec_2err_cnt) ;
	printk("rx_hlend_hec_3err_cnt:   %.8x\n", counter.rx_hlend_hec_3err_cnt) ;	
	printk("rx_alloc_hec_1err_cnt:   %.8x\n", counter.rx_alloc_hec_1err_cnt) ;
	printk("rx_alloc_hec_2err_cnt:   %.8x\n", counter.rx_alloc_hec_2err_cnt) ;
	printk("rx_alloc_hec_3err_cnt:   %.8x\n", counter.rx_alloc_hec_3err_cnt) ;	
    printk("rx_hdr_hec_1err_cnt:     %.8x\n", counter.rx_hdr_hec_1err_cnt) ;
	printk("rx_hdr_hec_1err_cnt:     %.8x\n", counter.rx_hdr_hec_2err_cnt) ;
	printk("rx_hdr_hec_1err_cnt:     %.8x\n", counter.rx_hdr_hec_3err_cnt) ;    
	printk("rx_pon_id_hec_err_cnt:   %.8x\n", counter.rx_pon_id_hec_err_cnt) ;	
	printk("rx_sfc_hec_err_cnt:      %.8x\n", counter.rx_sfc_hec_err_cnt) ;		
	printk("rx_omci_mic_err_cnt:     %.8x\n", (counter.rx_omci_mic_err_cnt+omciMicErrSwCnt)) ;
	printk("rx_ploam_mic_err_cnt:    %.8x\n", counter.rx_ploam_mic_err_cnt) ;		
	printk("rx_eth_crc_err_cnt:      %.8x\n", counter.rx_eth_crc_err_cnt) ;	
	printk("rx_bip_err_cnt:          %.8x\n", counter.rx_bip_err_cnt) ;	
	printk("rx_key_err_cnt:          %.8x\n", counter.rx_key_err_cnt) ;	
	printk("rx_lost_wcnt:            %.8x\n", counter.rx_lost_wcnt) ;
	printk("invld_prof_bst_gnt_cnt:  %.8x\n", counter.invld_prof_bst_gnt_cnt) ;
	printk("rx_mbi_xgem_drop_cnt:    %.8x\n", counter.rx_mbi_xgem_drop_cnt) ;

	return 0;
}
/******************************************************************************
******************************************************************************/
static int gpon_err_counter_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[40]={0};
    int cmd = 0;
	GPON_DEV_NORMAL_COUNTER_T counter;
	
	memset(&counter, 0x0, sizeof(GPON_DEV_NORMAL_COUNTER_T));
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';

	if(cmd == 1){
		counter.clear = GPON_ERR_COUNTER_CLEAR;
		gponDevSetCounterClear(&counter);
        omciMicErrSwCnt = 0;
	}
	return count;
}

/******************************************************************************
******************************************************************************/
static int gpon_err_status_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	GPON_DEV_ERR_STATUS_T status;

	memset(&status, 0x0, sizeof(GPON_DEV_ERR_STATUS_T));
	memcpy(&status, &gpGponPriv->gponCfg.intErrStatus, sizeof(GPON_DEV_ERR_STATUS_T)) ;

	printk("Fifo error status:%.8x\n",status.gponFifoErrSts.Raw);
	printk("tx_aes_rdm_ciph_fifo_ovrn: %d    |   Tx error status:%.8x\n", status.gponFifoErrSts.Bits.tx_aes_rdm_ciph_fifo_ovrn,status.gponTxErrSts.Raw) ;
	printk("byte_mib_cmd_fifo_ovrn:    %d    |   tx_prof_invld_err:     %d\n", status.gponFifoErrSts.Bits.byte_mib_cmd_fifo_ovrn,status.gponTxErrSts.Bits.tx_prof_invld_err) ;
	printk("frm_mib_cmd_fifo_ovrn:     %d    |   tx_late_start_err:     %d\n", status.gponFifoErrSts.Bits.frm_mib_cmd_fifo_ovrn,status.gponTxErrSts.Bits.tx_late_start_err) ;	
	printk("mib_cmd_fifo_ovrn:         %d    |   tx_bst_sgl_diff_err:   %d\n", status.gponFifoErrSts.Bits.mib_cmd_fifo_ovrn,status.gponTxErrSts.Bits.tx_bst_sgl_diff_err) ;
	printk("mib_tx_cmd_fifo_ovrn:      %d    |\n", status.gponFifoErrSts.Bits.mib_tx_cmd_fifo_ovrn) ;
	printk("snf_ds_fifo_ovrn:          %d    |   Rx error status:%.8x\n", status.gponFifoErrSts.Bits.snf_ds_fifo_ovrn,status.gponRxErrSts.Raw) ;	
	printk("snf_us_fifo_ovrn:          %d    |   rx_omci_mic_err:       %d\n", status.gponFifoErrSts.Bits.snf_us_fifo_ovrn,status.gponRxErrSts.Bits.rx_omci_mic_err) ;	
	printk("rx_ploamd_fifo_udrn:       %d    |   rx_ploam_mic_err:      %d\n", status.gponFifoErrSts.Bits.rx_ploamd_fifo_udrn,status.gponRxErrSts.Bits.rx_ploam_mic_err) ;		
	printk("rx_ploamd_fifo_ovrn:       %d    |   rx_eth_crc_err:        %d\n", status.gponFifoErrSts.Bits.rx_ploamd_fifo_ovrn,status.gponRxErrSts.Bits.rx_eth_crc_err) ;
	printk("rx_mbi_pl_fifo_ovrn:       %d    |   rx_aes_key_err:        %d\n", status.gponFifoErrSts.Bits.rx_mbi_pl_fifo_ovrn,status.gponRxErrSts.Bits.rx_aes_key_err) ;		
	printk("rx_mbi_hdr_fifo_ovrn:      %d    |   rx_gem_intlv_err:      %d\n", status.gponFifoErrSts.Bits.rx_mbi_hdr_fifo_ovrn,status.gponRxErrSts.Bits.rx_gem_intlv_err) ;	
	printk("rx_aes_rdm_ciph_fifo_ovrn: %d    |   rx_los_gem_del_err:    %d\n", status.gponFifoErrSts.Bits.rx_aes_rdm_ciph_fifo_ovrn,status.gponRxErrSts.Bits.rx_los_gem_del_err) ;	
	printk("rx_aes_ciph_txt_fifo_ovrn: %d    |   rx_bip_err:            %d\n", status.gponFifoErrSts.Bits.rx_aes_ciph_txt_fifo_ovrn,status.gponRxErrSts.Bits.rx_bip_err) ;	
	printk("rx_aes_cryp_cnt_fifo_ovrn: %d    |   rx_hlend_hec_err:      %d\n", status.gponFifoErrSts.Bits.rx_aes_cryp_cnt_fifo_ovrn,status.gponRxErrSts.Bits.rx_hlend_hec_err) ;
	printk("tx_align_fifo_udrn:        %d    |   rx_pon_id_hec_err:     %d\n", status.gponFifoErrSts.Bits.tx_align_fifo_udrn,status.gponRxErrSts.Bits.rx_pon_id_hec_err) ;
	printk("tx_ploamu_fifo_ovrn:       %d    |   rx_sfc_hec_uc_err:     %d\n", status.gponFifoErrSts.Bits.tx_ploamu_fifo_ovrn,status.gponRxErrSts.Bits.rx_sfc_hec_uc_err) ;
	printk("bst_fifo_ovrn:             %d    |   rx_eof_err:            %d\n", status.gponFifoErrSts.Bits.bst_fifo_ovrn,status.gponRxErrSts.Bits.rx_eof_err) ;
	printk("sgl_fifo_ovrn:             %d    |   rx_mbi_xgem_drop_err   %d\n", status.gponFifoErrSts.Bits.sgl_fifo_ovrn,status.gponRxErrSts.Bits.rx_mbi_xgem_drop_err) ;
	printk("                                |   rx_alloc_hec_err       %d\n", status.gponRxErrSts.Bits.rx_alloc_hec_err) ;    
	printk("                                |   rx_xgem_hdr_hec_err    %d\n", status.gponRxErrSts.Bits.rx_xgem_hdr_hec_err) ;
	printk("Bwmap chk error status:%.8x       \n", status.gponbwpChkStatus.Raw);


#ifdef TCSUPPORT_CPU_EN7581
	printk("o2349_continue_gnt_recv:       %d   |   o49_no_deft_tcont_gnt_recv:  %d\n", status.gponbwpChkStatus.Bits.o2349_continue_gnt_recv,status.gponbwpChkStatus.Bits.o49_no_deft_tcont_gnt_recv) ;
	printk("o2349_no_ploamu_only_gnt_recv: %d   |   o2349_no_ploamu_gnt_recv:    %d\n", status.gponbwpChkStatus.Bits.o2349_no_ploamu_only_gnt_recv,status.gponbwpChkStatus.Bits.o2349_no_ploamu_gnt_recv ) ;
	printk("other_tid_ins_my_bst_err:      %d   |   max_bst_len_err:             %d\n", status.gponbwpChkStatus.Bits.other_tid_ins_my_bst_err,status.gponbwpChkStatus.Bits.max_bst_len_err) ;
	printk("max_my_bst_num_err:            %d   |   max_my_alloc_num_err:        %d\n", status.gponbwpChkStatus.Bits.max_my_bst_num_err,status.gponbwpChkStatus.Bits.max_my_alloc_num_err) ;
	printk("max_bst_alloc_num_err:         %d   |   max_bwm_alloc_num_err:       %d\n", status.gponbwpChkStatus.Bits.max_bst_alloc_num_err,status.gponbwpChkStatus.Bits.max_bwm_alloc_num_err) ;
	printk("alloc_hec_uc_err:              %d   |   bst_split_err:               %d\n", status.gponbwpChkStatus.Bits.alloc_hec_uc_err ,status.gponbwpChkStatus.Bits.bst_split_err) ;
	printk("my_tid_ins_bst_err:            %d   |   min_gsize_err:               %d\n", status.gponbwpChkStatus.Bits.my_tid_ins_bst_err,status.gponbwpChkStatus.Bits.min_gsize_err ) ;
	printk("max_gsize_err:                 %d   |   start_time_order_err:        %d\n", status.gponbwpChkStatus.Bits.max_gsize_err ,status.gponbwpChkStatus.Bits.start_time_order_err ) ;
	printk("max_start_time_err:            %d   |   min_bst_intvl_err:           %d\n", status.gponbwpChkStatus.Bits.max_start_time_err ,status.gponbwpChkStatus.Bits.min_bst_intvl_err ) ;
#endif


	return 0;
}
/******************************************************************************
******************************************************************************/
static int gpon_err_status_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[40]={0};
    int cmd = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';

	if(cmd == 1){
		memset(&gpGponPriv->gponCfg.intErrStatus, 0x0, sizeof(GPON_DEV_ERR_STATUS_T));
	}
	
	return count;
}
/******************************************************************************
******************************************************************************/
static int gpon_key_info_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	/*dump security info*/
    XPON_ARR_PRINT(MSG_DBG,"pontag(hex): ","%x ",gpGponPriv->gponCfg.ponTag,GPON_TAG_LENS);
	XPON_ARR_PRINT(MSG_DBG,"msk(hex): ","%x ",gpGponPriv->gponSecurity.msk,GPON_MSK_LENS);
	XPON_ARR_PRINT(MSG_DBG,"sk(hex): ","%x ",gpGponPriv->gponSecurity.sk,GPON_SK_LENS);
	
	printk("ploam ik index:%d \n",gpGponPriv->gponSecurity.ploamIkIdx);
	XPON_ARR_PRINT(MSG_DBG,"ploamIK0(hex): ","%x ",gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX0],GPON_PLOAM_IK_LENS);
	XPON_ARR_PRINT(MSG_DBG,"ploamIK1(hex): ","%x ",gpGponPriv->gponSecurity.ploamIk[GPON_PLOAM_IK_IDX1],GPON_PLOAM_IK_LENS);
	
	printk("omci ik index:%d \n",gpGponPriv->gponSecurity.omciIkIdx);
	XPON_ARR_PRINT(MSG_DBG,"omciIK0(hex): ","%x ",gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX0],GPON_OMCI_IK_LENS);
	XPON_ARR_PRINT(MSG_DBG,"omciIK1(hex): ","%x ",gpGponPriv->gponSecurity.omciIk[GPON_OMCI_IK_IDX1],GPON_OMCI_IK_LENS);

    printk("kek ik index:%d \n",gpGponPriv->gponSecurity.kekIdx);
	XPON_ARR_PRINT(MSG_DBG,"kek0(hex): ","%x ",gpGponPriv->gponSecurity.kek[GPON_KEK_IK_IDX0],GPON_KEK_LENS);
	XPON_ARR_PRINT(MSG_DBG,"kek1(hex): ","%x ",gpGponPriv->gponSecurity.kek[GPON_KEK_IK_IDX1],GPON_KEK_LENS);

	printk("aes uckey index:%d \n",gpGponPriv->gponSecurity.aesUcKeyIdx);
	XPON_ARR_PRINT(MSG_DBG,"unicast key0(hex): ","%x ",gpGponPriv->gponSecurity.aesUcKey[0],GPON_DATA_ENCRYPT_KEY_LENS);
	XPON_ARR_PRINT(MSG_DBG,"unicast key1(hex): ","%x ",gpGponPriv->gponSecurity.aesUcKey[1],GPON_DATA_ENCRYPT_KEY_LENS);

	XPON_ARR_PRINT(MSG_DBG,"broadcast key0(hex): ","%x ",gpGponPriv->gponSecurity.aesBcKey[0],GPON_DATA_ENCRYPT_KEY_LENS);
	XPON_ARR_PRINT(MSG_DBG,"broadcast key1(hex): ","%x ",gpGponPriv->gponSecurity.aesBcKey[1],GPON_DATA_ENCRYPT_KEY_LENS);
	
	return 0;
}
/******************************************************************************
******************************************************************************/
static int gpon_bbf247_flag_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	//printk("%s\n",gpPonSysData->sysBBF247 ? "Enable" : "Disable");
	int len;
    
	len = sprintf(buf, "%s\n", gpPonSysData->sysBBF247 ? "1" : "0") ;
    len -= off;
    *start = buf + off;

    if (len > count){
        len = count;
    }else{
        *eof = 1;
    }
	
    if (len < 0){
        len = 0;
   	}
	
	return len;
}

/******************************************************************************
******************************************************************************/

static int gpon_key_info_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[40]={0};
    int cmd = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';

	if(cmd == 1){
		
	}
	
	return count;
}
/*****************************************************************************
******************************************************************************/
#ifdef CONFIG_TP_IMAGE

static int gpon_linkstate_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(buf, "%d", gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON ? 1 : 0);
}

static int gpon_state_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(buf, "%d", gpGponPriv->state);
}

static int gpon_onuid_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	return sprintf(buf, "%d", gpGponPriv->gponCfg.onu_id);
}

#endif /* CONFIG_TP_IMAGE */

/*****************************************************************************
******************************************************************************/
/*****************************************************************************
******************************************************************************/

static int gpon_fast_slow_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	if (xgpon_fast_mode_flag) {
		printk("xgpon_fast_mode_flag value is %d, xgpon up stream fast mode enable\n", xgpon_fast_mode_flag);
	} else {
		printk("xgpon_fast_mode_flag value is %d, xgpon up stream fast mode disable\n", xgpon_fast_mode_flag);
	}
	return 0;
}


/*****************************************************************************
******************************************************************************/

static int gpon_fast_slow_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[96] = {0}, cmd[32]={0}, subcmd[32]={0};

	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	sscanf(val_string, "%31s %31s", cmd, subcmd) ;
	val_string[count] = '\0';
	if(!strcmp(cmd, "fast")){
		if(!strcmp(subcmd, "enable") || !strcmp(subcmd, "1")) {
			xgpon_fast_mode_flag= 1;
		} else {
			xgpon_fast_mode_flag= 0;
		}
	} else{
	
	}
	
	return count;
}

/******************************************************************************
******************************************************************************/
int xpon_proc_init(void)
{
	uint i = 0;
	struct proc_dir_entry *temp_proc = NULL;		
	
	if(gpon_proc_dir != NULL){
		return -1;
	}
	
	gpon_proc_dir = proc_mkdir("xgpon", NULL);
	if(NULL == gpon_proc_dir){		
		printk("Error while creating %s directory\n", "xgpon");
		return -1;
	}
	while(proc_gpon_cmds[i].cmd[0]!='0'){
		temp_proc = create_proc_read_entry(proc_gpon_cmds[i].cmd, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH, gpon_proc_dir, proc_gpon_cmds[i].read_proc, NULL);
		if(NULL == temp_proc){
			printk("%s : %04d >> temp_proc is NULL !\n",__FUNCTION__,__LINE__);
			return -1;
		}
		temp_proc->write_proc = proc_gpon_cmds[i].write_proc;
		i++;
	}	

	return 0;
}
/*****************************************************************************
******************************************************************************/
int xpon_proc_dest(void){

	uint i =0;
	
	while(proc_gpon_cmds[i].cmd[0]!='0')
	{
		remove_proc_entry(proc_gpon_cmds[i].cmd, gpon_proc_dir);
		i++;
	}
	
	remove_proc_entry("xgpon", NULL);
	return 0;
}

