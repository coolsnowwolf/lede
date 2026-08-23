#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>

#include "epon/epon.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include "common/xpon_global.h"
#include "common/xpon_led.h"
#include "epon/epon_compile_option_wrapper.h"
#include <asm/tc3162/ledcetrl.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include "common/phy_if_wrapper.h"



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
extern uint8_t RgstrAckFlag[EPON_LLID_MAX_NUM];
/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
extern void record_random_dly_test(void);
extern void set_fix_reg_list(void);

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
#ifdef TCSUPPORT_AUTOBENCH
int epon_slt_test = 0;
EXPORT_SYMBOL(epon_slt_test);
#endif
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
**		epon_show_global_isr_state
**	description:
**		show isr status
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_show_global_isr_state(void)
{
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"Global state is %x ",READ_REG_WORD(e_glb_cfg));
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"intrrupt enable is 0x%x ",READ_REG_WORD(e_int_en));
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"intrrupt m-llid-enable is 0x%x",READ_REG_WORD(e_int_en3));
}

/*______________________________________________________________________________
**	function name
**		epon_time_record
**	description:
**		record current time and print 
**	parameters:
**		Time
**	global:
**		None
**	return:
**		None
**	call:
**		ktime_to_sec
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_time_record(__u8 *Time)
{
    ktime_t time;
 
	__u32 sec = 0;
    __u32 min = 0;
    __u32 hour = 0;

	time = ktime_get();

	sec  =(__u32) ktime_to_sec(time) % 60;
    min = ((__u32) ktime_to_sec(time) / 60) % 60;
    hour = (__u32) ktime_to_sec(time) / 3600;

    sprintf(Time, "%d h:%d min:%d s",hour,min,sec);

}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_set_sync_time
**	description:
**		set sync time
**	parameters:
**		None
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_set_sync_time(void)
{
	REG_e_sync_time eSyncTime;
    EPON_LLID_INFO_T *llid_info = NULL;
    EPON_GET_LLID_BY_INDEX(llid_info,0);

    eSyncTime.Raw = READ_REG_WORD(e_sync_time);
    DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "%s begin sync time is %x ",__FUNCTION__,eSyncTime.Raw);	
	if (eSyncTime.Bits.sync_time > MPCP_SYNC_TIME_MAX){
		if (MPCP_STATE_REGISTERED != llid_info->eponMpcp.mpcpDiscState)
{
			WRITE_REG_WORD(e_sync_time, MPCP_SYNC_TIME_MAX);
			WRITE_REG_WORD(e_trx_adjust_time1, 0x008efff1);
		}
	}
	else if (eSyncTime.Bits.sync_time == 0){
		WRITE_REG_WORD(e_sync_time, gp_epon_global_data->sync_time);
		eSyncTime.Raw = READ_REG_WORD(e_sync_time);
		WRITE_REG_WORD(e_trx_adjust_time1, (0x002ffff1+ ((gp_epon_global_data->sync_time)<<16)));
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE ,"enter eponMpcpDiscvGateIntHandler:sync_time == 0, set gMpcpSynctime = 0x%x", gp_epon_global_data->sync_time);
	}
	else{
		WRITE_REG_WORD(e_trx_adjust_time1, (0x002ffff1+ (eSyncTime.Bits.sync_time<<16)));
		if(gp_epon_global_data->sync_time != eSyncTime.Bits.sync_time){
			gp_epon_global_data->sync_time = eSyncTime.Bits.sync_time;
		}
	}
    DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "%s end sync_time is %x",__FUNCTION__,gp_epon_global_data->sync_time);
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_set_register_ack_flag
**	description:
**		set register ack flag
**	parameters:
**		llidIndex
**		rgstAckFlag
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_set_register_ack_flag(__u8 llidIndex , __u8 rgstAckFlag){
	if((llidIndex > 31 )||(rgstAckFlag >1)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponMpcpSetDiscvRgstAck param error");
		return -1;
	}
	gp_epon_global_data->llid_entry[llidIndex].eponMpcp.mpcpDiscMacr_RgstrAck  = rgstAckFlag;
	return 0;
}
/*______________________________________________________________________________
**	function name
**		eponMacCheckRegreqDoneBit
**	description:
**		check reg req done 
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponMacCheckRegreqDoneBit(void){
	REG_e_llid_dscvry_ctrl dscvCtrl;

	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	if(dscvCtrl.Bits.mpcp_cmd_done == 1)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacCheckRegreqDoneBit done");
		return 0;
	}else{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: eponMacCheckRegreqDoneBit not ready");
		return -1;
	}
	return 0;
}

/*______________________________________________________________________________
**	function name
**		epon_send_mpcp_register_request
**	description:
**		sed mpcp request
**	parameters:
**		llid_index
**		msg_type
**		rgst_req_flag
**	global:
**		None
**	return:
**		None
**	call:
**		epon_dev_set_tx_burst_mode
**		epon_dev_set_hw_discv_states
**		epon_dev_check_last_msg_tx_done
**		epon_add_msg_to_send_list
**		epon_dev_send_mpcp_msg
**		epon_show_global_isr_state
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_send_mpcp_register_request(__u8 llid_index,__u8 msg_type, __u8 rgst_req_flag)
{
    MPCP_SEND_INFO_T msg_info ;

    memset(&msg_info, 0, sizeof(MPCP_SEND_INFO_T));
    if(REGISTER_REQ_TX_IN_DISCVRY_GATE == msg_type)
        epon_dev_set_tx_burst_mode(TRUE);/*make sure it is burst mode before send REQ*/
    else
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"tx burst mode set fail\n");
    
    epon_dev_set_hw_discv_states(llid_index,EPON_HW_DSCV_STATE_REGISTERING);

	msg_info.llid_index 		= llid_index;
    msg_info.msg_type			= msg_type;
    msg_info.register_req_flag	= rgst_req_flag;
		
    epon_dev_send_mpcp_msg(&msg_info);

    epon_show_global_isr_state();
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_dev_send_mpcp_register_ack
**	description:
**		send mpcp ack
**	parameters:
**		llid_index
**		register_ack_flag
**	global:
**		None
**	return:
**		None
**	call:
**		epon_dev_set_tx_burst_mode
**		eponMacCheckRegreqDoneBit
**		epon_show_global_isr_state
**	revision:
**		v1.0
**____________________________________________________________________________*/

			
void epon_dev_send_mpcp_register_ack(__u8 llid_index,__u8 register_ack_flag)
{
    REG_e_llid_dscvry_ctrl dscvCtrl;
	epon_dev_set_tx_burst_mode(FALSE);
				
    if(eponMacCheckRegreqDoneBit() <0){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: epon_dev_send_mpcp_register_ack done bit not ready");
        return;
	}

	dscvCtrl.Bits.mpcp_cmd = REGISTER_ACK_TX_IN_NORMAL_GATE;
	dscvCtrl.Bits.rgstr_ack_flg = register_ack_flag;
	dscvCtrl.Bits.tx_mpcp_llid_idx = llid_index;//////////////
			dscvCtrl.Bits.mpcp_cmd_done = 1;
			WRITE_REG_WORD(e_llid_dscvry_ctrl, dscvCtrl.Raw);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s end  llid %d, ack flag %d mpcp_cmd %d value is %x",
        __FUNCTION__,llid_index,dscvCtrl.Bits.rgstr_ack_flg,dscvCtrl.Bits.mpcp_cmd,dscvCtrl.Raw);
  
    epon_show_global_isr_state();
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_set_mpcp_send_info
**	description:
**		set mpcp send info
**	parameters:
**		llid_index
**		mpcp_msg
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
			
void epon_set_mpcp_send_info(__u8 llid_index,EPON_MPCP_MSG_T mpcp_msg)
		{
	EPON_LLID_INFO_T *llid_info = NULL;
	if(!EPON_CHECK_LLID_INDEX_VALID(llid_index))
				{
    	return;
				}
	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);	

	llid_info->mpcp_send_info.msg_type = mpcp_msg;
    llid_info->mpcp_send_info.send_flag = TRUE;
    return;
			}
/*______________________________________________________________________________
**	function name
**		epon_us_collsion_discovery_gate_process
**	description:
**		handle us collsion discvy gate
**	parameters:
**		llid_index
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_send_mpcp_register_request
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_us_collsion_discovery_gate_process(__u8 llid_index)
{
			
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin  llid %d",__FUNCTION__,llid_index);
	epon_send_mpcp_register_request(llid_index,REGISTER_REQ_TX_IN_DISCVRY_GATE,REGISTER_REQ_FLAG_REGISTER);
   // epon_set_mpcp_send_info(llid_index,MPCP_MSG_REGISTER_REQUEST);
    return EPON_SUCCESS;
	}
/*______________________________________________________________________________
**	function name
**		epon_normal_discovery_gate_process
**	description:
**		handle normal discvy gate
**	parameters:
**		llid_index
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_show_global_isr_state
**		epon_send_mpcp_register_request
**	revision:
**		v1.0
**____________________________________________________________________________*/
	
int epon_normal_discovery_gate_process(__u8 llid_index)
	{
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin llid %d",__FUNCTION__,llid_index);
    //epon_change_state(llid_index,MPCP_STATE_REGISTERING);
    epon_show_global_isr_state();
		
	epon_send_mpcp_register_request(llid_index,REGISTER_REQ_TX_IN_DISCVRY_GATE,REGISTER_REQ_FLAG_REGISTER);
 //   epon_set_mpcp_send_info(llid_index,MPCP_MSG_REGISTER_REQUEST);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d send Register_Req",llid_index);
    return EPON_SUCCESS;	
}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_local_deregister
**	description:
**		local deregister
**	parameters:
**		llid_index
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_send_mpcp_register_request
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_local_deregister(__u8 llid_index)
{
	EPON_LLID_INFO_T *llid_info = NULL;
    eponMpcp_t *mpcpDiscFsm = NULL;
    
	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);
	mpcpDiscFsm = &(llid_info->eponMpcp);

	if(mpcpDiscFsm->begin == FALSE){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "epon_mpcp_local_deregister mpcpDiscFsm->begin == FALSE");
		return -1;
}	

	if(mpcpDiscFsm->mpcpDiscState ==  MPCP_STATE_REGISTERED){
		epon_send_mpcp_register_request(llid_index,REGISTER_REQ_TX_IN_NORMAL_GATE,REGISTER_REQ_FLAG_DEREGISTER);
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "epon_mpcp_local_deregister send dereigster" );
	}

	epon_de_register_change_state(llid_info);
	epon_llid_reinit(llid_info);

	return 0;
}
/*______________________________________________________________________________
**	function name
**		epon_should_response_discovery_gate
**	description:
**		filter non response discvy gate
**	parameters:
**		llid_index
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

bool epon_should_response_discovery_gate(__u8 llid_index)
{
	EPON_LLID_INFO_T *llid_info = NULL;
    if(!EPON_CHECK_LLID_INDEX_VALID(llid_index)) {
    	return FALSE;
	}

	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);

	if(llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTERED)
        return FALSE;
    
	if(llid_info->eponMpcp.mpcpReqRetryFlag > 0)
    {
    	llid_info->eponMpcp.mpcpReqRetryFlag -= 1;
		if(llid_info->eponMpcp.mpcpReqRetryFlag <= 0)
			return FALSE;
	}
    else
	{
		llid_info->eponMpcp.mpcpReqRetryFlag = 3;//retry 3times
	}
	
    /*pon led flicker*/
    if(TX_POWER_ENABLE){
        change_pon_led_status(LED_FLICKER);
    }

    return TRUE;
    }
/*______________________________________________________________________________
**	function name
**		epon_mpcp_discovery_gate_handler
**	description:
**		handle discvy gate
**	parameters:
**		data
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_should_response_discovery_gate
**		epon_mpcp_set_sync_time
**		epon_normal_discovery_gate_process
**		epon_us_collsion_discovery_gate_process
**	revision:
**		v1.0
**____________________________________________________________________________*/

    
int epon_mpcp_discovery_gate_handler(void *data)
{
	EPON_LLID_INFO_T *llid_info = NULL;
    __u8 llid_index 	= 0;
    __u8 current_state 	= 0;
    __u8 mpcp_send 		= FALSE;
	static __u8 ignore_cnt = 0;

    DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "%s begin ",__FUNCTION__);
	/*test for m-llid*/
	if((gp_epon_global_data->m_llid_flag)&&(gp_epon_global_data->ignore_flag))
	{
		ignore_cnt++;
		if(TEST_M_LLID_IGNORE_NUM == ignore_cnt)
		{
			ignore_cnt = 0;
			gp_epon_global_data->ignore_flag = 0;
		}
		return EPON_SUCCESS;
	}
	/*test for m-llid*/
    for(llid_index = 0;llid_index < EPON_LLID_MAX_NUM; llid_index++)
    {
        if(FALSE == epon_should_response_discovery_gate(llid_index))
        	continue;

    	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);
        current_state = llid_info->eponMpcp.mpcpDiscState;
		XPON_PHY_TX_ENABLE();
        epon_mpcp_set_sync_time();
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d Receive Discoverr Gate & current_state: %d",llid_index,current_state);
        switch(current_state)
		{
        	case MPCP_STATE_REGISTERING:
                epon_normal_discovery_gate_process(llid_index);
                mpcp_send = TRUE;
                break;
            case MPCP_STATE_REGISTER_REQUEST:
                epon_us_collsion_discovery_gate_process(llid_index);
                mpcp_send = TRUE;
                break;
            default:
                break;               
		}
        //register_req send once in one discovery gate
        if(mpcp_send == TRUE) break;
     }
        
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "%s end ",__FUNCTION__);
	return EPON_SUCCESS;
    }

/*______________________________________________________________________________
**	function name
**		epon_reg_check_mpcp_tx_done
**	description:
**		check mpcp tx done
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_reg_check_mpcp_tx_done(void)
{
	REG_e_llid_dscvry_ctrl dscvCtrl;
	__u32    cnt =0;

    while(1)
    {
            cnt++;
            dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
        if(dscvCtrl.Bits.mpcp_cmd_done == EPON_REG_MPCP_CMD_DONE_SUCCESS)
            {
            DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s success",__FUNCTION__);
            return EPON_SUCCESS;
            }
        if(cnt > 3000000){
            return EPON_NORMAL_ERROR;
        }
    }
}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_shutdown_disc_sync_update
**	description:
**		shutdown discvy sync time uodate
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_mpcp_shutdown_disc_sync_update(void)
{
    int raw_tmp = READ_REG_WORD(e_mpcp_timeout_intvl);
    raw_tmp &= (~SHUT_DOWN_DISC_SYNC_UPDATE);
	WRITE_REG_WORD(e_mpcp_timeout_intvl, raw_tmp);/*shut down discovergate sync time update*/
}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_register_req_tx_done
**	description:
**		mpcp reg req tx done 
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		show_mpcp_tx_rx_counters
**		epon_change_state
**		epon_mpcp_shutdown_disc_sync_update
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_register_req_tx_done(EPON_LLID_INFO_T *llid_info)
{
	show_mpcp_tx_rx_counters();
    epon_change_state(llid_info->llidIndex,MPCP_STATE_REGISTER_REQUEST);//test for random delay
	//DEBUG_MSG_EPON(0,"random delay: %x",READ_REG_WORD(0xbfb663dc));//test for random delay
    epon_mpcp_shutdown_disc_sync_update();
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d HW Send Rgst Req Done",llid_info->llidIndex);
	record_random_dly_test();
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_mpcp_discovery_completed
**	description:
**		mpcp discovery done 
**	parameters:
**		llid_info
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		show_current_hw_disc_state
**		epon_dev_set_hw_discv_states
**		epon_change_state
**		epon_time_record
**		epon_event_report
**		xmcs_create_llid
**          start_omci_oam_monitor
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_discovery_completed(EPON_LLID_INFO_T *llid_info)
{
	struct XMCS_EponLlidCreate_S eponLlidCreate= {0};
    int ret = 0;
   
    show_current_hw_disc_state(llid_info->llidIndex);  
    epon_dev_set_hw_discv_states(llid_info->llidIndex,EPON_HW_DSCV_STATE_REGISTERED);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: llid %d MPCP Handshake Done",llid_info->llidIndex);
    show_current_hw_disc_state(llid_info->llidIndex);  

	QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;

    epon_change_state(llid_info->llidIndex,MPCP_STATE_REGISTERED);//test for random delay

	if(gp_epon_global_data->lost_count == 0)
	{
		epon_time_record(gp_epon_global_data->register_time);
	}
	else
	{
		memcpy(gp_epon_global_data->last_register_time,gp_epon_global_data->register_time,MAX_CHAR_NUM);
		memset(gp_epon_global_data->register_time, 0, MAX_CHAR_NUM);
		epon_time_record(gp_epon_global_data->register_time);
	}
		
    epon_event_report(XMCS_EVENT_EPON_REGISTER,llid_info->llidIndex);
    start_omci_oam_monitor();

    eponLlidCreate.idx = llid_info->llidIndex;
	eponLlidCreate.llid = llid_info->llid;

    ret = xmcs_create_llid(&eponLlidCreate);
    if(0 != ret)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[ERROR]xmcs_create_llid fail");
        return -1;
	}
    
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid index %d change to registered and create llid %x",llid_info->llidIndex,llid_info->llid);

	/*test for m-llid*/
	if(gp_epon_global_data->m_llid_flag)
	{
		if(0 == (llid_info->llidIndex+1)%4)
		{
			gp_epon_global_data->ignore_flag = 1;
		}
	}/*test for m-llid*/
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_mpcp_register_ack_tx_done
**	description:
**		mpcp reg ack tx done 
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		show_mpcp_tx_rx_counters
**		epon_mpcp_discovery_completed
**		epon_mpcp_state_init
**	revision:
**		v1.0
**____________________________________________________________________________*/


int epon_mpcp_register_ack_tx_done(EPON_LLID_INFO_T *llid_info)
{
    __u8 rgst_ack_flag = llid_info->eponMpcp.mpcpDiscMacr_RgstrAck;
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: llid %d check register ack begin",llid_info->llidIndex); 
		
	show_mpcp_tx_rx_counters();
    if(ACK == rgst_ack_flag)
    {
		epon_mpcp_discovery_completed(llid_info);
		/*pon led on*/
		change_pon_led_status(LED_ON);
		change_alarm_led_status(LED_OFF);
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d  Send Register ack complete mpcp discovery",llid_info->llidIndex);
    }
    else if(NACK == rgst_ack_flag)
    {
    	epon_mpcp_state_init(llid_info->llidIndex);
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d Send Register Nack change to initial state ",llid_info->llidIndex);
	}else
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"DEBUG: llid %d check register flag wrong",llid_info->llidIndex);
    }
    
#ifdef TCSUPPORT_AUTOBENCH
        epon_slt_test = 1;
#endif

	return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_send_mpcp_check_handler
**	description:
**		mpcp message send handle 
**	parameters:
**		data
**	global:
**		None
**	return:
**		None
**	call:
**		epon_reg_check_mpcp_tx_done
**		epon_mpcp_register_req_tx_done
**		epon_mpcp_register_ack_tx_done
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_send_mpcp_check_handler(unsigned long data)
{
	EPON_LLID_INFO_T *llid_info = NULL;
    __u8 llid_index 	= 0;
    __u32 ret = 0;

    for(llid_index = 0;llid_index < EPON_LLID_MAX_NUM; llid_index++)
    {
    	if(!EPON_CHECK_LLID_INDEX_VALID(llid_index))
        	continue;
			
    	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);
        if(TRUE != llid_info->mpcp_send_info.send_flag)
        {
            continue;
        }

	    ret = epon_reg_check_mpcp_tx_done();
	    if(EPON_SUCCESS != ret)
	    {
	        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_mpcp_register_req_send_check llid %d",llid_info->llidIndex);
	        continue;
		}
        llid_info->mpcp_send_info.send_flag = FALSE;

        if((llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTERING)
        && (llid_info->mpcp_send_info.msg_type == MPCP_MSG_REGISTER_REQUEST))
            {
			epon_mpcp_register_req_tx_done(llid_info);
			}
        else if((llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTER_PENDING)
        && (llid_info->mpcp_send_info.msg_type == MPCP_MSG_REGISTER_ACK))
			{
        	epon_mpcp_register_ack_tx_done(llid_info);
        }

        llid_info->mpcp_send_info.msg_type = MPCP_MSG_NONE;
  	}
	return;
}
/*______________________________________________________________________________
**	function name
**		epon_init_reg_llid_discovery_status
**	description:
**		init discovy status 
**	parameters:
**		mpcpDiscFsm
**		llidIndex
**	global:
**		None
**	return:
**		None
**	call:
**		epon_dev_set_hw_discv_states
**		show_current_hw_disc_state
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_init_reg_llid_discovery_status(eponMpcp_t *mpcpDiscFsm,__u8 llidIndex)
{
	__u32 pollingCnt = 0;
    			
	if(mpcpDiscFsm->begin == TRUE)
    {   
		//change disc status to unregister
		while(pollingCnt < 10){
			epon_dev_set_hw_discv_states(llidIndex,EPON_HW_DSCV_STATE_UNREGISTERED);
			pollingCnt++;
			}
		//change disc status to registing
		epon_dev_set_hw_discv_states(llidIndex,EPON_HW_DSCV_STATE_REGISTERING);
        show_current_hw_disc_state(llidIndex);
	}
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_mpcp_state_init
**	description:
**		init mpcp status 
**	parameters:
**		llidIndex
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_init_reg_llid_discovery_status
**		show_current_hw_disc_stop_omci_oam_monitor_not_onlinestate
**	revision:
**		v1.0
**____________________________________________________________________________*/		
int epon_mpcp_state_init(__u8 llidIndex)
{
	EPON_LLID_INFO_T *llid_info = NULL;
	eponMpcp_t *mpcpDiscFsm = NULL;

	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);
	
	if(!EPON_CHECK_LLID_INDEX_VALID(llidIndex))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s llidIndex %d bit mask %x outbound",
            __FUNCTION__,llidIndex,gp_epon_global_data->llid_bit_mask);
        return EPON_NORMAL_ERROR;
	}

    EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);
    mpcpDiscFsm = &llid_info->eponMpcp;

	mpcpDiscFsm->begin = TRUE;
	mpcpDiscFsm->registered = FALSE;
	if(!RgstrAckFlag[llidIndex])
	{
		mpcpDiscFsm->mpcpDiscMacr_RgstrAck = NACK;
		DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: TEST %s llid %d State Init NACK",__FUNCTION__,llidIndex); 
	}
	else
	{
		mpcpDiscFsm->mpcpDiscMacr_RgstrAck = ACK;
	}
	mpcpDiscFsm->mpcpDiscState = MPCP_STATE_REGISTERING;
	mpcpDiscFsm->mpcpReqRetryFlag = 0;

	epon_init_reg_llid_discovery_status(mpcpDiscFsm,llidIndex);

    stop_omci_oam_monitor_not_online();
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);

	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_llid_become_silent
**	description:
**		set mac silent
**	parameters:
**		llid_info
**	global:
**		gp_epon_global_data
**	return:
**		None
**	call:
**		epon_change_state
**		epon_event_report
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_llid_become_silent(EPON_LLID_INFO_T *llid_info)
{
	__u32 silent_time = gp_epon_global_data->silent_time_config;
	epon_change_state(llid_info->llidIndex,MPCP_STATE_DENIED);
    epon_event_report(XMCS_EVENT_EPON_DENIED,llid_info->llidIndex);
    llid_info->silent_time_left = silent_time;
	EPON_START_TIMER(gp_epon_global_data->silent_timer,EPON_SILENT_INTERVAL);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s llid %d time %d",__FUNCTION__,llid_info->llidIndex,silent_time); 
}
/*______________________________________________________________________________
**	function name
**		epon_rcv_re_register_change_state
**	description:
**		set re-register status
**	parameters:
**		llid_info
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_change_state
**		epon_dev_set_hw_discv_states
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_rcv_re_register_change_state(void *param)
{
    EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;

    if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
    EPON_CHECK_CURRENT_STATE(llid_info,MPCP_STATE_REGISTERED);
    epon_dev_set_hw_discv_states(llid_info->llidIndex,EPON_HW_DSCV_STATE_REGISTERING);
	epon_change_state(llid_info->llidIndex,MPCP_STATE_REGISTER_PENDING);	
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d Receive OLT RE_Register Msg",llid_info->llidIndex);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_rcv_rgst_ack_change_state
**	description:
**		set receive register ack status
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_change_state
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_rcv_rgst_ack_change_state(void *param)
{
    EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;
    if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
    EPON_CHECK_CURRENT_STATE(llid_info,MPCP_STATE_REGISTER_REQUEST);
	//epon_mpcp_set_sync_time();
    epon_change_state(llid_info->llidIndex,MPCP_STATE_REGISTER_PENDING);
	UNION_IC_FUNCTION_HOOK(ONLINE_ACTION_EPON_GET_OLT_MAC_ADDRESS, NULL, llid_info);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d Receive OLT Register ACK Msg ",llid_info->llidIndex);
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_act_send_register_ack
**	description:
**		send register ack 
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_dev_check_last_msg_tx_done
**		epon_add_msg_to_send_list
**	revision:
**		v1.0
**____________________________________________________________________________*/	
int epon_act_send_register_ack(void *param)
{
	MPCP_SEND_INFO_T msg_info;
	EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;

	if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }

	memset(&msg_info, 0, sizeof(MPCP_SEND_INFO_T));

	msg_info.llid_index 		= llid_info->llidIndex;
    msg_info.msg_type			= REGISTER_ACK_TX_IN_NORMAL_GATE;
    msg_info.register_ack_flag	= llid_info->eponMpcp.mpcpDiscMacr_RgstrAck;
	
    epon_dev_send_mpcp_msg(&msg_info);

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d SW Send Register Ack ",llid_info->llidIndex);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_de_register_change_state
**	description:
**		set de-register status 
**	parameters:
**		llid_info
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_llid_become_silent
**		epon_change_state
**		epon_event_report
**		epon_time_record
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_de_register_change_state(void *param)
{
    EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;

    if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"DEBUG:llid %d Receive OLT DE_Register Msg 0xbfb6611c:0x%x",llid_info->llidIndex,READ_REG_WORD(e_oam_stat));

    epon_change_state(llid_info->llidIndex,MPCP_STATE_REMOTE_DEREGISTER);
    epon_event_report(XMCS_EVENT_EPON_DEREGISTER,llid_info->llidIndex);
    
	if((TRUE == gp_epon_global_data->oam_auth_fail)
     &&(gp_epon_global_data->silent_time_config > 0))
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"DEBUG: %s llid %d ",__FUNCTION__,llid_info->llidIndex); 
#ifndef TCSUPPORT_OPENWRT
     	epon_llid_become_silent(llid_info);
#endif
	}
   
	memset(gp_epon_global_data->lost_time, 0, MAX_CHAR_NUM);
	epon_time_record(gp_epon_global_data->lost_time);
	gp_epon_global_data->lost_count++;
	
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);  
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_llid_reinit
**	description:
**		re-init llid 
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		xmcs_remove_llid
**		epon_phy_ready_hw_init
**		epon_phy_ready_sw_init
**		epon_olt_de_register_led_control
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_llid_reinit(void *param)
{
    EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;
    
    if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
	epon_dev_tx_rx_disable();
	epon_reset(NULL);
    xmcs_remove_llid(llid_info->llidIndex);
    //epon_mpcp_state_init(llid_info->llidIndex);
    epon_phy_ready_hw_init(NULL);
	set_fix_reg_list();
    epon_phy_ready_sw_init(NULL);
    epon_olt_de_register_led_control();	
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);  
    return EPON_SUCCESS;	
}

/*______________________________________________________________________________
**	function name
**		epon_rcv_nack_flag_msg_process
**	description:
**		handle mpcp message flag is nack 
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_event_report
**		epon_mpcp_state_init
**		epon_llid_become_silent
**		epon_olt_nack_led_control
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_rcv_nack_flag_msg_process(void *param)
{
	EPON_LLID_INFO_T *llid_info = (EPON_LLID_INFO_T *)param;
	int ret = EPON_SUCCESS;

	if(llid_info == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] LLID INFO IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d Receive OLT Register Nack Msg",llid_info->llidIndex);
  //  EPON_CHECK_CURRENT_STATE(llid_info,MPCP_STATE_REGISTER_REQUEST);
	//return ret;
	epon_event_report(XMCS_EVENT_EPON_DENIED,llid_info->llidIndex);
	
	epon_mpcp_state_init(llid_info->llidIndex);

	//epon_change_state(llid_info->llidIndex,MPCP_STATE_WAIT);
	
	if(gp_epon_global_data->silent_time_config != 0)
        epon_llid_become_silent(llid_info);

    epon_olt_nack_led_control();

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);    
	return ret;
}

/*______________________________________________________________________________
**	function name
**		get_llid_hw_discv_state
**	description:
**		get hw discvy status
**	parameters:
**		llid_info
**		status
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int get_llid_hw_discv_state(EPON_LLID_INFO_T *llid_info,REG_e_llid_dscvry_sts *status)
{
	if(llid_info->llidIndex < 32)
		status->Raw = READ_REG_WORD(g_epon_llid_dscv_stat[llid_info->llidIndex]);
	else {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR:  %s llid index %d is invalid",__FUNCTION__, llid_info->llidIndex);
		return EPON_NORMAL_ERROR;
	}
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "%s llid_index=%d disc status is %x",__FUNCTION__,
	            llid_info->llidIndex, status->Raw);
    /* update llid value if receive ack or reregister msg */
	if((MPCP_REGISTER_FLAG_ACK == status->Bits.llid_rgstr_flg_sts)
     ||(MPCP_REGISTER_FLAG_RE_REGISTER == status->Bits.llid_rgstr_flg_sts))
    {
		if(TRUE == status->Bits.llid_valid)
	    {
	    	llid_info->llid = status->Bits.llid_value;
	        DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "DEBUG: llid_index=%d llid value from olt =%x",
	            llid_info->llidIndex, llid_info->llid);
		}
	    else
	    {
	    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR:  llid index %d is invalid",llid_info->llidIndex);
	        return EPON_NORMAL_ERROR;
		}
	}
    return EPON_SUCCESS;
}


/*______________________________________________________________________________
**	function name
**		show_mac_phy_staus
**	description:
**		show mac/phy status
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void show_mac_phy_staus(void)
{
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### mac #### %X : %X ##########" , (0xbfb6611c), READ_REG_WORD(0xbfb6611c));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### mac #### %X : %X ##########" , (0xbfb66204), READ_REG_WORD(0xbfb66204));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### mac #### %X : %X ##########" , (0xbfb6620c), READ_REG_WORD(0xbfb6620c));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### mac #### %X : %X ##########" , (0xbfb66120), READ_REG_WORD(0xbfb66120));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### mac #### %X : %X ##########" , (0xbfb66360), READ_REG_WORD(0xbfb66360));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### phy #### %X : %X ##########" , (0xbfaf129c), READ_REG_WORD(0xbfaf129c));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### phy #### %X : %X ##########" , (0xbfaf12a0), READ_REG_WORD(0xbfaf12a0));
	WRITE_REG_WORD(0xbfaf1070 , 0x1);	
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### phy #### %X : %X ##########" , (0xbfaf1078), READ_REG_WORD(0xbfaf1078));
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR," #### phy #### %X : %X ##########" , (0xbfaf1020), READ_REG_WORD(0xbfaf1020));
}

/*______________________________________________________________________________
**	function name
**		epon_mpcp_rcv_register_msg_handler
**	description:
**		handle mpcp register message
**	parameters:
**		index
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		get_llid_hw_discv_state
**		epon_msg_route_dispatch
**		epon_rcv_nack_flag_msg_process
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_rcv_register_msg_handler(void *param)
{
	REG_e_llid_dscvry_sts discv_status;
    EPON_LLID_INFO_T *llid_info = NULL;
    __u32 llid_index = 0 ;//& MAX_LLID_INDEX;
    __u8  register_flag = 0;
    int ret = EPON_SUCCESS;

    if(param == NULL)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s][%d] param IS NULL\n",__FUNCTION__, __LINE__);
        return EPON_NORMAL_ERROR;
    }
    llid_index = *(__u32 *)param ;
    discv_status.Raw = 0;
    
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin llid %d",__FUNCTION__,llid_index);

	if(!EPON_CHECK_LLID_INDEX_VALID(llid_index))
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s llid %d invalid",__FUNCTION__,llid_index);
		return EPON_NORMAL_ERROR;
	}
	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);

    if(EPON_SUCCESS != get_llid_hw_discv_state(llid_info,&discv_status))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s get_llid_hw_discv_state fail",__FUNCTION__);
        return EPON_NORMAL_ERROR;
	}
			
    register_flag = discv_status.Bits.llid_rgstr_flg_sts;
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s discv_status %x register flag is %d",__FUNCTION__,discv_status.Raw,register_flag);

    switch(register_flag)
    {
    	case MPCP_REGISTER_FLAG_RE_REGISTER:
            ret = epon_msg_route_dispatch(EPON_MSG_RCV_RE_REGISTER, llid_info);
          	break;
     	case MPCP_REGISTER_FLAG_DE_REGISTER:
			//show_mac_phy_staus();
            ret = epon_msg_route_dispatch(EPON_MSG_RCV_DE_REGISTER, llid_info);
          	break;
    	case MPCP_REGISTER_FLAG_ACK:
            ret = epon_msg_route_dispatch(EPON_MSG_RCV_ACK_REGISTER, llid_info);
          	break;
    	case MPCP_REGISTER_FLAG_NACK:
            ret = epon_msg_route_dispatch(EPON_MSG_RCV_NACK_REGISTER, llid_info);;
          	break;
     	default:
          	break;           
    }

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    return ret;
}

int eponGetTrafficStatus(void)
{
    __u8 llidIndex = 0;
    __u8 isReg = 0;
	EPON_LLID_INFO_T *llid_info = NULL;
	
    for (llidIndex = 0;llidIndex < EPON_LLID_MAX_NUM ;llidIndex ++ ){
		EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);
        if (MPCP_STATE_REGISTERED == llid_info->eponMpcp.mpcpDiscState){
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

/*______________________________________________________________________________
**	function name
**		epon_mpcp_register_done
**	description:
**		check mpcp done
**	parameters:
**		index
**	global:
**		gpPonSysData
**	return:
**		0:fail
**		8:registered
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mpcp_register_done(void)
{
    __u8 llidIndex = 0;
	REG_e_llid_dscvry_ctrl dscvCtrl;
    EPON_LLID_INFO_T *llid_info = NULL;
    
    if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
    {
        return 0;
    }
    
        
	dscvCtrl.Raw = READ_REG_WORD(e_llid_dscvry_ctrl);
	llidIndex = dscvCtrl.Bits.tx_mpcp_llid_idx;
    EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"epon_mpcp_register_done index %d status: %d",llidIndex, llid_info->eponMpcp.mpcpDiscState);
    return llid_info->eponMpcp.mpcpDiscState;
    
}

