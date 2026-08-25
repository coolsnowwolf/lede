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
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/types.h>
#include "common/xpon_global.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_security.h"
#include "xpon_event_global.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define NON_09_DEBUG_CAP_MASK  0xff7ff3bf; //bit 23 11 10 6 set 0

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern void gponDevSendPloamMsg(PLOAM_RAW_General_T *pPloamMsg);


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
#ifdef CONFIG_USE_FOR_TEST
uint eqdDbgFlag = 0;
uint EqdCounter = 0;
uint EqdArray[1001] = {0};
#endif /* CONFIG_USE_FOR_TEST */
extern int snSendInO23Cnt;

#ifdef TCSUPPORT_AUTOBENCH
int gpon_slt_test =0;
EXPORT_SYMBOL(gpon_slt_test);
#endif

extern struct tasklet_struct clear_channel_task;


/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**	function name
**		ploam_send_serial_number_msg
**	description:
**		handle upstream serial number ploam message
**	parameters:
**		none
**	global:
**		gpGponPriv.
**	return:
**		none
**	call:
**		gponDevSendPloamMsg.	
**	revision:
**		v1.0
**____________________________________________________________________________*/
void ploam_send_serial_number_msg(void)
{
	PLOAM_RAW_Serial_Number_T regMsg;
	REG_RDM_DLY randomDelay;
    uint delay =0;
	__u32 tmp = 0;
    
	memset(&regMsg, 0x0, sizeof(PLOAM_RAW_Serial_Number_T));
	
	regMsg.raw.key_index = GPON_PLOAM_IK_IDX1;
	regMsg.raw.dest_id[PLOAM_DEST_ID_HIGHER] = (unchar)(PLOAM_UNASSIGNED_ADDR >> 8); /*unassigned ONU-ID in PLOAMu*/
	regMsg.raw.dest_id[PLOAM_DEST_ID_LOWER] = (unchar)(PLOAM_UNASSIGNED_ADDR );
	regMsg.raw.msg_id = PLOAM_UP_MSG_SERIAL_NUMBER ;
	regMsg.raw.seq_no = 0;
	memcpy(&regMsg.value[2], gpGponPriv->gponCfg.sn, GPON_SN_LENS);
	/* set the hw random delay */    
	randomDelay.Raw = IO_GREG(RDM_DLY);
    randomDelay.Bits.max_rdm_dly =(randomDelay.Bits.max_rdm_dly|0x800);
    /* max_rdm_dly unit is word, need to transform to bit unit*/
    delay = ((randomDelay.Bits.max_rdm_dly&0x7ff)*32*8);
    IO_SREG(RDM_DLY, randomDelay.Raw) ;
    regMsg.raw.random_delay[0]= (unchar)(delay>>24);
    regMsg.raw.random_delay[1]= (unchar)(delay>>16);
    regMsg.raw.random_delay[2]= (unchar)(delay>>8);    
    regMsg.raw.random_delay[3]= (unchar)(delay);
      
	if(NGPON2_MODE){
		tmp = IO_GREG(CUR_DS_PON_ID);
		regMsg.raw.cur_ds_pon_id[0] = (unchar)(tmp>>24);
		regMsg.raw.cur_ds_pon_id[1] = (unchar)(tmp>>16);
		regMsg.raw.cur_ds_pon_id[2] = (unchar)(tmp>>8);
		regMsg.raw.cur_ds_pon_id[3] = (unchar)(tmp);
		tmp = IO_GREG(CUR_US_PON_ID);
		regMsg.raw.cur_us_pon_id[0] = (unchar)(tmp>>24);
		regMsg.raw.cur_us_pon_id[1] = (unchar)(tmp>>16);
		regMsg.raw.cur_us_pon_id[2] = (unchar)(tmp>>8);
		regMsg.raw.cur_us_pon_id[3] = (unchar)(tmp);
		regMsg.raw.us_line_rate_cap = (unchar)(IO_GREG(US_RATE_CAP) & 0x0f);
	}else if(XMCS_IF_WAN_DETECT_MODE_XGSPON == gpPonSysData->sysPonMode || XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G == gpPonSysData->sysPonMode){
		regMsg.raw.us_line_rate_cap = 0x03;
	}else{
		regMsg.raw.us_line_rate_cap = 0x00;
	}
	
	/* Set the MAC registedr to send PLOAM message */
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&regMsg);    
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Send serial number ploam message:",regMsg.value,PLOAM_UP_MSG_LENGTH);
	gpGponPriv->usPloamCounter[PLOAM_UP_MSG_SERIAL_NUMBER] ++;
}
/*______________________________________________________________________________
**	function name
**		ploam_send_registration_msg
**	description:
**		handle upstream registration ploam message 
**	parameters:
**		seqNo: same as downstream seq_no or 0
**		regId: registraton ID of ONU.	
**	global:
**		gpGponPriv.
**	return:
**		none
**	call:
**		gponDevSendPloamMsg.	
**	revision:
**		v1.0
**____________________________________________________________________________*/
void ploam_send_registration_msg(unchar seqNo, unchar *regId)
{
	PLOAM_RAW_Registration_T regMsg ;
	
	memset(&regMsg, 0x0, sizeof(PLOAM_RAW_Registration_T)) ;
    /*mac will detect the registration ploam, and then change the ploamIk to index 0;So the key_index must be the same with index 0*/
	regMsg.raw.key_index = GPON_PLOAM_IK_IDX0;
	regMsg.raw.dest_id[PLOAM_DEST_ID_HIGHER] = (unchar)(gpGponPriv->gponCfg.onu_id >>8);
	regMsg.raw.dest_id[PLOAM_DEST_ID_LOWER] = (unchar)(gpGponPriv->gponCfg.onu_id);
	regMsg.raw.msg_id = PLOAM_UP_MSG_REGISTRATION ;
	regMsg.raw.seq_no = seqNo ;
	memcpy(regMsg.raw.registration_id, regId, GPON_REG_ID_LENS);
	/* Set the MAC register to send PLOAM message */
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&regMsg) ;
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Send registration ploam message:", regMsg.value,PLOAM_UP_MSG_LENGTH) ;
	gpGponPriv->usPloamCounter[PLOAM_UP_MSG_REGISTRATION] ++;
}
/*______________________________________________________________________________
**	function name
**		ploam_send_key_report_msg
**	description:
**		handle upstream key report  ploam message
**	parameters:
**		seqNo: same as downstream seq_no or 0
**		type: new key or existing key
**		index: key index
**		num: key fragment number
**		key: key fragment	
**	global:
**		gpGponPriv.
**	return:
**		none
**	call:
**		gponDevSendPloamMsg.	
**	revision:
**		v1.0
**____________________________________________________________________________*/
void ploam_send_key_report_msg(unchar seqNo, unchar type, unchar index, unchar num, unchar *key, unchar keyLen)
{
	PLOAM_RAW_Key_Report_T keyReportMsg ;
	
	memset(&keyReportMsg, 0x0, sizeof(PLOAM_RAW_Key_Report_T));
	keyReportMsg.raw.key_index = gpGponPriv->gponSecurity.ploamIkIdx ;
	keyReportMsg.raw.dest_id[PLOAM_DEST_ID_HIGHER] = (unchar)(gpGponPriv->gponCfg.onu_id >>8);
	keyReportMsg.raw.dest_id[PLOAM_DEST_ID_LOWER] = (unchar)(gpGponPriv->gponCfg.onu_id);
	keyReportMsg.raw.msg_id = PLOAM_UP_MSG_KEY_REPORT ;
	keyReportMsg.raw.seq_no = seqNo ;
	keyReportMsg.raw.report_type = type ;
	keyReportMsg.raw.report_key_index = index ;
	keyReportMsg.raw.fragment_num = num ;
	if(keyLen <= PLOAM_KEY_FRAGMENT_LEN){
		memcpy(keyReportMsg.raw.key_fragment, key, keyLen);
    }else{
        PON_MSG((MSG_OAM|MSG_SECUR), "PLOAM: key length of key report is not right\n");
        return;
    }
	
	/* Set the MAC registedr to send PLOAM message */
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&keyReportMsg);	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Send key report  ploam message:", keyReportMsg.value,PLOAM_UP_MSG_LENGTH);
	gpGponPriv->usPloamCounter[PLOAM_UP_MSG_KEY_REPORT] ++;
}
/*______________________________________________________________________________
**	function name
**		ploam_send_acknowledge_msg
**	description:
**		handle upstream Acknowledgement  ploam message
**	parameters:
**		seqNo: same as downstream seq_no or 0
**		completionCode: value of the completion code
**	global:
**		gpGponPriv.
**	return:
**		none
**	call:
**		gponDevSendPloamMsg.	
**	revision:
**		v1.0
**____________________________________________________________________________*/

void ploam_send_acknowledge_msg(unchar seqNo, unchar completionCode)
{
	PLOAM_RAW_Acknowledgment_T ackMsg ;
	
	memset(&ackMsg, 0x0, sizeof(PLOAM_RAW_Acknowledgment_T)) ;
	ackMsg.raw.key_index = gpGponPriv->gponSecurity.ploamIkIdx ;
	ackMsg.raw.dest_id[PLOAM_DEST_ID_HIGHER] = (unchar)(gpGponPriv->gponCfg.onu_id >>8) ;
	ackMsg.raw.dest_id[PLOAM_DEST_ID_LOWER] = (unchar)(gpGponPriv->gponCfg.onu_id) ;
	ackMsg.raw.msg_id = PLOAM_UP_MSG_ACKNOWLEDGE ;
	ackMsg.raw.seq_no = seqNo ;
	ackMsg.raw.completion_code = completionCode ;
	
	/* Set the MAC registedr to send PLOAM message */
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&ackMsg) ;	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Send acknowledge ploam message:", ackMsg.value,PLOAM_UP_MSG_LENGTH) ;
	gpGponPriv->usPloamCounter[PLOAM_UP_MSG_ACKNOWLEDGE] ++;
}
void ploam_send_tuning_resp_msg(unchar seqNo, unchar operCode,uint respCode)
{
	PLOAM_RAW_Tuning_Resp_T tuningResMsg;
	memset(&tuningResMsg, 0x0, sizeof(PLOAM_RAW_Tuning_Resp_T)) ;
	if(ng2_tun_resp_key == 0){
		tuningResMsg.raw.key_index = GPON_PLOAM_IK_IDX0;
	}
	else if(ng2_tun_resp_key == 1){
		tuningResMsg.raw.key_index = GPON_PLOAM_IK_IDX1;
	}
	else{
		tuningResMsg.raw.key_index = gpGponPriv->gponSecurity.ploamIkIdx;
	}
	//tuningResMsg.raw.key_index = GPON_PLOAM_IK_IDX1; //xuwf must use default ploam key
	tuningResMsg.raw.dest_id[PLOAM_DEST_ID_HIGHER] = (unchar)(gpGponPriv->gponCfg.onu_id >>8) ;
	tuningResMsg.raw.dest_id[PLOAM_DEST_ID_LOWER] = (unchar)(gpGponPriv->gponCfg.onu_id) ;
	tuningResMsg.raw.msg_id = PLOAM_UP_TUNING_RESPONSE ;
	tuningResMsg.raw.seq_no = seqNo ;
	tuningResMsg.raw.operCode = operCode ;
	tuningResMsg.raw.respCode[0] = (unchar)((respCode >> 8) & 0x000000ff);
	tuningResMsg.raw.respCode[1] = (unchar)(respCode & 0x000000ff);
	memcpy(tuningResMsg.raw.vendorID, gpGponPriv->gponCfg.sn, 4);
	memcpy(tuningResMsg.raw.vssn, &gpGponPriv->gponCfg.sn[4], 4);
#if 0
	tuningResMsg.raw.ponID[0]  = gpGponPriv->gponCfg.ng2.workPonID[0];
	tuningResMsg.raw.ponID[1]  = gpGponPriv->gponCfg.ng2.workPonID[1];
	tuningResMsg.raw.ponID[2]  = gpGponPriv->gponCfg.ng2.workPonID[2];
	tuningResMsg.raw.ponID[3]  = gpGponPriv->gponCfg.ng2.workPonID[3];
	tuningResMsg.raw.uwlchID  = gpGponPriv->gponCfg.ng2.workUsChannel;
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G)
		tuningResMsg.raw.usLineRateCap = 0x02;
	else
		tuningResMsg.raw.usLineRateCap = 0x01;
#endif
	gponDevSendPloamMsg((PLOAM_RAW_General_T *)&tuningResMsg) ;
	PON_MSG(MSG_ACT,"PLOAM: Send tuning response ploam message: operCode=%d\n", operCode) ;
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Send tuning response ploam message:", tuningResMsg.value,PLOAM_UP_MSG_LENGTH) ;
	gpGponPriv->usPloamCounter[PLOAM_UP_TUNING_RESPONSE] ++;
}

int ploam_recv_asb_priv_msg(PLOAM_RAW_General_T *pGenPloamMsg)
{	
	unsigned char ploam_msg[XMCS_MAX_BUFFER_LEN] = {0};
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive 0x77 ploam message:", pGenPloamMsg->value,PLOAM_DOWN_MSG_LENGTH) ;  
	memcpy(ploam_msg,pGenPloamMsg,sizeof(PLOAM_RAW_General_T));	
	xmcs_report_event_buf(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_PRIV_PLOAM, 0,ploam_msg) ;
	return 0;
}

/*______________________________________________________________________________
**	function name
**		ploam_parser_down_message
**	description:
**		parset down raw ploam message and call different handler by msgId
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		none.	
**	revision:
**		v1.0
**____________________________________________________________________________*/
int ploam_parser_down_message(PLOAM_RAW_General_T *pGenPloamMsg)
{
	int ret = 0 ;
	uint msgId = pGenPloamMsg->raw.msg_id ;
	ploam_recv_handler_t ploam_recv_handler ;
	uint msgDestId =((pGenPloamMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pGenPloamMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	
	if(msgId == 0x77 && msgDestId == GPON_ONU_ID)

	{
		ploam_recv_asb_priv_msg(pGenPloamMsg);
		return 0;
	}

	if(msgId >= PLOAM_DOWN_MAX_TYPE || gpGponPriv->ploamRecvHandler[msgId] == NULL) {
		GPON_PLOAM_MSG_RAW(MSG_ERR,"PLOAM: Receive unknow message ploam message:", pGenPloamMsg->value,PLOAM_DOWN_MSG_LENGTH);
		gpon_set_alarmBit(MEM_INDEX);
		return -1 ;
	}
	gpGponPriv->dsPloamCounter[msgId] ++;
	
	ploam_recv_handler = gpGponPriv->ploamRecvHandler[msgId] ;
	ret = ploam_recv_handler(pGenPloamMsg) ;

	return ret ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_profile
**	description:
**		handle downstream Profile ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gponDevSetProfileInfo.	
**		gponDevSetPonTag
**		gponDevSetPhyProfile
**		ploam_send_acknowledge_msg
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_profile(PLOAM_RAW_Profile_T *pRecvProfMsg)
{	
	uint msgDestId =0;
	uint profLens =0;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive profile ploam message:", pRecvProfMsg->value,PLOAM_DOWN_MSG_LENGTH) ;
	
	msgDestId =((pRecvProfMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pRecvProfMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if((GPON_CURR_STATE == GPON_10G_STATE_O2_3) || (GPON_CURR_STATE == GPON_10G_STATE_O4) || (GPON_CURR_STATE == GPON_10G_STATE_O5) ) {
		if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGPON){
			if((GPON_CURR_STATE == GPON_10G_STATE_O2_3) && (msgDestId != PLOAM_BROADCAST_ADDR)) {
				PON_MSG(MSG_OAM, "PLOAM: Receive unicast profile ploam message in O2_3\n") ;
				return -1 ;
			}
			if(pRecvProfMsg->raw.line_rate == PLOAM_XGPON_LINE_RATE){
				PON_MSG(MSG_OAM, "PLOAM: parsing XGPON profiles\n");
				pRecvProfMsg->raw.preamble_repeat_cnt = pRecvProfMsg->raw.preamble_repeat_cnt & XGPON_PREAMBLE_PEPEATE_MASK;
			}else{
				PON_MSG(MSG_OAM, "PLOAM: discard XGSPON profiles\n");
				return  -1;
			}
		}else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON  ||
			gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			if((GPON_CURR_STATE == GPON_10G_STATE_O2_3) && (msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR_NOKIA)) {
				PON_MSG(MSG_OAM, "PLOAM: Receive unicast profile ploam message in O2_3\n") ;
				return -1 ;
			}
			if(pRecvProfMsg->raw.line_rate == PLOAM_XGSPON_LINE_RATE){
				PON_MSG(MSG_OAM, "PLOAM: parsing XGSPON profiles\n");
				pRecvProfMsg->raw.preamble_repeat_cnt = pRecvProfMsg->raw.preamble_repeat_cnt & XGSPON_PREAMBLE_PEPEATE_MASK;
			}else{
				PON_MSG(MSG_OAM, "PLOAM: discard XGPON profiles\n");
				return -1;
			}
		}else{
            PON_MSG(MSG_OAM, "PLOAM: wrong pon mode type\n");
            return -1;
		}
		/*set each profile valid and lenght to mac*/

		if(NGPON2_MODE){
			//record prof, only set reg while pon-id matched
			memcpy(&gpGponPriv->gponCfg.ng2.burstProf[pRecvProfMsg->raw.prof_index],pRecvProfMsg,sizeof(PLOAM_RAW_Profile_T));
			if(pRecvProfMsg->raw.cross && (pRecvProfMsg->raw.ds_pon_id[3] != gpGponPriv->gponCfg.ng2.workPonID[3])){
				if((msgDestId != PLOAM_BROADCAST_ADDR)&&(msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
					ploam_send_acknowledge_msg(pRecvProfMsg->raw.seq_no,XGPON_PLOAM_ACK_OK) ; /*send Ack in unicast */
				}
				return 0;
			}
		}
		profLens = (pRecvProfMsg->raw.preamble_lens * pRecvProfMsg->raw.preamble_repeat_cnt)
				+ pRecvProfMsg->raw.delimiter_lens ;
		gponDevSetProfileInfo(pRecvProfMsg->raw.prof_index, profLens, pRecvProfMsg->raw.prof_version);

		/*record by SW*/
		gpGponPriv->gponCfg.profVersion = pRecvProfMsg->raw.prof_version ;
		memcpy(gpGponPriv->gponCfg.ponTag, pRecvProfMsg->raw.pon_tag, GPON_TAG_LENS);
		/*set the pon tag to mac*/
		gponDevSetPonTag(pRecvProfMsg->raw.pon_tag);
		
		if(GPON_CURR_STATE == GPON_10G_STATE_O2_3)
			tasklet_hi_schedule(&gpGponPriv->securityKey_task);

		/*set preamble,delimiter,PSBu len information prmb rpt times,prmb len,dlmt len,fec value to  PHY */
		gponDevSetPhyProfile(pRecvProfMsg);

        if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGPON){
			if(msgDestId != PLOAM_BROADCAST_ADDR) {
				ploam_send_acknowledge_msg(pRecvProfMsg->raw.seq_no,XGPON_PLOAM_ACK_OK) ; /*send Ack in unicast */
			}else
			    return 0 ;
			
		}else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON ||
			gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G){
			if((msgDestId != PLOAM_BROADCAST_ADDR)&&(msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
				ploam_send_acknowledge_msg(pRecvProfMsg->raw.seq_no,XGPON_PLOAM_ACK_OK) ; /*send Ack in unicast */
			}else
			    return 0 ;
		}else{
			PON_MSG(MSG_OAM, "PLOAM: ignore profile ploam in pon mode:%d \n",gpPonSysData->sysPonMode) ;
		}
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_assign_onu_id
**	description:
**		handle downstream Assign_ONU-ID ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gponDevSetOnuId.	
**		gwan_create_new_gemport
**		gpon_act_change_state
**		gwan_remove_all_tcont
**		gwan_remove_all_gemport_for_disable
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_assign_onu_id(PLOAM_RAW_Assign_OnuID_T *pAssignOnuIdMsg)
{	
	uint msgDestId = 0;
	uint msgAssignOnuId = 0;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive Assign_ONU_ID message:", pAssignOnuIdMsg->value,PLOAM_DOWN_MSG_LENGTH);	
	
	msgDestId = ((pAssignOnuIdMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pAssignOnuIdMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	msgAssignOnuId = ((pAssignOnuIdMsg->raw.onu_id_m <<8) | (pAssignOnuIdMsg->raw.onu_id_l)) ;
	if((msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: The dest. address(%.4x) of PLOAM message is incorrect.\n", msgDestId) ;
		return -1 ;
	}
		
	/* Compare the serial number of system and Assign_ONU-ID PLOAM message */
	if(memcmp(gpGponPriv->gponCfg.sn, pAssignOnuIdMsg->raw.sn, GPON_SN_LENS)) {
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: The serial number of PLOAM message is incorrect\n") ;
		return -1 ;
	}

	if(GPON_CURR_STATE == GPON_10G_STATE_O2_3) {
		/* Store the latest ONU ID*/
		gpGponPriv->gponCfg.onu_id = msgAssignOnuId ;
		gpGponPriv->gponCfg.omcc = gpGponPriv->gponCfg.onu_id ;
		gpWanPriv->gpon.allocId[0] = gpGponPriv->gponCfg.onu_id ;
		gpWanPriv->activeChannelNum = 1;
		/* Setting the ONU ID to MAC register */
		gponDevSetOnuId(msgAssignOnuId, GPON_ONU_ID_VALID) ;
        gwan_create_new_gemport(msgAssignOnuId,0,GPON_UNICAST_GEM,msgAssignOnuId);
		PON_MSG(MSG_OAM, "PLOAM: set ONU_ID:%d in O2_3 \n",msgAssignOnuId) ;
		/* Change the current state */
		gpon_act_change_state(GPON_10G_STATE_O4) ;
        if(gpGponPriv->gponCfg.ploamCtrl == XGPON_SW)
            tasklet_hi_schedule(&gpGponPriv->swreplyploam_task);
        else{
            /*Do nothing*/
    }
    }else if((GPON_CURR_STATE == GPON_10G_STATE_O4) || (GPON_CURR_STATE == GPON_10G_STATE_O5)){
		if(gpGponPriv->gponCfg.onu_id == msgAssignOnuId ){
			PON_MSG(MSG_OAM, "PLOAM: ignore consistent ONU-ID setting in O4/O5 \n") ;
		}else{
			/* discard ONU-ID,default OMCI, XGEM port id*/
			gpGponPriv->gponCfg.eqd = 0 ;
			gpGponPriv->gponCfg.onu_id = GPON_UNASSIGN_ONU_ID ;
			gpGponPriv->gponCfg.omcc = GPON_UNASSIGN_ONU_ID ;
			gwan_remove_all_tcont() ;
			gwan_remove_all_gemport_for_disable();
			/* discard burst porfiles ...
			*/	
			
			PON_MSG(MSG_OAM, "PLOAM: receive inconsistent ONU-ID setting in O4/O5 \n") ;
			/* Change the current state */
			gpon_act_change_state(GPON_10G_STATE_O1) ;
		}
	}else{
	    PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: Receive Assign_ONU_ID in wrong state\n") ;
		return -1 ;
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_ranging_time
**	description:
**		handle downstream Ranging_Time ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gponDevSetEqdValue.	
**		gpon_act_change_state
**		ploam_send_acknowledge_msg
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_ranging_time(PLOAM_RAW_Ranging_Time_T *pRangingMsg)
{
	uint msgDestId = 0 ;
	uint newEqd = 0;
#if defined(TCSUPPORT_CPU_AN7583)
	uint EqdDiff = 0;
#endif
	int ret=0;
	REG_DBG_CAP_SETTING gponCapSet ;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive Ranging Time message:", pRangingMsg->value,PLOAM_DOWN_MSG_LENGTH) ;	
	msgDestId =((pRangingMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pRangingMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	newEqd  = ((pRangingMsg->raw.eqd_value[0]<<24) 
				| (pRangingMsg->raw.eqd_value[1]<<16) 
				| (pRangingMsg->raw.eqd_value[2]<<8) 
				| (pRangingMsg->raw.eqd_value[3]));	
#ifdef CONFIG_USE_FOR_TEST
	if(eqdDbgFlag){
		EqdCounter++;
		EqdArray[EqdCounter] = newEqd;
	}
#endif /* CONFIG_USE_FOR_TEST */
	if(GPON_CURR_STATE == GPON_10G_STATE_O4) {
		if(msgDestId == GPON_ONU_ID){
			if(pRangingMsg->raw.eqd_mode == PLOAM_EQD_ABSOLUTE){ /*eqd absolute adjust*/
				/*to do stop timer O1*/
				/* Adjust the eqd value for eqd test */
				if(gpGponPriv->gponCfg.eqdO4Offset) {
					if(gpGponPriv->gponCfg.flags.eqdOffsetFlag == GPON_EQD_OFFSET_FLAG_SUBTRACT) {
						newEqd -= gpGponPriv->gponCfg.eqdO4Offset ;
						PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: Adjust the main path EqD in O4, Offset:%x.\n", gpGponPriv->gponCfg.eqdO4Offset) ;
					} else {
						newEqd += gpGponPriv->gponCfg.eqdO4Offset ;
						PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: Adjust the main path EqD in O4, Offset:%x.\n", gpGponPriv->gponCfg.eqdO4Offset) ;
					}
				}else{
                    PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: no EqD offset in O4\n") ;
				}
				/* Setting the EqD to MAC register */
#if !defined(TCSUPPORT_CPU_AN7583)
				gpGponPriv->gponCfg.eqd_olt_absolute = newEqd;
				gponDevRefreshEqdValue();
				PON_MSG((MSG_OAM| MSG_EQD), "PLOAM: set directed ranging time in O4, absolute eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd_olt_absolute) ;	
#else

				ret = gponDevSetEqdValue(newEqd,&gpGponPriv->gponCfg.eqd, gpPonSysData->sysPonMode,pRangingMsg->raw.eqd_mode,pRangingMsg->raw.eqd_adjust);
				if(ret != 0){
					PON_MSG((MSG_ERR|MSG_EQD), "PLOAM: set directed ranging time in O4 fail \n") ;
                    return -1 ;
                }
				PON_MSG((MSG_OAM| MSG_EQD), "PLOAM: set directed ranging time in O4, absolute eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd) ;				
#endif
				snSendInO23Cnt=0;
				if(ng2_o4_to_09 == 1){
					gponCapSet.Raw = IO_GREG(DBG_CAP_SETTING) ;
					gponCapSet.Raw &= NON_09_DEBUG_CAP_MASK;
					IO_SREG(DBG_CAP_SETTING, gponCapSet.Raw) ;
					gpon_act_change_state(GPON_10G_STATE_O9) ;
				}else{
				gpon_act_change_state(GPON_10G_STATE_O5) ;                
				}
                gpGponPriv->gponCfg.rangingAckSeqNo = pRangingMsg->raw.seq_no;
                tasklet_hi_schedule(&gpGponPriv->rangingAck_task);
				start_omci_oam_monitor();
                xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ACTIVATE, 0) ;
			}
			else{ /*eqd relative adjust*/
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: ignore directed relative ranging time set in O4 \n") ;
			}
		}
		else{
			PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: ignore broadcast ranging time set in O4 \n") ;
		}	
	}else if(GPON_CURR_STATE == GPON_10G_STATE_O5 ||  GPON_CURR_STATE == GPON_10G_STATE_O9){
		if(pRangingMsg->raw.eqd_mode == PLOAM_EQD_ABSOLUTE){ /*eqd absolute adjust*/
			if(msgDestId == GPON_ONU_ID){
#if !defined(TCSUPPORT_CPU_AN7583)
				gpGponPriv->gponCfg.eqd_olt_absolute = newEqd;
				ret = gponDevSetEqdValue(gpGponPriv->gponCfg.eqd_olt_absolute, gpGponPriv->gponCfg.eqd_olt_init, gpPonSysData->sysPonMode);
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: get directed absolute ranging time from olt just in O5, eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd_olt_absolute) ;
				PON_MSG((MSG_OAM|MSG_EQD), "print the eqd_olt_init of the current record, eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd_olt_init);
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: set absolute ranging time just in O5 for reg, eqd:0x%.8X \n",IO_GREG(EQD)) ;
#else				
				if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON) {
					if(gpGponPriv->gponCfg.eqd > newEqd*4)
						EqdDiff = gpGponPriv->gponCfg.eqd - newEqd*4;
					else
						EqdDiff = newEqd*4 - gpGponPriv->gponCfg.eqd;
				} else {
					if(gpGponPriv->gponCfg.eqd > newEqd)
						EqdDiff = gpGponPriv->gponCfg.eqd - newEqd;
					else
						EqdDiff = newEqd - gpGponPriv->gponCfg.eqd;
				}
				/* Setting the EqD to MAC register */
				ret =gponDevSetEqdValue(newEqd,&gpGponPriv->gponCfg.eqd,gpPonSysData->sysPonMode,pRangingMsg->raw.eqd_mode,pRangingMsg->raw.eqd_adjust);
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: set directed absolute ranging time in O5, eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd) ;

				if(EqdDiff > EQD_DIFF_THRESHOLD ){
					//resync Tx
					gponDevMpiTxStop(XPON_RESET_HOLD_ON);
					gponDevSwResyncCompleteProcess();
					gponDevMpiTxStop(XPON_RESET_RELEASE);
					PON_MSG((MSG_ACT|MSG_EQD), "PLOAM: cur state %d, receive EqdDiff %d, eqd:%d, Resync Tx\n",GPON_CURR_STATE,EqdDiff,newEqd) ;	
				}
#endif
				if(ret != 0){
					PON_MSG((MSG_ERR|MSG_EQD), "PLOAM: set absolute ranging time in O5 fail \n") ;
                    return -1 ;
                }

				if( newEqd != 0  && GPON_CURR_STATE == GPON_10G_STATE_O5){
				ploam_send_acknowledge_msg(pRangingMsg->raw.seq_no,XGPON_PLOAM_ACK_OK); /*send Ack */
				}
			}else{
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: ignore broadcast absolute ranging time in O5 \n") ;
			}
		}else{  /*eqd relative adjust*/
			if((msgDestId == GPON_ONU_ID) || (msgDestId == PLOAM_BROADCAST_ADDR)|| (msgDestId == PLOAM_XGSPON_BROADCAST_ADDR)){
#if !defined(TCSUPPORT_CPU_AN7583)
				if(pRangingMsg->raw.eqd_adjust == PLOAM_EQD_POSITIVE){
					gpGponPriv->gponCfg.eqd_olt_absolute = gpGponPriv->gponCfg.eqd_olt_absolute + newEqd;
				} else if(pRangingMsg->raw.eqd_adjust == PLOAM_EQD_NEGATIVE) {
					gpGponPriv->gponCfg.eqd_olt_absolute = gpGponPriv->gponCfg.eqd_olt_absolute - newEqd;
				}
				
				ret = gponDevSetEqdValue(gpGponPriv->gponCfg.eqd_olt_absolute, gpGponPriv->gponCfg.eqd_olt_init, gpPonSysData->sysPonMode);
				if(ret != 0){
					PON_MSG((MSG_ERR|MSG_EQD), "PLOAM: set relative ranging time in O5 fail \n") ;
                    return -1 ;
                }
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: set relative ranging time in O5,converted to absolute eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd_olt_absolute) ;
				PON_MSG((MSG_OAM|MSG_EQD), "print the eqd_olt_init of the current record, eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd_olt_init);
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: set absolute ranging time just in O5 for reg, eqd:0x%.8X \n",IO_GREG(EQD)) ;
			
#else				
				ret = gponDevSetEqdValue(newEqd,&gpGponPriv->gponCfg.eqd,gpPonSysData->sysPonMode,pRangingMsg->raw.eqd_mode,pRangingMsg->raw.eqd_adjust);
				if(ret != 0){
					PON_MSG((MSG_ERR|MSG_EQD), "PLOAM: set relative ranging time in O5 fail \n") ;
                    return -1 ;
                }
				PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: set relative ranging time in O5, eqd:0x%.8X \n",gpGponPriv->gponCfg.eqd) ;
#endif			
				if( newEqd != 0  && msgDestId == GPON_ONU_ID && GPON_CURR_STATE == GPON_10G_STATE_O5){
					ploam_send_acknowledge_msg(pRangingMsg->raw.seq_no,XGPON_PLOAM_ACK_OK); /*send Ack */
				}
			}
		}
	}else{	
        PON_MSG((MSG_OAM|MSG_EQD), "PLOAM: ignore ranging time in state O:%d \n",GPON_CURR_STATE) ;
        return -1 ;
	}
#ifdef TCSUPPORT_AUTOBENCH
	if(gpon_slt_test!=1){
		printk("xg(s)pon O5 pass\n");
		gpon_slt_test = 1;
	}
#endif
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_deactivate_onu
**	description:
**		handle downstream Deactivate_ONU-ID ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gpon_disable.	
**		xmcs_report_event
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_deactivate_onu(PLOAM_RAW_Deactivate_OnuID_T *pDeactMsg)
{
	uint msgDestId = 0;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive deactivate onu ploam message:", pDeactMsg->value,PLOAM_DOWN_MSG_LENGTH);
	msgDestId =((pDeactMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pDeactMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if((msgDestId != GPON_ONU_ID) 
		&& (msgDestId != PLOAM_BROADCAST_ADDR)
		&& (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.4x) of PLOAM message is incorrect.\n",msgDestId) ;
		return -1 ;
	}
	if(ng2_ignore_disable == 1){
		printk("ingore deactive!\n");
		return 0;
	}
	if((GPON_CURR_STATE == GPON_10G_STATE_O2_3) 
		|| (GPON_CURR_STATE == GPON_10G_STATE_O4)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O5)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O5_2)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O8)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O9)){
		if(isEN7581 || isAN7583){
			gpon_disable(GPON_MAC_PLAIN_RESET) ; 
		}else{
			gpon_disable(GPON_MAC_WITH_PHY_RESET) ; 
		}	
        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVNET_GPON_DEACTIVATE, 0) ;
		gpon_set_alarmBit(DACT_INDEX);
		stop_omci_oam_monitor() ;
	}else{
        /*Do nothing*/
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_disable_serial_number
**	description:
**		handle downstream recv_disable_serial_number ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gpon_disable.	
**		XPON_PHY_TX_ENABLE
**		xmcs_report_event
**	revision:
**		v1.0
**____________________________________________________________________________*/
int ploam_recv_disable_serial_number(PLOAM_RAW_Disable_SN_T *pDisSnMsg)
{

	uint msgDestId = 0;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive disable SN ploam message:", pDisSnMsg->value,PLOAM_DOWN_MSG_LENGTH);
	
	msgDestId =((pDisSnMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pDisSnMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;	
	if((msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.4x) of PLOAM message is incorrect.\n", msgDestId) ;
		return -1 ;
	}

	if(ng2_ignore_disable == 1){
		printk("ingore disable!\n");
		return 0;
	}
	if(GPON_CURR_STATE == GPON_10G_STATE_O7) {
		if((pDisSnMsg->raw.mode == PLAOM_DISABLE_ALLOWED_ALL) || 
		  ((pDisSnMsg->raw.mode == PLAOM_DISABLE_ALLOWED_SPECIFIC) && !memcmp(gpGponPriv->gponCfg.sn, pDisSnMsg->raw.sn, 8))) {
		  
		   	PON_MSG((MSG_OAM), "SN enable mode:%x\n",pDisSnMsg->raw.mode) ;
			gpGponPriv->emergencyState = 0;
            /*XPON_PHY_TX_ENABLE();*/
			gpon_disable(GPON_MAC_PLAIN_RESET);
			XPON_PHY_TX_ENABLE();
            xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ENABLE, 0) ;
		}else{
            PON_MSG((MSG_OAM), "PLOAM: ignore allowed SN ploam in state O:%d \n",GPON_CURR_STATE) ;
		}
		gpon_clear_alarmBit(DIS_INDEX);
	}else if((GPON_CURR_STATE == GPON_10G_STATE_O2_3)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O4) 
		|| (GPON_CURR_STATE == GPON_10G_STATE_O5)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O8)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O9)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O5_2)) {
		if((pDisSnMsg->raw.mode == PLOAM_DISABLE_DENIED_ALL) || 
	      ((pDisSnMsg->raw.mode == PLOAM_DISABLE_DENIED_SPECIFIC) && !memcmp(gpGponPriv->gponCfg.sn, pDisSnMsg->raw.sn, 8))) {

			PON_MSG((MSG_OAM), "SN disable mode:%x\n",pDisSnMsg->raw.mode) ;
			/* Change the current state */			
			gpon_act_change_state(GPON_10G_STATE_O7) ;
			gpGponPriv->emergencyState = 1 ;
			gpon_set_alarmBit(DIS_INDEX);
	        /*XPON_PHY_TX_DISABLE(); */
			XPON_PHY_TX_DISABLE();
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_DISABLE, 0) ;
		}
		else if((pDisSnMsg->raw.mode == PLOAM_DISABLE_DSICOVERY) && (GPON_CURR_STATE == GPON_10G_STATE_O2_3)){
			if(pDisSnMsg->raw.mode == PLOAM_DISABLE_DSICOVERY){
				//denied new register, if ONU stay in O2-3, keep slient
				PON_MSG((MSG_OAM | MSG_ACT), "SN disable discovery; mode:%x\n",pDisSnMsg->raw.mode) ;
				XPON_PHY_TX_DISABLE();
			}
			else if((pDisSnMsg->raw.mode == PLAOM_DISABLE_ALLOWED_ALL) ||
				((pDisSnMsg->raw.mode == PLAOM_DISABLE_ALLOWED_SPECIFIC) && !memcmp(gpGponPriv->gponCfg.sn, pDisSnMsg->raw.sn, 8))){
				//allow send SN to register
				PON_MSG((MSG_OAM | MSG_ACT), "SN allow discovery; mode:%x\n",pDisSnMsg->raw.mode) ;
				XPON_PHY_TX_ENABLE();
			}
		}else{
            PON_MSG((MSG_OAM), "PLOAM: ignore denied SN ploam in state O:%d \n",GPON_CURR_STATE) ;
		}
	}
	else{
        PON_MSG((MSG_OAM), "PLOAM: ignore disable SN ploam in state O:%d \n",GPON_CURR_STATE) ;
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_request_registration
**	description:
**		handle downstream Request_Registration ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		gpon_disable.	
**		XPON_PHY_TX_ENABLE
**		xmcs_report_event
**		ploam_send_registration_msg
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_request_registration(PLOAM_RAW_Request_Registration_T *pReqRegistMsg)
{
	uint msgDestId = 0;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive request registration ploam message:", pReqRegistMsg->value,PLOAM_DOWN_MSG_LENGTH);
	
	msgDestId =((pReqRegistMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pReqRegistMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;	
	if(msgDestId != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pReqRegistMsg->raw.dest_id[0], pReqRegistMsg->raw.dest_id[1]) ;
		return -1 ;
	}
	if(GPON_CURR_STATE == GPON_10G_STATE_O5 || (ng2_o4_to_09 == 1 && GPON_CURR_STATE == GPON_10G_STATE_O9)) {
		PON_MSG(MSG_OAM, "PLOAM: OLT request registration in O5\n") ;
		if(gpGponPriv->gponSecurity.smaValid == GPON_SMA_INVALID){
            if(gpGponPriv->gponSecurity.registerIDState == GPON_REG_ID_NOT_REPORT){
    			gpGponPriv->gponSecurity.ploamIkIdx = 
                    (gpGponPriv->gponSecurity.ploamIkIdx == GPON_PLOAM_IK_IDX0)? GPON_PLOAM_IK_IDX1: GPON_PLOAM_IK_IDX0;
    			gpGponPriv->gponSecurity.omciIkIdx = 
                    (gpGponPriv->gponSecurity.omciIkIdx == GPON_OMCI_IK_IDX0)? GPON_OMCI_IK_IDX1: GPON_OMCI_IK_IDX0;
                gpGponPriv->gponSecurity.kekIdx = 
                    (gpGponPriv->gponSecurity.kekIdx == GPON_KEK_IK_IDX0)? GPON_KEK_IK_IDX1: GPON_KEK_IK_IDX0;
                gpGponPriv->gponSecurity.registerIDState = GPON_REG_ID_REPORTED;
            }else{
                PON_MSG(MSG_OAM, "PLOAM: request registration has been reported, ignore \n") ;
            }
		}else{
			gpon_ploamIk_index_change_by_OMCI_base_secure(&gpGponPriv->gponSecurity);
		}
		ploam_send_registration_msg(pReqRegistMsg->raw.seq_no,gpGponPriv->gponCfg.reg_id) ;
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_assign_alloc_id
**	description:
**		handle downstream Assign_Alloc_Id ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		ploam_send_acknowledge_msg
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_assign_alloc_id(PLOAM_RAW_Assign_AllocID_T *pAssAllocIdMsg)
{
	uint msgDestId = 0 ;
	ushort allocId = 0;
	int ret = 0;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive assign alloc id ploam message:", pAssAllocIdMsg->value,PLOAM_DOWN_MSG_LENGTH);
	msgDestId =((pAssAllocIdMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pAssAllocIdMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;	
	if(msgDestId != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n",msgDestId) ;
		return -1 ;
	}	 
	if(GPON_CURR_STATE == GPON_10G_STATE_O5 || (ng2_o4_to_09 == 1 && GPON_CURR_STATE == GPON_10G_STATE_O9)) {
		allocId = ((pAssAllocIdMsg->raw.alloc_id_m<<8) | (pAssAllocIdMsg->raw.alloc_id_l)) ;		
		if(allocId != gpGponPriv->gponCfg.onu_id) {
			if(pAssAllocIdMsg->raw.alloc_id_type == PLOAM_ALLOC_ID_ASSIGN){
				PON_MSG(MSG_OAM,"PLOAM: assign alloc id:%x \n", allocId);
				if(FALSE == gwanCheckAllocIdExist(allocId)){
					gpGponPriv->gponCfg.allocIdConfig.allocId = allocId;					
					gpGponPriv->gponCfg.allocIdConfig.allocIdType = PLOAM_ALLOC_ID_ASSIGN;
					//tasklet_hi_schedule(&gpGponPriv->allocId_task);
					ret = gponDevAssignNewAllocId((unsigned long)&(gpGponPriv->gponCfg.allocIdConfig));
#if defined(TCSUPPORT_CPU_AN7583)					
					if(isFPGA &&(0 == ret)) {
						GPON_START_TIMER(gpGponPriv->fe_chn_done_timer,gpGponPriv->gponCfg.feChnSetDoneTimer) ;	
					}
#endif
				}else{
                    PON_MSG((MSG_OAM), "PLOAM: ignore existed assign alloc id \n") ;
				}
			} else if(pAssAllocIdMsg->raw.alloc_id_type == PLOAM_ALLOC_ID_DEALLOCATE) {
				PON_MSG(MSG_OAM,"PLOAM: deallocate alloc id:%x \n", allocId);
				gpGponPriv->gponCfg.allocIdConfig.allocId = allocId;					
				gpGponPriv->gponCfg.allocIdConfig.allocIdType = PLOAM_ALLOC_ID_DEALLOCATE;
				//tasklet_hi_schedule(&gpGponPriv->allocId_task);
				gponDevAssignNewAllocId((unsigned long)&(gpGponPriv->gponCfg.allocIdConfig));
			}else{
				PON_MSG(MSG_ERR,"PLOAM: wrong alloc id type:%x \n", pAssAllocIdMsg->raw.alloc_id_type);
                return -1 ;
			}
		}
		if( 0 == ret ){			
			PON_MSG(MSG_OAM, "PLOAM: OLT Assign ALLOC_ID %d\n", allocId) ;			
			ploam_send_acknowledge_msg(pAssAllocIdMsg->raw.seq_no,XGPON_PLOAM_ACK_OK); 		
		}else{			
			PON_MSG(MSG_OAM, "PLOAM: OLT Assign ALLOC_ID %d error \n", allocId) ;			
			ploam_send_acknowledge_msg(pAssAllocIdMsg->raw.seq_no,XGPON_PLOAM_ACK_PROCES_ERR); 		
		}
		//PON_MSG(MSG_OAM, "PLOAM: OLT Assign ALLOC_ID %d\n", allocId) ;
		//ploam_send_acknowledge_msg(pAssAllocIdMsg->raw.seq_no,XGPON_PLOAM_ACK_OK); 
	}else{
        PON_MSG((MSG_OAM), "PLOAM: ignore assign alloc ploam in state O:%d \n",GPON_CURR_STATE) ;
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_key_control
**	description:
**		handle downstream Key_Control ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_key_control(PLOAM_RAW_Key_Control_T *pKeyControl)
{
	uint msgDestId = 0 ;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive key control ploam message:", pKeyControl->value,PLOAM_DOWN_MSG_LENGTH);

	msgDestId =((pKeyControl->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pKeyControl->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;	
	if((msgDestId != GPON_ONU_ID) && (msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)){
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.4x) of PLOAM message is incorrect.\n",msgDestId) ;
		return -1 ;
	}
	
	if((GPON_CURR_STATE == GPON_10G_STATE_O5)
		|| (GPON_CURR_STATE == GPON_10G_STATE_O9 && ng2_o4_to_09)){
		if(pKeyControl->raw.key_lens == 0){
			PON_MSG(MSG_OAM, "PLOAM: 256 bytes data encryption key is not support\n") ;
			return -1;
		}
        gpGponPriv->gponCfg.keyReport.keyIndex = pKeyControl->raw.key_index ;	
        gpGponPriv->gponCfg.keyReport.keyControl = pKeyControl->raw.control ;
        gpGponPriv->gponCfg.keyReport.seqNo = pKeyControl->raw.seq_no ;     
        tasklet_hi_schedule(&gpGponPriv->keyReport_task);            
	}else{
        PON_MSG((MSG_OAM), "PLOAM: ignore key control ploam in state O:%d \n",GPON_CURR_STATE) ;
	}
	return 0 ;
}
/*______________________________________________________________________________
**	function name
**		ploam_recv_sleep_allow
**	description:
**		handle downstream Sleep_Allow ploam message
**	parameters:
**		message body
**	global:
**		gpGponPriv.
**	return:
**		0:  ok
**		others: failure.
**	call:
**		
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_sleep_allow(PLOAM_RAW_Sleep_Allow_T *pSleepAllow)
{
	uint msgDestId =0 ;
	
	msgDestId =((pSleepAllow->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pSleepAllow->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if((msgDestId != GPON_ONU_ID) && (msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pSleepAllow->raw.dest_id[0],pSleepAllow->raw.dest_id[1]) ;
		return -1 ;
	}

	return 0 ;
}


/*______________________________________________________________________________
**	function name
**		ploam_recv_reboot_onu
**	description:
**		handle downstream ploam_recv_reboot_onu ploam message
**	parameters:
**		message body
**	global:
**		
**	return:
**		0:  ok
**		others: failure.
**	call:
**		
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int ploam_recv_reboot_onu(PLOAM_RAW_Reboot_ONU_T *pRebootOun)
{
	uint msgDestId =0 ;
	XPON_REBOOT_t reboot_event;
	
	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive reboot ONU message:", pRebootOun->value,PLOAM_DOWN_MSG_LENGTH);	
	memset(&reboot_event, 0, sizeof(XPON_REBOOT_t));
	
	msgDestId =((pRebootOun->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pRebootOun->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if((msgDestId != GPON_ONU_ID) && (msgDestId != PLOAM_BROADCAST_ADDR) && (msgDestId != PLOAM_XGSPON_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%.2x) of PLOAM message is incorrect.\n", (unsigned int)(unsigned long)pRebootOun->raw.dest_id) ;
		return -1 ;
	}

	if(pRebootOun->raw.reboot_depth > 0x3){
		PON_MSG((MSG_ERR|MSG_OAM), "The reboot depth:%d is invalid.\n", pRebootOun->raw.reboot_depth) ;
		return -1 ;
	}else{
		reboot_event.depth = pRebootOun->raw.reboot_depth;
	}
	
	if(pRebootOun->raw.reboot_image > 0x1){
		PON_MSG((MSG_ERR|MSG_OAM), "The reboot image:%d is invalid.\n", pRebootOun->raw.reboot_image) ;
		return -1 ;
	}else{
		reboot_event.image = pRebootOun->raw.reboot_image;
	}
	
	if(pRebootOun->raw.onu_state > 0x1){
		PON_MSG((MSG_ERR|MSG_OAM), "The reboot onu state:%d is invalid.\n", pRebootOun->raw.onu_state) ;
		return -1 ;
	}else{
		reboot_event.state = pRebootOun->raw.onu_state;
	}
	
	if(pRebootOun->raw.flags > 0x2){
		PON_MSG((MSG_ERR|MSG_OAM), "The reboot onu flags:%d is invalid.\n", pRebootOun->raw.flags) ;
		return -1 ;
	}else{
		reboot_event.flags = pRebootOun->raw.flags;
	}
	
	PON_MSG((MSG_ERR|MSG_OAM), "xmcs report reboot onu event,depth:%d image:%d state:%d flags:%d\n", 
		reboot_event.depth,reboot_event.image,reboot_event.state,reboot_event.flags) ;

	xmcs_report_event_buf(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_REBOOT_ONU, 0, (unsigned char*)&reboot_event) ;
	
	return 0 ;
}

static int ploam_recv_calibration_request(PLOAM_RAW_General_T *pPloamMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive calibration_request message.(%.8X %.8X %.8X)\n", htonl(pPloamMsg->value[0]), htonl(pPloamMsg->value[1]), htonl(pPloamMsg->value[2])) ;
	return 0 ;
}
static int ploam_recv_adjust_tx_wavelength(PLOAM_RAW_General_T *pPloamMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive adjust_tx_wavelength message.(%.8X %.8X %.8X)\n", htonl(pPloamMsg->value[0]), htonl(pPloamMsg->value[1]), htonl(pPloamMsg->value[2])) ;
	return 0 ;
}
static int ploam_recv_tuning_control(PLOAM_RAW_Tuning_Ctrl_T *pTunCtrlMsg)
{
	uint msgDestId =0 ;
	__u32 tmp = 0;
	__u32 tmp_sfc_lsb16 = 0;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive turn_control profile  message:", pTunCtrlMsg->value,PLOAM_DOWN_MSG_LENGTH) ;
	if(NGPON2_MODE == 0){
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: Receive ploam_recv_tuning_control do nothing in non-NGPON2 mode\n") ;
		return 0;
	}
	msgDestId =((pTunCtrlMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pTunCtrlMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if(0){//(msgDestId != GPON_ONU_ID) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pTunCtrlMsg->raw.dest_id[0],pTunCtrlMsg->raw.dest_id[1]) ;
		return -1 ;
	}
	PON_MSG(MSG_OAM, "PLOAM:Receive ploam_recv_tuning_control operCode=%d\n", pTunCtrlMsg->raw.operCode) ;


	if(pTunCtrlMsg->raw.operCode == TUNING_CTRL_REQUEST){
		PON_MSG((MSG_ACT),"tuing_control REQUEST\n");
		if(GPON_CURR_STATE != GPON_10G_STATE_O5) {
			PON_MSG((MSG_OAM | MSG_ERR), "ploam_recv_tuning_control requset no in O5 state, do nothing\n") ;
			return 0;
		}

		//set Scheuled SFC
		tmp = IO_GREG(DBG_DS_SPF_CNT_H) ;
		IO_SREG(TURNING_SFC_H, tmp) ;
		tmp = IO_GREG(DBG_DS_SPF_CNT_L) ;

		PON_MSG((MSG_OAM | MSG_ACT), "ploam_recv_tuning_control current SFC=0x%u\n",tmp) ;
		tmp_sfc_lsb16 = (pTunCtrlMsg->raw.scheduledSFC[0] << 8) + pTunCtrlMsg->raw.scheduledSFC[1];
		if((tmp & 0x0000ffff) > tmp_sfc_lsb16){
			tmp += 0x00010000; //carry bit
		}
		tmp = (tmp & 0xffff0000) + tmp_sfc_lsb16;
		IO_SREG(TURNING_SFC_L, tmp) ;
		PON_MSG((MSG_OAM | MSG_ACT), "ploam_recv_tuning_control sheduled SFC=0x%u\n",tmp) ;

		//enter O5.2 Pending State
		gpon_act_change_state(GPON_10G_STATE_O5_2) ;

		//set ds & us channel
		gpGponPriv->gponCfg.ng2.standbyDsChannel = pTunCtrlMsg->raw.targetDnPonID[3] & 0x0f;
		gpGponPriv->gponCfg.ng2.standbyUsChannel = pTunCtrlMsg->raw.targetUsPonID[3] & 0x0f;
		gpGponPriv->gponCfg.ng2.rollbackFlag = pTunCtrlMsg->raw.rollbackFlag;
		gpGponPriv->gponCfg.ng2.tuningOnGoing = 1;

		//record seq_no, then send complete_u in the new channe
		gpGponPriv->gponCfg.ng2.preSeqNo = pTunCtrlMsg->raw.seq_no;

		//send response
		ploam_send_tuning_resp_msg(pTunCtrlMsg->raw.seq_no,TUNING_RSP_ACK,0);
	}
	else if(pTunCtrlMsg->raw.operCode == TUNING_CTRL_COMPLETE_D){
		PON_MSG((MSG_ACT),"tuing_control COMPLETE_D\n");
		if(0){//(GPON_CURR_STATE != GPON_10G_STATE_O9) {
			PON_MSG((MSG_OAM | MSG_ERR), "ploam_recv_tuning_control requset no in O9 state, do nothing\n") ;
			return 0;
		}

		gpGponPriv->gponCfg.ng2.tuningOnGoing = 0;
		if(NGPON2_MODE)
			GPON_STOP_TIMER(gpGponPriv->to5_timer) ;

		if(GPON_CURR_STATE != GPON_10G_STATE_O5){
			gponDevMpiTxStop(XPON_RESET_HOLD_ON);
			gpon_act_change_state(GPON_10G_STATE_O5) ;
			gponDevMpiTxStop(XPON_RESET_RELEASE);
		}
	}

	return 0 ;
}

static int ploam_recv_system_profile(PLOAM_RAW_System_Profile_T *pSystemProf)
{
	uint msgDestId =0 ;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive system profile  message:", pSystemProf->value,PLOAM_DOWN_MSG_LENGTH) ;

	if(NGPON2_MODE == 0){
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: ploam_recv_system_profile do nothing in non-NGPON2 mode\n") ;
		return 0;
	}

	msgDestId =((pSystemProf->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pSystemProf->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;	
	if(msgDestId != PLOAM_BROADCAST_ADDR && msgDestId != PLOAM_XGSPON_BROADCAST_ADDR_NOKIA)  {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pSystemProf->raw.dest_id[0],pSystemProf->raw.dest_id[1]) ;
		return -1 ;
	}

	if(gpGponPriv->gponCfg.ng2.sysProfVer ==  pSystemProf->raw.sys_prof_ver){
		//duplicated message, do nothing
		return 0;
	}

	PON_MSG(MSG_OAM, "set system profile parameters. version = 0x%x\n",pSystemProf->raw.sys_prof_ver) ;

	gpGponPriv->gponCfg.ng2.sysProfVer =  pSystemProf->raw.sys_prof_ver;
	gpGponPriv->gponCfg.ng2.ng2sysId[0] = pSystemProf->raw.ng2_sys_id[0]; //note: becare endian, where to use?
	gpGponPriv->gponCfg.ng2.ng2sysId[1] = pSystemProf->raw.ng2_sys_id[1];
	gpGponPriv->gponCfg.ng2.ng2sysId[2] = pSystemProf->raw.ng2_sys_id[2];

	gpGponPriv->gponCfg.ng2.sysProfVer = pSystemProf->raw.sys_prof_ver;
	gpGponPriv->gponCfg.ng2.twdmChannelCnt = pSystemProf->raw.twdm_channel_cnt & 0x0f;

	 /* no need save, tell phy/optical directly */
	//gpGponPriv->gponCfg.ng2.usOpWaveBands = pSystemProf->raw.us_op_wave_bands & 0x03;
	//gpGponPriv->gponCfg.ng2.channelSpacing= pSystemProf->raw.channel_spacing ; //unit GHz
	//gpGponPriv->gponCfg.ng2.usMSE = pSystemProf->raw.us_mse ;
	//gpGponPriv->gponCfg.ng2.fsr = pSystemProf->raw.fsr ;

	return 0 ;
}

static int ploam_recv_channel_profile(PLOAM_RAW_Channel_Profile_T *pChannelProf)
{
	uint msgDestId =0 ;
	unchar index = 0;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive channel profile  message:", pChannelProf->value,PLOAM_DOWN_MSG_LENGTH) ;

	if(NGPON2_MODE == 0){
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: ploam_recv_channel_profile do nothing in non-NGPON2 mode\n") ;
		return 0;
	}

	msgDestId =((pChannelProf->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pChannelProf->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if(msgDestId != PLOAM_BROADCAST_ADDR && msgDestId != PLOAM_XGSPON_BROADCAST_ADDR_NOKIA) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pChannelProf->raw.dest_id[0],pChannelProf->raw.dest_id[1]) ;
		return -1 ;
	}

	index = pChannelProf->raw.prof_index;
	if(gpGponPriv->gponCfg.ng2.channenProf[index].profVer == pChannelProf->raw.prof_version){
		//duplicated message, do nothing
		return 0;
	}

	PON_MSG(MSG_OAM, "set channel profile parameters. version = 0x%x\n",pChannelProf->raw.prof_version) ;

	if(pChannelProf->raw.channel_indicator == 0 
		&& pChannelProf->raw.dn_void_incator == 1
		&& pChannelProf->raw.us_void_incator == 1){
		memset(&gpGponPriv->gponCfg.ng2.channenProf[index],0,sizeof(Channel_Prof_T));
			PON_MSG(MSG_OAM, "clear channel profile index = %d\n",index) ;
			return 0;
	}

	gpGponPriv->gponCfg.ng2.channenProf[index].profVer = pChannelProf->raw.prof_version;
	gpGponPriv->gponCfg.ng2.channenProf[index].ponId[0] = pChannelProf->raw.pon_id[0];
	gpGponPriv->gponCfg.ng2.channenProf[index].ponId[1] = pChannelProf->raw.pon_id[1];
	gpGponPriv->gponCfg.ng2.channenProf[index].ponId[2] = pChannelProf->raw.pon_id[2];
	gpGponPriv->gponCfg.ng2.channenProf[index].ponId[3] = pChannelProf->raw.pon_id[3];
	gpGponPriv->gponCfg.ng2.channenProf[index].dwlcdId  = pChannelProf->raw.pon_id[3] & 0x0f;

	if(pChannelProf->raw.dn_void_incator == 0){
		gpGponPriv->gponCfg.ng2.channenProf[index].dnFreqOffset = pChannelProf->raw.ds_freq_offset;
		gpGponPriv->gponCfg.ng2.channenProf[index].dnRate = pChannelProf->raw.dn_line_rate;
		gpGponPriv->gponCfg.ng2.channenProf[index].dnFEC = pChannelProf->raw.dn_fec;
	}

	if(pChannelProf->raw.us_void_incator == 0){
		gpGponPriv->gponCfg.ng2.channenProf[index].uwlchId = pChannelProf->raw.uwlch_id; // 4 bytes us pon id ?
		gpGponPriv->gponCfg.ng2.channenProf[index].usPonId[0] = pChannelProf->raw.pon_id[0];
		gpGponPriv->gponCfg.ng2.channenProf[index].usPonId[1] = pChannelProf->raw.pon_id[1];
		gpGponPriv->gponCfg.ng2.channenProf[index].usPonId[2] = pChannelProf->raw.pon_id[2];
		gpGponPriv->gponCfg.ng2.channenProf[index].usPonId[3] = (pChannelProf->raw.pon_id[3] & 0xf0) + (pChannelProf->raw.uwlch_id & 0x0f);

		gpGponPriv->gponCfg.ng2.channenProf[index].usFreq = unchar_convert_to_u32(pChannelProf->raw.us_freq,sizeof(pChannelProf->raw.us_freq));
		gpGponPriv->gponCfg.ng2.channenProf[index].usRate = pChannelProf->raw.us_rate;
		gpGponPriv->gponCfg.ng2.channenProf[index].opticalLinkType = pChannelProf->raw.optical_link_type;
	}

	PON_MSG((MSG_OAM | MSG_ACT), "index=%d,Ds ponId[3] = %d, Us ponId[3] = %d\n",index,
		gpGponPriv->gponCfg.ng2.channenProf[index].ponId[3],gpGponPriv->gponCfg.ng2.channenProf[index].usPonId[3]);

	if(pChannelProf->raw.channel_indicator == 1){ //current channel
		gpGponPriv->gponCfg.ng2.workDsChannel = pChannelProf->raw.pon_id[3] & 0x0f;
		gpGponPriv->gponCfg.ng2.workUsChannel = pChannelProf->raw.uwlch_id;
		gpGponPriv->gponCfg.ng2.workPonID[0] = pChannelProf->raw.pon_id[0];
		gpGponPriv->gponCfg.ng2.workPonID[1] = pChannelProf->raw.pon_id[1];
		gpGponPriv->gponCfg.ng2.workPonID[2] = pChannelProf->raw.pon_id[2];
		gpGponPriv->gponCfg.ng2.workPonID[3] = pChannelProf->raw.pon_id[3];

		//current channel, set parameter to reg which used for SN report
		IO_SREG(CUR_DS_PON_ID, unchar_convert_to_u32(gpGponPriv->gponCfg.ng2.channenProf[index].ponId,4)) ;
		IO_SREG(CUR_US_PON_ID, unchar_convert_to_u32(gpGponPriv->gponCfg.ng2.channenProf[index].usPonId,4)) ;
		IO_SREG(US_RATE_CAP, gpGponPriv->gponCfg.ng2.channenProf[index].usRate) ;

		//set NG2 Transmitter Channel
		XPON_PHY_SET_NG2_TX_CHAN(gpGponPriv->gponCfg.ng2.workUsChannel);
	}

	PON_MSG(MSG_OAM, "cahnnel profile index=%d dwlcdId=0x%x uwlchId=0x%x usFreq=%d\n",index,gpGponPriv->gponCfg.ng2.channenProf[index].dwlcdId,
		gpGponPriv->gponCfg.ng2.channenProf[index].uwlchId,gpGponPriv->gponCfg.ng2.channenProf[index].usFreq) ;

	return 0 ;
}

static int ploam_recv_protect_control(PLOAM_RAW_Protect_Ctl_T *pProtectCtrlMsg)
{
	uint msgDestId =0 ;
	//int i = 0;
	//int findFlag = 0;

	GPON_PLOAM_MSG_RAW(MSG_OAM,"PLOAM: Receive protect control  message:", pProtectCtrlMsg->value,PLOAM_DOWN_MSG_LENGTH) ;

	if(NGPON2_MODE == 0){
		PON_MSG((MSG_ERR|MSG_OAM), "PLOAM: ploam_recv_protect_control do nothing in non-NGPON2 mode\n") ;
		return 0;
	}

	msgDestId =((pProtectCtrlMsg->raw.dest_id[PLOAM_DEST_ID_HIGHER]<<8) | (pProtectCtrlMsg->raw.dest_id[PLOAM_DEST_ID_LOWER])) ;
	if(0){//((msgDestId != GPON_ONU_ID) || (msgDestId != PLOAM_BROADCAST_ADDR)) {
		PON_MSG((MSG_ERR|MSG_OAM), "The dest. address(%02x,%02x) of PLOAM message is incorrect.\n", pProtectCtrlMsg->raw.dest_id[0],pProtectCtrlMsg->raw.dest_id[1]) ;
		return -1 ;
	}

	if(pProtectCtrlMsg->raw.enable_flag == PROTECT_ENABLE){	
#if 0
		for(i = 0; i<NG2_CHANNEL_MAX_NUM; i++){
			if(match_pon_id(gpGponPriv->gponCfg.ng2.channenProf[i].ponId,pProtectCtrlMsg->raw.protect_ds_pon) &&
				match_pon_id(gpGponPriv->gponCfg.ng2.channenProf[i].usPonId,pProtectCtrlMsg->raw.protect_us_pon))
			{
				findFlag = 1;
			}
		}

		findFlag = 1;
		if(findFlag == 1){
#endif
			gpGponPriv->gponCfg.ng2.standbyDsChannel = pProtectCtrlMsg->raw.protect_ds_pon[3] & 0x0f;
			gpGponPriv->gponCfg.ng2.standbyUsChannel = pProtectCtrlMsg->raw.protect_us_pon[3] & 0x0f;
			gpGponPriv->gponCfg.ng2.protectFlag = PROTECT_ENABLE;
			PON_MSG((MSG_OAM | MSG_ACT), "ploam_recv_protect_control enable, ds-channel:%d,us-channe:%d\n",
				gpGponPriv->gponCfg.ng2.standbyDsChannel,gpGponPriv->gponCfg.ng2.standbyUsChannel);
#if 0
		}
		else{
			PON_MSG((MSG_OAM | MSG_ERR | MSG_ACT), "ploam_recv_protect_control enable, parameter error! ds-channel:%d,us-channe:%d\n",
				gpGponPriv->gponCfg.ng2.standbyDsChannel,gpGponPriv->gponCfg.ng2.standbyUsChannel);
			ploam_send_acknowledge_msg(pProtectCtrlMsg->raw.seq_no,XGPON_PLAOM_ACK_PARAM_ERR);
			return -1;
		}
#endif
	}
	else if(pProtectCtrlMsg->raw.enable_flag == PROTECT_DISABLE){
		gpGponPriv->gponCfg.ng2.standbyDsChannel = gpGponPriv->gponCfg.ng2.workDsChannel;
		gpGponPriv->gponCfg.ng2.standbyUsChannel = gpGponPriv->gponCfg.ng2.workUsChannel;
		gpGponPriv->gponCfg.ng2.protectFlag = PROTECT_DISABLE;
		PON_MSG((MSG_OAM | MSG_ACT), "ploam_recv_protect_control disable.\n") 
	}
	else{
		PON_MSG((MSG_OAM), "ploam_recv_protect_control wrong flag\n") 
		ploam_send_acknowledge_msg(pProtectCtrlMsg->raw.seq_no,XGPON_PLAOM_ACK_PARAM_ERR);
		return -1;
	}

	ploam_send_acknowledge_msg(pProtectCtrlMsg->raw.seq_no,XGPON_PLOAM_ACK_OK);
	return 0 ;
}

static int ploam_recv_change_power_level(PLOAM_RAW_General_T *pPloamMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive change_power_level message.(%.8X %.8X %.8X)\n", htonl(pPloamMsg->value[0]), htonl(pPloamMsg->value[1]), htonl(pPloamMsg->value[2])) ;

	return 0 ;
}

static int ploam_recv_power_consum_inquire(PLOAM_RAW_General_T *pPloamMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive power_consumption_inquire message.(%.8X %.8X %.8X)\n", htonl(pPloamMsg->value[0]), htonl(pPloamMsg->value[1]), htonl(pPloamMsg->value[2])) ;

	return 0 ;
}

static int ploam_recv_rate_control(PLOAM_RAW_General_T *pPloamMsg)
{
	PON_MSG(MSG_OAM, "PLOAM: Receive rate_control message.(%.8X %.8X %.8X)\n", htonl(pPloamMsg->value[0]), htonl(pPloamMsg->value[1]), htonl(pPloamMsg->value[2])) ;

	return 0 ;
}

/*______________________________________________________________________________
**  function name
**		ploam_init
**  description:
**		init tasklet and ploam recv handler
**  parameters:
**		none
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		tasklet_init
**  revision:
**		v1.0
**____________________________________________________________________________*/
void ploam_init(void)
{	
	tasklet_init(&gpGponPriv->securityKey_task, gponDevIntegrityKeySet,0x0);
	//tasklet_init(&gpGponPriv->allocId_task, gponDevAssignNewAllocId,(unsigned long)(&gpGponPriv->gponCfg.allocIdConfig));
    tasklet_init(&gpGponPriv->keyReport_task,gponDevUnicastKeyExchange,(unsigned long)(&gpGponPriv->gponCfg.keyReport));    
    tasklet_init(&gpGponPriv->rangingAck_task, gponDevRangingAck,(unsigned long)(&gpGponPriv->gponCfg.rangingAckSeqNo));
    tasklet_init(&gpGponPriv->swreplyploam_task, gponDevSwReplyRegistrationPloam,0x0);
	memset(gpGponPriv->ploamRecvHandler, 0x0, sizeof(ploam_recv_handler_t)*PLOAM_DOWN_MAX_TYPE) ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_PROFILE] 				= (ploam_recv_handler_t)ploam_recv_profile ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ASSIGN_ONUID] 			= (ploam_recv_handler_t)ploam_recv_assign_onu_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_RANGING_TIME] 			= (ploam_recv_handler_t)ploam_recv_ranging_time ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_DEACTIVATE_ONUID] 		= (ploam_recv_handler_t)ploam_recv_deactivate_onu ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_DISABLE_SERIAL_NUM] 	= (ploam_recv_handler_t)ploam_recv_disable_serial_number ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_REQUEST_REGISTRATION] 	= (ploam_recv_handler_t)ploam_recv_request_registration ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ASSIGN_ALLOCID] 		= (ploam_recv_handler_t)ploam_recv_assign_alloc_id ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_KEY_CONTROL] 			= (ploam_recv_handler_t)ploam_recv_key_control ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_SLEEP_ALLOW] 			= (ploam_recv_handler_t)ploam_recv_sleep_allow ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_CALIBRATION_REQUEST] 	= (ploam_recv_handler_t)ploam_recv_calibration_request ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_ADJUST_TX_WAVELENGTH] 	= (ploam_recv_handler_t)ploam_recv_adjust_tx_wavelength ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_TUNING_CONTROL] 		= (ploam_recv_handler_t)ploam_recv_tuning_control ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_SYSTEM_PROFILE] 		= (ploam_recv_handler_t)ploam_recv_system_profile ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_CHANNEL_PROFILE] 		= (ploam_recv_handler_t)ploam_recv_channel_profile ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_PROTECTION_CONTROL] 	= (ploam_recv_handler_t)ploam_recv_protect_control ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_CHANGE_POWER_LEVEL] 	= (ploam_recv_handler_t)ploam_recv_change_power_level ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_POWER_CONSUM_INQUIRE] 	= (ploam_recv_handler_t)ploam_recv_power_consum_inquire ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_RATE_CONTROL] 			= (ploam_recv_handler_t)ploam_recv_rate_control ;
	gpGponPriv->ploamRecvHandler[PLOAM_DOWN_MSG_REBOOT_ONU] 			= (ploam_recv_handler_t)ploam_recv_reboot_onu ;
	
	GPON_CREATE_TIMER(&gpGponPriv->silence_timer,gpon_silence_interval_expires,gpGponPriv->gponCfg.silenceInterval);/* the default value is 70000 */

	memset(gpGponPriv->dsPloamCounter, 0x0, sizeof(__u32)*PLOAM_DOWN_MAX_TYPE) ;
	memset(gpGponPriv->usPloamCounter, 0x0, sizeof(__u32)*PLOAM_UP_MAX_TYPE) ;
	memset(&gpGponPriv->prePloamMsg, 0x0, sizeof(PLOAM_RAW_General_T)) ;
}
/*______________________________________________________________________________
**  function name
**		ploam_deinit
**  description:
**		deinit tasklet 
**  parameters:
**		none
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		tasklet_kill
**  revision:
**		v1.0
**____________________________________________________________________________*/
void ploam_deinit(void){

	tasklet_kill(&gpGponPriv->securityKey_task);
	//tasklet_kill(&gpGponPriv->allocId_task);
    tasklet_kill(&gpGponPriv->keyReport_task);
    tasklet_kill(&gpGponPriv->rangingAck_task);    
	tasklet_kill(&gpGponPriv->swreplyploam_task);
	tasklet_kill(&clear_channel_task);
}
