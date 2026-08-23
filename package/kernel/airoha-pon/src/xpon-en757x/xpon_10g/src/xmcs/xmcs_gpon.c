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
#include <linux/uaccess.h>
#include "common/xpon_global.h"
#include "xmcs/xmcs_gpon.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_security.h"
#include "gpon/gpon_recovery.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

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

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
int distanceAdjust = 0;

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
extern void XGPON_MAC_EVENT_HANDLER(PON_PHY_Event_data_t * pEvent);

/***************************************************************
***************************************************************/
static int xmcs_get_activation_counter(unsigned int *counter)
{
	*counter = gpGponPriv->activationCnt;
	return 0;
}
static int xmcs_get_ploamMsg_counter(GPON_PLOAM_CNT_T* counter)
{
	*counter= gpGponPriv->ploamMsgcounter;
	return 0;
}
static int xmcs_get_omciRX_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_cnt;
	return 0;
}
static int xmcs_get_omciRX_Base_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_cnt \
                            - gpWanPriv->gpon.rx_omci_extend_cnt;
	return 0;
}

static int xmcs_get_omciRX_Extend_counter(GPON_OMCI_CNT_T *counter)
{
	counter->rxOmciMsgCnt = gpWanPriv->gpon.rx_omci_extend_cnt ;
	return 0;
}

static int xmcs_get_gponAlarm(GPON_Alarm_T * alarmInfo)
{
	*alarmInfo = gpGponPriv->gponAlarm;
	return 0;
}

static int xmcs_set_act_timer(struct XMCS_XgponActTimer_S *pActTimer) 
{
    struct XMCS_XgponActTimer_S actTimerTmp;

    if(copy_from_user(&actTimerTmp, pActTimer, sizeof(struct XMCS_XgponActTimer_S))){
        printk("[%s %d]ERR: copy_from_user\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }
    
	gpGponPriv->gponCfg.to1Timer = actTimerTmp.to1Timer ;
	gpGponPriv->to1_timer.expires = gpGponPriv->gponCfg.to1Timer ;

	gpGponPriv->gponCfg.to2Timer = actTimerTmp.to2Timer ;
	gpGponPriv->to2_timer.expires = gpGponPriv->gponCfg.to2Timer ;

	gpGponPriv->gponCfg.to3Timer = actTimerTmp.to3Timer ;
	gpGponPriv->to3_timer.expires = gpGponPriv->gponCfg.to3Timer ;

	gpGponPriv->gponCfg.to4Timer = actTimerTmp.to4Timer ;
	gpGponPriv->to4_timer.expires = gpGponPriv->gponCfg.to4Timer ;

	gpGponPriv->gponCfg.to5Timer = actTimerTmp.to5Timer ;
	gpGponPriv->to5_timer.expires = gpGponPriv->gponCfg.to5Timer ;

	gpGponPriv->gponCfg.toZTimer = actTimerTmp.toZTimer ;
	gpGponPriv->toZ_timer.expires = gpGponPriv->gponCfg.toZTimer ;

	return 0 ;
}
/***************************************************************
***************************************************************/
static int xmcs_set_sn_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{

    if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGPON)&&\
        (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGSPON) && \
		(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G) && \
		(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G) && \
        (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G))
    {
        return -1;
    }

	memset(gpGponPriv->gponCfg.sn, 0, GPON_SN_LENS) ;
	memcpy(gpGponPriv->gponCfg.sn, pSnPasswd->sn, GPON_SN_LENS) ;
	gponDevSetSerialNumber(pSnPasswd->sn) ;	

	memset(gpGponPriv->gponCfg.reg_id, 0, GPON_REG_ID_LENS) ;
	memcpy(gpGponPriv->gponCfg.reg_id, pSnPasswd->regid, GPON_REG_ID_LENS) ;
	gponDevSetRegId(gpGponPriv->gponCfg.reg_id);
	PON_MSG(MSG_TRACE, "SN:%s, PASSWD:%s\n", pSnPasswd->sn, pSnPasswd->passwd) ;
	
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
	
static int xmcs_set_sn(char *sn)
{
	memcpy((void*)gpGponPriv->gponCfg.sn, (void*)sn, GPON_SN_LENS);
	gponDevSetSerialNumber(sn) ;
	
	return 0 ;
}

static int xmcs_set_passwd(char *passwd) 
{

	memset(gpGponPriv->gponCfg.reg_id, 0, GPON_REG_ID_LENS) ;
	memcpy(gpGponPriv->gponCfg.reg_id, (void*)passwd, GPON_REG_ID_LENS) ;
	gponDevSetRegId(gpGponPriv->gponCfg.reg_id);

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
static int xmcs_set_emergency_state(XPON_Mode_t mode) 
{
	gpGponPriv->emergencyState = mode;
	if(XPON_ENABLE == mode) 
	{
		gpon_act_change_state(GPON_STATE_O7);
	}
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
static int xmcs_set_msk(struct XGMCS_XgponMsk_S *pMsk) 
{	
	/* Compare the msk */
	if(memcmp(gpGponPriv->gponSecurity.msk, pMsk->msk, GPON_MSK_LENS) == 0) {
		PON_MSG((MSG_SECUR), "PLOAM: The msk is same\n") ;
		return 0 ;
	}
	memset(gpGponPriv->gponSecurity.msk, 0, GPON_MSK_LENS) ;
	memcpy(gpGponPriv->gponSecurity.msk, pMsk->msk, GPON_MSK_LENS) ;
	
	if(gponDevKeySetOmciBasedMSK(&gpGponPriv->gponSecurity)!=0){
		return -1;
	}
	gpon_omciIk_index_change_by_OMCI_base_secure(&gpGponPriv->gponSecurity);
	gpGponPriv->gponSecurity.smaValid = GPON_SMA_VALID;
	
	return 0 ;
}
/*****************************************************************************
******************************************************************************/
static int xmcs_set_broadcast_key(struct XGMCS_XgponBroadcast_Key_S *pBroadcasetKey) 
{	
	uint i = 0;
	
	printk("ioctl set broadcast keyindex: %d FragmentNum:%d FragmentIndex:%d\n key:",pBroadcasetKey->keyIndex,pBroadcasetKey->keyFragmentNum,pBroadcasetKey->keyFragmentIndex);
	for(i=0;i<16;i++){
		printk("%x ",pBroadcasetKey->key[i]);
	}
	
	printk("\n");
	
	return 0 ;
}
/***************************************************************
***************************************************************/
static int xmcs_set_o23_o4_ploam_ctrl(XGPON_SW_HW_SELECT_T mode)
{
	gpGponPriv->gponCfg.ploamCtrl = mode ;
	gponDevSetO23O4PloamCtrl(mode);
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_o23_o4_ploam_ctrl(XGPON_SW_HW_SELECT_T *mode)
{
	gponDevGetO23O4PloamCtrl(mode);
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_set_eqd_offset_flag(char flag) 
{
	gpGponPriv->gponCfg.flags.eqdOffsetFlag = (flag == '-') ? XGPON_EQD_OFFSET_FLAG_SUBTRACT : XGPON_EQD_OFFSET_FLAG_ADD ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_eqd_offset_o4(unchar offset) 
{
	gpGponPriv->gponCfg.eqdO4Offset = offset;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_eqd_offset_o5(unchar offset) 
{
	gpGponPriv->gponCfg.eqdO5Offset = offset;
	
	if(gpGponPriv->gponCfg.eqdO5Offset == 0) {
		return 0 ;
	} else {
		/* For O5 EqD offset test */
		if(gpGponPriv->gponCfg.flags.eqdOffsetFlag == XGPON_EQD_OFFSET_FLAG_SUBTRACT) {
			gpGponPriv->gponCfg.eqd = gpGponPriv->gponCfg.eqd - gpGponPriv->gponCfg.eqdO5Offset;
		} else {
			gpGponPriv->gponCfg.eqd = gpGponPriv->gponCfg.eqd + gpGponPriv->gponCfg.eqdO5Offset;
		}
	}
	gponDevSetEqd(gpGponPriv->gponCfg.eqd);
    PON_MSG(MSG_EQD, "PLOAM: Adjust the main path EqD in O5, Offset:%x.\n", gpGponPriv->gponCfg.eqdO5Offset) ;
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_eqd_offset(struct XGMCS_EqdOffset_S *pXgponEqdOffset) 
{
	pXgponEqdOffset->O4 = gpGponPriv->gponCfg.eqdO4Offset;
	pXgponEqdOffset->O5 = gpGponPriv->gponCfg.eqdO5Offset;
	pXgponEqdOffset->eqdOffsetFlag = gpGponPriv->gponCfg.flags.eqdOffsetFlag;
	
	return 0 ;
}
/***************************************************************
***************************************************************/
static int xmcs_get_onu_info(struct XGMCS_XgponOnuInfo_S *pOnuInfo) 
{
	memset(pOnuInfo, 0x0, sizeof(struct XGMCS_XgponOnuInfo_S)) ;
	
	pOnuInfo->onuId = GPON_ONU_ID ;
	pOnuInfo->state = GPON_CURR_STATE ;
	memcpy(&pOnuInfo->sn, gpGponPriv->gponCfg.sn, GPON_SN_LENS) ;
	memcpy(&pOnuInfo->regid, gpGponPriv->gponCfg.reg_id, GPON_REG_ID_LENS) ;
	pOnuInfo->actTo1Timer = gpGponPriv->gponCfg.to1Timer ;
	pOnuInfo->actTo2Timer = gpGponPriv->gponCfg.to2Timer ;
	pOnuInfo->omcc = GPON_OMCC_ID ;
	pOnuInfo->EmergencyState = gpGponPriv->emergencyState ;
	pOnuInfo->onuRespTime= gpGponPriv->gponCfg.onuResponseTime ;
	
	return 0 ;
}
/***************************************************************
***************************************************************/
static int xmcs_gpon_get_onu_info(struct XMCS_GponOnuInfo_S *pOnuInfo) 
{
	memset(pOnuInfo, 0, sizeof(struct XMCS_GponOnuInfo_S)) ;
	
	pOnuInfo->onuId = GPON_ONU_ID ;
	pOnuInfo->state = GPON_CURR_STATE ;
	memcpy(&pOnuInfo->sn, gpGponPriv->gponCfg.sn, GPON_SN_LENS) ;
	memcpy(&pOnuInfo->passwd, gpGponPriv->gponCfg.passwd, GPON_PASSWD_LENS) ;
    pOnuInfo->PasswdLength = gpGponPriv->gponCfg.passwdLength;
    pOnuInfo->hexFlag = gpGponPriv->gponCfg.hexFlag;
	pOnuInfo->actTo1Timer = gpGponPriv->gponCfg.to1Timer ;
	pOnuInfo->actTo2Timer = gpGponPriv->gponCfg.to2Timer ;
	pOnuInfo->omcc = GPON_OMCC_ID ;
	pOnuInfo->EmergencyState = gpGponPriv->emergencyState ;
	
	return 0 ;
}
/***************************************************************
***************************************************************/
static int xmcs_set_sniffer_mode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer) 
{
	memcpy(&gpGponPriv->gponCfg.sniffer_mode, sniffer, sizeof(GPON_10G_DEV_SNIFFER_MODE_T));
	return gponDevSetSniffMode(sniffer);
}
/***************************************************************
***************************************************************/
static int xmcs_get_sniffer_mode(GPON_10G_DEV_SNIFFER_MODE_T *sniffer) 
{
	gponDevGetSniffMode(sniffer);
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_set_ploam_filter_mode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter) 
{
	return gponDevSetPloamFilterMode(filter);
}
/***************************************************************
***************************************************************/
static int xmcs_get_ploam_filter_mode(GPON_10G_DEV_PLOAMD_FILTER_MODE_T *filter) 
{
	gponDevGetPloamFilterMode(filter);
	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_set_ploam_micerr_drop_mode(XPON_Mode_t mode) 
{
	gpGponPriv->gponCfg.ploamMicErrDrop = mode;	
	return gponDevSetErrMicPloamDrop(gpGponPriv->gponCfg.ploamMicErrDrop);
}

/***************************************************************
***************************************************************/
static int xmcs_get_ploam_micerr_drop_mode(XPON_Mode_t *mode) 
{
	gponDevGetErrMicPloamDrop(mode);
	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_set_sw_dying_gasp_mode(GPON_10G_DYING_GASP_MODE_T *dyingGasp) 
{

	gpGponPriv->gponCfg.dyingGasp.dyingGaspCtrl = dyingGasp->dyingGaspCtrl;
	gpGponPriv->gponCfg.dyingGasp.dyingGaspNum = dyingGasp->dyingGaspNum;

    return gponDevSetDyingGaspMode(&gpGponPriv->gponCfg.dyingGasp);
}
/***************************************************************
***************************************************************/
static int xmcs_get_sw_dying_gasp_mode(GPON_10G_DYING_GASP_MODE_T *dyingGasp) 
{
	return gponDevGetDyingGaspMode(dyingGasp);
}
/***************************************************************
***************************************************************/
int xmcs_set_phy_mode(XGMCSIF_PhyMode_t mode)
{
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ};
	
	PON_MSG(MSG_DBG, "xmcs_set_phy_mode :%d\n",mode) ;
	if(mode ==XGMCS_IF_PHY_READY){
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		XGPON_MAC_EVENT_HANDLER(&phy_event);
	}
	else if(mode ==XGMCS_IF_PHY_LOSS){
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		XGPON_MAC_EVENT_HANDLER(&phy_event);
	}

	return 0 ;
}
/***************************************************************
***************************************************************/
int xmcs_set_ack_num(unchar ackNum)
{
    unchar i=0;
	
    for(i=0; i<ackNum; i++){
        ploam_send_acknowledge_msg(0,XGPON_PLOAM_ACK_OK) ; /*send Ack in unicast */
    }
	return 0 ;
}
/***************************************************************
***************************************************************/
int xmcs_set_dba_backdoor(GPON_10G_DEV_DBA_BACKDOOR_T *dba)
{
    return(gponDevSetDbaBackdoorMode(dba));    
}
/***************************************************************
***************************************************************/
int xmcs_get_dba_backdoor(GPON_10G_DEV_DBA_BACKDOOR_T *dba)
{
    unsigned int dbaMode = 0;
    unsigned int modifyValue = 0;
    
    gponDevGetDbaBackdoorMode(&dbaMode,&modifyValue);
    dba->mode = dbaMode;
    if(dbaMode == GPON_DBA_BACKDOOR_FIX_MODE){
        dba->value = modifyValue;
    }else if(dbaMode == GPON_DBA_BACKDOOR_ADD_MODE){
        dba->value = modifyValue & 0x7fffffff;
        dba->addMode = ((modifyValue & 0x80000000)==0) ? GPON_DBA_BACKDOOR_ADD : GPON_DBA_BACKDOOR_MINUS;
    }else if(dbaMode == GPON_DBA_BACKDOOR_SHIFT_MODE){
        dba->value = modifyValue & 0x7fffffff;
        dba->shiftMode = ((modifyValue & 0x80000000) ==0) ? GPON_DBA_BACKDOOR_LEFT_SHIFT : GPON_DBA_BACKDOOR_RIGHT_SHIFT;
    }else{
        return -EINVAL;
    }

	return 0 ;
}
/***************************************************************
***************************************************************/
#if defined(TCSUPPORT_CPU_AN7583)
int xmcs_set_dba_backdoor_seperate(GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T *dba)
{
    return(gponDevSetDbaBackdoorModeSeperate(dba));    
}
#endif
/***************************************************************
***************************************************************/
int xmcs_set_omci_mic_ctrl(GPON_10G_DEV_OMCI_MIC_CTRL_T *omciMicCtrl)
{
    gpGponPriv->gponCfg.dsOmciMicCtrl = omciMicCtrl->dsOmciMicMode;    
    gpGponPriv->gponCfg.usOmciMicCtrl = omciMicCtrl->usOmciMicMode;
    gponDevSetDownstreamOmciMicCtrl(omciMicCtrl->dsOmciMicMode);
    gponDevSetUpstreamOmciMicCtrl(omciMicCtrl->usOmciMicMode);
    
    return 0 ;    
}
/***************************************************************
***************************************************************/
int xmcs_get_omci_mic_ctrl(GPON_10G_DEV_OMCI_MIC_CTRL_T *omciMicCtrl)
{
    gponDevGetOmciMicCtrl(omciMicCtrl);
    return 0 ;    
}

/***************************************************************
***************************************************************/
int xmcs_set_omci_broadcast_key(struct XMCS_OMCI_BROADCAST_KEY_S * pOmciBroadCastKey)
{
    __u8 *fragment = NULL;
	uint ret = 0;
 	unchar broadcastKey[16]={0};	
    unchar kekIndex =0;

	GPON_BC_KEY_CTRL_T keyContrl;
	
	keyContrl.raw = pOmciBroadCastKey->key_index;
    fragment = pOmciBroadCastKey->fragment;
	
    kekIndex = gpGponPriv->gponSecurity.kekIdx;
	ret = gpon_aes_ecb_decrypt(gpGponPriv->gponSecurity.aesEcbTfm,gpGponPriv->gponSecurity.kek[kekIndex],
		fragment,GPON_DATA_ENCRYPT_KEY_LENS,broadcastKey);
	if(ret != 0){
		PON_MSG((MSG_ERR), "PLOAM: decrypt broadcast key by AES_ECB kekindex %d failed, %d\n",kekIndex,ret) ;
		return -1;
	}
	
    if(1 == keyContrl.bit.keyIndex)
    {
        gponDevSetAesBcKey0(broadcastKey);
        gponDevSetAesRxKeyValid(GPON_AES_BC_FIRST_KEY);
    }else if(2 == keyContrl.bit.keyIndex)
    {
        gponDevSetAesBcKey1(broadcastKey);
        gponDevSetAesRxKeyValid(GPON_AES_BC_SECOND_KEY);
    }else
    {
        printk("[%s %d]ERROR: keyIndex error.\n", __FUNCTION__, __LINE__);
    }
    
    return 0;
}

/***************************************************************
***************************************************************/
int xmcs_clean_omci_broadcast_key(struct XMCS_OMCI_BROADCAST_KEY_S * pOmciBroadCastKey)
{
    union{
        struct{
            __u8 fragLen    :4;
            __u8 reserved   :2;
            __u8 opr        :2;
        }bit;
        __u8 raw;
    }rowControl;
    
    union{
        struct{
            __u8 keyIndex   :2;
            __u8 reserved   :2;
            __u8 fragNum    :4;
        }bit;
        __u8 raw;
    }rowIdentifier;
    
    rowControl.raw = pOmciBroadCastKey->row_ctl;
    rowIdentifier.raw = pOmciBroadCastKey->key_index;

    if(1 == rowControl.bit.opr)
    {
        if(0 == rowIdentifier.bit.keyIndex)
        {
            gponDevCleanAesBcKey0();
            gponDevSetAesRxKeyInvalid(GPON_AES_BC_FIRST_KEY);
        }else if(1 == rowIdentifier.bit.keyIndex)
        {
            gponDevCleanAesBcKey1();
            gponDevSetAesRxKeyInvalid(GPON_AES_BC_SECOND_KEY);
        }else
        {
            printk("[%s %d]ERROR: key index error.\n", __FUNCTION__, __LINE__);
        }
    }else if(2 == rowControl.bit.opr)
    {
        gponDevCleanAesBcKey0();
        gponDevSetAesRxKeyInvalid(GPON_AES_BC_FIRST_KEY);
        gponDevCleanAesBcKey1();
        gponDevSetAesRxKeyInvalid(GPON_AES_BC_SECOND_KEY);
    }else
    {
        printk("[%s %d]ERROR: operate error.\n", __FUNCTION__, __LINE__);
    }
    
    return 0;
}
/*_____________________________________________________________________________
**      function name: get_counter_from_reg
**      descriptions:
**           It's used to get gem counter from reg .
**      parameters:
**            see XMCS_GponGemCounter_S.
**      global:
**             None
**      return:
**             success: 0
**      call:
**   	         gponDevGetGemPortCounter
**      revision:
**            None
**____________________________________________________________________________
*/
int get_counter_from_reg(struct XMCS_CounterCfg_S *pGemCouter)
{    
    ushort gemIdx = 0;
    GWAN_GemInfo_T * gemInfo = NULL;
    ushort portid = 0;

    if(pGemCouter == NULL)
        return -1;

    gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCouter->gemPortId] & GPON_GEM_IDX_MASK);

    gemInfo = & gpWanPriv->gpon.gemPort[gemIdx].info;
    
    if(!gemInfo->valid)
        return -1;

    portid = gemInfo->portId;
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_FRAME_CNT,   &pGemCouter->sta.rxGemFrame) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_PL_BYTE_CNT, &pGemCouter->sta.rxGemPayload) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_FRAME_CNT,   &pGemCouter->sta.txGemFrame) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_PL_BYTE_CNT, &pGemCouter->sta.txGemPayload) )
        return -1;

  	return 0;
}
/*_____________________________________________________________________________
**      function name: xmcs_get_gem_counter
**      descriptions:
**           It's used to get gem counter from reg .
**      parameters:
**            see XMCS_GponGemCounter_S.
**      global:
**             None
**      return:
**             success: 0
**             fail: EINVAL
**      call:
**   	         get_counter_from_reg
**      revision:
**            None
**____________________________________________________________________________
*/
int xmcs_get_gem_counter(struct XMCS_CounterCfg_S *pGponGemPortCount) 
{
	int ret = 0;
	
	ret = get_counter_from_reg(pGponGemPortCount);
	if(0!=ret)
	{
		return -EINVAL;
	}

	return ret ;
}

/***************************************************************
***************************************************************/
int xmcs_get_tcont_counter(STATISTIC_CFG_t *pGponTcontCount) 
{
    STATISTIC_CFG_t gemTemp = {0};
    int i = 0;
    struct XMCS_GemPortInfo_S *gemInfo = NULL ;
     
	gemInfo = kzalloc(sizeof(struct XMCS_GemPortInfo_S), GFP_ATOMIC);
	if(gemInfo == NULL)
	{
	    return -EINVAL ;
	}
	if(xmcs_get_gem_port_info(gemInfo) < 0) {
		printk("xmcs_get_gem_port_info exec failed") ;
	} else {
		for(i=0 ; i<gemInfo->entryNum ; i++) {
			if(pGponTcontCount->allocId == gemInfo->info[i].allocId)
			{
			    memset(&gemTemp, 0, sizeof(STATISTIC_CFG_t));
			    gemTemp.gemPortId = gemInfo->info[i].gemPortId;
			    get_counter_from_reg(&gemTemp);
                pGponTcontCount->sta.rxGemFrame += gemTemp.sta.rxGemFrame;
                pGponTcontCount->sta.rxGemPayload += gemTemp.sta.rxGemPayload;
                pGponTcontCount->sta.txGemFrame += gemTemp.sta.txGemFrame;
                pGponTcontCount->sta.txGemPayload += gemTemp.sta.txGemPayload;
			}
		}
	}
	if(gemInfo != NULL)
	{
	    kfree(gemInfo);
	}
    return 0 ;
}

/*_____________________________________________________________________________
**      function name: xmcs_set_counter_type
**      descriptions:
**           It's used to set the counter type to get.
**      parameters:
**            see GPON_10G_COUNTER_TYPE_t.
**      global:
**             None
**      return:
**             success: 0
**      call:
**   	         gponDevSetMibCounterType
**      revision:
**            None
**____________________________________________________________________________
*/
static int xmcs_set_counter_type(GPON_10G_COUNTER_TYPE_t type) 
{
	gpGponPriv->gponCfg.counterType = type;
	gponDevSetMibCounterType(type);
	
	return 0 ;
}
/*_____________________________________________________________________________
**      function name: xmcs_get_counter_type
**      descriptions:
**           It's used to get the counter type to get.
**      parameters:
**            see GPON_10G_COUNTER_TYPE_t.
**      global:
**             None
**      return:
**             success: 0
**      call:
**   	         gponDevGetMibCounterType
**      revision:
**            None
**____________________________________________________________________________
*/
static int xmcs_get_counter_type(GPON_10G_COUNTER_TYPE_t *type) 
{
	gponDevGetMibCounterType(type);
	
	return 0 ;
}
/***************************************************************
**      function name: xmcs_set_sw_reset
**      descriptions:
**           It's used to start the reset  by sw
**      parameters:
**            None
**      global:
**             None
**      return:
**             success: 0
**      call:
**   	         gpon_dvt_sw_resync
**      revision:
** 
***************************************************************/
static int xmcs_set_sw_reset(void)
{
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_gpon_traffic_state(uint *arg)
{
    if( GPON_TRAFFIC_UP == gwan_get_traffic_status() )
	{
		put_user(0,arg); /*upstream traffic up*/
	return 0;
}
	else
	{
		put_user(1,arg);
		return 0;
	}
}

/*_____________________________________________________________________________
**      function name: xmcs_get_xgpon_tc_counter
**      descriptions:
**           It's used to get xg-pon TC counter from reg .
**      parameters:
**            see GPON_10G_TC_COUNTER_T.
**      global:
**             None
**      return:
**             success: 0
**             fail: EINVAL
**      call:
**   	         get_counter_from_reg
**      revision:
**            None
**____________________________________________________________________________
*/
static int xmcs_get_xgpon_tc_counter(GPON_10G_TC_COUNTER_T *pXgponTcCounter) 
{
	int ret = 0;
	GPON_10G_TC_COUNTER_T pXgponTcCounterTemp = {0} ;
	
    if(gponDevGetTCCounter(&pXgponTcCounterTemp)!=0){
        return -EINVAL ;
    } 
	
	if ( 0 != copy_to_user(pXgponTcCounter, &pXgponTcCounterTemp, sizeof(pXgponTcCounterTemp ) ) ) {
		return -EFAULT;
	}

	return ret ;	
}

static int xmcs_get_xgpon_ds_mgnt_counter(GPON_10G_DS_MGNT_COUNTER_T *pXgponDsMgntCounter) 
{
	int ret = 0;
	GPON_10G_DS_MGNT_COUNTER_T pXgponDsMgntCounterTemp = {0} ;
	
    	if(gponDevGetDsMgntCounter(&pXgponDsMgntCounterTemp)!=0){
        	return -EINVAL ;
	} 
	
	if ( 0 != copy_to_user(pXgponDsMgntCounter, &pXgponDsMgntCounterTemp, sizeof(pXgponDsMgntCounterTemp ) ) ) {
		return -EFAULT;
	}

	return ret ;	
}

static int xmcs_get_xgpon_us_mgnt_counter(GPON_10G_US_MGNT_COUNTER_T *pXgponUsMgntCounter) 
{
	int ret = 0;
	GPON_10G_US_MGNT_COUNTER_T pXgponUsMgntCounterTemp = {0} ;
	
	if(gponDevGetUsMgntCounter(&pXgponUsMgntCounterTemp)!=0){
		return -EINVAL ;
	} 
	
	if ( 0 != copy_to_user(pXgponUsMgntCounter, &pXgponUsMgntCounterTemp, sizeof(pXgponUsMgntCounterTemp ) ) ) {
		return -EFAULT;
	}

	return ret ;	
}


/*_____________________________________________________________________________
**      function name: xmcs_get_gpon_tcont_info
**      descriptions:
**           It's used to get tcont index.
**      parameters:
**            see XMCS_GponTcontInfo_S.
**      global:
**             None
**      return:
**             success: 0
**             fail: EINVAL
**      call:
**   	         gponDevGetTcontInfo
**      revision:
**            None
**____________________________________________________________________________
*/
static int xmcs_get_gpon_tcont_info(struct XMCS_GponTcontInfo_S * pTcontInfo)
{
	int ret = 0;
	struct XMCS_GponTcontInfo_S temp = {0};
	ret = copy_from_user(&temp, pTcontInfo, sizeof(struct XMCS_GponTcontInfo_S));
	if(0 != ret)
	{
		return -EINVAL ;
	}
	ret = gponDevGetTcontInfo(temp.allocId,&temp.tcontIdx);
	if(0 != ret)
    {
		return -EINVAL ;
    }
	ret = copy_to_user(pTcontInfo, &temp, sizeof(struct XMCS_GponTcontInfo_S));
	if(0 != ret)
	{
		return -EINVAL ;
	}
	return ret ;
}

/***************************************************************
***************************************************************/
#ifdef TCSUPPORT_CPU_EN7581
static int xmcs_set_gpon_tod(struct XMCS_GponTodCfg_S *pGponTod) 
{
	gponDevSetNewTod(*pGponTod) ;
	return 0 ;
}
#endif

static int xmcs_set_mib_flag(uint arg) 
{
	gponRecovery.mibFlag = arg;
	if(gponRecovery.mibFlag){		
		stop_omci_oam_monitor();
		gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;		
		start_omci_oam_monitor();
	}
	return 0 ;
}

static int xmcs_set_xgpon_us_aes_mode(UPAES_Mode_t mode) 
{
    if((XMCS_IF_WAN_DETECT_MODE_XGPON != gpPonSysData->sysPonMode) 
        && (XMCS_IF_WAN_DETECT_MODE_XGSPON  != gpPonSysData->sysPonMode))
    {
        return 0;
    }

    if(mode >= UPAES_MODE_MAX)
        mode = UPAES_MODE_NONE;
		
    if( mode != gpGponPriv->gemUpAESMode){
	    gpGponPriv->gemUpAESMode = mode;
        gponDevSetUpAesMode();
    }
	return 0 ;
}

int xmcs_get_olt_distance(uint32_t * oltDistance)
{
    REG_EQD gponEqd;
    uint32_t eqd = 0;
    uint32_t distance = 0;
	int hw5800_base = 0;
	int mode_rate = 0;

    gponEqd.Raw = IO_GREG(EQD);
    eqd = gponEqd.Bits.eqd;
	if(oltDistance == NULL){
		return -1;
	}
    /** FD = (TEqd - EqD)*102  TEqd = 250us ; Eqd = regVal/mode_rate
     *  (250-regVal/mode_rate) *102 , up_xgspon_rate=9953, up_xgpon_rate=2488
     */
    PON_MSG(MSG_XMCS,"eqd=%#x(%d),response time=%#x(%d)\n",
    		eqd,eqd,gpGponPriv->gponCfg.onuResponseTime,gpGponPriv->gponCfg.onuResponseTime);
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON 
		|| gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)
	{
		mode_rate = 9953;
	}else{
		mode_rate = 2488;
	}
	  		
    distance = 250* 102 - (eqd+gpGponPriv->gponCfg.onuResponseTime)* 102/mode_rate ;
	if(distance < 1000){
		hw5800_base = 0;
	}else{
		hw5800_base = -(1+(distance-1000)/2000);
	}	
	PON_MSG(MSG_XMCS,"auto adjust by hw5800 base = %d\n", hw5800_base);
    *oltDistance = distance + hw5800_base + distanceAdjust;
   	PON_MSG(MSG_XMCS,"onu cal olt distance = %u\n", *oltDistance);
    return 0;
}

/***************************************************************
***************************************************************/
int gpon_10g_cmd_proc(uint cmd, ulong arg) 
{
	int ret = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
		cmd = cmd & IOCTL_CMD;
#endif

	switch(cmd) {
		case GPON_10G_IOS_SN :
			{
				struct XMCS_XgponSn_S temp;
				memset(&temp, 0, sizeof(struct XMCS_XgponSn_S));
				COPY_FROM_USER(&temp, (struct XMCS_XgponSn_S *)arg, sizeof(struct XMCS_XgponSn_S), ret);
				ret = xmcs_set_sn(temp.sn);
			}
			break ;	
		case GPON_10G_IOS_ACT_TIMER :
			{
				struct XMCS_XgponActTimer_S temp;
				memset(&temp, 0, sizeof(struct XMCS_XgponActTimer_S));
				COPY_FROM_USER(&temp, (struct XMCS_XgponActTimer_S *)arg, sizeof(struct XMCS_XgponActTimer_S), ret);
				ret = xmcs_set_act_timer(&temp);
			}
			break ;
		case GPON_10G_IOS_MSK :
			{
				struct XGMCS_XgponMsk_S temp;
				memset(&temp, 0, sizeof(struct XGMCS_XgponMsk_S));
				COPY_FROM_USER(&temp, (struct XGMCS_XgponMsk_S *)arg, sizeof(struct XGMCS_XgponMsk_S), ret);
				ret = xmcs_set_msk(&temp);
			}
			break ;
		case GPON_10G_IOS_BROADCAST_KEY :
			{
				struct XGMCS_XgponBroadcast_Key_S temp;
				memset(&temp, 0, sizeof(struct XGMCS_XgponBroadcast_Key_S));
				COPY_FROM_USER(&temp, (struct XGMCS_XgponBroadcast_Key_S *)arg, sizeof(struct XGMCS_XgponBroadcast_Key_S), ret);
				ret = xmcs_set_broadcast_key(&temp);
			}
			break ;	
		case GPON_10G_IOS_O23_O4_PLOAM_CTRL :
			ret = xmcs_set_o23_o4_ploam_ctrl((XGPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_10G_IOG_O23_O4_PLOAM_CTRL :
			{
				XGPON_SW_HW_SELECT_T temp;
				memset(&temp, 0, sizeof(XGPON_SW_HW_SELECT_T));
				ret = xmcs_get_o23_o4_ploam_ctrl(&temp);
				COPY_TO_USER((XGPON_SW_HW_SELECT_T *)arg, &temp, sizeof(XGPON_SW_HW_SELECT_T), ret);	
			}
			break ;
		case GPON_10G_IOS_EQD_OFFSET_FLAG :
			ret = xmcs_set_eqd_offset_flag((char)arg) ;
			break ;
		case GPON_10G_IOS_EQD_OFFSET_O4 :
			ret = xmcs_set_eqd_offset_o4((unchar)arg) ;
			break ;
		case GPON_10G_IOS_EQD_OFFSET_O5 :
			ret = xmcs_set_eqd_offset_o5((unchar)arg) ;
			break ;
		case GPON_10G_IOG_EQD_OFFSET :
			{
				struct XGMCS_EqdOffset_S temp;
				memset(&temp, 0, sizeof(struct XGMCS_EqdOffset_S));
				ret = xmcs_get_eqd_offset(&temp);
				COPY_TO_USER((struct XGMCS_EqdOffset_S *)arg, &temp, sizeof(struct XGMCS_EqdOffset_S), ret);	
			}
			break ;
		case GPON_10G_IOG_ONU_INFO :
			{
				struct XGMCS_XgponOnuInfo_S temp;
				memset(&temp, 0, sizeof(struct XGMCS_XgponOnuInfo_S));
				ret = xmcs_get_onu_info(&temp);
				COPY_TO_USER((struct XGMCS_XgponOnuInfo_S *)arg, &temp, sizeof(struct XGMCS_XgponOnuInfo_S), ret);	
			}
			break ;
		case GPON_10G_IOS_SNIFFER_GTC :
			{
				GPON_10G_DEV_SNIFFER_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_SNIFFER_MODE_T));
				COPY_FROM_USER(&temp, (GPON_10G_DEV_SNIFFER_MODE_T *)arg, sizeof(GPON_10G_DEV_SNIFFER_MODE_T), ret);
				ret = xmcs_set_sniffer_mode(&temp);
			}
			break ;
		case GPON_10G_IOG_SNIFFER_GTC :
			{
				GPON_10G_DEV_SNIFFER_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_SNIFFER_MODE_T));
				ret = xmcs_get_sniffer_mode(&temp);
				COPY_TO_USER((GPON_10G_DEV_SNIFFER_MODE_T *)arg, &temp, sizeof(GPON_10G_DEV_SNIFFER_MODE_T), ret);	
			}
			break ;
		case GPON_10G_IOS_PLOAM_FILTER	:
			{
				GPON_10G_DEV_PLOAMD_FILTER_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_PLOAMD_FILTER_MODE_T));
				COPY_FROM_USER(&temp, (GPON_10G_DEV_PLOAMD_FILTER_MODE_T *)arg, sizeof(GPON_10G_DEV_PLOAMD_FILTER_MODE_T), ret);
				ret = xmcs_set_ploam_filter_mode(&temp);
			}
			break ;
		case GPON_10G_IOG_PLOAM_FILTER :
			{
				GPON_10G_DEV_PLOAMD_FILTER_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_PLOAMD_FILTER_MODE_T));
				ret = xmcs_get_ploam_filter_mode(&temp);
				COPY_TO_USER((GPON_10G_DEV_PLOAMD_FILTER_MODE_T *)arg, &temp, sizeof(GPON_10G_DEV_PLOAMD_FILTER_MODE_T), ret);	
			}
			break ;
		case GPON_10G_IOS_ERR_PLOAM_DROP :
			ret = xmcs_set_ploam_micerr_drop_mode((XPON_Mode_t) arg) ;
			break ;				
		case GPON_10G_IOG_ERR_PLOAM_DROP :
			{
				XPON_Mode_t temp;
				memset(&temp, 0, sizeof(XPON_Mode_t));
				ret = xmcs_get_ploam_micerr_drop_mode(&temp);
				COPY_TO_USER((XPON_Mode_t *)arg, &temp, sizeof(XPON_Mode_t), ret);	
			}
			break ;			
		case GPON_10G_IOS_SW_DYING_GASP :
			{
				GPON_10G_DYING_GASP_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DYING_GASP_MODE_T));
				COPY_FROM_USER(&temp, (GPON_10G_DYING_GASP_MODE_T *)arg, sizeof(GPON_10G_DYING_GASP_MODE_T), ret);
				ret = xmcs_set_sw_dying_gasp_mode(&temp) ;
			}
			break ;
		case GPON_10G_IOG_SW_DYING_GASP :
			{
				GPON_10G_DYING_GASP_MODE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DYING_GASP_MODE_T));
				ret = xmcs_get_sw_dying_gasp_mode(&temp);
				COPY_TO_USER((GPON_10G_DYING_GASP_MODE_T *)arg, &temp, sizeof(GPON_10G_DYING_GASP_MODE_T), ret);	
			}
			break ;
		case GPON_10G_IOS_PHY_MODE :
			ret = xmcs_set_phy_mode((XGMCSIF_PhyMode_t)arg);
			break ;
        case GPON_10G_IOS_ACK_SEND :
			ret = xmcs_set_ack_num((unchar)arg);
            break ;
        case GPON_10G_IOS_DBA_BACKDOOR :
			{
				GPON_10G_DEV_DBA_BACKDOOR_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_DBA_BACKDOOR_T));
				COPY_FROM_USER(&temp, (GPON_10G_DEV_DBA_BACKDOOR_T *)arg, sizeof(GPON_10G_DEV_DBA_BACKDOOR_T), ret);
				ret = xmcs_set_dba_backdoor(&temp);
			}
            break ; 
        case GPON_10G_IOG_DBA_BACKDOOR :
			{
				GPON_10G_DEV_DBA_BACKDOOR_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_DBA_BACKDOOR_T));
				ret = xmcs_get_dba_backdoor(&temp);
				COPY_TO_USER((GPON_10G_DEV_DBA_BACKDOOR_T *)arg, &temp, sizeof(GPON_10G_DEV_DBA_BACKDOOR_T), ret);	
			}
			break ; 
        case GPON_10G_IOS_OMCI_MIC_CTRL :
			{
				GPON_10G_DEV_OMCI_MIC_CTRL_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_OMCI_MIC_CTRL_T));
				COPY_FROM_USER(&temp, (GPON_10G_DEV_OMCI_MIC_CTRL_T *)arg, sizeof(GPON_10G_DEV_OMCI_MIC_CTRL_T), ret);
				ret = xmcs_set_omci_mic_ctrl(&temp);
			}
            break ; 
        case GPON_10G_IOG_OMCI_MIC_CTRL :
			{
				GPON_10G_DEV_OMCI_MIC_CTRL_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_OMCI_MIC_CTRL_T));
				ret = xmcs_get_omci_mic_ctrl(&temp);
				COPY_TO_USER((GPON_10G_DEV_OMCI_MIC_CTRL_T *)arg, &temp, sizeof(GPON_10G_DEV_OMCI_MIC_CTRL_T), ret);	
			}
			break ;
        case GPON_10G_IOS_OMCI_BROADCAST_KEY :
			{
				struct XMCS_OMCI_BROADCAST_KEY_S temp;
				memset(&temp, 0, sizeof(struct XMCS_OMCI_BROADCAST_KEY_S));
				COPY_FROM_USER(&temp, (struct XMCS_OMCI_BROADCAST_KEY_S *)arg, sizeof(struct XMCS_OMCI_BROADCAST_KEY_S), ret);
				ret = xmcs_set_omci_broadcast_key(&temp);
			}
			break ;
        case GPON_10G_IOC_OMCI_BROADCAST_KEY :
			{
				struct XMCS_OMCI_BROADCAST_KEY_S temp;
				memset(&temp, 0, sizeof(struct XMCS_OMCI_BROADCAST_KEY_S));
				COPY_FROM_USER(&temp, (struct XMCS_OMCI_BROADCAST_KEY_S *)arg, sizeof(struct XMCS_OMCI_BROADCAST_KEY_S), ret);
           		ret = xmcs_clean_omci_broadcast_key(&temp);
        	}
			break ;
		case GPON_10G_IOG_GEM_COUNTER :
			{
				struct XMCS_CounterCfg_S temp;
				memset(&temp, 0, sizeof(struct XMCS_CounterCfg_S));
				COPY_FROM_USER(&temp, (struct XMCS_CounterCfg_S *)arg, sizeof(struct XMCS_CounterCfg_S), ret);
				ret = xmcs_get_gem_counter(&temp);
				COPY_TO_USER((struct XMCS_CounterCfg_S *)arg, &temp, sizeof(struct XMCS_CounterCfg_S), ret);	
			}
			break ;
        case GPON_10G_IOS_CLEAR_GEM_COUNTER:
			ret = xmcs_set_counter_type((GPON_10G_COUNTER_TYPE_t)arg) ;
            break;
		case GPON_10G_IOS_COUNTER_TYPE :
			ret = xmcs_set_counter_type((GPON_10G_COUNTER_TYPE_t)arg) ;
			break ;
		case GPON_10G_IOG_COUNTER_TYPE :
			{
				GPON_10G_COUNTER_TYPE_t temp;
				memset(&temp, 0, sizeof(GPON_10G_COUNTER_TYPE_t));
				COPY_FROM_USER(&temp, (GPON_10G_COUNTER_TYPE_t *)arg, sizeof(GPON_10G_COUNTER_TYPE_t), ret);
				ret = xmcs_get_counter_type(&temp);
				COPY_TO_USER((GPON_10G_COUNTER_TYPE_t *)arg, &temp, sizeof(GPON_10G_COUNTER_TYPE_t), ret);	
			}
			break ;			
        case GPON_10G_IOS_SW_RESET :
			ret = xmcs_set_sw_reset() ;
			break ;
        case GPON_10G_IOS_CLEAR_All_COUNTER:
            ret = gponDevClearHwCounter() ;   
			break ; 
		case GPON_10G_IOS_TC_COUNTER:
			ret = xmcs_get_xgpon_tc_counter((GPON_10G_TC_COUNTER_T *)arg) ; 
			break ; 	
		case GPON_10G_IOS_DS_MGNT_COUNTER:
			ret = xmcs_get_xgpon_ds_mgnt_counter((GPON_10G_DS_MGNT_COUNTER_T *)arg) ; 
			break ; 	
		case GPON_10G_IOS_US_MGNT_COUNTER:
			ret = xmcs_get_xgpon_us_mgnt_counter((GPON_10G_US_MGNT_COUNTER_T *)arg) ;	
			break ; 
        case GPON_10G_IOS_GEM_UP_AES_MODE:
			ret = xmcs_set_xgpon_us_aes_mode((UPAES_Mode_t)arg) ; 
			break ;   
#if defined(TCSUPPORT_CPU_AN7583)
                case GPON_10G_IOS_DBA_BACKDOOR_SEPERATE:
			{
				GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T temp;
				memset(&temp, 0, sizeof(GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T));
				COPY_FROM_USER(&temp, (GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T *)arg, sizeof(GPON_10G_DEV_DBA_BACKDOOR_SEPERATE_T), ret);
				ret = xmcs_set_dba_backdoor_seperate(&temp);
			}
			break ;
#endif 
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}

	return ret ;
}


int gpon_cmd_proc(uint cmd, ulong arg) 
{
	int ret = -1;	
	struct XMCS_GponSnPasswd_S SnPasswd;
	struct XMCS_CounterCfg_S GemCounter;
	

#ifdef TCSUPPORT_CPU_ARMV8_64
	cmd = cmd & IOCTL_CMD;
#endif
	
	switch(cmd) {
		case GPON_IOS_SN_PASSWD :
			{
				struct XMCS_GponSnPasswd_S temp ;
				memset(&temp, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
				COPY_FROM_USER(&temp,(struct XMCS_GponSnPasswd_S *)arg,sizeof(struct XMCS_GponSnPasswd_S),ret);
				ret = xmcs_set_sn_passwd(&temp) ;
			}
			break ;
		case GPON_IOS_SN :
			if(copy_from_user((void*)&SnPasswd, (void __user *)arg, sizeof(struct XMCS_GponSnPasswd_S))){
				printk("[%s %d]ERR: copy_from_user\n", __FUNCTION__, __LINE__);
				return -EINVAL;
			}
			ret = xmcs_set_sn(SnPasswd.sn);
			break ;
		case GPON_IOS_PASSWD :
			if(copy_from_user((void*)&SnPasswd, (void __user *)arg, sizeof(struct XMCS_GponSnPasswd_S))){
				printk("[%s %d]ERR: copy_from_user\n", __FUNCTION__, __LINE__);
				return -EINVAL;
			}
			ret = xmcs_set_passwd(SnPasswd.regid) ;
			break ;
		case GPON_IOS_EMERGENCY_STATE :
			ret = xmcs_set_emergency_state((XPON_Mode_t)arg) ;
			break ;
		case GPON_GET_TRAFFIC_STATE:
			{
				uint temp = 0;
				ret = xmcs_get_gpon_traffic_state(&temp);
				COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);					
			}
			break ; 
		case GPON_IOG_GEM_COUNTER :			
			memset(&GemCounter, 0, sizeof(struct XMCS_CounterCfg_S));
			if(copy_from_user((void*)&GemCounter, (void __user *)arg, sizeof(struct XMCS_CounterCfg_S))){
				printk("[%s %d]ERR: copy_from_user\n", __FUNCTION__, __LINE__);
				return -EINVAL;
			}
			ret = xmcs_get_gem_counter(&GemCounter) ;
			ret = copy_to_user((void __user *)arg, &GemCounter, sizeof(struct XMCS_CounterCfg_S));
			if(0!=ret)	{
				return -EINVAL ;
			}
			break ;
		case GPON_IOG_TCONT_COUNTER :
			{
				STATISTIC_CFG_t temp = {0};
				COPY_FROM_USER(&temp,(STATISTIC_CFG_t *)arg,sizeof(STATISTIC_CFG_t),ret);	
			    ret = xmcs_get_tcont_counter(&temp) ;
				COPY_TO_USER((STATISTIC_CFG_t *)arg,&temp,sizeof(STATISTIC_CFG_t),ret);
			}			
			break ;
		case GPON_IOG_ONU_INFO :
			{
				struct XMCS_GponOnuInfo_S temp = {0};
				memset(&temp, 0, sizeof(struct XMCS_GponOnuInfo_S));
				ret = xmcs_gpon_get_onu_info(&temp);
				COPY_TO_USER((struct XMCS_GponOnuInfo_S *)arg, &temp, sizeof(struct XMCS_GponOnuInfo_S), ret);
			}
			break ;		
		case GPON_IOG_GPON_TCONT_IDX:
			ret = xmcs_get_gpon_tcont_info((struct XMCS_GponTcontInfo_S *)arg);
			break;
		case GPON_IOS_TOD_CFG :
			{
#ifdef TCSUPPORT_CPU_EN7581
				struct XMCS_GponTodCfg_S temp = {0};
				if(copy_from_user(&temp,(struct XMCS_GponTodCfg_S *)arg,sizeof(struct XMCS_GponTodCfg_S)))
				{
					printk("[%s %d]ERR: tod copy_from_user\n", __FUNCTION__, __LINE__);
					return -EINVAL;				
				}
			
				ret = xmcs_set_gpon_tod(&temp) ;
#endif
			}
			break;
		case GPON_IOS_MIB_FLAG :
            ret = xmcs_set_mib_flag((uint)arg) ;
            break;
		case GPON_IOG_OLT_DISTANCE :
			{
				uint32_t olt_distance = 0;
				ret = xmcs_get_olt_distance(&olt_distance);
				COPY_TO_USER((uint32_t *)arg,&olt_distance,sizeof(uint32_t),ret);
			}
			break;
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}

	return ret ;
}

int rdkb_10g_cmd_proc(uint cmd, ulong arg)
{
				int ret = 0;
				#ifdef TCSUPPORT_CPU_ARMV8_64
				cmd = cmd & IOCTL_CMD;
				#endif
				switch(cmd) {
					case RDKB_IOG_ACTIVATION_CNT:
						{
							uint temp = 0;
							ret = xmcs_get_activation_counter(&temp);
							COPY_TO_USER((uint *)arg,&temp,sizeof(uint),ret);
						}
						break ;
					case RDKB_IOG_PLOAM_CNT:
						{
							GPON_PLOAM_CNT_T temp = {0};
							COPY_FROM_USER(&temp,(GPON_PLOAM_CNT_T *)arg,sizeof(GPON_PLOAM_CNT_T),ret);
							ret = xmcs_get_ploamMsg_counter(&temp);
							COPY_TO_USER((GPON_PLOAM_CNT_T *)arg,&temp,sizeof(GPON_PLOAM_CNT_T),ret);
						}
						break ;;
					case RDKB_IOG_OMCI_CNT:
						{
							GPON_OMCI_CNT_T temp = {0};
							COPY_FROM_USER(&temp,(GPON_OMCI_CNT_T *)arg,sizeof(GPON_OMCI_CNT_T),ret);
							ret = xmcs_get_omciRX_counter(&temp);
							COPY_TO_USER((GPON_OMCI_CNT_T *)arg,&temp,sizeof(GPON_OMCI_CNT_T),ret);
						}
						break ;
                    case RDKB_IOG_OMCI_BASE_CNT:
                        {
							GPON_OMCI_CNT_T temp = {0};
							COPY_FROM_USER(&temp,(GPON_OMCI_CNT_T *)arg,sizeof(GPON_OMCI_CNT_T),ret);
							ret = xmcs_get_omciRX_Base_counter(&temp);
							COPY_TO_USER((GPON_OMCI_CNT_T *)arg,&temp,sizeof(GPON_OMCI_CNT_T),ret);
						}
						break ;
                    case RDKB_IOG_OMCI_EXT_CNT:
                        {
							GPON_OMCI_CNT_T temp = {0};
							COPY_FROM_USER(&temp,(GPON_OMCI_CNT_T *)arg,sizeof(GPON_OMCI_CNT_T),ret);
							ret = xmcs_get_omciRX_Extend_counter(&temp);
							COPY_TO_USER((GPON_OMCI_CNT_T *)arg,&temp,sizeof(GPON_OMCI_CNT_T),ret);
						}
						break ;
					case RDKB_IOG_ALARM:
						{
							GPON_Alarm_T temp = {0};
							COPY_FROM_USER(&temp,(GPON_Alarm_T *)arg,sizeof(GPON_Alarm_T),ret);
							ret = xmcs_get_gponAlarm(&temp) ;
							COPY_TO_USER((GPON_Alarm_T *)arg,&temp,sizeof(GPON_Alarm_T),ret);
						}
						break;
					case RDKB_IOG_PHY_STATUS:
						{
							PHY_STATUS_t temp = PHY_LOS_STATUS;
							ret = 0;
							if(gpPhyData->phy_link_status != PHY_LINK_STATUS_LOS){
								temp = PHY_READY_STATUS;
							}
							COPY_TO_USER((PHY_STATUS_t *)arg,&temp,sizeof(PHY_STATUS_t),ret);
						}
						break;
					case RDKB_IOS_SIGNAL_FAIL_THRESHOLD:
						ret = 0;
						gpPhyData->signal_fail = (uint)arg;
						break;
					case RDKB_IOG_SIGNAL_FAIL_THRESHOLD:
						{
							ret = 0;
							COPY_TO_USER((uint *)arg,&gpPhyData->signal_fail,sizeof(uint),ret);
						}
						break;
					case RDKB_IOS_SIGNAL_DEGRADE_THRESHOLD:
						ret = 0;
						gpPhyData->signal_degrade = (uint)arg;
						break;
					case RDKB_IOG_SIGNAL_DEGRADE_THRESHOLD:
						{
							ret = 0;
							COPY_TO_USER((uint *)arg,&gpPhyData->signal_degrade,sizeof(uint),ret);
						}
					
						break;
					default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}
				return ret;
}	

