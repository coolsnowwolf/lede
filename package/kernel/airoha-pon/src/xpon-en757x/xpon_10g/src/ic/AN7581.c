/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/


#include "xepon_mac_c_header_en7581.h"
#include "common/union_ic_init.h"

/**********************************************************************************************
**************************************Extern Declare*******************************************
***********************************************************************************************/

extern char get_onutype(void);

/**********************************************************************************************
**************************************Private Define*******************************************
***********************************************************************************************/
static uint32_t llids_cfg_map[8] = {
	/* offset */
	0x6050,//e_llid0_3_cfg,
	0x6054,//e_llid4_7_cfg,
	0x6058,//e_llid8_11_cfg,
	0x605c,//e_llid12_15_cfg,
	0x6060,//e_llid16_19_cfg,
	0x6064,//e_llid20_23_cfg,
	0x6068,//e_llid24_27_cfg,
	0x606c,//e_llid28_31_cfg,
};

#define SCU_PON_MAC_RESET          (1<<31)

static int an7581_epon_triple_churning_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum);
static int an7581_epon_dpoe_decrypt_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum);

/********************************************************************************************
 *********************************function table*********************************************
 ********************************************************************************************/

ic_func_tbl_t an7581_action[]=
{
{REGISTER_ACTION_EPON_CHECK_MAC_TX_MBI_STOP, 	an7581_epon_check_mac_tx_mbi_stop},
{REGISTER_ACTION_EPON_CHECK_MAC_TX_MPI_STOP, 	an7581_epon_check_mac_tx_mpi_stop},
{ONLINE_ACTION_EPON_GET_OLT_MAC_ADDRESS, 		an7581_epon_get_olt_mac_address},
{REGISTER_ACTION_EPON_INT_ENABLE, 				an7581_epon_ext_int_enable},
{REGISTER_ACTION_EPON_SET_Q_THRESHLD_CFG,		an7581_epon_set_queue_threshold_cfg},
{REGISTER_ACTION_EPON_GET_Q_THRESHLD_CFG,		an7581_epon_get_queue_threshold_cfg},
{REGISTER_ACTION_EPON_GET_WAN_CONF, 			an7581_epon_get_wan_cfg},
{PACKETS_ACTION_EPON_QOS_TX_RATE_METER_CFG, 	an7581_epon_qos_tx_rate_meter_cfg},
{REGISTER_ACTION_EPON_DEFAULT_REPORT_METHOD,	an7581_epon_set_default_report_method},
{REGISTER_ACTION_EPON_SET_IPG,					an7581_epon_set_ipg},
{REGISTER_ACTION_EPON_MPCP_TIMEOUT_VALUE,		an7581_epon_set_mpcp_tmout_intvl},
{REGISTER_ACTION_EPON_RTT_ADJUST,				an7581_epon_rtt_adjust},
{REGISTER_ACTION_EPON_GET_REPORT_BITMAP,		an7581_epon_get_llid_report_bitmap},
{REGISTER_ACTION_EPON_INIT_DSCVRY_STS,			an7581_epon_init_discovery_status},
{REGISTER_ACTION_EPON_DOWN_KEY_CHANGE,			an7581_epon_downstream_key_change_handler},
{REGISTER_ACTION_EPON_DOWN_KEY_MISS,			an7581_epon_downstream_key_miss_handler},
{REGISTER_ACTION_EPON_UP_KEY_CHANGE,			an7581_epon_upstream_key_change_handler},
{REGISTER_ACTION_EPON_SET_LLID_KEY,				an7581_epon_set_llid_key},
{REGISTER_ACTION_EPON_GET_LLID_KEY, 			an7581_epon_get_llid_key},
{REGISTER_ACTION_EPON_SET_LLID_TX_FEC,			an7581_epon_set_llid_tx_fec},
{REGISTER_ACTION_EPON_GET_LLID_TX_FEC,			an7581_epon_get_llid_tx_fec},
{REGISTER_ACTION_EPON_SET_LLID_THRSHLD_NUM, 	an7581_epon_set_llid_thrshld_num},
{REGISTER_ACTION_EPON_GET_LLID_THRSHLD_NUM, 	an7581_epon_get_llid_thrshld_num},
{PACKETS_ACTION_EPON_TX_BUFF_USAGE_CFG, 		an7581_epon_set_tx_buff_usage},
{REGISTER_ACTION_EPON_SET_10G_LLID_KEY,			an7581_epon_set_10G_llid_key},
{REGISTER_ACTION_EPON_GET_10G_LLID_KEY, 		an7581_epon_get_10G_llid_key},
{REGISTER_ACTION_EPON_SET_DPOE_LLID_KEY,		an7581_epon_set_dpoe_decrypt_llid_key},
{REGISTER_ACTION_PON_MAC_SCU_RESET, 			an7581_pon_mac_scu_reset},
{REGISTER_ACTION_EPON_MAC_LOGIC_RESET,			an7581_epon_mac_logic_reset},
{ONLINE_ACTION_PON_GET_ONU_MODE_TYPE,			an7581_pon_get_onu_mode_type},
{REGISTER_ACTION_EPON_SET_DYGASP_HW_EN, 		an7581_epon_set_dygasp_hw_en},
{REGISTER_ACTION_EPON_GET_DYGASP_HW_EN, 		an7581_epon_get_dygasp_hw_en},
{REGISTER_ACTION_EPON_SET_DYGASP_NUM,			an7581_epon_set_dygasp_num},
{REGISTER_ACTION_EPON_GET_DYGASP_NUM,			an7581_epon_get_dygasp_num},

/*********************debug action function********************************/
{REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT,		an7581_epon_get_debug_statistic_count},
{REGISTER_ACTION_EPON_DBG_GET_STATE,			an7581_epon_get_register_state},
{REGISTER_ACTION_EPON_DBG_SET_SNIFFER,			an7581_epon_set_sniffer},
{REGISTER_ACTION_EPON_DBG_SET_STATIC_RPT,		an7581_epon_set_static_report},
{REGISTER_ACTION_EPON_DBG_SET_BAND_UTILIZATION,	an7581_epon_set_band_utilization},
{REGISTER_ACTION_EPON_DBG_SET_TX_RATE_ANALYZE,	an7581_epon_tx_rate_analyze},
/*******************only defined, but not used*****************************/
{REGISTER_ACTION_EPON_REPORT_QSIZEADJS,			an7581_epon_set_mac_report_qsizeadjs},
{REGISTER_ACTION_EPON_REPORT_QSIZEADJS_FEC, 	an7581_epon_set_mac_report_qsizeadjs_for_fec},
{REGISTER_ACTION_EPON_SET_REPORT_BITMAP,		an7581_epon_set_llid_report_bitmap},

/*************************please add above*********************************/
{MAX_ACTION_NUM, NULL}

};

/**********************************************************************************************
*************************************Function Define*******************************************
***********************************************************************************************/

int an7581_epon_check_mac_tx_mbi_stop(const void *in, void *out)
{
	return ((READ_REG_WORD(e_glb_sts1) & (0x10000000))?TRUE:FALSE);
}

int an7581_epon_check_mac_tx_mpi_stop(const void *in, void *out)
{
	return ((READ_REG_WORD(e_glb_sts1) & (0x40000000))?TRUE:FALSE);
}

int an7581_epon_get_olt_mac_address(const void *in, void *out)
{
    uint32_t mac_addr_low = 0;
    uint32_t mac_addr_high  = 0;
	EPON_LLID_INFO_T *llidEntry_p = (EPON_LLID_INFO_T *)out;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"enter epon_get_olt_mac_address");

	if(llidEntry_p == NULL){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s] llidEntry_p == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

    mac_addr_low  = e_olt_macaddr_l_GET_olt_macaddr_l(e_olt_macaddr_l);
    mac_addr_high = e_olt_macaddr_l_GET_olt_macaddr_l(e_olt_macaddr_h);
	llidEntry_p->oltMacAddr[0] = (uint8_t)(mac_addr_high>>8);
	llidEntry_p->oltMacAddr[1] = (uint8_t)(mac_addr_high>>0);
	llidEntry_p->oltMacAddr[2] = (uint8_t)(mac_addr_low>>24);
	llidEntry_p->oltMacAddr[3] = (uint8_t)(mac_addr_low>>16);
	llidEntry_p->oltMacAddr[4] = (uint8_t)(mac_addr_low>>8);
	llidEntry_p->oltMacAddr[5] = (uint8_t)(mac_addr_low>>0);

	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,">>>epon_get_olt_mac_address mac :%x %x %x %x %x %x\n",llidEntry_p->oltMacAddr[0],llidEntry_p->oltMacAddr[1],llidEntry_p->oltMacAddr[2],
		llidEntry_p->oltMacAddr[3],llidEntry_p->oltMacAddr[4],llidEntry_p->oltMacAddr[5]);
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG," epon_get_olt_mac_address %x %x success",mac_addr_high,mac_addr_low);
	return EPON_SUCCESS;
}

int an7581_epon_ext_int_enable(const void *in, void *out)
{
	uint32_t raw = 0;
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	raw = *(uint32_t *)out;
	raw = raw | US_KEY_CHANGE_INT | DS_KEY_MISS_INT | DS_KEY_CHANGE_INT;
	*(uint32_t *)out = raw;
	return EPON_SUCCESS;
}

int an7581_epon_set_queue_threshold_cfg(const void *in, void *out)
{
	unsigned int eponThresholdCfg = 0 ;
	eponQueueThreshold_t *queue_threshold = (eponQueueThreshold_t *)in;
	int threshold_chnn_shift = 24;
	int threshold_chnn_mask = 0x1f;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	
	eponThresholdCfg = EPON_Q_THRESHLD_RWCMD | \
						((queue_threshold->value & EPON_Q_THRESHLD_VALUE_MASK)<<EPON_Q_THRESHLD_VALUE_SHIFT) |   \
						((queue_threshold->thrIdx & EPON_Q_THRESHLD_TYPE_MASK)<<EPON_Q_THRESHLD_TYPE_SHIFT) |   \
						((queue_threshold->channel &threshold_chnn_mask )<<threshold_chnn_shift) |	\
						((queue_threshold->queue &EPON_Q_THRESHLD_QUEUE_MASK)<<EPON_Q_THRESHLD_QUEUE_SHIFT);
	
	WRITE_REG_WORD(e_rpt_qthld_cfg , eponThresholdCfg);

	return EPON_SUCCESS;
}

int an7581_epon_get_queue_threshold_cfg(const void *in, void *out)
{
	unsigned int eponThresholdCfg = 0 ;
	eponQueueThreshold_t *queue_threshold = (eponQueueThreshold_t *)in;
	int threshold_chnn_shift = 24;
	int threshold_chnn_mask = 0x1f;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	
	eponThresholdCfg = ((queue_threshold->thrIdx & EPON_Q_THRESHLD_TYPE_MASK)<<EPON_Q_THRESHLD_TYPE_SHIFT) |	\
						((queue_threshold->channel & threshold_chnn_mask)<<threshold_chnn_shift) |	  \
						((queue_threshold->queue & EPON_Q_THRESHLD_QUEUE_MASK)<<EPON_Q_THRESHLD_QUEUE_SHIFT);
	
	WRITE_REG_WORD(e_rpt_qthld_cfg , eponThresholdCfg);

	return EPON_SUCCESS;
}


int an7581_epon_get_wan_cfg(const void *in, void *out)
{
	uint32_t wan_raw = 0;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	
	wan_raw = GET_WAN_CONF();

	*(uint32_t *)out = wan_raw;

	return EPON_SUCCESS;
}

int an7581_epon_qos_tx_rate_meter_cfg(const void *in, void *out)
{
	QDMA_TxRateMeter_T *txRateMeter = (QDMA_TxRateMeter_T *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	txRateMeter->txRateMeterTimeSlice = 0x3e8;
	return EPON_SUCCESS;
}

int an7581_epon_set_default_report_method(const void *in, void *out)
{
	WRITE_REG_WORD(e_rpt_bmap, 0xFFFF);
	if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON) || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)){
		//use default e_rpt_cfg value 
	}else{
		WRITE_REG_WORD(e_rpt_cfg, 0x1);
	}

	return EPON_SUCCESS;
}

int an7581_epon_set_ipg(const void *in, void *out)
{
	uint8_t value = 0;
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	value = *(uint8_t *)in;
	
	e_tx_cal_cnst_SET_default_ovrhd(e_tx_cal_cnst, value);
	
	return EPON_SUCCESS;
}

int an7581_epon_set_mpcp_tmout_intvl(const void *in, void *out)
{
	uint32_t value = 0;
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	value = *(uint32_t *)in;
    e_mpcp_timeout_intvl_SET_mpcp_timeout_intvl(e_mpcp_timeout_intvl, (value & 0x3FF));

	return EPON_SUCCESS;
}

int an7581_epon_rtt_adjust(const void *in, void *out)
{
    if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON){
        e_trx_adjust_time2_SET_rx_tmstp_adj(e_trx_adjust_time2, RX_TMSTP_ADJ);
    }else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON){
        e_trx_adjust_time2_SET_rx_tmstp_adj(e_trx_adjust_time2, D2G_RX_TMSTP_ADJ);
    }else if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON){
        e_trx_adjtime4_SET_d10g_rx_tsadj(e_trx_adjtime4, D10G_RX_TSADJ);
    }else if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){
        e_trx_adjtime4_SET_d10g_rx_tsadj(e_trx_adjtime4, D10G_RX_TSADJ);
        e_trx_adjtime3_SET_u10g_tx_stmadj(e_trx_adjtime3, U10G_TX_STMADJ);
    }

	return EPON_SUCCESS;
}

int an7581_epon_get_llid_report_bitmap(const void *in, void *out)
{
	epon_llid_report_bitmap_t *report_bitmap = (epon_llid_report_bitmap_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	
    if(report_bitmap->llidIndex >= EPON_LLID_MAX_NUM){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]llid index greater %d!\n",__FUNCTION__, EPON_LLID_MAX_NUM);
		return EPON_NORMAL_ERROR;
	}
	report_bitmap->bitmap = e_rpt_bmap_GET_rpt_bitmap_set(e_rpt_bmap);
	return EPON_SUCCESS;
}


int an7581_epon_init_discovery_status(const void *in, void *out)
{
	int i = 0;
	for(i = 0;i < 32;i++){
		g_epon_llid_dscv_stat[i] = e_llid0_31_dscvry_sts(i);
	}

	return EPON_SUCCESS;
}


int an7581_epon_downstream_key_change_handler(const void *in, void *out)
{
	uint32_t key_index_1 = 0;
	uint32_t key_index_2 = 0;
	uint8_t i=0;

    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
	
	key_index_1 = READ_REG_WORD(e_ds_keychg);
	key_index_2 = READ_REG_WORD(e_ds_keychg2);

	WRITE_REG_WORD(e_ds_keychg , 0xffffffff);/*clear status bit*/	
	WRITE_REG_WORD(e_ds_keychg2 , 0xffffffff);/*clear status bit*/	

	for(i=0;i<52;i++){
		if(i<32){
			if(key_index_1&(1<<i)){
				DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s llid index:%d changed downstream keyindex\n",__FUNCTION__,i);				
			}
		}else{
			if(key_index_2&(1<<(i-32))){
				DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s llid index:%d changed downstream keyindex\n",__FUNCTION__,i);
			}
		}			
	}	
    return EPON_SUCCESS;
}

int an7581_epon_downstream_key_miss_handler(const void *in, void *out)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);
    return EPON_SUCCESS;
}

int an7581_epon_upstream_key_change_handler(const void *in, void *out)
{
    DEBUG_MSG_EPON(MSG_LEVEL_INTERRUPT,"DEBUG: %s begin",__FUNCTION__);	
    return EPON_SUCCESS;
}

int an7581_epon_set_llid_key(const void *in, void *out)
{
    /* This function for 1G setting churning */
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL; /*network byte order buffer*/
    uint32_t val = 0;
	uint32_t llidcfg_reg = 0;
    uint32_t keyValue = 0;
    uint8_t* p = (uint8_t*)(&keyValue);
    int cnt = 100;
    REG_e_enckey_cfg cfg = {0};
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
	key = io_info->info;
	
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

	llidcfg_reg = llids_cfg_map[llidIndex/4];
    /* enable llidx decrypt */   
    val = READ_REG_WORD(llidcfg_reg);
    val |= (0x4<<((llidIndex-(llidIndex/4)*4)*8));
    WRITE_REG_WORD(llidcfg_reg, val);

    /* setting key value */   
	p[2] = key[0];
	p[1] = key[1];
	p[0] = key[2];
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "[%s %d]key: 0x%x.", __FUNCTION__, __LINE__, keyValue);	
    e_key_value_SET_key_value(e_key_value, keyValue);

    /* config key */
    //e_security_key_cfg_SET_key_llid_index(e_security_key_cfg, llidIndex);
    //e_security_key_cfg_SET_key_idx(e_security_key_cfg, keyIndex);
    //e_security_key_cfg_SET_key_rwcmd(e_security_key_cfg, 1);
    cfg.Bits.enckey_rwcmd = 1;
	cfg.Bits.enckey_keyidx = keyIndex;
	cfg.Bits.enckey_llididx = llidIndex;
	cfg.Bits.enckey_dwidx = 0;
	WRITE_REG_WORD(e_security_key_cfg, cfg.Raw);
    gp_epon_global_data->llid_entry[llidIndex].llid_key_index = keyIndex;

    /* polling done bit */
    while(cnt){
        if(e_security_key_cfg_GET_key_rwcmd_done(e_security_key_cfg)){
            return 0;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Done bit error.", __FUNCTION__, __LINE__);
    return -1;
}

int an7581_epon_get_llid_key(const void *in, void *out)
{
    int cnt = 100;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL; 
	uint32_t keyValue = 0;
	uint8_t* p = NULL;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
	key = io_info->info;
	
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);	
        return -1;
    }
    /* config key */
    e_security_key_cfg_SET_key_llid_index(e_security_key_cfg, llidIndex);
    e_security_key_cfg_SET_key_idx(e_security_key_cfg, keyIndex);
    e_security_key_cfg_SET_key_rwcmd(e_security_key_cfg, 0);

    /* polling done bit */
    while(cnt){
        if(e_security_key_cfg_GET_key_rwcmd_done(e_security_key_cfg)){
            keyValue = e_key_value_GET_key_value(e_key_value);
			DEBUG_MSG_EPON(0, "[%s %d]key: 0x%x.", __FUNCTION__, __LINE__, keyValue);
			keyValue = htonl(keyValue);
            p = (uint8_t*)&keyValue;
            memcpy(key, p+1, 3);
            return 0;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Done bit error.", __FUNCTION__, __LINE__);
    return -1;
}


int an7581_epon_set_llid_tx_fec(const void *in, void *out)
{
	uint8_t llidIndex = 0;
	uint8_t fecFlag = 0;
    uint32_t val = 0;
    uint32_t feFec = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;
	uint32_t llidcfg_reg = 0;
	
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	fecFlag = io_info->param0;
	
    if((llidIndex>=EPON_LLID_MAX_NUM)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);	
        return -1;
    }
	llidcfg_reg = llids_cfg_map[llidIndex/4];
	val = READ_REG_WORD(llidcfg_reg);
	feFec = READ_FE_REG(0xbfb57130);
	
    if(fecFlag==ENABLE){
        val |= (0x8<<((llidIndex-(llidIndex/4)*4)*8));
        feFec |= (1<<llidIndex);
    }else{
        val &= ~(0x8<<((llidIndex-(llidIndex/4)*4)*8));
        feFec &= ~(1<<llidIndex);
    }
	
    WRITE_REG_WORD(llidcfg_reg, val);
	if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON
		&& feFec > 0){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]10G_10G EPON mode should set QDMA[%x] DBA Report FEC OFF.", 
								__FUNCTION__, __LINE__,0xbfb57130);
		WRITE_FE_REG(0xbfb57130, 0);
	}
	else{
    	WRITE_FE_REG(0xbfb57130, feFec);
	}

    return 0;
}

int an7581_epon_get_llid_tx_fec(const void *in, void *out)
{
    uint32_t val = 0;
	uint8_t llidIndex = 0;
	uint8_t fecFlag = 0;
	uint32_t llidcfg_reg = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidIndex = io_info->llidIndex;
    
    if(llidIndex>=EPON_LLID_MAX_NUM){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);	
        return -1;
    }

	llidcfg_reg = llids_cfg_map[llidIndex/4];
	val = READ_REG_WORD(llidcfg_reg);

	/* up 10G, tx fec is forced on */
    if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
	{
        fecFlag = 1;
        return 0;
    }
	else
	{
	    if(val & (0x8<<((llidIndex-(llidIndex/4)*4)*8))){
	        fecFlag = 1;
	    }else{
	        fecFlag = 0;
	    }
	}

	io_info->param0 = fecFlag;
    return 0;
}

int an7581_epon_set_llid_thrshld_num(const void *in, void *out)
{
    uint32_t val = 0;   
	uint8_t llidIndex = 0;
	uint8_t num = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	num = io_info->param0;

    if((llidIndex>=EPON_LLID_MAX_NUM)||(num>3)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }
    if(llidIndex<16){
        val = READ_REG_WORD(e_rpt_cfg);
		val &= ~(3<<(llidIndex*2));
        val |= (num<<(llidIndex*2));
        WRITE_REG_WORD(e_rpt_cfg, val);
    }else{
        val = READ_REG_WORD(e_rpt_cfg2);
		val &= (3<<((llidIndex-16)*2));
        val |= (num<<((llidIndex-16)*2));
        WRITE_REG_WORD(e_rpt_cfg2, val);
    }

    return 0;
}

int an7581_epon_get_llid_thrshld_num(const void *in, void *out)
{
    uint32_t val = 0;
    uint32_t offset = 0;
	uint8_t llidIndex = 0;
	uint8_t num = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidIndex = io_info->llidIndex;

    if(llidIndex>=EPON_LLID_MAX_NUM){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

    if(llidIndex<16){
        offset = llidIndex*2;
        val = READ_REG_WORD(e_rpt_cfg);
    }else{
        offset = (llidIndex-16)*2;
        val = READ_REG_WORD(e_rpt_cfg2);
    }
    num = ((val>>offset)&0x3);

	io_info->info[0] = num;

    return 0;    
}

int an7581_epon_set_tx_buff_usage(const void *in, void *out)
{
	uint32_t llidMask = 0;
	uint8_t llidIndex = 0;
	uint8_t llidCnt = 0;
#if 0
	uint32_t value = 0;
#endif
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidMask = *(uint32_t *)in;
	
	for( llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM; llidIndex++ ){
		if(((1<<llidIndex)&llidMask) != 0)
			llidCnt++;
	}

	gpWanPriv->activeChannelNum = llidCnt;
#if 0
	if(llidCnt <= 1){
		if (gpWanPriv->devCfg.flags.isQosUp == 1)
			WRITE_REG_WORD(0xBFB51890, 0x800020C0);
		else
			WRITE_REG_WORD(0xbfb51890,0);
	}
	else{
		value = 0x80/llidCnt;
		if (gpWanPriv->devCfg.flags.isQosUp == 1 && value > 0x20)
			value = 0x20;
		value <<= 8;
		value |= 0x80000080;
		WRITE_REG_WORD(0xbfb51890,value);
	}
#endif
    return 0;
}

static int an7581_epon_triple_churning_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum)
{
    /* This function for 10G/1G, 10G/10G setting churning */
    REG_e_enckey_cfg cfg = {0};
    int cnt = 100;
    
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(keyNum>2)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }
    
    /* config key */
    cfg.Bits.enckey_rwcmd = opt;
    cfg.Bits.enckey_keyidx = keyIndex;
    cfg.Bits.enckey_llididx = llidIndex;
    cfg.Bits.enckey_dwidx = keyNum;
    WRITE_REG_WORD(e_security_key_cfg, cfg.Raw);

    /* polling done bit */
    while(cnt){
        if(e_security_key_cfg_GET_key_rwcmd_done(e_security_key_cfg)){
            return 0;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Done bit error.", __FUNCTION__, __LINE__);
    
    return -1;
}

int an7581_epon_set_10G_llid_key(const void *in, void *out)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t keyValue = 0;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL;
	uint32_t llidcfg_reg = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
    key = io_info->info;
	
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

    /* enable llidx decrypt */   
	llidcfg_reg = llids_cfg_map[llidIndex/4];
    val = READ_REG_WORD(llidcfg_reg);
    val |= (0x4<<((llidIndex-(llidIndex/4)*4)*8));
    WRITE_REG_WORD(llidcfg_reg, val);

    for(i=0;i < EPON_10G_MAX_KEY_NUM; i++){
        /* setting key value */
        keyValue = ((*key)<<16) | ((*(key+1))<<8) | (*(key+2));
        key += 3;
        e_key_value_SET_key_value(e_key_value, keyValue);
        if(an7581_epon_triple_churning_key_cfg(EPON_WRITE_KEY, llidIndex, keyIndex, i)){
        	DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Set key Fail.", __FUNCTION__, __LINE__);
            return -1;
        }
    }

    return 0;
}

int an7581_epon_get_10G_llid_key(const void *in, void *out)
{
    uint8_t i = 0;
    uint32_t keyValue = 0;
    uint8_t* p = NULL;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
    key = io_info->info;
    
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key == NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

    for(i=0; i<EPON_10G_MAX_KEY_NUM; i++){
        if(an7581_epon_triple_churning_key_cfg(EPON_READ_KEY, llidIndex, keyIndex, i)){
        	DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Get key Fail.", __FUNCTION__, __LINE__);
            return -1;
        }
        keyValue = e_key_value_GET_key_value(e_key_value);
        p = (uint8_t*)&keyValue;
        memcpy(key, p+1, EPON_THURNING_KEY_LEN);
        key += 3;
        DEBUG_MSG_EPON(0, "[%s %d]key%d: 0x%x.", __FUNCTION__, __LINE__, i, keyValue);
    }
    
    return 0;
}
static int an7581_epon_dpoe_decrypt_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum)
{
    /* This function for dpoe */
    REG_e_security_key_cfg cfg = {0};
    int cnt = 100;
    
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(keyNum>3)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

    /* config key */
    cfg.Bits.key_rwcmd = opt;
    cfg.Bits.key_idx = keyIndex;
    cfg.Bits.key_llid_index = llidIndex;
    cfg.Bits.key_dw_indx = keyNum;
    WRITE_REG_WORD(e_security_key_cfg, cfg.Raw);

    /* polling done bit */
    while(cnt){
        if(e_security_key_cfg_GET_key_rwcmd_done(e_security_key_cfg)){
            return 0;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Done bit error.", __FUNCTION__, __LINE__);
    
    return -1;
}

int an7581_epon_set_dpoe_decrypt_llid_key(const void *in, void *out)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t keyValue = 0;
    int cnt = 100;
	REG_e_desec_srcaddr_cfg reg_desec_srcaddr_cfg = {0}; 
    pEPON_LLID_INFO_T llid_info = NULL;
    uint32_t olt_mac_addr_low = 0;
    uint32_t olt_mac_addr_high  = 0;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL;
	uint32_t llidcfg_reg = 0;
	uint32_t reg_reset_val = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
    key = io_info->info;
	
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], llidindex:%d keyindex:%d\n", __FUNCTION__, __LINE__,llidIndex,keyIndex);

    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }
	
	reg_reset_val = GET_SCU_RSTCTRL1();
	
    if(reg_reset_val>>31){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]reseting register, can't crypt.", __FUNCTION__, __LINE__);
		return 0;
	}
	else{
	    /* enable llidx decrypt */   
		llidcfg_reg = llids_cfg_map[llidIndex/4];
	    val = READ_REG_WORD(llidcfg_reg);
	    val |= (0x6<<((llidIndex-(llidIndex/4)*4)*8));//0x4 tripble churning ,0x6 AES
	    WRITE_REG_WORD(llidcfg_reg, val);

		/* set decrypt mode ctr */		
		if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON){		
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], SET_decrpt_mode: CFB mode \n", __FUNCTION__, __LINE__);
			e_crpt_cfg_SET_decrpt_mode(e_crpt_cfg,3);// CFB mode 
		}else{	
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], SET_decrpt_mode: CTR mode \n", __FUNCTION__, __LINE__);
			e_crpt_cfg_SET_decrpt_mode(e_crpt_cfg,2);// ctr 
			/* config SA address */
			EPON_GET_LLID_BY_INDEX(llid_info,llidIndex);

			olt_mac_addr_low = epon_get32(llid_info->oltMacAddr+2);
			olt_mac_addr_high = epon_get16(llid_info->oltMacAddr);
			e_desec_srcaddr_val_SET_desec_srcaddr_value_l(e_desec_srcaddr_val,olt_mac_addr_low);
		    reg_desec_srcaddr_cfg.Bits.desec_srcaddr_rwcmd = 1;
		    reg_desec_srcaddr_cfg.Bits.desec_srcaddr_llid_idx = llidIndex;
		    reg_desec_srcaddr_cfg.Bits.desec_srcaddr_value_h = olt_mac_addr_high;
		    WRITE_REG_WORD(e_desec_srcaddr_cfg, reg_desec_srcaddr_cfg.Raw);
			/* polling done bit */
			while(cnt){
				if(e_desec_srcaddr_cfg_GET_desec_srcaddr_rwcmd_done(e_desec_srcaddr_cfg)){
					break;		
				}
			}
		}	

		/* config key */
	    for(i=0;i < DPOE_MAX_KEY_NUM; i++){
	        /* setting key value */
	        keyValue = ((*key)<<24) |((*(key+1))<<16) | ((*(key+2))<<8) | (*(key+3));
	        key += 4;
	        e_key_value_SET_key_value(e_key_value, keyValue);
	        if(an7581_epon_dpoe_decrypt_key_cfg(EPON_WRITE_KEY, llidIndex, keyIndex, (DPOE_MAX_KEY_NUM -1 -i))){
	        	DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Set key Fail.", __FUNCTION__, __LINE__);
	            return -1;
	        }	
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], SET_key_value indx:%d keyValue:%x \n", __FUNCTION__, __LINE__,i,keyValue);
	    }
	}

    return 0;
}

int an7581_epon_dpoe_encrypt_key_cfg(__u8 opt,__u8 llidIndex, __u8 keyIndex,__u8 keyNum){
    /* This function for dpoe */
    REG_e_enckey_cfg cfg = {0};
    int cnt = 100;
    
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(keyNum>3)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

    /* config key */
    cfg.Bits.enckey_rwcmd = opt;
    cfg.Bits.enckey_keyidx = keyIndex;
    cfg.Bits.enckey_llididx = llidIndex;
    cfg.Bits.enckey_dwidx = keyNum;
    WRITE_REG_WORD(e_enckey_cfg, cfg.Raw);

    /* polling done bit */
    while(cnt){
        if(e_enckey_cfg_GET_enckey_rwcmd_done(e_enckey_cfg)){
            return 0;
        }
    }
    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Done bit error.", __FUNCTION__, __LINE__);
    
    return -1;
}

int an7581_epon_set_dpoe_encrypt_llid_key(const void *in, void *out)
{
    uint32_t val = 0;
    uint8_t i = 0;
    uint32_t keyValue = 0;

	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
    key = io_info->info;
	
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], llidindex:%d keyindex:%d\n", __FUNCTION__, __LINE__,llidIndex,keyIndex);

    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)||gpPonSysData->sysPonMode!=XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }
    /* enable encrypt keyindex */   
    val = READ_REG_WORD(llids_cfg_map[llidIndex/4]);
    val |= ((keyIndex<<5)<<((llidIndex-(llidIndex/4)*4)*8));//bit 5 for encrypt keyindex
    WRITE_REG_WORD(llids_cfg_map[llidIndex/4], val);

	/* set encrypt mode ctr */	
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], SET_decrpt_mode: CTR mode \n", __FUNCTION__, __LINE__);
	e_crpt_cfg_SET_encrpt_mode(e_crpt_cfg,2);// ctr mode

	/* config key */
    for(i=0;i < DPOE_MAX_KEY_NUM; i++){
        /* setting key value */
        keyValue = ((*key)<<24) |((*(key+1))<<16) | ((*(key+2))<<8) | (*(key+3));
        key += 4;    
		e_enckey_val_SET_enckey_value(e_enckey_val, keyValue);
		if(an7581_epon_dpoe_encrypt_key_cfg(EPON_WRITE_KEY, llidIndex, keyIndex, (DPOE_MAX_KEY_NUM -1 -i))){
        	DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Set key Fail.", __FUNCTION__, __LINE__);
            return -1;
        }
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "[%s %d], SET_key_value indx:%d keyValue:%x \n", __FUNCTION__, __LINE__,i,keyValue);
    }

	/* enable llidx encrypt */	 
	val = READ_REG_WORD(llids_cfg_map[llidIndex/4]);
	val |= (0x10<<((llidIndex-(llidIndex/4)*4)*8));//0x10 enable aes encryption
	WRITE_REG_WORD(llids_cfg_map[llidIndex/4], val);

    return 0;
}

int an7581_pon_mac_scu_reset(const void *in, void *out)
{
	uint32_t Raw = 0;

	/* reset mac */
	Raw = GET_SCU_RSTCTRL1();
	Raw |= SCU_PON_MAC_RESET  ;
	SET_SCU_RSTCTRL1(Raw);
	udelay(1);
	/* release mac */
	Raw = GET_SCU_RSTCTRL1();
	Raw &= ~SCU_PON_MAC_RESET ;
	SET_SCU_RSTCTRL1(Raw);
	return 0;
}

int an7581_epon_mac_logic_reset(const void *in, void *out)
{
	int Raw = 0;
	int hold_on_off = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	hold_on_off = *(int *)in;

	Raw = GET_SSR3();
	if(hold_on_off == EPON_LOGIC_RESET_HOLD_ON){
		Raw |= EPON_LOGIC_RESET_BIT;
	}else if(hold_on_off == EPON_LOGIC_RESET_HOLD_OFF){
		Raw &= (~EPON_LOGIC_RESET_BIT);
	}else{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: Error INPUT Parameter, hold_on_off=%d ", __FUNCTION__, hold_on_off);
		return EPON_INPUT_PARA_ERROR;
	}		
	SET_SSR3(Raw);
    udelay(1);
	return 0;
}

int an7581_pon_get_onu_mode_type(const void *in, void *out)
{
	char mode_type=0;
#if 1
	mode_type = get_onutype(); //EN7523 ARM Function
#else
	mode_type = READ_FLASH_BYTE(flash_base + XPON_MODE_FLASH_ADDR);
#endif
	gpPonSysData->sysOnuType = mode_type & ONUTYPE_MASK;
	gpPonSysData->sysComboPon= (mode_type & COMBOPON_MASK) >> 2;
	gpPonSysData->sysBBF247 = (mode_type & BBF247_MASK) >> 3;
	gpPonSysData->sysPonMode = (mode_type & ONUMODE_MASK ) >> 4;
	if(gpPonSysData->sysComboPon == 1){
		if(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_XGSPON){
			printk("*** Flash onu mode is %d, change to XGSPON ! ***\n", gpPonSysData->sysPonMode);
			gpPonSysData->sysPonMode = XMCS_IF_WAN_DETECT_MODE_XGSPON;
		}		
	}
	dump_pon_type_mode_info(gpPonSysData->sysOnuType, gpPonSysData->sysPonMode);
	return 0;
}

int an7581_epon_set_dygasp_hw_en(const void *in, void *out)
{
	uint8_t dygasp_hw_en = 1;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	dygasp_hw_en = *(uint8_t *)in;

	e_dyinggsp_cfg_SET_hw_dygasp_en(e_dyinggsp_cfg, dygasp_hw_en);

	return EPON_SUCCESS;
}

int an7581_epon_get_dygasp_hw_en(const void *in, void *out)
{
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	*(uint8_t *)out = e_dyinggsp_cfg_GET_hw_dygasp_en(e_dyinggsp_cfg);

	return EPON_SUCCESS;
}

int an7581_epon_set_dygasp_num(const void *in, void *out)
{
	uint32_t dygasp_num = 1;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	dygasp_num = *(uint32_t *)in;

	e_dyinggsp_cfg_SET_dygasp_num_of_times(e_dyinggsp_cfg, dygasp_num);

	return EPON_SUCCESS;
}

int an7581_epon_get_dygasp_num(const void *in, void *out)
{
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	*(uint32_t *)out = e_dyinggsp_cfg_GET_dygasp_num_of_times(e_dyinggsp_cfg);
	

	return EPON_SUCCESS;
}

/*************************************************************************************************************/
/***********************************************debug function defined****************************************/
/*************************************************************************************************************/

/*===================================================================*/
/*                 an7581 epon mac register table                    */
/*===================================================================*/

reg_check_t an7581_epon_reg[]=
{
	/*Register name ,		Type,				Address,			Default Value, Mask*/
	{"e_glb_cfg",						RW,				0x6000, 0x00800002, 0x03ff3f9a},
	{"e_glb_cfg2",						RW,				0x6004, 0x80a08000, 0x80f3f061},
	{"e_glb_sts1",						RO|NO_DEF, 		0x6008, 0x00000000, 0xffffffff},
	{"e_glb_sts2",						RO|NO_DEF,		0x600c, 0x00000000, 0xffffffff},
	{"e_int_status", 					W1C|NO_DEF,		0x6010, 0x00000000, 0x3fffffff},
	{"e_int_en",						RW|NO_DEF,		0x6014, 0x00000000, 0x3fffffff},
	{"e_int_sts2",						W1C|NO_DEF,		0x6018, 0x00000000, 0x00001fff},
	{"e_int_en2",						RW|NO_DEF,		0x601c, 0x00000000, 0x00001fff},
	{"e_int_sts3",						W1C|NO_DEF, 	0x6020, 0x00000000, 0x00ffffff},
	{"e_int_en3",						RW|NO_DEF,		0x6024, 0x00000000, 0x00ffffff},
	{"e_exc_sts",						W1C|NO_DEF, 	0x6028, 0x00000000, 0x0fff377f},
	{"e_rpt_timeout", 					W1C|NO_DEF, 	0x602c, 0x00000000, 0xffffffff},
	{"e_mpcp_timeout",					W1C|NO_DEF, 	0x6030, 0x00000000, 0xffffffff},
	{"e_ds_keychg",						W1C|NO_DEF, 	0x6034, 0x00000000, 0xffffffff},
	{"e_ds_keychg2", 					W1C|NO_DEF, 	0x6038, 0x00000000, 0x000fffff},
	{"e_ds_keymis", 					W1C|NO_DEF, 	0x603c, 0x00000000, 0xffffffff},
	{"e_ds_keymis2", 					W1C|NO_DEF, 	0x6040, 0x00000000, 0x000fffff},
	{"e_us_keychg", 					W1C|NO_DEF, 	0x6044, 0x00000000, 0xffffffff},
	{"e_llid0_3_cfg",					RW|NO_DEF,		0x6050, 0x00000000, 0xffffffff},
	{"e_llid4_7_cfg",					RW|NO_DEF,		0x6054, 0x00000000, 0xffffffff},
	{"e_llid8_11_cfg",					RW|NO_DEF,		0x6058, 0x00000000, 0xffffffff},
	{"e_llid12_15_cfg",					RW|NO_DEF,		0x605c, 0x00000000, 0xffffffff},
	{"e_llid16_19_cfg", 				RW|NO_DEF,		0x6060, 0x00000000, 0xffffffff},
	{"e_llid20_23_cfg", 				RW|NO_DEF,		0x6064, 0x00000000, 0xffffffff},
	{"e_llid24_27_cfg", 				RW|NO_DEF,		0x6068, 0x00000000, 0xffffffff},
	{"e_llid28_31_cfg", 				RW|NO_DEF,		0x606c, 0x00000000, 0xffffffff},
	{"e_llid_dscvry_ctrl",				RW|RO|NO_DEF,	0x607c, 0x00000000, 0xc000111f},
	{"e_llid0_dscvry_sts",				RW|RO|NO_DEF,	0x6080, 0x00000000, 0xc0000000},
	{"e_llid1_dscvry_sts",				RW|RO|NO_DEF,	0x6084, 0x00000000, 0xc0000000},
	{"e_llid2_dscvry_sts",				RW|RO|NO_DEF,	0x6088, 0x00000000, 0xc0000000},
	{"e_llid3_dscvry_sts",				RW|RO|NO_DEF,	0x608c, 0x00000000, 0xc0000000},
	{"e_llid4_dscvry_sts",				RW|RO|NO_DEF,	0x6090, 0x00000000, 0xc0000000},
	{"e_llid5_dscvry_sts",				RW|RO|NO_DEF,	0x6094, 0x00000000, 0xc0000000},
	{"e_llid6_dscvry_sts",				RW|RO|NO_DEF,	0x6098, 0x00000000, 0xc0000000},
	{"e_llid7_dscvry_sts",				RW|RO|NO_DEF,	0x609c, 0x00000000, 0xc0000000},
	{"e_llid8_dscvry_sts",				RW|RO|NO_DEF,	0x60a0, 0x00000000, 0xc0000000},
	{"e_llid9_dscvry_sts",				RW|RO|NO_DEF,	0x60a4, 0x00000000, 0xc0000000},
	{"e_llid10_dscvry_sts",				RW|RO|NO_DEF,	0x60a8, 0x00000000, 0xc0000000},
	{"e_llid11_dscvry_sts", 			RW|RO|NO_DEF,	0x60ac, 0x00000000, 0xc0000000},
	{"e_llid12_dscvry_sts", 			RW|RO|NO_DEF,	0x60b0, 0x00000000, 0xc0000000},
	{"e_llid13_dscvry_sts", 			RW|RO|NO_DEF,	0x60b4, 0x00000000, 0xc0000000},
	{"e_llid14_dscvry_sts", 			RW|RO|NO_DEF,	0x60b8, 0x00000000, 0xc0000000},
	{"e_llid15_dscvry_sts", 			RW|RO|NO_DEF,	0x60bc, 0x00000000, 0xc0000000},
	{"e_llid16_dscvry_sts",				RW|RO|NO_DEF,	0x60c0, 0x00000000, 0xc0000000},
	{"e_llid17_dscvry_sts",				RW|RO|NO_DEF,	0x60c4, 0x00000000, 0xc0000000},
	{"e_llid18_dscvry_sts", 			RW|RO|NO_DEF,	0x60c8, 0x00000000, 0xc0000000},
	{"e_llid19_dscvry_sts", 			RW|RO|NO_DEF,	0x60cc, 0x00000000, 0xc0000000},
	{"e_llid20_dscvry_sts", 			RW|RO|NO_DEF,	0x60d0, 0x00000000, 0xc0000000},
	{"e_llid21_dscvry_sts", 			RW|RO|NO_DEF,	0x60d4, 0x00000000, 0xc0000000},
	{"e_llid22_dscvry_sts", 			RW|RO|NO_DEF,	0x60d8, 0x00000000, 0xc0000000},
	{"e_llid23_dscvry_sts", 			RW|RO|NO_DEF,	0x60dc, 0x00000000, 0xc0000000},
	{"e_llid24_dscvry_sts", 			RW|RO|NO_DEF,	0x60e0, 0x00000000, 0xc0000000},
	{"e_llid25_dscvry_sts", 			RW|RO|NO_DEF,	0x60e4, 0x00000000, 0xc0000000},
	{"e_llid26_dscvry_sts", 			RW|RO|NO_DEF,	0x60e8, 0x00000000, 0xc0000000},
	{"e_llid27_dscvry_sts", 			RW|RO|NO_DEF,	0x60ec, 0x00000000, 0xc0000000},
	{"e_llid28_dscvry_sts", 			RW|RO|NO_DEF,	0x60f0, 0x00000000, 0xc0000000},
	{"e_llid29_dscvry_sts", 			RW|RO|NO_DEF,	0x60f4, 0x00000000, 0xc0000000},
	{"e_llid30_dscvry_sts", 			RW|RO|NO_DEF,	0x60f8, 0x00000000, 0xc0000000},
	{"e_llid31_dscvry_sts", 			RW|RO|NO_DEF,	0x60fc, 0x00000000, 0xc0000000},
	{"e_pending_gnt_num", 				RO|RW, 			0x6100, 0x00000040, 0x000000ff},
	{"e_mac_addr_cfg",					RW|NO_DEF,		0x6104, 0x00000000, 0x8000003f},
	{"e_mac_addr_value",				RO|NO_DEF,		0x6108, 0x00000000, 0xffffffff},
	{"e_security_key_cfg",				RW|NO_DEF,		0x610c, 0x00000000, 0x80003f13},
	{"e_key_value",						RW|NO_DEF,		0x6110, 0x00000000, 0xffffffff},
	{"e_enckey_cfg", 					RW|NO_DEF,		0x6114, 0x00000000, 0x80003f13},	
	{"e_enckey_value", 					RW|NO_DEF,		0x6118, 0x00000000, 0xffffffff},
	{"e_rpt_cfg",						RW|NO_DEF,		0x6124, 0x00000000, 0xffffffff},
	{"e_rpt_cfg2",						RW|NO_DEF,		0x6128, 0x00000000, 0xffffffff},
	{"e_rpt_qthld_cfg",					RW|NO_DEF,		0x612c, 0x00000000, 0x9f0000c7},
	{"e_rpt_glb_cfg",					RW,				0x6130, 0x00000000, 0xffffffff},
	{"e_rpt_bmap",						RW,				0x6134, 0x0000ffff, 0xffffffff},
	{"e_frpt_thr",						RW, 			0x6138, 0x001000c0, 0xffffffff},
	{"e_u1g_rpt_qsizeadj",				RW, 			0x613c, 0x00020002, 0xffffffff},
	{"e_u10g_rpt_qsizeadj",				RW,				0x6140, 0x00000002, 0x0000ffff},
	{"e_cfg_dmy8",      				RW,				0x6148, 0x00547254, 0xffffffff},	
	{"e_cfg_dmy9",      				RW,				0x614c, 0x00000087, 0xffffffff},	
	{"e_cfg_dmy10",      				RW,				0x6150, 0x00000000, 0xffffffff},
    {"e_dfrpt_data5",					RW,				0x6180, 0x0101f000, 0xffffffff},
	{"e_dfrpt_data6",					RW, 			0x6184, 0x00000000, 0xffffffff},
	{"e_dfrpt_data7",					RW,				0x6188, 0x00000000, 0xffffffff},
	{"e_dfrpt_data8",					RW,				0x618c, 0x00000000, 0xffffffff},
	{"e_dfrpt_data9",					RW,				0x6190, 0x00000000, 0xffffffff},
	{"e_dfrpt_data10",					RW,				0x6194, 0x00000000, 0xffffffff},
	{"e_dfrpt_data11",					RW,				0x6198, 0x00000000, 0xffffffff},
	{"e_dfrpt_data12",					RW,				0x619c, 0x00000000, 0xffffffff},
	{"e_dfrpt_data13",					RW,				0x61a0, 0x00000000, 0xffffffff},
	{"e_dfrpt_data14",					RW,				0x61a4, 0x00000000, 0xffffffff},
	{"e_laser_onoff_time",				RO,				0x61c0, 0x00002020, 0x0000ffff},
	{"e_sync_time", 					W1C|RO, 		0x61c4, 0x00000020, 0x00010000},
	{"e_overhead_time_thr", 			RW, 			0x61c8, 0xe0df2020, 0xe0ffffff},
	{"e_olt_overhead_time", 			RO|NO_DEF, 		0x61cc, 0x00000000, 0xffffffff},
	{"e_olt_dscvinfo", 					RO|NO_DEF, 		0x61d0, 0x00000000, 0x000000ff},
	{"e_grd_threshold", 				RW, 			0x61d4, 0x00000008, 0x000000ff},
	{"e_mpcp_timeout_intvl",			RW, 			0x61d8, 0x010001f4, 0xffffffff},
	{"e_rpt_timeout_intvl", 			RW, 			0x61dc, 0x00000010, 0x00ffffff},
	{"e_max_future_gnt_time",			RW, 			0x61e0, 0x03b9aca0, 0xffffffff},
	{"e_min_proc_time", 				RW, 			0x61e4, 0x00000400, 0x0000ffff},
	{"e_trx_adjust_time1", 				RW, 			0x61e8, 0x0000fff1, 0x0000ffff},
	{"e_trx_adjust_time2", 				RW,				0x61ec, 0x00000018, 0xffffffff},
	{"e_trx_adjust_time3",				RW, 			0x61f0, 0x00000008, 0xffffffff},
	{"e_trx_adjust_time4",				RW, 			0x61f4, 0xff900000, 0xffff1f00},
	{"e_trx_adjust_time5",				RW, 			0x61f8, 0x00000000, 0xffff0000},
	{"e_rxphydly_adjtime",				RW|NO_DEF,		0x61fc, 0x00000000, 0x8fffffff},
	{"e_txfetch_cfg", 					RW, 			0x6200, 0x002a03e8, 0x00ffffff},
	{"e_tx_cal_cnst", 					RW, 			0x6204, 0x2a120408, 0xffffff3f},
	{"e_tx_cal_cnst2",					RW, 			0x6208, 0xa2000d04, 0xbf00ffff},
	{"e_tx_cal_cnst3",					RW, 			0x620c, 0x37030e2a, 0xffffffff},
	{"e_txsch_cfg",						RW,				0x6210, 0xc0040020, 0xffffffff},
	{"e_rxfifo_thr", 					RW, 			0x6240, 0xffff0000, 0xffffffff},
	{"e_bcllid_cfg",					RW, 			0x6244, 0x7ffe7fff, 0xffffffff},
	{"e_txfrm_cfg1", 					RW, 			0x6248, 0xc2000001, 0xffffffff},
	{"e_txfrm_cfg2", 					RW, 			0x624c, 0x01808808, 0xffffffff},
	{"e_txfrm_cfg3", 					RW, 			0x6250, 0x00040006, 0xffffffff},
	{"e_tod_sync_x", 					RW|NO_DEF, 		0x6260, 0x00000000, 0xffffffff},
	{"e_tod_ltncy", 					RO|NO_DEF, 		0x6264, 0x00000000, 0x00ffffff},
	{"e_new_tod_p2p_offset_sec_l32",	RW|NO_DEF,		0x6268, 0x00000000, 0xffffffff},
	{"e_new_tod_p2p_offset_nsec", 		RW|NO_DEF,		0x626c, 0x00000000, 0xffffffff},
	{"e_tod_p2p_tod_sec_l32",			RW|NO_DEF, 		0x6270, 0x00000000, 0x00000000},
	{"e_tod_p2p_tod_nsec", 				RW|NO_DEF, 		0x6274, 0x00000000, 0x00000000},
	{"e_tod_period", 					RW|NO_DEF, 		0x6278, 0x00000000, 0x000000ff},
	{"e_tod_1pps_ctrl", 				RW, 			0x627c, 0x01312d00, 0xffffffff},
	{"e_pwr_sv_cfg",					RW|RO|NO_DEF, 	0x6280, 0x00000000, 0x3700ffff},
	{"e_slp_durt_max", 					RW|NO_DEF, 		0x6284, 0x00000000, 0xffffffff},
	{"e_slp_duration",					RW|NO_DEF, 		0x6288, 0x00000000, 0xffffffff},
	{"e_act_duration", 					RW|NO_DEF, 		0x628c, 0x00000000, 0xffffffff},
	{"e_pwron_dly", 					RW|NO_DEF, 		0x6290, 0x00000000, 0xffffffff},
	{"e_rx_slpalw_info", 				RO|NO_DEF,		0x6294, 0x00000000, 0xffffffff},
	{"e_oui_a_cfg", 					RW,				0x6298, 0x8058d08f, 0xffffffff},
	{"e_oui_b_cfg", 					RW, 			0x629c, 0x80908260, 0xffffffff},
	{"e_oui_c_cfg", 					RW, 			0x62a0, 0x80c4e032, 0xffffffff},
	{"e_dyinggsp_cfg", 					RW, 			0x62ac, 0x00000100, 0x8000ff00},
	{"e_dyinggsp_w1", 					RW, 			0x62b0, 0x88090300, 0xffffffff},
    {"e_dyinggsp_w2", 					RW, 			0x62b4, 0x52000110, 0xffffffff},
    {"e_dyinggsp_w3", 					RW, 			0x62b8, 0x01000000, 0xffffffff},
    {"e_dyinggsp_w4", 					RW, 			0x62bc, 0x0f05ee00, 0xffffffff},
    {"e_dyinggsp_w5", 					RW, 			0x62c0, 0x13250022, 0xffffffff},
    {"e_dyinggsp_w6", 					RW, 			0x62c4, 0x01000210, 0xffffffff},
    {"e_dyinggsp_w7", 					RW, 			0x62c8, 0x01000000, 0xffffffff},
    {"e_dyinggsp_w8", 					RW, 			0x62cc, 0x0f05ee00, 0xffffffff},
    {"e_dyinggsp_w9", 					RW, 			0x62d0, 0x13250000, 0xffffffff},
    {"e_dyinggsp_w10", 					RW|NO_DEF, 		0x62d4, 0x00000000, 0xffffffff},
    {"e_dyinggsp_w11", 					RW|NO_DEF, 		0x62d8, 0x00000000, 0xffffffff},
    {"e_dyinggsp_w12", 					RW|NO_DEF, 		0x62dc, 0x00000000, 0xffffffff},
	{"e_oam_kpalv_ctrl", 				RW, 			0x62e0, 0x000fff00, 0x03ffff07},
    {"e_oam_kpalvw1", 					RW|NO_DEF, 		0x62e4, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw2", 					RW|NO_DEF, 		0x62e8, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw3", 					RW|NO_DEF, 		0x62ec, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw4", 					RW|NO_DEF, 		0x62f0, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw5", 					RW|NO_DEF, 		0x62f4, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw6", 					RW|NO_DEF, 		0x62f8, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw7", 					RW|NO_DEF, 		0x62fc, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw8", 					RW|NO_DEF, 		0x6300, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw9", 					RW|NO_DEF, 		0x6304, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw10", 					RW|NO_DEF, 		0x6308, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw11", 					RW|NO_DEF, 		0x630c, 0x00000000, 0xffffffff},
    {"e_oam_kpalvw12", 					RW|NO_DEF, 		0x6310, 0x00000000, 0xffffffff},
	{"e_kpalv_hwensts", 				RO|NO_DEF, 		0x6314, 0x00000000, 0x00000000},
    {"e_mcgrp_en", 						RW|NO_DEF, 		0x6320, 0x00000000, 0x0000ffff},
	{"e_mcgrp_llidval0",				RW|NO_DEF,		0x6324, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval1",				RW|NO_DEF,		0x6328, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval2",				RW|NO_DEF,		0x632c, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval3",				RW|NO_DEF,		0x6330, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval4",				RW|NO_DEF,		0x6334, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval5",				RW|NO_DEF,		0x6338, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval6",				RW|NO_DEF,		0x633c, 0x00000000, 0xffffffff},
	{"e_mcgrp_llidval7",				RW|NO_DEF,		0x6340, 0x00000000, 0xffffffff},
	{"e_crpt_cfg", 						RW|NO_DEF, 		0x6350, 0x00000000, 0xffffffff},
	{"e_cfbsec_cfg",					RW|NO_DEF,		0x6354, 0x00000000, 0xffffffff},
	{"e_ctrsec_cfg",					RW|NO_DEF,		0x6358, 0x00000000, 0xffffffff},
/*	{"e_desec_srcaddr_cfg",				RW|NO_DEF,		0x6360, 0x00000000, 0x803fffff},
	{"e_desec_srcaddr_val", 			RO|NO_DEF,		0x6364, 0x00000000, 0xffffffff},*/ /*indirect r/w*/
	{"e_olt_macaddr_h", 				RO|NO_DEF,		0x6390, 0x00000000, 0x0000ffff},
	{"e_olt_macaddr_l", 				RO|NO_DEF,		0x6394, 0x00000000, 0xffffffff},
	{"total_tx_cyc_l",					RO|NO_DEF,		0x6474, 0x00000000, 0x00000000},
    {"e_snf_cfg", 						RW, 			0x6400, 0x03020001, 0xffdfffff},
    {"e_snf_sp_tag", 					RW, 			0x6404, 0x00070000, 0xffffffff},
    {"e_snf_dah", 						RW, 			0x6408, 0x00554433, 0xffffffff},
    {"e_snf_dal", 						RW, 			0x640c, 0x22110000, 0xffffffff},
    {"e_snf_sal", 						RW, 			0x6410, 0x00000001, 0xffffffff},
    {"e_snf_etype", 					RW, 			0x6414, 0x000088b8, 0x0000ffff},
    {"e_rdmdly_cfg", 					RW|NO_DEF, 		0x6430, 0x00000000, 0xffffffff},
    {"e_rdmdly_stat", 					RO|NO_DEF, 		0x6434, 0x00000000, 0x00000000},
	{"e_dbg_prb_sel", 					RW|NO_DEF, 		0x6440, 0x00000000, 0xffffffff},
	{"e_dbg_prb_h32", 					RO|NO_DEF, 		0x6444, 0x00000000, 0x00000000},
	{"e_dbg_prb_l32", 					RO|NO_DEF, 		0x6448, 0x00000000, 0x00000000},
	{"e_utili_cfg", 					RW|NO_DEF, 		0x644c, 0x00000000, 0xffffffff},
	{"e_utili_check",					RW|RO|NO_DEF,	0x6450, 0x00000000, 0x00000001},/*AO*/
	{"e_total_gnt_sizeh",				RO|NO_DEF,		0x6454, 0x00000000, 0x00000000},
	{"e_total_gnt_sizel",				RO|NO_DEF,		0x6458, 0x00000000, 0x00000000},
	{"e_total_pkt_cnt",					RO|NO_DEF,		0x645c, 0x00000000, 0x00000000},
	{"e_total_pkt_size_l", 				RO|NO_DEF,		0x6460, 0x00000000, 0x00000000},
	{"e_total_pkt_size_h",				RO|NO_DEF,		0x6464, 0x00000000, 0x00000000},
	{"e_total_u10g_nouse_gntlen",		RO|NO_DEF,		0x6468, 0x00000000, 0x00000000},
	{"e_total_gnt_cnt",					RO|NO_DEF,		0x646c, 0x00000000, 0x00000000},
	{"total_tx_cyc_h",					RO|NO_DEF,		0x6470, 0x00000000, 0x00000000},	
	{"e_mbist_delsel_cfg0",				RW,				0x64a0, 0x22222222, 0xffffffff},
	{"e_mbist_delsel_cfg1", 			RW,				0x64a4, 0x22222222, 0xffffffff},
	{"e_mbist_delsel_cfg2", 			RW,				0x64a8, 0x22222222, 0xffffffff},
	{"e_mbist_delsel_cfg3", 			RW,				0x64ac, 0x22222222, 0xffffffff},
	{"e_mbist_done_sts0",   			RO|NO_DEF,		0x64b0, 0x00000000, 0x00000000},
	{"e_mbist_fail_sts0",				RO|NO_DEF,		0x64b4, 0x00000000, 0x00000000},
	{"e_mbist_fail_sts1",				RO|NO_DEF,		0x64b8, 0x00000000, 0x00000000},
	{"rx_sld_sts",						W1C|NO_DEF,		0x64d0, 0x00000000, 0x000000ff},
    {"e_glue_cfg", 						RW|RO|NO_DEF, 	0x64e0, 0x00000000, 0x000087ff},
    {"e_gntreq_tmout", 					RW, 			0x64e4, 0x89502f90, 0xffffffff},
	{"e_fpga_gendef",					RO|NO_DEF, 		0x64f0, 0x00000000, 0x00000000},
	{"e_fpga_genver",					RO|NO_DEF,		0x64f4, 0x00000000, 0x00000000},
    {"e_cnt_clr", 						RO|NO_DEF, 		0x6500, 0x00000000, 0x00000000},/*AO*/
	{"e_rxmbi_eth_cnt", 				RO|NO_DEF, 		0x6510, 0x00000000, 0x00000000},
	{"e_rxmpi_eth_cnt", 				RO|NO_DEF, 		0x6514, 0x00000000, 0x00000000},
	{"e_txmbi_eth_cnt", 				RO|NO_DEF, 		0x6518, 0x00000000, 0x00000000},
	{"e_txmpi_eth_cnt", 				RO|NO_DEF, 		0x651c, 0x00000000, 0x00000000},
	{"e_oam_sta", 						RO|NO_DEF, 		0x6520, 0x00000000, 0x00000000},
	{"e_mpcp_sta", 						RO|NO_DEF, 		0x6524, 0x00000000, 0x00000000},
	{"e_mpcp_rgst_stat", 				RO|NO_DEF, 		0x6528, 0x00000000, 0x00000000},
    {"e_rxmbi_bytecnt_h", 				RO|NO_DEF, 		0x6580, 0x00000000, 0x00000000},
    {"e_rxmbi_bytecnt_l", 				RO|NO_DEF, 		0x6584, 0x00000000, 0x00000000},
    {"e_rxmbi_snf_cnt", 				RO|NO_DEF, 		0x6588, 0x00000000, 0x00000000},
    {"e_rxmpi_uc_cnt", 					RO|NO_DEF, 		0x65a0, 0x00000000, 0x00000000},
    {"e_rxmpi_bc_cnt", 					RO|NO_DEF, 		0x65a4, 0x00000000, 0x00000000},
    {"e_rxmpi_mc_cnt", 					RO|NO_DEF, 		0x65a8, 0x00000000, 0x00000000},    
    {"e_rxmpi_oam_cnt", 				RO|NO_DEF, 		0x65ac, 0x00000000, 0x00000000},
    {"e_rxmpi_mpcp_cnt", 				RO|NO_DEF, 		0x65b0, 0x00000000, 0x00000000},
    {"e_rxmpi_gate_cnt", 				RO|NO_DEF, 		0x65b4, 0x00000000, 0x00000000},
    {"e_rxmpi_nrlgate_cnt", 			RO|NO_DEF, 		0x65b8, 0x00000000, 0x00000000},
    {"e_txmbi_uceth_cnt", 				RO|NO_DEF, 		0x65d0, 0x00000000, 0x00000000},
    {"e_txmbi_mceth_cnt", 				RO|NO_DEF, 		0x65d4, 0x00000000, 0x00000000},
    {"e_tx_dygasp_cnt", 				RO|NO_DEF, 		0x65d8, 0x00000000, 0x00000000},
    {"e_tx_rpt_cnt", 					RO|NO_DEF, 		0x65dc, 0x00000000, 0x00000000},
    {"e_kpaom_stat", 					RO|NO_DEF, 		0x65e0, 0x00000000, 0x00000000},
    {"e_txmpi_mpcp_cnt", 				RO|NO_DEF, 		0x65e4, 0x00000000, 0x00000000},
    {"e_txmpi_uceth_cnt", 				RO|NO_DEF, 		0x65e8, 0x00000000, 0x00000000},
    {"e_txmpi_mceth_cnt", 				RO|NO_DEF, 		0x65ec, 0x00000000, 0x00000000},
    {"e_rxmpi_dscvexc_cnt", 			RO|NO_DEF, 		0x6600, 0x00000000, 0x00000000},
    {"e_rxmpi_churn_cnt", 				RO|NO_DEF, 		0x6604, 0x00000000, 0x00000000},
    {"e_rxmpi_churn_info", 				RO|NO_DEF, 		0x6608, 0x00000000, 0x00000000},
    {"e_demacs_ok_cnt", 				RO|NO_DEF, 		0x6610, 0x00000000, 0x00000000},
    {"e_rxmpi_p_cnt", 					RO|NO_DEF, 		0x6614, 0x00000000, 0x00000000},
    {"e_rxmpi_crcerr_cnt", 				RO|NO_DEF, 		0x6618, 0x00000000, 0x00000000},
    {"e_demacs_drop_cnt", 				RO|NO_DEF, 		0x661c, 0x00000000, 0x00000000},
	{"e_demacs_drop_cnt2",				RO|NO_DEF,		0x6620, 0x00000000, 0x00000000},
    {"e_rxmpi_drop_cnt", 				RO|NO_DEF, 		0x6624, 0x00000000, 0x00000000},
    {"e_rxmbi_drop_cnt", 				RO|NO_DEF, 		0x6640, 0x00000000, 0x00000000},
    {"e_rxmbi_drop_cnt2", 				RO|NO_DEF, 		0x6644, 0x00000000, 0x00000000},
    {"e_txmbi_err_cnt", 				RO|NO_DEF, 		0x6680, 0x00000000, 0x00000000},
	{"e_gnt_type_stat", 				RO|NO_DEF, 		0x66c0, 0x00000000, 0x00000000},
	{"e_gnt_pending_stat", 				RO|NO_DEF, 		0x66c4, 0x00000000, 0x00000000},
	{"e_gnt_length_stat", 				RO|NO_DEF, 		0x66c8, 0x00000000, 0x00000000},
/*	{"e_local_time", 					RO|NO_DEF, 		0x66cc, 0x00000000, 0x00000000},*/
	{"e_time_drft_stat", 				RO|NO_DEF, 		0x66d0, 0x00000000, 0x00000000},
    {"e_tx_timedrift_stat", 			RO|NO_DEF, 		0x66d4, 0x00000000, 0x00000000},
    {"e_rxfifo_depth_stat", 			RO|NO_DEF, 		0x66d8, 0x00000000, 0x00000000},
    {"e_txmbi_bufwrap_wrcnt", 			RO|NO_DEF, 		0x66f4, 0x00000000, 0x00000000},
    {"e_txmbi_bufwrap_rdcnt", 			RO|NO_DEF, 		0x66f8, 0x00000000, 0x00000000},
    {"e_txmbi_bufwrap_rdcnt2",			RO|NO_DEF, 		0x66fc, 0x00000000, 0x00000000},

	{NULL, 0x0, 0x0, 0x0, 0x0}
};

static int eponMacAddrIndTest(uint32_t pattern)
{
	int retval = 0;
	uint8_t llidIndex = 0;
    uint32_t mac_addr_low = pattern;
    uint32_t mac_addr_high  = pattern;

	EPON_CHECK_MAC_ADDR_CFG_DONE;

	printk("%s [%d]\n",__FUNCTION__,__LINE__);
	for(llidIndex = 0; llidIndex < 32; llidIndex++){
		retval = epon_set_llid_regs_mac_address(EPON_ADDR_REG_LOW, llidIndex, mac_addr_low);
	    retval |= epon_set_llid_regs_mac_address(EPON_ADDR_REG_HIGH, llidIndex, mac_addr_high);
	    if(retval != EPON_SUCCESS)
	    {
	    	printk("ERROR: %s [%d] epon_set_llid_regs_mac_address fail,llidIndex = %d\n",__FUNCTION__,__LINE__,llidIndex);
	        return EPON_NORMAL_ERROR;
	    }
	    retval = epon_get_llid_regs_mac_address(EPON_ADDR_REG_LOW, llidIndex, &mac_addr_low);
	    retval |= epon_get_llid_regs_mac_address(EPON_ADDR_REG_HIGH, llidIndex, &mac_addr_high);
	    if(retval != EPON_SUCCESS)
	    {
	    	printk("ERROR: %s [%d] epon_get_llid_regs_mac_address fail,llidIndex = %d\n",__FUNCTION__,__LINE__,llidIndex);
	        return EPON_NORMAL_ERROR;
	    }
		if((mac_addr_low != pattern) || (mac_addr_high != pattern))
		{
	    	printk("ERROR: %s [%d] check llid regs mac address fail,llidIndex = %d\n",__FUNCTION__,__LINE__,llidIndex);
			return EPON_NORMAL_ERROR;
		}
	}
	return retval;
}

static int eponSecKeyIndTest(uint32_t pattern)
{
	int retval = 0;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t dwIndex = 0;
    uint32_t keyValue = pattern;
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	uint32_t timers = 0;

	printk("%s [%d]\n",__FUNCTION__,__LINE__);
	for(llidIndex = 0; llidIndex < 64; llidIndex ++){
		for(keyIndex = 0; keyIndex < 2; keyIndex ++){
			for(dwIndex = 0; dwIndex < 4; dwIndex ++){
				WRITE_REG_WORD(e_key_value , keyValue);
				memset(&eponSecurityKeyCfgReg, 0, sizeof(REG_e_security_key_cfg));
				
				eponSecurityKeyCfgReg.Bits.key_rwcmd = 1;
				eponSecurityKeyCfgReg.Bits.key_llid_index = llidIndex;
				eponSecurityKeyCfgReg.Bits.key_idx = keyIndex;
				eponSecurityKeyCfgReg.Bits.key_dw_indx = dwIndex;

				DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d]eponSetLlidKey eponSecurityKeyCfgReg = 0x%x\n",__FUNCTION__,__LINE__, eponSecurityKeyCfgReg.Raw);	
				WRITE_REG_WORD(e_security_key_cfg , eponSecurityKeyCfgReg.Raw);
				
				//check done bit is ready
				timers = 0;
				while(timers < 100){
					eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
					if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
						break;
					}
					timers++;
				}
				if(timers == 100){
					printk(" [%s][%d]eponSetLlidKey set  timers == 100\r\n",__FUNCTION__,__LINE__);
					return -1;
				}

				eponSecurityKeyCfgReg.Bits.key_rwcmd = 0;
				DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d]eponSetLlidKey eponSecurityKeyCfgReg = 0x%x\n",__FUNCTION__,__LINE__, eponSecurityKeyCfgReg.Raw);	
				WRITE_REG_WORD(e_security_key_cfg , eponSecurityKeyCfgReg.Raw);

				//check done bit is ready
				timers = 0;
				while(timers < 100){
					eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
					if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
						break;
					}
					timers++;
				}
				if(timers == 100){
					printk(" [%s][%d]eponSetLlidKey set  timers == 100\r\n",__FUNCTION__,__LINE__);
					return -1;
				}

			    keyValue = READ_REG_WORD(e_key_value);
				
				if(keyValue != pattern)
				{
					printk("ERROR: [%s][%d] check llid key regs fail,llidIndex = %d,keyIndex =%d,dwIndex =%d\n",
						__FUNCTION__,__LINE__,llidIndex,keyIndex,dwIndex);
					return -1;
				}
			}
		}
	}	
	return retval;
}


static int eponSecEncryptKeyIndTest(uint32_t pattern)
{
	int retval = 0;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t dwIndex = 0;
    uint32_t keyValue = pattern;
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	uint32_t timers = 0;
	
	printk("%s [%d]\n",__FUNCTION__,__LINE__);
	for(llidIndex = 0; llidIndex < 64; llidIndex ++){
		for(keyIndex = 0; keyIndex < 2; keyIndex ++){
			for(dwIndex = 0; dwIndex < 4; dwIndex ++){
				WRITE_REG_WORD(e_enckey_val , keyValue);
				memset(&eponSecurityKeyCfgReg, 0, sizeof(REG_e_security_key_cfg));
				
				eponSecurityKeyCfgReg.Bits.key_rwcmd = 1;
				eponSecurityKeyCfgReg.Bits.key_llid_index = llidIndex;
				eponSecurityKeyCfgReg.Bits.key_idx = keyIndex;
				eponSecurityKeyCfgReg.Bits.key_dw_indx = dwIndex;

				DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d]eponSetLlidKey eponSecurityKeyCfgReg = 0x%x\n",__FUNCTION__,__LINE__, eponSecurityKeyCfgReg.Raw);	
				WRITE_REG_WORD(e_enckey_cfg , eponSecurityKeyCfgReg.Raw);
				
				//check done bit is ready
				timers = 0;
				while(timers < 100){
					eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_enckey_cfg);
					if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
						break;
					}
					timers++;
				}
				if(timers == 100){
					printk(" [%s][%d]eponSetLlidKey set  timers == 100\r\n",__FUNCTION__,__LINE__);
					return -1;
				}

				eponSecurityKeyCfgReg.Bits.key_rwcmd = 0;
				DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d]eponSetLlidKey eponSecurityKeyCfgReg = 0x%x\n",__FUNCTION__,__LINE__, eponSecurityKeyCfgReg.Raw);	
				WRITE_REG_WORD(e_enckey_cfg , eponSecurityKeyCfgReg.Raw);

				//check done bit is ready
				timers = 0;
				while(timers < 100){
					eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_enckey_cfg);
					if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
						break;
					}
					timers++;
				}
				if(timers == 100){
					printk(" [%s][%d]eponSetLlidKey set  timers == 100\r\n",__FUNCTION__,__LINE__);
					return -1;
				}

			    keyValue = READ_REG_WORD(e_enckey_val);
				
				if(keyValue != pattern)
				{
					printk("ERROR: [%s][%d] check llid key regs fail,llidIndex = %d,keyIndex =%d,dwIndex =%d\n",
						__FUNCTION__,__LINE__,llidIndex,keyIndex,dwIndex);
					return -1;
				}
			}
		}
	}	
	return retval;
}

static int eponQueueThresIndTest(uint32_t pattern)
{
	int retval = 0;
	uint8_t llidIndex = 0;
	uint8_t thrIdx = 0;
	uint8_t queueIndex = 0;
	eponQueueThreshold_t queue_threshold = {0};

	printk("%s [%d]\n",__FUNCTION__,__LINE__);
	for(llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM;llidIndex++){
		for(queueIndex = 0; queueIndex< EPON_MAX_QUEUE_PER_CHANNEL;queueIndex++){
			for(thrIdx = 0; thrIdx< EPON_MAX_QUEUE_THRESHOLD;thrIdx++){
				memset(&queue_threshold, 0, sizeof(QDMA_EponQueueThreshold_T));
				queue_threshold.channel = llidIndex;
				queue_threshold.queue = queueIndex ;
				queue_threshold.thrIdx = thrIdx;
				queue_threshold.value = (__u16)pattern;
				retval = epon_set_queue_threshold(&queue_threshold);	
				if(retval < 0){
					printk("[%s][%d]epon_set_queue_threshold error,llidIndex = %d,queueIndex =%d,thrIdx =%d\n",
						__FUNCTION__,__LINE__,llidIndex,queueIndex,thrIdx);
					return -1;
				}
			
				retval = epon_get_queue_threshold(&queue_threshold);
				if(retval < 0){
					printk("[%s][%d]epon_get_queue_threshold error,llidIndex = %d,queueIndex =%d,thrIdx =%d\n",
						__FUNCTION__,__LINE__,llidIndex,queueIndex,thrIdx);
					return -1;
				}
				if(queue_threshold.value != (__u16)pattern)
				{
					printk("ERROR: [%s][%d] check queue threshold regs fail,llidIndex = %d,queueIndex =%d,thrIdx =%d\n",
						__FUNCTION__,__LINE__,llidIndex,queueIndex,thrIdx);
					return -1;
				}
			}
		}
	}
	return retval;
}
static int eponMacSecSrcAddrIndTest(uint32_t pattern)
{
	int retval = 0;
	uint8_t llidIndex = 0;
    uint32_t srcAddrValL = pattern;
    __u16 srcAddrValH = (__u16)pattern;
	REG_e_desec_srcaddr_cfg eponMacSecSrcAddrCfg;
	uint32_t timers = 0;

	printk("%s [%d]\n",__FUNCTION__,__LINE__);
	for(llidIndex = 0; llidIndex < 64; llidIndex ++){
		WRITE_REG_WORD(e_desec_srcaddr_val , srcAddrValL);
		memset(&eponMacSecSrcAddrCfg, 0, sizeof(REG_e_desec_srcaddr_cfg));

		eponMacSecSrcAddrCfg.Bits.desec_srcaddr_rwcmd = 1;
		eponMacSecSrcAddrCfg.Bits.desec_srcaddr_llid_idx = llidIndex;
		eponMacSecSrcAddrCfg.Bits.desec_srcaddr_value_h = srcAddrValH;
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d] eponMacSecSrcAddrCfg = 0x%x\n",__FUNCTION__,__LINE__, eponMacSecSrcAddrCfg.Raw);	
		WRITE_REG_WORD(e_desec_srcaddr_cfg , eponMacSecSrcAddrCfg.Raw);
		
		//check done bit is ready
		timers = 0;
		while(timers < 100){
			eponMacSecSrcAddrCfg.Raw = READ_REG_WORD(e_desec_srcaddr_cfg);
			if(eponMacSecSrcAddrCfg.Bits.desec_srcaddr_rwcmd_done == 1){
				break;
			}
			timers++;
		}
		if(timers == 100){
			printk("[%s][%d] eponMacSecSrcAddrCfg set  timers == 100\r\n",__FUNCTION__,__LINE__);
			return -1;
		}
		eponMacSecSrcAddrCfg.Bits.desec_srcaddr_rwcmd = 0;
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "[%s][%d] eponMacSecSrcAddrCfg = 0x%x\n",__FUNCTION__,__LINE__, eponMacSecSrcAddrCfg.Raw);	
		WRITE_REG_WORD(e_desec_srcaddr_cfg , eponMacSecSrcAddrCfg.Raw);

		//check done bit is ready
		timers = 0;
		while(timers < 100){
			eponMacSecSrcAddrCfg.Raw = READ_REG_WORD(e_desec_srcaddr_cfg);
			if(eponMacSecSrcAddrCfg.Bits.desec_srcaddr_rwcmd_done == 1){
				break;
			}
			timers++;
		}
		if(timers == 100){
			printk("[%s][%d] eponMacSecSrcAddrCfg get  timers == 100\r\n",__FUNCTION__,__LINE__);
			return -1;
		}
		srcAddrValL = READ_REG_WORD(e_desec_srcaddr_val);
		
		if((srcAddrValL != pattern) || (eponMacSecSrcAddrCfg.Bits.desec_srcaddr_value_h != (__u16)pattern))
		{
			printk("ERROR: [%s][%d] check mac sec source mac regs fail,llidIndex = %d\n",__FUNCTION__,__LINE__,llidIndex);
			return -1;
		}
	}
	return retval;
}
reg_ind_check_t an7581_indirect_reg[] =
{
    {"RW MAC Address Value", 			eponMacAddrIndTest},
	{"RW Security Key Value", 			eponSecKeyIndTest},
	{"RW Security EncryKey Value", 		eponSecEncryptKeyIndTest},
	{"RW Queue Threshold Value", 		eponQueueThresIndTest},
	{"RW MAC SEC Source Addr Value", 	eponMacSecSrcAddrIndTest},
	{NULL, NULL}
};

int an7581_epon_get_debug_statistic_count(const void *in, void *out)
{
    REG_e_mpcp_stat mpcp_stat;
    REG_e_mpcp_rgst_stat mpcp_rgst_stat;
    REG_e_oam_stat oam_stat;
    REG_e_rxmpi_crc8err_cnt rxmpi_crc_err_cnt;
    REG_e_rxmbi_drop_cnt rxmbi_drop_cnt;
    REG_e_rxmbi_drop_cnt2 rxmbi_drop_cnt2;
    REG_e_txmbi_mceth_cnt txmbi_mceth_cnt;
    REG_e_txmpi_mpcp_cnt txmpi_mpcp_cnt;
    REG_e_txmpi_mceth_cnt txmpi_mceth_cnt;
    REG_e_tx_rpt_cnt tx_rpt_cnt;
    REG_e_rxmpi_churn_cnt rxmpi_churn_cnt;
    REG_e_rxmpi_drop_cnt rxmpi_drop_cnt;
	epon_debug_statistic_t *tmpReg = (epon_debug_statistic_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	memset(tmpReg, 0, sizeof(epon_debug_statistic_t));
    /* Reg: mpcp stat*/
    mpcp_stat.Raw = READ_REG_WORD(e_mpcp_stat);
    tmpReg->mpcpErrCnt = mpcp_stat.Bits.mpcp_err_cnt;
    tmpReg->mpcpRgstCnt = mpcp_stat.Bits.mpcp_rgst_cnt;
    tmpReg->mpcpDscvGateCnt = mpcp_stat.Bits.mpcp_dscv_gate_cnt;

    /*Reg: mpcp_rgst_stat*/
    mpcp_rgst_stat.Raw = READ_REG_WORD(e_mpcp_rgst_stat);
    tmpReg->mpcpRgstReqCnt = mpcp_rgst_stat.Bits.mpcp_rgst_req_cnt;
    tmpReg->mpcpRgstAckCnt = mpcp_rgst_stat.Bits.mpcp_rgst_ack_cnt;

    /*Reg: rxmpi_eth*/
    tmpReg->rxMpiEth = READ_REG_WORD(e_rxmpi_eth_cnt);

    /*Reg: rxmbi_eth*/
    tmpReg->rxMbiEth = READ_REG_WORD(e_rxmbi_eth_cnt);

    /*Reg: txmpi_eth*/
    tmpReg->txMpiEth = READ_REG_WORD(e_txmpi_eth_cnt);

    /*Reg: txmbi_eth*/
    tmpReg->txMbiEth = READ_REG_WORD(e_txmbi_eth_cnt);
    
    /*Reg: oam_stat*/
    oam_stat.Raw = READ_REG_WORD(e_oam_stat);
    tmpReg->rxOamCnt = oam_stat.Bits.rx_oam_cnt;
    tmpReg->txOamCnt = oam_stat.Bits.tx_oam_cnt;

    /*Reg: rxmpi_crc_err_cnt*/
    rxmpi_crc_err_cnt.Raw = READ_REG_WORD(e_rxmpi_crc8err_cnt);
    tmpReg->rxMpiCrc32ErrCnt = rxmpi_crc_err_cnt.Bits.rxmpi_crc32err_cnt;
    tmpReg->rxMpiCrc8ErrCnt = rxmpi_crc_err_cnt.Bits.rxmpi_crc8err_cnt;

    /*Reg: rxmpi_drop_cnt*/
    rxmpi_drop_cnt.Raw = READ_REG_WORD(e_rxmpi_drop_cnt);
    tmpReg->rxMpiEofDropCnt = rxmpi_drop_cnt.Bits.rxmpi_eofdrop_cnt;
    tmpReg->rxMpiFifoOvRunCnt = rxmpi_drop_cnt.Bits.rxmpi_fifoovrun_cnt;

    /*Reg: rxmbi_drop_cnt*/
    rxmbi_drop_cnt.Raw = READ_REG_WORD(e_rxmbi_drop_cnt);
    tmpReg->rxMbiSofDropCnt = rxmbi_drop_cnt.Bits.rxmbi_sofdrop_cnt;
    tmpReg->rxMbiSnfDropCnt = rxmbi_drop_cnt.Bits.rxmbi_snfdrop_cnt;

    /*Reg: rxmbi_drop_cnt2*/
    rxmbi_drop_cnt2.Raw = READ_REG_WORD(e_rxmbi_drop_cnt2);
    tmpReg->rxMbiCrcErrCnt = rxmbi_drop_cnt2.Bits.rxmbi_crcerr_cnt;
    tmpReg->rxMbiEndDropCnt = rxmbi_drop_cnt2.Bits.rxmbi_enddrop_cnt;

    /*Reg: txmbi_err_cnt*/
    tmpReg->txMbiErrCnt = READ_REG_WORD(e_txmbi_err_cnt);
    
    /*Reg: rxmbi_snf_cnt*/
    tmpReg->rxMbiSnfCnt = READ_REG_WORD(e_rxmbi_snf_cnt);

    /*Reg: rxmpi_uc_cnt*/
    tmpReg->rxMpiUcEthCnt = READ_REG_WORD(e_rxmpi_uc_cnt);
    
    /*Reg: e_rxmpi_bc_cnt*/
    tmpReg->rxMpiBcEthCnt = READ_REG_WORD(e_rxmpi_bc_cnt);

    /*Reg: e_rxmpi_mc_cnt*/
    tmpReg->rxMpiMcEthCnt = READ_REG_WORD(e_rxmpi_mc_cnt);

    /*Reg: rxmpi_oam_cnt*/
    tmpReg->rxMpiOamCnt = READ_REG_WORD(e_rxmpi_oam_cnt);

    /*Reg: rxmpi_mpcp_cnt*/
    tmpReg->rxMpiMpcpCnt = READ_REG_WORD(e_rxmpi_mpcp_cnt);

    /*Reg: rxmpi_gate_cnt*/
    tmpReg->rxMpiGateCnt = READ_REG_WORD(e_rxmpi_gate_cnt);
    
    /*Reg: rxmpi_nrlgate_cnt*/
    tmpReg->rxMpiNrlGateCnt = READ_REG_WORD(e_rxmpi_nrlgate_cnt);
    
    /*Reg: txmbi_uceth_cnt*/
    tmpReg->txMbiUcEthCnt = READ_REG_WORD(e_txmbi_uceth_cnt);

    /*Reg: txmbi_mceth_cnt*/
    txmbi_mceth_cnt.Raw = READ_REG_WORD(e_txmbi_mceth_cnt);
    tmpReg->txMbiBcEthCnt = txmbi_mceth_cnt.Bits.txmbi_bceth_cnt;
    tmpReg->txMbiMcEthCnt = txmbi_mceth_cnt.Bits.txmbi_mceth_cnt;
    
    /*Reg: tx_rpt_cnt*/
    tx_rpt_cnt.Raw = READ_REG_WORD(e_tx_rpt_cnt);
    tmpReg->txMbiRptCnt = tx_rpt_cnt.Bits.txmbi_rpt_cnt;
    tmpReg->txMpiRptCnt = tx_rpt_cnt.Bits.txmpi_rpt_cnt;

    /*Reg: txmpi_mpcp_cnt*/
    txmpi_mpcp_cnt.Raw = READ_REG_WORD(e_txmpi_mpcp_cnt);
    tmpReg->txMpiOamCnt = txmpi_mpcp_cnt.Bits.txmpi_oam_cnt;
    tmpReg->txMpiRgstReqCnt = txmpi_mpcp_cnt.Bits.txmpi_rgreq_cnt;
    tmpReg->txMpiRgstAckCnt = txmpi_mpcp_cnt.Bits.txmpi_rgack_cnt;
    
    /*Reg: txmpi_uceth_cnt*/
    tmpReg->txMpiUcEthCnt = READ_REG_WORD(e_txmpi_uceth_cnt);
    
    /*Reg: txmpi_mceth_cnt*/
    txmpi_mceth_cnt.Raw = READ_REG_WORD(e_txmpi_mceth_cnt);
    tmpReg->txMpiBcEthCnt = txmpi_mceth_cnt.Bits.txmpi_bceth_cnt;
    tmpReg->txMpiMcEthCnt = txmpi_mceth_cnt.Bits.txmpi_mceth_cnt;

    /*Reg: rxmpi_churn_cnt*/
    rxmpi_churn_cnt.Raw = READ_REG_WORD(e_rxmpi_churn_cnt);
    tmpReg->rxMpiChurnOkCnt = rxmpi_churn_cnt.Bits.rxmpi_churn_crcok_cnt;
    tmpReg->rxMpiChurnErrCnt = rxmpi_churn_cnt.Bits.rxmpi_churn_crcerr_cnt;

	return 0;
}

int an7581_epon_get_register_state(const void *in, void *out)
{
	epon_dbg_state_t *tmpReg = (epon_dbg_state_t *)out;

	
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	
    tmpReg->glbCfg = READ_REG_WORD(e_glb_cfg);
    tmpReg->glbCfg2 = READ_REG_WORD(e_glb_cfg2);
    tmpReg->intSts = READ_REG_WORD(e_int_status);
    tmpReg->intEn = READ_REG_WORD(e_int_en);
    tmpReg->intSts2 = READ_REG_WORD(e_int_sts2);
    tmpReg->intEn2 = READ_REG_WORD(e_int_en2);
    tmpReg->intSts3 = READ_REG_WORD(e_int_sts3);
    tmpReg->intEn3 = READ_REG_WORD(e_int_en3);
    tmpReg->llidCfg0_3 = READ_REG_WORD(e_llid0_3_cfg);
    tmpReg->rptCfg = READ_REG_WORD(e_rpt_cfg);
    tmpReg->laserTm = READ_REG_WORD(e_laser_onoff_time);
    tmpReg->syncTm = READ_REG_WORD(e_sync_time);
    tmpReg->txCnst = READ_REG_WORD(e_tx_cal_cnst);
    tmpReg->pwrSvCfg = READ_REG_WORD(e_pwr_sv_cfg);
    tmpReg->dygspCfg = READ_REG_WORD(e_dyinggsp_cfg);
    tmpReg->rdmDlySts = READ_REG_WORD(e_rdmdly_stat);

    tmpReg->glbSts1 = READ_REG_WORD(e_glb_sts1);
    tmpReg->glbSts2 = READ_REG_WORD(e_glb_sts2);
    tmpReg->dsKeyChg = READ_REG_WORD(e_ds_keychg);
    tmpReg->usKeyChg = READ_REG_WORD(e_us_keychg);
    tmpReg->rptBmp = READ_REG_WORD(e_rpt_bmap);
    tmpReg->oltOvTm = READ_REG_WORD(e_overhead_time_thr);
    tmpReg->crptCfg = READ_REG_WORD(e_crpt_cfg);
    tmpReg->rdmDlyCfg = READ_REG_WORD(e_rdmdly_cfg);

	return 0;
}

int an7581_epon_set_sniffer(const void *in, void *out)
{
    EPON_SNIFFER_MODE_T sniffer_mode = 0;
	REG_e_snf_cfg snf_val;
	snf_val.Raw = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	sniffer_mode = *(EPON_SNIFFER_MODE_T *)in;
	switch(sniffer_mode)
	{
		case EPON_SNIFFER_ENABLE_SELF_ALL:
			snf_val.Bits.sniffer_mode = 1;
			snf_val.Bits.llidinfo_snf = 1;
			snf_val.Bits.us_rpt_snf = 1;
			snf_val.Bits.us_mpcp_snf = 1;
			snf_val.Bits.us_oam_snf = 1;
			snf_val.Bits.ds_nrlgate_snf = 1;
			snf_val.Bits.ds_mpcp_snf = 1;
			printk("Epon EcoNet MPCP sniffer enable.\n");
			break;
		case EPON_SNIFFER_DISABLE_ALL:
			WRITE_REG_WORD(e_snf_cfg, 0);
			printk("Epon sniffer disable.\n");
			break;
		case EPON_SNIFFER_ENABLE_OTHER_MPCP:
			snf_val.Bits.sniffer_mode = 1;
			snf_val.Bits.llidinfo_snf = 1;
			snf_val.Bits.alluc_llid_snf = 1;
			snf_val.Bits.alluc_nrlgate_snf = 1;
			printk("Epon Other MPCP sniffer enable.\n");
			break;
		case EPON_SNIFFER_ENABLE_OTHER_ETHERNET:
			snf_val.Bits.sniffer_mode = 1;
			snf_val.Bits.llidinfo_snf = 1;
			snf_val.Bits.alluc_llid_snf = 1;
			snf_val.Bits.alluc_oam_snf = 1;
			snf_val.Bits.alluc_eth_snf = 1;
			printk("Epon Other Ether sniffer enable.\n");
			break;
		default:
			DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: ERROR Sniffer Mode", __FUNCTION__);
			return -1;
	};

	if(sniffer_mode == EPON_SNIFFER_DISABLE_ALL)
	{
		del_fix_reg_list((uint32_t)e_snf_cfg);
	}
	else
	{
		WRITE_REG_WORD(e_snf_cfg,snf_val.Raw);
        add_fix_reg_list((uint32_t)e_snf_cfg, READ_REG_WORD(e_snf_cfg));	
	}
	return 0;
}

int an7581_epon_set_static_report(const void *in, void *out)
{
    static_report_info_t *static_rpt =(static_report_info_t *)in;
	uint32_t raw = 0;
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	if(static_rpt->enable)
	{
		gp_epon_global_data->static_report_enable = 1;
		e_rpt_glb_cfg_SET_tx_default_rpt(e_rpt_glb_cfg, 1);
		add_fix_reg_list((uint32_t)e_rpt_glb_cfg, IO_GREG(e_rpt_glb_cfg));
		raw = IO_GREG(e_dfrpt_data5);
		raw = (raw & 0xFFFF0000) | (static_rpt->report_val&0xFFFF);
        IO_SREG(e_dfrpt_data5, raw);
        add_fix_reg_list((uint32_t)e_dfrpt_data5, raw);
        printk("queueSet[31:24]bitmap[23:16]q0buf[15:0] 0x%x\n", raw);
	}
	else
	{
		gp_epon_global_data->static_report_enable = 0;
		e_rpt_glb_cfg_SET_tx_default_rpt(e_rpt_glb_cfg, 0);
		del_fix_reg_list(e_rpt_glb_cfg);
		del_fix_reg_list(e_dfrpt_data5);
		printk("Static Report disable.\n");
	}

	return 0;
}

int an7581_epon_set_band_utilization(const void *in, void *out)
{
    uint32_t cfg = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	cfg = *(uint32_t *)in;

	if(cfg > 0x3){
		printk("[%s] bit0[1/0]: enable/disable; bit1[1]: clean utilization counter\n",__FUNCTION__);
		return -1;
	}
	printk("[%s] %x = %x\n", __FUNCTION__, (uint32_t)e_utili_check, cfg);
	if(cfg & 0x2){
		printk("clean utilization counter, ");
	}

	if(cfg & 0x1){
		printk("enable utilization function\n");
	}
	else{
		printk("disable utilization function\n");
	}

	WRITE_REG_WORD(e_utili_check,cfg);

	return 0;
}

int an7581_epon_tx_rate_analyze(const void *in, void *out)
{
	unsigned long long total_gnt_size = 0;	
	unsigned long total_pkt_count = 0;
	unsigned long long total_pkt_size = 0;
	unsigned long  total_u10g_nousegntlen = 0;
	unsigned long  total_gntcnt = 0;	
	unsigned long long total_tx_cyc = 0;
	unsigned int rate = 0;	
	unsigned int oltRate = 0;
	unsigned int syncTime = 0;
	unsigned int laserOnOffTime = 0;
	
	EPON_START_TIMER(eponTxRate,4000);
	if(gp_epon_global_data->tx_rate_mode ==1)
	{
		e_utili_check_SET_cf_utilization_chken(e_utili_check,0);		
		
		total_gnt_size =(((uint64_t)total_gnt_sizeh_GET_total_gnt_size_h32(total_gnt_sizeh))<<32) + total_gnt_sizel_GET_total_gnt_size_l32(total_gnt_sizel);
		total_pkt_count = total_pkt_cnt_GET_total_pkt_cnt(total_pkt_cnt);
		total_pkt_size = (((uint64_t)total_pkt_size_h_GET_total_pkt_size_h32(total_pkt_size_h))<<32) + total_pkt_size_l_GET_total_pkt_size_l32(total_pkt_size_l);	
		total_u10g_nousegntlen = total_u10g_nouse_gntlen_GET_total_u10g_nouse_gntlen(total_u10g_nouse_gntlen);
		total_gntcnt = total_gnt_cnt_GET_total_gnt_cnt(total_gnt_cnt);
		total_tx_cyc = (((uint64_t)total_tx_cyc_h_GET_total_tx_cyc_h32(total_tx_cyc_h))<<32) + total_tx_cyc_l_GET_total_tx_cyc_l32(total_tx_cyc_l);
		if((gpPonSysData->sysPonMode ==XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
		{
			rate= (total_pkt_size+total_pkt_count*20)*8*1000*1000/(total_tx_cyc*32);//kbps
		}else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){
			rate= (total_pkt_size+total_pkt_count*20)*8*1000*1000*5/(total_tx_cyc*32);//kbps
		}

		syncTime = e_sync_time_GET_sync_time(e_sync_time);
		laserOnOffTime = e_laser_onoff_time_GET_laser_off_time(e_laser_onoff_time) + e_laser_onoff_time_GET_laser_on_time(e_laser_onoff_time);

		if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
		{
			oltRate  = (total_gnt_size*16 - total_gntcnt*(syncTime+laserOnOffTime)*16)*1244*1000/(total_tx_cyc*32);//kbps
		}
		else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
		{
			oltRate  = ((total_gnt_size*16 - total_gntcnt*(syncTime+laserOnOffTime)*16)*10312)*10*1000*5/(total_tx_cyc*64);//kbps
		}

		e_utili_check_SET_cf_utilization_cntclr(e_utili_check,1);
		e_utili_cfg_SET_cf_utili_pkttype(e_utili_cfg,1); // only ether packet
		e_utili_cfg_SET_cf_utili_mode(e_utili_cfg,0);
		e_utili_check_SET_cf_utilization_chken(e_utili_check,1);
	}
	else if(gp_epon_global_data->tx_rate_mode ==0)
	{
		//e_utili_check_SET_cf_utilization_chken(e_utili_check,1);
		total_gnt_size =(((uint64_t)total_gnt_sizeh_GET_total_gnt_size_h32(total_gnt_sizeh))<<32) + total_gnt_sizel_GET_total_gnt_size_l32(total_gnt_sizel);
		total_pkt_count = total_pkt_cnt_GET_total_pkt_cnt(total_pkt_cnt);	
		total_pkt_size = (((uint64_t)total_pkt_size_h_GET_total_pkt_size_h32(total_pkt_size_h))<<32) + total_pkt_size_l_GET_total_pkt_size_l32(total_pkt_size_l);	
		total_u10g_nousegntlen = total_u10g_nouse_gntlen_GET_total_u10g_nouse_gntlen(total_u10g_nouse_gntlen);
		total_gntcnt = total_gnt_cnt_GET_total_gnt_cnt(total_gnt_cnt);
		//total_tx_cyc = (((uint64_t)total_tx_cyc_h_GET_total_tx_cyc_h32(total_tx_cyc_h))<<32) + total_tx_cyc_l_GET_total_tx_cyc_l32(total_tx_cyc_l);

		rate= (total_pkt_size+total_pkt_count*20)*8/1000;//Kbps
		syncTime = e_sync_time_GET_sync_time(e_sync_time);
		laserOnOffTime = e_laser_onoff_time_GET_laser_off_time(e_laser_onoff_time) + e_laser_onoff_time_GET_laser_on_time(e_laser_onoff_time);

		if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
		{
			oltRate  = (total_gnt_size*16 - total_gntcnt*(syncTime+laserOnOffTime)*16)*1244/(1000*1000);//kbps
		}
		else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
		{
			oltRate  = ((total_gnt_size*16 - total_gntcnt*(syncTime+laserOnOffTime)*16)*10312)/(1000*1000);//kbps
		}

		e_utili_check_SET_cf_utilization_cntclr(e_utili_check,1);
		e_utili_cfg_SET_cf_utili_pkttype(e_utili_cfg,0);// all packets, include mpcp report
		
		e_utili_cfg_SET_cf_utili_mode(e_utili_cfg,1);			
		e_utili_cfg_SET_cf_utili_wdm(e_utili_cfg,1000);//1s 	
		e_utili_cfg_SET_cf_utili_pktovhd(e_utili_cfg,0);
		e_utili_check_SET_cf_utilization_chken(e_utili_check,1);
	}
	printk("epon_rate:%d Kbps  oltRate:%d Kbps \n\n",rate,oltRate);

	return 0;
}
/*************************************************************************************************************/
/***********************************************only defined, but not used************************************/
/*************************************************************************************************************/

int an7581_epon_set_mac_report_qsizeadjs(const void *in, void *out)
{
    if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON \
       || gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON){
        e_u1g_rpt_qsizeadj_SET_u1g_fecon_rpt_qsizeadj(e_u1g_rpt_qsizeadj, 0xf1);
        e_u1g_rpt_qsizeadj_SET_u1g_fecoff_rpt_qsizeadj(e_u1g_rpt_qsizeadj, 0xf1);
    }else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){
        e_u10g_rpt_qsizeadj_SET_u10g_rpt_qsizeadj(e_u10g_rpt_qsizeadj, 0x19);
    }

	return EPON_SUCCESS;
}

int an7581_epon_set_llid_report_bitmap(const void *in, void *out)
{
	epon_llid_report_bitmap_t *llid_bitmap = (epon_llid_report_bitmap_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
    if(llid_bitmap->llidIndex>=EPON_LLID_MAX_NUM){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"Error ! llidIndex is %d, max llidIndex is %d\n",llid_bitmap->llidIndex,(EPON_LLID_MAX_NUM - 1));
		return EPON_NORMAL_ERROR;
	}

    /* set bitmap 0xff */
	e_rpt_bmap_SET_rpt_bitmap_ctrl(e_rpt_bmap, llid_bitmap->bitmap);
	e_rpt_bmap_SET_rpt_bitmap_set(e_rpt_bmap, llid_bitmap->bitmap);

	return EPON_SUCCESS;
}

int an7581_epon_set_mac_report_qsizeadjs_for_fec(const void *in, void *out)
{
	uint8_t fecFlag = 0;
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	fecFlag = *(uint8_t *)in;
	if(gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON ||\
       gpPonSysData->sysPonMode==XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON){
       if(fecFlag==ENABLE){
            e_u1g_rpt_qsizeadj_SET_u1g_fecon_rpt_qsizeadj(e_u1g_rpt_qsizeadj, 0x101);
        }
    }

    return 0;
}


