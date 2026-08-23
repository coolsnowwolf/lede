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
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include "common/xpon_global.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "common/phy_if_wrapper.h"

#include "gpon/gpon_security.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_recovery.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
Pxgpon_mac_reg_REGS g_xgpon_mac_reg_BASE = NULL;
/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
#ifdef TCSUPPORT_AUTOBENCH
	extern char get_sninit(void);
#endif
/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**  function name
**		gpon_init_private_data
**  description:
**		init gpon private data
**  parameters:
**		xgpon_priv_p.: gpon global private data
**  global:
**		gpPonSysData.
**  return:
**		None.   
**  call:
**		spin_lock_init.    
**  revision:
**		v1.0
**____________________________________________________________________________*/
static void gpon_init_private_data(GPON_GlbPriv_T *xgpon_priv_p) 
{
    REG_DBG_CAP_SETTING		dbg_cap_setting;
	unchar sn[GPON_SN_LENS] = {0x4D, 0x54, 0x4B, 0x47, 0x00, 0x00, 0x00, 0x01} ;
	unchar defaultRegId[GPON_REG_ID_LENS] = {0x30,0x30,0x30,0x30, 0x30,0x30,0x30,0x30,  0x30,0x30,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0,} ;
	unchar defaultMsk[GPON_MSK_LENS] ={0x24,0x37,0xbe,0x54,0xe9,0x5e,0x6e,0xe3,0x53,0x8b,0xb1,0xb4,0xb5,0xd4,0x32,0xeb};	
	char cage[8] = "BoB";
	char moduleVendor[GPON_MODULE_VENDOR_LEN] = "HUMAX";
	char moduleName[GPON_MODULE_NAME_LEN] = "AA001";
	char moduleVersion[GPON_MODULE_NAME_LEN] = "v1.0";
	char moduleFwVersion[GPON_MODULE_FW_VERSION_LEN] = "v1.1.1";
	char connector[24] = "SC";
	char status[GPON_MODULE_STATUS_LEN] = "Up";
#ifdef TCSUPPORT_AUTOBENCH
	sn[7] = get_sninit();
#endif
	memset(xgpon_priv_p->gponCfg.sn, 0x0, GPON_SN_LENS) ;
	memcpy(xgpon_priv_p->gponCfg.sn, sn, GPON_SN_LENS) ;
	memset(xgpon_priv_p->gponCfg.reg_id, 0x0, GPON_REG_ID_LENS) ;
	memcpy(xgpon_priv_p->gponCfg.reg_id, defaultRegId, GPON_REG_ID_LENS) ;

	xgpon_priv_p->gponCfg.onu_id =		GPON_UNASSIGN_ONU_ID ;
	xgpon_priv_p->gponCfg.omcc = 		GPON_UNASSIGN_ONU_ID ;
	xgpon_priv_p->gponCfg.to1Timer = 	GPON_ACT_TO1_TIMER ;
	xgpon_priv_p->gponCfg.to2Timer = 	GPON_ACT_TO2_TIMER ;
	xgpon_priv_p->gponCfg.to3Timer = 	GPON_ACT_TO3_TIMER ;
	xgpon_priv_p->gponCfg.to4Timer = 	GPON_ACT_TO4_TIMER ;
	xgpon_priv_p->gponCfg.to5Timer = 	GPON_ACT_TO5_TIMER ;
	//xgpon_priv_p->gponCfg.to6Timer = 	GPON_ACT_TO6_TIMER ;  //hw monitor gnt, interrput  mon_not_gnt_int
	xgpon_priv_p->gponCfg.toZTimer = 	GPON_ACT_TOZ_TIMER ;
	xgpon_priv_p->gponCfg.hardware_timer = GPON_HARDWARE_TIMER;
    xgpon_priv_p->gponCfg.silenceInterval =  GPON_ACT_SILENCE_TIMER ;
	xgpon_priv_p->gponCfg.feChnSetDoneTimer = GPON_SET_CHN_RETIRE_DONE_TIMER;
	xgpon_priv_p->gponCfg.ploamCtrl = 	XGPON_HW ;
    xgpon_priv_p->gponCfg.usOmciMicCtrl = 	XGPON_HW ;
    xgpon_priv_p->gponCfg.dsOmciMicCtrl = 	XGPON_HW ;
	xgpon_priv_p->gponCfg.ploamMicErrDrop = XPON_DISABLE;
	xgpon_priv_p->gponCfg.txLateResyncEn = XPON_DISABLE;
	xgpon_priv_p->gponCfg.MbiTxStopEn = XPON_ENABLE;
	xgpon_priv_p->gponCfg.dyingGasp.dyingGaspCtrl = XGPON_HW;
    xgpon_priv_p->gponCfg.dyingGasp.dyingGaspNum = 255;
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
		xgpon_priv_p->gponCfg.dsFecMode	=	GPON_10G_USE_DEFAULT_SET_AS_SEPC;
	}else{
		xgpon_priv_p->gponCfg.dsFecMode	=	GPON_10G_DS_FEC_FORCE_ON;
	}
    if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)
	    xgpon_priv_p->gponCfg.onuResponseTime = XGSPON_RSP_TIME;
	else
	xgpon_priv_p->gponCfg.onuResponseTime = XGPON_RSP_TIME;
    xgpon_priv_p->gponCfg.counterType   =  GPON_10G_COUNTER_TYPE_ETHERNET ;
	xgpon_priv_p->gponCfg.ploamFilter.ploamd_filter = XPON_DISABLE;
	xgpon_priv_p->gponCfg.ploamFilter.profile_filter = XPON_DISABLE;
	if(isFPGA){
		if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			xgpon_priv_p->gponCfg.idleGemThreshold = 0x800;	
		}else{
			xgpon_priv_p->gponCfg.idleGemThreshold = 0x200;	
		}
	}else{
		if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			xgpon_priv_p->gponCfg.idleGemThreshold = 0x120;	
		}else{
			xgpon_priv_p->gponCfg.idleGemThreshold = 0x1A;
		}
	}
	xgpon_priv_p->gponSecurity.tfm = 	NULL ;
	memset(xgpon_priv_p->gponSecurity.defaultMsk, 0x0, GPON_MSK_LENS);
	memcpy(xgpon_priv_p->gponSecurity.defaultMsk, defaultMsk, GPON_MSK_LENS);
	memset(xgpon_priv_p->gponSecurity.defaultPloamIk, 0x55, GPON_PLOAM_IK_LENS);
	xgpon_priv_p->gponSecurity.ploamIkIdx = GPON_PLOAM_IK_IDX1;
	xgpon_priv_p->gponSecurity.omciIkIdx = GPON_OMCI_IK_IDX1;	
    xgpon_priv_p->gponSecurity.kekIdx = GPON_KEK_IK_IDX1;
    xgpon_priv_p->gponSecurity.smaValid = GPON_SMA_INVALID;
    xgpon_priv_p->gponSecurity.registerIDState = GPON_REG_ID_NOT_REPORT;
    xgpon_priv_p->gponSecurity.txKeyValid = 1;
    xgpon_priv_p->gemUpAESMode = UPAES_MODE_NONE; /* disable UPSTREAM AES as default value */
    spin_lock_init(&xgpon_priv_p->act_lock) ;
	spin_lock_init(&xgpon_priv_p->mac_reset_lock) ;    
	spin_lock_init(&xgpon_priv_p->cmac_test) ;
	memset(&xgpon_priv_p->gponCfg.intErrStatus, 0x0, sizeof(GPON_DEV_ERR_STATUS_T));
    memset(&gponRecovery, 0, sizeof(struct Gpon_Recovery_S));
	gpon_recovery_init();
#if defined(TCSUPPORT_CPU_AN7583)
	xgpon_priv_p->gponCfg.bwmChkCtrl =BWM_CHK_CTRL_SETTING;
	xgpon_priv_p->gponCfg.bwmLenLimit = BWM_LEN_LIMIT_SETTING;
	xgpon_priv_p->gponCfg.Wan2WanTrafficTest = XPON_DISABLE;
#endif

	memset(xgpon_priv_p->gponCfg.phyMedModule.cage, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.cage));
	memset(xgpon_priv_p->gponCfg.phyMedModule.moduleVendor, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.moduleVendor));
	memset(xgpon_priv_p->gponCfg.phyMedModule.moduleName, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.moduleName));
	memset(xgpon_priv_p->gponCfg.phyMedModule.moduleVersion, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.moduleVersion));
	memset(xgpon_priv_p->gponCfg.phyMedModule.moduleFwVersion, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.moduleFwVersion));
	memset(xgpon_priv_p->gponCfg.phyMedModule.connector, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.connector));
	memset(xgpon_priv_p->gponCfg.phyMedModule.status, 0, sizeof(xgpon_priv_p->gponCfg.phyMedModule.status));
	//NG2
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.cage, cage, 8);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.moduleVendor, moduleVendor, GPON_MODULE_VENDOR_LEN);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.moduleName, moduleName, GPON_MODULE_NAME_LEN);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.moduleVersion, moduleVersion, GPON_MODULE_VERSION_LEN);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.moduleFwVersion, moduleFwVersion, GPON_MODULE_FW_VERSION_LEN);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.connector, connector, 24);
	memcpy(xgpon_priv_p->gponCfg.phyMedModule.status, status, GPON_MODULE_STATUS_LEN);
	ng2_system_config_reset();
	ng2_mon_not_gnt = 1;
	
	xgpon_priv_p->gponAlmKeepTime = JIFFIES_TIMEOUT_HZ_CNT;

    dbg_cap_setting.Raw = IO_GREG(DBG_CAP_SETTING);
	dbg_cap_setting.Bits.o52_idle_only_en = 1;
	IO_SREG(DBG_CAP_SETTING, dbg_cap_setting.Raw) ;
}
/*______________________________________________________________________________
**  function name
**		gpon_init
**  description:
**		gpon device init
**  parameters:
**		none
**  global:
**		gpPonSysData.
**		g_xgpon_mac_reg_BASE
**  return:
**		0:  ok
**		others: failure.  
**  call:
**		gpon_init_private_data.    
**		gponDevSetRspTime.    
**		XPON_PHY_SET_MODE.    
**		XPON_PHY_SET_MODE.    
**		ploam_init.    
**		gpon_act_init.    
**		gpon_dev_init.   
**		gpon_security_init.   
**		gponDevResetCtrl.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
int gpon_init(void)
{
	int ret = 0 ;

    if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGPON)&&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGSPON) &&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)  &&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G)  &&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G)){
        PON_MSG(MSG_ERR, " No GPON init, pon mode:%d \n",gpPonSysData->sysPonMode) ;
        return ret;
    }

#ifdef TCSUPPORT_CPU_ARMV8_64
	/* Base Register of  XGPON */
	g_xgpon_mac_reg_BASE = 0x0;
#else
	g_xgpon_mac_reg_BASE = (Pxgpon_mac_reg_REGS)(ioremap_nocache(CONFIG_XGPON_BASE_ADDR, CONFIG_XGPON_REG_RANGE));

	PON_MSG(MSG_TRACE, " xgpon_mac_reg_BASE: %08x \n",(__u32)g_xgpon_mac_reg_BASE) ;

	if(!g_xgpon_mac_reg_BASE) {
		PON_MSG(MSG_ERR, "ioremap the XGPON base address failed.\n") ;
		return -ENOMEM ;
	}
#endif

    gpon_init_private_data(gpGponPriv) ;
	gponDevSetRspTime(gpGponPriv->gponCfg.onuResponseTime);
	XPON_PHY_SET_MODE(PHY_GPON_CONFIG);
	gponDevSetPloamFilterMode(&gpGponPriv->gponCfg.ploamFilter);
	ploam_init() ;
	gpon_act_init() ;
	gponDevMpiStop(XPON_RESET_HOLD_ON) ;
	gpon_dev_init() ;
	gpon_INT_init() ;
	

	ret = gpon_security_init() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "XGPON security initialization failed.\n") ;
		return ret ;
	}	
    gponDevResetCtrl(XPON_RESET_HOLD_ON);
	
	return ret ;
}
/*______________________________________________________________________________
**  function name
**		gpon_deinit
**  description:
**		deinit gpon related device
**  parameters:
**		none
**  global:
**		none.
**		g_xgpon_mac_reg_BASE
**  return:
**		none
**  call:
**		gpon_init_private_data.    
**		gponDevSetRspTime.    
**		XPON_PHY_SET_MODE.    
**		XPON_PHY_SET_MODE.    
**		ploam_init.    
**		gpon_act_init.    
**		gpon_dev_init.   
**		gpon_security_init.   
**		gponDevResetCtrl.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_deinit(void)
{
	printk("==NGPON2== xpondrv_cleanup, %d \n",__LINE__);

	if(g_xgpon_mac_reg_BASE) {
		iounmap(g_xgpon_mac_reg_BASE) ;
		g_xgpon_mac_reg_BASE = NULL ;
	}	
    ploam_deinit();
	gpon_act_deinit();
    gpon_INT_deinit();
	gpon_security_exit();
}

