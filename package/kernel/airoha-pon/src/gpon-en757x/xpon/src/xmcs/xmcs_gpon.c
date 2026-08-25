/*
* File Name: xmcs_gpon.c
* Description: GPON I/O Command Process for xPON Management Control 
*			  Subsystem
*
******************************************************************/
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include "common/drv_global.h"
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_gpon.h"
#include "fe_reg.h"

/***************************************************************
***************************************************************/
static int xmcs_set_sn_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.sn, 0, GPON_SN_LENS) ;
	memcpy(gpGponPriv->gponCfg.sn, pSnPasswd->sn, GPON_SN_LENS) ;
	gponDevSetSerialNumber(pSnPasswd->sn) ;
	
	memset(gpGponPriv->gponCfg.passwd, 0, GPON_PASSWD_LENS) ;
	memcpy(gpGponPriv->gponCfg.passwd, pSnPasswd->passwd,GPON_PASSWD_LENS) ;
    gpGponPriv->gponCfg.PasswdLength=pSnPasswd->PasswdLength;
    gpGponPriv->gponCfg.hexFlag=pSnPasswd->hexFlag;
#if !(defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT))
    gpGponPriv->emergencystate = pSnPasswd->EmergencyState;
#endif

	PON_MSG(MSG_TRACE, "SN:%s, PASSWD:%s\n", pSnPasswd->sn, pSnPasswd->passwd) ;
	
	return 0 ;
}
		
static int xmcs_set_sn(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.sn, 0, GPON_SN_LENS) ;
	memcpy(gpGponPriv->gponCfg.sn, pSnPasswd->sn, GPON_SN_LENS) ;
	gponDevSetSerialNumber(pSnPasswd->sn) ;
	
	return 0 ;
}

static int xmcs_set_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd) 
{
	memset(gpGponPriv->gponCfg.passwd, 0, GPON_PASSWD_LENS) ;
	strncpy(gpGponPriv->gponCfg.passwd, pSnPasswd->passwd, GPON_PASSWD_LENS) ;
	
	return 0 ;
}

static int xmcs_set_emergency_state(XPON_Mode_t mode) 
{
	gpGponPriv->emergencystate = mode;
	if(XPON_ENABLE == mode) 
	{
		gpon_act_change_gpon_state(GPON_STATE_O7);
		gpon_record_emergence_info(GPON_OMCI_SET_EMERGNCE_STATE);
	}
	return 0 ;
}
/***************************************************************
***************************************************************/

/***************************************************************
***************************************************************/
static int xmcs_set_act_timer(struct XMCS_GponActTimer_S *pActTimer) 
{
    struct XMCS_GponActTimer_S actTimerTmp;

    if(copy_from_user(&actTimerTmp, pActTimer, sizeof(struct XMCS_GponActTimer_S))){
        printk("[%s %d]ERR: copy_from_user\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }
    
	gpGponPriv->gponCfg.to1Timer = actTimerTmp.to1Timer ;
	gpGponPriv->to1_timer.data = gpGponPriv->gponCfg.to1Timer ;

	gpGponPriv->gponCfg.to2Timer = actTimerTmp.to2Timer ;
	gpGponPriv->to2_timer.data = gpGponPriv->gponCfg.to2Timer ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_act_timer_to1(uint to1Timer) 
{
	gpGponPriv->gponCfg.to1Timer = to1Timer ;
	gpGponPriv->to1_timer.data = gpGponPriv->gponCfg.to1Timer ;

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_act_timer_to2(uint to2Timer) 
{
	gpGponPriv->gponCfg.to2Timer = to2Timer ;
	gpGponPriv->to2_timer.data = gpGponPriv->gponCfg.to2Timer ;

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_csr(void) 
{
	return gponDevDumpCsr();
}
/***************************************************************
***************************************************************/
static int xmcs_get_gem_table_info(void) 
{
	return gponDevDumpGemInfo();
}

/***************************************************************
***************************************************************/
static int xmcs_get_tcont_table_info(void) 
{
	return gponDevDumpTcontInfo();
}

/***************************************************************
***************************************************************/
static int xmcs_set_idle_gem(ushort idle_gem_thld) 
{
	gpGponPriv->gponCfg.idle_gem_thld = idle_gem_thld;
	return gponDevSetIdleGemThreshold(idle_gem_thld);
}

/***************************************************************
***************************************************************/
static int xmcs_get_idle_gem(ushort *idle_gem_thld) 
{
	*idle_gem_thld = gpGponPriv->gponCfg.idle_gem_thld;
	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_onu_info(struct XMCS_GponOnuInfo_S *pOnuInfo) 
{
	memset(pOnuInfo, 0, sizeof(struct XMCS_GponOnuInfo_S)) ;
	
	pOnuInfo->onuId = GPON_ONU_ID ;
	pOnuInfo->state = GPON_CURR_STATE ;
	memcpy(&pOnuInfo->sn, gpGponPriv->gponCfg.sn, GPON_SN_LENS) ;
	memcpy(&pOnuInfo->passwd, gpGponPriv->gponCfg.passwd, GPON_PASSWD_LENS) ;
    pOnuInfo->PasswdLength = gpGponPriv->gponCfg.PasswdLength;
    pOnuInfo->hexFlag = gpGponPriv->gponCfg.hexFlag;
	pOnuInfo->keyIdx = gpGponPriv->gponCfg.keyIdx ;
	memcpy(&pOnuInfo->key, gpGponPriv->gponCfg.key, GPON_ENCRYPT_KEY_LENS) ;
	pOnuInfo->actTo1Timer = gpGponPriv->gponCfg.to1Timer ;
	pOnuInfo->actTo2Timer = gpGponPriv->gponCfg.to2Timer ;
	pOnuInfo->omcc = GPON_OMCC_ID ;
	pOnuInfo->EmergencyState = gpGponPriv->emergencystate ;

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_gpon_block_size(ushort blockSize)
{
	gponDevSetDBABlockSize(blockSize) ;
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_gpon_block_size(ushort *blockSize)
{
	gponDevGetDBABlockSize(blockSize) ;
	return 0 ;
}

/***************************************************************
struct XMCS_GponTrtcmConfig_S {
	XPON_Mode_t						trtcmMode ;
	XMCSGPON_TrtcmScale_t			trtcmScale ;
} ;
***************************************************************/
static int xmcs_set_gpon_trtcm_mode(struct XMCS_GponTrtcmConfig_S *pTrtcmConfig)
{
	QDMA_Mode_t mode ;
	
	mode = (pTrtcmConfig->trtcmMode==XPON_ENABLE) ? QDMA_ENABLE : QDMA_DISABLE ;	
#ifdef TCSUPPORT_CPU_EN7521
	//todo: not implement yet
#else
	qdma_set_gpon_trtcm_mode(mode) ;
#endif
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_GPON_TRTCM_SCALE_1B = 0,
	XMCS_GPON_TRTCM_SCALE_2B,
	XMCS_GPON_TRTCM_SCALE_4B, 
	XMCS_GPON_TRTCM_SCALE_8B,
	XMCS_GPON_TRTCM_SCALE_16B,
	XMCS_GPON_TRTCM_SCALE_32B,
	XMCS_GPON_TRTCM_SCALE_64B,
	XMCS_GPON_TRTCM_SCALE_128B,
	XMCS_GPON_TRTCM_SCALE_256B,
	XMCS_GPON_TRTCM_SCALE_512B,
	XMCS_GPON_TRTCM_SCALE_1K,
	XMCS_GPON_TRTCM_SCALE_2K,
	XMCS_GPON_TRTCM_SCALE_4K,
	XMCS_GPON_TRTCM_SCALE_8K,
	XMCS_GPON_TRTCM_SCALE_16K,
	XMCS_GPON_TRTCM_SCALE_32K,
	XMCS_GPON_TRTCM_SCALE_ITEMS
} XMCSGPON_TrtcmScale_t ;
struct XMCS_GponTrtcmConfig_S {
	XPON_Mode_t						trtcmMode ;
	XMCSGPON_TrtcmScale_t			trtcmScale ;
} ;
***************************************************************/
static int xmcs_set_gpon_trtcm_scale(struct XMCS_GponTrtcmConfig_S *pTrtcmConfig)
{
	QDMA_TrtcmScale_t gponTrtcmScale[XMCS_GPON_TRTCM_SCALE_ITEMS] = { QDMA_TRTCM_SCALE_1BYTE,
																	  QDMA_TRTCM_SCALE_2BYTE,
																	  QDMA_TRTCM_SCALE_4BYTE, 
																	  QDMA_TRTCM_SCALE_8BYTE,
																	  QDMA_TRTCM_SCALE_16BYTE,
																	  QDMA_TRTCM_SCALE_32BYTE,
																	  QDMA_TRTCM_SCALE_64BYTE,
																	  QDMA_TRTCM_SCALE_128BYTE,
																	  QDMA_TRTCM_SCALE_256BYTE,
																	  QDMA_TRTCM_SCALE_512BYTE,
																	  QDMA_TRTCM_SCALE_1KBYTE,
																	  QDMA_TRTCM_SCALE_2KBYTE,
																	  QDMA_TRTCM_SCALE_4KBYTE,
																	  QDMA_TRTCM_SCALE_8KBYTE,
																	  QDMA_TRTCM_SCALE_16KBYTE,
																	  QDMA_TRTCM_SCALE_32KBYTE } ;

	if(pTrtcmConfig->trtcmScale<0 || pTrtcmConfig->trtcmScale>=XMCS_GPON_TRTCM_SCALE_ITEMS) {
		return -EINVAL ;
	}
#ifndef TCSUPPORT_CPU_EN7521
	qdma_set_gpon_trtcm_scale(gponTrtcmScale[pTrtcmConfig->trtcmScale]) ;
#endif
	return 0 ;
}

/***************************************************************
typedef enum {
	XMCS_GPON_TRTCM_SCALE_1B = 0,
	XMCS_GPON_TRTCM_SCALE_2B,
	XMCS_GPON_TRTCM_SCALE_4B, 
	XMCS_GPON_TRTCM_SCALE_8B,
	XMCS_GPON_TRTCM_SCALE_16B,
	XMCS_GPON_TRTCM_SCALE_32B,
	XMCS_GPON_TRTCM_SCALE_64B,
	XMCS_GPON_TRTCM_SCALE_128B,
	XMCS_GPON_TRTCM_SCALE_256B,
	XMCS_GPON_TRTCM_SCALE_512B,
	XMCS_GPON_TRTCM_SCALE_1K,
	XMCS_GPON_TRTCM_SCALE_2K,
	XMCS_GPON_TRTCM_SCALE_4K,
	XMCS_GPON_TRTCM_SCALE_8K,
	XMCS_GPON_TRTCM_SCALE_16K,
	XMCS_GPON_TRTCM_SCALE_32K,
	XMCS_GPON_TRTCM_SCALE_ITEMS
} XMCSGPON_TrtcmScale_t ;
struct XMCS_GponTrtcmConfig_S {
	XPON_Mode_t						trtcmMode ;
	XMCSGPON_TrtcmScale_t			trtcmScale ;
} ;
***************************************************************/
static int xmcs_get_gpon_trtcm_config(struct XMCS_GponTrtcmConfig_S *pTrtcmConfig)
{
	return 0 ;
}

/***************************************************************
struct XMCS_GponTrtcmParams_S {
	unchar					channel ;
	ushort					cirValue ;
	ushort					cbsUnit ;
	ushort					pirValue ;
	ushort					pbsUnit ;
} ;
***************************************************************/
static int xmcs_set_gpon_trtcm_params(struct XMCS_GponTrtcmParams_S *pTrtcmParams)
{
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_TxDbaReport_T gponTrtcmParams ;
#else
	QDMA_TcontTrtcm_T gponTrtcmParams ;
#endif
	
	if(pTrtcmParams->channel<0 || pTrtcmParams->channel>=XPON_CHANNEL_NUMBER) {
		return -EINVAL ;
	}

	gponTrtcmParams.channel = pTrtcmParams->channel ;
	gponTrtcmParams.cirParamValue = pTrtcmParams->cirValue ;
	gponTrtcmParams.cbsParamValue = pTrtcmParams->cbsUnit ;
	gponTrtcmParams.pirParamValue = pTrtcmParams->pirValue ;
	gponTrtcmParams.pbsParamValue = pTrtcmParams->pbsUnit ;
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_SET_TX_DBA_REPORT(ECNT_QDMA_WAN, &gponTrtcmParams);
#else
	return qdma_set_gpon_trtcm_params(&gponTrtcmParams) ;
#endif
}

/***************************************************************
struct XMCS_GponTrtcmParams_S {
	unchar					channel ;
	ushort					cirValue ;
	ushort					cbsUnit ;
	ushort					pirValue ;
	ushort					pbsUnit ;
} ;
***************************************************************/
static int xmcs_get_gpon_trtcm_params(struct XMCS_GponTrtcmParams_S *pTrtcmParams)
{
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_TxDbaReport_T gponTrtcmParams ;
#else
	QDMA_TcontTrtcm_T gponTrtcmParams ;
#endif
	int ret = 0 ;
	
	if(pTrtcmParams->channel<0 || pTrtcmParams->channel>=XPON_CHANNEL_NUMBER) {
		return -EINVAL ;
	}

	gponTrtcmParams.channel = pTrtcmParams->channel ;
#ifndef TCSUPPORT_CPU_EN7521
	ret = qdma_get_gpon_trtcm_params(&gponTrtcmParams) ;
	if(ret != 0) {
		return ret ;
	}
#endif	
	pTrtcmParams->cirValue = gponTrtcmParams.cirParamValue ;
	pTrtcmParams->cbsUnit = gponTrtcmParams.cbsParamValue ;
	pTrtcmParams->pirValue = gponTrtcmParams.pirParamValue ;
	pTrtcmParams->pbsUnit = gponTrtcmParams.pbsParamValue ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_gpon_tod(struct XMCS_GponTodCfg_S *pGponTod) 
{
	gponDevSetNewTod(pGponTod->superframe, pGponTod->sec, pGponTod->nanosec) ;
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_gpon_tod(struct XMCS_GponTodCfg_S *pGponTod) 
{
	gponDevGetCurrentTod(&pGponTod->sec, &pGponTod->nanosec) ;

	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_gpon_new_tod(struct XMCS_GponTodCfg_S *pGponNewTod)
{
	gponDevGetNewTod(&pGponNewTod->sec, &pGponNewTod->nanosec) ;

	return 0 ;
}

/***************************************************************
***************************************************************/
int get_counter_from_reg(struct XMCS_GponGemCounter_S *pGemCouter)
{
    ushort gemIdx = (gpWanPriv->gpon.gemIdToIndex[pGemCouter->gemPortId] & GPON_GEM_IDX_MASK);
    GWAN_GemInfo_T * gemInfo = & gpWanPriv->gpon.gemPort[gemIdx].info;
    
    if(!gemInfo->valid)
        return -1;

    ushort portid = gemInfo->portId;

	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_FRAME_CNT,   &pGemCouter->rxGemFrameH,   &pGemCouter->rxGemFrameL) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_RX_PL_BYTE_CNT, &pGemCouter->rxGemPayloadH, &pGemCouter->rxGemPayloadL) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_FRAME_CNT,   &pGemCouter->txGemFrameH,   &pGemCouter->txGemFrameL) )
        return -1;
    
	if( 0!= gponDevGetGemPortCounter(portid, GEMPORT_TX_PL_BYTE_CNT, &pGemCouter->txGemPayloadH, &pGemCouter->txGemPayloadL) )
        return -1;

  	return 0;
}
/***************************************************************
***************************************************************/
static int xmcs_get_tod_switch_time(struct XMCS_GponTodCfg_S *pgponTodCfg)
{
	gponDevGetTodSwitchTime(&pgponTodCfg->superframe) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
int xmcs_get_gem_counter(struct XMCS_GponGemCounter_S *pGponGemPortCount) 
{
	uint i;
	int ret = 0;
	struct XMCS_GponGemCounter_S temp;
	memset(&temp, 0, sizeof(struct XMCS_GponGemCounter_S));
	
	ret= copy_from_user(&temp, pGponGemPortCount, sizeof(struct XMCS_GponGemCounter_S));
	if(0!=ret)
	{
		return -EINVAL ;
	}  
	
	ret = get_counter_from_reg(&temp);
    if(0!=ret)
    {
		return -EINVAL ;
    }
    
	ret = copy_to_user(pGponGemPortCount, &temp, sizeof(struct XMCS_GponGemCounter_S));
	if(0!=ret)
	{
		return -EINVAL ;
	}
	return ret ;
}

EXPORT_SYMBOL(xmcs_get_gem_counter) ;

/***************************************************************
***************************************************************/
static int xmcs_set_eqd_offset_flag(char flag) 
{
	gpGponPriv->gponCfg.flags.eqdOffsetFlag = (flag == '-') ? GPON_EQD_OFFSET_FLAG_SUBTRACT : GPON_EQD_OFFSET_FLAG_ADD ;
	
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
	gpon_dvt_eqd_adjustment(gpGponPriv->gponCfg.eqd) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_eqd_offset(struct XMCS_EqdOffset_S *pGponEqdOffset) 
{
	pGponEqdOffset->O4 = gpGponPriv->gponCfg.eqdO4Offset;
	pGponEqdOffset->O5 = gpGponPriv->gponCfg.eqdO5Offset;
	pGponEqdOffset->eqdOffsetFlag = gpGponPriv->gponCfg.flags.eqdOffsetFlag;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_counter_type(GPON_COUNTER_TYPE_t type) 
{
	gpGponPriv->gponCfg.counter_type = type;
	gponDevSetCounterType(type);
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_counter_type(GPON_COUNTER_TYPE_t *type) 
{
	gponDevGetCounterType(type);
	
	return 0 ;
}


/***************************************************************
***************************************************************/
static int xmcs_set_response_time(ushort time) 
{
	gpGponPriv->gponCfg.onuResponseTime = time;
	gponDevSetResponseTime(time) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_response_time(ushort *time) 
{
	gponDevGetResponseTime(time) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_internal_delay_fine_tune(unchar delay) 
{
	gpGponPriv->gponCfg.internalDelayFineTune = delay;
	gponDevSetInternalDelayFineTune(delay) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_internal_delay_fine_tune(unchar *delay) 
{
	gponDevGetInternalDelayFineTune(delay) ;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_burst_mode_overhead(GPON_BURST_MODE_OVERHEAD_LEN_T type) 
{
	gpGponPriv->gponCfg.flags.preambleFlag = type;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_get_burst_mode_overhead(GPON_BURST_MODE_OVERHEAD_LEN_T *type) 
{
	*type = gpGponPriv->gponCfg.flags.preambleFlag;
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_clear_counter(GPON_SW_HW_SELECT_T clear) 
{
	switch(clear) {
		case GPON_SW:
			gponDevClearSwCounter();
			break;
		case GPON_HW:
			gponDevClearHwCounter();
			break;
		case GPON_SW_HW:
			gponDevClearSwCounter();
			gponDevClearHwCounter();
			break;
		default:
			return -1;
	}
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_set_key_switch_time(uint spf) 
{
	return gponDevSetKeySwithTime(spf);
}

/***************************************************************
***************************************************************/
static int xmcs_set_encrypt_key(unchar *key) 
{
	return gponDevSetEncryptKey(key);
}

/***************************************************************
***************************************************************/
static int xmcs_get_encrypt_key_info(GPON_DEV_ENCRYPT_KEY_INFO_T *keyInfo) 
{
	return gponDevGetEncryptKey(keyInfo);
}

static int xmcs_set_dying_gasp_mode(GPON_SW_HW_SELECT_T mode) 
{
	gpPonSysData->dyingGaspData.isGponHwFlag = mode;
	return gponDevHardwareDyingGasp(mode);
}

/***************************************************************
***************************************************************/
static int xmcs_get_dying_gasp_mode(GPON_SW_HW_SELECT_T *mode) 
{
	return gponDevGetDyingGaspMode(mode);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dying_gasp_num(uint num) 
{
	return gponDevSetDyingGaspNum(num);
}

/***************************************************************
***************************************************************/
static int xmcs_get_dying_gasp_num(uint *num) 
{
	return gponDevGetDyingGaspNum(num);
}

/***************************************************************
***************************************************************/
static int xmcs_set_up_traffic(GPON_DEV_UP_TRAFFIC_T *up_traffic) 
{
	if((up_traffic->omci != XPON_DISABLE) && (up_traffic->omci != XPON_ENABLE)) {
		return -1;
	}

	if((up_traffic->data != XPON_DISABLE) && (up_traffic->data != XPON_ENABLE)) {
		return -1;
	}
		
	gpPonSysData->isUpOmciOamTraffic = up_traffic->omci;
	gpPonSysData->isUpDataTraffic = up_traffic->data;

	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_up_traffic(GPON_DEV_UP_TRAFFIC_T *up_traffic) 
{
	up_traffic->omci = gpPonSysData->isUpOmciOamTraffic;
	up_traffic->data = gpPonSysData->isUpDataTraffic;

	return 0;
}

/***************************************************************
***************************************************************/
static int xmcs_get_int_mask(uint *mask) 
{
	return gponDevGetIntMask(mask);
}

#ifdef TCSUPPORT_CPU_EN7521
/*********************************************************************************************************************
Description:
	reset alloc id, gemport id
Input Args:

Ret Value:
	if success, return 0
	if fali, return non-zero error code
*********************************************************************************************************************/
static int xmcs_gpon_reset_service(void)
{
	struct Gpon_Recovery_S * pReset = gpGponPriv->pGponRecovery;
	
	while(pReset->gemPortNum)
	{
		if(0 != gwan_remove_gemport(pReset->gemPort[pReset->gemPortNum-1].gemPortId))
		{
			if(gponRecovery.dbgPrint){
				printk("%s %d line remove gem err\n", __FUNCTION__,__LINE__);
			}
			pReset->gemPortNum--;
			continue;
		}
	}
	
	while(pReset->allocIdNum)
	{	
		if(0 != gwan_remove_tcont(pReset->allocId[pReset->allocIdNum-1]))
		{
			if(gponRecovery.dbgPrint){
				printk("%s %d line remove tcont err\n", __FUNCTION__,__LINE__);
			}
			pReset->allocIdNum--;
			continue;
		}
	}
	
	return XPON_SUCCESS;
}

/***************************************************************
***************************************************************/
static int xmcs_set_sniffer_mode(GPON_DEV_SNIFFER_MODE_T *sniffer) 
{
	memcpy(&gpGponPriv->gponCfg.sniffer_mode, sniffer, sizeof(GPON_DEV_SNIFFER_MODE_T));
	return gponDevSetSniffMode(sniffer);
}

/***************************************************************
***************************************************************/
static int xmcs_get_sniffer_mode(GPON_DEV_SNIFFER_MODE_T *sniffer) 
{
	return gponDevGetSniffMode(sniffer);
}

/***************************************************************
***************************************************************/
static int xmcs_set_ext_bst_len_ploamd_filter(XPON_Mode_t enable)
{
	return gponDevSetFilterExtBurstLengthPLOAM(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_get_ext_bst_len_ploamd_filter(XPON_Mode_t *enable)
{
	return gponDevGetFilterExtBurstLengthPLOAM(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_up_overhead_ploamd_filter(XPON_Mode_t enable)
{
	return gponDevSetFilterUpstreamOverheadPLOAM(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_get_up_overhead_ploamd_filter(XPON_Mode_t *enable)
{
	return gponDevGetFilterUpstreamOverheadPLOAM(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_tx_4bytes_align(XPON_Mode_t enable)
{
	return gponDevSetTx4bytesAlign(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_get_tx_4bytes_align(XPON_Mode_t *enable)
{
	return gponDevGetTx4bytesAlign(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_backdoor(XPON_Mode_t enable)
{
	return gponDevSetDbaBackdoor(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_backdoor_total(uint total_size)
{
	return gponDevSetDbaBackdoorTotal(total_size);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_backdoor_green(uint green_size)
{
	return gponDevSetDbaBackdoorGreen(green_size);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_backdoor_yellow(uint yellow_size)
{
	return gponDevSetDbaBackdoorYellow(yellow_size);
}

/***************************************************************
***************************************************************/
static int xmcs_get_dba_backdoor(GPON_DEV_DBA_BACKDOOR_T *dba_backdoor)
{
	return gponDevGetDbaBackdoor(dba_backdoor);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_slight_modify(XPON_Mode_t enable)
{
	return gponDevSetDbaSlightModify(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_slight_modify_total(ushort total_size)
{
	return gponDevSetDbaSlightModifyTotal(total_size);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_slight_modify_green(ushort green_size)
{
	return gponDevSetDbaSlightModifyGreen(green_size);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_slight_modify_yellow(ushort yellow_size)
{
	return gponDevSetDbaSlightModifyYellow(yellow_size);
}

/***************************************************************
***************************************************************/
static int xmcs_get_dba_slight_modify(GPON_DEV_SLIGHT_MODIFY_T *dba_slight_modify)
{
	return gponDevGetDbaSlightModify(dba_slight_modify);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_shift_modify(XPON_Mode_t enable)
{
	return gponDevSetDbaShiftModify(enable);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_shift_modify_total(struct XMCS_DBAShiftMod_S *gponShiftTotal)
{
	return gponDevSetDbaShiftModifyTotal(gponShiftTotal);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_shift_modify_green(struct XMCS_DBAShiftMod_S *gponShiftGreen)
{
	return gponDevSetDbaShiftModifyGreen(gponShiftGreen);
}

/***************************************************************
***************************************************************/
static int xmcs_set_dba_shift_modify_yellow(struct XMCS_DBAShiftMod_S *gponShiftYellow)
{
	return gponDevSetDbaShiftModifyYellow(gponShiftYellow);
}

/***************************************************************
***************************************************************/
static int xmcs_set_o3_o4_ploam_ctrl(GPON_SW_HW_SELECT_T sel)
{
	return gponDevSetO3O4PloamCtrl(sel);
}

/***************************************************************
***************************************************************/
static int xmcs_get_o3_o4_ploam_ctrl(GPON_SW_HW_SELECT_T *sel)
{
	return gponDevGetO3O4PloamCtrl(sel);
}

/***************************************************************
***************************************************************/
static int xmcs_get_tx_sync_offset(unchar *tx_sync)
{
	return gponDevGetTxSyncOffset(tx_sync);
}

/***************************************************************
***************************************************************/
static int xmcs_set_aes_key_switch_by_sw(void)
{
	return gpon_dvt_aes_key_switch_by_sw();
}

/***************************************************************
***************************************************************/
static int xmcs_set_sw_reset(void)
{
	return gpon_dvt_sw_reset();
}

/***************************************************************
***************************************************************/
static int xmcs_set_sw_resync(void)
{
	return gpon_dvt_sw_resync();
}
#endif

#ifdef TCSUPPORT_CPU_EN7521
/***************************************************************
***************************************************************/
static int xmcs_set_1pps_h_w(uint width)
{
	return gponDevSet1ppsHighWidth(width);
}

/***************************************************************
***************************************************************/
static int xmcs_get_1pps_h_w(uint *width)
{
	return gponDevGet1ppsHighWidth(width);
}

/***************************************************************
***************************************************************/
static int xmcs_set_send_ploamu_wait_mode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T mode)
{
	return gponDevSetSendPloamuWaitMode(mode);
}

/***************************************************************
***************************************************************/
static int xmcs_get_send_ploamu_wait_mode(GPON_DEV_SEND_PLOAMU_WAIT_MODE_T *mode)
{
	return gponDevGetSendPloamuWaitMode(mode);
}
#endif

/***************************************************************
***************************************************************/
static int xmcs_test_send_ploam_sleep_request(GPON_PLOAMu_SLEEP_MODE_t mode) 
{
	return ploam_send_sleep_request_msg(mode);
}

/***************************************************************
***************************************************************/
static int xmcs_test_send_ploam_rei(uint counter) 
{
	return ploam_send_rei_msg(counter, &gpGponPriv->gponCfg.reiSeq);
}

/***************************************************************
***************************************************************/
static int xmcs_test_send_ploam_dying_gasp(void) 
{
	REG_DBG_US_DYING_GASP_CTRL us_dying_gasp_ctrl;
	if(gpPonSysData->dyingGaspData.isGponHwFlag == GPON_HW) {
	us_dying_gasp_ctrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
	us_dying_gasp_ctrl.Bits.dying_gasp_test = 1;
	IO_SREG(DBG_US_DYING_GASP_CTRL, us_dying_gasp_ctrl.Raw) ;
	} else {
		ploam_send_dying_gasp() ;
	}
	
	return 0 ;
}

/***************************************************************
***************************************************************/
static int xmcs_test_tod(uint newTime) 
{
	return gpon_dvt_tod(newTime);
}

/***************************************************************
***************************************************************/
/*GponGetCounter_table[] mapping the value of register */
static int xmcs_get_rx_ethernet_frame_counter (struct XMCS_GponGetCounter_S *gponGetCounter)
{
	gponGetCounter->gponGetCounter_table[0] = 0;                                      
	gponGetCounter->gponGetCounter_table[1] = 2;
	gponGetCounter->gponGetCounter_table[2] = IO_GREG(GDMA2_RX_ETHDROPCNT);
	gponGetCounter->gponGetCounter_table[3] = IO_GREG(GDMA2_RX_ETHERPLEN);
	gponGetCounter->gponGetCounter_table[4] = IO_GREG(GDMA2_RX_ETHERPCNT);
	gponGetCounter->gponGetCounter_table[5] = IO_GREG(GDMA2_RX_ETHBCCNT);            
	gponGetCounter->gponGetCounter_table[6] = IO_GREG(GDMA2_RX_ETHMCCNT);
	gponGetCounter->gponGetCounter_table[7] = IO_GREG(GDMA2_RX_ETHCRCCNT);
	gponGetCounter->gponGetCounter_table[8] = IO_GREG(GDMA2_RX_ETHRUNTCNT);
	gponGetCounter->gponGetCounter_table[9] = IO_GREG(GDMA2_RX_ETHLONGCNT);
	gponGetCounter->gponGetCounter_table[10] = IO_GREG(GDMA2_RX_ETH_64_CNT);
	gponGetCounter->gponGetCounter_table[11] = IO_GREG(GDMA2_RX_ETH_65_TO_127_CNT);
	gponGetCounter->gponGetCounter_table[12] = IO_GREG(GDMA2_RX_ETH_128_TO_255_CNT);
	gponGetCounter->gponGetCounter_table[13] = IO_GREG(GDMA2_RX_ETH_256_TO_511_CNT);
	gponGetCounter->gponGetCounter_table[14] = IO_GREG(GDMA2_RX_ETH_512_TO_1023_CNT);
	gponGetCounter->gponGetCounter_table[15] = IO_GREG(GDMA2_RX_ETH_1024_TO_1518_CNT);
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

/***************************************************************
***************************************************************/
static int xmcs_test_gpon_hotplug(XPON_Mode_t enable)
{
	if(isFPGA) {
    	gpGponPriv->gponCfg.flags.hotplug = enable;
	}
	return 0;
}

/***************************************************************
***************************************************************/
#ifndef TCSUPPORT_CPU_EN7521
static int xmcs_get_upstream_block_state(uint *arg)
{
	if(MONITOR_DONE == gpPonSysData->Omci_Oam_Monitor.run_state)
	{
		put_user(0,arg);
	}
	else
	{
		put_user(1,arg);
	}	
	return 0;
}
#endif /* TCSUPPORT_CPU_EN7521 */


/***************************************************************
***************************************************************/
static int xmcs_get_gpon_online_duration(ulong *arg)
{
	unsigned long duration=0;
	
	if( GPON_TRAFFIC_UP == gwan_get_traffic_status() )
	{
		duration = (jiffies-gpPonSysData->onlineStartTime)/HZ;
		put_user(duration,arg); 
		return 0;	
	}
	else
	{
		put_user(duration,arg);
		return 0;
	}
}

/***************************************************************
***************************************************************/

int gpon_cmd_proc(uint cmd, ulong arg) 
{
	int ret = -1;
	
	//parser get/set command first
	
	switch(cmd) {
		case GPON_IOS_SN_PASSWD :
			ret = xmcs_set_sn_passwd((struct XMCS_GponSnPasswd_S *)arg) ;
			break ;		
		case GPON_IOS_SN :
			ret = xmcs_set_sn((struct XMCS_GponSnPasswd_S *)arg) ;
			break ;
		case GPON_IOS_PASSWD :
			ret = xmcs_set_passwd((struct XMCS_GponSnPasswd_S *)arg) ;
			break ;
		case GPON_IOS_EMERGENCY_STATE :
			ret = xmcs_set_emergency_state((XPON_Mode_t)arg) ;
			break;
		case GPON_IOS_ACT_TIMER :
			ret = xmcs_set_act_timer((struct XMCS_GponActTimer_S *)arg) ;
			break ;
		case GPON_IOS_ACT_TIMER_TO1 :
			ret = xmcs_set_act_timer_to1((uint)arg) ;
			break ;
		case GPON_IOS_ACT_TIMER_TO2 :
			ret = xmcs_set_act_timer_to2((uint)arg) ;
			break ;
		case GPON_IOG_CSR:
			ret = xmcs_get_csr();
			break ;
		case GPON_IOG_GEM_TABLE_INFO:
			ret = xmcs_get_gem_table_info();
			break ;
		case GPON_IOG_TCONT_TABLE_INFO:
			ret = xmcs_get_tcont_table_info();
			break ;
		case GPON_IOS_IDLE_GEM:
			ret = xmcs_set_idle_gem((ushort)arg);
			break ;
		case GPON_IOG_IDLE_GEM:
			ret = xmcs_get_idle_gem((ushort *)arg);
			break ;
		case GPON_IOG_ONU_INFO :
			ret = xmcs_get_onu_info((struct XMCS_GponOnuInfo_S *)arg) ;
			break ;
		case GPON_IOS_DBA_BLOCK_SIZE :
			ret = xmcs_set_gpon_block_size((ushort)arg) ;
			break ;
		case GPON_IOG_DBA_BLOCK_SIZE :
			ret = xmcs_get_gpon_block_size((ushort *)arg) ;
			break ;
		case GPON_IOS_GPON_TRTCM_MODE : 
			ret = xmcs_set_gpon_trtcm_mode((struct XMCS_GponTrtcmConfig_S *)arg) ;
			break ;
		case GPON_IOS_GPON_TRTCM_SCALE : 
			ret = xmcs_set_gpon_trtcm_scale((struct XMCS_GponTrtcmConfig_S *)arg) ;
			break ;
		case GPON_IOG_GPON_TRTCM_CONFIG :
			ret = xmcs_get_gpon_trtcm_config((struct XMCS_GponTrtcmConfig_S *)arg) ;
			break ;
		case GPON_IOS_GPON_TRTCM_PARAMS :
			ret = xmcs_set_gpon_trtcm_params((struct XMCS_GponTrtcmParams_S *)arg) ;
			break ;
		case GPON_IOG_GPON_TRTCM_PARAMS :
			ret = xmcs_get_gpon_trtcm_params((struct XMCS_GponTrtcmParams_S *)arg) ;
			break ;
		case GPON_IOS_TOD_CFG :
			ret = xmcs_set_gpon_tod((struct XMCS_GponTodCfg_S *)arg) ;
			break ;
		case GPON_IOG_CURRENT_TOD :
			ret = xmcs_get_gpon_tod((struct XMCS_GponTodCfg_S *)arg) ;
			break ;
		case GPON_IOG_NEW_TOD :
			ret = xmcs_get_gpon_new_tod((struct XMCS_GponTodCfg_S *)arg) ;
			break ;
		case GPON_IOG_TOD_SWITCH_TIME :
			ret = xmcs_get_tod_switch_time((struct XMCS_GponTodCfg_S *)arg) ;
			break ;
		case GPON_IOG_GEM_COUNTER :
			ret = xmcs_get_gem_counter((struct XMCS_GponGemCounter_S *)arg) ;
			break ;
        case GPON_IOS_CLEAR_GEM_COUNTER:
            ret = gponDevClearGemPortCounter((ushort)arg) ;
            break;
		case GPON_IOS_EQD_OFFSET_FLAG :
			ret = xmcs_set_eqd_offset_flag((char)arg) ;
			break ;
		case GPON_IOS_EQD_OFFSET_O4 :
			ret = xmcs_set_eqd_offset_o4((unchar)arg) ;
			break ;
		case GPON_IOS_EQD_OFFSET_O5 :
			ret = xmcs_set_eqd_offset_o5((unchar)arg) ;
			break ;
		case GPON_IOG_EQD_OFFSET :
			ret = xmcs_get_eqd_offset((struct XMCS_EqdOffset_S *)arg) ;
			break ;
		case GPON_IOG_SUPER_FRAME_COUNTER :
			ret = gponDevGetSuperframe((uint *)arg) ;
			break ;
		case GPON_IOS_COUNTER_TYPE :
			ret = xmcs_set_counter_type((GPON_COUNTER_TYPE_t)arg) ;
			break ;
		case GPON_IOG_COUNTER_TYPE :
			ret = xmcs_get_counter_type((GPON_COUNTER_TYPE_t *)arg) ;
			break ;
		case GPON_IOS_RESPONSE_TIME :
			ret = xmcs_set_response_time((ushort)arg) ;
			break ;
		case GPON_IOG_RESPONSE_TIME :
			ret = xmcs_get_response_time((ushort *)arg) ;
			break ;
		case GPON_IOS_INTERNAL_DELAY_FINE_TUNE :
			ret = xmcs_set_internal_delay_fine_tune((unchar)arg) ;
			break ;
		case GPON_IOG_INTERNAL_DELAY_FINE_TUNE :
			ret = xmcs_get_internal_delay_fine_tune((unchar *)arg) ;
			break ;
		case GPON_IOS_BURST_MODE_OVERHEAD_LEN :
			ret = xmcs_set_burst_mode_overhead((GPON_BURST_MODE_OVERHEAD_LEN_T)arg) ;
			break ;
		case GPON_IOG_BURST_MODE_OVERHEAD_LEN :
			ret = xmcs_get_burst_mode_overhead((GPON_BURST_MODE_OVERHEAD_LEN_T *)arg) ;
			break ;
		case GPON_IOS_CLEAR_COUNTER :
			ret = xmcs_set_clear_counter((GPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_IOS_KEY_SWITCH_TIME :
			ret = xmcs_set_key_switch_time((uint)arg) ;
			break ;
		case GPON_IOS_ENCRYPT_KEY :
			ret = xmcs_set_encrypt_key((unchar *)arg) ;
			break ;
		case GPON_IOG_ENCRYPT_KEY_INFO :
			ret = xmcs_get_encrypt_key_info((GPON_DEV_ENCRYPT_KEY_INFO_T *)arg) ;
			break ;
		case GPON_IOS_DYING_GASP_MODE :
			ret = xmcs_set_dying_gasp_mode((GPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_IOG_DYING_GASP_MODE :
			ret = xmcs_get_dying_gasp_mode((GPON_SW_HW_SELECT_T *)arg) ;
			break ;
		case GPON_IOS_DYING_GASP_NUM :
			ret = xmcs_set_dying_gasp_num((uint)arg) ;
			break ;
		case GPON_IOG_DYING_GASP_NUM :
			ret = xmcs_get_dying_gasp_num((uint *)arg) ;
			break ;
		case GPON_IOS_UP_TRAFFIC :
			ret = xmcs_set_up_traffic((GPON_DEV_UP_TRAFFIC_T *)arg) ;
			break ;
		case GPON_IOG_UP_TRAFFIC :
			ret = xmcs_get_up_traffic((GPON_DEV_UP_TRAFFIC_T *)arg) ;
			break ;
		case GPON_IOG_INT_MASK :
			ret = xmcs_get_int_mask((uint *)arg) ;
			break ;
#ifdef TCSUPPORT_CPU_EN7521
		case GPON_IOS_SNIFFER_GTC :
			ret = xmcs_set_sniffer_mode((GPON_DEV_SNIFFER_MODE_T *)arg) ;
			break ;
		case GPON_IOG_SNIFFER_GTC :
			ret = xmcs_get_sniffer_mode((GPON_DEV_SNIFFER_MODE_T *)arg) ;
			break ;
		case GPON_IOS_EXT_BST_LEN_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_set_ext_bst_len_ploamd_filter((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_EXT_BST_LEN_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_get_ext_bst_len_ploamd_filter((XPON_Mode_t *)arg) ;
			break ;
		case GPON_IOS_UP_OVERHEAD_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_set_up_overhead_ploamd_filter((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_UP_OVERHEAD_PLOAMD_FILTER_IN_O5 :
			ret = xmcs_get_up_overhead_ploamd_filter((XPON_Mode_t *)arg) ;
			break ;
		case GPON_IOS_TX_4BYTES_ALIGN :
			ret = xmcs_set_tx_4bytes_align((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOG_TX_4BYTES_ALIGN :
			ret = xmcs_get_tx_4bytes_align((XPON_Mode_t *)arg) ;
			break ;
		case GPON_IOS_DBA_BACKDOOR :
			ret = xmcs_set_dba_backdoor((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOS_DBA_BACKDOOR_TOTAL :
			ret = xmcs_set_dba_backdoor_total((uint)arg) ;
			break ;
		case GPON_IOS_DBA_BACKDOOR_GREEN :
			ret = xmcs_set_dba_backdoor_green((uint)arg) ;
			break ;
		case GPON_IOS_DBA_BACKDOOR_YELLOW :
			ret = xmcs_set_dba_backdoor_yellow((uint)arg) ;
			break ;
		case GPON_IOG_DBA_BACKDOOR :
			ret = xmcs_get_dba_backdoor((GPON_DEV_DBA_BACKDOOR_T *)arg) ;
			break ;
		case GPON_IOS_DBA_SLIGHT_MODIFY :
			ret = xmcs_set_dba_slight_modify((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOS_DBA_SLIGHT_MODIFY_TOTAL :
			ret = xmcs_set_dba_slight_modify_total((ushort)arg) ;
			break ;
		case GPON_IOS_DBA_SLIGHT_MODIFY_GREEN :
			ret = xmcs_set_dba_slight_modify_green((ushort)arg) ;
			break ;
		case GPON_IOS_DBA_SLIGHT_MODIFY_YELLOW :
			ret = xmcs_set_dba_slight_modify_yellow((ushort)arg) ;
			break ;
		case GPON_IOG_DBA_SLIGHT_MODIFY :
			ret = xmcs_get_dba_slight_modify((GPON_DEV_SLIGHT_MODIFY_T *)arg) ;
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY :
			ret = xmcs_set_dba_shift_modify((XPON_Mode_t)arg) ;
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_TOTAL :
			ret = xmcs_set_dba_shift_modify_total((struct XMCS_DBAShiftMod_S *)arg) ;
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_GREEN :
			ret = xmcs_set_dba_shift_modify_green((struct XMCS_DBAShiftMod_S *)arg) ;
			break ;
		case GPON_IOS_DBA_SHIFT_MODIFY_YELLOW :
			ret = xmcs_set_dba_shift_modify_yellow((struct XMCS_DBAShiftMod_S *)arg) ;
			break ;
		case GPON_IOS_O3_O4_PLOAM_CTRL :
			ret = xmcs_set_o3_o4_ploam_ctrl((GPON_SW_HW_SELECT_T)arg) ;
			break ;
		case GPON_IOG_O3_O4_PLOAM_CTRL :
			ret = xmcs_get_o3_o4_ploam_ctrl((GPON_SW_HW_SELECT_T *)arg) ;
			break ;
		case GPON_IOG_TX_SYNC :
			ret = xmcs_get_tx_sync_offset((unchar *)arg) ;
			break ;
		case GPON_IOS_AES_KEY_SWITCH_BY_SW :
			ret = xmcs_set_aes_key_switch_by_sw() ;
			break ;
		case GPON_IOS_SW_RESET :
			ret = xmcs_set_sw_reset() ;
			break ;
		case GPON_IOS_SW_RESYNC :
			ret = xmcs_set_sw_resync() ;
			break ;
		case GPON_IOS_RESET_SERVICE :
			ret = xmcs_gpon_reset_service();
			break ;

		case GPON_IOS_1PPS_HIGH_WIDTH :
			if (isEN7526c || isEN751627) {
				ret = xmcs_set_1pps_h_w((uint)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
		case GPON_IOG_1PPS_HIGH_WIDTH :
			if (isEN7526c || isEN751627) {
				ret = xmcs_get_1pps_h_w((uint *)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
		case GPON_IOS_SEND_PLOAMU_WAIT_MODE :
			if (isEN7526c || isEN751627) {
				ret = xmcs_set_send_ploamu_wait_mode((GPON_DEV_SEND_PLOAMU_WAIT_MODE_T)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
		case GPON_IOG_SEND_PLOAMU_WAIT_MODE :
			if (isEN7526c || isEN751627) {
				ret = xmcs_get_send_ploamu_wait_mode((GPON_DEV_SEND_PLOAMU_WAIT_MODE_T *)arg) ;
			} else {
				/* only supported by EN7526FC */
				ret = -1;
			}
			break ;
#endif /* TCSUPPORT_CPU_EN7521 */
		/* TEST cmd */		
		case GPON_IOS_TEST_SEND_PLOAM_SLEEP_REQUEST :
			ret = xmcs_test_send_ploam_sleep_request((GPON_PLOAMu_SLEEP_MODE_t)arg) ;
			break ;
		case GPON_IOS_TEST_SEND_PLOAM_REI :
			ret = xmcs_test_send_ploam_rei((uint)arg) ;
			break ;
		case GPON_IOS_TEST_SEND_PLOAM_DYING_GASP :
			ret = xmcs_test_send_ploam_dying_gasp() ;
			break ;
		case GPON_IOS_TEST_TOD :
			ret = xmcs_test_tod((uint)arg) ;
			break ;
		case GPON_IOS_GET_RX_ETHERNET_FRAME_COUNTER :
			ret = xmcs_get_rx_ethernet_frame_counter((struct XMCS_GponGetCounter_S *)arg);
			break;
		case GPON_GET_TRAFFIC_STATE :
			ret = xmcs_get_gpon_traffic_state((uint*) arg);
			break;
		case GPON_IOS_TEST_HOTPLUG :
			ret = xmcs_test_gpon_hotplug((XPON_Mode_t) arg);
			break;
#ifndef TCSUPPORT_CPU_EN7521
		case XPON_GET_UPSTREAM_BLOCK_STATE :
			ret = xmcs_get_upstream_block_state((uint*) arg);
			break;
#endif /* TCSUPPORT_CPU_EN7521 */
		case GPON_GET_ONLINE_DURATION :
			ret = xmcs_get_gpon_online_duration((ulong*) arg);
			break;	
            
		default:
			PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
			break ;
	}

	return ret ;
}

