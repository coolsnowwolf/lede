/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	epon_main.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
#include <linux/pktflow.h>
#endif
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <linux/jiffies.h>

#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>

#include "epon/epon.h"
#include "common/xpondrv.h"
#include "common/xpon_led.h"

#include <ecnt_hook/ecnt_hook_qdma.h>
#include <ecnt_hook/ecnt_hook_fe.h>

#ifdef EPON_MAC_HW_TEST
#include "epon/epon_hwtest.h"
#endif

#include "common/drv_global.h"

#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
#include "epon/fe_reg.h"	
#endif

#include "common/phy_if_wrapper.h"
#include "xmcs/xmcs_if.h"
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif

#include <linux/ktime.h>

#ifdef TCSUPPORT_CPU_ARMV8
#include <asm/div64.h>
#endif

/************************************************************************
*                      E X T E R N A L   F U N C T I O N
*************************************************************************
*/

extern long eponMacIoctl(struct file *file, unsigned int cmd, unsigned long arg);
extern void eponPhyReadyRealStart(void);
extern void set_fix_reg_list(void);
extern int eponCmdInit(void);
extern void eponCmdExit(void);
extern void eponMacTableInit(void);
extern void eponMacTableExit(void);
extern __u32 eponDebugLevel;
extern void get_register_count(XPON_DEBUG_STATISTIC *tmpReg);
extern void eponUpdateStackMpcpErrCnt(void);



/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/
atomic_t eponMacRestart_flag = ATOMIC_INIT(0);

/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

void set_fix_reg_list(void);

int eponMacOpen(struct inode *inode, struct file *filp);
void attack_protect_set(int active, int mode);

static int get_oam_swtx_count(u32 * pkt_cnt);
static void get_oam_trx_count(TIMER_FUN_PAAM arg);
static void epon_check_oamTxRx(TIMER_FUN_PAAM arg);

struct tasklet_struct eponMacMpcpDscvGateTask;
struct tasklet_struct eponStartTask;//task_struct

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

#ifdef TCSUPPORT_PON_TEST
__u32 eponDrvDbgMask = 0;
__u8 eponDrvTmpMacAddr[6] = {0};
#endif

PEPON_MAC_REGS g_EPON_MAC_BASE = NULL;
epon_t eponDrv;
u8 isSfu = 1;
spinlock_t epon_reg_lock;
int eponReadyFlag = 0;
int no_epon_mac_reset = 0;
__u32 g_epon_llid_dscv_stat[EPON_1G_MAX_LLID_NUM] = {0};

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
int randomAdjust_flag = 0;
unsigned int random_config = 0;
int minus_disable = 0;
#endif

struct timer_list eponTxPwrDwnTmr;
struct timer_list eponCheckTxRxTmr;
#define EPON_OAM_RX_DIFF 45
__u32 epon_rx_diff = EPON_OAM_RX_DIFF;
__u32 oam_rx_cnt = 0;
__u32 oam_tx_cnt = 0;
__u32 enum_rx = 0;
__u32 oam_issue_flag = 0;
__u32 auto_switch_oam_fav = 1;

extern __u8 eponStaticRptEnable;
extern __u32 staticRptData;
extern __u32 eponDefLlidMask;
extern int sniffer_flag;
extern unsigned int sniffer_config;
extern unsigned int special_tag;

extern __u8 g_silence_time;
extern __u8 RgstrAckFlag[8];
extern int rogueOnuDisableTxPowerFlag;
extern int turnOffOpticalModuel;
extern void eponPhyReadyRealStart(void);
#ifdef TCSUPPORT_CPU_ARMV8
extern int get_ethaddr(unsigned char *ethaddr, int len);
#endif

#ifdef EPON_MAC_HW_TEST
extern __u32 eponSetProbe;
extern __u32 ignoreIntMask;
#endif

extern void eponPhyTxPwrUp(TIMER_FUN_PAAM lparam);
extern void setTxOamFav(__u32 value);
extern __u32 modifyBit(__u32 rgst,__u32 p,__u32 b);

extern int eponMacTxRxEnable(void);
extern int eponMacTxRxDisable(void);
extern int eponMacTxEnable(void);
extern int eponMacTxDisable(void);

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/
static struct timer_list gEponTypeBTimer;
static txOamCnt_S g_txOamStats = {0};

/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

static int eponTod1ppsIntHandler(void){
	return 0;
}

static int eponTodUpdtIntHandler(void){
	return 0;
}

static int eponPtpMsgTxIntHandler(void){
	return 0;
}

#if 0
__u32 macIsrVlaue = 0;


void eponMacTaskHandler(__u32 Value){	
	int ret = -1;
	__u32 isrValue = macIsrVlaue;
	
	/* INT handler */

	if(isrValue == 0){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\neponIsr no INT bit!!!!!!" );
		return ;
	}
	
	if(isrValue&TIMEDRFT_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  TIMEDRFT_INT ");
		isrValue |=  TIMEDRFT_INT;
		//WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponTimeDrftIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nTIMEDRFT_INT eponTimeDrftIntHandler ret=%d", ret);
		}
	}


	if(isrValue&RPT_OVERINTVL_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  RPT_OVERINTVL_INT ");
		isrValue |=  RPT_OVERINTVL_INT;
		//WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRptOvrIntvalIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nRPT_OVERINTVL_INT eponMpcpRptOvrIntvalIntHandler ret=%d", ret);
		}
	}




	if(isrValue&MPCP_TIMEOUT_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  MPCP_TIMEOUT_INT ");
		isrValue |=  MPCP_TIMEOUT_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpTmOutIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nMPCP_TIMEOUT_INT eponMpcpTmOutIntHandler ret=%d", ret);
		}
	}




	if(isrValue&TOD_1PPS_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  TOD_1PPS_INT ");
		isrValue |=  TOD_1PPS_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponTod1ppsIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nTOD_1PPS_INT eponTod1ppsIntHandler ret=%d", ret);
		}
	}



	if(isrValue&TOD_UPDT_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  TOD_UPDT_INT ");
		isrValue |=  TOD_UPDT_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponTodUpdtIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nTOD_UPDT_INT eponTodUpdtIntHandler ret=%d", ret);
		}
	}



	if(isrValue&PTP_MSG_TX_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  PTP_MSG_TX_INT ");
		isrValue |=  PTP_MSG_TX_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponPtpMsgTxIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nPTP_MSG_TX_INT eponPtpMsgTxIntHandler ret=%d", ret);
		}
	}



	if(isrValue&GNT_BUF_OVRRUN_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  GNT_BUF_OVRRUN_INT ");
		isrValue |=  GNT_BUF_OVRRUN_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpGntOvrRunIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nGNT_BUF_OVRRUN_INT eponMpcpGntOvrRunHandler ret=%d", ret);
		}
	}



	if(isrValue&LLID7_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID7_RCV_RGST_INT ");
		isrValue |=  LLID7_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(7);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID7_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}



	if(isrValue&LLID6_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID6_RCV_RGST_INT ");
		isrValue |=  LLID6_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(6);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID6_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}



	if(isrValue&LLID5_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID5_RCV_RGST_INT ");
		isrValue |=  LLID5_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(5);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID5_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}



	if(isrValue&LLID4_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID4_RCV_RGST_INT ");
		isrValue |=  LLID4_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(4);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID4_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}


	if(isrValue&LLID3_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID3_RCV_RGST_INT ");
		isrValue |=  LLID3_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(3);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID3_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}


	if(isrValue&LLID2_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID2_RCV_RGST_INT ");
		isrValue |=  LLID2_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(2);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID2_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}


	if(isrValue&LLID1_RCV_RGST_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  LLID1_RCV_RGST_INT ");
		isrValue |=  LLID1_RCV_RGST_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(1);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID1_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}


	if(isrValue&LLID0_RCV_RGST_INT){
		
		isrValue |=  LLID0_RCV_RGST_INT;
		//WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpRgstIntHandler(0);
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID0_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	}


	if(isrValue&RCV_DSCVRY_GATE_INT){
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr  RCV_DSCVRY_GATE_INT ");
		isrValue |=  RCV_DSCVRY_GATE_INT;
		//	WRITE_REG_WORD(e_int_status , isrValue);
		ret = eponMpcpDiscvGateIntHandler();
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nRCV_DSCVRY_GATE_INT eponMpcpDiscvGateIntHandler ret=%d", ret);
		}
	}


	//clear IRQ bit
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nleave eponMacTaskHandler ret=%d", ret);
	
	return;
}
#endif

extern atomic_t mpcpTmOutCnt;
#ifdef TCSUPPORT_EPON_POWERSAVING
__u16 sleepCount = 0;
void eponMacPowerSavingHandler(__u32 isrStatus)
{
	if(isrStatus&PS_EARLY_WAKEUP_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr PS_EARLY_WAKEUP_INT");
	}

	if(isrStatus&RX_SLEEP_ALLOW_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr RX_SLEEP_ALLOW_INT");
	}
	
	if(isrStatus&PS_WAKEUP_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr PS_WAKEUP_INT");
		EPON_STOP_TIMER(gpEponPriv->early_wakeup_timer) ;
	}

	if(isrStatus&PS_SLEEP_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr PS_SLEEP_INT");
		if (gpEponPriv->eponCfg.earlyWakeupFlag == 1)
		{
			sleepCount++;
			if (gpEponPriv->eponCfg.earlyWakeupCount == sleepCount)
			{
				EPON_START_TIMER(gpEponPriv->early_wakeup_timer,gpEponPriv->eponCfg.earlyWakeupTimer) ;
				sleepCount = 0;
			}
		}
		else
		{
		    sleepCount = 0;
		}
	}
}
#endif
void eponIsr( void )
{
	__u32 isrValue;
	__u32 isrValue2;
	int ret = 0;
	int index = 0;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponIsr");

	isrValue = READ_REG_WORD(e_int_status);

	WRITE_REG_WORD(e_int_status , 0xffffffff);			//clear

	/* INT handler */

	if(eponDrv.hldoverEnable && (TRUE == eponDrv.typeBOnGoing))
		return;
	
	if(isrValue == 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR , "eponIsr interrupt status is 0");
		return ;
	}

#ifdef EPON_MAC_HW_TEST
	isrValue2 = READ_REG_WORD(e_int_status);
	if(isrValue2){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "eponIsr interrupt status 0x%x --> 0x%x, after clear",isrValue, isrValue2);
	}
	isrValue &= ignoreIntMask;
#endif

	if(isrValue&TIMEDRFT_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr TIMEDRFT_INT isrValue=%X", isrValue);

		isrValue |=  TIMEDRFT_INT;
		eponTimeDrftIntHandler(0);
	}

	if(isrValue&MPCP_TIMEOUT_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr MPCP_TIMEOUT_INT");

		isrValue |=  MPCP_TIMEOUT_INT;
		eponMpcpTmOutIntHandler(0);
	}
else
    {
		atomic_set(&mpcpTmOutCnt, 0);
    }

	if (gpEponPriv->mpcpInterruptMode) {
		if(isrValue&REG_REQ_DONE_INT)
		{
			dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr REGISTER_REQ_DONE_INT");
			eponMpcpRgstReqIntHandler();
		}
		if(isrValue&REG_ACK_DONE_INT)
		{
			dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr REGISTER_ACK_DONE_INT");
			eponMpcpRgstAckIntHandler();
		}
	}
	
	for(index = 0 ; index < EPON_1G_MAX_LLID_NUM ; index++)
	{
		if(isrValue & (LLID0_RCV_RGST_INT << index) )
		{
			dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr LLID%d_RCV_RGST_INT", index);
			ret = eponMpcpRgstIntHandler(index);
			
			if(ret <0){
				dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: LLID%d_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", index, ret);
			}

            /*ignore current discovery gate interrupt*/
            isrValue &= 0xfffffffe;
		}
	}

	if(isrValue&RCV_DSCVRY_GATE_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "EponIsr Receive DiscvGate Message");
		
		isrValue |=  RCV_DSCVRY_GATE_INT;
		tasklet_schedule(&eponMacMpcpDscvGateTask);
	}

	if(isrValue&TOD_1PPS_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr TOD_1PPS_INT");
		
		isrValue |=  TOD_1PPS_INT;
		ret = eponTod1ppsIntHandler();
		
		if(ret <0){
			dbgtoMem(EPON_DEBUG_LEVEL_ERR , "ERR: TOD_1PPS_INT eponTod1ppsIntHandler ret=%d", ret);
		}
	}
	
	if(isrValue&TOD_UPDT_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr TOD_UPDT_INT");
		
		isrValue |=  TOD_UPDT_INT;
		ret = eponTodUpdtIntHandler();
		
		if(ret <0){
			dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: TOD_UPDT_INT eponTodUpdtIntHandler ret=%d", ret);
		}
	}

	if(isrValue&PTP_MSG_TX_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr PTP_MSG_TX_INT");
		
		isrValue |=  PTP_MSG_TX_INT;
		ret = eponPtpMsgTxIntHandler();
		
		if(ret <0){
			dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: PTP_MSG_TX_INT eponPtpMsgTxIntHandler ret=%d", ret);
		}
	}

	if(isrValue&GNT_BUF_OVRRUN_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr GNT_BUF_OVRRUN_INT");

		isrValue |=  GNT_BUF_OVRRUN_INT;
		ret = eponMpcpGntOvrRunIntHandler();
		
		if(ret <0){
			dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: GNT_BUF_OVRRUN_INT eponMpcpGntOvrRunHandler ret=%d", ret);
		}
	}

	if(isrValue&RPT_OVERINTVL_INT)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr RPT_OVERINTVL_INT");
		
		isrValue |=  RPT_OVERINTVL_INT;
		ret = eponMpcpRptOvrIntvalIntHandler();
		
		if(ret <0){
			dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: RPT_OVERINTVL_INT eponMpcpRptOvrIntvalIntHandler ret=%d", ret);
		}
	}
	
#ifdef TCSUPPORT_EPON_POWERSAVING
	eponMacPowerSavingHandler(isrValue);
#endif

	return;
}

/*************************PUBLIC PRINT******************************************/
static void dbgPrintInit(void);
static void stringWrite(void);
static void stringRead(void);
static void dbgPrintTimer(TIMER_FUN_PAAM arg);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,90)
spinlock_t dbgPrintLock = SPIN_LOCK_UNLOCKED;
#else
static DEFINE_SPINLOCK(dbgPrintLock);
#endif
unsigned long dbgPrintFlags;

#define PRINT_MEM_COL 128	
#define PRINT_MEM_ROW 256	

struct SCREEM_PRINT
{
	ktime_t time_info;
	char mem[PRINT_MEM_COL];
}screenprint;

struct SCREEM_PRINT print_mem[PRINT_MEM_ROW];

static struct SCREEM_PRINT *pWrite = NULL;
static struct SCREEM_PRINT *pRead = NULL;
static char temp[128] = {0};

struct timer_list dbgPrint;
void dbgPrintQuit(void)
{
	del_timer(&dbgPrint);
}

static void dbgPrintInit(void)
{
	EPON_CREATE_TIMER(&dbgPrint,dbgPrintTimer,(jiffies + (HZ >> 1)));
	add_timer(&dbgPrint);

    memset(&print_mem, 0, sizeof(struct SCREEM_PRINT)*PRINT_MEM_ROW);
	pWrite = print_mem;
	pRead = print_mem;
}

/*print time information*/
static inline void print_time_information(ktime_t * time)
{    
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	u32 nsec = time->tv.nsec % 1000;
	u32 usec = (time->tv.nsec / 1000) % 1000;    
	u32 msec = (time->tv.nsec / 1000000) % 1000;   
	u32 sec = time->tv.sec % 60;
	u32 min = time->tv.sec / 60;
#else   /*3.18 kernel*/
#ifdef TCSUPPORT_CPU_ARMV8
	u64 ktns = ktime_to_ns(*time);
	u64 ktus = ktime_to_us(*time);
	u64 ktms = ktime_to_ms(*time);
    u32 nsec = do_div(ktns,1000);
    u32 usec = do_div(ktus,1000);
	u32 msec = do_div(ktms,1000);
	u32 sec = do_div(ktms,60);
	u32 min = ktms;
#else
    u32 nsec = ktime_to_ns(*time) % 1000;
    u32 usec = ktime_to_us(*time)% 1000;
    u32 msec = ktime_to_ms(*time)% 1000;
    u32 sec = (ktime_to_ms(*time) / 1000) % 60;
    u32 min = (ktime_to_ms(*time) / 60000);
#endif
#endif
    printk("[%03dmin : %03ds : %03dms: %03dus : %03dns] ", min, sec, msec, usec, nsec);
}


/*write debug information to memory*/
static void stringWrite()
{
    static int cnt = 0;
	if(pWrite->mem[0] != '\0'){
	    if(!(cnt%PRINT_MEM_ROW))
	    {
		    printk("Memory out.\n");
		    cnt = 0;
		}
		else
		    cnt++;
	}
	else
	{
		pWrite->time_info = ktime_get();
		memcpy(pWrite->mem, temp, sizeof(temp));
		if (pWrite == (print_mem + PRINT_MEM_ROW-1))
		    pWrite = print_mem;
		else
		    pWrite++;
	}

}

/*read log from memory*/
static void stringRead()
{
	int i;
	
	for (i = 0; i < 60; i++)
	{		
		if(pRead->mem[0] == '\0'){
			break;
		}
		
		print_time_information(&pRead->time_info);
		printk("%s\n", pRead->mem);
		mb();
		memset(pRead, 0, sizeof(screenprint));
		if(pRead == (print_mem + PRINT_MEM_ROW-1))
			pRead = print_mem;
		else
		    pRead++;
	}
}

/*timer to write log*/
static void dbgPrintTimer(TIMER_FUN_PAAM arg)
{
	stringRead();
	mod_timer(&dbgPrint, jiffies + (HZ >> 1));	
}

/*print debug*/
void  dbgtoMem(__u32 debugLevel, char *fmt,...)
{
    va_list wp;	
	if (eponDebugLevel < debugLevel){
		return;
	}

	spin_lock_irqsave(&dbgPrintLock, dbgPrintFlags);
	memset(temp, 0, PRINT_MEM_COL);
	va_start(wp, fmt);
	vsnprintf(temp, PRINT_MEM_COL, fmt, wp);
	stringWrite();
	va_end(wp);
	
	spin_unlock_irqrestore(&dbgPrintLock, dbgPrintFlags);
}
EXPORT_SYMBOL(dbgtoMem);
/*************************PUBLIC PRINT END******************************************/
int eponIrqRegister(void)
{
    int err = 0;

	err = QDMA_API_REGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_EPON_MAC_HANDLER, eponIsr);
	if(err){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponIrqInit request_irq err=%d", err);
		return err;
	}

	return 0;
}

void eponIrqUnregister(void)
{
	QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_EPON_MAC_HANDLER);
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponIrqDinit QDMA_CALLBACK_EPON_MAC_HANDLER");

    return;
}

int getOnuMacAddr(__u8 *macAddr){
	unsigned char flashMacAddr[6] = {0};
#ifndef TCSUPPORT_CPU_ARMV8
    int i = 0;
#endif

#ifdef TCSUPPORT_CPU_ARMV8
    if(get_ethaddr(flashMacAddr, sizeof(flashMacAddr)) != 0){
        printk("func:%s get ethaddr error!!!\n",__func__);
    }
#else  
	for (i=0; i<6; i++) {
		flashMacAddr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}
#endif
    
	if( (flashMacAddr[0] == 0) && (flashMacAddr[1] == 0) && (flashMacAddr[2] == 0) &&
	    (flashMacAddr[3] == 0) && (flashMacAddr[4] == 0) && (flashMacAddr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");	    
	else{
  		memcpy(macAddr, flashMacAddr, 6);  	
	}
		return 0;
}



int eponMacCheckMacCfg(void){
	REG_e_mac_addr_cfg eponMacAddrCfgReg;
	__u32 timers = 0;

	
	//check done bit is ready
	while(timers < 100){
		eponMacAddrCfgReg.Raw = READ_REG_WORD(e_mac_addr_cfg);
		if(eponMacAddrCfgReg.Bits.mac_addr_rwcmd_done == 0){
			break;
		}
		timers++;
	}

	if(timers == 100){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr timers == 100");
		return -1;
	}
	return 0;
}


int eponMacSetMacAddr(eponLlid_t *llidEntry_p)
{
	REG_e_mac_addr_cfg eponMacAddrCfgReg;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE , "enter eponMacSetMacAddr");


	if(llidEntry_p == NULL){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr llidEntry_p == NULL");
		return -1;
	}


	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr timers == 100");
		return -1;
	}
	eponMacAddrCfgReg.Raw = 0;
	//lower 32 bit
	WRITE_REG_WORD(e_mac_addr_value , get32(llidEntry_p->macAddr+2));
	
	eponMacAddrCfgReg.Bits.mac_addr_dw_idx = 0;
	eponMacAddrCfgReg.Bits.mac_addr_llid_indx = llidEntry_p->llidIndex;
	eponMacAddrCfgReg.Bits.mac_addr_rwcmd = 1;
	WRITE_REG_WORD(e_mac_addr_cfg , eponMacAddrCfgReg.Raw);

	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr set 1 timers == 100");
		return -1;
	}
	eponMacAddrCfgReg.Raw = 0;

	//high 16 bit
	WRITE_REG_WORD(e_mac_addr_value , get16(llidEntry_p->macAddr));
	
	eponMacAddrCfgReg.Bits.mac_addr_dw_idx = 1;
	eponMacAddrCfgReg.Bits.mac_addr_llid_indx = llidEntry_p->llidIndex;
	eponMacAddrCfgReg.Bits.mac_addr_rwcmd = 1;
	WRITE_REG_WORD(e_mac_addr_cfg , eponMacAddrCfgReg.Raw);

	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr set 2 timers == 100");
		return -1;
	}
	
	return 0;
}


int eponMacGetMacAddr(eponLlid_t *llidEntry_p){
		REG_e_mac_addr_cfg eponMacAddrCfgReg;
	__u32 macLow = 0;
	__u32 macHigh = 0;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE , "enter eponMacGetMacAddr");
	
	if(llidEntry_p == NULL){
		return -1;
	}


	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacGetMacAddr timers == 100");
		return -1;
	}
	eponMacAddrCfgReg.Raw = 0;
	//lower 32 bit
	//WRITE_REG_WORD(e_mac_addr_value , get32(llidEntry_p->macAddr+2));
	
	eponMacAddrCfgReg.Bits.mac_addr_dw_idx = 0;
	eponMacAddrCfgReg.Bits.mac_addr_llid_indx = llidEntry_p->llidIndex;
	eponMacAddrCfgReg.Bits.mac_addr_rwcmd = 0;
	WRITE_REG_WORD(e_mac_addr_cfg , eponMacAddrCfgReg.Raw);

	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr set 1 timers == 100");
		return -1;
	}

	macLow = READ_REG_WORD(e_mac_addr_value);
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMacSetMacAddr set 1 macLow == %8X", macLow);
	
	eponMacAddrCfgReg.Raw = 0;

	//high 16 bit
	//WRITE_REG_WORD(e_mac_addr_value , get16(llidEntry_p->macAddr));
	
	eponMacAddrCfgReg.Bits.mac_addr_dw_idx = 1;
	eponMacAddrCfgReg.Bits.mac_addr_llid_indx = llidEntry_p->llidIndex;
	eponMacAddrCfgReg.Bits.mac_addr_rwcmd = 0;
	WRITE_REG_WORD(e_mac_addr_cfg , eponMacAddrCfgReg.Raw);

	//check done bit is ready
	if(eponMacCheckMacCfg() < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr set 2 timers == 100");
		return -1;
	}


	macHigh = READ_REG_WORD(e_mac_addr_value);
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMacSetMacAddr set 2 macHigh == %8X", macHigh);
	
	return 0;
}

extern __u8 eponOnuMacAddr[];

 int eponLlidEnable(__u8 llidIndex){
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	__u32 tmp = 0;
	__u8 macAddr[6] = {0};
	__u32 * mac_int = NULL;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponLlidEnable");

	/*  init MAC addr */
	//getOnuMacAddr(macAddr);

#ifdef TCSUPPORT_PON_TEST
	if (eponDrvDbgMask & EPON_DBGMASK_TEMPMAC){
		memcpy(macAddr, eponDrvTmpMacAddr, 6);
	}else
#endif
	memcpy( (char *)macAddr , (char *)eponOnuMacAddr ,6);

	mac_int = (__u32 *)(macAddr+2);
	tmp =ntohl(*mac_int);
	tmp += llidIndex;	
	*mac_int = htonl(tmp);
	macAddr[2] = eponOnuMacAddr[2];
	memcpy(llidEntry_p->macAddr , macAddr , 6);
	tmp = 0;
	
	if(eponMacSetMacAddr(llidEntry_p) < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr fail llidIndex=%d", llidIndex);
		//return -1;
	}


	if(eponDebugLevel >= EPON_DEBUG_LEVEL_NOTIFY)
	eponMacGetMacAddr(llidEntry_p);
	/* MPCP status init */
	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_WAIT;
	
	llidEntry_p->eponMpcp.eponDiscFsm.begin = TRUE;
	llidEntry_p->eponMpcp.eponDiscFsm.registered = FALSE;

	eponMpcpDscvFsmWaitHandler(llidIndex);

	//MACR init
	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscMacr_RgstrAck = RgstrAckFlag[llidIndex];	

	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_REGISTERING;
	
	/* enable flag */
	llidEntry_p->enableFlag= XPON_ENABLE;

	//enable INT
	tmp = READ_REG_WORD(e_int_en);
	tmp |= (1<<(llidIndex+1));
	WRITE_REG_WORD(e_int_en , tmp);
	
	return 0;
}



int eponLlidDisable(__u8 llidIndex){
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	__u32 tmp = 0;
	eponLlidDscvStsReg_t sdcvSts;
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	QDMA_ChannelStatus_T chnlCloseStatusSet = {0};
#endif

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponLlidDisable");

	/* disable flag */
	
	llidEntry_p->enableFlag = XPON_DISABLE;

	//DISABLE INT
	tmp = READ_REG_WORD(e_int_en);
	tmp &= ~(1<<(llidIndex+1));
	WRITE_REG_WORD(e_int_en , tmp);
	
	//change disc status to unregist
	sdcvSts.Raw = READ_REG_WORD(g_epon_llid_dscv_stat[llidIndex]);
	sdcvSts.Bits.llidDscvrySts = 0;
	WRITE_REG_WORD(g_epon_llid_dscv_stat[llidIndex], sdcvSts.Raw);

	//retire the channel
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	chnlCloseStatusSet.chnlIdx = llidIndex;
	chnlCloseStatusSet.chnlStatus = 0xFF;
	QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
#endif

	FE_API_SET_CHANNEL_RETIRE_ONE(FE_GDM_SEL_GDMA2, llidIndex);

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	chnlCloseStatusSet.chnlIdx = llidIndex;
	chnlCloseStatusSet.chnlStatus = 0x0;
	QDMA_API_SET_CHANNEL_CLOSE_STATUS(ECNT_QDMA_WAN,&chnlCloseStatusSet);
#endif

	
	//delete the interface
	xmcs_remove_llid(llidIndex);
	
	return 0;
}

void eponSetLlid0DeafultThreshold(void)
{
    __u8  queueIndex = 0;
    eponQueueThreshold_t EponQThr = {0};
    int ret = 0;
	Xpon_Phy_Mode_t current_mode = PHY_UNKNOWN_CONFIG;
	current_mode = XPON_PHY_GET(PON_GET_PHY_MODE) ;
	if(current_mode != PHY_EPON_CONFIG)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "mode has changed, not epon\n");
		return;
	}
    
	for (queueIndex = 0; queueIndex< 8;queueIndex++)
	{
		EponQThr.channel = 0;
		EponQThr.queue = queueIndex;
		EponQThr.thrIdx = 0;
		EponQThr.value = eponDrv.DefaultThreshold;
		ret = epon_set_queue_threshold(&EponQThr) ;
		if(ret < 0 ){
			printk("doEponSetLlidQueThrshld error\n");
			return;
		}		
	}
	eponSetLlidThrshldNum(0,1);
    dbgtoMem(EPON_DEBUG_LEVEL_ERR, "set epon llid 0 set default threshold id 1 val %d",eponDrv.DefaultThreshold);

    return;
}

static int getValidLlidNum(__u32 llidMask){
    int llidNum = 0;
    while(llidMask){
        if(llidMask & 0x1){
            llidNum++;
        }
        llidMask >>= 1;
    }

    return llidNum;
}
static int eponInitAllLlid(void){
	__u8 llidIndex = 0;
	eponLlid_t *llidEntry_p = NULL;
	int temp = 0;
	int currentState = 0;
#if defined(TCSUPPORT_CPU_EN7580)
	QDMA_TxQCngstChannelCfg_T txChannelCfg = {0};
#endif

	while(llidIndex < EPON_1G_MAX_LLID_NUM){
		llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
		temp = llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout;
		currentState = llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState;
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponInitAllLlid llid %d , Timeout %d, State %d",
			llidIndex, temp, currentState);

		memset(llidEntry_p, 0 , sizeof(eponLlid_t));

		llidEntry_p->llidIndex = llidIndex;

		if((eponDrv.llidMask)&(1<<llidIndex)){//this LLID enable
			eponLlidEnable(llidIndex);
#if defined(TCSUPPORT_CPU_EN7580)
			/*set channel flowctrl normal*/
			txChannelCfg.channel = llidIndex;
			txChannelCfg.channelMode = QDMA_CHANNEL_NORMAL;
			QDMA_API_SET_TXQ_CNGST_CHANNEL_NONBLOCKING(ECNT_QDMA_WAN, &txChannelCfg);
#endif
		}	
		
		
		if (MPCP_STATE_DENIED == currentState)
		{
			if(temp > 0 && temp <= g_silence_time)
				llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout = temp;
			
			llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = currentState;
		}

		llidIndex++;
	}

	return 0;
}

int epon_set_qdma_qos(void)
{
	QDMA_TxRateMeter_T txRateMeter;
	QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
	QDMA_txCngstCfg_t txCngstCfg;
	
	memset(&txRateMeter, 0, sizeof(QDMA_TxRateMeter_T));
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	txRateMeter.txRateMeterTimeSlice = 0xc8;
	txRateMeter.txRateMeterWindowSize = 20;
#else
	txRateMeter.txRateMeterTimeSlice = 0x3e8;
	txRateMeter.txRateMeterTimeDivisor = 0;
#endif
	QDMA_API_SET_TX_RATEMETER(ECNT_QDMA_WAN, &txRateMeter);
	
	memset(&txCngstCfg, 0, sizeof(QDMA_txCngstCfg_t));
	txCngstCfg.txCngstMode = QDMA_TXCNGST_DYNAMIC_PEAKRATE_MARGIN;
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    txCngstCfg.peekRateMargin = QDMA_TXCNGST_PEEKRATE_MARGIN_0;
#else
	txCngstCfg.peekRateMargin = QDMA_TXCNGST_PEEKRATE_MARGIN_100;
#endif
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	txCngstCfg.peekRateDuration = 0xffff;
#else
	txCngstCfg.peekRateDuration = 0xff;
#endif
	QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(ECNT_QDMA_WAN, &txCngstCfg);

	memset(&txqDynCngstThrh, 0, sizeof(QDMA_TxQDynCngstThrh_T));
	QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
	if(!(isEN7523 || isAN7552)){
		txqDynCngstThrh.dynCngstTotalMinThrh = 0x333;
		txqDynCngstThrh.dynCngstChnlMinThrh = 0xA0;
		txqDynCngstThrh.dynCngstQueueMinThrh = 0x30;
	}
	QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);

	return 0;
}

int eponTxOamFavorModeCtl(__u8 flag)
{
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
       if (flag == XPON_ENABLE){
           QDMA_API_SET_OAM_MODIFY_FP_EN(ECNT_QDMA_WAN, QDMA_ENABLE);
       }else{
           QDMA_API_SET_OAM_MODIFY_FP_EN(ECNT_QDMA_WAN, QDMA_DISABLE);
       }
#else
	__u32 Raw = 0;
	QDMA_TxQCngstQueueCfg_T txqCngstQueueCfg;

	if (!isEN7526c)
		return 0;

	memset(&txqCngstQueueCfg, 0, sizeof(QDMA_TxQCngstQueueCfg_T));
	if (flag == XPON_ENABLE)
	{
		txqCngstQueueCfg.queueMode = QDMA_QUEUE_NONBLOCKING;
		txqCngstQueueCfg.queue = 7;
		QDMA_API_SET_TXQ_CNGST_QUEUE_NONBLOCKING(ECNT_QDMA_WAN, &txqCngstQueueCfg);

		FE_API_SET_TX_FAVOR_OAM_ENABLE(FE_ENABLE);

		Raw = READ_REG_WORD(e_glb_cfg);
		Raw |= (1<<25);
		WRITE_REG_WORD(e_glb_cfg , Raw);
	}
	else
	{
		txqCngstQueueCfg.queueMode = QDMA_QUEUE_NORMAL;
		txqCngstQueueCfg.queue = 7;
		QDMA_API_SET_TXQ_CNGST_QUEUE_NONBLOCKING(ECNT_QDMA_WAN, &txqCngstQueueCfg);

		FE_API_SET_TX_FAVOR_OAM_ENABLE(FE_DISABLE);

		Raw = READ_REG_WORD(e_glb_cfg);
		Raw &= ~(1<<25);
		WRITE_REG_WORD(e_glb_cfg , Raw);
	}
#endif

	return 0;
}


static int eponInitParam(void){

	//memset(&eponDrv , 0, sizeof(epon_t));
	eponDrv.hldoverEnable = 0;
	eponDrv.hldOverTime = 0;
	eponDrv.timeDrftEq255Cnt = 0;
	eponDrv.timeDrftFrom8To16Cnt = 0;
	eponDrv.llidMask = eponDefLlidMask;
	gpWanPriv->activeChannelNum = getValidLlidNum(eponDrv.llidMask);

	eponInitAllLlid();
	epon_set_qdma_qos();
	xpon_reset_qdma_tx_buf();
	
	return 0;
}


/* eponHwDygaspCtrl
flag: ENABLE/DISABLE
*/
int eponHwDygaspCtrl(__u8 flag){
	__u32 Raw;
	Raw = READ_REG_WORD(e_dyinggsp_cfg);
	
	
	if(flag == XPON_ENABLE){
		// set reg		
		Raw = 0x80000102;
		WRITE_REG_WORD(e_dyinggsp_cfg , Raw);
	}else if((flag == XPON_DISABLE)){
		// set reg
		Raw = 0x00000000;
		WRITE_REG_WORD(e_dyinggsp_cfg , Raw);
	}
	return 0;
}

int eponDevGetDyingGaspMode(__u8 *mode)
{
	REG_e_dyinggsp_cfg eponDGCtrl;

	eponDGCtrl.Raw = READ_REG_WORD(e_dyinggsp_cfg);
	*mode = eponDGCtrl.Bits.hw_dying_gasp_en;

	return 0 ;
}

int eponDevSetDyingGaspNum(__u32 num)
{
	REG_e_dyinggsp_cfg eponDGCtrl;

	eponDGCtrl.Raw = READ_REG_WORD(e_dyinggsp_cfg);
	eponDGCtrl.Bits.dygsp_num_of_times = num;
	WRITE_REG_WORD(e_dyinggsp_cfg, eponDGCtrl.Raw) ;

	return 0;
}

int eponDevGetDyingGaspNum(__u32 *num)
{
	REG_e_dyinggsp_cfg eponDGCtrl;

	eponDGCtrl.Raw = READ_REG_WORD(e_dyinggsp_cfg);
	*num = eponDGCtrl.Bits.dygsp_num_of_times;

	return 0;
}

void eponSetReportData(void){
	WRITE_REG_WORD(e_rpt_len,0x100);
	WRITE_REG_WORD(e_rpt_len,6);
	WRITE_REG_WORD(e_rpt_data,0x02ff03e8);
	WRITE_REG_WORD(e_rpt_data,0);
	WRITE_REG_WORD(e_rpt_data,0);
	WRITE_REG_WORD(e_rpt_data,0);
	WRITE_REG_WORD(e_rpt_data,0xffff);
	WRITE_REG_WORD(e_rpt_data,0xff000000);
	WRITE_REG_WORD(e_rpt_data,0);
	
	return;
}

void eponSetStaticReport(void){
    printk("Enable epon static report\n");
	e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 1); // clear rpt buffer
	e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
	e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
	e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
	e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
	e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 0); // return normal
	e_rpt_len_SET_df_rpt_dt_len(e_rpt_len, 1);// set def rpt data length
	e_rpt_data_SET_df_rpt_data(e_rpt_data, staticRptData); // static report value
	e_rpt_data_SET_df_rpt_data(e_rpt_data, 0);					
	e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 1);
	e_tx_cal_cnst_SET_default_ovrhd(e_tx_cal_cnst, 0x8);

    return;
}

void eponSnifferModeCtrl(__u8 flag)
{
	__u32 Raw = 0;
	__u32 tmp = 0;
	__u8 mask = gpEponPriv->eponCfg.snifferModeConfig.snifferModeLanMask;

	if (!isEN7526c)
		return;

	Raw = READ_REG_WORD(e_glb_cfg);
	if (flag != SNIFFER_DISABLE)
	{
		Raw |= (SNIFFER_MODE | MPCP_FWD | RX_NML_GATE_FWD); /*enable sniffer normal mode*/
		if (flag == SNIFFER_PROMISC)
		{
			Raw |= ALL_UNICAST_LLID_PKT_FWD; /* enable sniffer promiscuous mode,forward all uni-cast llid packets */
		}
		WRITE_REG_WORD(e_glb_cfg , Raw);

		tmp = READ_REG_WORD(e_sniff_sp_tag);
		tmp &= ~(SNIFFER_SP_TAG_MASK << SNIFFER_SP_TAG_SHIFT);
		tmp |= ((mask & SNIFFER_SP_TAG_MASK) << SNIFFER_SP_TAG_SHIFT); /*set spetial tag*/
		WRITE_REG_WORD(e_sniff_sp_tag , tmp);
	}
	else
	{
		Raw &= 0xFBBBFBFF;/* disable sniffer mode,clear bit 26,bit 22, bit 10, bit 18 */
		WRITE_REG_WORD(e_glb_cfg , Raw);
		WRITE_REG_WORD(e_sniff_sp_tag , 0);
	}

    return;
}

int eponSetSnifferModeConfig(__u8 enable, __u16 mask)
{
	if (!isEN7526c)
	return 0;

	gpEponPriv->eponCfg.snifferModeConfig.snifferModeEnable=  enable;
	gpEponPriv->eponCfg.snifferModeConfig.snifferModeLanMask =  mask;
	eponSnifferModeCtrl(enable);

	return 0;
}

/*****************************************************************************
//function :
//		setMpcpFwd
//description : 
//		this function is set bit 22 of bfb66000
//input :	
//		0:disable MPCP_FWD
//      1:Enable MPCP_FWD
//output :
//		N/A
******************************************************************************/
void setMpcpFwd(int flag){
	uint read_data = 0;
	uint write_data = 0;

	read_data = READ_REG_WORD(e_glb_cfg);
	if((flag == 1) && !(read_data & (MAC_CONTROL_MPCP_FWD))){
		write_data = read_data | (MAC_CONTROL_MPCP_FWD);
		WRITE_REG_WORD(e_glb_cfg, write_data);
		//printk("setMpcpFwd : enable MPCP_FWD\n");
	}else if((flag == 0) && (read_data & (MAC_CONTROL_MPCP_FWD))){
		write_data = read_data & (~MAC_CONTROL_MPCP_FWD);
		WRITE_REG_WORD(e_glb_cfg, write_data);
		//printk("setMpcpFwd : disable MPCP_FWD\n");
	}
}

void setTxOamFav(__u32 value){
	__u32 read_data = 0;
	__u32 write_data = 0;

	read_data = READ_REG_WORD(e_glb_cfg);
	write_data = modifyBit(read_data, 25, value);
	WRITE_REG_WORD(e_glb_cfg, write_data);	

}

int eponMacSwReset(void){
	__u32 valueRst;
    REG_e_glb_cfg global_cfg;
    
    memset(&global_cfg, 0, sizeof(REG_e_glb_cfg));

	if(!no_epon_mac_reset)
	{
        #ifdef TCSUPPORT_CPU_ARMV8
		/* reset mac */
		valueRst = GET_SCU_RSTCTRL1();
		valueRst |= SCU_EPON_MAC_RESET  ;
		SET_SCU_RSTCTRL1(valueRst);
		udelay(1);
		/* release mac */
		valueRst = GET_SCU_RSTCTRL1();
		valueRst &= ~SCU_EPON_MAC_RESET ;
		SET_SCU_RSTCTRL1(valueRst);
        #else
    	valueRst = READ_REG_WORD(REG_E_SW_RST);
    	valueRst |= (1<<31);
    	WRITE_REG_WORD(REG_E_SW_RST , valueRst);
    	udelay(1);
    	valueRst &= 0x7fffffff;
    	WRITE_REG_WORD(REG_E_SW_RST , valueRst);
        #endif
    }

    /*reset mpcp_cmd_done bit(bfb66028[16] = 1)*/
	global_cfg.Raw = READ_REG_WORD(e_glb_cfg);
	global_cfg.Bits.epon_mac_sw_rst = 1;
	WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);
	global_cfg.Bits.epon_mac_sw_rst = 0;
	WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);
	

#ifdef EPON_MAC_HW_TEST
	if(eponSetProbe != 0) {
		WRITE_REG_WORD( e_dbg_prb_sel,eponSetProbe ) ;
	}
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"e_int_status=%x,e_llid0_dscvry_sts=%x",
	READ_REG_WORD(e_int_status),
	READ_REG_WORD(e_llid0_dscvry_sts));
#endif

	/*  send controll packet first */
	global_cfg.Bits.rpt_txpri_ctrl = 1;
    /* loc_cnt_sync_method 1 */
    global_cfg.Bits.loc_cnt_sync_method = 1;
    /* When send REG_req, MAC assert burst_en(burst mode) to PHY */
    global_cfg.Bits.discv_burst_en = 1;
	WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);

#ifdef CONFIG_USE_MT7520_ASIC 
	WRITE_REG_WORD(e_txfetch_cfg, 0x202403e8);	
#else
	WRITE_REG_WORD(e_txfetch_cfg, 0x242a03e8);	//0x1a2603e8
#endif

#ifdef CONFIG_USE_MT7520_ASIC 
	if (eponStaticRptEnable == 1){
		eponSetStaticReport();
	}else{/*If QDMA do not response to MAC report request, MAC will send static report, workaround for OSBNB00090282*/
        eponSetReportData();
    }
#endif

	/* 7521 synctime always have, don't need this */
	setMpcpFwd(gpEponPriv->fwdFlag);

	change_pon_led_status(ALARM_LED_OFF);

#ifdef TCSUPPORT_CPU_EN7528
	WRITE_FE_REG(0xbfb55630,0);
	WRITE_FE_REG(0xbfb55634,0xcf506);
	WRITE_REG_WORD(e_tx_cal_cnst,0x2612010c);
#endif

	if(isEN7523 || isAN7552){
    	WRITE_FE_REG(0xbfb57130,0);
    	/*report size overhead config,bfb57134[15:8]== bfb660d8[15:8],bfb57134[7:0] = bfb660d8[23:16]*/
    	WRITE_FE_REG(0xbfb57134,0x010c);
        /*set the grant required for report message,fec on:bfb66160[29:24]=0x38 TQ,fec off:bfb66160[23:16]=0x2b TQ*/
        WRITE_REG_WORD(e_snf_mpcp_oam_ctl,0x382b0000);
	}

    set_fix_reg_list();

	return 0;
}



int eponMacStop(void){
	__u8 llidIndex = 0;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMacStop");

	/* EPON MAC SW reset */
	eponMacSwReset();

	for(llidIndex = 0; llidIndex < EPON_1G_MAX_LLID_NUM;llidIndex++){
		xmcs_remove_llid(llidIndex);
	}

	return 0;
}

int eponMacRestart(void){
	__u8 llidIndex = 0;
	__u8 snifferModeEnable = 0;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMacRestart");

	atomic_set(&eponMacRestart_flag, 1);
	
	/* EPON MAC SW reset */
	eponMacSwReset();

	for(llidIndex = 0; llidIndex < EPON_1G_MAX_LLID_NUM;llidIndex++){
		xmcs_remove_llid(llidIndex);
	}

	eponInitAllLlid();

	if (gpPonSysData->dyingGaspData.isEponHwFlag)
		eponHwDygaspCtrl(XPON_ENABLE);
	else
		eponHwDygaspCtrl(XPON_DISABLE);

	/*Init EPON dying gasp pakets number*/
	eponDevSetDyingGaspNum(EPON_DYING_GASP_NUM);
	
	if (gpEponPriv->eponCfg.txOamFavorMode)
		eponTxOamFavorModeCtl(XPON_ENABLE);
	else
		eponTxOamFavorModeCtl(XPON_DISABLE);

	snifferModeEnable = gpEponPriv->eponCfg.snifferModeConfig.snifferModeEnable;
	eponSnifferModeCtrl(snifferModeEnable);
	
	atomic_set(&eponMacRestart_flag, 0);

	return 0;
}

int eponMacTxRxEnable(void){
	__u32 Raw = READ_REG_WORD(e_glb_cfg);

	Raw &= ~(RXMBI_STOP|TXMBI_STOP);
	WRITE_REG_WORD(e_glb_cfg,Raw);
	return 0;
}

int eponMacTxRxDisable(void){
	__u32 Raw = READ_REG_WORD(e_glb_cfg);
	
	
	Raw |= (RXMBI_STOP|TXMBI_STOP);
	WRITE_REG_WORD(e_glb_cfg,Raw);
	return 0;
}

int eponMacTxEnable(void){
	__u32 Raw = READ_REG_WORD(e_glb_cfg);

	Raw &= ~(TXMBI_STOP);
	WRITE_REG_WORD(e_glb_cfg,Raw);
	return 0;
}


int eponMacTxDisable(void){
	__u32 Raw = READ_REG_WORD(e_glb_cfg);
	
	
	Raw |= (TXMBI_STOP);
	WRITE_REG_WORD(e_glb_cfg,Raw);
	return 0;
}


void FEChannelRetire(void){
	__u8 i = 0;
    /* disable all tx channel */
	for(i = 0; i < 32; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_DISABLE);
	}
	/* disable all rx channel */
	for(i = 0; i < 16; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_DISABLE);
	}
	/* disable CDM2 rx DMA */
	for(i = 0; i < 8; i++) {
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_DISABLE);
	}
	/* wait for GDM2 finish */
	udelay(60);
	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 32);
}

int eponWanStopWithChannelRetire(void){
	unsigned long flags;
	spin_lock_irqsave(&epon_reg_lock,flags);

	//eponMacTxRxDisable
	eponMacTxRxDisable();

	/* disable cpu traffic */
	disable_cpu_us_traffic();

    /*FE channel retire*/
	FEChannelRetire();

	eponMacStop();
	eponMacTxRxDisable();
	
	spin_unlock_irqrestore(&epon_reg_lock,flags); 
	
	return 0;
}

uint g_EponLaserTime = 0;

void eponSetlaserTime(void)
{
	__u32 Raw =0 ;
	__u32 temp = 0;
	
	/* laser off time modify */
	Raw = (READ_REG_WORD(e_laser_onoff_time));
	temp = (Raw & 0xFF00 >> 8) - g_EponLaserTime;
	Raw &= 0xFFFF00FF;
	Raw = Raw | (temp << 8);
	WRITE_REG_WORD(e_laser_onoff_time , Raw);
}

int eponWanResetWithChannelRetire(void){
	__u8 i = 0;
	unsigned long flags;
	spin_lock_irqsave(&epon_reg_lock,flags);
	
	//eponMacTxRxDisable
	eponMacTxRxDisable();
	FEChannelRetire();

	eponMacRestart();
	eponMacTxRxDisable();

	for(i = 0; i < 8; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE);
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_ENABLE);
	}

	if (gpEponPriv->eponCfg.txOamFavorMode) {
		for (i=16;i<24;i++) {
			/* enable high 16bit TX channel for enable TX favor OAM mode */
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE); 
		}
	}
    
	eponMacTxRxEnable();	
	eponSetlaserTime();
	spin_unlock_irqrestore(&epon_reg_lock,flags); 

	return 0;
}

__u8 eponOnuMacAddr[6] = {0};

#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
extern uint8 noRxDscpInt;
extern uint8 attackEnhance;
extern unsigned int dropThred;
extern uint8 rxDropCheckTimer;
#endif
static struct timer_list rgstIntvalTimer;

void eponSetMpcpDiscStateToDefault(void)
{
	__u8 llidIndex = 0;	
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[0]);
	for(llidIndex = 0; llidIndex< EPON_1G_MAX_LLID_NUM; llidIndex++, llidEntry_p++)
	{
		llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_WAIT;
	}
}

void rgstIntvalTimerHandler(TIMER_FUN_PAAM date)
{
	__u8 llidIndex = 0;
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
	static uint8 timer_count=0;
 	static unsigned int oldDropCounter = 0;
	static unsigned int newDropCounter = 0;
#endif
	int rgs_flag = 0;

	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[0]);
	if(TRUE == gpPonSysData->ponMacPhyReset) {
		goto restart_timer;
	}

#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE) 
	if(timer_count == rxDropCheckTimer)
	{
		FE_API_GET_RX_DROP_FIFO_CNT(FE_GDM_SEL_GDMA2, (uint *)&newDropCounter);
	
		if(newDropCounter - oldDropCounter > dropThred)
		{
			printk("GDMA2_RX_OVDROPCNT:   newDropCounter = 0x%08x\n",newDropCounter);
			printk("GDMA2_RX_OVDROPCNT:   oldDropCounter = 0x%08x\n",oldDropCounter);
			noRxDscpInt = 1;
		}
		else
		{
			noRxDscpInt = 0;
		}
		oldDropCounter = newDropCounter;
		timer_count = 0;
	} 
	else
	{
		timer_count++;
	}

#endif

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter rgstIntvalTimerHandler phy ready is %d", eponReadyFlag);

	//check all LLID status ,who need discovery
	for(llidIndex = 0; llidIndex< EPON_1G_MAX_LLID_NUM; llidIndex++, llidEntry_p++){
		/* start epon case */
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "rgstIntvalTimerHandler start epon: llidIndex=%d state=%d rgstAgainTimeout = %d", llidIndex, llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState, llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout);

		if (llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout > 0) {
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "Onu is in silent time out is %d rest, delete 1",llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout );
			llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout--;
			rgs_flag = 1;
		}
		/* realy start epon */
 		if ((rgs_flag == 0) && (TRUE == XPON_PHY_GET(PON_GET_PHY_IS_SYNC)) && (MPCP_STATE_DENIED == llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState)) 
		{
			//eponReadyFlag = 0;
			eponPhyReadyRealStart();
			llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_REGISTERING;
			
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon phy up and silent time is 0, begin to response MPCP");
		}
	}


restart_timer:
	mod_timer(&rgstIntvalTimer, jiffies + (HZ ));

}

void eponSetFecAdjust(void)
{
	__u32 cfgValue = 0;

	cfgValue = READ_REG_WORD(e_rpt_cfg);
	cfgValue = (cfgValue & 0x0000ffff) | (eponDrv.eponFecAdjust << 16);
    WRITE_REG_WORD(e_rpt_cfg , cfgValue);

    return;
}

void eponEnable(void)
{
	unsigned long flags;

	spin_lock_irqsave(&epon_reg_lock,flags);

	/*Config EPON registers and Enable FE Channel*/
	eponRegAndFeChannelConfig();

	spin_unlock_irqrestore(&epon_reg_lock,flags);
	/* IRQ init */
#ifndef TCSUPPORT_CPU_ARMV8
	eponIrqRegister();
#endif

    return;
}
void eponDisable(void)
{
	change_pon_led_status(ALARM_LED_OFF);
	/* IRQ deinit */
#ifndef TCSUPPORT_CPU_ARMV8
	eponIrqUnregister();
#endif
	/* EPON MAC stop */
	eponWanStopWithChannelRetire();

    return;
}



/*****************************************************************************
*function :
*		eponFirmwareSetMode
*description : 
*		set Firmware in burst mode
*input :
*		0 : burst mode 1 : conutinue mode
*output :
*		N/A
******************************************************************************/
void eponFirmwareSetMode(int mode)
{
	if ( isEPONFWID )
	{
		XPON_PHY_SET_EPON_TS_CONTINUE_MODE(mode);
	}	
	return;
}

void eponSetIFG(unchar ifg_val)
{
	REG_e_tx_cal_cnst regRaw;
    memset(&regRaw, 0, sizeof(REG_e_tx_cal_cnst));
    
	regRaw.Raw = READ_REG_WORD(e_tx_cal_cnst);
	regRaw.Bits.default_ovrhd = (ifg_val & 0x3f);

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    /*set fec tail guard*/
    regRaw.Bits.fec_tail_grd = 0x0c;
#endif

	eponDbgPrint(EPON_DEBUG_LEVEL_NOTIFY, "eponSet IFG = %d,reg val:0x%08x", ifg_val,regRaw.Raw);	
	WRITE_REG_WORD(e_tx_cal_cnst , regRaw.Raw);	

    return;
}

void eponWanDevice(void){
	__u32 Raw = 0;
#ifdef TCSUPPORT_CPU_ARMV8
	Raw = GET_WAN_CONF();
	Raw = (Raw & 0xffffff00) | 0x01;
	SET_WAN_CONF(Raw);
#else
	Raw = READ_REG_WORD(0xbfb00070);
	Raw = (Raw & 0xfffffff8) | 0x01;
	IO_SREG(0xbfb00070,Raw);
#endif

	return;
}

void eponStart(unsigned long data)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponStart");

	eponWanDevice();
	
	/* set burst mode */
	eponFirmwareSetMode(0);	
	//tasklet_enable(&eponMacMpcpDscvGateTask);

	/*  Init all EPON ONU param */
	eponInitParam();

	eponEnable();

	return ;
}

int eponCleanAllLlidQueThrod(void ){
	__u8 queueIndex = 0;
	__u8 llidIndex = 0;
	__u8 setIndex = 0;
	eponQueueThreshold_t EponQThr;
	int ret = 0;
    
//	printk("eponCleanAllLlidQueThrod enter\n");
	for(llidIndex = 0 ;llidIndex < EPON_1G_MAX_LLID_NUM ;llidIndex++ ){
		for(setIndex = 0; setIndex< 3;setIndex++){
			for(queueIndex = 0; queueIndex< 8;queueIndex++){//set queue threshold
				EponQThr.channel = llidIndex;
				EponQThr.queue = queueIndex;
				EponQThr.thrIdx = setIndex;
				EponQThr.value = 0;

				ret = epon_set_queue_threshold(&EponQThr) ;
				if(ret < 0 ){
					dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponSetLlidQueThrod error");	
					return -1;
				}
			}
		}
	}
	return 0;
}


int eponDumpAllLlidQueThrod(void){
	__u8 queueIndex = 0;
	eponQueueThreshold_t EponQThr;
	int ret = 0;
	__u8 llidIndex = 0;
	__u8 setIndex = 0;
	
	printk("eponDumpAllLlidQueThrod enter\n");
	for(llidIndex = 0 ;llidIndex < EPON_1G_MAX_LLID_NUM ;llidIndex++ ){
		for(setIndex = 0; setIndex< 3;setIndex++){
			printk("\n ch%d,set%d : ",llidIndex,setIndex);
			for(queueIndex = 0; queueIndex< 8;queueIndex++){//set queue threshold
				memset(&EponQThr, 0, sizeof(QDMA_EponQueueThreshold_T));
				EponQThr.channel = llidIndex;
				EponQThr.queue = queueIndex ;
				EponQThr.thrIdx = setIndex;

				ret = epon_get_queue_threshold(&EponQThr);
				if(ret < 0){
					dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponGetLlidQueThrod error");	
					return -1;
				}
				printk("%x ",EponQThr.value);
			}
		}
		printk("\n");
	}
	return 0;
}

#if 0
int epon_set_llid_report_bitmap(__u8 llidIndex, __u8 bitmap){
	__u32 regAddr = 0;
	REG_e_report_ctrl rptCtrl;

	memset(&rptCtrl, 0, sizeof(REG_e_report_ctrl));

	switch(llidIndex){
		case 0:
		case 1:
			regAddr = (__u32)e_llid01_rpt_ctl;
			break;
		case 2:
		case 3:
			regAddr = (__u32)e_llid23_rpt_ctl;
			break;
		case 4:
		case 5:
			regAddr = (__u32)e_llid45_rpt_ctl;
			break;
		case 6:
		case 7:
			regAddr = (__u32)e_llid67_rpt_ctl;
			break;
		default:
			printk("ERROR ! llidIndex is %u\n",llidIndex);
			return -1;	
	}

	rptCtrl.Raw = READ_REG_WORD(regAddr);
	if((llidIndex % 2) == 0){
		rptCtrl.Bits.even_llid_rpt_bitmap_ctrl = 1;
		rptCtrl.Bits.even_llid_rpt_bitmap_set = bitmap;
	}else{
		rptCtrl.Bits.odd_llid_rpt_bitmap_ctrl = 1;
		rptCtrl.Bits.odd_llid_rpt_bitmap_set = bitmap;
	}

	WRITE_REG_WORD(regAddr, rptCtrl.Raw);

	return 0;
}
#endif

int epon_get_llid_report_bitmap(__u8 llidIndex, __u8* bitmap){
	__u32 regAddr = 0;
	REG_e_report_ctrl rptCtrl;

	memset(&rptCtrl, 0, sizeof(REG_e_report_ctrl));

	switch(llidIndex){
		case 0:
		case 1:
			regAddr = (__u32)e_llid01_rpt_ctl;
			break;
		case 2:
		case 3:
			regAddr = (__u32)e_llid23_rpt_ctl;
			break;
		case 4:
		case 5:
			regAddr = (__u32)e_llid45_rpt_ctl;
			break;
		case 6:
		case 7:
			regAddr = (__u32)e_llid67_rpt_ctl;
			break;
		default:
			printk("ERROR ! llidIndex is %u\n",llidIndex);
			return -1;		
	}

	rptCtrl.Raw = READ_REG_WORD(regAddr);
	if((llidIndex % 2) == 0){
		*bitmap = rptCtrl.Bits.even_llid_rpt_bitmap_set;
	}else{
		*bitmap = rptCtrl.Bits.odd_llid_rpt_bitmap_set;
	}

	return 0;
}

int epon_set_queue_threshold(eponQueueThreshold_t *pEponQThr)
{
	unsigned int eponThresholdCfg = 0 ;
	int RETRY = 3 ;
	
	if (pEponQThr->channel>=8 || pEponQThr->queue>=8 || pEponQThr->thrIdx>=3) 
	{
		return -1 ;
	}
	
	eponThresholdCfg = (EPON_QTHRESHLD_RWCMD | 
				(((pEponQThr->value)<<EPON_QTHRESHLD_VALUE_SHIFT)&EPON_QTHRESHLD_VALUE_MASK) |
				(((pEponQThr->thrIdx)<<EPON_QTHRESHLD_TYPE_SHIFT)&EPON_QTHRESHLD_TYPE_MASK) |
				(((pEponQThr->channel)<<EPON_QTHRESHLD_CHANNEL_SHIFT)&EPON_QTHRESHLD_CHANNEL_MASK) |
				(((pEponQThr->queue)<<EPON_QTHRESHLD_QUEUE_SHIFT)&EPON_QTHRESHLD_QUEUE_MASK)) ;

//	printk("epon_set_queue_threshold: eponThresholdCfg = %x\n", eponThresholdCfg);
	WRITE_REG_WORD(e_rpt_qthld_cfg , eponThresholdCfg);
	
	while(RETRY--) 
	{
		eponThresholdCfg = READ_REG_WORD(e_rpt_qthld_cfg) ;
		
		if(eponThresholdCfg&EPON_QTHRESHLD_DONE) 
		{
			return 0 ;
		}
		udelay(1) ;
	}

	printk("Timeout for set EPON Threshold configuration.\n") ;
	
	return -1 ;
}

int epon_get_queue_threshold(eponQueueThreshold_t *pEponQThr)
{
	int ret ;
	unsigned int eponThresholdCfg = 0 ;
	int RETRY = 3 ;
	
	if(pEponQThr->channel>=8 || pEponQThr->queue>=8 || pEponQThr->thrIdx>=3) 
	{
		return -1 ;
	}

	eponThresholdCfg = ((((pEponQThr->thrIdx)<<EPON_QTHRESHLD_TYPE_SHIFT)&EPON_QTHRESHLD_TYPE_MASK) |
				(((pEponQThr->channel)<<EPON_QTHRESHLD_CHANNEL_SHIFT)&EPON_QTHRESHLD_CHANNEL_MASK) |
				(((pEponQThr->queue)<<EPON_QTHRESHLD_QUEUE_SHIFT)&EPON_QTHRESHLD_QUEUE_MASK)) ;
	WRITE_REG_WORD(e_rpt_qthld_cfg, eponThresholdCfg) ;

	while(RETRY--) 
	{
		eponThresholdCfg = READ_REG_WORD(e_rpt_qthld_cfg) ;
		
		if(eponThresholdCfg&EPON_QTHRESHLD_DONE) 
		{
			ret = ((eponThresholdCfg&EPON_QTHRESHLD_VALUE_MASK)>>EPON_QTHRESHLD_VALUE_SHIFT) ;
			if (ret >= 0)
			{
				pEponQThr->value = ret ;
				return 0;
			}
			else
				return ret ;
		} 
		udelay(1) ;
	}

	printk("Timeout for get EPON Threshold configuration.\n") ;
	
	return -1 ;
}


void eponStop(void)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponStop");
	eponDisable();
	XPON_PHY_COUNTER_CLEAR(0x1f);
	
	return;
}



struct file_operations eponMacFops = {
	.owner =		THIS_MODULE,
	.write =		NULL,
	.read =			NULL,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl =	eponMacIoctl,
#else	
	.ioctl =		eponMacIoctl,
#endif	
	.open =			eponMacOpen,
	.release =		NULL,
};


int eponMacOpen(struct inode *inode, struct file *filp)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMacOpen entered");
	return 0;
}

/*****************************************************************************
******************************************************************************/

#define TYPEB_SUCCESS   1
#define TYPEB_FAIL  0

void eponDetectPhyLosHandler(void)
{   
    gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_LOS, 0) ;

    #if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_LOS) ;
    #endif/*TCSUPPORT_COMPILE*/

#ifdef EPON_MAC_HW_TEST
	if ((ignoreIntMask &0x8000) == 0){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "ignore epon phy los");
		return;
	}
#endif

	eponReadyFlag = 0; /* clean this epon start flag */

	dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr Epon Phy Los");

	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
#ifdef TCSUPPORT_AUTOBENCH
#ifdef TCSUPPORT_CPU_ARMV8
	if((GET_WAN_CONF() & 0xff) == 1)
#else
	if((IO_GREG(0xbfb00070) & 0x7) == 1)
#endif
#endif
	{
		eponStop();
	}
	gpWanPriv->activeChannelNum = 1;	
	xpon_reset_qdma_tx_buf();
	
	gpPonSysData->sysPrevLink = PON_LINK_STATUS_EPON ;

	eponFirmwareSetMode(0);
	
    stop_omci_oam_monitor();

	oam_check_txrx_reset(EPON_RXTX_TIMER_STOP);
	dbgtoMem(EPON_DEBUG_LEVEL_ERR, "%s[%d]Phy LOS, stop timer.\n", __FUNCTION__, __LINE__);
    return;
}

static void epon_los_timer_func(TIMER_FUN_PAAM arg)
{
	eponDrv.typeBOnGoing = FALSE;
    dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr typeB time out");
	if(eponDrv.hldoverEnable && (XPON_PHY_GET(PON_GET_PHY_IS_SYNC) == PHY_TRUE))
		return;
	eponDetectPhyLosHandler();
}

static int epon_typeB_phy_los(void)
{
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[0]);	
	eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
	int ret = TYPEB_FAIL;
	if(eponDrv.hldoverEnable && (mpcpDiscFsm->mpcpDiscState == MPCP_STATE_REGISTERED))
	{
		ret = TYPEB_SUCCESS;
		gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
		dbgtoMem(EPON_DEBUG_LEVEL_ISR ,"EponIsr Epon typeB Phy Los");
		EPON_STOP_TIMER(gEponTypeBTimer);
		EPON_START_TIMER(gEponTypeBTimer, eponDrv.hldOverTime);
		eponDrv.typeBOnGoing = TRUE;
	}
	return ret;
}

void eponDetectPhyLosLof(void)
{
    if(turnOffOpticalModuel)
	{
		change_alarm_led_status(ALARM_LED_ON);
	}
	else
	{
		change_alarm_led_status(ALARM_LED_FLICKER);
	}
	if(TYPEB_FAIL == epon_typeB_phy_los()){
		eponDetectPhyLosHandler();
	}
}

/* start epon mpcp start */
void eponPhyReadyRealStart(void)
{
#ifdef EPON_MAC_HW_TEST
	if ((ignoreIntMask &0x8000) == 0){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"ignore epon phy ready");
		return;
	}
#endif

	dbgtoMem(EPON_DEBUG_LEVEL_ISR,"EponIsr Epon Phy Ready");
	
	if(gpPonSysData->sysPrevLink == PON_LINK_STATUS_GPON) {
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_MODE_CHANGE, 0);
	}
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_PHY_READY, 0);
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;
		tasklet_schedule(&eponStartTask);
		gpWanPriv->devCfg.flags.isTxDropOmcc = 0;
	}
}

extern int isSlientBegin;

void eponDetectPhyReady(void)
{
	eponReadyFlag = 1;	
#ifdef EPON_MAC_HW_TEST
	if ((ignoreIntMask &0x8000) == 0){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "ignore epon phy ready");
		return;
	}
#endif

#if defined(TCSUPPORT_CT_PON_CN_CN) 
	if (TRUE == XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )
#endif
    if(!turnOffOpticalModuel)
	{
		change_alarm_led_status(ALARM_LED_OFF);
	}

	dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr Epon Phy Ready");

	if(FALSE == eponDrv.typeBOnGoing){
		/* set burst mode */
		eponFirmwareSetMode(0);
	}
	
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_PHY_READY, 0);
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;
		if(FALSE == eponDrv.typeBOnGoing){
			tasklet_schedule(&eponStartTask);
		}
	}
}
/*****************************************************************************
******************************************************************************/

static int get_oam_swtx_count(u32 * pkt_cnt)
{
	PWAN_NetPriv_T *tmpPriv = NULL;
    struct net_device *dev  = NULL;

    dev = gpWanPriv->pPonNetDev[PWAN_IF_OAM];
    if(NULL == dev)
    {
        printk("[%s %d]ERR: dev is null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    tmpPriv = (PWAN_NetPriv_T *)netdev_priv(dev);
    if(NULL == tmpPriv)
    {
        printk("[%s %d]ERR: data is null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    
    *pkt_cnt = tmpPriv->stats.tx_packets;
    return 0;
}


static void get_oam_trx_count(TIMER_FUN_PAAM arg)
{
    if(g_txOamStats.tryCnt == gpEponPriv->maxTryCnt)
    {
        if((0==g_txOamStats.hwFlg)&&(0!=g_txOamStats.swFlg))
        {
            if(0 == gpEponPriv->BitmapFlag)
            {
                gpEponPriv->BitmapFlag = 1;
                printk("Bitmap dummy enable\n");
            }
        }

        memset(&g_txOamStats, 0, sizeof(g_txOamStats));
        return;
    }

    g_txOamStats.hwNxt = (READ_REG_WORD(e_oam_stat))&(0xffff);
    
    if(g_txOamStats.hwNxt != g_txOamStats.hwPre)
    {
        g_txOamStats.hwFlg++;
    }
    
    g_txOamStats.hwPre = g_txOamStats.hwNxt;
    
    if( 0!= get_oam_swtx_count(&g_txOamStats.swNxt ) )
    {
        return;
    }
    
    if(g_txOamStats.swNxt != g_txOamStats.swPre)
    {
        g_txOamStats.swFlg++;
    }
    
    g_txOamStats.swPre = g_txOamStats.swNxt;
    g_txOamStats.tryCnt++;
    XPON_START_TIMER(gpEponPriv->oamCntTimer,1000);
    return;
}

void oam_check_txrx_reset(int stop_flag){
	oam_tx_cnt = 0;
	oam_rx_cnt = 0;
	enum_rx = 0;
	if(stop_flag == 0){
		eponMacTxOamFav(1);
		oam_fav_set_disabled = 0;
		rcv_srv_oam_flag = 0;
		keepalive_count = 0;
		EPON_STOP_TIMER(eponCheckTxRxTmr);	
	}
	return;
}

static void epon_check_oamTxRx(TIMER_FUN_PAAM arg)
{	
	XPON_DEBUG_STATISTIC tmpReg;

	__u32 oam_tx_prv = oam_tx_cnt;
	__u32 oam_rx_prv = oam_rx_cnt;
	
	memset(&tmpReg,0,sizeof(XPON_DEBUG_STATISTIC));
	
	if(gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON){

		get_register_count(&tmpReg);
	}else{
		oam_check_txrx_reset(EPON_RXTX_TIMER_CONTINUE);
		
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "%s[%d]It's GPON, can't access the register.\n", __FUNCTION__, __LINE__);
	}

	oam_tx_cnt = tmpReg.txOamCnt;
	oam_rx_cnt = tmpReg.rxOamCnt;

	if((oam_tx_cnt-oam_tx_prv) == 0){
		enum_rx += (oam_rx_cnt - oam_rx_prv);
		
		printk("%s[%d]Unusual OAM Tx = %d, Rx = %d.\n", __FUNCTION__, __LINE__, oam_tx_cnt, oam_rx_cnt);
		
		if(enum_rx >= epon_rx_diff){
			
			printk("%s[%d]OAM statistic is unusual, reset WAN.\n", __FUNCTION__, __LINE__);
			
			eponWanResetWithChannelRetire();
			oam_check_txrx_reset(EPON_RXTX_TIMER_CONTINUE); 
			oam_issue_flag = 1;
		}else{
		
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "%s[%d]OAM rx diff is less than threshold, no need to reset yet.\n", __FUNCTION__, __LINE__);
		}
	}else{
		enum_rx = 0;
		
		dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "%s[%d]OAM statistic is usual, restart timer. Tx = %d, Rx = %d\n", __FUNCTION__, __LINE__, oam_tx_cnt, oam_rx_cnt);
	}

	EPON_START_TIMER(eponCheckTxRxTmr,EPON_CHECK_TXRX_FRQ);
	
	return;
}

static void sendQueuePkt(unsigned long data)
{
    struct sk_buff * skb = NULL;
    struct ethhdr * ethhdr = NULL;
    PWAN_FETxMsg_T txBmMsg;
    int queue = 0;
    int ret = 0;

    memset(&txBmMsg, 0, sizeof(PWAN_FETxMsg_T));
    
    for(queue = 0; queue < 7; queue++)
    {
        skb = skbmgr_alloc_skb2k();
        if(!skb){
            printk("%s[%d]ERR:alloc skb\n", __FUNCTION__, __LINE__);
            return;
        }

        ethhdr = (struct ethhdr * )(skb->data);
        memcpy(ethhdr->h_source, eponOnuMacAddr, sizeof(eponOnuMacAddr));
        memset(ethhdr->h_dest, 0, sizeof(eponOnuMacAddr));
        skb->len = 1500;

        skb->mark &= (~QOS_FILTER_MARK);
        skb->mark |= (queue<<4);

        txBmMsg.raw.fport = TXMSG_FPORT_GMAC;
        txBmMsg.raw.oam = 0;
        txBmMsg.raw.queue = queue;
        txBmMsg.raw.channel = gpEponPriv->oamChannel;

        ret = QDMA_API_TRANSMIT_PACKETS(ECNT_QDMA_WAN, skb, txBmMsg.word[0], txBmMsg.word[1], NULL);
        if(ret != 0){
            dev_kfree_skb_any(skb);
            printk("[%s %d]ERR: QDMA_API_TRANSMIT_PACKETS\n", __FUNCTION__, __LINE__);
        }      
    }

    return;
}

int bitmap_format_convert(void)
{
    if(gpEponPriv->BitmapFlag){
        tasklet_schedule(&gpEponPriv->epon_bitmap_format_tasklet);
    }

    return 0;
}

#ifdef TCSUPPORT_EPON_POWERSAVING
void epon_early_wakeup_expires(unsigned long arg)
{
	__u32 Raw = 0;
	Raw = READ_REG_WORD(e_pwr_sv_cfg);
	Raw |= (1<<25); /* onu wakeup */
	WRITE_REG_WORD(e_pwr_sv_cfg,Raw);

	dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "ONU manual wakeup!\n");
}

int eponSetEarlyWakeUpConfig(__u8 enable, __u32 timer, __u16 count)
{
	gpEponPriv->eponCfg.earlyWakeupFlag = enable;
	gpEponPriv->eponCfg.earlyWakeupTimer = timer;
	gpEponPriv->eponCfg.earlyWakeupCount = count;
	
	return 0;
}
#endif

void eponFeChannelEnable(void){
    int i = 0;

    /*Enable MAC tx_stop and rx_stop*/
    eponMacTxRxDisable();
    
    for(i = 0; i < 8; i++) {
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE);
		FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX, i, FE_ENABLE);
		FE_API_SET_HWFWD_CHANNEL(FE_CDM_SEL_CDMA2, i, FE_ENABLE);
	}

	if (gpEponPriv->eponCfg.txOamFavorMode) {
		for (i=16;i<24;i++) {
			/* enable high 16bit TX channel for enable TX favor OAM mode */
			FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE); 
		}
	}

    /*Disable MAC Tx_stop and Rx_stop*/
    eponMacTxRxEnable();
    
    return;
}

void eponMacReset(void){
    __u32 valueRst = 0;
    
    if(!no_epon_mac_reset)
    {
    	eponUpdateStackMpcpErrCnt();
#ifdef TCSUPPORT_CPU_ARMV8
        /* reset mac */
        valueRst = GET_SCU_RSTCTRL1();
        valueRst |= SCU_EPON_MAC_RESET  ;
        SET_SCU_RSTCTRL1(valueRst);
        udelay(1);
        /* release mac */
        valueRst = GET_SCU_RSTCTRL1();
        valueRst &= ~SCU_EPON_MAC_RESET ;
        SET_SCU_RSTCTRL1(valueRst);
#else
        valueRst = READ_REG_WORD(REG_E_SW_RST);
        valueRst |= (1<<31);
        WRITE_REG_WORD(REG_E_SW_RST , valueRst);
        udelay(1);
        valueRst &= 0x7fffffff;
        WRITE_REG_WORD(REG_E_SW_RST , valueRst);
#endif
    }

    return;
}

void eponGlbMacCtrlInit(void){
    REG_e_glb_cfg global_cfg;

    memset(&global_cfg, 0, sizeof(REG_e_glb_cfg));
    
    /*reset mpcp_cmd_done bit(bfb66028[16] = 1)*/
    global_cfg.Raw = READ_REG_WORD(e_glb_cfg);
    global_cfg.Bits.epon_mac_sw_rst = 1;
    WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);
    global_cfg.Bits.epon_mac_sw_rst = 0;
    WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);

    dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponGlbMacCtrlInit: e_llid_dscvry_ctrl=0x%x\n",
        READ_REG_WORD(e_llid_dscvry_ctrl));
    
    /*  send controll packet first */
    global_cfg.Bits.rpt_txpri_ctrl = 1;
    /* loc_cnt_sync_method 1 */
    global_cfg.Bits.loc_cnt_sync_method = 1;
    /* When send REG_req, MAC assert burst_en(burst mode) to PHY */
    global_cfg.Bits.discv_burst_en = 1;
    WRITE_REG_WORD(e_glb_cfg , global_cfg.Raw);

    return;
}

void eponIntEnableInit(void){
    __u32 intEnableMask = 0;
    __u32 llidMask = eponDrv.llidMask;
    
    //enble INT mask
	intEnableMask |= (RPT_OVERINTVL_INT | MPCP_TIMEOUT_INT | TIMEDRFT_INT | TOD_1PPS_INT | TOD_UPDT_INT | GNT_BUF_OVRRUN_INT | RCV_DSCVRY_GATE_INT);
#ifdef TCSUPPORT_EPON_POWERSAVING
	intEnableMask |= (PS_EARLY_WAKEUP_INT | RX_SLEEP_ALLOW_INT | PS_WAKEUP_INT | PS_SLEEP_INT);
#endif
	if (gpEponPriv->mpcpInterruptMode){
		intEnableMask |= (REG_REQ_DONE_INT | REG_ACK_DONE_INT);
	}

    /*Enable LLID_n receive mpcp register interrupt*/
    intEnableMask |= ((llidMask << 1) & 0x1fe);
    
	WRITE_REG_WORD(e_int_en , intEnableMask);

    return;
}

int eponLlidMacInit(void){
    __u32 llidIndex = 0;
    eponLlid_t *llidEntry_p = NULL;

    while(llidIndex < EPON_1G_MAX_LLID_NUM){
        if((eponDrv.llidMask) & (1 << llidIndex)){
            llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
            if(eponMacSetMacAddr(llidEntry_p) < 0){
        		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr fail llidIndex=%d", llidIndex);
        		return -1;
        	}


        	if(eponDebugLevel >= EPON_DEBUG_LEVEL_NOTIFY){
        	    eponMacGetMacAddr(llidEntry_p);
        	}
        }
        llidIndex++;
    }

    return 0;
}

void eponMpcpReportInit(void){
#ifdef CONFIG_USE_MT7520_ASIC 
    if (eponStaticRptEnable == 1){
        eponSetStaticReport();
    }else{
        /*If QDMA do not response to MAC report request, MAC will send static report, workaround for OSBNB00090282*/
        /*Notice:FPGA do not use this function!!!*/
        eponSetReportData();
    }
#endif

#ifdef TCSUPPORT_CPU_EN7528
    /*config EN7528 MPCP report*/
    WRITE_FE_REG(0xbfb55630,0);
    WRITE_FE_REG(0xbfb55634,0xcf506);
    WRITE_REG_WORD(e_tx_cal_cnst,0x2612010c);
#endif
    
    if(isEN7523 || isAN7552){
        /*config EN7523 MPCP report*/
        WRITE_FE_REG(0xbfb57130,0);
        /*report size overhead config,bfb57134[15:8]== bfb660d8[15:8],bfb57134[7:0] = bfb660d8[23:16]*/
        WRITE_FE_REG(0xbfb57134,0x010c);
        /*set the grant required for report message,fec on:bfb66160[29:24]=0x38 TQ,fec off:bfb66160[23:16]=0x2b TQ*/
        WRITE_REG_WORD(e_snf_mpcp_oam_ctl,0x382b0000);
    }

    eponSetIFG(EPON_IFG_DEFAULT);
    eponSetFecAdjust();

    eponSetLlid0DeafultThreshold();
    
    return;
}

void eponRegInitConfig(void){
    __u8 snifferModeEnable = 0;

    /*Enable MAC tx_stop and rx_stop*/
    eponMacTxRxDisable();

    /*Epon global mac control register config*/
    eponGlbMacCtrlInit();

    /*Enable EPON interrupts*/
    eponIntEnableInit();

    eponMpcpReportInit();

    eponLlidMacInit();

    eponSetlaserTime();
    
#ifdef EPON_MAC_HW_TEST
    if(eponSetProbe != 0){
        WRITE_REG_WORD( e_dbg_prb_sel,eponSetProbe ) ;
    }
#endif
    
#ifdef CONFIG_USE_MT7520_ASIC 
     WRITE_REG_WORD(e_txfetch_cfg, 0x202403e8);
#else
    WRITE_REG_WORD(e_txfetch_cfg, 0x242a03e8);  //0x1a2603e8
#endif
    
    
    /* 7521 synctime always have, don't need this */
    setMpcpFwd(gpEponPriv->fwdFlag);

    if(sniffer_flag){
		WRITE_REG_WORD(e_glb_cfg ,sniffer_config);
		WRITE_REG_WORD(e_sniff_sp_tag ,special_tag);
	}

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	if(randomAdjust_flag){
        REG_e_dscv_rdmdly_ctl rdm_dly_ctl;
        memset(&rdm_dly_ctl, 0, sizeof(rdm_dly_ctl));
        rdm_dly_ctl.Bits.rdm_dly_mode = 1;
        rdm_dly_ctl.Bits.rdm_dly_len = (random_config & 0xffff);
		WRITE_REG_WORD(e_dscv_rdmdly_ctl,rdm_dly_ctl.Raw);
	}

	if(minus_disable){
		WRITE_REG_WORD(e_snf_mpcp_oam_ctl, 0x382b0010);
	}
#endif

    if (gpPonSysData->dyingGaspData.isEponHwFlag)
		eponHwDygaspCtrl(XPON_ENABLE);
	else
		eponHwDygaspCtrl(XPON_DISABLE);

	/*Init EPON dying gasp pakets number*/
	eponDevSetDyingGaspNum(EPON_DYING_GASP_NUM);
	
	if (gpEponPriv->eponCfg.txOamFavorMode)
		eponTxOamFavorModeCtl(XPON_ENABLE);
	else
		eponTxOamFavorModeCtl(XPON_DISABLE);

	snifferModeEnable = gpEponPriv->eponCfg.snifferModeConfig.snifferModeEnable;
	eponSnifferModeCtrl(snifferModeEnable);

    set_fix_reg_list();
    
    /*Notice: config other reg before this line !!!*/
    /*Disable MAC Tx_stop and Rx_stop*/
    eponMacTxRxEnable();
    
    return;
}

void eponRegAndFeChannelConfig(void){
    /*Config EPON registers when phy is up*/
	eponRegInitConfig();
    
    eponFeChannelEnable();

    return;
}

void eponMacReinit(int channelRetireEnable){
    unsigned long flags;
	
	change_pon_led_status(ALARM_LED_OFF);


    spin_lock_irqsave(&epon_reg_lock,flags);
    
    eponMacTxRxDisable();

    /*FE channel retire*/
    if(channelRetireEnable){
	    FEChannelRetire();
    }
	eponMacStop();

    eponInitAllLlid();

    /*Config EPON registers and Enable FE Channel*/
    eponRegAndFeChannelConfig();
    
    eponMacTxRxEnable();
    spin_unlock_irqrestore(&epon_reg_lock,flags);

    return;
}

static int epon_init_private_data(EPON_GlbPriv_T *epon_priv_p)
{
	int ret = 0 ;
	int i = 0;
#ifdef TCSUPPORT_EPON_POWERSAVING
	epon_priv_p->eponCfg.earlyWakeupTimer = 50;
	epon_priv_p->eponCfg.earlyWakeupFlag = 0;
	epon_priv_p->eponCfg.earlyWakeupCount = 0;
	sleepCount = 0;
#endif

	epon_priv_p->eponCfg.snifferModeConfig.snifferModeEnable = 0;
	epon_priv_p->eponCfg.snifferModeConfig.snifferModeLanMask = 0;
	if(isEN7523 || isAN7552){
        epon_priv_p->eponCfg.txOamFavorMode= 1;
    }else{
	    epon_priv_p->eponCfg.txOamFavorMode= 0;
    }
    
	if (isEN7526c || isEN7523 || isAN7552) {
		epon_priv_p->mpcpInterruptMode = 1;
	}else{
        epon_priv_p->mpcpInterruptMode = 0;
    }

    epon_priv_p->gBitmapSet = 0;
    epon_priv_p->BitmapFlag = 0;
    epon_priv_p->maxTryCnt = 3;
    epon_priv_p->oamChannel = 0;
	
	for(i = 0;i < EPON_1G_MAX_LLID_NUM;i++){
		g_epon_llid_dscv_stat[i] = (__u32)e_llid_dscvry_sts(i);
	}

	epon_priv_p->eponRdmMfy.grtFlag = 0;
    epon_priv_p->eponRdmMfy.grtLen = 0;
    epon_priv_p->eponRdmMfy.grtMode = 0;
    epon_priv_p->fwdFlag = 0;
    epon_priv_p->eponRdmMfy.mpcpGrantLen = 0x800;
    epon_priv_p->eponRdmMfy.grtLenStep = 0x50;
    
	return ret;
}

int eponInit(void)
{
	int ret = -1;
	XMCSIF_OnuType_t type;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponInit");	

	spin_lock_init(&epon_reg_lock);

	/* Base Register of  EPON */
#ifdef TCSUPPORT_CPU_ARMV8
	g_EPON_MAC_BASE = 0x0;
#else
	g_EPON_MAC_BASE = (PEPON_MAC_REGS)(ioremap_nocache(CONFIG_EPON_BASE_ADDR, CONFIG_EPON_REG_RANGE)) ; 
	if(!g_EPON_MAC_BASE) {
		dbgtoMem(EPON_DEBUG_LEVEL_ERR ,"ERR: ioremap the EPON base address failed:addr=0x%x,size=0x%x",CONFIG_EPON_BASE_ADDR,CONFIG_EPON_REG_RANGE) ;
		return -ENOMEM ;
	}
#endif
	ret = epon_init_private_data(gpEponPriv);
	if (ret < 0) {
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "EPON private data initialization failed");
		return ret;
	}
		
	eponMacTableInit();

	getOnuMacAddr(eponOnuMacAddr);
#ifdef TCSUPPORT_PON_TEST
	memcpy(eponDrvTmpMacAddr, eponOnuMacAddr, 6);
	eponDrvDbgMask = 0;
#endif

	/* cmd init */
	eponCmdInit();
	/* init epon driver */	
	memset(&eponDrv , 0, sizeof(epon_t));

	eponFirmwareSetMode(0);
	
	ret = register_chrdev(EPON_MAC_MAJOR, "epon_mac", &eponMacFops);
	if (ret < 0)
		return ret;

	tasklet_init(&eponMacMpcpDscvGateTask, eponMpcpDiscvGateIntHandler, 0);	
	tasklet_init(&eponStartTask, eponStart, 0);	
	
	eponMacAddTimer(&rgstIntvalTimer,1,rgstIntvalTimerHandler,0);

	xmcs_get_onu_type(&type);
	if (type != XMCS_IF_ONU_TYPE_HGU)
	{
		isSfu = 1;
	}
	else
	{
		isSfu = 0;	
	}

	EPON_CREATE_TIMER(&gEponTypeBTimer,epon_los_timer_func,1000);

#ifdef TCSUPPORT_EPON_POWERSAVING
	EPON_CREATE_TIMER(&gpEponPriv->early_wakeup_timer,epon_early_wakeup_expires,gpEponPriv->eponCfg.earlyWakeupTimer);
#endif

    tasklet_init(&gpEponPriv->epon_bitmap_format_tasklet, sendQueuePkt, 0);
	dbgPrintInit();

    memset(&g_txOamStats, 0, sizeof(g_txOamStats));
	EPON_CREATE_TIMER(&gpEponPriv->oamCntTimer,get_oam_trx_count,1000);
    
    memset(&eponTxPwrDwnTmr, 0, sizeof(struct timer_list));
	EPON_CREATE_TIMER(&eponTxPwrDwnTmr,eponPhyTxPwrUp,0);

	memset(&eponCheckTxRxTmr, 0, sizeof(struct timer_list));
	EPON_CREATE_TIMER(&eponCheckTxRxTmr,epon_check_oamTxRx,1000);
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	eponDrv.eponFecAdjust = 0x0;
#else
	eponDrv.eponFecAdjust = 0x1f;
#endif

	eponDrv.DefaultThreshold = EPON_DEFAULT_THRESHOLD;

	return ret;
}

void eponExit(void)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponExit");

	/* cmd exit */
	eponCmdExit();
	
	unregister_chrdev(EPON_MAC_MAJOR, "epon_mac");
	
	tasklet_kill(&eponMacMpcpDscvGateTask);
	tasklet_kill(&eponStartTask);
    tasklet_kill(&gpEponPriv->epon_bitmap_format_tasklet);
    
	eponMacDelTimer(&rgstIntvalTimer);
#ifdef TCSUPPORT_EPON_POWERSAVING
	EPON_STOP_TIMER(gpEponPriv->early_wakeup_timer) ;
#endif

	dbgPrintQuit();

	if(g_EPON_MAC_BASE) {
		iounmap(g_EPON_MAC_BASE) ;
		g_EPON_MAC_BASE = NULL ;
	}
	eponMacTableExit();

	EPON_STOP_TIMER(gEponTypeBTimer);
    EPON_STOP_TIMER(gpEponPriv->oamCntTimer);
    EPON_STOP_TIMER(eponTxPwrDwnTmr);
	EPON_STOP_TIMER(eponCheckTxRxTmr);

	return;
}
