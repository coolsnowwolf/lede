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
	epon.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/

#ifndef __EPON_H
#define __EPON_H

#include "../common/xpondrv.h"
#include "epon/epon_types.h"
#include "epon/epon_util.h"
#include "epon/epon_mpcp.h"
#include "xpon_epon_ioctl.h"
#include "epon/epon_timer.h"
#include "epon/epon_reg.h"
#include "xmcs/xmcs_sdi.h"
#include <asm/tc3162/cmdparse.h>


#define MODULE_VERSION_EPON	"MTK EPON MAC driver version 0.1"


#define EPON_TIMEDRIFT_THRSHLD 	0x08
#define EPON_TIMEDRIFT_RESET_THRSHLD 	0x10

#define EPON_QUEUE_NUM_PER_LLID  8
#define EPON_DEFAULT_THRESHOLD   1000	/*unit is TQ*/

#define FE_CHANNEL_RETIRE_DISABLE   0
#define FE_CHANNEL_RETIRE_ENABLE    1

#define EPON_CHECK_TXRX_FRQ      10000
#define EPON_KEEPALIVE_OAM_THRESHOLD 5

#define EPON_STOP_TIMER(timer)	do	            \
			{                                   \
				if(in_interrupt()) {            \
					del_timer(&timer) ;         \
				} else {                        \
					del_timer_sync(&timer) ;    \
				}                               \
			}while(0)

#define EPON_START_TIMER(timer,para)	{ timer.expires = para;mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }

int epon_create_timer(struct timer_list *timer, eponTimerCallback callback, unsigned long param);
#define EPON_CREATE_TIMER(timer,func,para) epon_create_timer(timer,func,para)


typedef struct{
    uint tryCnt;
    uint hwPre;
    uint hwNxt;
    uint swPre;
    uint swNxt;
    uint hwFlg;
    uint swFlg;
}txOamCnt_S;

/*put log to memory*/
void dbgtoMem(__u32 debugLevel,char * fmt,...);
typedef struct {
	__u8 snifferModeEnable;
	__u8 snifferModeLanMask;
} EPON_SNIFFER_MODE_T;

typedef struct eponGrtLen_s{
    unsigned int grtLen;
    unsigned int grtFlag;
    unsigned int grtMode;
    unsigned short int mpcpGrantLen;
    unsigned short int grtLenStep;
}eponGrtLen_t, *eponGrtLen_p;

typedef struct {
	__u32 earlyWakeupTimer;
	__u8 earlyWakeupFlag;
	__u16 earlyWakeupCount;
	__u8 txOamFavorMode;
	EPON_SNIFFER_MODE_T snifferModeConfig;
} EPON_Config_T ;


typedef struct {
	struct timer_list		early_wakeup_timer;
	EPON_Config_T			eponCfg ;
	__u8 mpcpInterruptMode; /* 0: polling mode; 1: interrupt mode */
    uint                    gBitmapSet;
    uint                    BitmapFlag;
    uint                    maxTryCnt;
    struct tasklet_struct   epon_bitmap_format_tasklet;
    struct timer_list       oamCntTimer;
    uint                    oamChannel;
    eponGrtLen_t eponRdmMfy;
	int fwdFlag;
} EPON_GlbPriv_T ;

#define SNIFFER_DISABLE 0
#define SNIFFER_NORMAL 1
#define SNIFFER_PROMISC 2

/* debug level define */
#define EPON_DEBUG_LEVEL_NONE		0
#define EPON_DEBUG_LEVEL_ERR		1
#define EPON_DEBUG_LEVEL_NOTIFY		2
#define EPON_DEBUG_LEVEL_ISR			3
#define EPON_DEBUG_LEVEL_TRACE		4


#define EPON_DBGMASK_TEMPMAC 1

//define struct for each LLID
typedef struct eponLlid_s{
	__u8 llidIndex;
	__u8 enableFlag;
	__u16 llid;
	__u8 macAddr[6];
	__u8 oui[3];
	__u8 vendorSpecInfo[4];	
	eponMpcp_t eponMpcp;
	//spinlock_t mpcpDscvStsLock;
}eponLlid_t, *eponLlid_p;

//EPON main strcut
typedef struct epon_s{
	__u32 llidMask;
	__u8 hldoverEnable;//0:disable; 1:enable
	__u16 hldOverTime;//ms
	__u8 typeBOnGoing;
	//__u32 debugLevel;
	__u32 timeDrftEq255Cnt;//MAX_timedrift=255 counter
	__u32 timeDrftFrom8To16Cnt;//MAX_timedrift = 8~16 counter
	
	eponLlid_t eponLlidEntry[EPON_1G_MAX_LLID_NUM];
	__u32 eponTxPowerFlag;
    u32 	eponFecAdjust;
    __u16   DefaultThreshold;
}epon_t, *epon_p;

typedef struct
{
    /*mpcp counter*/
    uint16_t     mpcpErrCnt;
    uint8_t     mpcpRgstCnt;
    uint16_t    mpcpDscvGateCnt;
    uint8_t     mpcpRgstReqCnt;
    uint8_t     mpcpRgstAckCnt;
	uint32_t	mpcpNormalGateCnt;
	uint16_t	mpcpTxReportCnt;

    /*eth counter*/
    uint32_t    rxMpiEth;
    uint32_t    rxMbiEth;
    uint32_t    txMbiEth;
    uint32_t    txMpiEth;

    /*oam counter*/
    uint16_t    rxOamCnt;
    uint16_t    txOamCnt;

#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    /*CRC error counter*/
    uint8_t     rxPrmbCrc8ErrCnt;
    uint32_t    rxCrc32ErrCnt;//include data and OAM packet
#endif
}XPON_DEBUG_STATISTIC,*pXPON_DEBUG_STATISTIC;

typedef struct
{
    uint32_t    glbCfg;
    uint32_t    intSts;
    uint32_t    intEn;
    uint32_t    llidCfg0_3;
    uint32_t    rptCfg;
    uint32_t    laserTm;
    uint32_t    syncTm;
    uint32_t    txCnst;
    uint32_t    pwrSvCfg;
    uint32_t    dygspCfg;
}XPON_DEBUG_STATE,*pXPON_DEBUG_STATE;

/*Set register value, init does't modify it*/
typedef struct reg_info_s{   
    unsigned int        addr;
    unsigned int        value;
}REG_INFO_T;

typedef struct fix_reg_list_entry_s{
    struct list_head    list; 
    struct rcu_head		rcu;
    REG_INFO_T	regInfo;
}FIX_REG_LIST_ENTRY_T;

typedef enum{
	EPON_RXTX_TIMER_STOP = 0,
	EPON_RXTX_TIMER_CONTINUE
}EPON_RXTX_TIMER_FLAG;

extern int cmd_register(cmds_t *);
extern int cmd_unregister(char *name);
extern int subcmd(const cmds_t tab[], int argc, char *argv[], void *p);
extern struct timer_list eponCheckTxRxTmr;
extern __u32 oam_tx_cnt;
extern __u32 oam_rx_cnt;
extern __u32 enum_rx;
extern __u32 oam_issue_flag;
extern uint oam_fav_set_disabled;
extern uint rcv_srv_oam_flag;
extern uint keepalive_count;
extern void oam_check_txrx_reset(int stop_flag);

int eponLlidEnable(__u8 llidIndex);
int eponLlidDisable(__u8 llidIndex);
void FEChannelRetire(void);
int eponMacDumpAllReg(void);

int eponMacGetRegTblSize(void);

void eponStart(unsigned long);
void eponStop(void);

void eponGlbMacCtrlInit(void);
void eponIntEnableInit(void);
void eponMpcpReportInit(void);
int eponLlidMacInit(void);
void eponRegInitConfig( void);
void eponFeChannelEnable(void);
void eponRegAndFeChannelConfig(void);
void eponSetFecAdjust(void);
void eponMacReset(void);
void eponMacReinit(int isFeChlRetire);

int eponMacTxRxDisable(void);
int eponMacTxRxEnable(void);

int eponInit(void);
void eponExit(void);
int epon_set_llid_report_bitmap(__u8 llidIndex, __u8 bitmap);
int epon_get_llid_report_bitmap(__u8 llidIndex, __u8* bitmap);
int epon_set_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int epon_get_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int eponSetLlidThrshldNum(__u8 llidIndex, __u8 num);
int eponGetLlidThrshldNum(__u8 llidIndex, __u8 *num);

int eponHwDygaspCtrl(__u8 flag);
int eponDevGetDyingGaspMode(__u8 *mode);
int eponDevSetDyingGaspNum(__u32 num);
int eponDevGetDyingGaspNum(__u32 *num);

void eponSetReportData(void);
void eponSetStaticReport(void);

void eponDetectPhyReady(void);
void eponDetectPhyLosLof(void);

#ifdef TCSUPPORT_EPON_POWERSAVING
int eponSetEarlyWakeUpConfig(__u8 enable, __u32 timer, __u16 count);
#endif
int eponSetSnifferModeConfig(__u8 enable, __u16 mask);

#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
#define EPON_IFG_DEFAULT  12
#else
#define EPON_IFG_DEFAULT  8
#endif
#define EPON_DYING_GASP_NUM 3
#define EPON_MAC_ADDRESS_LENTH   (6)
#define EPON_MAC_ADDRESS_OFFSET	 (0xff48)

void eponSetIFG(unchar ifg_val);
void eponWanDevice(void);
void eponSetLlid0DeafultThreshold(void);

int eponMacOpen(struct inode *inode, struct file *filp);

#endif /* __EPON_H */

