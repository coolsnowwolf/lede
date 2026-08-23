#include <linux/module.h>
#include <linux/kernel.h>

#include "epon/epon_act.h"
#include "epon/epon.h"
#include "common/xpon_global.h"


int epon_isr_en7580(__u32* isr_m_llid_val, __u32* isr_val2);



extern __u32 ignoreIntMask;
extern uint32_t llid_rcv_rgst_int_sts_mark;

epon_int_t epon_int_name[] = {
		{0, "rcv_dauc_dscvgate"},
		{1, "rcv_crcerr"},
		{2, "rcv_eofdrop"},
		{3, "rcvgnt_infofail"},
		{4, "schgnt_inv"},
		{5, "schfch_timeo"},
		{6, "txmpi_pldenneq"},
		{7, "rxfifo_ovrun"},
		{8, "schfch_nopkt"},
		{9, "rcv_len_err"},
		{10, "rcy_rgst_exc"},
		{11, "rcv_ifchk_err"},
		{12, "tx_latestart"},		
		/*add cmd above this line*/
		{255,               NULL},
};

/*______________________________________________________________________________
**	function name
**		epon_isr
**	description:
**		 epon isr handle function
**	parameters:
**		None
**	global:
**		ignoreIntMask
**          gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_isr_en7580
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr(void)
{
	__u32 isr_val = 0;
	__u32 isr_en = 0;
	__u32 isr_m_llid_val = 0;
	__u32 isr_val2 = 0;
    __u8  bit_index = 0;
    epon_isr_handler_t isr_proc = NULL;

    isr_val = READ_REG_WORD(e_int_status);
	isr_en = READ_REG_WORD(e_int_en);

	isr_val &= ignoreIntMask;

    WRITE_REG_WORD(e_int_status , 0xffffffff);/*clear interrupt bit*/	
	
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin int_status is %x",__FUNCTION__,isr_val);

	isr_val &= isr_en;
	
	epon_isr_en7580(&isr_m_llid_val,&isr_val2);

	if(llid_rcv_rgst_int_sts_mark&0xFF)
	{		
		isr_val &= (~((llid_rcv_rgst_int_sts_mark&0xFF)<<1));
	}
	else if(llid_rcv_rgst_int_sts_mark&0xFFFFFF00)
	{
		isr_m_llid_val &= (~((llid_rcv_rgst_int_sts_mark&0xFFFFFF00)>>8));
	}
	
	/*e_int_status register maybe receives reg_req_done_int , llid_rcv_rgst_int and rcv_dscvry_gate_int at the same time, which will 
	cause state machine confusion*/
	if(isr_val & (1<<REGISTER_REQ_SEND_DONE_IDX)){
		isr_val &= 0xfefffffe;
		isr_proc = gp_epon_global_data->e_isr_handler[REGISTER_REQ_SEND_DONE_IDX];
		if(isr_proc != NULL)	
            isr_proc(REGISTER_REQ_SEND_DONE_IDX);
	}

    /*Type B*/
    if(gp_epon_global_data->typeb_holdOn_enable)
        return EPON_SUCCESS;

    for(bit_index = 0; bit_index < INT_STATUS_MAX; bit_index++)
    {
    	if(EPON_REG_BIT_VALID(isr_val,isr_m_llid_val,isr_val2,bit_index))
        {
            isr_proc = gp_epon_global_data->e_isr_handler[bit_index];
            DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG:int_status bit %d is set do procession",bit_index);
        	if(isr_proc != NULL)	
                isr_proc(bit_index);

        }
        else
        {
			DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG:int_status bit %d is not set continue",bit_index);
			continue;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s end",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_en7580
**	description:
**		 epon isr handle function for 7580 externed
**	parameters:
**		isr_m_llid_val
**		isr_val2
**	global:
**		ignoreIntMask
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_en7580(__u32* isr_m_llid_val, __u32* isr_val2)
{
	__u32 isr_m_llid_en = 0;
	__u32 isr_en2 = 0;

	if((isr_m_llid_val == NULL) || (isr_val2 == NULL))
		return EPON_INPUT_POINT_NULL;
	
	*isr_m_llid_val = READ_REG_WORD(e_int_sts3);
	isr_m_llid_en = READ_REG_WORD(e_int_en3);

	*isr_val2 = READ_REG_WORD(e_int_sts2);
	isr_en2 = READ_REG_WORD(e_int_en2);
	
	*isr_m_llid_val &= ignoreIntMask;
	*isr_val2 &= ignoreIntMask;

	WRITE_REG_WORD(e_int_sts3 , 0xffffffff);/*clear interrupt bit*/
    WRITE_REG_WORD(e_int_sts2 , 0x1fff);/*clear interrupt bit*/
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin isr_m_llid_int is %x",__FUNCTION__,*isr_m_llid_val);

	*isr_m_llid_val &= isr_m_llid_en;
	*isr_val2 &= isr_en2;
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_discovery_gate_handler
**	description:
**		 discvy gate handler
**	parameters:
**		para
**	global:
**		ignoreIntMask
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_msg_route_dispatch
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_discovery_gate_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s ",__FUNCTION__);
    epon_msg_route_dispatch(EPON_MSG_RCV_DISCOVERY_GATE,NULL);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_llid_register_handler
**	description:
**		llid register handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_msg_route_dispatch
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_llid_register_handler(__u32 para)
{
	__u32  llid_index = 0;
	if(para < GRANT_BUFFER_OVERRUN_IDX)
		llid_index = para - 1;
	else if((para > SNIFF_FIFO_TX_OVERRUN_IDX)&&(para < RCV_DAUC_DSCVGATE_IDX))
		llid_index = para - LLID8_REGISTER_IDX + 8;
    else
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"DEBUG: %s ,llid %d llid is wrong",__FUNCTION__,llid_index);
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s ,llid %d",__FUNCTION__,llid_index);
    epon_msg_route_dispatch(EPON_MSG_RCV_REGISTER_MSG,&llid_index);
    //epon_mpcp_rcv_register_msg_handler(llid_index);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_grant_buffer_overrun_handler
**	description:
**		grant buffer overrun handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_grant_buffer_overrun_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_p2p_message_tx_handler
**	description:
**		p2p message handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_p2p_message_tx_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_tod_update_handler
**	description:
**		tod update handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_tod_update_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_tod_1pps_handler
**	description:
**		tod 1pps handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_tod_1pps_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_time_drift_handler
**	description:
**		time drift handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_time_drift_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_mpcp_time_out_handler
**	description:
**		mpcp time out handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_mpcp_time_out_handler(__u32 para)
{
	__u8 llid_index = 0;
	
	
	for(llid_index = 0; llid_index < EPON_LLID_MAX_NUM; llid_index++)
	{
		if(EPON_CHECK_LLID_INDEX_VALID(llid_index))
	    {
			DEBUG_MSG_EPON(0,"DEBUG: %s begin,onu local deregister llid %d",__FUNCTION__,llid_index);
			epon_mpcp_local_deregister(llid_index);
	    }
	}
	
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_report_over_interval_handler
**	description:
**		report over interval  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_report_over_interval_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_tx_fifo_under_run_handler
**	description:
**		tx fifo under run  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_tx_fifo_under_run_handler(__u32 para)
{
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_power_saving_sleep_handler
**	description:
**		power saving sleep  handler
**	parameters:
**		para
**	global:
**		gpEponPriv
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_power_saving_sleep_handler(__u32 para)
{
#ifdef TCSUPPORT_EPON_POWERSAVING
	static __u16 sleepCount = 0;
	if (gpEponPriv->eponCfg.earlyWakeupFlag == 1)
	{
		sleepCount++;
		if (gpEponPriv->eponCfg.earlyWakeupCount == sleepCount)
		{
			EPON_START_TIMER(gpEponPriv->early_wakeup_timer,gpEponPriv->eponCfg.earlyWakeupTimer) ;
			sleepCount = 0;
		}
	}
#endif
	DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_power_saving_wake_up_handler
**	description:
**		power saving wake up  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_power_saving_wake_up_handler(__u32 para)
{
	//EPON_STOP_TIMER(gpEponPriv->early_wakeup_timer) ;
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_rx_sleep_allow_handler
**	description:
**		rx sleep allow  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_rx_sleep_allow_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_power_saving_early_wake_up_handler
**	description:
**		power saving early wake up  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_power_saving_early_wake_up_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_hiden_grant_handler
**	description:
**		hiden grant  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_hiden_grant_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_back_to_back_grant_handler
**	description:
**		back to back grant  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_back_to_back_grant_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_record1_grant_handler
**	description:
**		record1 grant  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_record1_grant_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_register_req_tx_done_handler
**	description:
**		register req tx done  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_register_req_tx_done_handler(__u32 para)
{
	__u8 llidIndex = 0;
	REG_e_llid_dscvry_ctrl dscvCtrl;
    EPON_LLID_INFO_T *llid_info = NULL;
    
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s begin",__FUNCTION__);
    
	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	llidIndex = dscvCtrl.Bits.tx_mpcp_llid_idx;
    EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);
    
    epon_mpcp_register_req_tx_done(llid_info);
    
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_register_ack_tx_done_handler
**	description:
**		register ack tx done  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_register_ack_tx_done_handler(__u32 para)
{
	__u8 llidIndex = 0;
	REG_e_llid_dscvry_ctrl dscvCtrl;
    EPON_LLID_INFO_T *llid_info = NULL;
    
	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	llidIndex = dscvCtrl.Bits.tx_mpcp_llid_idx;
    EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s begin llid %d",__FUNCTION__,llidIndex);
	if(llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTER_PENDING){
		epon_mpcp_register_ack_tx_done(llid_info);
	}else{
		epon_mpcp_state_init(llid_info->llidIndex);
	}    

    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_sniff_fifo_tx_overrun_handler
**	description:
**		sniff fifo tx overrun  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_sniff_fifo_tx_overrun_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_isr_int2_handler
**	description:
**		externed interrupt handler
**	parameters:
**		para
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_int2_handler(__u32 para)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin interrupt: %s",__FUNCTION__, epon_int_name[para-RCV_DAUC_DSCVGATE_IDX].int_name);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_isr_downstream_key_change_handler
**	description:
**		sniff fifo tx overrun  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_downstream_key_change_handler(__u32 para)
{
	return UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DOWN_KEY_CHANGE, &para, NULL);
}

/*______________________________________________________________________________
**	function name
**		epon_isr_downstream_key_miss_handler
**	description:
**		sniff fifo tx overrun  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_downstream_key_miss_handler(__u32 para)
{
    return UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DOWN_KEY_MISS, &para, NULL);
}

/*______________________________________________________________________________
**	function name
**		epon_isr_upstream_key_change_handler
**	description:
**		sniff fifo tx overrun  handler
**	parameters:
**		para
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_isr_upstream_key_change_handler(__u32 para)
{
    return UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_UP_KEY_CHANGE, &para, NULL);
}

/*______________________________________________________________________________
**	function name
**		epon_isr_handler_init
**	description:
**		init epon isr handler
**	parameters:
**		None
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_isr_handler_init(void)
{
	int index = 0;
	memset(gp_epon_global_data->e_isr_handler,0,(sizeof(epon_isr_handler_t)*EPON_INTERRUPT_STATUS_BIT_MAX));

    gp_epon_global_data->e_isr_handler[DISCOVERY_GATE_IDX]				= epon_isr_discovery_gate_handler;
    
	for(index = LLID0_REGISTER_IDX;index<=LLID7_REGISTER_IDX;index++)
    {
    	gp_epon_global_data->e_isr_handler[index]						= epon_isr_llid_register_handler;
	}
    
	gp_epon_global_data->e_isr_handler[GRANT_BUFFER_OVERRUN_IDX]		= epon_isr_grant_buffer_overrun_handler;
    gp_epon_global_data->e_isr_handler[P2P_MSG_TX_IDX]					= epon_isr_p2p_message_tx_handler;
    gp_epon_global_data->e_isr_handler[TOD_UPDATE_IDX]					= epon_isr_tod_update_handler;
    gp_epon_global_data->e_isr_handler[TOD_ONE_PPS_IDX]					= epon_isr_tod_1pps_handler;
    gp_epon_global_data->e_isr_handler[TIME_DRIFT_IDX]					= epon_isr_time_drift_handler;
    gp_epon_global_data->e_isr_handler[MPCP_TIME_OUT_IDX]				= epon_isr_mpcp_time_out_handler;
    gp_epon_global_data->e_isr_handler[REPORT_OVER_INTERVAL_IDX]		= epon_isr_report_over_interval_handler;
    gp_epon_global_data->e_isr_handler[TX_FIFO_UNDER_RUN_IDX]			= epon_isr_tx_fifo_under_run_handler;
	gp_epon_global_data->e_isr_handler[POWER_SAVING_SLEEP_IDX]			= epon_isr_power_saving_sleep_handler;
    gp_epon_global_data->e_isr_handler[POWER_SAVING_WAKE_UP_IDX]		= epon_isr_power_saving_wake_up_handler;
    gp_epon_global_data->e_isr_handler[RX_SLEEP_ALLOW_IDX]				= epon_isr_rx_sleep_allow_handler;
    gp_epon_global_data->e_isr_handler[POWER_SAVING_EARLY_WAKE_UP_IDX]	= epon_isr_power_saving_early_wake_up_handler;
	gp_epon_global_data->e_isr_handler[HIDEN_GRANT_IDX]					= epon_isr_hiden_grant_handler;
	gp_epon_global_data->e_isr_handler[BACK_TO_BACK_GRANT_IDX]			= epon_isr_back_to_back_grant_handler;
    gp_epon_global_data->e_isr_handler[RECORD1_GRANT_IDX]				= epon_isr_record1_grant_handler;
    gp_epon_global_data->e_isr_handler[REGISTER_REQ_SEND_DONE_IDX]		= epon_isr_register_req_tx_done_handler;
    gp_epon_global_data->e_isr_handler[REGISTER_ACK_SEND_DONE_IDX]		= epon_isr_register_ack_tx_done_handler;
    gp_epon_global_data->e_isr_handler[SNIFF_FIFO_TX_OVERRUN_IDX]		= epon_isr_sniff_fifo_tx_overrun_handler;

	for(index = LLID8_REGISTER_IDX;index<=LLID31_REGISTER_IDX;index++)
    {
    	gp_epon_global_data->e_isr_handler[index]						= (epon_isr_handler_t)epon_isr_llid_register_handler;
	}
	for(index = RCV_DAUC_DSCVGATE_IDX;index < INT_STATUS_MAX;index++)
	{
		gp_epon_global_data->e_isr_handler[index]						= (epon_isr_handler_t)epon_isr_int2_handler;
	}
	gp_epon_global_data->e_isr_handler[DOWNSTRAEM_KEY_CHANGE_IDX]		= epon_isr_downstream_key_change_handler;
    gp_epon_global_data->e_isr_handler[DOWNSTRAEM_KEY_MISS_IDX]			= epon_isr_downstream_key_miss_handler;
    gp_epon_global_data->e_isr_handler[UPSTRAEM_KEY_CHANGE_IDX]			= epon_isr_upstream_key_change_handler;
}

