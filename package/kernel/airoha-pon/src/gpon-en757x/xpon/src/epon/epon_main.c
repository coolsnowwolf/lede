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
#ifdef TCSUPPORT_EPON_DUMMY
#include <linux/jiffies.h>
#endif

#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>

#include "epon/epon.h"
#include "../../inc/common/xpondrv.h"
#include "../../inc/common/xpon_led.h"


#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_qdma.h>
#include "qdma_bmgr.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#else
#include "qdma_api.h"
#endif

#ifdef EPON_MAC_HW_TEST
#include "epon/epon_hwtest.h"
#endif

#include "common/drv_global.h"

#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
#include "epon/fe_reg.h"	
#endif

#include "../../inc/common/phy_if_wrapper.h"
#include "../../inc/xmcs/xmcs_if.h"
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
extern int led_xpon_status;
#endif

#include <linux/ktime.h>

PEPON_MAC_REGS g_EPON_MAC_BASE = NULL;
#ifdef TCSUPPORT_PON_TEST
__u32 eponDrvDbgMask = 0;
__u8 eponDrvTmpMacAddr[6] = {0};
#endif
u8 isSfu = 1;

spinlock_t epon_reg_lock;

extern void eponPhyReadyRealStart(void);
int eponReadyFlag = 0;

extern __u8 g_silence_time;
static struct timer_list gEponTypeBTimer;

static txOamCnt_S g_txOamStats= {0};

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
#ifndef TCSUPPORT_CPU_EN7521
extern void (*fe_checkEponMacRestart)(void);
#endif
static int eponCmdInit(void);
static void eponCmdExit(void);
static int doEponMpcp(int argc, char *argv[], void *p);
static int doEponSet(int argc, char *argv[], void *p);
static int doEponDebuglevel(int argc, char *argv[], void *p);
static int doEponDeregister(int argc, char *argv[], void *p);
int eponMacOpen(struct inode *inode, struct file *filp);
void attack_protect_set(int active, int mode);

#ifdef EPON_MAC_HW_TEST
static int doEponRegtest(int argc, char *argv[], void *p);
static int doEponHwtest(int argc, char *argv[], void *p);
static int doEponRegDefCheck(int argc, char *argv[], void *p);
#endif

static int doEponSetTmdrftflag(int argc, char *argv[], void *p);
static int doEponDumpAllReg(int argc, char *argv[], void *p);

#ifdef EPON_MAC_HW_TEST
static int doEponSetProbe(int argc, char *argv[], void *p);
static int doEponResetMac(int argc, char *argv[], void *p);
static int doEponResetWan(int argc, char *argv[], void *p);
static int doEponSetRgstAckType(int argc, char *argv[], void *p);
static int doEponSetIgnoreIntMask(int argc, char *argv[], void *p);
static int doEponSetDefLlidMask(int argc, char *argv[], void *p);

#endif

#ifdef TCSUPPORT_CPU_EN7521
static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p);
static int doEponSetLlidQueThrshld(int argc, char *argv[], void *p);
#endif

static int get_oam_swtx_count(u32 * pkt_cnt);
static int get_oam_trx_count(void);
extern __u8 g_silence_time;

//int eponMacReset = 0;
//struct task_struct eponMacTmDrfTask;
//struct task_struct eponMacMpcpTmOutTask;
/*
struct task_struct eponMacMpcpRgstTask_0;
struct task_struct eponMacMpcpRgstTask_1;
struct task_struct eponMacMpcpRgstTask_2;
struct task_struct eponMacMpcpRgstTask_3;
struct task_struct eponMacMpcpRgstTask_4;
struct task_struct eponMacMpcpRgstTask_5;
struct task_struct eponMacMpcpRgstTask_6;
struct task_struct eponMacMpcpRgstTask_7;
*/
struct tasklet_struct eponMacMpcpDscvGateTask;
struct tasklet_struct eponStartTask;//task_struct

static const cmds_t eponMpcpCmds[] = {
	{"deregister",		doEponDeregister,		0x02,  	1,  "<deregister>"},
	{"tmdrftflag",		doEponSetTmdrftflag, 	0x02,	1,	"<tmdrftflag>"},
#ifdef EPON_MAC_HW_TEST
	{"setprobe",		doEponSetProbe, 	0x02,	1,	"<probe>"},
	{"wanreset",		doEponResetWan, 	0x02,	0,	"<wanreset>"},
	{"macreset",		doEponResetMac, 	0x02,	0,	"<macreset>"},
	{"rgstack",		doEponSetRgstAckType, 	0x02,	0,	"<llid,ack>"},
	{"ignoreIntMask",		doEponSetIgnoreIntMask, 	0x02,	0,	"<mask(32bit)>"},
	{"setDefLlidMask",		doEponSetDefLlidMask, 	0x02,	1,	"<mask(16bit)>"},
#endif
	{NULL,			NULL,					0x10,	0,	NULL},
};

static const cmds_t eponSetCmds[] = {
#ifdef TCSUPPORT_CPU_EN7521
	{"llidThrshldNum",		doEponSetLlidThrshldNum,		0x02,  	1,  "<llidIndex(0~7)> <llidThrshldNum(1~3)>"},
	{"llidQueThrshld",		doEponSetLlidQueThrshld, 	0x02,	1,	"<llidIndex(0~7)> <thrshldIndex(0~2)> <q0 value> ... <q7 value>"},
#endif
	{NULL,			NULL,					0,	0,	NULL},
};


#ifdef EPON_MAC_HW_TEST
static const cmds_t eponHwtestCmds[] = {
	{"regtest",		doEponRegtest,		0x02,  	1,  "<times>"},
	{"regDefCheck",	doEponRegDefCheck,		0x02,  	0,  "<regDefCheck>"},
	{NULL,			NULL,					0x10,	0,	NULL},
};
#endif

static const cmds_t eponCmds[] = {
	{"debuglevel",		doEponDebuglevel,	0x02,  	0,  "<debuglevel>"},
	{"dumpallreg",		doEponDumpAllReg,	0x02,  	0,  "<dumpallreg>"},
	{"mpcp",			doEponMpcp,			0x12,  	0,  NULL},
	{"set",			doEponSet,			0x12,  	0,  NULL},
	
#ifdef EPON_MAC_HW_TEST
	{"hwtest",		doEponHwtest,			0x12,  	0,  NULL},
#endif
	{NULL,			NULL,					0x10,	0,	NULL},
};


eponMacHwtestReg_t *eponMacRegTable = NULL;




/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/

//__u32 epon_llid_mask = 1;//LLID bit mask, 1: enable;  default enable LLID-0
#ifdef TCSUPPORT_AUTOBENCH
__u32 eponDebugLevel = EPON_DEBUG_LEVEL_NONE;
#else
__u32 eponDebugLevel = EPON_DEBUG_LEVEL_ERR;
#endif
epon_t eponDrv;


/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/


/************************************************************************
*        F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

int eponMacGetRegTblSize(void){
	int i = 0;
	if(NULL == eponMacRegTable)
		return 0;
	while(eponMacRegTable[i].addr != NULL)
		i++;
	return i;
}


static int doEpon(int argc, char *argv[], void *p)
{
	
	return subcmd(eponCmds, argc, argv, p);
}


static int doEponMpcp(int argc, char *argv[], void *p)
{
	
	return subcmd(eponMpcpCmds, argc, argv, p);
}

static int doEponSet(int argc, char *argv[], void *p)
{
	
	return subcmd(eponSetCmds, argc, argv, p);
}


static int doEponDebuglevel(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\ndebugLevel = %u\n" , eponDebugLevel);
		return 0;
	}
		
	eponDebugLevel = simple_strtoul(argv[1], NULL, 16);
	return 0;
}

static int doEponDumpAllReg(int argc, char *argv[], void *p){
	
	eponMacDumpAllReg();
	return 0;
}

static int doEponDeregister(int argc, char *argv[], void *p){
	__u32 llidIndex;
	if(argc < 2){
		printk("\r\nderegister <LLID Index | 0-7>\n" );
		return 0;
	}
		
	llidIndex = simple_strtoul(argv[1], NULL, 16);


	eponMpcpLocalDergstr((__u8)llidIndex);
	return 0;
}




extern __u8 eponTimeDrftHandleFlag ;

static int doEponSetTmdrftflag(int argc, char *argv[], void *p){


	if(argc < 2){
		printk("\r\ntmdrftflag <0 | 1>\n" );
		return 0;
	}

	if(simple_strtoul(argv[1], NULL, 16) == 1){
		eponTimeDrftHandleFlag = XPON_ENABLE;
	}else{
		eponTimeDrftHandleFlag = XPON_DISABLE;
	}
	

	return 0;
}
#ifdef EPON_MAC_HW_TEST
__u32 eponSetProbe = 0 ;
__u32 eponDefLlidMask = 1;
__u32 ignoreIntMask = 0xffffffff;
static int doEponSetProbe(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\n set epon mac probe after reset\n" );
		printk("\r\n current defalt probe :0x%x\n",eponSetProbe);
		return 0;
	}
	eponSetProbe = simple_strtoul(argv[1], NULL, 16);
	printk("set probe bfb66100 :0x%x",eponSetProbe);

	return 0;
}

static int doEponSetDefLlidMask(int argc, char *argv[], void *p){
	if(argc < 2){
		
		printk("\r\n set default llid mask after reset\n" );
		printk("\r\n current defalt probe :0x%x\n",eponDefLlidMask);
		return 0;
	}
	eponDefLlidMask = simple_strtoul(argv[1], NULL, 16);
	printk("set default llid mask :0x%x",eponDefLlidMask);

	return 0;
}


static int doEponResetMac(int argc, char *argv[], void *p){
	eponMacRestart();
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "epon mac reset");
	return 0;

}
static int doEponResetWan(int argc, char *argv[], void *p){
	
	eponWanResetWithChannelRetire();
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "epon wan reset");
	return 0;

}


static int doEponSetRgstAckType(int argc, char *argv[], void *p){
	__u8 llidIndex=0,ack=1;
	if(argc < 3){
		printk("\r\n rgstack <llid:0~7> <flag:0(nack)/1(ack)> \n" );
		printk(" set status(ack/nack) of Register_ack. \n");
		return 0;
	}
	llidIndex = simple_strtoul(argv[1], NULL, 16);
	ack = simple_strtoul(argv[2], NULL, 16);
	if(ack !=0)
		ack =1;
	eponMpcpSetDiscvRgstAck(llidIndex,ack);
	printk( "\r\n rgstack llid=%x,ack=%x\n",llidIndex,ack);
	return 0;

}


static int doEponSetIgnoreIntMask(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\n ignoreIntMask <mask>,current mask %x \n",ignoreIntMask);
		return 0;
	}
	ignoreIntMask = simple_strtoul(argv[1], NULL, 16);
	printk("\r\n ignoreIntMask =%x\n",ignoreIntMask);
	return 0;

}
#endif

#ifdef TCSUPPORT_CPU_EN7521
static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p)
{
	__u8 llidIndex = 0;
	__u8 num = 0;
	
	if (argc < 3)
	{
		printk("Usage: llidThrshldNum <llidIndex(0~7)> <llidThrshldNum(1~3)>\n");
		return -1;
	}
	
	llidIndex = (__u8)simple_strtoul(argv[1], NULL, 10);
	num = (__u8)simple_strtoul(argv[2], NULL, 10);

	eponSetLlidThrshldNum(llidIndex, num);

	return 0;
}

static int doEponSetLlidQueThrshld(int argc, char *argv[], void *p)
{
	__u8 queueIndex = 0;
	__u8 llidIndex = 0;
	__u8 thrshldIndex = 0;
	__u16 queueThreshold[8] = {0};
	__u8 i = 0;
	int ret = 0;
	eponQueueThreshold_t EponQThr;
	
	if (argc < 11)
	{
		printk("Usage: llidQueThrshld <llidIndex(0~7)> <thrshldIndex(0~2)> <q0 value> ... <q7 value>\n");	
		return -1;
	}

	llidIndex = (__u8)simple_strtoul(argv[1], NULL, 10);
	thrshldIndex = (__u8)simple_strtoul(argv[2], NULL, 10);
	
	for (i =0; i<8; i++)
	{
		queueThreshold[i] = (__u16)simple_strtoul(argv[3+i], NULL, 10);
	}

	for (queueIndex = 0; queueIndex< 8;queueIndex++)
	{
		if (queueThreshold[queueIndex] != 0)
		{
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex;
			EponQThr.thrIdx = thrshldIndex;
			EponQThr.value = queueThreshold[queueIndex];
			ret = epon_set_queue_threshold(&EponQThr) ;
			if(ret < 0 ){
				printk("doEponSetLlidQueThrshld error\n");
				return -1;
			}		
		}
	}
	
	return 0;
}
#endif

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

__u8 eponStaticRptEnable = 0;
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
				gpEponPriv->early_wakeup_timer.data = gpEponPriv->eponCfg.earlyWakeupTimer;
				EPON_START_TIMER(gpEponPriv->early_wakeup_timer) ;
				sleepCount = 0;
			}
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

#ifndef TCSUPPORT_CPU_EN7521
	if(atomic_read(&fe_reset_flag)){
		return;
	}
#endif

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
	
	for(index = 0 ; index < EPON_LLID_MAX_NUM ; index++)
	{
		if(isrValue & (LLID0_RCV_RGST_INT << index) )
		{
			dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr LLID%d_RCV_RGST_INT", index);
			ret = eponMpcpRgstIntHandler(index);
			
			if(ret <0){
				dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: LLID%d_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", index, ret);
			}
		}
	}

#if 0
	if(isrValue&LLID7_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID7_RCV_RGST_INT ");
		isrValue |=  LLID7_RCV_RGST_INT;

	#if 1
		ret = eponMpcpRgstIntHandler(7);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID7_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_7);
	#endif
	}
	
	if(isrValue&LLID6_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID6_RCV_RGST_INT ");
		isrValue |=  LLID6_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(6);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID6_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_6);
	#endif	
		}
	
	if(isrValue&LLID5_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID5_RCV_RGST_INT ");
		isrValue |=  LLID5_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(5);
	
		if(ret <0){
			eponDbgPrint(__u32 debugLevel,char * fmt,...)(EPON_DEBUG_LEVEL_ERR , "\r\nLLID5_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_5);
	#endif	
	}

	if(isrValue&LLID4_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID4_RCV_RGST_INT ");
		isrValue |=  LLID4_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(4);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID4_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_4);
	#endif
	}
	
	if(isrValue&LLID3_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID3_RCV_RGST_INT ");
		isrValue |=  LLID3_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(3);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID3_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
			tasklet_schedule(&eponMacMpcpRgstTask_3);
	#endif
	}
		
	if(isrValue&LLID2_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID2_RCV_RGST_INT ");
		isrValue |=  LLID2_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(2);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID2_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_2);
	#endif	
	}
	
	if(isrValue&LLID1_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID1_RCV_RGST_INT ");
		isrValue |=  LLID1_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(1);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID1_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_1);
	#endif
	}
	
	if(isrValue&LLID0_RCV_RGST_INT)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG, "\r\n eponIsr	LLID0_RCV_RGST_INT ");
		isrValue |=  LLID0_RCV_RGST_INT;
		
	#if 1
		ret = eponMpcpRgstIntHandler(0);
	
		if(ret <0){
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nLLID0_RCV_RGST_INT eponMpcpRgstIntHandler ret=%d", ret);
		}
	#else
		tasklet_schedule(&eponMacMpcpRgstTask_0);
	#endif
	}
#endif

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
static void dbgPrintTimer(void);

spinlock_t dbgPrintLock = SPIN_LOCK_UNLOCKED;
unsigned long dbgPrintFlags;

#define PRINT_MEM_COL 128	
#define PRINT_MEM_ROW 256	

struct SCREEM_PRINT
{
	ktime_t time_info;
	char mem[PRINT_MEM_COL];
}screenprint;

struct SCREEM_PRINT print_mem[PRINT_MEM_ROW]={0};
char test [PRINT_MEM_COL] = {0};

static volatile struct SCREEM_PRINT *pWrite = NULL;
static volatile struct SCREEM_PRINT *pRead = NULL;
static char temp[128] = {0};

struct timer_list dbgPrint;
void dbgPrintQuit(void)
{
	del_timer(&dbgPrint);
}

static void dbgPrintInit(void)
{
	init_timer(&dbgPrint);
	dbgPrint.function = dbgPrintTimer;
	dbgPrint.expires = jiffies + (HZ >> 1);
	add_timer(&dbgPrint);

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
	printk("[%03dmin : %03ds : %03dms: %03dus : %03dns] ", min, sec, msec, usec, nsec);
#endif
}


/*write debug information to memory*/
static void stringWrite()
{
	if (pWrite == (print_mem + PRINT_MEM_ROW)){
		pWrite = print_mem;
	}
	
	if(strcmp(pWrite, &test)){
		printk("Memory out.\n");
	}
	else
	{
		pWrite->time_info = ktime_get();
		memcpy(pWrite->mem, temp, sizeof(temp));
		pWrite++;
	}

}

/*read log from memory*/
static void stringRead()
{
	int i;
	
	for (i = 0; i < 60; i++)
	{
		if(pRead == (print_mem + PRINT_MEM_ROW)){
			pRead = print_mem;
		}
		
		if(strcmp(pRead->mem, test) == 0){
			break;
		}
		
		print_time_information(&pRead->time_info);
		printk("%s\n", pRead->mem);
		mb();
		memset(pRead, 0, sizeof(screenprint));
		pRead++;
	}
}

/*timer to write log*/
static void dbgPrintTimer(void)
{
	stringRead();
	mod_timer(&dbgPrint, jiffies + (HZ >> 1));	
}

/*print debug*/
void  dbgtoMem(__u32 debugLevel, char *fmt,...)
{
	if (eponDebugLevel < debugLevel){
		return;
	}

	spin_lock_irqsave(&dbgPrintLock, dbgPrintFlags);
	
	va_list wp;	
	memset(temp, 0, 128);
	va_start(wp, fmt);
	vsnprintf(temp, 128, fmt, wp);
	stringWrite();
	va_end(wp);
	
	spin_unlock_irqrestore(&dbgPrintLock, dbgPrintFlags);
}
EXPORT_SYMBOL(dbgtoMem);
/*************************PUBLIC PRINT END******************************************/


int eponIrqRegister(void)
{
	int err;
#ifndef TCSUPPORT_CPU_EN7521
	err = qdma_register_callback_function(QDMA_CALLBACK_EPON_MAC_HANDLER , eponIsr);
#else
	err = QDMA_API_REGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_EPON_MAC_HANDLER, eponIsr);
#endif
	if(err){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponIrqInit request_irq err=%d", err);
		return err;
	}
	return 0;
}

void eponIrqUnregister(void)
{
#ifndef TCSUPPORT_CPU_EN7521
	qdma_unregister_callback_function(QDMA_CALLBACK_EPON_MAC_HANDLER);
#else
	QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_EPON_MAC_HANDLER);
#endif
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponIrqDinit QDMA_CALLBACK_EPON_MAC_HANDLER");
}

int getOnuMacAddr(__u8 *macAddr){
	//__u8 *flashMacAddr = (__u8 *)0xbfc0ff48;
	char flashMacAddr[6];
	int i;
	
	for (i=0; i<6; i++) {
		flashMacAddr[i] = READ_FLASH_BYTE(flash_base + 0xff48 + i);
	}

	if( (flashMacAddr[0] == 0) && (flashMacAddr[1] == 0) && (flashMacAddr[2] == 0) &&
	    (flashMacAddr[3] == 0) && (flashMacAddr[4] == 0) && (flashMacAddr[5] == 0) )
		printk(KERN_INFO "The MAC address in flash is null!\n");	    
	else    
  		memcpy(macAddr, flashMacAddr, 6);  	
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

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponLlidEnable");

	/*  init MAC addr */
	//getOnuMacAddr(macAddr);

#ifdef TCSUPPORT_PON_TEST
	if (eponDrvDbgMask & EPON_DBGMASK_TEMPMAC){
		memcpy(macAddr, eponDrvTmpMacAddr, 6);
	}else
#endif
	memcpy( macAddr ,eponOnuMacAddr ,6);

	tmp = get32(macAddr+2);
	tmp += llidIndex;
	memcpy(macAddr+3 ,(__u8 *)&tmp+1 , 3 );
	memcpy(llidEntry_p->macAddr , macAddr , 6);
	tmp = 0;
	
	if(eponMacSetMacAddr(llidEntry_p) < 0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacSetMacAddr fail llidIndex=%d", llidIndex);
		return -1;
	}


	if(eponDebugLevel >= EPON_DEBUG_LEVEL_NOTIFY)
	eponMacGetMacAddr(llidEntry_p);
	/* MPCP status init */
	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_WAIT;
	
	llidEntry_p->eponMpcp.eponDiscFsm.begin = TRUE;
	llidEntry_p->eponMpcp.eponDiscFsm.registered = FALSE;

	eponMpcpDscvFsmWaitHandler(llidIndex);

	//MACR init
	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscMacr_RgstrAck = ACK;	

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

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponLlidDisable");

	/* disable flag */
	
	llidEntry_p->enableFlag = XPON_DISABLE;

	//DISABLE INT
	tmp = READ_REG_WORD(e_int_en);
	tmp &= ~(1<<(llidIndex+1));
	WRITE_REG_WORD(e_int_en , tmp);
	
	//change disc status to unregist
	sdcvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts+ llidIndex);
	sdcvSts.Bits.llidDscvrySts = 0;
	WRITE_REG_WORD(e_llid0_dscvry_sts+ llidIndex, sdcvSts.Raw);

	//retire the channel
#ifndef TCSUPPORT_CPU_EN7521
	qdma_set_channel_retire(llidIndex);
#else
	FE_API_SET_CHANNEL_RETIRE_ONE(FE_GDM_SEL_GDMA2, llidIndex);
#endif
	
	//delete the interface
	xmcs_remove_llid(llidIndex);
	
	return 0;
}


static int eponInitAllLlid(void){
	__u8 llidIndex=0;
	__u32 Raw = 0;
	eponLlid_t *llidEntry_p = NULL;
	extern spinlock_t mpcpDscvGateLock;
	extern spinlock_t mpcpDscvPendingLock;
	int temp = 0;
	int currentState = 0;
	
	//enble INT mask
	Raw |= (RPT_OVERINTVL_INT | MPCP_TIMEOUT_INT | TIMEDRFT_INT | TOD_1PPS_INT | TOD_UPDT_INT | GNT_BUF_OVRRUN_INT | RCV_DSCVRY_GATE_INT);
#ifdef TCSUPPORT_EPON_POWERSAVING
	Raw |= (PS_EARLY_WAKEUP_INT | RX_SLEEP_ALLOW_INT | PS_WAKEUP_INT | PS_SLEEP_INT);
#endif
	if (gpEponPriv->mpcpInterruptMode)
		Raw |= (REG_REQ_DONE_INT | REG_ACK_DONE_INT);
	WRITE_REG_WORD(e_int_en , Raw);


	
	eponDrv.llidMask = 1;//only one LLID when start

#ifdef EPON_MAC_HW_TEST
	eponDrv.llidMask = eponDefLlidMask;
#endif	
	while(llidIndex < EPON_LLID_MAX_NUM){
		llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
		temp = llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout;
		currentState = llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState;
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponInitAllLlid llid %d , Timeout %d, State %d",
			llidIndex, temp, currentState);

		memset(llidEntry_p, 0 , sizeof(eponLlid_t));


		llidEntry_p->llidIndex = llidIndex;
		spin_lock_init(&mpcpDscvPendingLock);
		spin_lock_init(&mpcpDscvGateLock);


		if((eponDrv.llidMask)&(1<<llidIndex)){//this LLID enable
			eponLlidEnable(llidIndex);
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

#ifdef TCSUPPORT_CPU_EN7521
int epon_set_qdma_qos(void)
{
	QDMA_TxRateMeter_T txRateMeter;
	QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
	QDMA_txCngstCfg_t txCngstCfg;
	
	memset(&txRateMeter, 0, sizeof(QDMA_TxRateMeter_T));
	txRateMeter.txRateMeterTimeSlice = 0x3e8;
	txRateMeter.txRateMeterTimeDivisor = 0;
	QDMA_API_SET_TX_RATEMETER(ECNT_QDMA_WAN, &txRateMeter);
	
	memset(&txCngstCfg, 0, sizeof(QDMA_txCngstCfg_t));
	txCngstCfg.txCngstMode = QDMA_TXCNGST_DYNAMIC_PEAKRATE_MARGIN;
	txCngstCfg.peekRateMargin = QDMA_TXCNGST_PEEKRATE_MARGIN_100;
	txCngstCfg.peekRateDuration = 0xff;
	QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(ECNT_QDMA_WAN, &txCngstCfg);

	memset(&txqDynCngstThrh, 0, sizeof(QDMA_TxQDynCngstThrh_T));
	QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
	txqDynCngstThrh.dynCngstTotalMinThrh = 0x300;
	txqDynCngstThrh.dynCngstChnlMinThrh = 0x30;
	txqDynCngstThrh.dynCngstQueueMinThrh = 0x30;
	QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);

	return 0;
}
#endif

int eponTxOamFavorModeCtl(__u8 flag)
{
#ifdef TCSUPPORT_CPU_EN7521
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
	eponDrv.llidMask = 1;

	gpWanPriv->activeChannelNum = 1;
	xpon_reset_qdma_tx_buf();
	
#ifdef TCSUPPORT_CPU_EN7521
	epon_set_qdma_qos();
#endif
	
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

void eponSnifferModeCtrl(__u8 flag)
{
#ifdef TCSUPPORT_CPU_EN7521
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
#endif
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

#ifndef TCSUPPORT_CPU_EN7521
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

	read_data = IO_GREG(e_glb_cfg);
	if((flag == 1) && !(read_data & (MAC_CONTROL_MPCP_FWD))){
		write_data = read_data | (MAC_CONTROL_MPCP_FWD);
		IO_SREG(e_glb_cfg, write_data);
		//printk("setMpcpFwd : enable MPCP_FWD\n");
	}else if((flag == 0) && (read_data & (MAC_CONTROL_MPCP_FWD))){
		write_data = read_data & (~MAC_CONTROL_MPCP_FWD);
		IO_SREG(e_glb_cfg, write_data);
		//printk("setMpcpFwd : disable MPCP_FWD\n");
	}
}
#endif
#ifdef CONFIG_USE_MT7520_ASIC 
#define EPON_RST_LOOP_CNT	(800000)
#else
#define EPON_RST_LOOP_CNT	(80000)
#endif
int eponMacSwReset(void){
	__u32 Raw;
	__u32 valueRst;
	volatile __u32 cnt = 0;
	
	valueRst = READ_REG_WORD(REG_E_SW_RST);
	valueRst |= (1<<31);
	WRITE_REG_WORD(REG_E_SW_RST , valueRst);
	while(cnt <EPON_RST_LOOP_CNT){
		cnt++;
	}

	valueRst &= 0x7fffffff;
	WRITE_REG_WORD(REG_E_SW_RST , valueRst);

	cnt = 0;
	Raw = READ_REG_WORD(e_glb_cfg);
	Raw |= 0x10;
	WRITE_REG_WORD(e_glb_cfg , Raw);

	while(cnt <EPON_RST_LOOP_CNT){
		cnt++;
	}
	Raw &= 0xffffffef;
	
	WRITE_REG_WORD(e_glb_cfg , Raw);
	cnt = 0;
	while(cnt <EPON_RST_LOOP_CNT){ 
			cnt++;
	}

	
#ifdef EPON_MAC_HW_TEST
	if(eponSetProbe != 0)
		WRITE_REG_WORD( e_dbg_prb_sel,eponSetProbe ) ;

		dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"e_int_status=%x,e_llid0_dscvry_sts=%x",
		READ_REG_WORD(e_int_status),
		READ_REG_WORD(e_llid0_dscvry_sts));
#endif

	/*  send controll packet first  */
	Raw |= 0x2;
	WRITE_REG_WORD(e_glb_cfg , Raw);

	/* set e_grd_thrshld to 0x10*/
	WRITE_REG_WORD(e_grd_thrshld , EPON_TIMEDRIFT_THRSHLD);



	//for epon FPGA  2012/12/6
	
	WRITE_REG_WORD(e_trx_adjust_time1, 0x004ffff1);
	WRITE_REG_WORD(e_trx_adjust_time2, 6);
#ifdef CONFIG_USE_MT7520_ASIC 
	WRITE_REG_WORD(e_txfetch_cfg, 0x202403e8);	
#else
	WRITE_REG_WORD(e_txfetch_cfg, 0x242a03e8);	//0x1a2603e8
#endif

#ifdef CONFIG_USE_MT7520_ASIC 
	e_glb_cfg_SET_loc_cnt_sync_method(e_glb_cfg, 1);
	e_glb_cfg_SET_rpt_txpri_ctrl(e_glb_cfg, 1);
	if (eponStaticRptEnable == 1){
		printk("Enable epon static report\n");
		e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 1); // clear rpt buffer
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 0); // return normal
		e_rpt_len_SET_df_rpt_dt_len(e_rpt_len, 1);// set def rpt data length
		e_rpt_data_SET_df_rpt_data(e_rpt_data, 0x0101a000); // static report value
		e_rpt_data_SET_df_rpt_data(e_rpt_data, 0);					
		e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 1);
		e_tx_cal_cnst_SET_default_ovrhd(e_tx_cal_cnst, 0x8);
	}
#else
	Raw = READ_REG_WORD(e_glb_cfg);
	Raw |= 0x42;
	WRITE_REG_WORD(e_glb_cfg , Raw);

	WRITE_REG_WORD(e_rpt_len, 0x100);
	//delay
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);
	READ_REG_WORD(e_rpt_len);

	WRITE_REG_WORD(e_rpt_len, 1);
	WRITE_REG_WORD(e_rpt_data, 0x0101a000);
	WRITE_REG_WORD(e_rpt_data, 0);
#endif
#ifdef TCSUPPORT_CPU_EN7521
	/* When send REG_req, MAC assert burst_en(burst mode) to PHY */
	Raw = READ_REG_WORD(e_glb_cfg);
	Raw |= (1<<23);
	WRITE_REG_WORD(e_glb_cfg, Raw);
	printk("Raw = %x\n", Raw);
#endif
#ifndef TCSUPPORT_CPU_EN7521
	/* 7521 synctime always have, don't need this */
	setMpcpFwd(1);
#endif

#if defined(TCSUPPORT_XPON_LED)
	ledTurnOff(LED_XPON_STATUS);		
	ledTurnOff(LED_XPON_TRYING_STATUS);
	led_xpon_status = 0;
#endif

	return 0;
}



int eponMacStop(void){
	__u8 llidIndex = 0;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMacStop");

	/* EPON MAC SW reset */
	eponMacSwReset();

	for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM;llidIndex++){
		xmcs_remove_llid(llidIndex);
	}

	return 0;
}


void checkEponMacRestart(void){
	int cnt = 0;

	while(cnt <= 1000){
		if(atomic_read(&eponMacRestart_flag)) {
			mdelay(1);
		}else{
			break;
		}
		cnt++;
	}
}

int eponMacRestart(void){
	__u8 llidIndex = 0;
	__u8 snifferModeEnable = 0;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMacRestart");

	atomic_set(&eponMacRestart_flag, 1);
	
	/* EPON MAC SW reset */
	eponMacSwReset();

	for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM;llidIndex++){
		xmcs_remove_llid(llidIndex);
	}

	
	/*  Init all EPON ONU param */
	//eponInitParam();
	
	
	
	eponInitAllLlid();

	if (gpPonSysData->dyingGaspData.isEponHwFlag)
		eponHwDygaspCtrl(XPON_ENABLE);
	else
		eponHwDygaspCtrl(XPON_DISABLE);

	if (gpEponPriv->eponCfg.txOamFavorMode)
		eponTxOamFavorModeCtl(XPON_ENABLE);
	else
		eponTxOamFavorModeCtl(XPON_DISABLE);

	snifferModeEnable = gpEponPriv->eponCfg.snifferModeConfig.snifferModeEnable;
	eponSnifferModeCtrl(snifferModeEnable);
	
	atomic_set(&eponMacRestart_flag, 0);

	return 0;
}



#if 0
void eponMpcpRgstIntHandler_0(__u32 data){
	eponDbgPrint(EPON_DEBUG_LEVEL_ERR,"\r\n>>>>>>>> enter eponMpcpRgstIntHandler_0");
	eponMpcpRgstIntHandler(0);
}

void eponMpcpRgstIntHandler_1(__u32 data){
	eponDbgPrint(EPON_DEBUG_LEVEL_ERR,"\r\n>>>>>>>> enter eponMpcpRgstIntHandler_1");
	eponMpcpRgstIntHandler(1);
}

void eponMpcpRgstIntHandler_2(__u32 data){
	eponMpcpRgstIntHandler(2);
}

void eponMpcpRgstIntHandler_3(__u32 data){
	eponMpcpRgstIntHandler(3);
}
void eponMpcpRgstIntHandler_4(__u32 data){
	eponMpcpRgstIntHandler(4);
}
void eponMpcpRgstIntHandler_5(__u32 data){
	eponMpcpRgstIntHandler(5);
}
void eponMpcpRgstIntHandler_6(__u32 data){
	eponMpcpRgstIntHandler(6);
}
void eponMpcpRgstIntHandler_7(__u32 data){
	eponMpcpRgstIntHandler(7);
}

#endif


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


#if 0 

int eponWanReset(void){
	__u8 chanIndex =0;


	eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG,"\r\neponWanReset");
	//eponMacTxRxDisable
	eponMacTxRxDisable();

	//disable GDM2 enable
	//TODO: should place with FE API
	WRITE_REG_WORD(0xbfb5152c,0);




	//disable QDMA TX/RX DMA
//	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_ENABLE);

	
	//retire qdma
	while(chanIndex < 8){
		qdma_set_channel_retire(chanIndex);
		chanIndex++;
	}

	//enable QDMA TX/RX DMA
	//qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE);
	eponMacRestart();
	eponMacTxRxDisable();

	//TODO: should place with FE API
	WRITE_REG_WORD(0xbfb5152c,0xffffffff);
	
	eponMacTxRxEnable();	



	
	return 0;
}
#endif


int eponWanStopWithChannelRetire(void){
	__u8 chanIndex =0;
	__u8 i = 0;
	
	unsigned long flags;
	spin_lock_irqsave(&epon_reg_lock,flags);

	//eponMacTxRxDisable
	eponMacTxRxDisable();
	/* wait for MBI interface finish */
	mdelay(1);

	/* disable cpu traffic */
	disable_cpu_us_traffic();

#ifndef TCSUPPORT_CPU_EN7521
	WRITE_REG_WORD(0xbfb5152c,0);
	//retire qdma
	while(chanIndex < 8){
		qdma_set_channel_retire(chanIndex);
		chanIndex++;
	}
#else
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
	mdelay(1);
	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 8);
#endif

	//disable QDMA TX/RX DMA
//	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_ENABLE);

	eponMacStop();
	eponMacTxRxDisable();

#ifndef TCSUPPORT_CPU_EN7521
	WRITE_REG_WORD(0xbfb5152c,0xffffffff);
#endif
	
	spin_unlock_irqrestore(&epon_reg_lock,flags); 
	
	return 0;
}

#ifdef TCSUPPORT_CPU_EN7521
uint g_EponLaserTime = 0;
#else
uint g_EponLaserTime = 4;
#endif
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
	__u8 chanIndex =0;
	__u32 Raw =0 ;
	__u32 valueRst;
	volatile __u32 cnt = 0;
	__u8 i = 0;
	unsigned long flags;
	spin_lock_irqsave(&epon_reg_lock,flags);
	
	//eponMacTxRxDisable
	eponMacTxRxDisable();

	cnt = 0;
	while(cnt <EPON_RST_LOOP_CNT){
		cnt++;
	}
	
	cnt = 0;
	Raw = READ_REG_WORD(e_glb_cfg);
	Raw |= 0x10;
	WRITE_REG_WORD(e_glb_cfg , Raw);

	while(cnt <EPON_RST_LOOP_CNT){ 
		cnt++;
	}
	
	Raw &= 0xffffffef;
	
	WRITE_REG_WORD(e_glb_cfg , Raw);
	cnt = 0;
	while(cnt <EPON_RST_LOOP_CNT){ 
			cnt++;
	}

	valueRst = READ_REG_WORD(REG_E_SW_RST);
	valueRst |= (1<<31);
	WRITE_REG_WORD(REG_E_SW_RST , valueRst);
	while(cnt <EPON_RST_LOOP_CNT){
		cnt++;
	}
	valueRst &= 0x7fffffff;
	WRITE_REG_WORD(REG_E_SW_RST , valueRst);


	//disable GDM2 enable
	//TODO: should place with FE API
#ifndef TCSUPPORT_CPU_EN7521
	WRITE_REG_WORD(0xbfb5152c,0);
	//retire qdma
	while(chanIndex < 8){
		qdma_set_channel_retire(chanIndex);
		chanIndex++;
	}
#else
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
	mdelay(1);
	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 8);
#endif

	//disable QDMA TX/RX DMA
//	qdma_dma_mode(QDMA_DISABLE, QDMA_DISABLE, QDMA_ENABLE);

	//enable QDMA TX/RX DMA
	//qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE);
	eponMacRestart();
	eponMacTxRxDisable();

#ifndef TCSUPPORT_CPU_EN7521
	WRITE_REG_WORD(0xbfb5152c,0xffffffff);
#else
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
#endif
	eponMacTxRxEnable();	
	eponSetlaserTime();
	spin_unlock_irqrestore(&epon_reg_lock,flags); 

	return 0;
}



#if 0
int eponWanResetWithChannelRe(void){
	//eponMacTxRxDisable
	eponMacTxRxDisable();

	//disable GDM2 enable
	//TODO: should place with FE API
	WRITE_REG_WORD(0xbfb5152c,0);

	eponMacRestart();
	eponMacTxRxDisable();

	//TODO: should place with FE API
	WRITE_REG_WORD(0xbfb5152c,0xffffffff);
	
	eponMacTxRxEnable();	
	return 0;
}

#endif

void eponPhyInit(void)
{
	//PHY init
	IO_SPHYREG(0x124, 0x7f000000) ;
	IO_SPHYREG(0x108, 0xCD810110) ;
	IO_SPHYREG(0x108, 0x45810110) ;
	IO_SPHYREG(0x1D0, 0x00086100) ;
	IO_SPHYREG(0x108, 0x4581E130) ;
	IO_SPHYREG(0x1F0, 0x400000C8) ;
	IO_SPHYREG(0x1D4, 0x00000013) ;
	IO_SPHYREG(0x204, 0x00000045) ;//fix crc error
}

__u8 eponOnuMacAddr[6] = {0};

#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
extern uint8 noRxDscpInt;
extern uint8 attackEnhance;
extern unsigned long int dropThred;
extern uint8 rxDropCheckTimer;
#endif
static struct timer_list rgstIntvalTimer;
void rgstIntvalTimerHandler(unsigned long date)
{
	__u8 llidIndex = 0;
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
	static uint8 timer_count=0;
 	static unsigned long int oldDropCounter = 0;
	static unsigned long int newDropCounter = 0;
#endif

	if(TRUE == gpPonSysData->ponMacPhyReset
#ifndef TCSUPPORT_CPU_EN7521
       || atomic_read(&fe_reset_flag) 
#endif
        ) {
		goto restart_timer;
	}

	int rgs_flag = 0;

	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[0]);
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE) 
	if(timer_count == rxDropCheckTimer)
	{
#ifdef TCSUPPORT_CPU_EN7521
		FE_API_GET_RX_DROP_FIFO_CNT(FE_GDM_SEL_GDMA2, &newDropCounter);
#else		
		newDropCounter = VPint(GDMA2_RX_OVDROPCNT);
#endif		
		if(newDropCounter - oldDropCounter > dropThred)
		{
			printk("GDMA2_RX_OVDROPCNT:   newDropCounter = 0x%08lx\n",newDropCounter);
			printk("GDMA2_RX_OVDROPCNT:   oldDropCounter = 0x%08lx\n",oldDropCounter);
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
	for(llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM; llidIndex++, llidEntry_p++){
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
		
	#if 0

		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "rgstIntvalTimerHandler llidIndex=%d state=%d", llidIndex , llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState);

		if(llidEntry_p->enableFlag== XPON_DISABLE){
			continue;
		}	
		if(llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState ==MPCP_STATE_DENIED ){
			llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout--;
			if(llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout == 0)
			{
				dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "rgstIntvalTimerHandler llidIndex=%d state=DENIED-->WAIT", llidIndex);
				llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState =MPCP_STATE_WAIT; 
				eponMpcpDscvFsmWaitHandler(llidIndex);
			}
		}
		#endif
	}


restart_timer:
	mod_timer(&rgstIntvalTimer, jiffies + (HZ ));

}


void eponEnable(void)
{
	int ret;

	//IO_SREG(0xbfb00070,1);

	/* EPON MAC SW reset */
	eponWanResetWithChannelRetire();
	
	/* IRQ init */
	eponIrqRegister();
}
void eponDisable(void)
{
#ifdef TCSUPPORT_XPON_LED
	ledTurnOff(LED_XPON_STATUS);		
	ledTurnOff(LED_XPON_TRYING_STATUS);
	led_xpon_status = 0;
#endif
	/* IRQ deinit */
	eponIrqUnregister();
	/* EPON MAC stop */
	eponWanStopWithChannelRetire();

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
		if (0 == mode)
		{
			/* set burst mode */
			IO_CBITS(PHY_CSR_PHYSET10,(1<<26));	
		}
		else if (1 == mode)
		{
			/* set continue mode */
			IO_SBITS(PHY_CSR_PHYSET10,(1<<26));	
		}
	}	
	return;
}

void eponSetIFG(unchar ifg_val)
{
	__u32 Raw = 0;
	Raw = READ_REG_WORD(e_tx_cal_cnst);
	Raw &= ~(0x1f);
	Raw |= (ifg_val & 0x1f);
	eponDbgPrint(EPON_DEBUG_LEVEL_NOTIFY, "eponSet IFG = %d,reg val", ifg_val,Raw);	
	WRITE_REG_WORD(e_tx_cal_cnst , Raw);	
}

void eponStart(unsigned long data)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponStart");

	IO_SREG(0xbfb00070,1);
	/* set burst mode */
	eponFirmwareSetMode(0);	
	//tasklet_enable(&eponMacMpcpDscvGateTask);

	/*  Init all EPON ONU param */
	eponInitParam();
			
	eponInitAllLlid();

	eponEnable();

    eponSetIFG(EPON_IFG_DEFAULT);

	return ;
}

int eponCleanAllLlidQueThrod(void ){
	__u8 queueIndex = 0;
	__u8 llidIndex = 0;
	__u8 setIndex = 0;
#ifndef TCSUPPORT_CPU_EN7521
	QDMA_EponQueueThreshold_T EponQThr;
#else
	eponQueueThreshold_t EponQThr;
#endif
	int ret = 0;
//	printk("eponCleanAllLlidQueThrod enter\n");
	for(llidIndex = 0 ;llidIndex < EPON_LLID_MAX_NUM ;llidIndex++ ){
		for(setIndex = 0; setIndex< 3;setIndex++){
			for(queueIndex = 0; queueIndex< 8;queueIndex++){//set queue threshold
				EponQThr.channel = llidIndex;
				EponQThr.queue = queueIndex;
				EponQThr.thrIdx = setIndex;
				EponQThr.value = 0;
#ifndef TCSUPPORT_CPU_EN7521
				ret = qdma_set_epon_queue_threshold(&EponQThr) ;
#else
				ret = epon_set_queue_threshold(&EponQThr) ;
#endif
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
#ifndef TCSUPPORT_CPU_EN7521
	QDMA_EponQueueThreshold_T EponQThr;
#else
	eponQueueThreshold_t EponQThr;
#endif
	int ret = 0;
	__u8 llidIndex = 0;
	__u8 setIndex = 0;
	
	printk("eponDumpAllLlidQueThrod enter\n");
	for(llidIndex = 0 ;llidIndex < EPON_LLID_MAX_NUM ;llidIndex++ ){
		for(setIndex = 0; setIndex< 3;setIndex++){
			printk("\n ch%d,set%d : ",llidIndex,setIndex);
			for(queueIndex = 0; queueIndex< 8;queueIndex++){//set queue threshold
				memset(&EponQThr, 0, sizeof(QDMA_EponQueueThreshold_T));
				EponQThr.channel = llidIndex;
				EponQThr.queue = queueIndex ;
				EponQThr.thrIdx = setIndex;
#ifndef TCSUPPORT_CPU_EN7521
				ret = qdma_get_epon_queue_threshold(&EponQThr); 
#else
				ret = epon_get_queue_threshold(&EponQThr);
#endif
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

#ifdef TCSUPPORT_CPU_EN7521
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
		mdelay(1) ;
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
		mdelay(1) ;
	}

	printk("Timeout for get EPON Threshold configuration.\n") ;
	
	return -1 ;
}
#endif

void eponStop(void)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponStop");
	eponCleanAllLlidQueThrod();
	//tasklet_disable(&eponMacMpcpDscvGateTask);
	eponDisable();
	XPON_PHY_COUNTER_CLEAR(0x1f);
	/* Temporarily mark */
#ifndef TCSUPPORT_CPU_EN7521
    XPON_PHY_SET(PON_SET_EPONMODE_PHY_RESET);
#endif
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



static int eponCmdInit(void)
{
	cmds_t eponCmd;
	int ret = -1;
	memset(&eponCmd , 0 , sizeof(cmds_t));

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponCmdInit");

	
	/* init  root ci-cmd */ 
	eponCmd.name = "epon";
	eponCmd.func = doEpon;
	eponCmd.flags = 0x12;
	eponCmd.argcmin = 0;
	eponCmd.argc_errmsg = NULL;

	/* register  ci-cmd */
	ret = cmd_register(&eponCmd); 
	if(ret <0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponCmdInit ret=%d \n", ret);
	}
	return ret;
}

static void eponCmdExit(void)
{
	cmd_unregister("epon");
}
/*****************************************************************************
******************************************************************************/
#ifdef TCSUPPORT_EPON_DUMMY
__u8 gDummyEnable = 0;
atomic_t gDummyTempHalt = ATOMIC_INIT(1);
static struct timer_list gSendDummyTimer;
static __u8 auto_disable = 0;
#endif

#define TYPEB_SUCCESS   1
#define TYPEB_FAIL  0

void eponDetectPhyLosHandler(void)
{
#ifdef TCSUPPORT_EPON_DUMMY
    if(auto_disable && gDummyEnable){
        atomic_set(&gDummyTempHalt, 1);
    }
#endif
    
    gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_LOS, 0) ;


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
	if(IO_GREG(0xbfb00070) == 1)
#endif
	{
		eponStop();
	}
	gpWanPriv->activeChannelNum = 1;	
	xpon_reset_qdma_tx_buf();
	
	gpPonSysData->sysPrevLink = PON_LINK_STATUS_EPON ;

	eponFirmwareSetMode(0);
	
    stop_omci_oam_monitor();
    
	schedule_fe_reset();
}

static void epon_los_timer_func(void)
{
	eponDrv.typeBOnGoing = FALSE;
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
		gEponTypeBTimer.data = eponDrv.hldOverTime;	
		EPON_STOP_TIMER(gEponTypeBTimer);
		EPON_START_TIMER(gEponTypeBTimer);
		eponDrv.typeBOnGoing = TRUE;
	}
	return ret;
}

void eponDetectPhyLosLof(void)
{
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
		xpon_set_qdma_qos(gpWanPriv->devCfg.flags.isQosUp);		
	}
}

extern int isSlientBegin;
#ifdef TCSUPPORT_EPON_DUMMY
static __u8 auto_recover = 0;
#endif
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
    change_alarm_led_status(ALARM_LED_OFF);

	dbgtoMem(EPON_DEBUG_LEVEL_ISR, "EponIsr Epon Phy Ready");

	if(FALSE == eponDrv.typeBOnGoing){
		/* set burst mode */
		eponFirmwareSetMode(0);
	}
	
	if(gpPonSysData->sysPrevLink == PON_LINK_STATUS_GPON) {
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_MODE_CHANGE, 0);
	}
	
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_PHY_READY, 0);
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;
		if(FALSE == eponDrv.typeBOnGoing){
			tasklet_schedule(&eponStartTask);
			gpWanPriv->devCfg.flags.isTxDropOmcc = 0;
			xpon_set_qdma_qos(gpWanPriv->devCfg.flags.isQosUp);		
#ifdef TCSUPPORT_EPON_DUMMY       
	        if( auto_recover && atomic_read(&gDummyTempHalt) && gDummyEnable ){
	            atomic_set(&gDummyTempHalt, 0);
	        }
#endif
		}
	}
}
/*****************************************************************************
******************************************************************************/

#ifdef TCSUPPORT_EPON_DUMMY
static size_t gDummyPktSize = 1500; /*below 1500, qdma ok*/
#define MAX_CMDLINE_ARGC 4

static size_t gPktNum = 800;
static __u32 gDummyTimerInterval = 10;

struct tasklet_struct   epon_dummy_tasklet;

static inline void print_dummy_help()
{
    printk("Command e.g.:\n"
           "    help   #show this message\n"
           "    start  #start sending upstream dummy pkts\n"
           "    stop   #stop sending upstream dummy pkts\n"
           "    timer  <interval> #config timer timeout interval\n"
           "    pkt number <num>  #config how many pkts sent when timeout\n"
           "    pkt size <sz>     #config the size of every dummy pkt\n"
           "    dscp max <num>    #config max number of SW DSCP for dumy\n"
           "    show config       #show current config\n"
           "    debug <enable/disable> #enable/disable debug message\n\n"
           );
}

size_t gDummyMaxDscp = 400;
__u8 gDummyDbgMsg = 0;
__u8 traffic_status_monitor = 0;

static int epon_dummy_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char string[64] = {0}, *argv[MAX_CMDLINE_ARGC];
    size_t argc = 0;
	
	if (count > sizeof(string) - 1)
		return -EINVAL ;

	if (copy_from_user(string, buffer, count))
		return -EFAULT ;

    string[count] = 0;
    
    xpon_prepare_params(string, &argc, argv);

    if(0 == argc)
    {
        printk("Command needed!\n");
        print_dummy_help();
        goto print_config;
    }
  
    if( !strcmp("help", argv[0]) ){
        print_dummy_help();
        return count;
    }

    if( !strcmp("start", argv[0]) ){
        printk("Start sending dummy pkts!\n");
        gDummyEnable = 1;	
        EPON_START_TIMER(gSendDummyTimer);
        if (2 == argc && !strcmp("-f", argv[1]) ){
            atomic_set(&gDummyTempHalt, 0);
        }
    }

    else if( !strcmp("stop", argv[0]) ){
        printk("Stop sending dummy pkts!\n");
        gDummyEnable = 0;
        /*atomic_set(&gDummyTempHalt, 0);*/
        EPON_STOP_TIMER(gSendDummyTimer);
    }

    else if( !strcmp("pkt", argv[0]) ){
        if(3 != argc){
            printk("Error! \"pkt\" command takes 2 params!\n");
            goto print_config;
        }
        
        if( !strcmp("number", argv[1]) ){
            sscanf(argv[2], "%u", &gPktNum);
            goto print_config;
        }else if( !strcmp("size", argv[1]) ){
            sscanf(argv[2], "%u", &gDummyPktSize);
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
    }

    else if( !strcmp("timer", argv[0]) ){
        if (2 != argc){
            printk("Error! \"timer\" command takes 1 integer param!\n");
            print_dummy_help();
            goto print_config;
        }
        
        sscanf(argv[1], "%u", &gDummyTimerInterval);
        
        gSendDummyTimer.data = gDummyTimerInterval;
        if(gDummyEnable){
            EPON_START_TIMER(gSendDummyTimer);
        }
        
        goto print_config;
    }

    else if( !strcmp("dscp", argv[0]) ){
        if (3 != argc){
            printk("Error! \"desp\" command takes 2 params!\n");
            print_dummy_help();
            goto print_config;
        }
        if( !strcmp("max", argv[1]) ){
            sscanf(argv[2], "%u", &gDummyMaxDscp);
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
        goto print_config;
    }

    else if( !strcmp("show", argv[0]) ){
        if( !strcmp("config", argv[1]) ){
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
    }

    else if( !strcmp("auto_recover", argv[0]) ){
        if (2!= argc){
            printk("Error! \"auto_recover\" command takes 1 string param!\n");
            print_dummy_help();
            goto print_config;
        }
        
        if( !strcmp("on", argv[1]) ){
            auto_recover = 1;
            goto print_config;
        }else if( !strcmp("off", argv[1]) ){
            auto_recover = 0;
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
        
    }


    else if( !strcmp("auto_disable", argv[0]) ){
        if (2!= argc){
            printk("Error! \"auto_disable\" command takes 1 string param!\n");
            print_dummy_help();
            goto print_config;
        }
        
        if( !strcmp("on", argv[1]) ){
            auto_disable = 1;
            goto print_config;
        }else if( !strcmp("off", argv[1]) ){
            auto_disable = 0;
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
        
    }

    else if( !strcmp("traffic_status", argv[0]) ){
        if (2!= argc){
            printk("Error! \"auto_disable\" command takes 1 string param!\n");
            print_dummy_help();
            goto print_config;
        }
        
        if( !strcmp("down", argv[1]) ){
            if(traffic_status_monitor){
                atomic_set(&gDummyTempHalt, 1);
                goto print_config;
            }else{
                goto ret_label;
            }
        }else if( !strcmp("up", argv[1]) ){
            if(traffic_status_monitor){
                atomic_set(&gDummyTempHalt, 0);
                goto print_config;
            }else{
                goto ret_label;
            }
        }else if( !strcmp("concern", argv[1]) ){
            traffic_status_monitor = 1;
            goto print_config;
        }else if( !strcmp("not_concern", argv[1]) ){
            traffic_status_monitor = 0;
            atomic_set(&gDummyTempHalt, 0);
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
    }

    else if( !strcmp("debug", argv[0]) ){
        if (2 != argc){
            printk("Error! \"debug\" command takes 1 string param!\n");
            print_dummy_help();
            goto print_config;
        }
        if( !strcmp("enable", argv[1]) ){
            gDummyDbgMsg = 1;
            goto print_config;
        }else if( !strcmp("disable", argv[1]) ){
            gDummyDbgMsg = 0;
            goto print_config;
        }else{
            printk("Invalid sub command: %s\n", argv[1]);
        }
    }

    else {
        printk("Unknown command: \"%s\"\n", argv[0]);
        print_dummy_help();
        goto print_config;
    }

print_config:
    printk("Current Confing:\n"
           "    Send status: %s, TempHalt = %d, auto_disable, = %d, auto_recover = %d\n"
           "    traffic status: %s\n"
           "    Send %u packet(s) every %ums, pkt len: %u\n"
           "    Max number of soft desp can be used by dummy: %u\n"
           "    Debug Msg: %s\n",
                gDummyEnable ? "Enable" : "Disable",
                gDummyTempHalt, auto_disable, auto_recover,
                traffic_status_monitor ? "Concern" : "Not concern",
                gPktNum, gDummyTimerInterval, gDummyPktSize,
                gDummyMaxDscp,
                gDummyDbgMsg ? "enable" : "disable");

ret_label:
    return count;
}


static int epon_dummy_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	 int len = 0;

	 len = sprintf(buf,"%d\n", ((isEPONFWID) ? 1 : 0) );

	 if (len < off + count)
		 *eof = 1;
 
	 len -= off;
	 *start = buf + off;
	 if(len > count)
		 len = count;
	  if(len < 0)
		 len = 0;

	return len;
}

/*atomic_t gDummyDscpCount = ATOMIC_INIT(0);*/

extern atomic_t gDummyDscpCount;
extern uint gDummyDrop_SW_DSCP_MAX_USED;
extern uint gDummyDrop_TEMP_HALT;
extern uint gDummyDrop_MPCP_UNREGISTERED;

struct sk_buff * gDummySkb = NULL;

static void sendDummyPkt(void)
{
    struct sk_buff * skb =  gDummySkb;
    struct ethhdr * ethhdr = NULL;
    size_t pktNum = 0;
    struct net_device * dev = NULL;

    dev = dev_get_by_name(&init_net, "pon");
	/*dev = gpWanPriv->pPonNetDev[PWAN_IF_DATA];*/

    if (!dev) goto end;

    for(pktNum = 0; pktNum < gPktNum; pktNum++){

        if(atomic_read(&gDummyTempHalt) /*|| gpWanPriv->epon.llid[0].info.txDrop*/){
            gDummyDrop_TEMP_HALT += gPktNum - pktNum;
            break;
        }

        
        if(MPCP_STATE_REGISTERED != eponDrv.eponLlidEntry[0].eponMpcp.eponDiscFsm.mpcpDiscState){
            gDummyDrop_MPCP_UNREGISTERED += gPktNum - pktNum;
            break;
        }
        
        if(atomic_read(&gDummyDscpCount) >= gDummyMaxDscp){
            ++ gDummyDrop_SW_DSCP_MAX_USED;
		continue;
		/*start_epon_dummy();
		break;*/
        }

        if(unlikely(!skb)){
            skb = gDummySkb = skbmgr_alloc_skb2k();

            if(unlikely(!skb))
                goto end;
            
            ethhdr = (struct ethhdr * )(skb->data);
            memcpy(ethhdr->h_source, eponOnuMacAddr, sizeof(eponOnuMacAddr));
            memset(ethhdr->h_dest, 0, sizeof(eponOnuMacAddr));
            ethhdr->h_proto = DUMMY_ETHTYPE; // lowest queue
        }
        skb->len = gDummyPktSize;
        pwan_net_start_xmit(skb, dev);
    }

    dev_put(dev);
end:
    return;
}

static void epon_dummy_tasklet_handler(unsigned long data)
{
	if(gDummyEnable){
		sendDummyPkt();
	}
}

static void send_dummy_timer_func(unsigned long data)
{
	if(gDummyEnable)
		tasklet_schedule(&epon_dummy_tasklet);
	EPON_START_TIMER(gSendDummyTimer);
}

void start_epon_dummy(){
	if(gDummyEnable)
		tasklet_schedule(&epon_dummy_tasklet);	
	return;
}

#endif

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


static int get_oam_trx_count(void)
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
        return 1;
    }

    g_txOamStats.hwNxt = (IO_GREG(e_oam_stat))&(0xffff);
    
    if(g_txOamStats.hwNxt != g_txOamStats.hwPre)
    {
        g_txOamStats.hwFlg++;
    }
    
    g_txOamStats.hwPre = g_txOamStats.hwNxt;
    
    if( 0!= get_oam_swtx_count(&g_txOamStats.swNxt ) )
    {
        return 0;
    }
    
    if(g_txOamStats.swNxt != g_txOamStats.swPre)
    {
        g_txOamStats.swFlg++;
    }
    
    g_txOamStats.swPre = g_txOamStats.swNxt;
    g_txOamStats.tryCnt++;
    XPON_START_TIMER(gpEponPriv->oamCntTimer);
    return 0;
}

static void sendQueuePkt(unsigned data)
{
    struct sk_buff * skb = NULL;
    struct ethhdr * ethhdr = NULL;
    PWAN_FETxMsg_T txBmMsg = {0};
    int queue = 0;
    int ret = 0;

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

#ifdef TCSUPPORT_CPU_EN7521
        ret = QDMA_API_TRANSMIT_PACKETS(ECNT_QDMA_WAN, skb, txBmMsg.word[0], txBmMsg.word[1], NULL);
        if(ret != 0){
            dev_kfree_skb_any(skb);
            printk("[%s %d]ERR: QDMA_API_TRANSMIT_PACKETS\n", __FUNCTION__, __LINE__);
        }
#else
        ret = qdma_transmit_packet(&txBmMsg, sizeof(txBmMsg), skb);
        if(ret != 0){
            dev_kfree_skb_any(skb);
            printk("[%s %d]ERR: qdma_transmit_packet\n", __FUNCTION__, __LINE__);
        }
#endif

    }
}

int bitmap_format_convert(void)
{
    if(gpEponPriv->BitmapFlag){
        tasklet_schedule(&gpEponPriv->epon_bitmap_format_tasklet);
    }

    return 0;
}

void eponMacTableInit(void);
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
static int epon_init_private_data(EPON_GlbPriv_T *epon_priv_p)
{
	int ret = 0 ;
#ifdef TCSUPPORT_EPON_POWERSAVING
	epon_priv_p->eponCfg.earlyWakeupTimer = 50;
	epon_priv_p->eponCfg.earlyWakeupFlag = 0;
	epon_priv_p->eponCfg.earlyWakeupCount = 0;
#endif

	epon_priv_p->mpcpInterruptMode = 0;
	epon_priv_p->eponCfg.snifferModeConfig.snifferModeEnable = 0;
	epon_priv_p->eponCfg.snifferModeConfig.snifferModeLanMask = 0;
	epon_priv_p->eponCfg.txOamFavorMode= 0;
	if (isEN7526c) {
		epon_priv_p->mpcpInterruptMode = 1;
	}
    epon_priv_p->gBitmapSet = 0;
    epon_priv_p->BitmapFlag = 0;
    epon_priv_p->maxTryCnt = 3;
    epon_priv_p->oamChannel = 0;
	return ret;
}

int eponInit(void)
{
	int ret = -1;
	XMCSIF_OnuType_t type;
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponInit");	

	spin_lock_init(&epon_reg_lock);

	/* Base Register of  EPON */
	g_EPON_MAC_BASE = (PEPON_MAC_REGS)(ioremap_nocache(CONFIG_EPON_BASE_ADDR, CONFIG_EPON_REG_RANGE)) ; 
	if(!g_EPON_MAC_BASE) {
		dbgtoMem(EPON_DEBUG_LEVEL_ERR ,"ERR: ioremap the EPON base address failed:addr=0x%x,size=0x%x",CONFIG_EPON_BASE_ADDR,CONFIG_EPON_REG_RANGE) ;
		return -ENOMEM ;
	}
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

#ifdef TCSUPPORT_EPON_DUMMY
	printk("Initing gSendDummyTimer!\n");
	init_timer(&gSendDummyTimer) ;
        gDummyTimerInterval = gSendDummyTimer.data = 10;
	gSendDummyTimer.function = send_dummy_timer_func;    
	tasklet_init(&epon_dummy_tasklet, epon_dummy_tasklet_handler, 0); 
	
	struct proc_dir_entry *epon_dummy_proc;
	epon_dummy_proc = create_proc_entry("epon_dummy", 0, NULL);
	epon_dummy_proc->read_proc = epon_dummy_read_proc;
	epon_dummy_proc->write_proc = epon_dummy_write_proc;
#endif

	init_timer(&gEponTypeBTimer) ;
	gEponTypeBTimer.data = 1000;
	gEponTypeBTimer.function = epon_los_timer_func;
#ifdef TCSUPPORT_EPON_POWERSAVING
	init_timer(&gpEponPriv->early_wakeup_timer);
	gpEponPriv->early_wakeup_timer.data = gpEponPriv->eponCfg.earlyWakeupTimer;
	gpEponPriv->early_wakeup_timer.function = epon_early_wakeup_expires;
#endif

    tasklet_init(&gpEponPriv->epon_bitmap_format_tasklet, sendQueuePkt, 0);

#ifndef TCSUPPORT_CPU_EN7521
	fe_checkEponMacRestart = checkEponMacRestart;
#endif

	dbgPrintInit();

    memset(&g_txOamStats, 0, sizeof(g_txOamStats));
    init_timer(&gpEponPriv->oamCntTimer);
    gpEponPriv->oamCntTimer.data = 1000;  //1s 
    gpEponPriv->oamCntTimer.function = get_oam_trx_count;

	return ret;

}

void eponMacTableExit(void);

void eponExit(void)
{
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponExit");

	/* cmd exit */
	eponCmdExit();
	
	unregister_chrdev(EPON_MAC_MAJOR, "epon_mac");
	
	tasklet_kill(&eponMacMpcpDscvGateTask);
	tasklet_kill(&eponStartTask);
#ifdef TCSUPPORT_EPON_DUMMY	
	tasklet_kill(&epon_dummy_tasklet);
#endif	
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
#ifndef TCSUPPORT_CPU_EN7521
	fe_checkEponMacRestart = NULL;
#endif
	EPON_STOP_TIMER(gEponTypeBTimer);
    EPON_STOP_TIMER(gpEponPriv->oamCntTimer);

	return;
}

void eponMacTableInit(void)
{
	int i = 0;
#ifdef TCSUPPORT_CPU_EN7521
	eponMacRegTable = (eponMacHwtestReg_t *)kmalloc(sizeof(eponMacHwtestReg_t)*70, GFP_KERNEL) ;
#else
	eponMacRegTable = (eponMacHwtestReg_t *)kmalloc(sizeof(eponMacHwtestReg_t)*60, GFP_KERNEL) ;
#endif
	eponMacRegTable[i].addr = e_glb_cfg ;//reset bit can not be set
	eponMacRegTable[i].def_value = 0x00800042;
	eponMacRegTable[i].rwmask = 0x003f0007;
	i++;
	eponMacRegTable[i].addr = e_int_status;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_int_en;//must disable all INT
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_rpt_mpcp_timeout_llid_idx;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_dyinggsp_cfg;
	eponMacRegTable[i].def_value = 0x00000100;
	eponMacRegTable[i].rwmask = 0x8000037f;
	i++;
	eponMacRegTable[i].addr = e_pending_gnt_num;
	eponMacRegTable[i].def_value = 0x00000040;
	eponMacRegTable[i].rwmask = 0x0000007f;
	i++;
	eponMacRegTable[i].addr = e_llid0_3_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponMacRegTable[i].addr = e_llid4_7_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponMacRegTable[i].addr = e_llid_dscvry_ctrl;
	eponMacRegTable[i].def_value = 0x00010000;
	eponMacRegTable[i].rwmask = 0x80001107;
	i++;
	eponMacRegTable[i].addr = e_llid0_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid1_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid2_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid3_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid4_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid5_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid6_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_llid7_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = e_mac_addr_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x8000000f;
	i++;
	eponMacRegTable[i].addr = e_mac_addr_value;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_security_key_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x8000003f;
	i++;
	eponMacRegTable[i].addr = e_key_value;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_rpt_data;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = e_rpt_len;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0000013f;
	i++;
#ifdef TCSUPPORT_CPU_EN7521
	eponMacRegTable[i].addr = e_rpt_cfg;
	eponMacRegTable[i].def_value = 0x000a0000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_rpt_qthld_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
#endif
	eponMacRegTable[i].addr = e_local_time;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
#ifdef TCSUPPORT_CPU_EN7521
	eponMacRegTable[i].addr = e_pwr_sv_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_slp_durt_max;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_slp_duration;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_act_duration;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_pwron_dly;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_slp_duration_i;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
#endif
	eponMacRegTable[i].addr = e_txfetch_cfg;
	eponMacRegTable[i].def_value = 0x242a03e8;
	eponMacRegTable[i].rwmask = 0xffff0fff;
	i++;
	eponMacRegTable[i].addr = e_sync_time;//0x0000ffff, read only????
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0000ffff;
	i++;
	eponMacRegTable[i].addr = e_tx_cal_cnst;
	eponMacRegTable[i].def_value = 0x2612040c;
	eponMacRegTable[i].rwmask = 0xffffff3f;
	i++;
	eponMacRegTable[i].addr = e_laser_onoff_time;
	eponMacRegTable[i].def_value = 0x00002020;
	eponMacRegTable[i].rwmask = 0x0000ffff;
	i++;
	eponMacRegTable[i].addr = e_grd_thrshld;
	eponMacRegTable[i].def_value = 0x00000008;
	eponMacRegTable[i].rwmask = 0x000000ff;
	i++;
	eponMacRegTable[i].addr = e_mpcp_timeout_intvl;
	eponMacRegTable[i].def_value = 0x03b9aca0;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = e_rpt_timeout_intvl;
	eponMacRegTable[i].def_value = 0x002faf08;
	eponMacRegTable[i].rwmask = 0x00ffffff;
	i++;
	eponMacRegTable[i].addr = e_max_future_gnt_time;
	eponMacRegTable[i].def_value = 0x03b9aca0;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = e_min_proc_time;
	eponMacRegTable[i].def_value = 0x00000400;
	eponMacRegTable[i].rwmask = 0x0000ffff;
	i++;
	eponMacRegTable[i].addr = e_trx_adjust_time1;
	eponMacRegTable[i].def_value = 0x004ffff1;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = e_trx_adjust_time2;
	eponMacRegTable[i].def_value = 0x00000006;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = e_dbg_prb_sel;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0000001f;
	i++;
	eponMacRegTable[i].addr = e_dbg_prb_h32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask =0x00000000 ;
	i++;
	eponMacRegTable[i].addr = e_dbg_prb_l32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = e_rxmbi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_rxmpi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_txmbi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_txmpi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_oam_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_mpcp_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_mpcp_rgst_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_gnt_pending_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_gnt_length_stat;
	eponMacRegTable[i].def_value = 0x0000ffff;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_gnt_type_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_time_drft_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
#ifdef TCSUPPORT_CPU_EN7521
	eponMacRegTable[i].addr = e_snf_mpcp_oam_ctl;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = e_rpt_adj;
	eponMacRegTable[i].def_value = 0x00000002;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
#endif	
	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponMacRegTable[i].addr = NULL;
	eponMacRegTable[i].rwmask = 0x0;

}

void eponMacTableExit(void)
{
	kfree(eponMacRegTable);
	eponMacRegTable = NULL;
}

int eponMacDumpAllReg(void){
	__u32 n = 0;
	__u32 Raw = 0;

	if(NULL == eponMacRegTable){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: dump epon mac register fail");
		return -1;
	}
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "Dump EPON MAC REG");
		
	n = 0;
	while(eponMacRegTable[n].addr != 0){
		Raw = READ_REG_WORD(eponMacRegTable[n].addr);
		printk("\r\n %X : %X" , eponMacRegTable[n].addr, Raw);
		n++;
	}
	
//	printk("\ndump all llid threshold\n");
//	eponDumpAllLlidQueThrod();
	return 0;
}



#ifdef EPON_MAC_HW_TEST
static int doEponHwtest(int argc, char *argv[], void *p)
{
	return subcmd(eponHwtestCmds, argc, argv, p);
}


static int doEponRegtest(int argc, char *argv[], void *p){
	__u32 times =0;
	if(argc < 2){
		printk("\r\nregtest <times>" );
		return 0;
	}


	times = simple_strtoul(argv[1], NULL, 16);
	eponMacRegTest(times);
	return 0;
}
static int doEponRegDefCheck(int argc, char *argv[], void *p){
	eponMacSwReset();
	
	if (eponRegDefCheck(eponMacRegTable , eponMacGetRegTblSize()) == -1)
	{
		printk("EPON Register default value verifycation is failure!!\r\n");
	}
	else
	{
		printk("EPON Register default value verifycation is ok!!\r\n");
	}
	
	return 0;
}


#endif/* EPON_MAC_HW_TEST */

