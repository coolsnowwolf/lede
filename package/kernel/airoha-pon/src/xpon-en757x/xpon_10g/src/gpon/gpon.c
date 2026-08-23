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
#include <linux/timer.h>
#include "common/xpon_global.h" 
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "common/phy_if_wrapper.h"
#include "common/xpon_api.h"
#include "gpon/gpon_security.h"
/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
extern int max_cnt;


/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/
int snSendInO23Cnt=0;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern uint bwmap_print_flag;
extern uint sw_resync_flag;
//extern void set_fix_reg_list(void);

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
**		gpon_phy_ready_handler
**  description:
**		handle the processing when phy report ready event
**  parameters:
**		src: phy event type
**  global:
**		gpGponPriv
**  return:
**		none
**  call:
**		GPON_STOP_TIMER.    
**		XPON_PHY_TX_DISABLE.    
**		gponDevResetCtrl.    
**		gpon_act_change_state.    
**		gpon_enable.    
**		cgponDevClearCntAndErr.    
**		gponDevGetTxSyncRdy.   
**		cploam_send_serial_number_msg.   
**		xmcs_report_event.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
static uint32 profileRegReadData = 0;

void gpon_phy_ready_handler(PHY_Event_Source_t src)
{
	REG_SW_SET_KIDX gponKeyIdx;
	PON_MSG((MSG_INT|MSG_ACT), "PHY IRQ: xgpon detect phy ready.\n") ;
	
    if(gpGponPriv->typeBOnGoing && GPON_CURR_STATE == GPON_10G_STATE_O6){
		PON_MSG((MSG_INT|MSG_ACT), "PHY IRQ: xgpon typeB on going.\n") ;
		if(NGPON2_MODE && gpGponPriv->gponCfg.ng2.protectFlag == PROTECT_ENABLE){
			GPON_STOP_TIMER(gpGponPriv->to3_timer)
		}else{
			GPON_STOP_TIMER(gpGponPriv->to2_timer)
		}
		gponDevMacReset(XPON_RESET_RELEASE);
		gponKeyIdx.Raw = IO_GREG(SW_SET_KIDX);
		gponKeyIdx.Bits.sw_set_pik_idx = gpGponPriv->gponCurKeyIdx.Bits.cur_pik_idx;
		gponKeyIdx.Bits.sw_set_pik_en = 1;
		gponKeyIdx.Bits.sw_set_oik_idx = gpGponPriv->gponCurKeyIdx.Bits.cur_oik_idx;
		gponKeyIdx.Bits.sw_set_oik_en = 1;
		IO_SREG(SW_SET_KIDX, gponKeyIdx.Raw);
		gponDevMpiRxStop(XPON_RESET_RELEASE);
#if !defined(TCSUPPORT_CPU_AN7583)
		gponDevRefreshEqdValue();
#endif		
		gpon_act_change_state(GPON_10G_STATE_O5) ;
		if(gponDevTxSyncRdy() != 0 ){
			printk("error!tx sync not ready in ready hanle,continue processing\n");
		}
		gponDevSwResync();
		PON_MSG((MSG_OAM|MSG_EQD), "synd ready, eqd:0x%.8X \n",IO_GREG(EQD)) ;
		gponDevMpiTxStop(XPON_RESET_RELEASE) ;
		if(gpGponPriv->gponCfg.MbiTxStopEn){
			gponDevTxMbiStop(XPON_RESET_RELEASE);
		}
		/*Restore upstream profile valid bit*/
		IO_SREG(US_PROF_VLD, profileRegReadData) ;
		gpGponPriv->typeBOnGoing = FALSE ;
    }else if(GPON_CURR_STATE == GPON_10G_STATE_O1) {
		/* Change the current state */
		    gpon_enable();
//		    set_fix_reg_list();
            gponDevClearCntAndErr();
			gpon_act_change_state(GPON_10G_STATE_O2_3) ;			
            
            if((gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS) &&(gpGponPriv->gponCfg.ploamCtrl == XGPON_SW)){
                ploam_send_serial_number_msg();
            }
	}else if(GPON_CURR_STATE == GPON_10G_STATE_O7){
			PON_MSG((MSG_INT|MSG_ACT), "Phy tx disable and pon mac release.\n") ;
			XPON_PHY_TX_DISABLE();
            gponDevResetCtrl(XPON_RESET_RELEASE);
	}else if(GPON_CURR_STATE == GPON_10G_STATE_O8){
			PON_MSG((MSG_INT|MSG_ACT), "ng2 tuning, dosnstrean synced. \n") ;
			if(NGPON2_MODE)
				GPON_STOP_TIMER(gpGponPriv->to4_timer) ;

			gpGponPriv->typeBOnGoing = FALSE ;
			gponDevResetCtrl(XPON_RESET_RELEASE);

			gponDevSetOmciIkIdx(0); //reset will recover omci ik index 1, while omci use 0

			if(ng2_o8_to_05){
				gpon_act_change_state(GPON_10G_STATE_O5);
				if(gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS)
				{
					ng2_o9_recv_grant_handler();
				}
			}
			else{
				//set eqd 0, OLT will set eqd 0 and ONU receive and handle in 09;
				//gponDevSetEqdValue(0,&gpGponPriv->gponCfg.eqd,gpPonSysData->sysPonMode,PLOAM_EQD_ABSOLUTE,PLOAM_EQD_POSITIVE);

				if(ng2_man_set_09 == 0){
					gpon_act_change_state(GPON_10G_STATE_O9);
				}
			}
	}
	else{
		PON_MSG((MSG_ERR|MSG_ACT), "Phy get ready handler at the error state.\n") ;
	}
	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_GPON ;
    xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_PHY_READY, 0) ;
}
/*______________________________________________________________________________
**  function name
**		gpon_phy_loss_handler
**  description:
**		handle the processing when phy report loss event
**  parameters:
**		src: phy event type
**  global:
**		gpGponPriv
**  return:
**		none
**  call:
**		gpon_disable.    
**		xmcs_report_event.    
**		gpon_act_change_state.    
**  revision:
**		v1.0
**____________________________________________________________________________*/

void gpon_phy_loss_handler(PHY_Event_Source_t src)
{
	REG_US_PROF_VLD usProfileVld;
	PHY_FecCount_T phyFecCount;
	
	PON_MSG((MSG_INT|MSG_ACT), "PHY IRQ: gpon detect phy loss.\n") ;
	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
	gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
	
	if((GPON_10G_STATE_O2_3 == GPON_CURR_STATE) ||(GPON_10G_STATE_O4 == GPON_CURR_STATE) || (GPON_10G_STATE_O9 == GPON_CURR_STATE)) {
		gpon_disable(GPON_MAC_PLAIN_RESET);        
	} else if( GPON_10G_STATE_O5 == GPON_CURR_STATE) {	
		/* Change the current state */
		profileRegReadData = IO_GREG(US_PROF_VLD);
		/*Modify the value of us_prof_vld bit*/
		usProfileVld.Raw = profileRegReadData;
		usProfileVld.Bits.us_prof3_vld = 0; 	//0:invalid
		usProfileVld.Bits.us_prof2_vld = 0;
		usProfileVld.Bits.us_prof1_vld = 0;
		usProfileVld.Bits.us_prof0_vld = 0;
		/*Disable upstream profile valid bit*/
		IO_SREG(US_PROF_VLD, usProfileVld.Raw) ;
		if(gpGponPriv->gponCfg.MbiTxStopEn){
			gponDevTxMbiStop(XPON_RESET_HOLD_ON);
		}
		gponDevMpiStop(XPON_RESET_HOLD_ON);
		gpon_act_change_state(GPON_10G_STATE_O6) ;
	}else if( GPON_10G_STATE_O8 == GPON_CURR_STATE) {
		gponDevResetCtrl(XPON_RESET_HOLD_ON);
	}else{
		PON_MSG((MSG_ERR|MSG_ACT), "Phy get loss handler at the error state.\n") ;
	}

	XPON_PHY_GET_API(PON_GET_PHY_RX_FEC_COUNTER, &phyFecCount);
	gphyFecCount.correct_codewords += phyFecCount.correct_codewords;
	gphyFecCount.uncorrect_codewords += phyFecCount.uncorrect_codewords;
	gphyFecCount.correct_bytes += phyFecCount.correct_bytes;
}
/*______________________________________________________________________________
**  function name
**		gpon_enable
**  description:
**		release the mac reset and init device 
**  parameters:
**		none
**  global:
**		none
**  return:
**		none
**  call:
**		gponDevResetCtrl.    
**		gpon_dev_init.    
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_enable(void)
{
	PON_MSG(MSG_INT, "gpon enable.\n") ;
	if((GPON_CURR_STATE!=GPON_10G_STATE_O1)
		&&(GPON_CURR_STATE!=GPON_10G_STATE_O2_3)
		&&(GPON_CURR_STATE!=GPON_10G_STATE_O7)) {
		return ;
	}
    gponDevResetCtrl(XPON_RESET_RELEASE);
	gpon_dev_init() ;

	gponDevSetPhyDsFecMde(gpGponPriv->gponCfg.dsFecMode);
}
/*______________________________________________________________________________
**  function name
**		gpon_disable
**  description:
**		change the onu state when onu is deactivated or phy loss, etc 
**  parameters:
**		none
**  global:
**		none
**  return:
**		none
**  call:
**		gwan_remove_all_tcont.    
**		gwan_remove_all_gemport_for_disable. 
**		XPON_PHY_TX_DISABLE.   
**		gpon_act_change_state.    
**		gponDevResetCtrl.    
**		gpon_phy_ready_handler
**		XPON_PHY_GET
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_disable(GPON_RESET_TYPE_t type)
{
	PON_PHY_Event_data_t phy_event;
	PON_MSG(MSG_INT, "gpon_disable: mac %s reset.\n",(type==GPON_MAC_PLAIN_RESET ? "plain":"with phy")) ;
		
	snSendInO23Cnt=0;
	/* reset the onu_id and omcc id */
	gpGponPriv->gponCfg.onu_id = GPON_UNASSIGN_ONU_ID ;
	CALL_USER_HOOK_SET_ONU_ID(gpGponPriv->gponCfg.onu_id);
	gpGponPriv->gponCfg.omcc = GPON_UNASSIGN_ONU_ID ;
	CALL_USER_HOOK_SET_OMCC_ID(gpGponPriv->gponCfg.omcc);
	gpGponPriv->gponCfg.eqd = 0 ;
    gpGponPriv->gponSecurity.ploamIkIdx = GPON_PLOAM_IK_IDX1;
	gpGponPriv->gponSecurity.omciIkIdx = GPON_OMCI_IK_IDX1;	
    gpGponPriv->gponSecurity.kekIdx = GPON_KEK_IK_IDX1;
	gwan_remove_all_tcont();	
	gwan_remove_all_gemport_for_disable() ;
    /*		to do discard burst porfiles ...	*/
	if(gpGponPriv->emergencyState) {
		XPON_PHY_TX_DISABLE();
		gpon_act_change_state(GPON_10G_STATE_O7) ;
	}else{
	    gponDevResetCtrl(XPON_RESET_HOLD_ON);
		gpon_act_change_state(GPON_10G_STATE_O1) ; 
		gponDevSetOnuId(GPON_UNASSIGN_ONU_ID, GPON_ONU_ID_INVALID);
		if(type==GPON_MAC_WITH_PHY_RESET){
			gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
			XPON_PHY_SET(PON_SET_PHY_LOGIC_RESET);
		}else if(gpPonSysData->sysStartup == PON_WAN_START){
			if(XPON_PHY_GET(PON_GET_PHY_IS_SYNC) == PHY_TRUE){
			PON_MSG(MSG_INT, "PHY IRQ: xgpon get phy ready status true.\n") ;
			gpPhyData->phy_link_status = PHY_LINK_STATUS_READY ;
			gpon_phy_ready_handler(phy_event.src);
			}else if (XPON_PHY_GET(PON_GET_PHY_IS_SYNC) == PHY_FALSE){
				PON_MSG(MSG_INT, "PHY IRQ: xgpon get phy ready status false.\n") ;
				gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
				xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
				CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS);
			}
		}
	}
}
/*______________________________________________________________________________
**	function name
**		is_same_ploam_msg_as_previous
**	description:
**		check if the ploam received is same as previous
**	parameters:
**		pMsg:ploam raw data
**	global:
**		None
**	return:
**		0: scuess
**		others: fail
**	call:
**		none
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline int is_same_ploam_msg_as_previous( PLOAM_RAW_General_T * pMsg)
{
	return !memcmp(&gpGponPriv->prePloamMsg, pMsg, sizeof(PLOAM_RAW_General_T)) ;
}
/*______________________________________________________________________________
**	function name
**		should_ignore_ploam_msg
**	description:
**		check if the same ploam received should be ignore
**	parameters:
**		pMsg:ploam raw data
**	global:
**		None
**	return:
**		0: scuess
**		others: fail
**	call:
**		none
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline int should_ignore_ploam_msg(PLOAM_RAW_General_T * pPloamMsg)
{
	static int same_cnt = 0;
	int is_same_msg = 0;

	/* 3 consecutivly same ploam msg, only process once */
	is_same_msg = is_same_ploam_msg_as_previous(pPloamMsg);
	
	if(is_same_msg) same_cnt ++;

	if((is_same_msg) && ((same_cnt % 3) != 0) ){
		PON_MSG(MSG_OAM, "Receive the PLOAM message same:%8X\n", pPloamMsg->value[0]) ;
		return TRUE;
	}
	else{
		memcpy(&gpGponPriv->prePloamMsg, pPloamMsg, sizeof(PLOAM_RAW_General_T)) ;
		same_cnt = 0;
	}

	return FALSE;
}
/*______________________________________________________________________________
**	function name
**		gpon_recv_ploam_message
**	description:
**		receive the ploam msg for hw
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevGetPloamMsg
**		should_ignore_ploam_msg
**		ploam_parser_down_message
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_recv_ploam_message(void)
{
	PLOAM_RAW_General_T ploamMsg;
	int depth=0;	
	REG_PLOAMd_FIFO_STS gponPloamStatus;	
	
	gponPloamStatus.Raw = IO_GREG(PLOAMd_FIFO_STS);	
	do {
		if((depth = gponDevGetPloamMsg(&ploamMsg)) <= 0){
			PON_MSG(MSG_ERR, "Receive PLOAM message failed.\n") ;
            return;
		} 
		else if(FALSE == should_ignore_ploam_msg(&ploamMsg) ){
            ploam_parser_down_message(&ploamMsg) ;
        }
		else{
			;
		}
		gpGponPriv->ploamMsgcounter.rxPloamMsgCnt++;
	} while(depth > PLOAM_DOWN_MSG_LENGTH) ;
}
/*______________________________________________________________________________
**	function name
**		gpon_fifo_err_sts
**	description:
**		check if the fifo err happen
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		none
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_fifo_err_sts(void)
{
	REG_FIFO_ERR_STS fifoErrStatus ;
	
	fifoErrStatus.Raw = IO_GREG(FIFO_ERR_STS) ;
	//Clear the fifo error  status
	IO_SREG(FIFO_ERR_STS, fifoErrStatus.Raw) ;
	
	if(fifoErrStatus.Bits.rx_ploamd_fifo_udrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: fifoErrStatus.\n") ;
	}
	if(fifoErrStatus.Bits.rx_ploamd_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_ploamd_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.rx_mbi_pl_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_mbi_pl_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.rx_mbi_hdr_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_mbi_hdr_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.rx_aes_rdm_ciph_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_aes_rdm_ciph_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.rx_aes_ciph_txt_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_aes_ciph_txt_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.rx_aes_cryp_cnt_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: rx_aes_cryp_cnt_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.tx_align_fifo_udrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: tx_align_fifo_udrn.\n") ;
	}
	if(fifoErrStatus.Bits.tx_ploamu_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: tx_ploamu_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.bst_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: bst_fifo_ovrn.\n") ;
	}
	if(fifoErrStatus.Bits.sgl_fifo_ovrn) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON fifo err status: sgl_fifo_ovrn.\n") ;
	}
}

/*______________________________________________________________________________
**	function name
**		gpon_bwm_chk_err_sts
**	description:
**		check if the bwmap err happen
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevGetPloamMsg
**		should_ignore_ploam_msg
**		ploam_parser_down_message
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_bwm_chk_err_sts(void)
{
	REG_DBG_BWM_CKH_STS bwpChkStatus ;
	bwpChkStatus.Raw = IO_GREG(DBG_BWM_CKH_STS);
	//Clear the bwmap error	status
	IO_SREG(DBG_BWM_CKH_STS, bwpChkStatus.Raw) ;
	bwpChkStatus.Raw &= bwmap_print_flag ;
#if !defined(TCSUPPORT_CPU_EN7581)
	if(bwpChkStatus.Bits.o23_o4_data_gnt_recv) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: o23_o4_data_gnt_recv.\n") ;
	}
#endif
	if(bwpChkStatus.Bits.other_tid_ins_my_bst_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: other_tid_ins_my_bst_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_bst_len_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_bst_len_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_my_bst_num_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_my_bst_num_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_my_alloc_num_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_my_alloc_num_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_bst_alloc_num_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_bst_alloc_num_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_bwm_alloc_num_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_bwm_alloc_num_err.\n") ;
	}
	if(bwpChkStatus.Bits.alloc_hec_uc_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: alloc_hec_uc_err.\n") ;
	}
	if(bwpChkStatus.Bits.bst_split_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: bst_split_err.\n") ;
	}
	if(bwpChkStatus.Bits.my_tid_ins_bst_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: my_tid_ins_bst_err.\n") ;
	}
	if(bwpChkStatus.Bits.min_gsize_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: min_gsize_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_gsize_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_gsize_err.\n") ;
	}
	if(bwpChkStatus.Bits.start_time_order_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: start_time_order_err.\n") ;
	}
	if(bwpChkStatus.Bits.max_start_time_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: max_start_time_err.\n") ;
	}
	if(bwpChkStatus.Bits.min_bst_intvl_err) {
		PON_MSG((MSG_INT|MSG_ERR), "GPON bwmap check err status: min_bst_intvl_err.\n") ;
	}
}
void gpon_sw_resync(void)
{
	if(snSendInO23Cnt>=max_cnt){
		snSendInO23Cnt=0;
		if(isEN7581 || isAN7583){
			gponDevResetCtrl(XPON_RESET_HOLD_ON);
			XPON_PHY_SET(PON_SET_PHY_SCU_RST);
			gponDevResetCtrl(XPON_RESET_RELEASE);
			printk("XGPON RESET PON !\n");
		}else{
			gponDevMpiStop(XPON_RESET_HOLD_ON);
			gponDevSwResync();
			gponDevMpiStop(XPON_RESET_RELEASE);
			printk("XGPON SW Resync!\n");
		}
	}else{
		snSendInO23Cnt++;
	}
}
/*______________________________________________________________________________
**	function name
**		ng2_tuning_sfc_match_handler
**	description:
**		NGPON2 tuning Super Frame Counter matched
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gpon_act_change_state
**		swap_work_standby_channel
**	revision:
**		v1.0
**____________________________________________________________________________*/
void ng2_tuning_sfc_match_handler(void){

	if(GPON_CURR_STATE != GPON_10G_STATE_O5_2) {
		PON_MSG((MSG_OAM | MSG_ERR), "ng2_o9_recv_grant_handler not in O9 state, do nothing\n") ;
		return;
	}

	PON_MSG((MSG_OAM | MSG_ACT), "ng2_tuning_sfc_match_handler -> 08\n");

	gponDevMpiTxStop(XPON_RESET_HOLD_ON);

	//enter O8 Ds Tuning State
	gpon_act_change_state(GPON_10G_STATE_O8) ;

	swap_work_standby_channel();
}

/*______________________________________________________________________________
**	function name
**		ng2_o9_recv_grant_handler
**	description:
**		NGPON2 O9 receive grant hanlder
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		ploam_send_tuning_resp_msg
**	revision:
**		v1.0
**____________________________________________________________________________*/
void ng2_o9_recv_grant_handler(void){
	static int cnt = 0;

	// send Complete_u, then OLT send Complete_d
	cnt ++ ;
	PON_MSG((MSG_OAM | MSG_ACT), "ng2_o9_recv_grant_handler cnt=%d\n",cnt) ;

	//send response
	ploam_send_tuning_resp_msg(gpGponPriv->gponCfg.ng2.preSeqNo++,TUNING_RSP_COMPLETE_U,0);
}

/*______________________________________________________________________________
**	function name
**		mon_not_gnt_handler
**	description:
**		monitor not grant receive inturrupt handler
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**
**	revision:
**		v1.0
**____________________________________________________________________________*/
void mon_not_gnt_handler(void){
#ifdef TCSUPPORT_CPU_EN7581
	if(GPON_CURR_STATE != GPON_10G_STATE_O5) {
		PON_MSG((MSG_ACT | MSG_ERR), "cal_gnt_zero_handler  not in O5 state, do nothing\n") ;
		return;
	}

	//offline
	gpon_disable(GPON_MAC_PLAIN_RESET);
#else
	PON_MSG((MSG_DBG), "mon_not_gnt_handler non-7581 not support!\n") ;
#endif
}
/*______________________________________________________________________________
**	function name
**		gpon_isrc
**	description:
**		check if the gpon interrupt happen
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevGetPloamMsg
**		gpon_key_index_change_by_hw
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_isr(void)
{
	REG_INT_STATUS intStatus ;
	REG_INT_ENABLE intEnable ;
	GPON_DEV_ERR_STATUS_T status;
	
	intStatus.Raw = IO_GREG(INT_STATUS) ;
	intEnable.Raw = IO_GREG(INT_ENABLE) ;

	//Clear the interrupt status
	intStatus.Raw &= intEnable.Raw ;
	IO_SREG(INT_STATUS, intStatus.Raw) ;

	memset(&status, 0x0, sizeof(GPON_DEV_ERR_STATUS_T));		
	gponDevGetErrStatus(&status); 
	
	/*clear the interrupt detail error status*/
	gponDevSetErrStatusClear();

	if(intStatus.Raw & intEnable.Raw){
		if(intStatus.Raw & XGPON_INT_INDICATION){
			if(intStatus.Bits.o23_sn_onu_req_recv_int) {
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: SN_Request received interrupt.\n") ;
			}
			if(intStatus.Bits.o23_sn_onu_send_int) {
				gpon_sw_resync();
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: SN sent in O2_3 state interrupt.\n") ;
				gpGponPriv->ploamMsgcounter.txPloamMsgCnt++;
			}
			if(intStatus.Bits.o4_ranging_req_recv_int) {			
				gpon_key_index_change_by_hw(&gpGponPriv->gponSecurity);
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: Ranging_Request received interrupt.\n") ;
			}
			if(intStatus.Bits.o4_registration_send_int) {
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: registration send in O4 interrupt.\n") ;
				gpGponPriv->ploamMsgcounter.txPloamMsgCnt++;
			}
			if(intStatus.Bits.us_no_msg_send_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: no ploam message send interrupt.\n") ;
			}			
			if(intStatus.Bits.us_prof_idx_chg_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: us profile index changing interrupt.\n") ;
			}
			if(intStatus.Bits.olt_ds_fec_chg_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: olt ds fec change interrupt.\n") ;
			}			
			if(intStatus.Bits.dying_gasp_send_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: dying gasp send interrupt.\n") ;
			}
			if(intStatus.Bits.o5_eqd_adj_done_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: eqd adjustment done interrupt.\n") ;
			}
			if(intStatus.Bits.fwi_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: fwi interrupt.\n") ;
			}
			if(intStatus.Bits.lwi_int) {
				PON_MSG(MSG_INT, "XGPON IRQ: lwi interrupt.\n") ;
			}
			if(intStatus.Bits.turning_sfc_match_int) {
				PON_MSG(MSG_INT, "GPON IRQ: turning SFC matched interrupt.\n") ;
				ng2_tuning_sfc_match_handler();
			}
			if(intStatus.Bits.o9_gnt_recv_int) {
				PON_MSG(MSG_INT, "GPON IRQ: O9 grant recvied interrupt\n") ;
				ng2_o9_recv_grant_handler();
			}
#ifdef TCSUPPORT_CPU_EN7581
			if(intStatus.Bits.cal_gnt_zero_int)
			{
				PON_MSG(MSG_INT, "XGPON IRQ: calculate grant zero interrupt.\n") ;
			}
			if(intStatus.Bits.mon_not_gnt_int)
			{
				PON_MSG(MSG_INT, "XGPON IRQ: monitor not gnt interrupt.\n") ;
			}
#endif
		}
		if(intStatus.Raw & XGPON_INT_PLOAM) {
			if(intStatus.Bits.ploamu_send_int) {
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: A PLOAM message sent interrupt.\n") ;
			}
			if(intStatus.Bits.ploamd_recv_int) {
				PON_MSG((MSG_INT|MSG_ACT), "XGPON IRQ: A PLOAM message received interrupt.\n") ;
				gpon_recv_ploam_message();
			}
		}
		if(intStatus.Raw & XGPON_INT_ERROR) {
			if(intStatus.Bits.fifo_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "XGPON IRQ: fifo error interrupt,status:%.8x\n",status.gponFifoErrSts.Raw) ;
				/*gpon_fifo_err_sts();*/				
				gpGponPriv->gponCfg.intErrStatus.gponFifoErrSts.Raw |= status.gponFifoErrSts.Raw;
			}
			if(intStatus.Bits.tx_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "XGPON IRQ: tx error interrupt,status:%.8x\n",status.gponTxErrSts.Raw) ;
				if((status.gponTxErrSts.Bits.tx_late_start_err) && (sw_resync_flag)){
					gponDevSwResyncCompleteProcess();
					PON_MSG((MSG_INT|MSG_ERR), "XGPON IRQ: Tx late happen, trigger sw resync\n") ; 			
				}
				gpGponPriv->gponCfg.intErrStatus.gponTxErrSts.Raw |= status.gponTxErrSts.Raw;
			}
			if(intStatus.Bits.rx_err_int) {
				if(gpGponPriv->gponCfg.hec3errCtrl.hec_3err_enable){
					gpon_rxhec3err_statistic_test();
				}
				if(status.gponRxErrSts.Bits.rx_los_gem_del_err) {
					gpon_set_alarmBit(LCDG_INDEX);
					PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Loss of GEM delineation interrupt.\n") ;
				}
				PON_MSG((MSG_INT|MSG_ERR), "XGPON IRQ: rx error interrupt,status:%.8x\n",status.gponRxErrSts.Raw ) ;
				gpGponPriv->gponCfg.intErrStatus.gponRxErrSts.Raw |= status.gponRxErrSts.Raw;
			}
            if(intStatus.Bits.bwm_chk_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "XGPON IRQ: bwmap check error interrupt,status:%.8x\n",status.gponbwpChkStatus.Raw) ;
				gpGponPriv->gponCfg.intErrStatus.gponbwpChkStatus.Raw |= status.gponbwpChkStatus.Raw;
			}
		}
		if(intStatus.Raw & XGPON_INT_SECURITY) {
			if(intStatus.Bits.us_key_switch_done_int) {				
				PON_MSG((MSG_INT|MSG_SECUR), "XGPON IRQ: upstream have sen XGEM using new key index interrupt.\n") ;
			}			
		}
	}
	
	gpon_check_alarm_jiffiesClk();
}
/*______________________________________________________________________________
**	function name
**		gpon_stop_timer
**	description:
**		stop the TO1 ,TO2,gemMibTimer,TK4 and TK5 timer
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		GPON_STOP_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_stop_timer(void)
{
	GPON_STOP_TIMER(gpGponPriv->to1_timer) ;
	GPON_STOP_TIMER(gpGponPriv->to2_timer) ;
	if(NGPON2_MODE){
		GPON_STOP_TIMER(gpGponPriv->to3_timer) ;
		GPON_STOP_TIMER(gpGponPriv->to4_timer) ;
		GPON_STOP_TIMER(gpGponPriv->to5_timer) ;
		//GPON_STOP_TIMER(gpGponPriv->to6_timer) ;
		GPON_STOP_TIMER(gpGponPriv->toZ_timer) ;
	}
	GPON_STOP_TIMER(gpWanPriv->gpon.gemMibTimer) ;
    GPON_STOP_TIMER(gpGponPriv->gponSecurity.TK4_timer);
	GPON_STOP_TIMER(gpGponPriv->gponSecurity.TK5_timer);
}

/*______________________________________________________________________________
**	function name
**		gpon_silence_interval_expires
**	description:
**		stop the gpon silence when timer expires
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevSetO23O4PloamCtrl
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_silence_interval_expires(TIMER_FUN_PAAM arg)
{
    /*Clear silence!*/	
	PON_MSG((MSG_DBG), "XGPON silence interval expires\n") ;
	gponDevSetO23O4PloamCtrl(XGPON_HW);
}
/*______________________________________________________________________________
**	function name
**		gpon_start_silence
**	description:
**		start the gpon silence by stopping HW auto reply
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevSetO23O4PloamCtrl
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_start_silence(void)
{
	PON_MSG((MSG_DBG), "XGPON start silence\n") ;
	gponDevSetO23O4PloamCtrl(XGPON_SW);
	GPON_START_TIMER(gpGponPriv->silence_timer,gpGponPriv->gponCfg.silenceInterval ) ;
}
/*______________________________________________________________________________
**	function name
**		gpon_stop_silence
**	description:
**		stop the gpon silence by restoring HW auto reply
**	parameters:
**		none
**	global:
**		None
**	return:
**		none
**	call:
**		gponDevSetO23O4PloamCtrl
**	revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_stop_silence(void)
{
	PON_MSG((MSG_DBG), "XGPON stop silence\n") ;
	gponDevSetO23O4PloamCtrl(XGPON_HW);
	GPON_STOP_TIMER(gpGponPriv->silence_timer) ;
}

int gpon_create_timer(struct timer_list *timer, gponTimerCallback callback, unsigned long param)
{
	if((timer == NULL) ||( callback == NULL) ){
		return -1;
	}
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	init_timer(timer);
	timer->function = callback;
#else
	timer_setup(timer, callback, 0);
#endif
	timer->expires= param;

	return 0;
}

int is_ngpon2_mode(void)
{
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G
		|| gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G
		|| gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G)
	{
		return 1;
	}

	return 0;
}

void ng2_system_config_reset(void)
{
	int index = 0;

	PON_MSG((MSG_DBG), "ng2_system_config_reset\n") ;

	for(index = 0; index < NG2_CHANNEL_MAX_NUM; index ++){
		memset(&gpGponPriv->gponCfg.ng2.channenProf[index],0,sizeof(Channel_Prof_T));
		gpGponPriv->gponCfg.ng2.channenProf[index].profVer = NGPON2_PROF_VER_INVALID;
	}

	//reset system profile
	memset(&gpGponPriv->gponCfg.ng2,0,sizeof(NGPON2_system_T));
	gpGponPriv->gponCfg.ng2.sysProfVer = NGPON2_PROF_VER_INVALID;
	gpGponPriv->gponCfg.ng2.protectFlag = PROTECT_DISABLE;
}

void swap_work_standby_channel(void)
{
	unchar tmp = 0;
	__u32  tmp32 = 0;

	tmp = gpGponPriv->gponCfg.ng2.workDsChannel;
	gpGponPriv->gponCfg.ng2.workDsChannel = gpGponPriv->gponCfg.ng2.standbyDsChannel;
	gpGponPriv->gponCfg.ng2.standbyDsChannel = tmp;

	tmp = gpGponPriv->gponCfg.ng2.workUsChannel;
	gpGponPriv->gponCfg.ng2.workUsChannel = gpGponPriv->gponCfg.ng2.standbyUsChannel;
	gpGponPriv->gponCfg.ng2.standbyUsChannel = tmp;

	gpGponPriv->gponCfg.ng2.workPonID[3] &= 0xf0;
	gpGponPriv->gponCfg.ng2.workPonID[3] |= (gpGponPriv->gponCfg.ng2.workDsChannel & 0x0f);


	if(gpGponPriv->gponCfg.ng2.workDsChannel < PHY_NGPON2_TX_CHAN_UNKNOWN_CONFIG
		|| gpGponPriv->gponCfg.ng2.workUsChannel < PHY_NGPON2_RX_CHAN_UNKNOWN_CONFIG){

		XPON_PHY_SET_NG2_RX_CHAN(gpGponPriv->gponCfg.ng2.workDsChannel);
		XPON_PHY_SET_NG2_TX_CHAN(gpGponPriv->gponCfg.ng2.workUsChannel);

		PON_MSG((MSG_OAM | MSG_ACT), "swap_work_standby_channel ds/us from %d %d to %d %d\n",
			gpGponPriv->gponCfg.ng2.standbyDsChannel,gpGponPriv->gponCfg.ng2.standbyUsChannel,
			gpGponPriv->gponCfg.ng2.workDsChannel,gpGponPriv->gponCfg.ng2.workUsChannel) ;
	}

	//current channel, set parameter to reg which used for SN report
	tmp32 = unchar_convert_to_u32(gpGponPriv->gponCfg.ng2.workPonID,4);
	IO_SREG(CUR_DS_PON_ID, tmp32) ;
	tmp32 &= 0xfffffff0;
	tmp32 |= gpGponPriv->gponCfg.ng2.workUsChannel;
	IO_SREG(CUR_US_PON_ID, tmp32) ;
}
void forgotten_onu_monitor(void)
{
#ifdef TCSUPPORT_CPU_EN7581
	//REG_CAL_GNT_SIZE_TCONT_EN calGntSizeTcontEn;
	//REG_CAL_GNT_SIZE_CTRL_STS calGntSizeCtrlSts;
	REG_MONI_CTRL_FOR_NOT_GNT monCtrlForNotGnt;

	if(ng2_mon_not_gnt == 0){
		PON_MSG((MSG_DBG), "ng2_to6_disable\n") ;
		return ;
	}

#if 0
	//impelment by CAL_GNT_SIZE_CTRL_STS
	//start channel allocation monitor
	calGntSizeTcontEn.Bits.cal_gnt_size_tcont_en = ng2_zero_tcont_mask;
	calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS);
	calGntSizeCtrlSts.Bits.cal_gnt_size_gtc_num = 8000 * 5;  //5s
	calGntSizeCtrlSts.Bits.cal_gnt_size_start = 1;
	calGntSizeCtrlSts.Bits.cal_gnt_size_auto = 1;

	IO_SREG(CAL_GNT_SIZE_TCONT_EN, calGntSizeTcontEn.Raw);
	IO_SREG(CAL_GNT_SIZE_CTRL_STS, calGntSizeCtrlSts.Raw);

	//start TO6
	GPON_START_TIMER(gpGponPriv->to6_timer,gpGponPriv->gponCfg.to6Timer) ;
#endif

	//implement by Hardware int mon_not_gnt_int
	monCtrlForNotGnt.Raw =  IO_GREG(MONI_CTRL_FOR_NOT_GNT);
	monCtrlForNotGnt.Bits.mon_not_gnt_en = 1;
	monCtrlForNotGnt.Bits.mon_gtc_num_for_not_gnt = 8000 * 10; //10s
	IO_SREG(MONI_CTRL_FOR_NOT_GNT, monCtrlForNotGnt.Raw);

#else
	PON_MSG((MSG_DBG), "forgotten_onu_monitor non-7581 not support!\n") ;
#endif
}

__u32 unchar_convert_to_u32(unchar *tmp,int len)
{
	unchar buf[4] = {0};
	int i = 0;

	if(tmp == NULL || len > 4){
		return 0;
	}

	/* avoid memory alignment error  */
	for(i = 0; i < 4; i++){
		buf[i] = tmp[i];
	}

	return ntohl(*(__u32*)buf);
}

void gpon_set_alarmBit(ALARM_INDEX_T index)
{
	uint tmp = 0;
	PON_MSG(MSG_DBG, "gpon set alarm bit : %d.\n",index);
	tmp = *((uint *)&(gpGponPriv->gponAlarm)) | (1 << index);
	gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
	gpGponPriv->jiffiesClk[index] = jiffies + gpGponPriv->gponAlmKeepTime  * HZ;
}

void gpon_clear_alarmBit(ALARM_INDEX_T index)
{
	uint tmp = 0;
	PON_MSG(MSG_DBG, "gpon clear alarm bit : %d.\n",index);
	if(gpGponPriv->gponAlmKeepTime == 0){
		return;
	}
	tmp = *((uint *)&(gpGponPriv->gponAlarm)) & ~(1 << index);
	gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
}

static int should_alarm_clear_by_clk(ALARM_INDEX_T index){
	int ret = 1;

	switch(index){
		case TF_INDEX:
		case LODS_INDEX:
		case DIS_INDEX:
		case DACT_INDEX:
		case LOS_INDEX:
		case LOF_INDEX:
			ret = 0;
			break;
		default:
			break;
	}

	return ret;
}
void gpon_check_alarm_jiffiesClk(void)
{
	ALARM_INDEX_T index = RDI_INDEX;
	uint tmp = 0;

	if(gpGponPriv->gponAlmKeepTime == 0){
		return;
	}

	PON_MSG(MSG_INT, "GPON ISR: enter checking alarm.\n") ;
	for( index = RDI_INDEX; index < MAX_INDEX; index++){
		if(should_alarm_clear_by_clk(index) == 0)
			continue;
		if(time_after_eq(jiffies,gpGponPriv->jiffiesClk[index])){;
			tmp = *((uint *)&(gpGponPriv->gponAlarm)) & ~(1 << index);
			gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
		}
	}
}

void gpon_reset_alarm_jiffiesClk(void)
{
	ALARM_INDEX_T index = RDI_INDEX;
	uint tmp = *(uint *)&gpGponPriv->gponAlarm;
	for( index = RDI_INDEX; index < MAX_INDEX; index++){
		if(tmp &  1 << index)
			gpGponPriv->jiffiesClk[index] = jiffies + gpGponPriv->gponAlmKeepTime  * HZ;
	}
	printk("gpon_reset_alarm_jiffiesClk, gponAlmKeepTime=%x\n",gpGponPriv->gponAlmKeepTime);
}

void gpon_clear_all_alarm(void)
{
	memset(&gpGponPriv->gponAlarm,0,sizeof(gpGponPriv->gponAlarm));
	printk("gpon_clear_all_alarm\n");
}

