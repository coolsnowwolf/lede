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
#include "xepon_mac_c_header_en7580.h"
#include "common/union_ic_init.h"

/**********************************************************************************************
**************************************Extern Declare*******************************************
***********************************************************************************************/



/**********************************************************************************************
**************************************Private Define*******************************************
***********************************************************************************************/
#define SCU_WAN_CONF_REG            (0xBFB00070)    /* used to select GPON/EPON MAC */
#define EPON_MAC_BASE    			0xBFB60000
#define SCU_RESET_REG     			0xBFB00834  //bit31:for reset PON mac
#define EPON_REG_LOGIC_RST     		0xBFB00094  //bit10:for logic reset epon mac

static uint32_t g_epon_u1g_rpt_qsizeadj[4] = {0};
static uint32_t g_epon_u10g_rpt_qsizeadj[4] = {0};
static uint32_t g_epon_u1g_rpt_qsizeadj5[4] = {0};
static uint32_t llids_cfg_map[8] = {
		0xbfb66020,//e_llid0_3_cfg,
		0xbfb66024,//e_llid4_7_cfg,
		0xbfb66410,//e_llid8_11_cfg,
		0xbfb66414,//e_llid12_15_cfg,
		0xbfb66418,//e_llid16_19_cfg,
		0xbfb6641c,//e_llid20_23_cfg,
		0xbfb66420,//e_llid24_27_cfg,
		0xbfb66424,//e_llid28_31_cfg,
	
		/*add reg above here*/
};


typedef  union
{
  struct
  {
#ifdef __BIG_ENDIAN
    uint32_t qsize_fec_adj:16;
	uint32_t eponChn7QrptCfg:2;
	uint32_t eponChn6QrptCfg:2;
	uint32_t eponChn5QrptCfg:2;
	uint32_t eponChn4QrptCfg:2;
	uint32_t eponChn3QrptCfg:2;
	uint32_t eponChn2QrptCfg:2;
	uint32_t eponChn1QrptCfg:2;
	uint32_t eponChn0QrptCfg:2;
#else
	uint32_t eponChn0QrptCfg:2;
	uint32_t eponChn1QrptCfg:2;
	uint32_t eponChn2QrptCfg:2;
	uint32_t eponChn3QrptCfg:2;
	uint32_t eponChn4QrptCfg:2;
	uint32_t eponChn5QrptCfg:2;
	uint32_t eponChn6QrptCfg:2;
	uint32_t eponChn7QrptCfg:2;
	uint32_t qsize_fec_adj:16;

#endif /* __BIG_ENDIAN */
  } Bits;
  uint32_t Raw;
} eponRptCfgReg_t; 


static int epon_check_report_bitmap_rwcmd_done(void);
static int en7580_epon_triple_churning_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum);

/********************************************************************************************
 *********************************function table*********************************************
 ********************************************************************************************/
ic_func_tbl_t en7580_action[]=
{
{REGISTER_ACTION_EPON_CHECK_MAC_TX_MBI_STOP, 	en7580_epon_check_mac_tx_mbi_stop},
{REGISTER_ACTION_EPON_CHECK_MAC_TX_MPI_STOP, 	en7580_epon_check_mac_tx_mpi_stop},
{REGISTER_ACTION_EPON_SET_Q_THRESHLD_CFG,		en7580_epon_set_queue_threshold_cfg},
{REGISTER_ACTION_EPON_GET_Q_THRESHLD_CFG,		en7580_epon_get_queue_threshold_cfg},
{REGISTER_ACTION_EPON_GET_WAN_CONF,				en7580_epon_get_wan_cfg},
{PACKETS_ACTION_EPON_QOS_TX_RATE_METER_CFG,		en7580_epon_qos_tx_rate_meter_cfg},
{REGISTER_ACTION_EPON_DEFAULT_REPORT_METHOD,	en7580_epon_set_default_report_method},
{REGISTER_ACTION_EPON_SET_IPG,					en7580_epon_set_ipg},
{REGISTER_ACTION_EPON_MPCP_TIMEOUT_VALUE,		en7580_epon_set_mpcp_tmout_intvl},
{REGISTER_ACTION_EPON_RTT_ADJUST,				en7580_epon_rtt_adjust},
{REGISTER_ACTION_EPON_GET_REPORT_BITMAP,		en7580_epon_get_llid_report_bitmap},
{REGISTER_ACTION_EPON_INIT_DSCVRY_STS,			en7580_epon_init_discovery_status},
{REGISTER_ACTION_EPON_MAC_BASE_REG_ADDRESS,		en7580_epon_get_mac_reg_address},
{REGISTER_ACTION_EPON_SET_LLID_KEY, 			en7580_epon_set_llid_key},
{REGISTER_ACTION_EPON_GET_LLID_KEY, 			en7580_epon_get_llid_key},
{REGISTER_ACTION_EPON_SET_LLID_TX_FEC,			en7580_epon_set_llid_tx_fec},
{REGISTER_ACTION_EPON_GET_LLID_TX_FEC,			en7580_epon_get_llid_tx_fec},
{REGISTER_ACTION_EPON_SET_LLID_THRSHLD_NUM,		en7580_epon_set_llid_thrshld_num},
{REGISTER_ACTION_EPON_GET_LLID_THRSHLD_NUM, 	en7580_epon_get_llid_thrshld_num},
{PACKETS_ACTION_EPON_TX_BUFF_USAGE_CFG,			en7580_epon_set_tx_buff_usage},
{REGISTER_ACTION_EPON_SET_10G_LLID_KEY, 		en7580_epon_set_10G_llid_key},
{REGISTER_ACTION_EPON_GET_10G_LLID_KEY, 		en7580_epon_get_10G_llid_key},
{REGISTER_ACTION_PON_MAC_SCU_RESET,				en7580_pon_mac_scu_reset},
{REGISTER_ACTION_EPON_MAC_LOGIC_RESET,			en7580_epon_mac_logic_reset},
{ONLINE_ACTION_PON_GET_ONU_MODE_TYPE,			en7580_pon_get_onu_mode_type},
{REGISTER_ACTION_EPON_SET_DYGASP_HW_EN,			en7580_epon_set_dygasp_hw_en},
{REGISTER_ACTION_EPON_GET_DYGASP_HW_EN, 		en7580_epon_get_dygasp_hw_en},
{REGISTER_ACTION_EPON_SET_DYGASP_NUM,			en7580_epon_set_dygasp_num},
{REGISTER_ACTION_EPON_GET_DYGASP_NUM, 			en7580_epon_get_dygasp_num},

/*********************debug action function********************************/
{REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT,		en7580_epon_get_debug_statistic_count},
{REGISTER_ACTION_EPON_DBG_GET_STATE,			en7580_epon_get_register_state},
{REGISTER_ACTION_EPON_DBG_SET_SNIFFER,			en7580_epon_set_sniffer},
{REGISTER_ACTION_EPON_DBG_SET_STATIC_RPT,		en7580_epon_set_static_report},
/*******************only defined, but not used*****************************/
{REGISTER_ACTION_EPON_REPORT_QSIZEADJS, 		en7580_epon_set_mac_report_qsizeadjs},
{REGISTER_ACTION_EPON_REPORT_QSIZEADJS_FEC,		en7580_epon_set_mac_report_qsizeadjs_for_fec},
{REGISTER_ACTION_EPON_SET_REPORT_BITMAP,		en7580_epon_set_llid_report_bitmap},

/*************************please add above*********************************/
{MAX_ACTION_NUM, NULL}
};

/**********************************************************************************************
*************************************Function Define*******************************************
***********************************************************************************************/

int en7580_epon_check_mac_tx_mbi_stop(const void *in, void *out)
{
	return ((READ_REG_WORD(e_glb_sts) & (0x10))?TRUE:FALSE);
}

int en7580_epon_check_mac_tx_mpi_stop(const void *in, void *out)
{
	return ((READ_REG_WORD(e_glb_sts) & (0x40))?TRUE:FALSE);
}

int en7580_epon_set_queue_threshold_cfg(const void *in, void *out)
{
	unsigned int eponThresholdCfg = 0 ;
	eponQueueThreshold_t *queue_threshold = (eponQueueThreshold_t *)in;
	int threshold_chnn_shift = 3;
	int threshold_chnn_mask = 0x7;

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

int en7580_epon_get_queue_threshold_cfg(const void *in, void *out)
{
	unsigned int eponThresholdCfg = 0 ;
	eponQueueThreshold_t *queue_threshold = (eponQueueThreshold_t *)in;
	int threshold_chnn_shift = 3;
	int threshold_chnn_mask = 0x7;

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

int en7580_epon_get_wan_cfg(const void *in, void *out)
{
	uint32_t wan_raw = 0;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	wan_raw = READ_REG_WORD(SCU_WAN_CONF_REG);

	*(uint32_t *)out = wan_raw;

	return EPON_SUCCESS;
}


int en7580_epon_qos_tx_rate_meter_cfg(const void *in, void *out)
{
	QDMA_TxRateMeter_T *txRateMeter = (QDMA_TxRateMeter_T *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	txRateMeter->txRateMeterTimeSlice = 0x3e8;
	//txRateMeter->txRateMeterTimeDivisor = 0;

	return EPON_SUCCESS;
}

int en7580_epon_set_default_report_method(const void *in, void *out)
{
	REG_e_txsch_cfg regRaw;
    
    memset(&regRaw, 0, sizeof(REG_e_txsch_cfg));
   
    if(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){
        regRaw.Raw = READ_REG_WORD(e_txsch_cfg);
        regRaw.Bits.rpt_qsize_mgnt_sel = 1;
        WRITE_REG_WORD(e_txsch_cfg,regRaw.Raw);
    }

	return EPON_SUCCESS;
}

int en7580_epon_set_ipg(const void *in, void *out)
{
	uint8_t value = 0;
	uint32_t Raw = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	value = *(uint8_t *)in;
	
	Raw = READ_REG_WORD(e_tx_cal_cnst);
	Raw &= ~(0x1f);
	Raw |= (value & 0x1f);
	WRITE_REG_WORD(e_tx_cal_cnst , Raw);
	
	return EPON_SUCCESS;
}

int en7580_epon_set_mpcp_tmout_intvl(const void *in, void *out)
{
	uint32_t value = 0;
	uint32_t Raw = 0;
	
	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	value = *(uint32_t *)in;
	
	Raw = READ_REG_WORD(e_mpcp_timeout_intvl);
	Raw &= 0xfffffc00;
	Raw |= (value & 0x3FF);
	WRITE_REG_WORD(e_mpcp_timeout_intvl, Raw);

	return EPON_SUCCESS;
}

int en7580_epon_rtt_adjust(const void *in, void *out){
	uint32_t Raw = 0;
	
	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON){
		Raw = READ_REG_WORD(e_trx_adjust_time2);
		Raw &= 0xffff0000;
		Raw |= RX_TMSTP_ADJ;
		WRITE_REG_WORD(e_trx_adjust_time2,Raw);
	}else {/* xepon */
		Raw = READ_REG_WORD(e_trx_adjtime4);
		Raw &= 0xffff;
		Raw |= (D10G_RX_TSADJ << 16);
		WRITE_REG_WORD(e_trx_adjtime4,Raw);

		if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON){ /* xepon-sym */
			Raw = READ_REG_WORD(e_trx_adjtime3);
			Raw &= 0xffff0000;
			Raw |= U10G_TX_STMADJ;
			WRITE_REG_WORD(e_trx_adjtime3,Raw);
		}
	}

	return EPON_SUCCESS;
}

int en7580_epon_get_llid_report_bitmap(const void *in, void *out)
{
	REG_e_rpt_bitmap_cfg bitmapCfg;
	REG_e_rpt_bitmap_val bitmapVal;
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
	memset(&bitmapCfg, 0 , sizeof(REG_e_rpt_bitmap_cfg));
	memset(&bitmapVal, 0 , sizeof(REG_e_rpt_bitmap_val));
	
	bitmapCfg.Raw = READ_REG_WORD(e_rpt_bitmap_cfg);
	bitmapCfg.Bits.rpt_bmap_rwcmd = 0;
	bitmapCfg.Bits.rpt_bmap_llid_idx = report_bitmap->llidIndex;

	WRITE_REG_WORD(e_rpt_bitmap_cfg, bitmapCfg.Raw);
	bitmapVal.Raw = READ_REG_WORD(e_rpt_bitmap_val);
	if(epon_check_report_bitmap_rwcmd_done() != 0){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_check_report_bitmap_rwcmd_done read fail\n");
		return EPON_NORMAL_ERROR;
	}

	report_bitmap->bitmap = bitmapVal.Bits.rpt_bitmap_set;
	return EPON_SUCCESS;
}


int en7580_epon_init_discovery_status(const void *in, void *out)
{
	int i = 0;
	for(i = 0;i < 4;i++){
		g_epon_u1g_rpt_qsizeadj[i] = (uint32_t)e_u1g_rpt_qsizeadj(i);
	}
	for(i = 0;i < 4;i++){
		g_epon_u10g_rpt_qsizeadj[i] = (uint32_t)e_u10g_rpt_qsizeadj(i);
	}
	for(i = 0;i < 8;i++){
		g_epon_llid_dscv_stat[i] = (uint32_t)e_llid0_7_dscvry_sts(i);
	}
	for(i = 0;i < 24;i++){
		g_epon_llid_dscv_stat[i+8] = (uint32_t)e_llid8_31_dscvry_sts(i);
	}
	for(i = 0;i < 3;i++){
		g_epon_u1g_rpt_qsizeadj5[i] = (uint32_t)e_u1g_rpt_qsizeadj(4 + i);
	}

	return EPON_SUCCESS;
}

int en7580_epon_get_mac_reg_address(const void *in, void *out)
{
	g_EPON_MAC_BASE = (PEPON_MAC_REGS)(ioremap_nocache(CONFIG_EPON_BASE_ADDR, CONFIG_EPON_REG_RANGE)) ; 
	if(!g_EPON_MAC_BASE) {
		printk("ERR: ioremap the EPON base address failed:addr=0x%x,size=0x%x\n",CONFIG_EPON_BASE_ADDR,(uint32_t)CONFIG_EPON_REG_RANGE) ;
		return -ENOMEM ;
	}
    return EPON_SUCCESS;
}

int en7580_epon_set_llid_key(const void *in, void *out)
{
    /* This function for 1G setting churning */
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	uint32_t keyValue = 0;
	uint32_t timers = 0;
	uint8_t *cp = NULL;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL; /*network byte order buffer*/
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
	key = io_info->info;

	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));
	
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

	cp = (__u8 *)(&keyValue);
#ifdef __LITTLE_ENDIAN
			cp[2] = key[0];
			cp[1] = key[1];
			cp[0] = key[2];
#else
			cp++;
			memcpy(cp , key , 3);
#endif
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetLlidKey keyValue = 0x%x", keyValue);
	WRITE_REG_WORD(e_key_value , keyValue);
	
	eponSecurityKeyCfgReg.Bits.key_rwcmd = 1;
	eponSecurityKeyCfgReg.Bits.key_llid_index = (llidIndex&KEY_LLID_INDEX_MASK);
	eponSecurityKeyCfgReg.Bits.key_idx = (keyIndex&0x01);
	gp_epon_global_data->llid_entry[llidIndex].llid_key_index = (keyIndex&0x01);
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetLlidKey eponSecurityKeyCfgReg = 0x%x", eponSecurityKeyCfgReg.Raw);	
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
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\n eponSetLlidKey set  timers == 100");
		return -1;
	}
	return 0;
}

int en7580_epon_get_llid_key(const void *in, void *out)
{
    /* This function for 1G setting churning */
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	uint32_t keyValue = 0;
	uint32_t timers = 0;
	uint8_t *cp = NULL;
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL; /*network byte order buffer*/
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
	key = io_info->info;

	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));
	
    if((llidIndex>=EPON_LLID_MAX_NUM)||(keyIndex>1)||(key==NULL)){
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "[%s %d]Param error.", __FUNCTION__, __LINE__);
        return -1;
    }

	eponSecurityKeyCfgReg.Bits.key_rwcmd = 0;	
    eponSecurityKeyCfgReg.Bits.key_llid_index = (llidIndex&KEY_LLID_INDEX_MASK);	
	eponSecurityKeyCfgReg.Bits.key_idx = (keyIndex&0x01);
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
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\n eponSetLlidKey get  timers == 100");
		return -1;
	}
	
	keyValue = READ_REG_WORD(e_key_value);
	cp = (__u8 *)&keyValue;
	cp++;
	memcpy(key , cp ,3);
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetLlidKey key=0x%x", keyValue);
	return 0;

}

int en7580_epon_set_llid_tx_fec(const void *in, void *out)
{
	uint8_t llidIndex = 0;
	uint8_t fecFlag = 0;
    uint32_t regAddr = 0;
	uint32_t Raw = 0;
	uint8_t offset = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

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

	regAddr = llids_cfg_map[llidIndex/4];
	offset = (llidIndex - (llidIndex/4)*4)*8 + 3;

	Raw = READ_REG_WORD(regAddr);
	if(fecFlag == ENABLE){//enable
		Raw |= (1<<offset);
	}else{//disable
		Raw &= ~(1<<offset);
	}
	
	WRITE_REG_WORD(regAddr , Raw);
	return 0;
}

int en7580_epon_get_llid_tx_fec(const void *in, void *out)
{
	uint8_t llidIndex = 0;
	uint8_t fecFlag = 0;
    uint32_t regAddr = 0;
	uint32_t Raw = 0;
	uint8_t offset = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidIndex = io_info->llidIndex;
	
	if(llidIndex >= EPON_LLID_MAX_NUM){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetLlidFec param error");	
		return -1;
	}

	if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
	{
		fecFlag = 1;	/*10g-10g-EPON mode, tx fec is forced on*/
	}
	else
	{

		regAddr = llids_cfg_map[llidIndex/4];
		offset = (llidIndex - (llidIndex/4)*4)*8 + 3;

		Raw = READ_REG_WORD(regAddr);
		if(Raw & (1<<offset)){//enable
			fecFlag = 1;
		}else{//disable
			fecFlag = 0;
		}
	}

	io_info->param0 = fecFlag;
	
	return 0;
}

int en7580_epon_set_llid_thrshld_num(const void *in, void *out)
{
	uint32_t regAddr[3] = {(uint32_t)e_rpt_cfg,(uint32_t)e_rpt_cfg2,(uint32_t)e_rpt_cfg3};
	uint32_t llidRegAddr = 0;
	uint32_t Raw =0;
	eponRptCfgReg_t eponRptCfgReg;
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

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "\r\nenter eponSetLlidThrshldNum llidIndex=%d ,num=%d ", llidIndex, num);	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(num > 3)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetLlidThrshldNum param error");	
		return -1;
	}
	eponRptCfgReg.Raw = 0;	
	
	Raw = num;
	if(llidIndex <= 7){
		Raw <<= (llidIndex*2);
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[0]);
		eponRptCfgReg.Raw &= ~(3<<(llidIndex*2));
		llidRegAddr = regAddr[0];
	}else if(llidIndex >= 8 && llidIndex <= 15){
		Raw <<= ((llidIndex-8)*2);
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[1]);
		eponRptCfgReg.Raw &= ~(3<<((llidIndex-8)*2));
		llidRegAddr = regAddr[1];
	}else{
		Raw <<= ((llidIndex-16)*2);
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[2]);
		eponRptCfgReg.Raw &= ~(3<<((llidIndex-16)*2));
		llidRegAddr = regAddr[2];
	}
	
	eponRptCfgReg.Raw |= Raw;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetLlidThrshldNum eponRptCfgReg.Raw=%x", eponRptCfgReg.Raw);	
	WRITE_REG_WORD(llidRegAddr , eponRptCfgReg.Raw);
	return 0;
}

int en7580_epon_get_llid_thrshld_num(const void *in, void *out)

{
	uint32_t regAddr[3] = {(uint32_t)e_rpt_cfg,(uint32_t)e_rpt_cfg2,(uint32_t)e_rpt_cfg3};
	eponRptCfgReg_t eponRptCfgReg;
	uint8_t llidIndex = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)out;

	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	llidIndex = io_info->llidIndex;

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "\r\nenter eponGetLlidQueNum");	
	if((llidIndex >= EPON_LLID_MAX_NUM)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponGetLlidQueNum param error");	
		return -1;
	}
	
	eponRptCfgReg.Raw = 0;
	
	if(llidIndex <= 7){
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[0]);
		eponRptCfgReg.Raw &= (3<<(llidIndex*2));
		eponRptCfgReg.Raw >>= (llidIndex*2);
	}else if((llidIndex >= 8) && (llidIndex <= 15)){
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[1]);
		eponRptCfgReg.Raw &= (3<<((llidIndex-8)*2));
		eponRptCfgReg.Raw >>= ((llidIndex-8)*2);
	}else{	//llidIndex >= 16 && llidIndex <= 31
		eponRptCfgReg.Raw = READ_REG_WORD(regAddr[2]);
		eponRptCfgReg.Raw &= (3<<((llidIndex-16)*2));
		eponRptCfgReg.Raw >>= ((llidIndex-16)*2);
	}

	io_info->info[0]= (uint8_t)eponRptCfgReg.Raw;
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponGetLlidQueNum eponRptCfgReg.Raw=%x", io_info->info[0]);	
	return 0;
}


int en7580_epon_set_tx_buff_usage(const void *in, void *out)
{
	uint32_t llidMask = 0;
	uint8_t llidIndex = 0;
	uint8_t llidCnt = 0;
	uint32_t value = 0;

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
    return 0;
}

static int en7580_epon_triple_churning_key_cfg(uint8_t opt,uint8_t llidIndex, uint8_t keyIndex,uint8_t keyNum)
{
	uint32_t timers = 0;
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	
	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));
	eponSecurityKeyCfgReg.Bits.key_rwcmd = opt;
    eponSecurityKeyCfgReg.Bits.key_llid_index = (llidIndex&KEY_LLID_INDEX_MASK);
	eponSecurityKeyCfgReg.Bits.key_idx = (keyIndex&0x01);
    eponSecurityKeyCfgReg.Bits.key_dw_indx = keyNum;
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetLlidKey eponSecurityKeyCfg = 0x%x", eponSecurityKeyCfgReg.Raw);	
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
		printk("eponSetLlidKey set  timers == 100 key %x \n",keyNum);
		return -1;
	}
    
    return 0;
}

int en7580_epon_set_10G_llid_key(const void *in, void *out)
{
	uint32_t keyValue = 0;
	uint8_t *cp = NULL;
    uint8_t i = 0;
    REG_e_llid0_3_cfg llid_cfg ;
	REG_e_key_value key_val ;
    uint32_t val = 0;
#ifdef __LITTLE_ENDIAN
    uint8_t key_tmp[3] = {0};
	uint8_t *key_tmp_p = NULL;
#endif
	uint8_t llidIndex = 0;
	uint8_t keyIndex = 0;
	uint8_t *key = NULL;
	uint32_t regAddr = 0;
	eponMacIoctl_t *io_info = (eponMacIoctl_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	llidIndex = io_info->llidIndex;
	keyIndex = io_info->param0;
    key = io_info->info;

    llid_cfg.Raw = 0;
    key_val.Raw = 0;
	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(keyIndex >1)||(key == NULL)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetLlidKey param error");	
		return -1;
	}
	
	for(i = 0;i < EPON_10G_MAX_KEY_NUM; i++)
    {
		cp = (uint8_t *)(&keyValue);
		
#ifdef __LITTLE_ENDIAN		
		key_tmp_p = key;
		key_tmp[2] = *key_tmp_p++;
		key_tmp[1] = *key_tmp_p++;
		key_tmp[0] = *key_tmp_p;
		memcpy(cp , key_tmp , EPON_THURNING_KEY_LEN);
#else
		cp++;
		memcpy(cp , key , EPON_THURNING_KEY_LEN);
#endif
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSet 10G LlidKey %d keyValue = 0x%x",i, keyValue);
		key_val.Bits.key_value = keyValue;
		WRITE_REG_WORD(e_key_value , key_val.Raw);
        
        if(0 != en7580_epon_triple_churning_key_cfg(EPON_WRITE_KEY,llidIndex,keyIndex,i))
        {
        	printk("set eponThurningKeyCfg Fail \n");
            return -1;
        }
        key+=3;
    }

	regAddr = llids_cfg_map[llidIndex/4];

	llid_cfg.Raw = READ_REG_WORD(regAddr);
	val = llid_cfg.Raw | (0x4 << ((llidIndex - (llidIndex/4)*4)*8));
	WRITE_REG_WORD(regAddr , val);

  

    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"set key reg config read is %x, write is %x\n",llid_cfg.Raw,val);
	return 0;
	
}

int en7580_epon_get_10G_llid_key(const void *in, void *out)
{
	uint32_t keyValue = 0;
	uint8_t *cp = NULL;
    uint8_t i = 0;
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
	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(keyIndex >1)||(key == NULL)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetLlidKey param error");	
		return -1;
	}
    

	for(i = 0;i < EPON_10G_MAX_KEY_NUM; i++)
    {
        if(0 != en7580_epon_triple_churning_key_cfg(EPON_READ_KEY,llidIndex,keyIndex,i))
        {
        	printk("eponThurningKeyCfg Fail \n");
            return -1;
        }
        keyValue = READ_REG_WORD(e_key_value);
		cp = (uint8_t *)&keyValue;
		cp++;
		memcpy(key , cp ,EPON_THURNING_KEY_LEN);
        key+=3;
        printk("epon get 10G LlidKey %d Value = 0x%x",i, keyValue);
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "eponSet 10G LlidKey %d keyValue = 0x%x",i, keyValue);
    }
    return 0;
}

int en7580_pon_mac_scu_reset(const void *in, void *out)
{
	int Raw = 0;
	Raw = READ_REG_WORD(SCU_RESET_REG);
	Raw |= SCU_RESET_BIT;
	WRITE_REG_WORD(SCU_RESET_REG , Raw);
    udelay(1);
	Raw = READ_REG_WORD(SCU_RESET_REG);
	Raw &= (~SCU_RESET_BIT);
	WRITE_REG_WORD(SCU_RESET_REG , Raw);
	return 0;
}

int en7580_epon_mac_logic_reset(const void *in, void *out)
{
	int Raw = 0;
	int hold_on_off = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
	hold_on_off = *(int *)in;

	Raw = READ_REG_WORD(EPON_REG_LOGIC_RST);
	if(hold_on_off == EPON_LOGIC_RESET_HOLD_ON){
		Raw |= EPON_LOGIC_RESET_BIT;
	}else if(hold_on_off == EPON_LOGIC_RESET_HOLD_OFF){
		Raw &= (~EPON_LOGIC_RESET_BIT);
	}else{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: Error INPUT Parameter, hold_on_off=%d ", __FUNCTION__, hold_on_off);
		return EPON_INPUT_PARA_ERROR;
	}	
	
	WRITE_REG_WORD(EPON_REG_LOGIC_RST , Raw);
    udelay(1);
	return 0;
}

int en7580_pon_get_onu_mode_type(const void *in, void *out)
{
	uint32_t mode_type = 0;
	
	mode_type = READ_FLASH_BYTE(flash_base + XPON_MODE_FLASH_ADDR);

	gpPonSysData->sysOnuType = mode_type & ONUTYPE_MASK;
	gpPonSysData->sysPonMode = (mode_type & ONUMODE_MASK )>>4;
	gpPonSysData->sysBBF247 = (mode_type & BBF247_MASK) >> 3;

	dump_pon_type_mode_info(gpPonSysData->sysOnuType, gpPonSysData->sysPonMode);
	return 0;
}

int en7580_epon_set_dygasp_hw_en(const void *in, void *out)
{
	uint8_t dygasp_hw_en = 1;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	dygasp_hw_en = *(uint8_t *)in;

	gpPonSysData->dyingGaspData.isEponHwFlag = !!dygasp_hw_en;

	e_dyinggsp_cfg_SET_hw_dying_gasp_en(e_dyinggsp_cfg, dygasp_hw_en);

	return EPON_SUCCESS;
}

int en7580_epon_get_dygasp_hw_en(const void *in, void *out)
{
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	*(uint8_t *)out = e_dyinggsp_cfg_GET_hw_dying_gasp_en(e_dyinggsp_cfg);

	return EPON_SUCCESS;
}

int en7580_epon_set_dygasp_num(const void *in, void *out)
{
	uint32_t dygasp_num = 1;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	dygasp_num = *(uint32_t *)in;

	e_dyinggsp_cfg_SET_dygsp_num_of_times(e_dyinggsp_cfg, dygasp_num);

	return EPON_SUCCESS;
}

int en7580_epon_get_dygasp_num(const void *in, void *out)
{
	if(out == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: OUTPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	*(uint32_t *)out = e_dyinggsp_cfg_GET_dygsp_num_of_times(e_dyinggsp_cfg);

	return EPON_SUCCESS;
}

/*************************************************************************************************************/
/***********************************************debug function defined****************************************/
/*************************************************************************************************************/

/*===================================================================*/
/*                 en7580 epon mac register table                    */
/*===================================================================*/
reg_check_t en7580_epon_reg[]=
{
	/*Register name , Type, Address, Default Value, Mask*/
	{"e_glb_cfg", 								RW, 			(EPON_MAC_BASE + 0x6000), 0x00800002, 0x07ff3fbe},
	{"e_int_status", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6004), 0x00000000, 0x00000000},
	{"e_int_en", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x6008), 0x00000000, 0x07ffffff},
	{"e_rpt_mpcp_timeout_llid_idx", 			RO|NO_DEF, 		(EPON_MAC_BASE + 0x600c), 0x00000000, 0x00000000},
	{"e_dyinggsp_cfg", 							RO|RW, 			(EPON_MAC_BASE + 0x6010), 0x00000100, 0x80000300},
	{"e_pending_gnt_num", 						RO|RW, 			(EPON_MAC_BASE + 0x6014), 0x00000040, 0x000000ff},
	{"e_llid0_3_cfg", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6020), 0x00000000, 0xffffffff},
	{"e_llid4_7_cfg", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6024), 0x00000000, 0xffffffff},
	{"e_llid_dscvry_ctrl", 						RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6028), 0x00000000, 0xc001111f},
	{"e_llid0_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x602c), 0x00000000, 0xc0000000},
	{"e_llid1_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6030), 0x00000000, 0xc0000000},
	{"e_llid2_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6034), 0x00000000, 0xc0000000},
	{"e_llid3_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6038), 0x00000000, 0xc0000000},
	{"e_llid4_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x603c), 0x00000000, 0xc0000000},
	{"e_llid5_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6040), 0x00000000, 0xc0000000},
	{"e_llid6_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6044), 0x00000000, 0xc0000000},
	{"e_llid7_dscvry_status", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6048), 0x00000000, 0xc0000000},
	{"e_mac_addr_cfg", 							RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6050), 0x00000000, 0x8000003f},
	{"e_mac_addr_value", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6054), 0x00000000, 0x00000000},
	{"e_security_key_cfg", 						W1C|RW|NO_DEF, 	(EPON_MAC_BASE + 0x6058), 0x00000000, 0x800000ff},
	{"e_key_value", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x605c), 0x00000000, 0x00000000},
	{"e_rpt_cfg", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x6068), 0x00000000, 0x0000ffff},
	{"e_rpt_qthld_cfg", 						RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x606c), 0x00000000, 0x800000ff},
	{"e_local_time", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6080), 0x00000000, 0x00000000},
	{"e_tod_sync_x", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6084), 0x00000000, 0xffffffff},
	{"e_tod_ltncy", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6088), 0x00000000, 0x00000000},
	{"e_new_tod_p2p_offset_sec_l32", 			RW|NO_DEF, 		(EPON_MAC_BASE + 0x6094), 0x00000000, 0xffffffff},
	{"e_new_tod_p2p_offset_nsec", 				RW|NO_DEF, 		(EPON_MAC_BASE + 0x6098), 0x00000000, 0xffffffff},
	{"e_tod_p2p_tod_sec_l32", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x609c), 0x00000000, 0x00000000},
	{"e_tod_p2p_tod_nsec", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x60a0), 0x00000000, 0x00000000},
	{"e_tod_period", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x60a4), 0x00000000, 0x000000ff},
	{"e_pwr_sv_cfg", 							RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x60b0), 0x00000000, 0x3700ffff},
	{"e_slp_durt_max", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x60b4), 0x00000000, 0xffffffff},
	{"e_slp_duration", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x60b8), 0x00000000, 0xffffffff},
	{"e_act_duration", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x60bc), 0x00000000, 0xffffffff},
	{"e_pwron_dly", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x60c0), 0x00000000, 0xffffffff},
	{"e_slp_duration", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x60c4), 0x00000000, 0x00000000},
	{"e_txfetch_cfg", 							RW, 			(EPON_MAC_BASE + 0x60d0), 0x002a03e8, 0x00ffffff},
	{"e_sync_time", 							W1C|RO|NO_DEF, 	(EPON_MAC_BASE + 0x60d4), 0x00000000, 0x00000000},
	{"e_tx_cal_cnst", 							RW, 			(EPON_MAC_BASE + 0x60d8), 0x2a12040c, 0xffffff3f},
	{"e_laser_onoff_time", 						RO, 			(EPON_MAC_BASE + 0x60dc), 0x00002020, 0x00000000},
	{"e_grd_threshold", 						RW, 			(EPON_MAC_BASE + 0x60e0), 0x00000008, 0x000000ff},
	{"e_mpcp_timeout_intvl", 					RW, 			(EPON_MAC_BASE + 0x60e4), 0x010001f4, 0xffffffff},
	{"e_rpt_timeout_intvl", 					RW, 			(EPON_MAC_BASE + 0x60e8), 0x00000010, 0x00ffffff},
	{"e_max_future_gnt_time", 					RW, 			(EPON_MAC_BASE + 0x60ec), 0x03b9aca0, 0xffffffff},
	{"e_min_proc_time", 						RW, 			(EPON_MAC_BASE + 0x60f0), 0x00000400, 0x0000ffff},
	{"e_trx_adjust_time1", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x60f4), 0x00000000, 0x0000ffff},
	{"e_trx_adjust_time2", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x60f8), 0x00000000, 0xffffffff},
	{"e_dbg_prb_sel", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6100), 0x00000000, 0x00031f1f},
	{"e_dbg_prb_h32", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6104), 0x00000000, 0x00000000},
	{"e_dbg_prb_l32", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6108), 0x00000000, 0x00000000},
	{"e_rxmbi_eth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x610c), 0x00000000, 0x00000000},
	{"e_rxmpi_eth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6110), 0x00000000, 0x00000000},
	{"e_txmbi_eth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6114), 0x00000000, 0x00000000},
	{"e_txmpi_eth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6118), 0x00000000, 0x00000000},
	{"e_oam_sta", 								RO|NO_DEF, 		(EPON_MAC_BASE + 0x611c), 0x00000000, 0x00000000},
	{"e_mpcp_sta", 								RO|NO_DEF, 		(EPON_MAC_BASE + 0x6120), 0x00000000, 0x00000000},
	{"e_mpcp_rgst_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6124), 0x00000000, 0x00000000},
	{"e_gnt_pending_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6128), 0x00000000, 0x00000000},
	{"e_gnt_length_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x612c), 0x00000000, 0x00000000},
	{"e_gnt_type_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6130), 0x00000000, 0x00000000},
	{"e_time_drft_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6134), 0x00000000, 0x00000000},
	{"e_llid0_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6138), 0x00000000, 0x00000000},
	{"e_llid1_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x613c), 0x00000000, 0x00000000},
    {"e_llid2_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6140), 0x00000000, 0x00000000},
    {"e_llid3_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6144), 0x00000000, 0x00000000},
    {"e_llid4_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6148), 0x00000000, 0x00000000},
    {"e_llid5_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x614c), 0x00000000, 0x00000000},
    {"e_llid6_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6150), 0x00000000, 0x00000000},
    {"e_llid7_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6154), 0x00000000, 0x00000000},
    {"e_snf_mpcp_oam_ctl", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6160), 0x00000000, 0xffffffff},
    {"e_dyinggsp_w1", 							RW, 			(EPON_MAC_BASE + 0x616c), 0x88090300, 0xffffffff},
    {"e_dyinggsp_w2", 							RW, 			(EPON_MAC_BASE + 0x6170), 0x52000110, 0xffffffff},
    {"e_dyinggsp_w3", 							RW, 			(EPON_MAC_BASE + 0x6174), 0x01000000, 0xffffffff},
    {"e_dyinggsp_w4", 							RW, 			(EPON_MAC_BASE + 0x6178), 0x0f05ee00, 0xffffffff},
    {"e_dyinggsp_w5", 							RW, 			(EPON_MAC_BASE + 0x617c), 0x13250022, 0xffffffff},
    {"e_dyinggsp_w6", 							RW, 			(EPON_MAC_BASE + 0x6180), 0x01000210, 0xffffffff},
    {"e_dyinggsp_w7", 							RW, 			(EPON_MAC_BASE + 0x6184), 0x01000000, 0xffffffff},
    {"e_dyinggsp_w8", 							RW, 			(EPON_MAC_BASE + 0x6188), 0x0f05ee00, 0xffffffff},
    {"e_dyinggsp_w9", 							RW, 			(EPON_MAC_BASE + 0x618c), 0x13250000, 0xffffffff},
    {"e_dyinggsp_w10", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6190), 0x00000000, 0xffffffff},
    {"e_dyinggsp_w11", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6194), 0x00000000, 0xffffffff},
    {"e_dyinggsp_w12", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6198), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw1", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x619c), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw2", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61a0), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw3", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61a4), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw4", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61a8), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw5", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61ac), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw6", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61b0), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw7", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61b4), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw8", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61b8), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw9", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61bc), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw10", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61c0), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw11", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61c4), 0x00000000, 0xffffffff},
    {"e_oam_kpalvw12", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x61c8), 0x00000000, 0xffffffff},
    {"e_oam_kpalv_ctrl", 						RW, 			(EPON_MAC_BASE + 0x61cc), 0x000fff00, 0x03ffff07},
    {"e_tod_1pps_ctrl", 						RW, 			(EPON_MAC_BASE + 0x61d0), 0x01312d00, 0xffffffff},
    {"e_sniff_sp_tag", 							RW, 			(EPON_MAC_BASE + 0x61d8), 0x00070000, 0xffffffff},
    {"e_glb_cfg2", 								RW, 			(EPON_MAC_BASE + 0x6200), 0xc3228c00, 0xff77ffe7},
    {"e_int_sts2", 								W1C|NO_DEF, 	(EPON_MAC_BASE + 0x6204), 0x00000000, 0x00000000},
    {"e_int_en2", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x6208), 0x00000000, 0x000007ff},
    {"e_exc_sts", 								W1C|NO_DEF, 	(EPON_MAC_BASE + 0x620c), 0x00000000, 0x00000000},
    {"e_olt_dscvinfo", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6210), 0x00000000, 0x00000000},
    {"e_laser_onoff_time2", 					RO, 			(EPON_MAC_BASE + 0x6214), 0x00000000, 0x00000000},
    {"e_trx_adjtime3", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6218), 0x00000000, 0xffffffff},
    {"e_trx_adjtime4", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x621c), 0x00000000, 0xffff1f00},
    {"e_trx_adjtime5", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6220), 0x00000000, 0xffff0000},
    {"e_rxphydly_adjtime", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6228), 0x00000000, 0x8fffffff},
    {"e_overhead_time_thr", 					RW, 			(EPON_MAC_BASE + 0x622c), 0xe0df2020, 0xe0ffffff},
    {"e_txcal_cnst2", 							RW, 			(EPON_MAC_BASE + 0x6230), 0xa2000d04, 0x8000ffff},
    {"e_txcal_cnst3", 							RW, 			(EPON_MAC_BASE + 0x6234), 0x37030e2a, 0xffffffff},
    {"e_txsch_cfg", 							RW, 			(EPON_MAC_BASE + 0x6238), 0x00400020, 0xffffffff},
    {"e_rxfifo_thr", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x623c), 0x00000000, 0x0000001f},
    {"e_bcllid_cfg", 							RW, 			(EPON_MAC_BASE + 0x6240), 0x7ffe7fff, 0xffffffff},
    {"e_txfrm_cfg1", 							RW, 			(EPON_MAC_BASE + 0x6244), 0xc2000001, 0xffffffff},
    {"e_txfrm_cfg2", 							RW, 			(EPON_MAC_BASE + 0x6248), 0x01808808, 0xffffffff},
    {"e_txfrm_cfg3", 							RW, 			(EPON_MAC_BASE + 0x624c), 0x00040006, 0xffffffff},
    {"e_u1g_rpt_qsizeadj1", 					RW, 			(EPON_MAC_BASE + 0x6250), 0x00020002, 0xffffffff},
    {"e_u1g_rpt_qsizeadj2", 					RW, 			(EPON_MAC_BASE + 0x6254), 0x00020002, 0xffffffff},
    {"e_u1g_rpt_qsizeadj3", 					RW, 			(EPON_MAC_BASE + 0x6258), 0x00020002, 0xffffffff},
    {"e_u1g_rpt_qsizeadj4", 					RW, 			(EPON_MAC_BASE + 0x625c), 0x00020002, 0xffffffff},
    {"e_u1g_rpt_qsizeadj5", 					RW, 			(EPON_MAC_BASE + 0x6260), 0x000a000a, 0xffffffff},
    {"e_u1g_rpt_qsizeadj6", 					RW, 			(EPON_MAC_BASE + 0x6264), 0x000a000a, 0xffffffff},
    {"e_u1g_rpt_qsizeadj7", 					RW, 			(EPON_MAC_BASE + 0x6268), 0x000a000a, 0xffffffff},
    {"e_u1g_rpt_qsizeadj8", 					RW, 			(EPON_MAC_BASE + 0x626c), 0x000a000a, 0xffffffff},
    {"e_u10g_rpt_qsizeadj1", 					RW, 			(EPON_MAC_BASE + 0x6270), 0x00020002, 0xffffffff},
    {"e_u10g_rpt_qsizeadj2", 					RW, 			(EPON_MAC_BASE + 0x6274), 0x00020002, 0xffffffff},
    {"e_u10g_rpt_qsizeadj3", 					RW, 			(EPON_MAC_BASE + 0x6278), 0x00020002, 0xffffffff},
    {"e_u10g_rpt_qsizeadj4", 					RW, 			(EPON_MAC_BASE + 0x627c), 0x00020002, 0xffffffff},
    {"e_oam_rpt_pkt_sizeadj", 					RW, 			(EPON_MAC_BASE + 0x6280), 0x80000018, 0x0000ffff},
    {"e_snf_dah", 								RW, 			(EPON_MAC_BASE + 0x6290), 0x00554433, 0xffffffff},
    {"e_snf_dal", 								RW, 			(EPON_MAC_BASE + 0x6294), 0x22110000, 0xffffffff},
    {"e_snf_sal", 								RW, 			(EPON_MAC_BASE + 0x6298), 0x00000001, 0xffffffff},
    {"e_snf_etype", 							RW, 			(EPON_MAC_BASE + 0x629c), 0x000088b8, 0x0000ffff},
    {"e_rxuni_mcllid_cfg0", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62a0), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg1", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62a4), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg2", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62a8), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg3", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62ac), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg4", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62b0), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg5", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62b4), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg6", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62b8), 0x00000000, 0xffffffff},
    {"e_rxuni_mcllid_cfg7", 					RW|NO_DEF, 		(EPON_MAC_BASE + 0x62bc), 0x00000000, 0xffffffff},
    {"e_dfrpt_data5", 							RW, 			(EPON_MAC_BASE + 0x62d0), 0x0101f000, 0xffffffff},
    {"e_dfrpt_data6", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62d4), 0x00000000, 0xffffffff},
    {"e_dfrpt_data7", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62d8), 0x00000000, 0xffffffff},
    {"e_dfrpt_data8", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62dc), 0x00000000, 0xffffffff},
    {"e_dfrpt_data9", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62e0), 0x00000000, 0xffffffff},
    {"e_dfrpt_data10", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62e4), 0x00000000, 0xffffffff},
    {"e_dfrpt_data11", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62e8), 0x00000000, 0xffffffff},
    {"e_dfrpt_data12", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62ec), 0x00000000, 0xffffffff},
    {"e_dfrpt_data13", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62f0), 0x00000000, 0xffffffff},
    {"e_dfrpt_data14", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x62f4), 0x00000000, 0xffffffff},
    {"e_glue_cfg", 								RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x62f8), 0x00000000, 0x000087ff},
    {"e_gntreq_tmout", 							RW, 			(EPON_MAC_BASE + 0x62fc), 0x89502f90, 0xffffffff},
    {"e_glb_sts", 								RO|NO_DEF, 		(EPON_MAC_BASE + 0x6300), 0x00000000, 0x00000000},
    {"e_kpalv_hwensts", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6304), 0x00000000, 0x00000000},
    {"e_cnt_clr", 								RO|NO_DEF, 		(EPON_MAC_BASE + 0x6330), 0x00000000, 0x00000000},/*AO*/
    {"e_rxadv_cnt", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6334), 0x00000000, 0x00000000},
    {"e_tx_dygasp_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6338), 0x00000000, 0x00000000},
    {"e_tx_rpt_cnt", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x633c), 0x00000000, 0x00000000},
    {"e_txmpi_mpcp_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6340), 0x00000000, 0x00000000},
    {"e_txmpi_uc_cnt", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6344), 0x00000000, 0x00000000},
    {"e_rxmpi_bc_cnt", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x6348), 0x00000000, 0x00000000},
    {"e_rxmpi_mc_cnt", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x634c), 0x00000000, 0x00000000},    
    {"e_rxmpi_oam_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6350), 0x00000000, 0x00000000},
    {"e_rxmpi_mpcp_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6354), 0x00000000, 0x00000000},
    {"e_rxmpi_gate_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6358), 0x00000000, 0x00000000},
    {"e_rxmpi_nrlgate_cnt", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x635c), 0x00000000, 0x00000000},
    {"e_rxmpi_crc8err_cnt", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x6360), 0x00000000, 0x00000000},
    {"e_rxmpi_drop_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6364), 0x00000000, 0x00000000},
    {"e_rxmpi_churn_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6368), 0x00000000, 0x00000000},
    {"e_rxmpi_snf_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x636c), 0x00000000, 0x00000000},
    {"e_rxmbi_drop_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6370), 0x00000000, 0x00000000},
    {"e_rxmbi_drop_cnt2", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6374), 0x00000000, 0x00000000},
    {"e_rxmbi_bytecnt_h", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6378), 0x00000000, 0x00000000},
    {"e_rxmbi_bytecnt_l", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x637c), 0x00000000, 0x00000000},
    {"e_txmbi_uceth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6380), 0x00000000, 0x00000000},
    {"e_txmbi_mceth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6384), 0x00000000, 0x00000000},
    {"e_txmbi_err_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6388), 0x00000000, 0x00000000},
    {"e_tx_timedrift_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x638c), 0x00000000, 0x00000000},
    {"e_rxmpi_churn_cnt2", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6390), 0x00000000, 0x00000000},
    {"e_rxmpi_churn_cnt3", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6394), 0x00000000, 0x00000000},
    {"e_txmpi_uceth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x6398), 0x00000000, 0x00000000},
    {"e_txmpi_mceth_cnt", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x639c), 0x00000000, 0x00000000},
    {"e_llid8_9_gnt_stat", 						RO|NO_DEF, 		(EPON_MAC_BASE + 0x63a0), 0x00000000, 0x00000000},
    {"e_llid10_11_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63a4), 0x00000000, 0x00000000},
    {"e_llid12_13_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63a8), 0x00000000, 0x00000000},
    {"e_llid14_15_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63ac), 0x00000000, 0x00000000},
    {"e_llid16_17_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63b0), 0x00000000, 0x00000000},
    {"e_llid18_19_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63b4), 0x00000000, 0x00000000},
    {"e_llid20_21_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63b8), 0x00000000, 0x00000000},
    {"e_llid22_23_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63bc), 0x00000000, 0x00000000},
    {"e_llid24_25_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63c0), 0x00000000, 0x00000000},
    {"e_llid26_27_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63c4), 0x00000000, 0x00000000},
    {"e_llid28_29_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63c8), 0x00000000, 0x00000000},
    {"e_llid30_31_gnt_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63cc), 0x00000000, 0x00000000},
    {"e_rxfifo_depth_stat", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x63d0), 0x00000000, 0x00000000},
    {"e_rdmdly_stat", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x63dc), 0x00000000, 0x00000000},
    {"e_kpaom_stat", 							RO|NO_DEF, 		(EPON_MAC_BASE + 0x63e0), 0x00000000, 0x00000000},
    {"e_int_sts3", 								W1C|NO_DEF, 	(EPON_MAC_BASE + 0x6404), 0x00000000, 0x00000000},
    {"e_int_en3", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x6408), 0x00000000, 0x00ffffff},
    {"e_rpt_mpcp_timout2", 						W1C|NO_DEF, 	(EPON_MAC_BASE + 0x640c), 0x00000000, 0x00000000},
    {"e_llid8_11_cfg", 							RW|NO_DEF, 		(EPON_MAC_BASE + 0x6410), 0x00000000, 0xffffffff},
    {"e_llid12_15_cfg", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6414), 0x00000000, 0xffffffff},
    {"e_llid16_19_cfg", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6418), 0x00000000, 0xffffffff},
    {"e_llid20_23_cfg",							RW|NO_DEF, 		(EPON_MAC_BASE + 0x641c), 0x00000000, 0xffffffff},
    {"e_llid24_27_cfg", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6420), 0x00000000, 0xffffffff},
    {"e_llid28_31_cfg", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6424), 0x00000000, 0xffffffff},
    {"e_llid8_dscvry_sts", 						RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6428), 0x00000000, 0xc0000000},
    {"e_llid9_dscvry_sts", 						RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x642c), 0x00000000, 0xc0000000},
    {"e_llid10_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6430), 0x00000000, 0xc0000000},
    {"e_llid11_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6434), 0x00000000, 0xc0000000},
    {"e_llid12_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6438), 0x00000000, 0xc0000000},
    {"e_llid13_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x643c), 0x00000000, 0xc0000000},
    {"e_llid14_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6440), 0x00000000, 0xc0000000},
    {"e_llid15_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6444), 0x00000000, 0xc0000000},
    {"e_llid16_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6448), 0x00000000, 0xc0000000},
    {"e_llid17_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x644c), 0x00000000, 0xc0000000},
    {"e_llid18_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6450), 0x00000000, 0xc0000000},
    {"e_llid19_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6454), 0x00000000, 0xc0000000},
    {"e_llid20_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6458), 0x00000000, 0xc0000000},
    {"e_llid21_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x645c), 0x00000000, 0xc0000000},
    {"e_llid22_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6460), 0x00000000, 0xc0000000},
    {"e_llid23_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6464), 0x00000000, 0xc0000000},
    {"e_llid24_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6468), 0x00000000, 0xc0000000},
    {"e_llid25_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x646c), 0x00000000, 0xc0000000},
    {"e_llid26_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6470), 0x00000000, 0xc0000000},
    {"e_llid27_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6474), 0x00000000, 0xc0000000},
    {"e_llid28_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6478), 0x00000000, 0xc0000000},
    {"e_llid29_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x647c), 0x00000000, 0xc0000000},
    {"e_llid30_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6480), 0x00000000, 0xc0000000},
    {"e_llid31_dscvry_sts", 					RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6484), 0x00000000, 0xc0000000},
    {"e_rpt_cfg2", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x6488), 0x00000000, 0x0000ffff},
    {"e_rpt_cfg3", 								RW|NO_DEF, 		(EPON_MAC_BASE + 0x648c), 0x00000000, 0xffffffff},
    {"e_rpt_qthld_cfg2", 						RW|NO_DEF, 		(EPON_MAC_BASE + 0x6490), 0x00000000, 0x00000003},
    {"e_rpt_bitmap_cfg", 						RW|RO|NO_DEF, 	(EPON_MAC_BASE + 0x6494), 0x00000000, 0x8000001f},
    {"e_rpt_bitmap_val", 						RO, 			(EPON_MAC_BASE + 0x6498), 0x0000ffff, 0x00000000},
    {"e_txmbi_bufwrap_wrcnt", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x64c0), 0x00000000, 0x00000000},
    {"e_txmbi_bufwrap_rdcnt", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x64c4), 0x00000000, 0x00000000},
    {"e_txmbi_bufwrap_rdcnt2", 					RO|NO_DEF, 		(EPON_MAC_BASE + 0x64c8), 0x00000000, 0x00000000},
	{NULL, 0x0, 0x0, 0x0, 0x0}
};

int en7580_epon_get_debug_statistic_count(const void *in, void *out)
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
    tmpReg->rxMpiCrc8ErrCnt = rxmpi_crc_err_cnt.Bits.rxmpi_crc8err_cnt;

    /*Reg: rxmpi_drop_cnt*/
    tmpReg->rxMpiEofDropCnt = READ_REG_WORD(e_rxmpi_drop_cnt);

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

	return 0;
}


int en7580_epon_get_register_state(const void *in, void *out)
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

	return 0;
}

int en7580_epon_set_sniffer(const void *in, void *out)
{
    EPON_SNIFFER_MODE_T sniffer_mode = 0;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	sniffer_mode = *(EPON_SNIFFER_MODE_T *)in;
	switch(sniffer_mode)
	{
		case EPON_SNIFFER_ENABLE_SELF_ALL:
			e_glb_cfg_SET_sniffer_mode(e_glb_cfg, 1);
	        e_glb_cfg_SET_mpcp_fwd(e_glb_cfg, 1);
	        e_glb_cfg_SET_rx_nml_gate_fwd(e_glb_cfg, 1);
			printk("Epon EcoNet MPCP sniffer enable.\n");
			break;
		case EPON_SNIFFER_DISABLE_ALL:
			e_glb_cfg_SET_sniffer_mode(e_glb_cfg, 0);
	        e_glb_cfg_SET_mpcp_fwd(e_glb_cfg, 0);
	        e_glb_cfg_SET_rx_nml_gate_fwd(e_glb_cfg, 0);
	        e_glb_cfg2_SET_us_snf(e_glb_cfg2, 0);
	        e_glb_cfg2_SET_all_ucllid_snf(e_glb_cfg2, 0);
	        e_glb_cfg2_SET_all_ucllid_mpcpsnf(e_glb_cfg2, 0);
	        e_glb_cfg2_SET_all_ucllid_ethsnf(e_glb_cfg2, 0);
			printk("Epon sniffer disable.\n");
			break;
		case EPON_SNIFFER_ENABLE_OTHER_MPCP:
			e_glb_cfg_SET_sniffer_mode(e_glb_cfg, 1);
			e_glb_cfg_SET_mpcp_fwd(e_glb_cfg, 1);
            e_glb_cfg2_SET_all_ucllid_snf(e_glb_cfg2, 1);
            e_glb_cfg2_SET_all_ucllid_mpcpsnf(e_glb_cfg2, 1);
			printk("Epon Other MPCP sniffer enable.\n");
			break;
		case EPON_SNIFFER_ENABLE_OTHER_ETHERNET:
			e_glb_cfg_SET_sniffer_mode(e_glb_cfg, 1);
			e_glb_cfg2_SET_all_ucllid_snf(e_glb_cfg2, 1);
            e_glb_cfg2_SET_all_ucllid_ethsnf(e_glb_cfg2, 1);
			printk("Epon Other Ether sniffer enable.\n");
			break;
		default:
			DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: ERROR Sniffer Mode", __FUNCTION__);
			return -1;
	};

	if(sniffer_mode == EPON_SNIFFER_DISABLE_ALL)
	{
		del_fix_reg_list((uint32_t)e_glb_cfg);
		del_fix_reg_list((uint32_t)e_glb_cfg2);
	}
	else
	{
		add_fix_reg_list((uint32_t)e_glb_cfg, READ_REG_WORD(e_glb_cfg));
        add_fix_reg_list((uint32_t)e_glb_cfg2, READ_REG_WORD(e_glb_cfg2));  	
	}
	return 0;
}

int en7580_epon_set_static_report(const void *in, void *out)
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
		e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 1);
		add_fix_reg_list((uint32_t)e_glb_cfg, IO_GREG(e_glb_cfg));
        raw = IO_GREG(e_dfrpt_data5);
		raw = raw | (static_rpt->report_val&0xFFFF);
        IO_SREG(e_dfrpt_data5, raw);
        add_fix_reg_list((uint32_t)e_dfrpt_data5, raw);
        printk("queueSet[31:24]bitmap[23:16]q0buf[15:0] 0x%x\n", raw);
	}
	else
	{
		gp_epon_global_data->static_report_enable = 0;
		e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 0);
		del_fix_reg_list((uint32_t)e_glb_cfg);
		del_fix_reg_list((uint32_t)e_dfrpt_data5);
		printk("Static Report disable.\n");
	}

	return 0;
}


/*************************************************************************************************************/
/***********************************************only defined, but not used************************************/
/*************************************************************************************************************/

int en7580_epon_set_mac_report_qsizeadjs(const void *in, void *out)
{
    uint Raw = 0;
    uint index = 0;
    if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)  \
        || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)) {
        Raw = 0xf1;  /*for 1G upstream  adj */      
        Raw |= (Raw << 16);
        for (index = 0; index < 4; index++) 
            WRITE_REG_WORD(g_epon_u1g_rpt_qsizeadj[index] , Raw);  
    }else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON) {
        Raw = 0x19;/*for 10G upstream  adj*/
        Raw |= (Raw << 16);
        for (index = 0; index < 4; index++)
            WRITE_REG_WORD(g_epon_u10g_rpt_qsizeadj[index] , Raw);
    }else
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: %s WRONG MODE",__FUNCTION__);

	return EPON_SUCCESS;        
}

static int epon_check_report_bitmap_rwcmd_done(void)
{
	int RETRY = 10 ;
    REG_e_rpt_bitmap_cfg bitmapCfg;

	memset(&bitmapCfg, 0 , sizeof(REG_e_rpt_bitmap_cfg));
	
    while(RETRY--) 
	{
		bitmapCfg.Raw = READ_REG_WORD(e_rpt_bitmap_cfg) ;
		if(bitmapCfg.Bits.rpt_bmap_rwcmd_done) 
		{
            return EPON_SUCCESS;
		} 
		udelay(1) ;
	}

    return EPON_NORMAL_ERROR;
}

int en7580_epon_set_llid_report_bitmap(const void *in, void *out)
{
	REG_e_rpt_bitmap_cfg bitmapCfg;
	REG_e_rpt_bitmap_val bitmapVal;
	epon_llid_report_bitmap_t *llid_bitmap = (epon_llid_report_bitmap_t *)in;

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}
    

	memset(&bitmapCfg, 0 , sizeof(REG_e_rpt_bitmap_cfg));
	memset(&bitmapVal, 0 , sizeof(REG_e_rpt_bitmap_val));

	if(llid_bitmap->llidIndex >= EPON_LLID_MAX_NUM){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"Error ! llidIndex is %d, max llidIndex is %d\n",llid_bitmap->llidIndex,(EPON_LLID_MAX_NUM - 1));
		return EPON_NORMAL_ERROR;
	}
	
	bitmapCfg.Raw = READ_REG_WORD(e_rpt_bitmap_cfg);
	bitmapCfg.Bits.rpt_bmap_rwcmd = 0;
	bitmapCfg.Bits.rpt_bmap_llid_idx = llid_bitmap->llidIndex;
	WRITE_REG_WORD(e_rpt_bitmap_cfg, bitmapCfg.Raw);
	bitmapVal.Raw = READ_REG_WORD(e_rpt_bitmap_val);
	if(epon_check_report_bitmap_rwcmd_done() != 0){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_check_report_bitmap_rwcmd_done read fail\n");
		return EPON_NORMAL_ERROR;
	}

	bitmapCfg.Bits.rpt_bmap_rwcmd = 1;
	bitmapCfg.Bits.rpt_bmap_llid_idx = llid_bitmap->llidIndex;

	bitmapVal.Bits.rpt_bitmap_ctrl = 0xff;
	bitmapVal.Bits.rpt_bitmap_set = llid_bitmap->bitmap;
	
	WRITE_REG_WORD(e_rpt_bitmap_val, bitmapVal.Raw);
	/**Notice: config e_rpt_bitmap_val before e_rpt_bitmap_cfg**/
	WRITE_REG_WORD(e_rpt_bitmap_cfg, bitmapCfg.Raw);

	if(epon_check_report_bitmap_rwcmd_done() != 0){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon_check_report_bitmap_rwcmd_done write fail\n");
		return EPON_NORMAL_ERROR;
	}
	
	return EPON_SUCCESS;
}

int en7580_epon_set_mac_report_qsizeadjs_for_fec(const void *in, void *out)
{
	uint8_t fecFlag = 0;
    uint32_t Raw = 0;
    uint32_t index = 0;
	uint32_t regAddr[4] = {(uint32_t)e_u1g_rpt_qsizeadj5,(uint32_t)e_u1g_rpt_qsizeadj6,(uint32_t)e_u1g_rpt_qsizeadj7,(uint32_t)e_u1g_rpt_qsizeadj8};

	if(in == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: INPUT Parameter == NULL", __FUNCTION__);
		return EPON_INPUT_POINT_NULL;
	}

	fecFlag = *(uint8_t *)in;

	if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)  \
        || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)) {
        if(fecFlag == ENABLE) {
            Raw = 0x101;  /*for 1G upstream  adj fec on*/        
            Raw |= (Raw << 16);
            for (index = 0; index < 4; index++) 
                WRITE_REG_WORD(regAddr[index] , Raw);  
        } else {
            Raw = 0xf1;    /*for 1G upstream  adj fec off*/     
            Raw |= (Raw << 16);
            for (index = 0; index < 4; index++) 
                WRITE_REG_WORD(regAddr[index] , Raw); 
        }
    }else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON) {
      DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "\r\nfor 10G upstream fec adj");
    }else
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\nwrong mode");

    return 0;  
}


