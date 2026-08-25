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
#include "epon/epon_ioctl.h"
#include "epon/epon_timer.h"
#include "epon/epon_reg.h"
#include "xmcs/xmcs_sdi.h"
#include <asm/tc3162/cmdparse.h>


#define MODULE_VERSION_EPON	"MTK EPON MAC driver version 0.1"
#define EPON_LLID_MAX_NUM		8
#define EPON_MAC_MAJOR		221

#define EPON_TIMEDRIFT_THRSHLD 	0x08
#define EPON_TIMEDRIFT_RESET_THRSHLD 	0x10

#define EPON_STOP_TIMER(timer)	do	            \
			{                                   \
				if(in_interrupt()) {            \
					del_timer(&timer) ;         \
				} else {                        \
					del_timer_sync(&timer) ;    \
				}                               \
			}while(0)

#define EPON_START_TIMER(timer)			{ mod_timer(&timer, (jiffies + ((timer.data*HZ)/1000))) ; }

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
} EPON_GlbPriv_T ;

#define SNIFFER_DISABLE 0
#define SNIFFER_NORMAL 1
#define SNIFFER_PROMISC 2

#if 0
#if DBG
u32    RTDebugLevel;
#define DBGPRINT(Level, fmt, args...) 					\
{                                   \
    if (Level <= RT_DEBUG_TRACE)      \
    {                               \
        printf(NIC_DBG_STRING);   \
		printf( fmt, ## args);			\
    }                               \
}
#else
#define DBGPRINT(Level, fmt, args...) 	
#endif
#endif





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
	
	eponLlid_t eponLlidEntry[EPON_LLID_MAX_NUM];
	__u32 eponTxPowerFlag;
}epon_t, *epon_p;






extern int cmd_register(cmds_t *);
extern int cmd_unregister(char *name);
extern int subcmd(const cmds_t tab[], int argc, char *argv[], void *p);

int eponLlidEnable(__u8 llidIndex);
int eponLlidDisable(__u8 llidIndex);
int eponMacRestart(void);
int eponWanResetWithChannelRetire(void);
int eponMacDumpAllReg(void);

int eponMacGetRegTblSize(void);

void eponStart(unsigned long);
void eponStop(void);

int eponInit(void);
void eponExit(void);
#ifdef TCSUPPORT_CPU_EN7521
int epon_set_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int epon_get_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int eponSetLlidThrshldNum(__u8 llidIndex, __u8 num);
int eponGetLlidThrshldNum(__u8 llidIndex, __u8 *num);
#endif

int eponHwDygaspCtrl(__u8 flag);
int eponDevGetDyingGaspMode(__u8 *mode);
int eponDevSetDyingGaspNum(__u32 num);
int eponDevGetDyingGaspNum(__u32 *num);

void eponDetectPhyReady(void);
void eponDetectPhyLosLof(void);

#ifdef TCSUPPORT_EPON_POWERSAVING
int eponSetEarlyWakeUpConfig(__u8 enable, __u32 timer, __u16 count);
#endif
int eponSetSnifferModeConfig(__u8 enable, __u16 mask);
#define EPON_IFG_DEFAULT  8
void eponSetIFG(unchar ifg_val);

#endif /* __EPON_H */

