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
	epon_mpcp.c

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
#include <linux/kthread.h>

#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>

#include "epon/epon.h"
#include "common/drv_global.h"
#include "../../inc/common/phy_if_wrapper.h"

#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
extern int led_xpon_status;
#endif


#define MPCP_SYNC_TIME_MAX  ((isEPONFWID)?(0x40):(0x5F))

int gDiscoveryGateMaxCnt = 100;
int gDiscoveryGateCnt = 0;
int gFeResetRetryCnt = 0;

unsigned short int gMpcpSynctime = 0x20;	
extern epon_t eponDrv;
spinlock_t mpcpDscvGateLock;
spinlock_t mpcpDscvPendingLock;
extern __u32 eponDebugLevel;
int isAuthFail = 0;
int isSlientBegin = 0;
#ifndef TCSUPPORT_CPU_EN7521
extern void setMpcpFwd(int flag);
#endif
__u8 g_silence_time = MPCP_RGST_AGAIN_TIMEOUT;

extern void eponFirmwareSetMode(int mode);

/*****************************************************************************
//Function :
//		max_dscv_gate_read_proc
//Description : 
//		This function is read proc of feResetFlag
//Input :	
//		N/A
//Output :
//		N/A
//Date:
//		
//
******************************************************************************/
int max_dscv_gate_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;
	
	uint read_data = 0;
	
	len = sprintf(page, "%d\n", gDiscoveryGateMaxCnt);

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


/*****************************************************************************
//Function :
//		max_dscv_gate_write_proc
//Description : 
//		This function is write proc of feResetFlag
//Input :	
//		N/A
//Output :
//		N/A
//Date:
//		
//
******************************************************************************/
int max_dscv_gate_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[8];
	int cnt;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';
	sscanf(val_string, "%d", &cnt) ;
	
	if(cnt != 0){
		gDiscoveryGateMaxCnt = cnt;
	}
    
	gFeResetRetryCnt = 0;
	gDiscoveryGateCnt = 0;
	
	return count;
}

int eponMacCheckRegreqDoneBit(void){
	REG_e_llid_dscvry_ctrl dscvCtrl;


	//memset(&dscvCtrl , 0 , sizeof(REG_e_llid_dscvry_ctrl));
	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	
	//check done bit
	if(dscvCtrl.Bits.mpcp_cmd_done == 1)
	{
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMacCheckRegreqDoneBit done");
		return 0;
	}else{
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMacCheckRegreqDoneBit not ready");
		return -1;
	}
	return 0;
}


#if 0
int eponMacTimerCheckRegreqDoneBit(eponLlid_t *llidEntry_p){
	REG_e_llid_dscvry_ctrl dscvCtrl;

	memset(&dscvCtrl , 0 , sizeof(REG_e_llid_dscvry_ctrl));
	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	
	//check done bit
	if(dscvCtrl.Bits.mpcp_cmd_done == 1)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\neponMacTimerCheckRegreqDoneBit done llidindex=%d", llidEntry_p->llidIndex);
		eponMacDelTimer(&(llidEntry_p->eponMpcp.eponDiscFsm.eponMpcpTimer));
	}else{
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\neponMacTimerCheckRegreqDoneBit not ready!!! llidindex=%d", llidEntry_p->llidIndex);
	}
	return 0;
}
#endif

extern __u8 eponOnuMacAddr[6];
uint eponGetRandomRetryTimes(void)
{
    uint random_num = 0;
    get_random_bytes(&random_num, sizeof(random_num));
    return (__u8)((random_num & 0x07) + 1);
}

atomic_t mpcpTmOutCnt = ATOMIC_INIT(0);

void eponMpcpDiscvGateIntHandler(unsigned long data){
	__u8 llidIndex = 0;
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[0]);
	REG_e_llid_dscvry_ctrl dscvCtrl;
	int pollingCnt = 0;
	__u32 tmp = 0;
	unsigned long flags = 0;
	REG_e_sync_time eSyncTime;
	eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
	//static __u8 i = 0;
    atomic_t mpcpTmOutCnt = ATOMIC_INIT(0);
    
	if(gpPonSysData->sysLinkStatus != PON_LINK_STATUS_EPON)
		return;
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMpcpDiscvGateIntHandler gMpcpSynctime =0x%x", gMpcpSynctime);

	MPCP_SYNC_TIME_HANDLER(gMpcpSynctime);
#ifndef TCSUPPORT_CPU_EN7521
	setMpcpFwd(0);
#endif

	//check all LLID status ,who need discovery
	for(llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM; llidIndex++, llidEntry_p++){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDiscvGateIntHandler llidIndex=%d state=%d", llidIndex , llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState);
		if(llidEntry_p->enableFlag== XPON_DISABLE){
			continue;
		}	
		if((llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState == MPCP_STATE_REGISTERING) 
		 ||(llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState == MPCP_STATE_REGISTER_REQUEST))
		{			
			if(llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState == MPCP_STATE_REGISTER_REQUEST){
				dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "MPCP DiscState is REGISTER_REQUEST");				
			}
		
			// from HW's suggest, MPCP request should not send every time
			if(llidEntry_p->eponMpcp.eponDiscFsm.mpcpReqRetryFlag > 0 ){
				//skip this time
				llidEntry_p->eponMpcp.eponDiscFsm.mpcpReqRetryFlag -= 1;
				//continue;
				//llid should register one by one
				return ; // return 0;
			}else{
				gDiscoveryGateCnt++;
				dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDiscvGateIntHandler gDiscoveryGateCnt=%d,gFeResetRetryCnt=%d", gDiscoveryGateCnt,gFeResetRetryCnt);

				if(gDiscoveryGateCnt >= gDiscoveryGateMaxCnt ){               
					if(gFeResetRetryCnt <= 2){
#ifndef TCSUPPORT_CPU_EN7521
				        gpPhyData->phy_link_status  = PHY_LINK_STATUS_LOS ;
				        gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
				        schedule_fe_reset();
#endif
				        gFeResetRetryCnt++; 
					}
					gDiscoveryGateCnt = 0;
				}
    			
				if((eponMacCheckRegreqDoneBit() <0) ){
					dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "Epon MPCP DiscvGate IntHandler done bit not ready");
					if(eponDebugLevel >=EPON_DEBUG_LEVEL_TRACE ){
						eponMacDumpAllReg();
					}
					return ; //return -1;
				}
				llidEntry_p->eponMpcp.eponDiscFsm.mpcpReqRetryFlag = eponGetRandomRetryTimes();//retry random times
			}

			if((llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState == MPCP_STATE_REGISTERING) ||(llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState == MPCP_STATE_REGISTER_REQUEST)){
			spin_lock_irqsave(&mpcpDscvGateLock,flags);
			pollingCnt = 0;
			//change disc status to unregister
			
#if 1	//TCSUPPORT_FPGA_VERIFY
			tmp = READ_REG_WORD(e_llid0_dscvry_sts+llidIndex) ;
			tmp &= 0x3fffffff;
			WRITE_REG_WORD(e_llid0_dscvry_sts+llidIndex , tmp);	
				
			while(pollingCnt <30){
				//tmp = READ_REG_WORD(e_llid0_dscvry_sts+llidIndex) ;
				pollingCnt++;
			}
#else
			while(pollingCnt <10){
				tmp = READ_REG_WORD(e_llid0_dscvry_sts+llidIndex) ;
				tmp &= 0x3fffffff;
				WRITE_REG_WORD(e_llid0_dscvry_sts+llidIndex , tmp);				
				pollingCnt++;
			}
#endif


			//change disc status to registing
			tmp = READ_REG_WORD(e_llid0_dscvry_sts+llidIndex) ;
			tmp &= 0x3fffffff;
			tmp |= 0x7fffffff;
			WRITE_REG_WORD(e_llid0_dscvry_sts+llidIndex , tmp);
		
            eponFirmwareSetMode(0);/*make sure it is burst mode before send REQ*/
			//send register_REQ
			dscvCtrl.Raw = 0;//READ_REG_WORD(e_llid_dscvry_ctrl);
			
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDiscvGateIntHandler READ_REG_WORD(e_llid_dscvry_ctrl)");

			dscvCtrl.Bits.tx_mpcp_llid_idx = llidIndex;
			dscvCtrl.Bits.rgstr_req_flg = 0;
			dscvCtrl.Bits.mpcp_cmd = 1;
			dscvCtrl.Bits.mpcp_cmd_done = 1;
			WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw);
			
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDiscvGateIntHandler enter while dscvCtrl.Raw=%08X", dscvCtrl.Raw);
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "EponIsr Receive DiscvGate Message");
			
		if (!gpEponPriv->mpcpInterruptMode)
		{
			pollingCnt = 0;
			while(1){
				dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
				//do not check now
				if(dscvCtrl.Bits.mpcp_cmd_done == 1)
				{
					break;
				}
				if(pollingCnt >= 5000000){
					dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDiscvGateIntHandler done bit timeout");
					break;
				}
				pollingCnt++;
			}
			eponFirmwareSetMode(1);

			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Send MPCP REGISTER_REQUEST DiscvGate Finished");

			//change status
			llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_REGISTER_REQUEST;
		}
			spin_unlock_irqrestore(&mpcpDscvGateLock,flags);
			}

#if defined(TCSUPPORT_XPON_LED)
			ledTurnOff(LED_XPON_STATUS);		
#if defined(TCSUPPORT_CT_PON_CN_CN) 
				if (TRUE == XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )
#endif
					ledTurnOn(LED_XPON_TRYING_STATUS);
			led_xpon_status = 1;	
#endif
			//llid should register one by one
			return ; //return 0;
		}		
			
	}
	
	return ; //return 0;
}

/*****************************************************************************
*function :
*		eponFirmwareModeSelect
*description : 
*		decide working mode should be mode3 or mode4 and set continue mode 
*input :
*		N/A
*output :
*		N/A
******************************************************************************/
static void eponFirmwareModeSelect(void){
	REG_e_sync_time eSyncTime;

	if (isEPONFWID)
	{
		/* decide working mode should be mode3 or mode4 */
		eSyncTime.Raw = READ_REG_WORD(e_sync_time);
		if (eSyncTime.Bits.sync_time <= 32){
			/* sync time <= 32 use mode 3 */
			IO_CBITS(e_mpcp_timeout_intvl,(1<<31));
			IO_CBITS(e_mpcp_timeout_intvl,(1<<30));
		}else{
			/* sync time > 32 use mode 4 */
			IO_CBITS(e_mpcp_timeout_intvl,(1<<31));
			IO_SBITS(e_mpcp_timeout_intvl,(1<<30));
		}
		
	}
	/* set continue mode */
	eponFirmwareSetMode(1);	
}

void eponClearRetryCounter(void)
{

	gDiscoveryGateCnt = 0;
	gFeResetRetryCnt = 0;

}	

int eponMpcpRgstReqIntHandler(void)
{
	__u8 llidIndex = 0;
	eponLlid_t *llidEntry_p = NULL;
	REG_e_llid_dscvry_ctrl dscvCtrl;

	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	llidIndex = dscvCtrl.Bits.tx_mpcp_llid_idx;

	llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);

	llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState = MPCP_STATE_REGISTER_REQUEST;
	dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Send MPCP REGISTER_REQUEST DiscvGate Finished, llidIndex = %d", llidIndex);

	return 0;
}

int eponMpcpRgstAckIntHandler(void)
{
	__u8 llidIndex = 0;
	eponLlid_t *llidEntry_p = NULL;
	eponMpcpDiscFsm_t *mpcpDiscFsm = NULL;
	REG_e_llid_dscvry_ctrl dscvCtrl;
	eponLlidDscvStsReg_t sdcvSts;
	struct XMCS_EponLlidCreate_S eponLlidCreate;

	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	llidIndex = dscvCtrl.Bits.tx_mpcp_llid_idx;

	llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);

	//set LLID_DSCVRY_STS
	 sdcvSts.Raw = 0;
        sdcvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts+ llidIndex);
        sdcvSts.Bits.llidDscvrySts = 2;
        WRITE_REG_WORD(e_llid0_dscvry_sts+ llidIndex, sdcvSts.Raw);

	//change status
        mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTERED;
        atomic_set(&mpcpTmOutCnt, 0);
	eponClearRetryCounter();

	//create epon interface
        eponLlidCreate.idx = llidIndex;
        eponLlidCreate.llid = llidEntry_p->llid;
	xmcs_report_event(XMCS_EVENT_TYPE_EPON,XMCS_EVENT_EPON_REGISTER,llidIndex);
	
	dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Send MPCP Ack, Mpcp Handshake Done,llidIndex = %d", llidIndex);
	xmcs_create_llid(&eponLlidCreate);

	/* decide working mode should be mode3 or mode4 
           and set continue mode */
        eponFirmwareModeSelect();
#ifndef TCSUPPORT_CPU_EN7521
        enable_cpu_us_omci_oam_traffic();
        disable_cpu_us_data_traffic();
#endif
        start_omci_oam_monitor();

        return 0;
}

#ifdef TCSUPPORT_AUTOBENCH
int epon_slt_test = 0;
EXPORT_SYMBOL(epon_slt_test);
#endif

static int eponMpcpDscvFsmPendingHandler(__u8 llidIndex){
    //eponLlidDscvStsReg_t llidDscvSts;
    eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
    eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
    REG_e_llid_dscvry_ctrl dscvCtrl;
    eponLlidDscvStsReg_t sdcvSts;
    __u32 cnt = 0;
    unsigned long flags;
    
    struct XMCS_EponLlidCreate_S eponLlidCreate;

    if(eponMacCheckRegreqDoneBit() <0){
        dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMpcpDscvFsmPendingHandler done bit not ready");
        return -1;
    }
    
    dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDscvFsmPendingHandler done bit OK!! RgstrAck=%d", mpcpDiscFsm->mpcpDiscMacr_RgstrAck);

    spin_lock_irqsave(&mpcpDscvPendingLock,flags);      
    sdcvSts.Raw = 0;

    //MPCP FSM pengding handler
    if(mpcpDiscFsm->mpcpDiscMacr_RgstrAck == ACK){
        //ONU want to ACK
        mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTER_ACK;
        mpcpDiscFsm->registered = TRUE;
        //send register ack
        dscvCtrl.Raw = 0;//READ_REG_WORD(e_llid_dscvry_ctrl);
        dscvCtrl.Bits.mpcp_cmd = 3;
        dscvCtrl.Bits.rgstr_ack_flg = 1;
        dscvCtrl.Bits.tx_mpcp_llid_idx = llidIndex;//////////////
        dscvCtrl.Bits.mpcp_cmd_done = 1;
        WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw);       

	if (!gpEponPriv->mpcpInterruptMode)
	{
        cnt =0;
        while(1){
            cnt++;
            dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
            if(dscvCtrl.Bits.mpcp_cmd_done == 1)
            {
                dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDscvFsmPendingHandler mpcp_cmd_done, cnt=%d", cnt);
                break;
            }
            if(cnt > 1000000){
                //eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponMpcpDscvFsmPendingHandler cnt > 100000 cnt=%d", cnt);
                //break;
#ifdef EPON_MAC_HW_TEST
                break;
#endif
                
            }
        }
        //set LLID_DSCVRY_STS
        sdcvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts+ llidIndex);
        sdcvSts.Bits.llidDscvrySts = 2;
        WRITE_REG_WORD(e_llid0_dscvry_sts+ llidIndex, sdcvSts.Raw);

        //change status
        mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTERED;
		atomic_set(&mpcpTmOutCnt, 0);
		eponClearRetryCounter();
        
        if(gpEponPriv->gBitmapSet){
            XPON_START_TIMER(gpEponPriv->oamCntTimer);
        }
        
        //create epon interface
        eponLlidCreate.idx = llidIndex;
        eponLlidCreate.llid = llidEntry_p->llid;
        xmcs_report_event(XMCS_EVENT_TYPE_EPON,XMCS_EVENT_EPON_REGISTER,llidIndex);
#ifdef TCSUPPORT_AUTOBENCH
        epon_slt_test = 1;
#endif
        dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Send MPCP Ack, Mpcp Handshake Done");
        
        xmcs_create_llid(&eponLlidCreate);

        /* decide working mode should be mode3 or mode4 
           and set continue mode */
        eponFirmwareModeSelect();
#ifndef TCSUPPORT_CPU_EN7521
        enable_cpu_us_omci_oam_traffic();
        disable_cpu_us_data_traffic();
#endif
        start_omci_oam_monitor();
	}
    }

    if(mpcpDiscFsm->mpcpDiscMacr_RgstrAck == NACK){
        //ONU want to NACK
        mpcpDiscFsm->mpcpDiscState = MPCP_STATE_NACK;

        //send back NACK to MPCPDU
        dscvCtrl.Raw = 0;//READ_REG_WORD(e_llid_dscvry_ctrl);
        dscvCtrl.Bits.mpcp_cmd = 3;
        dscvCtrl.Bits.rgstr_ack_flg = 0;
        dscvCtrl.Bits.tx_mpcp_llid_idx = llidIndex;//////////////
        dscvCtrl.Bits.mpcp_cmd_done = 1;
        WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw);
        cnt =0;
        while(1){
            cnt++;
            dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
            if(dscvCtrl.Bits.mpcp_cmd_done == 1)
            {
                dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDscvFsmPendingHandler111  mpcp_cmd_done cnt=%d", cnt);
                break;
            }
            if(cnt > 1000000){
                //eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\neponMpcpDscvFsmPendingHandler 111 cnt > 10000");
                //break;
            }
        }
        /*set Burst mode */
        eponFirmwareSetMode(0);
        
        //change status
        mpcpDiscFsm->mpcpDiscState = MPCP_STATE_WAIT;
        eponMpcpDscvFsmWaitHandler(llidIndex);
        dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Send MPCP Nack, MPCP Retry");
    }
    spin_unlock_irqrestore(&mpcpDscvPendingLock,flags); 
    return 0;
}

typedef enum{
    MPCP_REG_RE_REGISTER,
    MPCP_REG_DE_REGISTER,
    MPCP_REG_ACK        ,
    MPCP_REG_NACK
} Mpcp_Register_Flag_t;


int eponMpcpRgstIntHandler(__u8 llidIndex){
	eponLlidDscvStsReg_t llidDscvSts;
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
	//REG_e_llid_dscvry_ctrl dscvCtrl;
	eponLlidDscvStsReg_t sdcvSts;
	REG_e_sync_time eSyncTime;

	sdcvSts.Raw = 0;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMpcpRgstIntHandler llidIndex=%d, mpcpDiscState=%d", llidIndex, mpcpDiscFsm->mpcpDiscState);


	if(llidEntry_p->enableFlag== XPON_DISABLE){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMpcpRgstIntHandler enableFlag== DISABLE");
		return 0;
	}	
	
	if(mpcpDiscFsm->begin == FALSE){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRgstIntHandler lidEntry_p->eponMpcp.eponDiscFsm.begin == FALSE");
		return 0;
	}

	llidEntry_p->eponMpcp.eponDiscFsm.mpcpReqRetryFlag = 0;

	llidDscvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts + llidIndex);
	
	if(MPCP_REG_ACK != llidDscvSts.Bits.rgstrFlgSts){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRgstIntHandler(%d)  llidDscvSts=%x", llidIndex, llidDscvSts.Raw);
	}
    
	if(mpcpDiscFsm->mpcpDiscState == MPCP_STATE_REGISTER_REQUEST){
		dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Receive MPCP Register, Now State is REGISTER_REQUEST, llidDscvSts=%x", llidDscvSts.Raw);

		MPCP_SYNC_TIME_HANDLER(gMpcpSynctime);

		if(MPCP_REG_ACK == llidDscvSts.Bits.rgstrFlgSts){
			// flag_rx= ack
			if( llidDscvSts.Bits.llidValid == 1){
				llidEntry_p->llid = llidDscvSts.Bits.llidValue;
				dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRgstIntHandler LLID=%d llidEntry_p->llid =%d", llidIndex, llidEntry_p->llid);
			}
			else{
				dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMpcpRgstIntHandler  llidDscvSts.Bits.llidValid error");
				return -1;
			}
			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTER_PENDING;

			eponMpcpDscvFsmPendingHandler(llidIndex);	
			gFeResetRetryCnt = 0;
			gDiscoveryGateCnt = 0;

			dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponMpcpRgstIntHandler:gFeResetRetryCnt = 0");			
		}

		
		if(MPCP_REG_NACK == llidDscvSts.Bits.rgstrFlgSts){	
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "eponMpcpRgstIntHandler LLID=%d flag_rx=nack" , llidIndex);
			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_DENIED;
			xmcs_report_event(XMCS_EVENT_TYPE_EPON,XMCS_EVENT_EPON_DENIED,llidIndex);

			if(0 == g_silence_time)
			{
				mpcpDiscFsm->mpcpDiscState = MPCP_STATE_WAIT;	
			}
			else
			{
				llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout = g_silence_time;
				dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon received MPCP Nack change to silent, time %d",g_silence_time);
			}
			
#if defined(TCSUPPORT_XPON_LED)
#if !defined(TCSUPPORT_CT_PON_C9)
			ledTurnOff(LED_XPON_STATUS);		
#if !defined(TCSUPPORT_CUC) 
#if !defined(TCSUPPORT_CT_PON_CN_JS)
			ledTurnOff(LED_XPON_TRYING_STATUS);
#endif
			led_xpon_status = 0;	
#endif
#endif
#endif						
			eponMpcpDscvFsmWaitHandler(llidIndex);
		}
	}else if(mpcpDiscFsm->mpcpDiscState == MPCP_STATE_REGISTERED){
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRgstIntHandler  mpcpDiscState==MPCP_STATE_REGISTERED");

		if(MPCP_REG_RE_REGISTER == llidDscvSts.Bits.rgstrFlgSts){
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Receive MPCP Reregister Msg LLID %d ", llidIndex);

			//set LLID_DSCVRY_STS
			sdcvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts+ llidIndex);
			sdcvSts.Bits.llidDscvrySts = 1;
			WRITE_REG_WORD(e_llid0_dscvry_sts+ llidIndex, sdcvSts.Raw);

			
			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTER_PENDING;

			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Receive REREGISTER Message , Report DEREGISTER Event");
			eponMpcpDscvFsmPendingHandler(llidIndex);
			
			/* set burst mode */
			/*eponFirmwareSetMode(0);*/	
		}

		if(MPCP_REG_DE_REGISTER == llidDscvSts.Bits.rgstrFlgSts){
			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "Epon Receive MPCP DEREGISTER Message LLID %d ", llidIndex);

			if(isAuthFail)
            {
				mpcpDiscFsm->mpcpDiscState = MPCP_STATE_DENIED;
				xmcs_report_event(XMCS_EVENT_TYPE_EPON,XMCS_EVENT_EPON_DENIED,llidIndex);
				dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "AuthFail DiscState Change to DENIED, Report EPON_DENIED Event ");
				dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "change to silent, time %d",g_silence_time);				
				llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout = g_silence_time;
				
            #if defined(TCSUPPORT_XPON_LED) && defined(TCSUPPORT_CUC)
				ledTurnOff(LED_XPON_STATUS);		
				ledTurnOn(LED_XPON_TRYING_STATUS);
				led_xpon_status = 1;
            #endif	
			}
			else
			{
    			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REMOTE_DEREGISTER;

    			//	 TODO: MACI(register , status = deregister)
    			xmcs_report_event(XMCS_EVENT_TYPE_EPON,XMCS_EVENT_EPON_DEREGISTER,llidIndex);
    			dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "DiscState Change to DEREGISTER , Report DEREGISTER Event");

    			xmcs_remove_llid(llidIndex);
    			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_WAIT;
    			
            #if defined(TCSUPPORT_XPON_LED) && defined(TCSUPPORT_CUC)
    			ledTurnOff(LED_XPON_STATUS);		
    			ledTurnOff(LED_XPON_TRYING_STATUS);
    			led_xpon_status = 0;
            #endif	
			}
			eponMpcpDscvFsmWaitHandler(llidIndex);

#if defined(TCSUPPORT_XPON_LED) && !defined(TCSUPPORT_CUC)
			ledTurnOff(LED_XPON_STATUS);		
#if !defined(TCSUPPORT_CT_PON_CN_JS)
			ledTurnOff(LED_XPON_TRYING_STATUS);
#endif
			led_xpon_status = 0;
#endif	
#ifndef TCSUPPORT_CPU_EN7521
	        gpPhyData->phy_link_status  = PHY_LINK_STATUS_LOS ;
	        gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
	        schedule_fe_reset();
            dbgtoMem(EPON_DEBUG_LEVEL_NOTIFY, "7525 receive DEREGISTER , call fe reset");
#endif
			/* set burst mode */
			eponFirmwareSetMode(0);	
		}
		
	}

	
	return 0;
}


/*  
	MACR( REGISTER_REQ, STATUS= deregister )

*/
int eponMpcpLocalDergstr(__u8 llidIndex){
	//eponLlidDscvStsReg_t llidDscvSts;
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
	REG_e_llid_dscvry_ctrl dscvCtrl;
	eponLlidDscvStsReg_t sdcvSts;

	sdcvSts.Raw = 0;


	if(mpcpDiscFsm->begin == FALSE){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMpcpLocalDergstr mpcpDiscFsm->begin == FALSE");
		return -1;
	}

	//when status = MPCP_STATE_REGISTERED
	if(mpcpDiscFsm->mpcpDiscState ==  MPCP_STATE_REGISTERED){
		if(eponMacCheckRegreqDoneBit() <0){
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpLocalDergstr done bit not ready");
			return -1;
		}
		mpcpDiscFsm->mpcpDiscState =  MPCP_STATE_LOCAL_DEREGISTER;
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "Receive Local Deregister Message , Report DEREGISTER Event");

		/* set burst mode */
		eponFirmwareSetMode(0);	
#if 0
		//set LLID_DSCVRY_STS
		sdcvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts+ llidIndex);
		sdcvSts.Bits.llidDscvrySts = 1;
		WRITE_REG_WORD(e_llid0_dscvry_sts+ llidIndex, sdcvSts.Raw);
#endif
		
		//send deregister MPCPDU		
		dscvCtrl.Raw = 0;//READ_REG_WORD(e_llid_dscvry_ctrl);
		dscvCtrl.Bits.mpcp_cmd = 2;
		dscvCtrl.Bits.rgstr_req_flg = 1;
		dscvCtrl.Bits.tx_mpcp_llid_idx = llidIndex;//////////////
		dscvCtrl.Bits.mpcp_cmd_done = 1;
		WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw);
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpLocalDergstr send dereigster dscvCtrl.Raw=0x%x", dscvCtrl.Raw);
			
		while(1){
			dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
			if(dscvCtrl.Bits.mpcp_cmd_done == 1)
			{
				break;
			}
		}
		mpcpDiscFsm->mpcpDiscState =  MPCP_STATE_REGISTERED;
		eponClearRetryCounter();
		dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "REPORT REGISTER EVENT");
		/* set continue mode */
		eponFirmwareSetMode(1);	
	}

	//when status = MPCP_STATE_REGISTER_REQUEST
	if(mpcpDiscFsm->mpcpDiscState ==  MPCP_STATE_REGISTER_REQUEST){
		mpcpDiscFsm->mpcpDiscState = MPCP_STATE_WAIT;
		eponMpcpDscvFsmWaitHandler(llidIndex);
	}
	
	return 0;
}


int eponMpcpSetDiscvRgstAck(__u8 llidIndex , __u8 rgstAckFlag){
	if((llidIndex > 8 )||(rgstAckFlag >1)){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponMpcpSetDiscvRgstAck param error");
		return -1;
	}
	
	eponDrv.eponLlidEntry[llidIndex].eponMpcp.eponDiscFsm.mpcpDiscMacr_RgstrAck = rgstAckFlag;
	return 0;
}


#if 0
 // extern int eponMacReset;
 
 // static struct task_struct *epon_mac_task;
 
 int eponMacThread(void *data){
	eponDbgPrint(EPON_DEBUG_LEVEL_DEBUG,"\r\n enter eponMacThread");
	//eponMacRestart();
	eponWanReset();
	return 0;
}
#endif

__u8 eponTimeDrftHandleFlag = XPON_ENABLE;

 void eponTimeDrftIntHandler(__u32 data){
 	__u32 Raw = 0;
	
	if(eponTimeDrftHandleFlag == XPON_ENABLE){
		Raw = READ_REG_WORD(e_time_drft_stat);
		Raw = Raw&0xff;
		if( Raw == 0xff){//by DC's comment , it's obnormal issue, do not care
			if(eponDebugLevel >= EPON_DEBUG_LEVEL_TRACE){
				eponMacDumpAllReg();
			}
			
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponTimeDrftIntHandler 255");
			
			eponDrv.timeDrftEq255Cnt++;
			//eponMacRestart();
		}else if(Raw <= EPON_TIMEDRIFT_RESET_THRSHLD){
			//increase e_grd_thrshld
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponTimeDrftIntHandler 8~16 Raw=%d", Raw);

			if(eponDebugLevel >= EPON_DEBUG_LEVEL_TRACE){
				eponMacDumpAllReg();
			}
			eponDrv.timeDrftFrom8To16Cnt++;
		}else{//EPON_TIMEDRIFT_RESET_THRSHLD~254			
			dbgtoMem(EPON_DEBUG_LEVEL_TRACE,"eponTimeDrftIntHandler error Raw=%d", Raw);

			if(eponDebugLevel >= EPON_DEBUG_LEVEL_TRACE){
				eponMacDumpAllReg();
			}
			//eponMacRestart();
			eponWanResetWithChannelRetire();
		}
	}

		//reset e_time_drft_stat
		WRITE_REG_WORD(e_time_drft_stat, 0x0);
}


void eponMpcpTmOutIntHandler(__u32 data){
	__u8 llidIndex = 0;
	__u8 llidmask = 0;
	//eponLlidDscvStsReg_t llidDscvSts;
	eponLlid_t *llidEntry_p;
	eponMpcpDiscFsm_t *mpcpDiscFsm;
	//REG_e_llid_dscvry_ctrl dscvCtrl;
	eponRptMpcpLlidReg_t eponRptMpcpLlid;
	

	//find which LLID is timeout
	
	eponRptMpcpLlid.Raw = READ_REG_WORD(e_rpt_mpcp_timeout_llid_idx);
	llidmask = eponRptMpcpLlid.Bits.mpcpTmoutLlid;


	for(llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM ; llidIndex++){
			if(llidmask&(1<<llidIndex)){
				dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpTmOutIntHandler llidIndex=%d", llidIndex);

				llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
				if(llidEntry_p->enableFlag== XPON_DISABLE){
					continue;
				}
				mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);

				if(mpcpDiscFsm->begin == TRUE){
					//TODO: MACI(REGISTER, status = deregistered)


					//return to WAIT
					mpcpDiscFsm->mpcpDiscState = MPCP_STATE_WAIT;
					//eponMpcpDscvFsmWaitHandler(llidIndex);
					atomic_inc(&mpcpTmOutCnt);
				}
			}
		}
	//clear bit map
	WRITE_REG_WORD(e_rpt_mpcp_timeout_llid_idx , (eponRptMpcpLlid.Raw&0x0000ff00));
#if 0
	//reset EPON MAC
	if(atomic_read(&mpcpTmOutCnt) >= 2){
		atomic_set(&mpcpTmOutCnt, 0);
		eponWanResetWithChannelRetire();
	}
#endif
	if(atomic_read(&mpcpTmOutCnt) <= 10){
		eponWanResetWithChannelRetire();
	}
	else if((atomic_read(&mpcpTmOutCnt) % 2) == 0){
		eponWanResetWithChannelRetire();
	}

}

extern void stop_omci_oam_monitor_not_online(void);
int eponMpcpDscvFsmWaitHandler(__u8 llidIndex){
	//eponLlidDscvStsReg_t llidDscvSts;
	eponLlid_t *llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
	eponMpcpDiscFsm_t *mpcpDiscFsm = &(llidEntry_p->eponMpcp.eponDiscFsm);
	//REG_e_llid_dscvry_ctrl dscvCtrl;
	__u32 tmp =0;
	__u32 pollingCnt = 0;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpDscvFsmWaitHandler llidIndex=%d", llidIndex);

	if(mpcpDiscFsm->begin == TRUE){
		mpcpDiscFsm->registered= FALSE;
		
#ifdef TCSUPPORT_EPON_RGST_SILENT
		if(mpcpDiscFsm->mpcpDiscState != MPCP_STATE_DENIED)
		{
#endif
			mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTERING;
#ifdef TCSUPPORT_EPON_RGST_SILENT
		}
#endif

		//change disc status to unregister
			pollingCnt = 0;
			//change disc status to unregister
			while(pollingCnt <10){
				tmp = VPint(e_llid0_dscvry_sts+llidIndex) ;
				tmp &= 0x3fffffff;
				//tmp |= 0x7fffffff;
				VPint(e_llid0_dscvry_sts+llidIndex) = tmp;
				pollingCnt++;
			}
			
		//change disc status to registing
		tmp = VPint(e_llid0_dscvry_sts+llidIndex) ;
		tmp &= 0x3fffffff;
		tmp |= 0x7fffffff;
		VPint(e_llid0_dscvry_sts+llidIndex) = tmp;
	}

    stop_omci_oam_monitor_not_online();
	return 0;
}


int eponMpcpGntOvrRunIntHandler(void){
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpGntOvrRunIntHandler");
	//if PENDING_GNT_NUM set to 64, SW can do nothing
	return 0;	
}


int eponMpcpRptOvrIntvalIntHandler(void){
	//__u32 Raw = 0;
	__u8 llidIndex = 0;
	__u8 llidmask = 0;
	//eponLlidDscvStsReg_t llidDscvSts;
	//eponLlid_t *llidEntry_p;
	//eponMpcpDiscFsm_t *mpcpDiscFsm;
	//REG_e_llid_dscvry_ctrl dscvCtrl;
	eponRptMpcpLlidReg_t eponRptMpcpLlid;

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRptOvrIntvalIntHandler");

	eponRptMpcpLlid.Raw  = READ_REG_WORD(e_rpt_mpcp_timeout_llid_idx);
	llidmask = eponRptMpcpLlid.Bits.rptOverIntvlLlid;
	
	for(llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM ; llidIndex++){
			if(llidmask&(1<<llidIndex)){
				dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "eponMpcpRptOvrIntvalIntHandler llidIndex=%d", llidIndex);
			}
		}
	//clear bit map
	WRITE_REG_WORD(e_rpt_mpcp_timeout_llid_idx , (eponRptMpcpLlid.Raw&0x000000ff));
	
	return 0;	
}

int eponGetTrafficStatus(void)
{
    __u8 llidIndex = 0;
    eponLlid_t *llidEntry_p = NULL;
	eponMpcpDiscFsm_t *mpcpDiscFsm = NULL;
    __u8 isReg = 0;

    for (llidIndex = 0;llidIndex < EPON_LLID_MAX_NUM ;llidIndex ++ ){
        if (MPCP_STATE_REGISTERED == eponDrv.eponLlidEntry[llidIndex].eponMpcp.eponDiscFsm.mpcpDiscState){
            isReg =1;
            break;
        }
    }
    
    if( (MONITOR_DONE != gpPonSysData->Omci_Oam_Monitor.run_state) || 
		( 1 != isReg)){
        return 0;
    }

    return 1;    
}

