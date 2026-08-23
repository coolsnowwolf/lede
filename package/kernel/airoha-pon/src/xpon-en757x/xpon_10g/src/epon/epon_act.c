#include <linux/module.h>
#include <linux/kernel.h>
#include "epon/epon.h"
#include "epon/epon_reg.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include "common/xpon_global.h"
#include "common/xpondrv.h"
#include "epon/epon_compile_option_wrapper.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include "common/phy_if_wrapper.h"
#include "../inc/common/xpon_led.h"


extern uint32_t tx_favor_oam_enable;
extern void set_fix_reg_list(void);



char mpcpStateStr[MAX_MPCP_STATE_NUM][MAX_MPCP_STATE_STR_LEN] = 
{
	"WAIT",
	"REGISTERING",
	"REGISTER_REQUEST",
	"REGISTER_PENDING",
	"RETRY",
	"DENIED",
	"REGISTER_ACK",
	"NACK",
	"REGISTERED",
	"REMOTE_DEREGISTER",
	"LOCAL_DEREGISTER"
};
/*______________________________________________________________________________
**	function name
**		epon_get32
**	description:
**		get 4 byte of string
**	parameters:
**		cp
**	global:
**		None
**	return:
**		rval
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

__u32 epon_get32(__u8 *cp)
{
    __u32 rval;
	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;
    return rval;
}
/*______________________________________________________________________________
**	function name
**		epon_get16
**	description:
**		get 2 byte of string
**	parameters:
**		cp
**	global:
**		None
**	return:
**		x
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

__u16 epon_get16(__u8 *cp)
{
	__u16 x;
	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
} /* get16 */

/*______________________________________________________________________________
**	function name
**		epon_silent_timer_expires
**	description:
**		None
**	parameters:
**		data
**	global:
**		gp_epon_global_data
**	return:
**		None
**	call:
**		epon_change_state
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_silent_timer_expires(TIMER_FUN_PAAM data)
{	   
    __u8 llidIndex = 0;
    __u8 rgs_flag = 0 ;
    EPON_LLID_INFO_T *llid_info = NULL;
    llid_info = &(gp_epon_global_data->llid_entry[0]);
    for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM; llidIndex++)
	{
	    if(!EPON_CHECK_LLID_INDEX_VALID(llidIndex))
            continue;
        DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "epon_silent_timer_expires start epon: llidIndex=%d state=%d rgstAgainTimeout = %d", llidIndex, llid_info->eponMpcp.mpcpDiscState, llid_info->silent_time_left);

        if (llid_info->silent_time_left > 0) 
        {
            DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "Onu is in silent time out is %d rest, delete 1",llid_info->silent_time_left );
            llid_info->silent_time_left --;
            rgs_flag = 1;
			EPON_START_TIMER(gp_epon_global_data->silent_timer,EPON_SILENT_INTERVAL);
        }
        if ((rgs_flag == 0)  && (MPCP_STATE_DENIED == llid_info->eponMpcp.mpcpDiscState)) 
        {
            epon_change_state(llid_info->llidIndex,MPCP_STATE_REGISTERING);
            DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "XEpon phy up and silent time is 0, begin to response MPCP");
        }
    }
	
return;
}

/*______________________________________________________________________________
**	function name
**		epon_typeb_timer_expires
**	description:
**		for typeB expires
**	parameters:
**		data
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_typeb_timer_expires(TIMER_FUN_PAAM data)
{
	int ret = EPON_SUCCESS;

    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: epon_typeb_timer_expires clear flag and do los process");
    
	EPON_DRIVER_SET(TYPE_B_ENABLE,FALSE);

    if(XPON_PHY_GET(PON_GET_PHY_IS_SYNC) == PHY_FALSE)
    {
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: Phy true los");
    	ret = epon_msg_route_dispatch(EPON_MSG_TRUE_LOS,MSG_INPUT_NULL);
        if(EPON_SUCCESS != ret){
        	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s fail",__FUNCTION__);
            return;
        }
    }

    return;
}

/*______________________________________________________________________________
**	function name
**		epon_oamCnt_check_timer_expires
**	description:
**		for check oam cnt when disable/enable PHY TX Power
**	parameters:
**		data
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_oamCnt_check_timer_expires(TIMER_FUN_PAAM data)
{
	__u8 llidIndex = 0;
	EPON_LLID_INFO_T *llid_info = NULL;
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: timeout, start to check oam cnt when TX Power off to on by oam pkt");
    
	for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM; llidIndex++)
	{
	    if(!EPON_CHECK_LLID_INDEX_VALID(llidIndex))
            continue;
		EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);	
		DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"INFO: llid index=%d, oam Counter = %d in %d second", 
									llidIndex, gp_epon_global_data->oamCnt[llidIndex], EPON_CHECK_OAM_CNT_TME_DEFAULT);
	    if(gp_epon_global_data->oamCnt[llidIndex] < 5 
			&& llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTERED)
		{
			DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"INFO: llid index=%d do de register to change ONU state", llidIndex);
			change_pon_led_status(LED_FLICKER);
			epon_msg_route_dispatch(EPON_MSG_RCV_DE_REGISTER, llid_info);
		}
		else
		{
			if(llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTERED)
			{
				xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE, TRAFFIC_UP) ;
				DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"INFO: llid index=%d is online, PHY TX Power On action do not change ONU state", llidIndex);
			}
			else
				DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"INFO: llid index=%d is registering", llidIndex);
		}
    }
	

    return;
}

void start_oamCnt_ckeck_timer(void)
{
	__u8 llidIndex = 0;
	EPON_LLID_INFO_T *llid_info = NULL;
	int start_flag = 0;

	if ((XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_LOS_HAPPEN) /* no los and phy ready stats led should be configed */
			|| (XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_FALSE)) 
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"INFO: PHY Already LOS, do not start a timer to check ONU State On OLT");
		return;
	}
	//xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE, TRAFFIC_UP) ;
	memset(gp_epon_global_data->oamCnt , 0, sizeof(gp_epon_global_data->oamCnt));
    for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM; llidIndex++)
	{
	    if(!EPON_CHECK_LLID_INDEX_VALID(llidIndex))
            continue;
		EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);	

	    if(llid_info->eponMpcp.mpcpDiscState == MPCP_STATE_REGISTERED)
		{
			start_flag = 1;
			change_pon_led_status(LED_ON);
			break;
		}
		else{
			change_pon_led_status(LED_FLICKER);
		}
    }

	if(start_flag)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"INFO: start a timer(%d second) to check ONU State On OLT", EPON_CHECK_OAM_CNT_TME_DEFAULT);
		EPON_START_TIMER(gp_epon_global_data->oamCnt_timer,EPON_CHECK_OAM_CNT_TME_DEFAULT);
	}

}

/*______________________________________________________________________________
**	function name
**		epon_mac_trans
**	description:
**		trans mac for big/little endian
**	parameters:
**		tmMmac
**          macAddr
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_mac_trans(__u32* tmMmac, __u8* macAddr)
{
#ifdef __BIG_ENDIAN
	memcpy(macAddr+3 ,(__u8 *)tmMmac+1 , 3 );//big_endian
#else
	memcpy(macAddr+3,(__u8 *)tmMmac+2, 1);//little_endian
	memcpy(macAddr+4,(__u8 *)tmMmac+1, 1);//little_endian
	memcpy(macAddr+5,(__u8 *)tmMmac, 1);//little_endian
#endif /* __BIG_ENDIAN */
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_init_llid_mac
**	description:
**		init epon mac
**	parameters:
**		llid_info
**	global:
**		gp_epon_global_data
**	return:
**		None
**	call:
**		show_onu_mac_addr
**		epon_mac_trans
**	revision:
**		v1.0
**____________________________________________________________________________*/
int epon_llid_mac_is_set(EPON_LLID_INFO_T *llid_info)
{
    return (llid_info->macAddr[0] | llid_info->macAddr[1] | llid_info->macAddr[2] | \
            llid_info->macAddr[3] | llid_info->macAddr[4] | llid_info->macAddr[5]);
}

void epon_init_llid_mac(EPON_LLID_INFO_T *llid_info)
{
	__u32 tmp = 0;
	__u8 macAddr[6] = {0};
    
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"global mac address is");
    show_onu_mac_addr(gp_epon_global_data->onu_mac_addr);

    if(!epon_llid_mac_is_set(llid_info))
    {
        memcpy( macAddr ,gp_epon_global_data->onu_mac_addr,EPON_MAC_ADDRESS_LENTH);
        tmp = epon_get32(macAddr+2);
        tmp += llid_info->llidIndex;
        epon_mac_trans(&tmp,macAddr);
        memcpy(llid_info->macAddr , macAddr , EPON_MAC_ADDRESS_LENTH);
	}
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"llid %x mac address is",llid_info->llidIndex);
    show_onu_mac_addr(llid_info->macAddr);       
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_llid_enable
**	description:
**		enable epon llid
**	parameters:
**		llid_info
**	global:
**		None
**	return:
**		0: success
**	call:
**		epon_init_llid_mac
**		epon_set_mac_address
**		epon_mpcp_state_init
**		EPON_SET_REG_BIT
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_llid_enable(EPON_LLID_INFO_T *llid_info)
{
	__u32 llid_index = 0;
    int ret = EPON_SUCCESS;

    llid_index = llid_info->llidIndex;

	epon_init_llid_mac(llid_info);

    ret = epon_set_mac_address(llid_info);
    if(EPON_SUCCESS!=ret)
    {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: eponMacSetMacAddr fail llidIndex=%d", llid_index);
		//return EPON_NORMAL_ERROR;
	}
    
	ret = epon_mpcp_state_init(llid_index);
    if(EPON_SUCCESS!=ret)
    {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: eponMacSetMacAddr fail llidIndex=%d", llid_index);
		return EPON_NORMAL_ERROR;
	}
    
	if(llid_index < 8)
    	EPON_SET_REG_BIT((__u32)e_int_en,0x02<<llid_index);
	else if((llid_index > 7)&&(llid_index < 32)){
		EPON_SET_REG_BIT((__u32)e_int_en3,0x01<<(llid_index-8));
    }
	else {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: %s fail llidIndex=%d", __FUNCTION__, llid_index);
		return EPON_NORMAL_ERROR;
	}
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: epon_llid_enable set llid %d enable, global en %x",llid_index,READ_REG_WORD(e_int_en));
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_backup_last_llid_info
**	description:
**		back up llid info which did not send out
**	parameters:
**		p_last
**		p_current
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_backup_last_llid_info(EPON_LLID_INFO_T *p_last,EPON_LLID_INFO_T *p_current)
{
    p_last->eponMpcp.mpcpDiscState = p_current->eponMpcp.mpcpDiscState;
    p_last->silent_time_left = p_current->silent_time_left;	
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_recover_last_llid_info
**	description:
**		recover the last llid info before denied
**	parameters:
**		p_last
**		p_current
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_recover_last_llid_info(EPON_LLID_INFO_T *p_current,EPON_LLID_INFO_T *p_last)
{
    if(MPCP_STATE_DENIED == p_last->eponMpcp.mpcpDiscState)
    {
    	if(EPON_CHECK_SILENT_TIME_VALID(p_last->silent_time_left))
        {
        	p_current->silent_time_left = p_last->silent_time_left;
        }
        p_current->eponMpcp.mpcpDiscState = p_last->eponMpcp.mpcpDiscState;
    }
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_llid_init_by_index
**	description:
**		init llid by index in for ioop
**	parameters:
**		llid_index
**	global:
**		None
**	return:
**		None
**	call:
**		epon_backup_last_llid_info
**		epon_llid_enable
**		epon_recover_last_llid_info
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_llid_init_by_index(__u8 llid_index)
{
    pEPON_LLID_INFO_T llid_info = NULL;
    EPON_LLID_INFO_T llid_info_last = {0};

	if(!EPON_CHECK_LLID_INDEX_VALID(llid_index))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_llid_init_by_index EPON_CHECK_LLID_VALID fail index %x",llid_index);
        return EPON_INPUT_PARA_ERROR;
    }
	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);
    epon_backup_last_llid_info(&llid_info_last,llid_info);
    
	//memset(llid_info,0,sizeof(EPON_LLID_INFO_T));
    llid_info->llidIndex = llid_index; 
    epon_llid_enable(llid_info);
    
    epon_recover_last_llid_info(llid_info,&llid_info_last);
	llid_info->enableFlag= XPON_ENABLE;
    
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_init_all_llid
**	description:
**		init 32 llid
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**	call:
**		epon_llid_init_by_index
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_init_all_llid(void)
{
	__u8 llid_index = 0;
    
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s begin llid mask is %x ",__FUNCTION__,gp_epon_global_data->llid_bit_mask);
	for(llid_index = 0; llid_index < EPON_LLID_MAX_NUM; llid_index++)
	{
		if(EPON_CHECK_LLID_INDEX_VALID(llid_index))
	    {
			epon_llid_init_by_index(llid_index);
	    }
	}

    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_get_onu_mac_addr
**	description:
**		get onu mac from flash
**	parameters:
**		macAddr
**	global:
**		None
**	return:
**		0:success
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_get_onu_mac_addr(__u8 *macAddr)
{
	getPonMacfromflash(macAddr);
	return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_set_other_modules
**	description:
**		init other modules
**	parameters:
**		None
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**		enable_gdma2
**		epon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_set_other_modules(void)
{
    enable_gdma2();
    gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;
    epon_set_qdma_qos();	
}

/*______________________________________________________________________________
**	function name
**		epon_phy_ready_sw_init
**	description:
**		init epon status before response to discovergate
**	parameters:
**		data
**	global:
**		None
**	return:
**		0:success
**	call:
**		epon_event_report
**		epon_init_all_llid
**		QDMA_API_SET_OAM_MODIFY_FP_EN
**		FE_API_SET_CHANNEL_ENABLE
**		epon_dev_interrupt_mask_enable
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_phy_ready_sw_init(void * data)
{
	int ret = EPON_SUCCESS;
    int i = 0;
   // epon_get_onu_mac_addr(gp_epon_global_data->onu_mac_addr);
    epon_event_report(XMCS_EVENT_EPON_PHY_READY, 0);
    EPON_DRIVER_SET(EPON_ENABLE_FLAG,TRUE);
    epon_init_all_llid();
    /*enable qdma tx favor oam*/
	if(tx_favor_oam_enable)
    	QDMA_API_SET_OAM_MODIFY_FP_EN(ECNT_QDMA_WAN, QDMA_ENABLE);
	else
		QDMA_API_SET_OAM_MODIFY_FP_EN(ECNT_QDMA_WAN, QDMA_DISABLE);
    /*enable qdma tx favor oam*/
    /*enable GDMA2 tx channel*/
    for (i = 0; i < EPON_LLID_MAX_NUM; i++)
    {
        FE_API_SET_CHANNEL_ENABLE(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX, i, FE_ENABLE);
    }
    /*enable GDMA2 tx channel*/
	epon_dev_interrupt_mask_enable(); //enable means interrupt will be sent to cpu.
	/*Set EPON/XEPON_ASY report method*/
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DEFAULT_REPORT_METHOD,NULL,NULL);
    return ret;
}
/*______________________________________________________________________________
**	function name
**		epon_phy_ready_hw_init
**	description:
**		init epon hw vstatus before response to discovergate
**	parameters:
**		data
**	global:
**		None
**	return:
**		0:success
**	call:
**		epon_dev_set_tx_burst_mode
**		epon_dev_hw_dying_gasp_control
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_phy_ready_hw_init(void * data)
{
	int epon_logic_state = EPON_LOGIC_RESET_HOLD_OFF;
	uint8_t hw_dygasp_en = TRUE;
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_MAC_LOGIC_RESET,&epon_logic_state,NULL);
    epon_dev_set_tx_burst_mode(TRUE);
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_DYGASP_HW_EN,&hw_dygasp_en,NULL);

    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_start
**	description:
**		start to handle phy ready message
**	parameters:
**		data
**	global:
**		None
**	return:
**		None
**	call:
**		epon_msg_route_dispatch
**		epon_dev_hw_dying_gasp_control
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_start(unsigned long data)
{
	uint8_t ipg_val = EPON_IPG_DEFAULT;
	uint32_t mpcp_timeout_val = EPON_MPCP_TMOUT_INTVL;
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);
    epon_msg_route_dispatch(EPON_MSG_TRUE_READY,NULL);
	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: %s end global cfg is 0X%x enable is 0X%x  0X%x",
                   __FUNCTION__,READ_REG_WORD(e_glb_cfg),READ_REG_WORD(e_int_en), READ_REG_WORD(e_int_en3));

    /*set ipg 8*/
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_IPG,&ipg_val,NULL);
	/*set mpcp timeout to 1s*/
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_MPCP_TIMEOUT_VALUE,&mpcp_timeout_val,NULL);
	
	/*adjust RTT*/
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_RTT_ADJUST,NULL,NULL);
    
    set_fix_reg_list();
}
/*______________________________________________________________________________
**	function name
**		epon_check_queue_threshold_operate_done
**	description:
**		check epon queue threshold set success or not
**	parameters:
**		eponThresholdCfg
**	global:
**		None
**	return:
**		0: success
**		-1: fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_check_queue_threshold_operate_done(unsigned int *eponThresholdCfg)
{
	int RETRY = 10 ;
    unsigned int threshold_local = 0;
    
    while(RETRY--) 
	{
		threshold_local = READ_REG_WORD(e_rpt_qthld_cfg) ;
		if(threshold_local&EPON_QTHRESHLD_DONE) 
		{
			*eponThresholdCfg = threshold_local;
            return EPON_SUCCESS;
		} 
		udelay(1) ;
	}

    return EPON_NORMAL_ERROR;
}

/*______________________________________________________________________________
**	function name
**		epon_set_queue_threshold
**	description:
**		set epon queue threshold value
**	parameters:
**		p_queue_threshold
**	global:
**		None
**	return:
**		0: success
**		-1: fail
**	call:
**		epon_check_queue_threshold_operate_done
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_set_queue_threshold(eponQueueThreshold_t *p_queue_threshold)
{
	int ret = EPON_SUCCESS;
	unsigned int eponThresholdCfg = 0 ;

    DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s begin",__FUNCTION__);
	if (CHECK_QUEUE_THRESHOLD_PARAM_OUTBOUND(p_queue_threshold)) 
	{
		return EPON_NORMAL_ERROR;
	}

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_Q_THRESHLD_CFG,p_queue_threshold, NULL);

    ret = epon_check_queue_threshold_operate_done(&eponThresholdCfg);
    if(ret != EPON_SUCCESS)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_check_queue_threshold_operate_done fail");
        return ret;
    }

    DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s end",__FUNCTION__);
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_get_queue_threshold
**	description:
**		get epon queue threshold value
**	parameters:
**		p_queue_threshold
**	global:
**		None
**	return:
**		0: success
**		-1: fail
**	call:
**		epon_check_queue_threshold_operate_done
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_get_queue_threshold(eponQueueThreshold_t *p_queue_threshold)
{
	int ret ;
	unsigned int eponThresholdCfg = 0 ;
    
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s begin",__FUNCTION__);
	if(CHECK_QUEUE_THRESHOLD_PARAM_OUTBOUND(p_queue_threshold)) 
	{
		return EPON_NORMAL_ERROR;
	}

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_Q_THRESHLD_CFG,p_queue_threshold, NULL);
    
    ret = epon_check_queue_threshold_operate_done(&eponThresholdCfg);
    if(ret != EPON_SUCCESS)
    {	
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_check_queue_threshold_operate_done fail");
        return ret;
    }

    ret = (eponThresholdCfg&EPON_QTHRESHLD_VALUE_MASK)>>EPON_QTHRESHLD_VALUE_SHIFT;
	if (ret)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: set queue threhold val");
		p_queue_threshold->value = ret ;
	}

    DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"DEBUG: %s end",__FUNCTION__);
	return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_clear_all_queue_threshold_cfg
**	description:
**		clear epon queue threshold cfg
**	parameters:
**		None
**	global:
**		None
**	return:
**		0: success
**		-1: fail
**	call:
**		set_epon_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_clear_all_queue_threshold_cfg(void ){
	__u8 queueIndex = 0;
	__u8 llidIndex = 0;
	__u8 setIndex = 0;
    int ret = 0;
    eponQueueThreshold_t EponQThr;

	for(llidIndex = 0 ;llidIndex < EPON_LLID_MAX_NUM ;llidIndex++ ){
		for(setIndex = 0; setIndex< EPON_MAX_QUEUE_THRESHOLD;setIndex++){
			for(queueIndex = 0; queueIndex< EPON_MAX_QUEUE_PER_CHANNEL;queueIndex++){//set queue threshold
				EponQThr.channel = llidIndex;
				EponQThr.queue = queueIndex;
				EponQThr.thrIdx = setIndex;
				EponQThr.value = 0;
                ret = set_epon_queue_threshold(&EponQThr);
				if(EPON_SUCCESS != ret){
					DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: epon_set_queue_threshold error");	
					return ret;
				}
			}
		}
	}
    
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_disable_llid_mac_registers
**	description:
**		disable llid mac
**	parameters:
**		llidIndex
**	global:
**		None
**	return:
**		0: success
**		-1: fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_disable_llid_mac_registers(__u8 llidIndex)
{
	pEPON_LLID_INFO_T llid_info = NULL;
	REG_e_llid0_dscvry_sts sdcvSts;

	sdcvSts.Raw = 0;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "%s begins", __FUNCTION__);	
	if(!EPON_CHECK_LLID_INDEX_VALID(llidIndex))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"ERROR: epon_disable_llid_mac_registers EPON_CHECK_LLID_VALID fail index %d",llidIndex);
        return EPON_INPUT_PARA_ERROR;
    }
	EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);

	/* disable flag */
	llid_info->enableFlag = XPON_DISABLE;

	//DISABLE INT
	//change disc status to unregist	

    if(llidIndex < 8)
    {	
		EPON_CLEAR_REG_BIT((__u32)e_int_en,(1<<(llidIndex+1)));
    }else {
		EPON_CLEAR_REG_BIT((__u32)e_int_en3,(1<<(llidIndex-8)));
    }
	sdcvSts.Raw = READ_REG_WORD(g_epon_llid_dscv_stat[llidIndex]);
	sdcvSts.Bits.llid0_dscvry_sts = 0;
	WRITE_REG_WORD(g_epon_llid_dscv_stat[llidIndex], sdcvSts.Raw);
	
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_clear_all_llid_info
**	description:
**		clear all llid message
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		epon_disable_llid_mac_registers
**		xmcs_remove_llid
**		disable_gdma2_and_channel_retire
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_clear_all_llid_info(void)
{
	__u8 llidIndex = 0;
	for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM;llidIndex++){
        epon_disable_llid_mac_registers(llidIndex);
		xmcs_remove_llid(llidIndex);
	}
	return;
}
/*______________________________________________________________________________
**	function name
**		epon_stop
**	description:
**		clear epon status when receive deregister
**	parameters:
**		param
**	global:
**		None
**	return:
**		0:success
**	call:
**		epon_dev_tx_rx_disable
**		epon_clear_all_queue_threshold_cfg
**		epon_clear_all_llid_info
**		stop_omci_oam_monitor
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_stop(void * data)
{
	int ret = EPON_SUCCESS;

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);

    EPON_DRIVER_CLEAR(EPON_ENABLE_FLAG);
    EPON_SET_LINK_STATUS(PON_LINK_STATUS_OFF);
    
	epon_dev_tx_rx_disable();

    ret = epon_clear_all_queue_threshold_cfg();
    if(EPON_SUCCESS != ret)
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_clear_all_queue_threshold_cfg process fail");

    epon_clear_all_llid_info();
    
    stop_omci_oam_monitor();
    
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_mac_reset
**	description:
**		reset epon mac
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**	call:
**		EPON_RESET_REG_BIT_THEN_WAIT
**		EPON_CLEAR_REG_BIT
**		EPON_RESET_REG_BIT_THEN_WAIT
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_mac_reset(void)
{
	int epon_logic_state = EPON_LOGIC_RESET_HOLD_ON;
	
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);
    
    WRITE_REG_WORD(e_trx_adjust_time1,0xfff6);
    WRITE_REG_WORD(e_trx_adjust_time2,EPON_REG_ADJUST_TIME2_DEFAULT);
    WRITE_REG_WORD(e_txfetch_cfg,0x2a03e8);
	
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_MAC_LOGIC_RESET,&epon_logic_state,NULL);
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_reset
**	description:
**		reset epon
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**	call:
**		epon_mac_reset
**		xpon_reset_qdma_tx_buf
**		disable_gdma2_and_channel_retire
**		enable_gdma2
**	revision:
**		v1.0
**____________________________________________________________________________*/


int epon_reset(void * param)
{
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);

	disable_gdma2_and_channel_retire();
	
	epon_mac_reset();

	xpon_reset_qdma_tx_buf();

    enable_gdma2();
    
    
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_change_state
**	description:
**		change epon status
**	parameters:
**		llid_index
**		new_state
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_change_state(__u8 llid_index, __u8 new_state)
{
	pEPON_LLID_INFO_T llid_info = NULL;
	if(!EPON_CHECK_LLID_INDEX_VALID(llid_index))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_change_state EPON_CHECK_LLID_VALID fail index %x",llid_index);
        return;
    }
	
	EPON_GET_LLID_BY_INDEX(llid_info,llid_index);	

    llid_info->eponMpcp.mpcpDiscState = new_state;
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_check_current_state
**	description:
**		check epon status
**	parameters:
**		llid_info
**		state
**	global:
**		mpcpStateStr
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

bool epon_check_current_state(pEPON_LLID_INFO_T llid_info, __u8 state)
{
	__u8 current_state = llid_info->eponMpcp.mpcpDiscState;
	if(current_state == state)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: current state is %s",mpcpStateStr[state]);
        return TRUE;
    }
    
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: current state is not %s, actural state %s",
        mpcpStateStr[state],mpcpStateStr[current_state]);
    return FALSE;
}

